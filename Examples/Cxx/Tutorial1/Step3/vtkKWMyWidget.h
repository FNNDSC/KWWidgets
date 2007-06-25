#ifndef __vtkKWMyWidget_h
#define __vtkKWMyWidget_h

#include "vtkKWCompositeWidget.h"

class vtkKWLabel;
class vtkKWCheckButton;

class vtkKWMyWidget : public vtkKWCompositeWidget
{
public:
  static vtkKWMyWidget* New();
  vtkTypeRevisionMacro(vtkKWMyWidget,vtkKWCompositeWidget);

  // Description:
  // Get the internal widgets.
  vtkGetObjectMacro(Label, vtkKWLabel);
  vtkGetObjectMacro(CheckButton, vtkKWCheckButton);

protected:
  vtkKWMyWidget();
  ~vtkKWMyWidget();

  vtkKWLabel *Label;
  vtkKWCheckButton *CheckButton;

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Description:
  // Update the label's aspect according to the checkbutton's state.
  virtual void UpdateLabel();

  // Description:
  // Processes the events that are passed through CallbackCommand (or others).
  virtual void ProcessCallbackCommandEvents(
    vtkObject *caller, unsigned long event, void *calldata);

private:
  vtkKWMyWidget(const vtkKWMyWidget&);   // Not implemented.
  void operator=(const vtkKWMyWidget&);  // Not implemented.
};

#endif
