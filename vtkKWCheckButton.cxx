/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWCheckButton.cxx,v $
  Language:  C++
  Date:      $Date: 2002-08-29 19:49:16 $
  Version:   $Revision: 1.13 $

Copyright (c) 2000-2001 Kitware Inc. 469 Clifton Corporate Parkway,
Clifton Park, NY, 12065, USA.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither the name of Kitware nor the names of any contributors may be used
   to endorse or promote products derived from this software without specific 
   prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "vtkKWApplication.h"
#include "vtkKWCheckButton.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWCheckButton );
vtkCxxRevisionMacro(vtkKWCheckButton, "$Revision: 1.13 $");


vtkKWCheckButton::vtkKWCheckButton() 
{
  this->IndicatorOn = 1;
  this->MyText = 0;
}

vtkKWCheckButton::~vtkKWCheckButton() 
{
  this->SetMyText(0);
}

void vtkKWCheckButton::SetIndicator(int ind)
{
  if (ind != this->IndicatorOn)
    {
    this->IndicatorOn = ind;
    if (this->Application)
      {
      if (ind)
        {
        this->Script("%s configure -indicatoron 1", this->GetWidgetName());
        }
      else
        {
        this->Script("%s configure -indicatoron 0", this->GetWidgetName());
        }
      }
    }
  this->SetMyText(0);
}

void vtkKWCheckButton::SetText(const char* txt)
{
  this->SetMyText(txt);
  
  if (this->Application)
    {
    if (this->MyText)
      {
      this->Script("%s configure -text {%s}", this->GetWidgetName(), this->MyText);
      }
    }
}

const char* vtkKWCheckButton::GetText()
{
  return this->MyText;
}

int vtkKWCheckButton::GetState()
{
  if ( this->Application )
    {
    this->Script("set %sValue",this->GetWidgetName());
    
    return vtkKWObject::GetIntegerResult(this->Application);
    }
  return 0;
}

void vtkKWCheckButton::SetState(int s)
{
  if ( this->Application )
    {
    if (s)
      {
      this->Script("%s select",this->GetWidgetName());
      }
    else
      {
      this->Script("%s deselect",this->GetWidgetName());
      }
    }
}


void vtkKWCheckButton::Create(vtkKWApplication *app, const char *args)
{
  const char *wname;

  // must set the application
  if (this->Application)
    {
    vtkErrorMacro("CheckButton already created");
    return;
    }

  this->SetApplication(app);

  // create the top level
  wname = this->GetWidgetName();
  if (!this->IndicatorOn)
    {
    this->Script("checkbutton %s -indicatoron 0 -variable %sValue %s",
                 wname,wname,(args?args:""));
    }
  else
    {
    this->Script("checkbutton %s -variable %sValue %s",
                 wname,wname,(args?args:""));
    }

  if (this->MyText)
    {
    this->Script("%s configure -text %s", this->GetWidgetName(), this->MyText);
    }

}


//----------------------------------------------------------------------------
void vtkKWCheckButton::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
