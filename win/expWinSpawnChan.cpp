/* ----------------------------------------------------------------------------
 * expWinSpawnChan.cpp --
 *
 *	Implements the Windows specific 'spawn' channel.
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
 * RCS: @(#) $Id: expWinSpawnChan.c,v 1.1.2.1.2.4 2003/08/26 20:46:53 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinPort.h"
#include "expWinConsoleDebugger.hpp"

#ifndef INVALID_SET_FILE_POINTER
#   define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif

/* prototypes */
static Tcl_DriverCloseProc	ExpWinSpawnClose;
static Tcl_DriverInputProc	ExpWinSpawnInput;
static Tcl_DriverOutputProc	ExpWinSpawnOutput;
static Tcl_DriverSetOptionProc	ExpWinSpawnSetOption;
static Tcl_DriverGetOptionProc	ExpWinSpawnGetOption;
static Tcl_DriverWatchProc	ExpWinSpawnWatch;
static Tcl_DriverBlockModeProc	ExpWinSpawnBlock;
#ifdef TCL_CHANNEL_VERSION_4
static Tcl_DriverThreadActionProc	ExpWinSpawnThreadAction;
#endif

Tcl_ChannelType ExpWinSpawnChannelType = {
    "spawn",		    /* Type name. */
#ifdef TCL_CHANNEL_VERSION_4
    TCL_CHANNEL_VERSION_4,  /* v4 channel */
#else
    TCL_CHANNEL_VERSION_2,  /* v2 channel */
#endif
    ExpWinSpawnClose,	    /* Close proc. */
    ExpWinSpawnInput,	    /* Input proc. */
    ExpWinSpawnOutput,	    /* Output proc. */
    NULL,		    /* Seek proc. N/A */
    ExpWinSpawnSetOption,   /* Set option proc. */
    ExpWinSpawnGetOption,   /* Get option proc. */
    ExpWinSpawnWatch,	    /* Set up notifier to watch the channel. */
    NULL,		    /* Get an OS handle from channel. N/A */
    NULL,		    /* close2proc N/A */
    ExpWinSpawnBlock,	    /* Set blocking or non-blocking mode.*/
    NULL,		    /* flush proc. N/A */
    NULL,		    /* handler proc. N/A */
#ifdef TCL_CHANNEL_VERSION_4
    NULL,		    /* Wide Seek proc. */
    ExpWinSpawnThreadAction /* Thread action cut/splice */
#endif
};

static Tcl_EventSetupProc	ExpWinSpawnEventSetupProc;
static Tcl_EventCheckProc	ExpWinSpawnEventCheckProc;
static Tcl_EventProc		ExpWinSpawnEventProc;
static Tcl_EventDeleteProc	ExpWinSpawnRemovePendingEvents;
static Tcl_EventDeleteProc	ExpWinSpawnRemoveAllPendingEvents;
static Tcl_ExitProc		ExpWinSpawnThreadExitHandler;
static Tcl_ExitProc		ExpWinSpawnExit;

Tcl_ChannelTypeVersion		ExpSpawnGetTclMaxChannelVer(
				    Tcl_ChannelTypeVersion designMax);

/* forward reference it. */
class ExpWinSpawnClass;

typedef CMclLinkedList<ExpWinSpawnClass *> READYQ;

struct ExpWinSpawnEvent {
    Tcl_Event header;		/* Information that is standard for
				 * all events. */
    ExpWinSpawnClass *instance;
};

struct ThreadSpecificData {
    READYQ *readyQ;
    CMclEvent *needAwake;
};

ThreadSpecificData *ExpWinSpawnInit(void);


/* file scope globals */
static int spawnID = 0;
static int initialized = 0;
static Tcl_ThreadDataKey dataKey;
TCL_DECLARE_MUTEX(spawnMutex)


/* our instanceData for our 'spawn' channel is an instance of this class. */
class ExpWinSpawnClass : public ConsoleDebuggerCallback
{
public:
    LONG eventInQ;
    int watchMask;
    int cutMask;
    int blocking;
    Tcl_Channel channel;

    ExpWinSpawnClass(TCHAR *cmdline, TCHAR *env, TCHAR *dir,
	    int show, const char *injPath, ThreadSpecificData *_tsd)
	: channel(0L), watchMask(0), blocking(0), eventInQ(0), tsd(_tsd)
    {
	CMclEvent readyUp;

	threadId = Tcl_GetCurrentThread();
	debugger = new ConsoleDebugger(cmdline, env, dir, show, injPath, outLL,
		errLL, readyUp, *this);
	debuggerThread = new CMclThread(debugger);
	if (getenv("EXPECT_DEBUGGER_HIGH_PRIORITY") != NULL) {
	    debuggerThread->SetPriority(THREAD_PRIORITY_ABOVE_NORMAL);
	}
	readyUp.Wait(INFINITE);
	status = debugger->Status();
    }
    ~ExpWinSpawnClass()
    {
	delete debuggerThread, debugger;
    }
    int Read (char *bufPtr, int toRead, int *errorCodePtr);
    int Write (const char *bufPtr, int toWrite, int *errorCodePtr);
    int Close (Tcl_Interp *interp, int exiting);
    int SetOption (Tcl_Interp *interp, const char *nameStr, const char *valStr);
    int GetOption (Tcl_Interp *interp, const char *nameStr, Tcl_DString *dsPtr);
    void Watch (int mask);
    int Block (int mode);
    void Cut (void);
    void Splice (void);

    inline int Readable (void) {
	return (outLL.Empty() ? 0 : 1);
    }
    inline DWORD Pid (void) {
	return debugger->Pid();
    }
    inline HANDLE Handle (void) {
	return debugger->Handle();
    }
    inline DWORD Status (void) {
	return status;
    }

    // see GenerateConsoleCtrlEvent()
    inline BOOL SendKill (DWORD dwCtrlEvent) {
	IPCMsg msg;
	DWORD err;

	msg.type = CTRL_EVENT;
	msg.event = dwCtrlEvent;
	err = debugger->Write(&msg);
	if (err != NO_ERROR) {
	    SetLastError(err);
	    return FALSE;
	} else {
	    return TRUE;
	}
    }

private:
    virtual void AlertReadable (void)
    {
	DWORD waitStatus;
	ExpWinSpawnClass *us = this;

	/*
	 * Only alert if there is an interest for readable.
	 * Add an event if one is not currently in the ready queue for us,
	 * then alert the thread.
	 */
	if (watchMask & TCL_READABLE) {
	    __try {
		if (!InterlockedExchange(&eventInQ, 1)) {
		    tsd->readyQ->PutOnTailOfList(us);
		}
		waitStatus = tsd->needAwake->Wait(0);
		/* if the notifier is waiting, zap it. */
		if (CMclWaitSucceeded(waitStatus, 1)) {
		    Tcl_ThreadAlert(threadId);
		}
	    } __except (EXCEPTION_EXECUTE_HANDLER) {}
	}
    }
    CMclLinkedList<Message *> outLL;
    CMclLinkedList<Message *> errLL;
    ConsoleDebugger *debugger;
    CMclThread *debuggerThread;
    ThreadSpecificData *tsd;
    Tcl_ThreadId threadId;
    DWORD status;
};


