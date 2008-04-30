subdirs(Tcl)
subdirs(Cxx)

if (VTK_WRAP_PYTHON)
  subdirs(Python)
endif (VTK_WRAP_PYTHON)

if(PYTHON_EXECUTABLE AND VTK_HEADER_TESTING_PY)
  add_test(KWWidgets-HeaderTesting ${PYTHON_EXECUTABLE}
    ${VTK_HEADER_TESTING_PY}
    "${KWWidgets_SOURCE_DIR}"
    vtkKWEvent.h
    vtkKWWidgets.h
    vtkKWFileBrowserUtilities.h
    vtkKWRenderWidgetCallbackCommand.h
    )
endif(PYTHON_EXECUTABLE AND VTK_HEADER_TESTING_PY)
