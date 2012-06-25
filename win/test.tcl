
proc read_spawn {spawn} {
    set got [read $spawn]
    puts -nonewline $got
    if {[eof $spawn]} {
	puts "closing $spawn..."
        set status [catch {close $spawn} result]
	if {$status == 0} {

	    # The command succeeded, and wrote nothing to stderr.

	} elseif {[string equal $::errorCode NONE]} {

	    # The command exited with a normal status, but wrote something
	    # to stderr, which is included in $result.
	    puts "stderr was: $result"

	} else {

	    switch -exact -- [lindex $::errorCode 0] {

		CHILDKILLED {

		    foreach { - pid sigName msg } $::errorCode break
		    # A child process, whose process ID was $pid,
		    # died on a signal named $sigName.  A human-
		    # readable message appears in $msg.
		    puts "pid $pid died on $sigName: $msg"
		    puts "details: $result"
		}

		CHILDSTATUS {

		    foreach { - pid code } $::errorCode break

		    # A child process, whose process ID was $pid,
		    # exited with a non-zero exit status, $code.
		    puts "pid $pid exited with code: $code"
		    puts "stderr: $result"
		}

		CHILDSUSP {

		    foreach { - pid sigName msg } $::errorCode break
		    # A child process, whose process ID was $pid,
		    # has been suspended because of a signal named
		    # $sigName.  A human-readable description of the
		    # signal appears in $msg
		    puts "$pid was suspended for $sigName: $msg"
		}

		POSIX - WIN32 - EXPECT {

		    foreach { group errName msg } $::errorCode break
		    # One of the kernel calls to launch the command
		    # failed.  The error code is in $errName, and a
		    # human-readable message is in $msg.
		    puts "$group, $errName, $msg"
		}
	    }
	}
    }
}
