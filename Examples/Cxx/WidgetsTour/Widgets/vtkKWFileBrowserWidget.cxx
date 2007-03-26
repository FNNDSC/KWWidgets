#include "vtkKWFileBrowserWidget.h"
#include "vtkKWApplication.h"
#include "vtkKWWindow.h"

#include "vtkKWWidgetsTourExample.h"

class vtkKWFileBrowserWidgetItem : public KWWidgetsTourItem
{
public:
  virtual int GetType();
  virtual void Create(vtkKWWidget *parent, vtkKWWindow *);
};

void vtkKWFileBrowserWidgetItem::Create(vtkKWWidget *parent, vtkKWWindow *)
{
  vtkKWApplication *app = parent->GetApplication();

  // -----------------------------------------------------------------------

  // Create a file browser widget

  vtkKWFileBrowserWidget *filebrowser_widget1 = vtkKWFileBrowserWidget::New();
  filebrowser_widget1->SetParent(parent);
  filebrowser_widget1->Create();
  filebrowser_widget1->SetBorderWidth(2);
  filebrowser_widget1->SetReliefToGroove();
  filebrowser_widget1->SetPadX(2);
  filebrowser_widget1->SetPadY(2);
  filebrowser_widget1->SetWidth(760);

  app->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 2", 
    filebrowser_widget1->GetWidgetName());

  filebrowser_widget1->Delete();
}

int vtkKWFileBrowserWidgetItem::GetType()
{
  return KWWidgetsTourItem::TypeComposite;
}

KWWidgetsTourItem* vtkKWFileBrowserWidgetEntryPoint()
{
  return new vtkKWFileBrowserWidgetItem();
}
