/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWExtent.cxx,v $
  Language:  C++
  Date:      $Date: 2003-01-13 04:49:43 $
  Version:   $Revision: 1.17 $

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
#include "vtkKWExtent.h"

#include "vtkKWApplication.h"
#include "vtkKWScale.h"
#include "vtkObjectFactory.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWExtent );
vtkCxxRevisionMacro(vtkKWExtent, "$Revision: 1.17 $");

//----------------------------------------------------------------------------
int vtkKWExtentCommand(ClientData cd, Tcl_Interp *interp,
                       int argc, char *argv[]);

//----------------------------------------------------------------------------
vtkKWExtent::vtkKWExtent()
{
  this->CommandFunction = vtkKWExtentCommand;
  this->Command = NULL;

  this->XMinScale = vtkKWScale::New();
  this->XMinScale->SetParent(this);
  this->XMaxScale = vtkKWScale::New();
  this->XMaxScale->SetParent(this);
  this->YMinScale = vtkKWScale::New();
  this->YMinScale->SetParent(this);
  this->YMaxScale = vtkKWScale::New();
  this->YMaxScale->SetParent(this);
  this->ZMinScale = vtkKWScale::New();
  this->ZMinScale->SetParent(this);
  this->ZMaxScale = vtkKWScale::New();
  this->ZMaxScale->SetParent(this);

  this->Extent[0] = 0;
  this->Extent[1] = 0;
  this->Extent[2] = 0;
  this->Extent[3] = 0;
  this->Extent[4] = 0;
  this->Extent[5] = 0;
}

//----------------------------------------------------------------------------
vtkKWExtent::~vtkKWExtent()
{
  if (this->Command)
    {
    delete [] this->Command;
    }

  this->XMinScale->Delete();  
  this->XMaxScale->Delete();
  this->YMinScale->Delete();  
  this->YMaxScale->Delete();
  this->ZMinScale->Delete();  
  this->ZMaxScale->Delete();
}

//----------------------------------------------------------------------------
void vtkKWExtent::Create(vtkKWApplication *app, const char *args)
{
  const char *wname;

  // Set the application

  if (this->IsCreated())
    {
    vtkErrorMacro("Extent already created");
    return;
    }

  this->SetApplication(app);

  // create the top level
  wname = this->GetWidgetName();
  this->Script("frame %s -bd 0 %s", wname, (args ? args : ""));

  this->XMinScale->Create(this->Application,"-length 190");
  this->XMinScale->SetCommand(this, "ExtentSelected");
  this->XMinScale->DisplayEntry();
  this->XMinScale->DisplayLabel("Minimum X (Units)");

  this->XMaxScale->Create(this->Application,"");
  this->XMaxScale->SetCommand(this, "ExtentSelected");
  this->XMaxScale->DisplayEntry();
  this->XMaxScale->DisplayLabel("Maximum X (Units)");

  this->YMinScale->Create(this->Application,"");
  this->YMinScale->SetCommand(this, "ExtentSelected");
  this->YMinScale->DisplayEntry();
  this->YMinScale->DisplayLabel("Minimum Y (Units)");

  this->YMaxScale->Create(this->Application,"");
  this->YMaxScale->SetCommand(this, "ExtentSelected");
  this->YMaxScale->DisplayEntry();
  this->YMaxScale->DisplayLabel("Maximum Y (Units)");

  this->ZMinScale->Create(this->Application,"");
  this->ZMinScale->SetCommand(this, "ExtentSelected");
  this->ZMinScale->DisplayEntry();
  this->ZMinScale->DisplayLabel("Minimum Z (Units)");

  this->ZMaxScale->Create(this->Application,"");
  this->ZMaxScale->SetCommand(this, "ExtentSelected");
  this->ZMaxScale->DisplayEntry();
  this->ZMaxScale->DisplayLabel("Maximum Z (Units)");

  this->Script(
    "pack %s %s %s %s %s %s -padx 2 -pady 2 -fill x -expand yes -anchor w",
    this->XMinScale->GetWidgetName(),
    this->XMaxScale->GetWidgetName(),
    this->YMinScale->GetWidgetName(),
    this->YMaxScale->GetWidgetName(),
    this->ZMinScale->GetWidgetName(),
    this->ZMaxScale->GetWidgetName());

  // Update enable state

  this->UpdateEnableState();
}

