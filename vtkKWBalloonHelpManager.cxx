/*=========================================================================

  Module:    $RCSfile: vtkKWBalloonHelpManager.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWBalloonHelpManager.h"

#include "vtkKWLabel.h"
#include "vtkKWWidget.h"
#include "vtkKWTopLevel.h"
#include "vtkKWApplication.h"

#include "vtkObjectFactory.h"

#include <vtksys/stl/string>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWBalloonHelpManager );
vtkCxxRevisionMacro(vtkKWBalloonHelpManager, "$Revision: 1.7 $");

//----------------------------------------------------------------------------
vtkKWBalloonHelpManager::vtkKWBalloonHelpManager()
{
  this->CurrentWidget = NULL;
  this->AfterTimerId = NULL;

  this->TopLevel = NULL;
  this->Label = NULL;

  this->Delay = 1;
  this->Visibility = 1;
}

//----------------------------------------------------------------------------
vtkKWBalloonHelpManager::~vtkKWBalloonHelpManager()
{
  this->SetCurrentWidget(NULL);
  this->SetAfterTimerId(NULL);

  if (this->TopLevel)
    {
    this->TopLevel->Delete();
    this->TopLevel = NULL;
    }

  if (this->Label)
    {
    this->Label->Delete();
    this->Label = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkKWBalloonHelpManager::CreateBalloon()
{
  if (!this->TopLevel)
    {
    this->TopLevel = vtkKWTopLevel::New();
    }

  if (!this->Label)
    {
    this->Label = vtkKWLabel::New();
    }

  vtkKWApplication *app = this->GetApplication();
  if (!app && this->CurrentWidget)
    {
    app = this->CurrentWidget->GetApplication();
    }

  if (app)
    {
    if (!this->TopLevel->IsCreated())
      {
      this->TopLevel->HideDecorationOn();
      this->TopLevel->Create(app);
      this->TopLevel->SetBackgroundColor(0.0, 0.0, 0.0);
      this->TopLevel->SetBorderWidth(1);
      this->TopLevel->SetReliefToFlat();
      }

    if (!this->Label->IsCreated() && this->TopLevel)
      {
      this->Label->SetParent(this->TopLevel);    
      this->Label->Create(app);
      this->Label->SetBackgroundColor(1.0, 1.0, 0.88);
      this->Label->SetForegroundColor(0.0, 0.0, 0.0);
      this->Label->SetJustificationToLeft();
      this->Label->SetWrapLength("2i");
      app->Script("pack %s", this->Label->GetWidgetName());
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWBalloonHelpManager::TriggerCallback(vtkKWWidget *widget)
{
  if (!widget || !widget->IsCreated() || this->ApplicationInExit())
    {
    return;
    }

  // If there is no help string, do not bother

  if (!this->Visibility || !widget->GetBalloonHelpString() || this->Delay <= 0)
    {
    this->SetAfterTimerId(NULL);
    return;
    }

  // Cancel the previous balloon help, and setup the new one

  this->CancelCallback();

  this->SetCurrentWidget(widget);
  this->SetAfterTimerId(
    widget->Script("after %d {catch {%s DisplayCallback %s}}", 
                   this->Delay * 1000,
                   this->GetTclName(), widget->GetTclName()));
}

//----------------------------------------------------------------------------
void vtkKWBalloonHelpManager::CancelCallback()
{
  if (!this->GetApplication() || this->ApplicationInExit())
    {
    return;
    }

  // Cancel the previous timer
  
  if (this->AfterTimerId)
    {
    this->Script("after cancel %s", this->AfterTimerId);
    this->SetAfterTimerId(NULL);
    }
  
  this->SetCurrentWidget(NULL);

  // Hide the balloon help

  if (this->TopLevel)
    {
    this->TopLevel->Withdraw();
    }
}

//----------------------------------------------------------------------------
void vtkKWBalloonHelpManager::WithdrawCallback()
{
  if (!this->GetApplication() || this->ApplicationInExit())
    {
    return;
    }

  // Hide the balloon help
  
  if (this->TopLevel)
    {
    this->TopLevel->Withdraw();
    }

  // Re-schedule the balloon help for the current widget (if any)

  if (this->CurrentWidget)
    {
    this->TriggerCallback(this->CurrentWidget);
    }
}

//----------------------------------------------------------------------------
void vtkKWBalloonHelpManager::DisplayCallback(vtkKWWidget *widget)
{
  if (!this->GetApplication() || this->ApplicationInExit() ||
      !this->Visibility || !widget || !widget->IsCreated())
    {
    return;
    }

  // If there is no help string, return

  if (!widget->GetBalloonHelpString())
    {
    this->SetAfterTimerId(NULL);
    return;
    }

  // Create the UI, set the help string

  this->CreateBalloon();

  this->Label->SetText(widget->GetBalloonHelpString());

  // Get the position of the mouse

  int x = atoi(this->Script( "winfo pointerx %s", widget->GetWidgetName()));
  int y = atoi(this->Script( "winfo pointery %s", widget->GetWidgetName()));

  // Get the position in and size of the parent window of the one needing help

  int xw = atoi(
    this->Script( "winfo rootx %s", widget->GetParent()->GetWidgetName()));

  int dxw = atoi(
    this->Script( "winfo width %s", widget->GetParent()->GetWidgetName()));

  // Get the size of the balloon window

  int dx = atoi(
    this->Script("winfo reqwidth %s", this->Label->GetWidgetName()));
  
  // Set the position of the widget relative to the mouse
  // Try to keep the help from going past the right edge of the widget
  // If it goes too far right, move it to the left, 
  // but not past the left edge of the parent widget

  if (x + dx > xw + dxw)
    {
    x = xw + dxw - dx;
    if (x < xw)
      {
      x = xw;
      }
    }

  // Place the balloon

  this->TopLevel->SetPosition(x, y + 15);
  this->Script("update");

  // Map the balloon

  if (this->AfterTimerId)
    {
    this->TopLevel->DeIconify();
    this->TopLevel->Raise();
    }
  
  this->SetAfterTimerId(NULL);
}

//----------------------------------------------------------------------------
void vtkKWBalloonHelpManager::SetVisibility(int v)
{
  if (this->Visibility == v)
    {
    return;
    }

  this->Visibility = v;
  this->Modified();

  if (!this->Visibility)
    {
    this->CancelCallback();
    }
}

//----------------------------------------------------------------------------
void vtkKWBalloonHelpManager::SetCurrentWidget(vtkKWWidget *widget)
{
  if (this->CurrentWidget == widget || 
      (this->ApplicationInExit() && widget))
    {
    return;
    }

#if 0
  if (this->CurrentWidget)
    {
    this->CurrentWidget->UnRegister(this);
    this->CurrentWidget = NULL;
    }

  if (widget)
    {
    this->CurrentWidget = widget;
    this->CurrentWidget->Register(this);
    }
#else
  this->CurrentWidget = widget;
#endif
 

  this->Modified();
}

//----------------------------------------------------------------------------
int vtkKWBalloonHelpManager::ApplicationInExit()
{
  vtkKWApplication *app = this->GetApplication();
  if (!app && this->CurrentWidget)
    {
    app = this->CurrentWidget->GetApplication();
    }
  
  return (app && app->GetInExit()) ? 1 : 0;
}

//----------------------------------------------------------------------------
void vtkKWBalloonHelpManager::AddBindings(vtkKWWidget *widget)
{
  if (!widget || !widget->IsCreated())
    {
    return;
    }

  const char *enter_bindings = 
    widget->Script("bind %s <Enter>", widget->GetWidgetName());
  if (strstr(enter_bindings, "TriggerCallback"))
    {
    return;
    }

  vtksys_stl::string command("TriggerCallback ");
  command += widget->GetTclName();
  widget->AddBinding("<Enter>", this, command.c_str());

  widget->AddBinding("<ButtonPress>", this, "WithdrawCallback");
  widget->AddBinding("<KeyPress>", this, "WithdrawCallback");
  widget->AddBinding("<B1-Motion>", this, "WithdrawCallback");
  widget->AddBinding("<Leave>", this, "CancelCallback");
}

//----------------------------------------------------------------------------
void vtkKWBalloonHelpManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Visibility: " << (this->Visibility ? "on":"off") << endl;
  os << indent << "Delay: " << this->GetDelay() << endl;
}
