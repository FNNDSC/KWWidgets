/*=========================================================================

  Module:    $RCSfile: vtkKWMenuButton.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWMenuButton - an option menu widget
// .SECTION Description
// A widget that looks like a button but when pressed provides a list
// of options that the user can select.

#ifndef __vtkKWMenuButton_h
#define __vtkKWMenuButton_h

#include "vtkKWCoreWidget.h"

class vtkKWApplication;
class vtkKWMenu;
class vtkKWIcon;

class KWWIDGETS_EXPORT vtkKWMenuButton : public vtkKWCoreWidget
{
public:
  static vtkKWMenuButton* New();
  vtkTypeRevisionMacro(vtkKWMenuButton,vtkKWCoreWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create the widget.
  virtual void Create(vtkKWApplication *app);

  // Description:
  // Set/Get the current entry of this option menu.
  // This can be an image name if any entry in the menu uses an image
  // instead of a label.
  virtual const char *GetValue();
  virtual void SetValue(const char *name);

  // Description:
  // Get the menu object
  vtkGetObjectMacro(Menu, vtkKWMenu);

  // Description:
  // Add/Insert radiobutton entries to the internal menu. The following
  // methods are just convenience calls to the internal vtkKWMenu's
  // AddRadioButton methods, but we make sure all radiobuttons share
  // the same internal variable reference so that they behave as if
  // they were part of the same group.
  virtual void AddRadioButton(const char *label);
  virtual void AddRadioButton(
    const char *label, vtkObject *obj, const char *method, 
    const char *help = 0);
  virtual void AddRadioButtonImage(
    const char *image_name, vtkObject *obj, const char *method, 
    const char *help = 0);

  // Description
  // Set the indicator On/Off. To be called after creation.
  void IndicatorOn();
  void IndicatorOff();

  // Description:
  // Convenience method to set the button width (in chars if text, 
  // in pixels if image).
  void SetWidth(int width);
  
  // Description:
  // Set/Get the maximum width of the option menu label
  // This does not modify the internal value, this is just for display
  // purposes: the option menu button can therefore be automatically
  // shrinked, while the menu associated to it will display all entries
  // correctly.
  // Set width to 0 (default) to prevent auto-cropping.
  virtual void SetMaximumLabelWidth(int);
  vtkGetMacro(MaximumLabelWidth, int);

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
  // Callbacks (don't call)
  virtual void TracedVariableChangedCallback(
    const char *, const char *, const char *);

protected:
  vtkKWMenuButton();
  ~vtkKWMenuButton();

  vtkGetStringMacro(CurrentValue);
  vtkSetStringMacro(CurrentValue);

  char      *CurrentValue;  
  vtkKWMenu *Menu;
  int       MaximumLabelWidth;

  virtual void UpdateOptionMenuLabel();

private:
  vtkKWMenuButton(const vtkKWMenuButton&); // Not implemented
  void operator=(const vtkKWMenuButton&); // Not implemented
};


#endif



