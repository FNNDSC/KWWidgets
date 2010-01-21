/*=========================================================================

  Module:    $RCSfile: vtkKWDirectoryExplorer.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkKWDirectoryExplorer.h"

#include "vtkKWApplication.h"
#include "vtkKWEntry.h"
#include "vtkKWOptions.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWIcon.h"
#include "vtkKWInternationalization.h"
#include "vtkKWFileBrowserUtilities.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWPushButton.h"
#include "vtkKWPushButtonWithMenu.h"
#include "vtkKWScrollbar.h"
#include "vtkKWSimpleEntryDialog.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWToolbar.h"
#include "vtkKWTree.h"
#include "vtkKWTreeWithScrollbars.h"

#include "vtkDirectory.h"
#include "vtkObjectFactory.h"

#include <vtksys/SystemTools.hxx>
#include <vtksys/ios/sstream>
#include <vtksys/stl/string>
#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

#include <sys/stat.h>
#include <time.h>

#ifdef _WIN32
#define _WIN32_WINNT 0x0501
#include "vtkWindows.h" //for GetLogicalDrives on Windows
#include <shellapi.h>
#include <shlobj.h>
#endif

//#define _MY_DEBUG

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWDirectoryExplorer );
vtkCxxRevisionMacro(vtkKWDirectoryExplorer, "$Revision: 1.48 $");

vtkIdType vtkKWDirectoryExplorer::IdCounter = 1;

//----------------------------------------------------------------------------
class vtkKWDirectoryExplorerInternals
{
public:
  
  vtkKWDirectoryExplorerInternals()
  {
    this->RootNode = "root";
    
    this->IsNavigatingNode = 0;
    this->IsOpeningDirectory = 0;
    this->TempPath = "";
    this->MostRecentDirCurrent = this->MostRecentDirList.begin();
  }
 
  // Most recent directories list (history)

  typedef vtksys_stl::list<vtksys_stl::string> MostRecentDirContainer;
  typedef vtksys_stl::list<vtksys_stl::string>::iterator MostRecentDirIterator;
  
  MostRecentDirContainer MostRecentDirList;
  MostRecentDirIterator MostRecentDirCurrent;
  
  const char* RootNode;
  int IsNavigatingNode;
  int IsOpeningDirectory;
  vtksys_stl::string TempPath;
  vtksys_stl::string FolderImage;
};

//----------------------------------------------------------------------------
vtkKWDirectoryExplorer::vtkKWDirectoryExplorer()
{ 
  this->MaximumNumberOfDirectoriesInHistory = 20;
  
  this->Internals           = new vtkKWDirectoryExplorerInternals;

  this->Toolbar            = vtkKWToolbar::New();
  this->DirectoryTree      = vtkKWTreeWithScrollbars::New();
  this->CreateFolderButton = vtkKWPushButton::New();
  this->BackButton         = vtkKWPushButtonWithMenu::New();
  this->ForwardButton      = vtkKWPushButtonWithMenu::New();
  this->UpButton           = vtkKWPushButton::New();
  this->ContextMenu        = NULL;
  
  this->DirectorySelectedCommand = NULL;
  this->DirectoryCreatedCommand  = NULL;
  this->DirectoryDeletedCommand  = NULL;
  this->DirectoryOpenedCommand   = NULL;
  this->DirectoryClosedCommand   = NULL;
  this->DirectoryRenamedCommand  = NULL;
}

//----------------------------------------------------------------------------
vtkKWDirectoryExplorer::~vtkKWDirectoryExplorer()
{ 
  this->DirectoryTree->Delete();
  this->CreateFolderButton->Delete();
  this->BackButton->Delete();
  this->ForwardButton->Delete();
  this->UpButton->Delete();
  this->Toolbar->Delete();

  if (this->DirectoryCreatedCommand)
    {
    delete [] this->DirectoryCreatedCommand;
    this->DirectoryCreatedCommand = NULL;
    }

  if (this->DirectoryDeletedCommand)
    {
    delete [] this->DirectoryDeletedCommand;
    this->DirectoryDeletedCommand = NULL;
    }

  if (this->DirectoryOpenedCommand )
    {
    delete [] this->DirectoryOpenedCommand;
    this->DirectoryOpenedCommand = NULL;
    }

  if (this->DirectoryClosedCommand)
    {
    delete [] this->DirectoryClosedCommand;
    this->DirectoryClosedCommand = NULL;
    }

  if (this->DirectorySelectedCommand)
    {
    delete [] this->DirectorySelectedCommand;
    this->DirectorySelectedCommand = NULL;
    }

  if (this->DirectoryRenamedCommand)
    {
    delete [] this->DirectoryRenamedCommand;
    this->DirectoryRenamedCommand = NULL;
    }

  if (this->ContextMenu)
    {
    this->ContextMenu->Delete();
    this->ContextMenu = NULL;
    }
 
  if (this->Internals)
    {      
    delete this->Internals;
    }
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }
 
  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();
  
  // Toolbar

  this->Toolbar->SetParent(this);
  this->Toolbar->Create();

  // Go back button

  this->BackButton->SetParent(this->Toolbar->GetFrame());
  this->BackButton->Create();
  this->BackButton->GetPushButton()->SetImageToPredefinedIcon(
    vtkKWIcon::IconBrowserBack);
  this->BackButton->SetBalloonHelpString("Back to previous directory");
  this->BackButton->GetPushButton()->SetCommand(
    this,  "BackToPreviousDirectoryCallback");
  this->BackButton->GetPushButton()->SetConfigurationOptionAsInt(
    "-takefocus", 0);
  this->Toolbar->AddWidget(this->BackButton);
  
  // Go forward button

  this->ForwardButton->SetParent(this->Toolbar->GetFrame());
  this->ForwardButton->Create();
  this->ForwardButton->GetPushButton()->SetImageToPredefinedIcon(
    vtkKWIcon::IconBrowserForward);
  this->ForwardButton->SetBalloonHelpString("Go to next directory");
  this->ForwardButton->GetPushButton()->SetCommand(
    this, "ForwardToNextDirectoryCallback");
  this->ForwardButton->GetPushButton()->SetConfigurationOptionAsInt(
    "-takefocus", 0);
  this->Toolbar->AddWidget(this->ForwardButton);

  // Go up button

  this->UpButton->SetParent(this->Toolbar->GetFrame());
  this->UpButton->Create();
  this->UpButton->SetImageToPredefinedIcon(vtkKWIcon::IconBrowserUp);
  this->UpButton->SetBalloonHelpString("Go up one directory");
  this->UpButton->SetCommand(this, "GoUpDirectoryCallback");
  this->UpButton->SetConfigurationOptionAsInt(
    "-takefocus", 0);
  this->Toolbar->AddWidget(this->UpButton);
  
  // Create folder button

  this->CreateFolderButton->SetParent(this->Toolbar->GetFrame());
  this->CreateFolderButton->Create();
  this->CreateFolderButton->SetImageToPredefinedIcon(
    vtkKWIcon::IconFolderNew);
  this->CreateFolderButton->SetBalloonHelpString("Create new folder");
  this->CreateFolderButton->SetCommand(
    this, "CreateNewFolderCallback");
  this->CreateFolderButton->SetConfigurationOptionAsInt(
    "-takefocus", 0);
  this->Toolbar->AddWidget(this->CreateFolderButton);

  this->Toolbar->SetToolbarAspectToFlat();
  this->Toolbar->SetWidgetsAspectToFlat();

  this->Script("pack %s -side top -anchor nw", 
               this->Toolbar->GetWidgetName());           
   
  // Directory Tree

  this->DirectoryTree->SetParent(this);
  this->DirectoryTree->Create();
  this->DirectoryTree->GetVerticalScrollbar()->SetConfigurationOptionAsInt(
    "-takefocus", 0);
  this->DirectoryTree->GetHorizontalScrollbar()->SetConfigurationOptionAsInt(
    "-takefocus", 0);

  vtkKWTree *dirtree = this->DirectoryTree->GetWidget();  
  dirtree->SetBorderWidth(2);
  dirtree->SetReliefToGroove();
  dirtree->SetSelectionModeToSingle();
  dirtree->SelectionFillOn();
  dirtree->SetLinesVisibility(0);
  dirtree->SetPadX(20);
  dirtree->SetWidth(30);
  dirtree->SetHeight(15);
  dirtree->SetBackgroundColor(1.0, 1.0, 1.0);
  dirtree->UseRawNodeUserDataOn();
  dirtree->UseRawNodeTextOn();

  dirtree->SetOpenCommand(this,"DirectoryOpenedCallback");
  dirtree->SetCloseCommand(this, "DirectoryClosedCallback");
  dirtree->SetSingleClickOnNodeCommand(this, "SingleClickOnNodeCallback");
  dirtree->SetBinding("<Delete>", this, "RemoveSelectedNodeCallback");
  dirtree->SetSelectionChangedCommand(this, "DirectorySelectedCallback");
  dirtree->SetRightClickOnNodeCommand(this, "RightClickCallback %X %Y");
  dirtree->SetBinding("<F2>", this, "RenameCallback");
    
  this->Script(
    "pack %s -side top -fill both -expand true -padx 1 -pady 1",
    this->DirectoryTree->GetWidgetName());

  // Setup the image for the tree node

  this->Internals->FolderImage = dirtree->GetWidgetName();
  this->Internals->FolderImage.append("_0");
  vtkKWIcon *tmpIcon = vtkKWIcon::New();
  tmpIcon->SetImage(vtkKWIcon::IconFolderXP);
  if (!vtkKWTkUtilities::UpdatePhoto(this->GetApplication(),
                                     this->Internals->FolderImage.c_str(),
                                     tmpIcon->GetData(), 
                                     tmpIcon->GetWidth(), 
                                     tmpIcon->GetHeight(), 
                                     tmpIcon->GetPixelSize()))
    {
    vtkWarningMacro(
      << "Error updating Tk photo " 
      << this->Internals->FolderImage.c_str());
    }
  tmpIcon->Delete();

  // Load root directory the first time we are mapped

  this->LoadRootDirectory();
  //  dirtree->SetBinding("<Map>", this, "LoadRootDirectoryCallback");

  //Update the Back/Forward button. Should be disabled

  this->Update();
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::LoadRootDirectoryCallback()
{
  this->DirectoryTree->GetWidget()->RemoveBinding(
    "<Map>", this, "LoadRootDirectoryCallback");
  
  this->LoadRootDirectory();
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::LoadRootDirectory()
{
#ifndef _WIN32           // UNIX flavor

  vtkIdType dirID = vtkKWDirectoryExplorer::IdCounter++;

  vtkKWIcon *tmpIcon = vtkKWIcon::New();
  tmpIcon->SetImage(vtkKWIcon::IconFolderXP);

  char strDirID[20];
  sprintf(strDirID, "%lu", dirID);

  this->AddDirectoryNode(
    this->Internals->RootNode, 
    strDirID, 
    "/", 
    KWFileBrowser_UNIX_ROOT_DIRECTORY, 
    tmpIcon);

  tmpIcon->Delete();

  //this->UpdateDirectoryNode(strDirID);
  this->OpenDirectoryNode(strDirID, 0, 1);

#else                   // Windows flavor

  vtksys_stl::string name;
  vtksys_stl::string realname;
  vtksys_stl::string disklabel;
  
  char strVolName[MAX_PATH];
  char strFS[MAX_PATH];
  DWORD serialnum;

  vtkKWIcon *tmpIcon = vtkKWIcon::New();

  // Loop over drive letters

  DWORD mask;
  for (mask = GetLogicalDrives(), name = "A:"; mask; mask >>= 1, name[0]++)
    {
    // Skip unavailable drives

    if (!(mask&1)) 
      {
      continue;
      }

    // Find out drive types

    UINT drivetype = GetDriveTypeA(name.c_str());

    switch (drivetype)
      {
      case DRIVE_REMOVABLE:
        if (name[0] == 'A' || name[0] == 'B')
          {
          tmpIcon->SetImage(vtkKWIcon::IconFloppyDrive);
          }
        else
          {
          //maybe a zip icon?
          tmpIcon->SetImage(vtkKWIcon::IconFloppyDrive);
          }
        disklabel = "Removable";
        break;

      case DRIVE_REMOTE:
        tmpIcon->SetImage(vtkKWIcon::IconNetDrive);
        disklabel = "Network";
        break;

      case DRIVE_CDROM:
        tmpIcon->SetImage(vtkKWIcon::IconCdRom);
        disklabel = "CD-ROM";
        break;

      case DRIVE_RAMDISK:
        tmpIcon->SetImage(vtkKWIcon::IconFolderXP);
        disklabel = "RAM Disk";
        break;

      case DRIVE_FIXED:
        tmpIcon->SetImage(vtkKWIcon::IconHardDrive);
        disklabel = "Local Disk";
        break;

      case DRIVE_UNKNOWN:
      case DRIVE_NO_ROOT_DIR:
      default:
        tmpIcon->SetImage(vtkKWIcon::IconFolderXP);
        disklabel = "Unknown";
        break;
      }

    // if there is an end backslash, remove it.

    char* cleaned_name = vtksys::SystemTools::RemoveChars(
      name.c_str(), "\\");
    if (cleaned_name)
      {
      name = cleaned_name;
      delete [] cleaned_name;
      }
    else
      {
      continue;
      }

    realname = disklabel;

    if(drivetype != DRIVE_REMOTE)
      {
      vtksys_stl::string volPath = name;
      if (GetVolumeInformation(volPath.append("\\").c_str(),
        strVolName, MAX_PATH, &serialnum, NULL,
        NULL, strFS, 
        MAX_PATH))
        {
        if (strcmp(strVolName, ""))
          {
          realname = strVolName;
          }
        }
      }
    else
      {
      DWORD dwUniSz = 1024;
      if(!WNetGetConnection(name.c_str(),
        strVolName,
        &dwUniSz))
        {
        if (strcmp(strVolName, ""))
          {
          realname = strVolName;
          }
        }
      }

    realname += " (";
    realname += name;
    realname += ")";
      
    // Add directory node

    vtkIdType dirID = vtkKWDirectoryExplorer::IdCounter++;
    char strDirID[20];
    sprintf(strDirID, "%lu", dirID);
    // Be consistent with SystemTools::GetParentDirectory() returning as "C:/"
    
    vtksys_stl::string dirname = name;
    dirname += '\\';
    this->AddDirectoryNode(this->Internals->RootNode, 
                           strDirID, 
                           realname.c_str(), 
                           dirname.c_str(), 
                           tmpIcon);
    }

  tmpIcon->Delete();
#endif
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::AddDirectoryNode(
  const char* parentnode, 
  const char* node, 
  const char* text, 
  const char* fullname, 
  vtkKWIcon *nodeicon)
{
  this->DirectoryTree->GetWidget()->AddNode(parentnode, node, text);
  this->DirectoryTree->GetWidget()->SetNodeUserData(
    node, vtksys::SystemTools::EscapeChars(
      fullname, KWFileBrowser_ESCAPE_CHARS).c_str());
  this->DirectoryTree->GetWidget()->SetNodeImageToIcon(node, nodeicon);
}

//----------------------------------------------------------------------------
bool vtkKWDirectoryExplorerSortDirPredicate(const char *d1, const char *d2)
{
  return vtksys::SystemTools::Strucmp(d1, d2) < 0 ? true : false;
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::UpdateDirectoryNode(const char* node)
{
#if defined (_MY_DEBUG)  
  cout << "-----------------UpdateDirectoryNode: " << nodepath << endl;
  clock_t start = clock();
#endif

  vtkKWTree *dirtree = this->DirectoryTree->GetWidget();
  const char *dirtreename = dirtree->GetWidgetName();
  
  // Check if this node has children, if yes,
  // check if those directory still there.

  vtksys_stl::vector<vtksys_stl::string> children;
  vtksys::SystemTools::Split(
    dirtree->GetNodeChildren(node), children, ' ');
  vtksys_stl::vector<vtksys_stl::string>::iterator node_it, node_end;
  int num_children = (int)children.size();

  vtksys_stl::list<vtksys_stl::string> children_text;
  vtksys_stl::list<vtksys_stl::string>::iterator node_text_it, node_text_end;
  
  node_it = children.begin();
  node_end =  children.end();
  for (; node_it != node_end; node_it++)
    {
    children_text.push_back(dirtree->GetNodeText((*node_it).c_str()));
    }

  node_text_end =  children_text.end();
  
#if defined (_MY_DEBUG)  
  double durationchildren = (double)(clock() - start) / CLOCKS_PER_SEC;
  cout << "Get Node children time: " << durationchildren << endl;
  clock_t start = clock();
#endif

  vtksys_stl::string nodepath = dirtree->GetNodeUserData(node);

  vtkDirectory *dir = vtkDirectory::New();
  if (!dir->Open(nodepath.c_str()))
    {
    dir->Delete();
    return;
    }
  
  int num_files = dir->GetNumberOfFiles();
  
#if defined (_MY_DEBUG)  
  double durationopen = (double)(clock() - start) / CLOCKS_PER_SEC;
  cout << "Dir open time: " << durationopen << endl;
  start = clock();
#endif
  
  // First collect all the dirs

  vtksys_stl::vector<const char*> dir_list;
  dir_list.reserve(num_files);
  bool dotfound = false, dotdotfound = false;

  for (int i = 0; i < num_files; i++)
    {
    const char *filename = dir->GetFile(i);

    // skip . and ..

    if (!dotfound || !dotdotfound)
      { 
      if (strcmp(filename, ".") == 0)
        {
        dotfound=true;
        continue;
        }
      else if (strcmp(filename, "..") == 0)
        {
        dotdotfound = true;
        continue;
        }
      }
      
    if (dir->FileIsDirectory(filename))
      {
      dir_list.push_back(filename);
      }
    }

  // Sort them, and process one by one

#ifdef _WIN32
  // Already sorted on Win32
#else
  vtksys_stl::sort(dir_list.begin(), dir_list.end(), 
                   vtkKWDirectoryExplorerSortDirPredicate);
#endif

  vtkIdType dirID;
  char strDirID[20];

  // Have these two flags so that we do not need to do strcmp
  // for every file in the directory

  vtksys_ios::ostringstream tk_cfgcmd;
  vtksys_ios::ostringstream tk_treecmd;

  // The following variables is for checking whether a
  // directory has sub directories; if yes, then add the 
  // '+' before the tree node

  vtkDirectory *tmpdir = vtkDirectory::New();
  vtksys_stl::string tmp_str, tmp_file, tmp_name;
  
  vtksys_stl::string treecmd = dirtreename;
  treecmd.append(" insert end ").append(node).append(" ");
  
  vtksys_stl::string fullname = "";
  const char* image_name = this->Internals->FolderImage.c_str();
   
  if (!KWFileBrowser_HasTrailingSlash(nodepath.c_str()))
    {
    nodepath += KWFileBrowser_PATH_SEPARATOR;
    }

#if defined (_MY_DEBUG)  
  clock_t scriptstart = clock();
#endif

  vtksys_stl::vector<const char*>::iterator dir_list_it = dir_list.begin();
  vtksys_stl::vector<const char*>::iterator dir_list_end = dir_list.end();

  int nb_new_dirs = 0;
  int nb_dirs_found = 0;
  
  for (; dir_list_it != dir_list_end; dir_list_it++)
    {
    const char *filename = *dir_list_it;

    // if the node already has children, we need to find any 
    // new directories and add them

    bool isadded = false;
    if (num_children && nb_dirs_found < num_children)
      {
      node_text_it = children_text.begin();
      for (; node_text_it != node_text_end; node_text_it++)
        {
        if (strcmp(node_text_it->c_str(), filename) == 0)
          {
          isadded = true;
          nb_dirs_found++;
          children_text.erase(node_text_it);
          break;
          }
        }
      }

    // if this directory is not added yet, add this new directory

    if (!isadded)
      {
      fullname = nodepath;
      fullname += filename;

      dirID = vtkKWDirectoryExplorer::IdCounter++;
      sprintf(strDirID, "%lu", dirID);
      tk_treecmd << treecmd;   
      tk_treecmd << strDirID << " -text {" 
                 << filename << "}" << " -image {" 
                 << image_name << "}" << " -data \"" 
                 << vtksys::SystemTools::EscapeChars(
                   fullname.c_str(), KWFileBrowser_ESCAPE_CHARS).c_str() 
                 << "\"" << endl;

      nb_new_dirs++;  

#ifdef _WIN32 // disable that on Unix, too slow for NFS

#if defined (_MY_DEBUG)  
      start = clock();
#endif

#if 1
      // Check if this new folder has subfolders.
      
      if (!tmpdir->Open(fullname.c_str()))
        {
        continue;
        }
        
      tmp_str = fullname;
      if(!KWFileBrowser_HasTrailingSlash(tmp_str.c_str()))
        {
        tmp_str += KWFileBrowser_PATH_SEPARATOR;
        }

      bool dot1found = false;
      bool dot2found = false;

      for(int j = 0; j < tmpdir->GetNumberOfFiles(); j++)
        {
        tmp_name = tmp_str;
        tmp_file = tmpdir->GetFile(j);

        // Skip . and ..

        if (!dot1found || !dot2found)
          {
          if (!dot1found && strcmp(tmp_file.c_str(), ".") == 0)
            {
            dot1found = true;
            continue;
            }
          if (!dot2found && strcmp(tmp_file.c_str(), "..") == 0)
            {
            dot2found = true;
            continue;
            }
          }
        tmp_name += tmp_file;
        struct stat tmp_fs;
        if (stat(tmp_name.c_str(), &tmp_fs) != 0)
          {
          continue;
          }
        else
          {
          if (
#if defined( _WIN32 )
            tmp_fs.st_mode & _S_IFDIR
#else
            (S_ISDIR(tmp_fs.st_mode))
#endif
            )
            {
            tk_cfgcmd << dirtreename << " itemconfigure " 
                      << strDirID << " -drawcross allways" << endl;
            break;
            }
          }
        } //end for

#else // Try differently? Crashs on some Unixes...

      vtksys_stl::string tmpNewdir =  vtksys::SystemTools::EscapeChars(
        fullname.c_str(), KWFileBrowser_ESCAPE_CHARS);
      
      tk_cfgcmd << "set dir \"" << tmpNewdir.c_str() << "\"" <<endl;
      tk_cfgcmd << "set contents [glob -nocomplain -directory $dir -type d *]" << endl;
      tk_cfgcmd << "if { [llength $contents] } {" << endl;
      tk_cfgcmd << dirtreename << " itemconfigure " << strDirID << " -drawcross allways" << endl;
      tk_cfgcmd << "} else { " << endl;
      tk_cfgcmd << "set newcontents [glob -nocomplain -directory $dir -type d .*]" << endl;
      tk_cfgcmd << "set pos [lsearch -exact $newcontents \"" <<tmpNewdir.c_str()<<"/.\"]" <<endl;
      tk_cfgcmd << "if { $pos >= 0 } {set newcontents [lreplace $newcontents $pos $pos]}" <<endl;
      tk_cfgcmd << "set pos [lsearch -exact $newcontents \"" << tmpNewdir.c_str() << "/..\"]" <<endl;
      tk_cfgcmd << "if { $pos >= 0 } {set newcontents [lreplace $newcontents $pos $pos]}" <<endl;
      tk_cfgcmd << "if { [llength $newcontents] } {" << endl;
      tk_cfgcmd << dirtreename << " itemconfigure " << strDirID << " -drawcross allways }" << endl;
      tk_cfgcmd << " }" <<endl;
#endif

#if defined (_MY_DEBUG)  
      double durationsub = (double)(clock() - start) / CLOCKS_PER_SEC;
      cout << tmp_name << "---- Check sub folder time: "   
           << durationsub << endl;
#endif

#endif // Win32, do not check for subdirs, too slow for NFS
       
      }//end if (!added)
    }//end for

  tmpdir->Delete();
    
#if defined (_MY_DEBUG)  
  double durationscript = (double)(clock() - scriptstart) / CLOCKS_PER_SEC;
  cout << "Creat Script time: " << durationscript << endl;
  start = clock();
#endif

  // Run add the tree node command if available

  if (tk_treecmd.str() != "" && nb_new_dirs > 0)
    {
    vtkKWTkUtilities::EvaluateSimpleString(
      this->GetApplication(), tk_treecmd.str().c_str());
    }

  // Run the script for adding/removing 'cross' image to tree node

  if (tk_cfgcmd.str() != "")
    {
    vtkKWTkUtilities::EvaluateSimpleString(
      this->GetApplication(), tk_cfgcmd.str().c_str());
    }

#if defined (_MY_DEBUG)  
  double durationrun = (double)(clock() - start) / CLOCKS_PER_SEC;
  cout << "Run script time: " << durationrun << endl;
  start = clock();
#endif

  // The node has child nodes already, we need to make sure
  // the child node directories are still there, 
  // otherwise, remove them

  if (num_children && nb_dirs_found < num_children)
    {
    node_it = children.begin();
    for (; node_it != node_end; node_it++)
      {
      if (!vtksys::SystemTools::FileExists(
            dirtree->GetNodeUserData((*node_it).c_str())))
        {
        dirtree->DeleteNode((*node_it).c_str());
        }
      }
    }

  dir->Delete();

#if defined (_MY_DEBUG)  
  double durationclean = (double)(clock() - start) / CLOCKS_PER_SEC;
  cout << "Check dir exists time: " << durationclean << endl;
#endif
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::OpenDirectoryNode(const char* node, 
                                               int select,
                                               int opennode)
{
  // Change mouse cursor to wait.

  vtksys_stl::string node_str = node;
  if (!this->DirectoryTree->GetWidget()->HasNode(node_str.c_str()))
    {
    return;
    }

  vtkKWTkUtilities::SetTopLevelMouseCursor(this, "watch");

  // Set internal flag

  this->Internals->IsOpeningDirectory = 1;
    
  // Check/Load all the directories and files under this node  

  this->UpdateDirectoryNode(node_str.c_str());

  // Open the node

  if (opennode && 
    !this->DirectoryTree->GetWidget()->IsNodeOpen(node_str.c_str()))
    {
    this->DirectoryTree->GetWidget()->OpenNode(node_str.c_str());
    this->DirectoryTree->GetWidget()->DisplayChildNodes(node_str.c_str());
    }

  // Select the node

  if (select)
    {
    if(this->DirectoryTree->GetWidget()->GetSelectionMode() ==
      vtkKWOptions::SelectionModeSingle)
      {
      this->DirectoryTree->GetWidget()->ClearSelection();
      }
    this->DirectoryTree->GetWidget()->SelectNode(node_str.c_str());
    this->DirectoryTree->GetWidget()->SeeNode(node_str.c_str());
    this->InvokeDirectorySelectedCommand(this->GetNthSelectedDirectory(0));
    }
    
  // Update dir history list

  if (!this->Internals->IsNavigatingNode)
    {     
    this->UpdateMostRecentDirectoryHistory(node_str.c_str());
    }
 
  // Update Back/Forward button state  

  this->Update();
  
  this->Internals->IsOpeningDirectory = 0;

  // Set back mouse cursor

  vtkKWTkUtilities::SetTopLevelMouseCursor(this, NULL);
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::Update()
{
  if (!this->DirectoryTree->IsCreated())
    {
    return;
    }

  this->UpdateEnableState();

  vtksys_stl::string callback = "OpenDirectoryNodeCallback ";

  // History buttons

  if (this->Internals->MostRecentDirList.size() > 1)
    {
    // First item in the list

    if (this->Internals->MostRecentDirCurrent == 
        this->Internals->MostRecentDirList.begin())
      { 
      this->ForwardButton->SetEnabled(0);
      }
    // Last item in the list  

    else if (strcmp((*this->Internals->MostRecentDirCurrent).c_str(), 
                    this->Internals->MostRecentDirList.back().c_str()) == 0)
      {
      this->BackButton->SetEnabled(0);
      }
    }
  else
    {
    this->ForwardButton->SetEnabled(0);
    this->BackButton->SetEnabled(0);
    }

  // History forward/Backward

  vtkKWTree *tree = this->DirectoryTree->GetWidget();  

  if (this->Internals->MostRecentDirCurrent != 
      this->Internals->MostRecentDirList.end())
    {
    // History forward

    vtkKWMenu *menu = this->ForwardButton->GetMenu();
    menu->DeleteAllItems();

    vtkKWDirectoryExplorerInternals::MostRecentDirIterator it = 
      this->Internals->MostRecentDirCurrent;
          
    vtksys_stl::string menucommand;
    vtksys_stl::string menutext;
    int offset = -1;
    char buff[10];
  
    // Keep the most recent item
  
    if (it != this->Internals->MostRecentDirList.begin())
      {
      it--;
      }
  
    while (it != this->Internals->MostRecentDirList.begin())
      {
      menutext = tree->GetNodeText((*it).c_str());
      menucommand = callback;
      menucommand.append((*it).c_str());
      sprintf(buff, " %d", offset);
      menucommand.append(buff);
      menu->AddCommand(menutext.c_str(), this, menucommand.c_str());
      offset--;
      it--;
      }
  
    // Now, the first item of the list  
    // NOTE: that deserve more comment, what is going on here?
    
    if (tree->HasNode((*it).c_str()))
      {
      menucommand = callback;
      menutext = tree->GetNodeText((*it).c_str());
      menucommand.append((*it).c_str());
      sprintf(buff, " %d", offset);
      menucommand.append(buff);
      menu->AddCommand(menutext.c_str(), this, menucommand.c_str());
      }
    else
      {
      this->Internals->MostRecentDirList.erase(it);
      }
    
    // History backward
  
    menu = this->BackButton->GetMenu();
    menu->DeleteAllItems();
    it = this->Internals->MostRecentDirCurrent;
    offset = 1;

    // Keep the most recent item
    
    if (it != this->Internals->MostRecentDirList.end())
      {
      it++;
    }
    
    // Skip the last iterator
    
    while (it != this->Internals->MostRecentDirList.end())
      {
      menutext = tree->GetNodeText((*it).c_str());
      menucommand = callback;
      menucommand.append((*it).c_str());
      sprintf(buff, " %d", offset);
      menucommand.append(buff);
      menu->AddCommand(menutext.c_str(), this, menucommand.c_str());
      it++;
      offset++;
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::ScrollToDirectory(const char* prefix)
{
  if(prefix && *prefix)
    {
    vtkKWTree *dirTree = this->DirectoryTree->GetWidget();
    vtksys_stl::string parentnode = this->GetNthSelectedNode(0);

    vtksys_stl::vector<vtksys_stl::string> children;
    vtksys::SystemTools::Split(dirTree->
      GetNodeChildren(parentnode.c_str()), children, ' ');
    vtksys_stl::vector<vtksys_stl::string>::iterator it = children.begin();
    vtksys_stl::vector<vtksys_stl::string>::iterator end = children.end();

    vtksys_stl::string nodetext;
    
    for (; it != end; it++)
      {
      nodetext = dirTree->GetNodeText((*it).c_str());
      if (!strncmp(nodetext.c_str(), prefix, strlen(prefix)))
        {
        dirTree->SeeNode((*it).c_str());
        break;
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::ReloadDirectoryNode(const char* node)
{
  if (node && *node)
    {
    vtksys_stl::string node_str = node;
    
    // Open whole tree

    this->OpenWholeTree(node_str.c_str());
   
    // Open the directory node  

    this->OpenDirectoryNode(node_str.c_str());
  }
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::SelectDirectory(const char* dirname)
{
  if (!dirname || !(*dirname))
    {
    return;
    }
  vtksys_stl::string dirpath = dirname;
  vtkKWTree *tree =  this->DirectoryTree->GetWidget();

  vtkstd::vector<vtkstd::string> nodes;
  vtksys_stl::vector<vtksys_stl::string>::iterator it;

  // Take care of Unix root directory
  if (!strcmp(dirpath.c_str(), KWFileBrowser_UNIX_ROOT_DIRECTORY))
    {
#ifndef _WIN32
    vtksys::SystemTools::Split(
      tree->GetNodeChildren(this->Internals->RootNode), nodes, ' ');
    if(nodes.size()>0)
      {
      tree->SelectNode(nodes.front().c_str());
      this->InvokeDirectorySelectedCommand(KWFileBrowser_UNIX_ROOT_DIRECTORY);
      }
#endif
    return;
    }

  if(!vtksys::SystemTools::FileIsDirectory(dirpath.c_str()))
    {
    // Take care of Windows logic drives 
    // (vtksys::SystemTools::FileIsDirectory() does not recognize
    // drives in the format "C:" or "C:\"

    nodes.clear();
    vtksys::SystemTools::Split(
      tree->GetNodeChildren(this->Internals->RootNode), nodes, ' ');
    bool bFound = false;
    for(it = nodes.begin(); it != nodes.end(); it++)
      {
      if(!strcmp(tree->GetNodeUserData((*it).c_str()), dirpath.c_str()))
        {
        if(!this->IsNodeSelected((*it).c_str()))
          {
          tree->SelectNode((*it).c_str());
          this->InvokeDirectorySelectedCommand(dirpath.c_str());
          }
        break;
        }
      }
    }
  else
    {
    vtksys_stl::string parentdir = 
      vtksys::SystemTools::GetParentDirectory(dirpath.c_str());;

#ifndef _WIN32
  // since on unix, the GetParentDirectory return "" 
  // for root directory "/" let's add the root directory "/"
  if (parentdir.empty() || strcmp(parentdir.c_str(), "") == 0)
    {
    parentdir = KWFileBrowser_UNIX_ROOT_DIRECTORY;
    }
#endif  

    const char* parentnode = 
      this->OpenDirectoryInternal(parentdir.c_str(), 0);
    if(parentnode)
      {
      nodes.clear();
      vtksys::SystemTools::Split(
        tree->GetNodeChildren(parentnode), nodes, ' ');
      vtksys_stl::string nodedir;
      vtksys::SystemTools::ConvertToUnixSlashes(dirpath);
      for(it = nodes.begin(); it != nodes.end(); it++)
        {
        nodedir = tree->GetNodeUserData((*it).c_str());
        vtksys::SystemTools::ConvertToUnixSlashes(nodedir);
        if(vtksys::SystemTools::ComparePath(
          nodedir.c_str(), dirpath.c_str()))
          {
          if(!this->IsNodeSelected((*it).c_str()))
            {
            tree->SelectNode((*it).c_str());
            this->InvokeDirectorySelectedCommand(dirpath.c_str());
            }
          break;
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::DeselectDirectory(const char* dirname)
{
  if (!dirname || !(*dirname) ||
    !vtksys::SystemTools::FileIsDirectory(dirname))
    {
    return;
    }

  vtksys_stl::string dirpath = dirname;
  vtksys_stl::string nodedir;
  vtksys::SystemTools::ConvertToUnixSlashes(dirpath);

  vtkstd::vector<vtkstd::string> selnodes;
  vtksys::SystemTools::Split(
    this->DirectoryTree->GetWidget()->GetSelection(), selnodes, ' ');
  vtksys_stl::vector<vtksys_stl::string>::iterator it;

  for(it = selnodes.begin(); it != selnodes.end(); it++)
    {
    nodedir = this->DirectoryTree->GetWidget()->
      GetNodeUserData((*it).c_str());
    //Convert the path for comparing with other path
    vtksys::SystemTools::ConvertToUnixSlashes(nodedir);
    if (vtksys::SystemTools::ComparePath(nodedir.c_str(), 
      dirpath.c_str()))
      {
      this->DirectoryTree->GetWidget()->DeselectNode((*it).c_str());
      break;
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::ClearSelection()
{
  this->DirectoryTree->GetWidget()->ClearSelection();
}

//----------------------------------------------------------------------------
int vtkKWDirectoryExplorer::IsNodeSelected(const char* node)
{
  if (!node || !(*node) ||
    !this->DirectoryTree->GetWidget()->HasNode(node))
    {
    return 0;
    }
  vtksys_stl::string nodeId = node;

  vtkstd::vector<vtkstd::string> selnodes;
  vtksys::SystemTools::Split(
    this->DirectoryTree->GetWidget()->GetSelection(), selnodes, ' ');
  vtksys_stl::vector<vtksys_stl::string>::iterator it;

  for(it = selnodes.begin(); it != selnodes.end(); it++)
    {
    if(!strcmp((*it).c_str(), nodeId.c_str()))
      {
      return 1;
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
const char* vtkKWDirectoryExplorer::ReloadDirectory(
  const char* node, 
  const char* dirname)
{
  // if the node is not changed, meaning the node and path 
  // are still the same, reload the node

  if (node && *node && 
     this->DirectoryTree->GetWidget()->HasNode(node) &&
     strcmp(dirname, 
            this->DirectoryTree->GetWidget()->GetNodeUserData(node)) == 0)
    {
    // If this node is not the selected node
    if (strcmp(this->GetNthSelectedNode(0), node) != 0)
      {
      this->ReloadDirectoryNode(node);
      }
    return node;
    }

  // If the node is not there or the directory is changed

  else if (dirname && *dirname && 
           vtksys::SystemTools::FileIsDirectory(dirname))
    {
    // If this directory is already opened and selected, 
    // just return the node
    if (this->DirectoryTree->GetWidget()->HasSelection() 
        && !strcmp(dirname, this->GetSelectedDirectory()))
      {
      return this->GetNthSelectedNode(0);
      }
      
    // The directory is not loaded yet. Load the nodes and directories

    if (this->OpenDirectory(dirname))
      {
      return this->GetNthSelectedNode(0);
      }
    return NULL;
    }

  else // error
    {
    vtksys_stl::string message = "The direcotry does not exist: \n";
    message.append(dirname);
    vtkKWMessageDialog::PopupMessage(
      this->GetApplication(), this, 
      ks_("DirectoryExplorer|Title|Error!"),
      message.c_str(), 
      vtkKWMessageDialog::ErrorIcon | 
      vtkKWMessageDialog::InvokeAtPointer);
    
    return NULL;
    }
}

//----------------------------------------------------------------------------
int vtkKWDirectoryExplorer::OpenDirectory(const char* dirname)
{
  const char* newnode = this->OpenDirectoryInternal(dirname, 1);
  if (newnode)
    {
    vtksys_stl::string newnode_str(newnode);
    this->UpdateMostRecentDirectoryHistory(newnode_str.c_str());
    // update Back/Forward button state  
    this->Update();
    return 1;
    }
  return 0;
}

//----------------------------------------------------------------------------
const char* vtkKWDirectoryExplorer::OpenDirectoryInternal(
  const char* dirname, int select )
{
  if (!dirname || !(*dirname))
    {
    return NULL;
    }

  vtksys_stl::string path = dirname;
  // "!vtksys::SystemTools::FileIsDirectory(dirname)" does not
  // recognize "C:" or "C:/", so using vtkDirectory to check
  vtkDirectory *dir = vtkDirectory::New();
  if (!dir->Open(path.c_str()))
    {
    dir->Delete();
    return NULL;
    }
  dir->Delete();
  
  const char* childnode = NULL;
  // Take care of Unix root directory
  if (!strcmp(path.c_str(), KWFileBrowser_UNIX_ROOT_DIRECTORY))
    {
#ifndef _WIN32
    childnode = this->ReloadDirectory(
      this->Internals->RootNode, path.c_str(),select);
#endif
    }
  else
    {
    // Get all the directories for the node layers in the tree

    vtksys_stl::string rootdir = path;
    vtksys_stl::string parentdir = 
      vtksys::SystemTools::GetParentDirectory(path.c_str());
    vtksys_stl::list<vtksys_stl::string> dirlist;
    dirlist.push_front(path);

    // Find the most upper level node for this node
    // and save each level of the found directory into a list

    while (!parentdir.empty() && 
           strcmp(parentdir.c_str(), KWFileBrowser_UNIX_ROOT_DIRECTORY) &&
           strcmp(parentdir.c_str(), rootdir.c_str()) != 0)
      {
      rootdir = parentdir;
      dirlist.push_front(parentdir);
      parentdir = vtksys::SystemTools::GetParentDirectory(parentdir.c_str());
      }
    
  #ifndef _WIN32
    // since on unix, the GetParentDirectory return "" 
    // for directories like "/home" let's add the root directory "/"
    if (!strcmp(parentdir.c_str(), KWFileBrowser_UNIX_ROOT_DIRECTORY) ||
       parentdir.empty() || strcmp(parentdir.c_str(), "") == 0)
      {
      dirlist.push_front(KWFileBrowser_UNIX_ROOT_DIRECTORY);
      }
  #endif  

    vtksys_stl::string parentnode = this->Internals->RootNode;
    vtksys_stl::string subdir;
    
    // Reload back each directory in the list. 
    // Make sure to only select the last directory and
    // only load the files in that directory

    while (dirlist.size() > 1) //!dirlist.empty())
      {
      subdir = dirlist.front().c_str();
      const char* aNode = this->ReloadDirectory(parentnode.c_str(), 
                                                subdir.c_str(),
                                                0);
      if (!aNode || !(*aNode))
        {
        dirlist.clear();
        return NULL;
        }
      parentnode = aNode;
      dirlist.pop_front();
      }
      
    // Now, load the directory that will be selected and displayed

    subdir = dirlist.front().c_str();
    childnode = this->ReloadDirectory(
      parentnode.c_str(), subdir.c_str(),select);
    
    dirlist.clear();
    }
  
  if (!childnode || !(*childnode))
    {
    return NULL;
    }
  else
    {
    static char buffer[100];
    strcpy(buffer, childnode);
    return buffer;
    }
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::OpenSubDirectory(
  const char* parentnode,
  const char* fullname, 
  int select)
{
  vtksys_stl::string parentnode_str = parentnode;

  const char* subnode = this->ReloadDirectory(
    parentnode_str.c_str(), fullname, select);
                 
  if (subnode && *subnode &&
      this->DirectoryTree->GetWidget()->HasNode(parentnode_str.c_str()) &&
      !this->DirectoryTree->GetWidget()->IsNodeOpen(parentnode_str.c_str()))
    {
    this->Internals->IsOpeningDirectory = 1;
    this->DirectoryTree->GetWidget()->OpenNode(parentnode_str.c_str());
    this->Internals->IsOpeningDirectory = 0;
    }
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::SetSelectionMode(int arg)
{
  if (arg != this->DirectoryTree->GetWidget()->GetSelectionMode())
    {
    this->DirectoryTree->GetWidget()->SetSelectionMode(arg);
    }
}

void vtkKWDirectoryExplorer::SetSelectionModeToSingle() 
{ 
  this->SetSelectionMode(vtkKWOptions::SelectionModeSingle); 
};
void vtkKWDirectoryExplorer::SetSelectionModeToMultiple() 
{ 
  this->SetSelectionMode(vtkKWOptions::SelectionModeMultiple); 
};

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::Focus()
{
  if (this->IsCreated())
    {
    // force the focus of the window to the dir tree.
    this->DirectoryTree->GetWidget()->Focus();
    }
}

//----------------------------------------------------------------------------
int vtkKWDirectoryExplorer::HasFocus()
{
  if (this->IsCreated())
    {
    // force the focus of the window to the dir tree.
    return this->DirectoryTree->GetWidget()->HasFocus();
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::GetSelectionBackgroundColor(
  double *r, double *g, double *b)
{
  this->DirectoryTree->GetWidget()->GetSelectionBackgroundColor(r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWDirectoryExplorer::GetSelectionBackgroundColor()
{
  return this->DirectoryTree->GetWidget()->GetSelectionBackgroundColor();
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::SetSelectionBackgroundColor(
  double r, double g, double b)
{
  this->DirectoryTree->GetWidget()->SetSelectionBackgroundColor(r, g, b);
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::GetSelectionForegroundColor(
  double *r, double *g, double *b)
{
  this->DirectoryTree->GetWidget()->GetSelectionForegroundColor(r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWDirectoryExplorer::GetSelectionForegroundColor()
{
  return this->DirectoryTree->GetWidget()->GetSelectionForegroundColor();
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::SetSelectionForegroundColor(double r, double g, double b)
{
  this->DirectoryTree->GetWidget()->SetSelectionForegroundColor(r, g, b);
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::GetTreeBackgroundColor(
  double *r, double *g, double *b)
{
  this->DirectoryTree->GetWidget()->GetBackgroundColor(r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWDirectoryExplorer::GetTreeBackgroundColor()
{
  return this->DirectoryTree->GetWidget()->GetBackgroundColor();
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::SetTreeBackgroundColor(
  double r, double g, double b)
{
  this->DirectoryTree->GetWidget()->SetBackgroundColor(r, g, b);
}

//----------------------------------------------------------------------------
int vtkKWDirectoryExplorer::HasSelection()
{
  return this->DirectoryTree->GetWidget()->HasSelection();
}

//----------------------------------------------------------------------------
int vtkKWDirectoryExplorer::GetNumberOfSelectedDirectories()
{
  if (!this->DirectoryTree->GetWidget()->HasSelection())
    {
    return 0;
    }

  vtkstd::vector<vtkstd::string> selnodes;
  vtksys::SystemTools::Split(this->DirectoryTree->GetWidget()->GetSelection(), 
                             selnodes, ' ');

  return (int)selnodes.size();
}

//----------------------------------------------------------------------------
const char *vtkKWDirectoryExplorer::GetNthSelectedDirectory(int i)
{
  // TODO: this should be optimize. We are counting the number of selected
  // node twice for each GetNth (that's 2 splits per Nth!)
  // Optimisation should be done in vtkKWTree. See GetNthSelectedNode too.

  if (i < 0 || i >= this->GetNumberOfSelectedDirectories())
    {
    return NULL;
    }

  vtkstd::vector<vtkstd::string> selnodes;
  vtksys::SystemTools::Split(this->DirectoryTree->GetWidget()->GetSelection(), 
    selnodes, ' ');

  return KWFileBrowser_GetUnixPath(
    this->DirectoryTree->GetWidget()->GetNodeUserData(selnodes[i].c_str()));
}

//----------------------------------------------------------------------------
const char* vtkKWDirectoryExplorer::GetNthSelectedNode(int i)
{
  if (i < 0 || i >= this->GetNumberOfSelectedDirectories())
    {
    return NULL;
    }

  vtkstd::vector<vtkstd::string> selnodes;
  vtksys::SystemTools::Split(this->DirectoryTree->GetWidget()->GetSelection(), 
    selnodes, ' ');

  static char buffer[100];
  strcpy(buffer, selnodes[i].c_str());
  return buffer;
}

//----------------------------------------------------------------------------
const char* vtkKWDirectoryExplorer::ReloadDirectory(
  const char* parentnode, 
  const char* dirname,
  int select)
{
  vtksys_stl::string nodedir, nodepath;
  vtksys_stl::string dirpath = dirname;

  vtksys_stl::vector<vtksys_stl::string> children;
  vtksys::SystemTools::Split(
    this->DirectoryTree->GetWidget()->GetNodeChildren(parentnode), 
    children, ' ');

  vtksys_stl::vector<vtksys_stl::string>::iterator it = children.begin();
  vtksys_stl::vector<vtksys_stl::string>::iterator end = children.end();

  // Convert the path for comparing with other path

  vtksys::SystemTools::ConvertToUnixSlashes(dirpath);
  static char tmpStr[20];

  // Look for the directory under the parent node

  for (; it != end; it++)
    {
    nodedir = this->DirectoryTree->GetWidget()->GetNodeUserData(
      (*it).c_str());
    vtksys::SystemTools::ConvertToUnixSlashes(nodedir);
    if (vtksys::SystemTools::ComparePath(nodedir.c_str(), dirpath.c_str()))
      {
      this->Internals->IsNavigatingNode = 1;
      this->OpenDirectoryNode((*it).c_str(), select);
      strcpy(tmpStr, (*it).c_str());
      this->Internals->IsNavigatingNode = 0;
      return tmpStr;
      }
    }
  
  return NULL;
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::RemoveDirectoryFromHistory(
  const char* node)
{
  // Update the most recent directory pointer and the history list

  int oldsize = (int)this->Internals->MostRecentDirList.size();
  if (oldsize > 0)
    {
    vtksys_stl::string node_str = node;
    vtkKWTree *tree = this->DirectoryTree->GetWidget();
    const char *nodechildren = tree->GetNodeChildren(node_str.c_str());
    if (nodechildren && *nodechildren)
      {
      vtksys_stl::vector<vtksys_stl::string> children;
      vtksys::SystemTools::Split(nodechildren, children, ' ');
      vtksys_stl::vector<vtksys_stl::string>::iterator it = children.begin();
      for (; it != children.end(); it++)
        {
        this->RemoveDirectoryFromHistory((*it).c_str());
        }
      }

    this->Internals->MostRecentDirList.remove(node_str.c_str());

    int newsize = (int)this->Internals->MostRecentDirList.size();

    if (newsize > 0 && newsize < oldsize)
      {
      this->Internals->MostRecentDirCurrent = 
        this->Internals->MostRecentDirList.begin();
      this->Update();
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::UpdateMostRecentDirectoryHistory(
  const char* node)
{
  // Update the most recent directory pointer and the history list

  if (this->Internals->MostRecentDirList.size() > 0)
    {
    if (strcmp((*this->Internals->MostRecentDirCurrent).c_str(), 
               node) != 0)
      {
      if (this->Internals->MostRecentDirCurrent != 
          this->Internals->MostRecentDirList.begin())
        {
        this->Internals->MostRecentDirList.erase(
          this->Internals->MostRecentDirList.begin(), 
          this->Internals->MostRecentDirCurrent);
        }
      this->Internals->MostRecentDirList.push_front(node);
      this->Internals->MostRecentDirCurrent = 
        this->Internals->MostRecentDirList.begin();
      }
    }
  else
    {
    this->Internals->MostRecentDirList.push_front(node);
    this->Internals->MostRecentDirCurrent = 
      this->Internals->MostRecentDirList.begin();    
    }    
  if (this->Internals->MostRecentDirList.size() > 
      (size_t)this->MaximumNumberOfDirectoriesInHistory)
    {
    this->Internals->MostRecentDirList.pop_back();
    }
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::OpenWholeTree(const char* node)
{
  vtksys_stl::string node_str = node;
  if (!this->DirectoryTree->GetWidget()->HasNode(node_str.c_str()))
    {
    return;
    }

  vtksys_stl::string parentnode = 
    this->DirectoryTree->GetWidget()->GetNodeParent(node_str.c_str());

  this->Internals->IsOpeningDirectory=1;
  while (strcmp(parentnode.c_str(), this->Internals->RootNode) != 0)
    {
    if (!this->DirectoryTree->GetWidget()->IsNodeOpen(parentnode.c_str()))
      {
      this->DirectoryTree->GetWidget()->OpenNode(parentnode.c_str());
      }
    parentnode = this->DirectoryTree->GetWidget()->
      GetNodeParent(parentnode.c_str());
    }
  
  this->Internals->IsOpeningDirectory=0;
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::CreateNewFolderCallback()
{
  if (!this->DirectoryTree->GetWidget()->HasSelection())
    {
    vtkKWMessageDialog::PopupMessage(
      this->GetApplication(), this, 
      ks_("DirectoryExplorer|Title|Error!"),
      k_("Please select a directory first."), 
      vtkKWMessageDialog::ErrorIcon | 
      vtkKWMessageDialog::InvokeAtPointer);
    return;
    }
  
  vtksys_stl::string dirnode = this->GetNthSelectedNode(0);
  vtksys_stl::string parentdir = 
    this->DirectoryTree->GetWidget()->GetNodeUserData(dirnode.c_str());
  
  // Prompt the user for the name of the folder  

  vtkKWSimpleEntryDialog *dlg = vtkKWSimpleEntryDialog::New();
  dlg->SetParent(this);
  dlg->SetMasterWindow(this->GetParentTopLevel());
  dlg->SetDisplayPositionToPointer();
  dlg->SetTitle(ks_("Directory Explorer|Dialog|Title|Create new folder"));
  dlg->SetStyleToOkCancel();
  dlg->Create();
  dlg->GetEntry()->GetLabel()->SetText(
    ks_("Directory Explorer|Dialog|Folder name:"));
  dlg->SetText(
    ks_("Directory Explorer|Dialog|Enter a name for this new folder"));
  
  int ok = dlg->Invoke();
  vtksys_stl::string foldername = dlg->GetEntry()->GetWidget()->GetValue();
  dlg->Delete();
  if (ok)
    {
    if (foldername.empty() || 
        strcmp(foldername.c_str(), ".") == 0 ||
        strcmp(foldername.c_str(), "..") == 0)
      {
      vtkKWMessageDialog::PopupMessage(
        this->GetApplication(), this, 
        ks_("Directory Explorer|Title|Error!"),
        "You must enter a valid folder name!", 
        vtkKWMessageDialog::ErrorIcon | 
        vtkKWMessageDialog::InvokeAtPointer);
      return;
      }
    }
  else
    {
    return;
    }    
  
  vtkDirectory *dir = vtkDirectory::New();
  if (!dir->Open(parentdir.c_str()))
    {
    dir->Delete();
    vtkKWMessageDialog::PopupMessage(
      this->GetApplication(), this, 
      ks_("DirectoryExplorer|Title|Error!"),
      k_("The selected directory can't be opened."), 
      vtkKWMessageDialog::ErrorIcon | 
      vtkKWMessageDialog::InvokeAtPointer);
    return;
    }

  vtksys_stl::string filename, fullname;

  // Check if the folder is already created

  for (int i = 0; i < dir->GetNumberOfFiles(); i++)
    {
    filename = dir->GetFile(i);
    if (strcmp(filename.c_str(), foldername.c_str())==0)
      {
      vtkKWMessageDialog::PopupMessage(
        this->GetApplication(), this, 
        ks_("Directory Explorer|Title|Error!"),
        k_("The folder name already exists."), 
        vtkKWMessageDialog::ErrorIcon | 
        vtkKWMessageDialog::InvokeAtPointer);
      dir->Delete();
      return;
      }
    }
    
  // Add the new folder

  if (!KWFileBrowser_HasTrailingSlash(parentdir.c_str()))
    {
    parentdir += KWFileBrowser_PATH_SEPARATOR;
    }
  fullname = parentdir + foldername;
  
  if (!dir->MakeDirectory(fullname.c_str()))
    {
    vtkKWMessageDialog::PopupMessage(
      this->GetApplication(), this, 
      ks_("Directory Explorer|Title|Error!"),
      k_("The new directory can not be created."), 
      vtkKWMessageDialog::ErrorIcon | 
      vtkKWMessageDialog::InvokeAtPointer);
    dir->Delete();
    return;
    }
  
  dir->Delete();
  
  vtkIdType dirID;
  char strDirID[20];
  dirID = vtkKWDirectoryExplorer::IdCounter++;
  sprintf(strDirID, "%lu", dirID);
  vtkKWIcon *tmpIcon = vtkKWIcon::New();
  tmpIcon->SetImage(vtkKWIcon::IconFolderXP);
  this->AddDirectoryNode(dirnode.c_str(), 
                         strDirID, 
                         foldername.c_str(), 
                         fullname.c_str(), 
                         tmpIcon);
  tmpIcon->Delete();

  if (!this->DirectoryTree->GetWidget()->IsNodeOpen(dirnode.c_str()))
    {
    this->DirectoryTree->GetWidget()->OpenNode(dirnode.c_str());
    }

  this->InvokeDirectoryCreatedCommand(fullname.c_str());

  this->DirectoryTree->GetWidget()->SeeNode(strDirID);
  this->DirectoryTree->GetWidget()->ClearSelection();
  this->DirectoryTree->GetWidget()->SelectNode(strDirID);
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::BackToPreviousDirectoryCallback()
{
  if (this->Internals->MostRecentDirList.size() > 1 && 
    this->Internals->MostRecentDirCurrent != 
    this->Internals->MostRecentDirList.end())
    {
    if (strcmp((*this->Internals->MostRecentDirCurrent).c_str(), 
      this->Internals->MostRecentDirList.back().c_str())!=0)
      {
      vtksys_stl::string currentnode = 
        *(++this->Internals->MostRecentDirCurrent);
      this->OpenDirectoryNodeCallback(currentnode.c_str(), 0);
      }
    }  
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::OpenDirectoryNodeCallback(
  const char* node, 
  int offsetFromCurrent)
{
  if (node)
    {
    this->Internals->IsNavigatingNode = 1;
    vtkKWTkUtilities::SetTopLevelMouseCursor(this, "watch");
    if (strcmp(node, this->Internals->RootNode) == 0)
      {
      this->BackToRoot();
      }
    else if (this->DirectoryTree->GetWidget()->HasNode(node))
      {
      this->ReloadDirectoryNode(node);
      }
    
    // Move the list current pointer  

    if (this->Internals->MostRecentDirList.size() > 0 && 
        offsetFromCurrent !=0 &&
        strcmp((*this->Internals->MostRecentDirCurrent).c_str(), node) != 0)
      {
      if (offsetFromCurrent > 0)
        {
        for(int offset = 0; offset < offsetFromCurrent; offset++)
          {
          this->Internals->MostRecentDirCurrent++;
          }
        }
      else if (offsetFromCurrent < 0)
        {
        for(int offset=offsetFromCurrent; offset<0; offset++)
          {
          this->Internals->MostRecentDirCurrent--;
          }
        }
          
      this->Update();
      }
    vtkKWTkUtilities::SetTopLevelMouseCursor(this, NULL);
    this->Internals->IsNavigatingNode = 0;
    }
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::ForwardToNextDirectoryCallback()
{
  if (this->Internals->MostRecentDirList.size() > 1 && 
      this->Internals->MostRecentDirCurrent != 
      this->Internals->MostRecentDirList.begin())
    {
    vtksys_stl::string currentnode = 
      *(--this->Internals->MostRecentDirCurrent);
    this->OpenDirectoryNodeCallback(currentnode.c_str(), 0);
    }
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::GoUpDirectoryCallback()
{
  if (this->DirectoryTree->GetWidget()->HasSelection())
    {
    if (strcmp(this->DirectoryTree->GetWidget()->GetNodeParent(
                 this->GetNthSelectedNode(0)),
               this->Internals->RootNode) != 0)
      {
      this->ReloadDirectoryNode(this->DirectoryTree->GetWidget()->
                                GetNodeParent(this->GetNthSelectedNode(0)));
      }
    else
      {
      this->BackToRoot();
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::BackToRoot()
{
  vtksys_stl::vector<vtksys_stl::string> children;
  vtksys::SystemTools::Split(this->DirectoryTree->GetWidget()->
    GetNodeChildren(this->Internals->RootNode), children, ' ');
  vtksys_stl::vector<vtksys_stl::string>::iterator it = 
    children.begin();
  vtksys_stl::vector<vtksys_stl::string>::iterator end = 
    children.end();
  for (; it != end; it++)
    {
    this->DirectoryTree->GetWidget()->CloseTree((*it).c_str());
    }
  
   this->Update();
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::SingleClickOnNodeCallback(
  const char* node)
{
  if (!this->Internals->IsOpeningDirectory)
    {
    // This is invoked from vtkKWTree while single clicking on the tree node.
    // However, it will NOT be invoked if the Ctrl key is down in case of
    // multipleselection. So it is OK, and necessary, to clear 
    // the selection here.
    this->DirectoryTree->GetWidget()->ClearSelection();
    vtksys_stl::string node_str = node;
    this->SelectDirectoryNode(node_str.c_str());
    }
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::SelectDirectoryNode(
  const char* node, 
  int opennode)
{ 
  vtksys_stl::string node_str = node;
    
  if (!this->Internals->IsNavigatingNode)
    {
    this->OpenDirectoryNode(node_str.c_str(), 1, opennode);
    }
  
  // if the directory selection is already changed, such as
  // by MouseClick, or Up/Down arrow key, call this method again.  

  if (this->DirectoryTree->GetWidget()->HasSelection() && 
      strcmp(this->GetNthSelectedNode(0), 
             node_str.c_str())!=0)
    {
    this->SelectDirectoryNode(this->GetNthSelectedNode(0), opennode);
    }
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::DirectoryClosedCallback(
  const char* node)
{
  vtksys_stl::string selectednode = this->GetNthSelectedNode(0);

  // If the closed node is the selected node, or the selected node
  // is not one of the parent nodes of the selected node, just return

  if (selectednode.empty() || strcmp(node, selectednode.c_str()) == 0)
    {
    return;
    }

  // If the selected node is a child of the closed node, 
  // the closed node should be selected, but not opened.

  vtksys_stl::string parentnode = this->DirectoryTree->GetWidget()->
    GetNodeParent(selectednode.c_str());

  while (strcmp(parentnode.c_str(), this->Internals->RootNode) != 0)
    {
    if (strcmp(parentnode.c_str(), node) == 0)
      {
      // Set flag to suppress the SelectDirectoryNode
      this->Internals->IsNavigatingNode = 1;
      vtkKWTkUtilities::SetTopLevelMouseCursor(this, "watch");
      this->UpdateDirectoryNode(node);
      this->DirectoryTree->GetWidget()->SelectNode(node);
      this->UpdateMostRecentDirectoryHistory(node);
      this->InvokeDirectorySelectedCommand(this->GetNthSelectedDirectory(0));
      
      // update Back/Forward button state  
      this->Update();
      vtkKWTkUtilities::SetTopLevelMouseCursor(this, NULL);
      this->Internals->IsNavigatingNode = 0;
      break;
      }
    parentnode = this->DirectoryTree->GetWidget()->GetNodeParent(
      parentnode.c_str());
    }
    
  this->InvokeDirectoryClosedCommand(
    this->DirectoryTree->GetWidget()->GetNodeUserData(node));
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::DirectoryOpenedCallback(
  const char* node)
{
  if (this->Internals->IsOpeningDirectory)
    {
    return;
    }
  vtkKWTkUtilities::SetTopLevelMouseCursor(this, "watch");
  this->UpdateDirectoryNode(node);
  this->InvokeDirectoryOpenedCommand(
    this->DirectoryTree->GetWidget()->GetNodeUserData(node));
  vtkKWTkUtilities::SetTopLevelMouseCursor(this, NULL);
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::DirectorySelectedCallback()
{
  this->TreeKeyNavigationCallback();
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::RightClickExploreCallback(
  const char* node)
{
#ifdef _WIN32
  if (this->DirectoryTree->GetWidget()->HasSelection() && node && *node)
    {
    this->GetApplication()->OpenLink(
      this->DirectoryTree->GetWidget()->GetNodeUserData(node));
    }
#endif
}

//----------------------------------------------------------------------------
int vtkKWDirectoryExplorer::RenameCallback()
{
  if (this->DirectoryTree->GetWidget()->HasSelection())
    {
    vtksys_stl::string node = this->GetNthSelectedNode(0);

    // Prompt the user for confirmation

    vtksys_stl::string parentdir = 
      this->DirectoryTree->GetWidget()->GetNodeUserData(
        this->DirectoryTree->GetWidget()->GetNodeParent(node.c_str()));
    
    // Prompt the user for the name of the folder  

    vtkKWSimpleEntryDialog *dlg = vtkKWSimpleEntryDialog::New();
    dlg->SetParent(this);
    dlg->SetMasterWindow(this->GetParentTopLevel());
    dlg->SetDisplayPositionToPointer();
    dlg->SetTitle(
      ks_("Directory Explorer|Dialog|Title|Rename directory"));
    dlg->SetStyleToOkCancel();
    dlg->Create();
    dlg->GetEntry()->GetLabel()->SetText(
      ks_("Directory Explorer|Dialog|Directory name:"));
    vtksys_stl::string txtname = this->DirectoryTree->GetWidget()->
      GetNodeText(node.c_str());
    dlg->GetEntry()->GetWidget()->SetValue(txtname.c_str());
    dlg->SetText(
      ks_("Directory Explorer|Dialog|Enter a new directory name:"));
    
    int ok = dlg->Invoke();
    vtksys_stl::string newname = dlg->GetEntry()->GetWidget()->GetValue();
    dlg->Delete();
    if (ok)
      {
      if (newname.empty() || strcmp(newname.c_str(), "") == 0 ||
          strcmp(newname.c_str(), ".") == 0 ||
          strcmp(newname.c_str(), "..") == 0)
        {
        vtkKWMessageDialog::PopupMessage(
          this->GetApplication(), this, 
          ks_("Directory Explorer|Title|Error!"),
          "You must enter a valid folder name!", 
          vtkKWMessageDialog::ErrorIcon | 
          vtkKWMessageDialog::InvokeAtPointer);
        return 0;
        }
      }
    else
      {
      return 0;
      }    
    
    vtkDirectory *dir = vtkDirectory::New();
    if (!dir->Open(parentdir.c_str()))
      {
      dir->Delete();
      vtkKWMessageDialog::PopupMessage(
        this->GetApplication(), this, 
        ks_("Directory Explorer|Title|Error!"),
        k_("The directory can't be opened."), 
        vtkKWMessageDialog::ErrorIcon | 
        vtkKWMessageDialog::InvokeAtPointer);
      return 0;
      }

    vtksys_stl::string filename, fullname;

    for (int i = 0; i < dir->GetNumberOfFiles(); i++)
      {
      filename = dir->GetFile(i);
      if (strcmp(filename.c_str(), newname.c_str()) == 0)
        {
        vtkKWMessageDialog::PopupMessage(
          this->GetApplication(), this, 
          ks_("Directory Explorer|Title|Error!"),
          k_("The folder name already exists."), 
          vtkKWMessageDialog::ErrorIcon | 
          vtkKWMessageDialog::InvokeAtPointer);
        dir->Delete();
        return 0;
        }
      }
    
    dir->Delete();

    // Rename the selected file

    if (!KWFileBrowser_HasTrailingSlash(parentdir.c_str()))
      {
      parentdir += KWFileBrowser_PATH_SEPARATOR;
      }
    fullname = parentdir + newname;
    vtksys_stl::string oldfile = this->GetSelectedDirectory();
    
    if (rename(oldfile.c_str(), fullname.c_str()) == 0)
      {
      this->DirectoryTree->GetWidget()->SetNodeText(
        node.c_str(), newname.c_str());
      this->DirectoryTree->GetWidget()->SetNodeUserData(
        node.c_str(), vtksys::SystemTools::EscapeChars(
          fullname.c_str(), KWFileBrowser_ESCAPE_CHARS).c_str());
      this->Update();
      this->InvokeDirectoryRenamedCommand(oldfile.c_str(), fullname.c_str());
      return 1;
      }
    else
      {
      vtkKWMessageDialog::PopupMessage(
        this->GetApplication(), this, 
        ks_("Directory Explorer|Title|Error!"),
        "The directory name can not be changed!", 
        vtkKWMessageDialog::ErrorIcon | 
        vtkKWMessageDialog::InvokeAtPointer);
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::TreeKeyNavigationCallback()
{
  if (this->DirectoryTree->GetWidget()->HasSelection())
    {
    if (!this->Internals->IsOpeningDirectory)
      {
      this->SelectDirectoryNode(this->GetNthSelectedNode(0), 0);
      }
    } 
}

//----------------------------------------------------------------------------
int vtkKWDirectoryExplorer::RemoveSelectedNodeCallback()
{
  if (this->DirectoryTree->GetWidget()->HasSelection())
    {
    vtksys_stl::string node = this->GetNthSelectedNode(0);
    vtksys_stl::string parentnode = 
      this->DirectoryTree->GetWidget()->GetNodeParent(node.c_str());
    if (strcmp(parentnode.c_str(), this->Internals->RootNode) == 0)
      {
      vtkKWMessageDialog::PopupMessage(
        this->GetApplication(), this, 
        ks_("Directory Explorer|Title|Error!"),
        "The root directories can not be removed!", 
        vtkKWMessageDialog::ErrorIcon | 
        vtkKWMessageDialog::InvokeAtPointer);
      return 0;
      }
    
    // Prompt the user for confirmation

    if (vtkKWMessageDialog::PopupYesNo( 
          this->GetApplication(), 
          this, 
          ks_("Directory Explorer|Title|Delete directory"),
          k_("Are you sure you want to delete the selected directory?"),
          vtkKWMessageDialog::WarningIcon | 
          vtkKWMessageDialog::InvokeAtPointer))
      {
      vtksys_stl::string dir = 
        this->DirectoryTree->GetWidget()->GetNodeUserData(node.c_str());
      if (vtksys::SystemTools::RemoveADirectory(dir.c_str()))
        {
        this->RemoveDirectoryFromHistory(node.c_str());
        this->DirectoryTree->GetWidget()->DeleteNode(node.c_str());
        this->ReloadDirectoryNode(parentnode.c_str());
        this->InvokeDirectoryDeletedCommand(dir.c_str());
        return 1;
        }
      else
        {
        vtkKWMessageDialog::PopupMessage(
          this->GetApplication(), this, 
          ks_("Directory Explorer|Title|Error!"),
          "The directory can not be removed!", 
          vtkKWMessageDialog::ErrorIcon | 
          vtkKWMessageDialog::InvokeAtPointer);
        return 0;
        }
      }
    }
    
  return 0;  
}

//----------------------------------------------------------------------------
int vtkKWDirectoryExplorer::DeleteDirectory(const char* dirname)
{
  if (!dirname || !(*dirname) || !this->HasSelection())
    {
    return 0;
    }

  vtksys_stl::string parentnode = this->GetNthSelectedNode(0);

  vtksys_stl::vector<vtksys_stl::string> children;
  vtksys::SystemTools::Split(this->DirectoryTree->GetWidget()->
    GetNodeChildren(parentnode.c_str()), children, ' ');
  vtksys_stl::vector<vtksys_stl::string>::iterator it = children.begin();
  vtksys_stl::vector<vtksys_stl::string>::iterator end = children.end();

  vtksys_stl::string nodedir, nodepath;
  vtksys_stl::string dirpath = dirname;

  // Just to compare

  vtksys::SystemTools::ConvertToUnixSlashes(dirpath);
  
  for (; it != end; it++)
    {
    nodedir = this->DirectoryTree->GetWidget()->GetNodeUserData((*it).c_str());

    // Just to compare

    vtksys::SystemTools::ConvertToUnixSlashes(nodedir);
    if (vtksys::SystemTools::ComparePath(nodedir.c_str(), dirpath.c_str()))
      {
      // If the directory still exists, remove it.
      if (vtksys::SystemTools::FileExists(dirpath.c_str()))
        {
        if (!vtksys::SystemTools::RemoveADirectory(dirpath.c_str()))
          {
          vtkKWMessageDialog::PopupMessage(
            this->GetApplication(), this, 
            ks_("Directory Explorer|Title|Error!"),
            "The directory can not be removed!", 
            vtkKWMessageDialog::ErrorIcon | 
            vtkKWMessageDialog::InvokeAtPointer);
          return 0;
          }
        }

      bool updateSelection = false;
      if (this->DirectoryTree->GetWidget()->HasSelection() &&
          strcmp((*it).c_str(), this->GetNthSelectedNode(0)) == 0)
        {
        updateSelection = true;
        }
      this->RemoveDirectoryFromHistory((*it).c_str());
      this->DirectoryTree->GetWidget()->DeleteNode((*it).c_str());
      if (updateSelection)
        {
        this->ReloadDirectoryNode(parentnode.c_str());
        }
      return 1;
      }
    }
    
  return 0;
}

//----------------------------------------------------------------------------
int vtkKWDirectoryExplorer::RenameDirectory(
      const char* oldname,
      const char* newname)
{
  if (!this->HasSelection())
    {
    return 0;
    }

  vtksys_stl::string parentnode = 
    this->DirectoryTree->GetWidget()->GetSelection();  
  vtksys_stl::vector<vtksys_stl::string> children;
  vtksys::SystemTools::Split(this->DirectoryTree->GetWidget()->
    GetNodeChildren(parentnode.c_str()), children, ' ');
  vtksys_stl::vector<vtksys_stl::string>::iterator it = children.begin();
  vtksys_stl::vector<vtksys_stl::string>::iterator end = children.end();

  vtksys_stl::string nodedir, nodepath;
  vtksys_stl::string dirpath = oldname;

  // Just to compare

  vtksys::SystemTools::ConvertToUnixSlashes(dirpath);
  
  for (; it != end; it++)
    {
    nodedir = this->DirectoryTree->GetWidget()->GetNodeUserData((*it).c_str());

    // Compare

    vtksys::SystemTools::ConvertToUnixSlashes(nodedir);
    if (vtksys::SystemTools::ComparePath(nodedir.c_str(), dirpath.c_str()))
      {
      this->DirectoryTree->GetWidget()->SetNodeText(
        (*it).c_str(), vtksys::SystemTools::GetFilenameName(newname).c_str());
      this->DirectoryTree->GetWidget()->SetNodeUserData(
        (*it).c_str(), vtksys::SystemTools::EscapeChars(
          newname, KWFileBrowser_ESCAPE_CHARS).c_str());
      this->Update();
      return 1;
      }
    }
    
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::SetDirectorySelectedCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->DirectorySelectedCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::InvokeDirectorySelectedCommand(
  const char* directory)
{
  vtksys_stl::string dirpath = directory;
  if (this->DirectorySelectedCommand && *this->DirectorySelectedCommand)
    {
    this->Script("%s \"%s\"", this->DirectorySelectedCommand, 
                 vtksys::SystemTools::EscapeChars(
                   KWFileBrowser_GetUnixPath(dirpath.c_str()), 
                   KWFileBrowser_ESCAPE_CHARS).c_str());
    }
  this->InvokeEvent(vtkKWDirectoryExplorer::DirectorySelectedEvent, 
                    (void*)dirpath.c_str());
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::SetDirectoryCreatedCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->DirectoryCreatedCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::InvokeDirectoryCreatedCommand(
  const char* path)
{
  if (this->DirectoryCreatedCommand && *this->DirectoryCreatedCommand && 
      path && *path && vtksys::SystemTools::FileIsDirectory(path))
    {
    this->Script("%s \"%s\"", this->DirectoryCreatedCommand, 
                 vtksys::SystemTools::EscapeChars(
                   KWFileBrowser_GetUnixPath(path), 
                   KWFileBrowser_ESCAPE_CHARS).c_str());
    }
  this->InvokeEvent(vtkKWDirectoryExplorer::DirectoryCreatedEvent, (void*)path);
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::SetDirectoryDeletedCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->DirectoryDeletedCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::InvokeDirectoryDeletedCommand(
  const char* path)
{
  vtksys_stl::string fullpath;
  if (path && *path)
    {
    fullpath = path;
    }
  else
    {
    return;
    }

  if (this->DirectoryDeletedCommand && *this->DirectoryDeletedCommand)
    {
    this->Script("%s \"%s\"", this->DirectoryDeletedCommand, 
                 vtksys::SystemTools::EscapeChars(
                   KWFileBrowser_GetUnixPath(fullpath.c_str()), 
                   KWFileBrowser_ESCAPE_CHARS).c_str());
    }
  
  this->InvokeEvent(
    vtkKWDirectoryExplorer::DirectoryDeletedEvent, (void*)fullpath.c_str());
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::SetDirectoryClosedCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->DirectoryClosedCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::InvokeDirectoryClosedCommand(
  const char* path)
{
  if (this->DirectoryClosedCommand && *this->DirectoryClosedCommand && 
      path && *path && vtksys::SystemTools::FileIsDirectory(path))
    {
    this->Script("%s \"%s\"", this->DirectoryClosedCommand, 
                 vtksys::SystemTools::EscapeChars(
                   KWFileBrowser_GetUnixPath(path), 
                   KWFileBrowser_ESCAPE_CHARS).c_str());
    }
  this->InvokeEvent(vtkKWDirectoryExplorer::DirectoryClosedEvent, (void*)path);
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::SetDirectoryOpenedCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->DirectoryOpenedCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::InvokeDirectoryOpenedCommand(
  const char* path)
{
  if (this->DirectoryOpenedCommand && *this->DirectoryOpenedCommand && 
      path && *path && vtksys::SystemTools::FileIsDirectory(path))
    {
    this->Script("%s \"%s\"", this->DirectoryOpenedCommand, 
                 vtksys::SystemTools::EscapeChars(
                   KWFileBrowser_GetUnixPath(path), 
                   KWFileBrowser_ESCAPE_CHARS).c_str());
    }
  this->InvokeEvent(vtkKWDirectoryExplorer::DirectoryOpenedEvent, (void*)path);
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::SetDirectoryRenamedCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->DirectoryRenamedCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::InvokeDirectoryRenamedCommand(
  const char* oldname,
  const char* newname)
{
  if (this->DirectoryRenamedCommand && *this->DirectoryRenamedCommand)
    {
    this->Script("%s \"%s\" \"%s\"", 
                 this->DirectoryRenamedCommand, 
                 vtksys::SystemTools::EscapeChars(
                   KWFileBrowser_GetUnixPath(oldname), 
                   KWFileBrowser_ESCAPE_CHARS).c_str(), 
                 vtksys::SystemTools::EscapeChars(
                   KWFileBrowser_GetUnixPath(newname), 
                   KWFileBrowser_ESCAPE_CHARS).c_str());
    }
}

//---------------------------------------------------------------------------
void vtkKWDirectoryExplorer::RightClickCallback(
  int x, int y, const char* node)
{
  if (!node || !(*node) || !this->IsCreated())
    {
    return;
    }
    
  if (!this->DirectoryTree->GetWidget()->HasSelection())
    {
    this->DirectoryTree->GetWidget()->SelectNode(node);
    }
  else
    {
    if (strcmp(this->GetNthSelectedNode(0), node) != 0)
      {
      this->DirectoryTree->GetWidget()->ClearSelection();
      this->DirectoryTree->GetWidget()->SelectNode(node);
      }
    }
  
  if (!this->DirectoryTree->GetWidget()->HasSelection())
    {
    return;
    }

  if (!this->ContextMenu)
    {
    this->ContextMenu = vtkKWMenu::New();
    }
  if (!this->ContextMenu->IsCreated())
    {
    this->ContextMenu->SetParent(this->DirectoryTree->GetWidget());
    this->ContextMenu->Create();
    }
  this->ContextMenu->DeleteAllItems();
  this->PopulateContextMenu(
    this->ContextMenu, this->GetNthSelectedNode(0));
  if (this->ContextMenu->GetNumberOfItems())
    {
    this->ContextMenu->PopUp(x, y);
    }
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::PopulateContextMenu(
  vtkKWMenu *menu,
  const char* node)
{
  char command[256];
  vtksys_stl::string dirNode = node;

  // create new folder
  sprintf(command, "CreateNewFolderCallback");
  menu->AddCommand("Create new folder", this, command);

#ifdef _WIN32
  // Explore to open native explorer file
  sprintf(command, "RightClickExploreCallback %s", dirNode.c_str());
  menu->AddCommand("Explore", this, command);
#endif

  // Rename file
  int index = menu->AddCommand("Rename", this, "RenameCallback");
  menu->SetItemAccelerator(index, "F2");
  menu->SetBindingForItemAccelerator(index, menu->GetParent());

  // Delete file
  menu->AddCommand("Delete", this, "RemoveSelectedNodeCallback");
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->PropagateEnableState(this->DirectoryTree);
  this->PropagateEnableState(this->Toolbar);
  this->PropagateEnableState(this->BackButton);
  this->PropagateEnableState(this->ForwardButton);
  this->PropagateEnableState(this->UpButton);
  this->PropagateEnableState(this->CreateFolderButton);
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::PruneMostRecentDirectoriesInHistory()
{
  bool bCurrentChanged = false;
  bool bUpdate = false;

  // Update the most recent directory pointer and the history list

  while(this->Internals->MostRecentDirList.size() > 
        (size_t)this->MaximumNumberOfDirectoriesInHistory)
    {
    // check if MostRecentDirCurrent is being removed, if yes
    // we need to reset it to the first item in the list after pruning.
    if (!bCurrentChanged &&
        strcmp((*this->Internals->MostRecentDirCurrent).c_str(), 
               this->Internals->MostRecentDirList.back().c_str())==0)
      {
      bCurrentChanged = true;
      }
    this->Internals->MostRecentDirList.pop_back();
    bUpdate = true;
    }
  
  if (bCurrentChanged)
    {
    this->Internals->MostRecentDirCurrent = 
      this->Internals->MostRecentDirList.begin();
    }
  if (bUpdate)
    {
    this->Update();
    }  
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::SetMaximumNumberOfDirectoriesInHistory(
  int maxnum)
{
  if (maxnum < 1)
    {
    maxnum = 1;
    }
  else if (maxnum > 100)
    {
    maxnum = 100;
    }
  if (this->MaximumNumberOfDirectoriesInHistory == maxnum)
    {
    return;
    }
  
  this->MaximumNumberOfDirectoriesInHistory = maxnum;
  this->Modified();

  this->PruneMostRecentDirectoriesInHistory();
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::AddBindingToInternalWidget(const char* kwevent,
    vtkObject *obj, const char* method)
{
  this->DirectoryTree->GetWidget()->AddBinding(kwevent, obj, method);
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::SetTreeWidth(int width)
{
  if (this->DirectoryTree)
    {
    this->DirectoryTree->GetWidget()->SetWidth(width);
    }
}

//----------------------------------------------------------------------------
int vtkKWDirectoryExplorer::GetTreeWidth()
{
  if (this->DirectoryTree)
    {
    return this->DirectoryTree->GetWidget()->GetWidth();
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::SetTreeHeight(int height)
{
  if (this->DirectoryTree)
    {
    this->DirectoryTree->GetWidget()->SetHeight(height);
    }
}

//----------------------------------------------------------------------------
int vtkKWDirectoryExplorer::GetTreeHeight()
{
  if (this->DirectoryTree)
    {
    return this->DirectoryTree->GetWidget()->GetHeight();
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWDirectoryExplorer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "MaximumNumberOfDirectoriesInHistory: " 
     << this->GetMaximumNumberOfDirectoriesInHistory() << endl;
}
