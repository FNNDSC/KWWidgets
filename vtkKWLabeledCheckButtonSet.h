/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWLabeledCheckButtonSet.h,v $
  Language:  C++
  Date:      $Date: 2002-12-12 21:51:39 $
  Version:   $Revision: 1.2 $

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
// .NAME vtkKWLabeledCheckButtonSet - a checkbutton set with a label
// .SECTION Description
// This class creates a checkbutton set with a label on top of it; both are
// contained in a frame
// .SECTION See Also
// vtkKWCheckButtonSet

#ifndef __vtkKWLabeledCheckButtonSet_h
#define __vtkKWLabeledCheckButtonSet_h

#include "vtkKWLabeledWidget.h"

class vtkKWCheckButtonSet;

class VTK_EXPORT vtkKWLabeledCheckButtonSet : public vtkKWLabeledWidget
{
public:
  static vtkKWLabeledCheckButtonSet* New();
  vtkTypeRevisionMacro(vtkKWLabeledCheckButtonSet, vtkKWLabeledWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create the widget
  virtual void Create(vtkKWApplication *app, const char *args = 0);

  // Description:
  // Get the internal object
  vtkGetObjectMacro(CheckButtonSet, vtkKWCheckButtonSet);

  // Description:
  // Set the widget packing order to be horizontal (default is vertical).
  virtual void SetPackHorizontally(int);
  vtkBooleanMacro(PackHorizontally, int);
  vtkGetMacro(PackHorizontally, int);

  // Description:
  // Set/Get the enabled state.
  // Override to pass down to children.
  virtual void SetEnabled(int);

  // Description:
  // Set the string that enables balloon help for this widget.
  // Override to pass down to children.
  virtual void SetBalloonHelpString(const char *str);
  virtual void SetBalloonHelpJustification(int j);

protected:
  vtkKWLabeledCheckButtonSet();
  ~vtkKWLabeledCheckButtonSet();

  vtkKWCheckButtonSet *CheckButtonSet;

  int PackHorizontally;

  // Pack or repack the widget.

  virtual void Pack();

private:
  vtkKWLabeledCheckButtonSet(const vtkKWLabeledCheckButtonSet&); // Not implemented
  void operator=(const vtkKWLabeledCheckButtonSet&); // Not implemented
};


#endif
