# ----------------------------------------------------------------------------
# INCR_TCL_GET_VERSION
# Try to find the major/minor version of the [incr Tcl] library.
#
# in: major_version: name of the var the major version is written to
#     minor_version: name of the var the minor version is written to
#
# ex: 
# INCR_TCL_GET_VERSION("INCR_TCL_MAJOR_VERSION" "INCR_TCL_MINOR_VERSION")
# SET(INCR_TCL_VERSION "${INCR_TCL_MAJOR_VERSION}.${INCR_TCL_MINOR_VERSION}")

macro(INCR_TCL_GET_VERSION major_version minor_version)

  # Try to find the current version by matching TCL_LIBRARY against some
  # version numbers

  set(${major_version} "")
  set(${minor_version} "")

  foreach(major_version_try "3")
    foreach(minor_version_try "2" "3")
      if("${INCR_TCL_LIBRARY}" 
          MATCHES "tcl${major_version_try}\\.?${minor_version_try}")
        set(${major_version} ${major_version_try})
        set(${minor_version} ${minor_version_try})
      endif("${INCR_TCL_LIBRARY}" 
        MATCHES "tcl${major_version_try}\\.?${minor_version_try}")
    endforeach(minor_version_try)
  endforeach(major_version_try)
  
  # Try to find the current version directly from the include file

  if(NOT ${major_version} AND NOT ${minor_version})
    foreach(dir ${INCR_TCL_INCLUDE_PATH})
      if(EXISTS "${dir}/itcl.h")
        file(READ "${dir}/itcl.h" itcl_include_file)
        string(REGEX REPLACE
          ".*#define ITCL_VERSION[ \t]*\"([0-9][0-9]*\\.[0-9][0-9]*)\".*" "\\1"
          itcl_include_file "${itcl_include_file}")
        if(${itcl_include_file} MATCHES "^[0-9]*\\.[0-9]*$")
          string(REGEX REPLACE "^([0-9]*)\\.([0-9]*)$" "\\1" ${major_version}
            "${itcl_include_file}")
          string(REGEX REPLACE "^([0-9]*)\\.([0-9]*)$" "\\2" ${minor_version}
            "${itcl_include_file}")
        endif(${itcl_include_file} MATCHES "^[0-9]*\\.[0-9]*$")
      endif(EXISTS "${dir}/itcl.h")
    endforeach(dir)
  endif(NOT ${major_version} AND NOT ${minor_version})

endmacro(INCR_TCL_GET_VERSION)

# ----------------------------------------------------------------------------
# INCR_TCL_GET_SUPPORT_DIR, INCR_TK_GET_SUPPORT_DIR
# Try to find the [incr Tcl] or [incr Tk] support lib dir.
#
# in: incr_tcl_support_lib_dir: name of the var the support dir is written to
#     incr_tk_support_lib_dir: name of the var the support dir is written to
#
# ex: 
# INCR_TCL_GET_SUPPORT_LIB_DIR("INCR_TCL_SUPPORT_LIB_DIR")

macro(INCR_TCL_GET_SUPPORT_LIB_DIR incr_tcl_support_lib_dir)

  set(${incr_tcl_support_lib_dir} "")
  if(INCR_TCL_LIBRARY)
    get_filename_component(incr_tcl_path "${INCR_TCL_LIBRARY}" PATH)
    # On Win32, the lib is into the support dir, on Unix it's one up.
    incr_tcl_get_version("INCR_TCL_MAJOR_VERSION" "INCR_TCL_MINOR_VERSION")
    set(INCR_TCL_VERSION "${INCR_TCL_MAJOR_VERSION}.${INCR_TCL_MINOR_VERSION}")
    if(EXISTS "${incr_tcl_path}/itcl.tcl")
      set(${incr_tcl_support_lib_dir} "${incr_tcl_path}")
    else(EXISTS "${incr_tcl_path}/itcl.tcl")
      set(incr_tcl_path "${incr_tcl_path}/itcl${INCR_TCL_VERSION}")
      if(EXISTS "${incr_tcl_path}/itcl.tcl")
        set(${incr_tcl_support_lib_dir} "${incr_tcl_path}")
      endif(EXISTS "${incr_tcl_path}/itcl.tcl")
    endif(EXISTS "${incr_tcl_path}/itcl.tcl")
  endif(INCR_TCL_LIBRARY)

