/*=========================================================================

Copyright (c) 1998-2003 Kitware Inc. 469 Clifton Corporate Parkway,
Clifton Park, NY, 12065, USA.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither the name of Kitware nor the names of any contributors may be used
   to endorse or promote products derived from this software without specific
   prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "vtkKWRange.h"

#include "vtkKWApplication.h"
#include "vtkKWEntry.h"
#include "vtkKWFrame.h"
#include "vtkKWIcon.h"
#include "vtkKWImageLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWPushButtonSet.h"
#include "vtkKWTkUtilities.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro( vtkKWRange );
vtkCxxRevisionMacro(vtkKWRange, "$Revision: 1.17 $");

#define VTK_KW_RANGE_MIN_SLIDER_SIZE        2
#define VTK_KW_RANGE_MIN_THICKNESS          (2*VTK_KW_RANGE_MIN_SLIDER_SIZE+1)
#define VTK_KW_RANGE_MIN_INTERNAL_THICKNESS 5
#define VTK_KW_RANGE_MIN_LENGTH             (2*VTK_KW_RANGE_MIN_THICKNESS)

#define VTK_KW_RANGE_NB_ENTRIES             2

#define VTK_KW_RANGE_WHOLE_RANGE_TAG        "whole_range"
#define VTK_KW_RANGE_RANGE_TAG              "range"
#define VTK_KW_RANGE_SLIDER1_TAG            "slider1"
#define VTK_KW_RANGE_SLIDER2_TAG            "slider2"
#define VTK_KW_RANGE_SLIDERS_TAG            "sliders"

// For some reasons, the end-point of a line/box is not drawn on Windows. 
// Comply with that.

#ifndef _WIN32
#define LSTRANGE 0
#else
#define LSTRANGE 1
#endif
#define RSTRANGE 1

//----------------------------------------------------------------------------
vtkKWRange::vtkKWRange()
{
  int i;

  this->WholeRange[0]       = 0;
  this->WholeRange[1]       = 1;  
  this->Range[0]            = this->WholeRange[0];
  this->Range[1]            = this->WholeRange[1];  
  this->Resolution          = (this->WholeRange[1]-this->WholeRange[0])/100.0;
  this->AdjustResolution    = 0;
  this->Thickness           = 19;
  this->InternalThickness   = 0.5;
  this->Orientation         = vtkKWRange::ORIENTATION_HORIZONTAL;
  this->Inverted            = 0;
  this->SliderSize          = 3;
  this->ShowEntries         = 0;
  this->LabelPosition       = vtkKWRange::POSITION_SIDE1;
  this->EntriesPosition     = vtkKWRange::POSITION_SIDE1;
  this->ZoomButtonsPosition = vtkKWRange::POSITION_SIDE1;
  this->EntriesResolution   = 2;
  this->EntriesWidth        = 10;
  this->SliderCanPush       = 0;
  this->DisableCommands     = 0;
  this->ShowZoomButtons     = 0;

  this->InInteraction            = 0;
  this->StartInteractionPos      = 0;
  this->StartInteractionRange[0] = this->Range[0];
  this->StartInteractionRange[1] = this->Range[1];

  this->RangeColor[0]      = -1; // will used a shade of -bg at runtime
  this->RangeColor[1]      = -1;
  this->RangeColor[2]      = -1;

  this->RangeInteractionColor[0] = 0.59;
  this->RangeInteractionColor[1] = 0.63;
  this->RangeInteractionColor[2] = 0.82;

  this->Command             = NULL;
  this->StartCommand        = NULL;
  this->EndCommand          = NULL;
  this->EntriesCommand      = NULL;

  this->CanvasFrame         = vtkKWFrame::New();
  this->Canvas              = vtkKWWidget::New();

  for (i = 0; i < VTK_KW_RANGE_NB_ENTRIES; i++)
    {
    this->Entries[i]        = NULL;
    }

  this->ZoomButtons         = NULL;

  this->ConstraintRanges();

  this->ConstraintResolution();
}

//----------------------------------------------------------------------------
vtkKWRange::~vtkKWRange()
{
  if (this->Command)
    {
    delete [] this->Command;
    this->Command = NULL;
    }

  if (this->StartCommand)
    {
    delete [] this->StartCommand;
    this->StartCommand = NULL;
    }

  if (this->EndCommand)
    {
    delete [] this->EndCommand;
    this->EndCommand = NULL;
    }

  if (this->EntriesCommand)
    {
    delete [] this->EntriesCommand;
    this->EntriesCommand = NULL;
    }

  if (this->CanvasFrame)
    {
    this->CanvasFrame->Delete();
    this->CanvasFrame = NULL;
    }

  if (this->Canvas)
    {
    this->Canvas->Delete();
    this->Canvas = NULL;
    }

  for (int i = 0; i < VTK_KW_RANGE_NB_ENTRIES; i++)
    {
    if (this->Entries[i])
      {
      this->Entries[i]->Delete();
      this->Entries[i] = NULL;
      }
    }

  if (this->ZoomButtons)
    {
    this->ZoomButtons->Delete();
    this->ZoomButtons = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkKWRange::Create(vtkKWApplication *app, const char *args)
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro("Range already created");
    return;
    }

  // Call the superclass, this will set the application,
  // create the frame and the Label

  this->Superclass::Create(app, args);

  // Now we need the canvas

  this->CanvasFrame->SetParent(this);
  this->CanvasFrame->Create(app, "");

  this->Canvas->SetParent(this->CanvasFrame);
  this->Canvas->Create(app, 
                       "canvas", 
                       "-bd 0 -highlightthickness 0 -width 0 -height 0");

  this->Script("bind %s <Configure> {%s ConfigureCallback}",
               this->CanvasFrame->GetWidgetName(), this->GetTclName());

  // Create more stuff

  if (this->ShowEntries)
    {
    this->CreateEntries();
    }

  if (this->ShowZoomButtons)
    {
    this->CreateZoomButtons();
    }

  // Pack the widget

  this->Pack();

  // Set the bindings

  this->Bind();

  // Update enable state

  this->UpdateEnableState();
}

//----------------------------------------------------------------------------
void vtkKWRange::CreateEntries()
{
  for (int i = 0; i < VTK_KW_RANGE_NB_ENTRIES; i++)
    {
    if (!this->Entries[i])
      {
      this->Entries[i] = vtkKWEntry::New();
      }

    if (!this->Entries[i]->IsCreated() && this->IsCreated())
      {
      this->Entries[i]->SetParent(this);
      this->Entries[i]->Create(this->Application, "");
      this->Entries[i]->SetWidth(this->EntriesWidth);
      this->Entries[i]->SetEnabled(this->Enabled);
      this->Script("bind %s <Return> {%s EntriesUpdateCallback %d}",
                   this->Entries[i]->GetWidgetName(), this->GetTclName(), i);
      this->Script("bind %s <FocusOut> {%s EntriesUpdateCallback %d}",
                   this->Entries[i]->GetWidgetName(), this->GetTclName(), i);
      }
    }

  this->UpdateEntriesResolution();
}

//----------------------------------------------------------------------------
void vtkKWRange::CreateZoomButtons()
{
  if (!this->ZoomButtons)
    {
    this->ZoomButtons = vtkKWPushButtonSet::New();
    }

  if (!this->ZoomButtons->IsCreated() && this->IsCreated())
    {
    this->ZoomButtons->SetParent(this);
    this->ZoomButtons->Create(this->Application, "");

    vtkKWPushButton *button;

    this->ZoomButtons->AddButton(
      0, "+", this, "EnlargeRangeCallback",
      "Enlarge the range");

    button = this->ZoomButtons->GetButton(0);
    if (button)
      {
      vtkKWIcon *icon = vtkKWIcon::New();
      icon->SetImageData(vtkKWIcon::ICON_PLUS);
      vtkKWTkUtilities::SetImageOption(this->Application->GetMainInterp(), 
                                       button->GetWidgetName(), icon, ".plus");
      icon->Delete();
      }

    this->ZoomButtons->AddButton(
      1, "-", this, "ShrinkRangeCallback",
      "Shrink the range");

    button = this->ZoomButtons->GetButton(1);
    if (button)
      {
      vtkKWIcon *icon = vtkKWIcon::New();
      icon->SetImageData(vtkKWIcon::ICON_MINUS);
      vtkKWTkUtilities::SetImageOption(this->Application->GetMainInterp(), 
                                       button->GetWidgetName(), icon,".minus");
      icon->Delete();
      }

    this->ZoomButtons->SetBorderWidth(0);
    this->ZoomButtons->SetPadding(0, 0);
    }
}

//----------------------------------------------------------------------------
void vtkKWRange::Pack()
{
  if (!this->IsCreated())
    {
    return;
    }

  // Unpack everything

  this->Label->UnpackSiblings();

  // Repack everything

  ostrstream tk_cmd;
  int is_horiz = (this->Orientation == vtkKWRange::ORIENTATION_HORIZONTAL);

  tk_cmd << "pack " << this->Canvas->GetWidgetName() 
         << " -fill both -expand y -pady 0 -padx 0 -ipady 0 -ipadx 0" << endl;

  /*
     ALIGNED: L [--------------]            E1 [--------------] E2

                       L                       E1            E2
     SIDE1:     [--------------]               [--------------]

     SIDE2:     [--------------]               [--------------]
                       L                       E1            E2

             0 1  2      3      4  5 6  7
         +-------------------------------
        0|        E0     L      Z E1 
        1|   L E0 [----------------] E1 Z
        2|        E0     L      Z E1
  */

  // We need a 6x3 grid

  char o_row[15], o_col[15], o_span[15], colconfig[20], stickydir[15];

  sprintf(o_row, " %s ", is_horiz ? "-row" : "-column");
  sprintf(o_col, " %s ", is_horiz ? "-column" : "-row");
  sprintf(o_span, " %s ", is_horiz ? "-columnspan" : "-rowspan");
  sprintf(colconfig, " %s ", is_horiz ? "columnconfigure" : "rowconfigure");
  sprintf(stickydir, " -sticky %s ", is_horiz ? "ew" : "ns");

  int row, col;

  // Label

  if (this->ShowLabel)
    {
    row = this->LabelPosition == vtkKWRange::POSITION_ALIGNED ? 1 : 
      (this->LabelPosition == vtkKWRange::POSITION_SIDE1 ? 0 : 2);
    col = this->LabelPosition == vtkKWRange::POSITION_ALIGNED ? 0 : 3;
    tk_cmd << "grid " << this->Label->GetWidgetName() 
           << o_row << row << o_col << col
           << (this->LabelPosition != vtkKWRange::POSITION_ALIGNED ? 
               stickydir : (is_horiz ? " -sticky w" : " -sticky n")) << endl;
    tk_cmd << this->Label->GetWidgetName() << " config -anchor "
           << (this->LabelPosition != vtkKWRange::POSITION_ALIGNED ? "c" : 
               (is_horiz ? "w" : "c")) << endl;
    }

  // Entries

  if (this->ShowEntries)
    {
    row = this->EntriesPosition == vtkKWRange::POSITION_ALIGNED ? 1 : 
      (this->EntriesPosition == vtkKWRange::POSITION_SIDE1 ? 0 : 2);
    int col0 = this->EntriesPosition == vtkKWRange::POSITION_ALIGNED ? 1 : 2;
    int col1 = this->EntriesPosition == vtkKWRange::POSITION_ALIGNED ? 6 : 5;
    tk_cmd << "grid " << this->Entries[this->Inverted ? 1 : 0]->GetWidgetName()
           << o_row << row << o_col << col0
           << " -sticky " << (is_horiz ? "w" : "n") << endl;
    tk_cmd << "grid " << this->Entries[this->Inverted ? 0 : 1]->GetWidgetName()
           << o_row << row << o_col << col1
           << " -sticky " << (is_horiz ? "e" : "s") << endl;
    }

  // Canvas

  tk_cmd << "grid " << this->CanvasFrame->GetWidgetName() 
         << o_row << 1 << o_col << 2 << stickydir << o_span << 4;
  if (this->ShowEntries)
    {
    if (this->EntriesPosition == vtkKWRange::POSITION_ALIGNED)
      {
      tk_cmd << (is_horiz ? " -padx " : " -pady ") << 2;
      }
    else
      {
      tk_cmd << (is_horiz ? " -pady " : " -padx ") << 2;
      }
    }
  tk_cmd << endl;

  // Zoom buttons

  if (this->ShowZoomButtons)
    {
    this->ZoomButtons->SetPackHorizontally(is_horiz ? 1 : 0);
    row = this->ZoomButtonsPosition == vtkKWRange::POSITION_ALIGNED ? 1 : 
      (this->ZoomButtonsPosition == vtkKWRange::POSITION_SIDE1 ? 0 : 2);
    col = this->ZoomButtonsPosition == vtkKWRange::POSITION_ALIGNED ? 7 : 4;
    tk_cmd << "grid " << this->ZoomButtons->GetWidgetName() 
           << o_row << row << o_col << col
           << " -sticky " << (is_horiz ? "e" : "s") << endl;
    }

  // Make sure it will resize properly

  for (int i = 2; i <= 5; i++)
    {
    tk_cmd << "grid " << colconfig
           << this->CanvasFrame->GetParent()->GetWidgetName() << " " << i
           << " -weight 1" << endl;
    }

  tk_cmd << ends;
  this->Script(tk_cmd.str());
  tk_cmd.rdbuf()->freeze(0);
}

