/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWLabel.cxx,v $
  Language:  C++
  Date:      $Date: 2000-08-07 20:15:27 $
  Version:   $Revision: 1.5 $

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
#include "vtkKWLabel.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkKWLabel* vtkKWLabel::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkKWLabel");
  if(ret)
    {
    return (vtkKWLabel*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkKWLabel;
}

vtkKWLabel::vtkKWLabel()
{
  this->Label = new char[1];
  this->Label[0] = 0;
}

vtkKWLabel::~vtkKWLabel()
{
  delete [] this->Label;
}

void vtkKWLabel::SetLabel(const char* l)
{
  if(!l)
    {
    l = "";
    }
  
  delete [] this->Label;
  this->Label = strcpy(new char[strlen(l)+1], l);
  if(this->Application)
    {
    // if this has been created then change the text
    this->Script("%s configure -text {%s}", this->GetWidgetName(), 
		 this->Label);
    }
}


void vtkKWLabel::Create(vtkKWApplication *app, const char *args)
{
  const char *wname;

  // must set the application
  if (this->Application)
    {
    vtkErrorMacro("Label already created");
    return;
    }

  this->SetApplication(app);

  // create the top level
  wname = this->GetWidgetName();
  this->Script("label %s -text {%s} %s", wname, this->Label, args);
}

