/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWImageLabel.cxx,v $
  Language:  C++
  Date:      $Date: 2002-02-06 22:01:35 $
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
#include "vtkKWApplication.h"
#include "vtkKWImageLabel.h"
#include "vtkObjectFactory.h"
#include "vtkKWIcon.h"


//------------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWImageLabel );

vtkKWImageLabel::vtkKWImageLabel()
{
  this->ImageDataLabel = 0;
}

vtkKWImageLabel::~vtkKWImageLabel()
{
  this->SetImageDataLabel(0);
}

void vtkKWImageLabel::Create(vtkKWApplication *app, const char *args)
{
  this->vtkKWLabel::Create(app, args);
}

void vtkKWImageLabel::SetImageData(vtkKWIcon* icon)
{
  this->SetImageData(icon->GetData(), icon->GetWidth(), icon->GetHeight());
}
/*
void vtkKWImageLabel::SetImageData(const unsigned char* data, 
				   int width, int height)
{
  int r, g, b;
  this->GetBackgroundColor(&r, &g, &b);
  this->Script("image create photo -height %d -width %d", width, height);
  this->SetImageDataLabel(this->Application->GetMainInterp()->result);
  Tk_PhotoHandle photo;
  Tk_PhotoImageBlock block, sblock;

  block.width = width;
  block.height = height;
  block.pixelSize = 4;
  block.pitch = block.width*block.pixelSize;
  block.offset[0] = 0;
  block.offset[1] = 1;
  block.offset[2] = 2;
  block.pixelPtr = new unsigned char [block.pitch*block.height];

  photo = Tk_FindPhoto(this->Application->GetMainInterp(),
		       this->ImageDataLabel);

  unsigned char *pp = block.pixelPtr;
  const unsigned char *dd = data;
  int cc;
  int xx, yy;
  for ( yy=0; yy < block.width; yy++ )
    {
    for ( xx=0; xx < block.width; xx++ )
      {
      float alpha = static_cast<float>(*(dd+3)) / 255.0;
      
      *(pp)   = static_cast<int>(r*(1-alpha) + *(dd) * alpha);
      *(pp+1) = static_cast<int>(g*(1-alpha) + *(dd+1) * alpha);
      *(pp+2) = static_cast<int>(b*(1-alpha) + *(dd+2) * alpha);
      *(pp+3) = *(dd+3);
      
      pp+=4;
      dd+=4;
      }
    }
  Tk_PhotoPutBlock(photo, &block, 0, 0, block.width, block.height);
  delete [] block.pixelPtr;
  this->Script("%s configure -image %s", this->GetWidgetName(),
	       this->ImageDataLabel);
}
*/

void vtkKWImageLabel::SetImageData(const unsigned char* data, 
				   int width, int height)
{
  int r, g, b;
  this->GetBackgroundColor(&r, &g, &b);
  this->Script("image create photo -height %d -width %d", width, height);
  this->SetImageDataLabel(this->Application->GetMainInterp()->result);
  Tk_PhotoHandle photo;
  Tk_PhotoImageBlock sblock;
  photo = Tk_FindPhoto(this->Application->GetMainInterp(),
		       this->ImageDataLabel);
  Tk_PhotoBlank(photo);
  const unsigned char *dd = data;
  int xx, yy;

  unsigned char array[4];
  sblock.pixelPtr  = array;
  sblock.width     = 1;
  sblock.height    = 1;
  sblock.pitch     = 0;
  sblock.offset[0] = 0;
  sblock.offset[1] = 1;
  sblock.offset[2] = 2;

  unsigned char *pp = sblock.pixelPtr;
  
  for ( yy=0; yy < height; yy++ )
    {
    for ( xx=0; xx < width; xx++ )
      {
      float alpha = static_cast<float>(*(dd+3)) / 255.0;
      
      *(pp)   = static_cast<int>(r*(1-alpha) + *(dd) * alpha);
      *(pp+1) = static_cast<int>(g*(1-alpha) + *(dd+1) * alpha);
      *(pp+2) = static_cast<int>(b*(1-alpha) + *(dd+2) * alpha);
      *(pp+3) = *(dd+3);
      Tk_PhotoPutBlock(photo, &sblock, xx, yy, 1, 1);
      dd+=4;
      }
    }
  this->Script("%s configure -image %s", this->GetWidgetName(),
	       this->ImageDataLabel);
}