int
ExpWinSpawnClass::Read (char *bufPtr, int toRead, int *errorCodePtr)
{
    int bytesRead = 0, done = 0;
    Message *msg;
    char *bufPos = bufPtr;
    DWORD timeout;
    ExpWinSpawnClass *us = this;

    *errorCodePtr = 0;

    /* Pending events are being serviced, indicate clean Q. */
    InterlockedExchange(&eventInQ, 0);

    timeout = (blocking ? INFINITE : 0);

    if (!outLL.Empty() || blocking) {
	while (outLL.GetFromHeadOfList(msg, timeout)) {
	    switch (msg->type) {
	    case Message::TYPE_NORMAL:
#ifdef EXP_DEBUG
		expDiagLog("Read %d, already %d + %d -> ",
			   toRead, bytesRead, msg->length);
#endif
		if (bytesRead + (int) msg->length > toRead) {
		    /*
		     * Too large to fit - consume what we can to fill the
		     * buffer and push the rest back on for later.
		     */
		    int avail = toRead - bytesRead;
		    PBYTE rest;

		    memcpy(bufPos, msg->bytes, avail);
		    bytesRead += avail;
		    bufPos    += avail;
		    rest = new BYTE [msg->length - avail];
		    memcpy(rest, msg->bytes + avail, msg->length - avail);
		    delete [] msg->bytes;
		    msg->bytes   = rest;
		    msg->length -= avail;

		    outLL.PutOnHeadOfList(msg);
		    /* Events remain in Q - flop bit on */
		    InterlockedExchange(&eventInQ, 1);
		    tsd->readyQ->PutOnTailOfList(us);
		    done = 1;
		} else {
		    memcpy(bufPos, msg->bytes, msg->length);
		    bytesRead += msg->length;
		    bufPos    += msg->length;
		    delete msg;
		}
#ifdef EXP_DEBUG
		expDiagLog("NOW %d of %d\n", bytesRead, toRead);
#endif
		break;

	    case Message::TYPE_SLAVEDONE:
#ifdef EXP_DEBUG
		expDiagLog("SlaveDone %d of %d\n",
			   toRead, bytesRead);
#endif
		if (bytesRead) {
		    /* repost this for later. */
		    outLL.PutOnHeadOfList(msg);
		    /* Events remain in Q - flop bit on */
		    InterlockedExchange(&eventInQ, 1);
		    tsd->readyQ->PutOnTailOfList(us);
		} else {
		    bytesRead = 0;
		    delete msg;
		}
		done = 1;
		break;

	    case Message::TYPE_ERROR:
		/*
		 * XXX: What to do with this case?
		 */
		expErrorLog("Read Error: (%d) '%s'\n",
			    msg->status, msg->bytes);
		break;
	    }
	    if (done || blocking) break;
	}
    } else {
	/* If there's nothing to read, return EWOULDBLOCK. */
#ifdef EXP_DEBUG
	expDiagLog("Read %d read %d WOULDBLOCK\n",
		   toRead, bytesRead);
#endif
	*errorCodePtr = EWOULDBLOCK;
	return -1;
    }

    return bytesRead;
}

int
ExpWinSpawnClass::Write (const char *bufPtr, int toWrite, int *errorCodePtr)
{
    DWORD err;
    int bytesWritten;

    *errorCodePtr = 0;
    bytesWritten = debugger->Write(bufPtr, toWrite, &err);
    if (err != NO_ERROR) {
	TclWinConvertError(err);
	*errorCodePtr = Tcl_GetErrno();
	bytesWritten = -1;
    }
    return bytesWritten;
}

int
ExpWinSpawnClass::Close (Tcl_Interp *interp, int exiting)
{
    Message *msg;
    int sig = 0;
    ExpWinSpawnClass *us = this;
    DWORD dwWait;

    channel = 0L;

    /* Only do these when the IO subsystem is alive. */
    if (!exiting && ExpWinSpawnChannelType.version == TCL_CHANNEL_VERSION_2) {
	Cut();
    }

    /* We must block on the application. */
    dwWait = debuggerThread->Wait(200);
    if (!CMclWaitSucceeded(dwWait, 1)) {

	// Give it a friendly "go down" command.
	SendKill(CTRL_BREAK_EVENT);

	// wait again..
	dwWait = debuggerThread->Wait(40);

	if (!CMclWaitSucceeded(dwWait, 1)) {
	    // timed-out..  kill it hard!
	    TerminateProcess(debugger->Handle(), CONTROL_C_EXIT);
	    // wait again..
	    debuggerThread->Wait(INFINITE);
	}
    }

    /* did we die because of a major failure of the debugger? */
    if (interp && !errLL.Empty()) {
	Tcl_Obj *resultObj = Tcl_GetObjResult(interp);
	while (errLL.GetFromHeadOfList(msg, 0)) {
	    if (msg->status != NO_ERROR) {
		SetLastError(msg->status);
		if (msg->status != ERROR_EXP_WIN32_BACKTRACE) {
		    Tcl_AppendResult(interp, ExpWinError(interp, msg->bytes, 0L), "\n", 0L);
		} else {
		    ExpWinError(interp, 0L, 0L);
		    Tcl_AppendToObj(resultObj, (char *) msg->bytes, msg->length);
		    Tcl_AppendToObj(resultObj, "\n", 1);
		}
	    } else {
		Tcl_AppendToObj(resultObj, (char *) msg->bytes, msg->length);
	    }
	    delete msg;
	}
	Tcl_AppendResult(interp, "child process crashed.", 0L);
	return TCL_ERROR;
    }

    return TCL_OK;
}

int
ExpWinSpawnClass::SetOption (Tcl_Interp *interp,
			     const char *optionName,
			     const char *value)
{
    size_t len, vlen;
    DWORD dwMode;
    int mode;
    HANDLE console = debugger->Console();

    /* 
     * Parse options
     */
    len = strlen(optionName);
    vlen = strlen(value);

    /* 
     * Option -mode baud,parity,databits,stopbits
     */
    if ((len > 2) && (strncmp(optionName, "-mode", len) == 0)) {
        if (!GetConsoleMode(console, &dwMode)) {
            if (interp) {
                Tcl_AppendResult(interp, 
				 "can't get console mode", (char *) NULL);
            }
            return TCL_ERROR;
        }
        if (Tcl_GetInt(interp, value, &mode) != TCL_OK) {
            return TCL_ERROR;
        }
	dwMode = mode;
        if (!SetConsoleMode(console, dwMode)) {
            if (interp) {
                Tcl_AppendResult(interp, 
				 "can't set console mode", (char *) NULL);
            }
            return TCL_ERROR;
        }
        return TCL_OK;
    }

    return Tcl_BadChannelOption(interp, optionName, "mode");
}

