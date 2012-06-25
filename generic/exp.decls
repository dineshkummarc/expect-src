# ----------------------------------------------------------------------------
# exp.decls --
#
#	This file contains the declarations for all supported public and
#	private functions that are exported by the Expect library via the
#	Stubs table.  This file is used to generate the expDecls.h,
#	expPlatDecls.h, expIntDecls.h, expStubInit.c and expStubLib.c files.
#
# ----------------------------------------------------------------------------
#
# Written by: Don Libes, libes@cme.nist.gov, NIST, 12/3/90
# 
# Design and implementation of this program was paid for by U.S. tax
# dollars.  Therefore it is public domain.  However, the author and NIST
# would appreciate credit if this program or parts of it are used.
# 
# Copyright (c) 2001-2002 Telindustrie, LLC
# Copyright (c) 2003 ActiveState Corporation
#	Work by David Gravereaux <davygrvy@pobox.com> for any Win32 OS.
#	Based on work by Gordon Chaffee <chaffee@bmrc.berkeley.edu>
#
# ----------------------------------------------------------------------------
# URLs:    http://expect.nist.gov/
#	   http://expect.sf.net/
#	   http://bmrc.berkeley.edu/people/chaffee/expectnt.html
# ----------------------------------------------------------------------------
# RCS: @(#) $Id: exp.decls,v 1.1.4.7 2002/03/11 06:52:53 davygrvy Exp $
# ----------------------------------------------------------------------------

library exp

# Define the tcl interface with several sub interfaces:
#     expPlat	 - platform specific public
#     expInt	 - generic private
#     expIntPlat - platform specific private

interface exp
hooks {expPlat expInt expIntPlat}

# Declare each of the functions in the public Expect interface.  Note that
# the an index should never be reused for a different function in order
# to preserve backwards compatibility.

# ----------------------------------------------------------------------------

declare 0 generic {
    int Expect_Init (Tcl_Interp *interp)
}
# Umm.. NO!, but leave the slot just the same.
#declare 1 generic {
#    int Expect_SafeInit (Tcl_Interp *interp)
#}

# ----------------------------------------------------------------------------
# The Tcl_[Obj]CmdProcs.
#
# I'm not sure _exactly_ why, but I think these should be in the Stubs table
# as they are functions.
# ------------------------------------------------------------------------

