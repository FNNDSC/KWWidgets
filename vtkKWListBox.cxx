/*=========================================================================

  Module:    $RCSfile: vtkKWListBox.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWApplication.h"
#include "vtkKWListBox.h"
#include "vtkObjectFactory.h"


//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWListBox );
vtkCxxRevisionMacro(vtkKWListBox, "$Revision: 1.22 $");


//----------------------------------------------------------------------------
int vtkKWListBoxCommand(ClientData cd, Tcl_Interp *interp,
                        int argc, char *argv[]);

//----------------------------------------------------------------------------
vtkKWListBox::vtkKWListBox()
{   
  this->CurrentSelection = 0;
  this->Item = 0; 
  this->CommandFunction = vtkKWListBoxCommand;
  
  this->Scrollbar = vtkKWWidget::New();
  this->Scrollbar->SetParent(this);
  
  this->Listbox = vtkKWWidget::New();
  this->Listbox->SetParent(this);

  this->ScrollbarFlag = 1;
}

//----------------------------------------------------------------------------
vtkKWListBox::~vtkKWListBox()
{
  delete [] this->Item;
  delete [] this->CurrentSelection;
  
  this->Scrollbar->Delete();
  this->Listbox->Delete();
  
}


//----------------------------------------------------------------------------
int vtkKWListBox::GetNumberOfItems()
{
  if ( !this->IsCreated() )
    {
    return 0;
    }
  this->Script("%s size", this->Listbox->GetWidgetName());
  char* result = this->Application->GetMainInterp()->result;
  return atoi(result);
}

//----------------------------------------------------------------------------
void vtkKWListBox::DeleteRange(int start, int end)
{
  this->Script("%s delete %d %d", this->Listbox->GetWidgetName(), start, end);
}

//----------------------------------------------------------------------------
const char* vtkKWListBox::GetItem(int index)
{
  this->Script("%s get %d", this->Listbox->GetWidgetName(), index);
  char* result = this->Application->GetMainInterp()->result;
  delete [] this->Item;
  this->Item = strcpy(new char[strlen(result)+1], result);
  return this->Item;
}

//----------------------------------------------------------------------------
void vtkKWListBox::SetSelectionIndex(int sel)
{
  if ( sel < 0 )
    {
    return;
    }
  this->Script("%s selection set %d", this->Listbox->GetWidgetName(), sel);
}

//----------------------------------------------------------------------------
int vtkKWListBox::GetSelectionIndex()
{
  if ( !this->IsCreated() )
    {
    return 0;
    }
  this->Script("%s curselection", this->Listbox->GetWidgetName(),
               this->GetWidgetName());
  char* result = this->Application->GetMainInterp()->result;
  if ( strlen(result)>0 )
    {
    return atoi(result);
    }
  return -1;
}

  
//----------------------------------------------------------------------------
const char *vtkKWListBox::GetSelection()
{
  if ( !this->IsCreated() )
    {
    return 0;
    }
  if ( this->GetSelectionIndex() < 0 )
    {
    return 0;
    }
  this->Script("%s get [%s curselection]", this->Listbox->GetWidgetName(),
               this->Listbox->GetWidgetName());
  char* result = this->Application->GetMainInterp()->result;
  this->CurrentSelection = strcpy(new char[strlen(result)+1], result);
  return this->CurrentSelection;
}


//----------------------------------------------------------------------------
void vtkKWListBox::SetSelectState(int idx, int state)
{
  if ( idx < 0 )
    {
    return;
    }
  if (state)
    {
    this->Script("%s selection set %d", this->Listbox->GetWidgetName(), idx);
    }
  else
    {
    this->Script("%s selection clear %d", this->Listbox->GetWidgetName(), idx);
    }
}

//----------------------------------------------------------------------------
int vtkKWListBox::GetItemIndex(const char* item)
{
  if ( !item )
    {
    return 0;
    }
  int cc;
  for ( cc = 0; cc < this->GetNumberOfItems(); cc ++ )
    {
    if ( strcmp(item, this->GetItem(cc)) == 0 )
      {
      return cc;
      }
    }
  return -1;
}

//----------------------------------------------------------------------------
int vtkKWListBox::GetSelectState(int idx)
{
  if ( idx < 0 )
    {
    return 0;
    }

  this->Script("%s selection includes %d", this->Listbox->GetWidgetName(), idx);

  int result = atoi(this->Application->GetMainInterp()->result);
  return result;
}

//----------------------------------------------------------------------------
void vtkKWListBox::InsertEntry(int index, const char *name)
{
  this->Script("%s insert %d {%s}", this->Listbox->GetWidgetName(), index, name);
}


 
//----------------------------------------------------------------------------
void vtkKWListBox::SetDoubleClickCallback(vtkKWObject* obj, 
                                          const char* methodAndArgs)
{
  this->Script("bind %s <Double-1> {%s %s}", this->Listbox->GetWidgetName(),
               obj->GetTclName(), methodAndArgs);
}

//----------------------------------------------------------------------------
void vtkKWListBox::SetSingleClickCallback(vtkKWObject* obj, 
                                          const char* methodAndArgs)
{
  this->Script("bind %s <ButtonRelease-1> {%s %s}", this->Listbox->GetWidgetName(),
               obj->GetTclName(), methodAndArgs);
}


//----------------------------------------------------------------------------
int vtkKWListBox::AppendUnique(const char* name)
{
  int size = this->GetNumberOfItems();
  int found = 0;
  for(int i =0; i < size; i++)
    {
    if(strcmp(this->GetItem(i), name) == 0)
      {
      found = 1;
      break;
      }
    }
  if(!found)
    {
    this->InsertEntry(size, name);
    }
  return !found;
}



//----------------------------------------------------------------------------
void vtkKWListBox::Create(vtkKWApplication *app, const char *args)
{
  const char *wname;

  // Set the application

  if (this->IsCreated())
    {
    vtkErrorMacro("OptionListBox already created");
    return;
    }

  this->SetApplication(app);

  // create the top level
  wname = this->GetWidgetName();
  
  this->Script("frame %s ", wname);
  this->Scrollbar->Create( app, "scrollbar", "" );
  
  this->Listbox->Create( app, "listbox", (args?args:"") );
  
  this->Script( "%s configure -yscroll {%s set}", 
                this->Listbox->GetWidgetName(),
                this->Scrollbar->GetWidgetName());
  
  this->Script( "%s configure -command {%s yview}", 
                this->Scrollbar->GetWidgetName(),
                this->Listbox->GetWidgetName());
    
  if (this->ScrollbarFlag)
    {
    this->Script("pack %s -side right -fill y", this->Scrollbar->GetWidgetName());
    }
  this->Script("pack %s -side left -expand 1 -fill both", this->Listbox->GetWidgetName());

  // Update enable state

  this->UpdateEnableState();
}


//----------------------------------------------------------------------------
void vtkKWListBox::SetWidth(int w)
{
  this->Script("%s configure -width %d", this->Listbox->GetWidgetName(), w);
}

//----------------------------------------------------------------------------
void vtkKWListBox::SetScrollbarFlag(int v)
{
  this->ScrollbarFlag = v;

  if (this->Application)
    {
    this->Script("pack forget %s", this->Scrollbar->GetWidgetName());
    this->Script("pack forget %s", this->Listbox->GetWidgetName());
    if (this->ScrollbarFlag)
      {
      this->Script("pack %s -side right -fill y", this->Scrollbar->GetWidgetName());
      }
    this->Script("pack %s -side left -expand 1 -fill both", this->Listbox->GetWidgetName());
    }
}



//----------------------------------------------------------------------------
void vtkKWListBox::SetHeight(int h)
{
  this->Script("%s configure -height %d", this->Listbox->GetWidgetName(), h);
}

//----------------------------------------------------------------------------
void vtkKWListBox::DeleteAll()
{
  int n =  this->GetNumberOfItems();
  this->DeleteRange(0, n);
}

//----------------------------------------------------------------------------
void vtkKWListBox::SetBalloonHelpString(const char *str)
{
  this->Listbox->SetBalloonHelpString( str );
  this->Scrollbar->SetBalloonHelpString( str );
}


//----------------------------------------------------------------------------
void vtkKWListBox::SetBalloonHelpJustification( int j )
{
  this->Listbox->SetBalloonHelpJustification( j );
  this->Scrollbar->SetBalloonHelpJustification( j );
}

//----------------------------------------------------------------------------
void vtkKWListBox::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->PropagateEnableState(this->Scrollbar);
  this->PropagateEnableState(this->Listbox);
}
//----------------------------------------------------------------------------
void vtkKWListBox::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << "Listbox " << this->Listbox << endl;
}

