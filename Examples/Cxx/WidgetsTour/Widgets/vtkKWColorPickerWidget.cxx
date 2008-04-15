#include "vtkKWColorPickerWidget.h"
#include "vtkKWApplication.h"
#include "vtkKWPushButton.h"
#include "vtkKWWindow.h"

#include "vtkKWWidgetsTourExample.h"

class vtkKWColorPickerWidgetItem : public KWWidgetsTourItem
{
public:
  virtual int GetType();
  virtual void Create(vtkKWWidget *parent, vtkKWWindow *);

  vtkKWColorPickerWidgetItem();
  ~vtkKWColorPickerWidgetItem();
  vtkKWColorPickerWidget *ColorPickerWidget;
};

vtkKWColorPickerWidgetItem::vtkKWColorPickerWidgetItem() 
{ 
  this->ColorPickerWidget = NULL; 
} 

vtkKWColorPickerWidgetItem::~vtkKWColorPickerWidgetItem() 
{ 
  if (this->ColorPickerWidget)
    {
    this->ColorPickerWidget->Delete(); 
    }
} 

void vtkKWColorPickerWidgetItem::Create(vtkKWWidget *parent, vtkKWWindow *win)
{
  vtkKWApplication *app = parent->GetApplication();

  // Create a color selector

  this->ColorPickerWidget = vtkKWColorPickerWidget::New();
  this->ColorPickerWidget->SetParent(parent);
  this->ColorPickerWidget->Create();
  this->ColorPickerWidget->SetBorderWidth(2);
  this->ColorPickerWidget->SetReliefToGroove();

  app->Script(
    "pack %s -side top -anchor nw -expand y -fill none -padx 2 -pady 2", 
    this->ColorPickerWidget->GetWidgetName());
}

int vtkKWColorPickerWidgetItem::GetType()
{
  return KWWidgetsTourItem::TypeComposite;
}

KWWidgetsTourItem* vtkKWColorPickerWidgetEntryPoint()
{
  return new vtkKWColorPickerWidgetItem();
}
