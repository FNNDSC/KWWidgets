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
#include "vtkXMLKWWindowWriter.h"

#include "vtkKWUserInterfaceNotebookManager.h"
#include "vtkKWWindow.h"
#include "vtkObjectFactory.h"
#include "vtkXMLDataElement.h"
#include "vtkXMLKWUserInterfaceNotebookManagerWriter.h"

vtkStandardNewMacro(vtkXMLKWWindowWriter);
vtkCxxRevisionMacro(vtkXMLKWWindowWriter, "$Revision: 1.3 $");

//----------------------------------------------------------------------------
char* vtkXMLKWWindowWriter::GetRootElementName()
{
  return "KWWindow";
}

//----------------------------------------------------------------------------
char* vtkXMLKWWindowWriter::GetUserInterfaceElementName()
{
  return "UserInterface";
}

//----------------------------------------------------------------------------
char* vtkXMLKWWindowWriter::GetUserInterfaceManagerElementName()
{
  return "UserInterfaceManager";
}

//----------------------------------------------------------------------------
int vtkXMLKWWindowWriter::AddNestedElements(vtkXMLDataElement *elem)
{
  if (!this->Superclass::AddNestedElements(elem))
    {
    return 0;
    }

  vtkKWWindow *obj = vtkKWWindow::SafeDownCast(this->Object);
  if (!obj)
    {
    vtkWarningMacro(<< "The KWWindow is not set!");
    return 0;
    }

  // User Interface

  vtkXMLDataElement *ui_elem = vtkXMLDataElement::New();
  ui_elem->SetName(vtkXMLKWWindowWriter::GetUserInterfaceElementName());
  elem->AddNestedElement(ui_elem);
  ui_elem->Delete();
  this->WriteUserInterfaceElement(ui_elem);

  return 1;
}

//----------------------------------------------------------------------------
int vtkXMLKWWindowWriter::WriteUserInterfaceElement(
  vtkXMLDataElement *ui_elem)
{
  if (!ui_elem)
    {
    return 0;
    }

  vtkKWWindow *obj = vtkKWWindow::SafeDownCast(this->Object);
  if (!obj)
    {
    vtkWarningMacro(<< "The KWWindow is not set!");
    return 0;
    }

  // Set attributes
 
  ui_elem->SetIntAttribute(
    "PropertiesVisibility", obj->GetPropertiesVisiblity());

  // User Interface Manager

  vtkKWUserInterfaceNotebookManager *uim_nb = 
    vtkKWUserInterfaceNotebookManager::SafeDownCast(
      obj->GetUserInterfaceManager());
  if (uim_nb)
    {
    vtkXMLKWUserInterfaceNotebookManagerWriter *xmlw = 
      vtkXMLKWUserInterfaceNotebookManagerWriter::New();
    xmlw->SetObject(uim_nb);
    xmlw->CreateInNestedElement(
      ui_elem, vtkXMLKWWindowWriter::GetUserInterfaceManagerElementName());
    xmlw->Delete();
    }

  return 1;
}
