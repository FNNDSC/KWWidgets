/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWApplication.cxx,v $
  Language:  C++
  Date:      $Date: 2003-04-30 19:45:01 $
  Version:   $Revision: 1.113 $

Copyright (c) 2000-2001 Kitware Inc. 469 Clifton Corporate Parkway,
Clifton Park, NY, 12065, USA.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither the name of Kitware nor the names of any contributors may be used
   to endorse or promote products derived from this software without specific 
   prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "vtkKWApplication.h"

#include "vtkArrayMap.txx"
#include "vtkKWApplicationSettingsInterface.h"
#include "vtkKWBWidgets.h"
#include "vtkKWLabel.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWObject.h"
#include "vtkKWRegisteryUtilities.h"
#include "vtkKWSplashScreen.h"
#include "vtkKWWidgetsConfigure.h"
#include "vtkKWWindow.h"
#include "vtkKWWindowCollection.h"
#include "vtkObjectFactory.h"
#include "vtkOutputWindow.h"
#include "vtkString.h"
#include "vtkTclUtil.h"

#include <stdarg.h>

#ifdef USE_INSTALLED_TCLTK_PACKAGES
static Tcl_Interp *Et_Interp = 0;
#else
#include "kwinit.h"
#endif

#ifdef _WIN32
#include <htmlhelp.h>
#include "kwappicon.h"
#endif

int vtkKWApplication::WidgetVisibility = 1;


//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWApplication );
vtkCxxRevisionMacro(vtkKWApplication, "$Revision: 1.113 $");

extern "C" int Vtktcl_Init(Tcl_Interp *interp);
extern "C" int Vtkkwwidgetstcl_Init(Tcl_Interp *interp);

int vtkKWApplicationCommand(ClientData cd, Tcl_Interp *interp,
                            int argc, char *argv[]);

vtkKWApplication::vtkKWApplication()
{
  this->BalloonHelpWidget = 0;
  this->CommandFunction = vtkKWApplicationCommand;
  
  this->ApplicationName = vtkString::Duplicate("Kitware");
  this->MajorVersion = 1;
  this->MinorVersion = 0;
  this->ApplicationVersionName = vtkString::Duplicate("Kitware10");
  this->ApplicationReleaseName = vtkString::Duplicate("unknown");
  
  this->InExit = 0;
  this->DialogUp = 0;
  this->TraceFile = NULL;

  this->ExitStatus = 0;

  this->Registery = 0;
  this->RegisteryLevel = 10;

  this->UseMessageDialogs = 1;  

  this->Windows = vtkKWWindowCollection::New();  
  
  // add the application as $app

  if (vtkKWApplication::WidgetVisibility)
    {
    this->BalloonHelpWindow = vtkKWWidget::New();
    this->BalloonHelpLabel = vtkKWLabel::New();
    }
  else
    {
    this->BalloonHelpWindow = 0;
    this->BalloonHelpLabel = 0;
    }
  this->BalloonHelpPending = NULL;
  this->BalloonHelpDelay = 2;

  // setup tcl stuff
  this->MainInterp = Et_Interp;
  if ( !this->MainInterp )
    {
    vtkErrorMacro("Interpreter not set. This probably means that Tcl was not initialized properly...");
    return;
    }

  //vtkTclGetObjectFromPointer(this->MainInterp, (void *)this, 
  //                           vtkKWApplicationCommand);

  //this->Script("set Application %s",this->MainInterp->result);  
  this->Script("set Application %s",this->GetTclName());

  if (vtkKWApplication::WidgetVisibility)
    {
    //this->BalloonHelpWindow->SetParent(this->GetParentWindow());
    this->BalloonHelpWindow->Create(
      this, "toplevel", "-background black -borderwidth 1 -relief flat");
    this->BalloonHelpLabel->SetParent(this->BalloonHelpWindow);    
    this->BalloonHelpLabel->Create(
      this, "-background LightYellow -foreground black -justify left "
      "-wraplength 2i");
    this->Script("pack %s", this->BalloonHelpLabel->GetWidgetName());
    this->Script("wm overrideredirect %s 1", 
                 this->BalloonHelpWindow->GetWidgetName());
    this->Script("wm withdraw %s", this->BalloonHelpWindow->GetWidgetName());
    this->SplashScreen = vtkKWSplashScreen::New();
    }
  else
    {
    this->SplashScreen = NULL;
    }

  this->ExitOnReturn = 0;

  this->HasSplashScreen = 0;
  this->ShowSplashScreen = 1;

  if (this->HasRegisteryValue(
    2, "RunTime", VTK_KW_SHOW_TOOLTIPS_REG_KEY))
    {
    this->ShowBalloonHelp = this->GetIntRegisteryValue(
      2, "RunTime", VTK_KW_SHOW_TOOLTIPS_REG_KEY);
    }
  else
    {
    this->ShowBalloonHelp = 1;
    }

  this->ApplicationExited = 0;
}

