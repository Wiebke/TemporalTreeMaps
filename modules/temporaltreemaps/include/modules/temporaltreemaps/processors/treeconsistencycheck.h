/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Köpp
 *  Init    : Wednesday, November 22, 2017 - 17:27:15
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
#include <modules/temporaltreemaps/datastructures/tree.h>
#include <modules/temporaltreemaps/datastructures/treeport.h>

namespace inviwo
{
namespace kth
{

/** \docpage{org.inviwo.TemporalTreeConsistencyCheck, Tree Consistency Check}
    ![](org.inviwo.TemporalTreeConsistencyCheck.png?classIdentifier=org.inviwo.TemporalTreeConsistencyCheck)

    Checks the consistency of a tree
    
    ### Inports
      * __<Inport>__ The tree to be checked for consistency.
    
    ### Outports
      * __<Outport>__ <description>.
*/


/** \class TemporalTreeConsistencyCheck
    \brief Checks if a temporal tree is consistent

    @author Tino Weinkauf and Wiebke Köpp
*/
class IVW_MODULE_TEMPORALTREEMAPS_API TemporalTreeConsistencyCheck : public Processor
{ 
//Friends
//Types
public:

//Construction / Deconstruction
public:
    TemporalTreeConsistencyCheck();
    virtual ~TemporalTreeConsistencyCheck() = default;

//Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    ///Our main computation function
    virtual void process() override;

//Ports
public:
    TemporalTreeInport portInTree;

//Properties
public:

//Attributes
private:

};

} // namespace kth
} // namespace
