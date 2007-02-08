PROJECT(TkDnD)

IF(0)
  INCLUDE(TkDnD.cmake)
  TkDnD_GET_LIBRARY(supported sources includes libs)
  
  INCLUDE_DIRECTORIES(${includes})
  ADD_LIBRARY(kwtkdnd STATIC ${sources})
  TARGET_LINK_LIBRARIES(kwtkdnd ${libs})
ENDIF(0)