//----------------------------------------------------------------------------
void vtkKWRange::Bind()
{
  if (!this->IsAlive())
    {
    return;
    }

  ostrstream tk_cmd;

  // Canvas

  if (this->Canvas->IsCreated())
    {
    const char *canv = this->Canvas->GetWidgetName();

    // Range

    tk_cmd << canv << " bind " <<  VTK_KW_RANGE_RANGE_TAG 
           << " <ButtonPress-1> {" << this->GetTclName() 
           << " StartInteractionCallback %%x %%y}" << endl;

    tk_cmd << canv << " bind " <<  VTK_KW_RANGE_RANGE_TAG 
           << " <ButtonRelease-1> {" << this->GetTclName() 
           << " EndInteractionCallback}" << endl;

    tk_cmd << canv << " bind " <<  VTK_KW_RANGE_RANGE_TAG
           << " <B1-Motion> {" << this->GetTclName() 
           << " RangeMotionCallback %%x %%y}" << endl;

    tk_cmd << canv << " bind " <<  VTK_KW_RANGE_RANGE_TAG 
           << " <Double-1> {" << this->GetTclName() 
           << " MaximizeRangeCallback}" << endl;

    // Sliders

    tk_cmd << canv << " bind " <<  VTK_KW_RANGE_SLIDERS_TAG 
           << " <ButtonPress-1> {" << this->GetTclName() 
           << " StartInteractionCallback %%x %%y}" << endl;

    tk_cmd << canv << " bind " <<  VTK_KW_RANGE_SLIDERS_TAG 
           << " <ButtonRelease-1> {" << this->GetTclName() 
           << " EndInteractionCallback}" << endl;

    tk_cmd << canv << " bind " <<  VTK_KW_RANGE_SLIDER1_TAG 
           << " <B1-Motion> {" << this->GetTclName() 
           << " SliderMotionCallback " 
           << vtkKWRange::SLIDER_INDEX_1 << " %%x %%y}" << endl;

    tk_cmd << canv << " bind " <<  VTK_KW_RANGE_SLIDER2_TAG 
           << " <B1-Motion> {" << this->GetTclName() 
           << " SliderMotionCallback " 
           << vtkKWRange::SLIDER_INDEX_2 << " %%x %%y}" << endl;
    }

  tk_cmd << ends;
  this->Script(tk_cmd.str());
  tk_cmd.rdbuf()->freeze(0);
}

