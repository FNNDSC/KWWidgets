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
#include "vtkKWFileListTable.h"
#include "vtkKWInternationalization.h"
#include "vtkKWLabel.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWPushButton.h"

#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
#include "vtkCallbackCommand.h"

#include <vtksys/RegularExpression.hxx>
#include <vtksys/SystemTools.hxx>

#ifdef _WIN32
#define KWFileBrowser_PATH_SEPARATOR "\\"
#else
#define KWFileBrowser_PATH_SEPARATOR "/"
#endif
#define KWFileBrowser_UNIX_ROOT_DIRECTORY "/"

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWFileBrowserDialog );
vtkCxxRevisionMacro(vtkKWFileBrowserDialog, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
class vtkKWFileBrowserDialogInternals
{
public:

  vtkKWFileBrowserDialogInternals()
  {
    this->CurrentFileExtensions = ".*";
  }
  
  vtksys_stl::string CurrentFileExtensions;
};

//----------------------------------------------------------------------------
vtkKWFileBrowserDialog::vtkKWFileBrowserDialog()
{
  this->FileTypes         = NULL;
  this->LastPath          = NULL;
  this->PreviewFrame      = NULL;
  this->InitialFileName   = NULL;
  this->DefaultExtension  = NULL;

  this->SaveDialog       = 0;
  this->ChooseDirectory  = 0;
  this->FileNames        = vtkStringArray::New();

  this->SetTitle("Open File");
  this->SetFileTypes("{{Text Document} {.txt}}");

  this->FileNameChangedCommand      = NULL;

  this->Internals          = new vtkKWFileBrowserDialogInternals;

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
  this->SetFileTypes(NULL);
  this->SetLastPath(NULL);
  this->SetInitialFileName(NULL);
  this->SetDefaultExtension(NULL);
  this->SetLastPath(NULL);

  if (this->FileNames)
    {
    this->FileNames->Delete();
    this->FileNames = NULL;
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
  // Show/Hide components

  if (this->ChooseDirectory)
    {
    this->FileBrowserWidget->DirectoryExplorerVisibilityOn();
    this->FileBrowserWidget->FavoriteDirectoriesFrameVisibilityOff();
    this->FileBrowserWidget->FileListTableVisibilityOff();
    this->SetMinimumSize(300, 300);
    this->SetSize(500, 400);
    }
  else
    {
    this->FileBrowserWidget->FavoriteDirectoriesFrameVisibilityOn();
    this->FileBrowserWidget->FileListTableVisibilityOn();
    this->SetMinimumSize(700, 400);
    this->SetSize(808, 455);
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

    this->CreatePreviewFrame();
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
    this->Script("pack %s -side top -fill x -padx 2 -pady 2",
                 this->PreviewFrame->GetWidgetName());
    }
  else
    {
    this->Script("pack forget %s",
                 this->PreviewFrame->GetWidgetName());
    }
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::Display()
{
  this->Superclass::Display();
  
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
  
  this->PopulateFileTypes(); 

  if (this->InitialFileName && *this->InitialFileName)
    {
    this->FileNameText->SetValue(this->InitialFileName);
    }

  this->FileBrowserWidget->SetFocusToDirectoryExplorer();
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::PopulateFileTypes()
{ 
  this->FileTypesBox->DeleteAllValues();

  if (!this->GetFileTypes())
    {
    this->FileTypesBox->SetValue("All Files (.*)");
    return;
    }
    
  vtksys::RegularExpression filetyperegexp("({{[^}]+} {[^}]+}})");
  vtksys_stl::vector<vtksys_stl::string> filetypes;
  vtksys_stl::string filetypetext;
  vtksys_stl::string filetypeext;
  bool has_default = false;
  vtksys_stl::string defaultValue = "";

  vtksys_stl::string strfiletypes = this->GetFileTypes();
  while (filetyperegexp.find(strfiletypes))
    {
    filetypetext = filetyperegexp.match(0);
    filetypetext = vtksys::SystemTools::RemoveChars(
      filetypetext.c_str(), "{{");
    
    filetypes.clear();  
    vtksys::SystemTools::Split(filetypetext.c_str(), filetypes, '}');
    if (filetypes.size() > 1)
      {
      filetypetext = filetypes.front().c_str();
      filetypeext = (*(++filetypes.begin())).c_str();
      filetypeext = vtksys::SystemTools::RemoveChars(
        filetypeext.c_str(), " ");
      filetypetext.append(" (").append(filetypeext.c_str()).append(")");

      // if this is the default extension, remember it for later use

      if (!has_default && 
          this->DefaultExtension && 
          this->DefaultExtension 
          && strcmp(this->DefaultExtension, filetypeext.c_str()) == 0)
        {
        has_default = true;
        defaultValue = filetypetext;
        }

      if (!this->FileTypesBox->HasValue(filetypetext.c_str()))
        {
        this->FileTypesBox->AddValue(filetypetext.c_str());
        }
      }
    strfiletypes = 
      strfiletypes.substr(filetyperegexp.end(), strfiletypes.length());
    }

  if (this->FileTypesBox->GetNumberOfValues() > 0)
    {
    this->FileTypesBox->AddValue("All Files (.*)");
    if (has_default)
      {
      this->FileTypesBox->SetValue(defaultValue.c_str());
      this->FileTypeChangedCallback(defaultValue.c_str());
      return;
      }
    }
  this->FileTypesBox->SetValue("All Files (.*)");
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
void vtkKWFileBrowserDialog::Cancel()
{
  this->FileNames->Reset();
  this->Superclass::Cancel();
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserDialog::OK()
{
  this->FileNames->Reset();

  int res = this->ChooseDirectory ? this->DirectoryOK() : this->FileOK();
  if (!res || !this->GetNumberOfFileNames())
    {
    return;
    }

  this->GenerateLastPath(this->GetFileName());
  this->InvokeFileNameChangedCommand(this->GetFileName());

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
    this->FileBrowserWidget->GetFileListTable()->GetNumberOfSelectedFiles();

  // More than one file selected

  if (num_files > 1)
    {
    vtksys_stl::string selfile;
    for (int i = 0; i < num_files; i++)
      {
      selfile = 
        this->FileBrowserWidget->GetFileListTable()->GetNthSelectedFile(i);
      if (vtksys::SystemTools::FileIsDirectory(selfile.c_str()))
        {
        vtkKWMessageDialog::PopupMessage(
          this->GetApplication(), this, 
          ks_("File Browser|Title|Error!"),
          k_("A directory can't be part of the selection."), 
          vtkKWMessageDialog::ErrorIcon | vtkKWMessageDialog::InvokeAtPointer);
        this->FileNames->Reset();
        return 0;
        }
      this->FileNames->InsertNextValue(selfile.c_str());
      }

#if 0
    if (this->SaveDialog)
      {
      if (!this->ConfirmOverwrite(
         this->FileBrowserWidget->GetFileListTable()->GetNthSelectedFile(0)))
        {
        this->FileNames->Reset();
        return 0;
        }
      }
#endif

    return 1;
    }

  // If the filename box has a value, check if it is a valid file  
  // This also happens if there was only one single file selected
  // since its name get copied over automatically to the filenametext

  if (this->FileNameText->GetValue() && *(this->FileNameText->GetValue()))
    {
    char * realname = vtksys::SystemTools::RemoveChars(
      this->FileNameText->GetValue(), " \r\n\t");
    if (!realname || !*(realname))
      {
      this->FileNameText->SetValue(NULL);
      return 0;
      }

    vtksys_stl::string fullname = 
      this->FileBrowserWidget->GetFileListTable()->GetParentDirectory();
    if (strcmp(fullname.c_str(), KWFileBrowser_UNIX_ROOT_DIRECTORY) != 0)
      {
      fullname.append(KWFileBrowser_PATH_SEPARATOR);
      }
    fullname.append(realname);
      
    if (vtksys::SystemTools::FileExists(fullname.c_str()))
      {
      // If this is a directory, open the node
      if (vtksys::SystemTools::FileIsDirectory(fullname.c_str()))
        {
        this->FileBrowserWidget->OpenDirectory(fullname.c_str());
        return 0;
        }

      // If this is a file, OK
#if 0
      if (this->SaveDialog)
        {
        if (!this->ConfirmOverwrite(fullname.c_str()))
          {
          return 0;
          }
        }
#endif
      this->FileNames->InsertNextValue(fullname.c_str());
      return 1;
      }

    // File in the filename box does not exist

    this->FileBrowserWidget->GetFileListTable()->ShowFileList(
      this->FileBrowserWidget->GetFileListTable()->GetParentDirectory(),
      realname, NULL);
    return 0;
    }

  // If the filename box is empty, but there is a selected item in the 
  // file list, it has to be a directory

  if (num_files == 1)
    {
    vtksys_stl::string fullname = 
      this->FileBrowserWidget->GetFileListTable()->GetNthSelectedFile(0);
    if (vtksys::SystemTools::FileIsDirectory(fullname.c_str()))
      {
      this->FileBrowserWidget->GetDirectoryExplorer()->OpenDirectory(
        fullname.c_str());
      }
    }

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
void vtkKWFileBrowserDialog::FileTypeChangedCallback(
  const char* fileextensions)
{
  if (this->FileBrowserWidget->IsCreated() 
      && fileextensions && *fileextensions)
    {
    if (strcmp(this->Internals->CurrentFileExtensions.c_str(), 
               fileextensions)!=0)
      {
      this->FileBrowserWidget->FilterFilesByExtensions(fileextensions);
      this->Internals->CurrentFileExtensions = fileextensions;
      }
    }  
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
    
  return this->LastPath;
}

//----------------------------------------------------------------------------
const char* vtkKWFileBrowserDialog::GetFileName()
{
  return this->GetNthFileName(0);
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
    vtkErrorMacro(<< this->GetClassName()
                  << " index for GetNthFileName is out of range");
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
void vtkKWFileBrowserDialog::ProcessDirectoryChangedEvent(
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
  if (!fullname || !*fullname || 
      vtksys::SystemTools::FileIsDirectory(fullname))
    {
    this->FileNameText->SetValue("");    
    } 
  else if (vtksys::SystemTools::FileExists(fullname))
    {  
    this->FileNameText->SetValue(
      (vtksys::SystemTools::GetFilenameName(fullname)).c_str());
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
    vtkKWDirectoryExplorer::DirectoryChangedEvent);

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
    vtkKWDirectoryExplorer::DirectoryChangedEvent);

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
           event == vtkKWDirectoryExplorer::DirectoryChangedEvent)
    {
    this->ProcessDirectoryChangedEvent(fullname);
    }
  
  this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);
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
}
