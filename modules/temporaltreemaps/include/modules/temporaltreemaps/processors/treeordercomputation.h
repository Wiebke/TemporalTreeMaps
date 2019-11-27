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
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/util/timer.h>
#include <modules/temporaltreemaps/datastructures/treeport.h>
#include <modules/temporaltreemaps/datastructures/constraint.h>
#include <modules/tools/performancetimer.h>
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <random>

namespace inviwo
{
namespace kth
{

/** \docpage{org.inviwo.TemporalTreeOrderOptimization, Tree Order Computation Heuristic}
![](org.inviwo.TemporalTreeOrderOptimization.png?classIdentifier=org.inviwo.TemporalTreeOrderOptimization)

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

/** \class TemporalTreeOrderOptimization
\brief VERY_BRIEFLY_DESCRIBE_THE_PROCESSOR

DESCRIBE_THE_PROCESSOR_FROM_A_DEVELOPER_PERSPECTIVE

@author Tino Weinkauf and Wiebke Koepp
*/
class IVW_MODULE_TEMPORALTREEMAPS_API TemporalTreeOrderOptimization : public Processor
{
//Friends
//Types
public:
    struct OptimizationState
    {
        size_t iteration = 0;
        TemporalTree::TTreeOrder order;
        ConstraintsStatistic statistic;
        double value = 0;
    };

//Construction / Deconstruction
public:
    TemporalTreeOrderOptimization();
    virtual ~TemporalTreeOrderOptimization() = default;

//Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:


    ///Fill some properties based on the statistics file
    void fillStatistics(const ConstraintsStatistic& statistic);

    void setInitialOrder();

    ///Initalize everything
    virtual void initializeResources() override;

    double weighUnfulfilledConstraint(Constraint& constraint);

    double evaluateOrder(const TemporalTree::TTreeOrder& order, ConstraintsStatistic* statistic);

    double evaluateOrder(const TemporalTree::TTreeOrder& order);

    ///Reset only statistic things and settings
    virtual void restart();

    ///Is the optimization converged
    virtual bool isConverged() = 0;

    ///Do a single optimization step
    virtual void singleStep() = 0;

    ///Run until convergence criterion is reached
    virtual void runUntilConvergence() = 0;

    ///Update the output
    void updateOutput();

    ///Save the log file
    void saveLog();

    virtual void logStep();

    virtual void initializeLog();

    virtual void logProperties() = 0;

    ///Our main computation function (Does nothing)
    virtual void process() override = 0;

    void setFileNames();

//Ports
public:
    ///Tree for which we compute the order
    TemporalTreeInport portInTree;

    ///Tree with an order
    TemporalTreeOutport portOutTree;

    ///Optimization LogFile
    DataFrameOutport portOutLogOptimization;

    ///Settings LogFile
    DataFrameOutport portOutLogSettings;

//Properties
public:
    ///Everything regarding settings
    CompositeProperty propSettings;

    ///Maximum number of iterations
    IntSizeTProperty propIterationsMax;

    ///Everything relating to the objective function
    CompositeProperty propObjectiveFunction;

    ///Weight by type only
    BoolProperty propWeightByTypeOnly;

    ///Weight for type only, meaning hierarchy constraints will get weight w, 
	///topology constraints get 1-w
    DoubleProperty propWeightTypeOnly;

    ///Give Merge/Split Constraints and Hierarchy Constraints different weights
    BoolProperty propWeightByType;

    ///Weight for merge/split
    DoubleProperty propWeightMergeSplit;

    ///Weight for hierarchy
    DoubleProperty propWeightHierarchy;

    ///Weight according to size (number of leaves)
    BoolProperty propWeightBySize;

    ///Weight for size
    DoubleProperty propWeightSize;

    ///Weight according to level
    BoolProperty propWeightByLevel;

    ///Weight for level
    DoubleProperty propWeightLevel;

	///Everything regarding the inital order
	CompositeProperty propInitialOrder;

	///Take input order as inital one if there is one
	BoolProperty propUseInputOrder;

	///Randomize the inital order
	BoolProperty propRandomizeOrder;

	///Whether to use the seed for the optimization or start randomly
	BoolProperty propRandomOrSeedOrder;

	///Seed for the optimization
	IntProperty propSeedOrder;

	///Settings regarding randomness
	CompositeProperty propRandomnessOptimization;

	///Whether to use the seed for the optimization or start randomly
	BoolProperty propRandomOrSeedOptimization;

	///Seed for the optimization
	IntProperty propSeedOptimization;

    /* Buttons */

    ///Everything regarding settings
    CompositeProperty propControls;

    ///Button for restarting Optimization
    ///(Mostly for debugging for now)
    ButtonProperty propRestart;

    ///Button for doing one optimization step
    ButtonProperty propSingleStep;

    ///Button for doing one optimization step at a time 
    ///until the button is pressed again.
    ButtonProperty propRunStepWise;

    ///Button for running the optimization until convergence
    ButtonProperty propRunUntilConvergence;

    ///Timer for running until convergece
    Timer runTimer;

    /* Current State */

    ///Everything regarding constraints
    CompositeProperty propCurrentState;

    ///Display current iteration
    IntProperty propCurrentIteration;

    ///Display current iteration
    IntProperty propBestIteration;

    ///Write out best or current state
    BoolProperty propOutputBestOrder;

    ///Display current number of all constraints fulfilled
    StringProperty propFulfilledConstraintsTotal;

    ///Display statistics for merge/split constraints
    StringProperty propStatisticsMergeSplit;

    ///Display statistics for hierarchy constraints
    StringProperty propStatisticsHierarchy;

    ///Display current number of all constraints fulfilled
    FloatProperty propObjectiveValue;

    ///Display the time the last action has taken in ms
    FloatProperty propTimeForLastAction;

    ///Display the time the last action has taken in ms
    FloatProperty propTimeUntilBest;

    ///Timer for the last action
    PerformanceTimer performanceTimer;

    ///Where to save things
    CompositeProperty propLog;

    ///Do we create log files or not
    BoolProperty propSaveLog;
    
    ///Where to put the logs
    DirectoryProperty propLogDirectory;

    ///Directory where we are savings settings
    FileProperty propLogSettingsFile;

    ///Directory where we are savings per step info
    FileProperty propLogOptimizationFile;

//Attributes
protected:
    ///Has this processor been initialized or not
    bool initialized;

    ///Input tree
    std::shared_ptr<const TemporalTree> pInputTree;

	///Random generator used in the optimization
	mutable std::mt19937 randomGen;

    ///Extracted constraints 
    std::vector<Constraint> constraints;
    
    ///Statistics about the extracted constraints
    std::vector<size_t> numByLevelHierarchy;
    std::vector<size_t> numByLevelMergeSplit;

    ///Total number of merge/split constraints
    size_t numConstraintsMergeSplit;

    ///Total number of hierarchy constraints
    size_t numConstraintsHierarchy;

    ///Max constraint level
    size_t maxConstraintLevel;

    ///Max constraint size
    size_t maxConstraintSize;

    ///Current state of the optimization
    OptimizationState currentState;

    ///Best state
    OptimizationState bestState;

    ///Collect some statisics during the optimization run
    std::shared_ptr<DataFrame> optimizationStatistics;

    ///Collect some statisics during the optimization run
    std::shared_ptr<DataFrame> optimizationSettings;

	///Prefix for the type of optimization
    std::string logPrefix;

};

} // namespace
} // namespace
