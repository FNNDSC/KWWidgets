# ---------------------------------------------------------------------------
# KWWidgets_GENERATE_SETUP_PATHS_SCRIPTS
# Generate a set of small setup scripts (.bat, .sh, .csh) that can be sourced
# to setup the environment variables required to run an executable built
# against the current project and some known third-party dependencies (VTK, 
# ITK, SOViewer, KWWidgets, etc.)
# 'output_path': location (dir) where to store the generated scripts
# This macro can take optional parameters:
# 'basename': basename for generated scripts (extension of originals are kept)
# 'extra_library_dirs': list of extra libraries dirs

macro(KWWidgets_GENERATE_SETUP_PATHS_SCRIPTS 
    output_path)

  if(NOT "${ARGV1}" STREQUAL "")
    set(basename "${ARGV1}")
  else(NOT "${ARGV1}" STREQUAL "")
    set(basename "KWWidgetsSetupPaths")
  endif(NOT "${ARGV1}" STREQUAL "")

  if(NOT "${ARGV2}" STREQUAL "")
    set(extra_library_dirs "${ARGV2}")
  else(NOT "${ARGV2}" STREQUAL "")
    set(extra_library_dirs "")
  endif(NOT "${ARGV2}" STREQUAL "")

  kwwidgets_generate_setup_paths(
    "${output_path}"
    "${basename}"
    1
    0 "" ""
    "${extra_library_dirs}"
    )
  
endmacro(KWWidgets_GENERATE_SETUP_PATHS_SCRIPTS)

# ---------------------------------------------------------------------------
# KWWidgets_GENERATE_SETUP_PATHS_LAUNCHER
# Generate a lightweight C launcher for a *specific* executable.
# The launcher sets up all the environments variables (PATH, TCLLIBPATH,
# LD_LIBRARY_PATH, etc.) required by this executable and some known 
# third-party dependencies (VTK, ITK, SOV, KWWidgets, etc.), before launching
# the executable itself. Note that the path to the executable to launch is 
# hard-coded in the launcher: do not move the target exe around, or copy the
# launcher to your installation tree, if any (ultimately, all the DLLs needed
# by a project should/could be stored together in the installation tree, and a
# launcher should not be needed in that case).
# This macro also create the corresponding executable target.
# 'output_path': location (dir) where to store the generated launcher C source
# 'basename': basename for both the generated C source and launcher exe
# 'exe_dir': location of the executable to generate a C launcher for
# 'exe_name': name of the executable to generate a C launcher for
# Note that if 'exe_name' is empty, the launcher will run the executable passed
# as first parameter. 
# Note that if If 'exe_dir' is empty, the launcher will change its working
# dir to the launcher directory, and therefore attempt to locate 'exe_name'
# in that same directory.
# This macro can take optional parameters:
# 'extra_library_dirs': list of extra libraries dirs

macro(KWWidgets_GENERATE_SETUP_PATHS_LAUNCHER
    output_path
    basename
    exe_dir exe_name)

  if(NOT "${ARGV4}" STREQUAL "")
    set(extra_library_dirs "${ARGV4}")
  else(NOT "${ARGV4}" STREQUAL "")
    set(extra_library_dirs "")
  endif(NOT "${ARGV4}" STREQUAL "")

  kwwidgets_generate_setup_paths(
    "${output_path}"
    "${basename}"
    0
    1 "${exe_dir}" "${exe_name}"
    "${extra_library_dirs}"
    )
  
endmacro(KWWidgets_GENERATE_SETUP_PATHS_LAUNCHER)

# ---------------------------------------------------------------------------
# KWWidgets_GENERATE_SETUP_PATHS
# Front-end to the macro that generate the scripts or launcher for all known
# configuration types. This macro collects and fixes some of the paths that
# still need to be fixed or hacked in third-party packages, then call
# the macro that loop over all the configuration types.
# 'output_path': location (dir) where to store the generated scripts/launcher
# 'basename': basename for generated scripts (extension of originals are kept)
# 'generate_scripts': if true generate setup path scripts (.bat, .sh, .csh)
# 'generate_launcher': if true generate C launcher
# 'exe_dir': location of the executable to generate a C launcher for
# 'exe_name': name of the executable to generate a C launcher for
# 'extra_library_dirs': list of extra libraries dirs

