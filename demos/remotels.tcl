#!/bin/sh
# \
exec tclsh "$0"  ${1+"$@"}

#
# remote ls
# simple remote directory lister
#
# Copyright 2004 ActiveState, a division of Sophos
# All rights reserved.
#
# This application logs into a remote host and returns the files
# and directories of the default cwd.
#

# This is required to declare that we will use Expect
package require Expect

# Turn off echo logging to the user by default
exp_log_user 0

# This special variable used by the ActiveState Expect for Windows
# port will enable actual viewing of the controlled console.
# Otherwise it remains hidden (default).
#set ::exp::winnt_debug 1

# This is the magic timeout variable used by Expect (time in seconds).
set timeout 10

# We want to operate very simply - force a dumb terminal mode
set env(TERM) dumb

# The default option settings
array set OPTS {
    host    ""
    user    ""
    passwd  ""
    login   telnet
    prompt  "(%|#|>|\\$) $"
    ls      "/bin/ls -A1"
}

proc usage {code} {
    # Output usage information and exit
    global OPTS
    puts [expr {$code ? "stderr" : "stdout"}] \
	"$::argv0 -user username -host hostname ?options?
	-passwd password (you will be prompted if none is given)
	-login  type     (telnet, ssh, rlogin, slogin {$OPTS(login)})
	-prompt prompt   (RE of prompt to expect on host {$OPTS(prompt)})
	-log    bool     (display expect log info {[exp_log_user]})
	-ls     lspath   (path to ls on host {$OPTS(ls)})
	-help            (print out this message)"
    exit $code
}

proc parseargs {argc argv} {
    # Parse the command line arguments
    global OPTS
    foreach {key val} $argv {
	switch -exact -- $key {
	    "-user"   { set OPTS(user)   $val }
	    "-host"   { set OPTS(host)   $val }
	    "-passwd" { set OPTS(passwd) $val }
	    "-login"  { set OPTS(login)  $val }
	    "-prompt" { set OPTS(prompt) $val }
	    "-ls"     { set OPTS(ls)     $val }
	    "-log"    { exp_log_user $val }
	    "-help"   { usage 0 }
	}
    }
}
parseargs $argc $argv

# Ensure that both a user and a host has been specified
if {$OPTS(host) == "" || $OPTS(user) == ""} {
    usage 1
}

# If the password wasn't specified, ask the user
if {$OPTS(passwd) == ""} {
    stty -echo; # don't show passwd input
    puts -nonewline "password required for $OPTS(host): "
    flush stdout
    gets stdin ::OPTS(passwd)
    stty echo
}

proc timedout {{msg {none}}} {
    # We call this procedure when a timeout occurs receiving data.
    # It outputs the reason and some debugging info so we might be
    # able to better pinpoint the reason for the timeout.
    send_user "Timed out (reason: $msg)\n"
    if {[info exists ::expect_out]} {
	parray ::expect_out
    }
    exit 1
}

# If the user isn't using telnet to login, we will need the extra -l opt.
switch -exact $OPTS(login) {
    "telnet" { set pid [spawn telnet $OPTS(host)] }
    "ssh"    -
    "slogin" -
    "rlogin" { set pid [spawn $OPTS(login) $OPTS(host) -l $OPTS(user)] }
}
# Capture the spawn_id implicitly set by the spawn command for later use.
set id $spawn_id

# If the user uses telnet to login, then there is an extra step to
# expect to pass the username in.
if {$OPTS(login) == "telnet"} {
    expect -i $id timeout {
	timedout "in user login"
    } eof {
	timedout "spawn failed with eof on login"
    } -re "(login|Username):.*" {
	exp_send -i $id -- "$OPTS(user)\r"
    }
}

set logged_in 0
while {!$logged_in} {
    # In this loop, we try to capture the various states towards a proper
    # login.  More Error handling (like bad password or username) could be
    # added.
    expect -i $id timeout {
	# we encountered a timeout - none of the other branches matched
	# within our established timeout timeframe.
	timedout "in while loop"
	break
    } eof {
	# for some reason we lost connection to the application.
	timedout "spawn failed with eof"
	break
    } "Are you sure you want to continue connecting (yes/no)? " {
	# ssh request for a box we don't know yet
	exp_send -i $id -- "yes\r"
    } "\[Pp\]assword*" {
	# we received a request to provide the password
	exp_send -i $id -- "$OPTS(passwd)\r"
    } "TERM = (*) " {
	# we received a request to set the terminal mode
	exp_send -i $id -- "$env(TERM)\r"
    } -re $OPTS(prompt) {
	# we received the user prompt.  At this point we can move on.
	set logged_in 1
    }
}

if {$logged_in} {
    # If we have successfully logged in, send the 'ls' request.
    exp_send -i $id -- "$OPTS(ls)\r"
    expect -i $id timeout {timedout "on prompt"} -re $OPTS(prompt) {
	# Capture the output of the result into the 'dir' variable.
	set dir $expect_out(buffer)
    }
    # Exit from the login session.
    exp_send -i $id -- "exit\r"
    if {[info exists dir]} {
	# Take out the extra CRs in the output (could use string map).
	regsub "\r" $dir "" dir
	set files [split $dir \n]
	# Cut of the ls command and final prompt in output
	set files [lrange $files 1 [expr {[llength $files]-2}]]
	puts "\n[llength $files] FILES AND DIRS:"
	puts $files
    }
}
# Close the spawned session.
exp_close -i $id
# To be clean, we should wait on the result, causing it to be reaped
# from the system processes.  If this hangs, then the exp_close was
# not able to close the spawned process and it may need to be killed.
exp_wait -i $id
