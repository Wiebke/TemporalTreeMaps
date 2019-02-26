/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Saturday, January 06, 2018 - 15:58:11
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <modules/temporaltreemaps/processors/treecushioncomputation.h>
#include <modules/temporaltreemaps/datastructures/cushion.h>

namespace inviwo
{
namespace kth
{

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TemporalTreeCushionComputation::processorInfo_
{
    "org.inviwo.TemporalTreeCushionComputation",      // Class identifier
    "Tree Cushion Computation",                // Display name
    "Temporal Tree",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};

const ProcessorInfo TemporalTreeCushionComputation::getProcessorInfo() const
{
    return processorInfo_;
}


TemporalTreeCushionComputation::TemporalTreeCushionComputation()
    :Processor()
    , portInTree("inTree")
    , portOutTree("outTree")
    , propCushionBaseHeight("cushionBaseHeight", "Base Height", 1.0f, 0.1f, 10.0f)
    , propCushionScaleFactor("cushionScaleFactor", "Scale Factor", 1.0f, 0.1f, 1.0f)
    , propCushionFrom("cushioFrom", "From Depth", 1, 0)
    , propCushionTo("cushionTo", "Until Depth", -1, -1)
{
    // Ports
    addPort(portInTree);
    addPort(portOutTree);

    // Properties
    addProperty(propCushionBaseHeight);
    addProperty(propCushionScaleFactor);
    addProperty(propCushionFrom);
    addProperty(propCushionTo);
}


void TemporalTreeCushionComputation::process()
{
    // Get tree
    std::shared_ptr<const TemporalTree> pInTree = portInTree.getData();
    if (!pInTree) return;

    std::shared_ptr<TemporalTree> pOutTree = std::make_shared<TemporalTree>(TemporalTree(*pInTree));

    int levels = int(pOutTree->getNumLevels(0));

    propCushionTo.setMaxValue(levels);
    propCushionFrom.setMaxValue(levels);

    std::set<uint64_t> times = pOutTree->getTimes();
    uint64_t tMin = *times.begin();
    uint64_t tMax = *times.rbegin();

    TemporalTree::TCushionMap& rootCushion = pOutTree->nodes[0].cushion;

    for (auto time : times)
    {
        rootCushion.insert({ time,{ vec3(0), vec3(0) } });
    }

    traverseToLeavesForCushions(*pOutTree, 0, tMin, tMax, 0);

    portOutTree.setData(pOutTree);

}

void TemporalTreeCushionComputation::traverseToLeavesForCushions(TemporalTree& tree, size_t nodeIndex, 
    uint64_t startTime, uint64_t endTime, uint8_t depth)
{
    // Shorthands
    TemporalTree::TNode& node = tree.nodes[nodeIndex];
    auto& cushion = node.cushion;
    auto& lowerLimit = node.lowerLimit;
    auto& upperLimit = node.upperLimit;

    if (lowerLimit.empty() || upperLimit.empty())
    {
        LogProcessorError("Skipping node" << nodeIndex <<  " and all of its children because it has no limit information");
        return;
    }

    // A parent might have 0 values that extend over the time of all its children
    // but we will have no limit information for these
    startTime = std::max(startTime, lowerLimit.begin()->first);
    endTime = std::min(endTime, lowerLimit.rbegin()->first);

    // Exclude the root and stop spanning cushions 
    if (depth >= propCushionFrom.get() && (propCushionTo.get() == -1 || depth <= propCushionTo.get()))
    {
        // Span cushions for this node over only the time frame that we want here 
        auto itLowerEnd = std::next(lowerLimit.find(endTime));
        auto itUpperEnd = std::next(upperLimit.find(endTime));
        for (auto itLowerLimit = lowerLimit.find(startTime),
            itUpperLimit = upperLimit.find(startTime);
            itLowerLimit != itLowerEnd && itUpperLimit != itUpperEnd; itLowerLimit++, itUpperLimit++)
        {
            vec3 first = cushion::makeCushion(itLowerLimit->second.first, itUpperLimit->second.first,
                propCushionBaseHeight.get(), propCushionScaleFactor.get(), uint8_t(depth-propCushionFrom));
            vec3 second = cushion::makeCushion(itLowerLimit->second.second, itUpperLimit->second.second,
                propCushionBaseHeight.get(), propCushionScaleFactor.get(), uint8_t(depth-propCushionFrom));
            auto inserted = cushion.emplace(itLowerLimit->first, std::make_pair(first, second));
            if (!inserted.second)
            {
                // inserted.first is the iterator in the map to where we tried to inserted, 
                // but there was already an element there -> second is the pair of cushions and first is the first one of those
                // We overwrite the element
                if (itLowerLimit->first != startTime)
                {
                    inserted.first->second.first += first;
                }
                if (itLowerLimit->first != endTime)
                {
                    inserted.first->second.second += second;
                }
            }
        }
    }

    // Go to all children
    for (auto& child : tree.getHierarchicalChildren(nodeIndex))
    {
        TemporalTree::TNode& childNode = tree.nodes[child];
        // Find the time range for which this is the child

        if (!TemporalTree::TNode::isOverlappingTemporally(startTime, endTime,
            childNode.startTime(), childNode.endTime()))
        {
            continue;
        }

        uint64_t startTimeChild = std::max(startTime, childNode.startTime());
        uint64_t endTimeChild = std::min(endTime, childNode.endTime());

        auto& cushionChild = childNode.cushion;

        // Initialize cushions of the child (Overlap - i.e. already inserted should only happen at the beginning and end)
        auto itParent = cushion.find(startTimeChild);
        auto itEnd = std::next(cushion.find(endTimeChild));
        for (; itParent != itEnd; itParent++)
        {
            auto inserted = cushionChild.emplace(itParent->first, itParent->second);
            if (!inserted.second)
            {
                // inserted.first is the iterator in the map to where we tried to inserted, 
                // but there was already an element there -> second is the pair of cushions and first is the first one of those
                // We only want to change the element that we are concerned with
                if (itParent->first != startTimeChild)
                {
                    inserted.first->second.first = itParent->second.first;
                }
                if (itParent->first != endTimeChild)
                {
                    inserted.first->second.second = itParent->second.second;
                }
            }
        }

        // traverse further until we have reached a leaf
        traverseToLeavesForCushions(tree, child, startTimeChild, endTimeChild, depth + 1);
    }
}

} // namespace kth
} // namespace

