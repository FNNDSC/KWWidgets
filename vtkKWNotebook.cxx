/*=========================================================================

Program:   Visualization Toolkit
Module:    $RCSfile: vtkKWNotebook.cxx,v $
Language:  C++
Date:      $Date: 2003-02-27 21:33:32 $
Version:   $Revision: 1.41 $

Copyright (c) 2000-2001 Kitware Inc. 469 Clifton Corporate Parkway,
Clifton Park, NY, 12065, USA.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

* Neither the name of Kitware nor the names of any contributors may be used
to endorse or promote products derived from this software without specific 
prior written permission.

* Modified source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "vtkKWNotebook.h"

#include "vtkKWApplication.h"
#include "vtkKWFrame.h"
#include "vtkKWIcon.h"
#include "vtkKWImageLabel.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWWidgetsConfigure.h"
#include "vtkLinkedList.txx"
#include "vtkLinkedListIterator.txx"
#include "vtkMath.h"
#include "vtkObjectFactory.h"

// The amount of padding that is to be added to the internal vertical padding of
// the selected tab (basically defines the additional height of the selected
// tab compared to an unselected tab).

#define VTK_KW_NB_TAB_SELECT_BD_Y  2

// The amount of horizontal padding separating each tab in the top tab row.

#define VTK_KW_NB_TAB_PADX         1

// The size of the border of each tab (as well as the main notebook body/frame)

#define VTK_KW_NB_TAB_BD           2

// The amount of horizontal padding around the tabs frame itself (i.e. defines
// a margin between the first (respectively last) tab and the left (respectively
// right) border of the notebook widget

#define VTK_KW_NB_TAB_FRAME_PADX   10

// Given the HSV (Hue, Saturation, Value) of a frame, give the % of Value used
// by the corresponding tab when it is not selected (i.e. dim unselected tabs).

#define VTK_KW_NB_TAB_UNSELECTED_VALUE 0.93

// The color of the border of a pinned tab

#define VTK_KW_NB_TAB_PIN_R 255
#define VTK_KW_NB_TAB_PIN_G 255
#define VTK_KW_NB_TAB_PIN_B 194

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWNotebook);
vtkCxxRevisionMacro(vtkKWNotebook, "$Revision: 1.41 $");

//------------------------------------------------------------------------------
int vtkKWNotebookCommand(ClientData cd, Tcl_Interp *interp,
                         int argc, char *argv[]);

//------------------------------------------------------------------------------
void vtkKWNotebook::Page::Delete()
{
  if (this->Title)
    {
    delete [] this->Title;
    this->Title = NULL;
    }

  if (this->Frame)
    {
    this->Frame->Delete();
    this->Frame = NULL;
    }

  if (this->TabFrame)
    {
    this->TabFrame->Delete();
    this->TabFrame = NULL;
    }

  if (this->Label)
    {
    this->Label->Delete();
    this->Label = NULL;
    }

  if (this->ImageLabel)
    {
    this->ImageLabel->Delete();
    this->ImageLabel = NULL;
    }

  if (this->Icon)
    {
    this->Icon->Delete();
    this->Icon = NULL;
    }
}

//------------------------------------------------------------------------------
vtkKWNotebook::vtkKWNotebook()
{
  this->CommandFunction = vtkKWNotebookCommand;

  this->MinimumWidth   = 360;
  this->MinimumHeight  = 600;
  this->AlwaysShowTabs = 0;
  this->ShowAllPagesWithSameTag = 0;
  this->ShowOnlyPagesWithSameTag = 0;
  this->ShowOnlyMostRecentPages = 0;
  this->NumberOfMostRecentPages = 4;
  this->PagesCanBePinned = 0;
#ifdef USE_NOTEBOOK_ICONS
  this->ShowIcons      = 1;
#else
  this->ShowIcons      = 0;
#endif

  this->IdCounter      = 0;
  this->CurrentId      = -1;
  this->Expanding      = 0;

  this->Body           = vtkKWWidget::New();
  this->Mask           = vtkKWWidget::New();
  this->TabsFrame      = vtkKWWidget::New();

  this->Pages          = vtkKWNotebook::PagesContainer::New();

  this->MostRecentPages = vtkKWNotebook::PagesContainer::New();
}

//------------------------------------------------------------------------------
vtkKWNotebook::~vtkKWNotebook()
{
  this->Body->Delete();
  this->Mask->Delete();
  this->TabsFrame->Delete();

  // Delete all pages

  vtkKWNotebook::Page *page = NULL;
  vtkKWNotebook::PagesContainerIterator *it = this->Pages->NewIterator();
  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    if (it->GetData(page) == VTK_OK)
      {
      page->Delete();
      delete page;
      }
    it->GoToNextItem();
    }
  it->Delete();

  // Delete the container

  this->Pages->Delete();

  // Empty the most recent pages

  this->MostRecentPages->Delete();
}

//------------------------------------------------------------------------------
void vtkKWNotebook::Create(vtkKWApplication *app, const char *args)
{
  // Set the application

  if (this->IsCreated())
    {
    vtkErrorMacro("The notebook is already created");
    return;
    }

  this->SetApplication(app);

  const char *wname;

  ostrstream cmd;

  // Create the frame holding the whole widget

  wname = this->GetWidgetName();
  this->Script("frame %s -width %d -height %d -bd 0 -relief flat %s",
               wname, this->MinimumWidth, this->MinimumHeight, args);

  // Create the frame that stores the tabs "button"

  this->TabsFrame->SetParent(this);
  this->TabsFrame->Create(app, "frame", "-bd 0 -relief flat");

  cmd << "pack " << this->TabsFrame->GetWidgetName() 
      << " -fill x -expand y -side top -anchor n " 
      << " -padx " << VTK_KW_NB_TAB_FRAME_PADX << endl;

  // Create the frame where each page will be mapped

  this->Body->SetParent(this);
  this->Body->Create(app, "frame", "");

  cmd << this->Body->GetWidgetName() << " config -relief raised "
      << " -bd " << VTK_KW_NB_TAB_BD << endl;

  // Create the mask used to hide the seam between the current active tab and
  // the body (i.e. between the tab and the corresponding page under the tab).

  this->Mask->SetParent(this);
  this->Mask->Create(app, "frame","-bd 0 -relief flat");

  // Bind <Configure> event to both the tabs frame (in case a tab is added
  // and its size requires the whole widget to be resized) and the body.

  cmd << "bind " << this->TabsFrame->GetWidgetName() << " <Configure> {" 
      << this->GetTclName() << " ScheduleResize}" << endl;

  cmd << "bind " << this->Body->GetWidgetName() << " <Configure> {" 
      << this->GetTclName() << " ScheduleResize}" << endl;

  cmd << ends;
  this->Script(cmd.str());
  cmd.rdbuf()->freeze(0);

  // Update enable state

  this->UpdateEnableState();
}

//------------------------------------------------------------------------------
vtkKWNotebook::Page* vtkKWNotebook::GetPage(int id)
{
  vtkKWNotebook::Page *page = NULL;
  vtkKWNotebook::Page *found = NULL;
  vtkKWNotebook::PagesContainerIterator *it = this->Pages->NewIterator();

  // Return the page corresponding to that id

  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    if (it->GetData(page) == VTK_OK && page->Id == id)
      {
      found = page;
      break;
      }
    it->GoToNextItem();
    }
  it->Delete();

  return found;
}

//------------------------------------------------------------------------------
vtkKWNotebook::Page* vtkKWNotebook::GetPage(const char *title)
{
  if (title == NULL)
    {
    return NULL;
    }

  vtkKWNotebook::Page *page = NULL;
  vtkKWNotebook::Page *found = NULL;
  vtkKWNotebook::PagesContainerIterator *it = this->Pages->NewIterator();

  // Return the page corresponding to that title

  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    if (it->GetData(page) == VTK_OK && 
        page->Title && 
        !strcmp(title, page->Title))
      {
      found = page;
      break;
      }
    it->GoToNextItem();
    }
  it->Delete();

  return found;
}

//------------------------------------------------------------------------------
vtkKWNotebook::Page* vtkKWNotebook::GetPage(const char *title, int tag)
{
  if (title == NULL)
    {
    return NULL;
    }

  vtkKWNotebook::Page *page = NULL;
  vtkKWNotebook::Page *found = NULL;
  vtkKWNotebook::PagesContainerIterator *it = this->Pages->NewIterator();

  // Return the page corresponding to that title and that tag

  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    if (it->GetData(page) == VTK_OK &&
        page->Tag == tag &&
        page->Title && 
        !strcmp(title, page->Title))
      {
      found = page;
      break;
      }
    it->GoToNextItem();
    }
  it->Delete();

  return found;
}

//------------------------------------------------------------------------------
vtkKWNotebook::Page* vtkKWNotebook::GetFirstVisiblePage()
{
  vtkKWNotebook::Page *page = NULL;
  vtkKWNotebook::Page *found = NULL;
  vtkKWNotebook::PagesContainerIterator *it = this->Pages->NewIterator();

  // Return the first visible page

  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    if (it->GetData(page) == VTK_OK && page->Visibility)
      {
      found = page;
      break;
      }
    it->GoToNextItem();
    }
  it->Delete();

  return found;
}

//------------------------------------------------------------------------------
vtkKWNotebook::Page* vtkKWNotebook::GetFirstPageMatchingTag(int tag)
{
  vtkKWNotebook::Page *page = NULL;
  vtkKWNotebook::Page *found = NULL;
  vtkKWNotebook::PagesContainerIterator *it = this->Pages->NewIterator();

  // Return the first page matching tag

  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    if (it->GetData(page) == VTK_OK && page->Tag == tag)
      {
      found = page;
      break;
      }
    it->GoToNextItem();
    }
  it->Delete();

  return found;
}

//------------------------------------------------------------------------------
vtkKWNotebook::Page* vtkKWNotebook::GetFirstPackedPageNotMatchingTag(int tag)
{
  if (!this->IsCreated())
    {
    return NULL;
    }

  // Get the slaves in the tabs frame

  char **slaves = 0;
  int nb_slaves = vtkKWTkUtilities::GetSlaves(
    this->Application->GetMainInterp(),this->TabsFrame->GetWidgetName(),&slaves);
  if (!nb_slaves)
    {
    return NULL;
    }

  // Iterate over each slave and find the corresponding page and its tag

  vtkKWNotebook::Page *found = NULL;
  int i;
  for (i = 0 ; i < nb_slaves && !found; i++)
    {
    vtkKWNotebook::Page *page = NULL;
    vtkKWNotebook::PagesContainerIterator *it = this->Pages->NewIterator();

    it->InitTraversal();
    while (!it->IsDoneWithTraversal())
      {
      if (it->GetData(page) == VTK_OK && 
          page->Tag != tag && 
          !strcmp(slaves[i], page->TabFrame->GetWidgetName()))
        {
        found = page;
        break;
        }
      it->GoToNextItem();
      }
    it->Delete();
    }

  // Deallocate slaves

  for (i = 0 ; i < nb_slaves; i++)
    {
    delete [] slaves[i];
    }
  delete [] slaves;

  return found;
}

//------------------------------------------------------------------------------
unsigned int vtkKWNotebook::GetNumberOfPages()
{
  return (unsigned int)this->Pages->GetNumberOfItems();
}

//------------------------------------------------------------------------------
unsigned int vtkKWNotebook::GetNumberOfVisiblePages()
{
  unsigned int count = 0;
  vtkKWNotebook::Page *page = NULL;
  vtkKWNotebook::PagesContainerIterator *it = this->Pages->NewIterator();

  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    if (it->GetData(page) == VTK_OK && page->Visibility)
      {
      count++;
      }
    it->GoToNextItem();
    }
  it->Delete();

  return count;
}

//------------------------------------------------------------------------------
int vtkKWNotebook::AreTabsVisible()
{
  int visible_pages = this->GetNumberOfVisiblePages();
  return 
    (visible_pages > 1 || (visible_pages == 1 && this->AlwaysShowTabs)) ? 1 : 0;
}

//------------------------------------------------------------------------------
vtkKWWidget *vtkKWNotebook::GetFrame(int id)
{
  // Return the frame corresponding to that id

  vtkKWNotebook::Page *page = this->GetPage(id);
  if (page)
    {
    return page->Frame->GetFrame();
    }
  return NULL;
}

//------------------------------------------------------------------------------
vtkKWWidget *vtkKWNotebook::GetFrame(const char *title)
{
  // Return the frame corresponding to that title

  vtkKWNotebook::Page *page = this->GetPage(title);
  if (page)
    {
    return page->Frame->GetFrame();
    }
  return NULL;
}

//------------------------------------------------------------------------------
vtkKWWidget *vtkKWNotebook::GetFrame(const char *title, int tag)
{
  // Return the frame corresponding to that title and tag

  vtkKWNotebook::Page *page = this->GetPage(title, tag);
  if (page)
    {
    return page->Frame->GetFrame();
    }
  return NULL;
}

//------------------------------------------------------------------------------
int vtkKWNotebook::AddPage(const char *title)
{
  return this->AddPage(title, 0);
}

//------------------------------------------------------------------------------
int vtkKWNotebook::AddPage(const char *title, 
                           const char *balloon)
{
  return this->AddPage(title, balloon, 0);
}

//------------------------------------------------------------------------------
int vtkKWNotebook::AddPage(const char *title, 
                           const char *balloon, 
                           vtkKWIcon *icon)
{
  return this->AddPage(title, balloon, icon, 0);
}

//------------------------------------------------------------------------------
int vtkKWNotebook::AddPage(const char *title, 
                           const char *balloon, 
                           vtkKWIcon *icon,
                           int tag)
{
  if (!this->IsCreated())
    {
    return -1;
    }

  ostrstream cmd;

  // Create a new page, insert it in the container
  
  vtkKWNotebook::Page *page = new vtkKWNotebook::Page;
  if (this->Pages->AppendItem(page) != VTK_OK)
    {
    vtkErrorMacro("Error while adding a page to the notebook");
    delete page;
    return -1;
    }

  // Each page has a unique ID in the notebook lifetime
  // It is visible and not pinned by default

  page->Id = this->IdCounter++;
  page->Pinned = 0;
  page->Tag = tag;

  // Create the page frame (this is where user-defined widgets will be packed)

  page->Frame = vtkKWFrame::New();
  page->Frame->SetParent(this->Body);
  page->Frame->Create(this->Application, 0);

  // Store the page title for fast page retrieval on title

  page->Title = new char [strlen(title) + 1];
  strcpy(page->Title, title);

  // Create the "tab" part of the page

  page->TabFrame = vtkKWFrame::New();
  page->TabFrame->SetParent(this->TabsFrame);
  page->TabFrame->Create(this->Application, 0);

  cmd << page->TabFrame->GetWidgetName() << " config -relief raised "
      << " -bd " << VTK_KW_NB_TAB_BD << endl;

  // Create the label that holds the page title

  page->Label = vtkKWLabel::New();
  page->Label->SetParent(page->TabFrame);
  page->Label->Create(this->Application, "-highlightthickness 0");
  page->Label->SetLabel(page->Title);
  if (balloon)
    {
    page->Label->SetBalloonHelpString(balloon);
    }

  cmd << "pack " << page->Label->GetWidgetName() 
      << " -side left -fill both -expand y -anchor c" << endl
      << "bind " << page->Label->GetWidgetName() << " <Button-1> {" 
      << this->GetTclName() << " Raise " << page->Id << "}" << endl
      << "bind " << page->Label->GetWidgetName() << " <Double-1> {" 
      << this->GetTclName() << " PinPageToggleCallback " << page->Id << "}" 
      << endl;

  // Create the icon if any. We want to keep both the icon and the image label
  // since the icon is required to recreate the label when its background
  // color changes

  page->ImageLabel = 0;
  page->Icon = 0;

  if (icon && icon->GetData())
    {
    page->Icon = vtkKWIcon::New();
    page->Icon->SetImageData(icon);

    page->ImageLabel = vtkKWImageLabel::New();
    page->ImageLabel->SetParent(page->TabFrame);
    page->ImageLabel->Create(this->Application, "");
    page->ImageLabel->SetImageData(page->Icon);

    cmd << "bind " << page->ImageLabel->GetWidgetName() << " <Button-1> {" 
        << this->GetTclName() << " Raise " << page->Id << "}" << endl;

    if (this->ShowIcons)
      {
      cmd << "pack " << page->ImageLabel->GetWidgetName() 
          << " -side left -fill both -anchor c "
          << " -before " << page->Label->GetWidgetName() << endl;
      }
    }

  cmd << ends;
  this->Script(cmd.str());
  cmd.rdbuf()->freeze(0);

  // Show the page. Set Visibility to Off first. If this page can really
  // be shown, Visibility will be set to On automatically.
  // Do not show anything if we are maintaining a list of most recent pages
  // shown (which should be controled by the user, not the way developpers
  // have added pages).

  page->Visibility = 0;
  
  if (!this->ShowOnlyMostRecentPages)
    {
    this->ShowPage(page);
    }

  return page->Id;
}

//------------------------------------------------------------------------------
int vtkKWNotebook::RemovePage(int id)
{
  return this->RemovePage(this->GetPage(id));
}

//------------------------------------------------------------------------------
int vtkKWNotebook::RemovePage(const char *title)
{
  return this->RemovePage(this->GetPage(title));
}

//------------------------------------------------------------------------------
int vtkKWNotebook::RemovePage(vtkKWNotebook::Page *page)
{
  if (!this->IsCreated())
    {
    return 0;
    }

  if (page == NULL)
    {
    vtkErrorMacro("Can not remove a NULL page from the notebook.");
    return 0;
    }

  vtkIdType pos = 0;
  if (this->Pages->FindItem(page, pos) != VTK_OK)
    {
    vtkErrorMacro("Error while removing a page from the notebook "
                  "(can not find the page).");
    return 0;
    }

  // Remove the page from the container

  if (this->Pages->RemoveItem(pos) != VTK_OK)
    {
    vtkErrorMacro("Error while removing a page from the notebook.");
    return 0;
    }

  // If the page to delete is selected, select another one among
  // the visible one (which will be different than the current one since
  // we just removed it from the pool).
  // if no other one, set current selection to nothing (-1)

  if (page->Id == this->CurrentId)
    {
    vtkKWNotebook::Page *new_page = this->GetFirstVisiblePage();
    if (new_page)
      {
      this->RaisePage(new_page);
      }
    else
      {
      this->CurrentId = -1;
      }
    }

  // Unpack the widgets, delete the page

  this->Script("pack forget %s %s", 
               page->TabFrame->GetWidgetName(), page->Frame->GetWidgetName());
  page->Delete();
  delete page;

  // Bring or remove more pages depending on options
  
  this->ConstrainVisiblePages();

  // Schedule a resize since removing a page might make a the tab frame smaller
  // (the <Configure> event might do the trick too, but I don't trust Tk, 
  // yadi yada...)

  this->ScheduleResize();

  return 1;
}

//------------------------------------------------------------------------------
void vtkKWNotebook::RemovePagesMatchingTag(int tag)
{
  vtkKWNotebook::Page *page = NULL;
  vtkKWNotebook::PagesContainerIterator *it = this->Pages->NewIterator();

  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    if (it->GetData(page) == VTK_OK && page->Tag == tag)
      {
      it->GoToNextItem();
      this->RemovePage(page);
      }
    else
      {
      it->GoToNextItem();
      }
    }
  it->Delete();
}

//------------------------------------------------------------------------------
void vtkKWNotebook::Raise(int id)
{
  this->RaisePage(this->GetPage(id));
}

//------------------------------------------------------------------------------
void vtkKWNotebook::Raise(const char *title)
{
  this->RaisePage(this->GetPage(title));
}

//------------------------------------------------------------------------------
void vtkKWNotebook::Raise(const char *title, int tag)
{
  this->RaisePage(this->GetPage(title, tag));
}

//------------------------------------------------------------------------------
void vtkKWNotebook::RaisePage(vtkKWNotebook::Page *page)
{
  if (page == NULL || !this->IsCreated())
    {
    return;
    }
  
  // Lower the current one (unpack the page, shrink the selected tab)

  if (page->Id != this->CurrentId)
    {
    vtkKWNotebook::Page *old_page = this->GetPage(this->CurrentId);
    if (old_page)
      {
      this->LowerPage(old_page);
      }
    }

  // Set the selected page

  this->CurrentId = page->Id;

  // A raised page becomes automatically visible (i.e., it's a way to unhide it)

  page->Visibility = 1;

  // Show the tab body

  ostrstream cmd;
  cmd << "pack " << page->Frame->GetWidgetName() << " -fill both -anchor n" 
      << endl;

  // Show the page tab

  this->ShowPageTab(page);
  
  // Select the page tab: enlarge it

  cmd << "pack " << page->TabFrame->GetWidgetName() 
      << " -ipadx 0 -ipady " << VTK_KW_NB_TAB_SELECT_BD_Y 
      << " -padx " << VTK_KW_NB_TAB_PADX
      << endl;

  // Warning: the following can have *very* nasty side effects. For example, 
  // select a vtkKWView might trigger a ViewSelectedEvent that will bring
  // up the UI for that view. If showing that UI involves raising a page,
  // the focus will be lost *aynchronously* (because of the events). Therefore,
  // although the view has been selected, the focus is not in the view, and
  // events like keys might not be triggered.
  // => Let's NOT use it :) Focus is evil.
  // cmd << "focus " << page->Frame->GetWidgetName() << endl;

  cmd << ends;
  this->Script(cmd.str());
  cmd.rdbuf()->freeze(0);
  
  // Update the page color

  this->UpdatePageTabBackgroundColor(page, 1);
 
  // Bring or remove more pages depending on options
  
  this->ConstrainVisiblePages();

  // Schedule a resize since raising a page might make a larger tab or a larger
  // page show up (the <Configure> event might do the trick too, but I don't
  // trust Tk that much)

  this->ScheduleResize();
}

//------------------------------------------------------------------------------
int vtkKWNotebook::GetRaisedPageId()
{
  return this->CurrentId;
}

//------------------------------------------------------------------------------
void vtkKWNotebook::RaiseFirstPageMatchingTag(int tag)
{
  this->RaisePage(this->GetFirstPageMatchingTag(tag));
}

//------------------------------------------------------------------------------
void vtkKWNotebook::ShowPageTab(vtkKWNotebook::Page *page)
{
  if (page == NULL || !this->IsCreated())
    {
    return;
    }
  
  ostrstream cmd;
  cmd << "pack " << page->TabFrame->GetWidgetName() << " -side left -anchor s";

  // If the tab was not packed, we are about to bring up a new page tab
  // which should be enqueue in the most recent pages list.

  if (this->ShowOnlyMostRecentPages && 
      page->Visibility && 
      !page->TabFrame->IsPacked())
    {
    this->EnqueueMostRecentPage(page);

    // Also, if we show only the most recent pages, the expected behaviour would
    // be to bring the page so that it is packed as the first one in the list.
    // If ShowAllPagesWithSameTag is Off, just pack the tab in front of all
    // others, otherwise try to pack in front of the first page that has a
    // different tag, so that pages with the same tag will still be packed
    // in the right order.

    if (page->TabFrame->GetParent()->GetNumberOfPackedChildren())
      {
      if (!this->ShowAllPagesWithSameTag)
        {
        cmd << " -before [lindex [pack slaves " 
            << page->TabFrame->GetParent()->GetWidgetName() << "] 0]";
        }
      else
        {
        vtkKWNotebook::Page *other_page = 
          this->GetFirstPackedPageNotMatchingTag(page->Tag);
        if (other_page)
          {
          cmd << " -before " << other_page->TabFrame->GetWidgetName();
          }
        else
          {
          cmd << " -before [lindex [pack slaves " 
              << page->TabFrame->GetParent()->GetWidgetName() << "] 0]";
          }
        }
      }
    }

  cmd << ends;
  this->Script(cmd.str());
  cmd.rdbuf()->freeze(0);
}

//------------------------------------------------------------------------------
void vtkKWNotebook::ShowPageTabAsLow(vtkKWNotebook::Page *page)
{
  if (page == NULL || !this->IsCreated())
    {
    return;
    }
  
  // Show the page tab

  this->ShowPageTab(page);

  // Unselect the page tab: shrink it (no ipady)

  this->Script("pack %s -ipadx 0 -ipady 0 -padx %d",
               page->TabFrame->GetWidgetName(), VTK_KW_NB_TAB_PADX);

  // Update the page color (dim it)

  this->UpdatePageTabBackgroundColor(page, 0);

  // Make sure everything has been resized properly

  this->ScheduleResize();
}

//------------------------------------------------------------------------------
void vtkKWNotebook::LowerPage(vtkKWNotebook::Page *page)
{
  if (page == NULL || !this->IsCreated())
    {
    return;
    }
  
  ostrstream cmd;

  // Unpack the page

  cmd << "pack forget " << page->Frame->GetWidgetName() << endl;

  cmd << ends;
  this->Script(cmd.str());
  cmd.rdbuf()->freeze(0);

  // Lower the tab (which will schedule a resize)

  this->ShowPageTabAsLow(page);
}

//------------------------------------------------------------------------------
void vtkKWNotebook::SetPageTag(int id, int tag)
{
  this->SetPageTag(this->GetPage(id), tag);
}

//------------------------------------------------------------------------------
void vtkKWNotebook::SetPageTag(const char *title, int tag)
{
  this->SetPageTag(this->GetPage(title), tag);
}

//------------------------------------------------------------------------------
void vtkKWNotebook::SetPageTag(vtkKWNotebook::Page *page, int tag)
{
  if (page == NULL || !this->IsCreated())
    {
    return;
    }

  page->Tag = tag;
}

//------------------------------------------------------------------------------
int vtkKWNotebook::GetPageTag(int id)
{
  return this->GetPageTag(this->GetPage(id));
}

//------------------------------------------------------------------------------
int vtkKWNotebook::GetPageTag(const char *title)
{
  return this->GetPageTag(this->GetPage(title));
}

//------------------------------------------------------------------------------
int vtkKWNotebook::GetPageTag(vtkKWNotebook::Page *page)
{
  if (page == NULL || !this->IsCreated())
    {
    vtkErrorMacro("Can not query page tag.");
    return 0;
    }

  return page->Tag;
}

//------------------------------------------------------------------------------
char* vtkKWNotebook::GetPageTitle(int id)
{
  return this->GetPageTitle(this->GetPage(id));
}

//------------------------------------------------------------------------------
char* vtkKWNotebook::GetPageTitle(vtkKWNotebook::Page *page)
{
  if (page == NULL || !this->IsCreated())
    {
    vtkErrorMacro("Can not query page title.");
    return 0;
    }

  return page->Title;
}

//------------------------------------------------------------------------------
void vtkKWNotebook::ShowPage(int id)
{
  this->ShowPage(this->GetPage(id));
}

//------------------------------------------------------------------------------
void vtkKWNotebook::ShowPage(const char *title)
{
  this->ShowPage(this->GetPage(title));
}

//------------------------------------------------------------------------------
void vtkKWNotebook::ShowPage(vtkKWNotebook::Page *page)
{
  if (page == NULL || !this->IsCreated() || page->Visibility)
    {
    return;
    }
  
  // If the number of visible pages is 0, raise this one automatically
    
  if (this->GetNumberOfVisiblePages() == 0)
    {
    this->RaisePage(page);
    return;
    }

  page->Visibility = 1;

  // Otherwise just display the page tab in the non-selected mode "low" mode.

  this->ShowPageTabAsLow(page);

  // Bring or remove more pages depending on options
  
  this->ConstrainVisiblePages();
}

//------------------------------------------------------------------------------
int vtkKWNotebook::CanBeHidden(int id)
{
  return this->CanBeHidden(this->GetPage(id));
}

//------------------------------------------------------------------------------
int vtkKWNotebook::CanBeHidden(const char *title)
{
  return this->CanBeHidden(this->GetPage(title));
}

//------------------------------------------------------------------------------
int vtkKWNotebook::CanBeHidden(vtkKWNotebook::Page *page)
{
  if (page == NULL || !this->IsCreated())
    {
    return -1;
    }
  
  // A pinned page can not be hidden

  return !page->Pinned;
}

//------------------------------------------------------------------------------
void vtkKWNotebook::HidePage(int id)
{
  this->HidePage(this->GetPage(id));
}

//------------------------------------------------------------------------------
void vtkKWNotebook::HidePage(const char *title)
{
  this->HidePage(this->GetPage(title));
}

//------------------------------------------------------------------------------
void vtkKWNotebook::HidePage(vtkKWNotebook::Page *page)
{
  if (page == NULL || 
      !this->IsCreated() || 
      !page->Visibility || 
      !this->CanBeHidden(page))
    {
    return;
    }
  
  page->Visibility = 0;
  
  // If the page to hide is selected, select another one among
  // the visible one (which will be different than the current one since
  // we just removed it from the pool of visible pages).
  // if no other one, set current selection to nothing (-1)
    
  if (page->Id == this->CurrentId)
    {
    vtkKWNotebook::Page *new_page = this->GetFirstVisiblePage();
    if (new_page)
      {
      this->RaisePage(new_page);
      }
    else
      {
      this->LowerPage(page);
      this->CurrentId = -1;
      }
    }

  // If tab to hide was packed, unpack it
  
  if (page->TabFrame->IsPacked())
    {
    this->Script("pack forget %s", page->TabFrame->GetWidgetName());
    }

  // Bring or remove more pages depending on options
  
  this->ConstrainVisiblePages();

  // Make sure everything has been resized properly

  this->ScheduleResize();
}

//------------------------------------------------------------------------------
void vtkKWNotebook::SetPageVisibility(int id, int flag)
{
  if (flag)
    {
    this->ShowPage(id);
    }
  else
    {
    this->HidePage(id);
    }
}

//------------------------------------------------------------------------------
void vtkKWNotebook::SetPageVisibility(const char *title, int flag)
{
  if (flag)
    {
    this->ShowPage(title);
    }
  else
    {
    this->HidePage(title);
    }
}

//------------------------------------------------------------------------------
int vtkKWNotebook::GetPageVisibility(int id)
{
  return this->GetPageVisibility(this->GetPage(id));
}

//------------------------------------------------------------------------------
int vtkKWNotebook::GetPageVisibility(const char *title)
{
  return this->GetPageVisibility(this->GetPage(title));
}

//------------------------------------------------------------------------------
int vtkKWNotebook::GetPageVisibility(vtkKWNotebook::Page *page)
{
  if (page == NULL || !this->IsCreated())
    {
    return -1;
    }
  
  return page->Visibility;
}

//------------------------------------------------------------------------------
void vtkKWNotebook::ShowPagesMatchingTag(int tag)
{
  vtkKWNotebook::Page *page = NULL;
  vtkKWNotebook::PagesContainerIterator *it = this->Pages->NewIterator();

  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    if (it->GetData(page) == VTK_OK && page->Tag == tag)
      {
      this->ShowPage(page);
      }
    it->GoToNextItem();
    }
  it->Delete();
}

//------------------------------------------------------------------------------
void vtkKWNotebook::ShowPagesMatchingTagReverse(int tag)
{
  vtkKWNotebook::Page *page = NULL;
  vtkKWNotebook::PagesContainerIterator *it = this->Pages->NewIterator();

  it->GoToLastItem();
  while (!it->IsDoneWithTraversal())
    {
    if (it->GetData(page) == VTK_OK && page->Tag == tag)
      {
      this->ShowPage(page);
      }
    it->GoToPreviousItem();
    }
  it->Delete();
}

//------------------------------------------------------------------------------
void vtkKWNotebook::HidePagesMatchingTag(int tag)
{
  vtkKWNotebook::Page *page = NULL;
  vtkKWNotebook::PagesContainerIterator *it = this->Pages->NewIterator();

  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    if (it->GetData(page) == VTK_OK && page->Tag == tag)
      {
      this->HidePage(page);
      }
    it->GoToNextItem();
    }
  it->Delete();
}

//------------------------------------------------------------------------------
void vtkKWNotebook::ShowPagesNotMatchingTag(int tag)
{
  vtkKWNotebook::Page *page = NULL;
  vtkKWNotebook::PagesContainerIterator *it = this->Pages->NewIterator();

  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    if (it->GetData(page) == VTK_OK && page->Tag != tag)
      {
      this->ShowPage(page);
      }
    it->GoToNextItem();
    }
  it->Delete();
}

//------------------------------------------------------------------------------
void vtkKWNotebook::HidePagesNotMatchingTag(int tag)
{
  vtkKWNotebook::Page *page = NULL;
  vtkKWNotebook::PagesContainerIterator *it = this->Pages->NewIterator();

  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    if (it->GetData(page) == VTK_OK && page->Tag != tag)
      {
      this->HidePage(page);
      }
    it->GoToNextItem();
    }
  it->Delete();
}

//------------------------------------------------------------------------------
int vtkKWNotebook::EnqueueMostRecentPage(vtkKWNotebook::Page *page)
{
  if (page == NULL)
    {
    return 0;
    }

  this->MostRecentPages->PrependItem(page);
  return 1;
}

//------------------------------------------------------------------------------
void vtkKWNotebook::PinPage(int id)
{
  this->PinPage(this->GetPage(id));
}

//------------------------------------------------------------------------------
void vtkKWNotebook::PinPage(const char *title)
{
  this->PinPage(this->GetPage(title));
}

//------------------------------------------------------------------------------
void vtkKWNotebook::PinPage(vtkKWNotebook::Page *page)
{
  if (page == NULL || !this->IsCreated())
    {
    return;
    }

  page->Pinned = 1;
  this->UpdatePageTabBackgroundColor(page, this->CurrentId == page->Id);
}

//------------------------------------------------------------------------------
void vtkKWNotebook::UnpinPage(int id)
{
  this->UnpinPage(this->GetPage(id));
}

//------------------------------------------------------------------------------
void vtkKWNotebook::UnpinPage(const char *title)
{
  this->UnpinPage(this->GetPage(title));
}

//------------------------------------------------------------------------------
void vtkKWNotebook::UnpinPage(vtkKWNotebook::Page *page)
{
  if (page == NULL || !this->IsCreated())
    {
    return;
    }

  page->Pinned = 0;
  this->UpdatePageTabBackgroundColor(page, this->CurrentId == page->Id);
}

//------------------------------------------------------------------------------
unsigned int vtkKWNotebook::GetNumberOfPinnedPages()
{
  unsigned int count = 0;
  vtkKWNotebook::Page *page = NULL;
  vtkKWNotebook::PagesContainerIterator *it = this->Pages->NewIterator();

  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    if (it->GetData(page) == VTK_OK && page->Pinned)
      {
      count++;
      }
    it->GoToNextItem();
    }
  it->Delete();

  return count;
}

//------------------------------------------------------------------------------
void vtkKWNotebook::PinPageToggleCallback(int id)
{
  vtkKWNotebook::Page *page = this->GetPage(id);

  if (page == NULL || !this->PagesCanBePinned)
    {
    return;
    }

  if (page->Pinned)
    {
    this->UnpinPage(page);
    }
  else
    {
    this->PinPage(page);
    }
}

//------------------------------------------------------------------------------
void vtkKWNotebook::UpdatePageTabBackgroundColor(vtkKWNotebook::Page *page, 
                                                 int selected)
{
  // If selected, use the same tab color as the page, otherwise use
  // a shade of that page color.

  ostrstream cmd;

  if (selected)
    {
    cmd << page->Label->GetWidgetName() << " config -bg [" 
        << page->Frame->GetWidgetName() << " cget -bg]" << endl;

    if (page->Icon)
      {
      this->Script("%s config -bg [%s cget -bg]",
                   page->ImageLabel->GetWidgetName(), 
                   page->Frame->GetWidgetName());
      // Reset the imagelabel so that the icon is blended with the background
      page->ImageLabel->SetImageData(page->Icon);
      }

    // If the tab is pinned, use a color for the border of the tab
    // and render text italic

    if (!page->Pinned)
      {
      cmd << page->TabFrame->GetWidgetName() << " config -bg [" 
          << page->Frame->GetWidgetName() << " cget -bg]" << endl;
      vtkKWTkUtilities::ChangeFontSlantToRoman(
        this->Application->GetMainInterp(),
        page->Label->GetWidgetName());
      }
    else
      {
      char color[10];
      sprintf(color, "#%02x%02x%02x", 
              VTK_KW_NB_TAB_PIN_R, VTK_KW_NB_TAB_PIN_G, VTK_KW_NB_TAB_PIN_B);
      cmd << page->TabFrame->GetWidgetName() << " config -bg " << color << endl;
      vtkKWTkUtilities::ChangeFontSlantToItalic(
        this->Application->GetMainInterp(),
        page->Label->GetWidgetName());
      }
    }
  else
    {
    // Compute the shaded color based on the current frame background color

    float fr, fg, fb;
    page->Frame->GetBackgroundColor(&fr, &fg, &fb);

    float fh, fs, fv;
    if (fr == fg && fg == fb)
      {
      fh = fs = 0.0;
      fv = fr;
      }
    else
      {
      vtkMath::RGBToHSV(fr, fg, fb, &fh, &fs, &fv);
      }

    float r, g, b;
    fv *= VTK_KW_NB_TAB_UNSELECTED_VALUE;
    vtkMath::HSVToRGB(fh, fs, fv, &r, &g, &b);

    char shade[10];
    sprintf(shade, "#%02x%02x%02x", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0));
    
    cmd << page->Label->GetWidgetName() << " config -bg " << shade << endl;

    if (page->Icon)
      {
      this->Script("%s config -bg %s", page->ImageLabel->GetWidgetName(), shade);
      // Reset the imagelabel so that the icon is blended with the background
      page->ImageLabel->SetImageData(page->Icon);
      }

    // If the tab is pinned, use a different hue for the border of the tab

    if (!page->Pinned)
      {
      cmd << page->TabFrame->GetWidgetName() << " config -bg " << shade << endl;
      }
    else
      {
      fr = (float)VTK_KW_NB_TAB_PIN_R / 255.0;
      fg = (float)VTK_KW_NB_TAB_PIN_G / 255.0;
      fb = (float)VTK_KW_NB_TAB_PIN_B / 255.0;

      if (fr == fg && fg == fb)
        {
        fh = fs = 0.0;
        fv = fr;
        }
      else
        {
        vtkMath::RGBToHSV(fr, fg, fb, &fh, &fs, &fv);
        }

      fv *= VTK_KW_NB_TAB_UNSELECTED_VALUE;
      vtkMath::HSVToRGB(fh, fs, fv, &r, &g, &b);

      sprintf(shade, "#%02x%02x%02x", 
              (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0));
    
      cmd << page->TabFrame->GetWidgetName() << " config -bg " << shade << endl;
      }
    }

  cmd << ends;
  this->Script(cmd.str());
  cmd.rdbuf()->freeze(0);

  // If the page that has just been update was the selected page, update
  // the mask position since the color has an influence on the mask size (win32)

#if _WIN32
  if (this->CurrentId == page->Id)
    {
    this->UpdateMaskPosition();
    }
#endif
}

//------------------------------------------------------------------------------
void vtkKWNotebook::UpdateBodyPosition()
{
  if (!this->IsCreated())
    {
    return;
    }

  // If the tabs should not be displayed, place the body so that it uses
  // the whole notebook space

  if (!this->AreTabsVisible())
    {
    this->Script("place %s -x 0 -y 0 -relwidth 1.0 -relheight 1.0 -height 0",
                 this->Body->GetWidgetName());
    }
  else
    {
    // Otherwise get the size of the frame hold the tabs, and place the body
    // right under it, but slightly higher so that the bottom border of the
    // tabs is hidden (which will give the "notebook" look).

    this->Script("winfo reqheight %s", this->TabsFrame->GetWidgetName());
    int rheight = vtkKWObject::GetIntegerResult(this->Application);

    // if 1, then we have not been mappep/configured at the moment, but this
    // function will be called by Resize() when Configure is triggered, so
    // we can return safely now.

    if (rheight <= 1)
      {
      return;
      }

    rheight -= VTK_KW_NB_TAB_BD;

    this->Script("place %s -x 0 -y %d -relwidth 1.0 -relheight 1.0 -height %d",
                 this->Body->GetWidgetName(), rheight, -rheight);
    }
}

//------------------------------------------------------------------------------
void vtkKWNotebook::UpdateMaskPosition()
{
  if (!this->IsCreated())
    {
    return;
    }

  // If the tabs should not be displayed, don't bother about the mask,
  // just forget it.

  if (!this->AreTabsVisible())
    {
    this->Script("place forget %s", this->Mask->GetWidgetName());
    }
  else
    {
    // Otherwise get the selected page

    vtkKWNotebook::Page *page = this->GetPage(this->CurrentId);
    if (!page)
      {
      vtkErrorMacro("Error while updating the mask position in the notebook "
                    "(no current page)");
      return;
      }

    const char *res;

    // tabs_x: horizontal pos of the tabs container inside the notebook
    // tab_x:  horizontal pos of the selected tab inside the tabs container

    int tabs_x, tab_x = 0, tab_is_mapped;
    res = this->Script("concat [winfo x %s] [winfo ismapped %s]", 
                       this->TabsFrame->GetWidgetName(),
                       page->TabFrame->GetWidgetName());
    sscanf(res, "%d %d", &tabs_x, &tab_is_mapped);

    // For some reasons, even if the tabs container is mapped and its position
    // can be queried, the position of the tab inside the container can not
    // be queried if it is not mapped. In that case use a slower method by
    // computing the size of previous slaves inside the container. Hopefully
    // this will be done only at startup, not in later interaction.
    // UPDATE: if it is mapped, it does not seem to be reliable, so let's use
    // the slowest method anyway.

#if 0
    if (tab_is_mapped)
      {
      this->Script("winfo x %s", page->TabFrame->GetWidgetName());
      tab_x = vtkKWObject::GetIntegerResult(this->Application);
      }
    else
#endif
      {
      vtkKWTkUtilities::GetPackSlaveHorizontalPosition(
        this->Application->GetMainInterp(),
        this->TabsFrame->GetWidgetName(),
        page->TabFrame->GetWidgetName(),
        &tab_x);
      }

    // tab_width: width of the selected tab
    // body_y:    vertical pos of the body (i.e. the page itself)

    int tab_width, body_y;
    res = this->Script("concat [winfo reqwidth %s] [winfo y %s]", 
                       page->TabFrame->GetWidgetName(),
                       this->Body->GetWidgetName());
    sscanf(res, "%d %d", &tab_width, &body_y);

    // Now basically the mask:
    //   - starts right after the inner left border of the selected tab 
    //     horizontally, and at the begginning of the body top border vertically
    //   - ends right before the inner right border of the selected tab 
    //     horizontally, and at the end of the body top border vertically

    int x0 = tabs_x + tab_x + VTK_KW_NB_TAB_BD;
    int y0 = body_y;
    int w0 = tab_width - VTK_KW_NB_TAB_BD * 2;
    int h0 = VTK_KW_NB_TAB_BD;

    // Now this is a bit trickier for Windows. If the tab frame and the 
    // label have the same background color, the "relief" of the tab is
    // actually drawn by Tk so that the right half of the left border is
    // the same color as the tab background itself. Thus, let's remove
    // this half-size.

#if _WIN32
    int tr, tg, tb, lr, lg, lb;
    page->TabFrame->GetBackgroundColor(&tr, &tg, &tb);
    page->Label->GetBackgroundColor(&lr, &lg, &lb);
    if (tr == lr && tg == lg && tb == lb)
      {
      x0 -= (int)(VTK_KW_NB_TAB_BD * 0.5); 
      w0 += (int)(VTK_KW_NB_TAB_BD * 0.5);
      }
#endif

    this->Script("place %s -x %d -y %d -width %d -height %d",
                 this->Mask->GetWidgetName(), x0, y0, w0, h0);
    }
}

//------------------------------------------------------------------------------
void vtkKWNotebook::ScheduleResize()
{  
  if (this->Expanding)
    {
    return;
    }
  this->Expanding = 1;

  // We need a "catch" here since in the "idle time" we could have been
  // destroyed.

  this->Script("after idle {catch {%s Resize}}", this->GetTclName());
}

//------------------------------------------------------------------------------
void vtkKWNotebook::Resize()
{
  if (!this->IsCreated())
    {
    return;
    }

  const char *res;

  // First update the body position, because we need to know where the page
  // location is vertically.

  this->UpdateBodyPosition();

  // We need to compute the height required by both the page and the
  // tabs. First get the width and height of the current page, 
  // and the width of the tabs container

  int height, width, tabs_width;

  res = this->Script(
    "concat [winfo reqheight %s] [winfo reqwidth %s] [winfo reqwidth %s]", 
    this->Body->GetWidgetName(),
    this->Body->GetWidgetName(),
    this->TabsFrame->GetWidgetName());
  sscanf(res, "%d %d %d", &height, &width, &tabs_width);

  // If the tabs container is visible, add the vertical space required for 
  // the tabs to get the total required height (use the body position since
  // the tabs are not completely visible)

  if (this->AreTabsVisible())
    {
    this->Script("winfo y %s", this->Body->GetWidgetName());
    height += vtkKWObject::GetIntegerResult(this->Application);
    }

  // Now if the tabs require more width than the page, use the tabs width
  // as the new width
  
  if (tabs_width > width)
    {
    width = tabs_width;
    }
  
  // If the new size is smaller than minimum size, use the minimum size

  if (width < this->MinimumWidth)
    {
    width = this->MinimumWidth;
    }
  if (height < this->MinimumHeight)
    {
    height = this->MinimumHeight;
    }

  // Resize the whole notebook to fit the page and the tabs

  this->Script("%s configure -width %d -height %d",
               this->GetWidgetName(), width, height);

  // Update the mask position

  this->UpdateMaskPosition();

  this->Expanding = 0;
}

//----------------------------------------------------------------------------
void vtkKWNotebook::SetShowIcons(int arg)
{
  if (this->ShowIcons == arg)
    {
    return;
    }
  this->ShowIcons = arg;
  this->Modified();

  if (!this->IsCreated())
    {
    return;
    }

  // Pack or unpack the icons if needed

  ostrstream cmd;

  vtkKWNotebook::Page *page = NULL;
  vtkKWNotebook::PagesContainerIterator *it = this->Pages->NewIterator();
  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    if (it->GetData(page) == VTK_OK && page->Icon)
      {
      if (this->ShowIcons)
        {
        cmd << "pack " << page->ImageLabel->GetWidgetName() 
                << " -side left -before " << page->Label->GetWidgetName() 
                << endl;
        }
      else
        {
        cmd << "pack forget " << page->ImageLabel->GetWidgetName() << endl;
        }
      }
    it->GoToNextItem();
    }
  it->Delete();

  cmd << ends;
  this->Script(cmd.str());
  cmd.rdbuf()->freeze(0);

  this->ScheduleResize();
}

//----------------------------------------------------------------------------
void vtkKWNotebook::SetMinimumWidth(int arg)
{
  if (this->MinimumWidth == arg)
    {
    return;
    }
  this->MinimumWidth = arg;
  this->Modified();

  if (this->IsCreated())
    {
    this->ScheduleResize();
    }
}

//----------------------------------------------------------------------------
void vtkKWNotebook::SetMinimumHeight(int arg)
{
  if (this->MinimumHeight == arg)
    {
    return;
    }
  this->MinimumHeight = arg;
  this->Modified();

  if (this->IsCreated())
    {
    this->ScheduleResize();
    }
}

//----------------------------------------------------------------------------
void vtkKWNotebook::SetAlwaysShowTabs(int arg)
{
  if (this->AlwaysShowTabs == arg)
    {
    return;
    }
  this->AlwaysShowTabs = arg;
  this->Modified();

  if (this->IsCreated())
    {
    this->ScheduleResize();
    }
}

//----------------------------------------------------------------------------
void vtkKWNotebook::SetShowAllPagesWithSameTag(int arg)
{
  if (this->ShowAllPagesWithSameTag == arg)
    {
    return;
    }

  this->ShowAllPagesWithSameTag = arg;
  this->Modified();

  this->ConstrainVisiblePages();
}

//----------------------------------------------------------------------------
void vtkKWNotebook::SetShowOnlyPagesWithSameTag(int arg)
{
  if (this->ShowOnlyPagesWithSameTag == arg)
    {
    return;
    }

  this->ShowOnlyPagesWithSameTag = arg;
  this->Modified();

  this->ConstrainVisiblePages();
}

//----------------------------------------------------------------------------
void vtkKWNotebook::SetShowOnlyMostRecentPages(int arg)
{
  if (this->ShowOnlyMostRecentPages == arg)
    {
    return;
    }

  this->ShowOnlyMostRecentPages = arg;
  this->Modified();

  // Empty the most recent pages buffer

  this->MostRecentPages->RemoveAllItems();

  // If we are enabling this feature, put the current tabs in the most
  // recent pages

  if (this->ShowOnlyMostRecentPages && this->IsCreated())
    {
    char **slaves = 0;
    int nb_slaves = vtkKWTkUtilities::GetSlaves(
      this->Application->GetMainInterp(),
      this->TabsFrame->GetWidgetName(), &slaves);

    // Iterate over each slave and find the corresponding page

    if (nb_slaves)
      {
      int i;
      for (i = nb_slaves - 1; i >= 0; i--)
        {
        vtkKWNotebook::Page *page = NULL;
        vtkKWNotebook::PagesContainerIterator *it = this->Pages->NewIterator();
        it->InitTraversal();
        while (!it->IsDoneWithTraversal())
          {
          if (it->GetData(page) == VTK_OK && 
              !strcmp(slaves[i], page->TabFrame->GetWidgetName()))
            {
            this->EnqueueMostRecentPage(page);
            break;
            }
          it->GoToNextItem();
          }
        it->Delete();

        delete [] slaves[i];
        }
      
      delete [] slaves;
      }
    }

  // Make sure the constraint are satisfied

  this->ConstrainVisiblePages();
}

//----------------------------------------------------------------------------
void vtkKWNotebook::ConstrainVisiblePages()
{
  if (!this->IsCreated())
    {
    return;
    }

  // Show only the pages with the same tag (as the current selected page,
  // except if it is pinned)

  if (this->ShowOnlyPagesWithSameTag)
    {
    vtkKWNotebook::Page *selected_page = this->GetPage(this->CurrentId);
    if (selected_page && !selected_page->Pinned)
      {
      this->HidePagesNotMatchingTag(selected_page->Tag);
      }
    }

  // Show all pages with the same tag

  if (this->ShowAllPagesWithSameTag)
    {
    vtkKWNotebook::Page *page = NULL;
    vtkKWNotebook::PagesContainerIterator *it = this->Pages->NewIterator();

    it->InitTraversal();
    while (!it->IsDoneWithTraversal())
      {
      if (it->GetData(page) == VTK_OK && page->Visibility)
        {
        this->ShowPagesMatchingTag(page->Tag);
        }
      it->GoToNextItem();
      }
    it->Delete();
    }

  // Show only the most recent pages

  if (this->ShowOnlyMostRecentPages && 
      this->NumberOfMostRecentPages > 0 && 
      this->MostRecentPages->GetNumberOfItems() > this->NumberOfMostRecentPages)
    {
    int diff = 
      this->MostRecentPages->GetNumberOfItems() - this->NumberOfMostRecentPages;

    vtkIdType key;
    vtkKWNotebook::Page *page = NULL;
    vtkKWNotebook::PagesContainerIterator *it = 
      this->MostRecentPages->NewIterator();

    // There are move pages than allowed, try to remove some of them

    it->GoToLastItem();
    while (diff && !it->IsDoneWithTraversal())
      {
      int res = (it->GetKey(key) == VTK_OK && it->GetData(page) == VTK_OK);
      it->GoToPreviousItem();
      if (res && 
          this->CanBeHidden(page) &&
          this->MostRecentPages->RemoveItem(key) == VTK_OK)
        {
        this->HidePage(page);
        diff--;
        }
      }
    it->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkKWNotebook::SetPagesCanBePinned(int arg)
{
  if (this->PagesCanBePinned == arg)
    {
    return;
    }
  this->PagesCanBePinned = arg;
  this->Modified();

  if (this->IsCreated() && !this->PagesCanBePinned)
    {
    vtkKWNotebook::Page *page = NULL;
    vtkKWNotebook::PagesContainerIterator *it = this->Pages->NewIterator();

    it->InitTraversal();
    while (!it->IsDoneWithTraversal())
      {
      if (it->GetData(page) == VTK_OK && page->Pinned)
        {
        this->UnpinPage(page);
        }
      it->GoToNextItem();
      }
    it->Delete();
    }
}

//----------------------------------------------------------------------------
int vtkKWNotebook::GetPageIdContainingCoordinatesInTab(int x, int y)
{
  if (!this->IsCreated())
    {
    return -1;
    }

  int found = -1;
  vtkKWNotebook::Page *page = NULL;
  vtkKWNotebook::PagesContainerIterator *it = this->Pages->NewIterator();

  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    if (it->GetData(page) == VTK_OK && 
        page->Visibility &&
        page->TabFrame->IsCreated() &&
        vtkKWTkUtilities::ContainsCoordinates(
          this->GetApplication()->GetMainInterp(),
          page->TabFrame->GetWidgetName(),
          x, y))
      {
      found = page->Id;
      break;
      }
    it->GoToNextItem();
    }
  it->Delete();

  return found;
}

//----------------------------------------------------------------------------
void vtkKWNotebook::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "MinimumHeight: " << this->GetMinimumHeight() << endl;
  os << indent << "MinimumWidth: " << this->GetMinimumWidth() << endl;
  os << indent << "NumberOfPages: " << this->GetNumberOfPages() << endl;
  os << indent << "AlwaysShowTabs: " << (this->AlwaysShowTabs ? "On" : "Off")
     << endl;
  os << indent << "ShowAllPagesWithSameTag: " 
     << (this->ShowAllPagesWithSameTag ? "On" : "Off") << endl;
  os << indent << "ShowOnlyPagesWithSameTag: " 
     << (this->ShowOnlyPagesWithSameTag ? "On" : "Off") << endl;
  os << indent << "ShowOnlyMostRecentPages: " 
     << (this->ShowOnlyMostRecentPages ? "On" : "Off") << endl;
  os << indent << "NumberOfMostRecentPages: " 
     << this->GetNumberOfMostRecentPages() << endl;
  os << indent << "ShowIcons: " << (this->ShowIcons ? "On" : "Off")
     << endl;
  os << indent << "PagesCanBePinned: " << (this->PagesCanBePinned ? "On" : "Off")
     << endl;
}
