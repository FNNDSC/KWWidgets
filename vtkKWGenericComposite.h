/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWGenericComposite.h,v $
  Language:  C++
  Date:      $Date: 2000-01-18 19:59:35 $
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
// .NAME vtkKWGenericComposite
// .SECTION Description
// vtkKWGenericComposite is a subclass of vtkKWComposite designed to 
// handle images. Its property sheet includes support for probing the image
// scaling it, adjusting the window and level settings.

#ifndef __vtkKWGenericComposite_h
#define __vtkKWGenericComposite_h


#include "vtkKWComposite.h"
#include "vtkProp.h"

class vtkKWApplication;
class vtkKWView;
class vtkStructuredPoints;

class VTK_EXPORT vtkKWGenericComposite : public vtkKWComposite
{
public:
  static vtkKWGenericComposite* New();
  vtkTypeMacro(vtkKWGenericComposite,vtkKWComposite);

  // Description:
  // Get the prop for this composite
  virtual vtkProp *GetProp() {return this->Prop;};
  vtkSetObjectMacro(Prop,vtkProp);
  
protected:
  vtkKWGenericComposite();
  ~vtkKWGenericComposite();
  vtkKWGenericComposite(const vtkKWGenericComposite&) {};
  void operator=(const vtkKWGenericComposite&) {};

  vtkProp *Prop;
};


#endif


