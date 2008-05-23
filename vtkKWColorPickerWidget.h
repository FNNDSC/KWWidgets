/*=========================================================================

  Module:    $RCSfile: vtkKWColorPickerWidget.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWColorPickerWidget - a color picker widget
// .SECTION Description
// A widget that can be used to pick a color. It provides interactive
// RGB and HSV sliders, a color spectrum area, and various other means to
// select or enter color coordinates. It features a set of basic colors, a 
// color preset editor that can be used store a list of persistent/favorite 
// colors, and a history of "recently picked" colors.
// This widget can be embedded in any other UI. For a standalone 
// toplevel/dialog color picker, check the vtkKWColorPickerDialog class.
// .SECTION Thanks
// This work is part of the National Alliance for Medical Image
// Computing (NAMIC), funded by the National Institutes of Health
// through the NIH Roadmap for Medical Research, Grant U54 EB005149.
// Information on the National Centers for Biomedical Computing
// can be obtained from http://nihroadmap.nih.gov/bioinformatics.
// .SECTION See Also
// vtkKWColorPickerDialog vtkKWColorSpectrumWidget vtkKWColorPresetSelector

#ifndef __vtkKWColorPickerWidget_h
#define __vtkKWColorPickerWidget_h

#include "vtkKWCompositeWidget.h"

class vtkKWColorSpectrumWidget;
class vtkKWColorPresetSelector;
class vtkKWColorTransferFunctionEditor;
class vtkKWEntryWithLabel;
class vtkKWFrame;
class vtkKWFrameSet;
class vtkKWLabel;
class vtkKWLabelSet;
class vtkKWNotebook;
class vtkKWRadioButton;

class KWWidgets_EXPORT vtkKWColorPickerWidget : public vtkKWCompositeWidget
{
public:
  static vtkKWColorPickerWidget* New();
  vtkTypeRevisionMacro(vtkKWColorPickerWidget,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the current color as RGB, or HSV.
  virtual double *GetNewColorAsRGB();
  virtual void GetNewColorAsRGB(double &_arg1, double &_arg2, double &_arg3);
  virtual void GetNewColorAsRGB(double _arg[3]);
  virtual void SetNewColorAsRGB(double r, double g, double b);
  virtual void SetNewColorAsRGB(double rgb[3]);
  virtual double *GetNewColorAsHSV();
  virtual void GetNewColorAsHSV(double &_arg1, double &_arg2, double &_arg3);
  virtual void GetNewColorAsHSV(double _arg[3]);
  virtual void SetNewColorAsHSV(double r, double g, double b);
  virtual void SetNewColorAsHSV(double rgb[3]);

  // Description:
  // Access the color preset selector. Note that you can set their
  // RegistryKey ivar to save/restore the favorites and history presets
  // automatically (as done by the vtkKWColorPickerDialog class). 
  // Make sure RegistryKey is set *before* calling the Create() method.
  // Check the vtkKWColorPresetSelector for more details.
  virtual vtkKWColorPresetSelector* GetFavoritesColorPresetSelector();
  virtual vtkKWColorPresetSelector* GetHistoryColorPresetSelector();

  // Description:
  // Set/Get the color spectrum visibility.
  vtkBooleanMacro(ColorSpectrumVisibility, int);
  virtual void SetColorSpectrumVisibility(int);
  vtkGetMacro(ColorSpectrumVisibility, int);

  // Description:
  // Set/Get the basic colors visibility.
  vtkBooleanMacro(BasicColorsVisibility, int);
  virtual void SetBasicColorsVisibility(int);
  vtkGetMacro(BasicColorsVisibility, int);

  // Description:
  // Set/Get the favorites preset selector visibility.
  vtkBooleanMacro(FavoritesVisibility, int);
  virtual void SetFavoritesVisibility(int);
  vtkGetMacro(FavoritesVisibility, int);

  // Description:
  // Set/Get the history preset selector visibility.
  vtkBooleanMacro(HistoryVisibility, int);
  virtual void SetHistoryVisibility(int);
  vtkGetMacro(HistoryVisibility, int);

  // Description:
  // Callbacks
  virtual void RGBSlidersChangingCallback();
  virtual void HSVSlidersChangingCallback();
  virtual void FavoritesColorPresetApplyCallback(int id);
  virtual int  FavoritesColorPresetAddCallback();
  virtual void FavoritesColorPresetUpdateCallback(int id);
  virtual void HistoryColorPresetApplyCallback(int id);
  virtual void CurrentColorCallback();
  virtual void HexadecimalColorEntryCallback(const char*);
  virtual void ColorSpectrumChangingCallback();
  virtual void BasicColorsCallback(const char *color);

  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object is updated
  // and propagated to its internal parts/subwidgets. This will, for example,
  // enable/disable parts of the widget UI, enable/disable the visibility
  // of 3D widgets, etc.
  virtual void UpdateEnableState();

protected:
  vtkKWColorPickerWidget();
  ~vtkKWColorPickerWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Description:
  // Pack.
  virtual void Pack();

  // Description:
  // Set/Get internal colors. 
  double InternalNewColorAsRGB[3];
  vtkGetVector3Macro(InternalNewColorAsRGB, double);
  virtual void SetInternalNewColorAsRGB(double r, double g, double b);
  virtual void SetInternalNewColorAsRGB(double rgb[3]);
  double InternalNewColorAsHSV[3];
  vtkGetVector3Macro(InternalNewColorAsHSV, double);
  virtual void SetInternalNewColorAsHSV(double h, double s, double v);
  virtual void SetInternalNewColorAsHSV(double hsv[3]);

  vtkKWFrame                       *SlidersFrame;

  vtkKWFrame                       *RGBSlidersFrame;
  vtkKWColorTransferFunctionEditor *RGBSliders[3];

  vtkKWFrame                       *HSVSlidersFrame;
  vtkKWColorTransferFunctionEditor *HSVSliders[3];

  vtkKWRadioButton                  *FixedAxisRadioButton[6];
  vtkKWLabel                        *SliderUnitLabel[6];

  vtkKWNotebook *Notebook;

  vtkKWEntryWithLabel              *HexadecimalColorEntry;
  vtkKWColorSpectrumWidget         *ColorSpectrumWidget;
  vtkKWColorPresetSelector         *FavoritesColorPresetSelector;
  vtkKWColorPresetSelector         *HistoryColorPresetSelector;
  vtkKWFrameSet                    *BasicColorsFrameSet;

  vtkKWFrame                       *ColorsFrame;
  vtkKWLabelSet                    *ColorsLabelSet;
  vtkKWLabelSet                    *ColorsNameLabelSet;

  virtual void UpdateSlidersRGB(double rgb[3]);
  virtual void UpdateSlidersHSV(double hsv[3]);
  virtual void UpdateColorLabel(vtkKWLabel *label, double rgb[3]);
  virtual void UpdateHexadecimalColorEntry(double rgb[3]);

  int ColorSpectrumVisibility;
  int BasicColorsVisibility;
  int FavoritesVisibility;
  int HistoryVisibility;

  virtual void CreateFavoritesColorPresetSelector();
  virtual void CreateHistoryColorPresetSelector();
  virtual void CreateBasicColorsFrameSet();

  // Description:
  // Processes the events that are passed through CallbackCommand (or others).
  // Subclasses can oberride this method to process their own events, but
  // should call the superclass too.
  virtual void ProcessCallbackCommandEvents(
    vtkObject *caller, unsigned long event, void *calldata);
  
private:
  vtkKWColorPickerWidget(const vtkKWColorPickerWidget&); // Not implemented
  void operator=(const vtkKWColorPickerWidget&); // Not implemented
};


#endif
