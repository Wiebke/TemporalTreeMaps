/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Thursday, November 30, 2017 - 16:22:12
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <modules/temporaltreemaps/processors/treefilter.h>
#include <inviwo/core/util/utilities.h>

namespace inviwo
{
namespace kth
{

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TemporalTreeFilter::processorInfo_
{
    "org.inviwo.TemporalTreeFilter",      // Class identifier
    "Tree Filter",                // Display name
    "Temporal Tree",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};

const ProcessorInfo TemporalTreeFilter::getProcessorInfo() const
{
    return processorInfo_;
}


TemporalTreeFilter::TemporalTreeFilter()
    :Processor()
    , portInTree("inTree")
    , portOutTree("outTree")
    // Related to fading
	, propFading("fadeLeaves", "Fade Leaves")
    , propDoFade("doFade", "Fade", true)
    , propDeltaTFade("deltaTFade", "Fading Timespan", 1, 1)
	// Related to filtering
    , propFilter("filterLeaves", "Filter Laves")
    , propDoFilter("doFilter", "Filter", true)
    , propMinLifespan("minLifeSpan", "Minimum Lifespan", 2)
	, propDoFilterTimeSpan("propDoFilterTimeSpan", "Filter Timespan", false)
    , propStartLifespan("startLifeSpan", "Start Lifespan", 0)
    , propEndLifespan("endLifeSpan", "End Lifespan", 100000000)
    , propFilterDepth("filterDepth", "Filter Depth", 10, 2, 20)
	, propDoFilterFirstLevel("propDoFilterFirstLevel", "Filter First Level", false)
    , propFilterFirstLevelNodes("filterFirstLevelNodes", "First Level")
	// Splitting of nodes
    , propDoSplitLeaves("doSplitLeaves", "Split Nodes (Temporarily Leaves)", true)

{
    // Ports
    addPort(portInTree);
    addPort(portOutTree);
    

    portInTree.onChange([this]()
    {
        std::shared_ptr<const TemporalTree> pInTree = portInTree.getData();
        if (!pInTree) return;
        uint64_t tMax = pInTree->nodes[0].endTime();
        uint64_t tMin = pInTree->nodes[0].startTime();
        propEndLifespan.set(tMax);
        propEndLifespan.setMaxValue(tMax);
        propEndLifespan.setMinValue(tMin);
        propStartLifespan.setMaxValue(tMax);
        propStartLifespan.setMinValue(tMin);
        propStartLifespan.set(tMin);
        propFilterDepth.setMaxValue(pInTree->getNumLevels(0));
        propDoFilter.propertyModified();
        propDoFilterFirstLevel.propertyModified();
    });

    addProperty(propFilter);
    propFilter.addProperty(propDoFilter);
    propFilter.addProperty(propMinLifespan);

    propFilter.addProperty(propDoFilterTimeSpan);
    propFilter.addProperty(propStartLifespan);
    propFilter.addProperty(propEndLifespan);

    propFilter.addProperty(propFilterDepth);
    propFilter.addProperty(propDoFilterFirstLevel);

    propFilter.addProperty(propFilterFirstLevelNodes);

    propDoFilterFirstLevel.onChange([this]()
    {
        std::shared_ptr<const TemporalTree> pInTree = portInTree.getData();
        if (!pInTree) return;
        TemporalTree temp = TemporalTree(*pInTree);
        temp.computeAccumulated();

        float maxValueTree = std::numeric_limits<float>::min();
        for (auto& timeValuePair : temp.nodes[0].values)
        {
            if (timeValuePair.second > maxValueTree)
            {
                maxValueTree = timeValuePair.second;
            }
        }
        // Add new properties
        auto firstLevelNodes = pInTree->getHierarchicalChildren(0);

        if (propDoFilterFirstLevel.get())
        {
            // Remove any old properties
            auto properties = propFilterFirstLevelNodes.getProperties();
            for (auto& prop : properties)
            {
                propFilterFirstLevelNodes.removeProperty(prop->getIdentifier());
            }

            for (auto& nodeIndex : firstLevelNodes)
            {
                float maxValue = std::numeric_limits<float>::min();
                for (auto& timeValuePair : temp.nodes[nodeIndex].values)
                {
                    if (timeValuePair.second > maxValue)
                    {
                        maxValue = timeValuePair.second;
                    }
                }
                auto bp = new BoolProperty("prop" + std::to_string(nodeIndex),
                    pInTree->nodes[nodeIndex].name + " - " + std::to_string(int(maxValue)), maxValue < 0.015 * maxValueTree);
                propFilterFirstLevelNodes.addProperty(*bp);
            }
        } else
        {
            // Remove any old properties
            auto properties = propFilterFirstLevelNodes.getProperties();
            for (auto& prop : properties)
            {
                propFilterFirstLevelNodes.removeProperty(prop);
            }
        }
    });

    propDoFilter.onChange([this]()
    {
        if (propDoFilter.get() == 0)
        {
            util::hide(propMinLifespan, propStartLifespan, propEndLifespan, propFilterDepth, propFilterFirstLevelNodes);
            // Remove any old properties
            auto properties = propFilterFirstLevelNodes.getProperties();
            for (auto& prop : properties)
            {
                propFilterFirstLevelNodes.removeProperty(prop);
            }
        }
        else
        {
            util::show(propMinLifespan, propStartLifespan, propEndLifespan, propFilterDepth, propFilterFirstLevelNodes);
        }
    });

    addProperty(propDoSplitLeaves);

    addProperty(propFading);
    propFading.addProperty(propDoFade);
    propFading.addProperty(propDeltaTFade);

    propDoFade.onChange([this]()
    {
        if (propDoFade.get() == 0)
        {
            util::hide(propDeltaTFade);
        }
        else
        {
            util::show(propDeltaTFade);
        }
    });
}


void TemporalTreeFilter::process()
{
    // Get tree
    std::shared_ptr<const TemporalTree> pInTree = portInTree.getData();

    uint64_t timeSpan = pInTree->nodes[0].endTime() - pInTree->nodes[0].startTime();
    propDeltaTFade.setMaxValue(timeSpan);

    // Copy the tree, we need to change the values, edges or reverse edges
    std::shared_ptr<TemporalTree> pOutTree = std::make_shared<TemporalTree>(TemporalTree(*pInTree));
    
    if (propDoFilter.get())
    {
        size_t filtered(pOutTree->nodes.size());
        filterLeaves(pOutTree);
        // Reverse Edges need to be updated (we might have deleted some)
        pOutTree->computeReverseEdges();

        filtered -= pOutTree->nodes.size();
        LogProcessorInfo("Filtered " << filtered << " nodes.");
    }else
    {
        pOutTree->computeReverseEdges();
    }

    if (propDoSplitLeaves.get())
    {
        splitTemporalLeaves(pOutTree);
        pOutTree->computeReverseEdges();
    }

    if (propDoFade.get())
    {
        fadeLeaves(pOutTree);
    }

    portOutTree.setData(pOutTree);

}

void TemporalTreeFilter::fadeLeaves(std::shared_ptr<TemporalTree>& tree)
{
    auto leaves = tree->getLeaves();
    std::set<uint64_t> times = tree->getTimes();

    std::vector<size_t> fadeIn;
    std::vector<size_t> fadeOut;

    tree->computeReverseEdges();

    // Find all new insert times (These need to be handled in the fading of other nodes as well)
    for (auto leaf : leaves)
    {
        TemporalTree::TNode& leafNode = tree->nodes[leaf];

        // If this node truly appears, its beginning value is not 0 and the time is not the global beginning
        if (tree->getTemporalPredecessorsWithReverse(leaf).size() == 0 && leafNode.startTime() != *times.begin())
        {
            // Minimum out of global end, leaf end and leaf befin + delta fade
            uint64_t fadeInEnd = std::min(std::min(leafNode.startTime() + propDeltaTFade.get(), leafNode.endTime())
                , *times.rbegin());
            times.insert(fadeInEnd);
            fadeIn.push_back(leaf);
        }

        // Treat last value exactly the same way, but fade out instead
        if (tree->getTemporalSuccessors(leaf).size() == 0 && leafNode.endTime() != *times.rbegin())
        {
            // Maximum out of global begin, leaf begin and leaf end - delta fade (We need to treat potential unsigned int underflow here)
            uint64_t fadeOutBegin = std::max(std::max(
                (leafNode.endTime() - propDeltaTFade.get())*(leafNode.endTime() >= propDeltaTFade.get()), 
                leafNode.startTime()), *times.begin());
            times.insert(fadeOutBegin);
            fadeOut.push_back(leaf);
        }
    }

    // Go through the leaves again (but only the ones that need to be faded this time)
    for (auto leaf : fadeIn)
    {
        TemporalTree::TNode& leafNode = tree->nodes[leaf];

        // If this node truly appears and the time is not the global beginning
        if (tree->getTemporalPredecessorsWithReverse(leaf).size() == 0 && leafNode.startTime() != *times.begin())
        {
            std::pair<uint64_t, float> left = { (leafNode.startTime()), 0.0f };
            uint64_t fadeInEnd = std::min(leafNode.startTime() + propDeltaTFade.get(), *times.rbegin());
            // If the lifespan of the node is shorter than the fading in timespan, it will just be 0
            std::pair<uint64_t, float> right = { fadeInEnd, leafNode.getValueAt(fadeInEnd) };
            // Iterate until the endTime 
            // (but not longer then the lifespan, so that we do not add additional values, parents might not be active either )
            auto itEnd = std::next(times.find(std::min(fadeInEnd, leafNode.endTime())));
            for (auto itTimes = times.find(leafNode.startTime());
                itTimes != itEnd; itTimes++)
            {
                leafNode.values[*itTimes] = linearInterpolation(left, right, *itTimes);
            }
        }
    }

    for (auto leaf : fadeOut)
    {
        TemporalTree::TNode& leafNode = tree->nodes[leaf];

        if (tree->getTemporalSuccessors(leaf).size() == 0 && leafNode.endTime() != *times.rbegin())
        {
            std::pair<uint64_t, float> right = { leafNode.endTime(), 0.0f };
            uint64_t fadeOutBegin = std::max(
                (leafNode.endTime() - propDeltaTFade.get())*(leafNode.endTime() >= propDeltaTFade.get()), 
                *times.begin());
            // If the lifespan of the node is shorter than the fading out timespan, it will just be 0
            std::pair<uint64_t, float> left = { fadeOutBegin, leafNode.getValueAt(fadeOutBegin) };
            auto itEnd = std::next(times.find(leafNode.endTime()));
            for (auto itTimes = times.find(std::max(fadeOutBegin, leafNode.startTime()));
                itTimes != itEnd; itTimes++)
            {
                leafNode.values[*itTimes] = linearInterpolation(left, right, *itTimes);
            }
        }
    }
}

void TemporalTreeFilter::filterLeaves(std::shared_ptr<TemporalTree>& tree)
{
    bool filtered = false;
    std::set<size_t> removeNodes;

    tree->computeReverseEdges();
    TemporalTree::TNode& root = tree->nodes[0];
    const uint64_t treeStartTime = root.startTime();
    const uint64_t treeEndtime = root.endTime();

    std::set<uint64_t> times;
    tree->getTimes(0, times);

    if (propStartLifespan >= propEndLifespan)
    {
        LogProcessorError("No filtering to a  (less then) single timestep");
    }

    size_t nodeIndex(0);
    for (auto& node : tree->nodes)
    {
        if (nodeIndex == 0)
        {
            nodeIndex++;
            continue;
        }
        size_t depth = tree->depthWithReverse(nodeIndex);
        if (
            depth > propFilterDepth || 
            // 
            (tree->isLeaf(nodeIndex) && node.endTime() - node.startTime() < propMinLifespan.get() &&
            tree->getTemporalSuccessors(nodeIndex).empty() &&
            tree->getTemporalPredecessorsWithReverse(nodeIndex).empty())
            // Regardless of sucessors or predecessors, 
            // we filter nodes existing only for the first or 
            // last timestep
            //|| node.startTime() == treeEndtime
            //|| node.endTime() == treeStartTime
            || (node.endTime() < propStartLifespan && propDoFilterTimeSpan) 
            || (node.startTime() > propEndLifespan && propDoFilterTimeSpan))
        {
            removeNodes.insert(nodeIndex);
            filtered = true;
        }
        if (propDoFilterFirstLevel && depth == 1 && !propFilterFirstLevelNodes.getProperties().empty())
        {
            auto pp = dynamic_cast<BoolProperty*>(
                propFilterFirstLevelNodes.getPropertyByIdentifier("prop" + std::to_string(nodeIndex)));
            if (pp &&  pp->get())
            {
                removeNodes.insert(nodeIndex);
                std::vector<size_t> LevelIndices{ nodeIndex };
                size_t Level = 1;
                while (!LevelIndices.empty())
                {
                    LevelIndices = tree->getLevel(Level+1, LevelIndices, Level);
                    removeNodes.insert(LevelIndices.begin(), LevelIndices.end());
                }

            }
        }


        TemporalTree::TValueMap values;
        for (auto it=times.find(propStartLifespan); it != times.upper_bound(propEndLifespan); it++)
        {
            uint64_t time = *it;
            if (time > propEndLifespan)
            {
                break;
            }
            if (time < node.startTime())
            {
                continue;
            }
            if (time > node.endTime())
            {
                break;
            }
            values[time] = node.getValueAt(time);
        }
        if (values.size() < 2)
        {
            removeNodes.insert(nodeIndex);
        }
        node.values = values;
        nodeIndex++;
    }

    if (removeNodes.empty())
    {
        return;
    }

    std::set<size_t>::iterator itRemoveLeaves = removeNodes.begin();
    std::map<size_t, size_t> oldToNewIndices;

    nodeIndex = 0;
    for (size_t newIndex = 0; nodeIndex < tree->nodes.size(); nodeIndex++, newIndex++)
    {
        if (itRemoveLeaves != removeNodes.end() && *itRemoveLeaves == nodeIndex)
        {
            newIndex--;
            itRemoveLeaves++;
        }
        else
        {
            oldToNewIndices[nodeIndex] = newIndex;
        }
    }

    // Go through the nodes vector to delete the marked leaves
    for (auto itRemoveLeavesReverse = removeNodes.rbegin(); 
        itRemoveLeavesReverse != removeNodes.rend(); itRemoveLeavesReverse++)
    {
        tree->nodes.erase(tree->nodes.begin() + *itRemoveLeavesReverse);
    }

    TemporalTree::TAdjacency oldEdgesHierarchy = tree->edgesHierarchy;
    tree->edgesHierarchy.clear();

    for (const auto& adjacencyNode : oldEdgesHierarchy)
    {
        size_t edgeFrom = adjacencyNode.first;
        // We wont have deleted a node that has children 
        // (unless in the first or last timestep)
        auto itFrom = oldToNewIndices.find(edgeFrom);
        if (itFrom != oldToNewIndices.end())
        {
            size_t edgeFromNew = itFrom->second;
            for (auto edgeTo : adjacencyNode.second)
            {
                auto itTo = oldToNewIndices.find(edgeTo);
                // If its not in this map it was a leaf we just deleted
                if (itTo != oldToNewIndices.end())
                {
                    tree->addHierarchyEdge(edgeFromNew, itTo->second);
                }
            }
        }
    }
    TemporalTree::TAdjacency oldEdgesTime = tree->edgesTime;
    tree->edgesTime.clear();

    for (const auto& adjacencyNode : oldEdgesTime)
    {
        size_t edgeFrom = adjacencyNode.first;
        auto itFrom = oldToNewIndices.find(edgeFrom);
        if (itFrom != oldToNewIndices.end())
        {
            size_t edgeFromNew = itFrom->second;
            for (auto edgeTo : adjacencyNode.second)
            {
                // We won't have deleted nodes that have predecessors or successors
                // (unless in the first or last timestep)
                auto itTo = oldToNewIndices.find(edgeTo);
                // If its not in this map it was a leaf we just deleted
                if (itTo != oldToNewIndices.end())
                {
                    tree->addTemporalEdge(edgeFromNew, itTo->second);
                }
            }
        }
    }

    TemporalTree::TTreeOrder newOrder;

    // Tree order, if it exists
    if (!tree->order.empty())
    {
        for (auto oldIndex : tree->order){
            auto itNew = oldToNewIndices.find(oldIndex);
            if (itNew != oldToNewIndices.end())
            {
                newOrder.push_back(itNew->second);
            }
        }
    }

    tree->order = newOrder;

    // Update root values
    tree->nodes[0].values.clear();
    tree->nodes[0].values[propStartLifespan] = 0.0;
    tree->nodes[0].values[propEndLifespan] = 0.0;

    // Maybe we now have new leaves that need to be filtered
    if (filtered)
    {
        filterLeaves(tree);
    }
}

void TemporalTreeFilter::splitTemporalLeaves(std::shared_ptr<TemporalTree>& tree)
{

    tree->splitTemporaryLeaves();
}

float TemporalTreeFilter::linearInterpolation(const std::pair<uint64_t, float>& left, const std::pair<uint64_t, float>& right, const uint64_t x)
{
    if (left.first == right.first)
    {
        if (left.first == x)
        {
            return left.second;
        }
        return 0.0;
    }
    return left.second + float(x - left.first) * (right.second - left.second) / float(right.first - left.first);
}

} // namespace kth
} // namespace
