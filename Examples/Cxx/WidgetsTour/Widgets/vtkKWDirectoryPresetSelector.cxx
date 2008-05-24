#include "vtkKWDirectoryPresetSelector.h"
#include "vtkKWApplication.h"
#include "vtkKWWindow.h"
#include "vtkKWLabelWithLabel.h"
#include "vtkKWLabel.h"

#include "vtkKWWidgetsTourExample.h"

#if WIN32
#define PATH_SEP ';'
#else
#define PATH_SEP ':'
#endif

class vtkKWDirectoryPresetSelectorItem : public KWWidgetsTourItem
{
public:
  virtual int GetType();
  virtual void Create(vtkKWWidget *parent, vtkKWWindow *);
};

void vtkKWDirectoryPresetSelectorItem::Create(vtkKWWidget *parent, vtkKWWindow *)
{
  vtkKWApplication *app = parent->GetApplication();

  // -----------------------------------------------------------------------

  // Create a directory preset selector

  vtkKWDirectoryPresetSelector *dps1 = vtkKWDirectoryPresetSelector::New();
  dps1->SetParent(parent);
  dps1->Create();
  dps1->SetBorderWidth(2);
  dps1->SetReliefToGroove();
  dps1->SetPadX(2);
  dps1->SetPadY(2);
  dps1->SetMaximumDirectoryLength(75);
  dps1->SetHelpLabelVisibility(1);
  dps1->GetHelpLabel()->GetWidget()->SetText("This work is part of the National Alliance for Medical Image Computing (NAMIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149. Information on the National Centers for Biomedical Computing can be obtained from http://nihroadmap.nih.gov/bioinformatics.");

  dps1->AddEnabledPresetDirectoriesFromDelimitedString(
    (const char*)getenv("PATH"), PATH_SEP);

  app->Script(
    "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2", 
    dps1->GetWidgetName());
  
  dps1->Delete();
}

int vtkKWDirectoryPresetSelectorItem::GetType()
{
  return KWWidgetsTourItem::TypeComposite;
}

KWWidgetsTourItem* vtkKWDirectoryPresetSelectorEntryPoint()
{
  return new vtkKWDirectoryPresetSelectorItem();
}
