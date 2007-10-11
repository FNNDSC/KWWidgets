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

#define VTK_KW_WLPS_TOLERANCE 0.005

const char *vtkKWVolumePropertyPresetSelector::ModalityColumnName  = "Modality";

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWVolumePropertyPresetSelector);
vtkCxxRevisionMacro(vtkKWVolumePropertyPresetSelector, "$Revision: 1.10 $");

//----------------------------------------------------------------------------
vtkKWVolumePropertyPresetSelector::vtkKWVolumePropertyPresetSelector()
{
  vtkKWIcon *icon = vtkKWIcon::New();
  icon->SetImage(vtkKWIcon::IconDocumentVolumeProperty);
  icon->TrimTop();
  icon->TrimRight();
  this->SetPresetButtonsBaseIcon(icon);
  icon->Delete();
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
        this->DeepCopyVolumeProperty(ptr, prop);
        this->SetPresetUserSlotAsObject(id, "VolumeProperty", ptr);
        ptr->Delete();
        }
      else
        {
        this->DeepCopyVolumeProperty(ptr, prop);
        this->ScheduleUpdatePresetRow(id);
        }
      }
    else
      {
      this->DeletePresetUserSlot(id, "VolumeProperty");
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
int vtkKWVolumePropertyPresetSelector::SetPresetModality(
  int id, const char *val)
{
  return this->SetPresetUserSlotAsString(id, "Modality", val);
}

//----------------------------------------------------------------------------
const char* vtkKWVolumePropertyPresetSelector::GetPresetModality(int id)
{
  return this->GetPresetUserSlotAsString(id, "Modality");
}

//----------------------------------------------------------------------------
void vtkKWVolumePropertyPresetSelector::CreateColumns()
{
  this->Superclass::CreateColumns();

  vtkKWMultiColumnList *list = this->PresetList->GetWidget();

  int col;

  // Modality

  col = list->InsertColumn(
    this->GetCommentColumnIndex(), 
    ks_("Volume Property Preset Selector|Column|Modality"));

  list->SetColumnName(col, 
                      vtkKWVolumePropertyPresetSelector::ModalityColumnName);
  list->SetColumnResizable(col, 1);
  list->SetColumnStretchable(col, 0);
  list->SetColumnEditable(col, 0);
  list->ColumnVisibilityOff(col);
}

//----------------------------------------------------------------------------
int vtkKWVolumePropertyPresetSelector::GetModalityColumnIndex()
{
  return this->PresetList ? 
    this->PresetList->GetWidget()->GetColumnIndexWithName(
      vtkKWVolumePropertyPresetSelector::ModalityColumnName) : -1;
}

//----------------------------------------------------------------------------
void vtkKWVolumePropertyPresetSelector::SetModalityColumnVisibility(int arg)
{
  if (this->PresetList)
    {
    this->PresetList->GetWidget()->SetColumnVisibility(
      this->GetModalityColumnIndex(), arg);
    }
}

//----------------------------------------------------------------------------
int vtkKWVolumePropertyPresetSelector::GetModalityColumnVisibility()
{
  if (this->PresetList)
    {
    return this->PresetList->GetWidget()->GetColumnVisibility(
      this->GetModalityColumnIndex());
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
    row, this->GetModalityColumnIndex(), this->GetPresetModality(id));
  
  return 1;
}

//----------------------------------------------------------------------------
void vtkKWVolumePropertyPresetSelector::DeepCopyVolumeProperty(
    vtkVolumeProperty *target, vtkVolumeProperty *source)
{
  if (!target || !source)
    {
    return;
    }

#if VTK_MAJOR_VERSION > 5 || (VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION > 0)
  target->DeepCopy(source);
#else
  target->SetIndependentComponents(source->GetIndependentComponents());

  target->SetInterpolationType(source->GetInterpolationType());

  for (int i = 0; i < VTK_MAX_VRCOMP; i++)
    {
    target->SetComponentWeight(i, source->GetComponentWeight(i));
    
    // Force ColorChannels to the right value and/or create a default tfunc
    // then DeepCopy all the points

    if (source->GetColorChannels(i) > 1)
      {
      target->SetColor(i, target->GetRGBTransferFunction(i));
      target->GetRGBTransferFunction(i)->DeepCopy(
        source->GetRGBTransferFunction(i));
      }
    else
      {
      target->SetColor(i, target->GetGrayTransferFunction(i));
      target->GetGrayTransferFunction(i)->DeepCopy(
        source->GetGrayTransferFunction(i));
      }

    target->GetScalarOpacity(i)->DeepCopy(source->GetScalarOpacity(i));

    target->SetScalarOpacityUnitDistance(
      i, source->GetScalarOpacityUnitDistance(i));

    target->GetGradientOpacity(i)->DeepCopy(source->GetGradientOpacity(i));

    target->SetDisableGradientOpacity(i, source->GetDisableGradientOpacity(i));

    target->SetShade(i, source->GetShade(i));
    target->SetAmbient(i, source->GetAmbient(i));
    target->SetDiffuse(i, source->GetDiffuse(i));
    target->SetSpecular(i, source->GetSpecular(i));
    target->SetSpecularPower(i, source->GetSpecularPower(i));
    }
#endif
}

//----------------------------------------------------------------------------
void vtkKWVolumePropertyPresetSelector::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
