/*=========================================================================

  Module:    $RCSfile: vtkKWCheckButton.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWCheckButton - check button widget
// .SECTION Description
// A simple widget that represents a check button. It can be modified 
// and queried using the GetSelectedState and SetSelectedState methods.

#ifndef __vtkKWCheckButton_h
#define __vtkKWCheckButton_h

#include "vtkKWCoreWidget.h"

class vtkKWIcon;

class KWWIDGETS_EXPORT vtkKWCheckButton : public vtkKWCoreWidget
{
public:
  static vtkKWCheckButton* New();
  vtkTypeRevisionMacro(vtkKWCheckButton,vtkKWCoreWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create the widget.
  virtual void Create();

  // Description:
  // Set/Get/Toggle the selected state of the check button 0 = off 1 = on
  virtual void SetSelectedState(int );
  virtual int GetSelectedState();
  vtkBooleanMacro(SelectedState, int);
  virtual void ToggleSelectedState();
  virtual void Select() { this->SetSelectedState(1); };
  virtual void Deselect() { this->SetSelectedState(0); };

  // Description:
  // Tell the widget whether it should use an indicator (check box)
  virtual void SetIndicatorVisibility(int ind);
  vtkGetMacro(IndicatorVisibility, int);
  vtkBooleanMacro(IndicatorVisibility, int);

  // Description:
  // Set the text.
  virtual void SetText(const char* txt);
  virtual const char* GetText();

  // Description:
  // Set the variable name.
  vtkGetStringMacro(VariableName);
  virtual void SetVariableName(const char *);

  // Description:
  // Specifies a command to associate with the widget. This command is 
  // typically invoked when the button is selected or deselected.
  // The first argument is the object that will have the method called on it.
  // The second argument is the name of the method to be called and any
  // arguments in string form. If the object is NULL, the method
  // is still evaluated as a simple command. 
  virtual void SetCommand(vtkObject *object, const char *method);

  // Description:
  // Set/Get the anchoring.
  // Specifies how the information in a widget (e.g. text or a bitmap) is to
  // be displayed in the widget.
  // Valid constants can be found in vtkKWTkOptions::AnchorType.
  virtual void SetAnchor(int);
  virtual int GetAnchor();
  virtual void SetAnchorToNorth() 
    { this->SetAnchor(vtkKWTkOptions::AnchorNorth); };
  virtual void SetAnchorToNorthEast() 
    { this->SetAnchor(vtkKWTkOptions::AnchorNorthEast); };
  virtual void SetAnchorToEast() 
    { this->SetAnchor(vtkKWTkOptions::AnchorEast); };
  virtual void SetAnchorToSouthEast() 
    { this->SetAnchor(vtkKWTkOptions::AnchorSouthEast); };
  virtual void SetAnchorToSouth() 
    { this->SetAnchor(vtkKWTkOptions::AnchorSouth); };
  virtual void SetAnchorToSouthWest() 
    { this->SetAnchor(vtkKWTkOptions::AnchorSouthWest); };
  virtual void SetAnchorToWest() 
    { this->SetAnchor(vtkKWTkOptions::AnchorWest); };
  virtual void SetAnchorToNorthWest() 
    { this->SetAnchor(vtkKWTkOptions::AnchorNorthWest); };
  virtual void SetAnchorToCenter() 
    { this->SetAnchor(vtkKWTkOptions::AnchorCenter); };

  // Description:
  // Set/Get the 3-D effect desired for the widget. 
  // Specifies the relief for the button when the indicator is not drawn 
  // (i.e. IndicatorVisibility is Off) and the button is not selected. 
  // The default value is Raised.  By setting this option to Flat and setting
  // IndicatorVisibility to Off and OverRelief to Raised or Flat, the effect
  // is achieved  of having a flat button that raises on mouse-over and which
  // is depressed when activated. This is the behavior typically exhibited by
  // the Bold, Italic, and Underline checkbuttons on the toolbar of a 
  // word-processor, for example. 
  // Valid constants can be found in vtkKWTkOptions::ReliefType.
  virtual void SetOffRelief(int);
  virtual int GetOffRelief();
  virtual void SetOffReliefToRaised() 
    { this->SetOffRelief(vtkKWTkOptions::ReliefRaised); };
  virtual void SetOffReliefToSunken() 
    { this->SetOffRelief(vtkKWTkOptions::ReliefSunken); };
  virtual void SetOffReliefToFlat() 
    { this->SetOffRelief(vtkKWTkOptions::ReliefFlat); };
  virtual void SetOffReliefToRidge() 
    { this->SetOffRelief(vtkKWTkOptions::ReliefRidge); };
  virtual void SetOffReliefToSolid() 
    { this->SetOffRelief(vtkKWTkOptions::ReliefSolid); };
  virtual void SetOffReliefToGroove() 
    { this->SetOffRelief(vtkKWTkOptions::ReliefGroove); };

  // Description:
  // Set/Get the 3-D effect desired for the widget. 
  // Specifies an alternative relief for the button, to be used when the mouse
  // cursor is over the widget. This option can be used to make toolbar 
  // buttons, by configuring SetRelief to Flat and OverRelief to Raised.
  // Valid constants can be found in vtkKWTkOptions::ReliefType.
  // If the value of this option is None, then no alternative relief is used
  // when the mouse cursor is over the checkbutton. 
  virtual void SetOverRelief(int);
  virtual int GetOverRelief();
  virtual void SetOverReliefToRaised() 
    { this->SetOverRelief(vtkKWTkOptions::ReliefRaised); };
  virtual void SetOverReliefToSunken() 
    { this->SetOverRelief(vtkKWTkOptions::ReliefSunken); };
  virtual void SetOverReliefToFlat() 
    { this->SetOverRelief(vtkKWTkOptions::ReliefFlat); };
  virtual void SetOverReliefToRidge() 
    { this->SetOverRelief(vtkKWTkOptions::ReliefRidge); };
  virtual void SetOverReliefToSolid() 
    { this->SetOverRelief(vtkKWTkOptions::ReliefSolid); };
  virtual void SetOverReliefToGroove() 
    { this->SetOverRelief(vtkKWTkOptions::ReliefGroove); };
  virtual void SetOverReliefToNone()
    { this->SetOverRelief(vtkKWTkOptions::ReliefUnknown); };

  // Description:
  // Specifies an image to display in the widget. Typically, if the image
  // is specified then it overrides other options that specify a bitmap or
  // textual value to display in the widget. Invoke vtkKWWidget's 
  // SetConfigurationOption("-image", imagename) to use a specific 
  // pre-existing Tk image, or call one of the following functions.
  // The SetImageToPredefinedIcon method accepts an index to one of the
  // predefined icon listed in vtkKWIcon.
  // The SetImageToPixels method sets the image using pixel data. It expects
  // a pointer to the pixels and the structure of the image, i.e. its width, 
  // height and the pixel_size (how many bytes per pixel, say 3 for RGB, or
  // 1 for grayscale). If buffer_length = 0, it is computed automatically
  // from the previous parameters. If it is not, it will most likely indicate
  // that the buffer has been encoded using base64 and/or zlib.
  // If pixel_size > 3 (i.e. RGBA), the image is blend the with background
  // color of the widget.
  virtual void SetImageToIcon(vtkKWIcon *icon);
  virtual void SetImageToPredefinedIcon(int icon_index);
  virtual void SetImageToPixels(
    const unsigned char *pixels, int width, int height, int pixel_size,
    unsigned long buffer_length = 0);
  
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
  virtual void CommandCallback();

protected:

  vtkSetStringMacro(InternalText);

  vtkKWCheckButton();
  ~vtkKWCheckButton();

  int IndicatorVisibility;

  char *InternalText;
  char *VariableName;

  virtual void Configure();

  char *Command;
  virtual void InvokeCommand();

private:
  vtkKWCheckButton(const vtkKWCheckButton&); // Not implemented
  void operator=(const vtkKWCheckButton&); // Not Implemented
};

#endif
