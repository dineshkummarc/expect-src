# all.tcl --
#
# This file contains a top-level script to run all of the Tcl
# tests.  Execute it by invoking "source all" when running tclTest
# in this directory.

package require tcltest

tcltest::testsDirectory [file dir [info script]]
tcltest::runAllTests

return