int
ExpWinSpawnClass::GetOption (Tcl_Interp *interp,
			     const char *optionName,
			     Tcl_DString *dsPtr)
{
    ExpWinSpawnClass *us = this;
    size_t len;
    int valid = 0;  /* flag if valid option parsed */
    char buf[2 * TCL_INTEGER_SPACE + 16];

    if (optionName == NULL) {
        len = 0;
    } else {
        len = strlen(optionName);
    }

    /*
     * get option -mode
     */

    HANDLE console = debugger->Console();
    if ((len == 0) ||
        ((len > 2) && (strncmp(optionName, "-mode", len) == 0))) {
        DWORD dw;

	if (len == 0) {
	    Tcl_DStringAppendElement(dsPtr, "-mode");
	}
        if (!GetConsoleMode(console, &dw)) {
            if (interp) {
                Tcl_AppendResult(interp, 
				 "can't get console mode", (char *) NULL);
            }
            return TCL_ERROR;
        }
	valid = 1;
        wsprintfA(buf, "%d", dw);
        Tcl_DStringAppendElement(dsPtr, buf);
    }
#if 0
    if ((len == 0) ||
        ((len > 2) && (strncmp(optionName, "-consolesize", len) == 0))) {
	HANDLE sbuffer = debugger->ConsoleScreenBuffer();
        CONSOLE_SCREEN_BUFFER_INFO sbInfo;

	if (len == 0) {
	    Tcl_DStringAppendElement(dsPtr, "-consolesize");
	    Tcl_DStringStartSublist(dsPtr);
	}
        if (!GetConsoleScreenBufferInfo(console, &sbInfo)) {
            if (interp) {
                Tcl_AppendResult(interp,
				 "can't get console screen buffer info",
				 (char *) NULL);
            }
            return TCL_ERROR;
        }
	valid = 1;
        wsprintfA(buf, "%d", sbInfo.dwSize.X);
	Tcl_DStringAppendElement(dsPtr, buf);
        wsprintfA(buf, "%d", sbInfo.dwSize.Y);
	Tcl_DStringAppendElement(dsPtr, buf);
	if (len == 0) {
	    Tcl_DStringEndSublist(dsPtr);
	}
    }

    if ((len == 0) ||
        ((len > 2) && (strncmp(optionName, "-consoleposition", len) == 0))) {
	HANDLE sbuffer = debugger->ConsoleScreenBuffer();
        CONSOLE_SCREEN_BUFFER_INFO sbInfo;

	if (len == 0) {
	    Tcl_DStringAppendElement(dsPtr, "-consoleposition");
	    Tcl_DStringStartSublist(dsPtr);
	}
        if (!GetConsoleScreenBufferInfo(console, &sbInfo)) {
            if (interp) {
                Tcl_AppendResult(interp,
				 "can't get console screen buffer info",
				 (char *) NULL);
            }
            return TCL_ERROR;
        }
	valid = 1;
        wsprintfA(buf, "%d", sbInfo.dwCursorPosition.X);
	Tcl_DStringAppendElement(dsPtr, buf);
        wsprintfA(buf, "%d", sbInfo.dwCursorPosition.Y);
	Tcl_DStringAppendElement(dsPtr, buf);
	if (len == 0) {
	    Tcl_DStringEndSublist(dsPtr);
	}
    }

    if (!valid) {
	return Tcl_BadChannelOption(interp, optionName, "mode consolesize consoleposition");
    }
#else
    if (!valid) {
	return Tcl_BadChannelOption(interp, optionName, "mode");
    }
#endif
    return TCL_OK;
}

void
ExpWinSpawnClass::Watch (int mask)
{
    ExpWinSpawnClass *us = this;

    /*
     * We are always writable!  When asked, always respond with an event.
     */
    if ((mask & TCL_WRITABLE) || ((mask & TCL_READABLE) && Readable())) {
	__try {
	    tsd->readyQ->PutOnTailOfList(us);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
	}
    }
    watchMask = mask;
}

int
ExpWinSpawnClass::Block (int mode)
{
    if (mode == TCL_MODE_BLOCKING) {
	blocking = 1;
    } else {
	blocking = 0;
    }
    return TCL_OK;
}

void
ExpWinSpawnClass::Cut ()
{
    ExpWinSpawnClass *us = this;

    /*
     * Walk tsdPtr->readyQ, and remove references to ourselves.
     */
    __try {
	tsd->readyQ->RemoveIf(us);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }

    /*
     * Remove any references to us on the event loop.
     */
    Tcl_DeleteEvents(ExpWinSpawnRemovePendingEvents, this);

    /*
     * Make sure AlertReadable() won't announce during the transition.
     */
    watchMask = 0;
    tsd = NULL;
}

void
ExpWinSpawnClass::Splice ()
{
    tsd = ExpWinSpawnInit();
}

Tcl_ChannelTypeVersion
ExpSpawnGetTclMaxChannelVer (Tcl_ChannelTypeVersion designMax)
{
    Tcl_ChannelType fake;

    if (designMax == (Tcl_ChannelTypeVersion) 0x1) {
	return designMax;
    }
    
    /* Tcl_ChannelVersion only touches the ->version field. */
    fake.version = designMax;

    while (Tcl_ChannelVersion(&fake) == TCL_CHANNEL_VERSION_1) {
	fake.version = (Tcl_ChannelTypeVersion)((int)fake.version - 1);
	if (fake.version == (Tcl_ChannelTypeVersion) 0x1) break;
    }
    return fake.version;
}

ThreadSpecificData *
ExpWinSpawnInit (void)
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);

    Tcl_MutexLock(&spawnMutex);
    if (!initialized) {
	initialized = 1;
	/*
	 * Allow the ability to back load. ie. Load into a core
	 * less than what we built against.
	 */
	ExpWinSpawnChannelType.version =
		ExpSpawnGetTclMaxChannelVer(ExpWinSpawnChannelType.version);
    }
    Tcl_MutexUnlock(&spawnMutex);

    /* per thread init */
    if (tsdPtr->readyQ == 0L) {
	Tcl_CreateEventSource(ExpWinSpawnEventSetupProc, ExpWinSpawnEventCheckProc, NULL);
	Tcl_CreateThreadExitHandler(ExpWinSpawnThreadExitHandler, tsdPtr);
	tsdPtr->readyQ = new READYQ;
	/* manual reset, initial off state. */
	tsdPtr->needAwake = new CMclEvent(TRUE);
    }
    return tsdPtr;
}