declare 3 generic {
    int Exp_CloseObjCmd (ClientData clientData, Tcl_Interp *interp,
	int objc, struct Tcl_Obj *CONST objv[])
}
declare 4 generic {
    int Exp_DisconnectCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, CONST84 char *argv[])
}
declare 5 generic {
    int Exp_ExitCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, CONST84 char *argv[])
}
declare 6 generic {
    int Exp_ExpContinueCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, CONST84 char *argv[])
}
declare 7 generic {
    int Exp_ExpectObjCmd (ClientData clientData, Tcl_Interp *interp,
	int objc, struct Tcl_Obj *CONST objv[])
}
declare 8 generic {
    int Exp_ExpectGlobalObjCmd (ClientData clientData, Tcl_Interp *interp,
	int objc, struct Tcl_Obj *CONST objv[])
}
declare 9 generic {
    int Exp_ExpInternalCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, CONST84 char *argv[])
}
declare 10 generic {
    int Exp_ExpPidCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, CONST84 char *argv[])
}
declare 11 generic {
    int Exp_ExpVersionCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, CONST84 char *argv[])
}
declare 12 generic {
    int Exp_ForkCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, CONST84 char *argv[])
}
declare 13 generic {
    int Exp_GetpidDeprecatedCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, CONST84 char *argv[])
}
#declare 14 generic {
#    int Exp_InterpreterCmd (ClientData clientData, Tcl_Interp *interp,
#	int argc, CONST84 char *argv[])
#}
declare 15 generic {
    int Exp_InterpreterObjCmd (ClientData clientData,
	Tcl_Interp *interp, int objc, struct Tcl_Obj * CONST objv[])
}
declare 16 generic {
    int Exp_InterReturnObjCmd (ClientData clientData,
	Tcl_Interp *interp, int objc, struct Tcl_Obj * CONST objv[])
}
declare 17 generic {
    int Exp_KillCmd (ClientData clientData,Tcl_Interp *interp,
	int argc, CONST84 char *argv[])
}
declare 18 generic {
    int Exp_LogFileCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, CONST84 char *argv[])
}
declare 19 generic {
    int Exp_LogUserCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, CONST84 char *argv[])
}
declare 20 generic {
    int Exp_MatchMaxCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, CONST84 char *argv[])
}
declare 21 generic {
    int Exp_OpenCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, CONST84 char *argv[])
}
declare 22 generic {
    int Exp_OverlayCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, CONST84 char *argv[])
}
declare 23 generic {
    int Exp_ParityCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, CONST84 char *argv[])
}
declare 24 generic {
    int Exp_Prompt1ObjCmd (ClientData clientData, Tcl_Interp *interp,
	int objc, struct Tcl_Obj * CONST objv[])
}
declare 25 generic {
    int Exp_Prompt2ObjCmd (ClientData clientData, Tcl_Interp *interp,
	int objc, struct Tcl_Obj * CONST objv[])
}
declare 26 generic {
    int Exp_RemoveNullsCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, CONST84 char *argv[])
}
declare 27 generic {
    int Exp_SendLogCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, CONST84 char *argv[])
}
declare 28 generic {
    int Exp_SendObjCmd (ClientData clientData, Tcl_Interp *interp,
	int objc, struct Tcl_Obj * CONST objv[])
}
declare 29 generic {
    int Exp_SleepCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, CONST84 char *argv[])
}
declare 30 generic {
    int Exp_SpawnObjCmd  (ClientData clientData, Tcl_Interp *interp,
	int objc, struct Tcl_Obj * CONST objv[])
}
declare 31 generic {
    int Exp_StraceCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, CONST84 char *argv[])
}
declare 32 generic {
    int Exp_SttyCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, CONST84 char *argv[])
}
declare 33 generic {
    int Exp_SystemCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, CONST84 char *argv[])
}
declare 34 generic {
    int Exp_TimestampCmd  (ClientData clientData, Tcl_Interp *interp,
	int argc, CONST84 char *argv[])
}
declare 35 generic {
    int Exp_TrapCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, CONST84 char *argv[])
}
declare 36 generic {
    int Exp_WaitCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, CONST84 char *argv[])
}
declare 37 generic {
    int Exp_CloseOnEofCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, CONST84 char *argv[])
}

# ----------------------------------------------------------------------------

declare 40 generic {
    Tcl_Channel Exp_CreateSpawnChannel(Tcl_Interp *interp,
	Exp_SpawnOptionSet *opts, int objc, struct Tcl_Obj * CONST objv[],
	unsigned long *pid, Tcl_Pid *theUglyHandleHackJob)
}


# ----------------------------------------------------------------------------
interface expInt

# ----------------------------------------------------------------------------
# expect_tcl.h ->
declare 0 generic {
    void exp_parse_argv (Tcl_Interp *interp, int argc, CONST84 char *argv[])
}
declare 1 generic {
    int	exp_interpreter (Tcl_Interp *interp, Tcl_Obj *eofObj)
}
#declare 2 generic {
#    int exp_interpret_cmdfile (Tcl_Interp *interp, FILE *notXplatform)
#}
declare 3 generic {
    int	exp_interpret_cmdfilename (Tcl_Interp *interp, CONST char *filename)
}
declare 4 generic {
    void exp_interpret_rcfiles (Tcl_Interp *interp, int my_rc, int sys_rc)
}
declare 5 generic {
    CONST char *exp_cook (CONST char *s, int *len)
}
declare 6 generic {
    void expCloseOnExec (int fd)
}
declare 7 generic {
    void exp_exit_handlers (ClientData clientData)
}
declare 8 generic {
    void exp_error (Tcl_Interp *interp, ...)
}

#declare 0 generic {
#    void ExpSpawnInit (void)
#}
#declare 1 generic {
#    void exp_background_channelhandlers_run_all (void)
#}

# Old expect.h ->
#declare 15 generic {
#    void exp_slave_control (int master, int control)
#}
#declare 16 generic {
#    void exp_debuglog (CONST char *fmt, ...)
#}
#declare 17 generic {
#    void exp_errorlog (CONST char *fmt, ...)
#}
#declare 18 generic {
#    int exp_disconnect (void)
#}

# -----------------------------------------------------------------------
# exp_command.h ->

