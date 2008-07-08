/*=========================================================================

  Module:    $RCSfile: vtkKWColorSpectrumWidget.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWColorSpectrumWidget.h"

#include "vtkObjectFactory.h"
#include "vtkKWLabel.h"
#include "vtkKWRadioButtonSet.h"
#include "vtkKWRadioButton.h"
#include "vtkMath.h"
#include "vtkKWCanvas.h"
#include "vtkKWTkUtilities.h"

#include <vtksys/stl/string>
#include <vtksys/ios/sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWColorSpectrumWidget );
vtkCxxRevisionMacro(vtkKWColorSpectrumWidget, "$Revision: 1.3 $");

#define VTK_KW_COLOR_SPECTRUM_WIDGET_FIXED_FONT "fixed"
#define VTK_KW_COLOR_SPECTRUM_WIDGET_FIXED_FONT_85 "TkDefaultFont"

#define VTK_KW_COLOR_SPECTRUM_WIDGET_IMAGE_TAG   "image"
#define VTK_KW_COLOR_SPECTRUM_WIDGET_CURSOR_TAG  "sel"
#define VTK_KW_COLOR_SPECTRUM_WIDGET_CANVAS_SIZE 256

//----------------------------------------------------------------------------
class vtkKWColorSpectrumWidgetInternals
{
public:
  double PreviousPickColor[3]; // track the color that was picked

  double PreviousFixedAxisValue; // track the color that was rendered
  int PreviousFixedAxis;         // in the canvas to avoid re-rendering
};

//----------------------------------------------------------------------------
vtkKWColorSpectrumWidget::vtkKWColorSpectrumWidget()
{
  this->Internals = new vtkKWColorSpectrumWidgetInternals;
  this->Internals->PreviousFixedAxisValue = -1; /* unitialized */
  this->Internals->PreviousFixedAxis = -1;

  this->ColorCanvas       = NULL;
  this->FixedAxisRadioButtonSet = NULL;

  this->FixedAxis   = vtkKWColorSpectrumWidget::FixedAxisV;
  this->FixedAxisSelectorVisibility = 1;

  this->InternalColorRGB[0] = -1; /* unitialized */
  this->InternalColorRGB[1] = -1;
  this->InternalColorRGB[2] = -1;
  this->InternalColorHSV[0] = -1;
  this->InternalColorHSV[1] = -1;
  this->InternalColorHSV[2] = -1;

  this->SetColorAsHSV(0.5, 1.0, 1.0);

  this->ColorChangedCommand  = NULL;
  this->ColorChangingCommand = NULL;
}

