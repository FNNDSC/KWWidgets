#ifndef __vtkKWMyOperand2Step_h
#define __vtkKWMyOperand2Step_h

#include "vtkKWWizardStep.h"

class vtkKWMyWizardDialog;
class vtkKWSpinBox;

class vtkKWMyOperand2Step : public vtkKWWizardStep
{
public:
  static vtkKWMyOperand2Step* New();
  vtkTypeRevisionMacro(vtkKWMyOperand2Step,vtkKWWizardStep);

  // Description:
  // Show/hide the UI, validate the step.
  virtual void ShowUserInterface();
  virtual void HideUserInterface();
  virtual void Validate();

  // Description:
  // Check if the operand is valid
  //BTX
  enum 
  {
    Operand2IsValid = 0,
    Operand2IsEmpty,
    Operand2IsZero
  };
  //ETX
  virtual int IsOperand2Valid();

  // Description:
  // Get the operand value
  virtual double GetOperand2Value();

  // Description:
  // Set/Get the wizard widget this step should install its UI in.
  vtkGetObjectMacro(WizardDialog, vtkKWMyWizardDialog);
  virtual void SetWizardDialog(vtkKWMyWizardDialog*);

  // Description:
  // Callbacks
  virtual int Operand2EntryChangedCallback(const char *value);

protected:
  vtkKWMyOperand2Step();
  ~vtkKWMyOperand2Step();

  vtkKWSpinBox *Operand2SpinBox;
  vtkKWMyWizardDialog *WizardDialog;

private:
  vtkKWMyOperand2Step(const vtkKWMyOperand2Step&);   // Not implemented.
  void operator=(const vtkKWMyOperand2Step&);  // Not implemented.
};

#endif
