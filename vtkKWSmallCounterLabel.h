/*=========================================================================

  Module:    vtkKWSmallCounterLabel.h,v

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWSmallCounterLabel - a (very) small counter label (icon)
// .SECTION Description
// A widget that represents a very small counter (as an icon), counting
// from 1 to 9 (any value over 9 is represented as 9+).

#ifndef __vtkKWSmallCounterLabel_h
#define __vtkKWSmallCounterLabel_h

#include "vtkKWLabel.h"

class KWWidgets_EXPORT vtkKWSmallCounterLabel : public vtkKWLabel
{
public:
  static vtkKWSmallCounterLabel* New();
  vtkTypeRevisionMacro(vtkKWSmallCounterLabel,vtkKWLabel);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the value of the counter.
  // Set it to 0 to hide the counter. Any value over 9 will be stored as
  // 10 and represented as a 9+ icon.
  virtual void SetValue(unsigned int v);
  vtkGetMacro(Value, unsigned int);

  // Description:
  // Place the icon on top of its parent
  virtual void Place();
 

protected:
  vtkKWSmallCounterLabel();
  ~vtkKWSmallCounterLabel();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Description:
  // Update the icon.
  virtual void UpdateIcon();

  unsigned int Value;
  vtkKWWidget *OriginalParent;

private:
  vtkKWSmallCounterLabel(const vtkKWSmallCounterLabel&); // Not implemented
  void operator=(const vtkKWSmallCounterLabel&); // Not implemented
};


#endif
