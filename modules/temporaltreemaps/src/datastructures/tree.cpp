/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Köpp
 *  Init    : Monday, October 09, 2017 - 16:31:44
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <modules/temporaltreemaps/datastructures/tree.h>
#include <inviwo/core/util/exception.h>

namespace inviwo
{
namespace kth
{

    /**** Utility functions for accessing parts of the tree ****/

/*
    TemporalTree TemporalTree::getHierarchyAt(uint64_t time, bool accumulate = false, uint64_t deltaTime = 1) const
    {
        TemporalTree snapshot = TemporalTree();

        // Start with the root node
        size_t currentRoot = std::numeric_limits<size_t>::max();
        bool foundRoot = false;
        for (auto root : this->rootIndices)
        {
            const TemporalTree::TNode& currentRootNode = nodes[currentRoot];
            if (time >= currentRootNode.startEvent.time  && time < currentRootNode.endEvent.time)
            {
                // The tree would not be consistent if there are multiple roots for this time,
                // we can stop at the first found one
                currentRoot = root;
                foundRoot = true;
                break;
            }
        }

        // No root node was active in the specified time return the empty tree
        if (foundRoot == false)
        {
            return snapshot;
        }

        // Maps node indices of the entire tree to the snapshot
        std::map<size_t, size_t> indexMap;

        // Nodes and Edges for the snapshot
        std::vector<TNode> snapshotNodes;
        TAdjacency snapshotEdges;

        auto copyRelevantNodeAttributes = [&](const TemporalTree::TNode& node) -> TemporalTree::TNode
        {
            TemporalTree::TNode copy = TemporalTree::TNode();
            // Copy name 
            copy.name = node.name;
            // Start and end events have no meaning for a snapshot
            // thus set to creating and annihilation for given time
            copy.startEvent = { time, TDelimitingEventType::Creation };
            copy.endEvent = { time + deltaTime, TDelimitingEventType::Annihilation };
            // Get value for the current time
            copy.values = { { time, node.getValueAt(time) } };
            return copy;
        };

        // Set root
        snapshotNodes.push_back(copyRelevantNodeAttributes(nodes[currentRoot]));
        indexMap.insert(std::make_pair(currentRoot, size_t(0)));
        snapshot.rootIndices = { 0 };

        // Iterate over all edges and extract the ones that are active at the current time
        for (auto it = this->edgesHierarchy.begin();
            it != this->edgesHierarchy.end(); it++)
        {
            for (auto edge : it->second)
            {
                // Edge is active at the current time
                size_t edgeFrom = it->first;
                if (time >= edge.startTime && time < edge.endTime)
                {
                    std::map<size_t, size_t>::iterator indexPairFrom = indexMap.find(edgeFrom);
                    std::map<size_t, size_t>::iterator indexPairTo = indexMap.find(edge.to);
                    // From node of the edge has not been added to the snapshot
                    if (indexPairFrom == indexMap.end())
                    {
                        snapshotNodes.push_back(copyRelevantNodeAttributes(nodes[edgeFrom]));
                        snapshotNodes[snapshotNodes.size() - 1].index = snapshotNodes.size() - 1;
                        indexPairFrom = indexMap.insert(std::make_pair(edgeFrom, snapshotNodes.size() - 1)).first;
                    }
                    // To node has not been added to the snapshot
                    if (indexPairTo == indexMap.end())
                    {
                        snapshotNodes.push_back(copyRelevantNodeAttributes(nodes[edge.to]));
                        snapshotNodes[snapshotNodes.size() - 1].index = snapshotNodes.size() - 1;
                        indexPairTo = indexMap.insert(std::make_pair(edge.to, snapshotNodes.size() - 1)).first;
                    }
                    // Add edge with the snapshot indices
                    snapshot.addHierarchyEdge(indexPairFrom->second, indexPairTo->second, time, time + deltaTime);
                } // Active edge
            } // Outgoing edges for one node
        }

        // Fill the snapshot with the collected data
        // No temporal edges because the tree is just a single snapshot
        snapshot.nodes = snapshotNodes;
        snapshot.edgesHierarchy = snapshotEdges;

        // TODO: Check if accumulation has already been done and just use those values
        if (accumulate)
        {
            snapshot.computeAccumulated();
        }

        return snapshot;
    }
*/

    std::vector<size_t> TemporalTree::getLevel(const size_t level,
                                         const std::vector<size_t>& prevLevelIndices,
                                         const size_t prevLevel) const
    {
        size_t currentLevel;
        std::set<size_t> currentLevelIndices;

        // Use the given last level, if that is set 
        if (!prevLevelIndices.empty())
        {
            ivwAssert(level > prevLevel, "Desired level needs to be larger than the given one.");
            currentLevelIndices.insert(prevLevelIndices.cbegin(), prevLevelIndices.cend());
            currentLevel = prevLevel;
        }
        else
        {
            //At Level 0, we look at the root
            //ACHTUNG: We assume here that the root has the index 0!
            currentLevel = 0;
            currentLevelIndices.emplace(0);
        }
        
        std::set<size_t> nextLevelIndices;
        while (currentLevel < level)
        {
            nextLevelIndices.clear();

            // Get all children of Nodes in the current Level
            for (const auto parentIndex : currentLevelIndices)
            {
                std::map<size_t, std::vector<size_t> >::const_iterator it = edgesHierarchy.find(parentIndex);
                if (it != edgesHierarchy.end())
                {
                    nextLevelIndices.insert(it->second.cbegin(), it->second.cend());
                }
            }

            // Update the current level
            currentLevel++;
            currentLevelIndices = nextLevelIndices;
        }

        return std::vector<size_t>(currentLevelIndices.cbegin(), currentLevelIndices.cend());
    }


    size_t TemporalTree::getNumLevels(const size_t nodeIndex) const
    {
        //Is this a leaf?
        // Node has no children if it has no outgoing hierarchical edges
        auto it = edgesHierarchy.find(nodeIndex);
        if (it == edgesHierarchy.end())
        {
            return 0;
        }
        else
        {
            size_t MaxChildrenLevel(0);
            for(const auto& ID : it->second)
            {
                const size_t ThisChildDepth = getNumLevels(ID);
                if (ThisChildDepth > MaxChildrenLevel) MaxChildrenLevel = ThisChildDepth;
            }

            return 1 + MaxChildrenLevel;
        }
    }

    void TemporalTree::leafDepthStatistic(const size_t nodeIndex, std::vector<bool>& visited, const TemporalTree::TAdjacency& edges, 
        std::vector<size_t>& leafDepthCounters, size_t depth) const
    {
        visited[nodeIndex] = true;
        //Is this a leaf -> record its depth (according to edges)
        auto it = edgesHierarchy.find(nodeIndex);
        if (it == edgesHierarchy.end())
        {
            if(leafDepthCounters.size() < depth+1)
            {
                leafDepthCounters.resize(depth+1, 0);
            }
            leafDepthCounters[depth]++;
        }
        // Traverse edges
        auto itEdges = edges.find(nodeIndex);
        if (itEdges != edges.end())
        {
            for (const auto& child : itEdges->second)
            {
                if (!visited[child])
                {
                    leafDepthStatistic(child, visited, edges, leafDepthCounters, depth + 1);
                }
            }
        }
    }

