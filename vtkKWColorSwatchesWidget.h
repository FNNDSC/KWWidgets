/*=========================================================================

  Module:    $RCSfile: vtkKWColorSwatchesWidget.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWColorSwatchesWidget - a color swatches widget.
// .SECTION Description
// This class displays a list of color swatches.
// Different color swatches collections can be created independently.
// .SECTION See Also
// vtkKWColorPickerWidget

#ifndef __vtkKWColorSwatchesWidget_h
#define __vtkKWColorSwatchesWidget_h

#include "vtkKWCompositeWidget.h"

class vtkKWColorSwatchesWidgetInternals;
class vtkKWFrameSet;

class KWWidgets_EXPORT vtkKWColorSwatchesWidget : public vtkKWCompositeWidget
{
public:
  static vtkKWColorSwatchesWidget* New();
  vtkTypeRevisionMacro(vtkKWColorSwatchesWidget,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Add a swatches collection (the name can not be empty)
  // Return unique ID (>= 0) of the collection, or -1 on error. 
  virtual int AddCollection(const char *name);

  // Description:
  // Remove a swatches collection given its ID, or all collections
  // Return 1 on success, 0 otherwise
  virtual int RemoveCollection(int collection_id);
  virtual void RemoveAllCollections();

  // Description:
  // Get the unique ID of a swatches collection given its name
  // Return -1 on error. 
  virtual int GetCollectionId(const char *name);

  // Description:
  // Add a swatch to a collection, given the unique ID of the collection,
  // the name of the swatch (can be empty) and its RGB value.
  // Return 1 on success, 0 on error.
  virtual int AddRGBSwatch(
    int collection_id, const char *name, double rgb[3]);
  virtual int AddRGBSwatch(
    int collection_id, const char *name, double r, double g, double b);

  // Description:
  // Set/Get the swatch size in pixels.
  vtkGetMacro(SwatchSize, int);
  virtual void SetSwatchSize(int);

  // Description:
  // Set/Get the maximum number of swatches packed horizontally (per row).
  virtual void SetMaximumNumberOfSwatchesPerRow(int);
  virtual int GetMaximumNumberOfSwatchesPerRow();

  // Description:
  // Set/Get the padding between each swatch.
  virtual void SetSwatchesPadding(int);
  virtual int GetSwatchesPadding();

  // Description:
  // Create the default collections
  virtual void AddDefaultCollections();

  // Description:
  // Specifies a command to associate with the widget. This command is 
  // invoked when a swatch is selected.
  // The 'object' argument is the object that will have the method called on
  // it. The 'method' argument is the name of the method to be called and any
  // arguments in string form. If the object is NULL, the method is still
  // evaluated as a simple command. 
  // The following parameters are also passed to the command:
  // - swatch rgb value: double r, double g, double b
  virtual void SetSwatchSelectedCommand(vtkObject *object, const char *method);

  // Description:
  // Callbacks. Internal, do not use.
  virtual void SwatchSelectedCallback(double r, double g, double b);
  virtual void CollectionSelectedCallback(const char*);
  virtual void PopulateCollectionsCallback();
  virtual void PopulateSwatchesCallback();

  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object is updated
  // and propagated to its internal parts/subwidgets. This will, for example,
  // enable/disable parts of the widget UI, enable/disable the visibility
  // of 3D widgets, etc.
  virtual void UpdateEnableState();

protected:
  vtkKWColorSwatchesWidget();
  ~vtkKWColorSwatchesWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Description:
  // Callbacks. Internal, do not use.
  char *SwatchSelectedCommand;
  virtual void InvokeSwatchSelectedCommand(double r, double g, double b);

  // Description:
  // Populate the swatches frame and collectio combo
  virtual void PopulateCollections();
  virtual void PopulateSwatches();
  virtual void SchedulePopulateCollections();
  virtual void SchedulePopulateSwatches();
  
  int SwatchSize;
  vtkKWColorSwatchesWidgetInternals *Internals;

private:
  vtkKWColorSwatchesWidget(const vtkKWColorSwatchesWidget&); // Not implemented
  void operator=(const vtkKWColorSwatchesWidget&); // Not Implemented
};

#endif

