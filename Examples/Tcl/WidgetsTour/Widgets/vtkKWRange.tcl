proc vtkKWRangeEntryPoint {parent win} {

  set app [$parent GetApplication] 

  # -----------------------------------------------------------------------

  # Create a range

  set range1 [vtkKWRange New]
  $range1 SetParent $parent
  $range1 Create
  $range1 SetLabelText "A range:"
  $range1 SetWholeRange 0.0 100.0
  $range1 SetRange 20.0 60.0
  $range1 SetResolution 0.1
  $range1 SetReliefToGroove
  $range1 SetBorderWidth 2
  $range1 SetPadX 2
  $range1 SetPadY 2
  $range1 SetBalloonHelpString \
    "A range widget i.e. a pair of values the range within a larger pair\
    of values the *whole* range."

  pack [$range1 GetWidgetName] -side top -anchor nw -expand n -padx 2 -pady 2

  # -----------------------------------------------------------------------

  # Create another range but put the label and entry on top

  set range2 [vtkKWRange New]
  $range2 SetParent $parent
  $range2 Create
  $range2 SetLabelText "Another range:"
  $range2 SymmetricalInteractionOn
  eval $range2 SetWholeRange [$range1 GetWholeRange] 
  eval $range2 SetRange [$range1 GetRange] 
  $range2 SetResolution 2.0
  $range2 SetLabelPositionToLeft
  $range2 SetEntry1PositionToLeft
  $range2 SetEntry2PositionToRight
  $range2 SetSliderSize 4
  $range2 SetThickness 23
  $range2 SetInternalThickness 0.7
  $range2 SetRequestedLength 200
  $range2 SetBalloonHelpString \
    "Another range widget the label and entries are in different positions,\
    the slider and the thickness of the widget has changed and we set a\
    longer minimum length. The range is set to symmetrical mode, where both\
    sliders at each end are modified symmetrically. Also note that changing\
    this range sets the value of the first range."

  pack [$range2 GetWidgetName] -side top -anchor nw -expand n -padx 2 -pady 6

  $range2 SetCommand $range1 "SetRange"

  # -----------------------------------------------------------------------

  # Create another range

  set range3 [vtkKWRange New]
  $range3 SetLabelText "Another range, custom colors"
  $range3 SetParent $parent
  $range3 Create
  $range3 SetLabelPositionToRight
  $range3 SetEntry1PositionToLeft
  $range3 SetEntry2PositionToLeft
  $range3 SetRequestedLength 150
  $range3 SliderCanPushOn
  $range3 SetRangeColor 0.0  0.0  1.0
  $range3 SetRangeInteractionColor 0.6  0.7  0.4
  $range3 SetSlider1Color 1.0  0.0  0.0
  $range3 SetSlider2Color 0.0  1.0  0.0
  $range3 SetBalloonHelpString \
    "Another range widget. We changed the positions again. The sliders can\
    push each others. The interaction color has been changed."

  pack [$range3 GetWidgetName] -side top -anchor nw -expand n -padx 2 -pady 6

  # TODO: vertical range
}

proc vtkKWRangeGetType {} {
  return "TypeComposite"
}
