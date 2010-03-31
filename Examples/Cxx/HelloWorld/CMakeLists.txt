cmake_minimum_required(VERSION 2.6)

project(HelloWorld)

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

# Create the executable, and link it against the KWWidgets libraries.

add_executable(${EXE_NAME} WIN32 ${EXE_SRCS} ${RC_FILENAME})
target_link_libraries(${EXE_NAME} ${KWWidgets_LIBRARIES})

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

# Register this example as a test. Our executable supports a --test
# configuration option so that it can be run non-interactively as a test.
# If you are using this CMakeLists.txt file to create your own application
# based on KWWidgets, you should omit this section, unless your application
# supports that feature too and you checked how the macro is working.

if(BUILD_TESTING AND KWWidgets_BUILD_TESTING)
  include("${KWWidgets_CMAKE_DIR}/KWWidgetsTestingMacros.cmake")
  kwwidgets_add_test_from_c_example(KWWidgets-${PROJECT_NAME} ${EXE_NAME})

  if(KWWidgets_USE_Squish AND SQUISH_FOUND)
    set(Squish_TST_NAME "${KWWidgets_SOURCE_DIR}/Examples/Cxx/${PROJECT_NAME}/Testing/Squish/suite_KWWHellowWorldExample/tst_StartAbout")
    kwwidgets_add_squish_test_from_c_example(
      KWWidgets-${PROJECT_NAME}Squish ${EXE_NAME} "${Squish_TST_NAME}")    
  endif(KWWidgets_USE_Squish AND SQUISH_FOUND)

endif(BUILD_TESTING AND KWWidgets_BUILD_TESTING)
