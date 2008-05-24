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
// .NAME vtkKWDirectoryPresetSelector - a directory preset selector.
// .SECTION Description
// This class is a widget that can be used to pick and list of set of directories.
// presets. 
// .SECTION Thanks
// This work is part of the National Alliance for Medical Image
// Computing (NAMIC), funded by the National Institutes of Health
// through the NIH Roadmap for Medical Research, Grant U54 EB005149.
// Information on the National Centers for Biomedical Computing
// can be obtained from http://nihroadmap.nih.gov/bioinformatics.
// .SECTION See Also
// vtkKWPresetSelector

#ifndef __vtkKWDirectoryPresetSelector_h
#define __vtkKWDirectoryPresetSelector_h

#include "vtkKWPresetSelector.h"

class vtkKWFileBrowserDialog;

class KWWidgets_EXPORT vtkKWDirectoryPresetSelector : public vtkKWPresetSelector
{
public:
  static vtkKWDirectoryPresetSelector* New();
  vtkTypeRevisionMacro(vtkKWDirectoryPresetSelector, vtkKWPresetSelector);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the directory for a given preset.
  // Return 1 on success, 0 otherwise
  virtual int SetPresetDirectory(int id, const char *directory);
  virtual const char* GetPresetDirectory(int id);

  // Description:
  // Query if the pool has a given directory preset
  virtual int HasPresetWithDirectory(const char *directory);

  // Description:
  // Set/Get the directory enabled flag for a given preset.
  // Return 1 on success, 0 otherwise
  virtual int GetPresetDirectoryEnabled(int id);
  virtual int SetPresetDirectoryEnabled(int id, int flag);

  // Description:
  // Set/Get the visibility of the directory enabled flag column. 
  // Hidden by default.
  // No effect if called before Create().
  virtual void SetDirectoryEnabledColumnVisibility(int);
  virtual int GetDirectoryEnabledColumnVisibility();
  vtkBooleanMacro(DirectoryEnabledColumnVisibility, int);

  // Description:
  // Set/Get the maximum length of the directory paths, in characters.
  // If set to 0 (default), do not shorten the paths automatically.
  virtual void SetMaximumDirectoryLength(int);
  vtkGetMacro(MaximumDirectoryLength, int);

  // Description:
  // Set/Get if unique directories are enforced..
  virtual void SetUniqueDirectories(int);
  vtkGetMacro(UniqueDirectories,int);
  vtkBooleanMacro(UniqueDirectories,int);


  // Description:
  // Add/retrieve the whole list of enabled directories from/to a string. Each
  // directory in this list is separated by a given delimiter (ex: ':' or ';').
  // Presets are added in an "enabled" state (see GetPresetDirectoryEnabled)
  // to the end of the preset list.
  // Only presets that were enabled are retrieved and stored into a string
  // (note that said string will be allocated to the proper size with 'new' and
  // should therefore be de-allocated by the called with 'delete []').
  // Return the number of enabled directories added, or retrieved.
  virtual int AddEnabledPresetDirectoriesFromDelimitedString(
    const char *from_str, const char from_delim);
  virtual int GetEnabledPresetDirectoriesToDelimitedString(
    char **to_str, const char to_delim);

  // Description:
  // Add/retrieve the whole list of directories *and* their enabled flag
  // from/to a string representation.
  // Each item in this list is a directory and a boolean flag (0 or 1) 
  // specifying if that directory is actually enabled or not in the UI.
  // Each element is separated by a given delimiter (ex: with '|' as delim, 
  // "c:/temp|0|d:/foo/bar|1|c:/windows|1" refers to "c:/temp" as disabled and
  // both "d:/foo/bar" and "c:/windows" enabled; at this point, the string
  // returned by AddEnabledPresetDirectoriesFromDelimitedString would be
  // "d:/foo/bar;c:/windows" with a ';' delimiter).
  // Presets are added to the end of the preset list.
  // (note that said string will be allocated to the proper size with 'new' and
  // should therefore be de-allocated by the caller with 'delete []').
  // Return the number of directories added, or retrieved.
  virtual int AddPresetDirectoriesFromDelimitedString(
    const char *from_str, const char from_delim);
  virtual int GetPresetDirectoriesToDelimitedString(
    char **to_str, const char to_delim);

