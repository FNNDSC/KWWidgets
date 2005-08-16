/*=========================================================================

  Module:    $RCSfile: vtkKWVolumePropertyWidget.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWVolumePropertyWidget - a transfer function widget
// .SECTION Description
// This class contains the UI components and methods to edit a 
// ColorTransferFunction in concert with a PiecewiseFunction for opacity.
// New control points can be added by clicking with the left mouse button
// and they can be removed by dragging them out of the window.

#ifndef __vtkKWVolumePropertyWidget_h
#define __vtkKWVolumePropertyWidget_h

#include "vtkKWCompositeWidget.h"

class vtkDataSet;
class vtkKWCheckButton;
class vtkKWColorTransferFunctionEditor;
class vtkKWHSVColorSelector;
class vtkKWHistogramSet;
class vtkKWFrame;
class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWScaleWithEntrySetWithLabel;
class vtkKWMenuButton;
class vtkKWPiecewiseFunctionEditor;
class vtkKWScalarComponentSelectionWidget;
class vtkKWScaleWithEntry;
class vtkKWVolumeMaterialPropertyWidget;
class vtkVolumeProperty;

class KWWIDGETS_EXPORT vtkKWVolumePropertyWidget : public vtkKWCompositeWidget
{
public:
  static vtkKWVolumePropertyWidget* New();
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeRevisionMacro(vtkKWVolumePropertyWidget,vtkKWCompositeWidget);

  // Description:
  // Create the widget.
  virtual void Create(vtkKWApplication *app);

  // Description:
  // Get/Set the transfer function mapping scalar value to color
  vtkGetObjectMacro(VolumeProperty, vtkVolumeProperty);
  virtual void SetVolumeProperty(vtkVolumeProperty*);

  // Description:
  // The data this volume property is used for. 
  // Will be used to get the scalar range of the transfer functions for
  // example.
  vtkGetObjectMacro(DataSet, vtkDataSet);
  virtual void SetDataSet(vtkDataSet*);

  // Description:
  // Set/Get the histograms for the data this volume property is used for.
  // Obviously those histograms must have been computed using the same
  // data as the DataSet Ivar above.
  vtkGetObjectMacro(HistogramSet, vtkKWHistogramSet);
  virtual void SetHistogramSet(vtkKWHistogramSet*);

  // Description:
  // Set/get the current component controlled by the widget
  virtual void SetSelectedComponent(int);
  vtkGetMacro(SelectedComponent, int);

  // Description:
  // Set/Get the window/level for those transfer functions that support
  // this mode (SetInteractiveWindowLevel will trigger interactive events)
  // IsInWindowLevelMode return true if part of this widget is in
  // window/level mode.
  virtual void SetWindowLevel(float window, float level);
  virtual void SetInteractiveWindowLevel(float window, float level);
  virtual int IsInWindowLevelMode();

  // Description:
  // Set/Get the component selection widget visibility
  vtkBooleanMacro(ComponentSelectionVisibility, int);
  virtual void SetComponentSelectionVisibility(int);
  vtkGetMacro(ComponentSelectionVisibility, int);

  // Description:
  // Set/Get the interpolation type widget visibility
  vtkBooleanMacro(InterpolationTypeVisibility, int);
  virtual void SetInterpolationTypeVisibility(int);
  vtkGetMacro(InterpolationTypeVisibility, int);

  // Description:
  // Set/Get the material widget + enable shading visibility
  vtkBooleanMacro(MaterialPropertyVisibility, int);
  virtual void SetMaterialPropertyVisibility(int);
  vtkGetMacro(MaterialPropertyVisibility, int);

  // Description:
  // Set/Get the gradient opacity function visibility
  vtkBooleanMacro(GradientOpacityFunctionVisibility, int);
  virtual void SetGradientOpacityFunctionVisibility(int);
  vtkGetMacro(GradientOpacityFunctionVisibility, int);

  // Description:
  // Set/Get the component weight visibility
  vtkBooleanMacro(ComponentWeightsVisibility, int);
  virtual void SetComponentWeightsVisibility(int);
  vtkGetMacro(ComponentWeightsVisibility, int);

  // Description:
  // If true, an "Enable Shading" checkbox will be displayed and will
  // control the shading flag of all components at once 
  // (based on the first one). If false, the shading flag will be available
  // on a per-component basis in the shading dialog.
  vtkBooleanMacro(EnableShadingForAllComponents, int);
  virtual void SetEnableShadingForAllComponents(int);
  vtkGetMacro(EnableShadingForAllComponents, int);

  // Description:
  // Update the whole UI depending on the value of the Ivars
  virtual void Update();

  // Description:
  // Set/get whether the above commands should be called or not.
  vtkSetMacro(DisableCommands, int);
  vtkGetMacro(DisableCommands, int);
  vtkBooleanMacro(DisableCommands, int);

  // Description:
  // Set commands.
  virtual void SetVolumePropertyChangedCommand(
    vtkObject* object,const char *method);
  virtual void SetVolumePropertyChangingCommand(
    vtkObject* object,const char *method);
  virtual void InvokeVolumePropertyChangedCommand();
  virtual void InvokeVolumePropertyChangingCommand();

  // Description:
  // Callbacks
  virtual void SelectedComponentCallback(int);
  virtual void InterpolationTypeCallback(int type);
  virtual void EnableShadingCallback();
  virtual void MaterialPropertyChangedCallback();
  virtual void MaterialPropertyChangingCallback();
  virtual void ScalarOpacityFunctionChangedCallback();
  virtual void ScalarOpacityFunctionChangingCallback();
  virtual void WindowLevelModeCallback();
  virtual void LockOpacityAndColorCallback();
  virtual void ScalarOpacityUnitDistanceChangedCallback();
  virtual void ScalarOpacityUnitDistanceChangingCallback();
  virtual void RGBTransferFunctionChangedCallback();
  virtual void RGBTransferFunctionChangingCallback();
  virtual void RGBTransferFunctionSelectionChangedCallback();
  virtual void EnableGradientOpacityCallback(int val);
  virtual void GradientOpacityFunctionChangedCallback();
  virtual void GradientOpacityFunctionChangingCallback();
  virtual void HSVColorSelectionChangedCallback(double h, double s, double v);
  virtual void HSVColorSelectionChangingCallback(double h, double s, double v);
  virtual void ComponentWeightChangedCallback(int index);
  virtual void ComponentWeightChangingCallback(int index);

  // Description:
  // Access to the editors
  vtkGetObjectMacro(ScalarOpacityFunctionEditor, vtkKWPiecewiseFunctionEditor);
  vtkGetObjectMacro(ScalarColorFunctionEditor, vtkKWColorTransferFunctionEditor);
  vtkGetObjectMacro(GradientOpacityFunctionEditor, vtkKWPiecewiseFunctionEditor);
  vtkGetObjectMacro(ScalarOpacityUnitDistanceScale, vtkKWScaleWithEntry);
  vtkGetObjectMacro(HSVColorSelector, vtkKWHSVColorSelector);
 
  // Description:
  // Pack or repack the UI
  virtual void Pack();

  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object is updated
  // and propagated to its internal parts/subwidgets. This will, for example,
  // enable/disable parts of the widget UI, enable/disable the visibility
  // of 3D widgets, etc.
  virtual void UpdateEnableState();

protected:
  vtkKWVolumePropertyWidget();
  ~vtkKWVolumePropertyWidget();

  vtkVolumeProperty *VolumeProperty;
  vtkDataSet        *DataSet;
  vtkKWHistogramSet *HistogramSet;

  int   SelectedComponent;
  int   DisableCommands;
  int   EnableShadingForAllComponents;

  int   ComponentSelectionVisibility;
  int   InterpolationTypeVisibility;
  int   MaterialPropertyVisibility;
  int   GradientOpacityFunctionVisibility;
  int   ComponentWeightsVisibility;

  // Commands

  char  *VolumePropertyChangedCommand;
  char  *VolumePropertyChangingCommand;

  virtual void InvokeCommand(const char *command);
  
  // GUI

  vtkKWFrameWithLabel                 *EditorFrame;
  vtkKWHSVColorSelector               *HSVColorSelector;
  vtkKWFrame                          *InnerLeftFrame;
  vtkKWScalarComponentSelectionWidget *ComponentSelectionWidget;
  vtkKWMenuButtonWithLabel            *InterpolationTypeOptionMenu;
  vtkKWVolumeMaterialPropertyWidget   *MaterialPropertyWidget;
  vtkKWCheckButton                    *EnableShadingCheckButton;
  vtkKWCheckButton                    *InteractiveApplyCheckButton;
  vtkKWPiecewiseFunctionEditor        *ScalarOpacityFunctionEditor;
  vtkKWScaleWithEntry                 *ScalarOpacityUnitDistanceScale;
  vtkKWColorTransferFunctionEditor    *ScalarColorFunctionEditor;
  vtkKWCheckButton                    *LockOpacityAndColorCheckButton;
  vtkKWPiecewiseFunctionEditor        *GradientOpacityFunctionEditor;
  vtkKWMenuButton                     *EnableGradientOpacityOptionMenu;
  vtkKWScaleWithEntrySetWithLabel     *ComponentWeightScaleSet;

  int                                 LockOpacityAndColor[VTK_MAX_VRCOMP];
  int                                 WindowLevelMode[VTK_MAX_VRCOMP];

  // Are the components independent of each other?

  virtual int GetIndependentComponents();
  
  // Update HSV selector

  virtual void UpdateHSVColorSelectorFromScalarColorFunctionEditor();

  // This methods will be overriden in subclasses so that something
  // different than the DataSet ivar will be used to compute the
  // corresponding items
  virtual int GetNumberOfComponents();
  virtual int GetDataSetScalarRange(int comp, double range[2]);
  virtual int GetDataSetAdjustedScalarRange(int comp, double range[2]);
  virtual const char* GetDataSetScalarName();
  virtual int GetDataSetScalarOpacityUnitDistanceRangeAndResolution(
    double range[2], double *resolution);

private:
  vtkKWVolumePropertyWidget(const vtkKWVolumePropertyWidget&); // Not implemented
  void operator=(const vtkKWVolumePropertyWidget&); // Not implemented
};

#endif
