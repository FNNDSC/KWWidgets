/*=========================================================================

  Module:    $RCSfile: vtkKWTkTreeCtrlInit.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWTkTreeCtrlInit.h"

#include "vtkObjectFactory.h"
#include "vtkKWTkUtilities.h"

#include "vtkTk.h"

#include "Utilities/TkTreeCtrl/vtkKWTkTreeCtrlTclLibrary.h"

extern "C" int Treectrl_Init(Tcl_Interp *interp);

#ifdef _WIN32
extern "C" int Shellicon_Init(Tcl_Interp *interp);
#endif
 
//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWTkTreeCtrlInit );
vtkCxxRevisionMacro(vtkKWTkTreeCtrlInit, "$Revision: 1.1 $");

int vtkKWTkTreeCtrlInit::Initialized = 0;

//----------------------------------------------------------------------------
void vtkKWTkTreeCtrlInit::Initialize(Tcl_Interp* interp)
{
  if (vtkKWTkTreeCtrlInit::Initialized)
    {
    return;
    }

  if (!interp)
    {
    vtkGenericWarningMacro(
      "An interpreter is needed to initialize the TkTreeCtrl library.");
    return;
    }

  vtkKWTkTreeCtrlInit::Initialized = 1;

  Treectrl_Init(interp);

#ifdef _WIN32
  Shellicon_Init(interp);
#endif

  // Evaluate the library
  
  vtkKWTkUtilities::EvaluateEncodedString(
    interp, 
    file_filelist_bindings_tcl, 
    file_filelist_bindings_tcl_length,
    file_filelist_bindings_tcl_decoded_length);

  vtkKWTkUtilities::EvaluateEncodedString(
    interp, 
    file_treectrl_tcl, 
    file_treectrl_tcl_length,
    file_treectrl_tcl_decoded_length);

  vtkKWTkUtilities::EvaluateSimpleString(
    interp, "set ::treectrl_library \"\"\n");
}

//----------------------------------------------------------------------------
void vtkKWTkTreeCtrlInit::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


