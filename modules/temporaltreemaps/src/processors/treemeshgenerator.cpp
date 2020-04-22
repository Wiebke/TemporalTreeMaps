/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Wednesday, December 06, 2017 - 23:04:14
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <modules/temporaltreemaps/processors/treemeshgenerator.h>
#include <modules/temporaltreemaps/datastructures/cushion.h>
#include <modules/temporaltreemaps/datastructures/treeorder.h>

namespace inviwo {
namespace kth {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TemporalTreeMeshGenerator::processorInfo_{
    "org.inviwo.TemporalTreeMeshGenerator",  // Class identifier
    "Tree Mesh Generator",                   // Display name
    "Temporal Tree",                         // Category
    CodeState::Experimental,                 // Code state
    Tags::None,                              // Tags
};

const ProcessorInfo TemporalTreeMeshGenerator::getProcessorInfo() const { return processorInfo_; }

TemporalTreeMeshGenerator::TemporalTreeMeshGenerator()
    : Processor()
    // Ports
    , portInTree("inTree")
    , portOutMeshBands("outMeshBands")
    , portOutMeshLines("outMeshLines")
    // Debug
    , propNumLeaves("numLeaves", "Stop after number", -1, -1, 20)
    // Transitions
    , propTransitions("transitions", "Transitions")
    , propMergeSplitBlend("mergeSplitBlend", "Blending Fraction Splits/Merges", 0.05f)
    // Lines
    , propLines("lines", "Lines")
    , propColorLines("colorLines", "Line Color", vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f),
                     vec4(1.0f), vec4(0.1f), InvalidationLevel::InvalidOutput,
                     PropertySemantics::Color)
    , propRenderInfo("renderInfo", "Render Info")
    , propInterpretAsCoefficients("coeffInterpret", "Normal is Coefficient", false) {
    // Ports
    addPort(portInTree);
    addPort(portOutMeshBands);
    addPort(portOutMeshLines);

    // Properties

    // Debug
    addProperty(propNumLeaves);

    // Transitions
    addProperty(propTransitions);
    propTransitions.addProperty(propMergeSplitBlend);

    // Lines
    addProperty(propLines);
    propLines.addProperty(propColorLines);

    addProperty(propRenderInfo);
    propRenderInfo.addProperty(propInterpretAsCoefficients);
}

void TemporalTreeMeshGenerator::process() {
    std::shared_ptr<const TemporalTree> pTree = portInTree.getData();

    if (!treeorder::fitsWithTree(*pTree, pTree->order)) {
        LogProcessorError("Order does not fit with the tree.");
        return;
    }

    propNumLeaves.setMaxValue(int(pTree->order.size()));

    if (pTree->edgesTime.size() != 0 && pTree->reverseEdgesTime.size() == 0) {
        LogProcessorError("Reverse Edges have not been computed");
        return;
    }

    // Make a new mesh and new vertex arrays
    auto meshLines = std::make_shared<BasicMesh>();
    std::vector<BasicMesh::Vertex> verticesLines;
    auto meshBands = std::make_shared<BasicMesh>();
    std::vector<BasicMesh::Vertex> verticesBands;

    makeMesh(*pTree, meshBands, verticesBands, meshLines, verticesLines);

    meshBands->addVertices(verticesBands);
    meshLines->addVertices(verticesLines);

    portOutMeshBands.setData(meshBands);
    portOutMeshLines.setData(meshLines);
}

void TemporalTreeMeshGenerator::makeMesh(const TemporalTree& tree,
                                         std::shared_ptr<BasicMesh> meshBands,
                                         std::vector<BasicMesh::Vertex>& verticesBands,
                                         std::shared_ptr<BasicMesh> meshLines,
                                         std::vector<BasicMesh::Vertex>& verticesLines) {
    auto times = tree.getTimes();
    uint64_t tMin = *times.begin();
    uint64_t tMax = *times.rbegin();

    auto indexBufferLine = meshLines->addIndexBuffer(DrawType::Lines, ConnectivityType::Strip);
    auto indexBufferSplitsMerges =
        meshBands->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
    // Start with zeros for each time step as a baseline

    std::map<uint64_t, size_t> lastLowerNode;

    for (auto time : times) {
        drawLineVertex(normalTime(time, tMin, tMax), 0.0f, *indexBufferLine, verticesLines);
    }

    size_t leafCounter(0);

    TemporalTree::TTreeOrderMap orderMap;
    treeorder::toOrderMap(orderMap, tree.order);

    for (auto leaf : tree.order) {
        if (propNumLeaves.get() != -1 && leafCounter >= propNumLeaves.get()) {
            break;
        }
        auto indexBufferBand =
            meshBands->addIndexBuffer(DrawType::Triangles, ConnectivityType::Strip);
        indexBufferLine = meshLines->addIndexBuffer(DrawType::Lines, ConnectivityType::Strip);

        const TemporalTree::TNode& leafNode = tree.nodes[leaf];

        auto& lowerLimitLeaf = leafNode.lowerLimit;
        auto& upperLimitLeaf = leafNode.upperLimit;
        auto& cushionLeaf = leafNode.cushion;
        auto& colorsLeaf = leafNode.colors;

        if (leafNode.values.size() < lowerLimitLeaf.size()) {
            LogProcessorError("Drawing limits do not have enough values for band to draw "
                              << leafCounter << ".");
            return;
        }

        if (lowerLimitLeaf.size() != upperLimitLeaf.size()) {
            LogProcessorError("Upper and lower limit for band to draw "
                              << leafCounter << " have different sizes.");
            return;
        }

        if (lowerLimitLeaf.size() != upperLimitLeaf.size()) {
            LogProcessorError("Upper and lower limit for band to draw "
                              << leafCounter << " have different sizes.");
            return;
        }

        if (colorsLeaf.size() != upperLimitLeaf.size()) {
            LogProcessorError("Colors and limits for band to draw " << leafCounter
                                                                    << " have different sizes.");
            return;
        }
        if (cushionLeaf.size() != upperLimitLeaf.size()) {
            LogProcessorError("Cushions and limits for band to draw " << leafCounter
                                                                      << " have different sizes.");
            return;
        }

        uint64_t tMinLeaf = lowerLimitLeaf.begin()->first;
        uint64_t tMaxLeaf = lowerLimitLeaf.rbegin()->first;

        auto itCushion = cushionLeaf.begin();
        auto itColor = colorsLeaf.begin();

        const auto& successors = tree.getTemporalSuccessors(leaf);
        const bool isSplit = successors.size() > 1
                             // Blend splits and nodes that correspond directly to the following
                             // node Number of sucessors is one for merges as well, therefore we
                             // need to test specifically for direct correspondence
                             ||
                             (successors.size() == 1 &&
                              tree.getTemporalPredecessorsWithReverse(successors[0]).size() == 1);

        const bool isMerge = tree.getTemporalPredecessorsWithReverse(leaf).size() > 1;

        if (lowerLimitLeaf.size() < 2) {
            LogProcessorWarn("Skipping a leaf with less then two values");
            continue;
        }

        const uint64_t tSecondToLast = upperLimitLeaf.size() > 1
                                           ? std::next(upperLimitLeaf.rbegin())->first
                                           : upperLimitLeaf.rbegin()->first;
        const float splitTime =
            isSplit ? std::max(normalTime(tMaxLeaf, tMin, tMax) - propMergeSplitBlend.get(),
                               normalTime(tSecondToLast, tMin, tMax))
                    : std::numeric_limits<float>::max();

        const uint64_t tSecond = upperLimitLeaf.size() > 1
                                     ? std::next(upperLimitLeaf.begin())->first
                                     : upperLimitLeaf.begin()->first;
        const float mergeTime =
            isMerge ? std::min(normalTime(tMinLeaf, tMin, tMax) + propMergeSplitBlend.get(),
                               normalTime(tSecond, tMin, tMax))
                    : std::numeric_limits<float>::max();

        for (auto itLowerLimit = lowerLimitLeaf.begin(), itUpperLimit = upperLimitLeaf.begin();
             itLowerLimit != lowerLimitLeaf.end() && itUpperLimit != upperLimitLeaf.end();
             itLowerLimit++, itUpperLimit++, itCushion++, itColor++) {
            float normalTimeLeaf = normalTime(itLowerLimit->first, tMin, tMax);

            if (normalTimeLeaf >= splitTime) {
                // Take already this line and split it up
                if (itLowerLimit->first == tSecondToLast) {
                    itLowerLimit++;
                    itUpperLimit++;
                    itCushion++;
                    itColor++;
                    normalTimeLeaf = normalTime(itLowerLimit->first, tMin, tMax);
                }
                // Make a new line before

                // Get left and right cushion and interpolate
                vec3 cushionLeft = std::prev(itCushion)->second.second;

                vec3 xLeft = vec3(std::prev(itLowerLimit)->second.second, 0.0f,
                                  std::prev(itUpperLimit)->second.second);
                vec3 xLeftCushion = xLeft;
                vec3 yLeft = vec3(0.0f);
                cushion::getPoints(xLeftCushion, yLeft, cushionLeft);

                vec3 cushionRight = itCushion->second.first;

                vec3 xRight = vec3(itLowerLimit->second.first, 0.0f, itUpperLimit->second.first);
                vec3 xRightCushion = xRight;
                vec3 yRight = vec3(0.0f);
                cushion::getPoints(xRightCushion, yRight, cushionRight);

                float tSecondToLastNormal = normalTime(tSecondToLast, tMin, tMax);
                float t =
                    (splitTime - tSecondToLastNormal) / (normalTimeLeaf - tSecondToLastNormal);

                // indices that we are spliting into
                auto splitees = tree.getTemporalSuccessors(leaf);

                float spliteeSum = 0.0;

                for (auto split : splitees) {
                    if (!tree.isLeaf(split)) {
                        t = 1.0;
                    }
                    spliteeSum += tree.nodes[split].upperLimit.at(tMaxLeaf).second -
                                  tree.nodes[split].lowerLimit.at(tMaxLeaf).second;
                }

                vec3 xInterpolatedCushion = t * xRightCushion + (1 - t) * xLeftCushion;
                vec3 yInterpolated = t * yRight + (1 - t) * yLeft;
                vec3 cushionInterpolated = t * cushionRight + (1 - t) * cushionLeft;

                auto extremum = cushion::getGlobalExtremum(xRightCushion, cushionInterpolated);
                vec3 xInterpolated = t * xRight + (1 - t) * xLeft;
                xInterpolated.y = extremum.first;

                // Adapt only location of the split point, not the cushions themselves
                if (xInterpolated.y < xInterpolated.x || xInterpolated.y > xInterpolated.z) {
                    xInterpolated.y = (xInterpolated.x + xInterpolated.z) / 2.0f;
                }

                vec4 oldColor = itColor->second;

                drawLineVertex(tSecondToLastNormal, std::prev(itUpperLimit)->second.second,
                               *indexBufferLine, verticesLines);
                drawLineVertex(normalTimeLeaf, itUpperLimit->second.first, *indexBufferLine,
                               verticesLines);

                // Lower vertex for the new line
                verticesBands.push_back(
                    {vec3(splitTime, xInterpolated.x, 0.0f),
                     propInterpretAsCoefficients.get() ? cushionInterpolated : xInterpolatedCushion,
                     yInterpolated, oldColor});
                size_t splitLineLowerVertex = verticesBands.size() - 1;
                indexBufferBand->add(static_cast<std::uint32_t>(splitLineLowerVertex));

                // Middle / Maximum vertex for the new line
                verticesBands.push_back(
                    {vec3(splitTime, xInterpolated.y, 0.0f),
                     propInterpretAsCoefficients.get() ? cushionInterpolated : xInterpolatedCushion,
                     yInterpolated, oldColor});
                size_t splitLineMiddleVertex = verticesBands.size() - 1;

                // Upper vertex for the new line
                verticesBands.push_back(
                    {vec3(splitTime, xInterpolated.z, 0.0f),
                     propInterpretAsCoefficients.get() ? cushionInterpolated : xInterpolatedCushion,
                     yInterpolated, oldColor});
                size_t splitLineUpperVertex = verticesBands.size() - 1;
                indexBufferBand->add(static_cast<std::uint32_t>(splitLineUpperVertex));

                if (std::fabs(t - 1.0) < std::numeric_limits<float>::epsilon()) {
                    continue;
                }

                // Go through these in the order that there are drawn in
                treeorder::sortNodesByOrder(orderMap, tree, splitees);

                float ratio = (xRight.z - xRight.x) / spliteeSum;

                float xLower = xRight.x;

                for (auto split : splitees) {
                    float splitValue = tree.nodes[split].upperLimit.at(tMaxLeaf).second -
                                       tree.nodes[split].lowerLimit.at(tMaxLeaf).second;
                    float xUpper = xLower + splitValue * ratio;

                    vec3 cushionSplit = tree.nodes[split].cushion.at(tMaxLeaf).second;
                    vec3 xSplit = vec3(xLower, 0.0f, xUpper);
                    vec3 ySplit = vec3(0.0f);
                    cushion::getPoints(xSplit, ySplit, cushionSplit);
                    vec4 splitColor = tree.nodes[split].colors.at(tMaxLeaf);

                    indexBufferSplitsMerges->add(static_cast<std::uint32_t>(splitLineMiddleVertex));
                    verticesBands.push_back(
                        {vec3(normalTimeLeaf, xLower, 0.0f),
                         propInterpretAsCoefficients.get() ? cushionSplit : xSplit, ySplit,
                         splitColor});
                    indexBufferSplitsMerges->add(
                        static_cast<std::uint32_t>(verticesBands.size() - 1));

                    verticesBands.push_back(
                        {vec3(normalTimeLeaf, xUpper, 0.0f),
                         propInterpretAsCoefficients.get() ? cushionSplit : xSplit, ySplit,
                         splitColor});
                    indexBufferSplitsMerges->add(
                        static_cast<std::uint32_t>(verticesBands.size() - 1));

                    // If this is the first split make a triangle with the lower part of the split
                    // line
                    if (std::fabs(xLower - xRight.x) < std::numeric_limits<float>::epsilon()) {
                        indexBufferSplitsMerges->add(
                            static_cast<std::uint32_t>(splitLineLowerVertex));
                        indexBufferSplitsMerges->add(
                            static_cast<std::uint32_t>(splitLineMiddleVertex));
                        indexBufferSplitsMerges->add(
                            static_cast<std::uint32_t>(verticesBands.size() - 2));
                    }

                    if (std::fabs(xUpper - xRight.z) < std::numeric_limits<float>::epsilon()) {
                        indexBufferSplitsMerges->add(
                            static_cast<std::uint32_t>(splitLineMiddleVertex));
                        indexBufferSplitsMerges->add(
                            static_cast<std::uint32_t>(splitLineUpperVertex));
                        indexBufferSplitsMerges->add(
                            static_cast<std::uint32_t>(verticesBands.size() - 1));
                    }

                    xLower = xUpper;
                }

                break;
            }

            if (itLowerLimit->first == tMinLeaf) {
                if (mergeTime > normalTimeLeaf && mergeTime <= normalTime(tSecond, tMin, tMax)) {
                    // Get left and right cushion and interpolate
                    // @todo: Put cushion interpolation somewhere else
                    vec3 cushionLeft = itCushion->second.second;

                    vec3 xLeft =
                        vec3(itLowerLimit->second.second, 0.0f, itUpperLimit->second.second);
                    vec3 xLeftCushion = xLeft;
                    vec3 yLeft = vec3(0.0f);
                    cushion::getPoints(xLeftCushion, yLeft, cushionLeft);

                    vec3 cushionRight = std::next(itCushion)->second.first;

                    vec3 xRight = vec3(std::next(itLowerLimit)->second.first, 0.0f,
                                       std::next(itUpperLimit)->second.first);
                    vec3 xRightCushion = xRight;
                    vec3 yRight = vec3(0.0f);
                    cushion::getPoints(xRightCushion, yRight, cushionRight);

                    float tSecondNormal = normalTime(tSecond, tMin, tMax);
                    float t = (mergeTime - normalTimeLeaf) / (tSecondNormal - normalTimeLeaf);

                    // indices that we have merges from
                    auto mergees = tree.getTemporalPredecessors(leaf);

                    for (auto merge : mergees) {
                        if (!tree.isLeaf(merge)) {
                            t = 0.0;
                            break;
                        }
                    }

                    // the maximum point is at (xInterpolated.y, yInterpolated.y)
                    vec3 xInterpolatedCushion = t * xRightCushion + (1 - t) * xLeftCushion;
                    vec3 yInterpolated = t * yRight + (1 - t) * yLeft;
                    vec3 cushionInterpolated = t * cushionRight + (1 - t) * cushionLeft;

                    auto extremum = cushion::getGlobalExtremum(xRightCushion, cushionInterpolated);
                    vec3 xInterpolated = t * xRight + (1 - t) * xLeft;
                    xInterpolated.y = extremum.first;

                    // The maximum point does not lie between upper and lower bound
                    if (xInterpolated.y < xInterpolated.x || xInterpolated.y > xInterpolated.z) {
                        xInterpolated.y = (xInterpolated.x + xInterpolated.z) / 2.0f;
                    }

                    // Indices of the last upper and lower vertex
                    vec4 newColor = std::next(itColor)->second;

                    drawLineVertex(normalTimeLeaf, itUpperLimit->second.second, *indexBufferLine,
                                   verticesLines);
                    drawLineVertex(tSecondNormal, std::next(itUpperLimit)->second.first,
                                   *indexBufferLine, verticesLines);

                    // Lower vertex for the new line
                    verticesBands.push_back({vec3(mergeTime, xInterpolated.x, 0.0f),
                                             propInterpretAsCoefficients.get()
                                                 ? cushionInterpolated
                                                 : xInterpolatedCushion,
                                             yInterpolated, newColor});
                    size_t mergeLineLowerVertex = verticesBands.size() - 1;
                    indexBufferBand->add(static_cast<std::uint32_t>(mergeLineLowerVertex));

                    // Middle / Maximum vertex for the new line
                    verticesBands.push_back({vec3(mergeTime, xInterpolated.y, 0.0f),
                                             propInterpretAsCoefficients.get()
                                                 ? cushionInterpolated
                                                 : xInterpolatedCushion,
                                             yInterpolated, newColor});
                    size_t mergeLineMiddleVertex = verticesBands.size() - 1;

                    // Upper vertex for the new line
                    verticesBands.push_back({vec3(mergeTime, xInterpolated.z, 0.0f),
                                             propInterpretAsCoefficients.get()
                                                 ? cushionInterpolated
                                                 : xInterpolatedCushion,
                                             yInterpolated, newColor});
                    size_t mergeLineUpperVertex = verticesBands.size() - 1;
                    indexBufferBand->add(static_cast<std::uint32_t>(mergeLineUpperVertex));

                    if (std::fabs(t) < std::numeric_limits<float>::epsilon()) {
                        // We are not drawing any of the mergees anyways
                        // So we can skip this parts
                        continue;
                    }

                    // Go through these in the order that there are drawn in
                    treeorder::sortNodesByOrder(orderMap, tree, mergees);

                    float xLower = xLeft.x;

                    for (auto merge : mergees) {
                        float mergeValue = tree.nodes[merge].upperLimit.at(tMinLeaf).first -
                                           tree.nodes[merge].lowerLimit.at(tMinLeaf).first;
                        float xUpper = xLower + mergeValue;

                        vec3 cushionMerge = tree.nodes[merge].cushion.at(tMinLeaf).first;
                        vec3 xMerge = vec3(xLower, 0.0f, xUpper);
                        vec3 yMerge = vec3(0.0f);
                        cushion::getPoints(xMerge, yMerge, cushionMerge);
                        vec4 mergeColor = tree.nodes[merge].colors.at(tMinLeaf);

                        indexBufferSplitsMerges->add(
                            static_cast<std::uint32_t>(mergeLineMiddleVertex));
                        verticesBands.push_back(
                            {vec3(normalTimeLeaf, xLower, 0.0f),
                             propInterpretAsCoefficients.get() ? cushionMerge : xMerge, yMerge,
                             mergeColor});
                        indexBufferSplitsMerges->add(
                            static_cast<std::uint32_t>(verticesBands.size() - 1));

                        verticesBands.push_back(
                            {vec3(normalTimeLeaf, xUpper, 0.0f),
                             propInterpretAsCoefficients.get() ? cushionMerge : xMerge, yMerge,
                             mergeColor});
                        indexBufferSplitsMerges->add(
                            static_cast<std::uint32_t>(verticesBands.size() - 1));

                        // If this is the first split make a triangle with the lower part of the
                        // split line
                        if (std::fabs(xLower - xLeft.x) < std::numeric_limits<float>::epsilon()) {
                            indexBufferSplitsMerges->add(
                                static_cast<std::uint32_t>(mergeLineLowerVertex));
                            indexBufferSplitsMerges->add(
                                static_cast<std::uint32_t>(mergeLineMiddleVertex));
                            indexBufferSplitsMerges->add(
                                static_cast<std::uint32_t>(verticesBands.size() - 2));
                        }

                        if (std::fabs(xUpper - xLeft.z) < std::numeric_limits<float>::epsilon()) {
                            indexBufferSplitsMerges->add(
                                static_cast<std::uint32_t>(mergeLineMiddleVertex));
                            indexBufferSplitsMerges->add(
                                static_cast<std::uint32_t>(mergeLineUpperVertex));
                            indexBufferSplitsMerges->add(
                                static_cast<std::uint32_t>(verticesBands.size() - 1));
                        }

                        xLower = xUpper;
                    }

                    if (std::fabs(mergeTime - normalTime(tSecond, tMin, tMax)) <
                        std::numeric_limits<float>::epsilon()) {
                        itLowerLimit++;
                        itUpperLimit++;
                        itCushion++;
                    }
                } else {
                    drawVertexPair(normalTimeLeaf, itLowerLimit->second.second,
                                   itUpperLimit->second.second, itCushion->second.second,
                                   itColor->second, *indexBufferBand, verticesBands);
                    drawLineVertex(normalTimeLeaf, itUpperLimit->second.second, *indexBufferLine,
                                   verticesLines);
                    // Either we fade in or we merge -> both cases need just one edge
                    // updateUpper(normalTimeLeaf, normalTimeLeaf, itLowerLimit->second.second,
                    // indexBufferLine, verticesLines);
                }
            } else if (itLowerLimit->first == tMaxLeaf) {
                drawVertexPair(normalTimeLeaf, itLowerLimit->second.first,
                               itUpperLimit->second.first, itCushion->second.first, itColor->second,
                               *indexBufferBand, verticesBands);
                drawLineVertex(normalTimeLeaf, itUpperLimit->second.first, *indexBufferLine,
                               verticesLines);
                // Either we fade out or we split -> both cases do not need a closing edge
            } else {
                drawVertexPair(normalTimeLeaf, itLowerLimit->second.first,
                               itUpperLimit->second.first, itCushion->second.first, itColor->second,
                               *indexBufferBand, verticesBands);
                drawLineVertex(normalTimeLeaf, itUpperLimit->second.first, *indexBufferLine,
                               verticesLines);
                // Suffices to test for one, we have added the same value to both anyways
                if (std::fabs(itLowerLimit->second.second - itLowerLimit->second.first) >
                    std::numeric_limits<float>::epsilon()) {
                    drawVertexPair(normalTimeLeaf, itLowerLimit->second.second,
                                   itUpperLimit->second.second, itCushion->second.second,
                                   itColor->second, *indexBufferBand, verticesBands);
                    drawLineVertex(normalTimeLeaf, itUpperLimit->second.second, *indexBufferLine,
                                   verticesLines);
                }
            }
        }

        leafCounter++;
    }
}

void TemporalTreeMeshGenerator::drawVertexPair(const float x, const float yLower,
                                               const float yUpper, const vec3& coefficients,
                                               const vec4& color, IndexBufferRAM& indexBufferBand,
                                               std::vector<BasicMesh::Vertex>& verticesBands) {
    // float yUpper = yLower + dY;
    vec3 normal = vec3(yLower, 0.0f, yUpper);
    vec3 tex = vec3(0.0f);
    cushion::getPoints(normal, tex, coefficients);

    vec3 coefficientsTest = cushion::getCoefficients(normal, tex);

    /*if (std::fabs(coefficients.x - coefficientsTest.x) > 0.01
        || std::fabs(coefficients.y - coefficientsTest.y) > 0.01
        || std::fabs(coefficients.z - coefficientsTest.z) > 0.01)
    {
        LogProcessorWarn("Parabola conversion back failed with differences." <<
            coefficients.x - coefficientsTest.x << "," <<
            coefficients.y - coefficientsTest.y << "," <<
            coefficients.z - coefficientsTest.z << ",");
    }*/

    // Lower vertex (same position as the lastVertexBelow)
    verticesBands.push_back({vec3(x, yLower, 0.0f),
                             propInterpretAsCoefficients.get() ? coefficients : normal, tex,
                             color});
    indexBufferBand.add(static_cast<std::uint32_t>(verticesBands.size() - 1));

    // Upper vertex (same x, y dependent on value) (unless it is exactly the same one)
    if (std::fabs(yUpper - yLower) >= std::numeric_limits<float>::epsilon()) {
        verticesBands.push_back({vec3(x, yUpper, 0.0f),
                                 propInterpretAsCoefficients.get() ? coefficients : normal, tex,
                                 color});
        indexBufferBand.add(static_cast<std::uint32_t>(verticesBands.size() - 1));
    }
}

void TemporalTreeMeshGenerator::drawLineVertex(const float x, const float y,
                                               IndexBufferRAM& indexBufferLine,
                                               std::vector<BasicMesh::Vertex>& verticesLines) {
    verticesLines.push_back({vec3(x, y, 0.0f), vec3(0), vec3(0), propColorLines.get()});
    indexBufferLine.add(static_cast<std::uint32_t>(verticesLines.size() - 1));
}

}  // namespace kth
}  // namespace inviwo