vtkKWApplication::~vtkKWApplication()
{
  if ( this->BalloonHelpWindow )
    {
    this->BalloonHelpWindow->Delete();
    }
  if ( this->BalloonHelpLabel )
    {
    this->BalloonHelpLabel->Delete();
    }
  this->SetBalloonHelpWidget(0);
  if (this->Windows)
    {
    this->Windows->Delete();
    this->Windows = NULL;
    this->MainInterp = NULL;
    vtkObjectFactory::UnRegisterAllFactories();
    }

  this->SetApplicationName(NULL);
  this->SetApplicationVersionName(NULL);
  this->SetApplicationReleaseName(NULL);

  if (this->TraceFile)
    {
    this->TraceFile->close();
    delete this->TraceFile;
    this->TraceFile = NULL;
    }
  if (this->Registery )
    {
    this->Registery->Delete();
    }
}

void vtkKWApplication::SetApplication(vtkKWApplication*) 
{ 
  vtkErrorMacro( << "Do not set the Application on an Application" << endl); 
}

//----------------------------------------------------------------------------
const char* vtkKWApplication::EvaluateString(const char* format, ...)
{  
  ostrstream str;
  str << "eval set vtkKWApplicationEvaluateStringTemporaryString " 
      << format << ends;
  va_list var_args1, var_args2;
  va_start(var_args1, format);
  va_start(var_args2, format);
  const char* result = this->ScriptInternal(str.str(), var_args1, var_args2);
  va_end(var_args1);
  va_end(var_args2);
  str.rdbuf()->freeze(0);
  return result;
}

//----------------------------------------------------------------------------
int vtkKWApplication::EvaluateBooleanExpression(const char* format, ...)
{
  va_list var_args1, var_args2;
  va_start(var_args1, format);
  va_start(var_args2, format);
  const char* result = this->ScriptInternal(format, var_args1, var_args2);
  va_end(var_args1);
  va_end(var_args2);
  if(vtkString::Equals(result, "1" ))
    {
    return 1;
    }
  return 0;
}

//----------------------------------------------------------------------------
const char* vtkKWApplication::ExpandFileName(const char* format, ...)
{
  ostrstream str;
  str << "eval file join {\"" << format << "\"}" << ends;
  va_list var_args1, var_args2;
  va_start(var_args1, format);
  va_start(var_args2, format);
  const char* result = this->ScriptInternal(str.str(), var_args1, var_args2);  
  va_end(var_args1);
  va_end(var_args2);
  str.rdbuf()->freeze(0);
  return result;
}

//----------------------------------------------------------------------------
const char* vtkKWApplication::Script(const char* format, ...)
{
  va_list var_args1, var_args2;
  va_start(var_args1, format);
  va_start(var_args2, format);
  const char* result = this->ScriptInternal(format, var_args1, var_args2);
  va_end(var_args1);
  va_end(var_args2);
  return result;
}

//----------------------------------------------------------------------------
const char* vtkKWApplication::ScriptInternal(const char* format,
                                             va_list var_args1,
                                             va_list var_args2)
{
  // We need a place to construct the script.
  char event[1600];
  char* buffer = event;
  
  // Estimate the length of the result string.  Never underestimates.
  int length = this->EstimateFormatLength(format, var_args1);
  
  // If our stack-allocated buffer is too small, allocate on one on
  // the heap that will be large enough.
  if(length > 1599)
    {
    buffer = new char[length+1];
    }
  
  // Print to the string.
  vsprintf(buffer, format, var_args2);
  
  // Evaluate the string in Tcl.
  if(Tcl_GlobalEval(this->MainInterp, buffer) != TCL_OK)
    {
    vtkErrorMacro("\n    Script: \n" << buffer
                  << "\n    Returned Error on line "
                  << this->MainInterp->errorLine << ": \n"  
                  << Tcl_GetStringResult(this->MainInterp) << endl);
    }
  
  // Free the buffer from the heap if we allocated it.
  if(buffer != event)
    {
    delete [] buffer;
    }
  
  // Convert the Tcl result to its string representation.
  return Tcl_GetStringResult(this->MainInterp);
}