macro(KWWidgets_GENERATE_SETUP_PATHS 
    output_path
    basename
    generate_scripts
    generate_launcher exe_dir exe_name
    extra_library_dirs)

  # VTK

  set(VTK_TCL_PATHS "${VTK_TCL_HOME}")

  if(VTK_INSTALL_PREFIX)
    if(WIN32)
      set(VTK_PYTHON_PATHS "${VTK_TCL_HOME}/../site-packages")
    else(WIN32)
      if(PYTHON_EXECUTABLE)
        exec_program("${PYTHON_EXECUTABLE}" ARGS "-V" OUTPUT_VARIABLE version)
        string(REGEX REPLACE "^(Python )([0-9]\\.[0-9])(.*)$" "\\2" 
          major_minor "${version}")
        set(VTK_PYTHON_PATHS 
          "${VTK_TCL_HOME}/../python${major_minor}/site-packages")
      else(PYTHON_EXECUTABLE)
        set(VTK_PYTHON_PATHS "${VTK_TCL_HOME}/../python2.4/site-packages")
      endif(PYTHON_EXECUTABLE)
    endif(WIN32)
  else(VTK_INSTALL_PREFIX)
    set(VTK_PYTHON_PATHS "${VTK_TCL_HOME}/../Python")
  endif(VTK_INSTALL_PREFIX)

  # ITK
  # Try to find out if ITK is installed
  get_filename_component(name "${ITK_LIBRARY_DIRS}" NAME)
  if("${name}" STREQUAL "InsightToolkit")
    if(WIN32)
      set(ITK_RUNTIME_DIRS "${ITK_LIBRARY_DIRS}/../../bin")
    else(WIN32)
      set(ITK_RUNTIME_DIRS "${ITK_LIBRARY_DIRS}")
    endif(WIN32)
    set(ITK_CONFIGURATION_TYPES)
  else("${name}" STREQUAL "InsightToolkit")
    set(ITK_RUNTIME_DIRS ${ITK_LIBRARY_DIRS})
    set(ITK_CONFIGURATION_TYPES ${CMAKE_CONFIGURATION_TYPES})
  endif("${name}" STREQUAL "InsightToolkit")

  # KWWidgets

  set(KWWidgets_TCL_PATHS "${KWWidgets_TCL_PACKAGE_INDEX_DIR}")

  set(KWWidgets_PYTHON_PATHS ${KWWidgets_PYTHON_PATHS} 
    "${KWWidgets_PYTHON_MODULE_DIR}")

  kwwidgets_generate_setup_paths_for_all_configuration_types(
    "${output_path}"
    "${basename}"
    "${generate_scripts}"
    "${generate_launcher}" "${exe_dir}" "${exe_name}"
    "${VTK_LIBRARY_DIRS}"
    "${VTK_RUNTIME_DIRS}"
    "${VTK_TCL_PATHS}"
    "${VTK_PYTHON_PATHS}"
    "${ITK_LIBRARY_DIRS}"
    "${ITK_RUNTIME_DIRS}"
    "${SOV_LIBRARY_DIRS}"
    "${SOV_RUNTIME_DIRS}"
    "${KWWidgets_LIBRARY_DIRS}"
    "${KWWidgets_RUNTIME_DIRS}"
    "${KWWidgets_TCL_PATHS}"
    "${KWWidgets_PYTHON_PATHS}"
    "${extra_library_dirs}"
    )
  
endmacro(KWWidgets_GENERATE_SETUP_PATHS)

# ---------------------------------------------------------------------------
# KWWidgets_GENERATE_SETUP_PATHS_FOR_ALL_CONFIGURATION_TYPES
# Generate the scripts and/or launcher for all known configuration types. 
# This macro loops over all configuration types for each project (say Debug,
# Release, RelDebWithInfo), postfix all the path parameters that need to be
# postfixed then generate the scripts/launcher for each config type in the
#  appropriate config directory.
# 'output_path': location (dir) where to store the generated scripts/launcher
# 'basename': basename for generated scripts (extension of originals are kept)
# 'generate_scripts': if true generate setup path scripts (.bat, .sh, .csh)
# 'generate_launcher': if true generate C launcher
# 'exe_dir': location of the executable to generate a C launcher for
# 'exe_name': name of the executable to generate a C launcher for