void
ExpWinSpawnThreadExitHandler (ClientData clientData)
{
    ThreadSpecificData *tsdPtr = (ThreadSpecificData *) clientData;

    Tcl_DeleteEventSource(ExpWinSpawnEventSetupProc, ExpWinSpawnEventCheckProc, NULL);
    delete tsdPtr->readyQ, tsdPtr->needAwake;
    tsdPtr->readyQ = NULL;
    tsdPtr->needAwake = NULL;
}


void
ExpWinSpawnExit (ClientData clientData)
{
    /* We aren't registered, so we can close it directly. */
    Tcl_Close(NULL, (Tcl_Channel) clientData);
}


/*
 *-----------------------------------------------------------------------
 * ExpWinSpawnEventSetupProc --
 *
 *  Happens before the event loop is to wait in the notifier.
 *
 *-----------------------------------------------------------------------
 */
static void
ExpWinSpawnEventSetupProc (
    ClientData clientData,
    int flags)
{
    ThreadSpecificData *tsdPtr;
    Tcl_Time blockTime = {0, 0};

    if (!(flags & TCL_FILE_EVENTS)) {
	return;
    }

    tsdPtr = ExpWinSpawnInit();

    /*
     * If any ready events exist now, avoid waiting in the notifier.
     * This function call is very inexpensive.
     */

    if (!tsdPtr->readyQ->Empty()) {
	Tcl_SetMaxBlockTime(&blockTime);
    } else {
	/* allow the notifier to be awoken by us. */
	tsdPtr->needAwake->Set();
    }
}

/*
 *-----------------------------------------------------------------------
 * ExpWinSpawnEventCheckProc --
 *
 *  Happens after the notifier has waited.
 *
 *-----------------------------------------------------------------------
 */
static void
ExpWinSpawnEventCheckProc (
    ClientData clientData,
    int flags)
{
    ThreadSpecificData *tsdPtr;
    ExpWinSpawnClass *instance;
    ExpWinSpawnEvent *evPtr;

    if (!(flags & TCL_FILE_EVENTS)) {
	return;
    }

    tsdPtr = ExpWinSpawnInit();

    /* Disallow awaking the notifier as we aren't waiting for it anymore. */
    tsdPtr->needAwake->Reset();

    /* Do we have any jobs to queue? */
    while (tsdPtr->readyQ->GetFromHeadOfList(instance, 0)) {
	evPtr = (ExpWinSpawnEvent *) ckalloc(sizeof(ExpWinSpawnEvent));
	evPtr->header.proc = ExpWinSpawnEventProc;
	evPtr->instance = instance;
	Tcl_QueueEvent((Tcl_Event *) evPtr, TCL_QUEUE_TAIL);
    }
}

/*
 *-----------------------------------------------------------------------
 * ExpWinSpawnEventProc --
 *
 *  Tcl's event loop is now servicing this.
 *
 *-----------------------------------------------------------------------
 */
static int
ExpWinSpawnEventProc (
    Tcl_Event *evPtr,		/* Event to service. */
    int flags)			/* Flags that indicate what events to
				 * handle, such as TCL_FILE_EVENTS. */
{
    ExpWinSpawnClass *instance = ((ExpWinSpawnEvent *)evPtr)->instance;
    int readyMask = 0;

    if (!(flags & TCL_FILE_EVENTS)) {
	return 0;
    }

    if ((instance->watchMask & TCL_READABLE) && instance->Readable()) {
	readyMask |= TCL_READABLE;
    }

    /* we're always writable */
    if (instance->watchMask & TCL_WRITABLE) {
	readyMask |= TCL_WRITABLE;
    }

    if (readyMask) Tcl_NotifyChannel(instance->channel, readyMask);
    return 1;
}

static int
ExpWinSpawnRemovePendingEvents (Tcl_Event *ev, ClientData cData)
{
    ExpWinSpawnClass *instance = (ExpWinSpawnClass *) cData;
    ExpWinSpawnEvent *sev = (ExpWinSpawnEvent *) ev;

    if (ev->proc == ExpWinSpawnEventProc && sev->instance == instance) {
	return 1;
    } else {
	return 0;
    }
}

static int
ExpWinSpawnRemoveAllPendingEvents (Tcl_Event *ev, ClientData cData)
{
    if (ev->proc == ExpWinSpawnEventProc) {
	return 1;
    } else {
	return 0;
    }
}


