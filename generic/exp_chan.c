/* ----------------------------------------------------------------------------
 * exp_chan.c --
 *
 *	Implements the generic 'exp' and 'exp_pair' channel types that only
 *	act as containers for the specific one in use.  The ExpState is stored
 *	in the instanceData and operations are forwarded to the specific channel
 *	in use.  This layering allows use to not be concerned about what
 *	is inside a channel that we are 'expecting' upon.
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

#include "expInt.h"

static Tcl_DriverCloseProc ExpChanClose;
static Tcl_DriverInputProc ExpChanInput;
static Tcl_DriverOutputProc ExpChanOutput;
static Tcl_DriverSetOptionProc ExpChanSetOption;
static Tcl_DriverGetOptionProc ExpChanGetOption;
static Tcl_DriverWatchProc ExpChanWatch;
static Tcl_DriverGetHandleProc ExpChanGetHandle;
static Tcl_DriverBlockModeProc	ExpChanBlock;
static Tcl_DriverFlushProc ExpChanFlush;
//static Tcl_DriverHandlerProc ExpChanHandler;

static Tcl_FileProc ExpChanReadable;
static Tcl_FileProc ExpChanWritable;
static Tcl_CloseProc ExpChanCloseHandler;

Tcl_ChannelType ExpChannelType = {
    "exp",
    TCL_CHANNEL_VERSION_2,
    ExpChanClose,		/* Close proc. */
    ExpChanInput,		/* Input proc. */
    ExpChanOutput,		/* Output proc. */
    NULL,         		/* No seek! Expect channels are sequential. */
    ExpChanSetOption,		/* Set option proc. */
    ExpChanGetOption,		/* Get option proc. */
    ExpChanWatch,		/* Initialize notifier. */
    ExpChanGetHandle,		/* Get OS handles out of channel. */
    NULL,			/* No close2proc used. */
    ExpChanBlock,		/* Set blocking/nonblocking mode.*/
    ExpChanFlush,		/* Flush proc. */
    NULL,			/* Handle channel event proc. */
};

typedef struct ExpPairState {
    Tcl_Channel thisChannelPtr;	/* The toplevel channel */
    Tcl_Channel inChannelPtr;	/* The input child channel */
    Tcl_Channel outChannelPtr;	/* The output child channel */
    int watchMask;		/* Events that are being checked for */
    int blockingPropagate;	/* Propagate a blocking option to children */
    struct ExpPairState *nextPtr;
} ExpPairState;


typedef struct ThreadSpecificData {
    int initialized;

    /*
     * List of all exp channels currently open.  This is per thread and is
     * used to match up fd's to channels, which rarely occurs.
     */

    ExpState *firstExpPtr;
    ExpPairState *firstExpPairPtr;
    int channelCount;	 /* this is process-wide as it is used to
			     give user some hint as to why a spawn has failed
			     by looking at process-wide resource usage */
} ThreadSpecificData;
static Tcl_ThreadDataKey dataKey;

static int expIds = 0;

void
expChanInit()
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    tsdPtr->initialized = 1;
    tsdPtr->channelCount = 0;
}

Tcl_Channel
Exp_CreateExpChannel (
    Tcl_Interp *interp,
    Tcl_Channel chan,	    /* channel we want to wrap */
    int pid,		    /* pid, if any */
    Tcl_Pid tclPid,	    /* pid (HANDLE on Win32), if any */
    ExpState **esOut)
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    ExpState *esPtr;
    int mask;
    Tcl_DString dStr;

    esPtr = (ExpState *) ckalloc((unsigned) sizeof(ExpState));
    esPtr->nextPtr = tsdPtr->firstExpPtr;
    tsdPtr->firstExpPtr = esPtr;

    if (esOut != NULL) {
	*esOut = esPtr;
    }

    /*
     * For now, stupidly assume this.  We we will likely have to revisit this
     * later to prevent people from doing stupid things.
     */

    mask = TCL_READABLE | TCL_WRITABLE;

    sprintf(esPtr->name,"exp%d", expIds++);
    esPtr->channel = Tcl_CreateChannel(&ExpChannelType, esPtr->name,
	    (ClientData) esPtr, mask);
    Tcl_RegisterChannel(interp, esPtr->channel);
    esPtr->registered = TRUE;

    /* not sure about this - what about adopted channels */
    esPtr->validMask = mask | TCL_EXCEPTION;
    esPtr->watchMask = 0;
    esPtr->fdBusy = FALSE;

    esPtr->slave = chan;
    
    Tcl_CreateCloseHandler(chan, ExpChanCloseHandler, (ClientData) esPtr);

    Tcl_SetChannelOption(interp,esPtr->channel,"-buffering","none");
    Tcl_SetChannelOption(interp,esPtr->channel,"-blocking","0");
    Tcl_SetChannelOption(interp,esPtr->channel,"-translation","auto lf");

    /* Ensure encoding of the top channel is the same as the lower one. */
    Tcl_DStringInit(&dStr);
    if (Tcl_GetChannelOption(interp, chan, "-encoding", &dStr) != TCL_ERROR) {
	Tcl_SetChannelOption(interp, esPtr->channel,
		"-encoding", Tcl_DStringValue(&dStr));
    }
    Tcl_DStringFree(&dStr);

    esPtr->pid = pid;
    esPtr->tclPid = tclPid;
    esPtr->msize = 0;

    /* initialize a dummy buffer */
    esPtr->buffer = Tcl_NewStringObj("",0);
    Tcl_IncrRefCount(esPtr->buffer);
    esPtr->umsize = exp_default_match_max;
    /* this will reallocate object with an appropriate sized buffer */
    expAdjust(esPtr);

    esPtr->printed = 0;
    esPtr->echoed = 0;
    esPtr->rm_nulls = exp_default_rm_nulls;
    esPtr->parity = exp_default_parity;
    esPtr->close_on_eof = exp_default_close_on_eof;
    esPtr->key = expect_key++;
    esPtr->force_read = FALSE;
    esPtr->fg_armed = FALSE;
