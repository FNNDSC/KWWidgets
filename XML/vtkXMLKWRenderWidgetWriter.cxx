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
#include "vtkXMLKWRenderWidgetWriter.h"

#include "vtkCamera.h"
#include "vtkCornerAnnotation.h"
#include "vtkKWRenderWidget.h"
#include "vtkObjectFactory.h"
#include "vtkTextActor.h"
#include "vtkXMLCameraWriter.h"
#include "vtkXMLCornerAnnotationWriter.h"
#include "vtkXMLDataElement.h"
#include "vtkXMLTextActorWriter.h"

vtkStandardNewMacro(vtkXMLKWRenderWidgetWriter);
vtkCxxRevisionMacro(vtkXMLKWRenderWidgetWriter, "$Revision: 1.5 $");

//----------------------------------------------------------------------------
char* vtkXMLKWRenderWidgetWriter::GetRootElementName()
{
  return "KWRenderWidget";
}

//----------------------------------------------------------------------------
char* vtkXMLKWRenderWidgetWriter::GetCurrentCameraElementName()
{
  return "CurrentCamera";
}

//----------------------------------------------------------------------------
char* vtkXMLKWRenderWidgetWriter::GetCornerAnnotationElementName()
{
  return "CornerAnnotation";
}

//----------------------------------------------------------------------------
char* vtkXMLKWRenderWidgetWriter::GetHeaderAnnotationElementName()
{
  return "HeaderAnnotation";
}

//----------------------------------------------------------------------------
int vtkXMLKWRenderWidgetWriter::AddAttributes(vtkXMLDataElement *elem)
{
  if (!this->Superclass::AddAttributes(elem))
    {
    return 0;
    }

  vtkKWRenderWidget *obj = vtkKWRenderWidget::SafeDownCast(this->Object);
  if (!obj)
    {
    vtkWarningMacro(<< "The KWRenderWidget is not set!");
    return 0;
    }

  elem->SetVectorAttribute("BackgroundColor", 3, obj->GetBackgroundColor());

  elem->SetAttribute("Units", obj->GetUnits());

  return 1;
}

//----------------------------------------------------------------------------
int vtkXMLKWRenderWidgetWriter::AddNestedElements(vtkXMLDataElement *elem)
{
  if (!this->Superclass::AddNestedElements(elem))
    {
    return 0;
    }

  vtkKWRenderWidget *obj = vtkKWRenderWidget::SafeDownCast(this->Object);
  if (!obj)
    {
    vtkWarningMacro(<< "The KWRenderWidget is not set!");
    return 0;
    }

  // Camera

  vtkCamera *cam = obj->GetCurrentCamera();
  if (cam)
    {
    vtkXMLCameraWriter *xmlw = vtkXMLCameraWriter::New();
    xmlw->SetObject(cam);
    xmlw->CreateInNestedElement(elem, this->GetCurrentCameraElementName());
    xmlw->Delete();
    }

  // Corner Annotation

  vtkCornerAnnotation *canno = obj->GetCornerAnnotation();
  if (canno)
    {
    vtkXMLCornerAnnotationWriter *xmlw = vtkXMLCornerAnnotationWriter::New();
    xmlw->SetObject(canno);
    xmlw->CreateInNestedElement(elem, this->GetCornerAnnotationElementName());
    xmlw->Delete();
    }

  // Header Annotation

  vtkTextActor *texta = obj->GetHeaderAnnotation();
  if (texta)
    {
    vtkXMLTextActorWriter *xmlw = vtkXMLTextActorWriter::New();
    xmlw->SetObject(texta);
    xmlw->CreateInNestedElement(elem, this->GetHeaderAnnotationElementName());
    xmlw->Delete();
    }
  
  return 1;
}

