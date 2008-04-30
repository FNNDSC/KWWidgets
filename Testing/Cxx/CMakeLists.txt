include("${KWWidgets_CMAKE_DIR}/KWWidgetsTestingMacros.cmake")

set(TESTS 
  Random
  Registry
  StateMachine
  )

foreach(TEST ${TESTS})
  set(EXE_NAME KWWidgetsTest${TEST})
  add_executable(${EXE_NAME} Test${TEST}.cxx)
  target_link_libraries(${EXE_NAME} ${KWWidgets_LIBRARIES})
  kwwidgets_add_test_with_launcher(KWWidgets-${TEST} ${EXE_NAME})
endforeach(TEST)
