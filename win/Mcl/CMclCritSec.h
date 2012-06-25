//
// FILE: CMclCritSec.h
//
// Copyright (c) 1997 by Aaron Michael Cohen and Mike Woodring
//
/////////////////////////////////////////////////////////////////////////
#ifndef __CMCLCRITSEC_H__
#define __CMCLCRITSEC_H__

#include "CMclGlobal.h"

class CMclCritSec {
private:
    CRITICAL_SECTION m_CritSec;

public:
    // constructor creates a CRITICAL_SECTION inside
    // the C++ object...
    CMclCritSec(void);
    
    // destructor...
    virtual ~CMclCritSec();

    // enter the critical section...
    void Enter(void);

    // leave the critical section...
    void Leave(void);

    // return a pointer to the internal
    // critical section...
    CRITICAL_SECTION *GetCritSec(void);
};

#endif

