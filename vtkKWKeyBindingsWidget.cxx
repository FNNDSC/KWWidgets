/*=========================================================================

  Module:    $RCSfile: vtkKWKeyBindingsWidget.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkKWKeyBindingsWidget.h"

#include "vtkKWApplication.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWIcon.h"
#include "vtkKWInternationalization.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWPushButton.h"
#include "vtkKWText.h"
#include "vtkKWTextWithScrollbars.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWToolbar.h"
#include "vtkKWKeyBindingsManager.h"

#include "vtkObjectFactory.h"

#include <vtksys/SystemTools.hxx>
#include <vtksys/ios/sstream>
#include <vtksys/stl/string>
#include <vtksys/stl/vector>
#include <vtksys/stl/map>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWKeyBindingsWidget );
vtkCxxRevisionMacro(vtkKWKeyBindingsWidget, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
class vtkKWKeyBindingsWidgetInternals
{
public:
};

//----------------------------------------------------------------------------
vtkKWKeyBindingsWidget::vtkKWKeyBindingsWidget()
{  
  this->Internals          = new vtkKWKeyBindingsWidgetInternals;

  this->Toolbar            = NULL;
  this->KeyBindingsList    = NULL;
  this->SaveButton         = NULL;
  this->DetailFrame        = NULL;
  this->DetailText         = NULL;
  this->KeyBindingsManager = NULL;
}

//----------------------------------------------------------------------------
vtkKWKeyBindingsWidget::~vtkKWKeyBindingsWidget()
{
  if (this->KeyBindingsManager )
    {
    this->SetKeyBindingsManager(NULL);
    }
  if (this->Internals)
    {
    delete this->Internals;
    }
  if (this->KeyBindingsList)
    { 
    this->KeyBindingsList->Delete();
    }
  if (this->SaveButton)
    {
    this->SaveButton->Delete();
    }
  if (this->DetailFrame)
    {
    this->DetailFrame->Delete();
    }
  if (this->DetailText)
    {
    this->DetailText->Delete();
    }
  if (this->Toolbar)
    {
    this->Toolbar->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkKWKeyBindingsWidget::SetKeyBindingsManager(vtkKWKeyBindingsManager *arg)
{
  if (this->KeyBindingsManager == arg)
    {
    return;
    }

  if (this->KeyBindingsManager)
    {
    this->KeyBindingsManager->UnRegister(this);
    }
    
  this->KeyBindingsManager = arg;

  if (this->KeyBindingsManager)
    {
    this->KeyBindingsManager->Register(this);
    }

  this->Update();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkKWKeyBindingsWidget::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  // Setup toolbar

  if (!this->Toolbar)
    {
    this->Toolbar = vtkKWToolbar::New();
    }
  this->Toolbar->SetParent(this);
  this->Toolbar->Create();
  this->Toolbar->SetToolbarAspectToFlat();
  this->Toolbar->SetWidgetsAspectToFlat();

  this->Script("pack %s -side top -anchor nw -padx 0 -pady 0",
               this->Toolbar->GetWidgetName());
  
  // Save Button

  if (!this->SaveButton)
    {
    this->SaveButton = vtkKWLoadSaveButton::New();
    }
  this->SaveButton->SetParent(this->Toolbar->GetFrame());
  this->SaveButton->Create();
  this->SaveButton->SetImageToPredefinedIcon(
    vtkKWIcon::IconNuvola16x16ActionsFileSave);
  this->SaveButton->SetBalloonHelpString(
    "Write shortcuts to a tab-delimited text file");
  this->SaveButton->SetCommand(this, "WriteKeyBindingsToFileCallback");
  this->SaveButton->GetLoadSaveDialog()->SaveDialogOn();
  this->SaveButton->GetLoadSaveDialog()->SetDefaultExtension(".txt");
  this->SaveButton->GetLoadSaveDialog()->SetFileTypes("{ {Text} {*.txt} }");
  this->SaveButton->GetLoadSaveDialog()->RetrieveLastPathFromRegistry(
    "OpenPath");
  this->Toolbar->AddWidget(this->SaveButton);

  // Setup multicolumn list

  if (!this->KeyBindingsList)
    {
    this->KeyBindingsList = vtkKWMultiColumnListWithScrollbars::New();
    }
  this->KeyBindingsList->SetParent(this);
  this->KeyBindingsList->Create();

  vtkKWMultiColumnList *tablelist = this->KeyBindingsList->GetWidget();
  tablelist->MovableColumnsOn();
  tablelist->SetWidth(0);
  tablelist->SetHeight(21);
  tablelist->ExportSelectionOff();
  tablelist->SetColumnSeparatorsVisibility(0);
  tablelist->SetStripeHeight(0);
  tablelist->ColorSortedColumnOn();
  tablelist->ClearStripeBackgroundColor();
  tablelist->SetRowSpacing(0);
  tablelist->SetSelectionModeToSingle();
  tablelist->SetSelectionChangedCommand(this, "SelectionChangedCallback");

  int col_index;

  col_index = tablelist->AddColumn("Key");

  col_index = tablelist->AddColumn("Context");

  col_index = tablelist->AddColumn("Description");

  for (int i = 0; i < tablelist->GetNumberOfColumns(); i++)
    {
    tablelist->SetColumnEditable(i, 0);
    }

  this->Script(
    "pack %s -side top -fill both -expand true -padx 0 -pady 2",
    this->KeyBindingsList->GetWidgetName());
  
  // Description Text Frame

  if (!this->DetailFrame)
    {
    this->DetailFrame = vtkKWFrameWithLabel::New();
    }
  this->DetailFrame->SetParent(this);
  this->DetailFrame->Create();
  this->DetailFrame->SetLabelText("Selected key binding Description");

#if 0
  this->Script("pack %s -side top -anchor nw  -fill x  -padx 0 -pady 0",
               this->DetailFrame->GetWidgetName());
#endif
  
  // Description Text Box

  if (!this->DetailText)
    {
    this->DetailText = vtkKWTextWithScrollbars::New();
    }
  this->DetailText->SetParent(this->DetailFrame->GetFrame());
  this->DetailText->Create();
  this->DetailText->GetWidget()->SetHeight(8);
  this->DetailText->GetWidget()->SetReadOnly(1);

  // Not used yet
  this->Script("pack %s -side top -fill both -expand true",
               this->DetailText->GetWidgetName());
  
  this->Update();
}

//----------------------------------------------------------------------------
void vtkKWKeyBindingsWidget::Update()
{
  if (!this->IsCreated())
    {
    return;
    }

  this->UpdateEnableState();

  if (!this->KeyBindingsList || !this->KeyBindingsManager)
    {
    return;
    }

  vtkKWMultiColumnList *tablelist = this->KeyBindingsList->GetWidget();
  tablelist->DeleteAllRows();

  typedef vtksys_stl::map<vtksys_stl::string, vtksys_stl::string> 
    StringToStringContainer;
  typedef vtksys_stl::map<vtksys_stl::string, vtksys_stl::string>::iterator 
    StringToStringIterator
;
  typedef vtksys_stl::map<vtksys_stl::string, StringToStringContainer> 
    StringToStringToStringContainer;
  typedef vtksys_stl::map<vtksys_stl::string,StringToStringContainer>::iterator 
    StringToStringToStringIterator;

  StringToStringToStringContainer keybindings;
  
  // Iterate over all key bindings and store unique ones

  int nb_targets = this->KeyBindingsManager->GetNumberOfTargets();
  int t;
  for (t = 0; t < nb_targets; ++t)
    {
    vtkObject *target = this->KeyBindingsManager->GetNthTarget(t);
    int nb_bindings = this->KeyBindingsManager->GetNumberOfBindings(target);
    int b;
    for (b = 0; b < nb_bindings; ++b)
      {
      const char *binding = this->KeyBindingsManager->GetNthBinding(target, b);
      int nb_callback_objects = 
        this->KeyBindingsManager->GetNumberOfCallbackObjects(target, binding);
      int o;
      for (o = 0; o < nb_callback_objects; ++o)
        {
        vtkObject *callback_object = 
          this->KeyBindingsManager->GetNthCallbackObject(target, binding, o);
        int nb_key_bindings = this->KeyBindingsManager->GetNumberOfKeyBindings(
          target, binding, callback_object);
        int k;
        for (k = 0; k < nb_key_bindings; ++k)
          {
          const char *context = this->KeyBindingsManager->GetNthContext(
            target, binding, callback_object, k);
          const char *desc = this->KeyBindingsManager->GetNthDescription(
            target, binding, callback_object, k);

          // Store unique key bindings

          keybindings[context][binding] = desc;
          }
        }
      }
    }

  // Iterate over unique context/binding/description bindings
  // and populate the list

  int row = 0;
  StringToStringToStringIterator it = keybindings.begin();
  StringToStringToStringIterator end = keybindings.end();
  for (;it != end; ++it)
    {
    StringToStringIterator it2 = (it->second).begin();
    StringToStringIterator end2 = (it->second).end();
    for (;it2 != end2; ++it2)
      {
      const char *binding = 
        this->KeyBindingsManager->GetPrettyBinding((it2->first).c_str());
      const char *context = 
        this->KeyBindingsManager->GetPrettyContext((it->first).c_str());
      const char *desc = (it2->second).c_str();
      tablelist->AddRow();
      tablelist->SetCellText(row, 0, binding);
      tablelist->SetCellText(row, 1, context);
      tablelist->SetCellText(row, 2, desc);
      ++row;
      }
    }
}

//----------------------------------------------------------------------------
int vtkKWKeyBindingsWidget::WriteKeyBindingsToStream(ostream& os)
{
  if (!this->KeyBindingsList || !this->KeyBindingsList->IsCreated())
    {
    return 0;
    }

  // Write all keybindings

  vtkKWMultiColumnList *keybinding_list = this->KeyBindingsList->GetWidget();
  int numrows = keybinding_list->GetNumberOfRows();

  for (int i = 0; i < numrows; i++)
    {
    const char *tmp = keybinding_list->GetCellText(i, 0);
    if (tmp)
      {
      vtksys_stl::string binding(tmp);

      tmp = keybinding_list->GetCellText(i, 1);
      vtksys_stl::string context(tmp ? tmp : "");

      tmp = keybinding_list->GetCellText(i, 2);
      vtksys_stl::string desc(tmp ? tmp : "");

      os << binding << '\t' << context << '\t' << desc << endl;
      }
    }
    
  return 1;         
}

//----------------------------------------------------------------------------
int vtkKWKeyBindingsWidget::WriteKeyBindingsToFile(const char* filename)
{
  // Make sure the user specified a filename

  if (!filename || !(*filename))
    {
    vtkErrorMacro(<< "Please specify a valid file name!");
    return 0;
    }
    
  // Open text file

  ofstream fout(filename, ios::out | ios::trunc);
  if (fout.fail())
    {
    vtkWarningMacro(<< "Unable to open file to write: " << filename);
    return 0;
    }
  
  // Write all keybindings

  int res = this->WriteKeyBindingsToStream(fout);
  fout.close();
    
  return res;
}

//----------------------------------------------------------------------------
void vtkKWKeyBindingsWidget::SelectionChangedCallback()
{
}

//----------------------------------------------------------------------------
void vtkKWKeyBindingsWidget::WriteKeyBindingsToFileCallback()
{
  if (this->SaveButton && this->SaveButton->IsCreated())
    {
    this->SaveButton->SetText(NULL);
    if (this->SaveButton->GetLoadSaveDialog()->GetStatus() ==
        vtkKWDialog::StatusOK)
      {
      this->SaveButton->GetLoadSaveDialog()->SaveLastPathToRegistry(
        "OpenPath");
      this->WriteKeyBindingsToFile(this->SaveButton->GetFileName());
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWKeyBindingsWidget::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->PropagateEnableState(this->KeyBindingsList);
  this->PropagateEnableState(this->Toolbar);
  this->PropagateEnableState(this->SaveButton);
}

//----------------------------------------------------------------------------
void vtkKWKeyBindingsWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "KeyBindingsManager: ";
  if (this->KeyBindingsManager)
    {
    os << this->KeyBindingsManager << endl;
    }
  else
    {
    os << "None" << endl;
    }
}