declare 20 generic {
    CONST char *exp_get_var (Tcl_Interp *interp, CONST char *var)
}
declare 21 generic {
    int	exp_one_arg_braced (Tcl_Obj *objPtr)
}
declare 22 generic {
    int	exp_eval_with_one_arg (ClientData clientData, Tcl_Interp *interp,
	struct Tcl_Obj * CONST objv[])
}
#declare 23 generic {
#    void exp_lowmemcpy (char *, char *, int)
#}
declare 24 generic {
    int exp_flageq_code (CONST char *flag, CONST char *string, int minlen)
}
declare 25 generic {
    void expAdjust (ExpState *esPtr)
}
declare 26 generic {
    void exp_buffer_shuffle (Tcl_Interp *interp, ExpState *esPtr, int save_flags,
	CONST char *array_name, CONST char *caller_name)
}
declare 27 generic {
    int	exp_close (Tcl_Interp *interp, ExpState *esPtr)
}
declare 28 generic {
    void exp_close_all (Tcl_Interp *interp)
}
#declare 29 generic {
#    void exp_ecmd_remove_fd_direct_and_indirect (Tcl_Interp *interp, int)
#}
declare 30 generic {
    void exp_trap_on (int master)
}
declare 31 generic {
    int exp_trap_off (CONST char *name)
}
declare 32 generic {
    void exp_strftime (CONST char *format, CONST struct tm *timeptr, Tcl_DString *dstring)
}
declare 33 generic {
    void exp_init_pty (Tcl_Interp *interp)
}
declare 34 generic {
    void exp_pty_exit (void)
}
declare 35 generic {
    void exp_init_tty (void)
}
declare 36 generic {
    void exp_init_stdio (void)
}
declare 37 generic {
    void exp_init_spawn_ids (Tcl_Interp *interp)
}
declare 38 generic {
    void exp_init_spawn_id_vars (Tcl_Interp *interp)
}
declare 39 generic {
    void exp_init_trap (void)
}
#declare 40 generic {
#    void exp_init_send (void)
#}
declare 41 generic {
    void exp_init_unit_random (void)
}
declare 42 generic {
    void exp_init_sig (void)
}
declare 43 generic {
    void expChanInit (void)
}
declare 44 generic {
    int expChannelCountGet (void)
}
declare 45 generic {
    int exp_tcl2_returnvalue (int x)
}
declare 46 generic {
    int	exp_2tcl_returnvalue (int x)
}
declare 47 generic {
    void exp_rearm_sigchld (Tcl_Interp *interp)
}
declare 48 generic {
    int	exp_string_to_signal (Tcl_Interp *interp, CONST char *s)
}
declare 49 generic {
    struct exp_i *exp_new_i_complex (Tcl_Interp *interp, CONST char *arg, int duration, Tcl_VarTraceProc *updateproc)
}
declare 50 generic {
    struct exp_i *exp_new_i_simple (ExpState *esPtr, int duration)
}
declare 51 generic {
    struct exp_state_list *exp_new_state (ExpState *esPtr)
}
declare 52 generic {
    void exp_free_i (Tcl_Interp *interp, struct exp_i *i, Tcl_VarTraceProc *updateproc)
}
declare 53 generic {
    void exp_free_state (struct exp_state_list *fd_first)
}
declare 54 generic {
    void exp_free_state_single (struct exp_state_list *fd)
}
declare 55 generic {
    int	exp_i_update (Tcl_Interp *interp, struct exp_i *i)
}
declare 56 generic {
    void exp_create_commands (Tcl_Interp *interp, struct exp_cmd_data *c)
}
declare 57 generic {
    void exp_init_main_cmds (Tcl_Interp *interp)
}
declare 58 generic {
    void exp_init_expect_cmds (Tcl_Interp *interp)
}
declare 59 generic {
    void exp_init_most_cmds (Tcl_Interp *interp)
}
declare 60 generic {
    void exp_init_trap_cmds (Tcl_Interp *interp)
}
#declare 61 generic {
#    void exp_init_interact_cmds (Tcl_Interp *interp)
#}
declare 62 generic {
    void exp_init_tty_cmds(Tcl_Interp *interp)
}
declare 63 generic {
    ExpState *expStateCheck (Tcl_Interp *interp, ExpState *esPtr, int open, int adjust, CONST char *msg)
}
declare 64 generic {
    ExpState *expStateCurrent (Tcl_Interp *interp, int opened, int adjust, int any)
}
declare 65 generic {
    ExpState *expStateFromChannelName (Tcl_Interp *interp, CONST char *name, int opened, int adjust, int any, CONST char *msg)
}
declare 66 generic {
    void expStateFree (ExpState *esPtr)
}
#declare 67 generic {
#    ExpState *expCreateChannel (Tcl_Interp *interp, int fdin, int fdout, int pid)
#}
declare 68 generic {
    ExpState *expWaitOnAny (void)
}
declare 69 generic {
    ExpState *expWaitOnOne (void)
}
declare 70 generic {
    void expExpectVarsInit (void)
}
declare 71 generic {
    int expStateAnyIs (ExpState *esPtr)
}
declare 72 generic {
    int	expDevttyIs (ExpState *esPtr)
}
declare 73 generic {
    int expStdinOutIs (ExpState *esPtr)
}
declare 74 generic {
    ExpState *expStdinoutGet (void)
}
declare 75 generic {
    ExpState *expDevttyGet (void)
}
declare 76 generic {
    int expSizeGet (ExpState *esPtr)
}
declare 77 generic {
    int expSizeZero (ExpState *esPtr)
}
declare 78 generic {
    void exp_ecmd_remove_state_direct_and_indirect (Tcl_Interp *interp,
	ExpState *esPtr)
}
declare 79 generic {
    int expWriteChars (ExpState *esPtr, CONST char *str, int len)
}

