from kwwidgets import vtkKWMessageDialog
from kwwidgets import vtkKWPushButton
from kwwidgets import vtkKWApplication
from kwwidgets import vtkKWWindow



def vtkKWMessageDialogEntryPoint(parent, win):

    app = parent.GetApplication()
    
    # -----------------------------------------------------------------------
    
    # Create a message box
    
    msg_dlg1 = vtkKWMessageDialog()
    msg_dlg1.SetParent(parent)
    msg_dlg1.SetMasterWindow(win)
    msg_dlg1.SetStyleToOkCancel()
    msg_dlg1.Create()
    msg_dlg1.SetTitle("Your attention please!")
    msg_dlg1.SetText(
        "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Nunc felis. "
        "Nulla gravida. Aliquam erat volutpat. Mauris accumsan quam non sem. "
        "Sed commodo, magna quis bibendum lacinia, elit turpis iaculis augue, "
        "eget hendrerit elit dui vel elit.")
    
    # -----------------------------------------------------------------------
    
    # Create a push button to invoke the message box
    
    msg_dlg_button1 = vtkKWPushButton()
    msg_dlg_button1.SetParent(parent)
    msg_dlg_button1.Create()
    msg_dlg_button1.SetText("Press to invoke message box")
    msg_dlg_button1.SetCommand(msg_dlg1, "Invoke")
    
    app.Script(
        "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
        msg_dlg_button1.GetWidgetName())
    
    
    
    return "TypeComposite"
