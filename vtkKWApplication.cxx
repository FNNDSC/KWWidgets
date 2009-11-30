/*=========================================================================

  Module:    vtkKWApplication.cxx,v

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifdef __APPLE__
#include <AvailabilityMacros.h>
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1040
#include <Carbon/Carbon.h>
#define Cursor X11Cursor 
#endif
#endif // __APPLE__

#include "vtkKWApplication.h"

#include "vtkObjectFactory.h"
#include "vtkOutputWindow.h"
#include "vtkTclUtil.h"

#include "vtkKWBalloonHelpManager.h"
#include "vtkKWColorPickerDialog.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWEvent.h"
#include "vtkKWFrame.h"
#include "vtkKWIcon.h"
#include "vtkKWInternationalization.h"
#include "vtkKWLabel.h"
#include "vtkKWLanguage.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWLogDialog.h"
#include "vtkKWLogWidget.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWObject.h"
#include "vtkKWOptionDataBase.h"
#include "vtkKWPushButton.h"
#include "vtkKWRegistryHelper.h"
#include "vtkKWSeparator.h"
#include "vtkKWSplashScreen.h"
#include "vtkKWTclInteractor.h"
#include "vtkKWText.h"
#include "vtkKWTextWithScrollbars.h"
#include "vtkKWTheme.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWTkcon.h"
#include "vtkKWToolbar.h"
#include "vtkKWWindowBase.h"

#include <stdarg.h>

#include <vtksys/SystemTools.hxx>
#include <vtksys/ios/sstream>
#include <vtksys/stl/vector>
#include <vtksys/stl/algorithm>

#include "Resources/KWWidgets.rc.h"
#include "vtkKWWidgetsConfigure.h"
#include "vtkKWWidgetsBuildConfigure.h"

#include "Resources/vtkKWMiscResources.h"

#include "vtkToolkits.h"

static Tcl_Interp *Et_Interp = 0;

#ifdef KWWidgets_USE_TKDND
#include "Utilities/tkdnd/vtkKWTkDnDInit.h"
#endif

#ifdef KWWidgets_USE_TKTREECTRL
#include "Utilities/TkTreeCtrl/vtkKWTkTreeCtrlInit.h"
#endif

#ifdef _WIN32
#include "vtkWindows.h"
#include <shellapi.h>
#include <process.h>
#include <mapi.h>
#ifdef KWWidgets_USE_HTML_HELP
#include <htmlhelp.h>
#endif
#include "Utilities/ApplicationIcon/vtkKWSetApplicationIconTclCommand.h"
#endif

#ifdef __APPLE__
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1040
#undef Cursor
#endif
#endif // __APPLE__

const char *vtkKWApplication::ExitDialogName = "ExitApplication";
const char *vtkKWApplication::SendErrorLogDialogName = "SendErrorLog";
const char *vtkKWApplication::BalloonHelpVisibilityRegKey = "ShowBalloonHelp";
const char *vtkKWApplication::SaveUserInterfaceGeometryRegKey = "SaveUserInterfaceGeometry";
const char *vtkKWApplication::SplashScreenVisibilityRegKey = "ShowSplashScreen";
const char *vtkKWApplication::PrintTargetDPIRegKey = "PrintTargetDPI";
const char *vtkKWApplication::MostRecentVersionLaunchedRegKey = "MostRecentVersionLaunched";

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWApplication );
vtkCxxRevisionMacro(vtkKWApplication, "$Revision: 1.356 $");

extern "C" int Kwwidgets_Init(Tcl_Interp *interp);

// Initialize VTK Tcl. If we have not wrapped VTK for Tcl, then
// we can only rely on Common and the two renderwidgets

extern "C" int Vtkcommontcl_Init(Tcl_Interp *interp);

#ifdef KWWidgets_BUILD_VTK_WIDGETS
extern "C" int Vtktkrenderwidget_Init(Tcl_Interp *interp);
extern "C" int Vtktkimageviewerwidget_Init(Tcl_Interp *interp);
#ifdef VTK_WRAP_TCL
extern "C" int Vtkfilteringtcl_Init(Tcl_Interp *interp);
extern "C" int Vtkimagingtcl_Init(Tcl_Interp *interp);
extern "C" int Vtkgraphicstcl_Init(Tcl_Interp *interp);
extern "C" int Vtkiotcl_Init(Tcl_Interp *interp);
#ifdef VTK_USE_RENDERING
extern "C" int Vtkrenderingtcl_Init(Tcl_Interp *interp);
extern "C" int Vtkvolumerenderingtcl_Init(Tcl_Interp *interp);
extern "C" int Vtkhybridtcl_Init(Tcl_Interp *interp);
extern "C" int Vtkwidgetstcl_Init(Tcl_Interp *interp);
#endif // VTK_USE_RENDERING
#ifdef VTK_USE_PARALLEL
extern "C" int Vtkparalleltcl_Init(Tcl_Interp *interp);
#endif // VTK_USE_PARALLEL
#endif // VTK_WRAP_TCL
#endif // KWWidgets_BUILD_VTK_WIDGETS

#ifdef KWWidgets_USE_INCR_TCL
extern "C" int Itcl_Init(Tcl_Interp *interp);
extern "C" int Itk_Init(Tcl_Interp *interp);
#endif


//----------------------------------------------------------------------------
class KWWidgets_EXPORT vtkKWOutputWindow : public vtkOutputWindow
{
public:
  vtkTypeMacro(vtkKWOutputWindow,vtkOutputWindow);
  static vtkKWOutputWindow* New();

  void DisplayDebugText(const char* t)
    { 
      this->Application->DebugMessage(t); 
      if (this->PromptUser)
        {
        this->Application->DisplayLogDialog(NULL);
        }
    }
  void DisplayWarningText(const char* t)
    { 
      this->Application->WarningMessage(t); 
      if (this->PromptUser)
        {
        this->Application->DisplayLogDialog(NULL);
        }
    }
  void DisplayErrorText(const char* t)
    { 
      this->Application->ErrorMessage(t); 
      if (this->PromptUser)
        {
        this->Application->DisplayLogDialog(NULL);
        }
    }
  void DisplayText(const char* t)
    { 
      this->Application->InformationMessage(t); 
      if (this->PromptUser)
        {
        this->Application->DisplayLogDialog(NULL);
        }
    }
  void DisplayGenericWarningText(const char* t)
    { 
      this->DisplayWarningText(t); 
    }
  
  void SetApplication(vtkKWApplication *app)
    { 
      this->Application = app; 
    }

protected:
  vtkKWOutputWindow()
    { 
      this->Application = NULL; 
    }
  vtkKWApplication *Application;

private:
  vtkKWOutputWindow(const vtkKWOutputWindow&);
  void operator=(const vtkKWOutputWindow&);
};

//----------------------------------------------------------------------------
class vtkKWApplicationInternals
{
public:
  typedef vtksys_stl::vector<vtkKWWindowBase*> WindowsContainer;
  typedef vtksys_stl::vector<vtkKWWindowBase*>::iterator WindowsContainerIterator;

  WindowsContainer Windows;

  // Some temporary storage var that do not need to be exposed in the .h

  vtksys_stl::string VersionNameTemp;
  vtksys_stl::string LimitedEditionModeNameTemp;
  vtksys_stl::string RegistryVersionNameTemp;

  // For ::PutEnv

  class DeletingCharVector : public vtksys_stl::vector<char*>
  {
  public:
    ~DeletingCharVector()
      {
        for(vtksys_stl::vector<char*>::iterator i = this->begin();
            i != this->end(); ++i)
          {
          delete []*i;
          }
      }
  };

  vtkOutputWindow *PreviousOutputWindow;
  vtkKWOutputWindow *LogOutputWindow;
};

vtkStandardNewMacro(vtkKWOutputWindow);

//----------------------------------------------------------------------------
vtkKWApplication::vtkKWApplication()
{
  /* IMPORTANT:
     Do *ALL* simple inits *first* before a possible
     early return due to an error condition. Avoids
     crashing during destructor when some members have
     not been properly NULL initialized...
  */

  this->Internals                 = NULL;
  this->MajorVersion              = 1;
  this->MinorVersion              = 0;
  this->Name                      = NULL;
  this->VersionName               = NULL;
  this->ReleaseName               = NULL;
  this->PrettyName                = NULL;
  this->ReleaseMode               = 0;
  this->RegistryVersionName       = NULL;
  this->LimitedEditionMode        = 0;
  this->LimitedEditionModeName    = NULL;
  this->HelpDialogStartingPage    = NULL;
  this->TutorialStartingPage    = NULL;
  this->InstallationDirectory     = NULL;
  this->UserDataDirectory         = NULL;
  this->EmailFeedbackAddress      = NULL;
  this->InExit                    = 0;
  this->ExitStatus                = 0;
  this->ExitAfterLoadScript       = 0;
  this->PromptBeforeExit          = 1;
  this->SendErrorLogBeforeExit    = 0;
  this->DialogUp                  = 0;
  this->SaveUserInterfaceGeometry = 1;
  this->RegistryHelper            = NULL;
  this->OptionDataBase            = NULL;
  this->RegistryLevel             = 10;
  this->BalloonHelpManager        = NULL;
  this->CharacterEncoding         = VTK_ENCODING_UNKNOWN;
  this->AboutDialog               = NULL;
  this->AboutDialogImage          = NULL;
  this->AboutRuntimeInfo          = NULL;
  this->SplashScreen              = NULL;
  this->SupportSplashScreen       = 0;
  this->SplashScreenVisibility    = 1;
  this->PrintTargetDPI            = 100.0;
  this->Theme                     = NULL;
  this->LogDialog                 = NULL;
  this->TclInteractor             = NULL;
  this->ColorPickerDialog             = NULL;

  /* IMPORTANT:
     Do *NOT* call anything that retrieves the application's TclName.
     Doing so will force a mapping between a Tcl object and this class with
     the wrong command function, i.e. a Tcl object will be created and
     mapped to a vtkKWApplication Tcl command. As a result, any subclass
     vtkFoo of vtkKWApplication will appear to be a vtkKWApplication instead
     of a vtkFoo, and will be missing all the corresponding wrapped methods.
  */

  // Setup Tcl

  if (!this->GetMainInterp())
    {
    vtkErrorMacro(
      "Interpreter not set. This probably means that Tcl was not "
      "initialized properly...");
    return;
    }

  // Instantiate the PIMPL Encapsulation for STL containers

  this->Internals = new vtkKWApplicationInternals;

  // Output win

  this->InstallOutputWindow();

  // Application name and version

  // Try to find if we are running from a script and set the application name
  // accordingly. Otherwise try to find the executable name.

  vtksys_stl::string script =
    vtksys::SystemTools::GetFilenameWithoutExtension(
      vtksys::SystemTools::GetFilenameName(
        vtkKWTkUtilities::GetCurrentScript(this->GetMainInterp())));
  if (script.size())
    {
    this->Name = vtksys::SystemTools::DuplicateString(script.c_str());
    }
  else
    {
    const char *nameofexec = Tcl_GetNameOfExecutable();
    if (nameofexec && vtksys::SystemTools::FileExists(nameofexec))
      {
      vtksys_stl::string filename = 
        vtksys::SystemTools::GetFilenameName(nameofexec);
      vtksys_stl::string filenamewe = 
        vtksys::SystemTools::GetFilenameWithoutExtension(filename);
      if (!vtksys::SystemTools::StringStartsWith(filenamewe.c_str(), "wish") &&
          !vtksys::SystemTools::StringStartsWith(filenamewe.c_str(), "tclsh"))
        {
        this->Name = 
          vtksys::SystemTools::DuplicateString(filenamewe.c_str());
        }
      }
    }

  // Still no name... use default...

  if (!this->Name)
    {
    this->Name = 
      vtksys::SystemTools::DuplicateString("Sample Application");
    }

  // Encoding...

  this->SetCharacterEncoding(VTK_ENCODING_ISO_8859_1);

  vtksys::SystemTools::EnableMSVCDebugHook();

  if (this->ReleaseMode)
    {
    vtkObject::GlobalWarningDisplayOff();
    }
}

