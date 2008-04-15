#include "vtkKWColorPresetSelector.h"
#include "vtkKWApplication.h"
#include "vtkKWWindow.h"

#include <vtksys/stl/string>
#include "vtkKWWidgetsTourExample.h"

class vtkKWColorPresetSelectorItem : public KWWidgetsTourItem
{
public:
  virtual int GetType();
  virtual void Create(vtkKWWidget *parent, vtkKWWindow *);
};

void vtkKWColorPresetSelectorItem::Create(vtkKWWidget *parent, vtkKWWindow *)
{
  vtkKWApplication *app = parent->GetApplication();

  // -----------------------------------------------------------------------

  // Create a push button

  vtkKWColorPresetSelector *cps1 = vtkKWColorPresetSelector::New();
  cps1->SetParent(parent);
  cps1->Create();

  int id;
  double hsv[3] = {0.0, 1.0, 1.0};
  
  for (hsv[0] = 0.0; hsv[0] < 1.0; hsv[0] += 0.01)
    {
    id = cps1->AddPreset();
    cps1->SetPresetColorAsHSV(id, hsv[0], hsv[1], hsv[2]);
    cps1->SetPresetCommentAsHexadecimalRGB(id);
    }

  app->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
    cps1->GetWidgetName());

  cps1->Delete();
}

int vtkKWColorPresetSelectorItem::GetType()
{
  return KWWidgetsTourItem::TypeComposite;
}

KWWidgetsTourItem* vtkKWColorPresetSelectorEntryPoint()
{
  return new vtkKWColorPresetSelectorItem();
}
