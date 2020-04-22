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
#include <modules/temporaltreemaps/processors/treewriter.h>
#include <modules/temporaltreemaps/temporaltreemapsmodule.h>
#include <modules/webbrowser/interaction/cefinteractionhandler.h>
#include <modules/webbrowser/properties/propertycefsynchronizer.h>
#include <modules/webbrowser/webbrowserclient.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace inviwo {
namespace kth {

// The Class Identifier has to be globally unique. Use a reverse DNS naming
// scheme
const ProcessorInfo NTGRenderer::processorInfo_{
    "org.inviwo.NTGRenderer",  // Class identifier
    "NTGRenderer",             // Display name
    "Temporal Tree",           // Category
    CodeState::Experimental,   // Code state
    Tags::None,                // Tags
};

const ProcessorInfo NTGRenderer::getProcessorInfo() const { return processorInfo_; }

NTGRenderer::NTGRenderer()
    : WebBrowserProcessor()
    , inTree("inTree")
    , propExportWeightScale("exportWScale", "Tree Export W Scale", 1.0f, 0.000001f, 1.0f, 0.1f)
    , propTreeString("treeString", "Tree String", "")
    , propXScale("xScale", "X Scale", 1.0f, 0.0f, 5.0f, 0.01f)
    , propYScale("yScale", "Y Scale", 1.0f, 0.0f, 5.0f, 0.01f)
    , propWScale("wScale", "W Scale", 1.0f, 0.0f, 1.0f, 0.01f)
    , propForceClassic("forceClassic", "Classic NTG Layout", false)
    , propColorBrewerScheme("colorBrewerScheme", "Color Brewer Scheme")
    , propSvgDimensions("svgDimensions", "Output Dimensions")
    , propSvgX("svgX", "Output X", 400, 100, 2000)
    , propSvgY("svgY", "Output Y", 800, 100, 2000)
    , propSvgString("svgString", "SVG String", "")
    , propSvgFilename("svgFileName", "File Name", "")
    , propSaveSvg("saveSvg", "Save SVG")
    , propOverwriteSvg("overWriteSvg", "Overwrite", false) {
    // Ports
    addPort(inTree);

    addProperty(propExportWeightScale);

    addProperty(propTreeString);
    propTreeString.setReadOnly(true);
    propTreeString.setSemantics(PropertySemantics::TextEditor);

    inTree.onChange([&]() {
        if (!inTree.hasData()) {
            return;
        }
        std::shared_ptr<const TemporalTree> InTree = inTree.getData();

        json jAll =
            TemporalTreeWriter::createJSON(InTree, true, true, true, propExportWeightScale.get());

        std::stringstream ss;
        ss << jAll;

        std::string treeJSON(ss.str());
        propTreeString.set(treeJSON);
    });

    addProperty(propForceClassic);
    addProperty(propXScale);
    addProperty(propYScale);
    addProperty(propWScale);
    addProperty(propColorBrewerScheme);

    propColorBrewerScheme.addOption("YlOrRd_6", "6-class YlOrRd", 0);
    propColorBrewerScheme.addOption("YlGnBu_6", "6-class YlGnBu", 1);
    propColorBrewerScheme.addOption("YlOrRd_6", "6-class PuBuGn", 2);
    propColorBrewerScheme.addOption("Blues_3", "3-class blue", 3);
    propColorBrewerScheme.addOption("Greens_3", "3-class green", 4);

    addProperty(propSvgDimensions);
    addProperty(propSvgX);
    addProperty(propSvgY);
    propSvgDimensions.setReadOnly(true);
    propSvgX.setReadOnly(true);
    propSvgY.setReadOnly(true);

    propSvgDimensions.onChange([&]() {
        auto dims = propSvgDimensions.get();
        propSvgX.set(dims.x);
        propSvgY.set(dims.y);
    });

    addProperty(propSvgString);
    propSvgString.setReadOnly(true);
    propSvgString.setSemantics(PropertySemantics::TextEditor);

    addProperty(propSvgFilename);
    addProperty(propSaveSvg);
    addProperty(propOverwriteSvg);
    propSaveSvg.onChange([&]() { saveSvg(); });

    auto path = InviwoApplication::getPtr()->getModuleByType<TemporalTreeMapsModule>()->getPath(
                    ModulePath::Data) +
                "/webpage/index.html";
    if (!filesystem::fileExists(path)) {
        throw Exception("Could not find " + path);
    }

    fileName_.set(path);

    // Hide/Remove dynamic properties from the webbrowser we do not need
    removeProperty("addProperty");
    // util::hide(fileName_, sourceType_);
}

void NTGRenderer::saveSvg() {

    // Get filename and open file
    const std::string& Filename = propSvgFilename.get();

    if (filesystem::fileExists(Filename) && !propOverwriteSvg.get()) {
        LogWarn("File already exists: " << Filename);
        return;
    }

    std::ofstream outfile;
    outfile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    try {
        outfile.open(Filename);
    } catch (const std::ofstream::failure& e) {
        LogError("File could not be opened: " << Filename);
        LogError("  Error Code: " << e.code() << "    . " << e.what());
        return;
    }

    // Stream it out as ASCII or Binary
    try {
        std::string svgString = propSvgString.get();

        outfile << svgString << std::endl;

    } catch (const std::ofstream::failure& e) {
        LogError("Error during save: " << Filename);
        LogError("  Error Code: " << e.code() << "    . " << e.what());
        return;
    }

    outfile.close();
}

void NTGRenderer::process() { WebBrowserProcessor::process(); }

}  // namespace kth
}  // namespace inviwo
