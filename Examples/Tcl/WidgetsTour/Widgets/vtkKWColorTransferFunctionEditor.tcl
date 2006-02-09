proc vtkKWColorTransferFunctionEditorEntryPoint {parent win} {

  set app [$parent GetApplication] 

  # Create the color transfer function that is modified by the 1st editor

  set cpsel_tfunc1 [vtkColorTransferFunction New]
  $cpsel_tfunc1 SetColorSpaceToHSV
  $cpsel_tfunc1 AddHSVPoint 0.0 0.66 1.0 1.0
  $cpsel_tfunc1 AddHSVPoint 512.0 0.33 1.0 1.0
  $cpsel_tfunc1 AddHSVPoint 1024.0 0.0 1.0 1.0

  # Create a color transfer function editor
  # Assign our tfunc to the editor
  # Make sure we show the whole range of the tfunc

  set cpsel_tfunc1_editor [vtkKWColorTransferFunctionEditor New]
  $cpsel_tfunc1_editor SetParent $parent
  $cpsel_tfunc1_editor Create
  $cpsel_tfunc1_editor SetBorderWidth 2
  $cpsel_tfunc1_editor SetReliefToGroove
  $cpsel_tfunc1_editor SetPadX 2
  $cpsel_tfunc1_editor SetPadY 2
  $cpsel_tfunc1_editor ParameterRangeVisibilityOff
  $cpsel_tfunc1_editor SetCanvasHeight 30
  $cpsel_tfunc1_editor SetBalloonHelpString \
    "A color transfer function editor. Double-click on a point to pop-up\
    a color chooser. The parameter range slider is hidden in this one."

  $cpsel_tfunc1_editor SetColorTransferFunction $cpsel_tfunc1
  $cpsel_tfunc1_editor SetWholeParameterRangeToFunctionRange
  $cpsel_tfunc1_editor SetVisibleParameterRangeToWholeParameterRange

  pack [$cpsel_tfunc1_editor GetWidgetName] -side top -anchor nw -expand n -padx 2 -pady 2

  # -----------------------------------------------------------------------

  # This other transfer function editor is based on a real image data
  # Let's load it first

  set cpsel_reader [vtkXMLImageDataReader New]
  $cpsel_reader SetFileName [file join [file dirname [info script]] ".." ".." Data "head100x100x47.vti"]

  # The build an histogram of the data it will be used inside the editor
  # as if we were trying to tune a tfunc based on the real values

  $cpsel_reader Update
  set cpsel_hist [vtkKWHistogram New]
  $cpsel_hist BuildHistogram \
    [[[$cpsel_reader GetOutput] GetPointData] GetScalars] 0

  set range [$cpsel_hist GetRange] 

  # Create the color transfer function that will be modified by the 2nd editor
  # This one shows a different look & feel

  set cpsel_tfunc2 [vtkColorTransferFunction New]
  $cpsel_tfunc2 SetColorSpaceToHSV
  $cpsel_tfunc2 AddHSVPoint [lindex $range 0] 0.66 1.0 1.0
  $cpsel_tfunc2 AddHSVPoint [expr ([lindex $range 0] + [lindex $range 1]) * 0.5] 0.0 1.0 1.0
  $cpsel_tfunc2 AddHSVPoint [lindex $range 1] 0.33 1.0 1.0

  # Create a color transfer function editor
  # Assign our tfunc to the editor
  # Make sure we show the whole range of the tfunc
  # Use an histogram

  set cpsel_tfunc2_editor [vtkKWColorTransferFunctionEditor New]
  $cpsel_tfunc2_editor SetParent $parent
  $cpsel_tfunc2_editor Create
  $cpsel_tfunc2_editor SetBorderWidth 2
  $cpsel_tfunc2_editor SetReliefToGroove
  $cpsel_tfunc2_editor SetPadX 2
  $cpsel_tfunc2_editor SetPadY 2
  $cpsel_tfunc2_editor ExpandCanvasWidthOff
  $cpsel_tfunc2_editor SetCanvasWidth 450
  $cpsel_tfunc2_editor SetCanvasHeight 150
  $cpsel_tfunc2_editor SetLabelText "Transfer Function Editor"
  $cpsel_tfunc2_editor SetRangeLabelPositionToTop
  $cpsel_tfunc2_editor SetBalloonHelpString \
    "Another color transfer function editor. The point position is now on\
    top the point style is an arrow down guidelines are shown for each\
    point useful when combined with an histogram point indices are\
    hidden ticks are displayed in the parameter space the label\
    and the parameter range are on top its width is set explicitly.\
    The range and histogram are based on a real image data."

  $cpsel_tfunc2_editor SetColorTransferFunction $cpsel_tfunc2
  $cpsel_tfunc2_editor SetWholeParameterRangeToFunctionRange
  $cpsel_tfunc2_editor SetVisibleParameterRangeToWholeParameterRange

  $cpsel_tfunc2_editor SetPointPositionInValueRangeToTop
  $cpsel_tfunc2_editor SetPointStyleToCursorDown
  $cpsel_tfunc2_editor FunctionLineVisibilityOff
  $cpsel_tfunc2_editor PointGuidelineVisibilityOn
  $cpsel_tfunc2_editor SelectedPointIndexVisibilityOn
  $cpsel_tfunc2_editor MidPointEntryVisibilityOn
  $cpsel_tfunc2_editor SharpnessEntryVisibilityOn
  $cpsel_tfunc2_editor SetLabelPositionToTop

  $cpsel_tfunc2_editor SetHistogram $cpsel_hist

  $cpsel_tfunc2_editor ParameterTicksVisibilityOn
  $cpsel_tfunc2_editor ComputeValueTicksFromHistogramOn
  $cpsel_tfunc2_editor SetParameterTicksFormat "%-#6.0f"

  pack [$cpsel_tfunc2_editor GetWidgetName] -side top -anchor nw -expand n -padx 2 -pady 20
}

proc vtkKWColorTransferFunctionEditorGetType {} {
  return "TypeVTK"
}
