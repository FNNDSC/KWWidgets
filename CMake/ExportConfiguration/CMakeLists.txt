# Generate the KWWidgetsConfig.cmake file in the build tree. Also configure
# one for installation. The file tells external projects how to use
# KWWidgets.

# Help store a literal dollar in a string.  CMake 2.2 allows escaped
# dollars but we have to support CMake 2.0.
set(DOLLAR "$")

#-----------------------------------------------------------------------------
# Settings shared between the build tree and install tree.

export_library_dependencies(
  ${KWWidgets_BINARY_DIR}/KWWidgetsLibraryDepends.cmake)
include(${CMAKE_ROOT}/Modules/CMakeExportBuildSettings.cmake)
cmake_export_build_settings(
  ${KWWidgets_BINARY_DIR}/KWWidgetsBuildSettings.cmake)

if(NOT KWWidgets_INSTALL_NO_DEVELOPMENT)
  install_files(${KWWidgets_INSTALL_PACKAGE_DIR} FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/UseKWWidgets.cmake
    ${KWWidgets_BINARY_DIR}/KWWidgetsLibraryDepends.cmake
    ${KWWidgets_BINARY_DIR}/KWWidgetsBuildSettings.cmake
    )
endif(NOT KWWidgets_INSTALL_NO_DEVELOPMENT)

#-----------------------------------------------------------------------------
# Settings specific to the build tree.

# The install-only section is empty for the build tree.
set(KWWidgets_CONFIG_INSTALL_ONLY)

# The "use" file.
set(KWWidgets_USE_FILE_CONFIG 
  ${CMAKE_CURRENT_SOURCE_DIR}/UseKWWidgets.cmake)

# The build settings file.
set(KWWidgets_BUILD_SETTINGS_FILE_CONFIG 
  ${KWWidgets_BINARY_DIR}/KWWidgetsBuildSettings.cmake)

# The library directories.
set(KWWidgets_LIBRARY_DIRS_CONFIG ${KWWidgets_LIBRARY_DIRS})

# The runtime directories.
set(KWWidgets_RUNTIME_DIRS_CONFIG ${KWWidgets_RUNTIME_DIRS})

# The include directories.
set(KWWidgets_INCLUDE_DIRS_CONFIG ${KWWidgets_INCLUDE_PATH})

# The library dependencies file.
set(KWWidgets_LIBRARY_DEPENDS_FILE 
  ${KWWidgets_BINARY_DIR}/KWWidgetsLibraryDepends.cmake)

# The examples dir.
set(KWWidgets_EXAMPLES_DIR_CONFIG 
  ${KWWidgets_EXAMPLES_DIR})

# The templates dir.
set(KWWidgets_TEMPLATES_DIR_CONFIG 
  ${KWWidgets_TEMPLATES_DIR})

# The resources dir.
set(KWWidgets_RESOURCES_DIR_CONFIG 
  ${KWWidgets_RESOURCES_DIR})

# The CMake macros dir.
set(KWWidgets_CMAKE_DIR_CONFIG 
  ${KWWidgets_CMAKE_DIR})

# The Tcl/Tk options.
set(KWWidgets_TCL_PACKAGE_INDEX_DIR_CONFIG ${KWWidgets_TCL_PACKAGE_INDEX_DIR})

# The Python options.
set(KWWidgets_PYTHON_MODULE_DIR_CONFIG ${KWWidgets_PYTHON_MODULE_DIR})

# The Doxygen options.
set(KWWidgets_DOXYGEN_DIR_CONFIG 
  ${KWWidgets_SOURCE_DIR}/Utilities/Doxygen)

# The VTK options.
set(KWWidgets_VTK_DIR_CONFIG ${VTK_DIR})

# The gettext library. This is only defined for build dirs to help projects
# find the gettext library (it is safe to assume we are on the same machine;
# we could not guarantee that if we are building against an installed 
# KWWidgets though).
get_filename_component(
  GETTEXT_SEARCH_PATH_CONFIG "${GETTEXT_INTL_LIBRARY}" PATH)

# The build configuration information.
set(KWWidgets_CONFIGURATION_TYPES_CONFIG ${KWWidgets_CONFIGURATION_TYPES})
set(KWWidgets_BUILD_TYPE_CONFIG ${CMAKE_BUILD_TYPE})

# Configure KWWidgetsConfig.cmake for the build tree.
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/KWWidgetsConfig.cmake.in
  ${KWWidgets_BINARY_DIR}/KWWidgetsConfig.cmake @ONLY IMMEDIATE)

#-----------------------------------------------------------------------------
# Settings specific to the install tree.

