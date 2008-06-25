from kwwidgets import vtkKWTree
from kwwidgets import vtkKWApplication
from kwwidgets import vtkKWWindow



def vtkKWTreeEntryPoint(parent, win):

    app = parent.GetApplication()
    
    # -----------------------------------------------------------------------
    
    # Create a tree
    
    tree1 = vtkKWTreeWithScrollbars()
    tree1.SetParent(parent)
    tree1.Create()
    tree1.SetBalloonHelpString("A simple tree")
    tree1.SetBorderWidth(2)
    tree1.SetReliefToGroove()
    tree1.ResizeButtonsVisibilityOn()

    tree = tree1.GetWidget()
    tree.SelectionFillOn()
    tree.EnableReparentingOn()
    
    tree.AddNode(None, "inbox_node", "Inbox")
    
    tree.AddNode(None, "outbox_node", "Outbox")
    
    tree.AddNode(None, "kitware_node", "Kitware")
    tree.SetNodeFontWeightToBold("kitware_node")
    tree.SetNodeSelectableFlag("kitware_node", 0)
    tree.OpenTree("kitware_node")
    
    tree.AddNode("kitware_node", "berk_node", "Berk Geveci")
    
    tree.AddNode("kitware_node", "seb_node", "Sebastien Barre")
    
    tree.AddNode("kitware_node", "ken_node", "Ken Martin")
    
    app.Script(
        "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
        tree1.GetWidgetName())
    
    
    
    return "TypeCore"
