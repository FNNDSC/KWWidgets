# - Find [incr Tcl] includes and libraries.
# This module finds if [incr Tcl] is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#  INCR_TCL_LIBRARY        = path to [incr Tcl] library
#  INCR_TCL_INCLUDE_PATH   = path to where itcl.h can be found
#  INCR_TK_LIBRARY         = path to [incr Tk] library
#  INCR_TK_INCLUDE_PATH    = path to where itk.h can be found

include(CMakeFindFrameworks)
include(FindTCL)

get_filename_component(TCL_TCLSH_PATH "${TCL_TCLSH}" PATH)
get_filename_component(TCL_TCLSH_PATH_PARENT "${TCL_TCLSH_PATH}" PATH)

get_filename_component(TK_WISH_PATH "${TK_WISH}" PATH)
get_filename_component(TK_WISH_PATH_PARENT "${TK_WISH_PATH}" PATH)

get_filename_component(TCL_INCLUDE_PATH_PARENT "${TCL_INCLUDE_PATH}" PATH)
get_filename_component(TK_INCLUDE_PATH_PARENT "${TK_INCLUDE_PATH}" PATH)

get_filename_component(TCL_LIBRARY_PATH "${TCL_LIBRARY}" PATH)
get_filename_component(TCL_LIBRARY_PATH_PARENT "${TCL_LIBRARY_PATH}" PATH)

get_filename_component(TK_LIBRARY_PATH "${TK_LIBRARY}" PATH)
get_filename_component(TK_LIBRARY_PATH_PARENT "${TK_LIBRARY_PATH}" PATH)

set(INCR_TCLTK_POSSIBLE_LIB_PATHS
  "${TCL_LIBRARY_PATH}"
  "${TK_LIBRARY_PATH}"
  "${TCL_INCLUDE_PATH_PARENT}/lib"
  "${TK_INCLUDE_PATH_PARENT}/lib"
  "${TCL_TCLSH_PATH_PARENT}/lib"
  "${TK_WISH_PATH_PARENT}/lib"
  /usr/lib 
  /usr/local/lib
  )

if(WIN32)
  get_filename_component(
    ActiveTcl_CurrentVersion 
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ActiveState\\ActiveTcl;CurrentVersion]" 
    NAME)
  set(INCR_TCLTK_POSSIBLE_LIB_PATHS ${INCR_TCLTK_POSSIBLE_LIB_PATHS}
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ActiveState\\ActiveTcl\\${ActiveTcl_CurrentVersion}]/lib"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.6;Root]/lib"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.5;Root]/lib"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.4;Root]/lib"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.3;Root]/lib"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.2;Root]/lib"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.0;Root]/lib"
    "$ENV{ProgramFiles}/Tcl/Lib"
    "C:/Program Files/Tcl/lib" 
    "C:/Tcl/lib" 
    )
endif(WIN32)

foreach(version "3.4" "3.3" "3.2")
  set(INCR_TCL_POSSIBLE_LIB_PATHS)
  set(INCR_TK_POSSIBLE_LIB_PATHS)
  foreach(path ${INCR_TCLTK_POSSIBLE_LIB_PATHS})
    set(INCR_TCL_POSSIBLE_LIB_PATHS ${INCR_TCL_POSSIBLE_LIB_PATHS} 
      "${path}"
      "${path}/itcl${version}"
      )
    set(INCR_TK_POSSIBLE_LIB_PATHS ${INCR_TK_POSSIBLE_LIB_PATHS} 
      "${path}"
      "${path}/itk${version}"
      )
  endforeach(path)
  string(REGEX REPLACE "\\." "" version_collapsed ${version})
  find_library(INCR_TCL_LIBRARY
    NAMES "itcl${version_collapsed}" "itcl${version}"
    PATHS ${INCR_TCL_POSSIBLE_LIB_PATHS}
    )
  find_library(INCR_TK_LIBRARY
    NAMES "itk${version_collapsed}" "itk${version}"
    PATHS ${INCR_TK_POSSIBLE_LIB_PATHS}
    )
endforeach(version)

mark_as_advanced(
  INCR_TCL_LIBRARY
  INCR_TK_LIBRARY
  )

cmake_find_frameworks(Tcl)
cmake_find_frameworks(Tk)

set(TCL_FRAMEWORK_INCLUDES)
if(Tcl_FRAMEWORKS)
  if(NOT TCL_INCLUDE_PATH)
    foreach(dir ${Tcl_FRAMEWORKS})
      set(TCL_FRAMEWORK_INCLUDES ${TCL_FRAMEWORK_INCLUDES} ${dir}/Headers)
    endforeach(dir)
  endif(NOT TCL_INCLUDE_PATH)
