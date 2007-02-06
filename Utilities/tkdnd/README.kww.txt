The changes to TkDnD for KWWidgets are:

- in generic/TkDND.h
replace:
  extern "C" HWND Tk_GetHWND(Window win);
with
  EXTERN HWND Tk_GetHWND(Window win);

- in generic/TkDND.h, generic/TkDNDBind.c, win/OleDND.cpp, win/OleDND.h, win/tkOleDND.cpp, win/tkShape.cpp

add the following two lines *before* any other #include:
#include "vtkTcl.h"
#include "vtkTk.h"

- in generic/tkDND.c
move:
  #ifdef __cplusplus
  }
  #endif
down after all the other declarations, i.e. after 
  extern int TkDND_GetCurrentScript(Tcl_Interp *interp, Tk_Window topwin,
                Tcl_HashTable *table, char *windowPath, char *typeStr,
                unsigned long eventType, unsigned long eventMask);
