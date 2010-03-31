cmake_minimum_required(VERSION 2.6)

project(Tutorial1)

# Find KWWidgets

find_package(KWWidgets REQUIRED)
include(${KWWidgets_USE_FILE})

# The name of our executable and the corresponding source files.

set(STEP "1" CACHE STRING "Select which step of the tutorial to build")

set(EXE_NAME "KWTutorial1")
set(EXE_SRCS 
  "${EXE_NAME}.cxx"
  ${CMAKE_CURRENT_SOURCE_DIR}/Step${STEP}/vtkKWMyWidget.cxx
  )

# Create the executable, and link it against the KWWidgets libraries

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_SOURCE_DIR}/Step${STEP}
  )
add_executable(${EXE_NAME} WIN32 ${EXE_SRCS})
target_link_libraries(${EXE_NAME} ${KWWidgets_LIBRARIES})

# Since this tutorial is a standalone external project:
# - Generate a few small scripts (.bat, .sh, .csh) that can be sourced to set
# the various environments variables (PATH, TCLLIBPATH, LD_LIBRARY_PATH, etc.) 
# required by this executable and its known third-party dependencies (VTK, ITK,
# SOV, KWWidgets, etc.).
# - Generate a lightweight C launcher for this *specific* executable: It sets
# the above environment variables before launching the executable itself.

include("${KWWidgets_CMAKE_DIR}/KWWidgetsPathsMacros.cmake")
kwwidgets_generate_setup_paths_scripts("${CMAKE_CURRENT_BINARY_DIR}")
set(LAUNCHER_EXE_NAME "${EXE_NAME}Launcher")
kwwidgets_generate_setup_paths_launcher(
  "${CMAKE_CURRENT_BINARY_DIR}" "${LAUNCHER_EXE_NAME}" "" "${EXE_NAME}")

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