  // Description:
  // Static helper method.
  // This method extracts the list of enabled directories (to a string) from
  // the list of directories + flag (as a string, as it was returned by 
  // GetPresetDirectoriesToDelimitedString for example).
  // Each list can be separated by different delimiters (ex: with '|' as 
  // from_delim, "c:/temp|0|d:/foo/bar|1|c:/windows|1" as from_str will be
  // output to to_str as "d:/foo/bar;c:/windows" if to_delim is ';').
  // (note that said string will be allocated to the proper size with 'new' and
  // should therefore be de-allocated by the caller with 'delete []').
  // Return the number of enabled directories retrieved.
  static int GetEnabledPresetDirectoriesFromPresetDirectories(
    char **to_str, const char to_delim,
    const char *from_str, const char from_delim);

  // Description:
  // Static helper method.
  // This method *updates* the list of directories + flag (to a string) from
  // a list of enabled directories (as a string, as it was returned by 
  // Duplicated directories are not supported here.
  // Each list can be separated by different delimiters (ex: with ';' as 
  // from_delim, "c:/temp;d:/bill" as from_str, '|' as to_delim and 
  // "c:/temp|0|d:/foo/bar|0|c:/windows|1" as to_str, to_str will be updated
  // (i.e. potentially re-allocated) to a new string 
  // "c:/temp|1|d:/foo/bar|0|d:/bill|1", where the already 
  // existing "c:/temp" was updated/enabled, ""d:/foo/bar" was ignored, 
  // "c:/windows" was removed (since considered not enabled) and "d:/bill" was
  // added.
  // (note that said string will be allocated to the proper size with 'new' and
  // should therefore be de-allocated by the caller with 'delete []').
  // Return the number of directories updated (i.e. removed, updated or added).
  static int UpdatePresetDirectoriesFromEnabledPresetDirectories(
    char **update_str, const char update_delim,
    const char *from_str, const char from_delim);

  // Description:
  // Some constants
  //BTX
  static const char *DirectoryColumnName;
  static const char *DirectoryEnabledColumnName;
  //ETX

  // Description:
  // Callback invoked when the user starts editing a specific preset field
  // located at cell ('row', 'col'), which current contents is 'text'.
  // This method returns the value that is to become the initial 
  // contents of the temporary embedded widget used for editing: most of the
  // time, this is the same value as 'text'.
  // The next step (validation) is handled by PresetCellEditEndCallback
  virtual const char* PresetCellEditStartCallback(
    int row, int col, const char *text);

  // Description:
  // Callback invoked when the user ends editing a specific preset field
  // located at cell ('row', 'col').
  // The main purpose of this method is to perform a final validation of
  // the edit window's contents 'text'.
  // This method returns the value that is to become the new contents
  // for that cell.
  // The next step (updating) is handled by PresetCellUpdateCallback
  virtual const char* PresetCellEditEndCallback(
    int row, int col, const char *text);

  // Description:
  // Callback invoked when the user successfully updated the preset field
  // located at ('row', 'col') with the new contents 'text', as a result
  // of editing the corresponding cell interactively.
  virtual void PresetCellUpdatedCallback(int row, int col, const char *text);

  // Description:
  // Callback invoked when the user press the "Add" button
  virtual int AddDirectoryCallback();

  // Description:
  // Access to sub-widgets.
  vtkGetObjectMacro(FileBrowserDialog, vtkKWFileBrowserDialog);

protected:
  vtkKWDirectoryPresetSelector();
  ~vtkKWDirectoryPresetSelector();

  int MaximumDirectoryLength;
  int UniqueDirectories;

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Description:
  // Create the columns.
  // Subclasses should override this method to add their own columns and
  // display their own preset fields (do not forget to call the superclass
  // first).
  virtual void CreateColumns();

  // Description:
  // Update the preset row, i.e. add a row for that preset if it is not
  // displayed already, hide it if it does not match GroupFilter, and
  // update the table columns with the corresponding preset fields.
  // Subclass should override this method to display their own fields.
  // Return 1 on success, 0 if the row was not (or can not be) updated.
  // Subclasses should call the parent's UpdatePresetRow, and abort
  // if the result is not 1.
  virtual int UpdatePresetRow(int id);

  // Description:
  // Get the index of a given column
  virtual int GetDirectoryColumnIndex();
  virtual int GetDirectoryEnabledColumnIndex();

  vtkKWFileBrowserDialog *FileBrowserDialog;

private:

  vtkKWDirectoryPresetSelector(const vtkKWDirectoryPresetSelector&); // Not implemented
  void operator=(const vtkKWDirectoryPresetSelector&); // Not implemented
};

#endif
