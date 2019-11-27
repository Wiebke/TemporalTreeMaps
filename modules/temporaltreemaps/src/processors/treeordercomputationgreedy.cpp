/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Thursday, March 29, 2018 - 15:00:03
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <modules/temporaltreemaps/processors/treeordercomputationgreedy.h>
#include <modules/temporaltreemaps/processors/treeordercomputationheuristic.h>

namespace inviwo
{
namespace kth
{

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TemporalTreeOrderComputationGreedy::processorInfo_
{
    "org.inviwo.TemporalTreeOrderComputationGreedy",      // Class identifier
    "Tree Order Computation Greedy",                // Display name
    "Temporal Tree",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};

const ProcessorInfo TemporalTreeOrderComputationGreedy::getProcessorInfo() const
{
    return processorInfo_;
}

TemporalTreeOrderComputationGreedy::TemporalTreeOrderComputationGreedy() :TemporalTreeOrderOptimization()
{

	/* Constrols */

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
        singleStep();
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

    logPrefix = "greedy";
}

void TemporalTreeOrderComputationGreedy::resolveConstraint()
{
	// best solution Ids, constraint first, 
    std::vector<std::pair<size_t, size_t>> bestIds;
	double bestValue = std::numeric_limits<double>::max();

    for (auto& constraintId : unfulfilledConstraints)
    {
        Constraint& constraint = constraints[constraintId];

        size_t minOrder;
        size_t maxOrder;
        TemporalTree::TTreeOrder conflictingLeaves;
        TemporalTree::TTreeOrder nonConflictingAndConstraintLeaves;
        TemporalTreeOrderComputationHeuristic::findConflictingLeaves(pInputTree, constraint, currentState.order, minOrder, maxOrder, conflictingLeaves, nonConflictingAndConstraintLeaves);

        TemporalTree::TTreeOrder temporaryOrder;
        ConstraintsStatistic temporaryStatistics;

        for (int numConflictBefore(int(conflictingLeaves.size())); numConflictBefore >= 0; numConflictBefore--)
        {
            TemporalTreeOrderComputationHeuristic::buildNewOrder(temporaryOrder, currentState.order, numConflictBefore, conflictingLeaves, nonConflictingAndConstraintLeaves, minOrder, maxOrder);
            double newValue = evaluateOrder(temporaryOrder, &temporaryStatistics);

			// The new value is the same as best
			if (std::abs(bestValue - newValue) < std::numeric_limits<double>::epsilon())
			{
				bestIds.emplace_back(constraintId, numConflictBefore);
			}
			// The new value is better than the best so far
			else if (newValue < bestValue)
			{
				bestIds.clear();
				bestIds.emplace_back(constraintId, numConflictBefore);
				bestValue = newValue;
			}
		}
    }

    std::uniform_int_distribution<int> chooseSolution (0, static_cast<int>(bestIds.size()-1));

    size_t solutionId = chooseSolution(randomGen);

	std::pair<size_t, size_t> solution = bestIds[solutionId];

    size_t minOrder;
    size_t maxOrder;
    TemporalTree::TTreeOrder conflictingLeaves;
    TemporalTree::TTreeOrder nonConflictingAndConstraintLeaves;
    TemporalTreeOrderComputationHeuristic::findConflictingLeaves(pInputTree, constraints[solution.first], currentState.order, minOrder, maxOrder, conflictingLeaves, nonConflictingAndConstraintLeaves);

    TemporalTree::TTreeOrder temporaryOrder;
    TemporalTreeOrderComputationHeuristic::buildNewOrder(temporaryOrder, currentState.order, solution.second, conflictingLeaves, nonConflictingAndConstraintLeaves, minOrder, maxOrder);

    currentState.order = temporaryOrder;
    currentState.statistic.clear();
    currentState.value = evaluateOrder(currentState.order, &currentState.statistic);
}

void TemporalTreeOrderComputationGreedy::initializeResources()
{
    TemporalTreeOrderOptimization::initializeResources();

    initialized = true;
    restart();
}

void TemporalTreeOrderComputationGreedy::restart()
{
    TemporalTreeOrderOptimization::restart();

    prepareNextStep();

    bestState = currentState;
    
	logStep();

}

bool TemporalTreeOrderComputationGreedy::isConverged()
{
    // the iteration number is an index starting at 0, the max is a number >= -1
    if (currentState.iteration > propIterationsMax - 1)
    {
        LogProcessorInfo("Converged by reaching maximum number of iterations.");
        return true;
    }
    if (std::abs(currentState.value) < std::numeric_limits<float>::epsilon())
    {
        LogProcessorInfo("Converged by reaching a global optimum.");
        return true;
    }
    if (unfulfilledConstraints.size() == 0)
    {
        LogProcessorInfo("Converged by no more constraints to fulfill");
        return true;
    }
    return false;
}

void TemporalTreeOrderComputationGreedy::singleStep()
{
    // Go through the queue until we have reached the maximum iterations or found a constraint to resolve
    if (!isConverged())
    {
        resolveConstraint();
        currentState.iteration++;
        logStep();
        if (currentState.value < bestState.value)
        {
            bestState = currentState;
        }
        prepareNextStep();
    }

}

void TemporalTreeOrderComputationGreedy::runUntilConvergence()
{
    while (!isConverged())
    {
        resolveConstraint();
        currentState.iteration++;
        logStep();
        if (currentState.value < bestState.value)
        {
            propTimeUntilBest.set(performanceTimer.ElapsedTime());
            bestState = currentState;
        }
        prepareNextStep();
    }
}

void TemporalTreeOrderComputationGreedy::prepareNextStep()
{
    unfulfilledConstraints.clear();

    size_t constraintId(0);
    for (auto& constraint : constraints)
    {
        if (!constraint.fulfilled)
        {
            unfulfilledConstraints.push_back(constraintId);
        }
        constraintId++;
    }
}

void TemporalTreeOrderComputationGreedy::logStep()
{
    TemporalTreeOrderOptimization::logStep();
}

void TemporalTreeOrderComputationGreedy::initializeLog()
{
    TemporalTreeOrderOptimization::initializeLog();
}

void TemporalTreeOrderComputationGreedy::logProperties()
{
    const std::vector<std::string> colHeaders{
		propSeedOrder.getDisplayName(),
		propSeedOptimization.getDisplayName(),
        propIterationsMax.getDisplayName(),
        propWeightByTypeOnly.getDisplayName(),
        propWeightTypeOnly.getDisplayName(),
        propBestIteration.getDisplayName(),
        propObjectiveValue.getDisplayName(),
        propTimeUntilBest.getDisplayName(),
        propTimeForLastAction.getDisplayName() };

    const std::vector<std::string> exampleRow{
		std::to_string(propSeedOrder),
		std::to_string(propSeedOptimization),
        std::to_string(propIterationsMax),
        std::to_string(propWeightByTypeOnly),
        std::to_string(propWeightTypeOnly),
        std::to_string(bestState.iteration),
        std::to_string(bestState.value),
        std::to_string(propTimeUntilBest),
        std::to_string(propTimeForLastAction) };

    optimizationSettings = createDataFrame({ exampleRow }, colHeaders);
    optimizationSettings->addRow(exampleRow);
}


void TemporalTreeOrderComputationGreedy::process()
{
    // Do nothing 
}

} // namespace
} // namespace

