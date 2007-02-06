/*=========================================================================

  Module:    $RCSfile: vtkKWTablelistInit.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWTablelistInit.h"

#include "vtkObjectFactory.h"
#include "vtkKWTkUtilities.h"

#include "vtkTk.h"

#include "Utilities/Tablelist/vtkKWTablelistTclLibrary.h"
 
//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWTablelistInit );
vtkCxxRevisionMacro(vtkKWTablelistInit, "$Revision: 1.6 $");

int vtkKWTablelistInit::Initialized = 0;

//----------------------------------------------------------------------------
void vtkKWTablelistInit::Initialize(Tcl_Interp* interp)
{
  if (vtkKWTablelistInit::Initialized)
    {
    return;
    }

  if (!interp)
    {
    vtkGenericWarningMacro(
      "An interpreter is needed to initialize the Tablelist library.");
    return;
    }

  vtkKWTablelistInit::Initialized = 1;

  // Evaluate the library

  vtkKWTkUtilities::EvaluateEncodedString(interp, 
                          file_tablelistPublic_tcl, 
                          file_tablelistPublic_tcl_length,
                          file_tablelistPublic_tcl_decoded_length);

  vtkKWTkUtilities::EvaluateEncodedString(interp, 
                          file_tablelist_tcl, 
                          file_tablelist_tcl_length,
                          file_tablelist_tcl_decoded_length);

  vtkKWTkUtilities::EvaluateEncodedString(interp, 
                          file_mwutil_tcl, 
                          file_mwutil_tcl_length,
                          file_mwutil_tcl_decoded_length);

  vtkKWTkUtilities::EvaluateEncodedString(interp, 
                          file_tablelistBitmaps_tcl, 
                          file_tablelistBitmaps_tcl_length,
                          file_tablelistBitmaps_tcl_decoded_length);

  vtkKWTkUtilities::EvaluateEncodedString(interp, 
                          file_tablelistBind_tcl, 
                          file_tablelistBind_tcl_length,
                          file_tablelistBind_tcl_decoded_length);

  vtkKWTkUtilities::EvaluateEncodedString(interp, 
                          file_tablelistConfig_tcl, 
                          file_tablelistConfig_tcl_length,
                          file_tablelistConfig_tcl_decoded_length);

  vtkKWTkUtilities::EvaluateEncodedString(interp, 
                          file_tablelistEdit_tcl, 
                          file_tablelistEdit_tcl_length,
                          file_tablelistEdit_tcl_decoded_length);

  vtkKWTkUtilities::EvaluateEncodedString(interp, 
                          file_tablelistMove_tcl, 
                          file_tablelistMove_tcl_length,
                          file_tablelistMove_tcl_decoded_length);

  vtkKWTkUtilities::EvaluateEncodedString(interp, 
                          file_tablelistSort_tcl, 
                          file_tablelistSort_tcl_length,
                          file_tablelistSort_tcl_decoded_length);

  vtkKWTkUtilities::EvaluateEncodedString(interp, 
                          file_tablelistThemes_tcl, 
                          file_tablelistThemes_tcl_length,
                          file_tablelistThemes_tcl_decoded_length);

  vtkKWTkUtilities::EvaluateEncodedString(interp, 
                          file_tablelistUtil_tcl, 
                          file_tablelistUtil_tcl_length,
                          file_tablelistUtil_tcl_decoded_length);

  vtkKWTkUtilities::EvaluateEncodedString(interp, 
                          file_tablelistUtil2_tcl, 
                          file_tablelistUtil2_tcl_length,
                          file_tablelistUtil2_tcl_decoded_length);

  vtkKWTkUtilities::EvaluateEncodedString(interp, 
                          file_tablelistWidget_tcl, 
                          file_tablelistWidget_tcl_length,
                          file_tablelistWidget_tcl_decoded_length);
}

//----------------------------------------------------------------------------
void vtkKWTablelistInit::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


