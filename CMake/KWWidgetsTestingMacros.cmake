# ---------------------------------------------------------------------------
# KWWidgets_GET_CMAKE_BUILD_TYPE
# Get CMAKE_BUILD_TYPE

macro(KWWidgets_GET_CMAKE_BUILD_TYPE varname)

  set(cmake_build_type_found "${CMAKE_BUILD_TYPE}")
  if(CMAKE_CONFIGURATION_TYPES)
    if(NOT cmake_build_type_found)
      foreach(var ${CMAKE_CONFIGURATION_TYPES})
        if(NOT cmake_build_type_found)
          set(cmake_build_type_found "${var}")
        endif(NOT cmake_build_type_found)
      endforeach(var)
    endif(NOT cmake_build_type_found)
  endif(CMAKE_CONFIGURATION_TYPES)
  
  set(${varname} ${cmake_build_type_found})

endmacro(KWWidgets_GET_CMAKE_BUILD_TYPE)

# ---------------------------------------------------------------------------
# KWWidgets_GET_FULL_PATH_TO_EXECUTABLE
# Get full path to exec

macro(KWWidgets_GET_FULL_PATH_TO_EXECUTABLE exe_name varname)

  get_filename_component(exe_name_name "${exe_name}" NAME)
  if("${exe_name_name}" STREQUAL "${exe_name}")
    if(EXECUTABLE_OUTPUT_PATH)
      set(exe_dir "${EXECUTABLE_OUTPUT_PATH}/")
    else(EXECUTABLE_OUTPUT_PATH)
      set(exe_dir "${CMAKE_CURRENT_BINARY_DIR}/")
    endif(EXECUTABLE_OUTPUT_PATH)
  else("${exe_name_name}" STREQUAL "${exe_name}")
    set(exe_dir)
  endif("${exe_name_name}" STREQUAL "${exe_name}")

  kwwidgets_get_cmake_build_type(DEFAULT_CMAKE_BUILD_TYPE)
  if(CMAKE_CONFIGURATION_TYPES)
    set(CONFIGURATION_TYPE "${DEFAULT_CMAKE_BUILD_TYPE}/")
  else(CMAKE_CONFIGURATION_TYPES)
    set(CONFIGURATION_TYPE)
  endif(CMAKE_CONFIGURATION_TYPES)

  set(${varname} "${exe_dir}${CONFIGURATION_TYPE}${exe_name}")

endmacro(KWWidgets_GET_FULL_PATH_TO_EXECUTABLE)

# ---------------------------------------------------------------------------
# KWWidgets_ADD_TEST_WITH_LAUNCHER
# Add specific distribution-related C test

macro(KWWidgets_ADD_TEST_WITH_LAUNCHER
    test_name
    exe_name)

  # If we are building the test from the library itself, use the
  # unique launcher created by the library, instead of creating
  # a specific launcher when building out-of-source.
  
  if(KWWidgets_SOURCE_DIR)
    set(LAUNCHER_EXE_NAME "KWWidgetsSetupPathsLauncher")
    kwwidgets_get_full_path_to_executable(${exe_name} exe_path)
    add_test(${test_name} 
      ${EXECUTABLE_OUTPUT_PATH}/${LAUNCHER_EXE_NAME} ${exe_path} ${ARGN})
  else(KWWidgets_SOURCE_DIR)
    include("${KWWidgets_CMAKE_DIR}/KWWidgetsPathsMacros.cmake")
    set(LAUNCHER_EXE_NAME "${exe_name}Launcher")
    kwwidgets_generate_setup_paths_launcher(
      "${CMAKE_CURRENT_BINARY_DIR}" "${LAUNCHER_EXE_NAME}" 
      "${EXECUTABLE_OUTPUT_PATH}" "${exe_name}")
    add_test(${test_name} 
      ${EXECUTABLE_OUTPUT_PATH}/${LAUNCHER_EXE_NAME} ${ARGN})
  endif(KWWidgets_SOURCE_DIR)

endmacro(KWWidgets_ADD_TEST_WITH_LAUNCHER)

