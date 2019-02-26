#--------------------------------------------------------------------
# Inviwo TemporalTreeMaps Module
ivw_module(TemporalTreeMaps)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/modules/temporaltreemaps/datastructures/constraint.h
    include/modules/temporaltreemaps/datastructures/cushion.h
    include/modules/temporaltreemaps/datastructures/tree.h
    include/modules/temporaltreemaps/datastructures/treecolor.h
    include/modules/temporaltreemaps/datastructures/treejsonreader.h
    include/modules/temporaltreemaps/datastructures/treeorder.h
    include/modules/temporaltreemaps/datastructures/treeport.h
    include/modules/temporaltreemaps/processors/ntgrenderer.h
    include/modules/temporaltreemaps/processors/treecoloring.h
    include/modules/temporaltreemaps/processors/treeconsistencycheck.h
    include/modules/temporaltreemaps/processors/treecushioncomputation.h
    include/modules/temporaltreemaps/processors/treefilter.h
    include/modules/temporaltreemaps/processors/treegeneratefromcsv.h
    include/modules/temporaltreemaps/processors/treegeneratefromfilesystem.h
    include/modules/temporaltreemaps/processors/treegeneratefromgit.h
    include/modules/temporaltreemaps/processors/treegeneratefromtrackinggraph.h
    include/modules/temporaltreemaps/processors/treelayoutcomputation.h
    include/modules/temporaltreemaps/processors/treelayoutrenderer.h
    include/modules/temporaltreemaps/processors/treemeshgenerator.h
    include/modules/temporaltreemaps/processors/treemeshgeneratortopo.h
    include/modules/temporaltreemaps/processors/treeordercomputation.h
    include/modules/temporaltreemaps/processors/treeordercomputationgreedy.h
    include/modules/temporaltreemaps/processors/treeordercomputationheuristic.h
    include/modules/temporaltreemaps/processors/treeordercomputationsa.h
    include/modules/temporaltreemaps/processors/treeordercomputationsaconstraints.h
    include/modules/temporaltreemaps/processors/treeordercomputationsaedges.h
    include/modules/temporaltreemaps/processors/treeordercomputationsanodes.h
    include/modules/temporaltreemaps/processors/treesource.h
    include/modules/temporaltreemaps/processors/treestatistics.h
    include/modules/temporaltreemaps/processors/treewriter.h
    include/modules/temporaltreemaps/temporaltreemapsmodule.h
    include/modules/temporaltreemaps/temporaltreemapsmoduledefine.h

)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/datastructures/constraint.cpp
    src/datastructures/cushion.cpp
    src/datastructures/tree.cpp
    src/datastructures/treecolor.cpp
    src/datastructures/treejsonreader.cpp
    src/datastructures/treeorder.cpp
    src/processors/ntgrenderer.cpp
    src/processors/treecoloring.cpp
    src/processors/treeconsistencycheck.cpp
    src/processors/treecushioncomputation.cpp
    src/processors/treefilter.cpp
    src/processors/treegeneratefromcsv.cpp
    src/processors/treegeneratefromfilesystem.cpp
    src/processors/treegeneratefromgit.cpp
    src/processors/treegeneratefromtrackinggraph.cpp
    src/processors/treelayoutcomputation.cpp
    src/processors/treelayoutrenderer.cpp
    src/processors/treemeshgenerator.cpp
    src/processors/treemeshgeneratortopo.cpp
    src/processors/treeordercomputation.cpp
    src/processors/treeordercomputationgreedy.cpp
    src/processors/treeordercomputationheuristic.cpp
    src/processors/treeordercomputationsa.cpp
    src/processors/treeordercomputationsaconstraints.cpp
    src/processors/treeordercomputationsaedges.cpp
    src/processors/treeordercomputationsanodes.cpp
    src/processors/treesource.cpp
    src/processors/treestatistics.cpp
    src/processors/treewriter.cpp
    src/temporaltreemapsmodule.cpp

)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    glsl/treelayoutrenderer.frag
    glsl/treelayoutrenderer.geom
    glsl/treelayoutrenderer.vert
    glsl/treelinerenderer.vert
    glsl/treelinerenderer.frag
)
ivw_group("Shader Files" ${SHADER_FILES})


#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
	#tests/unittests/kxtrees-unittest-main.cpp
	#tests/unittests/kxtrees-test.cpp
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

target_link_libraries(inviwo-module-temporaltreemaps PUBLIC
    nlohmann_json::nlohmann_json
)

#--------------------------------------------------------------------
# Add shader directory to pack
# ivw_add_to_module_pack(glsl)
