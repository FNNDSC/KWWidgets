/*=========================================================================

  Module:    vtkKWKeyBindingsManager.h,v

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWKeyBindingsManager - a keyboard shortcut manager.
// .SECTION Description
// This class is basically a manager that acts as a container for a set of
// key bindings.
// Any object that define a key binding can register it here. 
// This manager can be queried later on to list all key bindings, for example.
// This class does not support reassigning key bindings yet.
// .SECTION See Also
// vtkKWKeyBindingsWidget


#ifndef __vtkKWKeyBindingsManager_h
#define __vtkKWKeyBindingsManager_h

#include "vtkKWObject.h"

class vtkKWKeyBindingsManagerInternals;
class vtkKWEventMap;

class KWWidgets_EXPORT vtkKWKeyBindingsManager : public vtkKWObject
{
public:
  static vtkKWKeyBindingsManager* New();
  vtkTypeRevisionMacro(vtkKWKeyBindingsManager,vtkKWObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Add or set or remove a key binding. Setting a key binding will remove any
  // items that were previously associated to that specific binding.
  // 'target' is generally a pointer to the class that set up this binding.
  // 'binding' is the binding itself, in a Tk event form (say, <KeyPress-p>)
  // 'callback_object' and 'callback_command' define the callback associated
  //     to this binding. The 'object' argument is the object that will have 
  //     the 'command' (method) called on it. The 'method' argument is the 
  //     name of the method to be called and any arguments in string form.
  //     If the 'object' is NULL, the method is still evaluated as a 
  //     simple Tcl command. 
  //     simple Tcl command. 
  // 'context' is a string in plain English (or preferably localized) that
  // explains in which context this key binding is valid. For example: 
  // "Any 2D View", or "Any Main Window". It usually is a simple/short
  // description of the class setting the binding (i.e. the 'target'). 
  // 'description' is a string in plain English (or preferably localized) that
  // explains what that binding does. For example: "Reset the camera".
  virtual void AddKeyBinding(
    vtkObject *target, 
    const char *binding, 
    vtkObject *callback_object = NULL, 
    const char *callback_command = NULL,
    const char *context = NULL,
    const char *description = NULL);
  virtual void SetKeyBinding(
    vtkObject *target, 
    const char *binding, 
    vtkObject *callback_object = NULL, 
    const char *callback_command = NULL,
    const char *context = NULL,
    const char *description = NULL);
  virtual void RemoveKeyBinding(
    vtkObject *target, 
    const char *binding = NULL, 
    vtkObject *callback_object = NULL, 
    const char *callback_command = NULL);
  
  // Description:
  // Query the key bindings. While a little convoluted, this is the fastest
  // way to query the internal bindings: iterate over the targets, then
  // iterate over the bindings for each target, then iterate over the callback
  // objects for each binding, then iterate over the key bindings entries 
  // themselves for each callback objet; for each entry (given a target, 
  // binding, callback object and index), you can retrieve the callback
  // command, context and description.
  // See vtkKWKeyBindingsWidget.cxx for an example.
  virtual int GetNumberOfTargets();
  virtual vtkObject* GetNthTarget(int idx);
  virtual int GetNumberOfBindings(vtkObject *target);
  virtual const char* GetNthBinding(vtkObject *target, int idx);
  virtual int GetNumberOfCallbackObjects(
    vtkObject *target, const char *binding);
  virtual vtkObject* GetNthCallbackObject(
    vtkObject *target, const char *binding, int idx);
  virtual int GetNumberOfKeyBindings(
    vtkObject *target, const char *binding, vtkObject *callback_object);
  virtual const char* GetNthCallbackCommand(
    vtkObject *target, const char *binding,vtkObject *callback_object,int idx);
  virtual const char* GetNthContext(
    vtkObject *target, const char *binding,vtkObject *callback_object,int idx);
  virtual const char* GetNthDescription(
    vtkObject *target, const char *binding,vtkObject *callback_object,int idx);

  // Description:
  // Convenience method that can be used to add all the key and keysym
  // bindings found in a vtkKWEventMap.
  virtual void SetKeyBindingsFromEventMap(vtkKWEventMap *map);

  // Description:
  // Get a "pretty" representation of a binding (remove trailing <>, Key-,
  // KeyPress-, translate some keysyms, change - into +, uppercase the key).
  virtual const char* GetPrettyBinding(const char *binding);

  // Description:
  // Get a "pretty" representation of a context (if the context is a single
  // word, i.e. maybe a class name, remove the usual prefixes, and separate
  // each words).
  virtual const char* GetPrettyContext(const char *context);

protected:
  vtkKWKeyBindingsManager();
  ~vtkKWKeyBindingsManager();

  //BTX
  // PIMPL Encapsulation for STL containers
  vtkKWKeyBindingsManagerInternals *Internals;
  //ETX

  // Description:
  // Processes the events that are passed through CallbackCommand (or others).
  // Subclasses can oberride this method to process their own events, but
  // should call the superclass too.
  virtual void ProcessCallbackCommandEvents(
    vtkObject *caller, unsigned long event, void *calldata);
  
private:
  
  vtkKWKeyBindingsManager(const vtkKWKeyBindingsManager&); // Not implemented
  void operator=(const vtkKWKeyBindingsManager&); // Not implemented
};

#endif

