/* ----------------------------------------------------------------------------
 * exp_trap.c --
 *
 *	Expect's trap command.
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
 * RCS: @(#) $Id: expTrap.c,v 1.1.2.1.2.3 2002/02/10 12:04:22 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expInt.h"
#include <signal.h>
#if defined(SIGCLD) && !defined(SIGCHLD)
#define SIGCHLD SIGCLD
#endif

#ifdef TCL_DEBUGGER
#include "Dbg.h"
#endif

#define NO_SIG 0

#define SIGNAL_DEFAULT 0
#define SIGNAL_IGNORE  1
#define SIGNAL_ENABLED 2

static struct trap {
    char *action;		/* Tcl command to execute upon sig */
				/* Each is handled by the eval_trap_action */
    int mark;			/* TRUE if signal has occurred */
    Tcl_Interp *interp;		/* interp to use or 0 if we should use the */
				/* interpreter active at the time the sig */
				/* is processed */
    int code;			/* return our new code instead of code */
				/* available when signal is processed */
    CONST char *name;			/* name of signal */
    int reserved;		/* if unavailable for trapping */
    int enabled;		/* set on Win32 if we are trapping signal */
} traps[NSIG];

int sigchld_count = 0;	/* # of sigchlds caught but not yet processed */

static int eval_trap_action();

static int got_sig;		/* this records the last signal received */
				/* it is only a hint and can be wiped out */
				/* by multiple signals, but it will always */
				/* be left with a valid signal that is */
				/* pending */

static Tcl_AsyncHandler async_handler;

static CONST char *
signal_to_string(sig)
    int sig;
{
	if (sig <= 0 || sig > NSIG) return("SIGNAL OUT OF RANGE");
	return(traps[sig].name);
}

/* current sig being processed by user sig handler */
static int current_sig = NO_SIG;

int exp_nostack_dump = FALSE;	/* TRUE if user has requested unrolling of */
				/* stack with no trace */



/*ARGSUSED*/
static int
tophalf(ClientData clientData, Tcl_Interp *interp, int code)
{
    struct trap *trap;	/* last trap processed */
    int rc;
    int i;
    Tcl_Interp *sig_interp;
/*  extern Tcl_Interp *exp_interp;*/

    expDiagLog("sighandler: handling signal(%d)\r\n",got_sig);

    if (got_sig <= 0 || got_sig >= NSIG) {
	expErrorLog("caught impossible signal %d\r\n",got_sig);
	abort();
    }

    /* start to work on this sig.  got_sig can now be overwritten */
    /* and it won't cause a problem */
    current_sig = got_sig;
    trap = &traps[current_sig];

    trap->mark = FALSE;

#ifdef SIGCHLD
    /* decrement below looks dangerous */
    /* Don't we need to temporarily block bottomhalf? */
    if (current_sig == SIGCHLD) {
	sigchld_count--;
	expDiagLog("sigchld_count-- == %d\n",sigchld_count);
    }
#endif

    if (!trap->action) {
	/* In this one case, we let ourselves be called when no */
	/* signaler predefined, since we are calling explicitly */
	/* from another part of the program, and it is just simpler */
	if (current_sig == 0) return code;
	expErrorLog("caught unexpected signal: %s (%d)\r\n",
		 signal_to_string(current_sig),current_sig);
	abort();
    }

    if (trap->interp) {
	/* if trap requested original interp, use it */
	sig_interp = trap->interp;
    } else if (!interp) {
	/* else if another interp is available, use it */
	sig_interp = interp;
    } else {
	/* fall back to exp_interp */
	sig_interp = exp_interp;
    }

    rc = eval_trap_action(sig_interp,current_sig,trap,code);
    current_sig = NO_SIG;

    /*
     * scan for more signals to process
     */

    /* first check for additional SIGCHLDs */
    if (sigchld_count) {
#ifdef SIGCHLD
	got_sig = SIGCHLD;
	traps[SIGCHLD].mark = TRUE;
	Tcl_AsyncMark(async_handler);
#endif
    } else {
	got_sig = -1;
	for (i=1;i<NSIG;i++) {
	    if (traps[i].mark) {
		got_sig = i;
		Tcl_AsyncMark(async_handler);
		break;
	    }
	}
    }
    return rc;
}

#ifdef REARM_SIG
int sigchld_sleep;
static int rearm_sigchld = FALSE;	/* TRUE if sigchld needs to be */
					/* rearmed (i.e., because it has
					/* just gone off) */
static int rearming_sigchld = FALSE;
#endif

