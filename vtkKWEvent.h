/*=========================================================================

  Module:    $RCSfile: vtkKWEvent.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWEvent - Event enumeration.

#ifndef __vtkKWEvent_h
#define __vtkKWEvent_h

#include "vtkKWWidgets.h" // Needed for export symbols directives

class KWWIDGETS_EXPORT vtkKWEvent
{
public:

  static const char* GetStringFromEventId(unsigned long event);
  static unsigned long GetEventIdFromString(const char* event);

  enum 
  {
    KWWidgetEvents = 2000,
    MessageDialogInvokeEvent,
    FocusInEvent,
    FocusOutEvent,
    // VV    
    AngleVisibilityChangedEvent,
    AnnotationColorChangedEvent,
    ApplicationAreaChangedEvent,
    BackgroundColorChangedEvent,
    ContourAnnotationActiveChangedEvent,
    ContourAnnotationAddAtPositionEvent,
    ContourAnnotationAddEvent,
    ContourAnnotationColorByScalarsChangedEvent,
    ContourAnnotationColorChangedEvent,
    ContourAnnotationComputeStatisticsEvent,
    ContourAnnotationLineWidthChangedEvent,
    ContourAnnotationOpacityChangedEvent,
    ContourAnnotationRemoveAllEvent,
    ContourAnnotationRemoveEvent,
    ContourAnnotationSurfacePropertyChangedEvent,
    ContourAnnotationSurfaceQualityChangedEvent,
    ContourAnnotationSurfaceRepresentationChangedEvent,
    ContourAnnotationVisibilityChangedEvent,
    ControlLeftMouseOptionChangedEvent,
    ControlMiddleMouseOptionChangedEvent,
    ControlRightMouseOptionChangedEvent,
    CroppingPlanesPositionChangedEvent,
    CroppingRegionFlagsChangedEvent,
    CroppingRegionsVisibilityChangedEvent,
    Cursor3DInteractiveStateChangedEvent,
    Cursor3DPositionChangedEvent,
    Cursor3DPositionChangingEvent,
    Cursor3DTypeChangedEvent,
    Cursor3DVisibilityChangedEvent,
    Cursor3DXColorChangedEvent,
    Cursor3DYColorChangedEvent,
    Cursor3DZColorChangedEvent,
    DataFileNameChangedEvent,
    DistanceVisibilityChangedEvent,
    EnableShadingEvent,
    GradientOpacityFunctionChangedEvent,
    GradientOpacityFunctionPresetApplyEvent,
    GradientOpacityStateChangedEvent,
    HistogramChangedEvent,
    ImageAngleVisibilityChangedEvent,
    ImageBackgroundColorChangedEvent,
    ImageCameraFocalPointAndPositionChangedEvent,
    ImageCameraResetEvent,
    ImageColorMappingEvent,
    ImageCornerAnnotationChangedEvent,
    ImageDistanceVisibilityChangedEvent,
    ImageHeaderAnnotationChangedEvent,
    ImageInterpolateEvent,
    ImageMouseBindingChangedEvent,
    ImageScaleBarColorChangedEvent,
    ImageScaleBarVisibilityChangedEvent,
    ImageSliceChangedEvent,
    ImageZoomFactorChangedEvent,
    InteractiveRenderStartEvent,
    LeftMouseOptionChangedEvent,
    LightActiveChangedEvent,
    LightColorChangedEvent,
    LightIntensityChangedEvent,
    LightPositionChangedEvent,
    LightVisibilityChangedEvent,
    LightboxOrientationChangedEvent,
    LightboxResolutionChangedEvent,
    Marker2DColorChangedEvent,
    Marker2DPositionChangedEvent,
    Marker2DVisibilityChangedEvent,
    Marker3DAddMarkerEvent,
    Marker3DAddMarkerInGroupEvent,
    Marker3DAddMarkersGroupEvent,
    Marker3DColorChangedEvent,
    Marker3DPositionChangedEvent,
    Marker3DRemoveAllMarkerGroupsEvent,
    Marker3DRemoveAllMarkersEvent,
    Marker3DRemoveSelectedMarkerEvent,
    Marker3DRemoveSelectedMarkerGroupEvent,
    Marker3DSelectMarkerGroupEvent,
    Marker3DVisibilityChangedEvent,
    MaterialPropertyChangedEvent,
    MaterialPropertyChangingEvent,
    MiddleMouseOptionChangedEvent,
    MouseBindingChangedEvent,
    MouseOperationsChangedEvent,
    NotebookHidePageEvent,
    NotebookPinPageEvent,
    NotebookRaisePageEvent,
    NotebookShowPageEvent,
    NotebookUnpinPageEvent,
    ObjectActionEvent,
    ObjectNameChangedEvent,
    ObjectSetChangedEvent,
    ObjectStateChangedEvent,
    ObliqueProbeColorChangedEvent,
    ObliqueProbeMovementEvent,
    ObliqueProbeResetEvent,
    ObliqueProbeScalarsVisibilityChangedEvent,
    ObliqueProbeVisibilityChangedEvent,
    PerspectiveViewAngleChangedEvent,
    PluginFilterApplyEvent,
    PluginFilterApplyPrepareEvent,
    PluginFilterCancelEvent,
    PluginFilterListAddedEvent,
    PluginFilterListEvent,
    PluginFilterListRemovedEvent,
    PluginFilterRedoEvent,
    PluginFilterRemoveMeshEvent,
    PluginFilterSelectEvent,
    PluginFilterUndoEvent,
    ProbeInformationChangedEvent,
    ProbeInformationOffEvent,
    ProjectionTypeChangedEvent,
    ReceiveRemoteSessionEvent,
    RenderEvent,
    RenderWidgetInSelectionFrameChangedEvent,
    RightMouseOptionChangedEvent,
    ScalarColorFunctionChangedEvent,
    ScalarColorFunctionPresetApplyEvent,
    ScalarComponentChangedEvent,
    ScalarComponentWeightChangedEvent,
    ScalarComponentWeightChangingEvent,
    ScalarOpacityFunctionChangedEvent,
    ScalarOpacityFunctionPresetApplyEvent,
    ScaleBarVisibilityChangedEvent,
    SelectionChangedEvent,
    ShiftLeftMouseOptionChangedEvent,
    ShiftMiddleMouseOptionChangedEvent,
    ShiftRightMouseOptionChangedEvent,
    SplineSurfaceAddEvent,
    SplineSurfaceActiveChangedEvent,
    SplineSurfaceColorChangedEvent,
    SplineSurfaceLineWidthChangedEvent,
    SplineSurfaceNumberOfHandlesChangedEvent,
    SplineSurfaceHandleChangedEvent,
    SplineSurface2DHandleChangedEvent,
    SplineSurfaceOpacityChangedEvent,
    SplineSurfacePropertyChangedEvent,
    SplineSurfaceQualityChangedEvent,
    SplineSurfaceRemoveEvent,
    SplineSurfaceRemoveAllEvent,
    SplineSurfaceRepresentationChangedEvent,
    SplineSurfaceVisibilityChangedEvent,
    SplineSurfaceHandlePositionChangedEvent,
    SplineSurface2DHandlePositionChangedEvent,
    StandardInteractivityChangedEvent,
    SurfacePropertyChangedEvent,
    SurfacePropertyChangingEvent,
    SwitchToVolumeProEvent,
    TimeChangedEvent,
    TransferFunctionsChangedEvent,
    TransferFunctionsChangingEvent,
    UserInterfaceVisibilityChangedEvent,
    ViewAnnotationChangedEvent,
    ViewSelectedEvent,
    VolumeBackgroundColorChangedEvent,
    VolumeBlendModeChangedEvent,
    VolumeBoundingBoxColorChangedEvent,
    VolumeBoundingBoxVisibilityChangedEvent,
    VolumeCameraResetEvent,
    VolumeCornerAnnotationChangedEvent,
    VolumeDistanceVisibilityChangedEvent,
    VolumeFlySpeedChangedEvent,
    VolumeHeaderAnnotationChangedEvent,
    VolumeMapperRenderEndEvent,
    VolumeMapperRenderStartEvent,
    VolumeMaterialPropertyChangedEvent,
    VolumeMaterialPropertyChangingEvent,
    VolumeMouseBindingChangedEvent,
    VolumeOrientationMarkerColorChangedEvent,
    VolumeOrientationMarkerVisibilityChangedEvent,
    VolumePropertyChangedEvent,
    VolumePropertyChangingEvent,
    VolumeReformatBoxVisibilityChangedEvent,
    VolumeReformatChangedEvent,
    VolumeReformatManipulationStyleChangedEvent,
    VolumeReformatPlaneChangedEvent,
    VolumeReformatThicknessChangedEvent,
    VolumeScalarBarComponentChangedEvent,
    VolumeScalarBarWidgetChangedEvent,
    VolumeScaleBarColorChangedEvent,
    VolumeScaleBarVisibilityChangedEvent,
    VolumeStandardCameraViewEvent,
    VolumeZSamplingChangedEvent,
    WindowInterfaceChangedEvent,
    WindowLayoutChangedEvent,
    WindowLevelChangedEvent,
    WindowLevelChangingEndEvent,
    WindowLevelChangingEvent,
    WindowLevelResetEvent,
    // PV
    ErrorMessageEvent,
    InitializeTraceEvent,
    ManipulatorModifiedEvent,
    WarningMessageEvent,
    SourceDeletedEvent,
    //
    FinalBogusNotUsedEvent
  };
};

#endif
