/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Wednesday, March 28, 2018 - 11:45:51
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
#include <modules/temporaltreemaps/processors/treeordercomputationsa.h>

namespace inviwo
{
namespace kth
{

/** \docpage{org.inviwo.TreeOrderComputationSAConstraints, Tree Order Computation SAConstraints}
    ![](org.inviwo.TreeOrderComputationSAConstraints.png?classIdentifier=org.inviwo.TreeOrderComputationSAConstraints)

    Explanation of how to use the processor.
    
    ### Inports
      * __<Inport1>__ <description>.
    
    ### Outports
      * __<Outport1>__ <description>.
    
    ### Properties
      * __<Prop1>__ <description>.
      * __<Prop2>__ <description>
*/


/** \class TreeOrderComputationSAConstraints
    \brief VERY_BRIEFLY_DESCRIBE_THE_PROCESSOR
    
    DESCRIBE_THE_PROCESSOR_FROM_A_DEVELOPER_PERSPECTIVE

    @author Tino Weinkauf and Wiebke Koepp
*/
class IVW_MODULE_TEMPORALTREEMAPS_API TemporalTreeOrderComputationSAConstraints : public TemporalTreeSimulatedAnnealing
{ 
//Friends
//Types
public:

//Construction / Deconstruction
public:
	TemporalTreeOrderComputationSAConstraints();
    virtual ~TemporalTreeOrderComputationSAConstraints() = default;

//Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
	///Resolve a single given constraint, either by choosing the best or a random resolution
	///If multiple "best", choose at random from them
    void resolveConstraint(std::shared_ptr<const TemporalTree> tree,
        Constraint& constraint);

    ///Neighbor Solution from the current state 
    void neighborSolution() override;

    ///Initalize everything
    void initializeResources() override;

    ///Reset only statistic things and settings
    void restart() override;

    void setLastToCurrent() override;

    void setCurrentToLast() override;

    void setBest() override;

    void prepareNextStep() override;

    ///Our main computation function
    virtual void process() override;

//Ports
public:

//Properties
public:
    ///Once a constraint to resolve has been choosen
    ///either choose the best one or random 
    BoolProperty propResolveWithBest;

//Attributes
private:
    ///All current unfulfilled constraints
    std::vector<size_t> unfulfilledConstraints;
};

} // namespace
} // namespace
