proc vtkKWListBoxEntryPoint {parent win} {

  set app [$parent GetApplication] 

  set days {"Monday" "Tuesday" "Wednesday" "Thursday" "Friday"}

  # -----------------------------------------------------------------------

  # Create a listbox
  # Add some entries

  set listbox1 [vtkKWListBox New]
  $listbox1 SetParent $parent
  $listbox1 Create
  $listbox1 SetSelectionModeToSingle
  $listbox1 SetBalloonHelpString "A simple listbox"

  for {set i 0} {$i < [llength $days]} {incr i} {

    $listbox1 AppendUnique [lindex $days $i]
  }

  pack [$listbox1 GetWidgetName] -side top -anchor nw -expand n -padx 2 -pady 2

  # -----------------------------------------------------------------------

  # Create another listbox this time with scrollbars and multiple choices

  set listbox2 [vtkKWListBoxWithScrollbars New]
  $listbox2 SetParent $parent
  $listbox2 Create
  $listbox2 SetBorderWidth 2
  $listbox2 SetReliefToGroove
  $listbox2 SetPadX 2
  $listbox2 SetPadY 2
  [$listbox2 GetWidget] SetSelectionModeToMultiple 
  [$listbox2 GetWidget] ExportSelectionOff 
  $listbox2 SetBalloonHelpString \
    "A list box with scrollbars. Multiple entries can be selected"

  for {set i 0} {$i < 15} {incr i} {

    [$listbox2 GetWidget] AppendUnique "Entry $i"
  }

  pack [$listbox2 GetWidgetName] -side top -anchor nw -expand n -padx 2 -pady 6

  # TODO: use callbacks
}

proc vtkKWListBoxGetType {} {
  return "TypeCore"
}
