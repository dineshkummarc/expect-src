#!/bin/sh
# \
exec tclsh "$0"  ${1+"$@"}

#
# read a complete line from stdin
# aborting after the number of seconds (given as an argument, default 10s)
# Author: Jeff Hobbs, based off shorter example by Don Libes
#

# This is required to declare that we will use Expect
package require Expect

# Accept alternate timeout value to be passed in
set timeout 10 ; # wait 10 seconds for output by default
if {$argc && [string is integer -strict [lindex $argv 0]]} {
    set timeout [lindex $argv 0]
}

# don't echo user output
exp_log_user 0

# indicate that we are waiting for something
puts -nonewline "Please enter data: "
flush stdout

# print out what we received, or a meaningful error on timeout
expect timeout {
    puts "\nLine not received within $timeout seconds"
} -re "\n" {
    puts "Received: '[string trimright $expect_out(buffer) \n]'"
}
