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

#include "KitwareLogo.h"
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
#include "vtkKWNotebook.h"
#include "vtkKWProgressGauge.h"
#include "vtkKWSplitFrame.h"
#include "vtkKWTclInteractor.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWToolbar.h"
#include "vtkKWToolbarSet.h"
#include "vtkKWUserInterfaceNotebookManager.h"
#include "vtkKWView.h"
#include "vtkKWViewCollection.h"
#include "vtkKWWidgetCollection.h"
#include "vtkKWWindowCollection.h"
#include "vtkObjectFactory.h"
#include "vtkString.h"
#include "vtkVector.txx"
#include "vtkVectorIterator.txx"

#define VTK_KW_HIDE_PROPERTIES_LABEL "Hide Left Panel" 
#define VTK_KW_SHOW_PROPERTIES_LABEL "Show Left Panel"
#define VTK_KW_WINDOW_DEFAULT_GEOMETRY "900x700+0+0"

vtkCxxRevisionMacro(vtkKWWindow, "$Revision: 1.201 $");
vtkCxxSetObjectMacro(vtkKWWindow, PropertiesParent, vtkKWWidget);

#define VTK_KW_RECENT_FILES_MAX 20

//----------------------------------------------------------------------------
class vtkKWRecentFileEntry
{
public:
  vtkKWRecentFileEntry();
  ~vtkKWRecentFileEntry();

  char *GetFileName() { return this->FileName; }
  void SetFileName(const char *file);
  char *GetCommand() { return this->Command; }
  void SetCommand(const char *command);
  vtkKWObject *GetTarget() { return this->Target; }
  void SetTarget(vtkKWObject *target) { this->Target = target; }

  int Same(const char *filename, vtkKWObject *target, const char *command);
  
private:

  char *FileName;
  char *Command;
  vtkKWObject *Target;  
};

vtkKWRecentFileEntry::vtkKWRecentFileEntry()
{
  this->FileName = 0;
  this->Target   = 0;
  this->Command  = 0;
}

vtkKWRecentFileEntry::~vtkKWRecentFileEntry()
{
  if (this->FileName)
    {
    delete [] this->FileName;
    }
  if (this->Command)
    {
    delete [] this->Command;
    }
}

void vtkKWRecentFileEntry::SetFileName(const char *file)
{
  if (this->FileName)
    {
    delete [] this->FileName;
    this->FileName = 0;
    }
  if (file)
    {
    this->FileName = vtkString::Duplicate(file);
    }
}

void vtkKWRecentFileEntry::SetCommand(const char *command)
{
  if (this->Command)
    {
    delete [] this->Command;
    this->Command = 0;
    }
  if (command)
    {
    this->Command = vtkString::Duplicate(command);
    }
}

int vtkKWRecentFileEntry::Same(
  const char *filename, vtkKWObject *vtkNotUsed(target), const char *vtkNotUsed(command))
{
  return (this->FileName && filename && !strcmp(filename, this->FileName));
}

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWWindow );

int vtkKWWindowCommand(ClientData cd, Tcl_Interp *interp,
                             int argc, char *argv[]);

//----------------------------------------------------------------------------
vtkKWWindow::vtkKWWindow()
{
  this->Views                 = vtkKWViewCollection::New();

  this->PropertiesParent      = NULL;
  this->SelectedView          = NULL;

  this->Menu                  = vtkKWMenu::New();
  this->MenuFile              = vtkKWMenu::New();
  this->MenuHelp              = vtkKWMenu::New();
  this->PageMenu              = vtkKWMenu::New();
  this->MenuRecentFiles       = NULL;
  this->MenuEdit              = NULL;
  this->MenuView              = NULL;
  this->MenuWindow            = NULL;

  this->Toolbars              = vtkKWToolbarSet::New();
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
  this->SupportPrint           = 1;
  this->WindowClass           = NULL;
  this->Title                 = NULL;
  this->PromptBeforeClose     = 1;
  this->RecentFilesVector     = 0;
  this->NumberOfRecentFiles   = 10;
  this->ScriptExtension       = 0;
  this->ScriptType            = 0;

  this->InExit                = 0;

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

  if (this->RecentFilesVector)
    {
    vtkVectorIterator<vtkKWRecentFileEntry *> *it = 
      this->RecentFilesVector->NewIterator();
    while (!it->IsDoneWithTraversal())
      {
      vtkKWRecentFileEntry *rfe = 0;
      if (it->GetData(rfe) == VTK_OK && rfe)
        {
        delete rfe;
        }
      it->GoToNextItem();
      }
    it->Delete();
    this->RecentFilesVector->Delete();
    }

  this->Notebook->Delete();
  this->SetPropertiesParent(NULL);
  this->SetSelectedView(NULL);
  if (this->Views)
    {
    this->Views->Delete();
    this->Views = NULL;
    }

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
  this->MenuFile->Delete();
  if (this->MenuRecentFiles)
    {
    this->MenuRecentFiles->Delete();
    }
  this->MenuHelp->Delete();
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
  
  if (this->MenuEdit)
    {
    this->MenuEdit->Delete();
    }
  if (this->MenuView)
    {
    this->MenuView->Delete();
    }
  if (this->MenuWindow)
    {
    this->MenuWindow->Delete();
    }

  this->SetStatusImageName(0);
  this->SetWindowClass(0);
  this->SetTitle(0);
  this->SetScriptExtension(0);
  this->SetScriptType(0);
}

