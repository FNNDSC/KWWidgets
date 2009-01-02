proc main {} {
    waitForObject ":KWCallbacksExample.vtkKWWindow0.vtkKWFrame3.vtkKWSplitFrame0.vtkKWFrame2.vtkKWSplitFrame0.vtkKWFrame2.vtkKWNotebook0.vtkKWFrame1.vtkKWFrame0.vtkKWMyWidget0.vtkKWScale0"
    invoke scrollTo ":KWCallbacksExample.vtkKWWindow0.vtkKWFrame3.vtkKWSplitFrame0.vtkKWFrame2.vtkKWSplitFrame0.vtkKWFrame2.vtkKWNotebook0.vtkKWFrame1.vtkKWFrame0.vtkKWMyWidget0.vtkKWScale0" 32000 
    
    snooze 1    
    test vp "VP1"
    
    waitForObject ":KWCallbacksExample.vtkKWWindow0.vtkKWFrame3.vtkKWSplitFrame0.vtkKWFrame2.vtkKWSplitFrame0.vtkKWFrame2.vtkKWNotebook0.vtkKWFrame1.vtkKWFrame0.vtkKWMyWidget0.vtkKWScale0"
    invoke scrollTo ":KWCallbacksExample.vtkKWWindow0.vtkKWFrame3.vtkKWSplitFrame0.vtkKWFrame2.vtkKWSplitFrame0.vtkKWFrame2.vtkKWNotebook0.vtkKWFrame1.vtkKWFrame0.vtkKWMyWidget0.vtkKWScale0" 71000 
    snooze 1.5
}
