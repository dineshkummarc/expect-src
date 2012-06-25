/* ----------------------------------------------------------------------------
 * expInt.h --
 *
 *	Declarations of things used internally by Expect.
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
 * RCS: @(#) $Id: expInt.h,v 1.1.4.5 2002/03/07 02:49:36 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#ifndef _EXPINT
#define _EXPINT

#ifndef _EXP
#   include "exp.h"
#endif

#ifndef _TCLPORT
#if (TCL_MAJOR_VERSION > 8) || ((TCL_MAJOR_VERSION == 8) && (TCL_MINOR_VERSION > 4))
/* Tcl 8.5+. Use official internal API */
#   include "tclInt.h"
#else
/* Tcl 8.4. Goes deeper into the internals */
#   include "tclPort.h"
#endif
#endif

#ifndef _EXPPORT
#   include "expPort.h"
#endif



#undef TCL_STORAGE_CLASS
#if defined(BUILD_slavedriver)
#   define TCL_STORAGE_CLASS
#elif defined(BUILD_exp)
#   define TCL_STORAGE_CLASS DLLEXPORT
#else
#   ifdef USE_EXP_STUBS
#	define TCL_STORAGE_CLASS
#   else
#	define TCL_STORAGE_CLASS DLLIMPORT
#   endif
#endif

/*
 * Fix Tcl bug #803489 the right way.  We need to always use the old Stub
 * slot positions, not the new broken ones part of TIP 127.  I do like
 * that these functions have moved to the public space (about time), but
 * the slot change is the killer and is the painful side affect.
 */

#if defined(USE_TCL_STUBS) && \
	(TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION >= 5)
#   undef Tcl_CreateNamespace
#   define Tcl_CreateNamespace \
	(tclIntStubsPtr->tcl_CreateNamespace)
#   undef Tcl_DeleteNamespace
#   define Tcl_DeleteNamespace \
	(tclIntStubsPtr->tcl_DeleteNamespace)
#   undef Tcl_AppendExportList
#   define Tcl_AppendExportList \
	(tclIntStubsPtr->tcl_AppendExportList)
#   undef Tcl_Export
#   define Tcl_Export \
	(tclIntStubsPtr->tcl_Export)
#   undef Tcl_Import
#   define Tcl_Import \
	(tclIntStubsPtr->tcl_Import)
#   undef Tcl_ForgetImport
#   define Tcl_ForgetImport \
	(tclIntStubsPtr->tcl_ForgetImport)
#   undef Tcl_GetCurrentNamespace
#   define Tcl_GetCurrentNamespace \
	(tclIntStubsPtr->tcl_GetCurrentNamespace)
#   undef Tcl_GetGlobalNamespace
#   define Tcl_GetGlobalNamespace \
	(tclIntStubsPtr->tcl_GetGlobalNamespace)
#   undef Tcl_FindNamespace
#   define Tcl_FindNamespace \
	(tclIntStubsPtr->tcl_FindNamespace)
#   undef Tcl_FindCommand
#   define Tcl_FindCommand \
	(tclIntStubsPtr->tcl_FindCommand)
#   undef Tcl_GetCommandFromObj
#   define Tcl_GetCommandFromObj \
	(tclIntStubsPtr->tcl_GetCommandFromObj)
#   undef Tcl_GetCommandFullName
#   define Tcl_GetCommandFullName \
	(tclIntStubsPtr->tcl_GetCommandFullName)
#endif


#ifdef HAVE_SYS_WAIT_H
  /* ISC doesn't def WNOHANG unless _POSIX_SOURCE is def'ed */
# ifdef WNOHANG_REQUIRES_POSIX_SOURCE
#  define _POSIX_SOURCE
# endif
# include <sys/wait.h>
# ifdef WNOHANG_REQUIRES_POSIX_SOURCE
#  undef _POSIX_SOURCE
# endif
#endif

#define EXP_CHANNELNAMELEN (16 + TCL_INTEGER_SPACE)


#define exp_flageq(flag,string,minlen) \
(((string)[0] == (flag)[0]) && (exp_flageq_code(((flag)+1),((string)+1),((minlen)-1))))

/* exp_flageq for single char flags */
#define exp_flageq1(flag,string) \
	((string[0] == flag) && (string[1] == '\0'))

#define EXP_SPAWN_ID_USER		0
#define EXP_SPAWN_ID_ANY_LIT		"-1"

#define EXP_CHANNEL_PREFIX "exp"
#define EXP_CHANNEL_PREFIX_LENGTH 3
#define isExpChannelName(name) \
    (0 == strncmp(name,EXP_CHANNEL_PREFIX,EXP_CHANNEL_PREFIX_LENGTH))

#define exp_is_stdinfd(x)	((x) == 0)
#define exp_is_devttyfd(x)	((x) == exp_dev_tty)

