/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWLabeledEntry.h,v $
  Language:  C++
  Date:      $Date: 2002-02-01 14:55:30 $
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
// .NAME vtkKWLabeledEntry - an entry with a label
// .SECTION Description
// The LabeledEntry creates an entry with a label in front of it; both are
// contained in a frame


#ifndef __vtkKWLabeledEntry_h
#define __vtkKWLabeledEntry_h

#include "vtkKWLabel.h"
#include "vtkKWEntry.h"

class vtkKWApplication;

class VTK_EXPORT vtkKWLabeledEntry : public vtkKWWidget
{
public:
  static vtkKWLabeledEntry* New();
  vtkTypeMacro(vtkKWLabeledEntry, vtkKWWidget);

  // Description:
  // Create a Tk widget
  void Create(vtkKWApplication *app);

  // Description:
  // Set the label for the frame.
  void SetLabel(const char *);
  
  // Description:
  // get the internal entry
  vtkKWEntry *GetEntry() { return this->Entry; }
  
  // Description:
  // Set/Get the value of the entry in a few different formats.
  // In the SetValue method with float, the second argument is the
  // number of decimal places to display.
  void SetValue(const char *);
  void SetValue(int a);
  void SetValue(float f,int size);
  char *GetValue();
  int GetValueAsInt();
  float GetValueAsFloat();

protected:
  vtkKWLabeledEntry();
  ~vtkKWLabeledEntry();

  vtkKWLabel *Label;
  vtkKWEntry *Entry;
private:
  vtkKWLabeledEntry(const vtkKWLabeledEntry&); // Not implemented
  void operator=(const vtkKWLabeledEntry&); // Not implemented
};


#endif
