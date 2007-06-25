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
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the internal widgets.
  vtkGetObjectMacro(Label, vtkKWLabel);
  vtkGetObjectMacro(CheckButton, vtkKWCheckButton);

  // Description:
  // Set/Get the background color of the label when the checkbutton is selected
  vtkGetVector3Macro(SelectedBackgroundColor, double);
  virtual void SetSelectedBackgroundColor(double r, double g, double b);
  virtual void SetSelectedBackgroundColor(double rgb[3])
    { this->SetSelectedBackgroundColor(rgb[0], rgb[1], rgb[2]); }; 

  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object is updated
  // and propagated to its internal parts/subwidgets.
  virtual void UpdateEnableState();

protected:
  vtkKWMyWidget();
  ~vtkKWMyWidget();

  vtkKWLabel *Label;
  vtkKWCheckButton *CheckButton;

  double SelectedBackgroundColor[3];

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
