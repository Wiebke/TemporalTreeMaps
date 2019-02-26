/*********************************************************************
 *  Author  : Wiebke K�pp
 *  Init    : Monday, October 09, 2017 - 16:29:42
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <modules/temporaltreemaps/temporaltreemapsmodule.h>
#include <modules/opengl/shader/shadermanager.h>
#include <modules/temporaltreemaps/datastructures/treejsonreader.h>
#include <modules/temporaltreemaps/processors/treewriter.h>
#include <modules/temporaltreemaps/processors/treesource.h>
#include <modules/temporaltreemaps/processors/treegeneratefromfilesystem.h>
#include <modules/temporaltreemaps/processors/treegeneratefromgit.h>
#include <modules/temporaltreemaps/processors/treelayoutrenderer.h>
#include <modules/temporaltreemaps/processors/treeconsistencycheck.h>
#include <modules/temporaltreemaps/processors/treeordercomputation.h>
#include <modules/temporaltreemaps/processors/treefilter.h>
#include <modules/temporaltreemaps/processors/treecoloring.h>
#include <modules/temporaltreemaps/processors/treegeneratefromtrackinggraph.h>
#include <modules/temporaltreemaps/processors/treegeneratefromcsv.h>
#include <modules/temporaltreemaps/processors/treemeshgenerator.h>
#include <modules/temporaltreemaps/processors/treemeshgeneratortopo.h>
#include <modules/temporaltreemaps/processors/treelayoutcomputation.h>
#include <modules/temporaltreemaps/processors/treecushioncomputation.h>
#include <modules/temporaltreemaps/processors/treestatistics.h>
#include <modules/temporaltreemaps/processors/treeordercomputationheuristic.h>
#include <modules/temporaltreemaps/processors/treeordercomputationsaconstraints.h>
#include <modules/temporaltreemaps/processors/treeordercomputationsanodes.h>
#include <modules/temporaltreemaps/processors/treeordercomputationsaedges.h>
#include <modules/temporaltreemaps/processors/treeordercomputationgreedy.h>
#include <modules/temporaltreemaps/processors/ntgrenderer.h>


namespace inviwo
{

using namespace kth;
    
TemporalTreeMapsModule::TemporalTreeMapsModule(InviwoApplication* app) : InviwoModule(app, "TemporalTreeMaps")
{
    // Add a directory to the search path of the Shadermanager
    ShaderManager::getPtr()->addShaderSearchPath(getPath(ModulePath::GLSL));

    // Register objects that can be shared with the rest of inviwo here:
    
    // Processors
    registerProcessor<TemporalTreeWriter>();
    registerProcessor<TemporalTreeSource>();
    registerProcessor<TemporalTreeGenerateFromFileSystem>();
    registerProcessor<TemporalTreeGenerateFromGit>();
    registerProcessor<TemporalTreeLayoutRenderer>();
    registerProcessor<TemporalTreeConsistencyCheck>();
    registerProcessor<TemporalTreeFilter>();
    registerProcessor<TemporalTreeColoring>();
    registerProcessor<TemporalTreeGenerateFromTrackingGraph>();
    registerProcessor<TemporalTreeMeshGenerator>();
    registerProcessor<TemporalTreeMeshGeneratorTopo>();
    registerProcessor<TemporalTreeLayoutComputation>();
    registerProcessor<TemporalTreeCushionComputation>();
    registerProcessor<TemporalTreeGenerateFromCSV>();
    registerProcessor<TemporalTreeStatistics>();
    registerProcessor<TemporalTreeOrderComputationHeuristic>();
    registerProcessor<TemporalTreeOrderComputationSAEdges>();
    registerProcessor<TemporalTreeOrderComputationSAConstraints>();
    registerProcessor<TemporalTreeOrderComputationSANodes>();
    registerProcessor<TemporalTreeOrderComputationGreedy>();
    registerProcessor<NTGRenderer>();
    
    // Properties
    // registerProperty<modules/temporaltreemapsProperty>();
    
    // Readers and writes
    registerDataReader(util::make_unique<TemporalTreeJSONReader>());
    registerDataReader(util::make_unique<TemporalTreeJSONReaderCBOR>());
    registerDataReader(util::make_unique<TemporalTreeJSONReaderMsgPack>());
    registerDataReader(util::make_unique<TemporalTreeJSONReaderNTG>());
   

    // Ports
    registerPort<TemporalTreeOutport>();
    registerPort<TemporalTreeInport>();
}

} // namespace
