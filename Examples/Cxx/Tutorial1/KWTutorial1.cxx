#include "vtkKWApplication.h"
#include "vtkKWWindowBase.h"
#include "vtkKWMyWidget.h"
#include "vtkKWLabel.h"

#include <vtksys/SystemTools.hxx>

int my_main(int argc, char *argv[])
{
  // Initialize Tcl

  Tcl_Interp *interp = vtkKWApplication::InitializeTcl(argc, argv, &cerr);
  if (!interp)
    {
    cerr << "Error: InitializeTcl failed" << endl ;
    return 1;
    }

  // Create the application

  vtkKWApplication *app = vtkKWApplication::New();
  app->SetName("KWTutorial1");

  // Add a window

  vtkKWWindowBase *win = vtkKWWindowBase::New();
  app->AddWindow(win);
  win->Create();

  // Add our widget
  
  vtkKWMyWidget *my_widget = vtkKWMyWidget::New();
  my_widget->SetParent(win->GetViewFrame());
  my_widget->Create();
  my_widget->GetLabel()->SetText("Agree?");

  app->Script("pack %s -side left -anchor c -expand y", 
              my_widget->GetWidgetName());

  // Start the application

  int ret = 0;
  win->Display();
  app->Start(argc, argv);
  ret = app->GetExitStatus();
  win->Close();

  // Deallocate and exit

  my_widget->Delete();
  win->Delete();
  app->Delete();
  
  return ret;
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