    void TemporalTree::splitTemporaryLeaves()
    {
        bool madeSplit = false;

        std::set<uint64_t> times;
        getTimes(0, times);
        /// Go to every single node and check if it temporaly becomes a leaf

        size_t numNodes = nodes.size();
        
        for (size_t nodeIndex(1); nodeIndex < numNodes; nodeIndex++)
        {
            TNode& node = nodes[nodeIndex];

            // Node is always a leaf, nothing to be done
            if (isLeaf(nodeIndex))
            {
                continue;
            }

            std::set<size_t> leafIndices;
            getLeaves(nodeIndex, node.startTime(), node.endTime(), node.startTime(), node.endTime(), leafIndices);

            // Times at which this node has leaves
            std::set<uint64_t> hasLeaves;

            for (auto leafIndex : leafIndices)
            {
                auto itStartTimeChild = times.find(std::max(node.startTime(), nodes[leafIndex].startTime()));
                const auto itOneAfterEndTimeChild = std::next(times.find(std::min(node.endTime(), nodes[leafIndex].endTime())));
                for (; itStartTimeChild != itOneAfterEndTimeChild; itStartTimeChild++)
                {
                    hasLeaves.insert(*itStartTimeChild);
                }
            }

            auto itStartTimeNode = times.find(node.startTime());
            const auto itEndTimeNode = std::next(times.find(node.endTime()));

            // Are there times at which this node is a leaf 
            if (std::distance(itStartTimeNode, itEndTimeNode) != int(hasLeaves.size()))
            {
                LogInfo("Node " << nodeIndex << " temporarily becomes a leaf.");
                // Split at all the points where this node becomes a leaf
                std::vector<TNode> newNodes;
                // Get expanded version of the values for this node
                TValueMap nodeValues = node.values; 
                TNode::fillWithLeftNeighborInterpolation(times, nodeValues);
                
                TNode& lastNode = node;
                lastNode.values.clear();

                int stateBefore = 0; // 0:First Node, 1: Non-leaf, 2:Leaf

                for (auto it = itStartTimeNode; it != itEndTimeNode; it++)
                {
                    // The node is leaf now, but was not a leaf before 
                    // or the node it not a leaf now, but was one before
                    const auto itIsLeaf = hasLeaves.find(*it);

                    // Same state as before or very beginning
                    if (stateBefore == 0 || (itIsLeaf != hasLeaves.end() && stateBefore == 1)
                        || (itIsLeaf == hasLeaves.end() && stateBefore == 2))
                    {
                        // Just extend the node
                        lastNode.values.emplace(*it, nodeValues[*it]);
                    // Was not a leaf before, now is a leaf
                    } else if (itIsLeaf == hasLeaves.end() && stateBefore == 1){
                        // We need to start a new node (a leaf) and also give it this value and the previous one
                        // Previous one will always exists, there cannot be a one value leaf
                        // that later becomes a non-leaf or vice versa (because when would it?)
                        newNodes.push_back(lastNode);
                        lastNode = TNode(node.name + "_" + std::to_string(newNodes.size()));
                        lastNode.values.emplace(*std::prev(it), nodeValues[*std::prev(it)]);
                        lastNode.values.emplace(*it, nodeValues[*it]);
                        // newState == 2
                    // Was a leaf before, now is not a leaf
                    } else //if (itIsLeaf != hasLeaves.end() && stateBefore == 2))
                    {
                        // Finish the old node 
                        lastNode.values.emplace(*it, nodeValues[*it]);
                        newNodes.push_back(lastNode);
                        // Start a new one with this value
                        lastNode = TNode(node.name + "_" + std::to_string(newNodes.size()));
                        lastNode.values.emplace(*it, nodeValues[*it]);
                        // newState == 1
                    }
                    // Update the state for the next iteration
                    stateBefore = itIsLeaf == hasLeaves.end() ? 2 : 1;
                }
                // Add the final node
                newNodes.push_back(lastNode);

                // The first node replaces the old node
                nodes[nodeIndex] = newNodes[0];
                
                // If we have just a single node, this node will have to be
                // processed again in a later iteration, but all its connections are still valid

                if (newNodes.size() >= 2)
                {
                    madeSplit = true;
                    // Add the new nodes to the tree
                    std::vector<size_t> newIndices(newNodes.size());
                    newIndices[0] = nodeIndex;
                    for (size_t index = 1; index<newNodes.size(); index++)
                    {
                        newIndices[index] = nodes.size();
                        nodes.push_back(newNodes[index]);
                    }

                    // Update temporal Edges:
                    // Everything that points to the node, will now point to the first of the new nodes (Fine)
                    // Everything that the node pointed to, the last new node needs to point to
                    auto itSuccessors = edgesTime.find(nodeIndex);
                    if (itSuccessors != edgesTime.end())
                    {
                        std::vector<size_t>& sucessors = itSuccessors->second;
                        edgesTime.emplace_hint(edgesTime.end(), newIndices.back(), sucessors);
                    }
                    // Connect the new nodes with each other
                    for (size_t index = 0; index<newNodes.size()-1; index++)
                    {
                        std::vector<size_t> sucessor = { newIndices[index + 1] };
                        edgesTime.insert_or_assign(newIndices[index], sucessor);
                    }

                    // Update hierarchy edges
                    // For parents: Delete the old value and find overlaps with new nodes
                    std::vector<size_t> parents = getHierarchicalParentsWithReverse(nodeIndex);
                    for (auto parent : parents)
                    {
                        // Parents were found through hierarchy edges, so we can safely assume them to be there
                        auto& itAllChildren = edgesHierarchy[parent];
                        auto itChild = std::find(itAllChildren.begin(), itAllChildren.end(), nodeIndex);
                        itAllChildren.erase(itChild);

                        const uint64_t parentStartTime = nodes[parent].startTime();
                        const uint64_t parentEndTime = nodes[parent].endTime();

                        // Connect to all children that overlap
                        for (size_t index = 0; index<newNodes.size(); index++)
                        {
                            const uint64_t overlapStart = std::max(parentStartTime, newNodes[index].startTime());
                            const uint64_t overlapEnd = std::min(parentEndTime, newNodes[index].endTime());
                            if (overlapStart < overlapEnd ||
                                // newNodes[index] is just a single timestep node
                                (overlapStart == overlapEnd && 
                                    (newNodes[index].startTime() == newNodes[index].endTime() 
                                        || parentStartTime == parentEndTime)))
                            {
                                itAllChildren.push_back(newIndices[index]);
                            }
                        }
                    }
                    // For children: Essentially the same
                    auto itChildren = edgesHierarchy.find(nodeIndex);
                    // Copy the children (we are about to delete the array)
                    auto children = itChildren->second;
                    edgesHierarchy.erase(itChildren);
                    for (auto child : children)
                    {
                        const uint64_t childStartTime = nodes[child].startTime();
                        const uint64_t childEndTime = nodes[child].endTime();

                        for (size_t index = 0; index < newNodes.size(); index++)
                        {
                            const uint64_t overlapStart = std::max(childStartTime, newNodes[index].startTime());
                            const uint64_t overlapEnd = std::min(childEndTime, newNodes[index].endTime());
                            // Connect if there is overlap
                            if (overlapStart < overlapEnd)
                            {
                                addHierarchyEdge(newIndices[index], child);
                            }
                            // For single time value Children, there will only be a single edge connection (the first )
                            else if (overlapStart == overlapEnd && (childStartTime == childEndTime || 
                                newNodes[index].startTime() == newNodes[index].endTime()))
                            {
                                addHierarchyEdge(newIndices[index], child);
                                break;
                            }
                        }
                    }
                }
            }

            // A bit overkill, we could also update this while we are adding the new forward edges
            reverseEdgesHierachy = getReverseEdges(edgesHierarchy);
        }

        if (madeSplit)
        {
            splitTemporaryLeaves();
        }

    }

