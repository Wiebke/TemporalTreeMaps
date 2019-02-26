/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Monday, December 11, 2017 - 19:50:26
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#pragma once

#include <modules/temporaltreemaps/temporaltreemapsmoduledefine.h>
#include <modules/temporaltreemaps/datastructures/treeport.h>
#include <modules/temporaltreemaps/datastructures/treeorder.h>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
//#include <inviwo/core/ports/volumeport.h>
//#include <inviwo/core/ports/meshport.h>
//#include <inviwo/core/properties/boolcompositeproperty.h>
//#include <inviwo/core/properties/boolproperty.h>
//#include <inviwo/core/properties/buttonproperty.h>
//#include <inviwo/core/properties/compositeproperty.h>
//#include <inviwo/core/properties/fileproperty.h>
//#include <inviwo/core/properties/minmaxproperty.h>
//#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
//#include <inviwo/core/properties/stringproperty.h>
//#include <inviwo/core/properties/transferfunctionproperty.h>

namespace inviwo
{
namespace kth
{

/** \docpage{org.inviwo.TemporalTreeMeshGeneratorTopo, Tree Mesh Generator Topo}
    ![](org.inviwo.TemporalTreeMeshGeneratorTopo.png?classIdentifier=org.inviwo.TemporalTreeMeshGeneratorTopo)

    Explanation of how to use the processor.
    
    ### Inports
      * __<Inport1>__ <description>.
    
    ### Outports
      * __<Outport1>__ <description>.
    
    ### Properties
      * __<Prop1>__ <description>.
      * __<Prop2>__ <description>
*/


/** \class TemporalTreeMeshGeneratorTopo
    \brief VERY_BRIEFLY_DESCRIBE_THE_PROCESSOR
    
    DESCRIBE_THE_PROCESSOR_FROM_A_DEVELOPER_PERSPECTIVE

    @author Tino Weinkauf and Wiebke Koepp
*/
class IVW_MODULE_TEMPORALTREEMAPS_API TemporalTreeMeshGeneratorTopo : public Processor
{ 
//Friends
//Types
public:
    struct TLayerOrderItem
    {
        size_t OrderRow;
        bool bFullCoverage;
        size_t idxLeaf;
        size_t idxNodeToBeDrawn;
    };

    typedef std::vector<TLayerOrderItem> TLayerOrder;

//Construction / Deconstruction
public:
    TemporalTreeMeshGeneratorTopo();
    virtual ~TemporalTreeMeshGeneratorTopo() = default;

//Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    ///Our main computation function
    virtual void process() override;

    ///Creates the actual mesh
    void CreateMesh(const TemporalTree& Tree, const size_t Level, const size_t MaxLevel,
                    const size_t NumRows, const TLayerOrder& Order,
                    std::shared_ptr<BasicMesh>& Mesh, std::vector<BasicMesh::Vertex>& Vertices);

//Ports
public:
    ///Tree to be visualized
    TemporalTreeInport portInTree;

    ///Mesh output
    MeshOutport portOutMeshBands;

//Properties
public:
    ///Space between bands
    FloatProperty propSpacing;

    ///The amount of space in a band to be reserved for the level drawing.
    FloatProperty propLevelPortion;

    ///Size of the constraint indication
    FloatProperty propConstraintIndication;

//Attributes
private:

};

} // namespace kth
} // namespace
