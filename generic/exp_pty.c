/* exp_pty.c - generic routines to allocate and test ptys

Written by: Don Libes, NIST,  3/9/93

Design and implementation of this program was paid for by U.S. tax
dollars.  Therefore it is public domain.  However, the author and NIST
would appreciate credit if this program or parts of it are used.

*/

#include "expInt.h"

/* 
 * expDiagLog needs a different definition, depending on whether its
 * called inside of Expect or the clib.  Allow it to be set using this
 * function.  It's done here because this file (and pty_XXX.c) are the 
 * ones that call expDiagLog from the two different environments.
 */

static expDiagLogProc *expDiagLogPtrVal;

void
expDiagLogPtrSet(fn)
     expDiagLogProc *fn;
{
    expDiagLogPtrVal = fn;
}

void
expDiagLogPtr(str)
     CONST char *str;
{
    (*expDiagLogPtrVal)(str);
}


void
expDiagLogPtrX(fmt,num)
     CONST char *fmt;
     int num;
{
    static char buf[1000];
    sprintf(buf,fmt,num);
    (*expDiagLogPtrVal)(buf);
}


void
expDiagLogPtrStr(fmt,str1)
     CONST char *fmt;
     CONST char *str1;
{
    static char buf[1000];
    sprintf(buf,fmt,str1);
    (*expDiagLogPtrVal)(buf);
}

void
expDiagLogPtrStrStr(fmt,str1,str2)
     CONST char *fmt;
     CONST char *str1, *str2;
{
    static char buf[1000];
    sprintf(buf,fmt,str1,str2);
    (*expDiagLogPtrVal)(buf);
}

static expErrnoMsgProc *expErrnoMsgVal;

CONST char *
expErrnoMsg(errorNo)
    int errorNo;
{
    return (*expErrnoMsgVal)(errorNo);
}

void
expErrnoMsgSet(fn)
    expErrnoMsgProc *fn;
{
    expErrnoMsgVal = fn;
}
