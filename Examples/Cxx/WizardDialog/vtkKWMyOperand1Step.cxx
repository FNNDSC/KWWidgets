#include "vtkKWMyOperand1Step.h"

#include "vtkObjectFactory.h"

#include "vtkKWApplication.h"
#include "vtkKWWizardStep.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWEntry.h"
#include "vtkKWStateMachineInput.h"

#include "vtkKWMyWizardDialog.h"
#include "vtkKWMyOperatorStep.h"

#include <vtksys/ios/sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWMyOperand1Step );
vtkCxxRevisionMacro(vtkKWMyOperand1Step, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkKWMyOperand1Step::vtkKWMyOperand1Step()
{
  this->SetName("Operand 1");
  this->SetDescription("Enter an operand");
  this->WizardDialog = NULL;
  this->Operand1Entry = NULL;

  // This input is pushed when the operand 1 has been validated in a context
  // where only one operand is needed.

  this->Operand1ValidationSucceededForOneOperandInput = 
    vtkKWStateMachineInput::New();
  this->Operand1ValidationSucceededForOneOperandInput->SetName("valid 1/1");
}

//----------------------------------------------------------------------------
vtkKWMyOperand1Step::~vtkKWMyOperand1Step()
{
  if (this->Operand1Entry)
    {
    this->Operand1Entry->Delete();
    }
  this->SetWizardDialog(NULL);
  this->Operand1ValidationSucceededForOneOperandInput->Delete();
}

//----------------------------------------------------------------------------
void vtkKWMyOperand1Step::SetWizardDialog(vtkKWMyWizardDialog *arg)
{
  this->WizardDialog = arg;
}

//----------------------------------------------------------------------------
void vtkKWMyOperand1Step::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizard_widget = 
    this->GetWizardDialog()->GetWizardWidget();

  if (!this->Operand1Entry)
    {
    this->Operand1Entry = vtkKWEntry::New();
    }
  if (!this->Operand1Entry->IsCreated())
    {
    this->Operand1Entry->SetParent(wizard_widget->GetClientArea());
    this->Operand1Entry->Create();
    this->Operand1Entry->SetRestrictValueToDouble();
    this->Operand1Entry->SetCommandTriggerToAnyChange();
    this->Operand1Entry->SetCommand(this, "Operand1EntryChangedCallback");
    }
  
  this->Script("pack %s -side top -expand y -fill none -anchor center", 
               this->Operand1Entry->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkKWMyOperand1Step::HideUserInterface()
{
  this->Superclass::HideUserInterface();
  this->GetWizardDialog()->GetWizardWidget()->ClearPage();
}

//----------------------------------------------------------------------------
int vtkKWMyOperand1Step::Operand1EntryChangedCallback(const char *)
{
  this->GetWizardDialog()->GetWizardWidget()->Update();
  return 1;
}

//----------------------------------------------------------------------------
int vtkKWMyOperand1Step::IsOperand1Valid()
{
  // If the operand is empty, we are invalid!

  if (!this->Operand1Entry || 
      !this->Operand1Entry->GetValue() || !*this->Operand1Entry->GetValue())
    {
    return vtkKWMyOperand1Step::Operand1IsEmpty;
    }

  // If the operand is negative and we are using square root, we are invalid!

  if (this->Operand1Entry->GetValueAsDouble() < 0.0 &&
      this->GetWizardDialog()->GetOperatorStep()->GetSelectedOperator() == 
      vtkKWMyOperatorStep::OperatorSquareRoot)
    {
    return vtkKWMyOperand1Step::Operand1IsNegative;
    }

  return vtkKWMyOperand1Step::Operand1IsValid;
}

//----------------------------------------------------------------------------
void vtkKWMyOperand1Step::Validate()
{
  vtkKWWizardWidget *wizard_widget = 
    this->GetWizardDialog()->GetWizardWidget();

  vtkKWWizardWorkflow *wizard_workflow = wizard_widget->GetWizardWorkflow();

  // If the operand is invalid, display an error and push the input
  // that will bring us back to the same state. Otherwise move on
  // to the next step!

  int valid = this->IsOperand1Valid();
  if (valid == vtkKWMyOperand1Step::Operand1IsEmpty)
    {
    wizard_widget->SetErrorText("Empty operand!");
    wizard_workflow->PushInput(vtkKWWizardStep::GetValidationFailedInput());
    }
  else if (valid == vtkKWMyOperand1Step::Operand1IsNegative)
    {
    wizard_widget->SetErrorText("Negative operand!");
    wizard_workflow->PushInput(vtkKWWizardStep::GetValidationFailedInput());
    }
  else
    {
    // If the operator is square root, we need to move to a specific step
    // that will display the result given only one operand. 

    if (this->GetWizardDialog()->GetOperatorStep()->GetSelectedOperator() == 
        vtkKWMyOperatorStep::OperatorSquareRoot)
      {
      wizard_workflow->PushInput(
        this->GetOperand1ValidationSucceededForOneOperandInput());
      }
    else
      {
      wizard_workflow->PushInput(
        vtkKWWizardStep::GetValidationSucceededInput());
      }
    }

  wizard_workflow->ProcessInputs();
}

//----------------------------------------------------------------------------
double vtkKWMyOperand1Step::GetOperand1Value()
{
  return this->Operand1Entry ? this->Operand1Entry->GetValueAsDouble() : 0;
}
