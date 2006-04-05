/*=========================================================================

  Module:    $RCSfile: vtkKWListBox.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWListBox - List Box
// .SECTION Description
// A widget that can have a list of items. 
// Use vtkKWListBoxWithScrollbars if you need scrollbars.
// .SECTION See Also
// vtkKWListBoxWithScrollbars

#ifndef __vtkKWListBox_h
#define __vtkKWListBox_h

#include "vtkKWCoreWidget.h"

class KWWidgets_EXPORT vtkKWListBox : public vtkKWCoreWidget
{
public:
  static vtkKWListBox* New();
  vtkTypeRevisionMacro(vtkKWListBox,vtkKWCoreWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create the widget.
  virtual void Create();

  // Description:
  // Set/Get the background color of the widget.
  virtual void GetBackgroundColor(double *r, double *g, double *b);
  virtual double* GetBackgroundColor();
  virtual void SetBackgroundColor(double r, double g, double b);
  virtual void SetBackgroundColor(double rgb[3])
    { this->SetBackgroundColor(rgb[0], rgb[1], rgb[2]); };
  
  // Description:
  // Set/Get the foreground color of the widget.
  virtual void GetForegroundColor(double *r, double *g, double *b);
  virtual double* GetForegroundColor();
  virtual void SetForegroundColor(double r, double g, double b);
  virtual void SetForegroundColor(double rgb[3])
    { this->SetForegroundColor(rgb[0], rgb[1], rgb[2]); };

  // Description:
  // Set/Get the foreground color of the widget when it is disabled.
  virtual void GetDisabledForegroundColor(double *r, double *g, double *b);
  virtual double* GetDisabledForegroundColor();
  virtual void SetDisabledForegroundColor(double r, double g, double b);
  virtual void SetDisabledForegroundColor(double rgb[3])
    { this->SetDisabledForegroundColor(rgb[0], rgb[1], rgb[2]); };

  // Description:
  // Set/Get the highlight thickness, a non-negative value indicating the
  // width of the highlight rectangle to draw around the outside of the
  // widget when it has the input focus.
  virtual void SetHighlightThickness(int);
  virtual int GetHighlightThickness();
  
  // Description:
  // Set/Get the border width, a non-negative value indicating the width of
  // the 3-D border to draw around the outside of the widget (if such a border
  // is being drawn; the Relief option typically determines this).
  virtual void SetBorderWidth(int);
  virtual int GetBorderWidth();
  
  // Description:
  // Set/Get the 3-D effect desired for the widget. 
  // The value indicates how the interior of the widget should appear
  // relative to its exterior. 
  // Valid constants can be found in vtkKWTkOptions::ReliefType.
  virtual void SetRelief(int);
  virtual int GetRelief();
  virtual void SetReliefToRaised();
  virtual void SetReliefToSunken();
  virtual void SetReliefToFlat();
  virtual void SetReliefToRidge();
  virtual void SetReliefToSolid();
  virtual void SetReliefToGroove();

  // Description:
  // Set/Get the one of several styles for manipulating the selection. 
  // Valid constants can be found in vtkKWTkOptions::SelectionModeType.
  virtual void SetSelectionMode(int);
  virtual int GetSelectionMode();
  virtual void SetSelectionModeToSingle(); 
  virtual void SetSelectionModeToBrowse(); 
  virtual void SetSelectionModeToMultiple();
  virtual void SetSelectionModeToExtended();

  // Description:
  // Specifies whether or not a selection in the widget should also be the X
  // selection. If the selection is exported, then selecting in the widget
  // deselects the current X selection, selecting outside the widget deselects
  // any widget selection, and the widget will respond to selection retrieval
  // requests when it has a selection.  
  virtual void SetExportSelection(int);
  virtual int GetExportSelection();
  vtkBooleanMacro(ExportSelection, int);
  
  // Description:
  // Get the current selected string in the list.  This is used when
  // Select mode is single or browse.
  virtual const char *GetSelection();
  virtual int GetSelectionIndex();
  virtual void SetSelectionIndex(int);

  // Description:
  // When selectmode is multiple or extended, then these methods can
  // be used to set and query the selection.
  virtual void SetSelectState(int idx, int state);
  virtual int GetSelectState(int idx);
  
  // Description:
  // Add an entry.
  virtual void InsertEntry(int index, const char *name);

  // Description:
  // Append a unique string to the list. If the string exists,
  // it will not be appended
  virtual int AppendUnique(const char* name);

  // Description:
  // Append a string to the list. This call does not check if the string
  // is unique.
  virtual int Append(const char* name);
  
  // Description:
  // Set callback for single and double click on a list item.
  virtual void SetDoubleClickCommand(vtkObject *obj, const char *method);
  virtual void SetSingleClickCommand(vtkObject *obj, const char *method);
  
  // Description:
  // Get number of items in the list.
  virtual int GetNumberOfItems();
  
  // Description: 
  // Get the item at the given index.
  virtual const char* GetItem(int index);

  // Description:
  // Returns the index of the first given item.
  virtual int GetItemIndex(const char* item);
  
  // Description:
  // Delete a range of items in the list.
  virtual void DeleteRange(int start, int end);
  
  // Description:
  // Delete all items from the list.
  virtual void DeleteAll();
  
  // Description: 
  // Set the width of the list box. If the width is less than or equal to 0,
  // then the width is set to the size of the largest string.
  virtual void SetWidth(int);
  virtual int GetWidth();

  // Description: 
  // Set the height of the list box. If the height is less than or equal to 0,
  // then the height is set to the size of the number of items in the listbox.
  virtual void SetHeight(int);
  virtual int GetHeight();

  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object is updated
  // and propagated to its internal parts/subwidgets. This will, for example,
  // enable/disable parts of the widget UI, enable/disable the visibility
  // of 3D widgets, etc.
  virtual void UpdateEnableState();
 
protected:
  vtkKWListBox();
  ~vtkKWListBox();

  char* CurrentSelection;       // store last call of CurrentSelection
  char* Item;                   // store last call of GetItem
  
private:
  vtkKWListBox(const vtkKWListBox&); // Not implemented
  void operator=(const vtkKWListBox&); // Not implemented
};


#endif



