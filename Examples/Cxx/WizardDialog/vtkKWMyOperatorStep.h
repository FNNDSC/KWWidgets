#ifndef __vtkKWMyOperatorStep_h
#define __vtkKWMyOperatorStep_h

#include "vtkKWWizardStep.h"

class vtkKWMyWizardDialog;
class vtkKWRadioButtonSet;
class vtkKWStateMachineInput;

class vtkKWMyOperatorStep : public vtkKWWizardStep
{
public:
  static vtkKWMyOperatorStep* New();
  vtkTypeRevisionMacro(vtkKWMyOperatorStep,vtkKWWizardStep);

  // Description:
  // Show/hide the UI, validate the step.
  virtual void ShowUserInterface();
  virtual void HideUserInterface();
  virtual void Validate();

  // Description:
  // Get selected operator
  //BTX
  enum 
  {
    OperatorAddition = 0,
    OperatorDivision,
    OperatorSquareRoot,
    OperatorUnknown
  };
  //ETX
  virtual int GetSelectedOperator();

  // Description:
  // Set/Get the wizard widget this step should install its UI in.
  vtkGetObjectMacro(WizardDialog, vtkKWMyWizardDialog);
  virtual void SetWizardDialog(vtkKWMyWizardDialog*);

protected:
  vtkKWMyOperatorStep();
  ~vtkKWMyOperatorStep();

  vtkKWRadioButtonSet *OperatorRadioButtonSet;
  vtkKWMyWizardDialog *WizardDialog;

private:
  vtkKWMyOperatorStep(const vtkKWMyOperatorStep&);   // Not implemented.
  void operator=(const vtkKWMyOperatorStep&);  // Not implemented.
};

#endif
