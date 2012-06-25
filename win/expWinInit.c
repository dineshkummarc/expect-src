/* ----------------------------------------------------------------------------
 * expWinInit.c --
 *
 *	Win OS specific inits.
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
 * Copyright (c) 2003-2004 ActiveState Corporation
 *	Work by David Gravereaux <davygrvy@pobox.com> for any Win32 OS.
 *	Based on work by Gordon Chaffee <chaffee@bmrc.berkeley.edu>
 *
 * ----------------------------------------------------------------------------
 * URLs:    http://expect.nist.gov/
 *	    http://expect.sf.net/
 *	    http://bmrc.berkeley.edu/people/chaffee/expectnt.html
 * ----------------------------------------------------------------------------
 * RCS: @(#) $Id: expWinInit.c,v 1.1.4.5 2003/08/26 20:46:52 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinPort.h"

#ifdef _MSC_VER
    /* Only do this when MSVC++ is compiling us. */
#   ifdef USE_TCL_STUBS
	/* Mark this .obj as needing tcl's Stubs library. */
#	pragma comment(lib, "tclstub" \
		STRINGIFY(JOIN(TCL_MAJOR_VERSION,TCL_MINOR_VERSION)) ".lib")
#	if !defined(_MT) || !defined(_DLL) || defined(_DEBUG)
	    /* This fixes a bug with how the Stubs library was compiled.
	     * The requirement for msvcrt.lib from tclstubXX.lib should
	     * be removed. */
#	    pragma comment(linker, "-nodefaultlib:msvcrt.lib")
#	endif
#   elif !defined(STATIC_BUILD)
	/* Mark this .obj needing the import library */
#	pragma comment(lib, "tcl" \
		STRINGIFY(JOIN(TCL_MAJOR_VERSION,TCL_MINOR_VERSION)) ".lib")
#   endif
#endif


static ExpWinProcs expAsciiProcs = {
    0,
    CreateFileA,
    CreateProcessA,
    GetFileAttributesA,
    GetShortPathNameA,
    SearchPathA
};

static ExpWinProcs expUnicodeProcs = {
    1,
    (HANDLE (WINAPI *)(const TCHAR *, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE)) CreateFileW,
    (BOOL (WINAPI *)(const TCHAR *, TCHAR *, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, const TCHAR *, LPSTARTUPINFO, LPPROCESS_INFORMATION)) CreateProcessW,
    (DWORD (WINAPI *)(const TCHAR *)) GetFileAttributesW,
    (DWORD (WINAPI *)(const TCHAR *, TCHAR *, DWORD)) GetShortPathNameW,
    (DWORD (WINAPI *)(const TCHAR *, const TCHAR *, const TCHAR *, DWORD, TCHAR *, TCHAR **)) SearchPathW
};

ExpWinProcs *expWinProcs = &expAsciiProcs;
static int initialized = 0;

static BOOL ExpWinNTDebugVar      = FALSE;
static BOOL ExpWinNTStripVT100Var = TRUE;

HMODULE expModule = NULL;

/*
 *----------------------------------------------------------------------
 *  ExpWinInit --
 *
 *	Switches to the correct native API at run-time.  Works in
 *	tandem with Tcl_WinUtfToTchar().
 *
 *  Returns:
 *	Standard Tcl error code.
 *
 *----------------------------------------------------------------------
 */
int
ExpWinInit(Tcl_Interp *interp)
{
    if (!initialized) {
	if (TclWinGetPlatformId() == VER_PLATFORM_WIN32_NT) {	 
	    expWinProcs = &expUnicodeProcs;
	}

	/*
	 * Link to exp::winnt_debug for extra info
	 */
	if (Tcl_LinkVar(interp, "::exp::winnt_debug",
		    (char *) &ExpWinNTDebugVar,
		    TCL_LINK_BOOLEAN) != TCL_OK) {
	    Tcl_ResetResult(interp);
	}

	/*
	 * Link to exp::winnt_usevt100 for extra info
	 */
	if (Tcl_LinkVar(interp, "::exp::winnt_stripvt100",
		    (char *) &ExpWinNTStripVT100Var,
		    TCL_LINK_BOOLEAN) != TCL_OK) {
	    Tcl_ResetResult(interp);
	}

#ifdef STATIC_BUILD
	/* this may not be correct, but closer than not. */
	expModule = TclWinGetTclInstance();
#endif

	/* Magic env var to raise expect priority */
	if (getenv("EXPECT_PROCESS_HIGH_PRIORITY") != NULL) {
	    SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
	}
    }
    return TCL_OK;
}

BOOL
ExpWinNTDebug(void)
{
    return ExpWinNTDebugVar;
}

BOOL
ExpWinNTStripVT100(void)
{
    return ExpWinNTStripVT100Var;
}

#ifndef STATIC_BUILD
BOOL WINAPI
DllMain (HINSTANCE hInst, ULONG ulReason, LPVOID lpReserved)
{
    if (ulReason == DLL_PROCESS_ATTACH) {
	expModule = hInst;
	DisableThreadLibraryCalls(expModule);
    }
    return TRUE; /* successful load */
}
#endif
