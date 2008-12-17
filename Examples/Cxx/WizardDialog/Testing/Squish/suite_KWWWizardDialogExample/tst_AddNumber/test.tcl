proc main {} {
    waitForObject ":KWWizardDialogExample.vtkKWMyWizardDialog0.vtkKWWizardWidget0.vtkKWFrame4.vtkKWPushButton1"
    invoke clickButton ":KWWizardDialogExample.vtkKWMyWizardDialog0.vtkKWWizardWidget0.vtkKWFrame4.vtkKWPushButton1" 
    waitForObject ":KWWizardDialogExample.vtkKWMyWizardDialog0.vtkKWWizardWidget0.vtkKWFrame2.vtkKWFrame1.vtkKWEntry1"
    invoke mouseClick ":KWWizardDialogExample.vtkKWMyWizardDialog0.vtkKWWizardWidget0.vtkKWFrame2.vtkKWFrame1.vtkKWEntry1" 26 7 272 1 
    waitForObject ":KWWizardDialogExample.vtkKWMyWizardDialog0.vtkKWWizardWidget0.vtkKWFrame2.vtkKWFrame1.vtkKWEntry1"
    invoke type ":KWWizardDialogExample.vtkKWMyWizardDialog0.vtkKWWizardWidget0.vtkKWFrame2.vtkKWFrame1.vtkKWEntry1" "5" 
    waitForObject ":KWWizardDialogExample.vtkKWMyWizardDialog0.vtkKWWizardWidget0.vtkKWFrame4.vtkKWPushButton1"
    invoke clickButton ":KWWizardDialogExample.vtkKWMyWizardDialog0.vtkKWWizardWidget0.vtkKWFrame4.vtkKWPushButton1" 
    waitForObject ":KWWizardDialogExample.vtkKWMyWizardDialog0.vtkKWWizardWidget0.vtkKWFrame2.vtkKWFrame1.vtkKWSpinBox2"
    invoke mouseClick ":KWWizardDialogExample.vtkKWMyWizardDialog0.vtkKWWizardWidget0.vtkKWFrame2.vtkKWFrame1.vtkKWSpinBox2" 172 7 272 1 
    waitForObject ":KWWizardDialogExample.vtkKWMyWizardDialog0.vtkKWWizardWidget0.vtkKWFrame2.vtkKWFrame1.vtkKWSpinBox2"
    invoke mouseClick ":KWWizardDialogExample.vtkKWMyWizardDialog0.vtkKWWizardWidget0.vtkKWFrame2.vtkKWFrame1.vtkKWSpinBox2" 172 7 272 1 
    waitForObject ":KWWizardDialogExample.vtkKWMyWizardDialog0.vtkKWWizardWidget0.vtkKWFrame4.vtkKWPushButton1"
    invoke clickButton ":KWWizardDialogExample.vtkKWMyWizardDialog0.vtkKWWizardWidget0.vtkKWFrame4.vtkKWPushButton1" 
    waitForObject ":KWWizardDialogExample.vtkKWMyWizardDialog0.vtkKWWizardWidget0.vtkKWFrame4.vtkKWPushButton5"
    invoke clickButton ":KWWizardDialogExample.vtkKWMyWizardDialog0.vtkKWWizardWidget0.vtkKWFrame4.vtkKWPushButton5" 
}