    std::vector<size_t> TemporalTree::getLeaves() const
    {
        std::vector<size_t> leaves;

        // Find all nodes that do not have children
        for (size_t nodeIndex(0);nodeIndex<nodes.size();nodeIndex++)
        {
            // Node has no children if it has no outgoing hierarchical edges
            auto it = this->edgesHierarchy.find(nodeIndex);
            if (it == edgesHierarchy.end())
            {
                leaves.push_back(nodeIndex);
            }
        }

        return leaves;
    }

    void TemporalTree::getLeaves(const size_t nodeIndex, const uint64_t initialStarttTime, 
        const uint64_t initialEndTime, uint64_t startTime, uint64_t endTime, std::set<size_t>& Leaves) const
    {
        const TNode& node = nodes[nodeIndex];

        if (!TNode::isOverlappingTemporally(startTime, endTime, node.startTime(), node.endTime()))
        {
            return;
        }

        auto it = edgesHierarchy.find(nodeIndex);
        // Called on a leaf
        if (it == edgesHierarchy.end())  
        {
            // Get only leaves that are overlapping with start,end excluding only overlap on the endpoints
            if (initialStarttTime == initialEndTime ||
                (initialStarttTime != endTime && initialEndTime != startTime))
            {
                Leaves.insert(nodeIndex);
            }

            return;
        }

        // Update the startime for this node (if for some reason function was called with times beyond the nodes lifetime)
        startTime = std::max(startTime, node.startTime());
        endTime = std::min(endTime, node.endTime());

        for(const auto& childIndex : it->second)
        {
        // Is the child overlapping with the given time
            if (TNode::isOverlappingTemporally(startTime, endTime, nodes[childIndex].startTime(), nodes[childIndex].endTime()))
            {
                if (edgesHierarchy.find(childIndex) == edgesHierarchy.end())
                {
                    // It overlaps with initial time, so we insert it

                    if (initialStarttTime == initialEndTime ||
                        (initialStarttTime != nodes[childIndex].endTime() && initialEndTime != nodes[childIndex].startTime()))
                    {
                        Leaves.insert(childIndex);
                    }
                }
                else
                {
                    // Compute timeframe in which the child overlaps
                    uint64_t startTimeChild = std::max(startTime, nodes[childIndex].startTime());
                    uint64_t endTimeChild = std::min(endTime, nodes[childIndex].endTime());
                    getLeaves(childIndex, startTimeChild, endTimeChild, startTimeChild, endTimeChild, Leaves);
                }
            }
        }
        
    }

    size_t TemporalTree::depth(const size_t nodeIndex) const
    {
        if (nodeIndex == 0) return 0;
        auto parents = getHierarchicalParents(nodeIndex);
        ivwAssert(parents.size() != 0, "A node does not have a parent");
        // A parents will have the same distance to the root
        return 1 + depth(parents[0]);
    }

    size_t TemporalTree::depthWithReverse(const size_t nodeIndex) const
    {
        if (nodeIndex == 0) return 0;
        auto parents = getHierarchicalParentsWithReverse(nodeIndex);
        ivwAssert(parents.size() != 0, "A node does not have a parent");
        // A parents will have the same distance to the root
        return 1 + depth(parents[0]);
    }

    void TemporalTree::sortByTime(std::vector<size_t>& nodesIndices) const
    {
        std::sort(nodesIndices.begin(), nodesIndices.end(),
            [&](const size_t a, const size_t b) -> bool
        {
            uint64_t startA = nodes[a].startTime();
            uint64_t startB = nodes[b].startTime();
            if (startA != startB)
            {
                return startA < startB;
            } else
            {
                return nodes[a].endTime() < nodes[b].endTime();
            }
        });
    }

    void TemporalTree::sortByTimeBackwards(std::vector<size_t>& nodesIndices) const
    {
        std::sort(nodesIndices.begin(), nodesIndices.end(),
            [&](const size_t a, const size_t b) -> bool
        {
            uint64_t endA = nodes[a].endTime();
            uint64_t endB = nodes[b].endTime();
            if (endA != endB)
            {
                return endA > endB;
            }
            else
            {
                return nodes[a].startTime() > nodes[b].startTime();
            }
        });
    }

    std::map<size_t, std::vector<size_t> > TemporalTree::getReverseEdges(const TAdjacency& Edges) const
    {
        std::map<size_t, std::vector<size_t>> reverseEdges;
        // Iterate through all edges and add reverse eges to the map created here
        std::map<size_t, std::vector<size_t> >::iterator itReverse;
        for (std::map<size_t, std::vector<size_t> >::const_iterator it = Edges.begin();
            it != Edges.end(); it++)
        {
            for (auto edgeTo : it->second)
            {
                itReverse = reverseEdges.find(edgeTo);
                if (itReverse == reverseEdges.end())
                {
                    std::vector<size_t> edgeFrom = { { it->first } };
                    reverseEdges.emplace(edgeTo, edgeFrom);
                }
                else
                {
                    itReverse->second.push_back(it->first);
                }
            }
        }
        return reverseEdges;
    }

    
    std::vector<size_t> TemporalTree::getEdgesFrom(const size_t nodeIndex, const TAdjacency& Edges) const
    {
        std::map<size_t, std::vector<size_t> >::const_iterator it = Edges.find(nodeIndex);

        // Node has no successors if it has no outgoing edges
        if (it == Edges.end())
        {
            return std::vector<size_t>();
        }

        return it->second;
    }


    std::vector<size_t> TemporalTree::getEdgesTo(const size_t nodeIndex, const TAdjacency & Edges) const
    {
        std::vector<size_t> EdgesPointingToHere;

        //Go through all edges and collect the ones that point to the given node
        for (auto& it : Edges)
        {
            for (auto edgeTo : it.second)
            {
                if (edgeTo == nodeIndex)
                {
                    EdgesPointingToHere.push_back(it.first);
                }
            }
        }

        return EdgesPointingToHere;
    }


