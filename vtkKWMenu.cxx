/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWMenu.cxx,v $
  Language:  C++
  Date:      $Date: 2002-05-12 15:28:19 $
  Version:   $Revision: 1.22 $

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
#include "vtkKWMenu.h"
#include "vtkObjectFactory.h"
#include "vtkKWWindow.h"

//------------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWMenu );




int vtkKWMenuCommand(ClientData cd, Tcl_Interp *interp,
                       int argc, char *argv[]);

vtkKWMenu::vtkKWMenu()
{
  this->CommandFunction = vtkKWMenuCommand;
  this->TearOff = 0;
}

vtkKWMenu::~vtkKWMenu()
{
}

void vtkKWMenu::Create(vtkKWApplication* app, const char* args)
{
  // must set the application
  if (this->Application)
    {
    vtkErrorMacro("Menu already created");
    return;
    }
  this->SetApplication(app);
  this->Script("menu %s -tearoff %d %s", this->GetWidgetName(), this->TearOff, args); 
  this->Script("bind %s <<MenuSelect>> {%s DisplayHelp %%W}", this->GetWidgetName(),
	       this->GetTclName());
  
}

void vtkKWMenu::SetTearOff(int val)
{
  if (val == this->TearOff)
    {
    return;
    }
  this->Modified();
  this->TearOff = val;

  if (this->Application)
    {
    this->Script("%s configure -tearoff %d", this->GetWidgetName(), val);
    }
}


void vtkKWMenu::DisplayHelp(const char* widget)
{
  const char* tname = this->GetTclName();
  this->Script(
    "if [catch {set %sTemp $%sHelpArray([%s entrycget active -label])} %sTemp ]"
    " { set %sTemp \"\"}; set %sTemp", 
    tname, tname, widget, tname, tname, tname );
  if(this->GetApplication()->GetMainInterp()->result)
    {
    vtkKWWindow* window = this->GetWindow();
    window->SetStatusText(
      this->GetApplication()->GetMainInterp()->result);
    }
}


void vtkKWMenu::AddGeneric(const char* addtype, 
			   const char* label,
			   vtkKWObject* Object,
			   const char* MethodAndArgString,
			   const char* extra, 
			   const char* help)
{
  ostrstream str;
  str << this->GetWidgetName() << " add " 
      << addtype << " -label {" << label
      << "} -command {" << Object->GetTclName() 
      << " " << MethodAndArgString << "} " ;
  if(extra)
    {
    str << extra;
    }
  str << ends;
  
  this->Application->SimpleScript(str.str());
  delete [] str.str();
  if(!help)
    {
    help = label;
    }
  this->Script("set {%sHelpArray(%s)} {%s}", this->GetTclName(), 
	       label, help);
}



void vtkKWMenu::InsertGeneric(int position, const char* addtype, 
			      const char* label, 
			      vtkKWObject* Object,
			      const char* MethodAndArgString, 
			      const char* extra, 
			      const char* help)
{
  ostrstream str;
  str << this->GetWidgetName() << " insert " << position << " " 
      << addtype << " -label {" << label
      << "} -command {" << Object->GetTclName() 
      << " " << MethodAndArgString << "} ";
  if(extra)
    {
    str << extra;
    }
  str << ends;
  if(!help)
    {
    help = label;
    }
  this->Application->SimpleScript(str.str());
  delete [] str.str();
  this->Script("set {%sHelpArray(%s)} {%s}", this->GetTclName(), 
	       label, help);

}

void vtkKWMenu::AddCascade(const char* label, vtkKWMenu* menu, 
			   int underline, const char* help)
{
  ostrstream str;
  str << this->GetWidgetName() << " add cascade -label \"" << label << "\" -menu " 
      << menu->GetWidgetName() << " -underline " << underline << ends;
  this->Application->SimpleScript(str.str());
  delete [] str.str();
  if(!help)
    {
    help = label;
    }
  this->Script("set {%sHelpArray(%s)} {%s}", this->GetTclName(), 
	       label, help);

}



void  vtkKWMenu::InsertCascade(int position, 
			       const char* label, 
			       vtkKWMenu* menu, 
			       int underline, const char* help)
{
  ostrstream str;
  
  str << this->GetWidgetName() << " insert " << position 
      << " cascade -label \"" << label << "\" -menu " 
      << menu->GetWidgetName() << " -underline " << underline << ends;
  this->Application->SimpleScript(str.str());
  delete [] str.str();
  if(!help)
    {
    help = label;
    }
  this->Script("set {%sHelpArray(%s)} {%s}", this->GetTclName(), 
	       label, help);
}

