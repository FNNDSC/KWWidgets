if(WIN32)
  if(NOT KWWidgets_INSTALL_NO_DEVELOPMENT)
    install_files(${KWWidgets_INSTALL_DATA_DIR}/Resources FILES
      ${KWWidgets_RESOURCES_DIR}/KWWidgets.rc.in
      ${KWWidgets_RESOURCES_DIR}/KWWidgetsSetupManifest.xml)
    install_files(${KWWidgets_INSTALL_INCLUDE_DIR}/Resources FILES
      ${KWWidgets_RESOURCES_DIR}/KWWidgets.rc.h
      ${KWWidgets_RESOURCES_DIR}/KWWidgetsIcon16.ico
      ${KWWidgets_RESOURCES_DIR}/KWWidgetsIcon32.ico)
  endif(NOT KWWidgets_INSTALL_NO_DEVELOPMENT)
endif(WIN32)
