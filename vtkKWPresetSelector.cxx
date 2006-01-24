/*=========================================================================

  Module:    $RCSfile: vtkKWPresetSelector.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkKWPresetSelector.h"

#include "vtkKWApplication.h"
#include "vtkImageData.h"
#include "vtkImageResample.h"
#include "vtkKWBalloonHelpManager.h"
#include "vtkKWIcon.h"
#include "vtkKWLabel.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWPushButton.h"
#include "vtkKWPushButtonSet.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkWindowToImageFilter.h"
#include "vtkImagePermute.h"
#include "vtkImageClip.h"
#include "vtkKWIcon.h"

#include <vtksys/SystemTools.hxx>
#include <vtksys/stl/vector>
#include <vtksys/stl/string>
#include <vtksys/stl/map>

#include <time.h>

int vtkKWPresetSelector::SelectPreviousButtonId = 0;
int vtkKWPresetSelector::SelectNextButtonId     = 1;
int vtkKWPresetSelector::AddButtonId            = 2;
int vtkKWPresetSelector::ApplyButtonId          = 3;
int vtkKWPresetSelector::UpdateButtonId         = 4;
int vtkKWPresetSelector::RemoveButtonId         = 5;
int vtkKWPresetSelector::LocateButtonId         = 6;
int vtkKWPresetSelector::EmailButtonId          = 7;

const char *vtkKWPresetSelector::IdColumnName        = "Id";
const char *vtkKWPresetSelector::ThumbnailColumnName = "Image";
const char *vtkKWPresetSelector::GroupColumnName     = "Group";
const char *vtkKWPresetSelector::CommentColumnName   = "Comment";

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWPresetSelector);
vtkCxxRevisionMacro(vtkKWPresetSelector, "$Revision: 1.33 $");

//----------------------------------------------------------------------------
class vtkKWPresetSelectorInternals
{
public:
  
  class UserSlotType
  {
  public:
    double DoubleValue;
    int IntValue;
    vtksys_stl::string StringValue;
    void *PointerValue;
  };

  typedef vtksys_stl::map<vtksys_stl::string, UserSlotType> UserSlotPoolType;
  typedef vtksys_stl::map<vtksys_stl::string, UserSlotType>::iterator UserSlotPoolIterator;

  class PresetNode
  {
  public:
    int Id;
    UserSlotPoolType UserSlotPool;
  };

  static int PresetNodeCounter;

  typedef vtksys_stl::vector<PresetNode*> PresetPoolType;
  typedef vtksys_stl::vector<PresetNode*>::iterator PresetPoolIterator;

  PresetPoolType PresetPool;

  PresetPoolIterator GetPresetNode(int id);

  vtksys_stl::string GroupSlotName;
  vtksys_stl::string CommentSlotName;
  vtksys_stl::string FileNameSlotName;
  vtksys_stl::string CreationTimeSlotName;
  vtksys_stl::string ThumbnailSlotName;
  vtksys_stl::string ScreenshotSlotName;
};

int vtkKWPresetSelectorInternals::PresetNodeCounter = 0;

//---------------------------------------------------------------------------
vtkKWPresetSelectorInternals::PresetPoolIterator 
vtkKWPresetSelectorInternals::GetPresetNode(int id)
{
  vtkKWPresetSelectorInternals::PresetPoolIterator it = 
    this->PresetPool.begin();
  vtkKWPresetSelectorInternals::PresetPoolIterator end = 
    this->PresetPool.end();
  for (; it != end; ++it)
    {
    if ((*it)->Id == id)
      {
      return it;
      }
    }
  return end;
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::DeAllocatePreset(int id)
{
  vtkKWIcon *thumbnail = this->GetPresetThumbnail(id);
  if (thumbnail)
    {
    thumbnail->Delete();
    }
  vtkKWIcon *screenshot = this->GetPresetScreenshot(id);
  if (screenshot)
    {
    screenshot->Delete();
    }
}

//----------------------------------------------------------------------------
vtkKWPresetSelector::vtkKWPresetSelector()
{
  this->Internals = new vtkKWPresetSelectorInternals;

  this->Internals->GroupSlotName = "DefaultGroupSlot";
  this->Internals->CommentSlotName = "DefaultCommentSlot";
  this->Internals->FileNameSlotName = "DefaultFileNameSlot";
  this->Internals->CreationTimeSlotName = "DefaultCreationTimeSlot";
  this->Internals->ThumbnailSlotName = "DefaultThumbnailSlot";
  this->Internals->ScreenshotSlotName = "DefaultScreenshotSlot";

  this->PresetAddCommand        = NULL;
  this->PresetUpdateCommand     = NULL;
  this->PresetApplyCommand      = NULL;
  this->PresetRemoveCommand     = NULL;
  this->PresetHasChangedCommand = NULL;

  this->PresetList              = NULL;
  this->PresetControlFrame      = NULL;
  this->PresetButtons           = NULL;

  this->ApplyPresetOnSelection = 1;
  this->SelectSpinButtonsVisibility = 1;
  this->LocateButtonVisibility = 0;
  this->RemoveButtonVisibility = 1;
  this->EmailButtonVisibility = 0;

  this->ThumbnailSize = 32;
  this->ScreenshotSize = 144;
  this->PromptBeforeRemovePreset = 1;

  this->GroupFilter = NULL;
}

//----------------------------------------------------------------------------
vtkKWPresetSelector::~vtkKWPresetSelector()
{
  if (this->PresetList)
    {
    this->PresetList->Delete();
    this->PresetList = NULL;
    }

  if (this->PresetControlFrame)
    {
    this->PresetControlFrame->Delete();
    this->PresetControlFrame = NULL;
    }

  if (this->PresetButtons)
    {
    this->PresetButtons->Delete();
    this->PresetButtons = NULL;
    }

  if (this->PresetAddCommand)
    {
    delete [] this->PresetAddCommand;
    this->PresetAddCommand = NULL;
    }

  if (this->PresetUpdateCommand)
    {
    delete [] this->PresetUpdateCommand;
    this->PresetUpdateCommand = NULL;
    }

  if (this->PresetApplyCommand)
    {
    delete [] this->PresetApplyCommand;
    this->PresetApplyCommand = NULL;
    }

  if (this->PresetRemoveCommand)
    {
    delete [] this->PresetRemoveCommand;
    this->PresetRemoveCommand = NULL;
    }

  if (this->PresetHasChangedCommand)
    {
    delete [] this->PresetHasChangedCommand;
    this->PresetHasChangedCommand = NULL;
    }

  // Remove all presets

  this->DeleteAllPresets();

  // Delete our pool

  delete this->Internals;
  this->Internals = NULL;

  this->SetGroupFilter(NULL);
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::Create()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::Create();

  // --------------------------------------------------------------
  // Preset : preset list

  if (!this->PresetList)
    {
    this->PresetList = vtkKWMultiColumnListWithScrollbars::New();
    }

  this->PresetList->SetParent(this);
  this->PresetList->Create();
  this->PresetList->HorizontalScrollbarVisibilityOff();

  this->Script(
    "pack %s -side top -anchor nw -fill both -expand t -padx 2 -pady 2",
    this->PresetList->GetWidgetName());

  vtkKWMultiColumnList *list = this->PresetList->GetWidget();
  if (this->ApplyPresetOnSelection)
    {
    list->SetSelectionModeToBrowse();
    }
  else
    {
    list->SetSelectionModeToExtended();
    }
  list->SetSelectionCommand(
    this, "PresetSelectionCallback");
  list->SetSelectionChangedCommand(
    this, "PresetSelectionChangedCallback");
  list->SetPotentialCellColorsChangedCommand(
    list, "RefreshColorsOfAllCellsWithWindowCommand");
  // list->SetSelectionBackgroundColor(0.988, 1.0, 0.725);
  // list->ColumnLabelsVisibilityOff();
  list->ColumnSeparatorsVisibilityOn();
  list->SetEditStartCommand(this, "PresetCellEditStartCallback");
  list->SetEditEndCommand(this, "PresetCellEditEndCallback");
  list->SetCellUpdatedCommand(this, "PresetCellUpdatedCallback");

  this->CreateColumns();

  // --------------------------------------------------------------
  // Preset : control frame

  if (!this->PresetControlFrame)
    {
    this->PresetControlFrame = vtkKWFrame::New();
    }

  this->PresetControlFrame->SetParent(this);
  this->PresetControlFrame->Create();

  this->Script("pack %s -side top -anchor nw -fill both -expand f",
               this->PresetControlFrame->GetWidgetName());

  // --------------------------------------------------------------
  // Preset : buttons

  if (!this->PresetButtons)
    {
    this->PresetButtons = vtkKWPushButtonSet::New();
    }

  this->PresetButtons->SetParent(this->PresetControlFrame);
  this->PresetButtons->PackHorizontallyOn();
  this->PresetButtons->SetWidgetsPadX(2);
  this->PresetButtons->SetWidgetsPadY(2);
  this->PresetButtons->SetWidgetsInternalPadY(1);
  this->PresetButtons->ExpandWidgetsOn();
  this->PresetButtons->Create();

  this->CreatePresetButtons();

  this->SetDefaultHelpStrings();

  // Pack

  this->Pack();

  // Update enable state

  this->Update();
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::CreatePresetButtons()
{
  if (!this->PresetButtons)
    {
    return;
    }

  vtkKWPushButton *pb = NULL;

  // select previous preset

  pb = this->PresetButtons->AddWidget(
    vtkKWPresetSelector::SelectPreviousButtonId);
  pb->SetImageToPredefinedIcon(vtkKWIcon::IconSpinUp);
  pb->SetHeight(12);
  pb->SetCommand(this, "SelectPreviousPreset");

  // select next preset

  pb = this->PresetButtons->AddWidget(
    vtkKWPresetSelector::SelectNextButtonId);
  pb->SetImageToPredefinedIcon(vtkKWIcon::IconSpinDown);
  pb->SetHeight(12);
  pb->SetCommand(this, "SelectNextPreset");

  // add preset

  pb = this->PresetButtons->AddWidget(vtkKWPresetSelector::AddButtonId);
  pb->SetImageToPredefinedIcon(vtkKWIcon::IconPresetAdd);
  pb->SetCommand(this, "PresetAddCallback");

  // apply preset

  pb = this->PresetButtons->AddWidget(vtkKWPresetSelector::ApplyButtonId);
  pb->SetImageToPredefinedIcon(vtkKWIcon::IconPresetApply);
  pb->SetCommand(this, "PresetApplyCallback");

  // update preset

  pb = this->PresetButtons->AddWidget(vtkKWPresetSelector::UpdateButtonId);
  pb->SetImageToPredefinedIcon(vtkKWIcon::IconPresetUpdate);
  pb->SetCommand(this, "PresetUpdateCallback");

  // remove preset

  pb = this->PresetButtons->AddWidget(vtkKWPresetSelector::RemoveButtonId);
  pb->SetImageToPredefinedIcon(vtkKWIcon::IconPresetDelete);
  pb->SetCommand(this, "PresetRemoveCallback");

  // locate preset

  pb = this->PresetButtons->AddWidget(vtkKWPresetSelector::LocateButtonId);
  pb->SetImageToPredefinedIcon(vtkKWIcon::IconPresetLocate);
  pb->SetCommand(this, "PresetLocateCallback");

  // email preset

  pb = this->PresetButtons->AddWidget(vtkKWPresetSelector::EmailButtonId);
  pb->SetImageToPredefinedIcon(vtkKWIcon::IconPresetEmail);
  pb->SetCommand(this, "PresetEmailCallback");
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::Pack()
{
  if (!this->IsCreated())
    {
    return;
    }

  if (this->PresetControlFrame)
    {
    this->PresetControlFrame->UnpackChildren();
    }

  if (this->PresetButtons && this->PresetButtons->IsCreated())
    {
    this->Script("pack %s -side top -anchor nw -fill x -expand t",
                 this->PresetButtons->GetWidgetName());
    }
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::CreateColumns()
{
  vtkKWMultiColumnList *list = this->PresetList->GetWidget();

  int col;

  // We need that column to retrieve the Id

  col = list->AddColumn(vtkKWPresetSelector::IdColumnName);
  list->SetColumnName(col, vtkKWPresetSelector::IdColumnName);
  list->ColumnVisibilityOff(col);

  // Thumbnail

  col = list->AddColumn(vtkKWPresetSelector::ThumbnailColumnName);
  list->SetColumnName(col, vtkKWPresetSelector::ThumbnailColumnName);
  list->SetColumnWidth(col, -this->ThumbnailSize);
  list->SetColumnResizable(col, 0);
  list->SetColumnStretchable(col, 0);
  list->SetColumnEditable(col, 0);
  list->SetColumnSortModeToReal(col);
  list->SetColumnFormatCommandToEmptyOutput(col);
  list->ColumnVisibilityOff(col);

  // Group

  col = list->AddColumn(vtkKWPresetSelector::GroupColumnName);
  list->SetColumnName(col, vtkKWPresetSelector::GroupColumnName);
  list->SetColumnResizable(col, 1);
  list->SetColumnStretchable(col, 0);
  list->SetColumnEditable(col, 0);
  list->ColumnVisibilityOff(col);

  // Comment

  col = list->AddColumn(vtkKWPresetSelector::CommentColumnName);
  list->SetColumnName(col, vtkKWPresetSelector::CommentColumnName);
  list->SetColumnResizable(col, 1);
  list->SetColumnStretchable(col, 1);
  list->SetColumnEditable(col, 1);
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetDefaultHelpStrings()
{
  if (!this->PresetButtons)
    {
    return;
    }

  this->PresetButtons->GetWidget(vtkKWPresetSelector::SelectPreviousButtonId)->
    SetBalloonHelpString("Select and apply previous preset");
  
  this->PresetButtons->GetWidget(vtkKWPresetSelector::SelectNextButtonId)->
    SetBalloonHelpString("Select and apply next preset");

  this->PresetButtons->GetWidget(vtkKWPresetSelector::AddButtonId)->
    SetBalloonHelpString("Add a preset");

  this->PresetButtons->GetWidget(vtkKWPresetSelector::ApplyButtonId)->
    SetBalloonHelpString("Apply the selected preset(s)");

  this->PresetButtons->GetWidget(vtkKWPresetSelector::UpdateButtonId)->
    SetBalloonHelpString("Update the selected preset(s)");
    
  this->PresetButtons->GetWidget(vtkKWPresetSelector::RemoveButtonId)->
    SetBalloonHelpString("Delete the selected preset(s)");

  this->PresetButtons->GetWidget(vtkKWPresetSelector::LocateButtonId)->
    SetBalloonHelpString("Locate the selected preset(s)");

  this->PresetButtons->GetWidget(vtkKWPresetSelector::EmailButtonId)->
    SetBalloonHelpString("Email the selected preset(s)");
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::GetIdColumnIndex()
{
  return this->PresetList ? 
    this->PresetList->GetWidget()->GetColumnIndexWithName(
      vtkKWPresetSelector::IdColumnName) : -1;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::GetThumbnailColumnIndex()
{
  return this->PresetList ? 
    this->PresetList->GetWidget()->GetColumnIndexWithName(
      vtkKWPresetSelector::ThumbnailColumnName) : -1;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::GetGroupColumnIndex()
{
  return this->PresetList ? 
    this->PresetList->GetWidget()->GetColumnIndexWithName(
      vtkKWPresetSelector::GroupColumnName) : -1;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::GetCommentColumnIndex()
{
  return this->PresetList ? 
    this->PresetList->GetWidget()->GetColumnIndexWithName(
      vtkKWPresetSelector::CommentColumnName) : -1;
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetApplyPresetOnSelection(int arg)
{
  if (this->ApplyPresetOnSelection == arg)
    {
    return;
    }

  this->ApplyPresetOnSelection = arg;
  this->Modified();

  if (this->PresetList)
    {
    if (this->ApplyPresetOnSelection)
      {
      this->PresetList->GetWidget()->SetSelectionModeToBrowse();
      }
    else
      {
      this->PresetList->GetWidget()->SetSelectionModeToExtended();
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetSelectSpinButtonsVisibility(int arg)
{
  if (this->SelectSpinButtonsVisibility == arg)
    {
    return;
    }

  this->SelectSpinButtonsVisibility = arg;
  this->Modified();

  this->Update();
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetLocateButtonVisibility(int arg)
{
  if (this->LocateButtonVisibility == arg)
    {
    return;
    }

  this->LocateButtonVisibility = arg;
  this->Modified();

  this->Update();
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetRemoveButtonVisibility(int arg)
{
  if (this->RemoveButtonVisibility == arg)
    {
    return;
    }

  this->RemoveButtonVisibility = arg;
  this->Modified();

  this->Update();
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetEmailButtonVisibility(int arg)
{
  if (this->EmailButtonVisibility == arg)
    {
    return;
    }

  this->EmailButtonVisibility = arg;
  this->Modified();

  this->Update();
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetListHeight(int h)
{
  if (this->PresetList)
    {
    this->PresetList->GetWidget()->SetHeight(h);
    }
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::GetListHeight()
{
  if (this->PresetList)
    {
    return this->PresetList->GetWidget()->GetHeight();
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetThumbnailColumnVisibility(int arg)
{
  if (this->PresetList)
    {
    this->PresetList->GetWidget()->SetColumnVisibility(
      this->GetThumbnailColumnIndex(), arg);
    }
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::GetThumbnailColumnVisibility()
{
  if (this->PresetList)
    {
    return this->PresetList->GetWidget()->GetColumnVisibility(
      this->GetThumbnailColumnIndex());
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetCommentColumnVisibility(int arg)
{
  if (this->PresetList)
    {
    this->PresetList->GetWidget()->SetColumnVisibility(
      this->GetCommentColumnIndex(), arg);
    }
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::GetCommentColumnVisibility()
{
  if (this->PresetList)
    {
    return this->PresetList->GetWidget()->GetColumnVisibility(
      this->GetCommentColumnIndex());
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetGroupColumnVisibility(int arg)
{
  if (this->PresetList)
    {
    this->PresetList->GetWidget()->SetColumnVisibility(
      this->GetGroupColumnIndex(), arg);
    }
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::GetGroupColumnVisibility()
{
  if (this->PresetList)
    {
    return this->PresetList->GetWidget()->GetColumnVisibility(
      this->GetGroupColumnIndex());
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetGroupColumnTitle(const char *arg)
{
  if (this->PresetList)
    {
    this->PresetList->GetWidget()->SetColumnTitle(
      this->GetGroupColumnIndex(), arg);
    }
}

//----------------------------------------------------------------------------
const char* vtkKWPresetSelector::GetGroupColumnTitle()
{
  if (this->PresetList)
    {
    return this->PresetList->GetWidget()->GetColumnTitle(
      this->GetGroupColumnIndex());
    }
  return NULL;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::AddPreset()
{
  if (!this->Internals)
    {
    return -1;
    }

  int id =  vtkKWPresetSelectorInternals::PresetNodeCounter++;

  vtkKWPresetSelectorInternals::PresetNode *node = 
    new vtkKWPresetSelectorInternals::PresetNode;

  node->Id = id;
  this->Internals->PresetPool.push_back(node);

  this->SetPresetCreationTime(id, vtksys::SystemTools::GetTime());
  // this->UpdatePresetRow(id); // called by SetPresetCreationTime

  if (this->PresetList)
    {
    int row = this->GetPresetRow(id);
    if (row >= 0)
      {
      this->PresetList->GetWidget()->SeeRow(row);
      }
    }

  this->NumberOfPresetsHasChanged();

  return id;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::HasPreset(int id)
{
  if (this->Internals)
    {
    vtkKWPresetSelectorInternals::PresetPoolIterator it = 
      this->Internals->GetPresetNode(id);
    return (it != this->Internals->PresetPool.end()) ? 1 : 0;
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetPresetGroupSlotName(const char *name)
{
  if (name && *name && 
      this->Internals && this->Internals->GroupSlotName.compare(name))
    {
    this->Internals->GroupSlotName = name;
    this->UpdatePresetRows();
    }
}

//----------------------------------------------------------------------------
const char* vtkKWPresetSelector::GetPresetGroupSlotName()
{
  if (this->Internals)
    {
    return this->Internals->GroupSlotName.c_str();
    }
  return NULL;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::SetPresetGroup(int id, const char *group)
{
  int res = this->SetPresetUserSlotAsString(
    id, this->GetPresetGroupSlotName(), group);
  if (res && this->GroupFilter)
    {
    // Changing the group of a preset may change the number of visible widgets
    // (for example, if the visibility of presets is filtered by groups), 
    // which can enable/disable some buttons
    this->Update(); 
    }
  return res;
}

//----------------------------------------------------------------------------
const char* vtkKWPresetSelector::GetPresetGroup(int id)
{
  return this->GetPresetUserSlotAsString(id, this->GetPresetGroupSlotName());
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetPresetCommentSlotName(const char *name)
{
  if (name && *name && 
      this->Internals && this->Internals->CommentSlotName.compare(name))
    {
    this->Internals->CommentSlotName = name;
    this->UpdatePresetRows();
    }
}

//----------------------------------------------------------------------------
const char* vtkKWPresetSelector::GetPresetCommentSlotName()
{
  if (this->Internals)
    {
    return this->Internals->CommentSlotName.c_str();
    }
  return NULL;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::SetPresetComment(int id, const char *comment)
{
  return this->SetPresetUserSlotAsString(
    id, this->GetPresetCommentSlotName(), comment);
}

//----------------------------------------------------------------------------
const char* vtkKWPresetSelector::GetPresetComment(int id)
{
  return this->GetPresetUserSlotAsString(
    id, this->GetPresetCommentSlotName());
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetPresetFileNameSlotName(const char *name)
{
  if (name && *name && 
      this->Internals && this->Internals->FileNameSlotName.compare(name))
    {
    this->Internals->FileNameSlotName = name;
    this->UpdatePresetRows();
    }
}

//----------------------------------------------------------------------------
const char* vtkKWPresetSelector::GetPresetFileNameSlotName()
{
  if (this->Internals)
    {
    return this->Internals->FileNameSlotName.c_str();
    }
  return NULL;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::SetPresetFileName(int id, const char *filename)
{
  return this->SetPresetUserSlotAsString(
    id, this->GetPresetFileNameSlotName(), filename);
}

//----------------------------------------------------------------------------
const char* vtkKWPresetSelector::GetPresetFileName(int id)
{
  return this->GetPresetUserSlotAsString(
    id, this->GetPresetFileNameSlotName());
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetPresetCreationTimeSlotName(const char *name)
{
  if (name && *name && 
      this->Internals && this->Internals->CreationTimeSlotName.compare(name))
    {
    this->Internals->CreationTimeSlotName = name;
    this->UpdatePresetRows();
    }
}

//----------------------------------------------------------------------------
const char* vtkKWPresetSelector::GetPresetCreationTimeSlotName()
{
  if (this->Internals)
    {
    return this->Internals->CreationTimeSlotName.c_str();
    }
  return NULL;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::SetPresetCreationTime(int id, double value)
{
  return this->SetPresetUserSlotAsDouble(
    id, this->GetPresetCreationTimeSlotName(), value);
}

//----------------------------------------------------------------------------
double vtkKWPresetSelector::GetPresetCreationTime(int id)
{
  return this->GetPresetUserSlotAsDouble(
    id, this->GetPresetCreationTimeSlotName());
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetPresetThumbnailSlotName(const char *name)
{
  if (name && *name && 
      this->Internals && this->Internals->ThumbnailSlotName.compare(name))
    {
    this->Internals->ThumbnailSlotName = name;
    this->UpdatePresetRows();
    }
}

//----------------------------------------------------------------------------
const char* vtkKWPresetSelector::GetPresetThumbnailSlotName()
{
  if (this->Internals)
    {
    return this->Internals->ThumbnailSlotName.c_str();
    }
  return NULL;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::SetPresetThumbnail(
  int id, vtkKWIcon *icon)
{
  if (this->HasPreset(id))
    {
    vtkKWIcon *ptr = this->GetPresetThumbnail(id);
    if (icon)
      {
      if (!ptr)
        {
        ptr = vtkKWIcon::New();
        }
      ptr->DeepCopy(icon);
      }
    else
      {
      if (ptr)
        {
        ptr->Delete();
        ptr = NULL;
        }
      }
    this->SetPresetUserSlotAsPointer(
      id, this->GetPresetThumbnailSlotName(), ptr);
    return 1;
    }

  return 0;
}

//----------------------------------------------------------------------------
vtkKWIcon* vtkKWPresetSelector::GetPresetThumbnail(int id)
{
  return (vtkKWIcon*)this->GetPresetUserSlotAsPointer(
    id, this->GetPresetThumbnailSlotName());
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetPresetScreenshotSlotName(const char *name)
{
  if (name && *name && 
      this->Internals && this->Internals->ScreenshotSlotName.compare(name))
    {
    this->Internals->ScreenshotSlotName = name;
    this->UpdatePresetRows();
    }
}

//----------------------------------------------------------------------------
const char* vtkKWPresetSelector::GetPresetScreenshotSlotName()
{
  if (this->Internals)
    {
    return this->Internals->ScreenshotSlotName.c_str();
    }
  return NULL;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::SetPresetScreenshot(
  int id, vtkKWIcon *icon)
{
  if (this->HasPreset(id))
    {
    vtkKWIcon *ptr = this->GetPresetScreenshot(id);
    if (icon)
      {
      if (!ptr)
        {
        ptr = vtkKWIcon::New();
        }
      ptr->DeepCopy(icon);
      }
    else
      {
      if (ptr)
        {
        ptr->Delete();
        ptr = NULL;
        }
      }
    this->SetPresetUserSlotAsPointer(
      id, this->GetPresetScreenshotSlotName(), ptr);
    return 1;
    }

  return 0;
}

//----------------------------------------------------------------------------
vtkKWIcon* vtkKWPresetSelector::GetPresetScreenshot(int id)
{
  return (vtkKWIcon*)this->GetPresetUserSlotAsPointer(
    id, this->GetPresetScreenshotSlotName());
}

//---------------------------------------------------------------------------
int vtkKWPresetSelector::BuildPresetThumbnailAndScreenshotFromImage(
  int id, vtkImageData *image)
{
  if (!this->Internals)
    {
    return 0;
    }

  vtkKWPresetSelectorInternals::PresetPoolIterator it = 
    this->Internals->GetPresetNode(id);
  if (it == this->Internals->PresetPool.end())
    {
    return 0;
    }

  // Empty image, remove thumbnail/screenshot

  int *image_dims = image ? image->GetDimensions() : NULL;
  if (!image_dims ||
      image_dims[0] == 0 || 
      image_dims[1] == 0 || 
      image_dims[2] == 0)
    {
    this->SetPresetThumbnail(id, NULL);
    this->SetPresetScreenshot(id, NULL);
    return 1;
    }

  double factor;
  vtkImageData *resample_input, *resample_output;

  // First, let's make sure we are processing the image as it
  // is by clipping its UpdateExtent. By doing so, we prevent our resample
  // and permute filter the process the image's *whole* extent.

  vtkImageClip *clip = vtkImageClip::New();
  clip->SetInput(image);
  clip->SetOutputWholeExtent(image->GetUpdateExtent());
  clip->Update();

  // Permute, as a convenience

  int *clip_dims = clip->GetOutput()->GetDimensions();

  vtkImagePermute *permute = NULL;
  if (clip_dims[2] != 1)
    {
    permute = vtkImagePermute::New();
    permute->SetInput(clip->GetOutput());
    if (clip_dims[0] == 1)
      {
      permute->SetFilteredAxes(1, 2, 0);
      }
    else
      {
      permute->SetFilteredAxes(0, 2, 1);
      }
    resample_input = permute->GetOutput();
    }
  else
    {
    resample_input = clip->GetOutput();
    }

  resample_input->Update();
  int *resample_input_dims = resample_input->GetDimensions();
  double *resample_input_spacing = resample_input->GetSpacing();

  int large_dim = 0, small_dim = 1;
  if (resample_input_dims[0] < resample_input_dims[1])
    {
    large_dim = 1; small_dim = 0;
    }

  vtkImageResample *resample = vtkImageResample::New();
  resample->SetInput(resample_input);
  resample->SetInterpolationModeToCubic();
  resample->SetDimensionality(2);

  // Create the screenshot

  factor = 
    (double)this->ScreenshotSize / (double)resample_input_dims[large_dim];
  resample->SetAxisMagnificationFactor(large_dim, factor);
  resample->SetAxisMagnificationFactor(
    small_dim, factor * (resample_input_spacing[small_dim] / 
                         resample_input_spacing[large_dim]));
  resample->Update();
  resample_output = resample->GetOutput();

  vtkKWIcon *screenshot = vtkKWIcon::New();
  screenshot->SetImage(
    (const unsigned char*)resample_output->GetScalarPointer(),
    resample_output->GetDimensions()[0],
    resample_output->GetDimensions()[1],
    3,
    0,
    vtkKWIcon::ImageOptionFlipVertical);
  this->SetPresetScreenshot(id, screenshot);
  screenshot->Delete();

  // Create the thumbnail

  factor = 
    (double)this->ThumbnailSize / (double)resample_input_dims[large_dim];
  resample->SetAxisMagnificationFactor(large_dim, factor);
  resample->SetAxisMagnificationFactor(
    small_dim, factor * (resample_input_spacing[small_dim] / 
                         resample_input_spacing[large_dim]));
  resample->Update();
  resample_output = resample->GetOutput();

  vtkKWIcon *thumbnail = vtkKWIcon::New();
  thumbnail->SetImage(
    (const unsigned char*)resample_output->GetScalarPointer(),
    resample_output->GetDimensions()[0],
    resample_output->GetDimensions()[1],
    3,
    0,
    vtkKWIcon::ImageOptionFlipVertical);
  this->SetPresetThumbnail(id, thumbnail);
  thumbnail->Delete();

  // Deallocate

  clip->Delete();
  resample->Delete();
  if (permute)
    {
    permute->Delete();
    }

  // Update the icon cell

  this->UpdatePresetRow(id);

  return 1;
}

//---------------------------------------------------------------------------
int vtkKWPresetSelector::BuildPresetThumbnailAndScreenshotFromRenderWindow(
  int id, vtkRenderWindow *win)
{
  if (win)
    {
    vtkWindowToImageFilter *filter = vtkWindowToImageFilter::New();
    filter->ShouldRerenderOff();
    filter->SetInput(win);
    filter->Update();
    int res = this->BuildPresetThumbnailAndScreenshotFromImage(id, filter->GetOutput());
    filter->Delete();
    return res;
    }
  return 0;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::HasPresetUserSlot(int id, const char *slot_name)
{
  if (this->Internals)
    {
    vtkKWPresetSelectorInternals::PresetPoolIterator it = 
      this->Internals->GetPresetNode(id);
    if (it != this->Internals->PresetPool.end())
      {
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        (*it)->UserSlotPool.find(slot_name);
      if (s_it != (*it)->UserSlotPool.end())
        {
        return 1;
        }
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::SetPresetUserSlotAsDouble(
  int id, const char *slot_name, double value)
{
  if (this->Internals)
    {
    vtkKWPresetSelectorInternals::PresetPoolIterator it = 
      this->Internals->GetPresetNode(id);
    if (it != this->Internals->PresetPool.end())
      {
      (*it)->UserSlotPool[slot_name].DoubleValue = value;
      this->UpdatePresetRow(id);
      return 1;
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
double vtkKWPresetSelector::GetPresetUserSlotAsDouble(
  int id, const char *slot_name)
{
  if (this->Internals)
    {
    vtkKWPresetSelectorInternals::PresetPoolIterator it = 
      this->Internals->GetPresetNode(id);
    if (it != this->Internals->PresetPool.end())
      {
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        (*it)->UserSlotPool.find(slot_name);
      if (s_it != (*it)->UserSlotPool.end())
        {
        return s_it->second.DoubleValue;
        }
      }
    }

  return 0.0;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::SetPresetUserSlotAsInt(
  int id, const char *slot_name, int value)
{
  if (this->Internals)
    {
    vtkKWPresetSelectorInternals::PresetPoolIterator it = 
      this->Internals->GetPresetNode(id);
    if (it != this->Internals->PresetPool.end())
      {
      (*it)->UserSlotPool[slot_name].IntValue = value;
      this->UpdatePresetRow(id);
      return 1;
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::GetPresetUserSlotAsInt(
  int id, const char *slot_name)
{
  if (this->Internals)
    {
    vtkKWPresetSelectorInternals::PresetPoolIterator it = 
      this->Internals->GetPresetNode(id);
    if (it != this->Internals->PresetPool.end())
      {
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        (*it)->UserSlotPool.find(slot_name);
      if (s_it != (*it)->UserSlotPool.end())
        {
        return s_it->second.IntValue;
        }
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::SetPresetUserSlotAsString(
  int id, const char *slot_name, const char* value)
{
  if (this->Internals)
    {
    vtkKWPresetSelectorInternals::PresetPoolIterator it = 
      this->Internals->GetPresetNode(id);
    if (it != this->Internals->PresetPool.end())
      {
      (*it)->UserSlotPool[slot_name].StringValue = value ? value : "";
      this->UpdatePresetRow(id);
      return 1;
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
const char* vtkKWPresetSelector::GetPresetUserSlotAsString(
  int id, const char *slot_name)
{
  if (this->Internals)
    {
    vtkKWPresetSelectorInternals::PresetPoolIterator it = 
      this->Internals->GetPresetNode(id);
    if (it != this->Internals->PresetPool.end())
      {
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        (*it)->UserSlotPool.find(slot_name);
      if (s_it != (*it)->UserSlotPool.end())
        {
        return s_it->second.StringValue.c_str();
        }
      }
    }

  return NULL;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::SetPresetUserSlotAsPointer(
  int id, const char *slot_name, void *value)
{
  if (this->Internals)
    {
    vtkKWPresetSelectorInternals::PresetPoolIterator it = 
      this->Internals->GetPresetNode(id);
    if (it != this->Internals->PresetPool.end())
      {
      (*it)->UserSlotPool[slot_name].PointerValue = value;
      this->UpdatePresetRow(id);
      return 1;
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
void* vtkKWPresetSelector::GetPresetUserSlotAsPointer(
  int id, const char *slot_name)
{
  if (this->Internals)
    {
    vtkKWPresetSelectorInternals::PresetPoolIterator it = 
      this->Internals->GetPresetNode(id);
    if (it != this->Internals->PresetPool.end())
      {
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        (*it)->UserSlotPool.find(slot_name);
      if (s_it != (*it)->UserSlotPool.end())
        {
        return s_it->second.PointerValue;
        }
      }
    }

  return NULL;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::GetNumberOfPresets()
{
  if (this->Internals)
    {
    return this->Internals->PresetPool.size();
    }
  return 0;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::GetNumberOfPresetsWithGroup(const char *group)
{
  int count = 0;
  if (this->Internals && group && *group)
    {
    vtkKWPresetSelectorInternals::PresetPoolIterator it = 
      this->Internals->PresetPool.begin();
    vtkKWPresetSelectorInternals::PresetPoolIterator end = 
      this->Internals->PresetPool.end();
    for (; it != end; ++it)
      {
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        (*it)->UserSlotPool.find(this->GetPresetGroupSlotName());
      if (s_it != (*it)->UserSlotPool.end() &&
          !s_it->second.StringValue.compare(group))
        {
        count++;
        }
      }
    }

  return count;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::GetNumberOfVisiblePresets()
{
  if (this->GroupFilter && *this->GroupFilter)
    {
    return this->GetNumberOfPresetsWithGroup(this->GroupFilter);
    }
  return this->GetNumberOfPresets();
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::GetNthPresetId(int index)
{
  if (this->Internals && index >= 0 && index < this->GetNumberOfPresets())
    {
    return this->Internals->PresetPool[index]->Id;
    }

  return -1;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::GetNthPresetWithGroupId(int index, const char *group)
{
  int rank = this->GetNthPresetWithGroupRank(index, group);
  if (rank >= 0)
    {
    return this->GetNthPresetId(rank);
    }
  return -1;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::GetPresetAtRowId(int row_index)
{
  if (this->PresetList)
    {
    vtkKWMultiColumnList *list = this->PresetList->GetWidget();
    if (row_index >= 0 && row_index < list->GetNumberOfRows())
      {
      return list->GetCellTextAsInt(row_index, this->GetIdColumnIndex());
      }
    }
  return -1;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::GetPresetRow(int id)
{
  if (this->PresetList)
    {
    return this->PresetList->GetWidget()->FindCellTextAsIntInColumn(
      this->GetIdColumnIndex(), id);
    }
  return -1;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::GetNthPresetWithGroupRank(
  int index, const char *group)
{
  if (this->Internals && index >= 0 && group && *group)
    {
    vtkKWPresetSelectorInternals::PresetPoolIterator it = 
      this->Internals->PresetPool.begin();
    vtkKWPresetSelectorInternals::PresetPoolIterator end = 
      this->Internals->PresetPool.end();
    for (int nth = 0; it != end; ++it, nth++)
      {
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        (*it)->UserSlotPool.find(this->GetPresetGroupSlotName());
      if (s_it != (*it)->UserSlotPool.end() &&
          !s_it->second.StringValue.compare(group))
        {
        index--;
        if (index < 0)
          {
          return nth;
          }
        }
      }
    }

  return -1;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::RemovePreset(int id)
{
  if (this->Internals)
    {
    vtkKWPresetSelectorInternals::PresetPoolIterator it = 
      this->Internals->GetPresetNode(id);
    if (it != this->Internals->PresetPool.end())
      {
      this->DeAllocatePreset(id);
      if (this->PresetList)
        {
        int row = this->GetPresetRow(id);
        if (row >= 0)
          {
          this->PresetList->GetWidget()->DeleteRow(row);
          }
        }
      delete (*it);
      this->Internals->PresetPool.erase(it);
      this->NumberOfPresetsHasChanged();
      return 1;
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::DeleteAllPresets()
{
  if (this->Internals)
    {
    int nb_deleted = this->GetNumberOfPresets();
    vtkKWPresetSelectorInternals::PresetPoolIterator end = 
      this->Internals->PresetPool.end();
    vtkKWPresetSelectorInternals::PresetPoolIterator it;

    // First give a chance to third-party or subclasses to deallocate
    // the presets cleanly (without messing up the iterator framework)

    it = this->Internals->PresetPool.begin();
    for (; it != end; ++it)
      {
      this->DeAllocatePreset((*it)->Id);
      }

    // Then remove the presets

    it = this->Internals->PresetPool.begin();
    for (; it != end; ++it)
      {
      delete (*it);
      }
    this->Internals->PresetPool.clear();

    return nb_deleted;
    }

  return 0;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::RemoveAllPresets()
{
  int nb_deleted = this->DeleteAllPresets();

  if (this->PresetList)
    {
    this->PresetList->GetWidget()->DeleteAllRows();
    }

  if (nb_deleted)
    {
    this->NumberOfPresetsHasChanged();
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::RemoveAllPresetsWithGroup(const char *group)
{
  // Is faster than calling RemovePreset on each preset

  if (this->Internals && group && *group)
    {
    vtkKWPresetSelectorInternals::PresetPoolIterator it;
    vtkKWPresetSelectorInternals::PresetPoolIterator end = 
      this->Internals->PresetPool.end();

    // First give a chance to third-party or subclasses to deallocate
    // the presets cleanly (without messing up the iterator framework)

    it = this->Internals->PresetPool.begin();
    for (; it != end; ++it)
      {
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        (*it)->UserSlotPool.find(this->GetPresetGroupSlotName());
      if (s_it != (*it)->UserSlotPool.end() &&
          !s_it->second.StringValue.compare(group))
        {
        this->DeAllocatePreset((*it)->Id);
        if (this->PresetList)
          {
          int row = this->GetPresetRow((*it)->Id);
          if (row >= 0)
            {
            this->PresetList->GetWidget()->DeleteRow(row);
            }
          }
        }
      }

    // Then remove the presets

    int nb_deleted = 0;
    int done = 0;
    while (!done)
      {
      done = 1;
      it = this->Internals->PresetPool.begin();
      for (; it != end; ++it)
        {
        vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
          (*it)->UserSlotPool.find(this->GetPresetGroupSlotName());
        if (s_it != (*it)->UserSlotPool.end() &&
            !s_it->second.StringValue.compare(group))
          {
          delete (*it);
          this->Internals->PresetPool.erase(it);
          nb_deleted++;
          done = 0;
          break;
          }
        }
      }
    
    if (nb_deleted)
      {
      this->NumberOfPresetsHasChanged();
      }
    }

  return 1;
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SelectPreset(int id)
{
  int row = this->GetPresetRow(id);
  if (row >= 0 && this->PresetList)
    {
    this->PresetList->GetWidget()->SelectSingleRow(row);
    this->PresetList->GetWidget()->SeeRow(row);
    }
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SelectPreviousPreset()
{
  if (this->PresetList)
    {
    vtkKWMultiColumnList *list = this->PresetList->GetWidget();
    int nb_rows = list->GetNumberOfRows();
    if (nb_rows)
      {
      int prev_row; 
      if (!list->GetNumberOfSelectedRows())
        {
        prev_row = nb_rows - 1;
        }
      else
        {
        int sel_row = list->GetIndexOfFirstSelectedRow();
        prev_row = (nb_rows == 1 || sel_row == 0) ? nb_rows - 1 : sel_row - 1;
        }
      list->SelectSingleRow(prev_row);
      list->SeeRow(prev_row);
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SelectNextPreset()
{
  if (this->PresetList)
    {
    vtkKWMultiColumnList *list = this->PresetList->GetWidget();
    int nb_rows = list->GetNumberOfRows();
    if (nb_rows)
      {
      int next_row; 
      if (!list->GetNumberOfSelectedRows())
        {
        next_row = 0;
        }
      else
        {
        int sel_row = list->GetIndexOfFirstSelectedRow();
        next_row = (nb_rows == 1 || sel_row == nb_rows - 1) ? 0 : sel_row + 1;
        }
      list->SelectSingleRow(next_row);
      list->SeeRow(next_row);
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::ClearSelection()
{
  if (this->PresetList)
    {
    this->PresetList->GetWidget()->ClearSelection();
    }
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::NumberOfPresetsHasChanged()
{
  this->Update(); // enable/disable some buttons valid only if we have presets
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetGroupFilter(const char* _arg)
{
  if (this->GroupFilter == NULL && _arg == NULL) 
    { 
    return;
    }

  if (this->GroupFilter && _arg && (!strcmp(this->GroupFilter, _arg))) 
    { 
    return;
    }

  if (this->GroupFilter) 
    { 
    delete [] this->GroupFilter; 
    }

  if (_arg)
    {
    this->GroupFilter = new char[strlen(_arg)+1];
    strcpy(this->GroupFilter,_arg);
    }
   else
    {
    this->GroupFilter = NULL;
    }

  this->Modified();

  this->UpdatePresetRows();
} 

//----------------------------------------------------------------------------
void vtkKWPresetSelector::UpdatePresetRows()
{
  int nb_visible_presets = this->GetNumberOfVisiblePresets();
  int nb_presets = this->GetNumberOfPresets();
  for (int i = 0; i < nb_presets; i++)
    {
    this->UpdatePresetRow(this->GetNthPresetId(i));
    }

  // If the number of visible presets changed, this can enable/disable
  // some buttons

  if (nb_visible_presets != this->GetNumberOfVisiblePresets())
    {
    this->Update();
    }
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::UpdatePresetRow(int id)
{
  if (!this->HasPreset(id))
    {
    return 0;
    }

  vtkKWMultiColumnList *list = this->PresetList->GetWidget();

  // Look for this node in the list

  int row = this->GetPresetRow(id);

  const char *group = this->GetPresetGroup(id);
  int group_filter_exclude =
    this->GroupFilter && *this->GroupFilter && 
    group && *group &&
    strcmp(group, this->GroupFilter);

  // Not found ? Insert it, or ignore it if the group filter does not match

  if (row < 0)
    {
    if (group_filter_exclude)
      {
      return 0;
      }
    list->AddRow();
    row = list->GetNumberOfRows() - 1;
    if (row < 0)
      {
      return 0;
      }
    }

  // Found ? Remove it if the group filter does not match

  else
    {
    if (group_filter_exclude)
      {
      list->DeleteRow(row);
      return 0;
      }
    }

  // Id (not shown, but useful to retrieve the id of a preset from
  // a cell position

  char buffer[256];
  sprintf(buffer, "%03d", id);
  list->SetCellText(row, this->GetIdColumnIndex(), buffer);

  int image_col_index = this->GetThumbnailColumnIndex();
  list->SetCellWindowCommand(
    row, image_col_index, this, "PresetCellThumbnailCallback");
  list->SetCellWindowDestroyCommandToRemoveChild(row, image_col_index);
  if (this->GetThumbnailColumnVisibility())
    {
    list->RefreshCellWithWindowCommand(row, image_col_index);
    }

  list->SetCellTextAsDouble(
    row, image_col_index, this->GetPresetCreationTime(id));

  list->SetCellText(
    row, this->GetGroupColumnIndex(), group);

  list->SetCellText(
    row, this->GetCommentColumnIndex(), this->GetPresetComment(id));

  return 1;
}

//---------------------------------------------------------------------------
void vtkKWPresetSelector::PresetCellThumbnailCallback(
  const char *, int row, int, const char *widget)
{
  if (!this->PresetList || !widget)
    {
    return;
    }

  vtkKWMultiColumnList *list = this->PresetList->GetWidget();

  int id = this->GetPresetAtRowId(row);
  if (this->HasPreset(id))
    {
    vtkKWLabel *child = vtkKWLabel::New();
    child->SetWidgetName(widget);
    child->SetParent(list);

    vtkKWIcon *screenshot = this->GetPresetScreenshot(id);
    if (screenshot)
      {
      // Create out own balloon help manager for this one, so that
      // we can set a much shorter delay
      vtkKWBalloonHelpManager *mgr = vtkKWBalloonHelpManager::New();
      mgr->SetApplication(list->GetApplication());
      child->SetBalloonHelpManager(mgr);
      mgr->SetDelay(10);
      mgr->Delete();
      child->SetBalloonHelpIcon(screenshot);
      }

    child->Create();
    child->SetBorderWidth(0);
    child->SetHighlightThickness(0);
    child->SetWidth(this->ThumbnailSize);
    child->SetHeight(this->ThumbnailSize);
    child->SetBackgroundColor(list->GetCellCurrentBackgroundColor(
                                row, this->GetThumbnailColumnIndex()));

    vtkKWIcon *thumbnail = this->GetPresetThumbnail(id);
    if (thumbnail)
      {
      child->SetImageToIcon(thumbnail);
      }
    else
      {
      child->SetImageToPredefinedIcon(vtkKWIcon::IconEmpty1x1);
      }

    list->AddBindingsToWidget(child);
    child->Delete();
    }
}

//---------------------------------------------------------------------------
const char* vtkKWPresetSelector::PresetCellEditStartCallback(
  int, int, const char *text)
{
  return text;
}

//---------------------------------------------------------------------------
const char* vtkKWPresetSelector::PresetCellEditEndCallback(
  int, int, const char *text)
{
  return text;
}

//---------------------------------------------------------------------------
void vtkKWPresetSelector::PresetCellUpdatedCallback(
  int row, int col, const char *text)
{
  int id = this->GetPresetAtRowId(row);
  if (this->HasPreset(id))
    {
    if (col == this->GetCommentColumnIndex())
      {
      this->SetPresetComment(id, text);
      this->InvokePresetHasChangedCommand(id);
      return;
      }
    }
}

//---------------------------------------------------------------------------
void vtkKWPresetSelector::PresetAddCallback()
{
  this->InvokePresetAddCommand();
}

//---------------------------------------------------------------------------
void vtkKWPresetSelector::PresetApplyCallback()
{
  if (this->PresetList)
    {
    vtkKWMultiColumnList *list = this->PresetList->GetWidget();
    int *indices = new int [list->GetNumberOfRows()];
    int i, nb_selected_rows = list->GetSelectedRows(indices);
    for (i = 0; i < nb_selected_rows; i++)
      {
      int id = this->GetPresetAtRowId(indices[i]);
      if (this->HasPreset(id))
        {
        this->InvokePresetApplyCommand(id);
        }
      }
    delete [] indices;
    }
}

//---------------------------------------------------------------------------
void vtkKWPresetSelector::PresetUpdateCallback()
{
  if (this->PresetList)
    {
    vtkKWMultiColumnList *list = this->PresetList->GetWidget();

    // First collect the indices of the presets to update
    // Then update them

    int *indices = new int [list->GetNumberOfRows()];
    int *ids = new int [list->GetNumberOfRows()];

    int i, nb_selected_rows = list->GetSelectedRows(indices);
    for (i = 0; i < nb_selected_rows; i++)
      {
      ids[i] = this->GetPresetAtRowId(indices[i]);
      }

    for (i = 0; i < nb_selected_rows; i++)
      {
      this->InvokePresetUpdateCommand(ids[i]);
      }

    delete [] indices;
    delete [] ids;
    }
}

//---------------------------------------------------------------------------
void vtkKWPresetSelector::PresetRemoveCallback()
{
  if (this->PresetList)
    {
    vtkKWMultiColumnList *list = this->PresetList->GetWidget();

    // First collect the indices of the presets to remove

    int *indices = new int [list->GetNumberOfRows()];
    int *ids = new int [list->GetNumberOfRows()];

    int i, nb_selected_rows = list->GetSelectedRows(indices);
    for (i = 0; i < nb_selected_rows; i++)
      {
      ids[i] = this->GetPresetAtRowId(indices[i]);
      }

    // Then remove them

    if (nb_selected_rows)
      {
      if (!this->PromptBeforeRemovePreset ||
          vtkKWMessageDialog::PopupYesNo( 
            this->GetApplication(), 
            this->GetApplication()->GetNthWindow(0), 
            "Delete Preset",
            "Are you sure you want to delete the selected item(s)?", 
            vtkKWMessageDialog::WarningIcon | 
            vtkKWMessageDialog::InvokeAtPointer))
        {
        for (i = 0; i < nb_selected_rows; i++)
          {
          if (this->InvokePresetRemoveCommand(ids[i]))
            {
            this->RemovePreset(ids[i]);
            }
          }
        }
      }
    
    delete [] indices;
    delete [] ids;
    }
}

//---------------------------------------------------------------------------
void vtkKWPresetSelector::PresetLocateCallback()
{
  if (this->PresetList)
    {
    vtkKWMultiColumnList *list = this->PresetList->GetWidget();

    int *indices = new int [list->GetNumberOfRows()];
    int i, nb_selected_rows = list->GetSelectedRows(indices);
    for (i = 0; i < nb_selected_rows; i++)
      {
      int id = this->GetPresetAtRowId(indices[i]);
      const char *filename = this->GetPresetFileName(id);
      if (filename && *filename && vtksys::SystemTools::FileExists(filename))
        {
        this->GetApplication()->ExploreLink(filename);
        }
      }
    
    delete [] indices;
    }
}

//---------------------------------------------------------------------------
void vtkKWPresetSelector::PresetEmailCallback()
{
  if (this->PresetList)
    {
    vtkKWMultiColumnList *list = this->PresetList->GetWidget();

    int *indices = new int [list->GetNumberOfRows()];
    int i, nb_selected_rows = list->GetSelectedRows(indices);
    for (i = 0; i < nb_selected_rows; i++)
      {
      int id = this->GetPresetAtRowId(indices[i]);
      const char *filename = this->GetPresetFileName(id);
      if (!filename || !*filename ||!vtksys::SystemTools::FileExists(filename))
        {
        continue;
        }
      
      vtksys_stl::string collapsed_filename = 
        vtksys::SystemTools::CollapseFullPath(filename);
      vtksys_stl::string native_filename(collapsed_filename);
#if _WIN32
      vtksys::SystemTools::ReplaceString(native_filename, "/", "\\");
#endif

      const char *comment = this->GetPresetComment(id);

      vtksys_stl::string subject;
      subject = this->GetApplication()->GetPrettyName();
      subject += ": \"";
      subject += vtksys::SystemTools::GetFilenameName(native_filename);
      subject += "\"";
      if (comment && *comment)
        {
        subject += " (";
        subject += comment;
        subject += ")";
        }

      vtksys_stl::string message;
      message = "This file was sent from ";
      message += this->GetApplication()->GetPrettyName();
      message += "\n\n";

      message += "File: ";
      message += native_filename;
      message += "\n";

      if (comment && *comment)
        {
        message += "Comment: ";
        message += comment;
        message += "\n";
        }
        
      message += "Creation Time: ";
      time_t t = (time_t)this->GetPresetCreationTime(id);
      message += ctime(&t);

      this->GetApplication()->SendEmail(
        NULL, subject.c_str(), message.c_str(), native_filename.c_str(), NULL);
      }
    
    delete [] indices;
    }
}

//---------------------------------------------------------------------------
void vtkKWPresetSelector::PresetSelectionCallback()
{
  this->Update(); // this enable/disable the remove button if no selection

  if (this->ApplyPresetOnSelection)
    {
    this->PresetApplyCallback();
    }
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetPresetAddCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->PresetAddCommand, object, method);
  this->Update(); // this show/hide the add button
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::InvokePresetAddCommand()
{
  this->InvokeObjectMethodCommand(this->PresetAddCommand);
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetPresetUpdateCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->PresetUpdateCommand, object, method);
  this->Update(); // this show/hide the update button
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::InvokePresetUpdateCommand(int id)
{
  if (this->PresetUpdateCommand && 
      *this->PresetUpdateCommand && 
      this->IsCreated())
    {
    this->Script("%s %d", this->PresetUpdateCommand, id);
    }
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetPresetApplyCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->PresetApplyCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::InvokePresetApplyCommand(int id)
{
  if (this->PresetApplyCommand && 
      *this->PresetApplyCommand && 
      this->IsCreated())
    {
    this->Script("%s %d", this->PresetApplyCommand, id);
    }
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetPresetRemoveCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->PresetRemoveCommand, object, method);
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::InvokePresetRemoveCommand(int id)
{
  if (this->PresetRemoveCommand && 
      *this->PresetRemoveCommand && 
      this->IsCreated())
    {
    return atoi(this->Script("%s %d", this->PresetRemoveCommand, id));
    }
  return 1;
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetPresetHasChangedCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(
    &this->PresetHasChangedCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::InvokePresetHasChangedCommand(int id)
{
  if (this->PresetHasChangedCommand && 
      *this->PresetHasChangedCommand && 
      this->IsCreated())
    {
    this->Script("%s %d", this->PresetHasChangedCommand, id);
    }
}

//---------------------------------------------------------------------------
void vtkKWPresetSelector::Update()
{
  this->UpdateEnableState();

  if (!this->PresetButtons)
    {
    return;
    }

  int has_selection = 
    (this->PresetList && 
     this->PresetList->GetWidget()->GetNumberOfSelectedCells());

  int has_presets = this->GetNumberOfVisiblePresets();

  // Select prev/next

  this->PresetButtons->SetWidgetVisibility(
    vtkKWPresetSelector::SelectPreviousButtonId, 
    this->SelectSpinButtonsVisibility);

  this->PresetButtons->GetWidget(
    vtkKWPresetSelector::SelectPreviousButtonId)->SetEnabled(
      has_presets ? this->PresetButtons->GetEnabled() : 0);

  this->PresetButtons->SetWidgetVisibility(
    vtkKWPresetSelector::SelectNextButtonId, 
    this->SelectSpinButtonsVisibility);

  this->PresetButtons->GetWidget(
    vtkKWPresetSelector::SelectNextButtonId)->SetEnabled(
      has_presets ? this->PresetButtons->GetEnabled() : 0);

  // Add

  this->PresetButtons->SetWidgetVisibility(
    vtkKWPresetSelector::AddButtonId, 
    (this->PresetAddCommand && *this->PresetAddCommand) ? 1 : 0);

  // Apply

  this->PresetButtons->SetWidgetVisibility(
    vtkKWPresetSelector::ApplyButtonId, 
    this->PresetApplyCommand && *this->PresetApplyCommand &&
    !this->ApplyPresetOnSelection ? 1 : 0);

  this->PresetButtons->GetWidget(
    vtkKWPresetSelector::ApplyButtonId)->SetEnabled(
      has_selection ? this->PresetButtons->GetEnabled() : 0);

  // Update

  this->PresetButtons->SetWidgetVisibility(
    vtkKWPresetSelector::UpdateButtonId, 
    (this->PresetUpdateCommand && *this->PresetUpdateCommand) ? 1 : 0);

  this->PresetButtons->GetWidget(
    vtkKWPresetSelector::UpdateButtonId)->SetEnabled(
      has_selection ? this->PresetButtons->GetEnabled() : 0);

  // Remove

  this->PresetButtons->GetWidget(
    vtkKWPresetSelector::RemoveButtonId)->SetEnabled(
      has_selection ? this->PresetButtons->GetEnabled() : 0);

  this->PresetButtons->SetWidgetVisibility(
    vtkKWPresetSelector::RemoveButtonId, 
    this->RemoveButtonVisibility ? 1 : 0);

  // Locate

  this->PresetButtons->SetWidgetVisibility(
    vtkKWPresetSelector::LocateButtonId, 
    this->LocateButtonVisibility ? 1 : 0);

  this->PresetButtons->GetWidget(
    vtkKWPresetSelector::LocateButtonId)->SetEnabled(
      has_selection ? this->PresetButtons->GetEnabled() : 0);

  // Email

  this->PresetButtons->SetWidgetVisibility(
    vtkKWPresetSelector::EmailButtonId, 
    this->EmailButtonVisibility ? 1 : 0);

  this->PresetButtons->GetWidget(
    vtkKWPresetSelector::EmailButtonId)->SetEnabled(
      has_selection ? this->PresetButtons->GetEnabled() : 0);
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  if (this->PresetList)
    {
    this->PresetList->SetEnabled(this->GetEnabled());
    }

  if (this->PresetControlFrame)
    {
    this->PresetControlFrame->SetEnabled(this->GetEnabled());
    }

  if (this->PresetButtons)
    {
    this->PresetButtons->SetEnabled(this->GetEnabled());
    }
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
