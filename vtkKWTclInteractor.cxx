/*=========================================================================

  Module:    $RCSfile: vtkKWTclInteractor.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWTclInteractor.h"

#include "vtkKWApplication.h"
#include "vtkKWEntry.h"
#include "vtkKWFrame.h"
#include "vtkKWLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWText.h"
#include "vtkKWTextWithScrollbars.h"
#include "vtkObjectFactory.h"

#include <vtksys/stl/string>

//-------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWTclInteractor );
vtkCxxRevisionMacro(vtkKWTclInteractor, "$Revision: 1.41 $");

//----------------------------------------------------------------------------
vtkKWTclInteractor::vtkKWTclInteractor()
{
  this->ButtonFrame = vtkKWFrame::New();
  this->DismissButton = vtkKWPushButton::New();

  this->CommandFrame = vtkKWFrame::New();
  this->CommandLabel = vtkKWLabel::New();
  this->CommandEntry = vtkKWEntry::New();
  
  this->DisplayText = vtkKWTextWithScrollbars::New();
  
  this->SetTitle("Tcl Interactor");
  
  this->TagNumber = 1;
  this->CommandIndex = 0;
}

//----------------------------------------------------------------------------
vtkKWTclInteractor::~vtkKWTclInteractor()
{
  this->DismissButton->Delete();
  this->DismissButton = NULL;
  this->ButtonFrame->Delete();
  this->ButtonFrame = NULL;
  
  this->CommandLabel->Delete();
  this->CommandLabel = NULL;
  this->CommandEntry->Delete();
  this->CommandEntry = NULL;
  this->CommandFrame->Delete();
  this->CommandFrame = NULL;
  
  this->DisplayText->Delete();
  this->DisplayText = NULL;
}

//----------------------------------------------------------------------------
void vtkKWTclInteractor::Create(vtkKWApplication *app)
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::Create(app);

  this->ButtonFrame->SetParent(this);
  this->ButtonFrame->Create(app);
  this->Script("pack %s -side bottom -fill both -expand 0 -pady 2m",
               this->ButtonFrame->GetWidgetName());
  
  this->DismissButton->SetParent(this->ButtonFrame);
  this->DismissButton->Create(app);
  this->DismissButton->SetCommand(this, "Withdraw");
  this->DismissButton->SetText("Dismiss");
  this->Script("pack %s -side left -expand 1 -fill x",
               this->DismissButton->GetWidgetName());

  this->CommandFrame->SetParent(this);
  this->CommandFrame->Create(app);
  
  this->CommandLabel->SetParent(this->CommandFrame);
  this->CommandLabel->Create(app);
  this->CommandLabel->SetText("Command:");
  
  this->CommandEntry->SetParent(this->CommandFrame);
  this->CommandEntry->Create(app);
  this->CommandEntry->SetWidth(40);
  this->CommandEntry->SetHighlightThickness(1);

  this->CommandEntry->SetBinding("<Return>", this, "Evaluate");
  
  this->Script("pack %s -side left", this->CommandLabel->GetWidgetName());
  this->Script("pack %s -side left -expand 1 -fill x",
               this->CommandEntry->GetWidgetName());
  
  this->DisplayText->SetParent(this);
  this->DisplayText->Create(app);
  this->DisplayText->VerticalScrollbarVisibilityOn();

  vtkKWText *text = this->DisplayText->GetWidget();
  text->SetWidth(100);
  text->SetHeight(20);
  text->SetWrapToWord();
  text->ReadOnlyOn();

  this->Script("pack %s -side bottom -expand 1 -fill both",
               this->DisplayText->GetWidgetName());

  this->Script("pack %s -pady 3m -padx 2m -side bottom -fill x",
               this->CommandFrame->GetWidgetName());

  this->Script("set commandList \"\"");

  this->SetBinding("<Down>", this, "DownCallback");
  this->SetBinding("<Up>", this, "UpCallback");
  
  this->Withdraw();
}

//----------------------------------------------------------------------------
void vtkKWTclInteractor::Evaluate()
{
  this->CommandIndex = this->TagNumber;
  this->TagNumber++;

  char buffer_tag[32];
  sprintf(buffer_tag, "%d", this->CommandIndex);

  vtksys_stl::string entry(this->CommandEntry->GetValue());

  this->DisplayText->GetWidget()->AppendValue(entry.c_str(),
                                 buffer_tag);
  this->DisplayText->GetWidget()->AppendValue("\n");

  this->Script("set commandList [linsert $commandList end [concat {%s}]]",
               entry.c_str());

  this->Register(this);

  this->Script("catch {eval [list %s]} _tmp_err",  
               entry.c_str());

  if (this->GetApplication()->GetInExit())
    {
    this->UnRegister(this);
    return;
    }
  this->UnRegister(this);

  vtksys_stl::string res(this->Script("set _tmp_err"));
  this->DisplayText->GetWidget()->AppendValue(res.c_str());
  this->DisplayText->GetWidget()->AppendValue("\n\n");

  this->Script("%s yview end", 
               this->DisplayText->GetWidget()->GetWidgetName());
  
  this->CommandEntry->SetValue("");
}

//----------------------------------------------------------------------------
void vtkKWTclInteractor::AppendText(const char* text)
{
  this->DisplayText->GetWidget()->AppendValue(text);
  this->DisplayText->GetWidget()->AppendValue("\n");
}

//----------------------------------------------------------------------------
void vtkKWTclInteractor::DownCallback()
{
  if ( ! this->IsCreated())
    {
    return;
    }
  
  if (this->CommandIndex < this->TagNumber-1)
    {
    this->CommandIndex++;
    this->Script("set commandString [lindex $commandList %d]",
                 this->CommandIndex);
    this->Script("%s delete 0 end", this->CommandEntry->GetWidgetName());
    this->Script("%s insert end $commandString",
                 this->CommandEntry->GetWidgetName());
    }
  else
    {
    this->CommandEntry->SetValue("");
    }
}

//----------------------------------------------------------------------------
void vtkKWTclInteractor::UpCallback()
{
  if ( ! this->IsCreated())
    {
    return;
    }
  
  if (this->CommandIndex > 0)
    {
    this->CommandIndex--;
    this->Script("set commandString [lindex $commandList %d]",
                 this->CommandIndex);
    this->Script("%s delete 0 end", this->CommandEntry->GetWidgetName());
    this->Script("%s insert end $commandString",
                 this->CommandEntry->GetWidgetName());
    }
}

//----------------------------------------------------------------------------
void vtkKWTclInteractor::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->PropagateEnableState(this->ButtonFrame);
  this->PropagateEnableState(this->DismissButton);
  this->PropagateEnableState(this->CommandFrame);
  this->PropagateEnableState(this->CommandLabel);
  this->PropagateEnableState(this->CommandEntry);
  this->PropagateEnableState(this->DisplayText);
}

//----------------------------------------------------------------------------
void vtkKWTclInteractor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

