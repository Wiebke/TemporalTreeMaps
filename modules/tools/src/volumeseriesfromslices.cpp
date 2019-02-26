/*********************************************************************
 *  Author  : Wiebke Koepp
 *  Init    : Monday, September 17, 2018 - 15:18:48
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <modules/tools/volumeseriesfromslices.h>
#include <modules/base/algorithm/volume/volumeramsubset.h>

namespace inviwo
{
namespace kth
{

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeSeriesFromSlices::processorInfo_
{
    "org.inviwo.VolumeSeriesFromSlices",      // Class identifier
    "Volume Series From Slices",                // Display name
    "Undefined",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};

const ProcessorInfo VolumeSeriesFromSlices::getProcessorInfo() const
{
    return processorInfo_;
}

VolumeSeriesFromSlices::VolumeSeriesFromSlices()
    :VolumeSourceSeries()
    , inVol("volumeInport")
    , propSliceAlongAxis("sliceAxis", "Slice along axis")
{

    addPort(inVol);
    inVol.onChange([&]() { fillCache(); });

    // Hide/Alter the properties this processor does not use or uses differently
    propFileNames.setVisible(false);

    propFileIndex.setDisplayName("Slice Index");

    // Additional property for the slicing axis
    propSliceAlongAxis.addOption("x", "y-z plane (X axis)",
        CartesianCoordinateAxis::X);
    propSliceAlongAxis.addOption("y", "z-x plane (Y axis)",
        CartesianCoordinateAxis::Y);
    propSliceAlongAxis.addOption("z", "x-y plane (Z axis)",
        CartesianCoordinateAxis::Z);
    propSliceAlongAxis.set(CartesianCoordinateAxis::Z);
    propSliceAlongAxis.setCurrentStateAsDefault();
    propSliceAlongAxis.onChange([this]() { fillCache(); });
    addProperty(propSliceAlongAxis);

}

void VolumeSeriesFromSlices::update()
{
    // Update max for cachesize and file index
    const int NumSlices = (int)GetNumFiles();
    propMaxCacheSize.setMaxValue(NumSlices);
    propMaxCacheSize.set(NumSlices);

    if (VolumeCache.GetMaxCacheSize() != static_cast<size_t>(propMaxCacheSize.get()))
    {
        VolumeCache.SetMaxCacheSize(propMaxCacheSize.get());
        updateCacheInfo();
    }

    //Update indexer
    propFileIndex.setMinValue(0);
    propFileIndex.setMaxValue(NumSlices - 1);
}

size_t VolumeSeriesFromSlices::GetNumFiles() const
{
    if (!inVol.hasData())
    {
        return 0;
    }

    // Number of the slices is equal to dimensionality in the slicing axis
    const size3_t dims{ inVol.getData()->getDimensions() };
    switch (propSliceAlongAxis)
    {
    case CartesianCoordinateAxis::X:
        return dims.x;
    case CartesianCoordinateAxis::Y:
        return dims.y;
    case CartesianCoordinateAxis::Z:
        return dims.z;
    default:
        return dims.z;
    }
}

void VolumeSeriesFromSlices::process()
{
    VolumeSourceSeries::process();
}

void VolumeSeriesFromSlices::fillCache()
{
    if (!inVol.hasData()) return;

    // Update max for cachesize and file index
    update();
    
    size_t numSlices;
    const size3_t dims{ inVol.getData()->getDimensions() };
    size3_t dimsSlice;
    size3_t offsetInc;

    switch (propSliceAlongAxis)
    {
    case CartesianCoordinateAxis::X:
        numSlices = dims.x;
        dimsSlice = size3_t(1, dims.y, dims.z);
        offsetInc = size3_t(1, 0, 0);
        break;
    case CartesianCoordinateAxis::Y:
        numSlices = dims.y;
        dimsSlice = size3_t(dims.x, 1, dims.z);
        offsetInc = size3_t(0, 1, 0);
        break;
    case CartesianCoordinateAxis::Z:
        numSlices = dims.z;
        dimsSlice = size3_t(dims.x, dims.y, 1);
        offsetInc = size3_t(0, 0, 1);
        break;
    default:
        numSlices = dims.z;
        dimsSlice = size3_t(dims.x, dims.y, 1);
        offsetInc = size3_t(0, 0, 1);
        break;
    }

    size3_t offset = size3_t(0, 0, 0);

    for (size_t idx = 0; idx < numSlices; idx++, offset+=offsetInc)
    {
        const VolumeRAM* vol = inVol.getData()->getRepresentation<VolumeRAM>();
        std::shared_ptr<Volume> pVolume = std::make_shared<Volume>(Volume(VolumeRAMSubSet::apply(vol, dimsSlice, offset)));
        // Copy meta data
        // pass meta data on
        pVolume->copyMetaDataFrom(*inVol.getData());
        pVolume->dataMap_ = inVol.getData()->dataMap_;

        VolumeCache.Add(idx, pVolume);
    }

}

} // namespace kth
} // namespace

