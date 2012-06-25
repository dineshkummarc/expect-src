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
package require tile
package require ctext

package require style::as
style::as::init

proc about {} {
    tk_messageBox -title "About TkRemote" -type ok \
	-message "Simple remote script runner\
		\nCopyright 2004-2005 ActiveState, a division of Sophos"
}

proc init {} {
    global OPTS

    log_user 0

    set ::timeout 10
    set ::env(TERM) dumb
    set ::LASTDIR [pwd]

    array set OPTS {
	host    machete
	user    jeffh
	passwd  ""
	login   C:/temp/test/bin/telnet.exe
	prtype  -re
	prompt  "(%|#|>|\\$) $"
	ls      "/bin/ls"
	lsOpts  "-A1l"
	conn    "Disconnected"
	path    ""
	status  "Disconnected"
	cd      "/"
	term    "dumb"
	get     "cat --"
	get     "uuencoder --"
    }
    parseargs $::argc $::argv

    if {[string match uuencode* $OPTS(get)]} {
	package require uuencode
    }

    bind all <F12> { catch {console show} }
    bind all <F11> { catch {console hide} }

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
    tk_messageBox -title "Timed Out" -message "Timed out $msg"
    if {[info exists ::expect_out]} {
	parray ::expect_out
    }
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

proc select_script {w} {
    set types {
	{{All Files}	*}
	{{sh Files}	{.sh}}
    }
    set file [tk_getOpenFile -title "Select Script" -filetypes $types \
		  -parent [winfo toplevel $w]]
    if {![file exists $file]} { return }

    set fid  [open $file]
    set data [read $fid]
    close $fid
    $w delete 1.0 end
    $w insert 1.0 $data
}

proc createui_script {f} {
    global W

    set sw [ScrolledWindow $f.sw -relief sunken -bd 1]
    set t [ctext $sw.script -relief flat -bd 0 -width 5 -height 5 \
	       -highlightthickness 0 -wrap none]
    $sw setwidget $t

    set l   [label $f.msg -text "Script to run on remote machine:" -anchor w]
    set btn [button $f.open -text "Open ..." \
		 -command [list select_script $t]]

    set W(script) $t

    grid $l $btn -sticky ew -padx 2
    grid $sw -sticky news -padx 2 -pady 2 -columnspan 2

    grid rowconfigure $f 1 -weight 1
    grid columnconfig $f 0 -weight 1
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
	       -highlightthickness 0 -width 20 -multicolumn true \
	       -selectmode single]
    $lsw setwidget $l
    set W(lb) $l

    bind $t <<TreeSelect>>    [list select_node $t $l]
    bind $l <<ListboxSelect>> [list retrieve_ok $t $l]

    $pw add $tsw -sticky news -minsize 20
    $pw add $lsw -sticky news -minsize 20

    pack $pw -fill both -expand 1 -padx 2 -pady 2
}

