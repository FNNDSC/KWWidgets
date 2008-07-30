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
#include "vtkKWColorSpectrumWidget.h"
#include "vtkKWColorPresetSelector.h"
#include "vtkKWColorTransferFunctionEditor.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWEvent.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameSet.h"
#include "vtkKWIcon.h"
#include "vtkKWLabel.h"
#include "vtkKWLabelSet.h"
#include "vtkKWNotebook.h"
#include "vtkKWRadioButton.h"

#define VTK_KW_COLOR_PICKER_WIDGET_SPECTRUM_TAG 0
#define VTK_KW_COLOR_PICKER_WIDGET_BASIC_COLORS_TAG 1
#define VTK_KW_COLOR_PICKER_WIDGET_FAVORITES_TAG 2
#define VTK_KW_COLOR_PICKER_WIDGET_HISTORY_TAG 3

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWColorPickerWidget );
vtkCxxRevisionMacro(vtkKWColorPickerWidget, "$Revision: 1.6 $");

//----------------------------------------------------------------------------
vtkKWColorPickerWidget::vtkKWColorPickerWidget()
{
  this->ColorSpectrumVisibility = 1;
  this->BasicColorsVisibility = 1;
  this->FavoritesVisibility = 1;
  this->HistoryVisibility = 1;

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
  this->ColorSpectrumWidget      = NULL;
  this->FavoritesColorPresetSelector = vtkKWColorPresetSelector::New();
  this->HistoryColorPresetSelector   = vtkKWColorPresetSelector::New();
  this->BasicColorsFrameSet   = NULL;
  this->ColorsFrame                  = NULL;
  this->ColorsLabelSet               = NULL;
  this->ColorsNameLabelSet           = NULL;

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

  if (this->BasicColorsFrameSet)
    {
    this->BasicColorsFrameSet->Delete();
    this->BasicColorsFrameSet = NULL;
    }
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

  ostrstream tk_cmd;
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
  this->Notebook->SetMinimumWidth(268);
  this->Notebook->SetMinimumHeight(294);
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
  // Basic Colors

  icon->SetImage(vtkKWIcon::IconColorSquares);

  page_id = this->Notebook->AddPage(
    NULL, "Basic Colors", icon, VTK_KW_COLOR_PICKER_WIDGET_BASIC_COLORS_TAG);
  this->Notebook->SetPageVisibility(page_id, this->BasicColorsVisibility);

  this->CreateBasicColorsFrameSet();

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

  tk_cmd << "pack " << this->ColorsNameLabelSet->GetWidgetName() 
         << " -side top -anchor center -expand y -fill x  -pady 0" << endl;

  if (!this->ColorsLabelSet)
    {
    this->ColorsLabelSet = vtkKWLabelSet::New();
    }
  this->ColorsLabelSet->SetParent(this->ColorsFrame);
  this->ColorsLabelSet->Create();
  this->ColorsLabelSet->PackHorizontallyOn();
  this->ColorsLabelSet->SetBorderWidth(2);
  this->ColorsLabelSet->SetReliefToSunken();

  tk_cmd << "pack " << this->ColorsLabelSet->GetWidgetName() 
         << " -side top -anchor center -expand y -fill x  -pady 0" << endl;

  vtkKWLabel *label = NULL;
  for (i = 0; i <= 1; i++)
    {
    label = this->ColorsNameLabelSet->AddWidget(i);

    label = this->ColorsLabelSet->AddWidget(i);
    label->SetBorderWidth(0);
    label->SetWidth(10);
    label->SetHeight(3);
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

  tk_cmd << "pack " << this->HexadecimalColorEntry->GetWidgetName() 
         << " -side top -anchor nw -expand n -fill none -padx 0 -pady 5"
         << endl;

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

  tk_cmd << "pack " << this->ColorsFrame->GetWidgetName() 
         << " -side top -anchor center -expand n -fill none -padx 0 -pady 0" 
         << endl;

  tk_cmd << "pack " << this->RGBSlidersFrame->GetWidgetName() 
         << " -side top -anchor sw -expand n -fill both -padx 0 -pady 20" 
         << endl;

  tk_cmd << "pack " << this->HSVSlidersFrame->GetWidgetName() 
         << " -side top -anchor sw -expand n -fill both -padx 0 -pady 0" 
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
    this->RGBSliders[i]->SetCanvasWidth(256);
    this->RGBSliders[i]->SetCanvasHeight(18);
    this->RGBSliders[i]->SetColorRampHeight(14);
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
    this->HSVSliders[i]->SetCanvasWidth(this->RGBSliders[i]->GetCanvasWidth());
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
    this->HSVSliders[i]->SetCanvasHeight(
      this->RGBSliders[i]->GetCanvasHeight());
    this->HSVSliders[i]->SetColorRampHeight(
      this->RGBSliders[i]->GetColorRampHeight());
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

  tk_cmd << ends;
  this->Script(tk_cmd.str());
  tk_cmd.rdbuf()->freeze(0);

  // Update

  this->ColorSpectrumWidget->SetColorAsHSV(this->InternalNewColorAsHSV);

  this->UpdateSlidersRGB(this->InternalNewColorAsRGB);
  this->UpdateSlidersHSV(this->InternalNewColorAsHSV);

  this->UpdateColorLabel(
    this->ColorsLabelSet->GetWidget(0), this->InternalNewColorAsRGB);

  this->UpdateColorLabel(
    this->ColorsLabelSet->GetWidget(1), this->InternalCurrentColorAsRGB);

  this->UpdateHexadecimalColorEntry(this->InternalNewColorAsRGB);

  this->Pack();
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::CreateBasicColorsFrameSet()
{
  if (!this->IsCreated())
    {
    return;
    }

  if (!this->BasicColorsFrameSet)
    {
    this->BasicColorsFrameSet = vtkKWFrameSet::New();
    }

  if (this->BasicColorsFrameSet->IsCreated())
    {
    return;
    }

  this->BasicColorsFrameSet->SetParent(
    this->Notebook->GetFrame(NULL, 
                             VTK_KW_COLOR_PICKER_WIDGET_BASIC_COLORS_TAG));
  this->BasicColorsFrameSet->Create();
  this->BasicColorsFrameSet->PackHorizontallyOn();
  this->BasicColorsFrameSet->SetMaximumNumberOfWidgetsInPackingDirection(13);
  this->BasicColorsFrameSet->SetWidgetsPadX(2);
  this->BasicColorsFrameSet->SetWidgetsPadY(2);

  const char *colors[] = 
    {
      "#ff8080", "#ffff80", "#80ff80", "#00ff80", 
      "#80ffff", "#0080ff", "#ff80c0", "#ff80ff",
      "#ff0000", "#ffff00", "#80ff00", "#00ff40", 
      "#00ffff", "#0080c0", "#8080c0", "#ff00ff",
      "#804040", "#ff8040", "#00ff00", "#008080", 
      "#004080", "#8080ff", "#800040", "#ff0080",
      "#800000", "#ff8000", "#008000", "#008040", 
      "#0000ff", "#0000a0", "#800080", "#8000ff",
      "#400000", "#804000", "#004000", "#004040", 
      "#000080", "#000040", "#400040", "#400080",
      "#000000", "#808000", "#808040", "#808080", 
      "#408080", "#c0c0c0", "#400040", "#ffffff"
    };

  int i;
  char command[128];

  for (i = 0; i < sizeof(colors) / sizeof(colors[0]); i++)
    {
    vtkKWFrame *frame = this->BasicColorsFrameSet->AddWidget(i);
    // for speed, use Script
    this->Script(
      "%s configure -bd 1 -relief solid -width 16 -height 16 -bg %s", 
      frame->GetWidgetName(), colors[i]);
    sprintf(command, "BasicColorsCallback {%s}", colors[i]);
    frame->SetBinding("<Any-ButtonPress>", this, command);
    }

  this->Script("pack %s -side left -anchor nw -expand n -fill none",
               this->BasicColorsFrameSet->GetWidgetName());
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

  ostrstream tk_cmd;

  tk_cmd << "pack " << this->SlidersFrame->GetWidgetName();

  if (this->ColorSpectrumVisibility ||
      this->BasicColorsVisibility ||
      this->FavoritesVisibility ||
      this->HistoryVisibility)
    {
    tk_cmd << " " << this->Notebook->GetWidgetName();
    }
  
  tk_cmd << " -padx 2 -pady 2 -side left -anchor nw -expand n -fill both" 
         << endl;

  tk_cmd << ends;
  this->Script(tk_cmd.str());
  tk_cmd.rdbuf()->freeze(0);
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

  this->InvokeEvent(vtkKWColorPickerWidget::NewColorChangingEvent);
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

  this->InvokeEvent(vtkKWColorPickerWidget::NewColorChangingEvent);
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
      this->InvokeEvent(vtkKWColorPickerWidget::NewColorChangedEvent);
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
      this->InvokeEvent(vtkKWColorPickerWidget::NewColorChangedEvent);
      }
    }
}

