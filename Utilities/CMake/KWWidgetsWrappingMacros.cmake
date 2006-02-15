# ---------------------------------------------------------------------------
# KWWidgets_WRAP_TCL
# Macro around various VTK wrapping macros and wrapping-related settings

MACRO(KWWidgets_WRAP_TCL target src_list_name sources commands)

  IF(VTK_MAJOR_VERSION AND VTK_MAJOR_VERSION LESS 5)
      
    # Attempt at supporting 4.4. 
    # This won't work anyway, as the lexer in VTK 4.x was updated later on
    # to support export macros not starting with VTK_ (say, KWWidgets_Export).

    VTK_WRAP_TCL2(
      ${target}
      SOURCES ${src_list_name} ${sources}
      COMMANDS ${commands})
    
  ELSE(VTK_MAJOR_VERSION AND VTK_MAJOR_VERSION LESS 5)
    
    IF(NOT VTK_CMAKE_DIR)
      SET(VTK_CMAKE_DIR "${VTK_SOURCE_DIR}/CMake")
    ENDIF(NOT VTK_CMAKE_DIR)
    INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
    
    VTK_WRAP_TCL3(
      ${target}
      ${src_list_name} "${sources}" 
      "${commands}" 
      ${ARGN})
    
  ENDIF(VTK_MAJOR_VERSION AND VTK_MAJOR_VERSION LESS 5)

ENDMACRO(KWWidgets_WRAP_TCL)