proc retrieve_ok {t l {node {}}} {
    global W
    set state [expr {[llength [$l selection get]]? "normal" : "disabled"}]
    $W(fmenu) entryconfigure "Retrieve File" -state $state
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

    set f [$nb insert end "prefs" -text "Preferences"]
    createui_prefs $f

    set f [$nb insert end "script" -text "Script"]
    createui_script $f

    set f [$nb insert end "fs" -text "RFS" -state disabled]
    createui_fs $f

    $nb compute_size
    $nb raise "prefs"

    # addressbar
    set af [frame $w.abar]
    entry $af.cd -state readonly -textvariable ::OPTS(path) -bd 1
    button $af.conn -bd 1 -text "Connect" -width 11 -command { init_tree }
    set W(cd)   $af.cs
    set W(conn) $af.conn
    pack $af.cd   -side left -fill x -expand 1 -padx 4
    pack $af.conn -side left -fill x -padx 4

    # statusbar
    set sf [frame $w.sbar]
    frame $sf.sep -height 2 -relief ridge -bd 1
    label $sf.status -textvariable ::OPTS(status) -anchor w
    pack $sf.sep    -side top -fill x -expand 1
    pack $sf.status -side top -fill x -expand 1

    pack $af -side top -fill x -pady 4
    pack $nb -side top -fill both -expand 1 -padx 4 -pady 4
    pack $sf -side top -fill x -pady 2

    # MENU
    option add *Menu.tearOff 0
    set menu [menu .menu]
    $menu add cascade -label "File" -menu $menu.file
    $menu add cascade -label "Edit" -menu $menu.edit
    $menu add cascade -label "Help" -menu $menu.help

    # File
    set m [menu $menu.file]
    set W(fmenu) $m
    $m add command -label "Put File" -underline 0 -state disabled \
	-command { putfilecur $OPTS(cd) }
    $m add command -label "Retrieve File" -underline 0 -state disabled \
	-command {
	    retrievefilecur $OPTS(cd) \
		[$::W(lb) itemcget [lindex [$::W(lb) selection get] 0] -text]
	}
    $m add separator
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

    $W(nb) raise "fs"

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
    global OPTS W

    # disable selections while we work
    bind $tree <<TreeSelect>> {}

    if {$node ne ""} {
	$tree selection set $node
	update idle
    } else {
	set node [lindex [$tree selection get] 0]
    }

    # clear the listbox
    eval [linsert [$list items] 0 $list delete]
    $W(fmenu) entryconfigure "Retrieve File" -state disabled
    $W(fmenu) entryconfigure "Put File" -state normal

    # the node is the full dir path
    set OPTS(path) "$OPTS(user)@$OPTS(host):$node"
    set OPTS(cd) $node

    # retrieve full dirpath stored in data
    set dir [$tree itemcget $node -data]
    if {[$tree itemcget $node -drawcross] eq "allways"} {
	# populate this dir
	set entries [listfilescur $dir]
	populate $tree $node $dir $entries
        set dir [$tree itemcget $node -data]
    }

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

    # reenable selections
    bind $tree <<TreeSelect>> [list select_node $tree $list]
}

