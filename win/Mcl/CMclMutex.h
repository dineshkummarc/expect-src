//
// FILE: CMclMutex.h
//
// Copyright (c) 1997 by Aaron Michael Cohen and Mike Woodring
//
/////////////////////////////////////////////////////////////////////////
#ifndef __CMCLMUTEX_H__
#define __CMCLMUTEX_H__

#include "CMclGlobal.h"
#include "CMclKernel.h"

#ifdef _MSC_VER
#   pragma warning ( push )
    // Yes, CMclKernel declares a copy constructor but only to cause a
    // compiler error should it ever be accessed.  Don't warn us about
    // this oddity.
#   pragma warning ( disable : 4511 )
    // Ditto for the assignment operator.
#   pragma warning ( disable : 4512 )
#endif

class CMclMutex : public CMclKernel {
public:
    // constructors create a mutex object...
    CMclMutex( BOOL bInitialOwner = FALSE, LPCTSTR lpName = NULL, LPSECURITY_ATTRIBUTES lpMutexAttributes = NULL);
    
    // constructor opens an existing named mutex...
    // you must check the status after using this constructor,
    // it will NOT throw an error exception if the object cannot be opened...
    CMclMutex( LPCTSTR lpName, BOOL bInheritHandle = FALSE, DWORD dwDesiredAccess = MUTEX_ALL_ACCESS);

    // release a lock on a mutex...
    BOOL Release(void);
};

#ifdef _MSC_VER
#   pragma warning ( pop )
#endif

#endif



