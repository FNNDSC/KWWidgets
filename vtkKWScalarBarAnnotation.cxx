/*=========================================================================

Copyright (c) 1998-2003 Kitware Inc. 469 Clifton Corporate Parkway,
Clifton Park, NY, 12065, USA.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither the name of Kitware nor the names of any contributors may be used
   to endorse or promote products derived from this software without specific
   prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "vtkKWScalarBarAnnotation.h"

#include "vtkKWCheckButton.h"
#include "vtkKWEntry.h"
#include "vtkKWEvent.h"
#include "vtkKWFrame.h"
#include "vtkKWLabel.h"
#include "vtkKWLabeledEntry.h"
#include "vtkKWLabeledFrame.h"
#include "vtkKWLabeledPopupButton.h"
#include "vtkKWPopupButton.h"
#include "vtkKWScale.h"
#include "vtkKWTextProperty.h"
#include "vtkKWThumbWheel.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkScalarBarActor.h"
#include "vtkScalarBarWidget.h"
#include "vtkTextProperty.h"
#ifndef DO_NOT_BUILD_XML_RW
#include "vtkXMLScalarBarWidgetWriter.h"
#endif

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWScalarBarAnnotation );
vtkCxxRevisionMacro(vtkKWScalarBarAnnotation, "$Revision: 1.1 $");

int vtkKWScalarBarAnnotationCommand(ClientData cd, Tcl_Interp *interp,
                                    int argc, char *argv[]);

//----------------------------------------------------------------------------
vtkKWScalarBarAnnotation::vtkKWScalarBarAnnotation()
{
  this->CommandFunction = vtkKWScalarBarAnnotationCommand;

  this->AnnotationChangedEvent  = vtkKWEvent::ViewAnnotationChangedEvent;
  this->PopupTextProperty       = 0;
  this->ScalarBarWidget         = NULL;

  // GUI

  this->TitleFrame                      = vtkKWFrame::New();
  this->TitleEntry                      = vtkKWLabeledEntry::New();
  this->TitleTextPropertyWidget         = vtkKWTextProperty::New();
  this->TitleTextPropertyPopupButton    = NULL;

  this->LabelFrame                      = vtkKWFrame::New();
  this->LabelFormatEntry                = vtkKWLabeledEntry::New();
  this->LabelTextPropertyWidget         = vtkKWTextProperty::New();
  this->LabelTextPropertyPopupButton    = NULL;

  this->MaximumNumberOfColorsThumbWheel = vtkKWThumbWheel::New();
  this->NumberOfLabelsScale             = vtkKWScale::New();
}

//----------------------------------------------------------------------------
vtkKWScalarBarAnnotation::~vtkKWScalarBarAnnotation()
{
  // GUI

  if (this->TitleFrame)
    {
    this->TitleFrame->Delete();
    this->TitleFrame = NULL;
    }

  if (this->TitleEntry)
    {
    this->TitleEntry->Delete();
    this->TitleEntry = NULL;
    }

  if (this->TitleTextPropertyWidget)
    {
    this->TitleTextPropertyWidget->Delete();
    this->TitleTextPropertyWidget = NULL;
    }

  if (this->TitleTextPropertyPopupButton)
    {
    this->TitleTextPropertyPopupButton->Delete();
    this->TitleTextPropertyPopupButton = NULL;
    }

  if (this->LabelFrame)
    {
    this->LabelFrame->Delete();
    this->LabelFrame = NULL;
    }

  if (this->LabelFormatEntry)
    {
    this->LabelFormatEntry->Delete();
    this->LabelFormatEntry = NULL;
    }

  if (this->LabelTextPropertyWidget)
    {
    this->LabelTextPropertyWidget->Delete();
    this->LabelTextPropertyWidget = NULL;
    }

  if (this->LabelTextPropertyPopupButton)
    {
    this->LabelTextPropertyPopupButton->Delete();
    this->LabelTextPropertyPopupButton = NULL;
    }

  if (this->MaximumNumberOfColorsThumbWheel)
    {
    this->MaximumNumberOfColorsThumbWheel->Delete();
    this->MaximumNumberOfColorsThumbWheel = NULL;
    }

  if (this->NumberOfLabelsScale)
    {
    this->NumberOfLabelsScale->Delete();
    this->NumberOfLabelsScale = NULL;
    }

  this->SetScalarBarWidget(NULL);
}

//----------------------------------------------------------------------------
void vtkKWScalarBarAnnotation::SetScalarBarWidget(vtkScalarBarWidget *_arg)
{ 
  if (this->ScalarBarWidget == _arg) 
    {
    return;
    }

  if (this->ScalarBarWidget != NULL) 
    { 
    this->ScalarBarWidget->UnRegister(this); 
    }

  this->ScalarBarWidget = _arg;

  if (this->ScalarBarWidget != NULL) 
    { 
    this->ScalarBarWidget->Register(this); 
    }

  this->Modified();

  // Update the GUI. Test if it is alive because we might be in the middle
  // of destructing the whole GUI

  if (this->IsAlive())
    {
    this->Update();
    }
} 

//----------------------------------------------------------------------------
void vtkKWScalarBarAnnotation::Create(vtkKWApplication *app, 
                                   const char *args)
{
  // Create the superclass widgets

  if (this->IsCreated())
    {
    vtkErrorMacro("ScalarBarAnnotation already created");
    return;
    }

  this->Superclass::Create(app, args);

  int popup_text_property = 
    this->PopupTextProperty && !this->PopupMode;

  vtkKWWidget *frame = this->Frame->GetFrame();

  // --------------------------------------------------------------
  // If in popup mode, modify the popup button

  if (this->PopupMode)
    {
    this->PopupButton->SetLabel("Edit...");
    }

  // --------------------------------------------------------------
  // Edit the labeled frame

  this->Frame->SetLabel("Scalar Bar");

  // --------------------------------------------------------------
  // Edit the check button (Annotation visibility)

  this->CheckButton->SetText("Display scalar bar");

  this->CheckButton->SetBalloonHelpString(
    "Toggle the visibility of the scalar bar representing the mapping "
    "of scalar value to RGB color");

  // --------------------------------------------------------------
  // Title frame

  this->TitleFrame->SetParent(frame);
  this->TitleFrame->Create(app, 0);

  this->Script("pack %s -side top -fill both -expand y", 
               this->TitleFrame->GetWidgetName());
  
  // --------------------------------------------------------------
  // Scalar Bar title

  this->TitleEntry->SetParent(this->TitleFrame);
  this->TitleEntry->Create(app, 0);
  this->TitleEntry->SetLabel("Title:");
  this->TitleEntry->GetEntry()->BindCommand(this, "ScalarBarTitleCallback");

  this->TitleEntry->SetBalloonHelpString(
    "Set the scalar bar title. The text will automatically scale "
    "to fit within the allocated space");

  this->Script("pack %s -padx 2 -pady 2 -side %s -anchor nw -expand y -fill x",
               this->TitleEntry->GetWidgetName(),
               (!this->PopupMode ? "left" : "top"));
  
  // --------------------------------------------------------------
  // Scalar Bar title text property : popup button if needed

  if (popup_text_property)
    {
    if (!this->TitleTextPropertyPopupButton)
      {
      this->TitleTextPropertyPopupButton = vtkKWLabeledPopupButton::New();
      }
    this->TitleTextPropertyPopupButton->SetParent(this->TitleFrame);
    this->TitleTextPropertyPopupButton->Create(this->Application);
    this->TitleTextPropertyPopupButton->SetLabel("Title properties:");
    this->TitleTextPropertyPopupButton->SetPopupButtonLabel("Edit...");
    this->Script("%s configure -bd 2 -relief groove", 
                 this->TitleTextPropertyPopupButton->GetPopupButton()
                 ->GetPopupFrame()->GetWidgetName());

    this->Script("pack %s -padx 2 -pady 2 -side left -anchor w", 
                 this->TitleTextPropertyPopupButton->GetWidgetName());

    this->TitleTextPropertyWidget->SetParent(
      this->TitleTextPropertyPopupButton->GetPopupButton()->GetPopupFrame());
    }
  else
    {
    this->TitleTextPropertyWidget->SetParent(this->TitleFrame);
    }

  // --------------------------------------------------------------
  // Scalar Bar title text property

  this->TitleTextPropertyWidget->LongFormatOn();
  this->TitleTextPropertyWidget->LabelOnTopOn();
  this->TitleTextPropertyWidget->Create(this->Application);
  this->TitleTextPropertyWidget->ShowLabelOn();
  this->TitleTextPropertyWidget->GetLabel()->SetLabel("Title properties:");
  this->TitleTextPropertyWidget->SetChangedCommand(
    this, "TitleTextPropertyCallback");

  this->Script("pack %s -padx 2 -pady %d -side top -anchor nw -fill y", 
               this->TitleTextPropertyWidget->GetWidgetName(),
               this->TitleTextPropertyWidget->GetLongFormat() ? 0 : 2);

  // --------------------------------------------------------------
  // Label frame

  this->LabelFrame->SetParent(frame);
  this->LabelFrame->Create(app, 0);

  this->Script("pack %s -side top -fill both -expand y -pady %d", 
               this->LabelFrame->GetWidgetName(),
               (this->PopupMode ? 6 : 0));
  
  // --------------------------------------------------------------
  // Scalar Bar label format

  this->LabelFormatEntry->SetParent(this->LabelFrame);
  this->LabelFormatEntry->Create(app, 0);
  this->LabelFormatEntry->SetLabel("Label format:");
  this->LabelFormatEntry->GetEntry()->BindCommand(
    this, "ScalarBarLabelFormatCallback");

  this->LabelFormatEntry->SetBalloonHelpString(
    "Set the scalar bar label format.");

  this->Script("pack %s -padx 2 -pady 2 -side %s -anchor nw -expand y -fill x",
               this->LabelFormatEntry->GetWidgetName(),
               (!this->PopupMode ? "left" : "top"));

  // --------------------------------------------------------------
  // Scalar Bar label text property : popup button if needed

  if (popup_text_property)
    {
    if (!this->LabelTextPropertyPopupButton)
      {
      this->LabelTextPropertyPopupButton = vtkKWLabeledPopupButton::New();
      }
    this->LabelTextPropertyPopupButton->SetParent(this->LabelFrame);
    this->LabelTextPropertyPopupButton->Create(this->Application);
    this->LabelTextPropertyPopupButton->SetLabel("Label properties:");
    this->LabelTextPropertyPopupButton->SetPopupButtonLabel("Edit...");
    this->Script("%s configure -bd 2 -relief groove", 
                 this->LabelTextPropertyPopupButton->GetPopupButton()
                 ->GetPopupFrame()->GetWidgetName());

    this->Script("pack %s -padx 2 -pady 2 -side left -anchor w", 
                 this->LabelTextPropertyPopupButton->GetWidgetName());

    this->LabelTextPropertyWidget->SetParent(
      this->LabelTextPropertyPopupButton->GetPopupButton()->GetPopupFrame());
    }
  else
    {
    this->LabelTextPropertyWidget->SetParent(this->LabelFrame);
    }

  // --------------------------------------------------------------
  // Scalar Bar label text property

  this->LabelTextPropertyWidget->LongFormatOn();
  this->LabelTextPropertyWidget->LabelOnTopOn();
  this->LabelTextPropertyWidget->Create(this->Application);
  this->LabelTextPropertyWidget->ShowLabelOn();
  this->LabelTextPropertyWidget->GetLabel()->SetLabel(
    "Label text properties:");
  this->LabelTextPropertyWidget->SetChangedCommand(
    this, "LabelTextPropertyCallback");

  this->Script("pack %s -padx 2 -pady %d -side top -anchor nw -fill y", 
               this->LabelTextPropertyWidget->GetWidgetName(),
               this->LabelTextPropertyWidget->GetLongFormat() ? 0 : 2);

  // --------------------------------------------------------------
  // Maximum number of colors

  vtkScalarBarActor *foo = vtkScalarBarActor::New();

  this->MaximumNumberOfColorsThumbWheel->SetParent(frame);
  this->MaximumNumberOfColorsThumbWheel->PopupModeOn();
  this->MaximumNumberOfColorsThumbWheel->SetMinimumValue(
    foo->GetMaximumNumberOfColorsMinValue());
  this->MaximumNumberOfColorsThumbWheel->ClampMinimumValueOn();
  this->MaximumNumberOfColorsThumbWheel->SetMaximumValue(2048);
    //    foo->GetMaximumNumberOfColorsMaxValue());
  this->MaximumNumberOfColorsThumbWheel->ClampMaximumValueOn();
  this->MaximumNumberOfColorsThumbWheel->SetResolution(1);
  this->MaximumNumberOfColorsThumbWheel->Create(this->Application, "");
  this->MaximumNumberOfColorsThumbWheel->DisplayLabelOn();
  this->MaximumNumberOfColorsThumbWheel->SetLabel("Maximum number of colors:");
  this->MaximumNumberOfColorsThumbWheel->DisplayEntryOn();
  this->MaximumNumberOfColorsThumbWheel->DisplayEntryAndLabelOnTopOff();
  this->MaximumNumberOfColorsThumbWheel->GetEntry()->SetWidth(5);

  this->MaximumNumberOfColorsThumbWheel->SetBalloonHelpString(
    "Set the maximum number of scalar bar segments to show.");

  this->MaximumNumberOfColorsThumbWheel->SetEndCommand(
    this, "MaximumNumberOfColorsEndCallback");

  this->MaximumNumberOfColorsThumbWheel->SetEntryCommand(
    this, "MaximumNumberOfColorsEndCallback");

  this->Script("pack %s -padx 2 -pady 2 -side top -anchor w -fill x", 
               this->MaximumNumberOfColorsThumbWheel->GetWidgetName());
  
  // --------------------------------------------------------------
  // Number of labels

  this->NumberOfLabelsScale->SetParent(frame);
  this->NumberOfLabelsScale->SetRange(
    foo->GetNumberOfLabelsMinValue(), foo->GetNumberOfLabelsMaxValue());
  this->NumberOfLabelsScale->SetResolution(1);
  this->NumberOfLabelsScale->PopupScaleOn();
  this->NumberOfLabelsScale->Create(this->Application, "");
  this->NumberOfLabelsScale->DisplayEntry();
  this->NumberOfLabelsScale->DisplayEntryAndLabelOnTopOff();
  this->NumberOfLabelsScale->DisplayLabel("Number of labels:");
  this->NumberOfLabelsScale->SetEntryWidth(5);

  this->NumberOfLabelsScale->SetBalloonHelpString(
    "Set the number of labels to show.");

  this->NumberOfLabelsScale->SetEndCommand(
    this, "NumberOfLabelsEndCallback");

  this->NumberOfLabelsScale->SetEntryCommand(
    this, "NumberOfLabelsEndCallback");

  this->Script("pack %s -padx 2 -pady 2 -side top -anchor w -fill x", 
               this->NumberOfLabelsScale->GetWidgetName());
  
  foo->Delete();

  // --------------------------------------------------------------
  // Update the GUI according to the Ivar value (i.e. the corner prop, etc.)

  this->Update();
}

//----------------------------------------------------------------------------
void vtkKWScalarBarAnnotation::Update()
{
  this->Superclass::Update();

  vtkScalarBarActor *anno = NULL;

  if (!this->ScalarBarWidget)
    {
    this->SetEnabled(0);
    }
  else
    {
    anno = this->ScalarBarWidget->GetScalarBarActor();
    }

  if (!anno || !this->IsCreated())
    {
    return;
    }

  // Title

  if (this->TitleEntry)
    {
    this->TitleEntry->GetEntry()->SetValue(
      anno->GetTitle() ? anno->GetTitle() : "");
    }

  // Title text property

  if (this->TitleTextPropertyWidget)
    {
    this->TitleTextPropertyWidget->SetTextProperty(
      anno->GetTitleTextProperty());
    this->TitleTextPropertyWidget->SetActor2D(anno);
    this->TitleTextPropertyWidget->Update();
    }

  // Label format

  if (this->LabelFormatEntry)
    {
    this->LabelFormatEntry->GetEntry()->SetValue(
      anno->GetLabelFormat() ? anno->GetLabelFormat() : "");
    }

  // Label text property

  if (this->LabelTextPropertyWidget)
    {
    this->LabelTextPropertyWidget->SetTextProperty(
      anno->GetLabelTextProperty());
    this->LabelTextPropertyWidget->SetActor2D(anno);
    this->LabelTextPropertyWidget->Update();
    }

  // Maximum number of colors

  if (this->MaximumNumberOfColorsThumbWheel)
    {
    this->MaximumNumberOfColorsThumbWheel->SetValue(
      anno->GetMaximumNumberOfColors());
    }

  // Number of labels

  if (this->NumberOfLabelsScale)
    {
    this->NumberOfLabelsScale->SetValue(
      anno->GetNumberOfLabels());
    }
}

//----------------------------------------------------------------------------
void vtkKWScalarBarAnnotation::Render() 
{
  if (this->ScalarBarWidget && this->ScalarBarWidget->GetInteractor())
    {
    this->ScalarBarWidget->GetInteractor()->Render();
    }
}

//----------------------------------------------------------------------------
int vtkKWScalarBarAnnotation::GetVisibility() 
{
  if (!this->ScalarBarWidget)
    {
    return 0;
    }

  return this->ScalarBarWidget->GetEnabled();
}

//----------------------------------------------------------------------------
void vtkKWScalarBarAnnotation::SetVisibility(int state)
{
  if (!this->ScalarBarWidget)
    {
    return;
    }

  int old_visibility = this->GetVisibility();
  this->ScalarBarWidget->SetEnabled(state);
  if (old_visibility != this->GetVisibility())
    {
    this->Update();
    this->Render();
    this->SendChangedEvent();
    }
}

//----------------------------------------------------------------------------
void vtkKWScalarBarAnnotation::CheckButtonCallback() 
{
  if (this->CheckButton && this->CheckButton->IsCreated())
    {
    this->SetVisibility(this->CheckButton->GetState() ? 1 : 0);
    }
}

//----------------------------------------------------------------------------
void vtkKWScalarBarAnnotation::TitleTextPropertyCallback()
{
  if (this->GetVisibility())
    {
    this->Render();
    }

  this->SendChangedEvent();
}

//----------------------------------------------------------------------------
void vtkKWScalarBarAnnotation::LabelTextPropertyCallback()
{
  if (this->GetVisibility())
    {
    this->Render();
    }

  this->SendChangedEvent();
}

//----------------------------------------------------------------------------
void vtkKWScalarBarAnnotation::SetScalarBarTitle(const char *text) 
{
  if (this->ScalarBarWidget && text && 
      this->ScalarBarWidget->GetScalarBarActor() &&
      (!this->ScalarBarWidget->GetScalarBarActor()->GetTitle() || 
       strcmp(this->ScalarBarWidget->GetScalarBarActor()->GetTitle(), text)))
    {
    this->ScalarBarWidget->GetScalarBarActor()->SetTitle(text);

    this->Update();

    if (this->GetVisibility())
      {
      this->Render();
      }

    this->SendChangedEvent();
    }
}

//----------------------------------------------------------------------------
void vtkKWScalarBarAnnotation::ScalarBarTitleCallback() 
{
  if (this->IsCreated() && this->TitleEntry)
    {
    this->SetScalarBarTitle(this->TitleEntry->GetEntry()->GetValue());
    }
}

//----------------------------------------------------------------------------
void vtkKWScalarBarAnnotation::SetScalarBarLabelFormat(const char *text) 
{
  if (this->ScalarBarWidget && text && 
      this->ScalarBarWidget->GetScalarBarActor() &&
      (!this->ScalarBarWidget->GetScalarBarActor()->GetLabelFormat() || 
       strcmp(this->ScalarBarWidget->GetScalarBarActor()->GetLabelFormat(), 
              text)))
    {
    this->ScalarBarWidget->GetScalarBarActor()->SetLabelFormat(text);

    this->Update();

    if (this->GetVisibility())
      {
      this->Render();
      }

    this->SendChangedEvent();
    }
}

//----------------------------------------------------------------------------
void vtkKWScalarBarAnnotation::ScalarBarLabelFormatCallback() 
{
  if (this->IsCreated() && this->LabelFormatEntry)
    {
    this->SetScalarBarLabelFormat(
      this->LabelFormatEntry->GetEntry()->GetValue());
    }
}

//----------------------------------------------------------------------------
void vtkKWScalarBarAnnotation::MaximumNumberOfColorsEndCallback()
{
  if (this->MaximumNumberOfColorsThumbWheel && 
      this->MaximumNumberOfColorsThumbWheel->IsCreated() &&
      this->ScalarBarWidget &&
      this->ScalarBarWidget->GetScalarBarActor())
    {
    int old_v = 
      this->ScalarBarWidget->GetScalarBarActor()->GetMaximumNumberOfColors();
    this->ScalarBarWidget->GetScalarBarActor()->SetMaximumNumberOfColors(
      this->MaximumNumberOfColorsThumbWheel->GetValue());
    if (old_v != 
        this->ScalarBarWidget->GetScalarBarActor()->GetMaximumNumberOfColors())
      {
      this->Update();
      this->Render();
      this->SendChangedEvent();
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWScalarBarAnnotation::NumberOfLabelsEndCallback()
{
  if (this->NumberOfLabelsScale && 
      this->NumberOfLabelsScale->IsCreated() &&
      this->ScalarBarWidget &&
      this->ScalarBarWidget->GetScalarBarActor())
    {
    int old_v = 
      this->ScalarBarWidget->GetScalarBarActor()->GetNumberOfLabels();
    this->ScalarBarWidget->GetScalarBarActor()->SetNumberOfLabels(
      this->NumberOfLabelsScale->GetValue());
    if (old_v != 
        this->ScalarBarWidget->GetScalarBarActor()->GetNumberOfLabels())
      {
      this->Update();
      this->Render();
      this->SendChangedEvent();
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWScalarBarAnnotation::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  if (this->TitleFrame)
    {
    this->TitleFrame->SetEnabled(this->Enabled);
    }

  if (this->TitleEntry)
    {
    this->TitleEntry->SetEnabled(this->Enabled);
    }

  if (this->TitleTextPropertyWidget)
    {
    this->TitleTextPropertyWidget->SetEnabled(this->Enabled);
    }

  if (this->TitleTextPropertyPopupButton)
    {
    this->TitleTextPropertyPopupButton->SetEnabled(this->Enabled);
    }

  if (this->LabelFrame)
    {
    this->LabelFrame->SetEnabled(this->Enabled);
    }

  if (this->LabelFormatEntry)
    {
    this->LabelFormatEntry->SetEnabled(this->Enabled);
    }

  if (this->LabelTextPropertyWidget)
    {
    this->LabelTextPropertyWidget->SetEnabled(this->Enabled);
    }

  if (this->LabelTextPropertyPopupButton)
    {
    this->LabelTextPropertyPopupButton->SetEnabled(this->Enabled);
    }

  if (this->MaximumNumberOfColorsThumbWheel)
    {
    this->MaximumNumberOfColorsThumbWheel->SetEnabled(this->Enabled);
    }

  if (this->NumberOfLabelsScale)
    {
    this->NumberOfLabelsScale->SetEnabled(this->Enabled);
    }
}

//----------------------------------------------------------------------------
void vtkKWScalarBarAnnotation::SendChangedEvent()
{
  if (!this->ScalarBarWidget || !this->ScalarBarWidget->GetScalarBarActor())
    {
    return;
    }

#ifdef DO_NOT_BUILD_XML_RW
  this->InvokeEvent(this->AnnotationChangedEvent, NULL);
#else
  ostrstream event;

  vtkXMLScalarBarWidgetWriter *xmlw = vtkXMLScalarBarWidgetWriter::New();
  xmlw->SetObject(this->ScalarBarWidget);
  xmlw->Write(event);
  xmlw->Delete();

  event << ends;

  this->InvokeEvent(this->AnnotationChangedEvent, event.str());
  event.rdbuf()->freeze(0);
#endif
}

//----------------------------------------------------------------------------
void vtkKWScalarBarAnnotation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "AnnotationChangedEvent: " 
     << this->AnnotationChangedEvent << endl;
  os << indent << "ScalarBarWidget: " << this->GetScalarBarWidget() << endl;
  os << indent << "PopupTextProperty: " 
     << (this->PopupTextProperty ? "On" : "Off") << endl;
}
