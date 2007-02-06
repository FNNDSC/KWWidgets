/*=========================================================================

  Module:    $RCSfile: vtkKWTkconInit.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWTkconInit.h"

#include "vtkObjectFactory.h"
#include "vtkKWTkUtilities.h"

#include "vtkTk.h"

#include "Utilities/tkcon/vtkKWTkconTclLibrary.h"
 
//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWTkconInit );
vtkCxxRevisionMacro(vtkKWTkconInit, "$Revision: 1.3 $");

int vtkKWTkconInit::Initialized = 0;

//----------------------------------------------------------------------------
void vtkKWTkconInit::Initialize(Tcl_Interp* interp)
{
  if (vtkKWTkconInit::Initialized)
    {
    return;
    }

  if (!interp)
    {
    vtkGenericWarningMacro(
      "An interpreter is needed to initialize the tkcon library.");
    return;
    }

  vtkKWTkconInit::Initialized = 1;

  // Evaluate the library

  unsigned char *buffer = 
    new unsigned char [file_tkcon_tcl_length];

  unsigned int i;
  unsigned char *cur_pos = buffer;
  for (i = 0; i < file_tkcon_tcl_nb_sections; i++)
    {
    size_t len = strlen((const char*)file_tkcon_tcl_sections[i]);
    memcpy(cur_pos, file_tkcon_tcl_sections[i], len);
    cur_pos += len;
    }

  vtkKWTkUtilities::EvaluateEncodedString(
    interp, 
    buffer, 
    file_tkcon_tcl_length,
    file_tkcon_tcl_decoded_length);
  
  delete [] buffer;
}

//----------------------------------------------------------------------------
void vtkKWTkconInit::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


