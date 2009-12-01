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
#include "vtkKWInternationalization.h"
#include "vtkKWLabel.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWLabelWithLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWWindowBase.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWPushButton.h"
#include "vtkKWPushButtonSet.h"
#include "vtkKWToolbar.h"
#include "vtkKWTkUtilities.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkWindowToImageFilter.h"
#include "vtkImagePermute.h"
#include "vtkImageClip.h"

#include <vtksys/SystemTools.hxx>
#include <vtksys/stl/list>
#include <vtksys/stl/string>
#include <vtksys/stl/map>
#include <vtksys/RegularExpression.hxx>
#include <vtksys/ios/sstream>

#include <time.h>

const char *vtkKWPresetSelector::IdColumnName        = "Id";
const char *vtkKWPresetSelector::ThumbnailColumnName = "Image";
const char *vtkKWPresetSelector::GroupColumnName     = "Group";
const char *vtkKWPresetSelector::CommentColumnName   = "Comment";

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWPresetSelector);
vtkCxxRevisionMacro(vtkKWPresetSelector, "$Revision: 1.86 $");

//----------------------------------------------------------------------------
class vtkKWPresetSelectorInternals
{
public:

  // User slot

  typedef vtksys_stl::string UserSlotNameType;

  class UserSlotValueType
  {
  public:
    int Type;
    double DoubleValue;
    int IntValue;
    unsigned long UnsignedLongValue;
    vtkTypeInt64 Int64Value;
    vtksys_stl::string StringValue;
    void *PointerValue;
    vtkObject *ObjectValue;
    
    void DeAllocate();
  };

  // User slot pool

  typedef vtksys_stl::map<UserSlotNameType, UserSlotValueType> UserSlotPoolType;
  typedef vtksys_stl::map<UserSlotNameType, UserSlotValueType>::iterator UserSlotPoolIterator;

  // Preset node

  class PresetNode
  {
  public:
    int Id;
    UserSlotPoolType UserSlotPool;
  };

  // Preset pool
  // We maintain two pools, one is a list, because the ordering of the
  // preset matters (one can insert at any position), and the another one
  // as a map for faster/direct access.

  int PresetNodeCounter;

  typedef vtksys_stl::list<PresetNode*> PresetPoolType;
  typedef vtksys_stl::list<PresetNode*>::iterator PresetPoolIterator;

  PresetPoolType PresetPool;
  PresetPoolIterator GetPresetNodeIterator(int id);

  typedef vtksys_stl::map<int, PresetNode*> SortedPresetPoolType;
  typedef vtksys_stl::map<int, PresetNode*>::iterator SortedPresetPoolIterator;

  SortedPresetPoolType SortedPresetPool; // for faster quick access
  SortedPresetPoolIterator GetSortedPresetNodeIterator(int id);

  PresetNode* GetPresetNode(int id);

  // Timers for updating preset rows

  typedef vtksys_stl::map<int,vtksys_stl::string> ScheduleUpdatePresetRowTimerPoolType;
  typedef vtksys_stl::map<int,vtksys_stl::string>::iterator ScheduleUpdatePresetRowTimerPoolIterator;
  ScheduleUpdatePresetRowTimerPoolType ScheduleUpdatePresetRowTimerPool;

  vtksys_stl::string ScheduleUpdatePresetRowsTimerId;
  vtksys_stl::string SchedulePresetSelectionCallbackTimerId;
  vtksys_stl::string SchedulePresetSelectionChangedCallbackTimerId;

  // User slot name for the default fields

  vtksys_stl::string GroupSlotName;
  vtksys_stl::string CommentSlotName;
  vtksys_stl::string FileNameSlotName;
  vtksys_stl::string CreationTimeSlotName;
  vtksys_stl::string ThumbnailSlotName;
  vtksys_stl::string ScreenshotSlotName;

  // Filter constraint

  class PresetFilterConstraint
  {
  public:
    vtksys_stl::string StringValue;
    int IsRegularExpression;
  };

  // Filter pool

  typedef vtksys_stl::map<UserSlotNameType, PresetFilterConstraint> PresetFilterType;
  typedef vtksys_stl::map<UserSlotNameType, PresetFilterConstraint>::iterator PresetFilterIterator;

  PresetFilterType PresetFilter;

  // Button label (internal purposes)

  vtksys_stl::string SelectPreviousButtonLabel;
  vtksys_stl::string SelectNextButtonLabel;
  vtksys_stl::string AddButtonLabel;
  vtksys_stl::string ApplyButtonLabel;
  vtksys_stl::string UpdateButtonLabel;
  vtksys_stl::string RemoveButtonLabel;
  vtksys_stl::string RemoveAllButtonLabel;
  vtksys_stl::string LocateButtonLabel;
  vtksys_stl::string EmailButtonLabel;
  vtksys_stl::string LoadButtonLabel;
  vtksys_stl::string FilterButtonLabel;

  // Let's try to speed things up by caching some info
  // - preset id to row index
  // - row index to preset id

  typedef vtksys_stl::map<int, int> PresetIdToRowIndexCacheType;
  typedef vtksys_stl::map<int, int>::iterator PresetIdToRowIndexCacheTypeIterator;
  PresetIdToRowIndexCacheType PresetIdToRowIndexCache;

  typedef vtksys_stl::map<int, int> RowIndexToPresetIdCacheType;
  typedef vtksys_stl::map<int, int>::iterator RowIndexToPresetIdCacheTypeIterator;
  RowIndexToPresetIdCacheType RowIndexToPresetIdCache;
};

//---------------------------------------------------------------------------
vtkKWPresetSelectorInternals::PresetPoolIterator
vtkKWPresetSelectorInternals::GetPresetNodeIterator(int id)
{
  vtkKWPresetSelectorInternals::PresetPoolIterator it = 
    this->PresetPool.begin();
  vtkKWPresetSelectorInternals::PresetPoolIterator end = 
    this->PresetPool.end();
  for (; it != end; it++)
    {
    if ((*it)->Id == id)
      {
      return it;
      }
    }

  return end;
}

//---------------------------------------------------------------------------
vtkKWPresetSelectorInternals::SortedPresetPoolIterator
vtkKWPresetSelectorInternals::GetSortedPresetNodeIterator(int id)
{
  return this->SortedPresetPool.find(id);
}

//---------------------------------------------------------------------------
vtkKWPresetSelectorInternals::PresetNode*
vtkKWPresetSelectorInternals::GetPresetNode(int id)
{
#if 1
  vtkKWPresetSelectorInternals::SortedPresetPoolIterator it = 
    this->GetSortedPresetNodeIterator(id);
  if (it != this->SortedPresetPool.end())
    {
    return it->second;
    }
#else
  vtkKWPresetSelectorInternals::PresetPoolIterator it = 
    this->GetPresetNodeIterator(id);
  if (it != this->PresetPool.end())
    {
    return *it;
    }
#endif

  return NULL;
}

