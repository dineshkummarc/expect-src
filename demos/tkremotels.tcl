#!/bin/sh
# -*- tcl -*- \
exec tclsh "$0"  ${1+"$@"}

#
# Tk remote ls
# simple remote directory lister with UI
#
# Copyright 2004-2005 ActiveState, a division of Sophos
# All rights reserved.
#

package require Tcl 8.4
package require Expect
package require Tk
package require BWidget
catch {
    package require style::as
    style::as::init
}

proc about {} {
    tk_messageBox -title "About Remote ls" -type ok \
	-message "Simple remote directory lister\
		\nCopyright 2004-2005 ActiveState, a division of Sophos"
}

proc init {} {
    global OPTS

    log_user 0

    set ::timeout 10
    set ::env(TERM) dumb

    array set OPTS {
	host    ""
	user    ""
	passwd  ""
	login   telnet
	prtype  -re
	prompt  "(%|#|>|\\$) $"
	ls      "/bin/ls"
	lsOpts  "-A1F"
	conn    "Disconnected"
	path    ""
	cd      "/"
	term    "dumb"
    }
    parseargs $::argc $::argv

    createui .
}

proc parseargs {argc argv} {
    global OPTS
    foreach {key val} $argv {
	switch -exact -- $key {
	    "-user"   { set OPTS(user)   $val }
	    "-host"   { set OPTS(host)   $val }
	    "-passwd" { set OPTS(passwd) $val }
	    "-login"  { set OPTS(login)  $val }
	    "-prompt" { set OPTS(prompt) $val }
	    "-ls"     { set OPTS(ls)     $val }
	    "-log"    { log_user $val }
	}
    }
}

proc timedout {{msg {none}}} {
    send_user "Timed out (reason: $msg)\n"
    if {[info exists ::expect_out]} {
	parray ::expect_out
    }
    exit 1
}

proc colorize {P w} {
    $w configure -bg [expr {($P eq "") ? "lightyellow" : "white"}]
    return 1
}

proc createui_prefs {f} {
    label $f.lhost -text "Host:" -anchor w
    entry $f.host -textvariable ::OPTS(host) \
	-validate key -vcmd [list colorize %P $f.host] -bg lightyellow
    label $f.luser -text "User:" -anchor w
    entry $f.user -textvariable ::OPTS(user) \
	-validate key -vcmd [list colorize %P $f.user] -bg lightyellow
    label $f.lpass -text "Password:" -anchor w
    entry $f.pass -textvariable ::OPTS(passwd) -show * \
	-validate key -vcmd [list colorize %P $f.pass] -bg lightyellow
    label $f.lls -text "Path to 'ls':" -anchor w
    entry $f.ls -textvariable ::OPTS(ls) \
	-validate key -vcmd [list colorize %P $f.ls] -bg lightyellow

    set lf [labelframe $f.lflogin -text " Login Type "]
    foreach type {telnet ssh rlogin slogin} {
	radiobutton $lf.l$type -text $type -value $type \
	    -variable ::OPTS(login)
	# disable this selection if we can't find the exe on disk
	if {[auto_execok $type] eq ""} { $lf.l$type configure -state disabled }
	pack $lf.l$type -side left
    }

    set lf [labelframe $f.lfprompt -text " Prompt "]
    foreach {type name} {-re Regexp -gl Glob -ex Exact} {
	radiobutton $lf.l$type -text $name -value $type \
	    -variable ::OPTS(prtype)
	pack $lf.l$type -side left
    }
    entry $lf.prompt -width 10 -textvariable ::OPTS(prompt) \
	-validate key -vcmd [list colorize %P $lf.prompt] -bg lightyellow
    pack $lf.prompt -side left -expand 1 -fill x -padx {0 4}

    set x 4
    set y 2
    grid $f.lhost    $f.host -sticky ew -padx $x -pady $y
    grid $f.luser    $f.user -sticky ew -padx $x -pady $y
    grid $f.lpass    $f.pass -sticky ew -padx $x -pady $y
    grid $f.lls      $f.ls -sticky ew -padx $x -pady $y
    grid $f.lflogin  - -sticky ew -padx $x -pady $y
    grid $f.lfprompt - -sticky ew -padx $x -pady $y
    grid columnconfigure $f 1 -weight 1
    grid rowconfigure    $f 6 -weight 1
}

