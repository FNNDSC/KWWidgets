subdirs(
  Callbacks
  FileBrowserDialog
  HelloWorld
  Theme
  WidgetsTour
  WindowWithPanels
  WizardDialog
  )

if(KWWidgets_USE_INTERNATIONALIZATION)
  subdirs(Internationalization)
endif(KWWidgets_USE_INTERNATIONALIZATION)

if(KWWidgets_BUILD_VTK_WIDGETS)
  subdirs(
    MedicalImageViewer
    PolygonalObjectViewer
    )
  if(KWWidgets_USE_SOVIEWER)
    subdirs(SOViewer)
  endif(KWWidgets_USE_SOVIEWER)
endif(KWWidgets_BUILD_VTK_WIDGETS)
