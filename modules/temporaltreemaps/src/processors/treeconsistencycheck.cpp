/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Wednesday, November 22, 2017 - 17:27:15
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <modules/temporaltreemaps/processors/treeconsistencycheck.h>
#include <inviwo/core/util/exception.h>

namespace inviwo
{
namespace kth
{

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TemporalTreeConsistencyCheck::processorInfo_
{
    "org.inviwo.TemporalTreeConsistencyCheck",      // Class identifier
    "Tree Consistency Check",                // Display name
    "Temporal Tree",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};

const ProcessorInfo TemporalTreeConsistencyCheck::getProcessorInfo() const
{
    return processorInfo_;
}


TemporalTreeConsistencyCheck::TemporalTreeConsistencyCheck()
    :Processor(),
    portInTree("inTree")
{
    addPort(portInTree);
    //addProperty();
}


void TemporalTreeConsistencyCheck::process()
{
    //Get the tree
    std::shared_ptr<const TemporalTree> inTree = portInTree.getData();

    if (!inTree->checkConsistency())
    {
        LogProcessorError("Tree is not consistent.");
    }
    else
    {
        LogProcessorInfo("Tree is consistent.")
    }
}

} // namespace kth
} // namespace