# ---------------------------------------------------------------------------
# KWWidgets_ADD_TEST_FROM_EXAMPLE
# Add specific distribution-related test

macro(KWWidgets_ADD_TEST_FROM_EXAMPLE 
    test_name
    exe_name exe_options
    out_of_source_exe_name out_of_source_exe_options)

  add_test(${test_name} 
    ${EXECUTABLE_OUTPUT_PATH}/${exe_name} ${exe_options} ${ARGN})

  if(KWWidgets_SOURCE_DIR AND KWWidgets_TEST_OUT_OF_SOURCE)
    kwwidgets_add_out_of_source_test(
      ${test_name}OoS
      ${PROJECT_NAME}
      "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}OoS"
      ${out_of_source_exe_name} ${out_of_source_exe_options} ${ARGN})
  endif(KWWidgets_SOURCE_DIR AND KWWidgets_TEST_OUT_OF_SOURCE)

endmacro(KWWidgets_ADD_TEST_FROM_EXAMPLE)

# ---------------------------------------------------------------------------
# KWWidgets_ADD_TEST_FROM_C_EXAMPLE
# Add specific distribution-related C test

macro(KWWidgets_ADD_TEST_FROM_C_EXAMPLE 
    test_name
    exe_name)

  # Try to find the full path to the test executable

  kwwidgets_get_full_path_to_executable(${exe_name} exe_path)

  # If we are building the test from the library itself, use the
  # unique launcher created by the library, instead of creating
  # a specific launcher when building out-of-source.
  
  set(LAUNCHER_EXE_NAME "${exe_name}Launcher")

  if(KWWidgets_SOURCE_DIR)
    kwwidgets_add_test_from_example(${test_name} 
      "KWWidgetsSetupPathsLauncher" "${exe_path}"
      ${LAUNCHER_EXE_NAME} ""
      "--test")
  else(KWWidgets_SOURCE_DIR)
    # No need to create a launcher (supposed to be done by the example already)
    kwwidgets_add_test_from_example(${test_name} 
      ${LAUNCHER_EXE_NAME} ""
      ${LAUNCHER_EXE_NAME} ""
      "--test")
  endif(KWWidgets_SOURCE_DIR)

endmacro(KWWidgets_ADD_TEST_FROM_C_EXAMPLE)

# ---------------------------------------------------------------------------
# KWWidgets_ADD_TEST_FROM_TCL_EXAMPLE
# Add specific distribution-related Tcl test

macro(KWWidgets_ADD_TEST_FROM_TCL_EXAMPLE 
    test_name
    script_name)

  if(KWWidgets_BUILD_SHARED_LIBS AND VTK_WRAP_TCL AND TCL_TCLSH)

    get_filename_component(name_we "${script_name}" NAME_WE)
    set(LAUNCHER_EXE_NAME "${name_we}TclLauncher")

    # If we are building the test from the library itself, use the
    # unique launcher created by the library, instead of creating
    # a specific launcher when building out-of-source.

    if(KWWidgets_SOURCE_DIR)
      kwwidgets_add_test_from_example(${test_name} 
        "KWWidgetsSetupPathsLauncher" "${TCL_TCLSH}"
        ${LAUNCHER_EXE_NAME} ""
        "${script_name}" "--test")
    else(KWWidgets_SOURCE_DIR)
      include("${KWWidgets_CMAKE_DIR}/KWWidgetsPathsMacros.cmake")
      kwwidgets_generate_setup_paths_launcher(
        "${CMAKE_CURRENT_BINARY_DIR}" "${LAUNCHER_EXE_NAME}" "" "${TCL_TCLSH}")
      kwwidgets_add_test_from_example(${test_name} 
        ${LAUNCHER_EXE_NAME} "" 
        ${LAUNCHER_EXE_NAME} "" 
        "${script_name}" "--test")
    endif(KWWidgets_SOURCE_DIR)

  endif(KWWidgets_BUILD_SHARED_LIBS AND VTK_WRAP_TCL AND TCL_TCLSH)