//----------------------------------------------------------------------------
void vtkKWExtent::SetExtentRange(float *er)
{
  this->SetExtentRange(er[0],er[1],er[2],er[3],er[4],er[5]);
}

//----------------------------------------------------------------------------
void vtkKWExtent::SetExtentRange(float x1, float x2, float y1, float y2, 
                                 float z1, float z2)
{
  this->XMinScale->SetResolution((x2<x1)?((x1-x2)/1024.0):((x2-x1)/1024.0));
  this->XMaxScale->SetResolution((x2<x1)?((x1-x2)/1024.0):((x2-x1)/1024.0));
  this->YMinScale->SetResolution((y2<y1)?((y1-y2)/1024.0):((y2-y1)/1024.0));
  this->YMaxScale->SetResolution((y2<y1)?((y1-y2)/1024.0):((y2-y1)/1024.0));
  this->ZMinScale->SetResolution((y2<y1)?((y1-y2)/1024.0):((y2-y1)/1024.0));
  this->ZMaxScale->SetResolution((y2<y1)?((y1-y2)/1024.0):((y2-y1)/1024.0));
  
  this->XMinScale->SetRange(x1,x2);
  this->XMaxScale->SetRange(x1,x2);
  this->YMinScale->SetRange(y1,y2);
  this->YMaxScale->SetRange(y1,y2);
  this->ZMinScale->SetRange(z1,z2);
  this->ZMaxScale->SetRange(z1,z2);

  float ex1, ex2, ey1, ey2, ez1, ez2;

  ex1 = (this->Extent[0] < x1 || this->Extent[0] > x2) ? x1 : this->Extent[0];
  ex2 = (this->Extent[1] < x1 || this->Extent[1] > x2) ? x2 : this->Extent[1];
  ey1 = (this->Extent[2] < y1 || this->Extent[2] > y2) ? y1 : this->Extent[2];
  ey2 = (this->Extent[3] < y1 || this->Extent[3] > y2) ? y2 : this->Extent[3];
  ez1 = (this->Extent[4] < z1 || this->Extent[4] > z2) ? z1 : this->Extent[4];
  ez2 = (this->Extent[5] < z1 || this->Extent[5] > z2) ? z2 : this->Extent[5];

  this->SetExtent(ex1, ex2, ey1, ey2, ez1, ez2);
}

//----------------------------------------------------------------------------
void vtkKWExtent::SetExtent(float x1, float x2, float y1, float y2, float z1, float z2)
{
  if (this->Extent[0] == x1 &&
      this->Extent[1] == x2 &&
      this->Extent[2] == y1 &&
      this->Extent[3] == y2 &&
      this->Extent[4] == z1 &&
      this->Extent[5] == z2)
    {
    return;
    }

  this->XMinScale->SetValue(x1);
  this->XMaxScale->SetValue(x2);
  this->YMinScale->SetValue(y1);
  this->YMaxScale->SetValue(y2);
  this->ZMinScale->SetValue(z1);
  this->ZMaxScale->SetValue(z2);
}

//----------------------------------------------------------------------------
void vtkKWExtent::SetExtent(float *er)
{
  this->SetExtent(er[0],er[1],er[2],er[3],er[4],er[5]);
}

//----------------------------------------------------------------------------
void vtkKWExtent::ExtentSelected()
{
  // first check to see if anything changed.
  // Normally something should have changed, but 
  // on initialization this isn;t the case.
  if (this->Extent[0] == this->XMinScale->GetValue() &&
      this->Extent[1] == this->XMaxScale->GetValue() &&
      this->Extent[2] == this->YMinScale->GetValue() &&
      this->Extent[3] == this->YMaxScale->GetValue() &&
      this->Extent[4] == this->ZMinScale->GetValue() &&
      this->Extent[5] == this->ZMaxScale->GetValue())
    {
    return;
    }
  
  this->Extent[0] = this->XMinScale->GetValue();
  this->Extent[1] = this->XMaxScale->GetValue();
  this->Extent[2] = this->YMinScale->GetValue();
  this->Extent[3] = this->YMaxScale->GetValue();
  this->Extent[4] = this->ZMinScale->GetValue();
  this->Extent[5] = this->ZMaxScale->GetValue();
 
  // handle error conditions
  if (this->Extent[0] > this->Extent[1])
    {
    this->Extent[0] = this->Extent[1];
    this->XMinScale->SetValue(this->Extent[0]);    
    }
  if (this->Extent[2] > this->Extent[3])
    {
    this->Extent[2] = this->Extent[3];
    this->YMinScale->SetValue(this->Extent[2]);    
    }
  if (this->Extent[4] > this->Extent[5])
    {
    this->Extent[4] = this->Extent[5];
    this->ZMinScale->SetValue(this->Extent[4]);    
    }
  if ( this->Command )
    {
    this->Script("eval %s",this->Command);
    }
}

