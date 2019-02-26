/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Tuesday, March 27, 2018 - 15:05:01
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <modules/temporaltreemaps/datastructures/constraint.h>

namespace inviwo
{
namespace kth
{

namespace constraint
{
    void ConstraintsStatistic::clear()
    {
        unhappyLeaves.clear();
        std::fill(fulfilledByLevelMergeSplit.begin(), fulfilledByLevelMergeSplit.end(), 0);
        std::fill(fulfilledByLevelHierarchy.begin(), fulfilledByLevelHierarchy.end(), 0);
    }

    size_t ConstraintsStatistic::numFulfilledHierarchyConstraints() const
    {
        return numConstraints(fulfilledByLevelHierarchy);
    }

    size_t ConstraintsStatistic::numFulFilledMergeSplitConstraints() const
    {
        return numConstraints(fulfilledByLevelMergeSplit);
    }

    void ConstraintsStatistic::update(const Constraint & constraint)
    {
        if (constraint.fulfilled)
        {
            auto& levelStatistic = constraint.type == Hierarchy ?
                fulfilledByLevelHierarchy : fulfilledByLevelMergeSplit;
            if (levelStatistic.size() < constraint.level + 1)
            {
                levelStatistic.resize(constraint.level + 1);
                levelStatistic[constraint.level] = 0;
            }
            levelStatistic[constraint.level]++;
        }
        else
        {
            unhappyLeaves.insert(constraint.leaves.begin(), constraint.leaves.end());
        }
    }

    size_t numConstraints(const std::vector<size_t>& numByLevel)
    {
        size_t sum(0);
        for (auto number : numByLevel)
        {
            sum += number;
        }
        return sum;
    }

    bool isFulFilled(Constraint& constraint, std::shared_ptr<const TemporalTree> tree, const TemporalTree::TTreeOrder& order,
        const TemporalTree::TTreeOrderMap& orderMap)
    {
        size_t minOrder(order.size()); // numbere of leaves is maximum order
        size_t maxOrder(0); // 0 is minimum order

        // Record minimum and maximum order index for each leaf
        for (const auto leaf : constraint.leaves)
        {
            const auto mappedTo = orderMap.at(leaf);
            if (mappedTo < minOrder) minOrder = mappedTo;
            if (mappedTo > maxOrder) maxOrder = mappedTo;
        }

        // Check that the leaves are all together
        int NumOverlap((int)constraint.leaves.size());
        if (int(maxOrder) - int(minOrder) + 1 == NumOverlap)
        {
            constraint.fulfilled = true;
            return true;
        }
        else
        {
            for (size_t r(minOrder); r <= maxOrder&&r<order.size() && NumOverlap >= 0; r++)
            {
                //A leaf in the drawing area; may not be ours. If it is not ours, but it overlaps
                //then we do not fulfill the hierarchy constraint.

                //Leaf's time

                // Note: This means that if a node has no children at some point, still drawing children 
                // within that parent would warrant a violation of the hierarchy constraint for that parent
                // Therefore we need to split up parent A: splitTemporalLeaves
                // (--- child of parent A, xxx child of parent B)
                // |------||  A has no children  |  
                // |-------|   |xxxxxxxx|        |--------|
                // Similarly if other leaves start or end at the exact same time as the parent under 
                // investigation ends (for leaf starting) or starts, this will lead to a violation as well
                //     |--------|
                // |---| or     |-----|
                // We need to exclude this case for hierarchy constraints, for merge/split constraints it is still relevant 
                if (isOverlappingWithConstraint(tree->nodes[order[r]], constraint))
                {
                    NumOverlap--;
                }

            }
            ivwAssert(NumOverlap <= 0, "Missed a child? How? Not ok!");
            if (NumOverlap == 0)
            {
                constraint.fulfilled = true;
                return true;
            }
        }

        constraint.fulfilled = false;
        return false;
    }