//---------------------------------------------------------------------------
void vtkKWColorPickerWidget::BasicColorsCallback(const char *color)
{
  int r, g, b;
  if (color && sscanf(color, "#%02x%02x%02x", &r, &g, &b) == 3)
    {
    double old_r, old_g, old_b, new_r, new_g, new_b;
    this->GetNewColorAsRGB(old_r, old_g, old_b);
    this->SetNewColorAsRGB(
      (double)r / 255.0, (double)g / 255.0,(double)b / 255.0);
    this->GetNewColorAsRGB(new_r, new_g, new_b);
    if (old_r != new_r || old_g != new_g || old_b != new_b)
      {
      this->InvokeEvent(vtkKWColorPickerWidget::NewColorChangedEvent);
      }
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
    this->InvokeEvent(vtkKWColorPickerWidget::NewColorChangedEvent);
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

      this->InvokeEvent(vtkKWColorPickerWidget::NewColorChangingEvent);

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

      this->InvokeEvent(vtkKWColorPickerWidget::NewColorChangingEvent);

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
void vtkKWColorPickerWidget::SetBasicColorsVisibility(int arg)
{
  if (this->BasicColorsVisibility == arg)
    {
    return;
    }

  this->BasicColorsVisibility = arg;

  this->Modified();

  if (this->Notebook)
    {
    this->Notebook->SetPageVisibility(
      NULL, VTK_KW_COLOR_PICKER_WIDGET_BASIC_COLORS_TAG, 
      this->BasicColorsVisibility);
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
        this->InvokeEvent(vtkKWColorPickerWidget::NewColorChangedEvent);
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
        this->InvokeEvent(vtkKWColorPickerWidget::NewColorChangedEvent);
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
        this->InvokeEvent(vtkKWColorPickerWidget::NewColorChangedEvent);
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
  this->PropagateEnableState(this->BasicColorsFrameSet);
  this->PropagateEnableState(this->FavoritesColorPresetSelector);
  this->PropagateEnableState(this->HistoryColorPresetSelector);

  this->PropagateEnableState(this->ColorsFrame);
  this->PropagateEnableState(this->ColorsLabelSet);
  this->PropagateEnableState(this->ColorsNameLabelSet);
}

//----------------------------------------------------------------------------
void vtkKWColorPickerWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  os << indent << "ColorSpectrumVisibility: "
     << (this->ColorSpectrumVisibility ? "On" : "Off") << endl;

  os << indent << "BasicColorsVisibility: "
     << (this->BasicColorsVisibility ? "On" : "Off") << endl;

  os << indent << "FavoritesVisibility: "
     << (this->FavoritesVisibility ? "On" : "Off") << endl;

  os << indent << "HistoryVisibility: "
     << (this->HistoryVisibility ? "On" : "Off") << endl;

  this->Superclass::PrintSelf(os,indent);
}
