/*=========================================================================

  Module:    $RCSfile: vtkKWListSelectOrder.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWListSelectOrder.h"

#include "vtkKWApplication.h"
#include "vtkKWIcon.h"
#include "vtkKWLabel.h"
#include "vtkKWListBox.h"
#include "vtkObjectFactory.h"
#include "vtkKWPushButton.h"
#include "vtkKWFrame.h"

#include <vtkstd/string>
#include <vtkstd/vector>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWListSelectOrder );
vtkCxxRevisionMacro(vtkKWListSelectOrder, "$Revision: 1.7 $");

//----------------------------------------------------------------------------
vtkKWListSelectOrder::vtkKWListSelectOrder()
{
  this->SourceList = vtkKWListBox::New();
  this->FinalList = vtkKWListBox::New();

  this->AddButton = vtkKWPushButton::New();
  this->AddAllButton = vtkKWPushButton::New();
  this->RemoveButton = vtkKWPushButton::New();
  this->RemoveAllButton = vtkKWPushButton::New();
  this->UpButton = vtkKWPushButton::New();
  this->DownButton = vtkKWPushButton::New();
  this->EllipsisCommand = 0;
}

//----------------------------------------------------------------------------
vtkKWListSelectOrder::~vtkKWListSelectOrder()
{
  this->SourceList->Delete();
  this->FinalList->Delete();

  this->AddButton->Delete();
  this->AddAllButton->Delete();
  this->RemoveButton->Delete();
  this->RemoveAllButton->Delete();
  this->UpButton->Delete();
  this->DownButton->Delete();

  this->SetEllipsisCommand(0);
}

//----------------------------------------------------------------------------
void vtkKWListSelectOrder::Create(vtkKWApplication *app, const char *args)
{
  const char *wname;

  // Set the application

  if (this->IsCreated())
    {
    vtkErrorMacro("Entry already created");
    return;
    }

  this->SetApplication(app);

  // create the top level
  wname = this->GetWidgetName();

  this->Script("frame %s %s",wname, (args?args:""));
  vtkKWFrame* frame;
  this->SourceList->SetParent(this);
  this->SourceList->Create(app, 0);
  this->SourceList->GetListbox()->Configure("-selectmode multiple");
  this->Script("pack %s -side left -expand true -fill both",
    this->SourceList->GetWidgetName());
  frame = vtkKWFrame::New();
  frame->SetParent(this);
  frame->Create(app, 0);
  this->AddButton->SetParent(frame->GetFrame());
  this->AddButton->Create(app,0);
  this->AddButton->SetLabel("Add");
  this->AddButton->SetCommand(this, "AddCallback");
  this->AddAllButton->SetParent(frame->GetFrame());
  this->AddAllButton->Create(app,0);
  this->AddAllButton->SetLabel("Add All");
  this->AddAllButton->SetCommand(this, "AddAllCallback");
  this->RemoveButton->SetParent(frame->GetFrame());
  this->RemoveButton->Create(app,0);
  this->RemoveButton->SetLabel("Remove");
  this->RemoveButton->SetCommand(this, "RemoveCallback");
  this->RemoveAllButton->SetParent(frame->GetFrame());
  this->RemoveAllButton->Create(app,0);
  this->RemoveAllButton->SetLabel("RemoveAll");
  this->RemoveAllButton->SetCommand(this, "RemoveAllCallback");
  this->Script("pack %s %s %s %s -side top -fill x",
    this->AddButton->GetWidgetName(),
    this->AddAllButton->GetWidgetName(),
    this->RemoveButton->GetWidgetName(),
    this->RemoveAllButton->GetWidgetName());
  this->Script("pack %s -side left -expand false -fill y",
    frame->GetWidgetName());
  frame->Delete();
  frame = vtkKWFrame::New();
  frame->SetParent(this);
  frame->Create(app, 0);
  this->FinalList->SetParent(frame->GetFrame());
  this->FinalList->Create(app, 0);
  this->FinalList->GetListbox()->Configure("-selectmode multiple");
  this->Script("pack %s -side top -expand true -fill both",
    this->FinalList->GetWidgetName());
  vtkKWFrame* btframe = vtkKWFrame::New();
  btframe->SetParent(frame->GetFrame());
  btframe->Create(app, 0);

  this->UpButton->SetParent(btframe->GetFrame());
  this->UpButton->Create(app, 0);
  this->UpButton->SetLabel("Up");
  this->UpButton->SetCommand(this, "UpCallback");
  this->DownButton->SetParent(btframe->GetFrame());
  this->DownButton->Create(app, 0);
  this->DownButton->SetLabel("Down");
  this->DownButton->SetCommand(this, "DownCallback");
  this->Script("pack %s %s -side left -fill x",
    this->UpButton->GetWidgetName(),
    this->DownButton->GetWidgetName());
  this->Script("pack %s -side top -expand false -fill x",
    btframe->GetWidgetName());
  btframe->Delete();
  
  this->Script("pack %s -side left -expand true -fill both",
    frame->GetWidgetName());
  frame->Delete();
  this->DisplayEllipsis();
}

//----------------------------------------------------------------------------
void vtkKWListSelectOrder::AddElement(vtkKWListBox* l1, vtkKWListBox* l2, 
  const char* element, int force)
{
  int idx = l2->GetItemIndex(element);
  if ( idx >= 0 )
    {
    if ( force )
      {
      l2->DeleteRange(idx, idx);
      l1->AppendUnique(element);
      }
    }
  else
    {
    l1->AppendUnique(element);
    }
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkKWListSelectOrder::AddSourceElement(const char* element, int force /* = 0 */)
{
  this->RemoveEllipsis();
  this->AddElement(this->SourceList, this->FinalList, element, force);
  this->DisplayEllipsis();
}

