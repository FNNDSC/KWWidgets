#include "vtkKWTree.h"
#include "vtkKWTreeWithScrollbars.h"
#include "vtkKWApplication.h"
#include "vtkKWWindow.h"

#include "vtkKWWidgetsTourExample.h"

class vtkKWTreeItem : public KWWidgetsTourItem
{
public:
  virtual int GetType();
  virtual void Create(vtkKWWidget *parent, vtkKWWindow *);
};

void vtkKWTreeItem::Create(vtkKWWidget *parent, vtkKWWindow *)
{
  vtkKWApplication *app = parent->GetApplication();

  // -----------------------------------------------------------------------

  // Create a tree

  vtkKWTreeWithScrollbars *tree1 = vtkKWTreeWithScrollbars::New();
  tree1->SetParent(parent);
  tree1->Create();
  tree1->SetBalloonHelpString("A simple tree");
  tree1->SetBorderWidth(2);
  tree1->SetReliefToGroove();
  tree1->ResizeButtonsVisibilityOn();

  vtkKWTree *tree = tree1->GetWidget();
  tree->SelectionFillOn();
  tree->EnableReparentingOn();

  tree->AddNode(NULL, "inbox_node", "Inbox");

  tree->AddNode(NULL, "outbox_node", "Outbox");
  
  tree->AddNode(NULL, "kitware_node", "Kitware");
  tree->SetNodeFontWeightToBold("kitware_node");
  tree->SetNodeSelectableFlag("kitware_node", 0);
  tree->OpenTree("kitware_node");

  tree->AddNode("kitware_node", "berk_node", "Berk Geveci");
  
  tree->AddNode("kitware_node", "seb_node", "Sebastien Barre");
  
  tree->AddNode("kitware_node", "ken_node", "Ken Martin");
  
  app->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
    tree1->GetWidgetName());

  tree1->Delete();
}

int vtkKWTreeItem::GetType()
{
  return KWWidgetsTourItem::TypeCore;
}

KWWidgetsTourItem* vtkKWTreeEntryPoint()
{
  return new vtkKWTreeItem();
}
