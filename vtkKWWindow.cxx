/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWWindow.cxx,v $
  Language:  C++
  Date:      $Date: 2000-07-10 20:39:08 $
  Version:   $Revision: 1.17 $

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
#include "vtkKWWindow.h"
#include "vtkKWView.h"
#include "vtkObjectFactory.h"
#include "vtkKWMenu.h"

#include "KitwareLogo.h"

//------------------------------------------------------------------------------
vtkKWWindow* vtkKWWindow::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkKWWindow");
  if(ret)
    {
    return (vtkKWWindow*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkKWWindow;
}

int vtkKWWindowCommand(ClientData cd, Tcl_Interp *interp,
                             int argc, char *argv[]);

vtkKWWindow::vtkKWWindow()
{
  this->PropertiesParent = NULL;
  this->SelectedView = NULL;
  this->Views = vtkKWViewCollection::New();

  this->Menu = vtkKWMenu::New();
  this->Menu->SetParent(this);
  
  this->MenuFile = vtkKWMenu::New();
  this->MenuFile->SetParent(this->Menu);

  this->MenuHelp = vtkKWMenu::New();
  this->MenuHelp->SetParent(this->Menu);
  
  this->ToolbarFrame = vtkKWWidget::New();
  this->ToolbarFrame->SetParent(this);  

  this->MiddleFrame = vtkKWWidget::New();
  this->MiddleFrame->SetParent(this);

  this->ViewFrame = vtkKWWidget::New();
  this->ViewFrame->SetParent(this->MiddleFrame);

  this->StatusFrame = vtkKWWidget::New();
  this->StatusFrame->SetParent(this);
    
  this->StatusLabel = vtkKWWidget::New();
  this->StatusLabel->SetParent(this->StatusFrame);
  this->StatusImage = vtkKWWidget::New();
  this->StatusImage->SetParent(this->StatusFrame);
  this->StatusImageName = NULL;
  
  this->ProgressFrame = vtkKWWidget::New();
  this->ProgressFrame->SetParent(this->StatusFrame);
  this->ProgressGauge = vtkKWProgressGauge::New();
  this->ProgressGauge->SetParent(this->ProgressFrame);

  this->Notebook = vtkKWNotebook::New();
  
  this->CommandFunction = vtkKWWindowCommand;

  this->MenuEdit = NULL;
  this->MenuView = NULL;
  this->MenuProperties = NULL;
  this->NumberOfMRUFiles = 0;
}

vtkKWWindow::~vtkKWWindow()
{
  this->Notebook->Delete();
  this->SetPropertiesParent(NULL);
  this->SetSelectedView(NULL);
  if (this->Views)
    {
    this->Views->Delete();
    this->Views = NULL;
    }
  this->Menu->Delete();
  this->MenuFile->Delete();
  this->MenuHelp->Delete();
  this->ToolbarFrame->Delete();
  this->ViewFrame->Delete();
  this->MiddleFrame->Delete();
  this->StatusFrame->Delete();
  this->StatusImage->Delete();
  this->StatusLabel->Delete();
  this->ProgressFrame->Delete();
  this->ProgressGauge->Delete();
  
  if (this->MenuEdit)
    {
    this->MenuEdit->Delete();
    }
  if (this->MenuProperties)
    {
    this->MenuProperties->Delete();
    }
  if (this->MenuView)
    {
    this->MenuView->Delete();
    }
  if (this->StatusImageName)
    {
    delete [] this->StatusImageName;
    }
}

void vtkKWWindow::DisplayHelp()
{
  this->Application->DisplayHelp();
}

void vtkKWWindow::AddView(vtkKWView *v) 
{
  v->SetParentWindow(this);
  this->Views->AddItem(v);
}
void vtkKWWindow::RemoveView(vtkKWView *v) 
{
  v->SetParentWindow(NULL);
  this->Views->RemoveItem(v);
}

void vtkKWWindow::CreateDefaultPropertiesParent()
{
  if (!this->PropertiesParent)
    {
    vtkKWWidget *pp = vtkKWWidget::New();
    pp->SetParent(this->MiddleFrame);
    pp->Create(this->Application,"frame","-bd 0");
    this->Script("pack %s -before %s -side left -fill y -anchor nw",
                 pp->GetWidgetName(), 
                 this->ViewFrame->GetWidgetName());
    this->SetPropertiesParent(pp);
    pp->Delete();
    }
  else
    {
    vtkDebugMacro("Properties Parent already set for Window");
    }
}

void vtkKWWindow::SetSelectedView(vtkKWView *_arg)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting SelectedView to " << _arg ); 
  if (this->SelectedView != _arg) 
    { 
    if (this->SelectedView != NULL) 
      { 
      this->SelectedView->Deselect(this);
      this->SelectedView->UnRegister(this); 
      }
    this->SelectedView = _arg; 
    if (this->SelectedView != NULL) 
      { 
      this->SelectedView->Register(this); 
      this->SelectedView->Select(this);
      } 
    this->Modified(); 
    } 
}

