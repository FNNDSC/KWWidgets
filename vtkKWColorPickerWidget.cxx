/*=========================================================================

  Module:    $RCSfile: vtkKWColorPickerWidget.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWColorPickerWidget.h"

#include "vtkColorTransferFunction.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"

#include "vtkKWApplication.h"
#include "vtkKWColorPresetSelector.h"
#include "vtkKWColorSpectrumWidget.h"
#include "vtkKWColorSwatchesWidget.h"
#include "vtkKWColorTransferFunctionEditor.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWEvent.h"
#include "vtkKWFrame.h"
#include "vtkKWIcon.h"
#include "vtkKWLabel.h"
#include "vtkKWLabelSet.h"
#include "vtkKWLabelWithLabel.h"
#include "vtkKWNotebook.h"
#include "vtkKWPushButton.h"
#include "vtkKWRadioButton.h"

#include <vtksys/ios/sstream>
#include <vtksys/stl/string>

#define VTK_KW_COLOR_PICKER_WIDGET_SPECTRUM_TAG 0
#define VTK_KW_COLOR_PICKER_WIDGET_COLOR_SWATCHES_TAG 1
#define VTK_KW_COLOR_PICKER_WIDGET_FAVORITES_TAG 2
#define VTK_KW_COLOR_PICKER_WIDGET_HISTORY_TAG 3

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWColorPickerWidget );
vtkCxxRevisionMacro(vtkKWColorPickerWidget, "$Revision: 1.19 $");

//----------------------------------------------------------------------------
class vtkKWColorPickerWidgetInternals
{
public:
  vtksys_stl::string ScheduleUpdateInfoLabelTimerId;
};

//----------------------------------------------------------------------------
vtkKWColorPickerWidget::vtkKWColorPickerWidget()
{
  this->Internals = new vtkKWColorPickerWidgetInternals;

  this->ColorSpectrumVisibility = 1;
  this->ColorSwatchesVisibility = 1;
  this->FavoritesVisibility = 1;
  this->HistoryVisibility = 1;
  this->CompactMode = 0;

  this->EventCallData = NULL;

  this->SlidersFrame    = NULL;
  this->RGBSlidersFrame = NULL;
  this->HSVSlidersFrame = NULL;

  int i;
  for (i = 0; i < 3; i++)
    {
    this->RGBSliders[i] = NULL;
    this->HSVSliders[i] = NULL;
    }

  for (i = 0; i < 6; i++)
    {
    this->FixedAxisRadioButton[i] = NULL;
    this->SliderUnitLabel[i] = NULL;
    }

  this->Notebook                     = NULL;
  this->HexadecimalColorEntry        = NULL;
  this->ColorSpectrumWidget          = NULL;
  this->FavoritesColorPresetSelector = vtkKWColorPresetSelector::New();
  this->HistoryColorPresetSelector   = vtkKWColorPresetSelector::New();
  this->ColorSwatchesWidget          = NULL;
  this->ColorsFrame                  = NULL;
  this->ColorsLabelSet               = NULL;
  this->ColorsNameLabelSet           = NULL;
  this->InfoLabel                    = NULL;
  this->CompactModeButton            = NULL;

  this->InternalCurrentColorAsRGB[0] = -1; /* unitialized */
  this->InternalCurrentColorAsRGB[1] = -1;
  this->InternalCurrentColorAsRGB[2] = -1;
  this->InternalCurrentColorAsHSV[0] = -1;
  this->InternalCurrentColorAsHSV[1] = -1;
  this->InternalCurrentColorAsHSV[2] = -1;

  this->SetCurrentColorAsHSV(0.5, 1.0, 1.0);

  this->InternalNewColorAsRGB[0] = -1; /* unitialized */
  this->InternalNewColorAsRGB[1] = -1;
  this->InternalNewColorAsRGB[2] = -1;
  this->InternalNewColorAsHSV[0] = -1;
  this->InternalNewColorAsHSV[1] = -1;
  this->InternalNewColorAsHSV[2] = -1;
}

