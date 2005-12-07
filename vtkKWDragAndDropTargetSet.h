/*=========================================================================

  Module:    $RCSfile: vtkKWDragAndDropTargetSet.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWDragAndDropTargetSet - a set of drag and drop targets
// .SECTION Description
// This class is a container for a set of drag and drop targets.
// It provides methods to set drag and drop callbacks between a *single*
// source and one or more targets. 
// This is *not* a general drag and drop framework, where DnD types would
// be created and objects would register themselves as source and/or sinks
// for any of those types. This is a simpler framework to perform specific
// DnD actions between a know source and known targets.
// An instance of this class is available in vtkKWWidget and can be used
// to register DnD actions between that vtkKWWidget source and some targets.
// .SECTION See Also
// vtkKWWidget

#ifndef __vtkKWDragAndDropTargetSet_h
#define __vtkKWDragAndDropTargetSet_h

#include "vtkKWObject.h"

class vtkKWWidget;
class vtkKWDragAndDropTargetSetInternals;

class KWWidgets_EXPORT vtkKWDragAndDropTargetSet : public vtkKWObject
{
public:
  static vtkKWDragAndDropTargetSet* New();
  vtkTypeRevisionMacro(vtkKWDragAndDropTargetSet,vtkKWObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Enable/disable Drag and Drop between all the source and targets.
  virtual void SetEnable(int);
  vtkBooleanMacro(Enable, int);
  vtkGetMacro(Enable, int);

  // Description:
  // Set/Get the Drag and Drop source widget. This is the widget that the
  // the Drag and Drop originates from.
  // Make sure you have called SetApplication(..) on the instance before.
  virtual void SetSource(vtkKWWidget*);
  vtkGetObjectMacro(Source, vtkKWWidget);

  // Description:
  // Set/Get the optional Drag and Drop source anchor widget. This is the 
  // actual widget that the user drags and drops (i.e. the actual widget the
  // user starts the interaction with). It NULL, it defaults to the
  // Source widget automatically. The anchor widget is a convenient way to 
  // specify that a sub-part of the Source widget is to be dragged & dropped, 
  // not the whole Source widget. 
  // Make sure you have called SetApplication(..) on the instance before.
  virtual void SetSourceAnchor(vtkKWWidget*);
  vtkGetObjectMacro(SourceAnchor, vtkKWWidget);

  // Description:
  // Add/Query/Remove a Drag & Drop target. 
  virtual int AddTarget(vtkKWWidget *target);
  virtual int RemoveTarget(vtkKWWidget *target);
  virtual int HasTarget(vtkKWWidget *target);
  virtual int GetNumberOfTargets();

  // Description:
  // Set a Drag & Drop target callbacks/commands.
  // If the target has not been added yet, it will be added automatically.
  // The StartCommand of all targets is called when Drag & Drop starts.
  // The PerformCommand of all targets is called while Drag & Drop is 
  // performed.
  // The EndCommand of all targets that contain the drop coordinates is called
  // when Drag & Drop ends.
  // The 'object' argument is the object that will have the method called on
  // it. The 'method' argument is the name of the method to be called and any
  // arguments in string form. If the object is NULL, the method is still
  // evaluated as a simple command. 
  // The following parameters are also passed to the commands:
  // - absolute/screen (x,y) mouse coordinates: int, int
  // - the Source widget: vtkKWWidget*
  // - the SourceAnchor widget: vtkKWWidget* (same as Source widget most of
  // the times)
  // Additionally, EndCommand is passed a 5th parameter:
  // - the target: vtkKWWidget*
  virtual int SetTargetStartCommand(
    vtkKWWidget *target, vtkObject *object, const char *method);
  virtual int SetTargetPerformCommand(
    vtkKWWidget *target, vtkObject *object, const char *method);
  virtual int SetTargetEndCommand(
    vtkKWWidget *target, vtkObject *object, const char *method);

  // Description:
  // Independent of any targets, the following commands can be set:
  // The StartCommand is called when Drag & Drop starts.
  // The PerformCommand is called while Drag & Drop is performed.
  // The EndCommand is called when Drag & Drop ends.
  // The 'object' argument is the object that will have the method called on
  // it. The 'method' argument is the name of the method to be called and any
  // arguments in string form. If the object is NULL, the method is still
  // evaluated as a simple command. 
  // The following parameters are also passed to the commands:
  // - absolute/screen (x,y) mouse coordinates: int, int
  // None of them is called if Enable is false.
  // StartCommand and PerformCommand are called *before* any target's
  // corresponding command. EndCommand is called *after* any target's
  // EndCommand.
  virtual void SetStartCommand(vtkObject *object, const char *method);
  virtual void SetPerformCommand(vtkObject *object, const char *method);
  virtual void SetEndCommand(vtkObject *object, const char *method);

  // Description:
  // Callbacks. Internal, do not use.
  virtual void StartCallback(int x, int y);
  virtual void PerformCallback(int x, int y);
  virtual void EndCallback(int x, int y);

protected:
  vtkKWDragAndDropTargetSet();
  ~vtkKWDragAndDropTargetSet();

  // Drag and Drop

  //BTX

  class TargetSlot
  {
  public:
    vtkKWWidget *Target;
    char *StartCommand;
    char *PerformCommand;
    char *EndCommand;

    void SetStartCommand(const char*);
    void SetEndCommand(const char*);
    void SetPerformCommand(const char*);

    TargetSlot();
    ~TargetSlot();
  };

  // PIMPL Encapsulation for STL containers

  vtkKWDragAndDropTargetSetInternals *Internals;
  friend class vtkKWDragAndDropTargetSetInternals;

  TargetSlot* GetTarget(vtkKWWidget *target);

  //ETX

  int Enable;

  vtkKWWidget *SourceAnchor;
  vtkKWWidget *Source;

  char *StartCommand;
  char *PerformCommand;
  char *EndCommand;
  virtual void InvokeCommandWithCoordinates(const char *command, int x, int y);
  virtual void InvokeStartCommand(int x, int y);
  virtual void InvokePerformCommand(int x, int y);
  virtual void InvokeEndCommand(int x, int y);

  virtual void AddBindings();
  virtual void RemoveBindings();
  virtual void DeleteAllTargets();

private:
  
  vtkKWDragAndDropTargetSet(const vtkKWDragAndDropTargetSet&); // Not implemented
  void operator=(const vtkKWDragAndDropTargetSet&); // Not implemented
};

#endif
