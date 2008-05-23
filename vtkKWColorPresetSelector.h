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
// .NAME vtkKWColorPresetSelector - a color preset selector.
// .SECTION Description
// This class is a widget that can be used to store color presets. 
// For example, a list of favorite colors, or a list of "recently picked"
// colors. 
// It is used internally by the vtkKWColorPickerWidget class.
// .SECTION Thanks
// This work is part of the National Alliance for Medical Image
// Computing (NAMIC), funded by the National Institutes of Health
// through the NIH Roadmap for Medical Research, Grant U54 EB005149.
// Information on the National Centers for Biomedical Computing
// can be obtained from http://nihroadmap.nih.gov/bioinformatics.
// .SECTION See Also
// vtkKWColorPickerWidget vtkKWColorSpectrumWidget vtkKWPresetSelector

#ifndef __vtkKWColorPresetSelector_h
#define __vtkKWColorPresetSelector_h

#include "vtkKWPresetSelector.h"

class KWWidgets_EXPORT vtkKWColorPresetSelector : public vtkKWPresetSelector
{
public:
  static vtkKWColorPresetSelector* New();
  vtkTypeRevisionMacro(vtkKWColorPresetSelector, vtkKWPresetSelector);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the color for a given preset.
  // A method is available to retrieve the Id of the preset that has
  // been assigned a specific color.
  // Return 1 on success, 0 on error (id on success, -1 otherwise)
  virtual void GetPresetColorAsRGB(int id, double &r, double &g, double &b);
  virtual void GetPresetColorAsRGB(int id, double rgb[3]);
  virtual int SetPresetColorAsRGB(int id, double r, double g, double b);
  virtual int SetPresetColorAsRGB(int id, double rgb[3]);
  virtual void GetPresetColorAsHSV(int id, double &h, double &s, double &v);
  virtual void GetPresetColorAsHSV(int id, double hsv[3]);
  virtual int SetPresetColorAsHSV(int id, double h, double s, double v);
  virtual int SetPresetColorAsHSV(int id, double hsv[3]);
  virtual int GetIdOfPresetWithColorAsRGB(double r, double g, double b);
  virtual int GetIdOfPresetWithColorAsRGB(double rgb[3]);

  // Description:
  // Set/Get the preset comment to the hexadecimal RGB representation
  // Return 1 on success, 0 otherwise
  virtual int SetPresetCommentAsHexadecimalRGB(int id);
  virtual int GetPresetCommentAsHexadecimalRGB(int id, int &r, int &g, int &b);
  virtual int HasPresetCommentAsHexadecimalRGB(int id);

  // Description:
  // Add  a few default color presets (black, white, primary colors).
  virtual void AddDefaultColorPresets();

  // Description:
  // Remove all duplicated colors from the list. 
  // Return the number of duplicates removed.
  virtual int RemoveDuplicatedColors();

  // Description:
  // Set/Get if the color cell (i.e. the cell representing the color
  // for each preset entry) is drawn as a color button (i.e. a square frame
  // with a background color set to the preset color, a black outline, and
  // some margin so that it doesn't touch the row separators) or if the whole
  // cell background color is used to represent the color. The former option
  // looks nicer, but is a tad slower to display: while it takes about the
  // same amount of time to insert a lot of entries (about 1200 entries per 
  // second), displaying them is a little slower when this option is On (about
  // 285 vs. 400 entries per second). 
  // Note: set this option before inserting any entries; for performance
  // reasons, switching from one to the other interactively is not supported.
  virtual void SetDrawColorCellAsColorButton(int);
  vtkGetMacro(DrawColorCellAsColorButton,int);
  vtkBooleanMacro(DrawColorCellAsColorButton,int);

  // Description:
  // Set/Get the registry key under which presets will be saved and/or 
  // restored automatically. Note that presets are saved automatically only
  // when performing *user-interactions* (add/remove/update/edit). 
  // For performance reason, adding/removing/changing a preset 
  // programatically will not save the presets to the registry; it is up
  // to the user to call SavePresetsToRegistry() manually.
  // IMPORTANT: make sure this variable is set before calling Create().
  // This goes as well for composite widgets that embed an instance of this
  // class. The vtkKWColorPickerWidget class, for example, uses two
  // color preset selectors: make sure to set their RegistryKey (if needed)
  // *before* calling the Create() method on the vtkKWColorPickerWidget
  // instance.
  vtkSetStringMacro(RegistryKey);
  vtkGetStringMacro(RegistryKey);

  // Description:
  // Save/Restore colors to/from registry, using the RegistryKey ivar as key.
  // Note that presets are saved automatically when performing 
  // *user-interactions* (see RegistryKey)
  virtual void SavePresetsToRegistry();
  virtual void RestorePresetsFromRegistry();

  // Description:
  // Some constants
  //BTX
  static const char *ColorColumnName;
  //ETX

protected:
  vtkKWColorPresetSelector();
  ~vtkKWColorPresetSelector();

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
  // Convenience methods to get the index of a given column
  virtual int GetColorColumnIndex();

  int DrawColorCellAsColorButton;
  char *RegistryKey;

  virtual int  InvokePresetAddCommand();
  virtual void InvokePresetRemovedCommand();
  virtual void InvokePresetUpdateCommand(int id);
  virtual void InvokePresetHasChangedCommand(int id);

  static int CompareRGBColors(double rgb1[3], double rgb2[3]);
  static int CompareRGBColors(double r1, double g1, double b1, 
                              double r2, double g2, double b2);

private:

  vtkKWColorPresetSelector(const vtkKWColorPresetSelector&); // Not implemented
  void operator=(const vtkKWColorPresetSelector&); // Not implemented
};

#endif
