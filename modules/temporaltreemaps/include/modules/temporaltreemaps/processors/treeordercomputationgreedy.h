/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Thursday, March 29, 2018 - 15:00:03
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
#include <random>

namespace inviwo
{
namespace kth
{

/** \docpage{org.inviwo.TemporalTreeOrderComputationGreedy, Tree Order Computation Greedy}
    ![](org.inviwo.TreeOrderComputationGreedy.png?classIdentifier=org.inviwo.TemporalTreeOrderComputationGreedy)

    Explanation of how to use the processor.
    
    ### Inports
      * __<Inport1>__ <description>.
    
    ### Outports
      * __<Outport1>__ <description>.
    
    ### Properties
      * __<Prop1>__ <description>.
      * __<Prop2>__ <description>
*/


/** \class TemporalTreeOrderComputationGreedy
    \brief VERY_BRIEFLY_DESCRIBE_THE_PROCESSOR
    
    DESCRIBE_THE_PROCESSOR_FROM_A_DEVELOPER_PERSPECTIVE

    @author Tino Weinkauf and Wiebke Koepp
*/
class IVW_MODULE_TEMPORALTREEMAPS_API TemporalTreeOrderComputationGreedy : public TemporalTreeOrderOptimization
{ 
//Friends
//Types
public:

//Construction / Deconstruction
public:
	TemporalTreeOrderComputationGreedy();
    virtual ~TemporalTreeOrderComputationGreedy() = default;

//Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
	///Out of all currently unresolved constraints, choose the best resolution
	///if there are multiple "best", choose at random which of those to apply
    void resolveConstraint();

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

    ///Our main computation function
    virtual void process() override;

//Ports
public:

//Properties
public:

//Attributes
private:
    ///Vector of unfulfilled constraints
    std::vector<size_t> unfulfilledConstraints;
};

} // namespace
} // namespace
