/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWTextProperty.h,v $
  Language:  C++
  Date:      $Date: 2002-12-16 18:05:19 $
  Version:   $Revision: 1.8 $

Copyright (c) 2000-2001 Kitware Inc. 469 Clifton Corporate Parkway,
Clifton Park, NY, 12065, USA.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither the name of Kitware nor the names of any contributors may be used
   to endorse or promote products derived from this software without specific 
   prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkKWTextProperty - a GUI component that can be used to edit vtkTextProperty objects
// .SECTION Description
// The vtkKWTextProperty creates a set of GUI components that can be displayed
// and used selectively to edit all or part of a vtkTextProperty object.


#ifndef __vtkKWTextProperty_h
#define __vtkKWTextProperty_h

#include "vtkKWWidget.h"

class vtkActor2D;
class vtkKWApplication;
class vtkKWChangeColorButton;
class vtkKWLabel;
class vtkKWLabeledCheckButtonSet;
class vtkKWLabeledOptionMenu;
class vtkKWLabeledPushButtonSet;
class vtkKWPushButton;
class vtkKWScale;
class vtkTextProperty;

class VTK_EXPORT vtkKWTextProperty : public vtkKWWidget
{
public:
  static vtkKWTextProperty* New();
  vtkTypeRevisionMacro(vtkKWTextProperty,vtkKWWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create a Tk widget
  virtual void Create(vtkKWApplication *app);

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
  // Set the widget aspect to be long, i.e. the widgets will be packed on several
  // rows, with description labels. The default is short (all widgets on a row).
  void SetLongFormat(int);
  vtkBooleanMacro(LongFormat, int);
  vtkGetMacro(LongFormat, int);

  // Description:
  // Show the label on top (default0, otherwise on left. 
  // Valid if LongFormat is On.
  void SetLabelOnTop(int);
  vtkBooleanMacro(LabelOnTop, int);
  vtkGetMacro(LabelOnTop, int);

  // Description:
  // Show label.
  void SetShowLabel(int);
  vtkBooleanMacro(ShowLabel, int);
  vtkGetMacro(ShowLabel, int);
  vtkGetObjectMacro(Label, vtkKWLabel);

  // Description:
  // Show color.
  void SetShowColor(int);
  vtkBooleanMacro(ShowColor, int);
  vtkGetMacro(ShowColor, int);

  // Description:
  // Show font family.
  void SetShowFontFamily(int);
  vtkBooleanMacro(ShowFontFamily, int);
  vtkGetMacro(ShowFontFamily, int);

  // Description:
  // Show style.
  void SetShowStyles(int);
  vtkBooleanMacro(ShowStyles, int);
  vtkGetMacro(ShowStyles, int);

  // Description:
  // Show opacity.
  void SetShowOpacity(int);
  vtkBooleanMacro(ShowOpacity, int);
  vtkGetMacro(ShowOpacity, int);

  // Description:
  // Set/Get the command executed each time a change is made to the
  // text property.
  vtkSetStringMacro(OnChangeCommand);
  vtkGetStringMacro(OnChangeCommand);

  // Description:
  // Set/Get the command executed each time a change is made to the
  // color of the text property (the OnChangeCommand is run too).
  vtkSetStringMacro(OnColorChangeCommand);
  vtkGetStringMacro(OnColorChangeCommand);

  // Description:
  // Show copy button. This button can be used to synchronize different
  // text property widgets.
  void SetShowCopy(int);
  vtkBooleanMacro(ShowCopy, int);
  vtkGetMacro(ShowCopy, int);
  virtual vtkKWPushButton* GetCopyButton();

  // Description:
  // Copy the values from another text widget
  void CopyValuesFrom(vtkKWTextProperty*);

  // GUI components callbacks
  void ChangeColorButtonCallback(float, float, float);
  void SetColor(float, float, float);
  void SetColor(float *v) { this->SetColor(v[0], v[1], v[2]); };
  float* GetColor();
  void FontFamilyCallback();
  void SetFontFamily(int);
  void BoldCallback();
  void SetBold(int);
  void ItalicCallback();
  void SetItalic(int);
  void ShadowCallback();
  void SetShadow(int);
  void OpacityCallback();
  void OpacityEndCallback();
  void SetOpacity(float);
  void SetOpacityNoTrace(float);
  float GetOpacity();

  // Description:
  // Save out the text properties to a file.
  void SaveInTclScript(ofstream *file, const char *tcl_name = 0);

  // Description:
  // Set/Get the enabled state.
  // Override to pass down to children.
  virtual void SetEnabled(int);

protected:
  vtkKWTextProperty();
  ~vtkKWTextProperty();

  void Pack();

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
  vtkKWLabeledOptionMenu *FontFamilyOptionMenu;

  int ShowStyles;
  vtkKWLabeledCheckButtonSet *StylesCheckButtonSet;
  
  int ShowOpacity;
  vtkKWScale *OpacityScale;

  char *OnChangeCommand;
  char *OnColorChangeCommand;

  int ShowCopy;
  vtkKWLabeledPushButtonSet *PushButtonSet;

private:
  vtkKWTextProperty(const vtkKWTextProperty&); // Not implemented
  void operator=(const vtkKWTextProperty&); // Not implemented
};

#endif
