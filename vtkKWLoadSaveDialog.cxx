/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWLoadSaveDialog.cxx,v $
  Language:  C++
  Date:      $Date: 2003-03-18 21:45:07 $
  Version:   $Revision: 1.25 $

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
#include "vtkKWLoadSaveDialog.h"

#include "vtkKWApplication.h"
#include "vtkKWWindow.h"
#include "vtkObjectFactory.h"
#include "vtkString.h"

//------------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWLoadSaveDialog );
vtkCxxRevisionMacro(vtkKWLoadSaveDialog, "$Revision: 1.25 $");

vtkKWLoadSaveDialog::vtkKWLoadSaveDialog()
{
  this->Done = 1;
  this->FileTypes = 0;
  this->Title = 0;
  this->FileName = 0;
  this->LastPath = 0;

  this->SaveDialog = 0;
  this->ChooseDirectory = 0;
  this->DefaultExtension = 0;

  this->SetTitle("Open Text Document");
  this->SetFileTypes("{{Text Document} {.txt}}");
}

vtkKWLoadSaveDialog::~vtkKWLoadSaveDialog()
{
  this->SetFileTypes(0);
  this->SetTitle(0);
  this->SetFileName(0);
  this->SetDefaultExtension(0);
  this->SetLastPath(0);
}

void vtkKWLoadSaveDialog::Create(vtkKWApplication *app, const char* /*args*/)
{
  // Set the application

  if (this->IsCreated())
    {
    vtkErrorMacro("Load Save Dialog already created");
    return;
    }

  this->SetApplication(app);
  // Nothing else here for now
}

int vtkKWLoadSaveDialog::Invoke()
{
  this->Application->SetDialogUp(1);
  char *path = NULL;
  ostrstream command;

#if (TK_MAJOR_VERSION == 8) && (TK_MINOR_VERSION >= 3)
  if (this->ChooseDirectory)
    {
    command << "tk_chooseDirectory";
    }
  else
    {
#endif
    command << (this->SaveDialog ? "tk_getSaveFile" : "tk_getOpenFile");
#if (TK_MAJOR_VERSION == 8) && (TK_MINOR_VERSION >= 3)
    }
#endif

  command << " -title {" << this->Title << "}"
          << " -initialdir {" 
          << ((this->LastPath && strlen(this->LastPath)>0)? this->LastPath : ".")
          << "}";

#if (TK_MAJOR_VERSION == 8) && (TK_MINOR_VERSION >= 3)
  if (this->ChooseDirectory)
    {
    command << " -mustexist 1";
    }
  else
    {
#endif
    command << " -defaultextension {" 
            << (this->DefaultExtension ? this->DefaultExtension : "")
            << "}"
            << " -filetypes {" << this->FileTypes << "}";
#if (TK_MAJOR_VERSION == 8) && (TK_MINOR_VERSION >= 3)
    }
#endif
  
  vtkKWWindow* window = this->GetWindow();
  if (window)
    {
    command << " -parent " << window->GetWidgetName();
    }
  command << ends;
  this->Script(command.str());
  command.rdbuf()->freeze(0);

  int res = 0;

  // At this point, the string is encoded as UTF-8, which matches ASCII value
  // up to 0x7F in hexadecimal). Higher values are encoded as two or 3 bytes.
  // Example: the file Baumann_G�_0002 becomes Baumann_Gü_0002
  // convertfrom identity seems to do the trick to convert back to
  // an ASCII-8 string that can be understood by the system.

  path = this->Application->GetMainInterp()->result;

  if (path && strlen(path))
    {
    this->Script("encoding convertfrom identity {%s}", 
                 this->Application->GetMainInterp()->result);

    path = this->Application->GetMainInterp()->result;

    this->SetFileName(path);
#if (TK_MAJOR_VERSION == 8) && (TK_MINOR_VERSION >= 3)
    if (this->ChooseDirectory)
      {
      this->SetLastPath(path);
      }
    else
      {
#endif
      this->GenerateLastPath(path);
#if (TK_MAJOR_VERSION == 8) && (TK_MINOR_VERSION >= 3)
      }
#endif
    res = 1;
    }
  else
    {
    this->SetFileName(0);
    }
  this->Application->SetDialogUp(0);
  this->Script("update");
  return res;
}

//----------------------------------------------------------------------------
const char* vtkKWLoadSaveDialog::GenerateLastPath(const char* path)
{
  this->SetLastPath(0);
  // Store last path
  if ( path && vtkString::Length(path) > 0 )
    {
    char *pth = vtkString::Duplicate(path);
    int pos = vtkString::Length(path);
    // Strip off the file name
    while (pos && pth[pos] != '/' && pth[pos] != '\\')
      {
      pos--;
      }
    pth[pos] = '\0';
    // Store in the registery
    this->SetLastPath(pth);
    delete [] pth;
    }
  return this->LastPath;
}

//----------------------------------------------------------------------------
void vtkKWLoadSaveDialog::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "DefaultExtension: " << 
    (this->DefaultExtension?this->DefaultExtension:"none")
     << endl;
  os << indent << "FileName: " << (this->FileName?this->FileName:"none") 
     << endl;
  os << indent << "FileTypes: " << (this->FileTypes?this->FileTypes:"none") 
     << endl;
  os << indent << "LastPath: " << (this->LastPath?this->LastPath:"none")
     << endl;
  os << indent << "SaveDialog: " << this->GetSaveDialog() << endl;
  os << indent << "ChooseDirectory: " << this->GetChooseDirectory() << endl;
  os << indent << "Title: " << (this->Title?this->Title:"none") << endl;
}
