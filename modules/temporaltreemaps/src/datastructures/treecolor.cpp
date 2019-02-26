/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Saturday, January 06, 2018 - 16:09:05
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <modules/temporaltreemaps/datastructures/treecolor.h>
#include <inviwo/core/util/colorconversion.h>

namespace inviwo
{
namespace kth
{

void treecolor::spreadColorOverTree(const TemporalTree::TAdjacency& edges, TemporalTree& tree, 
    const size_t nodeIndex,
    const float rangeStart, const float rangeEnd, 
    bool alternate, uint8_t depth, const int colorFrom, const int colorTo)
{
    tree.nodes[nodeIndex].color = color::hsv2rgb(vec3((rangeStart + rangeEnd) / 2.0, 1.0, 1.0));

    const auto itHierarchyEdges = edges.find(nodeIndex);

    int sign = (rangeEnd - rangeStart < 0) ? -1 : ((rangeEnd - rangeStart > 0) ? 1 : 0);

    if (depth < colorFrom || (colorTo != -1 && depth +1 > colorTo))
    {
        alternate = false;
    }

    //not a leaf -> process children?
    if (itHierarchyEdges != edges.end())
    {
        float rangeStartChild = rangeStart;

        auto& children = itHierarchyEdges->second;
        float fractionPerChild = std::fabs(rangeStart - rangeEnd) / children.size();

        if (depth < colorFrom || (colorTo != -1 && depth + 1 > colorTo))
        {
            // We are not spreading color more in this case -> range stays the same
            fractionPerChild *= children.size();
        }

        bool evenChild = true;

        for (auto child : children)
        {
            float rangeEndChild = rangeStartChild + sign * fractionPerChild;
            if (evenChild)
            {
                treecolor::spreadColorOverTree(edges, tree, child, 
                    rangeStartChild, rangeEndChild, 
                    alternate, depth+1, colorFrom, colorTo);
            }
            else
            {
                treecolor::spreadColorOverTree(edges, tree, child, 
                    rangeEndChild, rangeStartChild, 
                    alternate, depth+1, colorFrom, colorTo);
            }
            rangeStartChild = rangeEndChild;

            if (depth < colorFrom || (colorTo != -1 && depth + 1 > colorTo))
            {
                // Again no further division of color
                rangeStartChild = rangeStart;
            }

            evenChild = !evenChild || !alternate;
        }
    }
}

void treecolor::traverseToLeavesForColor(TemporalTree & tree, size_t nodeIndex,
    float rangeStart, float rangeEnd, 
    bool alternate, uint64_t startTime, uint64_t endTime, uint8_t depth, float rangeDecay,
    const int colorFrom, const int colorTo, const std::function <vec3(float)>& sampleColor)
{
    // Shorthands
    TemporalTree::TNode& node = tree.nodes[nodeIndex];
    auto& colors = node.colors;
    auto& cushion = node.cushion;

    vec3 color = sampleColor((rangeStart + rangeEnd) / 2.0f);

    // A parent might have 0 values that extend over the time of all its children
    // but we will have no limit information for these
    startTime = std::max(startTime, cushion.begin()->first);
    endTime = std::min(endTime, cushion.rbegin()->first);

    // Fill colors
    auto itEnd = std::next(cushion.find(endTime));
    for (auto it = cushion.find(startTime); it != itEnd; it++)
    {
        // We might overwrite things here
        auto inserted = colors.emplace(it->first, vec4(color, 1.0f));
    }

    const auto itHierarchyEdges = tree.edgesHierarchy.find(nodeIndex);

    int sign = (rangeEnd - rangeStart < 0) ? -1 : ((rangeEnd - rangeStart > 0) ? 1 : 0);

    float decayAtOneEnd = std::abs(rangeEnd - rangeStart) * (1.0f - rangeDecay) / 2.0f;

    rangeStart += sign* decayAtOneEnd;
    rangeEnd -= sign* decayAtOneEnd;

    if (depth < colorFrom || (colorTo != -1 && depth + 1 > colorTo))
    {
        alternate = false;
    }

    //not a leaf -> process children?
    if (itHierarchyEdges != tree.edgesHierarchy.end())
    {
        float rangeStartChild = rangeStart;

        auto& children = itHierarchyEdges->second;
        float fractionPerChild = std::fabs(rangeStart - rangeEnd) / children.size();

        if (depth < colorFrom || (colorTo != -1 && depth + 1 > colorTo))
        {
            fractionPerChild *= children.size();
        }

        bool evenChild = true;

        for (auto child : children)
        {
            TemporalTree::TNode& childNode = tree.nodes[child];
            // Find the time range for which this is the child
            uint64_t startTimeChild = std::max(startTime, childNode.startTime());
            uint64_t endTimeChild = std::min(endTime, childNode.endTime());

            if (startTimeChild >= endTimeChild && startTimeChild != endTimeChild)
            {
                continue;
            }

            float rangeEndChild = rangeStartChild + sign * fractionPerChild;
            if (evenChild)
            {
                treecolor::traverseToLeavesForColor(tree, child,
                    rangeStartChild, rangeEndChild,
                    alternate, startTimeChild, endTimeChild, depth + 1, rangeDecay, colorFrom, colorTo, sampleColor);
            }
            else
            {
                treecolor::traverseToLeavesForColor(tree, child,
                    rangeEndChild, rangeStartChild,
                    alternate, startTimeChild, endTimeChild, depth + 1, rangeDecay, colorFrom, colorTo, sampleColor);
            }
            rangeStartChild = rangeEndChild;

            if (depth < colorFrom || (colorTo != -1 && depth + 1 > colorTo))
            {
                rangeStartChild = rangeStart;
            }

            evenChild = !evenChild || !alternate;
        }
    }
}

} // namespace kth
} // namespace