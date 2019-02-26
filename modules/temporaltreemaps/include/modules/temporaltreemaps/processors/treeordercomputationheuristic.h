/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Friday, March 23, 2018 - 15:58:29
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
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <modules/temporaltreemaps/processors/treeordercomputation.h>
#include <modules/temporaltreemaps/datastructures/constraint.h>


namespace inviwo
{
namespace kth
{

/** \docpage{org.inviwo.TemporalTreeOrderComputationHeuristic, Tree Order Computation Heuristic}
    ![](org.inviwo.TemporalTreeOrderComputationHeuristic.png?classIdentifier=org.inviwo.TemporalTreeOrderComputationHeuristic)

    Explanation of how to use the processor.
    
    ### Inports
      * __<Inport1>__ <description>.
    
    ### Outports
      * __<Outport1>__ <description>.
    
    ### Properties
      * __<Prop1>__ <description>.
      * __<Prop2>__ <description>
*/

using namespace constraint;

/** \class TemporalTreeOrderComputationHeuristic
    \brief VERY_BRIEFLY_DESCRIBE_THE_PROCESSOR
    
    DESCRIBE_THE_PROCESSOR_FROM_A_DEVELOPER_PERSPECTIVE

    @author Tino Weinkauf and Wiebke Koepp
*/
class IVW_MODULE_TEMPORALTREEMAPS_API TemporalTreeOrderComputationHeuristic : public TemporalTreeOrderOptimization
{ 
//Friends
//Types
public:

//Construction / Deconstruction
public:
    TemporalTreeOrderComputationHeuristic();
    virtual ~TemporalTreeOrderComputationHeuristic() = default;

//Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    static void findConflictingLeaves(std::shared_ptr<const TemporalTree>& tree, const Constraint& constraint, const TemporalTree::TTreeOrder& order, size_t& minOrder, size_t& maxOrder, TemporalTree::TTreeOrder& conflictingLeaves, TemporalTree::TTreeOrder& nonConflictingAndConstraintLeaves);

    static void buildNewOrder(TemporalTree::TTreeOrder& newOrder, const TemporalTree::TTreeOrder& order, const size_t numConflictBefore, const TemporalTree::TTreeOrder& conflictingLeaves,
        const TemporalTree::TTreeOrder& nonConflictingAndConstraintLeaves, const size_t minOrder, const size_t maxOrder);

protected:
    bool resolveConstraint(std::shared_ptr<const TemporalTree> tree, Constraint& constraint);

    ///Initial order
    void initializeResources() override;

    void restart() override;

    ///Is the optimization converged
    bool isConverged() override;

    ///Do a single optimization step
    void singleStep() override;

    ///Run until convergence criterion is reached
    void runUntilConvergence() override;

	void prepareNextStep();

    void logStep() override;

    void initializeLog() override;

    void logProperties() override;

    ///Our main computation function (Does nothing)
    virtual void process() override;

//Ports
public:

//Properties
public:
    ///Initial ordering of all unfulfilled constraints
    OptionPropertyInt propInitialConstraintOrder;

//Attributes
private:
    ///Order for the current order
    TemporalTree::TTreeOrderMap currentOrderMap;

	///Sorted constraints that we operate on
	std::vector<size_t> constraintOrder;

    ///Queue of unfulfilled constraints
    std::queue<size_t> constraintsQueue;
    
};

} // namespace
} // namespace
