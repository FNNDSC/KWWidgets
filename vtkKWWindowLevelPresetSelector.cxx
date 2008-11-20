/*=========================================================================

Copyright (c) 1998-2003 Kitware Inc. 469 Clifton Corporate Parkway,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/

#include "vtkKWWindowLevelPresetSelector.h"

#include "vtkObjectFactory.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWInternationalization.h"
#include "vtkKWIcon.h"

#include <vtksys/stl/string>

const char *vtkKWWindowLevelPresetSelector::WindowColumnName = "Window";
const char *vtkKWWindowLevelPresetSelector::LevelColumnName  = "Level";
const char *vtkKWWindowLevelPresetSelector::TypeColumnName  = "Type";

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWWindowLevelPresetSelector);
vtkCxxRevisionMacro(vtkKWWindowLevelPresetSelector, "1.17");

//----------------------------------------------------------------------------
class vtkKWWindowLevelPresetSelectorInternals
{
public:
  
  // User slot name for the default fields
  
  vtksys_stl::string TypeSlotName;
};

//----------------------------------------------------------------------------
vtkKWWindowLevelPresetSelector::vtkKWWindowLevelPresetSelector()
{
  this->Internals = new vtkKWWindowLevelPresetSelectorInternals;
  this->Internals->TypeSlotName = "DefaultTypeSlot";

  this->FilterButtonVisibility = 0; // not 1 since the Type column is hidden;
  this->SetFilterButtonSlotName(this->GetPresetTypeSlotName());

  this->SetPresetButtonsBaseIconToPredefinedIcon(vtkKWIcon::IconContrast);
}

//----------------------------------------------------------------------------
vtkKWWindowLevelPresetSelector::~vtkKWWindowLevelPresetSelector()
{
  delete this->Internals;
  this->Internals = NULL;
}

//----------------------------------------------------------------------------
int vtkKWWindowLevelPresetSelector::SetPresetWindow(
  int id, double val)
{
  return this->SetPresetUserSlotAsDouble(id, "Window", val);
}

//----------------------------------------------------------------------------
double vtkKWWindowLevelPresetSelector::GetPresetWindow(int id)
{
  return this->GetPresetUserSlotAsDouble(id, "Window");
}

//----------------------------------------------------------------------------
int vtkKWWindowLevelPresetSelector::SetPresetLevel(
  int id, double val)
{
  return this->SetPresetUserSlotAsDouble(id, "Level", val);
}

//----------------------------------------------------------------------------
double vtkKWWindowLevelPresetSelector::GetPresetLevel(int id)
{
  return this->GetPresetUserSlotAsDouble(id, "Level");
}

//----------------------------------------------------------------------------
int vtkKWWindowLevelPresetSelector::SetPresetType(
  int id, const char *val)
{
  return this->SetPresetUserSlotAsString(
    id, this->GetPresetTypeSlotName(), val);
}

//----------------------------------------------------------------------------
const char* vtkKWWindowLevelPresetSelector::GetPresetType(int id)
{
  return this->GetPresetUserSlotAsString(
    id, this->GetPresetTypeSlotName());
}

//----------------------------------------------------------------------------
void vtkKWWindowLevelPresetSelector::SetPresetTypeSlotName(const char *name)
{
  if (name && *name && 
      this->Internals && this->Internals->TypeSlotName.compare(name))
    {
    this->Internals->TypeSlotName = name;
    this->ScheduleUpdatePresetRows();
    }
}

//----------------------------------------------------------------------------
const char* vtkKWWindowLevelPresetSelector::GetPresetTypeSlotName()
{
  if (this->Internals)
    {
    return this->Internals->TypeSlotName.c_str();
    }
  return NULL;
}

//----------------------------------------------------------------------------
void vtkKWWindowLevelPresetSelector::CreateColumns()
{
  this->Superclass::CreateColumns();

  vtkKWMultiColumnList *list = this->PresetList->GetWidget();

  int col;

  // Type

  col = list->InsertColumn(
    this->GetCommentColumnIndex(), 
    ks_("Window/Level Preset Selector|Column|Type"));
  list->SetColumnName(col, vtkKWWindowLevelPresetSelector::TypeColumnName);
  list->SetColumnResizable(col, 1);
  list->SetColumnStretchable(col, 0);
  list->SetColumnEditable(col, 0);
  list->ColumnVisibilityOff(col);

  // Window

  col = list->InsertColumn(
    col + 1, ks_("Window/Level Preset Selector|Column|Window|W"));
  list->SetColumnName(col, vtkKWWindowLevelPresetSelector::WindowColumnName);
  list->SetColumnWidth(col, 6);
  list->SetColumnResizable(col, 1);
  list->SetColumnStretchable(col, 0);
  list->SetColumnEditable(col, 1);
  list->SetColumnSortModeToReal(col);

  // Level

  col = list->InsertColumn(
    col + 1, ks_("Window/Level Preset Selector|Level|L"));
  list->SetColumnName(col, vtkKWWindowLevelPresetSelector::LevelColumnName);
  list->SetColumnWidth(col, 6);
  list->SetColumnResizable(col, 1);
  list->SetColumnStretchable(col, 0);
  list->SetColumnEditable(col, 1);
  list->SetColumnSortModeToReal(col);
}

//----------------------------------------------------------------------------
int vtkKWWindowLevelPresetSelector::GetWindowColumnIndex()
{
  return this->PresetList ? 
    this->PresetList->GetWidget()->GetColumnIndexWithName(
      vtkKWWindowLevelPresetSelector::WindowColumnName) : -1;
}

//----------------------------------------------------------------------------
int vtkKWWindowLevelPresetSelector::GetLevelColumnIndex()
{
  return this->PresetList ? 
    this->PresetList->GetWidget()->GetColumnIndexWithName(
      vtkKWWindowLevelPresetSelector::LevelColumnName) : -1;
}

//----------------------------------------------------------------------------
int vtkKWWindowLevelPresetSelector::GetTypeColumnIndex()
{
  return this->PresetList ? 
    this->PresetList->GetWidget()->GetColumnIndexWithName(
      vtkKWWindowLevelPresetSelector::TypeColumnName) : -1;
}

//----------------------------------------------------------------------------
void vtkKWWindowLevelPresetSelector::SetTypeColumnVisibility(int arg)
{
  if (this->PresetList)
    {
    this->PresetList->GetWidget()->SetColumnVisibility(
      this->GetTypeColumnIndex(), arg);
    }
}

//----------------------------------------------------------------------------
int vtkKWWindowLevelPresetSelector::GetTypeColumnVisibility()
{
  if (this->PresetList)
    {
    return this->PresetList->GetWidget()->GetColumnVisibility(
      this->GetTypeColumnIndex());
    }
  return 0;
}

//----------------------------------------------------------------------------
int vtkKWWindowLevelPresetSelector::UpdatePresetRow(int id)
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
  
  list->SetCellTextAsFormattedDouble(
    row, this->GetWindowColumnIndex(), this->GetPresetWindow(id), 5);

  list->SetCellTextAsFormattedDouble(
    row, this->GetLevelColumnIndex(), this->GetPresetLevel(id), 5);
  
  return 1;
}

//---------------------------------------------------------------------------
const char* vtkKWWindowLevelPresetSelector::PresetCellEditEndCallback(
  int row, int col, const char *text)
{
  static char buffer[256];

  int id = this->GetIdOfPresetAtRow(row);
  if (this->HasPreset(id))
    {
    if (col == this->GetWindowColumnIndex() || 
        col == this->GetLevelColumnIndex())
      {
      double val = atof(text);
      sprintf(buffer, "%g", val);
      return buffer;
      }
    }
  return this->Superclass::PresetCellEditEndCallback(row, col, text);
}

//---------------------------------------------------------------------------
void vtkKWWindowLevelPresetSelector::PresetCellUpdatedCallback(
  int row, int col, const char *text)
{
  int id = this->GetIdOfPresetAtRow(row);
  if (this->HasPreset(id))
    {
    if (col == this->GetWindowColumnIndex() || 
        col == this->GetLevelColumnIndex())
      {
        double val = atof(text);
        if (col == this->GetWindowColumnIndex())
          {
          this->SetPresetWindow(id, val);
          }
        else
          {
          this->SetPresetLevel(id, val);
          }
        if (this->ApplyPresetOnSelection)
          {
          this->InvokePresetApplyCommand(id);
          }
        this->InvokePresetHasChangedCommand(id);
        return;
      }
    }

  this->Superclass::PresetCellUpdatedCallback(row, col, text);
}

//----------------------------------------------------------------------------
int vtkKWWindowLevelPresetSelector::HasPresetWithGroupWithWindowLevel(
  const char *group, double window, double level)
{
  int i, nb_presets = this->GetNumberOfPresetsWithGroup(group);
  for (i = 0; i < nb_presets; i++)
    {
    int id = this->GetIdOfNthPresetWithGroup(i, group);
    if (this->GetPresetWindow(id) == window && 
        this->GetPresetLevel(id) == level)
      {
      return 1;
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWWindowLevelPresetSelector::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