//---------------------------------------------------------------------------
void vtkKWPresetSelectorInternals::UserSlotValueType::DeAllocate()
{
  if (this->Type == vtkKWPresetSelector::UserSlotObjectType &&
      this->ObjectValue)
    {
    this->ObjectValue->UnRegister(NULL);
    }
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::DeAllocatePreset(int id)
{
  if (this->Internals)
    {
    vtkKWPresetSelectorInternals::PresetNode *node =
      this->Internals->GetPresetNode(id);
    if (node)
      {
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        node->UserSlotPool.begin();
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_end =
        node->UserSlotPool.end();
      while (s_it != s_end)
        {
        s_it->second.DeAllocate();
        ++s_it;
        }
      }
    }
}

//----------------------------------------------------------------------------
vtkKWPresetSelector::vtkKWPresetSelector()
{
  this->Internals = new vtkKWPresetSelectorInternals;
  this->Internals->PresetNodeCounter = 0;

  this->Internals->GroupSlotName        = "DefaultGroupSlot";
  this->Internals->CommentSlotName      = "DefaultCommentSlot";
  this->Internals->FileNameSlotName     = "DefaultFileNameSlot";
  this->Internals->CreationTimeSlotName = "DefaultCreationTimeSlot";
  this->Internals->ThumbnailSlotName    = "DefaultThumbnailSlot";
  this->Internals->ScreenshotSlotName   = "DefaultScreenshotSlot";

  this->Internals->SelectPreviousButtonLabel = 
    ks_("Preset Selector|Button|Previous");

  this->Internals->SelectNextButtonLabel = 
    ks_("Preset Selector|Button|Next");

  this->Internals->AddButtonLabel = 
    ks_("Preset Selector|Button|Add");

  this->Internals->ApplyButtonLabel = 
    ks_("Preset Selector|Button|Apply");

  this->Internals->UpdateButtonLabel = 
    ks_("Preset Selector|Button|Update");

  this->Internals->RemoveButtonLabel = 
    ks_("Preset Selector|Button|Remove");

  this->Internals->RemoveAllButtonLabel = 
    ks_("Preset Selector|Button|Remove All");

  this->Internals->LocateButtonLabel = 
    ks_("Preset Selector|Button|Locate");

  this->Internals->EmailButtonLabel = 
    ks_("Preset Selector|Button|Email");

  this->Internals->LoadButtonLabel = 
    ks_("Preset Selector|Button|Load");

  this->Internals->FilterButtonLabel = 
    ks_("Preset Selector|Button|Filter");

  this->PresetAddCommand                 = NULL;
  this->PresetUpdateCommand              = NULL;
  this->PresetApplyCommand               = NULL;
  this->PresetRemoveCommand              = NULL;
  this->PresetRemovedCommand             = NULL;
  this->PresetHasChangedCommand          = NULL;
  this->PresetLoadCommand                = NULL;
  this->PresetFilteringHasChangedCommand = NULL;

  this->CreateUserPresetButtonsCommand         = NULL;
  this->UpdateUserPresetButtonsCommand         = NULL;
  this->SetUserPresetButtonsIconsCommand       = NULL;
  this->SetUserPresetButtonsHelpStringsCommand = NULL;

  this->PresetList              = NULL;
  this->PresetControlFrame      = NULL;
  this->PresetButtons           = NULL;
  this->HelpLabel               = NULL;

  this->ApplyPresetOnSelection        = 1;
  this->SelectSpinButtonsVisibility   = 1;
  this->LocateButtonVisibility        = 0;
  this->LocateMenuEntryVisibility     = 0;
  this->RemoveButtonVisibility        = 1;
  this->RemoveMenuEntryVisibility     = 1;
  this->EmailButtonVisibility         = 0;
  this->EmailMenuEntryVisibility      = 0;
  this->HelpLabelVisibility           = 0;
  this->FilterButtonVisibility = 0;

  this->ThumbnailSize               = 32;
  this->ScreenshotSize              = 144;
  this->PromptBeforeRemovePreset    = 1;
  this->MaximumNumberOfPresets      = 0;

  this->EmailBody                 = NULL;
  this->FilterButtonSlotName                 = NULL;

  this->ContextMenu = NULL;

  this->Toolbar = NULL;

  this->PresetButtonsBaseIcon = NULL;
  this->SetPresetButtonsBaseIconToPredefinedIcon(vtkKWIcon::IconDocument);
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

  if (this->HelpLabel)
    {
    this->HelpLabel->Delete();
    this->HelpLabel = NULL;
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

  if (this->PresetRemovedCommand)
    {
    delete [] this->PresetRemovedCommand;
    this->PresetRemovedCommand = NULL;
    }

  if (this->PresetHasChangedCommand)
    {
    delete [] this->PresetHasChangedCommand;
    this->PresetHasChangedCommand = NULL;
    }

  if (this->PresetLoadCommand)
    {
    delete [] this->PresetLoadCommand;
    this->PresetLoadCommand = NULL;
    }

  if (this->PresetFilteringHasChangedCommand)
    {
    delete [] this->PresetFilteringHasChangedCommand;
    this->PresetFilteringHasChangedCommand = NULL;
    }

  if (this->CreateUserPresetButtonsCommand)
    {
    delete [] this->CreateUserPresetButtonsCommand;
    this->CreateUserPresetButtonsCommand = NULL;
    }

  if (this->UpdateUserPresetButtonsCommand)
    {
    delete [] this->UpdateUserPresetButtonsCommand;
    this->UpdateUserPresetButtonsCommand = NULL;
    }

  if (this->SetUserPresetButtonsIconsCommand)
    {
    delete [] this->SetUserPresetButtonsIconsCommand;
    this->SetUserPresetButtonsIconsCommand = NULL;
    }

  if (this->SetUserPresetButtonsHelpStringsCommand)
    {
    delete [] this->SetUserPresetButtonsHelpStringsCommand;
    this->SetUserPresetButtonsHelpStringsCommand = NULL;
    }

  // Remove all presets

  this->DeleteAllPresets();

  // Delete our pool

  delete this->Internals;
  this->Internals = NULL;

  if (this->ContextMenu)
    {
    this->ContextMenu->Delete();
    this->ContextMenu = NULL;
    }

  if (this->Toolbar)
    {
    this->Toolbar->Delete();
    this->Toolbar = NULL;
    }

  if (this->PresetButtonsBaseIcon)
    {
    this->PresetButtonsBaseIcon->Delete();
    this->PresetButtonsBaseIcon = NULL;
    }

  this->SetEmailBody(NULL);
  this->SetFilterButtonSlotName(NULL);
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  // --------------------------------------------------------------
  // Preset : preset list

  if (!this->PresetList)
    {
    this->PresetList = vtkKWMultiColumnListWithScrollbars::New();
    }

  this->PresetList->SetParent(this);
  this->PresetList->Create();
  //this->PresetList->HorizontalScrollbarVisibilityOff();

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
    list->SetSelectionModeToBrowse();
    }
  list->SetSelectionCommand(
    this, "SchedulePresetSelectionCallback");
  list->SetSelectionChangedCommand(
    this, "SchedulePresetSelectionChangedCallback");
  list->SetPotentialCellColorsChangedCommand(
    list, "ScheduleRefreshColorsOfAllCellsWithWindowCommand");
  list->ColumnSeparatorsVisibilityOn();
  list->ResizableColumnsOn();
  list->StretchableColumnsOn();
  list->SetEditStartCommand(this, "PresetCellEditStartCallback");
  list->SetEditEndCommand(this, "PresetCellEditEndCallback");
  list->SetCellUpdatedCommand(this, "PresetCellUpdatedCallback");
  list->SetRightClickCommand(this, "PresetRightClickCallback");
  list->SetColumnSortedCommand(this, "ColumnSortedCallback");
  list->SetRowMovedCommand(this, "RowMovedCallback");

  this->AddCallbackCommandObserver(
    list, vtkKWMultiColumnList::NumberOfRowsChangedEvent);

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
    this->PresetButtons = vtkKWToolbar::New();
    }

  this->PresetButtons->SetParent(this->PresetControlFrame);
  this->PresetButtons->Create();

  this->PresetButtons->SetWidgetsFlatAdditionalPadX(
    this->PresetButtons->GetWidgetsFlatAdditionalPadX() + 1);
  this->PresetButtons->SetWidgetsFlatAdditionalInternalPadX(
    this->PresetButtons->GetWidgetsFlatAdditionalInternalPadX() + 1);
  this->PresetButtons->SetWidgetsFlatAdditionalInternalPadY(
    this->PresetButtons->GetWidgetsFlatAdditionalInternalPadY() + 1);

  this->CreateToolbarPresetButtons(this->PresetButtons, 1);
  this->SetToolbarPresetButtonsIcons(this->PresetButtons);
  this->SetToolbarPresetButtonsHelpStrings(this->PresetButtons);

  // --------------------------------------------------------------
  // Help message

  if (!this->HelpLabel)
    {
    this->HelpLabel = vtkKWLabelWithLabel::New();
    }

  this->HelpLabel->SetParent(this->PresetControlFrame);
  this->HelpLabel->Create();
  this->HelpLabel->ExpandWidgetOn();
  this->HelpLabel->GetLabel()->SetImageToPredefinedIcon(
    vtkKWIcon::IconSilkHelp);

  vtkKWLabel *msg = this->HelpLabel->GetWidget();
  msg->SetJustificationToLeft();
  msg->SetAnchorToNorthWest();
  msg->AdjustWrapLengthToWidthOn();

  // Pack

  this->Pack();

  // Update enable state

  this->Update();
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::CreateToolbar()
{
  if (!this->Toolbar)
    {
    return;
    }

  if (!this->Toolbar->GetParent())
    {
    this->Toolbar->SetParent(this);
    }

  this->Toolbar->Create();
  this->Toolbar->SetWidgetsFlatAdditionalPadX(
    this->Toolbar->GetWidgetsFlatAdditionalPadX() + 1);
  this->Toolbar->SetWidgetsFlatAdditionalInternalPadX(
    this->Toolbar->GetWidgetsFlatAdditionalInternalPadX() + 1);

  this->CreateToolbarPresetButtons(this->Toolbar, 0);
  this->SetToolbarPresetButtonsIcons(this->Toolbar);
  this->SetToolbarPresetButtonsHelpStrings(this->Toolbar);
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetPresetButtonsBaseIcon(vtkKWIcon *icon)
{
  if (this->PresetButtonsBaseIcon == icon)
    {
    return;
    }

  if (this->PresetButtonsBaseIcon)
    {
    this->PresetButtonsBaseIcon->UnRegister(this);
    this->PresetButtonsBaseIcon = NULL;
    }

  if (icon)
    {
    this->PresetButtonsBaseIcon = icon;
    this->PresetButtonsBaseIcon->Register(this);
    }

  this->Modified();

  this->SetToolbarPresetButtonsIcons(this->PresetButtons);
  this->SetToolbarPresetButtonsIcons(this->Toolbar);
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetPresetButtonsBaseIconToPredefinedIcon(
  int icon_index)
{
  vtkKWIcon *icon = vtkKWIcon::New();
  icon->SetImage(icon_index);
  icon->TrimTop();
  icon->TrimRight();
  this->SetPresetButtonsBaseIcon(icon);
  icon->Delete();
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::CreateToolbarPresetButtons(
  vtkKWToolbar *toolbar, int use_separators)
{
  if (!toolbar)
    {
    return;
    }

  vtkKWPushButton *toolbar_pb;

  // Select previous preset

  toolbar_pb = vtkKWPushButton::New();
  toolbar_pb->SetParent(toolbar->GetFrame());
  toolbar_pb->Create();
  toolbar_pb->SetText(this->GetSelectPreviousButtonLabel());
  toolbar_pb->SetCommand(this, "SelectPreviousPreset");
  toolbar->AddWidget(toolbar_pb);
  toolbar_pb->Delete();

  // Select next preset

  toolbar_pb = vtkKWPushButton::New();
  toolbar_pb->SetParent(toolbar->GetFrame());
  toolbar_pb->Create();
  toolbar_pb->SetText(this->GetSelectNextButtonLabel());
  toolbar_pb->SetCommand(this, "SelectNextPreset");
  toolbar->AddWidget(toolbar_pb);
  toolbar_pb->Delete();

  if (use_separators)
    {
    toolbar->AddSeparator();
    }

  // Add preset

  toolbar_pb = vtkKWPushButton::New();
  toolbar_pb->SetParent(toolbar->GetFrame());
  toolbar_pb->Create();
  toolbar_pb->SetText(this->GetAddButtonLabel());
  toolbar_pb->SetCommand(this, "PresetAddCallback");
  toolbar->AddWidget(toolbar_pb);
  toolbar_pb->Delete();

  // Apply preset

  toolbar_pb = vtkKWPushButton::New();
  toolbar_pb->SetParent(toolbar->GetFrame());
  toolbar_pb->Create();
  toolbar_pb->SetText(this->GetApplyButtonLabel());
  toolbar_pb->SetCommand(this, "PresetApplyCallback");
  toolbar->AddWidget(toolbar_pb);
  toolbar_pb->Delete();

  // Update preset

  toolbar_pb = vtkKWPushButton::New();
  toolbar_pb->SetParent(toolbar->GetFrame());
  toolbar_pb->Create();
  toolbar_pb->SetText(this->GetUpdateButtonLabel());
  toolbar_pb->SetCommand(this, "PresetUpdateCallback");
  toolbar->AddWidget(toolbar_pb);
  toolbar_pb->Delete();

  // Remove preset

  toolbar_pb = vtkKWPushButton::New();
  toolbar_pb->SetParent(toolbar->GetFrame());
  toolbar_pb->Create();
  toolbar_pb->SetText(this->GetRemoveButtonLabel());
  toolbar_pb->SetCommand(this, "PresetRemoveCallback");
  toolbar->AddWidget(toolbar_pb);
  toolbar_pb->Delete();

  if (use_separators)
    {
    toolbar->AddSeparator();
    }

  // Locate preset

  toolbar_pb = vtkKWPushButton::New();
  toolbar_pb->SetParent(toolbar->GetFrame());
  toolbar_pb->Create();
  toolbar_pb->SetText(this->GetLocateButtonLabel());
  toolbar_pb->SetCommand(this, "PresetLocateCallback");
  toolbar->AddWidget(toolbar_pb);
  toolbar_pb->Delete();

  // Email preset

  toolbar_pb = vtkKWPushButton::New();
  toolbar_pb->SetParent(toolbar->GetFrame());
  toolbar_pb->Create();
  toolbar_pb->SetText(this->GetEmailButtonLabel());
  toolbar_pb->SetCommand(this, "PresetEmailCallback");
  toolbar->AddWidget(toolbar_pb);
  toolbar_pb->Delete();

  // Load preset

  toolbar_pb = vtkKWPushButton::New();
  toolbar_pb->SetParent(toolbar->GetFrame());
  toolbar_pb->Create();
  toolbar_pb->SetText(this->GetLoadButtonLabel());
  toolbar_pb->SetCommand(this, "PresetLoadCallback");
  toolbar->AddWidget(toolbar_pb);
  toolbar_pb->Delete();

  if (use_separators)
    {
    toolbar->AddSeparator();
    }

  // Filter By Group preset

  toolbar_pb = vtkKWPushButton::New();
  toolbar_pb->SetParent(toolbar->GetFrame());
  toolbar_pb->Create();
  toolbar_pb->SetText(this->GetFilterButtonLabel());
  toolbar_pb->SetCommand(this, "PresetFilterCallback");
  toolbar->AddWidget(toolbar_pb);
  toolbar_pb->Delete();

  // User buttons

  this->InvokeCreateUserPresetButtonsCommand(toolbar, use_separators);
}

//---------------------------------------------------------------------------
void vtkKWPresetSelector::UpdateToolbarPresetButtons(vtkKWToolbar *toolbar)
{
  if (!toolbar)
    {
    return;
    }

  toolbar->SetEnabled(this->GetEnabled());

  int has_selection = 
    (this->PresetList && 
     this->PresetList->GetWidget()->GetNumberOfSelectedCells());

  int has_filenames_in_selection = 
    has_selection ? this->GetNumberOfSelectedPresetsWithFileName() : 0;

  int has_presets = this->GetNumberOfVisiblePresets();

  vtkKWPushButton *toolbar_pb;

  // Select prev

  toolbar_pb = vtkKWPushButton::SafeDownCast(
    toolbar->GetWidget(this->GetSelectPreviousButtonLabel()));
  if (toolbar_pb)
    {
    toolbar->SetWidgetVisibility(
      toolbar_pb, this->SelectSpinButtonsVisibility);
    toolbar_pb->SetEnabled(
      has_presets ? toolbar->GetEnabled() : 0);
    }

  // Select next

  toolbar_pb = vtkKWPushButton::SafeDownCast(
    toolbar->GetWidget(this->GetSelectNextButtonLabel()));
  if (toolbar_pb)
    {
    toolbar->SetWidgetVisibility(
      toolbar_pb, this->SelectSpinButtonsVisibility);
    toolbar_pb->SetEnabled(
      has_presets ? toolbar->GetEnabled() : 0);
    }

  // Add

  toolbar_pb = vtkKWPushButton::SafeDownCast(
    toolbar->GetWidget(this->GetAddButtonLabel()));
  if (toolbar_pb)
    {
    toolbar->SetWidgetVisibility(
      toolbar_pb, (this->PresetAddCommand && *this->PresetAddCommand) ? 1 : 0);
    }

  // Apply

  toolbar_pb = vtkKWPushButton::SafeDownCast(
    toolbar->GetWidget(this->GetApplyButtonLabel()));
  if (toolbar_pb)
    {
    toolbar->SetWidgetVisibility(
      toolbar_pb,
      this->PresetApplyCommand && *this->PresetApplyCommand &&
      !this->ApplyPresetOnSelection ? 1 : 0);
    toolbar_pb->SetEnabled(
      has_selection ? toolbar->GetEnabled() : 0);
    }

  // Update

  toolbar_pb = vtkKWPushButton::SafeDownCast(
    toolbar->GetWidget(this->GetUpdateButtonLabel()));
  if (toolbar_pb)
    {
    toolbar->SetWidgetVisibility(
      toolbar_pb,
      (this->PresetUpdateCommand && *this->PresetUpdateCommand) ? 1 : 0);
    toolbar_pb->SetEnabled(
      has_selection ? toolbar->GetEnabled() : 0);
    }

  // Remove

  toolbar_pb = vtkKWPushButton::SafeDownCast(
    toolbar->GetWidget(this->GetRemoveButtonLabel()));
  if (toolbar_pb)
    {
    toolbar->SetWidgetVisibility(
      toolbar_pb, this->RemoveButtonVisibility ? 1 : 0);
    toolbar_pb->SetEnabled(
      has_selection ? toolbar->GetEnabled() : 0);
    }

  // Locate

  toolbar_pb = vtkKWPushButton::SafeDownCast(
    toolbar->GetWidget(this->GetLocateButtonLabel()));
  if (toolbar_pb)
    {
    toolbar->SetWidgetVisibility(
      toolbar_pb, this->LocateButtonVisibility ? 1 : 0);
    toolbar_pb->SetEnabled(
      has_filenames_in_selection ? toolbar->GetEnabled() : 0);
    }

  // Email

  toolbar_pb = vtkKWPushButton::SafeDownCast(
    toolbar->GetWidget(this->GetEmailButtonLabel()));
  if (toolbar_pb)
    {
    toolbar->SetWidgetVisibility(
      toolbar_pb, this->EmailButtonVisibility ? 1 : 0);
    toolbar_pb->SetEnabled(
      has_filenames_in_selection ? toolbar->GetEnabled() : 0);
    }

  // Load

  toolbar_pb = vtkKWPushButton::SafeDownCast(
    toolbar->GetWidget(this->GetLoadButtonLabel()));
  if (toolbar_pb)
    {
    toolbar->SetWidgetVisibility(
      toolbar_pb,
      (this->PresetLoadCommand && *this->PresetLoadCommand) ? 1 : 0);
    }

  // Filter By Group

  toolbar_pb = vtkKWPushButton::SafeDownCast(
    toolbar->GetWidget(this->GetFilterButtonLabel()));
  if (toolbar_pb)
    {
    toolbar->SetWidgetVisibility(
      toolbar_pb, this->FilterButtonVisibility ? 1 : 0);
    toolbar_pb->SetEnabled(
      this->FilterButtonSlotName ? toolbar->GetEnabled() : 0);
    }

  // User buttons

  this->InvokeUpdateUserPresetButtonsCommand(toolbar);
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetToolbarPresetButtonsIcons(vtkKWToolbar *toolbar)
{
  if (!toolbar)
    {
    return;
    }

  vtkKWPushButton *toolbar_pb;

  vtkKWIcon *icon = NULL;
  if (this->PresetButtonsBaseIcon)
    {
    icon = vtkKWIcon::New();
    }

  // Select previous preset

  toolbar_pb = vtkKWPushButton::SafeDownCast(
    toolbar->GetWidget(this->GetSelectPreviousButtonLabel()));
  if (toolbar_pb)
    {
    if (this->PresetButtonsBaseIcon)
      {
      icon->SetImage(this->PresetButtonsBaseIcon);
      icon->Compose(vtkKWIcon::IconPresetPrevious);
      toolbar_pb->SetImageToIcon(icon);
      }
    else
      {
      toolbar_pb->SetImageToPredefinedIcon(vtkKWIcon::IconPresetPrevious);
      }
    }

  // Select next preset

  toolbar_pb = vtkKWPushButton::SafeDownCast(
    toolbar->GetWidget(this->GetSelectNextButtonLabel()));
  if (toolbar_pb)
    {
    if (this->PresetButtonsBaseIcon)
      {
      icon->SetImage(this->PresetButtonsBaseIcon);
      icon->Compose(vtkKWIcon::IconPresetNext);
      toolbar_pb->SetImageToIcon(icon);
      }
    else
      {
      toolbar_pb->SetImageToPredefinedIcon(vtkKWIcon::IconPresetNext);
      }
    }
  
  // Add preset

  toolbar_pb = vtkKWPushButton::SafeDownCast(
    toolbar->GetWidget(this->GetAddButtonLabel()));
  if (toolbar_pb)
    {
    if (this->PresetButtonsBaseIcon)
      {
      icon->SetImage(this->PresetButtonsBaseIcon);
      icon->Compose(vtkKWIcon::IconPresetAdd);
      toolbar_pb->SetImageToIcon(icon);
      }
    else
      {
      toolbar_pb->SetImageToPredefinedIcon(vtkKWIcon::IconPresetAdd);
      }
    }

  // Apply preset

  toolbar_pb = vtkKWPushButton::SafeDownCast(
    toolbar->GetWidget(this->GetApplyButtonLabel()));
  if (toolbar_pb)
    {
    if (this->PresetButtonsBaseIcon)
      {
      icon->SetImage(this->PresetButtonsBaseIcon);
      icon->Compose(vtkKWIcon::IconPresetApply);
      toolbar_pb->SetImageToIcon(icon);
      }
    else
      {
      toolbar_pb->SetImageToPredefinedIcon(vtkKWIcon::IconPresetApply);
      }
    }
  
  // Update preset

  toolbar_pb = vtkKWPushButton::SafeDownCast(
    toolbar->GetWidget(this->GetUpdateButtonLabel()));
  if (toolbar_pb)
    {
    if (this->PresetButtonsBaseIcon)
      {
      icon->SetImage(this->PresetButtonsBaseIcon);
      icon->Compose(vtkKWIcon::IconPresetUpdate);
      toolbar_pb->SetImageToIcon(icon);
      }
    else
      {
      toolbar_pb->SetImageToPredefinedIcon(vtkKWIcon::IconPresetUpdate);
      }
    }

  // Remove preset

  toolbar_pb = vtkKWPushButton::SafeDownCast(
    toolbar->GetWidget(this->GetRemoveButtonLabel()));
  if (toolbar_pb)
    {
    if (this->PresetButtonsBaseIcon)
      {
      icon->SetImage(this->PresetButtonsBaseIcon);
      icon->Compose(vtkKWIcon::IconPresetDelete);
      toolbar_pb->SetImageToIcon(icon);
      }
    else
      {
      toolbar_pb->SetImageToPredefinedIcon(vtkKWIcon::IconPresetDelete);
      }
    }
  
  // Locate preset

  toolbar_pb = vtkKWPushButton::SafeDownCast(
    toolbar->GetWidget(this->GetLocateButtonLabel()));
  if (toolbar_pb)
    {
    if (this->PresetButtonsBaseIcon)
      {
      icon->SetImage(this->PresetButtonsBaseIcon);
      icon->Compose(vtkKWIcon::IconPresetLocate);
      toolbar_pb->SetImageToIcon(icon);
      }
    else
      {
      toolbar_pb->SetImageToPredefinedIcon(vtkKWIcon::IconPresetLocate);
      }
    }
  
  // Email preset

  toolbar_pb = vtkKWPushButton::SafeDownCast(
    toolbar->GetWidget(this->GetEmailButtonLabel()));
  if (toolbar_pb)
    {
    if (this->PresetButtonsBaseIcon)
      {
      icon->SetImage(this->PresetButtonsBaseIcon);
      icon->Compose(vtkKWIcon::IconPresetEmail);
      toolbar_pb->SetImageToIcon(icon);
      }
    else
      {
      toolbar_pb->SetImageToPredefinedIcon(vtkKWIcon::IconPresetEmail);
      }
    }

  // Load preset

  toolbar_pb = vtkKWPushButton::SafeDownCast(
    toolbar->GetWidget(this->GetLoadButtonLabel()));
  if (toolbar_pb)
    {
    if (this->PresetButtonsBaseIcon)
      {
      icon->SetImage(this->PresetButtonsBaseIcon);
      icon->Compose(vtkKWIcon::IconPresetLoad);
      toolbar_pb->SetImageToIcon(icon);
      }
    else
      {
      toolbar_pb->SetImageToPredefinedIcon(vtkKWIcon::IconPresetLoad);
      }
    }

  // Filter By Group

  toolbar_pb = vtkKWPushButton::SafeDownCast(
    toolbar->GetWidget(this->GetFilterButtonLabel()));
  if (toolbar_pb)
    {
    if (this->PresetButtonsBaseIcon)
      {
      icon->SetImage(this->PresetButtonsBaseIcon);
      icon->Compose(vtkKWIcon::IconPresetFilter);
      toolbar_pb->SetImageToIcon(icon);
      }
    else
      {
      toolbar_pb->SetImageToPredefinedIcon(vtkKWIcon::IconPresetFilter);
      }
    }
   
  // Release icon

  if (icon)
    {
    icon->Delete();
    }

  // User buttons

  this->InvokeSetUserPresetButtonsIconsCommand(toolbar);
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetToolbarPresetButtonsHelpStrings(vtkKWToolbar *toolbar)
{
  if (!toolbar)
    {
    return;
    }

  vtkKWWidget *toolbar_pb;

  // Select previous preset

  toolbar_pb = toolbar->GetWidget(this->GetSelectPreviousButtonLabel());
  if (toolbar_pb)
    {
    toolbar_pb->SetBalloonHelpString(
      ks_("Preset Selector|Select previous preset"));
    }

  // Select next preset

  toolbar_pb = toolbar->GetWidget(this->GetSelectNextButtonLabel());
  if (toolbar_pb)
    {
    toolbar_pb->SetBalloonHelpString(
      ks_("Preset Selector|Select next preset"));
    }
  
  // Add preset

  toolbar_pb = toolbar->GetWidget(this->GetAddButtonLabel());
  if (toolbar_pb)
    {
    toolbar_pb->SetBalloonHelpString(
      ks_("Preset Selector|Add a preset"));
    }

  // Apply preset

  toolbar_pb = toolbar->GetWidget(this->GetApplyButtonLabel());
  if (toolbar_pb)
    {
    toolbar_pb->SetBalloonHelpString(
      ks_("Preset Selector|Apply the selected preset(s)"));
    }
  
  // Update preset

  toolbar_pb = toolbar->GetWidget(this->GetUpdateButtonLabel());
  if (toolbar_pb)
    {
    toolbar_pb->SetBalloonHelpString(
      ks_("Preset Selector|Update the selected preset(s)"));
    }

  // Remove preset

  toolbar_pb = toolbar->GetWidget(this->GetRemoveButtonLabel());
  if (toolbar_pb)
    {
    toolbar_pb->SetBalloonHelpString(
      ks_("Preset Selector|Delete the selected preset(s)"));
    }
  
  // Locate preset

  toolbar_pb = toolbar->GetWidget(this->GetLocateButtonLabel());
  if (toolbar_pb)
    {
    toolbar_pb->SetBalloonHelpString(
      ks_("Preset Selector|Find the location of the selected preset(s) on disk"));
    }
  
  // Email preset

  toolbar_pb = toolbar->GetWidget(this->GetEmailButtonLabel());
  if (toolbar_pb)
    {
    toolbar_pb->SetBalloonHelpString(
      ks_("Preset Selector|Email the selected preset(s)"));
    }

  // Load preset

  toolbar_pb = toolbar->GetWidget(this->GetLoadButtonLabel());
  if (toolbar_pb)
    {
    toolbar_pb->SetBalloonHelpString(
      ks_("Preset Selector|Load a preset"));
    }

  // Filter By Group preset

  toolbar_pb = toolbar->GetWidget(this->GetFilterButtonLabel());
  if (toolbar_pb)
    {
    toolbar_pb->SetBalloonHelpString(
      ks_("Preset Selector|Filter preset(s)"));
    }

  // User buttons

  this->InvokeSetUserPresetButtonsHelpStringsCommand(toolbar);
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::PopulatePresetContextMenu(vtkKWMenu *menu, int id)
{
  if (!this->HasPreset(id))
    {
    return;
    }

  char command[256], label[256];

  int tcl_major, tcl_minor, tcl_patch_level;
  Tcl_GetVersion(&tcl_major, &tcl_minor, &tcl_patch_level, NULL);
  int show_icons = (tcl_major > 8 || (tcl_major == 8 && tcl_minor >= 5));

  int index;
  vtkKWToolbar *toolbar = this->PresetButtons;
  vtkKWPushButton *toolbar_pb;

  const char *filename = this->GetPresetFileName(id);
  int has_file = 
    (filename && *filename && vtksys::SystemTools::FileExists(filename));
  
  // Apply preset

  if (this->PresetApplyCommand)
    {
    sprintf(command, "PresetApplyCallback %d", id);
    index = menu->AddCommand(this->GetApplyButtonLabel(), this, command);
    if (show_icons)
      {
      toolbar_pb = vtkKWPushButton::SafeDownCast(
        toolbar->GetWidget(this->GetApplyButtonLabel()));
      menu->SetItemImage(index, toolbar_pb->GetConfigurationOption("-image"));
      menu->SetItemCompoundModeToLeft(index);
      }
    }

  // Update preset

  if (this->PresetUpdateCommand)
    {
    sprintf(command, "PresetUpdateCallback %d", id);
    index = menu->AddCommand(this->GetUpdateButtonLabel(), this, command);
    if (show_icons)
      {
      toolbar_pb = vtkKWPushButton::SafeDownCast(
        toolbar->GetWidget(this->GetUpdateButtonLabel()));
      menu->SetItemImage(index, toolbar_pb->GetConfigurationOption("-image"));
      menu->SetItemCompoundModeToLeft(index);
      }
    }

  // Remove preset

  if (this->RemoveMenuEntryVisibility)
    {
    sprintf(command, "PresetRemoveCallback %d", id);
    index = menu->AddCommand(this->GetRemoveButtonLabel(), this, command);
    if (show_icons)
      {
      toolbar_pb = vtkKWPushButton::SafeDownCast(
        toolbar->GetWidget(this->GetRemoveButtonLabel()));
      menu->SetItemImage(index, toolbar_pb->GetConfigurationOption("-image"));
      menu->SetItemCompoundModeToLeft(index);
      }

    index = menu->AddCommand(
      this->GetRemoveAllButtonLabel(), this, "PresetRemoveAllCallback");
    }

  // Locate preset

  if (has_file && this->LocateMenuEntryVisibility)
    {
    sprintf(command, "PresetLocateCallback %d", id);
    index = menu->AddCommand(this->GetLocateButtonLabel(), this, command);
    if (show_icons)
      {
      toolbar_pb = vtkKWPushButton::SafeDownCast(
        toolbar->GetWidget(this->GetLocateButtonLabel()));
      menu->SetItemImage(index, toolbar_pb->GetConfigurationOption("-image"));
      menu->SetItemCompoundModeToLeft(index);
      }
    }

  // Email preset

  if (has_file && this->EmailMenuEntryVisibility)
    {
    sprintf(command, "PresetEmailCallback %d", id);
    index = menu->AddCommand(this->GetEmailButtonLabel(), this, command);
    if (show_icons)
      {
      toolbar_pb = vtkKWPushButton::SafeDownCast(
        toolbar->GetWidget(this->GetEmailButtonLabel()));
      menu->SetItemImage(index, toolbar_pb->GetConfigurationOption("-image"));
      menu->SetItemCompoundModeToLeft(index);
      }
    }

  // now the editable fields

  vtkKWMultiColumnList *list = this->PresetList->GetWidget();
  int added_editable = 0, col_vis = 0, nb_columns = list->GetNumberOfColumns();
  for (int col = 0; col < nb_columns; col++)
    {
    if (list->GetColumnVisibility(col))
      {
      col_vis++;
      if (list->GetColumnEditable(col))
        {
        int row = this->GetPresetRow(id);
        if (row >= 0 && list->GetCellEditable(row, col))
          
          {
          sprintf(command, "EditCell %d %d", row, col);
          const char *col_name = list->GetColumnName(col);
          if (col_name)
            {
            sprintf(label, "Edit %s", col_name);
            }
          else
            {
            sprintf(label, "Edit column %d", col_vis);
            }
          if (!added_editable)
            {
            menu->AddSeparator();
            added_editable = 1;
            }
          menu->AddCommand(label, list, command);
          }
        }
      }
    }
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
    this->Script("pack %s -side top -anchor nw -fill none -expand t",
                 this->PresetButtons->GetWidgetName());
    }

  if (this->HelpLabelVisibility && this->HelpLabel)
    {
    this->Script("pack %s -side bottom -anchor nw -fill x -expand n -after %s",
                 this->HelpLabel->GetWidgetName(),
                 this->PresetButtons->GetWidgetName());
    }
  else
    {
    this->HelpLabel->Unpack();
    }
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::CreateColumns()
{
  vtkKWMultiColumnList *list = this->PresetList->GetWidget();

  int col;

  // We need that column to retrieve the Id

  col = list->AddColumn(ks_("Preset Selector|Column|Id"));
  list->SetColumnName(col, vtkKWPresetSelector::IdColumnName);
  list->ColumnVisibilityOff(col);

  // Thumbnail

  col = list->AddColumn(ks_("Preset Selector|Column|Image"));
  list->SetColumnName(col, vtkKWPresetSelector::ThumbnailColumnName);
  list->SetColumnWidth(col, -this->ThumbnailSize);
  list->SetColumnResizable(col, 1);
  list->SetColumnStretchable(col, 0);
  list->SetColumnEditable(col, 0);
  list->SetColumnSortModeToReal(col);
  list->SetColumnFormatCommandToEmptyOutput(col);
  list->ColumnVisibilityOff(col);

  // Group

  col = list->AddColumn(ks_("Preset Selector|Column|Group"));
  list->SetColumnName(col, vtkKWPresetSelector::GroupColumnName);
  list->SetColumnResizable(col, 1);
  list->SetColumnStretchable(col, 0);
  list->SetColumnEditable(col, 0);
  list->ColumnVisibilityOff(col);

  // Comment

  col = list->AddColumn(ks_("Preset Selector|Column|Comment"));
  list->SetColumnName(col, vtkKWPresetSelector::CommentColumnName);
  list->SetColumnResizable(col, 1);
  list->SetColumnStretchable(col, 1);
  list->SetColumnEditable(col, 1);
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
      this->PresetList->GetWidget()->SetSelectionModeToBrowse();
      }
    }

  // visibility of the apply preset button
  this->UpdateToolbarPresetButtons(this->PresetButtons); 
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
void vtkKWPresetSelector::SetFilterButtonVisibility(int arg)
{
  if (this->FilterButtonVisibility == arg)
    {
    return;
    }

  this->FilterButtonVisibility = arg;
  this->Modified();

  this->Update();
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetHelpLabelVisibility(int arg)
{
  if (this->HelpLabelVisibility == arg)
    {
    return;
    }

  this->HelpLabelVisibility = arg;
  this->Modified();

  this->Pack();
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetHelpLabelText(const char *str)
{
  if (this->HelpLabel)
    {
    this->HelpLabel->GetWidget()->SetText(str);
    }
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
void vtkKWPresetSelector::SetListWidth(int h)
{
  if (this->PresetList)
    {
    this->PresetList->GetWidget()->SetWidth(h);
    }
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::GetListWidth()
{
  if (this->PresetList)
    {
    return this->PresetList->GetWidget()->GetWidth();
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetThumbnailColumnVisibility(int arg)
{
  if (this->PresetList && this->GetThumbnailColumnVisibility() != arg)
    {
    this->PresetList->GetWidget()->SetColumnVisibility(
      this->GetThumbnailColumnIndex(), arg);
    this->ScheduleUpdatePresetRows(); // fill thumbnail column
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
  if (this->PresetList && this->GetCommentColumnVisibility() != arg)
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
  if (this->PresetList && this->GetGroupColumnVisibility() != arg)
    {
    this->PresetList->GetWidget()->SetColumnVisibility(
      this->GetGroupColumnIndex(), arg);
    this->ScheduleUpdatePresetRows(); // fill group column
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
  vtkKWPresetSelectorInternals::PresetNode *node = 
    new vtkKWPresetSelectorInternals::PresetNode;

  node->Id = this->Internals->PresetNodeCounter++;

  this->Internals->PresetPool.push_back(node);
  this->Internals->SortedPresetPool[node->Id] = node;

  this->ConfigureNewPreset(node->Id);

  this->ConstrainNumberOfPresets();
  this->NumberOfPresetsHasChanged();

  return node->Id;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::InsertPreset(int id)
{
  vtkKWPresetSelectorInternals::PresetPoolIterator it;
  if (id < 0)
    {
    it = this->Internals->PresetPool.begin();
    }
  else
    {
    it = this->Internals->GetPresetNodeIterator(id);
    if (it == this->Internals->PresetPool.end())
      {
      return -1;
      }
    }
  
  vtkKWPresetSelectorInternals::PresetNode *node = 
    new vtkKWPresetSelectorInternals::PresetNode;

  node->Id = this->Internals->PresetNodeCounter++;

  this->Internals->PresetPool.insert(it, node);
  this->Internals->SortedPresetPool[node->Id] = node;

  this->ConfigureNewPreset(node->Id);

  this->ConstrainNumberOfPresets();
  this->NumberOfPresetsHasChanged();

  return node->Id;
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::ConfigureNewPreset(int id)
{
  int row = this->UpdatePresetRowInMultiColumnList(id, 1);

  // Even though ScheduleUpdatePresetRow() is called by SetPresetCreationTime
  // let's update the row *now* so that people can both add a preset *and*
  // select it or see it right away programmatically. If we rely on 
  // ScheduleUpdatePresetRow, the row will *not* be created on time once
  // we hit SeeRow or SelectPreset, it will only be created the next time
  // the event loop is idle enough to do so.
  // Also, if some rows were schedules to be updated, do so now, otherwise
  // some rows may popup after this one.
  // UPDATE: not needed anymore, call UpdatePresetRowInMultiColumnList
  // to create the row with the id, which is more lightweight than updating
  // the full row contents.
  //this->CancelScheduleUpdatePresetRows();
  //this->UpdatePresetRow(id); 
  
  vtkTypeInt64 c_time = 
    (vtkTypeInt64)(vtksys::SystemTools::GetTime() * 1000.0);
  this->SetPresetCreationTime(id, c_time);

  if (this->PresetList && this->PresetList->IsMapped() && row >= 0)
    {
    this->PresetList->GetWidget()->SeeRow(row);
    }
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::RemovePreset(int id)
{
  if (this->Internals)
    {
    vtkKWPresetSelectorInternals::PresetPoolIterator it = 
      this->Internals->GetPresetNodeIterator(id);
    if (it != this->Internals->PresetPool.end())
      {
      vtkKWPresetSelectorInternals::SortedPresetPoolIterator it2 = 
        this->Internals->GetSortedPresetNodeIterator(id);
      vtkKWPresetSelectorInternals::PresetNode *preset = *it; // it->second;
      this->DeAllocatePreset(id);
      if (this->PresetList)
        {
        int row = this->GetPresetRow(id);
        if (row >= 0)
          {
          this->PresetList->GetWidget()->DeleteRow(row);
          }
        }
      delete preset;
      this->Internals->PresetPool.erase(it);
      this->Internals->SortedPresetPool.erase(it2);
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
      vtkKWPresetSelectorInternals::PresetNode *preset = *it; // it->second;
      this->DeAllocatePreset(preset->Id);
      }

    // Then remove the presets

    it = this->Internals->PresetPool.begin();
    for (; it != end; ++it)
      {
      vtkKWPresetSelectorInternals::PresetNode *preset = *it; // it->second;
      delete preset;
      }
    this->Internals->PresetPool.clear();
    this->Internals->SortedPresetPool.clear();

    return nb_deleted;
    }

  return 0;
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
      vtkKWPresetSelectorInternals::PresetNode *preset = *it; // it->second;
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        preset->UserSlotPool.find(this->GetPresetGroupSlotName());
      if (s_it != preset->UserSlotPool.end() &&
          !s_it->second.StringValue.compare(group))
        {
        this->DeAllocatePreset(preset->Id);
        if (this->PresetList)
          {
          int row = this->GetPresetRow(preset->Id);
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
        vtkKWPresetSelectorInternals::PresetNode *preset = *it; // it->second;
        vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
          preset->UserSlotPool.find(this->GetPresetGroupSlotName());
        if (s_it != preset->UserSlotPool.end() &&
            !s_it->second.StringValue.compare(group))
          {
          vtkKWPresetSelectorInternals::SortedPresetPoolIterator it2 = 
            this->Internals->GetSortedPresetNodeIterator(preset->Id);
          delete preset;
          this->Internals->PresetPool.erase(it);
          this->Internals->SortedPresetPool.erase(it2);
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
int vtkKWPresetSelector::RemoveAllPresets()
{
  int nb_deleted = this->DeleteAllPresets();

  if (this->PresetList && this->PresetList->GetWidget()->GetNumberOfRows())
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
int vtkKWPresetSelector::HasPreset(int id)
{
  if (this->Internals)
    {
    vtkKWPresetSelectorInternals::PresetNode *node =
      this->Internals->GetPresetNode(id);
    return node ? 1 : 0;
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
    this->ScheduleUpdatePresetRows();
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
  if (res && this->GetPresetFilterGroupConstraint())
    {
    // Changing the group of a preset may change the number of visible widgets
    // (for example, if the visibility of presets is filtered by groups), 
    // which can enable/disable some buttons
    this->PresetFilteringMayHaveChanged();
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
    this->ScheduleUpdatePresetRows();
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
    this->ScheduleUpdatePresetRows();
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
int vtkKWPresetSelector::GetIdOfPresetWithFileName(const char *filename)
{
  if (this->Internals && filename)
    {
    int is_full = vtksys::SystemTools::FileIsFullPath(filename) ? 1 : 0;

    vtkKWPresetSelectorInternals::PresetPoolIterator it = 
      this->Internals->PresetPool.begin();
    vtkKWPresetSelectorInternals::PresetPoolIterator end = 
      this->Internals->PresetPool.end();
    for (; it != end; it++)
      {
      vtkKWPresetSelectorInternals::PresetNode *preset = *it; // it->second;
      const char *it_filename = this->GetPresetFileName(preset->Id);
      if (it_filename)
        {
        if (is_full)
          {
          if (!strcmp(it_filename, filename))
            {
            return preset->Id;
            }
          }
        else
          {
          vtksys_stl::string it_name = 
            vtksys::SystemTools::GetFilenameName(it_filename);
          if (!strcmp(it_name.c_str(), filename))
            {
            return preset->Id;
            }
          }
        }
      }
    }

  return -1;
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetPresetCreationTimeSlotName(const char *name)
{
  if (name && *name && 
      this->Internals && this->Internals->CreationTimeSlotName.compare(name))
    {
    this->Internals->CreationTimeSlotName = name;
    this->ScheduleUpdatePresetRows();
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
int vtkKWPresetSelector::SetPresetCreationTime(int id, vtkTypeInt64 value)
{
  return this->SetPresetUserSlotAsInt64(
    id, this->GetPresetCreationTimeSlotName(), value);
}

//----------------------------------------------------------------------------
vtkTypeInt64 vtkKWPresetSelector::GetPresetCreationTime(int id)
{
  return this->GetPresetUserSlotAsInt64(
    id, this->GetPresetCreationTimeSlotName());
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetPresetThumbnailSlotName(const char *name)
{
  if (name && *name && 
      this->Internals && this->Internals->ThumbnailSlotName.compare(name))
    {
    this->Internals->ThumbnailSlotName = name;
    this->ScheduleUpdatePresetRows();
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
    if (icon)
      {
      vtkKWIcon *ptr = this->GetPresetThumbnail(id);
      if (!ptr)
        {
        ptr = vtkKWIcon::New();
        ptr->DeepCopy(icon);
        this->SetPresetUserSlotAsObject(
          id, this->GetPresetThumbnailSlotName(), ptr);
        ptr->Delete();
        }
      else
        {
        ptr->DeepCopy(icon);
        this->ScheduleUpdatePresetRow(id);
        }
      }
    else
      {
      this->DeletePresetUserSlot(id, this->GetPresetThumbnailSlotName());
      }
    return 1;
    }

  return 0;
}

//----------------------------------------------------------------------------
vtkKWIcon* vtkKWPresetSelector::GetPresetThumbnail(int id)
{
  return (vtkKWIcon*)this->GetPresetUserSlotAsObject(
    id, this->GetPresetThumbnailSlotName());
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetPresetScreenshotSlotName(const char *name)
{
  if (name && *name && 
      this->Internals && this->Internals->ScreenshotSlotName.compare(name))
    {
    this->Internals->ScreenshotSlotName = name;
    this->ScheduleUpdatePresetRows();
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
    if (icon)
      {
      vtkKWIcon *ptr = this->GetPresetScreenshot(id);
      if (!ptr)
        {
        ptr = vtkKWIcon::New();
        ptr->DeepCopy(icon);
        this->SetPresetUserSlotAsObject(
          id, this->GetPresetScreenshotSlotName(), ptr);
        ptr->Delete();
        }
      else
        {
        ptr->DeepCopy(icon);
        this->ScheduleUpdatePresetRow(id);
        }
      }
    else
      {
      this->DeletePresetUserSlot(id, this->GetPresetScreenshotSlotName());
      }
    return 1;
    }

  return 0;
}

//----------------------------------------------------------------------------
vtkKWIcon* vtkKWPresetSelector::GetPresetScreenshot(int id)
{
  return (vtkKWIcon*)this->GetPresetUserSlotAsObject(
    id, this->GetPresetScreenshotSlotName());
}

//---------------------------------------------------------------------------
int vtkKWPresetSelector::BuildPresetThumbnailAndScreenshotFromImage(
  int id, vtkImageData *image)
{
#ifdef KWWidgets_BUILD_VTK_WIDGETS
  if (!this->HasPreset(id))
    {
    return 0;
    }

  // Empty image, remove thumbnail/screenshot

  int image_dims[3];
  image->GetDimensions(image_dims);
  if (image_dims[0] == 0 || 
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

  int clip_dims[3];
  clip->GetOutput()->GetDimensions(clip_dims);

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
  int resample_input_dims[3], resample_output_dims[3];

  resample_input->GetDimensions(resample_input_dims);
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
  resample_output->GetDimensions(resample_output_dims);

  vtkKWIcon *screenshot = vtkKWIcon::New();
  screenshot->SetImage(
    (const unsigned char*)resample_output->GetScalarPointer(),
    resample_output_dims[0],
    resample_output_dims[1],
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
  resample_output->GetDimensions(resample_output_dims);

  vtkKWIcon *thumbnail = vtkKWIcon::New();
  thumbnail->SetImage(
    (const unsigned char*)resample_output->GetScalarPointer(),
    resample_output_dims[0],
    resample_output_dims[1],
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

  this->ScheduleUpdatePresetRow(id);
#endif
  return 1;
}

//---------------------------------------------------------------------------
int vtkKWPresetSelector::BuildPresetThumbnailAndScreenshotFromRenderWindow(
  int id, vtkRenderWindow *win)
{
#ifdef KWWidgets_BUILD_VTK_WIDGETS
  if (win)
    {
    vtkWindowToImageFilter *filter = vtkWindowToImageFilter::New();
    filter->ShouldRerenderOff();
    filter->SetInput(win);
    filter->Update();
    int res = this->BuildPresetThumbnailAndScreenshotFromImage(
      id, filter->GetOutput());
    filter->Delete();
    return res;
    }
#endif
  return 0;
}

//---------------------------------------------------------------------------
int vtkKWPresetSelector::FlipPresetThumbnailAndScreenshotVertically(
  int id)
{
  if (!this->HasPreset(id))
    {
    return 0;
    }

  int modified = 0;

  vtkKWIcon *screenshot = this->GetPresetScreenshot(id);
  if (screenshot)
    {
    screenshot->SetImage(screenshot->GetData(),
                         screenshot->GetWidth(),
                         screenshot->GetHeight(),
                         screenshot->GetPixelSize(),
                         0,
                         vtkKWIcon::ImageOptionFlipVertical);
    modified++;
    }

  vtkKWIcon *thumbnail = this->GetPresetThumbnail(id);
  if (thumbnail)
    {
    thumbnail->SetImage(thumbnail->GetData(),
                        thumbnail->GetWidth(),
                        thumbnail->GetHeight(),
                        thumbnail->GetPixelSize(),
                        0,
                        vtkKWIcon::ImageOptionFlipVertical);
    modified++;
    }

  // Update the icon cell

  if (modified)
    {
    this->ScheduleUpdatePresetRow(id);
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::HasPresetUserSlot(int id, const char *slot_name)
{
  if (this->Internals)
    {
    vtkKWPresetSelectorInternals::PresetNode *node =
      this->Internals->GetPresetNode(id);
    if (node)
      {
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        node->UserSlotPool.find(slot_name);
      if (s_it != node->UserSlotPool.end())
        {
        return 1;
        }
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::GetPresetUserSlotType(int id, const char *slot_name)
{
  if (this->Internals)
    {
    vtkKWPresetSelectorInternals::PresetNode *node =
      this->Internals->GetPresetNode(id);
    if (node)
      {
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        node->UserSlotPool.find(slot_name);
      if (s_it != node->UserSlotPool.end())
        {
        return s_it->second.Type;
        }
      }
    }

  return vtkKWPresetSelector::UserSlotUnknownType;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::DeletePresetUserSlot(int id, const char *slot_name)
{
  if (this->Internals)
    {
    vtkKWPresetSelectorInternals::PresetNode *node =
      this->Internals->GetPresetNode(id);
    if (node)
      {
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        node->UserSlotPool.find(slot_name);
      if (s_it != node->UserSlotPool.end())
        {
        s_it->second.DeAllocate();
        node->UserSlotPool.erase(s_it);
        this->ScheduleUpdatePresetRow(id);
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
    vtkKWPresetSelectorInternals::PresetNode *node =
      this->Internals->GetPresetNode(id);
    if (node)
      {
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        node->UserSlotPool.find(slot_name);
      if (s_it != node->UserSlotPool.end())
        {
        if (s_it->second.Type == vtkKWPresetSelector::UserSlotDoubleType &&
            s_it->second.DoubleValue == value)
          {
          return 1;
          }
        s_it->second.DeAllocate();
        }
      vtkKWPresetSelectorInternals::UserSlotValueType &slot = 
        node->UserSlotPool[slot_name];
      slot.DoubleValue = value;
      slot.Type = vtkKWPresetSelector::UserSlotDoubleType;
      if (this->GetPresetFilterUserSlotConstraint(slot_name))
        {
        this->PresetFilteringMayHaveChanged();
        this->UpdatePresetRow(id);
        }
      else
        {
        this->ScheduleUpdatePresetRow(id);
        }
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
    vtkKWPresetSelectorInternals::PresetNode *node =
      this->Internals->GetPresetNode(id);
    if (node)
      {
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        node->UserSlotPool.find(slot_name);
      if (s_it != node->UserSlotPool.end())
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
    vtkKWPresetSelectorInternals::PresetNode *node =
      this->Internals->GetPresetNode(id);
    if (node)
      {
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        node->UserSlotPool.find(slot_name);
      if (s_it != node->UserSlotPool.end())
        {
        if (s_it->second.Type == vtkKWPresetSelector::UserSlotIntType &&
            s_it->second.IntValue == value)
          {
          return 1;
          }
        s_it->second.DeAllocate();
        }
      vtkKWPresetSelectorInternals::UserSlotValueType &slot = 
        node->UserSlotPool[slot_name];
      slot.IntValue = value;
      slot.Type = vtkKWPresetSelector::UserSlotIntType;
      if (this->GetPresetFilterUserSlotConstraint(slot_name))
        {
        this->PresetFilteringMayHaveChanged();
        this->UpdatePresetRow(id);
        }
      else
        {
        this->ScheduleUpdatePresetRow(id);
        }
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
    vtkKWPresetSelectorInternals::PresetNode *node =
      this->Internals->GetPresetNode(id);
    if (node)
      {
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        node->UserSlotPool.find(slot_name);
      if (s_it != node->UserSlotPool.end())
        {
        return s_it->second.IntValue;
        }
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::SetPresetUserSlotAsUnsignedLong(
  int id, const char *slot_name, unsigned long value)
{
  if (this->Internals)
    {
    vtkKWPresetSelectorInternals::PresetNode *node =
      this->Internals->GetPresetNode(id);
    if (node)
      {
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        node->UserSlotPool.find(slot_name);
      if (s_it != node->UserSlotPool.end())
        {
        if (s_it->second.Type == vtkKWPresetSelector::UserSlotUnsignedLongType 
            && s_it->second.UnsignedLongValue == value)
          {
          return 1;
          }
        s_it->second.DeAllocate();
        }
      vtkKWPresetSelectorInternals::UserSlotValueType &slot = 
        node->UserSlotPool[slot_name];
      slot.UnsignedLongValue = value;
      slot.Type = vtkKWPresetSelector::UserSlotUnsignedLongType;
      if (this->GetPresetFilterUserSlotConstraint(slot_name))
        {
        this->PresetFilteringMayHaveChanged();
        this->UpdatePresetRow(id);
        }
      else
        {
        this->ScheduleUpdatePresetRow(id);
        }
      return 1;
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
unsigned long vtkKWPresetSelector::GetPresetUserSlotAsUnsignedLong(
  int id, const char *slot_name)
{
  if (this->Internals)
    {
    vtkKWPresetSelectorInternals::PresetNode *node =
      this->Internals->GetPresetNode(id);
    if (node)
      {
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        node->UserSlotPool.find(slot_name);
      if (s_it != node->UserSlotPool.end())
        {
        return s_it->second.UnsignedLongValue;
        }
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::SetPresetUserSlotAsInt64(
  int id, const char *slot_name, vtkTypeInt64 value)
{
  if (this->Internals)
    {
    vtkKWPresetSelectorInternals::PresetNode *node =
      this->Internals->GetPresetNode(id);
    if (node)
      {
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        node->UserSlotPool.find(slot_name);
      if (s_it != node->UserSlotPool.end())
        {
        if (s_it->second.Type == vtkKWPresetSelector::UserSlotInt64Type 
            && s_it->second.Int64Value == value)
          {
          return 1;
          }
        s_it->second.DeAllocate();
        }
      vtkKWPresetSelectorInternals::UserSlotValueType &slot = 
        node->UserSlotPool[slot_name];
      slot.Int64Value = value;
      slot.Type = vtkKWPresetSelector::UserSlotInt64Type;
      if (this->GetPresetFilterUserSlotConstraint(slot_name))
        {
        this->PresetFilteringMayHaveChanged();
        this->UpdatePresetRow(id);
        }
      else
        {
        this->ScheduleUpdatePresetRow(id);
        }
      return 1;
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
vtkTypeInt64 vtkKWPresetSelector::GetPresetUserSlotAsInt64(
  int id, const char *slot_name)
{
  if (this->Internals)
    {
    vtkKWPresetSelectorInternals::PresetNode *node =
      this->Internals->GetPresetNode(id);
    if (node)
      {
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        node->UserSlotPool.find(slot_name);
      if (s_it != node->UserSlotPool.end())
        {
        return s_it->second.Int64Value;
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
    vtkKWPresetSelectorInternals::PresetNode *node =
      this->Internals->GetPresetNode(id);
    if (node)
      {
      const char *fixed_value = value ? value : "";
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        node->UserSlotPool.find(slot_name);
      if (s_it != node->UserSlotPool.end())
        {
        if (s_it->second.Type == vtkKWPresetSelector::UserSlotStringType &&
            s_it->second.StringValue == fixed_value)
          {
          return 1;
          }
        s_it->second.DeAllocate();
        }
      vtkKWPresetSelectorInternals::UserSlotValueType &slot = 
        node->UserSlotPool[slot_name];
      slot.StringValue = fixed_value;
      slot.Type = vtkKWPresetSelector::UserSlotStringType;
      if (this->GetPresetFilterUserSlotConstraint(slot_name))
        {
        this->PresetFilteringMayHaveChanged();
        this->UpdatePresetRow(id);
        }
      else
        {
        this->ScheduleUpdatePresetRow(id);
        }
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
    vtkKWPresetSelectorInternals::PresetNode *node =
      this->Internals->GetPresetNode(id);
    if (node)
      {
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        node->UserSlotPool.find(slot_name);
      if (s_it != node->UserSlotPool.end())
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
    vtkKWPresetSelectorInternals::PresetNode *node =
      this->Internals->GetPresetNode(id);
    if (node)
      {
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        node->UserSlotPool.find(slot_name);
      if (s_it != node->UserSlotPool.end())
        {
        if (s_it->second.Type == vtkKWPresetSelector::UserSlotPointerType &&
            s_it->second.PointerValue == value)
          {
          return 1;
          }
        s_it->second.DeAllocate();
        }
      vtkKWPresetSelectorInternals::UserSlotValueType &slot = 
        node->UserSlotPool[slot_name];
      slot.PointerValue = value;
      slot.Type = vtkKWPresetSelector::UserSlotPointerType;
      if (this->GetPresetFilterUserSlotConstraint(slot_name))
        {
        this->PresetFilteringMayHaveChanged();
        this->UpdatePresetRow(id);
        }
      else
        {
        this->ScheduleUpdatePresetRow(id);
        }
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
    vtkKWPresetSelectorInternals::PresetNode *node =
      this->Internals->GetPresetNode(id);
    if (node)
      {
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        node->UserSlotPool.find(slot_name);
      if (s_it != node->UserSlotPool.end())
        {
        return s_it->second.PointerValue;
        }
      }
    }

  return NULL;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::SetPresetUserSlotAsObject(
  int id, const char *slot_name, vtkObject *value)
{
  if (this->Internals)
    {
    vtkKWPresetSelectorInternals::PresetNode *node =
      this->Internals->GetPresetNode(id);
    if (node)
      {
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        node->UserSlotPool.find(slot_name);
      if (s_it != node->UserSlotPool.end())
        {
        if (s_it->second.Type == vtkKWPresetSelector::UserSlotObjectType &&
            s_it->second.ObjectValue == value)
          {
          return 1;
          }
        s_it->second.DeAllocate();
        }
      vtkKWPresetSelectorInternals::UserSlotValueType &slot = 
        node->UserSlotPool[slot_name];
      slot.ObjectValue = value;
      slot.Type = vtkKWPresetSelector::UserSlotObjectType;
      if (value)
        {
        value->Register(this);
        }
      if (this->GetPresetFilterUserSlotConstraint(slot_name))
        {
        this->PresetFilteringMayHaveChanged();
        this->UpdatePresetRow(id);
        }
      else
        {
        this->ScheduleUpdatePresetRow(id);
        }
      return 1;
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
vtkObject* vtkKWPresetSelector::GetPresetUserSlotAsObject(
  int id, const char *slot_name)
{
  if (this->Internals)
    {
    vtkKWPresetSelectorInternals::PresetNode *node =
      this->Internals->GetPresetNode(id);
    if (node)
      {
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        node->UserSlotPool.find(slot_name);
      if (s_it != node->UserSlotPool.end())
        {
        return s_it->second.ObjectValue;
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
    return (int)this->Internals->PresetPool.size();
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
      vtkKWPresetSelectorInternals::PresetNode *preset = *it; // it->second;
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        preset->UserSlotPool.find(this->GetPresetGroupSlotName());
      if (s_it != preset->UserSlotPool.end() &&
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
  if (this->PresetList)
    {
    return this->PresetList->GetWidget()->GetNumberOfRows();
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetMaximumNumberOfPresets(int _arg)
{
  if (this->MaximumNumberOfPresets == _arg || _arg < 0)
    {
    return;
    }

  this->MaximumNumberOfPresets = _arg;
  this->Modified();
  
  this->ConstrainNumberOfPresets();
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::ConstrainNumberOfPresets()
{
  if (this->MaximumNumberOfPresets <= 0)
    {
    return;
    }

  while (this->GetNumberOfPresets() > this->MaximumNumberOfPresets)
    {
    
    vtkKWPresetSelectorInternals::PresetPoolIterator it = 
      this->Internals->PresetPool.begin();
    vtkKWPresetSelectorInternals::PresetPoolIterator end = 
      this->Internals->PresetPool.end();

    vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it;
    const char *c_time_slot_name = this->GetPresetCreationTimeSlotName(); 

    vtkTypeInt64 min_c_time = this->GetPresetCreationTime((*it)->Id);
    int min_id = -1;

    for (; it != end; ++it)
      {
#if 0
      vtkTypeInt64 c_time = this->GetPresetCreationTime((*it)->Id);
#else
      s_it =(*it)->UserSlotPool.find(c_time_slot_name);
      if (s_it != (*it)->UserSlotPool.end())
        {
        vtkTypeInt64 c_time = s_it->second.Int64Value;
        if (c_time < min_c_time)
          {
          min_c_time = c_time;
          min_id = (*it)->Id;
          }
        }
#endif
      }

    if (min_id != -1)
      {
      this->RemovePreset(min_id);
      }
    }
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::GetPresetVisibility(int id)
{
  return this->GetPresetRow(id) >= 0 ? 1 : 0;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::GetIdOfNthPreset(int index)
{
  if (this->Internals && index >= 0 && index < this->GetNumberOfPresets())
    {
    vtkKWPresetSelectorInternals::PresetPoolIterator it = 
      this->Internals->PresetPool.begin();
    while (index > 0)
      {
      ++it;
      --index;
      }
    vtkKWPresetSelectorInternals::PresetNode *preset = *it; // it->second;
    return preset->Id;
    }

  return -1;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::GetIdOfNthPresetWithGroup(int index, const char *group)
{
  int rank = this->GetRankOfNthPresetWithGroup(index, group);
  if (rank >= 0)
    {
    return this->GetIdOfNthPreset(rank);
    }
  return -1;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::GetIdOfPresetAtRow(int row_index)
{
  if (!this->PresetList)
    {
    return -1;
    }

  int id = this->GetRowIndexToPresetIdCacheEntry(row_index);
  if (id < 0)
    {
    vtkKWMultiColumnList *list = this->PresetList->GetWidget();
    if (row_index >= 0 && row_index < list->GetNumberOfRows())
      {
      id = list->GetCellTextAsInt(row_index, this->GetIdColumnIndex());
      this->SetRowIndexToPresetIdCacheEntry(row_index, id);
      }
    }
  return id;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::GetPresetRow(int id)
{
  if (!this->PresetList)
    {
    return -1;
    }

  int row_index = this->GetPresetIdToRowIndexCacheEntry(id);
  if (row_index < 0)
    {
    row_index = this->PresetList->GetWidget()->FindCellTextAsIntInColumn(
      this->GetIdColumnIndex(), id);
    this->SetPresetIdToRowIndexCacheEntry(id, row_index);
    }
  return row_index;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::GetRankOfNthPresetWithGroup(
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
      vtkKWPresetSelectorInternals::PresetNode *preset = *it; // it->second;
      vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
        preset->UserSlotPool.find(this->GetPresetGroupSlotName());
      if (s_it != preset->UserSlotPool.end() &&
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
void vtkKWPresetSelector::SelectPreset(int id)
{
  int row = this->GetPresetRow(id);
  if (row >= 0 && this->PresetList)
    {
    this->PresetList->GetWidget()->SelectSingleRow(row);
    this->PresetList->GetWidget()->SeeRow(row);
    this->UpdateToolbarPresetButtons(this->PresetButtons); 
    this->UpdateToolbarPresetButtons(this->Toolbar);
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
      this->UpdateToolbarPresetButtons(this->PresetButtons); 
      this->UpdateToolbarPresetButtons(this->Toolbar);
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
      this->UpdateToolbarPresetButtons(this->PresetButtons); 
      this->UpdateToolbarPresetButtons(this->Toolbar);
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::ClearSelection()
{
  if (this->PresetList)
    {
    this->PresetList->GetWidget()->ClearSelection();
    this->UpdateToolbarPresetButtons(this->PresetButtons); 
    this->UpdateToolbarPresetButtons(this->Toolbar);
    }
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::GetIdOfSelectedPreset()
{
  if (this->PresetList)
    {
    return this->GetIdOfPresetAtRow(
      this->PresetList->GetWidget()->GetIndexOfFirstSelectedRow());
    }
  return -1;
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::NumberOfPresetsHasChanged()
{
  this->Update(); // enable/disable some buttons valid only if we have presets
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::PresetFilteringMayHaveChanged()
{
  // Since filtering may have changed, some presets may show up or be
  // hidden, hence their location will change

  this->InvalidatePresetIdToRowIndexCache();
  this->InvalidateRowIndexToPresetIdCache();
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::PresetFilteringHasChanged()
{
  this->PresetFilteringMayHaveChanged();
  this->InvokePresetFilteringHasChangedCommand();
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::ClearPresetFilter()
{
  if (this->Internals && this->Internals->PresetFilter.size())
    {
    this->Internals->PresetFilter.clear();
    this->PresetFilteringHasChanged();
    this->ScheduleUpdatePresetRows();
    }
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetPresetFilterUserSlotConstraint(
  const char *slot_name, const char *value)
{
  int update = 1;
  vtkKWPresetSelectorInternals::PresetFilterIterator it =
    this->Internals->PresetFilter.find(slot_name);
  if (it != this->Internals->PresetFilter.end())
    {
    if (value)
      {
      if ((*it).second.StringValue.compare(value))
        {
        (*it).second.StringValue = value;
        }
      else
        {
        update = 0;
        }
      }
    else
      {
      this->Internals->PresetFilter.erase(it);
      }
    }
  else
    {
    if (value)
      {
      this->Internals->PresetFilter[slot_name].StringValue = value;
      this->Internals->PresetFilter[slot_name].IsRegularExpression = 0;
      }
    else
      {
      update = 0;
      }
    }
  if (update)
    {
    this->PresetFilteringHasChanged();
    this->ScheduleUpdatePresetRows();
    }
}

//----------------------------------------------------------------------------
const char* vtkKWPresetSelector::GetPresetFilterUserSlotConstraint(
  const char *slot_name)
{
  vtkKWPresetSelectorInternals::PresetFilterIterator it =
    this->Internals->PresetFilter.find(slot_name);
  if (it != this->Internals->PresetFilter.end())
    {
    return (*it).second.StringValue.c_str();
    }
  return NULL;
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::DeletePresetFilterUserSlotConstraint(
  const char *slot_name)
{
  vtkKWPresetSelectorInternals::PresetFilterIterator it =
    this->Internals->PresetFilter.find(slot_name);
  if (it != this->Internals->PresetFilter.end())
    {
    this->Internals->PresetFilter.erase(it);
    this->PresetFilteringHasChanged();
    this->ScheduleUpdatePresetRows();
    }
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetPresetFilterUserSlotConstraintToRegularExpression(
  const char *slot_name)
{
  vtkKWPresetSelectorInternals::PresetFilterIterator it =
    this->Internals->PresetFilter.find(slot_name);
  if (it != this->Internals->PresetFilter.end() && 
      !(*it).second.IsRegularExpression)
    {
    (*it).second.IsRegularExpression = 1;
    this->PresetFilteringHasChanged();
    this->ScheduleUpdatePresetRows();
    }
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetPresetFilterUserSlotConstraintToString(
  const char *slot_name)
{
  vtkKWPresetSelectorInternals::PresetFilterIterator it =
    this->Internals->PresetFilter.find(slot_name);
  if (it != this->Internals->PresetFilter.end() && 
      (*it).second.IsRegularExpression)
    {
    (*it).second.IsRegularExpression = 0;
    this->PresetFilteringHasChanged();
    this->ScheduleUpdatePresetRows();
    }
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetPresetFilterGroupConstraint(const char *value)
{
  this->SetPresetFilterUserSlotConstraint(
    this->GetPresetGroupSlotName(), value);
}

//----------------------------------------------------------------------------
const char* vtkKWPresetSelector::GetPresetFilterGroupConstraint()
{
  return this->GetPresetFilterUserSlotConstraint(
    this->GetPresetGroupSlotName());
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::IsPresetFiltered(int id)
{
  if (this->Internals->PresetFilter.empty())
    {
    return 1;
    }

  char buffer[100];

  vtkKWPresetSelectorInternals::PresetFilterIterator it = 
    this->Internals->PresetFilter.begin();
  vtkKWPresetSelectorInternals::PresetFilterIterator end = 
    this->Internals->PresetFilter.end();
  for (; it != end; ++it)
    {
    const char *slot_name = (*it).first.c_str();
    int slot_type = this->GetPresetUserSlotType(id, slot_name);
    const char *slot_value = NULL;
    if (slot_type == vtkKWPresetSelector::UserSlotStringType)
      {
      slot_value = this->GetPresetUserSlotAsString(id, slot_name);
      }
    else if (slot_type == vtkKWPresetSelector::UserSlotIntType)
      {
      sprintf(buffer, "%d", this->GetPresetUserSlotAsInt(id, slot_name));
      slot_value = buffer;
      }
    const char *filter_value = (*it).second.StringValue.c_str();
    if ((*it).second.IsRegularExpression)
      {
      vtksys::RegularExpression re(filter_value);
      if (!re.find((slot_value ? slot_value : "")))
        {
        return 0;
        }
      }
    else
      {
      if (strcmp((slot_value ? slot_value : ""), filter_value))
        {
        return 0;
        }
      }
    }
  return 1;
} 

//----------------------------------------------------------------------------
void vtkKWPresetSelector::UpdatePresetRows()
{
  int nb_visible_presets = this->GetNumberOfVisiblePresets();

  vtkKWPresetSelectorInternals::PresetPoolIterator it = 
    this->Internals->PresetPool.begin();
  vtkKWPresetSelectorInternals::PresetPoolIterator end = 
    this->Internals->PresetPool.end();
  for (; it != end; ++it)
    {
    vtkKWPresetSelectorInternals::PresetNode *preset = *it; // it->second;
    this->UpdatePresetRow(preset->Id);
    }

  // If the number of visible presets changed, this can enable/disable
  // some buttons

  if (nb_visible_presets != this->GetNumberOfVisiblePresets())
    {
    this->Update();
    }
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::ScheduleUpdatePresetRows()
{
  // Already scheduled

  if (this->Internals->ScheduleUpdatePresetRowsTimerId.size())
    {
    return;
    }

  this->Internals->ScheduleUpdatePresetRowsTimerId =
    this->Script(
      "after idle {catch {%s UpdatePresetRowsCallback}}", this->GetTclName());
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::CancelScheduleUpdatePresetRows()
{
  // Not scheduled

  if (!this->Internals->ScheduleUpdatePresetRowsTimerId.size())
    {
    return;
    }

  this->Script("after cancel %s", 
               this->Internals->ScheduleUpdatePresetRowsTimerId.c_str());

  this->UpdatePresetRowsCallback();
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::UpdatePresetRowsCallback()
{
  if (!this->GetApplication() || this->GetApplication()->GetInExit() ||
      !this->IsAlive())
    {
    return;
    }

  this->UpdatePresetRows();
  this->Internals->ScheduleUpdatePresetRowsTimerId = "";
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::UpdatePresetRowInMultiColumnList(int id, int is_new)
{
  const int err_code = -1;

  if (!this->HasPreset(id))
    {
    return err_code;
    }

  vtkKWMultiColumnList *list = this->PresetList->GetWidget();

  // Look for this row in the list

  int row = is_new ? -1 : this->GetPresetRow(id);

  int is_preset_filtered = this->IsPresetFiltered(id);

  // Not found ? Insert it, or ignore it if the group filter does not match

  if (row < 0)
    {
    if (!is_preset_filtered)
      {
      return err_code;
      }
    
    // Let's find where to insert it, we need to try to keep the order
    // of the internal list as consistent as possible

    vtkKWPresetSelectorInternals::PresetPoolIterator begin = 
      this->Internals->PresetPool.begin();
    vtkKWPresetSelectorInternals::PresetPoolIterator end = 
      this->Internals->PresetPool.end();

    // Fast case, we just added this one at the end.

    vtkKWPresetSelectorInternals::PresetPoolIterator back = end;
    --back;
    if ((*back)->Id == id)
      {
      row = list->GetNumberOfRows();
      }

    // Slow case, we need to find the row of the first visible preset
    // following the one we are trying to insert
    
    else
      {
      vtkKWPresetSelectorInternals::PresetPoolIterator it = 
        this->Internals->GetPresetNodeIterator(id);
      for (++it; it != end; ++it)
        {
        if (this->IsPresetFiltered((*it)->Id))
          {
          row = this->GetPresetRow((*it)->Id);
          if (row >= 0)
            {
            break;
            }
          }
        }
      if (it == end) // we didn't find any that was visible :(
        {
        row = list->GetNumberOfRows();
        }
      }

    list->InsertRow(row);

    // Id (not shown, but useful to retrieve the id of a preset from
    // a cell position
    
    list->SetCellTextAsInt(row, this->GetIdColumnIndex(), id);
    }

  // Found ? Remove it if the group filter does not match

  else
    {
    if (!is_preset_filtered)
      {
      list->DeleteRow(row);
      return err_code;
      }
    }

  return row;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::UpdatePresetRow(int id)
{
  int row = this->UpdatePresetRowInMultiColumnList(id);
  if (row < 0)
    {
    return 0;
    }

  vtkKWMultiColumnList *list = this->PresetList->GetWidget();

  if (this->GetThumbnailColumnVisibility())
    {
    int image_col_index = this->GetThumbnailColumnIndex();
  
    vtkKWIcon *thumbnail = this->GetPresetThumbnail(id);
    vtkKWIcon *screenshot = this->GetPresetScreenshot(id);
  
    list->SetCellWindowCommand(
      row, image_col_index, this, "PresetCellThumbnailCallback");
    list->SetCellWindowDestroyCommandToRemoveChild(row, image_col_index);
    if (this->GetThumbnailColumnVisibility())
      {
      list->RefreshCellWithWindowCommand(row, image_col_index);
      }

    list->SetCellTextAsDouble(
      row, image_col_index, (double)this->GetPresetCreationTime(id));
    }

  if (this->GetGroupColumnVisibility())
    {
    list->SetCellText(
      row, this->GetGroupColumnIndex(), this->GetPresetGroup(id));
    }

  list->SetCellText(
    row, this->GetCommentColumnIndex(), this->GetPresetComment(id));

  return 1;
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::ScheduleUpdatePresetRow(int id)
{
  // Already scheduled

  vtkKWPresetSelectorInternals::ScheduleUpdatePresetRowTimerPoolIterator it = 
    this->Internals->ScheduleUpdatePresetRowTimerPool.find(id);
  if (it != this->Internals->ScheduleUpdatePresetRowTimerPool.end())
    {
    return;
    }

  this->Internals->ScheduleUpdatePresetRowTimerPool[id] =
    this->Script("after idle {catch {%s UpdatePresetRowCallback %d}}", 
                 this->GetTclName(), id);
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::UpdatePresetRowCallback(int id)
{
  if (!this->GetApplication() || this->GetApplication()->GetInExit() ||
      !this->IsAlive())
    {
    return;
    }

  this->UpdatePresetRow(id);
  this->Internals->ScheduleUpdatePresetRowTimerPool.erase(
    this->Internals->ScheduleUpdatePresetRowTimerPool.find(id));
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

  int id = this->GetIdOfPresetAtRow(row);
  if (id >= 0)
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
      mgr->SetIgnoreIfNotEnabled(1);
      child->SetBalloonHelpManager(mgr);
      mgr->SetDelay(10);
      mgr->Delete();
      child->SetBalloonHelpIcon(screenshot);
      }
    else
      {
      child->SetBalloonHelpIcon(NULL);
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
  int id = this->GetIdOfPresetAtRow(row);
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
int vtkKWPresetSelector::PresetAddCallback()
{
  return this->InvokePresetAddCommand();
}

//---------------------------------------------------------------------------
void vtkKWPresetSelector::PresetApplyCallback(int id)
{
  if (this->HasPreset(id))
    {
    this->InvokePresetApplyCommand(id);
    }
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
      this->PresetApplyCallback(this->GetIdOfPresetAtRow(indices[i]));
      }
    delete [] indices;
    }
}

//---------------------------------------------------------------------------
void vtkKWPresetSelector::PresetUpdateCallback(int id)
{
  if (this->HasPreset(id))
    {
    this->InvokePresetUpdateCommand(id);
    }
}

//---------------------------------------------------------------------------
void vtkKWPresetSelector::PresetUpdateCallback()
{
  if (this->PresetList)
    {
    // First collect the indices of the presets to update
    // Then update them

    vtkKWMultiColumnList *list = this->PresetList->GetWidget();
    int *indices = new int [list->GetNumberOfRows()];
    int *ids = new int [list->GetNumberOfRows()];
    int i, nb_selected_rows = list->GetSelectedRows(indices);
    for (i = 0; i < nb_selected_rows; i++)
      {
      ids[i] = this->GetIdOfPresetAtRow(indices[i]);
      }
    for (i = 0; i < nb_selected_rows; i++)
      {
      this->PresetUpdateCallback(ids[i]);
      }
    delete [] indices;
    delete [] ids;
    }
}

//---------------------------------------------------------------------------
void vtkKWPresetSelector::PresetRemoveCallback(int id)
{
  if (!this->PromptBeforeRemovePreset ||
      vtkKWMessageDialog::PopupYesNo( 
        this->GetApplication(), 
        this->GetParentTopLevel(), 
        ks_("Preset Selector|Delete Preset Dialog|Title|Delete Preset?"),
        k_("Are you sure you want to delete the selected item?"), 
        vtkKWMessageDialog::WarningIcon | 
        vtkKWMessageDialog::InvokeAtPointer))
    {
    if (this->InvokePresetRemoveCommand(id) && this->RemovePreset(id))
      {
      this->InvokePresetRemovedCommand();
      }
    }
}

//---------------------------------------------------------------------------
void vtkKWPresetSelector::PresetRemoveAllCallback()
{
  if (!this->PromptBeforeRemovePreset ||
      vtkKWMessageDialog::PopupYesNo( 
        this->GetApplication(), 
        this->GetParentTopLevel(), 
        ks_("Preset Selector|Delete Preset Dialog|Title|Delete All Presets?"),
        k_("Are you sure you want to delete all items?"), 
        vtkKWMessageDialog::WarningIcon | 
        vtkKWMessageDialog::InvokeAtPointer))
    {
    vtkKWPresetSelectorInternals::PresetPoolIterator end = 
      this->Internals->PresetPool.end();
    int has_removed;
    do
      {
      has_removed = 0;
      vtkKWPresetSelectorInternals::PresetPoolIterator it = 
        this->Internals->PresetPool.begin();
      for (; it != end; ++it)
        {
        vtkKWPresetSelectorInternals::PresetNode *preset = *it; // it->second;
        int id = preset->Id;
        if (this->GetPresetVisibility(id) &&
            this->InvokePresetRemoveCommand(id) &&
            this->RemovePreset(id))
          {
          this->InvokePresetRemovedCommand();
          has_removed = 1;
          break;
          }
        }
      } while (has_removed);
    }
}

//---------------------------------------------------------------------------
void vtkKWPresetSelector::PresetRemoveCallback()
{
  if (this->PresetList)
    {
    // First collect the indices of the presets to remove
    // Then remove them

    vtkKWMultiColumnList *list = this->PresetList->GetWidget();
    int *indices = new int [list->GetNumberOfRows()];
    int *ids = new int [list->GetNumberOfRows()];
    int i, nb_selected_rows = list->GetSelectedRows(indices);
    for (i = 0; i < nb_selected_rows; i++)
      {
      ids[i] = this->GetIdOfPresetAtRow(indices[i]);
      }
    for (i = 0; i < nb_selected_rows; i++)
      {
      this->PresetRemoveCallback(ids[i]);
      }
    delete [] indices;
    delete [] ids;
    }
}

//---------------------------------------------------------------------------
void vtkKWPresetSelector::PresetFilterCallback()
{
  int x, y;

  vtkKWTkUtilities::GetMousePointerCoordinates(
    this->GetApplication()->GetMainInterp(), ".", &x, &y);

  // Create the context menu if needed

  if (!this->ContextMenu)
    {
    this->ContextMenu = vtkKWMenu::New();
    }
  if (!this->ContextMenu->IsCreated())
    {
    this->ContextMenu->SetParent(this);
    this->ContextMenu->Create();
    }
  this->ContextMenu->DeleteAllItems();

  // Parse the current values

  // It is expected to be in the ^(value1|value2|)$ form, where the last value
  // may be empty to match presets with no value for this specific slot.

  vtksys_stl::map<vtksys_stl::string, int> current_values;

  const char *res = this->GetPresetFilterUserSlotConstraint(
    this->GetFilterButtonSlotName());
  if (res)
    {
    // Remove ^( and )$
    const char *res_end = res + strlen(res);
    const char *open_regexp = "^(";
    if (vtksys::SystemTools::StringStartsWith(res, open_regexp))
      {
      res += strlen(open_regexp);
      }
    const char *close_regexp = ")$";
    if (vtksys::SystemTools::StringEndsWith(res, close_regexp))
      {
      res_end -= strlen(close_regexp);
      }
    vtksys_stl::string res_safe(res, res_end - res);

    // Split at | and build a map of unique current values
    // We actually *do* need the empty value to be registered as
    // a valid filtering value.
    vtksys_stl::vector<vtksys_stl::string> split_elems;
    vtksys::SystemTools::Split(res_safe.c_str(), split_elems, '|');
    if (!split_elems.size())
      {
      split_elems.push_back("");
      }
    vtksys_stl::vector<vtksys_stl::string>::iterator it = split_elems.begin();
    vtksys_stl::vector<vtksys_stl::string>::iterator end = split_elems.end();
    for (; it != end; it++)
      {
      current_values[*it] = 1;
      }
    }
  
  // Collect all the unique possible values for that slot

  vtksys_stl::map<vtksys_stl::string, int> possible_values;
  
  vtksys_stl::string empty_value;

  vtkKWPresetSelectorInternals::PresetPoolIterator it = 
    this->Internals->PresetPool.begin();
  vtkKWPresetSelectorInternals::PresetPoolIterator end = 
    this->Internals->PresetPool.end();
  for (; it != end; ++it)
    {
    vtkKWPresetSelectorInternals::PresetNode *preset = *it; // it->second;
    vtkKWPresetSelectorInternals::UserSlotPoolIterator s_it =
      preset->UserSlotPool.find(this->GetFilterButtonSlotName());
    vtksys_stl::string *possible_value = NULL;
    // If the slot was not found, accept it as an empty value...
    if (s_it != preset->UserSlotPool.end())
      {
      possible_value = &s_it->second.StringValue;
      }
    else
      {
      possible_value = &empty_value;
      }
    // Register the unique empty value (and count the occurences)
    vtksys_stl::map<vtksys_stl::string, int>::iterator p_it_found = 
      possible_values.find(*possible_value);
    if (p_it_found == possible_values.end())
      {
      possible_values[*possible_value] = 1;
      }
    else
      {
      ++possible_values[*possible_value];
      }
    }

  // Also add the current values, even if they were not found, they need
  // to be displayed (execept the empty one)

  vtksys_stl::map<vtksys_stl::string, int>::iterator c_it = 
    current_values.begin();
  vtksys_stl::map<vtksys_stl::string, int>::iterator c_end = 
    current_values.end();
  for (; c_it != c_end; ++c_it)
    {
    if (c_it->first.size())
      {
      possible_values[c_it->first] = 0;
      }
    }

  // Build the menu, allowing the new possible values

  vtksys_stl::map<vtksys_stl::string, int>::iterator p_it = 
    possible_values.begin();
  vtksys_stl::map<vtksys_stl::string, int>::iterator p_end = 
    possible_values.end();
  for (; p_it != p_end; ++p_it)
    {
    vtksys_ios::ostringstream label;
    const char *possible_value = p_it->first.c_str();
    label << (*possible_value ? possible_value : k_("Unknown")); 
    // << " (" << p_it->second << ")";
    int id = this->ContextMenu->AddCheckButton(label.str().c_str());

    // Build the filter for this specific button (i.e. the list of 
    // current values minus or plus this specific new possible value)

    vtksys_stl::map<vtksys_stl::string, int> new_values(current_values);
    vtksys_stl::map<vtksys_stl::string, int>::iterator n_it = 
      new_values.find(p_it->first);
    if (n_it == new_values.end())
      {
      new_values[p_it->first] = 1;
      }
    else
      {
      this->ContextMenu->SetItemSelectedState(id, 1);
      new_values.erase(n_it);
      }

    vtksys_stl::string cmd("PresetFilterApplyCallback");
    if (new_values.size())
      {
      vtksys_stl::map<vtksys_stl::string, int>::iterator n_begin = 
        new_values.begin();
      vtksys_stl::map<vtksys_stl::string, int>::iterator n_end = 
        new_values.end();
      vtksys_stl::string new_values_collated;
      for (n_it = n_begin; n_it != n_end; ++n_it)
        {
        if (n_it != n_begin)
          {
          new_values_collated += '|';
          }
        new_values_collated += n_it->first;
        }
      cmd = cmd + " \"^(" + new_values_collated + ")$\"";
      }
    else
      {
      cmd = cmd + " \"\"";
      }
    this->ContextMenu->SetItemCommand(id, this, cmd.c_str());
    }

  // Popup

  if (this->ContextMenu->GetNumberOfItems())
    {
    this->ContextMenu->PopUp(x, y);
    }
}

//---------------------------------------------------------------------------
void vtkKWPresetSelector::PresetFilterApplyCallback(const char *regexp)
{
  if (regexp && *regexp)
    {
    this->SetPresetFilterUserSlotConstraint(
      this->GetFilterButtonSlotName(), regexp);
    this->SetPresetFilterUserSlotConstraintToRegularExpression(
      this->GetFilterButtonSlotName());
    }
  else
    {
    this->DeletePresetFilterUserSlotConstraint(
      this->GetFilterButtonSlotName());
    }
}

//---------------------------------------------------------------------------
int vtkKWPresetSelector::GetNumberOfSelectedPresetsWithFileName()
{
  int nb = 0;
  if (this->PresetList)
    {
    vtkKWMultiColumnList *list = this->PresetList->GetWidget();
    int *indices = new int [list->GetNumberOfRows()];
    int i, nb_selected_rows = list->GetSelectedRows(indices);
    for (i = 0; i < nb_selected_rows; i++)
      {
      int id = this->GetIdOfPresetAtRow(indices[i]);
      const char *filename = this->GetPresetFileName(id);
      if (filename && *filename && vtksys::SystemTools::FileExists(filename))
        {
        nb++;
        }
      }
    delete [] indices;
    }
  return nb;
}

//---------------------------------------------------------------------------
void vtkKWPresetSelector::PresetLocateCallback(int id)
{
  if (this->HasPreset(id))
    {
    const char *filename = this->GetPresetFileName(id);
    if (filename && *filename && vtksys::SystemTools::FileExists(filename))
      {
      this->GetApplication()->ExploreLink(filename);
      }
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
      this->PresetLocateCallback(this->GetIdOfPresetAtRow(indices[i]));
      }
    delete [] indices;
    }
}

//---------------------------------------------------------------------------
void vtkKWPresetSelector::PresetEmailCallback(int id)
{
  if (!this->HasPreset(id))
    {
    return;
    }

  const char *filename = this->GetPresetFileName(id);
  if (!filename || !*filename ||!vtksys::SystemTools::FileExists(filename))
    {
    return;
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

  char buffer[500];
  sprintf(buffer,
          ks_("Preset Selector|Email Preset|This file was sent from %s"), 
          this->GetApplication()->GetPrettyName());

  message = buffer;
  message += "\n\n";

  message += ks_("Preset Selector|Email Preset|File:");
  message += " ";
  message += native_filename;
  message += "\n";

  if (comment && *comment)
    {
    message += ks_("Preset Selector|Email Preset|Comment:");
    message += " ";
    message += comment;
    message += "\n";
    }
        
  time_t t = (time_t)(this->GetPresetCreationTime(id) / 1000);
  const char *ctime_buffer = ctime(&t);
  if (ctime_buffer)
    {
    message += ks_("Preset Selector|Email Preset|Creation Time:");
    message += " ";
    message += ctime_buffer;
    }

  if (this->EmailBody)
    {
    message += "\n";
    message += this->EmailBody;
    }

  this->GetApplication()->SendEmail(
    NULL, subject.c_str(), message.c_str(), native_filename.c_str(), NULL);
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
      this->PresetEmailCallback(this->GetIdOfPresetAtRow(indices[i]));
      }
    delete [] indices;
    }
}

//---------------------------------------------------------------------------
int vtkKWPresetSelector::PresetLoadCallback()
{
  return this->InvokePresetLoadCommand();
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SchedulePresetSelectionCallback()
{
  // Already scheduled

  if (this->Internals->SchedulePresetSelectionCallbackTimerId.size())
    {
    return;
    }

  this->Internals->SchedulePresetSelectionCallbackTimerId =
    this->Script(
      "after idle {catch {%s PresetSelectionCallback}}", this->GetTclName());
}

//---------------------------------------------------------------------------
void vtkKWPresetSelector::PresetSelectionCallback()
{
  this->Internals->SchedulePresetSelectionCallbackTimerId = "";

  this->Update(); // this enable/disable the remove button if no selection

  if (this->ApplyPresetOnSelection)
    {
    this->PresetApplyCallback();
    }
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SchedulePresetSelectionChangedCallback()
{
  // Already scheduled

  if (this->Internals->SchedulePresetSelectionChangedCallbackTimerId.size())
    {
    return;
    }

  this->Internals->SchedulePresetSelectionChangedCallbackTimerId =
    this->Script(
      "after idle {catch {%s PresetSelectionChangedCallback}}", 
      this->GetTclName());
}

//---------------------------------------------------------------------------
void vtkKWPresetSelector::PresetSelectionChangedCallback()
{
  this->Internals->SchedulePresetSelectionChangedCallbackTimerId = "";
}

//---------------------------------------------------------------------------
void vtkKWPresetSelector::PresetRightClickCallback(
  int row, int vtkNotUsed(col), int x, int y)
{
  int id = this->GetIdOfPresetAtRow(row);
  if (!this->HasPreset(id))
    {
    return;
    }

  if (!this->ContextMenu)
    {
    this->ContextMenu = vtkKWMenu::New();
    }
  if (!this->ContextMenu->IsCreated())
    {
    this->ContextMenu->SetParent(this);
    this->ContextMenu->Create();
    }
  this->ContextMenu->DeleteAllItems();
  this->PopulatePresetContextMenu(this->ContextMenu, id);
  if (this->ContextMenu->GetNumberOfItems())
    {
    this->ContextMenu->PopUp(x, y);
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
int vtkKWPresetSelector::InvokePresetAddCommand()
{
  if (this->PresetAddCommand && *this->PresetAddCommand && this->IsCreated())
    {
    return atoi(vtkKWTkUtilities::EvaluateSimpleString(
                  this->GetApplication(), this->PresetAddCommand));
    }
  return -1;
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
void vtkKWPresetSelector::SetPresetRemovedCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->PresetRemovedCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::InvokePresetRemovedCommand()
{
  this->InvokeObjectMethodCommand(this->PresetRemovedCommand);
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

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetPresetLoadCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->PresetLoadCommand, object, method);
  this->Update(); // this show/hide the load button
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::InvokePresetLoadCommand()
{
  if (this->PresetLoadCommand && *this->PresetLoadCommand && this->IsCreated())
    {
    return atoi(vtkKWTkUtilities::EvaluateSimpleString(
                  this->GetApplication(), this->PresetLoadCommand));
    }
  return -1;
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetPresetFilteringHasChangedCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(
    &this->PresetFilteringHasChangedCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::InvokePresetFilteringHasChangedCommand()
{
  if (this->PresetFilteringHasChangedCommand && 
      *this->PresetFilteringHasChangedCommand && 
      this->IsCreated())
    {
    this->InvokeObjectMethodCommand(this->PresetFilteringHasChangedCommand);
    }
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetCreateUserPresetButtonsCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(
    &this->CreateUserPresetButtonsCommand, object, method);
  this->InvokeCreateUserPresetButtonsCommand(this->PresetButtons, 1);
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::InvokeCreateUserPresetButtonsCommand(
  vtkKWToolbar *toolbar, int use_separators)
{
  if (this->CreateUserPresetButtonsCommand && 
      *this->CreateUserPresetButtonsCommand && 
      this->IsCreated())
    {
    this->Script("%s %s %d", 
                 this->CreateUserPresetButtonsCommand, 
                 toolbar->GetTclName(), use_separators);
    }
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetUpdateUserPresetButtonsCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(
    &this->UpdateUserPresetButtonsCommand, object, method);
  this->InvokeUpdateUserPresetButtonsCommand(this->PresetButtons);
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::InvokeUpdateUserPresetButtonsCommand(
  vtkKWToolbar *toolbar)
{
  if (this->UpdateUserPresetButtonsCommand && 
      *this->UpdateUserPresetButtonsCommand && 
      this->IsCreated())
    {
    this->Script("%s %s", 
                 this->UpdateUserPresetButtonsCommand, 
                 toolbar->GetTclName());
    }
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetSetUserPresetButtonsIconsCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(
    &this->SetUserPresetButtonsIconsCommand, object, method);
  this->InvokeSetUserPresetButtonsIconsCommand(this->PresetButtons);
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::InvokeSetUserPresetButtonsIconsCommand(
  vtkKWToolbar *toolbar)
{
  if (this->SetUserPresetButtonsIconsCommand && 
      *this->SetUserPresetButtonsIconsCommand && 
      this->IsCreated())
    {
    this->Script("%s %s", 
                 this->SetUserPresetButtonsIconsCommand, 
                 toolbar->GetTclName());
    }
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetSetUserPresetButtonsHelpStringsCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(
    &this->SetUserPresetButtonsHelpStringsCommand, object, method);
  this->InvokeSetUserPresetButtonsHelpStringsCommand(this->PresetButtons);
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::InvokeSetUserPresetButtonsHelpStringsCommand(
  vtkKWToolbar *toolbar)
{
  if (this->SetUserPresetButtonsHelpStringsCommand && 
      *this->SetUserPresetButtonsHelpStringsCommand && 
      this->IsCreated())
    {
    this->Script("%s %s", 
                 this->SetUserPresetButtonsHelpStringsCommand, 
                 toolbar->GetTclName());
    }
}

//----------------------------------------------------------------------------
vtkKWToolbar* vtkKWPresetSelector::GetToolbar()
{
  if (!this->Toolbar)
    {
    this->Toolbar = vtkKWToolbar::New();
    }

  return this->Toolbar;
}

//---------------------------------------------------------------------------
const char* vtkKWPresetSelector::GetSelectPreviousButtonLabel()
{
  return this->Internals->SelectPreviousButtonLabel.c_str();
}

//---------------------------------------------------------------------------
const char* vtkKWPresetSelector::GetSelectNextButtonLabel()
{
  return this->Internals->SelectNextButtonLabel.c_str();
}

//---------------------------------------------------------------------------
const char* vtkKWPresetSelector::GetAddButtonLabel()
{
  return this->Internals->AddButtonLabel.c_str();
}

//---------------------------------------------------------------------------
const char* vtkKWPresetSelector::GetApplyButtonLabel()
{
  return this->Internals->ApplyButtonLabel.c_str();
}

//---------------------------------------------------------------------------
const char* vtkKWPresetSelector::GetUpdateButtonLabel()
{
  return this->Internals->UpdateButtonLabel.c_str();
}

//---------------------------------------------------------------------------
const char* vtkKWPresetSelector::GetRemoveButtonLabel()
{
  return this->Internals->RemoveButtonLabel.c_str();
}

//---------------------------------------------------------------------------
const char* vtkKWPresetSelector::GetRemoveAllButtonLabel()
{
  return this->Internals->RemoveAllButtonLabel.c_str();
}

//---------------------------------------------------------------------------
const char* vtkKWPresetSelector::GetLocateButtonLabel()
{
  return this->Internals->LocateButtonLabel.c_str();
}

//---------------------------------------------------------------------------
const char* vtkKWPresetSelector::GetEmailButtonLabel()
{
  return this->Internals->EmailButtonLabel.c_str();
}

//---------------------------------------------------------------------------
const char* vtkKWPresetSelector::GetLoadButtonLabel()
{
  return this->Internals->LoadButtonLabel.c_str();
}

//---------------------------------------------------------------------------
const char* vtkKWPresetSelector::GetFilterButtonLabel()
{
  return this->Internals->FilterButtonLabel.c_str();
}

//---------------------------------------------------------------------------
void vtkKWPresetSelector::Update()
{
  this->UpdateEnableState();

  this->UpdateToolbarPresetButtons(this->PresetButtons); 
  this->UpdateToolbarPresetButtons(this->Toolbar);
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

  if (this->HelpLabel)
    {
    this->HelpLabel->SetEnabled(this->GetEnabled());
    }

  this->PropagateEnableState(this->Toolbar);
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::ColumnSortedCallback()
{
  this->InvalidatePresetIdToRowIndexCache();
  this->InvalidateRowIndexToPresetIdCache();
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::RowMovedCallback()
{
  this->InvalidatePresetIdToRowIndexCache();
  this->InvalidateRowIndexToPresetIdCache();
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetPresetIdToRowIndexCacheEntry(
  int id, int row_index)
{
  this->Internals->PresetIdToRowIndexCache[id] = row_index;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::GetPresetIdToRowIndexCacheEntry(int id)
{
  vtkKWPresetSelectorInternals::PresetIdToRowIndexCacheTypeIterator it =
    this->Internals->PresetIdToRowIndexCache.find(id);
  if (it != this->Internals->PresetIdToRowIndexCache.end())
    {
    return it->second;
    }
  return -1;
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::InvalidatePresetIdToRowIndexCache()
{
  this->Internals->PresetIdToRowIndexCache.clear();
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::SetRowIndexToPresetIdCacheEntry(
  int row_index, int id)
{
  this->Internals->RowIndexToPresetIdCache[row_index] = id;
}

//----------------------------------------------------------------------------
int vtkKWPresetSelector::GetRowIndexToPresetIdCacheEntry(int row_index)
{
  vtkKWPresetSelectorInternals::RowIndexToPresetIdCacheTypeIterator it =
    this->Internals->RowIndexToPresetIdCache.find(row_index);
  if (it != this->Internals->RowIndexToPresetIdCache.end())
    {
    return it->second;
    }
  return -1;
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::InvalidateRowIndexToPresetIdCache()
{
  this->Internals->RowIndexToPresetIdCache.clear();
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::ProcessCallbackCommandEvents(vtkObject *caller,
                                                       unsigned long event,
                                                       void *calldata)
{
  vtkKWMultiColumnList *list = 
    this->PresetList ? this->PresetList->GetWidget() : NULL;

  if (caller == list)
    {
    switch (event)
      {
      case vtkKWMultiColumnList::NumberOfRowsChangedEvent:
        this->InvalidatePresetIdToRowIndexCache();
        this->InvalidateRowIndexToPresetIdCache();
        break;
      }
    }

  this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);
}

//----------------------------------------------------------------------------
void vtkKWPresetSelector::PrintSelf(ostream& os, vtkIndent indent)
{
  os << indent << "MaximumNumberOfPresets: " 
     << this->MaximumNumberOfPresets << endl;

  os << indent << "EmailBody: " 
     << (this->EmailBody ? this->EmailBody : "(None)") << endl;

  os << indent << "FilterButtonSlotName: " 
     << (this->FilterButtonSlotName ? this->FilterButtonSlotName : "(None)") << endl;

  this->Superclass::PrintSelf(os,indent);
}
