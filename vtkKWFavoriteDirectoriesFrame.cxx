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
#define VTK_KW_DIR_REGISTRY_COMMAND_KEYNAME_PATTERN "Path%02dCmd"
#define VTK_KW_DIR_REGISTRY_LABEL_KEYNAME_PATTERN "Path%02dLabel"
#define VTK_KW_DIR_REGISTRY_NODE_KEYNAME_PATTERN "Path%02dNode"

#define VTK_KW_FAVORITE_TOPLEVEL "KWFileBrowserFavorites"
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
vtkCxxRevisionMacro(vtkKWFavoriteDirectoriesFrame, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
class vtkKWFavoriteDirectoriesFrameInternals
{
public:
  
  vtkKWFavoriteDirectoriesFrameInternals()
  {
    //some constants
    FavSelectedBackgroundRGB[0]=0.98;
    FavSelectedBackgroundRGB[1]=0.98;
    FavSelectedBackgroundRGB[2]=0.98;   
  }
  
  class DirectoryEntry
  {
  public:
    DirectoryEntry() {};
    ~DirectoryEntry() {};
    
    vtksys_stl::string    FullPath;
    vtksys_stl::string    DisplayText;

    // Compare if the two instance is the same. Here only the full
    // path is relevant.
    int IsEqual(const char *fullpath, const char * displaytext) 
      { return (fullpath && !strcmp(fullpath, this->FullPath.c_str())); }
  };

  // Favorite directories list
  typedef vtksys_stl::list<DirectoryEntry*> DirectoryEntriesContainer;
  typedef vtksys_stl::list<DirectoryEntry*>::iterator 
    DirEntryContainerIterator;

  DirectoryEntriesContainer FavoriteDirEntries;
  
  // Constants
  double FavSelectedBackgroundRGB[3];
};

//----------------------------------------------------------------------------
vtkKWFavoriteDirectoriesFrame::vtkKWFavoriteDirectoriesFrame()
{
  this->MaximumNumberOfFavoriteDirectoriesInRegistry = 15;
  
  this->Internals = new vtkKWFavoriteDirectoriesFrameInternals;
  this->ToolbarFavorite = vtkKWToolbar::New();
  
  this->FavoriteButtonFrame = vtkKWFrameWithScrollbar::New();
  this->FavoritesAddingButton = vtkKWPushButton::New();
  this->ContextMenu = NULL;
  
  this->FavoriteDirectoryAddingCommand = NULL;
  this->FavoriteDirectorySelectedCommand = NULL;
  this->RegistryKey = NULL;
}

//----------------------------------------------------------------------------
vtkKWFavoriteDirectoriesFrame::~vtkKWFavoriteDirectoriesFrame()
{
  this->WriteDirectoriesToRegistry();

  if (this->RegistryKey)
    {
    delete [] this->RegistryKey;
    this->RegistryKey = NULL;
    }
  if (this->FavoriteDirectoryAddingCommand)
    {
    delete [] this->FavoriteDirectoryAddingCommand;
    this->FavoriteDirectoryAddingCommand = NULL;
    }
  if (this->FavoriteDirectorySelectedCommand)
    {
    delete [] this->FavoriteDirectorySelectedCommand;
    this->FavoriteDirectorySelectedCommand = NULL;
    }
      
  this->FavoritesAddingButton->Delete();
  if (this->ContextMenu)
    {
    this->ContextMenu->Delete();
    this->ContextMenu = NULL;
    }
  this->ToolbarFavorite->Delete();
   
  this->FavoriteButtonFrame->GetFrame()->RemoveAllChildren();
  this->FavoriteButtonFrame->Delete();
  
  // Clear internals list
  this->ClearInternalList();
  if (this->Internals)
    {
    delete this->Internals;
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

  this->CreateFavoriteDirectoriesFrame();  
  
  this->Initialize();
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::CreateFavoriteDirectoriesFrame()
{
  // Toolbar for Favorite Frame
  this->ToolbarFavorite->SetParent(this);
  this->ToolbarFavorite->Create();
  this->ToolbarFavorite->SetWidth(20);
  // Favorite Add button for ToolbarFavorite
  this->FavoritesAddingButton->SetParent(
    this->ToolbarFavorite->GetFrame());
  this->FavoritesAddingButton->Create();
  this->FavoritesAddingButton->SetImageToPredefinedIcon(
    vtkKWIcon::IconFavorites);
  this->FavoritesAddingButton->SetBalloonHelpString(
    "Add to favorites");
  this->FavoritesAddingButton->SetCommand(this, 
    "AddFavoriteDirectoryCallback");
  this->FavoritesAddingButton->SetConfigurationOptionAsInt("-takefocus", 0);
  this->ToolbarFavorite->AddWidget(
    this->FavoritesAddingButton);

  this->ToolbarFavorite->SetToolbarAspectToFlat();
  this->ToolbarFavorite->SetWidgetsAspectToFlat();
  this->Script("pack %s -side top -anchor nw -fill x",
    this->ToolbarFavorite->GetWidgetName());
  
  // Favorite button frame to hold Favorite directory buttons.
  this->FavoriteButtonFrame->SetParent(this);
  this->FavoriteButtonFrame->SetHorizontalScrollbarVisibility(0);
  this->FavoriteButtonFrame->Create();
  this->FavoriteButtonFrame->SetBackgroundColor(0.5, 0.5, 0.5);
  this->FavoriteButtonFrame->SetBorderWidth(1);
  this->FavoriteButtonFrame->SetReliefToSunken();
  this->Script("pack %s -side top -fill both -expand true",
       this->FavoriteButtonFrame->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::Initialize()
{
  if (!this->GetRegistryKey())
    { 
    this->SetRegistryKey(VTK_KW_FAVORITE_DIR_KEY);
    }

  //Retrieve the favorite directories from registry
  this->RestoreDirectoriesListFromRegistry();
  
  this->Update();
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::ClearInternalList()
{
  if (this->Internals)
    {
    vtkKWFavoriteDirectoriesFrameInternals::DirEntryContainerIterator
      it = this->Internals->FavoriteDirEntries.begin();
    vtkKWFavoriteDirectoriesFrameInternals::DirEntryContainerIterator
      end = this->Internals->FavoriteDirEntries.end();
    for (; it != end; ++it)
      {
      if (*it)
        {
        delete *it;
        }
      }
    }
}

//----------------------------------------------------------------------------
void  vtkKWFavoriteDirectoriesFrame::AddFavoriteDirectoryCallback()
{
  this->InvokeFavoriteDirectoryAddingCommand();
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::SetFavoriteDirectoryAddingCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(
    &this->FavoriteDirectoryAddingCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::SetFavoriteDirectorySelectedCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(
    &this->FavoriteDirectorySelectedCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::InvokeFavoriteDirectoryAddingCommand()
{
  if (this->FavoriteDirectoryAddingCommand 
    && *this->FavoriteDirectoryAddingCommand)
    {
    this->Script("%s", this->FavoriteDirectoryAddingCommand);
    }
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::SelectFavoriteDirectory(
  const char* favdir)
{
  if (favdir)
    {
    vtksys_stl::string fullpath = favdir;
    vtksys_stl::string dirtext = "";
    
    if (this->Internals->FavoriteDirEntries.size()>0)
      {
      vtkKWFavoriteDirectoriesFrameInternals::DirEntryContainerIterator
        it = this->Internals->FavoriteDirEntries.begin();
      for(; it != this->Internals->FavoriteDirEntries.end(); it++)
        {
        if (vtksys::SystemTools::ComparePath((*it)->FullPath.c_str(), 
                                             fullpath.c_str()))
          { 
          dirtext = (*it)->DisplayText;
          break;
          }
        }
      }
    
    if (strcmp(dirtext.c_str(), "") != 0)
      {
      this->UpdateFavoriteDirectorySelection(dirtext.c_str());
      return;
      }
    }
  
  this->ClearFavoriteDirectorySelection();
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::UpdateFavoriteDirectorySelection(
  const char* text)
{
  if (text != NULL && strcmp(text, "") !=0)
    {
    int nb_children = this->FavoriteButtonFrame->
                      GetFrame()->GetNumberOfChildren();
    bool found=false;
    // Look for which favorite button is clicked and 
    // set it background color as selected
    for(int index=0; index<nb_children; index++)
      {
      vtkKWPushButton *child = vtkKWPushButton::SafeDownCast(
          this->FavoriteButtonFrame->GetFrame()->GetNthChild(index));
      if (child == NULL || child->GetText() == NULL) continue;
      if (!found && child && child->IsPacked() &&
          strcmp(child->GetText(), text)==0) 
        {
        found = true;
        child->SetReliefToRidge();
        child->SetBackgroundColor(
          this->Internals->FavSelectedBackgroundRGB);
        }
        else
        {
        child->SetBackgroundColor(this->FavoriteButtonFrame->
          GetBackgroundColor());
        child->SetReliefToFlat();
        }
      }
    }
  else
    {
    this->ClearFavoriteDirectorySelection();
    }
}

//----------------------------------------------------------------------------
int vtkKWFavoriteDirectoriesFrame::HasFavoriteDirectoryWithName(const char* text)
{
  // Check if the input favorite dir name is already used 
  vtkKWFavoriteDirectoriesFrameInternals::DirEntryContainerIterator 
    it = this->Internals->FavoriteDirEntries.begin();
  vtkKWFavoriteDirectoriesFrameInternals::DirEntryContainerIterator 
    end = this->Internals->FavoriteDirEntries.end();
  if (this->Internals->FavoriteDirEntries.size() > 0)
    {
    for(; it!=end; it++)
      {
      if (strcmp(((*it)->DisplayText).c_str(), text) ==0)
        {
        return 1;
        }
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
int vtkKWFavoriteDirectoriesFrame::HasFavoriteDirectory(const char* path)
{
  // Check if the favorite folder is already there, 
  vtkKWFavoriteDirectoriesFrameInternals::DirEntryContainerIterator 
    it = this->Internals->FavoriteDirEntries.begin();
  vtkKWFavoriteDirectoriesFrameInternals::DirEntryContainerIterator 
    end = this->Internals->FavoriteDirEntries.end();
  if (this->Internals->FavoriteDirEntries.size() > 0)
    {
    for(; it!=end; it++)
      {
      if (vtksys::SystemTools::ComparePath(
        ((*it)->FullPath).c_str(), path))
        {
        return 1;
        }
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::AddFavoriteDirectory(
  const char *favoritedir, 
  const char *textname)
{
  // There must be a selected node
  if (!favoritedir || !*favoritedir || !textname || !*textname 
      || !vtksys::SystemTools::FileIsDirectory(favoritedir) 
      || !vtksys::SystemTools::FileExists(favoritedir))
    {
    return;
    }
    
  this->AddFavoriteDirectoryButton(textname,favoritedir);
  
  if (this->Internals->FavoriteDirEntries.size() >
    this->MaximumNumberOfFavoriteDirectoriesInRegistry)
    {
    this->PruneDirectoriesInRegistry();
    }
    
  this->WriteDirectoriesToRegistry();
    
  //Update the "Delete Favorites" button state
  this->Update();
} 

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::PopulateContextMenu(
  vtkKWMenu *menu, const char* path, int enabled)
{
  char command[256];
  vtksys_stl::string buttonpath = path;
#ifdef _WIN32
  // Explore to open native explorer file
  sprintf(command, "RightClickExploreCallback \"%s\"", 
    vtksys::SystemTools::EscapeChars(buttonpath.c_str(), 
       KWFileBrowser_ESCAPE_CHARS).c_str());
  menu->AddCommand("Explore", this, command);
#endif

  sprintf(command, "RightClickRenameCallback \"%s\"", 
    vtksys::SystemTools::EscapeChars(buttonpath.c_str(), 
       KWFileBrowser_ESCAPE_CHARS).c_str());
  // Rename file
  menu->AddCommand("Rename", this, command);

  sprintf(command, "RightClickRemoveFavoriteCallback \"%s\"", 
    vtksys::SystemTools::EscapeChars(buttonpath.c_str(), 
       KWFileBrowser_ESCAPE_CHARS).c_str());
  // Delete file
  menu->AddCommand("Delete", this, command);
  if (!enabled)
    {
    menu->EnabledOff();
    }
}
//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::DirectoryClickedCallback(
  const char* path, const char* text)
{
  this->SelectFavoriteDirectory(path);
  this->InvokeFavoriteDirectorySelectedCommand(path, text);
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::InvokeFavoriteDirectorySelectedCommand(
  const char* path, const char* text)
{
  if (this->FavoriteDirectorySelectedCommand 
    && *this->FavoriteDirectorySelectedCommand)
    {
    this->Script("%s \"%s\" \"%s\"", 
      this->FavoriteDirectorySelectedCommand, 
      vtksys::SystemTools::EscapeChars(path, 
       KWFileBrowser_ESCAPE_CHARS).c_str(), 
       vtksys::SystemTools::EscapeChars(text, 
       KWFileBrowser_ESCAPE_CHARS).c_str());
    }
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::DirectoryRightClickedCallback(
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
  this->PopulateContextMenu(this->ContextMenu, path, 1);
  if (this->ContextMenu->GetNumberOfItems())
    {
    this->ContextMenu->PopUp(x, y);
    }
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::RightClickRemoveFavoriteCallback(
  const char* path)
{
  if (!path || !(*path))
    {
    return;
    }

  vtksys_stl::string fullpath = path;
  vtksys_stl::string message(
    "Are you sure you want to delete this favorite directory? \n");
  message.append(fullpath.c_str());

      // Prompt the user for confirmation
  if (vtkKWMessageDialog::PopupYesNo( 
    this->GetApplication(), 
    this, 
    ks_("Favorite Directories|Title|Delete favorites"),
    k_(message.c_str()),
    vtkKWMessageDialog::WarningIcon | 
    vtkKWMessageDialog::InvokeAtPointer))
    {
    this->RemoveFavoriteDirectory(fullpath.c_str());
    this->Update();
    }
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::RightClickExploreCallback(
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
void vtkKWFavoriteDirectoriesFrame::RightClickRenameCallback(
  const char* path)
{
  if (!path || !(*path))
    {
    return;
    }

  vtksys_stl::string fullpath = path;
  vtksys_stl::string txtname = "";
  // Check if the input favorite dir name is already used 
  vtkKWFavoriteDirectoriesFrameInternals::DirEntryContainerIterator 
    it = this->Internals->FavoriteDirEntries.begin();
  if (this->Internals->FavoriteDirEntries.size() > 0)
    {
    for(; it!=this->Internals->FavoriteDirEntries.end(); it++)
      {
      if (strcmp(((*it)->FullPath).c_str(), fullpath.c_str()) ==0)
        {
        txtname = (*it)->DisplayText;
        break;
        }
      }
    }

  if (strcmp(txtname.c_str(), "") ==0)
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
  dlg->GetEntry()->GetWidget()->SetValue(txtname.c_str());
  dlg->SetText(
    ks_("Favorite Directories|Dialog|Enter a new favorite name:"));
  
  dlg->GetEntry()->GetWidget()->SetBinding("<Return>", dlg, "OK");
  dlg->GetOKButton()->SetBinding("<Return>", dlg, "OK");
  dlg->GetCancelButton()->SetBinding("<Return>", dlg, "Cancel");

  vtksys_stl::string newname;
  int ok = dlg->Invoke();
  if (ok)
  {
    newname = dlg->GetEntry()->GetWidget()->GetValue();
    if (newname.empty() || strcmp(newname.c_str(), "")==0 ||
      strcmp(newname.c_str(), ".")==0 ||
      strcmp(newname.c_str(), "..")==0)
      {
      vtkKWMessageDialog::PopupMessage(
        this->GetApplication(), this, 
        ks_("Favorite Directories|Title|Error!"),
        "You must enter a valid favorite name!", 
        vtkKWMessageDialog::ErrorIcon | 
        vtkKWMessageDialog::InvokeAtPointer);
      dlg->Delete();
      return;
      }
    dlg->Delete();
    }
  else
    {
    dlg->Delete();
    return;
    }    

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
  
  int nb_children = this->FavoriteButtonFrame->GetFrame()->
    GetNumberOfPackedChildren();
  for(int index=0; index<nb_children; index++)
    {
    vtkKWPushButton *child = vtkKWPushButton::SafeDownCast(
      this->FavoriteButtonFrame->GetFrame()->GetNthChild(index));
    if (child && child->IsPacked()
      && strcmp(child->GetText(), txtname.c_str())==0)
      {
      (*it)->DisplayText = newname.c_str(); 
      child->SetText(newname.c_str());
      this->WriteDirectoriesToRegistry();
      return;
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::AddFavoriteDirectoryButton(
  const char* text, const char *fullpath)
{
  // Check if the favorite folder is already there, 
  // if yes, popup error message
  if (this->HasFavoriteDirectory(fullpath))
    {
    return;
    }

  // Add selected folder to favoriate directory list
  vtkKWFavoriteDirectoriesFrameInternals::DirectoryEntry *direntry =
    new vtkKWFavoriteDirectoriesFrameInternals::DirectoryEntry;
  direntry->FullPath = fullpath;
  direntry->DisplayText = text;
  
  this->Internals->FavoriteDirEntries.push_front(direntry);

  vtkKWPushButton *dirbutton = vtkKWPushButton::New(); 
  dirbutton->SetParent(this->FavoriteButtonFrame->GetFrame());
  dirbutton->Create();
  dirbutton->SetText(text);
  dirbutton->SetReliefToFlat();
  dirbutton->SetOverReliefToRaised();
  dirbutton->SetCompoundModeToTop();
  dirbutton->SetBalloonHelpString(fullpath);
  dirbutton->SetImageToPredefinedIcon(vtkKWIcon::IconFolder32);
  dirbutton->SetBackgroundColor(this->FavoriteButtonFrame->
    GetBackgroundColor());
  dirbutton->SetActiveBackgroundColor(this->FavoriteButtonFrame->
    GetBackgroundColor());
  dirbutton->SetConfigurationOptionAsInt("-takefocus", 0);

  char cmd[256];
  sprintf(cmd, "DirectoryClickedCallback \"%s\" \"%s\"",
    vtksys::SystemTools::EscapeChars(fullpath, 
       KWFileBrowser_ESCAPE_CHARS).c_str(), text);
  dirbutton->SetCommand(this, cmd);

  char rightclickcmd[256];
  sprintf(rightclickcmd, "DirectoryRightClickedCallback \"%s\" %%X %%Y",
    vtksys::SystemTools::EscapeChars(fullpath, 
       KWFileBrowser_ESCAPE_CHARS).c_str());
  dirbutton->AddBinding("<Button-3>", this, rightclickcmd);

  int nb_children = this->FavoriteButtonFrame->GetFrame()->
    GetNumberOfPackedChildren();
                    
  if (nb_children > 0)
    {
    this->Script("pack %s -side top -fill x -pady 2 -before %s", 
      dirbutton->GetWidgetName(), 
      this->FavoriteButtonFrame->GetFrame()->
        GetNthChild(nb_children-1)->GetWidgetName());
    }
  else
    {
    this->Script("pack %s -side top -fill x -pady 2", 
      dirbutton->GetWidgetName());
    }
                    
  //Update "Delete Favorite" button state
  this->Update();
  dirbutton->Delete();
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::RemoveFavoriteDirectory(
  const char *fullpath)
{
  vtkKWFavoriteDirectoriesFrameInternals::DirEntryContainerIterator 
    it = this->Internals->FavoriteDirEntries.begin();
  bool found=false;
  if (this->Internals->FavoriteDirEntries.size()>0)
    {
    for(; it != this->Internals->FavoriteDirEntries.end(); it++)
      {
      if (strcmp((*it)->FullPath.c_str(), fullpath)==0)
        { 
        found = true;
        break;
        }
      }
    }
    
  if (found)
    {
    vtkKWPushButton *child = NULL;    
    int nb_children = this->FavoriteButtonFrame->
                      GetFrame()->GetNumberOfChildren();
    for (int index=0; index < nb_children; index++)
      {
      child = vtkKWPushButton::SafeDownCast(
          this->FavoriteButtonFrame->GetFrame()->GetNthChild(index));
      if (strcmp((*it)->DisplayText.c_str(), child->GetText())==0)
        {
        break;
        } 
      child = NULL;
      }
    if (child)
      {
      this->Internals->FavoriteDirEntries.erase(it);
      child->Unpack();
      child->SetParent(NULL);
      this->WriteDirectoriesToRegistry();
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::RelocateFavoriteDirectory(
  const char* oldname, 
  const char* newname)
{
  vtkKWFavoriteDirectoriesFrameInternals::DirEntryContainerIterator 
    it = this->Internals->FavoriteDirEntries.begin();
  bool found=false;
  if (this->Internals->FavoriteDirEntries.size()>0)
    {
    for(; it != this->Internals->FavoriteDirEntries.end(); it++)
      {
      if (strcmp((*it)->FullPath.c_str(), oldname)==0)
        { 
        found = true;
        break;
        }
      }
    }
    
  if (found)
    {
    vtkKWPushButton *child = NULL;    
    int nb_children = this->FavoriteButtonFrame->
      GetFrame()->GetNumberOfChildren();
    for (int index=0; index < nb_children; index++)
      {
      child = vtkKWPushButton::SafeDownCast(
        this->FavoriteButtonFrame->GetFrame()->GetNthChild(index));
      if (strcmp((*it)->DisplayText.c_str(), child->GetText())==0)
        {
        break;
        } 
      child = NULL;
      }
    if (child)
      {
      (*it)->FullPath = newname;
      (*it)->DisplayText = vtksys::SystemTools::GetFilenameName(
        newname).c_str();
      child->SetText((*it)->DisplayText.c_str());
        
      char cmd[256];
      sprintf(cmd, "DirectoryClickedCallback \"%s\" \"%s\"",
        vtksys::SystemTools::EscapeChars((*it)->FullPath.c_str(), 
           KWFileBrowser_ESCAPE_CHARS).c_str(), 
        vtksys::SystemTools::EscapeChars(child->GetText(),
           KWFileBrowser_ESCAPE_CHARS).c_str());
      child->SetCommand(this, cmd);

      this->WriteDirectoriesToRegistry();
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::WriteDirectoriesToRegistry()
{
  this->WriteDirectoriesToRegistry(
    this->RegistryKey, 
    this->MaximumNumberOfFavoriteDirectoriesInRegistry);
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::WriteDirectoriesToRegistry(
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
  vtkKWRegistryHelper* registryhelper = 
    vtkKWRegistryHelper::New();
                                       
  if (!registryhelper)
    {
    vtkErrorMacro(
      "Error! Failed to create vtkKWRegistryHelper class!");
    return;
    }
  else
    {
    if (!registryhelper->Open(
      VTK_KW_FAVORITE_TOPLEVEL, reg_key, 1)) 
      {
      vtkErrorMacro(
        "Error! Failed to open the registry key for writing!");
      registryhelper->Close();
      return;
      }
    }  

  // Store all favorite dir entries to registry
  vtkKWFavoriteDirectoriesFrameInternals::DirEntryContainerIterator 
    it = this->Internals->FavoriteDirEntries.begin();
  vtkKWFavoriteDirectoriesFrameInternals::DirEntryContainerIterator 
    end = this->Internals->FavoriteDirEntries.end();
  int count = 0;
  if (this->Internals->FavoriteDirEntries.size()>0)
    {
    for (; it != end && count < max_nb; ++it)
      {
      if (*it)
        {
        sprintf(dirname_key,
          VTK_KW_DIR_REGISTRY_PATHNAME_KEYNAME_PATTERN, count);
        sprintf(label_key,
          VTK_KW_DIR_REGISTRY_LABEL_KEYNAME_PATTERN, count);

        registryhelper->SetValue(
          reg_key, dirname_key, (*it)->FullPath.c_str());
        if ((*it)->DisplayText.size())
          {
          registryhelper->SetValue(
            reg_key, label_key, (*it)->DisplayText.c_str());
          }
        ++count;
        }
      }
    }
  
  // As a convenience, remove all others
  for (; count < VTK_KW_DIR_REGISTRY_MAX_ENTRIES; count++)
    {
    sprintf(dirname_key,
      VTK_KW_DIR_REGISTRY_PATHNAME_KEYNAME_PATTERN, count);
    sprintf(label_key,
      VTK_KW_DIR_REGISTRY_LABEL_KEYNAME_PATTERN, count);
    registryhelper->DeleteValue(
      reg_key, dirname_key);
    registryhelper->DeleteValue(
      reg_key, label_key);
    }
  
  registryhelper->Close();
  registryhelper->Delete();
  
#ifdef _WIN32
    this->UpdateFavoriteDirectoriesOfSystemRegistry();
#endif
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::RestoreDirectoriesListFromRegistry()
{
  //Restore the set of registry keys created by users
  this->RestoreDirectoriesListFromUserRegistry(
    this->RegistryKey, 
    this->MaximumNumberOfFavoriteDirectoriesInRegistry);
    
#ifdef _WIN32
    // loading system favorites
    this->RestoreDirectoriesListFromSystemRegistry();
    this->WriteDirectoriesToRegistry();
#endif
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::RestoreDirectoriesListFromUserRegistry(
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
                                       
  if (!registryhelper)
    {
    vtkErrorMacro("Error! Failed to create vtkKWRegistryHelper class!");
    return;
    }
  else
    {
    if (!registryhelper->Open(
      VTK_KW_FAVORITE_TOPLEVEL, reg_key, 0)) 
      {
      registryhelper->Close();
      registryhelper->Delete();
      return;
      }
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
    if (registryhelper->ReadValue(
          reg_key, dirname_key, dirname) &&
        strlen(dirname) >= 1)
      {
      if (vtksys::SystemTools::FileIsDirectory(dirname))
        {          
        if (!registryhelper->ReadValue(
              reg_key, label_key, label))
          {
          *label = '\0';
          }
        this->AddFavoriteDirectoryButton(label, dirname);            
        max_nb--;
        }
      }
    }
  registryhelper->Close();
  registryhelper->Delete();  
}

//Restore the set of registry keys from the system
//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::RestoreDirectoriesListFromSystemRegistry()
{
#ifdef _WIN32  
  char buff[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  vtksys_stl::string placekey = VTK_KW_WIN32_REGIRSTRY_PLACES_BAR_KEY;
  bool userdefined = false;
  char place[10];
  vtksys_stl::string value, placename;
  // If this system key exists, read from it and 
  // add them to the favorite dir entries.
  vtkKWWin32RegistryHelper* registryhelper = 
    vtkKWWin32RegistryHelper::SafeDownCast(this->GetApplication()->
    GetRegistryHelper());
  if (registryhelper && 
    registryhelper->OpenInternal(placekey.c_str(), 0)) 
    {
    //Windows only allow five entries under this key
    for(int i=VTK_KW_WIN32_REGIRSTRY_NUM_PLACES-1; i>=0; i--)
      {
      sprintf(place, "Place%d", i);
      placename=place;
      
      if (registryhelper->ReadValueInternal(placename.c_str(), buff))
        {
        value = buff;
        if (vtksys::SystemTools::FileIsDirectory(value.c_str()))
          {          
          this->AddFavoriteDirectoryButton(
            vtksys::SystemTools::GetFilenameName(value).c_str(),
            value.c_str());            
          userdefined = true;
          }
        else
          {
          int csidl=-1;
          sscanf(value.c_str(), "%d", &csidl);
          if (csidl >=0 && csidl <=0x003d)
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
  
#endif
}

//Update the favorite directories for the system registry
//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::UpdateFavoriteDirectoriesOfSystemRegistry()
{
#ifdef _WIN32
  int num_dir = this->Internals->FavoriteDirEntries.size();
  if (num_dir<=0)
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
    vtkKWWin32RegistryHelper::SafeDownCast(
    this->GetApplication()->GetRegistryHelper());
  if (registryhelper && 
    registryhelper->OpenInternal(placekey.c_str(), 1)) 
    {
    // Store all favorite dir entries to registry
    vtkKWFavoriteDirectoriesFrameInternals::DirEntryContainerIterator 
      it = this->Internals->FavoriteDirEntries.begin();
    vtkKWFavoriteDirectoriesFrameInternals::DirEntryContainerIterator 
      end = this->Internals->FavoriteDirEntries.end();
    
    int found=0;
    //Windows only allow five entries under this key
    for(int i=0; it != end 
      && i<VTK_KW_WIN32_REGIRSTRY_NUM_PLACES; i++, it++)
      {
      found = 0;
      for(int j=0; j<VTK_KW_WIN32_REGIRSTRY_NUM_PLACES; j++)
        {
        sprintf(place, "Place%d", j);
        placename=place;
        
        if (registryhelper->ReadValueInternal(
          placename.c_str(), buff))
          {
          value = buff;
          
          if (vtksys::SystemTools::FileIsDirectory(value.c_str()))
            {          
            if (it != end && vtksys::SystemTools::ComparePath(
              value.c_str(), (*it)->FullPath.c_str()))  
              {
              found=true;
              break;
              }
            }
          else
            {
            int csidl=-1;
            sscanf(value.c_str(), "%d", &csidl);
            if (csidl >=0 && csidl <=0x003d)
              {
              //Get the path to these folders
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
                  if (it != end && vtksys::SystemTools::ComparePath(
                    szPath, (*it)->FullPath.c_str()))  
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
          registryhelper->SetValueInternal(
            placename.c_str(), value.c_str());
          }
        else
          {
          int csidl=-1;
          sscanf(value.c_str(), "%d", &csidl);
          if (csidl >=0 && csidl <=0x003d)
            {
            registryhelper->SetValueInternal(
              placename.c_str(), &csidl);
            }
          }
        }
      else
        {
        registryhelper->SetValueInternal(placename.c_str(), 
          (*it)->FullPath.c_str());
        }
      }//end for it, i
    registryhelper->Close();
    }//end if open   
  
#endif
}

//----------------------------------------------------------------------------
int vtkKWFavoriteDirectoriesFrame::AddSpecialFavoriteFolder(int csidl)
{
#ifdef _WIN32
  vtksys_stl::string text;
  switch(csidl)
    {
    case CSIDL_DESKTOP:
      text = "Desktop";
      break;
    case CSIDL_PERSONAL:
      text = "My Documents";
      break;
    case CSIDL_DRIVES:
      text = "My Computer";
      break;
    case CSIDL_FAVORITES:
      text = "My Favorites";
      break;
    default:
      text = "My Folder";
      break;
    } // end switch
  //Get the path to these folders
  LPITEMIDLIST pidl;
  HRESULT hr = SHGetSpecialFolderLocation(NULL, csidl, &pidl);
  if (SUCCEEDED(hr))
    {
    // Convert the item ID list's binary 
    // representation into a file system path
    char szPath[_MAX_PATH];
    if (SHGetPathFromIDList(pidl, szPath))
      {
      this->AddFavoriteDirectoryButton(text.c_str(), szPath);        
      return 1;
      }
    }//end if succeed
#endif  
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::ClearFavoriteDirectorySelection()
{
  int nb_children = this->FavoriteButtonFrame->GetFrame()->
                    GetNumberOfChildren();
  for(int index=0; index<nb_children; index++)
    {
    vtkKWPushButton *child = vtkKWPushButton::SafeDownCast(
      this->FavoriteButtonFrame->GetFrame()->GetNthChild(index));
    child->SetReliefToFlat();
    child->SetBackgroundColor(this->FavoriteButtonFrame->
      GetFrame()->GetBackgroundColor());
    }    
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();
  if (this->FavoriteButtonFrame)
    {
    this->PropagateEnableState(this->FavoriteButtonFrame);
    this->PropagateEnableState(this->ToolbarFavorite);
    this->PropagateEnableState(this->FavoritesAddingButton);
    }
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::Update()
{
  if (this->FavoriteButtonFrame)
    {
    this->UpdateEnableState();
    }
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::PruneDirectoriesInRegistry()
{
  while(this->Internals->FavoriteDirEntries.size() > 
    this->MaximumNumberOfFavoriteDirectoriesInRegistry)
    {
    this->RemoveFavoriteDirectory(
      this->Internals->FavoriteDirEntries.back()->FullPath.c_str());
    this->Internals->FavoriteDirEntries.pop_back();
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
  this->PruneDirectoriesInRegistry();
  this->WriteDirectoriesToRegistry();
  //Update the "Delete Favorites" button state
  this->Update();
  
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkKWFavoriteDirectoriesFrame::PrintSelf(
  ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  
  os << indent << "FavoriteDirectoryAddingCommand: " 
     << (this->FavoriteDirectoryAddingCommand?this->FavoriteDirectoryAddingCommand:"none")
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

  if (this->Internals->FavoriteDirEntries.size()>0)
    {
    vtkKWFavoriteDirectoriesFrameInternals::DirEntryContainerIterator 
      it = this->Internals->FavoriteDirEntries.begin();
    vtkKWFavoriteDirectoriesFrameInternals::DirEntryContainerIterator
      end = this->Internals->FavoriteDirEntries.end();
    for (; it != end; ++it)
      {
      os << indent << "FavoriteDirectory: " 
         << ((*it)->FullPath.c_str()?(*it)->FullPath.c_str():"none")
         << endl;
      }
    }
}
