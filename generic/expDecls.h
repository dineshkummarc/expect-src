/* ----------------------------------------------------------------------------
 * expDecls.h --
 *
 *	Declarations of functions in the platform independent public
 *	Expect API.
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
 * RCS: @(#) $Id: expDecls.h,v 1.1.4.4 2002/06/18 23:13:42 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#ifndef _EXPDECLS
#define _EXPDECLS

/*
 * WARNING: This file is automatically generated by the $(TCLROOT)/tools/genStubs.tcl
 * script.  Any modifications to the function declarations below should be made
 * in the generic/exp.decls script.
 */

/* !BEGIN!: Do not edit below this line. */

/*
 * Exported function declarations:
 */

#ifndef Expect_Init_TCL_DECLARED
#define Expect_Init_TCL_DECLARED
/* 0 */
TCL_EXTERN(int)		Expect_Init _ANSI_ARGS_((Tcl_Interp * interp));
#endif
/* Slot 1 is reserved */
/* Slot 2 is reserved */
#ifndef Exp_CloseObjCmd_TCL_DECLARED
#define Exp_CloseObjCmd_TCL_DECLARED
/* 3 */
TCL_EXTERN(int)		Exp_CloseObjCmd _ANSI_ARGS_((ClientData clientData, 
				Tcl_Interp * interp, int objc, 
				struct Tcl_Obj *CONST objv[]));
#endif
#ifndef Exp_DisconnectCmd_TCL_DECLARED
#define Exp_DisconnectCmd_TCL_DECLARED
/* 4 */
TCL_EXTERN(int)		Exp_DisconnectCmd _ANSI_ARGS_((ClientData clientData, 
				Tcl_Interp * interp, int argc, 
				CONST84 char * argv[]));
#endif
#ifndef Exp_ExitCmd_TCL_DECLARED
#define Exp_ExitCmd_TCL_DECLARED
/* 5 */
TCL_EXTERN(int)		Exp_ExitCmd _ANSI_ARGS_((ClientData clientData, 
				Tcl_Interp * interp, int argc, 
				CONST84 char * argv[]));
#endif
#ifndef Exp_ExpContinueCmd_TCL_DECLARED
#define Exp_ExpContinueCmd_TCL_DECLARED
/* 6 */
TCL_EXTERN(int)		Exp_ExpContinueCmd _ANSI_ARGS_((
				ClientData clientData, Tcl_Interp * interp, 
				int argc, CONST84 char * argv[]));
#endif
#ifndef Exp_ExpectObjCmd_TCL_DECLARED
#define Exp_ExpectObjCmd_TCL_DECLARED
/* 7 */
TCL_EXTERN(int)		Exp_ExpectObjCmd _ANSI_ARGS_((ClientData clientData, 
				Tcl_Interp * interp, int objc, 
				struct Tcl_Obj *CONST objv[]));
#endif
#ifndef Exp_ExpectGlobalObjCmd_TCL_DECLARED
#define Exp_ExpectGlobalObjCmd_TCL_DECLARED
/* 8 */
TCL_EXTERN(int)		Exp_ExpectGlobalObjCmd _ANSI_ARGS_((
				ClientData clientData, Tcl_Interp * interp, 
				int objc, struct Tcl_Obj *CONST objv[]));
#endif
#ifndef Exp_ExpInternalCmd_TCL_DECLARED
#define Exp_ExpInternalCmd_TCL_DECLARED
/* 9 */
TCL_EXTERN(int)		Exp_ExpInternalCmd _ANSI_ARGS_((
				ClientData clientData, Tcl_Interp * interp, 
				int argc, CONST84 char * argv[]));
#endif
#ifndef Exp_ExpPidCmd_TCL_DECLARED
#define Exp_ExpPidCmd_TCL_DECLARED
/* 10 */
TCL_EXTERN(int)		Exp_ExpPidCmd _ANSI_ARGS_((ClientData clientData, 
				Tcl_Interp * interp, int argc, 
				CONST84 char * argv[]));
#endif
#ifndef Exp_ExpVersionCmd_TCL_DECLARED
#define Exp_ExpVersionCmd_TCL_DECLARED
/* 11 */
TCL_EXTERN(int)		Exp_ExpVersionCmd _ANSI_ARGS_((ClientData clientData, 
				Tcl_Interp * interp, int argc, 
				CONST84 char * argv[]));
