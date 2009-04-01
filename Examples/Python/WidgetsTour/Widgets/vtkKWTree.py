from kwwidgets import vtkKWApplication
from kwwidgets import vtkKWPushButton
from kwwidgets import vtkKWTree
from kwwidgets import vtkKWTreeWithScrollbars
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
    
    # Inbox node. Use a predefined icon as image, make room for it (pad)

    tree.AddNode(None, "inbox_node", "Inbox")
    tree.SetNodeImageToPredefinedIcon("inbox_node", 1903)
    tree.SetNodePadX("inbox_node", 18)
    
    # Outbox node. Use a predefined icon as image, make room for it (pad)

    tree.AddNode(None, "outbox_node", "Outbox")
    tree.SetNodeImageToPredefinedIcon("outbox_node", 1904)
    tree.SetNodePadX("outbox_node", 18)
    
    # Trash node. Create a pushbutton and use it as an extra widget to the
    # left of the node text. Make room for it (pad), make sure it blends
    # with the tree by matching the background color. Use a predefined icon
    # for the pushbutton image, and set a simple callback that will change
    # the image to a different icon (just to examplify).

    tree.AddNode(None, "trash_node", "Trash")
    empty_trash_button = vtkKWPushButton()
    empty_trash_button.SetParent(tree)
    empty_trash_button.Create()
    empty_trash_button.SetBorderWidth(0)
    empty_trash_button.SetBackgroundColor(tree.GetBackgroundColor())
    empty_trash_button.SetActiveBackgroundColor(tree.GetBackgroundColor())
    empty_trash_button.SetImageToPredefinedIcon(1902)
    empty_trash_button.SetCommand(
        empty_trash_button, "SetImageToPredefinedIcon 1901")
    tree.SetNodeWindow("trash_node", empty_trash_button)
    tree.SetNodePadX("trash_node", 18)
    
    # Company nodes

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