# The "use" file.
set(KWWidgets_USE_FILE_CONFIG 
  ${DOLLAR}{KWWidgets_INSTALL_PREFIX}${KWWidgets_INSTALL_PACKAGE_DIR}/UseKWWidgets.cmake)

# The build settings file.
set(KWWidgets_BUILD_SETTINGS_FILE_CONFIG 
  ${DOLLAR}{KWWidgets_INSTALL_PREFIX}${KWWidgets_INSTALL_PACKAGE_DIR}/KWWidgetsBuildSettings.cmake)

# The library directories.
if(CYGWIN AND KWWidgets_BUILD_SHARED_LIBS)
  # In Cygwin programs directly link to the .dll files.
  set(KWWidgets_LIBRARY_DIRS_CONFIG 
    ${DOLLAR}{KWWidgets_INSTALL_PREFIX}${KWWidgets_INSTALL_BIN_DIR})
else(CYGWIN AND KWWidgets_BUILD_SHARED_LIBS)
  set(KWWidgets_LIBRARY_DIRS_CONFIG 
    ${DOLLAR}{KWWidgets_INSTALL_PREFIX}${KWWidgets_INSTALL_LIB_DIR})
endif(CYGWIN AND KWWidgets_BUILD_SHARED_LIBS)

# The runtime directories.
if(WIN32)
  set(KWWidgets_RUNTIME_DIRS_CONFIG 
    ${DOLLAR}{KWWidgets_INSTALL_PREFIX}${KWWidgets_INSTALL_BIN_DIR})
else(WIN32)
  set(KWWidgets_RUNTIME_DIRS_CONFIG 
    ${DOLLAR}{KWWidgets_INSTALL_PREFIX}${KWWidgets_INSTALL_LIB_DIR})
endif(WIN32)

# The include directories.
set(KWWidgets_INCLUDE_DIRS_CONFIG
  ${DOLLAR}{KWWidgets_INSTALL_PREFIX}${KWWidgets_INSTALL_INCLUDE_DIR})

# The library dependencies file.
set(KWWidgets_LIBRARY_DEPENDS_FILE 
  ${DOLLAR}{KWWidgets_INSTALL_PREFIX}${KWWidgets_INSTALL_PACKAGE_DIR}/KWWidgetsLibraryDepends.cmake)

# The examples dir.
set(KWWidgets_EXAMPLES_DIR_CONFIG 
  ${DOLLAR}{KWWidgets_INSTALL_PREFIX}${KWWidgets_INSTALL_DATA_DIR}/Examples)

# The templates dir.
set(KWWidgets_TEMPLATES_DIR_CONFIG 
  ${DOLLAR}{KWWidgets_INSTALL_PREFIX}${KWWidgets_INSTALL_INCLUDE_DIR}/Templates)

# The resources dir.
set(KWWidgets_RESOURCES_DIR_CONFIG 
  ${DOLLAR}{KWWidgets_INSTALL_PREFIX}${KWWidgets_INSTALL_DATA_DIR}/Resources)

# The CMake macros dir.
set(KWWidgets_CMAKE_DIR_CONFIG 
  ${DOLLAR}{KWWidgets_INSTALL_PREFIX}${KWWidgets_INSTALL_PACKAGE_DIR}/CMake)

# The Tcl/Tk options.
set(KWWidgets_TCL_PACKAGE_INDEX_DIR_CONFIG 
  ${DOLLAR}{KWWidgets_INSTALL_PREFIX}${KWWidgets_INSTALL_LIB_DIR}/tcl)

# The Python options.
if(WIN32)
  set(python_subdir "")
else(WIN32)
  if(PYTHON_EXECUTABLE)
    exec_program("${PYTHON_EXECUTABLE}" ARGS "-V" OUTPUT_VARIABLE version)
    string(REGEX REPLACE "^(Python )([0-9]\\.[0-9])(.*)$" "\\2" 
      major_minor "${version}")
    set(python_subdir "python${major_minor}/")
  else(PYTHON_EXECUTABLE)
    set(python_subdir "python2.4/")
  endif(PYTHON_EXECUTABLE)
endif(WIN32)
set(KWWidgets_PYTHON_MODULE_DIR_CONFIG 
  ${DOLLAR}{KWWidgets_INSTALL_PREFIX}${KWWidgets_INSTALL_LIB_DIR}/../${python_subdir}site-packages)

# The Doxygen options.
set(KWWidgets_DOXYGEN_DIR_CONFIG 
  ${DOLLAR}{KWWidgets_INSTALL_PREFIX}${KWWidgets_INSTALL_DOXYGEN_DIR})

