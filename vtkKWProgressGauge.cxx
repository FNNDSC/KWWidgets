/*=========================================================================

  Module:    $RCSfile: vtkKWProgressGauge.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWProgressGauge.h"

#include "vtkKWApplication.h"
#include "vtkObjectFactory.h"
#include "vtkString.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWProgressGauge );
vtkCxxRevisionMacro(vtkKWProgressGauge, "$Revision: 1.20 $");

int vtkKWProgressGaugeCommand(ClientData cd, Tcl_Interp *interp,
                              int argc, char *argv[]);


vtkKWProgressGauge::vtkKWProgressGauge()
{ 
  this->CommandFunction = vtkKWProgressGaugeCommand;
  this->Length = 100;
  this->Height = 20;
  this->Value = 0;
  this->BarColor = vtkString::Duplicate("blue");
  this->BackgroundColor = 0;
}


vtkKWProgressGauge::~vtkKWProgressGauge()
{
  this->SetBackgroundColor(0);
  delete [] this->BarColor;
  this->BarColor = NULL;
}

void vtkKWProgressGauge::Create(vtkKWApplication *app, const char *args)
{
  const char *wname;

  // Set the application

  if (this->IsCreated())
    {
    vtkErrorMacro("CheckButton already created");
    return;
    }

  this->SetApplication(app);

  // create the top level
  wname = this->GetWidgetName();
  this->Script("frame %s", wname);
  this->Script("canvas %s.display -borderwidth 0  -highlightthickness 0 -width %d -height %d %s",
               wname, this->Length, this->Height, args);
  this->Script("pack %s.display -expand yes", wname);
  // initialize the bar color to the background so it does
  // not show up until used
  this->Script(
    "%s.display create rectangle 0 0 0 0 -outline \"\"  -tags bar", 
               wname );
  this->Script(
    "%s.display create text [expr 0.5 * %d] [expr 0.5 * %d] "
    "-anchor c -text \"\" -tags value",
    wname, this->Length, this->Height);

  // Update enable state

  this->UpdateEnableState();
}

void vtkKWProgressGauge::SetValue(int value)
{
  if (!this->IsMapped())
    {
    return;
    }
  int enabled = this->Enabled;
  if ( !enabled )
    {
    this->EnabledOn();
    }

  const char* wname = this->GetWidgetName();

  this->Value = value;

  if(this->Value < 0)
    {
    this->Value = 0;
    }
  if(this->Value > 100)
    {
    this->Value = 100;
    }

  ostrstream tk_cmd;

  if(this->Value == 0)
    {
    // if the Value is 0, set the text to nothing and the color
    // of the bar to the background (0 0 0 0) rectangles show
    // up as a pixel...
    
    tk_cmd << wname << ".display itemconfigure value -text {}" << endl
           << wname << ".display coords bar 0 0 0 0" << endl
           << wname << ".display itemconfigure bar -fill {}" << endl;
    }
  else
    {
    // if the Value is not 0 then use the BarColor for the bar

    tk_cmd << wname << ".display itemconfigure bar -fill " 
           << this->BarColor << endl;

    // Set the text to the percent done

    const char* textcolor = "-fill black";
    if(this->Value > 50)
      {
      textcolor = "-fill white";
      }
    
    char buffer[5];
    sprintf(buffer, "%3.0d", this->Value);

    tk_cmd << wname << ".display itemconfigure value -text {" << buffer 
           << "%%} " << textcolor << endl;

    // Draw the correct rectangle

    tk_cmd << wname << ".display coords bar 0 0 [expr 0.01 * " << this->Value 
           << " * [winfo width " << wname << ".display]] [winfo height "
           << wname << ".display]" << endl;
    }

  // Do an update

  tk_cmd << "update idletasks" << endl;

  tk_cmd << ends;
  this->Script(tk_cmd.str());
  tk_cmd.rdbuf()->freeze(0);

  if ( !enabled )
    {
    this->EnabledOff();
    }
}

//----------------------------------------------------------------------------
void vtkKWProgressGauge::SetHeight(int height)
{
  if (this->Height == height)
    {
    return;
    }

  this->Height = height;
  this->Modified();

  //  Change gauge height, move text

  if (this->IsCreated())
    {
    this->Script("%s.display config -height %d", 
                 this->GetWidgetName(), this->Height);

    this->Script("%s.display coords value [expr 0.5 * %d] [expr 0.5 * %d]", 
                 this->GetWidgetName(), this->Length, this->Height);
    }
}

//----------------------------------------------------------------------------
void vtkKWProgressGauge::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "BackgroundColor: " 
     << (this->BackgroundColor?this->BackgroundColor:"none") << endl;
  os << indent << "BarColor: " << (this->BarColor?this->BarColor:"none") 
     << endl;
  os << indent << "Height: " << this->GetHeight() << endl;
  os << indent << "Length: " << this->GetLength() << endl;
}

