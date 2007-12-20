/*=========================================================================

  Module:    $RCSfile: vtkKWFileBrowserWidget.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkKWFileBrowserWidget.h"

#include "vtkKWDirectoryExplorer.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWFavoriteDirectoriesFrame.h"
#include "vtkKWFileListTable.h"
#include "vtkKWFrame.h"
#include "vtkKWInternationalization.h"
#include "vtkKWLabel.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWOptions.h"
#include "vtkKWPushButton.h"
#include "vtkKWSimpleEntryDialog.h"
#include "vtkKWSplitFrame.h"
#include "vtkKWTree.h"
#include "vtkKWTreeWithScrollbars.h"

#include "vtkObjectFactory.h"
#include <vtksys/SystemTools.hxx>
#include <vtksys/stl/string>

#ifdef _WIN32
#include "vtkKWWin32RegistryHelper.h"
#include "vtkWindows.h" //for GetLogicalDrives on Windows
#include <shellapi.h>
#include <shlobj.h>
#endif

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWFileBrowserWidget );
vtkCxxRevisionMacro(vtkKWFileBrowserWidget, "$Revision: 1.17 $");

//----------------------------------------------------------------------------
class vtkKWFileBrowserWidgetInternals
{
public:
  
  vtkKWFileBrowserWidgetInternals()
  {
    this->CurrentFileExts = ".*";
  }
  
  vtksys_stl::string CurrentFileExts;
};

//----------------------------------------------------------------------------
vtkKWFileBrowserWidget::vtkKWFileBrowserWidget()
{
  this->Internals = new vtkKWFileBrowserWidgetInternals;
  
  this->DirFileFrame              = vtkKWSplitFrame::New();
  this->MainFrame                 = vtkKWSplitFrame::New();
  this->FavoriteDirectoriesFrame  = vtkKWFavoriteDirectoriesFrame::New();
  this->DirectoryExplorer         = vtkKWDirectoryExplorer::New();
  this->FileListTable             = vtkKWFileListTable::New(); 
  
  this->MultipleSelection                  = 0;
  this->FavoriteDirectoriesFrameVisibility = 1;
  this->DirectoryExplorerVisibility        = 1;
  this->FileListTableVisibility            = 1;

  this->SelectionForegroundColor[0] = 1.0;
  this->SelectionForegroundColor[1] = 1.0;
  this->SelectionForegroundColor[2] = 1.0;
  
  this->SelectionBackgroundColor[0] = 10.0 / 255.0;
  this->SelectionBackgroundColor[1] = 36.0 / 255.0;
  this->SelectionBackgroundColor[2] = 106.0 / 255.0;

  this->OutOfFocusSelectionForegroundColor[0] = 0.0;
  this->OutOfFocusSelectionForegroundColor[1] = 0.0;
  this->OutOfFocusSelectionForegroundColor[2] = 0.0;

  this->OutOfFocusSelectionBackgroundColor[0] = 0.926;
  this->OutOfFocusSelectionBackgroundColor[1] = 0.914;
  this->OutOfFocusSelectionBackgroundColor[2] = 0.847;
}

//----------------------------------------------------------------------------
vtkKWFileBrowserWidget::~vtkKWFileBrowserWidget()
{
  this->FileListTable->RemoveBindingFromInternalWidget(
    "<FocusIn>", this, "FileTableFocusInCallback");
  this->FileListTable->RemoveBindingFromInternalWidget(
    "<FocusOut>", this, "FileTableFocusOutCallback");
  this->FavoriteDirectoriesFrame->Delete();
  this->DirectoryExplorer->Delete();
  this->FileListTable->Delete();
  this->DirFileFrame->Delete();
  this->MainFrame->Delete();
    
  if (this->Internals)
    {
    delete this->Internals;
    }
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();
  
  // Main Frame

  this->MainFrame->SetParent(this);
  this->MainFrame->Create();
  this->MainFrame->SetFrame1MinimumSize(50);
  this->MainFrame->SetFrame2MinimumSize(300);
  this->MainFrame->SetSeparatorPosition(0.2);
  this->MainFrame->SetSeparatorVisibility(1);
  this->MainFrame->SetHeight(345);

  this->Script("pack %s -fill both -expand true",
               this->MainFrame->GetWidgetName());

  this->CreateDirectoryExplorer();
  this->CreateFileListTable(); 
  this->CreateFavoriteDirectoriesFrame();
  
  this->Pack();
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::Pack()
{
  this->MainFrame->UnpackChildren();
  
  if (this->DirectoryExplorerVisibility || this->FileListTableVisibility)
    {
    this->DirFileFrame->SetFrame1Visibility(this->DirectoryExplorerVisibility);
    this->DirFileFrame->SetFrame2Visibility(this->FileListTableVisibility);
    
    this->Script("pack %s -side top -fill both -expand true",
                 this->DirFileFrame->GetWidgetName());
    }
  else
    {
    this->MainFrame->SetFrame2Visibility(0);
    }

  this->MainFrame->SetFrame1Visibility(
    this->FavoriteDirectoriesFrameVisibility);
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::CreateDirectoryExplorerAndFileListTableFrame()
{
  if (this->DirectoryExplorerVisibility || this->FileListTableVisibility)
    {
    if (!this->DirFileFrame->IsCreated())
      {
      this->DirFileFrame->SetParent(this->MainFrame->GetFrame2());
      this->DirFileFrame->Create();
      this->DirFileFrame->SetFrame1MinimumSize(250);
      this->DirFileFrame->SetFrame2MinimumSize(200);
      this->DirFileFrame->SetSeparatorPosition(0.5);
      this->DirFileFrame->SetSeparatorVisibility(1);
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::CreateDirectoryExplorer()
{
  if (this->DirectoryExplorerVisibility)
    {
    if (!this->DirFileFrame->IsCreated())
      {
      this->CreateDirectoryExplorerAndFileListTableFrame();
      }
    if (!this->DirectoryExplorer->IsCreated())
      {
      this->DirectoryExplorer->SetParent(this->DirFileFrame->GetFrame1());
      this->DirectoryExplorer->Create();

      this->DirectoryExplorer->AddBindingToInternalWidget(
        "<FocusIn>", this, "DirectoryTreeFocusInCallback");
      this->DirectoryExplorer->AddBindingToInternalWidget(
        "<FocusOut>", this, "DirectoryTreeFocusOutCallback");
        
      this->DirectoryExplorer->SetDirectoryCreatedCommand(
        this, "DirectoryCreatedCallback");
      this->DirectoryExplorer->SetDirectorySelectedCommand(
        this, "DirectorySelectedCallback");
      this->DirectoryExplorer->SetDirectoryOpenedCommand(
        this, "DirectoryOpenedCallback");
      this->DirectoryExplorer->SetDirectoryClosedCommand(
        this, "DirectoryClosedCallback");
      this->DirectoryExplorer->SetDirectoryDeletedCommand(
        this, "DirectoryDeletedCallback");
      this->DirectoryExplorer->SetDirectoryRenamedCommand(
        this, "DirectoryRenamedCallback");
      }

    this->UpdateDirectorySelectionColor();
    this->PropagateMultipleSelection();

    this->Script(
      "pack %s -side top -fill both -expand true -padx 1 -pady 1",
      this->DirectoryExplorer->GetWidgetName());
    }
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::CreateFileListTable()
{
  if (this->FileListTableVisibility)
    {
    if (!this->DirFileFrame->IsCreated())
      {
      this->CreateDirectoryExplorerAndFileListTableFrame();
      }
    if (!this->FileListTable->IsCreated())
      {
      this->FileListTable->SetParent(this->DirFileFrame->GetFrame2());
      this->FileListTable->Create();

      this->FileListTable->AddBindingToInternalWidget(
        "<FocusIn>", this, "FileTableFocusInCallback");
      this->FileListTable->AddBindingToInternalWidget(
        "<FocusOut>", this, "FileTableFocusOutCallback");

      this->FileListTable->SetFileDoubleClickedCommand(
        this, "FileDoubleClickedCallback");
      this->FileListTable->SetFileSelectedCommand(
        this, "FileSelectionChangedCallback");
      this->FileListTable->SetFileDeletedCommand(
        this, "FileDeletedCallback");
      this->FileListTable->SetFileRenamedCommand(
        this, "FileRenamedCallback");
      this->FileListTable->SetFolderCreatedCommand(
        this, "FolderCreatedCallback");
      }

    this->UpdateFileSelectionColor();
    this->PropagateMultipleSelection();

    this->Script(
      "pack %s -side top -fill both -expand true -padx 1 -pady 1",
      this->FileListTable->GetWidgetName());
    }
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::CreateFavoriteDirectoriesFrame()
{
  if (this->FavoriteDirectoriesFrameVisibility)
    {
    if (!this->FavoriteDirectoriesFrame->IsCreated())
      {
      this->FavoriteDirectoriesFrame->SetParent(this->MainFrame->GetFrame1());
      this->FavoriteDirectoriesFrame->Create();
      this->FavoriteDirectoriesFrame->SetAddFavoriteDirectoryCommand(
        this, "AddFavoriteDirectoryCallback");
      this->FavoriteDirectoriesFrame->SetFavoriteDirectorySelectedCommand(
        this, "FavoriteDirectorySelectedCallback");
      }

    this->Script("pack %s -fill x -expand true",
                 this->FavoriteDirectoriesFrame->GetWidgetName());
    }
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::SetFavoriteDirectoriesFrameVisibility(int arg)
{
  if (this->FavoriteDirectoriesFrameVisibility == arg)
    {
    return;
    }

  this->FavoriteDirectoriesFrameVisibility = arg;
  this->Modified();

  this->CreateFavoriteDirectoriesFrame();
  this->Pack();
  this->UpdateForCurrentDirectory();
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::SetDirectoryExplorerVisibility(int arg)
{
  if (this->DirectoryExplorerVisibility == arg)
    {
    return;
    }

  this->DirectoryExplorerVisibility = arg;
  this->Modified();

  this->CreateDirectoryExplorer();
  this->Pack();
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::SetFileListTableVisibility(int arg)
{
  if (this->FileListTableVisibility == arg)
    {
    return;
    }

  this->FileListTableVisibility = arg;
  this->Modified();

  this->CreateFileListTable(); 
  this->Pack();
  this->PropagateMultipleSelection();
  this->UpdateForCurrentDirectory();
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::SetMultipleSelection(int arg)
{
  arg = arg ? 1 : 0;
  if (this->MultipleSelection == arg)
    {
    return;
    }

  this->MultipleSelection = arg;
  this->Modified();

  this->PropagateMultipleSelection();
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::PropagateMultipleSelection()
{
  int option = vtkKWOptions::SelectionModeSingle;
  if (this->MultipleSelection)
    {
    option = vtkKWOptions::SelectionModeMultiple;
    }

  if (this->DirectoryExplorer && this->DirectoryExplorer->IsCreated())
    {
    if (!this->FileListTableVisibility)
      {
      this->DirectoryExplorer->SetSelectionMode(option);
      }
    else
      {
      this->DirectoryExplorer->SetSelectionMode(
        vtkKWOptions::SelectionModeSingle);
      }
    }

  if (this->FileListTable && this->FileListTable->IsCreated())
    {
    if (this->MultipleSelection)
      {
      option = vtkKWOptions::SelectionModeExtended;
      }
    this->FileListTable->SetSelectionMode(option);
    }
}

//----------------------------------------------------------------------------
int vtkKWFileBrowserWidget::OpenDirectory(const char* path)
{
  if (this->DirectoryExplorer->IsCreated())
    {
    return this->DirectoryExplorer->OpenDirectory(path);
    }
  return 0; 
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::SetFocusToDirectoryExplorer()
{
  if (this->DirectoryExplorer->IsCreated() && 
      this->DirectoryExplorerVisibility)
    {
    this->DirectoryExplorer->Focus();
    }
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::SetSelectionForegroundColor(
  double r, double g, double b)
{
  if ((r == this->SelectionForegroundColor[0] && 
       g == this->SelectionForegroundColor[1] && 
       b == this->SelectionForegroundColor[2]) ||
      (r < 0.0 || r > 1.0) || (g < 0.0 || g > 1.0) || (b < 0.0 || b > 1.0))
    {
    return;
    }

  this->SelectionForegroundColor[0] = r;
  this->SelectionForegroundColor[1] = g;
  this->SelectionForegroundColor[2] = b;

  this->Modified();

  this->UpdateDirectorySelectionColor();
  this->UpdateFileSelectionColor();
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::SetSelectionBackgroundColor(
  double r, double g, double b)
{
  if ((r == this->SelectionBackgroundColor[0] && 
       g == this->SelectionBackgroundColor[1] && 
       b == this->SelectionBackgroundColor[2]) ||
      (r < 0.0 || r > 1.0) || (g < 0.0 || g > 1.0) || (b < 0.0 || b > 1.0))
    {
    return;
    }

  this->SelectionBackgroundColor[0] = r;
  this->SelectionBackgroundColor[1] = g;
  this->SelectionBackgroundColor[2] = b;

  this->Modified();

  this->UpdateDirectorySelectionColor();
  this->UpdateFileSelectionColor();
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::SetOutOfFocusSelectionForegroundColor(
  double r, double g, double b)
{
  if ((r == this->OutOfFocusSelectionForegroundColor[0] && 
       g == this->OutOfFocusSelectionForegroundColor[1] && 
       b == this->OutOfFocusSelectionForegroundColor[2]) ||
      (r < 0.0 || r > 1.0) || (g < 0.0 || g > 1.0) || (b < 0.0 || b > 1.0))
    {
    return;
    }

  this->OutOfFocusSelectionForegroundColor[0] = r;
  this->OutOfFocusSelectionForegroundColor[1] = g;
  this->OutOfFocusSelectionForegroundColor[2] = b;

  this->Modified();

  this->UpdateDirectorySelectionColor();
  this->UpdateFileSelectionColor();
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::SetOutOfFocusSelectionBackgroundColor(
  double r, double g, double b)
{
  if ((r == this->OutOfFocusSelectionBackgroundColor[0] && 
       g == this->OutOfFocusSelectionBackgroundColor[1] && 
       b == this->OutOfFocusSelectionBackgroundColor[2]) ||
      (r < 0.0 || r > 1.0) || (g < 0.0 || g > 1.0) || (b < 0.0 || b > 1.0))
    {
    return;
    }

  this->OutOfFocusSelectionBackgroundColor[0] = r;
  this->OutOfFocusSelectionBackgroundColor[1] = g;
  this->OutOfFocusSelectionBackgroundColor[2] = b;

  this->Modified();

  this->UpdateDirectorySelectionColor();
  this->UpdateFileSelectionColor();
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::UpdateDirectorySelectionColor(int /*infocus*/)
{
  //if (infocus)
  if (this->DirectoryExplorer->HasFocus())
    {
    this->DirectoryExplorer->SetSelectionBackgroundColor(
      this->SelectionBackgroundColor);
    this->DirectoryExplorer->SetSelectionForegroundColor(
      this->SelectionForegroundColor);
    }
  else if (this->DirectoryExplorer->HasSelection())
    {
    this->DirectoryExplorer->SetSelectionBackgroundColor(
      this->OutOfFocusSelectionBackgroundColor);
    this->DirectoryExplorer->SetSelectionForegroundColor(
      this->OutOfFocusSelectionForegroundColor);
    }
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::UpdateFileSelectionColor(int /*infocus*/)
{
  //if (infocus)
  if (this->FileListTable->HasFocus())
    {
    this->FileListTable->SetSelectionBackgroundColor(
      this->SelectionBackgroundColor);
    this->FileListTable->SetSelectionForegroundColor(
      this->SelectionForegroundColor);
    }
  else if (this->FileListTable->GetNumberOfSelectedFileNames() > 0)
    {
    this->FileListTable->SetSelectionBackgroundColor(
      this->OutOfFocusSelectionBackgroundColor);
    this->FileListTable->SetSelectionForegroundColor(
      this->OutOfFocusSelectionForegroundColor);
    }
}

//----------------------------------------------------------------------------
void  vtkKWFileBrowserWidget::SetFocusToFileListTable()
{
  if (this->FileListTable->IsCreated() && 
      this->FileListTableVisibility)
    {
    this->FileListTable->Focus();
    }
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::FilterFilesByExtensions(
  const char* fileextensions)
{
  if (this->FileListTable->IsCreated() && 
      fileextensions && *fileextensions)
    {
    vtksys_stl::string tmpExts = fileextensions;
    bool bUseExt = false;
    vtksys_stl::vector<vtksys_stl::string> extlist;
    vtksys::SystemTools::Split(tmpExts.c_str(), extlist, ' ');
    if (extlist.size()>0)
      {
      // if there is a .* in the extenstions list, or none of them
      // are extensions, ignore extensions.
      vtksys_stl::vector<vtksys_stl::string>::iterator it;
      for(it = extlist.begin(); it != extlist.end(); it++)
        {
        vtksys_stl::string ext = *it;
        if (ext.size() >= 2 && ext[0] == '.')
          {
          bUseExt = true;
          }
        if (strcmp(ext.c_str(), ".*") == 0)
          {
          bUseExt = false;
          break;
          }
        }
      }

    if (bUseExt)
      {
      this->FileListTable->ShowFileList(
        this->DirectoryExplorer->GetSelectedDirectory(),
        NULL, tmpExts.c_str());
      this->Internals->CurrentFileExts = tmpExts;
      return;
      }
    
    this->FileListTable->ShowFileList(
      this->DirectoryExplorer->GetSelectedDirectory(), NULL, NULL);
    this->Internals->CurrentFileExts = ".*";
    }
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::AddFavoriteDirectoryCallback()
{
  if (!this->DirectoryExplorer->HasSelection())
    {
    vtkKWMessageDialog::PopupMessage(
      this->GetApplication(), this, 
      ks_("File Browser|Title|Error!"),
      k_("Please select a directory first."), 
      vtkKWMessageDialog::ErrorIcon | vtkKWMessageDialog::InvokeAtPointer);
    return;
    }
  
  vtksys_stl::string favoritedir = 
    this->DirectoryExplorer->GetSelectedDirectory();
  vtksys_stl::string defaultname = 
    vtksys::SystemTools::GetFilenameName(favoritedir);
  
  // Check if the favorite folder is already there, 
  // if yes, popup error message

  if (this->FavoriteDirectoriesFrame->HasFavoriteDirectory(
        favoritedir.c_str()))
    {
    vtksys_stl::string message = 
      "The selected directoy has already been added.";
    vtkKWMessageDialog::PopupMessage(
      this->GetApplication(), this, 
      ks_("File Browser|Title|Warning!"),
      message.c_str(), 
      vtkKWMessageDialog::WarningIcon | vtkKWMessageDialog::InvokeAtPointer);
    return;
    }
    
  // Prompt the user for the name of the directory  

  vtkKWSimpleEntryDialog *dlg = vtkKWSimpleEntryDialog::New();
  dlg->SetParent(this);
  dlg->SetMasterWindow(this->GetParentTopLevel());
  dlg->SetDisplayPositionToPointer();
  dlg->SetTitle(ks_("File Browser|Dialog|Title|Add favorite"));
  dlg->SetStyleToOkCancel();
  dlg->Create();
  dlg->GetEntry()->GetLabel()->SetText(
    ks_("File Browser|Dialog|Favorite name:"));
  dlg->GetEntry()->GetWidget()->SetValue(defaultname.c_str());
  dlg->SetText(
    ks_("File Browser|Dialog|Enter a name for this favorite"));
  
  dlg->GetOKButton()->Focus();
  dlg->GetEntry()->GetWidget()->SetBinding("<Return>", dlg, "OK");
  dlg->GetOKButton()->SetBinding("<Return>", dlg, "OK");
  dlg->GetCancelButton()->SetBinding("<Return>", dlg, "Cancel");
  
  int ok = dlg->Invoke();
  vtksys_stl::string favoritename = dlg->GetEntry()->GetWidget()->GetValue();
  dlg->Delete();
  if (ok)
    {
    if (favoritename.empty())
      {
      vtkKWMessageDialog::PopupMessage(
        this->GetApplication(), this, 
        ks_("File Browser|Title|Error!"),
        "You can not enter an empty name!", 
        vtkKWMessageDialog::ErrorIcon | vtkKWMessageDialog::InvokeAtPointer);
      return;
      }

    // Check if the folder text is already used, 
    // if yes, popup error message
    
    if (this->FavoriteDirectoriesFrame->HasFavoriteDirectoryWithName(
          favoritename.c_str()))
      {
      vtksys_stl::string message = 
        "The name for this favorite is already used: ";
      message.append(favoritename.c_str());
      vtkKWMessageDialog::PopupMessage(
        this->GetApplication(), this, 
        ks_("File Browser|Title|Error!"),
        message.c_str(), 
          vtkKWMessageDialog::WarningIcon | 
        vtkKWMessageDialog::InvokeAtPointer);
      return;
      }
    }
  else
    {
    return;
    }    
  
  // Add selected folder to the favorites

  this->FavoriteDirectoriesFrame->AddFavoriteDirectory(
    favoritedir.c_str(), favoritename.c_str());
  this->FavoriteDirectoriesFrame->SelectFavoriteDirectory(
    favoritedir.c_str());
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::FavoriteDirectorySelectedCallback(
  const char* path, const char* text)
{  
  this->DirectoryExplorer->OpenDirectory(path);
  this->SetFocusToDirectoryExplorer();
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::DirectorySelectedCallback(
  const char* fullname)
{
  //  this->UpdateDirectorySelectionColor(1);
  //  this->UpdateFileSelectionColor(0);

  this->UpdateForCurrentDirectory();
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::DirectoryOpenedCallback(
  const char* fullname)
{
  //  this->UpdateDirectorySelectionColor(1);
  // this->UpdateFileSelectionColor(0);
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::DirectoryClosedCallback(
  const char* fullname)
{
  //  this->UpdateDirectorySelectionColor(1);
  //  this->UpdateFileSelectionColor(0);
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::DirectoryCreatedCallback(
  const char* fullname)
{
  this->DirectorySelectedCallback(fullname);
  this->SetFocusToDirectoryExplorer();
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::UpdateForCurrentDirectory()
{
  if (this->FavoriteDirectoriesFrameVisibility)
    {
    const char *sel_dir = this->DirectoryExplorer->GetSelectedDirectory();
    if (sel_dir)
      {
      vtksys_stl::string sel_dir_str(sel_dir);
      this->FavoriteDirectoriesFrame->SelectFavoriteDirectory(sel_dir_str.c_str());
      }
    }
    
  if (this->FileListTableVisibility)
    {
    this->FilterFilesByExtensions(this->Internals->CurrentFileExts.c_str());
    }
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::DirectoryDeletedCallback(
  const char* fullname)
{
  if (this->FavoriteDirectoriesFrameVisibility)
    {
    // Adding a fav is a manual process. Removing should be that way too.
    // this->FavoriteDirectoriesFrame->RemoveFavoriteDirectory(fullname);
    }
  this->SetFocusToDirectoryExplorer();
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::DirectoryRenamedCallback(
  const char* oldname, const char* newname)
{
  if (this->FavoriteDirectoriesFrameVisibility)
    {
    // Adding a fav is a manual process. Removing should be that way too.
    // this->FavoriteDirectoriesFrame->RelocateFavoriteDirectory(
    // oldname, newname);
    }

  this->FileListTable->SetParentDirectory(newname);
  this->SetFocusToDirectoryExplorer();
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::FileDoubleClickedCallback(
  const char* fullname)
{
  if (this->DirectoryExplorerVisibility)
    {
    if (fullname && *fullname && 
        vtksys::SystemTools::FileIsDirectory(fullname) && 
        this->DirectoryExplorer->HasSelection())
      {
      if(this->FavoriteDirectoriesFrameVisibility)
        {
        this->FavoriteDirectoriesFrame->SelectFavoriteDirectory(fullname);
        }
      this->DirectoryExplorer->OpenDirectory(fullname);
      this->SetFocusToFileListTable();
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::FolderCreatedCallback(const char* filename)
{
  if (this->DirectoryExplorerVisibility)
    {
    if (this->DirectoryExplorer->HasSelection())
      {
      this->DirectoryExplorer->OpenDirectory(
        this->DirectoryExplorer->GetSelectedDirectory());
      this->FilterFilesByExtensions(this->Internals->CurrentFileExts.c_str());
      this->SetFocusToFileListTable();
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::FileSelectionChangedCallback(
  const char* fullname)
{
  //this->UpdateDirectorySelectionColor(0);
  //this->UpdateFileSelectionColor(1);
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::FileDeletedCallback(
  const char* fullname, int isDir)
{
  if (fullname && *fullname && isDir)
    {
    vtksys_stl::string path = fullname;
    if (this->DirectoryExplorerVisibility && 
        this->DirectoryExplorer->HasSelection())
      {
      this->DirectoryExplorer->DeleteDirectory(path.c_str());
      }
    if (this->FavoriteDirectoriesFrameVisibility)
      {
      // Adding a fav is a manual process. Removing should be that way too.
      //this->FavoriteDirectoriesFrame->RemoveFavoriteDirectory(path.c_str());
      }
    }
  this->SetFocusToFileListTable();
}
//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::FileRenamedCallback(
  const char* oldname, const char* newname)
{
  if (oldname && *oldname && newname && *newname)
    {
    if (vtksys::SystemTools::FileIsDirectory(newname))
      {
      if (this->DirectoryExplorerVisibility  && 
          this->DirectoryExplorer->HasSelection())
        {
        this->DirectoryExplorer->RenameDirectory(oldname, newname);
        }
      if (this->FavoriteDirectoriesFrameVisibility)
        {
        // Adding a fav is a manual process. Removing should be that way too.
        //this->FavoriteDirectoriesFrame->RenameFavoriteDirectory(
        //oldname, newname);
        }
      }
    this->FileListTable->ShowFileList(
      this->FileListTable->GetParentDirectory(),
      NULL, this->Internals->CurrentFileExts.c_str());
    this->SetFocusToFileListTable();
    }
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::DirectoryTreeFocusInCallback()
{
  if (this->IsAlive())
    {
    this->UpdateDirectorySelectionColor(1);
    }
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::FileTableFocusInCallback()
{
  if (this->IsAlive())
    {
    this->UpdateFileSelectionColor(1);
    }
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::DirectoryTreeFocusOutCallback()
{
  if (this->IsAlive())
    {
    this->UpdateDirectorySelectionColor(0);
    }
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::FileTableFocusOutCallback()
{
  if (this->IsAlive())
    {
    this->UpdateFileSelectionColor(0);
    }
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->PropagateEnableState(this->MainFrame);
  this->PropagateEnableState(this->DirFileFrame);
  this->PropagateEnableState(this->FavoriteDirectoriesFrame);
  this->PropagateEnableState(this->DirectoryExplorer);
  this->PropagateEnableState(this->FileListTable);
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::SetWidth(int width)
{
  if (this->MainFrame)
    {
    this->MainFrame->SetWidth(width);
    }
  else
    {
    this->Superclass::SetWidth(width);
    }
}

//----------------------------------------------------------------------------
int vtkKWFileBrowserWidget::GetWidth()
{
  if (this->MainFrame)
    {
    return this->MainFrame->GetWidth();
    }
  return this->Superclass::GetWidth();
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::SetHeight(int height)
{
  if (this->MainFrame)
    {
    this->MainFrame->SetHeight(height);
    }
  else
    {
    this->Superclass::SetHeight(height);
    }
}

//----------------------------------------------------------------------------
int vtkKWFileBrowserWidget::GetHeight()
{
  if (this->MainFrame)
    {
    return this->MainFrame->GetHeight();
    }
  return this->Superclass::GetHeight();
}

//----------------------------------------------------------------------------
void vtkKWFileBrowserWidget::PrintSelf(
  ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "FavoriteDirectoriesFrameVisibility: " 
     << this->FavoriteDirectoriesFrameVisibility
     << endl;
  os << indent << "DirectoryExplorerVisibility: " 
     << this->DirectoryExplorerVisibility
     << endl;
  os << indent << "FileListTableVisibility: " 
     << this->FileListTableVisibility 
     << endl;
}
