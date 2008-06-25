proc vtkKWTreeEntryPoint {parent win} {

  set app [$parent GetApplication] 

  # -----------------------------------------------------------------------

  # Create a tree

  set tree1 [vtkKWTreeWithScrollbars New]
  $tree1 SetParent $parent
  $tree1 Create
  $tree1 SetBalloonHelpString "A simple tree"
  $tree1 SetBorderWidth 2
  $tree1 SetReliefToGroove
  $tree1 ResizeButtonsVisibilityOn

  set tree [$tree1 GetWidget]
  $tree SelectionFillOn
  $tree EnableReparentingOn

  $tree AddNode "" "inbox_node" "Inbox"

  $tree AddNode "" "outbox_node" "Outbox"
  
  $tree AddNode "" "kitware_node" "Kitware"
  $tree SetNodeFontWeightToBold "kitware_node"
  $tree SetNodeSelectableFlag "kitware_node" 0
  $tree OpenTree "kitware_node"

  $tree AddNode "kitware_node" "berk_node" "Berk Geveci"
  
  $tree AddNode "kitware_node" "seb_node" "Sebastien Barre"
  
  $tree AddNode "kitware_node" "ken_node" "Ken Martin"
  
  pack [$tree1 GetWidgetName] -side top -anchor nw -expand n -padx 2 -pady 2
}

proc vtkKWTreeGetType {} {
  return "TypeCore"
}