proc createui_fs {f} {
    global W

    set pw [panedwindow $f.pw -orient horizontal -bd 0 -relief flat \
		-sashpad 0 -sashwidth 5 -sashrelief flat -showhandle 0 \
		-opaqueresize 1]

    set tsw [ScrolledWindow $pw.tsw -relief sunken -bd 1]
    set t [Tree $tsw.tree -relief flat -bd 0 -width 15 -redraw 1 \
	       -highlightthickness 0 -linestipple gray50 -bg white \
	       -opencmd  [list tree_node open $tsw.tree] \
	       -closecmd [list tree_node close $tsw.tree] \
	      ]
    $tsw setwidget $t
    set W(tree) $t

    set lsw [ScrolledWindow $pw.lsw -relief sunken -bd 1]
    set l [ListBox::create $lsw.lb -relief flat -bd 0 -redraw 1 -bg white \
		-highlightthickness 0 -width 20 -multicolumn true]
    $lsw setwidget $l
    set W(lb) $l

    bind $t <<TreeSelect>>    [list select_node $t $l]
    #bind $l <<ListboxSelect>> [list select_node $t $l]

    $pw add $tsw -sticky news -minsize 20
    $pw add $lsw -sticky news -minsize 20

    pack $pw -fill both -expand 1 -padx 2 -pady 2
}

proc createui {root} {
    global OPTS
    global W

    if {$root eq "."} { set w "" } else { set w $root }
    wm withdraw $root
    wm title $root "Remote ls"

    # Notebook
    set nb [NoteBook $w.nb]
    set W(nb) $nb

    set f [$nb insert end "tree" -text "Tree"]
    createui_fs $f

    set f [$nb insert end "prefs" -text "Preferences"]
    createui_prefs $f

    $nb compute_size
    $nb raise "prefs"

    # addressbar
    set f [frame $w.sbar]
    entry $f.cd -state readonly -textvariable ::OPTS(path) -bd 1
    button $f.conn -bd 1 -text "Connect" -width 11 -command { init_tree }
    set W(cd)   $f.cs
    set W(conn) $f.conn
    pack $f.cd   -side left -fill x -expand 1 -padx 4
    pack $f.conn -side left -fill x -padx 4

    pack $f  -side top -fill x -pady 4
    pack $nb -side top -fill both -expand 1 -padx 4 -pady 4

    # MENU
    option add *Menu.tearOff 0
    set menu [menu .menu]
    $menu add cascade -label "File" -menu $menu.file
    $menu add cascade -label "Edit" -menu $menu.edit
    $menu add cascade -label "Help" -menu $menu.help

    # File
    set m [menu $menu.file]
    $m add command -label "Exit" -underline 1 -command { exit }

    # Edit
    set m [menu $menu.edit]
    $m add command -label "Cut"   -underline 2 -state disabled
    $m add command -label "Copy"  -underline 0 -state disabled
    $m add command -label "Paste" -underline 0 -state disabled

    # Help
    set m [menu $menu.help]
    $m add command -label "About" -underline 0 -command about

    $root configure -menu $menu
    wm deiconify $root
}

proc init_tree {} {
    global OPTS W

    $W(nb) raise tree

    set root "/"
    set OPTS(cd) $root

    set tree $W(tree)
    $tree delete [$tree nodes root]

    set node $root
    # FIX: node name and -data are redundant
    $tree insert end root $node \
	-text $root -data $root \
	-drawcross allways \
	-image [Bitmap::get openfold]
    # Selecting the node will populate the toplevel
    select_node $tree $W(lb) $node
}

proc select_node {tree list {node {}}} {
    global OPTS

    if {$node ne ""} {
	$tree selection set $node
	update idle
    } else {
	set node [lindex [$tree selection get] 0]
    }

    # clear the listbox
    eval [linsert [$list items] 0 $list delete]

    # retrieve full dirpath stored in data
    set dir [$tree itemcget $node -data]
    if {[$tree itemcget $node -drawcross] eq "allways"} {
	# populate this dir
	set entries [getdir $dir]
	populate $tree $node $dir $entries
        set dir [$tree itemcget $node -data]
    }
    # the node is the full dir path
    set OPTS(cd) $node

    foreach subnode [$tree nodes $node] {
        $list insert end $subnode \
            -text  [$tree itemcget $subnode -text] \
            -image [Bitmap::get folder]
    }
    set num 0
    foreach f $dir {
        $list insert end f:$num \
            -text  $f \
            -image [Bitmap::get file]
        incr num
    }
}

