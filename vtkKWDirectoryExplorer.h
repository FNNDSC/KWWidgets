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
  // Return if one (or more) directory is/are selected.
  virtual int HasSelection();

  // Description:
  // Get/Set the selected director(ies).
  virtual const char *GetSelectedDirectory()
    {return GetNthSelectedDirectory(0);}
  virtual int GetNumberOfSelectedDirectories();
  virtual const char *GetNthSelectedDirectory(int i);
  virtual void SelectDirectory(const char*);
  virtual void DeselectDirectory(const char*);
  virtual void ClearSelection();

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
  // Convenience method to Set/Get the width (in chars) and height (in lines)
  // of the tree widget.
  virtual void SetTreeWidth(int);
  virtual int GetTreeWidth();
  virtual void SetTreeHeight(int);
  virtual int GetTreeHeight();

  // Description:
  // Convenience method to Set/Get the tree background color.
  virtual void GetTreeBackgroundColor(double *r, double *g, double *b);
  virtual double* GetTreeBackgroundColor();
  virtual void SetTreeBackgroundColor(double r, double g, double b);
  virtual void SetTreeBackgroundColor(double rgb[3])
    { this->SetTreeBackgroundColor(rgb[0], rgb[1], rgb[2]); };

  // Description:
  // Specifies selection-related commands to associate with the widget.
  // DirectorySelected is called whenever the selection is changed or cleared. 
  // The 'object' argument is the object that will have the method called on
  // it. The 'method' argument is the name of the method to be called and any
  // arguments in string form. If the object is NULL, the method is still
  // evaluated as a simple command. 
  // - DirectorySelected is passed the first selected directory (const char *).
  // - DirectoryOpened and DirectoryClosed are passed the directory being 
  //   opened or closed (const char *).
  // - DirectoryAdded and DirectoryDeleted are passed the directory that was
  //   added or removed (const char *).
  // - DirectoryRenamed is passed the preview and new name (path) of the
  //   directory that was renamed (const char *, const char *).
  virtual void SetDirectorySelectedCommand(vtkObject *obj, const char *method);
  virtual void SetDirectoryOpenedCommand(vtkObject *obj, const char *method);
  virtual void SetDirectoryClosedCommand(vtkObject *obj, const char *method);
  virtual void SetDirectoryCreatedCommand(vtkObject *obj, const char *method);
  virtual void SetDirectoryDeletedCommand(vtkObject *obj, const char *method);
  virtual void SetDirectoryRenamedCommand(vtkObject *obj, const char *method);

  // Description:
  // Events are passed the same parameters as the commands, if any.
  //BTX
  enum
  {
    DirectorySelectedEvent = 10000,
    DirectoryOpenedEvent,
    DirectoryClosedEvent,
    DirectoryCreatedEvent,
    DirectoryDeletedEvent
  };
  //ETX

  // Description:
  // Set focus to the directory widget component of this widget.
  virtual void Focus();
  virtual int HasFocus();

  // Description:
  // Get the toolbar object.
  vtkGetObjectMacro(Toolbar, vtkKWToolbar);

  // Description:
  // Get the button objects.
  // DO NOT modify the callbacks, these accessors are provided to change
  // the button icons, for example.
  vtkGetObjectMacro(CreateFolderButton, vtkKWPushButton);
  vtkGetObjectMacro(BackButton, vtkKWPushButtonWithMenu);
  vtkGetObjectMacro(ForwardButton, vtkKWPushButtonWithMenu);
  vtkGetObjectMacro(UpButton, vtkKWPushButton);

  // Description:
  // Add event binding to the internal component widget 
  // for the directory hierarchy, so that these events will be 
  // invoked directly from the directory component of this widget.
  virtual void AddBindingToInternalWidget(
    const char* event, vtkObject *obj, const char* method);

  // Description:
  // Callback, do NOT use. 
  // When the dropdown menu button of the Back/Forward buttons
  // is clicked, navigate to the corresponding directory is the history list.
  // 'node' is a pointer to the unique directory node string,
  // 'offsetFromCurrentIndex' is the offset from current directory index in 
  //  history list.
  virtual void OpenDirectoryNodeCallback(
    const char* node, int offsetFromCurrentIndex);
  
  // Description:
  // Callback, do NOT use. 
  // When the node selection is changed in the directory tree, the
  // corresponding directory is refreshed, loading any new files or 
  // directories underneath it.
  virtual void SingleClickOnNodeCallback(const char* node);
   
  // Description:
  // Callback, do NOT use. 
  // Right-click to popup context menu.
  virtual void RightClickCallback( int x, int y, const char* node);
  
  // Description:
  // Callback, do NOT use. 
  // Rename directory node from right-click context menu.
  virtual int RenameCallback();
  
  // Description:
  // Callback, do NOT use. 
  // Launch native explorer from right-click context menu.
  virtual void RightClickExploreCallback(const char*);

  // Description:
  // Callback, do NOT use. 
  // When the Navigation keys (Up/Down, Home/End, 
  // Prior(PageUp)/Next(PageDown)) are pressed. This is used to setup
  // logic to update the new selected node after the KeyPress events.
  virtual void TreeKeyNavigationCallback();
  
  // Description:
  // Callback, do NOT use. 
  // When a node is opened in the directory tree, the corresponding
  // directory is refreshed, loading new files and directories.
  virtual void DirectoryOpenedCallback(const char* node);

  // Description:
  // Callback, do NOT use. 
  // When a node selection is changed in the directory tree. 
  virtual void DirectorySelectedCallback();

  // Description:
  // Callback, do NOT use. 
  // When a node is closed in the directory tree. 
  // Also need refreshing the contents of the corresponding directory.
  virtual void DirectoryClosedCallback(const char* node);    
   
  // Description:
  // Callback, do NOT use. 
  // When the "Create New Folder" button is clicked, a dialog is displayed
  // prompting the user for the name of the new directory. If the user inputs
  // a valid dir name, create the new directory, add it to the dir tree, and
  // select the node.
  virtual void CreateNewFolderCallback();
  
  // Description:
  // Callback, do NOT use. 
  // When the "Back to previous directory" button is clicked, navigate
  // to the previously visited directory. A history of visited directories
  // is saved in memory.
  virtual void BackToPreviousDirectoryCallback();
   
  // Description:
  // Callback, do NOT use. 
  // When the "Go to next directory" button is clicked, navigate
  // to next directory in the history list.
  virtual void ForwardToNextDirectoryCallback();
   
  // Description:
  // Callback, do NOT use. 
  // When the "Go up one directory" button is clicked, navigate
  // to the parent directory of the currently selected directory.
  virtual void GoUpDirectoryCallback();
  
  // Description:
  // Callback, do NOT use. 
  // When the 'Delete' key is pressed, remove the selected item from the dir
  // tree/file list.
  virtual int RemoveSelectedNodeCallback();

  // Description:
  // Callback, do NOT use. 
  virtual void LoadRootDirectoryCallback();

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
  // Load root directory. For Win32, this will load the drives;
  // for *nix, this will load root "/" directory
  virtual void LoadRootDirectory();
  
  // Description:
  // Close all the nodes up to root level: drives for Win32; '/' for *nix
  virtual void BackToRoot();
  
  // Description:
  // When the node selection is changed in the directory tree, the
  // corresponding directory is refreshed, loading any new files or 
  // directories underneath it.
  virtual void SelectDirectoryNode(const char* node, int opennode = 1);
     
  // Description:
  // If the directory is already loaded in the tree, navigate to 
  // and reload this input directory; if not, this function will
  // call OpenDirectory(path) to open this directory.
  // 'node' is a pointer to the node string (can be NULL), 'path' a pointer 
  // to the absolute path to the directory.
  // Return the node associated with this directory.
  virtual const char* ReloadDirectory(const char* node, const char* path);
 
  // Description:
  // Reload a directory node given the parent node, and the fullpath.
  // This is used to open the sub directory node of a parent node.
  // 'select' is a flag to indicate whether to select this sub node after 
  // opening it.
  // Return the node opened.
  virtual const char* ReloadDirectory(const char* parentnode, 
                                      const char* dirname,
                                      int select);
   
  // Description:
  // Open a subdirectory of a parent tree node.
  // 'parentnode' is the parent node of this directory, 'fullname' is the full
  // path name of this sub-directory, 'select' a flag to indicate whether to
  // select this sub node after opening it.
  virtual void OpenSubDirectory(const char* parentnode,
                                const char* fullname, 
                                int select = 1);
     
  // Description:
  // This is the main funtion used to find all the directories and 
  // adding all the corresponding node to the dir tree.
  virtual void OpenDirectoryNode(const char* node, 
                                 int select = 1,
                                 int opennode = 1);
  
  // Description:
  // Open the node, including all its parent nodes.
  virtual void OpenWholeTree(const char* node);
  
  // Description:
  // Calling OpenDirectoryNode after centain logic to determine
  // whether to open the whole tree.
  virtual void ReloadDirectoryNode(const char* node);
  
  // Description:
  // This is the actual funtion adding all the nodes to the tree,
  // called from OpenDirectoryNode.
  virtual void UpdateDirectoryNode(const char* node);
  
  // Description:
  // Add a node to the dir tree.
  virtual void AddDirectoryNode(
    const char* parentnode, 
    const char* node, 
    const char* text, 
    const char* fullname, 
    vtkKWIcon *nodeicon);  
  
  // Description:
  // Get one node from a multiple selection of nodes.  An
  // error will be generated if the index is out of range.
  const char *GetNthSelectedNode(int i);
  
  // Description:
  // Update the directory history list given the most recent node
  virtual void UpdateMostRecentDirectoryHistory(const char* node);
     
  // Description:
  // Update directory history list according to the
  // MaximumNumberOfDirectoriesInHistory
  virtual void PruneMostRecentDirectoriesInHistory();

  // Description:
  // Remove a directory node from the most recent history list
  virtual void RemoveDirectoryFromHistory(const char* node);

  // Description:
  // Populate the context menu.
  virtual void PopulateContextMenu(vtkKWMenu *menu, const char*);
  
  // Description:
  // Check if the node is one of the selected nodes.
  int IsNodeSelected(const char* node);

  // Description:
  // Open the directory. 
  // -dirname, the directory that is going to be opened
  // -select, is the flag to whether select this directory
  // return a node for this directory
  const char* OpenDirectoryInternal(const char* dirname, int select);

  // Description:
  // Commands
  char *DirectorySelectedCommand;
  char *DirectoryOpenedCommand;
  char *DirectoryClosedCommand;
  char *DirectoryCreatedCommand;
  char *DirectoryDeletedCommand;
  char *DirectoryRenamedCommand;
  
  // Description:
  // Invoke Commands
  virtual void InvokeDirectorySelectedCommand(const char* path);
  virtual void InvokeDirectoryOpenedCommand(const char* path);
  virtual void InvokeDirectoryClosedCommand(const char* path);
  virtual void InvokeDirectoryCreatedCommand(const char* path);
  virtual void InvokeDirectoryDeletedCommand(const char* path);
  virtual void InvokeDirectoryRenamedCommand(
    const char* oldname, const char* newname);
                                          
  // Description:
  // Internal PIMPL class for STL purposes.
  vtkKWDirectoryExplorerInternals *Internals;
  
  // Description:
  // GUI
  vtkKWToolbar            *Toolbar;
  vtkKWTreeWithScrollbars *DirectoryTree;
  vtkKWPushButton         *CreateFolderButton;
  vtkKWPushButtonWithMenu *BackButton;
  vtkKWPushButtonWithMenu *ForwardButton;
  vtkKWPushButton         *UpButton;
  vtkKWMenu               *ContextMenu;

  // Description:
  // Member variables
  int  MaximumNumberOfDirectoriesInHistory;
  
private:

  static vtkIdType IdCounter;

  vtkKWDirectoryExplorer(const vtkKWDirectoryExplorer&); // Not implemented
  void operator=(const vtkKWDirectoryExplorer&); // Not implemented
};
#endif
