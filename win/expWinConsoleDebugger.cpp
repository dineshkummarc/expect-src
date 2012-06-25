/* ----------------------------------------------------------------------------
 * expWinConsoleDebugger.cpp --
 *
 *	Console debugger core implimentation.
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
 * RCS: @(#) $Id: expWinConsoleDebugger.cpp,v 1.1.2.34 2003/08/26 20:46:52 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include <stddef.h>
#include <assert.h>
#include "expWinPort.h"
#include "expWinConsoleDebugger.hpp"
#include "expWinInjectorIPC.hpp"

#if 0  // Can't do DOS, so don't try.
#include <vdmdbg.h>
/* Platform SDK has this, but not the stock vdmdbg.h. */
#ifndef DBG_GPFAULT2
#   define DBG_GPFAULT2    21
#endif
#ifdef _MSC_VER
#   pragma comment (lib, "vdmdbg.lib")
#endif
#endif

/* Fix the error in winnt.h */
#if (_MSC_VER == 1200 && defined(DEFAULT_UNREACHABLE))
#   undef DEFAULT_UNREACHABLE
#   define DEFAULT_UNREACHABLE default: __assume(0)
#elif !defined(DEFAULT_UNREACHABLE)
#   define DEFAULT_UNREACHABLE default: break
#endif



#ifdef _MSC_VER
#   pragma comment (lib, "imagehlp.lib")
#endif

#ifdef _M_IX86
    // Breakpoint opcode on i386
#   define BRK_OPCODE	    0xCC
    // Single step flag
#   define SINGLE_STEP_BIT  0x100
#else
#   error "need opcodes for this hardware".
#endif



//  Constructor.
ConsoleDebugger::ConsoleDebugger (
	TCHAR *_cmdline,		// commandline string (in system encoding)
	TCHAR *_env,			// environment block (in system encoding)
	TCHAR *_dir,			// startup directory (in system encoding)
					// These 3 maintain a reference until
					//  _readyUp is signaled.
	int _show,			// $exp::nt_debug, shows spawned children.
	const char *_injPath,		// location -=[ ON THE FILE SYSTEM!!! ]=-
	CMclLinkedList<Message *> &_mQ,	// parent owned linkedlist for returning data stream.
	CMclLinkedList<Message *> &_eQ,	// parent owned linkedlist for returning error stream.
	CMclEvent &_readyUp,		// set when child ready (or failed).
	ConsoleDebuggerCallback &_callback
    ) :
    cmdline(_cmdline), env(_env), dir(_dir), mQ(_mQ), eQ(_eQ),
    readyUp(_readyUp), callback(_callback), ProcessList(0L),
    CursorKnown(FALSE), ConsoleOutputCP(0), ConsoleCP(0),
    pStartAddress(0L), originalExeEntryPointOpcode(0), show(_show),
    injectorStub(_injPath), pInjectorStub(0), injectorIPC(0L),
    interacting(false), status(NO_ERROR), pid(0), fatalException(0),
    pidKilled(0),
    hMasterConsole(INVALID_HANDLE_VALUE),
    hCopyScreenBuffer(INVALID_HANDLE_VALUE)
{
    OSVERSIONINFO osvi;
    DWORD n, i;

    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osvi);
    dwPlatformId = osvi.dwPlatformId;

    if (dwPlatformId == VER_PLATFORM_WIN32_NT) {
	pfnVirtualAllocEx = (PFNVIRTUALALLOCEX) GetProcAddress(
		GetModuleHandle("KERNEL32.DLL"),"VirtualAllocEx");
	pfnVirtualFreeEx = (PFNVIRTUALFREEEX) GetProcAddress(
		GetModuleHandle("KERNEL32.DLL"),"VirtualFreeEx");
    }

    n = GetEnvironmentVariable("Path", NULL, 0);
    n += GetEnvironmentVariable("_NT_SYMBOL_PATH", NULL, 0) + 1;
    n += GetEnvironmentVariable("_NT_ALT_SYMBOL_PATH", NULL, 0) + 1;
    n += GetEnvironmentVariable("SystemRoot", NULL, 0) + 1;

    SymbolPath = new char [n + 1];

    i = GetEnvironmentVariable("Path", SymbolPath, n);
    SymbolPath[i++] = ';';
    i += GetEnvironmentVariable("_NT_SYMBOL_PATH", &SymbolPath[i], n-i);
    SymbolPath[i++] = ';';
    i += GetEnvironmentVariable("_NT_ALT_SYMBOL_PATH", &SymbolPath[i], n-i);
    SymbolPath[i++] = ';';
    i += GetEnvironmentVariable("SystemRoot", &SymbolPath[i], n-i);

    //  Until further notice, assume this.
    //
    ConsoleSize.X = 80;
    ConsoleSize.Y = 25;

    //  Until further notice, assume this, too.
    //
    CursorPosition.X = 0;
    CursorPosition.Y = 0;

    //  Set all our breakpoint info.  We have to do this the long way, here in
    //  the constructor, because we need to have the 'this' pointer already
    //  set because we refer to ourselves by needing the address to member
    //  functions.
    //

    // interesting ones I might want to add:
    // DuplicateConsoleHandle

    BreakArrayKernel32[0].funcName = "AllocConsole";
    BreakArrayKernel32[0].nargs = 0;
    BreakArrayKernel32[0].breakProc = OnAllocConsole;
    BreakArrayKernel32[0].dwFlags = BREAK_OUT;

    BreakArrayKernel32[1].funcName = "Beep";
    BreakArrayKernel32[1].nargs = 2;
    BreakArrayKernel32[1].breakProc = OnBeep;
    BreakArrayKernel32[1].dwFlags = BREAK_OUT|BREAK_IN;

    BreakArrayKernel32[2].funcName = "CreateConsoleScreenBuffer";
    BreakArrayKernel32[2].nargs = 5;
    BreakArrayKernel32[2].breakProc = OnCreateConsoleScreenBuffer;
    BreakArrayKernel32[2].dwFlags = BREAK_OUT;

    BreakArrayKernel32[3].funcName = "FillConsoleOutputAttribute";
    BreakArrayKernel32[3].nargs = 5;
    BreakArrayKernel32[3].breakProc = OnFillConsoleOutputAttribute;
    BreakArrayKernel32[3].dwFlags = BREAK_OUT;

    BreakArrayKernel32[4].funcName = "FillConsoleOutputCharacterA";
    BreakArrayKernel32[4].nargs = 5;
    BreakArrayKernel32[4].breakProc = OnFillConsoleOutputCharacterA;
    BreakArrayKernel32[4].dwFlags = BREAK_OUT;

    BreakArrayKernel32[5].funcName = "FillConsoleOutputCharacterW";
    BreakArrayKernel32[5].nargs = 5;
    BreakArrayKernel32[5].breakProc = OnFillConsoleOutputCharacterW;
    BreakArrayKernel32[5].dwFlags = BREAK_OUT;

    BreakArrayKernel32[6].funcName = "FreeConsole";
    BreakArrayKernel32[6].nargs = 0;
    BreakArrayKernel32[6].breakProc = OnFreeConsole;
    BreakArrayKernel32[6].dwFlags = BREAK_OUT;

    BreakArrayKernel32[7].funcName = "GetStdHandle";
    BreakArrayKernel32[7].nargs = 1;
    BreakArrayKernel32[7].breakProc = OnGetStdHandle;
    BreakArrayKernel32[7].dwFlags = BREAK_OUT;

    BreakArrayKernel32[8].funcName = "OpenConsoleW";
    BreakArrayKernel32[8].nargs = 4;
    BreakArrayKernel32[8].breakProc = OnOpenConsoleW;
    BreakArrayKernel32[8].dwFlags = BREAK_OUT;

    BreakArrayKernel32[9].funcName = "ScrollConsoleScreenBufferA";
    BreakArrayKernel32[9].nargs = 5;
    BreakArrayKernel32[9].breakProc = OnScrollConsoleScreenBuffer;
    BreakArrayKernel32[9].dwFlags = BREAK_OUT;

    BreakArrayKernel32[10].funcName = "ScrollConsoleScreenBufferW";
    BreakArrayKernel32[10].nargs = 5;
    BreakArrayKernel32[10].breakProc = OnScrollConsoleScreenBuffer;
    BreakArrayKernel32[10].dwFlags = BREAK_OUT;

    BreakArrayKernel32[11].funcName = "SetConsoleActiveScreenBuffer";
    BreakArrayKernel32[11].nargs = 1;
    BreakArrayKernel32[11].breakProc = OnSetConsoleActiveScreenBuffer;
    BreakArrayKernel32[11].dwFlags = BREAK_OUT;

    BreakArrayKernel32[12].funcName = "SetConsoleCP";
    BreakArrayKernel32[12].nargs = 1;
    BreakArrayKernel32[12].breakProc = OnSetConsoleCP;
    BreakArrayKernel32[12].dwFlags = BREAK_OUT;

    BreakArrayKernel32[13].funcName = "SetConsoleCursorInfo";
    BreakArrayKernel32[13].nargs = 2;
    BreakArrayKernel32[13].breakProc = OnSetConsoleCursorInfo;
    BreakArrayKernel32[13].dwFlags = BREAK_OUT;

    BreakArrayKernel32[14].funcName = "SetConsoleCursorPosition";
    BreakArrayKernel32[14].nargs = 2;
    BreakArrayKernel32[14].breakProc = OnSetConsoleCursorPosition;
    BreakArrayKernel32[14].dwFlags = BREAK_OUT;

    BreakArrayKernel32[15].funcName = "SetConsoleMode";
    BreakArrayKernel32[15].nargs = 2;
    BreakArrayKernel32[15].breakProc = OnSetConsoleMode;
    BreakArrayKernel32[15].dwFlags = BREAK_OUT;

    BreakArrayKernel32[16].funcName = "SetConsoleOutputCP";
    BreakArrayKernel32[16].nargs = 1;
    BreakArrayKernel32[16].breakProc = OnSetConsoleOutputCP;
    BreakArrayKernel32[16].dwFlags = BREAK_OUT;

    BreakArrayKernel32[17].funcName = "SetConsoleTextAttribute";
    BreakArrayKernel32[17].nargs = 2;
    BreakArrayKernel32[17].breakProc = OnSetConsoleTextAttribute;
    BreakArrayKernel32[17].dwFlags = BREAK_OUT;

    BreakArrayKernel32[18].funcName = "SetConsoleWindowInfo";
    BreakArrayKernel32[18].nargs = 3;
    BreakArrayKernel32[18].breakProc = OnSetConsoleWindowInfo;
    BreakArrayKernel32[18].dwFlags = BREAK_OUT;

    BreakArrayKernel32[19].funcName = "WriteConsoleA";
    BreakArrayKernel32[19].nargs = 5;
    BreakArrayKernel32[19].breakProc = OnWriteConsoleA;
    BreakArrayKernel32[19].dwFlags = BREAK_OUT;

    BreakArrayKernel32[20].funcName = "WriteConsoleW";
    BreakArrayKernel32[20].nargs = 5;
    BreakArrayKernel32[20].breakProc = OnWriteConsoleW;
    BreakArrayKernel32[20].dwFlags = BREAK_OUT;

    BreakArrayKernel32[21].funcName = "WriteConsoleOutputA";
    BreakArrayKernel32[21].nargs = 5;
    BreakArrayKernel32[21].breakProc = OnWriteConsoleOutputA;
    BreakArrayKernel32[21].dwFlags = BREAK_OUT;

    BreakArrayKernel32[22].funcName = "WriteConsoleOutputW";
    BreakArrayKernel32[22].nargs = 5;
    BreakArrayKernel32[22].breakProc = OnWriteConsoleOutputW;
    BreakArrayKernel32[22].dwFlags = BREAK_OUT;

    BreakArrayKernel32[23].funcName = "WriteConsoleOutputCharacterA";
    BreakArrayKernel32[23].nargs = 5;
    BreakArrayKernel32[23].breakProc = OnWriteConsoleOutputCharacterA;
    BreakArrayKernel32[23].dwFlags = BREAK_OUT;

    BreakArrayKernel32[24].funcName = "WriteConsoleOutputCharacterW";
    BreakArrayKernel32[24].nargs = 5;
    BreakArrayKernel32[24].breakProc = OnWriteConsoleOutputCharacterW;
    BreakArrayKernel32[24].dwFlags = BREAK_OUT;

    BreakArrayKernel32[25].funcName = "WriteFile";
    BreakArrayKernel32[25].nargs = 5;
    BreakArrayKernel32[25].breakProc = OnWriteFile;
    BreakArrayKernel32[25].dwFlags = BREAK_OUT;

    BreakArrayKernel32[26].funcName = 0L;
    BreakArrayKernel32[26].nargs = 0;
    BreakArrayKernel32[26].breakProc = 0L;
    BreakArrayKernel32[26].dwFlags = 0;

    BreakArrayUser32[0].funcName = "IsWindowVisible";
    BreakArrayUser32[0].nargs = 1;
    BreakArrayUser32[0].breakProc = OnIsWindowVisible;
    BreakArrayUser32[0].dwFlags = BREAK_OUT;

    BreakArrayUser32[1].funcName = 0L;
    BreakArrayUser32[1].nargs = 0;
    BreakArrayUser32[1].breakProc = 0L;
    BreakArrayUser32[1].dwFlags = 0;

    BreakPoints[0].dllName = "kernel32.dll";
    BreakPoints[0].breakInfo = BreakArrayKernel32;

    BreakPoints[1].dllName = "user32.dll";
    BreakPoints[1].breakInfo = BreakArrayUser32;

    BreakPoints[2].dllName = 0L;
    BreakPoints[2].breakInfo = 0L;

    hMasterConsole = CreateFile("CONOUT$", GENERIC_READ|GENERIC_WRITE,
	    FILE_SHARE_READ|FILE_SHARE_WRITE, 0L, OPEN_EXISTING, 0, 0L);

    hCopyScreenBuffer = CreateConsoleScreenBuffer(
	    GENERIC_READ|GENERIC_WRITE, 0, NULL,
	    CONSOLE_TEXTMODE_BUFFER, NULL);
}

