#--------------------------------------------------------------------
# Inviwo KTH Tools Module
ivw_module(Tools)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/modules/tools/amirameshvolumereader.h
    include/modules/tools/filelistproperty.h
    include/modules/tools/iotexthelpers.h
    include/modules/tools/performancetimer.h
    include/modules/tools/setminmaxdatamap.h
    include/modules/tools/volumeseriesfromslices.h
    include/modules/tools/volumesourceseries.h
    include/modules/tools/volumesourceseriesdata.h
    include/modules/tools/toolsmodule.h
    include/modules/tools/toolsmoduledefine.h
    include/modules/tools/simplelrucache.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/amirameshvolumereader.cpp
    src/filelistproperty.cpp
    src/iotexthelpers.cpp
    src/performancetimer.cpp
    src/setminmaxdatamap.cpp
    src/volumeseriesfromslices.cpp
    src/volumesourceseries.cpp
    src/volumesourceseriesdata.cpp
    src/toolsmodule.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})