endmacro(INCR_TCL_GET_SUPPORT_LIB_DIR)

macro(INCR_TK_GET_SUPPORT_LIB_DIR incr_tk_support_lib_dir)

  set(${incr_tk_support_lib_dir} "")
  if(INCR_TK_LIBRARY)
    get_filename_component(incr_tk_path "${INCR_TK_LIBRARY}" PATH)
    # On Win32, the lib is into the support dir, on Unix it's one up.
    incr_tcl_get_version("INCR_TCL_MAJOR_VERSION" "INCR_TCL_MINOR_VERSION")
    set(INCR_TCL_VERSION "${INCR_TCL_MAJOR_VERSION}.${INCR_TCL_MINOR_VERSION}")
    if(EXISTS "${incr_tk_path}/itk.tcl")
      set(${incr_tk_support_lib_dir} "${incr_tk_path}")
    else(EXISTS "${incr_tk_path}/itk.tcl")
      set(incr_tk_path "${incr_tk_path}/itk${INCR_TCL_VERSION}")
      if(EXISTS "${incr_tk_path}/itk.tcl")
        set(${incr_tk_support_lib_dir} "${incr_tk_path}")
      endif(EXISTS "${incr_tk_path}/itk.tcl")
    endif(EXISTS "${incr_tk_path}/itk.tcl")
  endif(INCR_TK_LIBRARY)

endmacro(INCR_TK_GET_SUPPORT_LIB_DIR)

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

macro(INCR_TCL_GET_SUPPORT_FILES incr_tcl_support_lib_dir list)

  # [incr Tcl] support files (*.tcl)

  file(GLOB INCR_TCL_SUPPORT_FILES_TCL "${incr_tcl_support_lib_dir}/*.tcl")
  set(${list} ${INCR_TCL_SUPPORT_FILES_TCL})

endmacro(INCR_TCL_GET_SUPPORT_FILES)

macro(INCR_TK_GET_SUPPORT_FILES incr_tk_support_lib_dir list)

  # [incr Tk] support files (*.tcl + *.itk + tclIndex, etc.)

  file(GLOB INCR_TK_SUPPORT_FILES_TCL "${incr_tk_support_lib_dir}/*.tcl")
  file(GLOB INCR_TK_SUPPORT_FILES_ITK "${incr_tk_support_lib_dir}/*.itk")
  set(${list}
    "${incr_tk_support_lib_dir}/tclIndex" 
    ${INCR_TK_SUPPORT_FILES_TCL}
    ${INCR_TK_SUPPORT_FILES_ITK})

endmacro(INCR_TK_GET_SUPPORT_FILES)

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

