/*=========================================================================

  Module:    $RCSfile: vtkKWTkTreeCtrlInit.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWTkTreeCtrlInit - class used to initialize TkTreeCtrl
// .SECTION Description
// This class is used to initialize the TkTreeCtrl library.

#ifndef __vtkKWTkTreeCtrlInit_h
#define __vtkKWTkTreeCtrlInit_h

#include "vtkObject.h"
#include "vtkKWWidgets.h" // Needed for export symbols directives
#include "vtkTcl.h" // Needed for Tcl interpreter

class KWWidgets_EXPORT vtkKWTkTreeCtrlInit : public vtkObject
{
public:
  static vtkKWTkTreeCtrlInit* New();
  vtkTypeRevisionMacro(vtkKWTkTreeCtrlInit,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Load the TkTreeCtrl library.
  static void Initialize(Tcl_Interp*);

protected:
  vtkKWTkTreeCtrlInit() {};
  ~vtkKWTkTreeCtrlInit() {};

  static int Initialized;

private:
  vtkKWTkTreeCtrlInit(const vtkKWTkTreeCtrlInit&);   // Not implemented.
  void operator=(const vtkKWTkTreeCtrlInit&);  // Not implemented.
};

#endif
