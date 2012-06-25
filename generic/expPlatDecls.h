/* ----------------------------------------------------------------------------
 * expPlatDecls.h --
 *
 *	Declarations of platform specific Expect APIs.
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
 * RCS: @(#) $Id: expPlatDecls.h,v 1.1.4.2 2002/02/10 12:04:22 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#ifndef _EXPPLATDECLS
#define _EXPPLATDECLS

	    
/* !BEGIN!: Do not edit below this line. */

/*
 * Exported function declarations:
 */

#ifdef __WIN32__
#ifndef Exp_WinApplicationType_TCL_DECLARED
#define Exp_WinApplicationType_TCL_DECLARED
/* 0 */
TCL_EXTERN(int)		Exp_WinApplicationType _ANSI_ARGS_((
				CONST char * originalName, 
				Tcl_DString * fullName));
#endif
#ifndef Exp_WinBuildCommandLine_TCL_DECLARED
#define Exp_WinBuildCommandLine_TCL_DECLARED
/* 1 */
TCL_EXTERN(void)	Exp_WinBuildCommandLine _ANSI_ARGS_((
				CONST char * executable, int objc, 
				struct Tcl_Obj * const objv[], 
				Tcl_DString * linePtr));
#endif
#ifndef Exp_Win32ErrId_TCL_DECLARED
#define Exp_Win32ErrId_TCL_DECLARED
/* 2 */
TCL_EXTERN(CONST char *) Exp_Win32ErrId _ANSI_ARGS_((unsigned long errorCode));
#endif
#ifndef Exp_Win32ErrMsg_TCL_DECLARED
#define Exp_Win32ErrMsg_TCL_DECLARED
/* 3 */
TCL_EXTERN(CONST char *) Exp_Win32ErrMsg _ANSI_ARGS_(TCL_VARARGS(unsigned long,errorCode));
#endif
#ifndef Exp_Win32ErrMsgVA_TCL_DECLARED
#define Exp_Win32ErrMsgVA_TCL_DECLARED
/* 4 */
TCL_EXTERN(CONST char *) Exp_Win32ErrMsgVA _ANSI_ARGS_((
				unsigned long errorCode, va_list argList));
#endif
#ifndef Exp_Win32Error_TCL_DECLARED
#define Exp_Win32Error_TCL_DECLARED
/* 5 */
TCL_EXTERN(CONST char *) Exp_Win32Error _ANSI_ARGS_(TCL_VARARGS(Tcl_Interp *,interp));
#endif
#endif /* __WIN32__ */

typedef struct ExpPlatStubs {
    int magic;
    struct ExpPlatStubHooks *hooks;

#ifdef __WIN32__
    int (*exp_WinApplicationType) _ANSI_ARGS_((CONST char * originalName, Tcl_DString * fullName)); /* 0 */
    void (*exp_WinBuildCommandLine) _ANSI_ARGS_((CONST char * executable, int objc, struct Tcl_Obj * const objv[], Tcl_DString * linePtr)); /* 1 */
    CONST char * (*exp_Win32ErrId) _ANSI_ARGS_((unsigned long errorCode)); /* 2 */
    CONST char * (*exp_Win32ErrMsg) _ANSI_ARGS_(TCL_VARARGS(unsigned long,errorCode)); /* 3 */
    CONST char * (*exp_Win32ErrMsgVA) _ANSI_ARGS_((unsigned long errorCode, va_list argList)); /* 4 */
    CONST char * (*exp_Win32Error) _ANSI_ARGS_(TCL_VARARGS(Tcl_Interp *,interp)); /* 5 */
#endif /* __WIN32__ */
} ExpPlatStubs;
TCL_EXTERNC ExpPlatStubs *expPlatStubsPtr;

#if defined(USE_EXP_STUBS) && !defined(USE_EXP_STUB_PROCS)

/*
 * Inline function declarations:
 */

#ifdef __WIN32__
#ifndef Exp_WinApplicationType
#define Exp_WinApplicationType \
	(expPlatStubsPtr->exp_WinApplicationType) /* 0 */
#endif
#ifndef Exp_WinBuildCommandLine
#define Exp_WinBuildCommandLine \
	(expPlatStubsPtr->exp_WinBuildCommandLine) /* 1 */
#endif
#ifndef Exp_Win32ErrId
#define Exp_Win32ErrId \
	(expPlatStubsPtr->exp_Win32ErrId) /* 2 */
#endif
#ifndef Exp_Win32ErrMsg
#define Exp_Win32ErrMsg \
	(expPlatStubsPtr->exp_Win32ErrMsg) /* 3 */
#endif
#ifndef Exp_Win32ErrMsgVA
#define Exp_Win32ErrMsgVA \
	(expPlatStubsPtr->exp_Win32ErrMsgVA) /* 4 */
#endif
#ifndef Exp_Win32Error
#define Exp_Win32Error \
	(expPlatStubsPtr->exp_Win32Error) /* 5 */
#endif
#endif /* __WIN32__ */

#endif /* defined(USE_EXP_STUBS) && !defined(USE_EXP_STUB_PROCS) */

/* !END!: Do not edit above this line. */

#endif /* _EXPPLATDECLS */


