//
// FILE: CMclAutoLock.h
//
// Copyright (c) 1997 by Aaron Michael Cohen and Mike Woodring
//
/////////////////////////////////////////////////////////////////////////
#ifndef __CMCLAUTOLOCK_H__
#define __CMCLAUTOLOCK_H__

#include "CMclGlobal.h"
#include "CMclMutex.h"
#include "CMclCritSec.h"

class CMclAutoLock {
private:
    HANDLE m_hMutexHandle;
    CRITICAL_SECTION *m_pCritSec;
    CMclMutex *m_pcMutex;
    CMclCritSec *m_pcCritSec;

public:
    // constructors...
    CMclAutoLock( HANDLE hMutexHandle);    
    CMclAutoLock( CMclMutex & rCMclMutex);
    CMclAutoLock( CRITICAL_SECTION * pCritSec);
    CMclAutoLock( CMclCritSec & rCMclCritSec);

    // destructor...
    ~CMclAutoLock(void);
};

#endif


