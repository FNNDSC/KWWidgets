/*=========================================================================

  Module:    $RCSfile: vtkKWHeaderAnnotationEditor.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWHeaderAnnotationEditor.h"

#include "vtkKWCheckButton.h"
#include "vtkKWEntry.h"
#include "vtkKWEvent.h"
#include "vtkKWFrame.h"
#include "vtkKWLabel.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWPopupButtonWithLabel.h"
#include "vtkKWPopupButton.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWTextPropertyEditor.h"
#include "vtkObjectFactory.h"
#include "vtkTextActor.h"
#include "vtkTextProperty.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWHeaderAnnotationEditor );
vtkCxxRevisionMacro(vtkKWHeaderAnnotationEditor, "$Revision: 1.7 $");

//----------------------------------------------------------------------------
vtkKWHeaderAnnotationEditor::vtkKWHeaderAnnotationEditor()
{
  this->AnnotationChangedEvent  = vtkKWEvent::ViewAnnotationChangedEvent;
  this->PopupTextProperty       = 0;
  this->RenderWidget            = NULL;

  // GUI

  this->TextFrame               = vtkKWFrame::New();
  this->TextEntry               = vtkKWEntryWithLabel::New();
  this->TextPropertyWidget      = vtkKWTextPropertyEditor::New();
  this->TextPropertyPopupButton = NULL;
}

//----------------------------------------------------------------------------
vtkKWHeaderAnnotationEditor::~vtkKWHeaderAnnotationEditor()
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
void vtkKWHeaderAnnotationEditor::SetRenderWidget(vtkKWRenderWidget *_arg)
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
void vtkKWHeaderAnnotationEditor::Create(vtkKWApplication *app)
{
  // Create the superclass widgets

  if (this->IsCreated())
    {
    vtkErrorMacro("HeaderAnnotation already created");
    return;
    }

  this->Superclass::Create(app);

  int popup_text_property = 
    this->PopupTextProperty && !this->PopupMode;

  vtkKWWidget *frame = this->Frame->GetFrame();

  // --------------------------------------------------------------
  // If in popup mode, modify the popup button

  if (this->PopupMode)
    {
    this->PopupButton->SetText("Edit...");
    }

  // --------------------------------------------------------------
  // Edit the labeled frame

  this->Frame->SetLabelText("Header annotation");

  // --------------------------------------------------------------
  // Edit the check button (Annotation visibility)

  this->CheckButton->SetText("Display header annotation");

  this->CheckButton->SetBalloonHelpString(
    "Toggle the visibility of the header annotation text");

  // --------------------------------------------------------------
  // Text frame

  this->TextFrame->SetParent(frame);
  this->TextFrame->Create(app);

  this->Script("pack %s -side top -fill both -expand y", 
               this->TextFrame->GetWidgetName());
  
  // --------------------------------------------------------------
  // Header text

  this->TextEntry->SetParent(this->TextFrame);
  this->TextEntry->Create(app);
  this->TextEntry->GetLabel()->SetText("Header:");
  this->TextEntry->GetWidget()->SetWidth(20);
  this->TextEntry->GetWidget()->SetCommand(this, "HeaderTextCallback");

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
      this->TextPropertyPopupButton = vtkKWPopupButtonWithLabel::New();
      }
    this->TextPropertyPopupButton->SetParent(this->TextFrame);
    this->TextPropertyPopupButton->Create(app);
    this->TextPropertyPopupButton->GetLabel()->SetText("Header properties:");
    this->TextPropertyPopupButton->GetWidget()->SetText("Edit...");
    this->Script("%s configure -bd 2 -relief groove", 
                 this->TextPropertyPopupButton->GetWidget()
                 ->GetPopupFrame()->GetWidgetName());

    this->Script("pack %s -padx 2 -pady 2 -side left -anchor w", 
                 this->TextPropertyPopupButton->GetWidgetName());

    this->TextPropertyWidget->SetParent(
      this->TextPropertyPopupButton->GetWidget()->GetPopupFrame());
    }
  else
    {
    this->TextPropertyWidget->SetParent(this->TextFrame);
    }

  // --------------------------------------------------------------
  // Text property

  this->TextPropertyWidget->LongFormatOn();
  this->TextPropertyWidget->LabelOnTopOn();
  this->TextPropertyWidget->ShowLabelOn();
  this->TextPropertyWidget->Create(app);
  this->TextPropertyWidget->GetLabel()->SetText("Header properties:");
  this->TextPropertyWidget->SetChangedCommand(this, "TextPropertyCallback");

  this->Script("pack %s -padx 2 -pady %d -side top -anchor nw -fill y", 
               this->TextPropertyWidget->GetWidgetName(),
               this->TextPropertyWidget->GetLongFormat() ? 0 : 2);

  // --------------------------------------------------------------
  // Update the GUI according to the Ivar value (i.e. the corner prop, etc.)

  this->Update();
}

//----------------------------------------------------------------------------
void vtkKWHeaderAnnotationEditor::Update()
{
  this->Superclass::Update();

  vtkTextActor *anno = NULL;
  if (this->RenderWidget)
    {
    anno = this->RenderWidget->GetHeaderAnnotation();
    }

  if (!this->IsCreated())
    {
    return;
    }

  // Corners text

  if (this->TextEntry)
    {
    if (anno)
      {
      this->TextEntry->GetWidget()->SetValue(
        anno->GetInput() ? anno->GetInput() : "");
      }
    }

  // Text property

  if (this->TextPropertyWidget)
    {
    this->TextPropertyWidget->SetTextProperty(
      anno ? anno->GetTextProperty() : NULL);
    this->TextPropertyWidget->SetActor2D(anno);
    this->TextPropertyWidget->Update();
    }
}

//----------------------------------------------------------------------------
void vtkKWHeaderAnnotationEditor::Render() 
{
  if (this->RenderWidget)
    {
    this->RenderWidget->Render();
    }
}

//----------------------------------------------------------------------------
int vtkKWHeaderAnnotationEditor::GetVisibility() 
{
  if (!this->RenderWidget)
    {
    return 0;
    }

  return this->RenderWidget->GetHeaderAnnotationVisibility();
}

//----------------------------------------------------------------------------
void vtkKWHeaderAnnotationEditor::SetVisibility(int state)
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
void vtkKWHeaderAnnotationEditor::CheckButtonCallback() 
{
  if (this->CheckButton && this->CheckButton->IsCreated())
    {
    this->SetVisibility(this->CheckButton->GetSelectedState() ? 1 : 0);
    }
}

//----------------------------------------------------------------------------
void vtkKWHeaderAnnotationEditor::TextPropertyCallback()
{
  if (this->GetVisibility())
    {
    this->Render();
    }

  this->SendChangedEvent();
}

//----------------------------------------------------------------------------
void vtkKWHeaderAnnotationEditor::SetHeaderText(const char *text) 
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
void vtkKWHeaderAnnotationEditor::HeaderTextCallback() 
{
  if (this->TextEntry && this->TextEntry->IsCreated())
    {
    this->SetHeaderText(this->TextEntry->GetWidget()->GetValue());
    }
}

//----------------------------------------------------------------------------
void vtkKWHeaderAnnotationEditor::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  int enabled = this->RenderWidget ? this->GetEnabled() : 0;

  if (this->TextFrame)
    {
    this->TextFrame->SetEnabled(enabled);
    }
  if (this->TextEntry)
    {
    this->TextEntry->SetEnabled(enabled);
    }
  if (this->TextPropertyWidget)
    {
    this->TextPropertyWidget->SetEnabled(enabled);
    }
  if (this->TextPropertyPopupButton)
    {
    this->TextPropertyPopupButton->SetEnabled(enabled);
    }
}

//----------------------------------------------------------------------------
void vtkKWHeaderAnnotationEditor::SendChangedEvent()
{
  if (!this->RenderWidget || !this->RenderWidget->GetHeaderAnnotation())
    {
    return;
    }

  this->InvokeEvent(this->AnnotationChangedEvent, NULL);
}

//----------------------------------------------------------------------------
void vtkKWHeaderAnnotationEditor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "AnnotationChangedEvent: " 
     << this->AnnotationChangedEvent << endl;
  os << indent << "RenderWidget: " << this->GetRenderWidget() << endl;
  os << indent << "PopupTextProperty: " 
     << (this->PopupTextProperty ? "On" : "Off") << endl;
}
