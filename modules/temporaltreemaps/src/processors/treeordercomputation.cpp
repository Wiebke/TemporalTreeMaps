/*********************************************************************
*  Author  : Tino Weinkauf and Wiebke Koepp
*  Init    : Friday, March 23, 2018 - 15:58:29
*
*  Project : KTH Inviwo Modules
*
*  License : Follows the Inviwo BSD license model
*********************************************************************
*/

#include <inviwo/core/util/utilities.h>
#include <modules/temporaltreemaps/processors/treeordercomputation.h>
#include <random>

namespace inviwo
{
namespace kth
{

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TemporalTreeOrderOptimization::processorInfo_
{
    "org.inviwo.TemporalTreeOrderOptimization", // Class identifier
    "Tree Order Optimization",         // Display name
    "Temporal Tree",          // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};

const ProcessorInfo TemporalTreeOrderOptimization::getProcessorInfo() const
{
    return processorInfo_;
}

TemporalTreeOrderOptimization::TemporalTreeOrderOptimization()
    :Processor()
    // Input/Output
    , portInTree("inTree")
    , portOutTree("outTree")
    , portOutLogOptimization("outLogOptimization")
    , portOutLogSettings("outLogSettings")
    // Settings
    , propSettings("settings", "Optimization Settings")
    , propIterationsMax("iterationsMax", "Max Iters", 1000, 10, 1000000000, 1)
    , propObjectiveFunction("objectiveFunction", "Objective Function")
    , propWeightByTypeOnly("weightByTypeOnly", "Weight By Type Only", true)
    , propWeightTypeOnly("weightTypeOnly", "Type Only", 0.5, 0.01, 1.0, 0.01)
    , propWeightByType("weightByType", "Weight By Type", false)
    , propWeightMergeSplit("weightMergeSplit", "Merge/Split", 1, 0.l, 10,0)
    , propWeightHierarchy("weightHierarchy", "Hierarchy", 1, 0.l, 10.0)
	, propWeightBySize("weightBySize", "Weight By Size", false)
	, propWeightSize("weightSize", "Size", 1, 0.l, 10.0)
    , propWeightByLevel("weightByLevel", "Weight By Level", false)
    , propWeightLevel("weightLevel", "Level", 1.0, 0.l, 10.0)
	, propInitialOrder("initialOrder", "Initial Order")
    , propUseInputOrder("useInputOrder", "Use input order", false)
    , propRandomizeOrder("randomizeInitial", "Randomize initial order", false)
    , propRandomOrSeedOrder("randomOrSeedOrder", "Random Seed for Order", true)
    , propSeedOrder("seedOrder", "Seed for Order", 0, 0, RAND_MAX + 1, 1)
    , propRandomnessOptimization("randomnessOptimization", "Randomness")
    , propRandomOrSeedOptimization("randomOrSeedOptimization", "Random Seed for Optimization", true)
    , propSeedOptimization("seedOptimization", "Seed for Optimization", 0, 0, RAND_MAX + 1, 1)
    // Buttons for starting/stopping/resetting
    , propControls("controls", "Controls")
    , propRestart("restart", "Initialize/Restart")
    , propSingleStep("SingleStep", "Single Step")
    , propRunStepWise("runStepwise", "Run Stepwise")
    , propRunUntilConvergence("runConvergence", "Run until Convergence")
    , runTimer(std::chrono::milliseconds{ 10 }, []() {})
    // Current State
    , propCurrentState("currentState", "Current State")
    , propCurrentIteration("currentIteration", "Current Iter", 0, 0, 1000000000, 1)
    , propBestIteration("bestIteration", "Best Iter", 0, 0, 1000000000, 1)
    , propOutputBestOrder("outputBestOrder", "Output Best Order", false)
    , propFulfilledConstraintsTotal("fulfilledConstraints",
        "Total", "")
    , propStatisticsMergeSplit("statisticsMergeSplit", "Merge/Split", "")
    , propStatisticsHierarchy("statisticsHierarchy", "Hierarchy", "")
    , propObjectiveValue("objectiveValue", "Value", 0.f, 0.f, 10000.f, 0.1f)
    , propTimeForLastAction("timeForLastAction", "Time for Last Action", 0.f, 0.f, 3600.f, 0.001f)
    , propTimeUntilBest("timeuntilBest", "Time until Best", 0.f, 0.f, 3600.f, 0.001f)
    /// Save
    , propLog("log", "Log")
    , propSaveLog("saveLog", "Save Log", false)
    , propLogDirectory("logDirectory", "Log Directory")
    , propLogSettingsFile("settingsFile", "Log File Settings")
    , propLogOptimizationFile("optimizationFile", "Log File Optimization")
{
    // Ports
    addPort(portInTree);
    portInTree.onChange([&]()
    {
        initializeResources();
        updateOutput();
    });

    addPort(portOutLogSettings);
    addPort(portOutTree);
    addPort(portOutLogOptimization);

    initialized = false;

    /* Settings */

    addProperty(propSettings);

    propSettings.addProperty(propIterationsMax);
    propIterationsMax.onChange([&]() { restart(); });
    propIterationsMax.setSemantics(PropertySemantics::Text);

    propSettings.addProperty(propInitialOrder);

	propInitialOrder.addProperty(propUseInputOrder);
    propUseInputOrder.onChange([&]()
    {
        if (!initialized)
        {
            initializeResources();
        }
        else
        {
            restart();
        }
    });

	propInitialOrder.addProperty(propRandomizeOrder);
	propRandomizeOrder.onChange([&]()
	{
		if (!initialized)
		{
			initializeResources();
		}
		else
		{
			restart();
		}
		if (propRandomizeOrder) {
			util::show(propSeedOrder, propRandomOrSeedOrder);
		}
		else {
			util::hide(propSeedOrder, propRandomOrSeedOrder);
		}
	});

	propInitialOrder.addProperty(propRandomOrSeedOrder);
	propRandomOrSeedOrder.onChange([&]()
	{
		propSeedOrder.setReadOnly(propRandomOrSeedOrder.get());
	});

	propInitialOrder.addProperty(propSeedOrder);
	propSeedOrder.setSemantics(PropertySemantics::Text);
	propSeedOrder.onChange([&]() { if (!propRandomOrSeedOrder.get()) restart(); });

	util::hide(propSeedOrder, propRandomOrSeedOrder);

	propSettings.addProperty(propRandomnessOptimization);

	propRandomnessOptimization.addProperty(propSeedOptimization);
	propSeedOptimization.setSemantics(PropertySemantics::Text);

	propRandomnessOptimization.addProperty(propRandomOrSeedOptimization);
	propRandomOrSeedOptimization.onChange([&]()
	{
		propSeedOptimization.setReadOnly(propRandomOrSeedOptimization.get());
	});
	propSeedOptimization.onChange([&]() { if (!propRandomOrSeedOptimization.get()) restart(); });
	
    propSettings.addProperty(propObjectiveFunction);

    propObjectiveFunction.addProperty(propWeightByType);
    propObjectiveFunction.addProperty(propWeightMergeSplit);
    propObjectiveFunction.addProperty(propWeightHierarchy);
    propWeightByType.onChange([&]()
    {
        if (propWeightByType)
        {
            util::show(propWeightMergeSplit, propWeightHierarchy);
        }
        else
        {
            util::hide(propWeightMergeSplit, propWeightHierarchy);
        }
    });

    propObjectiveFunction.addProperty(propWeightByTypeOnly);
    propObjectiveFunction.addProperty(propWeightTypeOnly);
    propWeightByTypeOnly.onChange([&]()
    {
        if (propWeightByTypeOnly)
        {
            util::show(propWeightTypeOnly);
            util::hide(propWeightBySize, propWeightByType, propWeightByLevel);
        }
        else
        {
            util::hide(propWeightTypeOnly);
            util::show(propWeightBySize, propWeightByType, propWeightByLevel);
        }
    });

    propObjectiveFunction.addProperty(propWeightBySize);
    propObjectiveFunction.addProperty(propWeightSize);
    propWeightBySize.onChange([&]()
    {
        if (propWeightBySize)
        {
            util::show(propWeightSize);
        }
        else
        {
            util::hide(propWeightSize);
        }
    });

    propObjectiveFunction.addProperty(propWeightByLevel);
    propObjectiveFunction.addProperty(propWeightLevel);
    propWeightByLevel.onChange([&]()
    {
        if (propWeightByLevel)
        {
            util::show(propWeightLevel);
        }
        else
        {
            util::hide(propWeightLevel);
        }
    });

    util::hide(propWeightSize, propWeightLevel, propWeightMergeSplit, propWeightHierarchy);
    util::hide(propWeightBySize, propWeightByType, propWeightByLevel);

    /* Controls */

    addProperty(propControls);

    propControls.addProperty(propRestart);
    propControls.addProperty(propSingleStep);
    propControls.addProperty(propRunStepWise);
    propControls.addProperty(propRunUntilConvergence);

    propRunStepWise.onChange([&]()
    {
        if (runTimer.isRunning())
        {
            propTimeForLastAction.set(performanceTimer.ElapsedTimeAndReset());
            runTimer.stop();
            propRunStepWise.setDisplayName("Run Stepwise");
        }
        else
        {
            performanceTimer.Reset();
            runTimer.start();
            propRunStepWise.setDisplayName("Stop");
        }
    });

    /* Current State */

    addProperty(propCurrentState);

    propCurrentState.addProperty(propCurrentIteration);
    propCurrentIteration.setReadOnly(true);
    propCurrentIteration.setSemantics(PropertySemantics::Text);

    propCurrentState.addProperty(propBestIteration);
    propBestIteration.setReadOnly(true);
    propBestIteration.setSemantics(PropertySemantics::Text);

    propCurrentState.addProperty(propOutputBestOrder);
    propOutputBestOrder.onChange([&]() { updateOutput(); });

    propCurrentState.addProperty(propFulfilledConstraintsTotal);
    propFulfilledConstraintsTotal.setReadOnly(true);

    propCurrentState.addProperty(propObjectiveValue);
    propObjectiveValue.setReadOnly(true);
    propObjectiveValue.setSemantics(PropertySemantics::Text);

    propCurrentState.addProperty(propStatisticsMergeSplit);
    propStatisticsMergeSplit.setReadOnly(true);

    propCurrentState.addProperty(propStatisticsHierarchy);
    propStatisticsHierarchy.setReadOnly(true);

    propCurrentState.addProperty(propTimeForLastAction);
    propTimeForLastAction.setReadOnly(true);
    propTimeForLastAction.setSemantics(PropertySemantics::Text);

    propCurrentState.addProperty(propTimeUntilBest);
    propTimeUntilBest.setReadOnly(true);
    propTimeUntilBest.setSemantics(PropertySemantics::Text);

    /* Log */

    addProperty(propLog);

    propLog.addProperty(propSaveLog);
    propLog.addProperty(propLogDirectory);

    propLog.addProperty(propLogOptimizationFile);
    propLogOptimizationFile.setReadOnly(true);
    propLog.addProperty(propLogSettingsFile);
    propLogSettingsFile.setReadOnly(true);
}

void TemporalTreeOrderOptimization::fillStatistics(const ConstraintsStatistic& statistic)
{
    const size_t numFulfilled = statistic.numFulfilledHierarchyConstraints() + statistic.numFulFilledMergeSplitConstraints();

    propFulfilledConstraintsTotal.set(std::to_string(numFulfilled) + " / " + std::to_string(constraints.size()));

    std::string mergeSplit;

    for (size_t level = 1; level < numByLevelMergeSplit.size(); level++)
    {
        if (level + 1 <= statistic.fulfilledByLevelMergeSplit.size())
        {
            mergeSplit += std::to_string(statistic.fulfilledByLevelMergeSplit[level]) 
            + "/" + std::to_string(numByLevelMergeSplit[level]);
        }
        else
        {
            mergeSplit += "0/" + std::to_string(numByLevelMergeSplit[level]);
        }

        if (level < numByLevelMergeSplit.size() -1)
        {
            mergeSplit += " - ";
        }
    }

    std::string hierarchy;

    for (size_t level = 1; level < numByLevelHierarchy.size(); level++)
    {
        if (level + 1 <= statistic.fulfilledByLevelHierarchy.size())
        {
            hierarchy += std::to_string(statistic.fulfilledByLevelHierarchy[level]) + "/" + std::to_string(numByLevelHierarchy[level]);
        }
        else
        {
            hierarchy += "0/" + std::to_string(numByLevelHierarchy[level]);
        }

        if (level < numByLevelHierarchy.size() - 1)
        {
            hierarchy += " - ";
        }
    }

    propStatisticsMergeSplit.set(mergeSplit);
    propStatisticsHierarchy.set(hierarchy);

}

void TemporalTreeOrderOptimization::setInitialOrder()
{

    currentState.order.clear();

    if (propUseInputOrder)
    {
		// We can use the input order only when it fits with the tree        
		if (treeorder::fitsWithTree(*pInputTree, pInputTree->order)) 
		{
			currentState.order = pInputTree->order;
		} 
		else {
			LogProcessorWarn("Inconsistent order given in the tree, falling back to depth-first ordering.");
			treeorder::orderAsDepthFirst(currentState.order, *pInputTree, pInputTree->edgesHierarchy);
		}
    }
    else
    {
        treeorder::orderAsDepthFirst(currentState.order, *pInputTree, pInputTree->edgesHierarchy);
    }

    if (propRandomizeOrder)
    {
		if (propUseInputOrder) {
			LogProcessorWarn("Using the input order takes precedence over order randomization. Order is not randomized.")
		}
		else {
			std::mt19937 randomGenOrder;

			//Generate a random seed if so desired and set it
			if (propRandomOrSeedOrder.get())
			{
				propSeedOrder.set(rand());
			}
			randomGenOrder.seed(static_cast<std::mt19937::result_type>(propSeedOrder));
			std::shuffle(currentState.order.begin(), currentState.order.end(), randomGenOrder);
		}	
    }

}

void TemporalTreeOrderOptimization::initializeResources()
{
    // Get tree
    std::shared_ptr<const TemporalTree> pTreeIn = portInTree.getData();
    if (!pTreeIn) return;

    // Copy tree to compute reverse edges
    auto pCopyTree = std::make_shared<TemporalTree>(TemporalTree(*pTreeIn));
    pCopyTree->computeReverseEdges();
    pInputTree = std::const_pointer_cast<const TemporalTree>(pCopyTree);

    constraints.clear();

    // Extract constraints from the tree
    numByLevelMergeSplit.clear();
    extractMergeSplitConstraints(pInputTree, constraints, numByLevelMergeSplit);
    numConstraintsMergeSplit = constraints.size();
    numByLevelHierarchy.clear();
    extractHierarchyConstraints(pInputTree, constraints, numByLevelHierarchy);
    numConstraintsHierarchy = constraints.size() - numConstraintsMergeSplit;
    
    maxConstraintSize = 0;
    maxConstraintLevel = 0;
    for (auto& constraint : constraints)
    {
        if (constraint.leaves.size() > maxConstraintSize) maxConstraintSize = constraint.leaves.size();
        if (constraint.level > maxConstraintLevel) maxConstraintLevel = constraint.level;
    }
    
}

double TemporalTreeOrderOptimization::weighUnfulfilledConstraint(Constraint& constraint)
{
    double constraintValue = 1.0;

    if (propWeightByTypeOnly)
    {
        constraintValue *= constraint.type == Hierarchy ? (propWeightTypeOnly) / numConstraintsHierarchy :
        (1.0 - propWeightTypeOnly) / numConstraintsMergeSplit;
    } else
    {
        if (propWeightByType)
        {
            constraintValue *= constraint.type == Hierarchy ? propWeightHierarchy : propWeightMergeSplit;
        }
        if (propWeightByLevel)
        {
            constraintValue *= propWeightLevel * (maxConstraintLevel - (constraint.level - 1)) / maxConstraintLevel;
        }
        if (propWeightBySize)
        {
            constraintValue *= propWeightSize * constraint.leaves.size() / maxConstraintSize;
        }
        if (!propWeightByType && !propWeightBySize && !propWeightByLevel)
        {
            constraintValue *= 1.0 / (numConstraintsMergeSplit + numConstraintsHierarchy);
        }
    }
    
    return constraintValue;
}


double TemporalTreeOrderOptimization::evaluateOrder(const TemporalTree::TTreeOrder& order, ConstraintsStatistic* statistic)
{
    double value = 0.0;

	// If a ConstraintsStatistic is given
    if (statistic)
    {
        (*statistic).clear();
    }

    TemporalTree::TTreeOrderMap orderMap;
    treeorder::toOrderMap(orderMap, order);

    for (auto& constraint : constraints)
    {
        if (!isFulFilled(constraint, pInputTree, order, orderMap))
        {
            value += weighUnfulfilledConstraint(constraint);
        }
        // If a ConstraintsStatistic is given
        if (statistic)
        {
            (*statistic).update(constraint);
        }

    }

    return value;
}

double TemporalTreeOrderOptimization::evaluateOrder(const TemporalTree::TTreeOrder& order)
{
    return evaluateOrder(order, nullptr);
}

void TemporalTreeOrderOptimization::restart()
{
	// Get tree
	std::shared_ptr<const TemporalTree> pTreeIn = portInTree.getData();
	if (!pTreeIn) return;

	if (!initialized || !pInputTree)
	{
		initializeResources();
	}

	//Generate a random seed if so desired and set it
	if (propRandomOrSeedOptimization.get())
	{
		propSeedOptimization.set(rand());
	}
	randomGen.seed(static_cast<std::mt19937::result_type>(propSeedOptimization));

	currentState.iteration = 0;
	currentState.statistic.clear();
	setInitialOrder();
	currentState.value = evaluateOrder(currentState.order, &currentState.statistic);

	setFileNames();
}

void TemporalTreeOrderOptimization::updateOutput()
{
    std::shared_ptr<const TemporalTree> pTreeIn = portInTree.getData();
    if (!pTreeIn) return;

    // Copy tree to output things
    std::shared_ptr<TemporalTree> pTreeOut =
        std::make_shared<TemporalTree>(TemporalTree(*pTreeIn));

    propCurrentIteration.set(int(currentState.iteration) - 1);
    propBestIteration.set(int(bestState.iteration) - 1);

    // Get statistics of how many constraints are fulfilled
    fillStatistics(propOutputBestOrder ? bestState.statistic : currentState.statistic);
    propObjectiveValue.set(propOutputBestOrder ? float(bestState.value) : float(currentState.value));

    // Set the order
    pTreeOut->order = propOutputBestOrder ? bestState.order : currentState.order;

    // Set data
    portOutTree.setData(pTreeOut);
}

void TemporalTreeOrderOptimization::saveLog()
{
	OptimizationState currentStateBackup = currentState;
	logProperties();
	// Log the best state in the very last row
    currentState = bestState;
    logStep();
	currentState = currentStateBackup;
    portOutLogOptimization.setData(optimizationStatistics);
    portOutLogSettings.setData(optimizationSettings);
}

void TemporalTreeOrderOptimization::logStep()
{
    if (!optimizationStatistics) return;

    bool firstRow = optimizationStatistics->getNumberOfRows() == 0;

    size_t numUnfulfilledMergeSplit = numConstraintsMergeSplit - currentState.statistic.numFulFilledMergeSplitConstraints();
    size_t numUnfulfilledHierarchy = numConstraintsHierarchy - currentState.statistic.numFulfilledHierarchyConstraints();

    std::vector<std::string> newRow{
        std::to_string(int(currentState.iteration) - 1), //"Iteration"
        std::to_string(currentState.value), //"Current Value",
        std::to_string(firstRow ? numConstraintsMergeSplit : numUnfulfilledMergeSplit),
        std::to_string(firstRow ? numConstraintsHierarchy : numUnfulfilledHierarchy),
        std::to_string(firstRow ? numConstraintsHierarchy + numConstraintsMergeSplit : numUnfulfilledMergeSplit + numUnfulfilledHierarchy),
        std::to_string(currentState.statistic.unhappyLeaves.size())
    };

    optimizationStatistics->addRow(newRow);
}

void TemporalTreeOrderOptimization::initializeLog()
{
    const std::vector<std::string> colHeaders{
        "Iteration",
        "Value",
        "Unfulfilled Merge",
        "Unfulfilled Hierarchy",
        "Unfulfilled Total",
        "Unhappy leaves" };

    const std::vector<std::string> exampleRow{
        std::to_string(currentState.iteration),
        std::to_string(currentState.value),
        std::to_string(numConstraintsMergeSplit),
        std::to_string(numConstraintsHierarchy),
        std::to_string(numConstraintsHierarchy),
        std::to_string(currentState.statistic.unhappyLeaves.size()) };

    optimizationStatistics = createDataFrame({ exampleRow }, colHeaders);
}

void TemporalTreeOrderOptimization::setFileNames()
{
    initializeLog();

    time_t t = time(0);   // get time now
    struct tm * now = localtime(&t);

    std::stringstream ss;

    ss << (now->tm_year + 1900) << '-'
        << (now->tm_mon + 1) << '-'
        << now->tm_mday << '_'
        << now->tm_hour << '-'
        << now->tm_min << '-'
        << now->tm_sec << '-' << clock();
    std::string timeStamp = ss.str();

    propLogOptimizationFile.set(propLogDirectory.get() + "/" + logPrefix + timeStamp + ".csv" );
    propLogSettingsFile.set(propLogDirectory.get() + "/" + logPrefix + timeStamp + "_settings.csv");
}

} // namespace
} // namespace