endif(Tcl_FRAMEWORKS)

set(TK_FRAMEWORK_INCLUDES)
if(Tk_FRAMEWORKS)
  if(NOT TK_INCLUDE_PATH)
    foreach(dir ${Tk_FRAMEWORKS})
      set(TK_FRAMEWORK_INCLUDES ${TK_FRAMEWORK_INCLUDES}
        ${dir}/Headers ${dir}/PrivateHeaders)
    endforeach(dir)
  endif(NOT TK_INCLUDE_PATH)
endif(Tk_FRAMEWORKS)

get_filename_component(INCR_TCL_LIBRARY_PATH "${INCR_TCL_LIBRARY}" PATH)
get_filename_component(INCR_TCL_LIBRARY_PATH_PARENT "${INCR_TCL_LIBRARY_PATH}" PATH)

get_filename_component(INCR_TK_LIBRARY_PATH "${INCR_TK_LIBRARY}" PATH)
get_filename_component(INCR_TK_LIBRARY_PATH_PARENT "${INCR_TK_LIBRARY_PATH}" PATH)

set(INCR_TCLTK_POSSIBLE_INCLUDE_PATHS
  "${INCR_TCL_LIBRARY_PATH_PARENT}/include"
  "${INCR_TK_LIBRARY_PATH_PARENT}/include"
  "${TCL_INCLUDE_PATH}"
  "${TK_INCLUDE_PATH}"
  "${TCL_LIBRARY_PATH_PARENT}/include"
  "${TK_LIBRARY_PATH_PARENT}/include"
  "${TCL_TCLSH_PATH_PARENT}/include"
  "${TK_WISH_PATH_PARENT}/include"
  /usr/include
  /usr/local/include
  /usr/include/tcl8.5
  /usr/include/tcl8.4
  /usr/include/tcl8.3
  /usr/include/tcl8.2
  /usr/include/tcl8.0
  )

if(WIN32)
  set(INCR_TCLTK_POSSIBLE_INCLUDE_PATHS ${INCR_TCLTK_POSSIBLE_INCLUDE_PATHS}
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ActiveState\\ActiveTcl\\${ActiveTcl_CurrentVersion}]/include"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.6;Root]/include"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.5;Root]/include"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.4;Root]/include"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.3;Root]/include"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.2;Root]/include"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.0;Root]/include"
    "$ENV{ProgramFiles}/Tcl/include"
    "C:/Program Files/Tcl/include"
    "C:/Tcl/include"
    )
endif(WIN32)

find_path(INCR_TCL_INCLUDE_PATH 
  NAMES itcl.h
  PATHS ${TCL_FRAMEWORK_INCLUDES} ${INCR_TCLTK_POSSIBLE_INCLUDE_PATHS}
  )

find_path(INCR_TK_INCLUDE_PATH 
  NAMES itk.h
  PATHS ${TK_FRAMEWORK_INCLUDES} ${INCR_TCLTK_POSSIBLE_INCLUDE_PATHS}
  )

mark_as_advanced(
  INCR_TCL_INCLUDE_PATH
  INCR_TK_INCLUDE_PATH
  )

if(INCR_TCL_INCLUDE_PATH AND INCR_TK_INCLUDE_PATH AND
    INCR_TCL_LIBRARY AND INCR_TK_LIBRARY)
  set(INCR_TCL_FOUND 1)
else(INCR_TCL_INCLUDE_PATH AND INCR_TK_INCLUDE_PATH AND
    INCR_TCL_LIBRARY AND INCR_TK_LIBRARY)
  set(INCR_TCL_FOUND 0)
endif(INCR_TCL_INCLUDE_PATH AND INCR_TK_INCLUDE_PATH AND
  INCR_TCL_LIBRARY AND INCR_TK_LIBRARY)

if(NOT INCR_TCL_FOUND)
  set(INCR_TCL_MESSAGE
    "[incr Tcl] was not found. Make sure INCR_TCL_LIBRARY, INCR_TK_LIBRARY, INCR_TCL_INCLUDE_PATH and INCR_TK_INCLUDE_PATH are set.")
  if(IncrTCL_FIND_QUIETLY)
  else(IncrTCL_FIND_QUIETLY)
    if(IncrTCL_FIND_REQUIRED)
      message(FATAL_ERROR "${INCR_TCL_MESSAGE}")
    else(IncrTCL_FIND_REQUIRED)
      message("${INCR_TCL_MESSAGE}")
    endif(IncrTCL_FIND_REQUIRED)
  endif(IncrTCL_FIND_QUIETLY)
endif(NOT INCR_TCL_FOUND)
