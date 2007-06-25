#include "vtkKWMyWidget.h"

#include "vtkObjectFactory.h"
#include "vtkKWLabel.h"
#include "vtkKWCheckButton.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWMyWidget );
vtkCxxRevisionMacro(vtkKWMyWidget, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkKWMyWidget::vtkKWMyWidget()
{
  this->Label = NULL;
  this->CheckButton = NULL;
}

//----------------------------------------------------------------------------
vtkKWMyWidget::~vtkKWMyWidget()
{
  if (this->Label)
    {
    this->Label->Delete();
    this->Label = NULL;
    }

  if (this->CheckButton)
    {
    this->CheckButton->Delete();
    this->CheckButton = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkKWMyWidget::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the composite widget container

  this->Superclass::CreateWidget();

  // Create our internal label widget

  this->Label = vtkKWLabel::New();
  this->Label->SetParent(this);
  this->Label->Create();

  // Create our internal checkbutton widget

  this->CheckButton = vtkKWCheckButton::New();
  this->CheckButton->SetParent(this);
  this->CheckButton->Create();

  // Pack both our internal widgets next to each other

  this->Script("pack %s %s -side left", 
               this->CheckButton->GetWidgetName(),
               this->Label->GetWidgetName());
}