ConsoleDebugger::~ConsoleDebugger()
{
    delete [] SymbolPath;
    if (injectorIPC) delete injectorIPC;
    if (hMasterConsole != INVALID_HANDLE_VALUE) {
	CloseHandle(hMasterConsole);
    }
    if (hCopyScreenBuffer != INVALID_HANDLE_VALUE) {
	CloseHandle(hCopyScreenBuffer);
    }
}

unsigned
ConsoleDebugger::ThreadHandlerProc(void)
{
    DWORD ok, exitcode;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    DWORD createFlags = 0;

    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwXCountChars = 80;
    si.dwYCountChars = 25;
    if (show) {
	si.wShowWindow = SW_SHOWNOACTIVATE;
    } else {
	si.wShowWindow = SW_HIDE;
    }
    si.dwFlags = STARTF_FORCEONFEEDBACK | STARTF_USESHOWWINDOW |
	STARTF_USECOUNTCHARS;

    createFlags = DEBUG_PROCESS |	// <- Oh, so important!
	CREATE_NEW_CONSOLE |		// Yes, please.
	CREATE_DEFAULT_ERROR_MODE |	// Is this correct so error dialogs don't pop up?
	(expWinProcs->useWide ? CREATE_UNICODE_ENVIRONMENT : 0);

    /* Magic env var to raise expect priority */
    if (getenv("EXPECT_SLAVE_HIGH_PRIORITY") != NULL) {
	createFlags |= ABOVE_NORMAL_PRIORITY_CLASS;
    }

    ok = expWinProcs->createProcessProc(
	    0L,		// Module name (not needed).
	    cmdline,	// Command line string (must be writable!).
	    0L,		// Process handle will not be inheritable.
	    0L,		// Thread handle will not be inheritable.
	    FALSE,	// No handle inheritance.
	    createFlags,// Creation flags.
	    env,	// Use custom environment block, or parent's if NULL.
	    dir,	// Use custom starting directory, or parent's if NULL.
	    &si,	// Pointer to STARTUPINFO structure.
	    &pi);	// Pointer to PROCESS_INFORMATION structure.

    if (!ok) {
	status = GetLastError();
	readyUp.Set();
	return 0;
    }

    pid = pi.dwProcessId;

    // The process handle is to be closed by Tcl_WaitPid.
    hRootProcess = pi.hProcess;
    CloseHandle(pi.hThread);
    
    exitcode = CommonDebugger();

#ifdef EXP_DEBUG
    expDiagLog("ConsoleDebugger::ThreadHandlerProc: %x\n",
	       exitcode);
#endif
    NotifyDone();
    return exitcode;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::CommonDebugger --
 *
 *	This is the function that is the debugger for all slave processes
 *
 * Results:
 *	None.  This thread exits with ExitThread() when the subprocess dies.
 *
 * Side Effects:
 *	Adds the process to the things being waited for by
 *	WaitForMultipleObjects
 *
 *----------------------------------------------------------------------
 */
DWORD
ConsoleDebugger::CommonDebugger()
{
    DEBUG_EVENT debEvent;	// debugging event info.
    DWORD dwContinueStatus;	// exception continuation.
    Process *proc;
    int breakCount = 0;
    BOOL ok;

again:
    dwContinueStatus = DBG_CONTINUE;

    // Wait (forever) for a debugging event to occur.
    //
    if (WaitForDebugEvent(&debEvent, INFINITE) == FALSE) {
	status = GetLastError();
	return 0;
    }

    // Find the process that is responsible for this event.
    //
    for (proc = ProcessList; proc; proc = proc->nextPtr) {
	if (proc->pid == debEvent.dwProcessId) {
	    break;
	}
    }

    if (!proc && debEvent.dwDebugEventCode != CREATE_PROCESS_DEBUG_EVENT) {
/*	char buf[50];
	wsprintf(buf, "%d/%d (%d)", 
		debEvent.dwProcessId, debEvent.dwThreadId,
		debEvent.dwDebugEventCode);
	EXP_LOG1(MSG_DT_UNEXPECTEDDBGEVENT, buf);*/
	if (debEvent.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) {
	    /*char buf[50];
	    wsprintf(buf, "0x%08x", debEvent.u.Exception.ExceptionRecord.ExceptionCode);
	    EXP_LOG1(MSG_DT_EXCEPTIONDBGEVENT, buf);*/
	    dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
	}
	goto skip;
    }

    bpCritSec.Enter();

    // Process the debugging event code.
    //
    switch (debEvent.dwDebugEventCode) {
    case EXCEPTION_DEBUG_EVENT:
	switch (debEvent.u.Exception.ExceptionRecord.ExceptionCode) {
	case EXCEPTION_BREAKPOINT:
	{
	    // only the first three get special attention.
	    switch ((breakCount < 4) ? ++breakCount : 4) {
	    case 1:
		ok = OnXFirstBreakpoint(proc, &debEvent);
		break;
	    case 2:
		ok = OnXSecondBreakpoint(proc, &debEvent);
		break;
	    case 3:
		ok = OnXThirdBreakpoint(proc, &debEvent);
		// Notify creator, we are now ready.
		//
		readyUp.Set();
		break;
	    case 4:
		ok = OnXBreakpoint(proc, &debEvent);
	    }
	    if (!ok) {
		// Just in case we barfed on the first or second.
		//
		readyUp.Set();

		// We can't set the exit code properly.  Only exits that
		// are the result of an exception can be returned as
		// they can be decoded correctly. Returning a status
		// code as the exit code serves no purpose for conveying
		// detail of the reason the debugger is not able to
		// operate on this application.
		//
		return 0;
	    }
	    break;
	}

	case EXCEPTION_SINGLE_STEP:
	    OnXSingleStep(proc, &debEvent);
	    break;

	case DBG_CONTROL_C:
	case DBG_CONTROL_BREAK:
	    dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
	    break;

	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
	    // we die here.
	    return EXCEPTION_NONCONTINUABLE_EXCEPTION;

	default:
	    if (!debEvent.u.Exception.dwFirstChance) {
		// An exception was hit and it was not handled by the program.
		// Now it is time to get a backtrace before it's death.
		//
		OnXSecondChanceException(proc, &debEvent);
	    }
	    dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
	}
	break;

    case CREATE_THREAD_DEBUG_EVENT:
	OnXCreateThread(proc, &debEvent);
#ifdef EXP_DEBUG
	expDiagLog("CREATE_THREAD_DEBUG_EVENT: %d %p\n",
		   proc->threadCount, debEvent.dwThreadId);
#endif
	break;

    case CREATE_PROCESS_DEBUG_EVENT:
	OnXCreateProcess(proc, &debEvent);
	break;

    case EXIT_THREAD_DEBUG_EVENT:
	OnXDeleteThread(proc, &debEvent);
#ifdef EXP_DEBUG
	expDiagLog("EXIT_THREAD_DEBUG_EVENT (threadCount: %d) exit %x thread %p\n",
		   proc->threadCount,
		   debEvent.u.ExitThread.dwExitCode,
		   debEvent.dwThreadId);
#endif
	break;

    case EXIT_PROCESS_DEBUG_EVENT:
#ifdef EXP_DEBUG
	expDiagLog("EXIT_PROCESS_DEBUG_EVENT %p (exit %x)\n", proc,
		   debEvent.u.ExitProcess.dwExitCode);
#endif
	ProcessFree(proc);
	if (ProcessList == 0L) {
	    // When the last process exits, we exit.
	    //
	    if (fatalException) {
		return fatalException;
	    } else {
		return debEvent.u.ExitProcess.dwExitCode;
	    }
	}
#if 0
	/*
	 * http://aspn.activestate.com/ASPN/Mail/Message/expect/2795941
	 * For debug events returning EXIT_PROCESS_DEBUG_EVENT, they can't
	 * call fall through to ContinueDebugEvent() or it causes an exception
	 * in the child of the spawned app's thread, thereby causing the
	 * spawned app to crash, thus close.
	 */
	goto again;
#else
	break;
#endif

    case LOAD_DLL_DEBUG_EVENT:
	OnXLoadDll(proc, &debEvent);
	break;

    case UNLOAD_DLL_DEBUG_EVENT:
	OnXUnloadDll(proc, &debEvent);
	break;

    case OUTPUT_DEBUG_STRING_EVENT:
	OnXDebugString(proc, &debEvent);
	break;

    case RIP_EVENT:
	OnXRip(proc, &debEvent);
	break;
    }

    bpCritSec.Leave();

skip:
    // Resume executing the thread that reported the debugging event.
    //
    if (ContinueDebugEvent(debEvent.dwProcessId, debEvent.dwThreadId,
	    dwContinueStatus) == FALSE) {
	status = GetLastError();
	return 0;
    }
    goto again;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXFirstBreakpoint --
 *
 *	This routine is called when a EXCEPTION_DEBUG_EVENT with
 *	an exception code of EXCEPTION_BREAKPOINT, and it is the
 *	first one to occur in the program.  This happens when the
 *	process finally gets loaded into memory and is about to
 *	start.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */


BOOL
ConsoleDebugger::OnXFirstBreakpoint(Process *proc, LPDEBUG_EVENT pDebEvent)
{
    ThreadInfo *tinfo;

    for (tinfo = proc->threadList; tinfo != 0L; tinfo = tinfo->nextPtr) {
	if (pDebEvent->dwThreadId == tinfo->dwThreadId) {
	    break;
	}
    }

    // Install our entry breakpoint.  We can't run the injector Stub now
    // as the process heap hasn't been created.  (it was some memory related
    // error and I don't know exactly why, but moving the stub run later
    // solved it).  It seems that the first breakpoint naturally fired is
    // much too early in the creation of the initial process for much of
    // anything to be ready.
    //
    ReadSubprocessMemory(proc, pStartAddress,
	    &originalExeEntryPointOpcode, sizeof(BYTE));

    BYTE bpOpcode = BRK_OPCODE;
    WriteSubprocessMemory(proc, pStartAddress, &bpOpcode, sizeof(BYTE));

    return TRUE;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXSecondBreakpoint --
 *
 *	This routine is called when the second breakpoint is hit.
 *	The processes' entry-point is getting called.  We intercept this
 *	and make our injector.dll load first before the entry-point is
 *	called.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */

BOOL
ConsoleDebugger::OnXSecondBreakpoint(Process *proc, LPDEBUG_EVENT pDebEvent)
{
    ThreadInfo *tinfo;

    for (tinfo = proc->threadList; tinfo != 0L; tinfo = tinfo->nextPtr) {
	if (pDebEvent->dwThreadId == tinfo->dwThreadId) {
	    break;
	}
    }

    // Put the first opcode at the entry point back in place.
    WriteSubprocessMemory(proc, pStartAddress,
	    &originalExeEntryPointOpcode, sizeof(BYTE));

    //  Make some memory for our stub that we place into the processes' address
    //  space.  This stub (or set of opcodes) calls LoadLibrary() to bring in our
    //  injector dll that acts as the receiver for "injecting" console events.
    //
    if (!MakeSubprocessMemory(proc, sizeof(LOADLIBRARY_STUB), &pInjectorStub,
	    PAGE_EXECUTE_READWRITE)) {
	/* major failure. */
	return FALSE;
    }
    injectorStub.operand_PUSH_value = (DWORD) pInjectorStub +
	    offsetof(LOADLIBRARY_STUB, data_DllName);
    injectorStub.operand_MOV_EAX = (DWORD) GetProcAddress(GetModuleHandle(
	    "KERNEL32.DLL"), "LoadLibraryA");
    WriteSubprocessMemory(proc, pInjectorStub, &injectorStub,
	    sizeof(LOADLIBRARY_STUB));
    FlushInstructionCache(proc->hProcess, pInjectorStub, sizeof(LOADLIBRARY_STUB));


    //  Save the instruction pointer so we can restore it later.
    //
    preStubContext.ContextFlags = CONTEXT_FULL;
    GetThreadContext(tinfo->hThread, &preStubContext);

    //  Set instruction pointer to run the Stub instead of continuing
    //  from where we are (which is the app's entry point).
    //
    CONTEXT stubContext = preStubContext;
    stubContext.Eip = (DWORD) pInjectorStub;
    SetThreadContext(tinfo->hThread, &stubContext);

    return TRUE;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXThirdBreakpoint --
 *
 *	This routine is called when the third breakpoint is hit and
 *	our stub has run and our injector is loaded.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */

BOOL
ConsoleDebugger::OnXThirdBreakpoint(Process *proc, LPDEBUG_EVENT pDebEvent)
{
    ThreadInfo *tinfo;
    CHAR boxName[50];
    DWORD err;
#   define RETBUF_SIZE 2048
    BYTE retbuf[RETBUF_SIZE];
    BreakInfo *binfo;
    int i;

    for (tinfo = proc->threadList; tinfo != 0L; tinfo = tinfo->nextPtr) {
	if (pDebEvent->dwThreadId == tinfo->dwThreadId) {
	    break;
	}
    }

    // Create the IPC connection to our loaded injector.dll
    //
    wsprintf(boxName, "ExpectInjector_pid%d", proc->pid);
    injectorIPC = new CMclMailbox(IPC_NUMSLOTS, IPC_SLOTSIZE, boxName);

    // Check status.
    err = injectorIPC->Status();
    if (err != NO_ERROR && err != ERROR_ALREADY_EXISTS) {
	const char *msg = ExpWinErrMsg(err, boxName, 0L);
	int len = strlen(msg);
	WriteMasterError(msg, len, ERROR_EXP_WIN32_CANT_IPC);
	delete injectorIPC;
	injectorIPC = 0L;
	return err;
    }
    pidKilled = 0;

    // Set our thread to run the entry point, now, starting the
    // application once we return from this breakpoint.
    preStubContext.Eip -= sizeof(BYTE);
    SetThreadContext(tinfo->hThread, &preStubContext);


    // We should now remove the memory allocated in the sub process for
    // our injector stub.  The dll is already loaded and there's no sense
    // hogging a virtual memory page.
    //
    RemoveSubprocessMemory(proc, pInjectorStub);


    /////////////////////////////////////////////////////////////////////
    // Now create our breakpoints on all calls to the OS console API.
    /////////////////////////////////////////////////////////////////////


    // Set up the memory that will serve as the place for our
    // intercepted function return points.
    //
    if (!MakeSubprocessMemory(proc,  RETBUF_SIZE, &(proc->pSubprocessMemory),
			      PAGE_EXECUTE_READWRITE)) {
	/* major failure. */
	return FALSE;
    }

    // Fill the buffer with all breakpoint opcodes.
    //
    memset(retbuf, BRK_OPCODE, RETBUF_SIZE);

    // Write it out to our buffer space in the other process.
    //
    WriteSubprocessMemory(proc, proc->pSubprocessMemory, retbuf, RETBUF_SIZE);

    // Set all Console API breakpoints.
    //
    for (i = 0; BreakPoints[i].dllName; i++) {
	for (binfo = BreakPoints[i].breakInfo; binfo->funcName; binfo++) {
	    SetBreakpoint(proc, binfo);
	}
    }

#   undef RETBUF_SIZE
    return TRUE;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::SetBreakpoint --
 *
 *	Inserts a single breakpoint
 *
 * Results:
 *	TRUE if successful, FALSE if unsuccessful.
 *
 *----------------------------------------------------------------------
 */

BOOL
ConsoleDebugger::SetBreakpoint(Process *process, BreakInfo *info)
{
    PVOID funcPtr;

    if (process->funcTable.Find(info->funcName, &funcPtr) == TCL_ERROR) {
//	PCHAR buffer;
//	DWORD len;
//
//	buffer = new CHAR [128];
//	len = wsprintf(buffer, "Unable to set breakpoint at %s", info->funcName);
//	WriteMasterError(buffer, len);
	return FALSE;
    }

    // Set a breakpoint at the function start in the subprocess and
    // save the original code at the function start.
    //
    return SetBreakpointAtAddr(process, info, funcPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::SetBreakpointAtAddr --
 *
 *	Inserts a single breakpoint at the given address
 *
 * Results:
 *	The new BreakPoint instance.
 *
 *----------------------------------------------------------------------
 */

BOOL
ConsoleDebugger::SetBreakpointAtAddr(Process *proc, BreakInfo *info, PVOID funcPtr)
{
    Breakpoint *bpt, *lastBpt;
    BYTE code;
    BOOL ok;

    bpt = new Breakpoint;
    bpt->codePtr = funcPtr;
    bpt->codeReturnPtr = (PVOID) (proc->offset + (DWORD) proc->pSubprocessMemory);
    bpt->breakInfo = info;
    proc->offset += 2;
    bpt->nextPtr = lastBpt = proc->brkptList;
    proc->brkptList = bpt;

    if ((ok = ReadSubprocessMemory(proc, funcPtr, &bpt->code, sizeof(BYTE)))
	    == TRUE) {
	code = BRK_OPCODE;
	ok = WriteSubprocessMemory(proc, funcPtr, &code, sizeof(BYTE));
    }

    if (!ok) {
	// unsplice it.
	proc->brkptList = lastBpt;
	delete bpt;
	bpt = 0L;
    }

    return ok;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXSingleStep --
 *
 *	This routine is called when a EXCEPTION_DEBUG_EVENT with
 *	an exception code of EXCEPTION_SINGLE_STEP.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnXSingleStep(Process *proc, LPDEBUG_EVENT pDebEvent)
{
    BYTE code;
    ThreadInfo *tinfo;

    for (tinfo = proc->threadList; tinfo != 0L; tinfo = tinfo->nextPtr) {
	if (pDebEvent->dwThreadId == tinfo->dwThreadId) {
	    break;
	}
    }

    // Now, we need to restore the breakpoint that we had removed.
    //
    code = BRK_OPCODE;
    WriteSubprocessMemory(proc, tinfo->lastBrkpt->codePtr, &code,
			  sizeof(BYTE));
    FlushInstructionCache(proc->hProcess, tinfo->lastBrkpt->codePtr,
			  sizeof(BYTE));
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXSecondChanceException --
 *
 *	Handle a second chance exception
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnXSecondChanceException(Process *proc,
    LPDEBUG_EVENT pDebEvent)
{
    BOOL b;
    STACKFRAME frame;
    CONTEXT context;
    ThreadInfo *tinfo;
    Module *modPtr;
    DWORD displacement;
    BYTE symbolBuffer[sizeof(IMAGEHLP_SYMBOL) + 512];
    PIMAGEHLP_SYMBOL pSymbol = (PIMAGEHLP_SYMBOL)symbolBuffer;
    char *s;
    int result;
    unsigned int level = 0;
    HANDLE hProcess;

    for (tinfo = proc->threadList; tinfo != NULL; tinfo = tinfo->nextPtr) {
	if (pDebEvent->dwThreadId == tinfo->dwThreadId) {
	    break;
	}
    }

    context.ContextFlags = CONTEXT_FULL;
    GetThreadContext(tinfo->hThread, &context);

    /*
     * XXX: From what I can tell, SymInitialize is broken on Windows NT 4.0
     * if you try to have it iterate the modules in a process.  It always
     * returns an object mismatch error.  Instead, initialize without iterating
     * the modules.  Contrary to what MSDN documentation says,
     * Microsoft debuggers do not exclusively use the imagehlp API.  In
     * fact, the only thing VC 5.0 uses is the StackWalk function.
     * Windbg uses a few more functions, but it doesn't use SymInitialize.
     * We will then do the hard work of finding all the
     * modules and doing the right thing.
     */

    if (dwPlatformId == VER_PLATFORM_WIN32_NT) {
	hProcess = proc->hProcess;
    } else {
	hProcess = (HANDLE) proc->pid;  // lie to Win9x about this.
    }

    if (!SymInitialize(hProcess, SymbolPath, FALSE)) {
	LPCSTR msg = ExpWinErrMsg(GetLastError(), proc->exeModule->modName, 0L);
	SIZE_T len = strlen(msg);
	WriteMasterError(msg, len, ERROR_EXP_WIN32_NO_BACKTRACE);
	return;
    }

#ifdef _X86_
    memset(&frame, 0, sizeof(frame));
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrPC.Segment = 0;
    frame.AddrPC.Offset = context.Eip;

    frame.AddrReturn.Mode = AddrModeFlat;
    frame.AddrReturn.Segment = 0;
    frame.AddrReturn.Offset = context.Ebp; /* I think this is correct */

    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrFrame.Segment = 0;
    frame.AddrFrame.Offset = context.Ebp;

    frame.AddrStack.Mode = AddrModeFlat;
    frame.AddrStack.Segment = 0;
    frame.AddrStack.Offset = context.Esp;

    frame.FuncTableEntry = NULL;
    frame.Params[0] = context.Eax;
    frame.Params[1] = context.Ecx;
    frame.Params[2] = context.Edx;
    frame.Params[3] = context.Ebx;
    frame.Far = FALSE;
    frame.Virtual = FALSE;
    frame.Reserved[0] = 0;
    frame.Reserved[1] = 0;
    frame.Reserved[2] = 0;
    /* frame.KdHelp.* is not set */

    /*
     * Iterate through the loaded modules and load symbols for each one.
     */
    for (
	result = proc->moduleTable.Top(&modPtr);
	result == TCL_OK;
	result = proc->moduleTable.Next(&modPtr)
    ) {
	if (!modPtr->loaded) {
	    modPtr->dbgInfo = MapDebugInformation(modPtr->hFile, 0L,
		SymbolPath, (DWORD)modPtr->baseAddr);

	    SymLoadModule(proc->hProcess, modPtr->hFile,
		0L, 0L, (DWORD) modPtr->baseAddr, 0);
	    modPtr->loaded = TRUE;
	}
    }


    if (proc->exeModule && proc->exeModule->dbgInfo && 
	proc->exeModule->dbgInfo->ImageFileName) {
	s = proc->exeModule->dbgInfo->ImageFileName;
    } else {
	s = "";
    }

    fatalException = pDebEvent->u.Exception.ExceptionRecord.ExceptionCode;

    {
	SIZE_T len;
	CHAR buffer[MAX_PATH+600];
	len = _snprintf(buffer, MAX_PATH+600,
			"\nA fatal, second-chance exception has occured in"
			" \"%s\".\n(0x%X) -- %s\n"
			"This is the backtrace:\n"
			"-------------------------------------",
			s, fatalException,
			GetExceptionCodeString(fatalException));
	WriteMasterError(buffer, len, ERROR_EXP_WIN32_BACKTRACE);
    }

    while (1) {
        pSymbol->SizeOfStruct = sizeof(symbolBuffer);
        pSymbol->MaxNameLength = 512;

	b = StackWalk(
#ifdef _WIN64
	    IMAGE_FILE_MACHINE_IA64,
#else
	    IMAGE_FILE_MACHINE_I386,
#endif
	    proc->hProcess,
	    tinfo->hThread, &frame, &context, 0L,
	    SymFunctionTableAccess, SymGetModuleBase,
	    0L);

	if (b == FALSE || frame.AddrPC.Offset == 0) {
	    break;
	}

	level++;

        if (SymGetSymFromAddr(proc->hProcess, frame.AddrPC.Offset,
	    &displacement, pSymbol) ) {
	    DWORD base, len;
	    CHAR buffer[256];

	    base = SymGetModuleBase(proc->hProcess, frame.AddrPC.Offset);
	    if (proc->moduleTable.Find((void *)base, &modPtr) != TCL_ERROR) {
		if (modPtr->dbgInfo && modPtr->dbgInfo->ImageFileName) {
		    s = modPtr->dbgInfo->ImageFileName;
		} else {
		    s = "";
		}
	    } else {
		s = "";
	    }
	    len = _snprintf(buffer, 256, "%u) %-15.15s %#08.8x\t%s + %u",
			    level, s, frame.AddrPC.Offset,
			    pSymbol->Name, displacement);
	    WriteMasterError(buffer, len, ERROR_EXP_WIN32_BACKTRACE);
	} else {
	    DWORD len;
	    CHAR buffer[32];
	    len = _snprintf(buffer, 32, "%08.8x", frame.AddrPC.Offset);
	    WriteMasterError(buffer, len, ERROR_EXP_WIN32_BACKTRACE);
	}
    }

    SymCleanup(proc->hProcess);
#else
#  error "Unsupported architecture"
#endif
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::GetExceptionCodeString --
 *
 *	Returns a string about the exception code.  Much more can
 *	be added.
 *
 *----------------------------------------------------------------------
 */

PCSTR
ConsoleDebugger::GetExceptionCodeString (DWORD exCode)
{
    switch (exCode) {
    case EXCEPTION_ACCESS_VIOLATION:
	return "Access Violation.";
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
	return "Array access was out-of-bounds.";
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
	return "Divide by zero.";
    case EXCEPTION_ILLEGAL_INSTRUCTION:
    case EXCEPTION_PRIV_INSTRUCTION:
	return "Illegal opcode.";
    case EXCEPTION_STACK_OVERFLOW:
	return "Stack overflow.";
    default:
	return "exception unknown.";
    }
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXCreateThread --
 *
 *	This routine is called when a CREATE_THREAD_DEBUG_EVENT
 *	occurs.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnXCreateThread(Process *proc, LPDEBUG_EVENT pDebEvent)
{
    ThreadInfo *threadInfo;

    threadInfo = new ThreadInfo;
    threadInfo->dwThreadId = pDebEvent->dwThreadId;
    threadInfo->hThread = pDebEvent->u.CreateThread.hThread;
    proc->threadCount++;
    threadInfo->lastBrkpt = 0L;
    threadInfo->nextPtr = proc->threadList;
    proc->threadList = threadInfo;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXDeleteThread --
 *
 *	This routine is called when a CREATE_THREAD_DEBUG_EVENT
 *	occurs.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnXDeleteThread(Process *proc, LPDEBUG_EVENT pDebEvent)
{
    ThreadInfo *threadInfo;
    ThreadInfo *prev;

    prev = 0L;
    for (threadInfo = proc->threadList; threadInfo;
	 prev = threadInfo, threadInfo = threadInfo->nextPtr) {
	if (threadInfo->dwThreadId == pDebEvent->dwThreadId) {
	    if (prev == 0L) {
		proc->threadList = threadInfo->nextPtr;
	    } else {
		prev->nextPtr = threadInfo->nextPtr;
	    }
	    proc->threadCount--;
	    /*
	     * Do not close the thread handle, as it can lead to a crash,
	     * as reported by Bhava Avula (bavula at broadcom.com) and
	     * Frank Ellis (fellis at extremenetworks.com).
	     * Triggered by spawning tclsh that in turn spawns telnet.
	     CloseHandle(threadInfo->hThread);
	     */
	    delete threadInfo;
	    break;
	}
    }
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXCreateProcess --
 *
 *	This routine is called when a CREATE_PROCESS_DEBUG_EVENT
 *	occurs.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnXCreateProcess(Process *proc, LPDEBUG_EVENT pDebEvent)
{
    ThreadInfo *threadInfo;
    CREATE_PROCESS_DEBUG_INFO *info = &pDebEvent->u.CreateProcessInfo;
    int known;

    // Save the first processes' start address.
    if (!pStartAddress) {
	pStartAddress = info->lpStartAddress;
    }

    if (proc == 0L) {
	proc = ProcessNew();
	proc->hProcess = info->hProcess;
	proc->pid = pDebEvent->dwProcessId;
    }

    known = LoadedModule(proc, info->hFile, info->lpImageName, info->fUnicode,
	    info->lpBaseOfImage, info->dwDebugInfoFileOffset);

    threadInfo = new ThreadInfo;
    threadInfo->dwThreadId = pDebEvent->dwThreadId;
    threadInfo->hThread = info->hThread;
    threadInfo->nextPtr = proc->threadList;
    proc->threadCount++;
    proc->threadList = threadInfo;
    /*
     * XXX: Is the CloseHandle correct?  This indicates it might be:
     * http://msdn.microsoft.com/library/en-us/debug/base/writing_the_debugger_s_main_loop.asp
     CloseHandle(info->hFile);
    */
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXLoadDll --
 *
 *	This routine is called when a LOAD_DLL_DEBUG_EVENT is seen
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Some information is printed
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnXLoadDll(Process *proc, LPDEBUG_EVENT pDebEvent)
{
    WORD w;
    DWORD dw;
    DWORD ImageHdrOffset;
    PIMAGE_FILE_HEADER pfh;	/* File header image in subprocess memory */
#if 0 // not used
    PIMAGE_SECTION_HEADER psh;
#endif
    PIMAGE_OPTIONAL_HEADER poh;
    IMAGE_DATA_DIRECTORY dataDir;
    PIMAGE_EXPORT_DIRECTORY ped;
    IMAGE_EXPORT_DIRECTORY exportDir;
    DWORD n;
    DWORD base;
    CHAR funcName[256];
    CHAR dllname[256];
    PVOID ptr, namePtr, funcPtr;
    DWORD p;
    LPLOAD_DLL_DEBUG_INFO info = &pDebEvent->u.LoadDll;
    BOOL bFound;

    int unknown = !LoadedModule(proc, info->hFile,
	info->lpImageName, info->fUnicode,
	info->lpBaseOfDll, info->dwDebugInfoFileOffset);

    base = (DWORD) info->lpBaseOfDll;

    // Check for the DOS signature
    //
    ReadSubprocessMemory(proc, info->lpBaseOfDll, &w, sizeof(WORD));
    if (w != IMAGE_DOS_SIGNATURE) return;
    
    // Skip over the DOS signature and check the PE signature
    //
    p = base;
    p += 15 * sizeof(DWORD);
    ptr = (PVOID) p;
    ReadSubprocessMemory(proc, (PVOID) p, &ImageHdrOffset, sizeof(DWORD));

    p = base;
    p += ImageHdrOffset;
    ReadSubprocessMemory(proc, (PVOID) p, &dw, sizeof(DWORD));
    if (dw != IMAGE_NT_SIGNATURE) {
	return;
    }
    ImageHdrOffset += sizeof(DWORD);
    p += sizeof(DWORD);

    pfh = (PIMAGE_FILE_HEADER) p;
    ptr = &pfh->SizeOfOptionalHeader;
    ReadSubprocessMemory(proc, ptr, &w, sizeof(WORD));

    // We want to find the exports section.  It can be found in the
    // data directory that is part of the IMAGE_OPTIONAL_HEADER
    //
    if (!w) return;
    p += sizeof(IMAGE_FILE_HEADER);
    poh = (PIMAGE_OPTIONAL_HEADER) p;

    // Find the number of entries in the data directory
    //
    ptr = &poh->NumberOfRvaAndSizes;
    ReadSubprocessMemory(proc, ptr, &dw, sizeof(DWORD));
    if (dw == 0) return;

    // Read the export data directory
    //
    ptr = &poh->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    ReadSubprocessMemory(proc, ptr, &dataDir, sizeof(IMAGE_DATA_DIRECTORY));

    // This points us to the exports section
    //
    ptr = (PVOID) (base + dataDir.VirtualAddress);
    ped = (PIMAGE_EXPORT_DIRECTORY) ptr;
    ReadSubprocessMemory(proc, ptr, &exportDir, sizeof(IMAGE_EXPORT_DIRECTORY));

    // See if this is a DLL we are interested in
    //
    ptr = &ped->Name;
    ReadSubprocessMemory(proc, ptr, &dw, sizeof(DWORD));
    ptr = (PVOID) (base + dw);
    ReadSubprocessStringA(proc, ptr, dllname, sizeof(dllname));

    if (dllname[0] == '\0') {
	// image has no export section, so get the name another way.
	// TODO: How??

    } else {
	int len;
	CHAR msg[MAX_PATH];

 	len = _snprintf(msg, MAX_PATH, "LOADED(%p): %s\n", proc, dllname);
 	WriteMasterWarning(msg, len);
    }

    bFound = FALSE;
    for (n = 0; BreakPoints[n].dllName; n++) {
	if (stricmp(dllname, BreakPoints[n].dllName) == 0) {
	    bFound = TRUE;
	    break;
	}
    }
    if (!bFound) {
	return;
    }

    ptr = (PVOID) (base + (DWORD) exportDir.AddressOfNames);
    for (n = 0; n < exportDir.NumberOfNames; n++) {
	ReadSubprocessMemory(proc, ptr, &dw, sizeof(DWORD));
	namePtr = (PVOID) (base + dw);

	// Now, we should hopefully have a pointer to the name of the
	// function, so lets get it.
	//
	ReadSubprocessStringA(proc, namePtr, funcName, sizeof(funcName));

#if 0
	// Keep a list of all function names in a hash table
	//
	funcPtr = (PVOID) (base + n*sizeof(DWORD) +
	    (DWORD) exportDir.AddressOfFunctions);
#else
	// Now use the AddressOfNameOrdinals table to match the name up with
	// an ordinal.  Usually funcOrd == n, but not when AddressOfFunctions
	// isn't sorted by name.
	//
	PVOID funcOrdPtr;
	WORD funcOrd;
	funcOrdPtr = (PVOID) (base + (n * sizeof(WORD)) +
			      (DWORD) exportDir.AddressOfNameOrdinals);
	ReadSubprocessMemory(proc, funcOrdPtr, &funcOrd, sizeof(WORD));

	// Keep a list of all function names in a hash table
	//
	funcPtr = (PVOID) (base + (funcOrd * sizeof(DWORD)) +
			   (DWORD) exportDir.AddressOfFunctions);
#endif
	ReadSubprocessMemory(proc, funcPtr, &dw, sizeof(DWORD));
	funcPtr = (PVOID) (base + dw);

	proc->funcTable.Add(funcName, funcPtr);

	ptr = (PVOID) (sizeof(DWORD) + (ULONG) ptr);
    }
    /*
     * XXX: Is the CloseHandle correct?  This indicates it might be:
     * http://msdn.microsoft.com/library/en-us/debug/base/writing_the_debugger_s_main_loop.asp
     CloseHandle(info->hFile);
    */
#if 0
    // The IMAGE_SECTION_HEADER comes after the IMAGE_OPTIONAL_HEADER
    // (if the IMAGE_OPTIONAL_HEADER exists)
    //
    p += w;

    psh = (PIMAGE_SECTION_HEADER) p;
#endif
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXUnloadDll --
 *
 *	This routine is called when a UNLOAD_DLL_DEBUG_EVENT is seen
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Some information is printed
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnXUnloadDll(Process *proc, LPDEBUG_EVENT pDebEvent)
{
    Module *modPtr;

    if (proc->moduleTable.Extract(pDebEvent->u.UnloadDll.lpBaseOfDll, &modPtr)
	!= TCL_ERROR) {
	if (modPtr->hFile) {
	    CloseHandle(modPtr->hFile);
	}
	if (modPtr->modName) {
	    delete [] modPtr->modName;
	}
	if (modPtr->dbgInfo) {
	    UnmapDebugInformation(modPtr->dbgInfo);
	}
	delete modPtr;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXDebugString --
 *
 *	This routine is called when a OUTPUT_DEBUG_STRING_EVENT
 *	happens.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Allocates a buffer for the string that is not cleared *here*.
 *
 *----------------------------------------------------------------------
 */
void
ConsoleDebugger::OnXDebugString(Process *proc, LPDEBUG_EVENT pDebEvent)
{
    CHAR *buffer;
    DWORD len;

    if (pDebEvent->u.DebugString.fUnicode) {
	int wlen = pDebEvent->u.DebugString.nDebugStringLength;
	WCHAR *wbuffer = new WCHAR [wlen + 1];
	ReadSubprocessStringW(proc,
		pDebEvent->u.DebugString.lpDebugStringData,
		wbuffer, wlen);
	len = WideCharToMultiByte(CP_ACP, 0, wbuffer, wlen, 0L, 0, 0L, 0L);
	buffer = new CHAR [len+1];
	WideCharToMultiByte(CP_ACP, 0, wbuffer, wlen, buffer, len, 0L, 0L);
	delete [] wbuffer;
    } else {
	len = pDebEvent->u.DebugString.nDebugStringLength;
	buffer = new CHAR [len+1];
	ReadSubprocessStringA(proc,
		pDebEvent->u.DebugString.lpDebugStringData,
		buffer, len);
    }

    WriteMasterWarning(buffer, len-1);
    delete [] buffer;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXRip --
 *
 *	Catches and reports RIP events (system error messages).
 *	Is RIP short for Rest-In-Peace??
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	allocates memory that is freed by the Message destructor.
 *
 *----------------------------------------------------------------------
 */
void
ConsoleDebugger::OnXRip(Process *proc, LPDEBUG_EVENT pDebEvent)
{
    LPCSTR msg;
    DWORD len;

    msg = ExpWinErrMsg(pDebEvent->u.RipInfo.dwError, proc->exeModule->modName, 0L);
    len = strlen(msg);
    if (pDebEvent->u.RipInfo.dwType == SLE_ERROR) {
	WriteMasterError(msg, len, ERROR_EXP_WIN32_FATAL_RIP);
    } else {
	WriteMasterError(msg, len, ERROR_EXP_WIN32_NONFATAL_RIP);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXBreakpoint --
 *
 *	This routine is called when an EXCEPTION_DEBUG_EVENT with
 *	an exception code of EXCEPTION_BREAKPOINT happens.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */

BOOL
ConsoleDebugger::OnXBreakpoint(Process *proc, LPDEBUG_EVENT pDebEvent)
{
    LPEXCEPTION_DEBUG_INFO exceptInfo;
    CONTEXT context;
    ThreadInfo *tinfo;
    Breakpoint *pbrkpt, *brkpt;
    PDWORD pdw;
    DWORD i;
    DWORD dw;

    for (tinfo = proc->threadList; tinfo != 0L; tinfo = tinfo->nextPtr) {
	if (pDebEvent->dwThreadId == tinfo->dwThreadId) {
	    break;
	}
    }

    if (tinfo == 0L) {
	/* breakpoint not known to us. */
	return TRUE;
    }

    exceptInfo = &pDebEvent->u.Exception;

    pbrkpt = 0L;
    for (brkpt = proc->brkptList; brkpt != 0L;
	 pbrkpt = brkpt, brkpt = brkpt->nextPtr) {
	if (brkpt->codePtr == exceptInfo->ExceptionRecord.ExceptionAddress) {
	    if (brkpt->threadInfo == 0L) {
		break;
	    }
	    if (brkpt->threadInfo == tinfo) {
		break;
	    }
	}
    }

    if (brkpt == 0L) {
	// shouldn't happen, but does..
	// ask Borland's td32.exe for a new OS shell in a seperate console and this happens.
	return TRUE;
    }

    context.ContextFlags = CONTEXT_FULL;
    GetThreadContext(tinfo->hThread, &context);

    if (!brkpt->returning) {
	Breakpoint *bpt;
	// Get the arguments to the function and store them in the thread
	// specific data structure.
	for (pdw = tinfo->args, i=0; i < brkpt->breakInfo->nargs; i++, pdw++) {
	    ReadSubprocessMemory(proc, (PVOID) (context.Esp+(4*(i+1))),
				 pdw, sizeof(DWORD));
	}
	tinfo->nargs = brkpt->breakInfo->nargs;
	tinfo->context = &context;

	if (brkpt->breakInfo->dwFlags & BREAK_IN) {
	    ((this)->*(brkpt->breakInfo->breakProc))(proc, tinfo, brkpt, &context.Eax, BREAK_IN);
	}

	// Only set a return breakpoint if something is interested
	// in the return value
	if (brkpt->breakInfo->dwFlags & BREAK_OUT) {
	    bpt = new Breakpoint;
	    ReadSubprocessMemory(proc, (PVOID) context.Esp,
		&bpt->origRetAddr, sizeof(DWORD));
	    dw = (DWORD) brkpt->codeReturnPtr;
	    WriteSubprocessMemory(proc, (PVOID) context.Esp,
		&dw, sizeof(DWORD));
	    bpt->codePtr = brkpt->codeReturnPtr;
	    bpt->returning = TRUE;
	    bpt->codeReturnPtr = 0L;	// Doesn't matter
	    bpt->breakInfo = brkpt->breakInfo;
	    bpt->threadInfo = tinfo;
	    bpt->nextPtr = proc->brkptList;
	    proc->brkptList = bpt;

	}

	// Now, we need to restore the original code for this breakpoint.
	// Put the program counter back, then do a single-step and put
	// the breakpoint back again.
	//
	WriteSubprocessMemory(proc, brkpt->codePtr, &brkpt->code, sizeof(BYTE));
	FlushInstructionCache(proc->hProcess, brkpt->codePtr, sizeof(BYTE));

	context.EFlags |= SINGLE_STEP_BIT;
	context.Eip--;

	// Save this per-thread.
	//
	tinfo->lastBrkpt = brkpt;
    } else {
	// Make the callback with the params and the return value
	if (brkpt->breakInfo->dwFlags & BREAK_OUT) {
	    ((this)->*(brkpt->breakInfo->breakProc))(proc, tinfo, brkpt, &context.Eax, BREAK_OUT);
	}
	context.Eip = brkpt->origRetAddr;

	if (pbrkpt == 0L) {
	    proc->brkptList = brkpt->nextPtr;
	} else {
	    pbrkpt->nextPtr = brkpt->nextPtr;
	}
	delete brkpt;
    }
    SetThreadContext(tinfo->hThread, &context);

    return TRUE;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::ReadSubprocessMemory --
 *
 *	Reads memory from the subprocess.  Takes care of all the
 *	issues with page protection.
 *
 * Results:
 *	FALSE if unsuccessful, TRUE if successful.
 *
 * Notes:
 *	Currently safe, but slow.
 *
 *----------------------------------------------------------------------
 */

BOOL
ConsoleDebugger::ReadSubprocessMemory(Process *proc, LPVOID addr, LPVOID buf, DWORD len)
{
    DWORD oldProtection = 0;
    MEMORY_BASIC_INFORMATION mbi;
    DWORD err = NO_ERROR;

    // if inaccessible or not committed memory, abort
    //
    if (!VirtualQueryEx(proc->hProcess, addr, &mbi,
	    sizeof(MEMORY_BASIC_INFORMATION)) || mbi.State != MEM_COMMIT) {
	goto error;
    }

    // On Win9x, special ranges can't have their protection changed.
    //
    if (dwPlatformId == VER_PLATFORM_WIN32_WINDOWS
	    && addr >= (LPVOID) 0x80000000 && addr <= (LPVOID) 0xBFFFFFFF) {
	if (!ReadProcessMemory(proc->hProcess, addr, buf, len, 0L)) {
	    goto error;
	}
    } else {
	// if guarded memory, change protection temporarily.
	//
	if (!(mbi.Protect & PAGE_READONLY) && !(mbi.Protect & PAGE_READWRITE)) {
	    VirtualProtectEx(proc->hProcess, addr, len, PAGE_READONLY,
		    &oldProtection);
	}

	if (!ReadProcessMemory(proc->hProcess, addr, buf, len, 0L)) {
	    err = GetLastError();
	}

	// reset protection if changed.
	//
	if (oldProtection) {
	    VirtualProtectEx(proc->hProcess, addr, len, oldProtection,
		    &oldProtection);
	    if (err != NO_ERROR) {
		SetLastError(err);
		goto error;
	    }
	}
    }
    return TRUE;

error:
    {
	LPCSTR msg = ExpWinErrMsg(GetLastError(), proc->exeModule->modName, 0L);
	int len = strlen(msg);
	WriteMasterError(msg, len, ERROR_EXP_WIN32_CANT_READ_SUB_MEM);
    }
    return FALSE;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::WriteSubprocessMemory --
 *
 *	Writes memory from the subprocess.  Takes care of all the
 *	issues with page protection.
 *
 * Results:
 *	zero if unsuccessful, non-zero if successful.
 *
 *----------------------------------------------------------------------
 */

BOOL
ConsoleDebugger::WriteSubprocessMemory(Process *proc, LPVOID addr, LPVOID buf, DWORD len)
{
    DWORD oldProtection = 0;
    MEMORY_BASIC_INFORMATION mbi;
    BOOL ret = TRUE;
    DWORD err = ERROR_SUCCESS;

    // if inaccessible or not committed memory, abort
    if (!VirtualQueryEx(proc->hProcess, addr, &mbi,
			sizeof(MEMORY_BASIC_INFORMATION))
	|| mbi.State != MEM_COMMIT) {
	return FALSE;
    }

    // On Win9x, special ranges can't have their protection changed.
    //
    if (dwPlatformId == VER_PLATFORM_WIN32_WINDOWS
	    && addr >= (LPVOID) 0x80000000 && addr <= (LPVOID) 0xBFFFFFFF) {
	ret = WriteProcessMemory(proc->hProcess, addr, buf, len, 0L);
    } else {
	// if guarded memory, change protection temporarily.
	//
	if (!(mbi.Protect & PAGE_READWRITE || mbi.Protect & PAGE_EXECUTE_READWRITE)) {
	    if (!VirtualProtectEx(proc->hProcess, addr, len, PAGE_READWRITE,
		    &oldProtection)) {
		return FALSE;
	    }
	}

	if (!WriteProcessMemory(proc->hProcess, addr, buf, len, 0L)) {
	    ret = FALSE;
	    err = GetLastError();
	}

	// reset protection if changed.
	//
	if (oldProtection) {
	    VirtualProtectEx(proc->hProcess, addr, len, oldProtection,
		    &oldProtection);
	    SetLastError(err);
	}
    }
    return ret;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::ReadSubprocessStringA --
 *
 *	Read a character string from the subprocess
 *
 * Results:
 *	The length of the string
 *
 *----------------------------------------------------------------------
 */

int
ConsoleDebugger::ReadSubprocessStringA(Process *proc, PVOID base,
    PCHAR buf, int buflen)
{
    CHAR *ip, *op;
    int i;
    
    ip = static_cast<CHAR *>(base);
    op = buf;
    i = 0;
    while (i < buflen-1) {
	if (! ReadSubprocessMemory(proc, ip, op, sizeof(CHAR))) {
	    break;
	}
	if (*op == 0) break;
	op++; ip++; i++;
    }
    *op = 0;
    return i;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::ReadSubprocessStringW --
 *
 *	Read a character string from the subprocess
 *
 * Results:
 *	The length of the string
 *
 *----------------------------------------------------------------------
 */

int
ConsoleDebugger::ReadSubprocessStringW(Process *proc, PVOID base,
    PWCHAR buf, int buflen)
{
    WCHAR *ip, *op;
    int i;
    
    ip = static_cast<WCHAR *>(base);
    op = buf;
    i = 0;
    while (i < buflen-1) {
	if (! ReadSubprocessMemory(proc, ip, op, sizeof(WCHAR))) {
	    break;
	}
	if (*op == 0) break;
	op++; ip++; i++;
    }
    *op = 0;
    return i;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::LoadedModule --
 *
 *	A module with the specifed name was loaded.  Add it to our
 *	list of loaded modules and print any debugging information
 *	if debugging is enabled.
 *
 * Results:
 *	If the module is known, return TRUE.  Otherwise, return FALSE
 *
 *----------------------------------------------------------------------
 */

int
ConsoleDebugger::LoadedModule(Process *proc, HANDLE hFile, LPVOID modname,
    int isUnicode, LPVOID baseAddr, DWORD debugOffset)
{
    int known = 1;
    PVOID ptr;
    char mbstr[512];
    char *s = 0L;
    Module *modPtr;

    if (modname) {
	// This modname is a pointer to the name of the
	// DLL in the process space of the subprocess
	//
	if (ReadSubprocessMemory(proc, modname, &ptr, sizeof(PVOID)) && ptr) {
	    if (isUnicode) {
		WCHAR name[MAX_PATH];
		int len;
		ReadSubprocessStringW(proc, ptr, name, 512);
		len = WideCharToMultiByte(CP_ACP, 0, name, -1, 0L, 0, 0L, 0L);
		s = new char [len + 1];
		WideCharToMultiByte(CP_ACP, 0, name, -1, s, len, 0L, 0L);
	    } else {
		ReadSubprocessStringA(proc, ptr, mbstr, sizeof(mbstr));
		s = new char [strlen(mbstr) + 1];
		strcpy(s, mbstr);
	    }
	} else {
	    known = 0;
	}
    }

    modPtr = new Module;
    modPtr->loaded = FALSE;
    modPtr->hFile = hFile;
    modPtr->baseAddr = baseAddr;
    modPtr->modName = s;
    modPtr->dbgInfo = 0L;
    if (proc->exeModule == 0L) {
	proc->exeModule = modPtr;
    }

    proc->moduleTable.Add(baseAddr, modPtr);

    return known;
}

BOOL
ConsoleDebugger::MakeSubprocessMemory(Process *proc, SIZE_T amount,
    LPVOID *pBuff, DWORD access)
{
    if (dwPlatformId == VER_PLATFORM_WIN32_NT) {
        if (!(*pBuff = pfnVirtualAllocEx(proc->hProcess, 0, amount, MEM_COMMIT,
		access))) {
	    goto error;
	}
    } else {
        // In Windows 9X, create a small memory mapped file.  On this
        // platform, memory mapped files are above 2GB, and thus are
        // accessible to all processes.
	//
        HANDLE hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, 0,
		access | SEC_COMMIT, 0, amount, 0);

	if (!hFileMapping) {
	    goto error;
	}

	LPVOID buffer = MapViewOfFile(hFileMapping, FILE_MAP_WRITE, 0, 0,
		amount);

	if (!buffer) {
	    goto error;
	}

	// Save the association
	//
	spMemMapping.Add(buffer, hFileMapping);
	*pBuff = buffer;
    }
    return TRUE;

error:
    {
	LPCSTR msg = ExpWinErrMsg(GetLastError(), proc->exeModule->modName, 0L);
	int len = strlen(msg);
	WriteMasterError(msg, len, ERROR_EXP_WIN32_CANT_MAKE_SUB_MEM);
    }
    return FALSE;
}

BOOL
ConsoleDebugger::RemoveSubprocessMemory(Process *proc, LPVOID buff)
{
    BOOL ret;

    if (dwPlatformId == VER_PLATFORM_WIN32_NT) {
	ret = pfnVirtualFreeEx(proc->hProcess, buff, 0, MEM_RELEASE);
    } else {
	HANDLE hFileMapping;
	if (spMemMapping.Extract(buff, &hFileMapping) != TCL_OK) {
	    SetLastError(ERROR_FILE_NOT_FOUND);
	    return FALSE;
	}
	ret = UnmapViewOfFile(buff);
	CloseHandle(hFileMapping);
    }
    return ret;
}

/*
 *----------------------------------------------------------------------
 *
 * RefreshScreen --
 *
 *	Redraw the entire screen
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */
#if 0
void
ConsoleDebugger::RefreshScreen(void)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    CHAR buf[4096];
    DWORD bufpos = 0;
    CHAR_INFO consoleBuf[4096];
    COORD size = {ConsoleSize.X, ConsoleSize.Y};
    COORD begin = {0, 0};
    SMALL_RECT rect = {0, 0, ConsoleSize.X-1, ConsoleSize.Y-1};
    int x, y, prespaces, postspaces, offset;

    // Clear the screen
    bufpos += wsprintf(&buf[bufpos], "\033[2J");
    bufpos += wsprintf(&buf[bufpos], "\033[%d;%dH", CursorPosition.Y+1,
	    CursorPosition.X+1);
    CursorKnown = TRUE;

    WriteMasterCopy(buf, bufpos);
    bufpos = 0;

//    if (GetConsoleScreenBufferInfo(HConsole, &info) != FALSE) {
//	return;
//    }

    CursorPosition = info.dwCursorPosition;

//    if (! ReadConsoleOutput(HConsole, consoleBuf, size, begin, &rect)) {
//	return;
//    }

    offset = 0;
    for (y = 0; y < ConsoleSize.Y; y++) {
	offset += ConsoleSize.X;
	for (x = 0; x < ConsoleSize.X; x++) {
	    if (consoleBuf[offset+x].Char.AsciiChar != ' ') {
		break;
	    }
	}
	prespaces = x;
	if (prespaces == ConsoleSize.X) {
	    continue;
	}

	for (x = ConsoleSize.X-1; x >= 0; x--) {
	    if (consoleBuf[offset+x].Char.AsciiChar != ' ') {
		break;
	    }
	}
	postspaces = x;
	bufpos += wsprintf(&buf[bufpos], "\033[%d;%dH", y+1, prespaces+1);

	for (x = prespaces; x < postspaces; x++) {
	    buf[bufpos++] = consoleBuf[offset+x].Char.AsciiChar;
	}
    }

    bufpos += wsprintf(&buf[bufpos], "\033[%d;%dH", CursorPosition.Y+1,
	    CursorPosition.X+1);
    CursorKnown = TRUE;
    WriteMasterCopy(buf, bufpos);
}
#endif

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::ProcessNew --
 *
 *	Allocates a new structure for debugging a process and
 *	initializes it.
 *
 * Results:
 *	A new structure
 *
 * Side Effects:
 *	Memory is allocated, an event is created.
 *
 *----------------------------------------------------------------------
 */

ConsoleDebugger::Process *
ConsoleDebugger::ProcessNew(void)
{
    Process *proc;
    proc = new Process;
    proc->nextPtr = ProcessList;
    ProcessList = proc;
    return proc;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::ProcessFree --
 *
 *	Frees all allocated memory for a process and closes any
 *	open handles
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::ProcessFree(Process *proc)
{
    ThreadInfo *tcurr, *tnext;
    Breakpoint *bcurr, *bnext;
    Process *pcurr, *pprev;

    for (tcurr = proc->threadList; tcurr != 0L; tcurr = tnext) {
	tnext = tcurr->nextPtr;
	proc->threadCount--;
	CloseHandle(tcurr->hThread);
	delete tcurr;
    }
    for (bcurr = proc->brkptList; bcurr != 0L; bcurr = bnext) {
	bnext = bcurr->nextPtr;
	delete bcurr;
    }
    for (pprev = 0L, pcurr = ProcessList; pcurr != 0L;
	 pcurr = pcurr->nextPtr)
    {
	if (pcurr == proc) {
	    if (pprev == 0L) {
		ProcessList = pcurr->nextPtr;
	    } else {
		pprev->nextPtr = pcurr->nextPtr;
	    }
	    break;
	}
    }
    CloseHandle(proc->hProcess);

    pidKilled = 1;
    delete proc;
}

void
ConsoleDebugger::WriteMaster(LPCSTR buf, SIZE_T len)
{
    Message *msg;

    // avoid zero byte reads!
    if (len == 0) return;

    msg = new Message;
    msg->type = Message::TYPE_NORMAL;
    msg->bytes = new BYTE [len];
    memcpy(msg->bytes, buf, len);
    msg->length = len;
#ifdef EXP_DEBUG
    {
	char b[1024];
	_snprintf(b, 1024, "WriteMaster[%d]: %s",
		len, buf);
	OutputDebugString(b);
    }
#endif
    mQ.PutOnTailOfList(msg);
    callback.AlertReadable();
}

void
ConsoleDebugger::WriteMasterWarning(LPCSTR buf, SIZE_T len)
{
    // Just report this in debugging mode
    OutputDebugString(buf);
}

void
ConsoleDebugger::WriteMasterError(LPCSTR buf, SIZE_T len, DWORD status)
{
    Message *msg;
    msg = new Message;
    msg->type = Message::TYPE_ERROR;
    msg->bytes = new BYTE [len];
    memcpy(msg->bytes, buf, len);
    msg->length = len;
    msg->status = status;
    eQ.PutOnTailOfList(msg);
}

void
ConsoleDebugger::NotifyDone()
{
    Message *msg;
    msg = new Message;
    msg->type = Message::TYPE_SLAVEDONE;
    msg->bytes = 0L;
    msg->length = 0;
    mQ.PutOnTailOfList(msg);
    callback.AlertReadable();
}

DWORD
ConsoleDebugger::Write (IPCMsg *msg)
{
    DWORD result = NO_ERROR;

    /*
     * Also check to see if the pid is already dead through other means
     * (like an outside kill). [Bug 33826]
     */
    if (injectorIPC == 0L || pidKilled) {
	result = ERROR_BROKEN_PIPE;
    } else if (! injectorIPC->Post(msg)) {
	result = injectorIPC->Status();
    }

    return result;
}

void
ConsoleDebugger::EnterInteract (HANDLE OutConsole)
{
    bpCritSec.Enter();

//    interactingConsole = OutConsole;

    // More stuff to do here... What?
    // Copy entire screen contents, how?
    // Set interactingConsole to the proper size?
    // more ???  help!

    interacting = true;
    bpCritSec.Leave();
}

void
ConsoleDebugger::ExitInteract ()
{
//    interactingConsole = 0L;
    interacting = false;
}

const DWORD ConsoleDebugger::KEY_CONTROL= 0;
const DWORD ConsoleDebugger::KEY_SHIFT	= 1;
const DWORD ConsoleDebugger::KEY_LSHIFT	= 1;
const DWORD ConsoleDebugger::KEY_RSHIFT	= 2;
const DWORD ConsoleDebugger::KEY_ALT	= 3;

const ConsoleDebugger::KEY_MATRIX ConsoleDebugger::ModifierKeyArray[] = {
/* Control */	{ 17,  29, 0},
/* LShift */	{ 16,  42, 0},
/* RShift */	{ 16,  54, 0},
/* Alt */	{ 18,  56, 0}
};

const ConsoleDebugger::KEY_MATRIX ConsoleDebugger::AsciiToKeyArray[] = {
/*   0 */	{ 50,   3, RIGHT_CTRL_PRESSED|SHIFT_PRESSED},
/*   1 */	{ 65,  30, RIGHT_CTRL_PRESSED},
/*   2 */	{ 66,  48, RIGHT_CTRL_PRESSED},
/*   3 */	{ 67,  46, RIGHT_CTRL_PRESSED},
/*   4 */	{ 68,  32, RIGHT_CTRL_PRESSED},
/*   5 */	{ 69,  18, RIGHT_CTRL_PRESSED},
/*   6 */	{ 70,  33, RIGHT_CTRL_PRESSED},
/*   7 */	{ 71,  34, RIGHT_CTRL_PRESSED},
/*   8 */	{ 72,  35, RIGHT_CTRL_PRESSED},
/*   9 */	{  9,  15, RIGHT_CTRL_PRESSED},
/*  10 */	{ 74,  36, RIGHT_CTRL_PRESSED},
/*  11 */	{ 75,  37, RIGHT_CTRL_PRESSED},
/*  12 */	{ 76,  38, RIGHT_CTRL_PRESSED},
/*  13 */	{ 13,  28, 0},
/*  14 */	{ 78,  49, RIGHT_CTRL_PRESSED},
/*  15 */	{ 79,  24, RIGHT_CTRL_PRESSED},
/*  16 */	{ 80,  25, RIGHT_CTRL_PRESSED},
/*  17 */	{ 81,  16, RIGHT_CTRL_PRESSED},
/*  18 */	{ 82,  19, RIGHT_CTRL_PRESSED},
/*  19 */	{ 83,  31, RIGHT_CTRL_PRESSED},
/*  20 */	{ 84,  20, RIGHT_CTRL_PRESSED},
/*  21 */	{ 85,  22, RIGHT_CTRL_PRESSED},
/*  22 */	{ 86,  47, RIGHT_CTRL_PRESSED},
/*  23 */	{ 87,  17, RIGHT_CTRL_PRESSED},
/*  24 */	{ 88,  45, RIGHT_CTRL_PRESSED},
/*  25 */	{ 89,  21, RIGHT_CTRL_PRESSED},
/*  26 */	{ 90,  44, RIGHT_CTRL_PRESSED},
/*  27 */	{219, 219, RIGHT_CTRL_PRESSED|SHIFT_PRESSED},
/*  28 */	{220, 220, RIGHT_CTRL_PRESSED|SHIFT_PRESSED},
/*  29 */	{221, 221, RIGHT_CTRL_PRESSED|SHIFT_PRESSED},
/*  30 */	{ 54,  54, RIGHT_CTRL_PRESSED|SHIFT_PRESSED},
/*  31 */	{189, 189, RIGHT_CTRL_PRESSED|SHIFT_PRESSED},
/*  32 */	{ 32,  32, 0},
/*  33 */	{ 49,  49, SHIFT_PRESSED},
/*  34 */	{222, 222, SHIFT_PRESSED},
/*  35 */	{ 51,  51, SHIFT_PRESSED},
/*  36 */	{ 52,  52, SHIFT_PRESSED},
/*  37 */	{ 53,  53, SHIFT_PRESSED},
/*  38 */	{ 55,  55, SHIFT_PRESSED},
/*  39 */	{222, 222, 0},
/*  40 */	{ 57,  57, SHIFT_PRESSED},
/*  41 */	{ 48,  48, SHIFT_PRESSED},
/*  42 */	{ 56,  56, SHIFT_PRESSED},
/*  43 */	{187, 187, SHIFT_PRESSED},
/*  44 */	{188, 188, 0},
/*  45 */	{189, 189, SHIFT_PRESSED},
/*  46 */	{190, 190, 0},
/*  47 */	{191, 191, 0},
/*  48 */	{ 48,  48, 0},
/*  49 */	{ 49,  49, 0},
/*  50 */	{ 50,   3, 0},
/*  51 */	{ 51,  51, 0},
/*  52 */	{ 52,  52, 0},
/*  53 */	{ 53,  53, 0},
/*  54 */	{ 54,  54, 0},
/*  55 */	{ 55,  55, 0},
/*  56 */	{ 56,  56, 0},
/*  57 */	{ 57,  57, 0},
/*  58 */	{186, 186, SHIFT_PRESSED},
/*  59 */	{186, 186, 0},
/*  60 */	{188, 188, SHIFT_PRESSED},
/*  61 */	{187, 187, SHIFT_PRESSED},
/*  62 */	{190, 190, SHIFT_PRESSED},
/*  63 */	{191, 191, SHIFT_PRESSED},
/*  64 */	{ 50,   3, 0},
/*  65 */	{ 65,  30, SHIFT_PRESSED},
/*  66 */	{ 66,  48, SHIFT_PRESSED},
/*  67 */	{ 67,  46, SHIFT_PRESSED},
/*  68 */	{ 68,  32, SHIFT_PRESSED},
/*  69 */	{ 69,  18, SHIFT_PRESSED},
/*  70 */	{ 70,  33, SHIFT_PRESSED},
/*  71 */	{ 71,  34, SHIFT_PRESSED},
/*  72 */	{ 72,  35, SHIFT_PRESSED},
/*  73 */	{ 73,  23, SHIFT_PRESSED},
/*  74 */	{ 74,  36, SHIFT_PRESSED},
/*  75 */	{ 75,  37, SHIFT_PRESSED},
/*  76 */	{ 76,  38, SHIFT_PRESSED},
/*  77 */	{ 77,  50, SHIFT_PRESSED},
/*  78 */	{ 78,  49, SHIFT_PRESSED},
/*  79 */	{ 79,  24, SHIFT_PRESSED},
/*  80 */	{ 80,  25, SHIFT_PRESSED},
/*  81 */	{ 81,  16, SHIFT_PRESSED},
/*  82 */	{ 82,  19, SHIFT_PRESSED},
/*  83 */	{ 83,  31, SHIFT_PRESSED},
/*  84 */	{ 84,  20, SHIFT_PRESSED},
/*  85 */	{ 85,  22, SHIFT_PRESSED},
/*  86 */	{ 86,  47, SHIFT_PRESSED},
/*  87 */	{ 87,  17, SHIFT_PRESSED},
/*  88 */	{ 88,  45, SHIFT_PRESSED},
/*  89 */	{ 89,  21, SHIFT_PRESSED},
/*  90 */	{ 90,  44, SHIFT_PRESSED},
/*  91 */	{219, 219, 0},
/*  92 */	{220, 220, 0},
/*  93 */	{221, 221, 0},
/*  94 */	{ 54,  54, SHIFT_PRESSED},
/*  95 */	{189, 189, SHIFT_PRESSED},
/*  96 */	{192, 192, 0},
/*  97 */	{ 65,  30, 0},
/*  98 */	{ 66,  48, 0},
/*  99 */	{ 67,  46, 0},
/* 100 */	{ 68,  32, 0},
/* 101 */	{ 69,  18, 0},
/* 102 */	{ 70,  33, 0},
/* 103 */	{ 71,  34, 0},
/* 104 */	{ 72,  35, 0},
/* 105 */	{ 73,  23, 0},
/* 106 */	{ 74,  36, 0},
/* 107 */	{ 75,  37, 0},
/* 108 */	{ 76,  38, 0},
/* 109 */	{ 77,  50, 0},
/* 110 */	{ 78,  49, 0},
/* 111 */	{ 79,  24, 0},
/* 112 */	{ 80,  25, 0},
/* 113 */	{ 81,  16, 0},
/* 114 */	{ 82,  19, 0},
/* 115 */	{ 83,  31, 0},
/* 116 */	{ 84,  20, 0},
/* 117 */	{ 85,  22, 0},
/* 118 */	{ 86,  47, 0},
/* 119 */	{ 87,  17, 0},
/* 120 */	{ 88,  45, 0},
/* 121 */	{ 89,  21, 0},
/* 122 */	{ 90,  44, 0},
/* 123 */	{219, 219, SHIFT_PRESSED},
/* 124 */	{220, 220, SHIFT_PRESSED},
/* 125 */	{221, 221, SHIFT_PRESSED},
/* 126 */	{192, 192, SHIFT_PRESSED},
#if 0
/* 127 */	{  8,  14, RIGHT_CTRL_PRESSED}
#else
/* Delete */	{ VK_DELETE, 83, 0}
#endif
};

const ConsoleDebugger::KEY_MATRIX ConsoleDebugger::FunctionToKeyArray[] = {
/* Cursor Up */	    {VK_UP,      72,	0},
/* Cursor Down */   {VK_DOWN,    80,	0},
/* Cursor Right */  {VK_RIGHT,   77,	0},
/* Cursor Left */   {VK_LEFT,    75,	0},
/* End */	    {VK_END,     79,	0},
/* Home */	    {VK_HOME,    71,	0},
/* PageUp */	    {VK_PRIOR,   73,	0},
/* PageDown */	    {VK_NEXT,    81,	0},
/* Insert */	    {VK_INSERT,  82,	0},
/* Delete */	    {VK_DELETE,  83,	0},
/* Select */	    {VK_SELECT,   0,	0},
/* F1 */	    {VK_F1,      59,	0},
/* F2 */	    {VK_F2,      60,	0},
/* F3 */	    {VK_F3,      61,	0},
/* F4 */	    {VK_F4,      62,	0},
/* F5 */	    {VK_F5,      63,	0},
/* F6 */	    {VK_F6,      64,	0},
/* F7 */	    {VK_F7,      65,	0},
/* F8 */	    {VK_F8,      66,	0},
/* F9 */	    {VK_F9,      67,	0},
/* F10 */	    {VK_F10,     68,	0},
/* F11 */	    {VK_F11,     87,	0},
/* F12 */	    {VK_F12,     88,	0},
/* F13 */	    {VK_F13,      0,	0},
/* F14 */	    {VK_F14,      0,	0},
/* F15 */	    {VK_F15,      0,	0},
/* F16 */	    {VK_F16,      0,	0},
/* F17 */	    {VK_F17,      0,	0},
/* F18 */	    {VK_F18,      0,	0},
/* F19 */	    {VK_F19,      0,	0},
/* F20 */	    {VK_F20,      0,	0}
};

const DWORD ConsoleDebugger::KEY_UP	= 0;
const DWORD ConsoleDebugger::KEY_DOWN	= 1;
const DWORD ConsoleDebugger::KEY_RIGHT	= 2;
const DWORD ConsoleDebugger::KEY_LEFT	= 3;
const DWORD ConsoleDebugger::KEY_END	= 4;
const DWORD ConsoleDebugger::KEY_HOME	= 5;
const DWORD ConsoleDebugger::KEY_PAGEUP	= 6;
const DWORD ConsoleDebugger::KEY_PAGEDOWN= 7;
const DWORD ConsoleDebugger::KEY_INSERT	= 8;
const DWORD ConsoleDebugger::KEY_DELETE	= 9;
const DWORD ConsoleDebugger::KEY_SELECT	= 10;
const DWORD ConsoleDebugger::KEY_F1	= 11;
const DWORD ConsoleDebugger::KEY_F2	= 12;
const DWORD ConsoleDebugger::KEY_F3	= 13;
const DWORD ConsoleDebugger::KEY_F4	= 14;
const DWORD ConsoleDebugger::KEY_F5	= 15;
const DWORD ConsoleDebugger::KEY_F6	= 16;
const DWORD ConsoleDebugger::KEY_F7	= 17;
const DWORD ConsoleDebugger::KEY_F8	= 18;
const DWORD ConsoleDebugger::KEY_F9	= 19;
const DWORD ConsoleDebugger::KEY_F10	= 20;
const DWORD ConsoleDebugger::KEY_F11	= 21;
const DWORD ConsoleDebugger::KEY_F12	= 22;
const DWORD ConsoleDebugger::KEY_F13	= 23;
const DWORD ConsoleDebugger::KEY_F14	= 24;
const DWORD ConsoleDebugger::KEY_F15	= 25;
const DWORD ConsoleDebugger::KEY_F16	= 26;
const DWORD ConsoleDebugger::KEY_F17	= 27;
const DWORD ConsoleDebugger::KEY_F18	= 28;
const DWORD ConsoleDebugger::KEY_F19	= 29;
const DWORD ConsoleDebugger::KEY_F20	= 30;
const DWORD ConsoleDebugger::WIN_RESIZE	= 31;

const ConsoleDebugger::FUNCTION_KEY ConsoleDebugger::VtFunctionKeys[] = {
    {"OP",	KEY_F1},
    {"OQ",	KEY_F2},
    {"OR",	KEY_F3},
    {"OS",	KEY_F4},
    {"[A",	KEY_UP},
    {"[B",	KEY_DOWN},
    {"[C",	KEY_RIGHT},
    {"[D",	KEY_LEFT},
    {"[F",	KEY_END},
    {"[H",	KEY_HOME},
    {"[2~",	KEY_INSERT},
    {"[3~",	KEY_DELETE},
    {"[4~",	KEY_SELECT},
    {"[5~",	KEY_PAGEUP},
    {"[6~",	KEY_PAGEDOWN},
    {"[11~",	KEY_F1},
    {"[12~",	KEY_F2},
    {"[13~",	KEY_F3},
    {"[14~",	KEY_F4},
    {"[15~",	KEY_F5},
    {"[17~",	KEY_F6},
    {"[18~",	KEY_F7},
    {"[19~",	KEY_F8},
    {"[20~",	KEY_F9},
    {"[21~",	KEY_F10},
    {"[23~",	KEY_F11},
    {"[24~",	KEY_F12},
    {"[25~",	KEY_F13},
    {"[26~",	KEY_F14},
    {"[28~",	KEY_F15},
    {"[29~",	KEY_F16},
    {"[31~",	KEY_F17},
    {"[32~",	KEY_F18},
    {"[33~",	KEY_F19},
    {"[34~",	KEY_F20},
    {"[39~",	WIN_RESIZE},
    {0L,	0}
};

DWORD
ConsoleDebugger::MapCharToIRs (CHAR c)
{
#ifndef IPC_MAXRECORDS // Only used if we send single key events
    UCHAR lc;
    DWORD mods, result;
    IPCMsg msg;

    /* strip off the upper 127 */
    lc = (UCHAR) (c & 0x7f);
    mods = AsciiToKeyArray[lc].dwControlKeyState;

    msg.type = IRECORD;
    msg.irecord.EventType = KEY_EVENT;

#if 0
    if (mods & RIGHT_CTRL_PRESSED) {
	/* First, generate a control key press */
	msg.irecord.Event.KeyEvent.bKeyDown = TRUE;
	msg.irecord.Event.KeyEvent.wRepeatCount = 1;
	msg.irecord.Event.KeyEvent.wVirtualKeyCode =
		ModifierKeyArray[KEY_CONTROL].wVirtualKeyCode;
	msg.irecord.Event.KeyEvent.wVirtualScanCode =
		ModifierKeyArray[KEY_CONTROL].wVirtualScanCode;
	msg.irecord.Event.KeyEvent.uChar.AsciiChar = 0;
	msg.irecord.Event.KeyEvent.dwControlKeyState = RIGHT_CTRL_PRESSED;
	result = Write(&msg);
	if (result != NO_ERROR) return result;
    }
    if (mods & SHIFT_PRESSED) {
	/* First, generate a control key press */
	msg.irecord.Event.KeyEvent.bKeyDown = TRUE;
	msg.irecord.Event.KeyEvent.wVirtualKeyCode =
		ModifierKeyArray[KEY_SHIFT].wVirtualKeyCode;
	msg.irecord.Event.KeyEvent.wVirtualScanCode =
		ModifierKeyArray[KEY_SHIFT].wVirtualScanCode;
	msg.irecord.Event.KeyEvent.uChar.AsciiChar = 0;
	msg.irecord.Event.KeyEvent.dwControlKeyState = mods;
	result = Write(&msg);
	if (result != NO_ERROR) return result;
    }
#endif

    /* keydown first. */
    msg.irecord.Event.KeyEvent.bKeyDown = TRUE;
    msg.irecord.Event.KeyEvent.wRepeatCount = 1;
    msg.irecord.Event.KeyEvent.wVirtualKeyCode =
	    AsciiToKeyArray[lc].wVirtualKeyCode;
    msg.irecord.Event.KeyEvent.wVirtualScanCode =
	    AsciiToKeyArray[lc].wVirtualScanCode;
    msg.irecord.Event.KeyEvent.dwControlKeyState =
	    AsciiToKeyArray[lc].dwControlKeyState;
    msg.irecord.Event.KeyEvent.uChar.AsciiChar = c;
    result = Write(&msg);
    if (result != NO_ERROR) return result;

    /* now keyup. */
    msg.irecord.Event.KeyEvent.bKeyDown = FALSE;
    msg.irecord.Event.KeyEvent.wRepeatCount = 1;
    msg.irecord.Event.KeyEvent.wVirtualKeyCode =
	    AsciiToKeyArray[lc].wVirtualKeyCode;
    msg.irecord.Event.KeyEvent.wVirtualScanCode =
	    AsciiToKeyArray[lc].wVirtualScanCode;
    msg.irecord.Event.KeyEvent.dwControlKeyState =
	    AsciiToKeyArray[lc].dwControlKeyState;
    msg.irecord.Event.KeyEvent.uChar.AsciiChar = c;
    result = Write(&msg);
    if (result != NO_ERROR) return result;

#if 0
    if (mods & SHIFT_PRESSED) {
	/* First, generate a control key press */
	msg.irecord.Event.KeyEvent.bKeyDown = FALSE;
	msg.irecord.Event.KeyEvent.wVirtualKeyCode =
		ModifierKeyArray[KEY_SHIFT].wVirtualKeyCode;
	msg.irecord.Event.KeyEvent.wVirtualScanCode =
		ModifierKeyArray[KEY_SHIFT].wVirtualScanCode;
	msg.irecord.Event.KeyEvent.uChar.AsciiChar = 0;
	msg.irecord.Event.KeyEvent.dwControlKeyState = mods & ~SHIFT_PRESSED;
	result = Write(&msg);
	if (result != NO_ERROR) return result;
    }
    if (mods & RIGHT_CTRL_PRESSED) {
	/* First, generate a control key press */
	msg.irecord.Event.KeyEvent.bKeyDown = FALSE;
	msg.irecord.Event.KeyEvent.wVirtualKeyCode =
		ModifierKeyArray[KEY_CONTROL].wVirtualKeyCode;
	msg.irecord.Event.KeyEvent.wVirtualScanCode =
		ModifierKeyArray[KEY_CONTROL].wVirtualScanCode;
	msg.irecord.Event.KeyEvent.uChar.AsciiChar = 0;
	msg.irecord.Event.KeyEvent.dwControlKeyState = 0;
	result = Write(&msg);
	if (result != NO_ERROR) return result;
    }
#endif
#endif // IPC_MAXRECORDS
    return NO_ERROR;
}

#ifndef IPC_MAXRECORDS // not used
DWORD
ConsoleDebugger::MapFKeyToIRs(DWORD fk)
{
    IPCMsg msg;
    DWORD result;

    msg.type = IRECORD;
    msg.irecord.EventType = KEY_EVENT;

    /* keydown first. */
    msg.irecord.Event.KeyEvent.bKeyDown = TRUE;
    msg.irecord.Event.KeyEvent.wRepeatCount = 1;
    msg.irecord.Event.KeyEvent.wVirtualKeyCode =
	    FunctionToKeyArray[fk].wVirtualKeyCode;
    msg.irecord.Event.KeyEvent.wVirtualScanCode =
	    FunctionToKeyArray[fk].wVirtualScanCode;
    msg.irecord.Event.KeyEvent.dwControlKeyState =
	    FunctionToKeyArray[fk].dwControlKeyState;
    msg.irecord.Event.KeyEvent.uChar.AsciiChar = 0;
    result = Write(&msg);
    if (result != NO_ERROR) return result;

    /* now keyup. */
    msg.irecord.Event.KeyEvent.bKeyDown = FALSE;
    msg.irecord.Event.KeyEvent.wRepeatCount = 1;
    msg.irecord.Event.KeyEvent.wVirtualKeyCode =
	    FunctionToKeyArray[fk].wVirtualKeyCode;
    msg.irecord.Event.KeyEvent.wVirtualScanCode =
	    FunctionToKeyArray[fk].wVirtualScanCode;
    msg.irecord.Event.KeyEvent.dwControlKeyState =
	    FunctionToKeyArray[fk].dwControlKeyState;
    msg.irecord.Event.KeyEvent.uChar.AsciiChar = 0;
    result = Write(&msg);
    if (result != NO_ERROR) return result;

    return NO_ERROR;
}
#endif

/*
 *----------------------------------------------------------------------
 *
 * FindEscapeKey --
 *
 *	Search for a matching escape key sequence
 *
 * Results:
 *	The matching key if found, -1 if not found, -2 if a partial match
 *
 *----------------------------------------------------------------------
 */

int
ConsoleDebugger::FindEscapeKey(LPCSTR buf, SIZE_T buflen)
{
    DWORD len;
    int i;

    for (i = 0; VtFunctionKeys[i].sequence; i++) {
	len = strlen(VtFunctionKeys[i].sequence);
	if (len == buflen) {
	    if (strncmp(VtFunctionKeys[i].sequence, buf, buflen) == 0) {
		return VtFunctionKeys[i].keyval;
	    }
	} else {
	    if (strncmp(VtFunctionKeys[i].sequence, buf, buflen) == 0) {
		/* Partial match */
		return -2;
	    }
	}
    }
    return -1;
}

int
ConsoleDebugger::Write (LPCSTR buffer, SIZE_T length, LPDWORD err)
{
#ifndef IPC_MAXRECORDS
    SIZE_T i;
    DWORD errorCode;
    int key;

    *err = NO_ERROR;

    for (i = 0; i < length; i++) {
	char c;

	c = *(buffer+i);
	// scan looking for '\033', send to MapFKeyToIRs instead.
	if (0 && c == '\033') {    // XXX: this needs work!  Bypass branch
	    key = FindEscapeKey(buffer+i, length-i);
	    if (key >= 0) {
		if ((errorCode = MapFKeyToIRs(key)) != NO_ERROR) {
		    *err = errorCode;
		    return -1;
		}
	    }
	} else {
	    if ((errorCode = MapCharToIRs(*(buffer+i))) != NO_ERROR) {
		*err = errorCode;
		return -1;
	    }
	}
    }
#else
    SIZE_T i, j;
    CHAR c;
    UCHAR lc;
    DWORD result;
    IPCMsg msg;

    *err = NO_ERROR;

    msg.type = IRECORD;

    for (i = 0, j = 0; i < length; i++) {
	c = buffer[i];
	/* strip off the upper 127 */
	lc = (UCHAR) (c & 0x7f);

	/* keydown first. */
	msg.irecord[j].EventType = KEY_EVENT;
	msg.irecord[j].Event.KeyEvent.bKeyDown = TRUE;
	msg.irecord[j].Event.KeyEvent.wRepeatCount = 1;
	msg.irecord[j].Event.KeyEvent.wVirtualKeyCode =
	    AsciiToKeyArray[lc].wVirtualKeyCode;
	msg.irecord[j].Event.KeyEvent.wVirtualScanCode =
	    AsciiToKeyArray[lc].wVirtualScanCode;
	msg.irecord[j].Event.KeyEvent.dwControlKeyState =
	    AsciiToKeyArray[lc].dwControlKeyState;
	msg.irecord[j].Event.KeyEvent.uChar.AsciiChar = c;
	j++;

	/* now keyup. */
	msg.irecord[j].EventType = KEY_EVENT;
	msg.irecord[j].Event.KeyEvent.bKeyDown = FALSE;
	msg.irecord[j].Event.KeyEvent.wRepeatCount = 1;
	msg.irecord[j].Event.KeyEvent.wVirtualKeyCode =
	    AsciiToKeyArray[lc].wVirtualKeyCode;
	msg.irecord[j].Event.KeyEvent.wVirtualScanCode =
	    AsciiToKeyArray[lc].wVirtualScanCode;
	msg.irecord[j].Event.KeyEvent.dwControlKeyState =
	    AsciiToKeyArray[lc].dwControlKeyState;
	msg.irecord[j].Event.KeyEvent.uChar.AsciiChar = c;
	j++;

	if ((j >= IPC_MAXRECORDS-1) || (i == length-1)) {
	    /*
	     * Write out the input records to the console.
	     */
	    msg.event = j;
	    result = Write(&msg);
	    if (result != NO_ERROR) {
		*err = result;
		return -1;
	    }
	    j = 0;
	}
    }
#endif

    return length;   // we always can write the whole thing.
}