//----------------------------------------------------------------------------
void vtkKWListSelectOrder::AddFinalElement(const char* element, int force /* = 0 */)
{
  this->RemoveEllipsis();
  this->AddElement(this->FinalList, this->SourceList, element, force);
  this->DisplayEllipsis();
}

//----------------------------------------------------------------------------
void vtkKWListSelectOrder::AddCallback()
{
  this->RemoveEllipsis();
  this->MoveSelectedList(this->SourceList, this->FinalList);
  this->DisplayEllipsis();
}

//----------------------------------------------------------------------------
void vtkKWListSelectOrder::AddAllCallback()
{
  this->RemoveEllipsis();
  this->MoveWholeList(this->SourceList, this->FinalList);
  this->DisplayEllipsis();
}

//----------------------------------------------------------------------------
void vtkKWListSelectOrder::RemoveCallback() 
{
  this->RemoveEllipsis();
  this->MoveSelectedList(this->FinalList, this->SourceList);
  this->DisplayEllipsis();
}

//----------------------------------------------------------------------------
void vtkKWListSelectOrder::RemoveAllCallback()
{
  this->RemoveEllipsis();
  this->MoveWholeList(this->FinalList, this->SourceList);
  this->DisplayEllipsis();
}

//----------------------------------------------------------------------------
void vtkKWListSelectOrder::ShiftItems(vtkKWListBox* l1, int down)
{
  const char* list = this->Script("lsort -integer %s [ %s curselection ]", 
    down?"-decreasing":"",
    l1->GetListbox()->GetWidgetName());
  char* selection = new char[ strlen(list) + 1 ];
  strcpy(selection, list);
  int idx = -1;
  int size = l1->GetNumberOfItems();
  istrstream sel(selection);
  vtkstd::string item;
  while(sel >> idx)
    {
    if ( idx < 0 )
      {
      break;
      }
    int newidx = idx - 1;
    if ( down )
      {
      newidx = idx + 1;
      }
    if ( newidx >= 0 && newidx < size )
      {
      item = l1->GetItem(idx);
      l1->DeleteRange(idx, idx);
      l1->InsertEntry(newidx, item.c_str());
      this->Script("%s selection set %d %d",
        l1->GetListbox()->GetWidgetName(), newidx, newidx);
      }

    idx = -1;
    }
  delete [] selection;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkKWListSelectOrder::UpCallback()
{
  this->ShiftItems(this->FinalList, 0);
}

//----------------------------------------------------------------------------
void vtkKWListSelectOrder::DownCallback()
{
  this->ShiftItems(this->FinalList, 1);
}

//----------------------------------------------------------------------------
void vtkKWListSelectOrder::MoveWholeList(vtkKWListBox* l1, vtkKWListBox* l2)
{
  this->Script("%s selection set 0 end",
    l1->GetListbox()->GetWidgetName());
  this->MoveSelectedList(l1, l2);
}

//----------------------------------------------------------------------------
void vtkKWListSelectOrder::MoveSelectedList(vtkKWListBox* l1, vtkKWListBox* l2)
{
  const char* cselected = this->Script("%s curselection", 
    l1->GetListbox()->GetWidgetName());
  this->MoveList(l1, l2, cselected);
}

//----------------------------------------------------------------------------
void vtkKWListSelectOrder::MoveList(vtkKWListBox* l1, vtkKWListBox* l2, 
  const char* list)
{
  char* selection = new char[ strlen(list) + 1 ];
  strcpy(selection, list);
  int idx = -1;
  vtkstd::string str;
  vtkstd::vector<int> vec;
  istrstream sel(selection);
  while(sel >> idx)
    {
    if ( idx < 0 )
      {
      break;
      }
    str = l1->GetItem(idx);
    l2->AppendUnique(str.c_str());
    vec.push_back(idx);
    idx = -1;
    }
  while( vec.size() )
    {
    idx = vec[vec.size() - 1];
    l1->DeleteRange(idx,idx);
    vec.erase(vec.end()-1);
    }
  delete [] selection;
  
  this->Modified();
}

//----------------------------------------------------------------------------
int vtkKWListSelectOrder::GetNumberOfElementsOnSourceList()
{
  if ( this->EllipsisDisplayed )
    {
    return 0;
    }
  return this->SourceList->GetNumberOfItems();
}

//----------------------------------------------------------------------------
int vtkKWListSelectOrder::GetNumberOfElementsOnFinalList()
{
  return this->FinalList->GetNumberOfItems();
}

//----------------------------------------------------------------------------
const char* vtkKWListSelectOrder::GetElementFromSourceList(int idx)
{
  if ( this->EllipsisDisplayed )
    {
    return 0;
    }
  return this->SourceList->GetItem(idx);
}

//----------------------------------------------------------------------------
const char* vtkKWListSelectOrder::GetElementFromFinalList(int idx)
{
  return this->FinalList->GetItem(idx);
}

//----------------------------------------------------------------------------
int vtkKWListSelectOrder::GetElementIndexFromSourceList(const char* element)
{
  if ( this->EllipsisDisplayed )
    {
    return -1;
    }
  return this->SourceList->GetItemIndex(element);
}

//----------------------------------------------------------------------------
int vtkKWListSelectOrder::GetElementIndexFromFinalList(const char* element)
{
  return this->FinalList->GetItemIndex(element);
}

//----------------------------------------------------------------------------
void vtkKWListSelectOrder::RemoveItemsFromSourceList()
{
  this->SourceList->DeleteAll();
  this->Modified();
  this->DisplayEllipsis();
}

//----------------------------------------------------------------------------
void vtkKWListSelectOrder::RemoveItemsFromFinalList()
{
  this->FinalList->DeleteAll();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkKWListSelectOrder::DisplayEllipsis()
{
  if ( this->SourceList->GetNumberOfItems() > 0 )
    {
    return;
    }
  this->SourceList->InsertEntry(0, "...");
  this->SourceList->GetListbox()->SetBind(this, "<Double-1>", "EllipsisCallback");
  this->EllipsisDisplayed = 1;
}

//----------------------------------------------------------------------------
void vtkKWListSelectOrder::RemoveEllipsis()
{
  if ( !this->EllipsisDisplayed )
    {
    return;
    }
  this->SourceList->DeleteAll();
  this->SourceList->GetListbox()->UnsetBind("<Double-1>");
  this->EllipsisDisplayed = 0;
}

//----------------------------------------------------------------------------
void vtkKWListSelectOrder::EllipsisCallback()
{
  if ( !this->EllipsisCommand )
    {
    return;
    }
  this->Script(this->EllipsisCommand);
}

//----------------------------------------------------------------------------
void vtkKWListSelectOrder::SetEllipsisCommand(vtkKWObject* obj, const char* command)
{
  ostrstream str;
  str << obj->GetTclName() << " " << command << ends;
  this->SetEllipsisCommand(str.str());
  str.rdbuf()->freeze(0);
}

//-----------------------------------------------------------------------------
void vtkKWListSelectOrder::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->PropagateEnableState(this->SourceList);
  this->PropagateEnableState(this->FinalList);

  this->PropagateEnableState(this->AddButton);
  this->PropagateEnableState(this->AddAllButton);
  this->PropagateEnableState(this->RemoveButton);
  this->PropagateEnableState(this->RemoveAllButton);
  this->PropagateEnableState(this->UpButton);
  this->PropagateEnableState(this->DownButton);
}

//----------------------------------------------------------------------------
void vtkKWListSelectOrder::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  
  os << indent << "EllipsisCommand: "
     << (this->EllipsisCommand ? this->EllipsisCommand : "(none)") << endl;
}