//----------------------------------------------------------------------------
vtkKWApplication::~vtkKWApplication()
{
  this->PrepareForDelete();

  delete this->Internals;
  this->Internals = NULL;

  vtkObjectFactory::UnRegisterAllFactories();

  this->SetLimitedEditionModeName(NULL);
  this->SetName(NULL);
  this->SetVersionName(NULL);
  this->SetReleaseName(NULL);
  this->SetPrettyName(NULL);
  this->SetRegistryVersionName(NULL);
  this->SetInstallationDirectory(NULL);
  this->SetUserDataDirectory(NULL);
  this->SetEmailFeedbackAddress(NULL);
  this->SetHelpDialogStartingPage(NULL);
  this->SetTutorialStartingPage(NULL);

  if (this->RegistryHelper )
    {
    this->RegistryHelper->Delete();
    this->RegistryHelper = NULL;
    }

  if (this->OptionDataBase )
    {
    this->OptionDataBase->Delete();
    this->OptionDataBase = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkKWApplication::PrepareForDelete()
{
  if (this->AboutDialogImage)
    {
    this->AboutDialogImage->Delete();
    this->AboutDialogImage = NULL;
    }

  if (this->AboutRuntimeInfo)
    {
    this->AboutRuntimeInfo->Delete();
    this->AboutRuntimeInfo = NULL;
    }
    
  if (this->AboutDialog)
    {
    this->AboutDialog->Delete();
    this->AboutDialog = NULL;
    }

  if (this->SplashScreen)
    {
    this->SplashScreen->Delete();
    this->SplashScreen = NULL;
    }

  if (this->TclInteractor)
    {
    this->TclInteractor->SetMasterWindow(NULL);
    this->TclInteractor->Delete();
    this->TclInteractor = NULL;
    }

  if (this->ColorPickerDialog)
    {
    this->ColorPickerDialog->SetMasterWindow(NULL);
    this->ColorPickerDialog->Delete();
    this->ColorPickerDialog = NULL;
    }

  // vtkKWOutputWindow is actually using the LogDialog, so before deleting
  // it, just switch back to the previous output window

  this->RestoreOutputWindow();

  if (this->LogDialog)
    {
    this->LogDialog->Delete();
    this->LogDialog = NULL;
    }

  if (this->BalloonHelpManager )
    {
    this->BalloonHelpManager->Delete();
    this->BalloonHelpManager = NULL;
    }

  if (this->GetMainInterp())
    {
    vtkKWTkUtilities::CancelAllTimerHandlers(this->GetMainInterp());
    }
}

//----------------------------------------------------------------------------
void vtkKWApplication::InstallOutputWindow()
{ 
  this->Internals->PreviousOutputWindow = vtkOutputWindow::GetInstance();
  this->Internals->PreviousOutputWindow->Register(this);

  this->Internals->LogOutputWindow = vtkKWOutputWindow::New();
  this->Internals->LogOutputWindow->SetApplication(this);
  vtkOutputWindow::SetInstance(this->Internals->LogOutputWindow);
}

//----------------------------------------------------------------------------
void vtkKWApplication::RestoreOutputWindow()
{ 
  if (this->Internals->LogOutputWindow)
    {
    if (vtkOutputWindow::GetInstance() == this->Internals->LogOutputWindow)
      {
      vtkOutputWindow::SetInstance(this->Internals->PreviousOutputWindow);
      }
    this->Internals->PreviousOutputWindow->Delete();
    this->Internals->LogOutputWindow->Delete();
    this->Internals->LogOutputWindow = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkKWApplication::SetApplication(vtkKWApplication*) 
{ 
  vtkErrorMacro( << "Do not set the Application on an Application" << endl); 
}

//----------------------------------------------------------------------------
int vtkKWApplication::AddWindow(vtkKWWindowBase *win)
{
  if (this->Internals)
    {
    this->Internals->Windows.push_back(win);
    win->Register(this);
    if (!win->GetApplication())
      {
      win->SetApplication(this);
      }
    return 1;
    }
  return 0;
}

//----------------------------------------------------------------------------
int vtkKWApplication::RemoveWindow(vtkKWWindowBase *win)
{
  // If this is the last window, go straight to Exit.
  // This will give the app a chance to ask the user for confirmation.
  // It is likely that this method has been called by vtkKWWindowBase::Close().
  // In that case, Exit() will call vtkKWWindowBase::Close() *again*, and we
  // will be back here, but this won't infinite loop since this->Exit()
  // will return false (as we are already "exiting", check this->InExit)

  if (this->GetNumberOfWindows() <= 1)
    {
    // We managed to exit right away
    if (this->Exit())
      {
      return 1;
      }
    // we could not exit, but not because we were already exiting, but
    // because of other factors like errors or user not confirming
    if (!this->InExit) 
      {
      return 0;
      }
    }

  if (this->Internals && win)
    {
    vtkKWApplicationInternals::WindowsContainerIterator it = 
      vtksys_stl::find(this->Internals->Windows.begin(),
                       this->Internals->Windows.end(),
                      win);
    if (it != this->Internals->Windows.end())
      {
      win->Withdraw();
      win->PrepareForDelete();
      win->UnRegister(this);
      this->Internals->Windows.erase(it);
      return 1;
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
vtkKWWindowBase* vtkKWApplication::GetNthWindow(int rank)
{
  if (this->Internals && rank >= 0 && rank < this->GetNumberOfWindows())
    {
    return this->Internals->Windows[rank];
    }
  return NULL;
}

//----------------------------------------------------------------------------
int vtkKWApplication::GetNumberOfWindows()
{
  if (this->Internals)
    {
    return (int)this->Internals->Windows.size();
    }
  return 0;
}

//----------------------------------------------------------------------------
int vtkKWApplication::GetNumberOfWindowsMapped()
{
  int i, nb_windows = this->GetNumberOfWindows(), count = 0;
  for (i = 0; i < nb_windows; i++)
    {
    vtkKWWindowBase *win = this->GetNthWindow(i);
    if (win && win->IsMapped())
      {
      ++count;
      }
    }
  return count;
}

//----------------------------------------------------------------------------
int vtkKWApplication::Exit()
{
  // Avoid a recursive exit.

  if (this->InExit)
    {
    return 0;
    }

  // Send error log

  if (this->SendErrorLogBeforeExit)
    {
    this->SendErrorLog();
    }

  // If a dialog is still up, complain and bail

  if (this->IsDialogUp())
    {
    vtkKWTkUtilities::Bell(this->GetMainInterp());
    return 0;
    }

  // Prompt confirmation if needed
  // Let's just use the first window here, if any, to center the dialog

  if (this->PromptBeforeExit && 
      !this->DisplayExitDialog(this->GetNthWindow(0)))
    {
    return 0;
    }

  this->InExit = 1;


  // I guess it does not hurt to save the application settings now
  // In order to restore them, the RestoreApplicationSettingsFromRegistry()
  // method should be called before the Start() method is invoked.
  // It can not be called in the constructore since the application name
  // is required. It also should probably be called after any command
  // line argument parsing that would change the registry level.

  this->SaveApplicationSettingsToRegistry();

  // Close all windows
  // This loop might be a little dangerous if a window never closes...
  // We could loop over the given number of window, and test if there
  // are still windows left at the end (= error)

  while (this->GetNumberOfWindows())
    {
    vtkKWWindowBase *win = this->GetNthWindow(0);
    if (win)
      {
      win->SetPromptBeforeClose(0);
      win->Close();
      }
    }

  // This call has to be here, not before the previous loop, so that
  // for example, the balloon help manager is not destroyed before
  // all windows are removed (in some rare occasions, a user can be fast
  // enough to quit the app while moving the mouse on a widget that
  // has a binding to the balloon help manager

  this->PrepareForDelete();

  return 1;
}
    
//----------------------------------------------------------------------------
Tcl_Interp *vtkKWApplication::InitializeTcl(int argc, 
                                            char *argv[], 
                                            ostream *err)
{
  // Command line args: language
  // This is also done in Start(), but this is should be as early
  // as we can change the language.

  int index = 0, pos = 0;
  if (vtkKWApplication::CheckForValuedArgument(
        argc, argv, "--lang", index, pos) == VTK_OK)
    {
    vtkKWLanguage::SetCurrentLanguage(
      vtkKWLanguage::GetLanguageFromXPG(argv[index] + pos));
    }

  Tcl_Interp *interp;
  char *args;
  char buf[100];

  (void)err;

  // The call to Tcl_FindExecutable has to be executed *now*, it does more 
  // than just finding the executable (for ex:, it will set variables 
  // depending on the value of TCL_LIBRARY, TK_LIBRARY)

  vtkTclApplicationInitExecutable(argc, argv);

  // Create the interpreter

  interp = Tcl_CreateInterp();
  args = Tcl_Merge(argc - 1, argv + 1);
  Tcl_SetVar(interp, (char *)"argv", args, TCL_GLOBAL_ONLY);
  ckfree(args);
  sprintf(buf, "%d", argc-1);
  Tcl_SetVar(interp, (char *)"argc", buf, TCL_GLOBAL_ONLY);
  Tcl_SetVar(interp, (char *)"argv0", argv[0], TCL_GLOBAL_ONLY);
  Tcl_SetVar(interp, (char *)"tcl_interactive", (char *)"0", TCL_GLOBAL_ONLY);

#ifdef __CYGWIN__
  Tcl_SetVar(interp, "tclDefaultLibrary", "/usr/share/tcl" TCL_VERSION, 
             TCL_GLOBAL_ONLY);
#endif

  const char* relative_dirs[] =
    {
      "lib",
      "../lib",
      "../../lib",
      "lib/TclTk",
      "../lib/TclTk",
      "../../lib/TclTk",
      "lib/TclTk/lib",
      "../lib/TclTk/lib",
      "../../lib/TclTk/lib",
      ".." KWWidgets_TclTk_INSTALL_LIB_DIR,    // for exe in PREFIX/bin
      "../.." KWWidgets_TclTk_INSTALL_LIB_DIR, // for exe in PREFIX/lib/foo-V.v
      0
    };
  vtkTclApplicationInitTclTk(interp, relative_dirs);

  Tcl_Interp *res = vtkKWApplication::InitializeTcl(interp, err);

  // If we are on Windows, bring a win32 message box since cerr is not
  // redirected to the command prompt.

#ifdef _WIN32
  if (!res && err)
    {
    MessageBox(NULL, 
               k_("The application failed to initialize Tcl/Tk!"),
               k_("Initialization Error!"),
               MB_ICONERROR | MB_OK);
    }
#endif

  return res;
}

//----------------------------------------------------------------------------
Tcl_Interp *vtkKWApplication::InitializeVTK(Tcl_Interp *interp, ostream *err)
{
  // Initialize VTK

  if (Vtkcommontcl_Init(interp) != TCL_OK) 
    {
    if (err)
      {
      *err << "Vtkcommontcl_Init error: " 
           << Tcl_GetStringResult(interp) << endl;
      }
    return NULL;
    }

#ifdef KWWidgets_BUILD_VTK_WIDGETS

  if (Vtktkrenderwidget_Init(interp) != TCL_OK) 
    {
    if (err)
      {
      *err << "Vtktkrenderwidget_Init error: " 
           << Tcl_GetStringResult(interp) << endl;
      }
    return NULL;
    }

  if (Vtktkimageviewerwidget_Init(interp) != TCL_OK) 
    {
    if (err)
      {
      *err << "Vtktkimageviewerwidget_Init error: " 
           << Tcl_GetStringResult(interp) << endl;
      }
    return NULL;
    }

#ifdef VTK_WRAP_TCL
  if (Vtkfilteringtcl_Init(interp) != TCL_OK)
    {
    if (err)
      {
      *err << "Vtkfilteringtcl_Init error: " 
           << Tcl_GetStringResult(interp) << endl;
      }
    return NULL;
    }

  if (Vtkimagingtcl_Init(interp) != TCL_OK)
    {
    if (err)
      {
      *err << "Vtkimagingtcl_Init error: " 
           << Tcl_GetStringResult(interp) << endl;
      }
    return NULL;
    }

  if (Vtkgraphicstcl_Init(interp) != TCL_OK)
    {
    if (err)
      {
      *err << "Vtkgraphicstcl_Init error: " 
           << Tcl_GetStringResult(interp) << endl;
      }
    return NULL;
    }

  if (Vtkiotcl_Init(interp) != TCL_OK)
    {
    if (err)
      {
      *err << "Vtkiotcl_Init error: " 
           << Tcl_GetStringResult(interp) << endl;
      }
    return NULL;
    }

#ifdef VTK_USE_RENDERING
  if (Vtkrenderingtcl_Init(interp) != TCL_OK)
    {
    if (err)
      {
      *err << "Vtkrenderingtcl_Init error: " 
           << Tcl_GetStringResult(interp) << endl;
      }
    return NULL;
    }

  if (Vtkvolumerenderingtcl_Init(interp) != TCL_OK)
    {
    if (err)
      {
      *err << "Vtkvolumerenderingtcl_Init error: " 
           << Tcl_GetStringResult(interp) << endl;
      }
    return NULL;
    }

  if (Vtkhybridtcl_Init(interp) != TCL_OK)
    {
    if (err)
      {
      *err << "Vtkhybridtcl_Init error: " 
           << Tcl_GetStringResult(interp) << endl;
      }
    return NULL;
    }

  if (Vtkwidgetstcl_Init(interp) != TCL_OK)
    {
    if (err)
      {
      *err << "Vtkwidgetstcl_Init error: " 
           << Tcl_GetStringResult(interp) << endl;
      }
    return NULL;
    }
#endif // VTK_USE_RENDERING

#ifdef VTK_USE_PARALLEL
  if (Vtkparalleltcl_Init(interp) != TCL_OK)
    {
    if (err)
      {
      *err << "Vtkparalleltcl_Init error: " 
           << Tcl_GetStringResult(interp) << endl;
      }
    return NULL;
    }
#endif // VTK_USE_PARALLEL

#endif // VTK_WRAP_TCL

#endif // KWWidgets_BUILD_VTK_WIDGETS

  return interp;
}

//----------------------------------------------------------------------------
Tcl_Interp *vtkKWApplication::InitializeTcl(Tcl_Interp *interp, ostream *err)
{
  // As a convenience, try to find the text domain binding for
  // KWWidgets right now

  vtkKWInternationalization::FindTextDomainBinding(
    "KWWidgets", KWWidgets_INSTALL_DATA_DIR);

  if (Et_Interp)
    {
    return NULL;
    }

  // Init Tcl

  Et_Interp = interp;

  int status;

  status = Tcl_Init(interp);
  if (status != TCL_OK)
    {
    if (err)
      {
      *err << "Tcl_Init error: " << Tcl_GetStringResult(interp) << endl;
      }
    return NULL;
    }

  // Init Tk

  if (!Tcl_PkgPresent(interp, "Tk", NULL, 0))
    {
    status = Tk_Init(interp);
    if (status != TCL_OK)
      {
      if (err)
        {
        *err << "Tk_Init error: " << Tcl_GetStringResult(interp) << endl;
        }
      return NULL;
      }

    Tcl_StaticPackage(interp, (char *)"Tk", Tk_Init, 0);
    }
    
  // As a convenience, withdraw the main Tk toplevel

  Tcl_GlobalEval(interp, "wm withdraw .");

  // Create the SetApplicationIcon command

#ifdef _WIN32
  vtkKWSetApplicationIconTclCommand_DoInit(interp);
#endif

#ifdef KWWidgets_USE_INCR_TCL

  // Initialize [incr Tcl]

  if (!Tcl_PkgPresent(interp, "Itcl", NULL, 0))
    {
    status = Itcl_Init(interp);
    if (status != TCL_OK)
      {
      if (err)
        {
        *err << "Itcl_Init error: " << Tcl_GetStringResult(interp) << endl;
        }
      return NULL;
      }
    }

  // Initialize [incr Tk]

  if (!Tcl_PkgPresent(interp, "Itk", NULL, 0))
    {
    status = Itk_Init(interp);
    if (status != TCL_OK)
      {
      if (err)
        {
        *err << "Itk_Init error: " << Tcl_GetStringResult(interp) << endl;
        }
      return NULL;
      }
    }

#endif

  // Initialize TkTreeCtrl

#ifdef KWWidgets_USE_TKTREECTRL
  vtkKWTkTreeCtrlInit::Initialize(interp);
#endif

  // Initialize tkdnd

#ifdef KWWidgets_USE_TKDND
  vtkKWTkDnDInit::Initialize(interp);
#endif

  // Initialize VTK

  vtkKWApplication::InitializeVTK(interp, err);

  // Initialize Widgets

  Kwwidgets_Init(interp);

  return interp;
}

//----------------------------------------------------------------------------
Tcl_Interp *vtkKWApplication::GetMainInterp()
{
  return Et_Interp;
}

//----------------------------------------------------------------------------
void vtkKWApplication::Start()
{ 
  int i;
  
  // look at Tcl for any args
  
  int argc = atoi(this->Script("set argc")) + 1;
  char **argv = new char *[argc];
  argv[0] = NULL;
  for (i = 1; i < argc; i++)
    {
    argv[i] = strdup(this->Script("lindex $argv %d",i-1));
    }
  this->Start(argc,argv);
  
  for (i = 0; i < argc; i++)
    {
    if (argv[i])
      {
      free(argv[i]);
      }
    }
  delete [] argv;
}

//----------------------------------------------------------------------------
void vtkKWApplication::ParseCommandLineArguments(int argc, char **argv)
{ 
  // Command line args: language
  
  int index = 0, pos = 0;
  if (vtkKWApplication::CheckForValuedArgument(
        argc, argv, "--lang", index, pos) == VTK_OK)
    {
    vtkKWLanguage::SetCurrentLanguage(
      vtkKWLanguage::GetLanguageFromXPG(argv[index] + pos));
    }
}

//----------------------------------------------------------------------------
void vtkKWApplication::Start(int argc, char **argv)
{ 
#ifdef _WIN32
#if (defined(_MSC_VER) && _MSC_VER < 1300)
#else
  // I hate to do that, but the way Tcl/Tk handles icons in menus is just
  // plain broken, even after reporting a bug back in 2, this was
  // half fixed, but icons are still corrupted in cascade menus. 
  // The only way to fix this is to temporarily disable menu animations
  // https://sourceforge.net/tracker/?func=detail&aid=1329198&group_id=12997&atid=112997

  OSVERSIONINFO ovi;
  ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  BOOL bRet = GetVersionEx(&ovi);

  BOOL bMenuFading = FALSE;
  BOOL bMenuAnim = FALSE;
  if (ovi.dwMajorVersion >= 5)
    {
    SystemParametersInfo(SPI_GETMENUFADE, 0, &bMenuFading, 0);
    SystemParametersInfo(SPI_SETMENUFADE, 0, 
                         (LPVOID) FALSE, SPIF_SENDWININICHANGE);
    
    SystemParametersInfo(SPI_GETMENUANIMATION, 0, &bMenuAnim, 0);
    SystemParametersInfo(SPI_SETMENUANIMATION, 0, 
                         (LPVOID) FALSE, SPIF_SENDWININICHANGE);
    }
#endif
#endif

  // Command line args

  this->ParseCommandLineArguments(argc, argv);

  // As a convenience, hide any splash screen

  if (this->SupportSplashScreen && this->SplashScreen)
    {
    this->GetSplashScreen()->Withdraw();
    }

  InstallTclBgErrorCallback();

  // If no windows has been mapped so far, then as a convenience,
  // map the first one

  int i, nb_windows = this->GetNumberOfWindows();
  for (i = 0; i < nb_windows && !this->GetNthWindow(i)->IsMapped(); i++)
    {
    }
  if (i >= nb_windows && nb_windows)
    {
    this->GetNthWindow(0)->Display();
    vtkKWTkUtilities::WithdrawTopLevel(this->GetMainInterp(), ".");
    }

  // Set the KWWidgets icon by default
  // For this to work, the executable should be linked against the
  // KWWidgets resource file KWWidgets.rc:
  // INCLUDE("${KWWidgets_CMAKE_DIR}/KWWidgetsResourceMacros.cmake")
  // KWWidgets_CREATE_RC_FILE("foo.rc")
  // ADD_EXECUTABLE(... foo.rc)

#ifdef _WIN32
  if (this->GetNumberOfWindows())
    {
    this->ProcessPendingEvents();
    vtkKWSetApplicationIcon(
      this->GetMainInterp(), NULL, IDI_KWWidgetsICO);
    vtkKWSetApplicationSmallIcon(
      this->GetMainInterp(), NULL, IDI_KWWidgetsICOSMALL);
    }
#endif

  // Start the event loop

  while (this->GetNumberOfWindows())
    {
    this->DoOneTclEvent();
    }


#ifdef _WIN32
#if (defined(_MSC_VER) && _MSC_VER < 1300)
#else
  // Restore menu animation
  if (ovi.dwMajorVersion >= 5)
    {
    SystemParametersInfo(SPI_SETMENUFADE, 0, 
                         (LPVOID) bMenuFading, SPIF_SENDWININICHANGE);
    SystemParametersInfo(SPI_SETMENUANIMATION, 0, 
                         (LPVOID) bMenuAnim, SPIF_SENDWININICHANGE);
    }
#endif
#endif

  // In case we never went through Exit()

  this->PrepareForDelete();

}

//----------------------------------------------------------------------------
void vtkKWApplication::RestoreApplicationSettingsFromRegistry()
{ 
  // Show balloon help ?

  vtkKWBalloonHelpManager *mgr = this->GetBalloonHelpManager();
  if (mgr && this->HasRegistryValue(
        2, "RunTime", vtkKWApplication::BalloonHelpVisibilityRegKey))
    {
    mgr->SetVisibility(
      this->GetIntRegistryValue(
        2, "RunTime", vtkKWApplication::BalloonHelpVisibilityRegKey));
    }

  // Save user interface geometry ?

  if (this->HasRegistryValue(
    2, "Geometry", vtkKWApplication::SaveUserInterfaceGeometryRegKey))
    {
    this->SaveUserInterfaceGeometry = this->GetIntRegistryValue(
      2, "Geometry", vtkKWApplication::SaveUserInterfaceGeometryRegKey);
    }

  // Show splash screen ?

  if (this->HasRegistryValue(
    2, "RunTime", vtkKWApplication::SplashScreenVisibilityRegKey))
    {
    this->SplashScreenVisibility = this->GetIntRegistryValue(
      2, "RunTime", vtkKWApplication::SplashScreenVisibilityRegKey);
    }

  if (this->RegistryLevel <= 0)
    {
    this->SplashScreenVisibility = 0;
    this->SaveUserInterfaceGeometry = 0;
    }

  // Printer settings

  if (this->HasRegistryValue(
        2, "RunTime", vtkKWApplication::PrintTargetDPIRegKey))
    {
    this->SetPrintTargetDPI(
      this->GetFloatRegistryValue(
        2, "RunTime", vtkKWApplication::PrintTargetDPIRegKey));
    }

  // Toolbar settings

  if (this->HasRegistryValue(
        2, "RunTime", vtkKWToolbar::ToolbarAspectRegKey) &&
    vtkKWToolbar::GetGlobalToolbarAspect() != 
      vtkKWToolbar::ToolbarAspectUnChanged)
    {
    vtkKWToolbar::SetGlobalToolbarAspect(
      this->GetApplication()->GetIntRegistryValue(
        2, "RunTime", vtkKWToolbar::ToolbarAspectRegKey));
    }

  if (this->GetApplication()->HasRegistryValue(
        2, "RunTime", vtkKWToolbar::WidgetsAspectRegKey) &&
    vtkKWToolbar::GetGlobalWidgetsAspect() != 
      vtkKWToolbar::WidgetsAspectUnChanged)
    {
    vtkKWToolbar::SetGlobalWidgetsAspect(
      this->GetApplication()->GetIntRegistryValue(
        2, "RunTime", vtkKWToolbar::WidgetsAspectRegKey));
    }
}

//----------------------------------------------------------------------------
void vtkKWApplication::SaveApplicationSettingsToRegistry()
{ 
  // Show balloon help ?

  vtkKWBalloonHelpManager *mgr = this->GetBalloonHelpManager();
  if (mgr)
    {
    this->SetRegistryValue(
      2, "RunTime", vtkKWApplication::BalloonHelpVisibilityRegKey, "%d", 
      mgr->GetVisibility());
    }
  
  // Save user interface geometry ?

  this->SetRegistryValue(
    2, "Geometry", vtkKWApplication::SaveUserInterfaceGeometryRegKey, "%d", 
    this->GetSaveUserInterfaceGeometry());
  
  // Show splash screen ?

  this->SetRegistryValue(
    2, "RunTime", vtkKWApplication::SplashScreenVisibilityRegKey, "%d", 
    this->GetSplashScreenVisibility());

  // Printer settings

  this->SetRegistryValue(
    2, "RunTime", vtkKWApplication::PrintTargetDPIRegKey, "%lf", 
    this->PrintTargetDPI);

  // Toolbar settings

  if (vtkKWToolbar::GetGlobalToolbarAspect() != 
      vtkKWToolbar::ToolbarAspectUnChanged)
    {
    this->SetRegistryValue(
      2, "RunTime", vtkKWToolbar::ToolbarAspectRegKey, "%d", 
      vtkKWToolbar::GetGlobalToolbarAspect());
    }

  if (vtkKWToolbar::GetGlobalWidgetsAspect() != 
      vtkKWToolbar::WidgetsAspectUnChanged)
    {
    this->SetRegistryValue(
      2, "RunTime", vtkKWToolbar::WidgetsAspectRegKey, "%d", 
      vtkKWToolbar::GetGlobalWidgetsAspect()); 
    }

  // Most recent version

  int recent_major = 0, recent_minor = 0;
  if (!this->GetMostRecentVersionLaunched(&recent_major, &recent_minor) ||
      this->MajorVersion > recent_major  || 
      (this->MajorVersion == recent_major && this->MinorVersion > recent_minor))
    {
    this->SetRegistryValue(
      2, "RunTime", vtkKWApplication::MostRecentVersionLaunchedRegKey, "%d.%d", 
      this->MajorVersion, this->MinorVersion);
    }
}

//----------------------------------------------------------------------------
void vtkKWApplication::DoOneTclEvent()
{
  Tcl_DoOneEvent(0);
}

//----------------------------------------------------------------------------
void vtkKWApplication::InstallTclBgErrorCallback()
{
  this->Script(
    "proc bgerror { m } "
    "{ global Application errorInfo; "
    "%s TclBgErrorCallback \"$m\n$errorInfo\"}", this->GetTclName());
}

//----------------------------------------------------------------------------
void vtkKWApplication::TclBgErrorCallback(const char* message)
{
  vtkErrorMacro("TclTk error: "<< message);
}

//----------------------------------------------------------------------------
int vtkKWApplication::OpenLink(const char *link)
{
#ifdef _WIN32
  HINSTANCE result = ShellExecute(
    NULL, "open", link, NULL, NULL, SW_SHOWNORMAL);
  if ((int)result > 32)
    {
    return 1;
    }
#endif

#if defined(__APPLE__) && MAC_OS_X_VERSION_MAX_ALLOWED >= 1040
  /* Refs:
     http://developer.apple.com/documentation/Carbon/Reference/LaunchServicesReference/Reference/reference.html
     http://developer.apple.com/qa/qa2001/qa1028.html
     http://www.omnigroup.com/mailman/archive/macosx-dev/2001-May/026547.html
     http://anonsvn.wireshark.org/wireshark/trunk/gtk/webbrowser.c
  */

  CFStringRef link_CFString = 
    CFStringCreateWithCString(NULL, link, kCFStringEncodingASCII);
  if (link_CFString)
    {
    // Get URL from string and open it
    CFURLRef link_CFURL = CFURLCreateWithString(NULL, link_CFString, NULL);
    CFRelease(link_CFString);
    if (link_CFURL) 
      {
      OSStatus status = LSOpenCFURLRef(link_CFURL, NULL);
      CFRelease(link_CFURL);
      if (!status)
        {
        return 1;
        }
      }
    }

  // Failed? Tried to interpret it as a filesystem path

  vtksys_stl::string path = 
    vtksys::SystemTools::CollapseFullPath(link);

  CFStringRef path_CFString = 
    CFStringCreateWithCString(NULL, link, kCFStringEncodingASCII);
  if (path_CFString)
    {
    Boolean is_dir = vtksys::SystemTools::FileIsDirectory(path.c_str());
    CFURLRef path_CFURL = CFURLCreateWithFileSystemPath(
      NULL, path_CFString, kCFURLPOSIXPathStyle, is_dir);
    CFRelease(path_CFString);
    if (path_CFURL) 
      {
      OSStatus status = LSOpenCFURLRef(path_CFURL, NULL);
      CFRelease(path_CFURL);
      if (!status)
        {
        return 1;
        }
      }
    }
#endif

  vtksys_stl::string msg(k_("Please open:\n"));
  if (link)
    {
    msg += link;
    }
  vtkKWMessageDialog::PopupMessage(
    this, this->GetNthWindow(0), 
    ks_("Display Help Dialog|Title|Open Link"), 
    msg.c_str(), vtkKWMessageDialog::WarningIcon);

  return 1;
}

//----------------------------------------------------------------------------
int vtkKWApplication::ExploreLink(const char *link)
{
  vtksys_stl::string path = 
    vtksys::SystemTools::CollapseFullPath(link);

#ifdef _WIN32
  vtksys::SystemTools::ReplaceString(path, "/", "\\");
  vtksys_stl::string command("explorer.exe /n,/e,");
  if (!vtksys::SystemTools::FileIsDirectory(path.c_str()))
    {
    command += "/select,";
    }
  command += path;
  if (WinExec(command.c_str(), SW_SHOWNORMAL) > 32)
    {
    return 1;
    }
#endif

#if defined(__APPLE__) && MAC_OS_X_VERSION_MAX_ALLOWED >= 1040

  /* Refs: AppleEvents
     http://developer.apple.com/documentation/AppleScript/Conceptual/AppleEvents/create_send_aepg/chapter_7_section_4.html
  */

  AppleEvent reveal_Event;

  CFStringRef path_CFString = 
    CFStringCreateWithCString(NULL, path.c_str(), kCFStringEncodingUTF8);
  if (path_CFString)
    {
    Boolean is_dir = vtksys::SystemTools::FileIsDirectory(path.c_str());
    CFURLRef path_CFURL = CFURLCreateWithFileSystemPath(
      kCFAllocatorDefault, path_CFString, kCFURLPOSIXPathStyle, is_dir);
    if (path_CFURL)
      {
      FSRef path_FSRef;
      if (CFURLGetFSRef(path_CFURL, &path_FSRef))
        {
        AliasHandle path_Alias;
        if (FSNewAlias(NULL, &path_FSRef, &path_Alias) == noErr)
          {
          const OSType finderSignature = 'MACS';
          if (AEBuildAppleEvent(
                kAEMiscStandards,
                kAEMakeObjectsVisible,
                typeApplSignature,
                &finderSignature,
                sizeof(finderSignature),
                kAutoGenerateReturnID,
                kAnyTransactionID,
                &reveal_Event,
                NULL,
                "'----':[alis(@@)]",
                path_Alias) == noErr)
            {
            OSErr err = AESendMessage(
              &reveal_Event,
              NULL,
              kAENoReply | kAEAlwaysInteract | kAECanSwitchLayer,
              kAEDefaultTimeout
              );
            AEDisposeDesc(&reveal_Event);
            if (err == noErr)
              {
              return 1;
              }
            }
          }
        }
      }
    }

  /* Refs (Navigation Services)
     http://developer.apple.com/documentation/Carbon/Conceptual/NavServicesIntro/ns_intro_carb/chapter_1_section_1.html
     http://developer.apple.com/documentation/Carbon/Conceptual/ProvidingNavigationDialogs/nsx_intro/chapter_1_section_1.html
     http://developer.apple.com/documentation/Carbon/Reference/Navigation_Services_Ref/Reference/reference.html
     http://developer.apple.com/documentation/Carbon/Conceptual/ProvidingNavigationDialogs/nsx_tasks/chapter_3_section_3.html
     http://developer.apple.com/qa/qa2001/qa1151.html
     http://developer.apple.com/documentation/Carbon/Reference/File_Manager/Reference/reference.html#//apple_ref/c/func/FSPathMakeRef
  */

#endif

  vtkKWLoadSaveDialog *dlg = vtkKWLoadSaveDialog::New();
  dlg->SetApplication(this);
  dlg->SetInitialFileName(path.c_str());
  dlg->SaveDialogOff();
  if (vtksys::SystemTools::FileIsDirectory(path.c_str()))
    {
    dlg->ChooseDirectoryOn();
    }
  else
    {
    vtksys_stl::string ext =
      vtksys::SystemTools::GetFilenameExtension(path);
    dlg->SetDefaultExtension(ext.c_str());
    vtksys_stl::string file_types("{{");
    file_types += ext;
    file_types += " files} {";
    file_types += ext;
    file_types += "}}";
    dlg->SetFileTypes(file_types.c_str());
    }
  dlg->MultipleSelectionOff();
  dlg->GenerateLastPath(path.c_str());
  dlg->Create();
  dlg->Invoke();
  dlg->Delete();

  return 1;
}

//----------------------------------------------------------------------------
void vtkKWApplication::ProcessPendingEvents()
{
  vtkKWTkUtilities::ProcessPendingEvents(this);
}

//----------------------------------------------------------------------------
void vtkKWApplication::ProcessIdleTasks()
{
  vtkKWTkUtilities::ProcessIdleTasks(this);
}

//----------------------------------------------------------------------------
int vtkKWApplication::DisplayExitDialog(vtkKWTopLevel *master)
{
  vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
  dialog->SetApplication(this);
  dialog->SetStyleToYesNo();
  dialog->SetMasterWindow(master);
  dialog->SetOptions(
    vtkKWMessageDialog::QuestionIcon | 
    vtkKWMessageDialog::RememberYes |
    vtkKWMessageDialog::Beep | 
    vtkKWMessageDialog::YesDefault);
  dialog->SetDialogName(vtkKWApplication::ExitDialogName);

  char buffer[500];

  sprintf(buffer, 
          k_("Are you sure you want to exit %s?"), this->GetPrettyName());
  dialog->SetText(buffer);

  sprintf(buffer, ks_("Exit Dialog|Title|Exit %s"), this->GetPrettyName());
  dialog->SetTitle(buffer);

  int ret = dialog->Invoke();
  dialog->Delete();

  // This UI interface usually displays a checkbox offering the choice
  // to prompt for exit or not. Update that UI.

  for (int i = 0; i < this->GetNumberOfWindows(); i++)
    {
    this->GetNthWindow(i)->Update();
    }

  return ret;
}

//----------------------------------------------------------------------------
void vtkKWApplication::DisplayHelpPage(const char *page, vtkKWTopLevel* master)
{
  if (!page || !*page)
    {
    return;
    }

  vtksys_stl::string helplink;

  // If it's not a remote link (crude test) and we can't find it yet, try in
  // the install/bin directory

  int is_local = strstr(page, "://") ? 0 : 1;
  if (is_local && !vtksys::SystemTools::FileExists(page))
    {
    if (this->GetInstallationDirectory())
      {
      vtksys_stl::string try_file;
      helplink = this->GetInstallationDirectory();
      helplink += "/";

      try_file = helplink + page;
      if (!vtksys::SystemTools::FileExists(try_file.c_str()))
        {
        helplink += "../";
        try_file = helplink + page;
        if (!vtksys::SystemTools::FileExists(try_file.c_str()))
          {
          helplink += "doc/";
          try_file = helplink + page;
          if (!vtksys::SystemTools::FileExists(try_file.c_str()))
            {
            helplink += "../Documentation/";
            }
          }
        }
      }
    }

  helplink += page;
  
  int status = 1;
  char buffer[500];

  const char *res = vtksys::SystemTools::FileExists(helplink.c_str()) 
    ? helplink.c_str() : page;

#if defined(_WIN32) && defined(KWWidgets_USE_HTML_HELP)
  // .chm ?

  if (strstr(helplink.c_str(), ".chm") || 
      strstr(helplink.c_str(), ".CHM"))
    {
    status = HtmlHelp(NULL, helplink.c_str(), HH_DISPLAY_TOPIC, 0) ? 1 : 0;
    }
  
  // otherwise just try to open

  else
#endif
    {
    status = this->OpenLink(helplink.c_str());
    }

  if (!status)
    {
    sprintf(
      buffer, 
      k_("The help resource %s cannot be displayed. This can be the result of "
         "the program being wrongly installed or the help file being "
         "corrupted."), res);
    vtkKWMessageDialog::PopupMessage(
      this, master, 
      ks_("Display Help Dialog|Title|Help Error!"), 
      buffer, vtkKWMessageDialog::ErrorIcon);
    }
}

//----------------------------------------------------------------------------
void vtkKWApplication::DisplayHelpDialog(vtkKWTopLevel* master)
{
  this->DisplayHelpPage(this->HelpDialogStartingPage, master);
}

//----------------------------------------------------------------------------
void vtkKWApplication::DisplayTutorial(vtkKWTopLevel* master)
{
  this->DisplayHelpPage(this->TutorialStartingPage, master);
}

//----------------------------------------------------------------------------
void vtkKWApplication::DisplayAboutDialog(vtkKWTopLevel* master)
{
  if (this->InExit)
    {
    return;
    }

  if (!this->AboutDialog)
    {
    this->AboutDialog = vtkKWMessageDialog::New();
    }

  if (!this->AboutDialog->IsCreated())
    {
    this->AboutDialog->SetApplication(this);
    this->AboutDialog->SetMasterWindow(master);
    this->AboutDialog->ModalOn();
    this->AboutDialog->Create();
    this->AboutDialog->SetBorderWidth(1);
    this->AboutDialog->SetReliefToSolid();
    }

  this->ConfigureAboutDialog();

  this->AboutDialog->Invoke();
}

//----------------------------------------------------------------------------
void vtkKWApplication::ConfigureAboutDialog()
{
  if (this->SupportSplashScreen)
    {
    this->CreateSplashScreen();
    const char *img_name = 
      this->SplashScreen ? this->SplashScreen->GetImageName() : NULL;
    if (img_name)
      {
      if (!this->AboutDialogImage)
        {
        this->AboutDialogImage = vtkKWLabel::New();
        }
      if (!this->AboutDialogImage->IsCreated())
        {
        this->AboutDialogImage->SetParent(this->AboutDialog->GetTopFrame());
        this->AboutDialogImage->Create();
        }
      this->AboutDialogImage->SetConfigurationOption("-image", img_name);
      this->Script("pack %s -side top", 
                   this->AboutDialogImage->GetWidgetName());
      int w = vtkKWTkUtilities::GetPhotoWidth(this->GetMainInterp(), img_name);
      int h = vtkKWTkUtilities::GetPhotoHeight(this->GetMainInterp(), img_name);
      this->AboutDialog->GetTopFrame()->SetWidth(w);
      this->AboutDialog->GetTopFrame()->SetHeight(h);
      if (w > this->AboutDialog->GetTextWidth())
        {
        this->AboutDialog->SetTextWidth(w);
        }
      this->Script(
        "pack %s -side bottom",  // -expand 1 -fill both
        this->AboutDialog->GetMessageDialogFrame()->GetWidgetName());
      }
    }

  if (!this->AboutRuntimeInfo)
    {
    this->AboutRuntimeInfo = vtkKWTextWithScrollbars::New();
    }
  if (!this->AboutRuntimeInfo->IsCreated())
    {
    this->AboutRuntimeInfo->SetParent(this->AboutDialog->GetBottomFrame());
    this->AboutRuntimeInfo->Create();
    this->AboutRuntimeInfo->VerticalScrollbarVisibilityOn();
    this->AboutRuntimeInfo->HorizontalScrollbarVisibilityOff();

    vtkKWText *text = this->AboutRuntimeInfo->GetWidget();
    text->SetWidth(60);
    text->SetHeight(8);
    text->SetWrapToWord();
    text->ReadOnlyOn();

    double r, g, b;
    vtkKWFrame *parent = vtkKWFrame::SafeDownCast(text->GetParent());
    parent->GetBackgroundColor(&r, &g, &b);
    text->SetBackgroundColor(r, g, b);
    this->Script("pack %s -side top -padx 2 -expand 1 -fill both",
                 this->AboutRuntimeInfo->GetWidgetName());
    }

  char buffer[500];
  sprintf(buffer, ks_("About Dialog|Title|About %s"), this->GetPrettyName());
  this->AboutDialog->SetTitle(buffer);

  vtksys_ios::ostringstream str;
  this->AddAboutText(str);
  str << endl;
  this->AddSystemInformation(str);
  str << endl;
  this->AddAboutCopyrights(str);

  this->AboutRuntimeInfo->GetWidget()->SetText( str.str().c_str() );
}

//----------------------------------------------------------------------------
void vtkKWApplication::AddSystemInformation(ostream &os)
{
}

//----------------------------------------------------------------------------
void vtkKWApplication::AddAboutText(ostream &os)
{
  os << this->GetPrettyName();
  const char *app_ver_name = this->GetVersionName();
  const char *app_rel_name = this->GetReleaseName();
  if ((app_ver_name && *app_ver_name) || (app_rel_name && *app_rel_name))
    {
    os << " (";
    if (app_ver_name && *app_ver_name)
      {
      os << app_ver_name;
      if (app_rel_name && *app_rel_name)
        {
        os << " ";
        }
      }
    if (app_rel_name && *app_rel_name)
      {
      os << app_rel_name;
      }
    os << ")";
    }
  os << endl;

  if (this->GetInstallationDirectory())
    {
    os << "Installation directory: " 
       << this->GetInstallationDirectory() << endl;
    }
  os << "User data directory: " << this->GetUserDataDirectory() << endl;

#ifdef KWWidgets_USE_INTERNATIONALIZATION
  int lang = vtkKWLanguage::GetCurrentLanguage();
  const char *lang_name = vtkKWLanguage::GetLanguageName(lang);
  const char *lang_xpg = vtkKWLanguage::GetXPGFromLanguage(lang);
  if (lang_name)
    {
    os << lang_name;
    }
  if (lang_xpg)
    {
    if (lang_name)
      {
      os << " (";
      }
    os << lang_xpg;
    if (lang_name)
      {
      os << ")";
      }
    }
  if (lang_name || lang_xpg)
    {
    os << endl;
    }
#endif
}

//----------------------------------------------------------------------------
void vtkKWApplication::AddAboutCopyrights(ostream &os)
{
  int tcl_major, tcl_minor, tcl_patch_level;
  Tcl_GetVersion(&tcl_major, &tcl_minor, &tcl_patch_level, NULL);

  os << "Tcl/Tk " 
     << tcl_major << "." << tcl_minor << "." << tcl_patch_level << endl
     << "  - Copyright (c) 1989-1994 The Regents of the University of "
     << "California." << endl
     << "  - Copyright (c) 1994 The Australian National University." << endl
     << "  - Copyright (c) 1994-1998 Sun Microsystems, Inc." << endl
     << "  - Copyright (c) 1998-2000 Ajuba Solutions." << endl;

#ifdef KWWidgets_USE_INTERNATIONALIZATION
  os << "GNU gettext runtime library (LGPL)" << endl;
#endif

  os << "Nuvola Icon Theme - Copyright (c)  2003-2004  David Vignoni." << endl;
  os << "Crystal Project Icons - Copyright (c)  2006-2007  Everaldo Coelho." << endl;
  os << "Silk Icons - http://www.famfamfam.com/lab/icons/silk/" << endl;
}

//----------------------------------------------------------------------------
int vtkKWApplication::SendErrorLog()
{
  int prompt = (vtkKWMessageDialog::RestoreMessageDialogResponseFromRegistry(
                  this, vtkKWApplication::SendErrorLogDialogName) == 0);
  if (!prompt || 
      !this->CanEmailFeedback() || 
      !this->LogDialog ||
      !this->LogDialog->GetLogWidget()->GetNumberOfRecords())
    {
    return 0;
    }

  this->DisplayLogDialog(NULL);
  
  vtkKWIcon *icon = vtkKWIcon::New();
  icon->SetImage(image_ErrorLogMenuEntry,
                 image_ErrorLogMenuEntry_width,
                 image_ErrorLogMenuEntry_height,
                 image_ErrorLogMenuEntry_pixel_size,
                 image_ErrorLogMenuEntry_length,
                 image_ErrorLogMenuEntry_decoded_length);

  vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
  dialog->SetApplication(this);
  dialog->SetStyleToOkCancel();
  dialog->SetMasterWindow(this->GetNthWindow(0));
  dialog->SetOptions(
    vtkKWMessageDialog::RememberNo |
    vtkKWMessageDialog::Beep | 
    vtkKWMessageDialog::CustomIcon |
    vtkKWMessageDialog::NoDefault);
  dialog->SetDialogName(vtkKWApplication::SendErrorLogDialogName);
  dialog->SetTitle(k_("Send Error Log?"));
  dialog->SetText(k_("Some errors were raised during your session. These errors did not cause any data loss, but by keeping us informed you can help us to improve our product.\n\nYou can send us this error log now. You can also review and send the error log in the future by accessing the \"Window\" menu or by clicking on the error icon located at the bottom right corner of the window."));
  dialog->SetOKButtonText(k_("Send Log"));
  dialog->SetCancelButtonText(k_("Do Not Send Log"));
  dialog->GetIcon()->SetImageToIcon(icon);

  int ret = dialog->Invoke();
  dialog->Delete();
  icon->Delete();

  this->GetLogDialog()->Withdraw();

  if (ret)
    {
    this->GetLogDialog()->GetLogWidget()->EmailRecords(
      this->EmailFeedbackAddress);
    }

  return ret;
}

//----------------------------------------------------------------------------
vtkKWSplashScreen *vtkKWApplication::GetSplashScreen()
{
  if (!this->SplashScreen)
    {
    this->SplashScreen = vtkKWSplashScreen::New();
    this->SplashScreen->SetApplication(this);
    this->SplashScreen->Create();
    }
  return this->SplashScreen;
}

//----------------------------------------------------------------------------
vtkKWOptionDataBase *vtkKWApplication::GetOptionDataBase()
{
  if (!this->OptionDataBase)
    {
    this->OptionDataBase = vtkKWOptionDataBase::New();
    }
  return this->OptionDataBase;
}

//----------------------------------------------------------------------------
vtkKWBalloonHelpManager *vtkKWApplication::GetBalloonHelpManager()
{
  if (!this->BalloonHelpManager && !this->InExit)
    {
    this->BalloonHelpManager = vtkKWBalloonHelpManager::New();
    this->BalloonHelpManager->SetApplication(this);
    }
  return this->BalloonHelpManager;
}

//----------------------------------------------------------------------------
vtkKWRegistryHelper *vtkKWApplication::GetRegistryHelper()
{
  if (!this->RegistryHelper)
    {
    this->RegistryHelper = vtkKWRegistryHelper::New();
    }
  return this->RegistryHelper;
}

//----------------------------------------------------------------------------
const char* vtkKWApplication::GetRegistryVersionName()
{
  if (this->RegistryVersionName)
    {
    return this->RegistryVersionName;
    }
  if (this->Name)
    {
    char versionname_buffer[1024];
    sprintf(versionname_buffer, "%s%d", this->Name, this->MajorVersion);
    this->Internals->RegistryVersionNameTemp = versionname_buffer;
    return this->Internals->RegistryVersionNameTemp.c_str();
    }
  return NULL;
}

//----------------------------------------------------------------------------
int vtkKWApplication::SetRegistryValue(int level, 
                                       const char* subkey, 
                                       const char* key, 
                                       const char* format, 
                                       ...)
{
  if (this->GetRegistryLevel() < 0 || this->GetRegistryLevel() < level)
    {
    return 0;
    }
  int res = 0;
  char buffer[vtkKWRegistryHelper::RegistryKeyNameSizeMax];
  char value[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  sprintf(buffer, "%s\\%s", this->GetRegistryVersionName(), subkey);
  va_list var_args;
  va_start(var_args, format);
  vsprintf(value, format, var_args);
  va_end(var_args);
  
  vtkKWRegistryHelper *reg = this->GetRegistryHelper();
  reg->SetTopLevel(this->GetName());
  res = reg->SetValue(buffer, key, value);
  return res;
}

//----------------------------------------------------------------------------
int vtkKWApplication::GetRegistryValue(int level, 
                                       const char* subkey, 
                                       const char* key, 
                                       char* value)
{
  if (this->GetRegistryLevel() < 0 || this->GetRegistryLevel() < level ||
      !value)
    {
    return 0;
    }
  int res = 0;
  char buff[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  char buffer[vtkKWRegistryHelper::RegistryKeyNameSizeMax];
  sprintf(buffer, "%s\\%s", this->GetRegistryVersionName(), subkey);

  vtkKWRegistryHelper *reg = this->GetRegistryHelper();
  reg->SetTopLevel(this->GetName());

  buff[0] = 0;
  res = reg->ReadValue(buffer, key, buff);
  if (res)
    {
    *value = 0;
    strcpy(value, buff);
    }  

  return res;
}

//----------------------------------------------------------------------------
int vtkKWApplication::DeleteRegistryValue(int level, 
                                          const char* subkey, 
                                          const char* key)
{
  if (this->GetRegistryLevel() < 0 || this->GetRegistryLevel() < level)
    {
    return 0;
    }
  int res = 0;
  char buffer[vtkKWRegistryHelper::RegistryKeyNameSizeMax];
  sprintf(buffer, "%s\\%s", this->GetRegistryVersionName(), subkey);
  vtkKWRegistryHelper *reg = this->GetRegistryHelper();
  reg->SetTopLevel(this->GetName());
  res = reg->DeleteValue(buffer, key);
  return res;
}

//----------------------------------------------------------------------------
int vtkKWApplication::HasRegistryValue(int level, 
                                       const char* subkey, 
                                       const char* key)
{
  char buffer[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  return this->GetRegistryValue(level, subkey, key, buffer);
}

//----------------------------------------------------------------------------
float vtkKWApplication::GetFloatRegistryValue(int level, 
                                              const char* subkey, 
                                              const char* key)
{
  if (this->GetRegistryLevel() < 0 || this->GetRegistryLevel() < level)
    {
    return 0;
    }
  float res = 0;
  char buffer[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  if (this->GetRegistryValue(level, subkey, key, buffer))
    {
    res = atof(buffer);
    }
  return res;
}

//----------------------------------------------------------------------------
int vtkKWApplication::GetIntRegistryValue(int level, 
                                          const char* subkey, 
                                          const char* key)
{
  if (this->GetRegistryLevel() < 0 || this->GetRegistryLevel() < level)
    {
    return 0;
    }
  int res = 0;
  char buffer[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  if (this->GetRegistryValue(level, subkey, key, buffer))
    {
    res = atoi(buffer);
    }
  return res;
}

//----------------------------------------------------------------------------
int vtkKWApplication::GetBooleanRegistryValue(int level, 
                                              const char* subkey, 
                                              const char* key, 
                                              const char* trueval)
{
  if (this->GetRegistryLevel() < 0 || this->GetRegistryLevel() < level)
    {
    return 0;
    }
  char buffer[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  int allset = 0;
  if (this->GetRegistryValue(level, subkey, key, buffer))
    {
    if (trueval && !strncmp(buffer+1, trueval+1, strlen(trueval)-1))
      {
      allset = 1;
      }
    }
  return allset;
}

//----------------------------------------------------------------------------
void vtkKWApplication::SaveColorRegistryValue(int level, 
                                              const char* key, 
                                              double rgb[3])
{
  this->SetRegistryValue(
    level, "Colors", key, "Color: %lf %lf %lf", rgb[0], rgb[1], rgb[2]);
}

//----------------------------------------------------------------------------
int vtkKWApplication::RetrieveColorRegistryValue(int level, 
                                                 const char* key, 
                                                 double rgb[3])
{
  char buffer[1024];
  rgb[0] = -1;
  rgb[1] = -1;
  rgb[2] = -1;

  int ok = 0;
  if (this->GetRegistryValue(
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
int vtkKWApplication::LoadScript(const char* filename)
{
  int res = 1;
  vtksys_stl::string filename_copy(filename);
  if (Tcl_EvalFile(this->GetMainInterp(), filename_copy.c_str()) != TCL_OK)
    {
    vtkErrorMacro("\n    Script: \n" << filename_copy.c_str()
                  << "\n    Returned Error on line "
                  << this->GetMainInterp()->errorLine << ": \n      "  
                  << Tcl_GetStringResult(this->GetMainInterp()) << endl);
    res = 0;
    if (this->ExitAfterLoadScript)
      {
      this->SetExitStatus(1);
      }
    }
  if (this->ExitAfterLoadScript)
    {
    this->SetPromptBeforeExit(0);
    this->Exit();
    }
  return res;
}

//----------------------------------------------------------------------------
void vtkKWApplication::SetLimitedEditionMode(int v)
{
  if (this->LimitedEditionMode == v)
    {
    return;
    }

  this->LimitedEditionMode = v;

  for (int i = 0; i < this->GetNumberOfWindows(); i++)
    {
    this->GetNthWindow(i)->Update();
    }

  this->Modified();
}

//----------------------------------------------------------------------------
int vtkKWApplication::GetLimitedEditionModeAndWarn(const char *feature)
{
  if (this->LimitedEditionMode)
    {
    const char *lem_name = this->GetLimitedEditionModeName() 
      ? this->GetLimitedEditionModeName() : "Limited Edition";

    char buffer[500];
    if (feature)
      {
      sprintf(
        buffer, 
        k_("You are running in \'%s' mode. The feature you are trying to use "
           "(%s) is not available in this mode."), lem_name, feature);
      }
    else
      {
      sprintf(
        buffer, 
        k_("You are running in \'%s' mode. The feature you are trying to use "
           "is not available in this mode."), lem_name);
      }

    vtkKWMessageDialog::PopupMessage(
      this, 0, this->GetPrettyName(), buffer, vtkKWMessageDialog::WarningIcon);
    }

  return this->LimitedEditionMode;
}

//----------------------------------------------------------------------------
void vtkKWApplication::SetReleaseMode(int arg)
{
  if (this->ReleaseMode == arg)
    {
    return;
    }

  this->ReleaseMode = arg;
  this->Modified();

  vtkObject::SetGlobalWarningDisplay(this->ReleaseMode ? 0 : 1);
}

//----------------------------------------------------------------------------
const char* vtkKWApplication::GetVersionName()
{
  if (this->VersionName)
    {
    return this->VersionName;
    }
  if (this->Name)
    {
    char versionname_buffer[1024];
    sprintf(versionname_buffer, "%s%d.%d", 
            this->Name, this->MajorVersion, this->MinorVersion);
    this->Internals->VersionNameTemp = versionname_buffer;
    return this->Internals->VersionNameTemp.c_str();
    }
  return NULL;
}

//----------------------------------------------------------------------------
int vtkKWApplication::GetMostRecentVersionLaunched(int *major, int *minor)
{
  if (!this->HasRegistryValue(
        2, "RunTime", vtkKWApplication::MostRecentVersionLaunchedRegKey))
    {
    return 0;
    }

  char buffer[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  this->GetRegistryValue(
    2, "RunTime", vtkKWApplication::MostRecentVersionLaunchedRegKey, buffer);
  if (sscanf(buffer, "%d.%d", major, minor) != 2)
    {
    return 0;
    }

  return 1;
}

//----------------------------------------------------------------------------
const char* vtkKWApplication::GetLimitedEditionModeName()
{
  if (this->LimitedEditionModeName)
    {
    return this->LimitedEditionModeName;
    }
  if (this->Name)
    {
    char lemname_buffer[1024];
    sprintf(lemname_buffer, "%s Limited Edition", this->Name);
    this->Internals->LimitedEditionModeNameTemp = lemname_buffer;
    return this->Internals->LimitedEditionModeNameTemp.c_str();
    }
  return NULL;
}

//----------------------------------------------------------------------------
const char* vtkKWApplication::GetPrettyName()
{
  vtksys_ios::ostringstream pretty_str;
  if (this->LimitedEditionMode)
    {
    const char *lem_name = this->GetLimitedEditionModeName();
    if (lem_name)
      {
      pretty_str << lem_name << " ";
      }
    else
      {
      if (this->Name)
        {
        pretty_str << this->Name << " ";
        }
      pretty_str << "Limited Edition ";
      }
    }
  else if (this->Name)
    {
    pretty_str << this->Name << " ";
    }
  pretty_str << this->MajorVersion << "." << this->MinorVersion;
  if (this->ReleaseName)
    {
    pretty_str << " " << this->ReleaseName;
    }

  this->SetPrettyName(pretty_str.str().c_str());

  return this->PrettyName;
}

//----------------------------------------------------------------------------
void vtkKWApplication::SetCharacterEncoding(int val)
{
  if (val == this->CharacterEncoding)
    {
    return;
    }

  if (val < VTK_ENCODING_NONE)
    {
    val = VTK_ENCODING_NONE;
    }
  else if (val > VTK_ENCODING_UNKNOWN)
    {
    val = VTK_ENCODING_UNKNOWN;
    }

  this->CharacterEncoding = val;
  
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkKWApplication::SetTheme(vtkKWTheme *val)
{
  if (val == this->Theme)
    {
    return;
    }

  if (this->Theme)
    {
    this->Theme->Uninstall();
    }

  this->Theme = val;

  if (this->Theme)
    {
    if (!this->Theme->GetApplication())
      {
      this->Theme->SetApplication(this);
      }
    this->Theme->Install();
    }
  
  this->Modified();
}

//----------------------------------------------------------------------------
int vtkKWApplication::CheckForArgument(
  int argc, char* argv[], const char *arg, int &index)
{
  if (!argc || !argv || !arg)
    {
    return VTK_ERROR;
    }

  // Check each arg
  // Be careful with valued argument (should not be, but who knows)

  int i;
  for (i = 0; i < argc; i++)
    {
    if (argv[i])
      {
      const char *equal = strchr(argv[i], '=');
      if (equal)
        {
        size_t part = equal - argv[i];
        if (strlen(arg) == part && !strncmp(arg, argv[i], part))
          {
          index = i;
          return VTK_OK;
          }
        }
      else
        {
        if (!strcmp(arg, argv[i]))
          {
          index = i;
          return VTK_OK;
          }
        }
      }
    }

  return VTK_ERROR;
}

//----------------------------------------------------------------------------
int vtkKWApplication::CheckForValuedArgument(
  int argc, char* argv[], const char *arg, int &index, int &value_pos)
{
  int found = vtkKWApplication::CheckForArgument(argc, argv, arg, index);
  if (found == VTK_OK)
    {
    const char *equal = strchr(argv[index], '=');
    if (equal)
      {
      value_pos = (equal - argv[index]) + 1;
      return VTK_OK;
      }
    }
  return VTK_ERROR;
}

//----------------------------------------------------------------------------
int vtkKWApplication::GetCheckForUpdatesPath(ostream &
#ifdef _WIN32
                                             path
#endif
  )
{
#ifdef _WIN32
  if (this->GetInstallationDirectory())
    {
    vtksys_ios::ostringstream upd;
    upd << this->GetInstallationDirectory() << "/WiseUpdt.exe";
    int res = vtksys::SystemTools::FileExists(upd.str().c_str());
    if (res)
      {
      path << upd.str().c_str();
      }
    return res;
    }
#endif

  return 0;
}

//----------------------------------------------------------------------------
int vtkKWApplication::HasCheckForUpdates()
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  vtksys_ios::ostringstream upd;
  int res = this->GetCheckForUpdatesPath(upd);
  return res;
#else
  return 0;
#endif
}

//----------------------------------------------------------------------------
void vtkKWApplication::CheckForUpdates()
{
  if (!this->HasCheckForUpdates())
    {
    return;
    }

#if defined(_WIN32) && !defined(__CYGWIN__)
  vtksys_ios::ostringstream upd;
  if (this->GetCheckForUpdatesPath(upd))
    {
#if defined (__BORLANDC__)
    spawnl(P_NOWAIT, upd.str().c_str(), upd.str().c_str(), NULL);
#else
   _spawnl(_P_NOWAIT, upd.str().c_str(), upd.str().c_str(), NULL);
#endif
    }
#endif
}

//----------------------------------------------------------------------------
int vtkKWApplication::CanEmailFeedback()
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  HMODULE g_hMAPI = ::LoadLibrary("MAPI32.DLL");
  int has_mapi = g_hMAPI ? 1 : 0;
  ::FreeLibrary(g_hMAPI);
  return (has_mapi && this->EmailFeedbackAddress) ? 1 : 0;
#endif

#if defined(__APPLE__) && MAC_OS_X_VERSION_MAX_ALLOWED >= 1040
  return 1;
#endif

  return 0;
}

//----------------------------------------------------------------------------
void vtkKWApplication::AddEmailFeedbackBody(ostream &os)
{
  os << this->GetPrettyName() << " (" << this->GetVersionName();
  if (this->GetReleaseName())
    {
    os << " "  << this->GetReleaseName();
    }
  os << ")" << endl;
  this->AddSystemInformation(os);
}

//----------------------------------------------------------------------------
void vtkKWApplication::AddEmailFeedbackSubject(ostream &os)
{
  os << this->GetPrettyName() << " " 
     << ks_("Email Feedback|Subject|User Feedback");
}

//----------------------------------------------------------------------------
int vtkKWApplication::SendEmail(
  const char *to,
  const char *subject,
  const char *message,
  const char *attachment_filename,
  const char *extra_error_msg)
{
#if defined(_WIN32) && !defined(__CYGWIN__)

  int retry = 1;
  ULONG err;

  while (retry)
    {
    // Load MAPI

    HMODULE g_hMAPI = ::LoadLibrary("MAPI32.DLL");
    if (!g_hMAPI)
      {
      return 0;
      }

    // Recipient To: (no SMTP: for Mozilla)

    MapiRecipDesc recip_to = 
      {
        0L,
        MAPI_TO,
        NULL,
        (LPSTR)to,
        0L,
        NULL
      };

    // Attachement

    MapiFileDesc attachment = 
      {
        0L,
        0L,
        -1L,
        (LPSTR)attachment_filename,
        NULL,
        NULL
      };

    // The email itself

    MapiMessage email = 
      {
        0, 
        (LPSTR)subject,
        (LPSTR)message,
        NULL, 
        NULL, 
        NULL, 
        0, 
        NULL,
        1, 
        &recip_to, 
        attachment_filename ? 1 : 0, 
        attachment_filename ? &attachment : NULL
      };

    // Send it

    err = ((LPMAPISENDMAIL)GetProcAddress(g_hMAPI, "MAPISendMail"))(
      0L,
      0L,
      &email,
      MAPI_DIALOG | MAPI_LOGON_UI,
      0L);

    retry = 0;

    if (err != SUCCESS_SUCCESS)
      {
      vtksys_stl::string msg =
        k_("Sorry, an error occurred while trying to send an email.\n\n"
           "Please make sure that your default email client has been "
           "configured properly. The Microsoft Simple MAPI (Messaging "
           "Application Program Interface) is used to perform this "
           "operation and it might not be accessible if your "
           "default email client is not running.");
      if (extra_error_msg)
        {
        msg += "\n\n";
        msg += extra_error_msg;
        }

      vtkKWMessageDialog *dlg = vtkKWMessageDialog::New();
      this->CreateEmailMessageDialog(
        dlg, to, subject, message, attachment_filename);
      dlg->SetText(msg.c_str());
      dlg->SetStyleToOkCancel();
      dlg->SetOKButtonText(ks_("Email Feedback Dialog|Button|Retry"));
      dlg->Invoke();
      int status = dlg->GetStatus();
      if (status == vtkKWMessageDialog::StatusOK)
        {
        retry = 1;
        }
      dlg->Delete();
      }

    ::FreeLibrary(g_hMAPI);
    }
  
  return (err != SUCCESS_SUCCESS ? 0 : 1);

#endif

#if defined(__APPLE__) && MAC_OS_X_VERSION_MAX_ALLOWED >= 1040
  /* Refs
     http://lists.apple.com/archives/applescript-studio/2006/Nov/msg00012.html
     http://www.mackb.com/Uwe/Forum.aspx/entourage/21582/Applescript-to-create-new-message
     http://www.microsoft.com/mac/developers/default.mspx?CTT=PageView&clr=99-21-0&target=9efe4640-7277-4372-a304-4181b2c098f21033&srcid=40d869d3-e1a6-4bce-9e5b-fb5b87bec3f41033&ep=7
     http://forums.macosxhints.com/archive/index.php/t-73192.html
     http://forums.macosxhints.com/archive/index.php/t-31163.html

     http://groups.google.com/group/EasyTask-Users/browse_thread/thread/fb2632fe253dd8e3
     http://fundisom.com/apple/search/applescript-send-mail/
     http://www.mail-archive.com/cocoa-dev@lists.apple.com/msg08778.html
     http://www.aaronsw.com/weblog/001054
     http://akosut.com/software/mailscripts.html
  */

  // Sending an email using AppleEvents directly is just plain
  // hard. Let's write and run an AppleScript instead.

  vtksys_stl::string script;
  script = 
    "tell application \"Mail\"\n"
    "  activate\n"
    "  set newMessage to make new outgoing message\n"
    "  tell newMessage\n"
    "    set visible to true\n";
  if (to)
    {
    script = script +
      "    make new recipient at end of to recipients with properties {address:\"" + to + "\"}\n";
    }
  if (subject)
    {
    script = script +
      "    set subject to \"" + 
      vtksys::SystemTools::EscapeChars(subject, "\"") + "\"\n";
    }
  if (message)
    {
    script = script +
      "    set content to \"" + 
      vtksys::SystemTools::EscapeChars(message, "\"") + "\n\n\n\"\n";
    }
  if (attachment_filename)
    {
    script = script +
      "    set filepath to POSIX path of \"" + attachment_filename + "\"\n" +
      "    make new attachment with properties {file name:filepath}\n";
    }
  script = script +
    "  end tell\n"
    "end tell\n";
  
  if (vtkKWApplication::RunAppleScript(script.c_str()))
    {
    return 1;
    }

#endif

  vtksys_stl::string msg =
    k_("Sorry, sending an email from this operating system is not "
       "supported at the moment.\n\n"
       "In the meantime, we suggest you open your favorite email client "
       "and copy/paste the fields below to a new email message.");
  if (extra_error_msg)
    {
    msg += "\n\n";
    msg += extra_error_msg;
    }

  vtkKWMessageDialog *dlg = vtkKWMessageDialog::New();
  this->CreateEmailMessageDialog(
    dlg, to, subject, message, attachment_filename);
  dlg->SetText(msg.c_str());
  dlg->Invoke();

  return 0;
}

//----------------------------------------------------------------------------
void vtkKWApplication::CreateEmailMessageDialog(
  vtkKWMessageDialog *dlg,
  const char *to,
  const char *subject,
  const char *message,
  const char *attachment_filename)
{
  if (!dlg)
    {
    return;
    }

  if (!dlg->GetApplication())
    {
    dlg->SetApplication(this);
    }
  dlg->SetStyleToCancel();
  dlg->SetOptions(vtkKWMessageDialog::ErrorIcon);
  dlg->SetTitle(ks_("Email Feedback Dialog|Title|Send Email Error!"));
  dlg->Create();

  vtkKWSeparator *sep = vtkKWSeparator::New();
  sep->SetParent(dlg->GetBottomFrame());
  sep->Create();
  sep->Delete();

  this->Script("pack %s -side top -padx 2 -pady 2 -expand 1 -fill x",
               sep->GetWidgetName());

  int label_width = 14;

  if (to)
    {
    vtkKWEntryWithLabel *to_entry = vtkKWEntryWithLabel::New();
    to_entry->SetParent(dlg->GetBottomFrame());
    to_entry->Create();
    to_entry->SetLabelText(ks_("Email Feedback Dialog|Field|To:"));
    to_entry->SetLabelWidth(label_width);
    to_entry->GetWidget()->SetValue(to ? to : "");
    to_entry->GetWidget()->ReadOnlyOn();
    to_entry->Delete();

    this->Script("pack %s -side top -padx 2 -pady 2 -expand 1 -fill x",
                 to_entry->GetWidgetName());
    }

  if (subject)
    {
    vtkKWEntryWithLabel *subject_entry = vtkKWEntryWithLabel::New();
    subject_entry->SetParent(dlg->GetBottomFrame());
    subject_entry->Create();
    subject_entry->SetLabelText(ks_("Email Feedback Dialog|Field|Subject:"));
    subject_entry->SetLabelWidth(label_width);
    subject_entry->GetWidget()->SetValue(subject ? subject : "");
    subject_entry->GetWidget()->ReadOnlyOn();
    subject_entry->Delete();

    this->Script("pack %s -side top -padx 2 -pady 2 -expand 1 -fill x",
                 subject_entry->GetWidgetName());
    }

  if (attachment_filename)
    {
    vtkKWFrame *attachment_frame = vtkKWFrame::New();
    attachment_frame->SetParent(dlg->GetBottomFrame());
    attachment_frame->Create();
    attachment_frame->Delete();

    this->Script("pack %s -side top -padx 0 -pady 0 -expand 1 -fill x",
                 attachment_frame->GetWidgetName());

    vtkKWEntryWithLabel *attachment_entry = vtkKWEntryWithLabel::New();
    attachment_entry->SetParent(attachment_frame);
    attachment_entry->Create();
    attachment_entry->SetLabelText(
      ks_("Email Feedback Dialog|Field|Attachment:"));
    attachment_entry->SetLabelWidth(label_width);
    attachment_entry->GetWidget()->SetValue(
      attachment_filename ? attachment_filename : "");
    attachment_entry->GetWidget()->ReadOnlyOn();
    attachment_entry->Delete();
  
    this->Script("pack %s -side left -padx 2 -pady 2 -expand 1 -fill x",
                 attachment_entry->GetWidgetName());

    vtkKWPushButton *attachment_locate_button = vtkKWPushButton::New();
    attachment_locate_button->SetParent(attachment_frame);
    attachment_locate_button->Create();
    attachment_locate_button->SetImageToPredefinedIcon(vtkKWIcon::IconFolder);
    attachment_locate_button->SetBalloonHelpString(
      ks_("Email Feedback Dialog|Field|Attachment|Locate attachment on disk"));
    vtksys_stl::string command("ExploreLink {");
    command += attachment_filename;
    command += "}";
    attachment_locate_button->SetCommand(this, command.c_str());
    attachment_locate_button->Delete();
  
    this->Script("pack %s -side left -padx 2 -pady 2 -expand 0 -fill none",
                 attachment_locate_button->GetWidgetName());
    }

  if (message)
    {
    vtkKWTextWithScrollbars *message_text = vtkKWTextWithScrollbars::New();
    message_text->SetParent(dlg->GetBottomFrame());
    message_text->Create();
    message_text->VerticalScrollbarVisibilityOn();
    message_text->HorizontalScrollbarVisibilityOff();
    vtkKWText *text_widget = message_text->GetWidget();
    text_widget->SetWidth(60);
    text_widget->SetHeight(8);
    text_widget->SetWrapToWord();
    text_widget->ReadOnlyOn();
    text_widget->SetText(message ? message : "");
    message_text->Delete();

    this->Script("pack %s -side top -padx 2 -pady 2 -expand 1 -fill both",
                 message_text->GetWidgetName());
    }

}

//----------------------------------------------------------------------------
void vtkKWApplication::EmailFeedback()
{
  if (!this->CanEmailFeedback())
    {
    return;
    }

  vtksys_ios::ostringstream email_subject;
  this->AddEmailFeedbackSubject(email_subject);

  vtksys_ios::ostringstream message;
  this->AddEmailFeedbackBody(message);
  message << endl;

  char buffer[500];
  sprintf(
    buffer, 
    k_("If you continue to experience problems please use your email "
       "client to send us feedback at %s."), this->EmailFeedbackAddress);

  this->SendEmail(
    this->EmailFeedbackAddress,
    email_subject.str().c_str(),
    message.str().c_str(),
    NULL,
    buffer);
}

//----------------------------------------------------------------------------
#ifdef __APPLE__
int vtkKWApplication::RunAppleScript(
  const char *text)
{
#if defined(__APPLE__) && MAC_OS_X_VERSION_MAX_ALLOWED >= 1040

  // Source: http://developer.apple.com/qa/qa2001/qa1026.html
  // Gotta link against -framework Carbon now :(

  long textLength = strlen(text);

  AEDesc scriptTextDesc;
  AECreateDesc(typeNull, NULL, 0, &scriptTextDesc);

  AEDesc resultData;
  OSStatus err;

  // Open the scripting component
  ComponentInstance theComponent = 
    OpenDefaultComponent(kOSAComponentType, typeAppleScript);
  if (theComponent == NULL) 
    { 
    err = paramErr; 
    }
  else
    {
    // Put the script text into an aedesc 
    err = AECreateDesc(typeChar, text, textLength, &scriptTextDesc);
    if (err == noErr) 
      {
      // Compile the script 
      OSAID scriptID = kOSANullScript;;
      err = OSACompile(theComponent, &scriptTextDesc, kOSAModeNull, &scriptID);
      if (err == noErr) 
        {
        // Run the script 
        OSAID resultID = kOSANullScript;;
        err = OSAExecute(
          theComponent, scriptID, kOSANullScript, kOSAModeNull, &resultID);
        // Collect the results - if any
        AECreateDesc(typeNull, NULL, 0, &resultData);
        if (err == errOSAScriptError) 
          {
          OSAScriptError(theComponent, kOSAErrorMessage, typeChar, &resultData);
          } 
        else if (err == noErr && resultID != kOSANullScript) 
          {
          OSADisplay(
            theComponent, resultID, typeChar, kOSAModeNull, &resultData);
          OSADispose(theComponent, resultID);
          }
        OSADispose(theComponent, scriptID);
        }
      }
    CloseComponent(theComponent);
    }

  AEDisposeDesc(&scriptTextDesc);

  return err == noErr ? 1 : 0;

#endif

  (void)text;
  return 0;
}
#endif

//----------------------------------------------------------------------------
void vtkKWApplication::RegisterDialogUp(vtkKWWidget *)
{
  this->DialogUp++;
}

//----------------------------------------------------------------------------
void vtkKWApplication::UnRegisterDialogUp(vtkKWWidget *)
{
  this->DialogUp--;

  if (this->DialogUp < 0)
    {
    vtkErrorMacro(
      "It seems that UnRegisterDialogUp() was called without a matching call to RegisterDialogUp(), since the number of dialogs supposed to be up is now negative... This is most likely not good.");
    }
}

//----------------------------------------------------------------------------
int vtkKWApplication::IsDialogUp()
{
  return (this->DialogUp > 0 ? 1 : 0);
}

//----------------------------------------------------------------------------
const char* vtkKWApplication::GetInstallationDirectory()
{
  if (!this->InstallationDirectory)
    {
    this->FindInstallationDirectory();
    }
  return this->InstallationDirectory;
}

//----------------------------------------------------------------------------
void vtkKWApplication::FindInstallationDirectory()
{
  const char *nameofexec = Tcl_GetNameOfExecutable();
  if (nameofexec && vtksys::SystemTools::FileExists(nameofexec))
    {
    vtksys_stl::string directory = 
      vtksys::SystemTools::GetFilenamePath(nameofexec);
    // remove the /bin from the end
    // directory[strlen(directory) - 4] = '\0';
    // => do not *do* that: first it breaks all the apps 
    // relying on this method to find where the binary is installed 
    // (hello plugins ?), second this is a hard-coded assumption, what
    // about msdev path, bin/release, bin/debug, etc.
    // If you need to remove whatever dir, just copy the result of this
    // method and strip it where needed.
    vtksys::SystemTools::ConvertToUnixSlashes(directory);
    this->SetInstallationDirectory(directory.c_str());
    }
  else
    {
    char setup_key[vtkKWRegistryHelper::RegistryKeyNameSizeMax];
    sprintf(setup_key, "%s\\Setup", this->GetRegistryVersionName());
    vtkKWRegistryHelper *reg = this->GetRegistryHelper();
    reg->SetTopLevel(this->GetName());
    char installed_path[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
    if (reg && reg->ReadValue(setup_key, "InstalledPath", installed_path))
      {
      vtksys_stl::string directory(installed_path);
      vtksys::SystemTools::ConvertToUnixSlashes(directory);
      this->SetInstallationDirectory(directory.c_str());
      }
    else
      {
      reg->SetGlobalScope(1);
      if (reg && reg->ReadValue(setup_key, "InstalledPath", installed_path))
        {
        vtksys_stl::string directory(installed_path);
        vtksys::SystemTools::ConvertToUnixSlashes(directory);
        this->SetInstallationDirectory(directory.c_str());
        }
      else
        {
        this->SetInstallationDirectory(0);
        }
      reg->SetGlobalScope(0);
      }
    }
}

//----------------------------------------------------------------------------
const char* vtkKWApplication::GetUserDataDirectory()
{
  if (!this->UserDataDirectory)
    {
    vtksys_stl::string dir;

#ifdef _WIN32
    vtksys_stl::string personal;
    if (vtksys::SystemTools::ReadRegistryValue(
          "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders;Personal", // or ;AppData
          personal)) 
      {
      dir = personal;

      dir += "/";
      }
#else
    dir = vtksys::SystemTools::GetEnv("HOME");
    dir += "/.";
#endif

    dir += this->GetName();
    vtksys::SystemTools::ConvertToUnixSlashes(dir);

    // We assume that if the user data dir is being requested, we should
    // create it now in case it does not exist, so that it can be used right
    // away.

    if (!vtksys::SystemTools::FileExists(dir.c_str()))
      {
      vtksys::SystemTools::MakeDirectory(dir.c_str());
      }

    this->UserDataDirectory = new char [dir.size() + 1];
    strcpy(this->UserDataDirectory, dir.c_str());
    }

  return this->UserDataDirectory;
}

//----------------------------------------------------------------------------
const char* vtkKWApplication::Script(const char* format, ...)
{
  va_list var_args1, var_args2;
  va_start(var_args1, format);
  va_start(var_args2, format);
  const char* result = vtkKWTkUtilities::EvaluateStringFromArgs(
    this, format, var_args1, var_args2);
  va_end(var_args1);
  va_end(var_args2);
  return result;
}

//----------------------------------------------------------------------------
int vtkKWApplication::EvaluateBooleanExpression(const char* format, ...)
{
  va_list var_args1, var_args2;
  va_start(var_args1, format);
  va_start(var_args2, format);
  const char* result = vtkKWTkUtilities::EvaluateStringFromArgs(
    this, format, var_args1, var_args2);
  va_end(var_args1);
  va_end(var_args2);

  return (result && !strcmp(result, "1")) ? 1 : 0;
}

//----------------------------------------------------------------------------
int vtkKWApplication::PutEnv(const char* value)
{ 
  static vtkKWApplicationInternals::DeletingCharVector local_environment;
  char *env_var = new char[strlen(value) + 1];
  strcpy(env_var, value);
  int ret = putenv(env_var);
  // save the pointer in the static vector so that it can
  // be deleted on exit
  // http://groups.google.com/group/comp.unix.wizards/msg/f0915a043bf259fa?dmode=source
  local_environment.push_back(env_var);
  return ret == 0;
}

//----------------------------------------------------------------------------
void vtkKWApplication::WarningMessage(const char* message)
{
  this->InvokeEvent(vtkKWEvent::WarningMessageEvent, (void*)message);
  if (this->CreateLogDialog())
    {
    this->GetLogDialog()->GetLogWidget()->AddWarningRecord(message);
    }
#ifdef _WIN32
  ::OutputDebugString(message);
#endif
  cerr << message << endl;
}

//----------------------------------------------------------------------------
void vtkKWApplication::ErrorMessage(const char* message)
{
  this->InvokeEvent(vtkKWEvent::ErrorMessageEvent, (void*)message);
  if (this->CreateLogDialog())
    {
    this->GetLogDialog()->GetLogWidget()->AddErrorRecord(message);
    }
#ifdef _WIN32
  ::OutputDebugString(message);
#endif
  cerr << message << endl;
}

//----------------------------------------------------------------------------
void vtkKWApplication::InformationMessage(const char* message)
{
  this->InvokeEvent(vtkKWEvent::InformationMessageEvent, (void*)message);
  if (this->CreateLogDialog())
    {
    this->GetLogDialog()->GetLogWidget()->AddInformationRecord(message);
    }
#ifdef _WIN32
  ::OutputDebugString(message);
#endif
  cerr << message << endl;
}

//----------------------------------------------------------------------------
void vtkKWApplication::DebugMessage(const char* message)
{
  this->InvokeEvent(vtkKWEvent::DebugMessageEvent, (void*)message);
  if (this->CreateLogDialog())
    {
    this->GetLogDialog()->GetLogWidget()->AddDebugRecord(message);
    }
#ifdef _WIN32
  ::OutputDebugString(message);
#endif
  cerr << message << endl;
}

//----------------------------------------------------------------------------
vtkKWLogDialog* vtkKWApplication::GetLogDialog()
{
  if (!this->InExit && !this->LogDialog)
    {
    this->LogDialog = vtkKWLogDialog::New();
    this->GetLogDialog()->SetApplication(this);
    }
  return this->LogDialog;
}

//----------------------------------------------------------------------------
int vtkKWApplication::CreateLogDialog()
{
  if (this->GetLogDialog())
    {
    if (!this->GetLogDialog()->IsCreated())
      {
      this->GetLogDialog()->Create();
      }
    return this->GetLogDialog()->IsCreated();
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWApplication::DisplayLogDialog(vtkKWTopLevel* master)
{
  if (this->CreateLogDialog())
    {
    this->GetLogDialog()->SetMasterWindow(
      master ? master : this->GetNthWindow(0));
    this->GetLogDialog()->Display();
    }
}

//----------------------------------------------------------------------------
vtkKWTclInteractor* vtkKWApplication::GetTclInteractor()
{
  if (!this->TclInteractor)
    {
    this->TclInteractor = vtkKWTkcon::New();
    }

  if (!this->TclInteractor->IsCreated())
    {
    this->TclInteractor->SetApplication(this);
    this->TclInteractor->Create();
    }
  
  return this->TclInteractor;
}

//----------------------------------------------------------------------------
void vtkKWApplication::DisplayTclInteractor(vtkKWTopLevel *master)
{
  vtkKWTclInteractor *tcl_interactor = this->GetTclInteractor();
  if (tcl_interactor)
    {
    if (!master)
      {
      master = this->GetNthWindow(0);
      }
    if (master)
      {
      vtksys_stl::string title;
      if (master->GetTitle())
        {
        title += master->GetTitle();
        title += " : ";
        }
      title += ks_("Tcl Interactor Dialog|Title|Tcl Interactor");
      tcl_interactor->SetTitle(title.c_str());
      tcl_interactor->SetMasterWindow(master);
      }
    tcl_interactor->Display();
    }
}

//----------------------------------------------------------------------------
vtkKWColorPickerDialog* vtkKWApplication::GetColorPickerDialog()
{
  if (!this->ColorPickerDialog)
    {
    this->ColorPickerDialog = vtkKWColorPickerDialog::New();
    }

  if (!this->ColorPickerDialog->IsCreated())
    {
    this->ColorPickerDialog->SetApplication(this);
    // do not set the master window otherwise TAB doesn't switch focus
    this->ColorPickerDialog->Create();
    this->ColorPickerDialog->SetDisplayPositionToPointer();
    }
  
  return this->ColorPickerDialog;
}

//----------------------------------------------------------------------------
void vtkKWApplication::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Name: " << this->GetName() << endl;
  os << indent << "MajorVersion: " << this->MajorVersion << endl;
  os << indent << "MinorVersion: " << this->MinorVersion << endl;
  os << indent << "ReleaseName: " 
     << (this->ReleaseName ? this->ReleaseName : "(None)") << endl;
  os << indent << "VersionName: " 
     << (this->VersionName ? this->VersionName : "(None)") << endl;
  os << indent << "RegistryVersionName: " 
     << (this->RegistryVersionName ? this->RegistryVersionName : "(None)") << endl;
  os << indent << "PrettyName: " 
     << this->GetPrettyName() << endl;
  os << indent << "EmailFeedbackAddress: "
     << (this->GetEmailFeedbackAddress() ? this->GetEmailFeedbackAddress() :
         "(none)")
     << endl;
  os << indent << "HelpDialogStartingPage: "
     << (this->HelpDialogStartingPage ? this->HelpDialogStartingPage :
         "(none)")
     << endl;
  os << indent << "TutorialStartingPage: "
     << (this->TutorialStartingPage ? this->TutorialStartingPage :
         "(none)")
     << endl;
  os << indent << "ExitStatus: " << this->GetExitStatus() << endl;
  os << indent << "RegistryLevel: " << this->GetRegistryLevel() << endl;
  os << indent << "ExitAfterLoadScript: " << (this->ExitAfterLoadScript ? "on":"off") << endl;
  os << indent << "InExit: " << (this->InExit ? "on":"off") << endl;
  if (this->SplashScreen)
    {
    os << indent << "SplashScreen: " << this->SplashScreen << endl;
    }
  else
    {
    os << indent << "SplashScreen: (none)" << endl;
    }
  if (this->BalloonHelpManager)
    {
    os << indent << "BalloonHelpManager: " << this->BalloonHelpManager << endl;
    }
  else
    {
    os << indent << "BalloonHelpManager: (none)" << endl;
    }
  os << indent << "SupportSplashScreen: " << (this->SupportSplashScreen ? "on":"off") << endl;
  os << indent << "SplashScreenVisibility: " << (this->SplashScreenVisibility ? "on":"off") << endl;
  os << indent << "PromptBeforeExit: " << (this->GetPromptBeforeExit() ? "on":"off") << endl;
  os << indent << "SendErrorLogBeforeExit: " << (this->GetSendErrorLogBeforeExit() ? "on":"off") << endl;
  os << indent << "InstallationDirectory: " 
     << (this->GetInstallationDirectory() ? this->GetInstallationDirectory() : "None") << endl;
  os << indent << "UserDataDirectory: " 
     << (this->UserDataDirectory ? UserDataDirectory : "None") << endl;
  os << indent << "SaveUserInterfaceGeometry: " 
     << (this->SaveUserInterfaceGeometry ? "On" : "Off") << endl;
  os << indent << "LimitedEditionMode: " 
     << (this->LimitedEditionMode ? "On" : "Off") << endl;
  os << indent << "CharacterEncoding: " << this->CharacterEncoding << "\n";
  os << indent << "Theme: ";
  if (this->Theme)
    {
    os << this->Theme << endl;
    }
  else
    {
    os << "NULL" << endl;
    }
  os << indent << "LimitedEditionModeName: " 
     << (this->LimitedEditionModeName ? this->LimitedEditionModeName
         : "None") << endl;
  os << indent << "ReleaseMode: " 
     << (this->ReleaseMode ? "On" : "Off") << endl;
  os << indent << "PrintTargetDPI: " << this->GetPrintTargetDPI() << endl;
  os << indent << "TclInteractor: " << this->GetTclInteractor() << endl;
  os << indent << "ColorPickerDialog: " << this->GetColorPickerDialog() << endl;
}
