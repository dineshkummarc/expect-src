/* ----------------------------------------------------------------------------
 * expWinConsoleDebugger.hpp --
 *
 *	Console debugger class declared here.
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
 * RCS: @(#) $Id: expWinConsoleDebugger.hpp,v 1.1.2.31 2003/08/26 20:46:52 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#ifndef INC_expWinConsoleDebugger_hpp__
#define INC_expWinConsoleDebugger_hpp__

#include "CMcl.h"
#include "expWinMessage.hpp"
#include "expWinInjectorIPC.hpp"

// for the hash table template (argh! I really don't want Tcl deps in this code).
#include "TclHash.hpp"
#include <imagehlp.h>


// callback type.
class ConsoleDebuggerCallback
{
    friend class ConsoleDebugger;
    virtual void AlertReadable (void) = 0;
};



//  This is our debugger.  We run it in a thread. 
//
class ConsoleDebugger : public CMclThreadHandler
{
public:

    // NOTE: TCHAR is a loose definition and *NOT* the normal
    // usage as found in the Win32 API reference.  TCHAR is
    // runtime selected to be WCHAR (unicode) on NT/2K/XP
    // or CHAR (local specific) on everything else.  See
    // Tcl_WinUtfToTChar in the Tcl docs for details and
    // the WinProcs struct in expWinPort.h
    //
    ConsoleDebugger(
	TCHAR *_cmdline,		// commandline string (in system encoding)
	TCHAR *_env,			// environment block (in system encoding)
	TCHAR *_dir,			// startup directory (in system encoding)
					// These 3 maintain a reference until
					//  _readyUp is signaled.
	int show,			// $exp::nt_debug, shows spawned children.
	const char *_injPath,		// location -=[ ON THE FILE SYSTEM!!! ]=-
	CMclLinkedList<Message *> &_mQ,	// parent owned linkedlist for returning data stream.
	CMclLinkedList<Message *> &_eQ,	// parent owned linkedlist for returning error stream.
	CMclEvent &_readyUp,		// set when child ready (or failed).
	ConsoleDebuggerCallback &_callback
	);

    // copying and passing by copy are not allowed...
    // this prevents confusion of internal object ownership...
    ConsoleDebugger(ConsoleDebugger & rhs);

    // assigning one ConsoleDebugger to another is not allowed,
    // this prevents confusion of internal object ownership...
    ConsoleDebugger & operator= (ConsoleDebugger & rhs);

    ~ConsoleDebugger();

    inline DWORD Status (void) { return status; }	    // retrieves error codes.
    inline DWORD Pid (void) { return pid; }
    inline HANDLE Handle (void) { return hRootProcess; }
    inline HANDLE Console (void) { return hMasterConsole; }
    inline HANDLE ConsoleScreenBuffer (void) { return hCopyScreenBuffer; }
    int Write (LPCSTR buffer, SIZE_T length, LPDWORD err); // send vt100 to the slave console.
    DWORD Write (IPCMsg *msg);			    // send an INPUT_RECORD or ctrlEvent instead.

    void EnterInteract (HANDLE OutConsole);	    // enters interact mode.
    void ExitInteract (void);			    // exits interact mode.

    DWORD fatalException;

private:
    virtual unsigned ThreadHandlerProc(void);

    // forward reference.
    class Process;
    class Breakpoint;

    class ThreadInfo {
	friend class ConsoleDebugger;
	HANDLE	    hThread;
	DWORD	    dwThreadId;
	DWORD	    nargs;
	DWORD	    args[16];	// Space for saving 16 args.  We need this
				// space while we are waiting for the return
				// value for the function.
	LPCONTEXT   context;	// Current context.
	Breakpoint  *lastBrkpt;	// Last breakpoint hit.
	ThreadInfo  *nextPtr;	// Linked list.
    };

    class BreakInfo {
	friend class ConsoleDebugger;
	const char  *funcName;	// Name of function to intercept.
	DWORD	    nargs;	// Number of arguments.
	void (ConsoleDebugger::*breakProc)(Process *, ThreadInfo *,
		Breakpoint *, PDWORD, DWORD);
				// Function to call when the breakpoint is hit.
#	define BREAK_IN  1	// Call handler on the way in.
#	define BREAK_OUT 2	// Call handler on the way out.
	DWORD   dwFlags;	// Bits for direction to call handler in.
    };

    class DllBreakpoints {
	friend class ConsoleDebugger;
	const char  *dllName;
	BreakInfo   *breakInfo;
    };

    class Breakpoint {
	friend class ConsoleDebugger;
	Breakpoint() : returning(FALSE), origRetAddr(0), threadInfo(0L) {}
	BOOL	    returning;	    // Is this a returning breakpoint?
	BYTE	    code;	    // Original code.
	PVOID	    codePtr;	    // Address of original code.
	PVOID	    codeReturnPtr;  // Address of return breakpoint.
	DWORD	    origRetAddr;    // Original return address.
	BreakInfo   *breakInfo;	    // Information about the breakpoint.
	ThreadInfo  *threadInfo;    // If this breakpoint is for a specific
				    //  thread.
	Breakpoint  *nextPtr;	    // Linked list.
    };

    class Module {
	friend class ConsoleDebugger;
	BOOL	    loaded;
	HANDLE	    hFile;
	LPVOID	    baseAddr;
	PCHAR	    modName;
	PIMAGE_DEBUG_INFORMATION dbgInfo;
    };

    typedef Tcl::Hash<PVOID, TCL_STRING_KEYS> STRING2PTR;
    typedef Tcl::Hash<Module*, TCL_ONE_WORD_KEYS> PTR2MODULE;

    //  There is one of these instances for each process that we are
    //  controlling.
    //
    class Process {
	friend class ConsoleDebugger;
	Process() : threadList(0L), threadCount(0), brkptList(0L),
	    offset(0), nBreakCount(0), consoleHandlesMax(0),
	    hProcess(0L), pSubprocessMemory(0), exeModule(0L) {}
	ThreadInfo  *threadList;	// List of threads in the subprocess.
	Breakpoint  *brkptList;		// List of breakpoints in the subprocess.
	DWORD	    offset;		// Breakpoint offset in allocated mem.
	DWORD	    nBreakCount;	// Number of breakpoints hit.
	DWORD	    consoleHandles[100];// A list of input console handles.
	DWORD	    consoleHandlesMax;
	HANDLE	    hProcess;		// Handle of process.
	DWORD	    pid;		// Global process id.
	DWORD	    threadCount;	// Number of threads in process.
	PVOID	    pSubprocessMemory;	// Pointer to allocated memory in subprocess.
	STRING2PTR  funcTable;		// Function table name to address mapping.
	PTR2MODULE  moduleTable;	// Win32 modules that have been loaded.
	Module	    *exeModule;		// Executable module info.
	Process	    *nextPtr;		// Linked list.
    };

#   include <pshpack1.h>
#   ifdef _M_IX86
    struct LOADLIBRARY_STUB
    {
        BYTE    instr_PUSH;
        DWORD   operand_PUSH_value;
        BYTE    instr_MOV_EAX;
        DWORD   operand_MOV_EAX;
        WORD    instr_CALL_EAX;
        BYTE    instr_INT_3;
        char    data_DllName[MAX_PATH];

        LOADLIBRARY_STUB (const char *path) :
            instr_PUSH(0x68), instr_MOV_EAX(0xB8),
            instr_CALL_EAX(0xD0FF), instr_INT_3(0xCC)
	{
	    strcpy(data_DllName, path);
	    strcat(data_DllName, "\\injector.dll");
	}
    };
#   else
#	error "need correct stub loader opcodes for this hardware."
#   endif
#   include <poppack.h>

    //  Direct debug event handlers.
    //
    BOOL OnXFirstBreakpoint	(Process *, LPDEBUG_EVENT);
    BOOL OnXSecondBreakpoint	(Process *, LPDEBUG_EVENT);
    BOOL OnXThirdBreakpoint	(Process *, LPDEBUG_EVENT);
    BOOL OnXBreakpoint		(Process *, LPDEBUG_EVENT);
    void OnXCreateProcess	(Process *, LPDEBUG_EVENT);
    void OnXCreateThread	(Process *, LPDEBUG_EVENT);
    void OnXDeleteThread	(Process *, LPDEBUG_EVENT);
    void OnXLoadDll		(Process *, LPDEBUG_EVENT);
    void OnXUnloadDll		(Process *, LPDEBUG_EVENT);
    void OnXDebugString		(Process *, LPDEBUG_EVENT);
    void OnXRip			(Process *, LPDEBUG_EVENT);
    void OnXSecondChanceException (Process *, LPDEBUG_EVENT);
    void OnXSingleStep		(Process *, LPDEBUG_EVENT);

    //  Our breakpoint handlers (indirect).  Called from OnXBreakpoint().
    //
    void OnAllocConsole		(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnBeep			(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnCreateConsoleScreenBuffer (Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnFillConsoleOutputAttribute (Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnFillConsoleOutputCharacterA (Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnFillConsoleOutputCharacterW (Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnFreeConsole		(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnGetStdHandle		(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnIsWindowVisible	(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnOpenConsoleW		(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnSetConsoleActiveScreenBuffer	(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnSetConsoleCP		(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnSetConsoleCursorInfo (Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnSetConsoleCursorPosition (Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnSetConsoleMode	(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnSetConsoleOutputCP	(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnSetConsoleTextAttribute (Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnSetConsoleWindowInfo	(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnScrollConsoleScreenBuffer (Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnWriteConsoleA	(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnWriteConsoleW	(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnWriteConsoleOutputA	(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnWriteConsoleOutputW	(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnWriteConsoleOutputCharacterA	(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnWriteConsoleOutputCharacterW	(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnWriteFile		(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);

    // Internal utilities
    //
    Process *ProcessNew		();
    void ProcessFree		(Process *);
    DWORD CommonDebugger	();
    BOOL SetBreakpoint		(Process *, BreakInfo *);
    BOOL SetBreakpointAtAddr	(Process *, BreakInfo *, PVOID);
    int LoadedModule		(Process *, HANDLE, LPVOID, int, LPVOID, DWORD);
    BOOL ReadSubprocessMemory	(Process *, LPVOID, LPVOID, DWORD);
    BOOL WriteSubprocessMemory	(Process *, LPVOID, LPVOID, DWORD);
    BOOL MakeSubprocessMemory   (Process *, SIZE_T, LPVOID *, DWORD);
    BOOL RemoveSubprocessMemory (Process *, LPVOID);
    int ReadSubprocessStringA	(Process *, PVOID, PCHAR, int);
    int ReadSubprocessStringW	(Process *, PVOID, PWCHAR, int);
    void CreateVtSequence	(Process *, COORD, DWORD);
    void OnFillCOC_Finish	(CHAR, DWORD, COORD);
    DWORD lastBeepDuration;
    PCSTR GetExceptionCodeString (DWORD);

//    void RefreshScreen		(void);

    // send info back to the parent
    void WriteMaster		(LPCSTR, SIZE_T);
    void WriteMasterWarning	(LPCSTR, SIZE_T);
    void WriteMasterError	(LPCSTR, SIZE_T, DWORD);


    // announce we are done.
    void NotifyDone		();

    // The arrays of functions where we set breakpoints
    //
    BreakInfo	BreakArrayKernel32[27];
    BreakInfo	BreakArrayUser32[2];
    DllBreakpoints BreakPoints[3];

    // private vars
    //
    Process	*ProcessList;   // Top of linked list of Process instances.
    HANDLE	hMasterConsole;	// Master console handle (us).
    HANDLE	hCopyScreenBuffer; // console buffer we write a complete copy to.
    DWORD	dwPlatformId;	// what OS are we under?
    DWORD	ConsoleInputMode;// Current flags for the master console.
    COORD	ConsoleSize;    // Size of the console in the slave.
    COORD	CursorPosition; // Coordinates of the cursor in the slave.
    char	*SymbolPath;
    UINT	ConsoleCP;	// console input code page of the slave.
    UINT	ConsoleOutputCP;// console output code page of the slave.
    BOOL	CursorKnown;    // Do we know where the slave's cursor is?
    CONSOLE_CURSOR_INFO CursorInfo;// Cursor info structure that is a copy of
				   //  the slave's.

    TCHAR	*cmdline;	// commandline string (in system encoding)
    TCHAR	*env;		// environment block (in system encoding)
    TCHAR	*dir;		// startup directory (in system encoding)
    CMclEvent	&readyUp;	// indicate to the parent when to unblock and check creation status.
    DWORD	pid;
    DWORD       pidKilled;
    HANDLE	hRootProcess;
    int show;			// show the child?

    // Thread-safe message queues used for communication back to the parent.
    //
    CMclLinkedList<Message *> &mQ;
    CMclLinkedList<Message *> &eQ;

    ConsoleDebuggerCallback &callback;

    // This critical section is set when breakpoints are running.
    //
    CMclCritSec bpCritSec;

    LPVOID	pStartAddress;	// Start address of the top process.
    BYTE	originalExeEntryPointOpcode;

    LOADLIBRARY_STUB injectorStub;// opcodes we use to force load our injector
				//  dll.
    PVOID	pInjectorStub;	// Pointer to memory in sub process used
				//  for the injector's loader.
    CONTEXT	preStubContext; // Thread context info before switching to run
				//  the stub.

    typedef Tcl::Hash<HANDLE, TCL_ONE_WORD_KEYS> PTR2HANDLE;
    PTR2HANDLE	spMemMapping;	// Used on Win9x to associate the file mapping
				//  handle to the memory address it provides.

    // A couple NT routines we might need when running on NT/2K/XP
    typedef LPVOID (__stdcall *PFNVIRTUALALLOCEX)(HANDLE,LPVOID,SIZE_T,DWORD,DWORD);
    typedef BOOL (__stdcall *PFNVIRTUALFREEEX)(HANDLE,LPVOID,SIZE_T,DWORD);

    PFNVIRTUALALLOCEX pfnVirtualAllocEx;
    PFNVIRTUALFREEEX pfnVirtualFreeEx;

    CMclMailbox *injectorIPC;	// IPC transfer mechanism to the injector dll.

    bool interacting;

    DWORD status;

    const static DWORD KEY_UP;
    const static DWORD KEY_DOWN;
    const static DWORD KEY_RIGHT;
    const static DWORD KEY_LEFT;
    const static DWORD KEY_END;
    const static DWORD KEY_HOME;
    const static DWORD KEY_PAGEUP;
    const static DWORD KEY_PAGEDOWN;
    const static DWORD KEY_INSERT;
    const static DWORD KEY_DELETE;
    const static DWORD KEY_SELECT;
    const static DWORD KEY_F1;
    const static DWORD KEY_F2;
    const static DWORD KEY_F3;
    const static DWORD KEY_F4;
    const static DWORD KEY_F5;
    const static DWORD KEY_F6;
    const static DWORD KEY_F7;
    const static DWORD KEY_F8;
    const static DWORD KEY_F9;
    const static DWORD KEY_F10;
    const static DWORD KEY_F11;
    const static DWORD KEY_F12;
    const static DWORD KEY_F13;
    const static DWORD KEY_F14;
    const static DWORD KEY_F15;
    const static DWORD KEY_F16;
    const static DWORD KEY_F17;
    const static DWORD KEY_F18;
    const static DWORD KEY_F19;
    const static DWORD KEY_F20;
    const static DWORD WIN_RESIZE;
    const static DWORD KEY_CONTROL;
    const static DWORD KEY_SHIFT;
    const static DWORD KEY_LSHIFT;
    const static DWORD KEY_RSHIFT;
    const static DWORD KEY_ALT;
    
    struct KEY_MATRIX {
	WORD wVirtualKeyCode;
	WORD wVirtualScanCode;
	DWORD dwControlKeyState;
    };
    
    struct FUNCTION_KEY {
	const char *sequence;
	DWORD keyval;
    };

    const static KEY_MATRIX ModifierKeyArray[];
    const static KEY_MATRIX AsciiToKeyArray[];
    const static KEY_MATRIX FunctionToKeyArray[];
    const static FUNCTION_KEY VtFunctionKeys[];

    DWORD MapCharToIRs (CHAR c);
    DWORD MapFKeyToIRs (DWORD fk);
    int FindEscapeKey (LPCSTR buf, SIZE_T buflen);
};

#endif // INC_expWinConsoleDebugger_hpp__
