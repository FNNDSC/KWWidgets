# ----------------------------------------------------------------------------
# INCR_TCL_GET_VERSION
# Return the major/minor version of the [incr Tcl] library.
#
# in: major_version: name of the var the major version is written to
#     minor_version: name of the var the minor version is written to
#
# ex: 
# INCR_TCL_GET_VERSION("INCR_TCL_MAJOR_VERSION" "INCR_TCL_MINOR_VERSION")
# SET(INCR_TCL_VERSION "${INCR_TCL_MAJOR_VERSION}.${INCR_TCL_MINOR_VERSION}")

MACRO(INCR_TCL_GET_VERSION major_version minor_version)

  # Try to find the current version by matching TCL_LIBRARY against some
  # version numbers

  SET(${major_version} "")
  SET(${minor_version} "")

  FOREACH(major_version_try "3")
    FOREACH(minor_version_try "2" "3")
      IF("${INCR_TCL_LIBRARY}" 
          MATCHES "tcl${major_version_try}\\.?${minor_version_try}")
        SET(${major_version} ${major_version_try})
        SET(${minor_version} ${minor_version_try})
      ENDIF("${INCR_TCL_LIBRARY}" 
        MATCHES "tcl${major_version_try}\\.?${minor_version_try}")
    ENDFOREACH(minor_version_try)
  ENDFOREACH(major_version_try)
  
  # Try to find the current version directly from the include file

  IF(NOT ${major_version} AND NOT ${minor_version})
    FOREACH(dir ${INCR_TCL_INCLUDE_PATH})
      IF(EXISTS "${dir}/itcl.h")
        FILE(READ "${dir}/itcl.h" itcl_include_file)
        STRING(REGEX REPLACE
          ".*#define ITCL_VERSION[ \t]*\"([0-9][0-9]*\\.[0-9][0-9]*)\".*" "\\1"
          itcl_include_file "${itcl_include_file}")
        IF(${itcl_include_file} MATCHES "^[0-9]*\\.[0-9]*$")
          STRING(REGEX REPLACE "^([0-9]*)\\.([0-9]*)$" "\\1" ${major_version}
            "${itcl_include_file}")
          STRING(REGEX REPLACE "^([0-9]*)\\.([0-9]*)$" "\\2" ${minor_version}
            "${itcl_include_file}")
        ENDIF(${itcl_include_file} MATCHES "^[0-9]*\\.[0-9]*$")
      ENDIF(EXISTS "${dir}/itcl.h")
    ENDFOREACH(dir)
  ENDIF(NOT ${major_version} AND NOT ${minor_version})

ENDMACRO(INCR_TCL_GET_VERSION)

# ----------------------------------------------------------------------------
# INCR_TCL_GET_SUPPORT_FILES, INCR_TK_GET_SUPPORT_FILES
# Get the list of [incr Tcl] and [incr Tk] support files.
# Support files are additional files that are mandatory for [incr Tcl/Tk]
# to work properly. Linking against the [incr Tcl/Tk] libraries is 
# not enough, [incr Tcl/Tk] needs to access those support files at run-time.
# A typical Tcl/Tk installation will store support files in sub-directories 
# inside the lib directory, organized by version number. 
# Example:
#    c:/tcl/lib/itcl3.2
#    c:/tcl/lib/itk3.2
# Given the path to the [incr Tcl] support lib dir, INCR_TCL_GET_SUPPORT_FILES 
# will return the corresponding list of support files.
# Given the path to the [incr Tk] support lib dir, INCR_TK_GET_SUPPORT_FILES
#  will return the corresponding list of support files.
#
# in: support_lib_dir: path to the [incr Tcl] (or Tk) support lib dir
#     list:            name of the var the list is written to

MACRO(INCR_TCL_GET_SUPPORT_FILES incr_tcl_support_lib_dir list)

  # [incr Tcl] support files (*.tcl)

  FILE(GLOB INCR_TCL_SUPPORT_FILES_TCL "${incr_tcl_support_lib_dir}/*.tcl")
  SET(${list} ${INCR_TCL_SUPPORT_FILES_TCL})

ENDMACRO(INCR_TCL_GET_SUPPORT_FILES)

