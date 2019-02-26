/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Monday, December 11, 2017 - 19:50:26
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <modules/temporaltreemaps/processors/treemeshgeneratortopo.h>
#include <inviwo/core/util/colorconversion.h>

namespace inviwo
{
namespace kth
{

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TemporalTreeMeshGeneratorTopo::processorInfo_
{
    "org.inviwo.TemporalTreeMeshGeneratorTopo",      // Class identifier
    "Tree Mesh Generator Topo",                // Display name
    "Temporal Tree",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};

const ProcessorInfo TemporalTreeMeshGeneratorTopo::getProcessorInfo() const
{
    return processorInfo_;
}


TemporalTreeMeshGeneratorTopo::TemporalTreeMeshGeneratorTopo()
    :Processor()
    ,portInTree("InTree")
    ,portOutMeshBands("OutMeshBands")
    ,propSpacing("Spacing", "Spacing", 10, 0, 50)
    ,propLevelPortion("LevelPortion", "Level Portion", 50, 0, 100)
    ,propConstraintIndication("ConstraintIndication", "Constraint Indication", 2, 0, 10)
{
    addPort(portInTree);
    addPort(portOutMeshBands);

    addProperty(propSpacing);
    addProperty(propLevelPortion);
    addProperty(propConstraintIndication);
}

namespace
{


void GetLayerOrder(const TemporalTree& Tree, const std::vector<size_t>& LeafOrder, const std::map<size_t, size_t>& LeafOrderMap,
                   const std::vector<size_t>& LevelIndices, TemporalTreeMeshGeneratorTopo::TLayerOrder& LayerOrder)
{
    //Prepare memory
    const size_t NumLeaves = LeafOrder.size();
    LayerOrder.reserve(NumLeaves);
    LayerOrder.clear();

    //For each parent, get all its leaves and make sure to be rendered there.
    std::set<size_t> LeavesOfParent;
    for(const size_t idxParent : LevelIndices)
    {
        //Get this parent's leaves
        LeavesOfParent.clear();

        const uint64_t tMinParent = Tree.nodes[idxParent].startTime();
        const uint64_t tMaxParent = Tree.nodes[idxParent].endTime();

        Tree.getLeaves(idxParent, tMinParent, tMaxParent, tMinParent, tMaxParent, LeavesOfParent);

        //Add to render queue
        size_t MinOrderRow(NumLeaves);
        size_t MaxOrderRow(0);
        for(const size_t idxLeaf : LeavesOfParent)
        {
            LayerOrder.emplace_back();
            LayerOrder.back().idxNodeToBeDrawn = idxParent;
            LayerOrder.back().idxLeaf = idxLeaf;
            LayerOrder.back().OrderRow = LeafOrderMap.at(idxLeaf);
            //LayerOrder.back().bFullCoverage = false; //later
            if (LayerOrder.back().OrderRow < MinOrderRow) MinOrderRow = LayerOrder.back().OrderRow;
            if (LayerOrder.back().OrderRow > MaxOrderRow) MaxOrderRow = LayerOrder.back().OrderRow;
        }

        //Compute coverage
        int NumOverlap((int)LeavesOfParent.size());
        if (int(MaxOrderRow) - int(MinOrderRow) + 1 == NumOverlap)
        {
            for(auto it=LayerOrder.rbegin();it!=LayerOrder.rend()&&it->idxNodeToBeDrawn==idxParent;it++)
            {
                it->bFullCoverage = true;
            }
        }
        else
        {
            //Gotta check temporal overlaps
            for(size_t r(MinOrderRow);r<=MaxOrderRow&&r<NumLeaves&&NumOverlap>=0;r++)
            {
                //A leaf in the drawing area; may not be ours. If it is not ours, but it overlaps, then we do not have full coverage.

                //Leaf's time
                const uint64_t tMinLeaf = Tree.nodes[LeafOrder[r]].startTime();
                const uint64_t tMaxLeaf = Tree.nodes[LeafOrder[r]].endTime();

                //Overlap?
                //     |------|          (Parent)
                // Cases to exclude: 
                //|---|                 (Leaf completely before, fulfills tMinLeaf<tMaxParent)
                //             |------|   (Leaf completely after, fulfills tMaxLeaf>tMinParent)
                if (std::max(tMinLeaf, tMinParent) < std::min(tMaxLeaf, tMaxParent)) NumOverlap--;
            }

            ivwAssert(NumOverlap <= 0, "Missed a child? How? Not ok!");
            for(auto it=LayerOrder.rbegin();it!=LayerOrder.rend()&&it->idxNodeToBeDrawn==idxParent;it++)
            {
                it->bFullCoverage = (NumOverlap == 0);
            }
        }
    }
}

}

void TemporalTreeMeshGeneratorTopo::CreateMesh(const TemporalTree& Tree, const size_t Level, const size_t MaxLevel,
                                         const size_t NumRows, const TLayerOrder& Order,
                                         std::shared_ptr<BasicMesh>& Mesh, std::vector<BasicMesh::Vertex>& Vertices)
{
    //Get temporal min/max
    uint64_t tMin, tMax;
    Tree.getMinMaxTimeShallow(0, tMin, tMax); //I am assuming root here!

    //Prepare buffer
    const size_t NumItems = Order.size();
    const size_t NumVerticesBefore(Vertices.size());
    Vertices.resize(NumVerticesBefore + NumItems * 8);

    //Prepare for normalization in x-direction
    const float Range(float(tMax - tMin));
    const float fMin = float(tMin);
    const float xIndicatorSize = propConstraintIndication / 100.0f;

    //Prepare for computations in y-direction
    const float yBandWidth = 1.0f / float(NumRows);
    const float ySpacing = propSpacing / float(2 * 100 * NumRows);
    const float yActualBandWidth = yBandWidth - 2.0f * ySpacing;
    const float yLevelOffset = yActualBandWidth * (float(Level)/float(MaxLevel)) * (propLevelPortion / (2.0f * 100.0f));
    const float yOffset = ySpacing + yLevelOffset;

    //Colors
    // Black for MaxLevel, White for Root
    vec4 DefaultColor(1.0f - float(Level)/float(MaxLevel));
    DefaultColor[3] = 1.0f;
    vec4 RuptureColor(0.5,0.2,0.2,1);

    //Constraints
    std::vector<std::pair<int, int>> Constraints;
    const size_t NumConstraints = Tree.getConstraintClusters(Constraints);

    //Draw each element
    for (size_t i(0);i<NumItems;i++)
    {
        //Shorthand
        const size_t idxThisNode = Order[i].idxNodeToBeDrawn;
        const size_t idRow = Order[i].OrderRow;
        const bool bIndicateRupture = !Order[i].bFullCoverage;
        const TemporalTree::TNode& ThisNode = Tree.nodes[idxThisNode];
        const size_t StartVertex = NumVerticesBefore + 8*i;

        //Get time and normalize
        const uint64_t StartTime = ThisNode.startTime();
        const uint64_t EndTime = ThisNode.endTime();
        const float xLeft = (float(StartTime) - fMin) / Range;
        const float xRight = (float(EndTime) - fMin) / Range;
        const float xMiddle = 0.5f * (xRight + xLeft);
        float xLeftIndication = xLeft + xIndicatorSize;
        if (xLeftIndication > xMiddle) xLeftIndication = xMiddle;
        float xRightIndication = xRight - xIndicatorSize;
        if (xRightIndication < xMiddle) xRightIndication = xMiddle;

        //Get height
        const float yBottom = float(idRow) / float(NumRows) + yOffset;
        const float yTop = float(idRow+1) / float(NumRows) - yOffset;

        //Set colors
        // - color to indicate constraints at the left of the node
        vec4 LeftColor(DefaultColor);
        if (Constraints[idxThisNode].first >= 0)
        {
            vec3 LeftColor3 = color::hsv2rgb(vec3(float(Constraints[idxThisNode].first) / float(NumConstraints), 1.0f, 1.0f));
            LeftColor[0] = LeftColor3[0];
            LeftColor[1] = LeftColor3[1];
            LeftColor[2] = LeftColor3[2];
        }
        // - color to indicate constraints at the right of the node
        vec4 RightColor(DefaultColor);
        if (Constraints[idxThisNode].second >= 0)
        {
            vec3 RightColor3 = color::hsv2rgb(vec3(float(Constraints[idxThisNode].second) / float(NumConstraints), 1.0f, 1.0f));
            RightColor[0] = RightColor3[0];
            RightColor[1] = RightColor3[1];
            RightColor[2] = RightColor3[2];
        }

        //Set vertices

        vec3 pos = { xLeft, yBottom, float(Level) };
        Vertices[StartVertex + 0] = { pos, vec3(0,0,0), pos, LeftColor };
        pos = { xLeft, yTop, float(Level) };
        Vertices[StartVertex + 1] = { pos, vec3(0,0,0), pos, LeftColor };
        pos = { xLeftIndication, yBottom, float(Level) };
        Vertices[StartVertex + 2] = { pos, vec3(0,0,0), pos, bIndicateRupture ? RuptureColor : DefaultColor };
        pos = { xLeftIndication, yTop, float(Level) };
        Vertices[StartVertex + 3] = { pos, vec3(0,0,0), pos, bIndicateRupture ? RuptureColor : DefaultColor };
        pos = { xRightIndication, yBottom, float(Level) };
        Vertices[StartVertex + 4] = { pos, vec3(0,0,0), pos, bIndicateRupture ? RuptureColor : DefaultColor };
        pos = { xRightIndication, yTop, float(Level) };
        Vertices[StartVertex + 5] = { pos, vec3(0,0,0), pos, bIndicateRupture ? RuptureColor : DefaultColor };
        pos = { xRight, yBottom, float(Level) };
        Vertices[StartVertex + 6] = { pos, vec3(0,0,0), pos, RightColor };
        pos = { xRight, yTop, float(Level) };
        Vertices[StartVertex + 7] = { pos, vec3(0,0,0), pos, RightColor };



        //Add to indexbuffer
        auto IdxBuffer = Mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::Strip);
        IdxBuffer->add(uint32_t(StartVertex + 0));
        IdxBuffer->add(uint32_t(StartVertex + 1));
        IdxBuffer->add(uint32_t(StartVertex + 2));
        IdxBuffer->add(uint32_t(StartVertex + 3));
        IdxBuffer->add(uint32_t(StartVertex + 4));
        IdxBuffer->add(uint32_t(StartVertex + 5));
        IdxBuffer->add(uint32_t(StartVertex + 6));
        IdxBuffer->add(uint32_t(StartVertex + 7));
    }
}


void TemporalTreeMeshGeneratorTopo::process()
{
    //Get the inputs
    std::shared_ptr<const TemporalTree> pTree = portInTree.getData();
    if (!pTree) return;
    if (!treeorder::fitsWithTree(*pTree, pTree->order))
    {
        LogError("Order does not fit with the tree.");
        return;
    }

    //Get a proper order
    TemporalTree::TTreeOrder Order(pTree->order);
    const size_t NumLeaves = Order.size();

    TemporalTree::TTreeOrderMap OrderMap;
    treeorder::toOrderMap(OrderMap, Order);

    //Get the output
    std::shared_ptr<BasicMesh> MeshBands = std::make_shared<BasicMesh>();
    std::vector<BasicMesh::Vertex> Vertices;

    //Mesh!
    TLayerOrder LayerOrder;
    size_t Level(1);
    size_t MaxLevel(pTree->getNumLevels(0));
    std::vector<size_t> LevelIndices;
    LevelIndices = pTree->getLevel(Level, LevelIndices, 0);
    while (!LevelIndices.empty())
    {
        ivwAssert(Level <= MaxLevel, "Too deep!");

        //Create an order of nodes in this layer, but depending on the leaves layer
        GetLayerOrder(*pTree, Order, OrderMap, LevelIndices, LayerOrder);

        //Create the actual mesh
        CreateMesh(*pTree, Level, MaxLevel, NumLeaves, LayerOrder, MeshBands, Vertices);

        //Forward to the next level
        LevelIndices = pTree->getLevel(Level+1, LevelIndices, Level);
        Level++;
    }

    //Push it out!
    MeshBands->addVertices(Vertices);
    portOutMeshBands.setData(MeshBands);
}

} // namespace kth
} // namespace
