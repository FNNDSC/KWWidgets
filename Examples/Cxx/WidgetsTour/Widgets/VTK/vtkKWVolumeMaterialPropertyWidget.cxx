#include "vtkVolumeProperty.h"
#include "vtkKWApplication.h"
#include "vtkKWVolumeMaterialPropertyWidget.h"
#include "vtkKWWindow.h"

#include "vtkKWWidgetsTourExample.h"

class vtkKWVolumeMaterialPropertyWidgetItem : public KWWidgetsTourItem
{
public:
  virtual int GetType();
  virtual void Create(vtkKWWidget *parent, vtkKWWindow *);
};

void vtkKWVolumeMaterialPropertyWidgetItem::Create(vtkKWWidget *parent, vtkKWWindow *)
{
  vtkKWApplication *app = parent->GetApplication();

  // -----------------------------------------------------------------------

  // Create the volume property that will be modified by the widget

  vtkVolumeProperty *volprop1 = vtkVolumeProperty::New();

  // -----------------------------------------------------------------------

  // Create the material widget
  // Assign our volume property to the editor

  vtkKWVolumeMaterialPropertyWidget *volprop1_widget = 
    vtkKWVolumeMaterialPropertyWidget::New();
  volprop1_widget->SetParent(parent);
  volprop1_widget->Create();
  volprop1_widget->SetBalloonHelpString(
    "A volume material property widget.");

  volprop1_widget->SetVolumeProperty(volprop1);

  app->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
    volprop1_widget->GetWidgetName());

  // -----------------------------------------------------------------------

  // Create another material widget, in popup mode
  // Assign our volume property to the editor

  vtkKWVolumeMaterialPropertyWidget *volprop2_widget = 
    vtkKWVolumeMaterialPropertyWidget::New();
  volprop2_widget->SetParent(parent);
  volprop2_widget->PopupModeOn();
  volprop2_widget->Create();
  volprop2_widget->SetMaterialColor(0.3, 0.4, 1.0);
  volprop2_widget->SetBalloonHelpString(
    "A volume material property widget, created in popup mode. Note that "
    "it edits the same volume property object as the first widget.");

  volprop2_widget->SetVolumeProperty(volprop1);

  app->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 15", 
    volprop2_widget->GetWidgetName());

  // Both editor are linked to the same vol prop, so they should notify
  // each other of any changes to refresh the preview nicely

  volprop2_widget->SetPropertyChangingCommand(volprop1_widget, "Update");
  volprop2_widget->SetPropertyChangedCommand(volprop1_widget, "Update");

  volprop1_widget->SetPropertyChangingCommand(volprop2_widget, "Update");
  volprop1_widget->SetPropertyChangedCommand(volprop2_widget, "Update");

  volprop1_widget->Delete();
  volprop2_widget->Delete();
  volprop1->Delete();
}

int vtkKWVolumeMaterialPropertyWidgetItem::GetType()
{
  return KWWidgetsTourItem::TypeVTK;
}

KWWidgetsTourItem* vtkKWVolumeMaterialPropertyWidgetEntryPoint()
{
  return new vtkKWVolumeMaterialPropertyWidgetItem();
}
