/* ----------------------------------------------------------------------------
 * expWinTty.c --
 *
 *	Implements some tty related functions.  Handles interaction with
 *	the console.
 *
 *	NOTE: The win32 console channel driver might need to have it's
 *	interest mask modified so we can get mouse and window events when
 *	[interact] is to be implimented.
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
 * RCS: @(#) $Id: expWinTty.c,v 1.1.2.1.2.5 2003/08/26 20:46:54 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expInt.h"

//struct exp_f *exp_dev_tty = NULL;
exp_file exp_dev_tty = EXP_BAD_FILE;
char *exp_dev_tty_id = "exp_tty";

int exp_ioctled_devtty = FALSE;
int exp_stdin_is_tty;
int exp_stdout_is_tty;

exp_tty exp_tty_original;

static int consoleInitialized = FALSE;
static DWORD originalConsoleOutMode;
static DWORD originalConsoleInMode;
static DWORD consoleOutMode;
static DWORD consoleInMode;
static HANDLE hConsoleOut;
static HANDLE hConsoleIn;

typedef struct ThreadSpecificData {
    Tcl_DString cookScratch;
} ThreadSpecificData;

static Tcl_ThreadDataKey dataKey;

#define tty_current exp_tty_current
#define tty_cooked exp_tty_cooked

int
exp_israw()
{
    if (! consoleInitialized) {
	return 0;
    }
    return !(consoleOutMode & ENABLE_PROCESSED_OUTPUT);
}

int
exp_isecho()
{
    if (! consoleInitialized) {
	return 1;
    }
    return (consoleInMode & ENABLE_ECHO_INPUT);
}

/*
 *----------------------------------------------------------------------
 *
 * exp_tty_raw --
 *
 *	Set the tty status.  If 1, set to raw.  Otherwise, set
 *	to normal.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	May change the current console settings
 *
 *----------------------------------------------------------------------
 */

void
exp_tty_raw(set)
    int set;
{
    if (! consoleInitialized) {
	return;
    }
    /*
     * FIX: I suspect we want to manage hConsoleIn and ENABLE_LINE_INPUT
     * as well to single-char reading, but there appears to be other
     * magic necessary to really make it work. - JH
     */
    if (set == 1) {
	consoleOutMode &= ~ENABLE_PROCESSED_OUTPUT;
	//consoleInMode &= ~ENABLE_LINE_INPUT;
    } else {
	consoleOutMode |= ENABLE_PROCESSED_OUTPUT;
	//consoleInMode |= ENABLE_LINE_INPUT;
    }
    SetConsoleMode(hConsoleOut, consoleOutMode);
    //SetConsoleMode(hConsoleIn, consoleInMode);
}

/*
 *----------------------------------------------------------------------
 *
 * exp_tty_echo --
 *
 *	Set the tty status.  If 1, set to echo.  Otherwise, set
 *	to no echo.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	May change the current console settings
 *
 *----------------------------------------------------------------------
 */

void
exp_tty_echo(set)
    int set;
{
    if (! consoleInitialized) {
	return;
    }
    if (set == 1) {
	consoleInMode |= ENABLE_ECHO_INPUT;
    } else {
	consoleInMode &= ~ENABLE_ECHO_INPUT;
    }
    SetConsoleMode(hConsoleIn, consoleInMode);
}

int
exp_tty_set_simple(tty)
    exp_tty *tty;
{
    return 0; /* no error, but do nothing */
}

int
exp_tty_get_simple(tty)
    exp_tty *tty;
{
    return 0; /* no error, but do nothing */
}

/* returns 0 if nothing changed */
/* if something changed, the out parameters are changed as well */
int
exp_tty_cooked_echo(interp,tty_old,was_raw,was_echo)
    Tcl_Interp *interp;
    exp_tty *tty_old;
    int *was_raw, *was_echo;
{
	if (exp_disconnected) return(0);
	if (!exp_israw() && !exp_isecho()) return(0);
	if (exp_dev_tty == EXP_BAD_FILE) return(0);

	*tty_old = tty_current;		/* save old parameters */
	*was_raw = exp_israw();
	*was_echo = exp_isecho();
	expDiagLog("tty_cooked_echo: was raw = %d  echo = %d\r\n",
		exp_israw(), exp_isecho());

	exp_tty_raw(-1);
	exp_tty_echo(1);

	if (exp_tty_set_simple(&tty_current) == -1) {
		expErrorLog("ioctl(noraw): %s\r\n",Tcl_PosixError(interp));

		/* SF #439042 -- Allow overide of "exit" by user / script
		 */
		{
		  char buffer [] = "exit 1";
		  Tcl_Eval(interp, buffer); 
		}
	}
	exp_ioctled_devtty = TRUE;

	return(1);
}

void
exp_tty_set(interp,tty,raw,echo)
    Tcl_Interp *interp;
    exp_tty *tty;
    int raw;
    int echo;
{
}

