#include "vtkKWFileListTable.h"
#include "vtkKWApplication.h"
#include "vtkKWWindow.h"

#include "vtkKWWidgetsTourExample.h"

class vtkKWFileListTableItem : public KWWidgetsTourItem
{
public:
  virtual int GetType();
  virtual void Create(vtkKWWidget *parent, vtkKWWindow *);
};

void vtkKWFileListTableItem::Create(vtkKWWidget *parent, vtkKWWindow *)
{
  vtkKWApplication *app = parent->GetApplication();

  // -----------------------------------------------------------------------

  // Create a file list table widget

  vtkKWFileListTable *filetable_widget1 = vtkKWFileListTable::New();
  filetable_widget1->SetParent(parent);
  filetable_widget1->Create();
  filetable_widget1->SetBorderWidth(2);
  filetable_widget1->SetReliefToGroove();
  filetable_widget1->SetPadX(2);
  filetable_widget1->SetPadY(2);

#ifdef _WIN32
  filetable_widget1->SetParentDirectory("C:");
#else
  filetable_widget1->SetParentDirectory("/");
#endif

  app->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 2", 
    filetable_widget1->GetWidgetName());

  filetable_widget1->Delete();
}

int vtkKWFileListTableItem::GetType()
{
  return KWWidgetsTourItem::TypeComposite;
}

KWWidgetsTourItem* vtkKWFileListTableEntryPoint()
{
  return new vtkKWFileListTableItem();
}