/*
 *----------------------------------------------------------------------
 *
 * Exp_CreateSpawnChannel --
 *
 *	Creates a new 'spawn' channel for the windows flavor.
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
    unsigned long *pid,
    Tcl_Pid *theUglyHandleHackJob)
{
    ThreadSpecificData *tsdPtr;
    char name[150];
    ExpWinSpawnClass *instance;
    int i, listLen;
    Tcl_Obj **elemArray;
    Tcl_DString env, dir, temp, cmdLine, exeFullPathUTF;
    TCHAR *envBlock, *startDir;
    DWORD applType;
    Tcl_Channel newChan = NULL;
    const char *injPath;

    Tcl_DStringInit(&temp);
    Tcl_DStringInit(&env);
    Tcl_DStringInit(&dir);
    Tcl_DStringInit(&cmdLine);
    Tcl_DStringInit(&exeFullPathUTF);

    *pid = 0L;
    *theUglyHandleHackJob = 0L;
    tsdPtr = ExpWinSpawnInit();

    /* prepare the environment */
    if (opts->env != NULL) {
	if (Tcl_ListObjGetElements(interp, opts->env, &listLen,
		&elemArray) != TCL_OK || !listLen) {
	    goto out;
	}
	/*
	 * Format is a null seperated, double null terminated
	 * list. ex. "key=val\0key=val\0"
	 */

	for (i = 0; i < listLen; i += 2) {
	    int len;
	    char *data;

	    data = Tcl_GetStringFromObj(elemArray[i], &len);
	    Tcl_WinUtfToTChar(data, len, &temp);
	    Tcl_DStringAppend(&env, Tcl_DStringValue(&temp),
		    Tcl_DStringLength(&temp));
	    /* Convert it to unicode the easy way */
	    Tcl_DStringAppend(&env, "=",
		    (expWinProcs->useWide?2:1));
	    data = Tcl_GetStringFromObj(elemArray[i+1], &len);
	    Tcl_WinUtfToTChar(data, len, &temp);
	    Tcl_DStringAppend(&env, Tcl_DStringValue(&temp),
		    Tcl_DStringLength(&temp));
	    /* Convert it to unicode the easy way */
	    Tcl_DStringAppend(&env, "\0",
		    (expWinProcs->useWide?2:1));
	}
	Tcl_DStringFree(&temp);
	/* Convert it to unicode the easy way */
	if (expWinProcs->useWide)
		Tcl_DStringAppend(&env, "\0", 1);
	envBlock = Tcl_DStringValue(&env);
    } else {
	/*
	 * If a custom environment block isn't being used, set the block pointer
	 * to NULL to indicate to CreateProcess() that we want the current
	 * parent's block to be inherited.  As opposed to a pointer to an empty
	 * string which I think means a blank environment.
	 */

	envBlock = 0L;
    }

    /* Prepare the starting directory */
    if (opts->dir != NULL) {
	int len;
	char *data;

	data = Tcl_GetStringFromObj(opts->dir, &len);
	Tcl_WinUtfToTChar(data, len, &dir);
	Tcl_DecrRefCount(opts->dir);
	startDir = Tcl_DStringValue(&dir);
    } else {
	startDir = 0L;
    }

    /*
     * Read the PE header to see if this application is ok for us to run.
     */

    applType = Exp_WinApplicationType(Tcl_GetString(objv[0]),
	    &exeFullPathUTF);

    if (applType == EXP_WINAPPL_NONE) {
	Tcl_SetObjResult(interp, Tcl_NewStringObj(
		ExpWinError(interp, Tcl_DStringValue(&exeFullPathUTF), 0L), -1));
	goto out;
    } else if (applType == EXP_WINAPPL_BATCH) {
	/* set our own error code for this event. */
	SetLastError(ERROR_EXP_WIN32_CANT_SPAWN_BATCH);
	Tcl_SetObjResult(interp, Tcl_NewStringObj(
		ExpWinError(interp, Tcl_DStringValue(&exeFullPathUTF), 0L), -1));
	goto out;
    } else if (applType == EXP_WINAPPL_DOS16) {
	/* set our own error code for this event. */
	SetLastError(ERROR_EXP_WIN32_CANT_SPAWN_DOS16);
	Tcl_SetObjResult(interp, Tcl_NewStringObj(
		ExpWinError(interp, Tcl_DStringValue(&exeFullPathUTF), 0L), -1));
	goto out;
    } else if (!(applType == EXP_WINAPPL_WIN32CUI ||
	    applType == EXP_WINAPPL_WIN64CUI)) {
	/* set our own error code for this event. */
	SetLastError(ERROR_EXP_WIN32_CANT_SPAWN_NONCUI);
	Tcl_SetObjResult(interp, Tcl_NewStringObj(
		ExpWinError(interp, Tcl_DStringValue(&exeFullPathUTF), 0L), -1));
	goto out;
    }

    /*
     * Turn the array of arguments into a string according to the
     * parse_cmdline() quoting rules.
     */

    Exp_WinBuildCommandLine(Tcl_DStringValue(&exeFullPathUTF),
	    objc, objv, &cmdLine);

    /* Where's the injector dll? */
    injPath = Tcl_GetVar(interp, "::exp::injector_path", 0);
    if (!injPath) {
	// Use local if not set.
	injPath = ".";
    }

    /* kick it up! */
    instance = new ExpWinSpawnClass(Tcl_DStringValue(&cmdLine), envBlock,
				    startDir, ExpWinNTDebug() /*show console*/,
				    injPath, tsdPtr);

    if (instance->Status() == NO_ERROR) {
	TclWinAddProcess(instance->Handle(), instance->Pid());
	*pid = instance->Pid();
	*theUglyHandleHackJob = (Tcl_Pid) instance->Handle();
	sprintf(name, "spawn%d", spawnID++);
	newChan = Tcl_CreateChannel(&ExpWinSpawnChannelType, name,
		instance, TCL_READABLE|TCL_WRITABLE);
	instance->channel = newChan;
	Tcl_CreateExitHandler(ExpWinSpawnExit, newChan);
	/*
	 * The communication link doesn't have a name like /dev/pty23
	 * but use the name of the shared memory object for the injector
	 * IPC as I can't think of anything closer that has a name and
	 * represents the communication link between the master (us) and
	 * the slave process.
	 */
	sprintf(name, "ExpectInjector_pid%d", instance->Pid());
	Tcl_SetVar2(interp,"spawn_out","slave,name",name,0);
    } else {
	SetLastError(instance->Status());
	Tcl_SetObjResult(interp, Tcl_NewStringObj(
		ExpWinError(interp, Tcl_DStringValue(&exeFullPathUTF), 0L), -1));
	delete instance;
    }

out:
    Tcl_DStringFree(&cmdLine);
    Tcl_DStringFree(&env);
    Tcl_DStringFree(&dir);
    Tcl_DStringFree(&cmdLine);
    Tcl_DStringFree(&exeFullPathUTF);
    return newChan;
}

