/*=========================================================================

  Module:    $RCSfile: vtkKWFileBrowserDialog.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWFileBrowserDialog - dialog for browsing and selecting 
// files or directories.
// .SECTION Description
// It includes a Favorite Directories Places Bar, a Directory Explorer, 
// a Multi-colomn list to list files, and some toolbar buttons to 
// perform some related tasks on these components.
// .SECTION Thanks
// This work is part of the National Alliance for Medical Image
// Computing (NAMIC), funded by the National Institutes of Health
// through the NIH Roadmap for Medical Research, Grant U54 EB005149.
// Information on the National Centers for Biomedical Computing
// can be obtained from http://nihroadmap.nih.gov/bioinformatics.
// .SECTION See Also
// vtkKWFileBrowserWidget vtkKWDiretoryExplorer vtkKWFileListTable 
// vtkKWFavoriteDirectoriesFrame

#ifndef __vtkKWFileBrowserDialog_h
#define __vtkKWFileBrowserDialog_h

#include "vtkKWDialog.h"

class vtkKWFileBrowserWidget;
class vtkKWPushButton;
class vtkKWFrame;
class vtkKWLabel;
class vtkKWEntry;
class vtkKWComboBox;
class vtkKWFileBrowserDialogInternals;
class vtkStringArray;

class KWWidgets_EXPORT vtkKWFileBrowserDialog : public vtkKWDialog
{
public:
  static vtkKWFileBrowserDialog* New();
  vtkTypeRevisionMacro(vtkKWFileBrowserDialog,vtkKWDialog);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Get the file path(s) the user selected. Note that if the dialog
  // was cancelled, the FileName is set to NULL, reflecting that the user
  // did not select anything (i.e., the previous selection is *not* kept).
  // If MultipleSelection is On, the above methods can also be used retrieve
  // all the file (or directory) names that were selected.
  virtual const char* GetFileName();
  int GetNumberOfFileNames();
  const char *GetNthFileName(int i);
  vtkGetObjectMacro(FileNames, vtkStringArray);  

  // Description:
  // Set/Get a filename to be displayed in the dialog when it pops up.
  vtkSetStringMacro(InitialFileName);
  vtkGetStringMacro(InitialFileName);

  // Description:
  // Set/Get the file types the dialog will open or save.
  // Should be in TK format. 
  // Example: "{{Text Document} {.txt}} {{JPEG image} {.jpg .jpeg}}"
  virtual void SetFileTypes(const char *);
  vtkGetStringMacro(FileTypes);

  // Description:
  // Set/Get the default file extension.
  virtual void SetDefaultExtension(const char *);
  vtkGetStringMacro(DefaultExtension);
  
  // Description:
  // Set/Get if the dialog should be a "save file" dialog, or a "load file"
  // dialog.
  virtual void SetSaveDialog(int);
  vtkBooleanMacro(SaveDialog, int);
  vtkGetMacro(SaveDialog, int);

  // Description:
  // Set/Get if the dialog should ask the user to pick a directory, or a file.
  virtual void SetChooseDirectory(int);
  vtkBooleanMacro(ChooseDirectory, int);
  vtkGetMacro(ChooseDirectory, int);

  // Description:
  // Set/Get if multiple selection are allowed.
  virtual int GetMultipleSelection();
  virtual void SetMultipleSelection(int);
  vtkBooleanMacro(MultipleSelection, int);

  // Description:
  // Set/Get the last path.
  virtual char* GetLastPath();
  vtkSetStringMacro(LastPath);
  
  // Description:
  // Update the LastPath from a full path to a file.
  const char* GenerateLastPath(const char* path);

  // Description:
  // Accessor for the file browser widget object.
  vtkGetObjectMacro(FileBrowserWidget, vtkKWFileBrowserWidget);
  
  // Description:
  // Accessor for the preview frame, i.e. the frame where people can
  // pack their own widgets (say preview images or files, etc.).
  vtkGetObjectMacro(PreviewFrame, vtkKWFrame);

  // Description:
  // Set/Get the visibility of the preview frame
  virtual void SetPreviewFrameVisibility(int);
  vtkBooleanMacro(PreviewFrameVisibility, int); 
  vtkGetMacro(PreviewFrameVisibility, int); 

  // Description:
  // Save/retrieve the last path to/from the registry.
  // Note that the subkey used here is "RunTime".
  virtual void SaveLastPathToRegistry(const char *key);
  virtual void RetrieveLastPathFromRegistry(const char *key);

  // Description:
  // Specifies a command to associate with the widget. This command is 
  // typically invoked when the a file has been selected.
  // The 'object' argument is the object that will have the method called on
  // it. The 'method' argument is the name of the method to be called and any
  // arguments in string form. If the object is NULL, the method is still
  // evaluated as a simple command. 
  // The following parameters are also passed to the command:
  // - filename (first selected): const char* (warning: NULL is passed as
  // empty string)
  virtual void SetFileNameChangedCommand(
    vtkObject *object, const char *method);

  // Description:
  // Events. The FileNameChangedEvent is triggered when the FileName variable
  // is changed.
  // The following parameters are also passed as client data:
  // - current filename: const char*
  //BTX
  enum
  {
    FileNameChangedEvent = 15000
  };
  //ETX

  // Description:
  // Callback. Confirm the action and close this dialog
  virtual void OK();
  
  // Description:
  // Callback. Cancel the action and close this dialog
  virtual void Cancel();

  // Description:
  // Callback. A file type is selected in the File dropdown box.
  virtual void FileTypeChangedCallback(const char* filetype);

  // Description:
  // Add all the default observers needed by that object, or remove
  // all the observers that were added through AddCallbackCommandObserver.
  // Subclasses can override these methods to add/remove their own default
  // observers, but should call the superclass too.
  virtual void AddCallbackCommandObservers();
  virtual void RemoveCallbackCommandObservers();
  
protected:
  vtkKWFileBrowserDialog();
  ~vtkKWFileBrowserDialog();

  // Description:
  // Create the widget.
  virtual void CreateWidget();
  
  // Description:
  // Create (and pack) preview frame.  
  virtual void CreatePreviewFrame();

  // Description:
  // Fill in file types in TK format. 
  // Default is: "{{Text Document} {.txt}}"   
  virtual void PopulateFileTypes();

  // Description:
  // Update the components of the dialog.
  virtual void Update();

  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object is updated
  // and propagated to its internal parts/subwidgets. This will, for example,
  // enable/disable parts of the widget UI, enable/disable the visibility
  // of 3D widgets, etc.
  virtual void UpdateEnableState();

  // Description:
  // Display the dialog.
  virtual void Display();
  
  // Description:
  // Prompt user for confirmation of overwriting file
  // Return 1, confirmed; 0 otherwise
  virtual int ConfirmOverwrite(const char* filename);

  // Description:
  // Final step on the selected files or directories
  virtual int FileOK();
  virtual int DirectoryOK();

  // Description:
  // Internal PIMPL class for STL purposes.
  vtkKWFileBrowserDialogInternals *Internals;
  
  // Description:
  // GUI
  vtkKWFileBrowserWidget *FileBrowserWidget;
  vtkKWFrame             *BottomFrame;
  vtkKWLabel             *FileNameLabel;
  vtkKWLabel             *FileTypesLabel;
  vtkKWEntry             *FileNameText;
  vtkKWComboBox          *FileTypesBox;
  vtkKWPushButton        *OKButton;
  vtkKWPushButton        *CancelButton;  
  vtkKWFrame             *PreviewFrame;

  // Description:
  // Member variables.
  char *FileTypes;
  char *LastPath;
  char *InitialFileName;
  char *DefaultExtension;
  int  PreviewFrameVisibility;
  int  SaveDialog;
  int  ChooseDirectory;
  vtkStringArray *FileNames;

  char *FileNameChangedCommand;
  virtual void InvokeFileNameChangedCommand(const char*);

  // Description:
  // Processes the events that are passed through CallbackCommand (or others).
  // Subclasses can oberride this method to process their own events, but
  // should call the superclass too.
  virtual void ProcessCallbackCommandEvents(
    vtkObject *caller, unsigned long event, void *calldata);
  virtual void ProcessDirectorySelectedEvent(const char* fullname);
  virtual void ProcessFileSelectionChangedEvent(const char* fullname);
  virtual void ProcessFileDoubleClickedEvent(const char* fullname);

private:
  vtkKWFileBrowserDialog(const vtkKWFileBrowserDialog&); // Not implemented
  void operator=(const vtkKWFileBrowserDialog&); // Not implemented
};
#endif
