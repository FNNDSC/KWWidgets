#include "vtkKWMyOperatorStep.h"

#include "vtkObjectFactory.h"

#include "vtkKWApplication.h"
#include "vtkKWWizardStep.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWRadioButtonSet.h"
#include "vtkKWRadioButton.h"

#include "vtkKWMyWizardDialog.h"

#include <vtksys/ios/sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWMyOperatorStep );
vtkCxxRevisionMacro(vtkKWMyOperatorStep, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkKWMyOperatorStep::vtkKWMyOperatorStep()
{
  this->SetName("Operator");
  this->SetDescription("Select a mathematical operator");
  this->WizardDialog = NULL;
  this->OperatorRadioButtonSet = NULL;
}

//----------------------------------------------------------------------------
vtkKWMyOperatorStep::~vtkKWMyOperatorStep()
{
  if (this->OperatorRadioButtonSet)
    {
    this->OperatorRadioButtonSet->Delete();
    }
  this->SetWizardDialog(NULL);
}

//----------------------------------------------------------------------------
void vtkKWMyOperatorStep::SetWizardDialog(vtkKWMyWizardDialog *arg)
{
  this->WizardDialog = arg;
}

//----------------------------------------------------------------------------
void vtkKWMyOperatorStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizard_widget = 
    this->GetWizardDialog()->GetWizardWidget();

  // Create radio buttons for each of the mathematical operator we support

  if (!this->OperatorRadioButtonSet)
    {
    this->OperatorRadioButtonSet = vtkKWRadioButtonSet::New();
    }
  if (!this->OperatorRadioButtonSet->IsCreated())
    {
    this->OperatorRadioButtonSet->SetParent(wizard_widget->GetClientArea());
    this->OperatorRadioButtonSet->Create();

    vtkKWRadioButton *radiob;

    radiob = this->OperatorRadioButtonSet->AddWidget(
      vtkKWMyOperatorStep::OperatorAddition);
    radiob->SetText("Addition");
    radiob->SetCommand(wizard_widget, "Update");

    radiob = this->OperatorRadioButtonSet->AddWidget(
      vtkKWMyOperatorStep::OperatorDivision);
    radiob->SetText("Division");
    radiob->SetCommand(wizard_widget, "Update");

    radiob = this->OperatorRadioButtonSet->AddWidget(
      vtkKWMyOperatorStep::OperatorSquareRoot);
    radiob->SetText("Square Root");
    radiob->SetCommand(wizard_widget, "Update");

    this->OperatorRadioButtonSet->GetWidget(
      vtkKWMyOperatorStep::OperatorAddition)->Select();
    }
  
  this->Script("pack %s -side top -expand y -fill none -anchor center", 
               this->OperatorRadioButtonSet->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkKWMyOperatorStep::HideUserInterface()
{
  this->Superclass::HideUserInterface();
  this->GetWizardDialog()->GetWizardWidget()->ClearPage();
}

//----------------------------------------------------------------------------
int vtkKWMyOperatorStep::GetSelectedOperator()
{
  if (this->OperatorRadioButtonSet)
    {
    if (this->OperatorRadioButtonSet->GetWidget(
          vtkKWMyOperatorStep::OperatorAddition)->GetSelectedState())
      {
      return vtkKWMyOperatorStep::OperatorAddition;
      }
    if (this->OperatorRadioButtonSet->GetWidget(
          vtkKWMyOperatorStep::OperatorDivision)->GetSelectedState())
      {
      return vtkKWMyOperatorStep::OperatorDivision;
      }
    if (this->OperatorRadioButtonSet->GetWidget(
          vtkKWMyOperatorStep::OperatorSquareRoot)->GetSelectedState())
      {
      return vtkKWMyOperatorStep::OperatorSquareRoot;
      }
    }

  return vtkKWMyOperatorStep::OperatorUnknown;
}

//----------------------------------------------------------------------------
void vtkKWMyOperatorStep::Validate()
{
  vtkKWWizardWorkflow *wizard_workflow = 
    this->GetWizardDialog()->GetWizardWidget()->GetWizardWorkflow();

  // This step always validates

  wizard_workflow->PushInput(
    vtkKWWizardStep::GetValidationSucceededInput());

  wizard_workflow->ProcessInputs();
}
