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
#include "vtkKWHeaderAnnotation.h"

#include "vtkKWCheckButton.h"
#include "vtkKWEntry.h"
#include "vtkKWEvent.h"
#include "vtkKWFrame.h"
#include "vtkKWLabel.h"
#include "vtkKWLabeledEntry.h"
#include "vtkKWLabeledFrame.h"
#include "vtkKWLabeledPopupButton.h"
#include "vtkKWPopupButton.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWTextProperty.h"
#include "vtkObjectFactory.h"
#include "vtkTextActor.h"
#include "vtkTextProperty.h"
#ifndef DO_NOT_BUILD_XML_RW
#include "vtkXMLTextActorWriter.h"
#endif

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWHeaderAnnotation );
vtkCxxRevisionMacro(vtkKWHeaderAnnotation, "$Revision: 1.3 $");

int vtkKWHeaderAnnotationCommand(ClientData cd, Tcl_Interp *interp,
                                 int argc, char *argv[]);

//----------------------------------------------------------------------------
vtkKWHeaderAnnotation::vtkKWHeaderAnnotation()
{
  this->CommandFunction = vtkKWHeaderAnnotationCommand;

  this->AnnotationChangedEvent  = vtkKWEvent::ViewAnnotationChangedEvent;
  this->PopupTextProperty       = 0;
  this->RenderWidget            = NULL;

  // GUI

  this->TextFrame               = vtkKWFrame::New();
  this->TextEntry               = vtkKWLabeledEntry::New();
  this->TextPropertyWidget      = vtkKWTextProperty::New();
  this->TextPropertyPopupButton = NULL;
}

//----------------------------------------------------------------------------
vtkKWHeaderAnnotation::~vtkKWHeaderAnnotation()
{
  // GUI

  if (this->TextFrame)
    {
    this->TextFrame->Delete();
    this->TextFrame = NULL;
    }

  if (this->TextEntry)
    {
    this->TextEntry->Delete();
    this->TextEntry = NULL;
    }

  if (this->TextPropertyWidget)
    {
    this->TextPropertyWidget->Delete();
    this->TextPropertyWidget = NULL;
    }

  if (this->TextPropertyPopupButton)
    {
    this->TextPropertyPopupButton->Delete();
    this->TextPropertyPopupButton = NULL;
    }

  this->SetRenderWidget(NULL);
}

