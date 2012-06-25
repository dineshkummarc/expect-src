/* ----------------------------------------------------------------------------
 * expWinErr.mc --
 *
 *	This file contains the message catalog for use with Win32 error
 *	reporting through ReportEvent() and FormatMessage().
 *
 * ----------------------------------------------------------------------------
 *
 * Written by: Don Libes, libes@cme.nist.gov, NIST, 12/3/90
 * 
 * Design and implementation of this program was paid for by U.S. tax
 * dollars.  Therefore it is public domain.  However, the author and NIST
 * would appreciate credit if this program or parts of it are used.
 * 
 * Copyright (c) 2001-2002 Telindustrie, LLC
 * Copyright (c) 2003 ActiveState Corporation
 *	Work by David Gravereaux <davygrvy@pobox.com> for any Win32 OS.
 *	Based on work by Gordon Chaffee <chaffee@bmrc.berkeley.edu>
 *
 * ----------------------------------------------------------------------------
 * URLs:    http://expect.nist.gov/
 *	    http://expect.sf.net/
 *	    http://bmrc.berkeley.edu/people/chaffee/expectnt.html
 * ----------------------------------------------------------------------------
 * RCS: @(#) $Id: slavedrvmc.mc,v 1.1.2.4 2002/06/25 22:50:07 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */
//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//
#define FACILITY_SYSTEM                  0x0
#define FACILITY_STUBS                   0x1
#define FACILITY_MAILBOX                 0x3
#define FACILITY_IO                      0x2
#define FACILITY_DBGTRAP                 0x6


//
// Define the severity codes
//
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_FATAL            0x3


//
// MessageId: ERROR_EXP_WIN32_CANT_SPAWN_BATCH
//
// MessageText:
//
//  '%1' is a batch file.  Use 'cmd.exe /c ...' as the spawn command instead.
//
#define ERROR_EXP_WIN32_CANT_SPAWN_BATCH ((DWORD)0xA0020001L)

//
// MessageId: ERROR_EXP_WIN32_CANT_SPAWN_DOS16
//
// MessageText:
//
//  '%1' is a DOS-16 application.  DOS-16 is not supported due to Win32 limitations in how the VDM subsystem exposes itself.
//
#define ERROR_EXP_WIN32_CANT_SPAWN_DOS16 ((DWORD)0xA0020002L)

//
// MessageId: ERROR_EXP_WIN32_CANT_SPAWN_NONCUI
//
// MessageText:
//
//  '%1' is not a Win32 console application.
//
#define ERROR_EXP_WIN32_CANT_SPAWN_NONCUI ((DWORD)0xA0020003L)

//
// MessageId: STATUS_EXP_WIN32_LOADED_DLL
//
// MessageText:
//
//  '%1' has loaded
//
#define STATUS_EXP_WIN32_LOADED_DLL      ((DWORD)0x60060001L)

//
// MessageId: STATUS_EXP_WIN32_OUTPUT_DBGSTR
//
// MessageText:
//
//  OutputDebugString: %1
//
#define STATUS_EXP_WIN32_OUTPUT_DBGSTR   ((DWORD)0x60060002L)

//
// MessageId: ERROR_EXP_WIN32_CANT_IPC
//
// MessageText:
//
//  IPC connection to injector.dll could not be made: %1
//
#define ERROR_EXP_WIN32_CANT_IPC         ((DWORD)0xE0060001L)

//
// MessageId: ERROR_EXP_WIN32_NO_BACKTRACE
//
// MessageText:
//
//  Unable to get backtrace: %1
//
#define ERROR_EXP_WIN32_NO_BACKTRACE     ((DWORD)0xE0060002L)

//
// MessageId: ERROR_EXP_WIN32_BACKTRACE
//
// MessageText:
//
//  %1
//
#define ERROR_EXP_WIN32_BACKTRACE        ((DWORD)0xE0060003L)

//
// MessageId: ERROR_EXP_WIN32_FATAL_RIP
//
// MessageText:
//
//  A fatal RIP error was caught: %1
//
#define ERROR_EXP_WIN32_FATAL_RIP        ((DWORD)0xA0060004L)

//
// MessageId: ERROR_EXP_WIN32_NONFATAL_RIP
//
// MessageText:
//
//  A non-fatal RIP error was caught: %1
//
#define ERROR_EXP_WIN32_NONFATAL_RIP     ((DWORD)0xA0060005L)

//
// MessageId: ERROR_EXP_WIN32_CANT_MAKE_SUB_MEM
//
// MessageText:
//
//  Can't create subprocess memory: %1
//
#define ERROR_EXP_WIN32_CANT_MAKE_SUB_MEM ((DWORD)0xE0060006L)

//
// MessageId: ERROR_EXP_WIN32_CANT_READ_SUB_MEM
//
// MessageText:
//
//  Can't read subprocess memory: %1
//
#define ERROR_EXP_WIN32_CANT_READ_SUB_MEM ((DWORD)0xE0060007L)

