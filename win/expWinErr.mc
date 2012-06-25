;/* ----------------------------------------------------------------------------
; * expWinErr.mc --
; *
; *	This file contains the message catalog for use with Win32 error
; *	reporting through ReportEvent() and FormatMessage().
; *
; * ----------------------------------------------------------------------------
; *
; * Written by: Don Libes, libes@cme.nist.gov, NIST, 12/3/90
; * 
; * Design and implementation of this program was paid for by U.S. tax
; * dollars.  Therefore it is public domain.  However, the author and NIST
; * would appreciate credit if this program or parts of it are used.
; * 
; * Copyright (c) 2001-2002 Telindustrie, LLC
; * Copyright (c) 2003 ActiveState Corporation
; *	Work by David Gravereaux <davygrvy@pobox.com> for any Win32 OS.
; *	Based on work by Gordon Chaffee <chaffee@bmrc.berkeley.edu>
; *
; * ----------------------------------------------------------------------------
; * URLs:    http://expect.nist.gov/
; *	    http://expect.sf.net/
; *	    http://bmrc.berkeley.edu/people/chaffee/expectnt.html
; * ----------------------------------------------------------------------------
; * RCS: @(#) $Id: slavedrvmc.mc,v 1.1.2.4 2002/06/25 22:50:07 davygrvy Exp $
; * ----------------------------------------------------------------------------
; */

MessageIdTypedef = DWORD
OutputBase = 16

SeverityNames = (
    Success=0x0:STATUS_SEVERITY_SUCCESS
    Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
    Warning=0x2:STATUS_SEVERITY_WARNING
    Fatal=0x3:STATUS_SEVERITY_FATAL
)

FacilityNames = (
    Catagories=0x0
    System=0x0:FACILITY_SYSTEM
    Stubs=0x1:FACILITY_STUBS
    Io=0x2:FACILITY_IO
    Mailbox=0x3:FACILITY_MAILBOX
    DbgTrap=0x6:FACILITY_DBGTRAP
)

LanguageNames=(English=0x409:MSG00409)


MessageId=0x1
Severity=Warning
Facility=Io
SymbolicName=ERROR_EXP_WIN32_CANT_SPAWN_BATCH
Language=English
'%1' is a batch file.  Use 'cmd.exe /c ...' as the spawn command instead.
.

MessageId=0x2
Severity=Warning
Facility=Io
SymbolicName=ERROR_EXP_WIN32_CANT_SPAWN_DOS16
Language=English
'%1' is a DOS-16 application.  DOS-16 is not supported due to Win32 limitations in how the VDM subsystem exposes itself.
.


MessageId=0x3
Severity=Warning
Facility=Io
SymbolicName=ERROR_EXP_WIN32_CANT_SPAWN_NONCUI
Language=English
'%1' is not a Win32 console application.
.

MessageId=0x1
Severity=Informational
Facility=DbgTrap
SymbolicName=STATUS_EXP_WIN32_LOADED_DLL
Language=English
'%1' has loaded
.

MessageId=0x2
Severity=Informational
Facility=DbgTrap
SymbolicName=STATUS_EXP_WIN32_OUTPUT_DBGSTR
Language=English
OutputDebugString: %1
.

MessageId=0x1
Severity=Fatal
Facility=DbgTrap
SymbolicName=ERROR_EXP_WIN32_CANT_IPC
Language=English
IPC connection to injector.dll could not be made: %1
.

MessageId=0x2
Severity=Fatal
Facility=DbgTrap
SymbolicName=ERROR_EXP_WIN32_NO_BACKTRACE
Language=English
Unable to get backtrace: %1
.

MessageId=0x3
Severity=Fatal
Facility=DbgTrap
SymbolicName=ERROR_EXP_WIN32_BACKTRACE
Language=English
%1
.

MessageId=0x4
Severity=Warning
Facility=DbgTrap
SymbolicName=ERROR_EXP_WIN32_FATAL_RIP
Language=English
A fatal RIP error was caught: %1
.

MessageId=0x5
Severity=Warning
Facility=DbgTrap
SymbolicName=ERROR_EXP_WIN32_NONFATAL_RIP
Language=English
A non-fatal RIP error was caught: %1
.

MessageId=0x6
Severity=Fatal
Facility=DbgTrap
SymbolicName=ERROR_EXP_WIN32_CANT_MAKE_SUB_MEM
Language=English
Can't create subprocess memory: %1
.

MessageId=0x7
Severity=Fatal
Facility=DbgTrap
SymbolicName=ERROR_EXP_WIN32_CANT_READ_SUB_MEM
Language=English
Can't read subprocess memory: %1
.
