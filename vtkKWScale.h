/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWScale.h,v $
  Language:  C++
  Date:      $Date: 1999-12-29 23:22:09 $
  Version:   $Revision: 1.1 $

Copyright (c) 1998-1999 Kitware Inc. 469 Clifton Corporate Parkway,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
// .NAME vtkKWScale
// .SECTION Description
// A widget that repsentes a scale (or slider) with options for 
// a label string and a text entry box.

#ifndef __vtkKWScale_h
#define __vtkKWScale_h

#include "vtkKWEntry.h"
class vtkKWApplication;

class VTK_EXPORT vtkKWScale : public vtkKWWidget
{
public:
  vtkKWScale();
  ~vtkKWScale();
  static vtkKWScale* New();
  const char *GetClassName() {return "vtkKWScale";};

  // Description:
  // Create a Tk widget
  void Create(vtkKWApplication *app, char *args);

  // Description:
  // Set/Get the value of the scale
  void SetValue(float v);
  virtual float GetValue() {return this->Value;};

  /// Description:
  // Set the range for this scale.
  void SetRange(float min, float max);

  // Description:
  // Display a label and or a text entry box. These are options. 
  void DisplayEntry();  
  void DisplayLabel(const char *l);  

  // Description:
  // Method that gets invoked when the sliders value has changed.
  virtual void ScaleValueChanged(float num);
  virtual void EntryValueChanged();
  virtual void InvokeStartCommand();
  virtual void InvokeEndCommand();
  
//BTX
  // Description:
  // A convienience method to invoke some tcl script code and
  // perform arguement substitution.
  virtual void SetCommand(char *EventString, ...);
  virtual void SetStartCommand(char *EventString, ...);
  virtual void SetEndCommand(char *EventString, ...);
//ETX

protected:
  char        *Command;
  char        *StartCommand;
  char        *EndCommand;
  float       Value;
  vtkKWEntry  *Entry;
  vtkKWWidget *ScaleWidget;
  vtkKWWidget *ScaleLabel;
};


#endif


