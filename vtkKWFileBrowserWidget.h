/*=========================================================================

  Module:    $RCSfile: vtkKWFileBrowserWidget.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWFileBrowserWidget - a file browser widget
// .SECTION Description
// The class assembles vtkKWFavoriteDirectoriesFrame, vtkKWDirectoryExploer,
// and vtkKWFileListTable widgets together and give user options to turn
// on/off (visible/invisible) each individual widget.
// .SECTION Thanks
// This work is part of the National Alliance for Medical Image
// Computing (NAMIC), funded by the National Institutes of Health
// through the NIH Roadmap for Medical Research, Grant U54 EB005149.
// Information on the National Centers for Biomedical Computing
// can be obtained from http://nihroadmap.nih.gov/bioinformatics.
// .SECTION See Also
// vtkKWFileBrowserDialog vtkKWDirectoryExplorer vtkKWFileListTable
// vtkKWFavoriteDirectoriesFrame

#ifndef __vtkKWFileBrowserWidget_h
#define __vtkKWFileBrowserWidget_h

#include "vtkKWCompositeWidget.h"

class vtkKWFavoriteDirectoriesFrame;
class vtkKWDirectoryExplorer;
class vtkKWFileListTable;
class vtkKWFrame;
class vtkKWSplitFrame;
class vtkKWFileBrowserWidgetInternals;

class KWWidgets_EXPORT vtkKWFileBrowserWidget : public vtkKWCompositeWidget
{
public:
  static vtkKWFileBrowserWidget* New();
  vtkTypeRevisionMacro(vtkKWFileBrowserWidget,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Open a directory.
  // Return 1 on success, 0 otherwise
  virtual int OpenDirectory(const char* path);
  
  // Description:
  // Filter the files of the file list table by file extensions
  virtual void FilterFilesByExtensions(const char* fileextensions);

  // Description:
  // Accessor for DirectoryExplorer and FileListTable
  vtkGetObjectMacro(DirectoryExplorer, vtkKWDirectoryExplorer);
  vtkGetObjectMacro(FileListTable, vtkKWFileListTable);

  // Description:
  // Set/Get the visibility of the directory explorer
  virtual void SetDirectoryExplorerVisibility(int);
  vtkBooleanMacro(DirectoryExplorerVisibility, int); 
  vtkGetMacro(DirectoryExplorerVisibility, int); 
  
  // Description:
  // Set/Get the visibility of the favorite directories frame
  virtual void SetFavoriteDirectoriesFrameVisibility(int);
  vtkBooleanMacro(FavoriteDirectoriesFrameVisibility, int); 
  vtkGetMacro(FavoriteDirectoriesFrameVisibility, int); 
  
  // Description:
  // Set/Get the visibility of the file list table
  virtual void SetFileListTableVisibility(int);
  vtkBooleanMacro(FileListTableVisibility, int); 
  vtkGetMacro(FileListTableVisibility, int); 

  // Description:
  // Set/Get if multiple selection are allowed.
  virtual void SetMultipleSelection(int);
  vtkBooleanMacro(MultipleSelection, int);
  vtkGetMacro(MultipleSelection, int);
  
  // Description:
  // Set/Get the foreground/background color of selected items, when
  // in focus or out of focus.
  vtkGetVector3Macro(SelectionForegroundColor, double);
  virtual void SetSelectionForegroundColor(double r, double g, double b);
  virtual void SetSelectionForegroundColor(double rgb[3])
    { this->SetSelectionForegroundColor(rgb[0], rgb[1], rgb[2]); };
  vtkGetVector3Macro(SelectionBackgroundColor, double);
  virtual void SetSelectionBackgroundColor(double r, double g, double b);
  virtual void SetSelectionBackgroundColor(double rgb[3])
    { this->SetSelectionBackgroundColor(rgb[0], rgb[1], rgb[2]); };
  vtkGetVector3Macro(OutOfFocusSelectionForegroundColor, double);
  virtual void SetOutOfFocusSelectionForegroundColor(
    double r, double g, double b);
  virtual void SetOutOfFocusSelectionForegroundColor(double rgb[3])
    { this->SetOutOfFocusSelectionForegroundColor(rgb[0], rgb[1], rgb[2]); };
  vtkGetVector3Macro(OutOfFocusSelectionBackgroundColor, double);
  virtual void SetOutOfFocusSelectionBackgroundColor(
    double r, double g, double b);
  virtual void SetOutOfFocusSelectionBackgroundColor(double rgb[3])
    { this->SetOutOfFocusSelectionBackgroundColor(rgb[0], rgb[1], rgb[2]); };
  
  // Description:
  // Set focus to directory explorer or file list table.
  virtual void SetFocusToDirectoryExplorer();
  virtual void SetFocusToFileListTable();
  
  // Description:
  // Callback, do NOT use
  // When the "Add Favorites" button is clicked in the favorite directories
  // frame, this function will be called and a dialog will popup prompting 
  // for a name of the directory that is going to be added to the favorite 
  // directory frame.
  virtual void AddFavoriteDirectoryCallback();
 
  // Description:
  // Callback, do NOT use
  // When a favorite button in the favorite directory frame is clicked on, 
  // navigate to the corresponding favorite directory (path is a pointer to
  // the absolute directory path, text a pointer to the favorite button label)
  virtual void FavoriteDirectorySelectedCallback(
    const char* path, 
    const char* text);
  
  // Description:
  // Callbacks, do NOT use
  // Callback for the vtkKWDirectoryExplorer widget.
  virtual void DirectoryClickedCallback();
  virtual void DirectoryAddedCallback(const char* fullname);
  virtual void DirectoryChangedCallback(const char* fullname);
  virtual void DirectoryOpenedCallback(const char* fullname);
  virtual void DirectoryClosedCallback(const char* fullname);
  virtual void DirectoryRemovedCallback(const char* fullname);
  virtual void DirectoryRenamedCallback(
    const char* oldname, 
    const char* newname);
  
  // Description:
  // Callbacks, do NOT use
  // Callback for the vtkKWFileListTable widget.
  virtual void FileSelectionChangedCallback(const char* fullname);
  virtual void FileDoubleClickedCallback(const char* fullname);
  virtual void FileRenamedCallback(
    const char* oldname, 
    const char* newname);
  virtual void FolderCreatedCallback(const char* filename);
  virtual void FileRemovedCallback(const char* fullname, int isDir);

  // Description:
  // Callback, do NOT use. 
  virtual void DirectoryTreeFocusInCallback();
  virtual void FileTableFocusInCallback();
  virtual void DirectoryTreeFocusOutCallback();
  virtual void FileTableFocusOutCallback();
  
protected:
  vtkKWFileBrowserWidget();
  ~vtkKWFileBrowserWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();
  
  // Description:
  // Pack all the widgets according to their visibility
  virtual void Pack();
  
  // Description:
  // Setup all the frames with widgets.
  virtual void CreateFavoriteDirectoriesFrame();
  virtual void CreateDirectoryExplorerAndFileListTableFrame();
  virtual void CreateDirectoryExplorer();
  virtual void CreateFileListTable();

  // Description:
  // Function to update the selected Background/Foreground.
  virtual void UpdateDirectorySelectionColor(int infocus = 0);
  virtual void UpdateFileSelectionColor(int infocus = 0);
  
  virtual void UpdateForCurrentDirectory();
  virtual void PropagateMultipleSelection();

  // Description:
  // Member variable
  // PIMPL for STL stuff.
  vtkKWFileBrowserWidgetInternals *Internals;
  
  // Description:
  // Member variable
  // A split frame holding the favorite frame, directory frame, and 
  // file list table.
  // Both directory frame and file list table will be in DirFileFrame, which
  // is parented to MainFrame->Frame2.
  vtkKWSplitFrame  *MainFrame;
  vtkKWSplitFrame  *DirFileFrame;
 
  // Description:
  // GUI members.
  vtkKWFavoriteDirectoriesFrame *FavoriteDirectoriesFrame;
  vtkKWDirectoryExplorer        *DirectoryExplorer;
  vtkKWFileListTable            *FileListTable;

  // Description:
  // Member variables
  int DirectoryExplorerVisibility;
  int FavoriteDirectoriesFrameVisibility;
  int FileListTableVisibility;
  int MultipleSelection;

  // Description:
  // Colors
  double SelectionForegroundColor[3];
  double SelectionBackgroundColor[3];
  double OutOfFocusSelectionForegroundColor[3];
  double OutOfFocusSelectionBackgroundColor[3];

private:
  vtkKWFileBrowserWidget(const vtkKWFileBrowserWidget&); // Not implemented
  void operator=(const vtkKWFileBrowserWidget&); // Not implemented
};

#endif
