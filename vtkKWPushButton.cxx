/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWPushButton.cxx,v $
  Language:  C++
  Date:      $Date: 2000-08-16 04:10:54 $
  Version:   $Revision: 1.3 $

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
#include "vtkKWApplication.h"
#include "vtkKWPushButton.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkKWPushButton* vtkKWPushButton::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkKWPushButton");
  if(ret)
    {
    return (vtkKWPushButton*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkKWPushButton;
}



void vtkKWPushButton::Create(vtkKWApplication *app, const char *args)
{
  const char *wname;

  // must set the application
  if (this->Application)
    {
    vtkErrorMacro("PushButton already created");
    return;
    }

  this->SetApplication(app);

  // create the top level
  wname = this->GetWidgetName();
  this->Script("button %s %s", wname,args);
}

void vtkKWPushButton::SetLabel( const char *name )
{
  if ( this->Application )
    {
    this->Script("%s configure -text {%s}", this->GetWidgetName(), name );
    }
}
