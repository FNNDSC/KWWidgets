#
# Build the documentation
#

include (${CMAKE_ROOT}/Modules/Documentation.cmake)

if (BUILD_DOCUMENTATION)

  option (DOCUMENTATION_DOWNLOAD_VTK_TAGFILE
    "Download the VTK tag file." ON)
  mark_as_advanced (
    DOCUMENTATION_DOWNLOAD_VTK_TAGFILE
    )

  # Backward compat 

  if(NOT DEFINED DOXYGEN AND DEFINED DOXYGEN_EXECUTABLE)
    set(DOXYGEN ${DOXYGEN_EXECUTABLE})
  endif(NOT DEFINED DOXYGEN AND DEFINED DOXYGEN_EXECUTABLE)

  if(NOT DEFINED DOT AND DEFINED DOXYGEN_DOT_EXECUTABLE)
    set(DOT ${DOXYGEN_DOT_EXECUTABLE})
  endif(NOT DEFINED DOT AND DEFINED DOXYGEN_DOT_EXECUTABLE)

  #
  # Configure the script and the doxyfile, then add target
  #
  set(HAVE_DOT_YESNO NO)
  if(DOT)
    set(HAVE_DOT_YESNO YES)
    if(NOT DOT_PATH)
      get_filename_component(DOT_PATH ${DOT} PATH)
    endif(NOT DOT_PATH)
  endif(DOT)

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/doxyfile)

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/doc_makeall.sh.in
    ${CMAKE_CURRENT_BINARY_DIR}/doc_makeall.sh)

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/doc_mainpage.dox.in
    ${CMAKE_CURRENT_BINARY_DIR}/doc_mainpage.dox)

  add_custom_target(KWWidgetsDoxygenDoc
    ${BASH}
    ${CMAKE_CURRENT_BINARY_DIR}/doc_makeall.sh)

endif (BUILD_DOCUMENTATION)
