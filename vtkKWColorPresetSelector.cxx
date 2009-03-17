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

#include "vtkKWColorPresetSelector.h"

#include "vtkObjectFactory.h"
#include "vtkMath.h"
#include "vtkXMLUtilities.h"
#include "vtkXMLDataElement.h"

#include "vtkKWApplication.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWInternationalization.h"
#include "vtkKWIcon.h"
#include "vtkKWRegistryHelper.h"

#include <vtksys/ios/sstream>

const char *vtkKWColorPresetSelector::ColorColumnName = "Color";

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWColorPresetSelector);
vtkCxxRevisionMacro(vtkKWColorPresetSelector, "$Revision: 1.37 $");

//----------------------------------------------------------------------------
vtkKWColorPresetSelector::vtkKWColorPresetSelector()
{
  this->DrawColorCellAsColorButton = 1;
  this->RegistryKey = NULL;
}

//----------------------------------------------------------------------------
vtkKWColorPresetSelector::~vtkKWColorPresetSelector()
{
  this->SetRegistryKey(NULL);
}

//----------------------------------------------------------------------------
int vtkKWColorPresetSelector::SetPresetColorAsRGB(
  int id, double r, double g, double b)
{
  return (this->SetPresetUserSlotAsDouble(id, "R", r) &&
          this->SetPresetUserSlotAsDouble(id, "G", g) &&
          this->SetPresetUserSlotAsDouble(id, "B", b)) ? 1 : 0;
}

//----------------------------------------------------------------------------
int vtkKWColorPresetSelector::SetPresetColorAsRGB(
  int id, double rgb[3])
{
  return this->SetPresetColorAsRGB(id, rgb[0], rgb[1], rgb[2]);
}

//----------------------------------------------------------------------------
void vtkKWColorPresetSelector::GetPresetColorAsRGB(
  int id, double &r, double &g, double &b)
{
  r = this->GetPresetUserSlotAsDouble(id, "R");
  g = this->GetPresetUserSlotAsDouble(id, "G");
  b = this->GetPresetUserSlotAsDouble(id, "B");
}

//----------------------------------------------------------------------------
void vtkKWColorPresetSelector::GetPresetColorAsRGB(
  int id, double rgb[3])
{
  this->GetPresetColorAsRGB(id, rgb[0], rgb[1], rgb[2]);
}

//----------------------------------------------------------------------------
int vtkKWColorPresetSelector::SetPresetColorAsHSV(
  int id, double h, double s, double v)
{
  double r, g, b;
  vtkMath::HSVToRGB(h, s, v, &r, &g, &b);
  return this->SetPresetColorAsRGB(id, r, g, b);
}

//----------------------------------------------------------------------------
int vtkKWColorPresetSelector::SetPresetColorAsHSV(
  int id, double hsv[3])
{
  return this->SetPresetColorAsHSV(id, hsv[0], hsv[1], hsv[2]);
}

//----------------------------------------------------------------------------
void vtkKWColorPresetSelector::GetPresetColorAsHSV(
  int id, double &h, double &s, double &v)
{
  double r, g, b;
  this->GetPresetColorAsRGB(id, r, g, b);
  vtkMath::RGBToHSV(r, g, b, &h, &s, &v);
}

//----------------------------------------------------------------------------
void vtkKWColorPresetSelector::GetPresetColorAsHSV(
  int id, double hsv[3])
{
  this->GetPresetColorAsHSV(id, hsv[0], hsv[1], hsv[2]);
}

//----------------------------------------------------------------------------
int vtkKWColorPresetSelector::GetIdOfPresetWithColorAsRGB(
  double r, double g, double b)
{
  double p_r, p_g, p_b;
  int i, nb_presets = this->GetNumberOfPresets();
  for (i = 0; i < nb_presets; i++)
    {
    int id = this->GetIdOfNthPreset(i);
    if (id >= 0)
      {
      this->GetPresetColorAsRGB(id, p_r, p_g, p_b);
      if (this->CompareRGBColors(r, g, b, p_r, p_g, p_b))
        {
        return id;
        }
      }
    }

  return -1;
}

