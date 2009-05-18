set(CTEST_PROJECT_NAME "KWWidgets")
set(CTEST_NIGHTLY_START_TIME "21:30:00 EDT")

set(CTEST_DROP_METHOD "http")
set(CTEST_DROP_SITE "www.cdash.org")
set(CTEST_DROP_LOCATION "/CDash/submit.php?project=KWWidgets")
set(CTEST_DROP_SITE_CDASH TRUE)
set(CTEST_TRIGGER_SITE "http://${CTEST_DROP_SITE}/cgi-bin/Submit-KWWidgets-TestingResults.cgi")