int
Exp_WinApplicationType (
    CONST char *originalName,	/* Name of the application to find. (in UTF-8) */
    Tcl_DString *fullName)	/* Buffer space filled with complete path to 
				 * application. (in UTF-8) */
{
    int applType, i, nameLen, nativeNameLen;
    HANDLE hFile;
    TCHAR *rest;
    char *ext;
    DWORD attr, read;
    IMAGE_DOS_HEADER p236;  /* p236, DOS (old-style) executable-file header */
    union {
	BYTE buf[200];
	IMAGE_NT_HEADERS pe;
	IMAGE_OS2_HEADER ne;
	IMAGE_VXD_HEADER le;
    } header;
    Tcl_DString nameBuf, nativeNameBuff, ds;
    const TCHAR *nativeName;
    WCHAR nativeShortPath[MAX_PATH];   /* needed for unicode space */
    static char extensions[][5] = {"", ".com", ".exe", ".bat", ".cmd"};
    int offset64;

    /* Look for the program as an external program.  First try the name
     * as it is, then try adding .com, .exe, and .bat, in that order, to
     * the name, looking for an executable.
     *
     * Using the raw SearchPath() procedure doesn't do quite what is 
     * necessary.  If the name of the executable already contains a '.' 
     * character, it will not try appending the specified extension when
     * searching (in other words, SearchPath will not find the program 
     * "a.b.exe" if the arguments specified "a.b" and ".exe").   
     * So, first look for the file as it is named.  Then manually append 
     * the extensions, looking for a match.  
     */

    applType = EXP_WINAPPL_NONE;
    Tcl_DStringInit(&nameBuf);
    Tcl_DStringInit(&nativeNameBuff);
    Tcl_DStringAppend(&nameBuf, originalName, -1);
    Tcl_DStringAppend(fullName, originalName, -1);
    nameLen = Tcl_DStringLength(&nameBuf);

    for (i = 0; i < (sizeof(extensions) / sizeof(extensions[0])); i++) {
	Tcl_DStringSetLength(&nameBuf, nameLen);
	Tcl_DStringAppend(&nameBuf, extensions[i], -1);
        nativeName = Tcl_WinUtfToTChar(Tcl_DStringValue(&nameBuf), 
		Tcl_DStringLength(&nameBuf), &ds);

	/* Just get the size of the buffer needed, when found. */
	nativeNameLen = expWinProcs->searchPathProc(NULL, nativeName,
		NULL, 0, NULL, &rest);

	if (nativeNameLen == 0) {
	    /* not found. */
	    Tcl_DStringFree(&ds);
	    SetLastError(ERROR_FILE_NOT_FOUND);
	    continue;
	}

	/* Set the buffer needed. */
	Tcl_DStringSetLength(&nativeNameBuff, 
		(expWinProcs->useWide ? nativeNameLen*2 : nativeNameLen));

	expWinProcs->searchPathProc(NULL, nativeName, NULL,
		Tcl_DStringLength(&nativeNameBuff),
		(TCHAR *) Tcl_DStringValue(&nativeNameBuff), &rest);
	Tcl_DStringFree(&ds);

	/*
	 * Ignore matches on directories, keep falling through
	 * when identified as something else.
	 */

	attr = expWinProcs->getFileAttributesProc(
		(TCHAR *) Tcl_DStringValue(&nativeNameBuff));
	if ((attr == -1) || (attr & FILE_ATTRIBUTE_DIRECTORY)) {
	    SetLastError(ERROR_ACCESS_DENIED);
	    continue;
	}
	Tcl_WinTCharToUtf((TCHAR *) Tcl_DStringValue(&nativeNameBuff),
		-1, fullName);

	ext = strrchr(Tcl_DStringValue(fullName), '.');
	if ((ext != NULL) && (stricmp(ext, ".bat") == 0 ||
		stricmp(ext, ".cmd") == 0)) {
	    applType = EXP_WINAPPL_BATCH;
	    break;
	}
	
	hFile = expWinProcs->createFileProc(
		(TCHAR *) Tcl_DStringValue(&nativeNameBuff),
		GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
	    SetLastError(ERROR_READ_FAULT);
	    applType = EXP_WINAPPL_NONE;
	    break;
	}

	p236.e_magic = 0;
	ReadFile(hFile, &p236, sizeof(IMAGE_DOS_HEADER), &read, NULL);
	if (p236.e_magic != IMAGE_DOS_SIGNATURE) {
	    /* 
	     * Doesn't have the magic number for relocatable executables.  If 
	     * filename ends with .com, assume it's a DOS application anyhow.
	     * Note that we didn't make this assumption at first, because some
	     * supposed .com files are really 32-bit executables with all the
	     * magic numbers and everything.  
	     */

	    /*
	     * Additional notes from Ralf Brown's interupt list:
	     *
	     * The COM files are raw binary executables and are a leftover
	     * from the old CP/M machines with 64K RAM.  A COM program can
	     * only have a size of less than one segment (64K), including code
	     * and static data since no fixups for segment relocation or
	     * anything else is included. One method to check for a COM file
	     * is to check if the first byte in the file could be a valid jump
	     * or call opcode, but this is a very weak test since a COM file
	     * is not required to start with a jump or a call. In principle,
	     * a COM file is just loaded at offset 100h in the segment and
	     * then executed.
	     *
	     * OFFSET              Count TYPE   Description
	     * 0000h                   1 byte   ID=0E9h
	     *                                  ID=0EBh
	     */

	    CloseHandle(hFile);
	    if ((ext != NULL) && (stricmp(ext, ".com") == 0)) {
		applType = EXP_WINAPPL_DOS16;
		break;
	    }
	    SetLastError(ERROR_INVALID_EXE_SIGNATURE);
	    applType = EXP_WINAPPL_NONE;
	    break;
	}
	if (p236.e_lfarlc < 0x40 || p236.e_lfanew == 0 /* reserved */) {
	    /* 
	     * Old-style header only.  Can't be more than a DOS16 executable.
	     */

	    CloseHandle(hFile);
	    applType = EXP_WINAPPL_DOS16;
	    break;
	}

	/* 
	 * The LONG at p236.e_lfanew points to the real exe header only
	 * when p236.e_lfarlc is set to 40h (or greater).
	 */
	
	if (SetFilePointer(hFile, p236.e_lfanew, NULL, FILE_BEGIN)
		== INVALID_SET_FILE_POINTER) {
	    /* Bogus PE header pointer. */
	    CloseHandle(hFile);
	    SetLastError(ERROR_BAD_EXE_FORMAT);
	    applType = EXP_WINAPPL_NONE;
	    break;
	}
	ReadFile(hFile, header.buf, 200, &read, NULL);
	CloseHandle(hFile);

	/*
	 * Check the sigs against the following list:
	 *  'PE\0\0'  Win32 (Windows NT and Win32s) portable executable based
	 *	    on Unix COFF.
	 *  'NE'  Windows or OS/2 1.x segmented ("new") executable.
	 *  'LE'  Windows virtual device driver (VxD) linear executable.
	 *  'LX'  Variant of LE used in OS/2 2.x
	 *  'W3'  Windows WIN386.EXE file; a collection of LE files
	 *	    (protected mode windows).
	 *  'W4'  Variant of above.
	 */

	if (header.pe.Signature == IMAGE_NT_SIGNATURE) {
	    if (!(header.pe.FileHeader.Characteristics &
		    IMAGE_FILE_EXECUTABLE_IMAGE)) {
		/* Not an executable. */
		SetLastError(ERROR_BAD_EXE_FORMAT);
		applType = EXP_WINAPPL_NONE;
		break;
	    }

	    if (header.pe.OptionalHeader.Magic ==
		    IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
		/* Win32 executable */
		offset64 = 0;
	    } else if (header.pe.OptionalHeader.Magic ==
		    IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
		/* Win64 executable */
		offset64 = 4;
	    } else {
		/* Unknown magic number */
		SetLastError(ERROR_INVALID_MODULETYPE);
		applType = EXP_WINAPPL_NONE;
		break;
	    }

	    if (header.pe.FileHeader.Characteristics & IMAGE_FILE_DLL) {
		/*
		 * DLLs are executable, but indirectly.  We shouldn't return
		 * EXP_WINAPPL_NONE or the subsystem that its said to run under
		 * as it's not the complete truth, so return a new type and
		 * let the user decide what to do.
		 */

		applType = EXP_WINAPPL_WIN32DLL + offset64;
		break;
	    }

	    switch (header.pe.OptionalHeader.Subsystem) {
		case IMAGE_SUBSYSTEM_WINDOWS_CUI:
		case IMAGE_SUBSYSTEM_OS2_CUI:
		case IMAGE_SUBSYSTEM_POSIX_CUI:
		    /* Runs in the CUI subsystem */
		    applType = EXP_WINAPPL_WIN32CUI + offset64;
		    break;

		case IMAGE_SUBSYSTEM_WINDOWS_GUI:
		case IMAGE_SUBSYSTEM_WINDOWS_CE_GUI:
		    /* Runs in the GUI subsystem */
		    applType = EXP_WINAPPL_WIN32GUI + offset64;
		    break;

		case IMAGE_SUBSYSTEM_UNKNOWN:
		case IMAGE_SUBSYSTEM_NATIVE:
		case IMAGE_SUBSYSTEM_NATIVE_WINDOWS:
		    /* Special Driver */
		    applType = EXP_WINAPPL_WIN32DRV + offset64;
		    break;
	    }

#define IMAGE_NE_FLAG_DRIVER	0x8000
#define IMAGE_NE_EXETYP_OS2	0x1
#define IMAGE_NE_EXETYP_WIN	0x2
#define IMAGE_NE_EXETYP_DOS4X	0x3
#define IMAGE_NE_EXETYP_WIN386	0x4

	} else if (header.ne.ne_magic == IMAGE_OS2_SIGNATURE) {
	    switch (header.ne.ne_exetyp) {
		case IMAGE_NE_EXETYP_OS2:    /* Microsoft/IBM OS/2 1.x */
		    if (header.ne.ne_flags & IMAGE_NE_FLAG_DRIVER) {
			applType = EXP_WINAPPL_OS2DRV;
		    } else {
			applType = EXP_WINAPPL_OS2;
		    }
		    break;

		case IMAGE_NE_EXETYP_WIN:    /* Microsoft Windows */
		case IMAGE_NE_EXETYP_WIN386: /* Same, but Protected mode */
		    if (header.ne.ne_flags & IMAGE_NE_FLAG_DRIVER) {
			applType = EXP_WINAPPL_WIN16DRV;
		    } else {
			applType = EXP_WINAPPL_WIN16;
		    }
		    break;

		case IMAGE_NE_EXETYP_DOS4X:  /* Microsoft MS-DOS 4.x */
		    applType = EXP_WINAPPL_DOS16;
		    break;

		default:
		    /* Unidentified */
		    SetLastError(ERROR_INVALID_MODULETYPE);
		    applType = EXP_WINAPPL_NONE;
	    }
	} else if (
		header.le.e32_magic == IMAGE_OS2_SIGNATURE_LE /* 'LE' */ ||
		header.le.e32_magic == 0x584C /* 'LX' */ ||
		header.le.e32_magic == 0x3357 /* 'W3' */ ||
		header.le.e32_magic == 0x3457 /* 'W4' */
	){
	    /* Virtual device drivers are not executables, per se. */
	    applType = EXP_WINAPPL_WIN16DRV;
	} else {
	    /* The loader will barf anyway, so barf now. */
	    SetLastError(ERROR_INVALID_EXE_SIGNATURE);
	    applType = EXP_WINAPPL_NONE;
	}
	break;
    }

    if (applType == EXP_WINAPPL_DOS16 || applType == EXP_WINAPPL_WIN16 ||
	    applType == EXP_WINAPPL_WIN16DRV || applType == EXP_WINAPPL_OS2 ||
	    applType == EXP_WINAPPL_OS2DRV) {
	/* 
	 * Replace long path name of executable with short path name for
	 * 16-bit applications.  Otherwise the application may not be able
	 * to correctly parse its own command line to separate off the
	 * application name from the arguments.
	 */

	expWinProcs->getShortPathNameProc(
		(TCHAR *) Tcl_DStringValue(&nativeNameBuff),
		(TCHAR *) nativeShortPath, MAX_PATH);
	Tcl_WinTCharToUtf((TCHAR *) nativeShortPath, -1, fullName);
    }
    Tcl_DStringFree(&nativeNameBuff);
    Tcl_DStringFree(&nameBuf);
    return applType;
}

