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