//----------------------------------------------------------------------------
const char* vtkKWApplication::SimpleScript(const char* script)
{
  // Tcl might modify the script in-place.  We need a temporary copy.
  char event[1600];
  char* buffer = event;  
  
  // Make sure we have a script.
  int length = vtkString::Length(script);
  if(length < 1)
    {
    return 0;
    }
  
  // If our stack-allocated buffer is too small, allocate on one on
  // the heap that will be large enough.
  if(length > 1599)
    {
    buffer = new char[length+1];
    }
  
  // Copy the string to our buffer.
  strcpy(buffer, script);
  
  // Evaluate the string in Tcl.
  if(Tcl_GlobalEval(this->MainInterp, buffer) != TCL_OK)
    {
    vtkErrorMacro("\n    Script: \n" << buffer
                  << "\n    Returned Error on line "
                  << this->MainInterp->errorLine << ": \n"  
                  << Tcl_GetStringResult(this->MainInterp) << endl);
    }
  
  // Free the buffer from the heap if we allocated it.
  if(buffer != event)
    {
    delete [] buffer;
    }
  
  // Convert the Tcl result to its string representation.
  return Tcl_GetStringResult(this->MainInterp);
}

//----------------------------------------------------------------------------
void vtkKWApplication::SetApplicationName(const char *_arg)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting ApplicationName to " << _arg ); 
  if ( _arg && vtkString::Equals(this->ApplicationName,_arg) )
    { 
    return;
    } 
  if (this->ApplicationName) 
    { 
    delete [] this->ApplicationName; 
    } 
  if (_arg) 
    { 
    this->ApplicationName = vtkString::Duplicate(_arg);
    } 
   else 
    { 
    this->ApplicationName = NULL; 
    } 
  this->Modified(); 
}

void vtkKWApplication::SetApplicationVersionName(const char *_arg)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting ApplicationVersionName to " << _arg ); 
  if ( this->ApplicationVersionName && _arg && (!strcmp(this->ApplicationVersionName,_arg)))
    { 
    return;
    } 
  if (this->ApplicationVersionName) 
    { 
    delete [] this->ApplicationVersionName; 
    } 
  if (_arg) 
    { 
    this->ApplicationVersionName = vtkString::Duplicate(_arg);
    } 
   else 
    { 
    this->ApplicationVersionName = NULL; 
    } 
  this->Modified(); 
}

void vtkKWApplication::SetApplicationReleaseName(const char *_arg)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting ApplicationReleaseName to " << _arg ); 
  if ( this->ApplicationReleaseName && _arg && (!strcmp(this->ApplicationReleaseName,_arg)))
    { 
    return;
    } 
  if (this->ApplicationReleaseName) 
    { 
    delete [] this->ApplicationReleaseName; 
    } 
  if (_arg) 
    { 
    this->ApplicationReleaseName = vtkString::Duplicate(_arg);
    } 
   else 
    { 
    this->ApplicationReleaseName = NULL; 
    } 
  this->Modified(); 
}

void vtkKWApplication::Close(vtkKWWindow *win)
{
  if ( this->Windows )
    {
    this->Windows->RemoveItem(win);
    if (this->Windows->GetNumberOfItems() < 1)
      {
      this->Exit();
      }
    }
}

vtkKWWindowCollection *vtkKWApplication::GetWindows()
{
  return this->Windows;
}


void vtkKWApplication::AddWindow(vtkKWWindow *w)
{
  this->Windows->AddItem(w);
}

