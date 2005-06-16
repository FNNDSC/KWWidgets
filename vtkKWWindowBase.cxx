/*=========================================================================

  Module:    $RCSfile: vtkKWWindowBase.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWWindowBase.h"

#include "vtkKWApplication.h"
#include "vtkKWEvent.h"
#include "vtkKWFrame.h"
#include "vtkKWIcon.h"
#include "vtkKWLabel.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWMenu.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWMostRecentFilesManager.h"
#include "vtkKWProgressGauge.h"
#include "vtkKWTclInteractor.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWToolbar.h"
#include "vtkKWToolbarSet.h"
#include "vtkObjectFactory.h"

#include <vtksys/SystemTools.hxx>

const char *vtkKWWindowBase::PrintOptionsMenuLabel = "Print options...";
const char *vtkKWWindowBase::FileMenuLabel = "File";
const char *vtkKWWindowBase::FileCloseMenuLabel = "Close";
const char *vtkKWWindowBase::FileExitMenuLabel = "Exit";
const char *vtkKWWindowBase::EditMenuLabel = "Edit";
const char *vtkKWWindowBase::ViewMenuLabel = "View";
const char *vtkKWWindowBase::WindowMenuLabel = "Window";
const char *vtkKWWindowBase::HelpMenuLabel = "Help";
const char *vtkKWWindowBase::HelpTopicsMenuLabel = "Help Topics";
const char *vtkKWWindowBase::HelpCheckForUpdatesMenuLabel = "Check for Updates";
const char *vtkKWWindowBase::ToolbarsVisibilityMenuLabel = "Toolbars";

const char *vtkKWWindowBase::OpenRecentFileMenuLabel = "Open Recent File";
const char *vtkKWWindowBase::WindowGeometryRegKey = "WindowGeometry";

const char *vtkKWWindowBase::DefaultGeometry = "900x700+0+0";

vtkCxxRevisionMacro(vtkKWWindowBase, "$Revision: 1.16 $");

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWWindowBase );

int vtkKWWindowBaseCommand(ClientData cd, Tcl_Interp *interp,
                           int argc, char *argv[]);

//----------------------------------------------------------------------------
vtkKWWindowBase::vtkKWWindowBase()
{
  // Menus

  this->FileMenu              = NULL;
  this->HelpMenu              = NULL;
  this->EditMenu              = NULL;
  this->ViewMenu              = NULL;
  this->WindowMenu            = NULL;

  // Toolbars

  this->MainToolbarSet               = vtkKWToolbarSet::New();
  this->ToolbarsVisibilityMenu = NULL; 
  this->MenuBarSeparatorFrame  = vtkKWFrame::New();

  // Main Frame

  this->MainFrame              = vtkKWFrame::New();

  // Status frame

  this->StatusFrameSeparator  = vtkKWFrame::New();
  this->StatusFrame           = vtkKWFrame::New();
  this->StatusLabel           = vtkKWLabel::New();
  this->StatusImage           = NULL;

  this->ProgressFrame         = vtkKWFrame::New();
  this->ProgressGauge         = vtkKWProgressGauge::New();

  this->TrayFrame             = vtkKWFrame::New();
  this->TrayImageError        = vtkKWLabel::New();

  this->TclInteractor         = NULL;

  this->CommandFunction       = vtkKWWindowBaseCommand;

  this->SupportHelp           = 0;
  this->SupportPrint          = 0;
  this->PromptBeforeClose     = 0;

  this->MostRecentFilesManager = vtkKWMostRecentFilesManager::New();

  this->SetWindowClass("KitwareWidget");

  this->ScriptExtension       = NULL;
  this->SetScriptExtension(".tcl");

  this->ScriptType            = NULL;
  this->SetScriptType("Tcl");
}

//----------------------------------------------------------------------------
vtkKWWindowBase::~vtkKWWindowBase()
{
  this->PrepareForDelete();

  if (this->TclInteractor)
    {
    this->TclInteractor->Delete();
    this->TclInteractor = NULL;
    }

  if (this->FileMenu)
    {
    this->FileMenu->Delete();
    this->FileMenu = NULL;
    }

  if (this->HelpMenu)
    {
    this->HelpMenu->Delete();
    this->HelpMenu = NULL;
    }

  if (this->MainToolbarSet)
    {
    this->MainToolbarSet->Delete();
    this->MainToolbarSet = NULL;
    }

  if (this->MenuBarSeparatorFrame)
    {
    this->MenuBarSeparatorFrame->Delete();
    this->MenuBarSeparatorFrame = NULL;
    }

  if (this->MainFrame)
    {
    this->MainFrame->Delete();
    this->MainFrame = NULL;
    }

  if (this->StatusFrameSeparator)
    {
    this->StatusFrameSeparator->Delete();
    this->StatusFrameSeparator = NULL;
    }

  if (this->StatusFrame)
    {
    this->StatusFrame->Delete();
    this->StatusFrame = NULL;
    }

  if (this->StatusImage)
    {
    this->StatusImage->Delete();
    this->StatusImage = NULL;
    }

  if (this->StatusLabel)
    {
    this->StatusLabel->Delete();
    this->StatusLabel = NULL;
    }

  if (this->ProgressFrame)
    {
    this->ProgressFrame->Delete();
    this->ProgressFrame = NULL;
    }

  if (this->ProgressGauge)
    {
    this->ProgressGauge->Delete();
    this->ProgressGauge = NULL;
    }

  if (this->TrayFrame)
    {
    this->TrayFrame->Delete();
    this->TrayFrame = NULL;
    }

  if (this->TrayImageError)
    {
    this->TrayImageError->Delete();
    this->TrayImageError = NULL;
    }

  if (this->EditMenu)
    {
    this->EditMenu->Delete();
    this->EditMenu = NULL;
    }

  if (this->ViewMenu)
    {
    this->ViewMenu->Delete();
    this->ViewMenu = NULL;
    }

  if (this->WindowMenu)
    {
    this->WindowMenu->Delete();
    this->WindowMenu = NULL;
    }

  if (this->ToolbarsVisibilityMenu)
    {
    this->ToolbarsVisibilityMenu->Delete();
    this->ToolbarsVisibilityMenu = NULL;
    }

  if (this->MostRecentFilesManager)
    {
    this->MostRecentFilesManager->Delete();
    this->MostRecentFilesManager = NULL;
    }

  this->SetScriptExtension(NULL);
  this->SetScriptType(NULL);
}

//----------------------------------------------------------------------------
void vtkKWWindowBase::PrepareForDelete()
{
  // Have reference to this object:
  // this->Menu
  // this->MenuBarSeparatorFrame
  // this->MainToolbarSet
  // this->MainFrame
  // this->StatusFrameSeparator
  // this->StatusFrame

  if (this->TclInteractor )
    {
    this->TclInteractor->SetMasterWindow(NULL);
    this->TclInteractor->Delete();
    this->TclInteractor = NULL;
    }

  if (this->MainToolbarSet)
    {
    this->MainToolbarSet->RemoveAllToolbars();
    }
}

//----------------------------------------------------------------------------
void vtkKWWindowBase::Create(vtkKWApplication *app)
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro("class already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::Create(app);

  vtksys_stl::string cmd;
  vtksys_stl::string label;
  vtkKWMenu *menu = NULL;

  this->SetIconName(app->GetPrettyName());

  // Menu : File

  menu = this->GetFileMenu();

  if (this->SupportPrint)
    {
    menu->AddCommand(
      vtkKWWindowBase::PrintOptionsMenuLabel, this, "PrintOptionsCallback", 4);
    menu->AddSeparator();
    }

  menu->AddCommand(
    vtkKWWindowBase::FileCloseMenuLabel, this, "Close", 0);
  menu->AddCommand(
    vtkKWWindowBase::FileExitMenuLabel, this->GetApplication(), "Exit", 1);

  this->MostRecentFilesManager->SetApplication(app);

  // Menu : Help

  menu = this->GetHelpMenu();

  if (this->SupportHelp)
    {
    cmd = "DisplayHelpDialog ";
    cmd += this->GetTclName();
    menu->AddCommand(
      vtkKWWindowBase::HelpTopicsMenuLabel, app, cmd.c_str(), 0);
    }

  if (app->HasCheckForUpdates())
    {
    menu->AddCommand(
      vtkKWWindowBase::HelpCheckForUpdatesMenuLabel,app,"CheckForUpdates",0);
    }
  
  menu->AddSeparator();
  label = "About ";
  label += app->GetPrettyName();
  cmd = "DisplayAboutDialog ";
  cmd += this->GetTclName();
  menu->AddCommand(label.c_str(), this->GetApplication(), cmd.c_str(), 0);

  // Menubar separator

  this->MenuBarSeparatorFrame->SetParent(this);  
  this->MenuBarSeparatorFrame->Create(app);
  this->MenuBarSeparatorFrame->SetHeight(2);
  this->MenuBarSeparatorFrame->SetBorderWidth(2);
#if defined(_WIN32)
  this->MenuBarSeparatorFrame->SetReliefToGroove();
#else
  this->MenuBarSeparatorFrame->SetReliefToSunken();
#endif

  this->Script("pack %s -side top -fill x -pady 2",
               this->MenuBarSeparatorFrame->GetWidgetName());

  // Toolbars

  this->MainToolbarSet->SetParent(this);  
  this->MainToolbarSet->Create(app);
  this->MainToolbarSet->ShowTopSeparatorOff();
  this->MainToolbarSet->ShowBottomSeparatorOn();
  this->MainToolbarSet->SynchronizeToolbarsVisibilityWithRegistryOn();
  this->MainToolbarSet->SetToolbarVisibilityChangedCommand(
    this, "ToolbarVisibilityChangedCallback");
  this->MainToolbarSet->SetNumberOfToolbarsChangedCommand(
    this, "NumberOfToolbarsChangedCallback");

  this->Script(
    "pack %s -padx 0 -pady 0 -side top -fill x -expand no -after %s",
    this->MainToolbarSet->GetWidgetName(),
    this->MenuBarSeparatorFrame->GetWidgetName());

  // Main frame

  this->MainFrame->SetParent(this);
  this->MainFrame->Create(app);

  this->Script("pack %s -side top -fill both -expand t",
               this->MainFrame->GetWidgetName());

  // Restore Window Geometry

  if (app->GetSaveUserInterfaceGeometry())
    {
    this->RestoreWindowGeometryFromRegistry();
    }
  else
    {
    this->SetGeometry(vtkKWWindowBase::DefaultGeometry);
    }

  // Status frame

  this->StatusFrame->SetParent(this);
  this->StatusFrame->Create(app);
  
  this->Script("pack %s -side bottom -fill x -pady 0",
               this->StatusFrame->GetWidgetName());

  // Status frame separator

  this->StatusFrameSeparator->SetParent(this);
  this->StatusFrameSeparator->Create(app);
  this->StatusFrameSeparator->SetHeight(2);
  this->StatusFrameSeparator->SetBorderWidth(2);
#if defined(_WIN32)
  this->StatusFrameSeparator->SetReliefToGroove();
#else
  this->StatusFrameSeparator->SetReliefToSunken();
#endif

  this->Script("pack %s -side bottom -fill x -pady 2",
               this->StatusFrameSeparator->GetWidgetName());
  
  // Status frame : image

  this->UpdateStatusImage();

  // Status frame : label

  this->StatusLabel->SetParent(this->StatusFrame);
  this->StatusLabel->Create(app);
  this->StatusLabel->SetReliefToSunken();
  this->StatusLabel->SetBorderWidth(0);
  this->StatusLabel->SetPadX(3);
  this->StatusLabel->SetAnchorToWest();

  this->Script("pack %s -side left -padx 1 -expand yes -fill both",
               this->StatusLabel->GetWidgetName());

  // Status frame : progress frame

  this->ProgressFrame->SetParent(this->StatusFrame);
  this->ProgressFrame->Create(app);
  this->ProgressFrame->SetBorderWidth(1);
  this->ProgressFrame->SetReliefToSunken();

  this->Script("pack %s -side left -padx 0 -fill y", 
               this->ProgressFrame->GetWidgetName());

  // Status frame : progress frame : gauge
  // Set the height to 0 so that it auto-expands to the right height

  this->ProgressGauge->SetParent(this->ProgressFrame);
  this->ProgressGauge->SetWidth(200);
  this->ProgressGauge->ExpandHeightOn();
  this->ProgressGauge->Create(app);

  this->Script("pack %s -side right -padx 2 -pady 2 -expand y -fill y",
               this->ProgressGauge->GetWidgetName());

  // Status frame : tray frame

  this->TrayFrame->SetParent(this->StatusFrame);
  this->TrayFrame->Create(app);
  this->TrayFrame->SetBorderWidth(1);
  this->TrayFrame->SetReliefToSunken();

  this->Script(
    "pack %s -side left -ipadx 0 -ipady 0 -padx 0 -pady 0 -fill both",
    this->TrayFrame->GetWidgetName());

  // Status frame : tray frame : error image

  this->TrayImageError->SetParent(this->TrayFrame);
  this->TrayImageError->Create(app);

  this->TrayImageError->SetImageOption(vtkKWIcon::ICON_SMALLERRORRED);
  
  this->TrayImageError->SetBind(this, "<Button-1>", "ErrorIconCallback");

  // Udpate the enable state

  this->UpdateEnableState();
}

//----------------------------------------------------------------------------
vtkKWFrame* vtkKWWindowBase::GetViewFrame()
{
  return this->MainFrame;
}

//----------------------------------------------------------------------------
int vtkKWWindowBase::DisplayCloseDialog()
{
  vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
  dialog->SetStyleToYesNo();
  dialog->SetMasterWindow(this);
  dialog->SetOptions(
    vtkKWMessageDialog::QuestionIcon | 
    vtkKWMessageDialog::Beep | 
    vtkKWMessageDialog::YesDefault);
  dialog->Create(this->GetApplication());
  dialog->SetText("Are you sure you want to close this window?");
  dialog->SetTitle(vtkKWWindowBase::FileCloseMenuLabel);
  int ret = dialog->Invoke();
  dialog->Delete();
  return ret;
}

//----------------------------------------------------------------------------
int vtkKWWindowBase::Close()
{
  // If a dialog is still up, complain and bail
  // This should be fixed, since we don't know here if the dialog that is
  // up is something that was created by this instance, and not by another
  // window instance (in the later case, it would be safe to close)

  if (this->GetApplication()->GetDialogUp())
    {
    this->Script("bell");
    return 0;
    }

  // Prompt confirmation if needed

  if (this->PromptBeforeClose && !this->DisplayCloseDialog())
    {
    return 0;
    }

  // Save its geometry

  if (this->GetApplication()->GetSaveUserInterfaceGeometry())
    {
    this->SaveWindowGeometryToRegistry();
    }

  // Remove this window from the application. 
  // It is likely that the application will exit if there are no more windows.

  return this->GetApplication()->RemoveWindow(this);
}

//----------------------------------------------------------------------------
void vtkKWWindowBase::SaveWindowGeometryToRegistry()
{
  if (!this->IsCreated())
    {
    return;
    }

  vtksys_stl::string geometry = this->GetGeometry();
  this->GetApplication()->SetRegistryValue(
    2, "Geometry", vtkKWWindowBase::WindowGeometryRegKey, "%s", geometry.c_str());
}

//----------------------------------------------------------------------------
void vtkKWWindowBase::RestoreWindowGeometryFromRegistry()
{
  if (!this->IsCreated())
    {
    return;
    }

  if (this->GetApplication()->HasRegistryValue(
        2, "Geometry", vtkKWWindowBase::WindowGeometryRegKey))
    {
    char geometry[40];
    if (this->GetApplication()->GetRegistryValue(
          2, "Geometry", vtkKWWindowBase::WindowGeometryRegKey, geometry))
      {
      this->SetGeometry(geometry);
      }
    }
  else
    {
    this->SetGeometry(vtkKWWindowBase::DefaultGeometry);
    }
}

//----------------------------------------------------------------------------
void vtkKWWindowBase::SetStatusText(const char *text)
{
  this->StatusLabel->SetText(text);
}

//----------------------------------------------------------------------------
const char *vtkKWWindowBase::GetStatusText()
{
  return this->StatusLabel->GetText();
}

//----------------------------------------------------------------------------
vtkKWMenu *vtkKWWindowBase::GetFileMenu()
{
  if (!this->FileMenu)
    {
    this->FileMenu = vtkKWMenu::New();
    }

  if (!this->FileMenu->IsCreated() && this->GetMenu() && this->IsCreated())
    {
    this->FileMenu->SetParent(this->GetMenu());
    this->FileMenu->SetTearOff(0);
    this->FileMenu->Create(this->GetApplication());
    this->GetMenu()->InsertCascade(
      0, vtkKWWindowBase::FileMenuLabel, this->FileMenu, 0);
    }
  
  return this->FileMenu;
}

//----------------------------------------------------------------------------
int vtkKWWindowBase::GetFileMenuInsertPosition()
{
  if (!this->IsCreated())
    {
    return 0;
    }

  // First find the print-related menu commands

  if (this->GetFileMenu()->HasItem(vtkKWWindowBase::PrintOptionsMenuLabel))
    {
    return this->GetFileMenu()->GetIndex(
      vtkKWWindowBase::PrintOptionsMenuLabel);
    }

  // Otherwise find Close or Exit if Close was removed

  if (this->GetFileMenu()->HasItem(vtkKWWindowBase::FileCloseMenuLabel))
    {
    return this->GetFileMenu()->GetIndex(
      vtkKWWindowBase::FileCloseMenuLabel);  
    }

  if (this->GetFileMenu()->HasItem(vtkKWWindowBase::FileExitMenuLabel))
    {
    return this->GetFileMenu()->GetIndex(vtkKWWindowBase::FileExitMenuLabel);  
    }

  return this->GetHelpMenu()->GetNumberOfItems();
}

//----------------------------------------------------------------------------
vtkKWMenu *vtkKWWindowBase::GetEditMenu()
{
  if (!this->EditMenu)
    {
    this->EditMenu = vtkKWMenu::New();
    }

  if (!this->EditMenu->IsCreated() && this->GetMenu() && this->IsCreated())
    {
    this->EditMenu->SetParent(this->GetMenu());
    this->EditMenu->SetTearOff(0);
    this->EditMenu->Create(this->GetApplication());
    // Usually after the File Menu (i.e., pos 1)
    this->GetMenu()->InsertCascade(
      1, vtkKWWindowBase::EditMenuLabel, this->EditMenu, 0);
    }
  
  return this->EditMenu;
}

//----------------------------------------------------------------------------
vtkKWMenu *vtkKWWindowBase::GetViewMenu()
{
  if (!this->ViewMenu)
    {
    this->ViewMenu = vtkKWMenu::New();
    }

  if (!this->ViewMenu->IsCreated() && this->GetMenu() && this->IsCreated())
    {
    this->ViewMenu->SetParent(this->GetMenu());
    this->ViewMenu->SetTearOff(0);
    this->ViewMenu->Create(this->GetApplication());
    // Usually after the Edit Menu (do not use GetEditMenu() here)
    this->GetMenu()->InsertCascade(
      1 + (this->EditMenu ? 1 : 0), 
      vtkKWWindowBase::ViewMenuLabel, this->ViewMenu, 0);
    }

  return this->ViewMenu;
}

//----------------------------------------------------------------------------
int vtkKWWindowBase::GetViewMenuInsertPosition()
{
  return 0;
}

//----------------------------------------------------------------------------
vtkKWMenu *vtkKWWindowBase::GetWindowMenu()
{
  if (!this->WindowMenu)
    {
    this->WindowMenu = vtkKWMenu::New();
    }

  if (!this->WindowMenu->IsCreated() && this->GetMenu() && this->IsCreated())
    {
    this->WindowMenu->SetParent(this->GetMenu());
    this->WindowMenu->SetTearOff(0);
    this->WindowMenu->Create(this->GetApplication());
    // Usually after View Menu (do not use GetEditMenu()/GetViewMenu() here)
    this->GetMenu()->InsertCascade(
      1 + (this->EditMenu ? 1 : 0) + (this->ViewMenu ? 1 : 0), 
      vtkKWWindowBase::WindowMenuLabel, this->WindowMenu, 0);
    }
  
  return this->WindowMenu;
}

//----------------------------------------------------------------------------
vtkKWMenu *vtkKWWindowBase::GetHelpMenu()
{
  if (!this->HelpMenu)
    {
    this->HelpMenu = vtkKWMenu::New();
    }

  if (!this->HelpMenu->IsCreated() && this->GetMenu() && this->IsCreated())
    {
    this->HelpMenu->SetParent(this->GetMenu());
    this->HelpMenu->SetTearOff(0);
    this->HelpMenu->Create(this->GetApplication());
    // Usually at the end
    this->GetMenu()->AddCascade(
      vtkKWWindowBase::HelpMenuLabel, this->HelpMenu, 0);
    }
  
  return this->HelpMenu;
}

//----------------------------------------------------------------------------
int vtkKWWindowBase::GetHelpMenuInsertPosition()
{
  if (!this->IsCreated())
    {
    return 0;
    }

  // Find about

  if (this->GetHelpMenu()->HasItem("About*"))
    {
    return this->GetHelpMenu()->GetIndex("About*") - 1;
    }

  return this->GetHelpMenu()->GetNumberOfItems();
}

//----------------------------------------------------------------------------
vtkKWMenu *vtkKWWindowBase::GetToolbarsVisibilityMenu()
{
  if (!this->ToolbarsVisibilityMenu)
    {
    this->ToolbarsVisibilityMenu = vtkKWMenu::New();
    }

  if (!this->ToolbarsVisibilityMenu->IsCreated() && 
      this->GetWindowMenu() &&
      this->IsCreated())
    {
    this->ToolbarsVisibilityMenu->SetParent(this->GetWindowMenu());
    this->ToolbarsVisibilityMenu->SetTearOff(0);
    this->ToolbarsVisibilityMenu->Create(this->GetApplication());
    this->GetWindowMenu()->InsertCascade(
      2, vtkKWWindowBase::ToolbarsVisibilityMenuLabel, 
      this->ToolbarsVisibilityMenu, 1, "Set Toolbars Visibility");
    }
  
  return this->ToolbarsVisibilityMenu;
}

//----------------------------------------------------------------------------
void vtkKWWindowBase::InsertRecentFilesMenu(
  int pos, vtkKWObject *target)
{
  if (!this->IsCreated() || !this->MostRecentFilesManager)
    {
    return;
    }

  // Create the sub-menu if not done already

  vtkKWMenu *mrf_menu = this->MostRecentFilesManager->GetMenu();
  if (!mrf_menu->IsCreated())
    {
    mrf_menu->SetParent(this->GetFileMenu());
    mrf_menu->SetTearOff(0);
    mrf_menu->Create(this->GetApplication());
    }

  // Remove the menu if already there (in case that function was used to
  // move the menu)

  if (this->GetFileMenu()->HasItem(vtkKWWindowBase::OpenRecentFileMenuLabel))
    {
    this->GetFileMenu()->DeleteMenuItem(
      vtkKWWindowBase::OpenRecentFileMenuLabel);
    }

  this->GetFileMenu()->InsertCascade(
    pos, vtkKWWindowBase::OpenRecentFileMenuLabel, mrf_menu, 6);

  // Fill the recent files vector with recent files stored in registry
  // this will also update the menu

  this->MostRecentFilesManager->SetDefaultTargetObject(target);
  this->MostRecentFilesManager->RestoreFilesListFromRegistry();
}

//----------------------------------------------------------------------------
void vtkKWWindowBase::AddRecentFile(const char *name, 
                                    vtkKWObject *target,
                                    const char *command)
{  
  if (this->MostRecentFilesManager)
    {
    this->MostRecentFilesManager->AddFile(name, target, command);
    this->MostRecentFilesManager->SaveFilesToRegistry();
    }
}

//----------------------------------------------------------------------------
void vtkKWWindowBase::NumberOfToolbarsChangedCallback()
{
  this->MainToolbarSet->PopulateToolbarsVisibilityMenu(
    this->GetToolbarsVisibilityMenu());

  this->UpdateToolbarState();
}

//----------------------------------------------------------------------------
void vtkKWWindowBase::ToolbarVisibilityChangedCallback()
{
  this->MainToolbarSet->UpdateToolbarsVisibilityMenu(
    this->GetToolbarsVisibilityMenu());

  this->UpdateToolbarState();
}

//----------------------------------------------------------------------------
void vtkKWWindowBase::LoadScript()
{
  vtkKWLoadSaveDialog* load_dialog = vtkKWLoadSaveDialog::New();
  this->GetApplication()->RetrieveDialogLastPathRegistryValue(
    load_dialog, "LoadScriptLastPath");
  load_dialog->SetParent(this);
  load_dialog->Create(this->GetApplication());
  load_dialog->SaveDialogOff();
  load_dialog->SetTitle("Load Script");
  load_dialog->SetDefaultExtension(this->ScriptExtension);

  vtksys_stl::string filetypes;
  filetypes += "{{";
  filetypes += this->ScriptType;
  filetypes += " Scripts} {";
  filetypes += this->ScriptExtension;
  filetypes += "}} {{All Files} {.*}}";
  load_dialog->SetFileTypes(filetypes.c_str());

  int enabled = this->GetEnabled();
  this->SetEnabled(0);

  if (load_dialog->Invoke() && 
      load_dialog->GetFileName() && 
      strlen(load_dialog->GetFileName()) > 0)
    {
    if (!vtksys::SystemTools::FileExists(load_dialog->GetFileName()))
      {
      vtkWarningMacro("Unable to open script file!");
      }
    else
      {
      this->GetApplication()->SaveDialogLastPathRegistryValue(
        load_dialog, "LoadScriptLastPath");
      this->LoadScript(load_dialog->GetFileName());
      }
    }

  this->SetEnabled(enabled);
  load_dialog->Delete();
}

//----------------------------------------------------------------------------
void vtkKWWindowBase::LoadScript(const char *filename)
{
  this->GetApplication()->LoadScript(filename);
}

//----------------------------------------------------------------------------
void vtkKWWindowBase::WarningMessage(const char* message)
{
  vtkKWMessageDialog::PopupMessage(
    this->GetApplication(), this, "Warning",
    message, vtkKWMessageDialog::WarningIcon);
  this->SetErrorIconToRed();
  this->InvokeEvent(vtkKWEvent::WarningMessageEvent, (void*)message);
}

//----------------------------------------------------------------------------
void vtkKWWindowBase::ErrorMessage(const char* message)
{
  vtkKWMessageDialog::PopupMessage(
    this->GetApplication(), this, "Error",
    message, vtkKWMessageDialog::ErrorIcon);
  this->SetErrorIconToRed();
  this->InvokeEvent(vtkKWEvent::ErrorMessageEvent, (void*)message);
}

//----------------------------------------------------------------------------
void vtkKWWindowBase::SetErrorIcon(int s)
{
  if (!this->TrayImageError || !this->TrayImageError->IsCreated())
    {
    return;
    }

  if (s > vtkKWWindowBase::ErrorIconNone) 
    {
    this->Script("pack %s -fill both -ipadx 4 -expand yes", 
                 this->TrayImageError->GetWidgetName());
    if (s == vtkKWWindowBase::ErrorIconRed)
      {
      this->TrayImageError->SetImageOption(vtkKWIcon::ICON_SMALLERRORRED);
      }
    else if (s == vtkKWWindowBase::ErrorIconBlack)
      {
      this->TrayImageError->SetImageOption(vtkKWIcon::ICON_SMALLERROR);
      }
    }
  else
    {
    this->Script("pack forget %s", this->TrayImageError->GetWidgetName());
    }
}

//----------------------------------------------------------------------------
void vtkKWWindowBase::ErrorIconCallback()
{
  this->SetErrorIcon(vtkKWWindowBase::ErrorIconBlack);
}

//----------------------------------------------------------------------------
char* vtkKWWindowBase::GetTitle()
{
  if (!this->Title && 
      this->GetApplication() && 
      this->GetApplication()->GetName())
    {
    return this->GetApplication()->GetName();
    }
  return this->Title;
}

//----------------------------------------------------------------------------
vtkKWTclInteractor* vtkKWWindowBase::GetTclInteractor()
{
  if (!this->TclInteractor)
    {
    this->TclInteractor = vtkKWTclInteractor::New();
    }

  if (!this->TclInteractor->IsCreated() && this->IsCreated())
    {
    this->TclInteractor->SetMasterWindow(this);
    this->TclInteractor->Create(this->GetApplication());
    }
  
  return this->TclInteractor;
}

//----------------------------------------------------------------------------
void vtkKWWindowBase::DisplayTclInteractor()
{
  vtkKWTclInteractor *tcl_interactor = this->GetTclInteractor();
  if (tcl_interactor)
    {
    vtksys_stl::string title;
    if (this->GetTitle())
      {
      title += this->GetTitle();
      title += " : ";
      }
    title += "Tcl Interactor";
    tcl_interactor->SetTitle(title.c_str());
    tcl_interactor->Display();
    }
}

//----------------------------------------------------------------------------
vtkKWLabel* vtkKWWindowBase::GetStatusImage()
{
  if (!this->StatusImage)
    {
    this->StatusImage = vtkKWLabel::New();
    }

  if (!this->StatusImage->IsCreated() && 
      this->StatusFrame && this->StatusFrame->IsCreated())
    {
    this->StatusImage->SetParent(this->StatusFrame);
    this->StatusImage->Create(this->StatusFrame->GetApplication());
    this->StatusImage->SetBorderWidth(1);
    this->StatusImage->SetReliefToSunken();
    vtksys_stl::string before;
    if (this->StatusLabel && this->StatusLabel->IsCreated())
      {
      before = " -before ";
      before += this->StatusLabel->GetWidgetName();
      }
    this->StatusImage->Script(
      "pack %s -side left -anchor c -ipadx 1 -ipady 1 -fill y %s", 
      this->StatusImage->GetWidgetName(), before.c_str());
    }

  return this->StatusImage;
}

//----------------------------------------------------------------------------
void vtkKWWindowBase::UpdateStatusImage()
{
  // No default image here

  // Subclasses will likely update the StatusImage with a logo of their own.
  // Here is, for example, how, provided that you created or updated
  // the myownlogo image (photo), using vtkKWTkUtilities::UpdatePhoto for ex.
  /*
  vtkKWLabel *status_image = this->GetStatusImage();
  if (status_image && status_image->IsCreated())
    {
    status_image->SetImageOption("myownlogo");
    }
  */
}

