This is a subset of the BWidget ToolKit 1.7.0. See info and license below.

The changes are:

- the plus.xbm and minus.xbm files from the images/ subdir, used in tree.tcl
  have been converted to resources and are created at run time into the
  bwplus and bwimage Tk images. In tree.tcl, the following code has to be
  changed:
    Widget::configure $path [list -crossopenbitmap @$file]
    Widget::configure $path [list -crossclosebitmap @$file]
  into:
    Widget::configure $path [list -crossopenimage bwminus]
    Widget::configure $path [list -crosscloseimage bwplus]

- the library uses ::BWIDGET::LIBRARY, set in pkgIndex.tcl. Since
  we include the lib at compile time, this variable has to be set manually
  (to any value really) and plugged in one of the files evaluated first
  (say, utils.tcl):
  namespace eval ::BWIDGET {};
  set ::BWIDGET::LIBRARY {};

--------------------------------------------------------------------------

BWidget ToolKit 1.7.0				December 2003
Copyright (c) 1998-1999 UNIFIX.
Copyright (c) 2001-2002 ActiveState Corp. 

See the file LICENSE.txt for license info (uses Tcl's BSD-style license).

WHAT IS BWIDGET ?

The BWidget Toolkit is a high-level Widget Set for Tcl/Tk built using
native Tcl/Tk 8.x namespaces.

The BWidgets have a professional look&feel as in other well known
Toolkits (Tix or Incr Widgets), but the concept is radically different
because everything is pure Tcl/Tk.  No platform dependencies, and no
compiling required.  The code is 100% Pure Tcl/Tk.

The BWidget library was originally developed by UNIFIX Online, and
released under both the GNU Public License and the Tcl license.
BWidget is now maintained as a community project, hosted by
Sourceforge.  Scores of fixes and enhancements have been added by
community developers.  See the ChangeLog file for details.

CONTACTS

The BWidget toolkit is maintained on Sourceforge, at
http://www.sourceforge.net/projects/tcllib/