MACRO(INCR_TK_GET_SUPPORT_FILES incr_tk_support_lib_dir list)

  # [incr Tk] support files (*.tcl + *.itk + tclIndex, etc.)

  FILE(GLOB INCR_TK_SUPPORT_FILES_TCL "${incr_tk_support_lib_dir}/*.tcl")
  FILE(GLOB INCR_TK_SUPPORT_FILES_ITK "${incr_tk_support_lib_dir}/*.itk")
  SET(${list}
    "${incr_tk_support_lib_dir}/tclIndex" 
    ${INCR_TK_SUPPORT_FILES_TCL}
    ${INCR_TK_SUPPORT_FILES_ITK})

ENDMACRO(INCR_TK_GET_SUPPORT_FILES)

# ----------------------------------------------------------------------------
# INCR_TCLTK_COPY_SUPPORT_FILES
# Copy (or install) [incr Tcl/Tk] support files to a specific location.
# See INCR_TCL_GET_SUPPORT_FILES for more infomation about support files.
# Given the paths to the [incr Tcl] and [incr Tk] support lib dirs, this macro
# will copy (or install) the appropriate support files to the destination 
# dirs, recreating the subdirs if needed.
# This macro takes an optional last parameter, if set to INSTALL the 
# files will be scheduled for installation (using CMake's INSTALL)
# instead of copied.
#
# in: incr_tcl_support_lib_dir:  path to the Tcl support lib dir
#     incr_tcl_support_lib_dest: destination dir for the Tcl support lib files
#     incr_tk_support_lib_dir:   path to the Tk support lib dir
#     incr_tk_support_lib_dest:  destination dir for the Tk support lib files
#     INSTALL:              optional parameter (install files instead of copy)
#
# ex: INCR_TCLTK_COPY_SUPPORT_FILES (
#       "c:/tcl/lib/itcl3.2" "d:/vtk-bin/TclTk/lib/itcl3.2"
#       "c:/tcl/lib/itk3.2" "d:/vtk-bin/TclTk/lib/tk3.2")
#     this will copy [incr Tcl] support files from:
#       c:/tcl/lib/itcl3.2
#     to:
#       d:/vtk-bin/TclTk/lib/itcl3.2
#     and copy [incr Tk] support files from:
#       c:/tcl/lib/itk3.2
#     to:
#       d:/vtk-bin/TclTk/lib/itk3.2

MACRO(INCR_TCLTK_COPY_SUPPORT_FILES 
    incr_tcl_support_lib_dir 
    incr_tcl_support_lib_dest 
    incr_tk_support_lib_dir 
    incr_tk_support_lib_dest)

  # Get the support files and copy them to dest dir
  # Check if EXISTS to work around CONFIGURE_FILE bug (if file does not
  # exist, it would create the subdirs anyway)

  INCR_TCL_GET_SUPPORT_FILES(
    ${incr_tcl_support_lib_dir} "INCR_TCL_SUPPORT_FILES")
  STRING(REGEX REPLACE "^/" "" 
    incr_tcl_support_lib_dest_cm24 "${incr_tcl_support_lib_dest}")
  FOREACH(file ${INCR_TCL_SUPPORT_FILES})
    IF(EXISTS ${file})
      STRING(REGEX REPLACE "${incr_tcl_support_lib_dir}/" "" filebase ${file})
      IF("${ARGV4}" STREQUAL "INSTALL")
        GET_FILENAME_COMPONENT(dir ${filebase} PATH)
        INSTALL(FILES "${file}"
          DESTINATION "${incr_tcl_support_lib_dest_cm24}/${dir}"
          COMPONENT RuntimeLibraries)
      ELSE("${ARGV4}" STREQUAL "INSTALL")
        CONFIGURE_FILE(
          ${file} "${incr_tcl_support_lib_dest}/${filebase}" COPYONLY)
      ENDIF("${ARGV4}" STREQUAL "INSTALL")
    ENDIF(EXISTS ${file})
  ENDFOREACH(file)

  INCR_TK_GET_SUPPORT_FILES(${incr_tk_support_lib_dir} "INCR_TK_SUPPORT_FILES")
  STRING(REGEX REPLACE "^/" "" 
    incr_tk_support_lib_dest_cm24 "${incr_tk_support_lib_dest}")
  FOREACH(file ${INCR_TK_SUPPORT_FILES})
    IF(EXISTS ${file})
      STRING(REGEX REPLACE "${incr_tk_support_lib_dir}/" "" filebase ${file})
      IF("${ARGV4}" STREQUAL "INSTALL")
        GET_FILENAME_COMPONENT(dir ${filebase} PATH)
        INSTALL(FILES "${file}"
          DESTINATION "${incr_tk_support_lib_dest_cm24}/${dir}"
          COMPONENT RuntimeLibraries)
      ELSE("${ARGV4}" STREQUAL "INSTALL")
        CONFIGURE_FILE(
          ${file} "${incr_tk_support_lib_dest}/${filebase}" COPYONLY)
      ENDIF("${ARGV4}" STREQUAL "INSTALL")
    ENDIF(EXISTS ${file})
  ENDFOREACH(file)

