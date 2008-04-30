# Configuration options that can be exported

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkKWWidgetsConfigure.h.in 
  ${KWWidgets_BINARY_DIR}/vtkKWWidgetsConfigure.h)

# Configuration options that are relevant to the build tree only

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkKWWidgetsBuildConfigure.h.in 
  ${KWWidgets_BINARY_DIR}/vtkKWWidgetsBuildConfigure.h)

if(NOT KWWidgets_INSTALL_NO_DEVELOPMENT)
  install_files(${KWWidgets_INSTALL_INCLUDE_DIR} FILES 
    ${KWWidgets_BINARY_DIR}/vtkKWWidgetsConfigure.h)
  install_files(${KWWidgets_INSTALL_INCLUDE_DIR}/Templates FILES 
    KWWidgetsContents.in
    KWWidgetsSetupPaths.bat.in
    KWWidgetsSetupPaths.cmake.in
    KWWidgetsSetupPaths.csh.in
    KWWidgetsSetupPaths.sh.in
    KWWidgetsSetupPathsLauncher.c.in
    vtkKWWidgetSetSubclass.cxx.in
    vtkKWWidgetSetSubclass.h.in
    vtkKWWidgetsPaths.h.in
    vtkKWWidgetWithLabelSubclass.cxx.in
    vtkKWWidgetWithLabelSubclass.h.in
    vtkKWWidgetWithScrollbarsSubclass.cxx.in
    vtkKWWidgetWithScrollbarsSubclass.h.in
    vtkKWWidgetWithSpinButtonsSubclass.cxx.in
    vtkKWWidgetWithSpinButtonsSubclass.h.in)
endif(NOT KWWidgets_INSTALL_NO_DEVELOPMENT)
