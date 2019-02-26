/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Tuesday, March 27, 2018 - 15:05:01
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
#include <modules/temporaltreemaps/datastructures/treeorder.h>

namespace inviwo
{
namespace kth
{

namespace constraint
{

    enum ConstraintType
    {
        Merge,
        Split,
        MergeSplit,
        Hierarchy,
        None
    };


    struct Constraint
    {
        std::set<size_t> leaves;
        uint64_t startTime = 0;
        uint64_t endTime = std::numeric_limits<uint64_t>::max();
        size_t level = 0;
        ConstraintType type = None;
        bool fulfilled = false;
    };

    struct ConstraintsStatistic
    {
        std::set<size_t> unhappyLeaves;
        std::vector<size_t> fulfilledByLevelMergeSplit;
        std::vector<size_t> fulfilledByLevelHierarchy;

        void clear();

        void update(const Constraint& constraint);

        size_t numFulfilledHierarchyConstraints() const;

        size_t numFulFilledMergeSplitConstraints() const;

    };

    /// Sums over the given vector
    size_t numConstraints(const std::vector<size_t>& numByLevel);

    /// CHecks if the given constraints if fulfilled
    bool isFulFilled(Constraint& constraint, std::shared_ptr<const TemporalTree> tree,
        const TemporalTree::TTreeOrder& order, const TemporalTree::TTreeOrderMap& orderMap);

    /// Get the number of fulfilled constraints, where constraints are given as a set of leaves and a time interval
    /// at which the constraint has to be fulfilled
    size_t numFulfilledConstraints(std::shared_ptr<const TemporalTree> tree,
        const TemporalTree::TTreeOrder& order, const TemporalTree::TTreeOrderMap& orderMap,
        std::vector<Constraint>& constraints, ConstraintsStatistic& statistic);

    void extractMergeSplitConstraints(std::shared_ptr<const TemporalTree> tree,
        std::vector<Constraint>& constraints, std::vector<size_t>& numByLevel);

    void extractHierarchyConstraints(std::shared_ptr<const TemporalTree> tree,
        std::vector<Constraint>& constraints, std::vector<size_t>& numByLevel);

    bool isOverlappingWithConstraint(const TemporalTree::TNode& leaf, const Constraint& constraint);

}

} // namespace
} // namespace
