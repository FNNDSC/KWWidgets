cmake_minimum_required(VERSION 2.6)

project(WidgetsTour)

# If we are not in the KWWidgets source tree, make sure we can find KWWidgets
# as an external package, and use it. If you are using this CMakeLists.txt 
# file to create your own application based on KWWidgets, you only need the
# FIND_PACKAGE(...) and INCLUDE(...) commands. 

if(NOT KWWidgets_SOURCE_DIR)
  find_package(KWWidgets REQUIRED)
  include(${KWWidgets_USE_FILE})
endif(NOT KWWidgets_SOURCE_DIR)

# The name of our targets (executable or libraries) will simply be based
# on the project name, with an extra prefix and suffix.

set(TARGET_BASE_NAME "KW${PROJECT_NAME}Example")

# We actually define a class in this example, and we want to be able to
# use its callbacks from our user interface. To do so, we need to create
# a library and wrap it automatically for the Tcl language.

set(LIB_NAME "${TARGET_BASE_NAME}Lib")
set(LIB_SRCS "vtk${TARGET_BASE_NAME}.cxx")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include("${KWWidgets_CMAKE_DIR}/KWWidgetsWrappingMacros.cmake")
kwwidgets_wrap_tcl(${LIB_NAME} LIB_TCL_SRCS "${LIB_SRCS}" "")

# Create the library. The library is built in static mode for convenience. 
# Check the 'Callbacks' example for more information about building it in
# shared mode, i.e. without the STATIC keyword (Win32 compilers requires
# an additional header file to setup DLL export symbols correctly).

add_library(${LIB_NAME} STATIC ${LIB_TCL_SRCS} ${LIB_SRCS})
target_link_libraries(${LIB_NAME} ${KWWidgets_LIBRARIES})

# The name of our executable and the corresponding source file.

set(EXE_NAME "${TARGET_BASE_NAME}")
set(EXE_SRCS "${EXE_NAME}.cxx")

# On Win32 platforms, let's create a .rc resource file to setup a decent
# application icon as well as some additional information in the "Version"
# tab of the properties panel.

if(WIN32 AND NOT BORLAND AND NOT CYGWIN)
  include("${KWWidgets_CMAKE_DIR}/KWWidgetsResourceMacros.cmake")
  set(RC_FILENAME "${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME}.rc")
  kwwidgets_create_rc_file(
    RC_FILENAME "${RC_FILENAME}"
    RC_APPLICATION_NAME "${EXE_NAME}"
    RC_COMPANY_NAME "Kitware, Inc.")
endif(WIN32 AND NOT BORLAND AND NOT CYGWIN)

# This example uses some files from the KWWidgets distribution tree.
# Let's configure KWWidgets's vtkKWWidgetsPaths.h.in into our
# own header file so that we can find the paths to KWWidgets files.

include_directories(${CMAKE_CURRENT_BINARY_DIR})
configure_file(
  ${KWWidgets_TEMPLATES_DIR}/vtkKWWidgetsPaths.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkKWWidgetsPaths.h)

# Now this gets tricky. We are going to iterate over all examples file
# in the Widgets sub-directory, and create a header file with the expected
# entry points for each one of them. This allows us to just drop examples
# in the Widgets sub-directory without having to modify an extra header
# file or a CMakeLists.txt file.

