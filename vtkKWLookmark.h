/*=========================================================================

  Program:   ParaView
  Module:    $RCSfile: vtkKWLookmark.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*----------------------------------------------------------------------------
 Copyright (c) Sandia Corporation
 See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.
----------------------------------------------------------------------------*/


// .NAME vtkKWLookmark - An interface widget for a lookmark.
// .SECTION Description
//
// This is the interface widget for lookmarks. It contains a name
// 
// .SECTION See Also
// vtkKWLookmarkContainer vtkPVLookmarkManager vtkPVLookmark

#ifndef __vtkKWLookmark_h
#define __vtkKWLookmark_h

#include "vtkKWWidget.h"

class vtkKWLabeledCheckButton;
class vtkKWApplication;
class vtkKWFrame;
class vtkKWLabel;
class vtkKWLabeledFrame;
class vtkKWCheckButton;
class vtkPVCameraIcon;
class vtkKWText;
class vtkKWLabeledCheckButton;
class vtkKWRadioButtonSet;

// vtkSetObjectMacro seems to require that this be included - is there a workaround?
//#include "vtkPVLookmark.h"

class VTK_EXPORT vtkKWLookmark : public vtkKWWidget
{
public:
  static vtkKWLookmark* New();
  vtkTypeRevisionMacro(vtkKWLookmark,vtkKWWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create a Tk widget
  virtual void Create(vtkKWApplication *app);

  // Description: 
  // Methods that manipulate and query the underlying vtkKWWidgets that make the lookmark widget. 
  // Would it be better to just provide other classes full access to the underlying widgets since
  // do so for a few of them in the following section?
  void SetLookmarkName(char *lmkName);
  char *GetLookmarkName();
  char *GetComments();
  void SetComments(char *);
  void SetDataset(char *);
  // In a sense convert from a vtkKWIcon to a vtkPVCameraIcon:
  void SetLookmarkImage(vtkKWIcon* icon);
  void SetSelectionState(int state);
  int GetSelectionState();

  vtkGetObjectMacro(LmkMainFrame,vtkKWLabeledFrame);
  vtkGetObjectMacro(SeparatorFrame,vtkKWFrame);

//  void DragAndDropStartCallback(int x, int y);
//  void DragAndDropEndCallback(int x, int y);

  // Set/Get methods called from vtkPVLookmarkManager
  vtkGetObjectMacro(LmkIcon,vtkPVCameraIcon);
  // needed in vtkPVLookmarkManager to set turn it enabled/disabled

  // Description:
  // The value represents this lookmark widget's packing location among sibling lmk widgets and lmk containers.
  // Used for moving widget.
  vtkGetMacro(Location,int);
  vtkSetMacro(Location,int);

  // Description:
  // If TRUE, use the default dataset when loading lookmark, else use the currently selected one in source list
  int IsLockedToDataset();

  // Description:
  // Each vtkKWLookmark object must know about its vtkPVLookmark counterpart for when a lookmark is renamed
  // Its the cleanest way I could think of telling the vtkPVLookmark object about the renaming.
//  vtkGetObjectMacro(Lookmark,vtkPVLookmark);
//  vtkSetObjectMacro(Lookmark,vtkPVLookmark);

  // Description: 
  // Convenience method for deleting the widget
  void RemoveLookmark();
  
  // Description:
  // Callback to double-clicking the lookmark widget label. Pressing 'Return' then calls ChangeLookmarkName
  void EditLookmarkCallback();
  void ChangeLookmarkName();

  // Description:
  // COnvenience method for encapsulating and reusing code that packs this widget
  // the argument tells Pack() whether to display the hidden widgets or not
  void Pack();

  void DragAndDropPerformCommand(int x, int y, vtkKWWidget *widget, vtkKWWidget *anchor);
  void RemoveDragAndDropTargetCues();

  virtual void UpdateEnableState();

protected:

  vtkKWLookmark();
  ~vtkKWLookmark();

  vtkKWFrame *LmkLeftFrame;
  vtkKWFrame *LmkRightFrame;
  vtkKWFrame *LmkFrame;
  vtkKWLabeledFrame *LmkMainFrame;
  vtkKWLabeledFrame *LmkCommentsFrame;
  vtkKWLabel *LmkDatasetLabel;
  vtkKWLabeledCheckButton *LmkDatasetCheckbox;
  vtkKWFrame *LmkDatasetFrame;
  vtkPVCameraIcon *LmkIcon;
  vtkKWText *LmkCommentsText;
  vtkKWText *LmkNameField;
  vtkKWFrame *SeparatorFrame;
  vtkKWCheckButton *Checkbox;
  vtkKWRadioButtonSet *DatasetOption;

  int Location;

  char *Dataset;
  int SelectionFlag;

//  vtkPVLookmark *Lookmark;

private:
  vtkKWLookmark(const vtkKWLookmark&); // Not implemented
  void operator=(const vtkKWLookmark&); // Not implemented
};

#endif
