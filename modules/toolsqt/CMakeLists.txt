#--------------------------------------------------------------------
# Tools Qt Widgets
ivw_module(ToolsQt)

find_package(Qt5Core QUIET REQUIRED)
find_package(Qt5Gui QUIET REQUIRED)
find_package(Qt5Widgets QUIET REQUIRED)

#--------------------------------------------------------------------
# Add MOC files
set(MOC_FILES
    include/modules/toolsqt/filelistpropertywidgetqt.h
)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/modules/toolsqt/toolsqtmoduledefine.h
    include/modules/toolsqt/toolsqtmodule.h
    ${MOC_FILES}
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/filelistpropertywidgetqt.cpp
    src/toolsqtmodule.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Create library
qt5_wrap_cpp(MOCED_FILES ${MOC_FILES})
source_group("MOC Files" FILES ${MOCED_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${MOCED_FILES} ${HEADER_FILES})
target_link_libraries(inviwo-module-toolsqt PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets)
ivw_make_package(InviwoToolsQtModule inviwo-module-toolsqt)
ivw_qt_add_to_install(toolsqt Qt5Core Qt5Gui Qt5Widgets)
