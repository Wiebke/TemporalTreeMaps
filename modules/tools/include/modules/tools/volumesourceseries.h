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

#pragma once

#include <modules/tools/toolsmoduledefine.h>

#include <modules/tools/filelistproperty.h>
#include <modules/tools/volumesourceseriesdata.h>
#include <modules/tools/simplelrucache.h>
    
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <modules/base/properties/basisproperty.h>
#include <modules/base/properties/volumeinformationproperty.h>
#include <inviwo/core/properties/boolproperty.h>


namespace inviwo
{
namespace kth
{ 

/** \docpage{org.inviwo.VolumeSourceSeries, Volume Series}
 * ![](org.inviwo.VolumeSourceSeries.png?classIdentifier=org.inviwo.VolumeSourceSeries)
 *
 * Loads a series of volumes, one at the time.
 * 
 * ### Outports
 *   * __Outport__ The currently loaded volume.
 *
 * ### Properties
 *   * __File names__ Files to load.
 *   * __Time step__ To select the current volume.
 *
 */
class IVW_MODULE_TOOLS_API VolumeSourceSeries : public Processor
{
//Friends
//Types
public:

//Construction / Deconstruction
public:
    VolumeSourceSeries();
    virtual ~VolumeSourceSeries() = default;

//Info
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

//Methods
public:
    ///Returns number of volumes in the series.
    virtual size_t GetNumFiles() const;

    ///Returns a particular volume of the series. @note May be NULL.
    std::shared_ptr< Volume > GetVolume(const size_t Idx);

protected:
    ///Updates User Interface.
    virtual void update();

    ///Our main computation method.
    virtual void process();

    ///Adds the file extension filters to the filelistproperty
    void addFileNameFilters();

    ///Updates the info display
    void updateCacheInfo() const
    {
        propCacheInfo.set("Filled "
                    + std::to_string(VolumeCache.GetCacheSize())
                    + " / "
                    + std::to_string(VolumeCache.GetMaxCacheSize()));
    }

//Ports
private:
    ///Resulting volume series
    VolumeSeriesOutport resVolumeSeries;

	///Result as a current time step
    VolumeOutport resVolume;

//Properties
public:
    ///File names of volumes to load.
    FileListProperty propFileNames;

    ///Maximal cache size
    IntProperty propMaxCacheSize;

    ///Info about the cache
    mutable StringProperty propCacheInfo;

	///Which volume to load.
    IntProperty propFileIndex;

    ///Information on the bounding box
    //BasisProperty propBasisInfo;

    ///Information about values and resolution
    VolumeInformationProperty propVolumeInfo;

    ///Whether to override the data range
    BoolProperty propOptionOverRideDataRange;

    ///Values for enforced data range
    DoubleMinMaxProperty propOverRideDataRange;


//Attributes
protected:
    ///The Series Data object to be put into the VolumeSeriesOutport resVolumeSeries.
    VolumeSourceSeriesData SSD;

    ///Cache of loaded volumes.
    mutable SimpleLRUCache<size_t, Volume> VolumeCache;

private:
    ///Settings in case a RAW Reader is used
    bool bRAWSettingsInitialized;
    bool littleEndian;
    ivec3 dimensions;
    DataFormatBase* format;
    DataMapper dataMap;
};

} // namespace kth
} // namespace