### ---------------------------------------------------------------------
# exp_log.h ->

declare 85 generic {
    void expErrorLog (CONST char *arg1, ...)
}
declare 86 generic {
    void expErrorLogU (CONST char *buf)
}
declare 87 generic {
    void expStdoutLog (int arg1, ...)
}
declare 88 generic {
    void expStdoutLogU (CONST char *buf, int force_stdout)
}
declare 89 generic {
    void expDiagInit (void)
}
declare 90 generic {
    int expDiagChannelOpen (Tcl_Interp *interp, CONST char *filename)
}
declare 91 generic {
    Tcl_Channel expDiagChannelGet (void)
}
declare 92 generic {
    void expDiagChannelClose (Tcl_Interp *interp)
}
declare 93 generic {
    CONST char *expDiagFilename (void)
}
declare 94 generic {
    int expDiagToStderrGet (void)
}
declare 95 generic {
    void expDiagToStderrSet (int val)
}
declare 96 generic {
    void expDiagWriteBytes (CONST char *str, int len)
}
declare 97 generic {
    void expDiagWriteChars (CONST char *str, int len)
}
declare 98 generic {
    void expDiagWriteObj (Tcl_Obj *obj)
}
declare 99 generic {
    void expDiagLog (CONST char *arg1, ...)
}
declare 100 generic {
    void expDiagLogU (CONST char *str)
}
declare 101 generic {
    CONST char *expPrintify (CONST char *s)
}
declare 102 generic {
    CONST char *expPrintifyObj (Tcl_Obj *obj)
}
declare 103 generic {
    void expLogInit (void)
}
declare 104 generic {
    int	expLogChannelOpen (Tcl_Interp *interp, CONST char *filename, int append)
}
declare 105 generic {
    Tcl_Channel expLogChannelGet (void)
}
declare 106 generic {
    int expLogChannelSet (Tcl_Interp *interp, CONST char *name)
}
declare 107 generic {
    void expLogChannelClose (Tcl_Interp *interp)
}
declare 108 generic {
    char *expLogFilenameGet (void)
}
declare 109 generic {
    void expLogAppendSet (int app)
}
declare 110 generic {
    int expLogAppendGet (void)
}
declare 111 generic {
    void expLogLeaveOpenSet (int app)
}
declare 112 generic {
    int expLogLeaveOpenGet (void)
}
declare 113 generic {
    void expLogAllSet (int app)
}
declare 114 generic {
    int expLogAllGet (void)
}
declare 115 generic {
    void expLogToStdoutSet (int app)
}
declare 116 generic {
    int expLogToStdoutGet (void)
}
declare 117 generic {
    void expLogDiagU (CONST char *buf)
}
declare 118 generic {
    int expWriteBytesAndLogIfTtyU (ExpState *esPtr, CONST char *buf, int lenBytes)
}
declare 119 generic {
    int expLogUserGet (void)
}
declare 120 generic {
    void expLogUserSet (int logUser)
}
declare 121 generic {
    void expLogInteractionU (ExpState *esPtr, CONST char *buf)
}

