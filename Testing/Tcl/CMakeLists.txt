if(VTK_PRINT_SELF_CHECK_TCL AND TCL_TCLSH)
  add_test(KWWidgets-PrintSelf ${TCL_TCLSH}
    ${VTK_PRINT_SELF_CHECK_TCL}
    ${KWWidgets_SOURCE_DIR})
endif(VTK_PRINT_SELF_CHECK_TCL AND TCL_TCLSH)

if(VTK_FIND_STRING_TCL AND TCL_TCLSH)
  add_test(KWWidgets-SetObjectMacro ${TCL_TCLSH}
    ${VTK_FIND_STRING_TCL}
    "${KWWidgets_SOURCE_DIR}/vtk\\\\*.h"
    "vtkSetObjectMacro")
endif(VTK_FIND_STRING_TCL AND TCL_TCLSH)
