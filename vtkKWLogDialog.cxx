/*=========================================================================

  Module:    $RCSfile: vtkKWLogDialog.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkKWLogDialog.h"
#include "vtkKWLogWidget.h"
#include "vtkKWApplication.h"
#include "vtkObjectFactory.h"
#include "vtkKWPushButton.h"

#include <vtksys/stl/string>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWLogDialog );
vtkCxxRevisionMacro(vtkKWLogDialog, "$Revision: 1.5 $");

//----------------------------------------------------------------------------
vtkKWLogDialog::vtkKWLogDialog()
{
  this->LogWidget = vtkKWLogWidget::New();

  this->Options = 
    vtkKWMessageDialog::YesDefault | vtkKWMessageDialog::Resizable;
  this->Modal = 0;
}

//----------------------------------------------------------------------------
vtkKWLogDialog::~vtkKWLogDialog()
{
  if (this->LogWidget)
    {
    this->LogWidget->Delete();  
    }
}

//----------------------------------------------------------------------------
void vtkKWLogDialog::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }
  
  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  this->SetMinimumSize(400, 450);
  this->SetSize(650, 550);

  vtksys_stl::string title;
  if (this->GetApplication()->GetName())
    {
    title += this->GetApplication()->GetName();
    title += ": ";
    }
  title += "Log Viewer";
  this->SetTitle(title.c_str());
  
  // Record viewer

  if (!this->LogWidget)
    {
    this->LogWidget = vtkKWLogWidget::New();
    }
  this->LogWidget->SetParent(this->GetBottomFrame());
  this->LogWidget->Create();
  this->Script("pack %s -anchor nw -fill both -expand true -padx 2 -pady 2",
               this->LogWidget->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkKWLogDialog::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  if(this->LogWidget)
    {
    this->LogWidget->PrintSelf(os, indent);
    }
}
