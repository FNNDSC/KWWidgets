/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWImageLabel.h,v $
  Language:  C++
  Date:      $Date: 2002-08-16 15:40:28 $
  Version:   $Revision: 1.7 $

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
// .NAME vtkKWImageLabel - image label widget
// .SECTION Description
// A simple subclass of the label widget which holds image. The image
// can have transparency. It has to be defined as vtkKWIcon or
// as array of unsigned char values, four per pixel (RGBA).

#ifndef __vtkKWImageLabel_h
#define __vtkKWImageLabel_h

#include "vtkKWLabel.h"
class vtkKWApplication;
class vtkKWIcon;

class VTK_EXPORT vtkKWImageLabel : public vtkKWLabel
{
public:
  static vtkKWImageLabel* New();
  vtkTypeRevisionMacro(vtkKWImageLabel,vtkKWLabel);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create a Tk widget
  virtual void Create(vtkKWApplication *app, const char *args);

  // Description:
  // Set image data (either using icon, or pixel data).
  void SetImageData(vtkKWIcon *icon);
  void SetImageData(const unsigned char* data, int width, int height, int pixel_size = 4);
  
  // Description:
  // Get the TK name of the image.
  vtkGetStringMacro(ImageDataName);

protected:
  vtkKWImageLabel();
  ~vtkKWImageLabel();

  // Description:
  // Set the TK name of the image.
  vtkSetStringMacro(ImageDataName);

private:
  char *ImageDataName;

  vtkKWImageLabel(const vtkKWImageLabel&); // Not implemented
  void operator=(const vtkKWImageLabel&); // Not implemented
};


#endif


