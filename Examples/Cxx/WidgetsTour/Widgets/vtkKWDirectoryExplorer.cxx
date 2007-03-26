#include "vtkKWDirectoryExplorer.h"
#include "vtkKWApplication.h"
#include "vtkKWWindow.h"

#include "vtkKWWidgetsTourExample.h"

class vtkKWDirectoryExplorerItem : public KWWidgetsTourItem
{
public:
  virtual int GetType();
  virtual void Create(vtkKWWidget *parent, vtkKWWindow *);
};

void vtkKWDirectoryExplorerItem::Create(vtkKWWidget *parent, vtkKWWindow *)
{
  vtkKWApplication *app = parent->GetApplication();

  // -----------------------------------------------------------------------

  // Create a directory explorer widget

  vtkKWDirectoryExplorer *dir_explorer_widget1 = vtkKWDirectoryExplorer::New();
  dir_explorer_widget1->SetParent(parent);
  dir_explorer_widget1->Create();
  dir_explorer_widget1->SetBorderWidth(2);
  dir_explorer_widget1->SetReliefToGroove();
  dir_explorer_widget1->SetPadX(2);
  dir_explorer_widget1->SetPadY(2);

  app->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 2", 
    dir_explorer_widget1->GetWidgetName());

  dir_explorer_widget1->Delete();
}

int vtkKWDirectoryExplorerItem::GetType()
{
  return KWWidgetsTourItem::TypeComposite;
}

KWWidgetsTourItem* vtkKWDirectoryExplorerEntryPoint()
{
  return new vtkKWDirectoryExplorerItem();
}
