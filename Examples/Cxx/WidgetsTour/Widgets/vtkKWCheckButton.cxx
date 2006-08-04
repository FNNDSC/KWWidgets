#include "vtkKWCheckButton.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWCheckButtonSet.h"
#include "vtkKWApplication.h"
#include "vtkKWWindow.h"
#include "vtkKWIcon.h"

#include <vtksys/stl/string>
#include "vtkKWWidgetsTourExample.h"

class vtkKWCheckButtonItem : public KWWidgetsTourItem
{
public:
  virtual int GetType();
  virtual void Create(vtkKWWidget *parent, vtkKWWindow *);
};

void vtkKWCheckButtonItem::Create(vtkKWWidget *parent, vtkKWWindow *)
{
  vtkKWApplication *app = parent->GetApplication();

  // -----------------------------------------------------------------------

  // Create a checkbutton

  vtkKWCheckButton *cb1 = vtkKWCheckButton::New();
  cb1->SetParent(parent);
  cb1->Create();
  cb1->SetText("A checkbutton");

  app->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
    cb1->GetWidgetName());

  // -----------------------------------------------------------------------

  // Create another checkbutton, but use an icon this time

  vtkKWCheckButton *cb2 = vtkKWCheckButton::New();
  cb2->SetParent(parent);
  cb2->Create();
  cb2->SetImageToPredefinedIcon(vtkKWIcon::IconLock);
  cb2->IndicatorVisibilityOff();
  cb2->SetBalloonHelpString("This time, use one of the predefined icon");

  app->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6", 
    cb2->GetWidgetName());

  // -----------------------------------------------------------------------

  // Create another checkbutton, but use both text and an icon

  vtkKWCheckButton *cb2b = vtkKWCheckButton::New();
  cb2b->SetParent(parent);
  cb2b->Create();
  cb2b->SetText("A checkbutton");
  cb2b->SetImageToPredefinedIcon(vtkKWIcon::IconLock);
  cb2b->SetCompoundModeToLeft();
  cb2b->IndicatorVisibilityOff();
  cb2b->SetBalloonHelpString(
    "This time, use both a text and one of the predefined icon");

  app->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6", 
    cb2b->GetWidgetName());

  // -----------------------------------------------------------------------

  // Create another checkbutton, with a label this time

  vtkKWCheckButtonWithLabel *cb3 = vtkKWCheckButtonWithLabel::New();
  cb3->SetParent(parent);
  cb3->Create();
  cb3->SetLabelText("Another checkbutton, with a label in front");
  cb3->SetBalloonHelpString(
    "This is a vtkKWCheckButtonWithLabel, i.e. a checkbutton associated to a "
    "label that can be positioned around the checkbutton.");

  app->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6", 
    cb3->GetWidgetName());

  // -----------------------------------------------------------------------

  // Create a set of checkbutton
  // An easy way to create a bunch of related widgets without allocating
  // them one by one

  vtkKWCheckButtonSet *cbs = vtkKWCheckButtonSet::New();
  cbs->SetParent(parent);
  cbs->Create();
  cbs->SetBorderWidth(2);
  cbs->SetReliefToGroove();
  cbs->SetMaximumNumberOfWidgetsInPackingDirection(2);

  for (int id = 0; id < 4; id++)
    {
    vtkKWCheckButton *cb = cbs->AddWidget(id);
    cb->SetBalloonHelpString(
      "This checkbutton is part of a unique set (a vtkKWCheckButtonSet), "
      "which provides an easy way to create a bunch of related widgets "
      "without allocating them one by one. The widgets can be layout as a "
      "NxM grid.");
    }

  // Let's be creative. The first two share the same variable name
  
  cbs->GetWidget(0)->SetText("Checkbutton 0 has the same variable name as 1");
  cbs->GetWidget(1)->SetText("Checkbutton 1 has the same variable name as 0");
  cbs->GetWidget(1)->SetVariableName(cbs->GetWidget(0)->GetVariableName());

  // The last two buttons trigger each other's states

  cbs->GetWidget(2)->SetSelectedState(1);
  cbs->GetWidget(2)->SetText("Checkbutton 2 also toggles 3");
  cbs->GetWidget(2)->SetCommand(cbs->GetWidget(3), "SetSelectedState");

  cbs->GetWidget(3)->SetText("Checkbutton 3 also toggles 2");
  cbs->GetWidget(3)->SetCommand(cbs->GetWidget(2), "SetSelectedState");
  
  app->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6", 
    cbs->GetWidgetName());

  // -----------------------------------------------------------------------

  // TODO: use vtkKWCheckButtonSetWithLabel

  cb1->Delete();
  cb2->Delete();
  cb2b->Delete();
  cb3->Delete();
  cbs->Delete();
}

int vtkKWCheckButtonItem::GetType()
{
  return KWWidgetsTourItem::TypeCore;
}

KWWidgetsTourItem* vtkKWCheckButtonEntryPoint()
{
  return new vtkKWCheckButtonItem();
}
