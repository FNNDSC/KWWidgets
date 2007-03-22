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
  // Remove/relocate a favorite directory.
  virtual void RemoveFavoriteDirectory(const char *fullpath);
  virtual void RelocateFavoriteDirectory(
    const char* oldpath, const char* newpath);
   
  // Description:
  // Select the directory, if it is in the favorite directories list;
  // otherwise, de-select all the directories.
  virtual void SelectFavoriteDirectory(const char *path);
  
  // Description
  // Check if the given directory has already been added
  // Return 1 for Yes; 0 for No
  virtual int HasFavoriteDirectory(const char* path);
  
  // Description
  // Check if the given directory name is already used 
  // Return 1 for Yes; 0 for No
  virtual int HasFavoriteDirectoryWithName(const char *name);

  // Description:
  // Set/Get the maximum number of favorite directories in the registry 
  virtual void SetMaximumNumberOfFavoriteDirectoriesInRegistry(int);
  vtkGetMacro(MaximumNumberOfFavoriteDirectoriesInRegistry, int);
  
  // Description:
  // Specifies commands to associate with the widget.
  // This command will be called from when the "AddFavorites" button is 
  // clicked. Since this widget does not know by itself what directory to add,
  // it is up to developpers to call 'AddFavoriteDirectory' to add a new
  // favorite directory (i.e. this is application specific).
  // The 'object' argument is the object that will have the method called on
  // it. The 'method' argument is the name of the method to be called and any
  // arguments in string form. If the object is NULL, the method is still
  // evaluated as a simple command. 
  virtual void SetFavoriteDirectoryAddingCommand(
    vtkObject *obj, const char *method);
  
  // Description:
  // Specifies commands to associate with the widget.
  // This command is called when a favorite directory is selected.
  // The 'object' argument is the object that will have the method called on
  // it. The 'method' argument is the name of the method to be called and any
  // arguments in string form. If the object is NULL, the method is still
  // evaluated as a simple command. 
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
  virtual void DirectoryClickedCallback(
    const char* path, const char* text);
  virtual void DirectoryRightClickedCallback(
    const char* path, int x, int y);
  
  // Description:
  // Callback, do NOT use. 
  // Rename directory callback from right-click context menu.
  virtual void RightClickRenameCallback(const char* path);
  
  // Description:
  // Callback, do NOT use. 
  // Launch native explorer callback from right-click context menu.
  virtual void RightClickExploreCallback(const char* path);
  
  // Description:
  // Callbacks, do NOT use
  // A dialog will popup for user confirmation of the deleting action.
  // If user confirms the action, the favorite folder will be removed from
  // the places bar of the dialog.
  virtual void RightClickRemoveFavoriteCallback(const char* path);

protected:
  vtkKWFavoriteDirectoriesFrame();
  ~vtkKWFavoriteDirectoriesFrame();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Description:
  // Create the actual frame widget and initialize
  virtual void CreateFavoriteDirectoriesFrame();
  virtual void Initialize();
     
  // Description:
  // Load/Save up to 'maximum_number' favorite dirs 
  // from/to the registry under the application's 'reg_key' and 
  // subkeys are "Place[n][type]"
  // The parameter-less methods use RegistryKey as 'reg_key' and
  // MaximumNumberOfFavoriteDirectoriesInRegistry as 'maximum_number'.
  virtual void RestoreDirectoriesListFromRegistry();
  virtual void RestoreDirectoriesListFromSystemRegistry();
  virtual void RestoreDirectoriesListFromUserRegistry(
    const char *reg_key, int maximum_number);
  virtual void WriteDirectoriesToRegistry();
  virtual void WriteDirectoriesToRegistry(
    const char *reg_key, int max_nb);
  virtual void UpdateFavoriteDirectoriesOfSystemRegistry();
  
  // Description:
  // Add the special folder from Win32 registry to the favorite dirs list,
  // such as "My Documents", "Desktop"
  virtual int AddSpecialFavoriteFolder(int csidl);
      
  // Description:
  // Add the favorite button to the favorite frame
  virtual void AddFavoriteDirectoryButton(
    const char* text, 
    const char *fullpath);    
   
  // Description:
  // Update favorite frame/buttons state
  virtual void UpdateFavoriteDirectoriesFrameAfterClick(
    const char* fullpath);

  // Description:
  // Update favorite directory entries in Registry according to the
  // MaximumNumberOfFavoriteDirectoriesInRegistry
  virtual void PruneDirectoriesInRegistry();
  
  // Description:
  // Remove a directory node from the most recent history list
  virtual void PopulateContextMenu(vtkKWMenu *menu, 
    const char* path, int enable);

  // Description:
  // Clear pointers from internal list
  virtual void ClearInternalList();

  // Description:
  // Reset favorite frame/buttons state to unselected
  virtual void ClearFavoriteState();

  // Description:
  // Set/Get the default registry key the favorite dirs are saved to or
  // loaded from.
  vtkGetStringMacro(RegistryKey);
  vtkSetStringMacro(RegistryKey);
    
  // Description:
  // Commands
  char *FavoriteDirectoryAddingCommand;
  char *FavoriteDirectorySelectedCommand;
  virtual void InvokeFavoriteDirectoryAddingCommand();
  virtual void InvokeFavoriteDirectorySelectedCommand(
    const char* path, const char* dirtext);
  
  // Description:
  // Internal class to keep track of things like favorite dir list
  vtkKWFavoriteDirectoriesFrameInternals *Internals;
  
  // Description:
  // Member variables
  vtkKWToolbar     *ToolbarFavorite;
  vtkKWFrameWithScrollbar *FavoriteButtonFrame;
  vtkKWPushButton  *FavoritesAddingButton;
  vtkKWMenu *ContextMenu;
  char *RegistryKey;
  unsigned int MaximumNumberOfFavoriteDirectoriesInRegistry;
  
private:
  vtkKWFavoriteDirectoriesFrame(const vtkKWFavoriteDirectoriesFrame&); // Not implemented
  void operator=(const vtkKWFavoriteDirectoriesFrame&); // Not implemented
};
#endif
