/*=========================================================================

  Module:    $RCSfile: vtkKWScalarComponentSelectionWidget.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWScalarComponentSelectionWidget - a scalar component selection widget
// .SECTION Description
// This class contains the UI for scalar component selection.

#ifndef __vtkKWScalarComponentSelectionWidget_h
#define __vtkKWScalarComponentSelectionWidget_h

#include "vtkKWCompositeWidget.h"

class vtkKWMenuButtonWithLabel;

class KWWIDGETS_EXPORT vtkKWScalarComponentSelectionWidget : public vtkKWCompositeWidget
{
public:
  static vtkKWScalarComponentSelectionWidget* New();
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeRevisionMacro(vtkKWScalarComponentSelectionWidget,vtkKWCompositeWidget);

  // Description:
  // Create the widget.
  virtual void Create();

  // Description:
  // Are the components independent of each other?
  virtual void SetIndependentComponents(int);
  vtkGetMacro(IndependentComponents, int);
  vtkBooleanMacro(IndependentComponents, int);
  
  // Description:
  // Set/get the number of components controlled by the widget
  virtual void SetNumberOfComponents(int);
  vtkGetMacro(NumberOfComponents, int);

  // Description:
  // Set/get the current component controlled by the widget (if controllable)
  virtual void SetSelectedComponent(int);
  vtkGetMacro(SelectedComponent, int);

  // Description:
  // Allow component selection (a quick way to hide the UI)
  virtual void SetAllowComponentSelection(int);
  vtkBooleanMacro(AllowComponentSelection, int);
  vtkGetMacro(AllowComponentSelection, int);

  // Description:
  // Update the whole UI depending on the value of the Ivars
  virtual void Update();

  // Description:
  // Specifies a command to associate with the widget. This command is 
  // typically invoked when the selected component is changed.
  // The first argument is the object that will have the method called on it.
  // The second argument is the name of the method to be called and any
  // arguments in string form. If the object is NULL, the method
  // is still evaluated as a simple command. 
  // Note that the selected component is passed as a parameter.
  virtual void SetSelectedComponentChangedCommand(
    vtkObject *object, const char *method);

  // Description:
  // Callbacks
  virtual void SelectedComponentCallback(int);

  // Description:
  // Access to objects
  vtkGetObjectMacro(SelectedComponentOptionMenu, vtkKWMenuButtonWithLabel);
 
  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object is updated
  // and propagated to its internal parts/subwidgets. This will, for example,
  // enable/disable parts of the widget UI, enable/disable the visibility
  // of 3D widgets, etc.
  virtual void UpdateEnableState();

protected:
  vtkKWScalarComponentSelectionWidget();
  ~vtkKWScalarComponentSelectionWidget();

  int IndependentComponents;
  int NumberOfComponents;
  int SelectedComponent;
  int AllowComponentSelection;

  char  *SelectedComponentChangedCommand;
  virtual void InvokeSelectedComponentChangedCommand();

  // GUI

  vtkKWMenuButtonWithLabel *SelectedComponentOptionMenu;

  // Pack
  virtual void Pack();

private:
  vtkKWScalarComponentSelectionWidget(const vtkKWScalarComponentSelectionWidget&); // Not implemented
  void operator=(const vtkKWScalarComponentSelectionWidget&); // Not implemented
};

#endif