    size_t TemporalTree::getNumEdges(const TAdjacency& Edges) const
    {
        size_t Num(0);

        for(const auto& edgeGroup : Edges)
        {
            Num += edgeGroup.second.size();
        }

        return Num;
    }

    void TemporalTree::deaggregate(TemporalTree& tree) const
    {
        if (!tree.nodes.empty() || !tree.edgesHierarchy.empty() || !tree.edgesTime.empty())
        {
            LogWarn("Passed tree for deaggregation should be empty. Its contents will be cleared.");
        }

        tree.nodes.clear();
        tree.edgesHierarchy.clear();
        tree.edgesTime.clear();


        // Compute times in this tree
        std::set<size_t> times;
        getTimes(0, times);

        // Mapping of the nodes in the original tree, to the nodes in the new tree
        std::map<size_t, std::vector<size_t>> nodesOrgingalToDeaggregated;

        // We just copy the root
        tree.addNode(nodes[0]);
        nodesOrgingalToDeaggregated[0] = { 0 };

        // Deaggregate nodes
        size_t nodeIndex(0);

        size_t numLeaves(0);
        for (auto node : nodes)
        {
            // Skip the root (already processed)
            if (nodeIndex == 0)
            {
                nodeIndex++;
                continue;
            }
            // A node gets deaggregated by creating a node for a every global time step
            // in which it exists
            TValueMap values = node.values;
            TNode::fillWithLeftNeighborInterpolation(times, values);

            std::vector<size_t> correspondingNodes;
            correspondingNodes.reserve(values.size());

            int lastIndex = -1;

            auto predecessors = getTemporalPredecessorsWithReverse(nodeIndex);
            auto successors = getTemporalSuccessors(nodeIndex);
            auto itEnd = std::prev(values.end());
            for (auto itValue = values.begin(); itValue != itEnd; itValue++)
            {
                std::string name = node.name + "_" + std::to_string(itValue->first);
                auto itNext = std::next(itValue);
                TNode newNode (name, { { itValue->first, itValue->second }});
                ivwAssert(itNext != values.end(), "Deaggregation: Next Value is not the end.")
                // Add the next value to this one as well
                newNode.values[itNext->first] = itNext->second;

                tree.addNode(newNode);
                size_t newIndex = tree.nodes.size() - 1;
                correspondingNodes.push_back(newIndex);
                // Except for the first one we need to connect to the first 
                if (lastIndex >= 0)
                {
                    tree.addTemporalEdge(size_t(lastIndex), newIndex);
                }
                lastIndex = int(newIndex);
            }
            if (node.endTime() == *times.rbegin())
            {
                auto itValue = node.values.rbegin();
                TNode newNode(node.name + "_" + std::to_string(itValue->first) , { { itValue->first, itValue->second } });
                tree.addNode(newNode);
                size_t newIndex = tree.nodes.size() - 1;
                correspondingNodes.push_back(newIndex);
                // Except for the first one we need to connect to the first 
                if (lastIndex >= 0)
                {
                    tree.addTemporalEdge(size_t(lastIndex), newIndex);
                }
            }
            nodesOrgingalToDeaggregated.emplace_hint(nodesOrgingalToDeaggregated.end(), nodeIndex, correspondingNodes);
            // If this node is a leaf, all its corresponding nodes will be too
            if (edgesHierarchy.find(nodeIndex) == edgesHierarchy.end())
            {
                numLeaves += correspondingNodes.size();
            }
            nodeIndex++;
            
        }

        for (auto outgoingTemporalEdges : edgesTime)
        { 
            // Index in the original tree
            size_t edgeFrom = outgoingTemporalEdges.first;
            // Corresponding edges in the deaggreagted tree start at the last corresponding node
            auto it = nodesOrgingalToDeaggregated.find(edgeFrom);
            if (it != nodesOrgingalToDeaggregated.end() && !it->second.empty())
            {
                edgeFrom = it->second.back();
            }
            else
            {
                LogWarn("Temporal edge group not in deaggregated")
                continue;
            }
            for (auto edgeTo : outgoingTemporalEdges.second)
            {
                it = nodesOrgingalToDeaggregated.find(edgeTo);
                if (it != nodesOrgingalToDeaggregated.end() && !it->second.empty())
                {
                    edgeTo = it->second.front();
                    tree.addTemporalEdge(edgeFrom, edgeTo);
                }
                else
                {
                    LogWarn("Temporal edge not in deaggregated");   
                }
            }

        }

        for (auto outgoingHierarchyEdges : edgesHierarchy)
        {
            size_t edgeFrom = outgoingHierarchyEdges.first;
            auto it = nodesOrgingalToDeaggregated.find(edgeFrom);
            std::vector<size_t> nodesFrom;
            if (it != nodesOrgingalToDeaggregated.end())
            {
                nodesFrom = it->second;
            }
            else
            {
                LogWarn("Hierarchy edge group not in deaggregated");
                continue;
            }
            for (auto edgeTo : outgoingHierarchyEdges.second)
            {
                it = nodesOrgingalToDeaggregated.find(edgeTo);
                std::vector<size_t> nodesTo;
                if (it != nodesOrgingalToDeaggregated.end())
                {
                    nodesTo = it->second;
                    // For first level nodes (connected to root in the original tree)
                    // deaggregated first level nodes are also all connected to the single root
                    if (edgeFrom == 0)
                    {
                        for (auto firstLevelNode : nodesTo) tree.addHierarchyEdge(nodesFrom[0], firstLevelNode);
                        continue;
                    }
                    // Add other hierarchy edges based on temporal overlap of the first timestep
                    // Using the start time also covers single timestep parents
                    for (auto itFrom = nodesFrom.begin(), itTo = nodesTo.begin(); 
                        itFrom != nodesFrom.end() && itTo != nodesTo.end(); )
                    {
                        if (tree.nodes[*itFrom].startTime() < tree.nodes[*itTo].startTime())
                        {
                            itFrom++;
                        } else if (tree.nodes[*itFrom].startTime() > tree.nodes[*itTo].startTime())
                        {
                            itTo++;
                        } else {
                            tree.addHierarchyEdge(*itFrom, *itTo);
                            itTo++;
                            itFrom++;
                        }
                    }

                }
                else
                {
                    LogWarn("Hierarchical edge not in deaggregated");
                }
            }
        }

        tree.order.reserve(numLeaves);

        if (!order.empty())
        {
            // Get the corresponding nodes for each leaf and insert them into the order;
            for (auto leaf : order)
            {
                if (edgesHierarchy.find(leaf) != edgesHierarchy.end())
                {
                    LogError("Deaggregation: Non-leaf of the original tree was in its order");
                }
                auto it = nodesOrgingalToDeaggregated.find(leaf);
                if (it != nodesOrgingalToDeaggregated.end())
                {
                    std::vector<size_t> leafNodes = it->second;
                    for (auto newLeaf : leafNodes)
                    {
                        if(tree.edgesHierarchy.find(newLeaf) != tree.edgesHierarchy.end())
                        {
                            LogError("Deaggregation: Node corresponding to leaf in old tree is not leaf anymore in new tree");
                        }
                        tree.order.push_back(newLeaf);
                    }
                }
                else
                {
                    LogError("Deaggregation: A leaf was not deaggregated.");
                }
            }
        }
    }

