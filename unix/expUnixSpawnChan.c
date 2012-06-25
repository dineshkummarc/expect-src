/* ----------------------------------------------------------------------------
 * expUnixSpawnChan.c --
 *
 *	Unix specific.
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
 * RCS: @(#) $Id: $
 * ----------------------------------------------------------------------------
 */

#include "expUnixInt.h"

static Tcl_DriverCloseProc ExpSpawnClose;
static Tcl_DriverInputProc ExpSpawnInput;
static Tcl_DriverOutputProc ExpSpawnOutput;
static Tcl_DriverSetOptionProc ExpSpawnSetOption;
static Tcl_DriverGetOptionProc ExpSpawnGetOption;
static Tcl_DriverWatchProc ExpSpawnWatch;
static Tcl_DriverGetHandleProc ExpSpawnGetHandle;
static Tcl_DriverBlockModeProc	ExpSpawnBlock;

static Tcl_ChannelType ExpSpawnChannelType = {
    "spawn",
    TCL_CHANNEL_VERSION_2,
    ExpSpawnClose,
    ExpSpawnInput,
    ExpSpawnOutput,
    NULL,         		/* Can't seek! */
    ExpSpawnSetOption,
    ExpSpawnGetOption,
    ExpSpawnWatch,
    ExpSpawnGetHandle,
    NULL,
    ExpSpawnBlock,
    NULL,
    NULL
};

void
ExpSpawnInit (void)
{
}

/*
 *----------------------------------------------------------------------
 *
 * Exp_CreateSpawnChannel --
 *
 *	Creates a new 'spawn' channel for the unix flavor.
 *
 * Results:
 *      The new Tcl_Channel or NULL for an error.  If an error occurs,
 *	$errorCode will be set with the reason for the error.
 *    
 * Side Effects:
 *	A new process will be created.  opts.startingDir, if set,
 *	must be decremented when done.
 *
 *----------------------------------------------------------------------
 */

Tcl_Channel
Exp_CreateSpawnChannel (
    Tcl_Interp *interp,
    Exp_SpawnOptionSet *opts,
    int objc,
    struct Tcl_Obj * CONST objv[],
    unsigned long *pid)
{
    return NULL;
}


/*
 *----------------------------------------------------------------------
 *
 * ExpSpawnClose --
 *
 *	Generic routine to close the expect channel
 *
 * Results:
 *      0 if successful or a POSIX errorcode with
 *      interp updated.
 *    
 * Side Effects:
 *	Channel is deleted.
 *
 *----------------------------------------------------------------------
 */

static int
ExpSpawnClose(
    ClientData instanceData,
    Tcl_Interp *interp)
{
    /* TODO: free ExpState here */
    return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpSpawnInput --
 *
 *	Generic read routine for expect console
 *
 * Returns:
 *	Amount read or -1 with errorcode in errorPtr.
 *    
 * Side Effects:
 *	Buffer is updated. 
 *
 *----------------------------------------------------------------------
 */

static int
ExpSpawnInput(
    ClientData instanceData,
    char *bufPtr,		/* (in) Ptr to buffer */
    int bufSize,		/* (in) sizeof buffer */
    int *errorPtr)		/* (out) error code */
{
    /* TODO: get from stacked channel. */
    *errorPtr = EINVAL;
    return -1;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpSpawnOutput --
 *
 *	Write routine for an expect channel
 *
 * Results:
 *	Amount written or -1 with errorcode in errorPtr
 *    
 * Side Effects:
 *	None. 
 *
 *----------------------------------------------------------------------
 */

static int
ExpSpawnOutput(
    ClientData instanceData,
    CONST char *bufPtr,		/* (in) Ptr to buffer */
    int toWrite,		/* (in) amount to write */
    int *errorPtr)		/* (out) error code */
{
    /* TODO: forward to stacked channel. */
    *errorPtr = EINVAL;
    return -1;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpSpawnSetOption --
 *
 *	Set the value of an ExpPair channel option
 *
 * Results:
 *	TCL_OK and dsPtr updated with the value or TCL_ERROR.
 *
 * Side Effects
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
ExpSpawnSetOption(
    ClientData instanceData,
    Tcl_Interp *interp,
    CONST char *nameStr,	/* (in) Name of option */
    CONST char *valStr)		/* (in) New value of option */
{
    /* TODO: dunno yet */
    return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpSpawnGetOption --
 *
 *	Queries ExpSpawn channel for the current value of
 *      the given option.
 *
 * Results:
 *	TCL_OK and dsPtr updated with the value or TCL_ERROR.
 *
 * Side Effects
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
ExpSpawnGetOption(
    ClientData instanceData,
    Tcl_Interp *interp,
    CONST char *nameStr,	/* (in) Name of option to retrieve */
    Tcl_DString *dsPtr)		/* (in) String to place value */
{
    /* TODO: dunno yet */
    return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpSpawnWatch --
 *
 *	Sets up event handling on a expect port Tcl_Channel using
 *	the underlying channel type.
 *
 * Results:
 *	Nothing
 *
 * Side Effects
 *	None.
 *
 *----------------------------------------------------------------------
 */

void
ExpSpawnWatch(
    ClientData instanceData,
    int mask)
{
    /* TODO: dunno yet. */
    return;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpSpawnGetHandle --
 *
 *	Get the Tcl_File for the appropriate direction in from the
 *	Tcl_Channel.
 *
 * Results:
 *	NULL because ExpSpawn ids are handled through other channel
 *	types.
 *
 * Side Effects
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
ExpSpawnGetHandle(
    ClientData instanceData,
    int direction,
    ClientData *handlePtr)
{
    *handlePtr = NULL;
    return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpSpawnBlock --
 *
 *	Generic routine to set I/O to blocking or non-blocking.
 *
 * Results:
 *	TCL_OK or TCL_ERROR.
 *    
 * Side Effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
ExpSpawnBlock(
    ClientData instanceData,
    int mode)			/* (in) Block or not */
{
    /* TODO: dunno yet */
    return TCL_ERROR;
}
