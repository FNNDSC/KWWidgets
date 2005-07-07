/*=========================================================================

  Module:    $RCSfile: vtkKWCheckButton.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWApplication.h"
#include "vtkKWCheckButton.h"
#include "vtkObjectFactory.h"
#include "vtkKWTkUtilities.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWCheckButton );
vtkCxxRevisionMacro(vtkKWCheckButton, "$Revision: 1.37 $");

//----------------------------------------------------------------------------
vtkKWCheckButton::vtkKWCheckButton() 
{
  this->Indicator = 1;
  this->MyText = 0;
  this->VariableName = NULL;
}

//----------------------------------------------------------------------------
vtkKWCheckButton::~vtkKWCheckButton() 
{
  this->SetMyText(0);
  this->SetVariableName(0);
}

//----------------------------------------------------------------------------
void vtkKWCheckButton::SetVariableName(const char* _arg)
{
  if (this->VariableName == NULL && _arg == NULL) 
    { 
    return;
    }

  if (this->VariableName && _arg && (!strcmp(this->VariableName, _arg))) 
    { 
    return;
    }

  if (this->VariableName) 
    { 
    delete [] this->VariableName; 
    }

  if (_arg)
    {
    this->VariableName = new char[strlen(_arg)+1];
    strcpy(this->VariableName,_arg);
    }
   else
    {
    this->VariableName = NULL;
    }

  this->Modified();
  
  if (this->VariableName)
    {
    this->SetConfigurationOption("-variable", this->VariableName);
    }
} 

//----------------------------------------------------------------------------
void vtkKWCheckButton::SetIndicator(int ind)
{
  if (ind != this->Indicator)
    {
    this->Indicator = ind;
    this->Modified();
    this->SetConfigurationOptionAsInt(
      "-indicatoron", (this->Indicator ? 1 : 0));
    }
  this->SetMyText(0);
}

//----------------------------------------------------------------------------
void vtkKWCheckButton::SetText(const char* txt)
{
  this->SetMyText(txt);
  this->SetTextOption(txt);
}

//----------------------------------------------------------------------------
const char* vtkKWCheckButton::GetText()
{
  return this->MyText;
}

//----------------------------------------------------------------------------
int vtkKWCheckButton::GetState()
{
  if (this->IsCreated())
    {
    return atoi(
      this->Script("expr {${%s}} == {[%s cget -onvalue]}",
                   this->VariableName, this->GetWidgetName()));
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWCheckButton::SetState(int s)
{
  if (this->IsCreated())
    {
    int was_disabled = !this->GetEnabled();
    if (was_disabled)
      {
      this->SetEnabled(1);
      }

    if (s)
      {
      this->Script("%s select",this->GetWidgetName());
      }
    else
      {
      this->Script("%s deselect",this->GetWidgetName());
      }

    if (was_disabled)
      {
      this->SetEnabled(0);
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWCheckButton::ToggleState()
{
  this->SetState(this->GetState() ? 0 : 1);
}

//----------------------------------------------------------------------------
void vtkKWCheckButton::Create(vtkKWApplication *app)
{
  // Call the superclass to create the widget and set the appropriate flags

  if (!this->Superclass::CreateSpecificTkWidget(app, "checkbutton"))
    {
    vtkErrorMacro("Failed creating widget " << this->GetClassName());
    return;
    }

  this->Configure();

  // Update enable state

  this->UpdateEnableState();
}

//----------------------------------------------------------------------------
void vtkKWCheckButton::Configure()
{
  const char *wname = this->GetWidgetName();

  this->Script("%s configure -indicatoron %d",
               wname, (this->Indicator ? 1 : 0));

  this->SetTextOption(this->MyText);

  // Set the variable name if not set already
  if (!this->VariableName)
    {
    char *vname = new char [strlen(wname) + 5 + 1];
    sprintf(vname, "%sValue", wname);
    this->SetVariableName(vname);
    delete [] vname;
    }
  else
    {
    this->Script("%s configure -variable {%s}", wname, this->VariableName);
    }
}

//----------------------------------------------------------------------------
void vtkKWCheckButton::SetImageToPixels(
  const unsigned char* pixels, 
  int width, 
  int height,
  int pixel_size,
  unsigned long buffer_length)
{
  this->Superclass::SetImageToPixels(
    pixels, width, height, pixel_size, buffer_length);

  vtkKWTkUtilities::SetImageOptionToPixels(
    this,
    pixels, 
    width, height, pixel_size,
    buffer_length,
    "-selectcolor", 
    "-selectimage");
}

// ---------------------------------------------------------------------------
void vtkKWCheckButton::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->SetStateOption(this->GetEnabled());
}

//----------------------------------------------------------------------------
void vtkKWCheckButton::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "VariableName: " 
     << (this->VariableName ? this->VariableName : "None" );
}

