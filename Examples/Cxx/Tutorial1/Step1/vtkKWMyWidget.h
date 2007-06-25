#ifndef __vtkKWMyWidget_h
#define __vtkKWMyWidget_h

#include "vtkKWCompositeWidget.h"

class vtkKWLabel;

class vtkKWMyWidget : public vtkKWCompositeWidget
{
public:
  static vtkKWMyWidget* New();
  vtkTypeRevisionMacro(vtkKWMyWidget,vtkKWCompositeWidget);

  // Description:
  // Get the internal widget.
  vtkGetObjectMacro(Label, vtkKWLabel);

protected:
  vtkKWMyWidget();
  ~vtkKWMyWidget();

  vtkKWLabel *Label;

  // Description:
  // Create the widget.
  virtual void CreateWidget();

private:
  vtkKWMyWidget(const vtkKWMyWidget&);   // Not implemented.
  void operator=(const vtkKWMyWidget&);  // Not implemented.
};

#endif