macro(INCR_TCLTK_COPY_SUPPORT_FILES 
    incr_tcl_support_lib_dir 
    incr_tcl_support_lib_dest 
    incr_tk_support_lib_dir 
    incr_tk_support_lib_dest)

  # Get the support files and copy them to dest dir
  # Check if EXISTS to work around CONFIGURE_FILE bug (if file does not
  # exist, it would create the subdirs anyway)

  incr_tcl_get_support_files(
    ${incr_tcl_support_lib_dir} "INCR_TCL_SUPPORT_FILES")
  string(REGEX REPLACE "^/" "" 
    incr_tcl_support_lib_dest_cm24 "${incr_tcl_support_lib_dest}")
  foreach(file ${INCR_TCL_SUPPORT_FILES})
    if(EXISTS ${file})
      string(REGEX REPLACE "${incr_tcl_support_lib_dir}/" "" filebase ${file})
      if("${ARGV4}" STREQUAL "INSTALL")
        get_filename_component(dir ${filebase} PATH)
        install(FILES "${file}"
          DESTINATION "${incr_tcl_support_lib_dest_cm24}/${dir}"
          COMPONENT RuntimeLibraries)
      else("${ARGV4}" STREQUAL "INSTALL")
        configure_file(
          ${file} "${incr_tcl_support_lib_dest}/${filebase}" COPYONLY)
      endif("${ARGV4}" STREQUAL "INSTALL")
    endif(EXISTS ${file})
  endforeach(file)

  incr_tk_get_support_files(${incr_tk_support_lib_dir} "INCR_TK_SUPPORT_FILES")
  string(REGEX REPLACE "^/" "" 
    incr_tk_support_lib_dest_cm24 "${incr_tk_support_lib_dest}")
  foreach(file ${INCR_TK_SUPPORT_FILES})
    if(EXISTS ${file})
      string(REGEX REPLACE "${incr_tk_support_lib_dir}/" "" filebase ${file})
      if("${ARGV4}" STREQUAL "INSTALL")
        get_filename_component(dir ${filebase} PATH)
        install(FILES "${file}"
          DESTINATION "${incr_tk_support_lib_dest_cm24}/${dir}"
          COMPONENT RuntimeLibraries)
      else("${ARGV4}" STREQUAL "INSTALL")
        configure_file(
          ${file} "${incr_tk_support_lib_dest}/${filebase}" COPYONLY)
      endif("${ARGV4}" STREQUAL "INSTALL")
    endif(EXISTS ${file})
  endforeach(file)

endmacro(INCR_TCLTK_COPY_SUPPORT_FILES)

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

macro(INCR_TCLTK_COPY_SUPPORT_FILES_TO_DIR 
    incr_tcl_support_lib_dir 
    incr_tk_support_lib_dir 
    target_dir)
  incr_tcl_get_version("INCR_TCL_MAJOR_VERSION" "INCR_TCL_MINOR_VERSION")
  if(INCR_TCL_MAJOR_VERSION AND INCR_TCL_MINOR_VERSION)
    set(INCR_TCL_VERSION "${INCR_TCL_MAJOR_VERSION}.${INCR_TCL_MINOR_VERSION}")
    incr_tcltk_copy_support_files(
      "${incr_tcl_support_lib_dir}"
      "${target_dir}/itcl${INCR_TCL_VERSION}"
      "${incr_tk_support_lib_dir}"
      "${target_dir}/itk${INCR_TCL_VERSION}"
      "${ARGV3}"
      )
  endif(INCR_TCL_MAJOR_VERSION AND INCR_TCL_MINOR_VERSION)

endmacro(INCR_TCLTK_COPY_SUPPORT_FILES_TO_DIR)

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

macro(INCR_TCLTK_COPY_SUPPORT_FILES_TO_BUILD_DIR 
    incr_tcl_support_lib_dir 
    incr_tk_support_lib_dir 
    build_dir 
    dir)

  # For each configuration type (Debug, RelInfo, Release, etc.)
  # Copy the TclTk support files to the corresponding sub-directory inside
  # the build dir

  if(CMAKE_CONFIGURATION_TYPES)
    set(CONFIG_TYPES ${CMAKE_CONFIGURATION_TYPES})
  else(CMAKE_CONFIGURATION_TYPES)
    set(CONFIG_TYPES .)
  endif(CMAKE_CONFIGURATION_TYPES)
  foreach(config ${CONFIG_TYPES})
    incr_tcltk_copy_support_files_to_dir(
      "${incr_tcl_support_lib_dir}"
      "${incr_tk_support_lib_dir}"
      "${build_dir}/${config}/${dir}"
      )
  endforeach(config)

endmacro(INCR_TCLTK_COPY_SUPPORT_FILES_TO_BUILD_DIR)

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

