/* ----------------------------------------------------------------------------
 * expWinConsoleDebuggerBreakPoints.cpp --
 *
 *	Breakpoints for the ConsoleDebugger class are in here.  These define
 *	the behavior of what to do when a breakpoint happens in the slave
 *	we are intercepting.  From here, we transfer the stream .... (TBF)
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
 * RCS: @(#) $Id: expWinConsoleDebuggerBreakPoints.cpp,v 1.1.2.17 2003/08/26 20:46:52 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinConsoleDebugger.hpp"
#include "expWinPort.h"

/////////////////////////////////////////////////////////////
// NOTE:  black magic abounds...  be warry young padwon... //
/////////////////////////////////////////////////////////////

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::CreateVtSequence --
 *
 *	When moving the cursor to a new location, this will create
 *	the appropriate VT100 type sequence to get the cursor there.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Characters are written to the pipe going to Expect
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::CreateVtSequence(Process *proc, COORD newPos, DWORD n)
{
    COORD oldPos;
    CHAR buf[128];
    DWORD count;

#ifdef EXP_DEBUG
    char b[1024];
    sprintf(b, "CVTS(%d): CursorKnown: %d @ %dx%d -> %dx%d (%dx%d ?)",
	    n, CursorKnown, CursorPosition.Y+1, CursorPosition.X+1,
	    newPos.Y+1, newPos.X+1,
	    newPos.Y+1+ (SHORT) (n / ConsoleSize.X),
	    newPos.X+1+ (SHORT) (n % ConsoleSize.X));
    OutputDebugString(b);
#endif

    if ((n == 0) || ExpWinNTStripVT100()) {
	return;
    }

    oldPos = CursorPosition;

    if (CursorKnown && (newPos.Y > oldPos.Y) && (newPos.X == 0)) {
	buf[0] = '\r';
	memset(&buf[1], '\n', newPos.Y - oldPos.Y);
	count = 1 + newPos.Y - oldPos.Y;
    } else {
	// VT100 sequence
	count = wsprintf(buf, "\033[%d;%dH", newPos.Y+1, newPos.X+1);
    }
    newPos.X += (SHORT) (n % ConsoleSize.X);
    newPos.Y += (SHORT) (n / ConsoleSize.X);
    CursorPosition = newPos;

    WriteMaster(buf, count);
}

/*
 *-----------------------------------------------------------------------------
 *
 * ConsoleDebugger::OnAllocConsole --
 *
 * Results:
 *	None
 *
 * Notes:
 *
 *-----------------------------------------------------------------------------
 */

void
ConsoleDebugger::OnAllocConsole(Process *proc, ThreadInfo *threadInfo,
    Breakpoint *brkpt, PDWORD returnValue, DWORD direction)
{
    if (*returnValue == FALSE) {
	return;
    }
    __asm nop;
}

/*
 *-----------------------------------------------------------------------------
 *
 * ConsoleDebugger::OnBeep --
 *
 *	This routine gets called when Beep is called.  At least in sshd,
 *	we don't want a beep to show up on the local console.  Instead,
 *	direct it back to the master with a ASCII 7.
 *
 * Results:
 *	None
 *
 * Notes:
 *	XXX: Setting the duration to 0 doesn't seem to make the local
 *	beep go away.  It seems we need to stop the call at this point
 *	(or point it to some other call with the same number of arguments)
 *
 *-----------------------------------------------------------------------------
 */

void
ConsoleDebugger::OnBeep(Process *proc, ThreadInfo *threadInfo,
    Breakpoint *brkpt, PDWORD returnValue, DWORD direction)
{
    CHAR buf[2] = {0,0};

    if (direction == BREAK_IN) {
	lastBeepDuration = threadInfo->args[1];
	// Modify the arguments so a beep doesn't sound in the slave.
	threadInfo->args[1] = 0;
    } else if (direction == BREAK_OUT) {
	if (*returnValue == 0) {
	    buf[0] = 7; // ASCII beep
	    WriteMaster(buf, 1);
	    if (interacting) {
		Beep(threadInfo->args[0], lastBeepDuration);
	    }
	}
    }
}

/*
 *-----------------------------------------------------------------------------
 *
 * ConsoleDebugger::OnCreateConsoleScreenBuffer --
 *
 *	This function gets called when a CreateConsoleScreenBuffer
 *	breakpoint is hit.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	unknown.
 *
 *-----------------------------------------------------------------------------
 */

void
ConsoleDebugger::OnCreateConsoleScreenBuffer(Process *proc,
    ThreadInfo *threadInfo, Breakpoint *brkpt, PDWORD returnValue,
    DWORD direction)
{
    if (*returnValue == FALSE) {
	return;
    }
    // TODO: what to do here?
    __asm nop;
}

/*
 *-----------------------------------------------------------------------------
 *
 * ConsoleDebugger::OnFillConsoleOutputAttribute --
 *
 * Results:
 *	None
 *
 * Notes:
 *
 *-----------------------------------------------------------------------------
 */

void
ConsoleDebugger::OnFillConsoleOutputAttribute(Process *proc, ThreadInfo *threadInfo,
    Breakpoint *brkpt, PDWORD returnValue, DWORD direction)
{
    if (*returnValue == FALSE) {
	return;
    }
    __asm nop;
}

