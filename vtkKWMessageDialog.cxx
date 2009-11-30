/*=========================================================================

  Module:    $RCSfile: vtkKWMessageDialog.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWMessageDialog.h"

#include "vtkKWApplication.h"
#include "vtkKWCheckButton.h"
#include "vtkKWEvent.h"
#include "vtkKWFrame.h"
#include "vtkKWIcon.h"
#include "vtkKWInternationalization.h"
#include "vtkKWLabel.h"
#include "vtkKWMessage.h"
#include "vtkKWPushButton.h"
#include "vtkKWRegistryHelper.h"
#include "vtkObjectFactory.h"

#include <vtksys/stl/string>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWMessageDialog );
vtkCxxRevisionMacro(vtkKWMessageDialog, "1.93");

//----------------------------------------------------------------------------
vtkKWMessageDialog::vtkKWMessageDialog()
{
  this->MessageDialogFrame = vtkKWFrame::New();
  this->TopFrame           = vtkKWFrame::New();
  this->BottomFrame        = vtkKWFrame::New();
  this->Message            = vtkKWMessage::New();
  this->CheckButton        = vtkKWCheckButton::New();
  this->ButtonFrame        = vtkKWFrame::New();
  this->OKFrame            = vtkKWFrame::New();
  this->CancelFrame        = vtkKWFrame::New();
  this->OtherFrame         = vtkKWFrame::New();
  this->OKButton           = vtkKWPushButton::New();
  this->CancelButton       = vtkKWPushButton::New();
  this->OtherButton        = vtkKWPushButton::New();
  this->Style              = vtkKWMessageDialog::StyleMessage;
  this->Icon               = vtkKWLabel::New();

  this->DialogName = 0;
  this->Options    = 0;
  this->DialogText = 0;

  this->OKButtonText = 0;
  this->SetOKButtonText(ks_("Message Dialog|Button|OK"));

  this->CancelButtonText = 0;
  this->SetCancelButtonText(ks_("Message Dialog|Button|Cancel"));

  this->OtherButtonText = 0;
  this->SetOtherButtonText(ks_("Message Dialog|Button|Other"));
}

//----------------------------------------------------------------------------
vtkKWMessageDialog::~vtkKWMessageDialog()
{
  this->Message->Delete();
  this->CheckButton->Delete();
  this->ButtonFrame->Delete();
  this->OKFrame->Delete();
  this->CancelFrame->Delete();
  this->OtherFrame->Delete();
  this->OKButton->Delete();
  this->CancelButton->Delete();
  this->OtherButton->Delete();
  this->Icon->Delete();
  this->MessageDialogFrame->Delete();
  this->TopFrame->Delete();
  this->BottomFrame->Delete();
  this->SetDialogName(0);
  this->SetDialogText(0);
  this->SetOKButtonText(0);
  this->SetCancelButtonText(0);
  this->SetOtherButtonText(0);
}

//----------------------------------------------------------------------------
void vtkKWMessageDialog::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro("MessageDialog already created");
    return;
    }

  this->Superclass::CreateWidget();
  
  this->MessageDialogFrame->SetParent(this);
  this->MessageDialogFrame->Create();

  this->Script("pack %s -side right -fill both -expand true -pady 0",
               this->MessageDialogFrame->GetWidgetName());

  if (this->Options & vtkKWMessageDialog::Beep)
    {
    this->BeepOn();
    }

  // Top

  this->TopFrame->SetParent(this->MessageDialogFrame);
  this->TopFrame->Create();

  // The message itself

  this->Message->SetParent(this->MessageDialogFrame);
  this->Message->Create();
  this->Message->SetWidth(300);

  this->UpdateMessage();

  // The checkbutton

  this->CheckButton->SetParent(this->MessageDialogFrame);
  this->CheckButton->Create();

  // Bottom frame

  this->BottomFrame->SetParent(this->MessageDialogFrame);
  this->BottomFrame->Create();

  // Button frame

  this->ButtonFrame->SetParent(this->MessageDialogFrame);
  this->ButtonFrame->Create();
  
  this->OKFrame->SetParent(this->ButtonFrame);
  this->OKFrame->Create();
  this->OKFrame->SetBorderWidth(3);
  this->OKFrame->SetReliefToFlat();
  this->OKButton->SetParent(this->OKFrame);
  this->OKButton->Create();
  this->OKButton->SetWidth(16);
  this->OKButton->SetText(this->OKButtonText);
  this->OKButton->SetCommand(this, "OK");
  this->OKButton->AddBinding(
    "<FocusIn>", this->OKFrame, "SetReliefToGroove");
  this->OKButton->AddBinding(
    "<FocusOut>", this->OKFrame, "SetReliefToFlat");
  this->OKButton->AddBinding(
    "<Return>", this, "OK");

  this->OtherFrame->SetParent(this->ButtonFrame);  
  this->OtherFrame->Create();
  this->OtherFrame->SetBorderWidth(3);
  this->OtherFrame->SetReliefToFlat();
  this->OtherButton->SetParent(this->OtherFrame);
  this->OtherButton->Create();
  this->OtherButton->SetWidth(16);
  this->OtherButton->SetText(this->OtherButtonText);
  this->OtherButton->SetCommand(this, "Other");
  this->OtherButton->AddBinding(
    "<FocusIn>", this->OtherFrame, "SetReliefToGroove");
  this->OtherButton->AddBinding(
    "<FocusOut>", this->OtherFrame, "SetReliefToFlat");
  this->OtherButton->AddBinding(
    "<Return>", this, "Other");

  this->CancelFrame->SetParent(this->ButtonFrame);  
  this->CancelFrame->Create();
  this->CancelFrame->SetBorderWidth(3);
  this->CancelFrame->SetReliefToFlat();
  this->CancelButton->SetParent(this->CancelFrame);
  this->CancelButton->Create();
  this->CancelButton->SetWidth(16);
  this->CancelButton->SetText(this->CancelButtonText);
  this->CancelButton->SetCommand(this, "Cancel");
  this->CancelButton->AddBinding(
    "<FocusIn>", this->CancelFrame, "SetReliefToGroove");
  this->CancelButton->AddBinding(
    "<FocusOut>", this->CancelFrame, "SetReliefToFlat");
  this->CancelButton->AddBinding(
    "<Return>", this, "Cancel");

  this->UpdateButtons();

  // Icon
  
  this->Icon->SetParent(this);
  this->Icon->Create();
  this->Icon->SetWidth(0);
  this->Icon->SetPadX(0);
  this->Icon->SetPadY(0);
  this->Icon->SetBorderWidth(0);

  this->Script("pack %s -side left -fill y", this->Icon->GetWidgetName());
  this->Script("pack forget %s", this->Icon->GetWidgetName());

  this->SetIcon();

  // Pack

  this->Pack();
}

//----------------------------------------------------------------------------
void vtkKWMessageDialog::Pack()
{
  if (!this->IsCreated())
    {
    return;
    }

  if (!this->MessageDialogFrame || !this->MessageDialogFrame->IsCreated())
    {
    return;
    }

  this->MessageDialogFrame->UnpackChildren();

  if (this->TopFrame->GetNumberOfChildren())
    {
    this->Script("pack %s -side top -fill both -expand t",
                 this->TopFrame->GetWidgetName());
    }

  if (this->Message && this->DialogText && *this->DialogText)
    {
    this->Script("pack %s -side top -fill x -padx 20 -pady 5 -expand t",
                 this->Message->GetWidgetName());
    }

  if (this->BottomFrame->GetNumberOfChildren())
    {
    this->Script("pack %s -side top -fill both -expand true",
                 this->BottomFrame->GetWidgetName());
    }

  if (this->CheckButton && this->GetDialogName())
    {
    this->CheckButton->SetText(
      ks_("Message Dialog|Do not show this dialog anymore."));
    this->Script("pack %s -side top -fill x -padx 20 -pady 5",
                 this->CheckButton->GetWidgetName());
    }

  this->Script("pack %s -side bottom -fill x -pady 2 -expand f",
               this->ButtonFrame->GetWidgetName());

  this->PackButtons();
}

//----------------------------------------------------------------------------
void vtkKWMessageDialog::UpdateButtons()
{
  if (this->Style == vtkKWMessageDialog::StyleYesNo)
    {
    this->SetOKButtonText(ks_("Message Dialog|Button|Yes"));
    if (this->OKButton)
      {
      this->OKButton->SetText(this->OKButtonText);
      }
    this->SetCancelButtonText(ks_("Message Dialog|Button|No"));
    if (this->CancelButton)
      {
      this->CancelButton->SetText(this->CancelButtonText);
      }
    }
  else if (this->Style == vtkKWMessageDialog::StyleMessage)
    {
    this->SetOKButtonText(ks_("Message Dialog|Button|OK"));
    if (this->OKButton)
      {
      this->OKButton->SetText(this->OKButtonText);
      }
    }
  else if (this->Style == vtkKWMessageDialog::StyleCancel)
    {
    this->SetCancelButtonText(ks_("Message Dialog|Button|Cancel"));
    if (this->CancelButton)
      {
      this->CancelButton->SetText(this->CancelButtonText);
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWMessageDialog::PackButtons()
{
  int has_ok = (this->Style == vtkKWMessageDialog::StyleMessage ||
                this->Style == vtkKWMessageDialog::StyleYesNo ||
                this->Style == vtkKWMessageDialog::StyleOkCancel ||
                this->Style == vtkKWMessageDialog::StyleOkOtherCancel);
  int has_cancel = (this->Style == vtkKWMessageDialog::StyleCancel ||
                    this->Style == vtkKWMessageDialog::StyleYesNo ||
                    this->Style == vtkKWMessageDialog::StyleOkCancel ||
                    this->Style == vtkKWMessageDialog::StyleOkOtherCancel);
  int has_other = (this->Style == vtkKWMessageDialog::StyleOkOtherCancel);

  // Pack buttons

  if (this->ButtonFrame && this->ButtonFrame->IsCreated())
    {
    this->ButtonFrame->UnpackChildren();
    }

  vtksys_stl::string pack_opt;
  if (this->Options & vtkKWMessageDialog::PackVertically)
    {
    pack_opt = "-side top -expand yes -fill x -padx 4";
    }
  else
    {
    pack_opt = "-side left -expand yes -padx 2";
    }

  if (has_ok && this->OKButton && this->OKButton->IsCreated())
    {
    this->Script("pack %s %s %s",
                 this->OKButton->GetWidgetName(),
                 this->OKFrame->GetWidgetName(), pack_opt.c_str());
    }

  if (has_other && this->OtherButton && this->OtherButton->IsCreated())
    {
    this->Script("pack %s %s %s",
                 this->OtherButton->GetWidgetName(),
                 this->OtherFrame->GetWidgetName(), pack_opt.c_str());
    }

  if (has_cancel && this->CancelButton && this->CancelButton->IsCreated())
    {
    this->Script("pack %s %s %s",
                 this->CancelButton->GetWidgetName(),
                 this->CancelFrame->GetWidgetName(), pack_opt.c_str());
    }
}

//----------------------------------------------------------------------------
void vtkKWMessageDialog::UpdateMessage()
{
  if (this->Message)
    {
    this->Message->SetText(this->DialogText);
    }
}

//----------------------------------------------------------------------------
void vtkKWMessageDialog::SetStyle(int arg)
{
  if (this->Style == arg)
    {
    return;
    }

  this->Style = arg;
  this->Modified();

  this->UpdateButtons();
  this->PackButtons();
}

void vtkKWMessageDialog::SetStyleToMessage() 
{ 
  this->SetStyle(vtkKWMessageDialog::StyleMessage); 
}

void vtkKWMessageDialog::SetStyleToYesNo() 
{ 
  this->SetStyle(vtkKWMessageDialog::StyleYesNo); 
}

void vtkKWMessageDialog::SetStyleToOkCancel() 
{ 
  this->SetStyle(vtkKWMessageDialog::StyleOkCancel); 
}

void vtkKWMessageDialog::SetStyleToOkOtherCancel() 
{ 
  this->SetStyle(vtkKWMessageDialog::StyleOkOtherCancel); 
}

void vtkKWMessageDialog::SetStyleToCancel() 
{ 
  this->SetStyle(vtkKWMessageDialog::StyleCancel); 
}

//----------------------------------------------------------------------------
void vtkKWMessageDialog::SetText(const char *txt)
{
  this->SetDialogText(txt);
  this->UpdateMessage();
  this->Pack();
}

//----------------------------------------------------------------------------
void vtkKWMessageDialog::SetTextWidth(int w)
{
  if (this->Message)
    {
    this->Message->SetWidth(w);
    }
}

//----------------------------------------------------------------------------
int vtkKWMessageDialog::GetTextWidth()
{
  if (this->Message)
    {
    return this->Message->GetWidth();
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWMessageDialog::Display()
{
  // Make sure we pack correctly depending on the frame's children

  this->Pack(); 

  if (this->Options & vtkKWMessageDialog::NoDefault ||
      this->Options & vtkKWMessageDialog::CancelDefault)
    {
    this->CancelButton->Focus();
    } 
  else if (this->Options & vtkKWMessageDialog::YesDefault ||
           this->Options & vtkKWMessageDialog::OkDefault)
    {
    this->OKButton->Focus();
    }
  if (this->OKButton->IsCreated() && this->CancelButton->IsCreated())
    {
    this->OKButton->SetBinding("<Right>", "focus [ tk_focusNext %W ]");
    this->OKButton->SetBinding("<Left>",  "focus [ tk_focusPrev %W ]");
    this->CancelButton->SetBinding("<Right>", "focus [ tk_focusNext %W ]");
    this->CancelButton->SetBinding("<Left>",  "focus [ tk_focusPrev %W ]");
    if (this->OtherButton->IsCreated())
      {
      this->OtherButton->SetBinding("<Right>", "focus [ tk_focusNext %W ]");
      this->OtherButton->SetBinding("<Left>",  "focus [ tk_focusPrev %W ]");
      }
    }
  if (this->Options & vtkKWMessageDialog::InvokeAtPointer)
    {
    this->SetDisplayPositionToPointer();
    }

  if (!(this->Options & vtkKWMessageDialog::Resizable))
    {
    this->SetResizable(0, 0);
    }

  // Display

  this->Superclass::Display();
}

//----------------------------------------------------------------------------
int vtkKWMessageDialog::PreInvoke()
{
  this->InvokeEvent(vtkKWEvent::MessageDialogInvokeEvent, this->DialogText);

  // Check if the user specified a default answer for this one, stored
  // in the registry

  if (this->DialogName)
    {
    int res = this->RestoreMessageDialogResponseFromRegistry(
      this->GetApplication(), this->DialogName);
    if (res == 1) 
      {
      this->Done = vtkKWDialog::StatusOK;
      return 1;
      }
    if (res == -1)
      {
      this->Done = vtkKWDialog::StatusCanceled;
      return 1;
      }
    }

  return this->Superclass::PreInvoke();
}

//----------------------------------------------------------------------------
void vtkKWMessageDialog::PostInvoke()
{
  this->Superclass::PostInvoke();

  // Check if the user specified a default answer for this one, and store it
  // in the registry

  if (this->DialogName && this->GetRememberMessage())
    {
    int ires = (this->Done == vtkKWDialog::StatusCanceled ? 0 : 1);
    if (this->Options & vtkKWMessageDialog::RememberYes)
      {
      ires = 1;
      }
    else if (this->Options & vtkKWMessageDialog::RememberNo)
      {
      ires = -1;
      }
    else
      {
      if (!ires)
        {
        ires = -1;
        }
      }
    this->SaveMessageDialogResponseToRegistry(
      this->GetApplication(), this->DialogName, ires);
    }
}

//----------------------------------------------------------------------------
void vtkKWMessageDialog::SetIcon()
{
  if (this->Options & vtkKWMessageDialog::ErrorIcon)
    {
    this->Icon->SetImageToPredefinedIcon(
      vtkKWIcon::IconNuvola48x48ActionsMessageBoxCritical);
    }
  else if (this->Options & vtkKWMessageDialog::QuestionIcon)
    {
    this->Icon->SetImageToPredefinedIcon(
      vtkKWIcon::IconNuvola48x48ActionsHelp);
    }
  else if (this->Options & vtkKWMessageDialog::WarningIcon)
    {
    this->Icon->SetImageToPredefinedIcon(
      vtkKWIcon::IconNuvola48x48ActionsMessageBoxWarning);
    }
  else if (!(this->Options & vtkKWMessageDialog::CustomIcon))
    {
    this->Icon->SetWidth(0);
    this->Icon->SetPadY(0);
    this->Icon->SetPadX(0);
    this->Icon->SetBorderWidth(0);
    this->Script("pack forget %s", this->Icon->GetWidgetName());
    return;
    }  
  
  this->Icon->SetAnchorToNorth();
  this->Icon->SetPadY(5);
  this->Icon->SetPadX(4);
  this->Icon->SetBorderWidth(4);

  this->Script("pack %s -pady 17 -side left -fill y", 
               this->Icon->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkKWMessageDialog::PopupMessage(vtkKWApplication *app, 
                                      vtkKWWidget *win,
                                      const char* title, 
                                      const char*message, int options)
{
  vtkKWMessageDialog *dlg2 = vtkKWMessageDialog::New();
  dlg2->SetApplication(app);
  dlg2->SetMasterWindow(win);
  dlg2->SetOptions(
    options | vtkKWMessageDialog::Beep | vtkKWMessageDialog::YesDefault);
  dlg2->SetTitle(title);
  dlg2->SetText(message);
  dlg2->Invoke();
  dlg2->Delete();
}

//----------------------------------------------------------------------------
int vtkKWMessageDialog::PopupYesNo(vtkKWApplication *app, 
                                   vtkKWWidget *win,
                                   const char* name, 
                                   const char* title, const char* message,
                                   int options)
{
  vtkKWMessageDialog *dlg2 = vtkKWMessageDialog::New();
  dlg2->SetApplication(app);
  dlg2->SetStyleToYesNo();
  dlg2->SetMasterWindow(win);
  dlg2->SetOptions(
    options | vtkKWMessageDialog::Beep | vtkKWMessageDialog::YesDefault);
  dlg2->SetDialogName(name);
  dlg2->SetTitle(title);
  dlg2->SetText(message);
  int ret = dlg2->Invoke();
  dlg2->Delete();
  return ret;
}

//----------------------------------------------------------------------------
int vtkKWMessageDialog::PopupYesNo(vtkKWApplication *app, 
                                   vtkKWWidget *win,
                                   const char* title, 
                                   const char*message, int options)
{
  return vtkKWMessageDialog::PopupYesNo(app, win, 0, title, message, 
                                        options);
}

//----------------------------------------------------------------------------
int vtkKWMessageDialog::PopupOkCancel(vtkKWApplication *app, 
                                      vtkKWWidget *win,
                                      const char* title, 
                                      const char*message, int options)
{
  vtkKWMessageDialog *dlg2 = vtkKWMessageDialog::New();
  dlg2->SetApplication(app);
  dlg2->SetStyleToOkCancel();
  dlg2->SetOptions(
    options | vtkKWMessageDialog::Beep | vtkKWMessageDialog::YesDefault);
  dlg2->SetMasterWindow(win);
  dlg2->SetTitle(title);
  dlg2->SetText(message);
  int ret = dlg2->Invoke();
  dlg2->Delete();
  return ret;
}

//----------------------------------------------------------------------------
int vtkKWMessageDialog::GetRememberMessage()
{
  int res = this->CheckButton->GetSelectedState();
  return res;
}

//----------------------------------------------------------------------------
void vtkKWMessageDialog::Other()
{
  this->Done = vtkKWMessageDialog::StatusOther;
  this->Withdraw();
}

//----------------------------------------------------------------------------
int vtkKWMessageDialog::RestoreMessageDialogResponseFromRegistry(
  vtkKWApplication *app,
  const char* dialogname)
{
  char buffer[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  int retval = 0;
  if (app && dialogname && 
      app->GetRegistryValue(3, "Dialogs", dialogname, buffer))
    {
    retval = atoi(buffer);
    }
  return retval;
}

//----------------------------------------------------------------------------
void vtkKWMessageDialog::SaveMessageDialogResponseToRegistry(
  vtkKWApplication *app,
  const char* dialogname, 
  int response)
{
  if (app && dialogname)
    {
    app->SetRegistryValue(3, "Dialogs", dialogname, "%d", response);
    }
}

//----------------------------------------------------------------------------
void vtkKWMessageDialog::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "DialogName: " << (this->DialogName?this->DialogName:"none")
     << endl;
  os << indent << "Options: " << this->GetOptions() << endl;
  os << indent << "Style: " << this->GetStyle() << endl;
  os << indent << "MessageDialogFrame: " << this->MessageDialogFrame << endl;
  os << indent << "OKButtonText: " << (this->OKButtonText?
                                       this->OKButtonText:"none") << endl;
  os << indent << "CancelButtonText: " << (this->CancelButtonText?
                                       this->CancelButtonText:"none") << endl;
  os << indent << "OtherButtonText: " << (this->OtherButtonText?
                                       this->OtherButtonText:"none") << endl;
  os << indent << "DialogName: " << (this->DialogName?this->DialogName:"none")
     << endl;
  os << indent << "TopFrame: " << this->TopFrame << endl;
  os << indent << "BottomFrame: " << this->BottomFrame << endl;
  os << indent << "OKButton: " << this->OKButton << endl;
  os << indent << "CancelButton: " << this->CancelButton << endl;
  os << indent << "OtherButton: " << this->OtherButton << endl;
}
