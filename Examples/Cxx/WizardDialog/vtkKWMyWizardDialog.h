#ifndef __vtkKWMyWizardDialog_h
#define __vtkKWMyWizardDialog_h

#include "vtkKWWizardDialog.h"

class vtkKWMyOperatorStep;
class vtkKWMyOperand1Step;
class vtkKWMyOperand2Step;
class vtkKWMyResultStep;

class vtkKWMyWizardDialog : public vtkKWWizardDialog
{
public:
  static vtkKWMyWizardDialog* New();
  vtkTypeRevisionMacro(vtkKWMyWizardDialog,vtkKWWizardDialog);

  // Description:
  // Access to the steps.
  vtkGetObjectMacro(OperatorStep, vtkKWMyOperatorStep);
  vtkGetObjectMacro(Operand1Step, vtkKWMyOperand1Step);
  vtkGetObjectMacro(Operand2Step, vtkKWMyOperand2Step);
  vtkGetObjectMacro(ResultStep,   vtkKWMyResultStep);

protected:
  vtkKWMyWizardDialog();
  ~vtkKWMyWizardDialog() {};

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Description:
  // Steps
  vtkKWMyOperatorStep *OperatorStep;
  vtkKWMyOperand1Step *Operand1Step;
  vtkKWMyOperand2Step *Operand2Step;
  vtkKWMyResultStep   *ResultStep;

private:
  vtkKWMyWizardDialog(const vtkKWMyWizardDialog&);   // Not implemented.
  void operator=(const vtkKWMyWizardDialog&);  // Not implemented.
};

#endif