/* called upon receipt of a user-declared signal */
static void
bottomhalf(int sig)
{
#ifdef REARM_SIG
    /*
     * tiny window of death if same signal should arrive here
     * before we've reinstalled it
     */

    /* In SV, sigchld must be rearmed after wait to avoid recursion */
    if (sig != SIGCHLD) {
	signal(sig,bottomhalf);
    } else {
	/* request rearm */
	rearm_sigchld = TRUE;
	if (rearming_sigchld) sigchld_sleep = TRUE;
    }
#endif

    traps[sig].mark = TRUE;
    got_sig = sig;		/* just a hint - can be wiped out by another */
    Tcl_AsyncMark(async_handler);

    /* if we are called while this particular async is being processed */
    /* original async_proc will turn off "mark" so that when async_proc */
    /* is recalled, it will see that nothing was left to do */

    /* In case of SIGCHLD though, we must recall it as many times as
     * we have received it.
     */
#ifdef SIGCHLD
    if (sig == SIGCHLD) {
	sigchld_count++;
/*		exp_debuglog(stderr,"sigchld_count++ == %d\n",sigchld_count);*/
    }
#endif
#if 0
    /* if we are doing an i_read, restart it */
    if (env_valid && (sig != 0)) longjmp(env,2);
#endif
}

/*ARGSUSED*/
void
exp_rearm_sigchld(interp)
    Tcl_Interp *interp;
{
#ifdef REARM_SIG
    if (rearm_sigchld) {
	rearm_sigchld = FALSE;
	rearming_sigchld = TRUE;
	signal(SIGCHLD,bottomhalf);
    }

    rearming_sigchld = FALSE;

    /* if the rearming immediately caused another SIGCHLD, slow down */
    /* It's probably one of Tcl's intermediary pipeline processes that */
    /* Tcl hasn't caught up with yet. */
    if (sigchld_sleep) {
	exp_dsleep(interp,0.2);
	sigchld_sleep = FALSE;
    }
#endif
}

#ifdef __WIN32__
static BOOL WINAPI
ctrl_event(DWORD event)
{
    int sig = 0;

    expDiagLog("trap: ctrl_event %d\r\n", event);
    if (event == CTRL_C_EVENT) {
	sig = 2;
    } else if (event == CTRL_BREAK_EVENT) {
	sig = 3;
    }
    if (sig) {
	switch (traps[sig].enabled) {
	case SIGNAL_IGNORE:
	    return TRUE;
	case SIGNAL_DEFAULT:
	    return FALSE;
	case SIGNAL_ENABLED:
	    bottomhalf(sig);
	    return TRUE;
	}
    }
    return FALSE;
}
#endif

void
exp_init_trap()
{
    int i;

    for (i=1;i<NSIG;i++) {
	traps[i].name = Tcl_SignalId(i);
	traps[i].action = 0;
	traps[i].reserved = FALSE;
	traps[i].enabled = SIGNAL_DEFAULT;
    }
#ifdef __WIN32__
    traps[3].name = "SIGQUIT";
#endif

    /*
     * fix up any special cases
     */

#if defined(SIGCLD)
    /* Tcl names it SIGCLD, not good for portable scripts */
    traps[SIGCLD].name = "SIGCHLD";
#endif
#if defined(SIGALRM)
    traps[SIGALRM].reserved = TRUE;
#endif
#if defined(SIGKILL)
    traps[SIGKILL].reserved = TRUE;
#endif
#if defined(SIGSTOP)
    traps[SIGSTOP].reserved = TRUE;
#endif

    async_handler = Tcl_AsyncCreate(tophalf,(ClientData)NULL);
}

/* given signal index or name as string, */
/* returns signal index or -1 if bad arg */
int
exp_string_to_signal(interp,s)
    Tcl_Interp *interp;
    CONST char *s;
{
    int sig;
    CONST char *name;

    /* try interpreting as an integer */
    if (1 == sscanf(s,"%d",&sig)) {
	if (sig > 0 && sig < NSIG) return sig;
    } else {
	/* try interpreting as a string */
	for (sig=1;sig<NSIG;sig++) {
	    name = traps[sig].name;
	    if (streq(s,name) || streq(s,name+3)) return(sig);
	}
    }

    exp_error(interp,"invalid signal %s",s);
	
    return -1;
}

