set(NIGHTLY_START_TIME "21:30:00 EDT")

if(DROP_METHOD MATCHES http)
  set(DROP_SITE "public.kitware.com")
  set(DROP_LOCATION "/cgi-bin/HTTPUploadDartFile.cgi")
else(DROP_METHOD MATCHES http)
  set(DROP_SITE "public.kitware.com")
  set(DROP_LOCATION "/incoming")
  set(DROP_SITE_USER "ftpuser")
  set(DROP_SITE_PASSWORD "public")
endif(DROP_METHOD MATCHES http)

set(TRIGGER_SITE 
  "http://${DROP_SITE}/cgi-bin/Submit-KWWidgets-TestingResults.pl")

set(PROJECT_URL "http://www.kwwidgets.org")

set(ROLLUP_URL "http://${DROP_SITE}/cgi-bin/KWWidgets-rollup-dashboard.sh")

set(CVS_WEB_URL "http://kwwidgets.org/cgi-bin/viewcvs.cgi/")
set(CVS_WEB_CVSROOT "KWWidgets")

set(USE_DOXYGEN "On")
set(DOXYGEN_URL "http://www.kwwidgets.org/doc/nightly/html/")

set(USE_GNATS "On")
set(GNATS_WEB_URL "${PROJECT_URL}/Bug/")

configure_file(
  ${KWWidgets_SOURCE_DIR}/Examples/Resources/KWWidgetsSplashScreenSmall.gif 
  ${KWWidgets_BINARY_DIR}/Testing/HTML/TestingResults/Icons/Logo.gif 
  COPYONLY)
