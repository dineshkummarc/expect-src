/* -*- c -*-
 *
 * Copyright (c) 2004 ActiveState Corp
 * Code handling license checks etc for TclPro.
 *
 * [Variant FINAL]: Accept Expect4Win licenses, and
 *
 * Copied into the sources by 'setup.sh' according to the directives
 * chosen in 'setvars.sh'.
 */

#include <time.h>

/*
 * I. Import the general license module of ActiveState
 */

#ifndef LICENSE_CHECK
#   define LICENSE_CHECK 1
#endif
#if LICENSE_CHECK
#   define  LIC_STATIC static
#   include "../../../License_V8/Runtime/Features.h"
#   include "../../../License_V8/Runtime/License.c"

/* Date of release for Expect 4 Win 1.0 */

#   define RELEASE_YEAR  2004 /* Future : Sets this through the build scripts */
#   define RELEASE_MONTH   02 /* s.a.                                         */
#   define RELEASE_DAY     28 /* s.a.                                         */

/* Date of release for Komodo 2.0 */

#   define KO_RELEASE_YEAR  2002 /* Future : Sets this through the build scripts */
#   define KO_RELEASE_MONTH    9 /* s.a.                                         */
#   define KO_RELEASE_DAY     10 /* s.a.                                         */
#endif

#include <tcl.h>

#if LICENSE_CHECK
static int release_y = RELEASE_YEAR;
static int release_m = RELEASE_MONTH;
static int release_d = RELEASE_DAY;

/*
 * Ia. Internal helper to debug the handling 1.5 licenses.
 */

static void
tp_dump_license (interp, feature)
     Tcl_Interp* interp;
     char*       feature;
{
  char* _license = NULL;
  char* err      = NULL;
  char* _feature = NULL;
  char* _issued  = NULL;
  int issued;

  if (!licReadFileEx(&_license, &err, NULL, LICFL_NONE)) {
    licFree(err);
    return;
  }
  _feature = licFindFeature(_license, feature);
  if (!_feature)
    return;

  _issued = licFetchValue(_feature, LK_ISSUED);
  if (!_issued || !StringToDate(_issued, &issued) || issued == 0) {
    licFree(_issued);
    return;
  }

  printf ("[%s]: Issued '%s' = %d\n", feature, _issued, issued);

  licFree (_issued);
  return;
}
#endif

/*
 * II. The error messages
 */

#define PURCHASE "\nTo purchase or upgrade a license, or to obtain a free evaluation license please visit: http://www.ActiveState.com/Products/ASPN_Tcl\n\nEmail: Sales@ActiveState.com\nToll-free (North America): 1.866.866.2802\nPhone: 604.484.6800\nFax: 604.484.6499"

/* All strings defined below have to contain the word 'license' in them. Komodo
 * tests for the existence of this word when checking if the TDK is available and
 * properly licensed.
 */
#define ASPNTCL_LICENSE_EXPIRED	  "Your trial license for Expect for Windows has expired."
#define ASPNTCL_LICENSE_NOCOVER   "Your Expect for Windows license does not cover this release."
#define ASPNTCL_LICENSE_NOFEAT    "Your license file for Expect for Windows is invalid."
/* "does not contain the 'TclPro2' feature, nor 'ASPN Tcl'." */
#define ASPNTCL_LICENSE_NOFILE    "No valid license file found."
#define ASPNTCL_BOGUS_CONTROL     "Bogus control setting. Internal error."

/*
 * III. Define the license check procedure called by the TclPro
 * packages when the license has to be checked.
 */

static Tcl_Obj* lexpiry   = NULL;
static Tcl_Obj* luser     = NULL;
static Tcl_Obj* lusermail = NULL;
static Tcl_Obj* ltype     = NULL;


static CONST char*
TdkCheckFeatures _ANSI_ARGS_ ((CONST char* features [], int* status));

static void
TdkRetrieve _ANSI_ARGS_ ((CONST char* feature));


