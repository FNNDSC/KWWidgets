/*=========================================================================

  Module:    $RCSfile: vtkXMLKWRenderWidgetReader.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkXMLKWRenderWidgetReader.h"

#include "vtkCamera.h"
#include "vtkCornerAnnotation.h"
#include "vtkKWRenderWidget.h"
#include "vtkObjectFactory.h"
#include "vtkTextActor.h"
#include "vtkXMLCameraReader.h"
#include "vtkXMLCornerAnnotationReader.h"
#include "vtkXMLDataElement.h"
#include "vtkXMLKWRenderWidgetWriter.h"
#include "vtkXMLTextActorReader.h"

vtkStandardNewMacro(vtkXMLKWRenderWidgetReader);
vtkCxxRevisionMacro(vtkXMLKWRenderWidgetReader, "$Revision: 1.8 $");

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

  float fbuffer3[3];
  const char *cptr;

  if (elem->GetVectorAttribute("BackgroundColor", 3, fbuffer3) == 3)
    {
    obj->SetBackgroundColor(fbuffer3);
    }

  cptr = elem->GetAttribute("DistanceUnits");
  if (cptr)
    {
    obj->SetDistanceUnits(cptr);
    }

  // Get nested elements

  // Camera

  vtkCamera *cam = obj->GetCurrentCamera();
  if (cam)
    {
    vtkXMLCameraReader *xmlr = vtkXMLCameraReader::New();
    xmlr->SetObject(cam);
    xmlr->ParseInNestedElement(
      elem, vtkXMLKWRenderWidgetWriter::GetCurrentCameraElementName());
    xmlr->Delete();
    }

  // Corner Annotation

  vtkCornerAnnotation *canno = obj->GetCornerAnnotation();
  if (canno)
    {
    vtkXMLCornerAnnotationReader *xmlr = vtkXMLCornerAnnotationReader::New();
    xmlr->SetObject(canno);
    if (xmlr->ParseInNestedElement(
          elem, vtkXMLKWRenderWidgetWriter::GetCornerAnnotationElementName()))
      {
      obj->SetCornerAnnotationVisibility(canno->GetVisibility()); // add prop
      }
    xmlr->Delete();
    }

  // Header Annotation

  vtkTextActor *texta = obj->GetHeaderAnnotation();
  if (texta)
    {
    vtkXMLTextActorReader *xmlr = vtkXMLTextActorReader::New();
    xmlr->SetObject(texta);
    if (xmlr->ParseInNestedElement(
          elem, vtkXMLKWRenderWidgetWriter::GetHeaderAnnotationElementName()))
      {
      obj->SetHeaderAnnotationVisibility(texta->GetVisibility()); // add prop
      }
    xmlr->Delete();
    }
  
  return 1;
}

