/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWEventNotifier.h,v $
  Language:  C++
  Date:      $Date: 2000-06-28 15:43:03 $
  Version:   $Revision: 1.2 $

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
// .NAME vtkKWEventNotifier - collect callbacks and invoke them when certain events occur
// .SECTION Description

#ifndef __vtkKWEventNotifier_h
#define __vtkKWEventNotifier_h

#include "vtkKWObject.h"
#include "vtkKWCallbackSpecification.h"


class VTK_EXPORT vtkKWEventNotifier : public vtkKWObject
{
public:
  static vtkKWEventNotifier* New();
  vtkTypeMacro(vtkKWEventNotifier,vtkKWObject);

  void AddCallback( const char *event,   vtkKWWindow *window,
		    vtkKWObject *object, const char *command );

  void RemoveCallback( const char *event,   vtkKWWindow *window,
		       vtkKWObject *object, const char *command );

  // This version invokes all callbacks of the specified type for
  // the specified window - even its own callback if it has one
  void InvokeCallbacks( const char *event, vtkKWWindow *window,
			const char *args );

  // This version won't invoke callbacks on the specified object 
  // Usually the calling object uses this to avoid calling itself
  void InvokeCallbacks( vtkKWObject *object, const char *event, 
                        vtkKWWindow *window, const char *args );

protected:
  vtkKWEventNotifier();
  ~vtkKWEventNotifier();
  vtkKWEventNotifier(const vtkKWEventNotifier&) {};
  void operator=(const vtkKWEventNotifier&) {};

  vtkKWCallbackSpecification **Callbacks;

  int ComputeIndex( const char *event );
};


#endif


