/*=========================================================================

  Module:    $RCSfile: vtkKWTkDnDInit.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWTkDnDInit - class used to initialize TkDnD
// .SECTION Description
// This class is used to initialize the TkDnD library.

#ifndef __vtkKWTkDnDInit_h
#define __vtkKWTkDnDInit_h

#include "vtkObject.h"
#include "vtkKWWidgets.h" // Needed for export symbols directives
#include "vtkTcl.h" // Needed for Tcl interpreter

class KWWidgets_EXPORT vtkKWTkDnDInit : public vtkObject
{
public:
  static vtkKWTkDnDInit* New();
  vtkTypeRevisionMacro(vtkKWTkDnDInit,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Load the TkDnD library.
  static void Initialize(Tcl_Interp*);

protected:
  vtkKWTkDnDInit() {};
  ~vtkKWTkDnDInit() {};

  static int Initialized;

private:
  vtkKWTkDnDInit(const vtkKWTkDnDInit&);   // Not implemented.
  void operator=(const vtkKWTkDnDInit&);  // Not implemented.
};

#endif
