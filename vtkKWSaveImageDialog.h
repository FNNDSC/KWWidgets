/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWSaveImageDialog.h,v $
  Language:  C++
  Date:      $Date: 2000-01-18 19:59:35 $
  Version:   $Revision: 1.2 $

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
// .NAME vtkKWSaveImageDialog
// .SECTION Description
// A simple dialog for saving a 2D image as a BMP, TIFF, or PNM.

#ifndef __vtkKWSaveImageDialog_h
#define __vtkKWSaveImageDialog_h

#include "vtkKWWidget.h"
class vtkKWApplication;

class VTK_EXPORT vtkKWSaveImageDialog : public vtkKWWidget
{
public:
  static vtkKWSaveImageDialog* New();
  vtkTypeMacro(vtkKWSaveImageDialog,vtkKWWidget);

  // Description:
  // Create a Tk widget
  virtual void Create(vtkKWApplication *app, char *args);

  // Description::
  // Invoke the dialog 
  virtual void Invoke();

  // Description:
  // Set/Get the file name selected
  vtkGetStringMacro(FileName);
  vtkSetStringMacro(FileName);

protected:
  vtkKWSaveImageDialog();
  ~vtkKWSaveImageDialog() {};
  vtkKWSaveImageDialog(const vtkKWSaveImageDialog&) {};
  void operator=(const vtkKWSaveImageDialog&) {};

  char *FileName;
};


#endif