endmacro(KWWidgets_ADD_TEST_FROM_TCL_EXAMPLE)

# ---------------------------------------------------------------------------
# KWWidgets_ADD_TEST_FROM_PYTHON_EXAMPLE
# Add specific distribution-related Python test

macro(KWWidgets_ADD_TEST_FROM_PYTHON_EXAMPLE 
    test_name
    script_name)

  if(KWWidgets_BUILD_SHARED_LIBS AND VTK_WRAP_PYTHON AND PYTHON_EXECUTABLE)

    get_filename_component(name_we "${script_name}" NAME_WE)
    set(LAUNCHER_EXE_NAME "${name_we}PythonLauncher")

    # If we are building the test from the library itself, use the
    # unique launcher created by the library, instead of creating
    # a specific launcher when building out-of-source.

    if(KWWidgets_SOURCE_DIR)
      kwwidgets_add_test_from_example(${test_name} 
        "KWWidgetsSetupPathsLauncher" "${PYTHON_EXECUTABLE}"
        ${LAUNCHER_EXE_NAME} ""
        "${script_name}" "--test")
    else(KWWidgets_SOURCE_DIR)
      include("${KWWidgets_CMAKE_DIR}/KWWidgetsPathsMacros.cmake")
      kwwidgets_generate_setup_paths_launcher(
        "${CMAKE_CURRENT_BINARY_DIR}" "${LAUNCHER_EXE_NAME}" "" "${PYTHON_EXECUTABLE}")
      kwwidgets_add_test_from_example(${test_name} 
        ${LAUNCHER_EXE_NAME} "" 
        ${LAUNCHER_EXE_NAME} "" 
        "${script_name}" "--test")
    endif(KWWidgets_SOURCE_DIR)

  endif(KWWidgets_BUILD_SHARED_LIBS AND VTK_WRAP_PYTHON AND PYTHON_EXECUTABLE)

endmacro(KWWidgets_ADD_TEST_FROM_PYTHON_EXAMPLE)

# ---------------------------------------------------------------------------
# KWWidgets_ADD_OUT_OF_SOURCE_TEST
# Add an out-of-source test 

