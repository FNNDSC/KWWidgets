# ---------------------------------------------------------------------------
# KWWidgets_GET_SOURCE_REVISION_AND_DATE
# Get vtkKWWidgetsVersion's source revision and date and store them in
# ${revision_varname} and ${date_varname} respectively.
#
# This macro can be used to require a specific revision of the KWWidgets
# library in between version changes.
# For example:
#   INCLUDE("${KWWidgets_CMAKE_DIR}/KWWidgetsVersionMacros.cmake")
#   KWWidgets_GET_SOURCE_REVISION_AND_DATE(source_revision source_date)
#   IF(source_revision LESS 1.4)
#    MESSAGE(FATAL_ERROR "Sorry, your KWWidgets library was last updated on ${source_date}. Its source revision, according to vtkKWWidgetsVersion.h, is ${source_revision}. Please update to a newer revision.")
#   ENDIF(source_revision LESS 1.4)

macro(KWWidgets_GET_SOURCE_REVISION_AND_DATE
    revision_varname
    date_varname)

  set(${revision_varname})
  set(${date_varname})
  foreach(dir ${KWWidgets_INCLUDE_DIRS} ${KWWidgets_INCLUDE_PATH})
    set(file "${dir}/vtkKWWidgetsVersion.h")
    if(EXISTS ${file})
      file(READ ${file} file_contents)
      string(REGEX REPLACE "(.*Revision: )([0-9]+\\.[0-9]+)( .*)" "\\2" 
        ${revision_varname} "${file_contents}")
      string(REGEX REPLACE "(.*Date: )(.+)( \\$.*)" "\\2" 
        ${date_varname} "${file_contents}")
    endif(EXISTS ${file})
  endforeach(dir)

  if(NOT ${revision_varname} OR NOT ${date_varname})
    message("Sorry, vtkKWWidgetsVersion's source revision could not be found, either because vtkKWWidgetsVersion.h is nowhere in sight, or its contents could not be parsed successfully.")
  endif(NOT ${revision_varname} OR NOT ${date_varname})

endmacro(KWWidgets_GET_SOURCE_REVISION_AND_DATE)
