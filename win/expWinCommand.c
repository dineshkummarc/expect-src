/* ----------------------------------------------------------------------------
 * expWinCommand.c --
 *
 *	Implements Windows specific parts required by expCommand.c.
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
 * RCS: @(#) $Id: expWinCommand.c,v 1.1.2.1.2.13 2003/08/26 20:46:52 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expInt.h"

#ifdef TCL_DEBUGGER
#include "Dbg.h"
#endif


/*
 *----------------------------------------------------------------------
 *
 * exp_f_new_platform --
 *
 *	Platform specific initialization of exp_f structure
 *
 * Results:
 *	TRUE if successful, FALSE if unsuccessful.
 *
 * Side Effects:
 *	None
 *
 *----------------------------------------------------------------------
 */
#if 0
int
exp_f_new_platform(f)
    struct exp_f *f;
{
    if (EXP_NOPID != f->pid) {
	f->tclPid = (Tcl_Pid)
	    OpenProcess(PROCESS_ALL_ACCESS, FALSE, f->pid);
	TclWinAddProcess((HANDLE) f->tclPid, f->pid);
    } else {
	f->tclPid = (Tcl_Pid) INVALID_HANDLE_VALUE;
    }

    /* WIN32 only fields */
    f->over.hEvent = NULL;
    return TRUE;
}
#endif
/*
 *----------------------------------------------------------------------
 *
 * exp_f_free_platform --
 *
 *	Frees any platform specific pieces of the exp_f structure.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */

#if 0
void
exp_f_free_platform(f)
    struct exp_f *f;
{
    /* Tcl_WaitPid closes the handle */

//    if (f->tclPid != (Tcl_Pid) INVALID_HANDLE_VALUE) {
//	__try {
//	    CloseHandle((HANDLE) f->tclPid);
//	}
//	__except (1) {};
//    }
}
#endif

void
exp_close_on_exec(fd)
    int fd;
{
    /* This is here for place keeping purposes */
}

/*
 *----------------------------------------------------------------------
 *
 * exp_getpidproc --
 *
 *	Return the process id for this process
 *
 * Results:
 *	A process id
 *
 *----------------------------------------------------------------------
 */

int
exp_getpidproc()
{
    return GetCurrentProcessId();
}
