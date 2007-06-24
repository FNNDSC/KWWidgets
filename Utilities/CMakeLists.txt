ADD_EXECUTABLE(KWConvertImageToHeader KWConvertImageToHeader.cxx)
TARGET_LINK_LIBRARIES(KWConvertImageToHeader ${KWWidgets_LIBRARIES})

SUBDIRS(
  Doxygen
  TkTreeCtrl
  )

IF(KWWidgets_USE_TKDND)
  SUBDIRS(tkdnd)
ENDIF(KWWidgets_USE_TKDND)
