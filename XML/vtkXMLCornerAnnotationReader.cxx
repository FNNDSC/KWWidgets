/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkXMLCornerAnnotationReader.cxx,v $
  Language:  C++
  Date:      $Date: 2003-04-16 18:27:58 $
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
#include "vtkXMLCornerAnnotationReader.h"

#include "vtkCornerAnnotation.h"
#include "vtkObjectFactory.h"
#include "vtkTextProperty.h"
#include "vtkXMLDataElement.h"
#include "vtkXMLTextPropertyReader.h"
#include "vtkXMLCornerAnnotationWriter.h"

vtkStandardNewMacro(vtkXMLCornerAnnotationReader);
vtkCxxRevisionMacro(vtkXMLCornerAnnotationReader, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
char* vtkXMLCornerAnnotationReader::GetRootElementName()
{
  return "CornerAnnotation";
}

//----------------------------------------------------------------------------
int vtkXMLCornerAnnotationReader::Parse(vtkXMLDataElement *elem)
{
  if (!this->Superclass::Parse(elem))
    {
    return 0;
    }

  vtkCornerAnnotation *obj = vtkCornerAnnotation::SafeDownCast(this->Object);
  if (!obj)
    {
    vtkWarningMacro(<< "The CornerAnnotation is not set!");
    return 0;
    }

  // Get attributes

  float fval;
  int ival;
  const char *cptr;

  for (int i = 0; i < 4; i++)
    {
    ostrstream text_name;
    text_name << "Text" << i << ends;
    cptr = elem->GetAttribute(text_name.str());
    if (cptr)
      {
      obj->SetText(i, cptr);
      }
    text_name.rdbuf()->freeze(0);
    }

  if (elem->GetScalarAttribute("MaximumLineHeight", fval))
    {
    obj->SetMaximumLineHeight(fval);
    }

  if (elem->GetScalarAttribute("MinimumFontSize", ival))
    {
    obj->SetMinimumFontSize(ival);
    }

  if (elem->GetScalarAttribute("LevelShift", fval))
    {
    obj->SetLevelShift(fval);
    }

  if (elem->GetScalarAttribute("LevelScale", fval))
    {
    obj->SetLevelScale(fval);
    }

  if (elem->GetScalarAttribute("ShowSliceAndImage", ival))
    {
    obj->SetShowSliceAndImage(ival);
    }

  // Get nested elements
  
  // Text property

  vtkXMLTextPropertyReader *xmlr = vtkXMLTextPropertyReader::New();
  if (xmlr->IsInNestedElement(
        elem, vtkXMLCornerAnnotationWriter::GetTextPropertyElementName()))
    {
    vtkTextProperty *tprop = obj->GetTextProperty();
    if (!tprop)
      {
      tprop = vtkTextProperty::New();
      obj->SetTextProperty(tprop);
      tprop->Delete();
      }
    xmlr->SetObject(tprop);
    xmlr->ParseInNestedElement(
      elem, vtkXMLCornerAnnotationWriter::GetTextPropertyElementName());
    }
  xmlr->Delete();
  
  return 1;
}
