/*=========================================================================

Copyright (c) 1998-2003 Kitware Inc. 469 Clifton Corporate Parkway,
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
#include "vtkKWObject.h"

#include "vtkKWApplication.h"
#include "vtkKWSerializer.h"
#include "vtkObjectFactory.h"
#include "vtkString.h"
#include "vtkTclUtil.h"

#include <stdarg.h>
#include <ctype.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWObject );
vtkCxxRevisionMacro(vtkKWObject, "$Revision: 1.39 $");

int vtkKWObjectCommand(ClientData cd, Tcl_Interp *interp,
                       int argc, char *argv[]);

//----------------------------------------------------------------------------
vtkKWObject::vtkKWObject()
{
  this->TclName = NULL;
  this->Application = NULL;  
  this->CommandFunction = vtkKWObjectCommand;
  this->NumberOfVersions = 0;
  this->Versions = NULL;
  this->VersionsLoaded = 0;
  this->TraceInitialized = 0;
  this->TraceReferenceObject = NULL;
  this->TraceReferenceCommand = NULL;
}

//----------------------------------------------------------------------------
vtkKWObject::~vtkKWObject()
{
  if (this->TclName)
    {
    delete [] this->TclName;
    }
  for (int i = 0; i < this->NumberOfVersions; i++)
    {
    delete [] this->Versions[i*2];
    delete [] this->Versions[i*2+1];    
    }
  if (this->Versions)
    {
    delete [] this->Versions;
    }
  
  this->SetApplication(NULL);
  this->SetTraceReferenceObject(NULL);
  this->SetTraceReferenceCommand(NULL);
}


//----------------------------------------------------------------------------
void vtkKWObject::Serialize(ostream& os, vtkIndent indent)
{
  os << this->GetClassName() << endl;
  os << indent << "  {\n";
  os << indent << "  Versions\n";
  os << indent << "    {\n";
  this->SerializeRevision(os, indent.GetNextIndent().GetNextIndent());
  os << indent << "    }\n";
  this->SerializeSelf(os, indent.GetNextIndent());
  os << indent << "  }\n";
}

//----------------------------------------------------------------------------
void vtkKWObject::ExtractRevision(ostream& os,const char *revIn)
{
  char rev[128];
  sscanf(revIn,"$Revision: %s",rev);
  os << rev << endl;
}

//----------------------------------------------------------------------------
void vtkKWObject::SerializeRevision(ostream& os, vtkIndent indent)
{
  os << indent << "vtkKWObject ";
  this->ExtractRevision(os,"$Revision: 1.39 $");
}

//----------------------------------------------------------------------------
void vtkKWObject::Serialize(istream& is)
{
  char token[VTK_KWSERIALIZER_MAX_TOKEN_LENGTH];
  char tmp[VTK_KWSERIALIZER_MAX_TOKEN_LENGTH];

  // get the class name
  this->VersionsLoaded = 0;
  vtkKWSerializer::GetNextToken(&is,token);
  if (strcmp(token,this->GetClassName()))
    {
    vtkDebugMacro("A class name mismatch has occured.");
    }
  vtkKWSerializer::ReadNextToken(&is,"{",this);

  // for each token process it
  do
    {
    vtkKWSerializer::GetNextToken(&is,token);
    if (token[0] == '{')
      {
      vtkKWSerializer::FindClosingBrace(&is,this);
      }
    else
      {
      // get the version istory first
      if (!strcmp(token,"Versions"))
        {
        vtkKWSerializer::GetNextToken(&is,token);
        do
          {
          if (token[0] != '{')
            {
            vtkKWSerializer::GetNextToken(&is,tmp);
            this->AddVersion(token,tmp);
            }
          vtkKWSerializer::GetNextToken(&is,token);
          }
        while (token[0] != '}');
        this->VersionsLoaded = 1;
        vtkKWSerializer::GetNextToken(&is,token);
        }
      this->SerializeToken(is,token);
      }
    }
  while (token[0] != '}' && is.tellg() >= 0);
}

//----------------------------------------------------------------------------
void vtkKWObject::SerializeToken(istream& /*is*/, const char * /*token*/)
{
}

