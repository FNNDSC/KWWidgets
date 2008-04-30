# Handle out-of-source builds correctly.
#
#  1. Create a list of Python files to be installed/copied.
#  2. Copy them to KWWIDGETS_BINARY_DIR if different from KWWIDGETS_SOURCE_DIR.
#  3. Use Python's compileall to compile the copied files.
#
# *** Step 1 has to be done carefully to avoid missing out files ***

if(PYTHON_EXECUTABLE)

  # Wrapping/Python/kwwidgets.py
  set(KWWidgets_PYTHON_FILES 
    kwwidgets
    )

  # Done listing of files.

  # Now copy these files if necessary.
  set(KWWidgets_PYTHON_SOURCE_FILES)
  set(KWWidgets_PYTHON_OUTPUT_FILES)
  foreach(file ${KWWidgets_PYTHON_FILES})
    set(src "${KWWidgets_BINARY_DIR}/Wrapping/Python/${file}.py")
    set(KWWidgets_PYTHON_SOURCE_FILES ${KWWidgets_PYTHON_SOURCE_FILES} ${src})
  endforeach(file)

  if ("${KWWidgets_BINARY_DIR}" MATCHES "^${KWWidgets_SOURCE_DIR}$")
    #MESSAGE("In source build -- no need to copy Python files.")
  else ("${KWWidgets_BINARY_DIR}" MATCHES "^${KWWidgets_SOURCE_DIR}$")
    foreach(file ${KWWidgets_PYTHON_FILES})
      set(src "${KWWidgets_SOURCE_DIR}/Wrapping/Python/${file}.py")
      set(tgt "${KWWidgets_BINARY_DIR}/Wrapping/Python/${file}.py")
      add_custom_command(DEPENDS ${src}
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy ${src} ${tgt}
        OUTPUT ${tgt}
        COMMENT "source copy")
    endforeach(file)
  endif ("${KWWidgets_BINARY_DIR}" MATCHES "^${KWWidgets_SOURCE_DIR}$")

  # Byte compile the Python files.
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/compile_all_vtk.py.in
    ${CMAKE_CURRENT_BINARY_DIR}/compile_all_vtk.py
    @ONLY IMMEDIATE)
  add_custom_command(
    COMMAND ${PYTHON_EXECUTABLE}
    ARGS ${CMAKE_CURRENT_BINARY_DIR}/compile_all_vtk.py
    DEPENDS ${KWWidgets_PYTHON_SOURCE_FILES} ${CMAKE_CURRENT_BINARY_DIR}/compile_all_vtk.py
    OUTPUT "${KWWidgets_BINARY_DIR}/Wrapping/Python/vtk_compile_complete"
    )

  # Create a target on which custom commands can depend.  When this
  # is up-to-date, it is safe to run VTK python code.  Therefore
  # this should also depend on the vtkpython executable.
  add_custom_target(vtkpython_pyc ALL echo "..."
    DEPENDS "${KWWidgets_BINARY_DIR}/Wrapping/Python/vtk_compile_complete")

  # If no runtime is to be installed then do not install python modules.
  if(KWWidgets_INSTALL_NO_RUNTIME)
    set(KWWidgets_INSTALL_NO_PYTHON 1)
  endif(KWWidgets_INSTALL_NO_RUNTIME)

  # Add a rule to use python distutils to install the python wrappers.
  if(NOT KWWidgets_INSTALL_NO_PYTHON)
    set(DOLLAR "$")

    # Create default python setup arguments if they are not set.
    if(DEFINED KWWidgets_PYTHON_SETUP_ARGS)
    else(DEFINED KWWidgets_PYTHON_SETUP_ARGS)
      set(KWWidgets_PYTHON_SETUP_ARGS "--prefix=\"${DOLLAR}{CMAKE_INSTALL_PREFIX}\""
        CACHE STRING "Arguments passed to \"python setup.py install ...\" during installation.")
      mark_as_advanced(KWWidgets_PYTHON_SETUP_ARGS)
    endif(DEFINED KWWidgets_PYTHON_SETUP_ARGS)

    # If there are multiple configurations then add a BUILD_TYPE=...
    # argument to the python setup.py call.  The build type to use is set
    # in the CMake variable BUILD_TYPE while running the install script.
    if(CMAKE_CONFIGURATION_TYPES)
      set(KWWidgets_PYTHON_SETUP_BUILD_TYPE "BUILD_TYPE=${DOLLAR}{BUILD_TYPE}")
    else(CMAKE_CONFIGURATION_TYPES)
      set(KWWidgets_PYTHON_SETUP_BUILD_TYPE)
    endif(CMAKE_CONFIGURATION_TYPES)

    # Configure the post-install script to run python on setup.py.
    configure_file(${KWWidgets_SOURCE_DIR}/Wrapping/Python/PythonInstall.cmake.in
      ${KWWidgets_BINARY_DIR}/Wrapping/Python/PythonInstall.cmake
      @ONLY IMMEDIATE)
    set_target_properties(vtkpython_pyc PROPERTIES POST_INSTALL_SCRIPT
      ${KWWidgets_BINARY_DIR}/Wrapping/Python/PythonInstall.cmake
      )
  endif(NOT KWWidgets_INSTALL_NO_PYTHON)
endif(PYTHON_EXECUTABLE)

# Create the setup.py file.
if(CMAKE_CONFIGURATION_TYPES)
  # The build has multiple configuration types.  If CMAKE_BUILD_TYPE
  # is set use it as the default BUILD_TYPE for setup.py to install.
  set(KWWidgets_PYTHON_HAS_CONFIG_TYPES 1)
  if(CMAKE_BUILD_TYPE)
    set(KWWidgets_PYTHON_BUILD_TYPE "\"${CMAKE_BUILD_TYPE}\"")
  else(CMAKE_BUILD_TYPE)
    set(KWWidgets_PYTHON_BUILD_TYPE "[]")
  endif(CMAKE_BUILD_TYPE)
else(CMAKE_CONFIGURATION_TYPES)
  # The build has one configuration type.  The build type does not
  # affect installation.
  set(KWWidgets_PYTHON_HAS_CONFIG_TYPES 0)
  set(KWWidgets_PYTHON_BUILD_TYPE "[]")
endif(CMAKE_CONFIGURATION_TYPES)
configure_file(${KWWidgets_SOURCE_DIR}/Wrapping/Python/setup.py.in
  ${KWWidgets_BINARY_DIR}/Wrapping/Python/setup.py @ONLY IMMEDIATE)

