/*=========================================================================

  Module:    $RCSfile: vtkKWSelectionFrame.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWSelectionFrame.h"

#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
#include "vtkMath.h"

#include "vtkKWFrame.h"
#include "vtkKWLabel.h"
#include "vtkKWInternationalization.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWPushButton.h"
#include "vtkKWIcon.h"
#include "vtkKWToolbarSet.h"
#include "vtkKWTkUtilities.h"

#include <vtksys/ios/sstream>
#include <vtksys/stl/list>
#include <vtksys/stl/string>

vtkStandardNewMacro(vtkKWSelectionFrame);
vtkCxxRevisionMacro(vtkKWSelectionFrame, "$Revision: 1.62 $");

//----------------------------------------------------------------------------
class vtkKWSelectionFrameInternals
{
public:
  typedef vtksys_stl::list<vtksys_stl::string> StringPoolType;
  typedef vtksys_stl::list<vtksys_stl::string>::iterator StringPoolIterator;

  StringPoolType StringPool;

  int OuterSelectionFrameBlinkingCounter;
  vtksys_stl::string OuterSelectionFrameBlinkingTimerId;
};

//----------------------------------------------------------------------------
vtkKWSelectionFrame::vtkKWSelectionFrame()
{
  this->Internals             = new vtkKWSelectionFrameInternals;
  this->Internals->OuterSelectionFrameBlinkingCounter = 0;

  this->OuterSelectionFrame   = vtkKWFrame::New();
  this->TitleBarFrame         = vtkKWFrame::New();
  this->TitleLabel                 = vtkKWLabel::New();
  this->TitleLabel->SetText(ks_("Selection Frame|<Click to Select>"));
  this->SelectionListMenuButton         = vtkKWMenuButton::New();
  this->CloseButton           = vtkKWPushButton::New();
  this->BodyFrame             = vtkKWFrame::New();
  this->ToolbarSet            = NULL;
  this->LeftUserFrame         = NULL;
  this->RightUserFrame        = NULL;
  this->TitleBarUserFrame     = NULL;

  this->CloseCommand          = NULL;
  this->SelectionListCommand  = NULL;
  this->SelectCommand         = NULL;
  this->DoubleClickCommand    = NULL;
  this->ChangeTitleCommand    = NULL;
  this->TitleChangedCommand    = NULL;

  this->TitleColor[0]                   = 1.0;
  this->TitleColor[1]                   = 1.0;
  this->TitleColor[2]                   = 1.0;

  this->TitleSelectedColor[0]           = 1.0;
  this->TitleSelectedColor[1]           = 1.0;
  this->TitleSelectedColor[2]           = 1.0;

  this->TitleBackgroundColor[0]         = 0.6;
  this->TitleBackgroundColor[1]         = 0.6;
  this->TitleBackgroundColor[2]         = 0.6;

  this->TitleSelectedBackgroundColor[0] = 0.0;
  this->TitleSelectedBackgroundColor[1] = 0.0;
  this->TitleSelectedBackgroundColor[2] = 0.5;

  this->OuterSelectionFrameColor[0]     = 0.6;
  this->OuterSelectionFrameColor[1]     = 0.6;
  this->OuterSelectionFrameColor[2]     = 0.6;

  this->OuterSelectionFrameSelectedColor[0] = 1.0;
  this->OuterSelectionFrameSelectedColor[1] = 0.93;
  this->OuterSelectionFrameSelectedColor[2] = 0.79;

  this->Selected                    = 0;
  this->TitleBarVisibility          = 1;
  this->SelectionListVisibility     = 1;
  this->AllowClose                  = 1;
  this->AllowChangeTitle            = 1;
  this->ToolbarSetVisibility        = 0;
  this->LeftUserFrameVisibility     = 0;
  this->RightUserFrameVisibility    = 0;
  this->OuterSelectionFrameWidth    = 0;
  this->OuterSelectionFrameBlinking = 0;
}

//----------------------------------------------------------------------------
vtkKWSelectionFrame::~vtkKWSelectionFrame()
{
  this->Close();

  // Delete our pool

  delete this->Internals;

  if (this->OuterSelectionFrame)
    {
    this->OuterSelectionFrame->Delete();
    this->OuterSelectionFrame = NULL;
    }

  if (this->TitleBarFrame)
    {
    this->TitleBarFrame->Delete();
    this->TitleBarFrame = NULL;
    }

  if (this->TitleLabel)
    {
    this->TitleLabel->Delete();
    this->TitleLabel = NULL;
    }

  if (this->SelectionListMenuButton)
    {
    this->SelectionListMenuButton->Delete();
    this->SelectionListMenuButton = NULL;
    }

  if (this->CloseButton)
    {
    this->CloseButton->Delete();
    this->CloseButton = NULL;
    }

  if (this->TitleBarUserFrame)
    {
    this->TitleBarUserFrame->Delete();
    this->TitleBarUserFrame = NULL;
    }

  if (this->ToolbarSet)
    {
    this->ToolbarSet->Delete();
    this->ToolbarSet = NULL;
    }

  if (this->LeftUserFrame)
    {
    this->LeftUserFrame->Delete();
    this->LeftUserFrame = NULL;
    }

  if (this->RightUserFrame)
    {
    this->RightUserFrame->Delete();
    this->RightUserFrame = NULL;
    }

  if (this->BodyFrame)
    {
    this->BodyFrame->Delete();
    this->BodyFrame = NULL;
    }

  if (this->CloseCommand)
    {
    delete [] this->CloseCommand;
    this->CloseCommand = NULL;
    }

  if (this->SelectionListCommand)
    {
    delete [] this->SelectionListCommand;
    this->SelectionListCommand = NULL;
    }

  if (this->SelectCommand)
    {
    delete [] this->SelectCommand;
    this->SelectCommand = NULL;
    }

  if (this->DoubleClickCommand)
    {
    delete [] this->DoubleClickCommand;
    this->DoubleClickCommand = NULL;
    }

  if (this->ChangeTitleCommand)
    {
    delete [] this->ChangeTitleCommand;
    this->ChangeTitleCommand = NULL;
    }

  if (this->TitleChangedCommand)
    {
    delete [] this->TitleChangedCommand;
    this->TitleChangedCommand = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  this->SetBorderWidth(1);
  this->SetReliefToRidge();

  // The outer selection frame

  this->OuterSelectionFrame->SetParent(this);
  this->OuterSelectionFrame->Create();
  this->OuterSelectionFrame->SetReliefToFlat();
  this->OuterSelectionFrame->SetBorderWidth(this->OuterSelectionFrameWidth);

  vtkKWWidget *parent = this->OuterSelectionFrame;

  // The title bar

  this->TitleBarFrame->SetParent(parent);
  this->TitleBarFrame->Create();

  // The selection button

  this->SelectionListMenuButton->SetParent(parent);
  this->SelectionListMenuButton->Create();
  this->SelectionListMenuButton->IndicatorVisibilityOff();
  this->SelectionListMenuButton->SetImageToPredefinedIcon(vtkKWIcon::IconExpand);

  // The close button

  this->CloseButton->SetParent(parent);
  this->CloseButton->Create();
  this->CloseButton->SetImageToPredefinedIcon(vtkKWIcon::IconShrink);
  this->CloseButton->SetCommand(this, "CloseCallback");
  this->CloseButton->SetBalloonHelpString(
    ks_("Selection Frame|Close window"));

  // The title itself

  this->TitleLabel->SetParent(this->TitleBarFrame);
  this->TitleLabel->Create();
  this->TitleLabel->SetJustificationToLeft();
  this->TitleLabel->SetAnchorToWest();
  
  // The body frame

  this->BodyFrame->SetParent(parent);
  this->BodyFrame->Create();
  this->BodyFrame->SetBackgroundColor(0.0, 0.0, 0.0);

  // Pack

  this->Pack();

  // Update aspect

  this->UpdateSelectedAspect();
}

//----------------------------------------------------------------------------
vtkKWToolbarSet* vtkKWSelectionFrame::GetToolbarSet()
{
  if (!this->ToolbarSet)
    {
    this->ToolbarSet = vtkKWToolbarSet::New();
    }

  if (!this->ToolbarSet->IsCreated() && this->IsCreated())
    {
    this->ToolbarSet->SetParent(this->OuterSelectionFrame);
    this->ToolbarSet->BottomSeparatorVisibilityOff();
    this->ToolbarSet->Create();
    this->Pack();
    this->UpdateEnableState();
    }

  return this->ToolbarSet;
}

//----------------------------------------------------------------------------
vtkKWFrame* vtkKWSelectionFrame::GetTitleBarUserFrame()
{
  if (!this->TitleBarUserFrame)
    {
    this->TitleBarUserFrame = vtkKWFrame::New();
    }

  if (!this->TitleBarUserFrame->IsCreated() && this->IsCreated())
    {
    this->TitleBarUserFrame->SetParent(this->TitleBarFrame);
    this->TitleBarUserFrame->Create();
    this->Pack();
    this->UpdateEnableState();
    }

  return this->TitleBarUserFrame;
}

//----------------------------------------------------------------------------
vtkKWFrame* vtkKWSelectionFrame::GetLeftUserFrame()
{
  if (!this->LeftUserFrame)
    {
    this->LeftUserFrame = vtkKWFrame::New();
    }

  if (!this->LeftUserFrame->IsCreated() && this->IsCreated())
    {
    this->LeftUserFrame->SetParent(this->OuterSelectionFrame);
    this->LeftUserFrame->Create();
    this->Pack();
    this->UpdateEnableState();
    }

  return this->LeftUserFrame;
}

//----------------------------------------------------------------------------
vtkKWFrame* vtkKWSelectionFrame::GetRightUserFrame()
{
  if (!this->RightUserFrame)
    {
    this->RightUserFrame = vtkKWFrame::New();
    }

  if (!this->RightUserFrame->IsCreated() && this->IsCreated())
    {
    this->RightUserFrame->SetParent(this->OuterSelectionFrame);
    this->RightUserFrame->Create();
    this->Pack();
    this->UpdateEnableState();
    }

  return this->RightUserFrame;
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::Pack()
{
  if (!this->IsAlive())
    {
    return;
    }

  this->UnpackChildren();

  vtksys_ios::ostringstream tk_cmd;

  int has_list     = this->SelectionListVisibility;
  int has_close    = this->AllowClose;
  int has_titlebar = this->TitleBarVisibility;
  int has_toolbar  = this->ToolbarSetVisibility;

  int need_left = this->LeftUserFrameVisibility || 
    (this->SelectionListVisibility && !TitleBarVisibility);

  int need_right = this->RightUserFrameVisibility || 
    (this->AllowClose && !TitleBarVisibility);

  vtkKWWidget *parent = this->OuterSelectionFrame;

  tk_cmd 
    << "pack " << this->OuterSelectionFrame->GetWidgetName()
    << " -expand y -fill both -padx 0 -pady 0 -ipadx 0 -ipady 0" << endl;

  if (has_titlebar && this->TitleBarFrame->IsCreated())
    {
    this->TitleBarFrame->UnpackChildren();
    tk_cmd 
      << "grid " << this->TitleBarFrame->GetWidgetName()
      << " -column " << (need_left && has_list ? 1 : 0)
      << " -columnspan " 
      << (1+(need_left && has_list ? 0 : 1)+(need_right && has_close ? 0 : 1))
      << " -row 0 -ipadx 1 -ipady 1 -sticky news" << endl;
    }

  if (this->SelectionListVisibility && 
      this->SelectionListMenuButton->IsCreated())
    {
    if (need_left)
      {
      tk_cmd << "grid " << this->SelectionListMenuButton->GetWidgetName()
             << " -column 0 -row 0 -sticky news -ipadx 1 -ipady 1"
             << " -in " << parent->GetWidgetName() << endl;
      }
    else
      {
      tk_cmd << "pack " << this->SelectionListMenuButton->GetWidgetName()
             << " -side left -anchor w -fill y -ipadx 1 -ipady 1"
             << " -in " << this->TitleBarFrame->GetWidgetName() << endl;
      }
    }

  if (this->TitleLabel->IsCreated())
    {
    tk_cmd << "pack " << this->TitleLabel->GetWidgetName()
           << " -side left -anchor w -fill x -expand y" << endl;
    }
  
  if (this->TitleBarUserFrame && this->TitleBarUserFrame->IsCreated())
    {
    tk_cmd << "pack " << this->TitleBarUserFrame->GetWidgetName()
           << " -side left -anchor e -padx 2 -fill x -expand n" << endl;
    }
  
  if (this->AllowClose && this->CloseButton->IsCreated())
    {
    if (need_right)
      {
      tk_cmd << "grid " << this->CloseButton->GetWidgetName()
             << " -column 2 -row 0 -sticky news -ipadx 1 -ipady 1"
             << " -in " << parent->GetWidgetName() << endl;
      }
    else
      {
      tk_cmd << "pack " << this->CloseButton->GetWidgetName()
             << " -side left -anchor e -fill y -ipadx 1 -ipady 1 "
             << " -in " << this->TitleBarFrame->GetWidgetName() << endl;
      }
    }

  if (has_toolbar && this->ToolbarSet && this->ToolbarSet->IsCreated())
    {
    tk_cmd 
      << "grid " << this->ToolbarSet->GetWidgetName()
      << " -column " << (need_left && has_list ? 1 : 0)
      << " -columnspan " 
      << (1+(need_left && has_list ? 0 : 1)+(need_right && has_close ? 0 : 1))
      << " -row " << (has_titlebar ? 1 : 0)
      << " -sticky news -padx 0 -pady 0" << endl;
    this->ToolbarSet->Pack();
    }

  for (int i = 0; i < 2; i++)
    {
    tk_cmd << "grid columnconfig " << parent->GetWidgetName() << " " << i 
           << " -weight 0" << endl;
    tk_cmd << "grid rowconfig " << parent->GetWidgetName() << " " << i 
           << " -weight 0" << endl;
    }

  if (this->LeftUserFrameVisibility && 
      this->LeftUserFrame && this->LeftUserFrame->IsCreated())
    {
    tk_cmd << "grid " << this->LeftUserFrame->GetWidgetName()
           << " -column 0 " << " -row " << (has_titlebar || has_list ? 1 : 0)
           << " -rowspan 3 -sticky news -padx 0 -pady 0" << endl;
    }

  if (this->BodyFrame->IsCreated())
    {
    int row = ((has_titlebar ? 1 : 0) + (has_toolbar ? 1 : 0));

    tk_cmd 
      << "grid " << this->BodyFrame->GetWidgetName()
      << " -column " << (need_left ? 1 : 0)
      << " -columnspan "  << (1 + (need_left ? 0 : 1) + (need_right ? 0 : 1))
      << " -row " << row << " -rowspan 3 -sticky news -padx 0 -pady 0" << endl;
    tk_cmd << "grid columnconfig " << parent->GetWidgetName() << " " 
           << (need_left ? 1 : 0) << " -weight 1" << endl;
    if (row == 0 && !has_titlebar && !has_toolbar && (has_list || has_close))
      {
      row++; // we do not want to expand the row that has the buttons
      }
    tk_cmd << "grid rowconfig " << parent->GetWidgetName() << " " 
           << row << " -weight 1" << endl;
    }

  if (this->RightUserFrameVisibility && 
      this->RightUserFrame && this->RightUserFrame->IsCreated())
    {
    tk_cmd << "grid " << this->RightUserFrame->GetWidgetName()
           << " -column 2 " << " -row " << (has_titlebar || has_close ? 1 : 0)
           << " -rowspan 3 -sticky news -padx 0 -pady 0" << endl;
    }

  this->Script(tk_cmd.str().c_str());
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::Bind()
{
  if (!this->IsAlive())
    {
    return;
    }

  this->AddCallbackCommandObservers();

  vtkKWWidget *widgets_b[] = 
    {
      this->OuterSelectionFrame,
      this->TitleBarFrame,
      this->TitleLabel,
      this->SelectionListMenuButton,
      this->CloseButton,
      this->BodyFrame,
      this->ToolbarSet,
      this->LeftUserFrame,
      this->RightUserFrame,
      this->TitleBarUserFrame
    };
  vtkKWWidget *widgets_db[] = 
    {
      this->OuterSelectionFrame,
      this->TitleBarFrame,
      this->TitleLabel
    };
      
  size_t i;
  for (i = 0; i < (sizeof(widgets_b) / sizeof(widgets_b[0])); i++)
    {
    if (widgets_b[i])
      {
      widgets_b[i]->SetBinding("<ButtonPress-1>", this, "SelectCallback");
      }
    }
  for (i = 0; i < (sizeof(widgets_db) / sizeof(widgets_db[0])); i++)
    {
    if (widgets_db[i])
      {
      widgets_db[i]->SetBinding("<Double-1>", this, "DoubleClickCallback");
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::UnBind()
{
  if (!this->IsAlive())
    {
    return;
    }

  this->RemoveCallbackCommandObservers();

  vtkKWWidget *widgets_b[] = 
    {
      this->OuterSelectionFrame,
      this->TitleBarFrame,
      this->TitleLabel,
      this->SelectionListMenuButton,
      this->CloseButton,
      this->BodyFrame,
      this->ToolbarSet,
      this->LeftUserFrame,
      this->RightUserFrame,
      this->TitleBarUserFrame
    };
  vtkKWWidget *widgets_db[] = 
    {
      this->OuterSelectionFrame,
      this->TitleBarFrame,
      this->TitleLabel
    };
      
  size_t i;
  for (i = 0; i < (sizeof(widgets_b) / sizeof(widgets_b[0])); i++)
    {
    if (widgets_b[i])
      {
      widgets_b[i]->RemoveBinding("<ButtonPress-1>");
      }
    }
  for (i = 0; i < (sizeof(widgets_db) / sizeof(widgets_db[0])); i++)
    {
    if (widgets_db[i])
      {
      widgets_db[i]->RemoveBinding("<Double-1>");
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::SetTitle(const char *title)
{
  if (this->TitleLabel)
    {
    vtksys_stl::string old_title(this->GetTitle());
    this->TitleLabel->SetText(title);
    if (strcmp(old_title.c_str(), this->GetTitle()))
      {
      this->InvokeTitleChangedCommand(this);
      }
    }
}

//----------------------------------------------------------------------------
const char* vtkKWSelectionFrame::GetTitle()
{
  if (this->TitleLabel)
    {
    return this->TitleLabel->GetText();
    }
  return NULL;
}

//----------------------------------------------------------------------------
int vtkKWSelectionFrame::SetColor(
  double *color, double r, double g, double b)
{
  if ((r == color[0] && g == color[1] &&  b == color[2]) ||
      (r < 0.0 || r > 1.0) || (g < 0.0 || g > 1.0) || (b < 0.0 || b > 1.0))
    {
    return 0;
    }

  color[0] = r;
  color[1] = g;
  color[2] = b;

  return 1;
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::SetTitleColor(
  double r, double g, double b)
{
  if (this->SetColor(this->TitleColor, r, g, b))
    {
    this->Modified();
    this->UpdateSelectedAspect();
    }
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::SetTitleSelectedColor(
  double r, double g, double b)
{
  if (this->SetColor(this->TitleSelectedColor, r, g, b))
    {
    this->Modified();
    this->UpdateSelectedAspect();
    }
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::SetTitleBackgroundColor(
  double r, double g, double b)
{
  if (this->SetColor(this->TitleBackgroundColor, r, g, b))
    {
    this->Modified();
    this->UpdateSelectedAspect();
    }
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::SetTitleSelectedBackgroundColor(
  double r, double g, double b)
{
  if (this->SetColor(this->TitleSelectedBackgroundColor, r, g, b))
    {
    this->Modified();
    this->UpdateSelectedAspect();
    }
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::SetOuterSelectionFrameColor(
  double r, double g, double b)
{
  if (this->SetColor(this->OuterSelectionFrameColor, r, g, b))
    {
    this->Modified();
    this->UpdateSelectedAspect();
    }
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::SetOuterSelectionFrameSelectedColor(
  double r, double g, double b)
{
  if (this->SetColor(this->OuterSelectionFrameSelectedColor, r, g, b))
    {
    this->Modified();
    this->UpdateSelectedAspect();
    }
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::SetOuterSelectionFrameWidth(int arg)
{
  if (this->OuterSelectionFrameWidth == arg)
    {
    return;
    }

  this->OuterSelectionFrameWidth = arg;

  this->Modified();
  this->UpdateSelectedAspect();
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::SetOuterSelectionFrameBlinking(int arg)
{
  if (this->OuterSelectionFrameBlinking == arg)
    {
    return;
    }

  this->OuterSelectionFrameBlinking = arg;

  if (this->OuterSelectionFrameBlinking)
    {
    this->CreateOuterSelectionFrameBlinkingTimer();
    }
  else
    {
    this->CancelOuterSelectionFrameBlinkingTimer();
    }

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::CreateOuterSelectionFrameBlinkingTimer()
{
  this->Internals->OuterSelectionFrameBlinkingTimerId = 
    vtkKWTkUtilities::CreateTimerHandler(
      this->GetApplication(), 80, 
      this, "OuterSelectionFrameBlinkingCallback");
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::CancelOuterSelectionFrameBlinkingTimer()
{
  if (this->Internals->OuterSelectionFrameBlinkingTimerId.size())
    {
    vtkKWTkUtilities::CancelTimerHandler(
      this->GetApplication(), 
      this->Internals->OuterSelectionFrameBlinkingTimerId.c_str());
    this->UpdateOuterSelectionFrameColor();
    }
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::OuterSelectionFrameBlinkingCallback()
{
  this->Internals->OuterSelectionFrameBlinkingCounter++;
  this->UpdateOuterSelectionFrameColor();
  this->CreateOuterSelectionFrameBlinkingTimer();
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::SetSelected(int arg)
{
  if (this->Selected == arg)
    {
    return;
    }

  this->Selected = arg;

  this->Modified();
  this->UpdateSelectedAspect();
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::SetSelectionListVisibility(int arg)
{
  if (this->SelectionListVisibility == arg)
    {
    return;
    }

  this->SelectionListVisibility = arg;

  this->Modified();
  this->Pack();
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::SetAllowClose(int arg)
{
  if (this->AllowClose == arg)
    {
    return;
    }

  this->AllowClose = arg;

  this->Modified();
  this->Pack();
  this->UpdateSelectionListMenuButton();
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::SetAllowChangeTitle(int arg)
{
  if (this->AllowChangeTitle == arg)
    {
    return;
    }

  this->AllowChangeTitle = arg;

  this->Modified();
  this->UpdateSelectionListMenuButton();
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::SetToolbarSetVisibility(int arg)
{
  if (this->ToolbarSetVisibility == arg)
    {
    return;
    }

  this->ToolbarSetVisibility = arg;

  this->Modified();
  this->Pack();
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::SetLeftUserFrameVisibility(int arg)
{
  if (this->LeftUserFrameVisibility == arg)
    {
    return;
    }

  this->LeftUserFrameVisibility = arg;

  this->Modified();
  this->Pack();
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::SetRightUserFrameVisibility(int arg)
{
  if (this->RightUserFrameVisibility == arg)
    {
    return;
    }

  this->RightUserFrameVisibility = arg;

  this->Modified();
  this->Pack();
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::SetTitleBarVisibility(int arg)
{
  if (this->TitleBarVisibility == arg)
    {
    return;
    }

  this->TitleBarVisibility = arg;

  this->Modified();
  this->Pack();
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::UpdateSelectedAspect()
{
  if (!this->IsCreated())
    {
    return;
    }

  double *title_fgcolor, *title_bgcolor;

  if (this->Selected)
    {
    title_fgcolor = this->TitleSelectedColor;
    title_bgcolor = this->TitleSelectedBackgroundColor;
    }
  else
    {
    title_fgcolor = this->TitleColor;
    title_bgcolor = this->TitleBackgroundColor;
    }

  this->TitleBarFrame->SetBackgroundColor(
    title_bgcolor[0], title_bgcolor[1], title_bgcolor[2]);

  this->TitleLabel->SetBackgroundColor(
    title_bgcolor[0], title_bgcolor[1], title_bgcolor[2]);

  this->TitleLabel->SetForegroundColor(
    title_fgcolor[0], title_fgcolor[1], title_fgcolor[2]);

  if (this->TitleBarUserFrame)
    {
    this->TitleBarUserFrame->SetBackgroundColor(
      title_bgcolor[0], title_bgcolor[1], title_bgcolor[2]);
    }

  if (this->OuterSelectionFrame)
    {
    this->OuterSelectionFrame->SetBorderWidth(this->OuterSelectionFrameWidth);
    }
  
  this->UpdateOuterSelectionFrameColor();
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::UpdateOuterSelectionFrameColor()
{
  if (!this->OuterSelectionFrame)
    {
    return;
    }

  double *selection_frame_bgcolor, blinking_rgb[3], blinking_hsv[3];

  if (this->Selected)
    {
    selection_frame_bgcolor = this->OuterSelectionFrameSelectedColor;
    }
  else
    {
    selection_frame_bgcolor = this->OuterSelectionFrameColor;
    }

  if (this->OuterSelectionFrameBlinking)
    {
    int counter = this->Internals->OuterSelectionFrameBlinkingCounter % 10 - 5;
    if (counter < 0)
      {
      counter = -counter;
      }
    vtkMath::RGBToHSV(selection_frame_bgcolor, blinking_hsv);
    blinking_hsv[2] = 0.2 * counter;
    vtkMath::HSVToRGB(blinking_hsv, blinking_rgb);
    selection_frame_bgcolor = blinking_rgb;
    }

  this->OuterSelectionFrame->SetBackgroundColor(selection_frame_bgcolor);
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::SetSelectionList(int num, const char **list)
{
  this->Internals->StringPool.clear();
  
  for (int i = 0; i < num; i++)
    {
    this->Internals->StringPool.push_back(list[i]);
    }

  this->UpdateSelectionListMenuButton();
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::SetSelectionList(vtkStringArray *list)
{
  this->Internals->StringPool.clear();

  if (list)
    {
    for (int i = 0; i < list->GetNumberOfValues(); i++)
      {
      this->Internals->StringPool.push_back(list->GetValue(i));
      }
    }

  this->UpdateSelectionListMenuButton();
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::UpdateSelectionListMenuButton()
{
  if (!this->SelectionListMenuButton->IsCreated())
    {
    return;
    }

  vtksys_stl::string callback;

  vtkKWMenu *menu = this->SelectionListMenuButton->GetMenu();
  menu->DeleteAllItems();
  
  vtkKWSelectionFrameInternals::StringPoolIterator it = 
    this->Internals->StringPool.begin();
  vtkKWSelectionFrameInternals::StringPoolIterator end = 
    this->Internals->StringPool.end();
  int nb_entries = 0;
  int insert_break = 0;
  for (; it != end; ++it)
    {
    ++nb_entries;
    if (!strcmp((*it).c_str(), "--"))
      {
      if (nb_entries > 22)
        {
        nb_entries = 0;
        insert_break = 1;
        }
      else
        {
        menu->AddSeparator();
        }
      }
    else
      {
      callback = "SelectionListCallback {";
      callback += *it;
      callback += "}";
      menu->AddRadioButton((*it).c_str(), this, callback.c_str());
      if (insert_break)
        {
        insert_break = 0;
        menu->SetItemColumnBreak(menu->GetNumberOfItems() - 1, 1);
        }
      }
    }

  // Add more commands

  if (this->AllowClose || this->AllowChangeTitle)
    {
    if (this->Internals->StringPool.size())
      {
      menu->AddSeparator();
      }
    int index;
    if (this->AllowChangeTitle)
      {
      index = menu->AddCommand(
        ks_("Selection Frame|Change Title"), this, "ChangeTitleCallback");
      menu->SetItemHelpString(
        index, ks_("Selection Frame|Change frame title"));
      }
    if (this->AllowClose)
      {
      index = menu->AddCommand(
        ks_("Selection Frame|Close"), this, "CloseCallback");
      menu->SetItemHelpString(index, ks_("Selection Frame|Close frame"));
      }
    }

  // The selection list is disabled when there are no entries

  this->UpdateEnableState();
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::SetSelectionListCommand(vtkObject *object,
                                                  const char *method)
{
  this->SetObjectMethodCommand(&this->SelectionListCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::InvokeSelectionListCommand(
  const char *item, vtkKWSelectionFrame *obj)
{
  if (this->SelectionListCommand && 
      *this->SelectionListCommand && 
      this->IsCreated())
    {
    this->Script("%s {%s} %s",
                 this->SelectionListCommand, item, obj->GetTclName());
    }
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::SetCloseCommand(vtkObject *object,
                                          const char *method)
{
  this->SetObjectMethodCommand(&this->CloseCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::InvokeCloseCommand(vtkKWSelectionFrame *obj)
{
  if (this->CloseCommand && *this->CloseCommand && this->IsCreated())
    {
    this->Script("%s %s",
                 this->CloseCommand, obj->GetTclName());
    }
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::SetSelectCommand(vtkObject *object,
                                           const char *method)
{
  this->SetObjectMethodCommand(&this->SelectCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::InvokeSelectCommand(vtkKWSelectionFrame *obj)
{
  if (this->SelectCommand && *this->SelectCommand && this->IsCreated())
    {
    this->Script("%s %s", this->SelectCommand, obj->GetTclName());
    }
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::SetDoubleClickCommand(vtkObject *object,
                                                const char *method)
{
  this->SetObjectMethodCommand(&this->DoubleClickCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::InvokeDoubleClickCommand(vtkKWSelectionFrame *obj)
{
  if (this->DoubleClickCommand && *this->DoubleClickCommand && 
      this->IsCreated())
    {
    this->Script("%s %s",
                 this->DoubleClickCommand, obj->GetTclName());
    }
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::SetChangeTitleCommand(vtkObject *object,
                                                const char *method)
{
  this->SetObjectMethodCommand(&this->ChangeTitleCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::InvokeChangeTitleCommand(vtkKWSelectionFrame *obj)
{
  if (this->ChangeTitleCommand && *this->ChangeTitleCommand && 
      this->IsCreated())
    {
    this->Script("%s %s",
                 this->ChangeTitleCommand, obj->GetTclName());
    }
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::SetTitleChangedCommand(vtkObject *object,
                                                const char *method)
{
  this->SetObjectMethodCommand(&this->TitleChangedCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::InvokeTitleChangedCommand(vtkKWSelectionFrame *obj)
{
  if (this->TitleChangedCommand && *this->TitleChangedCommand && 
      this->IsCreated())
    {
    this->Script("%s %s",
                 this->TitleChangedCommand, obj->GetTclName());
    }
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::SelectionListCallback(const char *menu_item)
{
  this->InvokeSelectionListCommand(menu_item, this);
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::CloseCallback()
{
  this->Close();
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::Close()
{
  this->UnBind();

  this->RemoveCallbackCommandObservers();

  this->InvokeCloseCommand(this);
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::SelectCallback()
{
  if (this->GetSelected())
    {
    return;
    }

  this->SelectedOn();

  this->InvokeSelectCommand(this);
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::DoubleClickCallback()
{
  this->SelectCallback();

  this->InvokeDoubleClickCommand(this);
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::ChangeTitleCallback()
{
  this->InvokeChangeTitleCommand(this);
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->PropagateEnableState(this->OuterSelectionFrame);
  this->PropagateEnableState(this->TitleBarFrame);
  this->PropagateEnableState(this->SelectionListMenuButton);
  this->PropagateEnableState(this->CloseButton);
  this->PropagateEnableState(this->TitleLabel);
  this->PropagateEnableState(this->TitleBarUserFrame);
  this->PropagateEnableState(this->ToolbarSet);
  this->PropagateEnableState(this->LeftUserFrame);
  this->PropagateEnableState(this->RightUserFrame);
  this->PropagateEnableState(this->BodyFrame);

  if (this->SelectionListMenuButton &&
      this->SelectionListMenuButton->GetMenu() &&
      !this->SelectionListMenuButton->GetMenu()->GetNumberOfItems())
    {
    this->SelectionListMenuButton->SetEnabled(0);
    }

  if (this->GetEnabled())
    {
    this->Bind();
    }
  else
    {
    this->UnBind();
    }
}

//----------------------------------------------------------------------------
void vtkKWSelectionFrame::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  
  os << indent << "BodyFrame: " << this->BodyFrame << endl;
  os << indent << "TitleBarUserFrame: " << this->TitleBarUserFrame
     << endl;
  os << indent << "SelectionListMenuButton: " << this->SelectionListMenuButton << endl;
  os << indent << "CloseButton: " << this->CloseButton << endl;
  os << indent << "ToolbarSet: " << this->ToolbarSet << endl;
  os << indent << "LeftUserFrame: " << this->LeftUserFrame << endl;
  os << indent << "RightUserFrame: " << this->RightUserFrame << endl;
  os << indent << "TitleColor: ("
     << this->TitleColor[0] << ", " 
     << this->TitleColor[1] << ", " 
     << this->TitleColor[2] << ")" << endl;
  os << indent << "TitleSelectedColor: ("
     << this->TitleSelectedColor[0] << ", " 
     << this->TitleSelectedColor[1] << ", " 
     << this->TitleSelectedColor[2] << ")" << endl;
  os << indent << "TitleBackgroundColor: ("
     << this->TitleBackgroundColor[0] << ", " 
     << this->TitleBackgroundColor[1] << ", " 
     << this->TitleBackgroundColor[2] << ")" << endl;
  os << indent << "TitleSelectedBackgroundColor: ("
     << this->TitleSelectedBackgroundColor[0] << ", " 
     << this->TitleSelectedBackgroundColor[1] << ", " 
     << this->TitleSelectedBackgroundColor[2] << ")" << endl;
  os << indent << "OuterSelectionFrameColor: ("
     << this->OuterSelectionFrameColor[0] << ", " 
     << this->OuterSelectionFrameColor[1] << ", " 
     << this->OuterSelectionFrameColor[2] << ")" << endl;
  os << indent << "Selected: " << (this->Selected ? "On" : "Off") << endl;
  os << indent << "SelectionListVisibility: " << (this->SelectionListVisibility ? "On" : "Off") << endl;
  os << indent << "AllowClose: " << (this->AllowClose ? "On" : "Off") << endl;
  os << indent << "AllowChangeTitle: " << (this->AllowChangeTitle ? "On" : "Off") << endl;
  os << indent << "ToolbarSetVisibility: " << (this->ToolbarSetVisibility ? "On" : "Off") << endl;
  os << indent << "LeftUserFrameVisibility: " << (this->LeftUserFrameVisibility ? "On" : "Off") << endl;
  os << indent << "RightUserFrameVisibility: " << (this->RightUserFrameVisibility ? "On" : "Off") << endl;
  os << indent << "TitleBarVisibility: " << (this->TitleBarVisibility ? "On" : "Off") << endl;
  os << indent << "OuterSelectionFrameWidth: " << this->OuterSelectionFrameWidth << endl;
  os << indent << "OuterSelectionFrameBlinking: " << (this->OuterSelectionFrameBlinking ? "On" : "Off") << endl;
}