/*
 *----------------------------------------------------------------------
 *
 * exp_cook --
 *
 *	take strings with newlines and insert carriage-returns.
 *	This allows user to write send_user strings without always
 *	putting in \r.  If len == 0, use strlen to compute it
 *	NB: if terminal is not in raw mode, nothing is done.
 *
 * Results:
 *	The same string if in raw mode, the modified string otherwise
 *
 *----------------------------------------------------------------------
 */

CONST char *
exp_cook(s,len)
    CONST char *s;
    int *len;	/* current and new length of s */
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    Tcl_DString *ds = &tsdPtr->cookScratch;

    if (s == 0) return("<null>");

    if (ds == NULL) {
	Tcl_DStringInit(ds);
    } else {
	Tcl_DStringSetLength(ds, 0);
    }

    while (*s != '\0') {
	if (*s != '\n') {
	    Tcl_DStringAppend(ds, s, 1);
	} else {
	    Tcl_DStringAppend(ds,"\r\n", 2);
	}
	s++;
    }

    if (len) {
	*len = Tcl_DStringLength(ds);
    }
    /*
     * XXX: FIX No DStringFree!
     */
    return Tcl_DStringValue(ds);
}


void
exp_init_stdio()
{
    exp_stdin_is_tty = isatty(0);
    exp_stdout_is_tty = isatty(1);

    setbuf(stdout,(char *)0);	/* unbuffer stdout */
}

/*
 *----------------------------------------------------------------------
 *
 * exp_tty_break --
 *
 *	Send a BREAK to the controlled subprocess
 *
 * Results:
 *	XXX: None, so we can't propagate an error back up
 *
 *----------------------------------------------------------------------
 */

/*ARGSUSED*/
void
exp_tty_break(interp,f)
    Tcl_Interp *interp;
    struct exp_f *f;
{
#ifdef POSIX
    tcsendbreak(fd,0);
#endif
    Tcl_AppendResult(interp, "exp_tty_break not implemented", NULL);
}

/*
 *----------------------------------------------------------------------
 *
 * Exp_SttyCmd --
 *
 *	Implements the 'stty' and 'exp_stty' command.
 *
 * Results:
 *	A standard Tcl result
 *
 * Side Effects:
 *	Can change the characteristics of the console
 *
 *----------------------------------------------------------------------
 */

/*ARGSUSED*/
static int
Exp_SttyCmd(clientData, interp, argc, argv)
    ClientData clientData;
    Tcl_Interp *interp;
    int argc;
    CONST char **argv;
{
#if 1
    /* redirection symbol is not counted as a stty arg in terms */
    /* of recognition. */
    int saw_unknown_stty_arg = FALSE;
    int saw_known_stty_arg = FALSE;
    int no_args = TRUE;

    int rc = TCL_OK;
    int cooked = FALSE;
    int was_raw, was_echo;
    char *resultStr = NULL;

    CONST char **argv0 = argv;

    for (argv=argv0+1;*argv;argv++) {
	if (argv[0][0] == '<') {
	    Tcl_AppendResult(interp, argv0,
		": redirection not supported on Windows NT", NULL);
	    return TCL_ERROR;
	}
    }

    was_raw = exp_israw();
    was_echo = exp_isecho();

    for (argv=argv0+1;*argv;argv++) {
	if (streq(*argv,"raw") ||
	    streq(*argv,"-cooked")) {
	    exp_tty_raw(1);
	    saw_known_stty_arg = TRUE;
	    no_args = FALSE;
	} else if (streq(*argv,"-raw") ||
	    streq(*argv,"cooked")) {
	    cooked = TRUE;
	    exp_tty_raw(-1);
	    saw_known_stty_arg = TRUE;
	    no_args = FALSE;
	} else if (streq(*argv,"echo")) {
	    exp_tty_echo(1);
	    saw_known_stty_arg = TRUE;
	    no_args = FALSE;
	} else if (streq(*argv,"-echo")) {
	    exp_tty_echo(-1);
	    saw_known_stty_arg = TRUE;
	    no_args = FALSE;
#ifdef XXX /* This can be implemented, but it isn't right now */
	} else if (streq(*argv,"rows")) {
	    if (*(argv+1)) {
		exp_win_rows_set(*(argv+1));
		argv++;
		no_args = FALSE;
	    } else {
		exp_win_rows_get(Tcl_GetStringResult(interp));
		return TCL_OK;
	    }
	} else if (streq(*argv,"columns")) {
	    if (*(argv+1)) {
		exp_win_columns_set(*(argv+1));
		argv++;
		no_args = FALSE;
	    } else {
		exp_win_columns_get(Tcl_GetStringResult(interp));
		return TCL_OK;
	    }
#endif
	} else {
	    saw_unknown_stty_arg = TRUE;
	}
    }

    resultStr = (char *) Tcl_GetStringResult(interp);
    /* if no result, make a crude one */
    if (*resultStr == '\0') {
	char buf[32];
	sprintf(buf,"%sraw %secho", (was_raw?"":"-"), (was_echo?"":"-"));
	Tcl_SetResult(interp, buf, TCL_VOLATILE);
    }

    return rc;
#else
    Tcl_AppendResult(interp, argv[0], ": not implemented", NULL);
    return TCL_ERROR;
#endif
}