extern int
tp_check_license (interp)
     Tcl_Interp* interp;
{
#if LICENSE_CHECK
  int         skip = 0;
  int         lic_status;
  CONST char* message     = NULL;
  CONST char* features [] = { LF_EXPECT4WIN, LF_ASPNTCL, NULL, NULL, NULL };
  CONST char* chosen      = NULL;
  CONST char* licExternalControl;

  /*
   * Check for a valid Expect4Windows, then ASPN Tcl license.
   */

#ifdef WIN32
  char *err  = NULL;
  char *home = NULL;
  int   homelen;
  struct stat statbuf;

  if (licGetHomeDir(&home, &homelen, &err, FALSE)) {
      char* filename = (char*)ckalloc(homelen + 20);
      strcpy(filename, home);
      strcat(filename, "/ActiveState.lic");
      licFree(home);
      home = NULL;

      if (stat(filename, &statbuf) != 0) {
	  /* error condition - file doesn't exist */
	  home = (char*) Tcl_GetVar2(interp, "env", "HOME", TCL_GLOBAL_ONLY);
	  if (home) {
	      Tcl_SetVar2(interp, "env", "HOME", "", TCL_GLOBAL_ONLY);
	  }
      }
      ckfree(filename);
  }
  licFree(err);
#endif

  /* Check if the tcl variable 'tdk_feature' is set, and if so, then
   * use it to overide the first feature we search for. This allows
   * the higher level code to ask for a different feature than is
   * hardcoded into the library. This is used, for example, by the
   * application 'kotcldebug' to look for a Komodo license instead of
   * the TDK.
   */

  licExternalControl = Tcl_GetVar2 (interp, "tdk_feature", NULL, TCL_GLOBAL_ONLY);
  if (licExternalControl != NULL) {
    if (strcmp("komodo-personal",licExternalControl) == 0) {

      features [0] = LF_KOMODO20_PERSONAL;
      features [1] = LF_KOMODO;
      features [2] = LF_KOMODO20_PROFESSIONAL;
      features [3] = NULL;

      release_y = KO_RELEASE_YEAR;
      release_m = KO_RELEASE_MONTH;
      release_d = KO_RELEASE_DAY;

    } else if (strcmp("komodo-professional",licExternalControl) == 0) {

      features [0] = LF_KOMODO20_PROFESSIONAL;
      features [1] = LF_KOMODO;
      features [2] = NULL;

      release_y = KO_RELEASE_YEAR;
      release_m = KO_RELEASE_MONTH;
      release_d = KO_RELEASE_DAY;

    } else if (strcmp("komodo-beta",licExternalControl) == 0) {

      features [0] = LF_KOMODO20_BETA1;
      features [1] = NULL;

      release_y = KO_RELEASE_YEAR;
      release_m = KO_RELEASE_MONTH;
      release_d = KO_RELEASE_DAY;

    } else if (strcmp("no-license",licExternalControl) == 0) {

         skip = 1;

         /*
	  * Fake data for the stuff which normally comes out the
	  * license, but we don't have one in this situation
	  */

	 luser     = Tcl_NewStringObj ("Unknown", -1);
	 lusermail = Tcl_NewStringObj ("unknown@unknown", -1);
	 ltype     = Tcl_NewStringObj ("unknown", -1);
	 lexpiry   = Tcl_NewStringObj ("99-99-9999", 0);

    } else if (strcmp("komodo-beta-hard",licExternalControl) == 0) {

        /* Hardwired check for timeout in two months.
	 * If this passes we do no other checks anymore.
	 */

#define EXPIRATION_TIME 1060560832 /* -- Sun Aug 10 17:13:52 PDT 2003 -- */

        time_t currentTime;
        time(&currentTime);
        if (currentTime <= EXPIRATION_TIME) {
	    skip = 1;

	    /*
	     * Fake data for the stuff which normally comes out the
	     * license, but we don't have one in this situation
	     */

	    luser     = Tcl_NewStringObj ("Unknown", -1);
	    lusermail = Tcl_NewStringObj ("unknown@unknown", -1);
	    ltype     = Tcl_NewStringObj ("Beta", -1);
	    lexpiry   = Tcl_NewStringObj ("10-08-2003", 0);

        } else {
	    message = ASPNTCL_LICENSE_EXPIRED;
        }

    } else {
        /* Illegal setting, cannot match */
        message = ASPNTCL_BOGUS_CONTROL;
    }
  }

  if (!skip && (message == NULL)) {
      chosen = TdkCheckFeatures (features, &lic_status);

      switch (lic_status) {
      case LIC_VALID     :  /* Valid feature found                 */
	TdkRetrieve (chosen);
	break;
      case LIC_EXPIRED   :  /* Expired trial feature               */
	message = ASPNTCL_LICENSE_EXPIRED;
	break;
      case LIC_NOTCOVERED:  /* Feature doesn't cover this release  */
	message = ASPNTCL_LICENSE_NOCOVER;
	break;
      case LIC_NOFEATURE :  /* No matching feature in license file */
	message = ASPNTCL_LICENSE_NOFEAT;
	break;
      case LIC_NOFILE    :  /* No license file at all              */
	message = ASPNTCL_LICENSE_NOFILE;
	break;
      }
  }

#ifdef WIN32
  if (home) {
      Tcl_SetVar2(interp, "env", "HOME", home, TCL_GLOBAL_ONLY);
  }
#endif

  if (message != NULL) {
    Tcl_SetResult    (interp, (char*) message,  TCL_STATIC);
    Tcl_AppendResult (interp, PURCHASE, NULL);
    return TCL_ERROR;
  }
#endif /* LICENSE_CHECK */
  return TCL_OK;
}


