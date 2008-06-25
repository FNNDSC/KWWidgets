/*=========================================================================

  Module:    $RCSfile: vtkKWTreeWithScrollbars.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkKWTreeWithScrollbars.h"
#include "vtkObjectFactory.h"
#include "vtkKWScrollbar.h"
#include "vtkKWTree.h"
#include "vtkKWPushButtonSet.h"
#include "vtkKWPushButton.h"
#include "vtkKWIcon.h"
#include "vtkKWInternationalization.h"
#include <vtksys/ios/sstream>

#include <vtksys/stl/string>

#define VTK_KWTWS_EXPAND_BUTTON_ID 0

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWTreeWithScrollbars);
vtkCxxRevisionMacro(vtkKWTreeWithScrollbars, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkKWTreeWithScrollbars::vtkKWTreeWithScrollbars()
{
  this->Widget = vtkKWTree::New();
  this->VerticalButtons = NULL;
  this->HorizontalButtons = NULL;
  this->ResizeButtonsVisibility = 0;
}

//----------------------------------------------------------------------------
vtkKWTreeWithScrollbars::~vtkKWTreeWithScrollbars()
{
  if (this->Widget)
    {
    this->Widget->Delete();
    this->Widget = NULL;
    }

  if (this->VerticalButtons)
    {
    this->VerticalButtons->Delete();
    this->VerticalButtons = NULL;
    }

  if (this->HorizontalButtons)
    {
    this->HorizontalButtons->Delete();
    this->HorizontalButtons = NULL;
    }
}

//----------------------------------------------------------------------------
vtkKWTree* vtkKWTreeWithScrollbars::GetWidget()
{
  return this->Widget;
}

//----------------------------------------------------------------------------
void vtkKWTreeWithScrollbars::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  // Create the widget

  this->Widget->SetParent(this);
  this->Widget->Create();
  if (!this->Widget->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " failed creating vtkKWTree");
    return;
    }

  // Vertical Buttons

  if (!this->VerticalButtons)
    {
    this->VerticalButtons = vtkKWPushButtonSet::New();
    }

  this->VerticalButtons->SetParent(this);
  this->VerticalButtons->PackHorizontallyOff();
  this->VerticalButtons->ExpandWidgetsOn();
  this->VerticalButtons->Create();

  vtkKWPushButton *pb = NULL;

  pb = this->VerticalButtons->AddWidget(VTK_KWTWS_EXPAND_BUTTON_ID);
  pb->SetBinding("<Shift-ButtonRelease-1>", 
                 this, "ShrinkTreeVerticallyCallback");
  pb->SetBinding("<ButtonRelease-1>",
                 this, "ExpandTreeVerticallyCallback");
  pb->SetImageToPredefinedIcon(vtkKWIcon::IconMoveV);
  pb->SetBalloonHelpString(
    ks_("Tree|Expand tree vertically. Shift + Click to shrink."));

  // Horizontal Buttons

  if (!this->HorizontalButtons)
    {
    this->HorizontalButtons = vtkKWPushButtonSet::New();
    }

  this->HorizontalButtons->SetParent(this);
  this->HorizontalButtons->PackHorizontallyOn();
  this->HorizontalButtons->ExpandWidgetsOn();
  this->HorizontalButtons->Create();

  pb = this->HorizontalButtons->AddWidget(VTK_KWTWS_EXPAND_BUTTON_ID);
  pb->SetBinding("<Shift-ButtonRelease-1>", 
                 this, "ShrinkTreeHorizontallyCallback");
  pb->SetBinding("<ButtonRelease-1>", 
                 this, "ExpandTreeHorizontallyCallback");
  pb->SetImageToPredefinedIcon(vtkKWIcon::IconMoveH);
  pb->SetBalloonHelpString(
    ks_("Tree|Expand tree horizontally. Shift + Click to shrink."));

  // Now that we have been created, make sure we are associated to
  // the scrollbars created by the super, *if any*

  this->AssociateVerticalScrollbarToWidget(this->Widget);
  this->AssociateHorizontalScrollbarToWidget(this->Widget);

  // Pack

  this->Pack();
  this->UpdateButtonsVisibility();

  // Update enable state

  this->UpdateEnableState();
}

//----------------------------------------------------------------------------
void vtkKWTreeWithScrollbars::CreateVerticalScrollbar()
{
  this->Superclass::CreateVerticalScrollbar();
  this->AssociateVerticalScrollbarToWidget(this->Widget);
}

//----------------------------------------------------------------------------
void vtkKWTreeWithScrollbars::CreateHorizontalScrollbar()
{
  this->Superclass::CreateHorizontalScrollbar();
  this->AssociateHorizontalScrollbarToWidget(this->Widget);
}

//----------------------------------------------------------------------------
void vtkKWTreeWithScrollbars::Pack()
{
  this->PackScrollbarsWithWidget(this->Widget);

  vtksys_ios::ostringstream tk_cmd;

  if (this->VerticalScrollbarVisibility && 
      this->VerticalButtons && this->VerticalButtons->IsCreated())
    {
    tk_cmd << "grid " << this->VerticalButtons->GetWidgetName() 
           << " -row 2 -column 3 -sticky ew" << endl;
    }

  if (this->HorizontalScrollbarVisibility && 
      this->HorizontalButtons && this->HorizontalButtons->IsCreated())
    {
    tk_cmd << "grid " << this->HorizontalButtons->GetWidgetName() 
           << " -row 3 -column 2 -sticky ns" << endl;
    }

  this->Script(tk_cmd.str().c_str());
}

//----------------------------------------------------------------------------
void vtkKWTreeWithScrollbars::ExpandTreeVerticallyCallback()
{
  if (this->Widget && this->Widget->IsCreated())
    {
    this->Widget->SetHeight(this->Widget->GetHeight() + 10);
    }
}

//----------------------------------------------------------------------------
void vtkKWTreeWithScrollbars::ShrinkTreeVerticallyCallback()
{
  if (this->Widget && this->Widget->IsCreated())
    {
    int height = this->Widget->GetHeight();
    if (height > 10)
      {
      this->Widget->SetHeight(height - 10);
      }
    else if (height > 1)
      {
      this->Widget->SetHeight(height >> 1);
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWTreeWithScrollbars::ExpandTreeHorizontallyCallback()
{
  if (this->Widget && this->Widget->IsCreated())
    {
    this->Widget->SetWidth(this->Widget->GetWidth() + 20);
    }
}

//----------------------------------------------------------------------------
void vtkKWTreeWithScrollbars::ShrinkTreeHorizontallyCallback()
{
  if (this->Widget && this->Widget->IsCreated())
    {
    int width = this->Widget->GetWidth();
    if (width > 20)
      {
      this->Widget->SetWidth(width - 20);
      }
    else if (width > 1)
      {
      this->Widget->SetWidth(width >> 1);
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWTreeWithScrollbars::SetResizeButtonsVisibility(int v)
{
  if (this->ResizeButtonsVisibility == v)
    {
    return;
    }

  this->ResizeButtonsVisibility = v;
  this->Modified();

  this->UpdateButtonsVisibility();
}

//----------------------------------------------------------------------------
void vtkKWTreeWithScrollbars::UpdateButtonsVisibility()
{
  if (this->VerticalButtons)
    {
    this->VerticalButtons->SetWidgetVisibility(
      VTK_KWTWS_EXPAND_BUTTON_ID, this->ResizeButtonsVisibility);
    }

  if (this->HorizontalButtons)
    {
    this->HorizontalButtons->SetWidgetVisibility(
      VTK_KWTWS_EXPAND_BUTTON_ID, this->ResizeButtonsVisibility);
    }
}

//----------------------------------------------------------------------------
void vtkKWTreeWithScrollbars::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->PropagateEnableState(this->Widget);
  this->PropagateEnableState(this->VerticalButtons);
  this->PropagateEnableState(this->HorizontalButtons);
}

//----------------------------------------------------------------------------
void vtkKWTreeWithScrollbars::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Widget: ";
  if (this->Widget)
    {
    os << this->Widget << endl;
    }
  else
    {
    os << "(None)" << endl;
    }

  os << indent << "ResizeButtonsVisibility: " 
     << (this->ResizeButtonsVisibility ? "On" : "Off") << endl;
}
