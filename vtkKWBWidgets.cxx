/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWBWidgets.cxx,v $
  Language:  C++
  Date:      $Date: 2002-08-09 21:55:12 $
  Version:   $Revision: 1.8 $

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
#include "vtkKWBWidgets.h"

#include "vtkKWApplication.h"
#include "vtkObjectFactory.h"
#include "vtkbwidgets.h"
#include "vtkKWTkUtilities.h"

#include <tk.h>
 
//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWBWidgets );
vtkCxxRevisionMacro(vtkKWBWidgets, "$Revision: 1.8 $");

int vtkKWBWidgetsCommand(ClientData cd, Tcl_Interp *interp,
                            int argc, char *argv[]);

#define minus_width 9
#define minus_height 9
static unsigned char minus_bits[] = {
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  0,0,0,
  0,0,0,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  0,0,0,
  0,0,0,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  0,0,0,
  0,0,0,
  255,255,255,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  255,255,255,
  0,0,0,
  0,0,0,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  0,0,0,
  0,0,0,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  0,0,0,
  0,0,0,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0
};

#define plus_width 9
#define plus_height 9
static unsigned char plus_bits[] = {
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  0,0,0,
  0,0,0,
  255,255,255,
  255,255,255,
  255,255,255,
  0,0,0,
  255,255,255,
  255,255,255,
  255,255,255,
  0,0,0,
  0,0,0,
  255,255,255,
  255,255,255,
  255,255,255,
  0,0,0,
  255,255,255,
  255,255,255,
  255,255,255,
  0,0,0,
  0,0,0,
  255,255,255,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  255,255,255,
  0,0,0,
  0,0,0,
  255,255,255,
  255,255,255,
  255,255,255,
  0,0,0,
  255,255,255,
  255,255,255,
  255,255,255,
  0,0,0,
  0,0,0,
  255,255,255,
  255,255,255,
  255,255,255,
  0,0,0,
  255,255,255,
  255,255,255,
  255,255,255,
  0,0,0,
  0,0,0,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  255,255,255,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0,
  0,0,0
};


//----------------------------------------------------------------------------
vtkKWBWidgets::vtkKWBWidgets()
{
  this->CommandFunction = vtkKWBWidgetsCommand;
}

//----------------------------------------------------------------------------
vtkKWBWidgets::~vtkKWBWidgets()
{
}

//----------------------------------------------------------------------------
int vtkKWBWidgets::CreatePhoto(Tcl_Interp* interp, char *name, 
                                unsigned char *data, int width, int height)
{
  ostrstream command;
  command << "image create photo " << name << " -height "
          << height << " -width " << width << ends;
  if (Tcl_GlobalEval(interp, command.str()) != TCL_OK)
    {
    vtkGenericWarningMacro(<< "Unable to create image: " << interp->result);
    command.rdbuf()->freeze(0);     
    return VTK_ERROR;
    }
  command.rdbuf()->freeze(0);     

  if (!vtkKWTkUtilities::UpdatePhoto(interp, name, data, width, height, 3))
    {
    vtkGenericWarningMacro(<< "Error updating Tk photo " << name);
    }
  
  return VTK_OK;

}

//----------------------------------------------------------------------------
void vtkKWBWidgets::Initialize(Tcl_Interp* interp)
{
  if (!interp)
    {
    vtkGenericWarningMacro("An interpreter is needed to initialize bwidgets.");
    return;
    }

  if ( CreatePhoto(interp, "bwminus", minus_bits,  minus_width, minus_height) 
       != VTK_OK )
    {
    return;
    }

  if ( CreatePhoto(interp, "bwplus", plus_bits,  plus_width, plus_height) 
       != VTK_OK )
    {
    return;
    }

  char* script = new char[strlen(bwidgets1)+1];
  strcpy(script, bwidgets1);
  if (Tcl_GlobalEval(interp, script) != TCL_OK)
    {
    vtkGenericWarningMacro(<< "BWidgets failed to initialize. Error:" 
    << interp->result);
    }
  delete[] script;
  

  script = new char[strlen(bwidgets2)+1];
  strcpy(script, bwidgets2);
  if (Tcl_GlobalEval(interp, script) != TCL_OK)
    {
    vtkGenericWarningMacro(<< "BWidgets failed to initialize. Error:" 
    << interp->result);
    }
  delete[] script;

  script = new char[strlen(bwidgets3)+1];
  strcpy(script, bwidgets3);
  if (Tcl_GlobalEval(interp, script) != TCL_OK)
    {
    vtkGenericWarningMacro(<< "BWidgets failed to initialize. Error:" 
    << interp->result);
    }
  delete[] script;

  script = new char[strlen(bwidgets4)+1];
  strcpy(script, bwidgets4);
  if (Tcl_GlobalEval(interp, script) != TCL_OK)
    {
    vtkGenericWarningMacro(<< "BWidgets failed to initialize. Error:" 
    << interp->result);
    }
  delete[] script;
}

//----------------------------------------------------------------------------
void vtkKWBWidgets::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
