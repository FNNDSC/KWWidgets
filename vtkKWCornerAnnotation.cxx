/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWCornerAnnotation.cxx,v $
  Language:  C++
  Date:      $Date: 2000-08-01 14:04:26 $
  Version:   $Revision: 1.8 $

Copyright (c) 1998-1999 Kitware Inc. 469 Clifton Corporate Parkway,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
#include "vtkKWCornerAnnotation.h"
#include "vtkKWWindow.h"
#include "vtkKWView.h"
#include "vtkObjectFactory.h"
#include "vtkCornerAnnotation.h"
#include "vtkKWApplication.h"
#include "vtkKWEventNotifier.h"

//-----------------------------------------------------------------------------
vtkKWCornerAnnotation* vtkKWCornerAnnotation::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkKWCornerAnnotation");
  if(ret)
    {
    return (vtkKWCornerAnnotation*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkKWCornerAnnotation;
}




int vtkKWCornerAnnotationCommand(ClientData cd, Tcl_Interp *interp,
				int argc, char *argv[]);

vtkKWCornerAnnotation::vtkKWCornerAnnotation()
{
  this->View = NULL;
  
  this->CommandFunction = vtkKWCornerAnnotationCommand;

  this->CornerDisplayFrame = vtkKWWidget::New();
  this->CornerDisplayFrame->SetParent( this->GetFrame() );
  this->CornerButton = vtkKWCheckButton::New();
  this->CornerButton->SetParent(this->CornerDisplayFrame);
  this->CornerColor = vtkKWChangeColorButton::New();
  this->CornerColor->SetParent(this->CornerDisplayFrame);
  this->CornerTopFrame = vtkKWWidget::New();
  this->CornerTopFrame->SetParent( this->GetFrame() );
  this->CornerBottomFrame = vtkKWWidget::New();
  this->CornerBottomFrame->SetParent( this->GetFrame() );

  for (int i = 0; i < 4; i++)
    {
    this->CornerFrame[i] = vtkKWWidget::New();
    this->CornerFrame[i]->SetParent( 
	     (i<2)?(this->CornerBottomFrame):(this->CornerTopFrame) );
    this->CornerLabel[i] = vtkKWWidget::New();    
    this->CornerLabel[i]->SetParent(this->CornerFrame[i]);
    this->CornerText[i] = vtkKWText::New();
    this->CornerText[i]->SetParent(this->CornerFrame[i]);
    }

  this->CornerProp = vtkCornerAnnotation::New();
  this->CornerProp->SetMaximumLineHeight(0.08);
  this->CornerComposite = vtkKWGenericComposite::New();
  this->CornerComposite->SetProp(this->CornerProp);

  this->LightboxID = -1;
  this->MasterCornerAnnotation = NULL;
}

vtkKWCornerAnnotation::~vtkKWCornerAnnotation()
{
  this->Application->GetEventNotifier()->RemoveCallbacks(this);
  this->SetMasterCornerAnnotation(NULL);
  
  this->CornerDisplayFrame->Delete();
  this->CornerButton->Delete();
  this->CornerColor->Delete();
  this->CornerTopFrame->Delete();
  this->CornerBottomFrame->Delete();

  for (int i = 0; i < 4; i++)
    {
    this->CornerFrame[i]->Delete();
    this->CornerLabel[i]->Delete();
    this->CornerText[i]->Delete();
    }
  this->CornerComposite->Delete();
  this->CornerProp->Delete();

}

void vtkKWCornerAnnotation::ShowProperties()
{
  // unpack any current children
  this->Script("eval pack forget [pack slaves %s]",
               this->View->GetPropertiesParent()->GetWidgetName());
}

void vtkKWCornerAnnotation::Close()
{
  if (this->CornerButton->GetState())
    {
    this->View->RemoveComposite(this->CornerComposite);
    this->CornerButton->SetState(0);
    }
}

void vtkKWCornerAnnotation::Create(vtkKWApplication *app)
{
  this->vtkKWLabeledFrame::Create(app);
  
  this->CornerDisplayFrame->Create( app, "frame", "" );
  this->Script(
    "pack %s -side top -padx 0 -pady 0 -expand 1 -fill x -anchor nw",
    this->CornerDisplayFrame->GetWidgetName() );
  this->CornerButton->Create(this->Application,
                             "-text {Display Corner Annotation}");
  this->CornerButton->SetBalloonHelpString("Toggle the visibility of the corner annotation");
  this->CornerButton->SetCommand(this, "OnDisplayCorner");
  this->Script("pack %s -side left -padx 2 -pady 4 -anchor nw",
               this->CornerButton->GetWidgetName());
  this->CornerColor->Create( app, "" );
  this->CornerColor->SetBalloonHelpJustificationToRight();
  this->CornerColor->SetBalloonHelpString( "Change the color of all four corner annotation text items" );
  this->Script("pack %s -side right -padx 2 -pady 4 -anchor ne",
               this->CornerColor->GetWidgetName());
  this->CornerColor->SetCommand( this, "SetTextColor" );

  this->CornerTopFrame->Create( app, "frame", "" );
  this->CornerBottomFrame->Create( app, "frame", "" );

  this->Script(
    "pack %s %s -side top -padx 2 -pady 2 -expand 1 -fill x -anchor nw",
    this->CornerTopFrame->GetWidgetName(),
    this->CornerBottomFrame->GetWidgetName() );

  this->CornerFrame[0]->Create( app, "frame", "" );
  this->CornerFrame[1]->Create( app, "frame", "" );

  this->Script(
    "pack %s %s -side left -padx 0 -pady 0 -expand 1 -fill x -anchor nw",
    this->CornerFrame[0]->GetWidgetName(),
    this->CornerFrame[1]->GetWidgetName() );

  this->CornerFrame[2]->Create( app, "frame", "" );
  this->CornerFrame[3]->Create( app, "frame", "" );

  this->Script(
    "pack %s %s -side left -padx 0 -pady 0 -expand 1 -fill x -anchor nw",
    this->CornerFrame[2]->GetWidgetName(),
    this->CornerFrame[3]->GetWidgetName() );

  this->CornerLabel[0]->Create(app,"label","-text {Lower Left}");
  this->CornerLabel[1]->Create(app,"label","-text {Lower Right}");
  this->CornerLabel[2]->Create(app,"label","-text {Upper Left}");
  this->CornerLabel[3]->Create(app,"label","-text {Upper Right}");

  for (int i = 0; i < 4; i++)
    {
    this->CornerText[i]->Create(app,"-height 4 -width 10 -wrap none");
    this->Script("bind %s <Return> {%s CornerChanged %i}",
                 this->CornerText[i]->GetWidgetName(), 
                 this->GetTclName(), i);
    this->Script("pack %s -side top -anchor w -padx 4 -expand yes",
                 this->CornerLabel[i]->GetWidgetName());
    this->Script("pack %s -side top -anchor w -padx 4 -pady 2 -expand yes -fill x",
                 this->CornerText[i]->GetWidgetName());
    }

  this->CornerLabel[0]->SetBalloonHelpString("Set the lower left corner annotation. The text will automatically scale to fit within the allocated space");

  this->CornerLabel[1]->SetBalloonHelpJustificationToRight();
  this->CornerLabel[1]->SetBalloonHelpString("Set the lower right corner annotation. The text will automatically scale to fit within the allocated space");

  this->CornerLabel[2]->SetBalloonHelpString("Set the upper left corner annotation. The text will automatically scale to fit within the allocated space");

  this->CornerLabel[3]->SetBalloonHelpJustificationToRight();
  this->CornerLabel[3]->SetBalloonHelpString("Set the upper right corner annotation. The text will automatically scale to fit within the allocated space");

  // Register the events we are interested. 

  // If we are part of a lightbox, but not the master lightbox view, we
  // have to update ourselves if the master lightbox corner anno changes in
  // any way.

  
  if ( this->LightboxID > 0 )
    {
    this->Application->GetEventNotifier()->
      AddCallback( "LightboxCornerAnnoChanged", this->View->GetWindow(),
		   this, "UpdateFromMaster" );
    }
}

void vtkKWCornerAnnotation::SetTextColor( float r, float g, float b )
{
  this->CornerProp->GetProperty()->SetColor( r, g, b );
  this->View->Render();

  if ( this->LightboxID == 0 )
    {
    this->Application->GetEventNotifier()->
      InvokeCallbacks( this, "LightboxCornerAnnoChanged",  
		       this->View->GetWindow(), "" );
    }
}

void vtkKWCornerAnnotation::OnDisplayCorner() 
{
  if (this->CornerButton->GetState())
    {
    this->View->AddComposite(this->CornerComposite);
    for (int i = 0; i < 4; i++)
      {
      this->CornerProp->SetText(i,this->GetCornerText(i));
      }
    this->View->Render();
    }
  else
    {
    this->View->RemoveComposite(this->CornerComposite);
    this->View->Render();
    }

  if ( this->LightboxID == 0 )
    {
    this->Application->GetEventNotifier()->
      InvokeCallbacks( this, "LightboxCornerAnnoChanged",  
		       this->View->GetWindow(), "" );
    }
}

int vtkKWCornerAnnotation::GetVisibility() 
{
  return this->CornerButton->GetState();
}

void vtkKWCornerAnnotation::SetVisibility(int state) 
{
  if (state == this->CornerButton->GetState())
    {
    return;
    }
  this->CornerButton->SetState(state);
  this->OnDisplayCorner();
}

void vtkKWCornerAnnotation::SetCornerText(const char *text, int corner) 
{
  this->CornerText[corner]->SetValue(text);
  
  this->CornerChanged(corner);
}

void vtkKWCornerAnnotation::CornerChanged(int i) 
{
  this->CornerProp->SetText(i,this->CornerText[i]->GetValue());
  if (this->CornerButton->GetState())
    {
    this->View->Render();
    }

  if ( this->LightboxID == 0 )
    {
    this->Application->GetEventNotifier()->
      InvokeCallbacks( this, "LightboxCornerAnnoChanged",  
		       this->View->GetWindow(), "" );
    }
}

void vtkKWCornerAnnotation::UpdateFromMaster()
{
  if ( this->MasterCornerAnnotation )
    {
    this->SetCornerText(this->MasterCornerAnnotation->GetCornerText(0), 0);
    this->SetCornerText(this->MasterCornerAnnotation->GetCornerText(1), 1);
    this->SetCornerText(this->MasterCornerAnnotation->GetCornerText(2), 2);
    this->SetCornerText(this->MasterCornerAnnotation->GetCornerText(3), 3);
    
    this->CornerProp->GetProperty()->
      SetColor( this->MasterCornerAnnotation->GetTextColor() );

    this->SetVisibility( this->MasterCornerAnnotation->GetVisibility() );
    
    this->View->Render();
    }
}

// Description:
// Chaining method to serialize an object and its superclasses.
void vtkKWCornerAnnotation::SerializeSelf(ostream& os, vtkIndent indent)
{
  // invoke superclass
  this->vtkKWLabeledFrame::SerializeSelf(os,indent);

  os << indent << "CornerText0 ";
  vtkKWSerializer::WriteSafeString(os, this->CornerText[0]->GetValue());
  os << endl;
  os << indent << "CornerText1 ";
  vtkKWSerializer::WriteSafeString(os, this->CornerText[1]->GetValue());
  os << endl;
  os << indent << "CornerText2 ";
  vtkKWSerializer::WriteSafeString(os, this->CornerText[2]->GetValue());
  os << endl;
  os << indent << "CornerText3 ";
  vtkKWSerializer::WriteSafeString(os, this->CornerText[3]->GetValue());
  os << endl;

  os << indent << "CornerButton " << this->CornerButton->GetState() << endl;
  os << indent << "CornerColor ";
  this->CornerColor->Serialize(os,indent);
}

void vtkKWCornerAnnotation::SerializeToken(istream& is, 
                                           const char token[1024])
{
  int i;
  char tmp[1024];
  
  if (!strcmp(token,"CornerButton"))
    {
    is >> i;
    this->CornerButton->SetState(i);
    this->OnDisplayCorner();
    return;
    }
  if (!strcmp(token,"CornerText0"))
    {
    vtkKWSerializer::GetNextToken(&is,tmp);
    this->CornerText[0]->SetValue(tmp);
    this->OnDisplayCorner();
    return;
    }
  if (!strcmp(token,"CornerText1"))
    {
    vtkKWSerializer::GetNextToken(&is,tmp);
    this->CornerText[1]->SetValue(tmp);
    this->OnDisplayCorner();
    return;
    }
  if (!strcmp(token,"CornerText2"))
    {
    vtkKWSerializer::GetNextToken(&is,tmp);
    this->CornerText[2]->SetValue(tmp);
    this->OnDisplayCorner();
    return;
    }
  if (!strcmp(token,"CornerText3"))
    {
    vtkKWSerializer::GetNextToken(&is,tmp);
    this->CornerText[3]->SetValue(tmp);
    this->OnDisplayCorner();
    return;
    }
  if (!strcmp(token,"CornerColor"))
    {
    this->CornerColor->Serialize(is);
    return;
    }

  vtkKWLabeledFrame::SerializeToken(is,token);
}

void vtkKWCornerAnnotation::SerializeRevision(ostream& os, vtkIndent indent)
{
  os << indent << "vtkKWCornerAnnotation ";
  this->ExtractRevision(os,"$Revision: 1.8 $");
  vtkKWLabeledFrame::SerializeRevision(os,indent);
}
