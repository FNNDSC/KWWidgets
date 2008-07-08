/*=========================================================================

  Module:    $RCSfile: vtkKWColorSpectrumWidget.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWColorSpectrumWidget - a color spectrum widget.
// .SECTION Description
// This widget can be used to display a 2D canvas interpolating a slice
// of a color space/spectrum.
// It is used internally by the vtkKWColorPickerWidget class.
// .SECTION Thanks
// This work is part of the National Alliance for Medical Image
// Computing (NAMIC), funded by the National Institutes of Health
// through the NIH Roadmap for Medical Research, Grant U54 EB005149.
// Information on the National Centers for Biomedical Computing
// can be obtained from http://nihroadmap.nih.gov/bioinformatics.
// .SECTION See Also
// vtkKWColorPickerDialog vtkKWColorSpectrumWidget vtkKWColorPresetSelector


#ifndef __vtkKWColorSpectrumWidget_h
#define __vtkKWColorSpectrumWidget_h

#include "vtkKWCompositeWidget.h"

class vtkKWLabel;
class vtkKWRadioButtonSet;
class vtkKWCanvas;
class vtkKWColorSpectrumWidgetInternals;

class KWWidgets_EXPORT vtkKWColorSpectrumWidget : public vtkKWCompositeWidget
{
public:
  static vtkKWColorSpectrumWidget* New();
  vtkTypeRevisionMacro(vtkKWColorSpectrumWidget,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the current color as RGB. 
  virtual double *GetColorAsRGB();
  virtual void GetColorAsRGB(double &_arg1, double &_arg2, double &_arg3);
  virtual void GetColorAsRGB(double _arg[3]);
  virtual void SetColorAsRGB(double r, double g, double b);
  virtual void SetColorAsRGB(double rgb[3]);

  // Description:
  // Set/Get the current color as HSV. 
  virtual double *GetColorAsHSV();
  virtual void GetColorAsHSV(double &_arg1, double &_arg2, double &_arg3);
  virtual void GetColorAsHSV(double _arg[3]);
  virtual void SetColorAsHSV(double r, double g, double b);
  virtual void SetColorAsHSV(double rgb[3]);

  // Description:
  // Set/Get the interpolation axis. 
  //BTX
  enum
  {
    FixedAxisR = 0,
    FixedAxisG,
    FixedAxisB,
    FixedAxisH,
    FixedAxisS,
    FixedAxisV
  };
  //ETX
  virtual void SetFixedAxis(int);
  vtkGetMacro(FixedAxis, int);
  virtual void SetFixedAxisToR();
  virtual void SetFixedAxisToG();
  virtual void SetFixedAxisToB();
  virtual void SetFixedAxisToH();
  virtual void SetFixedAxisToS();
  virtual void SetFixedAxisToV();

  // Description:
  // Set/Get the visibility of the fixed axis selector.
  virtual void SetFixedAxisSelectorVisibility(int);
  vtkGetMacro(FixedAxisSelectorVisibility,int);
  vtkBooleanMacro(FixedAxisSelectorVisibility,int);

  // Description:
  // Specifies commands to associate with the widget. 
  // 'ColorChangedCommand' is invoked when the selected color has
  // changed (i.e. at the end of the user interaction).
  // 'ColorChangingCommand' is invoked when the selected color is
  // changing (i.e. during the user interaction).
  // The need for a '...ChangedCommand' and '...ChangingCommand' can be
  // explained as follows: the former can be used to be notified about any
  // changes made to this widget *after* the corresponding user interaction has
  // been performed (say, after releasing the mouse button). The later can be
  // set *additionally* to be notified about the intermediate changes that
  // occur *during* the corresponding user interaction. While setting 
  // '...ChangedCommand' is enough to be notified about any changes, 
  // setting '...ChangingCommand' is an application-specific
  // choice that is likely to depend on how fast you want (or can) answer to
  // rapid changes occuring during user interaction, if any.
  // The 'object' argument is the object that will have the method called on
  // it. The 'method' argument is the name of the method to be called and any
  // arguments in string form. If the object is NULL, the method is still
  // evaluated as a simple command. 
  virtual void SetColorChangedCommand(vtkObject *object, const char *method);
  virtual void SetColorChangingCommand(vtkObject *object, const char *method);

  // Description:
  // Events.
  //BTX
  enum
  {
    ColorChangedEvent = 10000,
    ColorChangingEvent
  };
  //ETX

  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object is updated
  // and propagated to its internal parts/subwidgets. This will, for example,
  // enable/disable parts of the widget UI, enable/disable the visibility
  // of 3D widgets, etc.
  virtual void UpdateEnableState();

  // Description:
  // Callbacks. Internal, do not use.
  virtual void PickColorPressCallback(int x, int y);
  virtual void PickColorMoveCallback(int x, int y);
  virtual void PickColorReleaseCallback();

protected:
  vtkKWColorSpectrumWidget();
  ~vtkKWColorSpectrumWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Description:
  // Set/Get internal colors. 
  double InternalColorRGB[3];
  vtkGetVector3Macro(InternalColorRGB, double);
  virtual void SetInternalColorRGB(double r, double g, double b);
  virtual void SetInternalColorRGB(double rgb[3]);
  double InternalColorHSV[3];
  vtkGetVector3Macro(InternalColorHSV, double);
  virtual void SetInternalColorHSV(double h, double s, double v);
  virtual void SetInternalColorHSV(double hsv[3]);

  vtkKWCanvas         *ColorCanvas;
  vtkKWRadioButtonSet *FixedAxisRadioButtonSet;

  virtual void UpdateColorCanvas();
  virtual void UpdateColorCursor();
  virtual void Pack();

  int FixedAxis;
  int FixedAxisSelectorVisibility;

  // Description:
  // Commands.
  char *ColorChangedCommand;
  char *ColorChangingCommand;
  virtual void InvokeColorChangedCommand();
  virtual void InvokeColorChangingCommand();

  // Description:
  // Bind/Unbind all components.
  virtual void AddBindings();
  virtual void RemoveBindings();

  // PIMPL Encapsulation for STL containers
  //BTX
  vtkKWColorSpectrumWidgetInternals *Internals;
  //ETX

private:
  vtkKWColorSpectrumWidget(const vtkKWColorSpectrumWidget&); // Not implemented
  void operator=(const vtkKWColorSpectrumWidget&); // Not implemented
};


#endif