macro(KWWidgets_COPY_TCL_TK_SUPPORT_FILES dir)

  # Copy support files only if VTK_TCL_TK_COPY_SUPPORT_LIBRARY is ON
  # which is set in VTK by default to 1 if VTK_TCL_TK_STATIC is ON.

  if(VTK_TCL_TK_COPY_SUPPORT_LIBRARY)

    # Tcl/Tk support files

    if(VTK_TCL_SUPPORT_LIBRARY_PATH AND VTK_TK_SUPPORT_LIBRARY_PATH)
      include(${VTK_TCL_TK_MACROS_MODULE})
      vtk_copy_tcl_tk_support_files_to_dir(
        ${VTK_TCL_SUPPORT_LIBRARY_PATH} 
        ${VTK_TK_SUPPORT_LIBRARY_PATH}
        "${dir}")
    endif(VTK_TCL_SUPPORT_LIBRARY_PATH AND VTK_TK_SUPPORT_LIBRARY_PATH)

    # [incr Tcl/Tk] support files

    if(KWWidgets_USE_INCR_TCL)
      incr_tcl_get_support_lib_dir("INCR_TCL_SUPPORT_LIB_DIR")
      incr_tk_get_support_lib_dir("INCR_TK_SUPPORT_LIB_DIR")
      if(INCR_TCL_SUPPORT_LIB_DIR AND INCR_TK_SUPPORT_LIB_DIR)
        incr_tcltk_copy_support_files_to_dir(
          ${INCR_TCL_SUPPORT_LIB_DIR} 
          ${INCR_TK_SUPPORT_LIB_DIR}
          "${dir}")
      endif(INCR_TCL_SUPPORT_LIB_DIR AND INCR_TK_SUPPORT_LIB_DIR)
    endif(KWWidgets_USE_INCR_TCL)

  endif(VTK_TCL_TK_COPY_SUPPORT_LIBRARY)

endmacro(KWWidgets_COPY_TCL_TK_SUPPORT_FILES)

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

macro(KWWidgets_INSTALL_TCL_TK_SUPPORT_FILES dir)

  # Install support files only if VTK_TCL_TK_COPY_SUPPORT_LIBRARY is ON
  # which is set in VTK by default to 1 if VTK_TCL_TK_STATIC is ON.

  if(VTK_TCL_TK_COPY_SUPPORT_LIBRARY)

    # Tcl/Tk support files

    if(VTK_TCL_SUPPORT_LIBRARY_PATH AND VTK_TK_SUPPORT_LIBRARY_PATH)
      include(${VTK_TCL_TK_MACROS_MODULE})
      vtk_copy_tcl_tk_support_files_to_dir(
        ${VTK_TCL_SUPPORT_LIBRARY_PATH} 
        ${VTK_TK_SUPPORT_LIBRARY_PATH}
        "${dir}" 
        INSTALL)
    endif(VTK_TCL_SUPPORT_LIBRARY_PATH AND VTK_TK_SUPPORT_LIBRARY_PATH)

    # [incr Tcl/Tk] support files

    if(KWWidgets_USE_INCR_TCL)
      incr_tcl_get_support_lib_dir("INCR_TCL_SUPPORT_LIB_DIR")
      incr_tk_get_support_lib_dir("INCR_TK_SUPPORT_LIB_DIR")
      if(INCR_TCL_SUPPORT_LIB_DIR AND INCR_TK_SUPPORT_LIB_DIR)
        incr_tcltk_copy_support_files_to_dir(
          ${INCR_TCL_SUPPORT_LIB_DIR} 
          ${INCR_TK_SUPPORT_LIB_DIR}
          "${dir}" 
          INSTALL)
      endif(INCR_TCL_SUPPORT_LIB_DIR AND INCR_TK_SUPPORT_LIB_DIR)
    endif(KWWidgets_USE_INCR_TCL)

  endif(VTK_TCL_TK_COPY_SUPPORT_LIBRARY)

endmacro(KWWidgets_INSTALL_TCL_TK_SUPPORT_FILES)