//----------------------------------------------------------------------------
vtkKWColorPickerWidget::~vtkKWColorPickerWidget()
{
  if (this->SlidersFrame)
    {
    this->SlidersFrame->Delete();
    this->SlidersFrame = NULL;
    }

  if (this->ColorsFrame)
    {
    this->ColorsFrame->Delete();
    this->ColorsFrame = NULL;
    }

  if (this->ColorsLabelSet)
    {
    this->ColorsLabelSet->Delete();
    this->ColorsLabelSet = NULL;
    }

  if (this->ColorsNameLabelSet)
    {
    this->ColorsNameLabelSet->Delete();
    this->ColorsNameLabelSet = NULL;
    }

  int i;
  for (i = 0; i < 6; i++)
    {
    this->FixedAxisRadioButton[i]->Delete();
    this->FixedAxisRadioButton[i] = NULL;

    this->SliderUnitLabel[i]->Delete();
    this->SliderUnitLabel[i] = NULL;
    }

  if (this->RGBSlidersFrame)
    {
    this->RGBSlidersFrame->Delete();
    this->RGBSlidersFrame = NULL;
    }

  if (this->HSVSlidersFrame)
    {
    this->HSVSlidersFrame->Delete();
    this->HSVSlidersFrame = NULL;
    }

  for (i = 0; i < 3; i++)
    {
    if (this->RGBSliders[i])
      {
      this->RGBSliders[i]->Delete();
      this->RGBSliders[i] = NULL;
      }
    if (this->HSVSliders[i])
      {
      this->HSVSliders[i]->Delete();
      this->HSVSliders[i] = NULL;
      }
    }

  if (this->Notebook)
    {
    this->Notebook->Delete();
    this->Notebook = NULL;
    }

  if (this->HexadecimalColorEntry)
    {
    this->HexadecimalColorEntry->Delete();
    this->HexadecimalColorEntry = NULL;
    }

  if (this->InfoLabel)
    {
    this->InfoLabel->Delete();
    this->InfoLabel = NULL;
    }

  if (this->CompactModeButton)
    {
    this->CompactModeButton->Delete();
    this->CompactModeButton = NULL;
    }

  if (this->ColorSpectrumWidget)
    {
    this->ColorSpectrumWidget->Delete();
    this->ColorSpectrumWidget = NULL;
    }

  if (this->FavoritesColorPresetSelector)
    {
    this->FavoritesColorPresetSelector->Delete();
    this->FavoritesColorPresetSelector = NULL;
    }

  if (this->HistoryColorPresetSelector)
    {
    this->HistoryColorPresetSelector->Delete();
    this->HistoryColorPresetSelector = NULL;
    }

  if (this->ColorSwatchesWidget)
    {
    this->ColorSwatchesWidget->Delete();
    this->ColorSwatchesWidget = NULL;
    }

  delete this->Internals;
  this->Internals = NULL;
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  vtksys_ios::ostringstream tk_cmd;
  int i, page_id;

  vtkKWIcon *icon = vtkKWIcon::New();

  // --------------------------------------------------------------

  if (this->InternalNewColorAsRGB[0] == -1 ||
      this->InternalNewColorAsRGB[1] == -1 ||
      this->InternalNewColorAsRGB[2] == -1 ||
      this->InternalNewColorAsHSV[0] == -1 ||
      this->InternalNewColorAsHSV[1] == -1 ||
      this->InternalNewColorAsHSV[2] == -1)
    {
    this->SetNewColorAsHSV(this->GetCurrentColorAsHSV());
    }

  // --------------------------------------------------------------
  // Notebook

  if (!this->Notebook)
    {
    this->Notebook = vtkKWNotebook::New();
    }
  this->Notebook->SetParent(this);
  this->Notebook->Create();
  this->Notebook->AlwaysShowTabsOn();
  this->Notebook->ShowIconsOn();
  this->Notebook->SetSelectedPageTabPadding(2);

  this->AddCallbackCommandObserver(
    this->Notebook, vtkKWEvent::NotebookRaisePageEvent);

  // --------------------------------------------------------------
  // Spectrum

  icon->SetImage(vtkKWIcon::IconHSVDiagram);

  page_id = this->Notebook->AddPage(
    NULL, "Spectrum", icon, VTK_KW_COLOR_PICKER_WIDGET_SPECTRUM_TAG);
  this->Notebook->SetPageVisibility(
    page_id, this->ColorSpectrumVisibility);

  if (!this->ColorSpectrumWidget)
    {
    this->ColorSpectrumWidget = vtkKWColorSpectrumWidget::New();
    }
  this->ColorSpectrumWidget->SetParent(
    this->Notebook->GetFrame(page_id));
  this->ColorSpectrumWidget->FixedAxisSelectorVisibilityOff();
  this->ColorSpectrumWidget->Create();
  this->ColorSpectrumWidget->SetBorderWidth(2);
  this->ColorSpectrumWidget->SetReliefToSunken();

  this->AddCallbackCommandObserver(
    this->ColorSpectrumWidget, vtkKWColorSpectrumWidget::ColorChangedEvent);
  this->AddCallbackCommandObserver(
    this->ColorSpectrumWidget, vtkKWColorSpectrumWidget::ColorChangingEvent);

  tk_cmd << "pack " << this->ColorSpectrumWidget->GetWidgetName() 
         << " -side left -anchor nw -expand y -fill both -padx 2 -pady 2" 
         << endl;

  // --------------------------------------------------------------
  // Color swatches

  icon->SetImage(vtkKWIcon::IconSilkColorSwatch);

  page_id = this->Notebook->AddPage(
    NULL, "Color Swatches", icon, VTK_KW_COLOR_PICKER_WIDGET_COLOR_SWATCHES_TAG);
  this->Notebook->SetPageVisibility(page_id, this->ColorSwatchesVisibility);

  this->CreateColorSwatchesWidget();

  // --------------------------------------------------------------
  // Favorites presets

  icon->SetImage(vtkKWIcon::IconFavorites);

  page_id = this->Notebook->AddPage(
    NULL, "Favorites", icon, VTK_KW_COLOR_PICKER_WIDGET_FAVORITES_TAG);
  this->Notebook->SetPageVisibility(page_id, this->FavoritesVisibility);

  this->CreateFavoritesColorPresetSelector();

  // --------------------------------------------------------------
  // History

  icon->SetImage(vtkKWIcon::IconTime);

  page_id = this->Notebook->AddPage(
    NULL, "History", icon, VTK_KW_COLOR_PICKER_WIDGET_HISTORY_TAG);
  this->Notebook->SetPageVisibility(page_id, this->HistoryVisibility);

  this->HistoryColorPresetSelector->SetPresetApplyCommand(
    this, "HistoryColorPresetApplyCallback ");

  this->CreateHistoryColorPresetSelector();

  // --------------------------------------------------------------
  // Sliders frame

  if (!this->SlidersFrame)
    {
    this->SlidersFrame = vtkKWFrame::New();
    }
  this->SlidersFrame->SetParent(this);
  this->SlidersFrame->Create();

  // --------------------------------------------------------------
  // Colors

  if (!this->ColorsFrame)
    {
    this->ColorsFrame = vtkKWFrame::New();
    }
  this->ColorsFrame->SetParent(this->SlidersFrame);
  this->ColorsFrame->Create();

  if (!this->ColorsNameLabelSet)
    {
    this->ColorsNameLabelSet = vtkKWLabelSet::New();
    }
  this->ColorsNameLabelSet->SetParent(this->ColorsFrame);
  this->ColorsNameLabelSet->Create();
  this->ColorsNameLabelSet->PackHorizontallyOn();

  if (!this->ColorsLabelSet)
    {
    this->ColorsLabelSet = vtkKWLabelSet::New();
    }
  this->ColorsLabelSet->SetParent(this->ColorsFrame);
  this->ColorsLabelSet->Create();
  this->ColorsLabelSet->PackHorizontallyOn();
  this->ColorsLabelSet->SetBorderWidth(2);
  this->ColorsLabelSet->SetReliefToSunken();

  vtkKWLabel *label = NULL;
  for (i = 0; i <= 1; i++)
    {
    label = this->ColorsNameLabelSet->AddWidget(i);

    label = this->ColorsLabelSet->AddWidget(i);
    label->SetBorderWidth(0);
    }

  this->ColorsNameLabelSet->GetWidget(0)->SetText("new");
  this->ColorsNameLabelSet->GetWidget(1)->SetText("current");

  this->ColorsLabelSet->GetWidget(1)->SetBinding(
    "<Any-ButtonPress>", this, "CurrentColorCallback");

  // --------------------------------------------------------------
  // Hexadecimal Color 

  if (!this->HexadecimalColorEntry)
    {
    this->HexadecimalColorEntry = vtkKWEntryWithLabel::New();
    }
  this->HexadecimalColorEntry->SetParent(this->ColorsFrame);
  this->HexadecimalColorEntry->SetLabelText("#");
  this->HexadecimalColorEntry->Create();

  vtkKWEntry *entry = this->HexadecimalColorEntry->GetWidget();
  entry->SetWidth(7);
  entry->SetRestrictValueToHexadecimal();
  entry->SetCommand(this, "HexadecimalColorEntryCallback");

  // --------------------------------------------------------------
  // RGB sliders frame

  vtkKWFrame *rgb_frame;

  if (!this->RGBSlidersFrame)
    {
    this->RGBSlidersFrame = vtkKWFrame::New();
    }
  this->RGBSlidersFrame->SetParent(this->SlidersFrame);
  this->RGBSlidersFrame->Create();
  this->RGBSlidersFrame->SetBorderWidth(0);
  this->RGBSlidersFrame->SetReliefToGroove();
  rgb_frame = this->RGBSlidersFrame;

  // --------------------------------------------------------------
  // HSV sliders frame

  vtkKWFrame *hsv_frame;

  if (!this->HSVSlidersFrame)
    {
    this->HSVSlidersFrame = vtkKWFrame::New();
    }
  this->HSVSlidersFrame->SetParent(this->SlidersFrame);
  this->HSVSlidersFrame->Create();
  this->HSVSlidersFrame->SetBorderWidth(
    this->RGBSlidersFrame->GetBorderWidth());
  this->HSVSlidersFrame->SetRelief(this->RGBSlidersFrame->GetRelief());
  hsv_frame = this->HSVSlidersFrame;

  // --------------------------------------------------------------
  // Info label

  if (!this->InfoLabel)
    {
    this->InfoLabel = vtkKWLabelWithLabel::New();
    }
  this->InfoLabel->SetParent(this->SlidersFrame);
  this->InfoLabel->Create();
  this->InfoLabel->LabelVisibilityOff();
  this->InfoLabel->ExpandWidgetOn();
  this->InfoLabel->GetLabel()->SetImageToPredefinedIcon(
    vtkKWIcon::IconSilkHelp);

  label = this->InfoLabel->GetWidget();
  label->AdjustWrapLengthToWidthOn();

  // Can't decrease the font on Win32 for now, it's flickering :(

  double fr, fg, fb, fh, fs, fv;
  label->GetForegroundColor(&fr, &fg, &fb);
  vtkMath::RGBToHSV(fr, fg, fb, &fh, &fs, &fv);

  double br, bg, bb, bh, bs, bv;
  label->GetBackgroundColor(&br, &bg, &bb);
  vtkMath::RGBToHSV(br, bg, bb, &bh, &bs, &bv);

  fv = 0.7 * fv + 0.3 * bv;
  vtkMath::HSVToRGB(fh, fs, fv, &fr, &fg, &fb);
  label->SetForegroundColor(fr, fg, fb);

  // --------------------------------------------------------------
  // Compact mode

  if (!this->CompactModeButton)
    {
    this->CompactModeButton = vtkKWPushButton::New();
    }
  this->CompactModeButton->SetParent(this);
  this->CompactModeButton->Create();
  this->CompactModeButton->SetBorderWidth(0);
  this->CompactModeButton->SetCommand(this, "ToggleCompactMode");
  this->CompactModeButton->SetImageToPredefinedIcon(
    vtkKWIcon::IconSilkBulletToggleMinus);

  // Pack
  
  tk_cmd << "grid " << this->ColorsFrame->GetWidgetName() 
         << " -row 0 -column 0 -sticky {} -padx 0 -pady 0" 
         << endl;

  tk_cmd << "grid " << this->RGBSlidersFrame->GetWidgetName() 
         << " -row 1 -column 0 -sticky {} -padx 0 -pady 2" 
         << endl;

  tk_cmd << "grid " << this->HSVSlidersFrame->GetWidgetName() 
         << " -row 2 -column 0 -sticky {} -padx 0 -pady 2" 
         << endl;

  tk_cmd << "grid rowconfigure " 
         << this->InfoLabel->GetParent()->GetWidgetName() 
         << " 3 -weight 1" 
         << endl;

  // --------------------------------------------------------------
  // Fixed axis selector

  const int choices[6] = 
    {
      vtkKWColorSpectrumWidget::FixedAxisR,
      vtkKWColorSpectrumWidget::FixedAxisG,
      vtkKWColorSpectrumWidget::FixedAxisB,
      vtkKWColorSpectrumWidget::FixedAxisH,
      vtkKWColorSpectrumWidget::FixedAxisS,
      vtkKWColorSpectrumWidget::FixedAxisV
    };

  char command[128];
  for (i = 0; i < 6; i++)
    {
    if (!this->FixedAxisRadioButton[i])
      {
      this->FixedAxisRadioButton[i] = vtkKWRadioButton::New();
      }
    if (!this->SliderUnitLabel[i])
      {
      this->SliderUnitLabel[i] = vtkKWLabel::New();
      }
    if (i < 3)
      {
      this->FixedAxisRadioButton[i]->SetParent(rgb_frame);
      this->SliderUnitLabel[i]->SetParent(rgb_frame);
      }
    else
      {
      this->FixedAxisRadioButton[i]->SetParent(hsv_frame);
      this->SliderUnitLabel[i]->SetParent(hsv_frame);
      }
    this->FixedAxisRadioButton[i]->Create();
    this->FixedAxisRadioButton[i]->SetHighlightThickness(0);
    this->FixedAxisRadioButton[i]->SetBorderWidth(0);
    this->FixedAxisRadioButton[i]->SetPadX(0);
    this->FixedAxisRadioButton[i]->SetValueAsInt(choices[i]);
    this->FixedAxisRadioButton[i]->SetConfigurationOptionAsInt(
      "-takefocus", 0);

    if (i > 0)
      {
      this->FixedAxisRadioButton[i]->SetVariableName(
        this->FixedAxisRadioButton[0]->GetVariableName());
      }
    sprintf(command, "SetFixedAxis %d", choices[i]);
    this->FixedAxisRadioButton[i]->SetCommand(
      this->ColorSpectrumWidget, command);

    this->SliderUnitLabel[i]->Create();
    this->SliderUnitLabel[i]->SetWidth(1);
    }
  this->FixedAxisRadioButton[0]->SetVariableValueAsInt(
    this->ColorSpectrumWidget->GetFixedAxis());

  // --------------------------------------------------------------
  // RGB sliders

  const char *rgb_label[3] = {"R:", "G:", "B:"};
  const char *rgb_unit[3] = {" ", " ", " "};

  for (i = 0; i < 3; i++)
    {
    if (!this->RGBSliders[i])
      {
      this->RGBSliders[i] = vtkKWColorTransferFunctionEditor::New();
      }
    this->RGBSliders[i]->SetParent(rgb_frame);
    this->RGBSliders[i]->ExpandCanvasWidthOff();
    this->RGBSliders[i]->LabelVisibilityOn();
    this->RGBSliders[i]->SetLabelText(rgb_label[i]);
    this->RGBSliders[i]->SetLabelPositionToLeft();
    this->RGBSliders[i]->ParameterRangeLabelVisibilityOff();
    this->RGBSliders[i]->ParameterEntryVisibilityOn();
    this->RGBSliders[i]->PointEntriesVisibilityOn();
    this->RGBSliders[i]->SetPointEntriesPositionToRight();
    this->RGBSliders[i]->MidPointEntryVisibilityOff();
    this->RGBSliders[i]->SharpnessEntryVisibilityOff();
    this->RGBSliders[i]->SetParameterEntryFormat("%3.f");
    this->RGBSliders[i]->PointIndexVisibilityOff();
    this->RGBSliders[i]->SelectedPointIndexVisibilityOff();
    this->RGBSliders[i]->SetSelectedPointRadius(1.0);
    this->RGBSliders[i]->SetSelectedPointColor(
      this->RGBSliders[i]->GetPointColor());
    this->RGBSliders[i]->DisableAddAndRemoveOn();
    this->RGBSliders[i]->LockPointsValueOn();
    this->RGBSliders[i]->EnableDirectMoveOn();
    this->RGBSliders[i]->ComputePointColorFromValueOff();
    this->RGBSliders[i]->SetPointPositionInValueRangeToBottom();
    this->RGBSliders[i]->SetPointStyle(
      vtkKWParameterValueFunctionEditor::PointStyleCursorUp);
    this->RGBSliders[i]->FunctionLineVisibilityOff();
    this->RGBSliders[i]->SetPointMarginToCanvas(
      vtkKWParameterValueFunctionEditor::PointMarginHorizontalSides |
      vtkKWParameterValueFunctionEditor::PointMarginVerticalSides);
    this->RGBSliders[i]->ParameterRangeVisibilityOff();
    this->RGBSliders[i]->ValueRangeVisibilityOff();
    this->RGBSliders[i]->ValueRangeLabelVisibilityOff();
    this->RGBSliders[i]->ValueEntriesVisibilityOff();
    this->RGBSliders[i]->ColorSpaceOptionMenuVisibilityOff();
    this->RGBSliders[i]->ColorRampVisibilityOn();
    this->RGBSliders[i]->SetColorRampPositionToCanvas();
    this->RGBSliders[i]->CanvasOutlineVisibilityOff();
    this->RGBSliders[i]->CanvasBackgroundVisibilityOff();
    this->RGBSliders[i]->ChangeMouseCursorOff();
    this->RGBSliders[i]->Create();
    this->RGBSliders[i]->GetParameterEntry()->LabelVisibilityOff();
    this->RGBSliders[i]->GetParameterEntry()->GetWidget()->SetWidth(4);
    this->RGBSliders[i]->SetColorRampOutlineStyle(
        vtkKWColorTransferFunctionEditor::ColorRampOutlineStyleSunken);
    if (this->RGBSliders[i]->GetLabelVisibility())
      {
      this->RGBSliders[i]->GetLabel()->SetWidth(2);
      }

    this->AddCallbackCommandObserver(
      this->RGBSliders[i], 
      vtkKWParameterValueFunctionEditor::FunctionChangingEvent);
    this->AddCallbackCommandObserver(
      this->RGBSliders[i], 
      vtkKWParameterValueFunctionEditor::FunctionChangedEvent);

    this->SliderUnitLabel[i]->SetText(rgb_unit[i]);

    tk_cmd << "grid " << this->FixedAxisRadioButton[i]->GetWidgetName() 
           << " -pady 1 -sticky nws -column 0 -row " << i << endl;

    tk_cmd << "grid " << this->RGBSliders[i]->GetWidgetName() 
           << " -pady 1 -sticky nws -column 1 -row " << i << endl;

    tk_cmd << "grid " << this->SliderUnitLabel[i]->GetWidgetName() 
           << " -pady 1 -sticky nws -column 2 -row " << i << endl;
    }

  tk_cmd << "grid columnconfigure " 
         << this->SliderUnitLabel[0]->GetParent()->GetWidgetName() 
         << " 0 -uniform label" << endl;

  tk_cmd << "grid columnconfigure " 
         << this->SliderUnitLabel[0]->GetParent()->GetWidgetName() 
         << " 2 -uniform units" << endl;

  double r, g, b, s = 0.3, v = 1.0;

  vtkMath::HSVToRGB(0, s, v, &r, &g, &b);
  //  this->RGBSliders[0]->GetLabel()->SetBackgroundColor(r, g, b);
  this->RGBSliders[0]->GetParameterEntry()->GetWidget()->SetBackgroundColor(
    r, g, b);

  vtkMath::HSVToRGB(0.333, s, v, &r, &g, &b);
  //  this->RGBSliders[1]->GetLabel()->SetBackgroundColor(r, g, b);
  this->RGBSliders[1]->GetParameterEntry()->GetWidget()->SetBackgroundColor(
    r, g, b);

  vtkMath::HSVToRGB(0.666, s, v, &r, &g, &b);
  //  this->RGBSliders[2]->GetLabel()->SetBackgroundColor(r, g, b);
  this->RGBSliders[2]->GetParameterEntry()->GetWidget()->SetBackgroundColor(
    r, g, b);

  // --------------------------------------------------------------
  // HSV sliders

  const char *hsv_label[3] = {"H:", "S:", "V:"};
  const char *hsv_unit[3] = {"°", "%", "%"};

  for (i = 0; i < 3; i++)
    {
    if (!this->HSVSliders[i])
      {
      this->HSVSliders[i] = vtkKWColorTransferFunctionEditor::New();
      }
    this->HSVSliders[i]->SetParent(hsv_frame);
    this->HSVSliders[i]->SetExpandCanvasWidth(
      this->RGBSliders[i]->GetExpandCanvasWidth());
    this->HSVSliders[i]->SetLabelVisibility(
      this->RGBSliders[i]->GetLabelVisibility());
    this->HSVSliders[i]->SetLabelText(hsv_label[i]);
    this->HSVSliders[i]->SetLabelPosition(
      this->RGBSliders[i]->GetLabelPosition());
    this->HSVSliders[i]->SetParameterRangeLabelVisibility(
      this->RGBSliders[i]->GetParameterRangeLabelVisibility());
    this->HSVSliders[i]->SetParameterEntryVisibility(
      this->RGBSliders[i]->GetParameterEntryVisibility());
    this->HSVSliders[i]->SetPointEntriesPosition(
      this->RGBSliders[i]->GetPointEntriesPosition());
    this->HSVSliders[i]->SetPointEntriesVisibility(
      this->RGBSliders[i]->GetPointEntriesVisibility());
    this->HSVSliders[i]->SetMidPointEntryVisibility(
      this->RGBSliders[i]->GetMidPointEntryVisibility());
    this->HSVSliders[i]->SetSharpnessEntryVisibility(
      this->RGBSliders[i]->GetSharpnessEntryVisibility());
    this->HSVSliders[i]->SetParameterEntryFormat(
      this->RGBSliders[i]->GetParameterEntryFormat());
    this->HSVSliders[i]->SetPointIndexVisibility(
      this->RGBSliders[i]->GetPointIndexVisibility());
    this->HSVSliders[i]->SetSelectedPointIndexVisibility(
      this->RGBSliders[i]->GetSelectedPointIndexVisibility());
    this->HSVSliders[i]->SetSelectedPointRadius(
      this->RGBSliders[i]->GetSelectedPointRadius());
    this->HSVSliders[i]->SetSelectedPointColor(
      this->RGBSliders[i]->GetSelectedPointColor());
    this->HSVSliders[i]->SetDisableAddAndRemove(
      this->RGBSliders[i]->GetDisableAddAndRemove());
    this->HSVSliders[i]->SetLockPointsValue(
      this->RGBSliders[i]->GetLockPointsValue());
    this->HSVSliders[i]->SetEnableDirectMove(
      this->RGBSliders[i]->GetEnableDirectMove());
    this->HSVSliders[i]->SetComputePointColorFromValue(
      this->RGBSliders[i]->GetComputePointColorFromValue());
    this->HSVSliders[i]->SetPointPositionInValueRange(
      this->RGBSliders[i]->GetPointPositionInValueRange());
    this->HSVSliders[i]->SetPointStyle(
      this->RGBSliders[i]->GetPointStyle());
    this->HSVSliders[i]->SetFunctionLineVisibility(
      this->RGBSliders[i]->GetFunctionLineVisibility());
    this->HSVSliders[i]->SetPointMarginToCanvas(
      this->RGBSliders[i]->GetPointMarginToCanvas());
    this->HSVSliders[i]->SetParameterRangeVisibility(
      this->RGBSliders[i]->GetParameterRangeVisibility());
    this->HSVSliders[i]->SetValueRangeVisibility(
      this->RGBSliders[i]->GetValueRangeVisibility());
    this->HSVSliders[i]->SetValueRangeLabelVisibility(
      this->RGBSliders[i]->GetValueRangeLabelVisibility());
    this->HSVSliders[i]->SetValueEntriesVisibility(
      this->RGBSliders[i]->GetValueEntriesVisibility());
    this->HSVSliders[i]->SetColorSpaceOptionMenuVisibility(
      this->RGBSliders[i]->GetColorSpaceOptionMenuVisibility());
    this->HSVSliders[i]->SetColorRampVisibility(
      this->RGBSliders[i]->GetColorRampVisibility());
    this->HSVSliders[i]->SetColorRampPosition(
      this->RGBSliders[i]->GetColorRampPosition());
    this->HSVSliders[i]->SetCanvasOutlineVisibility(
      this->RGBSliders[i]->GetCanvasOutlineVisibility());
    this->HSVSliders[i]->SetCanvasBackgroundVisibility(
      this->RGBSliders[i]->GetCanvasBackgroundVisibility());
    this->HSVSliders[i]->SetChangeMouseCursor(
      this->RGBSliders[i]->GetChangeMouseCursor());
    this->HSVSliders[i]->Create();
    this->HSVSliders[i]->GetParameterEntry()->SetLabelVisibility(
      this->RGBSliders[i]->GetParameterEntry()->GetLabelVisibility());
    this->HSVSliders[i]->GetParameterEntry()->GetWidget()->SetWidth(
      this->RGBSliders[i]->GetParameterEntry()->GetWidget()->GetWidth());
    this->HSVSliders[i]->SetColorRampOutlineStyle(
      this->RGBSliders[i]->GetColorRampOutlineStyle());
    if (this->HSVSliders[i]->GetLabelVisibility())
      {
      this->HSVSliders[i]->GetLabel()->SetWidth(
        this->RGBSliders[i]->GetLabel()->GetWidth());
      }

    this->AddCallbackCommandObserver(
      this->HSVSliders[i], 
      vtkKWParameterValueFunctionEditor::FunctionChangingEvent);
    this->AddCallbackCommandObserver(
      this->HSVSliders[i], 
      vtkKWParameterValueFunctionEditor::FunctionChangedEvent);

    this->SliderUnitLabel[3 + i]->SetText(hsv_unit[i]);

    tk_cmd << "grid " << this->FixedAxisRadioButton[3 + i]->GetWidgetName() 
           << " -pady 1 -sticky nws -column 0 -row " << i << endl;

    tk_cmd << "grid " << this->HSVSliders[i]->GetWidgetName() 
           << " -pady 1 -sticky nws -column 1 -row " << i << endl;

    tk_cmd << "grid " << this->SliderUnitLabel[3 + i]->GetWidgetName() 
           << " -pady 1 -sticky nws -column 2 -row " << i << endl;
    }

  tk_cmd << "grid columnconfigure " 
         << this->SliderUnitLabel[3]->GetParent()->GetWidgetName() 
         << " 0 -uniform label" << endl;

  tk_cmd << "grid columnconfigure " 
         << this->SliderUnitLabel[3]->GetParent()->GetWidgetName() 
         << " 2 -uniform units" << endl;

  // --------------------------------------------------------------

  icon->Delete();

  this->Script(tk_cmd.str().c_str());

  // Update

  this->ColorSpectrumWidget->SetColorAsHSV(this->InternalNewColorAsHSV);

  this->UpdateSlidersRGB(this->InternalNewColorAsRGB);
  this->UpdateSlidersHSV(this->InternalNewColorAsHSV);

  this->UpdateColorLabel(
    this->ColorsLabelSet->GetWidget(0), this->InternalNewColorAsRGB);

  this->UpdateColorLabel(
    this->ColorsLabelSet->GetWidget(1), this->InternalCurrentColorAsRGB);

  this->UpdateHexadecimalColorEntry(this->InternalNewColorAsRGB);

  this->UpdateInfoLabel();

  this->Pack();

  // This helps top levels that would embed a color picker widget. 
  // (see vtkKWColorPickerDialog). Each time the dialog is brought back
  // to screen, make sure the label is updated immediately.

  this->AddBinding("<Map>", this, "UpdateInfoLabelCallback");

  this->AdjustToCompactMode();
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::CreateColorSwatchesWidget()
{
  if (!this->IsCreated())
    {
    return;
    }

  if (!this->ColorSwatchesWidget)
    {
    this->ColorSwatchesWidget = vtkKWColorSwatchesWidget::New();
    }

  if (this->ColorSwatchesWidget->IsCreated())
    {
    return;
    }

  this->ColorSwatchesWidget->SetParent(
    this->Notebook->GetFrame(NULL, 
                             VTK_KW_COLOR_PICKER_WIDGET_COLOR_SWATCHES_TAG));
  this->ColorSwatchesWidget->Create();
  this->ColorSwatchesWidget->AddDefaultCollections();
  this->ColorSwatchesWidget->SetSwatchSelectedCommand(
    this, "SwatchSelectedCallback");

  this->Script(
    "pack %s -side left -anchor nw -expand n -fill none -padx 2 -pady 2",
    this->ColorSwatchesWidget->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::CreateFavoritesColorPresetSelector()
{
  if (!this->IsCreated())
    {
    return;
    }

  if (!this->FavoritesColorPresetSelector)
    {
    this->FavoritesColorPresetSelector = vtkKWColorPresetSelector::New();
    }

  if (this->FavoritesColorPresetSelector->IsCreated())
    {
    return;
    }

  this->FavoritesColorPresetSelector->SetParent(
    this->Notebook->GetFrame(NULL, VTK_KW_COLOR_PICKER_WIDGET_FAVORITES_TAG));
  this->FavoritesColorPresetSelector->Create();
  this->FavoritesColorPresetSelector->SetPresetAddCommand(
    this, "FavoritesColorPresetAddCallback");
  this->FavoritesColorPresetSelector->SetPresetUpdateCommand(
    this, "FavoritesColorPresetUpdateCallback");
  this->FavoritesColorPresetSelector->SetPresetApplyCommand(
    this, "FavoritesColorPresetApplyCallback ");
  this->FavoritesColorPresetSelector->SetMaximumNumberOfPresets(64);
  this->FavoritesColorPresetSelector->SetListWidth(15);
  this->FavoritesColorPresetSelector->SetListHeight(4);

  this->Script("pack %s -side left -anchor nw -expand y -fill both",
               this->FavoritesColorPresetSelector->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::CreateHistoryColorPresetSelector()
{
  if (!this->IsCreated())
    {
    return;
    }

  if (!this->HistoryColorPresetSelector)
    {
    this->HistoryColorPresetSelector = vtkKWColorPresetSelector::New();
    }

  if (this->HistoryColorPresetSelector->IsCreated())
    {
    return;
    }

  this->HistoryColorPresetSelector->SetParent(
    this->Notebook->GetFrame(NULL, VTK_KW_COLOR_PICKER_WIDGET_HISTORY_TAG));
  this->HistoryColorPresetSelector->Create();
  this->HistoryColorPresetSelector->SetPresetApplyCommand(
    this, "HistoryColorPresetApplyCallback ");
  this->HistoryColorPresetSelector->SetMaximumNumberOfPresets(32);
  this->HistoryColorPresetSelector->SetListWidth(
    this->FavoritesColorPresetSelector->GetListWidth());
  this->HistoryColorPresetSelector->SetListHeight(
    this->FavoritesColorPresetSelector->GetListHeight());

  this->Script("pack %s -side left -anchor nw -expand y -fill both",
               this->HistoryColorPresetSelector->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::Pack()
{
  if (!this->IsCreated())
    {
    return;
    }

  this->UnpackChildren();

  vtksys_ios::ostringstream tk_cmd;

  tk_cmd << "pack " << this->SlidersFrame->GetWidgetName()
         << " -padx 2 -pady 2 -side left -anchor nw -expand n -fill y" 
         << endl;

  if (this->ColorSpectrumVisibility ||
      this->ColorSwatchesVisibility ||
      this->FavoritesVisibility ||
      this->HistoryVisibility)
    {
    tk_cmd << "pack " << this->Notebook->GetWidgetName()
           << " -padx 2 -pady 2 -side left -anchor nw -expand n -fill none" 
           << endl;
    }

  this->Script(tk_cmd.str().c_str());
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::AdjustToCompactMode()
{
  int i;
  int base_size;

  vtksys_ios::ostringstream tk_cmd;

  // Compact mode

  if (this->CompactMode)
    {
    base_size = 128;
    this->ColorSwatchesWidget->SetSwatchSize(9);
    this->ColorSwatchesWidget->SetMaximumNumberOfSwatchesPerRow(12);
    this->ColorSwatchesWidget->SetSwatchesPadding(1);
    for (i = 0; i <= 1; i++)
      {
      vtkKWLabel *label = this->ColorsLabelSet->GetWidget(i);
      label->SetWidth(7);
      label->SetHeight(1);
      }
    this->CompactModeButton->SetImageToPredefinedIcon(
      vtkKWIcon::IconSilkBulletTogglePlus);

    tk_cmd << "grid forget " << this->ColorsNameLabelSet->GetWidgetName() 
           << endl;
    
    tk_cmd << "grid " << this->HexadecimalColorEntry->GetWidgetName() 
           << " -row 1 -column 0 -sticky {w} -pady 0 -padx 2" << endl;

    tk_cmd << "grid forget " << this->InfoLabel->GetWidgetName() << endl;
    }

  // Full mode

  else
    {
    base_size = 256;
    this->ColorSwatchesWidget->SetSwatchSize(16);
    this->ColorSwatchesWidget->SetMaximumNumberOfSwatchesPerRow(13);
    this->ColorSwatchesWidget->SetSwatchesPadding(2);
    for (i = 0; i <= 1; i++)
      {
      vtkKWLabel *label = this->ColorsLabelSet->GetWidget(i);
      label->SetWidth(10);
      label->SetHeight(3);
      }
    this->CompactModeButton->SetImageToPredefinedIcon(
      vtkKWIcon::IconSilkBulletToggleMinus);

    tk_cmd << "grid " << this->ColorsNameLabelSet->GetWidgetName() 
           << " -row 0 -column 1 -sticky {ew} -pady 0" << endl;
    
    tk_cmd << "grid " << this->HexadecimalColorEntry->GetWidgetName() 
           << " -row 2 -column 1 -sticky {w} -pady 3 -padx 5" << endl;

    tk_cmd << "grid " << this->InfoLabel->GetWidgetName() 
           << " -row 3 -column 0 -sticky ews -padx 6 -pady 0" << endl;
    }

  this->Notebook->SetMinimumWidth(base_size + 12);
  this->Notebook->SetMinimumHeight(base_size + 39);
  this->ColorSpectrumWidget->SetCanvasSize(base_size);

  for (i = 0; i < 3; i++)
    {
    this->RGBSliders[i]->SetCanvasWidth(base_size);
    this->RGBSliders[i]->SetCanvasHeight(18);
    this->RGBSliders[i]->SetColorRampHeight(14);
    this->HSVSliders[i]->SetCanvasWidth(
      this->RGBSliders[i]->GetCanvasWidth());
    this->HSVSliders[i]->SetCanvasHeight(
      this->RGBSliders[i]->GetCanvasHeight());
    this->HSVSliders[i]->SetColorRampHeight(
      this->RGBSliders[i]->GetColorRampHeight());
    }

  tk_cmd << "grid " << this->ColorsLabelSet->GetWidgetName() 
         << " -row 1 -column 1 -sticky {ew} -pady 0" << endl;
    
  tk_cmd << "place " << this->CompactModeButton->GetWidgetName() 
         << " -anchor nw -x 0 -y 0" << endl;

  this->Script(tk_cmd.str().c_str());
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::SetInternalNewColorAsRGB(double rgb[3])
{
  this->SetInternalNewColorAsRGB(rgb[0], rgb[1], rgb[2]);
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::SetInternalNewColorAsRGB(
  double r, double g, double b)
{
  double range[2] = {0.0, 1.0};
  vtkMath::ClampValue(&r, range);
  vtkMath::ClampValue(&g, range);
  vtkMath::ClampValue(&b, range);

  if (r == this->InternalNewColorAsRGB[0] &&
      g == this->InternalNewColorAsRGB[1] &&
      b == this->InternalNewColorAsRGB[2])
    {
    return;
    }

  this->InternalNewColorAsRGB[0] = r;
  this->InternalNewColorAsRGB[1] = g;
  this->InternalNewColorAsRGB[2] = b;

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::SetNewColorAsRGB(double r, double g, double b)
{
  double range[2] = {0.0, 1.0};
  vtkMath::ClampValue(&r, range);
  vtkMath::ClampValue(&g, range);
  vtkMath::ClampValue(&b, range);

  double current_rgb[3];
  this->GetNewColorAsRGB(current_rgb);
  if (r == current_rgb[0] && g == current_rgb[1] && b == current_rgb[2])
    {
    return;
    }

  this->Modified();

  this->SetInternalNewColorAsRGB(r, g, b);

  double h, s, v;
  vtkMath::RGBToHSV(r, g, b, &h, &s, &v);
  this->SetInternalNewColorAsHSV(h, s, v);

  // Update UI (with RGB in mind)

  if (this->ColorsLabelSet)
    {
    this->UpdateColorLabel(
      this->ColorsLabelSet->GetWidget(0), this->InternalNewColorAsRGB);
    }

  this->UpdateSlidersRGB(this->InternalNewColorAsRGB);

  if (this->ColorSpectrumWidget)
    {
    this->ColorSpectrumWidget->SetColorAsRGB(this->InternalNewColorAsRGB);
    }

  this->UpdateHexadecimalColorEntry(this->InternalNewColorAsRGB);

  this->UpdateSlidersHSV(this->InternalNewColorAsHSV);

  this->ScheduleUpdateInfoLabel();
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::SetNewColorAsRGB(double rgb[3])
{
  this->SetNewColorAsRGB(rgb[0], rgb[1], rgb[2]);
}

//----------------------------------------------------------------------------
double* vtkKWColorPickerWidget::GetNewColorAsRGB()
{
  return this->GetInternalNewColorAsRGB();
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::GetNewColorAsRGB(
  double &_arg1, double &_arg2, double &_arg3)
{
  this->GetInternalNewColorAsRGB(_arg1, _arg2, _arg3);
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::GetNewColorAsRGB(double _arg[3])
{
  this->GetNewColorAsRGB(_arg[0], _arg[1], _arg[2]);
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::SetInternalNewColorAsHSV(double hsv[3])
{ 
  this->SetInternalNewColorAsHSV(hsv[0], hsv[1], hsv[2]); 
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::SetInternalNewColorAsHSV(
  double h, double s, double v)
{
  double range[2] = {0.0, 1.0};
  vtkMath::ClampValue(&h, range);
  vtkMath::ClampValue(&s, range);
  vtkMath::ClampValue(&v, range);

  if (h == this->InternalNewColorAsHSV[0] &&
      s == this->InternalNewColorAsHSV[1] &&
      v == this->InternalNewColorAsHSV[2])
    {
    return;
    }

  this->InternalNewColorAsHSV[0] = h;
  this->InternalNewColorAsHSV[1] = s;
  this->InternalNewColorAsHSV[2] = v;

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::SetNewColorAsHSV(double h, double s, double v)
{
  double range[2] = {0.0, 1.0};
  vtkMath::ClampValue(&h, range);
  vtkMath::ClampValue(&s, range);
  vtkMath::ClampValue(&v, range);

  double current_hsv[3];
  this->GetNewColorAsHSV(current_hsv);
  if (h == current_hsv[0] && s == current_hsv[1] && v == current_hsv[2])
    {
    return;
    }

  this->Modified();

  this->SetInternalNewColorAsHSV(h, s, v);

  double r, g, b;
  vtkMath::HSVToRGB(h, s, v, &r, &g, &b);
  this->SetInternalNewColorAsRGB(r, g, b);

  // Update UI (with HSV in mind)

  if (this->ColorsLabelSet)
    {
    this->UpdateColorLabel(
      this->ColorsLabelSet->GetWidget(0), this->InternalNewColorAsRGB);
    }

  this->UpdateSlidersRGB(this->InternalNewColorAsRGB);

  this->UpdateHexadecimalColorEntry(this->InternalNewColorAsRGB);

  if (this->ColorSpectrumWidget)
    {
    this->ColorSpectrumWidget->SetColorAsHSV(this->InternalNewColorAsHSV);
    }

  this->UpdateSlidersHSV(this->InternalNewColorAsHSV);

  this->ScheduleUpdateInfoLabel();
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::SetNewColorAsHSV(double hsv[3])
{ 
  this->SetNewColorAsHSV(hsv[0], hsv[1], hsv[2]); 
}

//----------------------------------------------------------------------------
double* vtkKWColorPickerWidget::GetNewColorAsHSV()
{
  return this->GetInternalNewColorAsHSV();
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::GetNewColorAsHSV(
  double &_arg1, double &_arg2, double &_arg3)
{
  this->GetInternalNewColorAsHSV(_arg1, _arg2, _arg3);
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::GetNewColorAsHSV(double _arg[3])
{ 
  this->GetNewColorAsHSV(_arg[0], _arg[1], _arg[2]); 
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::SetInternalCurrentColorAsRGB(double rgb[3])
{
  this->SetInternalCurrentColorAsRGB(rgb[0], rgb[1], rgb[2]);
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::SetInternalCurrentColorAsRGB(
  double r, double g, double b)
{
  double range[2] = {0.0, 1.0};
  vtkMath::ClampValue(&r, range);
  vtkMath::ClampValue(&g, range);
  vtkMath::ClampValue(&b, range);

  if (r == this->InternalCurrentColorAsRGB[0] &&
      g == this->InternalCurrentColorAsRGB[1] &&
      b == this->InternalCurrentColorAsRGB[2])
    {
    return;
    }

  this->InternalCurrentColorAsRGB[0] = r;
  this->InternalCurrentColorAsRGB[1] = g;
  this->InternalCurrentColorAsRGB[2] = b;

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::SetCurrentColorAsRGB(double r, double g, double b)
{
  double range[2] = {0.0, 1.0};
  vtkMath::ClampValue(&r, range);
  vtkMath::ClampValue(&g, range);
  vtkMath::ClampValue(&b, range);

  double current_rgb[3];
  this->GetCurrentColorAsRGB(current_rgb);
  if (r == current_rgb[0] && g == current_rgb[1] && b == current_rgb[2])
    {
    return;
    }

  this->Modified();

  this->SetInternalCurrentColorAsRGB(r, g, b);

  double h, s, v;
  vtkMath::RGBToHSV(r, g, b, &h, &s, &v);
  this->SetInternalCurrentColorAsHSV(h, s, v);

  // Update UI (with RGB in mind)

  if (this->ColorsLabelSet)
    {
    this->UpdateColorLabel(
      this->ColorsLabelSet->GetWidget(1), this->InternalCurrentColorAsRGB);
    }

  this->ScheduleUpdateInfoLabel();
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::SetCurrentColorAsRGB(double rgb[3])
{
  this->SetCurrentColorAsRGB(rgb[0], rgb[1], rgb[2]);
}

//----------------------------------------------------------------------------
double* vtkKWColorPickerWidget::GetCurrentColorAsRGB()
{
  return this->GetInternalCurrentColorAsRGB();
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::GetCurrentColorAsRGB(
  double &_arg1, double &_arg2, double &_arg3)
{
  this->GetInternalCurrentColorAsRGB(_arg1, _arg2, _arg3);
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::GetCurrentColorAsRGB(double _arg[3])
{
  this->GetCurrentColorAsRGB(_arg[0], _arg[1], _arg[2]);
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::SetInternalCurrentColorAsHSV(double hsv[3])
{ 
  this->SetInternalCurrentColorAsHSV(hsv[0], hsv[1], hsv[2]); 
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::SetInternalCurrentColorAsHSV(
  double h, double s, double v)
{
  double range[2] = {0.0, 1.0};
  vtkMath::ClampValue(&h, range);
  vtkMath::ClampValue(&s, range);
  vtkMath::ClampValue(&v, range);

  if (h == this->InternalCurrentColorAsHSV[0] &&
      s == this->InternalCurrentColorAsHSV[1] &&
      v == this->InternalCurrentColorAsHSV[2])
    {
    return;
    }

  this->InternalCurrentColorAsHSV[0] = h;
  this->InternalCurrentColorAsHSV[1] = s;
  this->InternalCurrentColorAsHSV[2] = v;

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::SetCurrentColorAsHSV(double h, double s, double v)
{
  double range[2] = {0.0, 1.0};
  vtkMath::ClampValue(&h, range);
  vtkMath::ClampValue(&s, range);
  vtkMath::ClampValue(&v, range);

  double current_hsv[3];
  this->GetCurrentColorAsHSV(current_hsv);
  if (h == current_hsv[0] && s == current_hsv[1] && v == current_hsv[2])
    {
    return;
    }

  this->Modified();

  this->SetInternalCurrentColorAsHSV(h, s, v);

  double r, g, b;
  vtkMath::HSVToRGB(h, s, v, &r, &g, &b);
  this->SetInternalCurrentColorAsRGB(r, g, b);

  // Update UI (with HSV in mind)

  if (this->ColorsLabelSet)
    {
    this->UpdateColorLabel(
      this->ColorsLabelSet->GetWidget(1), this->InternalCurrentColorAsRGB);
    }

  this->ScheduleUpdateInfoLabel();
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::SetCurrentColorAsHSV(double hsv[3])
{ 
  this->SetCurrentColorAsHSV(hsv[0], hsv[1], hsv[2]); 
}

//----------------------------------------------------------------------------
double* vtkKWColorPickerWidget::GetCurrentColorAsHSV()
{
  return this->GetInternalCurrentColorAsHSV();
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::GetCurrentColorAsHSV(
  double &_arg1, double &_arg2, double &_arg3)
{
  this->GetInternalCurrentColorAsHSV(_arg1, _arg2, _arg3);
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::GetCurrentColorAsHSV(double _arg[3])
{ 
  this->GetCurrentColorAsHSV(_arg[0], _arg[1], _arg[2]); 
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::UpdateSlidersRGB(double rgb[3])
{
  int i;

  // RGB sliders

  for (i = 0; i < 3; i++)
    {
    if (this->RGBSliders[i])
      {
      vtkColorTransferFunction *sliderfunc = 
        this->RGBSliders[i]->GetColorTransferFunction();
      int was_created = 0;
      if (!sliderfunc)
        {
        sliderfunc = vtkColorTransferFunction::New();
        was_created = 1;
        }
      sliderfunc->RemoveAllPoints();
      sliderfunc->AddRGBPoint(rgb[i], 1.0, 1.0, 1.0);
      this->RGBSliders[i]->SetColorTransferFunction(sliderfunc);
      this->RGBSliders[i]->SelectFirstPoint();
      if (was_created)
        {
        sliderfunc->Delete();
        }

      vtkColorTransferFunction *rampfunc = 
        this->RGBSliders[i]->GetColorRampTransferFunction();
      was_created = 0;
      if (!rampfunc)
        {
        rampfunc = vtkColorTransferFunction::New();
        was_created = 1;
        }
      rampfunc->RemoveAllPoints();
      rampfunc->AddRGBSegment(
        0.0, 
        (i == 0 ? 0.0 : rgb[0]), 
        (i == 1 ? 0.0 : rgb[1]), 
        (i == 2 ? 0.0 : rgb[2]), 
        1.0, 
        (i == 0 ? 1.0 : rgb[0]), 
        (i == 1 ? 1.0 : rgb[1]), 
        (i == 2 ? 1.0 : rgb[2])
        );
      this->RGBSliders[i]->SetColorRampTransferFunction(rampfunc);
      if (was_created)
        {
        rampfunc->Delete();
        }

      this->RGBSliders[i]->SetWholeParameterRange(0.0, 1.0);
      this->RGBSliders[i]->SetVisibleParameterRange(
        this->RGBSliders[i]->GetWholeParameterRange());
      this->RGBSliders[i]->SetDisplayedWholeParameterRange(0.0, 255.0);
      this->RGBSliders[i]->Update();
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::NewColorChanging()
{
  this->InvokeEvent(
    vtkKWColorPickerWidget::NewColorChangingEvent, this->EventCallData);
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::NewColorChanged()
{
  this->InvokeEvent(
    vtkKWColorPickerWidget::NewColorChangedEvent, this->EventCallData);
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::RGBSlidersChangingCallback()
{
  double new_color_rgb[3];
  int i;
  for (i = 0; i < 3; i++)
    {
    if (this->RGBSliders[i])
      {
      vtkColorTransferFunction *sliderfunc = 
        this->RGBSliders[i]->GetColorTransferFunction();
      if (sliderfunc)
        {
        new_color_rgb[i] = *sliderfunc->GetDataPointer();
        }
      }
    }

  if (new_color_rgb[0] == this->InternalNewColorAsRGB[0] && 
      new_color_rgb[1] == this->InternalNewColorAsRGB[1] && 
      new_color_rgb[2] == this->InternalNewColorAsRGB[2])
    {
    return;
    }

  this->SetInternalNewColorAsRGB(new_color_rgb);

  double new_color_hsv[3];
  vtkMath::RGBToHSV(new_color_rgb, new_color_hsv);
  this->SetInternalNewColorAsHSV(new_color_hsv);

  // Update UI (with RGB in mind)

  if (this->ColorsLabelSet)
    {
    this->UpdateColorLabel(
      this->ColorsLabelSet->GetWidget(0), this->InternalNewColorAsRGB);
    }

  if (this->ColorSpectrumWidget)
    {
    this->ColorSpectrumWidget->SetColorAsRGB(this->InternalNewColorAsRGB);
    }

  this->UpdateHexadecimalColorEntry(this->InternalNewColorAsRGB);

  this->UpdateSlidersHSV(this->InternalNewColorAsHSV);

  this->ScheduleUpdateInfoLabel();

  this->NewColorChanging();
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::UpdateSlidersHSV(double hsv[3])
{
  int i;

  // HSV sliders

  for (i = 0; i < 3; i++)
    {
    if (this->HSVSliders[i])
      {
      vtkColorTransferFunction *sliderfunc = 
        this->HSVSliders[i]->GetColorTransferFunction();
      int was_created = 0;
      if (!sliderfunc)
        {
        sliderfunc = vtkColorTransferFunction::New();
        was_created = 1;
        }
      sliderfunc->RemoveAllPoints();
      sliderfunc->AddHSVPoint(hsv[i], 1.0, 1.0, 1.0);
      this->HSVSliders[i]->SetColorTransferFunction(sliderfunc);
      this->HSVSliders[i]->SelectFirstPoint();
      if (was_created)
        {
        sliderfunc->Delete();
        }

      vtkColorTransferFunction *rampfunc = 
        this->HSVSliders[i]->GetColorRampTransferFunction();
      was_created = 0;
      if (!rampfunc)
        {
        rampfunc = vtkColorTransferFunction::New();
        rampfunc->SetColorSpaceToHSV();
        was_created = 1;
        }
      rampfunc->RemoveAllPoints();
      if (i == 0)
        {
        rampfunc->AddHSVPoint(0.0, 0.0, 1.0, 1.0);
        rampfunc->AddHSVPoint(0.5, 0.5, 1.0, 1.0);
        rampfunc->AddHSVPoint(1.0, 0.999, 1.0, 1.0);
        }
      else
        {
        rampfunc->AddHSVSegment(
          0.0, 
          (i == 0 ? 0.0 : hsv[0]), 
          (i == 1 ? 0.0 : hsv[1]), 
          (i == 2 ? 0.0 : hsv[2]), 
          1.0, 
          (i == 0 ? 1.0 : hsv[0]), 
          (i == 1 ? 1.0 : hsv[1]), 
          (i == 2 ? 1.0 : hsv[2])
          );
        }
      this->HSVSliders[i]->SetColorRampTransferFunction(rampfunc);
      if (was_created)
        {
        rampfunc->Delete();
        }

      this->HSVSliders[i]->SetWholeParameterRange(0.0, 1.0);
      this->HSVSliders[i]->SetVisibleParameterRange(
        this->HSVSliders[i]->GetWholeParameterRange());
      this->HSVSliders[i]->SetDisplayedWholeParameterRange(
        0.0, (i == 0 ? 360.0 : 100.0));
      this->HSVSliders[i]->Update();
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::HSVSlidersChangingCallback()
{
  double new_color_hsv[3];
  int i;
  for (i = 0; i < 3; i++)
    {
    if (this->HSVSliders[i])
      {
      vtkColorTransferFunction *sliderfunc = 
        this->HSVSliders[i]->GetColorTransferFunction();
      if (sliderfunc)
        {
        new_color_hsv[i] = *sliderfunc->GetDataPointer();
        }
      }
    }

  if (new_color_hsv[0] == this->InternalNewColorAsHSV[0] && 
      new_color_hsv[1] == this->InternalNewColorAsHSV[1] && 
      new_color_hsv[2] == this->InternalNewColorAsHSV[2])
    {
    return;
    }

  this->SetInternalNewColorAsHSV(new_color_hsv);

  double new_color_rgb[3];
  vtkMath::HSVToRGB(new_color_hsv, new_color_rgb);
  this->SetInternalNewColorAsRGB(new_color_rgb);

  // Update UI (with HSV in mind)

  if (this->ColorsLabelSet)
    {
    this->UpdateColorLabel(
      this->ColorsLabelSet->GetWidget(0), this->InternalNewColorAsRGB);
    }

  this->UpdateSlidersRGB(this->InternalNewColorAsRGB);

  this->UpdateHexadecimalColorEntry(this->InternalNewColorAsRGB);

  if (this->ColorSpectrumWidget)
    {
    this->ColorSpectrumWidget->SetColorAsHSV(this->InternalNewColorAsHSV);
    }

  this->ScheduleUpdateInfoLabel();

  this->NewColorChanging();
}

//---------------------------------------------------------------------------
void vtkKWColorPickerWidget::UpdateColorLabel(vtkKWLabel *label, double rgb[3])
{
  if (label)
    {
    label->SetBackgroundColor(rgb);

    double hsv[3];
    vtkMath::RGBToHSV(rgb, hsv);

    static char buffer[128];
    sprintf(
      buffer, 
      "#%02x%02x%02x\nRGB: [%1.2f, %1.2f, %1.2f]\nHSV: [%1.2f, %1.2f, %1.2f]",
      vtkMath::Round(rgb[0] * 255.0), 
      vtkMath::Round(rgb[1] * 255.0), 
      vtkMath::Round(rgb[2] * 255.0),
      rgb[0], rgb[1], rgb[2],
      hsv[0], hsv[1], hsv[2]);
    label->SetBalloonHelpString(buffer);
    }
}

//---------------------------------------------------------------------------
void vtkKWColorPickerWidget::UpdateHexadecimalColorEntry(double rgb[3])
{
  if (this->HexadecimalColorEntry)
    {
    this->HexadecimalColorEntry->GetWidget()->SetHexadecimalValueAsRGB(
      vtkMath::Round(rgb[0] * 255.0), 
      vtkMath::Round(rgb[1] * 255.0), 
      vtkMath::Round(rgb[2] * 255.0));
    }
}

//---------------------------------------------------------------------------
void vtkKWColorPickerWidget::UpdateInfoLabel()
{
  if (!this->InfoLabel || !this->InfoLabel->IsCreated())
    {
    return;
    }

  if (this->ColorSpectrumWidget &&
      this->ColorSpectrumWidget->IsMapped() && 
      this->ColorSpectrumWidget->GetFixedAxis() == 
      vtkKWColorSpectrumWidget::FixedAxisV &&
      this->InternalNewColorAsHSV[2] == 0.0)
    {
    this->InfoLabel->LabelVisibilityOn();
    this->InfoLabel->GetWidget()->SetText(
      "The spectrum widget on the right will appear black when interpolating "
      "Hue (H) and Saturation (S) over Value (V) = 0."
      );
    return;
    }
  
  this->InfoLabel->GetWidget()->SetText(NULL);
  this->InfoLabel->LabelVisibilityOff();
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::ScheduleUpdateInfoLabel()
{
  // Already scheduled

  if (this->Internals->ScheduleUpdateInfoLabelTimerId.size() ||
      !this->IsCreated())
    {
    return;
    }

  this->Internals->ScheduleUpdateInfoLabelTimerId =
    this->Script(
      "after 500 {catch {%s UpdateInfoLabelCallback}}", this->GetTclName());
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::UpdateInfoLabelCallback()
{
  if (!this->GetApplication() || 
      this->GetApplication()->GetInExit() ||
      !this->IsAlive())
    {
    return;
    }

  this->UpdateInfoLabel();
  this->Internals->ScheduleUpdateInfoLabelTimerId = "";
}

//---------------------------------------------------------------------------
void vtkKWColorPickerWidget::HexadecimalColorEntryCallback(const char*)
{
  if (this->HexadecimalColorEntry)
    {
    int r, g, b;
    this->HexadecimalColorEntry->GetWidget()->GetHexadecimalValueAsRGB(
      r, g, b);
    if (r >= 0 && g >= 0 && b >= 0)
      {
      this->SetNewColorAsRGB(
        (double)r / 255.0, (double)g / 255.0, (double)b / 255.0);
      }
    }

}

//---------------------------------------------------------------------------
void vtkKWColorPickerWidget::FavoritesColorPresetApplyCallback(int id)
{
  if (this->FavoritesColorPresetSelector &&
      this->FavoritesColorPresetSelector->HasPreset(id))
    {
    double r, g, b, old_r, old_g, old_b, new_r, new_g, new_b;
    this->FavoritesColorPresetSelector->GetPresetColorAsRGB(id, r, g, b);
    this->GetNewColorAsRGB(old_r, old_g, old_b);
    this->SetNewColorAsRGB(r, g, b);
    this->GetNewColorAsRGB(new_r, new_g, new_b);
    if (old_r != new_r || old_g != new_g || old_b != new_b)
      {
      this->NewColorChanged();
      }
    }
}

//---------------------------------------------------------------------------
int vtkKWColorPickerWidget::FavoritesColorPresetAddCallback()
{
  if (!this->FavoritesColorPresetSelector)
    {
    return -1;
    }

  int id = this->FavoritesColorPresetSelector->InsertPreset(
    this->FavoritesColorPresetSelector->GetIdOfNthPreset(0));

  this->FavoritesColorPresetUpdateCallback(id);

  this->FavoritesColorPresetSelector->SelectPreset(id);

  return id;
}

//---------------------------------------------------------------------------
void vtkKWColorPickerWidget::FavoritesColorPresetUpdateCallback(int id)
{
  if (!this->FavoritesColorPresetSelector)
    {
    return;
    }

  double r, g, b;
  this->GetNewColorAsRGB(r, g, b);
  this->FavoritesColorPresetSelector->SetPresetColorAsRGB(id, r, g, b);

  // If the comment was already in RGB hexadecimal form, or never set, 
  // update it

  if (!this->FavoritesColorPresetSelector->GetPresetComment(id) ||
      this->FavoritesColorPresetSelector->HasPresetCommentAsHexadecimalRGB(id))
    {
    this->FavoritesColorPresetSelector->SetPresetCommentAsHexadecimalRGB(id);
    }
}

//---------------------------------------------------------------------------
void vtkKWColorPickerWidget::HistoryColorPresetApplyCallback(int id)
{
  if (this->HistoryColorPresetSelector &&
      this->HistoryColorPresetSelector->HasPreset(id))
    {
    double r, g, b, old_r, old_g, old_b, new_r, new_g, new_b;
    this->HistoryColorPresetSelector->GetPresetColorAsRGB(id, r, g, b);
    this->GetNewColorAsRGB(old_r, old_g, old_b);
    this->SetNewColorAsRGB(r, g, b);
    this->GetNewColorAsRGB(new_r, new_g, new_b);
    if (old_r != new_r || old_g != new_g || old_b != new_b)
      {
      this->NewColorChanged();
      }
    }
}

//---------------------------------------------------------------------------
void vtkKWColorPickerWidget::SwatchSelectedCallback(
  double r, double g, double b)
{
  double old_r, old_g, old_b, new_r, new_g, new_b;
  this->GetNewColorAsRGB(old_r, old_g, old_b);
  this->SetNewColorAsRGB(r, g, b);
  this->GetNewColorAsRGB(new_r, new_g, new_b);
  if (old_r != new_r || old_g != new_g || old_b != new_b)
    {
    this->NewColorChanged();
    }
}

//---------------------------------------------------------------------------
void vtkKWColorPickerWidget::CurrentColorCallback()
{
  if (!this->ColorsLabelSet)
    {
    return;
    }

  double r, g, b, old_r, old_g, old_b, new_r, new_g, new_b;
  this->ColorsLabelSet->GetWidget(1)->GetBackgroundColor(&r, &g, &b);
  this->GetNewColorAsRGB(old_r, old_g, old_b);
  this->SetNewColorAsRGB(r, g, b);
  this->GetNewColorAsRGB(new_r, new_g, new_b);
  if (old_r != new_r || old_g != new_g || old_b != new_b)
    {
    this->NewColorChanged();
    }
}

//---------------------------------------------------------------------------
void vtkKWColorPickerWidget::ColorSpectrumChangingCallback()
{
  if (!this->ColorSpectrumWidget)
    {
    return;
    }

  double r, g, b;
  double h, s, v;

  switch (this->ColorSpectrumWidget->GetFixedAxis())
    {
    case vtkKWColorSpectrumWidget::FixedAxisR:
    case vtkKWColorSpectrumWidget::FixedAxisG: 
    case vtkKWColorSpectrumWidget::FixedAxisB:
      
      this->ColorSpectrumWidget->GetColorAsRGB(r, g, b);
      if (r == this->InternalNewColorAsRGB[0] && 
          g == this->InternalNewColorAsRGB[1] && 
          b == this->InternalNewColorAsRGB[2])
        {
        return;
        }

      this->SetInternalNewColorAsRGB(r, g, b);
      
      vtkMath::RGBToHSV(r, g, b, &h, &s, &v);
      this->SetInternalNewColorAsHSV(h, s, v);

      // Update UI (with RGB in mind)
      
      if (this->ColorsLabelSet)
        {
        this->UpdateColorLabel(
          this->ColorsLabelSet->GetWidget(0), this->InternalNewColorAsRGB);
        }
      
      this->UpdateSlidersRGB(this->InternalNewColorAsRGB);
      
      this->UpdateHexadecimalColorEntry(this->InternalNewColorAsRGB);
      
      this->UpdateSlidersHSV(this->InternalNewColorAsHSV);

      this->ScheduleUpdateInfoLabel();

      this->NewColorChanging();

      break;

    case vtkKWColorSpectrumWidget::FixedAxisH:
    case vtkKWColorSpectrumWidget::FixedAxisS:
    case vtkKWColorSpectrumWidget::FixedAxisV:

      this->ColorSpectrumWidget->GetColorAsHSV(h, s, v);
      if (h == this->InternalNewColorAsHSV[0] && 
          s == this->InternalNewColorAsHSV[1] && 
          v == this->InternalNewColorAsHSV[2])
        {
        return;
        }

      this->SetInternalNewColorAsHSV(h, s, v);

      vtkMath::HSVToRGB(h, s, v, &r, &g, &b);
      this->SetInternalNewColorAsRGB(r, g, b);

      // Update UI (with HSV in mind)

      if (this->ColorsLabelSet)
        {
        this->UpdateColorLabel(
          this->ColorsLabelSet->GetWidget(0), this->InternalNewColorAsRGB);
        }

      this->UpdateSlidersRGB(this->InternalNewColorAsRGB);

      this->UpdateHexadecimalColorEntry(this->InternalNewColorAsRGB);

      this->UpdateSlidersHSV(this->InternalNewColorAsHSV);

      this->ScheduleUpdateInfoLabel();

      this->NewColorChanging();

      break;
    }
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::SetColorSpectrumVisibility(int arg)
{
  if (this->ColorSpectrumVisibility == arg)
    {
    return;
    }

  this->ColorSpectrumVisibility = arg;

  this->Modified();

  if (this->Notebook)
    {
    this->Notebook->SetPageVisibility(
      NULL, VTK_KW_COLOR_PICKER_WIDGET_SPECTRUM_TAG, 
      this->ColorSpectrumVisibility);
    }

  this->Pack();
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::SetColorSwatchesVisibility(int arg)
{
  if (this->ColorSwatchesVisibility == arg)
    {
    return;
    }

  this->ColorSwatchesVisibility = arg;

  this->Modified();

  if (this->Notebook)
    {
    this->Notebook->SetPageVisibility(
      NULL, VTK_KW_COLOR_PICKER_WIDGET_COLOR_SWATCHES_TAG, 
      this->ColorSwatchesVisibility);
    }

  this->Pack();
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::SetFavoritesVisibility(int arg)
{
  if (this->FavoritesVisibility == arg)
    {
    return;
    }

  this->FavoritesVisibility = arg;

  this->Modified();

  if (this->Notebook)
    {
    this->Notebook->SetPageVisibility(
      NULL, VTK_KW_COLOR_PICKER_WIDGET_FAVORITES_TAG, 
      this->FavoritesVisibility);
    }

  this->Pack();
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::SetHistoryVisibility(int arg)
{
  if (this->HistoryVisibility == arg)
    {
    return;
    }

  this->HistoryVisibility = arg;

  this->Modified();

  if (this->Notebook)
    {
    this->Notebook->SetPageVisibility(
      NULL, VTK_KW_COLOR_PICKER_WIDGET_HISTORY_TAG, 
      this->HistoryVisibility);
    }

  this->Pack();
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::SetCompactMode(int arg)
{
  if (this->CompactMode == arg)
    {
    return;
    }

  this->CompactMode = arg;

  this->Modified();

  this->InvokeEvent(
    vtkKWColorPickerWidget::CompactModeChangedEvent, this->EventCallData);

  this->AdjustToCompactMode();
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::ToggleCompactMode()
{
  this->SetCompactMode(this->CompactMode ? 0 : 1);
}

//----------------------------------------------------------------------------
vtkKWColorPresetSelector* 
vtkKWColorPickerWidget::GetFavoritesColorPresetSelector()
{
  return this->FavoritesColorPresetSelector;
}

//----------------------------------------------------------------------------
vtkKWColorPresetSelector* 
vtkKWColorPickerWidget::GetHistoryColorPresetSelector()
{
  return this->HistoryColorPresetSelector;
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::ProcessCallbackCommandEvents(vtkObject *caller,
                                                          unsigned long event,
                                                          void *calldata)
{
  const char **cargs = (const char**)calldata;
  int tag;

  if (caller == this->Notebook)
    {
    switch (event)
      {
      case vtkKWEvent::NotebookRaisePageEvent:
        tag = (atoi)(cargs[1]);
        if (tag == VTK_KW_COLOR_PICKER_WIDGET_FAVORITES_TAG)
          {
          this->CreateFavoritesColorPresetSelector();
          }
        else if (tag == VTK_KW_COLOR_PICKER_WIDGET_HISTORY_TAG)
          {
          this->CreateHistoryColorPresetSelector();
          }
        this->ScheduleUpdateInfoLabel();
        break;
      }
    }

  if (caller == this->ColorSpectrumWidget)
    {
    switch (event)
      {
      case vtkKWColorSpectrumWidget::ColorChangingEvent:
        this->ColorSpectrumChangingCallback();
        break;

      case vtkKWColorSpectrumWidget::ColorChangedEvent:
        this->NewColorChanged();
        break;
      }
    }

  if (caller == this->RGBSliders[0] ||
      caller == this->RGBSliders[1] ||
      caller == this->RGBSliders[2])
    {
    switch (event)
      {
      case vtkKWParameterValueFunctionEditor::FunctionChangingEvent:
        this->RGBSlidersChangingCallback();
        break;

      case vtkKWParameterValueFunctionEditor::FunctionChangedEvent:
        this->NewColorChanged();
        break;
      }
    }

  if (caller == this->HSVSliders[0] ||
      caller == this->HSVSliders[1] ||
      caller == this->HSVSliders[2])
    {
    switch (event)
      {
      case vtkKWParameterValueFunctionEditor::FunctionChangingEvent:
        this->HSVSlidersChangingCallback();
        break;

      case vtkKWParameterValueFunctionEditor::FunctionChangedEvent:
        this->NewColorChanged();
        break;
      }
    }

  this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->PropagateEnableState(this->SlidersFrame);

  this->PropagateEnableState(this->RGBSlidersFrame);

  this->PropagateEnableState(this->HSVSlidersFrame);

  int i;
  for (i = 0; i < 3; i++)
    {
    this->PropagateEnableState(this->RGBSliders[i]);
    this->PropagateEnableState(this->HSVSliders[i]);
    }

  for (i = 0; i < 6; i++)
    {
    this->PropagateEnableState(this->FixedAxisRadioButton[i]);
    this->PropagateEnableState(this->SliderUnitLabel[i]);
    }

  this->PropagateEnableState(this->Notebook);

  this->PropagateEnableState(this->HexadecimalColorEntry);
  this->PropagateEnableState(this->ColorSpectrumWidget);
  this->PropagateEnableState(this->ColorSwatchesWidget);
  this->PropagateEnableState(this->FavoritesColorPresetSelector);
  this->PropagateEnableState(this->HistoryColorPresetSelector);

  this->PropagateEnableState(this->ColorsFrame);
  this->PropagateEnableState(this->ColorsLabelSet);
  this->PropagateEnableState(this->ColorsNameLabelSet);
  this->PropagateEnableState(this->InfoLabel);
  this->PropagateEnableState(this->CompactModeButton);
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  os << indent << "ColorSpectrumVisibility: "
     << (this->ColorSpectrumVisibility ? "On" : "Off") << endl;

  os << indent << "ColorSwatchesVisibility: "
     << (this->ColorSwatchesVisibility ? "On" : "Off") << endl;

  os << indent << "FavoritesVisibility: "
     << (this->FavoritesVisibility ? "On" : "Off") << endl;

  os << indent << "HistoryVisibility: "
     << (this->HistoryVisibility ? "On" : "Off") << endl;

  this->Superclass::PrintSelf(os,indent);
}
