#
# ---- Find Squish
# This module can be used to find Squish (currently support is aimed at version 3).
#
# ---- Variables and Macros
# SQUISH_FOUND                    If false, don't try to use Squish
# SQUISH_VERSION_MAJOR            The major version of Squish found
# SQUISH_VERSION_MINOR            The minor version of Squish found
# SQUISH_VERSION_PATCH            The patch version of Squish found
#
# SQUISH_DIR                      The Squish installation or build directory (containing bin, lib, etc)
# SQUISH_SERVER_EXECUTABLE        The squishserver executable
# SQUISH_CLIENT_EXECUTABLE        The squishrunner executable
#
# SQUISH_DIR_FOUND                Was the install or build directory found?
# SQUISH_SERVER_EXECUTABLE_FOUND  Was the server executable found?
# SQUISH_CLIENT_EXECUTABLE_FOUND  Was the client executable found?
#
# macro SQUISH_ADD_TEST(testName applicationUnderTest testSuite testCase)
#
# ---- Typical Use
# ENABLE_TESTING()
# FIND_PACKAGE(Squish)
# IF (SQUISH_FOUND)
#   SQUISH_ADD_TEST(myTestName myApplication testSuiteName testCaseName)
# ENDIF (SQUISH_FOUND)
#

SET(SQUISH_DIR_STRING "Directory containing the bin, doc, and lib directories for Squish; this should be the root of the installation directory.")
SET(SQUISH_SERVER_EXECUTABLE_STRING "The squishserver executable program.")
SET(SQUISH_CLIENT_EXECUTABLE_STRING "The squishclient executable program.")

# Search only if the location is not already known.
IF(NOT SQUISH_DIR)
  # Get the system search path as a list.
  IF(UNIX)
    STRING(REGEX MATCHALL "[^:]+" SQUISH_DIR_SEARCH1 "$ENV{PATH}")
  ELSE(UNIX)
    STRING(REGEX REPLACE "\\\\" "/" SQUISH_DIR_SEARCH1 "$ENV{PATH}")
  ENDIF(UNIX)
  STRING(REGEX REPLACE "/;" ";" SQUISH_DIR_SEARCH2 ${SQUISH_DIR_SEARCH1})

  # Construct a set of paths relative to the system search path.
  SET(SQUISH_DIR_SEARCH "")
  FOREACH(dir ${SQUISH_DIR_SEARCH2})
    SET(SQUISH_DIR_SEARCH ${SQUISH_DIR_SEARCH} "${dir}/../lib/fltk")
  ENDFOREACH(dir)

  # Look for an installation
  FIND_PATH(SQUISH_DIR bin/squishrunner
    # Look for an environment variable SQUISH_DIR.
    $ENV{SQUISH_DIR}

    # Look in places relative to the system executable search path.
    ${SQUISH_DIR_SEARCH}

    # Look in standard UNIX install locations.
    #/usr/local/squish

    DOC "The ${SQUISH_DIR_STRING}"
    )
ENDIF(NOT SQUISH_DIR)

# search for the executables
IF(SQUISH_DIR)
  SET(SQUISH_DIR_FOUND 1)

  # find the client program
  IF(NOT SQUISH_CLIENT_EXECUTABLE)
    FIND_PROGRAM(SQUISH_CLIENT_EXECUTABLE ${SQUISH_DIR}/bin/squishrunner DOC "The ${SQUISH_CLIENT_EXECUTABLE_STRING}")
  ENDIF(NOT SQUISH_CLIENT_EXECUTABLE)

  # find the server program
  IF(NOT SQUISH_SERVER_EXECUTABLE)
    FIND_PROGRAM(SQUISH_SERVER_EXECUTABLE ${SQUISH_DIR}/bin/squishserver DOC "The ${SQUISH_SERVER_EXECUTABLE_STRING}")
  ENDIF(NOT SQUISH_SERVER_EXECUTABLE)  

ELSE(SQUISH_DIR)
  SET(SQUISH_DIR_FOUND 0)
ENDIF(SQUISH_DIR)

# record if executables are set
IF(SQUISH_CLIENT_EXECUTABLE)
  SET(SQUISH_CLIENT_EXECUTABLE_FOUND 1)
ELSE(SQUISH_CLIENT_EXECUTABLE)    
  SET(SQUISH_CLIENT_EXECUTABLE_FOUND 0)    
ENDIF(SQUISH_CLIENT_EXECUTABLE)

IF(SQUISH_SERVER_EXECUTABLE)
  SET(SQUISH_SERVER_EXECUTABLE_FOUND 1)
ELSE(SQUISH_SERVER_EXECUTABLE)    
  SET(SQUISH_SERVER_EXECUTABLE_FOUND 0)    
ENDIF(SQUISH_SERVER_EXECUTABLE)

# record if Squish was found
SET(SQUISH_FOUND 1)
FOREACH(var SQUISH_DIR_FOUND SQUISH_CLIENT_EXECUTABLE_FOUND SQUISH_SERVER_EXECUTABLE_FOUND)
  IF(NOT ${var})
    SET(SQUISH_FOUND 0)
  ENDIF(NOT ${var})
ENDFOREACH(var)

MACRO(SQUISH_ADD_TEST testName testAUT testCase envVars testWraper)
  ADD_TEST(${testName}
    ${CMAKE_COMMAND} -V -VV
    "-Dsquish_aut:STRING=${testAUT}"
    "-Dsquish_server_executable:STRING=${SQUISH_SERVER_EXECUTABLE}"
    "-Dsquish_client_executable:STRING=${SQUISH_CLIENT_EXECUTABLE}"
    "-Dsquish_libqtdir:STRING=${QT_LIBRARY_DIR}"
    "-Dsquish_test_case:STRING=${testCase}"
    "-Dsquish_env_vars:STRING=${envVars}"
    "-Dsquish_wrapper:STRING=${testWraper}"
    -P "${KWWidgets_CMAKE_DIR}/SquishTestScript.cmake"
    )
  SET_TESTS_PROPERTIES(${testName}
    PROPERTIES FAIL_REGULAR_EXPRESSION "FAILED;ERROR;FATAL"
    )
ENDMACRO(SQUISH_ADD_TEST)