#endif
#ifndef Exp_ForkCmd_TCL_DECLARED
#define Exp_ForkCmd_TCL_DECLARED
/* 12 */
TCL_EXTERN(int)		Exp_ForkCmd _ANSI_ARGS_((ClientData clientData, 
				Tcl_Interp * interp, int argc, 
				CONST84 char * argv[]));
#endif
#ifndef Exp_GetpidDeprecatedCmd_TCL_DECLARED
#define Exp_GetpidDeprecatedCmd_TCL_DECLARED
/* 13 */
TCL_EXTERN(int)		Exp_GetpidDeprecatedCmd _ANSI_ARGS_((
				ClientData clientData, Tcl_Interp * interp, 
				int argc, CONST84 char * argv[]));
#endif
/* Slot 14 is reserved */
#ifndef Exp_InterpreterObjCmd_TCL_DECLARED
#define Exp_InterpreterObjCmd_TCL_DECLARED
/* 15 */
TCL_EXTERN(int)		Exp_InterpreterObjCmd _ANSI_ARGS_((
				ClientData clientData, Tcl_Interp * interp, 
				int objc, struct Tcl_Obj * CONST objv[]));
#endif
#ifndef Exp_InterReturnObjCmd_TCL_DECLARED
#define Exp_InterReturnObjCmd_TCL_DECLARED
/* 16 */
TCL_EXTERN(int)		Exp_InterReturnObjCmd _ANSI_ARGS_((
				ClientData clientData, Tcl_Interp * interp, 
				int objc, struct Tcl_Obj * CONST objv[]));
#endif
#ifndef Exp_KillCmd_TCL_DECLARED
#define Exp_KillCmd_TCL_DECLARED
/* 17 */
TCL_EXTERN(int)		Exp_KillCmd _ANSI_ARGS_((ClientData clientData, 
				Tcl_Interp * interp, int argc, 
				CONST84 char * argv[]));
#endif
#ifndef Exp_LogFileCmd_TCL_DECLARED
#define Exp_LogFileCmd_TCL_DECLARED
/* 18 */
TCL_EXTERN(int)		Exp_LogFileCmd _ANSI_ARGS_((ClientData clientData, 
				Tcl_Interp * interp, int argc, 
				CONST84 char * argv[]));
#endif
#ifndef Exp_LogUserCmd_TCL_DECLARED
#define Exp_LogUserCmd_TCL_DECLARED
/* 19 */
TCL_EXTERN(int)		Exp_LogUserCmd _ANSI_ARGS_((ClientData clientData, 
				Tcl_Interp * interp, int argc, 
				CONST84 char * argv[]));
#endif
#ifndef Exp_MatchMaxCmd_TCL_DECLARED
#define Exp_MatchMaxCmd_TCL_DECLARED
/* 20 */
TCL_EXTERN(int)		Exp_MatchMaxCmd _ANSI_ARGS_((ClientData clientData, 
				Tcl_Interp * interp, int argc, 
				CONST84 char * argv[]));
#endif
#ifndef Exp_OpenCmd_TCL_DECLARED
#define Exp_OpenCmd_TCL_DECLARED
/* 21 */
TCL_EXTERN(int)		Exp_OpenCmd _ANSI_ARGS_((ClientData clientData, 
				Tcl_Interp * interp, int argc, 
				CONST84 char * argv[]));
#endif
#ifndef Exp_OverlayCmd_TCL_DECLARED
#define Exp_OverlayCmd_TCL_DECLARED
/* 22 */
TCL_EXTERN(int)		Exp_OverlayCmd _ANSI_ARGS_((ClientData clientData, 
				Tcl_Interp * interp, int argc, 
				CONST84 char * argv[]));
#endif
#ifndef Exp_ParityCmd_TCL_DECLARED
#define Exp_ParityCmd_TCL_DECLARED
/* 23 */
TCL_EXTERN(int)		Exp_ParityCmd _ANSI_ARGS_((ClientData clientData, 
				Tcl_Interp * interp, int argc, 
				CONST84 char * argv[]));
