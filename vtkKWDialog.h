/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWDialog.h,v $
  Language:  C++
  Date:      $Date: 1999-12-29 23:22:09 $
  Version:   $Revision: 1.1 $

Copyright (c) 1998-1999 Kitware Inc. 469 Clifton Corporate Parkway,
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
// .NAME vtkKWDialog
// .SECTION Description
// A generic superclass for dialog boxes.

#ifndef __vtkKWDialog_h
#define __vtkKWDialog_h

#include "vtkKWWidget.h"
class vtkKWApplication;

class VTK_EXPORT vtkKWDialog : public vtkKWWidget
{
public:
  vtkKWDialog();
  ~vtkKWDialog();
  static vtkKWDialog* New();
  const char *GetClassName() {return "vtkKWDialog";};

  // Description:
  // Create a Tk widget
  void Create(vtkKWApplication *app, char *args);

  // Description:
  // Invoke the dialog and display it in a modal manner. 
  // This method returns a zero if the dilaog was killed or 
  // canceled, nonzero otherwise.
  virtual int Invoke();

  // Description:
  // Display the dialog in a non-modal manner.
  virtual void Display();

  // Description::
  // Close this Dialog
  virtual void Cancel();

  // Description::
  // Close this Dialog
  virtual void OK();

  // Description:
  // Returns 0 if the dialog is active e.g. displayed
  // 1 if it was Canceled 2 if it was OK.
  int GetStatus() {return this->Done;};

//BTX
  // Description:
  // A convienience method to invoke some tcl script code and
  // perform arguement substitution.
  virtual void SetCommand(char *EventString, ...);
//ETX

protected:
  char *Command;
  int Done;
};


#endif


