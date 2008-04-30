macro(TkDnD_GET_LIBRARY supported sources include_path libs)

  set(${sources})
  set(${include_path})
  set(${libs})

  set(${supported} 1)
  if(APPLE)
    if(VTK_USE_CARBON OR VTK_USE_COCOA)
      set(${supported} 0)
    endif(VTK_USE_CARBON OR VTK_USE_COCOA)
  endif(APPLE)

  if(${supported})
    set(_tkdnd_path "${KWWidgets_SOURCE_DIR}/Utilities/tkdnd")

    set(_tkdnd_include_path
      ${_tkdnd_path}/generic
      )
    set(_tkdnd_srcs 
      ${_tkdnd_path}/generic/tkDND.cpp
      ${_tkdnd_path}/generic/tkDNDBind.cpp
      ${_tkdnd_path}/vtkKWTkDnDInit.cxx
      )

    if(WIN32)
      set(_tkdnd_include_path ${_tkdnd_include_path} 
        ${_tkdnd_path}/win
        )
      set(_tkdnd_win32_srcs
        ${_tkdnd_path}/win/OleDND.cpp
        ${_tkdnd_path}/win/tkOleDND.cpp
        ${_tkdnd_path}/win/tkOleDND_TEnumFormatEtc.cpp
        ${_tkdnd_path}/win/tkShape.cpp
        )
      set(_tkdnd_srcs ${_tkdnd_srcs} ${_tkdnd_win32_srcs})
      #SET_SOURCE_FILES_PROPERTIES(${_tkdnd_win32_srcs}
      #  PROPERTIES LANGUAGE CXX)
      if(NOT BORLAND)
        # Can't use ATL because it is not support by Visual Studio Express 2005
        # We should either detect if we are using Express or Full 2005. This
        # would however not solve the problem for NMake mode with 2005 settings,
        # unless we read the entire atlbase.h file and search for the evil
        # atlthunk.lib reference.
        # This seems to be problematic for Visual Studio 6 as well.
        #SET_SOURCE_FILES_PROPERTIES(${_tkdnd_srcs}
        #  COMPILE_FLAGS "-DDND_ENABLE_DROP_TARGET_HELPER") 
      endif(NOT BORLAND)
    else(WIN32)
      set(_tkdnd_include_path ${_tkdnd_include_path}
        ${_tkdnd_path}/unix
        ${_tkdnd_path}/unix/Motif/lib
        )
      set(_tkdnd_unix_srcs
        ${_tkdnd_path}/unix/Motif.c
        ${_tkdnd_path}/unix/tkXDND.c
        ${_tkdnd_path}/unix/XDND.c
        ${_tkdnd_path}/unix/Motif/lib/Dnd.c
        ${_tkdnd_path}/unix/Motif/lib/Target.c
        )
      set_source_files_properties(${_tkdnd_srcs}
        COMPILE_FLAGS "-DTKDND_ENABLE_MOTIF_DROPS")
      set_source_files_properties(${_tkdnd_unix_srcs}
        COMPILE_FLAGS "-DTKDND_ENABLE_MOTIF_DROPS -DUSE_TK_STUBS")
      set(_tkdnd_srcs ${_tkdnd_srcs} ${_tkdnd_unix_srcs})
    endif(WIN32)

    # It does not seem possible to easily mix-match C/C++ within that library
    set_source_files_properties(${_tkdnd_srcs}
      PROPERTIES LANGUAGE CXX)

    set(${sources} ${_tkdnd_srcs})
    set(${include_path} ${_tkdnd_include_path})
    set(${libs})

  endif(${supported})

endmacro(TkDnD_GET_LIBRARY)
