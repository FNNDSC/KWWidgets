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
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
// .NAME vtkKWVolumePropertyHelper - a render widget for volumes
// .SECTION Description

#ifndef __vtkKWVolumePropertyHelper_h
#define __vtkKWVolumePropertyHelper_h

#include "vtkObject.h"
#include "vtkKWWidgets.h" // Needed for export symbols directives

class vtkVolumeProperty;
class vtkKWHistogramSet;
class vtkImageData;

class KWWidgets_EXPORT vtkKWVolumePropertyHelper : public vtkObject
{
public:
  static vtkKWVolumePropertyHelper* New();
  vtkTypeRevisionMacro(vtkKWVolumePropertyHelper, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Apply a normalized scalar opacity preset to a specific component
  // Such normalized values can later be converted to real scalar values
  // given an image data and/or an histogram (see ConvertNormalizedToReal).
  //BTX
  enum
  {
    // 2 points "ramp": 0% opacity at 0% to 20% opacity at 100%
    ScalarOpacityRamp0At0To20At100 = 0,

    // 2 points "ramp": 0% opacity at 25% to 100% opacity at 50%
    ScalarOpacityRamp0At25To100At50,

    // 2 points "ramp": 0% opacity at 50% to 100% opacity at 75%
    ScalarOpacityRamp0At50To100At75,

    // 4 steps: 0% opacity until 25%, 20% opacity from 25% to 50%,
    // 40% opacity from 50% to 75%, and 100% opacity from 75% to 100%.
    ScalarOpacitySteps0To25Then20To50Then40To75Then100To100
  };
  //ETX
  static void ApplyScalarOpacityPreset(
    vtkVolumeProperty *prop, int comp, int preset);

  // Description:
  // Apply a normalized color transfer function preset to a specific component.
  // Such normalized values can later be converted to real scalar values
  // given an image data and/or an histogram (see ConvertNormalizedToReal).
  // Some presets will set 2, 3, 4 or more color points. You can specify
  // any number of colors though, the missing ones will be picked/rotated
  // from the one you passed (i.e. if 2 points are created and one color is
  // provided, the same color will be used at both locations). 
  // Convenience methods are provided to pass one, two, three or four RGB
  // color(s) directly instead of an array of RGB colors (i.e. an array of
  // double triplets).
  //BTX
  enum
  {
    // 2 points "ramp": color at 0%, color at 100%
    RGBTransferFunctionRampAt0At100 = 0,

    // 3 points "ramp": color at 0%, color at 70%, color at 100%
    RGBTransferFunctionRampAt0At70At100,

    // 4 steps: color from 0% to  25%, color from 25% to 50%,
    // color from 50% to 75%, color from 75% to 100%.
    RGBTransferFunctionStepsTo25To50To75To100
  };
  //ETX
  static void ApplyRGBTransferFunctionPreset(
    vtkVolumeProperty *prop, int comp, int preset, 
    int nb_colors, double **rgb);
  static void ApplyRGBTransferFunctionPreset(
    vtkVolumeProperty *prop, int comp, int preset, 
    double rgb[3]);
  static void ApplyRGBTransferFunctionPreset(
    vtkVolumeProperty *prop, int comp, int preset, 
    double rgb1[3], double rgb2[3]);
  static void ApplyRGBTransferFunctionPreset(
    vtkVolumeProperty *prop, int comp, int preset, 
    double rgb1[3], double rgb2[3], double rgb3[3]);
  static void ApplyRGBTransferFunctionPreset(
    vtkVolumeProperty *prop, int comp, int preset, 
    double rgb1[3], double rgb2[3], double rgb3[3], double rgb4[3]);

  // Description:
  // Apply a normalized gradient opacity preset to a specific component
  // Such normalized values can later be converted to real scalar values
  // given an image data and/or an histogram (see ConvertNormalizedToReal).
  //BTX
  enum
  {
    // No edge detection
    GradientOpacityNoEdge = 0,

    // Medium edge detection
    GradientOpacityMediumEdge,

    // Strong edge detection
    GradientOpacityStrongEdge
  };
  //ETX
  static void ApplyGradientOpacityPreset(
    vtkVolumeProperty *prop, int comp, int preset);

  // Description:
  // Apply a lighting preset to a specific component
  //BTX
  enum
  {
    // Full ambient eliminating all directional shading
    LightingFullAmbient = 0,

    // Dull material properties (no specular lighting)
    LightingDull,

    // Smooth material properties (moderate specular lighting)
    LightingSmooth,

    // Shiny material properties (high specular lighting)
    LightingShiny
  };
  //ETX
  static void ApplyLightingPreset(
    vtkVolumeProperty *prop, int comp, int preset);

  // Description:
  // Apply a normalized preset, i.e. a combination of a scalar opacity 
  // preset, a color transfer function preset, a gradient opacity preset
  // and a lighting preset (see ApplyScalarOpacityPreset, 
  // ApplyRGBTransferFunctionPreset, ApplyGradientOpacityPreset and
  // ApplyLightingPreset).
  // IMPORTANT: note that the vtkVolumeProperty's IndependentComponenents
  // flag will be taken into account when setting the transfer functions
  // for each components of the property.
  // See ConvertNormalizedRange and/or ApplyPresetAndConvertNormalizedRange
  // to convert the normalized range to real scalar ranges.
  //BTX
  enum
  {
    // ScalarOpacityRamp0At0To20At100
    // RGBTransferFunctionRampAt0At100 
    //   => white for comp 0, then colors (red, green, blue, yellow)
    // GradientOpacityNoEdge
    // LightingDull
    // Shading Off
    Preset1 = 0,

    // ScalarOpacityRamp0At0To20At100
    // RGBTransferFunctionRampAt0At100
    //   => black to white for comp 0, then colors to white
    // GradientOpacityNoEdge
    // LightingDull
    // Shading Off
    Preset2,

    // ScalarOpacityRamp0At0To20At100
    // RGBTransferFunctionRampAt0At70At100
    //   => rainbox (blue -> green -> red) for all comps
    // GradientOpacityNoEdge
    // LightingDull
    // Shading Off
    Preset3,

    // ScalarOpacityRamp0At25To100At50
    // RGBTransferFunctionRampAt0At100
    //   => tan for comp 0, then other colors (red, green, blue, yellow)
    // LightingDull
    // Shading On
    Preset4,

    // ScalarOpacityRamp0At50To100At75
    // RGBTransferFunctionRampAt0At100
    //   => tan for comp 0, then other colors (red, green, blue, yellow)
    // GradientOpacityNoEdge
    // LightingDull
    // Shading On
    Preset5,

    // ScalarOpacitySteps0To25Then20To50Then40To75Then100To100
    // RGBTransferFunctionStepsTo25To50To75To100
    //   => red, green, blue, yellow steps for all comps
    // GradientOpacityNoEdge
    // LightingDull
    // Shading On
    Preset6
  };
  //ETX
  static void ApplyPreset(vtkVolumeProperty *prop, int preset);

  // Description:
  // Retrieve some solid color RGB presets.
  //BTX
  enum
  {
    Black = 0,
    White,
    Gray3,
    Red,
    Green,
    Blue,
    Yellow,
    Magenta,
    Cyan,
    Tan
  };
  //ETX
  static double* GetRGBColor(int preset);

  // Description:
  // Convert a normalized volume property to real scalar ranges.
  // The contents of normalized_prop is entirely deep-copied to
  // target_prop (EXCEPT the ScalarOpacityUnitDistance).
  // The points are all moved from normalized space to real scalar ranges
  // computed either from the image's data range or from its histogram (if any).
  static void ConvertNormalizedRange(
    vtkVolumeProperty *normalized_prop, 
    vtkVolumeProperty *target_prop, 
    vtkImageData *image,
    int independent_component,
    vtkKWHistogramSet *histogram_set
    );

  // Description:
  // Convenience method that will create a temporary volume property, 
  // use ApplyPreset to convert it to a normalized preset, then convert
  // its normalized values to real scalar ranges by calling
  // ConvertNormalizedRange.
  // The points are all moved from normalized space to real scalar ranges
  // computed either from the image's data range or from its histogram (if any).
  static void ApplyPresetAndConvertNormalizedRange(
    int preset,
    vtkVolumeProperty *target_prop, 
    vtkImageData *image,
    int independent_component,
    vtkKWHistogramSet *histogram_set
    );

  // Description:
  // Deep copy a volume property from source to target.
  static void DeepCopyVolumeProperty(
    vtkVolumeProperty *target, vtkVolumeProperty *source);

  // Description:
  // Copy a volume property from source to target, optionally skipping
  // some fields.
  //BTX
  enum
  {
    CopySkipOpacityUnitDistance   = 1,
    CopySkipIndependentComponents = 2
  };
  //ETX
  static void CopyVolumeProperty(
    vtkVolumeProperty *target, vtkVolumeProperty *source, int options);

protected:
  vtkKWVolumePropertyHelper() {};
  ~vtkKWVolumePropertyHelper() {};
  
private:
  vtkKWVolumePropertyHelper(const vtkKWVolumePropertyHelper&);  // Not implemented
  void operator=(const vtkKWVolumePropertyHelper&);  // Not implemented
};

#endif
