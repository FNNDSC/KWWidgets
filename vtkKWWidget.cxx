/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWWidget.cxx,v $
  Language:  C++
  Date:      $Date: 2000-05-30 19:14:58 $
  Version:   $Revision: 1.8 $

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
#include "vtkKWWidget.h"
#include "vtkObjectFactory.h"
#include "vtkKWWindow.h"


//------------------------------------------------------------------------------
vtkKWWidget* vtkKWWidget::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkKWWidget");
  if(ret)
    {
    return (vtkKWWidget*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkKWWidget;
}




int vtkKWWidgetCommand(ClientData cd, Tcl_Interp *interp,
                       int argc, char *argv[]);

vtkKWWidget::vtkKWWidget()
{
  this->WidgetName = NULL;
  this->Parent = NULL;
  this->CommandFunction = vtkKWWidgetCommand;
  this->Children = vtkKWWidgetCollection::New();
  this->DeletingChildren= 0;
  this->BalloonHelpString = NULL;  
  this->BalloonHelpJustification = 0;
}

vtkKWWidget::~vtkKWWidget()
{
  if (this->BalloonHelpString)
    {
    this->SetBalloonHelpString(NULL);
    }
  this->Children->Delete();
  
  if (this->Application)
    {
    this->Script("destroy %s",this->GetWidgetName());
    }
  if (this->WidgetName)
    {
    delete [] this->WidgetName;
    }
  this->SetParent(NULL);
  this->SetApplication(NULL);
}

void vtkKWWidget::SetParent(vtkKWWidget *p)
{
  if (this->Parent && p)
    {
    vtkErrorMacro("Error attempt to reparent a widget!");
    return;
    }
  if (this->Parent)
    {
    vtkKWWidget *tmp = this->Parent;
    this->Parent = NULL;
    tmp->UnRegister(this);
    tmp->RemoveChild(this);
    }
  else if (p)
    {
    this->Parent = p;
    p->Register(this);
    p->AddChild(this);
    }
}

const char *vtkKWWidget::GetWidgetName()
{
  static unsigned long count = 0;

  // is the name is already set the just return it
  if (this->WidgetName)
    {
    return this->WidgetName;
    }

  // create this widgets name
  char local[256];
  // get the parents name
  if (this->Parent)
    {
    const char *tmp = this->Parent->GetWidgetName();
    sprintf(local,"%s.%i",tmp,count);
    }
  else
    {
    sprintf(local,".%i",count);
    }
  count++;
  this->WidgetName = new char [strlen(local)+1];
  strcpy(this->WidgetName,local);
  return this->WidgetName;
}

void vtkKWWidget::Create(vtkKWApplication *app, char *name, char *args)
{
  // must set the application
  if (this->Application)
    {
    vtkErrorMacro("widget already created");
    return;
    }
  this->SetApplication(app);
  this->Script("%s %s %s",name,this->GetWidgetName(),args);
}

int  vtkKWWidget::GetNetReferenceCount() 
{
  int childCounts = 0;
  vtkKWWidget *child;
  
  for (this->Children->InitTraversal(); 
       child = this->Children->GetNextKWWidget();)
    {
    childCounts += child->GetNetReferenceCount();
    }
  return this->ReferenceCount + childCounts - 
    2*this->Children->GetNumberOfItems();
}

void vtkKWWidget::UnRegister(vtkObject *o)
{
  if (!this->DeletingChildren)
    {
    // delete the children if we are about to be deleted
    if (this->ReferenceCount == this->Children->GetNumberOfItems() + 1)
      {
      vtkKWWidget *child;
  
      this->DeletingChildren = 1;
      this->Children->InitTraversal();
      while(child = this->Children->GetNextKWWidget())
          {
          child->SetParent(NULL);
          }
      this->DeletingChildren = 0;
      }
    }
  
  this->vtkObject::UnRegister(o);
}


void vtkKWWidget::SetCommand(vtkKWObject* CalledObject, const char * CommandString)
{
  char* command = this->CreateCommand(CalledObject, CommandString);
  this->Application->SimpleScript(command);
  delete [] command;
}

char* vtkKWWidget::CreateCommand(vtkKWObject* CalledObject, const char * CommandString)
{
  ostrstream event;
  event << this->GetWidgetName() << " configure -command {" 
	<< CalledObject->GetTclName() 
	<< " " << CommandString << "} " << ends;
  char *rval = new char [strlen(event.str())+1];
  strcpy(rval,event.str());
  event.rdbuf()->freeze(0);
  return rval;
}

void vtkKWWidget::SetBalloonHelpString(char *str)
{
  if (this->Application == NULL)
    {
    vtkErrorMacro("Application needs to be set before balloon help.");
    return;
    }

  // A little overkill.
  if (this->BalloonHelpString == NULL && str == NULL)
    {
    return;
    }

  // We do not need to change the bindings if the string is replaced
  // with another string.
  if (this->BalloonHelpString == NULL && str != NULL)
    { // Turning balloon help on.
    this->Script("bind %s <Enter> {%s BalloonHelpTrigger %s}", 
               this->GetWidgetName(), this->Application->GetTclName(),
               this->GetTclName());
    this->Script("bind %s <ButtonPress> {%s BalloonHelpCancel}", 
               this->GetWidgetName(), this->Application->GetTclName());
    this->Script("bind %s <Leave> {%s BalloonHelpCancel}", 
               this->GetWidgetName(), this->Application->GetTclName());
    }
  if (this->BalloonHelpString != NULL && str == NULL)
    { // Turning balloon help off.
    this->Script("catch {bind %s <Enter> {}}", this->GetWidgetName());
    this->Script("catch {bind %s <ButtonPress> {}}", this->GetWidgetName());
    this->Script("catch {bind %s <Leave> {}}", this->GetWidgetName());
    }

  // Normal string stuff.
  if (this->BalloonHelpString)
    {
    delete [] this->BalloonHelpString;
    this->BalloonHelpString = NULL;
    }
  if (str != NULL)
    {
    this->BalloonHelpString = new char[strlen(str)+1];
    strcpy(this->BalloonHelpString, str);
    }
}

void vtkKWWidget::SerializeRevision(ostream& os, vtkIndent indent)
{
  vtkKWObject::SerializeRevision(os,indent);
  os << indent << "vtkKWWidget ";
  this->ExtractRevision(os,"$Revision: 1.8 $");
}

vtkKWWindow* vtkKWWidget::GetWindow()
{
  vtkKWWindow* win =0;
  vtkKWWidget* widget = this->GetParent();
  while(widget)
    {
    if((win = vtkKWWindow::SafeDownCast(widget)))
      {
      return win;
      }
    widget = widget->GetParent();
    }
  return win;
}