ENDMACRO(INCR_TCLTK_COPY_SUPPORT_FILES)

# ----------------------------------------------------------------------------
# INCR_TCLTK_COPY_SUPPORT_FILES_TO_DIR
# Front-end to INCR_TCLTK_COPY_SUPPORT_FILES, this macro will 
# copy (or install) the appropriate [incr Tcl/Tk] support files to a directory.
# The [incr Tcl/Tk[] version is retrieved automatically and used to create
# the subdirectories (see example below)
# This macro takes an optional last parameter, if set to INSTALL the 
# files will be scheduled for installation (using CMake's INSTALL)
# instead of copied.
#
# in: incr_tcl_support_lib_dir: path to the [incr Tcl] support lib dir
#     incr_tk_support_lib_dir:  path to the [incr Tk] support lib dir
#     target_dir:          target directory
#     INSTALL:             optional parameter (install files instead of copy)
#
# ex: INCR_TCLTK_COPY_SUPPORT_FILES_TO_DIR (
#        "c:/tcl/lib/itcl3.2" "c:/tcl/lib/itk3.2" "d:/vtk-bin/lib")
#     if this project is configured to use [incr Tcl/Tk 3.2], this will copy
#     support files from:
#       c:/tcl/lib/itcl3.2
#       c:/tcl/lib/itk3.2
#     to:
#       d:/vtk-bin/lib/itcl3.2
#       d:/vtk-bin/lib/itk3.2

MACRO(INCR_TCLTK_COPY_SUPPORT_FILES_TO_DIR 
    incr_tcl_support_lib_dir 
    incr_tk_support_lib_dir 
    target_dir)
  INCR_TCL_GET_VERSION("INCR_TCL_MAJOR_VERSION" "INCR_TCL_MINOR_VERSION")
  IF(INCR_TCL_MAJOR_VERSION AND INCR_TCL_MINOR_VERSION)
    SET(INCR_TCL_VERSION "${INCR_TCL_MAJOR_VERSION}.${INCR_TCL_MINOR_VERSION}")
    INCR_TCLTK_COPY_SUPPORT_FILES(
      "${incr_tcl_support_lib_dir}"
      "${target_dir}/itcl${INCR_TCL_VERSION}"
      "${incr_tk_support_lib_dir}"
      "${target_dir}/itk${INCR_TCL_VERSION}"
      "${ARGV3}"
      )
  ENDIF(INCR_TCL_MAJOR_VERSION AND INCR_TCL_MINOR_VERSION)

ENDMACRO(INCR_TCLTK_COPY_SUPPORT_FILES_TO_DIR)

