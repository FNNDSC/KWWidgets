# ---------------------------------------------------------------------------
# KWWidgets_CREATE_RC_FILE
# This macro can be used to create a Win32 .rc file out of the resource
# template found in Resources/KWWidgets.rc.in. Such a Win32 resource file
# can be added to the list of source files associated to a specific
# application/executable, and can be used to customize both the 16x16 and
# 32x32 icons, as well as the informations that are displayed in the "Version"
# tab of its properties panel.
#
# This macro accepts parameters as arg/value pairs or as a single arg if
# the arg is described as boolean (same as setting the arg to 1). The
# args can be specificied in any order and some of them are optionals.
#
# Required arguments:
# RC_FILENAME (filename): pathname of the resource file to create
#    Default to "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rc" if not found.
#
# Optional arguments:
# RC_ICON_BASENAME (path): path that will be used as basename for the 16x16
#    and 32x32 icons. The full pathname to the 16x16 icon is set to this
#    basename suffixed by "Icon16.ico" (Icon32.ico for the 32x32).
#    Default to "Resources/KWWidgets" if not found.
# RC_MAJOR_VERSION (string): major version number of the application
#    Default to 1 if not found.
# RC_MINOR_VERSION (string): minon version number of the application
#    Default to 0 if not found.
# RC_APPLICATION_NAME (string): the application name
#    Default to ${PROJECT_NAME} if not found.
# RC_APPLICATION_FILENAME (basename): the basename of application file, i.e.
#    its name without path or file extension.
#    Default to ${RC_APPLICATION_NAME}
# RC_COMPANY_NAME (string): the name of the company associated to the app
#    Default to "Unknown" if not found.
# RC_COPYRIGHT_YEAR (string): the copyright year(s) that apply to that 
#    application (say, 2005, or 2003-2006)
#    Default to "2006" if not found.

macro(KWWidgets_CREATE_RC_FILE)

  set(notset_value             "__not_set__")

  # Provide some reasonable defaults

  set(RC_FILENAME             "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rc")
  set(RC_ICON_BASENAME        "Resources/KWWidgets")
  set(RC_MAJOR_VERSION        1)
  set(RC_MINOR_VERSION        0)
  set(RC_APPLICATION_NAME     ${PROJECT_NAME})
  set(RC_APPLICATION_FILENAME ${notset_value})
  set(RC_COMPANY_NAME         "Unknown")
  set(RC_COPYRIGHT_YEAR       2006)

  # Parse the arguments

  set(valued_parameter_names "^(___RC_FILENAME|___RC_ICON_BASENAME|___RC_MAJOR_VERSION|___RC_MINOR_VERSION|___RC_APPLICATION_NAME|___RC_APPLICATION_FILENAME|___RC_COMPANY_NAME|___RC_COPYRIGHT_YEAR)$")
  set(boolean_parameter_names "^$")
  set(list_parameter_names "^$")

  set(next_arg_should_be_value 0)
  set(prev_arg_was_boolean 0)
  set(prev_arg_was_list 0)
  set(unknown_parameters)
  
  string(REGEX REPLACE ";;" ";FOREACH_FIX;" parameter_list "${ARGV}")
  foreach(arg ${parameter_list})

    if("${arg}" STREQUAL "FOREACH_FIX")
      set(arg "")
    endif("${arg}" STREQUAL "FOREACH_FIX")

    set(___arg "___${arg}")
    set(matches_valued 0)
    if("${___arg}" MATCHES ${valued_parameter_names})
      set(matches_valued 1)
    endif("${___arg}" MATCHES ${valued_parameter_names})

    set(matches_boolean 0)
    if("${___arg}" MATCHES ${boolean_parameter_names})
      set(matches_boolean 1)
    endif("${___arg}" MATCHES ${boolean_parameter_names})

    set(matches_list 0)
    if("${___arg}" MATCHES ${list_parameter_names})
      set(matches_list 1)
    endif("${___arg}" MATCHES ${list_parameter_names})
    
    if(matches_valued OR matches_boolean OR matches_list)
      if(prev_arg_was_boolean)
        set(${prev_arg_name} 1)
      else(prev_arg_was_boolean)
        if(next_arg_should_be_value AND NOT prev_arg_was_list)
          message(FATAL_ERROR 
            "Found ${arg} instead of value for ${prev_arg_name}")
        endif(next_arg_should_be_value AND NOT prev_arg_was_list)
      endif(prev_arg_was_boolean)
      set(next_arg_should_be_value 1)
      set(prev_arg_was_boolean ${matches_boolean})
      set(prev_arg_was_list ${matches_list})
      set(prev_arg_name ${arg})
    else(matches_valued OR matches_boolean OR matches_list)
      if(next_arg_should_be_value)
        if(prev_arg_was_boolean)
          if(NOT "${arg}" STREQUAL "1" AND NOT "${arg}" STREQUAL "0")
            message(FATAL_ERROR 
              "Found ${arg} instead of 0 or 1 for ${prev_arg_name}")
          endif(NOT "${arg}" STREQUAL "1" AND NOT "${arg}" STREQUAL "0")
        endif(prev_arg_was_boolean)
        if(prev_arg_was_list)
          set(${prev_arg_name} ${${prev_arg_name}} ${arg})
        else(prev_arg_was_list)
          set(${prev_arg_name} ${arg})
          set(next_arg_should_be_value 0)
        endif(prev_arg_was_list)
      else(next_arg_should_be_value)
        set(unknown_parameters ${unknown_parameters} ${arg})
      endif(next_arg_should_be_value)
      set(prev_arg_was_boolean 0)
    endif(matches_valued OR matches_boolean OR matches_list)

  endforeach(arg)

  if(next_arg_should_be_value)
    if(prev_arg_was_boolean)
      set(${prev_arg_name} 1)
    else(prev_arg_was_boolean)
      message(FATAL_ERROR "Missing value for ${prev_arg_name}")
    endif(prev_arg_was_boolean)
  endif(next_arg_should_be_value)
  if(unknown_parameters)
    message(FATAL_ERROR "Unknown parameter(s): ${unknown_parameters}")
  endif(unknown_parameters)

  # Fix some defaults

  if(${RC_APPLICATION_FILENAME} STREQUAL ${notset_value})
    set(RC_APPLICATION_FILENAME ${RC_APPLICATION_NAME})
  endif(${RC_APPLICATION_FILENAME} STREQUAL ${notset_value})

  # Create the resource file

  include_directories(${VTK_TK_RESOURCES_DIR})
  configure_file(${KWWidgets_RESOURCES_DIR}/KWWidgets.rc.in ${RC_FILENAME})

endmacro(KWWidgets_CREATE_RC_FILE)