macro(KWWidgets_GENERATE_SETUP_PATHS_FOR_ALL_CONFIGURATION_TYPES 
    output_path
    basename
    generate_scripts
    generate_launcher exe_dir exe_name
    vtk_lib_paths 
    vtk_runtime_paths 
    vtk_tcl_paths 
    vtk_python_paths
    itk_lib_paths 
    itk_runtime_paths 
    sov_lib_paths 
    sov_runtime_paths 
    kwwidgets_lib_paths 
    kwwidgets_runtime_paths 
    kwwidgets_tcl_paths 
    kwwidgets_python_paths
    extra_library_dirs)

  if(WIN32 AND CMAKE_CONFIGURATION_TYPES)

    # Scripts are generated first, then executable (below)

    foreach(config ${CMAKE_CONFIGURATION_TYPES})

      kwwidgets_generate_setup_paths_for_one_configuration_type(
        "${output_path}/${config}"
        "${basename}"
        "${config}"
        "${generate_scripts}"
        0 "" ""
        "${vtk_lib_paths}" 
        "${vtk_runtime_paths}" 
        "${vtk_tcl_paths}" 
        "${vtk_python_paths}"
        "${itk_lib_paths}" 
        "${itk_runtime_paths}" 
        "${sov_lib_paths}" 
        "${sov_runtime_paths}" 
        "${kwwidgets_lib_paths}"
        "${kwwidgets_runtime_paths}"
        "${kwwidgets_tcl_paths}"
        "${kwwidgets_python_paths}"
        "${extra_library_dirs}"
        )
      
    endforeach(config)

    # Executable to launch. We can not create one C file per config type
    # since we can't have the same executable target points to different
    # source files (one per each config type). Use the CMAKE_INTDIR
    # substitution instead: at compile time, CMAKE_INTDIR is replaced by
    # the right config type.

    kwwidgets_generate_setup_paths_for_one_configuration_type(
      "${output_path}"
      "${basename}"
      "\"CMAKE_INTDIR\""
      0
      "${generate_launcher}" "${exe_dir}" "${exe_name}"
      "${vtk_lib_paths}" 
      "${vtk_runtime_paths}" 
      "${vtk_tcl_paths}" 
      "${vtk_python_paths}"
      "${itk_lib_paths}" 
      "${itk_runtime_paths}" 
      "${sov_lib_paths}" 
      "${sov_runtime_paths}" 
      "${kwwidgets_lib_paths}"
      "${kwwidgets_runtime_paths}"
      "${kwwidgets_tcl_paths}"
      "${kwwidgets_python_paths}"
      "${extra_library_dirs}"
      )

  else(WIN32 AND CMAKE_CONFIGURATION_TYPES)

    kwwidgets_generate_setup_paths_for_one_configuration_type(
      "${output_path}"
      "${basename}"
      ""
      "${generate_scripts}"
      "${generate_launcher}" "${exe_dir}" "${exe_name}"
      "${vtk_lib_paths}" 
      "${vtk_runtime_paths}" 
      "${vtk_tcl_paths}" 
      "${vtk_python_paths}"
      "${itk_lib_paths}" 
      "${itk_runtime_paths}" 
      "${sov_lib_paths}" 
      "${sov_runtime_paths}" 
      "${kwwidgets_lib_paths}"
      "${kwwidgets_runtime_paths}"
      "${kwwidgets_tcl_paths}"
      "${kwwidgets_python_paths}"
      "${extra_library_dirs}"
      )

  endif(WIN32 AND CMAKE_CONFIGURATION_TYPES)
endmacro(KWWidgets_GENERATE_SETUP_PATHS_FOR_ALL_CONFIGURATION_TYPES)

# ---------------------------------------------------------------------------
# KWWidgets_GENERATE_SETUP_PATHS_FOR_ONE_CONFIGURATION_TYPE
# Generate the scripts and/or launcher for a specific config type. At this 
# point it is assumed all paths are correct and do not need any configuration
# type postfix (like Release/Debug, etc.)
# 'output_path': location (dir) where to store the generated scripts/launcher
# 'basename': basename for generated scripts (extension of originals are kept)
# 'config': the specific config type (Debug, Release, etc.)
# 'generate_scripts': if true generate setup path scripts (.bat, .sh, .csh)
# 'generate_launcher': if true generate C launcher
# 'exe_dir': location of the executable to generate a C launcher for
# 'exe_name': name of the executable to generate a C launcher for

