if(KWWidgets_BUILD_EXAMPLES)
  file(GLOB VTP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.vtp")
  install_files(
    "${KWWidgets_INSTALL_DATA_DIR}/Examples/Data" FILES ${VTP_FILES})

  file(GLOB VTI_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.vti")
  install_files(
    "${KWWidgets_INSTALL_DATA_DIR}/Examples/Data" FILES ${VTI_FILES})
endif(KWWidgets_BUILD_EXAMPLES)
