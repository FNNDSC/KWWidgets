/*=========================================================================

  Module:    $RCSfile: vtkKWSimpleEntryDialog.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWSimpleEntryDialog - a message dialog with a single entry superclass

#ifndef __vtkKWSimpleEntryDialog_h
#define __vtkKWSimpleEntryDialog_h

#include "vtkKWMessageDialog.h"

class vtkKWEntryWithLabel;

class KWWidgets_EXPORT vtkKWSimpleEntryDialog : public vtkKWMessageDialog
{
public:
  static vtkKWSimpleEntryDialog* New();
  vtkTypeRevisionMacro(vtkKWSimpleEntryDialog, vtkKWMessageDialog);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Access to the entry
  vtkGetObjectMacro(Entry, vtkKWEntryWithLabel);

  // Description:
  // Invoke the dialog and display it in a modal manner. 
  // This method returns a zero if the dilaog was killed or 
  // canceled, nonzero otherwise.
  virtual int Invoke();

protected:
  vtkKWSimpleEntryDialog();
  ~vtkKWSimpleEntryDialog();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  vtkKWEntryWithLabel *Entry;

private:
  vtkKWSimpleEntryDialog(const vtkKWSimpleEntryDialog&); // Not implemented
  void operator=(const vtkKWSimpleEntryDialog&); // Not implemented
};


#endif



