/*=========================================================================

  Module:    $RCSfile: vtkKWFrameLabeled.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWFrameLabeled.h"

#include "vtkKWApplication.h"
#include "vtkKWDragAndDropTargetSet.h"
#include "vtkKWFrame.h"
#include "vtkKWIcon.h"
#include "vtkKWLabel.h"
#include "vtkKWLabelLabeled.h"
#include "vtkKWTkUtilities.h"
#include "vtkObjectFactory.h"

#include <vtksys/SystemTools.hxx>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWFrameLabeled );
vtkCxxRevisionMacro(vtkKWFrameLabeled, "$Revision: 1.13 $");

int vtkKWFrameLabeled::LabelCase = vtkKWFrameLabeled::LabelCaseUppercaseFirst;
int vtkKWFrameLabeled::BoldLabel = 1;
int vtkKWFrameLabeled::AllowShowHide = 1;

//----------------------------------------------------------------------------
vtkKWFrameLabeled::vtkKWFrameLabeled()
{
  this->Border     = vtkKWFrame::New();
  this->Groove     = vtkKWFrame::New();
  this->Border2    = vtkKWFrame::New();
  this->Frame      = vtkKWFrame::New();
  this->LabelFrame = vtkKWFrame::New();
  this->Label      = vtkKWLabelLabeled::New();
  this->Icon       = vtkKWLabel::New();
  this->IconData   = vtkKWIcon::New();

  this->Displayed     = 1;
  this->ShowHideFrame = 0;
  this->ShowIconInLimitedEditionMode = 0;
}

//----------------------------------------------------------------------------
vtkKWFrameLabeled::~vtkKWFrameLabeled()
{
  this->Icon->Delete();
  this->IconData->Delete();
  this->Label->Delete();
  this->Frame->Delete();
  this->LabelFrame->Delete();
  this->Border->Delete();
  this->Border2->Delete();
  this->Groove->Delete();
}

//----------------------------------------------------------------------------
vtkKWLabel* vtkKWFrameLabeled::GetLabel()
{
  if (this->Label)
    {
    return this->Label->GetWidget();
    }
  return NULL;
}

//----------------------------------------------------------------------------
vtkKWLabel* vtkKWFrameLabeled::GetLabelIcon()
{
  if (this->Label)
    {
    return this->Label->GetLabel();
    }
  return NULL;
}

//----------------------------------------------------------------------------
void vtkKWFrameLabeled::SetLabelText(const char *text)
{
  if (!text)
    {
    return;
    }

  if (vtkKWFrameLabeled::LabelCase == 
      vtkKWFrameLabeled::LabelCaseUserSpecified)
    {
    this->GetLabel()->SetText(text);
    }
  else
    {
    vtksys_stl::string res;
    switch (vtkKWFrameLabeled::LabelCase)
      {
      case vtkKWFrameLabeled::LabelCaseUppercaseFirst:
        res = vtksys::SystemTools::CapitalizedWords(text);
        break;
      case vtkKWFrameLabeled::LabelCaseLowercaseFirst:
        res = vtksys::SystemTools::UnCapitalizedWords(text);
        break;
      }
    this->GetLabel()->SetText(res.c_str());
    }
}

//----------------------------------------------------------------------------
void vtkKWFrameLabeled::AdjustMargin()
{
  if (this->IsCreated())
    {
    // Get the height of the label frame, and share it between
    // the two borders (frame).

    int height = atoi(this->Script("winfo reqheight %s", 
                                   this->LabelFrame->GetWidgetName()));

    // If the frame has not been packed yet, reqheight will return 1,
    // so try the hard way by checking what's inside the pack, provided
    // that it's simple (i.e. packed in a single row or column)

    if (height <= 1) 
      {
      int width;
      vtkKWTkUtilities::GetSlavesBoundingBoxInPack(
        this->LabelFrame, &width, &height);
      }

    // Don't forget the show/hide collapse icon, it might be bigger than
    // the LabelFrame contents (really ?)

    if (vtkKWFrameLabeled::AllowShowHide && this->ShowHideFrame &&
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

    this->Script("%s configure -height %d", 
                 this->Border->GetWidgetName(), border_h);
    this->Script("%s configure -height %d", 
                 this->Border2->GetWidgetName(), border2_h);

    if ( vtkKWFrameLabeled::AllowShowHide && this->ShowHideFrame )
      {
      this->Script("place %s -relx 1 -x %d -rely 0 -y %d -anchor center",
                   this->Icon->GetWidgetName(),
                   -this->IconData->GetWidth() -1,
                   border_h + 1);    
      this->Script("raise %s", this->Icon->GetWidgetName());
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWFrameLabeled::Create(vtkKWApplication *app)
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::Create(app);

  this->Border->SetParent(this);
  this->Border->Create(app);

  this->Groove->SetParent(this);
  this->Groove->Create(app);
  this->Groove->SetReliefToGroove();
  this->Groove->SetBorderWidth(2);

  this->Border2->SetParent(this->Groove);
  this->Border2->Create(app);

  this->Frame->SetParent(this->Groove);
  this->Frame->Create(app);

  this->LabelFrame->SetParent(this);
  this->LabelFrame->Create(app);

  this->Label->SetParent(this->LabelFrame);
  this->Label->Create(app);
  this->Label->SetBorderWidth(0);
  this->Label->ExpandWidgetOff();

  this->Script("%s config -bd 1 -pady 0 -padx 0", 
               this->GetLabel()->GetWidgetName());

  // At this point, although this->Label (a labeled label) has been created,
  // UpdateEnableState() has been called already and ShowLabelOff() has been
  // called on the label. Therefore, the label of this->Label was not created
  // since it is lazy created/allocated on the fly only when needed.
  // Force label icon to be created now, so that we can set its image option.

  this->Label->ShowLabelOn();
  this->GetLabelIcon()->SetImageOption(vtkKWIcon::IconLock);
  this->Script("%s config -bd 0 -pady 0 -padx 0", 
               this->GetLabelIcon()->GetWidgetName());

  const char *lem_name = app->GetLimitedEditionModeName() 
    ? app->GetLimitedEditionModeName() : "Limited Edition";
  
  ostrstream balloon_str;
  balloon_str << "This feature is not available in \"" << lem_name 
              << "\" mode." << ends;
  this->GetLabelIcon()->SetBalloonHelpString(balloon_str.str());
  balloon_str.rdbuf()->freeze(0);

  if (vtkKWFrameLabeled::BoldLabel)
    {
    vtkKWTkUtilities::ChangeFontWeightToBold(this->GetLabel());
    }

  this->IconData->SetImage(vtkKWIcon::IconShrink);

  this->Icon->SetParent(this);
  this->Icon->Create(app);
  this->Icon->SetImageOption(this->IconData);
  this->Icon->SetBalloonHelpString("Shrink or expand the frame");
  
  this->Script("pack %s -fill x -side top", this->Border->GetWidgetName());
  this->Script("pack %s -fill x -side top", this->Groove->GetWidgetName());
  this->Script("pack %s -fill x -side top", this->Border2->GetWidgetName());
  this->Script("pack %s -padx 2 -pady 2 -fill both -expand yes",
               this->Frame->GetWidgetName());
  this->Script(
    "pack %s -anchor nw -side left -fill both -expand y -padx 2 -pady 0",
    this->Label->GetWidgetName());
  this->Script("place %s -relx 0 -x 5 -y 0 -anchor nw",
               this->LabelFrame->GetWidgetName());

  this->Script("raise %s", this->Label->GetWidgetName());

  if ( vtkKWFrameLabeled::AllowShowHide && this->ShowHideFrame )
    {
    this->Script("bind %s <ButtonRelease-1> { %s PerformShowHideFrame }",
                 this->Icon->GetWidgetName(),
                 this->GetTclName());
    }

  // If the label frame get resize, reconfigure the margins

  this->Script("bind %s <Configure> { catch {%s AdjustMargin} }",
               this->LabelFrame->GetWidgetName(), this->GetTclName());

  // Update enable state

  this->UpdateEnableState();
}

//----------------------------------------------------------------------------
void vtkKWFrameLabeled::PerformShowHideFrame()
{
  if ( this->Displayed )
    {
    this->Script("pack forget %s", this->Frame->GetWidgetName());
    this->Displayed = 0;
    this->IconData->SetImage(vtkKWIcon::IconExpand);
    }
  else
    {
    this->Script("pack %s -fill both -expand yes",
                 this->Frame->GetWidgetName());
    this->Displayed = 1;
    this->IconData->SetImage(vtkKWIcon::IconShrink);
   }
  this->Icon->SetImageOption(this->IconData);
}

//----------------------------------------------------------------------------
void vtkKWFrameLabeled::AllowShowHideOn() 
{ 
  vtkKWFrameLabeled::AllowShowHide = 1; 
}
void vtkKWFrameLabeled::AllowShowHideOff() 
{ 
  vtkKWFrameLabeled::AllowShowHide = 0; 
}

//----------------------------------------------------------------------------
void vtkKWFrameLabeled::BoldLabelOn() 
{ 
  vtkKWFrameLabeled::BoldLabel = 1; 
}
void vtkKWFrameLabeled::BoldLabelOff() 
{ 
  vtkKWFrameLabeled::BoldLabel = 0; 
}

//----------------------------------------------------------------------------

void vtkKWFrameLabeled::SetLabelCase(int v) 
{ 
  vtkKWFrameLabeled::LabelCase = v;
}

int vtkKWFrameLabeled::GetLabelCase() 
{ 
  return vtkKWFrameLabeled::LabelCase;
}

//----------------------------------------------------------------------------
void vtkKWFrameLabeled::SetShowIconInLimitedEditionMode(int arg)
{
  if (this->ShowIconInLimitedEditionMode == arg)
    {
    return;
    }

  this->ShowIconInLimitedEditionMode = arg;
  this->Modified();

  this->UpdateEnableState();
}

//----------------------------------------------------------------------------
void vtkKWFrameLabeled::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  // Disable only the label part of the labeled label
  // (we want the icon not to look disabled)

  this->PropagateEnableState(this->GetLabel());

  int limited = (this->GetApplication() && 
                 this->GetApplication()->GetLimitedEditionMode());
  
  if (limited && this->ShowIconInLimitedEditionMode && !this->GetEnabled())
    {
    this->Label->ShowLabelOn();
    }
  else
    {
    this->Label->ShowLabelOff();
    }
  this->PropagateEnableState(this->Frame);
  this->PropagateEnableState(this->LabelFrame);
  this->PropagateEnableState(this->Border);
  this->PropagateEnableState(this->Border2);
  this->PropagateEnableState(this->Groove);
  this->PropagateEnableState(this->Icon);
}

//----------------------------------------------------------------------------
vtkKWDragAndDropTargetSet* vtkKWFrameLabeled::GetDragAndDropTargetSet()
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
void vtkKWFrameLabeled::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "ShowHideFrame: " 
     << (this->ShowHideFrame ? "On" : "Off") << endl;
  os << indent << "Frame: " << this->Frame << endl;
  os << indent << "LabelFrame: " << this->LabelFrame << endl;
  os << indent << "Label: " << this->Label << endl;
  os << indent << "ShowIconInLimitedEditionMode: " 
     << (this->ShowIconInLimitedEditionMode ? "On" : "Off") << endl;
}
