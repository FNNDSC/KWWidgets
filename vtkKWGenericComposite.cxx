/*=========================================================================

  Module:    $RCSfile: vtkKWGenericComposite.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWGenericComposite.h"

#include "vtkObjectFactory.h"
#include "vtkProp.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWGenericComposite );
vtkCxxRevisionMacro(vtkKWGenericComposite, "$Revision: 1.11 $");



int vtkKWGenericCompositeCommand(ClientData cd, Tcl_Interp *interp,
                                int argc, char *argv[]);

vtkKWGenericComposite::vtkKWGenericComposite()
{
  this->CommandFunction = vtkKWGenericCompositeCommand;

  this->Prop = NULL;
}

vtkKWGenericComposite::~vtkKWGenericComposite()
{
  this->SetProp(NULL);
}

//----------------------------------------------------------------------------
#ifdef VTK_WORKAROUND_WINDOWS_MANGLE
# undef SetProp
// Define possible mangled names.
void vtkKWGenericComposite::SetPropA(vtkProp* prop)
{
  return this->SetProp(prop);
}
void vtkKWGenericComposite::SetPropW(vtkProp* prop)
{
  return this->SetProp(prop);
}
#endif
vtkSetObjectImplementationMacro(vtkKWGenericComposite, Prop, vtkProp);

//----------------------------------------------------------------------------
vtkProp* vtkKWGenericComposite::GetPropInternal()
{
  return this->Prop;
}

//----------------------------------------------------------------------------
void vtkKWGenericComposite::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

