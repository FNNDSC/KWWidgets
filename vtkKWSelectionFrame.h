/*=========================================================================

  Module:    $RCSfile: vtkKWSelectionFrame.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWSelectionFrame - Selection Frame 
// .SECTION Description
// The selction frame is what contains a render widget.  
// It is called a "selection frame" because in its title bar, you can 
// select which render widget to display in it.

#ifndef __vtkKWSelectionFrame_h
#define __vtkKWSelectionFrame_h

#include "vtkKWCompositeWidget.h"

class vtkKWFrame;
class vtkKWLabel;
class vtkKWMenuButton;
class vtkKWPushButton;
class vtkKWToolbarSet;
class vtkKWSelectionFrameInternals;

class KWWIDGETS_EXPORT vtkKWSelectionFrame : public vtkKWCompositeWidget
{
public:
  static vtkKWSelectionFrame* New();
  vtkTypeRevisionMacro(vtkKWSelectionFrame, vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Create the widget.
  virtual void Create(vtkKWApplication *app);
  
  // Description:
  // Set/Get title
  virtual void SetTitle(const char *title);
  virtual const char* GetTitle();

  // Description:
  // Select/Deselect the window
  virtual void SetSelected(int);
  vtkGetMacro(Selected, int);
  vtkBooleanMacro(Selected, int);

  // Description:
  // Set the selection list (array of num strings) and the command
  // that will be called when a selection is made by the user. 
  // This command will be passed both the selected string and 
  // a pointer to this object.
  // The selection list is represented as a pull down menu, which
  // visibility can be set.
  virtual void SetSelectionList(int num, const char **list);
  virtual void SetSelectionListCommand(
    vtkObject *object, const char *method);
  vtkGetObjectMacro(SelectionList, vtkKWMenuButton);
  virtual void SetShowSelectionList(int);
  vtkGetMacro(ShowSelectionList, int);
  vtkBooleanMacro(ShowSelectionList, int);

  // Description:
  // Show close (button and menu entry)
  // If set, a close button is added in the top right corner,
  // and a "Close" entry is added to the end of the selection list.
  // Set the close command, called when the the close button or the menu entry
  // is selected by the user.
  // This command will be passed a pointer to this object.
  virtual void SetShowClose(int);
  vtkGetMacro(ShowClose, int);
  vtkBooleanMacro(ShowClose, int);
  virtual void SetCloseCommand(
    vtkObject *object, const char *method);
  vtkGetObjectMacro(CloseButton, vtkKWPushButton);

  // Description:
  // Show change title (menu entry)
  // If set, a "Change title" entry is added to the end of the selection list.
  // Set the command, called when the menu entry is selected by the user.
  // This command will be passed a pointer to this object.
  virtual void SetShowChangeTitle(int);
  vtkGetMacro(ShowChangeTitle, int);
  vtkBooleanMacro(ShowChangeTitle, int);
  virtual void SetChangeTitleCommand(
    vtkObject *object, const char *method);

  // Description:
  // Set the select command, called when the frame is selected by the user
  // (click in title bar).
  // the double-click command is called when the frame title is double-clicked
  // (note that it will also invoke the select command, since the first
  // click acts as a select event).
  // This command will be passed a pointer to this object.
  virtual void SetSelectCommand(vtkObject *object, const char *method);
  virtual void SetDoubleClickCommand(vtkObject *object, const char *method);

  // Description:
  // Set/Get the title foregroud/background color (in both normal and 
  // selected mode). 
  vtkGetVector3Macro(TitleColor, double);
  vtkGetVector3Macro(TitleSelectedColor, double);
  vtkGetVector3Macro(TitleBackgroundColor, double);
  vtkGetVector3Macro(TitleBackgroundSelectedColor, double);
  virtual void SetTitleColor(double r, double g, double b);
  virtual void SetTitleColor(double rgb[3])
    { this->SetTitleColor(rgb[0], rgb[1], rgb[2]); };
  virtual void SetTitleSelectedColor(double r, double g, double b);
  virtual void SetTitleSelectedColor(double rgb[3])
    { this->SetTitleSelectedColor(rgb[0], rgb[1], rgb[2]); };
  virtual void SetTitleBackgroundColor(double r, double g, double b);
  virtual void SetTitleBackgroundColor(double rgb[3])
    { this->SetTitleBackgroundColor(rgb[0], rgb[1], rgb[2]); };
  virtual void SetTitleBackgroundSelectedColor(double r, double g, double b);
  virtual void SetTitleBackgroundSelectedColor(double rgb[3])
    { this->SetTitleBackgroundSelectedColor(rgb[0], rgb[1], rgb[2]); };
  
  // Description:
  // Show/Hide the toolbar set.
  vtkGetObjectMacro(ToolbarSet, vtkKWToolbarSet);
  virtual void SetShowToolbarSet(int);
  vtkGetMacro(ShowToolbarSet, int);
  vtkBooleanMacro(ShowToolbarSet, int);

  // Description:
  // Callbacks
  virtual void CloseCallback();
  virtual void SelectionListCallback(const char *menuItem);
  virtual void SelectCallback();
  virtual void DoubleClickCallback();
  virtual void ChangeTitleCallback();
  
  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object is updated
  // and propagated to its internal parts/subwidgets. This will, for example,
  // enable/disable parts of the widget UI, enable/disable the visibility
  // of 3D widgets, etc.
  virtual void UpdateEnableState();

  // Description:
  // Access to sub-widgets
  vtkGetObjectMacro(TitleBarRightSubframe, vtkKWFrame);
  vtkGetObjectMacro(BodyFrame, vtkKWFrame);
  
protected:
  vtkKWSelectionFrame();
  ~vtkKWSelectionFrame();
  
  vtkKWFrame      *TitleBar;
  vtkKWMenuButton *SelectionList;
  vtkKWPushButton *CloseButton;
  vtkKWToolbarSet *ToolbarSet;
  vtkKWLabel      *Title;
  vtkKWFrame      *TitleBarRightSubframe;
  vtkKWFrame      *BodyFrame;

  virtual void Pack();
  virtual void Bind();
  virtual void UnBind();

  virtual int SetColor(double *color, double r, double g, double b);
  virtual void UpdateColors();
  virtual void UpdateSelectionList();

  double TitleColor[3];
  double TitleSelectedColor[3];
  double TitleBackgroundColor[3];
  double TitleBackgroundSelectedColor[3];

  char *CloseCommand;
  char *SelectionListCommand;
  char *SelectCommand;
  char *DoubleClickCommand;
  char *ChangeTitleCommand;

  int Selected;
  int ShowSelectionList;
  int ShowClose;
  int ShowChangeTitle;
  int ShowToolbarSet;

  // PIMPL Encapsulation for STL containers

  vtkKWSelectionFrameInternals *Internals;

private:
  vtkKWSelectionFrame(const vtkKWSelectionFrame&);  // Not implemented
  void operator=(const vtkKWSelectionFrame&);  // Not implemented
};

#endif

