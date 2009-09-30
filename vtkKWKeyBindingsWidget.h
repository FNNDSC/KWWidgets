/*=========================================================================

  Module:    $RCSfile: vtkKWKeyBindingsWidget.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWKeyBindingsWidget - a key bindings widget.
// .SECTION Description
// This widget can be used to display key bindings stored in a key bindings
// manager class (vtkKWKeyBindingsManager).
// .SECTION See Also
// vtkKWKeyBindingsManager

#ifndef __vtkKWKeyBindingsWidget_h
#define __vtkKWKeyBindingsWidget_h

#include "vtkKWCompositeWidget.h"

class vtkKWMultiColumnListWithScrollbars;
class vtkKWKeyBindingsWidgetInternals;
class vtkKWToolbar;
class vtkKWPushButton;
class vtkKWLoadSaveButton;
class vtkKWTextWithScrollbars;
class vtkKWFrameWithLabel;
class vtkKWKeyBindingsManager;

class KWWidgets_EXPORT vtkKWKeyBindingsWidget : public vtkKWCompositeWidget
{
public:
  static vtkKWKeyBindingsWidget* New();
  vtkTypeRevisionMacro(vtkKWKeyBindingsWidget,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Set/Get the key bindings manager.
  vtkGetObjectMacro(KeyBindingsManager,vtkKWKeyBindingsManager);
  virtual void SetKeyBindingsManager(vtkKWKeyBindingsManager *mgr);

  // Description:
  // Write all keybindings to a stream or text file (tab delimited).
  // Return 1 on success, 0 otherwise
  virtual int WriteKeyBindingsToStream(ostream& os);
  virtual int WriteKeyBindingsToFile(const char *filename);

  // Description:
  // Expose a few internals
  vtkGetObjectMacro(Toolbar, vtkKWToolbar);
  vtkGetObjectMacro(SaveButton, vtkKWLoadSaveButton);
  
  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object 
  // is updated and propagated to its internal parts/subwidgets. 
  // This will, for example, enable/disable parts of the widget UI, 
  // enable/disable the visibility of 3D widgets, etc.
  virtual void UpdateEnableState();
  virtual void Update();

  // Description:
  // Callback, do NOT use. 
  virtual void SelectionChangedCallback();
  virtual void WriteKeyBindingsToFileCallback();

protected:
  vtkKWKeyBindingsWidget();
  ~vtkKWKeyBindingsWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();
  
  // Description:
  // Member variables
  vtkKWKeyBindingsWidgetInternals *Internals;
  vtkKWMultiColumnListWithScrollbars *KeyBindingsList;
  vtkKWToolbar *Toolbar;
  vtkKWLoadSaveButton *SaveButton;
  vtkKWFrameWithLabel *DetailFrame;
  vtkKWTextWithScrollbars *DetailText;
  vtkKWKeyBindingsManager *KeyBindingsManager;

private:

  vtkKWKeyBindingsWidget(const vtkKWKeyBindingsWidget&); // Not implemented
  void operator=(const vtkKWKeyBindingsWidget&); // Not implemented
};
#endif
