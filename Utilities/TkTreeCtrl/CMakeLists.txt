PROJECT(TkTreeCtrl)

SET(TkTreeCtrl_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/generic/qebind.c
  ${CMAKE_CURRENT_SOURCE_DIR}/generic/tkTreeColumn.c
  ${CMAKE_CURRENT_SOURCE_DIR}/generic/tkTreeCtrl.c
  ${CMAKE_CURRENT_SOURCE_DIR}/generic/tkTreeDisplay.c
  ${CMAKE_CURRENT_SOURCE_DIR}/generic/tkTreeDrag.c
  ${CMAKE_CURRENT_SOURCE_DIR}/generic/tkTreeElem.c
  ${CMAKE_CURRENT_SOURCE_DIR}/generic/tkTreeItem.c
  ${CMAKE_CURRENT_SOURCE_DIR}/generic/tkTreeMarquee.c
  ${CMAKE_CURRENT_SOURCE_DIR}/generic/tkTreeNotify.c
  ${CMAKE_CURRENT_SOURCE_DIR}/generic/tkTreeStyle.c
  ${CMAKE_CURRENT_SOURCE_DIR}/generic/tkTreeTheme.c
  ${CMAKE_CURRENT_SOURCE_DIR}/generic/tkTreeUtils.c
)

IF(WIN32)
  SET(TkTreeCtrl_SRCS ${TkTreeCtrl_SRCS}
    ${CMAKE_CURRENT_SOURCE_DIR}/shellicon/shellicon.c)
ENDIF(WIN32)

INCLUDE_DIRECTORIES(
  ${VTK_TK_INTERNAL_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/generic
  )

ADD_LIBRARY(kwtreectrl STATIC ${TkTreeCtrl_SRCS})
TARGET_LINK_LIBRARIES(kwtreectrl ${TCL_LIBRARY} ${TK_LIBRARY})