#endif
#ifndef Exp_Prompt1ObjCmd_TCL_DECLARED
#define Exp_Prompt1ObjCmd_TCL_DECLARED
/* 24 */
TCL_EXTERN(int)		Exp_Prompt1ObjCmd _ANSI_ARGS_((ClientData clientData, 
				Tcl_Interp * interp, int objc, 
				struct Tcl_Obj * CONST objv[]));
#endif
#ifndef Exp_Prompt2ObjCmd_TCL_DECLARED
#define Exp_Prompt2ObjCmd_TCL_DECLARED
/* 25 */
TCL_EXTERN(int)		Exp_Prompt2ObjCmd _ANSI_ARGS_((ClientData clientData, 
				Tcl_Interp * interp, int objc, 
				struct Tcl_Obj * CONST objv[]));
#endif
#ifndef Exp_RemoveNullsCmd_TCL_DECLARED
#define Exp_RemoveNullsCmd_TCL_DECLARED
/* 26 */
TCL_EXTERN(int)		Exp_RemoveNullsCmd _ANSI_ARGS_((
				ClientData clientData, Tcl_Interp * interp, 
				int argc, CONST84 char * argv[]));
#endif
#ifndef Exp_SendLogCmd_TCL_DECLARED
#define Exp_SendLogCmd_TCL_DECLARED
/* 27 */
TCL_EXTERN(int)		Exp_SendLogCmd _ANSI_ARGS_((ClientData clientData, 
				Tcl_Interp * interp, int argc, 
				CONST84 char * argv[]));
#endif
#ifndef Exp_SendObjCmd_TCL_DECLARED
#define Exp_SendObjCmd_TCL_DECLARED
/* 28 */
TCL_EXTERN(int)		Exp_SendObjCmd _ANSI_ARGS_((ClientData clientData, 
				Tcl_Interp * interp, int objc, 
				struct Tcl_Obj * CONST objv[]));
#endif
#ifndef Exp_SleepCmd_TCL_DECLARED
#define Exp_SleepCmd_TCL_DECLARED
/* 29 */
TCL_EXTERN(int)		Exp_SleepCmd _ANSI_ARGS_((ClientData clientData, 
				Tcl_Interp * interp, int argc, 
				CONST84 char * argv[]));
#endif
#ifndef Exp_SpawnObjCmd_TCL_DECLARED
#define Exp_SpawnObjCmd_TCL_DECLARED
/* 30 */
TCL_EXTERN(int)		Exp_SpawnObjCmd _ANSI_ARGS_((ClientData clientData, 
				Tcl_Interp * interp, int objc, 
				struct Tcl_Obj * CONST objv[]));
#endif
#ifndef Exp_StraceCmd_TCL_DECLARED
#define Exp_StraceCmd_TCL_DECLARED
/* 31 */
TCL_EXTERN(int)		Exp_StraceCmd _ANSI_ARGS_((ClientData clientData, 
				Tcl_Interp * interp, int argc, 
				CONST84 char * argv[]));
#endif
#ifndef Exp_SttyCmd_TCL_DECLARED
#define Exp_SttyCmd_TCL_DECLARED
/* 32 */
TCL_EXTERN(int)		Exp_SttyCmd _ANSI_ARGS_((ClientData clientData, 
				Tcl_Interp * interp, int argc, 
				CONST84 char * argv[]));
#endif
#ifndef Exp_SystemCmd_TCL_DECLARED
#define Exp_SystemCmd_TCL_DECLARED
/* 33 */
TCL_EXTERN(int)		Exp_SystemCmd _ANSI_ARGS_((ClientData clientData, 
				Tcl_Interp * interp, int argc, 
				CONST84 char * argv[]));
#endif
#ifndef Exp_TimestampCmd_TCL_DECLARED
#define Exp_TimestampCmd_TCL_DECLARED
/* 34 */
TCL_EXTERN(int)		Exp_TimestampCmd _ANSI_ARGS_((ClientData clientData, 
				Tcl_Interp * interp, int argc, 
				CONST84 char * argv[]));
