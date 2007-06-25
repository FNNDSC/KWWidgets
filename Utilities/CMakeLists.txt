ADD_EXECUTABLE(KWConvertImageToHeader KWConvertImageToHeader.cxx)
TARGET_LINK_LIBRARIES(KWConvertImageToHeader ${KWWidgets_LIBRARIES})

SUBDIRS(
  Doxygen
  )

IF(KWWidgets_USE_TKDND)
  SUBDIRS(tkdnd)
ENDIF(KWWidgets_USE_TKDND)

IF(KWWidgets_USE_TKTREECTRL)
  SUBDIRS(TkTreeCtrl)
ENDIF(KWWidgets_USE_TKTREECTRL)
