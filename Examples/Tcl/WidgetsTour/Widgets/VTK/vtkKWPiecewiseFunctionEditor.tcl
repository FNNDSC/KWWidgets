proc vtkKWPiecewiseFunctionEditorEntryPoint {parent win} {

  set app [$parent GetApplication] 

  # Create the transfer function that will be modified by the 1st editor

  set pfed_tfunc1 [vtkPiecewiseFunction New]
  $pfed_tfunc1 AddPoint 0.0 0.1
  $pfed_tfunc1 AddPoint 128.0 0.3
  $pfed_tfunc1 AddPoint 512.0 0.8
  $pfed_tfunc1 AddPoint 1024.0 0.5

  # Create a transfer function editor
  # Assign our tfunc to the editor
  # Make sure we show the whole range of the tfunc

  set pfed_tfunc1_editor [vtkKWPiecewiseFunctionEditor New]
  $pfed_tfunc1_editor SetParent $parent
  $pfed_tfunc1_editor Create
  $pfed_tfunc1_editor SetBorderWidth 2
  $pfed_tfunc1_editor SetReliefToGroove
  $pfed_tfunc1_editor SetPadX 2
  $pfed_tfunc1_editor SetPadY 2
  $pfed_tfunc1_editor ExpandCanvasWidthOff
  $pfed_tfunc1_editor SetCanvasWidth 250
  $pfed_tfunc1_editor ParameterRangeVisibilityOff
  $pfed_tfunc1_editor ValueRangeVisibilityOff
  $pfed_tfunc1_editor LockEndPointsParameterOn
  $pfed_tfunc1_editor SetBalloonHelpString \
    "A transfer function editor. The parameter range slider is hidden and\
    end points of the function are locked."

  $pfed_tfunc1_editor SetPiecewiseFunction $pfed_tfunc1
  $pfed_tfunc1_editor SetWholeParameterRangeToFunctionRange
  $pfed_tfunc1_editor SetVisibleParameterRangeToWholeParameterRange

  pack [$pfed_tfunc1_editor GetWidgetName] -side top -anchor nw -expand n -padx 2 -pady 2

  # -----------------------------------------------------------------------

  # This other transfer function editor is based on a real image data
  # Let's load it first

  set pfed_reader [vtkXMLImageDataReader New]
  $pfed_reader SetFileName [file join [file dirname [info script]] ".." ".." Data "head100x100x47.vti"]

  # The build an histogram of the data it will be used inside the editor
  # as if we were trying to tune a tfunc based on the real values

  $pfed_reader Update
  set pfed_hist [vtkKWHistogram New]
  $pfed_hist BuildHistogram \
    [[[$pfed_reader GetOutput] GetPointData] GetScalars] 0

  set range [$pfed_hist GetRange] 

  # Create the transfer function that will be modified by the 2nd editor
  # This one shows a different look & feel

  set r0 [lindex $range 0]
  set r1 [lindex $range 1]

  set version [vtkVersion New]
  set vtk_major [$version GetVTKMajorVersion]
  set vtk_minor [$version GetVTKMinorVersion]

  set pfed_tfunc2 [vtkPiecewiseFunction New]
  if {$vtk_major > 5 || ($vtk_major == 5 && $vtk_minor > 0)} {
    $pfed_tfunc2 AddPoint $r0                       0.1 0.5 0.00
    $pfed_tfunc2 AddPoint [expr ($r0 + $r1) * 0.15] 0.9 0.5 0.25
    $pfed_tfunc2 AddPoint [expr ($r0 + $r1) * 0.30] 0.1 0.5 0.50
    $pfed_tfunc2 AddPoint [expr ($r0 + $r1) * 0.45] 0.9 0.5 0.75
    $pfed_tfunc2 AddPoint [expr ($r0 + $r1) * 0.60] 0.1 0.5 1.00
    $pfed_tfunc2 AddPoint [expr ($r0 + $r1) * 0.75] 0.9 0.2 0.00
  } else {
    $pfed_tfunc2 AddPoint $r0                       0.1
    $pfed_tfunc2 AddPoint [expr ($r0 + $r1) * 0.15] 0.9
    $pfed_tfunc2 AddPoint [expr ($r0 + $r1) * 0.30] 0.1
    $pfed_tfunc2 AddPoint [expr ($r0 + $r1) * 0.45] 0.9
    $pfed_tfunc2 AddPoint [expr ($r0 + $r1) * 0.60] 0.1
    $pfed_tfunc2 AddPoint [expr ($r0 + $r1) * 0.75] 0.9
  }
  
  $pfed_tfunc2 AddPoint $r1                       0.1

  # Create a transfer function editor
  # Assign our tfunc to the editor
  # Make sure we show the whole range of the tfunc
  # Use an histogram

  set pfed_tfunc2_editor [vtkKWPiecewiseFunctionEditor New]
  $pfed_tfunc2_editor SetParent $parent
  $pfed_tfunc2_editor Create
  $pfed_tfunc2_editor SetBorderWidth 2
  $pfed_tfunc2_editor SetReliefToGroove
  $pfed_tfunc2_editor SetPadX 2
  $pfed_tfunc2_editor SetPadY 2
  $pfed_tfunc2_editor ExpandCanvasWidthOff
  $pfed_tfunc2_editor SetCanvasWidth 450
  $pfed_tfunc2_editor SetCanvasHeight 150
  $pfed_tfunc2_editor SetLabelText "Transfer Function Editor"
  $pfed_tfunc2_editor SetBalloonHelpString \
    "Another transfer function editor. Guidelines are dispayed\
    for each midpoint, ticks are displayed in the\
    parameter space at the bottom, the width is set explicitly.\
    The range and histogram are based on a real image data."

  $pfed_tfunc2_editor SetPiecewiseFunction $pfed_tfunc2

  $pfed_tfunc2_editor PointIndexVisibilityOff
  $pfed_tfunc2_editor SelectedPointIndexVisibilityOn
  $pfed_tfunc2_editor PointGuidelineVisibilityOff
  $pfed_tfunc2_editor MidPointVisibilityOn
  $pfed_tfunc2_editor MidPointGuidelineVisibilityOn
  $pfed_tfunc2_editor MidPointGuidelineValueVisibilityOn
  $pfed_tfunc2_editor SetMidPointGuidelineValueFormat "%-#6.0f"
  $pfed_tfunc2_editor MidPointEntryVisibilityOn
  $pfed_tfunc2_editor SharpnessEntryVisibilityOn
  $pfed_tfunc2_editor SetLabelPositionToTop

  $pfed_tfunc2_editor SetHistogram $pfed_hist

  $pfed_tfunc2_editor ParameterTicksVisibilityOn
  $pfed_tfunc2_editor ComputeValueTicksFromHistogramOn
  $pfed_tfunc2_editor SetParameterTicksFormat \
      [$pfed_tfunc2_editor GetMidPointGuidelineValueFormat]

  pack [$pfed_tfunc2_editor GetWidgetName] -side top -anchor nw -expand n -padx 2 -pady 20
}

proc vtkKWPiecewiseFunctionEditorGetType {} {
  return "TypeVTK"
}
