/*********************************************************************
 *  Author  : Wiebke Koepp
 *  Init    : Wednesday, October 03, 2018 - 10:36:11
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
#include <modules/webbrowser/processors/webbrowserprocessor.h>
#include <modules/temporaltreemaps/datastructures/treeport.h>
//#include <inviwo/core/ports/volumeport.h>
//#include <inviwo/core/ports/meshport.h>
//#include <inviwo/core/properties/boolcompositeproperty.h>
//#include <inviwo/core/properties/boolproperty.h>
//#include <inviwo/core/properties/buttonproperty.h>
//#include <inviwo/core/properties/compositeproperty.h>
//#include <inviwo/core/properties/fileproperty.h>
//#include <inviwo/core/properties/minmaxproperty.h>
//#include <inviwo/core/properties/optionproperty.h>
//#include <inviwo/core/properties/ordinalproperty.h>
//#include <inviwo/core/properties/stringproperty.h>
//#include <inviwo/core/properties/transferfunctionproperty.h>

namespace inviwo
{
namespace kth
{

/** \docpage{org.inviwo.NTGRenderer, NTGRenderer}
    ![](org.inviwo.NTGRenderer.png?classIdentifier=org.inviwo.NTGRenderer)

    Explanation of how to use the processor.
    
    ### Inports
      * __<Inport1>__ <description>.
    
    ### Outports
      * __<Outport1>__ <description>.
    
    ### Properties
      * __<Prop1>__ <description>.
      * __<Prop2>__ <description>
*/


/** \class NTGRenderer
    \brief VERY_BRIEFLY_DESCRIBE_THE_PROCESSOR
    
    DESCRIBE_THE_PROCESSOR_FROM_A_DEVELOPER_PERSPECTIVE

    @author Wiebke Koepp
*/
class IVW_MODULE_TEMPORALTREEMAPS_API NTGRenderer : public WebBrowserProcessor
{ 
//Friends
//Types
public:

//Construction / Deconstruction
public:
    NTGRenderer();
    virtual ~NTGRenderer() = default;

//Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    ///Our main computation function
    virtual void process() override;

//Ports
public:
    TemporalTreeInport inTree;

//Properties
public:
    ///Property for tree to render
    StringProperty propTreeString;
    
    ///Properties for rendering a NTG, equivalent to the ones in JavaScript
    FloatProperty propXScale;
    FloatProperty propYScale;
    FloatProperty propWScale;
    //TemplateOptionProperty<colorbrewer::Colormap> propColorBrewerScheme;

    ///Additional properties for our ordering
    FloatProperty propExportWeightScale;

    ///Choosing between classic NTG order based on GraphVis and our order
    BoolProperty propForceClassic;

    ///Properties for the SVG
    //IntVec2Property propSvgDimensions;
    ///SVG Export
    //String property because fileproperties are not synched to HTML yet
    //ButtonProperty propSaveSVG;

//Attributes
private:

};

} // namespace
} // namespace
