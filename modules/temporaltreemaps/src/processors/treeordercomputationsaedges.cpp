/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke K�pp
 *  Init    : Wednesday, November 22, 2017 - 18:46:01
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <modules/temporaltreemaps/processors/treeordercomputationsaedges.h>

namespace inviwo
{
namespace kth
{

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TemporalTreeOrderComputationSAEdges::processorInfo_
{
    "org.inviwo.TemporalTreeOrderComputationSAEdges", // Class identifier
    "Tree Order SA Edges",      // Display name
    "Temporal Tree",          // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};

const ProcessorInfo TemporalTreeOrderComputationSAEdges::getProcessorInfo() const
{
    return processorInfo_;
}

TemporalTreeOrderComputationSAEdges::TemporalTreeOrderComputationSAEdges()
    : TemporalTreeSimulatedAnnealing()
{

    /* Controls */

    propRestart.onChange([&]()
    {
        if (!initialized)
        {
            initializeResources();
        }
        else
        {
            restart();
        }
        updateOutput();
    });

    propSingleStep.onChange([&]()
    {
        if (!initialized) initializeResources();
        performanceTimer.Reset();
        singleStep();
        propTimeForLastAction.set(performanceTimer.ElapsedTimeAndReset());
        updateOutput();
    });

    runTimer.setCallback([this]()
    {
        if (!initialized) initializeResources();
		singleStep();
        // Stop timer and performance timer when we have converged
        if (isConverged())
        {
            propTimeForLastAction.set(performanceTimer.ElapsedTimeAndReset());
            runTimer.stop();
            propRunStepWise.setDisplayName("Run Stepwise");
        }
		updateOutput();
    });

    propRunUntilConvergence.onChange([&]()
    {
		if (!initialized || currentState.iteration != 0) initializeResources();
		restart();
        performanceTimer.Reset();
        runUntilConvergence();
        propTimeForLastAction.set(performanceTimer.ElapsedTimeAndReset());
		if (propSaveLog) {
			saveLog();
		}
		updateOutput();
    });

    logPrefix = "saEdges";
}


void TemporalTreeOrderComputationSAEdges::restart()
{    
    TemporalTreeSimulatedAnnealing::restart();

    // Clear the old state
    currentEdges.clear();
    currentReverseEdges.clear();
    activeNodes.clear();

	// Initialize heuristic edges
    std::vector<bool> visitedHierarchy(pInputTree->nodes.size(), false);
    std::vector<bool> visitedTime(pInputTree->nodes.size(), false);
    dfsHierarchy(pInputTree, 0, visitedHierarchy, visitedTime);

    // Remove nodes that do not have an impact on the final order
    cleanEdges(pInputTree);

	currentState.order.clear();
	treeorder::orderAsDepthFirst(currentState.order, *pInputTree, currentEdges);
	currentState.value = evaluateOrder(currentState.order, &currentState.statistic);

    float averageDegree = 0;

    // Initialize active nodes
    for (auto& currentEdge : currentEdges)
    {
        // The order can be affected at any node that has more than one outgoing connection
        if (currentEdge.second.size() > 1)
        {
            activeNodes.push_back(currentEdge.first);
            averageDegree += currentEdge.second.size();
        }
    }
    chooseNode = std::uniform_int_distribution<int>(0,
        static_cast<int>(activeNodes.size()) - 1);

    if (!activeNodes.empty())
    {
        LogInfo("Active nodes: " << activeNodes.size() << " with average degree: " << averageDegree / activeNodes.size());
    }

    lastEdges = currentEdges;
    bestEdges = currentEdges;

    bestState = currentState;
    lastState = currentState;

	logStep();
}

void TemporalTreeOrderComputationSAEdges::setLastToCurrent()
{
    lastEdges = currentEdges;
    lastState = currentState;
}

void TemporalTreeOrderComputationSAEdges::setCurrentToLast()
{
    currentEdges = lastEdges;
    currentState = lastState;
}

void TemporalTreeOrderComputationSAEdges::setBest()
{
    bestState = currentState;
    bestEdges = currentEdges;
}

void TemporalTreeOrderComputationSAEdges::prepareNextStep()
{
    // No preparation necessary here
}

void TemporalTreeOrderComputationSAEdges::neighborSolution()
{
    // TODO: Maybe consider Dynamic Neighbourhood Size in Simulated Annealing
    // Choose one node for which we want to swap to children
    std::vector<size_t>& nodes = currentEdges.at(activeNodes[chooseNode(randomGen)]);
    ivwAssert(nodes.size() >= 2, "");
    //LogInfo("Swapped at position " << index << ".");
    swapNodes(nodes);

    currentState.order.clear();
    treeorder::orderAsDepthFirst(currentState.order, *pInputTree, currentEdges);
}

void TemporalTreeOrderComputationSAEdges::initializeResources()
{
    std::shared_ptr<const TemporalTree> pTreeIn = portInTree.getData();
    if (!pTreeIn) return;
   

    /* Initialize merge/split constraints and tree */

    TemporalTreeOrderOptimization::initializeResources();

    /* Find the init case for this tree */

    bool mergeOrSplitFound = false; //There is a merge or a split somewhere in the tree
    bool onlyMerges = true; //There are only merges and these only occur on the leaf level
    bool onlySplits = true; // There are only splits and these only occur on the leaf level

    for (auto edge : pTreeIn->edgesTime)
    {
        // Found a split
        if (edge.second.size() > 1)
        {
            mergeOrSplitFound = true;
            // Split is outside the leaf level
            if (!pTreeIn->isLeaf(edge.first))
            {
                onlySplits = false;
                onlyMerges = false;
                break;
            }
            // Found a merge at the leaf level
            // -> There are not solely splits there
            onlyMerges = false;
        }
    }
    // We still have a chance for an easy case
    if (onlyMerges || onlySplits)
    {
        for (auto edge : pTreeIn->reverseEdgesTime)
        {
            // Found a merge
            if (edge.second.size() > 1)
            {
                mergeOrSplitFound = true;
                if (!pTreeIn->isLeaf(edge.first))
                {
                    onlySplits = false;
                    onlyMerges = false;
                    break;
                }
                // Found a merge at the leaf level
                // -> There are not solely splits there
                onlySplits = false;
            }
        }
    }

    // Set the case we are in according to the finding above
    if (!mergeOrSplitFound)
    {
        initCase = NoMergesOrSplits;
    }
    else
    {
        if (onlyMerges)
        {
            initCase = MergesOnly;
        }
        else if (onlySplits)
        {
            initCase = SplitsOnly;
        }
        else
        {
            initCase = MergesAndSplits;
        }
    }

    initialized = true;
    restart();

}

void TemporalTreeOrderComputationSAEdges::process()
{
    // Nothing to do, all button presses
}

void TemporalTreeOrderComputationSAEdges::printStatistic(std::shared_ptr<TemporalTree> tree) const
{
    // Get statistics about the tree
    std::vector<size_t> leafDepthStatisticOriginal;
    std::vector<bool> visited(tree->nodes.size(), false);
    tree->leafDepthStatistic(0, visited, tree->edgesHierarchy, leafDepthStatisticOriginal, 0);

    std::ostringstream statisticOriginal;
    if (!leafDepthStatisticOriginal.empty())
    {
        // Convert all but the last element to avoid a trailing ","
        std::copy(leafDepthStatisticOriginal.begin(), leafDepthStatisticOriginal.end() - 1,
            std::ostream_iterator<size_t>(statisticOriginal, ","));

        // Now add the last element with no delimiter
        statisticOriginal << leafDepthStatisticOriginal.back();
    }

    std::vector<size_t> leafDepthStatisticOptimization;
    // After going through the original tree, we will have visited each node
    // flipping resets all to false
    visited.flip();
    tree->leafDepthStatistic(0, visited, currentEdges, leafDepthStatisticOptimization, 0);

    std::ostringstream statisticOptimization;
    if (!leafDepthStatisticOptimization.empty())
    {
        // Convert all but the last element to avoid a trailing ","
        std::copy(leafDepthStatisticOptimization.begin(), leafDepthStatisticOptimization.end() - 1,
            std::ostream_iterator<size_t>(statisticOptimization, ","));

        // Now add the last element with no delimiter
        statisticOptimization << leafDepthStatisticOptimization.back();
    }

    //LogProcessorInfo("Depth of leaves: " << statisticOriginal.str() << " (original tree) " <<
    //    statisticOptimization.str() << " (optimization edges).");
    // Node has no children if it has no outgoing hierarchical edges
}

void TemporalTreeOrderComputationSAEdges::addEdge(const size_t from, const size_t to)
{
    //Add edge to the map (create either a new entry or update the entry for the from-key)
    auto itToAdd = currentEdges.find(from);
    if (itToAdd == currentEdges.end())
    {
        TemporalTree::TAdjacency::mapped_type edgesToAdd = { to };
        currentEdges.emplace(from, edgesToAdd);
    }
    else
    {
        (itToAdd->second).push_back(to);
    }
    TemporalTree::TAdjacency::mapped_type edgesToAdd = { from };
    const auto inserted = currentReverseEdges.emplace(to, edgesToAdd);
    ivwAssert(inserted.second, "We are building a tree, how can there already be a parent?");
}

void TemporalTreeOrderComputationSAEdges::dfsTime(std::shared_ptr<const TemporalTree> tree, size_t nodeIndex,
    std::vector<bool>& visited)
{
    // Go back in time when we just have merges
    auto& edgesTime = initCase != MergesOnly ? tree->edgesTime : tree->reverseEdgesTime;

    visited[nodeIndex] = true;
    const auto itEdgesTimes = edgesTime.find(nodeIndex);

    //Leaf?
    if (itEdgesTimes != edgesTime.end())
    {
        auto& sucessors = itEdgesTimes->second;
        for (auto sucessor : sucessors)
        {
            if (!visited[sucessor])
            {
                if (initCase == MergesOnly || initCase == SplitsOnly)
                {
                    addEdge(nodeIndex, sucessor);
                }
                else
                {
                    // Add an edge between the parent of your predeccessor (in the heuristic tree)
                    // And the node that is just being considered
                    addEdge(currentReverseEdges[nodeIndex][0], sucessor);
                }
                dfsTime(tree, sucessor, visited);
            }
        }
    }
}

void TemporalTreeOrderComputationSAEdges::dfsHierarchy(std::shared_ptr<const TemporalTree> tree, size_t nodeIndex,
    std::vector<bool>& visitedHierarchy, std::vector<bool>& visitedTime)
{
    visitedHierarchy[nodeIndex] = true;
    const auto itEdgesHierarchy = tree->edgesHierarchy.find(nodeIndex);

    //Leaf?
    if (itEdgesHierarchy != tree->edgesHierarchy.end())
    {
        auto children = itEdgesHierarchy->second;
        if (initCase != MergesOnly)
        {
            tree->sortByTime(children);
        }
        else
        {
            tree->sortByTimeBackwards(children);
        }
        for (auto child : children)
        {
            if (!visitedHierarchy[child])
            {
                if (!visitedTime[child])
                {
                    if (initCase != MergesAndSplits)
                    {
                        addEdge(nodeIndex, child);
                    }
                    else
                    {
                        // This node has multiple parents, i.e. is child of something that splits or merges
                        if (tree->reverseEdgesHierachy.at(child).size()>1)
                        {
                            // We add it to the parent of its parent in the heuristic tree
                            // It does not matter for which of its parents we look this up, they all shoudl
                            // have the same parent
                            addEdge(currentReverseEdges[nodeIndex][0], child);
                        }
                        else
                        {
                            addEdge(nodeIndex, child);
                        }

                    }
                    dfsTime(tree, child, visitedTime);
                }
                dfsHierarchy(tree, child, visitedHierarchy, visitedTime);
            }
        }
    }
}

void TemporalTreeOrderComputationSAEdges::cleanEdges(std::shared_ptr<const TemporalTree> tree)
{
    // We can remove nodes that are leafs in the optimization edge set
    // but not in the original tree
    for (size_t nodeIndex(0); nodeIndex < tree->nodes.size(); nodeIndex++)
    {
        auto itOptimization = currentEdges.find(nodeIndex);
        // Leaf in optimization edge set
        if (itOptimization == currentEdges.end())
        {
            // Not a leaf in the original tree
            auto itOriginal = tree->edgesHierarchy.find(nodeIndex);
            if (itOriginal != tree->edgesHierarchy.end())
            {
                // Find the node that is pointing to this one
                // We created a tree, so there needs to be a single reverse edge
                auto parent = currentReverseEdges.at(nodeIndex)[0];
                auto& outgoingFromParent = currentEdges.at(parent);
                // Delete the edge
                auto itToDelete = std::find(
                    outgoingFromParent.begin(), outgoingFromParent.end(), nodeIndex);
                outgoingFromParent.erase(itToDelete);
            }
        }
    }
}


} // namespace kth
} // namespace

