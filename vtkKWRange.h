/*=========================================================================

  Module:    $RCSfile: vtkKWRange.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWRange - a range widget
// .SECTION Description
// A widget that represents a range within a bigger range.

#ifndef __vtkKWRange_h
#define __vtkKWRange_h

#include "vtkKWLabeledWidget.h"

class vtkKWCanvas;
class vtkKWEntry;
class vtkKWFrame;
class vtkKWPushButtonSet;

class VTK_EXPORT vtkKWRange : public vtkKWLabeledWidget
{
public:
  static vtkKWRange* New();
  vtkTypeRevisionMacro(vtkKWRange,vtkKWLabeledWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create the widget.
  virtual void Create(vtkKWApplication *app, const char *args = 0);

  // Description:
  // Set/Get the whole range.
  vtkGetVector2Macro(WholeRange, double);
  virtual void SetWholeRange(double r0, double r1);
  virtual void SetWholeRange(double range[2]) 
    { this->SetWholeRange(range[0], range[1]); };

  // Description:
  // Set/Get the current (sub-)range.
  vtkGetVector2Macro(Range, double);
  virtual void SetRange(double r0, double r1);
  virtual void SetRange(double *range) 
    { this->SetRange(range[0], range[1]); };

  // Description:
  // Set/Get the current (sub-)range as relative positions in the whole range.
  virtual void GetRelativeRange(double &r0, double &r1);
  virtual void GetRelativeRange(double range[2])
    { this->GetRelativeRange(range[0], range[1]); };
  virtual void SetRelativeRange(double r0, double r1);
  virtual void SetRelativeRange(double range[2])
    { this->SetRelativeRange(range[0], range[1]); };
  
  // Description:
  // Method to set/get the resolution of the slider.
  // The whole range and sub range are not snapped to this resolution.
  // Both ranges can be set to any floating point number. 
  // Think of the sliders and the resolution as a way to set the bounds of
  // the sub range interactively using nice clean steps (power of 10 for 
  // example).
  // The entries associated to the sub range can be used to set the bounds to 
  // anything within the whole range, despite the resolution, allowing the user
  // to enter precise values that could not be reached given the resolution.
  // Of course, given a whole range of 1 to 64, if the resolution is set to 3
  // the slider will only snap to values ranging from 3 to 63 (within the 
  // whole range constraint), but the entries can be used to set accurate
  // values out of the resolution (i.e., 1, 2... 64).
  virtual void SetResolution(double r);
  vtkGetMacro(Resolution, double);

  // Description:
  // Adjust the resolution automatically (to a power of 10 in this implem)
  virtual void SetAdjustResolution(int);
  vtkBooleanMacro(AdjustResolution, int);
  vtkGetMacro(AdjustResolution, int);
  
  // Description:
  // Set/Get the orientation.
  //BTX
  enum 
  {
    ORIENTATION_HORIZONTAL = 0,
    ORIENTATION_VERTICAL   = 1
  };
  //ETX
  virtual void SetOrientation(int);
  vtkGetMacro(Orientation, int);
  virtual void SetOrientationToHorizontal()
    { this->SetOrientation(vtkKWRange::ORIENTATION_HORIZONTAL); };
  virtual void SetOrientationToVertical() 
    { this->SetOrientation(vtkKWRange::ORIENTATION_VERTICAL); };

  // Description:
  // Set/Get the order of the sliders (inverted means that the first slider
  // will be associated to Range[1], the last to Range[0])
  virtual void SetInverted(int);
  vtkBooleanMacro(Inverted, int);
  vtkGetMacro(Inverted, int);

  // Description:
  // Set/Get the desired narrow dimension of the widget. For horizontal widget
  // this is the widget height, for vertical this is the width.
  // In the current implementation, this controls the sliders narrow dim.
  virtual void SetThickness(int);
  vtkGetMacro(Thickness, int);
  
  // Description:
  // Set/Get the desired narrow dimension of the internal widget as a fraction
  // of the thickness of the widget (see Thickness). 
  // In the current implementation, this controls the range bar narrow dim.
  virtual void SetInternalThickness(double);
  vtkGetMacro(InternalThickness, double);
  
  // Description:
  // Set/Get the slider size.
  virtual void SetSliderSize(int);
  vtkGetMacro(SliderSize, int);
  
  // Description:
  // Set/Get if a slider can push another slider when bumping into it
  vtkSetMacro(SliderCanPush, int);
  vtkBooleanMacro(SliderCanPush, int);
  vtkGetMacro(SliderCanPush, int);

  // Description:
  // Set/Get the (sub) range scale color. 
  // Defaults to -1, -1, -1: a shade of the widget background color will
  // be used at runtime.
  vtkGetVector3Macro(RangeColor, double);
  virtual void SetRangeColor(double r, double g, double b);
  virtual void SetRangeColor(double rgb[3])
    { this->SetRangeColor(rgb[0], rgb[1], rgb[2]); };
  
  // Description:
  // Set/Get the (sub) range scale interaction color. Used when interaction
  // is performed using the sliders.
  // IF set to -1, -1, -1: a shade of the widget background color will
  // be used at runtime.
  vtkGetVector3Macro(RangeInteractionColor, double);
  virtual void SetRangeInteractionColor(double r, double g, double b);
  virtual void SetRangeInteractionColor(double rgb[3])
    { this->SetRangeInteractionColor(rgb[0], rgb[1], rgb[2]); };
  
  // Description:
  // Show/Hide the entries.
  virtual void SetShowEntries(int);
  vtkBooleanMacro(ShowEntries, int);
  vtkGetMacro(ShowEntries, int);

  // Description:
  // Get the entries object.
  virtual vtkKWEntry* GetEntry1()
    { return this->Entries[0]; };
  virtual vtkKWEntry* GetEntry2()
    { return this->Entries[1]; };

  // Description:
  // Set/Get the entries width (in chars).
  virtual void SetEntriesWidth(int width);
  vtkGetMacro(EntriesWidth, int);

  // Description:
  // Set/Get the position of the items (label and entries) in the widget.
  // POSITION_ALIGNED: the items are aligned with the range/sliders
  // POSITION_SIDE1: the items are on top/left of the range/sliders
  // POSITION_SIDE2: the items are at the bottom/right of the range/sliders
  //BTX
  enum
  {
    POSITION_ALIGNED = 0,
    POSITION_SIDE1   = 1,
    POSITION_SIDE2   = 2
  };
  //ETX
  virtual void SetLabelPosition(int);
  virtual void SetLabelPositionToAligned()
    { this->SetLabelPosition(vtkKWRange::POSITION_ALIGNED); };
  virtual void SetLabelPositionToSide1()
    { this->SetLabelPosition(vtkKWRange::POSITION_SIDE1); };
  virtual void SetLabelPositionToSide2()
    { this->SetLabelPosition(vtkKWRange::POSITION_SIDE2); };
  vtkBooleanMacro(LabelPosition, int);
  vtkGetMacro(LabelPosition, int);
  virtual void SetEntriesPosition(int);
  virtual void SetEntriesPositionToAligned()
    { this->SetEntriesPosition(vtkKWRange::POSITION_ALIGNED); };
  virtual void SetEntriesPositionToSide1()
    { this->SetEntriesPosition(vtkKWRange::POSITION_SIDE1); };
  virtual void SetEntriesPositionToSide2()
    { this->SetEntriesPosition(vtkKWRange::POSITION_SIDE2); };
  vtkBooleanMacro(EntriesPosition, int);
  vtkGetMacro(EntriesPosition, int);
  virtual void SetZoomButtonsPosition(int);
  virtual void SetZoomButtonsPositionToAligned()
    { this->SetZoomButtonsPosition(vtkKWRange::POSITION_ALIGNED); };
  virtual void SetZoomButtonsPositionToSide1()
    { this->SetZoomButtonsPosition(vtkKWRange::POSITION_SIDE1); };
  virtual void SetZoomButtonsPositionToSide2()
    { this->SetZoomButtonsPosition(vtkKWRange::POSITION_SIDE2); };
  vtkBooleanMacro(ZoomButtonsPosition, int);
  vtkGetMacro(ZoomButtonsPosition, int);

  // Description:
  // Show/Hide the zoom buttons
  virtual void SetShowZoomButtons(int);
  vtkBooleanMacro(ShowZoomButtons, int);
  vtkGetMacro(ShowZoomButtons, int);
  
  // Description:
  // Set commands.
  virtual void SetCommand(vtkKWObject* object, const char *method);
  virtual void SetStartCommand(vtkKWObject* object, const char *method);
  virtual void SetEndCommand(vtkKWObject* object, const char *method);
  virtual void SetEntriesCommand(vtkKWObject* object, const char *method);
  virtual void InvokeCommand();
  virtual void InvokeStartCommand();
  virtual void InvokeEndCommand();
  virtual void InvokeEntriesCommand();

  // Description:
  // Set/get whether the above commands should be called or not.
  // This allow you to disable the commands while you are setting the range
  // value for example.
  vtkSetMacro(DisableCommands, int);
  vtkGetMacro(DisableCommands, int);
  vtkBooleanMacro(DisableCommands, int);

  // Description:
  // Set the string that enables balloon help for this widget.
  // Override to pass down to children.
  virtual void SetBalloonHelpString(const char *str);
  virtual void SetBalloonHelpJustification(int j);

  // Description:
  // Callbacks
  //BTX
  enum
  {
    SLIDER_INDEX_1 = 1,
    SLIDER_INDEX_2 = 2
  };
  //ETX
  virtual void ConfigureCallback();
  virtual void MaximizeRangeCallback();
  virtual void EnlargeRangeCallback();
  virtual void ShrinkRangeCallback();
  virtual void EntriesUpdateCallback(int i);
  virtual void StartInteractionCallback(int x, int y);
  virtual void EndInteractionCallback();
  virtual void SliderMotionCallback(int slider_idx, int x, int y);
  virtual void RangeMotionCallback(int x, int y);

  // Description:
  // Access to the canvas
  vtkGetObjectMacro(Canvas, vtkKWCanvas);

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
  vtkSetMacro(ClampRange, int);
  vtkGetMacro(ClampRange, int);
  vtkBooleanMacro(ClampRange, int);

protected:
  vtkKWRange();
  ~vtkKWRange();

  double WholeRange[2];
  double Range[2];
  double WholeRangeAdjusted[2];
  double RangeAdjusted[2];
  double Resolution;
  int   AdjustResolution;
  int   Inverted;
  int   Thickness;
  double InternalThickness;
  int   Orientation;
  int   DisableCommands;
  int   SliderSize;
  double RangeColor[3];
  double RangeInteractionColor[3];
  int   ShowEntries;
  int   LabelPosition;
  int   EntriesPosition;
  int   ZoomButtonsPosition;
  int   EntriesWidth;
  int   SliderCanPush;
  int   ShowZoomButtons;

  int   InInteraction;
  int   StartInteractionPos;
  double StartInteractionRange[2];

  int ClampRange;

  char  *Command;
  char  *StartCommand;
  char  *EndCommand;
  char  *EntriesCommand;

  vtkKWFrame         *CanvasFrame;
  vtkKWCanvas        *Canvas;
  vtkKWEntry         *Entries[2];
  vtkKWPushButtonSet *ZoomButtons;

  virtual void CreateEntries();
  virtual void CreateZoomButtons();
  virtual void UpdateEntriesValue(double range[2]);
  virtual void ConstrainResolution();

  // Description:
  // Bind/Unbind all components.
  virtual void Bind();
  virtual void UnBind();

  // Description:
  // Make sure all elements are constrained correctly
  virtual void ConstrainRangeToResolution(double range[2], int adjust = 1);
  virtual void ConstrainRangeToWholeRange(
    double range[2], double whole_range[2], double *old_range_hint = 0);
  virtual void ConstrainWholeRange();
  virtual void ConstrainRange(double *old_range_hint = 0);
  virtual void ConstrainRanges();

  // Description:
  // Pack the widget
  virtual void Pack();

  // Description:
  // Get element colors (and shades)
  //BTX
  enum
  {
    DARK_SHADOW_COLOR,
    LIGHT_SHADOW_COLOR,
    BACKGROUND_COLOR,
    HIGHLIGHT_COLOR
  };
  //ETX
  virtual void GetWholeRangeColor(int type, int &r, int &g, int &b);
  virtual void GetRangeColor(int type, int &r, int &g, int &b);
  virtual void GetSliderColor(int type, int &r, int &g, int &b);

  // Description:
  // Redraw elements
  virtual void RedrawCanvas();
  virtual void RedrawWholeRange();
  virtual void RedrawRange();
  virtual void RedrawSliders();
  virtual void RedrawSlider(int x, int slider_idx);
  virtual void UpdateRangeColors();
  virtual void UpdateColors();

  // Description:
  // Convenience method to look for a tag
  virtual int HasTag(const char *tag, const char *suffix = 0);

  // Description:
  // Get the current sliders center positions
  virtual void GetSlidersPositions(int pos[2]);

private:
  vtkKWRange(const vtkKWRange&); // Not implemented
  void operator=(const vtkKWRange&); // Not implemented
};

#endif

