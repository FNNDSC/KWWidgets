macro(TkTreeCtrl_GET_LIBRARY supported sources include_path libs)

  if(DEFINED ${supported})
    set(default_val ${supported})
  else(DEFINED ${supported})
    set(default_val 0)
  endif(DEFINED ${supported})

  # Visual Studio 6 does not support/provide uxtheme.h
  # Borland fails on tkTreeDrag.c 
  # (http://public.kitware.com/pipermail/kwwidgets/2007-August/000491.html)

  if(CMAKE_GENERATOR MATCHES "Visual Studio 6" OR BORLAND)
    set(default_val 0)
  endif(CMAKE_GENERATOR MATCHES "Visual Studio 6" OR BORLAND)

  option(${supported}
    "Enable TkTreeCtrl support in KWWidgets." ${default_val})
  mark_as_advanced(${supported})

  set(${sources})
  set(${include_path})
  set(${libs})
  
  if(${supported})
    set(_tktreectrl_path "${KWWidgets_SOURCE_DIR}/Utilities/TkTreeCtrl")
    
    set(_tktreectrl_include_path
      ${VTK_TK_INTERNAL_DIR}
      ${_tktreectrl_path}/generic
      )

    set(_tktreectrl_srcs 
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

    if(WIN32)
      set(_tktreectrl_win32_srcs
        ${_tktreectrl_path}/shellicon/shellicon.c
        )
      set(_tktreectrl_srcs ${_tktreectrl_srcs} ${_tktreectrl_win32_srcs})
    endif(WIN32)

    set_source_files_properties(${_tktreectrl_srcs}
      PROPERTIES LANGUAGE C)

    set(_tktreectrl_srcs ${_tktreectrl_srcs} 
      ${_tktreectrl_path}/vtkKWTkTreeCtrlInit.cxx
      )

    set(${sources} ${_tktreectrl_srcs})
    set(${include_path} ${_tktreectrl_include_path})
    set(${libs})
    
  endif(${supported})

endmacro(TkTreeCtrl_GET_LIBRARY)