//----------------------------------------------------------------------------
void vtkKWWindowBase::Update()
{
  // Make sure everything is enable/disable accordingly

  this->UpdateEnableState();
}

//----------------------------------------------------------------------------
void vtkKWWindowBase::UpdateToolbarState()
{
  if (this->MainToolbarSet)
    {
    this->MainToolbarSet->SetToolbarsFlatAspect(
      vtkKWToolbar::GetGlobalFlatAspect());
    this->MainToolbarSet->SetToolbarsWidgetsFlatAspect(
      vtkKWToolbar::GetGlobalWidgetsFlatAspect());
    this->PropagateEnableState(this->MainToolbarSet);
    }
}

//----------------------------------------------------------------------------
void vtkKWWindowBase::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  // Update the toolbars

  this->UpdateToolbarState();

  // Update the Tcl interactor

  this->PropagateEnableState(this->TclInteractor);

  // Update the window element

  this->PropagateEnableState(this->MainFrame);
  this->PropagateEnableState(this->StatusFrame);
  this->PropagateEnableState(this->MenuBarSeparatorFrame);

  // Given the state, can we close or not ?

  this->SetDeleteWindowProtocolCommand(
    this, this->GetEnabled() ? 
    vtkKWWindowBase::FileCloseMenuLabel : 
    "SetStatusText \"Can not close while UI is disabled\"");

  // Update the menus

  this->UpdateMenuState();
}