proc tree_node {what tree node} {
    if {($what eq "open") && [$tree itemcget $node -drawcross] eq "allways"} {
	set dir [$tree itemcget $node -data]
	set entries [getdir $dir]
        populate $tree $node $dir $entries
        if {[llength [$tree nodes $node]]} {
            $tree itemconfigure $node -image [Bitmap::get openfold]
        } else {
            $tree itemconfigure $node -image [Bitmap::get folder]
        }
    } else {
        $tree itemconfigure $node -image \
	    [Bitmap::get [lindex {folder openfold} [string equal "open" $what]]]
    }
}

proc populate {tree node dir entries} {
    set files {}
    foreach f $entries {
        if {[string match "*/" $f]} {
	    # it's a directory, strip of /
	    set f [string range $f 0 end-1]
	    set full [file join $dir $f]
            $tree insert end $node $full \
                -text      $f \
                -image     [Bitmap::get folder] \
                -drawcross allways \
                -data      $full
        } else {
	    lappend files $f
        }
    }
    # store files in data, dirs are retrieved via 'nodes'
    $tree itemconfigure $node -drawcross auto -data $files
}

proc getdir {dir} {
    global OPTS W spawn_id

    set OPTS(cd) $dir
    set OPTS(path) "Spawning $OPTS(login) ..."
    switch -exact $OPTS(login) {
	"telnet" { set pid [spawn telnet $OPTS(host)] }
	"ssh"    -
	"slogin" -
	"rlogin" { set pid [spawn $OPTS(login) $OPTS(host) -l $OPTS(user)] }
    }
    set id $spawn_id

    if {$OPTS(login) eq "telnet"} {
	set OPTS(path) "Sending username $OPTS(user) ..."
	expect -i $id timeout {
	    timedout "in user login"
	} eof {
	    timedout "spawn failed with eof"
	    break
	} -re "(login|Username):.*" {
	    exp_send -i $id -- "$OPTS(user)\r"
	}
    }
    set logged_in 0
    while {1} {
	set OPTS(path) "Negotiating login ..."
	expect -i $id timeout {
	    timedout "in while loop"
	    break
	} eof {
	    timedout "spawn failed with eof"
	    break
	} "Are you sure you want to continue connecting (yes/no)? " {
	    # ssh request for a box we don't know yet
	    exp_send -i $id -- "yes\r"
	} "\[Pp\]assword*" {
	    #if {$OPTS(passwd) eq ""} { set OPTS(passwd) [getpasswd] }
	    exp_send -i $id -- "$OPTS(passwd)\r"
	} "TERM = (*) " {
	    exp_send -i $id -- "$OPTS(term)\r"
	} $OPTS(prtype) $OPTS(prompt) {
	    set logged_in 1
	    break
	}
    }

    set files {}
    if {$logged_in} {
	set OPTS(path) "$OPTS(user)@[file root $OPTS(host)]:$OPTS(cd)"
	exp_send -i $id -- "$OPTS(ls) $OPTS(lsOpts) $OPTS(cd)\r"
	expect -i $id timeout {timedout "on prompt"} -re $OPTS(prompt) {
	    set output $expect_out(buffer)
	}
	exp_send -i $id -- "exit\r"
	if {[info exists output]} {
	    # take out the extra CRs in the output
	    set files [split [string map [list "\r" ""] $output] \n]
	    # cut of the ls command and final prompt in output
	    set files [lrange $files 1 [expr {[llength $files]-2}]]
	}
    } else {
	set OPTS(path) "Login failed"
    }
    exp_close -i $id
    exp_wait -i $id
    return $files
}

if {![llength [winfo children .]]} {
    # Only call init if it looks like nothing was created yet
    #catch {console show}
    init
}
