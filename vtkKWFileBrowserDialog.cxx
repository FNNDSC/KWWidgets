/*=========================================================================

  Module:    $RCSfile: vtkKWFileBrowserDialog.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkKWFileBrowserDialog.h"

#include "vtkKWApplication.h"
#include "vtkKWComboBox.h"
#include "vtkKWDirectoryExplorer.h"
#include "vtkKWFileBrowserWidget.h"
#include "vtkKWFileBrowserUtilities.h"
#include "vtkKWFavoriteDirectoriesFrame.h"
#include "vtkKWFileListTable.h"
#include "vtkKWInternationalization.h"
#include "vtkKWLabel.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWPushButton.h"
#include "vtkKWRegistryHelper.h"
#include "vtkKWSplitFrame.h"
#include "vtkKWTkUtilities.h"

#include "vtkDirectory.h"
#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
#include "vtkCallbackCommand.h"

#include <vtksys/RegularExpression.hxx>
#include <vtksys/SystemTools.hxx>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWFileBrowserDialog );
vtkCxxRevisionMacro(vtkKWFileBrowserDialog, "$Revision: 1.48 $");

//----------------------------------------------------------------------------
class vtkKWFileBrowserDialogInternals
{
public:

  vtkKWFileBrowserDialogInternals()
  {
    this->CurrentFileExtensions = ".*";
    this->CurrentSelectedFileNames = "";
    this->IsEditingFileName = 0;
    this->GeometryRegKey = "KWFileBrowserGeometry";
  }
  
  vtksys_stl::string CurrentFileExtensions;
  vtksys_stl::string CurrentSelectedFileNames;
  vtksys_stl::string GeometryRegKey;

  int IsEditingFileName;
};

//----------------------------------------------------------------------------
vtkKWFileBrowserDialog::vtkKWFileBrowserDialog()
{
  this->FileTypes         = NULL;
  this->LastPath          = NULL;
  this->InitialFileName   = NULL;
  this->DefaultExtension  = NULL;
  this->FileName          = NULL;

  this->SaveDialog       = 0;
  this->ChooseDirectory  = 0;
  this->FileNames        = vtkStringArray::New();
  this->InitialSelecttedFileNames   = vtkStringArray::New();
  this->FileNameChangedCommand      = NULL;

  this->Internals          = new vtkKWFileBrowserDialogInternals;

  this->PreviewFrame       = NULL;
  this->FileBrowserWidget  = vtkKWFileBrowserWidget::New();
  this->BottomFrame        = vtkKWFrame::New();
  this->OKButton           = vtkKWPushButton::New();
  this->CancelButton       = vtkKWPushButton::New();
  this->FileNameText       = vtkKWEntry::New();
  this->FileTypesBox       = vtkKWComboBox::New();
  this->FileNameLabel      = vtkKWLabel::New();
  this->FileTypesLabel     = vtkKWLabel::New();
  
  this->PreviewFrameVisibility = 0;
}

//----------------------------------------------------------------------------
vtkKWFileBrowserDialog::~vtkKWFileBrowserDialog()
{
  if (this->FileTypes)
    {
    delete [] this->FileTypes;
    this->FileTypes = NULL;
    }

  if (this->LastPath)
    {
    delete [] this->LastPath;
    this->LastPath = NULL;
    }

  if (this->InitialFileName)
    {
    delete [] this->InitialFileName;
    this->InitialFileName = NULL;
    }

  if (this->FileName)
    {
    delete [] this->FileName;
    this->FileName = NULL;
    }

  if (this->DefaultExtension)
    {
    delete [] this->DefaultExtension;
    this->DefaultExtension = NULL;
    }

  if (this->FileNames)
    {
    this->FileNames->Delete();
    this->FileNames = NULL;
    }

  if (this->InitialSelecttedFileNames)
  {
    this->InitialSelecttedFileNames->Delete();
    this->InitialSelecttedFileNames = NULL;
  }

  if (this->FileNameChangedCommand)
    {
    delete [] this->FileNameChangedCommand;
    this->FileNameChangedCommand = NULL;
    }

  if (this->PreviewFrame)
    {
    this->PreviewFrame->Delete();
    this->PreviewFrame = NULL;
    }
  
  this->OKButton->Delete();
  this->CancelButton->Delete();
  this->FileNameText->Delete();
  this->FileTypesBox->Delete();
  this->FileNameLabel->Delete();
  this->FileTypesLabel->Delete();
  this->BottomFrame->Delete();
  this->FileBrowserWidget->Delete();  

  if (this->Internals)
    {
    delete this->Internals;
    }
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  this->SetIconName(this->GetApplication()->GetPrettyName());
  this->SetResizable(1, 1);

  // File browser

  this->FileBrowserWidget->SetParent(this);
  this->FileBrowserWidget->Create();

  this->Script("pack %s -side top -fill both -expand true",
               this->FileBrowserWidget->GetWidgetName());

  // Bottom frame

  this->BottomFrame->SetParent(this);
  this->BottomFrame->Create();
  this->Script("pack %s -side top -fill x", 
               this->BottomFrame->GetWidgetName());

  this->FileNameLabel->SetParent(this->BottomFrame);
  this->FileNameLabel->Create();
  this->FileNameLabel->SetText("File name:");

  this->FileTypesLabel->SetParent(this->BottomFrame);
  this->FileTypesLabel->Create();
  this->FileTypesLabel->SetText("Files of type:");
  
  this->FileNameText->SetParent(this->BottomFrame);
  this->FileNameText->Create();
  this->FileNameText->SetBinding("<Return>", this, "OK");
  this->FileNameText->SetCommand(this, "FileNameEditingCallback");
  this->FileNameText->SetCommandTriggerToAnyChange();

  this->FileTypesBox->SetParent(this->BottomFrame);
  this->FileTypesBox->Create();
  this->FileTypesBox->SetReadOnly(1);
  this->FileTypesBox->SetCommand(this, "FileTypeChangedCallback");  

  this->OKButton->SetParent(this->BottomFrame);
  this->OKButton->Create();
  this->OKButton->SetWidth(10);
  this->OKButton->SetCommand(this, "OK");

  this->CancelButton->SetParent(this->BottomFrame);
  this->CancelButton->Create();
  this->CancelButton->SetWidth(10);
  this->CancelButton->SetText("Cancel");
  this->CancelButton->SetCommand(this, "Cancel");

  this->AddCallbackCommandObservers();

  this->Update();
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::Update()
{
  if (!this->IsCreated())
    {
    return;
    }

  // Show/Hide components

  if (this->ChooseDirectory)
    {
    this->FileBrowserWidget->DirectoryExplorerVisibilityOn();
    this->FileBrowserWidget->FileListTableVisibilityOff();
    this->SetMinimumSize(565, 400);
    if (!this->GetTitle() || !strcmp(this->GetTitle(), "Select File"))
      {
      this->SetTitle("Select Directory");
      }
    }
  else
    {
    this->FileBrowserWidget->DirectoryExplorerVisibilityOn();
    this->FileBrowserWidget->FileListTableVisibilityOn();
    this->SetMinimumSize(780, 300);
    if (!this->GetTitle() || !strcmp(this->GetTitle(), "Select Directory"))
      {
      this->SetTitle("Select File");
      }
    }

  // OK Button

  if (this->SaveDialog)
    {
    this->OKButton->SetText("Save");
    }
  else if (this->ChooseDirectory)
    {
    this->OKButton->SetText("OK");
    }
  else 
    {
    this->OKButton->SetText("Open");
    }

  // Bottom Frame

  if (!this->ChooseDirectory)
    {
    this->Script("grid %s -column 0 -row 0 -sticky nw -padx 2 -pady 5", 
                 this->FileNameLabel->GetWidgetName());
    this->Script("grid %s -column 0 -row 1 -sticky nw -padx 2 -pady 5", 
                 this->FileTypesLabel->GetWidgetName());
    this->Script("grid %s -column 1 -row 0 -sticky ew -padx 2 -pady 2", 
                 this->FileNameText->GetWidgetName());
    this->Script("grid %s -column 1 -row 1 -sticky ew -padx 2 -pady 2", 
                 this->FileTypesBox->GetWidgetName());
    this->Script("grid %s -column 2 -row 0 -sticky ne -padx 2 -pady 2", 
                 this->OKButton->GetWidgetName());
    this->Script("grid %s -column 2 -row 1 -sticky ne -padx 2 -pady 2", 
                 this->CancelButton->GetWidgetName());

    this->Script("grid columnconfigure %s 1 -weight 1", 
      this->BottomFrame->GetWidgetName());
    }
  else
    {
    this->Script("grid remove %s", this->FileNameLabel->GetWidgetName());
    this->Script("grid remove %s", this->FileTypesLabel->GetWidgetName());
    this->Script("grid remove %s", this->FileNameText->GetWidgetName());
    this->Script("grid remove %s", this->FileTypesBox->GetWidgetName());
    this->Script("grid remove %s", this->OKButton->GetWidgetName());
    this->Script("grid remove %s", this->CancelButton->GetWidgetName());

    this->Script("grid columnconfigure %s 1 -weight 0", 
      this->BottomFrame->GetWidgetName());

    this->Script("grid %s -column 0 -row 0 -sticky nw -padx 2 -pady 2", 
                 this->OKButton->GetWidgetName());
    this->Script("grid %s -column 1 -row 0 -sticky nw -padx 2 -pady 2", 
                 this->CancelButton->GetWidgetName());
    }

  this->CreatePreviewFrame();
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::CreatePreviewFrame()
{
  if (!this->PreviewFrame)
    {
    this->PreviewFrame = vtkKWFrame::New();
    }
  if (!this->PreviewFrame->IsCreated())
    {
    this->PreviewFrame->SetParent(this);
    this->PreviewFrame->Create();
    this->PreviewFrame->SetReliefToFlat();
    }
  if (this->PreviewFrameVisibility)
    {
    this->Script("pack %s -side top -fill both -expand y -padx 2 -pady 2",
                 this->PreviewFrame->GetWidgetName());
    }
  else
    {
    this->Script("pack forget %s",
                 this->PreviewFrame->GetWidgetName());
    }
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::SaveGeometryToRegistry()
{
  if (this->IsCreated())
    {
    vtkKWRegistryHelper* registryhelper = vtkKWRegistryHelper::New();
    const char* subkey =  this->Internals->GeometryRegKey.c_str();
    if (!registryhelper->Open(VTK_KW_FAVORITE_TOPLEVEL, subkey, 1)) 
      {
      vtkErrorMacro(
        "Error! Failed to open the registry key for writing!");
      registryhelper->Delete();
      return;
      }

    char value[vtkKWRegistryHelper::RegistryKeyValueSizeMax];

    vtkKWFileBrowserWidget* widget = this->GetFileBrowserWidget();
    
    int width, height;
    vtkKWTkUtilities::GetWidgetSize(this, &width, &height);
    sprintf(value, "%d", width);
    if(this->ChooseDirectory)
      {
      registryhelper->SetValue(subkey, "DirExplorerDialogWidth", value);
      }
    else
      {
      registryhelper->SetValue(subkey, "FileBrowserDialogWidth", value);
      }

    sprintf(value, "%d", height);
    if(this->ChooseDirectory)
      {
      registryhelper->SetValue(subkey, "DirExplorerDialogHeight", value);
      }
    else
      {
      registryhelper->SetValue(subkey, "FileBrowserDialogHeight", value);

      vtkKWTkUtilities::GetWidgetSize(widget, &width, &height);
      sprintf(value, "%d", width);
      registryhelper->SetValue(subkey, "FileBrowserWidgetWidth",value);

      vtkKWTkUtilities::GetWidgetSize(widget->GetDirFileFrame(), &width, &height);
      sprintf(value, "%d", width);
      registryhelper->SetValue(subkey, "FileBrowserDirFileFrameWidth", value);
      vtkKWTkUtilities::GetWidgetSize(widget->GetMainFrame(), &width, &height);
      sprintf(value, "%d", width);
      registryhelper->SetValue(subkey, "FileBrowserMainFrameWidth", value);
      sprintf(value, "%d", widget->GetDirFileFrame()->GetFrame1Size());
      registryhelper->SetValue(subkey, "FileBrowserWidgetSubFrame1Size", value);
      }

    sprintf(value, "%d", widget->GetMainFrame()->GetFrame1Size());
    registryhelper->SetValue(subkey, "FileBrowserWidgetMainFrame1Size", value);

    int x, y;
    this->GetPosition(&x, &y);
    sprintf(value, "%d", x);
    registryhelper->SetValue(subkey, "FileBrowserDialogPosX", value);
    sprintf(value, "%d", y);
    registryhelper->SetValue(subkey, "FileBrowserDialogPosY", value);
    registryhelper->Close();
    registryhelper->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::RestoreGeometryFromRegistry()
{
  if (this->IsCreated())
    {
    vtkKWRegistryHelper* registryhelper = vtkKWRegistryHelper::New();
    const char* subkey =  this->Internals->GeometryRegKey.c_str();
    if (!registryhelper->Open(VTK_KW_FAVORITE_TOPLEVEL, subkey, 0)) 
      {
      registryhelper->Delete();
      return;
      }  

    vtkKWFileBrowserWidget* widget = this->GetFileBrowserWidget();
    char value[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
    int width = 0, height = 0;
    
    vtksys_stl::string widthKey, heightKey;
    if(this->ChooseDirectory)
      {
      widthKey = "DirExplorerDialogWidth";
      heightKey = "DirExplorerDialogHeight";
      }
    else
      {
      widthKey = "FileBrowserDialogWidth";
      heightKey = "FileBrowserDialogHeight";
      }

    value[0]=0;
    if (registryhelper->ReadValue(subkey, widthKey.c_str(), value))
      {
      width = *value ? atoi(value) : 0;
      }
    value[0]=0;
    if (registryhelper->ReadValue(subkey, heightKey.c_str(), value))
      {
      height = *value ? atoi(value) : 0;
      }  
    if(width > 0 && height > 0)
      {
      this->SetSize(width, height);
      }

    if(!this->ChooseDirectory)
      {
      value[0]=0;
      if (registryhelper->ReadValue(subkey, "FileBrowserWidgetWidth", value))
        {
        width = *value ? atoi(value) : 0;
        if(width > 0)
          {
          widget->SetWidth(width);
          }
        }  

      value[0]=0;
      if (registryhelper->ReadValue(subkey, "FileBrowserMainFrameWidth", value))
        {
        width = *value ? atoi(value) : 0;
        if(width > 0)
          {
          widget->GetMainFrame()->SetWidth(width);
          }
        }  
      
      value[0]=0;
      if (registryhelper->ReadValue(subkey, "FileBrowserDirFileFrameWidth", value))
        {
        width = *value ? atoi(value) : 0;
        if(width > 0)
          {
          widget->GetDirFileFrame()->SetWidth(width);
          }   
        }
      }

    value[0]=0;
    if (registryhelper->ReadValue(subkey, "FileBrowserWidgetMainFrame1Size", value))
      {
      width = *value ? atoi(value) : 0;
      if( width > 0)
        {
        widget->GetMainFrame()->SetFrame1Size(width);
        }
      }  

    // This is necessary to get the size right
    this->GetApplication()->ProcessPendingEvents();
    if(!this->ChooseDirectory)
      {
      value[0]=0;
      if (registryhelper->ReadValue(subkey, "FileBrowserWidgetSubFrame1Size", value))
        {
        width = *value ? atoi(value) : 0;
        if( width > 0)
          {
          widget->GetDirFileFrame()->SetFrame1Size(width);
          }
        }
      }

    int x=-1, y=-1;
    value[0]=0;
    if (registryhelper->ReadValue(subkey, "FileBrowserDialogPosX", value))
      {
      x = *value ? atoi(value) : 0;
      }
    value[0]=0;
    if (registryhelper->ReadValue(subkey, "FileBrowserDialogPosY", value))
      {
      y = *value ? atoi(value) : 0;
      }
    if(x>=0 && y>=0)
      {
      this->SetPosition(x, y);
      }
    registryhelper->Close();
    registryhelper->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::Display()
{
  if(this->GetApplication()->GetSaveUserInterfaceGeometry())
    {
    this->RestoreGeometryFromRegistry();
    }

  this->Superclass::Display();
  
  this->PopulateFileTypes(); 

  // If we can not set up initial selected files, go as usual
  if(!this->SetupInitialSelectedFiles())
    {
    // Load the last path

    if (this->LastPath && *(this->LastPath) &&
        vtksys::SystemTools::FileIsDirectory(this->LastPath))
      {
      this->FileBrowserWidget->OpenDirectory(this->LastPath);
      }
    else
      {
      this->FileBrowserWidget->OpenDirectory(
        vtksys::SystemTools::GetCurrentWorkingDirectory().c_str());
      }

    if (this->FileTypesBox->GetNumberOfValues() > 0)
      {
      vtksys_stl::string buffer(this->FileTypesBox->GetValueFromIndex(0));
      this->FileTypesBox->SetValue(buffer.c_str());
      this->FileTypeChangedCallback(buffer.c_str());
      }

    if (this->InitialFileName && *this->InitialFileName)
      {
      this->FileNameText->SetValue(
        vtksys::SystemTools::GetFilenameName(this->InitialFileName).c_str());
      }
  }

  this->FileBrowserWidget->GetFavoriteDirectoriesFrame()->RestoreFavoriteDirectoriesFromRegistry();

  this->FileBrowserWidget->SetFocusToDirectoryExplorer();
}

//----------------------------------------------------------------------------
int vtkKWFileBrowserDialog::SetupInitialSelectedFiles()
{
  if (!this->InitialSelecttedFileNames || 
    !this->InitialSelecttedFileNames->GetNumberOfValues())
    {
    this->FileBrowserWidget->GetDirectoryExplorer()->ClearSelection();
    this->FileBrowserWidget->GetFileListTable()->ClearSelection();
    return 0;
    }

  if (this->ChooseDirectory)
    {
    if(this->GetMultipleSelection())
      {
      for(int i=0; i<this->InitialSelecttedFileNames->GetNumberOfValues(); i++)
        {
        this->FileBrowserWidget->GetDirectoryExplorer()->SelectDirectory(
          this->InitialSelecttedFileNames->GetValue(i));
        }
      }
    else
      {
      this->FileBrowserWidget->GetDirectoryExplorer()->SelectDirectory(
      this->InitialSelecttedFileNames->GetValue(0));
      }
    }
  else
    {
    vtksys_stl::string selDir = vtksys::SystemTools::GetFilenamePath(
      this->InitialSelecttedFileNames->GetValue(0));
    if(!this->FileBrowserWidget->OpenDirectory(selDir.c_str()))
      {
      return 0;
      }

    this->FileBrowserWidget->GetFileListTable()->SetParentDirectory(selDir.c_str());

    if(this->GetMultipleSelection())
      {
      for(int i=0; i<this->InitialSelecttedFileNames->GetNumberOfValues(); i++)
        {
        this->FileBrowserWidget->GetFileListTable()->SelectFileName(
          this->InitialSelecttedFileNames->GetValue(i));
        }
      }
    else
      {
      this->FileBrowserWidget->GetFileListTable()->SelectFileName(
        this->InitialSelecttedFileNames->GetValue(0));
      }
    } 
  return 1;
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::SetFileTypes(const char* _arg)
{
  if (this->FileTypes == NULL && _arg == NULL) 
    { 
    return;
    }

  if (this->FileTypes && _arg && (!strcmp(this->FileTypes, _arg))) 
    {
    return;
    }

  if (this->FileTypes) 
    { 
    delete [] this->FileTypes; 
    }

  if (_arg)
    {
    this->FileTypes = new char[strlen(_arg) + 1];
    strcpy(this->FileTypes, _arg);
    }
  else
    {
    this->FileTypes = NULL;
    }

  this->Modified();

  if(this->IsCreated())
    {
    this->PopulateFileTypes();
    if (this->FileTypesBox->GetNumberOfValues() > 0)
      {
      vtksys_stl::string buffer(this->FileTypesBox->GetValueFromIndex(0));
      this->FileTypesBox->SetValue(buffer.c_str());
      this->FileTypeChangedCallback(buffer.c_str());
      }
    }
}

//----------------------------------------------------------------------------
const char* vtkKWFileBrowserDialog::GetCurrentFileExtensions()
{
  if(strcmp(this->Internals->CurrentFileExtensions.c_str(), ".*"))
    {
    return this->Internals->CurrentFileExtensions.c_str();
    }
  return NULL;
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::SetDefaultExtension(const char* _arg)
{
  if (this->DefaultExtension == NULL && _arg == NULL) 
    { 
    return;
    }

  if (this->DefaultExtension && _arg && (!strcmp(this->DefaultExtension, _arg))) 
    {
    return;
    }

  if (this->DefaultExtension) 
    { 
    delete [] this->DefaultExtension; 
    }

  if (_arg)
    {
    this->DefaultExtension = new char[strlen(_arg) + 1];
    strcpy(this->DefaultExtension, _arg);
    }
  else
    {
    this->DefaultExtension = NULL;
    }

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::PopulateFileTypes()
{ 
  if (!this->IsCreated())
    {
    return;
    }

  this->FileTypesBox->DeleteAllValues();

  if (!this->GetFileTypes())
    {
    this->FileTypesBox->SetValue("All Files (.*)");
    return;
    }
    
  vtksys::RegularExpression filetyperegexp(
    "{ *{([^}]+)} +{[^\\.]*([^}]+)} *}");
  vtksys_stl::vector<vtksys_stl::string> filetypes;
  vtksys_stl::string filetypetext;
  vtksys_stl::string filetypeext;

  vtksys_stl::string strfiletypes = this->GetFileTypes();

  while (filetyperegexp.find(strfiletypes))
    {
    filetypetext = filetyperegexp.match(1);
    filetypetext.append(" (");
    filetypetext.append(filetyperegexp.match(2));
    filetypetext.append(")");

    if (!this->FileTypesBox->HasValue(filetypetext.c_str()))
      {
      this->FileTypesBox->AddValue(filetypetext.c_str());
      }

    strfiletypes = 
      strfiletypes.substr(filetyperegexp.end(), strfiletypes.length());
    }
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::SetPreviewFrameVisibility(int arg)
{
  arg = arg ? 1 : 0;
  if (this->PreviewFrameVisibility == arg)
    {
    return;
    }

  this->PreviewFrameVisibility = arg;
  this->Modified();

  this->Update();
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::SetChooseDirectory(int arg)
{
  arg = arg ? 1 : 0;
  if (this->ChooseDirectory == arg)
    {
    return;
    }

  this->ChooseDirectory = arg;
  this->Modified();

  this->Update();
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::SetSaveDialog(int arg)
{
  arg = arg ? 1 : 0;
  if (this->SaveDialog == arg)
    {
    return;
    }

  this->SaveDialog = arg;
  this->Modified();
  this->Update();
}

//----------------------------------------------------------------------------
int vtkKWFileBrowserDialog::GetMultipleSelection()
{
  return this->FileBrowserWidget->GetMultipleSelection();
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::SetMultipleSelection(int arg)
{
  this->FileBrowserWidget->SetMultipleSelection(arg);
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::SetInitialSelectedFileNames(
  vtkStringArray* filenames)
{
  this->InitialSelecttedFileNames->Reset();

  if (!filenames || !filenames->GetNumberOfValues())
    {
    return;
    }

  this->InitialSelecttedFileNames->DeepCopy(filenames);
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::Cancel()
{
  this->FileNames->Reset();
  this->SetFileName(NULL);

  if(this->GetApplication()->GetSaveUserInterfaceGeometry())
    {
    this->SaveGeometryToRegistry();
    }
  
  this->Superclass::Cancel();
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::OK()
{
  this->FileNames->Reset();

  int res = this->ChooseDirectory ? this->DirectoryOK() : this->FileOK();
  if (!res || !this->GetNumberOfFileNames())
    {
    this->FileNames->Reset();
    this->SetFileName(NULL);
    return;
    }

  this->SetFileName(this->GetNthFileName(0));
  this->GenerateLastPath(this->GetFileName());

  if(this->GetApplication()->GetSaveUserInterfaceGeometry())
    {
    this->SaveGeometryToRegistry();
    }

  this->Superclass::OK();
}

//----------------------------------------------------------------------------
int vtkKWFileBrowserDialog::DirectoryOK()
{
  if (!this->FileBrowserWidget->GetDirectoryExplorer()->HasSelection())
    {
    return 0;
    }

  int num_files = this->FileBrowserWidget->GetDirectoryExplorer()->
    GetNumberOfSelectedDirectories();

  for (int i = 0; i < num_files; i++)
    {
    this->FileNames->InsertNextValue(this->FileBrowserWidget->
      GetDirectoryExplorer()->GetNthSelectedDirectory(i));
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkKWFileBrowserDialog::FileOK()
{
  int num_files = 
    this->FileBrowserWidget->GetFileListTable()->GetNumberOfSelectedFileNames();

  // More than one file selected

  if (num_files > 1)
    {
    vtksys_stl::string selfile;
    for (int i = 0; i < num_files; i++)
      {
      selfile = 
        this->FileBrowserWidget->GetFileListTable()->GetNthSelectedFileName(i);
      if (vtksys::SystemTools::FileIsDirectory(selfile.c_str()))
        {
        vtkKWMessageDialog::PopupMessage(
          this->GetApplication(), this, 
          ks_("File Browser|Title|Error!"),
          k_("A directory can't be part of the selection."), 
          vtkKWMessageDialog::ErrorIcon | vtkKWMessageDialog::InvokeAtPointer);
        return 0;
        }
      this->FileNames->InsertNextValue(selfile.c_str());
      }

    if (this->SaveDialog)
      {
      if (!this->ConfirmOverwrite(
         this->FileBrowserWidget->GetFileListTable()->GetNthSelectedFileName(0)))
        {
        return 0;
        }
      }

    return 1;
    }

  // If the filename box has a value, check if it is a valid file  
  // This also happens if there was only one single file selected
  // since its name get copied over automatically to the filenametext

  if (this->FileNameText->GetValue() && *(this->FileNameText->GetValue()))
    {
    char * realname = vtksys::SystemTools::RemoveChars(
      this->FileNameText->GetValue(), "\r\n\t");
    if (!realname || !*(realname))
      {
      this->FileNameText->SetValue(NULL);
      return 0;
      }

    // Here we check if the filename box is in the multiple selection mode

    if(this->GetMultipleSelection() && this->OpenMultipleFileNames(realname))
      {
      return 1;
      }
    
    // Allow user to input a full or relative path directly in the filename box

    vtksys_stl::string fullname;
    if (vtksys::SystemTools::FileIsFullPath(realname))
      {
      fullname = realname;
      }
    else
      {
      fullname = 
        this->FileBrowserWidget->GetFileListTable()->GetParentDirectory();
      if (!KWFileBrowser_HasTrailingSlash(fullname.c_str()))
        {
        fullname.append(KWFileBrowser_PATH_SEPARATOR);
        }
      fullname = fullname.append(realname);
      }

    fullname = vtksys::SystemTools::CollapseFullPath(fullname.c_str());

    // If this is a directory, open the node
    // "vtksys::SystemTools::FileIsDirectory(fullname)" does not
    // recognize "C:" or "C:/", so using vtkDirectory to check

    if (vtksys::SystemTools::FileExists(fullname.c_str()))
      {
      vtkDirectory *dir = vtkDirectory::New();
      int result = dir->Open(fullname.c_str());
      dir->Delete();
      if (result)
        {
        this->FileBrowserWidget->OpenDirectory(fullname.c_str());
        return 0;
        }
      }

    // Append DefaultExtension if it is set, and if the input filename
    // has no extension 

    if (this->DefaultExtension &&
        (!vtksys::SystemTools::FileExists(fullname.c_str()) 
        || this->SaveDialog ))
      {
      vtksys_stl::string ext = 
        vtksys::SystemTools::GetFilenameExtension(fullname.c_str());
      if (ext.size() == 0)
        {
        const char *extensions = 
         this->GetFileBrowserWidget()->GetFileListTable()->GetFileExtensions();
        if (extensions && *extensions)
          {
          vtksys_stl::vector<vtksys_stl::string> extensions_v;
          vtksys::SystemTools::Split(extensions, extensions_v, ' ');
          vtksys_stl::string firstext = *extensions_v.begin();
          if (firstext.size() >= 2 && firstext[0] == '.')
            {
            fullname.append(firstext);
            }
          else
            {
            fullname.append(this->DefaultExtension);
            }
          }
        else
          {
          fullname.append(this->DefaultExtension);
          }
        }
      }

    // If the file already exists, confirm for Save, or OK

    if (vtksys::SystemTools::FileExists(fullname.c_str()))
      {
      if (this->SaveDialog && !this->ConfirmOverwrite(fullname.c_str()))
        {
        return 0;
        }

      this->FileNames->InsertNextValue(
        KWFileBrowser_GetUnixPath(fullname.c_str()));
      return 1;
      }

    // File in the filename box does not exist: if SaveDialog, OK;
    // otherwise, use the filename text as file pattern to reload
    // the file table

    if (this->SaveDialog)
      {
      this->FileNames->InsertNextValue(
        KWFileBrowser_GetUnixPath(fullname.c_str()));
      return 1;
      }
    
    vtksys_stl::string fullPattern = realname;
    if(fullPattern.find("*") == vtksys_stl::string::npos &&
      fullPattern.find("?") == vtksys_stl::string::npos)
      {
      this->FileBrowserWidget->FilterFilesByExtensions(
        this->Internals->CurrentFileExtensions.c_str());
      }
      else
      {
      this->FileBrowserWidget->GetFileListTable()->ShowFileList(
        this->FileBrowserWidget->GetFileListTable()->GetParentDirectory(),
        fullPattern.c_str(), NULL);
      }
    return 0;
    }

  // If the filename box is empty, but there is a selected item in the 
  // file list, it has to be a directory

  if (num_files == 1)
    {
    vtksys_stl::string fullname = 
      this->FileBrowserWidget->GetFileListTable()->GetNthSelectedFileName(0);
    if (vtksys::SystemTools::FileIsDirectory(fullname.c_str()))
      {
      this->FileBrowserWidget->GetDirectoryExplorer()->OpenDirectory(
        fullname.c_str());
      }
    }

  // Refresh the filetable list with current extensions
  this->FileBrowserWidget->FilterFilesByExtensions(
    this->Internals->CurrentFileExtensions.c_str());

  return 0;
}

//----------------------------------------------------------------------------
int vtkKWFileBrowserDialog::ConfirmOverwrite(const char* filename)
{
  vtksys_stl::string message = "The file, ";
  message.append(filename).append(
    ", already exists. \n Do you want to overwrite it?");
  return vtkKWMessageDialog::PopupYesNo( 
          this->GetApplication(), 
          this, 
          ks_("File Browser Dialog|Title|Save file"),
          k_(message.c_str()),
          vtkKWMessageDialog::WarningIcon | 
          vtkKWMessageDialog::InvokeAtPointer);
}

//----------------------------------------------------------------------------
int vtkKWFileBrowserDialog::OpenMultipleFileNames(const char* inputnames)
{
  if(!inputnames || !(*inputnames) || !this->GetMultipleSelection())
    {
    return 0;
    }

  vtksys_stl::string filenames = inputnames;

  // If there are multiple filenames input, the filenames will/have to be
  // put in double quote and separated by space

  if(filenames.find("\"") == vtksys_stl::string::npos)
    {
    return 0;
    }

  vtksys_stl::string parentDir = 
    this->FileBrowserWidget->GetFileListTable()->GetParentDirectory();
  if (!KWFileBrowser_HasTrailingSlash(parentDir.c_str()))
    {
    parentDir.append(KWFileBrowser_PATH_SEPARATOR);
    }

  vtksys::RegularExpression filenamesregexp("\"[^\"]+\"");
  vtksys_stl::string filename;
  vtksys_stl::string fullfilename;

  while (filenamesregexp.find(filenames))
    {
    filename = filenamesregexp.match(0);
    filename = vtksys::SystemTools::RemoveChars(
      filename.c_str(), "\"");
    fullfilename = parentDir;
    fullfilename.append(filename);

    if (vtksys::SystemTools::FileIsDirectory(fullfilename.c_str()))
      {
      vtkKWMessageDialog::PopupMessage(
        this->GetApplication(), this, 
        ks_("File Browser|Title|Error!"),
        k_("A directory can't be part of the multiple filenames."), 
        vtkKWMessageDialog::ErrorIcon | vtkKWMessageDialog::InvokeAtPointer);
      return 0;
      }

    if (vtksys::SystemTools::FileExists(fullfilename.c_str()))
      {
      if (this->SaveDialog && !this->ConfirmOverwrite(fullfilename.c_str()))
        {
        return 0;
        }
      }
    else if(!this->SaveDialog)
      {
      vtksys_stl::string message;
      message.append("The input file name (").append(filename).append(
        ") does not exist!");
      vtkKWMessageDialog::PopupMessage(
        this->GetApplication(), this, 
        ks_("File Browser|Title|Error!"),
        k_(message.c_str()), 
        vtkKWMessageDialog::ErrorIcon | vtkKWMessageDialog::InvokeAtPointer);
      return 0;
      }

    this->FileNames->InsertNextValue(fullfilename.c_str());

    filenames = 
      filenames.substr(filenamesregexp.end(), filenames.length());
    }

  return this->FileNames->GetNumberOfValues() > 0;
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::SetFileName(const char *arg)
{
  if (this->FileName == NULL && !this->FileNames->GetNumberOfValues() && 
      arg == NULL) 
    { 
    return;
    }

  vtksys_stl::string currPath;
  if(arg)
    {
    currPath = arg;
    vtksys::SystemTools::ConvertToUnixSlashes(currPath);
    }

  if (this->FileName && arg &&
    KWFileBrowser_ComparePath(currPath.c_str(), this->FileName))
    {  
    return;
    }

  if (this->FileName) 
    {  
    delete [] this->FileName; 
    }

  if (arg)
    {
    this->FileName = new char[currPath.size()+1];
    strcpy(this->FileName,currPath.c_str());
    if(this->FileNames->GetNumberOfValues()==0 ||
      (this->GetNumberOfFileNames()>0 &&
      !KWFileBrowser_ComparePath(
      this->FileName, this->GetNthFileName(0))))
      {
      this->FileNames->Reset();
      this->FileNames->InsertNextValue(this->FileName);
      }

    if(!this->LastPath)
      {
      this->SetLastPath(vtksys::SystemTools::GetFilenamePath(
        this->FileName).c_str());
      }

    if(!this->InitialFileName)
      {
      this->SetInitialFileName(vtksys::SystemTools::GetFilenameName(
        this->FileName).c_str());
      }
    }
  else
    {
    this->FileName = NULL;
    this->FileNames->Reset();
    }

  this->Modified();

  this->InvokeFileNameChangedCommand(this->FileName);
} 

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::SetFileNameChangedCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->FileNameChangedCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::InvokeFileNameChangedCommand(const char *filename)
{
  if (this->GetApplication() &&
      this->FileNameChangedCommand && *this->FileNameChangedCommand)
    {
    this->Script("%s {%s}", 
                 this->FileNameChangedCommand, filename ? filename : "");
    }

  this->InvokeEvent(
    vtkKWFileBrowserDialog::FileNameChangedEvent, (void*)filename);
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::FileNameEditingCallback(const char* filename)
{
  if(filename && *filename && 
    strcmp(this->Internals->CurrentSelectedFileNames.c_str(),
    filename) == 0)
    {
    return;
    }

 this->Internals->IsEditingFileName = 1;
 this->FileBrowserWidget->GetFileListTable()->ClearSelection();
 this->FileBrowserWidget->GetFileListTable()->ScrollToFile(filename);
 this->FileBrowserWidget->GetDirectoryExplorer()->ScrollToDirectory(filename);
 this->Internals->CurrentSelectedFileNames = "";
 this->Internals->IsEditingFileName = 0;
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::FileTypeChangedCallback(
  const char* fileextensions)
{
  if (this->FileBrowserWidget->IsCreated() 
      && fileextensions && *fileextensions)
    {
    vtksys_stl::string fileexts = fileextensions;

    vtksys_stl::string::size_type pos1 = fileexts.rfind("(");
    vtksys_stl::string::size_type pos2 = fileexts.rfind(")");
    if (pos1 != vtksys_stl::string::npos && pos2 != vtksys_stl::string::npos)
      {
      fileexts = fileexts.substr(pos1+1, pos2-pos1-1).c_str();
      this->FileBrowserWidget->FilterFilesByExtensions(fileexts.c_str());
      this->Internals->CurrentFileExtensions = fileexts;
      }
    }  
}

//----------------------------------------------------------------------------
char* vtkKWFileBrowserDialog::GetLastPath()
{
  if(this->LastPath)
    {
    return KWFileBrowser_GetUnixPath(this->LastPath);
    }
  return NULL;
}

//----------------------------------------------------------------------------
const char* vtkKWFileBrowserDialog::GenerateLastPath(
  const char* path)
{
  if (path && *path)
    {
    vtksys_stl::string s(path);
    vtksys_stl::string p = vtksys::SystemTools::GetFilenamePath(s);
    this->SetLastPath(p.c_str());
    }
  else
    {
    this->SetLastPath(NULL);
    }
    
  return KWFileBrowser_GetUnixPath(this->LastPath);
}

//----------------------------------------------------------------------------
int vtkKWFileBrowserDialog::GetNumberOfFileNames()
{
  return this->FileNames ? this->FileNames->GetNumberOfValues() : 0;
}

//----------------------------------------------------------------------------
const char *vtkKWFileBrowserDialog::GetNthFileName(int i)
{
  if (!this->FileNames || i < 0 || i >= this->FileNames->GetNumberOfValues())
    {
    return NULL;
    }

  return this->FileNames->GetValue(i);
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::SaveLastPathToRegistry(const char* key)
{
  if (this->IsCreated() && this->GetLastPath())
    {
    this->GetApplication()->SetRegistryValue(
      1, "RunTime", key, this->GetLastPath());
    }
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::RetrieveLastPathFromRegistry(const char* key)
{
  if (this->IsCreated())
    {
    char buffer[1024];
    if (this->GetApplication()->GetRegistryValue(1, "RunTime", key, buffer) &&
        *buffer)
      {
      this->SetLastPath(buffer);
      }  
    }
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::ProcessDirectorySelectedEvent(
  const char* fullname)
{
  if (fullname && *fullname && 
      vtksys::SystemTools::FileIsDirectory(fullname))
    {
    this->FileNameText->SetValue("");    
    }
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::ProcessFileSelectionChangedEvent(
  const char* fullname)
{
  if(this->Internals->IsEditingFileName)
    {
    return;
    }

  this->Internals->CurrentSelectedFileNames = "";
  if(this->GetMultipleSelection() && 
    this->FileBrowserWidget->GetFileListTable()->
    GetNumberOfSelectedFileNames()>1)
    {
    vtkKWFileListTable* fileList = 
      this->FileBrowserWidget->GetFileListTable();
    this->Internals->CurrentSelectedFileNames.append("\"").append(
      vtksys::SystemTools::GetFilenameName(
      fileList->GetNthSelectedFileName(0))).append("\"");

    for(int i=1; i<fileList->GetNumberOfSelectedFileNames();i++)
      {
      this->Internals->CurrentSelectedFileNames.append(" \"").append(
        vtksys::SystemTools::GetFilenameName(
        fileList->GetNthSelectedFileName(i))).append("\"");
      }
    
    this->FileNameText->SetValue(
      this->Internals->CurrentSelectedFileNames.c_str());
    }
  else
    {
    if (!fullname || !*fullname)
      //|| vtksys::SystemTools::FileIsDirectory(fullname))
      {
      this->FileNameText->SetValue("");    
      }  
    else if (vtksys::SystemTools::FileExists(fullname))
      {  
      this->Internals->CurrentSelectedFileNames = 
        vtksys::SystemTools::GetFilenameName(fullname);
      this->FileNameText->SetValue(
        (vtksys::SystemTools::GetFilenameName(fullname)).c_str());
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::ProcessFileDoubleClickedEvent(
  const char* fullname)
{
  if (fullname && *fullname && 
      vtksys::SystemTools::FileExists(fullname) && 
      !vtksys::SystemTools::FileIsDirectory(fullname))
    {
    this->OK();  
    }
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::AddCallbackCommandObservers()
{
  this->Superclass::AddCallbackCommandObservers();

  this->AddCallbackCommandObserver(
    this->FileBrowserWidget->GetDirectoryExplorer(),
    vtkKWDirectoryExplorer::DirectorySelectedEvent);

  this->AddCallbackCommandObserver(
    this->FileBrowserWidget->GetFileListTable(),
    vtkKWFileListTable::FileSelectionChangedEvent);

  this->AddCallbackCommandObserver(
    this->FileBrowserWidget->GetFileListTable(),
    vtkKWFileListTable::FileDoubleClickedEvent);
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::RemoveCallbackCommandObservers()
{
  this->Superclass::RemoveCallbackCommandObservers();

  this->RemoveCallbackCommandObserver(
    this->FileBrowserWidget->GetDirectoryExplorer(),
    vtkKWDirectoryExplorer::DirectorySelectedEvent);

  this->RemoveCallbackCommandObserver(
    this->FileBrowserWidget->GetFileListTable(),
    vtkKWFileListTable::FileSelectionChangedEvent);

  this->RemoveCallbackCommandObserver(
    this->FileBrowserWidget->GetFileListTable(),
    vtkKWFileListTable::FileDoubleClickedEvent);
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::ProcessCallbackCommandEvents(vtkObject *caller,
                                                          unsigned long event,
                                                          void *calldata)
{
  const char * fullname = static_cast<const char*>(calldata);
  if (caller == this->FileBrowserWidget->GetFileListTable())
    {
    if (event == vtkKWFileListTable::FileSelectionChangedEvent)
      {
      this->ProcessFileSelectionChangedEvent(fullname);
      }
    else if (event == vtkKWFileListTable::FileDoubleClickedEvent)
      {
      this->ProcessFileDoubleClickedEvent(fullname);
      }
    }
  else if (caller == this->FileBrowserWidget->GetDirectoryExplorer() &&
           event == vtkKWDirectoryExplorer::DirectorySelectedEvent)
    {
    this->ProcessDirectorySelectedEvent(fullname);
    }
  
  this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->PropagateEnableState(this->FileBrowserWidget);
  this->PropagateEnableState(this->BottomFrame);
  this->PropagateEnableState(this->FileNameLabel);
  this->PropagateEnableState(this->FileNameText);
  this->PropagateEnableState(this->FileTypesLabel);
  this->PropagateEnableState(this->FileTypesBox);
  this->PropagateEnableState(this->OKButton);
  this->PropagateEnableState(this->CancelButton);
  this->PropagateEnableState(this->PreviewFrame);
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "DefaultExtension: " << 
    (this->DefaultExtension?this->DefaultExtension:"none")
     << endl;
  os << indent << "FileTypes: " << (this->FileTypes?this->FileTypes:"none") 
     << endl;
  os << indent << "InitialFileName: " 
     << (this->InitialFileName?this->InitialFileName:"none") 
     << endl;
  os << indent << "FileName: " 
    << (this->FileName?this->FileName:"none") 
    << endl;
  os << indent << "LastPath: " << (this->LastPath?this->LastPath:"none")
     << endl;
  os << indent << "SaveDialog: " << this->GetSaveDialog() << endl;
  os << indent << "ChooseDirectory: " << this->GetChooseDirectory() << endl;
  os << indent << "MultipleSelection: " << this->GetMultipleSelection() << endl;
  os << indent << "NumberOfFileNames: " << this->GetNumberOfFileNames() << endl;
  os << indent << "FileNames:  (" << this->GetFileNames() << ")\n";
  indent = indent.GetNextIndent();
  for(int i = 0; i < this->FileNames->GetNumberOfValues(); i++)
    {
    os << indent << this->FileNames->GetValue(i) << "\n";
    }
  os << indent << "InitialSelecttedFileNames:  (" 
    << this->InitialSelecttedFileNames << ")\n";
  indent = indent.GetNextIndent();
  for(int j = 0; j < this->InitialSelecttedFileNames->GetNumberOfValues(); j++)
    {
    os << indent << this->InitialSelecttedFileNames->GetValue(j) << "\n";
    }
}
