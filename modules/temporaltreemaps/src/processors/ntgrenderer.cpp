/*********************************************************************
 *  Author  : Wiebke Koepp
 *  Init    : Wednesday, October 03, 2018 - 10:36:11
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <inviwo/core/util/filesystem.h>
#include <modules/temporaltreemaps/processors/ntgrenderer.h>
#include <modules/temporaltreemaps/temporaltreemapsmodule.h>
#include <modules/temporaltreemaps/processors/treewriter.h>
#include <modules/webbrowser/properties/propertycefsynchronizer.h>
#include <modules/webbrowser/interaction/cefinteractionhandler.h>
#include <modules/webbrowser/webbrowserclient.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;


namespace inviwo
{
namespace kth
{

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo NTGRenderer::processorInfo_
{
    "org.inviwo.NTGRenderer",      // Class identifier
    "NTGRenderer",                // Display name
    "Temporal Tree",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};

const ProcessorInfo NTGRenderer::getProcessorInfo() const
{
    return processorInfo_;
}


NTGRenderer::NTGRenderer()
    :WebBrowserProcessor()
    , inTree("inTree")
    , propTreeString("treeString", "Tree String", "")
    , propXScale("xScale", "X Scale", 1.0f, 0.0f, 5.0f, 0.01f)
    , propYScale("yScale", "Y Scale", 1.0f, 0.0f, 5.0f, 0.01f)
    , propWScale("wScale", "W Scale", 1.0f, 0.0f, 1.0f, 0.01f)
    , propExportWeightScale("exportWScale", "Export Scale", 1.0f, 0.000001f, 1.0f, 0.1f)
    , propForceClassic("forceClassic", "Classic NTG Layout", false)
    //, propColorBrewerScheme("colorBrewerScheme", "Color Brewer Scheme")
    //, propSvgDimensions("svgDimensions", "Output Dimensions")
    //, propSVGString("svgString", "SVG String", "")
    //, propSVGFileName("svgFileName", "File Name", "")
    //, propSaveSVG("saveSVG", "Save SVG")
{
    // Ports
    addPort(inTree);

    addProperty(propExportWeightScale);

    addProperty(propTreeString);
    //propTreeString.setReadOnly(true);
    propTreeString.setSemantics(PropertySemantics::TextEditor);

    inTree.onChange([&]()
    {
        if (!inTree.hasData())
        {
            return;
        }
        std::shared_ptr<const TemporalTree> InTree = inTree.getData();
        
        json jAll = TemporalTreeWriter::createJSON(InTree, true, true, true, propExportWeightScale.get());

        std::stringstream ss;
        ss << jAll;

        std::string treeJSON(ss.str());
        propTreeString.set(treeJSON);

    });
    
    addProperty(propForceClassic);
    addProperty(propXScale);
    addProperty(propYScale);
    addProperty(propWScale);
    //addProperty(propColorBrewerScheme);
    //addProperty(propSvgDimensions);

    auto path = InviwoApplication::getPtr()->getModuleByType<TemporalTreeMapsModule>()->getPath(ModulePath::Data) + "/webpage/index.html";
    if (!filesystem::fileExists(path))
    {
        throw Exception("Could not find " + path);
    }

    fileName_.set(path);

    // Hide/Remove dynamic properties from the webbrowser we do not need
    removeProperty("addProperty");
    //util::hide(fileName_, sourceType_);

    // TODO: How do we setup properties here such that they are synchronized?
    // browserClient_->propertyCefSynchronizer_->startSynchronize(&propXScale);
}

void NTGRenderer::process()
{
    WebBrowserProcessor::process();
}

} // namespace
} // namespace

