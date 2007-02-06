The changes to TkTreeCtrlk for KWWidgets are:

- in generic/tkTreeCtrl.c
replace:
    tcl_findLibrary treectrl " PACKAGE_PATCHLEVEL " " PACKAGE_PATCHLEVEL " treectrl.tcl TREECTRL_LIBRARY treectrl_library\n\
with:
    tcl_findLibrary treectrl 2.2.1 2.2.1 treectrl.tcl TREECTRL_LIBRARY treectrl_library\n\

and replace:
    if (Tcl_PkgProvide(interp, PACKAGE_NAME, PACKAGE_PATCHLEVEL) != TCL_OK) {
with:
    if (Tcl_PkgProvide(interp, "treectrl", "2.2.1") != TCL_OK) {

- in shellicon/shellicon.c
replace:
    if (Tcl_PkgRequire(interp, "treectrl", PACKAGE_PATCHLEVEL, TRUE) == NULL)
with:
    if (Tcl_PkgRequire(interp, "treectrl", "2.2.1", TRUE) == NULL)

replace:
    if (Tcl_PkgProvide(interp, PACKAGE_NAME, PACKAGE_PATCHLEVEL) != TCL_OK) {
with:
    if (Tcl_PkgProvide(interp, "treectrl", "2.2.1") != TCL_OK) {

replace:
TreeCtrlStubs *stubs;
#define TreeCtrl_RegisterElementType(i,t) \
with:
TreeCtrlStubs *stubs;
extern int BooleanCO_Init(Tk_OptionSpec *, CONST char *);
extern int TreeCtrl_RegisterElementType(Tcl_Interp *, ElementType *);
#if 0
#define TreeCtrl_RegisterElementType(i,t) \

and:
#define StringTableCO_Init(ot,on,ta) \
	stubs->StringTableCO_Init(ot,on,ta)
with:
#define StringTableCO_Init(ot,on,ta) \
	stubs->StringTableCO_Init(ot,on,ta)
#endif

The aim here is to bypass the stubs by putting all #define foo \ stubs->foo inside a #if 0 ... #endif

- in library/treectrl.tcl

comment:
source [file join [file dirname [info script]] filelist-bindings.tcl]

- in tkTreeCtrl.h and qebind.c

add the following two lines *before* any other #include:
#include "vtkTcl.h"
#include "vtkTk.h"
