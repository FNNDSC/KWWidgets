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

#include "Utilities/TkDnD/vtkKWTkDnDTclLibrary.h"

#if defined(_WIN32) || defined(UNIX)
extern "C" int Tkdnd_Init(Tcl_Interp *interp);
#endif

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWTkDnDInit );
vtkCxxRevisionMacro(vtkKWTkDnDInit, "$Revision: 1.1 $");

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

#if defined(_WIN32) || defined(UNIX)

  vtkKWTkDnDInit::Initialized = 1;

  // Evaluate the library
  

  vtkKWTkUtilities::EvaluateEncodedString(
    interp, 
    file_tkdnd_tcl, 
    file_tkdnd_tcl_length,
    file_tkdnd_tcl_decoded_length);

  vtkKWTkUtilities::EvaluateSimpleString(
    interp, "tkdnd::initialise \"\"\n");

#if defined(_WIN32)
  vtkKWTkUtilities::EvaluateEncodedString(
    interp, 
    file_tkdnd_windows_tcl, 
    file_tkdnd_windows_tcl_length,
    file_tkdnd_windows_tcl_decoded_length);
#endif

#if defined(UNIX)
  vtkKWTkUtilities::EvaluateEncodedString(
    interp, 
    file_tkdnd_unix_tcl, 
    file_tkdnd_unix_tcl_length,
    file_tkdnd_unix_tcl_decoded_length);
#endif

  Tkdnd_Init(interp);

  vtkKWTkUtilities::EvaluateEncodedString(
    interp, 
    file_tkdnd_compat_tcl, 
    file_tkdnd_compat_tcl_length,
    file_tkdnd_compat_tcl_decoded_length);

#endif
}

//----------------------------------------------------------------------------
void vtkKWTkDnDInit::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


