#include "vtkKWLabel.h"
#include "vtkKWLabelWithLabel.h"
#include "vtkKWLabelSet.h"
#include "vtkKWLabelWithLabelSet.h"
#include "vtkKWApplication.h"
#include "vtkKWWindow.h"
#include "vtkKWIcon.h"
#include "vtkMath.h"

#include <vtksys/stl/string>
#include "vtkKWWidgetsTourExample.h"

class vtkKWLabelItem : public KWWidgetsTourItem
{
public:
  virtual int GetType();
  virtual void Create(vtkKWWidget *parent, vtkKWWindow *);
};

void vtkKWLabelItem::Create(vtkKWWidget *parent, vtkKWWindow *)
{
  vtkKWApplication *app = parent->GetApplication();
  int id;

  // -----------------------------------------------------------------------

  // Create a label

  vtkKWLabel *label1 = vtkKWLabel::New();
  label1->SetParent(parent);
  label1->Create();
  label1->SetText("A label");

  app->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
    label1->GetWidgetName());

  // -----------------------------------------------------------------------

  // Create another label, right justify it

  vtkKWLabel *label2 = vtkKWLabel::New();
  label2->SetParent(parent);
  label2->Create();
  label2->SetText("Another label");
  label2->SetJustificationToRight();
  label2->SetWidth(30);
  label2->SetBackgroundColor(0.5, 0.5, 0.95);
  label2->SetBorderWidth(2);
  label2->SetReliefToGroove();
  label2->SetBalloonHelpString(
    "Another label, explicit width, right-justified");

  app->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6", 
    label2->GetWidgetName());

  // -----------------------------------------------------------------------

  // Create another label, with an icon

  vtkKWLabel *label2b = vtkKWLabel::New();
  label2b->SetParent(parent);
  label2b->Create();
  label2b->SetText("A label with an icon");
  label2b->SetImageToPredefinedIcon(vtkKWIcon::IconInfoMini);
  label2b->SetCompoundModeToLeft();
  label2b->SetPadX(2);
  label2b->SetBalloonHelpString(
    "This is a label with one of the predefined icons in front of it.");

  app->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6", 
    label2b->GetWidgetName());

  // -----------------------------------------------------------------------

  // Create another label, with a label this time (!)

  vtkKWLabelWithLabel *label3 = vtkKWLabelWithLabel::New();
  label3->SetParent(parent);
  label3->Create();
  label3->GetLabel()->SetText("Name:");
  label3->GetLabel()->SetBackgroundColor(0.7, 0.7, 0.7);
  label3->GetWidget()->SetText("Sebastien Barre");
  label3->SetBalloonHelpString(
    "This is a vtkKWLabelWithLabel, i.e. a label associated to a "
    "label that can be positioned around the label. This can be used for "
    "example to label a value without having to construct a single "
    "label out of two separate elements, one of them likely not to change.");

  app->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6", 
    label3->GetWidgetName());

  // -----------------------------------------------------------------------

  // Create a set of label
  // An easy way to create a bunch of related widgets without allocating
  // them one by one

  vtkKWLabelSet *label_set = vtkKWLabelSet::New();
  label_set->SetParent(parent);
  label_set->Create();
  label_set->SetBorderWidth(2);
  label_set->SetReliefToGroove();
  label_set->SetMaximumNumberOfWidgetsInPackingDirection(3);
  label_set->ExpandWidgetsOn();
  label_set->SetWidgetsPadX(1);
  label_set->SetWidgetsPadY(1);
  label_set->SetPadX(1);
  label_set->SetPadY(1);

  char buffer[50];
  for (id = 0; id < 9; id++)
    {
    sprintf(buffer, "Label %d", id);
    vtkKWLabel *label = label_set->AddWidget(id);
    label->SetText(buffer);
    label->SetBackgroundColor(vtkMath::HSVToRGB((double)id / 8.0, 0.3, 0.75));
    label->SetBalloonHelpString(
      "This label is part of a unique set (a vtkKWLabelSet), "
      "which provides an easy way to create a bunch of related widgets "
      "without allocating them one by one. The widgets can be layout as a "
      "NxM grid.");
    }

  label_set->GetWidget(0)->SetText("Firs Label");

  app->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6", 
    label_set->GetWidgetName());

  // -----------------------------------------------------------------------

  // Even trickier: create a set of labeled label !
  // An easy way to create a bunch of related widgets without allocating
  // them one by one

  vtkKWLabelWithLabelSet *label_set2 = vtkKWLabelWithLabelSet::New();
  label_set2->SetParent(parent);
  label_set2->Create();
  label_set2->SetBorderWidth(2);
  label_set2->SetReliefToGroove();
  label_set2->SetWidgetsPadX(1);
  label_set2->SetWidgetsPadY(1);
  label_set2->SetPadX(1);
  label_set2->SetPadY(1);

  for (id = 0; id < 3; id++)
    {
    vtkKWLabelWithLabel *label = label_set2->AddWidget(id);
    label->SetLabelWidth(15);
    label->GetLabel()->SetBackgroundColor(0.7, 0.7, 0.7);
    label->SetBalloonHelpString(
      "This labeled label (!) is part of a unique set "
      "(a vtkKWWithLabelLabelSet).");
    }

  label_set2->GetWidget(0)->SetLabelText("First Name:");
  label_set2->GetWidget(0)->GetWidget()->SetText("Sebastien");
  label_set2->GetWidget(1)->SetLabelText("Name:");
  label_set2->GetWidget(1)->GetWidget()->SetText("Barre");
  label_set2->GetWidget(2)->SetLabelText("Company:");
  label_set2->GetWidget(2)->GetWidget()->SetText("Kitware, Inc.");

  app->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6", 
    label_set2->GetWidgetName());

  label1->Delete();
  label2->Delete();
  label2b->Delete();
  label3->Delete();
  label_set->Delete();
  label_set2->Delete();
}

int vtkKWLabelItem::GetType()
{
  return KWWidgetsTourItem::TypeCore;
}

KWWidgetsTourItem* vtkKWLabelEntryPoint()
{
  return new vtkKWLabelItem();
}