/*
 *-----------------------------------------------------------------------------
 *
 * ConsoleDebugger::OnFillConsoleOutputCharacterA --
 *
 *	This function gets called when an FillConsoleOutputCharacterA
 *	breakpoint is hit.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Prints some output.
 *
 *-----------------------------------------------------------------------------
 */

void
ConsoleDebugger::OnFillConsoleOutputCharacterA(Process *proc,
    ThreadInfo *threadInfo, Breakpoint *brkpt, PDWORD returnValue,
    DWORD direction)
{
    CHAR cCharacter;
    DWORD nLength;
    COORD dwWriteCoord;
//    PVOID ptr;

    if (*returnValue == 0) {
	return;
    }

    cCharacter = (CHAR) threadInfo->args[1];
    nLength = threadInfo->args[2];
    dwWriteCoord = *((PCOORD) &(threadInfo->args[3]));
//    ptr = (PVOID) threadInfo->args[4];
//    if (ptr) {
//	ReadSubprocessMemory(proc, ptr, &nLength, sizeof(DWORD));
//    }


    OnFillCOC_Finish(cCharacter, nLength, dwWriteCoord);
}

/*
 *-----------------------------------------------------------------------------
 *
 * ConsoleDebugger::OnFillConsoleOutputCharacterW --
 *
 *	This function gets called when an FillConsoleOutputCharacterW
 *	breakpoint is hit.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Prints some output.
 *
 *-----------------------------------------------------------------------------
 */

void
ConsoleDebugger::OnFillConsoleOutputCharacterW(Process *proc,
    ThreadInfo *threadInfo, Breakpoint *brkpt, PDWORD returnValue,
    DWORD direction)
{
    CHAR cCharacter;
    WCHAR cWCharacter;
    DWORD nLength;
    COORD dwWriteCoord;
//    PVOID ptr;

    if (*returnValue == 0) {
	return;
    }

    cWCharacter = (WCHAR) threadInfo->args[1];
    nLength = threadInfo->args[2];
    dwWriteCoord = *((PCOORD) &(threadInfo->args[3]));
//    ptr = (PVOID) threadInfo->args[4];
//    if (ptr) {
//	ReadSubprocessMemory(proc, ptr, &nLength, sizeof(DWORD));
//    }

    // For now, just truncated it.
    // TODO: fix me!
    //
    cCharacter = (cWCharacter & 0xff);

    OnFillCOC_Finish(cCharacter, nLength, dwWriteCoord);
}

void
ConsoleDebugger::OnFillCOC_Finish(CHAR cCharacter, DWORD nLength, COORD dwWriteCoord)
{
    DWORD dwWritten;
    FillConsoleOutputCharacter(hCopyScreenBuffer, cCharacter,
			       nLength, dwWriteCoord, &dwWritten);

    if (ExpWinNTStripVT100()) {
	char *buf = new char [nLength];
	FillMemory(buf, nLength, cCharacter);
	WriteMaster(buf, nLength);
	delete [] buf;
    } else {
	CHAR buf[4096];
	int bufpos;
	DWORD i;
	DWORD lines, preCols, postCols;
	BOOL eol, bol;		// Needs clearing to end, beginning of line
	CONSOLE_SCREEN_BUFFER_INFO info;

	preCols = 0;
	bufpos = 0;
	eol = bol = FALSE;
	if (dwWriteCoord.X) {
	    preCols = ConsoleSize.X - dwWriteCoord.X;
	    if (nLength <= preCols) {
		preCols = nLength;
		nLength = 0;
		if (nLength == preCols) {
		    eol = TRUE;
		}
	    } else {
		eol = TRUE;
		nLength -= preCols;
	    }
	} else if (nLength < (DWORD) ConsoleSize.X) {
	    bol = TRUE;
	    preCols = nLength;
	    nLength = 0;
	}

	lines = nLength / ConsoleSize.X;
	postCols = nLength % ConsoleSize.X;

	if (preCols) {
	    if (bol) {
		// Beginning of line to before end of line
		if (cCharacter == ' ') {
		    bufpos += wsprintf(&buf[bufpos], "\033[%d;%dH\033[1K",
				       dwWriteCoord.Y+1,
				       preCols+dwWriteCoord.X);
		} else {
		    bufpos += wsprintf(&buf[bufpos], "\033[%d;%dH",
				       dwWriteCoord.Y+1, dwWriteCoord.X+1);
		    memset(&buf[bufpos], cCharacter, preCols);
		    bufpos += preCols;
		}
	    } else {
		// After beginning of line to end of line
		bufpos += wsprintf(&buf[bufpos], "\033[%d;%dH",
				   dwWriteCoord.Y+1, dwWriteCoord.X+1);
		if (eol && cCharacter == ' ') {
		    bufpos += wsprintf(&buf[bufpos], "\033[K");
		} else {
		    memset(&buf[bufpos], cCharacter, preCols);
		    bufpos += preCols;
		}
	    }
	    dwWriteCoord.X = 0;
	    dwWriteCoord.Y++;
	}
	if (lines) {
	    if ((cCharacter == ' ')
		&& ((lines + dwWriteCoord.Y) >= (DWORD) ConsoleSize.Y)) {
		// Clear to end of screen
		bufpos += wsprintf(&buf[bufpos], "\033[%d;%dH\033[J",
				   dwWriteCoord.Y+1, dwWriteCoord.X+1);
	    } else if ((cCharacter == ' ') && (dwWriteCoord.Y == 0)
		       && (lines > 0)) {
		// Clear to top of screen
		bufpos += wsprintf(&buf[bufpos], "\033[%d;%dH\033[1J",
				   lines, 1);
	    } else {
		for (i = 0; i < lines; i++) {
		    bufpos += wsprintf(&buf[bufpos], "\033[%d;%dH",
				       dwWriteCoord.Y+i+1, dwWriteCoord.X+1);
		    if (cCharacter == ' ') {
			bufpos += wsprintf(&buf[bufpos], "\033[2K");
		    } else {
			memset(&buf[bufpos], cCharacter, ConsoleSize.X);
			bufpos += ConsoleSize.X;
		    }
		}
	    }
	    dwWriteCoord.Y += (SHORT) lines;
	}

	if (postCols) {
	    if (cCharacter == ' ') {
		// Clear to beginning of line
		bufpos += wsprintf(&buf[bufpos], "\033[%d;%dH\033[1K",
				   dwWriteCoord.Y+1, postCols+dwWriteCoord.X);
	    } else {
		bufpos += wsprintf(&buf[bufpos], "\033[%d;%dH",
				   dwWriteCoord.X+1, dwWriteCoord.Y+1);
		memset(&buf[bufpos], cCharacter, postCols);
		bufpos += postCols;
	    }
	}
	if (GetConsoleScreenBufferInfo(hMasterConsole, &info) == FALSE) {
	    //	char errbuf[200];
	    //	wsprintf(errbuf, "handle=0x%08x", hMasterConsole);
	    //	EXP_LOG2(MSG_DT_SCREENBUF, errbuf, GetSysMsg(GetLastError()));
	} else {
	    CursorPosition = info.dwCursorPosition;
	    bufpos += wsprintf(&buf[bufpos], "\033[%d;%dH",
			       CursorPosition.Y+1, CursorPosition.X+1);
	    CursorKnown = TRUE;
	}
	WriteMaster(buf, bufpos);
    }
}

