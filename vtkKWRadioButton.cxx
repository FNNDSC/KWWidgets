/*=========================================================================

  Module:    $RCSfile: vtkKWRadioButton.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWApplication.h"
#include "vtkKWRadioButton.h"
#include "vtkObjectFactory.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWRadioButton );
vtkCxxRevisionMacro(vtkKWRadioButton, "$Revision: 1.19 $");

//----------------------------------------------------------------------------
void vtkKWRadioButton::Create(vtkKWApplication *app)
{
  // Call the superclass to create the widget and set the appropriate flags

  if (!this->vtkKWWidget::CreateSpecificTkWidget(app, "radiobutton"))
    {
    vtkErrorMacro("Failed creating widget " << this->GetClassName());
    return;
    }

  this->SetConfigurationOptionAsInt("-value", 1);
  this->Configure();

  // Update enable state

  this->UpdateEnableState();
}

//----------------------------------------------------------------------------
void vtkKWRadioButton::SetValue(int v)
{
  if (this->IsCreated())
    {
    this->Script("%s configure -value %d", this->GetWidgetName(), v);
    }
}

//----------------------------------------------------------------------------
void vtkKWRadioButton::SetValue(const char *v)
{
  if (this->IsCreated())
    {
    this->Script("%s configure -value {%s}", this->GetWidgetName(), v);
    }
}

//----------------------------------------------------------------------------
void vtkKWRadioButton::SetVariableValue(int v)
{
  if (this->IsCreated())
    {
    this->Script("set %s %d", this->GetVariableName(), v);
    }
}

//----------------------------------------------------------------------------
void vtkKWRadioButton::SetVariableValue(const char *v)
{
  if (this->IsCreated())
    {
    this->Script("set %s {%s}", this->GetVariableName(), v);
    }
}

//----------------------------------------------------------------------------
int vtkKWRadioButton::GetState()
{
  if (this->IsCreated())
    {
    return atoi(
      this->Script("expr {${%s}} == {[%s cget -value]}",
                   this->VariableName, this->GetWidgetName()));
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWRadioButton::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

