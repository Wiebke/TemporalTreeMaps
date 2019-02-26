/*********************************************************************
 *  Author  : Wiebke Koepp
 *  Init    : Monday, September 17, 2018 - 15:18:48
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#pragma once

#include <modules/tools/toolsmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <modules/tools/volumesourceseries.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/datastructures/geometry/geometrytype.h>
#include <inviwo/core/properties/optionproperty.h>

namespace inviwo
{
namespace kth
{

/** \docpage{org.inviwo.VolumeSeriesFromSlices, Volume Series From Slices}
    ![](org.inviwo.VolumeSeriesFromSlices.png?classIdentifier=org.inviwo.VolumeSeriesFromSlices)

    Explanation of how to use the processor.
    
    ### Inports
      * __<Inport1>__ <description>.
    
    ### Outports
      * __<Outport1>__ <description>.
    
    ### Properties
      * __<Prop1>__ <description>.
      * __<Prop2>__ <description>
*/


/** \class VolumeSeriesFromSlices
    \brief VERY_BRIEFLY_DESCRIBE_THE_PROCESSOR
    
    DESCRIBE_THE_PROCESSOR_FROM_A_DEVELOPER_PERSPECTIVE

    @author Wiebke Koepp
*/
class IVW_MODULE_TOOLS_API VolumeSeriesFromSlices : public VolumeSourceSeries
{ 
//Friends
//Types
public:

//Construction / Deconstruction
public:
    VolumeSeriesFromSlices();
    virtual ~VolumeSeriesFromSlices() = default;

//Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    ///Updates User Interface.
    void update() override;

    ///Returns number of volumes in the series.
    size_t GetNumFiles() const override;

protected:
    ///Our main computation function
    virtual void process() override;

    void fillCache();

//Ports
public:
    VolumeInport inVol;


//Properties
public:
    TemplateOptionProperty<CartesianCoordinateAxis> propSliceAlongAxis;

//Attributes
private:

};

} // namespace kth
} // namespace
