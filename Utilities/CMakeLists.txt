add_executable(KWConvertImageToHeader KWConvertImageToHeader.cxx)
target_link_libraries(KWConvertImageToHeader ${KWWidgets_LIBRARIES})

subdirs(
  Doxygen
  )