#ifdef HAVE_PTYTRAP
    esPtr->slave_name = 0;
#endif /* HAVE_PTYTRAP */
    esPtr->open = TRUE;
    esPtr->notified = FALSE;
    esPtr->user_waited = FALSE;
    esPtr->sys_waited = FALSE;
    esPtr->bg_interp = 0;
    esPtr->bg_status = unarmed;
    esPtr->bg_ecount = 0;
    esPtr->freeWhenBgHandlerUnblocked = FALSE;
    esPtr->keepForever = FALSE;
    esPtr->valid = TRUE;
    tsdPtr->channelCount++;

    return esPtr->channel;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpChanCloseHandler --
 *
 *	This gets called when the underlying channel is closed.  Not
 *	likely, but possible.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */

static void
ExpChanCloseHandler (
    ClientData instanceData)
{
    ExpState *esPtr = (ExpState *) instanceData;
    esPtr->slave = NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpChanClose --
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
ExpChanClose (
    ClientData instanceData,
    Tcl_Interp *interp)
{
    ExpState *esPtr = (ExpState *) instanceData;
    Tcl_Channel channel = esPtr->slave;
    ExpState **nextPtrPtr;
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    int result = TCL_OK;

    Tcl_DecrRefCount(esPtr->buffer);

    /*
     * Conceivably, the process may not yet have been waited for.  If this
     * becomes a requirement, we'll have to revisit this code.  But for now, if
     * it's just Tcl exiting, the processes will exit on their own soon
     * anyway.
     */

    for (nextPtrPtr = &(tsdPtr->firstExpPtr); (*nextPtrPtr) != NULL;
	 nextPtrPtr = &((*nextPtrPtr)->nextPtr)) {
	if ((*nextPtrPtr) == esPtr) {
	    (*nextPtrPtr) = esPtr->nextPtr;
	    break;
	}
    }
    tsdPtr->channelCount--;

    if (esPtr->bg_status == blocked ||
	    esPtr->bg_status == disarm_req_while_blocked) {
	esPtr->freeWhenBgHandlerUnblocked = 1;
	/*
	 * If we're in the middle of a bg event handler, then the event
	 * handler will have to take care of freeing esPtr.
	 */
    } else {
	if (!esPtr->keepForever) {
	    /* Don't close if we're being called from TclFinalizeIOSubsystem() */
	    if (!TclInExit() && channel) {
		result = Tcl_Close(interp, channel);
	    }
	}
	expStateFree(esPtr);
	return result;
    }

    esPtr->registered = FALSE;

    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpChanInput --
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
ExpChanInput (
    ClientData instanceData,
    char *bufPtr,		/* (in) Ptr to buffer */
    int bufSize,		/* (in) sizeof buffer */
    int *errorPtr)		/* (out) error code */
{
    ExpState *esPtr = (ExpState *) instanceData;
    Tcl_Channel channel = esPtr->slave;
    int bytesRead;			/* How many bytes were actually
                                         * read from the input device? */
    if (!channel) {
	*errorPtr = EPIPE;
	return -1;
    }

    if (Tcl_ChannelInputProc(Tcl_GetChannelType(channel))) {
	bytesRead = (Tcl_ChannelInputProc(Tcl_GetChannelType(channel)))
		(Tcl_GetChannelInstanceData(channel), bufPtr, bufSize,
		errorPtr);

	if (bytesRead > 0) {
	    /* strip parity if requested */
	    if (esPtr->parity == 0) {
		char *end = bufPtr + bytesRead;
		for (;bufPtr < end; bufPtr++) {
		    *bufPtr &= 0x7f;
		}
	    }
	}
	return bytesRead;
    }
    *errorPtr = EINVAL;
    return -1;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpChanOutput --
 *
 *	Write routine for an expect channel
 *
 * Results:
 *	Amount written or -1 with POSIX errorcode in errorPtr
 *    
 * Side Effects:
 *	None. 
 *
 *----------------------------------------------------------------------
 */

static int
ExpChanOutput (
    ClientData instanceData,
    CONST char *bufPtr,		/* (in) Ptr to buffer */
    int toWrite,		/* (in) amount to write */
    int *errorPtr)		/* (out) error code */
{
    ExpState *esPtr = (ExpState *) instanceData;
    Tcl_Channel channel = esPtr->slave;

    if (toWrite < 0) {
	Tcl_Panic("ExpChanOutput: called with negative char count");
    }

    if (!channel) {
	*errorPtr = EPIPE;
	return -1;
    }

    if (Tcl_ChannelOutputProc(Tcl_GetChannelType(channel))) {
	return (Tcl_ChannelOutputProc(Tcl_GetChannelType(channel)))
		(Tcl_GetChannelInstanceData(channel), bufPtr, toWrite,
		errorPtr);
    }
    *errorPtr = EINVAL;
    return -1;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpChanSetOption --
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
ExpChanSetOption (
    ClientData instanceData,
    Tcl_Interp *interp,
    CONST char *nameStr,	/* (in) Name of option */
    CONST char *valStr)		/* (in) New value of option */
{
    ExpState *esPtr = (ExpState *) instanceData;
    Tcl_Channel channel = esPtr->slave;

    if (channel && Tcl_ChannelSetOptionProc(Tcl_GetChannelType(channel))) {
	return (Tcl_ChannelSetOptionProc(Tcl_GetChannelType(channel)))
		(Tcl_GetChannelInstanceData(channel), interp, nameStr, valStr);
    } else {
	return TCL_ERROR;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * ExpChanGetOption --
 *
 *	Queries ExpChan channel for the current value of
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
ExpChanGetOption (
    ClientData instanceData,
    Tcl_Interp *interp,
    CONST char *nameStr,	/* (in) Name of option to retrieve */
    Tcl_DString *dsPtr)		/* (in) String to place value */
{
    ExpState *esPtr = (ExpState *) instanceData;
    Tcl_Channel channel = esPtr->slave;

    if (channel && Tcl_ChannelGetOptionProc(Tcl_GetChannelType(channel))) {
	return (Tcl_ChannelGetOptionProc(Tcl_GetChannelType(channel)))
		(Tcl_GetChannelInstanceData(channel), interp, nameStr, dsPtr);
    } else {
	return TCL_ERROR;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * ExpChanWatch --
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
ExpChanWatch (
    ClientData instanceData,
    int mask)
{
    ExpState *esPtr = (ExpState *) instanceData;
    Tcl_Channel channel = esPtr->slave;
    int old_mask = esPtr->watchMask;

    if (!channel) {
	return;
    }

    /*
     * Make sure we only register for events that are valid on this exp.
     */

    mask &= esPtr->validMask;

    if (mask & TCL_READABLE) {
	if (!(old_mask & TCL_READABLE)) {
	    Tcl_CreateChannelHandler(channel, TCL_READABLE,
		ExpChanReadable, instanceData);
	}
    } else if (old_mask & TCL_READABLE) {
	Tcl_DeleteChannelHandler(channel, ExpChanReadable, instanceData);
    }

    if (mask & TCL_WRITABLE) {
	if (!(old_mask & TCL_WRITABLE)) {
	    Tcl_CreateChannelHandler(channel, TCL_WRITABLE,
		ExpChanReadable, instanceData);
	}
    } else if (old_mask & TCL_WRITABLE) {
	Tcl_DeleteChannelHandler(channel, ExpChanWritable, instanceData);
    }

    if (Tcl_ChannelWatchProc(Tcl_GetChannelType(channel))) {
	Tcl_ChannelWatchProc(Tcl_GetChannelType(channel))
	    (Tcl_GetChannelInstanceData(channel), mask);
    }

    esPtr->watchMask = mask;
    return;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpChanGetHandle --
 *
 *	Get the Tcl_File for the appropriate direction in from the
 *	Tcl_Channel.
 *
 * Results:
 *	Whatever the underlying channel decides.
 *
 * Side Effects
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
ExpChanGetHandle (
    ClientData instanceData,
    int direction,
    ClientData *handlePtr)
{
    ExpState *esPtr = (ExpState *) instanceData;
    Tcl_Channel channel = esPtr->slave;

    if (channel && Tcl_ChannelGetHandleProc(Tcl_GetChannelType(channel))) {
	return (Tcl_ChannelGetHandleProc(Tcl_GetChannelType(channel)))
		(Tcl_GetChannelInstanceData(channel), direction, handlePtr);
    } else {
	return TCL_ERROR;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * ExpChanBlock --
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
ExpChanBlock (
    ClientData instanceData,
    int mode)			/* (in) Block or not */
{
    ExpState *esPtr = (ExpState *) instanceData;
    Tcl_Channel channel = esPtr->slave;

    if (channel && Tcl_ChannelBlockModeProc(Tcl_GetChannelType(channel))) {
	return (Tcl_ChannelBlockModeProc(Tcl_GetChannelType(channel)))
		(Tcl_GetChannelInstanceData(channel), mode);
    } else {
	return TCL_ERROR;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * ExpChanFlush --
 *
 *	Generic routine to forward flush notices.
 *
 * Results:
 *	TCL_OK or TCL_ERROR.
 *
 * Side Effects:
 *	Whatever the slave channel does.
 *
 *----------------------------------------------------------------------
 */

static int
ExpChanFlush (
    ClientData instanceData)
{
    ExpState *esPtr = (ExpState *) instanceData;
    Tcl_Channel channel = esPtr->slave;

    if (channel && Tcl_ChannelFlushProc(Tcl_GetChannelType(channel))) {
	return (Tcl_ChannelFlushProc(Tcl_GetChannelType(channel)))
		(Tcl_GetChannelInstanceData(channel));
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpChanReadable --
 *
 *	Callback when an event occurs in the input channel.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	An event is generated for this channel.
 *
 *----------------------------------------------------------------------
 */

static void
ExpChanReadable (
    ClientData instanceData,
    int mask)
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    ExpState *esPtr = (ExpState *) instanceData;
    Tcl_Channel channel = esPtr->channel;

    if (! tsdPtr->initialized) {
	expChanInit();
    }
    /* forward up to ourselves from the contained channel */
    Tcl_NotifyChannel(channel, mask);
}

/*
 *----------------------------------------------------------------------
 *
 * ExpChanWritable --
 *
 *	Callback when an event occurs in the output channel.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	An event is generated for this channel.
 *
 *----------------------------------------------------------------------
 */

static void
ExpChanWritable (
    ClientData instanceData,
    int mask)
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    ExpState *esPtr = (ExpState *) instanceData;
    Tcl_Channel channel = esPtr->channel;

    if (! tsdPtr->initialized) {
	expChanInit();
    }
    /* forward up to ourselves from the contained channel */
    Tcl_NotifyChannel(channel, mask);
}


int
expChannelCountGet()
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    return tsdPtr->channelCount;
}

int
expSizeGet(esPtr)
    ExpState *esPtr;
{
    int len;
    Tcl_GetStringFromObj(esPtr->buffer, &len);
    return len;
}

int
expSizeZero(esPtr)
    ExpState *esPtr;
{
    int len;
    Tcl_GetStringFromObj(esPtr->buffer, &len);
    return (len == 0);
}

/* return 0 for success or negative for failure */
int
expWriteChars(esPtr,buffer,lenBytes)
    ExpState *esPtr;
    CONST char *buffer;
    int lenBytes;
{
    int rc;

    do {
	rc = Tcl_WriteChars(esPtr->channel, buffer, lenBytes);
    } while ((rc == -1) && (errno == EAGAIN));

    /* just return 0 rather than positive byte counts */
    return ((rc > 0) ? 0 : rc);
}


void
expStateFree(esPtr)
    ExpState *esPtr;
{
#ifndef __WIN32__
    if (esPtr->fdBusy) {
	close(esPtr->fdin);
    }
#endif

    esPtr->valid = FALSE;
    
    if (!esPtr->keepForever) {
	ckfree((char *)esPtr);
    }
}

/* close all connections
 * 
 * The kernel would actually do this by default, however Tcl is going to come
 * along later and try to reap its exec'd processes.  If we have inherited any
 * via spawn -open, Tcl can hang if we don't close the connections first.
 */
void
exp_close_all (
    Tcl_Interp *interp)
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    ExpState *esPtr;
    ExpState *esNextPtr;
    ExpPairState *epsPtr;
    ExpPairState *epsNextPtr;

    /* Save the nextPtr in a local variable before calling 'exp_close'
       as 'expStateFree' can be called from it under some
       circumstances, possibly causing the memory allocator to smash
       the value in 'esPtr'. - Andreas Kupries
    */

    /* no need to keep things in sync (i.e., tsdPtr, count) since we could only
       be doing this if we're exiting.  Just close everything down. */

    for (esPtr = tsdPtr->firstExpPtr;esPtr;esPtr = esNextPtr) {
        esNextPtr = esPtr->nextPtr;
	exp_close(interp,esPtr);
    }
    for (epsPtr = tsdPtr->firstExpPairPtr;epsPtr;epsPtr = epsNextPtr) {
        epsNextPtr = epsPtr->nextPtr;
	Tcl_UnregisterChannel(interp,epsPtr->thisChannelPtr);
	//Tcl_Close(interp,epsPtr->thisChannelPtr);
    }
}

/* wait for any of our own spawned processes we call waitpid rather
 * than wait to avoid running into someone else's processes.  Yes,
 * according to Ousterhout this is the best way to do it.
 * returns the ExpState or 0 if nothing to wait on */
ExpState *
expWaitOnAny()
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    Tcl_Pid result;
    ExpState *esPtr;

    for (esPtr = tsdPtr->firstExpPtr;esPtr;esPtr = esPtr->nextPtr) {
	if (esPtr->pid == exp_getpid) continue; /* skip ourself */
	if (esPtr->user_waited) continue;	/* one wait only! */
	if (esPtr->sys_waited) break;
      restart:
	result = Tcl_WaitPid(esPtr->tclPid, &esPtr->wait, WNOHANG);
	if (result == esPtr->tclPid) break;
	if (result == (Tcl_Pid) 0) continue;	/* busy, try next */
	if (result == (Tcl_Pid) -1) {
	    if (errno == EINTR) goto restart;
	    else break;
	}
    }
    return esPtr;
}

#ifndef __WIN32__
ExpState *
expWaitOnOne() {
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    ExpState *esPtr = NULL;
    int pid;
    /* should really be recoded using the common wait code in command.c */
    WAIT_STATUS_TYPE status;

    pid = wait(&status);
    for (esPtr = tsdPtr->firstExpPtr; esPtr; esPtr = esPtr->nextPtr) {
	if (esPtr->pid == pid) {
	    esPtr->sys_waited = TRUE;
	    esPtr->wait = status;
	    break;
	}
    }
    return esPtr;
}
#else
/* need to emulate this somehow */
ExpState *
expWaitOnOne() {
    return NULL;
}
#endif

void
exp_background_channelhandlers_run_all()
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    ExpState *esPtr;

    /* kick off any that already have input waiting */
    for (esPtr = tsdPtr->firstExpPtr;esPtr;esPtr = esPtr->nextPtr) {
	/* is bg_interp the best way to check if armed? */
	if (esPtr->bg_interp && !expSizeZero(esPtr)) {
	    exp_background_channelhandler((ClientData)esPtr,0);
	}
    }
}




static Tcl_DriverCloseProc ExpPairClose;
static Tcl_DriverInputProc ExpPairInput;
static Tcl_DriverOutputProc ExpPairOutput;
static Tcl_DriverSetOptionProc ExpPairSetOption;
static Tcl_DriverGetOptionProc ExpPairGetOption;
static Tcl_DriverWatchProc ExpPairWatch;
static Tcl_DriverGetHandleProc ExpPairGetHandle;
static Tcl_DriverBlockModeProc	ExpPairBlock;

static Tcl_CloseProc ExpPairInputCloseHandler;
static Tcl_CloseProc ExpPairOutputCloseHandler;
static Tcl_FileProc ExpPairReadable;
static Tcl_FileProc ExpPairWritable;

static Tcl_ChannelType ExpPairChannelType = {
    "exp_pair",
    TCL_CHANNEL_VERSION_2,
    ExpPairClose,
    ExpPairInput,
    ExpPairOutput,
    NULL,         		/* Can't seek! */
    ExpPairSetOption,
    ExpPairGetOption,
    ExpPairWatch,
    ExpPairGetHandle,
    NULL,
    ExpPairBlock,
    NULL,
    NULL
};


static int expPairCount = 0;

/*
 *----------------------------------------------------------------------
 *
 * ExpCreatePairChannel --
 *
 *	Routine that wraps an input channel and an output channel
 *	into a single channel.  By default, no translation or buffering
 *	occurs in this channel.
 *
 * Results:
 *	A Tcl_Channel.
 *    
 * Side Effects:
 *	Allocates memory.
 *
 *----------------------------------------------------------------------
 */

Tcl_Channel
Exp_CreatePairChannel (
    Tcl_Interp *interp,
    Tcl_Channel chanIn,
    Tcl_Channel chanOut,
    CONST char *chanName)		/* Name of resulting channel to create.
				 * If NULL, it gets created here */
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    Tcl_Channel chan;
    Tcl_DString dStr;
    ExpPairState *epsPtr;
    char channelNameStr[10];

    if (chanIn) {
	if ((Tcl_GetChannelMode(chanIn) & TCL_READABLE) == 0) {
	    Tcl_AppendResult(interp, Tcl_GetChannelName(chanIn), " is not a readable channel",
			     (char *) NULL);
	    return NULL;
	}
    } else {
	return NULL;
    }

    if (chanOut) {
	if ((Tcl_GetChannelMode(chanOut) & TCL_WRITABLE) == 0) {
	    Tcl_AppendResult(interp, Tcl_GetChannelName(chanOut), " is not a writable channel",
			     (char *) NULL);
	    return NULL;
	}
    } else {
	return NULL;
    }

    if (chanName == NULL) {
	sprintf(channelNameStr, "exp_pair%d", expPairCount++);
	chanName = channelNameStr;
    }
    epsPtr = (ExpPairState *) ckalloc(sizeof(ExpPairState));
    epsPtr->nextPtr = tsdPtr->firstExpPairPtr;
    tsdPtr->firstExpPairPtr = epsPtr;
    epsPtr->inChannelPtr = chanIn;
    epsPtr->outChannelPtr = chanOut;

    chan = Tcl_CreateChannel(&ExpPairChannelType, chanName,
			     (ClientData) epsPtr, TCL_READABLE|TCL_WRITABLE);
    if (chan == NULL) {
	ckfree((char *)epsPtr);
	return NULL;
    }
    epsPtr->thisChannelPtr = chan;
    epsPtr->watchMask = 0;
    epsPtr->blockingPropagate = 0;

    Tcl_CreateCloseHandler(chanIn, ExpPairInputCloseHandler,
	(ClientData) epsPtr);
    Tcl_CreateCloseHandler(chanOut, ExpPairOutputCloseHandler,
	(ClientData) epsPtr);

    /*
     * Setup the expect channel to always flush immediately
     */

    Tcl_SetChannelOption(interp, chan, "-buffering", "none");
    Tcl_SetChannelOption(interp, chan, "-translation","binary");
    Tcl_SetChannelOption(interp, chan, "-blockingpropagate", "off");
    Tcl_SetChannelOption(interp, chan, "-blocking", "off");

    /* Ensure encoding of the top channel is the same as the lower one. */
    Tcl_DStringInit(&dStr);
    if (Tcl_GetChannelOption(interp, chanOut, "-encoding", &dStr) != TCL_ERROR) {
	Tcl_SetChannelOption(interp, chan,
		"-encoding", Tcl_DStringValue(&dStr));
    }
    Tcl_DStringFree(&dStr);
    return chan;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpPairInputCloseHandler --
 *
 *	This gets called when the underlying input channel is closed.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */

static void
ExpPairInputCloseHandler (
    ClientData instanceData)
{
    ExpPairState *epsPtr = (ExpPairState *) instanceData;
    epsPtr->inChannelPtr = NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpPairOutputCloseHandler --
 *
 *	This gets called when the underlying output channel is closed.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */

static void
ExpPairOutputCloseHandler (
    ClientData instanceData)
{
    ExpPairState *epsPtr = (ExpPairState *) instanceData;
    epsPtr->outChannelPtr = NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpPairBlock --
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
ExpPairBlock (
    ClientData instanceData,
    int mode)			/* (in) Block or not */
{
    ExpPairState *ssPtr = (ExpPairState *) instanceData;
    Tcl_Channel inChannelPtr = ssPtr->inChannelPtr;
    Tcl_Channel outChannelPtr = ssPtr->outChannelPtr;
    int ret;

    if (! ssPtr->blockingPropagate) {
	return TCL_OK;
    }
    if (inChannelPtr && Tcl_ChannelBlockModeProc(Tcl_GetChannelType(inChannelPtr))) {
	ret = (Tcl_ChannelBlockModeProc(Tcl_GetChannelType(inChannelPtr)))
	    (Tcl_GetChannelInstanceData(inChannelPtr), mode);
	if (ret == TCL_ERROR) {
	    return ret;
	}
    }
    if (outChannelPtr && Tcl_ChannelBlockModeProc(Tcl_GetChannelType(outChannelPtr))) {
	return (Tcl_ChannelBlockModeProc(Tcl_GetChannelType(outChannelPtr)))
	    (Tcl_GetChannelInstanceData(outChannelPtr), mode);
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpPairInput --
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
ExpPairInput (
    ClientData instanceData,
    char *bufPtr,		/* (in) Ptr to buffer */
    int bufSize,		/* (in) sizeof buffer */
    int *errorPtr)		/* (out) error code */
{
    ExpPairState *ssPtr = (ExpPairState *)instanceData;
    Tcl_Channel channelPtr = ssPtr->inChannelPtr;

    if (! channelPtr) {
	*errorPtr = EPIPE;
	return -1;
    }

    if (channelPtr && Tcl_ChannelInputProc(Tcl_GetChannelType(channelPtr))) {
	return (Tcl_ChannelInputProc(Tcl_GetChannelType(channelPtr)))
	    (Tcl_GetChannelInstanceData(channelPtr), bufPtr, bufSize, errorPtr);
    }
    *errorPtr = EINVAL;
    return -1;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpPairOutput --
 *
 *	Write routine for expect console
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
ExpPairOutput (
    ClientData instanceData,
    CONST char *bufPtr,		/* (in) Ptr to buffer */
    int toWrite,		/* (in) amount to write */
    int *errorPtr)		/* (out) error code */
{
    ExpPairState *ssPtr = (ExpPairState *)instanceData;
    Tcl_Channel channelPtr = ssPtr->outChannelPtr;

    if (! channelPtr) {
	*errorPtr = EPIPE;
	return -1;
    }

    if (channelPtr && Tcl_ChannelOutputProc(Tcl_GetChannelType(channelPtr))) {
	return (Tcl_ChannelOutputProc(Tcl_GetChannelType(channelPtr)))
	    (Tcl_GetChannelInstanceData(channelPtr), bufPtr, toWrite, errorPtr);
    }
    *errorPtr = EINVAL;
    return -1;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpPairClose --
 *
 *	Generic routine to close the expect console
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
ExpPairClose (
    ClientData instanceData,
    Tcl_Interp *interp)
{
    ExpPairState *epsPtr = (ExpPairState *) instanceData;
    ExpPairState **nextPtrPtr;
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);

    for (nextPtrPtr = &(tsdPtr->firstExpPairPtr); (*nextPtrPtr) != NULL;
	 nextPtrPtr = &((*nextPtrPtr)->nextPtr)) {
	if ((*nextPtrPtr) == epsPtr) {
	    (*nextPtrPtr) = epsPtr->nextPtr;
	    break;
	}
    }

    /*
     *  Only let them go, don't actually close them.
     */

    if (epsPtr->inChannelPtr) {
	Tcl_DeleteCloseHandler(epsPtr->inChannelPtr, ExpPairInputCloseHandler,
	    (ClientData) epsPtr);
    }
    if (epsPtr->outChannelPtr) {
	Tcl_DeleteCloseHandler(epsPtr->outChannelPtr, ExpPairOutputCloseHandler,
	    (ClientData) epsPtr);
    }
    ckfree((char *)epsPtr);

    return TCL_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * ExpPairSetOption --
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
ExpPairSetOption (
    ClientData instanceData,
    Tcl_Interp *interp,
    CONST char *nameStr,		/* (in) Name of option */
    CONST char *valStr)			/* (in) New value of option */
{
    ExpPairState *ssPtr = (ExpPairState *) instanceData;
    Tcl_Channel inChannelPtr = ssPtr->inChannelPtr;
    Tcl_Channel outChannelPtr = ssPtr->outChannelPtr;
    int ret1, ret2;
    Tcl_DString dString;
    int len;
    int newMode;

    len = strlen(nameStr);
    if (strcmp(nameStr, "-blockingpropagate") == 0) {
        if (Tcl_GetBoolean(interp, valStr, &newMode) == TCL_ERROR) {
            return TCL_ERROR;
        }
	ssPtr->blockingPropagate = newMode;
	return TCL_OK;
    }

    /*
     * If the option can be applied to either channel, the result is OK.
     */
    ret1 = ret2 = TCL_OK;
    if (inChannelPtr && Tcl_ChannelSetOptionProc(Tcl_GetChannelType(inChannelPtr))) {
	ret1 = (Tcl_ChannelSetOptionProc(Tcl_GetChannelType(inChannelPtr)))
	    (Tcl_GetChannelInstanceData(inChannelPtr), interp, nameStr, valStr);
    }
    if (outChannelPtr && Tcl_ChannelSetOptionProc(Tcl_GetChannelType(outChannelPtr))) {
	Tcl_DStringInit(&dString);
	Tcl_DStringGetResult(interp, &dString);
	ret2 = (Tcl_ChannelSetOptionProc(Tcl_GetChannelType(outChannelPtr)))
	    (Tcl_GetChannelInstanceData(outChannelPtr), interp, nameStr, valStr);
	if (ret1 == TCL_OK && ret2 != TCL_OK) {
	    Tcl_DStringResult(interp, &dString);
	}
	Tcl_DStringFree(&dString);
    }

    if (ret1 == TCL_OK && ret2 == TCL_OK) {
	return TCL_OK;
    }
    return TCL_ERROR;
}


/*
 *----------------------------------------------------------------------
 *
 * ExpPairGetOption --
 *
 *	Queries ExpPair channel for the current value of
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
ExpPairGetOption (
    ClientData instanceData,
    Tcl_Interp *interp,
    CONST char *nameStr,		/* (in) Name of option to retrieve */
    Tcl_DString *dsPtr)			/* (in) String to place value */
{
    ExpPairState *ssPtr = (ExpPairState *) instanceData;
    Tcl_Channel inChannelPtr = ssPtr->inChannelPtr;
    Tcl_Channel outChannelPtr = ssPtr->outChannelPtr;
    int ret, len;
    int valid = 0;  /* flag if valid option parsed */

    len = nameStr ? strlen(nameStr) : 0;

    if (len == 0) {
	Tcl_DStringAppendElement(dsPtr, "-blockingpropagate");
    }
    if ((len == 0) ||
	    ((len > 2) && (strcmp(nameStr, "-blockingpropagate") == 0))) {
        Tcl_DStringAppendElement(dsPtr,
		(ssPtr->blockingPropagate) ? "0" : "1");
	valid = 1;
    }

    if (inChannelPtr && Tcl_ChannelGetOptionProc(Tcl_GetChannelType(inChannelPtr))) {
	ret = (Tcl_ChannelGetOptionProc(Tcl_GetChannelType(inChannelPtr)))
	    (Tcl_GetChannelInstanceData(inChannelPtr), interp, nameStr, dsPtr);
	valid = (ret == TCL_OK);
    }
    if (outChannelPtr && Tcl_ChannelGetOptionProc(Tcl_GetChannelType(outChannelPtr))) {
	return (Tcl_ChannelGetOptionProc(Tcl_GetChannelType(outChannelPtr)))
	    (Tcl_GetChannelInstanceData(outChannelPtr), interp, nameStr, dsPtr);
    }

    return (valid ? TCL_OK : TCL_ERROR);
}

/*
 *----------------------------------------------------------------------
 *
 * ExpPairGetHandle --
 *
 *	Get the Tcl_File for the appropriate direction in from the
 *	Tcl_Channel.
 *
 * Results:
 *	NULL because ExpPair ids are handled through other channel
 *	types.
 *
 * Side Effects
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
ExpPairGetHandle(instanceData, direction, handlePtr)
    ClientData instanceData;
    int direction;
    ClientData *handlePtr;
{
    Tcl_Channel inChannelPtr = ((ExpPairState *)instanceData)->inChannelPtr;
    Tcl_Channel outChannelPtr = ((ExpPairState *)instanceData)->outChannelPtr;

    if (direction == TCL_READABLE) {
	if (inChannelPtr && Tcl_ChannelGetHandleProc(Tcl_GetChannelType(inChannelPtr))) {
	    return (Tcl_ChannelGetHandleProc(Tcl_GetChannelType(inChannelPtr)))
		(Tcl_GetChannelInstanceData(inChannelPtr), direction, handlePtr);
	} else {
	    *handlePtr = NULL;
	    return TCL_ERROR;
	}
    } else {
	if (outChannelPtr && Tcl_ChannelGetHandleProc(Tcl_GetChannelType(outChannelPtr))) {
	    return (Tcl_ChannelGetHandleProc(Tcl_GetChannelType(outChannelPtr)))
		(Tcl_GetChannelInstanceData(outChannelPtr), direction, handlePtr);
	} else {
	    *handlePtr = NULL;
	    return TCL_ERROR;
	}
    }
}

/*
 *----------------------------------------------------------------------
 *
 * ExpPairWatch --
 *
 *	Sets up event handling on a expect console Tcl_Channel using
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
ExpPairWatch(instanceData, mask)
    ClientData instanceData;
    int mask;
{
    ExpPairState *ssPtr = (ExpPairState *) instanceData;
    Tcl_Channel inChannelPtr = ssPtr->inChannelPtr;
    Tcl_Channel outChannelPtr = ssPtr->outChannelPtr;
    int old_mask = ssPtr->watchMask;

    if (mask & TCL_READABLE) {
	if (inChannelPtr && Tcl_ChannelWatchProc(Tcl_GetChannelType(inChannelPtr))) {
	    (Tcl_ChannelWatchProc(Tcl_GetChannelType(inChannelPtr)))
		(Tcl_GetChannelInstanceData(inChannelPtr), mask & (~TCL_WRITABLE));
	}
	if (! (old_mask & TCL_READABLE)) {
	    Tcl_CreateChannelHandler(inChannelPtr, TCL_READABLE,
		ExpPairReadable, instanceData);
	}
    } else if (old_mask & TCL_READABLE) {
	Tcl_DeleteChannelHandler(inChannelPtr, ExpPairReadable, instanceData);
    }
    if (mask & TCL_WRITABLE) {
	if (outChannelPtr && Tcl_ChannelWatchProc(Tcl_GetChannelType(outChannelPtr))) {
	    (Tcl_ChannelWatchProc(Tcl_GetChannelType(outChannelPtr)))
		(Tcl_GetChannelInstanceData(outChannelPtr), mask & (~TCL_READABLE));
	}
	if (! (old_mask & TCL_WRITABLE)) {
	    Tcl_CreateChannelHandler(outChannelPtr, TCL_WRITABLE,
		ExpPairWritable, instanceData);
	}
    } else if (old_mask & TCL_WRITABLE) {
	Tcl_DeleteChannelHandler(outChannelPtr, ExpPairWritable, instanceData);
    }
    ssPtr->watchMask = mask;
    return;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpPairReadable --
 *
 *	Callback when an event occurs in the input channel.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	An event is generated for this channel.
 *
 *----------------------------------------------------------------------
 */

static void
ExpPairReadable(ClientData instanceData, int mask)
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    ExpPairState *ssPtr = (ExpPairState *) instanceData;
    Tcl_Channel channel = ssPtr->thisChannelPtr;

    if (! tsdPtr->initialized) {
	expChanInit();
    }
    Tcl_NotifyChannel(channel, mask);
}

/*
 *----------------------------------------------------------------------
 *
 * ExpPairWritable --
 *
 *	Callback when an event occurs in the output channel.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	An event is generated for this channel.
 *
 *----------------------------------------------------------------------
 */

static void
ExpPairWritable(ClientData instanceData, int mask)
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    ExpPairState *ssPtr = (ExpPairState *) instanceData;
    Tcl_Channel channel = ssPtr->thisChannelPtr;

    if (! tsdPtr->initialized) {
	expChanInit();
    }
    Tcl_NotifyChannel(channel, mask);
}
