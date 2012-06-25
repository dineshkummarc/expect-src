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
#   else
	/* Mark this .obj needing the import library */
#	pragma comment(lib, "tcl" \
		STRINGIFY(JOIN(TCL_MAJOR_VERSION,TCL_MINOR_VERSION)) ".lib")
#   endif
#endif



static Tcl_ObjCmdProc Test_SpawnObjCmd;

int
Test_SpawnObjCmd (
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    struct Tcl_Obj * CONST objv[])
{
    Tcl_Channel newChan;
    unsigned long pid;
    Exp_SpawnOptionSet opts;
    int j;

    /* Set option defaults. */
    opts.env = NULL;
    opts.dir = NULL;
    opts.ttyinit = 1;
    opts.ttycopy = 1;
    opts.echo = 1;
    opts.console = 0;
    opts.pty_only = 0;
    opts.leaveOpen = 0;
    opts.slave_write_ioctls = 1;
		/* by default, slave will be write-ioctled this many times */
    opts.slave_opens = 3;
		/* by default, slave will be opened this many times */
		/* first comes from initial allocation */
		/* second comes from stty */
		/* third is our own signal that stty is done */
    for (j = 0; j < NSIG; j++) {
	opts.ignore[j] = 0;
    }		/* don't ignore any signals in child by default */


    if (objc < 2) {
	Tcl_WrongNumArgs(interp, 1, objv, "<exe> ?args ...?");
	return TCL_ERROR;
    }

    newChan = Exp_CreateSpawnChannel(interp, &opts, objc-1, &objv[1], &pid);
    if (newChan) {
	Tcl_Obj *result[2];

	Tcl_RegisterChannel(interp, newChan);
	result[0] = Tcl_NewStringObj(Tcl_GetChannelName(newChan), -1);
	result[1] = Tcl_NewLongObj(pid);
	Tcl_SetObjResult(interp, Tcl_NewListObj(2, result));
	return TCL_OK;
    } else {
	return TCL_ERROR;
    }
}

TCL_DECLARE_MUTEX(initLock)
static int initialized = 0;


#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLEXPORT

EXTERN int
Expect_Init (Tcl_Interp *interp)
{
#ifdef USE_TCL_STUBS
    if (Tcl_InitStubs(interp, TCL_VERSION, 0) == 0L) {
	return TCL_ERROR;
    }
#endif

    Tcl_MutexLock(&initLock);
    if (!initialized) {
	ExpWinInit(interp);
    }
    ExpSpawnInit();
    Tcl_MutexUnlock(&initLock);

    Tcl_CreateObjCommand(interp, "exp::win_spawn", Test_SpawnObjCmd, NULL, NULL);
    Tcl_PkgProvide(interp, "Expect", EXP_VERSION);
    return TCL_OK;
}

HMODULE expModule;

#ifndef STATIC_BUILD
BOOL WINAPI
DllMain (HINSTANCE hInst, ULONG ulReason, LPVOID lpReserved)
{
    if (ulReason == DLL_PROCESS_ATTACH) {
	DisableThreadLibraryCalls(expModule = hInst);
    }
    return TRUE; /* successful load */
}
#endif