# The VTK options.
# If VTK_BINARY_DIR is set, then KWWidgets is part of a larger parent project
# that also builds VTK. In that case, KWWidgets_VTK_INSTALL_PACKAGE_DIR
# should be set to the value of VTK_INSTALL_PACKAGE_DIR. Otherwise, if not set,
# then we are using an outside VTK whose location will not change when we
# install.
if(VTK_BINARY_DIR)
  if(NOT KWWidgets_VTK_INSTALL_PACKAGE_DIR)
    message(
      "KWWidgets_VTK_INSTALL_PACKAGE_DIR must be set by the parent project to the value of VTK_INSTALL_PACKAGE_DIR it uses to configure VTK.")
  endif(NOT KWWidgets_VTK_INSTALL_PACKAGE_DIR)
  set(KWWidgets_VTK_DIR_CONFIG ${KWWidgets_VTK_INSTALL_PACKAGE_DIR})
else(VTK_BINARY_DIR)
  # Otherwise try to find VTK relative to ourself
  set(KWWidgets_VTK_DIR_CONFIG ${DOLLAR}{KWWidgets_INSTALL_PREFIX}${VTK_INSTALL_LIB_DIR})
endif(VTK_BINARY_DIR)

# The gettext library. This is only defined for build dirs to help projects
# find the gettext library (it is safe to assume we are on the same machine;
# we could not guarantee that if we are building against an installed 
# KWWidgets though).
set(GETTEXT_SEARCH_PATH_CONFIG)

# The build configuration information.
# The install tree only has one configuration.
set(KWWidgets_CONFIGURATION_TYPES_CONFIG)

# Configure KWWidgetsConfig.cmake for the install tree.

# Construct the proper number of GET_FILENAME_COMPONENT(... PATH)
# calls to compute the installation prefix from KWWidgets_DIR.
string(REGEX REPLACE "/" ";" KWWidgets_INSTALL_PACKAGE_DIR_COUNT
  "${KWWidgets_INSTALL_PACKAGE_DIR}")
set(KWWidgets_CONFIG_INSTALL_ONLY "
# Compute the installation prefix from KWWidgets_DIR.
set(KWWidgets_INSTALL_PREFIX \"${DOLLAR}{KWWidgets_DIR}\")
")
foreach(p ${KWWidgets_INSTALL_PACKAGE_DIR_COUNT})
  set(KWWidgets_CONFIG_INSTALL_ONLY
    "${KWWidgets_CONFIG_INSTALL_ONLY}GET_FILENAME_COMPONENT(KWWidgets_INSTALL_PREFIX \"${DOLLAR}{KWWidgets_INSTALL_PREFIX}\" PATH)\n"
    )
endforeach(p)

if(CMAKE_CONFIGURATION_TYPES)
  # There are multiple build configurations.  Configure one
  # KWWidgetsConfig.cmake for each configuration.
  foreach(config ${CMAKE_CONFIGURATION_TYPES})
    set(KWWidgets_BUILD_TYPE_CONFIG ${config})
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/KWWidgetsConfig.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/${config}/KWWidgetsConfig.cmake
      @ONLY IMMEDIATE)
  endforeach(config)

  # Install the config file corresponding to the build configuration
  # specified when building the install target.  The BUILD_TYPE variable
  # will be set while CMake is processing the install files.
  if(NOT KWWidgets_INSTALL_NO_DEVELOPMENT)
    install_files(${KWWidgets_INSTALL_PACKAGE_DIR} FILES
      ${CMAKE_CURRENT_BINARY_DIR}/${DOLLAR}{BUILD_TYPE}/KWWidgetsConfig.cmake)
  endif(NOT KWWidgets_INSTALL_NO_DEVELOPMENT)
else(CMAKE_CONFIGURATION_TYPES)
  # There is only one build configuration. Configure one KWWidgetsConfig.cmake.
  set(KWWidgets_BUILD_TYPE_CONFIG ${CMAKE_BUILD_TYPE})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/KWWidgetsConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/KWWidgetsConfig.cmake @ONLY IMMEDIATE)

  # Setup an install rule for the config file.
  if(NOT KWWidgets_INSTALL_NO_DEVELOPMENT)
    install_files(${KWWidgets_INSTALL_PACKAGE_DIR} FILES
      ${CMAKE_CURRENT_BINARY_DIR}/KWWidgetsConfig.cmake)
  endif(NOT KWWidgets_INSTALL_NO_DEVELOPMENT)
endif(CMAKE_CONFIGURATION_TYPES)
