#include "vtkKWColorSwatchesWidget.h"
#include "vtkKWApplication.h"
#include "vtkKWPushButton.h"
#include "vtkKWWindow.h"

#include "vtkKWWidgetsTourExample.h"

class vtkKWColorSwatchesWidgetItem : public KWWidgetsTourItem
{
public:
  virtual int GetType();
  virtual void Create(vtkKWWidget *parent, vtkKWWindow *);

  vtkKWColorSwatchesWidgetItem();
  ~vtkKWColorSwatchesWidgetItem();
  vtkKWColorSwatchesWidget *ColorSwatchesWidget;
};

vtkKWColorSwatchesWidgetItem::vtkKWColorSwatchesWidgetItem() 
{ 
  this->ColorSwatchesWidget = NULL; 
} 

vtkKWColorSwatchesWidgetItem::~vtkKWColorSwatchesWidgetItem() 
{ 
  if (this->ColorSwatchesWidget)
    {
    this->ColorSwatchesWidget->Delete(); 
    }
} 

void vtkKWColorSwatchesWidgetItem::Create(vtkKWWidget *parent, vtkKWWindow *win)
{
  vtkKWApplication *app = parent->GetApplication();

  // Create a color selector

  this->ColorSwatchesWidget = vtkKWColorSwatchesWidget::New();
  this->ColorSwatchesWidget->SetParent(parent);
  this->ColorSwatchesWidget->Create();
  this->ColorSwatchesWidget->SetBorderWidth(2);
  this->ColorSwatchesWidget->SetReliefToGroove();

  app->Script(
    "pack %s -side top -anchor nw -expand y -fill none -padx 2 -pady 2", 
    this->ColorSwatchesWidget->GetWidgetName());

  this->ColorSwatchesWidget->AddDefaultCollections();
}

int vtkKWColorSwatchesWidgetItem::GetType()
{
  return KWWidgetsTourItem::TypeComposite;
}

KWWidgetsTourItem* vtkKWColorSwatchesWidgetEntryPoint()
{
  return new vtkKWColorSwatchesWidgetItem();
}
