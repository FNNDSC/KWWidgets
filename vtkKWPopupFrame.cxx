/*=========================================================================

  Module:    $RCSfile: vtkKWPopupFrame.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWPopupFrame.h"

#include "vtkKWFrame.h"
#include "vtkKWFrameLabeled.h"
#include "vtkKWPopupButton.h"
#include "vtkObjectFactory.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWPopupFrame );
vtkCxxRevisionMacro(vtkKWPopupFrame, "$Revision: 1.8 $");

int vtkKWPopupFrameCommand(ClientData cd, Tcl_Interp *interp,
                           int argc, char *argv[]);

//----------------------------------------------------------------------------
vtkKWPopupFrame::vtkKWPopupFrame()
{
  this->CommandFunction = vtkKWPopupFrameCommand;

  // GUI

  this->PopupMode               = 0;

  this->PopupButton             = NULL;
  this->Frame                   = vtkKWFrameLabeled::New();
}

//----------------------------------------------------------------------------
vtkKWPopupFrame::~vtkKWPopupFrame()
{
  // GUI

  if (this->PopupButton)
    {
    this->PopupButton->Delete();
    this->PopupButton = NULL;
    }

  if (this->Frame)
    {
    this->Frame->Delete();
    this->Frame = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkKWPopupFrame::Create(vtkKWApplication *app)
{
  // Call the superclass to create the widget and set the appropriate flags

  if (!this->Superclass::CreateSpecificTkWidget(app, "frame"))
    {
    vtkErrorMacro("Failed creating widget " << this->GetClassName());
    return;
    }

  // --------------------------------------------------------------
  // If in popup mode, create the popup button

  if (this->PopupMode)
    {
    if (!this->PopupButton)
      {
      this->PopupButton = vtkKWPopupButton::New();
      }
    
    this->PopupButton->SetParent(this);
    this->PopupButton->Create(app);
    }

  // --------------------------------------------------------------
  // Create the labeled frame

  if (this->PopupMode)
    {
    this->Frame->ShowHideFrameOff();
    this->Frame->SetParent(this->PopupButton->GetPopupFrame());
    }
  else
    {
    this->Frame->SetParent(this);
    }

  this->Frame->Create(app);

  this->Script("pack %s -side top -anchor nw -fill both -expand y",
               this->Frame->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkKWPopupFrame::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->PropagateEnableState(this->PopupButton);
  this->PropagateEnableState(this->Frame);
}

//----------------------------------------------------------------------------
void vtkKWPopupFrame::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Frame: " << this->Frame << endl;
  os << indent << "PopupMode: " 
     << (this->PopupMode ? "On" : "Off") << endl;
  os << indent << "PopupButton: " << this->PopupButton << endl;
}

