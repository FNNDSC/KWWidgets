proc main {} {
    waitForObjectItem ":KWHelloWorldExample.vtkKWWindowBase0.#vtkKWWindowBase0#vtkKWMenu1" "File"
    invoke activateItem ":KWHelloWorldExample.vtkKWWindowBase0.#vtkKWWindowBase0#vtkKWMenu1" "File" 
    waitForObjectItem ":KWHelloWorldExample.vtkKWWindowBase0.#vtkKWWindowBase0#vtkKWMenu1.#vtkKWWindowBase0#vtkKWMenu1#vtkKWMenu1" "About KWHelloWorldExample 1.0"
    invoke activateItem ":KWHelloWorldExample.vtkKWWindowBase0.#vtkKWWindowBase0#vtkKWMenu1.#vtkKWWindowBase0#vtkKWMenu1#vtkKWMenu1" "About KWHelloWorldExample 1.0" 
    waitForObject ":KWHelloWorldExample.vtkKWMessageDialog1.vtkKWFrame0.vtkKWFrame4.vtkKWFrame0.vtkKWPushButton0"
    invoke clickButton ":KWHelloWorldExample.vtkKWMessageDialog1.vtkKWFrame0.vtkKWFrame4.vtkKWFrame0.vtkKWPushButton0" 
    snooze 1.8
    invoke closeWindow ":KWHelloWorldExample.vtkKWMessageDialog1" 
}