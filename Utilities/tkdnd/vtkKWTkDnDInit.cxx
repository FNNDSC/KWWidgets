/*=========================================================================

  Module:    $RCSfile: vtkKWTkDnDInit.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWTkDnDInit.h"

#include "vtkObjectFactory.h"
#include "vtkKWTkUtilities.h"

#include "vtkTk.h"

#include "Utilities/tkdnd/vtkKWTkDnDTclLibrary.h"

extern "C" int Tkdnd_Init(Tcl_Interp *interp);

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWTkDnDInit );
vtkCxxRevisionMacro(vtkKWTkDnDInit, "$Revision: 1.3 $");

int vtkKWTkDnDInit::Initialized = 0;

//----------------------------------------------------------------------------
void vtkKWTkDnDInit::Initialize(Tcl_Interp* interp)
{
  if (vtkKWTkDnDInit::Initialized)
    {
    return;
    }

  if (!interp)
    {
    vtkGenericWarningMacro(
      "An interpreter is needed to initialize the TkDnD library.");
    return;
    }

  vtkKWTkDnDInit::Initialized = 1;

  // Evaluate the library
  

  vtkKWTkUtilities::EvaluateEncodedString(
    interp, 
    file_tkdnd_tcl, 
    file_tkdnd_tcl_length,
    file_tkdnd_tcl_decoded_length);

  Tkdnd_Init(interp);
}

//----------------------------------------------------------------------------
void vtkKWTkDnDInit::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


