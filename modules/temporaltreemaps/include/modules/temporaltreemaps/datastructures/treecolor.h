/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Saturday, January 06, 2018 - 16:09:05
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

namespace treecolor
{ 

    void spreadColorOverTree(const TemporalTree::TAdjacency& edges, TemporalTree& tree, const size_t nodeIndex,
        const float rangeStart, const float rangeEnd, bool alternate, uint8_t depth, 
        const int colorFrom, const int colorTo);

    void traverseToLeavesForColor(TemporalTree& tree, size_t nodeIndex,
        float rangeStart, float rangeEnd, bool alternate,
        uint64_t startTime, uint64_t endTime, uint8_t depth, float rangeDecay, const int colorFrom, const int colorTo, 
        const std::function <vec3(float)>& sampleColor
    );

};

} // namespace kth
} // namespace
