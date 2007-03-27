/*=========================================================================

  Module:    $RCSfile: vtkKWLoadSaveDialog.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWLoadSaveDialog - dalog for loading or saving files
// .SECTION Description
// A dialog for loading or saving files. The file is returned through 
// GetFile method.

#ifndef __vtkKWLoadSaveDialog_h
#define __vtkKWLoadSaveDialog_h

#include "vtkKWFileBrowserDialog.h"

class vtkStringArray;

class KWWidgets_EXPORT vtkKWLoadSaveDialog : public vtkKWFileBrowserDialog
{
public:
  static vtkKWLoadSaveDialog* New();
  vtkTypeRevisionMacro(vtkKWLoadSaveDialog,vtkKWFileBrowserDialog);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkKWLoadSaveDialog() {};
  ~vtkKWLoadSaveDialog() {};

private:
  vtkKWLoadSaveDialog(const vtkKWLoadSaveDialog&); // Not implemented
  void operator=(const vtkKWLoadSaveDialog&); // Not implemented
};


#endif



