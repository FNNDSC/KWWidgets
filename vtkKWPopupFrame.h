/*=========================================================================

  Module:    $RCSfile: vtkKWPopupFrame.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWPopupFrame - a popup frame
// .SECTION Description
// A class that provides a frame that can be collapsed as a popup button.

#ifndef __vtkKWPopupFrame_h
#define __vtkKWPopupFrame_h

#include "vtkKWCompositeWidget.h"

class vtkKWFrameWithLabel;
class vtkKWPopupButton;

class KWWidgets_EXPORT vtkKWPopupFrame : public vtkKWCompositeWidget
{
public:
  static vtkKWPopupFrame* New();
  vtkTypeRevisionMacro(vtkKWPopupFrame,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Display the frame as a popup. This has to be called before Create().
  vtkSetMacro(PopupMode, int);
  vtkGetMacro(PopupMode, int);
  vtkBooleanMacro(PopupMode, int);
  
  // Description:
  // Access to sub-widgets
  vtkGetObjectMacro(PopupButton, vtkKWPopupButton);
  vtkGetObjectMacro(Frame, vtkKWFrameWithLabel);

  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object is updated
  // and propagated to its internal parts/subwidgets. This will, for example,
  // enable/disable parts of the widget UI, enable/disable the visibility
  // of 3D widgets, etc.
  virtual void UpdateEnableState();

protected:
  vtkKWPopupFrame();
  ~vtkKWPopupFrame();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // GUI

  int                     PopupMode;

  vtkKWPopupButton        *PopupButton;
  vtkKWFrameWithLabel       *Frame;

private:
  vtkKWPopupFrame(const vtkKWPopupFrame&); // Not implemented
  void operator=(const vtkKWPopupFrame&); // Not Implemented
};

#endif
