/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWOptionMenu.h,v $
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
// .NAME vtkKWOptionMenu
// .SECTION Description
// A widget that looks like a button but when pressed provides a list
// of options that the user can select.

#ifndef __vtkKWOptionMenu_h
#define __vtkKWOptionMenu_h

#include "vtkKWWidget.h"
class vtkKWApplication;

class VTK_EXPORT vtkKWOptionMenu : public vtkKWWidget
{
public:
  vtkKWOptionMenu();
  ~vtkKWOptionMenu();
  static vtkKWOptionMenu* New();
  const char *GetClassName() {return "vtkKWOptionMenu";};

  // Description:
  // Create a Tk widget
  virtual void Create(vtkKWApplication *app, char *args);

  // Description:
  // Set/Get the state of the option menu
  void SetValue(char *);
  char *GetValue();

  // Description:
  // Add entries to an option menu, with or without a command.
  void AddEntry(const char *name);
  void AddEntryWithCommand(const char *name, const char *obj, const char *method);

  // Description:
  // Set the current entry of this optionmenu
  void SetCurrentEntry(const char *name);

protected:
  char *CurrentValue;  
  vtkKWWidget *Menu;
};


#endif


