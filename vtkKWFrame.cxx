/*=========================================================================

  Module:    $RCSfile: vtkKWFrame.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWApplication.h"
#include "vtkKWFrame.h"
#include "vtkObjectFactory.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWFrame);
vtkCxxRevisionMacro(vtkKWFrame, "$Revision: 1.28 $");

//----------------------------------------------------------------------------
void vtkKWFrame::Create(vtkKWApplication *app)
{
  // Call the superclass to set the appropriate flags then create manually

  if (!this->Superclass::CreateSpecificTkWidget(app, "frame"))
    {
    vtkErrorMacro("Failed creating widget " << this->GetClassName());
    return;
    }
}

//----------------------------------------------------------------------------
void vtkKWFrame::SetWidth(int width)
{
  this->SetConfigurationOptionAsInt("-width", width);
}

//----------------------------------------------------------------------------
int vtkKWFrame::GetWidth()
{
  return this->GetConfigurationOptionAsInt("-width");
}

//----------------------------------------------------------------------------
void vtkKWFrame::SetHeight(int height)
{
  this->SetConfigurationOptionAsInt("-height", height);
}

//----------------------------------------------------------------------------
int vtkKWFrame::GetHeight()
{
  return this->GetConfigurationOptionAsInt("-height");
}

//----------------------------------------------------------------------------
void vtkKWFrame::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