/*ARGSUSED*/
static int
Exp_SystemCmd(clientData, interp, argc, argv)
    ClientData clientData;
    Tcl_Interp *interp;
    int argc;
    CONST char **argv;
{
    Tcl_AppendResult(interp, argv[0], ": not implemented", NULL);
    return TCL_ERROR;
}

static struct exp_cmd_data
cmd_data[]  = {
    {"stty",	0, Exp_SttyCmd,	0,	0},
    {"system",	0, Exp_SystemCmd,	0,	0},
    {0}
};

/*
 *----------------------------------------------------------------------
 *
 * exp_init_tty_cmds --
 *
 *	Adds tty related commands to the interpreter
 *
 * Results:
 *	A standard TCL result
 *
 * Side Effects:
 *	Commands are added to the interpreter.
 *
 *----------------------------------------------------------------------
 */

void
exp_init_tty_cmds(interp)
    Tcl_Interp *interp;
{
    exp_create_commands(interp,cmd_data);
}

/*
 *----------------------------------------------------------------------
 *
 * exp_init_pty --
 *
 *	This is where the console device is initialized.  We wrap
 *	it in a channel (if it is available).
 *
 * Results:
 *	None at the moment.
 *
 * Side Effects:
 *	A channel name is set and an exp_f structure is created.
 *
 * Notes:
 *	XXX: GetCurrentProcessId() might not be the appropriate thing
 *	as exp_getpid was previously used.  We might want to have
 *	a separate console input channel.
 *
 *----------------------------------------------------------------------
 */

void
exp_init_pty (
    Tcl_Interp *interp)
{
    HANDLE hOut, hIn;
    Tcl_Channel channel, inChannel, outChannel;
    int mode;
    Tcl_ChannelType *type;
    ExpState *esPtr;

    hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    /*
     * Due to a rather serious bug in the default loader of Windows
     * 2000, GetStdHandle can return a bogus HANDLE due to the
     * STARTUPINFO being filled with garbage.  Use Tcl_MakeFileChannel
     * to test the validity of this HANDLE as it has the logic to do
     * it.  There's no sense in re-inventing the test here.
     */

    outChannel = Tcl_MakeFileChannel(hOut, TCL_WRITABLE);
    if (outChannel != NULL) {
	/* good and valid */
	Tcl_RegisterChannel(interp, outChannel);
    } else {
	outChannel = Tcl_GetChannel(interp, "stdout", &mode);
	type = Tcl_GetChannelType(outChannel);
	if (!strcmp(type->typeName, "console") ||
		Tcl_GetChannelHandle(outChannel, TCL_WRITABLE, &hOut)
		== TCL_ERROR) {
	    /* No native console available.  Must be a GUI application */
	    return;
	}
    }

    hIn = GetStdHandle(STD_INPUT_HANDLE);

    inChannel = Tcl_MakeFileChannel(hIn, TCL_READABLE);
    if (inChannel != NULL) {
	/* good and valid */
	Tcl_RegisterChannel(interp, inChannel);
    } else {
	inChannel = Tcl_GetChannel(interp, "stdin", &mode);
	type = Tcl_GetChannelType(inChannel);
	if (!strcmp(type->typeName, "console") ||
		Tcl_GetChannelHandle(inChannel, TCL_READABLE, &hIn)
		== TCL_ERROR) {
	    /* No native console available.  Must be a GUI application */
	    return;
	}
    }

    /*
     * Create an alias channel
     */

    channel = Exp_CreatePairChannel(interp, inChannel, outChannel,
	    exp_dev_tty_id);
    Tcl_RegisterChannel(interp, channel);
    channel = Exp_CreateExpChannel(interp, channel, 0, EXP_NOPID, &esPtr);
    esPtr->keepForever = 1;

    /*
     * Get the original console modes
     */
    if (GetConsoleMode(hOut, &originalConsoleOutMode)) {
	consoleOutMode = originalConsoleOutMode;
	hConsoleOut = hOut;
    }

    if (GetConsoleMode(hIn, &originalConsoleInMode)) {
	consoleInMode = originalConsoleInMode;
	hConsoleIn = hIn;
    }
    consoleInitialized = TRUE;
}

/*
 *----------------------------------------------------------------------
 *
 * exp_pty_exit --
 *
 *	Routine to set the terminal mode back to normal on exit.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Console mode is set to what is was when we entered Expect.
 *
 *----------------------------------------------------------------------
 */

void
exp_pty_exit()
{
    if (consoleInitialized) {
	SetConsoleMode(hConsoleIn, originalConsoleInMode);
	SetConsoleMode(hConsoleOut, originalConsoleOutMode);
    }
}
