/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Saturday, January 06, 2018 - 15:57:52
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#pragma once

#include <modules/temporaltreemaps/temporaltreemapsmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <modules/temporaltreemaps/datastructures/treeport.h>
//#include <inviwo/core/ports/volumeport.h>
//#include <inviwo/core/ports/meshport.h>
//#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/boolproperty.h>
//#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
//#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
//#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>
//#include <inviwo/core/properties/transferfunctionproperty.h>

namespace inviwo
{
namespace kth
{

/** \docpage{org.inviwo.TemporalTreeLayoutComputation, Tree Layout Computation}
    ![](org.inviwo.TemporalTreeLayoutComputation.png?classIdentifier=org.inviwo.TemporalTreeLayoutComputation)

    Explanation of how to use the processor.
    
    ### Inports
      * __<Inport1>__ <description>.
    
    ### Outports
      * __<Outport1>__ <description>.
    
    ### Properties
      * __<Prop1>__ <description>.
      * __<Prop2>__ <description>
*/


/** \class TemporalTreeLayoutComputation
    \brief VERY_BRIEFLY_DESCRIBE_THE_PROCESSOR
    
    DESCRIBE_THE_PROCESSOR_FROM_A_DEVELOPER_PERSPECTIVE

    @author Tino Weinkauf and Wiebke Koepp
*/
class IVW_MODULE_TEMPORALTREEMAPS_API TemporalTreeLayoutComputation : public Processor
{ 
//Friends
//Types
public:

//Construction / Deconstruction
public:
    TemporalTreeLayoutComputation();
    virtual ~TemporalTreeLayoutComputation() = default;

//Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    ///Our main computation function
    virtual void process() override;

    ///Normalize a given value according to the given value map
    float normalValue(TemporalTree::TValueMap mapForNormalization, uint64_t time, float value) const;

    ///Add the normalized given values to the drawing limit
    void updateUpper(TemporalTree::TDrawingLimitMap& upperCurrent, TemporalTree::TValueMap& values,
        TemporalTree::TValueMap& mapForNormalization);

    ///Spread the upper and lower limit towars the root 
    ///(For each node recursively visit all parents for the time frame in which they are the parent)
    void traverseToRootForLimits(TemporalTree& tree, size_t nodeIndex, 
        uint64_t startTime, uint64_t endTime);

    ///Copy a drawing limit for a given time frame
    void copyLimitInBetween(TemporalTree::TDrawingLimitMap& upperCurrent, 
        TemporalTree::TDrawingLimitMap& limitToFill, 
        uint64_t startTime, uint64_t endTime);


//Ports
public:
    ///Input tree
    TemporalTreeInport portInTree;

    ///Tree with computed upper and lower drawing limits
    TemporalTreeOutport portOutTree;

//Properties
public:

    BoolProperty propSpaceFilling;

    FloatProperty propMaximum;
    StringProperty propActualMaximum;

    CompositeProperty propRenderInfo;
    BoolProperty propUnixTime;
    DoubleMinMaxProperty propTimeMinMax;
    DoubleMinMaxProperty propValueMinMax;


//Attributes
private:
};

} // namespace kth
} // namespace