#if LICENSE_CHECK
static CONST char*
TdkCheckFeatures (features, status)
     CONST char* features [];
     int*        status;
{
    CONST char* lookfor [2];
    CONST char* f;
    int   i, lic_status;

    for (i = 0, f = features [0];
	 f != NULL;
	 i++, f = features [i]) {
        lookfor [0] = f;
	lookfor [1] = NULL;

	lic_status = licCheckEx (lookfor,
	   release_y, release_m, release_d,
	   NULL /* No storage location for name of found feature */,
	   NULL /* No storage location for desc of found feature */,
	   NULL /* No in-memory license string */,
	   NULL /* No explicit license file, search for it */,
	   LICFL_NONE /* No flags = no special behaviour */
	   );

	*status = lic_status;
	if (lic_status == LIC_VALID) {
	    return f;
	}
    }
    return NULL;
}


static void
TdkRetrieve (chosen)
     CONST char* chosen;
{
    /*
     * Get the expiration date and othjer information, and convert
     * them into Tcl objects
     *
     * 99-99-9999 => Empty, else as is.
     */

    char* _license = NULL;
    char* err      = NULL;
    char* _feature = NULL;
    char* _value   = NULL;

    licReadFileEx(&_license, &err, NULL, LICFL_NONE);

    _feature = licFindFeature(_license, chosen);

    _value    = licFetchValue(_feature, LK_USERNAME);
    luser     = Tcl_NewStringObj (_value, -1);
    licFree (_value);

    _value    = licFetchValue(_feature, LK_USEREMAIL);
    lusermail = Tcl_NewStringObj (_value, -1);
    licFree (_value);

    _value    = licFetchValue(_feature, LK_LICENSETYPE);
    ltype     = Tcl_NewStringObj (_value, -1);
    licFree (_value);

    _value  = licFetchValue(_feature, LK_EXPIRATION);
    if (!strcmp (_value, "99-99-9999")) {
        lexpiry = Tcl_NewStringObj ("", 0);
    } else {
        lexpiry = Tcl_NewStringObj (_value, -1);
    }
    licFree (_value);

    licFree (_feature);
    licFree (_license);
    licFree (err);

    Tcl_IncrRefCount (lexpiry);
    Tcl_IncrRefCount (luser);
    Tcl_IncrRefCount (lusermail);
    Tcl_IncrRefCount (ltype);
}


/*
 * IV. Define a tcl command which can be used by the TDK applications
 * to inquire their expiration date and other information about the
 * license.
 */

static int
TdkLicenseObjCmd _ANSI_ARGS_ ((ClientData     dummy,
			       Tcl_Interp*    interp,
			       int            objc,
			       Tcl_Obj *CONST objv []));

static int
TdkLicenseObjCmd (dummy, interp, objc, objv)
     ClientData     dummy;
     Tcl_Interp*    interp;
     int            objc;
     Tcl_Obj *CONST objv [];

{
    int index;

    /* Dispatch according to the chosen subcommand.
     */

    static char *optionStrings[] = {
        "expiration-date", "user-name", "user-email", "type",
	NULL
    };
    enum options {
      L_EXPIRY, L_USER, L_USERMAIL, L_TYPE
    };

    if (objc < 2) {
    	Tcl_WrongNumArgs(interp, 1, objv, "expiration-date|user-name|user-email|type");
        return TCL_ERROR;
    }
    if (Tcl_GetIndexFromObj(interp, objv[1], optionStrings, "option", 0,
	    &index) != TCL_OK) {
	return TCL_ERROR;
    }

    switch ((enum options) index) {
        case L_EXPIRY:    Tcl_SetObjResult (interp, lexpiry);   break;
        case L_USER:      Tcl_SetObjResult (interp, luser);     break;
        case L_USERMAIL:  Tcl_SetObjResult (interp, lusermail); break;
        case L_TYPE:      Tcl_SetObjResult (interp, ltype);     break;
    }

    return TCL_OK;
}
#endif /* LICENSE_CHECK */
