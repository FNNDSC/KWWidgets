/*=========================================================================

  Module:    $RCSfile: vtkKWUserInterfacePanel.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkKWUserInterfacePanel.h"

#include "vtkKWUserInterfaceManager.h"
#include "vtkObjectFactory.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWUserInterfacePanel);
vtkCxxRevisionMacro(vtkKWUserInterfacePanel, "$Revision: 1.17 $");

//----------------------------------------------------------------------------
vtkKWUserInterfacePanel::vtkKWUserInterfacePanel()
{
  this->UserInterfaceManager = NULL;
  this->Enabled = 1;
  this->Name = NULL;
  this->PanelIsCreated = 0;
}

//----------------------------------------------------------------------------
vtkKWUserInterfacePanel::~vtkKWUserInterfacePanel()
{
  this->SetUserInterfaceManager(NULL);
  this->SetName(NULL);
}

//----------------------------------------------------------------------------
void vtkKWUserInterfacePanel::SetUserInterfaceManager(vtkKWUserInterfaceManager *_arg)
{ 
  vtkDebugMacro(<< this->GetClassName() << " (" << this 
                << "): setting UserInterfaceManager to " << _arg);

  if (this->UserInterfaceManager == _arg)
    {
    return;
    }

  if (this->IsCreated() && _arg)
    {
    vtkErrorMacro("The interface manager cannot be changed once this panel "
                  "has been created.");
    return;
    }

  if (this->UserInterfaceManager != NULL) 
    { 
    this->UserInterfaceManager->RemovePanel(this);
    }

  this->UserInterfaceManager = _arg; 

  if (this->UserInterfaceManager != NULL) 
    { 
    // Use the same application (for convenience)

    if (!this->GetApplication() && 
        this->UserInterfaceManager->GetApplication())
      {
      this->SetApplication(this->UserInterfaceManager->GetApplication());
      }

    this->UserInterfaceManager->AddPanel(this);
    
    } 
  
  this->Modified(); 
} 

// ---------------------------------------------------------------------------
void vtkKWUserInterfacePanel::Create()
{
  if (this->IsCreated())
    {
    // Comment this one. Some user interface manager creates their panels
    // early on, or on the fly, so it is not too uncommon that this method
    // is called twice, without a need to be bothered about it.
    // vtkErrorMacro("The panel is already created");
    return;
    }

  this->PanelIsCreated = 1;

  // As a convenience, if the manager associated to this panel has not been
  // created yet, it is created now. This might be useful since concrete
  // implementation of panels are likely to request pages in Create(),
  // which require the manager to be created.

  if (this->UserInterfaceManager && !this->UserInterfaceManager->IsCreated())
    {
    this->UserInterfaceManager->Create();
    }

  // Do *not* call Update() here.
}

// ---------------------------------------------------------------------------
int vtkKWUserInterfacePanel::IsCreated()
{
  return (this->GetApplication() != NULL && this->PanelIsCreated);
}

//----------------------------------------------------------------------------
int vtkKWUserInterfacePanel::AddPage(const char *title, 
                                     const char *balloon, 
                                     vtkKWIcon *icon)
{
  if (this->UserInterfaceManager == NULL)
    {
    vtkErrorMacro("The UserInterfaceManager manager needs to be set before"
                  "a page can be added.");
    return -1;
    }

  return this->UserInterfaceManager->AddPage(this, title, balloon, icon);
}

//----------------------------------------------------------------------------
int vtkKWUserInterfacePanel::RemovePage(const char *title)
{
  if (this->UserInterfaceManager == NULL)
    {
    vtkErrorMacro("The UserInterfaceManager manager needs to be set before"
                  "a page can be removed.");
    return -1;
    }

  return this->UserInterfaceManager->RemovePage(this, title);
}

//----------------------------------------------------------------------------
vtkKWWidget* vtkKWUserInterfacePanel::GetPageWidget(int id)
{
  if (this->UserInterfaceManager == NULL)
    {
    vtkErrorMacro("The UserInterfaceManager manager needs to be set before"
                  "a page can be queried.");
    return NULL;
    }

  return this->UserInterfaceManager->GetPageWidget(id);
}

//----------------------------------------------------------------------------
vtkKWWidget* vtkKWUserInterfacePanel::GetPageWidget(const char *title)
{
  if (this->UserInterfaceManager == NULL)
    {
    vtkErrorMacro("The UserInterfaceManager manager needs to be set before "
                  "a page can be queried.");
    return NULL;
    }

  return this->UserInterfaceManager->GetPageWidget(this, title);
}

//----------------------------------------------------------------------------
vtkKWWidget* vtkKWUserInterfacePanel::GetPagesParentWidget()
{
  if (this->UserInterfaceManager == NULL)
    {
    vtkErrorMacro("The UserInterfaceManager manager needs to be set before "
                  "a the pages parent can be queried.");
    return NULL;
    }

  return this->UserInterfaceManager->GetPagesParentWidget(this);
}

//----------------------------------------------------------------------------
void vtkKWUserInterfacePanel::RaisePage(int id)
{
  if (this->UserInterfaceManager == NULL)
    {
    vtkErrorMacro("The UserInterfaceManager manager needs to be set before"
                  "a page can be raised.");
    return;
    }
  
  this->UserInterfaceManager->RaisePage(id);
}

//----------------------------------------------------------------------------
void vtkKWUserInterfacePanel::RaisePage(const char *title)
{
  if (this->UserInterfaceManager == NULL)
    {
    vtkErrorMacro("The UserInterfaceManager manager needs to be set before "
                  "a page can be raised.");
    return;
    }

  this->UserInterfaceManager->RaisePage(this, title);
}

//----------------------------------------------------------------------------
int vtkKWUserInterfacePanel::Show()
{
  if (this->UserInterfaceManager == NULL)
    {
    vtkErrorMacro("The UserInterfaceManager manager needs to be set before "
                  "all pages can be shown.");
    return 0;
    }

  return this->UserInterfaceManager->ShowPanel(this);
}

//----------------------------------------------------------------------------
int vtkKWUserInterfacePanel::IsVisible()
{
  if (this->UserInterfaceManager == NULL)
    {
    vtkErrorMacro("The UserInterfaceManager manager needs to be set before "
                  "pages can be checked for visibility.");
    return 0;
    }

  return this->UserInterfaceManager->IsPanelVisible(this);
}

//----------------------------------------------------------------------------
int vtkKWUserInterfacePanel::Raise()
{
  this->Show();
  if (this->UserInterfaceManager)
    {
    return this->UserInterfaceManager->RaisePanel(this);
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWUserInterfacePanel::Update()
{
  // Update the enable state

  this->UpdateEnableState();

  // Update the panel according to the manager (i.e. manager-specific changes)

  if (this->UserInterfaceManager)
    {
    this->UserInterfaceManager->UpdatePanel(this);
    }
}

//----------------------------------------------------------------------------
void vtkKWUserInterfacePanel::SetEnabled(int e)
{
  if ( this->Enabled == e )
    {
    return;
    }

  this->Enabled = e;
  this->UpdateEnableState();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkKWUserInterfacePanel::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "UserInterfaceManager: " << this->UserInterfaceManager << endl;

  os << indent << "Enabled: " << (this->Enabled ? "On" : "Off") << endl;

  os << indent << "Name: " << (this->Name ? this->Name : "(none)") << endl;
}

