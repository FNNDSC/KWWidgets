/*=========================================================================

  Module:    $RCSfile: vtkKWNotebook.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWNotebook - a tabbed notebook of UI pages
// .SECTION Description
// The notebook represents a tabbed notebook component where you can
// add or remove pages.

#ifndef __vtkKWNotebook_h
#define __vtkKWNotebook_h

#include "vtkKWCompositeWidget.h"

class vtkKWFrame;
class vtkKWIcon;
class vtkKWLabel;
class vtkKWMenu;
class vtkKWNotebookInternals;

class KWWidgets_EXPORT vtkKWNotebook : public vtkKWCompositeWidget
{
public:
  static vtkKWNotebook* New();
  vtkTypeRevisionMacro(vtkKWNotebook,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Add a new page to the notebook. By setting balloon string, the page will
  // display a balloon help. An optional icon can also be specified and will 
  // be displayed on the left side of the tab label (all icons can be hidden
  // later on using the SetShowIcons() method). Finally, an optional tag
  // can be provided and will be associated to the page (see SetPageTag()) ; 
  // this/ tag will default to 0 otherwise.
  // Return a unique positive ID corresponding to that page, or < 0 on error.
  int AddPage(const char *title, const char* balloon, vtkKWIcon *icon,int tag);
  int AddPage(const char *title, const char* balloon, vtkKWIcon *icon);
  int AddPage(const char *title, const char* balloon);
  int AddPage(const char *title);

  // Description:
  // Does it have a given page
  int HasPage(int id);
  int HasPage(const char *title, int tag);
  int GetPageId(const char *title, int tag);

  // Description:
  // Accessors
  const char* GetPageTitle(int id);

  // Description:
  // Return the number of pages in the notebook.
  unsigned int GetNumberOfPages();
  unsigned int GetNumberOfPagesMatchingTag(int tag);
  
  // Description:
  // Set/Get a page tag. A tag (int) can be associated to a page (given the
  // page id). This provides a way to group pages. The default tag, if not 
  // provided, is 0. 
  // If a page title is provided instead of a page id, the first page matching
  // that title is considered.
  void SetPageTag(int id, int tag);
  void SetPageTag(const char *title, int tag);
  int GetPageTag(int id);
  int GetPageTag(const char *title);

  // Description:
  // Raise the specified page to be on the top (i.e. the one selected).
  // If a page title is provided instead of a page id, the first page matching
  // that title is considered. In the same way, if a tag is provided with the 
  // title, the page which match both title *and* tag is considered.
  // GetRaisedPageId() returns the id of the page raised at the moment, -1 if
  // none is raised.
  // RaiseFirstPageMatchingTag() raises the first page matching a given tag.
  void RaisePage(int id);
  void RaisePage(const char *title);
  void RaisePage(const char *title, int tag);
  int GetRaisedPageId();
  void RaiseFirstPageMatchingTag(int tag);
  
  // Description:
  // Get the vtkKWWidget corresponding to the frame of a specified page (Tab).
  // This is where the UI components should be inserted.
  // If a page title is provided instead of a page id, the first page matching
  // that title is considered. In the same way, if a tag is provided with the 
  // title, the page which match both title *and* tag is considered.
  // Return NULL on error.
  vtkKWFrame *GetFrame(int id);
  vtkKWFrame *GetFrame(const char *title);
  vtkKWFrame *GetFrame(const char *title, int tag);
  int GetPageIdFromFrameWidgetName(const char *frame_wname);

  // Description:
  // Remove a page from the notebook.
  // If a page title is provided instead of a page id, the first page matching
  // that title is considered.
  // If the currently selected page is removed, it is unselected first and
  // the first visible tab (if any) becomes selected instead.
  // Return 1 on success, 0 on error.
  int RemovePage(int id);
  int RemovePage(const char *title);

  // Description:
  // Remove all pages matching a tag.
  void RemovePagesMatchingTag(int tag);
  
  // Description:
  // Show/hide a page tab (i.e. Set/Get the page visibility). Showing a page 
  // tab does not raise the page, it just makes the page selectable by 
  // displaying its tab. A hidden page tab is not displayed in the tabs: the 
  // corresponding page can not be selected. 
  // If a page title is provided instead of a page id, the first page matching
  // that title is considered. In the same way, if a tag is provided with the 
  // title, the page which match both title *and* tag is considered.
  // If the currently selected page is hidden, it is unselected first and
  // the first visible tab (if any) becomes selected instead.
  // A pinned page tab can not be hidden (see PinPage()/UnpinPage()).
  void ShowPage(int id);
  void ShowPage(const char *title);
  void ShowPage(const char *title, int tag);
  void HidePage(int id);
  void HidePage(const char *title);
  void HidePage(const char *title, int tag);
  void SetPageVisibility(int id, int flag);
  void SetPageVisibility(const char *title, int flag);
  void SetPageVisibility(const char *title, int tag, int flag);
  int  GetPageVisibility(int id);
  int  GetPageVisibility(const char *title);
  int  GetPageVisibility(const char *title, int tag);
  void TogglePageVisibility(int id);
  void TogglePageVisibility(const char *title);
  void TogglePageVisibility(const char *title, int tag);
  int  CanBeHidden(int id);
  int  CanBeHidden(const char *title);
  int  CanBeHidden(const char *title, int tag);
  void HideAllPages();

  // Description:
  // Enable/Disable a page. Note that this differs from show/hide a tab in the
  // sense that the tab will still be visible but disabled. All tabs are 
  // enabled by default. Calling SetEnabled(flag) on this notebook will
  // set the state of all pages to 'flag' unless the page itself has been
  // disabled. The only way to re-enable a page that was disabled using
  // SetPageEnabled is to call SetPageEnabled again.
  void SetPageEnabled(int id, int flag);
  void SetPageEnabled(const char *title, int flag);
  void SetPageEnabled(const char *title, int tag, int fkag);
  
  // Description:
  // Return the number of visible pages in the notebook.
  unsigned int GetNumberOfVisiblePages();
  unsigned int GetNumberOfVisiblePagesMatchingTag(int tag);

  // Description:
  // Get the n-th visible page id (starting at index 0, i.e. the first visible 
  // page is at index 0, although it does not necessary reflects the way
  // the page tab are packed/ordered in the tab row).
  // Return -1 if the index is out of the range, or if there is no visible
  // page for that index.
  int GetVisiblePageId(int idx);
  
  // Description:
  // Show/Hide all page tabs matching or not matching a given tag. 
  // ShowPagesMatchingTagReverse processes pages starting from the last one.
  void HidePagesMatchingTag(int tag);
  void ShowPagesMatchingTag(int tag);
  void ShowPagesMatchingTagReverse(int tag);
  void HidePagesNotMatchingTag(int tag);
  void ShowPagesNotMatchingTag(int tag);

  // Description:
  // Make the notebook automatically bring up page tabs with the same tag 
  // (i.e. all page tabs that have the same tag are always shown).
  virtual void SetShowAllPagesWithSameTag(int);
  vtkGetMacro(ShowAllPagesWithSameTag, int);
  vtkBooleanMacro(ShowAllPagesWithSameTag, int);
  
  // Description:
  // Make the notebook automatically show only those page tabs that have the 
  // same tag as the currently selected page (i.e. once a page tab has been
  // selected, all pages not sharing the same tag are hidden).
  virtual void SetShowOnlyPagesWithSameTag(int);
  vtkGetMacro(ShowOnlyPagesWithSameTag, int);
  vtkBooleanMacro(ShowOnlyPagesWithSameTag, int);
  
  // Description:
  // Make the notebook automatically maintain a list of most recently used
  // page. The size of this list can be set (defaults to 4). Once it is full,
  // any new shown page will make the least recent page hidden.
  // It is suggested that ShowAllPagesWithSameTag and ShowOnlyPagesWithSameTag
  // shoud be Off for this feature to work properly.
  virtual void SetShowOnlyMostRecentPages(int);
  vtkGetMacro(ShowOnlyMostRecentPages, int);
  vtkBooleanMacro(ShowOnlyMostRecentPages, int);
  vtkSetMacro(NumberOfMostRecentPages, int);
  vtkGetMacro(NumberOfMostRecentPages, int);

  // Description:
  // Get the n-th most recent page id. Most recent pages indexes start at 0 
  // (i.e. the most recent page is at index 0).
  // Return -1 if the index is out of the range, or if there is no most
  // recent page for that index.
  int GetMostRecentPageId(int idx);

  // Description:
  // Pin/unpin a page tab. A pinned page tab can not be hidden.
  // If a page title is provided instead of a page id, the first page matching
  // that title is considered. In the same way, if a tag is provided with the 
  // title, the page which match both title *and* tag is considered.
  void PinPage(int id);
  void PinPage(const char *title);
  void PinPage(const char *title, int tag);
  void UnpinPage(int id);
  void UnpinPage(const char *title);
  void UnpinPage(const char *title, int tag);
  void TogglePagePinned(int id);
  void TogglePagePinned(const char *title);
  void TogglePagePinned(const char *title, int tag);
  int  GetPagePinned(int id);
  int  GetPagePinned(const char *title);
  int  GetPagePinned(const char *title, int tag);
  
  // Description:
  // Pin/Unpin all page tabs matching a given tag.
  void PinPagesMatchingTag(int tag);
  void UnpinPagesMatchingTag(int tag);

  // Description:
  // Allow pages to be pinned.
  virtual void SetPagesCanBePinned(int);
  vtkGetMacro(PagesCanBePinned, int);
  vtkBooleanMacro(PagesCanBePinned, int);

  // Description:
  // Return the number of pinned pages in the notebook.
  unsigned int GetNumberOfPinnedPages();

  // Description:
  // Get the n-th pinned page id (starting at index 0, i.e. the first pinned 
  // page is at index 0).
  // Return -1 if the index is out of the range, or if there is no pinned
  // page for that index.
  int GetPinnedPageId(int idx);
  
  // Description:
  // By default, pages are vtkKWFrame. If the user needs scrollbars, he can
  // make the page the parent of a vtkKWFrameWithScrollbar. For convenience,
  // this option can be turned on so that pages that are added from now on are
  // automatically put inside a vtkKWFrameWithScrollbar.
  // Note that HorizontalScrollbarVisibility is set to Off by default on
  // the vtkKWFrameWithScrollbar created by this class. 
  vtkSetMacro(UseFrameWithScrollbars, int);
  vtkGetMacro(UseFrameWithScrollbars, int);
  vtkBooleanMacro(UseFrameWithScrollbars, int);
  
  // Description:
  // The notebook will automatically resize itself to fit its
  // contents. This can lead to a lot of resizing. So you can
  // specify a minimum width and height for the notebook. This
  // can be used to significantly reduce or eliminate the resizing
  // of the notebook.
  virtual void SetMinimumWidth(int);
  vtkGetMacro(MinimumWidth,int);
  virtual void SetMinimumHeight(int);
  vtkGetMacro(MinimumHeight,int);

  // Description:
  // Normally, the tab frame is not shown when there is only
  // one page. Turn this on to override that behaviour.
  virtual void SetAlwaysShowTabs(int);
  vtkGetMacro(AlwaysShowTabs, int);
  vtkBooleanMacro(AlwaysShowTabs, int);
  
  // Description:
  // Show/hide all tab icons (if any).
  virtual void SetShowIcons(int);
  vtkGetMacro(ShowIcons, int);
  vtkBooleanMacro(ShowIcons, int);
  
  // Description:
  // Enable the page tab context menu.
  vtkSetMacro(EnablePageTabContextMenu, int);
  vtkGetMacro(EnablePageTabContextMenu, int);
  vtkBooleanMacro(EnablePageTabContextMenu, int);
  
  // Description:
  // Set/Get the background color of the widget.
  // Override the super to make sure all elements are set correctly.
  virtual void SetBackgroundColor(double r, double g, double b);
  virtual void SetBackgroundColor(double rgb[3])
    { this->SetBackgroundColor(rgb[0], rgb[1], rgb[2]); };

  // Description:
  // Set/Get the color of the page tabs, the outline of the pinned page tabs
  // and the selected page tab. 
  // Note that PageTabColor, and SelectedPageTabColor are
  // undefined by default (i.e. one of the RGB component is < 0), meaning that
  // the background color of the native window system will be used in place of
  // SelectedTabColor, and a slightly darker shade of that background color in
  // place of TabColor. 
  vtkGetVector3Macro(PageTabColor,double);
  virtual void SetPageTabColor(double r, double g, double b);
  virtual void SetPageTabColor(double rgb[3]) 
    { this->SetPageTabColor(rgb[0], rgb[1], rgb[2]); };
  vtkGetVector3Macro(SelectedPageTabColor,double);
  virtual void SetSelectedPageTabColor(double r, double g, double b);
  virtual void SetSelectedPageTabColor(double rgb[3]) 
    { this->SetSelectedPageTabColor(rgb[0], rgb[1], rgb[2]); };
  vtkGetVector3Macro(PinnedPageTabOutlineColor,double);
  virtual void SetPinnedPageTabOutlineColor(double r, double g, double b);
  virtual void SetPinnedPageTabOutlineColor(double rgb[3]) 
    { this->SetPinnedPageTabOutlineColor(rgb[0], rgb[1], rgb[2]); };

  // Description:
  // Set/Get the amount of padding that is to be added to the internal
  // vertical padding of the selected tab (basically defines the additional
  // height of the selected tab compared to an unselected tabs).
  vtkGetMacro(SelectedPageTabPadding, int);
  virtual void SetSelectedPageTabPadding(int arg);

  // Description:
  // Get the id of the visible page which tab contains a given pair of screen
  // coordinates (-1 if not found).
  virtual int GetPageIdContainingCoordinatesInTab(int x, int y);

  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object is updated
  // and propagated to its internal parts/subwidgets. This will, for example,
  // enable/disable parts of the widget UI, enable/disable the visibility
  // of 3D widgets, etc.
  virtual void UpdateEnableState();

  // Description:
  // Schedule the widget to resize itself, or resize it right away
  virtual void ScheduleResize();
  virtual void Resize();

  // Description:
  // Events: the following events are send by a notebook instance.
  // The below events are sent as a result of a direct user interaction
  // (clicking, context menu), not as a result of a programmatical call.
  // The following parameters are also passed as client data:
  // - the title/tag pair for the page, as a const char *[2] (first item
  //   in the array is title (eventually NULL), second is its numerical tag, 
  //   as a string)
  // vtkKWEvent::NotebookRaisePageEvent: a page has been raised
  // vtkKWEvent::NotebookPinPageEvent:   a page has been pinned (context menu)
  // vtkKWEvent::NotebookUnpinPageEvent: a page has been unpinned (context menu)
  // vtkKWEvent::NotebookShowPageEvent:  a page was shown  (context menu?)
  // vtkKWEvent::NotebookHidePageEvent:  a page was hidden (context menu)

  // Description:
  // Callbacks. Internal, do not use.
  virtual void PageTabContextMenuCallback(int id, int x, int y);
  virtual void RaiseCallback(int id);
  virtual void TogglePagePinnedCallback(int id);
  virtual void TogglePageVisibilityCallback(int id);

protected:
  vtkKWNotebook();
  ~vtkKWNotebook();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  int MinimumWidth;
  int MinimumHeight;
  int AlwaysShowTabs;
  int ShowIcons;
  int ShowAllPagesWithSameTag;
  int ShowOnlyPagesWithSameTag;
  int ShowOnlyMostRecentPages;
  int NumberOfMostRecentPages;
  int PagesCanBePinned;
  int EnablePageTabContextMenu;
  int UseFrameWithScrollbars;

  double PageTabColor[3];
  double SelectedPageTabColor[3];
  double PinnedPageTabOutlineColor[3];
  int SelectedPageTabPadding;

  vtkKWFrame *TabsFrame;
  vtkKWFrame *Body;
  vtkKWFrame *Mask;
  vtkKWMenu   *TabPopupMenu;

  //BTX

  // A notebook page

  class Page
  {
  public:
    void Delete();
    void UpdateEnableState();
    void Bind();
    void UnBind();

    int             Id;
    int             Visibility;
    int             Pinned;
    int             Tag;
    int             Enabled;
    char            *Title;
    vtkKWCoreWidget *Frame;
    vtkKWFrame      *TabFrame;
    vtkKWLabel      *Label;
    vtkKWLabel      *ImageLabel;
    vtkKWIcon       *Icon;
  };

  // PIMPL Encapsulation for STL containers

  vtkKWNotebookInternals *Internals;
  friend class vtkKWNotebookInternals;

  // Return a pointer to a page.
  // If a page title is provided instead of a page id, the first page matching
  // that title is considered. In the same way, if a tag is provided with the 
  // title, the page which title *and* tag match is considered.

  Page* GetPage(int id);
  Page* GetPage(const char *title);
  Page* GetPage(const char *title, int tag);

  // Get the first visible page
  // Get the first page matching a tag
  // Get the first packed page not matching a tag

  Page* GetFirstVisiblePage();
  Page* GetFirstPageMatchingTag(int tag);
  Page* GetFirstPackedPageNotMatchingTag(int tag);
  
  // Raise, Lower, Remove, Show, Hide, Pin, Unpin, Tag a specific page

  void SetPageTag(Page*, int tag);
  void RaisePage(Page*);
  void ShowPageTab(Page*);
  void ShowPageTabAsLow(Page*);
  void LowerPage(Page*);
  int  RemovePage(Page*);
  void ShowPage(Page*);
  void HidePage(Page*);
  void PinPage(Page*);
  void UnpinPage(Page*);
  void TogglePagePinned(Page*);
  int  GetPageVisibility(Page*);
  void TogglePageVisibility(Page*);
  int  CanBeHidden(Page*);
  int  GetPageTag(Page*);
  int  GetPagePinned(Page*);
  const char* GetPageTitle(Page*);
  void SetPageEnabled(Page*, int flag);

  int AddToMostRecentPages(Page*);
  int RemoveFromMostRecentPages(Page*);
  int PutOnTopOfMostRecentPages(Page*);

  // Update the tab frame color of a page

  virtual void UpdatePageTabAspect(Page*);
  virtual void UpdateAllPagesTabAspect();

  //ETX

  int IdCounter;
  int CurrentId;
  int Expanding;

  // Returns true if some tabs are visible.

  int AreTabsVisible();

  // Update the position of the body and mask elements

  void UpdateBodyPosition();
  void UpdateMaskPosition();

  // Constrain the visible pages depending on:
  // ShowAllPagesWithSameTag,
  // ShowOnlyPagesWithSameTag, 
  // ShowOnlyMostRecentPages

  void ConstrainVisiblePages();

  // Send event

  void SendEventForPage(unsigned long event, int id);

  // Description:
  // Bind/Unbind events.
  virtual void Bind();
  virtual void UnBind();

private:
  vtkKWNotebook(const vtkKWNotebook&); // Not implemented
  void operator=(const vtkKWNotebook&); // Not implemented
};

#endif

