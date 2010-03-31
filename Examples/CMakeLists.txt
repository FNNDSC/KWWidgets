cmake_minimum_required(VERSION 2.6)

project(KWWidgetsExamples)

# This is done for convenience, so that this very directory can
# be used as a top-level source directory for CMake. 
# Provided that KWWidgets_DIR is set, this will enable testing of all 
# examples separately from the library (say, from an installed KWWidgets tree)
# The relevant variables are:
#   KWWidgets_DIR
#   BUILD_SHARED_LIB
#   TCL_TCLSH (for Tcl tests)
#   TK_WISH (for Tcl tests)
#   PYTHON_EXECUTABLE (for Python tests)
#   KWWidgets_USE_SOVIEWER (for SOViewer tests)
#   SOV_DIR (for SOViewer tests)

if(NOT KWWidgets_SOURCE_DIR)
  find_package(KWWidgets REQUIRED)
  include(${KWWidgets_USE_FILE})
endif(NOT KWWidgets_SOURCE_DIR)

if(NOT DEFINED BUILD_TESTING)
  include(Dart)
  if(NOT DEFINED TCL_TCLSH)
    include(FindTclsh)
  endif(NOT DEFINED TCL_TCLSH)
  if(NOT DEFINED PYTHON_EXECUTABLE)
    include(FindPythonInterp)
  endif(NOT DEFINED PYTHON_EXECUTABLE)
  mark_as_advanced(BUILD_TESTING DART_ROOT TCL_TCLSH PYTHON_EXECUTABLE)
endif(NOT DEFINED BUILD_TESTING)

if(CMAKE_COMPILER_2005)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
endif(CMAKE_COMPILER_2005) 

if(BUILD_TESTING AND KWWidgets_BUILD_TESTING)
  enable_testing()
endif(BUILD_TESTING AND KWWidgets_BUILD_TESTING)

subdirs(Data Resources Cxx Tcl Python)
