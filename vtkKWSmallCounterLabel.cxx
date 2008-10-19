/*=========================================================================

  Module:    vtkKWSmallCounterLabel.cxx,v

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWSmallCounterLabel.h"

#include "vtkKWOptions.h"
#include "vtkKWIcon.h"
#include "vtkKWTopLevel.h"
#include "vtkObjectFactory.h"

#include <vtksys/stl/string>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWSmallCounterLabel );
vtkCxxRevisionMacro(vtkKWSmallCounterLabel, "1.119");

//----------------------------------------------------------------------------
vtkKWSmallCounterLabel::vtkKWSmallCounterLabel()
{
  this->Value = 0;
  this->OriginalParent = NULL;
}

//----------------------------------------------------------------------------
vtkKWSmallCounterLabel::~vtkKWSmallCounterLabel()
{
}

//----------------------------------------------------------------------------
void vtkKWSmallCounterLabel::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // We need to override

  this->OriginalParent = this->Parent;
  this->SetParent(this->GetParentTopLevel());

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  // It's an icon, really

  this->SetText(NULL);
  this->SetHighlightThickness(0);
  this->SetBorderWidth(0);
  this->SetPadX(0);
  this->SetPadY(0);

  this->UpdateIcon();
  this->Place();
}

//----------------------------------------------------------------------------
void vtkKWSmallCounterLabel::SetValue(unsigned int arg)
{
  if (arg < 0)
    {
    arg = 0;
    }
  else if (arg > 10)
    {
    arg = 10;
    }
  if (this->Value == arg)
    {
    return;
    }

  int old_value = this->Value;
  this->Value = arg;
  this->Modified();
  
  this->UpdateIcon();
  
  if (!arg || !old_value)
    {
    this->Place();
    }
}

//----------------------------------------------------------------------------
void vtkKWSmallCounterLabel::UpdateIcon()
{
  if (this->Value)
    {
    int v = (this->Value > 10 ? 10 : this->Value);
    this->SetImageToPredefinedIcon(vtkKWIcon::IconSmallCounterBlue1 + v - 1);
    }
}

//----------------------------------------------------------------------------
void vtkKWSmallCounterLabel::Place()
{
  if (!this->IsCreated())
    {
    return;
    }

  if (!this->Value || 
      (!this->GetParent()->IsPacked() && 
       !this->GetParent()->IsGridded() &&
       !this->GetParent()->IsPlaced()))
    {
    this->Unplace();
      }
  else
    {
    this->Script(
      "place %s -anchor center -relx 0.70 -y -3 -in %s", 
      this->GetWidgetName(), this->OriginalParent->GetWidgetName());
    }
}

//----------------------------------------------------------------------------
void vtkKWSmallCounterLabel::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Value: " << this->Value << endl;
}
