/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Tuesday, March 27, 2018 - 16:33:19
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <modules/temporaltreemaps/processors/treeordercomputationsa.h>

namespace inviwo
{
namespace kth
{

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TemporalTreeSimulatedAnnealing::processorInfo_
{
    "org.inviwo.TemporalTreeSimulatedAnnealing",      // Class identifier
    "Tree Simulated Annealing",                // Display name
    "Temporal Tree",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};

const ProcessorInfo TemporalTreeSimulatedAnnealing::getProcessorInfo() const
{
    return processorInfo_;
}


TemporalTreeSimulatedAnnealing::TemporalTreeSimulatedAnnealing()
    : TemporalTreeOrderOptimization()
    // Settings
    , propSimulatedAnnealing("simulatedAnnealing", "Simulated Annealing")
    , propInitialTemperature("initialTemperature", "Initial T", 10, 0, 1000, 0.25)
    , propMinimumTemperature("minimumTemperature", "Minimum T", 0, 0, 1, 10e-6)
    , propTemperatureDecay("temperatureDecay", "T Decay", 0.9, 0.6, 0.99, 0.1)
    , propIterationsPerTemp("iterationsPerTemp", "Iters Per T", 10, 1, 1000, 1)
    // Current State
    , propCurrentTemperature("currentTemperature", "Current T", 0, 0, 1000, 10e-6)
{
    /* Settings */

    propSettings.addProperty(propSimulatedAnnealing);

    propSimulatedAnnealing.addProperty(propInitialTemperature);
    propInitialTemperature.onChange([&]() { restart(); });
    propInitialTemperature.setSemantics(PropertySemantics::Text);

    propSimulatedAnnealing.addProperty(propMinimumTemperature);
    //propMinimumTemperature.onChange([&]() { restart(); });
    propMinimumTemperature.setSemantics(PropertySemantics::Text);

    propSimulatedAnnealing.addProperty(propTemperatureDecay);
    propTemperatureDecay.onChange([&]() { restart(); });
    propTemperatureDecay.setSemantics(PropertySemantics::Text);

    propSimulatedAnnealing.addProperty(propIterationsPerTemp);
    propIterationsPerTemp.onChange([&]() { restart(); });
    propIterationsPerTemp.setSemantics(PropertySemantics::Text);

    /* Current state */
    propCurrentState.addProperty(propCurrentTemperature);
    propCurrentTemperature.setSemantics(PropertySemantics::Text);
    propCurrentTemperature.setReadOnly(true);
}

void TemporalTreeSimulatedAnnealing::swapNodes(std::vector<size_t>& nodes)
{
    std::uniform_int_distribution<int> chooseSwapNodes(0, static_cast<int>(nodes.size()) - 1);
    // We have at least two nodes, so we can definately find a pair of nodes to swap
    int swapA = chooseSwapNodes(randomGen);
    int swapB = chooseSwapNodes(randomGen);

    while (swapA == swapB)
    {
        swapB = chooseSwapNodes(randomGen);
    }

    //LogInfo("Swapped positions " << swapA << " and " << swapB << ".");
    std::swap(nodes[swapA], nodes[swapB]);
}

void TemporalTreeSimulatedAnnealing::decayTemperature()
{
    // Exponential multiplicative cooling: 0.8 <= temperatureDecay <= 0.9
    currentTemperature *= propTemperatureDecay;

    // Others:
    // Logarithmical multiplicative cooling: temperatureDecay > 1
    // currentTemperature = initialTemperature / 1 + temperatureDecay*(log(1 + currentIteration + 1));
    // ... (7 or so others)
    propCurrentTemperature.set(currentTemperature);
}

bool TemporalTreeSimulatedAnnealing::acceptNeighbor(double deltaEnergy) const
{
    // If the new Energy is better or equal we accept it (Boltzmann/Metropolis critera)
    if (!(deltaEnergy <= 0))
    {
        // For positive \Delta E the following holds:
        // \lim_{T \rightarrow 0} \exp\left(\frac{-\Delta E}{T}\right) = \lim_{x \rightarrow -\infty} \exp(x) = 0
        // and
        // \lim_{T \rightarrow \infty} \exp\left(-\frac{\Delta E}{T}\right) = \lim_{x \rightarrow 0} \exp(x) = 1
        // this means if we want the initial probability to accept close to 1
        // then
        const double probabilityThreshold =
            currentTemperature > 0 ? std::exp(-(deltaEnergy) / currentTemperature) : -1.0;
        // Do not accept the solution
        const double probability = uniformReal(randomGen);
        if (probability > probabilityThreshold)
        {
            return false;
        }
    }

    // Others: Barker criterion

    return true;
}

void TemporalTreeSimulatedAnnealing::logProperties()
{
    const std::vector<std::string> colHeaders{
		propSeedOrder.getDisplayName(),
        propSeedOptimization.getDisplayName(),
        propIterationsMax.getDisplayName(),
        propInitialTemperature.getDisplayName(),
        propMinimumTemperature.getDisplayName(),
        propTemperatureDecay.getDisplayName(),
        propIterationsPerTemp.getDisplayName(),
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
        std::to_string(propInitialTemperature),
        std::to_string(propMinimumTemperature),
        std::to_string(propTemperatureDecay),
        std::to_string(propIterationsPerTemp),
        std::to_string(propWeightByTypeOnly),
        std::to_string(propWeightTypeOnly),
		std::to_string(bestState.iteration),
		std::to_string(bestState.value),
        std::to_string(propTimeUntilBest),
        std::to_string(propTimeForLastAction) };

    optimizationSettings = createDataFrame({ exampleRow }, colHeaders);
    optimizationSettings->addRow(exampleRow);
}

void TemporalTreeSimulatedAnnealing::initializeLog()
{
    const std::vector<std::string> colHeaders{
        "Iteration",
        "Temperature",
        "Energy",
        "Delta Energy",
        "Accept Propabilty",
        "Accepted",
        "Unfulfilled Merge",
        "Unfulfilled Hierarchy",
        "Unfulfilled Total", 
        "Unhappy leaves"};

    const std::vector<std::string> exampleRow{
        std::to_string(currentState.iteration),
        std::to_string(currentTemperature),
        std::to_string(currentState.value),
        std::to_string(currentState.value),
        std::to_string(currentState.value),
        std::to_string(true),
        std::to_string(numConstraintsMergeSplit),
        std::to_string(numConstraintsHierarchy),
        std::to_string(numConstraintsHierarchy),
        std::to_string(currentState.statistic.unhappyLeaves.size())};

    optimizationStatistics = createDataFrame({ exampleRow }, colHeaders);
}

void TemporalTreeSimulatedAnnealing::logStep()
{
	if (!optimizationStatistics) return;

    bool firstRow = optimizationStatistics->getNumberOfRows() == 0;
    
    size_t numUnfulfilledMergeSplit = numConstraintsMergeSplit - currentState.statistic.numFulFilledMergeSplitConstraints();
    size_t numUnfulfilledHierarchy = numConstraintsHierarchy - currentState.statistic.numFulfilledHierarchyConstraints();

    std::vector<std::string> newRow{
        std::to_string(int(currentState.iteration) - 1), //"Iteration"
        std::to_string(currentTemperature), //"Current Temperature",
        std::to_string(currentState.value), //"Current Value",
        std::to_string(lastDeltaEnergy), // "Delta Energy"
        std::to_string((currentTemperature > 0) ? (std::exp(-(lastDeltaEnergy) / currentTemperature)) : -1.0), //"Accept Propabilty"
        std::to_string(lastAccepted), // State was accepted
        std::to_string(firstRow ? numConstraintsMergeSplit : numUnfulfilledMergeSplit),
        std::to_string(firstRow ? numConstraintsHierarchy : numUnfulfilledHierarchy),
        std::to_string(firstRow ? numConstraintsHierarchy + numConstraintsMergeSplit : numUnfulfilledMergeSplit + numUnfulfilledHierarchy),
        std::to_string(currentState.statistic.unhappyLeaves.size())
    };
        
    optimizationStatistics->addRow(newRow);
}

void TemporalTreeSimulatedAnnealing::restart()
{

	TemporalTreeOrderOptimization::restart();

    currentTemperature = propInitialTemperature;
    propCurrentTemperature.set(currentTemperature);

	logStep();
}

bool TemporalTreeSimulatedAnnealing::isConverged()
{
    // the iteration number is an index starting at 0, the max is a number >= -1
    if (currentState.iteration > propIterationsMax - 1)
    {
        LogProcessorInfo("Converged by reaching maximum number of iterations.");
        return true;
    }
    if (currentTemperature < propMinimumTemperature)
    {
        LogProcessorInfo("Converged by temperature falling below the minimum.");
        return true;
    }
    if (std::abs(currentState.value) < std::numeric_limits<float>::epsilon())
    {
        LogProcessorInfo("Converged by reaching a global optimum.");
        return true;
    }

    // TODO: Iterations with no improvement
    return false;
}

void TemporalTreeSimulatedAnnealing::singleStep()
{
	if (isConverged())
	{
		return;
	}

    setLastToCurrent();

    // Generate a neighbor state (Changes current State)
    neighborSolution();
    
    // Evaluate new state
    currentState.statistic.clear();
    currentState.value = evaluateOrder(currentState.order, &currentState.statistic);

    lastDeltaEnergy = currentState.value - lastState.value;

    // Check if we can accept the new solution
    if (!acceptNeighbor(currentState.value - lastState.value))
    {
        // Go back to the previous state
        setCurrentToLast();
        lastAccepted = false;
    }
    else
    {
        // Prepare the next step
        prepareNextStep();
        
        // Update best
        if (currentState.value < bestState.value)
        {
            propTimeUntilBest.set(performanceTimer.ElapsedTime());
            setBest();
        }

        lastAccepted = true;
    }

    // Update for the next step
    currentState.iteration++;
    logStep();

    // Decay temperature after we have done the specified number of iterations
    if (currentState.iteration % propIterationsPerTemp == 0)
    {
        decayTemperature();
    }

}

void TemporalTreeSimulatedAnnealing::runUntilConvergence()
{
    while (!isConverged())
    {
        singleStep();
    }
}

} // namespace
} // namespace

