/* ----------------------------------------------------------------------------
 * exp.h --
 *
 *	Public include file for using the Expect extension.
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
 * Copyright (c) 2003-2005 ActiveState Corporation
 *	Work by David Gravereaux <davygrvy@pobox.com> for any Win32 OS.
 *	Based on work by Gordon Chaffee <chaffee@bmrc.berkeley.edu>
 *
 * ----------------------------------------------------------------------------
 * URLs:    http://expect.nist.gov/
 *	    http://expect.sf.net/
 *	    http://bmrc.berkeley.edu/people/chaffee/expectnt.html
 * ----------------------------------------------------------------------------
 * RCS: @(#) $Id: exp.h,v 1.1.4.10 2002/06/18 23:13:42 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#ifndef _EXP
#define _EXP


#ifndef _TCL
#   include "tcl.h"
#endif

#include <signal.h>

/*
 *  Version stuff.
 */

#define EXP_MAJOR_VERSION   5
#define EXP_MINOR_VERSION   43
#define EXP_RELEASE_LEVEL   TCL_FINAL_RELEASE
#define EXP_RELEASE_SERIAL  0

#define EXP_VERSION	    "5.43"
#define EXP_PATCH_LEVEL	    "5.43.0"

/*
 * The windows resource compiler defines this by default.  Skip the rest of this
 * file when included from an rc script.
 */

#ifndef RC_INVOKED


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


/*
 * Fix the Borland bug that's in the EXTERN macro from tcl.h.
 */
#ifndef TCL_EXTERN
#   undef DLLIMPORT
#   undef DLLEXPORT
#   ifdef __cplusplus
#	define TCL_EXTERNC extern "C"
#   else
#	define TCL_EXTERNC extern
#   endif
#   if defined(STATIC_BUILD)
#	define DLLIMPORT
#	define DLLEXPORT
#	define TCL_EXTERN(RTYPE) TCL_EXTERNC RTYPE
#   elif (defined(__WIN32__) && ( \
	    defined(_MSC_VER) || (__BORLANDC__ >= 0x0550) || \
	    defined(__LCC__) || defined(__WATCOMC__) || \
	    (defined(__GNUC__) && defined(__declspec)) \
	)) || (defined(MAC_TCL) && FUNCTION_DECLSPEC)
#	define DLLIMPORT __declspec(dllimport)
#	define DLLEXPORT __declspec(dllexport)
#	define TCL_EXTERN(RTYPE) TCL_EXTERNC TCL_STORAGE_CLASS RTYPE
#   elif defined(__BORLANDC__)
#	define DLLIMPORT __import
#	define DLLEXPORT __export
	/* Pre-5.5 Borland requires the attributes be placed after the */
	/* return type instead. */
#	define TCL_EXTERN(RTYPE) TCL_EXTERNC RTYPE TCL_STORAGE_CLASS
#   else
#	define DLLIMPORT
#	define DLLEXPORT
#	define TCL_EXTERN(RTYPE) TCL_EXTERNC TCL_STORAGE_CLASS RTYPE
#   endif
#endif


/* needed by some exports */
#ifdef TIME_WITH_SYS_TIME
#   include <sys/time.h>
#   include <time.h>
#else
#   ifdef HAVE_SYS_TIME_H
#	include <sys/time.h>
#   else
#	include <time.h>
#   endif
#endif


#define SCRIPTDIR	"example/"
#define EXECSCRIPTDIR	"example/"


/* common return codes for Expect functions */
/* The library actually only uses TIMEOUT and EOF */
#define EXP_ABEOF	-1	/* abnormal eof in Expect */
				/* when in library, this define is not used. */
				/* Instead "-1" is used literally in the */
				/* usual sense to check errors in system */
				/* calls */
#define EXP_TIMEOUT	-2
#define EXP_TCLERROR	-3
#define EXP_FULLBUFFER	-5
#define EXP_MATCH	-6
#define EXP_NOMATCH	-7
#define EXP_CANTMATCH	EXP_NOMATCH
#define EXP_CANMATCH	-8
#define EXP_DATA_NEW	-9	/* if select says there is new data */
#define EXP_DATA_OLD	-10	/* if we already read data in another cmd */
#define EXP_EOF		-11
#define EXP_RECONFIGURE	-12	/* changes to indirect spawn id lists */
				/* require us to reconfigure things */

/* in the unlikely event that a signal handler forces us to return this */
/* through expect's read() routine, we temporarily convert it to this. */
#define EXP_TCLRET	-20
#define EXP_TCLCNT	-21
#define EXP_TCLCNTTIMER	-22
#define EXP_TCLBRK	-23
#define EXP_TCLCNTEXP	-24
#define EXP_TCLRETTCL	-25

/* yet more TCL return codes */
/* Tcl does not safely provide a way to define the values of these, so */
/* use ridiculously different numbers for safety */
#define EXP_CONTINUE		-101	/* continue expect command */
					/* and restart timer */
#define EXP_CONTINUE_TIMER	-102	/* continue expect command */
					/* and continue timer */
#define EXP_TCL_RETURN		-103	/* converted by interact */
					/* and interpeter from */
					/* inter_return into */
					/* TCL_RETURN*/
#define EXP_TIME_INFINITY	-1

/*
 * Used by Exp_CreateSpawnChannel() to describe the job.
 */

typedef struct {
    Tcl_Obj *env;	/* environment list. */
    Tcl_Obj *dir;	/* directory where the application is started. */
    int ttyinit;
    int ttycopy;
    int echo;
    int console;
    int pty_only;
    int leaveOpen;
    int slave_write_ioctls;
    int slave_opens;
    int ignore[NSIG];		/* if true, signal in child is ignored */
				/* if false, signal gets default behavior */
} Exp_SpawnOptionSet;


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


typedef void (expDiagLogProc) _ANSI_ARGS_((CONST char *msg));
typedef CONST char *(expErrnoMsgProc) _ANSI_ARGS_((int errCode));

/*
 * Include the public function declarations that are accessible via
 * the stubs table.
 */

#include "expDecls.h"

/*
 * Include platform specific public function declarations that are
 * accessible via the stubs table.
 */

#include "expPlatDecls.h"

/*
 * Exp_InitStubs is used by apps/extensions that want to link
 * against the expect stubs library.  If we are not using stubs,
 * then this won't be declared.
 */

#ifdef USE_EXP_STUBS
TCL_EXTERNC CONST char *
Exp_InitStubs _ANSI_ARGS_((Tcl_Interp *interp, CONST char *version,
			  int exact));
#endif


#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLIMPORT

#endif /* RC_INVOKED */
#endif /* _EXP */
