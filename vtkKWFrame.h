/*=========================================================================

Copyright (c) 1998-2003 Kitware Inc. 469 Clifton Corporate Parkway,
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
// .NAME vtkKWFrame - a frame with a scroll bar
// .SECTION Description
// The ScrollableFrame creates a frame with an attached scrollbar


#ifndef __vtkKWFrame_h
#define __vtkKWFrame_h

#include "vtkKWWidget.h"
class vtkKWApplication;

class VTK_EXPORT vtkKWFrame : public vtkKWWidget
{
public:
  static vtkKWFrame* New();
  vtkTypeRevisionMacro(vtkKWFrame,vtkKWWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create a Tk widget
  virtual void Create(vtkKWApplication *app, const char* args);

  // Description:
  // Get the vtkKWWidget for the internal frame.
  vtkKWWidget *GetFrame() {return this->Frame;};

  // Description:
  // By default this is a simple frame. BY turning Scrollable on it becomes
  // a scrolled frame. This must be set prior to creation.
  vtkSetMacro(Scrollable,int);
  vtkGetMacro(Scrollable,int);
  vtkBooleanMacro(Scrollable,int);
  
protected:
  vtkKWFrame();
  ~vtkKWFrame();

  vtkKWWidget *Frame;
  vtkKWWidget *ScrollWindow;
  vtkKWWidget *ScrollFrame;

  char* FrameId;
  int Scrollable;
  
private:
  vtkKWFrame(const vtkKWFrame&); // Not implemented
  void operator=(const vtkKWFrame&); // Not implemented
};


#endif