// invoke the apps exit when selected
void vtkKWWindow::Exit()
{
  this->Application->Exit();
}

// invoke the apps close when selected
void vtkKWWindow::Close()
{
  vtkKWView *v;

  // Give each view a chance to close
  this->Views->InitTraversal();
  while(v = this->Views->GetNextKWView())
    {
    v->Close();
    }

  // Close this window in the application. The
  // application will exit if there are no more windows.
  this->Application->Close(this);
}

// invoke the apps about dialog when selected
void vtkKWWindow::DisplayAbout()
{
  this->Application->DisplayAbout(this);
}

void vtkKWWindow::SetStatusText(const char *text)
{
  if (text) 
    {
    this->Script("%s configure -text \"%s\"",
                 this->StatusLabel->GetWidgetName(), text);
    }
  else
    {
    this->Script("%s configure -text \"\"",
                 this->StatusLabel->GetWidgetName());
    }
}

// some common menus we provide here
vtkKWMenu *vtkKWWindow::GetMenuEdit()
{
  if (this->MenuEdit)
    {
    return this->MenuEdit;
    }
  
  this->MenuEdit = vtkKWMenu::New();
  this->MenuEdit->SetParent(this->GetMenu());
  this->MenuEdit->Create(this->Application,"-tearoff 0");
  // Make sure Edit menu is next to file menu
  this->Menu->InsertCascade(1, "Edit", this->MenuEdit, 0);
  return this->MenuEdit;
}

vtkKWMenu *vtkKWWindow::GetMenuView()
{
  if (this->MenuView)
    {
    return this->MenuView;
    }

  this->MenuView = vtkKWMenu::New();
  this->MenuView->SetParent(this->GetMenu());
  this->MenuView->Create(this->Application, "-tearoff 0");
  // make sure Help menu is on the right
  if (this->MenuEdit)
    { 
    this->Menu->InsertCascade(2, "View", this->MenuView, 0);
    }
  else
    {
    this->Menu->InsertCascade(1, "View", this->MenuView, 0);
    }
  
  return this->MenuView;
}

vtkKWMenu *vtkKWWindow::GetMenuProperties()
{
  if (this->MenuProperties)
    {
    return this->MenuProperties;
    }
  
  this->MenuProperties = vtkKWMenu::New();
  this->MenuProperties->SetParent(this->GetMenu());
  this->MenuProperties->Create(this->Application,"-tearoff 0");
  // make sure Help menu is on the right
  if (this->MenuView && this->MenuEdit)
    {
    this->Menu->InsertCascade(3, "Properties", this->MenuProperties, 0);
    }
  else if (this->MenuView || this->MenuEdit)
    {
    this->Menu->InsertCascade(2, "Properties", this->MenuProperties, 0);
    }
  else
    { 
    this->Menu->InsertCascade(1, "Properties", this->MenuProperties, 0);
    }
  return this->MenuProperties;
}

