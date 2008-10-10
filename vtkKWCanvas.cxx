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
#include "vtkKWCanvas.h"
#include "vtkObjectFactory.h"
#include "vtkKWOptions.h"
#include "vtkColorTransferFunction.h"

#include <vtksys/SystemTools.hxx>
#include <vtksys/ios/sstream>
#include <vtksys/stl/string>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWCanvas );
vtkCxxRevisionMacro(vtkKWCanvas, "$Revision: 1.15 $");

//----------------------------------------------------------------------------
void vtkKWCanvas::CreateWidget()
{
  // Call the superclass to create the widget and set the appropriate flags

  if (!vtkKWWidget::CreateSpecificTkWidget(this, 
        "canvas", "-highlightthickness 0"))
    {
    vtkErrorMacro("Failed creating widget " << this->GetClassName());
    return;
    }
}

//----------------------------------------------------------------------------
void vtkKWCanvas::SetWidth(int width)
{
  this->SetConfigurationOptionAsInt("-width", width);
}

//----------------------------------------------------------------------------
int vtkKWCanvas::GetWidth()
{
  return this->GetConfigurationOptionAsInt("-width");
}

//----------------------------------------------------------------------------
void vtkKWCanvas::SetHeight(int height)
{
  this->SetConfigurationOptionAsInt("-height", height);
}

//----------------------------------------------------------------------------
int vtkKWCanvas::GetHeight()
{
  return this->GetConfigurationOptionAsInt("-height");
}

