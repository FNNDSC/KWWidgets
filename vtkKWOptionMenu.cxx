/*=========================================================================

  Module:    $RCSfile: vtkKWOptionMenu.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWOptionMenu.h"

#include "vtkKWApplication.h"
#include "vtkKWMenu.h"
#include "vtkObjectFactory.h"
#include "vtkString.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWOptionMenu );
vtkCxxRevisionMacro(vtkKWOptionMenu, "$Revision: 1.27 $");

//----------------------------------------------------------------------------
vtkKWOptionMenu::vtkKWOptionMenu()
{
  this->CurrentValue = NULL;
  this->Menu = vtkKWMenu::New();
}

//----------------------------------------------------------------------------
vtkKWOptionMenu::~vtkKWOptionMenu()
{
  if (this->CurrentValue)
    {
    delete [] this->CurrentValue;
    this->CurrentValue = NULL;
    }
  this->Menu->Delete();
}

//----------------------------------------------------------------------------
const char *vtkKWOptionMenu::GetValue()
{
  if (this->CurrentValue)
    {
    delete [] this->CurrentValue;
    this->CurrentValue = 0;
    }
  if ( this->Application )
    {
    this->Script("set %sValue",this->GetWidgetName());
    this->CurrentValue = vtkString::Duplicate(
      this->Application->GetMainInterp()->result);
    }
  return this->CurrentValue;  
}

//----------------------------------------------------------------------------
void vtkKWOptionMenu::SetValue(const char *s)
{
  if (this->Application && s)
    {
    this->Script("set %sValue {%s}", this->GetWidgetName(),s);
    }
}

//----------------------------------------------------------------------------
void vtkKWOptionMenu::SetCurrentEntry(const char *name)
{ 
  this->SetValue(name);
}
 
//----------------------------------------------------------------------------
void vtkKWOptionMenu::SetCurrentImageEntry(const char *image_name)
{ 
  this->Script("%s configure -image %s", this->GetWidgetName(), image_name);
  this->SetValue(image_name);
}
 
//----------------------------------------------------------------------------
const char* vtkKWOptionMenu::GetEntryLabel(int index)
{ 
  return this->Menu->GetItemLabel(index);
}

//----------------------------------------------------------------------------
int vtkKWOptionMenu::GetNumberOfEntries()
{ 
  return this->Menu->GetNumberOfItems();
}
 
//----------------------------------------------------------------------------
void vtkKWOptionMenu::AddEntry(const char *name)
{
  this->AddEntryWithCommand(name, 0, 0, 0);
}

//----------------------------------------------------------------------------
void vtkKWOptionMenu::AddEntryWithCommand(const char *name, 
                                          vtkKWObject *obj, 
                                          const char *method,
                                          const char *options)
{
  ostrstream extra;
  extra << "-variable " << this->GetWidgetName() << "Value";
  if (options)
    {
    extra << " " << options;
    }
  extra << ends;
  this->Menu->AddGeneric("radiobutton", name, obj, method, extra.str(), 0);
  extra.rdbuf()->freeze(0);
}

//----------------------------------------------------------------------------
void vtkKWOptionMenu::AddImageEntryWithCommand(const char *image_name, 
                                               vtkKWObject *obj, 
                                               const char *method,
                                               const char *options)
{
  ostrstream extra;
  if (image_name)
    {
    extra << "-image " << image_name << " -selectimage " << image_name;
    }
  if (options)
    {
    extra << " " << options;
    }
  extra << ends;
  this->AddEntryWithCommand(image_name, obj, method, extra.str());
  extra.rdbuf()->freeze(0);
}

//----------------------------------------------------------------------------
void vtkKWOptionMenu::AddSeparator()
{
  this->Menu->AddSeparator();
}

//----------------------------------------------------------------------------
void vtkKWOptionMenu::DeleteEntry(const char* name)
{ 
  this->Menu->DeleteMenuItem(name);
}

//----------------------------------------------------------------------------
void vtkKWOptionMenu::DeleteEntry(int index)
{
  this->Menu->DeleteMenuItem(index);
}

//----------------------------------------------------------------------------
int vtkKWOptionMenu::HasEntry(const char *name)
{
  return this->Menu->HasItem(name);
}

//----------------------------------------------------------------------------
void vtkKWOptionMenu::ClearEntries()
{
  this->Menu->DeleteAllMenuItems();
}

//----------------------------------------------------------------------------
void vtkKWOptionMenu::Create(vtkKWApplication *app, const char *args)
{
  const char *wname;

  // Set the application

  if (this->IsCreated())
    {
    vtkErrorMacro("OptionMenu already created");
    return;
    }

  this->SetApplication(app);

  // Create the menu

  this->Menu->SetParent(this);

  // Create the top level

  wname = this->GetWidgetName();
  
  this->Script("menubutton %s -textvariable %sValue -indicatoron 1 -menu %s "
               "-relief raised -bd 2 -highlightthickness 0 -anchor c "
               "-direction flush %s", 
               wname, wname, this->Menu->GetWidgetName(), (args?args:""));

  this->Menu->Create(app, "-tearoff 0");

  // Update enable state

  this->UpdateEnableState();
}

//----------------------------------------------------------------------------
void vtkKWOptionMenu::IndicatorOn()
{
  if (!this->IsCreated())
    {
    return;
    }

  this->Script("%s config -indicatoron 1", this->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkKWOptionMenu::IndicatorOff()
{
  if (!this->IsCreated())
    {
    return;
    }

  this->Script("%s config -indicatoron 0", this->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkKWOptionMenu::SetWidth(int width)
{
  if (this->IsCreated())
    {
    this->Script("%s configure -width %d", this->GetWidgetName(), width);
    }
}

//----------------------------------------------------------------------------
void vtkKWOptionMenu::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Menu: " << this->Menu << endl;
}