macro(KWWidgets_GENERATE_SETUP_PATHS_FOR_ONE_CONFIGURATION_TYPE
    output_path
    basename
    config
    generate_scripts
    generate_launcher _exe_dir exe_name
    _vtk_lib_paths 
    _vtk_runtime_paths 
    _vtk_tcl_paths 
    vtk_python_paths
    _itk_lib_paths 
    _itk_runtime_paths 
    _sov_lib_paths 
    _sov_runtime_paths 
    _kwwidgets_lib_paths 
    _kwwidgets_runtime_paths 
    _kwwidgets_tcl_paths 
    kwwidgets_python_paths
    _extra_library_dirs)

  set(vtk_lib_paths ${_vtk_lib_paths})
  set(vtk_runtime_paths ${_vtk_runtime_paths})
  set(vtk_tcl_paths ${_vtk_tcl_paths})
  
  set(itk_lib_paths ${_itk_lib_paths})
  set(itk_runtime_paths ${_itk_runtime_paths})
  
  set(sov_lib_paths ${_sov_lib_paths})
  set(sov_runtime_paths ${_sov_runtime_paths})
  
  set(kwwidgets_lib_paths ${_kwwidgets_lib_paths})
  set(kwwidgets_runtime_paths ${_kwwidgets_runtime_paths})
  set(kwwidgets_tcl_paths ${_kwwidgets_tcl_paths})
  
  set(exe_dir ${_exe_dir})

  set(extra_library_dirs ${_extra_library_dirs})
  
  # Update some paths with the configuration type if needed

  if(NOT "${config}" STREQUAL "" AND WIN32 AND CMAKE_CONFIGURATION_TYPES)

    # VTK

    if(VTK_CONFIGURATION_TYPES)
      set(vtk_lib_paths2)
      set(vtk_runtime_paths2)
      set(vtk_tcl_paths2)
      foreach(dir ${_vtk_lib_paths})
        set(vtk_lib_paths2 ${vtk_lib_paths2} "${dir}/${config}")
      endforeach(dir)
      foreach(dir ${_vtk_runtime_paths})
        set(vtk_runtime_paths2 ${vtk_runtime_paths2} "${dir}/${config}")
      endforeach(dir)
      foreach(dir ${_vtk_tcl_paths})
        set(vtk_tcl_paths2 ${vtk_tcl_paths2} "${dir}/${config}")
      endforeach(dir)
      set(vtk_lib_paths ${vtk_lib_paths2})
      set(vtk_runtime_paths ${vtk_runtime_paths2})
      set(vtk_tcl_paths ${vtk_tcl_paths2})
    endif(VTK_CONFIGURATION_TYPES)

    # ITK

    if(ITK_CONFIGURATION_TYPES)
      set(itk_lib_paths2)
      set(itk_runtime_paths2)
      foreach(dir ${_itk_lib_paths})
        set(itk_lib_paths2 ${itk_lib_paths2} "${dir}/${config}")
      endforeach(dir)
      foreach(dir ${_itk_runtime_paths})
        set(itk_runtime_paths2 ${itk_runtime_paths2} "${dir}/${config}")
      endforeach(dir)
      set(itk_lib_paths ${itk_lib_paths2})
      set(itk_runtime_paths ${itk_runtime_paths2})
    endif(ITK_CONFIGURATION_TYPES)

    # SOV

    if(SOV_CONFIGURATION_TYPES)
      set(sov_lib_paths2)
      set(sov_runtime_paths2)
      foreach(dir ${_sov_lib_paths})
        set(sov_lib_paths2 ${sov_lib_paths2} "${dir}/${config}")
      endforeach(dir)
      foreach(dir ${_sov_runtime_paths})
        set(sov_runtime_paths2 ${sov_runtime_paths2} "${dir}/${config}")
      endforeach(dir)
      set(sov_lib_paths ${sov_lib_paths2})
      set(sov_runtime_paths ${sov_runtime_paths2})
    endif(SOV_CONFIGURATION_TYPES)

    # KWWidgets

    if(KWWidgets_CONFIGURATION_TYPES)
      set(kwwidgets_lib_paths2)
      set(kwwidgets_runtime_paths2)
      set(kwwidgets_tcl_paths2)
      foreach(dir ${_kwwidgets_lib_paths})
        set(kwwidgets_lib_paths2 
          ${kwwidgets_lib_paths2} "${dir}/${config}")
      endforeach(dir)
      foreach(dir ${_kwwidgets_runtime_paths})
        set(kwwidgets_runtime_paths2 
          ${kwwidgets_runtime_paths2} "${dir}/${config}")
      endforeach(dir)
      foreach(dir ${_kwwidgets_tcl_paths})
        set(kwwidgets_tcl_paths2 
          ${kwwidgets_tcl_paths2} "${dir}/${config}")
      endforeach(dir)
      set(kwwidgets_lib_paths ${kwwidgets_lib_paths2})
      set(kwwidgets_runtime_paths ${kwwidgets_runtime_paths2})
      set(kwwidgets_tcl_paths ${kwwidgets_tcl_paths2})
    endif(KWWidgets_CONFIGURATION_TYPES)

    # Extra

    set(extra_library_dirs2)
    foreach(dir ${_extra_library_dirs})
      set(extra_library_dirs2 
        ${extra_library_dirs2} "${dir}/${config}")
    endforeach(dir)
    set(extra_library_dirs ${extra_library_dirs2})

    set(exe_dir ${_exe_dir})
    if(NOT "${exe_dir}" STREQUAL "")
      set(exe_dir "${exe_dir}/${config}")
    endif(NOT "${exe_dir}" STREQUAL "")

  endif(NOT "${config}" STREQUAL "" AND WIN32 AND CMAKE_CONFIGURATION_TYPES)

  # Executable to launch

  if(${generate_launcher})
    set(EXECUTABLE)
    if(NOT "${exe_dir}" STREQUAL "")
      set(EXECUTABLE "${exe_dir}/")
    endif(NOT "${exe_dir}" STREQUAL "")
    set(EXECUTABLE "${EXECUTABLE}${exe_name}")
  endif(${generate_launcher})

  # For PATH

  set(KWWidgets_PATH_ENV 
    ${vtk_runtime_paths}
    ${vtk_lib_paths}
    ${itk_runtime_paths}
    ${itk_lib_paths}
    ${sov_runtime_paths}
    ${sov_lib_paths}
    ${kwwidgets_runtime_paths}
    ${kwwidgets_lib_paths}
    ${extra_library_dirs}
    ${EXECUTABLE_OUTPUT_PATH}
    ${LIBRARY_OUTPUT_PATH}
    )

  # Gettext lib

  if(KWWidgets_USE_INTERNATIONALIZATION)
    set(gettext_path)
    if(GETTEXT_INTL_LIBRARY)
      get_filename_component(path "${GETTEXT_INTL_LIBRARY}" PATH)
      set(gettext_path "${path}/../bin")
    else(GETTEXT_INTL_LIBRARY)
      if(GETTEXT_INCLUDE_DIR)
        set(gettext_path "${GETTEXT_INCLUDE_DIR}/../bin")
      else(GETTEXT_INCLUDE_DIR)
        if(GETTEXT_XGETTEXT_EXECUTABLE)
          get_filename_component(path "${GETTEXT_XGETTEXT_EXECUTABLE}" PATH)
          set(gettext_path "${path}/../bin")
        endif(GETTEXT_XGETTEXT_EXECUTABLE)
      endif(GETTEXT_INCLUDE_DIR)
    endif(GETTEXT_INTL_LIBRARY)
    # Still nothing found, let's try GETTEXT_SEARCH_PATH, which is defined
    # when building against a KWWidgets build
    if(NOT gettext_path AND GETTEXT_SEARCH_PATH)
      foreach(path "${GETTEXT_SEARCH_PATH}")
        set(gettext_path ${gettext_path} "${path}/../bin")
      endforeach(path)
    endif(NOT gettext_path AND GETTEXT_SEARCH_PATH)
    # Still nothing found, then we are probably building against am
    # installed KWWidgets, and need to find gettext ourself.
    if(NOT gettext_path)
      find_package(Gettext REQUIRED)
    endif(NOT gettext_path)
    if(gettext_path)
      set(KWWidgets_PATH_ENV ${KWWidgets_PATH_ENV} "${gettext_path}")
    endif(gettext_path)
  endif(KWWidgets_USE_INTERNATIONALIZATION)

  # [incr Tcl]

  if(KWWidgets_USE_INCR_TCL)
    if(NOT INCR_TCL_LIBRARY AND NOT INCR_TK_LIBRARY)
      include("${KWWidgets_CMAKE_DIR}/FindIncrTCL.cmake")
    endif(NOT INCR_TCL_LIBRARY AND NOT INCR_TK_LIBRARY)
    if(INCR_TCL_LIBRARY)
      get_filename_component(path "${INCR_TCL_LIBRARY}" PATH)
      if(path)
        set(KWWidgets_PATH_ENV ${KWWidgets_PATH_ENV} "${path}")
      endif(path)
    endif(INCR_TCL_LIBRARY)
    if(INCR_TK_LIBRARY)
      get_filename_component(path "${INCR_TK_LIBRARY}" PATH)
      if(path)
        set(KWWidgets_PATH_ENV ${KWWidgets_PATH_ENV} "${path}")
      endif(path)
    endif(INCR_TK_LIBRARY)
  endif(KWWidgets_USE_INCR_TCL)

  # If we have no TCL_LIBRARY or TCL_TCLSH, then we are probably being invoked
  # from an out-of-source example that is using either an installed VTK or
  # an installed KWWidgets. None of those projects export TCL_* variables
  # when they are installed. Let's try to find Tclsh at least.

  if (NOT TCL_LIBRARY AND NOT TCL_TCLSH)
    include(${CMAKE_ROOT}/Modules/FindTclsh.cmake)
  endif (NOT TCL_LIBRARY AND NOT TCL_TCLSH)
  
  if(TCL_LIBRARY)
    get_filename_component(path "${TCL_LIBRARY}" PATH)
    set(KWWidgets_PATH_ENV ${KWWidgets_PATH_ENV} ${path} "${path}/../bin")
  endif(TCL_LIBRARY)

  if(TK_LIBRARY)
    get_filename_component(path "${TK_LIBRARY}" PATH)
    set(KWWidgets_PATH_ENV ${KWWidgets_PATH_ENV} ${path} "${path}/../bin")
  endif(TK_LIBRARY)

  if(TCL_TCLSH)
    get_filename_component(path "${TCL_TCLSH}" PATH)
    set(KWWidgets_PATH_ENV ${KWWidgets_PATH_ENV} ${path})
  endif(TCL_TCLSH)

  if(PYTHON_EXECUTABLE)
    get_filename_component(path "${PYTHON_EXECUTABLE}" PATH)
    set(KWWidgets_PATH_ENV ${KWWidgets_PATH_ENV} ${path})
  endif(PYTHON_EXECUTABLE)

  if(CMAKE_MAKE_PROGRAM)
    get_filename_component(path "${CMAKE_MAKE_PROGRAM}" PATH)
    set(KWWidgets_PATH_ENV ${KWWidgets_PATH_ENV} ${path})
  endif(CMAKE_MAKE_PROGRAM)

  # Extra runtime lib
  
  set(extra_runtime_paths)
  if(vtkVolumeRendering_LIB_DEPENDS) # trying to catch Mesa lib path
    foreach(lib ${vtkVolumeRendering_LIB_DEPENDS})
      get_filename_component(path "${lib}" PATH)
      if(path)
        set(extra_runtime_paths ${extra_runtime_paths} ${path})
      endif(path)
    endforeach(lib)
  endif(vtkVolumeRendering_LIB_DEPENDS)

  if(KWWidgets_USE_INTERNATIONALIZATION)
    if(GETTEXT_INTL_LIBRARY)
      get_filename_component(path "${GETTEXT_INTL_LIBRARY}" PATH)
      if(path)
        set(extra_runtime_paths ${extra_runtime_paths} ${path})
      endif(path)
    endif(GETTEXT_INTL_LIBRARY)
  endif(KWWidgets_USE_INTERNATIONALIZATION)

  if(TCL_LIBRARY)
    get_filename_component(path "${TCL_LIBRARY}" PATH)
    set(extra_runtime_paths ${extra_runtime_paths} ${path})
  endif(TCL_LIBRARY)

  if(TK_LIBRARY)
    get_filename_component(path "${TK_LIBRARY}" PATH)
    set(extra_runtime_paths ${extra_runtime_paths} ${path})
  endif(TK_LIBRARY)

  if(KWWidgets_USE_INCR_TCL)
    if(INCR_TCL_LIBRARY)
      get_filename_component(path "${INCR_TCL_LIBRARY}" PATH)
      if(path)
        set(extra_runtime_paths ${extra_runtime_paths} ${path})
      endif(path)
    endif(INCR_TCL_LIBRARY)
    if(INCR_TK_LIBRARY)
      get_filename_component(path "${INCR_TK_LIBRARY}" PATH)
      if(path)
        set(extra_runtime_paths ${extra_runtime_paths} ${path})
      endif(path)
    endif(INCR_TK_LIBRARY)
  endif(KWWidgets_USE_INCR_TCL)

  # For LD_LIBRARY_PATH or equivalent
  
  if (WIN32)
    # there is no equivalent on Windows, use LD_LIBRARY_PATH as fake
    set(SHARED_LIBRARY_PATH_VAR_NAME "LD_LIBRARY_PATH")
    set(KWWidgets_SHARED_LIBRARY_PATH_ENV "")
  else (WIN32)
    # Try to inherit the path variable name from VTK
    if (VTK_RUNTIME_PATH_VAR_NAME)
      set (SHARED_LIBRARY_PATH_VAR_NAME ${VTK_RUNTIME_PATH_VAR_NAME})
    else (VTK_RUNTIME_PATH_VAR_NAME)
      set (SHARED_LIBRARY_PATH_VAR_NAME "LD_LIBRARY_PATH")
      if (APPLE)
        set (SHARED_LIBRARY_PATH_VAR_NAME "DYLD_LIBRARY_PATH")
      endif (APPLE)
    endif (VTK_RUNTIME_PATH_VAR_NAME)
    set(KWWidgets_SHARED_LIBRARY_PATH_ENV
      ${vtk_runtime_paths}
      ${itk_runtime_paths}
      ${sov_runtime_paths}
      ${kwwidgets_runtime_paths}
      ${extra_runtime_paths})
    if(CMAKE_SKIP_RPATH)
      set(KWWidgets_SHARED_LIBRARY_PATH_ENV 
        ${KWWidgets_SHARED_LIBRARY_PATH_ENV}
        ${EXECUTABLE_OUTPUT_PATH} 
        ${LIBRARY_OUTPUT_PATH})
    endif(CMAKE_SKIP_RPATH)
  endif (WIN32)

  # For ITCL_LIBRARY and ITK_LIBRARY

  set(KWWidgets_ITCL_LIBRARY_ENV)
  set(KWWidgets_ITK_LIBRARY_ENV)
  set(KWWidgets_USE_INCR_TCL_ENV "0")
  if(KWWidgets_USE_INCR_TCL)
    include("${KWWidgets_CMAKE_DIR}/KWWidgetsTclTkMacros.cmake")
    incr_tcl_get_support_lib_dir("KWWidgets_ITCL_LIBRARY_ENV")
    incr_tk_get_support_lib_dir("KWWidgets_ITK_LIBRARY_ENV")
    set(KWWidgets_USE_INCR_TCL_ENV "1")
  endif(KWWidgets_USE_INCR_TCL)

  # For TCLLIBPATH (space separated)

  set(KWWidgets_TCLLIBPATH_ENV)
  foreach(dir ${vtk_tcl_paths})
    set(KWWidgets_TCLLIBPATH_ENV 
      "${KWWidgets_TCLLIBPATH_ENV} {${dir}}")
  endforeach(dir)
  foreach(dir ${kwwidgets_tcl_paths})
    set(KWWidgets_TCLLIBPATH_ENV 
      "${KWWidgets_TCLLIBPATH_ENV} {${dir}}")
  endforeach(dir)
  
  # For PYTHONPATH
  
  set(KWWidgets_PYTHONPATH_ENV 
    ${vtk_python_paths}
    ${vtk_runtime_paths}
    ${kwwidgets_python_paths}
    ${kwwidgets_runtime_paths}
    )

  if(WIN32)

    # For Win32 PATH (semi-colon separated, no cygdrive)

    string(REGEX REPLACE "/cygdrive/(.)/" "\\1:/" 
      KWWidgets_PATH_ENV "${KWWidgets_PATH_ENV}")

    # For Win32 TCLLIBPATH (space separated, no cygdrive)

    string(REGEX REPLACE "/cygdrive/(.)/" "\\1:/" 
      KWWidgets_TCLLIBPATH_ENV "${KWWidgets_TCLLIBPATH_ENV}")

    # For Win32 PYTHONPATH (semi-colon separated, no cygdrive)

    set(KWWidgets_PYTHONPATH_SEP ";")
    string(REGEX REPLACE "/cygdrive/(.)/" "\\1:/" 
      KWWidgets_PYTHONPATH_ENV "${KWWidgets_PYTHONPATH_ENV}")

    # Configure the Win32 batch file

    set(KWWidgets_PATH_SEP ";")

    if(${generate_scripts})
      configure_file(
        ${KWWidgets_TEMPLATES_DIR}/KWWidgetsSetupPaths.bat.in
        ${output_path}/${basename}.bat
        IMMEDIATE)
    endif(${generate_scripts})

    if(${generate_scripts})
      configure_file(
        ${KWWidgets_TEMPLATES_DIR}/KWWidgetsSetupPaths.cmake.in
        ${output_path}/${basename}.cmake
        IMMEDIATE @ONLY)
    endif(${generate_scripts})

    if(${generate_launcher})
      configure_file(
        ${KWWidgets_TEMPLATES_DIR}/KWWidgetsSetupPathsLauncher.c.in
        ${output_path}/${basename}.c
        IMMEDIATE @ONLY)
    endif(${generate_launcher})

    # For Cygwin PATH (colon separated, use cygdrive)
    # Yes, we could use the cygpath tool in the file directly

    string(REGEX REPLACE "(.):/" "/cygdrive/\\1/" 
      KWWidgets_PATH_ENV "${KWWidgets_PATH_ENV}")

    set(KWWidgets_PATH_SEP ":")

    string(REGEX REPLACE ";" ${KWWidgets_PATH_SEP}
      KWWidgets_PATH_ENV "${KWWidgets_PATH_ENV}")

    # For Win32 TCLLIBPATH (space separated, no cygdrive)
    # Yes, we could use the cygpath tool in the file directly

    # For Cygwin PYTHONPATH (escaped semi-colon separated, no cygdrive)

    set(KWWidgets_PYTHONPATH_SEP "\;")
    string(REGEX REPLACE ";" "\\\;" 
      KWWidgets_PYTHONPATH_ENV "${KWWidgets_PYTHONPATH_ENV}")

    # Configure the Cygwin bash/tcsh file

    if(${generate_scripts})
      configure_file(
        ${KWWidgets_TEMPLATES_DIR}/KWWidgetsSetupPaths.sh.in
        ${output_path}/${basename}.sh
        IMMEDIATE)
      
      configure_file(
        ${KWWidgets_TEMPLATES_DIR}/KWWidgetsSetupPaths.csh.in
        ${output_path}/${basename}.csh
        IMMEDIATE)
    endif(${generate_scripts})

  else(WIN32)

    # For Unix PATH (colon separated)

    set(KWWidgets_PATH_SEP ":")
    string(REGEX REPLACE ";" ${KWWidgets_PATH_SEP}
      KWWidgets_PATH_ENV "${KWWidgets_PATH_ENV}")

    # For Unix LD_LIBRARY_PATH (colon separated)

    set(KWWidgets_SHARED_LIBRARY_PATH_SEP ":")
    string(REGEX REPLACE ";" ${KWWidgets_SHARED_LIBRARY_PATH_SEP}
      KWWidgets_SHARED_LIBRARY_PATH_ENV "${KWWidgets_SHARED_LIBRARY_PATH_ENV}")

    # For Unix TCLLIBPATH (space separated)

    # For Unix PYTHONPATH (colon separated)

    set(KWWidgets_PYTHONPATH_SEP ":")
    string(REGEX REPLACE ";" ${KWWidgets_PYTHONPATH_SEP}
      KWWidgets_PYTHONPATH_ENV "${KWWidgets_PYTHONPATH_ENV}")

    # Configure the Unix bash/tcsh file

    if(${generate_scripts})
      configure_file(
        ${KWWidgets_TEMPLATES_DIR}/KWWidgetsSetupPaths.sh.in
        ${output_path}/${basename}.sh
        IMMEDIATE)
      
      configure_file(
        ${KWWidgets_TEMPLATES_DIR}/KWWidgetsSetupPaths.csh.in
        ${output_path}/${basename}.csh
        IMMEDIATE)
    endif(${generate_scripts})

    configure_file(
      ${KWWidgets_TEMPLATES_DIR}/KWWidgetsSetupPaths.cmake.in
      ${output_path}/${basename}.cmake
      IMMEDIATE @ONLY)

    if(${generate_launcher})
      configure_file(
        ${KWWidgets_TEMPLATES_DIR}/KWWidgetsSetupPathsLauncher.c.in
        ${output_path}/${basename}.c
        IMMEDIATE @ONLY)
    endif(${generate_launcher})

  endif(WIN32)

  if(${generate_launcher})
    add_executable(${basename} ${output_path}/${basename}.c)

    if(MSVC AND NOT MSVC60 AND NOT MSVC70 AND NOT MSVC71)
      if("${basename}" MATCHES ".*Setup.*")
        set(exe "${CMAKE_CFG_INTDIR}/${basename}.exe")
        if(EXECUTABLE_OUTPUT_PATH)
          set(exe "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/${basename}.exe")
        endif(EXECUTABLE_OUTPUT_PATH)

        # Solve the "things named like *Setup prompt for admin privileges
        # on Vista" problem by merging a manifest fragment that contains a
        # requestedExecutionLevel element:
        #
        add_custom_command(TARGET ${basename}
          POST_BUILD COMMAND mt
          "-inputresource:${exe};#1"
          -manifest "${KWWidgets_RESOURCES_DIR}/KWWidgetsSetupManifest.xml"
          "-outputresource:${exe};#1"
          )
      endif("${basename}" MATCHES ".*Setup.*")
    endif(MSVC AND NOT MSVC60 AND NOT MSVC70 AND NOT MSVC71)
  endif(${generate_launcher})

endmacro(KWWidgets_GENERATE_SETUP_PATHS_FOR_ONE_CONFIGURATION_TYPE)
