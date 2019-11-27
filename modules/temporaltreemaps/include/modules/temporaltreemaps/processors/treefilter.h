/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Thursday, November 30, 2017 - 16:22:12
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
//#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>

namespace inviwo
{
namespace kth
{

/** \docpage{org.inviwo.TemporalTreeFilter, Tree Filter}
    ![](org.inviwo.TemporalTreeFilter.png?classIdentifier=org.inviwo.TemporalTreeFilter)

    Explanation of how to use the processor.
    
    ### Inports
      * __<Inport1>__ <description>.
    
    ### Outports
      * __<Outport1>__ <description>.
    
    ### Properties
      * __<Prop1>__ <description>.
      * __<Prop2>__ <description>
*/


/** \class TemporalTreeFilter
    \brief Filter values or entire leaves of a temporal tree 


    @author Tino Weinkauf and Wiebke Köpp
*/
class IVW_MODULE_TEMPORALTREEMAPS_API TemporalTreeFilter : public Processor
{ 
//Friends
//Types
public:

//Construction / Deconstruction
public:
    TemporalTreeFilter();
    virtual ~TemporalTreeFilter() = default;

//Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    ///Our main computation function
    virtual void process() override;

    ///Fading the values of leaves in and out linearly
    void fadeLeaves(std::shared_ptr<TemporalTree>& tree);
    
    ///Linear interpolation to y for x in between (x0, y0) and (x1, y1) 
    float linearInterpolation(const std::pair<uint64_t, float>& left, const std::pair<uint64_t, float>& right, const uint64_t x);
    
    /// Filter out leaves 
    void filterLeaves(std::shared_ptr<TemporalTree>& tree);

    /// Split up nodes that have temporally become leafes, so that in that time, they are actual leaves
    void splitTemporalLeaves(std::shared_ptr<TemporalTree>& tree);

//Ports
public:
    TemporalTreeInport portInTree;
    TemporalTreeOutport portOutTree;

//Properties
public:
    CompositeProperty propFading;
    BoolProperty propDoFade;
    IntSizeTProperty propDeltaTFade;

    CompositeProperty propFilter;
    BoolProperty propDoFilter;

    IntSizeTProperty propMinLifespan;

    BoolProperty propDoFilterTimeSpan;
    IntSizeTProperty propStartLifespan;
    IntSizeTProperty propEndLifespan;
    
    IntSizeTProperty propFilterDepth;

    BoolProperty propDoFilterFirstLevel;
    CompositeProperty propFilterFirstLevelNodes;

    BoolProperty propDoSplitLeaves;

//Attributes
private:
};

} // namespace kth
} // namespace
