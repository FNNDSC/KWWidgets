/*=========================================================================

  Module:    $RCSfile: vtkKWFavoriteDirectoriesFrame.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkKWFavoriteDirectoriesFrame.h"

#include "vtkKWApplication.h"
#include "vtkDirectory.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWFileBrowserUtilities.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithScrollbar.h"
#include "vtkKWIcon.h"
#include "vtkKWInternationalization.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWOptions.h"
#include "vtkKWPushButton.h"
#include "vtkKWRegistryHelper.h"
#include "vtkKWSimpleEntryDialog.h"
#include "vtkKWToolbar.h"

#include "vtkObjectFactory.h"
#include <vtksys/SystemTools.hxx>
#include <vtksys/stl/string>
#include <vtksys/stl/list>

#define VTK_KW_DIR_REGISTRY_PATHNAME_KEYNAME_PATTERN "Path%02d"
#define VTK_KW_DIR_REGISTRY_LABEL_KEYNAME_PATTERN "Path%02dLabel"

#define VTK_KW_FAVORITE_DIR_KEY "KWFavoriteDirs"

#define VTK_KW_DIR_REGISTRY_MAX_ENTRIES 30
#define VTK_KW_DIR_REGISTRY_MIN_ENTRIES 1

#ifdef _WIN32
#define VTK_KW_WIN32_REGIRSTRY_PLACES_BAR_KEY "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\comdlg32\\PlacesBar"
#define VTK_KW_WIN32_REGIRSTRY_NUM_PLACES 5
#include "vtkKWWin32RegistryHelper.h"
#include "vtkWindows.h" //for GetLogicalDrives on Windows
#include <shellapi.h>
#include <shlobj.h>
#endif

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWFavoriteDirectoriesFrame );
vtkCxxRevisionMacro(vtkKWFavoriteDirectoriesFrame, "$Revision: 1.27 $");

//----------------------------------------------------------------------------
class vtkKWFavoriteDirectoriesFrameInternals
{
public:
  
  vtkKWFavoriteDirectoriesFrameInternals()
    {
      // Some constants

      this->SelectedFavoriteDirectoryBackgroundColor[0] = 0.98;
      this->SelectedFavoriteDirectoryBackgroundColor[1] = 0.98;
      this->SelectedFavoriteDirectoryBackgroundColor[2] = 0.98;   
    }
  
  class FavoriteDirectoryEntry
  {
  public:
    vtksys_stl::string Path;
    vtksys_stl::string Name;

    int IsEqual(const char *path, const char *) 
      { return (path && !strcmp(path, this->Path.c_str())); }
  };

  // Favorite directories list

  typedef vtksys_stl::list<FavoriteDirectoryEntry*> FavoriteDirectoryEntryContainer;
  typedef vtksys_stl::list<FavoriteDirectoryEntry*>::iterator FavoriteDirectoryEntryIterator;

  FavoriteDirectoryEntryContainer FavoriteDirectories;
  
  // Constants
  double SelectedFavoriteDirectoryBackgroundColor[3];
};

//----------------------------------------------------------------------------
vtkKWFavoriteDirectoriesFrame::vtkKWFavoriteDirectoriesFrame()
{
  this->Internals = new vtkKWFavoriteDirectoriesFrameInternals;

  this->MaximumNumberOfFavoriteDirectoriesInRegistry = 15;
  
  this->Toolbar                    = vtkKWToolbar::New();
  this->ContainerFrame   = vtkKWFrameWithScrollbar::New();
  this->AddFavoriteDirectoryButton = vtkKWPushButton::New();
  this->ContextMenu                = NULL;
  this->UseSystemDefaultPlaces     = 0;

  this->AddFavoriteDirectoryCommand      = NULL;
  this->FavoriteDirectorySelectedCommand = NULL;

  this->RegistryKey = NULL;
}

//----------------------------------------------------------------------------
vtkKWFavoriteDirectoriesFrame::~vtkKWFavoriteDirectoriesFrame()
{
  if (this->RegistryKey)
    {
    delete [] this->RegistryKey;
    this->RegistryKey = NULL;
    }

  if (this->AddFavoriteDirectoryCommand)
    {
    delete [] this->AddFavoriteDirectoryCommand;
    this->AddFavoriteDirectoryCommand = NULL;
    }

  if (this->FavoriteDirectorySelectedCommand)
    {
    delete [] this->FavoriteDirectorySelectedCommand;
    this->FavoriteDirectorySelectedCommand = NULL;
    }
      
  if (this->ContextMenu)
    {
    this->ContextMenu->Delete();
    this->ContextMenu = NULL;
    }

  this->AddFavoriteDirectoryButton->Delete();
  this->Toolbar->Delete();
   
  if (this->ContainerFrame)
    {
    if (this->ContainerFrame->GetFrame())
      {
      this->ContainerFrame->GetFrame()->RemoveAllChildren();
      }
    this->ContainerFrame->Delete();
    this->ContainerFrame = NULL;
    }
  
  // Clear internals list

  this->ClearInternalList();

  if (this->Internals)
    {
    delete this->Internals;
    }
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::ClearInternalList()
{
  if (this->Internals)
    {
    vtkKWFavoriteDirectoriesFrameInternals::FavoriteDirectoryEntryIterator
      it = this->Internals->FavoriteDirectories.begin();
    vtkKWFavoriteDirectoriesFrameInternals::FavoriteDirectoryEntryIterator
      end = this->Internals->FavoriteDirectories.end();
    for (; it != end; ++it)
      {
      if (*it)
        {
        delete *it;
        }
      }
    this->Internals->FavoriteDirectories.clear();
    }
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }
  
  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  // Toolbar for Favorite Frame

  this->Toolbar->SetParent(this);
  this->Toolbar->Create();
  this->Toolbar->SetWidth(20);

  // Favorite Add button for Toolbar

  this->AddFavoriteDirectoryButton->SetParent(this->Toolbar->GetFrame());
  this->AddFavoriteDirectoryButton->Create();
  this->AddFavoriteDirectoryButton->SetImageToPredefinedIcon(
    vtkKWIcon::IconFavorites);
  this->AddFavoriteDirectoryButton->SetBalloonHelpString(
    "Add to favorites");
  this->AddFavoriteDirectoryButton->SetCommand(
    this, "AddFavoriteDirectoryCallback");
  this->AddFavoriteDirectoryButton->SetConfigurationOptionAsInt(
    "-takefocus", 0);
  this->Toolbar->AddWidget(this->AddFavoriteDirectoryButton);

  this->Toolbar->SetToolbarAspectToFlat();
  this->Toolbar->SetWidgetsAspectToFlat();

  this->Script("pack %s -side top -anchor nw -fill x",
               this->Toolbar->GetWidgetName());
  
  // Favorite button frame to hold favorite directory buttons.

  this->ContainerFrame->SetParent(this);
  this->ContainerFrame->SetHorizontalScrollbarVisibility(0);
  this->ContainerFrame->Create();
  this->ContainerFrame->SetBackgroundColor(0.5, 0.5, 0.5);
  this->ContainerFrame->SetBorderWidth(1);
  this->ContainerFrame->SetReliefToSunken();

  this->Script("pack %s -side top -fill both -expand true",
               this->ContainerFrame->GetWidgetName());

  // Initialize

  if (!this->GetRegistryKey())
    { 
    this->SetRegistryKey(VTK_KW_FAVORITE_DIR_KEY);
    }

  // Retrieve the favorite directories from registry

  this->RestoreFavoriteDirectoriesFromRegistry();

   this->Update();
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::Update()
{
  this->UpdateEnableState();
}

//----------------------------------------------------------------------------
int vtkKWFavoriteDirectoriesFrame::HasFavoriteDirectoryWithName(
  const char* name)
{
  if (name && *name)
    {
    vtkKWFavoriteDirectoriesFrameInternals::FavoriteDirectoryEntryIterator 
      it = this->Internals->FavoriteDirectories.begin();
    vtkKWFavoriteDirectoriesFrameInternals::FavoriteDirectoryEntryIterator 
      end = this->Internals->FavoriteDirectories.end();
    for(; it!=end; it++)
      {
      if (strcmp(((*it)->Name).c_str(), name) == 0)
        {
        return 1;
        }
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
const char* vtkKWFavoriteDirectoriesFrame::GetNameOfFavoriteDirectory(
  const char *path)
{
  if (path && *path)
    {
    vtksys_stl::string dirpath = path;
    vtksys::SystemTools::ConvertToUnixSlashes(dirpath);
  
    vtkKWFavoriteDirectoriesFrameInternals::FavoriteDirectoryEntryIterator 
      it = this->Internals->FavoriteDirectories.begin();
    vtkKWFavoriteDirectoriesFrameInternals::FavoriteDirectoryEntryIterator 
      end = this->Internals->FavoriteDirectories.end();

    vtksys_stl::string currPath;
    for(; it!=end; it++)
      {
      currPath = (*it)->Path;
      vtksys::SystemTools::ConvertToUnixSlashes(currPath);
      if (KWFileBrowser_ComparePath(currPath.c_str(), dirpath.c_str()))
        {
        return (*it)->Name.c_str();
        }
      }
    }
  return NULL;
}

//----------------------------------------------------------------------------
vtkKWPushButton* 
vtkKWFavoriteDirectoriesFrame::GetButtonOfFavoriteDirectoryWithName(
  const char* name)
{
  if (name && *name)
    {
    int nb_children = 
      this->ContainerFrame->GetFrame()->GetNumberOfChildren();
    for (int index = 0; index < nb_children; index++)
      {
      vtkKWPushButton *child = vtkKWPushButton::SafeDownCast(
        this->ContainerFrame->GetFrame()->GetNthChild(index));
      if (child && child->IsPacked() && strcmp(child->GetText(), name) == 0)
        {
        return child;
        }
      }
    }
  return NULL;
}

//----------------------------------------------------------------------------
int vtkKWFavoriteDirectoriesFrame::HasFavoriteDirectory(const char* path)
{
  return this->GetNameOfFavoriteDirectory(path) ? 1 : 0;
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::SetFavoriteDirectoryPath(
  const char* oldpath, 
  const char* newpath)
{
  vtkKWFavoriteDirectoriesFrameInternals::FavoriteDirectoryEntryIterator 
    it = this->Internals->FavoriteDirectories.begin();
  for(; it != this->Internals->FavoriteDirectories.end(); it++)
    {
    if (strcmp((*it)->Path.c_str(), oldpath) == 0)
      { 
      vtkKWPushButton *child = 
        this->GetButtonOfFavoriteDirectoryWithName((*it)->Name.c_str());
      (*it)->Path = newpath;
      this->UpdateFavoriteDirectoryButton(
        child, (*it)->Path.c_str(), (*it)->Name.c_str());
      this->WriteFavoriteDirectoriesToRegistry();
      break;
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::SetFavoriteDirectoryName(
  const char* oldname, 
  const char* newname)
{
  vtkKWFavoriteDirectoriesFrameInternals::FavoriteDirectoryEntryIterator 
    it = this->Internals->FavoriteDirectories.begin();
  for(; it != this->Internals->FavoriteDirectories.end(); it++)
    {
    if (strcmp((*it)->Name.c_str(), oldname) == 0)
      { 
      vtkKWPushButton *child = 
        this->GetButtonOfFavoriteDirectoryWithName((*it)->Name.c_str());
      (*it)->Name = newname;
      this->UpdateFavoriteDirectoryButton(
        child, (*it)->Path.c_str(), (*it)->Name.c_str());
      this->WriteFavoriteDirectoriesToRegistry();
      break;
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::SelectFavoriteDirectory(
  const char* path)
{
  if (path && *path && !this->IsFavoriteDirectorySelected(path))
    {
    this->ClearFavoriteDirectorySelection();
    vtksys_stl::string path_str(path);
    const char *name_of_fav = 
      this->GetNameOfFavoriteDirectory(path_str.c_str());
    if (name_of_fav)
      {
      vtksys_stl::string name_of_fav_str(name_of_fav);
      this->SelectFavoriteDirectoryWithName(name_of_fav_str.c_str());
      this->InvokeFavoriteDirectorySelectedCommand(
        path_str.c_str(), name_of_fav_str.c_str());
      }
    }
}

//----------------------------------------------------------------------------
int vtkKWFavoriteDirectoriesFrame::IsFavoriteDirectorySelected(
  const char* path)
{
  if (path && *path)
    {
    vtksys_stl::string path_str(path);
    const char *name_of_fav = 
      this->GetNameOfFavoriteDirectory(path_str.c_str());
    if (name_of_fav)
      {
      vtksys_stl::string name_of_fav_str(name_of_fav);
      const char *sel_path = this->GetSelectedFavoriteDirectory();
      if (sel_path && *sel_path)
        {
        vtksys_stl::string sel_path_str(sel_path);
        const char *sel_name_of_fav = 
          this->GetNameOfFavoriteDirectory(sel_path_str.c_str());
        if (sel_name_of_fav)
          {
          vtksys_stl::string sel_name_of_fav_str(sel_name_of_fav);
          if (!strcmp(sel_name_of_fav_str.c_str(), name_of_fav_str.c_str()))
            {
            return 1;
            }
          }
        }
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
const char* vtkKWFavoriteDirectoriesFrame::GetSelectedFavoriteDirectory()
{
  int nb_children = 
    this->ContainerFrame->GetFrame()->GetNumberOfChildren();
  for (int index = 0; index < nb_children; index++)
    {
    vtkKWPushButton *child = vtkKWPushButton::SafeDownCast(
      this->ContainerFrame->GetFrame()->GetNthChild(index));
    if (child && child->IsPacked() && 
      child->GetRelief() == vtkKWOptions::ReliefRidge)
      {
      return this->GetSelectedFavoriteDirectoryWithName(child->GetText());
      }
    }
  return NULL;
}

//----------------------------------------------------------------------------
const char* vtkKWFavoriteDirectoriesFrame::
  GetSelectedFavoriteDirectoryWithName(const char* name)
{
  if (name && *name)
    {
    vtkKWFavoriteDirectoriesFrameInternals::FavoriteDirectoryEntryIterator 
      it = this->Internals->FavoriteDirectories.begin();
    for(; it != this->Internals->FavoriteDirectories.end(); it++)
      {
      if (strcmp((*it)->Name.c_str(), name) == 0)
        { 
        return KWFileBrowser_GetUnixPath((*it)->Path.c_str());
        }
      }
    }
  return NULL;
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::SelectFavoriteDirectoryWithName(
  const char* name)
{
  this->ClearFavoriteDirectorySelection();
  vtkKWPushButton *child = this->GetButtonOfFavoriteDirectoryWithName(name);
  if (child)
    {
    child->SetReliefToRidge();
    child->SetBackgroundColor(
      this->Internals->SelectedFavoriteDirectoryBackgroundColor);
    }
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::ClearFavoriteDirectorySelection()
{
  if (!this->ContainerFrame->IsCreated())
    {
    return;
    }

  int nb_children = 
    this->ContainerFrame->GetFrame()->GetNumberOfChildren();
  for(int index = 0; index < nb_children; index++)
    {
    vtkKWPushButton *child = vtkKWPushButton::SafeDownCast(
      this->ContainerFrame->GetFrame()->GetNthChild(index));
    if (child)
      {
      child->SetReliefToFlat();
      child->SetBackgroundColor(
        this->ContainerFrame->GetFrame()->GetBackgroundColor());
      }
    }    
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::AddFavoriteDirectory(
  const char *path, 
  const char *name)
{
  if (!path || !*path || !name || !*name)
    {
    return;
    }

  vtksys_stl::string dirpath = path;
  
  vtkDirectory *dir = vtkDirectory::New();
  if (!dir->Open(dirpath.c_str()))
    {
    dir->Delete();
    return;
    }
  dir->Delete();

  this->AddFavoriteDirectoryToFrame(dirpath.c_str(), name);
  
  if (this->Internals->FavoriteDirectories.size() >
      (size_t)this->MaximumNumberOfFavoriteDirectoriesInRegistry)
    {
    this->PruneFavoriteDirectoriesInRegistry();
    }
    
  this->WriteFavoriteDirectoriesToRegistry();
    
  this->Update();
} 

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::AddFavoriteDirectoryToFrame(
  const char *path, const char* name)
{
  if (this->HasFavoriteDirectory(path))
    {
    return;
    }

  // Add selected folder to favorite directory list

  vtkKWFavoriteDirectoriesFrameInternals::FavoriteDirectoryEntry *direntry =
    new vtkKWFavoriteDirectoriesFrameInternals::FavoriteDirectoryEntry;
  direntry->Path = path;
  direntry->Name = name;
  
  this->Internals->FavoriteDirectories.push_front(direntry);

  vtkKWPushButton *dirbutton = vtkKWPushButton::New(); 
  dirbutton->SetParent(this->ContainerFrame->GetFrame());
  dirbutton->Create();
  dirbutton->SetReliefToFlat();
  dirbutton->SetOverReliefToRaised();
  dirbutton->SetCompoundModeToTop();
  dirbutton->SetImageToPredefinedIcon(vtkKWIcon::IconFolder32);
  dirbutton->SetBackgroundColor(
    this->ContainerFrame->GetBackgroundColor());
  dirbutton->SetActiveBackgroundColor(
    this->ContainerFrame->GetBackgroundColor());
  dirbutton->SetConfigurationOptionAsInt("-takefocus", 0);

  this->UpdateFavoriteDirectoryButton(dirbutton, path, name);

  int nb_children = 
    this->ContainerFrame->GetFrame()->GetNumberOfPackedChildren();
                    
  if (nb_children > 0)
    {
    this->Script("pack %s -side top -fill x -pady 2 -before %s", 
                 dirbutton->GetWidgetName(), 
                 this->ContainerFrame->GetFrame()->GetNthChild(nb_children-1)->GetWidgetName());
    }
  else
    {
    this->Script("pack %s -side top -fill x -pady 2", 
                 dirbutton->GetWidgetName());
    }

  dirbutton->Delete();
                    
  this->Update();
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::UpdateFavoriteDirectoryButton(
  vtkKWPushButton *button, const char *path, const char* name)
{
  if (!name || !path)
    {
    return;
    }

  button->SetText(name);
  button->SetBalloonHelpString(path);

  char cmd[256];
  sprintf(cmd, "SelectFavoriteDirectoryCallback \"%s\" \"%s\"",
          vtksys::SystemTools::EscapeChars(
            path, 
            KWFileBrowser_ESCAPE_CHARS).c_str(), 
          name);
  button->SetCommand(this, cmd);

  char rightclickcmd[256];
  sprintf(rightclickcmd, "PopupFavoriteDirectoryCallback \"%s\" %%X %%Y",
          vtksys::SystemTools::EscapeChars(
            path, 
            KWFileBrowser_ESCAPE_CHARS).c_str());

  button->AddBinding("<Button-3>", this, rightclickcmd);
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::PopulateContextMenu(
  vtkKWMenu *menu, const char *path)
{
  char command[256];
  vtksys_stl::string buttonpath = path;

#ifdef _WIN32
  // Explore to open native explorer file

  sprintf(command, "ExploreFavoriteDirectoryCallback \"%s\"", 
          vtksys::SystemTools::EscapeChars(
            buttonpath.c_str(), 
            KWFileBrowser_ESCAPE_CHARS).c_str());
  menu->AddCommand("Explore", this, command);
#endif

  // Rename file

  sprintf(command, "RenameFavoriteDirectoryCallback \"%s\"", 
          vtksys::SystemTools::EscapeChars(
            buttonpath.c_str(), 
            KWFileBrowser_ESCAPE_CHARS).c_str());
  menu->AddCommand("Rename", this, command);

  // Delete file

  sprintf(command, "RemoveFavoriteDirectoryCallback \"%s\"", 
          vtksys::SystemTools::EscapeChars(
            buttonpath.c_str(), 
            KWFileBrowser_ESCAPE_CHARS).c_str());
  menu->AddCommand("Delete", this, command);
}

//----------------------------------------------------------------------------
void  vtkKWFavoriteDirectoriesFrame::AddFavoriteDirectoryCallback()
{
  this->InvokeAddFavoriteDirectoryCommand();
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::SelectFavoriteDirectoryCallback(
  const char* path, const char* name)
{
  this->SelectFavoriteDirectory(path);
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::SetAddFavoriteDirectoryCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(
    &this->AddFavoriteDirectoryCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::InvokeAddFavoriteDirectoryCommand()
{
  if (this->AddFavoriteDirectoryCommand && *this->AddFavoriteDirectoryCommand)
    {
    this->Script("%s", this->AddFavoriteDirectoryCommand);
    }
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::SetFavoriteDirectorySelectedCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(
    &this->FavoriteDirectorySelectedCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::InvokeFavoriteDirectorySelectedCommand(
  const char* path, const char* name)
{
  if (this->FavoriteDirectorySelectedCommand 
    && *this->FavoriteDirectorySelectedCommand)
    {
    this->Script("%s \"%s\" \"%s\"", 
      this->FavoriteDirectorySelectedCommand, 
      vtksys::SystemTools::EscapeChars(KWFileBrowser_GetUnixPath(path), 
       KWFileBrowser_ESCAPE_CHARS).c_str(), 
       vtksys::SystemTools::EscapeChars(name, 
       KWFileBrowser_ESCAPE_CHARS).c_str());
    }
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::PopupFavoriteDirectoryCallback(
  const char* path, int x, int y)
{
  if (!path || !(*path))
    {
    return;
    }

  if (!this->ContextMenu)
    {
    this->ContextMenu = vtkKWMenu::New();
    }
  if (!this->ContextMenu->IsCreated())
    {
    this->ContextMenu->SetParent(this);
    this->ContextMenu->Create();
    }

  this->ContextMenu->DeleteAllItems();
  this->PopulateContextMenu(this->ContextMenu, path);

  if (this->ContextMenu->GetNumberOfItems())
    {
    this->ContextMenu->PopUp(x, y);
    }
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::RemoveFavoriteDirectoryCallback(
  const char* path)
{
  if (!path || !(*path))
    {
    return;
    }

  vtksys_stl::string path_str = path;
  vtksys_stl::string message(
    "Are you sure you want to delete this favorite directory? \n");
  message.append(path_str.c_str());

  // Prompt the user for confirmation

  if (vtkKWMessageDialog::PopupYesNo( 
        this->GetApplication(), 
        this, 
        ks_("Favorite Directories|Title|Delete favorites"),
        k_(message.c_str()),
        vtkKWMessageDialog::WarningIcon | 
        vtkKWMessageDialog::InvokeAtPointer))
    {
    this->RemoveFavoriteDirectory(path_str.c_str());
    this->Update();
    }
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::ExploreFavoriteDirectoryCallback(
  const char* path)
{
#ifdef _WIN32
  if (path && *path && vtksys::SystemTools::FileIsDirectory(path))
    {
    this->GetApplication()->OpenLink(path);
    }
#endif
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::RenameFavoriteDirectoryCallback(
  const char* path)
{
  if (!path || !(*path))
    {
    return;
    }

  const char *name = this->GetNameOfFavoriteDirectory(path);
  if (!name)
    {
    vtkKWMessageDialog::PopupMessage(
      this->GetApplication(), this, 
      ks_("Favorite Directories|Title|Error!"),
      "No valid directory found!", 
      vtkKWMessageDialog::ErrorIcon | 
      vtkKWMessageDialog::InvokeAtPointer);
    return;
    }

  // Prompt the user for the new name of the favorite  

  vtkKWSimpleEntryDialog *dlg = vtkKWSimpleEntryDialog::New();
  dlg->SetParent(this);
  dlg->SetMasterWindow(this->GetParentTopLevel());
  dlg->SetDisplayPositionToPointer();
  dlg->SetTitle(
    ks_("Favorite Directories|Dialog|Title|Rename favorite"));
  dlg->SetStyleToOkCancel();
  dlg->Create();
  dlg->GetEntry()->GetLabel()->SetText(
    ks_("Favorite Directories|Dialog|Favorite name:"));
  dlg->GetEntry()->GetWidget()->SetValue(name);
  dlg->SetText(
    ks_("Favorite Directories|Dialog|Enter a new favorite name:"));
  
  int ok = dlg->Invoke();
  vtksys_stl::string newname = dlg->GetEntry()->GetWidget()->GetValue();
  dlg->Delete();
  if (ok)
    {
    if (newname.empty() || 
        strcmp(newname.c_str(), "") == 0 ||
        strcmp(newname.c_str(), ".") == 0 ||
        strcmp(newname.c_str(), "..") == 0)
      {
      vtkKWMessageDialog::PopupMessage(
        this->GetApplication(), this, 
        ks_("Favorite Directories|Title|Error!"),
        "Please enter a valid favorite name!", 
        vtkKWMessageDialog::ErrorIcon | 
        vtkKWMessageDialog::InvokeAtPointer);
      return;
      }
    }
  else
    {
    return;
    }    

  // Do we have that name already?

  if (this->HasFavoriteDirectoryWithName(newname.c_str()))
    {
    vtkKWMessageDialog::PopupMessage(
      this->GetApplication(), this, 
      ks_("Favorite Directories|Title|Error!"),
      "This favorite name is already used!", 
      vtkKWMessageDialog::ErrorIcon | 
      vtkKWMessageDialog::InvokeAtPointer);
    return;
    }

  // Rename

  this->SetFavoriteDirectoryName(name, newname.c_str());
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::RemoveFavoriteDirectory(
  const char *path)
{
  vtkKWFavoriteDirectoriesFrameInternals::FavoriteDirectoryEntryIterator 
    it = this->Internals->FavoriteDirectories.begin();
  for(; it != this->Internals->FavoriteDirectories.end(); it++)
    {
    if (strcmp((*it)->Path.c_str(), path) == 0)
      { 
      vtkKWPushButton *child = 
        this->GetButtonOfFavoriteDirectoryWithName((*it)->Name.c_str());
      if (child)
        {
        child->Unpack();
        child->SetParent(NULL);
        }
      delete *it;
      this->Internals->FavoriteDirectories.erase(it);
      this->WriteFavoriteDirectoriesToRegistry();
      break;
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::WriteFavoriteDirectoriesToRegistry()
{
  if(this->GetApplication() && 
    this->GetApplication()->GetRegistryLevel() <=0)
    {
    return;
    }

  this->WriteFavoriteDirectoriesToRegistry(
    this->RegistryKey, 
    this->MaximumNumberOfFavoriteDirectoriesInRegistry);
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::WriteFavoriteDirectoriesToRegistry(
  const char *reg_key, int max_nb)
{
  if (!this->GetApplication())
    {
    vtkErrorMacro("Error! Application not set!");
    return;
    }

  if (!reg_key)
    {
    vtkErrorMacro("Error! Can not write to empty key in registry!");
    return;
    }

  char dirname_key[20], label_key[20];
  vtkKWRegistryHelper* registryhelper = vtkKWRegistryHelper::New();
  if (!registryhelper->Open(VTK_KW_FAVORITE_TOPLEVEL, reg_key, 1)) 
    {
    vtkErrorMacro(
      "Error! Failed to open the registry key for writing!");
    registryhelper->Delete();
    return;
    }
  
  // Store all favorite dir entries to registry

  vtkKWFavoriteDirectoriesFrameInternals::FavoriteDirectoryEntryIterator 
    it = this->Internals->FavoriteDirectories.begin();
  vtkKWFavoriteDirectoriesFrameInternals::FavoriteDirectoryEntryIterator 
    end = this->Internals->FavoriteDirectories.end();
  int count = 0;
  for (; it != end && count < max_nb; ++it)
    {
    if (*it)
      {
      sprintf(dirname_key,
              VTK_KW_DIR_REGISTRY_PATHNAME_KEYNAME_PATTERN, count);
      sprintf(label_key,
              VTK_KW_DIR_REGISTRY_LABEL_KEYNAME_PATTERN, count);
      
      registryhelper->SetValue(reg_key, dirname_key, (*it)->Path.c_str());
      if ((*it)->Name.size())
        {
        registryhelper->SetValue(reg_key, label_key, (*it)->Name.c_str());
        }
      ++count;
      }
    }
  
  // As a convenience, remove all others

  for (; count < VTK_KW_DIR_REGISTRY_MAX_ENTRIES; count++)
    {
    sprintf(dirname_key,
            VTK_KW_DIR_REGISTRY_PATHNAME_KEYNAME_PATTERN, count);
    sprintf(label_key,
            VTK_KW_DIR_REGISTRY_LABEL_KEYNAME_PATTERN, count);
    registryhelper->DeleteValue(reg_key, dirname_key);
    registryhelper->DeleteValue(reg_key, label_key);
    }
  
  registryhelper->Close();
  registryhelper->Delete();
  
#ifdef _WIN32
  this->WriteFavoriteDirectoriesToSystemRegistry();
#endif
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::RestoreFavoriteDirectoriesFromRegistry()
{
  if(this->GetApplication() && 
    this->GetApplication()->GetRegistryLevel() <=0)
    {
    return;
    }

  // Remove all

  if (this->ContainerFrame && this->ContainerFrame->GetFrame())
    {
    this->ContainerFrame->GetFrame()->UnpackChildren();
    this->ContainerFrame->GetFrame()->RemoveAllChildren();
    }
  this->ClearInternalList();

  // We will always have the directory, set by "HOME" 
  // evironment variable, as favorite
    
   vtksys_stl::string dir; 
   if(vtksys::SystemTools::GetEnv("HOME", dir)) 
     { 
     if(vtksys::SystemTools::FileIsDirectory(dir.c_str())) 
       { 
       this->AddFavoriteDirectoryToFrame(dir.c_str(), 
         vtksys::SystemTools::GetFilenameName(dir).c_str()); 
       } 
     } 

  // Restore the set of registry keys created by users

  this->RestoreFavoriteDirectoriesFromUserRegistry(
    this->RegistryKey, 
    this->MaximumNumberOfFavoriteDirectoriesInRegistry);
    
#ifdef _WIN32
    // Load system favorites

    this->RestoreFavoriteDirectoriesFromSystemRegistry();
#endif
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::RestoreFavoriteDirectoriesFromUserRegistry(
  const char *reg_key, int max_nb)
{
  if (!this->GetApplication())
    {
    vtkErrorMacro("Error! Application not set!");
    return;
    }

  if (!reg_key)
    {
    vtkErrorMacro("Error! Can not load from empty key in registry!");
    return;
    }
    
  vtkKWRegistryHelper* registryhelper = vtkKWRegistryHelper::New();
  if (!registryhelper->Open(VTK_KW_FAVORITE_TOPLEVEL, reg_key, 0)) 
    {
    registryhelper->Delete();
    return;
    }  

  char dirname_key[20], label_key[20];
  char dirname[1024], label[1024];
  int i;

  for (i = VTK_KW_DIR_REGISTRY_MAX_ENTRIES - 1; 
       i >= 0 && max_nb; 
       i--)
    {
    sprintf(dirname_key, 
            VTK_KW_DIR_REGISTRY_PATHNAME_KEYNAME_PATTERN, i);
    sprintf(label_key, 
            VTK_KW_DIR_REGISTRY_LABEL_KEYNAME_PATTERN, i);
    if (registryhelper->ReadValue(reg_key, dirname_key, dirname) &&
        strlen(dirname) >= 1)
      {
      if (vtksys::SystemTools::FileIsDirectory(dirname))
        {          
        if (!registryhelper->ReadValue(reg_key, label_key, label))
          {
          *label = '\0';
          }
        this->AddFavoriteDirectoryToFrame(dirname, label);
        max_nb--;
        }
      }
    }
  registryhelper->Close();
  registryhelper->Delete();  
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::RestoreFavoriteDirectoriesFromSystemRegistry()
{
#ifdef _WIN32  

  if(!this->UseSystemDefaultPlaces)
    {
    return;
    }

  char buff[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  vtksys_stl::string placekey = VTK_KW_WIN32_REGIRSTRY_PLACES_BAR_KEY;
  bool userdefined = false;
  char place[10];
  vtksys_stl::string value, placename;

  // If this system key exists, read from it and 
  // add them to the favorite dir entries.

  vtkKWWin32RegistryHelper* registryhelper = 
    vtkKWWin32RegistryHelper::New();

  if (registryhelper && 
      registryhelper->OpenInternal(placekey.c_str(), 0)) 
    {
    // Windows only allow five entries under this key

    for(int i = VTK_KW_WIN32_REGIRSTRY_NUM_PLACES - 1; i >= 0; i--)
      {
      sprintf(place, "Place%d", i);
      placename = place;
      
      if (registryhelper->ReadValueInternal(placename.c_str(), buff))
        {
        value = buff;
        if (vtksys::SystemTools::FileIsDirectory(value.c_str()))
          {          
          this->AddFavoriteDirectoryToFrame(
            value.c_str(),
            vtksys::SystemTools::GetFilenameName(value).c_str());            
          userdefined = true;
          }
        else
          {
          int csidl = -1;
          sscanf(value.c_str(), "%d", &csidl);
          if (csidl >= 0 && csidl <= 0x003d)
            {
            if (this->AddSpecialFavoriteFolder(csidl)) 
              {
              userdefined=true;
              }
            } //end if csidl
          }//end else
        } //end if readvalue
      }//end for
    registryhelper->Close();
    }//end if open   
  registryhelper->Delete();
  
#endif
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::WriteFavoriteDirectoriesToSystemRegistry()
{
#ifdef _WIN32

  if(!this->UseSystemDefaultPlaces)
    {
    return;
    }

  int num_dir = (int)this->Internals->FavoriteDirectories.size();
  if (num_dir <= 0)
    {
    return;
    }
    
  char buff[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  vtksys_stl::string placekey = VTK_KW_WIN32_REGIRSTRY_PLACES_BAR_KEY;
  bool userdefined = false;
  char place[10];
  vtksys_stl::string value, placename;

  // If this system key exists, read from it and 
  // add them to the favorite dir entries.

  vtkKWWin32RegistryHelper* registryhelper = 
    vtkKWWin32RegistryHelper::New();

  if (registryhelper && 
      registryhelper->OpenInternal(placekey.c_str(), 1)) 
    {
    vtkKWFavoriteDirectoriesFrameInternals::FavoriteDirectoryEntryIterator 
      it = this->Internals->FavoriteDirectories.begin();
    vtkKWFavoriteDirectoriesFrameInternals::FavoriteDirectoryEntryIterator 
      end = this->Internals->FavoriteDirectories.end();
    
    int found=0;

    for(int i=0; it != end && i < VTK_KW_WIN32_REGIRSTRY_NUM_PLACES; i++, it++)
      {
      found = 0;
      for(int j = 0; j < VTK_KW_WIN32_REGIRSTRY_NUM_PLACES; j++)
        {
        sprintf(place, "Place%d", j);
        placename = place;
        
        if (registryhelper->ReadValueInternal(
              placename.c_str(), buff))
          {
          value = buff;
          
          if (vtksys::SystemTools::FileIsDirectory(value.c_str()))
            {          
            if (it != end && KWFileBrowser_ComparePath(
                  value.c_str(), (*it)->Path.c_str()))  
              {
              found=true;
              break;
              }
            }
          else
            {
            int csidl = -1;
            sscanf(value.c_str(), "%d", &csidl);
            if (csidl >=0 && csidl <=0x003d)
              {
              // Get the path to these folders
              LPITEMIDLIST pidl;
              HRESULT hr = SHGetSpecialFolderLocation(
                NULL, csidl, &pidl);
              if (SUCCEEDED(hr))
                {
                // Convert the item ID list's binary 
                // representation into a file system path
                char szPath[_MAX_PATH];
                if (SHGetPathFromIDList(pidl, szPath))
                  {
                  if (it != end && 
                      KWFileBrowser_ComparePath(
                        szPath, (*it)->Path.c_str()))  
                    {          
                    found=true;
                    break;
                    }
                  }
                }//end if succeed
              } //end if csidl
            }//end else
          } //end if readvalue
        }//end for j
        
      sprintf(place, "Place%d", i);
      placename=place;
      if (found)
        {
        if (vtksys::SystemTools::FileIsDirectory(value.c_str()))
          { 
          // Only Windows path should be used here; otherwise, 
          // other applications, such as Notepad, won't recognize the path
          registryhelper->SetValueInternal(placename.c_str(), 
            vtksys::SystemTools::ConvertToOutputPath(
            value.c_str()).c_str());
          }
        else
          {
          int csidl = -1;
          sscanf(value.c_str(), "%d", &csidl);
          if (csidl >=0 && csidl <=0x003d)
            {
            registryhelper->SetValueInternal(placename.c_str(), &csidl);
            }
          }
        }
      else
        {
        // Only Windows path should be used here; otherwise, 
        // other applications, such as Notepad, won't recognize the path
        registryhelper->SetValueInternal(placename.c_str(), 
          vtksys::SystemTools::ConvertToOutputPath(
          (*it)->Path.c_str()).c_str());
        }
      }//end for it, i
    registryhelper->Close();
    }//end if open   

  registryhelper->Delete();
  
#endif
}

//----------------------------------------------------------------------------
int vtkKWFavoriteDirectoriesFrame::AddSpecialFavoriteFolder(int csidl)
{
#ifdef _WIN32
  vtksys_stl::string name;
  switch (csidl)
    {
    case CSIDL_DESKTOP:
      name = "Desktop";
      break;
    case CSIDL_PERSONAL:
      name = "My Documents";
      break;
    case CSIDL_DRIVES:
      name = "My Computer";
      break;
    case CSIDL_FAVORITES:
      name = "My Favorites";
      break;
    default:
      name = "My Folder";
      break;
    } // end switch

  // Get the path to these folders
  LPITEMIDLIST pidl;
  HRESULT hr = SHGetSpecialFolderLocation(NULL, csidl, &pidl);
  if (SUCCEEDED(hr))
    {
    // Convert the item ID list's binary 
    // representation into a file system path
    char szPath[_MAX_PATH];
    if (SHGetPathFromIDList(pidl, szPath))
      {
      this->AddFavoriteDirectoryToFrame(szPath, name.c_str());
      return 1;
      }
    }//end if succeed
#endif  

  return 0;
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->PropagateEnableState(this->ContainerFrame);
  this->PropagateEnableState(this->Toolbar);
  this->PropagateEnableState(this->AddFavoriteDirectoryButton);

  if (this->ContainerFrame->GetFrame())
    {
    int nb_children = 
      this->ContainerFrame->GetFrame()->GetNumberOfChildren();
    for(int index = 0; index < nb_children; index++)
      {
      this->ContainerFrame->GetFrame()->GetNthChild(index)->SetEnabled(
        this->GetEnabled());
      }
    }    
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::PruneFavoriteDirectoriesInRegistry()
{
  while (this->Internals->FavoriteDirectories.size() > 
         (size_t)this->MaximumNumberOfFavoriteDirectoriesInRegistry)
    {
    this->RemoveFavoriteDirectory(
      this->Internals->FavoriteDirectories.back()->Path.c_str());
    this->Internals->FavoriteDirectories.pop_back();
    }
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::SetMaximumNumberOfFavoriteDirectoriesInRegistry(
  int maxnum)
{
  if (maxnum < VTK_KW_DIR_REGISTRY_MIN_ENTRIES)
    {
    maxnum = VTK_KW_DIR_REGISTRY_MIN_ENTRIES;
    }
  else if (maxnum > VTK_KW_DIR_REGISTRY_MAX_ENTRIES)
    {
    maxnum = VTK_KW_DIR_REGISTRY_MAX_ENTRIES;
    }
  if (this->MaximumNumberOfFavoriteDirectoriesInRegistry == maxnum)
  {
  return;
  }
  
  this->MaximumNumberOfFavoriteDirectoriesInRegistry = maxnum;
  this->PruneFavoriteDirectoriesInRegistry();
  this->WriteFavoriteDirectoriesToRegistry();

  this->Update();
  
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::GetContainerFrameBackgroundColor(
  double *r, double *g, double *b)
{
  this->ContainerFrame->GetBackgroundColor(r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWFavoriteDirectoriesFrame::GetContainerFrameBackgroundColor()
{
  return this->ContainerFrame->GetBackgroundColor();
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::SetContainerFrameBackgroundColor(
  double r, double g, double b)
{
  this->ContainerFrame->SetBackgroundColor(r, g, b);
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::PrintSelf(
  ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  
  os << indent << "AddFavoriteDirectoryCommand: " 
     << (this->AddFavoriteDirectoryCommand?this->AddFavoriteDirectoryCommand:"none")
     << endl;
  os << indent << "FavoriteDirectorySelectedCommand: " 
     << (this->FavoriteDirectorySelectedCommand?this->FavoriteDirectorySelectedCommand:"none")
     << endl;
  os << indent << "RegistryKey: " 
     << (this->RegistryKey?this->RegistryKey:"none")
     << endl;
  os << indent << "MaximumNumberOfFavoriteDirectoriesInRegistry: " 
     << this->MaximumNumberOfFavoriteDirectoriesInRegistry
     << endl;

  vtkKWFavoriteDirectoriesFrameInternals::FavoriteDirectoryEntryIterator 
    it = this->Internals->FavoriteDirectories.begin();
  vtkKWFavoriteDirectoriesFrameInternals::FavoriteDirectoryEntryIterator
    end = this->Internals->FavoriteDirectories.end();
  for (; it != end; ++it)
    {
    os << indent << "FavoriteDirectory: " 
       << ((*it)->Path.c_str()?(*it)->Path.c_str():"none")
       << endl;
    }
}