    TemporalTree TemporalTree::aggregate() const
    {
        std::map<size_t, size_t> correspondences;
        
        // Functions for finding and uniting correspondences (union-find datastructure)
        std::function<size_t(size_t)> findCorrespondence = [&](size_t nodeIndex) -> size_t
        {
            if (correspondences[nodeIndex] == nodeIndex)
            {
                return nodeIndex;
            }

            return findCorrespondence(correspondences[nodeIndex]);
        };
        auto uniteCorrespondences = [&](size_t nodeIndexA, size_t nodeIndexB)
        {
            size_t correspondenceA = findCorrespondence(nodeIndexA);
            size_t correspondenceB = findCorrespondence(nodeIndexB);
            correspondences[correspondenceA] = correspondenceB;
        };
        // Initilize with self correspondance (make-set)
        for (size_t i(0);i<nodes.size();i++)
        {
            correspondences.insert(std::make_pair(i, i));
        }

        // Nodes correspond to each other if there is a connection between them and that connection 
        // contributes to neither a split nor a merge
        std::map<size_t, std::vector<size_t> > reverseEdges = this->getReverseEdges(this->edgesTime);

        for (std::map<size_t, std::vector<size_t> >::const_iterator it = this->edgesTime.begin();
            it != this->edgesTime.end(); it++)
        {
            // Just one outgoing edge from it->fist (between it->first and it->second[0])
            if (it->second.size() == 1)
            {
                // Check if there also is only one incoming edge for the to-part of the edge
                // i.e. it->first and it->second[0] are connected and it->first has no other
                // successor and it->second[0] has no other predecessor
                if (reverseEdges[it->second[0]].size() == 1)
                {
                    uniteCorrespondences(it->first, it->second[0]);
                }
            }
        }

        TemporalTree aggregatedTree = TemporalTree();

        // Maps node indices of this tree to the aggregated version
        std::map<size_t, size_t> indexMap;

        // Nodes and Edges for the aggregated version
        std::vector<TNode> aggregatedNodes;
        TAdjacency aggregatedEdgesHierarchy;
        TAdjacency aggregatedEdgesTime;

        // Add nodes 
        for (size_t nodeIndex = 0; nodeIndex < this->nodes.size(); nodeIndex++)
        {
            // Check if the corresponding node is already in the index map
            size_t correspondence = findCorrespondence(nodeIndex);
            std::map<size_t, size_t>::iterator it = indexMap.find(correspondence);
            // If it is not, create a new node for it
            if (it == indexMap.end())
            {
                // Copy the representative node (including all its values) from this tree to the aggregated one
                TNode aggregatedNode = TNode(nodes[correspondence]);
                aggregatedNodes.push_back(aggregatedNode);
                it = indexMap.insert(std::make_pair(correspondence, aggregatedNodes.size() - 1)).first;
            }
            // Update the information within the node, if the nodeIndex == correspondance 
            // (i.e. it is the representative of the node)
            // the we have just copied the entire node and do not need to add values here
            if (nodeIndex != correspondence)
            {
                // Put this node in the index map as well
                it = indexMap.insert(std::make_pair(nodeIndex, it->second)).first;
                TNode& aggregatedNode = aggregatedNodes[it->second];
                const TNode& thisNode = nodes[nodeIndex];
                // Add values 
                // TODO: Do we need to make sure the values are always consistent?
                aggregatedNode.values.insert(thisNode.values.begin(), thisNode.values.end());
            }
        }

        // Add hierarchical edges (can't use addHierarchyEdge function due to additional processing)
        for (auto it = this->edgesHierarchy.begin(); it != this->edgesHierarchy.end(); it++)
        {
            for (auto edgeTo : it->second)
            {
                size_t correspondenceFrom = findCorrespondence(it->first);
                size_t correspondenceToIndex = indexMap[findCorrespondence(edgeTo)];

                // Check if there are already outgoing edges for the from node
                auto itAggregated = aggregatedEdgesHierarchy.find(indexMap[correspondenceFrom]);

                if (itAggregated == aggregatedEdgesHierarchy.end())
                {   
                    // No edges have been added, thus we start a new vector
                    std::vector<size_t> edgesTo = { correspondenceToIndex };
                    aggregatedEdgesHierarchy.emplace(indexMap[correspondenceFrom], edgesTo);
                }
                else
                {
                    // Check if this edge already exists
                    auto itEdge = std::find(itAggregated->second.begin(), itAggregated->second.end(),
                        correspondenceToIndex);
                    // it does not: Create new edge
                    if (itEdge == itAggregated->second.end())
                    {
                        itAggregated->second.push_back(correspondenceToIndex);
                    }
                }
            }
        }

        // Add temporal edges if nodes do not correspond to each other
        for (auto it = this->edgesTime.begin(); it != this->edgesTime.end(); it++)
        {
            for (auto edgeTo : it->second)
            {
                size_t correspondenceFrom = findCorrespondence(it->first);
                size_t correspondenceTo = findCorrespondence(edgeTo);
                if (correspondenceFrom != correspondenceTo)
                {
                    // Check if there are already outgoing edges for the from node and either add an entry or modfiy the entry
                    auto itAggregated = aggregatedEdgesTime.find(indexMap[correspondenceFrom]);
                    if (itAggregated == aggregatedEdgesTime.end())
                    {
                        std::vector<size_t> edgesTo = { indexMap[correspondenceTo] };
                        aggregatedEdgesTime.insert(std::make_pair(indexMap[correspondenceFrom], edgesTo));
                    }
                    else
                    {
                        itAggregated->second.push_back(indexMap[correspondenceTo]);
                    }
                }
            }
        }

        aggregatedTree.nodes = aggregatedNodes;
        aggregatedTree.edgesHierarchy = aggregatedEdgesHierarchy;
        aggregatedTree.edgesTime = aggregatedEdgesTime;

        return aggregatedTree;

    }

    /**** Compute inner values ****/

    void TemporalTree::TNode::fillWithLeftNeighborInterpolation(const std::set<uint64_t>& times, std::map<uint64_t, float>& values)
    {
        // Fill in values for non-existent timesteps with left neighbor interpolation
        std::set<uint64_t>::const_iterator itTimes = times.find(values.begin()->first);
        std::map<uint64_t, float>::iterator itValues = values.begin();
        auto itEnd = std::next(times.find(values.rbegin()->first));
        while (itTimes != itEnd)
        {
            if (*itTimes == itValues->first)
            {
                itTimes++;
                itValues++;
            }
            // Insert the last value at times between the last and next
            // (left neighbor interpolation)
            else if (*itTimes < itValues->first)
            {
                values.emplace(*itTimes, std::prev(itValues, 1)->second);
                itTimes++;
            }
        }
    }

