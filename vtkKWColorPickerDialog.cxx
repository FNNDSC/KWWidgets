/*=========================================================================

  Module:    $RCSfile: vtkKWColorPickerDialog.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkKWColorPickerDialog.h"
#include "vtkKWColorPickerWidget.h"
#include "vtkKWColorPresetSelector.h"
#include "vtkKWApplication.h"
#include "vtkObjectFactory.h"
#include "vtkKWPushButton.h"

#include <vtksys/stl/string>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWColorPickerDialog );
vtkCxxRevisionMacro(vtkKWColorPickerDialog, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkKWColorPickerDialog::vtkKWColorPickerDialog()
{
  this->ColorPickerWidget = vtkKWColorPickerWidget::New();
  this->OKButton = NULL;
  this->CancelButton = NULL;
}

//----------------------------------------------------------------------------
vtkKWColorPickerDialog::~vtkKWColorPickerDialog()
{
  if (this->ColorPickerWidget)
    {
    this->ColorPickerWidget->Delete();  
    }

  if (this->OKButton)
    {
    this->OKButton->Delete();
    }

  if (this->CancelButton)
    {
    this->CancelButton->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkKWColorPickerDialog::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }
  
  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  this->SetResizable(0, 0);
  this->SetSize(632, 334);
  vtksys_stl::string title;
  if (this->GetApplication()->GetName())
    {
    title += this->GetApplication()->GetName();
    title += ": ";
    }
  title += "Color Picker";
  this->SetTitle(title.c_str());
  
  // Color Picker

  if (!this->ColorPickerWidget)
    {
    this->ColorPickerWidget = vtkKWColorPickerWidget::New();
    }
  this->ColorPickerWidget->SetParent(this);
  this->ColorPickerWidget->GetFavoritesColorPresetSelector()->SetRegistryKey(
    "Favorites");
  this->ColorPickerWidget->GetHistoryColorPresetSelector()->SetRegistryKey(
    "History");
  this->ColorPickerWidget->Create();
  this->ColorPickerWidget->SetBorderWidth(2);
  this->ColorPickerWidget->SetReliefToGroove();

  this->Script(
    "pack %s -side top -anchor center -fill both -expand true -padx 2 -pady 2",
    this->ColorPickerWidget->GetWidgetName());

  // OK button

  if (!this->OKButton)
    {
    this->OKButton = vtkKWPushButton::New();
    }
  this->OKButton->SetParent(this);
  this->OKButton->Create();
  this->OKButton->SetWidth(20);
  this->OKButton->SetText("OK");
  this->OKButton->SetCommand(this, "OK");
  this->OKButton->SetConfigurationOptionAsInt("-takefocus", 0);

  this->Script("pack %s -side right -anchor e -padx 2 -pady 2 -expand n",
               this->OKButton->GetWidgetName());

  // Cancel button

  if (!this->CancelButton)
    {
    this->CancelButton = vtkKWPushButton::New();
    }
  this->CancelButton->SetParent(this);
  this->CancelButton->Create();
  this->CancelButton->SetWidth(20);
  this->CancelButton->SetText("Cancel");
  this->CancelButton->SetCommand(this, "Cancel");
  this->CancelButton->SetConfigurationOptionAsInt("-takefocus", 0);

  this->Script("pack %s -side right -anchor e -pady 2 -padx 2 -expand n",
               this->CancelButton->GetWidgetName());

  this->AddBinding("<Return>", this, "OK");
  this->AddBinding("<Escape>", this, "Cancel");
}

//----------------------------------------------------------------------------
void vtkKWColorPickerDialog::OK()
{
  this->Superclass::OK();

  // We accepted that color, save it to the history

  if (this->ColorPickerWidget)
    {
    vtkKWColorPresetSelector *history = 
      this->ColorPickerWidget->GetHistoryColorPresetSelector();

    int id = history->InsertPreset(history->GetIdOfNthPreset(0));
    if (id >= 0)
      {
      double r, g, b;
      this->ColorPickerWidget->GetNewColorAsRGB(r, g, b);
      history->SetPresetColorAsRGB(id, r, g, b);
      if (!history->GetPresetComment(id) ||
          history->HasPresetCommentAsHexadecimalRGB(id))
        {
        history->SetPresetCommentAsHexadecimalRGB(id);
        }
      history->RemoveDuplicatedColors();
      history->SavePresetsToRegistry();
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWColorPickerDialog::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->PropagateEnableState(this->ColorPickerWidget);
  this->PropagateEnableState(this->OKButton);
  this->PropagateEnableState(this->CancelButton);
}

//----------------------------------------------------------------------------
void vtkKWColorPickerDialog::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  if(this->ColorPickerWidget)
    {
    this->ColorPickerWidget->PrintSelf(os, indent);
    }
}
