# tearoff.tcl --
#
# This file contains procedures that implement tear-off menus.
#
# RCS: @(#) $Id: tearoff.tcl,v 1.3 2003-12-11 21:39:09 barre Exp $
#
# Copyright (c) 1994 The Regents of the University of California.
# Copyright (c) 1994-1997 Sun Microsystems, Inc.
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

# ::tk::TearoffMenu --
# Given the name of a menu, this procedure creates a torn-off menu
# that is identical to the given menu (including nested submenus).
# The new torn-off menu exists as a toplevel window managed by the
# window manager.  The return value is the name of the new menu.
# The window is created at the point specified by x and y
#
# Arguments:
# w -			The menu to be torn-off (duplicated).
# x -			x coordinate where window is created
# y -			y coordinate where window is created

proc ::tk::TearOffMenu {w {x 0} {y 0}} {
    # Find a unique name to use for the torn-off menu.  Find the first
    # ancestor of w that is a toplevel but not a menu, and use this as
    # the parent of the new menu.  This guarantees that the torn off
    # menu will be on the same screen as the original menu.  By making
    # it a child of the ancestor, rather than a child of the menu, it
    # can continue to live even if the menu is deleted;  it will go
    # away when the toplevel goes away.

    if {$x == 0} {
    	set x [winfo rootx $w]
    }
    if {$y == 0} {
    	set y [winfo rooty $w]
    }

    set parent [winfo parent $w]
    while {[string compare [winfo toplevel $parent] $parent] \
	    || [string equal [winfo class $parent] "Menu"]} {
	set parent [winfo parent $parent]
    }
    if {[string equal $parent "."]} {
	set parent ""
    }
    for {set i 1} 1 {incr i} {
	set menu $parent.tearoff$i
	if {![winfo exists $menu]} {
	    break
	}
    }

    $w clone $menu tearoff

    # Pick a title for the new menu by looking at the parent of the
    # original: if the parent is a menu, then use the text of the active
    # entry.  If it's a menubutton then use its text.

    set parent [winfo parent $w]
    if {[string compare [$menu cget -title] ""]} {
    	wm title $menu [$menu cget -title]
    } else {
    	switch [winfo class $parent] {
	    Menubutton {
	    	wm title $menu [$parent cget -text]
	    }
	    Menu {
	    	wm title $menu [$parent entrycget active -label]
	    }
	}
    }

    $menu post $x $y

    if {[winfo exists $menu] == 0} {
	return ""
    }

    # Set tk::Priv(focus) on entry:  otherwise the focus will get lost
    # after keyboard invocation of a sub-menu (it will stay on the
    # submenu).

    bind $menu <Enter> {
	set tk::Priv(focus) %W
    }

    # If there is a -tearoffcommand option for the menu, invoke it
    # now.

    set cmd [$w cget -tearoffcommand]
    if {[string compare $cmd ""]} {
	uplevel #0 $cmd [list $w $menu]
    }
    return $menu
}

# ::tk::MenuDup --
# Given a menu (hierarchy), create a duplicate menu (hierarchy)
# in a given window.
#
# Arguments:
# src -			Source window.  Must be a menu.  It and its
#			menu descendants will be duplicated at dst.
# dst -			Name to use for topmost menu in duplicate
#			hierarchy.

proc ::tk::MenuDup {src dst type} {
    set cmd [list menu $dst -type $type]
    foreach option [$src configure] {
	if {[llength $option] == 2} {
	    continue
	}
	if {[string equal [lindex $option 0] "-type"]} {
	    continue
	}
	lappend cmd [lindex $option 0] [lindex $option 4]
    }
    eval $cmd
    set last [$src index last]
    if {[string equal $last "none"]} {
	return
    }
    for {set i [$src cget -tearoff]} {$i <= $last} {incr i} {
	set cmd [list $dst add [$src type $i]]
	foreach option [$src entryconfigure $i]  {
	    lappend cmd [lindex $option 0] [lindex $option 4]
	}
	eval $cmd
    }

    # Duplicate the binding tags and bindings from the source menu.

    set tags [bindtags $src]
    set srcLen [string length $src]
 
    # Copy tags to x, replacing each substring of src with dst.

    while {[set index [string first $src $tags]] != -1} {
	append x [string range $tags 0 [expr {$index - 1}]]$dst
	set tags [string range $tags [expr {$index + $srcLen}] end]
    }
    append x $tags

    bindtags $dst $x

    foreach event [bind $src] {
	unset x
	set script [bind $src $event]
	set eventLen [string length $event]

	# Copy script to x, replacing each substring of event with dst.

	while {[set index [string first $event $script]] != -1} {
	    append x [string range $script 0 [expr {$index - 1}]]
	    append x $dst
	    set script [string range $script [expr {$index + $eventLen}] end]
	}
	append x $script

	bind $dst $event $x
    }
}
