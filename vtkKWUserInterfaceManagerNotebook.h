/*=========================================================================

  Module:    $RCSfile: vtkKWUserInterfaceManagerNotebook.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWUserInterfaceManagerNotebook - a user interface manager.
// .SECTION Description
// This class is used to abstract the way a set of interface "panels" 
// (vtkKWUserInterfacePanel) can be grouped inside a widget. As such, it is a 
// concrete implementation of a vtkKWUserInterfaceManager. It uses a notebook
// under the hood and delivers a notebook's page when one of its managed panels
// request a "page" (i.e. a section within a panel). Within the notebook, each
// page will be associated to a tag corresponding to its panel's ID. This 
// allows panels to be shown once at a time, or grouped, or displayed using 
// more advanced combination (like most recently used pages among all panels, 
// pinned pages, etc.).
// This class is not a widget, the notebook is. Besides packing the notebook, 
// you will just have to set each panel's UserInterfaceManager ivar to point
// to this manager, and the rest should be taken care of (i.e. you do not 
// need to manually add a panel to a manager, or manually request a page from
// the manager, it should be done through the panel's API).
// .SECTION See Also
// vtkKWUserInterfaceManager vtkKWUserInterfacePanel

#ifndef __vtkKWUserInterfaceManagerNotebook_h
#define __vtkKWUserInterfaceManagerNotebook_h

#include "vtkKWUserInterfaceManager.h"

class vtkKWIcon;
class vtkKWNotebook;
class vtkKWUserInterfacePanel;
class vtkKWWidget;
class vtkKWUserInterfaceManagerNotebookInternals;

class KWWidgets_EXPORT vtkKWUserInterfaceManagerNotebook : public vtkKWUserInterfaceManager
{
public:
  static vtkKWUserInterfaceManagerNotebook* New();
  vtkTypeRevisionMacro(vtkKWUserInterfaceManagerNotebook,vtkKWUserInterfaceManager);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the user interface manager's notebook. This has to be done before
  // Create() is called (i.e. the sooner, the better), and can be done only
  // once.
  virtual void SetNotebook(vtkKWNotebook*);
  vtkGetObjectMacro(Notebook, vtkKWNotebook);
  
  // Description:
  // Get the application instance for this object.
  // Override the superclass to try to retrieve the notebook's application
  // if it was not set already.
  virtual vtkKWApplication* GetApplication();

  // Description:
  // Create the manager widget (i.e. the widget that will group and display
  // all user interface panels). A notebook must be associated to the manager
  // before it is created.
  virtual void Create();

  // Description:
  // Instruct the manager to reserve or remove a page for a given panel.
  // In this concrete implementation, this adds or removes a page to the 
  // notebook, and sets the page tag to be the panel's ID.
  // Note that you should use the panel's own API to add a page to a panel: 
  // this will automatically call this method with the proper panel parameter 
  // (see vtkKWUserInterfacePanel::AddPage() and 
  // vtkKWUserInterfacePanel::RemovePage()).
  // Return a unique positive ID for the page that was reserved/removed,
  // or < 0 on error.
  virtual int AddPage(vtkKWUserInterfacePanel *panel, 
                      const char *title, 
                      const char *balloon = 0, 
                      vtkKWIcon *icon = 0);
  virtual int RemovePage(vtkKWUserInterfacePanel *panel, 
                         const char *title);

  // Description:
  // Retrieve the widget corresponding to a given page reserved by the manager.
  // This can be done through the unique page ID, or using a panel and the
  // page title. The user UI components should be inserted into this widget.
  // In this concrete implementation, this returns the inner frame of a
  // notebook's page.
  // Note that you should use the panel's own API to get a page widget: this
  // will automatically call this method with the proper ID or panel parameter
  // (see vtkKWUserInterfacePanel::GetPageWidget()).
  // Return NULL on error.
  virtual vtkKWWidget* GetPageWidget(int id);
  virtual vtkKWWidget* GetPageWidget(vtkKWUserInterfacePanel *panel, 
                                     const char *title);

  // Description:
  // Retrieve the parent widget of the pages associated to a panel. It is
  // the unique widget that is common to all pages in the chain of parents.
  // Note that you should use the panel's own API to get the page parent: this
  // will automatically call this method with the proper panel parameter
  // (see vtkKWUserInterfacePanel::GetPagesParentWidget()).
  virtual vtkKWWidget *GetPagesParentWidget(vtkKWUserInterfacePanel *panel);

  // Description:
  // Raise a page reserved by the manager. This can be done through the unique 
  // page ID, or using a panel and the page title.
  // In this concrete implementation, this raises a notebook's page.
  // Note that you should use the panel's own API to raise a page: this
  // will automatically call this method with the proper ID or panel parameter
  // (see vtkKWUserInterfacePanel::RaisePage()).
  // Note that if the panel corresponding to the page to raise has not been
  // created yet, it will be created automatically by calling the panel's 
  // Create() method (see vtkKWUserInterfacePanel::Create()) ; this allows the
  // creation of the panel to be delayed until it is really needed.
  virtual void RaisePage(int id);
  virtual void RaisePage(vtkKWUserInterfacePanel *panel, 
                         const char *title);
  
  // Description:
  // Show/Hide a panel. It will make sure the pages reserved by the manager
  // for this panel are shown/hidden.
  // In this concrete implementation, this shows/hides all notebook's pages
  // belonging to this panel.
  // RaisePanel() behaves like ShowPanel(), but it will also try to bring
  // up the first page of the panel to the front (i.e., "select" it).
  // IsPanelVisible() checks if the pages of the panel are visible/shown.
  // Note that you should use the panel's own API to show a panel: this
  // will automatically call this method with the proper panel parameter
  // (see vtkKWUserInterfacePanel::Show()).
  // Note that if the panel has not been created yet, it will be created 
  // automatically by calling the panel's Create() method (see 
  // vtkKWUserInterfacePanel::Create()) ; this allows the creation of the 
  // panel to be delayed until it is really needed.
  // Return 1 on success, 0 on error.
  virtual int ShowPanel(vtkKWUserInterfacePanel *panel);
  virtual int HidePanel(vtkKWUserInterfacePanel *panel);
  virtual int IsPanelVisible(vtkKWUserInterfacePanel *panel);
  virtual int RaisePanel(vtkKWUserInterfacePanel *panel);
  
  // Description:
  // Update a panel according to the manager settings (i.e., it just performs 
  // manager-specific changes on the panel). Note that it does not call the
  // panel's Update() method, on the opposite the panel's Update() will call 
  // this method if the panel has a UIM set.
  virtual void UpdatePanel(vtkKWUserInterfacePanel *panel);

  // Description:
  // Get the panel from a page ID (return the ID of the panel that holds
  // that page).
  virtual vtkKWUserInterfacePanel* GetPanelFromPageId(int page_id);

  // Description:
  // Enable/disable Drag and Drop. If enabled, elements of the user interface
  // can be drag&drop within the same panel, or between different panels.
  virtual void SetEnableDragAndDrop(int);
  vtkBooleanMacro(EnableDragAndDrop, int);
  vtkGetMacro(EnableDragAndDrop, int);

  // Description:
  // Get the number of Drag&Drop entries so far.
  // Delete all Drag&Drop entries.
  virtual int GetNumberOfDragAndDropEntries();
  virtual int DeleteAllDragAndDropEntries();

  // Description:
  // Save/restore Drag&Drop entries to a text file. 
  // GetDragAndDropEntry() can be used to get a Drag&Drop entry parameters 
  // as plain text string. 
  // DragAndDropWidget() will perform a Drag&Drop given parameters similar
  // to those acquired through GetDragAndDropEntry().
  virtual int GetDragAndDropEntry(
    int idx, 
    ostream &widget_label, 
    ostream &from_panel_name, 
    ostream &from_page_title, 
    ostream &from_after_widget_label, 
    ostream &to_panel_name, 
    ostream &to_page_title,
    ostream &to_after_widget_label);
  virtual int DragAndDropWidget(
    const char *widget_label, 
    const char *from_panel_name, 
    const char *from_page_title, 
    const char *from_after_widget_label,
    const char *to_panel_name, 
    const char *to_page_title,
    const char *to_after_widget_label);

  // Description:
  // Lock Drag and Drop entries. If enabled, GetDragAndDropEntry() will
  // not return any entry, and DragAndDropWidget() will not set any.
  vtkBooleanMacro(LockDragAndDropEntries, int);
  vtkSetMacro(LockDragAndDropEntries, int);
  vtkGetMacro(LockDragAndDropEntries, int);

  //BTX
  // A Widget location. 
  // Store both the page the widget is packed in, and the widget it is 
  // packed after (if any).
  // @cond nested_class
  class WidgetLocation
  {
  public:
    WidgetLocation();
    void Empty();

    int PageId;
    vtkKWWidget *AfterWidget;
  };
  // @endcond

  // A D&D entry. 
  // Store the widget source and target location.
  // @cond nested_class
  class DragAndDropEntry
  {
  public:
    DragAndDropEntry();

    vtkKWWidget *Widget;
    WidgetLocation FromLocation;
    WidgetLocation ToLocation;
  };
  // @endcond
  //ETX

  // Description:
  // Callbacks. Internal, do not use.
  virtual void DragAndDropEndCallback(
    int x, int y, 
    vtkKWWidget *widget, vtkKWWidget *anchor, vtkKWWidget *target);

protected:
  vtkKWUserInterfaceManagerNotebook();
  ~vtkKWUserInterfaceManagerNotebook();

  // Description:
  // Remove the widgets of all pages belonging to a panel. It is called
  // by RemovePanel().
  // In this concrete implementation, this will remove all notebook's pages 
  // belonging to this panel.
  // Return 1 on success, 0 on error.
  virtual int RemovePageWidgets(vtkKWUserInterfacePanel *panel);
  
  vtkKWNotebook *Notebook;

  // Description:
  // Update Drag And Drop bindings
  virtual void UpdatePanelDragAndDrop(vtkKWUserInterfacePanel *panel);
  int EnableDragAndDrop;
  int LockDragAndDropEntries;

  //BTX

  // Description:
  // Check if a given widget can be Drag&Dropped given our framework.
  // At the moment, only labeled frame can be D&D. If **anchor is not NULL,
  // it will be assigned the widget D&D anchor (i.e. the internal part of
  // the widget that is actually used to grab the widget).
  // Return 1 if can be D&D, 0 otherwise.
  virtual int CanWidgetBeDragAndDropped(
    vtkKWWidget *widget, vtkKWWidget **anchor = 0);

  // Description:
  // Assuming that the widget can be Drag&Dropped given our framework, 
  // return a label that will be used to identify it. This is mostly used to
  // save a D&D event to a text string/file.
  virtual char* GetDragAndDropWidgetLabel(vtkKWWidget *widget);

  // PIMPL Encapsulation for STL containers

  vtkKWUserInterfaceManagerNotebookInternals *Internals;
  friend class vtkKWUserInterfaceManagerNotebookInternals;

  // Description:
  // Get the location of a widget.
  virtual int GetDragAndDropWidgetLocation(
    vtkKWWidget *widget, WidgetLocation *loc);

  // Description:
  // Get a D&D widget given its label (as returned by 
  // GetDragAndDropWidgetLabel()) and a hint about its location.
  virtual vtkKWWidget* GetDragAndDropWidgetFromLabelAndLocation(
    const char *widget_label, const WidgetLocation *loc_hint);

  // Description:
  // Get the last D&D entry that was added for a given widget
  DragAndDropEntry* GetLastDragAndDropEntry(vtkKWWidget *Widget);

  // Description:
  // Check if a widget that was drag & drop has gone back to its previous
  // location
  virtual int IsDragAndDropWidgetAtOriginalLocation(vtkKWWidget *widget);

  // Description:
  // Add a D&D entry to the list of entries, given a widget and its
  // target location (its current/source location will be computed 
  // automatically)
  int AddDragAndDropEntry(
    vtkKWWidget *Widget, 
    const WidgetLocation *from_loc,
    const WidgetLocation *to_loc);

  // Description:
  // Perform the actual D&D given a widget and its target location.
  // It will call AddDragAndDropEntry() and pack the widget to its new
  // location
  virtual int DragAndDropWidget(
    vtkKWWidget *widget, 
    const WidgetLocation *from_loc,
    const WidgetLocation *to_loc);

  //ETX

private:

  vtkKWUserInterfaceManagerNotebook(const vtkKWUserInterfaceManagerNotebook&); // Not implemented
  void operator=(const vtkKWUserInterfaceManagerNotebook&); // Not Implemented
};

#endif

