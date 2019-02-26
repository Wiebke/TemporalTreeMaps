/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <modules/tools/volumesourceseries.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/io/rawvolumereader.h>

namespace inviwo
{
namespace kth
{



// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeSourceSeries::processorInfo_
{
    "org.inviwo.VolumeSourceSeries",  // Class identifier
    "Volume Series",                    // Display name
    "Data Input",                       // Category
    CodeState::Experimental,            // Code state
    Tags::CPU,                          // Tags
};

const ProcessorInfo VolumeSourceSeries::getProcessorInfo() const
{
    return processorInfo_;
}


VolumeSourceSeries::VolumeSourceSeries()
    :Processor()
    ,resVolumeSeries("volumeSeriesOutport")
    ,resVolume("volumeOutport")
    ,propFileNames("FileNames", "File List", "default", InvalidationLevel::InvalidResources)
    ,propMaxCacheSize("MaxCacheSize", "Cache", 10, 1)
    ,propCacheInfo("CacheInfo", "Cache Info", "Loaded 0/0")
    ,propFileIndex("fileIndex", "File Index", 1, 1, 1, 1)
    //,propBasisInfo("Basis", "Basis and offset")
    ,propVolumeInfo("Information", "Data information")
    ,propOptionOverRideDataRange("OptionOverRideDataRange", "Override Data Range")
    ,propOverRideDataRange("OverRideDataRange", "Data Range", 0., 255.0, -DataFloat64::max(), DataFloat64::max(), 0.0, 0.0, InvalidationLevel::InvalidOutput, PropertySemantics("Text"))
    ,SSD(this)
    ,VolumeCache(propMaxCacheSize.get())
{
    addPort(resVolumeSeries);
    resVolumeSeries.setData(std::make_shared<VolumeSourceSeriesData>(SSD));

    addPort(resVolume);

    propFileNames.setContentType("volume");
    addFileNameFilters();
    addProperty(propFileNames);

    propFileNames.set(&propFileNames);

    addProperty(propMaxCacheSize);
    addProperty(propCacheInfo);
    addProperty(propFileIndex);
    //addProperty(propBasisInfo);
    addProperty(propVolumeInfo);
    addProperty(propOptionOverRideDataRange);
    addProperty(propOverRideDataRange);

    propFileNames.onChange([&]() { update(); });
    //propFileIndex.onChange([&]() { update(); });

    bRAWSettingsInitialized = false;
}

void VolumeSourceSeries::addFileNameFilters()
{
    auto rf = InviwoApplication::getPtr()->getDataReaderFactory();
    auto extensions = rf->getExtensionsForType<Volume>();
    propFileNames.clearNameFilters();
    for (auto& ext : extensions)
    {
        propFileNames.addNameFilter(ext.description_ + " (*." + ext.extension_ + ")");
    }
    extensions = rf->getExtensionsForType<VolumeSequence>();
    for (auto& ext : extensions)
    {
        propFileNames.addNameFilter(ext.description_ + " (*." + ext.extension_ + ")");
    }
}


size_t VolumeSourceSeries::GetNumFiles() const
{
    return propFileNames.get().size();
}


std::shared_ptr<Volume> VolumeSourceSeries::GetVolume(const size_t Idx)
{
    std::shared_ptr< Volume > pVolume = NULL;

    //Let's see if we have it in the cache.
    pVolume = VolumeCache.Find(Idx);
    if (pVolume) return pVolume;

    //Shorthand to file names
    const std::vector< std::string >& FileNames = propFileNames.get();
    
    //Safety
    const size_t NumFiles = FileNames.size();
    if (NumFiles == 0) return NULL;
    if (/*Idx < 0 ||*/ Idx >= NumFiles) return NULL;

    //Get current file name
    const std::string& CurrentFileName = FileNames[Idx];

    //Load
    auto rf = InviwoApplication::getPtr()->getDataReaderFactory();
    const std::string ext = filesystem::getFileExtension(CurrentFileName);
    if (auto volVecReader = rf->getReaderForTypeAndExtension<VolumeSequence>(ext))
    {
        try
        {
			//Read
            auto volumes = volVecReader->readData(CurrentFileName);
			pVolume = (*volumes)[0];
        }
        catch (DataReaderException const& /*e*/)
        {
            //LogProcessorError("Could not load data: " << CurrentFileName << ", " << e.getMessage());
			return pVolume;
        }
    }
    else
    if (auto volreader = rf->getReaderForTypeAndExtension<Volume>(ext))
    {
        try
        {
            //Check if we are using a RAWReader again, if yes use old settings
            if (dynamic_cast<RawVolumeReader*>(volreader.get()) && bRAWSettingsInitialized)
            {
                static_cast<RawVolumeReader*>(volreader.get())->setParameters(format, dimensions, littleEndian, dataMap);
            }
            //Read
            pVolume = volreader->readData(CurrentFileName);

            //Check if we have a RAW reader
            if (dynamic_cast<RawVolumeReader*>(volreader.get()) && !bRAWSettingsInitialized)
            {
                //Save settings
                auto volreaderRAW = static_cast<RawVolumeReader*>(volreader.get());
                littleEndian = volreaderRAW->haveReadLittleEndian();
                dimensions = static_cast<ivec3>(pVolume->getDimensions());
                format = const_cast<DataFormatBase*>(volreaderRAW->getFormat());
                dataMap = pVolume->dataMap_;
                bRAWSettingsInitialized = true;
            }
        }
        catch (DataReaderException const& /*e*/)
        {
            //LogProcessorError("Could not load data: " << CurrentFileName << ", " << e.getMessage());
			return pVolume;
		}
    }
    else
    {
        //LogProcessorError("Could not find a data reader for file: " << CurrentFileName);
		return pVolume;
    }

	assert(pVolume);

	//Add to cache
	VolumeCache.Add(Idx, pVolume);

    //Show Cache Info
    updateCacheInfo();

    return pVolume;
}


void VolumeSourceSeries::update()
{
    //Shorthand to file names
    const std::vector< std::string >& FileNames = propFileNames.get();

    //Get number of files
    const int NumFiles = (int)FileNames.size();

    //Update Max Cache Size and Info
    propMaxCacheSize.setMaxValue(NumFiles);

    //Update indexer
    propFileIndex.setMinValue(0);
    propFileIndex.setMaxValue(NumFiles - 1);
}


void VolumeSourceSeries::process()
{
    //inviwo::InvalidationLevel ILevel = this->getInvalidationLevel();
    //LogInfo("InvalidationLevel " << (int)ILevel);

    update();

    //Set Cache Size
    if (propMaxCacheSize.get() != VolumeCache.GetMaxCacheSize())
    {
        VolumeCache.SetMaxCacheSize(propMaxCacheSize.get());
        updateCacheInfo();
    }

    //Add volume to single volume outport
    const size_t idxCurrentFile = (size_t)propFileIndex.get();
    resVolume.setData(GetVolume(idxCurrentFile));

    //Add info to volume series data port
    // - taken care of, since that port refers back to us!

    //Show info
    if (resVolume.getData())
    {
        //propBasisInfo.updateForNewEntity(*resVolume.getData(), true);
        propVolumeInfo.updateForNewVolume(*resVolume.getData(), false);
    }
}
} // namespace kth
} // namespace
