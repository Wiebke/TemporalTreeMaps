/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Wednesday, December 06, 2017 - 23:04:14
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#pragma once

#include <modules/temporaltreemaps/temporaltreemapsmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <modules/temporaltreemaps/datastructures/treeport.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>

namespace inviwo
{

namespace kth
{

/** \docpage{org.inviwo.TemporalTreeMeshGenerator, Tree Mesh Generator}
    ![](org.inviwo.TemporalTreeMeshGenerator.png?classIdentifier=org.inviwo.TemporalTreeMeshGenerator)

    Explanation of how to use the processor.
    
    ### Inports
      * __<Inport1>__ <description>.
    
    ### Outports
      * __<Outport1>__ <description>.
    
    ### Properties
      * __<Prop1>__ <description>.
      * __<Prop2>__ <description>
*/


/** \class TemporalTreeMeshGenerator
    \brief VERY_BRIEFLY_DESCRIBE_THE_PROCESSOR
    
    DESCRIBE_THE_PROCESSOR_FROM_A_DEVELOPER_PERSPECTIVE

    @author Tino Weinkauf and Wiebke Koepp
*/
class IVW_MODULE_TEMPORALTREEMAPS_API TemporalTreeMeshGenerator : public Processor
{ 
//Friends
//Types
public:

//Construction / Deconstruction
public:
    TemporalTreeMeshGenerator();
    virtual ~TemporalTreeMeshGenerator() = default;

//Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    ///Our main computation function
    virtual void process() override;

    float normalTime(uint64_t time, uint64_t tMin, uint64_t tMax) const
    {
        return (time - tMin) / float(tMax - tMin);
    };

    uint64_t deNormalTime(float time, uint64_t tMin, uint64_t tMax) const
    {
        return uint64_t(time * float(tMax - tMin) + tMin);
    };

    void makeMesh(const TemporalTree& tree, std::shared_ptr<BasicMesh> meshBands, std::vector<BasicMesh::Vertex>& verticesBands,
        std::shared_ptr<BasicMesh> meshLines, std::vector<BasicMesh::Vertex>& verticesLines);

    void drawVertexPair(const float x, const float yLower, const float yUpper, const vec3& coefficients, const vec4& color,
        IndexBufferRAM& indexBufferBand, std::vector<BasicMesh::Vertex>& verticesBands);

    void drawLineVertex(const float x, const float y, IndexBufferRAM& indexBufferLine, 
        std::vector<BasicMesh::Vertex>& verticesLines);

//Ports
public:
    /// Tree for which we compute the meshes
    TemporalTreeInport portInTree;

    /// Mesh of the bands
    MeshOutport portOutMeshBands;

    /// Mesh of the bands
    MeshOutport portOutMeshLines;

//Properties
public:
    IntProperty propNumLeaves;

    CompositeProperty propTransitions;
    FloatProperty propMergeSplitBlend;

    CompositeProperty propLines;
    FloatVec4Property propColorLines;

    CompositeProperty propRenderInfo;
    BoolProperty propInterpretAsCoefficients;

//Attributes
private:

};

} // namespace kth

} // namespace
