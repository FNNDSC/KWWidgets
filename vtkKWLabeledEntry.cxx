/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWLabeledEntry.cxx,v $
  Language:  C++
  Date:      $Date: 2002-12-04 23:28:20 $
  Version:   $Revision: 1.9 $

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
//#include "vtkKWApplication.h"
#include "vtkKWLabeledEntry.h"
#include "vtkObjectFactory.h"
#include "vtkKWLabel.h"
#include "vtkKWEntry.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWLabeledEntry );
vtkCxxRevisionMacro(vtkKWLabeledEntry, "$Revision: 1.9 $");

int vtkKWLabeledEntryCommand(ClientData cd, Tcl_Interp *interp,
                             int argc, char *argv[]);

//----------------------------------------------------------------------------
vtkKWLabeledEntry::vtkKWLabeledEntry()
{
  this->CommandFunction = vtkKWLabeledEntryCommand;

  this->Label = vtkKWLabel::New();
  this->Entry = vtkKWEntry::New();
}

//----------------------------------------------------------------------------
vtkKWLabeledEntry::~vtkKWLabeledEntry()
{
  this->Label->Delete();
  this->Label = NULL;

  this->Entry->Delete();
  this->Entry = NULL;
}

//----------------------------------------------------------------------------
void vtkKWLabeledEntry::SetLabel(const char *text)
{
  this->Script("%s configure -text {%s}",
               this->Label->GetWidgetName(), text);  
}

//----------------------------------------------------------------------------
void vtkKWLabeledEntry::Create(vtkKWApplication *app)
{
  if (this->IsCreated())
    {
    vtkErrorMacro("LabeledEntry already created");
    return;
    }

  this->SetApplication(app);

  // Create the top level

  const char *wname = this->GetWidgetName();
  this->Script("frame %s -borderwidth 0 -relief flat", wname);

  this->Label->SetParent(this);
  this->Label->Create(app, "");

  this->Entry->SetParent(this);
  this->Entry->Create(app, "");

  // Usually you will want the entry to expand itself

  this->Script("pack %s -side left", 
               this->Label->GetWidgetName());

  this->Script("pack %s -side left -fill x -expand t", 
               this->Entry->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkKWLabeledEntry::SetValue(const char *value)
{
  this->Entry->SetValue(value);
}

//----------------------------------------------------------------------------
void vtkKWLabeledEntry::SetValue(int a)
{
  this->Entry->SetValue(a);
}

//----------------------------------------------------------------------------
void vtkKWLabeledEntry::SetValue(float f,int size)
{
  this->Entry->SetValue(f, size);
}

//----------------------------------------------------------------------------
char *vtkKWLabeledEntry::GetValue()
{
  return this->Entry->GetValue();
}

//----------------------------------------------------------------------------
int vtkKWLabeledEntry::GetValueAsInt()
{
  return this->Entry->GetValueAsInt();
}

//----------------------------------------------------------------------------
float vtkKWLabeledEntry::GetValueAsFloat()
{
  return this->Entry->GetValueAsFloat();
}

//----------------------------------------------------------------------------
void vtkKWLabeledEntry::SetEnabled(int e)
{
  // Propagate first (since objects can be modified externally, they might
  // not be in synch with this->Enabled)

  if (this->IsCreated())
    {
    this->Entry->SetEnabled(e);
    this->Label->SetEnabled(e);
    }

  // Then update internal Enabled ivar, although it is not of much use here

  if (this->Enabled == e)
    {
    return;
    }

  this->Enabled = e;
  this->Modified();
}

// ---------------------------------------------------------------------------
void vtkKWLabeledEntry::SetBalloonHelpString(const char *string)
{
  if (this->Label)
    {
    this->Label->SetBalloonHelpString(string);
    }

  if (this->Entry)
    {
    this->Entry->SetBalloonHelpString(string);
    }
}

// ---------------------------------------------------------------------------
void vtkKWLabeledEntry::SetBalloonHelpJustification(int j)
{
  if (this->Label)
    {
    this->Label->SetBalloonHelpJustification(j);
    }

  if (this->Entry)
    {
    this->Entry->SetBalloonHelpJustification(j);
    }
}

//----------------------------------------------------------------------------
void vtkKWLabeledEntry::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Label: " << this->Label << endl;
  os << indent << "Entry: " << this->Entry << endl;
}
