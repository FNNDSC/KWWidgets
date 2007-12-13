#include "vtkKWMyOperand2Step.h"

#include "vtkObjectFactory.h"

#include "vtkKWApplication.h"
#include "vtkKWWizardStep.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWSpinBox.h"

#include "vtkKWMyWizardDialog.h"
#include "vtkKWMyOperatorStep.h"

#include <vtksys/ios/sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWMyOperand2Step );
vtkCxxRevisionMacro(vtkKWMyOperand2Step, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkKWMyOperand2Step::vtkKWMyOperand2Step()
{
  this->SetName("Operand 2");
  this->SetDescription("Enter a second operand");
  this->WizardDialog = NULL;
  this->Operand2SpinBox = NULL;
}

//----------------------------------------------------------------------------
vtkKWMyOperand2Step::~vtkKWMyOperand2Step()
{
  if (this->Operand2SpinBox)
    {
    this->Operand2SpinBox->Delete();
    }
  this->SetWizardDialog(NULL);
}

//----------------------------------------------------------------------------
void vtkKWMyOperand2Step::SetWizardDialog(vtkKWMyWizardDialog *arg)
{
  this->WizardDialog = arg;
}

//----------------------------------------------------------------------------
void vtkKWMyOperand2Step::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizard_widget = 
    this->GetWizardDialog()->GetWizardWidget();

  if (!this->Operand2SpinBox)
    {
    this->Operand2SpinBox = vtkKWSpinBox::New();
    }

  if (!this->Operand2SpinBox->IsCreated())
    {
    this->Operand2SpinBox->SetParent(wizard_widget->GetClientArea());
    this->Operand2SpinBox->Create();
    this->Operand2SpinBox->SetRange(-10000, 10000);
    this->Operand2SpinBox->SetRestrictValueToDouble();
    this->Operand2SpinBox->SetCommandTriggerToAnyChange();
    this->Operand2SpinBox->SetCommand(this, "Operand2EntryChangedCallback");
    }
  
  this->Script("pack %s -side top -expand y -fill none -anchor center", 
               this->Operand2SpinBox->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkKWMyOperand2Step::HideUserInterface()
{
  this->Superclass::HideUserInterface();
  this->GetWizardDialog()->GetWizardWidget()->ClearPage();
}

//----------------------------------------------------------------------------
int vtkKWMyOperand2Step::Operand2EntryChangedCallback(const char *)
{
  this->GetWizardDialog()->GetWizardWidget()->Update();
  return 1;
}

//----------------------------------------------------------------------------
int vtkKWMyOperand2Step::IsOperand2Valid()
{
  // If the operand is empty, we are invalid!

  if (!this->Operand2SpinBox)
    {
    return vtkKWMyOperand2Step::Operand2IsEmpty;
    }

  // If the operand is 0 and the operator is a division, we are 
  // invalid!

  if (this->Operand2SpinBox->GetValue() == 0.0 &&
      this->GetWizardDialog()->GetOperatorStep()->GetSelectedOperator() == 
      vtkKWMyOperatorStep::OperatorDivision)
    {
    return vtkKWMyOperand2Step::Operand2IsZero;
    }

  return vtkKWMyOperand2Step::Operand2IsValid;
}

//----------------------------------------------------------------------------
void vtkKWMyOperand2Step::Validate()
{
  vtkKWWizardWidget *wizard_widget = 
    this->GetWizardDialog()->GetWizardWidget();

  vtkKWWizardWorkflow *wizard_workflow = wizard_widget->GetWizardWorkflow();

  // If the operand is invalid, display an error and push the input
  // that will bring us back to the same state. Otherwise move on
  // to the next step!

  int valid = this->IsOperand2Valid();
  if (valid == vtkKWMyOperand2Step::Operand2IsEmpty)
    {
    wizard_widget->SetErrorText("Empty operand!");
    wizard_workflow->PushInput(vtkKWWizardStep::GetValidationFailedInput());
    }
  else if (valid == vtkKWMyOperand2Step::Operand2IsZero)
    {
    wizard_widget->SetErrorText("Can not divide by zero!");
    wizard_workflow->PushInput(vtkKWWizardStep::GetValidationFailedInput());
    }
  else
    {
    wizard_workflow->PushInput(vtkKWWizardStep::GetValidationSucceededInput());
    }
 
  wizard_workflow->ProcessInputs();
}

//----------------------------------------------------------------------------
double vtkKWMyOperand2Step::GetOperand2Value()
{
  return this->Operand2SpinBox ? this->Operand2SpinBox->GetValue() : 0;
}
