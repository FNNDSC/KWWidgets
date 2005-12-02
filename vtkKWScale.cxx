/*=========================================================================

  Module:    $RCSfile: vtkKWScale.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWScale.h"

#include "vtkKWLabel.h"
#include "vtkObjectFactory.h"

#include <vtksys/stl/string>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWScale );
vtkCxxRevisionMacro(vtkKWScale, "$Revision: 1.109 $");

//----------------------------------------------------------------------------
vtkKWScale::vtkKWScale()
{
  this->Value      = 0;
  this->Range[0]   = 0;
  this->Range[1]   = 100;  
  this->Resolution = 1;

  this->Orientation = vtkKWTkOptions::OrientationHorizontal;
  this->Command      = NULL;
  this->StartCommand = NULL;
  this->EndCommand   = NULL;

  this->ClampValue      = 1;
  this->DisableCommands = 0;

  this->DisableScaleValueCallback = 1;
}

//----------------------------------------------------------------------------
vtkKWScale::~vtkKWScale()
{
  if (this->IsAlive())
    {
    this->UnBind();
    }

  if (this->Command)
    {
    delete [] this->Command;
    this->Command = NULL;
    }

  if (this->StartCommand)
    {
    delete [] this->StartCommand;
    this->StartCommand = NULL;
    }

  if (this->EndCommand)
    {
    delete [] this->EndCommand;
    this->EndCommand = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkKWScale::Create()
{
  // Call the superclass to set the appropriate flags then create manually

  if (!this->Superclass::CreateSpecificTkWidget("scale"))
    {
    vtkErrorMacro("Failed creating widget " << this->GetClassName());
    return;
    }

  this->ValueVisibilityOff();
  this->SetBorderWidth(2);
  this->SetHighlightThickness(0);

  this->UpdateOrientation();
  this->UpdateResolution();
  this->UpdateRange();
  this->UpdateValue();
  
  this->Bind();
}

//----------------------------------------------------------------------------
void vtkKWScale::UpdateOrientation()
{
  if (this->IsCreated())
    {
    this->SetConfigurationOption(
      "-orient", vtkKWTkOptions::GetOrientationAsTkOptionValue(
        this->Orientation));
    }
}

//----------------------------------------------------------------------------
void vtkKWScale::SetOrientation(int orientation)
{
  if (this->Orientation == orientation ||
      (orientation != vtkKWTkOptions::OrientationHorizontal &&
       orientation != vtkKWTkOptions::OrientationVertical))
    {
    return;
    }
      
  this->Orientation = orientation;
  this->Modified();

  this->UpdateOrientation();
}

//----------------------------------------------------------------------------
void vtkKWScale::GetTroughColor(double *r, double *g, double *b)
{
  this->GetConfigurationOptionAsColor("-troughcolor", r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWScale::GetTroughColor()
{
  return this->GetConfigurationOptionAsColor("-troughcolor");
}

//----------------------------------------------------------------------------
void vtkKWScale::SetTroughColor(double r, double g, double b)
{
  this->SetConfigurationOptionAsColor("-troughcolor", r, g, b);
}

//----------------------------------------------------------------------------
void vtkKWScale::Bind()
{
  this->SetBinding("<ButtonPress>", this, "ButtonPressCallback");
  this->SetBinding("<ButtonRelease>", this, "ButtonReleaseCallback");

  this->AddBinding("<ButtonPress>", this, "DisableScaleValueCallbackOff");
  this->AddBinding("<ButtonRelease>", this, "DisableScaleValueCallbackOn");

  char *command = NULL;
  this->SetObjectMethodCommand(&command, this, "ScaleValueCallback");
  this->SetConfigurationOption("-command", command);
  delete [] command;
}

//----------------------------------------------------------------------------
void vtkKWScale::UnBind()
{
  this->RemoveBinding("<ButtonPress>");
  this->RemoveBinding("<ButtonRelease>");

  this->SetConfigurationOption("-command", NULL);
}

//----------------------------------------------------------------------------
void vtkKWScale::SetResolution(double r)
{
  if (this->Resolution == r)
    {
    return;
    }

  this->Resolution = r;
  this->Modified();

  this->UpdateResolution();
}

//----------------------------------------------------------------------------
void vtkKWScale::UpdateResolution()
{
  if (this->IsCreated())
    {
    this->SetConfigurationOptionAsDouble("-resolution", this->Resolution);
    }
}

//----------------------------------------------------------------------------
void vtkKWScale::SetValue(double num)
{
  if (this->ClampValue)
    {
    if (this->Range[1] > this->Range[0])
      {
      if (num > this->Range[1]) 
        { 
        num = this->Range[1]; 
        }
      else if (num < this->Range[0])
        {
        num = this->Range[0];
        }
      }
    else
      {
      if (num < this->Range[1]) 
        { 
        num = this->Range[1]; 
        }
      else if (num > this->Range[0])
        {
        num = this->Range[0];
        }
      }
    }

  if (this->Value == num)
    {
    return;
    }

  this->Value = num;
  this->Modified();

  this->UpdateValue();

  this->InvokeCommand();
}

//----------------------------------------------------------------------------
void vtkKWScale::UpdateValue()
{
  if (!this->IsCreated())
    {
    return;
    }

  int was_disabled = !this->GetEnabled();
  if (was_disabled)
    {
    this->SetState(vtkKWTkOptions::StateNormal);
    this->SetEnabled(1);
    }

  this->Script("%s set %g", this->GetWidgetName(), this->Value);

  if (was_disabled)
    {
    this->SetState(vtkKWTkOptions::StateDisabled);
    this->SetEnabled(0);
    }
}

//----------------------------------------------------------------------------
void vtkKWScale::SetRange(double min, double max)
{
  if (this->Range[0] == min && this->Range[1] == max)
    {
    return;
    }

  this->Range[0] = min;
  this->Range[1] = max;

  this->Modified();

  this->UpdateRange();
}

//----------------------------------------------------------------------------
void vtkKWScale::UpdateRange()
{
  if (this->IsCreated())
    {
    this->SetConfigurationOptionAsDouble("-from", this->Range[0]);
    this->SetConfigurationOptionAsDouble("-to", this->Range[1]);
    }
}

//----------------------------------------------------------------------------
void vtkKWScale::ScaleValueCallback(double num)
{
  if (this->DisableScaleValueCallback)
    {
    return;
    }

  this->SetValue(num);
}

//----------------------------------------------------------------------------
void vtkKWScale::ButtonPressCallback()
{
  this->InvokeStartCommand();
}

//----------------------------------------------------------------------------
void vtkKWScale::ButtonReleaseCallback()
{
  this->InvokeEndCommand();
}

//----------------------------------------------------------------------------
void vtkKWScale::InvokeObjectMethodCommand(const char *command)
{
  if (!this->DisableCommands)
    {
    this->Superclass::InvokeObjectMethodCommand(command);
    }
}

//----------------------------------------------------------------------------
void vtkKWScale::SetCommand(vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->Command, object, method);
}

//----------------------------------------------------------------------------
void vtkKWScale::InvokeCommand()
{
  this->InvokeObjectMethodCommand(this->Command);
}

//----------------------------------------------------------------------------
void vtkKWScale::SetStartCommand(vtkObject *object, const char * method)
{
  this->SetObjectMethodCommand(&this->StartCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWScale::InvokeStartCommand()
{
  this->InvokeObjectMethodCommand(this->StartCommand);
}

//----------------------------------------------------------------------------
void vtkKWScale::SetEndCommand(vtkObject *object, const char * method)
{
  this->SetObjectMethodCommand(&this->EndCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWScale::InvokeEndCommand()
{
  this->InvokeObjectMethodCommand(this->EndCommand);
}

//----------------------------------------------------------------------------
void vtkKWScale::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->SetState(this->GetEnabled());
}

//----------------------------------------------------------------------------
void vtkKWScale::SetWidth(int width)
{
  this->SetConfigurationOptionAsInt("-width", width);
}

//----------------------------------------------------------------------------
int vtkKWScale::GetWidth()
{
  return this->GetConfigurationOptionAsInt("-width");
}

//----------------------------------------------------------------------------
void vtkKWScale::SetLength(int length)
{
  this->SetConfigurationOptionAsInt("-length", length);
}

//----------------------------------------------------------------------------
int vtkKWScale::GetLength()
{
  return this->GetConfigurationOptionAsInt("-length");
}

//----------------------------------------------------------------------------
void vtkKWScale::SetSliderLength(int length)
{
  this->SetConfigurationOptionAsInt("-sliderlength", length);
}

//----------------------------------------------------------------------------
int vtkKWScale::GetSliderLength()
{
  return this->GetConfigurationOptionAsInt("-sliderlength");
}

//----------------------------------------------------------------------------
void vtkKWScale::SetValueVisibility(int val)
{
  this->SetConfigurationOptionAsInt("-showvalue", val);
}

//----------------------------------------------------------------------------
int vtkKWScale::GetValueVisibility()
{
  return this->GetConfigurationOptionAsInt("-showvalue");
}

//----------------------------------------------------------------------------
void vtkKWScale::SetTickInterval(double val)
{
  this->SetConfigurationOptionAsDouble("-tickinterval", val);
}

//----------------------------------------------------------------------------
double vtkKWScale::GetTickInterval()
{
  return this->GetConfigurationOptionAsDouble("-tickinterval");
}

//---------------------------------------------------------------------------
void vtkKWScale::SetLabelText(const char *label)
{
  this->SetTextOption("-label", label); 
}

//---------------------------------------------------------------------------
const char* vtkKWScale::GetLabelText()
{
  return this->GetTextOption("-label"); 
}

//----------------------------------------------------------------------------
void vtkKWScale::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Value: " << this->Value << endl;
  os << indent << "Resolution: " << this->Resolution << endl;
  os << indent << "Orientation: " << this->Orientation << endl;
  os << indent << "Range: " << this->Range[0] << "..." <<  this->Range[1] << endl;
  os << indent << "DisableCommands: "
     << (this->DisableCommands ? "On" : "Off") << endl;
  os << indent << "ClampValue: " << (this->ClampValue ? "On" : "Off") << endl;
  os << indent << "DisableScaleValueCallback: " << (this->DisableScaleValueCallback ? "On" : "Off") << endl;
}
