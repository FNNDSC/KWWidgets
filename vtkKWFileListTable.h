/*=========================================================================

  Module:    $RCSfile: vtkKWFileListTable.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWFileListTable - a file list table widget
// .SECTION Description
// A widget for displaying and selecting files/directories within 
// a directory. It contains information about file name, size and
// modified time. The files can be sorted by these info individually.
// The right click context menu offers explore, rename, delete functions.
// .SECTION Thanks
// This work is part of the National Alliance for Medical Image
// Computing (NAMIC), funded by the National Institutes of Health
// through the NIH Roadmap for Medical Research, Grant U54 EB005149.
// Information on the National Centers for Biomedical Computing
// can be obtained from http://nihroadmap.nih.gov/bioinformatics.
// .SECTION See Also
// vtkKWFileBrowserDialog vtkKWFileBrowserWidget

#ifndef __vtkKWFileListTable_h
#define __vtkKWFileListTable_h

#include "vtkKWCompositeWidget.h"

class vtkKWMultiColumnListWithScrollbars;
class vtkKWMenu;
class vtkGlobFileNames;
class vtkKWFileListTableInternals;

class KWWidgets_EXPORT vtkKWFileListTable : public vtkKWCompositeWidget
{
public:
  static vtkKWFileListTable* New();
  vtkTypeRevisionMacro(vtkKWFileListTable,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Show folders and files of the given directory (path) in the table,
  // based on file patterns and/or extensions if they exist. 
  // If both patterns and extensions exist, only the patterns are used.
  // Return 1 on success, 0 otherwise
  virtual int ShowFileList(
    const char *path, 
    const char *filepattern,
    const char *fileextensions);

  // Description:
  // Set/Get the parent directory, of which this widget is 
  // displaying the files and directories
  virtual void SetParentDirectory(const char *arg);
  vtkGetStringMacro(ParentDirectory);

  // Description:
  // Set/Get the file pattern that should contains "*" or "?",
  // with which the files will be filtered. When the widget uses
  // file pattern to filter files, the file extensions will be ignored.
  // Example: "foo*" or "?bar"
  virtual void SetFilePattern(const char *arg);
  vtkGetStringMacro(FilePattern);

  // Description:
  // Set/Get the file extenstions that the file table will use
  // to filter its files. When the widget uses
  // file pattern to filter files, the file extensions will be ignored.
  // The argument is NOT a Tk format, just a simple string of extensions
  // with space between them.
  // Example: ".txt .text .jpg .jpeg"
  virtual void SetFileExtensions(const char *arg);
  vtkGetStringMacro(FileExtensions);

  // Description:
  // Set the one of several styles for manipulating the selection. 
  // Valid constants can be found in vtkKWOptions::SelectionModeType.
  virtual void SetSelectionMode(int);
  virtual void SetSelectionModeToSingle();
  virtual void SetSelectionModeToBrowse();
  virtual void SetSelectionModeToMultiple();
  virtual void SetSelectionModeToExtended();

  // Description:
  // Get the files that are selected.  This is meant
  // for use with MultipleSelection mode.
  virtual const char *GetSelectedFileName()
    {return GetNthSelectedFileName(0);}
  virtual int GetNumberOfSelectedFileNames();
  virtual const char *GetNthSelectedFileName(int i);
  virtual void SelectFileName(const char* filename);
  virtual void DeselectFileName(const char*);
  virtual void ClearSelection();

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
  // Set/Get the width (in chars) and height (in lines) of the table.
  // If width is set to 0, the widget will be large enough to show
  // all columns. In all cases, the widget will still obey to its packing
  // layout, i.e. it will stretch all the way if: -fill x
  virtual void SetTableWidth(int width);
  virtual int GetTableWidth();
  virtual void SetTableHeight(int height);
  virtual int GetTableHeight();

  // Description:
  // Convenience method to Set/Get the table background color.
  virtual void GetTableBackgroundColor(double *r, double *g, double *b);
  virtual double* GetTableBackgroundColor();
  virtual void SetTableBackgroundColor(double r, double g, double b);
  virtual void SetTableBackgroundColor(double rgb[3])
    { this->SetTableBackgroundColor(rgb[0], rgb[1], rgb[2]); };

  // Description:
  // Specifies commands to associate with the widget.
  // 'FileSelected' is called whenever the selection is changed. 
  // 'FileDoubleClicked' is called when a file/folder is double-clicked on.
  // 'FileDeleted' is called when a file/folder is removed
  // 'FileRenamed' is called when a file/folder is renamed by right click
  // 'FileUpdated' is called when a new folder is created from right click 
  //               on empty rows.
  // The 'object' argument is the object that will have the method called on
  // it. The 'method' argument is the name of the method to be called and any
  // arguments in string form. If the object is NULL, the method is still
  // evaluated as a simple command. 
  virtual void SetFileSelectedCommand(vtkObject *obj, const char *method);
  virtual void SetFileDoubleClickedCommand(vtkObject *obj, const char *method);
  virtual void SetFileDeletedCommand(vtkObject *obj, const char *method);
  virtual void SetFileRenamedCommand(vtkObject *obj, const char *method);
  virtual void SetFolderCreatedCommand(vtkObject *obj, const char *method);

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
    FileSelectionChangedEvent = 10000,
    FileDoubleClickedEvent,
    FileRenamedEvent,
    FolderCreatedEvent,
    FileDeletedEvent
  };
  //ETX
 
  // Description:
  // Set focus to the file list component of this widget.
  // Based on the visibility
  virtual void Focus();
  virtual int HasFocus();

  // Description:
  // Add event binding to the internal file list 
  // component of this widget, so that these events will be 
  // invoked directly from the file list component of this widget.
  virtual void AddBindingToInternalWidget(const char* event,
    vtkObject *obj, const char* method);
  
  // Description:
  // Callback, do NOT use. 
  // Right-click context menu callbacks
  virtual void ContextMenuCallback(int row, int col, int x, int y);
  
  // Description:
  // Callback, do NOT use. 
  // Callbacks for Prior/Next key navigations in the table
  virtual void KeyPriorNextNavigationCallback(
    const char *w, int x, int y, 
    int root_x, int root_y, const char* key);
  
  // Description:
  // Callback, do NOT use. 
  // Rename callback.
  virtual int RenameFileCallback();
  
  // Description:
  // Callback, do NOT use. 
  // Callback triggered when the file list table is getting focus.
  virtual void FocusInCallback();
  
  // Description:
  // Callback, do NOT use. 
  // Launch native explorer callback.
  virtual void ExploreFileCallback();

  // Description:
  // Callback, do NOT use. 
  // Create new folder callback.
  virtual void CreateNewFolderCallback(const char* parentdir);

  // Description:
  // Callback, do NOT use. 
  // Callback triggered when the file selection is changed in the file list. 
  virtual void SelectedFileChangedCallback();
  
  // Description:
  // Callback, do NOT use. 
  // Callback triggered for the Navigation keys: Home/End. 
  // This is used to change the default behavior of the
  // vtkKWMultiColumnList, so that the list behaves more like Win32 explorer
  virtual void KeyHomeEndNavigationCallback(const char *key);
    
  // Description:
  // Callback, do NOT use. 
  // Callback triggered when an item is double clicked in the file list. 
  virtual void FileDoubleClickCallback();
  
  // Description:
  // Callback, do NOT use. 
  // Callback triggered when the 'Delete' key is pressed.
  // Remove the selected item from the file list.
  virtual int RemoveSelectedFileCallback();

  // Description:
  // Callback, do NOT use. 
  // When the time column in the file list is displayed, convert 
  // the celltext ('+' as file or '-' as folder plus time value in seconds 
  // for sorting while keeps folders and files separate)) to ctime format.
  // Funtion returns the formatted string that will be displayed: char*       
  virtual char *GetFormatTimeStringCallback(const char* celltext);
  
  // Description:
  // Callback, do NOT use. 
  // Sort items by time.
  virtual int SortTimeCallback(const char* cell1, const char* cell2);

  // Description:
  // Callback, do NOT use. 
  // When the size column in the file list is displayed, convert 
  // the celltext (size value in bytes) to 'KB' format.
  // Funtion returns the formatted string that will be displayed: const char*
  virtual char *GetFormatSizeStringCallback(const char* celltext);
  
  // Description:
  // Callback, do NOT use. 
  // When the Name column in the file list is displayed, convert 
  // the celltext (1 as file or 0 as folder plus real name for sorting 
  // while keeping folders and files seperate) to real name.
  // Funtion returns the real name that will be displayed: const char*       
  virtual char *GetRealNameStringCallback(const char* celltext);
  
  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object 
  // is updated and propagated to its internal parts/subwidgets. 
  // This will, for example, enable/disable parts of the widget UI, 
  // enable/disable the visibility of 3D widgets, etc.
  virtual void UpdateEnableState();

protected:
  vtkKWFileListTable();
  ~vtkKWFileListTable();

  // Description:
  // Create the widget.
  virtual void CreateWidget();
  
  // Description:
  // Populate the right-click context menu.
  virtual void PopulateContextMenu(int rowselected);

  // Description:
  // Commands
  char *FileSelectedCommand;
  char *FileDoubleClickedCommand;
  char *FileDeletedCommand;
  char *FileRenamedCommand;
  char *FolderCreatedCommand;
  
  // Description:
  // Invoke Commands
  virtual void InvokeFileSelectedCommand(const char* path);
  virtual void InvokeFileDoubleClickedCommand(const char* path);
  virtual void InvokeFileDeletedCommand(const char* path, int isDir);
  virtual void InvokeFileRenamedCommand(
    const char* oldname, const char* newname);
  virtual void InvokeFolderCreatedCommand(const char* filename);
  
  // Description:
  // Set the parent directory, file pattern, file extensions
  virtual void SetParentDirectoryInternal(const char *arg);
  virtual void SetFilePatternInternal(const char *arg);
  virtual void SetFileExtensionsInternal(const char *arg);
  
  // Description:
  // Member variables
  vtkKWFileListTableInternals *Internals;
  vtkKWMultiColumnListWithScrollbars *FileList;
  vtkKWMenu *ContextMenu;
  char* ParentDirectory;
  char* FilePattern;
  char* FileExtensions;
  
private:
  vtkKWFileListTable(const vtkKWFileListTable&); // Not implemented
  void operator=(const vtkKWFileListTable&); // Not implemented

  // Description:
  // Get a temporary full filename given the row index, 
  // should be stored right away
  virtual char* GetRowFileName(int row);

  // Description:
  // Get the cell text
  virtual const char* GetCellText(int row, int col);

};
#endif