//----------------------------------------------------------------------------
int vtkKWColorPresetSelector::GetIdOfPresetWithColorAsRGB(double rgb[3])
{
  return this->GetIdOfPresetWithColorAsRGB(rgb[0], rgb[1], rgb[2]);
}

//----------------------------------------------------------------------------
void vtkKWColorPresetSelector::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  this->SetPresetButtonsBaseIconToPredefinedIcon(
    vtkKWIcon::IconDocumentRGBColor);

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  // --------------------------------------------------------------
  // Preset : preset list

  vtkKWMultiColumnList *list = this->PresetList->GetWidget();
  list->SetRowSpacing(0);

  this->RestorePresetsFromRegistry();
}

//----------------------------------------------------------------------------
void vtkKWColorPresetSelector::CreateColumns()
{
  this->Superclass::CreateColumns();

  vtkKWMultiColumnList *list = this->PresetList->GetWidget();

  // Needed since we are using special user-defined window for the
  // visibility and color cells

  list->SetPotentialCellColorsChangedCommand(NULL, NULL);
  
  int col;

  // Color

  col = list->InsertColumn(this->GetCommentColumnIndex(), NULL);
  list->SetColumnName(col, vtkKWColorPresetSelector::ColorColumnName);
  list->SetColumnResizable(col, 0);
  list->SetColumnStretchable(col, 0);
  list->SetColumnEditable(col, 0);
  list->SetColumnLabelImageToPredefinedIcon(col, vtkKWIcon::IconEmpty16x16);
  list->SetColumnFormatCommandToEmptyOutput(col);
}

//----------------------------------------------------------------------------
int vtkKWColorPresetSelector::GetColorColumnIndex()
{
  return this->PresetList ?
    this->PresetList->GetWidget()->GetColumnIndexWithName(
      vtkKWColorPresetSelector::ColorColumnName) : -1;
}

