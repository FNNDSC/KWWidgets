/*=========================================================================

  Module:    $RCSfile: vtkKWPushButton.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWApplication.h"
#include "vtkKWPushButton.h"
#include "vtkObjectFactory.h"
#include "vtkKWIcon.h"
#include "vtkKWTkUtilities.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWPushButton );
vtkCxxRevisionMacro(vtkKWPushButton, "$Revision: 1.23 $");

//----------------------------------------------------------------------------
vtkKWPushButton::vtkKWPushButton()
{
  this->ButtonText = 0;
}

//----------------------------------------------------------------------------
vtkKWPushButton::~vtkKWPushButton()
{
  this->SetButtonText(0);
}

//----------------------------------------------------------------------------
void vtkKWPushButton::Create(vtkKWApplication *app)
{
  // Call the superclass to create the widget and set the appropriate flags

  if (!this->Superclass::CreateSpecificTkWidget(app, "button"))
    {
    vtkErrorMacro("Failed creating widget " << this->GetClassName());
    return;
    }

  this->SetTextOption("-text", this->ButtonText);

  // Update enable state

  this->UpdateEnableState();
}

//----------------------------------------------------------------------------
void vtkKWPushButton::SetText( const char *name )
{
  this->SetButtonText(name);
  this->SetTextOption("-text", name);
}

//----------------------------------------------------------------------------
char* vtkKWPushButton::GetText()
{
  return this->ButtonText;
}

//----------------------------------------------------------------------------
void vtkKWPushButton::SetWidth(int width)
{
  if (this->IsCreated())
    {
    this->Script("%s configure -width %d", this->GetWidgetName(), width);
    }
}

//----------------------------------------------------------------------------
int vtkKWPushButton::GetWidth()
{
  return this->GetConfigurationOptionAsInt("-width");
}

//----------------------------------------------------------------------------
void vtkKWPushButton::SetAnchor(int anchor)
{
  this->SetConfigurationOption(
    "-anchor", vtkKWTkOptions::GetAnchorAsTkOptionValue(anchor));
}

//----------------------------------------------------------------------------
int vtkKWPushButton::GetAnchor()
{
  return vtkKWTkOptions::GetAnchorFromTkOptionValue(
    this->GetConfigurationOption("-anchor"));
}

//----------------------------------------------------------------------------
void vtkKWPushButton::SetImageToPredefinedIcon(int icon_index)
{
  vtkKWIcon *icon = vtkKWIcon::New();
  icon->SetImage(icon_index);
  this->SetImageToIcon(icon);
  icon->Delete();
}

//----------------------------------------------------------------------------
void vtkKWPushButton::SetImageToIcon(vtkKWIcon* icon)
{
  if (icon)
    {
    this->SetImageToPixels(
      icon->GetData(), 
      icon->GetWidth(), icon->GetHeight(), icon->GetPixelSize());
    }
}

//----------------------------------------------------------------------------
void vtkKWPushButton::SetImageToPixels(const unsigned char* pixels, 
                                       int width, 
                                       int height,
                                       int pixel_size,
                                       unsigned long buffer_length)
{
  vtkKWTkUtilities::SetImageOptionToPixels(
    this, pixels, width, height, pixel_size, buffer_length);
}

//----------------------------------------------------------------------------
void vtkKWPushButton::SetCommand(vtkObject *object, const char *method)
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
void vtkKWPushButton::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->SetStateOption(this->GetEnabled());
}

//----------------------------------------------------------------------------
void vtkKWPushButton::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
