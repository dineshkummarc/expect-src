/* ----------------------------------------------------------------------------
 * expWinPort.h --
 *
 *	This header file handles porting issues that occur because of
 *	differences between Windows and Unix. 
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
 * Copyright (c) 2003-2004 ActiveState Corporation
 *	Work by David Gravereaux <davygrvy@pobox.com> for any Win32 OS.
 *	Based on work by Gordon Chaffee <chaffee@bmrc.berkeley.edu>
 *
 * ----------------------------------------------------------------------------
 * URLs:    http://expect.nist.gov/
 *	    http://expect.sf.net/
 *	    http://bmrc.berkeley.edu/people/chaffee/expectnt.html
 * ----------------------------------------------------------------------------
 * RCS: @(#) $Id: expWinPort.h,v 1.1.2.1.2.6 2003/08/26 20:46:52 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#ifndef _EXPWINPORT
#define _EXPWINPORT

#ifndef _EXPINT
#   include "expInt.h"
#endif

#undef TCL_STORAGE_CLASS
#ifdef BUILD_exp
#   define TCL_STORAGE_CLASS DLLEXPORT
#else
#   ifdef USE_EXP_STUBS
#	define TCL_STORAGE_CLASS
#   else
#	define TCL_STORAGE_CLASS DLLIMPORT
#   endif
#endif

/* load the error codes. */
#include "expWinErr.h"

#define HAVE_SV_TIMEZONE    1

typedef struct {
    int useWide;
    HANDLE (WINAPI *createFileProc)(const TCHAR *, DWORD, DWORD, 
	    LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
    BOOL (WINAPI *createProcessProc)(const TCHAR *, TCHAR *, 
	    LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, 
	    LPVOID, const TCHAR *, LPSTARTUPINFO, LPPROCESS_INFORMATION);
    DWORD (WINAPI *getFileAttributesProc)(const TCHAR *);
    DWORD (WINAPI *getShortPathNameProc)(const TCHAR *, TCHAR *, DWORD); 
    DWORD (WINAPI *searchPathProc)(const TCHAR *, const TCHAR *, 
	    const TCHAR *, DWORD, TCHAR *, TCHAR **);
} ExpWinProcs;

TCL_EXTERNC ExpWinProcs *expWinProcs;
TCL_EXTERNC HMODULE expModule;

TCL_EXTERN(BOOL) ExpWinNTDebug(void);
TCL_EXTERN(BOOL) ExpWinNTStripVT100(void);

#define EXP_BAD_FILE	    ((exp_file) ERROR_INVALID_HANDLE)

/*
 * The following defines identify the various types of applications that 
 * run under windows and are returned by Exp_WinApplicationType().
 */

#define EXP_WINAPPL_NONE	0
#define EXP_WINAPPL_BATCH	1
#define EXP_WINAPPL_DOS16	2
#define EXP_WINAPPL_OS2		3
#define EXP_WINAPPL_OS2DRV	4
#define EXP_WINAPPL_WIN16	5
#define EXP_WINAPPL_WIN16DRV	6
#define EXP_WINAPPL_WIN32CUI	7
#define EXP_WINAPPL_WIN32GUI	8
#define EXP_WINAPPL_WIN32DLL	9
#define EXP_WINAPPL_WIN32DRV	10
#define EXP_WINAPPL_WIN64CUI	11
#define EXP_WINAPPL_WIN64GUI	12
#define EXP_WINAPPL_WIN64DLL	13
#define EXP_WINAPPL_WIN64DRV	14

#include "expIntPlatDecls.h"

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLIMPORT

#endif /* _EXPWINPORT */
