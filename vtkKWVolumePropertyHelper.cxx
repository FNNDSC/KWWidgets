/*=========================================================================

Copyright (c) 1998-2003 Kitware Inc. 469 Clifton Corporate Parkway,
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
MAINTENANCE, SUPPORT, UPDATE, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
#include "vtkKWVolumePropertyHelper.h"

#include "vtkColorTransferFunction.h"
#include "vtkObjectFactory.h"
#include "vtkPiecewiseFunction.h"
#include "vtkVolumeProperty.h"
#include "vtkKWHistogramSet.h"
#include "vtkKWHistogram.h"

#include "vtkMath.h"
#include "vtkImageData.h"
#include "vtkDataArray.h"
#include "vtkPointData.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWVolumePropertyHelper);
vtkCxxRevisionMacro(vtkKWVolumePropertyHelper, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
void vtkKWVolumePropertyHelper::ApplyScalarOpacityPreset(
  vtkVolumeProperty *prop, 
  int comp, 
  int preset)
{
  if (!prop || comp < 0 || comp > VTK_MAX_VRCOMP)
    {
    return;
    }

  vtkPiecewiseFunction *pwf = prop->GetScalarOpacity(comp);
  pwf->RemoveAllPoints();

  switch (preset)
    {
    case vtkKWVolumePropertyHelper::ScalarOpacityRamp0At0To20At100:
      pwf->AddPoint(0.0, 0.0);
      pwf->AddPoint(1.0, 0.2);
      break;

    case vtkKWVolumePropertyHelper::ScalarOpacityRamp0At25To100At50:
      pwf->AddPoint(0.0, 0.0);
      pwf->AddPoint(0.25, 0.0);
      pwf->AddPoint(0.5,  1.0);
      pwf->AddPoint(1.0,  1.0);
      break;

    case vtkKWVolumePropertyHelper::ScalarOpacityRamp0At50To100At75:
      pwf->AddPoint(0.0, 0.0);
      pwf->AddPoint(0.5, 0.0);
      pwf->AddPoint(0.75, 1.0);
      pwf->AddPoint(1.0, 1.0);
      break;

    case vtkKWVolumePropertyHelper::ScalarOpacitySteps0To25Then20To50Then40To75Then100To100:
      pwf->AddPoint(0.0, 0.0, 1.0, 1.0);
      pwf->AddPoint(0.25, 0.2, 1.0, 1.0);
      pwf->AddPoint(0.5, 0.4, 1.0, 1.0);
      pwf->AddPoint(0.75,  1.0);
      pwf->AddPoint(1.0,  1.0);
      break;
    }
}

//----------------------------------------------------------------------------
void vtkKWVolumePropertyHelper::ApplyRGBTransferFunctionPreset(
  vtkVolumeProperty *prop, 
  int comp, 
  int preset, 
  double rgb[3])
{
  vtkKWVolumePropertyHelper::ApplyRGBTransferFunctionPreset(
    prop, comp, preset, 1, &rgb);
}

//----------------------------------------------------------------------------
void vtkKWVolumePropertyHelper::ApplyRGBTransferFunctionPreset(
  vtkVolumeProperty *prop, 
  int comp, 
  int preset, 
  double rgb1[3], double rgb2[3])
{
  double *rgb[] = {rgb1, rgb2};
  vtkKWVolumePropertyHelper::ApplyRGBTransferFunctionPreset(
    prop, comp, preset, 2, rgb);
}

//----------------------------------------------------------------------------
void vtkKWVolumePropertyHelper::ApplyRGBTransferFunctionPreset(
  vtkVolumeProperty *prop, 
  int comp, 
  int preset, 
  double rgb1[3], double rgb2[3], double rgb3[3])
{
  double *rgb[] = {rgb1, rgb2, rgb3};
  vtkKWVolumePropertyHelper::ApplyRGBTransferFunctionPreset(
    prop, comp, preset, 3, rgb);
}

//----------------------------------------------------------------------------
void vtkKWVolumePropertyHelper::ApplyRGBTransferFunctionPreset(
  vtkVolumeProperty *prop, 
  int comp, 
  int preset, 
  double rgb1[3], double rgb2[3], double rgb3[3], double rgb4[3])
{
  double *rgb[] = {rgb1, rgb2, rgb3, rgb4};
  vtkKWVolumePropertyHelper::ApplyRGBTransferFunctionPreset(
    prop, comp, preset, 4, rgb);
}

//----------------------------------------------------------------------------
void vtkKWVolumePropertyHelper::ApplyRGBTransferFunctionPreset(
  vtkVolumeProperty *prop, 
  int comp, 
  int preset, 
  int nb_colors, double **rgb)
{
  if (!prop || comp < 0 || comp > VTK_MAX_VRCOMP || nb_colors < 1 || !rgb)
    {
    return;
    }

  vtkColorTransferFunction *ctf = prop->GetRGBTransferFunction(comp);
  ctf->RemoveAllPoints();
  ctf->SetColorSpaceToHSV();

  int i0 = 0;
  int i1 = 1 % nb_colors;
  int i2 = 2 % nb_colors;
  int i3 = 3 % nb_colors;

  switch (preset)
    {
    case vtkKWVolumePropertyHelper::RGBTransferFunctionRampAt0At100:
      ctf->AddRGBPoint(0.0, rgb[i0][0], rgb[i0][1], rgb[i0][2], 0.25, 0.0);
      ctf->AddRGBPoint(1.0, rgb[i1][0], rgb[i1][1], rgb[i1][2]);
      break;

    case vtkKWVolumePropertyHelper::RGBTransferFunctionRampAt0At70At100:
      ctf->AddRGBPoint(0.0, rgb[i0][0], rgb[i0][1], rgb[i0][2]);
      ctf->AddRGBPoint(0.7, rgb[i1][0], rgb[i1][1], rgb[i1][2]);
      ctf->AddRGBPoint(1.0, rgb[i2][0], rgb[i2][1], rgb[i2][2]);
      break;

    case vtkKWVolumePropertyHelper::RGBTransferFunctionStepsTo25To50To75To100:
      ctf->AddRGBPoint(0.0,  rgb[i0][0], rgb[i0][1], rgb[i0][2], 1.0, 1.0);
      ctf->AddRGBPoint(0.25, rgb[i1][0], rgb[i1][1], rgb[i1][2], 1.0, 1.0);
      ctf->AddRGBPoint(0.5,  rgb[i2][0], rgb[i2][1], rgb[i2][2], 1.0, 1.0);
      ctf->AddRGBPoint(0.75, rgb[i3][0], rgb[i3][1], rgb[i3][2]);
      ctf->AddRGBPoint(1.0,  rgb[i3][0], rgb[i3][1], rgb[i3][2]);
      break;
    }
}

//----------------------------------------------------------------------------
void vtkKWVolumePropertyHelper::ApplyGradientOpacityPreset(
  vtkVolumeProperty *prop, 
  int comp, 
  int preset)
{
  if (!prop || comp < 0 || comp > VTK_MAX_VRCOMP)
    {
    return;
    }

  vtkPiecewiseFunction *pwf = prop->GetStoredGradientOpacity(comp);
  pwf->RemoveAllPoints();

  // WARNING: 0.25 is a VolView 2 .0 hard-coded value because of the raycast
  // mapper behaviour (1/4 of the range). It should be investigated and
  // maybe dropped (if not, the point at 1.0 following 0.25 should be discarded)

  switch (preset)
    {
    case vtkKWVolumePropertyHelper::GradientOpacityNoEdge:
      pwf->AddPoint(0.0,  1.0);
      pwf->AddPoint(0.25, 1.0);
      pwf->AddPoint(1.0,  1.0);
      prop->SetDisableGradientOpacity(1);
      break;

    case vtkKWVolumePropertyHelper::GradientOpacityMediumEdge:
      pwf->AddPoint(0.0,  0.0);
      pwf->AddPoint(0.02, 1.0);
      pwf->AddPoint(0.25, 1.0);
      pwf->AddPoint(1.0,  1.0);
      prop->SetDisableGradientOpacity(0);
      break;

    case vtkKWVolumePropertyHelper::GradientOpacityStrongEdge:
      pwf->AddPoint(0.0,  0.0);
      pwf->AddPoint(0.01, 0.0);
      pwf->AddPoint(0.05, 1.0);
      pwf->AddPoint(0.25, 1.0);
      pwf->AddPoint(1.0,  1.0);
      prop->SetDisableGradientOpacity(0);
      break;
    }
}

//----------------------------------------------------------------------------
void vtkKWVolumePropertyHelper::ApplyLightingPreset(
  vtkVolumeProperty *prop, 
  int comp, 
  int preset)
{
  if (!prop || comp < 0 || comp > VTK_MAX_VRCOMP)
    {
    return;
    }

  switch (preset)
    {
    case vtkKWVolumePropertyHelper::LightingFullAmbient:
      prop->SetAmbient(comp, 1.0);
      prop->SetDiffuse(comp, 0.0);
      prop->SetSpecular(comp, 0.0);
      prop->SetSpecularPower(comp, 1.0);
      break;

    case vtkKWVolumePropertyHelper::LightingDull:
      prop->SetAmbient(comp, 0.2);
      prop->SetDiffuse(comp, 1.0);
      prop->SetSpecular(comp, 0.0);
      prop->SetSpecularPower(comp, 1.0);
      break;

    case vtkKWVolumePropertyHelper::LightingSmooth:
      prop->SetAmbient(comp, 0.1);
      prop->SetDiffuse(comp, 0.9);
      prop->SetSpecular(comp, 0.2);
      prop->SetSpecularPower(comp, 10.0);
      break;

    case vtkKWVolumePropertyHelper::LightingShiny:
      prop->SetAmbient(comp, 0.1);
      prop->SetDiffuse(comp, 0.6);
      prop->SetSpecular(comp, 0.5);
      prop->SetSpecularPower(comp, 40.0);
      break;
    }
}

//----------------------------------------------------------------------------
void vtkKWVolumePropertyHelper::ApplyPreset(
  vtkVolumeProperty *prop, 
  int preset)
{
  if (!prop)
    {
    return;
    }

  int i;

  double *black = 
    vtkKWVolumePropertyHelper::GetRGBColor(vtkKWVolumePropertyHelper::Black);
  double *white = 
    vtkKWVolumePropertyHelper::GetRGBColor(vtkKWVolumePropertyHelper::White);
  double *gray3 =
    vtkKWVolumePropertyHelper::GetRGBColor(vtkKWVolumePropertyHelper::Gray3);
  double *red =
    vtkKWVolumePropertyHelper::GetRGBColor(vtkKWVolumePropertyHelper::Red);
  double *green =
    vtkKWVolumePropertyHelper::GetRGBColor(vtkKWVolumePropertyHelper::Green);
  double *blue =
    vtkKWVolumePropertyHelper::GetRGBColor(vtkKWVolumePropertyHelper::Blue);
  double *yellow = 
    vtkKWVolumePropertyHelper::GetRGBColor(vtkKWVolumePropertyHelper::Yellow);
  double *magenta =
    vtkKWVolumePropertyHelper::GetRGBColor(vtkKWVolumePropertyHelper::Magenta);
  double *cyan =
    vtkKWVolumePropertyHelper::GetRGBColor(vtkKWVolumePropertyHelper::Cyan);
  double *tan =
    vtkKWVolumePropertyHelper::GetRGBColor(vtkKWVolumePropertyHelper::Tan);

  double *colors[VTK_MAX_VRCOMP] = { red, green, blue, yellow };
  double *color = NULL;

  switch (preset)
    {
    // A histogram adjusted preset from 0 opacity at 0% to .2 opacity at 100%
    // of the histogram with white at both ends for color, no shading

    case vtkKWVolumePropertyHelper::Preset1:
      prop->SetInterpolationTypeToLinear();
      for (i = 0; i < VTK_MAX_VRCOMP; i++)
        {
        vtkKWVolumePropertyHelper::ApplyScalarOpacityPreset(
          prop, i, 
          vtkKWVolumePropertyHelper::ScalarOpacityRamp0At0To20At100);
        
        color = (i == 0 ? white : colors[i - 1]);
        vtkKWVolumePropertyHelper::ApplyRGBTransferFunctionPreset(
          prop, i, 
          vtkKWVolumePropertyHelper::RGBTransferFunctionRampAt0At100, 
          color);

        vtkKWVolumePropertyHelper::ApplyGradientOpacityPreset(
          prop, i, vtkKWVolumePropertyHelper::GradientOpacityNoEdge);

        vtkKWVolumePropertyHelper::ApplyLightingPreset(
          prop, i, vtkKWVolumePropertyHelper::LightingDull);
        prop->ShadeOff(i);
        }
      break;

      // a histogram adjusted preset from 0 opacity at 0% to .2 opacity at
      // 100% of the histogram with black at the low end and white at the
      // high end for color, no shading

    case vtkKWVolumePropertyHelper::Preset2:
      prop->SetInterpolationTypeToLinear();
      for (i = 0; i < VTK_MAX_VRCOMP; i++)
        {
        vtkKWVolumePropertyHelper::ApplyScalarOpacityPreset(
          prop, i, 
          vtkKWVolumePropertyHelper::ScalarOpacityRamp0At0To20At100);
        
        color = (i == 0 ? white : colors[i - 1]);
        vtkKWVolumePropertyHelper::ApplyRGBTransferFunctionPreset(
          prop, i, 
          vtkKWVolumePropertyHelper::RGBTransferFunctionRampAt0At100, 
          black, color);

        vtkKWVolumePropertyHelper::ApplyGradientOpacityPreset(
          prop, i, vtkKWVolumePropertyHelper::GradientOpacityNoEdge);

        vtkKWVolumePropertyHelper::ApplyLightingPreset(
          prop, i, vtkKWVolumePropertyHelper::LightingDull);
        prop->ShadeOff(i);
        }
      break;

      // A histogram adjusted preset from 0 opacity at 0% to .2 opacity at
      // 100% of the histogram with a color transfer function (perhaps our
      // old rainbow friend...), no shading
      
    case vtkKWVolumePropertyHelper::Preset3:
      prop->SetInterpolationTypeToLinear();
      for (i = 0; i < VTK_MAX_VRCOMP; i++)
        {
        vtkKWVolumePropertyHelper::ApplyScalarOpacityPreset(
          prop, i, 
          vtkKWVolumePropertyHelper::ScalarOpacityRamp0At0To20At100);
        
        vtkKWVolumePropertyHelper::ApplyRGBTransferFunctionPreset(
          prop, i, 
          vtkKWVolumePropertyHelper::RGBTransferFunctionRampAt0At70At100,
          blue, green, red);

        vtkKWVolumePropertyHelper::ApplyGradientOpacityPreset(
          prop, i, vtkKWVolumePropertyHelper::GradientOpacityNoEdge);

        vtkKWVolumePropertyHelper::ApplyLightingPreset(
          prop, i, vtkKWVolumePropertyHelper::LightingDull);
        prop->ShadeOff(i);
        }
      break;

      // A histogram adjusted preset from 0 opacity at 25% to full opacity at
      // 50% with shading. Some light color would be good (maybe not white
      // for some visual interest - perhaps a slight tan color)

    case vtkKWVolumePropertyHelper::Preset4:
      prop->SetInterpolationTypeToLinear();
      for (i = 0; i < VTK_MAX_VRCOMP; i++)
        {
        vtkKWVolumePropertyHelper::ApplyScalarOpacityPreset(
          prop, i, 
          vtkKWVolumePropertyHelper::ScalarOpacityRamp0At25To100At50);
        
        color = (i == 0 ? tan : colors[i - 1]);
        vtkKWVolumePropertyHelper::ApplyRGBTransferFunctionPreset(
          prop, i, 
          vtkKWVolumePropertyHelper::RGBTransferFunctionRampAt0At100, 
          color);

        vtkKWVolumePropertyHelper::ApplyGradientOpacityPreset(
          prop, i, vtkKWVolumePropertyHelper::GradientOpacityNoEdge);

        vtkKWVolumePropertyHelper::ApplyLightingPreset(
          prop, i, vtkKWVolumePropertyHelper::LightingDull);
        prop->ShadeOn(i);
        }
      break;

      // A histogram adjusted preset from 0 opacity at 50% to full opacity
      // at 75% with shading. Some light color would be good (maybe not
      // white for some visual interest - perhaps a slight tan color)

    case vtkKWVolumePropertyHelper::Preset5:
      prop->SetInterpolationTypeToLinear();
      for (i = 0; i < VTK_MAX_VRCOMP; i++)
        {
        vtkKWVolumePropertyHelper::ApplyScalarOpacityPreset(
          prop, i, 
          vtkKWVolumePropertyHelper::ScalarOpacityRamp0At50To100At75);
        
        color = (i == 0 ? tan : colors[i - 1]);
        vtkKWVolumePropertyHelper::ApplyRGBTransferFunctionPreset(
          prop, i, 
          vtkKWVolumePropertyHelper::RGBTransferFunctionRampAt0At100, 
          color);

        vtkKWVolumePropertyHelper::ApplyGradientOpacityPreset(
          prop, i, vtkKWVolumePropertyHelper::GradientOpacityNoEdge);

        vtkKWVolumePropertyHelper::ApplyLightingPreset(
          prop, i, vtkKWVolumePropertyHelper::LightingDull);
        prop->ShadeOn(i);
        }
      break;

      // A function that is at 0 opacity until 25%, .2 opacity from 25% to
      // 50%, .4 opacity from 50% to 75%, and 1 opacity from 75% to 100%.
      // A different color (again, piecewise discrete) in each of these 
      // regions. This type of color map is probably best if we link it to 
      // detected changes in the histogram instead of fixed amounts - but it
      // is at least a starting point for someone.

    case vtkKWVolumePropertyHelper::Preset6:
      prop->SetInterpolationTypeToLinear();
      for (i = 0; i < VTK_MAX_VRCOMP; i++)
        {
        vtkKWVolumePropertyHelper::ApplyScalarOpacityPreset(
          prop, i, 
          vtkKWVolumePropertyHelper::ScalarOpacitySteps0To25Then20To50Then40To75Then100To100);
        
        vtkKWVolumePropertyHelper::ApplyRGBTransferFunctionPreset(
          prop, i, 
          vtkKWVolumePropertyHelper::RGBTransferFunctionStepsTo25To50To75To100,
          red, green, yellow, blue);

        vtkKWVolumePropertyHelper::ApplyGradientOpacityPreset(
          prop, i, vtkKWVolumePropertyHelper::GradientOpacityNoEdge);

        vtkKWVolumePropertyHelper::ApplyLightingPreset(
          prop, i, vtkKWVolumePropertyHelper::LightingDull);
        prop->ShadeOn(i);
        }
      break;
    }
}

//----------------------------------------------------------------------------
double* vtkKWVolumePropertyHelper::GetRGBColor(int preset)
{
  static double black[3]   = { 0.0, 0.0, 0.0 };
  static double white[3]   = { 1.0, 1.0, 1.0 };
  static double gray3[3]   = { 0.3, 0.3, 0.3 };
  static double red[3]     = { 1.0, 0.3, 0.3 };
  static double green[3]   = { 0.3, 1.0, 0.3 };
  static double blue[3]    = { 0.3, 0.3, 1.0 };
  static double yellow[3]  = { 1.0, 1.0, 0.3 };
  static double magenta[3] = { 1.0, 0.3, 1.0 };
  static double cyan[3]    = { 0.3, 1.0, 1.0 };
  static double tan[3]     = { 210.0 / 255.0, 180.0 / 255.0, 140.0 / 255.0 };

  switch (preset)
    {
    case vtkKWVolumePropertyHelper::Black:
      return black;
    case vtkKWVolumePropertyHelper::White:
      return white;
    case vtkKWVolumePropertyHelper::Gray3:
      return gray3;
    case vtkKWVolumePropertyHelper::Red:
      return red;
    case vtkKWVolumePropertyHelper::Green:
      return green;
    case vtkKWVolumePropertyHelper::Blue:
      return blue;
    case vtkKWVolumePropertyHelper::Yellow:
      return yellow;
    case vtkKWVolumePropertyHelper::Magenta:
      return magenta;
    case vtkKWVolumePropertyHelper::Cyan:
      return cyan;
    case vtkKWVolumePropertyHelper::Tan:
      return tan;
    };
  
  return NULL;
}

//----------------------------------------------------------------------------
void vtkKWVolumePropertyHelper::ConvertNormalizedRange(
  vtkVolumeProperty *normalized_prop, 
  vtkVolumeProperty *target_prop, 
  vtkImageData *image,
  int independent_component,
  vtkKWHistogramSet *histogram_set)
{
  if (!normalized_prop || !target_prop || !image)
    {
    return;
    }

  vtkDataArray *scalars = image->GetPointData()->GetScalars();
  if (!scalars)
    {
    return;
    }

  vtkKWVolumePropertyHelper::CopyVolumeProperty(
    target_prop, 
    normalized_prop, 
    (vtkKWVolumePropertyHelper::CopySkipOpacityUnitDistance |
     vtkKWVolumePropertyHelper::CopySkipIndependentComponents)
    );
  
  // independent_component should be the same as target_prop's 
  // IndependentComponents, but just in case...
  target_prop->SetIndependentComponents(independent_component);

  int nb_components = scalars->GetNumberOfComponents();
  int comp;

  // Scalar Opacity and Gradient Opacity

  for (comp = 0; comp < VTK_MAX_VRCOMP; ++comp)
    {
    // We have only 2 cases with dependent component:
    // 2 or 4 comps, in both cases the scalar field is in the last comp

    int scalar_field = independent_component ? comp : nb_components - 1;

    double range[2];
    scalars->GetRange(range, scalar_field);
    int collapsed_range = (range[0] == range[1] ? 1 : 0);
    if (collapsed_range) // avoid collapsed transfer functions
      {
      vtkMath::GetAdjustedScalarRange(scalars, scalar_field, range);
      }
    double range_delta = range[1] - range[0];

    vtkKWHistogram *hist = NULL;
    char hist_name[1024];
    double exclude_value;
    double total_occ;
    if (!collapsed_range && 
        histogram_set && 
        histogram_set->ComputeHistogramName(
          scalars->GetName(), scalar_field, NULL, hist_name))
      {
      hist = histogram_set->GetHistogramWithName(hist_name);
      if (hist)
        {
        exclude_value = range[0]; // exclude the background bin
        total_occ = hist->GetTotalOccurence() -
          hist->GetOccurenceAtValue(exclude_value);
        }
      }

    // Scalar opacity

    vtkPiecewiseFunction *normalized_pwf = 
      normalized_prop->GetScalarOpacity(comp);
    vtkPiecewiseFunction *target_pwf = 
      target_prop->GetScalarOpacity(comp);
    target_pwf->RemoveAllPoints();
    if (normalized_pwf)
      {
      int i, size = normalized_pwf->GetSize();
      double value[4];
      for (i = 0; i < size; i++)
        {
        if (normalized_pwf->GetNodeValue(i, value))
          {
          double x;
          if (hist)
            {
            x = hist->GetValueAtAccumulatedOccurence(
              total_occ * value[0], &exclude_value);
            }
          else
            {
            x = range[0] + value[0] * range_delta;
            }
          target_pwf->AddPoint(x, value[1], value[2], value[3]);
          }
        }
      }
                                                                        
    // Gradient Opacity

    normalized_pwf = 
      normalized_prop->GetStoredGradientOpacity(comp);
    target_pwf = 
      target_prop->GetStoredGradientOpacity(comp);
    target_pwf->RemoveAllPoints();
    if (normalized_pwf)
      {
      int i, size = normalized_pwf->GetSize();
      double value[4];
      for (i = 0; i < size; i++)
        {
        if (normalized_pwf->GetNodeValue(i, value))
          {
          double x;
          // do not use histogram with gradient for compat with VV 2.0
          if (hist && 0) 
            {
            x = hist->GetValueAtAccumulatedOccurence(
              total_occ * value[0], &exclude_value);
            }
          else
            {
            x = range[0] + value[0] * range_delta;
            }
          target_pwf->AddPoint(x, value[1], value[2], value[3]);
          }
        }
      }
    }

  // Color Transfer Functions
  // Done in a different loop because scalar_field is computed differently

  for (comp = 0; comp < VTK_MAX_VRCOMP; ++comp)
    {
    // We have only 1 case with dependent component:
    // 2  comp (rest is RGB), the scalar field is in the first comp.

    int scalar_field = independent_component ? comp : 0;

    double range[2];
    scalars->GetRange(range, scalar_field);
    int collapsed_range = (range[0] == range[1] ? 1 : 0);
    if (collapsed_range) // avoid collapsed transfer functions
      {
      vtkMath::GetAdjustedScalarRange(scalars, scalar_field, range);
      }
    double range_delta = range[1] - range[0];

    vtkKWHistogram *hist = NULL;
    char hist_name[1024];
    double exclude_value;
    double total_occ;
    if (!collapsed_range && 
        histogram_set && histogram_set->ComputeHistogramName(
          scalars->GetName(), scalar_field, NULL, hist_name))
      {
      hist = histogram_set->GetHistogramWithName(hist_name);
      if (hist)
        {
        exclude_value = range[0]; // exclude the background bin
        total_occ = hist->GetTotalOccurence() -
          hist->GetOccurenceAtValue(exclude_value);
        }
      }

    vtkColorTransferFunction *normalized_ctf = 
      normalized_prop->GetRGBTransferFunction(comp);
    vtkColorTransferFunction *target_ctf = 
      target_prop->GetRGBTransferFunction(comp);
    target_ctf->RemoveAllPoints();
    if (normalized_ctf)
      {
      int i, size = normalized_ctf->GetSize();
      double value[6];
      for (i = 0; i < size; i++)
        {
        if (normalized_ctf->GetNodeValue(i, value))
          {
          double x;
          if (hist)
            {
            x = hist->GetValueAtAccumulatedOccurence(
              total_occ * value[0], &exclude_value);
            }
          else
            {
            x = range[0] + value[0] * range_delta;
            }
          target_ctf->AddRGBPoint(
            x, value[1], value[2], value[3], value[4], value[5]);
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWVolumePropertyHelper::ApplyPresetAndConvertNormalizedRange(
  int preset,
  vtkVolumeProperty *target_prop, 
  vtkImageData *image,
  int independent_component,
  vtkKWHistogramSet *histogram_set
  )
{
  vtkVolumeProperty *temp_prop = vtkVolumeProperty::New();
  vtkKWVolumePropertyHelper::ApplyPreset(temp_prop, preset);
  vtkKWVolumePropertyHelper::ConvertNormalizedRange(
    temp_prop, target_prop, image, independent_component, histogram_set);
  temp_prop->Delete();
}

//----------------------------------------------------------------------------
void vtkKWVolumePropertyHelper::DeepCopyVolumeProperty(
  vtkVolumeProperty *target, vtkVolumeProperty *source)
{
  vtkKWVolumePropertyHelper::CopyVolumeProperty(target, source, 0);
}

//----------------------------------------------------------------------------
void vtkKWVolumePropertyHelper::CopyVolumeProperty(
  vtkVolumeProperty *target, vtkVolumeProperty *source, int options)
{
  if (!target || !source)
    {
    return;
    }

  if (!(options & vtkKWVolumePropertyHelper::CopySkipIndependentComponents))
    {
    target->SetIndependentComponents(source->GetIndependentComponents());
    }

  target->SetInterpolationType(source->GetInterpolationType());

  for (int i = 0; i < VTK_MAX_VRCOMP; i++)
    {
    target->SetComponentWeight(i, source->GetComponentWeight(i));
    
    // Force ColorChannels to the right value and/or create a default tfunc
    // then DeepCopy all the points

    if (source->GetColorChannels(i) > 1)
      {
      target->SetColor(i, target->GetRGBTransferFunction(i));
      target->GetRGBTransferFunction(i)->DeepCopy(
        source->GetRGBTransferFunction(i));
      }
    else
      {
      target->SetColor(i, target->GetGrayTransferFunction(i));
      target->GetGrayTransferFunction(i)->DeepCopy(
        source->GetGrayTransferFunction(i));
      }

    target->GetScalarOpacity(i)->DeepCopy(source->GetScalarOpacity(i));

    if (!(options & vtkKWVolumePropertyHelper::CopySkipOpacityUnitDistance))
      {
      target->SetScalarOpacityUnitDistance(
        i, source->GetScalarOpacityUnitDistance(i));
      }

    target->GetGradientOpacity(i)->DeepCopy(source->GetGradientOpacity(i));

    target->SetDisableGradientOpacity(i, source->GetDisableGradientOpacity(i));

    target->SetShade(i, source->GetShade(i));
    target->SetAmbient(i, source->GetAmbient(i));
    target->SetDiffuse(i, source->GetDiffuse(i));
    target->SetSpecular(i, source->GetSpecular(i));
    target->SetSpecularPower(i, source->GetSpecularPower(i));
    }
}

//----------------------------------------------------------------------------
void vtkKWVolumePropertyHelper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
