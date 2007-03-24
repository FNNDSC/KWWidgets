#include "vtkKWFileBrowserDialog.h"
#include "vtkKWApplication.h"

#include <vtksys/SystemTools.hxx>
#include <vtksys/CommandLineArguments.hxx>

int my_main(int argc, char *argv[])
{
  // Initialize Tcl

  Tcl_Interp *interp = vtkKWApplication::InitializeTcl(argc, argv, &cerr);
  if (!interp)
    {
    cerr << "Error: InitializeTcl failed" << endl ;
    return 1;
    }

  // Process some command-line arguments
  // The --test option here is used to run this example as a non-interactive 
  // test for software quality purposes. You can ignore it.

  int option_test = 0;
  vtksys::CommandLineArguments args;
  args.Initialize(argc, argv);
  args.AddArgument(
    "--test", vtksys::CommandLineArguments::NO_ARGUMENT, &option_test, "");
  args.Parse();
  
  // Create the application
  // If --test was provided, ignore all registry settings, and exit silently
  // Restore the settings that have been saved to the registry, like
  // the geometry of the user interface so far.

  vtkKWApplication *app = vtkKWApplication::New();
  app->SetName("KWFileBrowserDialogExample");
  if (option_test)
    {
    app->SetRegistryLevel(0);
    }
  app->PromptBeforeExitOff();
  app->RestoreApplicationSettingsFromRegistry();

  // Set a help link. Can be a remote link (URL), or a local file

  app->SetHelpDialogStartingPage("http://www.kwwidgets.org");

  // Create the file browser dialog

  vtkKWFileBrowserDialog *dlg = vtkKWFileBrowserDialog::New();
  dlg->SetApplication(app);
  dlg->Create();
  dlg->SetTitle(app->GetName());
  //dlg->ChooseDirectoryOn();
  //dlg->MultipleSelectionOn();
  dlg->SetDefaultExtension(".txt");
  dlg->SetFileTypes("{{All files} {.*}} {{Text Document} {.txt}} {{JPEG image} {.jpg .jpeg}}");
  //dlg->SetInitialFileName("cmakecache.txt");

  // Invoke the dialog
    
  dlg->RetrieveLastPathFromRegistry("KWFileBrowserLastPath");

  dlg->Invoke();
    
  if (dlg->GetStatus()==vtkKWDialog::StatusOK)
    {
    dlg->SaveLastPathToRegistry("KWFileBrowserLastPath");
    }
  dlg->Delete();
    
  app->Exit();
  
  app->Delete();
  
  return 0;
}

#if defined(_WIN32) && !defined(__CYGWIN__)
#include <windows.h>
int __stdcall WinMain(HINSTANCE, HINSTANCE, LPSTR lpCmdLine, int)
{
  int argc;
  char **argv;
  vtksys::SystemTools::ConvertWindowsCommandLineToUnixArguments(
    lpCmdLine, &argc, &argv);
  int ret = my_main(argc, argv);
  for (int i = 0; i < argc; i++) { delete [] argv[i]; }
  delete [] argv;
  return ret;
}
#else
int main(int argc, char *argv[])
{
  return my_main(argc, argv);
}
#endif
