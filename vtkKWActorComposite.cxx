/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWActorComposite.cxx,v $
  Language:  C++
  Date:      $Date: 2001-10-17 22:13:03 $
  Version:   $Revision: 1.4 $

Copyright (c) 2000-2001 Kitware Inc. 469 Clifton Corporate Parkway,
Clifton Park, NY, 12065, USA.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither the name of Kitware nor the names of any contributors may be used
   to endorse or promote products derived from this software without specific 
   prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "vtkKWActorComposite.h"
#include "vtkKWWidget.h"
#include "vtkKWView.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkKWActorComposite* vtkKWActorComposite::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkKWActorComposite");
  if(ret)
    {
    return (vtkKWActorComposite*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkKWActorComposite;
}




int vtkKWActorCompositeCommand(ClientData cd, Tcl_Interp *interp,
				int argc, char *argv[]);

vtkKWActorComposite::vtkKWActorComposite()
{
  this->CommandFunction = vtkKWActorCompositeCommand;

  this->Actor = vtkActor::New();
  this->Mapper = vtkPolyDataMapper::New();
  this->Actor->SetMapper(this->Mapper);
}

vtkKWActorComposite::~vtkKWActorComposite()
{
  if (this->Actor)
    {
    this->Actor->Delete();
    this->Actor = NULL;
    }
  
  if (this->Mapper)
    {
    this->Mapper->Delete();
    this->Mapper = NULL;
    }
}

void vtkKWActorComposite::SetInput(vtkPolyData *input)
{
  this->Mapper->SetInput(input); 
}

void vtkKWActorComposite::CreateProperties()
{
  // invoke superclass always
  this->vtkKWComposite::CreateProperties();
}