//----------------------------------------------------------------------------
void vtkKWRange::UnBind()
{
  if (!this->IsAlive())
    {
    return;
    }

  ostrstream tk_cmd;

  // Canvas

  if (this->Canvas->IsCreated())
    {
    const char *canv = this->Canvas->GetWidgetName();

    tk_cmd << canv << " bind " <<  VTK_KW_RANGE_SLIDERS_TAG 
           << " <ButtonPress-1> {}" << endl;
    
    tk_cmd << canv << " bind " <<  VTK_KW_RANGE_SLIDERS_TAG 
           << " <ButtonRelease-1> {}" << endl;

    tk_cmd << canv << " bind " <<  VTK_KW_RANGE_SLIDER1_TAG 
           << " <B1-Motion> {}" << endl;

    tk_cmd << canv << " bind " <<  VTK_KW_RANGE_SLIDER2_TAG 
           << " <B1-Motion> {}" << endl;
    }
  
  tk_cmd << ends;
  this->Script(tk_cmd.str());
  tk_cmd.rdbuf()->freeze(0);
}

//----------------------------------------------------------------------------
void vtkKWRange::SetWholeRange(float r0, float r1)
{
  if (this->WholeRange[0] == r0 && this->WholeRange[1] == r1)
    {
    return;
    }

  this->WholeRange[0] = r0;
  this->WholeRange[1] = r1;

  this->Modified();

  this->ConstraintRanges();

  this->RedrawCanvas();
}

//----------------------------------------------------------------------------
void vtkKWRange::SetRange(float r0, float r1)
{
  if (this->Range[0] == r0 && this->Range[1] == r1)
    {
    return;
    }

  float old_range[2];
  old_range[0] = this->Range[0];
  old_range[1] = this->Range[1];

  this->Range[0] = r0;
  this->Range[1] = r1;

  this->Modified();

  this->ConstraintRange(this->Range, old_range);

  // Update the widget aspect

  if (this->IsCreated())
    {
    this->RedrawRange();

    int pos[2];
    this->GetSlidersPositions(pos);

    if (old_range[0] != this->Range[0])
      {
      this->RedrawSlider(pos[0], vtkKWRange::SLIDER_INDEX_1);
      }
    if (old_range[1] != this->Range[1])
      {
      this->RedrawSlider(pos[1], vtkKWRange::SLIDER_INDEX_2);
      }

    for (int i = 0; i < VTK_KW_RANGE_NB_ENTRIES; i++)
      {
      if (this->Entries[i] && this->Entries[i]->IsCreated() &&
          old_range[i] != this->Range[i])
        {
        this->Entries[i]->SetValue(this->Range[i], this->EntriesResolution);
        }
      }
    }

  // Invoke callback if needed

  if (old_range[0] != this->Range[0] || old_range[1] != this->Range[1])
    {
    this->InvokeCommand();
    }
}

//----------------------------------------------------------------------------
void vtkKWRange::GetRelativeRange(float &r0, float &r1)
{
  float whole_range = (this->WholeRange[1] - this->WholeRange[0]);
  r0 = (this->Range[0] - this->WholeRange[0]) / whole_range;
  r1 = (this->Range[1] - this->WholeRange[0]) / whole_range;
}

//----------------------------------------------------------------------------
void vtkKWRange::SetRelativeRange(float r0, float r1)
{
  float whole_range = (this->WholeRange[1] - this->WholeRange[0]);
  this->SetRange(r0 * whole_range + this->WholeRange[0],
                 r1 * whole_range + this->WholeRange[0]);
}

//----------------------------------------------------------------------------
void vtkKWRange::SetResolution(float arg)
{
  if (this->Resolution == arg || arg <= 0.0)
    {
    return;
    }

  float old_res = this->Resolution;
  this->Resolution = arg;
  this->ConstraintResolution();

  if (this->Resolution == old_res)
    {
    return;
    }

  this->Modified();

  this->ConstraintRanges();
  
  this->RedrawCanvas();

  this->UpdateEntriesResolution();
}

//----------------------------------------------------------------------------
void vtkKWRange::SetAdjustResolution(int arg)
{
  if (this->AdjustResolution == arg)
    {
    return;
    }

  this->AdjustResolution = arg;

  this->Modified();

  this->ConstraintResolution();
}

//----------------------------------------------------------------------------
void vtkKWRange::ConstraintResolution()
{
  if (this->AdjustResolution)
    {
    double reslog = log10((double)this->Resolution);
    double intp, fracp;
    fracp = modf(reslog, &intp);
    if (fabs(fracp) > 0.00001)
      {
      float newres = (float)pow(10.0, floor(reslog));
      if (newres != this->Resolution)
        {
        this->SetResolution(newres);
        }
      }
    }
}

