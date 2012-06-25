#!/bin/sh
# \
exec tclsh "$0" ${1+"$@"}

# weather - Expect script to get the weather
# Don Libes, updates by Jeff Hobbs and David Gravereaux
# Version 1.50

# Usage:
#	tclsh weather.tcl ?-debug? [<2-char-state> | <3-char-region>]

# 2-char-state  is the 2-char state abbreviation (eg CA == California)
# 3-char-region is the National Weather Service designation for an area
#               (eg WBC == Washington DC (oh yeah, that's obvious))
# U.S. weather data available only (sorry)

# Notes from Larry Virden about the new host, rainmaker.wunderground.com:
# "[the] new site requires the machine doing the request be located in
# reverse dns lookup or it refuses to provide data."
# This appears to be a blind error condition on the part of rainmaker.

package require Tcl 8.4
package require Expect 5.40
package require cmdline

catch {console show}
log_user 0
#exp_internal 1

set timeout 10
set env(TERM) vt100	;# actual value doesn't matter, just has to be set
set host rainmaker.wunderground.com
set port 3000

set options {
    {debug	{Display spawned children rather than normally hiding them}}
}
set usage "\[options\] <3-digit-station-code> (to retrieve weather data)
or
[file rootname [file tail [info name]]] \[options\] <2-digit-state> (to list all stations in a state)
options are:"

if {[catch {
    array set Config [cmdline::getoptions argv $options $usage]
} err]} {
    send_error $err
    return
}

# display spawned children
set ::exp::winnt_debug $Config(debug)

if {[llength $argv] != 1} {
    send_error "bad arg. use -? for usage"
    return
}
if {[string length $argv] == 2} {
    set cmd 0
    set state $argv
} elseif {[string length $argv] == 3} {
    set cmd 1
    set code $argv
} else {
    exp_send_error "bad arg. use -? for usage"
    return
}

proc timedout {} {
    error "Weather server timed out.  Try again later when weather server is not so busy.\n"
}

proc get_to_mainmenu {host port} {
    global spawn_id

    if {[catch {
	if {$::tcl_platform(platform) eq "windows"} {
	    exp_spawn telnet
	    # we do expect Microsoft's telnet
	    expect "Microsoft Telnet> "
	    # local echo should be on or things behave badly.
	    exp_send "set LOCAL_ECHO\r"
	    exp_send "open $host $port\r"
	} else {
	    exp_spawn telnet $host $port
	}
    } err]} {
	error "Can't spawn telnet: $err\n"
    }
    while {1} {
	expect timeout {
	    error "failed to contact weather server (timeout)\n"
	} "Press Return to continue*" {
	   # this prompt used sometimes, eg, upon opening connection
	   exp_send "\r"
	} "Press Return for menu*" {
	   # this prompt used sometimes, eg, upon opening connection
	   exp_send "\r"
	} "M to display main menu*" {
	    # sometimes ask this if there is a weather watch in effect
	    exp_send "M\r"
	} "Change scrolling to screen*Selection:" {
	    break
	} eof {
	    error "failed to telnet to weather server (eof)\n"
	}
    }
    exp_send "C\r"			    ;# ask to set scrolling
    expect timeout {timedout} "Selection:"
    exp_send "4\r"			    ;# set unlimited page size
    expect timeout {timedout} "Selection:"
    # ready for command!
    return
}

proc get_weather_forecast {code} {
    global spawn_id

    exp_send "1\r"			    ;# switch to forecast menu
    expect timeout {timedout} "Selection:"
    exp_send "1\r"			    ;# ready for city code
    expect timeout {timedout} "city code:"
    exp_send "$code\r"		    ;# send it
    expect $code		    ;# discard the echo

    while {1} {
	match_max 50000		    ;# enlarge the buffer
				    ;# (special reports can be long)
	expect timeout {
	    timedout
	} "Press Return to continue*:*" {
	    exp_send "\r"
	} "Press Return to display statement, M for menu:*" {
	    exp_send "\r"
	} -re "(.*)CITY FORECAST MENU(.*)Selection:" {
	    exp_send_user -- "[string trim $expect_out(1,string) "\n\t "]\n"
	    break
	}
    }
}

proc get_stations_by_state {state} {
    global spawn_id

    exp_send "1\r"			    ;# switch to forecast menu
    expect timeout {timedout} "Selection:"
    exp_send "3\r"			    ;# ready for state
    expect timeout {timedout} "state code:"
    exp_send "$state\r"		    ;# send it
    expect $state		    ;# discard the echo

    while {1} {
	expect timeout {
	    timedout
	} -re "(.*?)(\\s*)Press Return(.*)exit:" {
	    exp_send_user -- "[string trim $expect_out(1,string) \n]\n"
	    exp_send "\r"
	    expect "\n"
	} -re "(.*)CITY FORECAST MENU(.*)Selection:" {
	    exp_send_user -- "[string trim $expect_out(1,string) \n]\n"
	    break
	}
    }
}

if {[catch {
    get_to_mainmenu $host $port
    switch -- $cmd {
	0 {get_stations_by_state $state}
	1 {get_weather_forecast $code}
    }
} err]} {
    send_error -- $err
    if {[string length $errorCode]} {
	exp_send_error "errorCode: $errorCode\n"
    }
    return
}

exp_send "X\r"
if {$::tcl_platform(platform) eq "windows"} {
    expect "Press any key to continue..."
    after 100		    ;# seems to need this pause and some
    exp_send "\r\r\r\r\r"	    ;# extra carriage returns to wake it up.
    expect "Microsoft Telnet> "
    exp_send "quit\r"
}
expect