#endif
#ifndef Exp_TrapCmd_TCL_DECLARED
#define Exp_TrapCmd_TCL_DECLARED
/* 35 */
TCL_EXTERN(int)		Exp_TrapCmd _ANSI_ARGS_((ClientData clientData, 
				Tcl_Interp * interp, int argc, 
				CONST84 char * argv[]));
#endif
#ifndef Exp_WaitCmd_TCL_DECLARED
#define Exp_WaitCmd_TCL_DECLARED
/* 36 */
TCL_EXTERN(int)		Exp_WaitCmd _ANSI_ARGS_((ClientData clientData, 
				Tcl_Interp * interp, int argc, 
				CONST84 char * argv[]));
#endif
#ifndef Exp_CloseOnEofCmd_TCL_DECLARED
#define Exp_CloseOnEofCmd_TCL_DECLARED
/* 37 */
TCL_EXTERN(int)		Exp_CloseOnEofCmd _ANSI_ARGS_((ClientData clientData, 
				Tcl_Interp * interp, int argc, 
				CONST84 char * argv[]));
#endif
/* Slot 38 is reserved */
/* Slot 39 is reserved */
#ifndef Exp_CreateSpawnChannel_TCL_DECLARED
#define Exp_CreateSpawnChannel_TCL_DECLARED
/* 40 */
TCL_EXTERN(Tcl_Channel)	 Exp_CreateSpawnChannel _ANSI_ARGS_((
				Tcl_Interp * interp, 
				Exp_SpawnOptionSet * opts, int objc, 
				struct Tcl_Obj * CONST objv[], 
				unsigned long * pid, 
				Tcl_Pid * theUglyHandleHackJob));
#endif

typedef struct ExpStubHooks {
    struct ExpPlatStubs *expPlatStubs;
    struct ExpIntStubs *expIntStubs;
    struct ExpIntPlatStubs *expIntPlatStubs;
} ExpStubHooks;

typedef struct ExpStubs {
    int magic;
    struct ExpStubHooks *hooks;

    int (*expect_Init) _ANSI_ARGS_((Tcl_Interp * interp)); /* 0 */
    void *reserved1;
    void *reserved2;
    int (*exp_CloseObjCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int objc, struct Tcl_Obj *CONST objv[])); /* 3 */
    int (*exp_DisconnectCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int argc, CONST84 char * argv[])); /* 4 */
    int (*exp_ExitCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int argc, CONST84 char * argv[])); /* 5 */
    int (*exp_ExpContinueCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int argc, CONST84 char * argv[])); /* 6 */
    int (*exp_ExpectObjCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int objc, struct Tcl_Obj *CONST objv[])); /* 7 */
    int (*exp_ExpectGlobalObjCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int objc, struct Tcl_Obj *CONST objv[])); /* 8 */
    int (*exp_ExpInternalCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int argc, CONST84 char * argv[])); /* 9 */
    int (*exp_ExpPidCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int argc, CONST84 char * argv[])); /* 10 */
    int (*exp_ExpVersionCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int argc, CONST84 char * argv[])); /* 11 */
    int (*exp_ForkCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int argc, CONST84 char * argv[])); /* 12 */
    int (*exp_GetpidDeprecatedCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int argc, CONST84 char * argv[])); /* 13 */
    void *reserved14;
    int (*exp_InterpreterObjCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int objc, struct Tcl_Obj * CONST objv[])); /* 15 */
    int (*exp_InterReturnObjCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int objc, struct Tcl_Obj * CONST objv[])); /* 16 */
    int (*exp_KillCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int argc, CONST84 char * argv[])); /* 17 */
    int (*exp_LogFileCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int argc, CONST84 char * argv[])); /* 18 */
    int (*exp_LogUserCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int argc, CONST84 char * argv[])); /* 19 */
    int (*exp_MatchMaxCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int argc, CONST84 char * argv[])); /* 20 */
    int (*exp_OpenCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int argc, CONST84 char * argv[])); /* 21 */
    int (*exp_OverlayCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int argc, CONST84 char * argv[])); /* 22 */
    int (*exp_ParityCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int argc, CONST84 char * argv[])); /* 23 */
    int (*exp_Prompt1ObjCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int objc, struct Tcl_Obj * CONST objv[])); /* 24 */
    int (*exp_Prompt2ObjCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int objc, struct Tcl_Obj * CONST objv[])); /* 25 */
    int (*exp_RemoveNullsCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int argc, CONST84 char * argv[])); /* 26 */
    int (*exp_SendLogCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int argc, CONST84 char * argv[])); /* 27 */
    int (*exp_SendObjCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int objc, struct Tcl_Obj * CONST objv[])); /* 28 */
    int (*exp_SleepCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int argc, CONST84 char * argv[])); /* 29 */
    int (*exp_SpawnObjCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int objc, struct Tcl_Obj * CONST objv[])); /* 30 */
    int (*exp_StraceCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int argc, CONST84 char * argv[])); /* 31 */
    int (*exp_SttyCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int argc, CONST84 char * argv[])); /* 32 */
    int (*exp_SystemCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int argc, CONST84 char * argv[])); /* 33 */
    int (*exp_TimestampCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int argc, CONST84 char * argv[])); /* 34 */
    int (*exp_TrapCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int argc, CONST84 char * argv[])); /* 35 */
    int (*exp_WaitCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int argc, CONST84 char * argv[])); /* 36 */
    int (*exp_CloseOnEofCmd) _ANSI_ARGS_((ClientData clientData, Tcl_Interp * interp, int argc, CONST84 char * argv[])); /* 37 */
    void *reserved38;
    void *reserved39;
    Tcl_Channel (*exp_CreateSpawnChannel) _ANSI_ARGS_((Tcl_Interp * interp, Exp_SpawnOptionSet * opts, int objc, struct Tcl_Obj * CONST objv[], unsigned long * pid, Tcl_Pid * theUglyHandleHackJob)); /* 40 */
} ExpStubs;
TCL_EXTERNC ExpStubs *expStubsPtr;

