proc vtkKWNotebookEntryPoint {parent win} {

  set app [$parent GetApplication] 

  # -----------------------------------------------------------------------

  # Create a notebook

  set notebook1 [vtkKWNotebook New]
  $notebook1 SetParent $parent
  $notebook1 SetMinimumWidth 400
  $notebook1 SetMinimumHeight 200
  $notebook1 Create

  pack [$notebook1 GetWidgetName] -side top -anchor nw -expand n -padx 2 -pady 2

  # Add some pages

  $notebook1 AddPage "Page 1"

  $notebook1 AddPage "Page Blue"
  [$notebook1 GetFrame "Page Blue"] SetBackgroundColor 0.2 0.2 0.9

  set page_id [$notebook1 AddPage "Page Red"] 
  [$notebook1 GetFrame $page_id] SetBackgroundColor 0.9 0.2 0.2

  # -----------------------------------------------------------------------

  # Create a notebook inside one of the page (because we can)

  set page_id [$notebook1 AddPage "Sub Notebook"] 

  set notebook2 [vtkKWNotebook New]
  $notebook2 SetParent [$notebook1 GetFrame $page_id]
  $notebook2 Create
  $notebook2 EnablePageTabContextMenuOn
  $notebook2 PagesCanBePinnedOn
  $notebook2 UseFrameWithScrollbarsOn

  # -----------------------------------------------------------------------

  # Create a message inside one of the page (as a test for scrollbars)

  set page_id [$notebook2 AddPage "Page A"]

  set lorem_ipsum "Lorem ipsum dolor sit amet consectetuer adipiscing elit. Nunc felis. Nulla gravida. Aliquam erat volutpat. Mauris accumsan quam non sem. Sed commodo magna quis bibendum lacinia elit turpis iaculis augue eget hendrerit elit dui vel elit.\n\nInteger ante eros auctor eu dapibus ac ultricies vitae lacus. Fusce accumsan mauris. Morbi felis. Class aptent taciti sociosqu ad litora torquent per conubia nostra per inceptos hymenaeos. Maecenas convallis imperdiet nunc."
  set message [vtkKWMessage New]
  $message SetParent [$notebook2 GetFrame $page_id]
  $message Create
  $message SetText "$lorem_ipsum"
  $message AppendText "$lorem_ipsum"
  pack [$message GetWidgetName] -side top -anchor nw -expand n -padx 2 -pady 2

  # -----------------------------------------------------------------------

  # Create a disabled page

  set page_id [$notebook2 AddPage "Page Disabled"]
  $notebook2 SetPageEnabled $page_id 0

  pack [$notebook2 GetWidgetName] -side top -anchor nw -expand y -fill both -padx 2 -pady 2

  # -----------------------------------------------------------------------

  # Create a button inside one of the page (as a test)

  set page_id [$notebook2 AddPage "Button Page"] 

  set pushbutton1 [vtkKWPushButton New]
  $pushbutton1 SetParent [$notebook2 GetFrame $page_id]
  $pushbutton1 Create
  $pushbutton1 SetText "A push button"

  pack [$pushbutton1 GetWidgetName] -side top -anchor c -expand y
}

proc vtkKWNotebookGetType {} {
  return "TypeComposite"
}