/*ARGSUSED*/
int
Exp_TrapCmd(clientData, interp, argc, argv)
    ClientData clientData;
    Tcl_Interp *interp;
    int argc;
    CONST char *argv[];
{
    CONST char *action = NULL;
    int n;		/* number of signals in list */
    char **list = NULL;	/* list of signals */
    int len;		/* length of action */
    int i;
    int show_name = FALSE;	/* if user asked for current sig by name */
    int show_number = FALSE;	/* if user asked for current sig by number */
    int show_max = FALSE;	/* if user asked for NSIG-1 */
    int rc = TCL_OK;
    int new_code = FALSE;	/* if action result should overwrite orig */
    Tcl_Interp *new_interp = interp;/* interp in which to evaluate */
				    /* action when signal occurs */

    argc--; argv++;

    while (*argv) {
	if (streq(*argv,"-code")) {
	    argc--; argv++; 
	    new_code = TRUE;
	} else if (streq(*argv,"-interp")) {
	    argc--; argv++; 
	    new_interp = 0;
	} else if (streq(*argv,"-name")) {
	    argc--; argv++;
	    show_name = TRUE;
	} else if (streq(*argv,"-number")) {
	    argc--; argv++;
	    show_number = TRUE;
	} else if (streq(*argv,"-max")) {
	    argc--; argv++;
	    show_max = TRUE;
	} else break;
    }

    if (show_name || show_number || show_max) {
	if (argc > 0) goto usage_error;
	if (show_max) {
	    Tcl_SetObjResult(interp, Tcl_NewIntObj(NSIG-1));
	    return TCL_OK;
	}

	if (current_sig == NO_SIG) {
	    exp_error(interp,"no signal in progress");
	    return TCL_ERROR;
	}
	if (show_name) {
	    /* skip over "SIG" */
	    Tcl_SetObjResult(interp,
		    Tcl_NewStringObj(signal_to_string(current_sig) + 3, -1));
	} else {
	    Tcl_SetObjResult(interp, Tcl_NewIntObj(current_sig));
	}
	return TCL_OK;
    }

    if (argc == 0 || argc > 2) goto usage_error;

    if (argc == 1) {
	int sig = exp_string_to_signal(interp,*argv);
	if (sig == -1) return TCL_ERROR;

	if (traps[sig].action) {
	    Tcl_AppendResult(interp,traps[sig].action,(char *)0);
	} else {
	    Tcl_SetResult(interp, "SIG_DFL", TCL_STATIC);
	}
	return TCL_OK;
    }

    action = *argv;

    /* argv[1] is the list of signals - crack it open */
    if (TCL_OK != Tcl_SplitList(interp,argv[1],&n,&list)) {
	expErrorLog("%s\r\n", Tcl_GetStringResult(interp));
	goto usage_error;
    }

    for (i=0;i<n;i++) {
	int sig = exp_string_to_signal(interp,list[i]);
	if (sig == -1) {
	    rc = TCL_ERROR;
	    break;
	}

	if (traps[sig].reserved) {
	    exp_error(interp,"cannot trap %s",signal_to_string(sig));
	    rc = TCL_ERROR;
	    break;
	}

#if 0
#ifdef TCL_DEBUGGER
	if (sig == SIGINT && exp_tcl_debugger_available) {
	    expDiagLog("trap: cannot trap SIGINT while using debugger\r\n");
	    continue;
	}
#endif /* TCL_DEBUGGER */
#endif

	expDiagLog("trap: setting up signal %d (\"%s\")\r\n",sig,list[i]);

	if (traps[sig].action) ckfree(traps[sig].action);

	if (streq(action,"SIG_DFL")) {
	    /* should've been free'd by now if nec. */
	    traps[sig].action = 0;
	    traps[sig].enabled = SIGNAL_DEFAULT;
#ifndef WIN32
	    signal(sig,SIG_DFL);
#endif
#ifdef REARM_SIG
	    if (sig == SIGCHLD)
		rearm_sigchld = FALSE;
#endif /*REARM_SIG*/
	} else {
	    len = 1 + strlen(action);
	    traps[sig].action = ckalloc(len);
	    memcpy(traps[sig].action,action,len);
	    traps[sig].interp = new_interp;
	    traps[sig].code = new_code;
	    if (streq(action,"SIG_IGN")) {
#ifdef WIN32
		traps[sig].enabled = SIGNAL_IGNORE;
#else
		signal(sig,SIG_IGN);
#endif
	    } else {
#ifdef WIN32
		if (sig == 2 || sig == 3) {
		    traps[sig].enabled = SIGNAL_ENABLED;
		    SetConsoleCtrlHandler(ctrl_event, TRUE);
		}
#else
		signal(sig,bottomhalf);
#endif
	    }
	}
    }
    if (list) {
	ckfree((char *)list);
    }
    return(rc);
usage_error:
    exp_error(interp,"usage: trap [command or SIG_DFL or SIG_IGN] {list of signals}");
    return TCL_ERROR;
}

