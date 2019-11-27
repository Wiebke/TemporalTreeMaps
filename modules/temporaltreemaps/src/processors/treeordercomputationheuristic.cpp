/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Friday, March 23, 2018 - 15:58:29
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <modules/temporaltreemaps/processors/treeordercomputationheuristic.h>
#include <modules/temporaltreemaps/processors/treeordercomputation.h>

namespace inviwo
{
namespace kth
{

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TemporalTreeOrderComputationHeuristic::processorInfo_
{
    "org.inviwo.TemporalTreeOrderComputationHeuristic", // Class identifier
    "Tree Order Heuristic",         // Display name
    "Temporal Tree",          // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};

const ProcessorInfo TemporalTreeOrderComputationHeuristic::getProcessorInfo() const
{
    return processorInfo_;
}

TemporalTreeOrderComputationHeuristic::TemporalTreeOrderComputationHeuristic()
    :TemporalTreeOrderOptimization()
    // Settings
    , propInitialConstraintOrder("initialConstraintOrder", "Initial Constraints Order")

{
    /* Settings */
    propSettings.addProperty(propInitialConstraintOrder);
	propInitialConstraintOrder.addOption("asIs", "As is", 0);
	propInitialConstraintOrder.addOption("random", "Random", 1);
    propInitialConstraintOrder.addOption("higherLevelLargerSize", "Higher Level, larger size", 2);
    propInitialConstraintOrder.addOption("lowerLevelSmallerSize", "Lower Level, smaller size", 3);
    propInitialConstraintOrder.onChange([&]() { restart(); });

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

    logPrefix = "heuristic";
}

void TemporalTreeOrderComputationHeuristic::findConflictingLeaves(std::shared_ptr<const TemporalTree>& tree, const Constraint& constraint, const TemporalTree::TTreeOrder& order, size_t& minOrder, size_t& maxOrder,
    TemporalTree::TTreeOrder & conflictingLeaves, TemporalTree::TTreeOrder & nonConflictingAndConstraintLeaves)
{

    conflictingLeaves.clear();
    nonConflictingAndConstraintLeaves.clear();

    // Seperate everything between minimum and maximum for constraint 
    // into conflicting and non-conflicting leaves

    bool foundFirstLeaf = false;

    size_t foundConstraintLeaves = 0;

    for (size_t r(0); r<order.size(); r++)
    {
        size_t leaf = order[r];

        if (constraint.leaves.find(order[r]) != constraint.leaves.end())
        {
            foundConstraintLeaves++;
            if (!foundFirstLeaf)
            {
                minOrder = r;
                foundFirstLeaf = true;
            }
            // First leaf has been found
            if (foundFirstLeaf && foundConstraintLeaves <= constraint.leaves.size())
            {
                nonConflictingAndConstraintLeaves.push_back(leaf);
            }
            if (foundConstraintLeaves == constraint.leaves.size())
            {
                // Found the last constraint leaf, we are done
                maxOrder = r;
                break;
            }

        }
        else
        {
            if (foundFirstLeaf)
            {
                if (!isOverlappingWithConstraint(tree->nodes[order[r]], constraint))
                {
                    nonConflictingAndConstraintLeaves.push_back(leaf);
                }
                else
                {
                    conflictingLeaves.push_back(leaf);
                }

            }
        }
    }
}

void TemporalTreeOrderComputationHeuristic::buildNewOrder(TemporalTree::TTreeOrder& newOrder, const TemporalTree::TTreeOrder& order, 
    const size_t numConflictBefore, const TemporalTree::TTreeOrder & conflictingLeaves, const TemporalTree::TTreeOrder & nonConflictingAndConstraintLeaves, const size_t minOrder, const size_t maxOrder)
{
    newOrder.clear();
    newOrder.reserve(order.size());
    // Insert all leaves occuring before the first constraint leaf, 
    // begin + minOrder is the first to exclude
    newOrder.insert(newOrder.end(), order.begin(), order.begin() + minOrder);

    // Insert as many before as specified
    newOrder.insert(newOrder.end(), conflictingLeaves.begin(), conflictingLeaves.begin() + numConflictBefore);

    // Insert all nonconflicting and constraint nodes
    newOrder.insert(newOrder.end(), nonConflictingAndConstraintLeaves.begin(), nonConflictingAndConstraintLeaves.end());

    // Insert all conflighting leaves that are left now
    newOrder.insert(newOrder.end(), conflictingLeaves.begin() + numConflictBefore, conflictingLeaves.end());

    // Insert all leaves occuring after the last constraint leaf, 
    // begin + maxOrder is the last to exclude
    newOrder.insert(newOrder.end(), order.begin() + (maxOrder + 1), order.end());
}

bool TemporalTreeOrderComputationHeuristic::resolveConstraint(std::shared_ptr<const TemporalTree> tree, 
    Constraint& constraint)
{
    if (constraint.fulfilled) return false;

    size_t minOrder(currentState.order.size()); // numbere of leaves is maximum order
    size_t maxOrder(0); // 0 is minimum order

    // Seperate everything between minimum and maximum for constraint 
    // into conflicting and non-conflicting leaves
    TemporalTree::TTreeOrder conflictingLeaves;
    TemporalTree::TTreeOrder nonConflictingAndConstraintLeaves;

    findConflictingLeaves(pInputTree, constraint, currentState.order, minOrder, maxOrder, conflictingLeaves, nonConflictingAndConstraintLeaves);

    TemporalTree::TTreeOrder temporaryOrder;

	std::vector<size_t> bestIds;
    double bestValue = std::numeric_limits<double>::max();

    for (int numConflictBefore(int(conflictingLeaves.size())); numConflictBefore >= 0; numConflictBefore--)
    {
        buildNewOrder(temporaryOrder, currentState.order, numConflictBefore, conflictingLeaves, nonConflictingAndConstraintLeaves, minOrder, maxOrder);
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

	buildNewOrder(temporaryOrder, currentState.order, bestIds[solutionId], conflictingLeaves, nonConflictingAndConstraintLeaves, minOrder, maxOrder);
    currentState.order = temporaryOrder;
    currentOrderMap.clear();
    treeorder::toOrderMap(currentOrderMap, currentState.order);
    currentState.statistic.clear();
    currentState.value = evaluateOrder(currentState.order, &currentState.statistic);
    
	//Add the constraints that are newly broken to the priority queue  
	prepareNextStep();

    return true;
}

void TemporalTreeOrderComputationHeuristic::initializeResources()
{
    TemporalTreeOrderOptimization::initializeResources();

    initialized = true;
    restart();
}

void TemporalTreeOrderComputationHeuristic::restart()
{
	TemporalTreeOrderOptimization::restart();

	currentOrderMap.clear();
	treeorder::toOrderMap(currentOrderMap, currentState.order);

	constraintOrder = std::vector<size_t>(constraints.size());
	// Fill the order with the index
	std::iota(constraintOrder.begin(), constraintOrder.end(), 0);

	switch (propInitialConstraintOrder){
	case 1:
		// Shuffle according to optimization random generator
		std::shuffle(constraintOrder.begin(), constraintOrder.end(), randomGen);
		break;
	case 2:
		// Sort according to higher level, constraint type, then larger size
		std::sort(constraintOrder.begin(), constraintOrder.end(),
			[&](const size_t a, const size_t b) -> bool
		{
			size_t sizeA = constraints[a].leaves.size();
			size_t sizeB = constraints[b].leaves.size();
			size_t levelA = constraints[a].level;
			size_t levelB = constraints[b].level;
			if (levelA == levelB) {
				if (constraints[a].type == Hierarchy && constraints[b].type != Hierarchy) return true;
				else if (constraints[a].type != Hierarchy && constraints[b].type == Hierarchy) return false;
				else return sizeA > sizeB;
			}
			else {
				return levelA > levelB;
			}
		});
		break;
	case 3:
		// Sort according to higher level, constraint type, then larger size
		std::sort(constraintOrder.begin(), constraintOrder.end(),
			[&](const size_t a, const size_t b) -> bool
		{
			size_t sizeA = constraints[a].leaves.size();
			size_t sizeB = constraints[b].leaves.size();
			size_t levelA = constraints[a].level;
			size_t levelB = constraints[b].level;
			if (levelA == levelB) {
				if (constraints[a].type == Hierarchy && constraints[b].type != Hierarchy) return true;
				else if (constraints[a].type != Hierarchy && constraints[b].type == Hierarchy) return false;
				else return sizeA < sizeB;
			}
			else {
				return levelA < levelB;
			}
		});
		break;
	default:
		// Do nothing
		break;
	}

    constraintsQueue = std::queue<size_t>();

	prepareNextStep();

    bestState = currentState;

    logStep();
}

bool TemporalTreeOrderComputationHeuristic::isConverged()
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
    if (constraintsQueue.empty())
    {
        LogProcessorInfo("Converged by no more constraints to fulfill");
        return true;
    }
    return false;
}

void TemporalTreeOrderComputationHeuristic::singleStep()
{   
	if (isConverged()) 
	{
		return;
	}

    // Go through the queue until we have reached the maximum iterations or found a constraint to resolve
	// There might be constraint in the queue that are already fulfilled, an interation fulfills another constraint
    while (!isConverged() && !resolveConstraint(portInTree.getData(), constraints[constraintsQueue.front()]))
    {
        constraintsQueue.pop();
    }
    if (currentState.value < bestState.value)
    {
        bestState = currentState;
    }
    currentState.iteration++;
    logStep();

}

void TemporalTreeOrderComputationHeuristic::runUntilConvergence()
{
    while (!isConverged())
    {
        if (resolveConstraint(portInTree.getData(), constraints[constraintsQueue.front()]))
        {
            currentState.iteration++;
            logStep();
            if (currentState.value < bestState.value)
            {
                propTimeUntilBest.set(performanceTimer.ElapsedTime());
                bestState = currentState;
            }
        }
        constraintsQueue.pop();
    }
}

void TemporalTreeOrderComputationHeuristic::prepareNextStep()
{
	// Add all unfulfilled constraints to a priority queue (first in, first out)
	for (size_t constraintId : constraintOrder)
	{
		Constraint& constraint = constraints[constraintId];
		if (!constraint.fulfilled)
		{
			constraintsQueue.push(constraintId);
		}
	}
}

void TemporalTreeOrderComputationHeuristic::logStep()
{
    TemporalTreeOrderOptimization::logStep();
}

void TemporalTreeOrderComputationHeuristic::initializeLog()
{
    TemporalTreeOrderOptimization::initializeLog();
}

void TemporalTreeOrderComputationHeuristic::logProperties()
{
    const std::vector<std::string> colHeaders{
		propSeedOrder.getDisplayName(),
		propSeedOptimization.getDisplayName(),
        propIterationsMax.getDisplayName(),
        propInitialConstraintOrder.getDisplayName(),
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
        std::to_string(propInitialConstraintOrder.get()),
        std::to_string(propWeightByTypeOnly),
        std::to_string(propWeightTypeOnly),
        std::to_string(bestState.iteration),
        std::to_string(bestState.value),
        std::to_string(propTimeUntilBest),
        std::to_string(propTimeForLastAction) };

    optimizationSettings = createDataFrame({ exampleRow }, colHeaders);
    optimizationSettings->addRow(exampleRow);
}

void TemporalTreeOrderComputationHeuristic::process()
{
    // Everthing is triggered by button press anyways
}

} // namespace
} // namespace

