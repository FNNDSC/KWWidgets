/*=========================================================================

  Module:    $RCSfile: vtkKWTextPropertyEditor.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWTextPropertyEditor - a GUI component that can be used to edit vtkTextProperty objects
// .SECTION Description
// The vtkKWTextPropertyEditor creates a set of GUI components that can be displayed
// and used selectively to edit all or part of a vtkTextProperty object.


#ifndef __vtkKWTextPropertyEditor_h
#define __vtkKWTextPropertyEditor_h

#include "vtkKWCompositeWidget.h"

class vtkActor2D;
class vtkKWApplication;
class vtkKWChangeColorButton;
class vtkKWLabel;
class vtkKWCheckButtonSetLabeled;
class vtkKWMenuButtonLabeled;
class vtkKWPushButtonSetLabeled;
class vtkKWPushButton;
class vtkKWScale;
class vtkTextProperty;

class KWWIDGETS_EXPORT vtkKWTextPropertyEditor : public vtkKWCompositeWidget
{
public:
  static vtkKWTextPropertyEditor* New();
  vtkTypeRevisionMacro(vtkKWTextPropertyEditor,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create the widget.
  virtual void Create(vtkKWApplication *app);

  // Description
  // Refresh/Update the interface according to the value of the text property
  // and actor2d
  virtual void Update();

  // Description:
  // Set/Get the text property to control.
  virtual void SetTextProperty(vtkTextProperty*);
  vtkGetObjectMacro(TextProperty, vtkTextProperty);

  // Description:
  // Set/Get the actor that uses TextProperty. This is optional, but might
  // help to solve some backward compatibility issues. For example, the
  // default vtkTextProperty color is -1, -1, -1 to specify to the mapper
  // that the vtkActor2D color has to be used instead.
  virtual void SetActor2D(vtkActor2D*);
  vtkGetObjectMacro(Actor2D, vtkActor2D);

  // Description:
  // Set the widget aspect to be long, i.e. the widgets will be packed on 
  // several rows, with description labels. The default is short (all widgets
  // on a row).
  virtual void SetLongFormat(int);
  vtkBooleanMacro(LongFormat, int);
  vtkGetMacro(LongFormat, int);

  // Description:
  // Show the label on top (default, otherwise on left. 
  // Valid if LongFormat is On.
  virtual void SetLabelOnTop(int);
  vtkBooleanMacro(LabelOnTop, int);
  vtkGetMacro(LabelOnTop, int);

  // Description:
  // Show label.
  virtual void SetShowLabel(int);
  vtkBooleanMacro(ShowLabel, int);
  vtkGetMacro(ShowLabel, int);
  vtkGetObjectMacro(Label, vtkKWLabel);

  // Description:
  // Show color.
  virtual void SetShowColor(int);
  vtkBooleanMacro(ShowColor, int);
  vtkGetMacro(ShowColor, int);

  // Description:
  // Show font family.
  virtual void SetShowFontFamily(int);
  vtkBooleanMacro(ShowFontFamily, int);
  vtkGetMacro(ShowFontFamily, int);

  // Description:
  // Show style.
  virtual void SetShowStyles(int);
  vtkBooleanMacro(ShowStyles, int);
  vtkGetMacro(ShowStyles, int);

  // Description:
  // Show opacity.
  virtual void SetShowOpacity(int);
  vtkBooleanMacro(ShowOpacity, int);
  vtkGetMacro(ShowOpacity, int);

  // Description:
  // Set/Get the command executed each time a change is made to the
  // text property.
  virtual void SetChangedCommand(
    vtkObject *object, const char *method);

  // Description:
  // Set/Get the command executed each time a change is made to the
  // color of the text property (the ChangedCommand is run too).
  virtual void SetColorChangedCommand(
    vtkObject *object, const char *method);

  // Description:
  // Show copy button. This button can be used to synchronize different
  // text property widgets.
  virtual void SetShowCopy(int);
  vtkBooleanMacro(ShowCopy, int);
  vtkGetMacro(ShowCopy, int);
  virtual vtkKWPushButton* GetCopyButton();

  // Description:
  // Copy the values from another text widget
  virtual void CopyValuesFrom(vtkKWTextPropertyEditor*);

  // GUI components callbacks
  virtual void ChangeColorButtonCallback(double, double, double);
  virtual void SetColor(double, double, double);
  virtual void SetColor(double *v) { this->SetColor(v[0], v[1], v[2]); };
  virtual double* GetColor();
  virtual void FontFamilyCallback();
  virtual void SetFontFamily(int);
  virtual void BoldCallback();
  virtual void SetBold(int);
  virtual void ItalicCallback();
  virtual void SetItalic(int);
  virtual void ShadowCallback();
  virtual void SetShadow(int);
  virtual void OpacityCallback();
  virtual void OpacityEndCallback();
  virtual void SetOpacity(float);
  virtual float GetOpacity();

  // Description:
  // Save out the text properties to a file.
  virtual void SaveInTclScript(ofstream *file, const char *tcl_name = 0,
                               int tabify = 1);

  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object is updated
  // and propagated to its internal parts/subwidgets. This will, for example,
  // enable/disable parts of the widget UI, enable/disable the visibility
  // of 3D widgets, etc.
  virtual void UpdateEnableState();

protected:
  vtkKWTextPropertyEditor();
  ~vtkKWTextPropertyEditor();

  virtual void Pack();

  void UpdateInterface();
  void UpdateLabel();
  void UpdateColorButton();
  void UpdateFontFamilyOptionMenu();
  void UpdateStylesCheckButtonSet();
  void UpdateBoldCheckButton();
  void UpdateItalicCheckButton();
  void UpdateShadowCheckButton();
  void UpdateOpacityScale();
  void UpdatePushButtonSet();

  vtkTextProperty *TextProperty;
  vtkActor2D *Actor2D;

  int LongFormat;

  int ShowLabel;
  int LabelOnTop;
  vtkKWLabel *Label;

  int ShowColor;
  vtkKWChangeColorButton *ChangeColorButton;

  int ShowFontFamily;
  vtkKWMenuButtonLabeled *FontFamilyOptionMenu;

  int ShowStyles;
  vtkKWCheckButtonSetLabeled *StylesCheckButtonSet;
  
  int ShowOpacity;
  vtkKWScale *OpacityScale;

  char *ChangedCommand;
  char *ColorChangedCommand;

  int ShowCopy;
  vtkKWPushButtonSetLabeled *PushButtonSet;

private:
  vtkKWTextPropertyEditor(const vtkKWTextPropertyEditor&); // Not implemented
  void operator=(const vtkKWTextPropertyEditor&); // Not implemented
};

#endif