### ---------------------------------------------------------------------
# exp_event.h ->
declare 125 generic {
    int exp_get_next_event (Tcl_Interp *interp, ExpState **esPtrs, int n,
	ExpState **esPtrOut, int timeout, int key)
}
declare 126 generic {
    int exp_get_next_event_info (Tcl_Interp *interp, ExpState *esPtr)
}
declare 127 generic {
    int exp_dsleep (Tcl_Interp *interp, double sec)
}
declare 128 generic {
    void exp_init_event (void)
}
#declare 129 generic {
#    void exp_event_disarm (ExpState *,Tcl_FileProc *)
#}
declare 130 generic {
    void exp_event_disarm_bg (ExpState *esPtr)
}
declare 131 generic {
    void exp_event_disarm_fg (ExpState *esPtr)
}
declare 132 generic {
    void exp_arm_background_channelhandler (ExpState *esPtr)
}
declare 133 generic {
    void exp_disarm_background_channelhandler (ExpState *esPtr)
}
declare 134 generic {
    void exp_disarm_background_channelhandler_force (ExpState *esPtr)
}
declare 135 generic {
    void exp_unblock_background_channelhandler (ExpState *esPtr)
}
declare 136 generic {
    void exp_block_background_channelhandler (ExpState *esPtr)
}
declare 137 generic {
    void exp_background_channelhandler (ClientData clientData, int mask)
}

# -----------------------------------------------------------------------
# exp_tty.h ->
declare 140 generic {
    void exp_tty_set (Tcl_Interp *interp, exp_tty *tty, int raw, int echo)
}
declare 141 generic {
    int exp_tty_cooked_echo (Tcl_Interp *interp, exp_tty *tty_old,
	int *was_raw, int *was_echo)
}

# -----------------------------------------------------------------------
# exp_glob.h (???) ->
declare 145 generic {
    int Exp_StringCaseMatch (CONST char *string, CONST char *pattern,
	int nocase, int *offset)
}

# -----------------------------------------------------------------------
# exp_int.h ->

#declare 150 generic {
#    void exp_console_set (void)
#}
declare 151 generic {
    void expDiagLogPtrSet (expDiagLogProc *func)
}
declare 152 generic {
    void expDiagLogPtr (CONST char *str)
}
declare 153 generic {
    void expDiagLogPtrX (CONST char *fmt, int num)
}
declare 154 generic {
    void expDiagLogPtrStr (CONST char *fmt, CONST char *str1)
}
declare 155 generic {
    void expDiagLogPtrStrStr (CONST char *fmt, CONST char *str1,
	CONST char *str2)
}
declare 156 generic {
    void expErrnoMsgSet (expErrnoMsgProc *errMsgProc)
}
declare 157 generic {
    CONST char * expErrnoMsg (int posixErrCode)
}

declare 160 generic {
    Tcl_Channel Exp_CreateExpChannel (Tcl_Interp *interp,
	Tcl_Channel chan, int pid, Tcl_Pid tclPid, ExpState **esOut)
}
declare 161 generic {
    Tcl_Channel Exp_CreatePairChannel (Tcl_Interp *interp, Tcl_Channel chanIn,
	Tcl_Channel chanOut, CONST char *chanName)
}

# -----------------------------------------------------------------------
interface expPlat

declare 0 win {
    int Exp_WinApplicationType (CONST char *originalName,
	Tcl_DString *fullName)
}
declare 1 win {
    void Exp_WinBuildCommandLine (CONST char *executable, int objc,
	struct Tcl_Obj * const objv[], Tcl_DString *linePtr)
}
declare 2 win {
    CONST char *Exp_Win32ErrId (unsigned long errorCode)
}
declare 3 win {
    CONST char *Exp_Win32ErrMsg (unsigned long errorCode, ...)
}
declare 4 win {
    CONST char *Exp_Win32ErrMsgVA (unsigned long errorCode, va_list argList)
}
declare 5 win {
    CONST char *Exp_Win32Error(Tcl_Interp *interp, ...)
}

# ----------------------------------------------------------------------------
interface expIntPlat

declare 0 win {
    int ExpWinInit(Tcl_Interp *interp)
}
declare 1 win {
    CONST char *ExpWinErrId (DWORD errorCode)
}
declare 2 win {
    CONST char *ExpWinErrMsg (DWORD errorCode, ...)
}
declare 3 win {
    CONST char *ExpWinErrMsgVA (DWORD errorCode, va_list argList)
}
declare 4 win {
    CONST char *ExpWinError(Tcl_Interp *interp, ...)
}