#if defined(USE_EXP_STUBS) && !defined(USE_EXP_STUB_PROCS)

/*
 * Inline function declarations:
 */

#ifndef Expect_Init
#define Expect_Init \
	(expStubsPtr->expect_Init) /* 0 */
#endif
/* Slot 1 is reserved */
/* Slot 2 is reserved */
#ifndef Exp_CloseObjCmd
#define Exp_CloseObjCmd \
	(expStubsPtr->exp_CloseObjCmd) /* 3 */
#endif
#ifndef Exp_DisconnectCmd
#define Exp_DisconnectCmd \
	(expStubsPtr->exp_DisconnectCmd) /* 4 */
#endif
#ifndef Exp_ExitCmd
#define Exp_ExitCmd \
	(expStubsPtr->exp_ExitCmd) /* 5 */
#endif
#ifndef Exp_ExpContinueCmd
#define Exp_ExpContinueCmd \
	(expStubsPtr->exp_ExpContinueCmd) /* 6 */
#endif
#ifndef Exp_ExpectObjCmd
#define Exp_ExpectObjCmd \
	(expStubsPtr->exp_ExpectObjCmd) /* 7 */
#endif
#ifndef Exp_ExpectGlobalObjCmd
#define Exp_ExpectGlobalObjCmd \
	(expStubsPtr->exp_ExpectGlobalObjCmd) /* 8 */
#endif
#ifndef Exp_ExpInternalCmd
#define Exp_ExpInternalCmd \
	(expStubsPtr->exp_ExpInternalCmd) /* 9 */
#endif
#ifndef Exp_ExpPidCmd
#define Exp_ExpPidCmd \
	(expStubsPtr->exp_ExpPidCmd) /* 10 */
#endif
#ifndef Exp_ExpVersionCmd
#define Exp_ExpVersionCmd \
	(expStubsPtr->exp_ExpVersionCmd) /* 11 */
#endif
#ifndef Exp_ForkCmd
#define Exp_ForkCmd \
	(expStubsPtr->exp_ForkCmd) /* 12 */
#endif
#ifndef Exp_GetpidDeprecatedCmd
#define Exp_GetpidDeprecatedCmd \
	(expStubsPtr->exp_GetpidDeprecatedCmd) /* 13 */
#endif
/* Slot 14 is reserved */
#ifndef Exp_InterpreterObjCmd
#define Exp_InterpreterObjCmd \
	(expStubsPtr->exp_InterpreterObjCmd) /* 15 */
