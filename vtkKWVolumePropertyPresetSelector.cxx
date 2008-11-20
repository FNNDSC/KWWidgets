/*=========================================================================

  Module:    $RCSfile: vtkKWVolumePropertyPresetSelector.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkKWVolumePropertyPresetSelector.h"

#include "vtkVolumeProperty.h"
#include "vtkKWInternationalization.h"
#include "vtkObjectFactory.h"
#include "vtkPiecewiseFunction.h"
#include "vtkColorTransferFunction.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWIcon.h"
#include "vtkKWVolumePropertyHelper.h"
#include "vtkKWToolbar.h"
#include "vtkKWPushButton.h"
#include "vtkKWPushButtonSet.h"
#include "vtkVolumeMapper.h"

#include <vtksys/stl/string>

#define VTK_KW_WLPS_TOLERANCE 0.005

const char *vtkKWVolumePropertyPresetSelector::TypeColumnName  = "Type";

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWVolumePropertyPresetSelector);
vtkCxxRevisionMacro(vtkKWVolumePropertyPresetSelector, "1.9");

//----------------------------------------------------------------------------
class vtkKWVolumePropertyPresetSelectorInternals
{
public:
  
  // User slot name for the default fields
  
  vtksys_stl::string TypeSlotName;
  vtksys_stl::string IndependentComponentsSlotName;
  vtksys_stl::string HistogramFlagSlotName;
  vtksys_stl::string BlendModeSlotName;

  // Range constraint

  double RangeConstraint[2];
  int HasRangeConstraint;
};

//----------------------------------------------------------------------------
vtkKWVolumePropertyPresetSelector::vtkKWVolumePropertyPresetSelector()
{
  this->Internals = new vtkKWVolumePropertyPresetSelectorInternals;
  this->Internals->TypeSlotName = 
    "DefaultTypeSlot";
  this->Internals->IndependentComponentsSlotName = 
    "DefaultIndependentComponentsSlot";
  this->Internals->HistogramFlagSlotName = 
    "DefaultHistogramFlagSlot";
  this->Internals->BlendModeSlotName = 
    "DefaultBlendModeSlot";
  this->Internals->HasRangeConstraint = 0;

  this->FilterButtonVisibility = 1;
  this->SetFilterButtonSlotName(this->GetPresetTypeSlotName());

  this->SetPresetButtonsBaseIconToPredefinedIcon(
    vtkKWIcon::IconNuvola16x16ActionsLedLightBlue);
}

//----------------------------------------------------------------------------
vtkKWVolumePropertyPresetSelector::~vtkKWVolumePropertyPresetSelector()
{
  delete this->Internals;
  this->Internals = NULL;
}

//----------------------------------------------------------------------------
int vtkKWVolumePropertyPresetSelector::SetPresetVolumeProperty(
  int id, vtkVolumeProperty *prop)
{
  if (this->HasPreset(id))
    {
    if (prop)
      {
      vtkVolumeProperty *ptr = this->GetPresetVolumeProperty(id);
      if (!ptr)
        {
        ptr = vtkVolumeProperty::New();
        vtkKWVolumePropertyHelper::DeepCopyVolumeProperty(ptr, prop);
        this->SetPresetUserSlotAsObject(id, "VolumeProperty", ptr);
        ptr->Delete();
        }
      else
        {
        vtkKWVolumePropertyHelper::DeepCopyVolumeProperty(ptr, prop);
        this->ScheduleUpdatePresetRow(id);
        }

      // Synchronize to the convenience IndependentComponents slot from
      // the volume property
      this->SetPresetIndependentComponents(
        id, prop->GetIndependentComponents());
      }
    else
      {
      this->DeletePresetUserSlot(id, "VolumeProperty");
      this->DeletePresetUserSlot(
        id, this->GetPresetIndependentComponentsSlotName());
      }
    return 1;
    }

  return 0;
}

//----------------------------------------------------------------------------
vtkVolumeProperty* vtkKWVolumePropertyPresetSelector::GetPresetVolumeProperty(
  int id)
{
  return (vtkVolumeProperty*)
    this->GetPresetUserSlotAsObject(id, "VolumeProperty");
}

//----------------------------------------------------------------------------
int vtkKWVolumePropertyPresetSelector::IsPresetRangeInsideRange(
  int id, double range[2])
{
  if (this->GetPresetHistogramFlag(id))
    {
    return 1;
    }

  vtkVolumeProperty *vprop = this->GetPresetVolumeProperty(id);
  if (!vprop)
    {
    return 0;
    }

  vtkPiecewiseFunction *so = vprop->GetScalarOpacity(0);
  if (so && (so->GetRange()[0] < range[0] || so->GetRange()[1] > range[1]))
    {
    return 0;
    }
  
  vtkColorTransferFunction *ctf = vprop->GetRGBTransferFunction(0);
  if (ctf && (ctf->GetRange()[0] < range[0] || ctf->GetRange()[1] > range[1]))
    {
    return 0;
    }
  
  return 1;
}

//----------------------------------------------------------------------------
int vtkKWVolumePropertyPresetSelector::SetPresetType(
  int id, const char *val)
{
  return this->SetPresetUserSlotAsString(
    id, this->GetPresetTypeSlotName(), val);
}

//----------------------------------------------------------------------------
const char* vtkKWVolumePropertyPresetSelector::GetPresetType(int id)
{
  return this->GetPresetUserSlotAsString(
    id, this->GetPresetTypeSlotName());
}

//----------------------------------------------------------------------------
int vtkKWVolumePropertyPresetSelector::SetPresetIndependentComponents(
  int id, int val)
{
  int res = this->SetPresetUserSlotAsInt(
    id, this->GetPresetIndependentComponentsSlotName(), val);

  // Synchronize from the convenience IndependentComponents slot to 
  // the volume property

  vtkVolumeProperty *ptr = this->GetPresetVolumeProperty(id);
  if (ptr)
    {
    ptr->SetIndependentComponents(val);
    }

  return res;
}

//----------------------------------------------------------------------------
int vtkKWVolumePropertyPresetSelector::GetPresetIndependentComponents(int id)
{
  return this->GetPresetUserSlotAsInt(
    id, this->GetPresetIndependentComponentsSlotName());
}

//----------------------------------------------------------------------------
int vtkKWVolumePropertyPresetSelector::SetPresetHistogramFlag(
  int id, int val)
{
  return this->SetPresetUserSlotAsInt(
    id, this->GetPresetHistogramFlagSlotName(), val);
}

//----------------------------------------------------------------------------
int vtkKWVolumePropertyPresetSelector::GetPresetHistogramFlag(int id)
{
  return this->GetPresetUserSlotAsInt(
    id, this->GetPresetHistogramFlagSlotName());
}

//----------------------------------------------------------------------------
int vtkKWVolumePropertyPresetSelector::SetPresetBlendMode(
  int id, int val)
{
  return this->SetPresetUserSlotAsInt(
    id, this->GetPresetBlendModeSlotName(), val);
}

//----------------------------------------------------------------------------
int vtkKWVolumePropertyPresetSelector::GetPresetBlendMode(int id)
{
  return this->GetPresetUserSlotAsInt(
    id, this->GetPresetBlendModeSlotName());
}

//----------------------------------------------------------------------------
int vtkKWVolumePropertyPresetSelector::HasPresetBlendMode(int id)
{
  return this->HasPresetUserSlot(
    id, this->GetPresetBlendModeSlotName());
}

//----------------------------------------------------------------------------
void vtkKWVolumePropertyPresetSelector::SetPresetTypeSlotName(const char *name)
{
  if (name && *name && 
      this->Internals && this->Internals->TypeSlotName.compare(name))
    {
    this->Internals->TypeSlotName = name;
    this->ScheduleUpdatePresetRows();
    }
}

//----------------------------------------------------------------------------
const char* vtkKWVolumePropertyPresetSelector::GetPresetTypeSlotName()
{
  if (this->Internals)
    {
    return this->Internals->TypeSlotName.c_str();
    }
  return NULL;
}

//----------------------------------------------------------------------------
void vtkKWVolumePropertyPresetSelector::SetPresetIndependentComponentsSlotName(const char *name)
{
  if (name && *name && 
      this->Internals && 
      this->Internals->IndependentComponentsSlotName.compare(name))
    {
    this->Internals->IndependentComponentsSlotName = name;
    this->ScheduleUpdatePresetRows();
    }
}

//----------------------------------------------------------------------------
const char* vtkKWVolumePropertyPresetSelector::GetPresetIndependentComponentsSlotName()
{
  if (this->Internals)
    {
    return this->Internals->IndependentComponentsSlotName.c_str();
    }
  return NULL;
}

//----------------------------------------------------------------------------
void vtkKWVolumePropertyPresetSelector::SetPresetHistogramFlagSlotName(const char *name)
{
  if (name && *name && 
      this->Internals && this->Internals->HistogramFlagSlotName.compare(name))
    {
    this->Internals->HistogramFlagSlotName = name;
    this->ScheduleUpdatePresetRows();
    }
}

//----------------------------------------------------------------------------
const char* vtkKWVolumePropertyPresetSelector::GetPresetHistogramFlagSlotName()
{
  if (this->Internals)
    {
    return this->Internals->HistogramFlagSlotName.c_str();
    }
  return NULL;
}

//----------------------------------------------------------------------------
void vtkKWVolumePropertyPresetSelector::SetPresetBlendModeSlotName(const char *name)
{
  if (name && *name && 
      this->Internals && this->Internals->BlendModeSlotName.compare(name))
    {
    this->Internals->BlendModeSlotName = name;
    this->ScheduleUpdatePresetRows();
    }
}

//----------------------------------------------------------------------------
const char* vtkKWVolumePropertyPresetSelector::GetPresetBlendModeSlotName()
{
  if (this->Internals)
    {
    return this->Internals->BlendModeSlotName.c_str();
    }
  return NULL;
}

//----------------------------------------------------------------------------
void vtkKWVolumePropertyPresetSelector::SetPresetFilterRangeConstraint(
  double range[2])
{
  if (this->Internals->HasRangeConstraint &&
      this->Internals->RangeConstraint[0] == range[0] &&
      this->Internals->RangeConstraint[1] == range[1])
    {
    return;
    }

  this->Internals->HasRangeConstraint = 1;
  this->Internals->RangeConstraint[0] = range[0];
  this->Internals->RangeConstraint[1] = range[1];

  this->PresetFilteringHasChanged();
  this->ScheduleUpdatePresetRows();
}

//----------------------------------------------------------------------------
double* vtkKWVolumePropertyPresetSelector::GetPresetFilterRangeConstraint()
{
  if (this->Internals->HasRangeConstraint)
    {
    return this->Internals->RangeConstraint;
    }
  return NULL;
}

//----------------------------------------------------------------------------
void vtkKWVolumePropertyPresetSelector::DeletePresetFilterRangeConstraint()
{
  if (!this->Internals->HasRangeConstraint)
    {
    return;
    }
  
  this->Internals->HasRangeConstraint = 0;

  this->PresetFilteringHasChanged();
  this->ScheduleUpdatePresetRows();
}

//----------------------------------------------------------------------------
int vtkKWVolumePropertyPresetSelector::IsPresetFiltered(int id)
{
  return this->Superclass::IsPresetFiltered(id) &&
    (!this->Internals->HasRangeConstraint ||
     this->IsPresetRangeInsideRange(id, this->Internals->RangeConstraint));
}

//----------------------------------------------------------------------------
void vtkKWVolumePropertyPresetSelector::CreateColumns()
{
  this->Superclass::CreateColumns();

  vtkKWMultiColumnList *list = this->PresetList->GetWidget();

  int col;

  // Type

  col = list->InsertColumn(
    this->GetCommentColumnIndex(), 
    ks_("Volume Property Preset Selector|Column|Type"));

  list->SetColumnName(col, 
                      vtkKWVolumePropertyPresetSelector::TypeColumnName);
  list->SetColumnResizable(col, 1);
  list->SetColumnStretchable(col, 0);
  list->SetColumnEditable(col, 0);
  list->ColumnVisibilityOff(col);
}

//----------------------------------------------------------------------------
int vtkKWVolumePropertyPresetSelector::GetTypeColumnIndex()
{
  return this->PresetList ? 
    this->PresetList->GetWidget()->GetColumnIndexWithName(
      vtkKWVolumePropertyPresetSelector::TypeColumnName) : -1;
}

//----------------------------------------------------------------------------
void vtkKWVolumePropertyPresetSelector::SetTypeColumnVisibility(int arg)
{
  if (this->PresetList)
    {
    this->PresetList->GetWidget()->SetColumnVisibility(
      this->GetTypeColumnIndex(), arg);
    }
}

//----------------------------------------------------------------------------
int vtkKWVolumePropertyPresetSelector::GetTypeColumnVisibility()
{
  if (this->PresetList)
    {
    return this->PresetList->GetWidget()->GetColumnVisibility(
      this->GetTypeColumnIndex());
    }
  return 0;
}

//----------------------------------------------------------------------------
int vtkKWVolumePropertyPresetSelector::UpdatePresetRow(int id)
{
  if (!this->Superclass::UpdatePresetRow(id))
    {
    return 0;
    }

  int row = this->GetPresetRow(id);
  if (row < 0)
    {
    return 0;
    }

  vtkKWMultiColumnList *list = this->PresetList->GetWidget();

  list->SetCellText(
    row, this->GetTypeColumnIndex(), this->GetPresetType(id));
  
  return 1;
}

//---------------------------------------------------------------------------
void vtkKWVolumePropertyPresetSelector::UpdateToolbarPresetButtons(vtkKWToolbar *toolbar)
{
  this->Superclass::UpdateToolbarPresetButtons(toolbar);

  if (!toolbar)
    {
    return;
    }

  int has_selection = 
    (this->PresetList && 
     this->PresetList->GetWidget()->GetNumberOfSelectedCells());

  int selected_id = has_selection ? this->GetIdOfSelectedPreset() : -1;

  int selection_is_histogram_preset = 
    selected_id >= 0 && this->GetPresetHistogramFlag(selected_id);

  vtkKWPushButton *toolbar_pb;

  // Update

  toolbar_pb = vtkKWPushButton::SafeDownCast(
    toolbar->GetWidget(this->GetUpdateButtonLabel()));
  if (toolbar_pb)
    {
    toolbar_pb->SetEnabled(
      toolbar_pb->GetEnabled() && !selection_is_histogram_preset);
    }
}

//----------------------------------------------------------------------------
void vtkKWVolumePropertyPresetSelector::AddDefaultNormalizedPresets(
  const char *type)
{
  vtkVolumeProperty *prop = NULL;
  int id = -1;

  const char *default_type = 
    ks_("Volume Property Preset Selector|Type|Default");
  if (!type)
    {
    type = default_type;
    }

  // -----------------------------------------------------------------
  // Independent Components Presets

  // Preset 1

  prop = vtkVolumeProperty::New();
  prop->SetIndependentComponents(1);
  vtkKWVolumePropertyHelper::ApplyPreset(
    prop, vtkKWVolumePropertyHelper::Preset1);

  id = this->AddPreset();
  this->SetPresetVolumeProperty(id, prop);
  this->SetPresetType(id, type);
  this->SetPresetHistogramFlag(id, 1);
  this->SetPresetBlendMode(id, vtkVolumeMapper::COMPOSITE_BLEND);
  this->SetPresetComment(id, "Ramp to 20%, White");
  prop->Delete();

  // Preset 2

  prop = vtkVolumeProperty::New();
  prop->SetIndependentComponents(1);
  vtkKWVolumePropertyHelper::ApplyPreset(
    prop, vtkKWVolumePropertyHelper::Preset2);

  id = this->AddPreset();
  this->SetPresetVolumeProperty(id, prop);
  this->SetPresetType(id, type);
  this->SetPresetHistogramFlag(id, 1);
  this->SetPresetBlendMode(id, vtkVolumeMapper::COMPOSITE_BLEND);
  this->SetPresetComment(id, "Ramp to 20%, Black to White");
  prop->Delete();

  // Preset 3

  prop = vtkVolumeProperty::New();
  prop->SetIndependentComponents(1);
  vtkKWVolumePropertyHelper::ApplyPreset(
    prop, vtkKWVolumePropertyHelper::Preset3);

  id = this->AddPreset();
  this->SetPresetVolumeProperty(id, prop);
  this->SetPresetType(id, type);
  this->SetPresetHistogramFlag(id, 1);
  this->SetPresetBlendMode(id, vtkVolumeMapper::COMPOSITE_BLEND);
  this->SetPresetComment(id, "Ramp to 20%, Rainbow");
  prop->Delete();

  // Preset 4

  prop = vtkVolumeProperty::New();
  prop->SetIndependentComponents(1);
  vtkKWVolumePropertyHelper::ApplyPreset(
    prop, vtkKWVolumePropertyHelper::Preset4);

  id = this->AddPreset();
  this->SetPresetVolumeProperty(id, prop);
  this->SetPresetType(id, type);
  this->SetPresetHistogramFlag(id, 1);
  this->SetPresetBlendMode(id, vtkVolumeMapper::COMPOSITE_BLEND);
  this->SetPresetComment(id, "Ramp between 25% & 50%, Tan");
  prop->Delete();

  // Preset 5

  prop = vtkVolumeProperty::New();
  prop->SetIndependentComponents(1);
  vtkKWVolumePropertyHelper::ApplyPreset(
    prop, vtkKWVolumePropertyHelper::Preset5);

  id = this->AddPreset();
  this->SetPresetVolumeProperty(id, prop);
  this->SetPresetType(id, type);
  this->SetPresetHistogramFlag(id, 1);
  this->SetPresetBlendMode(id, vtkVolumeMapper::COMPOSITE_BLEND);
  this->SetPresetComment(id, "Ramp between 50% & 75%, Tan");
  prop->Delete();

#if 0
  // No steps for now, until we can analyze histogram better

  // Preset 6

  prop = vtkVolumeProperty::New();
  prop->SetIndependentComponents(1);
  vtkKWVolumePropertyHelper::ApplyPreset(
    prop, vtkKWVolumePropertyHelper::Preset6);

  id = this->AddPreset();
  this->SetPresetVolumeProperty(id, prop);
  this->SetPresetType(id, type);
  this->SetPresetHistogramFlag(id, 1);
  this->SetPresetBlendMode(id, vtkVolumeMapper::COMPOSITE_BLEND);
  this->SetPresetComment(id, "Multicolor Steps");
  prop->Delete();
#endif

  // -----------------------------------------------------------------
  // Dependent Components Presets

  // Preset 3

  prop = vtkVolumeProperty::New();
  prop->SetIndependentComponents(0);
  vtkKWVolumePropertyHelper::ApplyPreset(
    prop, vtkKWVolumePropertyHelper::Preset3);

  id = this->AddPreset();
  this->SetPresetVolumeProperty(id, prop);
  this->SetPresetType(id, type);
  this->SetPresetHistogramFlag(id, 1);
  this->SetPresetBlendMode(id, vtkVolumeMapper::COMPOSITE_BLEND);
  this->SetPresetComment(id, "Ramp to 20%");
  prop->Delete();

  // Preset 4

  prop = vtkVolumeProperty::New();
  prop->SetIndependentComponents(0);
  vtkKWVolumePropertyHelper::ApplyPreset(
    prop, vtkKWVolumePropertyHelper::Preset4);

  id = this->AddPreset();
  this->SetPresetVolumeProperty(id, prop);
  this->SetPresetType(id, type);
  this->SetPresetHistogramFlag(id, 1);
  this->SetPresetBlendMode(id, vtkVolumeMapper::COMPOSITE_BLEND);
  this->SetPresetComment(id, "Ramp between 25% & 50%");
  prop->Delete();

  // Preset 5

  prop = vtkVolumeProperty::New();
  prop->SetIndependentComponents(0);
  vtkKWVolumePropertyHelper::ApplyPreset(
    prop, vtkKWVolumePropertyHelper::Preset5);

  id = this->AddPreset();
  this->SetPresetVolumeProperty(id, prop);
  this->SetPresetType(id, type);
  this->SetPresetHistogramFlag(id, 1);
  this->SetPresetBlendMode(id, vtkVolumeMapper::COMPOSITE_BLEND);
  this->SetPresetComment(id, "Ramp between 50% & 75%");
  prop->Delete();
}

//----------------------------------------------------------------------------
void vtkKWVolumePropertyPresetSelector::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
