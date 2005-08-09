/*=========================================================================

  Module:    $RCSfile: vtkKWScaleWithEntry.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWScaleWithEntry - a scale widget with a label and entry
// .SECTION Description
// A widget that represents a more complex scale (or slider) with options for 
// positioning a label, displaying an entry, working in popup mode, etc.
// For a more lightweight widget, check vtkKWScale.
// The label position and visibility can be changed through the 
// vtkKWWidgetWithLabel superclass methods (see SetLabelPosition).
// The default position for the label is on the left of the scale for 
// horizontal scales, on the right (top corner) for vertical ones.
// The default position for the entry is on the right of the scale for
// horizontal scales, on the right (bottom corner) for vertical ones.
// Both label and entry are visible by default.
// For convenience purposes, an empty label is not displayed though (make
// sure you use the SetLabelText method to set the label).
// In popup mode, a small button replaces the scale. Pressing that button
// will popup a scale the user can interact with, until it leaves the scale
// focus, which will withdraw the popup automatically. In that mode, the
// label and entry position are always displayed side by side, horizontally.
// Many of the vtkKWScale methods have been added to this API for 
// convenience purposes, but GetScale() can be used to retrieve the
// internal vtkKWScale and access the rest of the API.
// .SECTION See Also
// vtkKWScale vtkKWScaleWithLabel vtkKWWidgetWithLabel

#ifndef __vtkKWScaleWithEntry_h
#define __vtkKWScaleWithEntry_h

#include "vtkKWScaleWithLabel.h"

class vtkKWApplication;
class vtkKWEntry;
class vtkKWLabel;
class vtkKWPushButton;
class vtkKWTopLevel;

class KWWIDGETS_EXPORT vtkKWScaleWithEntry : public vtkKWScaleWithLabel
{
public:
  static vtkKWScaleWithEntry* New();
  vtkTypeRevisionMacro(vtkKWScaleWithEntry,vtkKWScaleWithLabel);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create the widget.
  virtual void Create(vtkKWApplication *app);

  // Description:
  // Set the range for this scale.
  virtual void SetRange(double min, double max);
  virtual void SetRange(const double *range) 
    { this->SetRange(range[0], range[1]); };
  virtual double *GetRange();
  virtual void GetRange(double &min, double &max);
  virtual void GetRange(double range[2])
    { this->GetRange(range[0], range[1]); };
  virtual double GetRangeMin() { return this->GetRange()[0]; };
  virtual double GetRangeMax() { return this->GetRange()[1]; };

  // Description:
  // Set/Get the value of the scale.
  virtual void SetValue(double v);
  virtual double GetValue();

  // Description:
  // Method to set/get the resolution of the slider.
  virtual void SetResolution(double r);
  virtual double GetResolution();

  // Description:
  // Turn on/off the automatic clamping of the end values when the 
  // user types a value beyond the range. Default is on.
  virtual void SetClampValue(int);
  virtual int GetClampValue();
  vtkBooleanMacro(ClampValue, int);

  // Description:
  // Get the internal vtkKWScale.
  // Retrive that object to set the resolution, range, value, etc.
  virtual vtkKWScale* GetScale()
    { return this->GetWidget(); };
  
  // Description:
  // Set/get whether to display the range of the scale.
  // If the scale orientation is horizontal, the min is displayed on the
  // left of the scale, the max range on the right. For vertical orientation,
  // min is on top, max at the bottom. In popup mode, the range is
  // displayed directly in the popup window using the same layout as above.
  void SetRangeVisibility(int flag);
  vtkGetMacro(RangeVisibility, int);
  vtkBooleanMacro(RangeVisibility, int);
  
  // Description:
  // Set/Get the internal entry visibility (On by default).
  // IMPORTANT: if you know you may not show the entry, try to
  // set that flag as early as possible (ideally, before calling Create()) 
  // in order to lower the footprint of the widget: the entry will not be
  // allocated and created if there is no need to show it.
  // Later on, you can still use that option to show the entry: it will be
  // allocated and created on the fly.
  virtual void SetEntryVisibility(int);
  vtkBooleanMacro(EntryVisibility, int);
  vtkGetMacro(EntryVisibility, int);

  // Description:
  // Convenience method to set the contents label.
  // IMPORTANT: this method will create the label on the fly, use it only if
  // you are confident that you will indeed display the label.
  // Override the superclass so that the label is packed/unpacked if the
  // text value is an empty string. 
  virtual void SetLabelText(const char *);
  
  // Description:
  // Get the internal entry.
  // IMPORTANT: the internal entry is "lazy created", i.e. it is neither
  // allocated nor created until GetEntry() is called. This allows 
  // for a lower footprint and faster UI startup. Therefore, do *not* use
  // GetEntry() to check if the entry exists, as it will automatically
  // allocate the entry. Use HasEntry() instead. 
  virtual vtkKWEntry* GetEntry();
  virtual int HasEntry();

  // Description:
  // Convenience method to set/get the entry width.
  // IMPORTANT: this method will create the entry on the fly, use it only if
  // you are confident that you will indeed display the entry.
  virtual void SetEntryWidth(int width);
  virtual int GetEntryWidth();

  // Description:
  // If supported, set the entry position in regards to the rest of
  // the composite widget. Check the subclass for more information about
  // what the Default position is, and if specific positions are supported.
  // The default position for the entry is on the right of the scale for
  // horizontal scales, on the right (bottom corner) for vertical ones.
  // Remember that the label position and visibility can also be changed
  // through the  vtkKWWidgetWithLabel superclass methods (SetLabelPosition).
  //BTX
  enum
  {
    EntryPositionDefault = 0,
    EntryPositionTop,
    EntryPositionBottom,
    EntryPositionLeft,
    EntryPositionRight
  };
  //ETX
  virtual void SetEntryPosition(int);
  vtkGetMacro(EntryPosition, int);
  virtual void SetEntryPositionToDefault()
    { this->SetEntryPosition(vtkKWScaleWithEntry::EntryPositionDefault); };
  virtual void SetEntryPositionToTop()
    { this->SetEntryPosition(vtkKWScaleWithEntry::EntryPositionTop); };
  virtual void SetEntryPositionToBottom()
    { this->SetEntryPosition(vtkKWScaleWithEntry::EntryPositionBottom); };
  virtual void SetEntryPositionToLeft()
    { this->SetEntryPosition(vtkKWScaleWithEntry::EntryPositionLeft); };
  virtual void SetEntryPositionToRight()
    { this->SetEntryPosition(vtkKWScaleWithEntry::EntryPositionRight); };

  // Description:
  // Convenience method to set both the label and entry position to the
  // top of the scale (label in the left corner, entry in the right corner)
  virtual void SetLabelAndEntryPositionToTop();

  // Description:
  // Set/Get a popup scale. 
  // WARNING: must be set *before* Create() is called.
  vtkSetMacro(PopupMode, int);
  vtkGetMacro(PopupMode, int);
  vtkBooleanMacro(PopupMode, int);  
  vtkGetObjectMacro(PopupPushButton, vtkKWPushButton);

  // Description:
  // Set/Get the entry expansion flag. This flag is only used if PopupMode 
  // mode is On. In that case, the default behaviour is to provide a widget
  // as compact as possible, i.e. the Entry won't be expanded if the widget
  // grows. Set ExpandEntry to On to override this behaviour.
  virtual void SetExpandEntry(int flag);
  vtkGetMacro(ExpandEntry, int);
  vtkBooleanMacro(ExpandEntry, int);  

  // Description:
  // Set/Get the orientation type.
  // For widgets that can lay themselves out with either a horizontal or
  // vertical orientation, such as scales, this option specifies which 
  // orientation should be used. 
  // Valid constants can be found in vtkKWTkOptions::OrientationType.
  virtual void SetOrientation(int);
  virtual int GetOrientation();
  virtual void SetOrientationToHorizontal() 
    { this->SetOrientation(vtkKWTkOptions::OrientationHorizontal); };
  virtual void SetOrientationToVertical() 
    { this->SetOrientation(vtkKWTkOptions::OrientationVertical); };

  // Description
  // Set/Get the desired long dimension of the scale. 
  // For vertical scales this is the scale's height, for horizontal scales
  // it is the scale's width. In pixel.
  virtual void SetLength(int length);
  virtual int GetLength();

  // Description:
  // Specifies commands to associate with the widget. 
  // The first argument is the object that will have the method called on it.
  // The second argument is the name of the method to be called and any
  // arguments in string form. If the object is NULL, the method
  // is evaluated as a simple command.
  virtual void SetCommand(vtkObject *object, const char *method);
  virtual void SetStartCommand(vtkObject *object, const char *method);
  virtual void SetEndCommand(vtkObject *object, const char *method);
  virtual void SetEntryCommand(vtkObject *object, const char *method);
  virtual void InvokeEntryCommand();

  // Description:
  // Set/get whether the above commands should be called or not.
  // This make it easier to disable the commands while setting the scale
  // value for example.
  virtual void SetDisableCommands(int);
  virtual int GetDisableCommands();
  vtkBooleanMacro(DisableCommands, int);

  // Description:
  // Setting this string enables balloon help for this widget.
  // Override to pass down to children for cleaner behavior
  virtual void SetBalloonHelpString(const char *str);

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
  virtual void DisplayPopupModeCallback();
  virtual void WithdrawPopupModeCallback();
  virtual void EntryValueCallback();
  virtual void ScaleValueCallback(double num);

protected:
  vtkKWScaleWithEntry();
  ~vtkKWScaleWithEntry();

  // Description:
  // Bind/Unbind all components so that values can be changed, but
  // no command will be called.
  void Bind();
  void UnBind();

  int PopupMode;
  int RangeVisibility;

  char *EntryCommand;
  int EntryVisibility;
  int EntryPosition;
  int ExpandEntry;

  vtkKWTopLevel   *TopLevel;
  vtkKWPushButton *PopupPushButton;

  vtkKWLabel *RangeMinLabel;
  vtkKWLabel *RangeMaxLabel;

  // Description:
  // Pack or repack the widget. To be implemented by subclasses.
  virtual void Pack();

  // Description:
  // Create the entry
  virtual void CreateEntry(vtkKWApplication *app);

  // Description:
  // Update internal widgets value
  virtual void UpdateValue();
  virtual void SetEntryValue(double num);
  virtual void UpdateRange();

private:

  // Description:
  // Internal entry
  // In 'private:' to allow lazy evaluation. GetEntry() will create the
  // entry if it does not exist. This allow the object to remain lightweight. 
  vtkKWEntry *Entry;

  vtkKWScaleWithEntry(const vtkKWScaleWithEntry&); // Not implemented
  void operator=(const vtkKWScaleWithEntry&); // Not implemented
};


#endif



