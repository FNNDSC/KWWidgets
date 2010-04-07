#
# This script launches a GUI test using Squish.  You should not call
# the script directly; instead, you should acces it via the
# SQUISH_ADD_TEST macro that is defined in FindSquish.cmake.
#
# This script starts the Squish server, launches the test on the
# client, and finally stops the squish server.  If any of these steps
# fail (including if the tests do not pass) then a fatal error is
# raised.
#

# print out the variable that we are using
message(STATUS "squish_aut='${squish_aut}'")
message(STATUS "squish_aut_args='${squish_aut_args}'")

message(STATUS "squish_server_executable='${squish_server_executable}'")
message(STATUS "squish_client_executable='${squish_client_executable}'")
message(STATUS "squish_libqtdir ='${squish_libqtdir}'")
message(STATUS "squish_test_case='${squish_test_case}'")
message(STATUS "squish_wrapper='${squish_wrapper}'")
message(STATUS "squish_env_vars='${squish_env_vars}'")
message(STATUS "squish_aut_script='${squish_aut_script}'")

set (squish_aut "${squish_aut} ${squish_aut_args}")
    
set (kwwSquishSh "${squish_shell_script}")
message(STATUS "The_sh_torun='${kwwSquishSh}'")

# parse enviornment variables
foreach(i ${squish_env_vars})
  message(STATUS "parsing env var key/value pair ${i}")
  string(REGEX MATCH "([^=]*)=(.*)" squish_env_name ${i})
  message(STATUS "key=${CMAKE_MATCH_1}")
  message(STATUS "value=${CMAKE_MATCH_2}")
  set ( ENV{${CMAKE_MATCH_1}} ${CMAKE_MATCH_2} )
endforeach()

if (QT4_INSTALLED)
  # record qt lib directory
  set ( ENV{${SQUISH_LIBQTDIR}} ${squish_libqtdir} )
endif (QT4_INSTALLED)

# run the test

if (UNIX)
  execute_process(
    COMMAND ${kwwSquishSh} ${squish_server_executable} ${squish_client_executable} ${squish_test_case} ${squish_wrapper} ${squish_aut}
    RESULT_VARIABLE test_rv
    )
endif (UNIX)

# check for an error with running the test
if(NOT "${test_rv}" STREQUAL "0")
  message(FATAL_ERROR "Error running Squish test")
endif(NOT "${test_rv}" STREQUAL "0")

