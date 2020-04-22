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

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <modules/temporaltreemaps/datastructures/treeport.h>
#include <modules/temporaltreemaps/temporaltreemapsmoduledefine.h>
#include <modules/webbrowser/processors/webbrowserprocessor.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>

namespace inviwo {
namespace kth {

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
class IVW_MODULE_TEMPORALTREEMAPS_API NTGRenderer : public WebBrowserProcessor {
    // Friends
    // Types
public:
    // Construction / Deconstruction
public:
    NTGRenderer();
    virtual ~NTGRenderer() = default;

    // Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    void saveSvg();

    /// Our main computation function
    virtual void process() override;

    // Ports
public:
    TemporalTreeInport inTree;

    // Properties
public:
    /// Additional properties for our ordering
    FloatProperty propExportWeightScale;

    /// Property for tree to render
    StringProperty propTreeString;

    /// Properties for rendering a NTG, equivalent to the ones in JavaScript
    FloatProperty propXScale;
    FloatProperty propYScale;
    FloatProperty propWScale;
    // Choosing between classic NTG order based on GraphVis and our order
    BoolProperty propForceClassic;
    OptionPropertyInt propColorBrewerScheme;

    /// Properties for the SVG
    // For linking with camera dimensions
    IntVec2Property propSvgDimensions;
    IntProperty propSvgX;
    IntProperty propSvgY;
    /// SVG Export
    // String property because fileproperties are not synched to HTML yet
    StringProperty propSvgString;
    FileProperty propSvgFilename;
    ButtonProperty propSaveSvg;
    BoolProperty propOverwriteSvg;

    // Attributes
private:
};

}  // namespace kth
}  // namespace inviwo
