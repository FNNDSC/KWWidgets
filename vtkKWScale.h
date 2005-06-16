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
// A widget that repsentes a scale (or slider) with options for 
// a label string and a text entry box.

#ifndef __vtkKWScale_h
#define __vtkKWScale_h

#include "vtkKWWidget.h"

class vtkKWApplication;
class vtkKWEntry;
class vtkKWLabel;
class vtkKWPushButton;
class vtkKWTopLevel;

class KWWIDGETS_EXPORT vtkKWScale : public vtkKWWidget
{
public:
  static vtkKWScale* New();
  vtkTypeRevisionMacro(vtkKWScale,vtkKWWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create the widget.
  virtual void Create(vtkKWApplication *app);

  // Description:
  // Set the range for this scale.
  virtual void SetRange(double min, double max);
  virtual void SetRange(const float *range) { this->SetRange(range[0], range[1]); };
  virtual void SetRange(const double *range) { this->SetRange(range[0], range[1]); };
  vtkGetVector2Macro(Range, double);
  virtual void GetRange(float &min, float &max) {
    min = (float)this->GetRange()[0];
    max = (float)this->GetRange()[1];
    }
  virtual void GetRange(float array[2]) {
    this->GetRange(array[0], array[1]);
    }
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
  // Set/get whether to display the range of the scale
  void SetDisplayRange(int flag);
  vtkGetMacro(DisplayRange, int);
  vtkBooleanMacro(DisplayRange, int);
  
  // Description:
  // Display a label and/or a text entry box. These are optional.
  // Get the corresponding internal objects.
  void DisplayEntry();  
  void DisplayLabel(const char *l);  
  vtkGetObjectMacro(Label, vtkKWLabel);
  vtkGetObjectMacro(Entry, vtkKWEntry);
  vtkGetObjectMacro(Scale, vtkKWWidget);
  vtkSetStringMacro(ShortLabel);
  vtkGetStringMacro(ShortLabel);
  
  // Description:
  // Set/Get the position of the label and/or entry (on top, or on the side).
  virtual void SetDisplayEntryAndLabelOnTop(int flag);
  vtkGetMacro(DisplayEntryAndLabelOnTop, int);
  vtkBooleanMacro(DisplayEntryAndLabelOnTop, int);  

  // Description:
  // Set/Get a popup scale. 
  // WARNING: must be set *before* Create() is called.
  vtkSetMacro(PopupScale, int);
  vtkGetMacro(PopupScale, int);
  vtkBooleanMacro(PopupScale, int);  
  void DisplayPopupScaleCallback();
  void WithdrawPopupScaleCallback();
  vtkGetObjectMacro(PopupPushButton, vtkKWPushButton);

  // Description:
  // Set/Get the entry expansion flag. This flag is only used if PopupScale 
  // mode is On. In that case, the default behaviour is to provide a widget
  // as compact as possible, i.e. the Entry won't be expanded if the widget
  // grows. Set ExpandEntry to On to override this behaviour.
  virtual void SetExpandEntry(int flag);
  vtkGetMacro(ExpandEntry, int);
  vtkBooleanMacro(ExpandEntry, int);  

  // Description:
  // Method that gets invoked when the sliders value has changed.
  virtual void ScaleValueCallback(double num);
  virtual void EntryValueCallback();
  virtual void InvokeStartCommand();
  virtual void InvokeEndCommand();
  virtual void InvokeEntryCommand();

  // Description:
  // A method to set callback functions on objects.  The first argument is
  // the KWObject that will have the method called on it.  The second is the
  // name of the method to be called and any arguments in string form.
  // The calling is done via TCL wrappers for the KWObject.
  virtual void SetCommand      (vtkKWObject* Object, 
                                const char *MethodAndArgString);
  virtual void SetStartCommand (vtkKWObject* Object, 
                                const char *MethodAndArgString);
  virtual void SetEndCommand   (vtkKWObject* Object, 
                                const char *MethodAndArgString);
  virtual void SetEntryCommand (vtkKWObject* Object, 
                                const char *MethodAndArgString);

  // Description:
  // Set/get whether the above commands should be called or not.
  // This allow you to disable the commands while you are setting the scale
  // value for example.
  vtkSetMacro(DisableCommands, int);
  vtkGetMacro(DisableCommands, int);
  vtkBooleanMacro(DisableCommands, int);

  // Description:
  // Setting this string enables balloon help for this widget.
  // Override to pass down to children for cleaner behavior
  virtual void SetBalloonHelpString(const char *str);

  // Description:
  // Bind/Unbind all components so that values can be changed, but
  // no command will be called.
  void Bind();
  void UnBind();

  // Description:
  // Set/Get the resize mode to be smart. In that mode, some elements like the
  // label and the entry will disappear if the widget gets to small.
  vtkSetMacro(SmartResize, int);
  vtkGetMacro(SmartResize, int);
  vtkBooleanMacro(SmartResize, int);  

  void Resize();

  // Description
  // Convenience method to set the width of the label or entry
  virtual void SetLabelWidth(int width);
  virtual void SetEntryWidth(int width);

  // Description
  // Convenience method to set the narrow dimension of scale. For vertical 
  // scales this is the trough's width; for horizontal scales this is the 
  // trough's height. In pixel.
  virtual void SetWidth(int width);

  // Description
  // Convenience method to set the desired long dimension of the scale. 
  // For vertical scales this is the scale's height, for horizontal scales
  // it is the scale's width. In pixel.
  virtual void SetLength(int length);

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
  // Internal
  vtkSetMacro(DisableScaleValueCallback, int);
  vtkBooleanMacro(DisableScaleValueCallback, int);

protected:
  vtkKWScale();
  ~vtkKWScale();

  int         DisplayEntryAndLabelOnTop;
  int         PopupScale;
  int         ExpandEntry;
  int         DisplayRange;
  int         SmartResize;
  int         DisableCommands;
  int         DisableScaleValueCallback;

  char        *Command;
  char        *StartCommand;
  char        *EndCommand;
  char        *EntryCommand;

  double       Value;
  double       Resolution;
  double       Range[2];

  vtkKWWidget   *Scale;
  vtkKWEntry    *Entry;
  vtkKWLabel    *Label;
  vtkKWTopLevel *TopLevel;
  vtkKWPushButton *PopupPushButton;

  vtkKWLabel *RangeMinLabel;
  vtkKWLabel *RangeMaxLabel;
  
  char *NormalLabel;
  char *ShortLabel;
  int LongWidth;
  int MediumWidth;
  int MediumShortWidth;
  int ShortWidth;
  int PackEntry;
  int PackRange;

  int ClampValue;

  vtkSetStringMacro(NormalLabel);
  
  void PackWidget();
  void UpdateEntryResolution();
  void RefreshValue();

private:
  vtkKWScale(const vtkKWScale&); // Not implemented
  void operator=(const vtkKWScale&); // Not implemented
};


#endif



