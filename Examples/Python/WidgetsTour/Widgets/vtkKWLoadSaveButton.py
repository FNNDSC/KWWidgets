from kwwidgets import vtkKWLoadSaveButton
from kwwidgets import vtkKWLoadSaveDialog
from kwwidgets import vtkKWApplication
from kwwidgets import vtkKWWindow



def vtkKWLoadSaveButtonEntryPoint(parent, win):

    app = parent.GetApplication()
    
    # -----------------------------------------------------------------------
    
    # Create a load button to pick a file
    
    load_button1 = vtkKWLoadSaveButton()
    load_button1.SetParent(parent)
    load_button1.Create()
    load_button1.SetText("Click to Pick a File")
    load_button1.GetLoadSaveDialog().SaveDialogOff() # load mode
    
    app.Script(
        "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
        load_button1.GetWidgetName())
    
    # -----------------------------------------------------------------------
    
    # Create a load button to pick a directory
    
    load_button2 = vtkKWLoadSaveButton()
    load_button2.SetParent(parent)
    load_button2.Create()
    load_button2.SetText("Click to Pick a Directory")
    load_button2.GetLoadSaveDialog().ChooseDirectoryOn()
    
    app.Script(
        "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
        load_button2.GetWidgetName())
    
    
    return "TypeComposite"
