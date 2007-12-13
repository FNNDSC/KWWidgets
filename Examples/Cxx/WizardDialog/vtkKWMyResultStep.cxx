#include "vtkKWMyResultStep.h"

#include "vtkObjectFactory.h"

#include "vtkKWApplication.h"
#include "vtkKWWizardStep.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWLabel.h"

#include "vtkKWMyWizardDialog.h"
#include "vtkKWMyOperatorStep.h"
#include "vtkKWMyOperand1Step.h"
#include "vtkKWMyOperand2Step.h"

#include <vtksys/ios/sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWMyResultStep );
vtkCxxRevisionMacro(vtkKWMyResultStep, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkKWMyResultStep::vtkKWMyResultStep()
{
  this->SetName("Result");
  this->SetDescription("Here is the result of the operation");
  this->WizardDialog = NULL;
  this->ResultLabel = NULL;
}

//----------------------------------------------------------------------------
vtkKWMyResultStep::~vtkKWMyResultStep()
{
  if (this->ResultLabel)
    {
    this->ResultLabel->Delete();
    }
  this->SetWizardDialog(NULL);
}

//----------------------------------------------------------------------------
void vtkKWMyResultStep::SetWizardDialog(vtkKWMyWizardDialog *arg)
{
  this->WizardDialog = arg;
}

//----------------------------------------------------------------------------
void vtkKWMyResultStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizard_widget = 
    this->GetWizardDialog()->GetWizardWidget();

  if (!this->ResultLabel)
    {
    this->ResultLabel = vtkKWLabel::New();
    }
  if (!this->ResultLabel->IsCreated())
    {
    this->ResultLabel->SetParent(wizard_widget->GetClientArea());
    this->ResultLabel->Create();
    }
  
  this->Script("pack %s -side top -expand y -fill none -anchor center", 
               this->ResultLabel->GetWidgetName());
  
  double operand1, operand2, result = 0.0;

  vtksys_ios::ostringstream str;

  switch (this->GetWizardDialog()->GetOperatorStep()->GetSelectedOperator())
    {
    case vtkKWMyOperatorStep::OperatorAddition:
      operand1 = 
        this->GetWizardDialog()->GetOperand1Step()->GetOperand1Value();
      operand2 = 
        this->GetWizardDialog()->GetOperand2Step()->GetOperand2Value();
      str << operand1 << " + " << operand2;
      result = operand1 + operand2;
      break;

    case vtkKWMyOperatorStep::OperatorDivision:
      operand1 = 
        this->GetWizardDialog()->GetOperand1Step()->GetOperand1Value();
      operand2 = 
        this->GetWizardDialog()->GetOperand2Step()->GetOperand2Value();
      str << operand1 << " / " << operand2;
      result = operand1 / operand2;
      break;

    case vtkKWMyOperatorStep::OperatorSquareRoot:
      operand1 = 
        this->GetWizardDialog()->GetOperand1Step()->GetOperand1Value();
      str << "sqrt(" << operand1 << ")";
      result = sqrt(operand1);
      break;
    }

  str << " = " << result;

  this->ResultLabel->SetText(str.str().c_str());
}

//----------------------------------------------------------------------------
void vtkKWMyResultStep::HideUserInterface()
{
  this->Superclass::HideUserInterface();
  this->GetWizardDialog()->GetWizardWidget()->ClearPage();
}

//----------------------------------------------------------------------------
int vtkKWMyResultStep::CanGoToSelf()
{
  // We can only go directly to the finish/result step if all the operands
  // needed for the current operator are valid.

  switch (this->GetWizardDialog()->GetOperatorStep()->GetSelectedOperator())
    {
    case vtkKWMyOperatorStep::OperatorAddition:
    case vtkKWMyOperatorStep::OperatorDivision:
      return 
        (this->GetWizardDialog()->GetOperand1Step()->IsOperand1Valid() == 
         vtkKWMyOperand1Step::Operand1IsValid &&
         this->GetWizardDialog()->GetOperand2Step()->IsOperand2Valid() ==
         vtkKWMyOperand2Step::Operand2IsValid);

    case vtkKWMyOperatorStep::OperatorSquareRoot:
      return 
        (this->GetWizardDialog()->GetOperand1Step()->IsOperand1Valid() == 
         vtkKWMyOperand1Step::Operand1IsValid);
    }

  return 0;
}
