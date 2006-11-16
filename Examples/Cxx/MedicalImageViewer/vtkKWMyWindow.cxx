#include "vtkKWMyWindow.h"

#include "vtkCornerAnnotation.h"
#include "vtkImageData.h"
#include "vtkImageViewer2.h"
#include "vtkKWApplication.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithSpinButtons.h"
#include "vtkKWMenuButtonWithSpinButtonsWithLabel.h"
#include "vtkKWNotebook.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWScale.h"
#include "vtkKWSimpleAnimationWidget.h"
#include "vtkKWWindow.h"
#include "vtkKWWindowLevelPresetSelector.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkXMLImageDataReader.h"

#include "vtkKWWidgetsPaths.h"
#include "vtkToolkits.h"

#include <vtksys/SystemTools.hxx>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWMyWindow );
vtkCxxRevisionMacro(vtkKWMyWindow, "$Revision: 1.3 $");

//----------------------------------------------------------------------------
vtkKWMyWindow::vtkKWMyWindow()
{
  this->RenderWidget = NULL;
  this->ImageViewer = NULL;
  this->SliceScale = NULL;
  this->WindowLevelPresetSelector = NULL;
  this->AnimationWidget = NULL;
}

//----------------------------------------------------------------------------
vtkKWMyWindow::~vtkKWMyWindow()
{
  if (this->SliceScale)
    {
    this->SliceScale->Delete();
    }
  if (this->ImageViewer)
    {
    this->ImageViewer->Delete();
    }
  if (this->RenderWidget)
    {
    this->RenderWidget->Delete();
    }
  if (this->WindowLevelPresetSelector)
    {
    this->WindowLevelPresetSelector->Delete();
    }
  if (this->AnimationWidget)
    {
    this->AnimationWidget->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkKWMyWindow::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro("class already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  vtkKWApplication *app = this->GetApplication();

  // Add a render widget, attach it to the view frame, and pack

  if (!this->RenderWidget)
    {
    this->RenderWidget = vtkKWRenderWidget::New();
    }
  this->RenderWidget->SetParent(this->GetViewFrame());
  this->RenderWidget->Create();
  this->RenderWidget->CornerAnnotationVisibilityOn();

  app->Script("pack %s -expand y -fill both -anchor c -expand y", 
              this->RenderWidget->GetWidgetName());

  // Create a volume reader

  vtkXMLImageDataReader *reader = vtkXMLImageDataReader::New();

  char data_path[2048];
  sprintf(data_path, "%s/Data/head100x100x47.vti", KWWidgets_EXAMPLES_DIR);
  if (!vtksys::SystemTools::FileExists(data_path))
    {
    sprintf(data_path,
            "%s/..%s/Examples/Data/head100x100x47.vti",
            app->GetInstallationDirectory(), KWWidgets_INSTALL_DATA_DIR);
    }
  reader->SetFileName(data_path);

  // Create an image viewer
  // Use the render window and renderer of the renderwidget

  if (!this->ImageViewer)
    {
    this->ImageViewer = vtkImageViewer2::New();
    }
  this->ImageViewer->SetRenderWindow(this->RenderWidget->GetRenderWindow());
  this->ImageViewer->SetRenderer(this->RenderWidget->GetRenderer());
  this->ImageViewer->SetInput(reader->GetOutput());
  this->ImageViewer->SetupInteractor(
    this->RenderWidget->GetRenderWindow()->GetInteractor());

  // Reset the window/level and the camera

  reader->Update();
  double *range = reader->GetOutput()->GetScalarRange();
  this->ImageViewer->SetColorWindow(range[1] - range[0]);
  this->ImageViewer->SetColorLevel(0.5 * (range[1] + range[0]));

  this->RenderWidget->ResetCamera();

  // The corner annotation has the ability to parse "tags" and fill
  // them with information gathered from other objects.
  // For example, let's display the slice and window/level in one corner
  // by connecting the corner annotation to our image actor and
  // image mapper

  vtkCornerAnnotation *ca = this->RenderWidget->GetCornerAnnotation();
  ca->SetImageActor(this->ImageViewer->GetImageActor());
  ca->SetWindowLevel(this->ImageViewer->GetWindowLevel());
  ca->SetText(2, "<slice>");
  ca->SetText(3, "<window>\n<level>");

  // Create a scale to control the slice

  if (!this->SliceScale)
    {
    this->SliceScale = vtkKWScale::New();
    }
  this->SliceScale->SetParent(this->GetViewPanelFrame());
  this->SliceScale->Create();
  this->SliceScale->SetCommand(this, "SetSliceFromScaleCallback");

  app->Script("pack %s -side top -expand n -fill x -padx 2 -pady 2",
              this->SliceScale->GetWidgetName());

  // Create a menu button to control the orientation

  vtkKWMenuButtonWithSpinButtonsWithLabel *orientation_menubutton =
    vtkKWMenuButtonWithSpinButtonsWithLabel::New();

  orientation_menubutton->SetParent(this->GetMainPanelFrame());
  orientation_menubutton->Create();
  orientation_menubutton->SetLabelText("Orientation:");
  orientation_menubutton->SetPadX(2);
  orientation_menubutton->SetPadY(2);
  orientation_menubutton->SetBorderWidth(2);
  orientation_menubutton->SetReliefToGroove();

  app->Script("pack %s -side top -anchor nw -expand n -fill x",
              orientation_menubutton->GetWidgetName());

  vtkKWMenuButton *mb = orientation_menubutton->GetWidget()->GetWidget();
  vtkKWMenu *menu = mb->GetMenu();

  menu->AddRadioButton("X-Y", this, "SetSliceOrientationToXYCallback");
  menu->AddRadioButton("X-Z", this, "SetSliceOrientationToXZCallback");
  menu->AddRadioButton("Y-Z", this, "SetSliceOrientationToYZCallback");

  mb->SetValue("X-Y");

  // Create a window/level preset selector

  vtkKWFrameWithLabel *wl_frame = vtkKWFrameWithLabel::New();
  wl_frame->SetParent(this->GetMainPanelFrame());
  wl_frame->Create();
  wl_frame->SetLabelText("Window/Level Presets");

  app->Script("pack %s -side top -anchor nw -expand n -fill x -pady 2",
              wl_frame->GetWidgetName());

  if (!this->WindowLevelPresetSelector)
    {
    this->WindowLevelPresetSelector = vtkKWWindowLevelPresetSelector::New();
    }
  this->WindowLevelPresetSelector->SetParent(wl_frame->GetFrame());
  this->WindowLevelPresetSelector->Create();
  this->WindowLevelPresetSelector->ThumbnailColumnVisibilityOn();
  this->WindowLevelPresetSelector->SetPresetAddCommand(
    this, "WindowLevelPresetAddCallback");
  this->WindowLevelPresetSelector->SetPresetApplyCommand(
    this, "WindowLevelPresetApplyCallback");
  this->WindowLevelPresetSelector->SetPresetUpdateCommand(
    this, "WindowLevelPresetUpdateCallback");
  this->WindowLevelPresetSelector->SetPresetHasChangedCommand(
    this, "WindowLevelPresetHasChangedCallback");

  app->Script("pack %s -side top -anchor nw -expand n -fill x",
              this->WindowLevelPresetSelector->GetWidgetName());

  // Create a simple animation widget

  vtkKWFrameWithLabel *animation_frame = vtkKWFrameWithLabel::New();
  animation_frame->SetParent(this->GetMainPanelFrame());
  animation_frame->Create();
  animation_frame->SetLabelText("Movie Creator");

  app->Script("pack %s -side top -anchor nw -expand n -fill x -pady 2",
              animation_frame->GetWidgetName());

  if (!this->AnimationWidget)
    {
    this->AnimationWidget = vtkKWSimpleAnimationWidget::New();
    }
  this->AnimationWidget->SetParent(animation_frame->GetFrame());
  this->AnimationWidget->Create();
  this->AnimationWidget->SetRenderWidget(this->RenderWidget);
  this->AnimationWidget->SetAnimationTypeToSlice();
  this->AnimationWidget->SetSliceSetCommand(this, "SetSliceCallback");
  this->AnimationWidget->SetSliceGetCommand(this, "GetSliceCallback");

  app->Script("pack %s -side top -anchor nw -expand n -fill x",
              this->AnimationWidget->GetWidgetName());

  this->UpdateSliceRanges();

  // Deallocate local objects

  reader->Delete();
  orientation_menubutton->Delete();
  wl_frame->Delete();
  animation_frame->Delete();
}

//----------------------------------------------------------------------------
void vtkKWMyWindow::SetSliceFromScaleCallback(double value)
{
  this->ImageViewer->SetSlice((int)value);
}

//----------------------------------------------------------------------------
void vtkKWMyWindow::SetSliceCallback(int slice)
{
  this->ImageViewer->SetSlice(slice);
}

//----------------------------------------------------------------------------
int vtkKWMyWindow::GetSliceCallback()
{
  return this->ImageViewer->GetSlice();
}

//----------------------------------------------------------------------------
int vtkKWMyWindow::GetSliceMinCallback()
{
  return this->ImageViewer->GetSliceMin();
}

//----------------------------------------------------------------------------
int vtkKWMyWindow::GetSliceMaxCallback()
{
  return this->ImageViewer->GetSliceMax();
}

//----------------------------------------------------------------------------
void vtkKWMyWindow::UpdateSliceRanges()
{
  this->SliceScale->SetRange(
    this->ImageViewer->GetSliceMin(), this->ImageViewer->GetSliceMax());
  this->SliceScale->SetValue(this->ImageViewer->GetSlice());

  this->AnimationWidget->SetSliceRange(
    this->ImageViewer->GetSliceMin(), this->ImageViewer->GetSliceMax());
}

//----------------------------------------------------------------------------
void vtkKWMyWindow::SetSliceOrientationToXYCallback()
{
  this->ImageViewer->SetSliceOrientationToXY();
  this->UpdateSliceRanges();
}

//----------------------------------------------------------------------------
void vtkKWMyWindow::SetSliceOrientationToXZCallback()
{
  this->ImageViewer->SetSliceOrientationToXZ();
  this->UpdateSliceRanges();
}

//----------------------------------------------------------------------------
void vtkKWMyWindow::SetSliceOrientationToYZCallback()
{
  this->ImageViewer->SetSliceOrientationToYZ();
  this->UpdateSliceRanges();
}

//----------------------------------------------------------------------------
void vtkKWMyWindow::WindowLevelPresetApplyCallback(int id)
{
  if (this->WindowLevelPresetSelector->HasPreset(id))
    {
    this->ImageViewer->SetColorWindow(
      this->WindowLevelPresetSelector->GetPresetWindow(id));
    this->ImageViewer->SetColorLevel(
      this->WindowLevelPresetSelector->GetPresetLevel(id));
    this->ImageViewer->Render();
    }
}
//----------------------------------------------------------------------------
void vtkKWMyWindow::WindowLevelPresetAddCallback()
{
  int id = this->WindowLevelPresetSelector->AddPreset();
  this->WindowLevelPresetUpdateCallback(id);
  this->WindowLevelPresetSelector->SelectPreset(id);
}

//----------------------------------------------------------------------------
void vtkKWMyWindow::WindowLevelPresetUpdateCallback(int id)
{
  this->WindowLevelPresetSelector->SetPresetWindow(
    id, this->ImageViewer->GetColorWindow());
  this->WindowLevelPresetSelector->SetPresetLevel(
    id, this->ImageViewer->GetColorLevel());
  this->WindowLevelPresetHasChangedCallback(id);
}

//----------------------------------------------------------------------------
void vtkKWMyWindow::WindowLevelPresetHasChangedCallback(int id)
{
  this->WindowLevelPresetSelector->
    BuildPresetThumbnailAndScreenshotFromRenderWindow(
      id, this->RenderWidget->GetRenderWindow());
}
