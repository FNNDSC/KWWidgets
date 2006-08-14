proc vtkKWColorPresetSelectorEntryPoint {parent win} {

  set app [$parent GetApplication] 

  # -----------------------------------------------------------------------

  # Create the color transfer function that will be modified by
  # our choice of preset

  set cpsel_func [vtkColorTransferFunction New]

  # Create a color transfer function editor to show how the
  # color transfer function is affected by our choices

  set cpsel_tfunc_editor [vtkKWColorTransferFunctionEditor New]
  $cpsel_tfunc_editor SetParent $parent
  $cpsel_tfunc_editor Create
  $cpsel_tfunc_editor SetBorderWidth 2
  $cpsel_tfunc_editor SetReliefToGroove
  $cpsel_tfunc_editor ExpandCanvasWidthOff
  $cpsel_tfunc_editor SetCanvasWidth 200
  $cpsel_tfunc_editor SetCanvasHeight 30
  $cpsel_tfunc_editor SetPadX 2
  $cpsel_tfunc_editor SetPadY 2
  $cpsel_tfunc_editor ParameterRangeVisibilityOff
  $cpsel_tfunc_editor ParameterEntryVisibilityOff
  $cpsel_tfunc_editor ParameterRangeLabelVisibilityOff
  $cpsel_tfunc_editor ColorSpaceOptionMenuVisibilityOff
  $cpsel_tfunc_editor ReadOnlyOn
  $cpsel_tfunc_editor SetColorTransferFunction $cpsel_func
  $cpsel_tfunc_editor SetBalloonHelpString \
    "A color transfer function editor to demonstrate how the color\
    transfer function is affected by our choice of preset"

  # -----------------------------------------------------------------------

  proc update_editor {editor name} {$editor Update}

  # Create a color preset selector

  set cpsel1 [vtkKWColorPresetSelector New]
  $cpsel1 SetParent $parent
  $cpsel1 Create
  $cpsel1 SetColorTransferFunction $cpsel_func
  $cpsel1 SetPresetSelectedCommand "" "update_editor $cpsel_tfunc_editor"
  $cpsel1 SetBalloonHelpString \
    "A set of predefined color presets. Select one of them to apply the\
    preset to a color transfer function vtkColorTransferFunction"

  pack [$cpsel1 GetWidgetName] -side top -anchor nw -expand n -padx 2 -pady 2

  # -----------------------------------------------------------------------

  # Create another color preset selector only the solid color

  set cpsel2 [vtkKWColorPresetSelector New]
  $cpsel2 SetParent $parent
  $cpsel2 Create
  $cpsel2 SetLabelText "Solid Color Presets:"
  $cpsel2 GradientPresetsVisibilityOff
  $cpsel2 SetColorTransferFunction $cpsel_func
  $cpsel2 SetPresetSelectedCommand "" "update_editor $cpsel_tfunc_editor"
  $cpsel2 SetBalloonHelpString \
    "A set of predefined color presets. Let's hide the gradient presets."

  pack [$cpsel2 GetWidgetName] -side top -anchor nw -expand n -padx 2 -pady 2

  # -----------------------------------------------------------------------

  # Create another color preset selector custom colors

  set cpsel3 [vtkKWColorPresetSelector New]
  $cpsel3 SetParent $parent
  $cpsel3 Create
  $cpsel3 SetLabelPositionToRight
  $cpsel3 SetColorTransferFunction $cpsel_func
  $cpsel3 SetPresetSelectedCommand "" "update_editor $cpsel_tfunc_editor"
  $cpsel3 RemoveAllPresets
  $cpsel3 SetPreviewSize [expr [$cpsel3 GetPreviewSize]  * 2]
  $cpsel3 SetLabelText "Custom Color Presets"
  $cpsel3 AddSolidRGBPreset "Gray" 0.3 0.3 0.3
  $cpsel3 AddSolidHSVPreset "Yellow" 0.15 1.0 1.0
  $cpsel3 AddGradientRGBPreset \
    "Blue to Red" 0.0 0.0 1.0 1.0 0.0 0.0
  $cpsel3 AddGradientHSVPreset \
    "Yellow to Magenta" 0.15 1.0 1.0 0.83 1.0 1.0
  $cpsel3 SetBalloonHelpString \
    "A set of color presets. Just ours. Double the size of the preview and\
    put the label on the right."

  pack [$cpsel3 GetWidgetName] -side top -anchor nw -expand n -padx 2 -pady 2

  pack [$cpsel_tfunc_editor GetWidgetName] -side top -anchor nw -expand n -padx 2 -pady 20
}

proc vtkKWColorPresetSelectorGetType {} {
  return "TypeVTK"
}
