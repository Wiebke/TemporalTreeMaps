/*********************************************************************
 *  Author  : Tino Weinkauf
 *  Init    : Sunday, March 11, 2018 - 11:12:19
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
#include <inviwo/core/properties/stringproperty.h>
#include <modules/temporaltreemaps/datastructures/treeport.h>

namespace inviwo
{
namespace kth
{

/** \docpage{org.inviwo.TemporalTreeStatistics, Tree Statistics}
    ![](org.inviwo.TemporalTreeStatistics.png?classIdentifier=org.inviwo.TemporalTreeStatistics)

    Reports statistics about a temporal tree.
    
    ### Inports
      * __<Inport1>__ <description>.
    
    ### Outports
      * __<Outport1>__ <description>.
    
    ### Properties
      * __<Prop1>__ <description>.
      * __<Prop2>__ <description>
*/


/** \class TemporalTreeStatistics
    \brief Reports statistics about the tree
    
    Reports number of nodes and edges
    as well as how many nodes and edges
    it would have in case it was not aggregated.

    @author Tino Weinkauf
*/
class IVW_MODULE_TEMPORALTREEMAPS_API TemporalTreeStatistics : public Processor
{ 
//Friends
//Types
public:

//Construction / Deconstruction
public:
    TemporalTreeStatistics();
    virtual ~TemporalTreeStatistics() = default;

//Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    ///Our main computation function
    virtual void process() override;

//Ports
public:
    ///Input tree for which we generate some stats
    TemporalTreeInport portInTree;

//Properties
public:
    ///Shows statistics of the given tree
    StringProperty propStatGivenTree;

    ///Shows statistics of the non-aggregated version of the input tree
    StringProperty propStatNonAggregatedTree;

//Attributes
private:

};

} // namespace
} // namespace