//----------------------------------------------------------------------------
const char *vtkKWObject::GetTclName()
{
  // is the name is already set the just return it
  if (this->TclName)
    {
    return this->TclName;
    }

  // otherwise we must register ourselves with tcl and get a name
  if (!this->GetApplication())
    {
    vtkErrorMacro("attempt to create Tcl instance before application was set!");
    return NULL;
    }

  vtkTclGetObjectFromPointer(this->GetApplication()->GetMainInterp(), 
                             (void *)this, this->CommandFunction);
  this->TclName = vtkString::Duplicate(
    this->GetApplication()->GetMainInterp()->result);
  return this->TclName;
}

//----------------------------------------------------------------------------
const char* vtkKWObject::Script(const char* format, ...)
{
  if(this->GetApplication())
    {
    va_list var_args1, var_args2;
    va_start(var_args1, format);
    va_start(var_args2, format);
    const char* result =
      this->GetApplication()->ScriptInternal(format, var_args1, var_args2);
    va_end(var_args1);
    va_end(var_args2);
    return result;
    }
  else
    {
    vtkWarningMacro("Attempt to script a command without a KWApplication.");
    return 0;
    }
}


//----------------------------------------------------------------------------
int vtkKWObject::GetIntegerResult(vtkKWApplication *app)
{
  int res;
  
  res = atoi(Tcl_GetStringResult(app->GetMainInterp()));
  return res;
}
float vtkKWObject::GetFloatResult(vtkKWApplication *app)
{
  float res;
  
  res = atof(Tcl_GetStringResult(app->GetMainInterp()));
  return res;
}

