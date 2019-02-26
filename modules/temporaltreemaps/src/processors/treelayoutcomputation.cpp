/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Saturday, January 06, 2018 - 15:57:52
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */


#include <inviwo/core/util/utilities.h>
#include <modules/temporaltreemaps/processors/treelayoutcomputation.h>
#include <modules/temporaltreemaps/datastructures/treeorder.h>

namespace inviwo
{
namespace kth
{

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TemporalTreeLayoutComputation::processorInfo_
{
    "org.inviwo.TemporalTreeLayoutComputation",      // Class identifier
    "Tree Layout Computation",                // Display name
    "Temporal Tree",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};

const ProcessorInfo TemporalTreeLayoutComputation::getProcessorInfo() const
{
    return processorInfo_;
}


TemporalTreeLayoutComputation::TemporalTreeLayoutComputation()
    :Processor()
    , portInTree("inTree")
    , portOutTree("outTree")
    , propSpaceFilling("spaceFilling", "Space Filling (Normalize Timestepwise)", true)
    , propMaximum("setMax", "Manual Maximum", 0, 0)
    , propActualMaximum("actualMax", "Maximum", "")
    , propRenderInfo("renderInfo", "Render Info")
    , propUnixTime("unixTime", "Unix Time", false)
    , propTimeMinMax("timeMinxMax", "Time", 0, std::numeric_limits<float>::max())
    , propValueMinMax("vlaueMinMax", "Value", 0, std::numeric_limits<float>::max())
{
    // Ports
    addPort(portInTree);
    addPort(portOutTree);

    addProperty(propSpaceFilling);

    addProperty(propMaximum);
    addProperty(propActualMaximum);

    propSpaceFilling.onChange([&]()
    {
        if (propSpaceFilling)
        {
            util::hide(propActualMaximum, propMaximum);
        }
        else
        {
            util::show(propActualMaximum, propMaximum);
        }
    });

    propActualMaximum.setReadOnly(true);

    addProperty(propRenderInfo);
    propRenderInfo.addProperty(propUnixTime);
    propRenderInfo.addProperty(propTimeMinMax);
    propRenderInfo.addProperty(propValueMinMax);

    propTimeMinMax.setSemantics(PropertySemantics::Text);
    propValueMinMax.setSemantics(PropertySemantics::Text);
}

 
void TemporalTreeLayoutComputation::process()
{
    // Get tree
    std::shared_ptr<const TemporalTree> pInTree = portInTree.getData();

    std::shared_ptr<TemporalTree> pOutTree = std::make_shared<TemporalTree>(TemporalTree(*pInTree));

    TemporalTree::TTreeOrder& order = pOutTree->order;
    if (!treeorder::fitsWithTree(*pOutTree, order))
    {
        LogProcessorError("Order does not fit with the tree.");
        return;
    }

    std::map<uint64_t, float> mapForNormalization;
    std::set<uint64_t> times = pOutTree->getTimes();
    mapForNormalization = pOutTree->computeAccumulatedRootOnly(times);

    TemporalTree::TDrawingLimitMap upperLimitCurrent;
    float maxValue = std::numeric_limits<float>::min();
    for (auto timeValuePair : mapForNormalization)
    {
        upperLimitCurrent.emplace(timeValuePair.first ,std::make_pair(0.0f, 0.0f));
        if (timeValuePair.second > maxValue)
        {
            maxValue = timeValuePair.second;
        }
    }

    if (std::fabs(maxValue) < std::numeric_limits<float>::epsilon())
    {
        LogProcessorError("Maximum value is basically 0.");
        return;
    }

    std::ostringstream ss;
    ss << maxValue;
    propActualMaximum.set(ss.str());
    propMaximum.setMaxValue(10*maxValue);

    if (propMaximum < maxValue)
    {
        propMaximum.set(maxValue);
    }

    if (propUnixTime)
    {
        time_t ttMin = *times.begin();
        tm tMin = *localtime(&ttMin);
        propTimeMinMax.setStart(tMin.tm_year + 1900 + tMin.tm_yday / 365.0);
        time_t ttMax = *times.rbegin();
        tm tMax = *localtime(&ttMax);
        propTimeMinMax.setEnd(tMax.tm_year + 1900 + tMax.tm_yday / 365.0);
    } else
    {
        propTimeMinMax.setStart(double(*times.begin()));
        propTimeMinMax.setEnd(double(*times.rbegin()));
    }

    propValueMinMax.setStart(0);
    propValueMinMax.setEnd(propSpaceFilling ? 1.0 : propMaximum);

    // Compute reverse edges if they have not been computed earlier
    if(!pOutTree->edgesHierarchy.empty() && pOutTree->reverseEdgesHierachy.empty())
    {
        pOutTree->computeReverseEdges();
    }

    for (auto leaf : order)
    {
        TemporalTree::TNode& leafNode = pOutTree->nodes[leaf];
        // Shorthand for the values for this band (expand these as well)
        std::map<uint64_t, float>& expandedValues = leafNode.values;
        TemporalTree::TNode::fillWithLeftNeighborInterpolation(times, expandedValues);

        uint64_t tMinLeaf = leafNode.startTime();
        uint64_t tMaxLeaf = leafNode.endTime();

        // Set the lower limit for the leaf
        copyLimitInBetween(upperLimitCurrent, leafNode.lowerLimit, tMinLeaf, tMaxLeaf);

        // Add the values for this node
        updateUpper(upperLimitCurrent, expandedValues, mapForNormalization);

        // Set the upper limit for the leaf
        copyLimitInBetween(upperLimitCurrent, leafNode.upperLimit, tMinLeaf, tMaxLeaf);

        // Push this information to all ancestors
        traverseToRootForLimits(*pOutTree, leaf, tMinLeaf, tMaxLeaf);
    }

    portOutTree.setData(pOutTree);
}

float TemporalTreeLayoutComputation::normalValue(TemporalTree::TValueMap mapForNormalization, 
    uint64_t time, float value) const
{
    auto itNormalizeBy = mapForNormalization.find(time);
    if (itNormalizeBy != mapForNormalization.end() && itNormalizeBy->second != 0.0f)
    {
        return value / itNormalizeBy->second;
    }
    return 0.0f;
}

void TemporalTreeLayoutComputation::updateUpper(TemporalTree::TDrawingLimitMap& upperLimitCurrent,
    TemporalTree::TValueMap& values, 
    TemporalTree::TValueMap& mapForNormalization)
{
    auto itUpper = upperLimitCurrent.find(values.begin()->first);;
    for (auto itAdd = values.begin(); itAdd != values.end(); itAdd++, itUpper++)
    {
        if (itUpper->first != itAdd->first)
        {
            LogProcessorInfo("Times do not line up");
        }
        // Unless we are at the very beginning of the values to be added, update the limit from the left
        // itUsed->second is the pair with (left limit, right limit) for each timestep
        if (itAdd->first != values.begin()->first)
        {
            itUpper->second.first += 
                propSpaceFilling ? 
                normalValue(mapForNormalization, itAdd->first, itAdd->second) : itAdd->second / propMaximum;
        }
        // Unless we are at the very end of the values to be added, update the limit from the right 
        if (std::next(itAdd) != values.end())
        {
            itUpper->second.second += 
                propSpaceFilling ?
                normalValue(mapForNormalization, itAdd->first, itAdd->second) : itAdd->second / propMaximum;
        }
    }
}

void TemporalTreeLayoutComputation::traverseToRootForLimits(TemporalTree& tree, size_t nodeIndex, 
    uint64_t startTime, uint64_t endTime)
{
    TemporalTree::TNode& node = tree.nodes[nodeIndex];
    auto& lowerLimit = node.lowerLimit;
    auto& upperLimit = node.upperLimit;

    // Go to all parents
    for (auto& parent : tree.getHierarchicalParentsWithReverse(nodeIndex))
    {
        TemporalTree::TNode& parentNode = tree.nodes[parent];
        // Find the time range for which this is the parent
        if (!TemporalTree::TNode::isOverlappingTemporally(startTime, endTime, 
            parentNode.startTime(), parentNode.endTime()))
        {
            continue;
        }

        uint64_t startTimeParent = std::max(startTime, parentNode.startTime());
        uint64_t endTimeParent = std::min(endTime, parentNode.endTime());


        auto& lowerLimitParent = parentNode.lowerLimit;
        auto& upperLimitParent = parentNode.upperLimit;

        // Update the lower limit of the parent
        // Lower limit really should only be set once, therefore we insert
        lowerLimitParent.insert(lowerLimit.find(startTimeParent), std::next(lowerLimit.find(endTimeParent)));
        // End and begin could be set by different children (i.e. already exist in the map), so we need to treat his here
        lowerLimitParent[startTimeParent].second = std::min(lowerLimitParent[startTimeParent].second, lowerLimit[startTimeParent].second);
        lowerLimitParent[endTimeParent].first = std::min(lowerLimitParent[endTimeParent].first, lowerLimit[endTimeParent].first);

        // Update the higher limit of the parent (as we lay bands on top of each other, we can always replace)
        // Skip the first and the last values
        auto itEnd = upperLimit.find(endTimeParent);
        for (auto itUpper = upperLimit.find(startTimeParent); itUpper != itEnd; itUpper++)
        {
            if (itUpper->first == startTimeParent)
            {
                continue;
            }
            upperLimitParent.insert_or_assign(itUpper->first, itUpper->second);
        }

        auto it = upperLimitParent.find(startTimeParent);
        if (it != upperLimitParent.end())
        {
            it->second.second = std::max(it->second.second, upperLimit[startTimeParent].second);
        }
        else
        {
            upperLimitParent.emplace(startTimeParent, upperLimit[startTimeParent]);
        }

        it = upperLimitParent.find(endTimeParent);
        if (it != upperLimitParent.end())
        {
            it->second.first = std::max(it->second.first, upperLimit[endTimeParent].first);
        }
        else
        {
            upperLimitParent.emplace(endTimeParent, upperLimit[endTimeParent]);
        }

        // traverse further to the root
        traverseToRootForLimits(tree, parent, startTimeParent, endTimeParent);

    }

}

void TemporalTreeLayoutComputation::copyLimitInBetween(TemporalTree::TDrawingLimitMap& limitToCopy,
    TemporalTree::TDrawingLimitMap& limitToFill, 
    uint64_t startTime, uint64_t endTime)
{
    if (startTime > endTime)
    {
        LogProcessorError("Start Time larger then End Time?");
    }
    limitToFill.insert(limitToCopy.find(startTime), std::next(limitToCopy.find(endTime)));
}

} // namespace kth
} // namespace

