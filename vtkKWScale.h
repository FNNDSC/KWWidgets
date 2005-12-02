/*=========================================================================

  Module:    $RCSfile: vtkKWScale.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWScale - a scale (slider) widget
// .SECTION Description
// A widget that repsentes a scale (or slider).
// .SECTION See Also
// vtkKWScaleWithEntry

#ifndef __vtkKWScale_h
#define __vtkKWScale_h

#include "vtkKWCoreWidget.h"

class vtkKWEntry;
class vtkKWLabel;
class vtkKWPushButton;
class vtkKWTopLevel;

class KWWIDGETS_EXPORT vtkKWScale : public vtkKWCoreWidget
{
public:
  static vtkKWScale* New();
  vtkTypeRevisionMacro(vtkKWScale,vtkKWCoreWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create the widget.
  virtual void Create();

  // Description:
  // Set the range for this scale.
  virtual void SetRange(double min, double max);
  virtual void SetRange(const double *range) 
    { this->SetRange(range[0], range[1]); };
  vtkGetVector2Macro(Range, double);
  virtual double GetRangeMin() { return this->GetRange()[0]; };
  virtual double GetRangeMax() { return this->GetRange()[1]; };

  // Description:
  // Set/Get the value of the scale.
  virtual void SetValue(double v);
  vtkGetMacro(Value, double);

  // Description:
  // Method to set/get the resolution of the slider.
  // The range or the value of the scale are not snapped to this resolution.
  // The range and the value can be any floating point number. 
  // Think of the slider and the resolution as a way to set the value
  // interactively using nice clean steps (power of 10 for example).
  // The entry associated to the scale can be used to set the value to 
  // anything within the range, despite the resolution, allowing the user
  // to enter a precise value that could not be reached given the resolution.
  virtual void SetResolution(double r);
  vtkGetMacro(Resolution, double);
  
  // Description:
  // Set/Get the orientation type.
  // For widgets that can lay themselves out with either a horizontal or
  // vertical orientation, such as scales, this option specifies which 
  // orientation should be used. 
  // Valid constants can be found in vtkKWTkOptions::OrientationType.
  virtual void SetOrientation(int);
  vtkGetMacro(Orientation, int);
  virtual void SetOrientationToHorizontal() 
    { this->SetOrientation(vtkKWTkOptions::OrientationHorizontal); };
  virtual void SetOrientationToVertical() 
    { this->SetOrientation(vtkKWTkOptions::OrientationVertical); };

  // Description:
  // Set/Get the trough color, i.e. the color to use for the rectangular
  // trough areas in widgets such as scrollbars and scales.
  virtual void GetTroughColor(double *r, double *g, double *b);
  virtual double* GetTroughColor();
  virtual void SetTroughColor(double r, double g, double b);
  virtual void SetTroughColor(double rgb[3])
    { this->SetTroughColor(rgb[0], rgb[1], rgb[2]); };

  // Description
  // Set/Get the narrow dimension of scale. For vertical 
  // scales this is the trough's width; for horizontal scales this is the 
  // trough's height. In pixel.
  virtual void SetWidth(int width);
  virtual int GetWidth();

  // Description
  // Set/Get the desired long dimension of the scale. 
  // For vertical scales this is the scale's height, for horizontal scales
  // it is the scale's width. In pixel.
  virtual void SetLength(int length);
  virtual int GetLength();

  // Description
  // Set/Get the size of the slider, measured in screen units along 
  // the slider's long dimension.
  virtual void SetSliderLength(int length);
  virtual int GetSliderLength();

  // Description:
  // Set/Get the visibility of the value on top of the slider.
  virtual void SetValueVisibility(int);
  virtual int GetValueVisibility();
  vtkBooleanMacro(ValueVisibility, int);

  // Description:
  // Method to set/get the tick interval.
  // Determines the spacing between numerical tick marks displayed below or to
  // the left of the slider. If 0, no tick marks will be displayed. 
  virtual void SetTickInterval(double val);
  virtual double GetTickInterval();
  
  // Description
  // Set/Get the sting to display as a label for the scale. 
  // For vertical scales the label is displayed just to the right of the top
  // end of the scale. For horizontal scales the label is displayed just above
  // the left end of the scale. If the option is specified as an empty string, 
  // no label is displayed. The position of the label can not be changed. For
  // more elaborated options, check vtkKWScaleWithEntry
  virtual void SetLabelText(const char *);
  virtual const char* GetLabelText();

  // Description:
  // Specifies commands to associate with the widget. 
  // 'Command' is invoked when the widget value is changed.
  // 'StartCommand' is invoked at the beginning of an interaction with
  // the widget.
  // 'EndCommand' is invoked at the end of an interaction with the widget.
  // The first argument is the object that will have the method called on it.
  // The second argument is the name of the method to be called and any
  // arguments in string form. If the object is NULL, the method
  // is still evaluated as a simple command. 
  virtual void SetCommand(vtkObject *object, const char *method);
  virtual void SetStartCommand(vtkObject *object, const char *method);
  virtual void SetEndCommand(vtkObject *object, const char *method);

  // Description:
  // Set/get whether the above commands should be called or not.
  // This make it easier to disable the commands while setting the scale
  // value for example.
  vtkSetMacro(DisableCommands, int);
  vtkGetMacro(DisableCommands, int);
  vtkBooleanMacro(DisableCommands, int);

  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object is updated
  // and propagated to its internal parts/subwidgets. This will, for example,
  // enable/disable parts of the widget UI, enable/disable the visibility
  // of 3D widgets, etc.
  virtual void UpdateEnableState();

  // Description:
  // Turn on/off the automatic clamping of the end values when the 
  // user types a value beyond the range. Default is on.
  vtkSetMacro(ClampValue, int);
  vtkGetMacro(ClampValue, int);
  vtkBooleanMacro(ClampValue, int);

  // Description:
  // Callbacks. Internal, do not use.
  vtkGetMacro(DisableScaleValueCallback, int);
  vtkSetMacro(DisableScaleValueCallback, int);
  vtkBooleanMacro(DisableScaleValueCallback, int);
  virtual void ScaleValueCallback(double num);
  virtual void ButtonPressCallback();
  virtual void ButtonReleaseCallback();

protected:
  vtkKWScale();
  ~vtkKWScale();

  // Description:
  // Bind/Unbind all components so that values can be changed, but
  // no command will be called.
  void Bind();
  void UnBind();

  int DisableCommands;
  int DisableScaleValueCallback;
  int ClampValue;

  char        *Command;
  char        *StartCommand;
  char        *EndCommand;
  char        *EntryCommand;

  double       Value;
  double       Resolution;
  double       Range[2];

  int Orientation;

  // Description:
  // Update internal widgets value
  virtual void UpdateRange();
  virtual void UpdateResolution();
  virtual void UpdateValue();
  virtual void UpdateOrientation();

  //BTX
  friend class vtkKWScaleWithEntry;
  //ETX

  virtual void InvokeObjectMethodCommand(const char *command);
  virtual void InvokeCommand();
  virtual void InvokeStartCommand();
  virtual void InvokeEndCommand();

private:
  vtkKWScale(const vtkKWScale&); // Not implemented
  void operator=(const vtkKWScale&); // Not implemented
};


#endif
