/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWEvent.cxx,v $
  Language:  C++
  Date:      $Date: 2003-04-25 18:41:53 $
  Version:   $Revision: 1.64 $

Copyright (c) 2000-2001 Kitware Inc. 469 Clifton Corporate Parkway,
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
#include "vtkKWEvent.h"
#include "vtkKWObject.h"
#include "vtkCommand.h"

static const char *vtkKWEventStrings[] = {
  "KWWidgetEvents",
  "MessageDialogInvokeEvent",
  // VV    
  "AngleVisibilityChangedEvent",
  "AnnotationColorChangedEvent",
  "ApplicationAreaChangedEvent",
  "BackgroundColorChangedEvent",
  "ColorImageEvent",
  "ContourAnnotationActiveChangedEvent",
  "ContourAnnotationAddAtPositionEvent",
  "ContourAnnotationAddEvent",
  "ContourAnnotationColorChangedEvent",
  "ContourAnnotationComputeStatisticsEvent",
  "ContourAnnotationLineWidthChangedEvent",
  "ContourAnnotationOpacityChangedEvent",
  "ContourAnnotationRemoveAllEvent",
  "ContourAnnotationRemoveEvent",
  "ContourAnnotationSurfacePropertyChangedEvent",
  "ContourAnnotationSurfaceQualityChangedEvent",
  "ContourAnnotationSurfaceRepresentationChangedEvent",
  "ContourAnnotationVisibilityChangedEvent",
  "ControlLeftMouseOptionChangedEvent",
  "ControlMiddleMouseOptionChangedEvent",
  "ControlRightMouseOptionChangedEvent",
  "CroppingPlanesPositionChangedEvent",
  "CroppingRegionFlagsChangedEvent",
  "CroppingRegionsVisibilityChangedEvent",
  "Cursor3DInteractiveStateChangedEvent",
  "Cursor3DPositionChangedEvent",
  "Cursor3DPositionChangingEvent",
  "Cursor3DTypeChangedEvent",
  "Cursor3DVisibilityChangedEvent",
  "Cursor3DXColorChangedEvent",
  "Cursor3DYColorChangedEvent",
  "Cursor3DZColorChangedEvent",
  "DistanceVisibilityChangedEvent",
  "EnableShadingEvent",
  "FilterRedoEvent",
  "FilterRemoveMeshEvent",
  "FilterUndoEvent",
  "GradientOpacityFunctionChangedEvent",
  "GradientOpacityFunctionPresetApplyEvent",
  "GradientOpacityStateChangedEvent",
  "GrayScaleImageEvent",
  "HistogramChangedEvent",
  "ImageAngleVisibilityChangedEvent",
  "ImageBackgroundColorChangedEvent",
  "ImageCameraFocalPointAndPositionChangedEvent",
  "ImageCameraResetEvent",
  "ImageCornerAnnotationChangedEvent",
  "ImageDistanceVisibilityChangedEvent",
  "ImageHeaderAnnotationColorChangedEvent",
  "ImageHeaderAnnotationTextChangedEvent",
  "ImageHeaderAnnotationVisibilityChangedEvent",
  "ImageMouseBindingChangedEvent",
  "ImageScaleBarColorChangedEvent",
  "ImageScaleBarVisibilityChangedEvent",
  "ImageSliceChangedEvent",
  "ImageZoomFactorChangedEvent",
  "InteractiveRenderStartEvent",
  "LeftMouseOptionChangedEvent",
  "LightActiveChangedEvent",
  "LightColorChangedEvent",
  "LightIntensityChangedEvent",
  "LightPositionChangedEvent",
  "LightVisibilityChangedEvent",
  "LightboxOrientationChangedEvent",
  "LightboxResolutionChangedEvent",
  "Marker2DColorChangedEvent",
  "Marker2DPositionChangedEvent",
  "Marker2DVisibilityChangedEvent",
  "Marker3DColorChangedEvent",
  "Marker3DAddMarkerEvent",
  "Marker3DPositionChangedEvent",
  "Marker3DRemoveAllMarkersEvent",
  "Marker3DRemoveSelectedMarkerEvent",
  "Marker3DVisibilityChangedEvent",
  "MaterialPropertyChangedEvent",
  "MiddleMouseOptionChangedEvent",
  "MouseBindingChangedEvent",
  "MouseOperationsChangedEvent",
  "MultiPassStillRenderEndEvent",
  "MultiPassStillRenderStageChangedEvent",
  "ObliqueProbeColorChangedEvent",
  "ObliqueProbeMovementEvent",
  "ObliqueProbeResetEvent",
  "ObliqueProbeScalarsVisibilityChangedEvent",
  "ObliqueProbeVisibilityChangedEvent",
  "PerspectiveViewAngleChangedEvent",
  "PluginFilterApplyEvent",
  "PluginFilterCancelEvent",
  "PluginFilterSelectEvent",
  "PrinterDPIChangedEvent",
  "ProbeInformationChangedEvent",
  "ProjectionTypeChangedEvent",
  "RenderEvent",
  "RightMouseOptionChangedEvent",
  "ScalarColorFunctionChangedEvent",
  "ScalarColorFunctionPresetApplyEvent",
  "ScalarOpacityFunctionChangedEvent",
  "ScalarOpacityFunctionPresetApplyEvent",
  "ScaleBarVisibilityChangedEvent",
  "ShiftLeftMouseOptionChangedEvent",
  "ShiftMiddleMouseOptionChangedEvent",
  "ShiftRightMouseOptionChangedEvent",
  "StandardInteractivityChangedEvent",
  "SurfacePropertyChangedEvent",
  "SwitchToVolumeProEvent",
  "TransferFunctionsChangedEvent",
  "TransferFunctionsChangingEvent",
  "UserInterfaceVisibilityChangedEvent",
  "ViewAnnotationChangedEvent",
  "ViewSelectedEvent",
  "VolumeBackgroundColorChangedEvent",
  "VolumeBlendModeChangedEvent",
  "VolumeBoundingBoxColorChangedEvent",
  "VolumeBoundingBoxVisibilityChangedEvent",
  "VolumeCameraResetEvent",
  "VolumeCornerAnnotationChangedEvent",
  "VolumeDistanceVisibilityChangedEvent",
  "VolumeFlySpeedChangedEvent",
  "VolumeHeaderAnnotationColorChangedEvent",
  "VolumeHeaderAnnotationTextChangedEvent",
  "VolumeHeaderAnnotationVisibilityChangedEvent",
  "VolumeMouseBindingChangedEvent",
  "VolumeOrientationMarkerColorChangedEvent",
  "VolumeOrientationMarkerVisibilityChangedEvent",
  "VolumePropertyChangedEvent",
  "VolumeReformatBoxVisibilityChangedEvent",
  "VolumeReformatChangedEvent",
  "VolumeReformatManipulationStyleChangedEvent",
  "VolumeReformatPlaneChangedEvent",
  "VolumeReformatThicknessChangedEvent",
  "VolumeScalarBarColorChangedEvent",
  "VolumeScalarBarTitleChangedEvent",
  "VolumeScalarBarVisibilityChangedEvent",
  "VolumeScaleBarColorChangedEvent",
  "VolumeScaleBarVisibilityChangedEvent",
  "VolumeStandardCameraViewEvent",
  "VolumeZSamplingChangedEvent",
  "WindowInterfaceChangedEvent",
  "WindowLayoutChangedEvent",
  "WindowLevelChangedEvent",
  "WindowLevelChangingEvent",
  "WindowLevelEndEvent",
  "WindowLevelResetEvent",
  // PV
  "ErrorMessageEvent",
  "InitializeTraceEvent",
  "ManipulatorModifiedEvent",
  "WarningMessageEvent",
  "WidgetModifiedEvent",
  //
  "FinalBogusNotUsedEvent",
  0
};

unsigned long vtkKWEvent::GetEventIdFromString(const char* cevent)
{
  unsigned long event = vtkCommand::GetEventIdFromString(cevent);
  if ( event != vtkCommand::NoEvent )
    {
    return event;
    }
  
  int cc;
  for ( cc = 0; vtkKWEventStrings[cc] != 0; cc ++ )
    {
    if ( strcmp(cevent, vtkKWEventStrings[cc]) == 0 )
      {
      return cc + vtkKWEvent::KWWidgetEvents;
      }
    }
  return vtkCommand::NoEvent;
}

const char *vtkKWEvent::GetStringFromEventId(unsigned long event)
{
  static unsigned long numevents = 0;
  
  // find length of table
  if (!numevents)
    {
    while (vtkKWEventStrings[numevents] != NULL)
      {
      numevents++;
      }
    }
  if ( event < vtkKWEvent::KWWidgetEvents ) 
    {
    return vtkCommand::GetStringFromEventId(event);
    }

  event -= 2000;

  if (event < numevents)
    {
    return vtkKWEventStrings[event];
    }
  else
    {
    return "UnknownEvent";
    }
}

