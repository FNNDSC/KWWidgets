/*=========================================================================

  Module:    $RCSfile: vtkKWWindow.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWWindow.h"

#include "vtkKWApplication.h"
#include "vtkKWApplicationSettingsInterface.h"
#include "vtkKWCheckButton.h"
#include "vtkKWEvent.h"
#include "vtkKWFrame.h"
#include "vtkKWIcon.h"
#include "vtkKWLabel.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWMenu.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWMostRecentFilesManager.h"
#include "vtkKWNotebook.h"
#include "vtkKWProgressGauge.h"
#include "vtkKWSplitFrame.h"
#include "vtkKWTclInteractor.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWToolbar.h"
#include "vtkKWToolbarSet.h"
#include "vtkKWUserInterfaceNotebookManager.h"
#include "vtkObjectFactory.h"

#include <kwsys/SystemTools.hxx>

#define VTK_KW_HIDE_PROPERTIES_LABEL "Hide Left Panel" 
#define VTK_KW_SHOW_PROPERTIES_LABEL "Show Left Panel"
#define VTK_KW_WINDOW_DEFAULT_GEOMETRY "900x700+0+0"

vtkCxxRevisionMacro(vtkKWWindow, "$Revision: 1.230 $");
vtkCxxSetObjectMacro(vtkKWWindow, PropertiesParent, vtkKWWidget);

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWWindow );

int vtkKWWindowCommand(ClientData cd, Tcl_Interp *interp,
                             int argc, char *argv[]);

//----------------------------------------------------------------------------
vtkKWWindow::vtkKWWindow()
{
  this->PropertiesParent      = NULL;

  this->Menu                  = vtkKWMenu::New();
  this->FileMenu              = vtkKWMenu::New();
  this->HelpMenu              = vtkKWMenu::New();
  this->PageMenu              = vtkKWMenu::New();

  this->EditMenu              = NULL;
  this->ViewMenu              = NULL;
  this->WindowMenu            = NULL;

  this->Toolbars              = vtkKWToolbarSet::New();
  this->ToolbarsMenu          = NULL; 
  this->MenuBarSeparatorFrame = vtkKWFrame::New();
  this->MiddleFrame           = vtkKWSplitFrame::New();
  this->ViewFrame             = vtkKWFrame::New();

  this->StatusFrameSeparator  = vtkKWFrame::New();
  this->StatusFrame           = vtkKWFrame::New();
  this->StatusLabel           = vtkKWLabel::New();
  this->StatusImage           = vtkKWLabel::New();
  this->StatusImageName       = NULL;

  this->ProgressFrame         = vtkKWFrame::New();
  this->ProgressGauge         = vtkKWProgressGauge::New();

  this->TrayFrame             = vtkKWFrame::New();
  this->TrayImageError        = vtkKWLabel::New();

  this->Notebook              = vtkKWNotebook::New();

  this->ExitDialogWidget      = NULL;

  this->TclInteractor         = NULL;

  this->CommandFunction       = vtkKWWindowCommand;

  this->PrintTargetDPI        = 100;
  this->SupportHelp           = 1;
  this->SupportPrint          = 1;
  this->PromptBeforeClose     = 1;
  this->ScriptExtension       = 0;
  this->ScriptType            = 0;

  this->InExit                = 0;

  this->MostRecentFilesManager = vtkKWMostRecentFilesManager::New();

  this->SetWindowClass("KitwareWidget");
  this->SetScriptExtension(".tcl");
  this->SetScriptType("Tcl");
}

//----------------------------------------------------------------------------
vtkKWWindow::~vtkKWWindow()
{
  if (this->TclInteractor)
    {
    this->TclInteractor->Delete();
    this->TclInteractor = NULL;
    }

  this->Notebook->Delete();
  this->SetPropertiesParent(NULL);

#if (TK_MAJOR_VERSION == 8) && (TK_MINOR_VERSION <= 2)
  // This "hack" is here to get around a Tk bug ( Bug: 3402 )
  // in tkMenu.c
  vtkKWMenu* menuparent = vtkKWMenu::SafeDownCast(this->PageMenu->GetParent());
  if (menuparent)
    {
    menuparent->DeleteMenuItem(VTK_KW_PAGE_SETUP_MENU_LABEL);
    }
#endif

  this->Menu->Delete();
  this->PageMenu->Delete();
  this->FileMenu->Delete();
  this->HelpMenu->Delete();
  this->Toolbars->Delete();
  this->MenuBarSeparatorFrame->Delete();
  this->ViewFrame->Delete();
  this->MiddleFrame->Delete();
  this->StatusFrameSeparator->Delete();
  this->StatusFrame->Delete();
  this->StatusImage->Delete();
  this->StatusLabel->Delete();
  this->ProgressFrame->Delete();
  this->ProgressGauge->Delete();
  this->TrayFrame->Delete();
  this->TrayImageError->Delete();
  
  if (this->EditMenu)
    {
    this->EditMenu->Delete();
    }
  if (this->ViewMenu)
    {
    this->ViewMenu->Delete();
    }
  if (this->WindowMenu)
    {
    this->WindowMenu->Delete();
    }
  if (this->ToolbarsMenu)
    {
    this->ToolbarsMenu->Delete();
    this->ToolbarsMenu = NULL;
    }
  this->SetStatusImageName(0);
  this->SetScriptExtension(0);
  this->SetScriptType(0);
  this->MostRecentFilesManager->Delete();
}

//----------------------------------------------------------------------------
void vtkKWWindow::Create(vtkKWApplication *app, const char *args)
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro("class already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::Create(app, args);

  const char *wname = this->GetWidgetName();

  this->Script("wm iconname %s {%s}", wname, app->GetPrettyName());

  // Set up standard menus

  this->Menu->SetParent(this);
  this->Menu->SetTearOff(0);
  this->Menu->Create(app, "");

  this->InstallMenu(this->Menu);

  // Menu : File

  this->FileMenu->SetParent(this->Menu);
  this->FileMenu->SetTearOff(0);
  this->FileMenu->Create(app, "");

  // Menu : Print quality

  this->PageMenu->SetParent(this->FileMenu);
  this->PageMenu->SetTearOff(0);
  this->PageMenu->Create(app, "");

  char* rbv = 
    this->PageMenu->CreateRadioButtonVariable(this, "PageSetup");

  this->Script( "set %s 0", rbv );
  this->PageMenu->AddRadioButton(0, "100 DPI", rbv, this, "OnPrint 1 0", 0);
  this->PageMenu->AddRadioButton(1, "150 DPI", rbv, this, "OnPrint 1 1", 1);
  this->PageMenu->AddRadioButton(2, "300 DPI", rbv, this, "OnPrint 1 2", 0);
  delete [] rbv;

  // Menu : File (cont.)

  this->Menu->AddCascade("File", this->FileMenu, 0);

  if (this->SupportPrint)
    {
    this->FileMenu->AddCascade(
      VTK_KW_PAGE_SETUP_MENU_LABEL, this->PageMenu, 8);
    this->FileMenu->AddSeparator();
    }

  this->FileMenu->AddCommand("Close", this, "Close", 0);
  this->FileMenu->AddCommand("Exit", this->GetApplication(), "Exit", 1);

  this->MostRecentFilesManager->SetApplication(app);

  // Menu : Window : Properties panel

  this->GetWindowMenu()->AddCommand(VTK_KW_HIDE_PROPERTIES_LABEL, this,
                                    "TogglePropertiesVisibilityCallback", 1 );

  // Help menu

  this->HelpMenu->SetParent(this->Menu);
  this->HelpMenu->SetTearOff(0);
  this->HelpMenu->Create(app, "");

  if (this->SupportHelp)
    {
    this->Menu->AddCascade("Help", this->HelpMenu, 0);
    }
  ostrstream helpCmd;
  helpCmd << "DisplayHelpDialog " << this->GetTclName() << ends;
  this->HelpMenu->AddCommand("Help", this->GetApplication(), helpCmd.str(), 0);
  helpCmd.rdbuf()->freeze(0);

  if (app->HasCheckForUpdates())
    {
    this->HelpMenu->AddCommand("Check For Updates", app, "CheckForUpdates", 0);
    }

  this->HelpMenu->AddSeparator();
  ostrstream about_label;
  about_label << "About " << app->GetPrettyName() << ends;
  ostrstream aboutCmd;
  aboutCmd << "DisplayAboutDialog " << this->GetTclName() << ends;
  this->HelpMenu->AddCommand(
    about_label.str(), this->GetApplication(), aboutCmd.str(), 0);
  about_label.rdbuf()->freeze(0);
  aboutCmd.rdbuf()->freeze(0);

  // Menubar separator

  this->MenuBarSeparatorFrame->SetParent(this);  
#if defined(_WIN32)
  this->MenuBarSeparatorFrame->Create(app, "-height 2 -bd 1 -relief groove");
#else
  this->MenuBarSeparatorFrame->Create(app, "-height 2 -bd 1 -relief sunken");
#endif

  this->Script("pack %s -side top -fill x -pady 2",
               this->MenuBarSeparatorFrame->GetWidgetName());

  // Toolbar frame

  this->Toolbars->SetParent(this);  
  this->Toolbars->Create(app, "");
  this->Toolbars->ShowBottomSeparatorOn();


  // Split frame
  this->MiddleFrame->SetParent(this);
  this->MiddleFrame->Create(app);

  this->Script("pack %s -side top -fill both -expand t",
               this->MiddleFrame->GetWidgetName());

  // Split frame : view frame

  this->ViewFrame->SetParent(this->MiddleFrame->GetFrame2());
  this->ViewFrame->Create(app, "");

  this->Script("pack %s -side right -fill both -expand yes",
               this->ViewFrame->GetWidgetName());

  // Restore Window Geometry

  if (app->GetSaveWindowGeometry())
    {
    this->RestoreWindowGeometry();
    }
  else
    {
    this->Script("wm geometry %s %s", 
                 this->GetWidgetName(), VTK_KW_WINDOW_DEFAULT_GEOMETRY);
    }

  // Window properties / Application settings (leading to preferences)

  this->CreateDefaultPropertiesParent();

  // Create the notebook

  this->Notebook->SetParent(this->GetPropertiesParent());
  this->Notebook->Create(app, "");
  this->Notebook->AlwaysShowTabsOn();

  // Status frame separator

  this->StatusFrameSeparator->SetParent(this);
  this->StatusFrameSeparator->Create(app, "-height 2 -bd 1");
#if defined(_WIN32)
  this->StatusFrameSeparator->ConfigureOptions("-relief groove");
#else
  this->StatusFrameSeparator->ConfigureOptions("-relief sunken");
#endif

  this->Script("pack %s -side top -fill x -pady 2",
               this->StatusFrameSeparator->GetWidgetName());
  
  // Status frame

  this->StatusFrame->SetParent(this);
  this->StatusFrame->Create(app, NULL);
  
  this->Script("pack %s -side top -fill x -pady 0",
               this->StatusFrame->GetWidgetName());
  
  // Status frame : image

  this->SetStatusImageName(this->Script("image create photo"));
  this->CreateStatusImage();

  this->StatusImage->SetParent(this->StatusFrame);
  this->StatusImage->Create(app, "-relief sunken -bd 1");

  this->Script("%s configure -image %s -fg white -bg white "
               "-highlightbackground white -highlightcolor white "
               "-highlightthickness 0 -padx 0 -pady 0", 
               this->StatusImage->GetWidgetName(),
               this->StatusImageName);

  this->Script("pack %s -side left -anchor c -ipadx 1 -ipady 1 -fill y", 
               this->StatusImage->GetWidgetName());

  // Status frame : label

  this->StatusLabel->SetParent(this->StatusFrame);
  this->StatusLabel->Create(app, "-relief sunken -bd 0 -padx 3 -anchor w");

  this->Script("pack %s -side left -padx 1 -expand yes -fill both",
               this->StatusLabel->GetWidgetName());

  // Status frame : progress frame

  this->ProgressFrame->SetParent(this->StatusFrame);
  this->ProgressFrame->Create(app, "-relief sunken -bd 1");

  this->Script("pack %s -side left -padx 0 -fill y", 
               this->ProgressFrame->GetWidgetName());

  // Status frame : progress frame : gauge

  this->ProgressGauge->SetParent(this->ProgressFrame);
  this->ProgressGauge->SetLength(200);
  this->ProgressGauge->SetHeight(
    vtkKWTkUtilities::GetPhotoHeight(app, this->StatusImageName) - 4);
  this->ProgressGauge->Create(app, "");

  this->Script("pack %s -side right -padx 2 -pady 2",
               this->ProgressGauge->GetWidgetName());

  // Status frame : tray frame

  this->TrayFrame->SetParent(this->StatusFrame);
  this->TrayFrame->Create(app, "-relief sunken -bd 1");

  this->Script(
   "pack %s -side left -ipadx 0 -ipady 0 -padx 0 -pady 0 -fill both",
   this->TrayFrame->GetWidgetName());

  // Status frame : tray frame : error image

  this->TrayImageError->SetParent(this->TrayFrame);
  this->TrayImageError->Create(app, "");

  this->TrayImageError->SetImageOption(vtkKWIcon::ICON_SMALLERRORRED);
  
  this->TrayImageError->SetBind(this, "<Button-1>", "ErrorIconCallback");

  // If we have a User Interface Manager, it's time to create it

  vtkKWUserInterfaceManager *uim = this->GetUserInterfaceManager();
  if (uim && !uim->IsCreated())
    {
    uim->Create(app);
    }

  if (this->GetApplication()->HasRegistryValue(
        2, "RunTime", VTK_KW_SHOW_MOST_RECENT_PANELS_REG_KEY) &&
      !this->GetApplication()->GetIntRegistryValue(
        2, "RunTime", VTK_KW_SHOW_MOST_RECENT_PANELS_REG_KEY))
    {
    this->ShowMostRecentPanels(0);
    }
  else
    {
    this->ShowMostRecentPanels(1);
    }

  vtkKWUserInterfaceNotebookManager *uim_nb = 
    vtkKWUserInterfaceNotebookManager::SafeDownCast(uim);
  if (uim_nb)
    {
    if (this->GetApplication()->HasRegistryValue(
          2, "RunTime", VTK_KW_ENABLE_GUI_DRAG_AND_DROP_REG_KEY))
      {
      uim_nb->SetEnableDragAndDrop(
        this->GetApplication()->GetIntRegistryValue(
          2, "RunTime", VTK_KW_ENABLE_GUI_DRAG_AND_DROP_REG_KEY));
      }
    }

  // Udpate the enable state

  this->UpdateEnableState();
}

//----------------------------------------------------------------------------
vtkKWFrame* vtkKWWindow::GetViewFrame()
{
  return this->ViewFrame;
  //return this->MiddleFrame ? this->MiddleFrame->GetFrame2() : NULL;
}

//----------------------------------------------------------------------------
void vtkKWWindow::CreateDefaultPropertiesParent()
{
  if (!this->PropertiesParent)
    {
    vtkKWWidget *pp = vtkKWWidget::New();
    pp->SetParent(this->MiddleFrame->GetFrame1());
    pp->Create(this->GetApplication(),"frame","-bd 0");
    this->Script("pack %s -side left -fill both -expand t -anchor nw",
                 pp->GetWidgetName());
    this->SetPropertiesParent(pp);
    pp->Delete();
    }
  else
    {
    vtkDebugMacro("Properties Parent already set for Window");
    }
}

//----------------------------------------------------------------------------
void vtkKWWindow::Exit()
{  
  if (this->GetApplication()->GetDialogUp())
    {
    this->Script("bell");
    return;
    }
  if (!this->InExit)
    {
    this->InExit = 1;
    }
  else
    {
    return;
    }
  if ( this->ExitDialog() )
    {
    this->PromptBeforeClose = 0;
    this->GetApplication()->Exit();
    }
  else
    {
    this->InExit = 0;
    }
}

//----------------------------------------------------------------------------
void vtkKWWindow::Close()
{
  if (this->GetApplication()->GetDialogUp())
    {
    this->Script("bell");
    return;
    }
  if (this->PromptBeforeClose &&
      this->GetApplication()->GetNumberOfWindows() <= 1)
    {
    if ( !this->ExitDialog() )
      {
      return;
      }
    }
  this->CloseNoPrompt();
}

//----------------------------------------------------------------------------
void vtkKWWindow::CloseNoPrompt()
{
  if (this->TclInteractor )
    {
    this->TclInteractor->SetMasterWindow(NULL);
    this->TclInteractor->Delete();
    this->TclInteractor = NULL;
    }

  // Save its geometry

  if (this->GetApplication()->GetSaveWindowGeometry())
    {
    this->SaveWindowGeometry();
    }

  // Close this window in the application. The
  // application will exit if there are no more windows.
  this->GetApplication()->CloseWindow(this);
}

//----------------------------------------------------------------------------
void vtkKWWindow::SaveWindowGeometry()
{
  if (this->IsCreated())
    {
    const char *res = this->Script("wm geometry %s", this->GetWidgetName());

    this->GetApplication()->SetRegistryValue(
      2, "Geometry", VTK_KW_WINDOW_GEOMETRY_REG_KEY, "%s", 
      res);

    this->GetApplication()->SetRegistryValue(
      2, "Geometry", VTK_KW_WINDOW_FRAME1_SIZE_REG_KEY, "%d", 
      this->MiddleFrame->GetFrame1Size());
    }
}

//----------------------------------------------------------------------------
void vtkKWWindow::RestoreWindowGeometry()
{
  if (this->IsCreated())
    {
    if (this->GetApplication()->HasRegistryValue(
          2, "Geometry", VTK_KW_WINDOW_GEOMETRY_REG_KEY))
      {
      char geometry[40];
      if (this->GetApplication()->GetRegistryValue(
            2, "Geometry", VTK_KW_WINDOW_GEOMETRY_REG_KEY, geometry))
        {
        this->Script("wm geometry %s %s", this->GetWidgetName(), geometry);
        }
      }
    else
      {
      this->Script("wm geometry %s %s", 
                   this->GetWidgetName(), VTK_KW_WINDOW_DEFAULT_GEOMETRY);
      }

    if (this->GetApplication()->HasRegistryValue(
          2, "Geometry", VTK_KW_WINDOW_FRAME1_SIZE_REG_KEY))
      {
      int reg_size = this->GetApplication()->GetIntRegistryValue(
        2, "Geometry", VTK_KW_WINDOW_FRAME1_SIZE_REG_KEY);
      if (reg_size >= this->MiddleFrame->GetFrame1MinimumSize())
        {
        this->MiddleFrame->SetFrame1Size(reg_size);
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWWindow::Render()
{
}

//----------------------------------------------------------------------------
void vtkKWWindow::SetStatusText(const char *text)
{
  this->StatusLabel->SetText(text);
}

//----------------------------------------------------------------------------
const char *vtkKWWindow::GetStatusText()
{
  return this->StatusLabel->GetText();
}

//----------------------------------------------------------------------------
vtkKWMenu *vtkKWWindow::GetEditMenu()
{
  if (this->EditMenu)
    {
    return this->EditMenu;
    }

  if ( !this->IsCreated() )
    {
    return 0;
    }
  
  this->EditMenu = vtkKWMenu::New();
  this->EditMenu->SetParent(this->GetMenu());
  this->EditMenu->SetTearOff(0);
  this->EditMenu->Create(this->GetApplication(),"");
  // Make sure Edit menu is next to file menu
  this->Menu->InsertCascade(1, "Edit", this->EditMenu, 0);
  return this->EditMenu;
}

//----------------------------------------------------------------------------
vtkKWMenu *vtkKWWindow::GetViewMenu()
{
  if (this->ViewMenu)
    {
    return this->ViewMenu;
    }

  if ( !this->IsCreated() )
    {
    return 0;
    }

  this->ViewMenu = vtkKWMenu::New();
  this->ViewMenu->SetParent(this->GetMenu());
  this->ViewMenu->SetTearOff(0);
  this->ViewMenu->Create(this->GetApplication(), "");
  // make sure Help menu is on the right
  if (this->EditMenu)
    { 
    this->Menu->InsertCascade(2, "View", this->ViewMenu, 0);
    }
  else
    {
    this->Menu->InsertCascade(1, "View", this->ViewMenu, 0);
    }
  
  return this->ViewMenu;
}

//----------------------------------------------------------------------------
void vtkKWWindow::AddToolbar(vtkKWToolbar* toolbar, const char* name,
  int visibility /*=1*/)
{
  if (!this->Toolbars->AddToolbar(toolbar))
    {
    return;
    }
  int id = this->Toolbars->GetNumberOfToolbars() - 1; 
  ostrstream command;
  command << "ToggleToolbarVisibility " << id << " " << name << ends;
  this->AddToolbarToMenu(toolbar, name, this, command.str());
  command.rdbuf()->freeze(0);

  // Restore state from registry.
  ostrstream reg_key;
  reg_key << name << "_ToolbarVisibility" << ends;
  if (this->GetApplication()->GetRegistryValue(2, "RunTime", reg_key.str(), 0))
    {
    visibility = this->GetApplication()->GetIntRegistryValue(2, "RunTime", reg_key.str());
    }
  this->SetToolbarVisibility(toolbar, name, visibility);
  reg_key.rdbuf()->freeze(0);
}

