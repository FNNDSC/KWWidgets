/*=========================================================================

  Module:    $RCSfile: vtkKWBWidgets.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWBWidgets - class used to initialized bwidgets
// .SECTION Description
// This class is used to initialize the bwidgets used by
// KW widgets.


#ifndef __vtkKWBWidgets_h
#define __vtkKWBWidgets_h

#include "vtkKWObject.h"

class VTK_EXPORT vtkKWBWidgets : public vtkKWObject
{
public:
  static vtkKWBWidgets* New();
  vtkTypeRevisionMacro(vtkKWBWidgets,vtkKWObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Load the bwidgets.
  static void Initialize(Tcl_Interp*);

  // Description:
  // Create a photo from an array. Used to initialized
  // some icons used in the Tree widget.
  static int CreatePhoto(Tcl_Interp*, char *name, 
                         unsigned char *data, int width, int height);

protected:
  vtkKWBWidgets();
  ~vtkKWBWidgets();

  static void Execute(Tcl_Interp*, const char* str, const char*);

private:
  vtkKWBWidgets(const vtkKWBWidgets&);   // Not implemented.
  void operator=(const vtkKWBWidgets&);  // Not implemented.
};

#endif








