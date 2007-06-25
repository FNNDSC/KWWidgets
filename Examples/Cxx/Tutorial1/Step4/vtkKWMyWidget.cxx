#include "vtkKWMyWidget.h"

#include "vtkObjectFactory.h"
#include "vtkKWLabel.h"
#include "vtkKWCheckButton.h"
#include "vtkKWTkUtilities.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWMyWidget );
vtkCxxRevisionMacro(vtkKWMyWidget, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkKWMyWidget::vtkKWMyWidget()
{
  this->Label = NULL;
  this->CheckButton = NULL;

  this->SelectedBackgroundColor[0] = 0.8;
  this->SelectedBackgroundColor[1] = 0.2;
  this->SelectedBackgroundColor[2] = 0.2;
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

  this->Script("pack %s %s -side left -padx 1", 
               this->CheckButton->GetWidgetName(),
               this->Label->GetWidgetName());

  // Listen to the checkbutton's SelectedStateChangedEvent so that we can
  // update the label's aspect each time the checkbutton's state changes.

  this->AddCallbackCommandObserver(
    this->CheckButton, vtkKWCheckButton::SelectedStateChangedEvent);

  // Update the label's aspect according to the checkbutton's state

  this->UpdateLabel();
}

// ---------------------------------------------------------------------------
void vtkKWMyWidget::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->PropagateEnableState(this->Label);
  this->PropagateEnableState(this->CheckButton);
}

//----------------------------------------------------------------------------
void vtkKWMyWidget::UpdateLabel()
{
  // If the checkbutton is selected, change the background color of the
  // label (reset the parent's background color otherwise)

  if (this->CheckButton && this->Label)
    {
    if (this->CheckButton->GetSelectedState())
      {
      this->Label->SetBackgroundColor(this->SelectedBackgroundColor);
      }
    else
      {
      this->Label->SetBackgroundColor(this->GetBackgroundColor());
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWMyWidget::ProcessCallbackCommandEvents(
  vtkObject *caller, unsigned long event, void *calldata)
{
  // We received a notification from the checkbutton that its state changed.
  // Let's update the label's font accordingly.

  if (caller == this->CheckButton && 
      event == vtkKWCheckButton::SelectedStateChangedEvent)
    {
    this->UpdateLabel();
    }

  this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);
}

//----------------------------------------------------------------------------
void vtkKWMyWidget::SetSelectedBackgroundColor(double r, double g, double b)
{
  if ((r == this->SelectedBackgroundColor[0] &&
       g == this->SelectedBackgroundColor[1] &&
       b == this->SelectedBackgroundColor[2]) ||
      r < 0.0 || r > 1.0 ||
      g < 0.0 || g > 1.0 ||
      b < 0.0 || b > 1.0)
    {
    return;
    }

  this->SelectedBackgroundColor[0] = r;
  this->SelectedBackgroundColor[1] = g;
  this->SelectedBackgroundColor[2] = b;
  
  this->Modified();

  this->UpdateLabel();
}

//----------------------------------------------------------------------------
void vtkKWMyWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "SelectedBackgroundColor: ("
     << this->SelectedBackgroundColor[0] << ", " 
     << this->SelectedBackgroundColor[1] << ", " 
     << this->SelectedBackgroundColor[2] << ")" << endl;
}