//----------------------------------------------------------------------------
void vtkKWExtent::SetCommand(vtkKWObject* CalledObject, const char *CommandString)
{ 
  if (this->Command)
    {
    delete [] this->Command;
    }
  ostrstream command;
  command << CalledObject->GetTclName() << " " << CommandString << ends;

  this->Command = command.str();
}

//----------------------------------------------------------------------------
void vtkKWExtent::SetStartCommand(vtkKWObject* Object, 
                                  const char *MethodAndArgString)
{ 
  if (this->XMinScale)
    {
    this->XMinScale->SetStartCommand(Object, MethodAndArgString);
    }

  if (this->XMaxScale)
    {
    this->XMaxScale->SetStartCommand(Object, MethodAndArgString);
    }

  if (this->YMinScale)
    {
    this->YMinScale->SetStartCommand(Object, MethodAndArgString);
    }

  if (this->YMaxScale)
    {
    this->YMaxScale->SetStartCommand(Object, MethodAndArgString);
    }

  if (this->ZMinScale)
    {
    this->ZMinScale->SetStartCommand(Object, MethodAndArgString);
    }

  if (this->ZMaxScale)
    {
    this->ZMaxScale->SetStartCommand(Object, MethodAndArgString);
    }
}

//----------------------------------------------------------------------------
void vtkKWExtent::SetEndCommand(vtkKWObject* Object, 
                                const char *MethodAndArgString)
{ 
  if (this->XMinScale)
    {
    this->XMinScale->SetEndCommand(Object, MethodAndArgString);
    }

  if (this->XMaxScale)
    {
    this->XMaxScale->SetEndCommand(Object, MethodAndArgString);
    }

  if (this->YMinScale)
    {
    this->YMinScale->SetEndCommand(Object, MethodAndArgString);
    }

  if (this->YMaxScale)
    {
    this->YMaxScale->SetEndCommand(Object, MethodAndArgString);
    }

  if (this->ZMinScale)
    {
    this->ZMinScale->SetEndCommand(Object, MethodAndArgString);
    }

  if (this->ZMaxScale)
    {
    this->ZMaxScale->SetEndCommand(Object, MethodAndArgString);
    }
}

//----------------------------------------------------------------------------
void vtkKWExtent::SetDisableCommands(int v)
{ 
  if (this->XMinScale)
    {
    this->XMinScale->SetDisableCommands(v);
    }

  if (this->XMaxScale)
    {
    this->XMaxScale->SetDisableCommands(v);
    }

  if (this->YMinScale)
    {
    this->YMinScale->SetDisableCommands(v);
    }

  if (this->YMaxScale)
    {
    this->YMaxScale->SetDisableCommands(v);
    }

  if (this->ZMinScale)
    {
    this->ZMinScale->SetDisableCommands(v);
    }

  if (this->ZMaxScale)
    {
    this->ZMaxScale->SetDisableCommands(v);
    }
}

// ---------------------------------------------------------------------------
void vtkKWExtent::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  if (this->XMinScale)
    {
    this->XMinScale->SetEnabled(this->Enabled);
    }

  if (this->XMaxScale)
    {
    this->XMaxScale->SetEnabled(this->Enabled);
    }

  if (this->YMinScale)
    {
    this->YMinScale->SetEnabled(this->Enabled);
    }

  if (this->YMaxScale)
    {
    this->YMaxScale->SetEnabled(this->Enabled);
    }

  if (this->ZMinScale)
    {
    this->ZMinScale->SetEnabled(this->Enabled);
    }

  if (this->ZMaxScale)
    {
    this->ZMaxScale->SetEnabled(this->Enabled);
    }
}

//----------------------------------------------------------------------------
void vtkKWExtent::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Extent: " << this->GetExtent() << endl;
}