void  vtkKWMenu::AddCheckButton(const char* label, vtkKWObject* Object, 
				const char* MethodAndArgString, const char* help )
{ 
  this->AddCheckButton(label, Object, MethodAndArgString, -1, help);
}
 
void  vtkKWMenu::AddCheckButton(const char* label, vtkKWObject* Object, 
				const char* MethodAndArgString, 
				int underline, const char* help )
{ 
  static int count = 0;
  ostrstream str;
  str << "-variable " << this->GetWidgetName() << "TempVar" << count++;
  if ( underline >= 0 )
    {
    str << " -underline " << underline;
    }
  str << ends;
  this->AddGeneric("checkbutton", label, Object, MethodAndArgString, str.str(), help);
  delete [] str.str();
}


void vtkKWMenu::InsertCheckButton(int position, 
				  const char* label, vtkKWObject* Object, 
				  const char* MethodAndArgString, const char* help )
{ 
  this->InsertCheckButton( position, label, Object, MethodAndArgString,
			   -1, help );
}

void vtkKWMenu::InsertCheckButton(int position, 
				  const char* label, vtkKWObject* Object, 
				  const char* MethodAndArgString, 
				  int underline, const char* help )
{ 
  static int count = 0;
  ostrstream str;
  str << "-variable " << this->GetWidgetName() << count++ << "TempVar ";
  if ( underline >= 0 )
    {
    str << " -underline " << underline;
    }
  str << ends;
  this->InsertGeneric(position, "checkbutton", label, Object, 
		      MethodAndArgString, str.str(), help);
  delete [] str.str();
}


void  vtkKWMenu::AddCommand(const char* label, vtkKWObject* Object,
			    const char* MethodAndArgString,
			    int underline, 
			    const char* help)
{
  ostrstream str;
  if ( underline >= 0 )
    {
    str << " -underline " << underline;
    }
  str << ends;
  this->AddGeneric("command", label, Object, 
		   MethodAndArgString, str.str(), help);
  delete [] str.str();
}

void  vtkKWMenu::AddCommand(const char* label, vtkKWObject* Object,
			    const char* MethodAndArgString ,
			    const char* help)
{
  this->AddGeneric("command", label, Object, 
		   MethodAndArgString, NULL, help);
}

void vtkKWMenu::InsertCommand(int position, const char* label, vtkKWObject* Object,
			      const char* MethodAndArgString,
			      int underline, 
			      const char* help)
{
  ostrstream str;
  if ( underline >= 0 )
    {
    str << " -underline " << underline;
    }
  str << ends;
  this->InsertGeneric(position, "command", label, Object,
		      MethodAndArgString, str.str(), help);
  delete [] str.str();
}

void vtkKWMenu::InsertCommand(int position, const char* label, vtkKWObject* Object,
			      const char* MethodAndArgString,
			      const char* help)
{
  this->InsertGeneric(position, "command", label, Object,
		      MethodAndArgString, NULL, help);
}

char* vtkKWMenu::CreateRadioButtonVariable(vtkKWObject* Object, 
                                           const char* varname)
{
  ostrstream str;
  str << Object->GetTclName() << varname << ends;
  return str.str();
}

  
  
int vtkKWMenu::GetRadioButtonValue(vtkKWObject* Object, 
                                   const char* varname)
{
  int res;
  
  char *rbv = 
    this->CreateRadioButtonVariable(Object,varname);
  this->Script("set %s",rbv);
  res = this->GetIntegerResult(this->Application);
  delete [] rbv;
  return res;
}
    
void vtkKWMenu::CheckRadioButton(vtkKWObject* Object, 
                                 const char* varname, int id)
{
  char *rbv = 
    this->CreateRadioButtonVariable(Object,varname);
  this->Script("set %s",rbv);
  if (this->GetIntegerResult(this->Application) != id)
    {
    this->Script("set %s %d",rbv,id);
    }
  delete [] rbv;
}


void vtkKWMenu::AddRadioButton(int value, const char* label, const char* buttonVar, 
			       vtkKWObject* Object, 
			       const char* MethodAndArgString,
			       int underline, 
			       const char* help)
{
  ostrstream str;
  str << "-value " << value << " -variable " << buttonVar;
  if ( underline >= 0 )
    {
    str << " -underline " << underline;
    }
  str << ends;
  this->AddGeneric("radiobutton", label, Object,
		   MethodAndArgString, str.str(), help);
  delete [] str.str();
}

void vtkKWMenu::AddRadioButton(int value, const char* label, const char* buttonVar, 
			       vtkKWObject* Object, 
			       const char* MethodAndArgString,
			       const char* help)
{
  this->AddRadioButton(value, label, buttonVar, Object, MethodAndArgString,
		       -1, help);
}