//----------------------------------------------------------------------------
void vtkKWWindow::Create(vtkKWApplication *app, const char *args)
{
  // Call the superclass to set the appropriate flags

  if (!this->Superclass::Create(app, NULL, NULL))
    {
    vtkErrorMacro("Failed creating widget " << this->GetClassName());
    return;
    }

  const char *wname = this->GetWidgetName();

  this->Script("toplevel %s -visual best %s -class %s",
               wname, (args ? args : ""), this->WindowClass);

  this->Script("wm title %s {%s}", 
               wname, this->GetTitle());

  this->Script("wm iconname %s {%s}",
               wname, app->GetApplicationPrettyName());

  // Set up standard menus

  this->Menu->SetParent(this);
  this->Menu->SetTearOff(0);
  this->Menu->Create(app, "");

  this->InstallMenu(this->Menu);

  // Menu : File

  this->MenuFile->SetParent(this->Menu);
  this->MenuFile->SetTearOff(0);
  this->MenuFile->Create(app, "");

  // Menu : Print quality

  this->PageMenu->SetParent(this->MenuFile);
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

  this->Menu->AddCascade("File", this->MenuFile, 0);

  if (this->SupportPrint)
    {
    this->MenuFile->AddCascade(
      VTK_KW_PAGE_SETUP_MENU_LABEL, this->PageMenu, 8);
    this->MenuFile->AddSeparator();
    }

  this->MenuFile->AddCommand("Close", this, "Close", 0);
  this->MenuFile->AddCommand("Exit", this, "Exit", 1);

  // Menu : Window : Properties panel

  this->GetMenuWindow()->AddCommand(VTK_KW_HIDE_PROPERTIES_LABEL, this,
                                    "TogglePropertiesVisibilityCallback", 1 );

  // Help menu

  this->MenuHelp->SetParent(this->Menu);
  this->MenuHelp->SetTearOff(0);
  this->MenuHelp->Create(app, "");

  if (this->SupportHelp)
    {
    this->Menu->AddCascade("Help", this->MenuHelp, 0);
    }
  this->MenuHelp->AddCommand("Help", this, "DisplayHelp", 0);

  if (app->HasCheckForUpdates())
    {
    this->MenuHelp->AddCommand("Check For Updates", app, "CheckForUpdates", 0);
    }

  this->MenuHelp->AddSeparator();
  ostrstream about_label;
  about_label << "About " << app->GetApplicationPrettyName() << ends;
  this->MenuHelp->AddCommand(about_label.str(), this, "DisplayAbout", 0);
  about_label.rdbuf()->freeze(0);

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
    vtkKWTkUtilities::GetPhotoHeight(
      app->GetMainInterp(), this->StatusImageName) - 4);
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
  
  this->TrayImageError->SetBind(this, "<Button-1>", "ProcessErrorClick");

  // If we have a User Interface Manager, it's time to create it

  vtkKWUserInterfaceManager *uim = this->GetUserInterfaceManager();
  if (uim && !uim->IsCreated())
    {
    uim->Create(app);
    }

  if (this->GetApplication()->HasRegisteryValue(
        2, "RunTime", VTK_KW_SHOW_MOST_RECENT_PANELS_REG_KEY) &&
      !this->GetIntRegisteryValue(
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
    if (this->GetApplication()->HasRegisteryValue(
          2, "RunTime", VTK_KW_ENABLE_GUI_DRAG_AND_DROP_REG_KEY))
      {
      uim_nb->SetEnableDragAndDrop(
        this->GetIntRegisteryValue(
          2, "RunTime", VTK_KW_ENABLE_GUI_DRAG_AND_DROP_REG_KEY));
      }
    }

  // Udpate the enable state

  this->UpdateEnableState();
}

//----------------------------------------------------------------------------
void vtkKWWindow::DisplayHelp()
{
  this->GetApplication()->DisplayHelp(this);
}

//----------------------------------------------------------------------------
void vtkKWWindow::AddView(vtkKWView *v) 
{
  v->SetParentWindow(this);
  this->Views->AddItem(v);
}

