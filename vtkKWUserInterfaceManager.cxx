/*=========================================================================

  Module:    $RCSfile: vtkKWUserInterfaceManager.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkKWUserInterfaceManager.h"

#include "vtkKWWidget.h"
#include "vtkKWUserInterfacePanel.h"
#include "vtkObjectFactory.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkKWUserInterfaceManager, "$Revision: 1.22 $");

//----------------------------------------------------------------------------
class vtkKWUserInterfaceManagerInternals
{
public:

  typedef vtksys_stl::list<vtkKWUserInterfaceManager::PanelSlot*> PanelsContainer;
  typedef vtksys_stl::list<vtkKWUserInterfaceManager::PanelSlot*>::iterator PanelsContainerIterator;

  PanelsContainer Panels;
};

//----------------------------------------------------------------------------
vtkKWUserInterfaceManager::vtkKWUserInterfaceManager()
{
  this->IdCounter = 0;
  this->ManagerIsCreated = 0;

  this->Internals = new vtkKWUserInterfaceManagerInternals;
}

//----------------------------------------------------------------------------
vtkKWUserInterfaceManager::~vtkKWUserInterfaceManager()
{
  // Delete all panels

  this->RemoveAllPanels();
  if (this->Internals)
    {
    delete this->Internals;
    }
}

//----------------------------------------------------------------------------
vtkKWUserInterfaceManager::PanelSlot* 
vtkKWUserInterfaceManager::GetPanelSlot(vtkKWUserInterfacePanel *panel)
{
  if (this->Internals && panel)
    {
    vtkKWUserInterfaceManagerInternals::PanelsContainerIterator it = 
      this->Internals->Panels.begin();
    vtkKWUserInterfaceManagerInternals::PanelsContainerIterator end = 
      this->Internals->Panels.end();
    for (; it != end; ++it)
      {
      if (*it && (*it)->Panel == panel)
        {
        return *it;
        }
      }
    }

  return NULL;
}

//----------------------------------------------------------------------------
vtkKWUserInterfaceManager::PanelSlot* 
vtkKWUserInterfaceManager::GetPanelSlot(int id)
{
  if (this->Internals)
    {
    vtkKWUserInterfaceManagerInternals::PanelsContainerIterator it = 
      this->Internals->Panels.begin();
    vtkKWUserInterfaceManagerInternals::PanelsContainerIterator end = 
      this->Internals->Panels.end();
    for (; it != end; ++it)
      {
      if (*it && (*it)->Id == id)
        {
        return *it;
        }
      }
    }

  return NULL;
}

//----------------------------------------------------------------------------
vtkKWUserInterfaceManager::PanelSlot* 
vtkKWUserInterfaceManager::GetPanelSlot(const char *panel_name)
{
  if (this->Internals && panel_name)
    {
    vtkKWUserInterfaceManagerInternals::PanelsContainerIterator it = 
      this->Internals->Panels.begin();
    vtkKWUserInterfaceManagerInternals::PanelsContainerIterator end = 
      this->Internals->Panels.end();
    for (; it != end; ++it)
      {
      if (*it && (*it)->Panel && (*it)->Panel->GetName() &&
          !strcmp((*it)->Panel->GetName(), panel_name))
        {
        return *it;
        }
      }
    }

  return NULL;
}

//----------------------------------------------------------------------------
vtkKWUserInterfaceManager::PanelSlot* 
vtkKWUserInterfaceManager::GetNthPanelSlot(int rank)
{
  if (this->Internals && rank >= 0 && rank < this->GetNumberOfPanels())
    {
    vtkKWUserInterfaceManagerInternals::PanelsContainerIterator it = 
      this->Internals->Panels.begin();
    vtkKWUserInterfaceManagerInternals::PanelsContainerIterator end = 
      this->Internals->Panels.end();
    for (; it != end; ++it)
      {
      if (*it && !rank--)
        {
        return *it;
        }
      }
    }

  return NULL;
}

//----------------------------------------------------------------------------
int vtkKWUserInterfaceManager::HasPanel(vtkKWUserInterfacePanel *panel)
{
  return this->GetPanelSlot(panel) ? 1 : 0;
}

//----------------------------------------------------------------------------
int vtkKWUserInterfaceManager::GetPanelId(vtkKWUserInterfacePanel *panel)
{
  vtkKWUserInterfaceManager::PanelSlot *panel_slot = this->GetPanelSlot(panel);
  if (!panel_slot)
    {
    return -1;
    }

  return panel_slot->Id;
}

//----------------------------------------------------------------------------
vtkKWUserInterfacePanel* vtkKWUserInterfaceManager::GetPanel(int id)
{
  vtkKWUserInterfaceManager::PanelSlot *panel_slot = this->GetPanelSlot(id);
  if (!panel_slot)
    {
    return NULL;
    }

  return panel_slot->Panel;
}

//----------------------------------------------------------------------------
vtkKWUserInterfacePanel* vtkKWUserInterfaceManager::GetPanel(
  const char *panel_name)
{
  vtkKWUserInterfaceManager::PanelSlot *panel_slot = 
    this->GetPanelSlot(panel_name);
  if (!panel_slot)
    {
    return NULL;
    }

  return panel_slot->Panel;
}

//----------------------------------------------------------------------------
vtkKWUserInterfacePanel* vtkKWUserInterfaceManager::GetNthPanel(int rank)
{
  vtkKWUserInterfaceManager::PanelSlot *panel_slot = 
    this->GetNthPanelSlot(rank);
  if (!panel_slot)
    {
    return NULL;
    }

  return panel_slot->Panel;
}

//----------------------------------------------------------------------------
void vtkKWUserInterfaceManager::Create()
{
  // Set the application

  if (this->IsCreated())
    {
    vtkErrorMacro("The manager is already created");
    return;
    }

  this->ManagerIsCreated = 1;
}

// ---------------------------------------------------------------------------
int vtkKWUserInterfaceManager::IsCreated()
{
  return (this->GetApplication() != NULL && this->ManagerIsCreated);
}

//----------------------------------------------------------------------------
void vtkKWUserInterfaceManager::SetEnabled(int arg)
{
  if (this->Internals)
    {
    vtkKWUserInterfaceManagerInternals::PanelsContainerIterator it = 
      this->Internals->Panels.begin();
    vtkKWUserInterfaceManagerInternals::PanelsContainerIterator end = 
      this->Internals->Panels.end();
    for (; it != end; ++it)
      {
      if (*it && (*it)->Panel)
        {
        (*it)->Panel->SetEnabled(arg);
        (*it)->Panel->Update();
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWUserInterfaceManager::Update()
{
  if (this->Internals)
    {
    vtkKWUserInterfaceManagerInternals::PanelsContainerIterator it = 
      this->Internals->Panels.begin();
    vtkKWUserInterfaceManagerInternals::PanelsContainerIterator end = 
      this->Internals->Panels.end();
    for (; it != end; ++it)
      {
      if (*it && (*it)->Panel)
        {
        (*it)->Panel->Update();
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWUserInterfaceManager::UpdateEnableState()
{
  if (this->Internals)
    {
    vtkKWUserInterfaceManagerInternals::PanelsContainerIterator it = 
      this->Internals->Panels.begin();
    vtkKWUserInterfaceManagerInternals::PanelsContainerIterator end = 
      this->Internals->Panels.end();
    for (; it != end; ++it)
      {
      if (*it && (*it)->Panel)
        {
        (*it)->Panel->UpdateEnableState();
        }
      }
    }
}

// ----------------------------------------------------------------------------
int vtkKWUserInterfaceManager::GetNumberOfPanels()
{
  return this->Internals ? this->Internals->Panels.size() : 0;
}

//----------------------------------------------------------------------------
int vtkKWUserInterfaceManager::AddPanel(vtkKWUserInterfacePanel *panel)
{
  if (!panel)
    {
    vtkErrorMacro("Can not add a NULL panel to the manager.");
    return -1;
    }
  
  // Don't allow duplicates (return silently though)

  if (this->HasPanel(panel))
    {
    return this->GetPanelId(panel);
    }

  // Add the panel slot to the manager

  vtkKWUserInterfaceManager::PanelSlot *panel_slot = 
    new vtkKWUserInterfaceManager::PanelSlot;

  this->Internals->Panels.push_back(panel_slot);
  
  // Each panel has a unique ID in the manager lifetime

  panel_slot->Panel = panel;
  panel_slot->Id = this->IdCounter++;

  // For convenience, make sure the panel use this instance.
  // Note that vtkKWUserInterfacePanel::SetUserInterfaceManager also
  // calls the current method for convenience.

  panel_slot->Panel->SetUserInterfaceManager(this);

  // Use the same application (for convenience)

  if (!this->GetApplication() && panel->GetApplication())
    {
    this->SetApplication(panel->GetApplication());
    }

  panel_slot->Panel->Register(this);

  this->NumberOfPanelsChanged();

  return panel_slot->Id;
}

//----------------------------------------------------------------------------
int vtkKWUserInterfaceManager::RemovePanel(vtkKWUserInterfacePanel *panel)
{
  if (!panel)
    {
    vtkErrorMacro("Can not remove a NULL panel from the manager.");
    return 0;
    }
  
  // Silently returns if the panel has been removed already

  if (!this->HasPanel(panel))
    {
    return 1;
    }

  // Remove the page widgets from the interface

  this->RemovePageWidgets(panel);

  // Find the panel in the manager

  vtkKWUserInterfaceManager::PanelSlot *panel_slot = this->GetPanelSlot(panel);

  vtkKWUserInterfaceManagerInternals::PanelsContainerIterator pos = 
    vtksys_stl::find(this->Internals->Panels.begin(),
                 this->Internals->Panels.end(),
                 panel_slot);

  if (pos == this->Internals->Panels.end())
    {
    vtkErrorMacro("Error while removing a panel from the manager "
                  "(can not find the panel).");
    return 0;
    }

  // Remove the panel from the container

  this->Internals->Panels.erase(pos);

  // For convenience, make sure the panel does not use this instance anymore.
  // Note that vtkKWUserInterfacePanel::SetUserInterfaceManager also
  // calls the current method for convenience.

  panel_slot->Panel->SetUserInterfaceManager(NULL);

  panel_slot->Panel->UnRegister(this);

  // Delete the panel slot

  delete panel_slot;

  this->NumberOfPanelsChanged();

  return 1;
}

//----------------------------------------------------------------------------
void vtkKWUserInterfaceManager::RemoveAllPanels()
{
  while (this->GetNumberOfPanels())
    {
    this->RemovePanel(this->GetNthPanel(0));
    }
}

//----------------------------------------------------------------------------
void vtkKWUserInterfaceManager::ShowAllPanels()
{
  if (this->Internals)
    {
    vtkKWUserInterfaceManagerInternals::PanelsContainerIterator it = 
      this->Internals->Panels.begin();
    vtkKWUserInterfaceManagerInternals::PanelsContainerIterator end = 
      this->Internals->Panels.end();
    for (; it != end; ++it)
      {
      if (*it && (*it)->Panel)
        {
        this->ShowPanel((*it)->Panel);
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWUserInterfaceManager::HideAllPanels()
{
  if (this->Internals)
    {
    vtkKWUserInterfaceManagerInternals::PanelsContainerIterator it = 
      this->Internals->Panels.begin();
    vtkKWUserInterfaceManagerInternals::PanelsContainerIterator end = 
      this->Internals->Panels.end();
    for (; it != end; ++it)
      {
      if (*it && (*it)->Panel)
        {
        this->HidePanel((*it)->Panel);
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWUserInterfaceManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
