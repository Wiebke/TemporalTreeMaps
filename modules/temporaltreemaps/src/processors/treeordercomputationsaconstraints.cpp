/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Wednesday, March 28, 2018 - 11:45:51
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <modules/temporaltreemaps/processors/treeordercomputationsaconstraints.h>
#include <modules/temporaltreemaps/processors/treeordercomputationheuristic.h>

namespace inviwo
{
namespace kth
{

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TemporalTreeOrderComputationSAConstraints::processorInfo_
{
    "org.inviwo.TemporalTreeOrderComputationSAConstraints",      // Class identifier
    "Tree Order SA Constraints",                // Display name
    "Temporal Tree",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};

const ProcessorInfo TemporalTreeOrderComputationSAConstraints::getProcessorInfo() const
{
    return processorInfo_;
}

TemporalTreeOrderComputationSAConstraints::TemporalTreeOrderComputationSAConstraints()
    :TemporalTreeSimulatedAnnealing()
    , propResolveWithBest("resolveWithBest", "Resolve with Best")
{
	/* Settings */

    propSettings.addProperty(propResolveWithBest);

	/* Controls */

    propRestart.onChange([&]()
    {
        if (!initialized)
        {
            initializeResources();
        }
        else
        {
            restart();
        }
        updateOutput();
    });

    propSingleStep.onChange([&]()
    {
        if (!initialized) initializeResources();
        performanceTimer.Reset();
        if (!isConverged())
        {
            singleStep();
        }
        propTimeForLastAction.set(performanceTimer.ElapsedTimeAndReset());
        updateOutput();
    });

    runTimer.setCallback([this]()
    {
        if (!initialized) initializeResources();
		singleStep();
        // Stop timer and performance timer when we have converged
        if (isConverged())
        {
            propTimeForLastAction.set(performanceTimer.ElapsedTimeAndReset());
            runTimer.stop();
            propRunStepWise.setDisplayName("Run Stepwise");
        }
		updateOutput();
    });

    propRunUntilConvergence.onChange([&]()
    {
		if (!initialized || currentState.iteration != 0) initializeResources();
		restart();
        performanceTimer.Reset();
        runUntilConvergence();
        propTimeForLastAction.set(performanceTimer.ElapsedTimeAndReset());
		if (propSaveLog) {
			saveLog();
		}
		updateOutput();
    });

    logPrefix = "saConstraints";
}


void TemporalTreeOrderComputationSAConstraints::resolveConstraint(
    std::shared_ptr<const TemporalTree> tree, Constraint & constraint)
{
    size_t minOrder(currentState.order.size()); // numbere of leaves is maximum order
    size_t maxOrder(0); // 0 is minimum order

    // Seperate everything between minimum and maximum for constraint 
    // into conflicting and non-conflicting leaves
    TemporalTree::TTreeOrder conflictingLeaves;
    TemporalTree::TTreeOrder nonConflictingAndConstraintLeaves;

    TemporalTreeOrderComputationHeuristic::findConflictingLeaves(pInputTree, constraint, currentState.order, minOrder, maxOrder, conflictingLeaves, nonConflictingAndConstraintLeaves);

    TemporalTree::TTreeOrder temporaryOrder;
    if (propResolveWithBest)
    {
		std::vector<size_t> bestIds;
        double bestValue = std::numeric_limits<double>::max();

        for (int numConflictBefore(int(conflictingLeaves.size())); numConflictBefore >= 0; numConflictBefore--)
        {

            TemporalTreeOrderComputationHeuristic::buildNewOrder(temporaryOrder, currentState.order, numConflictBefore, conflictingLeaves, nonConflictingAndConstraintLeaves, minOrder, maxOrder);

            double newValue = evaluateOrder(temporaryOrder);

			// The new value is the same as best
			if (std::abs(bestValue - newValue) < std::numeric_limits<double>::epsilon())
			{
				bestIds.emplace_back(numConflictBefore);
			}
			// The new value is better than the best so far
			else if (newValue < bestValue)
			{
				bestIds.clear();
				bestIds.emplace_back(numConflictBefore);
				bestValue = newValue;
			}
        }

		std::uniform_int_distribution<int> chooseSolution(0, static_cast<int>(bestIds.size() - 1));

		int solutionId = chooseSolution(randomGen);

		TemporalTreeOrderComputationHeuristic::buildNewOrder(temporaryOrder, currentState.order, bestIds[solutionId], conflictingLeaves, nonConflictingAndConstraintLeaves, minOrder, maxOrder);
    }
    else
    {
        // Chose the resolution randomly
        std::uniform_int_distribution<int> chooseResolution
        (0, static_cast<int>(conflictingLeaves.size()));

        // Find a constraint to resolve
        TemporalTreeOrderComputationHeuristic::buildNewOrder(temporaryOrder, currentState.order, chooseResolution(randomGen), conflictingLeaves, nonConflictingAndConstraintLeaves, minOrder, maxOrder);
    }

    // Update the current state
    currentState.order = temporaryOrder;

}

void TemporalTreeOrderComputationSAConstraints::neighborSolution()
{
    std::uniform_int_distribution<int> chooseConstraintToResolve
    (0, static_cast<int>(unfulfilledConstraints.size()) - 1);

    // Find a constraint to resolve
    size_t constraintId = size_t(chooseConstraintToResolve(randomGen));
    resolveConstraint(pInputTree, constraints[unfulfilledConstraints[constraintId]]);
}

void TemporalTreeOrderComputationSAConstraints::initializeResources()
{
    std::shared_ptr<const TemporalTree> pTreeIn = portInTree.getData();
    if (!pTreeIn) return;

    pInputTree = pTreeIn;

    /* Initialize merge/split constraints */
    TemporalTreeOrderOptimization::initializeResources();

    initialized = true;
    restart();
}

void TemporalTreeOrderComputationSAConstraints::restart()
{
	TemporalTreeSimulatedAnnealing::restart();

    lastState = currentState;
    bestState = currentState;

    // Fill unfulfilled constraints
    prepareNextStep();
}

void TemporalTreeOrderComputationSAConstraints::setLastToCurrent()
{
    lastState = currentState;
}

void TemporalTreeOrderComputationSAConstraints::setCurrentToLast()
{
    currentState = lastState;
}

void TemporalTreeOrderComputationSAConstraints::setBest()
{
    bestState = currentState;
}

void TemporalTreeOrderComputationSAConstraints::prepareNextStep()
{
    unfulfilledConstraints.clear();

    size_t constraintId(0);
    for (auto& constraint: constraints)
    {
        if (!constraint.fulfilled)
        {
            unfulfilledConstraints.push_back(constraintId);
        }
        constraintId++;
    }

}

    void TemporalTreeOrderComputationSAConstraints::process()
{
    // Nothing to do here: All buttons
}

} // namespace
} // namespace

