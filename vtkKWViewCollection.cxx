/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWViewCollection.cxx,v $
  Language:  C++
  Date:      $Date: 2002-08-07 13:52:15 $
  Version:   $Revision: 1.5 $

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

#include "vtkKWViewCollection.h"
#include "vtkObjectFactory.h"
#include "vtkKWView.h"

vtkStandardNewMacro( vtkKWViewCollection );
vtkCxxRevisionMacro(vtkKWViewCollection, "$Revision: 1.5 $");

void vtkKWViewCollection::AddItem(vtkKWView *a) 
{
  this->vtkCollection::AddItem((vtkObject *)a);
}

void vtkKWViewCollection::RemoveItem(vtkKWView *a) 
{
  this->vtkCollection::RemoveItem((vtkObject *)a);
}

int vtkKWViewCollection::IsItemPresent(vtkKWView *a) 
{
  return this->vtkCollection::IsItemPresent((vtkObject *)a);
}

vtkKWView *vtkKWViewCollection::GetNextKWView() 
{ 
  return vtkKWView::SafeDownCast(this->GetNextItemAsObject());
}

vtkKWView *vtkKWViewCollection::GetLastKWView() 
{ 
  if ( this->Bottom == NULL )
    {
    return NULL;
    }
  else
    {
    return vtkKWView::SafeDownCast(this->Bottom->Item);
    }
}

//----------------------------------------------------------------------------
void vtkKWViewCollection::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
