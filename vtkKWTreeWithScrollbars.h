/*=========================================================================

  Module:    $RCSfile: vtkKWTreeWithScrollbars.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWTreeWithScrollbars - a vtkKWTree associated to two vtkKWScrollbar's
// .SECTION Description
// This composite widget provides horizontal and vertical scrolling
// capabilities to a core widget. They can be enabled or disabled separately.
// As a subclass of vtkKWWidgetWithScrollbars, it inherits methods to
// retrieve the scrollbars and set their visibility and layout.
// .SECTION See Also
// vtkKWWidgetWithScrollbars vtkKWTree

#ifndef __vtkKWTreeWithScrollbars_h
#define __vtkKWTreeWithScrollbars_h

#include "vtkKWWidgetWithScrollbars.h"

class vtkKWScrollbar;
class vtkKWTree;
class vtkKWPushButtonSet;

class KWWidgets_EXPORT vtkKWTreeWithScrollbars : public vtkKWWidgetWithScrollbars
{
public:
  static vtkKWTreeWithScrollbars* New();
  vtkTypeRevisionMacro(vtkKWTreeWithScrollbars,vtkKWWidgetWithScrollbars);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the vtkKWTree for which we addded the scrollbars
  virtual vtkKWTree* GetWidget();
  
  // Descrition:
  // Set/Get the visibility of the small resize buttons placed next to each
  // scrollbar. Note that they won't show if the corresponding scrollbar
  // is not visible.
  virtual void SetResizeButtonsVisibility(int arg);
  vtkBooleanMacro(ResizeButtonsVisibility, int);
  vtkGetMacro(ResizeButtonsVisibility, int);

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
  virtual void ExpandTreeVerticallyCallback();
  virtual void ShrinkTreeVerticallyCallback();
  virtual void ExpandTreeHorizontallyCallback();
  virtual void ShrinkTreeHorizontallyCallback();

protected:
  vtkKWTreeWithScrollbars();
  ~vtkKWTreeWithScrollbars();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Description:
  // Set the visibility of the resize buttons
  int ResizeButtonsVisibility;

  // Description:
  // Internal widgets
  vtkKWTree *Widget;
  vtkKWPushButtonSet *VerticalButtons;
  vtkKWPushButtonSet *HorizontalButtons;

  // Description:
  // Create scrollbars and associate the scrollbars to a widget by
  // setting up the callbacks between both instances.
  // Re-implemented to set the associated now that we know the internal widget
  virtual void CreateHorizontalScrollbar();
  virtual void CreateVerticalScrollbar();

  // Description:
  // Pack.
  virtual void Pack();

  // Description:
  // Update buttons
  virtual void UpdateButtonsVisibility();

private:
  vtkKWTreeWithScrollbars(const vtkKWTreeWithScrollbars&); // Not implemented
  void operator=(const vtkKWTreeWithScrollbars&); // Not implemented
};

#endif
