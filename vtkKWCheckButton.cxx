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
#include "vtkKWIcon.h"

#include <vtksys/stl/string>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWCheckButton );
vtkCxxRevisionMacro(vtkKWCheckButton, "$Revision: 1.42 $");

//----------------------------------------------------------------------------
vtkKWCheckButton::vtkKWCheckButton() 
{
  this->IndicatorVisibility = 1;
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
void vtkKWCheckButton::SetIndicatorVisibility(int ind)
{
  if (ind != this->IndicatorVisibility)
    {
    this->IndicatorVisibility = ind;
    this->Modified();
    this->SetConfigurationOptionAsInt(
      "-indicatoron", (this->IndicatorVisibility ? 1 : 0));
    }
  this->SetMyText(0);
}

//----------------------------------------------------------------------------
void vtkKWCheckButton::SetText(const char* txt)
{
  this->SetMyText(txt);
  this->SetTextOption("-text", txt);
}

//----------------------------------------------------------------------------
const char* vtkKWCheckButton::GetText()
{
  return this->MyText;
}

//----------------------------------------------------------------------------
int vtkKWCheckButton::GetSelectedState()
{
  if (this->IsCreated() && this->VariableName)
    {
    const char *varvalue = 
      Tcl_GetVar(
        this->GetApplication()->GetMainInterp(), this->VariableName, 0);
    const char *onvalue = this->GetConfigurationOption("-onvalue");
    return varvalue && onvalue && !strcmp(varvalue, onvalue);
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWCheckButton::SetSelectedState(int s)
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
void vtkKWCheckButton::ToggleSelectedState()
{
  this->SetSelectedState(this->GetSelectedState() ? 0 : 1);
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
  this->SetConfigurationOptionAsInt(
    "-indicatoron", (this->IndicatorVisibility ? 1 : 0));

  this->SetTextOption("-text", this->MyText);

  // Set the variable name if not set already

  if (!this->VariableName)
    {
    vtksys_stl::string vname(this->GetWidgetName());
    vname += "Value";
    this->SetVariableName(vname.c_str());
    }
  else
    {
    this->SetConfigurationOption("-variable", this->VariableName);
    }
}

//----------------------------------------------------------------------------
void vtkKWCheckButton::SetCommand(vtkObject *object, const char *method)
{
  if (this->IsCreated())
    {
    char *command = NULL;
    this->SetObjectMethodCommand(&command, object, method);
    this->SetConfigurationOption("-command", command);
    delete [] command;
    }
}

//----------------------------------------------------------------------------
void vtkKWCheckButton::SetAnchor(int anchor)
{
  this->SetConfigurationOption(
    "-anchor", vtkKWTkOptions::GetAnchorAsTkOptionValue(anchor));
}

//----------------------------------------------------------------------------
int vtkKWCheckButton::GetAnchor()
{
  return vtkKWTkOptions::GetAnchorFromTkOptionValue(
    this->GetConfigurationOption("-anchor"));
}

//----------------------------------------------------------------------------
void vtkKWCheckButton::SetImageToPredefinedIcon(int icon_index)
{
  vtkKWIcon *icon = vtkKWIcon::New();
  icon->SetImage(icon_index);
  this->SetImageToIcon(icon);
  icon->Delete();
}

//----------------------------------------------------------------------------
void vtkKWCheckButton::SetImageToIcon(vtkKWIcon* icon)
{
  if (icon)
    {
    this->SetImageToPixels(
      icon->GetData(), 
      icon->GetWidth(), icon->GetHeight(), icon->GetPixelSize());
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
  vtkKWTkUtilities::SetImageOptionToPixels(
    this, pixels, width, height, pixel_size, buffer_length);

  vtkKWTkUtilities::SetImageOptionToPixels(
    this, pixels, width, height, pixel_size, buffer_length,
    "-selectcolor", "-selectimage");
}

// ---------------------------------------------------------------------------
void vtkKWCheckButton::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->SetState(this->GetEnabled());
}

//----------------------------------------------------------------------------
void vtkKWCheckButton::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "VariableName: " 
     << (this->VariableName ? this->VariableName : "None" );
}

