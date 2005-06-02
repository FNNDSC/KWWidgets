/*=========================================================================

  Module:    $RCSfile: vtkKWTopLevel.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWTopLevel - toplevel superclass
// .SECTION Description
// A generic superclass for toplevel.

#ifndef __vtkKWTopLevel_h
#define __vtkKWTopLevel_h

#include "vtkKWWidget.h"

class vtkKWMenu;

class KWWIDGETS_EXPORT vtkKWTopLevel : public vtkKWWidget
{
public:
  static vtkKWTopLevel* New();
  vtkTypeRevisionMacro(vtkKWTopLevel,vtkKWWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Class of the window. Used to group several windows under the same class
  // (they will, for example, de-iconify together).
  // Make sure you set it before a call to Create().
  vtkSetStringMacro(WindowClass);
  vtkGetStringMacro(WindowClass);

  // Description:
  // Set the widget/window to which this toplevel will be slave.
  // If set, this toplevel will always be on top of the master
  // window and will minimize with it (assuming that the windowing
  // system supports this).
  // For convenience purposes, the MasterWindow does not have to be a
  // toplevel, it can be a plain widget (its toplevel will be found
  // at runtime).
  virtual void SetMasterWindow(vtkKWWidget* win);
  vtkGetObjectMacro(MasterWindow, vtkKWWidget);

  // Description:
  // Create the widget.
  // Make sure WindowClass is set before calling this method (if needed).
  // If MasterWindow is set and is a vtkKWTopLevel, its class will be used
  // to set our own WindowClass.
  // Withdraw() is called at the end of the creation.
  virtual void Create(vtkKWApplication *app, const char *args);

  // Description:
  // Display the toplevel. Hide it with the Withdraw() method.
  // This also call DeIconify(), Focus() and Raise()
  virtual void Display();

  // Description:
  // Arranges for window to be withdrawn from the screen. This causes the
  // window to be unmapped and forgotten about by the window manager.
  virtual void Withdraw();

  // Description:
  // Inform the window manager that this toplevel should be modal.
  // If it is set, Display() will bring up the toplevel and grab it.
  // Withdraw() will bring down the toplevel, and release the grab.
  vtkSetClampMacro(Modal, int, 0, 1);
  vtkBooleanMacro(Modal, int);
  vtkGetMacro(Modal, int);

  // Description:
  // Arrange for the toplevel to be displayed in normal (non-iconified) form.
  // This is done by mapping the window.
  virtual void DeIconify();

  // Description:
  // Arranges for window to be displayed above all of its siblings in the
  // stacking order.
  virtual void Raise();

  // Description:
  // Set the title of the toplevel.
  virtual void SetTitle(const char *);
  vtkGetStringMacro(Title);

  // Description:
  // Convenience method to set/get the window position in screen pixel
  // coordinates. No effect if called before Create()
  // Return 1 on success, 0 otherwise.
  virtual int SetPosition(int x, int y);
  virtual int GetPosition(int *x, int *y);

  // Description:
  // Convenience method to set/get the window size in pixels 
  // No effect if called before Create()
  // This will in turn call GetWidget() and GetHeight()
  // Return 1 on success, 0 otherwise.
  virtual int SetSize(int w, int h);
  virtual int GetSize(int *w, int *h);

  // Description:
  // Convenience method to guess the width/height of the toplevel.
  virtual int GetWidth();
  virtual int GetHeight();

  // Description:
  // Convenience method to set/get the minimum window size. 
  // For gridded windows the dimensions are specified in grid units; 
  // otherwise they are specified in pixel units. The window manager will
  // restrict the window's dimensions to be greater than or equal to width
  // and height.
  // No effect if called before Create()
  // Return 1 on success, 0 otherwise.
  virtual int SetMinimumSize(int w, int h);
  virtual int GetMinimumSize(int *w, int *h);

  // Description:
  // Convenience method to set/get the window size and position in screen pixel
  // coordinates as a geometry format wxh+x+y (ex: 800x700+20+50). 
  // No effect if called before Create()
  // SetGeometry will return 1 on success, 0 otherwise.
  // GetGeometry will return the geometry in a temporary buffer on success
  // (copy the value to another string buffer as soon as possible), or NULL
  // otherwise
  virtual int SetGeometry(const char *);
  virtual const char* GetGeometry();

  // Description:
  // Return if the toplevel has ever been mapped (deiconified)
  vtkGetMacro(HasBeenMapped, int);

  // Description:
  // Set/Get if the toplevel should be displayed without decorations (i.e.
  // ignored by the window manager). Default to 0. If not decorated, the
  // toplevel will usually be displayed without a title bar, resizing handles,
  // etc.
  virtual void SetHideDecoration(int);
  vtkGetMacro(HideDecoration, int);
  vtkBooleanMacro(HideDecoration, int);

  // Description:
  // Get the menu associated to this toplevel.
  // Note that this menu is created on the fly to lower the footprint
  // of this object. 
  vtkKWMenu *GetMenu();

  // Description:
  // Convenience method to set the command used to manage window manage 
  // protocol WM_DELETE_WINDOW. Called when the user closes the window
  // using the window manager.
  // The first argument is the object that will have the method called on it.
  // The second is the name of the method to be called and any arguments
  // in string form.
  virtual void SetDeleteWindowProtocolCommand(
    vtkKWObject *obj, const char *command);

  // Description:
  // Convenience method to set the name inside the icon associated to this
  // window/toplevel.
  virtual void SetIconName(const char *name);

  // Description:
  // Set whether or not the user may interactively resize the toplevel window.
  // The parameters are boolean values that determine whether the width and
  // height of the window may be modified by the user.
  // No effect if called before Create()
  virtual void SetResizable(int w, int h);

  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object is updated
  // and propagated to its internal parts/subwidgets. This will, for example,
  // enable/disable parts of the widget UI, enable/disable the visibility
  // of 3D widgets, etc.
  virtual void UpdateEnableState();

protected:

  vtkKWTopLevel();
  ~vtkKWTopLevel();

  vtkKWWidget *MasterWindow;
  vtkKWMenu   *Menu;

  char *Title;
  char *WindowClass;

  int HasBeenMapped;
  int HideDecoration;
  int Modal;

  // Description:
  // Convenience method to get the width/height of the toplevel as requested
  // by the window manager. Not exposed in public since it is so Tk
  // related. Is is usually used to get the geometry of a window before
  // it is mapped to screen, as requested by the geometry manager.
  virtual int GetRequestedWidth();
  virtual int GetRequestedHeight();

private:
  vtkKWTopLevel(const vtkKWTopLevel&); // Not implemented
  void operator=(const vtkKWTopLevel&); // Not Implemented
};

#endif
