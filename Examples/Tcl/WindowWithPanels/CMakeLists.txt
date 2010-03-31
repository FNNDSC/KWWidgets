cmake_minimum_required(VERSION 2.6)

project(WindowWithPanels)

# If we are not in the KWWidgets source tree, make sure we can find KWWidgets
# as an external package, and use it. If you are using this CMakeLists.txt 
# file to create your own application based on KWWidgets, you only need the
# FIND_PACKAGE(...) and INCLUDE(...) commands. 

if (NOT KWWidgets_SOURCE_DIR)
  find_package(KWWidgets REQUIRED)
  include(${KWWidgets_USE_FILE})
endif (NOT KWWidgets_SOURCE_DIR)

# The name of our targets (scripts, mainly) will simply be based
# on the project name, with an extra prefix and suffix.

set(TARGET_BASE_NAME "KW${PROJECT_NAME}Example")

# The name of our executable script (and additional helper scripts if any).

set(EXE_NAME "${TARGET_BASE_NAME}")
set(EXE_SRCS "${EXE_NAME}.tcl")

# Install the scripts in the KWWidgets examples section. If you are
# using this CMakeLists.txt file to create your own application based on
# KWWidgets, you can most likely omit this section, unless you want to 
# install your scripts right where KWWidgets is already installed.

install_files(
  "${KWWidgets_INSTALL_DATA_DIR}/Examples/Tcl/${PROJECT_NAME}"
  .tcl ${EXE_SRCS})

# Register this example as a test. Our script supports a --test
# configuration option so that it can be run non-interactively as a test.
# If you are using this CMakeLists.txt file to create your own application
# based on KWWidgets, you should omit this section, unless your application
# supports that feature too and you checked how the macro is working.

if(BUILD_TESTING AND KWWidgets_BUILD_TESTING)
  include("${KWWidgets_CMAKE_DIR}/KWWidgetsTestingMacros.cmake")
  kwwidgets_add_test_from_tcl_example(KWWidgets-${PROJECT_NAME}Tcl
    "${CMAKE_CURRENT_SOURCE_DIR}/${EXE_NAME}.tcl")
endif(BUILD_TESTING AND KWWidgets_BUILD_TESTING)
