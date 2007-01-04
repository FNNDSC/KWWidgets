# - Find [incr Tcl] includes and libraries.
# This module finds if [incr Tcl] is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#  INCR_TCL_LIBRARY        = path to [incr Tcl] library
#  INCR_TCL_INCLUDE_PATH   = path to where itcl.h can be found
#  INCR_TK_LIBRARY         = path to [incr Tk] library
#  INCR_TK_INCLUDE_PATH    = path to where itk.h can be found

INCLUDE(CMakeFindFrameworks)
INCLUDE(FindTCL)

GET_FILENAME_COMPONENT(TCL_TCLSH_PATH "${TCL_TCLSH}" PATH)
GET_FILENAME_COMPONENT(TCL_LIBRARY_PATH "${TCL_LIBRARY}" PATH)

GET_FILENAME_COMPONENT(TK_WISH_PATH "${TK_WISH}" PATH)
GET_FILENAME_COMPONENT(TK_LIBRARY_PATH "${TK_LIBRARY}" PATH)

SET(INCR_TCLTK_POSSIBLE_LIB_PATHS
  "${TCL_LIBRARY_PATH}"
  "${TK_LIBRARY_PATH}"
  "${TCL_TCLSH_PATH}/../lib"
  "${TK_WISH_PATH}/../lib"
  "${TCL_INCLUDE_PATH}/../lib"
  "${TK_INCLUDE_PATH}/../lib"
  "$ENV{ProgramFiles}/Tcl/Lib"
  "C:/Program Files/Tcl/lib" 
  "C:/Tcl/lib" 
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.4;Root]/lib
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.3;Root]/lib
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.2;Root]/lib
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.0;Root]/lib
  /usr/lib 
  /usr/local/lib
)

FOREACH(version "3.3" "3.2")
  SET(INCR_TCL_POSSIBLE_LIB_PATHS)
  SET(INCR_TK_POSSIBLE_LIB_PATHS)
  FOREACH(path ${INCR_TCLTK_POSSIBLE_LIB_PATHS})
    SET(INCR_TCL_POSSIBLE_LIB_PATHS ${INCR_TCL_POSSIBLE_LIB_PATHS} 
      "${path}/itcl${version}"
      )
    SET(INCR_TK_POSSIBLE_LIB_PATHS ${INCR_TK_POSSIBLE_LIB_PATHS} 
      "${path}/itk${version}"
      )
  ENDFOREACH(path)
  STRING(REGEX REPLACE "\\." "" version_collapsed ${version})
  FIND_LIBRARY(INCR_TCL_LIBRARY
    NAMES "itcl${version_collapsed}"
    PATHS ${INCR_TCL_POSSIBLE_LIB_PATHS}
    )
  FIND_LIBRARY(INCR_TK_LIBRARY
    NAMES "itk${version_collapsed}"
    PATHS ${INCR_TK_POSSIBLE_LIB_PATHS}
    )
ENDFOREACH(version)

MARK_AS_ADVANCED(
  INCR_TCL_LIBRARY
  INCR_TK_LIBRARY
)
  
CMAKE_FIND_FRAMEWORKS(Tcl)
CMAKE_FIND_FRAMEWORKS(Tk)

SET(TCL_FRAMEWORK_INCLUDES)
IF(Tcl_FRAMEWORKS)
  IF(NOT TCL_INCLUDE_PATH)
    FOREACH(dir ${Tcl_FRAMEWORKS})
      SET(TCL_FRAMEWORK_INCLUDES ${TCL_FRAMEWORK_INCLUDES} ${dir}/Headers)
    ENDFOREACH(dir)
  ENDIF(NOT TCL_INCLUDE_PATH)
ENDIF(Tcl_FRAMEWORKS)

SET(TK_FRAMEWORK_INCLUDES)
IF(Tk_FRAMEWORKS)
  IF(NOT TK_INCLUDE_PATH)
    FOREACH(dir ${Tk_FRAMEWORKS})
      SET(TK_FRAMEWORK_INCLUDES ${TK_FRAMEWORK_INCLUDES}
        ${dir}/Headers ${dir}/PrivateHeaders)
    ENDFOREACH(dir)
  ENDIF(NOT TK_INCLUDE_PATH)
ENDIF(Tk_FRAMEWORKS)

GET_FILENAME_COMPONENT(INCR_TCL_LIBRARY_PATH "${INCR_TCL_LIBRARY}" PATH)
GET_FILENAME_COMPONENT(INCR_TK_LIBRARY_PATH "${INCR_TK_LIBRARY}" PATH)

SET(INCR_TCLTK_POSSIBLE_INCLUDE_PATHS
  "${INCR_TCL_LIBRARY_PATH}/../include"
  "${INCR_TK_LIBRARY_PATH}/../include"
  "${TCL_INCLUDE_PATH}"
  "${TK_INCLUDE_PATH}"
  ${TCL_TCLSH_PATH}/../include
  ${TK_WISH_PATH}/../include
  "${TCL_LIBRARY_PATH}/../include"
  "${TK_LIBRARY_PATH}/../include"
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.4;Root]/include
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.3;Root]/include
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.2;Root]/include
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.0;Root]/include
  "$ENV{ProgramFiles}/Tcl/include"
  "C:/Program Files/Tcl/include"
  C:/Tcl/include
  /usr/include
  /usr/local/include
  /usr/include/tcl8.4
  /usr/include/tcl8.3
  /usr/include/tcl8.2
  /usr/include/tcl8.0
)

FIND_PATH(INCR_TCL_INCLUDE_PATH itcl.h
  ${TCL_FRAMEWORK_INCLUDES} ${INCR_TCLTK_POSSIBLE_INCLUDE_PATHS}
)

FIND_PATH(INCR_TK_INCLUDE_PATH itk.h
  ${TK_FRAMEWORK_INCLUDES} ${INCR_TCLTK_POSSIBLE_INCLUDE_PATHS}
)

MARK_AS_ADVANCED(
  INCR_TCL_INCLUDE_PATH
  INCR_TK_INCLUDE_PATH
  )
 
IF(INCR_TCL_INCLUDE_PATH AND INCR_TK_INCLUDE_PATH AND
   INCR_TCL_LIBRARY AND INCR_TK_LIBRARY)
 SET(INCR_TCL_FOUND 1)
ELSE(INCR_TCL_INCLUDE_PATH AND INCR_TK_INCLUDE_PATH AND
    INCR_TCL_LIBRARY AND INCR_TK_LIBRARY)
 SET(INCR_TCL_FOUND 0)
ENDIF(INCR_TCL_INCLUDE_PATH AND INCR_TK_INCLUDE_PATH AND
  INCR_TCL_LIBRARY AND INCR_TK_LIBRARY)

IF(NOT INCR_TCL_FOUND)
  SET(INCR_TCL_MESSAGE
    "[incr Tcl] was not found. Make sure INCR_TCL_LIBRARY, INCR_TK_LIBRARY, INCR_TCL_INCLUDE_PATH and INCR_TK_INCLUDE_PATH are set.")
  IF(IncrTCL_FIND_QUIETLY)
  ELSE(IncrTCL_FIND_QUIETLY)
    IF(IncrTCL_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "${INCR_TCL_MESSAGE}")
    ELSE(IncrTCL_FIND_REQUIRED)
      MESSAGE("${INCR_TCL_MESSAGE}")
    ENDIF(IncrTCL_FIND_REQUIRED)
  ENDIF(IncrTCL_FIND_QUIETLY)
ENDIF(NOT INCR_TCL_FOUND)
