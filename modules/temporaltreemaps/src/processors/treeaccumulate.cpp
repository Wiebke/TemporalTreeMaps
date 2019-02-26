/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Wednesday, November 29, 2017 - 23:25:56
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <kxtrees/processors/treeaccumulate.h>

namespace inviwo
{
namespace kth
{

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TemporalTreeAccumulate::processorInfo_
{
    "org.inviwo.TemporalTreeAccumulate",      // Class identifier
    "Tree Accumulate",                // Display name
    "Temporal Tree",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};

const ProcessorInfo TemporalTreeAccumulate::getProcessorInfo() const
{
    return processorInfo_;
}


TemporalTreeAccumulate::TemporalTreeAccumulate()
    :Processor()
    , portInTree("inTree")
    , portOutTree("outTree")
{
    addPort(portInTree);
    addPort(portOutTree);
}


void TemporalTreeAccumulate::process()
{
    // Get tree
    std::shared_ptr<const TemporalTree> treeIn = portInTree.getData();
    if (!treeIn) return;

    // Copy the tree, so that we can adapt the values;
    std::shared_ptr<TemporalTree> treeOut =  std::make_shared<TemporalTree>(TemporalTree(*treeIn));
    // Spead up accumulation by computing the reverse edges
    if (treeOut->edgesHierarchy.size() != 0 && treeOut->reverseEdgesHierachy.size() == 0)
    {
        treeOut->computeReverseEdges();
    }
    // Actual accumulation of the values
    // each node now holds values for each global timestep
    // TODO: Make use of reverse if those has been computed
    treeOut->computeAccumulated();

    // Set output
    portOutTree.setData(treeOut);
}


} // namespace kth
} // namespace

