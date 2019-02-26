/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Saturday, January 06, 2018 - 15:58:11
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
//#include <inviwo/core/properties/boolproperty.h>
//#include <inviwo/core/properties/buttonproperty.h>
//#include <inviwo/core/properties/compositeproperty.h>
//#include <inviwo/core/properties/fileproperty.h>
//#include <inviwo/core/properties/minmaxproperty.h>
//#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
//#include <inviwo/core/properties/stringproperty.h>
//#include <inviwo/core/properties/transferfunctionproperty.h>

namespace inviwo
{
namespace kth
{

/** \docpage{org.inviwo.TemporalTreeCushionComputation, Tree Cushion Computation}
    ![](org.inviwo.TemporalTreeCushionComputation.png?classIdentifier=org.inviwo.TemporalTreeCushionComputation)

    Explanation of how to use the processor.
    
    ### Inports
      * __<Inport1>__ <description>.
    
    ### Outports
      * __<Outport1>__ <description>.
    
    ### Properties
      * __<Prop1>__ <description>.
      * __<Prop2>__ <description>
*/


/** \class TemporalTreeCushionComputation
    \brief VERY_BRIEFLY_DESCRIBE_THE_PROCESSOR
    
    DESCRIBE_THE_PROCESSOR_FROM_A_DEVELOPER_PERSPECTIVE

    @author Tino Weinkauf and Wiebke Koepp
*/
class IVW_MODULE_TEMPORALTREEMAPS_API TemporalTreeCushionComputation : public Processor
{ 
//Friends
//Types
public:

//Construction / Deconstruction
public:
    TemporalTreeCushionComputation();
    virtual ~TemporalTreeCushionComputation() = default;

//Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    ///Our main computation function
    virtual void process() override;

    void traverseToLeavesForCushions(TemporalTree& tree, size_t nodeIndex, uint64_t startTime, uint64_t endTime, uint8_t depth);


//Ports
public:
    ///Input tree
    TemporalTreeInport portInTree;

    ///Tree with cushions if upper and lower limits are set
    TemporalTreeOutport portOutTree;

    FloatProperty propCushionBaseHeight;
    FloatProperty propCushionScaleFactor;

    IntProperty propCushionFrom;
    IntProperty propCushionTo;

//Properties
public:

//Attributes
private:

};

} // namespace kth
} // namespace
