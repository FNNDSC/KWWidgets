/*=========================================================================

  Module:    $RCSfile: vtkKWLoadSaveDialog.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWLoadSaveDialog - dalog for loading or saving files
// .SECTION Description
// A dialog for loading or saving files. The file is returned through 
// GetFile method.

#ifndef __vtkKWLoadSaveDialog_h
#define __vtkKWLoadSaveDialog_h

#include "vtkKWWidget.h"
class vtkKWApplication;

class VTK_EXPORT vtkKWLoadSaveDialog : public vtkKWWidget
{
public:
  static vtkKWLoadSaveDialog* New();
  vtkTypeRevisionMacro(vtkKWLoadSaveDialog,vtkKWWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create a Tk widget
  virtual void Create(vtkKWApplication *app, const char *args);

  // Description:
  // Invoke the dialog and display it in a modal manner. 
  // This method returns a zero if the dilaog was killed or 
  // canceled, nonzero otherwise.
  virtual int Invoke();

  // Description:
  // Returns 0 if the dialog is active e.g. displayed
  // 1 if it was Canceled 2 if it was OK.
  int GetStatus() 
    { return this->Done; };

  // Description:
  // Set the file types the dialog will open or save
  // Should be in TK format
  vtkSetStringMacro(FileTypes);
  vtkGetStringMacro(FileTypes);

  // Description:
  // Retrieve the file path that the user selected
  vtkGetStringMacro(FileName);

  // Description:
  // Set the file path that the user selected
  vtkSetStringMacro(FileName);

  // Description:
  // Set default extension.
  virtual void SetDefaultExt(const char *)
    {
      vtkErrorMacro("This method is deprecated use SetDefaultExtension");
    }
  const char *GetDefaultExt()
    {
      vtkErrorMacro("This method is deprecated use GetDefaultExtension");
      return NULL;
    }
  
  // Description:
  // Set default extension.
  vtkSetStringMacro(DefaultExtension);
  vtkGetStringMacro(DefaultExtension);
  
  // Description:
  // Set a filename to be displayed in the dialog when it pops up
  vtkSetStringMacro(InitialFileName);
  vtkGetStringMacro(InitialFileName);

  // Description:
  // Set or reset the SaveDialog. If set, the dialog will be
  // save file dialog. If reset, the dialog will be load 
  // dialog
  vtkSetClampMacro(SaveDialog, int, 0, 1);
  vtkBooleanMacro(SaveDialog, int);
  vtkGetMacro(SaveDialog, int);

  // Description:
  // Set or reset the ChooseDirectory ivar.
  // If set to 1, the dialog will ask the user for a directory, not a file
  // If set to 0, the usual file dialog is displayed (default)
  vtkSetClampMacro(ChooseDirectory, int, 0, 1);
  vtkBooleanMacro(ChooseDirectory, int);
  vtkGetMacro(ChooseDirectory, int);

  // Description:
  // Set the title string of the dialog window. Should be called before
  // create otherwise it will have no effect.
  vtkSetStringMacro(Title);
  vtkGetStringMacro(Title);  

  // Description:
  // Set/Get last path
  vtkGetStringMacro(LastPath);
  vtkSetStringMacro(LastPath);

  // Description:
  // Figure out last path out of the file name.
  const char* GenerateLastPath(const char* path);

protected:
  vtkKWLoadSaveDialog();
  ~vtkKWLoadSaveDialog();

  char *FileTypes;
  char *InitialFileName;
  char *Title;
  char *FileName;
  char *DefaultExtension;
  char *LastPath;
  int SaveDialog;
  int ChooseDirectory;
  int Done;

private:
  vtkKWLoadSaveDialog(const vtkKWLoadSaveDialog&); // Not implemented
  void operator=(const vtkKWLoadSaveDialog&); // Not implemented
};


#endif



