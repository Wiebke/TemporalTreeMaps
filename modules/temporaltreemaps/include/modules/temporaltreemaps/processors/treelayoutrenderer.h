/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Wednesday, November 22, 2017 - 17:26:10
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
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <modules/opengl/shader/shader.h>

#include <modules/plotting/properties/axisproperty.h>
#include <modules/plottinggl/utils/axisrenderer.h>


namespace inviwo
{
namespace kth
{

/** \docpage{org.inviwo.TemporalTreeLayoutRenderer, Tree Layout Renderer}
    ![](org.inviwo.TemporalTreeLayoutRenderer.png?classIdentifier=org.inviwo.TemporalTreeLayoutRenderer)

    Explanation of how to use the processor.
    
    ### Inports
      * __<Inport1>__ <description>.
    
    ### Outports
      * __<Outport1>__ <description>.
    
    ### Properties
      * __<Prop1>__ <description>.
      * __<Prop2>__ <description>
*/


/** \class TemporalTreeLayoutRenderer
    \brief Render cushion trees and their borders

    @author Tino Weinkauf and Wiebke Koepp
*/
class IVW_MODULE_TEMPORALTREEMAPS_API TemporalTreeLayoutRenderer : public Processor
{ 
// Friends
// Types
public:

// Construction / Deconstruction
public:
    TemporalTreeLayoutRenderer();
    virtual ~TemporalTreeLayoutRenderer() = default;

// Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    void updateMeshs();

protected:

    /// Our main computation function
    virtual void process() override;

//Ports
public:
    /// Bands input mesh
    MeshInport portInMeshBands;

    /// Line input mesh
    MeshInport portInMeshLines;

    /// Later: Do the mesh rendering directly here
    ImageInport portInImage;

    /// Later: Do the mesh rendering directly here
    ImageOutport portOutImage;
// Properties
public:

    /// Corner positions
    FloatProperty top, bottom, left, right;

    /// World position of light
    FloatVec3Property propLightPosition;

    BoolProperty propInterpretAsCoefficients;

    FloatVec4Property propAmbientLight; 
    FloatVec4Property propDiffuseLight;

    CompositeProperty propAxes;

    plot::AxisProperty propXAxis;
    FloatProperty propXMargin;

    plot::AxisProperty propYAxis;
    FloatProperty propYMargin;


// Attributes
protected:
	/// Custom tree shader
	Shader bandShader;

	/// Custom band shader
	Shader lineShader;

	/// The two meshes to be drawn
	std::unique_ptr<MeshDrawer> bandMesh, lineMesh;

	/// Renderers for the two axis
    std::array<plot::AxisRenderer, 2> axisRenderers;

};

} // namespace kth
} // namespace
