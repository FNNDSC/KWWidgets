/*=========================================================================

  Module:    $RCSfile: vtkKWPopupButton.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWPopupButton.h"

#include "vtkKWApplication.h"
#include "vtkKWLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWPushButton.h"
#include "vtkObjectFactory.h"
#include "vtkKWTopLevel.h"

#include <vtksys/SystemTools.hxx>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWPopupButton);
vtkCxxRevisionMacro(vtkKWPopupButton, "$Revision: 1.26 $");

//----------------------------------------------------------------------------
vtkKWPopupButton::vtkKWPopupButton()
{
  this->PopupTopLevel = vtkKWTopLevel::New();

  this->PopupFrame = vtkKWFrame::New();

  this->PopupCloseButton = vtkKWPushButton::New();

  this->WithdrawCommand = 0;
}

//----------------------------------------------------------------------------
vtkKWPopupButton::~vtkKWPopupButton()
{
  if (this->PopupTopLevel)
    {
    this->PopupTopLevel->Delete();
    this->PopupTopLevel = NULL;
    }

  if (this->PopupFrame)
    {
    this->PopupFrame->Delete();
    this->PopupFrame = NULL;
    }

  if (this->PopupCloseButton)
    {
    this->PopupCloseButton->Delete();
    this->PopupCloseButton = NULL;
    }

  this->SetWithdrawCommand(0);
}

//----------------------------------------------------------------------------
void vtkKWPopupButton::Create(vtkKWApplication *app)
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro("PopupButton already created");
    return;
    }

  // Call the superclass, this will set the application and 
  // create the pushbutton.

  this->Superclass::Create(app);

  ostrstream tk_cmd;

  // Create  top level window

  this->PopupTopLevel->SetMasterWindow(this);
  this->PopupTopLevel->Create(app);
  this->PopupTopLevel->SetBorderWidth(2);
  this->PopupTopLevel->SetReliefToFlat();
  this->PopupTopLevel->Withdraw();

  if (!this->PopupTopLevel->GetTitle())
    {
    this->PopupTopLevel->SetTitle(
      this->Script("wm title [winfo toplevel %s]", this->GetWidgetName()));
    }

  this->PopupTopLevel->SetDeleteWindowProtocolCommand(
    this, "WithdrawPopupCallback");

  // Create the frame

  this->PopupFrame->SetParent(PopupTopLevel);
  this->PopupFrame->Create(app);
  this->PopupFrame->SetBorderWidth(2);

  tk_cmd << "pack " << this->PopupFrame->GetWidgetName() 
         << " -side top -expand y -fill both" << endl;

  // Create the close button

  this->PopupCloseButton->SetParent(PopupTopLevel);
  this->PopupCloseButton->Create(app);
  this->PopupCloseButton->SetText("Close");

  tk_cmd << "pack " << this->PopupCloseButton->GetWidgetName() 
         << " -side top -expand false -fill x -pady 2" << endl;

  // Pack, bind

  tk_cmd << ends;
  this->Script(tk_cmd.str());
  tk_cmd.rdbuf()->freeze(0);
  
  this->Bind();

  // Update enable state

  this->UpdateEnableState();
}

//----------------------------------------------------------------------------
void vtkKWPopupButton::Bind()
{
  if (!this->IsCreated())
    {
    return;
    }

  // Set the button so that it popups the top level window

  this->SetBinding("<ButtonPress>", this, "DisplayPopupCallback");

  if (this->PopupCloseButton)
    {
    this->PopupCloseButton->SetBinding(
      "<ButtonPress>", this, "WithdrawPopupCallback");
    }
}

//----------------------------------------------------------------------------
void vtkKWPopupButton::UnBind()
{
  if (!this->IsCreated())
    {
    return;
    }

  this->RemoveBinding("<ButtonPress>");

  if (this->PopupCloseButton && this->PopupCloseButton->IsCreated())
    {
    this->PopupCloseButton->RemoveBinding("<ButtonPress>");
    }
}

// ---------------------------------------------------------------------------
void vtkKWPopupButton::SetPopupTitle(const char* title)
{
  this->PopupTopLevel->SetTitle(title);
}

// ---------------------------------------------------------------------------
void vtkKWPopupButton::DisplayPopupCallback()
{
  if (!this->IsCreated())
    {
    return;
    }

  // Get the position of the mouse, the size of the top level window.

  const char *res = 
    this->Script("concat "
                 " [winfo pointerx %s] [winfo pointery %s]" 
                 " [winfo reqwidth %s] [winfo reqheight %s]"
                 " [winfo screenwidth %s] [winfo screenheight %s]",
                 this->GetWidgetName(), 
                 this->GetWidgetName(),
                 this->PopupTopLevel->GetWidgetName(), 
                 this->PopupTopLevel->GetWidgetName(),
                 this->GetWidgetName(), 
                 this->GetWidgetName());
  
  int px, py, tw, th, sw, sh;
  sscanf(res, "%d %d %d %d %d %d", &px, &py, &tw, &th, &sw, &sh);

  px -= tw / 2;
  if (px + tw > sw)
    {
    px -= (px + tw - sw);
    }
  if (px < 0)
    {
    px = 0;
    }

  py -= th / 2;
  if (py + th > sh)
    {
    py -= (py + th - sh);
    }
  if (py < 0)
    {
    py = 0;
    }

  this->PopupTopLevel->SetPosition(px, py);
  this->PopupTopLevel->DeIconify();
  this->PopupTopLevel->Raise();
}

// ---------------------------------------------------------------------------
void vtkKWPopupButton::WithdrawPopupCallback()
{
  if ( this->GetApplication()->IsDialogUp() )
    {
    this->Script("bell");
    return;
    }
  if (!this->IsCreated())
    {
    return;
    }

  this->PopupTopLevel->Withdraw();
  if ( this->WithdrawCommand )
    {
    this->Script(this->WithdrawCommand);
    }
}

//----------------------------------------------------------------------------
void vtkKWPopupButton::SetWithdrawCommand(vtkObject* obj, const char* method)
{
  char *command = NULL;
  this->SetObjectMethodCommand(&command, obj, method);
  this->SetWithdrawCommand(command);
  delete [] command;
}

//----------------------------------------------------------------------------
void vtkKWPopupButton::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->PropagateEnableState(this->PopupTopLevel);
  this->PropagateEnableState(this->PopupFrame);
  this->PropagateEnableState(this->PopupCloseButton);

  // Now given the state, bind or unbind

  if (this->IsCreated())
    {
    if (this->GetEnabled())
      {
      this->Bind();
      }
    else
      {
      this->UnBind();
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWPopupButton::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "PopupTopLevel: " << this->PopupTopLevel << endl;
  os << indent << "PopupFrame: " << this->PopupFrame << endl;
  os << indent << "PopupCloseButton: " << this->PopupCloseButton << endl;
  os << indent << "WithdrawCommand: "
     << (this->WithdrawCommand ? this->WithdrawCommand : "(none)") << endl;
}