#define EXP_NOPID	0	/* Used when there is no associated pid to */
				/* wait for.  For example: */
				/* 1) When fd opened by someone else, e.g., */
				/* Tcl's open */
				/* 2) When entry not in use */
				/* 3) To tell user pid of "spawn -open" */
				/* 4) stdin, out, error */

#define EXP_NOFD	-1

/* these are occasionally useful to distinguish between various expect */
/* commands and are also used as array indices into the per-fd eg[] arrays */
#define EXP_CMD_BEFORE	0
#define EXP_CMD_AFTER	1
#define EXP_CMD_BG	2
#define EXP_CMD_FG	3

/*
 * This structure describes per-instance state of an Exp channel.
 */

typedef struct ExpState {
    Tcl_Channel channel;/* Channel associated with this container channel. */
    Tcl_Channel slave;	/* The actual channel being forwarded to. */
    char name[EXP_CHANNELNAMELEN+1];
			/* expect and interact set variables to channel
			 * name, so for efficiency cache it here this may
			 * go away if we find it is not needed it might be
			 * needed by inherited channels */
    int watchMask;	/* Current interest mask (see validMask) */
    int validMask;	/* OR'ed combination of TCL_READABLE,
			 * TCL_WRITABLE, or TCL_EXCEPTION: indicates
			 * which operations are valid on the file. */
    int pid;		/* True process identifier or EXP_NOPID if no pid */
    Tcl_Pid tclPid;	/* ugly HANDLE abstraction used for the windows OS. */
    Tcl_Obj *buffer;	/* input buffer */
    int msize;	        /* # of bytes that buffer can hold (max) */
    int umsize;	        /* # of bytes (min) that is guaranteed to match */
			/* this comes from match_max command */
    int printed;	/* # of bytes written to stdout (if logging on) */
                        /* but not actually returned via a match yet */
    int echoed;	        /* additional # of bytes (beyond "printed" above) */
                        /* echoed back but not actually returned via a match */
                        /* yet.  This supports interact -echo */
    int rm_nulls;	/* if nulls should be stripped before pat matching */
    int open;		/* if fdin/fdout open */
    int user_waited;    /* if user has issued "wait" command */
    int sys_waited;	/* if wait() (or variant) has been called */
    int registered;	/* if channel registered */
    WAIT_STATUS_TYPE wait;
			/* raw status from wait() */
    int parity;	        /* if parity should be preserved */
    int close_on_eof;   /* if channel should be closed automatically on eof */
    int key;	        /* unique id that identifies what command instance */
                        /* last touched this buffer */
    int force_read;	/* force read to occur (even if buffer already has */
                        /* data).  This supports interact CAN_MATCH */
    int notified;	/* If Tcl_NotifyChannel has been called and we */
		        /* have not yet read from the channel. */
    int notifiedMask;	/* Mask reported when notified. */
    int fg_armed;	/* If Tk_CreateFileHandler is active for responding */
                        /* to foreground events */
#ifdef HAVE_PTYTRAP
    char *slave_name;   /* Full name of slave, i.e., /dev/ttyp0 */
#endif /* HAVE_PTYTRAP */
    /* may go away */
    int leaveopen;	/* If we should not call Tcl's close when we close - */
                        /* only relevant if Tcl does the original open */

    Tcl_Interp *bg_interp;	/* interp to process the bg cases */
    int bg_ecount;		/* number of background ExpStates */
    enum {
	blocked,	/* blocked because we are processing the */
			/* file handler */
	armed,		/* normal state when bg handler in use */
	unarmed,	/* no bg handler in use */
	disarm_req_while_blocked	/* while blocked, a request */
				/* was received to disarm it.  Rather than */
				/* processing the request immediately, defer */
				/* it so that when we later try to unblock */
				/* we will see at that time that it should */
				/* instead be disarmed */
    } bg_status;

    /*
     * If the channel is freed while in the middle of a bg event handler,
     * remember that and defer freeing of the ExpState structure until
     * it is safe.
     */
    int freeWhenBgHandlerUnblocked;

    /* If channel is closed but not yet waited on, we tie up the fd by
     * attaching it to /dev/null.  We play this little game so that we
     * can embed the fd in the channel name.  If we didn't tie up the
     * fd, we'd get channel name collisions.  I'd consider naming the
     * channels independently of the fd, but this makes debugging easier.
     */
    int fdBusy;

    /* 
     * stdinout and stderr never go away so that our internal refs to them
     * don't have to be invalidated.  Having to worry about invalidating them
     * would be a major pain.  */
    int keepForever;

    /*  Remember that "reserved" esPtrs are no longer in use. */
    int valid;
    
    struct ExpState *nextPtr;	/* Pointer to next file in list of all
				 * file channels. */
} ExpState;

#define EXP_SPAWN_ID_BAD	((ExpState *) NULL)

#define EXP_TIME_INFINITY	-1

#define EXP_TEMPORARY	1	/* expect */
#define EXP_PERMANENT	2	/* expect_after, expect_before, expect_bg */

