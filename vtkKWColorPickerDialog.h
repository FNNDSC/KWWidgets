/*=========================================================================

  Module:    $RCSfile: vtkKWColorPickerDialog.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWColorPickerDialog - a color picker dialog.
// .SECTION Description
// A widget that can be used to pick a color.
// This dialog is just a small standalone toplevel that embeds a
// vtkKWColorPickerWidget.
// .SECTION Thanks
// This work is part of the National Alliance for Medical Image
// Computing (NAMIC), funded by the National Institutes of Health
// through the NIH Roadmap for Medical Research, Grant U54 EB005149.
// Information on the National Centers for Biomedical Computing
// can be obtained from http://nihroadmap.nih.gov/bioinformatics.
// .SECTION See Also
// vtkKWColorPickerWidget

#ifndef __vtkKWColorPickerDialog_h
#define __vtkKWColorPickerDialog_h

#include "vtkKWDialog.h"

class vtkKWApplication;
class vtkKWColorPickerWidget;
class vtkKWPushButton;

class KWWidgets_EXPORT vtkKWColorPickerDialog : public vtkKWDialog
{
public:
  static vtkKWColorPickerDialog* New();
  vtkTypeRevisionMacro(vtkKWColorPickerDialog,vtkKWDialog);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the internal color picker widget
  vtkGetObjectMacro(ColorPickerWidget, vtkKWColorPickerWidget);   
   
  // Description:
  // Callback. Confirm the action and close this dialog
  virtual void OK();

  // Description:
  // Update the "enable" state of the object and its internal parts.
  virtual void UpdateEnableState();

protected:
  vtkKWColorPickerDialog();
  ~vtkKWColorPickerDialog();

  // Description:
  // Create the widget.
  virtual void CreateWidget();
  
  // Description:
  // Member variables
  vtkKWColorPickerWidget* ColorPickerWidget;
  vtkKWPushButton* OKButton;
  vtkKWPushButton* CancelButton;

private:
  vtkKWColorPickerDialog(const vtkKWColorPickerDialog&); // Not implemented
  void operator=(const vtkKWColorPickerDialog&); // Not implemented
};

#endif
