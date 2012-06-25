/* ----------------------------------------------------------------------------
 * expPlatIntDecls.h --
 *
 *	Declarations of platform specific internal Expect APIs.
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
 * RCS: @(#) $Id: expIntPlatDecls.h,v 1.1.4.6 2002/06/18 23:13:42 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#ifndef _EXPPLATINTDECLS
#define _EXPPLATINTDECLS

/* !BEGIN!: Do not edit below this line. */

/*
 * Exported function declarations:
 */

#ifdef __WIN32__
#ifndef ExpWinInit_TCL_DECLARED
#define ExpWinInit_TCL_DECLARED
/* 0 */
TCL_EXTERN(int)	ExpWinInit _ANSI_ARGS_((Tcl_Interp *interp));
#endif
#ifndef ExpWinErrId_TCL_DECLARED
#define ExpWinErrId_TCL_DECLARED
/* 1 */
TCL_EXTERN(CONST char *) ExpWinErrId _ANSI_ARGS_((DWORD errorCode));
#endif
#ifndef ExpWinErrMsg_TCL_DECLARED
#define ExpWinErrMsg_TCL_DECLARED
/* 2 */
TCL_EXTERN(CONST char *) ExpWinErrMsg _ANSI_ARGS_(TCL_VARARGS(DWORD,errorCode));
#endif
#ifndef ExpWinErrMsgVA_TCL_DECLARED
#define ExpWinErrMsgVA_TCL_DECLARED
/* 3 */
TCL_EXTERN(CONST char *) ExpWinErrMsgVA _ANSI_ARGS_((DWORD errorCode, 
				va_list argList));
#endif
#ifndef ExpWinError_TCL_DECLARED
#define ExpWinError_TCL_DECLARED
/* 4 */
TCL_EXTERN(CONST char *) ExpWinError _ANSI_ARGS_(TCL_VARARGS(Tcl_Interp *,interp));
#endif
#endif /* __WIN32__ */

typedef struct ExpIntPlatStubs {
    int magic;
    struct ExpIntPlatStubHooks *hooks;

#ifdef __WIN32__
    void (*expWinInit) _ANSI_ARGS_((void)); /* 0 */
    CONST char * (*expWinErrId) _ANSI_ARGS_((DWORD errorCode)); /* 1 */
    CONST char * (*expWinErrMsg) _ANSI_ARGS_(TCL_VARARGS(DWORD,errorCode)); /* 2 */
    CONST char * (*expWinErrMsgVA) _ANSI_ARGS_((DWORD errorCode, va_list argList)); /* 3 */
    CONST char * (*expWinError) _ANSI_ARGS_(TCL_VARARGS(Tcl_Interp *,interp)); /* 4 */
#endif /* __WIN32__ */
} ExpIntPlatStubs;
TCL_EXTERNC ExpIntPlatStubs *expIntPlatStubsPtr;

#if defined(USE_EXP_STUBS) && !defined(USE_EXP_STUB_PROCS)

/*
 * Inline function declarations:
 */

#ifdef __WIN32__
#ifndef ExpWinInit
#define ExpWinInit \
	(expIntPlatStubsPtr->expWinInit) /* 0 */
#endif
#ifndef ExpWinErrId
#define ExpWinErrId \
	(expIntPlatStubsPtr->expWinErrId) /* 1 */
#endif
#ifndef ExpWinErrMsg
#define ExpWinErrMsg \
	(expIntPlatStubsPtr->expWinErrMsg) /* 2 */
#endif
#ifndef ExpWinErrMsgVA
#define ExpWinErrMsgVA \
	(expIntPlatStubsPtr->expWinErrMsgVA) /* 3 */
#endif
#ifndef ExpWinError
#define ExpWinError \
	(expIntPlatStubsPtr->expWinError) /* 4 */
#endif
#endif /* __WIN32__ */

#endif /* defined(USE_EXP_STUBS) && !defined(USE_EXP_STUB_PROCS) */

/* !END!: Do not edit above this line. */

#endif /* _EXPPLATINTDECLS */