//----------------------------------------------------------------------------
vtkKWColorSpectrumWidget::~vtkKWColorSpectrumWidget()
{
  delete this->Internals;
  this->Internals = NULL;

  if (this->ColorCanvas)
    {
    this->ColorCanvas->Delete();
    this->ColorCanvas = NULL;
    }

  if (this->FixedAxisRadioButtonSet)
    {
    this->FixedAxisRadioButtonSet->Delete();
    this->FixedAxisRadioButtonSet = NULL;
    }

  if (this->ColorChangedCommand)
    {
    delete [] this->ColorChangedCommand;
    this->ColorChangedCommand = NULL;
    }

  if (this->ColorChangingCommand)
    {
    delete [] this->ColorChangingCommand;
    this->ColorChangingCommand = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::CreateWidget()
{
  // Check if already created
  
  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }
  
  // Call the superclass to create the whole widget
  
  this->Superclass::CreateWidget();

  ostrstream tk_cmd;
  
  double frame_rgb[3], frame_hsv[3];
  this->GetBackgroundColor(&frame_rgb[0], &frame_rgb[1], &frame_rgb[2]);
  vtkMath::RGBToHSV(frame_rgb, frame_hsv);
  frame_hsv[2] = frame_hsv[2] * 0.5;
  vtkMath::HSVToRGB(frame_hsv, frame_rgb);

  // --------------------------------------------------------------
  // Create the Hue/Sat canvas

  if (!this->ColorCanvas)
    {
    this->ColorCanvas = vtkKWCanvas::New();
    }
  this->ColorCanvas->SetParent(this);
  this->ColorCanvas->Create();
  this->ColorCanvas->SetReliefToFlat();
  this->ColorCanvas->SetHeight(VTK_KW_COLOR_SPECTRUM_WIDGET_CANVAS_SIZE);
  this->ColorCanvas->SetWidth(VTK_KW_COLOR_SPECTRUM_WIDGET_CANVAS_SIZE);
  this->ColorCanvas->SetReliefToSolid();
  this->ColorCanvas->SetBorderWidth(0);
  this->ColorCanvas->SetConfigurationOption("-cursor", "target"); // "circle"

  const char *canv = this->ColorCanvas->GetWidgetName();

  vtksys_ios::ostringstream img_name;
  img_name << this->ColorCanvas->GetWidgetName() 
           << "." << VTK_KW_COLOR_SPECTRUM_WIDGET_IMAGE_TAG;

  tk_cmd << "image create photo " << img_name.str().c_str() 
         << " -width " << VTK_KW_COLOR_SPECTRUM_WIDGET_CANVAS_SIZE
         << " -height " << VTK_KW_COLOR_SPECTRUM_WIDGET_CANVAS_SIZE 
         << endl;

  tk_cmd << canv << " create image 0 0 -anchor nw "
         << " -image " << img_name.str().c_str()
         << " -tag " << VTK_KW_COLOR_SPECTRUM_WIDGET_IMAGE_TAG
         << endl;

  tk_cmd << canv << " create oval 0 0 0 0 "
         << " -tag " << VTK_KW_COLOR_SPECTRUM_WIDGET_CURSOR_TAG
         << endl;

  // --------------------------------------------------------------
  // Fixed Axis choice

  if (!this->FixedAxisRadioButtonSet)
    {
    this->FixedAxisRadioButtonSet = vtkKWRadioButtonSet::New();
    }
  if (!this->FixedAxisRadioButtonSet->IsCreated())
    {
    this->FixedAxisRadioButtonSet->SetParent(this);
    this->FixedAxisRadioButtonSet->Create();
    this->FixedAxisRadioButtonSet->ExpandWidgetsOn();
    //this->FixedAxisRadioButtonSet->PackHorizontallyOn();
    
    typedef struct
    {
      int value;
      const char *label;
      const char *callback;
    } entry_t;

    entry_t entries[] =
      {
        {vtkKWColorSpectrumWidget::FixedAxisR, "R", "SetFixedAxisToR"},
        {vtkKWColorSpectrumWidget::FixedAxisG, "G", "SetFixedAxisToG"},
        {vtkKWColorSpectrumWidget::FixedAxisB, "B", "SetFixedAxisToB"},
        {vtkKWColorSpectrumWidget::FixedAxisH, "H", "SetFixedAxisToH"},
        {vtkKWColorSpectrumWidget::FixedAxisS, "S", "SetFixedAxisToS"},
        {vtkKWColorSpectrumWidget::FixedAxisV, "V", "SetFixedAxisToV"},
      };

    int tcl_major = 0, tcl_minor = 0, tcl_patch_level = 0;
    Tcl_GetVersion(&tcl_major, &tcl_minor, &tcl_patch_level, NULL);

    vtksys_stl::string font("{");
    font += (tcl_major < 8 || (tcl_major == 8 && tcl_minor < 5)) 
      ? VTK_KW_COLOR_SPECTRUM_WIDGET_FIXED_FONT 
      : VTK_KW_COLOR_SPECTRUM_WIDGET_FIXED_FONT_85;
    font += "} 7";

    for (int i = 0; i < sizeof(entries) / sizeof(entries[0]); i++)
      {
      entry_t entry = entries[i];
      vtkKWRadioButton *radiob = 
        this->FixedAxisRadioButtonSet->AddWidget(entry.value);
      radiob->SetText(entry.label);
      radiob->SetCommand(this, entry.callback);
      radiob->IndicatorVisibilityOff();
      radiob->SetFont(font.c_str());
      radiob->SetOffReliefToFlat();
      radiob->SetBorderWidth(0);
      radiob->SetSelectColor(frame_rgb);
      }

    this->FixedAxisRadioButtonSet->GetWidget(this->GetFixedAxis())->Select();
    }

  tk_cmd << ends;
  this->Script(tk_cmd.str());
  tk_cmd.rdbuf()->freeze(0);
  
  this->Pack();

  this->UpdateColorCanvas();
  this->UpdateColorCursor();

  this->AddBindings();
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::Pack()
{
  if (!this->IsCreated())
    {
    return;
    }
  
  this->UnpackChildren();
  
  ostrstream tk_cmd;
  
  if (this->FixedAxisSelectorVisibility)
    {
    tk_cmd << "pack " << this->FixedAxisRadioButtonSet->GetWidgetName() 
           << " -side left -anchor nw -expand n -fill none" << endl;
    this->ColorCanvas->SetBorderWidth(2);
    vtkKWRadioButton *radiob = this->FixedAxisRadioButtonSet->GetWidget(
      vtkKWColorSpectrumWidget::FixedAxisR);
    this->ColorCanvas->SetBackgroundColor(radiob->GetBackgroundColor());
    }
  else
    {
    this->ColorCanvas->SetBorderWidth(0);
    this->ColorCanvas->SetBackgroundColor(this->GetBackgroundColor());
    }
  
  tk_cmd << "pack " << this->ColorCanvas->GetWidgetName() 
         << " -side left -anchor nw -expand n -fill both" << endl;
  
  tk_cmd << ends;
  this->Script(tk_cmd.str());
  tk_cmd.rdbuf()->freeze(0);
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::SetColorAsRGB(double r, double g, double b)
{
  double range[2] = {0.0, 1.0};
  vtkMath::ClampValue(&r, range);
  vtkMath::ClampValue(&g, range);
  vtkMath::ClampValue(&b, range);

  double current[3];
  this->GetColorAsRGB(current);
  if (r == current[0] && g == current[1] && b == current[2])
    {
    return;
    }
  
  this->SetInternalColorRGB(r, g, b);

  double h, s, v;
  vtkMath::RGBToHSV(r, g, b, &h, &s, &v);
  this->SetInternalColorHSV(h, s, v);

  this->UpdateColorCanvas();
  this->UpdateColorCursor();
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::SetColorAsRGB(double rgb[3])
{
  this->SetColorAsRGB(rgb[0], rgb[1], rgb[2]);
}

//----------------------------------------------------------------------------
double* vtkKWColorSpectrumWidget::GetColorAsRGB()
{
  return this->GetInternalColorRGB();
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::GetColorAsRGB(
  double &_arg1, double &_arg2, double &_arg3)
{
  this->GetInternalColorRGB(_arg1, _arg2, _arg3);
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::GetColorAsRGB(double _arg[3])
{
  this->GetColorAsRGB(_arg[0], _arg[1], _arg[2]);
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::SetInternalColorRGB(
  double r, double g, double b)
{
  double range[2] = {0.0, 1.0};
  vtkMath::ClampValue(&r, range);
  vtkMath::ClampValue(&g, range);
  vtkMath::ClampValue(&b, range);

  if (r == this->InternalColorRGB[0] &&
      g == this->InternalColorRGB[1] &&
      b == this->InternalColorRGB[2])
    {
    return;
    }

  this->InternalColorRGB[0] = r;
  this->InternalColorRGB[1] = g;
  this->InternalColorRGB[2] = b;

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::SetInternalColorRGB(double rgb[3])
{
  this->SetInternalColorRGB(rgb[0], rgb[1], rgb[2]);
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::SetColorAsHSV(double h, double s, double v)
{
  double range[2] = {0.0, 1.0};
  vtkMath::ClampValue(&h, range);
  vtkMath::ClampValue(&s, range);
  vtkMath::ClampValue(&v, range);

  double current[3];
  this->GetColorAsHSV(current);
  if (h == current[0] && s == current[1] && v == current[2])
    {
    return;
    }

  this->SetInternalColorHSV(h, s, v);

  double r, g, b;
  vtkMath::HSVToRGB(h, s, v, &r, &g, &b);
  this->SetInternalColorRGB(r, g, b);

  this->UpdateColorCanvas();
  this->UpdateColorCursor();
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::SetColorAsHSV(double rgb[3])
{ 
  this->SetColorAsHSV(rgb[0], rgb[1], rgb[2]); 
}

//----------------------------------------------------------------------------
double* vtkKWColorSpectrumWidget::GetColorAsHSV()
{
  return this->GetInternalColorHSV();
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::GetColorAsHSV(
  double &_arg1, double &_arg2, double &_arg3)
{
  this->GetInternalColorHSV(_arg1, _arg2, _arg3);
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::GetColorAsHSV(double _arg[3])
{ 
  this->GetColorAsHSV(_arg[0], _arg[1], _arg[2]); 
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::SetInternalColorHSV(
  double h, double s, double v)
{
  double range[2] = {0.0, 1.0};
  vtkMath::ClampValue(&h, range);
  vtkMath::ClampValue(&s, range);
  vtkMath::ClampValue(&v, range);

  if (h == this->InternalColorHSV[0] &&
      s == this->InternalColorHSV[1] &&
      v == this->InternalColorHSV[2])
    {
    return;
    }

  this->InternalColorHSV[0] = h;
  this->InternalColorHSV[1] = s;
  this->InternalColorHSV[2] = v;

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::SetInternalColorHSV(double hsv[3])
{ 
  this->SetInternalColorHSV(hsv[0], hsv[1], hsv[2]); 
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::SetFixedAxis(int arg)
{
  if (arg < vtkKWColorSpectrumWidget::FixedAxisR)
    {
    arg = vtkKWColorSpectrumWidget::FixedAxisR;
    }
  else if (arg > vtkKWColorSpectrumWidget::FixedAxisV)
    {
    arg = vtkKWColorSpectrumWidget::FixedAxisV;
    }

  if (this->FixedAxis == arg)
    {
    return;
    }

  this->FixedAxis = arg;

  this->Modified();

  this->UpdateColorCanvas();
  this->UpdateColorCursor();
}

void vtkKWColorSpectrumWidget::SetFixedAxisToR()
{ 
  this->SetFixedAxis(vtkKWColorSpectrumWidget::FixedAxisR);
}

void vtkKWColorSpectrumWidget::SetFixedAxisToG()
{ 
  this->SetFixedAxis(vtkKWColorSpectrumWidget::FixedAxisG);
}

void vtkKWColorSpectrumWidget::SetFixedAxisToB()
{ 
  this->SetFixedAxis(vtkKWColorSpectrumWidget::FixedAxisB);
}

void vtkKWColorSpectrumWidget::SetFixedAxisToH()
{ 
  this->SetFixedAxis(vtkKWColorSpectrumWidget::FixedAxisH);
}

void vtkKWColorSpectrumWidget::SetFixedAxisToS()
{ 
  this->SetFixedAxis(vtkKWColorSpectrumWidget::FixedAxisS);
}

void vtkKWColorSpectrumWidget::SetFixedAxisToV()
{ 
  this->SetFixedAxis(vtkKWColorSpectrumWidget::FixedAxisV);
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::SetFixedAxisSelectorVisibility(int arg)
{
  if (this->FixedAxisSelectorVisibility == arg)
    {
    return;
    }

  this->FixedAxisSelectorVisibility = arg;
  this->Modified();

  this->Pack();
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::UpdateColorCanvas()
{
  if (!this->ColorCanvas || !this->ColorCanvas->IsCreated())
    {
    return;
    }

  const int size = VTK_KW_COLOR_SPECTRUM_WIDGET_CANVAS_SIZE;
  unsigned char img_buffer[size * size * 3];
  
  double h, s, v;
  double r, g, b;

  double inc, one_s;

  // Should be (size - 1), but precision issues make that the final value
  // which should be 1.0, is very slighly higher, triggering the next
  // swtich/case choice and a wrong value. Let's fix that. Visually the
  // difference won't be noticeable, and the right value will be used
  // during interaction anyway.

  inc = 1.0 / (double)size; 

  const double onehalf = 0.5;
  const double onethird = 1.0 / 3.0;
  const double onesixth = 1.0 / 6.0;
  const double onesixthinv = 6.0;
  const double twothird = 2.0 / 3.0;
  const double fivesixth = 5.0 / 6.0;

  unsigned char *img_ptr = img_buffer;

  int i, j;

  switch (this->FixedAxis)
    {
    // over R, G vertically (1.0 -> 0.0), B horizontally (0.0 -> 1.0)
    case vtkKWColorSpectrumWidget::FixedAxisR: 
      if (this->Internals->PreviousFixedAxis == this->FixedAxis &&
          this->Internals->PreviousFixedAxisValue == this->InternalColorRGB[0])
        {
        return;
        }
      this->Internals->PreviousFixedAxisValue = this->InternalColorRGB[0];
      r = this->InternalColorRGB[0];
      g = 1.0;
      for (i = 0; i < size; i++)
        {
        b = 0.0;
        for (j = 0; j < size; j++)
          {
          *img_ptr++ = (unsigned char)(255.0 * r);
          *img_ptr++ = (unsigned char)(255.0 * g);
          *img_ptr++ = (unsigned char)(255.0 * b);
          b += inc;
          }
        g -= inc;
        }
      break;

    // over G, R vertically (1.0 -> 0.0), B horizontally (0.0 -> 1.0)
    case vtkKWColorSpectrumWidget::FixedAxisG: 
      if (this->Internals->PreviousFixedAxis == this->FixedAxis &&
          this->Internals->PreviousFixedAxisValue == this->InternalColorRGB[1])
        {
        return;
        }
      this->Internals->PreviousFixedAxisValue = this->InternalColorRGB[1];
      g = this->InternalColorRGB[1];
      r = 1.0;
      for (i = 0; i < size; i++)
        {
        b = 0.0;
        for (j = 0; j < size; j++)
          {
          *img_ptr++ = (unsigned char)(255.0 * r);
          *img_ptr++ = (unsigned char)(255.0 * g);
          *img_ptr++ = (unsigned char)(255.0 * b);
          b += inc;
          }
        r -= inc;
        }
      break;

    // over B, G vertically (1.0 -> 0.0), R horizontally (0.0 -> 1.0)
    case vtkKWColorSpectrumWidget::FixedAxisB:
      if (this->Internals->PreviousFixedAxis == this->FixedAxis &&
          this->Internals->PreviousFixedAxisValue == this->InternalColorRGB[2])
        {
        return;
        }
      this->Internals->PreviousFixedAxisValue = this->InternalColorRGB[2];
      b = this->InternalColorRGB[2];
      g = 1.0;
      for (i = 0; i < size; i++)
        {
        r = 0.0;
        for (j = 0; j < size; j++)
          {
          *img_ptr++ = (unsigned char)(255.0 * r);
          *img_ptr++ = (unsigned char)(255.0 * g);
          *img_ptr++ = (unsigned char)(255.0 * b);
          r += inc;
          }
        g -= inc;
        }
      break;

    // over H, V vertically (1.0 -> 0.0), S horizontally (0.0 -> 1.0)
    case vtkKWColorSpectrumWidget::FixedAxisH:
      if (this->Internals->PreviousFixedAxis == this->FixedAxis &&
          this->Internals->PreviousFixedAxisValue == this->InternalColorHSV[0])
        {
        return;
        }
      this->Internals->PreviousFixedAxisValue = this->InternalColorHSV[0];
      h = this->InternalColorHSV[0];
      v = 1.0;
      for (i = 0; i < size; i++)
        {
        s = 0.0;
        for (j = 0; j < size; j++)
          {
          one_s = 1.0 - s;
          if (h > onesixth && h <= onethird) // green/red
            {
            r = (s*((onethird-h)*onesixthinv)+one_s)*v; g = v; b = one_s*v;
            }
          else if (h > onethird && h <= onehalf) // green/blue
            {
            r = one_s*v; g = v; b = (s*((h-onethird)*onesixthinv)+one_s)*v; 
            }
          else if (h > onehalf && h <= twothird) // blue/green
            {
            r = one_s*v; g = (s*((twothird-h)*onesixthinv)+one_s)*v; b = v; 
            }
          else if (h > twothird && h <= fivesixth) // blue/red
            {
            r = (s*((h-twothird)*onesixthinv)+one_s)*v; g = one_s*v; b = v; 
            }
          else if (h > fivesixth && h <= 1.0) // red/blue
            {
            r = v; g = one_s*v; b = (s*((1.0-h)*onesixthinv)+one_s)*v; 
            }
          else // red/green
            {
            r = v; g = (s*(h*onesixthinv)+one_s)*v; b = one_s*v;
            }
          *img_ptr++ = (unsigned char)(255.0 * r);
          *img_ptr++ = (unsigned char)(255.0 * g);
          *img_ptr++ = (unsigned char)(255.0 * b);
          s += inc;
          }
        v -= inc;
        }
      break;

    // over S, V vertically (1.0 -> 0.0), H horizontally (0.0 -> 1.0)
    case vtkKWColorSpectrumWidget::FixedAxisS:
      if (this->Internals->PreviousFixedAxis == this->FixedAxis &&
          this->Internals->PreviousFixedAxisValue == this->InternalColorHSV[1])
        {
        return;
        }
      this->Internals->PreviousFixedAxisValue = this->InternalColorHSV[1];
      s = this->InternalColorHSV[1];
      one_s = 1.0 - s;
      v = 1.0;
      for (i = 0; i < size; i++)
        {
        h = 0.0;
        for (j = 0; j < size; j++)
          {
          if (h > onesixth && h <= onethird) // green/red
            {
            r = (s*((onethird-h)*onesixthinv)+one_s)*v; g = v; b = one_s*v;
            }
          else if (h > onethird && h <= onehalf) // green/blue
            {
            r = one_s*v; g = v; b = (s*((h-onethird)*onesixthinv)+one_s)*v; 
            }
          else if (h > onehalf && h <= twothird) // blue/green
            {
            r = one_s*v; g = (s*((twothird-h)*onesixthinv)+one_s)*v; b = v; 
            }
          else if (h > twothird && h <= fivesixth) // blue/red
            {
            r = (s*((h-twothird)*onesixthinv)+one_s)*v; g = one_s*v; b = v; 
            }
          else if (h > fivesixth && h <= 1.0) // red/blue
            {
            r = v; g = one_s*v; b = (s*((1.0-h)*onesixthinv)+one_s)*v; 
            }
          else // red/green
            {
            r = v; g = (s*(h*onesixthinv)+one_s)*v; b = one_s*v;
            }
          *img_ptr++ = (unsigned char)(255.0 * r);
          *img_ptr++ = (unsigned char)(255.0 * g);
          *img_ptr++ = (unsigned char)(255.0 * b);
          h += inc;
          }
        v -= inc;
        }
      break;

    // over V, S vertically (1.0 -> 0.0), H horizontally (0.0 -> 1.0)
    case vtkKWColorSpectrumWidget::FixedAxisV:
      if (this->Internals->PreviousFixedAxis == this->FixedAxis &&
          this->Internals->PreviousFixedAxisValue == this->InternalColorHSV[2])
        {
        return;
        }
      this->Internals->PreviousFixedAxisValue = this->InternalColorHSV[2];
      v = this->InternalColorHSV[2];
      s = 1.0;
      for (i = 0; i < size; i++)
        {
        one_s = 1.0 - s;
        h = 0.0;
        for (j = 0; j < size; j++)
          {
          if (h > onesixth && h <= onethird) // green/red
            {
            r = (s*((onethird-h)*onesixthinv)+one_s)*v; g = v; b = one_s*v;
            }
          else if (h > onethird && h <= onehalf) // green/blue
            {
            r = one_s*v; g = v; b = (s*((h-onethird)*onesixthinv)+one_s)*v; 
            }
          else if (h > onehalf && h <= twothird) // blue/green
            {
            r = one_s*v; g = (s*((twothird-h)*onesixthinv)+one_s)*v; b = v; 
            }
          else if (h > twothird && h <= fivesixth) // blue/red
            {
            r = (s*((h-twothird)*onesixthinv)+one_s)*v; g = one_s*v; b = v; 
            }
          else if (h > fivesixth && h <= 1.0) // red/blue
            {
            r = v; g = one_s*v; b = (s*((1.0-h)*onesixthinv)+one_s)*v; 
            }
          else // red/green
            {
            r = v; g = (s*(h*onesixthinv)+one_s)*v; b = one_s*v;
            }
          *img_ptr++ = (unsigned char)(255.0 * r);
          *img_ptr++ = (unsigned char)(255.0 * g);
          *img_ptr++ = (unsigned char)(255.0 * b);
          h += inc;
          }
        s -= inc;
        }
      break;
    }

  this->Internals->PreviousFixedAxis = this->FixedAxis;

  // Update the image

  vtksys_ios::ostringstream img_name;
  img_name << this->ColorCanvas->GetWidgetName() 
           << "." << VTK_KW_COLOR_SPECTRUM_WIDGET_IMAGE_TAG;

  vtkKWTkUtilities::UpdatePhoto(this->GetApplication(),
                                img_name.str().c_str(),
                                img_buffer,
                                size, size, 3,
                                size * size * 3);
  
  // delete [] img_buffer;
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::UpdateColorCursor()
{
  if (!this->ColorCanvas || !this->ColorCanvas->IsCreated())
    {
    return;
    }

  double x, y;

  switch (this->FixedAxis)
    {
    // over R, G vertically (1.0 -> 0.0), B horizontally (0.0 -> 1.0)
    case vtkKWColorSpectrumWidget::FixedAxisR: 
      x = this->InternalColorRGB[2];
      y = this->InternalColorRGB[1];
      break;

    // over G, R vertically (1.0 -> 0.0), B horizontally (0.0 -> 1.0)
    case vtkKWColorSpectrumWidget::FixedAxisG: 
      x = this->InternalColorRGB[2];
      y = this->InternalColorRGB[0];
      break;

    // over B, G vertically (1.0 -> 0.0), R horizontally (0.0 -> 1.0)
    case vtkKWColorSpectrumWidget::FixedAxisB:
      x = this->InternalColorRGB[0];
      y = this->InternalColorRGB[1];
      break;

    // over H, V vertically (1.0 -> 0.0), S horizontally (0.0 -> 1.0)
    case vtkKWColorSpectrumWidget::FixedAxisH:
      x = this->InternalColorHSV[1];
      y = this->InternalColorHSV[2];
      break;

    // over S, V vertically (1.0 -> 0.0), H horizontally (0.0 -> 1.0)
    case vtkKWColorSpectrumWidget::FixedAxisS:
      x = this->InternalColorHSV[0];
      y = this->InternalColorHSV[2];
      break;

    // over V, S vertically (1.0 -> 0.0), H horizontally (0.0 -> 1.0)
    case vtkKWColorSpectrumWidget::FixedAxisV:
      x = this->InternalColorHSV[0];
      y = this->InternalColorHSV[1];
      break;
    }

  y = 1.0 - y;

  const int size = VTK_KW_COLOR_SPECTRUM_WIDGET_CANVAS_SIZE;
  double radius = 5.0;

  ostrstream tk_cmd;
  
  tk_cmd << this->ColorCanvas->GetWidgetName() 
         << " coords " << VTK_KW_COLOR_SPECTRUM_WIDGET_CURSOR_TAG
         << " " << x * size - radius
         << " " << y * size - radius
         << " " << x * size + radius
         << " " << y * size + radius
         << endl;

  tk_cmd << this->ColorCanvas->GetWidgetName() 
         << " itemconfigure " << VTK_KW_COLOR_SPECTRUM_WIDGET_CURSOR_TAG
         << " -outline " 
         << (this->InternalColorHSV[2] > 0.5 ? "black" : "#c9c9c9")
         << endl;

  tk_cmd << ends;
  this->Script(tk_cmd.str());
  tk_cmd.rdbuf()->freeze(0);
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::PickColorPressCallback(int x, int y)
{
  switch (this->FixedAxis)
    {
    case vtkKWColorSpectrumWidget::FixedAxisR:
    case vtkKWColorSpectrumWidget::FixedAxisG: 
    case vtkKWColorSpectrumWidget::FixedAxisB:
      this->Internals->PreviousPickColor[0] = this->InternalColorRGB[0];
      this->Internals->PreviousPickColor[1] = this->InternalColorRGB[1];
      this->Internals->PreviousPickColor[2] = this->InternalColorRGB[2];
      break;

    case vtkKWColorSpectrumWidget::FixedAxisH:
    case vtkKWColorSpectrumWidget::FixedAxisS:
    case vtkKWColorSpectrumWidget::FixedAxisV:
      this->Internals->PreviousPickColor[0] = this->InternalColorHSV[0];
      this->Internals->PreviousPickColor[1] = this->InternalColorHSV[1];
      this->Internals->PreviousPickColor[2] = this->InternalColorHSV[2];
      break;
    }

  this->PickColorMoveCallback(x, y);
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::PickColorMoveCallback(int x, int y)
{
  const int size = VTK_KW_COLOR_SPECTRUM_WIDGET_CANVAS_SIZE;
  
  double dx = (double)x / (double)size;
  double dy = (double)(size - y) / (double)size;

  double h, s, v;
  double r, g, b;

  switch (this->FixedAxis)
    {
    // over R, G vertically (1.0 -> 0.0), B horizontally (0.0 -> 1.0)
    case vtkKWColorSpectrumWidget::FixedAxisR:
      r = this->InternalColorRGB[0];
      g = dy;
      b = dx;
      break;

    // over G, R vertically (1.0 -> 0.0), B horizontally (0.0 -> 1.0)
    case vtkKWColorSpectrumWidget::FixedAxisG: 
      r = dy;
      g = this->InternalColorRGB[1];
      b = dx;
      break;

    // over B, G vertically (1.0 -> 0.0), R horizontally (0.0 -> 1.0)
    case vtkKWColorSpectrumWidget::FixedAxisB:
      r = dx;
      g = dy;
      b = this->InternalColorRGB[2];
      break;

    // over H, V vertically (1.0 -> 0.0), S horizontally (0.0 -> 1.0)
    case vtkKWColorSpectrumWidget::FixedAxisH:
      h = this->InternalColorHSV[0];
      s = dx;
      v = dy;
      break;

    // over S, V vertically (1.0 -> 0.0), H horizontally (0.0 -> 1.0)
    case vtkKWColorSpectrumWidget::FixedAxisS:
      h = dx;
      s = this->InternalColorHSV[1];
      v = dy;
      break;

    // over V, S vertically (1.0 -> 0.0), H horizontally (0.0 -> 1.0)
    case vtkKWColorSpectrumWidget::FixedAxisV:
      h = dx;
      s = dy;
      v = this->InternalColorHSV[2];
      break;
    }

  // Set color

  switch (this->FixedAxis)
    {
    case vtkKWColorSpectrumWidget::FixedAxisR:
    case vtkKWColorSpectrumWidget::FixedAxisG: 
    case vtkKWColorSpectrumWidget::FixedAxisB:
      if (this->Internals->PreviousPickColor[0] != r ||
          this->Internals->PreviousPickColor[1] != g ||
          this->Internals->PreviousPickColor[2] != b)
        {
        this->SetInternalColorRGB(r, g, b);

        vtkMath::RGBToHSV(r, g, b, &h, &s, &v);
        this->SetInternalColorHSV(h, s, v);

        this->UpdateColorCursor();
        this->InvokeColorChangingCommand();
        }
      break;

    case vtkKWColorSpectrumWidget::FixedAxisH:
    case vtkKWColorSpectrumWidget::FixedAxisS:
    case vtkKWColorSpectrumWidget::FixedAxisV:
      if (this->Internals->PreviousPickColor[0] != h ||
          this->Internals->PreviousPickColor[1] != s ||
          this->Internals->PreviousPickColor[2] != v)
        {
        this->SetInternalColorHSV(h, s, v);

        vtkMath::HSVToRGB(h, s, v, &r, &g, &b);
        this->SetInternalColorRGB(r, g, b);

        this->UpdateColorCursor();
        this->InvokeColorChangingCommand();
        }
      break;
    }
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::PickColorReleaseCallback()
{
  switch (this->FixedAxis)
    {
    case vtkKWColorSpectrumWidget::FixedAxisR:
    case vtkKWColorSpectrumWidget::FixedAxisG: 
    case vtkKWColorSpectrumWidget::FixedAxisB:
      if (this->Internals->PreviousPickColor[0] != this->InternalColorRGB[0] ||
          this->Internals->PreviousPickColor[1] != this->InternalColorRGB[1] ||
          this->Internals->PreviousPickColor[2] != this->InternalColorRGB[2])
        {
        this->InvokeColorChangedCommand();
        }
      break;

    case vtkKWColorSpectrumWidget::FixedAxisH:
    case vtkKWColorSpectrumWidget::FixedAxisS:
    case vtkKWColorSpectrumWidget::FixedAxisV:
      if (this->Internals->PreviousPickColor[0] != this->InternalColorHSV[0] ||
          this->Internals->PreviousPickColor[1] != this->InternalColorHSV[1] ||
          this->Internals->PreviousPickColor[2] != this->InternalColorHSV[2])
        {
        this->InvokeColorChangedCommand();
        }
      break;
    }
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::AddBindings()
{
  if (!this->IsCreated())
    {
    return;
    }

  if (this->ColorCanvas && this->ColorCanvas->IsAlive())
    {
    this->ColorCanvas->SetBinding(
      "<ButtonPress-1>", this, "PickColorPressCallback %x %y");
    this->ColorCanvas->SetBinding(
      "<B1-Motion>", this, "PickColorMoveCallback %x %y");
    this->ColorCanvas->SetBinding(
      "<ButtonRelease-1>", this, "PickColorReleaseCallback");
    }
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::RemoveBindings()
{
  if (!this->IsCreated())
    {
    return;
    }

  if (this->ColorCanvas && this->ColorCanvas->IsAlive())
    {
    this->ColorCanvas->RemoveBinding("<ButtonPress-1>");
    this->ColorCanvas->RemoveBinding("<B1-Motion>");
    this->ColorCanvas->RemoveBinding("<ButtonRelease-1>");
    }
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::SetColorChangedCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->ColorChangedCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::InvokeColorChangedCommand()
{
  this->InvokeEvent(vtkKWColorSpectrumWidget::ColorChangedEvent);
  this->InvokeObjectMethodCommand(this->ColorChangedCommand);
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::SetColorChangingCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->ColorChangingCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::InvokeColorChangingCommand()
{
  this->InvokeEvent(vtkKWColorSpectrumWidget::ColorChangingEvent);
  this->InvokeObjectMethodCommand(this->ColorChangingCommand);
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->PropagateEnableState(this->ColorCanvas);

  if (this->GetEnabled())
    {
    this->AddBindings();
    }
  else
    {
    this->RemoveBindings();
    }
}

//----------------------------------------------------------------------------
void vtkKWColorSpectrumWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
