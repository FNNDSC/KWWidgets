#------------------------------------------------------------------------------
# tablelist::emptyStr
#
# Return an empty string. Useful for -formatcommand
#------------------------------------------------------------------------------
proc tablelist::emptyStr {arg} {
  return ""
}

# This used to be in tablelist.tcl, but we can't keep it there, as it relies
# on the auto-load feature (createBindings won't be found when tablelist.tcl
# is parsed, and would have Tcl load tablelistWidget.tcl automatically, 
# something we can't do from vtkKWTablelistInit.cxx)

::tablelist::createBindings
