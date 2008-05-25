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

#include "vtkKWDirectoryPresetSelector.h"

#include "vtkObjectFactory.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWInternationalization.h"
#include "vtkKWIcon.h"
#include "vtkKWFileBrowserDialog.h"

#include <vtksys/SystemTools.hxx>
#include <vtksys/stl/vector>
#include <vtksys/stl/list>
#include <vtksys/ios/sstream>
#include <vtksys/stl/algorithm>

const char *vtkKWDirectoryPresetSelector::DirectoryColumnName  = "Directory";
const char *vtkKWDirectoryPresetSelector::DirectoryEnabledColumnName = "DirectoryEnabled";

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWDirectoryPresetSelector);
vtkCxxRevisionMacro(vtkKWDirectoryPresetSelector, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkKWDirectoryPresetSelector::vtkKWDirectoryPresetSelector()
{
  vtkKWIcon *icon = vtkKWIcon::New();
  icon->SetImage(vtkKWIcon::IconFolderXP);
  icon->TrimTop();
  icon->TrimRight();
  this->SetPresetButtonsBaseIcon(icon);
  icon->Delete();

  this->SelectSpinButtonsVisibility = 0;
  this->UniqueDirectories = 0;

  this->FileBrowserDialog = vtkKWFileBrowserDialog::New();

  this->MaximumDirectoryLength = 0;
}

//----------------------------------------------------------------------------
vtkKWDirectoryPresetSelector::~vtkKWDirectoryPresetSelector()
{
  if (this->FileBrowserDialog)
    {
    this->FileBrowserDialog->Delete();
    this->FileBrowserDialog = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkKWDirectoryPresetSelector::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro("vtkKWDirectoryPresetSelector already created");
    return;
    }

  // Call the superclass, this will set the application and 
  // create the pushbutton.

  this->Superclass::CreateWidget();

  this->FileBrowserDialog->SetParent(this);
  this->FileBrowserDialog->ChooseDirectoryOn();
  this->FileBrowserDialog->SaveDialogOff();

  this->SetCommentColumnVisibility(0);

  this->SetPresetAddCommand(this, "AddDirectoryCallback");

  vtkKWMultiColumnList *list = this->PresetList->GetWidget();
  list->SetRowSpacing(1);
}

//----------------------------------------------------------------------------
int vtkKWDirectoryPresetSelector::SetPresetDirectory(
  int id, const char *val)
{
  return this->SetPresetUserSlotAsString(id, "Directory", val);
}

//----------------------------------------------------------------------------
const char* vtkKWDirectoryPresetSelector::GetPresetDirectory(int id)
{
  return this->GetPresetUserSlotAsString(id, "Directory");
}

//----------------------------------------------------------------------------
int vtkKWDirectoryPresetSelector::SetPresetDirectoryEnabled(
  int id, int val)
{
  return this->SetPresetUserSlotAsInt(id, "DirectoryEnabled", val);
}

//----------------------------------------------------------------------------
int vtkKWDirectoryPresetSelector::GetPresetDirectoryEnabled(int id)
{
  return this->GetPresetUserSlotAsInt(id, "DirectoryEnabled");
}

//----------------------------------------------------------------------------
void vtkKWDirectoryPresetSelector::CreateColumns()
{
  this->Superclass::CreateColumns();

  vtkKWMultiColumnList *list = this->PresetList->GetWidget();

  int col;

  // DirectoryEnabled

  col = list->InsertColumn(this->GetCommentColumnIndex(), NULL);
  list->SetColumnName(col, vtkKWDirectoryPresetSelector::DirectoryEnabledColumnName);
  list->SetColumnResizable(col, 0);
  list->SetColumnStretchable(col, 0);
  list->SetColumnEditable(col, 1);
  list->SetColumnFormatCommandToEmptyOutput(col);
  list->SetColumnWidth(col, 3);

  // Directory

  col = list->InsertColumn(
    col + 1, ks_("Directory Preset Selector|Column|Directory"));
  list->SetColumnName(col, vtkKWDirectoryPresetSelector::DirectoryColumnName);
  list->SetColumnResizable(col, 1);
  list->SetColumnStretchable(col, 1);
  list->SetColumnEditable(col, 1);
}

//----------------------------------------------------------------------------
int vtkKWDirectoryPresetSelector::GetDirectoryColumnIndex()
{
  return this->PresetList ? 
    this->PresetList->GetWidget()->GetColumnIndexWithName(
      vtkKWDirectoryPresetSelector::DirectoryColumnName) : -1;
}

//----------------------------------------------------------------------------
int vtkKWDirectoryPresetSelector::GetDirectoryEnabledColumnIndex()
{
  return this->PresetList ? 
    this->PresetList->GetWidget()->GetColumnIndexWithName(
      vtkKWDirectoryPresetSelector::DirectoryEnabledColumnName) : -1;
}

//----------------------------------------------------------------------------
void vtkKWDirectoryPresetSelector::SetDirectoryEnabledColumnVisibility(int arg)
{
  if (this->PresetList)
    {
    this->PresetList->GetWidget()->SetColumnVisibility(
      this->GetDirectoryEnabledColumnIndex(), arg);
    }
}

//----------------------------------------------------------------------------
int vtkKWDirectoryPresetSelector::GetDirectoryEnabledColumnVisibility()
{
  if (this->PresetList)
    {
    return this->PresetList->GetWidget()->GetColumnVisibility(
      this->GetDirectoryEnabledColumnIndex());
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWDirectoryPresetSelector::SetMaximumDirectoryLength(int arg)
{
  if (this->MaximumDirectoryLength == arg)
    {
    return;
    }

  this->MaximumDirectoryLength = arg;
  this->Modified();
} 

//----------------------------------------------------------------------------
int vtkKWDirectoryPresetSelector::UpdatePresetRow(int id)
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

  list->SetCellTextAsInt(
    row, this->GetDirectoryEnabledColumnIndex(), 
    this->GetPresetDirectoryEnabled(id));
  list->SetCellWindowCommandToCheckButton(
    row, this->GetDirectoryEnabledColumnIndex());

  if (this->MaximumDirectoryLength > 0)
    {
    vtksys_stl::string cropped(
      vtksys::SystemTools::CropString(
        this->GetPresetDirectory(id), this->MaximumDirectoryLength));
    list->SetCellText(
      row, this->GetDirectoryColumnIndex(), cropped.c_str());
    }
  else
    {
    list->SetCellText(
      row, this->GetDirectoryColumnIndex(), this->GetPresetDirectory(id));
    }
  
  return 1;
}

//---------------------------------------------------------------------------
int vtkKWDirectoryPresetSelector::AddDirectoryCallback()
{
  if (!this->FileBrowserDialog->IsCreated())
    {
    this->FileBrowserDialog->Create();
    }
  this->FileBrowserDialog->MultipleSelectionOn();
  this->FileBrowserDialog->SetFileName(NULL);
  this->FileBrowserDialog->Invoke();

  vtksys_stl::list<vtksys_stl::string> paths;
  
  int i;
  for (i = 0; i < this->FileBrowserDialog->GetNumberOfFileNames(); i++)
    {
    const char *filename = this->FileBrowserDialog->GetNthFileName(i);
    if (filename)
      {
      paths.push_back(filename);
      }
    }

  paths.sort();

  vtksys_stl::list<vtksys_stl::string>::reverse_iterator it = paths.rbegin();
  vtksys_stl::list<vtksys_stl::string>::reverse_iterator end = paths.rend();

  int id = -1;
  for (; it != end; it++)
    {
    if (!this->UniqueDirectories || 
        !this->HasPresetWithDirectory((*it).c_str()))
      {
      id = this->InsertPreset(-1);
      if (id >= 0)
        {
        this->SetPresetDirectory(id, (*it).c_str());
        this->SetPresetDirectoryEnabled(id, 1);
        }
      }
    }

  this->SelectPreset(id);

  return id;
}

//---------------------------------------------------------------------------
const char* vtkKWDirectoryPresetSelector::PresetCellEditStartCallback(
  int row, int col, const char *text)
{
  if (col == this->GetDirectoryColumnIndex())
    {
    int id = this->GetIdOfPresetAtRow(row);
    vtksys_stl::string cell_contents(this->GetPresetDirectory(id));
    vtkKWMultiColumnList *list = this->PresetList->GetWidget();
    list->CancelEditing();
    if (!this->FileBrowserDialog->IsCreated())
      {
      this->FileBrowserDialog->Create();
      }
    this->FileBrowserDialog->MultipleSelectionOff();
    this->FileBrowserDialog->SetFileName(cell_contents.c_str());
    this->FileBrowserDialog->SetLastPath(cell_contents.c_str());
    this->FileBrowserDialog->Invoke();
    const char *filename = this->FileBrowserDialog->GetFileName();
    if (filename)
      {
      vtksys_stl::string validated_contents(
        list->EditEndCallback(NULL, row, col, filename));
      if (strcmp(validated_contents.c_str(), cell_contents.c_str()))
        {
        this->SetPresetDirectory(id, validated_contents.c_str());
        this->UpdatePresetRow(id);
        list->CellUpdatedCallback();
        }
      }
    return NULL;
    }
  
  return text;
}

//---------------------------------------------------------------------------
const char* vtkKWDirectoryPresetSelector::PresetCellEditEndCallback(
  int, int, const char *text)
{
  this->PresetList->GetWidget()->SeeColumn(
    this->GetDirectoryEnabledColumnIndex());

  return text;
}

//---------------------------------------------------------------------------
void vtkKWDirectoryPresetSelector::PresetCellUpdatedCallback(
  int row, int col, const char *text)
{
  int id = this->GetIdOfPresetAtRow(row);
  if (this->HasPreset(id))
    {
    if (col == this->GetDirectoryEnabledColumnIndex())
      {
      this->SetPresetDirectoryEnabled(id, atoi(text));
      this->InvokePresetHasChangedCommand(id);
      return;
      }
    else if (col == this->GetDirectoryColumnIndex())
      {
      // not use the text, it might be cropped
      this->InvokePresetHasChangedCommand(id);
      return;
      }
    }

  this->Superclass::PresetCellUpdatedCallback(row, col, text);
}

//----------------------------------------------------------------------------
int vtkKWDirectoryPresetSelector::AddEnabledPresetDirectoriesFromDelimitedString(
  const char *from_str, const char from_delim)
{
  if (!from_str || !*from_str)
    {
    return 0;
    }

  int count = 0;

  vtksys_stl::vector<vtksys_stl::string> from_paths;
  vtksys::SystemTools::Split(from_str, from_paths, from_delim);
  
  vtksys_stl::vector<vtksys_stl::string>::iterator from_it = from_paths.begin();
  vtksys_stl::vector<vtksys_stl::string>::iterator from_end = from_paths.end();
  for (; from_it != from_end; from_it++)
    {
    const char *dir = (*from_it).c_str();
    if (*dir && (!this->UniqueDirectories || 
                 !this->HasPresetWithDirectory(dir)))
      {
      int id = this->AddPreset();
      if (id >= 0)
        {
        this->SetPresetDirectory(id, dir);
        this->SetPresetDirectoryEnabled(id, 1);
        ++count;
        }
      }
    }
  
  return count;
}

//----------------------------------------------------------------------------
int vtkKWDirectoryPresetSelector::GetEnabledPresetDirectoriesToDelimitedString(
  char **to_str, const char to_delim)
{
  if (!to_str)
    {
    return 0;
    }

  int count = 0;
  vtksys_ios::ostringstream to_stream;

  int i, nb_presets = this->GetNumberOfPresets();
  for (i = 0; i < nb_presets; i++)
    {
    int id = this->GetIdOfNthPreset(i);
    if (id >= 0 && this->GetPresetDirectoryEnabled(id))
      {
      const char *dir = this->GetPresetDirectory(id);
      if (dir && *dir)
        {
        ++count;
        if (to_stream.str().size())
          {
          to_stream << to_delim;
          }
        to_stream << dir;
        }
      }
    }

  if (count)
    {
    *to_str = new char[to_stream.str().size() + 1];
    strcpy(*to_str, to_stream.str().c_str());
    }

  return count;
}

//----------------------------------------------------------------------------
int vtkKWDirectoryPresetSelector::AddPresetDirectoriesFromDelimitedString(
  const char *from_str, const char from_delim)
{
  if (!from_str || !*from_str)
    {
    return 0;
    }

  int count = 0;

  vtksys_stl::vector<vtksys_stl::string> from_paths;
  vtksys::SystemTools::Split(from_str, from_paths, from_delim);
  
  vtksys_stl::vector<vtksys_stl::string>::iterator from_it = from_paths.begin();
  vtksys_stl::vector<vtksys_stl::string>::iterator from_end = from_paths.end();
  for (; from_it != from_end; from_it++)
    {
    vtksys_stl::string dir((*from_it));
    ++from_it;
    if (from_it == from_end)
      {
      break;
      }
    int enabled = atoi((*from_it).c_str());
    if (dir.size() && (!this->UniqueDirectories || 
                       !this->HasPresetWithDirectory(dir.c_str())))
      {
      int id = this->AddPreset();
      if (id >= 0)
        {
        this->SetPresetDirectory(id, dir.c_str());
        this->SetPresetDirectoryEnabled(id, enabled);
        ++count;
        }
      }
    }
  
  return count;
}

//----------------------------------------------------------------------------
int vtkKWDirectoryPresetSelector::GetPresetDirectoriesToDelimitedString(
  char **to_str, const char to_delim)
{
  if (!to_str)
    {
    return 0;
    }

  int count = 0;
  vtksys_ios::ostringstream to_stream;

  int i, nb_presets = this->GetNumberOfPresets();
  for (i = 0; i < nb_presets; i++)
    {
    int id = this->GetIdOfNthPreset(i);
    if (id >= 0)
      {
      const char *dir = this->GetPresetDirectory(id);
      if (dir && *dir)
        {
        ++count;
        if (to_stream.str().size())
          {
          to_stream << to_delim;
          }
        to_stream << dir << to_delim << this->GetPresetDirectoryEnabled(id);
        }
      }
    }

  if (count)
    {
    *to_str = new char[to_stream.str().size() + 1];
    strcpy(*to_str, to_stream.str().c_str());
    }

  return count;
}

//----------------------------------------------------------------------------
int vtkKWDirectoryPresetSelector::GetEnabledPresetDirectoriesFromPresetDirectories(
  char **to_str, const char to_delim, 
  const char *from_str, const char from_delim)
{
  if (!from_str || !*from_str || !to_str)
    {
    return 0;
    }

  int count = 0;
  vtksys_ios::ostringstream to_stream;

  vtksys_stl::vector<vtksys_stl::string> from_paths;
  vtksys::SystemTools::Split(from_str, from_paths, from_delim);
  
  vtksys_stl::vector<vtksys_stl::string>::iterator from_it = from_paths.begin();
  vtksys_stl::vector<vtksys_stl::string>::iterator from_end = from_paths.end();
  for (; from_it != from_end; from_it++)
    {
    vtksys_stl::string dir((*from_it));
    ++from_it;
    if (from_it == from_end)
      {
      break;
      }
    int enabled = atoi((*from_it).c_str());
    if (dir.size() && enabled)
      {
      ++count;
      if (to_stream.str().size())
        {
        to_stream << to_delim;
        }
      to_stream << dir;
      }
    }

  if (count)
    {
    *to_str = new char[to_stream.str().size() + 1];
    strcpy(*to_str, to_stream.str().c_str());
    }
  
  return count;
}

//----------------------------------------------------------------------------
int vtkKWDirectoryPresetSelector::UpdatePresetDirectoriesFromEnabledPresetDirectories(
  char **update_str, const char update_delim, 
  const char *from_str, const char from_delim)
{
  if (!from_str || !*from_str || !update_str)
    {
    return 0;
    }

  int count = 0;
  vtksys_ios::ostringstream update_stream;

  vtksys_stl::vector<vtksys_stl::string> update_paths;
  if (*update_str)
    {
    vtksys::SystemTools::Split(*update_str, update_paths, update_delim);
    }

  vtksys_stl::vector<vtksys_stl::string> from_paths;
  vtksys::SystemTools::Split(from_str, from_paths, from_delim);

  vtksys_stl::vector<vtksys_stl::string>::iterator update_it = 
    update_paths.begin();
  vtksys_stl::vector<vtksys_stl::string>::iterator update_end = 
    update_paths.end();
  for (; update_it != update_end; update_it++)
    {
    vtksys_stl::string dir((*update_it));
    ++update_it;
    if (update_it == update_end)
      {
      break;
      }
    int enabled = atoi((*update_it).c_str());
    vtksys_stl::vector<vtksys_stl::string>::iterator found = vtksys_stl::find(
      from_paths.begin(), from_paths.end(), dir);
    if (enabled)
      {
      // if we have an enabled path not found in the new list, skip it, it
      // should not be part of the updated list
      if (found == from_paths.end())
        {
        ++count;
        continue;
        }
      // otherwise it was found, therefore enabled in both lists, so remove it
      // from the new list, since we do not need to add it at the end
      from_paths.erase(found);
      }
    else
      {
      // if we have a disabled path found in the new list, enable it instead
      if (found != from_paths.end())
        {
        ++count;
        enabled = 1;
        }
      }
    if (update_stream.str().size())
      {
      update_stream << update_delim;
      }
    update_stream << dir << update_delim << update_delim;
    }

  // Now add the remaining (as enabled)

  vtksys_stl::vector<vtksys_stl::string>::iterator from_it = from_paths.begin();
  vtksys_stl::vector<vtksys_stl::string>::iterator from_end = from_paths.end();
  for (; from_it != from_end; from_it++)
    {
    if (update_stream.str().size())
      {
      update_stream << update_delim;
      }
    update_stream << (*from_it).c_str() << update_delim << (int)1;
    ++count;
    }

  if (count)
    {
    *update_str = new char[update_stream.str().size() + 1];
    strcpy(*update_str, update_stream.str().c_str());
    }
  
  return count;
}

//----------------------------------------------------------------------------
void vtkKWDirectoryPresetSelector::SetUniqueDirectories(int arg)
{
  if (this->UniqueDirectories == arg)
    {
    return;
    }

  this->UniqueDirectories = arg;
  this->Modified();

  this->Update();
}

//----------------------------------------------------------------------------
int vtkKWDirectoryPresetSelector::HasPresetWithDirectory(
  const char *directory)
{
  if (!directory || !*directory)
    {
    return 0;
    }

  int i, nb_presets = this->GetNumberOfPresets();
  for (i = 0; i < nb_presets; i++)
    {
    int id = this->GetIdOfNthPreset(i);
    const char *dir = this->GetPresetDirectory(id);
    if (dir && !strcmp(dir, directory))
      {
      return 1;
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWDirectoryPresetSelector::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "FileBrowserDialog: " << this->FileBrowserDialog << endl;

  os << indent << "MaximumDirectoryLength: " 
     << this->MaximumDirectoryLength << endl;
}