void vtkKWApplication::Exit()
{
  // Avoid a recursive exit.
  if (this->InExit)
    {
    return;
    }
  this->InExit = 1;
  vtkKWWindow* win = 0;
  this->Windows->InitTraversal();
  
  while (this->Windows && (win = this->Windows->GetNextKWWindow()))
    {
    win->SetPromptBeforeClose(0);
    win->Close();
    if (this->Windows)
      {
      this->Windows->InitTraversal();
      }
    }
  
  this->SetBalloonHelpPending(NULL);
  if (this->BalloonHelpWindow)
    {
    this->BalloonHelpWindow->Delete();
    this->BalloonHelpWindow = NULL;
    }
  if (this->BalloonHelpLabel)
    {
    this->BalloonHelpLabel->Delete();
    this->BalloonHelpLabel = NULL;
    }
//  this->SetBalloonHelpPending(NULL);

  if (this->SplashScreen)
    {
    this->SplashScreen->Delete();
    this->SplashScreen = NULL;
    }

  this->InExit = 0;
  this->Cleanup();

  this->ApplicationExited = 1;
  return;
}
    
#ifndef USE_INSTALLED_TCLTK_PACKAGES
/* The following constants define internal paths (not on disk)   */
/* for Tcl/Tk to use when looking for initialization scripts     */
/* which are in this file. They do not represent any hardwired   */
/* paths                                                         */
#define ET_TCL_LIBRARY "/ThisIsNotAPath/Tcl/lib/tcl8.2"
#define ET_TK_LIBRARY "/ThisIsNotAPath/Tcl/lib/tk8.2"
#endif

Tcl_Interp *vtkKWApplication::InitializeTcl(int argc, char *argv[])
{
  Tcl_Interp *interp;
  char *args;
  char buf[100];

#ifndef USE_INSTALLED_TCLTK_PACKAGES
  putenv("TCL_LIBRARY=" ET_TCL_LIBRARY);
  putenv("TK_LIBRARY=" ET_TK_LIBRARY);
#endif
  
  Tcl_FindExecutable(argv[0]);
  interp = Tcl_CreateInterp();
  args = Tcl_Merge(argc-1, argv+1);
  Tcl_SetVar(interp, "argv", args, TCL_GLOBAL_ONLY);
  ckfree(args);
  sprintf(buf, "%d", argc-1);
  Tcl_SetVar(interp, "argc", buf, TCL_GLOBAL_ONLY);
  Tcl_SetVar(interp, "argv0", argv[0], TCL_GLOBAL_ONLY);
  Tcl_SetVar(interp, "tcl_interactive", "0", TCL_GLOBAL_ONLY);

#ifdef USE_INSTALLED_TCLTK_PACKAGES
  Et_Interp = interp;

  if (Tcl_Init(interp) == TCL_ERROR ||
      Tk_Init(interp) == TCL_ERROR)
    {
    // ??
    }
  
  Tcl_StaticPackage(interp, "Tk", Tk_Init, 0);
#else
  Et_DoInit(interp);
#endif
  
  // create the SetApplicationIcon command
#ifdef _WIN32
  ApplicationIcon_DoInit(interp);
#endif

  // initialize VTK
  Vtktcl_Init(interp);

  // initialize Widgets
  if (vtkKWApplication::WidgetVisibility)
    {
    Vtkkwwidgetstcl_Init(interp);

    vtkKWBWidgets::Initialize(interp);
    }

  return interp;
}

