# ---------------------------------------------------------------------------
# KWWidgets_WRAP_TCL
# Macro around various VTK wrapping macros and wrapping-related settings

macro(KWWidgets_WRAP_TCL target src_list_name sources commands)

  if(VTK_MAJOR_VERSION AND VTK_MAJOR_VERSION LESS 5)
    
    # Attempt at supporting 4.4. 
    # This won't work anyway, as the lexer in VTK 4.x was updated later on
    # to support export macros not starting with VTK_ (say, KWWidgets_Export).

    vtk_wrap_tcl2(
      ${target}
      SOURCES ${src_list_name} ${sources}
      COMMANDS ${commands})
    
  else(VTK_MAJOR_VERSION AND VTK_MAJOR_VERSION LESS 5)
    
    if(NOT VTK_CMAKE_DIR)
      set(VTK_CMAKE_DIR "${VTK_SOURCE_DIR}/CMake")
    endif(NOT VTK_CMAKE_DIR)
    include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
    
    # VTK 5.0 really need some help. The evil VTK_WRAP_TCL3_INIT_DIR hack was
    # fixed on the 5.0 branch but people have downloaded earlier 5.0 as well.
    # Furthermore, old 5.0 can not be used once it has been installed, since
    # vtkWrapperInit.data.in is not installed properly: report that sad fact.

    if("${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}" EQUAL "5.0")
      if(VTK_INSTALL_PREFIX)
        if(NOT EXISTS "${VTK_CMAKE_DIR}/vtkWrapperInit.data.in")
          message("Sorry, you are using a VTK 5.0 that can not be used properly once it has been installed. You can either download a more recent VTK 5.0 snapshot from the CVS repository, or simply point KWWidgets to your VTK build directory instead of your VTK install directory.")
        endif(NOT EXISTS "${VTK_CMAKE_DIR}/vtkWrapperInit.data.in")
      else(VTK_INSTALL_PREFIX)
        set(VTK_WRAP_TCL3_INIT_DIR "${VTK_SOURCE_DIR}/Wrapping")
        set(VTK_WRAP_PYTHON3_INIT_DIR "${VTK_SOURCE_DIR}/Wrapping")
      endif(VTK_INSTALL_PREFIX)
    endif("${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}" EQUAL "5.0")

    vtk_wrap_tcl3(
      ${target}
      ${src_list_name} "${sources}" 
      "${commands}" 
      ${ARGN})
    
  endif(VTK_MAJOR_VERSION AND VTK_MAJOR_VERSION LESS 5)

endmacro(KWWidgets_WRAP_TCL)