//----------------------------------------------------------------------------
void vtkKWWindowBase::UpdateMenuState()
{
  this->PropagateEnableState(this->FileMenu);
  this->PropagateEnableState(this->EditMenu);
  this->PropagateEnableState(this->ViewMenu);
  this->PropagateEnableState(this->WindowMenu);
  this->PropagateEnableState(this->HelpMenu);
  this->PropagateEnableState(this->ToolbarsVisibilityMenu);

  // Most Recent Files

  if (this->MostRecentFilesManager)
    {
    this->PropagateEnableState(this->MostRecentFilesManager->GetMenu());
    this->MostRecentFilesManager->UpdateMenuStateInParent();
    }

  // Update the About entry, since the pretty name also depends on the
  // limited edition mode

  if (this->HelpMenu) // do not use GetHelpMenu() here
    {
    vtksys_stl::string about_command = "DisplayAbout ";
    about_command +=  this->GetTclName();
    int pos = this->GetHelpMenu()->GetIndexOfCommand(
      this->GetApplication(), about_command.c_str());
    if (pos >= 0)
      {
      vtksys_stl::string label("-label {About ");
      label += this->GetApplication()->GetPrettyName();
      label += "}";
      this->GetHelpMenu()->ConfigureItem(pos, label.c_str());
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWWindowBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Menu: " << this->Menu << endl;
  os << indent << "FileMenu: " << this->FileMenu << endl;
  os << indent << "HelpMenu: " << this->HelpMenu << endl;
  os << indent << "MainFrame: " << this->MainFrame << endl;
  os << indent << "ProgressGauge: " << this->GetProgressGauge() << endl;
  os << indent << "PromptBeforeClose: " << this->GetPromptBeforeClose() 
     << endl;
  os << indent << "ScriptExtension: " << this->GetScriptExtension() << endl;
  os << indent << "ScriptType: " << this->GetScriptType() << endl;
  os << indent << "SupportHelp: " << this->GetSupportHelp() << endl;
  os << indent << "SupportPrint: " << this->GetSupportPrint() << endl;
  os << indent << "StatusFrame: " << this->GetStatusFrame() << endl;
  os << indent << "WindowClass: " << this->GetWindowClass() << endl;  
  os << indent << "TclInteractor: " << this->GetTclInteractor() << endl;
  os << indent << "MainToolbarSet: " << this->GetMainToolbarSet() << endl;
}