void vtkKWApplication::Start()
{ 
  int i;
  
  // look at Tcl for any args
  this->Script("set argc");
  int argc = vtkKWObject::GetIntegerResult(this) + 1;
  char **argv = new char *[argc];
  argv[0] = NULL;
  for (i = 1; i < argc; i++)
    {
    this->Script("lindex $argv %d",i-1);
    argv[i] = strdup(this->GetMainInterp()->result);
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

void vtkKWApplication::Start(char *arg)
{ 
  this->Start(1,&arg);
}

void vtkKWApplication::Start(int /*argc*/, char ** /*argv*/)
{ 
  // Go to set it here since we do not have the application name in the
  // constructor.

  if (this->HasRegisteryValue(
    2, "RunTime", VTK_KW_SHOW_TOOLTIPS_REG_KEY))
    {
    this->ShowBalloonHelp = this->GetIntRegisteryValue(
      2, "RunTime", VTK_KW_SHOW_TOOLTIPS_REG_KEY);
    }
  else
    {
    this->ShowBalloonHelp = 1;
    }

  while (this->Windows && this->Windows->GetNumberOfItems())
    {
    this->DoOneTclEvent();
    }
  
  //Tk_MainLoop();
}

void vtkKWApplication::DoOneTclEvent()
{
  Tcl_DoOneEvent(0);
}

void vtkKWApplication::DisplayHelp(vtkKWWindow* master)
{
#ifdef _WIN32
  char temp[1024];
  char loc[1024];
  vtkKWRegisteryUtilities *reg = this->GetRegistery();
  sprintf(temp, "%s\\Setup", this->GetApplicationVersionName());
  if ( !reg )
    {
    vtkKWMessageDialog *dlg = vtkKWMessageDialog::New();
    dlg->SetMasterWindow(master);
    dlg->Create(this,"");
    dlg->SetText(
      "Internal error... Cannot get the registery.");
    dlg->Invoke();  
    dlg->Delete();
    }
  if ( reg->ReadValue( temp, "InstalledPath", loc ) )
    {
    sprintf(temp,"%s/%s.chm::/Introduction.htm",
            loc,this->ApplicationName);
    }
  else
    {
    sprintf(temp,"%s.chm::/Introduction.htm",
            this->ApplicationName);
    }
  
  if ( !HtmlHelp(NULL, temp, HH_DISPLAY_TOPIC, 0) )
    {
    vtkKWMessageDialog::PopupMessage(
      this, master,
      "Loading Help Error",
      "Help file cannot be displayed. This can be a result of "
      "the program being wrongly installed or help file being "
      "corrupted. Please reinstall this program.", 
      vtkKWMessageDialog::ErrorIcon);
    }
#else
  vtkKWMessageDialog *dlg = vtkKWMessageDialog::New();
  dlg->SetMasterWindow(master);
  dlg->Create(this,"");
  dlg->SetText(
    "HTML help is included in the help subdirectory of\n"
    "this application. You can view this help using a\n"
    "standard web browser by loading the Help.htm file.");
  dlg->Invoke();  
  dlg->Delete();
#endif
}


//----------------------------------------------------------------------------
void vtkKWApplication::BalloonHelpTrigger(vtkKWWidget *widget)
{
  if ( this->InExit )
    {
    return;
    }
  char *result;

  // If there is no help string, return

  if (!this->ShowBalloonHelp || 
      !widget->GetBalloonHelpString() || 
      this->BalloonHelpDelay <= 0)
    {
    this->SetBalloonHelpPending(NULL);
    return;
    }
  
  this->BalloonHelpCancel();
  this->SetBalloonHelpWidget(widget);
  this->Script("after %d {catch {%s BalloonHelpDisplay %s}}", 
               this->BalloonHelpDelay * 1000,
               this->GetTclName(), widget->GetTclName());
  result = this->GetMainInterp()->result;
  this->SetBalloonHelpPending(result);
}


//----------------------------------------------------------------------------
void vtkKWApplication::BalloonHelpDisplay(vtkKWWidget *widget)
{
  if ( this->InExit )
    {
    return;
    }
  if (!this->ShowBalloonHelp ||
      !this->BalloonHelpLabel || 
      !this->BalloonHelpWindow ||
      !widget->GetParent())
    {
    return;
    }
  int x, y;

  // If there is no help string, return
  if ( !widget->GetBalloonHelpString() )
    {
    this->SetBalloonHelpPending(NULL);
    return;
    }

  // make sure it is really pending
  this->BalloonHelpLabel->SetLabel(widget->GetBalloonHelpString());

  // Get the position of the mouse in the renderer.
  this->Script( "winfo pointerx %s", widget->GetWidgetName());
  x = vtkKWObject::GetIntegerResult(this);
  this->Script( "winfo pointery %s", widget->GetWidgetName());
  y = vtkKWObject::GetIntegerResult(this);

  // Get the position of the parent widget of the one needing help
  this->Script( "winfo rootx %s", widget->GetParent()->GetWidgetName());
  int xw = vtkKWObject::GetIntegerResult(this);
  this->Script( "winfo rooty %s", widget->GetParent()->GetWidgetName());

  // get the size of the balloon window
  this->Script( "winfo reqwidth %s", this->BalloonHelpLabel->GetWidgetName());
  int dx = vtkKWObject::GetIntegerResult(this);
  this->Script( "winfo reqheight %s", this->BalloonHelpLabel->GetWidgetName());
  
  // get the size of the parent window of the one needing help
  this->Script( "winfo width %s", widget->GetParent()->GetWidgetName());
  int dxw = vtkKWObject::GetIntegerResult(this);
  this->Script( "winfo height %s", widget->GetParent()->GetWidgetName());
  
  // Set the position of the window relative to the mouse.
  int just = widget->GetBalloonHelpJustification();

  // just 0 == left just 2 == right
  if (just)
    {
    if (x + dx > xw + dxw)
      {
      x = xw + dxw - dx;
      }
    }
  // with left justification (default) still try to keep the 
  // help from going past the right edge of the widget
  else
    {
     // if it goes too far right
    if (x + dx > xw + dxw)
      {
      // move it to the left
      x = xw + dxw - dx;
      // but not past the left edge of the parent widget
      if (x < xw)
        {
        x = xw;
        }
      }
    }
  
  this->Script("wm geometry %s +%d+%d",
               this->BalloonHelpWindow->GetWidgetName(), x, y+15);
  this->Script("update");

  // map the window
  if (this->BalloonHelpPending)
    {
    this->Script("wm deiconify %s", this->BalloonHelpWindow->GetWidgetName());
    this->Script("raise %s", this->BalloonHelpWindow->GetWidgetName());
    }
  
  this->SetBalloonHelpPending(NULL);

}


//----------------------------------------------------------------------------
void vtkKWApplication::BalloonHelpCancel()
{
  if ( this->InExit )
    {
    return;
    }
  if (this->BalloonHelpPending)
    {
    this->Script("after cancel %s", this->BalloonHelpPending);
    this->SetBalloonHelpPending(NULL);
    }
  if ( this->BalloonHelpWindow )
    {
    this->Script("wm withdraw %s",this->BalloonHelpWindow->GetWidgetName());
    }
  this->SetBalloonHelpWidget(0);
}


//----------------------------------------------------------------------------
void vtkKWApplication::BalloonHelpWithdraw()
{
  if ( this->InExit )
    {
    return;
    }
  if ( !this->BalloonHelpLabel || !this->BalloonHelpWindow )
    {
    return;
    }
  this->Script("wm withdraw %s",this->BalloonHelpWindow->GetWidgetName());
  if ( this->BalloonHelpWidget )
    {
    this->BalloonHelpTrigger(this->BalloonHelpWidget);
    }
}

//----------------------------------------------------------------------------
void vtkKWApplication::SetShowBalloonHelp(int v)
{
  if (this->ShowBalloonHelp == v)
    {
    return;
    }
  this->ShowBalloonHelp = v;
  this->Modified();

  if (!this->ShowBalloonHelp)
    {
    this->BalloonHelpCancel();
    }
}

//----------------------------------------------------------------------------
void vtkKWApplication::SetWidgetVisibility(int v)
{
  vtkKWApplication::WidgetVisibility = v;
}

//----------------------------------------------------------------------------
int vtkKWApplication::GetWidgetVisibility() 
{
  return vtkKWApplication::WidgetVisibility;
}

//----------------------------------------------------------------------------
void vtkKWApplication::DisplayAbout(vtkKWWindow* master)
{
  if ( this->InExit )
    {
    return;
    }
  ostrstream str;
  str << "Application : " << this->GetApplicationName() << "\nVersion : " << this->GetApplicationVersionName() << "\nRelease : " << this->GetApplicationReleaseName() << ends;

  char* msg = str.str();
  vtkKWMessageDialog *dlg = vtkKWMessageDialog::New();
  dlg->SetMasterWindow(master);
  dlg->Create(this,"");
  dlg->SetText(msg);
  dlg->Invoke();  
  dlg->Delete(); 
  delete[] msg;
}


//----------------------------------------------------------------------------
void vtkKWApplication::AddTraceEntry(const char *format, ...)
{
  if (this->TraceFile == NULL)
    {
    return;
    }
  
  char event[1600];
  char* buffer = event;
  
  va_list ap;
  va_start(ap, format);
  int length = this->EstimateFormatLength(format, ap);
  va_end(ap);
  
  if(length > 1599)
    {
    buffer = new char[length+1];
    }

  va_list var_args;
  va_start(var_args, format);
  vsprintf(buffer, format, var_args);
  va_end(var_args);

  *(this->TraceFile) << buffer << endl;
  
  if(buffer != event)
    {
    delete [] buffer;
    }
}

vtkKWRegisteryUtilities *vtkKWApplication::GetRegistery( const char*toplevel )
{
  this->GetRegistery();
  this->Registery->SetTopLevel( toplevel );
  return this->Registery;
}

vtkKWRegisteryUtilities *vtkKWApplication::GetRegistery()
{
  if ( !this->Registery )
    {
    this->Registery = vtkKWRegisteryUtilities::New();
    }
  return this->Registery;
}

void vtkKWApplication::SetBalloonHelpWidget( vtkKWWidget *widget )
{
  if ( this->InExit && widget )
    {
    return;
    }
  if ( this->BalloonHelpWidget )
    {
    this->BalloonHelpWidget->UnRegister(this);
    this->BalloonHelpWidget = 0;
    }
  if ( widget )
    {
    this->BalloonHelpWidget = widget;
    this->BalloonHelpWidget->Register(this);
    }  
}


int vtkKWApplication::GetMessageDialogResponse(const char* dialogname)
{
  char buffer[1024];
  int retval = 0;
  if ( this->GetRegisteryValue(3, "Dialogs", dialogname, buffer) )
    {
    retval = atoi(buffer);
    }
  return retval;
}

void vtkKWApplication::SetMessageDialogResponse(const char* dialogname, 
                                               int response)
{
  this->SetRegisteryValue(3, "Dialogs", dialogname, "%d", response);
}


int vtkKWApplication::SetRegisteryValue(int level, const char* subkey, 
                                        const char* key, 
                                        const char* format, ...)
{
  if ( this->GetRegisteryLevel() < 0 ||
       this->GetRegisteryLevel() < level )
    {
    return 0;
    }
  int res = 0;
  char buffer[100];
  char value[16000];
  sprintf(buffer, "%s\\%s", 
          this->GetApplication()->GetApplicationVersionName(),
          subkey);
  va_list var_args;
  va_start(var_args, format);
  vsprintf(value, format, var_args);
  va_end(var_args);
  
  vtkKWRegisteryUtilities *reg 
    = this->GetRegistery(this->GetApplicationName());
  res = reg->SetValue(buffer, key, value);
  return res;
}

int vtkKWApplication::GetRegisteryValue(int level, const char* subkey, 
                                        const char* key, char* value)
{
  int res = 0;
  char buff[1024];
  if ( !this->GetApplication() ||
       this->GetRegisteryLevel() < 0 ||
       this->GetRegisteryLevel() < level )
    {
    return 0;
    }
  char buffer[1024];
  sprintf(buffer, "%s\\%s", 
          this->GetApplicationVersionName(),
          subkey);

  vtkKWRegisteryUtilities *reg 
    = this->GetRegistery(this->GetApplicationName());
  res = reg->ReadValue(buffer, key, buff);
  if ( *buff && value )
    {
    *value = 0;
    strcpy(value, buff);
    }  
  return res;
}

int vtkKWApplication::DeleteRegisteryValue(int level, const char* subkey, 
                                      const char* key)
{
  if ( this->GetRegisteryLevel() < 0 ||
       this->GetRegisteryLevel() < level )
    {
    return 0;
    }
  int res = 0;
  char buffer[100];
  sprintf(buffer, "%s\\%s", 
          this->GetApplicationVersionName(),
          subkey);
  
  vtkKWRegisteryUtilities *reg 
    = this->GetRegistery(this->GetApplicationName());
  res = reg->DeleteValue(buffer, key);
  return res;
}

//----------------------------------------------------------------------------
int vtkKWApplication::HasRegisteryValue(int level, const char* subkey, 
                                        const char* key)
{
  char buffer[1024];
  return this->GetRegisteryValue(level, subkey, key, buffer);
}

//----------------------------------------------------------------------------
int vtkKWApplication::SelfTest()
{
  int res = 0;
  this->EvaluateString("foo");
  res += (!this->EvaluateBooleanExpression("proc a {} { return 1; }; a"));
  res += this->EvaluateBooleanExpression("proc a {} { return 0; }; a");

  return (res == 0);
}

//----------------------------------------------------------------------------
int vtkKWApplication::LoadScript(const char* filename)
{
  int res = 1;
  char* file = vtkString::Duplicate(filename);
  // add this window as a variable
  if ( Tcl_EvalFile(this->MainInterp, file) != TCL_OK )
    {
    vtkErrorMacro("\n    Script: \n" << filename 
                  << "\n    Returned Error on line "
                  << this->MainInterp->errorLine << ": \n      "  
                  << Tcl_GetStringResult(this->MainInterp) << endl);
    res = 0;
    this->SetExitStatus(1);
    }
  delete [] file;

  if ( this->ExitOnReturn )
    {
    this->Exit();
    }
  return res;
}

//----------------------------------------------------------------------------
float vtkKWApplication::GetFloatRegisteryValue(int level, const char* subkey, 
                                               const char* key)
{
  if ( this->GetRegisteryLevel() < 0 ||
       this->GetRegisteryLevel() < level )
    {
    return 0;
    }
  float res = 0;
  char buffer[1024];
  if ( this->GetRegisteryValue( 
         level, subkey, key, buffer ) )
    {
    res = atof(buffer);
    }
  return res;
}

//----------------------------------------------------------------------------
int vtkKWApplication::GetIntRegisteryValue(int level, const char* subkey, 
                                      const char* key)
{
  if ( this->GetRegisteryLevel() < 0 ||
       this->GetRegisteryLevel() < level )
    {
    return 0;
    }

  int res = 0;
  char buffer[1024];
  if ( this->GetRegisteryValue( 
         level, subkey, key, buffer ) )
    {
    res = atoi(buffer);
    }
  return res;
}

//----------------------------------------------------------------------------
int vtkKWApplication::BooleanRegisteryCheck(int level, 
                                            const char* subkey,
                                            const char* key, 
                                            const char* trueval)
{
  if ( this->GetRegisteryLevel() < 0 ||
       this->GetRegisteryLevel() < level )
    {
    return 0;
    }
  char buffer[1024];
  int allset = 0;
  if ( this->GetRegisteryValue(level, subkey, key, buffer) )
    {
    if ( !strncmp(buffer+1, trueval+1, vtkString::Length(trueval)-1) )
      {
      allset = 1;
      }
    }
  return allset;
}


//----------------------------------------------------------------------------
void vtkKWApplication::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "ApplicationName: " << this->GetApplicationName() << endl;
  os << indent << "MajorVersion: " << this->MajorVersion << endl;
  os << indent << "MinorVersion: " << this->MinorVersion << endl;
  os << indent << "ApplicationReleaseName: " 
     << this->GetApplicationReleaseName() << endl;
  os << indent << "ApplicationVersionName: " 
     << this->GetApplicationVersionName() << endl;
  os << indent << "ShowBalloonHelp: " << (this->ShowBalloonHelp ? "on":"off") << endl;
  os << indent << "BalloonHelpDelay: " << this->GetBalloonHelpDelay() << endl;
  os << indent << "DialogUp: " << this->GetDialogUp() << endl;
  os << indent << "ExitStatus: " << this->GetExitStatus() << endl;
  os << indent << "RegisteryLevel: " << this->GetRegisteryLevel() << endl;
  os << indent << "UseMessageDialogs: " << this->GetUseMessageDialogs() 
     << endl;
  os << indent << "ExitOnReturn: " << (this->ExitOnReturn ? "on":"off") << endl;
  if (this->SplashScreen)
    {
    os << indent << "SplashScreen: " << this->SplashScreen << endl;
    }
  else
    {
    os << indent << "SplashScreen: (none)" << endl;
    }
  os << indent << "HasSplashScreen: " << (this->HasSplashScreen ? "on":"off") << endl;
  os << indent << "ShowSplashScreen: " << (this->ShowSplashScreen ? "on":"off") << endl;
  os << indent << "ApplicationExited: " << this->ApplicationExited << endl;
}