    size_t numFulfilledConstraints(std::shared_ptr<const TemporalTree> tree,
        const TemporalTree::TTreeOrder& order, const TemporalTree::TTreeOrderMap& orderMap,
        std::vector<Constraint>& constraints, ConstraintsStatistic& statistic)
    {
        size_t numFullfilled = 0;

        // For each constraint, make sure leaves are together at the time of the constraint
        for (auto& constraint : constraints)
        {
            if (isFulFilled(constraint, tree, order, orderMap))
            {
                numFullfilled++;
            }
            statistic.update(constraint);
        }

        return numFullfilled;
    }

    void extractMergeSplitConstraints(std::shared_ptr<const TemporalTree> tree,
        std::vector<Constraint>& constraints, std::vector<size_t>& numByLevel)
    {
        std::vector<std::pair<int, int>> constraintsForAllNodes;
        //const size_t numConstraints = tree->getConstraintClusters(constraintsForAllNodes);
        const size_t numConstraintsAlreadyPresent = constraints.size();

        //constraints.resize(numConstraintsAlreadyPresent + numConstraints);

	std::vector<std::pair<size_t, size_t>> numLeftRight(1, {0,0});

	//Get reversed time edges
	TemporalTree::TAdjacency reversedEdgesTime = tree->getReverseEdges(tree->edgesTime);

	size_t constraintId = 0;
	bool addConstraint = false;
	for (const auto& edge : tree->edgesTime)
	{
	    addConstraint = false;
	    Constraint newConstraint;
			
	    //Shorthands
	    const size_t idFromLeft(edge.first);

	    newConstraint.level= tree->depthWithReverse(idFromLeft);

	    std::set<size_t> leaves;
	    const auto& time = tree->nodes[idFromLeft].endTime();
	    tree->getLeaves(idFromLeft, time, time, time, time, leaves);
	    newConstraint.leaves.insert(leaves.begin(), leaves.end());
	    newConstraint.startTime = time;
	    newConstraint.endTime = time;
	    newConstraint.fulfilled = false;
	    numLeftRight[constraintId].first++;

	    //Safety
	    const std::vector<size_t>& idsToRight = edge.second;
	    ivwAssert(!idsToRight.empty(), "Time map is empty.");
	    if (idsToRight.empty()) continue;

	    // This is an actual split
	    if (idsToRight.size() > 1)
	    {
		for (auto idToRight : idsToRight)
		{
		    leaves.clear();
		    tree->getLeaves(idToRight, time, time, time, time, leaves);
		    newConstraint.leaves.insert(leaves.begin(), leaves.end());
		    numLeftRight[constraintId].second++;
		    addConstraint = true;
		}

	    }
	    else
	    {
		    auto leftForThatOneRight = reversedEdgesTime.find(idsToRight.front());
		    if (leftForThatOneRight != reversedEdgesTime.end())
		    {
			    // If it is a direct correspondance 
			    if (leftForThatOneRight->second.size() == 1)
			    {
				    leaves.clear();
				    tree->getLeaves(idsToRight.front(), time, time, time, time, leaves);
				    newConstraint.leaves.insert(leaves.begin(), leaves.end());
				    numLeftRight[constraintId].second++;
				    addConstraint = true;
			    }
		    }
	    }

	    constraintId++;

	    if (!addConstraint || newConstraint.leaves.size() < 2) {
		    constraintId--;
		    numLeftRight[constraintId] = { 0,0 };
		    continue;
	    };

	    constraints.push_back(newConstraint);
	    numLeftRight.emplace_back(0,0);
	}

	for (const auto& edge : reversedEdgesTime)
	{
		addConstraint = false;
		Constraint newConstraint;

		//Shorthands
		const size_t idToRight(edge.first);

		newConstraint.level = tree->depthWithReverse(idToRight);

		std::set<size_t> leaves;
		const auto& time = tree->nodes[idToRight].startTime();
		tree->getLeaves(idToRight, time, time, time, time, leaves);
		newConstraint.leaves.insert(leaves.begin(), leaves.end());
		newConstraint.startTime = time;
		newConstraint.endTime = time;
		newConstraint.fulfilled = false;
		numLeftRight[constraintId].second++;

		const std::vector<size_t>& idsFromLeft = edge.second;
		ivwAssert(!idsFromLeft.empty(), "Time map is empty.");
		if (idsFromLeft.empty()) continue;

		if (idsFromLeft.size() > 1)
		{
			for (auto idFromLeft : idsFromLeft)
			{
				leaves.clear();
				tree->getLeaves(idFromLeft, time, time, time, time, leaves);
				newConstraint.leaves.insert(leaves.begin(), leaves.end());
				numLeftRight[constraintId].first++;
				addConstraint = true;
			}
		}

		constraintId++;

		if (!addConstraint || newConstraint.leaves.size() < 2) {
			constraintId--;
			numLeftRight[constraintId] = { 0,0 };
			continue;
		};

		constraints.push_back(newConstraint);
		numLeftRight.emplace_back(0, 0);
	}

	size_t numConstraints = constraintId;

        for (constraintId=0; constraintId < numConstraints; constraintId++)
        {
			size_t level = constraints[numConstraintsAlreadyPresent + constraintId].level;
            if (numByLevel.size() < level + 1)
            {
                numByLevel.resize(level + 1);
            }
            numByLevel[level]++;

            if (numLeftRight[constraintId].first == 1 && numLeftRight[constraintId].second > 1)
            {
                constraints[numConstraintsAlreadyPresent + constraintId].type = Split;
            }
            else if (numLeftRight[constraintId].first > 1 && numLeftRight[constraintId].second == 1)
            {
                constraints[numConstraintsAlreadyPresent + constraintId].type = Merge;
            }
            else
            {
                constraints[numConstraintsAlreadyPresent + constraintId].type = MergeSplit;
            }
        }
    }

