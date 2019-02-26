/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Köpp
 *  Init    : Thursday, November 23, 2017 - 16:54:12
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#pragma once

#include <modules/temporaltreemaps/temporaltreemapsmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/temporaltreemaps/datastructures/tree.h>

namespace inviwo
{
namespace kth
{

namespace treeorder
{
    /// Use the indices of the tree as the order directly
    void orderAsInserted(TemporalTree::TTreeOrder& order, const TemporalTree& tree);

    /// Traverse the given tree depth first according to the 
    /// given edges (might not be the temporal or hierarchical edges of that tree)
    void orderAsDepthFirst(TemporalTree::TTreeOrder& order, const TemporalTree& tree, const TemporalTree::TAdjacency& edges);

    /// Checks if the order contains every leaf
    bool fitsWithTree(const TemporalTree& tree, const TemporalTree::TTreeOrder& leafOrder);

    /// Sorts a vector of nodes by the given order map
    void sortNodesByOrder(const TemporalTree::TTreeOrderMap& order, const TemporalTree& tree, std::vector<size_t>& nodeIndices);

    TemporalTree::TTreeOrderMap expandToFullTree(const TemporalTree& tree, const TemporalTree::TTreeOrderMap& leafOrder);

    void toSimpleOrder(TemporalTree::TTreeOrder& order, const TemporalTree::TTreeOrderMap& orderMap);

    void toOrderMap(TemporalTree::TTreeOrderMap& orderMap, const TemporalTree::TTreeOrder& order);

    size_t setToMinInChildren(const size_t nodeIndex, const TemporalTree& tree, TemporalTree::TTreeOrderMap& orderMap);

} 

} // namespace kth
} // namespace
