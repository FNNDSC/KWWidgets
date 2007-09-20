MACRO(TkDnD_GET_LIBRARY supported sources include_path libs)

  SET(${sources})
  SET(${include_path})
  SET(${libs})

  SET(${supported} 1)
  IF(APPLE)
    IF(VTK_USE_CARBON OR VTK_USE_COCOA)
      SET(${supported} 0)
    ENDIF(VTK_USE_CARBON OR VTK_USE_COCOA)
  ENDIF(APPLE)

  IF(${supported})
    SET(_tkdnd_path "${KWWidgets_SOURCE_DIR}/Utilities/tkdnd")

    SET(_tkdnd_include_path
      ${_tkdnd_path}/generic
      )
    SET(_tkdnd_srcs 
      ${_tkdnd_path}/generic/tkDND.cpp
      ${_tkdnd_path}/generic/tkDNDBind.cpp
      ${_tkdnd_path}/vtkKWTkDnDInit.cxx
      )

    IF(WIN32)
      SET(_tkdnd_include_path ${_tkdnd_include_path} 
        ${_tkdnd_path}/win
        )
      SET(_tkdnd_win32_srcs
        ${_tkdnd_path}/win/OleDND.cpp
        ${_tkdnd_path}/win/tkOleDND.cpp
        ${_tkdnd_path}/win/tkOleDND_TEnumFormatEtc.cpp
        ${_tkdnd_path}/win/tkShape.cpp
        )
      SET(_tkdnd_srcs ${_tkdnd_srcs} ${_tkdnd_win32_srcs})
      #SET_SOURCE_FILES_PROPERTIES(${_tkdnd_win32_srcs}
      #  PROPERTIES LANGUAGE CXX)
      IF(NOT BORLAND)
        # Can't use ATL because it is not support by Visual Studio Express 2005
        # We should either detect if we are using Express or Full 2005. This
        # would however not solve the problem for NMake mode with 2005 settings,
        # unless we read the entire atlbase.h file and search for the evil
        # atlthunk.lib reference.
        # This seems to be problematic for Visual Studio 6 as well.
        #SET_SOURCE_FILES_PROPERTIES(${_tkdnd_srcs}
        #  COMPILE_FLAGS "-DDND_ENABLE_DROP_TARGET_HELPER") 
      ENDIF(NOT BORLAND)
    ELSE(WIN32)
      SET(_tkdnd_include_path ${_tkdnd_include_path}
        ${_tkdnd_path}/unix
        ${_tkdnd_path}/unix/Motif/lib
        )
      SET(_tkdnd_unix_srcs
        ${_tkdnd_path}/unix/Motif.c
        ${_tkdnd_path}/unix/tkXDND.c
        ${_tkdnd_path}/unix/XDND.c
        ${_tkdnd_path}/unix/Motif/lib/Dnd.c
        ${_tkdnd_path}/unix/Motif/lib/Target.c
        )
      SET_SOURCE_FILES_PROPERTIES(${_tkdnd_srcs}
        COMPILE_FLAGS "-DTKDND_ENABLE_MOTIF_DROPS")
      SET_SOURCE_FILES_PROPERTIES(${_tkdnd_unix_srcs}
        COMPILE_FLAGS "-DTKDND_ENABLE_MOTIF_DROPS -DUSE_TK_STUBS")
      SET(_tkdnd_srcs ${_tkdnd_srcs} ${_tkdnd_unix_srcs})
    ENDIF(WIN32)

    # It does not seem possible to easily mix-match C/C++ within that library
    SET_SOURCE_FILES_PROPERTIES(${_tkdnd_srcs}
      PROPERTIES LANGUAGE CXX)

    SET(${sources} ${_tkdnd_srcs})
    SET(${include_path} ${_tkdnd_include_path})
    SET(${libs} ${TCL_LIBRARY} ${TK_LIBRARY})

  ENDIF(${supported})

ENDMACRO(TkDnD_GET_LIBRARY)