#endif
#ifndef Exp_InterReturnObjCmd
#define Exp_InterReturnObjCmd \
	(expStubsPtr->exp_InterReturnObjCmd) /* 16 */
#endif
#ifndef Exp_KillCmd
#define Exp_KillCmd \
	(expStubsPtr->exp_KillCmd) /* 17 */
#endif
#ifndef Exp_LogFileCmd
#define Exp_LogFileCmd \
	(expStubsPtr->exp_LogFileCmd) /* 18 */
#endif
#ifndef Exp_LogUserCmd
#define Exp_LogUserCmd \
	(expStubsPtr->exp_LogUserCmd) /* 19 */
#endif
#ifndef Exp_MatchMaxCmd
#define Exp_MatchMaxCmd \
	(expStubsPtr->exp_MatchMaxCmd) /* 20 */
#endif
#ifndef Exp_OpenCmd
#define Exp_OpenCmd \
	(expStubsPtr->exp_OpenCmd) /* 21 */
#endif
#ifndef Exp_OverlayCmd
#define Exp_OverlayCmd \
	(expStubsPtr->exp_OverlayCmd) /* 22 */
#endif
#ifndef Exp_ParityCmd
#define Exp_ParityCmd \
	(expStubsPtr->exp_ParityCmd) /* 23 */
#endif
#ifndef Exp_Prompt1ObjCmd
#define Exp_Prompt1ObjCmd \
	(expStubsPtr->exp_Prompt1ObjCmd) /* 24 */
#endif
#ifndef Exp_Prompt2ObjCmd
#define Exp_Prompt2ObjCmd \
	(expStubsPtr->exp_Prompt2ObjCmd) /* 25 */
#endif
#ifndef Exp_RemoveNullsCmd
#define Exp_RemoveNullsCmd \
	(expStubsPtr->exp_RemoveNullsCmd) /* 26 */
#endif
#ifndef Exp_SendLogCmd
#define Exp_SendLogCmd \
	(expStubsPtr->exp_SendLogCmd) /* 27 */
#endif
#ifndef Exp_SendObjCmd
#define Exp_SendObjCmd \
	(expStubsPtr->exp_SendObjCmd) /* 28 */
#endif
#ifndef Exp_SleepCmd
#define Exp_SleepCmd \
	(expStubsPtr->exp_SleepCmd) /* 29 */
#endif
#ifndef Exp_SpawnObjCmd
#define Exp_SpawnObjCmd \
	(expStubsPtr->exp_SpawnObjCmd) /* 30 */
#endif
#ifndef Exp_StraceCmd
#define Exp_StraceCmd \
	(expStubsPtr->exp_StraceCmd) /* 31 */
#endif
#ifndef Exp_SttyCmd
#define Exp_SttyCmd \
	(expStubsPtr->exp_SttyCmd) /* 32 */
#endif
#ifndef Exp_SystemCmd
#define Exp_SystemCmd \
	(expStubsPtr->exp_SystemCmd) /* 33 */
#endif
#ifndef Exp_TimestampCmd
#define Exp_TimestampCmd \
	(expStubsPtr->exp_TimestampCmd) /* 34 */
#endif
#ifndef Exp_TrapCmd
#define Exp_TrapCmd \
	(expStubsPtr->exp_TrapCmd) /* 35 */
#endif
#ifndef Exp_WaitCmd
#define Exp_WaitCmd \
	(expStubsPtr->exp_WaitCmd) /* 36 */
#endif
#ifndef Exp_CloseOnEofCmd
#define Exp_CloseOnEofCmd \
	(expStubsPtr->exp_CloseOnEofCmd) /* 23 */
#endif
/* Slot 38 is reserved */
/* Slot 39 is reserved */
#ifndef Exp_CreateSpawnChannel
#define Exp_CreateSpawnChannel \
	(expStubsPtr->exp_CreateSpawnChannel) /* 40 */
#endif

#endif /* defined(USE_EXP_STUBS) && !defined(USE_EXP_STUB_PROCS) */

/* !END!: Do not edit above this line. */

#endif /* _EXPDECLS */
