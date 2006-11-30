/*=========================================================================

  Module:    $RCSfile: vtkKWFrameWithLabel.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWFrameWithLabel.h"

#include "vtkKWApplication.h"
#include "vtkKWDragAndDropTargetSet.h"
#include "vtkKWFrame.h"
#include "vtkKWInternationalization.h"
#include "vtkKWIcon.h"
#include "vtkKWLabel.h"
#include "vtkKWLabelWithLabel.h"
#include "vtkKWTkUtilities.h"
#include "vtkObjectFactory.h"

#include <vtksys/SystemTools.hxx>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWFrameWithLabel );
vtkCxxRevisionMacro(vtkKWFrameWithLabel, "$Revision: 1.11 $");

int vtkKWFrameWithLabel::DefaultLabelCase = vtkKWFrameWithLabel::LabelCaseUppercaseFirst;
int vtkKWFrameWithLabel::DefaultLabelFontWeight = vtkKWFrameWithLabel::LabelFontWeightBold;
int vtkKWFrameWithLabel::DefaultAllowFrameToCollapse = 1;

//----------------------------------------------------------------------------
vtkKWFrameWithLabel::vtkKWFrameWithLabel()
{
  this->ExternalMarginFrame  = vtkKWFrame::New();
  this->CollapsibleFrame     = vtkKWFrame::New();
  this->InternalMarginFrame  = vtkKWFrame::New();
  this->Frame                = vtkKWFrame::New();
  this->LabelFrame           = vtkKWFrame::New();
  this->Label                = vtkKWLabelWithLabel::New();
  this->Icon                 = vtkKWLabel::New();
  this->IconData             = vtkKWIcon::New();

  this->AllowFrameToCollapse = 1;
  this->LimitedEditionModeIconVisibility = 0;
}

//----------------------------------------------------------------------------
vtkKWFrameWithLabel::~vtkKWFrameWithLabel()
{
  if (this->Icon)
    {
    this->Icon->Delete();
    this->Icon = NULL;
    }
  if (this->IconData)
    {
    this->IconData->Delete();
    this->IconData = NULL;
    }
  if (this->Label)
    {
    this->Label->Delete();
    this->Label = NULL;
    }
  if (this->Frame)
    {
    this->Frame->Delete();
    this->Frame = NULL;
    }
  if (this->LabelFrame)
    {
    this->LabelFrame->Delete();
    this->LabelFrame = NULL;
    }
  if (this->ExternalMarginFrame)
    {
    this->ExternalMarginFrame->Delete();
    this->ExternalMarginFrame = NULL;
    }
  if (this->InternalMarginFrame)
    {
    this->InternalMarginFrame->Delete();
    this->InternalMarginFrame = NULL;
    }
  if (this->CollapsibleFrame)
    {
    this->CollapsibleFrame->Delete();
    this->CollapsibleFrame = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkKWFrameWithLabel::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  this->ExternalMarginFrame->SetParent(this);
  this->ExternalMarginFrame->Create();

  this->CollapsibleFrame->SetParent(this);
  this->CollapsibleFrame->Create();
  this->CollapsibleFrame->SetReliefToGroove();
  this->CollapsibleFrame->SetBorderWidth(2);

  this->InternalMarginFrame->SetParent(this->CollapsibleFrame);
  this->InternalMarginFrame->Create();

  this->Frame->SetParent(this->CollapsibleFrame);
  this->Frame->Create();

  this->LabelFrame->SetParent(this);
  this->LabelFrame->Create();

  this->Label->SetParent(this->LabelFrame);
  this->Label->Create();
  this->Label->SetBorderWidth(0);
  this->Label->ExpandWidgetOff();

  vtkKWLabel *label = this->GetLabel();
  label->SetBorderWidth(1);
  label->SetPadX(0);
  label->SetPadY(0);

  // At this point, although this->Label (a labeled label) has been created,
  // UpdateEnableState() has been called already and ShowLabelOff() has been
  // called on the label. Therefore, the label of this->Label was not created
  // since it is lazy created/allocated on the fly only when needed.
  // Force label icon to be created now, so that we can set its image option.

  this->Label->LabelVisibilityOn();
  this->GetLabel()->SetBinding("<Double-1>", this, "LabelDoubleClickCallback");

  label = this->GetLabelIcon();
  label->SetImageToPredefinedIcon(vtkKWIcon::IconLock);
  label->SetBorderWidth(0);
  label->SetPadX(0);
  label->SetPadY(0);

  const char *lem_name = this->GetApplication()->GetLimitedEditionModeName() 
    ? this->GetApplication()->GetLimitedEditionModeName() : "Limited Edition";
  
  char buffer[500];
  sprintf(buffer, k_("This feature is not available in '%s'mode."), lem_name);
  this->GetLabelIcon()->SetBalloonHelpString(buffer);

  if (vtkKWFrameWithLabel::DefaultLabelFontWeight ==
      vtkKWFrameWithLabel::LabelFontWeightBold)
    {
    vtkKWTkUtilities::ChangeFontWeightToBold(this->GetLabel());
    }

  this->IconData->SetImage(vtkKWIcon::IconShrink);

  this->Icon->SetParent(this);
  this->Icon->Create();
  this->Icon->SetImageToIcon(this->IconData);
  this->Icon->SetBalloonHelpString(
    ks_("Frame With Label|Shrink or expand the frame"));
  
  this->Script(
    "pack %s -fill x -expand n -anchor nw -side top", this->ExternalMarginFrame->GetWidgetName());
  this->Script(
    "pack %s -fill both -expand y -anchor nw -side top", this->CollapsibleFrame->GetWidgetName());
  this->Script(
    "pack %s -fill x -expand n -side top", this->InternalMarginFrame->GetWidgetName());

  this->Script(
    "pack %s -padx 2 -pady 2 -fill both -expand yes -side top",
               this->Frame->GetWidgetName());

  this->Script(
    "pack %s -anchor nw -side left -fill both -expand y -padx 2 -pady 0",
    this->Label->GetWidgetName());

  this->Script("place %s -relx 0 -x 5 -y 0 -anchor nw",
               this->LabelFrame->GetWidgetName());
  this->Label->Raise();

  this->Icon->SetBinding("<ButtonRelease-1>",this,"CollapseButtonCallback");

  // If the label frame get resize, reset the margins

  vtksys_stl::string callback("catch {");
  callback += this->GetTclName();
  callback += " AdjustMarginCallback}";
  this->LabelFrame->SetBinding("<Configure>", NULL, callback.c_str());
}

//----------------------------------------------------------------------------
void vtkKWFrameWithLabel::SetWidth(int width)
{
  this->Superclass::SetWidth(width);
  if (this->CollapsibleFrame)
    {
    this->CollapsibleFrame->SetWidth(width);
    width -= this->CollapsibleFrame->GetBorderWidth() * 2;
    if (this->InternalMarginFrame)
      {
      this->InternalMarginFrame->SetWidth(width);
      }
    if (this->Frame)
      {
      int padx = 2; // see Create()
      this->Frame->SetWidth(width - padx * 2);
      }
    }
}

//----------------------------------------------------------------------------
int vtkKWFrameWithLabel::GetWidth()
{
  int width = this->Superclass::GetWidth();
  if (this->CollapsibleFrame)
    {
    int internal_width = this->CollapsibleFrame->GetWidth();
    if (internal_width > width)
      {
      width = internal_width;
      }
    if (this->Frame)
      {
      int padx = 2; // see Create()
      internal_width = this->Frame->GetWidth() + 
        padx * 2 + this->CollapsibleFrame->GetBorderWidth() * 2;
      if (internal_width > width)
        {
        width = internal_width;
        }
      }
    }
  return width;
}

//----------------------------------------------------------------------------
void vtkKWFrameWithLabel::SetHeight(int height)
{
  this->Superclass::SetHeight(height);
  if (this->CollapsibleFrame)
    {
    if (this->ExternalMarginFrame)
      {
      height -= this->ExternalMarginFrame->GetHeight();
      }
    this->CollapsibleFrame->SetHeight(height);
    height -= this->CollapsibleFrame->GetBorderWidth() * 2;
    if (this->InternalMarginFrame)
      {
      height -= this->InternalMarginFrame->GetHeight();
      }
    if (this->Frame)
      {
      int pady = 2; // see Create()
      this->Frame->SetHeight(height - pady * 2);
      }
    }
}

//----------------------------------------------------------------------------
int vtkKWFrameWithLabel::GetHeight()
{
  int height = this->Superclass::GetHeight();
  if (this->CollapsibleFrame)
    {
    int internal_height = this->CollapsibleFrame->GetHeight();
    if (this->ExternalMarginFrame)
      {
      internal_height += this->ExternalMarginFrame->GetHeight();
      }
    if (internal_height > height)
      {
      height = internal_height;
      }
    if (this->Frame)
      {
      int pady = 2; // see Create()
      internal_height = this->Frame->GetHeight() + 
        pady * 2 + this->CollapsibleFrame->GetBorderWidth() * 2;
      if (this->ExternalMarginFrame)
        {
        internal_height += this->ExternalMarginFrame->GetHeight();
        }
      if (this->InternalMarginFrame)
        {
        internal_height += this->InternalMarginFrame->GetHeight();
        }
      if (internal_height > height)
        {
        height = internal_height;
        }
      }
    }
  return height;
}

//----------------------------------------------------------------------------
vtkKWLabel* vtkKWFrameWithLabel::GetLabel()
{
  if (this->Label)
    {
    return this->Label->GetWidget();
    }
  return NULL;
}

//----------------------------------------------------------------------------
vtkKWLabel* vtkKWFrameWithLabel::GetLabelIcon()
{
  if (this->Label)
    {
    return this->Label->GetLabel();
    }
  return NULL;
}

//----------------------------------------------------------------------------
void vtkKWFrameWithLabel::SetLabelText(const char *text)
{
  if (!text)
    {
    return;
    }

  if (vtkKWFrameWithLabel::DefaultLabelCase == 
      vtkKWFrameWithLabel::LabelCaseUserSpecified)
    {
    this->GetLabel()->SetText(text);
    }
  else
    {
    vtksys_stl::string res;
    switch (vtkKWFrameWithLabel::DefaultLabelCase)
      {
      case vtkKWFrameWithLabel::LabelCaseUppercaseFirst:
        res = vtksys::SystemTools::CapitalizedWords(text);
        break;
      case vtkKWFrameWithLabel::LabelCaseLowercaseFirst:
        res = vtksys::SystemTools::UnCapitalizedWords(text);
        break;
      }
    this->GetLabel()->SetText(res.c_str());
    }
}

//----------------------------------------------------------------------------
void vtkKWFrameWithLabel::AdjustMarginCallback()
{
  if (this->IsCreated())
    {
    // Get the height of the label frame, and share it between
    // the two borders (frame).

    int height = 0, width = 0;
    vtkKWTkUtilities::GetWidgetRequestedSize(
      this->LabelFrame, &width, &height);

    // If the frame has not been packed yet, reqheight will return 1,
    // so try the hard way by checking what's inside the pack, provided
    // that it's simple (i.e. packed in a single row or column)

    if (height <= 1) 
      {
      vtkKWTkUtilities::GetSlavesBoundingBoxInPack(
        this->LabelFrame, &width, &height);
      }

    // Don't forget the show/hide collapse icon, it might be bigger than
    // the LabelFrame contents (really ?)

    if (vtkKWFrameWithLabel::DefaultAllowFrameToCollapse && 
        this->AllowFrameToCollapse &&
        height < this->IconData->GetHeight())
      {
      height = this->IconData->GetHeight();
      }

    int border_h = height / 2;
    int border2_h = height / 2;
#ifdef _WIN32
    border_h++;
#else
    border2_h++;
#endif

    this->ExternalMarginFrame->SetHeight(border_h);
    this->InternalMarginFrame->SetHeight(border2_h);

    if (vtkKWFrameWithLabel::DefaultAllowFrameToCollapse && 
        this->AllowFrameToCollapse)
      {
      this->Script("place %s -relx 1 -x %d -rely 0 -y %d -anchor center",
                   this->Icon->GetWidgetName(),
                   -this->IconData->GetWidth() -1,
                   border_h + 1);    
      this->Icon->Raise();
      }

    // Now check if we need to expand the widget to show the label
    // This is required if the frame is empty for example, since
    // the Label is 'placed', it does not claim its own space

    int min_width = width + this->IconData->GetWidth() + 40;
    if (this->GetWidth() < min_width)
      {
      vtkKWTkUtilities::GetWidgetRequestedSize(this, &width, NULL);
      if (width < min_width)
        {
        this->SetWidth(min_width);
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWFrameWithLabel::ExpandFrame()
{
  if (this->Frame && this->Frame->IsCreated())
    {
    this->Script("pack %s -fill both -expand yes -padx 2 -pady 2",
                 this->Frame->GetWidgetName());
    }
  if (this->IconData && this->Icon)
    {
    this->IconData->SetImage(vtkKWIcon::IconShrink);
    this->Icon->SetImageToIcon(this->IconData);
    }
}

//----------------------------------------------------------------------------
void vtkKWFrameWithLabel::CollapseFrame()
{
  if (this->Frame && this->Frame->IsCreated())
    {
    this->Script("pack forget %s", this->Frame->GetWidgetName());
    this->SetWidth(this->GetWidth());
    this->SetHeight(this->GetHeight());
    }
  if (this->IconData && this->Icon)
    {
    this->IconData->SetImage(vtkKWIcon::IconExpand);
    this->Icon->SetImageToIcon(this->IconData);
    }
}

//----------------------------------------------------------------------------
int vtkKWFrameWithLabel::IsFrameCollapsed()
{
  return (this->Frame && this->Frame->IsCreated() && !this->Frame->IsPacked());
}

//----------------------------------------------------------------------------
void vtkKWFrameWithLabel::LabelDoubleClickCallback()
{
  this->CollapseButtonCallback();
}

//----------------------------------------------------------------------------
void vtkKWFrameWithLabel::CollapseButtonCallback()
{
  if (vtkKWFrameWithLabel::DefaultAllowFrameToCollapse && 
      this->AllowFrameToCollapse)
    {
    if (this->IsFrameCollapsed())
      {
      this->ExpandFrame();
      }
    else
      {
      this->CollapseFrame();
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWFrameWithLabel::SetDefaultAllowFrameToCollapse(int arg) 
{ 
  vtkKWFrameWithLabel::DefaultAllowFrameToCollapse = arg; 
}

//----------------------------------------------------------------------------
int vtkKWFrameWithLabel::GetDefaultAllowFrameToCollapse() 
{ 
  return vtkKWFrameWithLabel::DefaultAllowFrameToCollapse; 
}

//----------------------------------------------------------------------------
void vtkKWFrameWithLabel::SetDefaultLabelFontWeight(int arg) 
{ 
  vtkKWFrameWithLabel::DefaultLabelFontWeight = arg; 
}

//----------------------------------------------------------------------------
int vtkKWFrameWithLabel::GetDefaultLabelFontWeight() 
{ 
  return vtkKWFrameWithLabel::DefaultLabelFontWeight; 
}

//----------------------------------------------------------------------------
void vtkKWFrameWithLabel::SetDefaultLabelCase(int v) 
{ 
  vtkKWFrameWithLabel::DefaultLabelCase = v;
}

//----------------------------------------------------------------------------
int vtkKWFrameWithLabel::GetDefaultLabelCase() 
{ 
  return vtkKWFrameWithLabel::DefaultLabelCase;
}

//----------------------------------------------------------------------------
void vtkKWFrameWithLabel::SetLimitedEditionModeIconVisibility(int arg)
{
  if (this->LimitedEditionModeIconVisibility == arg)
    {
    return;
    }

  this->LimitedEditionModeIconVisibility = arg;
  this->Modified();

  this->UpdateEnableState();
}

//----------------------------------------------------------------------------
void vtkKWFrameWithLabel::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  // Disable only the label part of the labeled label
  // (we want the icon not to look disabled)

  this->PropagateEnableState(this->GetLabel());

  int limited = (this->GetApplication() && 
                 this->GetApplication()->GetLimitedEditionMode());
  
  if (limited && this->LimitedEditionModeIconVisibility && !this->GetEnabled())
    {
    this->Label->LabelVisibilityOn();
    }
  else
    {
    this->Label->LabelVisibilityOff();
    }
  this->PropagateEnableState(this->Frame);
  this->PropagateEnableState(this->LabelFrame);
  this->PropagateEnableState(this->ExternalMarginFrame);
  this->PropagateEnableState(this->InternalMarginFrame);
  this->PropagateEnableState(this->CollapsibleFrame);
  this->PropagateEnableState(this->Icon);
}

//----------------------------------------------------------------------------
vtkKWDragAndDropTargetSet* vtkKWFrameWithLabel::GetDragAndDropTargetSet()
{
  int exist = this->HasDragAndDropTargetSet();
  vtkKWDragAndDropTargetSet *targets = this->Superclass::GetDragAndDropTargetSet();
  if (!exist)
    {
    targets->SetSourceAnchor(this->GetLabel());
    }
  return targets;
}

//----------------------------------------------------------------------------
void vtkKWFrameWithLabel::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "AllowFrameToCollapse: " 
     << (this->AllowFrameToCollapse ? "On" : "Off") << endl;
  os << indent << "Frame: " << this->Frame << endl;
  os << indent << "LabelFrame: " << this->LabelFrame << endl;
  os << indent << "Groove: " << this->CollapsibleFrame << endl;
  os << indent << "Label: " << this->Label << endl;
  os << indent << "LimitedEditionModeIconVisibility: " 
     << (this->LimitedEditionModeIconVisibility ? "On" : "Off") << endl;
}
