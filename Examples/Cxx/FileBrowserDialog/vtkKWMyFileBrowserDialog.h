#ifndef __vtkKWMyFileBrowserDialog_h
#define __vtkKWMyFileBrowserDialog_h

#include "vtkKWFileBrowserDialog.h"

class vtkKWTextWithScrollbarsWithLabel;

class vtkKWMyFileBrowserDialog : public vtkKWFileBrowserDialog
{
public:
  static vtkKWMyFileBrowserDialog* New();
  vtkTypeRevisionMacro(vtkKWMyFileBrowserDialog,vtkKWFileBrowserDialog);

protected:
  vtkKWMyFileBrowserDialog();
  ~vtkKWMyFileBrowserDialog();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  vtkKWTextWithScrollbarsWithLabel *ContentText;

  virtual void UpdatePreview();
  virtual void ProcessFileSelectionChangedEvent(const char* fullname);

private:
  vtkKWMyFileBrowserDialog(const vtkKWMyFileBrowserDialog&);   // Not implemented.
  void operator=(const vtkKWMyFileBrowserDialog&);  // Not implemented.
};

#endif
