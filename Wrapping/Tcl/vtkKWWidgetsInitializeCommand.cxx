/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWWidgetsInitializeCommand.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include <stdlib.h>

#include "vtkTclUtil.h"
#include "vtkKWApplication.h"

extern "C" {int VTK_TK_EXPORT Vtkkwwidgetsinitializecommand_Init(Tcl_Interp *interp);}

int VTK_TK_EXPORT Vtkkwwidgetsinitializecommand_Init(Tcl_Interp *interp)
{
  if(Tcl_PkgPresent(interp, (char *)"Tcl", (char *)TCL_VERSION, 0))
    {
    vtkKWApplication::InitializeTcl(interp);
    }
  
  return TCL_OK;
}
