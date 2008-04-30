subdirs(
  HelloWorld
  WidgetsTour
  WindowWithPanels
  )

if(KWWidgets_BUILD_VTK_WIDGETS)
  subdirs(
    MedicalImageViewer
    PolygonalObjectViewer
    )
endif(KWWidgets_BUILD_VTK_WIDGETS)