proc tree_node {what tree node} {
    if {($what eq "open") && [$tree itemcget $node -drawcross] eq "allways"} {
	set dir [$tree itemcget $node -data]
	set entries [listfilescur $dir]
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

proc status {{msg {}}} {
    if {[llength [info level 0]] == 2} {
	set ::OPTS(status) $msg
    }
    return $::OPTS(status)
}

proc connect {host user passwd} {
    global OPTS IDS spawn_id expect_out

    if {[info exists IDS($user@$host)]} {
	return $IDS($user@$host)
    }

    status "Spawning $OPTS(login) ..."
    switch -regexp $OPTS(login) {
	"telnet" { spawn $OPTS(login) $host }
	"ssh"    -
	"slogin" -
	"rlogin" { spawn $OPTS(login) $host -l $user }
    }
    set id $spawn_id; # something of the form 'exp#'

    set logged_in 0
    if {[string match *telnet* $OPTS(login)]} {
	status "Sending username $user ..."
	expect -i $id timeout {
	    timedout "waiting to provide user name for telnet"
	} eof {
	    timedout "with spawn receiving eof"
	} -re "(login|Username):.*" {
	    exp_send -i $id -- "$user\r"
	    after 500
	    set logged_in 1
	}
	if {!$logged_in} {
	    catch {exp_close -i $id}
	    catch {exp_wait -i $id}
	    return ""
	}
	set logged_in 0
    }
    status "Negotiating login ..."
    expect -i $id timeout {
	timedout "negotiating login"
    } eof {
	timedout "with spawn receiving eof"
    } "Are you sure you want to continue connecting (yes/no)? " {
	# ssh request for a box we don't know yet
	exp_send -i $id -- "yes\r"
	    after 500
	exp_continue
    } "\[Pp\]assword*" {
	#if {$passwd eq ""} { set passwd [getpasswd] }
	exp_send -i $id -- "$passwd\r"
	    after 500
	exp_continue
    } "TERM = (*) " {
	exp_send -i $id -- "$OPTS(term)\r"
	    after 500
	exp_continue
    } $OPTS(prtype) $OPTS(prompt) {
	set logged_in 1
    }
    #exp_send -i $id "stty onocr\r"
    #expect -i $id -re $OPTS(prompt)

    if {$logged_in} {
	expect_background -i $id eof [subst {
	    disconnect [list $host] [list $user]
	    tk_messageBox -title "Lost Connection" -type ok \
		-message "Lost connect to [list $user@$host]"
	}]
	expect -i $id -re ^.*$ ; # drain channel
	set IDS($user@$host) $id
	status "Connected to $user@$host"
	$::W(conn) configure -text "Disconnect" \
	    -command [list disconnect $host $user]
	return $id
    } else {
	catch {exp_close -i $id}
	catch {exp_wait -i $id}
	status "Failed to connect to $user@$host"
	return ""
    }
}

proc disconnect {host user} {
    global IDS W
    $W(conn) configure -text "Connect" -command { init_tree }
    $W(fmenu) entryconfigure "Retrieve File" -state disabled
    $W(fmenu) entryconfigure "Put File" -state disabled
    if {![info exists IDS($user@$host)]} {
	return
    }
    set id $IDS($user@$host)
    unset IDS($user@$host)
    catch {exp_send -i $id "exit\r"}
    catch {exp_close -i $id}
    exp_wait -i $id
}

proc listfiles {host user passwd dir} {
    global OPTS

    set id [connect $host $user $passwd]
    if {$id == ""} {
	status "Unable to connect to $user@$host"
	return
    }

    #exp_internal 1
    expect -i $id -timeout 1 -re ^.*$; # drain channel
    set cmd "echo BBEGIN; $OPTS(ls) $OPTS(lsOpts) $dir; echo EEND"
    exp_send -i $id -- "$cmd\r"
    set capture 0
    set files {}
    # mode links owner group size mtime(month day time|year) (file ?-> link?)
    set lsre {^(\S+)\s+(\d+)\s+(\S+)\s+(\S+)\s+(\d+)\s+(\S+\s+\S+\s+\S+)\s+(.*)$}
    set linkre {^(.*) -> (.*)$}
    exp_sleep 0.1
    expect -i $id timeout {
	timedout "waiting for ls output"
	status "waiting for ls output"
    } -re "^(\[^\n]*)\n" {
	# take out the extra CRs in the output
	set line [string trim $expect_out(1,string)]
	if {$line eq "BBEGIN"} {
	    set capture 1
	    exp_continue
	} elseif {$line eq "EEND"} {
	    set capture 0
	} elseif {$capture} {
	    if {[string match "total *" $line]} { exp_continue }
	    if {![regexp $lsre $line \
		      -> mode links owner group size mtime file]} {
		#puts "had to barf on '$line'"
		exp_continue
	    }
	    if {[string match l* $mode]} {
		regexp $linkre $file -> file link
		append file @
	    } else {
		set link ""
	    }
	    # d is a directory;
	    # D is a door;
	    # l is a symbolic link;
	    # b is a block special file;
	    # c is a character special file;
	    # p is a fifo (or "named pipe") special file;
	    # s is an AF_UNIX address family socket;
	    # - is an ordinary file;
	    if {[string match d* $mode]} {
		lappend files $file/
	    } else {
		lappend files $file
	    }
	    exp_continue
	} else {
	    exp_continue
	}
    } -re $OPTS(prompt) {
	# fall through
    } "" {
	exp_sleep 0.5
	exp_continue
    }
    #exp_internal 0

    return $files
}

proc listfilescur {dir} {
    global OPTS
    # uses current options
    return [listfiles $OPTS(host) $OPTS(user) $OPTS(passwd) $dir]
}

proc putfilecur {dir {file {}}} {
    global OPTS

    if {$file eq ""} {
	set file [tk_getOpenFile -title "Select File for $OPTS(cd)" \
		      -initialdir $::LASTDIR]
	if {$file eq ""} { return }
    }

    if {![file exists $file]} {
	return -code error "unable to locate file to put '$file'"
    }

    set fid [open $file]
    fconfigure $fid -translation binary
    set data [read $fid]
    close $fid

    # uses current options
    set id [connect $OPTS(host) $OPTS(user) $OPTS(passwd)]
    if {$id == ""} {
	status "Unable to connect to $user@$host"
	return 0
    }

    set ok 1
    # disabling echo
    exp_send -i $id "stty -echo\r"
    expect -i $id -re $OPTS(prompt)

    status "Sending $file"
    #exp_send "uuencode $file $outfile\r"
    exp_send -i $id "cat > '$OPTS(cd)/[file tail $file]'\r"
    # we need to have some timeout check for prompt in case
    # the user doesn't have file perms for the cat to file
    #expect -re $OPTS(prompt) ;# wait for prompt from cat
    exp_send -i $id -- "$data\r"
    exp_send -i $id "\004"		;# eof
    expect -i $id -re $OPTS(prompt)

    # restore echo and serendipitously reprompt
    exp_send -i $id "stty echo\r"
    expect -i $id -re $OPTS(prompt) ;# wait for prompt

    # we should re-list the files for the current dir
    if {$ok} {
	status "Sent $file"
	return [listfilescur $dir]
    } else {
	status "Problem sending $file"
    }

    return $ok
}

proc retrievefilecur {dir file} {
    global OPTS

    # uses current options
    set id [connect $OPTS(host) $OPTS(user) $OPTS(passwd)]
    if {$id == ""} {
	status "Unable to connect to $user@$host"
	return 0
    }

    set ok 1
    # disabling echo
    exp_send "stty -echo\r"
    expect -re $OPTS(prompt)
    # get the prompt and convert it to something really from the host
    set sawprompt [string trimleft \
		       [lindex [split $expect_out(buffer) \n] end]]

    set data {}
    set file "$OPTS(cd)/[file tail $file]"
    set err ""
    if {[string match cat* $OPTS(get)]} {
	# make sawprompt safe as an RE
	regsub -all {[\[()|\+\*\?\\]} $sawprompt {\\&} sawprompt
	regsub -all {\d+} $sawprompt {\\d+} sawprompt
	status "Retrieving $file (waiting for $sawprompt) "
	exp_send "$OPTS(get) '$file'\r"
	expect -re "^(.*)$sawprompt$" {
	    append data "$expect_out(1,string)"
	} -re "^(\[^\r]*)\r\n" {
	    append data "$expect_out(1,string)\n"
	    status "[status]."
	    exp_continue
	}
	#set data [uncatve $data]
    } elseif {[string match uuencode* $OPTS(get)]} {
	status "Retrieving $file "
	set cmd "$OPTS(get) '$file' '$file'"
	exp_send "$cmd\r"
	set capture 0
	# the first thing that should appear is the begin line
	expect -re {^(begin.*\n)} {
	    set capture 1
	    append data $expect_out(1,string)
	} -re {(?i)^.*: command not found.*} {
	    set err "Unable to retrieve file '$file':\
				\n[string trim $expect_out(buffer)]"
	}
	if {$capture} {
	    expect -re {^(end\n)} {
		# we are done
		append data $expect_out(1,string)
		set capture 0
	    } -re {^([^\n]*\n)} {
		if {$capture} {
		    # older uuencoders use " " where a pad char should be
		    append data [string map [list " " `] $expect_out(1,string)]
		}
		status "[status]."
		exp_continue
	    }
	} else {
	    set err "Unable to retrieve file '$file':\
				\nunrecognized output from '$cmd'"
	}
	expect -re $OPTS(prompt) ;# wait for prompt after uuencode
	catch {set data [lindex [::uuencode::uudecode -- $data] 0 2]}
    }
    if {$err eq ""} {
	status "Retrieved $file"

	set out [tk_getSaveFile -title "File to save as ..." \
		     -initialfile [file tail $file] \
		     -initialdir  $::LASTDIR]
	if {$out ne ""} {
	    set fid [open $out w]
	    fconfigure $fid -translation binary
	    puts -nonewline $fid $data
	    close $fid
	    set ::LASTDIR [file dirname $out]
	}
    } else {
	status "Unable to retrieve $file"
	tk_messageBox -title "Error Retrieving File" -type ok \
	    -message $err
	set ok 0
    }

    # restore echo and serendipitously reprompt
    exp_send "stty echo\r"
    expect -re $OPTS(prompt) ;# wait for prompt

    return $ok
}

proc uncatve {data} {
    # reverse process cat -ve output to binary data
    # -v    Non-printing characters (with the exception  of  tabs,
    #       new-lines  and  form-feeds) are printed visibly. ASCII
    #       control characters (octal 000 - 037)  are  printed  as
    #       ^n,  where  n  is the corresponding ASCII character in
    #       the range octal 100 - 137 (@, A, B, C, . . ., X, Y, Z,
    #       [,  \, ], ^, and _); the DEL character (octal 0177) is
    #       printed ^?. Other non-printable characters are printed
    #       as  M-x,  where  x is the ASCII character specified by
    #       the low-order seven bits.
    return $data
}

if {![llength [winfo children .]]} {
    init
}