//----------------------------------------------------------------------------
void vtkKWWindow::RemoveView(vtkKWView *v) 
{
  v->SetParentWindow(NULL);
  this->Views->RemoveItem(v);
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
void vtkKWWindow::SetSelectedView(vtkKWView *_arg)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting SelectedView to " << _arg ); 
  if (this->SelectedView != _arg) 
    { 
    if (this->SelectedView != NULL) 
      {
      this->SelectedView->Deselect(this);
      this->SelectedView->UnRegister(this); 
      }
    this->SelectedView = _arg; 
    if (this->SelectedView != NULL) 
      { 
      this->SelectedView->Register(this); 
      this->SelectedView->Select(this);
      } 
    this->Modified(); 
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
      this->GetApplication()->GetWindows()->GetNumberOfItems() <= 1)
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

  vtkKWView *v;

  // Give each view a chance to close
  this->Views->InitTraversal();
  while ((v = this->Views->GetNextKWView()))
    {
    v->Close();
    }

  // Close this window in the application. The
  // application will exit if there are no more windows.
  this->GetApplication()->Close(this);
}

//----------------------------------------------------------------------------
void vtkKWWindow::SaveWindowGeometry()
{
  if (this->IsCreated())
    {
    this->Script("wm geometry %s", this->GetWidgetName());

    this->GetApplication()->SetRegisteryValue(
      2, "Geometry", VTK_KW_WINDOW_GEOMETRY_REG_KEY, "%s", 
      this->GetApplication()->GetMainInterp()->result);

    this->GetApplication()->SetRegisteryValue(
      2, "Geometry", VTK_KW_WINDOW_FRAME1_SIZE_REG_KEY, "%d", 
      this->MiddleFrame->GetFrame1Size());
    }
}

