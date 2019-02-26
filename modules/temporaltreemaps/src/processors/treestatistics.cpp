/*********************************************************************
 *  Author  : Tino Weinkauf
 *  Init    : Sunday, March 11, 2018 - 11:12:19
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <modules/temporaltreemaps/processors/treestatistics.h>

#ifndef __clang__
    #include <omp.h>
#endif

namespace inviwo
{
namespace kth
{

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TemporalTreeStatistics::processorInfo_
{
    "org.inviwo.TemporalTreeStatistics",      // Class identifier
    "Tree Statistics",                // Display name
    "Temporal Tree",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};

const ProcessorInfo TemporalTreeStatistics::getProcessorInfo() const
{
    return processorInfo_;
}


TemporalTreeStatistics::TemporalTreeStatistics()
    :portInTree("InTree")
    ,propStatGivenTree("StatGivenTree", "Input Tree")
    ,propStatNonAggregatedTree("StatNonAggregatedTree", "Non-Aggregated Tree")
{
    addPort(portInTree);

    propStatGivenTree.setSemantics(PropertySemantics::Multiline);
    //propStatGivenTree.setReadOnly(true);
    addProperty(propStatGivenTree);

    propStatNonAggregatedTree.setSemantics(PropertySemantics::Multiline);
    //propStatNonAggregatedTree.setReadOnly(true);
    addProperty(propStatNonAggregatedTree);
}


namespace
{
    void GenerateStatisticsString(const size_t NumLevels
                                  ,const size_t NumLeaves
                                  ,const uint64_t tMin
                                  ,const uint64_t tMax
                                  ,const size_t NumNodes
                                  ,const size_t NumHierarchyEdges
                                  ,const size_t NumTimeEdges
                                  ,const size_t NumTimeSteps
                                  ,StringProperty& StatString)
    {
        std::stringstream ss;

        ss
            << "Number of Nodes: " << NumNodes<< std::endl
            << "Number of Hierarchy Edges: " << NumHierarchyEdges << std::endl
            << "Number of Time Edges: " << NumTimeEdges << std::endl
            << "Time Span: [" << tMin << ", " << tMax << "]" << std::endl
            << "Time Span Diff: " << tMax - tMin << std::endl
            << "Time Steps: " << NumTimeSteps << std::endl
            << "Number of Hierarchy Levels: " << NumLevels << std::endl
            << "Number of Leaves: " << NumLeaves << std::endl
            ;

        //Bytes
        const size_t BytesPerEdge = 2 * sizeof(size_t);
        const size_t BytesPerNode = sizeof(float);
        const size_t TotalBytes = NumNodes * BytesPerNode + (NumHierarchyEdges + NumTimeEdges) * BytesPerEdge;
        ss << "Memory: " << TotalBytes << " Bytes ("
            << float(TotalBytes)/(1024.*1024.) << " MB)" << std::endl;

        StatString.set(ss.str());
    }
}


void TemporalTreeStatistics::process()
{
    //Get the inputs
    std::shared_ptr<const TemporalTree> pTree = portInTree.getData();
    if (!pTree) return;

    //Statistics for the given tree in its current, likely aggregated, form
    const size_t MaxLevel(pTree->getNumLevels(0));
    const size_t NumLeaves = pTree->getLeaves().size();
    uint64_t tMin, tMax;
    pTree->getMinMaxTime(0, tMin, tMax);
    const size_t NumNodes(pTree->nodes.size());
    const size_t NumHierarchyEdges(pTree->getNumHierarchyEdges());
    const size_t NumTimeEdges(pTree->getNumTimeEdges());

    //Get all time steps
    std::set<uint64_t> AllTimeSteps;
    pTree->getTimes(0, AllTimeSteps);

    GenerateStatisticsString(MaxLevel, NumLeaves, tMin, tMax, NumNodes, NumHierarchyEdges, NumTimeEdges, AllTimeSteps.size(), propStatGivenTree);

    // NON-AGGREGATED VERSION

    const signed long long iNumNodes = (unsigned long long)NumNodes;

    signed long long NANumNodes = 0;
    signed long long NANumHierarchyEdges = 0;
    signed long long NANumTimeEdges = 0;
    signed long long NANumLeaves = 0;

    #ifndef __clang__
        omp_set_num_threads(std::thread::hardware_concurrency());
    #endif    
    //#pragma omp parallel for reduction(+: NANumNodes, NANumHierarchyEdges, NANumTimeEdges, NANumLeaves)
    for(signed long long i(0);i<iNumNodes;i++)
    {
        //This node would be as many nodes as we have time steps in its interval
        const uint64_t tNodeMin = pTree->nodes[i].startTime();
        const uint64_t tNodeMax = pTree->nodes[i].endTime();
        auto itStart = AllTimeSteps.find(tNodeMin);
        auto itEnd = AllTimeSteps.find(tNodeMax);
        const size_t NumAggregatedTimeStepHops = std::distance(itStart, itEnd);

        NANumNodes += NumAggregatedTimeStepHops + 1;
        NANumTimeEdges += NumAggregatedTimeStepHops;

        //Leaf?
        if (pTree->isLeaf(i)) NANumLeaves += NumAggregatedTimeStepHops + 1;

        //The children of this node have as many edges as we have overlapping time steps
        const auto Children = pTree->getHierarchicalChildren(i);
        for(const size_t& idxChild : Children)
        {
            const uint64_t tChildMin = pTree->nodes[idxChild].startTime();
            const uint64_t tChildMax = pTree->nodes[idxChild].endTime();

            auto itChildNodeStart = AllTimeSteps.find(tChildMin > tNodeMin ? tChildMin : tNodeMin);
            auto itChildNodeEnd = AllTimeSteps.find(tChildMax < tNodeMax ? tChildMax : tNodeMax);
            const size_t NumAggregatedChildTimeStepHops = std::distance(itChildNodeStart, itChildNodeEnd);

            NANumHierarchyEdges += NumAggregatedChildTimeStepHops + 1;
        }
    }

    GenerateStatisticsString(MaxLevel, NANumLeaves, tMin, tMax, NANumNodes, NANumHierarchyEdges, NANumTimeEdges, AllTimeSteps.size(), propStatNonAggregatedTree);
}

} // namespace
} // namespace

