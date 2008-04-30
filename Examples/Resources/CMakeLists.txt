if(KWWidgets_BUILD_EXAMPLES)
  file(GLOB PNG_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.png")
  install_files("${KWWidgets_INSTALL_DATA_DIR}/Examples/Resources" FILES ${PNG_FILES})
endif(KWWidgets_BUILD_EXAMPLES)
