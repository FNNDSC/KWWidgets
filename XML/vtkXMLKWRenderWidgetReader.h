/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkXMLKWRenderWidgetReader.h,v $
  Language:  C++
  Date:      $Date: 2003-03-28 22:48:22 $
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
// .NAME vtkXMLKWRenderWidgetReader - vtkKWRenderWidget XML Reader.
// .SECTION Description
// vtkXMLKWRenderWidgetReader provides XML reading functionality to 
// vtkKWRenderWidget.
// .SECTION See Also
// vtkXMLKWRenderWidgetWriter

#ifndef __vtkXMLKWRenderWidgetReader_h
#define __vtkXMLKWRenderWidgetReader_h

#include "vtkXMLObjectReader.h"

class VTK_EXPORT vtkXMLKWRenderWidgetReader : public vtkXMLObjectReader
{
public:
  static vtkXMLKWRenderWidgetReader* New();
  vtkTypeRevisionMacro(vtkXMLKWRenderWidgetReader, vtkXMLObjectReader);

  // Description:
  // Parse an XML tree.
  // Return 1 on success, 0 on error.
  virtual int Parse(vtkXMLDataElement*);

  // Description:
  // Return the name of the root element of the XML tree this reader
  // is supposed to read and process.
  virtual char* GetRootElementName();

protected:  
  vtkXMLKWRenderWidgetReader() {};
  ~vtkXMLKWRenderWidgetReader() {};

private:
  vtkXMLKWRenderWidgetReader(const vtkXMLKWRenderWidgetReader&); // Not implemented
  void operator=(const vtkXMLKWRenderWidgetReader&); // Not implemented    
};

#endif
