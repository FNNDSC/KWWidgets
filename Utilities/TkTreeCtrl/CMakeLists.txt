PROJECT(TkTreeCtrl)

IF(0)
  INCLUDE(TkTreeCtrl.cmake)
  TkTreeCtrl_GET_LIBRARY(supported sources includes libs)
  
  INCLUDE_DIRECTORIES(${includes})
  ADD_LIBRARY(kwtreectrl STATIC ${sources})
  TARGET_LINK_LIBRARIES(kwtreectrl ${libs})
ENDIF(0)