macro(KWWidgets_ADD_OUT_OF_SOURCE_TEST 
    test_name 
    project_name 
    src_dir bin_dir 
    exe_name)

  if(VTK_WRAP_TCL)

    kwwidgets_get_cmake_build_type(DEFAULT_CMAKE_BUILD_TYPE)
    if(CMAKE_CONFIGURATION_TYPES)
      set(CONFIGURATION_TYPE "${DEFAULT_CMAKE_BUILD_TYPE}/")
    else(CMAKE_CONFIGURATION_TYPES)
      set(CONFIGURATION_TYPE)
    endif(CMAKE_CONFIGURATION_TYPES)

    add_test("${test_name}" ${CMAKE_CTEST_COMMAND}
      --build-and-test "${src_dir}" "${bin_dir}"
      --build-generator ${CMAKE_GENERATOR}
      --build-makeprogram ${CMAKE_MAKE_PROGRAM}
      --build-project ${project_name}
      --build-config ${DEFAULT_CMAKE_BUILD_TYPE}
      --build-options 
      "-DKWWidgets_DIR:PATH=${KWWidgets_BINARY_DIR}" 
      "-DCMAKE_MAKE_PROGRAM:FILEPATH=${CMAKE_MAKE_PROGRAM}"
      "-DSOV_DIR:PATH=${SOV_DIR}"
      "-DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}"
      "-DBUILD_TESTING:BOOL=ON"
      "-DTCL_TCLSH:FILEPATH=${TCL_TCLSH}"
      "-DTCL_LIBRARY:FILEPATH=${TCL_LIBRARY}"
      "-DTK_WISH:FILEPATH=${TK_WISH}"
      "-DTK_LIBRARY:FILEPATH=${TK_LIBRARY}"
      "-DPYTHON_EXECUTABLE:FILEPATH=${PYTHON_EXECUTABLE}"
      "-DGETTEXT_INTL_LIBRARY:FILEPATH=${GETTEXT_INTL_LIBRARY}"
      "-DGETTEXT_INCLUDE_DIR:PATH=${GETTEXT_INCLUDE_DIR}"
      "-DGETTEXT_MSGCAT_EXECUTABLE:FILEPATH=${GETTEXT_MSGCAT_EXECUTABLE}"
      "-DGETTEXT_MSGCONV_EXECUTABLE:FILEPATH=${GETTEXT_MSGCONV_EXECUTABLE}"
      "-DGETTEXT_MSGFMT_EXECUTABLE:FILEPATH=${GETTEXT_MSGFMT_EXECUTABLE}"
      "-DGETTEXT_MSGINIT_EXECUTABLE:FILEPATH=${GETTEXT_MSGINIT_EXECUTABLE}"
      "-DGETTEXT_MSGMERGE_EXECUTABLE:FILEPATH=${GETTEXT_MSGMERGE_EXECUTABLE}"
      "-DGETTEXT_XGETTEXT_EXECUTABLE:FILEPATH=${GETTEXT_XGETTEXT_EXECUTABLE}"
      --test-command "${CONFIGURATION_TYPE}${exe_name}" ${ARGN})

    if(KWWidgets_TEST_INSTALLATION AND CMAKE_INSTALL_PREFIX)
      add_test("${test_name}wInst" ${CMAKE_CTEST_COMMAND}
        --build-and-test "${src_dir}" "${bin_dir}wInst"
        --build-generator ${CMAKE_GENERATOR}
        --build-makeprogram ${CMAKE_MAKE_PROGRAM}
        --build-project ${project_name}
        --build-config ${DEFAULT_CMAKE_BUILD_TYPE}
        --build-options 
        "-DKWWidgets_DIR:PATH=${CMAKE_INSTALL_PREFIX}/lib/KWWidgets" 
        "-DCMAKE_MAKE_PROGRAM:FILEPATH=${CMAKE_MAKE_PROGRAM}"
        "-DSOV_DIR:PATH=${SOV_DIR}"
        "-DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}"
        "-DBUILD_TESTING:BOOL=ON"
        "-DTCL_TCLSH:FILEPATH=${TCL_TCLSH}"
        "-DTCL_LIBRARY:FILEPATH=${TCL_LIBRARY}"
        "-DTK_WISH:FILEPATH=${TK_WISH}"
        "-DTK_LIBRARY:FILEPATH=${TK_LIBRARY}"
        "-DPYTHON_EXECUTABLE:FILEPATH=${PYTHON_EXECUTABLE}"
        "-DGETTEXT_INTL_LIBRARY:FILEPATH=${GETTEXT_INTL_LIBRARY}"
        "-DGETTEXT_INCLUDE_DIR:PATH=${GETTEXT_INCLUDE_DIR}"
        "-DGETTEXT_MSGCAT_EXECUTABLE:FILEPATH=${GETTEXT_MSGCAT_EXECUTABLE}"
        "-DGETTEXT_MSGCONV_EXECUTABLE:FILEPATH=${GETTEXT_MSGCONV_EXECUTABLE}"
        "-DGETTEXT_MSGFMT_EXECUTABLE:FILEPATH=${GETTEXT_MSGFMT_EXECUTABLE}"
        "-DGETTEXT_MSGINIT_EXECUTABLE:FILEPATH=${GETTEXT_MSGINIT_EXECUTABLE}"
        "-DGETTEXT_MSGMERGE_EXECUTABLE:FILEPATH=${GETTEXT_MSGMERGE_EXECUTABLE}"
        "-DGETTEXT_XGETTEXT_EXECUTABLE:FILEPATH=${GETTEXT_XGETTEXT_EXECUTABLE}"
        --test-command "${CONFIGURATION_TYPE}${exe_name}" ${ARGN})
    endif(KWWidgets_TEST_INSTALLATION AND CMAKE_INSTALL_PREFIX)

  endif(VTK_WRAP_TCL)

endmacro(KWWidgets_ADD_OUT_OF_SOURCE_TEST)