#define EXP_DIRECT	1
#define EXP_INDIRECT	2


#define exp_new(x)	(x *)malloc(sizeof(x))

struct exp_state_list {
	ExpState *esPtr;
	struct exp_state_list *next;
};

/* describes a -i flag */
struct exp_i {
	int cmdtype;	/* EXP_CMD_XXX.  When an indirect update is */
			/* triggered by Tcl, this helps tell us in what */
			/* exp_i list to look in. */
	int direct;	/* if EXP_DIRECT, then the spawn ids have been given */
			/* literally, else indirectly through a variable */
	int duration;	/* if EXP_PERMANENT, char ptrs here had to be */
			/* malloc'd because Tcl command line went away - */
			/* i.e., in expect_before/after */
	char *variable;
	char *value;	/* if type == direct, this is the string that the */
			/* user originally supplied to the -i flag.  It may */
			/* lose relevance as the fd_list is manipulated */
			/* over time.  If type == direct, this is  the */
			/* cached value of variable use this to tell if it */
			/* has changed or not, and ergo whether it's */
			/* necessary to reparse. */

	int ecount;	/* # of ecases this is used by */

	struct exp_state_list *state_list;
	struct exp_i *next;
};

/*
 * definitions for creating commands
 */

#define EXP_NOPREFIX	1	/* don't define with "exp_" prefix */
#define EXP_REDEFINE	2	/* stomp on old commands with same name */

#define exp_proc(cmdproc)   NULL, cmdproc
#define exp_deleteProc	    ((Tcl_CmdDeleteProc *) NULL)
#define exp_deleteObjProc   ((Tcl_CmdDeleteProc *) NULL)

TCL_EXTERNC int expect_key;

struct exp_cmd_data {
	CONST char	*name;
	Tcl_ObjCmdProc	*objproc;
	Tcl_CmdProc	*proc;
	ClientData	data;
	int 		flags;
};

#define EXP_CMDINFO_CLOSE  "expect/cmdinfo/close"
#define EXP_CMDINFO_RETURN "expect/cmdinfo/return"

TCL_EXTERNC void (*exp_event_exit) (Tcl_Interp *);

TCL_EXTERNC Tcl_FileProc	    exp_channelhandler;
TCL_EXTERNC Tcl_TimerProc    exp_timehandler;
TCL_EXTERNC Tcl_ExitProc	    exp_pty_exit_for_tcl;
TCL_EXTERNC char *exp_pty_slave_name;
TCL_EXTERNC char *exp_pty_error;
TCL_EXTERNC int exp_configure_count;	/* # of times descriptors have been closed */
TCL_EXTERNC char *exp_onexit_action;
TCL_EXTERNC int exp_disconnected;    /* proc. disc'd from controlling tty */
TCL_EXTERNC int exp_nostack_dump;
TCL_EXTERNC int exp_default_parity;
TCL_EXTERNC int exp_default_match_max;
TCL_EXTERNC int exp_default_rm_nulls;
TCL_EXTERNC int exp_default_close_on_eof;

/* abstraction for a file descriptor (int on Unix, HANDLE on windows) */
typedef struct exp_file_ *exp_file;
TCL_EXTERNC exp_file exp_dev_tty;

TCL_EXTERNC int exp_ioctled_devtty;
TCL_EXTERNC int exp_stdin_is_tty;
TCL_EXTERNC int exp_stdout_is_tty;
//void exp_tty_raw();
//void exp_tty_echo();
//void exp_tty_break();
//int exp_tty_raw_noecho();
//int exp_israw();
//int exp_isecho();

//void exp_tty_set();
//int exp_tty_set_simple();
//int exp_tty_get_simple();

#ifdef __WIN32__
typedef HANDLE exp_tty;  // <- FIXME!!!!
#else
typedef struct TERMINAL exp_tty;  // <- move this to expUnixPort.h
#endif
extern exp_tty exp_tty_original;
extern exp_tty exp_tty_current;
extern exp_tty exp_tty_cooked;

extern int exp_cmdlinecmds;
extern int exp_interactive;
//extern FILE *exp_cmdfile;
extern CONST char *exp_cmdfilename;
extern int exp_getpid;	/* pid of Expect itself */
extern int exp_buffer_command_input;
extern int exp_tcl_debugger_available;
TCL_EXTERNC Tcl_Interp *exp_interp;
#define Exp_Init Expect_Init
TCL_EXTERNC void	(*exp_app_exit) (Tcl_Interp *);


/* yes, I have a weak mind */
#define streq(x,y)	(0 == strcmp((x),(y)))

/* not exported */
TCL_EXTERNC int	getopt(int argc, const char *argv[], char *ostr);
TCL_EXTERNC Tcl_ChannelType ExpSpawnChannelType;
TCL_EXTERNC Tcl_ChannelType ExpChannelType;

#include "expIntDecls.h"
#include "expIntPlatDecls.h"

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLIMPORT

#endif /* _EXPINT */