//----------------------------------------------------------------------------
void vtkKWWindow::AddToolbarToMenu(vtkKWToolbar*, const char* name,
  vtkKWWidget* target, const char* command)
{
  if (!this->ToolbarsMenu)
    {
    this->ToolbarsMenu = vtkKWMenu::New();
    this->ToolbarsMenu->SetParent(this->GetWindowMenu());
    this->ToolbarsMenu->SetTearOff(0);
    this->ToolbarsMenu->Create(this->GetApplication(), "");
    this->GetWindowMenu()->InsertCascade(2, " Toolbars", this->ToolbarsMenu, 1,
      "Customize Toolbars");
    }
  char *rbv = this->ToolbarsMenu->CreateCheckButtonVariable(this, name);

  this->ToolbarsMenu->AddCheckButton(
    name, rbv, target, command, "Show/Hide this toolbar");

  delete [] rbv;
  
  this->ToolbarsMenu->CheckCheckButton(this, name, 1); 
}

//----------------------------------------------------------------------------
void vtkKWWindow::HideToolbar(vtkKWToolbar* toolbar, const char* name)
{
  this->SetToolbarVisibility(toolbar, name, 0);
}

//----------------------------------------------------------------------------
void vtkKWWindow::ShowToolbar(vtkKWToolbar* toolbar, const char* name)
{
  this->SetToolbarVisibility(toolbar, name, 1);
}