/*
 *-----------------------------------------------------------------------------
 *
 * ConsoleDebugger::OnFreeConsole --
 *
 * Results:
 *	None
 *
 * Notes:
 *
 *-----------------------------------------------------------------------------
 */

void
ConsoleDebugger::OnFreeConsole(Process *proc, ThreadInfo *threadInfo,
    Breakpoint *brkpt, PDWORD returnValue, DWORD direction)
{
    if (*returnValue == FALSE) {
	return;
    }
    __asm nop;
}

/*
 *-----------------------------------------------------------------------------
 *
 * ConsoleDebugger::OnGetStdHandle --
 *
 *	This function gets called when a GetStdHandle breakpoint
 *	is hit.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Sets some flags that are used in determining echoing
 *	characteristics of the slave driver.
 *
 *-----------------------------------------------------------------------------
 */

void
ConsoleDebugger::OnGetStdHandle(Process *proc, ThreadInfo *threadInfo,
    Breakpoint *brkpt, PDWORD returnValue, DWORD direction)
{
    DWORD i;
    BOOL found;

    if (*returnValue == (DWORD) INVALID_HANDLE_VALUE) {
	return;
    }
    if (threadInfo->args[0] != STD_INPUT_HANDLE) {
	return;
    }
    for (found = FALSE, i = 0; i < proc->consoleHandlesMax; i++) {
	if (proc->consoleHandles[i] == *returnValue) {
	    found = TRUE;
	    break;
	}
    }
    if (! found) {
	if (proc->consoleHandlesMax > 100) {
	    proc->consoleHandlesMax = 100;
	}
	proc->consoleHandles[proc->consoleHandlesMax++] = *returnValue;
    }
    return;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnOpenConsoleW --
 *
 *	This function gets called when an OpenConsoleW breakpoint
 *	is hit.  There is one big problem with this function--it
 *	isn't documented.  However, we only really care about the
 *	return value which is a console handle.  I think this is
 *	what this function declaration should be:
 *
 *	HANDLE OpenConsoleW(LPWSTR lpFileName,
 *			    DWORD dwDesiredAccess,
 *			    DWORD dwShareMode,
 *			    LPSECURITY_ATTRIBUTES lpSecurityAttributes);
 *
 *	So why do we intercept an undocumented function while we
 *	could just intercept CreateFileW and CreateFileA?  Well,
 *	those functions are going to get called alot more than this
 *	one, so limiting the number of intercepted functions
 *	improves performance since fewer breakpoints will be hit.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Save the return value in an array of known console handles
 *	with their statuses.
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnOpenConsoleW(Process *proc, ThreadInfo *threadInfo,
    Breakpoint *brkpt, PDWORD returnValue, DWORD direction)
{
    WCHAR name[256];
    PVOID ptr;

    if (*returnValue == (DWORD) INVALID_HANDLE_VALUE) {
	return;
    }

    // Save any console input handle.  No SetConsoleMode() calls will
    // succeed unless they are really attached to a console input buffer.
    //
    ptr = (PVOID) threadInfo->args[0];
    ReadSubprocessStringW(proc, ptr, name, 256);

    if (wcsicmp(name, L"CONIN$") == 0) {
	if (proc->consoleHandlesMax > 100) {
	    proc->consoleHandlesMax = 100;
	}
	proc->consoleHandles[proc->consoleHandlesMax++] = *returnValue;
    }
    return;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnScrollConsoleScreenBuffer --
 *
 *	This funtions gets called when a ScrollConsoleScreenBuffer
 *	breakpoint is hit.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Generate some VT100 sequences to insert lines
 *
 * Notes:
 *	XXX: Ideally, we should check if the screen buffer is the one that
 *	is currently being displayed.  However, that means we have to
 *	track CONOUT$ handles, so we don't do it for now.
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnScrollConsoleScreenBuffer(Process *proc,
    ThreadInfo *threadInfo, Breakpoint *brkpt, PDWORD returnValue,
    DWORD direction)
{
    DWORD count = 0;
    SMALL_RECT scroll, clip, *pClip;
    COORD dest;
    CHAR_INFO fill;
    CHAR c;
    PVOID ptr;

    if (*returnValue == FALSE) {
	return;
    }
    ptr = (PVOID) threadInfo->args[1];
    ReadSubprocessMemory(proc, ptr, &scroll, sizeof(SMALL_RECT));
    ptr = (PVOID) threadInfo->args[2];
    pClip = 0L;
    if (ptr) {
	pClip = &clip;
	ReadSubprocessMemory(proc, ptr, &clip, sizeof(SMALL_RECT));
    }
    dest = *((PCOORD) &threadInfo->args[3]);
    ptr = (PVOID) threadInfo->args[4];
    ReadSubprocessMemory(proc, ptr, &fill, sizeof(CHAR_INFO));
    c = fill.Char.AsciiChar;

    // Check for a full line scroll
    if (c == ' ' && scroll.Left == dest.X &&
	    scroll.Left == 0 && scroll.Right >= ConsoleSize.X-1) {
	if (ExpWinNTStripVT100()) {
	    WriteMaster("\n", 1);
	} else {
	    CHAR buf[100];
	    if (dest.Y < scroll.Top) {
		count = wsprintf(&buf[count], "\033[%d;%dr\033[%d;%dH\033[%dM",
				 dest.Y+1,scroll.Bottom+1,dest.Y+1,1,
				 scroll.Top - dest.Y);
	    } else {
		count = wsprintf(&buf[count], "\033[%d;%dr\033[%d;%dH\033[%dL",
				 scroll.Top+1,
				 dest.Y+1+(scroll.Bottom - scroll.Top),
				 scroll.Top+1,1,
				 dest.Y - scroll.Top);
	    }
	    count += wsprintf(&buf[count], "\033[%d;%dr", 1, ConsoleSize.Y);
	    WriteMaster(buf, count);
	}
    } else {
//	RefreshScreen(&proc->overlapped);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnSetConsoleActiveScreenBuffer --
 *
 *	This function gets called when a SetConsoleActiveScreenBuffer
 *	breakpoint is hit.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	We reread the entire console and send it to the master.
 *	Updates the current console cursor position
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnSetConsoleActiveScreenBuffer(Process *proc,
    ThreadInfo *threadInfo, Breakpoint *brkpt, PDWORD returnValue,
    DWORD direction)
{
    if (*returnValue == FALSE) {
	return;
    }

    __asm nop;
//    RefreshScreen(&proc->overlapped);
}

/*
 *-----------------------------------------------------------------------------
 *
 * ConsoleDebugger::OnSetConsoleCP --
 *
 * Results:
 *	None
 *
 * Notes:
 *
 *-----------------------------------------------------------------------------
 */

void
ConsoleDebugger::OnSetConsoleCP(Process *proc, ThreadInfo *threadInfo,
    Breakpoint *brkpt, PDWORD returnValue, DWORD direction)
{
    if (*returnValue == FALSE) {
	return;
    }
    ConsoleCP = (UINT) threadInfo->args[0];

#if 0  // probably not a good idea
	SetConsoleCP(ConsoleCP);
#endif
}

/*
 *-----------------------------------------------------------------------------
 *
 * ConsoleDebugger::OnSetConsoleCursorInfo --
 *
 * Results:
 *	None
 *
 * Notes:
 *	Updates the current console's cursor info.
 *
 *-----------------------------------------------------------------------------
 */

void
ConsoleDebugger::OnSetConsoleCursorInfo(Process *proc, ThreadInfo *threadInfo,
    Breakpoint *brkpt, PDWORD returnValue, DWORD direction)
{
    PVOID ptr;

    if (*returnValue == FALSE) {
	return;
    }
    ptr = (PVOID) threadInfo->args[1];
    ReadSubprocessMemory(proc, ptr, &CursorInfo, sizeof(CONSOLE_CURSOR_INFO));

    SetConsoleCursorInfo(hCopyScreenBuffer, &CursorInfo);
}

/*
 *-----------------------------------------------------------------------------
 *
 * ConsoleDebugger::OnSetConsoleCursorPosition --
 *
 *	This function gets called when a SetConsoleCursorPosition breakpoint
 *	is hit.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Updates the current console's cursor position
 *
 *-----------------------------------------------------------------------------
 */

void
ConsoleDebugger::OnSetConsoleCursorPosition(Process *proc,
    ThreadInfo *threadInfo, Breakpoint *brkpt, PDWORD returnValue,
    DWORD direction)
{
    COORD newPosition;

    if (*returnValue == FALSE) {
	return;
    }
    newPosition = *((PCOORD) &threadInfo->args[1]);

#ifdef EXP_DEBUG
    char b[1024];
    sprintf(b, "SCCP: %dx%d -> %dx%d",
	    CursorPosition.Y+1, CursorPosition.X+1, newPosition.Y+1, newPosition.X+1);
    OutputDebugString(b);
#endif
    if (ExpWinNTStripVT100()) {
	if ((CursorPosition.Y < newPosition.Y) && (newPosition.X == 0)) {
	    /*
	     * Convert vt100 move line down to correct number of \n's
	     */
	    int start, end;
	    for (start = CursorPosition.Y, end = newPosition.Y;
		 start < end; start++) {
#ifdef EXP_DEBUG
		OutputDebugString("OSCCP: Writing Newline");
#endif
		WriteMaster("\n", 1);
	    }
	}
    } else {
	CHAR buf[64];
	int count;

	/* convert to VT100 */
	count = wsprintf(buf, "\033[%d;%dH", newPosition.Y+1, newPosition.X+1);
	WriteMaster(buf, count);
    }

    CursorPosition = newPosition;
    CursorKnown = TRUE;

#if 0  // probably not a good idea unless this screen is visible.
    SetConsoleCursorPosition(hCopyScreenBuffer, CursorPosition);
#endif
}

/*
 *-----------------------------------------------------------------------------
 *
 * ConsoleDebugger::OnSetConsoleMode --
 *
 *	This function gets called when a SetConsoleMode breakpoint
 *	is hit.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Sets some flags that are used in determining echoing
 *	characteristics of the slave driver.
 *
 *-----------------------------------------------------------------------------
 */

void
ConsoleDebugger::OnSetConsoleMode(Process *proc, ThreadInfo *threadInfo,
    Breakpoint *brkpt, PDWORD returnValue, DWORD direction)
{
    DWORD i;
    BOOL found;

    // The console mode seems to get set even if the return value is FALSE
    if (*returnValue == FALSE) {
	return;
    }
    for (found = FALSE, i = 0; i < proc->consoleHandlesMax; i++) {
	if (threadInfo->args[0] == proc->consoleHandles[i]) {
	    found = TRUE;
	    break;
	}
    }
    if (found) {
	ConsoleInputMode = threadInfo->args[1];
    }

#if 0 // probably not a good idea at this time
    SetConsoleMode(hCopyScreenBuffer, ConsoleInputMode);
#endif
}

/*
 *-----------------------------------------------------------------------------
 *
 * ConsoleDebugger::OnSetConsoleOutputCP --
 *
 * Results:
 *	None
 *
 * Notes:
 *
 *-----------------------------------------------------------------------------
 */

void
ConsoleDebugger::OnSetConsoleOutputCP(Process *proc, ThreadInfo *threadInfo,
    Breakpoint *brkpt, PDWORD returnValue, DWORD direction)
{
    if (*returnValue == FALSE) {
	return;
    }
    ConsoleOutputCP = (UINT) threadInfo->args[0];

#if 0   // this might be trouble.
    SetConsoleOutputCP(ConsoleOutputCP);
#endif
}

/*
 *-----------------------------------------------------------------------------
 *
 * ConsoleDebugger::OnSetConsoleTextAttribute --
 *
 * Results:
 *	None
 *
 * Notes:
 *
 *-----------------------------------------------------------------------------
 */

void
ConsoleDebugger::OnSetConsoleTextAttribute(Process *proc, ThreadInfo *threadInfo,
    Breakpoint *brkpt, PDWORD returnValue, DWORD direction)
{
    WORD wAttributes;

    wAttributes = (WORD) threadInfo->args[1];
    SetConsoleTextAttribute(hCopyScreenBuffer, wAttributes);
}

/*
 *-----------------------------------------------------------------------------
 *
 * ConsoleDebugger::OnSetConsoleWindowInfo --
 *
 *	This function gets called when a SetConsoleWindowInfo breakpoint
 *	is hit.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Updates the current console cursor position
 *
 *-----------------------------------------------------------------------------
 */

void
ConsoleDebugger::OnSetConsoleWindowInfo(Process *proc,
    ThreadInfo *threadInfo, Breakpoint *brkpt, PDWORD returnValue,
    DWORD direction)
{
    HANDLE hConsoleOutput;
    BOOL bAbsolute;
    SMALL_RECT ConsoleWindowRect;
    PVOID ptr;

    if (*returnValue == FALSE) {
	return;
    }

    hConsoleOutput = (HANDLE) threadInfo->args[0];
    bAbsolute = threadInfo->args[1];
    ptr = (PVOID) threadInfo->args[2];
    if (ptr) {
	ReadSubprocessMemory(proc, ptr, &ConsoleWindowRect, sizeof(SMALL_RECT));
    }

    // TODO: fix me!  What do we do here?

    SetConsoleWindowInfo(hCopyScreenBuffer, bAbsolute, &ConsoleWindowRect);
}

/*
 *-----------------------------------------------------------------------------
 *
 * ConsoleDebugger::OnWriteConsoleA --
 *
 *	This function gets called when an WriteConsoleA breakpoint
 *	is hit.  The data is also redirected to expect since expect
 *	normally couldn't see any output going through this interface.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Prints some output.
 *
 *-----------------------------------------------------------------------------
 */

void
ConsoleDebugger::OnWriteConsoleA(Process *proc, ThreadInfo *threadInfo,
    Breakpoint *brkpt, PDWORD returnValue, DWORD direction)
{
    CHAR buf[1024];
    PVOID ptr;
    DWORD n, dummy;
    PCHAR p = buf;

    if (*returnValue == 0) {
	return;
    }
    // Get number of bytes written
    ptr = (PVOID) threadInfo->args[3];
    if (ptr == 0L) {
	n = threadInfo->args[2];
    } else {
	ReadSubprocessMemory(proc, ptr, &n, sizeof(DWORD));
    }
    if (n >= 1024) {
	p = new CHAR [n+1];
    }

    ptr = (PVOID) threadInfo->args[1];
    ReadSubprocessMemory(proc, ptr, p, n * sizeof(CHAR));
    p[n] = '\0';

    WriteMaster(p, n);

    WriteConsole(hCopyScreenBuffer, p, n, &dummy, 0L);

    if (p != buf) {
	delete [] p;
    }
    CursorKnown = FALSE;
}

/*
 *-----------------------------------------------------------------------------
 *
 * ConsoleDebugger::OnWriteConsoleW --
 *
 *	This function gets called when an WriteConsoleW breakpoint
 *	is hit.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Prints some output.
 *
 *-----------------------------------------------------------------------------
 */

void
ConsoleDebugger::OnWriteConsoleW(Process *proc, ThreadInfo *threadInfo,
    Breakpoint *brkpt, PDWORD returnValue, DWORD direction)
{
    static WCHAR buf[1024];
    static CHAR ansi[2048];
    PVOID ptr;
    DWORD n,dummy;
    PWCHAR p;
    PCHAR a;
    int asize;
    int w;

    if (*returnValue == 0) {
	return;
    }

    ptr = (PVOID) threadInfo->args[1];
    n = threadInfo->args[2];

    if (n >= 1024) {
	p = new WCHAR [n + 1];
	asize = n * sizeof(WCHAR) * sizeof(CHAR);
	a = new CHAR [asize + 1];
    } else {
	p = buf;
	a = ansi;
	asize = sizeof(ansi);
    }
    ReadSubprocessMemory(proc, ptr, p, n * sizeof(WCHAR));

    // Convert to ASCII and write-out the intercepted data.
    //
    w = WideCharToMultiByte(CP_ACP, 0, p, n, a, asize, 0L, 0L);
    a[w] = '\0';

    WriteMaster(a, w);

    WriteConsole(hCopyScreenBuffer, a, w, &dummy, 0L);

    if (p != buf) {
	delete [] p;
	delete [] a;
    }
    CursorKnown = FALSE;
}

/*
 *-----------------------------------------------------------------------------
 *
 * ConsoleDebugger::OnWriteConsoleOutputA --
 *
 *	This function gets called when an WriteConsoleOutputA breakpoint
 *	is hit.  The data is also redirected to expect since expect
 *	normally couldn't see any output going through this interface.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Prints some output.
 *
 *-----------------------------------------------------------------------------
 */

void
ConsoleDebugger::OnWriteConsoleOutputA(Process *proc,
    ThreadInfo *threadInfo, Breakpoint *brkpt, PDWORD returnValue,
    DWORD direction)
{
    CHAR buf[1024];
    PVOID ptr;
    DWORD n;
    CHAR *p, *end;
    int maxbuf;
    COORD bufferSize;
    COORD bufferCoord;
    COORD curr;
    SMALL_RECT writeRegion;
    CHAR_INFO *charBuf, *pcb;
    SHORT x, y;

    if (*returnValue == 0) {
	return;
    }

    bufferSize = *((PCOORD) &threadInfo->args[2]);
    bufferCoord = *((PCOORD) &threadInfo->args[3]);
    ptr = (PVOID) threadInfo->args[4]; // Get the rectangle written
    if (ptr == 0L) return;
    ReadSubprocessMemory(proc, ptr, &writeRegion, sizeof(SMALL_RECT));

    ptr = (PVOID) threadInfo->args[1]; // Get character array
    if (ptr == 0L) return;

    n = bufferSize.X * bufferSize.Y * sizeof(CHAR_INFO);
    charBuf = new CHAR_INFO [n];

    ReadSubprocessMemory(proc, ptr, charBuf, n);

    for (y = 0; y <= writeRegion.Bottom - writeRegion.Top; y++) {
	pcb = charBuf + ((y + bufferCoord.Y) * bufferSize.X) + bufferCoord.X;
	p = buf;
	maxbuf = sizeof(buf);
	end = buf + maxbuf;
	for (x = 0; x <= writeRegion.Right - writeRegion.Left; x++, pcb++) {
	    *p++ = pcb->Char.AsciiChar;
	    if (p == end) {
		WriteMaster(buf, maxbuf);
		p = buf;
	    }
	}
	curr.X = writeRegion.Left;
	curr.Y = writeRegion.Top + y;
	n = writeRegion.Right - writeRegion.Left;
	CreateVtSequence(proc, curr, n);
	maxbuf = p - buf;
	WriteMaster(buf, maxbuf);
	buf[maxbuf] = 0;
    }

    WriteConsoleOutput(hCopyScreenBuffer, charBuf, bufferSize, bufferCoord, &writeRegion);

    delete [] charBuf;
}

/*
 *-----------------------------------------------------------------------------
 *
 * ConsoleDebugger::OnWriteConsoleOutputW --
 *
 *	This function gets called when an WriteConsoleOutputW breakpoint
 *	is hit.  The data is also redirected to expect since expect
 *	normally couldn't see any output going through this interface.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Prints some output.
 *
 *-----------------------------------------------------------------------------
 */

void
ConsoleDebugger::OnWriteConsoleOutputW(Process *proc,
    ThreadInfo *threadInfo, Breakpoint *brkpt, PDWORD returnValue,
    DWORD direction)
{
    WCHAR buf[1024];
    PVOID ptr;
    DWORD n;
    WCHAR *p, *end;
    int maxbuf;
    COORD bufferSize;
    COORD bufferCoord;
    COORD curr;
    SMALL_RECT writeRegion;
    CHAR_INFO *charBuf, *pcb;
    SHORT x, y;

    if (*returnValue == 0) {
	return;
    }

    bufferSize = *((PCOORD) &threadInfo->args[2]);
    bufferCoord = *((PCOORD) &threadInfo->args[3]);
    ptr = (PVOID) threadInfo->args[4]; // Get the rectangle written
    if (ptr == 0L) return;
    ReadSubprocessMemory(proc, ptr, &writeRegion, sizeof(SMALL_RECT));

    ptr = (PVOID) threadInfo->args[1]; // Get character array
    if (ptr == 0L) return;

    n = bufferSize.X * bufferSize.Y * sizeof(CHAR_INFO);
    charBuf = new CHAR_INFO [n];

    ReadSubprocessMemory(proc, ptr, charBuf, n);

    for (y = 0; y <= writeRegion.Bottom - writeRegion.Top; y++) {
	pcb = charBuf + ((y + bufferCoord.Y) * bufferSize.X) + bufferCoord.X;
	p = buf;
	maxbuf = sizeof(buf);
	end = buf + maxbuf;
	for (x = 0; x <= writeRegion.Right - writeRegion.Left; x++, pcb++) {
	    *p++ = (CHAR) (pcb->Char.UnicodeChar & 0xff);
	    if (p == end) {
		WriteMaster((char *)buf, maxbuf);
		p = buf;
	    }
	}
	curr.X = writeRegion.Left;
	curr.Y = writeRegion.Top + y;
	n = writeRegion.Right - writeRegion.Left;
	CreateVtSequence(proc, curr, n);
	maxbuf = p - buf;
	WriteMaster((char *)buf, maxbuf);
	buf[maxbuf] = 0;
    }

    WriteConsoleOutput(hCopyScreenBuffer, charBuf, bufferSize, bufferCoord, &writeRegion);

    delete [] charBuf;
}

/*
 *-----------------------------------------------------------------------------
 *
 * ConsoleDebugger::OnWriteConsoleOutputCharacterA --
 *
 *	This function gets called when an WriteConsoleOutputCharacterA breakpoint
 *	is hit.  The data is also redirected to expect since expect
 *	normally couldn't see any output going through this interface.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Prints some output.
 *
 *-----------------------------------------------------------------------------
 */

void
ConsoleDebugger::OnWriteConsoleOutputCharacterA(Process *proc,
    ThreadInfo *threadInfo, Breakpoint *brkpt, PDWORD returnValue, DWORD direction)
{
    static CHAR buf[1024];
    PVOID ptr;
    DWORD nLength, dwWritten;
    PCHAR lpCharacter;
    COORD dwWriteCoord;

    if (*returnValue == 0) {
	return;
    }
    // Get number of bytes written
    ptr = (PVOID) threadInfo->args[4];
    if (ptr == 0L) {
	nLength = threadInfo->args[2];
    } else {
	ReadSubprocessMemory(proc, ptr, &nLength, sizeof(DWORD));
    }

    dwWriteCoord = *((PCOORD) &threadInfo->args[3]);
    CreateVtSequence(proc, dwWriteCoord, nLength);

    if (nLength >= 1024) {
	lpCharacter = new CHAR [nLength + 1];
    } else {
	lpCharacter = buf;
    }

    ptr = (PVOID) threadInfo->args[1];
    ReadSubprocessMemory(proc, ptr, lpCharacter, nLength * sizeof(CHAR));
    lpCharacter[nLength] = '\0';
    WriteMaster(lpCharacter, nLength);

    WriteConsoleOutputCharacter(hCopyScreenBuffer, lpCharacter, nLength,
				dwWriteCoord, &dwWritten);

    if (lpCharacter != buf) {
	delete [] lpCharacter;
    }
    CursorKnown = FALSE;
}

/*
 *-----------------------------------------------------------------------------
 *
 * ConsoleDebugger::OnWriteConsoleOutputCharacterW --
 *
 *	This function gets called when an WriteConsoleOutputCharacterW
 *	breakpoint is hit.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Prints some output.
 *
 *-----------------------------------------------------------------------------
 */

void
ConsoleDebugger::OnWriteConsoleOutputCharacterW(Process *proc,
    ThreadInfo *threadInfo, Breakpoint *brkpt, PDWORD returnValue,
    DWORD direction)
{
    static WCHAR buf[1024];
    static CHAR ansi[2048];
    PVOID ptr;
    DWORD nLength, dwWritten;
    PWCHAR p;
    PCHAR lpCharacter;
    COORD dwWriteCoord;
    int asize;

    if (*returnValue == 0) {
	return;
    }
    // Get number of bytes written
    ptr = (PVOID) threadInfo->args[4];
    if (ptr == 0L) {
	nLength = threadInfo->args[2];
    } else {
	ReadSubprocessMemory(proc, ptr, &nLength, sizeof(DWORD));
    }

    dwWriteCoord = *((PCOORD) &threadInfo->args[3]);
    CreateVtSequence(proc, dwWriteCoord, nLength);

    if (nLength >= 1024) {
	p = new WCHAR [nLength + 1];
	asize = nLength * 2 * sizeof(CHAR);
	lpCharacter = new CHAR [asize + 1];
    } else {
	p = buf;
	lpCharacter = ansi;
	asize = sizeof(ansi);
    }

    ptr = (PVOID) threadInfo->args[1];
    ReadSubprocessMemory(proc, ptr, p, nLength * sizeof(WCHAR));

    // Convert to ASCI and Write the intercepted data to the pipe.
    nLength = WideCharToMultiByte(CP_ACP, 0, p, nLength, lpCharacter,
				  asize, 0L, 0L);
    lpCharacter[nLength] = '\0';
    WriteMaster(lpCharacter, nLength);

    WriteConsoleOutputCharacter(hCopyScreenBuffer, lpCharacter, nLength,
				dwWriteCoord, &dwWritten);

    if (p != buf) {
	delete [] p;
	delete [] lpCharacter;
    }
    CursorKnown = FALSE;
}

/*
 *-----------------------------------------------------------------------------
 *
 * ConsoleDebugger::OnIsWindowVisible --
 *
 *	This routine gets called when IsWindowVisible is called.
 *	The MKS Korn shell uses this as an indication of a window
 *	that can be seen by the user.  If the window can't be seen,
 *	it pops up a graphical error notification.  We really, really
 *	don't want those damn things popping up, so this helps avoid
 *	it.  And there really doesn't seem to be any good reason to
 *	return FALSE given that nobody is ever going to see anything.
 *
 * Results:
 *	None
 *
 *-----------------------------------------------------------------------------
 */

void
ConsoleDebugger::OnIsWindowVisible(Process *proc, ThreadInfo *threadInfo,
    Breakpoint *brkpt, PDWORD returnValue, DWORD direction)
{
    *returnValue = TRUE;
}

/*
 *-----------------------------------------------------------------------------
 *
 * ConsoleDebugger::OnWriteFile --
 *
 *	This function gets called when a WriteFile
 *	breakpoint is hit.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	none yet..
 *
 *-----------------------------------------------------------------------------
 */

void
ConsoleDebugger::OnWriteFile(Process *proc,
    ThreadInfo *threadInfo, Breakpoint *brkpt, PDWORD returnValue,
    DWORD direction)
{
    // TODO: is this a console handle in the slave?
#if 0
    HANDLE hFile;		    // handle to file
    PCHAR lpBuffer;		    // data buffer
    DWORD nNumberOfBytesToWrite;    // number of bytes to write
    //LPOVERLAPPED lpOverlapped;	    // overlapped buffer
    PVOID ptr;

    hFile = (HANDLE) threadInfo->args[0];

    // Get number of bytes written, if available.
    ptr = (PVOID) threadInfo->args[4];
    if (ptr == 0L) {
	nNumberOfBytesToWrite = threadInfo->args[2];
    } else {
	ReadSubprocessMemory(proc, ptr, &nNumberOfBytesToWrite, sizeof(DWORD));
    }

    ptr = (PVOID) threadInfo->args[1];
    lpBuffer = new CHAR [nNumberOfBytesToWrite];
    ReadSubprocessMemory(proc, ptr, lpBuffer, nNumberOfBytesToWrite);

    delete [] lpBuffer;
#endif
}