    bool TemporalTree::TNode::isOverlappingTemporally(uint64_t startTimeA, uint64_t endTimeA, uint64_t startTimeB, uint64_t endTimeB)
    {
        return
            //   |---|      (A)
            //|-|           (B) starts before A, ends before start of A: startTimeA > endTimeB (<= yields false)
            // |-|          (B) starts before A, ends at start of A: startTimeA == endTimeB
            // |---|        (B) starts before A, ends before A: startTimeA < endTimeB
            // |-----|      (B) starts before A, ends at end of A: startTimeA < endTimeA/B 
            // |-------|    (B) starts before A, ends after A: startTimeA < endTimeA
            //   |          (B) starts at start of A, ends at start of A: startTimeA/B == endTimeB 
            //   |-|        (B) starts at start of A, ends before A: startTimeA/B < endTimeB
            //   |---|      (B) starts at start of A, ends at end of A: startTimeA/B < endTimeA/B     
            //   |-----|    (B) starts at start of A, ends after A: startTimeA/B < endTimeA
            //    |-|       (B) starts after start of A, ends before A: startTimeB < endTimeB
            //    |--|      (B) starts after start of A, ends at end of A: startTimeB < endTimeA/B 
            //    |---|     (B) starts after start of A, ends after A: startTimeB < endTimeA
            //       |      (B) starts at end of A, ends at end of A: startTimeB == endTimeA/B 
            //       |-|    (B) starts at end of A, ends after A: startTimeB == endTimeA
            //        |-|   (B) starts after A, ends after A: startTimeB > endTimeA (<= yields false)
            (std::max(startTimeA, startTimeB) <= std::min(endTimeA, endTimeB));
    }

    void TemporalTree::computeAccumulated(const size_t subtreeIndex, const std::set<uint64_t>& times, 
        std::vector<bool>& processed, const bool justLeaves = true)
    {
        TemporalTree::TNode& subtreeNode = nodes[subtreeIndex];

        // This node has been processed before, no need to do it again
        if (processed[subtreeIndex])
        {
            return;
        }

        // Forget inner information if there is any
        if (justLeaves && !isLeaf(subtreeIndex))
        {
            for (auto& timeValuePair : subtreeNode.values)
            {
                timeValuePair.second = 0;
            }
        }

        // Do the computation for all children
        for (auto& child : this->getHierarchicalChildren(subtreeIndex))
        {
            // Will be called multiple times for children that change their parents
            computeAccumulated(child, times, processed, justLeaves);
            // Add the accumulated values of the children to this node
            // but only where the time overlaps (maximum of the start times and minimum of the end times)
            TemporalTree::TNode& childNode = nodes[child];
            
            auto childIt = childNode.values.find(
                std::max(subtreeNode.startTime(), childNode.startTime()));
             // Include the last value if this node does not split afterwards 
            auto childItLast = childNode.values.find(
                std::min(subtreeNode.endTime(), childNode.endTime()));
            if (getTemporalSuccessors(child).empty())
            {
                childItLast++; // Because then we will include childItLast
            }

            for (childIt; childIt != childItLast; childIt++)
            {
                subtreeNode.values[childIt->first] += childIt->second;
            }

        }

        // Fill values for missing timesteps with left-neighbor interpolation
        if (justLeaves && isLeaf(subtreeIndex) || !justLeaves)
        {
            subtreeNode.fillWithLeftNeighborInterpolation(times);
        }

        processed[subtreeIndex] = true;
  
    }

    std::vector<unsigned int> TemporalTree::computeComponents(const TAdjacency& Edges) const
    {
        // Initialize components with 0 
        std::vector<unsigned int> componentsMap(this->nodes.size(), 0) ;

        auto reverseEdges = getReverseEdges(Edges);

        std::function<void(size_t, bool, unsigned int)> processNode = [&](const size_t node, const bool forward, const unsigned int componentId)
        {
            const TAdjacency& currentEdges = forward ? Edges : reverseEdges;
            auto it = currentEdges.find(node);
            if (it != currentEdges.end())
            {
                for (auto edgeTo : it->second)
                {
                    // process the node at the end of the edgeb only 
                    // if it has not been visited either
                    if (componentsMap[edgeTo] == 0)
                    {
                        componentsMap[edgeTo] = componentId;
                        processNode(edgeTo, true, componentId);
                        processNode(edgeTo, false, componentId);
                    }
                }
            }

        };

        unsigned int componentsCounter = 0;

        // Iterate over nodes (that have not been visited)
        for (size_t node = 0; node < nodes.size(); node++)
        {
            if (componentsMap[node] == 0)
            {
                // Start a new component
                componentsCounter++;
                componentsMap[node] = componentsCounter;
                // Traverse all forward and backward edges for this node
                // Labelling all encountered nodes as part of the new component
                processNode(node, true, componentsCounter);
                processNode(node, false, componentsCounter);
            }
        }

        return componentsMap;

        // Go over the components map and create components
        // @todo: If we keep using only this, just create it while computing components
        /*std::vector<std::vector<size_t>>components (componentsCounter);
        componentsLifeTime = std::vector<std::pair<uint64_t, uint64_t>>(componentsCounter, 
            std::make_pair(std::numeric_limits<uint64_t>::max(), std::numeric_limits<uint64_t>::min()));
        for (size_t node = 0; node < nodes.size(); node++)
        {
            components[componentsMap[node]].push_back(node);
            // Node has an earlier start
            if (this->nodes[node].startTime() < componentsLifeTime[componentsMap[node]].first)
            {
                componentsLifeTime[componentsMap[node]].first = this->nodes[node].startTime();
            }
            // Node has a later end
            if (this->nodes[node].endTime() < componentsLifeTime[componentsMap[node]].second)
            {
                componentsLifeTime[componentsMap[node]].second = this->nodes[node].endTime();
            }
        }
        return components;*/
    }

    std::map<uint64_t, float> TemporalTree::computeAccumulatedRootOnly(const std::set<uint64_t>& times) const
    {
        std::map<uint64_t, float> result;
        auto leaves = getLeaves();
        for (auto leaf : leaves)
        {
            // Copy values of the leaf
            std::map<uint64_t, float> values = nodes[leaf].values;
            TNode::fillWithLeftNeighborInterpolation(times, values);
            for (auto timeValuePair : values)
            {
                // If there are temporal sucessors, skip the value, the sucessors first value will contribute to the sum
                if (timeValuePair.first == values.rbegin()->first && !getTemporalSuccessors(leaf).empty())
                {
                    continue;
                }
                result[timeValuePair.first] += timeValuePair.second;
            }
        }

        return result;
    }