void vtkKWWindow::Create(vtkKWApplication *app, char *args)
{
  const char *wname;

  // must set the application
  if (this->Application)
    {
    vtkErrorMacro("Window already created");
    return;
    }

  this->SetApplication(app);
  Tcl_Interp *interp = this->Application->GetMainInterp();

  // create the top level
  wname = this->GetWidgetName();
  this->Script("toplevel %s -visual best %s",wname,args);
  this->Script("wm title %s {%s}",wname,
               app->GetApplicationName());
  this->Script("wm iconname %s {%s}",wname,
               app->GetApplicationName());
  this->Script("wm protocol %s WM_DELETE_WINDOW {%s Close}",
               wname, this->GetTclName());

  this->StatusFrame->Create(app,"frame","");
  this->Script("image create photo -height 34 -width 128");
  this->StatusImageName = new char [strlen(interp->result)+1];
  strcpy(this->StatusImageName,interp->result);
  this->CreateStatusImage();
  this->StatusImage->Create(app,"label",
                            "-relief sunken -bd 1 -height 38 -width 132 -fg #ffffff -bg #ffffff");
  this->Script("%s configure -image %s", this->StatusImage->GetWidgetName(),
               this->StatusImageName);
  this->Script("pack %s -side left -padx 2", 
               this->StatusImage->GetWidgetName());
  this->StatusLabel->Create(app,"label","-relief sunken -padx 3 -bd 1 -font \"Helvetica 10\" -anchor w");
  this->Script("pack %s -side left -padx 2 -expand yes -fill both",
               this->StatusLabel->GetWidgetName());
  this->Script("pack %s -side bottom -fill x -pady 2",
    this->StatusFrame->GetWidgetName());
  this->ProgressFrame->Create(app, "frame", "-relief sunken -borderwidth 2");
  this->ProgressGauge->SetLength(200);
  this->ProgressGauge->SetHeight(30);
  this->ProgressGauge->Create(app, "");
  this->Script("pack %s -side right -padx 2 -fill y", 
	       this->ProgressFrame->GetWidgetName());
  this->Script("pack %s -side right -padx 2 -pady 2",
               this->ProgressGauge->GetWidgetName());
  // To force the toolbar on top, I am create a separate "MiddleFrame" for the ViewFrame and PropertiesParent
  this->MiddleFrame->Create(app, "frame", "");
  this->Script("pack %s -side bottom -fill both -expand t",
    this->MiddleFrame->GetWidgetName());

  this->ViewFrame->Create(app,"frame","");
  this->Script("pack %s -side right -fill both -expand yes",
               this->ViewFrame->GetWidgetName());

  this->ToolbarFrame->Create(app, "frame", "-bd 0");
  this->Script("pack %s -side bottom -fill x -expand no",
    this->ToolbarFrame->GetWidgetName());

  // Set up standard menus
  this->Menu->Create(app,"-tearoff 0");
  this->MenuFile->Create(app,"-tearoff 0");
  this->Menu->AddCascade("File", this->MenuFile, 0);
  this->MenuFile->AddCommand("Load Script", this, "LoadScript");
  this->MenuFile->AddSeparator();
  this->MenuFile->AddCommand("Close", this, "Close");
  this->MenuFile->AddCommand("Exit", this, "Exit");
  // install the menu bar into this window
  this->InstallMenu(this->Menu);
  this->MenuHelp->Create(app,"-tearoff 0");
  this->Menu->AddCascade("Help", this->MenuHelp, 0);
  this->MenuHelp->AddCommand("OnLine Help", this, "DisplayHelp");
  this->MenuHelp->AddCommand("About", this, "DisplayAbout");

  char *rbv = 
    this->GetMenuProperties()->CreateRadioButtonVariable(
      this->GetMenuProperties(),"Radio");
  this->GetMenuProperties()->AddRadioButton(0,"Hide Properties", 
                                            rbv, this, "HideProperties");
  delete [] rbv;
}

void vtkKWWindow::ShowProperties()
{
  this->Script("pack %s -before %s -side left -fill y -anchor nw",
               this->PropertiesParent->GetWidgetName(), 
               this->ViewFrame->GetWidgetName());
}

void vtkKWWindow::HideProperties()
{
  // make sure the variable is set, otherwise set it
  this->GetMenuProperties()->CheckRadioButton(
    this->GetMenuProperties(),"Radio",0);
  
  // forget current props
  this->Script("pack forget %s",
               this->PropertiesParent->GetWidgetName());  
}

void vtkKWWindow::InstallMenu(vtkKWMenu* menu)
{ 
  this->Script("%s configure -menu %s", this->GetWidgetName(),
	       this->Menu->GetWidgetName());  
}

void vtkKWWindow::UnRegister(vtkObject *o)
{
  if (!this->DeletingChildren)
    {
    // delete the children if we are about to be deleted
    if (this->ReferenceCount == this->Views->GetNumberOfItems() + 
        this->Children->GetNumberOfItems() + 1)
      {
      if (!(this->Views->IsItemPresent((vtkKWView *)o) ||
            this->Children->IsItemPresent((vtkKWWidget *)o)))
        {
        vtkKWWidget *child;
        vtkKWView *v;
        
        this->DeletingChildren = 1;
        this->Children->InitTraversal();
        while(child = this->Children->GetNextKWWidget())
          {
          child->SetParent(NULL);
          }
        // deselect if required
        if (this->SelectedView)
          {
          this->SetSelectedView(NULL);
          }
        this->Views->InitTraversal();
        while(v = this->Views->GetNextKWView())
          {
          v->SetParentWindow(NULL);
          }
        this->DeletingChildren = 0;
        }
      }
    }
  
  this->vtkObject::UnRegister(o);
}

