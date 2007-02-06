The changes to TkDnD for KWWidgets are:

- library/tkdnd.tcl
comment:
        source $dir/library/tkdnd_unix.tcl
        [...]
        load $dir/libtkdnd20.so TkDND
        [...]
        source $dir/library/tkdnd_windows.tcl
        [...]
        load $dir/libtkdnd20.dll TkDND
        [...]
    source $dir/library/tkdnd_compat.tcl

- in unix/TkDND_XDND.c and win/OleDND.h

add the following two lines *before* any other #include:
#include "vtkTcl.h"
#include "vtkTk.h"

- in win/TkDND_OleDND.cpp and unix/TkDND_XDND.c
replace TKDND_PACKAGE by "tkdnd" and TKDND_VERSION by "2.0"

- in library/tkdnd_unix.tcl and library/tkdnd_win.tcl
put everything in the tkdnd namespace, i.e. put on top of each file:
  namespace eval tkdnd {
and at the end of each file:
  };# namespace tkdnd