# ----------------------------------------------------------------------------
# INCR_TCLTK_COPY_SUPPORT_FILES_TO_BUILD_DIR
# Front-end to INCR_TCLTK_COPY_SUPPORT_FILES_TO_DIR, this macro will copy the
# appropriate [incr Tcl/Tk] support files to a project build directory.
# The support files will be copied simultaneously to all configuration 
# sub-directories (Release, RelInfo, Debug, etc.) if needed.
# The [incr Tcl/Tk] version is retrieved automatically and used to create
# the subdirectories (see example below)
#
# in: incr_tcl_support_lib_dir: path to the [incr Tcl] support lib dir
#     incr_tk_support_lib_dir:  path to the [incr Tk] support lib dir
#     build_dir:           project build dir
#     dir:                 relative subdir inside the build dir, into which the
#                          support files will be copied.
#
# ex: INCR_TCLTK_COPY_SUPPORT_FILES_TO_BUILD_DIR (
#        "c:/tcl/lib/itcl3.2" "c:/tcl/lib/itk3.2" "d:/vtk-bin" "TclTk/lib")
#     if this project is configured to use [incr Tcl/Tk 3.2, this will copy
#     support files from:
#       c:/tcl/lib/itcl3.2
#       c:/tcl/lib/itk3.2
#     to (respectively):
#       d:/vtk-bin/TclTk/lib/itcl3.2
#       d:/vtk-bin/TclTk/lib/itk3.2
#     or (if configuration types are supported by the generator):
#       d:/vtk-bin/Release/TclTk/lib/itcl3.2
#       d:/vtk-bin/Release/TclTk/lib/itk3.2
#       d:/vtk-bin/Debug/TclTk/lib/itcl3.2
#       d:/vtk-bin/Debug/TclTk/lib/itk3.2
#       etc.

MACRO(INCR_TCLTK_COPY_SUPPORT_FILES_TO_BUILD_DIR 
    incr_tcl_support_lib_dir 
    incr_tk_support_lib_dir 
    build_dir 
    dir)

  # For each configuration type (Debug, RelInfo, Release, etc.)
  # Copy the TclTk support files to the corresponding sub-directory inside
  # the build dir

  IF(CMAKE_CONFIGURATION_TYPES)
    SET(CONFIG_TYPES ${CMAKE_CONFIGURATION_TYPES})
  ELSE(CMAKE_CONFIGURATION_TYPES)
    SET(CONFIG_TYPES .)
  ENDIF(CMAKE_CONFIGURATION_TYPES)
  FOREACH(config ${CONFIG_TYPES})
    INCR_TCLTK_COPY_SUPPORT_FILES_TO_DIR(
      "${incr_tcl_support_lib_dir}"
      "${incr_tk_support_lib_dir}"
      "${build_dir}/${config}/${dir}"
      )
  ENDFOREACH(config)

ENDMACRO(INCR_TCLTK_COPY_SUPPORT_FILES_TO_BUILD_DIR)

# ---------------------------------------------------------------------------
# KWWidgets_COPY_TCL_TK_SUPPORT_FILES
# Copy Tcl/Tk and [incr Tcl/Tk] support files to a directory.
# For example:
#   KWWidgets_COPY_TCL_TK_SUPPORT_FILES("d:/myapp-bin/lib")
# will copy the support files to:
#   d:/myapp-bin/lib/tcl8.4
#   d:/myapp-bin/lib/tk8.4
#   d:/myapp-bin/lib/itcl3.2
#   d:/myapp-bin/lib/itk3.2

MACRO(KWWidgets_COPY_TCL_TK_SUPPORT_FILES dir)

  # Copy support files only if VTK_TCL_TK_COPY_SUPPORT_LIBRARY is ON
  # which is set in VTK by default to 1 if VTK_TCL_TK_STATIC is ON.

  IF(VTK_TCL_TK_COPY_SUPPORT_LIBRARY)

    # Tcl/Tk support files

    IF(VTK_TCL_SUPPORT_LIBRARY_PATH AND VTK_TK_SUPPORT_LIBRARY_PATH)
      INCLUDE(${VTK_TCL_TK_MACROS_MODULE})
      VTK_COPY_TCL_TK_SUPPORT_FILES_TO_DIR(
        ${VTK_TCL_SUPPORT_LIBRARY_PATH} 
        ${VTK_TK_SUPPORT_LIBRARY_PATH}
        "${dir}")
    ENDIF(VTK_TCL_SUPPORT_LIBRARY_PATH AND VTK_TK_SUPPORT_LIBRARY_PATH)

    # [incr Tcl/Tk] support files

    IF(KWWidgets_USE_INCR_TCL)
      IF(INCR_TCL_LIBRARY AND INCR_TK_LIBRARY)
        GET_FILENAME_COMPONENT(
          INCR_TCL_LIBRARY_PATH "${INCR_TCL_LIBRARY}" PATH)
        GET_FILENAME_COMPONENT(
          INCR_TK_LIBRARY_PATH "${INCR_TK_LIBRARY}" PATH)
        INCR_TCLTK_COPY_SUPPORT_FILES_TO_DIR(
          ${INCR_TCL_LIBRARY_PATH} 
          ${INCR_TK_LIBRARY_PATH}
          "${dir}")
      ENDIF(INCR_TCL_LIBRARY AND INCR_TK_LIBRARY)
    ENDIF(KWWidgets_USE_INCR_TCL)

  ENDIF(VTK_TCL_TK_COPY_SUPPORT_LIBRARY)