//----------------------------------------------------------------------------
void vtkKWHeaderAnnotation::SetRenderWidget(vtkKWRenderWidget *_arg)
{ 
  if (this->RenderWidget == _arg) 
    {
    return;
    }

  if (this->RenderWidget != NULL) 
    { 
    this->RenderWidget->UnRegister(this); 
    }

  this->RenderWidget = _arg;

  if (this->RenderWidget != NULL) 
    { 
    this->RenderWidget->Register(this); 
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
void vtkKWHeaderAnnotation::Create(vtkKWApplication *app, 
                                   const char *args)
{
  // Create the superclass widgets

  if (this->IsCreated())
    {
    vtkErrorMacro("HeaderAnnotation already created");
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

  this->Frame->SetLabel("Header annotation");

  // --------------------------------------------------------------
  // Edit the check button (Annotation visibility)

  this->CheckButton->SetText("Display header annotation");

  this->CheckButton->SetBalloonHelpString(
    "Toggle the visibility of the header annotation text");

  // --------------------------------------------------------------
  // Text frame

  this->TextFrame->SetParent(frame);
  this->TextFrame->Create(app, 0);

  this->Script("pack %s -side top -fill both -expand y", 
               this->TextFrame->GetWidgetName());
  
  // --------------------------------------------------------------
  // Header text

  this->TextEntry->SetParent(this->TextFrame);
  this->TextEntry->Create(app, 0);
  this->TextEntry->SetLabel("Header:");
  this->TextEntry->GetEntry()->BindCommand(this, "HeaderTextCallback");

  this->TextEntry->SetBalloonHelpString(
    "Set the header annotation. The text will automatically scale "
    "to fit within the allocated space");

  this->Script("pack %s -padx 2 -pady 2 -side %s -anchor nw -expand y -fill x",
               this->TextEntry->GetWidgetName(),
               (popup_text_property ? "left" : "top"));
  
  // --------------------------------------------------------------
  // Text property : popup button if needed

  if (popup_text_property)
    {
    if (!this->TextPropertyPopupButton)
      {
      this->TextPropertyPopupButton = vtkKWLabeledPopupButton::New();
      }
    this->TextPropertyPopupButton->SetParent(this->TextFrame);
    this->TextPropertyPopupButton->Create(this->Application);
    this->TextPropertyPopupButton->SetLabel("Header properties:");
    this->TextPropertyPopupButton->SetPopupButtonLabel("Edit...");
    this->Script("%s configure -bd 2 -relief groove", 
                 this->TextPropertyPopupButton->GetPopupButton()
                 ->GetPopupFrame()->GetWidgetName());

    this->Script("pack %s -padx 2 -pady 2 -side left -anchor w", 
                 this->TextPropertyPopupButton->GetWidgetName());

    this->TextPropertyWidget->SetParent(
      this->TextPropertyPopupButton->GetPopupButton()->GetPopupFrame());
    }
  else
    {
    this->TextPropertyWidget->SetParent(this->TextFrame);
    }

  // --------------------------------------------------------------
  // Text property

  this->TextPropertyWidget->LongFormatOn();
  this->TextPropertyWidget->LabelOnTopOn();
  this->TextPropertyWidget->Create(this->Application);
  this->TextPropertyWidget->ShowLabelOn();
  this->TextPropertyWidget->GetLabel()->SetLabel("Header properties:");
  this->TextPropertyWidget->SetChangedCommand(this, "TextPropertyCallback");

  this->Script("pack %s -padx 2 -pady %d -side top -anchor nw -fill y", 
               this->TextPropertyWidget->GetWidgetName(),
               this->TextPropertyWidget->GetLongFormat() ? 0 : 2);

  // --------------------------------------------------------------
  // Update the GUI according to the Ivar value (i.e. the corner prop, etc.)

  this->Update();
}

//----------------------------------------------------------------------------
void vtkKWHeaderAnnotation::Update()
{
  this->Superclass::Update();

  vtkTextActor *anno = NULL;

  if (!this->RenderWidget)
    {
    this->SetEnabled(0);
    }
  else
    {
    anno = this->RenderWidget->GetHeaderAnnotation();
    }

  if (!anno || !this->IsCreated())
    {
    return;
    }

  // Corners text

  if (this->TextEntry)
    {
    this->TextEntry->GetEntry()->SetValue(
      anno->GetInput() ? anno->GetInput() : "");
    }

  // Text property

  if (this->TextPropertyWidget)
    {
    this->TextPropertyWidget->SetTextProperty(anno->GetTextProperty());
    this->TextPropertyWidget->SetActor2D(anno);
    this->TextPropertyWidget->Update();
    }
}

//----------------------------------------------------------------------------
void vtkKWHeaderAnnotation::Render() 
{
  if (this->RenderWidget)
    {
    this->RenderWidget->Render();
    }
}

//----------------------------------------------------------------------------
int vtkKWHeaderAnnotation::GetVisibility() 
{
  if (!this->RenderWidget)
    {
    return 0;
    }

  return this->RenderWidget->GetHeaderAnnotationVisibility();
}

//----------------------------------------------------------------------------
void vtkKWHeaderAnnotation::SetVisibility(int state)
{
  if (!this->RenderWidget)
    {
    return;
    }

  int old_visibility = this->GetVisibility();
  this->RenderWidget->SetHeaderAnnotationVisibility(state);
  if (old_visibility != this->GetVisibility())
    {
    this->Update();
    this->Render();
    this->SendChangedEvent();
    }
}

//----------------------------------------------------------------------------
void vtkKWHeaderAnnotation::CheckButtonCallback() 
{
  if (this->CheckButton && this->CheckButton->IsCreated())
    {
    this->SetVisibility(this->CheckButton->GetState() ? 1 : 0);
    }
}

//----------------------------------------------------------------------------
void vtkKWHeaderAnnotation::TextPropertyCallback()
{
  if (this->GetVisibility())
    {
    this->Render();
    }

  this->SendChangedEvent();
}

//----------------------------------------------------------------------------
void vtkKWHeaderAnnotation::SetHeaderText(const char *text) 
{
  if (this->RenderWidget && text &&
      (!this->RenderWidget->GetHeaderAnnotationText() || 
       strcmp(this->RenderWidget->GetHeaderAnnotationText(), text)))
    {
    this->RenderWidget->SetHeaderAnnotationText(text);

    this->Update();

    if (this->GetVisibility())
      {
      this->Render();
      }

    this->SendChangedEvent();
    }
}

//----------------------------------------------------------------------------
void vtkKWHeaderAnnotation::HeaderTextCallback() 
{
  if (this->TextEntry && this->TextEntry->IsCreated())
    {
    this->SetHeaderText(this->TextEntry->GetEntry()->GetValue());
    }
}

//----------------------------------------------------------------------------
void vtkKWHeaderAnnotation::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  if (this->TextFrame)
    {
    this->TextFrame->SetEnabled(this->Enabled);
    }

  if (this->TextEntry)
    {
    this->TextEntry->SetEnabled(this->Enabled);
    }

  if (this->TextPropertyWidget)
    {
    this->TextPropertyWidget->SetEnabled(this->Enabled);
    }

  if (this->TextPropertyPopupButton)
    {
    this->TextPropertyPopupButton->SetEnabled(this->Enabled);
    }
}

//----------------------------------------------------------------------------
void vtkKWHeaderAnnotation::SendChangedEvent()
{
  if (!this->RenderWidget || !this->RenderWidget->GetHeaderAnnotation())
    {
    return;
    }

#ifdef DO_NOT_BUILD_XML_RW
  this->InvokeEvent(this->AnnotationChangedEvent, NULL);
#else
  ostrstream event;

  vtkXMLTextActorWriter *xmlw = vtkXMLTextActorWriter::New();
  xmlw->SetObject(this->RenderWidget->GetHeaderAnnotation());
  xmlw->Write(event);
  xmlw->Delete();

  event << ends;

  this->InvokeEvent(this->AnnotationChangedEvent, event.str());
  event.rdbuf()->freeze(0);
#endif
}

//----------------------------------------------------------------------------
void vtkKWHeaderAnnotation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "AnnotationChangedEvent: " 
     << this->AnnotationChangedEvent << endl;
  os << indent << "RenderWidget: " << this->GetRenderWidget() << endl;
  os << indent << "PopupTextProperty: " 
     << (this->PopupTextProperty ? "On" : "Off") << endl;
}
