/*=========================================================================

  Module:    $RCSfile: vtkKWPresetSelector.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWPresetSelector - a preset selector.
// .SECTION Description
// This class is the basis for a preset selector framework. 
// Presets can be added to the selector and identified by unique IDs. 
// They can be assigned pre-defined fields like a group, a filename, 
// a comment, a thumbnail and a screenshot, as well as an unlimited number
// of user-defined slots.
// Presets are listed vertically in a table list widget (vtkKWMultiColumnList),
// one by row. Each column is used to display one of the predefined field.
// The class can be used as-is, or extended to support more columns or
// features, as examplified in the vtkKWWindowLevelPresetSelector and
// vtkKWVolumePropertyPresetSelector sub-classes.
// Several callbacks can be specified to enable external code to
// add presets, apply presets, update them, etc.
// presets. 
// .SECTION Thanks
// This work is part of the National Alliance for Medical Image
// Computing (NAMIC), funded by the National Institutes of Health
// through the NIH Roadmap for Medical Research, Grant U54 EB005149.
// Information on the National Centers for Biomedical Computing
// can be obtained from http://nihroadmap.nih.gov/bioinformatics.
// .SECTION See Also
// vtkKWWindowLevelPresetSelector vtkKWVolumePropertyPresetSelector

#ifndef __vtkKWPresetSelector_h
#define __vtkKWPresetSelector_h

#include "vtkKWCompositeWidget.h"

class vtkImageData;
class vtkKWIcon;
class vtkKWMultiColumnListWithScrollbars;
class vtkKWPresetSelectorInternals;
class vtkKWPushButtonSet;
class vtkKWSpinButtons;
class vtkRenderWindow;

class KWWIDGETS_EXPORT vtkKWPresetSelector : public vtkKWCompositeWidget
{
public:
  static vtkKWPresetSelector* New();
  vtkTypeRevisionMacro(vtkKWPresetSelector, vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create the widget.
  virtual void Create(vtkKWApplication *app);
  
  // Description:
  // Add a new preset.
  // Return the unique Id of the preset
  virtual int AddPreset();

  // Description:
  // Query if pool has given preset.
  // Return 1 if in the pool, 0 otherwise
  virtual int HasPreset(int id);

  // Description:
  // Set/Get the group associated to a preset.
  // This provide a way of grouping presets and filter them out using
  // the SetGroupFilter() method.
  // The group field is not displayed as a column by default, but this
  // can be changed using the SetGroupColumnVisibility() method.
  // This column can not be edited by default, but this can be changed by
  // a subclass.
  // Return 1 on success, 0 on error
  virtual int SetPresetGroup(int id, const char *group);
  virtual const char* GetPresetGroup(int id);

  // Description:
  // Set/Get the visibility of the group column. Hidden by default.
  // No effect if called before Create().
  virtual void SetGroupColumnVisibility(int);
  virtual int GetGroupColumnVisibility();
  vtkBooleanMacro(GroupColumnVisibility, int);

  // Description:
  // Set/Get the title of the group column.
  // No effect if called before Create().
  virtual void SetGroupColumnTitle(const char *);
  virtual const char* GetGroupColumnTitle();

  // Description:
  // Set/Get the group filter.
  // Set it to a specific group to show only those presets with a matching
  // group. Set it to NULL to display all presets.
  virtual void SetGroupFilter(const char *group);
  vtkGetStringMacro(GroupFilter);

  // Description:
  // Set/Get the comment associated to a preset.
  // This provide a way to create a small description or comment for
  // each preset. 
  // The comment field is displayed as a column by default, but this 
  // can be changed using the SetCommentColumnVisibility() method.
  // This column can be edited by default, by double-clicking
  // on the corresponding table cell.
  // Return 1 on success, 0 on error
  virtual int SetPresetComment(int id, const char *comment);
  virtual const char* GetPresetComment(int id);

  // Description:
  // Set/Get the visibility of the comment column.
  // No effect if called before Create().
  virtual void SetCommentColumnVisibility(int);
  virtual int GetCommentColumnVisibility();
  vtkBooleanMacro(CommentColumnVisibility, int);

  // Description:
  // Set/Get the filename associated to a preset.
  // This field is neither used nor displayed in this implementation
  // but is provided as a convenience for subclasses that need to
  // save preset to disks and keep track of the corresponding filename.
  // Return 1 on success, 0 on error
  virtual int SetPresetFileName(int id, const char *filename);
  virtual const char* GetPresetFileName(int id);

  // Description:
  // Get the creation time of a preset, as returned by
  // the vtksys::SystemTools::GetTime() method (can be cast to time_t)
  // This field is not displayed in this implementation, but is
  // used internally in the thumbnail column so that sorting by
  // "thumbnail" will actually sort by creation time.
  // Return 0 on error.
  virtual double GetPresetCreationTime(int id);

  // Description:
  // Assign an image/screenshot to a preset.
  // The 'image' is not stored but is used right-away to create a thumbnail
  // icon to be displayed in the thumbnail column, and a small screenshot to
  // be displayed as a pop-up when the user hovers over that thumbnail. 
  // The thumbnail field is not displayed as a column by default, but this 
  // can be changed using the SetThumbnailColumnVisibility() method.
  // If passed a vtkRenderWindow, grabs an image of the window contents.
  // Both thumbnail and screenshot icons can be retrieved.
  virtual int SetPresetImage(int id, vtkImageData *image);
  virtual int SetPresetImageFromRenderWindow(int id, vtkRenderWindow *win);
  virtual vtkKWIcon* GetPresetThumbnail(int id);
  virtual vtkKWIcon* GetPresetScreenshot(int id);

  // Description:
  // Set/Get the visibility of the thumbnail column.
  // No effect if called before Create().
  virtual void SetThumbnailColumnVisibility(int);
  virtual int GetThumbnailColumnVisibility();
  vtkBooleanMacro(ThumbnailColumnVisibility, int);

  // Description:
  // Set/Get the thumbnail size.
  // Changing the size will not resize the current thumbnails, but will
  // affect the presets added to the selector later on.
  vtkSetClampMacro(ThumbnailSize,int,8,512);
  vtkGetMacro(ThumbnailSize,int);

  // Description:
  // Set/Get the screenshot size, i.e. the image that appears as
  // a popup when the mouse is on top of the thumbnail.
  // Changing the size will not resize the current screenshots, but will
  // affect the presets added to the selector later on.
  vtkSetClampMacro(ScreenshotSize,int,8,2048);
  vtkGetMacro(ScreenshotSize,int);

  // Description:
  // Set/Get a preset user slot.
  // An unlimited number of slots can be added to a preset. Each slot is
  // identified by a name (string). Convenience methods are provided to store
  // and retrieve various types of data (double, int, string, pointer).
  // Return 1 on success, 0 on error
  virtual int HasPresetUserSlot(
    int id, const char *slot_name);
  virtual int SetPresetUserSlotAsDouble(
    int id, const char *slot_name, double value);
  virtual double GetPresetUserSlotAsDouble(
    int id, const char *slot_name);
  virtual int SetPresetUserSlotAsInt(
    int id, const char *slot_name, int value);
  virtual int GetPresetUserSlotAsInt(
    int id, const char *slot_name);
  virtual int SetPresetUserSlotAsString(
    int id, const char *slot_name, const char *value);
  virtual const char* GetPresetUserSlotAsString(
    int id, const char *slot_name);
  virtual int SetPresetUserSlotAsPointer(
    int id, const char *slot_name, void *ptr);
  virtual void* GetPresetUserSlotAsPointer(
    int id, const char *slot_name);

  // Description:
  // Get the number of presets, or the number of presets with a specific
  // group, or the number of visible presets, i.e. the presets that are
  // displayed according to the GroupFilter for example.
  virtual int GetNumberOfPresets();
  virtual int GetNumberOfPresetsWithGroup(const char *group);
  virtual int GetNumberOfVisiblePresets();

  // Description:
  // Retrieve the Id of the nth-preset, or the id of the
  // nth preset with a given group.
  // Return id on success, -1 otherwise
  virtual int GetNthPresetId(int index);
  virtual int GetNthPresetWithGroupId(int index, const char *group);

  // Description:
  // Retrieve the Id of the preset at a given row in the table, or
  // the row of a given preset.
  // Return id or row index on success, -1 otherwise
  virtual int GetPresetAtRowId(int row_index);
  virtual int GetPresetRow(int id);

  // Description:
  // Retrieve the rank of the nth preset with a given group
  // (i.e. the nth-preset with a given group).
  // This rank can then be used to retrieve the preset id using
  // the GetNthPresetId() method.
  // Return rank on success, -1 otherwise
  virtual int GetNthPresetWithGroupRank(int index, const char *group);

  // Description:
  // Remove a preset, or all of them, or all of the presets
  // with the same group.
  // Return 1 on success, 0 on error
  virtual int RemovePreset(int id);
  virtual int RemoveAllPresets();
  virtual int RemoveAllPresetsWithGroup(const char *group);

  // Description:
  // Set/Get the list height (in number of items)
  // No effect if called before Create().
  virtual void SetListHeight(int);
  virtual int GetListHeight();

  // Description:
  // Set/Get the visibility of the select spin buttons.
  // The select spin button are two buttons that can be used
  // to select the next or previous preset in the list.
  // Note that if ApplyPresetOnSelection is On, this will also apply
  // the preset at the same time, thus providing a quick way to
  // loop over all presets and apply them.
  virtual void SetSelectSpinButtonsVisibility(int);
  vtkGetMacro(SelectSpinButtonsVisibility,int);
  vtkBooleanMacro(SelectSpinButtonsVisibility,int);

  // Description:
  // Set/Get the visibility of the locate preset button (hidden by default).
  // If visible, triggering this button will locate all selected presets
  // by calling the GetPresetFileName method and trying to open
  // the directory they are in and select the proper file.
  // Win32 only at the moment.
  virtual void SetLocateButtonVisibility(int);
  vtkGetMacro(LocateButtonVisibility,int);
  vtkBooleanMacro(LocateButtonVisibility,int);

  // Description:
  // Set/Get the visibility of the email preset button (hidden by default).
  // If visible, triggering this button will email all selected presets
  // as attachments. The attachment location is retrieved by calling
  // the GetPresetFileName method. Win32/MAPI only at the moment.
  virtual void SetEmailButtonVisibility(int);
  vtkGetMacro(EmailButtonVisibility,int);
  vtkBooleanMacro(EmailButtonVisibility,int);

  // Description:
  // Specifies a command to be invoked when the "add preset" button is pressed.
  // This gives the opportunity for the application to check and collect the
  // relevant information to store in a new preset. The application is then
  // free to add the preset (using the AddPreset() method) and set its
  // fields independently (using the SetPresetGroup(), SetPresetComment(),
  // SetPreset...() methods).
  // Note that if not set, the "add preset" button is not visible.
  virtual void SetPresetAddCommand(vtkObject* object, const char *method);

  // Description:
  // Specifies a command to be invoked when the "update selected preset" button
  // is pressed. This gives the opportunity for the application to check and
  // collect the relevant information to update in the preset. The application
  // is then free to update the preset's fields independently (using the
  // SetPresetGroup(), SetPresetComment(), SetPreset...() methods).
  // The id of the preset to update is passed to the command.
  // Note that if not set, the "update selected preset" button is not visible.
  virtual void SetPresetUpdateCommand(vtkObject* object, const char *method);

  // Description:
  // Specifies a command to be invoked when the "apply selected preset" 
  // button is pressed, or when ApplyPresetOnSelection is On and the user
  // applies a preset by selecting it directly. This gives the opportunity
  // for the application to query the preset's fields independently (using the
  // GetPresetGroup(), GetPresetComment(), GetPreset...() methods) and
  // apply those values to the relevant objects.
  // The id of the preset to apply is passed to the command.
  // Note that if not set or ApplyPresetOnSelection is On, the 
  // "apply selected preset" button is not visible.
  virtual void SetPresetApplyCommand(vtkObject* object, const char *method);

  // Description:
  // Set/Get if a preset should be applied directly when it is selected by a
  // single-click, or only when the "apply selected preset" button is pressed.
  // If set, only one preset can be selected at a time (if not, multiple
  // preset can be selected, and removed for example).
  // Note that if set, the "apply selected preset" button is not visible.
  virtual void SetApplyPresetOnSelection(int);
  vtkGetMacro(ApplyPresetOnSelection,int);
  vtkBooleanMacro(ApplyPresetOnSelection,int);

  // Description:
  // Specifies a command to be invoked when the "remove selected preset"
  // button is pressed. This command is called *before* the preset is
  // removed from the pool: this gives the opportunity for the application 
  // to query the preset's fields independently (using the
  // GetPresetGroup(), GetPresetComment(), GetPreset...() methods),
  // decide if the preset should be removed or not, and delete it from
  // its internal structures accordingly, if needed.
  // The id of the preset to remove is passed to the command.
  // This command is expected to return an integer equal to 1 if the preset
  // is really to be removed, 0 otherwise.
  virtual void SetPresetRemoveCommand(vtkObject* object, const char *method);

  // Description:
  // Set/Get if the user should be prompted before removing one or
  // more presets using "remove selected preset" button.
  vtkSetMacro(PromptBeforeRemovePreset, int);
  vtkGetMacro(PromptBeforeRemovePreset, int);
  vtkBooleanMacro(PromptBeforeRemovePreset, int);

  // Description:
  // Specifies a command to be invoked when the preset has been
  // changed using direct user-interaction on the widget interface. 
  // This includes double-clicking on a table cell (the comment field for
  // example) and editing the contents of the cell directly, when allowed.
  // This gives the opportunity for the application to query the preset's
  // fields independently (using the GetPresetGroup(), GetPresetComment(), 
  // GetPreset...() methods), and update its internal structures accordingly, 
  // if needed.
  // The id of the preset that has changed is passed to the command.
  virtual void SetPresetHasChangedCommand(
    vtkObject* object, const char *method);

  // Description:
  // Refresh the interface.
  virtual void Update();

  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object is updated
  // and propagated to its internal parts/subwidgets. This will, for example,
  // enable/disable parts of the widget UI, enable/disable the visibility
  // of 3D widgets, etc.
  virtual void UpdateEnableState();

  // Description:
  // Callbacks
  virtual void PresetAddCallback();
  virtual void PresetApplyCallback();
  virtual void PresetUpdateCallback();
  virtual void PresetRemoveCallback();
  virtual void PresetEmailCallback();
  virtual void PresetLocateCallback();
  virtual void PresetSelectionCallback();
  virtual void PresetSelectPreviousCallback();
  virtual void PresetSelectNextCallback();

  // Description:
  // Callback used to refresh the contents of the image cell for each preset
  virtual void PresetCellThumbnailCallback(const char*, int, int, const char*);

  // Description:
  // Callback invoked when the user starts editing a specific preset field
  // located at cell ('row', 'col'), which current contents is 'text'.
  // This method returns the value that is to become the initial 
  // contents of the temporary embedded widget used for editing: most of the
  // time, this is the same value as 'text'.
  // The next step (validation) is handled by PresetCellEditEndCallback
  virtual const char* PresetCellEditStartCallback(
    int row, int col, const char *text);

  // Description:
  // Callback invoked when the user ends editing a specific preset field
  // located at cell ('row', 'col').
  // The main purpose of this method is to perform a final validation of
  // the edit window's contents 'text'.
  // This method returns the value that is to become the new contents
  // for that cell.
  // The next step (updating) is handled by PresetCellUpdateCallback
  virtual const char* PresetCellEditEndCallback(
    int row, int col, const char *text);

  // Description:
  // Callback invoked when the user successfully updated the preset field
  // located at ('row', 'col') with the new contents 'text', as a result
  // of editing the corresponding cell interactively.
  virtual void PresetCellUpdatedCallback(int row, int col, const char *text);

  // Description:
  // Some constants
  //BTX
  static const char *IdColumnName;
  static const char *ThumbnailColumnName;
  static const char *GroupColumnName;
  static const char *CommentColumnName;
  //ETX

protected:
  vtkKWPresetSelector();
  ~vtkKWPresetSelector();

  // Description:
  // Create the columns.
  // Subclasses should override this method to add their own columns and
  // display their own preset fields (do not forget to call the superclass
  // first).
  virtual void CreateColumns();

  // Description:
  // Deallocate a preset.
  // Subclasses should override this method to release the memory allocated
  // by their own preset fields  (do not forget to call the superclass
  // first).
  virtual void DeAllocatePreset(int vtkNotUsed(id)) {};

  // Description:
  // Update the preset row, i.e. add a row for that preset if it is not
  // displayed already, hide it if it does not match GroupFilter, and
  // update the table columns with the corresponding preset fields.
  // Subclass should override this method to display their own fields.
  // Return 1 on success, 0 if the row was not (or can not be) updated.
  // Subclasses should call the parent's UpdatePresetRow, and abort
  // if the result is not 1.
  virtual int UpdatePresetRow(int id);

  vtkKWMultiColumnListWithScrollbars *PresetList;
  vtkKWFrame                         *PresetControlFrame;
  vtkKWSpinButtons                   *PresetSelectSpinButtons;
  vtkKWPushButtonSet                 *PresetButtons;

  int ApplyPresetOnSelection;
  int SelectSpinButtonsVisibility;
  int EmailButtonVisibility;
  int LocateButtonVisibility;

  int ThumbnailSize;
  int ScreenshotSize;
  int PromptBeforeRemovePreset;

  char* GroupFilter;

  // Description:
  // Called when the number of presets has changed
  virtual void NumberOfPresetsHasChanged();

  // PIMPL Encapsulation for STL containers
  //BTX
  vtkKWPresetSelectorInternals *Internals;
  //ETX

  // Description:
  // Update all rows in the list
  virtual void UpdateRowsInPresetList();

  char *PresetAddCommand;
  virtual void InvokePresetAddCommand();

  char *PresetUpdateCommand;
  virtual void InvokePresetUpdateCommand(int id);

  char *PresetApplyCommand;
  virtual void InvokePresetApplyCommand(int id);

  char *PresetRemoveCommand;
  virtual int InvokePresetRemoveCommand(int id);

  char *PresetHasChangedCommand;
  virtual void InvokePresetHasChangedCommand(int id);

  // Description:
  // Convenience methods to get the index of a given column
  virtual int GetIdColumnIndex();
  virtual int GetThumbnailColumnIndex();
  virtual int GetGroupColumnIndex();
  virtual int GetCommentColumnIndex();

  // Description:
  // Pack
  virtual void Pack();

  // Description:
  // Set the default balloon help strings
  // Subclass can override this method to change the help strings
  // associated to the buttons.
  virtual void SetDefaultHelpStrings();

  // Description:
  // Some constants
  //BTX
  static int AddButtonId;
  static int ApplyButtonId;
  static int UpdateButtonId;
  static int RemoveButtonId;
  static int LocateButtonId;
  static int EmailButtonId;
  //ETX

private:

  vtkKWPresetSelector(const vtkKWPresetSelector&); // Not implemented
  void operator=(const vtkKWPresetSelector&); // Not implemented
};

#endif
