/* 
 * expStubLib.c --
 *
 *	Stub object that will be statically linked into extensions that wish
 *	to access Expect.
 *
 * Copyright (c) 2002 by Telindustrie, LLC
 * Copyright (c) 1998 Paul Duffin.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * RCS: $Id: expStubLib.c,v 1.1.2.2 2002/02/10 08:40:55 davygrvy Exp $
 */

/*
 * We need to ensure that we use the stub macros so that this file contains
 * no references to any of the stub functions.  This will make it possible
 * to build an extension that references Tcl_InitStubs but doesn't end up
 * including the rest of the stub functions.
 */

#ifndef USE_TCL_STUBS
#define USE_TCL_STUBS
#endif
#undef USE_TCL_STUB_PROCS

/*
 * This ensures that the Exp_InitStubs has a prototype in
 * exp.h and is not the macro that turns it into Tcl_PkgRequire
 */

#ifndef USE_EXP_STUBS
#define USE_EXP_STUBS
#endif

#include "expInt.h"

ExpStubs	*expStubsPtr;
ExpIntStubs	*expIntStubsPtr;
ExpPlatStubs	*expPlatStubsPtr;
ExpIntPlatStubs	*expIntPlatStubsPtr;

/*
 *----------------------------------------------------------------------
 *
 * Exp_InitStubs --
 *
 *	Tries to initialise the stub table pointers and ensures that
 *	the correct version of Expect is loaded.
 *
 * Results:
 *	The actual version of Expect that satisfies the request, or
 *	NULL to indicate that an error occurred.
 *
 * Side effects:
 *	Sets the stub table pointers.
 *
 *----------------------------------------------------------------------
 */

CONST char *
Exp_InitStubs (interp, version, exact)
    Tcl_Interp *interp;
    CONST char *version;
    int exact;
{
    CONST char *actualVersion;
    
    actualVersion = Tcl_PkgRequireEx(interp, "Expect", version, exact,
        (ClientData *) &expStubsPtr);

    if (actualVersion == NULL) {
	expStubsPtr = NULL;
	return NULL;
    }

    if (expStubsPtr->hooks) {
	expIntStubsPtr = expStubsPtr->hooks->expIntStubs;
	expPlatStubsPtr = expStubsPtr->hooks->expPlatStubs;
	expIntPlatStubsPtr = expStubsPtr->hooks->expIntPlatStubs;
    } else {
	expIntStubsPtr = NULL;
	expPlatStubsPtr = NULL;
	expIntPlatStubsPtr = NULL;
    }
    
    return actualVersion;
}
