#!/bin/sh
# \
exec tclsh "$0" ${1+"$@"}

#
# run a program for a given amount of time,
# aborting after the number of seconds
#
# Usage:
#	tclsh timed-run.tcl 20 long_running_program ?program_args?
#
# Author: Jeff Hobbs, based off shorter example by Don Libes
#

# This is required to declare that we will use Expect
package require Expect

proc usage {} {
    puts stderr "usage: $::argv0 <timeInSecs> <program> ?<program args>?"
    exit 1
}

if {$argc < 2} { usage }

# timeout value is first to be passed in
set timeout [lindex $argv 0]
if {![string is integer -strict $timeout]} { usage }

# program and args are the rest
set cmd [lrange $argv 1 end]

# invoke the cmd with spawn
eval spawn $cmd

# just call expect and wait for it to timeout or eof to occur
expect

# at this point we fall through and exit, which cleans up for us