//----------------------------------------------------------------------------
int vtkKWColorPresetSelector::UpdatePresetRow(int id)
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

  // Color

  double r, g, b;
  this->GetPresetColorAsRGB(id, r, g, b);
  
  double range[2] =  {0.0, 1.0 };
  vtkMath::ClampValue(&r, range);
  vtkMath::ClampValue(&g, range);
  vtkMath::ClampValue(&b, range);

  if (this->DrawColorCellAsColorButton)
    {
    char buffer[256];
    sprintf(buffer, "%g %g %g", r, g, b);
    int col_index = this->GetColorColumnIndex();
    list->SetCellText(row, col_index, buffer);
    list->SetCellWindowCommandToColorButton(row, col_index);
    }
  else
    {
    list->SetCellBackgroundColor(
      row, this->GetColorColumnIndex(), r, g, b);
    list->SetCellSelectionBackgroundColor(
      row, this->GetColorColumnIndex(), r, g, b);
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkKWColorPresetSelector::SetPresetCommentAsHexadecimalRGB(int id)
{
  double r, g, b;
  this->GetPresetColorAsRGB(id, r, g, b);
  
  double range[2] =  {0.0, 1.0 };
  vtkMath::ClampValue(&r, range);
  vtkMath::ClampValue(&g, range);
  vtkMath::ClampValue(&b, range);

  char color[10];
  sprintf(color, 
          "#%02x%02x%02x", 
          vtkMath::Round(r * 255.0), 
          vtkMath::Round(g * 255.0), 
          vtkMath::Round(b * 255.0));

  return this->SetPresetComment(id, color);
}

//----------------------------------------------------------------------------
int vtkKWColorPresetSelector::GetPresetCommentAsHexadecimalRGB(
  int id, int &r, int &g, int &b)
{
  const char *value = this->GetPresetComment(id);
  return (!value || !*value || strlen(value) != 6 ||
          sscanf(value, "%02x%02x%02x", &r, &g, &b) != 3) ? 0 : 1;
}

//----------------------------------------------------------------------------
int vtkKWColorPresetSelector::HasPresetCommentAsHexadecimalRGB(int id)
{
  int r, g, b;
  return this->GetPresetCommentAsHexadecimalRGB(id, r, g, b);
}

//----------------------------------------------------------------------------
void vtkKWColorPresetSelector::AddDefaultColorPresets()
{
  const char *colors[] = 
    {
      "#ff8080", "#ffff80", "#80ff80", "#00ff80", 
      "#80ffff", "#0080ff", "#ff80c0", "#ff80ff",
      "#ff0000", "#ffff00", "#80ff00", "#00ff40", 
      "#00ffff", "#0080c0", "#8080c0", "#ff00ff",
      "#804040", "#ff8040", "#00ff00", "#008080", 
      "#004080", "#8080ff", "#800040", "#ff0080",
      "#800000", "#ff8000", "#008000", "#008040", 
      "#0000ff", "#0000a0", "#800080", "#8000ff",
      "#400000", "#804000", "#004000", "#004040", 
      "#000080", "#000040", "#400040", "#400080",
      "#000000", "#808000", "#808040", "#808080", 
      "#408080", "#c0c0c0", "#400040", "#ffffff"
    };

  int i, id;
  int r, g, b;

  for (i = 0; i < sizeof(colors) / sizeof(colors[0]); i++)
    {
    if (sscanf(colors[i], "#%02x%02x%02x", &r, &g, &b) == 3)
      {
      id = this->AddPreset();
      this->SetPresetColorAsRGB(
        id, (double)r / 255.0, (double)g / 255.0, (double)b / 255.0);
      this->SetPresetComment(id, colors[i]);
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWColorPresetSelector::SetDrawColorCellAsColorButton(int arg)
{
  if (this->DrawColorCellAsColorButton == arg)
    {
    return;
    }

  this->DrawColorCellAsColorButton = arg;
  this->Modified();

  this->ScheduleUpdatePresetRows();
}

//----------------------------------------------------------------------------
void vtkKWColorPresetSelector::SavePresetsToRegistry()
{
  if (!this->RegistryKey)
    {
    return;
    }

  vtkXMLDataElement *colors = vtkXMLDataElement::New();
  colors->SetName("colors");

  int i, nb_presets = this->GetNumberOfPresets();
  for (i = 0; i < nb_presets; i++)
    {
    int id = this->GetIdOfNthPreset(i);
    if (id >= 0)
      {
      double rgb[3];
      this->GetPresetColorAsRGB(id, rgb);
      vtkXMLDataElement *color = vtkXMLDataElement::New();
      color->SetName("color");
      color->SetVectorAttribute("rgb", 3, rgb);
      color->SetAttribute("comment", this->GetPresetComment(id));
      vtksys_ios::ostringstream c_time_str;
      c_time_str << this->GetPresetCreationTime(id);
      color->SetAttribute("c_time", c_time_str.str().c_str());
      colors->AddNestedElement(color);
      color->Delete();
      }
    }

  vtksys_ios::ostringstream str;
  vtkXMLUtilities::FlattenElement(colors, str, NULL, 0);
  colors->Delete();
  
  this->GetApplication()->SetRegistryValue(
    2, "Colors", this->RegistryKey, "%s", str.str().c_str());
}

//----------------------------------------------------------------------------
void vtkKWColorPresetSelector::RestorePresetsFromRegistry()
{
  this->DeleteAllPresets();

  char buffer[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  if (!this->RegistryKey || 
      !this->GetApplication()->HasRegistryValue(
        2, "Colors", this->RegistryKey) ||
      !this->GetApplication()->GetRegistryValue(
        2, "Colors", this->RegistryKey, buffer))
    {
    return;
    }

  vtkXMLDataElement *colors = vtkXMLUtilities::ReadElementFromString(buffer);
  if (!colors)
    {
    return;
    }

  if (!strcmp(colors->GetName(), "colors"))
    {
    for (int i = 0; i < colors->GetNumberOfNestedElements(); i++)
      {
      vtkXMLDataElement *color = colors->GetNestedElement(i);
      if (!color || strcmp(color->GetName(), "color"))
        {
        continue;
        }
      double rgb[3];
      if (color->GetVectorAttribute("rgb", 3, rgb) == 3)
        {
        int id = this->AddPreset();
        if (id >= 0)
          {
          this->SetPresetColorAsRGB(id, rgb);
          }
        const char *comment = color->GetAttribute("comment");
        if (comment)
          {
          this->SetPresetComment(id, comment);
          }
        const char *c_time_ptr = color->GetAttribute("c_time");
        if (c_time_ptr)
          {
          vtksys_ios::istringstream c_time_str(c_time_ptr);
          vtkTypeInt64 c_time;
          c_time_str >> c_time;
          this->SetPresetCreationTime(id, c_time);
          }
        }
      }
    }

  colors->Delete();
}

//----------------------------------------------------------------------------
int vtkKWColorPresetSelector::InvokePresetAddCommand()
{
  int id = this->Superclass::InvokePresetAddCommand();
  this->SavePresetsToRegistry();
  return id;
}

//----------------------------------------------------------------------------
void vtkKWColorPresetSelector::InvokePresetRemovedCommand()
{
  this->Superclass::InvokePresetRemovedCommand();
  this->SavePresetsToRegistry();
}

//----------------------------------------------------------------------------
void vtkKWColorPresetSelector::InvokePresetUpdateCommand(int id)
{
  this->Superclass::InvokePresetUpdateCommand(id);
  this->SavePresetsToRegistry();
}

//----------------------------------------------------------------------------
void vtkKWColorPresetSelector::InvokePresetHasChangedCommand(int id)
{
  this->Superclass::InvokePresetHasChangedCommand(id);
  this->SavePresetsToRegistry();
}

//----------------------------------------------------------------------------
int vtkKWColorPresetSelector::RemoveDuplicatedColors()
{
  // This is pretty slow but hey

  int nb_removed = 0;
  int i = 0;
  do
    {
    int id = this->GetIdOfNthPreset(i);
    if (id >= 0)
      {
      double rgb1[3];
      this->GetPresetColorAsRGB(id, rgb1);
      int found;
      do
        {
        found = 0;
        int nb_presets = this->GetNumberOfPresets();
        for (int j = i + 1; j < nb_presets; j++)
          {
          int p_id = this->GetIdOfNthPreset(j);
          if (p_id >= 0)
            {
            double rgb2[3];
            this->GetPresetColorAsRGB(p_id, rgb2);
            if (this->CompareRGBColors(rgb1, rgb2) && this->RemovePreset(p_id))
              {
              nb_removed++;
              found = 1;
              break;
              }
            }
          }
        } while (found);
      }
    ++i;
    } while (i < this->GetNumberOfPresets());

  return nb_removed;
}

//----------------------------------------------------------------------------
int vtkKWColorPresetSelector::CompareRGBColors(
  double r1, double g1, double b1, 
  double r2, double g2, double b2)
{
  double epsilon = 0.001;
  return (fabs(r1 - r2) < epsilon &&
          fabs(g1 - g2) < epsilon &&
          fabs(b1 - b2) < epsilon) ? 1 : 0;
}

//----------------------------------------------------------------------------
int vtkKWColorPresetSelector::CompareRGBColors(double rgb1[3], double rgb2[3])
{
  return vtkKWColorPresetSelector::CompareRGBColors(
    rgb1[0], rgb1[1], rgb1[2],
    rgb2[0], rgb2[1], rgb2[2]);
}

//----------------------------------------------------------------------------
void vtkKWColorPresetSelector::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
