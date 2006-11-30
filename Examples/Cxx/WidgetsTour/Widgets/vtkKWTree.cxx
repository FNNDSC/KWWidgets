#include "vtkKWTree.h"
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

  vtkKWTree *tree1 = vtkKWTree::New();
  tree1->SetParent(parent);
  tree1->Create();
  tree1->SelectionFillOn();
  tree1->SetBalloonHelpString("A simple tree");
  tree1->SetBorderWidth(2);
  tree1->SetReliefToGroove();
  tree1->EnableReparentingOn();

  tree1->AddNode(NULL, "inbox_node", "Inbox");

  tree1->AddNode(NULL, "outbox_node", "Outbox");
  
  tree1->AddNode(NULL, "kitware_node", "Kitware");
  tree1->SetNodeFontWeightToBold("kitware_node");
  tree1->SetNodeSelectableFlag("kitware_node", 0);
  tree1->OpenTree("kitware_node");

  tree1->AddNode("kitware_node", "berk_node", "Berk Geveci");
  
  tree1->AddNode("kitware_node", "seb_node", "Sebastien Barre");
  
  tree1->AddNode("kitware_node", "ken_node", "Ken Martin");
  
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