    size_t TemporalTree::getConstraintClusters(std::vector<std::pair<int, int>>& Clusters) const
    {
        //Prepare memory
        const size_t NumNodes = nodes.size();
        Clusters.resize(NumNodes);
        std::fill(Clusters.begin(), Clusters.end(), std::make_pair<int, int>(-1, -1));
        int NumFound(0);

        //Get reversed time edges
        TAdjacency reversedEdgesTime = getReverseEdges(edgesTime);

        for (const auto& edge : edgesTime)
        {
            //Shorthands
            const size_t idFromLeft(edge.first);
            const std::vector<size_t>& idsToRight = edge.second;

            //Safety
            ivwAssert(!idsToRight.empty(), "Time map is empty.");
            if (idsToRight.empty()) continue;

            // This is an actual split
            if (idsToRight.size() > 1)
            {
                Clusters[idFromLeft].second = NumFound;

                for (auto idToRight : idsToRight)
                {
                    Clusters[idToRight].first = NumFound;
                }

                NumFound++;
            }
            else
            {
                auto leftForThatOneRight = reversedEdgesTime.find(idsToRight.front());
                if (leftForThatOneRight != reversedEdgesTime.end())
                {
                    // If it is a direct correspondance 
                    if (leftForThatOneRight->second.size() == 1)
                    {
                        Clusters[idsToRight.front()].first = NumFound;
                        Clusters[leftForThatOneRight->second.front()].second = NumFound;
                        NumFound++;
                    }
                }
            }
        }

        for (const auto& edge : reversedEdgesTime)
        {
            //Shorthands
            const size_t idToRight(edge.first);
            const std::vector<size_t>& idsFromLeft = edge.second;

            if (idsFromLeft.size() > 1)
            {
                Clusters[idToRight].first = NumFound;

                for (auto idFromLeft : idsFromLeft)
                {
                    Clusters[idFromLeft].second = NumFound;
                }

                NumFound++;
            }
        }

        /*std::vector<bool> bVisitedLeft(NumNodes, false);
        std::vector<bool> bVisitedRight(NumNodes, false);
        for(const auto& edge : edgesTime)
        {
            //Shorthands
            const size_t idFromLeft(edge.first);
            const std::vector<size_t>& idsToRight = edge.second;

            //Safety
            ivwAssert(!idsToRight.empty(), "Time map is empty.");
            if (idsToRight.empty()) continue;
            ivwAssert(bVisitedLeft[idFromLeft] == bVisitedRight[idsToRight.front()], "Visitation maps should be consistent.");

            if (bVisitedLeft[idFromLeft]) continue;

            //Start new component
            std::queue<std::pair<bool, size_t>> Q;
            Q.emplace(true, idFromLeft);
            bVisitedLeft[idFromLeft] = true;
            Clusters[idFromLeft].second = (int)NumFound;

            //Visit connected neighbors
            while (!Q.empty())
            {
                //Who is first?
                const auto& Front = Q.front();

                //The grass is always greener on the other side!
                const std::vector<size_t>& ToTheGreenerSide = Front.first ? edgesTime.at(Front.second) : reversedEdgesTime.at(Front.second);
                std::vector<bool>& bVisitedOnTheGreenerSide = Front.first ? bVisitedRight : bVisitedLeft;

                //Add the neighbors to the Queue
                for(const size_t& idNeigh : ToTheGreenerSide)
                {
                    if (!bVisitedOnTheGreenerSide[idNeigh])
                    {
                        Q.emplace(!Front.first, idNeigh);
                        bVisitedOnTheGreenerSide[idNeigh] = true;
                        if (Front.first)
                        {
                            Clusters[idNeigh].first = (int)NumFound;
                        }
                        else
                        {
                            Clusters[idNeigh].second = (int)NumFound;
                        }
                    }
                }

                //Remove current point from the queue
                Q.pop();
            }

            //Off to the next cluster.
            NumFound++;
        }*/

        return NumFound;
    }

    void TemporalTree::getMinMaxTime(const size_t nodeIndex, uint64_t& tMin, uint64_t& tMax) const
    {
        //Get the children of this node
        const auto itHierarchyEdges = edgesHierarchy.find(nodeIndex);

        //Is it a leaf or a parent?
        if (itHierarchyEdges == edgesHierarchy.end())
        {
            //Get the data values from the leaf
            auto& Values = nodes[nodeIndex].values;
            if (!Values.empty())
            {
                tMin = Values.begin()->first;
                tMax = Values.rbegin()->first;
            }
        }
        else
        {
            //Parents: visit children, ask them

            //Shorthand for the children
            const std::vector<size_t>& Children = itHierarchyEdges->second;

            //Call recursively, get times
            tMin = std::numeric_limits<uint64_t>::max();
            tMax = std::numeric_limits<uint64_t>::min();
            for(const size_t idChild : Children)
            {
                uint64_t tMinChild(std::numeric_limits<uint64_t>::max());
                uint64_t tMaxChild(std::numeric_limits<uint64_t>::min());
                getMinMaxTime(idChild, tMinChild, tMaxChild);

                //Update?
                if (tMinChild < tMin) tMin = tMinChild;
                if (tMaxChild > tMax) tMax = tMaxChild;
            }
        }
    }


    void TemporalTree::getMinMaxTimeShallow(const size_t nodeIndex, uint64_t& tMin, uint64_t& tMax) const
    {
        //Get the children of this node; or we take a reference to ourselves if we are a leaf
        const auto itHierarchyEdges = edgesHierarchy.find(nodeIndex);
        std::vector<size_t> JustMe;
        JustMe.push_back(nodeIndex);
        const std::vector<size_t>& Children = (itHierarchyEdges == edgesHierarchy.end()) ? JustMe : itHierarchyEdges->second;

        //Get the times of the children directly
        tMin = std::numeric_limits<uint64_t>::max();
        tMax = std::numeric_limits<uint64_t>::min();
        for(const size_t idChild : Children)
        {
            const auto& Values = nodes[idChild].values;
            if (!Values.empty())
            {
                if (Values.begin()->first < tMin) tMin = Values.begin()->first;
                if (Values.rbegin()->first > tMax) tMax = Values.rbegin()->first;
            }
        }
    }


    void TemporalTree::addDefaultTimesForParents(const size_t nodeIndex, uint64_t& tMin, uint64_t& tMax)
    {
        //Get the children of this node
        const auto itHierarchyEdges = edgesHierarchy.find(nodeIndex);

        //Is it a leaf or a parent?
        if (itHierarchyEdges == edgesHierarchy.end())
        {
            //Get the data values from the leaf
            auto& Values = nodes[nodeIndex].values;
            if (!Values.empty())
            {
                tMin = Values.begin()->first;
                tMax = Values.rbegin()->first;
            }
        }
        else
        {
            //Parents: visit children, ask them

            //Shorthand for the children
            const std::vector<size_t>& Children = itHierarchyEdges->second;

            //Call recursively, get times
            tMin = std::numeric_limits<uint64_t>::max();
            tMax = std::numeric_limits<uint64_t>::min();
            for(const size_t idChild : Children)
            {
                uint64_t tMinChild(std::numeric_limits<uint64_t>::max());
                uint64_t tMaxChild(std::numeric_limits<uint64_t>::min());
                addDefaultTimesForParents(idChild, tMinChild, tMaxChild);

                //Update?
                if (tMinChild < tMin) tMin = tMinChild;
                if (tMaxChild > tMax) tMax = tMaxChild;
            }

            //Set these times for this parent: overwrite values vector completely!
            auto& Values = nodes[nodeIndex].values;
            Values.clear();
            Values.emplace(tMin, 0.0f);
            Values.emplace(tMax, 0.0f);
        }
    }