/* called by tophalf() to process the given signal */
static int
eval_trap_action(interp,sig,trap,oldcode)
    Tcl_Interp *interp;
    int sig;
    struct trap *trap;
    int oldcode;
{
    int code_flag;
    int newcode;
    Tcl_DString ei;	/* errorInfo */
    CONST char *eip;
    Tcl_DString ec;	/* errorCode */
    CONST char *ecp;
    Tcl_DString ir;	/* interp's result */

    expDiagLog("async event handler: Tcl_Eval(%s)\r\n",trap->action);

    /* save to prevent user from redefining trap->code while trap */
    /* is executing */
    code_flag = trap->code;

    if (!code_flag) {
	/* 
	 * save return values
	 */
	eip = Tcl_GetVar(interp,"errorInfo",TCL_GLOBAL_ONLY);
	if (eip) {
	    Tcl_DStringInit(&ei);
	    eip = (char *) Tcl_DStringAppend(&ei,eip,-1);
	}
	ecp = Tcl_GetVar(interp,"errorCode",TCL_GLOBAL_ONLY);
	if (ecp) {
	    Tcl_DStringInit(&ec);
	    ecp = (char *) Tcl_DStringAppend(&ec,ecp,-1);
	}
	/* I assume interp's result is always non-zero, right? */
	Tcl_DStringInit(&ir);
	Tcl_DStringAppend(&ir, Tcl_GetStringResult(interp), -1);
    }

    newcode = Tcl_GlobalEval(interp,trap->action);

    /*
     * if new code is to be ignored (usual case - see "else" below)
     *	allow only OK/RETURN from trap, otherwise complain
     */

    if (code_flag) {
	char *resultStr;
	expDiagLog("return value = %d for trap %s, action %s\r\n",
		     newcode,signal_to_string(sig),trap->action);
	resultStr = (char *) Tcl_GetStringResult(interp);
	if (resultStr != 0) {
	    expErrorLog("%s\r\n", resultStr);

	    /*
	     * Check errorinfo and see if it contains -nostack.
	     * This shouldn't be necessary, but John changed the
	     * top level interp so that it distorts arbitrary
	     * return values into TCL_ERROR, so by the time we
	     * get back, we'll have lost the value of errorInfo
	     */

	    eip = Tcl_GetVar(interp,"errorInfo",TCL_GLOBAL_ONLY);
	    exp_nostack_dump =
		(eip && (0 == strncmp("-nostack",eip,8)));
	}
    } else if (newcode != TCL_OK && newcode != TCL_RETURN) {
	if (newcode != TCL_ERROR) {
	    exp_error(interp,"return value = %d for trap %s, action %s\r\n",newcode,signal_to_string(sig),trap->action);
	}
	Tcl_BackgroundError(interp);
    }

    if (!code_flag) {
	/*
	 * restore values
	 */
	Tcl_ResetResult(interp);	/* turns off Tcl's internal */
	/* flags: ERR_IN_PROGRESS, ERROR_CODE_SET */

	if (eip) {
	    Tcl_AddErrorInfo(interp,eip);
	    Tcl_DStringFree(&ei);
	} else {
	    Tcl_UnsetVar(interp,"errorInfo",0);
	}

	/* restore errorCode.  Note that Tcl_AddErrorInfo (above) */
	/* resets it to NONE.  If the previous value is NONE, it's */
	/* important to avoid calling Tcl_SetErrorCode since this */
	/* with cause Tcl to set its internal ERROR_CODE_SET flag. */
	if (ecp) {
	    if (!streq("NONE",ecp))
		Tcl_SetErrorCode(interp,ecp,(char *)0);
	    Tcl_DStringFree(&ec);
	} else {
	    Tcl_UnsetVar(interp,"errorCode",0);
	}

	Tcl_DStringResult(interp,&ir);
	Tcl_DStringFree(&ir);

	newcode = oldcode;

	/* note that since newcode gets overwritten here by old code */
	/* it is possible to return in the middle of a trap by using */
	/* "return" (or "continue" for that matter)! */
    }
    return newcode;
}

static struct exp_cmd_data
cmd_data[]  = {
    {"trap",	0, Exp_TrapCmd,	NULL,	0},
    {0}
};

void
exp_init_trap_cmds(interp)
    Tcl_Interp *interp;
{
    exp_create_commands(interp,cmd_data);
}

