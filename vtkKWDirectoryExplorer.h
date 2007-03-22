/*=========================================================================

  Module:    $RCSfile: vtkKWDirectoryExplorer.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWDirectoryExplorer - a widget for exploring directories
// .SECTION Description
// This class contains a hierarchy structure to explore directories of a
// file systems. It also includes a toolbar, which offers Back/Forward
// browsing of visited directories and functions to Create/Delete directories. 
// .SECTION Thanks
// This work is part of the National Alliance for Medical Image
// Computing (NAMIC), funded by the National Institutes of Health
// through the NIH Roadmap for Medical Research, Grant U54 EB005149.
// Information on the National Centers for Biomedical Computing
// can be obtained from http://nihroadmap.nih.gov/bioinformatics.
// .SECTION See Also
// vtkKWFileBrowserDialog vtkKWFileBrowserWidget

#ifndef __vtkKWDirectoryExplorer_h
#define __vtkKWDirectoryExplorer_h

#include "vtkKWCompositeWidget.h"

class vtkKWIcon;
class vtkKWToolbar;
class vtkKWPushButton;
class vtkKWTreeWithScrollbars;
class vtkKWPushButtonWithMenu;
class vtkKWMenu;
class vtkKWDirectoryExplorerInternals;

class KWWidgets_EXPORT vtkKWDirectoryExplorer : public vtkKWCompositeWidget
{
public:
  static vtkKWDirectoryExplorer* New();
  vtkTypeRevisionMacro(vtkKWDirectoryExplorer,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Open a directory given the full path.
  // Return 1 on success, 0 otherwise
  virtual int OpenDirectory(const char* path);

  // Description:
  // Rename a directory name, given the current and new name
  // of the directory. 
  // Return 1 on success, 0 otherwise
  virtual int RenameDirectory(const char* currentname, const char* newname);
  
  // Description:
  // Remove/delete a directory.
  // Return 1 on success, 0 otherwise
  virtual int DeleteDirectory(const char* dirname);

  // Description:
  // Set/Get the selection mode
  // Valid constants can be found in vtkKWOptions::SelectionModeType.
  virtual void SetSelectionMode(int);
  virtual void SetSelectionModeToSingle();
  virtual void SetSelectionModeToMultiple();
  
  // Description:
  // Return if a directory is selected.
  // Return 1 if yes; 0, otherwise
  virtual int HasSelection();

  // Description:
  // Get/Set the selected director(ies).
  vtkGetStringMacro(SelectedDirectory);
  virtual void SetSelectedDirectory(const char*);
  int GetNumberOfSelectedDirectories();
  const char *GetNthSelectedDirectory(int i);

  // Description:
  // Set/Get the maximum number of directories that will be stored
  // in back/forward history list. 
  virtual void SetMaximumNumberOfDirectoriesInHistory(int);
  vtkGetMacro(MaximumNumberOfDirectoriesInHistory, int);

  // Description:
  // Set/Get the selection background and foreground colors.
  virtual void GetSelectionBackgroundColor(double *r, double *g, double *b);
  virtual double* GetSelectionBackgroundColor();
  virtual void SetSelectionBackgroundColor(double r, double g, double b);
  virtual void SetSelectionBackgroundColor(double rgb[3])
    { this->SetSelectionBackgroundColor(rgb[0], rgb[1], rgb[2]); };
  virtual void GetSelectionForegroundColor(double *r, double *g, double *b);
  virtual double* GetSelectionForegroundColor();
  virtual void SetSelectionForegroundColor(double r, double g, double b);
  virtual void SetSelectionForegroundColor(double rgb[3])
    { this->SetSelectionForegroundColor(rgb[0], rgb[1], rgb[2]); };
  
  // Description:
  // Set focus to the directory widget component of this widget
  // based on the visibility.
  virtual void Focus();
  virtual int HasFocus();

  // Description:
  // Add event binding to the internal component widget 
  // for the directory hierarchy, so that these events will be 
  // invoked directly from the directory component of this widget.
  virtual void AddBindingToInternalWidget(const char* event,
    vtkObject *obj, const char* method);

  // Description:
  // Specifies selection-related commands to associate with the widget.
  // 'DirectoryChanged' is called whenever the selection 
  // was changed or cleared. 
  // The 'object' argument is the object that will have the method called on
  // it. The 'method' argument is the name of the method to be called and any
  // arguments in string form. If the object is NULL, the method is still
  // evaluated as a simple command. 
  virtual void SetDirectoryChangedCommand(vtkObject *obj, const char *method);
  virtual void SetDirectoryOpenedCommand(vtkObject *obj, const char *method);
  virtual void SetDirectoryClosedCommand(vtkObject *obj, const char *method);
  virtual void SetDirectoryClickedCommand(vtkObject *obj, const char *method);
  virtual void SetDirectoryAddedCommand(vtkObject *obj, const char *method);
  virtual void SetDirectoryRemovedCommand(vtkObject *obj, const char *method);
  virtual void SetDirectoryRenamedCommand(vtkObject *obj, const char *method);

  // Description:
  // Define the event types with enumeration
  // Even though it is highly recommended to use the commands
  // framework defined above to specify the callback methods you want to be
  // invoked when specific event occur, you can also use the observer
  // framework and listen to the corresponding events/
  // Note that they are passed the same parameters as the commands, if any.
  // If more than one numerical parameter is passed, they are all stored
  // in the calldata as an array of double.
  //BTX
  enum
  {
    DirectoryChangedEvent = 10000,
    DirectoryOpenedEvent,
    DirectoryClosedEvent,
    DirectoryAddedEvent,
    DirectoryRemovedEvent,
  };
  //ETX

  // Description:
  // Callback, do NOT use. 
  // When the dropdown menu button of the Back/Forward buttons
  // is clicked, navigate to the corresponding directory is the history list.
  // -node, pointer to the unique directory node string: const char*        
  // -offsetFromCurrentIndex, offset from current directory index
  //                     in dir history list: int
  virtual void OpenDirectoryNodeCallback(const char* node, 
    int offsetFromCurrentIndex);
  
  // Description:
  // Callback, do NOT use. 
  // When the node selection is changed in the directory tree, the
  // corresponsding directory will be refreshed, loading any new files or 
  // directories underneath it.
  virtual void SingleClickOnNodeCallback(const char* node);
   
  // Description:
  // Callback, do NOT use. 
  // Right-click to launch context menu callbacks
  virtual void RightClickCallback( int x, int y, const char* node);
  
  // Description:
  // Callback, do NOT use. 
  // Rename directory node callback from right-click context menu.
  virtual int RenameCallback();
  
  // Description:
  // Callback, do NOT use. 
  // Launch native explorer callback from right-click context menu.
  virtual void RightClickExploreCallback(const char*);

  // Description:
  // Callback, do NOT use. 
  // When the Navigation keys (Up/Down, Home/End, 
  // Prior(PageUp)/Next(PageDown)) are pressed. This is used to setup
  // logic to update the new selected node after the KeyPress events.
  virtual void TreeKeyNavigationCallback();
  
  // Description:
  // Callback, do NOT use. 
  // When a node is openned in the directory tree. The corresponding
  // directory will be refreshed, loading new files and directories.
  virtual void DirectoryOpenedCallback(const char* node);

  // Description:
  // Callback, do NOT use. 
  // Callback for when a node selection is changed in the directory tree. 
  // NOTICE: The vtkKWTree is not invoking this event, if you do it
  // programatically using SelectNode or SelectSingleNode. This is why the 
  // InvokeDirectoryChangedCommand is called from other places in this class.
  virtual void DirectoryChangedCallback();

  // Description:
  // Callback, do NOT use. 
  // Callback for when a node is closed in the directory tree. 
  // Also need refreshing the contents of the corresponing directory.
  virtual void DirectoryClosedCallback(const char* node);    
   
  // Description:
  // Callback, do NOT use. 
  // When the "Create New Folder" button is clicked, the function will 
  // be called and a dialog will popup prompting the user for a name of the 
  // new directory. If user inputs a valid dir name, 
  // create the new directory, add it to the dir tree, and select the node.
  virtual void CreateNewFolderCallback();
  
  // Description:
  // Callback, do NOT use. 
  // When the "Back to previous directory" button is clicked, navigate
  // to previous directory just visisted. A history of visited directories
  // will the saved in memeory, and the max number of entries to this history
  // defaults to MAX_NUMBER_OF_DIR_IN_HISTORY.
  virtual void BackToPreviousDirectoryCallback();
   
  // Description:
  // Callback, do NOT use. 
  // When the "Go to next directory" button is clicked, navigate
  // to next directory in the history list (see description for 
  //  BackToPreviousDirectoryCallback).
  virtual void ForwardToNextDirectoryCallback();
   
  // Description:
  // Callback, do NOT use. 
  // When the "Go up one directory" button is clicked, navigate
  // to the parent directory of the current selected directory.
  virtual void GoUpDirectoryCallback();
  
  // Description:
  // Callback, do NOT use. 
  // Callbacks for when the 'Delete' key is pressed.
  // Remove selected item from dir tree/file list
  virtual int RemoveSelectedNodeCallback();

  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object 
  // is updated and propagated to its internal parts/subwidgets. 
  // This will, for example, enable/disable parts of the widget UI, 
  // enable/disable the visibility of 3D widgets, etc.
  virtual void UpdateEnableState();
  virtual void Update();

protected:
  vtkKWDirectoryExplorer();
  ~vtkKWDirectoryExplorer();

  // Description:
  // Create the widget.
  virtual void CreateWidget();
  
  // Description:
  // Create the directory tree widget, and initialize it
  virtual void CreateDirectoryExplorer();
  virtual void Initialize();
  
  // Description:
  // Load root directory. For Win32, this will load the drives;
  // for *nix, this will load root "/" directory
  virtual void LoadRootDirectory();
  
  // Description:
  // Close all the node till root level: drives for Win32; '/' for *nix
  virtual void BackToRoot();
  
  // Description:
  // When the node selection is changed in the directory tree, the
  // corresponsding directory will be refreshed, loading any new files or 
  // directories underneath it.
  virtual void SelectDirectoryNode(const char* node, int opennode=1);
     
  // Description:
  // If the directory is already loaded in the tree, navigate to 
  // and reload this input directory; if not, this function will
  // call OpenDirectory(path) to open this directory.
  // -node, pointer to the node string: const char*  (Can be NULL)
  // -path, pointer to absolute path of the directory: const char*
  // Return the node assoicated with this directory
  virtual const char* ReloadDirectory(const char* node, 
                                     const char* path);
 
  // Description:
  // Open a subdirectory of a parent tree node.
  // -parentnode, the parent node of this directory.
  // -fullname, the full path name of this sub-directory
  // -select, a flag to indicate whether to select this 
  //          sub node after open
  // Return the node opened 
  virtual void OpenSubDirectory(const char* parentnode,
                                   const char* fullname, 
                                   int select=1);
     
  // Description:
  // Reload a directory node given the parent node, and the fullpath.
  // This is used to open the sub directory node of a parent node.
  // -select, a flag to indicate whether to select this sub node after open.
  // Return the node opened 
  virtual const char* ReloadDirectory(const char* parentnode, 
                                      const char* dirname,
                                      int select);
   
  // Description:
  // This is the main funtion of finding all the directories and 
  // adding all the corresponding node to the dir tree.
  virtual void OpenDirectoryNode(const char* node, 
                                 int select = 1,
                                 int opennode = 1);
  
  // Description:
  // Open the node, including all its parent node;
  virtual void OpenWholeTree(const char* node);
  
  // Description:
  // Calling OpenDirectoryNode after centain logic to determine
  // whether to open the whole tree.
  virtual void ReloadDirectoryNode(const char* nodeID);
  
  // Description:
  // This is the actual funtion adding all the nodes to the tree,
  // called from OpenDirectoryNode.
  virtual void UpdateDirectoryNode(const char* nodeID);
  
  // Description:
  // Add node to the dir tree
  virtual void AddDirectoryNode(
    const char* parentnode, const char* node, 
    const char* text, const char* fullname, 
    vtkKWIcon *nodeicon);  
  
  // Description:
  // Get one node from a multiple selection of nodes.  An
  // error will be generated if the index is out of range.
  char *GetNthSelectedNode(int i);
  
  // Description:
  // Update directory history list given the most recent node
  virtual void UpdateMostRecentDirectoryHistory(const char* nodeID);
     
  // Description:
  // Update directory history list according to the
  // MaximumNumberOfDirectoriesInHistory
  virtual void PruneDirectoriesInHistory();

  // Description:
  // Remove a directory node from the most recent history list
  virtual void RemoveDirectoryFromHistory(const char* nodeID);

  // Description:
  // Remove a directory node from the most recent history list
  virtual void PopulateContextMenu(
    vtkKWMenu *menu, const char*, int enable);

  // Description:
  // Commands
  char *DirectoryChangedCommand;
  char *DirectoryOpenedCommand;
  char *DirectoryClickedCommand;
  char *DirectoryClosedCommand;
  char *DirectoryAddedCommand;
  char *DirectoryRemovedCommand;
  char *DirectoryRenamedCommand;
  
  // Description:
  // Invoke Commands
  virtual void InvokeDirectoryChangedCommand(const char* path);
  virtual void InvokeDirectoryOpenedCommand(const char* path);
  virtual void InvokeDirectoryClosedCommand(const char* path);
  virtual void InvokeDirectoryClickedCommand();
  virtual void InvokeDirectoryAddedCommand(const char* path);
  virtual void InvokeDirectoryRemovedCommand(const char* path);
  virtual void InvokeDirectoryRenamedCommand(const char* oldname, 
                                        const char* newname);
                                          
  // Description:
  // Internal class to keep track of things like visited dir history, etc.                          
  vtkKWDirectoryExplorerInternals *Internals;
  
  // Description:
  // Member variables
  vtkKWToolbar     *ToolbarDir;
  vtkKWTreeWithScrollbars *DirectoryTree;
  vtkKWPushButton  *FolderCreatingButton;
  vtkKWPushButtonWithMenu  *DirBackButton;
  vtkKWPushButtonWithMenu  *DirForwardButton;
  vtkKWPushButton  *DirUpButton;
  vtkKWMenu *ContextMenu;
  unsigned int  MaximumNumberOfDirectoriesInHistory;

  char* SelectedDirectory;
  
private:

  static vtkIdType IdCounter;

  vtkKWDirectoryExplorer(const vtkKWDirectoryExplorer&); // Not implemented
  void operator=(const vtkKWDirectoryExplorer&); // Not implemented
};
#endif