void vtkKWMenu::InsertRadioButton(int position, int value, const char* label, 
                                  const char* buttonVar, 
				  vtkKWObject* Object, 
				  const char* MethodAndArgString,
				  const char* help)
{
  this->InsertRadioButton( position, value, label, buttonVar, Object,
			   MethodAndArgString, -1, help );
}

void vtkKWMenu::InsertRadioButton(int position, int value, const char* label, 
                                  const char* buttonVar, 
				  vtkKWObject* Object, 
				  const char* MethodAndArgString,
				  int underline,
				  const char* help)
{
  ostrstream str;
  str << "-value " << value << " -variable " << buttonVar;
  if ( underline >= 0 )
    {
    str << " -underline " << underline;
    }
  str << ends;
  this->InsertGeneric(position, "radiobutton", label, Object,
		      MethodAndArgString, str.str(), help);
  delete [] str.str();
}

void vtkKWMenu::Invoke(int position)
{
  this->Script("%s invoke %d", this->GetWidgetName(), position);
}

void vtkKWMenu::Invoke(const char* item)
{
  if ( !this->IsItemPresent(item) )
    {
    return;
    }
  this->Invoke(this->GetIndex(item));
}

void vtkKWMenu::DeleteMenuItem(int position)
{
  this->Script("catch {%s delete %d}", this->GetWidgetName(), position);
  this->Script("set {%sHelpArray([%s entrycget %d -label])} {}", 
	       this->GetWidgetName(), this->GetWidgetName(), 
	       position);
}

void vtkKWMenu::DeleteMenuItem(const char* menuitem)
{
  this->Script("catch {%s delete {%s}}", this->GetWidgetName(), menuitem);
  this->Script("set {%sHelpArray(%s)} {}", this->GetWidgetName(), menuitem);
}

void vtkKWMenu::DeleteAllMenuItems()
{
  int i, last;
  
  this->Script("%s index end", this->GetWidgetName());
  if (strcmp("none", this->GetApplication()->GetMainInterp()->result) == 0)
    {
    return;
    }
  
  last = vtkKWObject::GetIntegerResult(this->Application);
  
  for (i = last; i >= 0; --i)
    {
    this->DeleteMenuItem(i);
    }
}

int vtkKWMenu::GetIndex(const char* menuname)
{
  this->Script("%s index {%s}", this->GetWidgetName(), menuname);
  return vtkKWObject::GetIntegerResult(this->Application);
}

int vtkKWMenu::IsItemPresent(const char* menuname)
{
  this->Script("catch {%s index {%s}}", this->GetWidgetName(), menuname);
  return !vtkKWObject::GetIntegerResult(this->Application);
}

void vtkKWMenu::AddSeparator()
{
  this->Script( "%s add separator", this->GetWidgetName());
}

void vtkKWMenu::InsertSeparator(int position)
{
  this->Script( "%s insert %d separator", this->GetWidgetName(), position);
}

void vtkKWMenu::SetState(int index, int state)
{
  char stateStr[][9] = { "normal", "active", "disabled" };
  if ( state <= vtkKWMenu::Normal || state > vtkKWMenu::Disabled )
    {
    state = 0;
    }
  this->Script("%s entryconfigure %d -state %s", 
	       this->GetWidgetName(), index, stateStr[state] );
}

void vtkKWMenu::SetState(const char* item, int state)
{
  if ( !this->IsItemPresent(item) )
    {
    return;
    }
  int index = this->GetIndex(item);
  this->SetState(index, state);
}

void vtkKWMenu::ConfigureItem(int index, const char* conf)
{
  ostrstream str;
  str << this->GetWidgetName() << " entryconfigure "
      << index << " " << conf << ends;
  this->Script(str.str());
  str.rdbuf()->freeze(0);
}


void vtkKWMenu::SetEntryCommand(int index, vtkKWObject* object, 
			   const char* MethodAndArgString)
{
  ostrstream str;
  str << this->GetWidgetName() << " entryconfigure "
      << index << " -command {" << object->GetTclName() 
      << " " << MethodAndArgString << "}" << ends;
  this->Script(str.str());
  str.rdbuf()->freeze(0);
}

void vtkKWMenu::SetEntryCommand(const char* item, vtkKWObject* object, 
			   const char* MethodAndArgString)
{
  if ( !this->IsItemPresent(item) )
    {
    return;
    }
  int index = this->GetIndex(item);
  this->SetEntryCommand(index, object, MethodAndArgString);
}


//----------------------------------------------------------------------------
void vtkKWMenu::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "TearOff: " << this->GetTearOff() << endl;
}