//----------------------------------------------------------------------------
void vtkKWWindow::SetToolbarVisibility(vtkKWToolbar* toolbar, const char* name, int flag)
{
  this->Toolbars->SetToolbarVisibility(toolbar, flag);
  this->SetToolbarVisibilityInternal(toolbar, name, flag); 
}

//----------------------------------------------------------------------------
void vtkKWWindow::SetToolbarVisibilityInternal(vtkKWToolbar* ,
  const char* name, int flag)
{
  if (this->ToolbarsMenu)
    {
    this->ToolbarsMenu->CheckCheckButton(this, name, flag);
    }
  ostrstream reg_key;
  reg_key << name << "_ToolbarVisibility" << ends;
  this->GetApplication()->SetRegistryValue(2, "RunTime", reg_key.str(),
    "%d", flag);
  reg_key.rdbuf()->freeze(0); 
  this->UpdateToolbarState();
}

//----------------------------------------------------------------------------
void vtkKWWindow::ToggleToolbarVisibility(int id, const char* name)
{
  vtkKWToolbar* toolbar = this->Toolbars->GetToolbar(id);
  if (!toolbar)
    {
    return;
    }
  int new_visibility = (this->Toolbars->IsToolbarVisible(toolbar))? 0 : 1;
  this->SetToolbarVisibility(toolbar, name, new_visibility);
}
//----------------------------------------------------------------------------
vtkKWMenu *vtkKWWindow::GetWindowMenu()
{
  if (this->WindowMenu)
    {
    return this->WindowMenu;
    }

  if ( !this->IsCreated() )
    {
    return 0;
    }

  this->WindowMenu = vtkKWMenu::New();
  this->WindowMenu->SetParent(this->GetMenu());
  this->WindowMenu->SetTearOff(0);
  this->WindowMenu->Create(this->GetApplication(), "");
  // make sure Help menu is on the right
  if (this->EditMenu)
    { 
    this->Menu->InsertCascade(1, "Window", this->WindowMenu, 0);
    }
  else
    {
    this->Menu->InsertCascade(2, "Window", this->WindowMenu, 0);
    }
  
  return this->WindowMenu;
}

