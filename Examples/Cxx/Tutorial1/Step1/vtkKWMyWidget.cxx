#include "vtkKWMyWidget.h"

#include "vtkObjectFactory.h"
#include "vtkKWLabel.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWMyWidget );
vtkCxxRevisionMacro(vtkKWMyWidget, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkKWMyWidget::vtkKWMyWidget()
{
  this->Label = NULL;
}

//----------------------------------------------------------------------------
vtkKWMyWidget::~vtkKWMyWidget()
{
  if (this->Label)
    {
    this->Label->Delete();
    this->Label = NULL;
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

  // Pack both our internal widget

  this->Script("pack %s -side left", 
               this->Label->GetWidgetName());
}