ENDMACRO(KWWidgets_COPY_TCL_TK_SUPPORT_FILES)

# ---------------------------------------------------------------------------
# KWWidgets_COPY_TCL_TK_SUPPORT_FILES
# Install Tcl/Tk and [incr Tcl/Tk] support files to a directory.
# For example:
#   KWWidgets_COPY_TCL_TK_SUPPORT_FILES("/lib")
# will install the support files to:
#   ${install_prefix}/lib/tcl8.4
#   ${install_prefix}/lib/tk8.4
#   ${install_prefix}/lib/itcl3.2
#   ${install_prefix}/lib/itk3.2

MACRO(KWWidgets_INSTALL_TCL_TK_SUPPORT_FILES dir)

  # Install support files only if VTK_TCL_TK_COPY_SUPPORT_LIBRARY is ON
  # which is set in VTK by default to 1 if VTK_TCL_TK_STATIC is ON.

  IF(VTK_TCL_TK_COPY_SUPPORT_LIBRARY)

    # Tcl/Tk support files

    IF(VTK_TCL_SUPPORT_LIBRARY_PATH AND VTK_TK_SUPPORT_LIBRARY_PATH)
      INCLUDE(${VTK_TCL_TK_MACROS_MODULE})
      VTK_COPY_TCL_TK_SUPPORT_FILES_TO_DIR(
        ${VTK_TCL_SUPPORT_LIBRARY_PATH} 
        ${VTK_TK_SUPPORT_LIBRARY_PATH}
        "${dir}" 
        INSTALL)
    ENDIF(VTK_TCL_SUPPORT_LIBRARY_PATH AND VTK_TK_SUPPORT_LIBRARY_PATH)

    # [incr Tcl/Tk] support files

    IF(KWWidgets_USE_INCR_TCL)
      IF(INCR_TCL_LIBRARY AND INCR_TK_LIBRARY)
        GET_FILENAME_COMPONENT(
          INCR_TCL_LIBRARY_PATH "${INCR_TCL_LIBRARY}" PATH)
        # On Win32, the lib is right into the support lib dir, on Unix it's
        # one up.
        IF(NOT EXISTS "${INCR_TCL_LIBRARY_PATH}/itcl.tcl")
          GET_FILENAME_COMPONENT(
            INCR_TCL_LIBRARY_PATH "${INCR_TCL_LIBRARY_PATH}" PATH)
        ENDIF(NOT EXISTS "${INCR_TCL_LIBRARY_PATH}/itcl.tcl")
        GET_FILENAME_COMPONENT(
          INCR_TK_LIBRARY_PATH "${INCR_TK_LIBRARY}" PATH)
        IF(NOT EXISTS "${INCR_TK_LIBRARY_PATH}/itk.tcl")
          GET_FILENAME_COMPONENT(
            INCR_TK_LIBRARY_PATH "${INCR_TK_LIBRARY_PATH}" PATH)
        ENDIF(NOT EXISTS "${INCR_TK_LIBRARY_PATH}/itk.tcl")
        INCR_TCLTK_COPY_SUPPORT_FILES_TO_DIR(
          ${INCR_TCL_LIBRARY_PATH} 
          ${INCR_TK_LIBRARY_PATH}
          "${dir}" 
          INSTALL)
        ENDIF(INCR_TCL_LIBRARY AND INCR_TK_LIBRARY)
    ENDIF(KWWidgets_USE_INCR_TCL)

  ENDIF(VTK_TCL_TK_COPY_SUPPORT_LIBRARY)

ENDMACRO(KWWidgets_INSTALL_TCL_TK_SUPPORT_FILES)