/*    
 *----------------------------------------------------------------------
 *
 * Exp_WinBuildCommandLine --
 *
 *	The command line arguments are stored in linePtr separated
 *	by spaces, in a form that CreateProcess() understands.  Special 
 *	characters in individual arguments from argv[] must be quoted 
 *	when being stored in linePtr.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 * Comment: COPY OF NON_PUBLIC CORE FUNCTION WITH CHANGES!
 *
 *----------------------------------------------------------------------
 */
void
Exp_WinBuildCommandLine (
    CONST char *executable,	/* Full path of executable (including 
				 * extension) UTF-8.  Replacement for argv[0]. */
    int objc,			/* Number of arguments. */
    struct Tcl_Obj * const objv[], /* Array of args in UTF-8. */
    Tcl_DString *linePtr)	/* Initialized Tcl_DString that receives the
				 * command line (TCHAR). */
{
    const char *arg, *start, *special;
    int quote, i;
    Tcl_DString ds;

    Tcl_DStringInit(&ds);

    /*
     * Prime the path.
     */
    
    Tcl_DStringAppend(&ds, Tcl_DStringValue(linePtr), -1);
    
    for (i = 0; i < objc; i++) {
	if (i == 0) {
	    arg = executable;
	} else {
	    arg = Tcl_GetString(objv[i]);
	    Tcl_DStringAppend(&ds, " ", 1);
	}

	quote = 0;
	if (arg[0] == '\0') {
	    quote = 1;
	} else {
	    int count;
	    Tcl_UniChar ch;
	    for (start = arg; *start != '\0'; start += count) {
	        count = Tcl_UtfToUniChar(start, &ch);
		if (Tcl_UniCharIsSpace(ch)) { /* INTL: ISO space. */
		    quote = 1;
		    break;
		}
	    }
	}
	if (quote) {
	    Tcl_DStringAppend(&ds, "\"", 1);
	}
	start = arg;	    
	for (special = arg; ; ) {
	    if ((*special == '\\') && 
		    (special[1] == '\\' || special[1] == '"' || (quote && special[1] == '\0'))) {
		Tcl_DStringAppend(&ds, start, (int) (special - start));
		start = special;
		while (1) {
		    special++;
		    if (*special == '"' || (quote && *special == '\0')) {
			/* 
			 * N backslashes followed a quote -> insert 
			 * N * 2 + 1 backslashes then a quote.
			 */

			Tcl_DStringAppend(&ds, start,
				(int) (special - start));
			break;
		    }
		    if (*special != '\\') {
			break;
		    }
		}
		Tcl_DStringAppend(&ds, start, (int) (special - start));
		start = special;
	    }
	    if (*special == '"') {
		Tcl_DStringAppend(&ds, start, (int) (special - start));
		Tcl_DStringAppend(&ds, "\\\"", 2);
		start = special + 1;
	    }
	    if (*special == '\0') {
		break;
	    }
	    special++;
	}
	Tcl_DStringAppend(&ds, start, (int) (special - start));
	if (quote) {
	    Tcl_DStringAppend(&ds, "\"", 1);
	}
    }
    Tcl_DStringFree(linePtr);
    Tcl_WinUtfToTChar(Tcl_DStringValue(&ds), Tcl_DStringLength(&ds), linePtr);
    Tcl_DStringFree(&ds);
}

