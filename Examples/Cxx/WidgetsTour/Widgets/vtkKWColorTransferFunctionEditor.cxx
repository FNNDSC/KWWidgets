#include "vtkColorTransferFunction.h"
#include "vtkKWApplication.h"
#include "vtkKWColorTransferFunctionEditor.h"
#include "vtkKWHistogram.h"
#include "vtkImageData.h"
#include "vtkPointData.h"
#include "vtkKWLabel.h"
#include "vtkKWWindow.h"
#include "vtkXMLImageDataReader.h"

#include "vtkKWWidgetsTourExample.h"

class vtkKWColorTransferFunctionEditorItem : public KWWidgetsTourItem
{
public:
  virtual int GetType();
  virtual void Create(vtkKWWidget *parent, vtkKWWindow *);
};

void vtkKWColorTransferFunctionEditorItem::Create(vtkKWWidget *parent, vtkKWWindow *)
{
  vtkKWApplication *app = parent->GetApplication();

  // Create the color transfer function that is modified by the 1st editor

  vtkColorTransferFunction *cpsel_tfunc1 = vtkColorTransferFunction::New();
  cpsel_tfunc1->SetColorSpaceToHSV();
  cpsel_tfunc1->AddHSVPoint(0.0, 0.66, 1.0, 1.0);
  cpsel_tfunc1->AddHSVPoint(512.0, 0.33, 1.0, 1.0);
  cpsel_tfunc1->AddHSVPoint(1024.0, 0.0, 1.0, 1.0);

  // Create a color transfer function editor
  // Assign our tfunc to the editor
  // Make sure we show the whole range of the tfunc

  vtkKWColorTransferFunctionEditor *cpsel_tfunc1_editor = 
    vtkKWColorTransferFunctionEditor::New();
  cpsel_tfunc1_editor->SetParent(parent);
  cpsel_tfunc1_editor->Create();
  cpsel_tfunc1_editor->SetBorderWidth(2);
  cpsel_tfunc1_editor->SetReliefToGroove();
  cpsel_tfunc1_editor->SetPadX(2);
  cpsel_tfunc1_editor->SetPadY(2);
  cpsel_tfunc1_editor->ParameterRangeVisibilityOff();
  cpsel_tfunc1_editor->SetCanvasHeight(30);
  cpsel_tfunc1_editor->SetBalloonHelpString(
    "A color transfer function editor. Double-click on a point to pop-up "
    "a color chooser. The parameter range slider is hidden in this one.");

  cpsel_tfunc1_editor->SetColorTransferFunction(cpsel_tfunc1);
  cpsel_tfunc1_editor->SetWholeParameterRangeToFunctionRange();
  cpsel_tfunc1_editor->SetVisibleParameterRangeToWholeParameterRange();

  app->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
    cpsel_tfunc1_editor->GetWidgetName());

  // -----------------------------------------------------------------------

  // This other transfer function editor is based on a real image data
  // Let's load it first

  vtkXMLImageDataReader *cpsel_reader = vtkXMLImageDataReader::New();
  cpsel_reader->SetFileName(
    vtkKWWidgetsTourExample::GetPathToExampleData(app, "head100x100x47.vti"));

  // The build an histogram of the data, it will be used inside the editor
  // as if we were trying to tune a tfunc based on the real values

  cpsel_reader->Update();
  vtkKWHistogram *cpsel_hist = vtkKWHistogram::New();
  cpsel_hist->BuildHistogram(
    cpsel_reader->GetOutput()->GetPointData()->GetScalars(), 0);

  double *range = cpsel_hist->GetRange();

  // Create the color transfer function that will be modified by the 2nd editor
  // This one shows a different look & feel

  vtkColorTransferFunction *cpsel_tfunc2 = vtkColorTransferFunction::New();
  cpsel_tfunc2->SetColorSpaceToHSV();
  cpsel_tfunc2->AddHSVPoint(range[0], 0.66, 1.0, 1.0);
  cpsel_tfunc2->AddHSVPoint((range[0] + range[1]) * 0.5, 0.0, 1.0, 1.0);
  cpsel_tfunc2->AddHSVPoint(range[1], 0.33, 1.0, 1.0);

  // Create a color transfer function editor
  // Assign our tfunc to the editor
  // Make sure we show the whole range of the tfunc
  // Use an histogram

  vtkKWColorTransferFunctionEditor *cpsel_tfunc2_editor = 
    vtkKWColorTransferFunctionEditor::New();
  cpsel_tfunc2_editor->SetParent(parent);
  cpsel_tfunc2_editor->Create();
  cpsel_tfunc2_editor->SetBorderWidth(2);
  cpsel_tfunc2_editor->SetReliefToGroove();
  cpsel_tfunc2_editor->SetPadX(2);
  cpsel_tfunc2_editor->SetPadY(2);
  cpsel_tfunc2_editor->ExpandCanvasWidthOff();
  cpsel_tfunc2_editor->SetCanvasWidth(450);
  cpsel_tfunc2_editor->SetCanvasHeight(150);
  cpsel_tfunc2_editor->SetLabelText("Transfer Function Editor");
  cpsel_tfunc2_editor->SetRangeLabelPositionToTop();
  cpsel_tfunc2_editor->SetBalloonHelpString(
    "Another color transfer function editor. The point position is now on "
    "top, the point style is an arrow down, guidelines are shown for each "
    "point (useful when combined with an histogram), point indices are "
    "hidden, ticks are displayed in the parameter space, the label "
    "and the parameter range are on top, its width is set explicitly. "
    "The range and histogram are based on a real image data.");

  cpsel_tfunc2_editor->SetColorTransferFunction(cpsel_tfunc2);
  cpsel_tfunc2_editor->SetWholeParameterRangeToFunctionRange();
  cpsel_tfunc2_editor->SetVisibleParameterRangeToWholeParameterRange();

  cpsel_tfunc2_editor->SetPointPositionInValueRangeToTop();
  cpsel_tfunc2_editor->SetPointStyleToCursorDown();
  cpsel_tfunc2_editor->FunctionLineVisibilityOff();
  cpsel_tfunc2_editor->PointGuidelineVisibilityOn();
  cpsel_tfunc2_editor->PointIndexVisibilityOff();
  cpsel_tfunc2_editor->SelectedPointIndexVisibilityOn();
  cpsel_tfunc2_editor->MidPointEntryVisibilityOn();
  cpsel_tfunc2_editor->SharpnessEntryVisibilityOn();
  cpsel_tfunc2_editor->SetLabelPositionToTop();

  cpsel_tfunc2_editor->SetHistogram(cpsel_hist);

  cpsel_tfunc2_editor->ParameterTicksVisibilityOn();
  cpsel_tfunc2_editor->ComputeValueTicksFromHistogramOn();
  cpsel_tfunc2_editor->SetParameterTicksFormat("%-#6.0f");

  app->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 20", 
    cpsel_tfunc2_editor->GetWidgetName());

  cpsel_tfunc1_editor->Delete();
  cpsel_tfunc1->Delete();
  cpsel_tfunc2_editor->Delete();
  cpsel_tfunc2->Delete();
  cpsel_hist->Delete();
  cpsel_reader->Delete();
}

int vtkKWColorTransferFunctionEditorItem::GetType()
{
  return KWWidgetsTourItem::TypeVTK;
}

KWWidgetsTourItem* vtkKWColorTransferFunctionEditorEntryPoint()
{
  return new vtkKWColorTransferFunctionEditorItem();
}