    void extractHierarchyConstraints(std::shared_ptr<const TemporalTree> tree,
        std::vector<Constraint>& constraints, std::vector<size_t>& numByLevel)
    {
        for (size_t nodeIndex = 1; nodeIndex < tree->nodes.size(); nodeIndex++)
        {
            // Leafs by themselves do not impose a constraints
            if (tree->isLeaf(nodeIndex))
            {
                continue;
            }
            // Get nodes leaves and times
            uint64_t nodeStartTime = tree->nodes[nodeIndex].startTime();
            uint64_t nodeEndTime = tree->nodes[nodeIndex].endTime();

            std::set<size_t> leaves;
            tree->getLeaves(nodeIndex, nodeStartTime, nodeEndTime, nodeStartTime, nodeEndTime, leaves);
            // No constraints for single leafs
            if (leaves.size() < 2)
            {
                continue;
            }

            Constraint constraint;
            constraint.leaves = leaves;
            constraint.type = Hierarchy;
            constraint.startTime = nodeStartTime;
            constraint.endTime = nodeEndTime;
            constraint.level = tree->depthWithReverse(nodeIndex);

            if (numByLevel.size() < constraint.level + 1)
            {
                numByLevel.resize(constraint.level + 1);
            }
            numByLevel[constraint.level]++;

            constraints.push_back(constraint);
        }
    }

    bool isOverlappingWithConstraint(const TemporalTree::TNode& leaf, const Constraint & constraint)
    {
        const uint64_t overlapStart = std::max(constraint.startTime, leaf.startTime());
        const uint64_t overlapEnd = std::min(constraint.endTime, leaf.endTime());
        return overlapStart < overlapEnd ||
            // Unless we have a single timestep hierarchy or merge Constraints
            (overlapStart == overlapEnd &&
            (constraint.startTime == constraint.endTime || leaf.startTime() == leaf.endTime()));
    }

}

} // namespace
} // namespace

