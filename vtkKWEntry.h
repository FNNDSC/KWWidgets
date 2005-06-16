/*=========================================================================

  Module:    $RCSfile: vtkKWEntry.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWEntry - a single line text entry widget
// .SECTION Description
// A simple widget used for collecting keyboard input from the user. This
// widget provides support for single line input.

#ifndef __vtkKWEntry_h
#define __vtkKWEntry_h

#include "vtkKWWidget.h"

class vtkKWApplication;

class KWWIDGETS_EXPORT vtkKWEntry : public vtkKWWidget
{
public:
  static vtkKWEntry* New();
  vtkTypeRevisionMacro(vtkKWEntry,vtkKWWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create the widget.
  virtual void Create(vtkKWApplication *app);

  // Description:
  // Set/Get the value of the entry in a few different formats.
  // In the SetValue method with double, values are printed in printf's f or e
  // format, whichever is more compact for the given value and precision. 
  // The e format is used only when the exponent of the value is less than
  // -4 or greater than or equal to the precision argument (which can be
  // controlled using the the second parameter of SetValue). Trailing zeros
  // are truncated, and the decimal point appears only if one or more digits
  // follow it.
  void SetValue(const char *);
  void SetValue(int a);
  void SetValue(double f, int size);
  void SetValue(double f);
  char *GetValue();
  int GetValueAsInt();
  double GetValueAsFloat();
  
  // Description:
  // The width is the number of charaters wide the entry box can fit.
  // To keep from changing behavior of the entry,  the default
  // value is -1 wich means the width is not explicitely set.
  void SetWidth(int width);
  vtkGetMacro(Width, int);

  // Description:
  // Set or get readonly flag. This flags makes entry read only.
  void SetReadOnly(int);
  vtkBooleanMacro(ReadOnly, int);
  vtkGetMacro(ReadOnly, int);

  // Description:
  // Bind the command called when <Return> is pressed or the widget gets out
  // of focus.
  virtual void BindCommand(vtkKWObject *object, const char *command);

  // Description:
  // Add and delete values to put in the list.
  void AddValue(const char* value);
  void DeleteValue(int idx);
  int GetValueIndex(const char* value);
  const char* GetValueFromIndex(int idx);
  int GetNumberOfValues();
  void DeleteAllValues();

  // Description:
  // Make this entry a pulldown combobox.
  vtkSetClampMacro(PullDown, int, 0, 1);
  vtkBooleanMacro(PullDown, int);
  vtkGetMacro(PullDown, int);

  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object is updated
  // and propagated to its internal parts/subwidgets. This will, for example,
  // enable/disable parts of the widget UI, enable/disable the visibility
  // of 3D widgets, etc.
  virtual void UpdateEnableState();

protected:
  vtkKWEntry();
  ~vtkKWEntry();
  
  vtkSetStringMacro(ValueString);
  vtkGetStringMacro(ValueString);
  
  char *ValueString;
  int Width;
  int ReadOnly;
  int PullDown;

  vtkKWWidget* Entry;

private:
  vtkKWEntry(const vtkKWEntry&); // Not implemented
  void operator=(const vtkKWEntry&); // Not Implemented
};


#endif