    void TemporalTree::getTimes(const size_t subtreeIndex, std::set<uint64_t>& times) const
    {
        // Process all children of the node
        for (auto child : TemporalTree::getHierarchicalChildren(subtreeIndex))
        {
            getTimes(child, times);
        }
        const TemporalTree::TNode& subtreenode = this->nodes[subtreeIndex];

        // Process the times in this node itself
        for (auto& timeValuePair : subtreenode.values)
        {
            times.insert(timeValuePair.first);
        }
    }


    bool TemporalTree::checkConsistency() const
    {

        //Nodes with just a single value cause problems
        size_t nodesLessThanTwoValues(0);
        for (const auto& node : nodes)
        {
            if (node.values.size() < 2) nodesLessThanTwoValues++;
        }
        if (nodesLessThanTwoValues) LogInfo("There are " << nodesLessThanTwoValues << " nodes with less than two values.");

        //For temporal edges the associated time for the last value of the from node
        //and the first value of the to node are the same
        for (const auto& adjacencyNode : this->edgesTime)
        {
            size_t edgeFrom = adjacencyNode.first;
            for (auto edgeTo : adjacencyNode.second)
            {
                // At this point we have already checked that each node has >= two values
                if (nodes[edgeFrom].endTime() !=
                    nodes[edgeTo].startTime())
                {
                    LogError("The temporal edge ["<< edgeFrom << "," << edgeTo <<"] connects nodes with ending time" << nodes[edgeFrom].endTime() << " and "
                    << " begin time " << nodes[edgeTo].startTime() << ".")
                    return false;
                }
            }
        }

        //For the connected component withing one hierarchy level, the component must be connected to a 
        //single parent, the start and endtime need to be within that parents range. 

        // Data about components
        // Map for each node encoding which component it belongs to
        std::vector<unsigned int> componentsMap = computeComponents(this->edgesTime);
        size_t numComponents = *std::max_element(componentsMap.begin(), componentsMap.end());

        // Lifetime of the components
        std::vector<std::pair<uint64_t, uint64_t>> componentLifetimes (numComponents, 
        std::make_pair(std::numeric_limits<uint64_t>::max(), std::numeric_limits<uint64_t>::min()));

        std::vector<std::set<size_t>> componentEdgesToParents (numComponents);

        auto reverseEdgesHierarchy = this->getReverseEdges(this->edgesHierarchy);
        for (size_t node = 0; node < nodes.size(); node++)
        {
           // componentsMap has values [1, numComponents]
           auto componentId = componentsMap[node] - 1;
           auto parents = getEdgesFrom(node, reverseEdgesHierarchy);
           for (auto parent : parents)
           {
               componentEdgesToParents[componentId].insert(componentsMap[parent]-1);
           }
           // Node has an earlier start than so far computed value
           if (this->nodes[node].startTime() < componentLifetimes[componentsMap[node]-1].first)
           {
                componentLifetimes[componentId].first = this->nodes[node].startTime();
           }
           // Node has a later end than the so far computed value
           if (this->nodes[node].endTime() > componentLifetimes[componentId].second)
           {
               componentLifetimes[componentId].second = this->nodes[node].endTime();
           }
        }

        // There should only be a single root node at index 0
        if (!componentEdgesToParents[0].empty() || !getEdgesFrom(0, this->edgesTime).empty()
            || !getEdgesTo(0, this->edgesTime).empty())
        {
            LogError("The node at index 0 does not qualify as a root.");
            return false;
        }

        for (size_t component = 1; component < numComponents; component++)
        {
            // There needs to be exactly one parent (not zero, i.e. an additional root, or more)
            if (componentEdgesToParents[component].size() != 1)
            {
                LogError("There is a component that has none or more parents");
                return false;
            }

            // The times of the component need to overlap
            size_t parentComponent = *componentEdgesToParents[component].begin();
            if (componentLifetimes[parentComponent].first > componentLifetimes[component].first
                && componentLifetimes[parentComponent].second < componentLifetimes[component].second)
            {
                LogError("The child components lifeTime does not overlap with the lifetime of the parent.");
                return false;
            }
        }

        // Nodes that are connected by hierarchy edges have overlapping times 
        // Children can start before or end after a parent (if they split and merge)
        // 
        for (const auto& adjacencyNode : this->edgesHierarchy)
        {
            size_t edgeFrom = adjacencyNode.first;
            const TNode& parentNode = nodes[edgeFrom];
            for (auto edgeTo : adjacencyNode.second)
            {
                const TNode& childNode = nodes[edgeTo];

                // If both parent and children have at least two values (i.e. start and end time are different),
                // then the maximum of the start times should be strictly smaller than the minimum of the end times
                // Special cases occur if parent of child lives only for a single time step
                if (!TNode::isOverlappingTemporally(childNode.startTime(), childNode.endTime(), 
                    parentNode.startTime(), parentNode.endTime()))
                {
                    LogError("Hierarchy edge with non-overlapping time frame" <<
                        "(Child " << edgeTo << " [" << childNode.startTime() << "," <<
                        childNode.endTime() << "], Parent " << edgeFrom << 
                        " [" << parentNode.startTime() << "," << parentNode.endTime() << "])");
                    return false;
                }
            }
        }

        // @todo: I don't think components ensure consistency at splits and merges:
        // A child needs to have a single parent at all times 
        // With components we could have edges to both resulting nodes of a split 
        // without it causing a problem
        // Alternative: Look at aggregated parents of aggregated children and make sure that they are connected by
        // edges (Ensures both that the node always has a parent and we can check that the lifetime of that 
        // parent is ok)
        // Also we do not ensure that merged / (split can be treated the same way) 
        // nodes have the same parent like this e.g. 
        // two siblings have children that merge but they are in the same component because they also merge later
        // We need to make sure that all merged nodes and the merge result have the same parent and the time point
        // of the merge

        return true;
    }

    uint64_t TemporalTree::TNode::startTime() const
    {
        if (!values.empty())
        {
            return values.cbegin()->first;
        }
        return std::numeric_limits<uint64_t>::max();
    }

    uint64_t TemporalTree::TNode::endTime() const
    {
        if (!values.empty())
        {
            return values.crbegin()->first;
        }
        return std::numeric_limits<uint64_t>::min();
    }

    float TemporalTree::TNode::getValueAt(const uint64_t time) const
    {
        // Find the value with a time strictly larger then the given one
        // The value to be returned is associated to the element before
        std::map<uint64_t, float>::const_iterator it = this->values.upper_bound(time);

        // Given time is larger than any of the values in the map (or equal to the end time)
        if (it == this->values.end())
        {
            if (time == this->values.rbegin()->first)
            {
                return this->values.rbegin()->second;
            }
            return 0.0;
        }
        // Given time is smaller than any of the values in the map (the node does not exist yet)
        if (it == this->values.begin() && time != this->startTime())
        {
            return 0.0;
        }

        // Get the element before (corresponds to left neighbor interpolation)
        it--;
        return it->second;
    }


} // namespace kth
} // namespace