// ---------------------------------------------------------------------------
void vtkKWRange::UpdateEntriesResolution()
{
  if (fabs(this->Resolution) >= 1.0)
    {
    this->EntriesResolution = 0;
    }
  else 
    {
    if (this->IsCreated())
      {
      // Trick here: use the 'expr' Tcl command to display the shortest
      // representation of the floating point number this->Resolution.
      // sprintf would be of no help here.

      const char *res = this->Script("expr %f", fabs(this->Resolution));
      char *pos = strchr(res, '.');
      if (pos)
        {
        this->EntriesResolution = (int)(strlen(res)) - (pos - res) - 1;
        }
      }
    }

  for (int i = 0; i < VTK_KW_RANGE_NB_ENTRIES; i++)
    {
    if (this->Entries[i] && this->Entries[i]->IsCreated())
      {
      this->Entries[i]->SetValue(this->Range[i], this->EntriesResolution);
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWRange::ConstraintValueToResolution(float &value)
{
  if (fmod(value, this->Resolution) != 0.0)
    {
    value = this->Resolution * vtkMath::Round(value / this->Resolution);
    }
}

//----------------------------------------------------------------------------
void vtkKWRange::ConstraintWholeRange()
{
  // Resolution OK for WholeRange ?

  for (int i = 0; i <= 1; i++)
    {
    this->ConstraintValueToResolution(this->WholeRange[i]);
    }
}

//----------------------------------------------------------------------------
void vtkKWRange::ConstraintRange(float range[2], float *range_hint)
{
  int i;

  int inv = (this->WholeRange[0] > this->WholeRange[1]);
  int wmin_idx = (inv ? 1 : 0);
  int wmax_idx = (wmin_idx == 0 ? 1 : 0);

  // Resolution OK for this ? Is it out of WholeRange ?

  for (i = 0; i <= 1; i++)
    {
    this->ConstraintValueToResolution(range[i]);

    if (range[i] < this->WholeRange[wmin_idx])
      {
      range[i] = this->WholeRange[wmin_idx];
      }
    else if (range[i] > this->WholeRange[wmax_idx])
      {
      range[i] = this->WholeRange[wmax_idx];
      }
    }

  // Range not in right order ?

  if (range[wmin_idx] > range[wmax_idx])
    {
    if (range_hint) // range_hint is used as an old range value
      {
      if (range[1] == range_hint[1]) // range[0] is moving
        {
        if (this->SliderCanPush)  
          {
          range[1] = range[0];
          }
        else
          {
          range[0] = range[1];
          }
        }
      else                                 // range[1] is moving
        {
        if (this->SliderCanPush)
          {
          range[0] = range[1];
          }
        else
          {
          range[1] = range[0];
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWRange::ConstraintRanges()
{
  this->ConstraintWholeRange();
  this->ConstraintRange(this->Range);
}

//----------------------------------------------------------------------------
void vtkKWRange::SetOrientation(int arg)
{
  if (this->Orientation == arg ||
      arg < vtkKWRange::ORIENTATION_HORIZONTAL ||
      arg > vtkKWRange::ORIENTATION_VERTICAL)
    {
    return;
    }

  this->Orientation = arg;

  this->Modified();

  this->Pack();

  this->RedrawCanvas();
}

//----------------------------------------------------------------------------
void vtkKWRange::SetInverted(int arg)
{
  if (this->Inverted == arg)
    {
    return;
    }

  this->Inverted = arg;

  this->Pack();

  this->RedrawCanvas();
}

// ----------------------------------------------------------------------------
void vtkKWRange::SetShowEntries(int _arg)
{
  if (this->ShowEntries == _arg)
    {
    return;
    }
  this->ShowEntries = _arg;
  this->Modified();

  if (this->ShowEntries)
    {
    this->CreateEntries();
    }

  this->Pack();
}

// ----------------------------------------------------------------------------
void vtkKWRange::SetShowZoomButtons(int _arg)
{
  if (this->ShowZoomButtons == _arg)
    {
    return;
    }
  this->ShowZoomButtons = _arg;
  this->Modified();

  if (this->ShowZoomButtons)
    {
    this->CreateZoomButtons();
    }

  this->Pack();
}

// ----------------------------------------------------------------------------
void vtkKWRange::SetLabelPosition(int _arg)
{
  if (this->LabelPosition == _arg)
    {
    return;
    }
  this->LabelPosition = _arg;
  this->Modified();

  this->Pack();
}

// ----------------------------------------------------------------------------
void vtkKWRange::SetEntriesPosition(int _arg)
{
  if (this->EntriesPosition == _arg)
    {
    return;
    }
  this->EntriesPosition = _arg;
  this->Modified();

  this->Pack();
}

// ----------------------------------------------------------------------------
void vtkKWRange::SetZoomButtonsPosition(int _arg)
{
  if (this->ZoomButtonsPosition == _arg)
    {
    return;
    }
  this->ZoomButtonsPosition = _arg;
  this->Modified();

  this->Pack();
}

//----------------------------------------------------------------------------
void vtkKWRange::SetEntriesWidth(int arg)
{
  if (this->EntriesWidth == arg || arg <= 0)
    {
    return;
    }

  this->EntriesWidth = arg;

  this->Modified();

  for (int i = 0; i < VTK_KW_RANGE_NB_ENTRIES; i++)
    {
    if (this->Entries[i])
      {
      this->Entries[i]->SetWidth(this->EntriesWidth);
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWRange::SetThickness(int arg)
{
  if (this->Thickness == arg || arg < VTK_KW_RANGE_MIN_THICKNESS)
    {
    return;
    }

  this->Thickness = arg;

  this->Modified();

  this->RedrawCanvas();
}

//----------------------------------------------------------------------------
void vtkKWRange::SetInternalThickness(float arg)
{
  if (this->InternalThickness == arg || 
      arg < 0.0 || arg > 1.0)
    {
    return;
    }

  this->InternalThickness = arg;

  this->Modified();

  this->RedrawCanvas();
}

//----------------------------------------------------------------------------
void vtkKWRange::SetSliderSize(int arg)
{
  if (this->SliderSize == arg || 
      arg < VTK_KW_RANGE_MIN_SLIDER_SIZE)
    {
    return;
    }

  this->SliderSize = arg;

  this->Modified();

  this->RedrawCanvas();
}

//----------------------------------------------------------------------------
void vtkKWRange::SetRangeColor(float r, float g, float b)
{
  if ((r == this->RangeColor[0] &&
       g == this->RangeColor[1] &&
       b == this->RangeColor[2]) ||
      r < 0.0 || r > 1.0 ||
      g < 0.0 || g > 1.0 ||
      b < 0.0 || b > 1.0)
    {
    return;
    }

  this->RangeColor[0] = r;
  this->RangeColor[1] = g;
  this->RangeColor[2] = b;

  this->Modified();

  this->UpdateColors();
}

//----------------------------------------------------------------------------
void vtkKWRange::SetRangeInteractionColor(float r, float g, float b)
{
  if ((r == this->RangeInteractionColor[0] &&
       g == this->RangeInteractionColor[1] &&
       b == this->RangeInteractionColor[2]) ||
      r < 0.0 || r > 1.0 ||
      g < 0.0 || g > 1.0 ||
      b < 0.0 || b > 1.0)
    {
    return;
    }

  this->RangeInteractionColor[0] = r;
  this->RangeInteractionColor[1] = g;
  this->RangeInteractionColor[2] = b;

  this->Modified();

  this->UpdateColors();
}

//----------------------------------------------------------------------------
void vtkKWRange::GetWholeRangeColor(int type, int &r, int &g, int &b)
{
  if (!this->IsCreated())
    {
    return;
    }

  float fr, fg, fb;
  float fh, fs, fv;

  switch (type)
    {
    case vtkKWRange::DARK_SHADOW_COLOR:
    case vtkKWRange::LIGHT_SHADOW_COLOR:
    case vtkKWRange::HIGHLIGHT_COLOR:

      this->GetWholeRangeColor(vtkKWRange::BACKGROUND_COLOR, r, g, b);

      fr = (float)r / 255.0;
      fg = (float)g / 255.0;
      fb = (float)b / 255.0;

      if (fr == fg && fg == fb)
        {
        fh = fs = 0.0;
        fv = fr;
        }
      else
        {
        vtkMath::RGBToHSV(fr, fg, fb, &fh, &fs, &fv);
        }

      if (type == vtkKWRange::DARK_SHADOW_COLOR)
        {
        fv *= 0.3;
        }
      else if (type == vtkKWRange::LIGHT_SHADOW_COLOR)
        {
        fv *= 0.6;
        }
      else
        {
        fv = 1.0;
        }

      vtkMath::HSVToRGB(fh, fs, fv, &fr, &fg, &fb);

      r = (int)(fr * 255.0);
      g = (int)(fg * 255.0);
      b = (int)(fb * 255.0);

      break;

    case vtkKWRange::BACKGROUND_COLOR:
    default:

      vtkKWTkUtilities::GetBackgroundColor(
        this->GetApplication()->GetMainInterp(),
        this->Canvas->GetWidgetName(),
        &r, &g, &b);

      break;
    }
}

//----------------------------------------------------------------------------
void vtkKWRange::GetRangeColor(int type, int &r, int &g, int &b)
{
  if (!this->IsCreated())
    {
    return;
    }

  float fr, fg, fb;
  float fh, fs, fv;
  float *rgb;

  switch (type)
    {
    case vtkKWRange::DARK_SHADOW_COLOR:
    case vtkKWRange::LIGHT_SHADOW_COLOR:
    case vtkKWRange::HIGHLIGHT_COLOR:

      this->GetRangeColor(vtkKWRange::BACKGROUND_COLOR, r, g, b);

      fr = (float)r / 255.0;
      fg = (float)g / 255.0;
      fb = (float)b / 255.0;

      if (fr == fg && fg == fb)
        {
        fh = fs = 0.0;
        fv = fr;
        }
      else
        {
        vtkMath::RGBToHSV(fr, fg, fb, &fh, &fs, &fv);
        }

      if (type == vtkKWRange::DARK_SHADOW_COLOR)
        {
        fv *= 0.3;
        }
      else if (type == vtkKWRange::LIGHT_SHADOW_COLOR)
        {
        fv *= 0.6;
        }
      else
        {
        fv = 1.0;
        }

      vtkMath::HSVToRGB(fh, fs, fv, &fr, &fg, &fb);

      r = (int)(fr * 255.0);
      g = (int)(fg * 255.0);
      b = (int)(fb * 255.0);

      break;

    case vtkKWRange::BACKGROUND_COLOR:
    default:

      rgb = (this->InInteraction ? 
             this->RangeInteractionColor : this->RangeColor);

      if (rgb[0] < 0 || rgb[1] < 0 || rgb[2] < 0)
        {
        this->GetWholeRangeColor(vtkKWRange::BACKGROUND_COLOR, r, g, b);
        }
      else
        {
        r = (int)(rgb[0] * 255.0);
        g = (int)(rgb[1] * 255.0);
        b = (int)(rgb[2] * 255.0);
        }

      break;
    }
}

//----------------------------------------------------------------------------
void vtkKWRange::GetSliderColor(int type, int &r, int &g, int &b)
{
  this->GetWholeRangeColor(type, r, g, b);
}

//----------------------------------------------------------------------------
void vtkKWRange::InvokeCommand()
{
  if (this->Command && !this->DisableCommands)
    {
    this->Script("eval %s",this->Command);
    }
}

//----------------------------------------------------------------------------
void vtkKWRange::InvokeStartCommand()
{
  if (this->StartCommand && !this->DisableCommands)
    {
    this->Script("eval %s", this->StartCommand);
    }
}

//----------------------------------------------------------------------------
void vtkKWRange::InvokeEndCommand()
{
  if (this->EndCommand && !this->DisableCommands)
    {
    this->Script("eval %s", this->EndCommand);
    }
}

//----------------------------------------------------------------------------
void vtkKWRange::InvokeEntriesCommand()
{
  if (this->EntriesCommand && !this->DisableCommands)
    {
    this->Script("eval %s", this->EntriesCommand);
    }
}

//----------------------------------------------------------------------------
void vtkKWRange::SetCommand(vtkKWObject *object, const char *method)
{
  if (this->Command)
    {
    delete [] this->Command;
    this->Command = NULL;
    }

  if (!object)
    {
    return;
    }

  ostrstream command;
  command << object->GetTclName() << " " << method << ends;
  this->Command = command.str();
}

//----------------------------------------------------------------------------
void vtkKWRange::SetStartCommand(vtkKWObject *object, const char *method)
{
  if (this->StartCommand)
    {
    delete [] this->StartCommand;
    this->StartCommand = NULL;
    }

  if (!object)
    {
    return;
    }

  ostrstream command;
  command << object->GetTclName() << " " << method << ends;
  this->StartCommand = command.str();
}

//----------------------------------------------------------------------------
void vtkKWRange::SetEndCommand(vtkKWObject *object, const char *method)
{
  if (this->EndCommand)
    {
    delete [] this->EndCommand;
    this->EndCommand = NULL;
    }

  if (!object)
    {
    return;
    }

  ostrstream command;
  command << object->GetTclName() << " " << method << ends;
  this->EndCommand = command.str();
}

//----------------------------------------------------------------------------
void vtkKWRange::SetEntriesCommand(vtkKWObject *object, const char *method)
{
  if (this->EntriesCommand)
    {
    delete [] this->EntriesCommand;
    this->EntriesCommand = NULL;
    }

  if (!object)
    {
    return;
    }

  ostrstream command;
  command << object->GetTclName() << " " << method << ends;
  this->EntriesCommand = command.str();
}

//----------------------------------------------------------------------------
void vtkKWRange::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  if (this->CanvasFrame)
    {
    this->CanvasFrame->SetEnabled(this->Enabled);
    }

  if (this->Canvas)
    {
    this->Canvas->SetEnabled(this->Enabled);
    }

  for (int i = 0; i < VTK_KW_RANGE_NB_ENTRIES; i++)
    {
    if (this->Entries[i])
      {
      this->Entries[i]->SetEnabled(this->Enabled);
      }
    }

  if (this->ZoomButtons)
    {
    this->ZoomButtons->SetEnabled(this->Enabled);
    }

  if (this->Enabled)
    {
    this->Bind();
    }
  else
    {
    this->UnBind();
    }
}

// ---------------------------------------------------------------------------
void vtkKWRange::SetBalloonHelpString(const char *string)
{
  this->Superclass::SetBalloonHelpString(string);

  if (this->Canvas)
    {
    this->Canvas->SetBalloonHelpString(string);
    }

  for (int i = 0; i < VTK_KW_RANGE_NB_ENTRIES; i++)
    {
    if (this->Entries[i])
      {
      this->Entries[i]->SetBalloonHelpString(string);
      }
    }
}

// ---------------------------------------------------------------------------
void vtkKWRange::SetBalloonHelpJustification(int j)
{
  this->Superclass::SetBalloonHelpJustification(j);

  if (this->Canvas)
    {
    this->Canvas->SetBalloonHelpJustification(j);
    }

  for (int i = 0; i < VTK_KW_RANGE_NB_ENTRIES; i++)
    {
    if (this->Entries[i])
      {
      this->Entries[i]->SetBalloonHelpJustification(j);
      }
    }
}

//----------------------------------------------------------------------------
int vtkKWRange::HasTag(const char *tag, const char *suffix)
{
  if (!this->IsCreated())
    {
    return 0;
    }

  const char *res = this->Script(
    "%s gettags %s%s", 
    this->Canvas->GetWidgetName(), tag, (suffix ? suffix : ""));
  if (!res || !*res)
    {
    return 0;
    }

  return 1;
}

//----------------------------------------------------------------------------
void vtkKWRange::RedrawCanvas()
{
  if (!this->IsCreated())
    {
    return;
    }

  const char *canv = this->Canvas->GetWidgetName();

  // Resize the canvas

  int width, height;

  if (this->Orientation == vtkKWRange::ORIENTATION_HORIZONTAL)
    {
    width = atoi(this->Script("winfo width %s", 
                              this->CanvasFrame->GetWidgetName()));
    if (width < VTK_KW_RANGE_MIN_LENGTH)
      {
      width = VTK_KW_RANGE_MIN_LENGTH;
      }
    height = this->Thickness;
    }
  else
    {
    width = this->Thickness;
    height = atoi(this->Script("winfo height %s", 
                               this->CanvasFrame->GetWidgetName()));
    if (height < VTK_KW_RANGE_MIN_LENGTH)
      {
      height = VTK_KW_RANGE_MIN_LENGTH;
      }
    }

  this->Script("%s config -width %d -height %d -scrollregion {0 0 %d %d}",
               canv, width, height, width - 1, height - 1);

  // Draw the elements

  this->RedrawWholeRange();
  this->RedrawRange();
  this->RedrawSliders();

  this->UpdateColors();
}

//----------------------------------------------------------------------------
void vtkKWRange::RedrawWholeRange()
{
  if (!this->IsCreated())
    {
    return;
    }

  ostrstream tk_cmd;
  const char *canv = this->Canvas->GetWidgetName();
  const char *tag = VTK_KW_RANGE_WHOLE_RANGE_TAG;
  int was_created = this->HasTag(tag);

  int in_thick = (int)(this->Thickness * InternalThickness);
  if (in_thick < VTK_KW_RANGE_MIN_INTERNAL_THICKNESS)
    {
    in_thick = VTK_KW_RANGE_MIN_INTERNAL_THICKNESS;
    }

  int x_min, x_max, y_min, y_max;

  /* 
     x_min          x_max
     |               |
     v               v
     DDDDDDDDDDDDDDDDH <- y_min
     DLLLLLLLLLLLLLL.H
     DL..............H
     DL..............H
     DL..............H 
     D...............H
     HHHHHHHHHHHHHHHHH <- y_max
  */

  // Draw depending on the orientation

  if (this->Orientation == vtkKWRange::ORIENTATION_HORIZONTAL)
    {
    x_min = 0;
    x_max = atoi(this->Script("%s cget -width", canv)) - 1;
    y_min = (this->Thickness - in_thick) / 2;
    y_max = y_min + in_thick - 1;
    }
  else
    {
    x_min = (this->Thickness - in_thick) / 2;
    x_max = x_min + in_thick - 1;
    y_min = 0;
    y_max = atoi(this->Script("%s cget -height", canv)) - 1;
    }

  // '.' part (background)

  if (!was_created)
    {
    tk_cmd << canv << " create rectangle 0 0 0 0 "
           << "-tag {rtag wbgc " << tag << " " << tag << "b1}\n";
    }
    
  tk_cmd << canv << " coords " << tag << "b1 "
         << x_min + 1 << " " << y_min + 1 << " " 
         << x_max - 1 + RSTRANGE << " " << y_max - 1 + RSTRANGE << endl;

  // 'D' part (dark shadow)

  if (!was_created)
    {
    tk_cmd << canv << " create line 0 0 0 0 "
           << "-tag {ltag wdsc " << tag << " " << tag << "l1}\n";
    }

  tk_cmd << canv << " coords " << tag << "l1 "
         << x_min << " " << y_max - 1 << " "
         << x_min << " " << y_min << " " 
         << x_max - 1 + LSTRANGE << " " << y_min << endl;

  // 'H' part (highlight)

  if (!was_created)
    {
    tk_cmd << canv << " create line 0 0 0 0 "
           << "-tag {ltag whlc " << tag << " " << tag << "l2}\n";
    }

  tk_cmd << canv << " coords " << tag << "l2 "
         << x_max << " " << y_min << " "
         << x_max << " " << y_max << " "
         << x_min - LSTRANGE << " " << y_max << endl;

  // 'L' part (light shadow)

  if (!was_created)
    {
    tk_cmd << canv << " create line 0 0 0 0 "
           << "-tag {ltag wlsc " << tag << " " << tag << "l3}\n";
    }

  tk_cmd << canv << " coords " << tag << "l3 "
         << x_min + 1 << " " << y_max - 2 << " "
         << x_min + 1 << " " << y_min + 1 << " " 
         << x_max - 2 + LSTRANGE << " " << y_min + 1 << endl;

  tk_cmd << ends;
  this->Script(tk_cmd.str());
  tk_cmd.rdbuf()->freeze(0);
}

//----------------------------------------------------------------------------
void vtkKWRange::GetSlidersPositions(int pos[2])
{
  if (!this->IsCreated())
    {
    return;
    }

  const char *canv = this->Canvas->GetWidgetName();
  float whole_range = (this->WholeRange[1] - this->WholeRange[0]);
  int i, pos_min = 0, pos_max, pos_range;

  if (this->Orientation == vtkKWRange::ORIENTATION_HORIZONTAL)
    {
    pos_max = atoi(this->Script("%s cget -width", canv)) - 1;
    }
  else
    {
    pos_max = atoi(this->Script("%s cget -height", canv)) - 1;
    }

  pos_range = pos_max - pos_min;

  pos[0] = (int)((float)pos_range * 
                 ((this->Range[0] - this->WholeRange[0]) / whole_range));
  pos[1] = (int)((float)pos_range * 
                 ((this->Range[1] - this->WholeRange[0]) / whole_range));

  if (this->Inverted)
    {
    pos[0] = pos_max - pos[0];
    pos[1] = pos_max - pos[1];
    }
  else
    {
    pos[0] += pos_min;
    pos[1] += pos_min;
    }

  // Leave room for the slider so that it remains inside the widget

  for (i = 0; i < 2; i++)
    {
    if (pos[i] - this->SliderSize < 0)
      {
      pos[i] = this->SliderSize;
      }
    else if (pos[i] + this->SliderSize > pos_max)
      {
      pos[i] = pos_max - this->SliderSize;
      }
    }

}

//----------------------------------------------------------------------------
void vtkKWRange::RedrawRange()
{
  if (!this->IsCreated())
    {
    return;
    }

  ostrstream tk_cmd;
  const char *canv = this->Canvas->GetWidgetName();
  const char *tag = VTK_KW_RANGE_RANGE_TAG;
  int was_created = this->HasTag(tag);

  int pos[2];
  this->GetSlidersPositions(pos);

  int in_thick = (int)(this->Thickness * InternalThickness);
  if (in_thick < VTK_KW_RANGE_MIN_INTERNAL_THICKNESS)
    {
    in_thick = VTK_KW_RANGE_MIN_INTERNAL_THICKNESS;
    }

  if (!was_created)
    {
    // '.' part (background)
  
    tk_cmd << canv << " create rectangle 0 0 0 0 "
           << "-tag {rtag rbgc " << tag << " " << tag << "b1}\n";
    
    // 'D' part (dark shadow)

    tk_cmd << canv << " create line 0 0 0 0 "
           << "-tag {ltag rdsc " << tag << " " << tag << "l1}\n";

    // 'H' part (highlight)

    tk_cmd << canv << " create line 0 0 0 0 "
           << "-tag {ltag rhlc " << tag << " " << tag << "l2}\n";
  
    // 'L' part (light shadow)
  
    tk_cmd << canv << " create line 0 0 0 0 "
           << "-tag {ltag rlsc " << tag << " " << tag << "l3}\n";
    }

  // Draw depending on the orientation

  int min = (this->Thickness - in_thick) / 2;
  int max = min + in_thick - 1;

  if (this->Orientation == vtkKWRange::ORIENTATION_HORIZONTAL)
    {
    /* 
      pos[0]         pos[1]
       |               |
       v               v
       HHHHHHHHHHHHHHHHH <- min
       .................
       .................
       LLLLLLLLLLLLLLLLL
       DDDDDDDDDDDDDDDDD <- max
    */

    // '.' part (background)
  
    tk_cmd << canv << " coords " << tag << "b1 "
           << pos[0] << " " << min + 1 << " " 
           << pos[1] + RSTRANGE << " " << max - 2 + RSTRANGE << endl;

    // 'D' part (dark shadow)

    tk_cmd << canv << " coords " << tag << "l1 "
           << pos[0] << " " << max << " "
           << pos[1]  + LSTRANGE << " " << max << endl;

    // 'H' part (highlight)

    tk_cmd << canv << " coords " << tag << "l2 "
           << pos[0] << " " << min << " "
           << pos[1]  + LSTRANGE << " " << min << endl;

    // 'L' part (light shadow)

    tk_cmd << canv << " coords " << tag << "l3 "
           << pos[0] << " " << max - 1 << " "
           << pos[1]  + LSTRANGE << " " << max - 1 << endl;
    }
  else
    {
    /* 
      min  max
       |   |
       v   v
       H..LD <- pos[0]
       H..LD
       H..LD
       H..LD
       H..LD <- pos[1]
    */

    // '.' part (background)
  
    tk_cmd << canv << " coords " << tag << "b1 "
           << min + 1 << " " << pos[0] << " " 
           << max - 2 + RSTRANGE << " " << pos[1] + RSTRANGE << endl;

    // 'D' part (dark shadow)

    tk_cmd << canv << " coords " << tag << "l1 "
           << max << " " << pos[0] << " "
           << max << " " << pos[1] + LSTRANGE << endl;

    // 'H' part (highlight)

    tk_cmd << canv << " coords " << tag << "l2 "
           << min << " " << pos[0] << " "
           << min << " " << pos[1] + LSTRANGE << endl;

    // 'L' part (light shadow)

    tk_cmd << canv << " coords " << tag << "l3 "
           << max -1 << " " << pos[0] << " "
           << max - 1 << " " << pos[1] + LSTRANGE << endl;
    }

  tk_cmd << ends;
  this->Script(tk_cmd.str());
  tk_cmd.rdbuf()->freeze(0);
}

//----------------------------------------------------------------------------
void vtkKWRange::RedrawSliders()
{
  // Get the position of the sliders

  int pos[2];
  this->GetSlidersPositions(pos);

  // Draw the sliders

  this->RedrawSlider(pos[0], vtkKWRange::SLIDER_INDEX_1);
  this->RedrawSlider(pos[1], vtkKWRange::SLIDER_INDEX_2);
}

//----------------------------------------------------------------------------
void vtkKWRange::RedrawSlider(int pos, int slider_idx)
{
  if (!this->IsCreated())
    {
    return;
    }

  const char *tag = "";
  if (slider_idx == SLIDER_INDEX_1)
    {
    tag = VTK_KW_RANGE_SLIDER1_TAG;
    }
  else
    {
    tag = VTK_KW_RANGE_SLIDER2_TAG;
    }

  ostrstream tk_cmd;
  const char *canv = this->Canvas->GetWidgetName();
  int sw = this->SliderSize;
  const char *stag = VTK_KW_RANGE_SLIDERS_TAG;
  int was_created = this->HasTag(tag);

  int in_thick = (int)(this->Thickness * InternalThickness);
  if (in_thick < VTK_KW_RANGE_MIN_INTERNAL_THICKNESS)
    {
    in_thick = VTK_KW_RANGE_MIN_INTERNAL_THICKNESS;
    }
    
  int x_min, x_max, y_min, y_max, min_temp, max_temp;
  
  /*    x_min + sw - 1 
      x_min|pos (horiz)  
         | ||
         v vv 
         HHHHHHD <- y_min
         H.....D
         H.DDH.D <- y_min + sw - 1
         H.D H.D <- pos (vert)
         H.HHH.D <- y_max - sw + 1
         H.....D
         DDDDDDD <- y_max
             ^ ^
             | |
             | x_max
            x_max - sw + 1
  */

  // Draw depending on the orientation

  x_min = pos - sw;
  x_max = pos + sw;
#if 1
  y_min = 0;
  y_max = this->Thickness - 1;
#else
  y_min = (slider_idx == SLIDER_INDEX_1 ? 
           0 : (this->Thickness - in_thick) / 2);
  y_max = (slider_idx == SLIDER_INDEX_1 ? 
           (this->Thickness + in_thick) / 2 - 1 : this->Thickness - 1);
#endif

  if (this->Orientation == vtkKWRange::ORIENTATION_VERTICAL)
    {
    min_temp = x_min;
    max_temp = x_max;
    x_min = y_min;
    x_max = y_max;
    y_min = min_temp;
    y_max = max_temp;
    }

  // '.' part (background)

  if (sw > 2)
    {
    if (!this->HasTag(tag, "b1"))
      {
      tk_cmd << canv << " create rectangle 0 0 0 0 "
          << "-tag {rtag sbgc " << tag << " " << stag << " " << tag << "b1}\n";

      tk_cmd << canv << " create rectangle 0 0 0 0 "
          << "-tag {rtag sbgc " << tag << " " << stag << " " << tag << "b2}\n";

      tk_cmd << canv << " create rectangle 0 0 0 0 "
          << "-tag {rtag sbgc " << tag << " " << stag << " " << tag << "b3}\n";

      tk_cmd << canv << " create rectangle 0 0 0 0 "
          << "-tag {rtag sbgc " << tag << " " << stag << " " << tag << "b4}\n";
      }

    tk_cmd << canv << " coords " << tag << "b1 "
           << x_min + 1 << " " << y_min + 1 << " "
           << x_min + sw - 2 + RSTRANGE << " " << y_max - 1 + RSTRANGE << endl;

    tk_cmd << canv << " coords " << tag << "b2 "
           << x_max - sw + 2 << " " << y_min + 1 << " "
           << x_max - 1 + RSTRANGE << " " << y_max - 1 + RSTRANGE << endl;

    tk_cmd << canv << " coords " << tag << "b3 "
           << x_min + sw - 1 << " " << y_min + 1 << " "
           << x_max - sw + 1 + RSTRANGE << " " << y_min + sw - 2 + RSTRANGE
           << endl;

    tk_cmd << canv << " coords " << tag << "b4 "
           << x_min + sw - 1 << " " << y_max - sw + 2 << " "
           << x_max - sw + 1 + RSTRANGE << " " << y_max - 1 + RSTRANGE 
           << endl;
    }
    
  // 'D' part (dark shadow)

  if (!was_created)
    {
    tk_cmd << canv << " create line 0 0 0 0 "
         << " -tag {ltag sdsc " << tag << " " << stag << " " << tag << "l1}\n";
        
    tk_cmd << canv << " create line 0 0 0 0 "
        << " -tag {ltag sdsc " << tag << " " << stag << " " << tag << "l2}\n";
    }

  tk_cmd << canv << " coords " << tag << "l1 "
         << x_max << " " << y_min << " "
         << x_max << " " << y_max << " " 
         << x_min - LSTRANGE << " " << y_max << endl;

  tk_cmd << canv << " coords " << tag << "l2 "
         << x_min + sw - 1 << " " << y_max - sw << " "
         << x_min + sw - 1 << " " << y_min + sw - 1 << " " 
         << x_max - sw + LSTRANGE << " " << y_min + sw - 1 << endl;

  // 'H' part (highlight)

  if (!was_created)
    {
    tk_cmd << canv << " create line 0 0 0 0 "
        << " -tag {ltag shlc " << tag << " " << stag << " " << tag << "l3}\n";

    tk_cmd << canv << " create line 0 0 0 0 "
        << " -tag {ltag shlc " << tag << " " << stag << " " << tag << "l4}\n";
    }

  tk_cmd << canv << " coords " << tag << "l3 "
         << x_min << " " << y_max - 1 << " "
         << x_min << " " << y_min << " "
         << x_max - 1 + LSTRANGE << " " << y_min << endl;

  tk_cmd << canv << " coords " << tag << "l4 "
         << x_max - sw + 1 << " " << y_min + sw - 1 << " "
         << x_max - sw + 1 << " " << y_max - sw + 1 << " " 
         << x_min + sw - 1 - LSTRANGE << " " << y_max - sw + 1 << endl;

  tk_cmd << ends;
  this->Script(tk_cmd.str());
  tk_cmd.rdbuf()->freeze(0);
}

//----------------------------------------------------------------------------
void vtkKWRange::UpdateRangeColors()
{
  if (!this->IsCreated())
    {
    return;
    }

  ostrstream tk_cmd;
  const char *canv = this->Canvas->GetWidgetName();

  char bgcolor[10], dscolor[10], lscolor[10], hlcolor[10];
  int r, g, b;

  // Set the color of the Range

  this->GetRangeColor(vtkKWRange::BACKGROUND_COLOR, r, g, b);
  sprintf(bgcolor, "#%02x%02x%02x", r, g, b);

  this->GetRangeColor(vtkKWRange::DARK_SHADOW_COLOR, r, g, b);
  sprintf(dscolor, "#%02x%02x%02x", r, g, b);

  this->GetRangeColor(vtkKWRange::HIGHLIGHT_COLOR, r, g, b);
  sprintf(hlcolor, "#%02x%02x%02x", r, g, b);

  this->GetRangeColor(vtkKWRange::LIGHT_SHADOW_COLOR, r, g, b);
  sprintf(lscolor, "#%02x%02x%02x", r, g, b);

  tk_cmd << canv << " itemconfigure rbgc -outline {} -fill "<< bgcolor << endl;
  tk_cmd << canv << " itemconfigure rdsc -fill " << dscolor << endl;
  tk_cmd << canv << " itemconfigure rhlc -fill " << hlcolor << endl;
  tk_cmd << canv << " itemconfigure rlsc -fill " << lscolor << endl;

  tk_cmd << ends;
  this->Script(tk_cmd.str());
  tk_cmd.rdbuf()->freeze(0);
}

//----------------------------------------------------------------------------
void vtkKWRange::UpdateColors()
{
  if (!this->IsCreated())
    {
    return;
    }

  ostrstream tk_cmd;
  const char *canv = this->Canvas->GetWidgetName();

  char bgcolor[10], dscolor[10], lscolor[10], hlcolor[10];
  int r, g, b;

  // Set the color of the Whole Range

  this->GetWholeRangeColor(vtkKWRange::BACKGROUND_COLOR, r, g, b);
  sprintf(bgcolor, "#%02x%02x%02x", r, g, b);

  this->GetWholeRangeColor(vtkKWRange::DARK_SHADOW_COLOR, r, g, b);
  sprintf(dscolor, "#%02x%02x%02x", r, g, b);

  this->GetWholeRangeColor(vtkKWRange::HIGHLIGHT_COLOR, r, g, b);
  sprintf(hlcolor, "#%02x%02x%02x", r, g, b);

  this->GetWholeRangeColor(vtkKWRange::LIGHT_SHADOW_COLOR, r, g, b);
  sprintf(lscolor, "#%02x%02x%02x", r, g, b);

  tk_cmd << canv << " itemconfigure wbgc -outline {} -fill "<< bgcolor << endl;
  tk_cmd << canv << " itemconfigure wdsc -fill " << dscolor << endl;
  tk_cmd << canv << " itemconfigure whlc -fill " << hlcolor << endl;
  tk_cmd << canv << " itemconfigure wlsc -fill " << lscolor << endl;

  // Set the color of the Range

  this->UpdateRangeColors();

  // Set the color of all Sliders

  this->GetSliderColor(vtkKWRange::BACKGROUND_COLOR, r, g, b);
  sprintf(bgcolor, "#%02x%02x%02x", r, g, b);

  this->GetSliderColor(vtkKWRange::DARK_SHADOW_COLOR, r, g, b);
  sprintf(dscolor, "#%02x%02x%02x", r, g, b);

  this->GetSliderColor(vtkKWRange::HIGHLIGHT_COLOR, r, g, b);
  sprintf(hlcolor, "#%02x%02x%02x", r, g, b);

  tk_cmd << canv << " itemconfigure sbgc -outline {} -fill "<< bgcolor << endl;
  tk_cmd << canv << " itemconfigure sdsc -fill " << dscolor << endl;
  tk_cmd << canv << " itemconfigure shlc -fill " << hlcolor << endl;

  // Set line style

  tk_cmd << canv << " itemconfigure ltag -capstyle round " << endl;

  tk_cmd << ends;
  this->Script(tk_cmd.str());
  tk_cmd.rdbuf()->freeze(0);
}

// ---------------------------------------------------------------------------
void vtkKWRange::EntriesUpdateCallback(int i)
{
  if (i < 0 || i >= VTK_KW_RANGE_NB_ENTRIES ||
      !this->Entries[i] || !this->Entries[i]->IsCreated())
    {
    return;
    }

  float value = this->Entries[i]->GetValueAsFloat();
  float old_value = this->Range[i];

  if (i == 0)
    {
    this->SetRange(value, this->Range[1]);
    }
  else
    {
    this->SetRange(this->Range[0], value);
    }

  if (this->Range[i] != old_value)
    {
    this->InvokeEntriesCommand();
    }
}

//----------------------------------------------------------------------------
void vtkKWRange::ConfigureCallback()
{
  this->RedrawCanvas();
}

//----------------------------------------------------------------------------
void vtkKWRange::MaximizeRangeCallback()
{
  this->SetRange(this->GetWholeRange());
}

//----------------------------------------------------------------------------
void vtkKWRange::EnlargeRangeCallback()
{
  float *range = this->GetRange();
  float delta2 = ((range[1] - range[0]) / 2.0) * 2.0;
  float center = (range[1] + range[0]) / 2.0;
  this->SetRange(center - delta2, center + delta2);
}

//----------------------------------------------------------------------------
void vtkKWRange::ShrinkRangeCallback()
{
  float *range = this->GetRange();
  float delta2 = ((range[1] - range[0]) / 2.0) / 2.0;
  float center = (range[1] + range[0]) / 2.0;
  this->SetRange(center - delta2, center + delta2);
}

//----------------------------------------------------------------------------
void vtkKWRange::StartInteractionCallback(int x, int y)
{
  if (this->InInteraction)
    {
    return;
    }

  this->InInteraction = 1;

  // Save the current range and mouse position

  if (this->Orientation == vtkKWRange::ORIENTATION_HORIZONTAL)
    {
    this->StartInteractionPos = x;
    }
  else
    {
    this->StartInteractionPos = y;
    }
  this->StartInteractionRange[0] = this->Range[0];
  this->StartInteractionRange[1] = this->Range[1];

  this->UpdateRangeColors();
  this->InvokeStartCommand();
}

//----------------------------------------------------------------------------
void vtkKWRange::EndInteractionCallback()
{
  if (!this->InInteraction)
    {
    return;
    }

  this->InInteraction = 0;
  this->UpdateRangeColors();
  this->InvokeEndCommand();
}

//----------------------------------------------------------------------------
void vtkKWRange::SliderMotionCallback(int slider_idx, int x, int y)
{
  if (!this->IsCreated())
    {
    return;
    }

  const char *canv = this->Canvas->GetWidgetName();
  float whole_range = (this->WholeRange[1] - this->WholeRange[0]);

  // Update depending on the orientation

  int min, max, pos;

  if (this->Orientation == vtkKWRange::ORIENTATION_HORIZONTAL)
    {
    pos = x;
    min = 0;
    max = atoi(this->Script("%s cget -width", canv)) - 1;
    }
  else
    {
    pos = y;
    min = 0;
    max = atoi(this->Script("%s cget -height", canv)) - 1;
    }

  float new_value;
  if (this->Inverted)
    {
    new_value = (float)(max - pos);
    }
  else
    {
    new_value = (float)(pos - min);
    }
  new_value = 
    this->WholeRange[0] + (new_value / (float)(max - min)) * whole_range;

  if (slider_idx == vtkKWRange::SLIDER_INDEX_1)
    {
    this->SetRange(new_value, this->Range[1]);
    }
  else
    {
    this->SetRange(this->Range[0], new_value);
    }
}

//----------------------------------------------------------------------------
void vtkKWRange::RangeMotionCallback(int x, int y)
{
  if (!this->IsCreated())
    {
    return;
    }

  const char *canv = this->Canvas->GetWidgetName();
  float whole_range = (this->WholeRange[1] - this->WholeRange[0]);

  // Update depending on the orientation

  int pos, min, max;

  if (this->Orientation == vtkKWRange::ORIENTATION_HORIZONTAL)
    {
    pos = x;
    min = 0;
    max = atoi(this->Script("%s cget -width", canv)) - 1;
    }
  else
    {
    pos = y;
    min = 0;
    max = atoi(this->Script("%s cget -height", canv)) - 1;
    }

  float rel_delta = whole_range * 
    (float)((pos - this->StartInteractionPos) - min) / (float)(max - min);
  if (this->Inverted)
    {
    rel_delta = -rel_delta;
    }

  float new_range[2];
  new_range[0] = this->StartInteractionRange[0] + rel_delta;
  new_range[1] = this->StartInteractionRange[1] + rel_delta;

  // Check if the constrained new range has the same "width" as the old one

  if (!this->SliderCanPush)
    {
    float old_delta = 
      (this->StartInteractionRange[1] - this->StartInteractionRange[0]);
    this->ConstraintRange(new_range, this->Range);

    if (fabs(old_delta - (new_range[1] - new_range[0])) >= this->Resolution)
      {
      if (whole_range * rel_delta > 0)  // I just care of the sign
        {
        new_range[0] = new_range[1] - old_delta;
        }
      else
        {
        new_range[1] = new_range[0] + old_delta;
        }
      }
    }

  this->SetRange(new_range[0], new_range[1]);
}

//----------------------------------------------------------------------------
void vtkKWRange::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "WholeRange: " 
     << this->WholeRange[0] << "..." <<  this->WholeRange[1] << endl;
  os << indent << "Range: " 
     << this->Range[0] << "..." <<  this->Range[1] << endl;
  os << indent << "Resolution: " << this->Resolution << endl;
  os << indent << "EntriesResolution: " << this->EntriesResolution << endl;
  os << indent << "Thickness: " << this->Thickness << endl;
  os << indent << "InternalThickness: " << this->InternalThickness << endl;
  os << indent << "Orientation: "<< this->Orientation << endl;
  os << indent << "Inverted: "
     << (this->Inverted ? "On" : "Off") << endl;
  os << indent << "SliderSize: "<< this->SliderSize << endl;
  os << indent << "DisableCommands: "
     << (this->DisableCommands ? "On" : "Off") << endl;
  os << indent << "RangeColor: ("
     << this->RangeColor[0] << ", " 
     << this->RangeColor[1] << ", " 
     << this->RangeColor[2] << ")" << endl;
  os << indent << "RangeInteractionColor: ("
     << this->RangeInteractionColor[0] << ", " 
     << this->RangeInteractionColor[1] << ", " 
     << this->RangeInteractionColor[2] << ")" << endl;
  os << indent << "ShowEntries: " 
     << (this->ShowEntries ? "On" : "Off") << endl;
  os << indent << "ShowZoomButtons: " 
     << (this->ShowZoomButtons ? "On" : "Off") << endl;
  os << indent << "LabelPosition: " << this->LabelPosition << endl;
  os << indent << "EntriesPosition: " << this->EntriesPosition << endl;
  os << indent << "ZoomButtonsPosition: " << this->ZoomButtonsPosition << endl;
  os << indent << "EntriesWidth: " << this->EntriesWidth << endl;
  os << indent << "SliderCanPush: "
     << (this->SliderCanPush ? "On" : "Off") << endl;
  os << indent << "AdjustResolution: "
     << (this->AdjustResolution ? "On" : "Off") << endl;
  os << indent << "Canvas: "<< this->Canvas << endl;
}