void vtkKWWindow::LoadScript()
{
  char *path = NULL;

  this->Script("tk_getOpenFile -title \"Load Script\" -filetypes {{{Tcl Script} {.tcl}}}");
  path = 
    strcpy(new char[strlen(this->Application->GetMainInterp()->result)+1], 
	   this->Application->GetMainInterp()->result);
  if (strlen(path) != 0)
    {
    FILE *fin = fopen(path,"r");
    if (!fin)
      {
      vtkWarningMacro("Unable to open script file!");
      }
    else
      {
      this->LoadScript(path);
      }
    delete [] path;
    }
}

void vtkKWWindow::LoadScript(char *path)
{
  // add this window as a variable
  this->Script("set InitialWindow %s", this->GetTclName());
  this->Script("source {%s}",path);
}

void vtkKWWindow::CreateStatusImage()
{
  int x, y;
  Tk_PhotoHandle photo;
  Tk_PhotoImageBlock block;
  
  block.width = 128;
  block.height = 34;
  block.pixelSize = 3;
  block.pitch = block.width*block.pixelSize;
  block.offset[0] = 0;
  block.offset[1] = 1;
  block.offset[2] = 2;
  block.pixelPtr = new unsigned char [block.pitch*block.height];

  photo = Tk_FindPhoto(this->Application->GetMainInterp(),
		       this->StatusImageName);
  if (!photo)
    {
    vtkWarningMacro("error looking up color ramp image");
    return;
    }
  
  unsigned char *pp = block.pixelPtr;
  float *lp = KITLOGO + 33*128*3;
  for (y = 0; y < 34; y++)
    {
    for (x = 0; x < 128; x++)
      {
      pp[0] = (unsigned char)(*lp*255.0);
      lp++;
      pp[1] = (unsigned char)(*lp*255.0);
      lp++;
      pp[2] = (unsigned char)(*lp*255.0);
      lp++;
      pp += block.pixelSize;
      }
    lp = lp - 2*128*3;
    }
  
  Tk_PhotoPutBlock(photo, &block, 0, 0, block.width, block.height);
  delete [] block.pixelPtr;
}


#ifdef _WIN32
void ReadAValue(HKEY hKey,char *val,char *key, char *adefault)
{
  DWORD dwType, dwSize;
  
  dwType = REG_SZ;
  dwSize = 1023;
  if(RegQueryValueEx(hKey,key, NULL, &dwType, 
                     (BYTE *)val, &dwSize) != ERROR_SUCCESS)
    {
    strcpy(val,adefault);
    }
}
#endif

void vtkKWWindow::AddRecentFilesToMenu(char *key, vtkKWObject *target)
{
#ifdef _WIN32
  int i;
  char fkey[1024];
  char *KeyName[4] = {"File1","File2","File3","File4"};
  char *CmdName[4] = {"File1Cmd","File2Cmd","File3Cmd","File4Cmd"};
  char Cmd[1024];
  
  if (!key)
    {
    sprintf(fkey,"Software\\Kitware\\%s\\MRU",this->GetClassName());
    }
  else
    {
    sprintf(fkey,"Software\\Kitware\\%s\\MRU",key);
    }
  
  HKEY hKey;
  this->NumberOfMRUFiles = 0;
  if(RegOpenKeyEx(HKEY_CURRENT_USER, fkey, 
		  0, KEY_READ, &hKey) != ERROR_SUCCESS)
    {
    return;
    }
  else
    {
    char File[1024];
    
    this->GetMenuFile()->InsertSeparator(
      this->GetMenuFile()->GetIndex("Close") - 1);

    for (i = 0; i < 4; i++)
      {
      ReadAValue(hKey, File, KeyName[i],"");
      ReadAValue(hKey, Cmd, CmdName[i],"Open");
      if (strlen(File) > 1)
        {
        char *cmd = new char [strlen(Cmd) + strlen(File) + 10];
        sprintf(cmd,"%s {%s}",Cmd, File);
        if (strlen(File) > 40)
          {
          File[36] = '.';
          File[37] = '.';
          File[38] = '.';
          File[39] = '\0';
          }
        this->GetMenuFile()->InsertCommand(
          this->GetMenuFile()->GetIndex("Close") - 1,
          File, target, cmd);
        this->NumberOfMRUFiles++;
        delete [] cmd;
        }    
      }
    }
  RegCloseKey(hKey);
  
#endif
}

