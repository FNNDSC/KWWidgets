# Make sure we have Tcl

include(FindTCL)
if (NOT TCL_LIBRARY)
  message("VTK_WRAP_TCL is ON, but the Tcl library was not found.  Please set TCL_LIBRARY." "Error")
endif (NOT TCL_LIBRARY)

set(KWWidgets_TCL_LIBNAME_PREFIX "${CMAKE_SHARED_LIBRARY_PREFIX}")

# Configure the Tcl package index files for the build tree.
# You can then point TCLLIBPATH to the Wrapping/Tcl dir in the right
# build directory (so that the right shared lib is loaded)

set(KWWidgets_TCL_SCRIPT_DIR "${KWWidgets_SOURCE_DIR}/Wrapping/Tcl")
set(KWWidgets_TCL_CONFIGURATION_TYPES ${CMAKE_CONFIGURATION_TYPES})
if(KWWidgets_TCL_CONFIGURATION_TYPES)
  foreach(config ${KWWidgets_TCL_CONFIGURATION_TYPES})
    set(KWWidgets_TCL_LIBRARY_DIR "${LIBRARY_OUTPUT_PATH}/${config}")
    configure_file(${KWWidgets_SOURCE_DIR}/Wrapping/Tcl/pkgIndex.tcl.in
      ${KWWidgets_BINARY_DIR}/Wrapping/Tcl/${config}/pkgIndex.tcl
      @ONLY IMMEDIATE)
  endforeach(config)
else(KWWidgets_TCL_CONFIGURATION_TYPES)
  set(KWWidgets_TCL_LIBRARY_DIR "${LIBRARY_OUTPUT_PATH}")
  configure_file(${KWWidgets_SOURCE_DIR}/Wrapping/Tcl/pkgIndex.tcl.in
    ${KWWidgets_BINARY_DIR}/Wrapping/Tcl/pkgIndex.tcl
    @ONLY IMMEDIATE)
endif(KWWidgets_TCL_CONFIGURATION_TYPES)

# Configure the Tcl package index file for the install tree.
# You can then point TCLLIBPATH to the lib/KWWidgets/tcl dir in the right
# installation directory
# We configure a new pkgIndex file that uses KWWidgets_TCL_LIBRARY_DIR
# It has to be in Install/Hide so that Tcl does not find it automatically

set(KWWidgets_TCL_SCRIPT_DIR "[file dirname [info script]]")
if(UNIX)
  # KWWidgets_TCL_LIBRARY_DIR is used to find the library in the bin
  # directory relative to the pkgIndex file located in the lib directory
  set(KWWidgets_TCL_LIBRARY_DIR "[file dirname [file dirname [info script]]]")
else(UNIX)
  set(KWWidgets_TCL_LIBRARY_DIR
    "[file join [file dirname [file dirname [file dirname [file dirname [info script]]]]] bin]")
endif(UNIX)

configure_file(${KWWidgets_SOURCE_DIR}/Wrapping/Tcl/pkgIndex.tcl.in
  ${KWWidgets_BINARY_DIR}/Wrapping/Tcl/Install/Hide/pkgIndex.tcl
  @ONLY IMMEDIATE)
if(NOT KWWidgets_INSTALL_NO_RUNTIME)
  install_files(${KWWidgets_INSTALL_LIB_DIR}/tcl FILES
    ${KWWidgets_BINARY_DIR}/Wrapping/Tcl/Install/Hide/pkgIndex.tcl)
endif(NOT KWWidgets_INSTALL_NO_RUNTIME)

# Install kits

subdirs(kwwidgets)
