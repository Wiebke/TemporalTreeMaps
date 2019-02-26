/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Tuesday, March 27, 2018 - 16:33:19
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
#include <modules/temporaltreemaps/processors/treeordercomputation.h>
#include <random>

namespace inviwo
{
namespace kth
{

/** \docpage{org.inviwo.TemporalTreeSimulatedAnnealing, Tree Simulated Annealing}
    ![](org.inviwo.TemporalTreeSimulatedAnnealing.png?classIdentifier=org.inviwo.TemporalTreeSimulatedAnnealing)

    Explanation of how to use the processor.
    
    ### Inports
      * __<Inport1>__ <description>.
    
    ### Outports
      * __<Outport1>__ <description>.
    
    ### Properties
      * __<Prop1>__ <description>.
      * __<Prop2>__ <description>
*/


/** \class TemporalTreeSimulatedAnnealing
    \brief VERY_BRIEFLY_DESCRIBE_THE_PROCESSOR
    
    DESCRIBE_THE_PROCESSOR_FROM_A_DEVELOPER_PERSPECTIVE

    @author Tino Weinkauf and Wiebke Koepp
*/
class IVW_MODULE_TEMPORALTREEMAPS_API TemporalTreeSimulatedAnnealing : public TemporalTreeOrderOptimization
{ 
//Friends
//Types
public:

//Construction / Deconstruction
public:
    TemporalTreeSimulatedAnnealing();
    virtual ~TemporalTreeSimulatedAnnealing() = default;

//Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    // Swap two nodes in the given vector, assumes there are two 
    // nodes in that vector
    void swapNodes(std::vector<size_t>& nodes);

    ///Decay the temperature
    void decayTemperature();

    ///Accept a neighbor state based on current temperature and difference in energy
    bool acceptNeighbor(double deltaEnergy) const;

    ///Neighbor Solution from the current state 
    virtual void neighborSolution() = 0;

    ///Initalize everything
    virtual void initializeResources() override = 0;

    ///Reset only statistic things and settings
    virtual void restart() override;

    ///Is the optimization converged
    bool isConverged() override;

    ///Do a single optimization step
    void singleStep() override;

    ///Prepare the next step (update some optimization structures)
    virtual void prepareNextStep() = 0;

    ///Run until convergence criterion is reached
    void runUntilConvergence() override;

    virtual void setLastToCurrent() = 0;

    virtual void setCurrentToLast() = 0;

    virtual void setBest() = 0;

    virtual void logProperties() override;

    void initializeLog() override;

    void logStep() override;

    ///Our main computation function (does nothing)
    virtual void process() override = 0;

//Ports
public:

//Properties
public:
    ///All properties related to simulated annealing
    CompositeProperty propSimulatedAnnealing;

    ///Initial temperature
    DoubleProperty propInitialTemperature;

    ///Minimum temperature at which we stop the optimization
    DoubleProperty propMinimumTemperature;

    ///Temperature decay
    DoubleProperty propTemperatureDecay;

    ///Number of iterations per temperature setting
    IntProperty propIterationsPerTemp;

    ///The current temperature 
    DoubleProperty propCurrentTemperature;

//Attributes
protected:
    ///We need to access the input tree in a number of places
    std::shared_ptr<const TemporalTree> pInputTree;

    ///Current temperature
    double currentTemperature;

    ///Random distribution for sampling neighbor states
    std::uniform_real_distribution<> uniformReal{ 0.0, 1.0 };

    ///Last state so that we can back 
    OptimizationState lastState;

	///State info: Was the last neighbor accepted
    bool lastAccepted;

	///State info: What was the last enegery delta
    double lastDeltaEnergy;
};



} // namespace
} // namespace
