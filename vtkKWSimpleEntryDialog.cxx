/*=========================================================================

  Module:    $RCSfile: vtkKWSimpleEntryDialog.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWSimpleEntryDialog.h"

#include "vtkKWEntry.h"
#include "vtkKWFrame.h"
#include "vtkKWLabel.h"
#include "vtkKWLabeledEntry.h"
#include "vtkObjectFactory.h"

//----------------------------------------------------------------------------

vtkStandardNewMacro( vtkKWSimpleEntryDialog );
vtkCxxRevisionMacro(vtkKWSimpleEntryDialog, "$Revision: 1.5 $");

int vtkKWSimpleEntryDialogCommand(ClientData cd, Tcl_Interp *interp,
                                  int argc, char *argv[]);

//----------------------------------------------------------------------------
vtkKWSimpleEntryDialog::vtkKWSimpleEntryDialog()
{
  this->CommandFunction = vtkKWSimpleEntryDialogCommand;

  this->Entry = vtkKWLabeledEntry::New();
}

//----------------------------------------------------------------------------
vtkKWSimpleEntryDialog::~vtkKWSimpleEntryDialog()
{
  if (this->Entry)
    {
    this->Entry->Delete();
    this->Entry = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkKWSimpleEntryDialog::Create(vtkKWApplication *app, const char *args)
{
  // Invoke super method

  this->Superclass::Create(app, args);
  
  this->Entry->SetParent(this->MessageDialogFrame);
  this->Entry->Create(app, 0);

  this->Script("pack %s -side top -after %s -padx 4 -fill x -expand yes", 
               this->Entry->GetWidgetName(), this->Label->GetWidgetName());

  this->Script("bind %s <Return> {%s OK}",
               this->Entry->GetEntry()->GetWidgetName(), this->GetTclName());

  this->Script("bind %s <Escape> {%s Cancel}",
               this->Entry->GetEntry()->GetWidgetName(), this->GetTclName());
}

//----------------------------------------------------------------------------
int vtkKWSimpleEntryDialog::Invoke()
{
  if (this->IsCreated())
    {
    this->Entry->GetEntry()->Focus();
    }

  return this->Superclass::Invoke();
}

//----------------------------------------------------------------------------
void vtkKWSimpleEntryDialog::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Entry: " << this->Entry << endl;
}

