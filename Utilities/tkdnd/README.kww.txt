The changes to TkDnD for KWWidgets are:

- in generic
  rename all .c to .cpp (they are C++ files)

- in generic/TkDND.h
replace:
  extern "C" HWND Tk_GetHWND(Window win);
with
  EXTERN HWND Tk_GetHWND(Window win);

- in all files that include either tcl.h or tk.h

add the following two lines *before* any other #include:
#include "vtkTcl.h"
#include "vtkTk.h"

- in unix/tkXDND.c
replace:
     result = TkDND_GetDataAccordingToType(infoPtr, 

by:
     result = (unsigned char *)TkDND_GetDataAccordingToType(infoPtr, 

replace:
  static TkDND_LocalErrorHandler(Display *display, XErrorEvent *error) {
by
  static int TkDND_LocalErrorHandler(Display *display, XErrorEvent *error) {

- in unix/XDND.c
replace:
    XChangeProperty(dnd->display, request->requestor, request->property,
      request->target, 8, PropModeReplace, dnd->data, dnd->index);
by:
    XChangeProperty(dnd->display, request->requestor, request->property,
      request->target, 8, PropModeReplace, (const unsigned char*)dnd->data, dnd->index);

replace:
  dnd->data = data;	
  dnd->index = read;

by:
  dnd->data = (char*)data;
  dnd->index = read;

- in unix/tkXDND.c
replace:
  DndClass *TkDND_Init(Tcl_Interp *interp, Tk_Window topwin) {
by:
  void *TkDND_Init(Tcl_Interp *interp, Tk_Window topwin) {

- in generic/tkDND.cpp and generic/tkDNDBind.cpp
replace all 
   Tcl_SetResult(interp, "...", ...);
with 
   Tcl_SetResult(interp, (char*)"...", ...);