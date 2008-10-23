#
# This module is provided as KWWidgets_USE_FILE by KWWidgetsConfig.cmake.  
# It can be INCLUDEd in a project to load the needed compiler and linker
# settings to use KWWidgets:
#   FIND_PACKAGE(KWWidgets REQUIRED)
#   INCLUDE(${KWWidgets_USE_FILE})
#

if(NOT KWWidgets_USE_FILE_INCLUDED)
  set(KWWidgets_USE_FILE_INCLUDED 1)

  # Load the compiler settings used for KWWidgets.
  if(KWWidgets_BUILD_SETTINGS_FILE AND NOT SKIP_KWWidgets_BUILD_SETTINGS_FILE)
    include(${CMAKE_ROOT}/Modules/CMakeImportBuildSettings.cmake)
    cmake_import_build_settings(${KWWidgets_BUILD_SETTINGS_FILE})
  endif(KWWidgets_BUILD_SETTINGS_FILE AND NOT SKIP_KWWidgets_BUILD_SETTINGS_FILE)

  # Add compiler flags needed to use KWWidgets.
  set(CMAKE_C_FLAGS 
    "${CMAKE_C_FLAGS} ${KWWidgets_REQUIRED_C_FLAGS}")
  set(CMAKE_CXX_FLAGS 
    "${CMAKE_CXX_FLAGS} ${KWWidgets_REQUIRED_CXX_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS 
    "${CMAKE_EXE_LINKER_FLAGS} ${KWWidgets_REQUIRED_EXE_LINKER_FLAGS}")
  set(CMAKE_SHARED_LINKER_FLAGS 
    "${CMAKE_SHARED_LINKER_FLAGS} ${KWWidgets_REQUIRED_SHARED_LINKER_FLAGS}")
  set(CMAKE_MODULE_LINKER_FLAGS 
    "${CMAKE_MODULE_LINKER_FLAGS} ${KWWidgets_REQUIRED_MODULE_LINKER_FLAGS}")

  # Add include directories needed to use KWWidgets.
  include_directories(${KWWidgets_INCLUDE_DIRS})

  # Add link directories needed to use KWWidgets.
  link_directories(${KWWidgets_LIBRARY_DIRS})

  # Add cmake module path.
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${KWWidgets_CMAKE_DIR}")

  # Use VTK.
  if(NOT KWWidgets_NO_USE_VTK)
    set(VTK_DIR ${KWWidgets_VTK_DIR})
    find_package(VTK)
    if(VTK_FOUND)
      include(${VTK_USE_FILE})
    else(VTK_FOUND)
      message("VTK not found in KWWidgets_VTK_DIR=\"${KWWidgets_VTK_DIR}\".")
    endif(VTK_FOUND)
  endif(NOT KWWidgets_NO_USE_VTK)

  # Use IncrTcl.
  if(KWWidgets_USE_INCR_TCL)
    if(NOT INCR_TCL_LIBRARY AND NOT INCR_TK_LIBRARY)
      include("${KWWidgets_CMAKE_DIR}/FindIncrTCL.cmake")
    endif(NOT INCR_TCL_LIBRARY AND NOT INCR_TK_LIBRARY)
  endif(KWWidgets_USE_INCR_TCL)

endif(NOT KWWidgets_USE_FILE_INCLUDED)