//----------------------------------------------------------------------------
void vtkKWWindow::RestoreWindowGeometry()
{
  if (this->IsCreated())
    {
    if (this->GetApplication()->HasRegisteryValue(
          2, "Geometry", VTK_KW_WINDOW_GEOMETRY_REG_KEY))
      {
      char geometry[40];
      if (this->GetApplication()->GetRegisteryValue(
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

    if (this->GetApplication()->HasRegisteryValue(
          2, "Geometry", VTK_KW_WINDOW_FRAME1_SIZE_REG_KEY))
      {
      int reg_size = this->GetApplication()->GetIntRegisteryValue(
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
  vtkKWView *v;
  
  this->Views->InitTraversal();
  while ((v = this->Views->GetNextKWView()))
    {
    v->Render();
    }
}

//----------------------------------------------------------------------------
void vtkKWWindow::DisplayAbout()
{
  this->GetApplication()->DisplayAbout(this);
}

//----------------------------------------------------------------------------
void vtkKWWindow::SetStatusText(const char *text)
{
  this->StatusLabel->SetLabel(text);
}

//----------------------------------------------------------------------------
const char *vtkKWWindow::GetStatusText()
{
  return this->StatusLabel->GetLabel();
}

//----------------------------------------------------------------------------
vtkKWMenu *vtkKWWindow::GetMenuEdit()
{
  if (this->MenuEdit)
    {
    return this->MenuEdit;
    }

  if ( !this->IsCreated() )
    {
    return 0;
    }
  
  this->MenuEdit = vtkKWMenu::New();
  this->MenuEdit->SetParent(this->GetMenu());
  this->MenuEdit->SetTearOff(0);
  this->MenuEdit->Create(this->GetApplication(),"");
  // Make sure Edit menu is next to file menu
  this->Menu->InsertCascade(1, "Edit", this->MenuEdit, 0);
  return this->MenuEdit;
}

//----------------------------------------------------------------------------
vtkKWMenu *vtkKWWindow::GetMenuView()
{
  if (this->MenuView)
    {
    return this->MenuView;
    }

  if ( !this->IsCreated() )
    {
    return 0;
    }

  this->MenuView = vtkKWMenu::New();
  this->MenuView->SetParent(this->GetMenu());
  this->MenuView->SetTearOff(0);
  this->MenuView->Create(this->GetApplication(), "");
  // make sure Help menu is on the right
  if (this->MenuEdit)
    { 
    this->Menu->InsertCascade(2, "View", this->MenuView, 0);
    }
  else
    {
    this->Menu->InsertCascade(1, "View", this->MenuView, 0);
    }
  
  return this->MenuView;
}

//----------------------------------------------------------------------------
vtkKWMenu *vtkKWWindow::GetMenuWindow()
{
  if (this->MenuWindow)
    {
    return this->MenuWindow;
    }

  if ( !this->IsCreated() )
    {
    return 0;
    }

  this->MenuWindow = vtkKWMenu::New();
  this->MenuWindow->SetParent(this->GetMenu());
  this->MenuWindow->SetTearOff(0);
  this->MenuWindow->Create(this->GetApplication(), "");
  // make sure Help menu is on the right
  if (this->MenuEdit)
    { 
    this->Menu->InsertCascade(1, "Window", this->MenuWindow, 0);
    }
  else
    {
    this->Menu->InsertCascade(2, "Window", this->MenuWindow, 0);
    }
  
  return this->MenuWindow;
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
                   this->GetMenuWindow()->GetWidgetName(),
                   VTK_KW_HIDE_PROPERTIES_LABEL);
      }
    }
  else
    {
    if (this->GetPropertiesVisiblity())
      {
      this->MiddleFrame->Frame1VisibilityOff();
      this->Script("%s entryconfigure 0 -label {%s}",
                   this->GetMenuWindow()->GetWidgetName(),
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
void vtkKWWindow::InstallMenu(vtkKWMenu* menu)
{ 
  this->Script("%s configure -menu %s", this->GetWidgetName(),
               menu->GetWidgetName());  
}

//----------------------------------------------------------------------------
void vtkKWWindow::UnRegister(vtkObjectBase *o)
{
  if (!this->DeletingChildren)
    {
    // delete the children if we are about to be deleted
    if (this->ReferenceCount == 
        (this->Views->GetNumberOfItems() + 1 +
         (this->HasChildren() ? this->GetChildren()->GetNumberOfItems() : 0)))
      {
      if (!(this->Views->IsItemPresent((vtkKWView *)o) ||
            (this->HasChildren() && 
             this->GetChildren()->IsItemPresent((vtkKWWidget *)o))))
        {
        vtkKWWidget *child;
        vtkKWView *v;
        
        this->DeletingChildren = 1;
        if (this->HasChildren())
          {
          vtkKWWidgetCollection *children = this->GetChildren();
          children->InitTraversal();
          while ((child = children->GetNextKWWidget()))
            {
            child->SetParent(NULL);
            }
          }
        // deselect if required
        if (this->SelectedView)
          {
          this->SetSelectedView(NULL);
          }
        this->Views->InitTraversal();
        while ((v = this->Views->GetNextKWView()))
          {
          v->SetParentWindow(NULL);
          }
        this->DeletingChildren = 0;
        }
      }
    }
  
  this->Superclass::UnRegister(o);
}

//----------------------------------------------------------------------------
void vtkKWWindow::LoadScript()
{
  vtkKWLoadSaveDialog* loadScriptDialog = vtkKWLoadSaveDialog::New();
  this->RetrieveLastPath(loadScriptDialog, "LoadScriptLastPath");
  loadScriptDialog->SetParent(this);
  loadScriptDialog->Create(this->GetApplication(),"");
  loadScriptDialog->SaveDialogOff();
  loadScriptDialog->SetTitle("Load Script");
  loadScriptDialog->SetDefaultExtension(this->ScriptExtension);
  ostrstream filetypes;
  filetypes << "{{" << this->ScriptType << " Scripts} {" 
            << this->ScriptExtension << "}} {{All Files} {.*}}" << ends;
  loadScriptDialog->SetFileTypes(filetypes.str());
  filetypes.rdbuf()->freeze(0);

  int enabled = this->GetEnabled();
  this->SetEnabled(0);
  if (loadScriptDialog->Invoke() && 
      vtkString::Length(loadScriptDialog->GetFileName()) > 0)
    {
    FILE *fin = fopen(loadScriptDialog->GetFileName(), "r");
    if (!fin)
      {
      vtkWarningMacro("Unable to open script file!");
      }
    else
      {
      this->SaveLastPath(loadScriptDialog, "LoadScriptLastPath");
      this->LoadScript(loadScriptDialog->GetFileName());
      }
    }
  this->SetEnabled(enabled);
  loadScriptDialog->Delete();
}

//----------------------------------------------------------------------------
void vtkKWWindow::LoadScript(const char *path)
{
  this->Script("set InitialWindow %s", this->GetTclName());
  this->GetApplication()->LoadScript(path);
}

//----------------------------------------------------------------------------
void vtkKWWindow::CreateStatusImage()
{
  int x, y;
  Tk_PhotoHandle photo;
  Tk_PhotoImageBlock block;
  
  block.width = 128;
  block.height = 34;
  block.pixelSize = 3;
  block.pitch = block.width*block.pixelSize;
  block.offset[0] = 0;
  block.offset[1] = 1;
  block.offset[2] = 2;
#if (TK_MAJOR_VERSION == 8) && (TK_MINOR_VERSION >= 4)
  block.offset[3] = 3;
#endif
  block.pixelPtr = new unsigned char [block.pitch*block.height];

  photo = Tk_FindPhoto(this->GetApplication()->GetMainInterp(),
                       this->StatusImageName);
  if (!photo)
    {
    vtkWarningMacro("error looking up color ramp image");
    return;
    }

  Tk_PhotoSetSize(photo, block.width, block.height);
  
  unsigned char *pp = block.pixelPtr;
  float *lp = KITLOGO + 33*128*3;
  for (y = 0; y < 34; y++)
    {
    for (x = 0; x < 128; x++)
      {
      pp[0] = (unsigned char)(*lp*255.0);
      lp++;
      pp[1] = (unsigned char)(*lp*255.0);
      lp++;
      pp[2] = (unsigned char)(*lp*255.0);
      lp++;
      pp += block.pixelSize;
      }
    lp = lp - 2*128*3;
    }
  
#if (TK_MAJOR_VERSION == 8) && (TK_MINOR_VERSION >= 4) && !defined(USE_COMPOSITELESS_PHOTO_PUT_BLOCK)
  Tk_PhotoPutBlock(photo, &block, 0, 0, block.width, block.height, TK_PHOTO_COMPOSITE_SET);
#else
  Tk_PhotoPutBlock(photo, &block, 0, 0, block.width, block.height);
#endif
  delete [] block.pixelPtr;
}

//----------------------------------------------------------------------------
void vtkKWWindow::SetNumberOfRecentFiles(vtkIdType _arg)
{
  if (this->NumberOfRecentFiles == _arg ||
      _arg < 4 || _arg > VTK_KW_RECENT_FILES_MAX)
    {
    return;
    }

  this->NumberOfRecentFiles = _arg;
  this->Modified();

  this->UpdateRecentFilesMenu();
  this->StoreRecentFilesToRegistery();
}

//----------------------------------------------------------------------------
void vtkKWWindow::StoreRecentFilesToRegistery()
{
  char KeyNameP[20], CmdNameP[20];

  // Store all recent files (remove all others, up to the max)

  unsigned int i;
  for (i = 0; i < VTK_KW_RECENT_FILES_MAX; i++)
    {
    sprintf(KeyNameP, "File%d", i);
    sprintf(CmdNameP, "File%dCmd", i);

    int delete_key = 1;
    if (this->RecentFilesVector)
      {
      vtkKWRecentFileEntry *rfe = 0;
      if (this->RecentFilesVector->GetItem(i, rfe) == VTK_OK && rfe)
        {
        this->GetApplication()->SetRegisteryValue(
          1, "MRU", KeyNameP, rfe->GetFileName());
        this->GetApplication()->SetRegisteryValue(
          1, "MRU", CmdNameP, rfe->GetCommand());
        delete_key = 0;
        }
      }

    if (delete_key)
      {
      this->GetApplication()->DeleteRegisteryValue(1, "MRU", KeyNameP);
      this->GetApplication()->DeleteRegisteryValue(1, "MRU", CmdNameP);
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWWindow::AddRecentFilesMenu(
  const char *menuEntry, vtkKWObject *target, const char *label, int underline)
{
  if (!this->IsCreated() || !label || !this->MenuFile)
    {
    return;
    }

  // Create the menu if not done already

  if (!this->MenuRecentFiles)
    {
    this->MenuRecentFiles = vtkKWMenu::New();
    }

  if (!this->MenuRecentFiles->IsCreated())
    {
    this->MenuRecentFiles->SetParent(this->MenuFile);
    this->MenuRecentFiles->SetTearOff(0);
    this->MenuRecentFiles->Create(this->GetApplication(), "");
    }

  // Remove the menu if already there (in case that function was used to
  // move the menu)

  if (this->MenuFile->HasItem(label))
    {
    this->MenuFile->DeleteMenuItem(label);
    }

  // Find where to insert

  int insert_idx;
  if (!menuEntry || !this->MenuFile->HasItem(menuEntry))
    {
    insert_idx = this->GetFileMenuIndex();
    }
  else
    {
    insert_idx = this->MenuFile->GetIndex(menuEntry) - 1;
    }
  
  this->MenuFile->InsertCascade(
    insert_idx, label, this->MenuRecentFiles, underline);

  // Fill the recent files vector with recent files stored in registery

  char KeyNameP[20], CmdNameP[20], FileName[1024], Command[1024];

  int i;
  for (i = VTK_KW_RECENT_FILES_MAX - 1; i >= 0; i--)
    {
    sprintf(KeyNameP, "File%d", i);
    sprintf(CmdNameP, "File%dCmd", i);
    if (this->GetApplication()->GetRegisteryValue(
          1, "MRU", KeyNameP, FileName) &&
        this->GetApplication()->GetRegisteryValue(
          1, "MRU", CmdNameP, Command) &&
        vtkString::Length(FileName) >= 1)
      {
      this->InsertRecentFile(FileName, target, Command);
      }
    }

  // Build the menu

  this->UpdateRecentFilesMenu();
}

//----------------------------------------------------------------------------
void vtkKWWindow::UpdateRecentFilesMenu()
{ 
  if (!this->MenuRecentFiles || !this->RecentFilesVector)
    {
    return;
    }
  
  this->MenuRecentFiles->DeleteAllMenuItems();

  // Fill the menu

  vtkIdType cc, idx = 0;
  for (cc = 0; cc < this->NumberOfRecentFiles; cc++)
    {
    vtkKWRecentFileEntry *rfe = 0;
    if (this->RecentFilesVector->GetItem(cc, rfe) == VTK_OK && rfe &&
        rfe->GetFileName() && rfe->GetTarget() && rfe->GetCommand())
      {
      char *short_file = vtkString::Duplicate(rfe->GetFileName());
      vtkString::CropString(short_file, 40);
      ostrstream label;
      ostrstream cmd;
      label << idx << " " << short_file << ends;
      cmd << rfe->GetCommand() << " {" << rfe->GetFileName() << "}" << ends;
      this->MenuRecentFiles->AddCommand(
        label.str(), rfe->GetTarget(), cmd.str(), (idx < 10 ? 0 : -1),
        rfe->GetFileName());
      idx++;
      label.rdbuf()->freeze(0);
      cmd.rdbuf()->freeze(0);
      delete [] short_file;
      }
    }

  // Update the menu state to disable/enable this menu

  this->UpdateMenuState();
}

//----------------------------------------------------------------------------
void vtkKWWindow::AddRecentFile(const char *name, vtkKWObject *target,
                                const char *command)
{  
  const char* filename = this->GetApplication()->ExpandFileName(name);
  this->InsertRecentFile(filename, target, command);
  this->UpdateRecentFilesMenu();
  this->StoreRecentFilesToRegistery();
}

//----------------------------------------------------------------------------
void vtkKWWindow::InsertRecentFile(const char *filename, 
                                   vtkKWObject *target, 
                                   const char *command)
{
  // If not vector yet, create it

  if (!this->RecentFilesVector)
    {
    this->RecentFilesVector = vtkVector<vtkKWRecentFileEntry*>::New();
    }

  // Find if already inserted (and delete it)

  vtkKWRecentFileEntry *found = 0;

  vtkIdType cc;
  for (cc = 0; cc < this->RecentFilesVector->GetNumberOfItems(); cc++)
    {
    vtkKWRecentFileEntry *rfe = 0;
    if (this->RecentFilesVector->GetItem(cc, rfe) == VTK_OK && rfe &&
        rfe->Same(filename, target, command))
      {
      found = rfe;
      this->RecentFilesVector->RemoveItem(cc);
      break;
      }
    }

  // Not found, create new one

  if (!found)
    {
    found = new vtkKWRecentFileEntry;
    found->SetFileName(filename);
    found->SetTarget(target);
    found->SetCommand(command) ;
   }

  // Prepend it to array  

  this->RecentFilesVector->PrependItem(found);
}

//----------------------------------------------------------------------------
int vtkKWWindow::GetFileMenuIndex()
{
  if ( !this->IsCreated() )
    {
    return 0;
    }

  // First find the print-related menu commands

  if (this->GetMenuFile()->HasItem(VTK_KW_PAGE_SETUP_MENU_LABEL))
    {
    return this->GetMenuFile()->GetIndex(VTK_KW_PAGE_SETUP_MENU_LABEL);
    }

  // Otherwise find Close or Exit if Close was removed

  int clidx;
  if (this->GetMenuFile()->HasItem("Close"))
    {
    clidx = this->GetMenuFile()->GetIndex("Close");  
    }
  else
    {
    clidx = this->GetMenuFile()->GetIndex("Exit");  
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

  if (this->MenuHelp->HasItem("About*"))
    {
    return this->MenuHelp->GetIndex("About*") - 1;
    }

  return this->MenuHelp->GetNumberOfItems();
}

//----------------------------------------------------------------------------
int vtkKWWindow::ExitDialog()
{
  this->GetApplication()->SetBalloonHelpWidget(0);
  if ( this->ExitDialogWidget )
    {
    return 1;
    }
  ostrstream title;
  title << "Exit " << this->GetApplication()->GetApplicationPrettyName() 
        << ends;
  char* ttl = title.str();
  ostrstream str;
  str << "Are you sure you want to exit " 
      << this->GetApplication()->GetApplicationPrettyName() << "?" << ends;
  char* msg = str.str();
  
  vtkKWMessageDialog *dlg2 = vtkKWMessageDialog::New();
  this->ExitDialogWidget = dlg2;
  dlg2->SetStyleToYesNo();
  dlg2->SetMasterWindow(this);
  dlg2->SetOptions(
     vtkKWMessageDialog::QuestionIcon | vtkKWMessageDialog::RememberYes |
     vtkKWMessageDialog::Beep | vtkKWMessageDialog::YesDefault );
  dlg2->SetDialogName(VTK_KW_EXIT_DIALOG_NAME);
  dlg2->Create(this->GetApplication(),"");
  dlg2->SetText( msg );
  dlg2->SetTitle( ttl );
  int ret = dlg2->Invoke();
  this->ExitDialogWidget = 0;
  dlg2->Delete();

  delete[] msg;
  delete[] ttl;
 
  vtkKWApplicationSettingsInterface *asi =  
    this->GetApplicationSettingsInterface();
  if (asi)
    {
    asi->Update();
    }
 
  return ret;
}

//----------------------------------------------------------------------------
float vtkKWWindow::GetFloatRegisteryValue(int level, const char* subkey, 
                                          const char* key)
{
  return this->GetApplication()->GetFloatRegisteryValue(level, subkey, key);
}

//----------------------------------------------------------------------------
int vtkKWWindow::GetIntRegisteryValue(int level, const char* subkey, 
                                      const char* key)
{
  return this->GetApplication()->GetIntRegisteryValue(level, subkey, key);
}

//----------------------------------------------------------------------------
void vtkKWWindow::SaveLastPath(vtkKWLoadSaveDialog *dialog, const char* key)
{
  //  "OpenDirectory"
  if ( dialog->GetLastPath() )
    {
    this->GetApplication()->SetRegisteryValue(
      1, "RunTime", key, dialog->GetLastPath());
    }
}

//----------------------------------------------------------------------------
void vtkKWWindow::RetrieveLastPath(vtkKWLoadSaveDialog *dialog, const char* key)
{
  char buffer[1024];
  if ( this->GetApplication()->GetRegisteryValue(1, "RunTime", key, buffer) )
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
  this->GetApplication()->SetRegisteryValue(
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
  if (this->GetApplication()->GetRegisteryValue(
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
int vtkKWWindow::BooleanRegisteryCheck(int level, 
                                       const char* subkey,
                                       const char* key, 
                                       const char* trueval)
{
  return this->GetApplication()->BooleanRegisteryCheck(level, subkey, key, trueval);
}


//----------------------------------------------------------------------------
void vtkKWWindow::WarningMessage(const char* message)
{
  vtkKWMessageDialog::PopupMessage(
    this->GetApplication(), this, "VTK Warning",
    message, vtkKWMessageDialog::WarningIcon);
  this->SetErrorIcon(2);
  this->InvokeEvent(vtkKWEvent::WarningMessageEvent, (void*)message);
}

//----------------------------------------------------------------------------
void vtkKWWindow::SetErrorIcon(int s)
{
  if (s) 
    {
    this->Script("pack %s -fill both -ipadx 4 -expand yes", 
                 this->TrayImageError->GetWidgetName());
    if ( s > 1 )
      {
      this->TrayImageError->SetImageOption(vtkKWIcon::ICON_SMALLERRORRED);
      }
    else
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
void vtkKWWindow::ErrorMessage(const char* message)
{
  vtkKWMessageDialog::PopupMessage(
    this->GetApplication(), this, "VTK Error",
    message, vtkKWMessageDialog::ErrorIcon);
  this->SetErrorIcon(2);
  this->InvokeEvent(vtkKWEvent::ErrorMessageEvent, (void*)message);
}

//----------------------------------------------------------------------------
void vtkKWWindow::ProcessErrorClick()
{
  this->SetErrorIcon(1);
}

//----------------------------------------------------------------------------
void vtkKWWindow::SetTitle (const char* _arg)
{
  if (this->Title == NULL && _arg == NULL) 
    { 
    return;
    }

  if (this->Title && _arg && (!strcmp(this->Title, _arg))) 
    {
    return;
    }

  if (this->Title) 
    { 
    delete [] this->Title; 
    }

  if (_arg)
    {
    this->Title = new char[strlen(_arg)+1];
    strcpy(this->Title, _arg);
    }
  else
    {
    this->Title = NULL;
    }

  this->Modified();

  if (this->IsCreated() && this->Title)
    {
    this->Script("wm title %s {%s}", this->GetWidgetName(), this->GetTitle());
    }
} 

//----------------------------------------------------------------------------
char* vtkKWWindow::GetTitle()
{
  if (!this->Title && 
      this->GetApplication() && 
      this->GetApplication()->GetApplicationName())
    {
    return this->GetApplication()->GetApplicationName();
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
    this->TclInteractor->Create(this->GetApplication());
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
void vtkKWWindow::UpdateToolbarAspect()
{
  if (!this->Toolbars)
    {
    return;
    }

  int flat_frame;
  if (this->GetApplication()->HasRegisteryValue(
    2, "RunTime", VTK_KW_TOOLBAR_FLAT_FRAME_REG_KEY))
    {
    flat_frame = this->GetApplication()->GetIntRegisteryValue(
      2, "RunTime", VTK_KW_TOOLBAR_FLAT_FRAME_REG_KEY);
    }
  else
    {
    flat_frame = vtkKWToolbar::GetGlobalFlatAspect();
    }

  int flat_buttons;
  if (this->GetApplication()->HasRegisteryValue(
    2, "RunTime", VTK_KW_TOOLBAR_FLAT_BUTTONS_REG_KEY))
    {
    flat_buttons = this->GetApplication()->GetIntRegisteryValue(
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

  this->PropagateEnableState(this->Toolbars);

  // Update the notebook

  this->PropagateEnableState(this->Notebook);

  // Update the Tcl interactor

  this->PropagateEnableState(this->TclInteractor);

  this->PropagateEnableState(this->SelectedView);
  this->PropagateEnableState(this->MiddleFrame);
  this->PropagateEnableState(this->StatusFrame);
  //this->PropagateEnableState(this->StatusLabel);
  this->PropagateEnableState(this->PropertiesParent);
  this->PropagateEnableState(this->ViewFrame);
  this->PropagateEnableState(this->MenuBarSeparatorFrame);

  // Do not disable the status image, it has not functionality attached 
  // anyway, and is used to display the application logo: disabling it 
  // makes it look ugly.
  //this->PropagateEnableState(this->StatusImage);

  // Given the state, can we close or not ?

  if (this->IsCreated())
    {
    if (this->Enabled)
      {
      this->Script("wm protocol %s WM_DELETE_WINDOW {%s Close}",
                   this->GetWidgetName(), this->GetTclName());
      }
    else
      {
      this->Script("wm protocol %s WM_DELETE_WINDOW "
                   "{%s SetStatusText \"Can not close while UI is disabled\"}",
                   this->GetWidgetName(), this->GetTclName());
      }
    }

  // Update the menus

  this->UpdateMenuState();
}

//----------------------------------------------------------------------------
void vtkKWWindow::UpdateMenuState()
{
  int menu_enabled = this->Enabled ? vtkKWMenu::Normal : vtkKWMenu::Disabled;

  if (this->Menu)
    {
    this->Menu->SetEnabled(this->Enabled);
    this->Menu->SetState(menu_enabled);
    }

  // Most Recent Files

  if (this->MenuRecentFiles)
    {
    this->MenuRecentFiles->SetEnabled(this->Enabled);
    if (this->MenuRecentFiles->IsCreated())
      {
      vtkKWMenu *parent = 
        vtkKWMenu::SafeDownCast(this->MenuRecentFiles->GetParent());
      if (parent)
        {
        int index = parent->GetCascadeIndex(this->MenuRecentFiles);
        if (index >= 0)
          {
          int nb_items = this->MenuRecentFiles->GetNumberOfItems();
          parent->SetState(
            index,  nb_items ? menu_enabled :vtkKWMenu::Disabled);
          }
        }
      }
    }

  // Update the About entry, since the pretty name also depends on the
  // limited edition mode

  if (this->MenuHelp)
    {
    int pos = this->MenuHelp->GetIndexOfCommand(this, "DisplayAbout");
    if (pos >= 0)
      {
      ostrstream label;
      label << "-label {About " 
            << this->GetApplication()->GetApplicationPrettyName() << "}"<<ends;
      this->MenuHelp->ConfigureItem(pos, label.str());
      label.rdbuf()->freeze(0);
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWWindow::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Menu: " << this->GetMenu() << endl;
  os << indent << "MenuFile: " << this->GetMenuFile() << endl;
  os << indent << "Notebook: " << this->GetNotebook() << endl;
  os << indent << "NumberOfRecentFiles: " << this->GetNumberOfRecentFiles() 
     << endl;
  os << indent << "PrintTargetDPI: " << this->GetPrintTargetDPI() << endl;
  os << indent << "ProgressGauge: " << this->GetProgressGauge() << endl;
  os << indent << "PromptBeforeClose: " << this->GetPromptBeforeClose() 
     << endl;
  os << indent << "PropertiesParent: " << this->GetPropertiesParent() << endl;
  os << indent << "ScriptExtension: " << this->GetScriptExtension() << endl;
  os << indent << "ScriptType: " << this->GetScriptType() << endl;
  os << indent << "SelectedView: " << this->GetSelectedView() << endl;
  os << indent << "SupportHelp: " << this->GetSupportHelp() << endl;
  os << indent << "SupportPrint: " << this->GetSupportPrint() << endl;
  os << indent << "StatusFrame: " << this->GetStatusFrame() << endl;
  os << indent << "ViewFrame: " << this->GetViewFrame() << endl;
  os << indent << "WindowClass: " << this->GetWindowClass() << endl;  
  os << indent << "TclInteractor: " << this->GetTclInteractor() << endl;
  os << indent << "Title: " 
     << ( this->GetTitle() ? this->GetTitle() : "(none)" ) << endl;  
  os << indent << "Toolbars: " << this->GetToolbars() << endl;
}