//----------------------------------------------------------------------------
void vtkKWWindow::OnPrint(int propagate, int res)
{
  int dpis[] = { 100, 150, 300 };
  this->PrintTargetDPI = dpis[res];
  if ( propagate )
    {
    float dpi = res;
    this->InvokeEvent(vtkKWEvent::PrinterDPIChangedEvent, &dpi);
    }
  else
    {
    char array[][20] = { "100 DPI", "150 DPI", "300 DPI" };
    this->PageMenu->Invoke( this->PageMenu->GetIndex(array[res]) );
    }
}

//----------------------------------------------------------------------------
int vtkKWWindow::GetPropertiesVisiblity()
{
  return (this->MiddleFrame && this->MiddleFrame->GetFrame1Visibility() ? 1 : 0);
}

//----------------------------------------------------------------------------
void vtkKWWindow::SetPropertiesVisiblity(int arg)
{
  if (arg)
    {
    if (!this->GetPropertiesVisiblity())
      {
      this->MiddleFrame->Frame1VisibilityOn();
      this->Script("%s entryconfigure 0 -label {%s}",
                   this->GetWindowMenu()->GetWidgetName(),
                   VTK_KW_HIDE_PROPERTIES_LABEL);
      }
    }
  else
    {
    if (this->GetPropertiesVisiblity())
      {
      this->MiddleFrame->Frame1VisibilityOff();
      this->Script("%s entryconfigure 0 -label {%s}",
                   this->GetWindowMenu()->GetWidgetName(),
                   VTK_KW_SHOW_PROPERTIES_LABEL);
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWWindow::TogglePropertiesVisibilityCallback()
{
  int arg = !this->GetPropertiesVisiblity();
  this->SetPropertiesVisiblity(arg);
  float farg = arg;
  this->InvokeEvent(vtkKWEvent::UserInterfaceVisibilityChangedEvent, &farg);
}

//----------------------------------------------------------------------------
void vtkKWWindow::ShowWindowProperties()
{
  this->ShowProperties();
  
  // Forget current props and pack the notebook

  this->Notebook->UnpackSiblings();

  this->Script("pack %s -pady 0 -padx 0 -fill both -expand yes -anchor n",
               this->Notebook->GetWidgetName());
}

//----------------------------------------------------------------------------
int vtkKWWindow::ShowApplicationSettingsInterface()
{
  if (this->GetApplicationSettingsInterface())
    {
    this->ShowWindowProperties();
    return this->GetApplicationSettingsInterface()->Raise();
    }

  return 0;
}

//----------------------------------------------------------------------------
void vtkKWWindow::ShowMostRecentPanels(int arg)
{
  if (arg)
    {
    this->Notebook->ShowAllPagesWithSameTagOff();
    this->Notebook->ShowOnlyPagesWithSameTagOff();
    this->Notebook->SetNumberOfMostRecentPages(4);
    this->Notebook->ShowOnlyMostRecentPagesOn();
    }
  else
    {
    this->Notebook->ShowAllPagesWithSameTagOff();
    this->Notebook->ShowOnlyMostRecentPagesOff();
    this->Notebook->ShowOnlyPagesWithSameTagOn();
    }
}

//----------------------------------------------------------------------------
void vtkKWWindow::LoadScript()
{
  vtkKWLoadSaveDialog* load_dialog = vtkKWLoadSaveDialog::New();
  this->RetrieveLastPath(load_dialog, "LoadScriptLastPath");
  load_dialog->SetParent(this);
  load_dialog->Create(this->GetApplication(), NULL);
  load_dialog->SaveDialogOff();
  load_dialog->SetTitle("Load Script");
  load_dialog->SetDefaultExtension(this->ScriptExtension);

  kwsys_stl::string filetypes;
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
    if (!kwsys::SystemTools::FileExists(load_dialog->GetFileName()))
      {
      vtkWarningMacro("Unable to open script file!");
      }
    else
      {
      this->SaveLastPath(load_dialog, "LoadScriptLastPath");
      this->LoadScript(load_dialog->GetFileName());
      }
    }

  this->SetEnabled(enabled);
  load_dialog->Delete();
}

//----------------------------------------------------------------------------
void vtkKWWindow::LoadScript(const char *filename)
{
  this->GetApplication()->LoadScript(filename);
}

//----------------------------------------------------------------------------
void vtkKWWindow::AddRecentFilesMenu(
  const char *menuEntry, vtkKWObject *target, const char *label, int underline)
{
  if (!this->IsCreated() || !label || !this->FileMenu || 
      !this->MostRecentFilesManager)
    {
    return;
    }

  // Create the menu if not done already

  vtkKWMenu *mrf_menu = this->MostRecentFilesManager->GetMenu();
  if (!mrf_menu->IsCreated())
    {
    mrf_menu->SetParent(this->FileMenu);
    mrf_menu->SetTearOff(0);
    mrf_menu->Create(this->GetApplication(), NULL);
    }

  // Remove the menu if already there (in case that function was used to
  // move the menu)

  if (this->FileMenu->HasItem(label))
    {
    this->FileMenu->DeleteMenuItem(label);
    }

  // Find where to insert

  int insert_idx;
  if (!menuEntry || !this->FileMenu->HasItem(menuEntry))
    {
    insert_idx = this->GetFileMenuIndex();
    }
  else
    {
    insert_idx = this->FileMenu->GetIndex(menuEntry) - 1;
    }
  this->FileMenu->InsertCascade(insert_idx, label, mrf_menu, underline);

  // Fill the recent files vector with recent files stored in registry
  // this will also update the menu

  this->MostRecentFilesManager->SetDefaultTargetObject(target);
  this->MostRecentFilesManager->LoadFilesFromRegistry();
}

//----------------------------------------------------------------------------
void vtkKWWindow::AddRecentFile(const char *name, 
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
int vtkKWWindow::GetFileMenuIndex()
{
  if ( !this->IsCreated() )
    {
    return 0;
    }

  // First find the print-related menu commands

  if (this->GetFileMenu()->HasItem(VTK_KW_PAGE_SETUP_MENU_LABEL))
    {
    return this->GetFileMenu()->GetIndex(VTK_KW_PAGE_SETUP_MENU_LABEL);
    }

  // Otherwise find Close or Exit if Close was removed

  int clidx;
  if (this->GetFileMenu()->HasItem("Close"))
    {
    clidx = this->GetFileMenu()->GetIndex("Close");  
    }
  else
    {
    clidx = this->GetFileMenu()->GetIndex("Exit");  
    }

  return clidx;  
}

//----------------------------------------------------------------------------
int vtkKWWindow::GetHelpMenuIndex()
{
  if (!this->IsCreated())
    {
    return 0;
    }

  // Find about

  if (this->HelpMenu->HasItem("About*"))
    {
    return this->HelpMenu->GetIndex("About*") - 1;
    }

  return this->HelpMenu->GetNumberOfItems();
}

//----------------------------------------------------------------------------
int vtkKWWindow::ExitDialog()
{
  //this->GetApplication()->GetBalloonHelpWidget(0);
  if ( this->ExitDialogWidget )
    {
    return 1;
    }
  ostrstream title;
  title << "Exit " << this->GetApplication()->GetPrettyName() 
        << ends;
  ostrstream str;
  str << "Are you sure you want to exit " 
      << this->GetApplication()->GetPrettyName() << "?" << ends;
  
  vtkKWMessageDialog *dlg2 = vtkKWMessageDialog::New();
  this->ExitDialogWidget = dlg2;
  dlg2->SetStyleToYesNo();
  dlg2->SetMasterWindow(this);
  dlg2->SetOptions(
     vtkKWMessageDialog::QuestionIcon | vtkKWMessageDialog::RememberYes |
     vtkKWMessageDialog::Beep | vtkKWMessageDialog::YesDefault );
  dlg2->SetDialogName(VTK_KW_EXIT_DIALOG_NAME);
  dlg2->Create(this->GetApplication(),"");
  dlg2->SetText( str.str() );
  dlg2->SetTitle( title.str() );
  int ret = dlg2->Invoke();
  this->ExitDialogWidget = 0;
  dlg2->Delete();

  str.rdbuf()->freeze(0);
  title.rdbuf()->freeze(0);
 
  vtkKWApplicationSettingsInterface *asi =  
    this->GetApplicationSettingsInterface();
  if (asi)
    {
    asi->Update();
    }
 
  return ret;
}

//----------------------------------------------------------------------------
void vtkKWWindow::SaveLastPath(vtkKWLoadSaveDialog *dialog, const char* key)
{
  //  "OpenDirectory"
  if ( dialog->GetLastPath() )
    {
    this->GetApplication()->SetRegistryValue(
      1, "RunTime", key, dialog->GetLastPath());
    }
}

//----------------------------------------------------------------------------
void vtkKWWindow::RetrieveLastPath(vtkKWLoadSaveDialog *dialog, const char* key)
{
  char buffer[1024];
  if ( this->GetApplication()->GetRegistryValue(1, "RunTime", key, buffer) )
    {
    if ( *buffer )
      {
      dialog->SetLastPath( buffer );
      }  
    }
}

//----------------------------------------------------------------------------
void vtkKWWindow::SaveColor(int level, const char* key, double rgb[3])
{
  this->GetApplication()->SetRegistryValue(
    level, "Colors", key, "Color: %lf %lf %lf", rgb[0], rgb[1], rgb[2]);
}

//----------------------------------------------------------------------------
void vtkKWWindow::SaveColor(int level, const char* key, float rgb[3])
{
  double drgb[3];
  drgb[0] = rgb[0];
  drgb[1] = rgb[1];
  drgb[2] = rgb[2];
  this->SaveColor(level, key, drgb);
}

//----------------------------------------------------------------------------
int vtkKWWindow::RetrieveColor(int level, const char* key, double rgb[3])
{
  char buffer[1024];
  rgb[0] = -1;
  rgb[1] = -1;
  rgb[2] = -1;

  int ok = 0;
  if (this->GetApplication()->GetRegistryValue(
        level, "Colors", key, buffer) )
    {
    if (*buffer)
      {      
      sscanf(buffer, "Color: %lf %lf %lf", rgb, rgb+1, rgb+2);
      ok = 1;
      }
    }
  return ok;
}

//----------------------------------------------------------------------------
int vtkKWWindow::RetrieveColor(int level, const char* key, float rgb[3])
{
  double drgb[3];
  int res = this->RetrieveColor(level, key, drgb);
  rgb[0] = drgb[0];
  rgb[1] = drgb[1];
  rgb[2] = drgb[2];
  return res;
}

//----------------------------------------------------------------------------
void vtkKWWindow::WarningMessage(const char* message)
{
  vtkKWMessageDialog::PopupMessage(
    this->GetApplication(), this, "Warning",
    message, vtkKWMessageDialog::WarningIcon);
  this->SetErrorIcon(vtkKWWindow::ERROR_ICON_RED);
  this->InvokeEvent(vtkKWEvent::WarningMessageEvent, (void*)message);
}

//----------------------------------------------------------------------------
void vtkKWWindow::ErrorMessage(const char* message)
{
  vtkKWMessageDialog::PopupMessage(
    this->GetApplication(), this, "Error",
    message, vtkKWMessageDialog::ErrorIcon);
  this->SetErrorIcon(vtkKWWindow::ERROR_ICON_RED);
  this->InvokeEvent(vtkKWEvent::ErrorMessageEvent, (void*)message);
}

//----------------------------------------------------------------------------
void vtkKWWindow::SetErrorIcon(int s)
{
  if (!this->TrayImageError || !this->TrayImageError->IsCreated())
    {
    return;
    }

  if (s > vtkKWWindow::ERROR_ICON_NONE) 
    {
    this->Script("pack %s -fill both -ipadx 4 -expand yes", 
                 this->TrayImageError->GetWidgetName());
    if (s == vtkKWWindow::ERROR_ICON_RED)
      {
      this->TrayImageError->SetImageOption(vtkKWIcon::ICON_SMALLERRORRED);
      }
    else if (s == vtkKWWindow::ERROR_ICON_BLACK)
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
void vtkKWWindow::ErrorIconCallback()
{
  this->SetErrorIcon(vtkKWWindow::ERROR_ICON_BLACK);
}

//----------------------------------------------------------------------------
char* vtkKWWindow::GetTitle()
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
void vtkKWWindow::DisplayCommandPrompt()
{
  if ( ! this->TclInteractor )
    {
    this->TclInteractor = vtkKWTclInteractor::New();
    ostrstream title;
    if (this->GetTitle())
      {
      title << this->GetTitle() << " : ";
      }
    title << "Command Prompt" << ends;
    this->TclInteractor->SetTitle(title.str());
    title.rdbuf()->freeze(0);
    this->TclInteractor->SetMasterWindow(this);
    this->TclInteractor->Create(this->GetApplication(), NULL);
    }
  
  this->TclInteractor->Display();
}

//----------------------------------------------------------------------------
void vtkKWWindow::ProcessEvent( vtkObject* object,
                                unsigned long event, 
                                void *clientdata, void *calldata)
{   
  float *fArgs = 0;
  if (calldata)
    {    
    fArgs = reinterpret_cast<float *>(calldata);
    }

  vtkKWWindow *self = reinterpret_cast<vtkKWWindow *>(clientdata);
  self->InternalProcessEvent(object, event, fArgs, calldata);
}

//----------------------------------------------------------------------------
void vtkKWWindow::InternalProcessEvent(vtkObject*, unsigned long, 
                                       float *, void *)
{
  // No implementation
}

//----------------------------------------------------------------------------
void vtkKWWindow::UpdateToolbarState()
{
  if (!this->Toolbars)
    {
    return;
    }

  int flat_frame;
  if (this->GetApplication()->HasRegistryValue(
    2, "RunTime", VTK_KW_TOOLBAR_FLAT_FRAME_REG_KEY))
    {
    flat_frame = this->GetApplication()->GetIntRegistryValue(
      2, "RunTime", VTK_KW_TOOLBAR_FLAT_FRAME_REG_KEY);
    }
  else
    {
    flat_frame = vtkKWToolbar::GetGlobalFlatAspect();
    }

  int flat_buttons;
  if (this->GetApplication()->HasRegistryValue(
    2, "RunTime", VTK_KW_TOOLBAR_FLAT_BUTTONS_REG_KEY))
    {
    flat_buttons = this->GetApplication()->GetIntRegistryValue(
      2, "RunTime", VTK_KW_TOOLBAR_FLAT_BUTTONS_REG_KEY);
    }
  else
    {
    flat_buttons = vtkKWToolbar::GetGlobalWidgetsFlatAspect();
    }

  this->Toolbars->SetToolbarsFlatAspect(flat_frame);
  this->Toolbars->SetToolbarsWidgetsFlatAspect(flat_buttons);

  // The split frame packing mechanism is so weird that I will have
  // to unpack the toolbar frame myself in case it's empty, otherwise
  // the middle frame won't claim the space used by the toolbar frame

  if (this->Toolbars->IsCreated())
    {
    if (this->Toolbars->GetNumberOfVisibleToolbars())
      {
      this->Script(
        "pack %s -padx 0 -pady 0 -side top -fill x -expand no -after %s",
        this->Toolbars->GetWidgetName(),
        this->MenuBarSeparatorFrame->GetWidgetName());
      this->Toolbars->PackToolbars();
      }
    else
      {
      this->Toolbars->Unpack();
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWWindow::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  // Update the toolbars

  if (this->Toolbars)
    {
    this->PropagateEnableState(this->Toolbars);
    this->UpdateToolbarState();
    }

  // Update the notebook

  this->PropagateEnableState(this->Notebook);

  // Update all the user interface panels

  if (this->GetUserInterfaceManager())
    {
    this->GetUserInterfaceManager()->SetEnabled(this->GetEnabled());
    // As a side effect, SetEnabled() call an Update() on the panel, 
    // which will call an UpdateEnableState() too,
    // this->GetUserInterfaceManager()->UpdateEnableState();
    // this->GetUserInterfaceManager()->Update();
    }

  // Update the Tcl interactor

  this->PropagateEnableState(this->TclInteractor);

  // Update the window element

  this->PropagateEnableState(this->MiddleFrame);
  this->PropagateEnableState(this->StatusFrame);
  this->PropagateEnableState(this->PropertiesParent);
  this->PropagateEnableState(this->MenuBarSeparatorFrame);

  // Do not disable the status image, it has not functionality attached 
  // anyway, and is used to display the application logo: disabling it 
  // only makes it look fairly ugly/dithered.
  // this->PropagateEnableState(this->StatusImage);

  // Given the state, can we close or not ?

  this->SetDeleteWindowProtocolCommand(
    this, this->GetEnabled() ? 
    "Close" : "SetStatusText \"Can not close while UI is disabled\"");

  // Update the menus

  this->UpdateMenuState();
}

//----------------------------------------------------------------------------
void vtkKWWindow::UpdateMenuState()
{
  this->PropagateEnableState(this->Menu);
  /*
  if (this->Menu)
    {
    this->Menu->SetState(
    this->GetEnabled() ? vtkKWMenu::Normal : vtkKWMenu::Disabled);
    }
  */

  this->PropagateEnableState(this->FileMenu);
  this->PropagateEnableState(this->EditMenu);
  this->PropagateEnableState(this->ViewMenu);
  this->PropagateEnableState(this->WindowMenu);
  this->PropagateEnableState(this->HelpMenu);
  this->PropagateEnableState(this->PageMenu);
  this->PropagateEnableState(this->ToolbarsMenu);

  // Most Recent Files

  if (this->MostRecentFilesManager)
    {
    this->PropagateEnableState(this->MostRecentFilesManager->GetMenu());
    this->MostRecentFilesManager->UpdateMenuStateInParent();
    }

  // Update the About entry, since the pretty name also depends on the
  // limited edition mode

  if (this->HelpMenu)
    {
    kwsys_stl::string about_command = "DisplayAbout ";
    about_command +=  this->GetTclName();
    int pos = this->HelpMenu->GetIndexOfCommand(
      this->GetApplication(), about_command.c_str());
    if (pos >= 0)
      {
      ostrstream label;
      label << "-label {About " 
            << this->GetApplication()->GetPrettyName() << "}"<<ends;
      this->HelpMenu->ConfigureItem(pos, label.str());
      label.rdbuf()->freeze(0);
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWWindow::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Menu: " << this->GetMenu() << endl;
  os << indent << "FileMenu: " << this->GetFileMenu() << endl;
  os << indent << "HelpMenu: " << this->GetHelpMenu() << endl;
  os << indent << "Notebook: " << this->GetNotebook() << endl;
  os << indent << "PrintTargetDPI: " << this->GetPrintTargetDPI() << endl;
  os << indent << "ProgressGauge: " << this->GetProgressGauge() << endl;
  os << indent << "PromptBeforeClose: " << this->GetPromptBeforeClose() 
     << endl;
  os << indent << "PropertiesParent: " << this->GetPropertiesParent() << endl;
  os << indent << "ScriptExtension: " << this->GetScriptExtension() << endl;
  os << indent << "ScriptType: " << this->GetScriptType() << endl;
  os << indent << "SupportHelp: " << this->GetSupportHelp() << endl;
  os << indent << "SupportPrint: " << this->GetSupportPrint() << endl;
  os << indent << "StatusFrame: " << this->GetStatusFrame() << endl;
  os << indent << "ViewFrame: " << this->GetViewFrame() << endl;
  os << indent << "WindowClass: " << this->GetWindowClass() << endl;  
  os << indent << "TclInteractor: " << this->GetTclInteractor() << endl;
  os << indent << "Toolbars: " << this->GetToolbars() << endl;
}


