#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWApplication.h"
#include "vtkKWWindow.h"

#include "vtkKWWidgetsTourExample.h"

class vtkKWLoadSaveButtonItem : public KWWidgetsTourItem
{
public:
  virtual int GetType();
  virtual void Create(vtkKWWidget *parent, vtkKWWindow *);
};

void vtkKWLoadSaveButtonItem::Create(vtkKWWidget *parent, vtkKWWindow *win)
{
  vtkKWApplication *app = parent->GetApplication();
  win->InsertRecentFilesMenu(0, NULL);

  // -----------------------------------------------------------------------

  // Create a load button to pick a file

  vtkKWLoadSaveButton *load_button1 = vtkKWLoadSaveButton::New();
  load_button1->SetParent(parent);
  load_button1->Create();
  load_button1->SetText("Click to Pick a File");
  load_button1->GetLoadSaveDialog()->SaveDialogOff(); // load mode

  char command[1024];
  sprintf(command, "AddRecentFile [%s GetFileName] {} {}",
          load_button1->GetLoadSaveDialog()->GetTclName());
  load_button1->SetCommand(win, command);

  app->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
    load_button1->GetWidgetName());

  load_button1->Delete();

  // -----------------------------------------------------------------------

  // Create a load button to pick a directory

  vtkKWLoadSaveButton *load_button2 = vtkKWLoadSaveButton::New();
  load_button2->SetParent(parent);
  load_button2->Create();
  load_button2->SetText("Click to Pick a Directory");
  load_button2->GetLoadSaveDialog()->ChooseDirectoryOn();

  app->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 4", 
    load_button2->GetWidgetName());

  load_button2->Delete();
}

int vtkKWLoadSaveButtonItem::GetType()
{
  return KWWidgetsTourItem::TypeComposite;
}

KWWidgetsTourItem* vtkKWLoadSaveButtonEntryPoint()
{
  return new vtkKWLoadSaveButtonItem();
}
