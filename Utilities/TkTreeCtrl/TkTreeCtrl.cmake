MACRO(TkTreeCtrl_GET_LIBRARY supported sources include_path libs)

  SET(${sources})
  SET(${include_path})
  SET(${libs})
  
  SET(${supported} 1)
  
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
    SET(${libs} ${TCL_LIBRARY} ${TK_LIBRARY})
    
  ENDIF(${supported})

ENDMACRO(TkTreeCtrl_GET_LIBRARY)