file(GLOB WIDGETS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/Widgets/*.cxx")
if(KWWidgets_BUILD_VTK_WIDGETS)
  file(GLOB WIDGETS_VTK_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/Widgets/VTK/*.cxx")
endif(KWWidgets_BUILD_VTK_WIDGETS)
foreach(file ${WIDGETS_SRCS} ${WIDGETS_VTK_SRCS})
  get_filename_component(filename ${file} NAME)
  get_filename_component(WIDGET_NAME ${file} NAME_WE)
  set(WIDGET_ENTRY_POINT "${WIDGET_NAME}EntryPoint")
  set(WIDGETS_FORWARD_DECL
    "${WIDGETS_FORWARD_DECL}
         extern KWWidgetsTourItem* ${WIDGET_ENTRY_POINT}();")
  set(WIDGETS_NODES
    "${WIDGETS_NODES}
         {\"${WIDGET_NAME}\", ${WIDGET_ENTRY_POINT}},")
endforeach(file)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/KWWidgetsTourExampleEntryPoints.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/KWWidgetsTourExampleEntryPoints.h
  IMMEDIATE)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# Create the executable, and link it against the KWWidgets libraries.

add_executable(${EXE_NAME} WIN32 
  ${EXE_SRCS} ${WIDGETS_SRCS} ${WIDGETS_VTK_SRCS} ${RC_FILENAME})
target_link_libraries(${EXE_NAME} ${LIB_NAME})

# If we are building this example as a standalone external project:
# - Generate a few small scripts (.bat, .sh, .csh) that can be sourced to set
# the various environments variables (PATH, TCLLIBPATH, LD_LIBRARY_PATH, etc.) 
# required by this executable and its known third-party dependencies (VTK, ITK,
# SOV, KWWidgets, etc.).
# - Generate a lightweight C launcher for this *specific* executable: It sets
# the above environment variables before launching the executable itself.

if(NOT KWWidgets_SOURCE_DIR)
  include("${KWWidgets_CMAKE_DIR}/KWWidgetsPathsMacros.cmake")
  kwwidgets_generate_setup_paths_scripts("${CMAKE_CURRENT_BINARY_DIR}")
  set(LAUNCHER_EXE_NAME "${EXE_NAME}Launcher")
  kwwidgets_generate_setup_paths_launcher(
    "${CMAKE_CURRENT_BINARY_DIR}" "${LAUNCHER_EXE_NAME}" "" "${EXE_NAME}")
endif(NOT KWWidgets_SOURCE_DIR)

# If needed, copy the Tcl/Tk support files required at run-time 
# to initialize Tcl/Tk. This is only triggered if VTK was built
# against a Tcl/Tk static library.

include("${KWWidgets_CMAKE_DIR}/KWWidgetsTclTkMacros.cmake")
if(NOT KWWidgets_SOURCE_DIR)
  kwwidgets_copy_tcl_tk_support_files("${PROJECT_BINARY_DIR}/lib")
endif(NOT KWWidgets_SOURCE_DIR)

# Install the example target. 
# If we are not building from the KWWidgets directory, install the Tcl/Tk
# support files as well.

install_targets(${KWWidgets_INSTALL_BIN_DIR} ${EXE_NAME})
if(NOT KWWidgets_SOURCE_DIR)
  kwwidgets_install_tcl_tk_support_files("/lib")
endif(NOT KWWidgets_SOURCE_DIR)

# Install all individual widgets examples. They are used by the executable
# to display the C++ and Tcl source code side-by-side.

install_files(
  "${KWWidgets_INSTALL_DATA_DIR}/Examples/Cxx/${PROJECT_NAME}/Widgets"
  .cxx ${WIDGETS_SRCS})
if(KWWidgets_BUILD_VTK_WIDGETS)
  install_files(
    "${KWWidgets_INSTALL_DATA_DIR}/Examples/Cxx/${PROJECT_NAME}/Widgets/VTK"
    .cxx ${WIDGETS_VTK_SRCS})
endif(KWWidgets_BUILD_VTK_WIDGETS)

# If we are not building from the KWWidgets directory, install the Tcl
# and Python example files as well.

if(NOT KWWidgets_SOURCE_DIR AND KWWidgets_EXAMPLES_DIR)
  file(GLOB TCL_FILES 
    "${KWWidgets_EXAMPLES_DIR}/Tcl/${PROJECT_NAME}/Widgets/*.tcl")
  install_files(
    "${KWWidgets_INSTALL_DATA_DIR}/Examples/Tcl/${PROJECT_NAME}/Widgets"
    FILES ${TCL_FILES})
  file(GLOB TCL_FILES 
    "${KWWidgets_EXAMPLES_DIR}/Tcl/${PROJECT_NAME}/Widgets/VTK/*.tcl")
  install_files(
    "${KWWidgets_INSTALL_DATA_DIR}/Examples/Tcl/${PROJECT_NAME}/Widgets/VTK"
    FILES ${TCL_FILES})
  file(GLOB PYTHON_FILES 
    "${KWWidgets_EXAMPLES_DIR}/Python/${PROJECT_NAME}/Widgets/*.py")
  install_files(
    "${KWWidgets_INSTALL_DATA_DIR}/Examples/Python/${PROJECT_NAME}/Widgets"
    FILES ${PYTHON_FILES})
  file(GLOB PYTHON_FILES 
    "${KWWidgets_EXAMPLES_DIR}/Python/${PROJECT_NAME}/Widgets/VTK/*.py")
  install_files(
    "${KWWidgets_INSTALL_DATA_DIR}/Examples/Python/${PROJECT_NAME}/Widgets/VTK"
    FILES ${PYTHON_FILES})
endif(NOT KWWidgets_SOURCE_DIR AND KWWidgets_EXAMPLES_DIR)

# If we are not building from the KWWidgets directory, install the common
# data as well.

if(NOT KWWidgets_SOURCE_DIR AND KWWidgets_EXAMPLES_DIR)
  file(GLOB VTP_FILES "${KWWidgets_EXAMPLES_DIR}/Data/*.vtp")
  install_files(
    "${KWWidgets_INSTALL_DATA_DIR}/Examples/Data" FILES ${VTP_FILES})
  file(GLOB VTI_FILES "${KWWidgets_EXAMPLES_DIR}/Data/*.vti")
  install_files(
    "${KWWidgets_INSTALL_DATA_DIR}/Examples/Data" FILES ${VTI_FILES})
  install_files(
    "${KWWidgets_INSTALL_DATA_DIR}/Examples/Resources" 
    FILES "${KWWidgets_EXAMPLES_DIR}/Resources/KWWidgetsSplashScreen.png")
endif(NOT KWWidgets_SOURCE_DIR AND KWWidgets_EXAMPLES_DIR)

# Register this example as a test. Our executable supports a --test
# configuration option so that it can be run non-interactively as a test.
# If you are using this CMakeLists.txt file to create your own application
# based on KWWidgets, you should omit this section, unless your application
# supports that feature too and you checked how the macro is working.

if(BUILD_TESTING AND KWWidgets_BUILD_TESTING)
  include("${KWWidgets_CMAKE_DIR}/KWWidgetsTestingMacros.cmake")
  kwwidgets_add_test_from_c_example(KWWidgets-${PROJECT_NAME} ${EXE_NAME})

  if(KWWidgets_USE_Squish AND SQUISH_FOUND)
    set(Squish_TST_NAME "${KWWidgets_SOURCE_DIR}/Examples/Cxx/${PROJECT_NAME}/Testing/Squish/suite_KWWTourExample/tst_BrowseCoreWidgets")
    kwwidgets_add_squish_test_from_c_example(
      KWWidgets-${PROJECT_NAME}Squish ${EXE_NAME} "${Squish_TST_NAME}")    
  endif(KWWidgets_USE_Squish AND SQUISH_FOUND)

endif(BUILD_TESTING AND KWWidgets_BUILD_TESTING)
