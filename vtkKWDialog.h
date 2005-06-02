/*=========================================================================

  Module:    $RCSfile: vtkKWDialog.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWDialog - dialog box superclass
// .SECTION Description
// A generic superclass for dialog boxes.
// This is a toplevel that is modal by default, and centered in its
// master window (or on screen)

#ifndef __vtkKWDialog_h
#define __vtkKWDialog_h

#include "vtkKWTopLevel.h"

class KWWIDGETS_EXPORT vtkKWDialog : public vtkKWTopLevel
{
public:
  static vtkKWDialog* New();
  vtkTypeRevisionMacro(vtkKWDialog,vtkKWTopLevel);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create the widget
  virtual void Create(vtkKWApplication *app, const char *args);

  // Description:
  // Invoke the dialog, display it and enter an event loop until the user
  // confirm (OK) or cancel the dialog.
  // Note that a dialog is a modal toplevel by default.
  // This method returns a zero if the dilaog was killed or 
  // canceled, nonzero otherwise.
  virtual int Invoke();

  // Description:
  // Display the dialog. 
  // Note that a dialog is a modal toplevel by default.
  virtual void Display();

  // Description:
  // Close this Dialog
  virtual void Cancel();

  // Description:
  // Close this Dialog
  virtual void OK();

  // Description:
  // Returns 0 if the dialog is active e.g. displayed
  // 1 if it was Canceled 2 if it was OK.
  int GetStatus() { return this->Done; };

  // Description:
  // Return frame to pack into.
  vtkKWWidget* GetFrame() { return this; }

  // Description:
  // Play beep when the dialog is displayed
  vtkSetClampMacro(Beep, int, 0, 1);
  vtkBooleanMacro(Beep, int);
  vtkGetMacro(Beep, int);

  // Description:
  // Sets the beep type
  vtkSetMacro(BeepType, int);
  vtkGetMacro(BeepType, int);

protected:

  vtkKWDialog();
  ~vtkKWDialog() {};

  int Done;
  int Beep;
  int BeepType;

private:
  vtkKWDialog(const vtkKWDialog&); // Not implemented
  void operator=(const vtkKWDialog&); // Not Implemented
};

#endif
