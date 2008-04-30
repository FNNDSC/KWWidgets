# Handle the subdirectories

subdirs(Tcl)

if(VTK_WRAP_PYTHON)
  subdirs(Python)
endif(VTK_WRAP_PYTHON)

# Generate a few small scripts (.bat, .sh, .csh) that can be sourced to set
# the various environments variables (PATH, TCLLIBPATH, LD_LIBRARY_PATH, etc.) 
# required by this executable and its known third-party dependencies (VTK, ITK,
# SOV, KWWidgets, etc.).
# Also Generate a lightweight C launcher for testing purposes. This 
# launcher sets the above environment variables before launching an executable.

if(KWWidgets_USE_SOVIEWER AND SOV_FOUND)
  include(${SOV_USE_FILE})
endif(KWWidgets_USE_SOVIEWER AND SOV_FOUND)

include("${KWWidgets_CMAKE_DIR}/KWWidgetsPathsMacros.cmake")
kwwidgets_generate_setup_paths_scripts("${EXECUTABLE_OUTPUT_PATH}")
if(BUILD_TESTING AND KWWidgets_BUILD_TESTING)
  kwwidgets_generate_setup_paths_launcher(
    "${CMAKE_CURRENT_BINARY_DIR}" "KWWidgetsSetupPathsLauncher" "" "")
endif(BUILD_TESTING AND KWWidgets_BUILD_TESTING)
