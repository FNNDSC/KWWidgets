/*=========================================================================

  Module:    $RCSfile: vtkKWApplication.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWApplication - manage all the windows in an application
// .SECTION Description
// vtkKWApplication is the overall class that represents the entire 
// applicaiton. It is a fairly small class that is primarily responsible
// for managing the all the vtkKWWindows in the application. 


#ifndef __vtkKWApplication_h
#define __vtkKWApplication_h

#include "vtkKWObject.h"
#include "vtkTcl.h" // Needed for Tcl_Interp
#include "vtkTk.h" // Needed for Tk_Window

class vtkKWLabel;
class vtkKWMessageDialog;
class vtkKWMessageDialog;
class vtkKWRegisteryUtilities;
class vtkKWSplashScreen;
class vtkKWWidget;
class vtkKWWindow;
class vtkKWWindowCollection;
class vtkKWText;

//BTX
template<class KeyType,class DataType> class vtkAbstractMap;
//ETX

class VTK_EXPORT vtkKWApplication : public vtkKWObject
{
public:
  static vtkKWApplication* New();
  vtkTypeRevisionMacro(vtkKWApplication,vtkKWObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  virtual vtkKWApplication *GetApplication()  { return this;  }
  virtual void SetApplication (vtkKWApplication*);
  
  // Description:
  // Start running the main application.
  virtual void Start();
  virtual void Start(char *arg);
  virtual void Start(int argc, char *argv[]);

  // Description:
  // Initialize Tcl/Tk
  // Return NULL on error (eventually provides an ostream where detailed
  // error messages will be stored).
  //BTX
  static Tcl_Interp *InitializeTcl(int argc, char *argv[], ostream *err = 0);
  //ETX
  
  // Description:
  // Get the interpreter being used by this application
  Tcl_Interp *GetMainInterp() {return this->MainInterp;};
  static Tcl_Interp *GetGlobalInterp() {return vtkKWApplication::GlobalInterp;};

  // Description:
  // The method to invoke when the user exits the app
  virtual void Exit();

  // Description:
  // This method is invoked when a window closes
  virtual void Close(vtkKWWindow *);

  // Description:
  // Display the on-line help and about dialog for this application.
  virtual void DisplayHelp(vtkKWWindow *master);
  virtual void DisplayAbout(vtkKWWindow *master);

  // Description:
  // Set or get the ExitOnReturn flag. If this flag is set, then 
  // the next tcl script will make application exit when return.
  // Useful for tests. 
  vtkSetClampMacro(ExitOnReturn, int, 0, 1);
  vtkBooleanMacro(ExitOnReturn, int);
  vtkGetMacro(ExitOnReturn, int);

  // Description:
  // Add a window to this application.
  void AddWindow(vtkKWWindow *w);
  //BTX
  vtkKWWindowCollection *GetWindows();
  //ETX
  
  // Description:
  // Set/Get the ApplicationName
  vtkSetStringMacro(ApplicationName);
  vtkGetStringMacro(ApplicationName);

  // Description:
  // Set/Get the major and minor version.
  vtkGetMacro(MajorVersion, int);
  vtkGetMacro(MinorVersion, int);

  // Description:
  // Set/Get the ApplicationVersionName - this is the name + version number
  // (no spaces, usually used as the master key to store registery settings,
  //  ex: VolView20, ParaView1.1, etc)
  vtkSetStringMacro(ApplicationVersionName);
  vtkGetStringMacro(ApplicationVersionName);

  // Description:
  // Set/Get the ApplicationReleaseName - this is the release of the 
  // application version, typically: beta1, beta2, final, patch1, patch2
  vtkSetStringMacro(ApplicationReleaseName);
  vtkGetStringMacro(ApplicationReleaseName);

  // Description:
  // Get the "pretty" name for the application. This is most of the time
  // used as windows/dialog title, About box, etc. It combines the application
  // name, its mode (limited edition or not) and its version.
  virtual const char* GetApplicationPrettyName();

  // Descrition:
  // Set/Get if the application is running in limited edition mode.
  // GetLimitedEditionModeAndWarn will return the limited edition mode ; if 
  // the mode is true, it will also display a popup warning stating that a
  // feature (string parameter) is not available in this mode.
  // Also set/Get the name of the application when in Limited Edition Mode, 
  // as displayed in the GUI or after in the title bar instead of the app name.
  //BTX 
  virtual void SetLimitedEditionMode(int arg);
  vtkBooleanMacro(LimitedEditionMode, int);
  vtkGetMacro(LimitedEditionMode, int);
  //ETX
  virtual int GetLimitedEditionModeAndWarn(const char *feature);
  vtkSetStringMacro(LimitedEditionModeName);
  vtkGetStringMacro(LimitedEditionModeName);

  // Description:
  // Set/Get the directory in which the current application is supposed
  // to be installed.
  vtkGetStringMacro(ApplicationInstallationDirectory);
  
  // Description:
  // Load script from a file. Resturn if script was successful.
  static int LoadTclScript(const char* filename);
  int LoadScript(const char* filename);

  // Description:
  // A convienience method to invoke some tcl script code and
  // perform arguement substitution.
  const char* SimpleScript(const char* script);
  //BTX
  virtual const char* Script(const char* format, ...);
  const char* EvaluateString(const char* format, ...);
  const char* ExpandFileName(const char* format, ...);
  int EvaluateBooleanExpression(const char* format, ...);
  //ETX
  
  // Description:
  // Internal implementation method for Script invocation.  This
  // function needs to walk through the var_args list twice.  The only
  // portable way to do this is to pass two copies of the list's start
  // pointer.
  const char* ScriptInternal(const char* format, va_list var_args1,
                             va_list var_args2);
  //ETX

  // Description:
  // Test some of the features that cannot be tested from the tcl.
  int SelfTest();

  // Description:
  // Internal Balloon help callbacks.
  void BalloonHelpTrigger(vtkKWWidget *widget);
  void BalloonHelpDisplay(vtkKWWidget *widget);
  void BalloonHelpCancel();
  void BalloonHelpWithdraw();
  void SetBalloonHelpWidget(vtkKWWidget *widget);

  // Description:
  // Set the delay for the balloon help in seconds.
  // To disable balloon help, set it to 0.
  vtkSetClampMacro(BalloonHelpDelay, int, 0, 5);
  vtkGetMacro(BalloonHelpDelay, int);

  // Description:
  // Show balloon help.
  virtual void SetShowBalloonHelp(int);
  vtkGetMacro(ShowBalloonHelp, int);
  vtkBooleanMacro(ShowBalloonHelp, int);

  // Description:
  // This variable can be used to hide the user interface.  
  // When WidgetVisibility is off, The cherat methods of vtkKWWidgets 
  // should not create the TK widgets.
  static void SetWidgetVisibility(int v);
  static int GetWidgetVisibility();
  vtkBooleanMacro(WidgetVisibility, int);
  
  // Description:
  // This can be used to trace the application.
  // Look at vtkKWWidgets to see how it is used.
  ofstream *GetTraceFile() {return this->TraceFile;}
  virtual void AddSimpleTraceEntry(const char* trace);
  //BTX
  virtual void AddTraceEntry(const char* format, ...);
  //ETX
  virtual int GetApplicationKey() {return -1;};

  //BTX
  // Description:
  // Return the Registery object. It is created on first use
  // and deleted on exiting the application.
  vtkKWRegisteryUtilities *GetRegistery( const char* toplevel);
  vtkKWRegisteryUtilities *GetRegistery( );
  //ETX

  // Description:
  // Set or get the current registery level. If the called
  // registery level is lower than current one, the operation is 
  // successfull. Registery level -1 means to ignore all the 
  // registery operations.
  vtkSetClampMacro(RegisteryLevel, int, -1, 10);
  vtkGetMacro(RegisteryLevel, int);

  // Description:
  // This value will be returned by application1 at exit.
  // Use this from scripts if you want ParaView exit with an
  // error status (for example to indicate that a regression test 
  // failed)
  vtkSetMacro(ExitStatus, int);
  vtkGetMacro(ExitStatus, int);

  // Description:
  // When a modal dialog is up, this flag should be set.
  // vtkKWWindow will check this at exit and if set,
  // it will refuse to exit
  vtkSetMacro(DialogUp, int);
  vtkGetMacro(DialogUp, int);

  // Description:
  // This method will suppress all message dialogs.
  // There is no way back for now.
  void SupressMessageDialogs() { this->UseMessageDialogs = 0; }
  vtkGetMacro(UseMessageDialogs, int);

  // Description:
  // This method returns a message dialog response.
  // The string is the name of dialog and the return is:
  // 0 for no response (display dialog), 1 for response ok/yes
  // and -1 for response cancel/no.
  int GetMessageDialogResponse(const char* dialogname);
  
  // Description:
  // Set the message dialog response. Set 1 for ok/yes and -1 for 
  // cancel/no.
  void SetMessageDialogResponse(const char* dialogname, int response);

  //Description:
  // Set or get the registery value for the application.
  // When storing multiple arguments, separate with spaces.
  // If the level is lower than current registery level, operation 
  // will be successfull.
  //BTX
  int SetRegisteryValue(int level, const char* subkey, const char* key, 
                        const char* format, ...);
  //ETX
  int GetRegisteryValue(int level, const char* subkey, const char* key, 
                        char* value);
  int DeleteRegisteryValue(int level, const char* subkey, const char* key);
  int HasRegisteryValue(int level, const char* subkey, const char* key);
  
  // Description:
  // Perform a boolean check of the value in registery. If the value
  // at the key is trueval, then return true, otherwise return false.
  int BooleanRegisteryCheck(int level, const char* subkey, const char* key, 
                            const char* trueval);

  // Description:
  // Get float registery value (zero if not found).
  // If the level is lower than current registery level, operation 
  // will be successfull.
  float GetFloatRegisteryValue(int level, const char* subkey, 
                               const char* key);
  int   GetIntRegisteryValue(int level, const char* subkey, const char* key);
  
  // Description:
  // Get the splash screen, if this app have/show a splash screen.
  vtkGetObjectMacro(SplashScreen, vtkKWSplashScreen);
  vtkGetMacro(HasSplashScreen, int);
  vtkGetMacro(ShowSplashScreen, int);
  vtkSetMacro(ShowSplashScreen, int);
  vtkBooleanMacro(ShowSplashScreen, int);

  // Description:
  // Save windows geometry.
  vtkGetMacro(SaveWindowGeometry, int);
  vtkSetMacro(SaveWindowGeometry, int);
  vtkBooleanMacro(SaveWindowGeometry, int);

  // Description:
  // At the end of Exit(), this is set to true. Other objects
  // can use this to cleanup properly.
  vtkGetMacro(ApplicationExited, int);

  // Description:
  // This return 1 when application is exiting.
  vtkGetMacro(InExit, int);

  // Descrition:
  // Get those application settings that are stored in the registery
  // Should be called once the application name is known (and the registery
  // level set).
  virtual void GetApplicationSettingsFromRegistery();

  // Description:
  // Get/Set the internal character encoding of the application.
  virtual void SetCharacterEncoding(int val);
  vtkGetMacro(CharacterEncoding, int);
  
  // Description:
  // Test if we have some logic to check for application update and
  // eventually perform that check.
  virtual int HasCheckForUpdates();
  virtual void CheckForUpdates();

  // Description:
  // Test if we have some logic to report a bug and
  // eventually report a bug check.
  virtual int CanEmailFeedback();
  virtual void EmailFeedback();
  vtkSetStringMacro(EmailFeedbackAddress);
  vtkGetStringMacro(EmailFeedbackAddress);

  // Description:
  // Convenience method to call UpdateEnableState on all windows
  virtual void UpdateEnableStateForAllWindows();

  // Description:
  // Convenience method to get the operating system version
  virtual int GetSystemVersion(ostream &os);

protected:
  vtkKWApplication();
  ~vtkKWApplication();

  // Description:
  // Do one tcl event and whatever application might want to do.
  virtual void DoOneTclEvent();

  // Description:
  // Cleanup everything before exiting.
  virtual void Cleanup() { };

  Tk_Window MainWindow;
  Tcl_Interp *MainInterp;
  static Tcl_Interp *GlobalInterp;
  vtkKWWindowCollection *Windows;

  char *ApplicationName;
  char *ApplicationVersionName;
  char *ApplicationReleaseName;
  int MajorVersion;
  int MinorVersion;

  char *ApplicationPrettyName;
  vtkSetStringMacro(ApplicationPrettyName);

  vtkSetStringMacro(ApplicationInstallationDirectory);
  char *ApplicationInstallationDirectory;
  virtual void FindApplicationInstallationDirectory();

  virtual void AddEmailFeedbackBody(ostream &);
  virtual void AddEmailFeedbackSubject(ostream &);
  char *EmailFeedbackAddress;

  int ApplicationExited;

  // For Balloon help
  vtkKWWidget *BalloonHelpWindow;
  vtkKWLabel *BalloonHelpLabel;
  char *BalloonHelpPending;
  vtkSetStringMacro(BalloonHelpPending);
  vtkKWWidget *BalloonHelpWidget;
  int ShowBalloonHelp;

  vtkGetStringMacro(DisplayHelpStartingPage);
  vtkSetStringMacro(DisplayHelpStartingPage);
  char *DisplayHelpStartingPage;

  static int WidgetVisibility;
  int InExit;
  int DialogUp;

  int ExitStatus;
  int LimitedEditionMode;
  char *LimitedEditionModeName;

  ofstream *TraceFile;

  vtkKWRegisteryUtilities *Registery;
  int RegisteryLevel;
  int BalloonHelpDelay;

  int UseMessageDialogs;

  int ExitOnReturn;

  // Splash screen

  virtual void CreateSplashScreen() {};
  vtkKWSplashScreen *SplashScreen;
  int HasSplashScreen;
  int ShowSplashScreen;

  int SaveWindowGeometry;

  // About dialog

  virtual void ConfigureAbout();
  virtual void AddAboutText(ostream &);
  virtual void AddAboutCopyrights(ostream &);
  vtkKWMessageDialog *AboutDialog;
  vtkKWLabel         *AboutDialogImage;
  vtkKWText          *AboutRuntimeInfo;
  vtkKWWidget        *AboutScrollBar;

  int CharacterEncoding;

  // Description:
  // Check for an argument (example: --foo, /C, -bar, etc).
  // Return VTK_OK if found and set 'index' to the position of the 
  // argument in argv[].
  // Return VTK_ERROR if not found.
  static int CheckForArgument(
    int argc, char* argv[], const char *arg, int &index);

  // Description:
  // Check for a valued argument (example: --foo=bar, /C=bar, -bar=foo, etc).
  // Return VTK_OK if found and set 'index' to the position of the 
  // argument in argv[], 'value_pos' to the position right after the '='
  // in that argument.
  // Return VTK_ERROR if not found.
  static int CheckForValuedArgument(
    int argc, char* argv[], const char *arg, int &index, int &value_pos);

  virtual int GetCheckForUpdatesPath(ostream &path);

private:
  vtkKWApplication(const vtkKWApplication&);   // Not implemented.
  void operator=(const vtkKWApplication&);  // Not implemented.
};

#endif
