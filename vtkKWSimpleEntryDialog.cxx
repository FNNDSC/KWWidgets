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
#include "vtkKWMessage.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWPushButton.h"

#include "vtkObjectFactory.h"

//----------------------------------------------------------------------------

vtkStandardNewMacro( vtkKWSimpleEntryDialog );
vtkCxxRevisionMacro(vtkKWSimpleEntryDialog, "$Revision: 1.16 $");

//----------------------------------------------------------------------------
vtkKWSimpleEntryDialog::vtkKWSimpleEntryDialog()
{
  this->Entry = vtkKWEntryWithLabel::New();
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
void vtkKWSimpleEntryDialog::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro("SimpleEntryDialog already created");
    return;
    }

  this->Superclass::CreateWidget();
  
  this->Entry->SetParent(this->MessageDialogFrame);
  this->Entry->Create();

  this->Entry->SetBinding("<Return>", this, "OK");
  this->Entry->GetWidget()->SetBinding("<Return>", this, "OK");
  this->Entry->SetBinding("<Escape>", this, "Cancel");
  this->Entry->GetWidget()->SetBinding("<Escape>", this, "Cancel");

  this->GetOKButton()->SetBinding("<Return>", this, "OK");
  this->GetCancelButton()->SetBinding("<Return>", this, "Cancel");

  this->Pack();
}

//----------------------------------------------------------------------------
void vtkKWSimpleEntryDialog::Pack()
{
  this->Superclass::Pack();

  if (this->Entry && this->Entry->IsCreated())
    {
    this->Script("pack %s -side top -before %s -padx 4 -fill x -expand yes", 
                 this->Entry->GetWidgetName(), 
                 this->BottomFrame->GetWidgetName());
    }
}

//----------------------------------------------------------------------------
int vtkKWSimpleEntryDialog::Invoke()
{
  if (this->IsCreated())
    {
    this->Entry->GetWidget()->Focus();
    }

  return this->Superclass::Invoke();
}

//----------------------------------------------------------------------------
void vtkKWSimpleEntryDialog::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Entry: " << this->Entry << endl;
}