void vtkKWWindow::AddRecentFile(char *key, char *name,vtkKWObject *target,
                                const char *command)
{
#ifdef _WIN32
  char fkey[1024];
  char File[1024];
  char Cmd[1024];
  
  if (!key)
    {
    sprintf(fkey,"Software\\Kitware\\%s\\MRU",this->GetClassName());
    }
  else
    {
    sprintf(fkey,"Software\\Kitware\\%s\\MRU",key);
    }
  
  HKEY hKey;
  DWORD dwDummy;

  if(RegCreateKeyEx(HKEY_CURRENT_USER, fkey,
		    0, "", REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, 
		    NULL, &hKey, &dwDummy) != ERROR_SUCCESS) 
    {
    return;
    }
  else
    {
    // if this is the same as the current File1 then ignore
    ReadAValue(hKey, File,"File1","");
    if (!strcmp(name,File))
      {
      RegCloseKey(hKey);
      return;
      }
    
    // if this is the first addition
    if (!this->NumberOfMRUFiles)
      {
      this->GetMenuFile()->InsertSeparator(
        this->GetMenuFile()->GetIndex("Close") - 1);
      }
    
    // remove the old entry number 4
    ReadAValue(hKey, File,"File4","");
    if (strlen(File) > 1)
      {
      this->GetMenuFile()->DeleteMenuItem(
        this->GetMenuFile()->GetIndex("Close") - 2);
      this->NumberOfMRUFiles--;
      }
    
    // move the other three down
    ReadAValue(hKey, File,"File3","");
    ReadAValue(hKey, Cmd,"File3Cmd","");
    RegSetValueEx(hKey, "File4", 0, REG_SZ, 
		  (CONST BYTE *)(const char *)File, strlen(File)+1);
    RegSetValueEx(hKey, "File4Cmd", 0, REG_SZ, 
		  (CONST BYTE *)(const char *)Cmd, strlen(Cmd)+1);
    ReadAValue(hKey, File,"File2","");
    ReadAValue(hKey, Cmd,"File2Cmd","");
    RegSetValueEx(hKey, "File3", 0, REG_SZ, 
		  (CONST BYTE *)(const char *)File, strlen(File)+1);
    RegSetValueEx(hKey, "File3Cmd", 0, REG_SZ, 
		  (CONST BYTE *)(const char *)Cmd, strlen(Cmd)+1);
    ReadAValue(hKey, File,"File1","");
    ReadAValue(hKey, Cmd,"File1Cmd","");
    RegSetValueEx(hKey, "File2", 0, REG_SZ, 
		  (CONST BYTE *)(const char *)File, strlen(File)+1);
    RegSetValueEx(hKey, "File2Cmd", 0, REG_SZ, 
		  (CONST BYTE *)(const char *)Cmd, strlen(Cmd)+1);
    RegSetValueEx(hKey, "File1", 0, REG_SZ, 
		  (CONST BYTE *)(const char *)name, 
		  strlen(name)+1);
    RegSetValueEx(hKey, "File1Cmd", 0, REG_SZ, 
		  (CONST BYTE *)(const char *)command, 
                  strlen(command)+1);

    this->NumberOfMRUFiles++;
    // add the new entry
    if (strlen(File) > 1)
      {
      char cmd[1024];
      sprintf(cmd,"%s {%s}",command, name);
      if (strlen(name) > 40)
        {
        char *name2 = new char [strlen(name)+1];
        sprintf(name2,"%s",name);
        name2[36] = '.';
        name2[37] = '.';
        name2[38] = '.';
        name2[39] = '\0';
        this->GetMenuFile()->InsertCommand(
          this->GetFileMenuIndex()+2,name2,target,cmd);
        delete [] name2;
        }
      else
        {
        this->GetMenuFile()->InsertCommand(
          this->GetFileMenuIndex()+2,name,target,cmd);
        }
      }
    else
      {
      char cmd[1024];
      sprintf(cmd,"%s {%s}",command, name);
      if (strlen(name) > 40)
        {
        char *name2 = new char [strlen(name)+1];
        sprintf(name2,"%s",name);
        name2[36] = '.';
        name2[37] = '.';
        name2[38] = '.';
        name2[39] = '\0';
        this->GetMenuFile()->InsertCommand(
          this->GetFileMenuIndex()+2,name2,target,cmd);
        delete [] name2;
        }
      else
        {
        this->GetMenuFile()->InsertCommand(
          this->GetMenuFile()->GetIndex("Close")-1,name,target,cmd);
        }
      }
    }
  RegCloseKey(hKey);
    
#endif
}

int vtkKWWindow::GetFileMenuIndex()
{
  // first find Close
  int clidx = this->GetMenuFile()->GetIndex("Close");  
  if (this->NumberOfMRUFiles > 0)
    {
    return clidx - this->NumberOfMRUFiles - 2;
    }
  return clidx - 1;  
}

void vtkKWWindow::SerializeRevision(ostream& os, vtkIndent indent)
{
  vtkKWWidget::SerializeRevision(os,indent);
  os << indent << "vtkKWWindow ";
  this->ExtractRevision(os,"$Revision: 1.17 $");
}
