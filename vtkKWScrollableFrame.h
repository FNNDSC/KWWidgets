/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWScrollableFrame.h,v $
  Language:  C++
  Date:      $Date: 2002-03-20 19:14:08 $
  Version:   $Revision: 1.1 $

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
// .NAME vtkKWScrollableFrame - a frame with a scroll bar
// .SECTION Description
// The ScrollableFrame creates a frame with an attached scrollbar


#ifndef __vtkKWScrollableFrame_h
#define __vtkKWScrollableFrame_h

#include "vtkKWWidget.h"
class vtkKWApplication;

class VTK_EXPORT vtkKWScrollableFrame : public vtkKWWidget
{
public:
  static vtkKWScrollableFrame* New();
  vtkTypeMacro(vtkKWScrollableFrame,vtkKWWidget);

  // Description:
  // Create a Tk widget
  void Create(vtkKWApplication *app);

  // Description:
  // Get the vtkKWWidget for the internal frame.
  vtkKWWidget *GetFrame() {return this->Frame;};

  // Description:
  // These methods are internal. They are public because
  // they have to be wrapped.
  void ResizeFrame();
  void ResizeCanvas();

protected:
  vtkKWScrollableFrame();
  ~vtkKWScrollableFrame();

  void CalculateBBox(vtkKWWidget* canvas, char* name, int bbox[4]);

  vtkKWWidget *Frame;
  vtkKWWidget *Canvas;
  vtkKWWidget *ScrollBar;

  char* FrameId;

private:
  vtkKWScrollableFrame(const vtkKWScrollableFrame&); // Not implemented
  void operator=(const vtkKWScrollableFrame&); // Not implemented
};


#endif