//----------------------------------------------------------------------------
void vtkKWCanvas::GetBackgroundColor(double *r, double *g, double *b)
{
  this->GetConfigurationOptionAsColor("-background", r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWCanvas::GetBackgroundColor()
{
  return this->GetConfigurationOptionAsColor("-background");
}

//----------------------------------------------------------------------------
void vtkKWCanvas::SetBackgroundColor(double r, double g, double b)
{
  this->SetConfigurationOptionAsColor("-background", r, g, b);
}

//----------------------------------------------------------------------------
void vtkKWCanvas::SetHighlightThickness(int width)
{
  this->SetConfigurationOptionAsInt("-highlightthickness", width);
}

//----------------------------------------------------------------------------
int vtkKWCanvas::GetHighlightThickness()
{
  return this->GetConfigurationOptionAsInt("-highlightthickness");
}

//----------------------------------------------------------------------------
void vtkKWCanvas::SetBorderWidth(int width)
{
  this->SetConfigurationOptionAsInt("-bd", width);
}

//----------------------------------------------------------------------------
int vtkKWCanvas::GetBorderWidth()
{
  return this->GetConfigurationOptionAsInt("-bd");
}

//----------------------------------------------------------------------------
void vtkKWCanvas::SetRelief(int relief)
{
  this->SetConfigurationOption(
    "-relief", vtkKWOptions::GetReliefAsTkOptionValue(relief));
}

void vtkKWCanvas::SetReliefToRaised()     
{ 
  this->SetRelief(vtkKWOptions::ReliefRaised); 
};
void vtkKWCanvas::SetReliefToSunken() 
{ 
  this->SetRelief(vtkKWOptions::ReliefSunken); 
};
void vtkKWCanvas::SetReliefToFlat() 
{ 
  this->SetRelief(vtkKWOptions::ReliefFlat); 
};
void vtkKWCanvas::SetReliefToRidge() 
{ 
  this->SetRelief(vtkKWOptions::ReliefRidge); 
};
void vtkKWCanvas::SetReliefToSolid() 
{ 
  this->SetRelief(vtkKWOptions::ReliefSolid); 
};
void vtkKWCanvas::SetReliefToGroove() 
{ 
  this->SetRelief(vtkKWOptions::ReliefGroove); 
};

//----------------------------------------------------------------------------
int vtkKWCanvas::GetRelief()
{
  return vtkKWOptions::GetReliefFromTkOptionValue(
    this->GetConfigurationOption("-relief"));
}

//----------------------------------------------------------------------------
void vtkKWCanvas::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->SetState(this->GetEnabled());
}

//----------------------------------------------------------------------------
int vtkKWCanvas::AddGradient(vtkColorTransferFunction *ctf, 
                             int x1, int y1, int x2, int y2,
                             const char *tag,
                             int horizontal)
{
  if (!ctf || !this->IsCreated())
    {
    return 0;
    }

  int temp;
  if (x1 > x2)
    {
    temp = x1;
    x1 = x2;
    x2 = temp;
    }
  if (y1 > y2)
    {
    temp = y1;
    y1 = y2;
    y2 = temp;
    }

  int width = x2 - x1 + 1;
  int height = y2 - y1 + 1;

  const unsigned char *rgb_table = 
    ctf->GetTable(ctf->GetRange()[0], ctf->GetRange()[1], 
                  horizontal ? width : height);

  vtksys_ios::ostringstream tk_cmd;

  const char *wname = this->GetWidgetName();
  char rgb[10];

  vtksys_stl::string extra;
  if (tag)
    {
    extra = extra + " -tags {" + tag + "} ";
    }
  extra += " -fill #";

  if (horizontal)
    {
    while (x1 <= x2)
      {
      sprintf(rgb, "%02x%02x%02x", 
              (int)rgb_table[0], (int)rgb_table[1], (int)rgb_table[2]);
      rgb_table += 3;
      tk_cmd << wname << " create line " 
             << x1 << " " << y1 << " " << x1 << " " << y2 
             << extra.c_str() << rgb << endl;
      ++x1;
      }
    }
  else
    {
    while (y1 <= y2)
      {
      sprintf(rgb, "%02x%02x%02x", 
              (int)rgb_table[0], (int)rgb_table[1], (int)rgb_table[2]);
      rgb_table += 3;
      tk_cmd << wname << " create line " 
             << x1 << " " << y1 << " " << x2 << " " << y1
             << extra.c_str() << rgb << endl;
      ++y1;
      }
    }

  this->Script(tk_cmd.str().c_str());
  
  return 1;
}

//----------------------------------------------------------------------------
int vtkKWCanvas::AddHorizontalGradient(vtkColorTransferFunction *ctf, 
                                       int x1, int y1, int x2, int y2,
                                       const char *tag)
{
  return this->AddGradient(ctf, x1, y1, x2, y2, tag, 1);
}

//----------------------------------------------------------------------------
int vtkKWCanvas::AddHorizontalRGBGradient(double r1, double g1, double b1, 
                                          double r2, double g2, double b2, 
                                          int x1, int y1, int x2, int y2,
                                          const char *tag)
{
  vtkColorTransferFunction *ctf = vtkColorTransferFunction::New();
  ctf->SetColorSpaceToRGB();
  ctf->AddRGBPoint(0.0, r1, g1, b1);
  ctf->AddRGBPoint(1.0, r2, g2, b2);
  int res = this->AddHorizontalGradient(ctf, x1, y1, x2, y2, tag);
  ctf->Delete();
  return res;
}

//----------------------------------------------------------------------------
int vtkKWCanvas::AddVerticalGradient(vtkColorTransferFunction *ctf, 
                                       int x1, int y1, int x2, int y2,
                                       const char *tag)
{
  return this->AddGradient(ctf, x1, y1, x2, y2, tag, 0);
}

//----------------------------------------------------------------------------
int vtkKWCanvas::AddVerticalRGBGradient(double r1, double g1, double b1, 
                                          double r2, double g2, double b2, 
                                          int x1, int y1, int x2, int y2,
                                          const char *tag)
{
  vtkColorTransferFunction *ctf = vtkColorTransferFunction::New();
  ctf->SetColorSpaceToRGB();
  ctf->AddRGBPoint(0.0, r1, g1, b1);
  ctf->AddRGBPoint(1.0, r2, g2, b2);
  int res = this->AddVerticalGradient(ctf, x1, y1, x2, y2, tag);
  ctf->Delete();
  return res;
}

//----------------------------------------------------------------------------
void vtkKWCanvas::SetCanvasBinding(
  const char *tag, const char *event, vtkObject *object, const char *method)
{
  if (this->IsCreated())
    {
    char *command = NULL;
    this->SetObjectMethodCommand(&command, object, method);
    this->Script("%s bind %s %s {%s}", 
                 this->GetWidgetName(), tag, event, command);
    delete [] command;
    }
}

//----------------------------------------------------------------------------
void vtkKWCanvas::SetCanvasBinding(
  const char *tag, const char *event, const char *command)
{
  this->SetCanvasBinding(tag, event, NULL, command);
}

//----------------------------------------------------------------------------
const char* vtkKWCanvas::GetCanvasBinding(const char *tag, const char *event)
{
  if (this->IsCreated())
    {
    return this->Script("%s bind %s %s", this->GetWidgetName(), tag, event);
    }
  return NULL;
}

//----------------------------------------------------------------------------
void vtkKWCanvas::AddCanvasBinding(
  const char *tag, const char *event, vtkObject *object, const char *method)
{
  if (this->IsCreated())
    {
    char *command = NULL;
    this->SetObjectMethodCommand(&command, object, method);
    this->Script("%s bind %s %s {+%s}", 
                 this->GetWidgetName(), tag, event, command);
    delete [] command;
    }
}

//----------------------------------------------------------------------------
void vtkKWCanvas::AddCanvasBinding(
  const char *tag, const char *event, const char *command)
{
  this->AddCanvasBinding(tag, event, NULL, command);
}

//----------------------------------------------------------------------------
void vtkKWCanvas::RemoveCanvasBinding(
  const char *tag, const char *event, vtkObject *object, const char *method)
{
  if (this->IsCreated())
    {
    char *command = NULL;
    this->SetObjectMethodCommand(&command, object, method);

    // Retrieve the bindings, remove the command, re-assign

    vtksys_stl::string bindings(
      this->Script("%s bind %s %s", this->GetWidgetName(), tag, event));

    vtksys::SystemTools::ReplaceString(bindings, command, "");
  
    this->Script(
      "%s bind %s %s {%s}", 
      this->GetWidgetName(), tag, event, bindings.c_str());
    delete [] command;
    }
}

//----------------------------------------------------------------------------
void vtkKWCanvas::RemoveCanvasBinding(const char *tag, const char *event)
{
  if (this->IsCreated())
    {
    this->Script("%s bind %s %s {}", 
                 this->GetWidgetName(), tag, event);
    }
}

//----------------------------------------------------------------------------
void vtkKWCanvas::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
