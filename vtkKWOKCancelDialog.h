/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWOKCancelDialog.h,v $
  Language:  C++
  Date:      $Date: 2000-09-15 20:37:14 $
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
// .NAME vtkKWOKCancelDialog - a dialog box with an OK and Cancel button
// .SECTION Description
// A class for OKCancelDialog boxes.

#ifndef __vtkKWOKCancelDialog_h
#define __vtkKWOKCancelDialog_h

#include "vtkKWDialog.h"
class vtkKWApplication;

class VTK_EXPORT vtkKWOKCancelDialog : public vtkKWDialog
{
public:
  static vtkKWOKCancelDialog* New();
  vtkTypeMacro(vtkKWOKCancelDialog,vtkKWDialog);

  // Description:
  // Create a Tk widget
  void Create(vtkKWApplication *app, const char *args);

  // Description:
  // Set the text of the message
  void SetText(const char *);
  
protected:
  vtkKWOKCancelDialog();
  ~vtkKWOKCancelDialog();
  vtkKWOKCancelDialog(const vtkKWOKCancelDialog&) {};
  void operator=(const vtkKWOKCancelDialog&) {};

  vtkKWWidget *Message;
  vtkKWWidget *ButtonFrame;
  vtkKWWidget *OKButton;
  vtkKWWidget *CancelButton;
};


#endif


