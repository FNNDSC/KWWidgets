#ifndef __vtkKWMyResultStep_h
#define __vtkKWMyResultStep_h

#include "vtkKWWizardStep.h"

class vtkKWMyWizardDialog;
class vtkKWLabel;

class vtkKWMyResultStep : public vtkKWWizardStep
{
public:
  static vtkKWMyResultStep* New();
  vtkTypeRevisionMacro(vtkKWMyResultStep,vtkKWWizardStep);

  // Description:
  // Show/hide the UI, check if this finish step is reachable.
  virtual void ShowUserInterface();
  virtual void HideUserInterface();
  virtual int CanGoToSelf();

  // Description:
  // Set/Get the wizard widget this step should install its UI in.
  vtkGetObjectMacro(WizardDialog, vtkKWMyWizardDialog);
  virtual void SetWizardDialog(vtkKWMyWizardDialog*);

protected:
  vtkKWMyResultStep();
  ~vtkKWMyResultStep();

  vtkKWLabel *ResultLabel;
  vtkKWMyWizardDialog *WizardDialog;

private:
  vtkKWMyResultStep(const vtkKWMyResultStep&);   // Not implemented.
  void operator=(const vtkKWMyResultStep&);  // Not implemented.
};

#endif
