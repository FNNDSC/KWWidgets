/*=========================================================================

  Module:    $RCSfile: vtkKWFavoriteDirectoriesFrame.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWFavoriteDirectoriesFrame - A favorite directories frame
// .SECTION Description
// Used as part of a file browser widget, this class includes a toolbar to 
// add a favorite directory, and a frame to store all the favorite 
// directories as pushbuttons.
// .SECTION Thanks
// This work is part of the National Alliance for Medical Image
// Computing (NAMIC), funded by the National Institutes of Health
// through the NIH Roadmap for Medical Research, Grant U54 EB005149.
// Information on the National Centers for Biomedical Computing
// can be obtained from http://nihroadmap.nih.gov/bioinformatics.
// .SECTION See Also
// vtkKWFileBrowserDialog vtkKWFileBrowserWidget

#ifndef __vtkKWFavoriteDirectoriesFrame_h
#define __vtkKWFavoriteDirectoriesFrame_h

#include "vtkKWCompositeWidget.h"

class vtkKWPushButton;
class vtkKWToolbar;
class vtkKWFrameWithScrollbar;
class vtkKWFavoriteDirectoriesFrameInternals;
class vtkKWMenu;

class KWWidgets_EXPORT vtkKWFavoriteDirectoriesFrame : public vtkKWCompositeWidget
{
public:
  static vtkKWFavoriteDirectoriesFrame* New();
  vtkTypeRevisionMacro(vtkKWFavoriteDirectoriesFrame,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
   
  // Description:
  // Add a directory to the favorite directories, 
  // given the path of the directory and the name to display.
  virtual void AddFavoriteDirectory(const char *path, const char *name);

  // Description:
  // Set/Get the path or name of a favorite directory
  virtual void SetFavoriteDirectoryPath(
    const char* oldpath, const char* newpath);
  virtual void SetFavoriteDirectoryName(
    const char* oldname, const char* newname);
  
  // Description:
  // Remove/relocate a favorite directory.
  virtual void RemoveFavoriteDirectory(const char *path);
   
  // Description:
  // Select a favorite directory, if it is in the favorite directories list;
  // otherwise, de-select all the favorite directories.
  virtual void SelectFavoriteDirectory(const char *path);
  
  // Description
  // Check if the given directory has already been added
  virtual int HasFavoriteDirectory(const char* path);
  
  // Description
  // Check if the given favorite name is already used 
  virtual int HasFavoriteDirectoryWithName(const char *name);

  // Description:
  // Set/Get the maximum number of favorite directories to store in the 
  // registry.
  virtual void SetMaximumNumberOfFavoriteDirectoriesInRegistry(int);
  vtkGetMacro(MaximumNumberOfFavoriteDirectoriesInRegistry, int);
  
  // Description:
  // Specifies commands to associate with the widget.
  // This command will be called from when the "AddFavorites" button is 
  // clicked. Since this widget does not know by itself what directory to add,
  // this callback gives you the opportunity to call 'AddFavoriteDirectory' to
  // add a new favorite directory given your application context.
  // The 'object' argument is the object that will have the method called on
  // it. The 'method' argument is the name of the method to be called and any
  // arguments in string form. If the object is NULL, the method is still
  // evaluated as a simple command. 
  virtual void SetAddFavoriteDirectoryCommand(
    vtkObject *obj, const char *method);
  
  // Description:
  // Specifies commands to associate with the widget.
  // This command is called when a favorite directory is selected.
  // The 'object' argument is the object that will have the method called on
  // it. The 'method' argument is the name of the method to be called and any
  // arguments in string form. If the object is NULL, the method is still
  // evaluated as a simple command. 
  // The following parameters are also passed to the command:
  // - the path to the favorite: const char*
  // - the name of the favorite: const char*
  virtual void SetFavoriteDirectorySelectedCommand(
    vtkObject *obj, const char *method);

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
  // Callbacks, do NOT use
  // When the "Add Favorites" button is clicked, the function will 
  // be called and a dialog will popup prompt user for a name of the directory
  // that will be added to the frame. The default name is the displayed text
  // of the directory.
  virtual void AddFavoriteDirectoryCallback();

  // Description:
  // Callback, do NOT use. 
  virtual void SelectFavoriteDirectoryCallback(
    const char* path, const char*name);
  virtual void PopupFavoriteDirectoryCallback(
    const char* path, int x, int y);
  
  // Description:
  // Callback, do NOT use. 
  // Rename directory callback from right-click context menu.
  virtual void RenameFavoriteDirectoryCallback(const char* path);
  
  // Description:
  // Callback, do NOT use. 
  // Launch native explorer callback from right-click context menu.
  virtual void ExploreFavoriteDirectoryCallback(const char* path);
  
  // Description:
  // Callbacks, do NOT use
  // A dialog will popup for user confirmation of the deleting action.
  // If user confirms the action, the favorite folder will be removed from
  // the places bar of the dialog.
  virtual void RemoveFavoriteDirectoryCallback(const char* path);

protected:
  vtkKWFavoriteDirectoriesFrame();
  ~vtkKWFavoriteDirectoriesFrame();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Description:
  // Get the name of a favorite directory given its path (NULL if not found)
  virtual const char* GetNameOfFavoriteDirectory(const char *path);
  virtual vtkKWPushButton* GetButtonOfFavoriteDirectoryWithName(
    const char *name);

  // Description:
  // Load/Save up to 'maximum_number' favorite dirs 
  // from/to the registry under the application's 'reg_key'.
  // Subkeys are "Place[n][type]"
  // The parameter-less methods use RegistryKey as 'reg_key' and
  // MaximumNumberOfFavoriteDirectoriesInRegistry as 'maximum_number'.
  virtual void RestoreFavoriteDirectoriesFromRegistry();
  virtual void RestoreFavoriteDirectoriesFromSystemRegistry();
  virtual void RestoreFavoriteDirectoriesFromUserRegistry(
    const char *reg_key, int max_nb);
  virtual void WriteFavoriteDirectoriesToRegistry();
  virtual void WriteFavoriteDirectoriesToRegistry(
    const char *reg_key, int max_nb);
  virtual void WriteFavoriteDirectoriesToSystemRegistry();
  
  // Description:
  // Update favorite directory entries in Registry according to the
  // MaximumNumberOfFavoriteDirectoriesInRegistry
  virtual void PruneFavoriteDirectoriesInRegistry();
  
  // Description:
  // Add the special folders from Win32 registry to the favorite dirs list,
  // such as "My Documents", "Desktop"
  virtual int AddSpecialFavoriteFolder(int csidl);
      
  // Description:
  // Add a favorite button to the favorite frame
  virtual void AddFavoriteDirectoryToFrame(
    const char *path,
    const char *name); 
  virtual void UpdateFavoriteDirectoryButton(
    vtkKWPushButton *button, const char *path, const char *name); 

  // Description:
  // Select a favorite directory given its name
  virtual void SelectFavoriteDirectoryWithName(const char* path);

  // Description:
  // Remove a directory node from the most recent history list
  virtual void PopulateContextMenu(vtkKWMenu *menu, const char* path);

  // Description:
  // Clear pointers from internal list
  virtual void ClearInternalList();

  // Description:
  // Reset favorite frame/buttons state to unselected
  virtual void ClearFavoriteDirectorySelection();

  // Description:
  // Set/Get the default registry key the favorite dirs are saved to or
  // loaded from.
  vtkGetStringMacro(RegistryKey);
  vtkSetStringMacro(RegistryKey);
    
  // Description:
  // Commands
  char *AddFavoriteDirectoryCommand;
  char *FavoriteDirectorySelectedCommand;
  virtual void InvokeAddFavoriteDirectoryCommand();
  virtual void InvokeFavoriteDirectorySelectedCommand(
    const char* path, const char *name);
  
  // Description:
  // Internal PIMPL class for STL purposes.
  vtkKWFavoriteDirectoriesFrameInternals *Internals;
  
  // Description:
  // GUI
  vtkKWToolbar            *Toolbar;
  vtkKWFrameWithScrollbar *FavoriteDirectoryFrame;
  vtkKWPushButton         *AddFavoriteDirectoryButton;
  vtkKWMenu               *ContextMenu;

  // Description:
  // Member variables
  char *RegistryKey;
  int MaximumNumberOfFavoriteDirectoriesInRegistry;
  
private:

  vtkKWFavoriteDirectoriesFrame(const vtkKWFavoriteDirectoriesFrame&); // Not implemented
  void operator=(const vtkKWFavoriteDirectoriesFrame&); // Not implemented
};
#endif
