/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkXMLKWRenderWidgetReader.cxx,v $
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
#include "vtkXMLKWRenderWidgetReader.h"

#include "vtkCamera.h"
#include "vtkCornerAnnotation.h"
#include "vtkKWRenderWidget.h"
#include "vtkObjectFactory.h"
#include "vtkXMLCameraReader.h"
#include "vtkXMLCornerAnnotationReader.h"
#include "vtkXMLDataElement.h"
#include "vtkXMLKWRenderWidgetWriter.h"

vtkStandardNewMacro(vtkXMLKWRenderWidgetReader);
vtkCxxRevisionMacro(vtkXMLKWRenderWidgetReader, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
char* vtkXMLKWRenderWidgetReader::GetRootElementName()
{
  return "KWRenderWidget";
}

//----------------------------------------------------------------------------
int vtkXMLKWRenderWidgetReader::Parse(vtkXMLDataElement *elem)
{
  if (!this->Superclass::Parse(elem))
    {
    return 0;
    }

  vtkKWRenderWidget *obj = vtkKWRenderWidget::SafeDownCast(this->Object);
  if (!obj)
    {
    vtkWarningMacro(<< "The KWRenderWidget is not set!");
    return 0;
    }

  // Get attributes

  float fbuffer3[3], fval;
  const char *cptr;
  int ival;

  if (elem->GetVectorAttribute("BackgroundColor", 3, fbuffer3) == 3)
    {
    obj->SetBackgroundColor(fbuffer3);
    }

  cptr = elem->GetAttribute("Units");
  if (cptr)
    {
    obj->SetUnits(cptr);
    }

  if (elem->GetScalarAttribute("ScalarShift", fval))
    {
    obj->SetScalarShift(fval);
    }

  if (elem->GetScalarAttribute("ScalarScale", fval))
    {
    obj->SetScalarScale(fval);
    }

  // Get nested elements

  vtkXMLDataElement *nested_elem;

  // Camera

  vtkCamera *cam = obj->GetCurrentCamera();
  if (cam)
    {
    vtkXMLCameraReader *xmlr = vtkXMLCameraReader::New();
    nested_elem = elem->FindNestedElementWithName(xmlr->GetRootElementName());
    if (nested_elem)
      {
      xmlr->SetObject(cam);
      xmlr->Parse(nested_elem);
      }
    xmlr->Delete();
    }

  // Corner Annotation

  vtkCornerAnnotation *canno = obj->GetCornerAnnotation();
  if (canno)
    {
    vtkXMLCornerAnnotationReader *xmlr = vtkXMLCornerAnnotationReader::New();
    nested_elem = elem->FindNestedElementWithName(xmlr->GetRootElementName());
    if (nested_elem)
      {
      xmlr->SetObject(canno);
      xmlr->Parse(nested_elem);
      if (nested_elem->GetScalarAttribute("Visibility", ival))
        {
        obj->SetCornerAnnotationVisibility(ival);
        }
      }
    xmlr->Delete();
    }

  // Header Annotation

  vtkXMLDataElement *ha_elem = 
    elem->FindNestedElementWithName(
      vtkXMLKWRenderWidgetWriter::GetHeaderAnnotationElementName());
  if (ha_elem)
    {
    if (ha_elem->GetScalarAttribute("Visibility", ival))
      {
      obj->SetHeaderAnnotationVisibility(ival);
      }
    if (ha_elem->GetVectorAttribute("Color", 3, fbuffer3) == 3)
      {
      obj->SetHeaderAnnotationColor(fbuffer3);
      }
    cptr = ha_elem->GetAttribute("Text");
    if (cptr)
      {
      obj->SetHeaderAnnotationText(cptr);
      }
    }
  
  return 1;
}
