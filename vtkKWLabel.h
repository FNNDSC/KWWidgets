/*=========================================================================

  Module:    $RCSfile: vtkKWLabel.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWLabel - label widget
// .SECTION Description
// A simple widget that represents a label. The label can be set with 
// the SetText method.

#ifndef __vtkKWLabel_h
#define __vtkKWLabel_h

#include "vtkKWWidget.h"
class vtkKWApplication;

class KWWIDGETS_EXPORT vtkKWLabel : public vtkKWWidget
{
public:
  static vtkKWLabel* New();
  vtkTypeRevisionMacro(vtkKWLabel,vtkKWWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create the widget.
  virtual void Create(vtkKWApplication *app);
  
  // Description:
  // Set the text on the label.
  virtual void SetText(const char*);
  vtkGetStringMacro(Text);

  // Description:
  // Set the way label treats long text. 
  // Multiline will wrap text. You have to specify width
  // when using multiline label.
  //BTX
  enum 
  {
    SingleLine,
    MultiLine
  };
  //ETX
  virtual void SetLineType(int type);

  // Description:
  // Set/Get width of the label.
  virtual void SetWidth(int);
  vtkGetMacro(Width, int);

  // Description:
  // Set/Get the justification mode.
  // When there are multiple lines of text displayed in a widget, this option
  // determines how the lines line up with each other.
  // Valid constants can be found in vtkKWTkOptions::JustificationType.
  virtual void SetJustification(int);
  virtual int GetJustification();
  virtual void SetJustificationToLeft() 
    { this->SetJustification(vtkKWTkOptions::JustificationLeft); };
  virtual void SetJustificationToCenter() 
    { this->SetJustification(vtkKWTkOptions::JustificationCenter); };
  virtual void SetJustificationToRight() 
    { this->SetJustification(vtkKWTkOptions::JustificationRight); };

  // Description:
  // Set/Get the wrap length mode.
  // For widgets that can perform word-wrapping, this option specifies the
  // maximum line length. Lines that would exceed this length are wrapped onto
  // the next line, so that no line is longer than the specified length. 
  // The value may be specified in any of the standard forms for screen
  // distances.
  virtual void SetWrapLength(const char *length);
  virtual const char* GetWrapLength();
  
  // Description:
  // Adjust the -wraplength argument so that it matches the width of
  // the widget automatically (through the <Configure> event).
  virtual void SetAdjustWrapLengthToWidth(int);
  vtkGetMacro(AdjustWrapLengthToWidth, int);
  vtkBooleanMacro(AdjustWrapLengthToWidth, int);
  virtual void AdjustWrapLengthToWidthCallback();

  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object is updated
  // and propagated to its internal parts/subwidgets. This will, for example,
  // enable/disable parts of the widget UI, enable/disable the visibility
  // of 3D widgets, etc.
  virtual void UpdateEnableState();

protected:
  vtkKWLabel();
  ~vtkKWLabel();

  virtual void UpdateBindings();
  virtual void UpdateText();

private:
  char* Text;
  int LineType;
  int Width;
  int AdjustWrapLengthToWidth;

  vtkKWLabel(const vtkKWLabel&); // Not implemented
  void operator=(const vtkKWLabel&); // Not implemented
};


#endif