/*
 *----------------------------------------------------------------------
 *
 * ExpSpawnClose --
 *
 *	Generic routine to close the expect channel
 *
 * Results:
 *      0 if successful or a POSIX errorcode.
 *      interp's $errorCode updated.
 *    
 * Side Effects:
 *	Channel is deleted.
 *
 *----------------------------------------------------------------------
 */

static int
ExpWinSpawnClose(
    ClientData instanceData,
    Tcl_Interp *interp)
{
    ExpWinSpawnClass *instance = (ExpWinSpawnClass *) instanceData;
    int result;

    Tcl_DeleteExitHandler(ExpWinSpawnExit, instance->channel);
    result = instance->Close(interp, TclInExit());
    delete instance;
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpSpawnInput --
 *
 *	Generic read routine for an expect console
 *
 * Returns:
 *	Amount read or -1 with errorcode in errorPtr.
 *    
 * Side Effects:
 *	Channel buffer is updated. 
 *
 *----------------------------------------------------------------------
 */

static int
ExpWinSpawnInput(
    ClientData instanceData,
    char *bufPtr,		/* (in) Ptr to buffer */
    int toRead,			/* (in) Maximum number of bytes to read. */
    int *errorCodePtr)		/* (out) error code */
{
    ExpWinSpawnClass *instance = (ExpWinSpawnClass *) instanceData;
#ifdef EXP_DEBUG
    int read = instance->Read(bufPtr, toRead, errorCodePtr);
    expDiagLog("ExpWinSpawnInput %d -> error %d got %d (%x)\n", toRead, *errorCodePtr, read, (read == 1) ? *bufPtr : 0);
    return read;
#else
    return instance->Read(bufPtr, toRead, errorCodePtr);
#endif
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
ExpWinSpawnOutput(
    ClientData instanceData,
    CONST char *bufPtr,		/* (in) Ptr to buffer */
    int toWrite,		/* (in) amount to write */
    int *errorPtr)		/* (out) error code */
{
    ExpWinSpawnClass *instance = (ExpWinSpawnClass *) instanceData;
    return instance->Write(bufPtr, toWrite, errorPtr);
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
ExpWinSpawnSetOption(
    ClientData instanceData,
    Tcl_Interp *interp,
    CONST char *nameStr,	/* (in) Name of option */
    CONST char *valStr)		/* (in) New value of option */
{
    ExpWinSpawnClass *instance = (ExpWinSpawnClass *) instanceData;
    return instance->SetOption(interp, nameStr, valStr);
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
ExpWinSpawnGetOption(
    ClientData instanceData,
    Tcl_Interp *interp,
    CONST char *nameStr,	/* (in) Name of option to retrieve */
    Tcl_DString *dsPtr)		/* (in) String to place value */
{
    ExpWinSpawnClass *instance = (ExpWinSpawnClass *) instanceData;
    return instance->GetOption(interp, nameStr, dsPtr);
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
ExpWinSpawnWatch(
    ClientData instanceData,
    int mask)
{
    ExpWinSpawnClass *instance = (ExpWinSpawnClass *) instanceData;
    instance->Watch(mask);
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
 * ----------------------------------------------------------------------
 */

static int
ExpWinSpawnBlock (
    ClientData instanceData,
    int mode)			/* (in) Block or not */
{
    ExpWinSpawnClass *instance = (ExpWinSpawnClass *) instanceData;
    return instance->Block(mode);
}

#ifdef TCL_CHANNEL_VERSION_4
void
ExpWinSpawnThreadAction (
    ClientData instanceData,
    int action)
{
    ExpWinSpawnClass *instance = (ExpWinSpawnClass *) instanceData;

    if (action == TCL_CHANNEL_THREAD_INSERT) {
        instance->Splice();
    } else {
        instance->Cut();
    }
}
#endif

/*
 *----------------------------------------------------------------------
 *
 * Exp_KillCmd --
 *
 *	Implements the 'kill' and 'exp_kill' commands.  There were
 *	not in the Unix version of expect, but since there is no
 *	kill command on NT (well, at least not by default), this
 *	gives us a way to kill a slave.  The argument is the signal
 *	number to send to the subprocess.  On NT, this is interpreted
 *	interpret a bit differently than on Unix.  For a signal of 2,
 *	a CTRL-C is sent to the subprocess.  For a signal of 3, a
 *	CTRL-BREAK is sent to the subprocess.  All other signals cause
 *	the subprocess to be directly terminated.
 *
 * Results:
 *	A standard TCL result
 *
 * Side Effects:
 *	A process may be killed
 *
 *----------------------------------------------------------------------
 */

/*ARGSUSED*/

int
Exp_KillCmd (
    ClientData clientData,
    Tcl_Interp *interp,
    int argc,
    CONST char *argv[])
{
    ExpWinSpawnClass *instance;
    ExpState *esPtr;
    CONST char *chanName = NULL;
    CONST char *argv0 = argv[0];
    int signal = 9;
    DWORD ok;

    argc--; argv++;

    for (;argc>0;argc--,argv++) {
	if (streq(*argv,"-i")) {
	    argc--; argv++;
	    if (!*argv) {
		exp_error(interp,"usage: -i spawn_id");
		return TCL_ERROR;
	    }
	    chanName = *argv;
	} else break;
    }

    if (argc > 0) {
	if (Tcl_GetInt(interp, argv[0], &signal) != TCL_OK) {
	    return TCL_ERROR;
	}
    }

    if (!chanName) {
	esPtr = expStateCurrent(interp, 0, 0, 0);
    } else {
	esPtr = expStateFromChannelName(interp, chanName, 0, 0, 0, argv0);
    }
    if (esPtr == NULL) {
	return(TCL_ERROR);
    }

    if (!esPtr->tclPid) {
	Tcl_AppendResult(interp, "cannot kill ", esPtr->name,
			 ": not a spawned process", NULL);
	return TCL_ERROR;
    }

    instance = (ExpWinSpawnClass *) Tcl_GetChannelInstanceData(esPtr->slave);

    switch (signal) {
    case 2:
	/* Send Ctrl-C */
	ok = instance->SendKill(CTRL_C_EVENT);
	break;
    case 3:
	/* Send Ctrl-Break */
	ok = instance->SendKill(CTRL_BREAK_EVENT);
	break;
    default:
	/* Terminate subprocess with prejudice */
	ok = TerminateProcess((HANDLE)esPtr->tclPid, CONTROL_C_EXIT);
	break;
    }

    if (!ok) {
	Tcl_SetObjResult(interp, Tcl_NewStringObj(
		ExpWinError(interp, 0L), -1));
	return TCL_ERROR;
    }

    return TCL_OK;
}
