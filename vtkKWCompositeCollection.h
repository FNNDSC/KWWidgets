/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWCompositeCollection.h,v $
  Language:  C++
  Date:      $Date: 2002-08-07 13:52:13 $
  Version:   $Revision: 1.8 $

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
// .NAME vtkKWCompositeCollection - a collection of composites
// .SECTION Description
// vtkKWCompositeCollection represents and provides methods to manipulate a 
// list of composites (i.e., vtkKWComposite and subclasses). The list is 
// unsorted and duplicate entries are not prevented.


#ifndef __vtkKWCompositeC_h
#define __vtkKWCompositeC_h

#include "vtkCollection.h"

class vtkKWComposite;

class VTK_EXPORT vtkKWCompositeCollection : public vtkCollection
{
public:
  static vtkKWCompositeCollection *New();
  vtkTypeRevisionMacro(vtkKWCompositeCollection,vtkCollection);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Add an KWComposite to the list.
  void AddItem(vtkKWComposite *a);

  // Description:
  // Remove an KWComposite from the list.
  void RemoveItem(vtkKWComposite *a);

  // Description:
  // Determine whether a particular KWComposite is present. 
  // Returns its position in the list.
  int IsItemPresent(vtkKWComposite *a);

  // Description:
  // Get the next KWComposite in the list.
  vtkKWComposite *GetNextKWComposite();

  // Description:
  // Get the last KWComposite in the list.
  vtkKWComposite *GetLastKWComposite();

protected:
  vtkKWCompositeCollection() {};
  ~vtkKWCompositeCollection() {};

private:
  vtkKWCompositeCollection(const vtkKWCompositeCollection&); // Not implemented
  void operator=(const vtkKWCompositeCollection&); // Not Implemented
};

#endif





