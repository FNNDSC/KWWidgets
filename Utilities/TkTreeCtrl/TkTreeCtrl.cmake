MACRO(TkTreeCtrl_GET_LIBRARY supported sources include_path libs)

  IF(DEFINED ${supported})
    SET(default_val ${supported})
  ELSE(DEFINED ${supported})
    SET(default_val 0)
  ENDIF(DEFINED ${supported})

  # Visual Studio 6 does not support/provide uxtheme.h
  # Borland fails on tkTreeDrag.c 
  # (http://public.kitware.com/pipermail/kwwidgets/2007-August/000491.html)

  IF(CMAKE_GENERATOR MATCHES "Visual Studio 6" OR BORLAND)
    SET(default_val 0)
  ENDIF(CMAKE_GENERATOR MATCHES "Visual Studio 6" OR BORLAND)

  OPTION(${supported}
    "Enable TkTreeCtrl support in KWWidgets." ${default_val})
  MARK_AS_ADVANCED(${supported})

  SET(${sources})
  SET(${include_path})
  SET(${libs})
  
  IF(${supported})
    SET(_tktreectrl_path "${KWWidgets_SOURCE_DIR}/Utilities/TkTreeCtrl")
    
    SET(_tktreectrl_include_path
      ${VTK_TK_INTERNAL_DIR}
      ${_tktreectrl_path}/generic
      )

    SET(_tktreectrl_srcs 
      ${_tktreectrl_path}/generic/qebind.c
      ${_tktreectrl_path}/generic/tkTreeColumn.c
      ${_tktreectrl_path}/generic/tkTreeCtrl.c
      ${_tktreectrl_path}/generic/tkTreeDisplay.c
      ${_tktreectrl_path}/generic/tkTreeDrag.c
      ${_tktreectrl_path}/generic/tkTreeElem.c
      ${_tktreectrl_path}/generic/tkTreeItem.c
      ${_tktreectrl_path}/generic/tkTreeMarquee.c
      ${_tktreectrl_path}/generic/tkTreeNotify.c
      ${_tktreectrl_path}/generic/tkTreeStyle.c
      ${_tktreectrl_path}/generic/tkTreeTheme.c
      ${_tktreectrl_path}/generic/tkTreeUtils.c
      )

    IF(WIN32)
      SET(_tktreectrl_win32_srcs
        ${_tktreectrl_path}/shellicon/shellicon.c
        )
      SET(_tktreectrl_srcs ${_tktreectrl_srcs} ${_tktreectrl_win32_srcs})
    ENDIF(WIN32)

    SET_SOURCE_FILES_PROPERTIES(${_tktreectrl_srcs}
      PROPERTIES LANGUAGE C)

    SET(_tktreectrl_srcs ${_tktreectrl_srcs} 
      ${_tktreectrl_path}/vtkKWTkTreeCtrlInit.cxx
      )

    SET(${sources} ${_tktreectrl_srcs})
    SET(${include_path} ${_tktreectrl_include_path})
    SET(${libs})
    
  ENDIF(${supported})

ENDMACRO(TkTreeCtrl_GET_LIBRARY)