//----------------------------------------------------------------------------
void vtkKWObject::SetApplication (vtkKWApplication* arg)
{  
  vtkDebugMacro(<< this->GetClassName() 
  << " (" << this << "): setting " << "Application" " to " << arg ); 
  if (this->Application != arg) 
    { 
    if (this->Application != NULL) 
      { 
      this->Application->UnRegister(this); 
      this->Application = 0;
      }
    this->Application = arg;
    if (this->Application != NULL)
      {
      this->Application->Register(this);
      }
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkKWObject::AddVersion(const char *cname, const char *version)
{
  // allocate more space
  int cnt;
  
  char **objs = new char *[2*(this->NumberOfVersions+1)];

  // copy the old to the new
  for (cnt = 0; cnt < this->NumberOfVersions*2; cnt++)
    {
    objs[cnt] = this->Versions[cnt];
    }
  if (this->Versions)
    {
    delete [] this->Versions;
    }
  this->Versions = objs;
  char *classname = new char [strlen(cname)+1];
  sprintf(classname,"%s",cname);
  this->Versions[this->NumberOfVersions*2] = classname;

  
  // compute the revision, strip out extra junk
  char *revision = new char [strlen(version)+1];
  sprintf(revision,"%s",version);
  this->Versions[this->NumberOfVersions*2+1] = revision;
  this->NumberOfVersions++;
}

//----------------------------------------------------------------------------
int vtkKWObject::CompareVersions(const char *v1, const char *v2)
{
  if (!v1 && !v2)
    {
    return 0;
    }
  if (!v1)
    {
    return -1;
    }
  if (!v2)
    {
    return 1;
    }
  // both v1 and v2 are non NULL
  int nVer1 = 0;
  int nVer2 = 0;
  int ver1[7];
  int ver2[7];
  nVer1 = sscanf(v1,"%i.%i.%i.%i.%i.%i.%i", ver1, ver1+1, ver1+2,
                 ver1+3, ver1+4, ver1+5, ver1+6);
  nVer2 = sscanf(v2,"%i.%i.%i.%i.%i.%i.%i", ver2, ver2+1, ver2+2,
                 ver2+3, ver2+4, ver2+5, ver2+6);

  int pos = 0;
  // compare revisions
  while (pos < nVer1 && pos < nVer2)
    {
    if (ver1[pos] < ver2[pos])
      {
      return -1;
      }
    if (ver1[pos] > ver2[pos])
      {
      return 1;
      }    
                pos++;
    }
  // revisions match but maybe one has more .2.3.4.2
  if (nVer1 < nVer2)
    {
    return -1;
    }
  if (nVer1 > nVer2)
    {
    return 1;
    }
  // They are identical in every way
  return 0;
}

//----------------------------------------------------------------------------
const char *vtkKWObject::GetVersion(const char *cname)
{
  for (int i = 0; i < this->NumberOfVersions; i++)
    {
    if (!strcmp(this->Versions[i*2],cname))
      {
      return this->Versions[i*2+1];
      }
    }
  return NULL;
}


//----------------------------------------------------------------------------
int vtkKWObject::InitializeTrace(ofstream* file)
{
  int stateFlag = 0;
  int dummyInit = 0;
  int* pInit;

  if(this->Application == NULL)
    {
    return 0;
    }

  // Special logic for state files.
  // The issue is that this KWObject can only keep track of initialization
  // for one file, and I do not like any possible solutions to extend this.
  if (file == NULL || file == this->Application->GetTraceFile())
    { // Tracing:  Keep track of initialization.
    file = this->Application->GetTraceFile();
    pInit = &(this->TraceInitialized);
    }
  else
    { // Saving state: Ignore trace initialization.
    stateFlag = 1;
    pInit = &(dummyInit);
    }

  // There is no need to do anything if there is no trace file.
  if (file == NULL)
    {
    return 0;
    }
  if (*pInit)
    {
    return 1;
    }
  if (this->TraceReferenceObject && this->TraceReferenceCommand)
    {
    if (this->TraceReferenceObject->InitializeTrace(file))
      {
      *file << "set kw(" << this->GetTclName() << ") [$kw(" 
            << this->TraceReferenceObject->GetTclName() << ") "
            << this->TraceReferenceCommand << "]" << endl;
      *pInit = 1;
      }
    }

  // Hack to get state working.
  if (stateFlag)
    {  // Tracing relies on sources being initialized outside of this call.
    return 1;
    }

  return *pInit;
}  

//----------------------------------------------------------------------------
void vtkKWObject::AddTraceEntry(const char *format, ...)
{
  if ( !this->Application )
    {
    return;
    }

  ofstream *os;

  os = this->Application->GetTraceFile();
  if (os == NULL)
    {
    return;
    }

  if (this->Application == NULL || this->InitializeTrace(os) == 0)
    {
    return;
    }

  char event[1600];
  char* buffer = event;
  
  va_list ap;
  va_start(ap, format);
  int length = this->EstimateFormatLength(format, ap);
  va_end(ap);
  
  if(length > 1599)
    {
    buffer = new char[length+1];
    }
  
  va_list var_args;
  va_start(var_args, format);
  vsprintf(buffer, format, var_args);
  va_end(var_args);
  
  *os << buffer << endl;
  
  if(buffer != event)
    {
    delete [] buffer;
    }
}


//----------------------------------------------------------------------------
void vtkKWObject::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Application: " << this->GetApplication() << endl;
  os << indent << "TraceInitialized: " << this->GetTraceInitialized() << endl;
  os << indent << "TraceReferenceCommand: " 
     << (this->TraceReferenceCommand?this->TraceReferenceCommand:"none") 
     << endl;
  os << indent << "TraceReferenceObject: " 
     << this->GetTraceReferenceObject() << endl;
}

//----------------------------------------------------------------------------
int vtkKWObject::EstimateFormatLength(const char* format, va_list ap)
{
  // Quick-hack attempt at estimating the length of the string.
  // Should never under-estimate.
  
  // Start with the length of the format string itself.
  int length = vtkString::Length(format);
  
  // Increase the length for every argument in the format.
  const char* cur = format;
  while(*cur)
    {
    if(*cur++ == '%')
      {
      // Skip "%%" since it doesn't correspond to a va_arg.
      if(*cur != '%')
        {
        while(!int(isalpha(*cur)))
          {
          ++cur;
          }
        switch (*cur)
          {
          case 's':
            {
            // Check the length of the string.
            char* s = va_arg(ap, char*);
            if(s)
              {
              length += vtkString::Length(s);
              }
            } break;
          case 'e':
          case 'f':
          case 'g':
            {
            // Assume the argument contributes no more than 64 characters.
            length += 64;
            
            // Eat the argument.
            static_cast<void>(va_arg(ap, double));
            } break;
          default:
            {
            // Assume the argument contributes no more than 64 characters.
            length += 64;
            
            // Eat the argument.
            static_cast<void>(va_arg(ap, int));
            } break;
          }
        }
      
      // Move past the characters just tested.
      ++cur;
      }
    }
  
  return length;
}


