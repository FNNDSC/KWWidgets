/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWObject.h,v $
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
// .NAME vtkKWObject
// .SECTION Description
// vtkKWObject is the superclass for most application classes.
// It is a direct subclass of vtkObject but adds functionality for 
// invoking Tcl scripts, obtains results from those scripts, and
// obtaining a Tcl name for an instance. This class requires a 
// vtkKWApplicaiton in order to work (as do all classes).

// .SECTION See Also
// vtkKWApplication

#ifndef __vtkKWObject_h
#define __vtkKWObject_h

#include "vtkKWApplication.h"
#include "vtkKWSerializer.h"

// var args
#ifndef _WIN32
#include <unistd.h>
#endif
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "tcl.h"

class VTK_EXPORT vtkKWObject : public vtkObject
{
public:
  vtkKWObject();
  ~vtkKWObject();
  static vtkKWObject* New();
  const char *GetClassName() {return "vtkKWObject";};

  // Description:
  // Get the name of the tcl object this instance represents.
  const char *GetTclName();

  // Description:
  // Get the application instance for this class.
  vtkGetObjectMacro(Application,vtkKWApplication);
  vtkSetObjectMacro(Application,vtkKWApplication);

  // Description:
  // Convienience methods to get results of Tcl commands.
  static int GetIntegerResult(vtkKWApplication *);
  static float GetFloatResult(vtkKWApplication *);


  // Description:
  // Chaining method to serialize an object and its superclasses.
  void Serialize(ostream& os, vtkIndent indent);
  void Serialize(istream& is);
  virtual void SerializeSelf(ostream& os, vtkIndent indent) {};
  virtual void SerializeToken(istream& is, const char token[1024]) {};


//BTX
  // Description:
  // A convienience method to invoke some tcl script code and
  // perform arguement substitution.
  static void Script(vtkKWApplication* ,char *EventString, ...);


private:
  char *TclName;

protected:
  vtkKWApplication *Application;
  // this instance variable holds the command functions for this class.
  int (*CommandFunction)(ClientData, Tcl_Interp *,
                         int, char *[]);
//ETX
};


#endif


