//
// FILE: CMclEvent.h
//
// Copyright (c) 1997 by Aaron Michael Cohen and Mike Woodring
//
/////////////////////////////////////////////////////////////////////////
#ifndef __CMCLEVENT_H__
#define __CMCLEVENT_H__

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

class CMclEvent : public CMclKernel {
public:
    // constructor creates an event object...
    CMclEvent( BOOL bManualReset = FALSE, BOOL bInitialState = FALSE, LPCTSTR lpName = NULL, LPSECURITY_ATTRIBUTES lpEventAttributes = NULL);
    
    // constructor opens an existing named event,
    // you must check the status after using this constructor,
    // it will NOT throw an error exception if the object cannot be opened...
    CMclEvent( LPCTSTR lpName, BOOL bInheritHandle = FALSE, DWORD dwDesiredAccess = EVENT_ALL_ACCESS);

    // operations on event object...
    BOOL Set(void);
    BOOL Reset(void);
    BOOL Pulse(void);
};

#ifdef _MSC_VER
#   pragma warning ( pop )
#endif

#endif



