/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Köpp
 *  Init    : Thursday, November 23, 2017 - 16:54:12
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <modules/temporaltreemaps/datastructures/treeorder.h>

namespace inviwo
{
namespace kth
{

namespace treeorder
{

    void orderAsInserted(TemporalTree::TTreeOrder& order, const TemporalTree& tree)
    {
        auto leaves = tree.getLeaves();

        order.reserve(leaves.size());

        for (auto leaf : leaves)
        {
            order.push_back(leaf);
        }
    }

    void dfs(size_t nodeIndex, std::vector<bool>& visited, TemporalTree::TTreeOrder& order, const TemporalTree& tree,
        const TemporalTree::TAdjacency& edges)
    {
        visited[nodeIndex] = true;

        if (tree.isLeaf(nodeIndex))
        {
            order.push_back(nodeIndex);
        }

        const auto itHierarchyEdges = edges.find(nodeIndex);

        //not a leaf -> process children?
        if (itHierarchyEdges != edges.end())
        {
            auto& children = itHierarchyEdges->second;
            for (auto child : children)
            {
                if (!visited[child])
                {
                    dfs(child, visited, order, tree, edges);
                }
            }
        }
    }

    void orderAsDepthFirst(TemporalTree::TTreeOrder& order, const TemporalTree& tree, const TemporalTree::TAdjacency& edges)
    {
        std::vector<bool> visited(tree.nodes.size(), false);
        dfs(0, visited, order, tree, edges);
    }

    bool fitsWithTree(const TemporalTree& tree, const TemporalTree::TTreeOrder& leafOrder)
    {
        auto leaves = tree.getLeaves();
        
        // if sizes already mismatch, not all leaves can be contained
        if (leaves.size() != leafOrder.size())
        {
            return false;
        }

        // Check that all the leaves are part of the sort vector
        std::set<size_t> indexSetOrder (leafOrder.begin(), leafOrder.end());

        for (auto leaf : leaves)
        {
            if (indexSetOrder.find(leaf) == indexSetOrder.end())
            {
                return false;
            }
        }

        return true;
    }

    void sortNodesByOrder(const TemporalTree::TTreeOrderMap& orderMap, const TemporalTree& tree, std::vector<size_t>& nodeIndices)
    {
        std::sort(nodeIndices.begin(), nodeIndices.end(),
            [&orderMap, &tree](const size_t a, const size_t b) -> bool
        {
            auto itIndexA = orderMap.find(a);
            auto itIndexB = orderMap.find(b);
            // If either index a or b are not in the order, we cannot sort the given
            ivwAssert(itIndexA != orderMap.end() && itIndexB != orderMap.end(), 
                "We cannot sort nodes when their indices are not in the given order.");

            // Compare by sort indices (first cases should only occur when we are comparing non-leaves)
            if (itIndexA->second == itIndexB->second)
            {
                // Sort by beginning times
                return tree.nodes[a].startTime() < tree.nodes[b].startTime();
            }
            return itIndexA->second < itIndexB->second;
        });
    }

    TemporalTree::TTreeOrderMap expandToFullTree(const TemporalTree& tree, const TemporalTree::TTreeOrderMap& leafOrderMap)
    {
        TemporalTree::TTreeOrderMap treeOrderMap(leafOrderMap);
        setToMinInChildren(0, tree, treeOrderMap);
        return treeOrderMap;
    }

    void toSimpleOrder(TemporalTree::TTreeOrder& order, const TemporalTree::TTreeOrderMap& orderMap)
    {
        order.resize(orderMap.size());

        for (const auto indexSortIndexPair : orderMap)
        {
            order[indexSortIndexPair.second] = indexSortIndexPair.first;
        }
    }

    void toOrderMap(TemporalTree::TTreeOrderMap& orderMap, const TemporalTree::TTreeOrder& order)
    {
        for (size_t index = 0; index < order.size(); index++)
        {
            orderMap.insert_or_assign(order[index], index);
        }
    }

    //@todo: Might only work for no edgecrossings??
    size_t setToMinInChildren(const size_t nodeIndex, const TemporalTree& tree, TemporalTree::TTreeOrderMap& orderMap)
    {
        const auto itHierarchyEdges = tree.edgesHierarchy.find(nodeIndex);

        //Is it a leaf or a parent?
        // Leaf only has one order value: the one already given in the order
        if (itHierarchyEdges != tree.edgesHierarchy.end())
        {
            //Parents: visit children, ask them

            //Shorthand for the children
            const std::vector<size_t>& Children = itHierarchyEdges->second;

            //Call recursively, order for children
            size_t orderMin = std::numeric_limits<size_t>::max();
            for (const size_t idChild : Children)
            {
                size_t orderMinChild = setToMinInChildren(idChild, tree, orderMap);
                //Update?
                if (orderMinChild < orderMin) orderMin = orderMinChild;
            }
            orderMap[nodeIndex] = orderMin;
        }
        return orderMap[nodeIndex];
    }
}

} // namespace kth
} // namespace

