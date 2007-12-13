#ifndef __vtkKWMyOperand1Step_h
#define __vtkKWMyOperand1Step_h

#include "vtkKWWizardStep.h"

class vtkKWMyWizardDialog;
class vtkKWEntry;
class vtkKWStateMachineInput;

class vtkKWMyOperand1Step : public vtkKWWizardStep
{
public:
  static vtkKWMyOperand1Step* New();
  vtkTypeRevisionMacro(vtkKWMyOperand1Step,vtkKWWizardStep);

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
    Operand1IsValid = 0,
    Operand1IsEmpty,
    Operand1IsNegative
  };
  //ETX
  virtual int IsOperand1Valid();

  // Description:
  // Get the operand value
  virtual double GetOperand1Value();

  // Description:
  // Set/Get the wizard widget this step should install its UI in.
  vtkGetObjectMacro(WizardDialog, vtkKWMyWizardDialog);
  virtual void SetWizardDialog(vtkKWMyWizardDialog*);
 
  // Description:
  // Get the input to that can be used to branch when the operator
  // requires only one operand.
  vtkGetObjectMacro(Operand1ValidationSucceededForOneOperandInput, vtkKWStateMachineInput);

  // Description:
  // Callbacks
  virtual int Operand1EntryChangedCallback(const char *value);

protected:
  vtkKWMyOperand1Step();
  ~vtkKWMyOperand1Step();

  vtkKWStateMachineInput *Operand1ValidationSucceededForOneOperandInput;
  vtkKWEntry *Operand1Entry;
  vtkKWMyWizardDialog *WizardDialog;

private:
  vtkKWMyOperand1Step(const vtkKWMyOperand1Step&);   // Not implemented.
  void operator=(const vtkKWMyOperand1Step&);  // Not implemented.
};

#endif
