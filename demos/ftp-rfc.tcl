#!/bin/sh
# \
exec tclsh "$0"  ${1+"$@"}

#
# retrieves an rfc (or the index) from uunet
#
# Usage:
#	tclsh ftp-rfc.tcl <rfc-number>
#	tclsh ftp-rfc.tcl -index
#

# This is required to declare that we will use Expect
package require Expect

# Accept alternate timeout value to be passed in
set rfc [lindex $argv 0]
if {![string is integer -strict $rfc] && $rfc ne "-index"} {
    puts stderr "usage: $argv0 <rfc-number> | -index"
    exit 1
}

# this is the expected filename format in uunet's ftp archive
set file "rfc$rfc.Z"

set timeout 60
spawn ftp ftp.uu.net
expect "User*:"
send "anonymous\r"
expect "Password:"
send "expect@nist.gov\r"
expect "ftp>"
send "binary\r"
expect "ftp>"
send "cd inet/rfc\r"
expect "550*ftp>" exit "250*ftp>"
send "get $file\r"
expect "550*ftp>" exit "200*226*ftp>"
close
wait

# get full pathname to file
set file [file normalize $file]

# try to find a valid .Z uncompressor
set compressed "no uncompressor found"
foreach {unzipapp opts} {uncompress {} gzip {-d}} {
    set app [auto_execok $unzipapp]
    if {$app != ""} {
	send_user "\nuncompressing file - wait...\n"
	if {[catch {eval exec $app $opts [list $file]} compressed] == 0} {
	    # no error occurred
	    set compressed ""
	}
	break
    }
}

if {$compressed ne ""} {
    send_user "\nunable to uncompress file ($compressed)\
		\nleaving compressed file: $file"
} else {
    send_user "\nuncompressed file at [file root $file]"
}
