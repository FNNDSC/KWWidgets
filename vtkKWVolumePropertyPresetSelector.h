/*=========================================================================

  Module:    $RCSfile: vtkKWVolumePropertyPresetSelector.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWVolumePropertyPresetSelector - a volume property preset selector.
// .SECTION Description
// This class is a widget that can be used to store and apply volume property
// presets. 
// .SECTION Thanks
// This work is part of the National Alliance for Medical Image
// Computing (NAMIC), funded by the National Institutes of Health
// through the NIH Roadmap for Medical Research, Grant U54 EB005149.
// Information on the National Centers for Biomedical Computing
// can be obtained from http://nihroadmap.nih.gov/bioinformatics.

#ifndef __vtkKWVolumePropertyPresetSelector_h
#define __vtkKWVolumePropertyPresetSelector_h

#include "vtkKWPresetSelector.h"

class vtkVolumeProperty;
class vtkKWVolumePropertyPresetSelectorInternals;

class KWWidgets_EXPORT vtkKWVolumePropertyPresetSelector : public vtkKWPresetSelector
{
public:
  static vtkKWVolumePropertyPresetSelector* New();
  vtkTypeRevisionMacro(vtkKWVolumePropertyPresetSelector, vtkKWPresetSelector);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the volume property associated to the preset in the pool.
  // Note that the volume property object passed as parameter is neither
  // stored nor Register()'ed, only a copy is stored (and updated each
  // time this method is called later on).
  // Return 1 on success, 0 on error
  virtual int SetPresetVolumeProperty(int id, vtkVolumeProperty *prop);
  virtual vtkVolumeProperty* GetPresetVolumeProperty(int id);

  // Description:
  // Set/Get the type for a given preset.
  // The type column can be used, for example, to put the medical modality
  // a specific presets applies to (say, CT, MR)
  // The type field is not displayed as a column by default, but this
  // can be changed using the SetTypeColumnVisibility() method.
  // This column can not be edited.
  // Return 1 on success, 0 otherwise
  virtual int SetPresetType(int id, const char *type);
  virtual const char* GetPresetType(int id);

  // Description:
  // Set/Get if the volume property is designed with independent components
  // in mind.
  // IMPORTANT: this slot is a convenience slot that reflect the value
  // of the vtkVolumeProperty's own IndependentComponents ivar. Each time
  // the volume property preset is set (see SetPresetVolumeProperty), this
  // slot is updated automatically, and vice-versa.
  // It is provided for filtering purposes (i.e. so that you can use
  // this slot in a preset filter constraint, and only show the presets
  // that are relevant to your data, if it has independent components or not).
  // Return 1 on success, 0 otherwise
  virtual int GetPresetIndependentComponents(int id);
  virtual int SetPresetIndependentComponents(int id, int flag);

  // Description:
  // Set/Get if the volume property is designed with a specific blend mode
  // in mind. Valid constants are the ones found in vtkVolumeMapper, i.e.
  //   vtkVolumeMapper::COMPOSITE_BLEND, 
  //   vtkVolumeMapper::MAXIMUM_INTENSITY_BLEND,
  //   vtkVolumeMapper::MINIMUM_INTENSITY_BLEND.
  // Return 1 on success, 0 otherwise
  virtual int GetPresetBlendMode(int id);
  virtual int SetPresetBlendMode(int id, int flag);
  virtual int HasPresetBlendMode(int id);

  // Description:
  // Set/Get if the normalized scalar values in the volume property are to 
  // be interpreted relative to an histogram of the scalars.
  // Return 1 on success, 0 otherwise
  virtual int GetPresetHistogramFlag(int id);
  virtual int SetPresetHistogramFlag(int id, int flag);

  // Description:
  // Query if the preset range falls inside a given range
  // (the preset range is computed by checking the largest scalar range among
  // the volume property transfer functions for component 0).
  // Return 1 on success (or HistogramFlag is On), 0 on error
  virtual int IsPresetRangeInsideRange(int id, double range[2]);

  // Description:
  // Set/Get the visibility of the type column. Hidden by default.
  // No effect if called before Create().
  virtual void SetTypeColumnVisibility(int);
  virtual int GetTypeColumnVisibility();
  vtkBooleanMacro(TypeColumnVisibility, int);

  // Description:
  // Add default normalized presets.
  // The type parameter will be used to call SetPresetType on each new preset.
  virtual void AddDefaultNormalizedPresets(const char *type);

  // Description:
  // Add a preset filter constraint on the preset group field.
  virtual void SetPresetFilterRangeConstraint(double range[2]);
  virtual double* GetPresetFilterRangeConstraint();
  virtual void DeletePresetFilterRangeConstraint();

  // Description:
  // Query if a given preset matches the current preset filter constraints.
  // Return 1 if match or if no filter was defined, 0 otherwise
  // Override (augment) the superclass to handle range constraint (see
  // SetPresetFilterRangeConstraint).
  virtual int IsPresetFiltered(int id);

  // Description:
  // Most (if not all) of the information associated to a preset (say group, 
  // comment, filename, creation time, thumbnail and screenshot) is stored
  // under the hood as user slots using the corresponding API (i.e. 
  // Set/GetPresetUserSlotAs...()). Since each slot requires a unique name,
  // the following methods are provided to retrieve the slot name for
  // the preset fields. This can be useful to avoid collision between
  // the default slots and your own user slots. Note that the default slot
  // names can be changed too, but doing so will not transfer the value
  // stored at the old slot name to the new slot name (it is up to you to do
  // so, if needed).
  virtual void SetPresetTypeSlotName(const char *);
  virtual const char* GetPresetTypeSlotName();
  virtual void SetPresetIndependentComponentsSlotName(const char *);
  virtual const char* GetPresetIndependentComponentsSlotName();
  virtual void SetPresetHistogramFlagSlotName(const char *);
  virtual const char* GetPresetHistogramFlagSlotName();
  virtual void SetPresetBlendModeSlotName(const char *);
  virtual const char* GetPresetBlendModeSlotName();

  // Description:
  // Some constants
  //BTX
  static const char *TypeColumnName;
  //ETX

protected:
  vtkKWVolumePropertyPresetSelector();
  ~vtkKWVolumePropertyPresetSelector();

  // Description:
  // Create the columns.
  // Subclasses should override this method to add their own columns and
  // display their own preset fields (do not forget to call the superclass
  // first).
  virtual void CreateColumns();

  // Description:
  // Update the preset row, i.e. add a row for that preset if it is not
  // displayed already, hide it if it does not match GroupFilter, and
  // update the table columns with the corresponding preset fields.
  // Subclass should override this method to display their own fields.
  // Return 1 on success, 0 if the row was not (or can not be) updated.
  // Subclasses should call the parent's UpdatePresetRow, and abort
  // if the result is not 1.
  virtual int UpdatePresetRow(int id);

  // Description:
  // Get the index of a given column.
  virtual int GetTypeColumnIndex();

  // Description:
  // Update the toolbar preset buttons state/visibility.
  virtual void UpdateToolbarPresetButtons(vtkKWToolbar*);

  // Description:
  // Populate the pop-up context menu that is displayed when right-clicking
  // on a give preset. It should replicate the commands available through the
  // preset buttons.
  virtual void PopulatePresetContextMenu(vtkKWMenu *menu, int id);

  // PIMPL Encapsulation for STL containers
  //BTX
  vtkKWVolumePropertyPresetSelectorInternals *Internals;
  //ETX

private:

  vtkKWVolumePropertyPresetSelector(const vtkKWVolumePropertyPresetSelector&); // Not implemented
  void operator=(const vtkKWVolumePropertyPresetSelector&); // Not implemented
};

#endif
