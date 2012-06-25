//
// FILE: CMclLinkedLists.h
//
// Copyright (c) 1997 by Aaron Michael Cohen and Mike Woodring
//
/////////////////////////////////////////////////////////////////////////
#ifndef __CMCLLINKEDLISTS_H__
#define __CMCLLINKEDLISTS_H__

#include "CMclGlobal.h"
#include "CMclSemaphore.h"
#include "CMclCritSec.h"

// this linked list template class can be instantiated for
// any type which has an accessable copy constructor and
// assignment operator...
template <class T>
class CMclLinkedList {
protected:
    // abstract node class...
    class CMclLinkedListNode {
    public:
        CMclLinkedListNode *m_pNext;
        CMclLinkedListNode *m_pPrev;
        virtual void SetData( T & rData) = 0;
        virtual void GetData( T & rData) = 0;
    };

    // derived class for master and free list nodes...
    class CMclLinkedListSentinelNode : public CMclLinkedListNode {
    public:
        virtual void SetData( T & rData) {
            // this function should never be called...
            // the data type you're storing might need a copy constructor
            CMclThrowError(ERROR_ACCESS_DENIED);
	    rData = rData;
        };
        virtual void GetData( T & rData) {
            // this function should never be called...
            // the data type you're storing might need a copy constructor
            CMclThrowError(ERROR_ACCESS_DENIED);
	    rData = rData;
        };
    };

    // derived class for data nodes...
    class CMclLinkedListDataNode : public CMclLinkedListNode {
    public:
        T m_data;

        CMclLinkedListDataNode( T & rData) : m_data(rData) {
            return;
        };

        virtual void SetData( T & rData) {
            m_data = rData;
        };
        virtual void GetData( T & rData) {
            rData = m_data;
        };
    };

    CMclLinkedListSentinelNode m_MasterNode;
    CMclLinkedListSentinelNode m_FreeNode;
    CMclCritSec m_cCritSec;
    CMclSemaphore m_csNotEmpty;
    DWORD m_dwStatus;

public:
    CMclLinkedList() : m_dwStatus(NO_ERROR), m_csNotEmpty( 0, 0x7FFFFFFF), m_cCritSec() {
        CMCL_CHECK_CREATION_STATUS(m_csNotEmpty.Status(), m_dwStatus);
        // both the master and free nodes point to themselves
        // to indicate that the lists are empty...
        m_MasterNode.m_pNext = m_MasterNode.m_pPrev = &m_MasterNode;
        m_FreeNode.m_pNext = m_FreeNode.m_pPrev = &m_FreeNode;
    };

    // copying and passing by copy are not allowed...
    // this prevents confusion of internal object ownership...
    CMclLinkedList(CMclLinkedList &cpy);

    // assigning one object to another is not allowed,
    // this prevents confusion of internal object ownership...
    CMclLinkedList & operator= (CMclLinkedList & rhs);

    ~CMclLinkedList() {
        Cleanup();
    };

    BOOL PutOnHeadOfList( T & rData) {
        // acquire the list critical section lock...
        CMclAutoLock autoLock(m_cCritSec);

        // get a free list node and attach the data to it...
        CMclLinkedListNode *pNewNode = AllocateListNode(rData);
        if (pNewNode == NULL) {
            // this is a memory allocation failure...
            CMclThrowError(ERROR_OUTOFMEMORY);
            return FALSE;
        }

        // put the node at the head of the list...
        pNewNode->m_pNext = m_MasterNode.m_pNext;
        m_MasterNode.m_pNext->m_pPrev = pNewNode;
        pNewNode->m_pPrev = &m_MasterNode;
        m_MasterNode.m_pNext = pNewNode;

        // add one to the semaphore count which tracks the
        // number of elements in the list...
        m_csNotEmpty.Release(1);

        return TRUE;
    };

    BOOL PutOnTailOfList( T & rData) {
        // acquire the list critical section lock...
        CMclAutoLock autoLock(m_cCritSec);

        // get a free list node and attach the data to it...
        CMclLinkedListNode *pNewNode = AllocateListNode(rData);
        if (pNewNode == NULL) {
            // this is a memory allocation failure...
            CMclThrowError(ERROR_OUTOFMEMORY);
            return FALSE;
        }

        // put the node at the tail of the list...        
        m_MasterNode.m_pPrev->m_pNext = pNewNode;
        pNewNode->m_pPrev = m_MasterNode.m_pPrev;
        m_MasterNode.m_pPrev = pNewNode;
        pNewNode->m_pNext = &m_MasterNode;

        // add one to the semaphore count which tracks the
        // number of elements in the list...
        m_csNotEmpty.Release(1);

        return TRUE;
    };
    
    BOOL GetFromHeadOfList( T & rData, DWORD dwTimeout, CMclEvent *pInterrupt = NULL) {
        // wait until there is an element on the list or the
        // interrupt event is signaled...
        if (pInterrupt) {
            m_dwStatus = pInterrupt->WaitForTwo( m_csNotEmpty, FALSE, dwTimeout);
            if (!CMclWaitSucceeded(m_dwStatus, 2))
                return FALSE;
            if (CMclWaitSucceededIndex(m_dwStatus) == 0)
                return FALSE;
        }
        else {
            m_dwStatus = m_csNotEmpty.Wait(dwTimeout);
            if (!CMclWaitSucceeded(m_dwStatus, 1))
                return FALSE;
        }

        // acquire the list critical section lock...
        CMclAutoLock autoLock(m_cCritSec);

        // take the node off the head of the list...
        CMclLinkedListNode *pNode = m_MasterNode.m_pNext;        
        m_MasterNode.m_pNext = pNode->m_pNext;
        pNode->m_pNext->m_pPrev = &m_MasterNode;

        // copy the data from the list node...
        pNode->GetData(rData);

        // add the list node to the free list...
        AddToFreeList(pNode);

        // all done...
        return TRUE;
    };

    BOOL GetFromTailOfList( T & rData, DWORD dwTimeout, CMclEvent *pInterrupt = NULL) {
        // wait until there is an element on the list or the
        // interrupt event is signaled...
        if (pInterrupt) {
            m_dwStatus = pInterrupt->WaitForTwo( m_csNotEmpty, FALSE, dwTimeout);
            if (!CMclWaitSucceeded(m_dwStatus, 2))
                return FALSE;
            if (CMclWaitSucceededIndex(m_dwStatus) == 0)
                return FALSE;
        }
        else {
            m_dwStatus = m_csNotEmpty.Wait(dwTimeout);
            if (!CMclWaitSucceeded(m_dwStatus, 1))
                return FALSE;
        }

        // acquire the list critical section lock...
        CMclAutoLock autoLock(m_cCritSec);

        // take the node off the tail of the list...
        CMclLinkedListNode *pNode = m_MasterNode.m_pPrev;        
        m_MasterNode.m_pPrev = pNode->m_pPrev;
        pNode->m_pPrev->m_pNext = &m_MasterNode;

        // copy the data from the list node...
        pNode->GetData(rData);

        // add the list node to the free list...
        AddToFreeList(pNode);

        // all done...
        return TRUE;
    };

    BOOL Empty (void) {
	m_dwStatus = m_csNotEmpty.Wait(0);
        if (!CMclWaitSucceeded(m_dwStatus, 1)) {
	    return TRUE;
	} else {
	    // add one to the semaphore count because we just took
	    // one away.
	    m_csNotEmpty.Release(1);
	    return FALSE;
	}
    }

    void RemoveIf ( T & rData1 ) {
	CMclLinkedListNode *pNode;
	T rData2;

	// acquire the list critical section lock...
        CMclAutoLock autoLock(m_cCritSec);

	// Get top node.
        pNode = m_MasterNode.m_pNext;
        while (pNode != &m_MasterNode) {
            CMclLinkedListNode *pOldNode = pNode;
	    // save next.
            pNode = pNode->m_pNext;
	    pOldNode->GetData(rData2);
            if (rData1 == rData2) {
		// unsplice.
		pOldNode->m_pPrev->m_pNext = pOldNode->m_pNext;
		pOldNode->m_pNext->m_pPrev = pOldNode->m_pPrev;
		// add the list node to the free list...
		AddToFreeList(pOldNode);
		// decrease the count by one.
		m_csNotEmpty.Wait(0);
	    }
        }
    }

    DWORD Status(void) {
        return m_dwStatus;
    };

protected:
    void AddToFreeList(CMclLinkedListNode *pFreeNode) {
        // attach node to the end of the free list...
        m_FreeNode.m_pPrev->m_pNext = pFreeNode;
        pFreeNode->m_pPrev = m_FreeNode.m_pPrev;
        m_FreeNode.m_pPrev = pFreeNode;
        pFreeNode->m_pNext = &m_FreeNode;
    };

    CMclLinkedListNode *AllocateListNode( T & rData) {
        // grab a node off the free list, or create
        // a new one if none are available...
        CMclLinkedListNode *pNode = m_FreeNode.m_pNext;
        if (pNode != &m_FreeNode) {
            pNode->m_pPrev->m_pNext = pNode->m_pNext;
            pNode->m_pNext->m_pPrev = pNode->m_pPrev;
            pNode->m_pPrev = pNode;
            pNode->m_pNext = pNode;
            pNode->SetData(rData);
        }
        else {
            pNode = new CMclLinkedListDataNode(rData);
        }
        return pNode;
    };

    void Cleanup(void) {
        // delete all of the list nodes on the master list...
        CMclLinkedListNode *pNode = m_MasterNode.m_pNext;
        while (pNode != &m_MasterNode) {
            CMclLinkedListNode *pOldNode = pNode;
            pNode = pNode->m_pNext;
            delete pOldNode;
        }

        // delete all of the list nodes on the free list...
        pNode = m_FreeNode.m_pNext;
        while (pNode != &m_FreeNode) {
            CMclLinkedListNode *pOldNode = pNode;
            pNode = pNode->m_pNext;
            delete pOldNode;
        }
    };
};

#ifdef _MSC_VER
#   pragma warning ( push )
    // Yes, CMclLinkedList<T> declares a copy constructor but only to cause a
    // compiler error should it ever be accessed.  Don't warn us about
    // this oddity.
#   pragma warning ( disable : 4511 )
    // Ditto for the assignment operator.
#   pragma warning ( disable : 4512 )
#endif

template <class T>
class CMclQueue : protected CMclLinkedList<T> {
public:
    CMclQueue(){};

    // copying and passing by copy are not allowed...
    // this prevents confusion of internal object ownership...
    CMclQueue(CMclQueue &cpy);

    // assigning one object to another is not allowed,
    // this prevents confusion of internal object ownership...
    CMclQueue & operator= (CMclQueue & rhs);

    virtual BOOL Put( T & rData) {
        return PutOnTailOfList(rData);
    };

    virtual BOOL Get( T & rData, DWORD dwTimeout = INFINITE, CMclEvent *pInterrupt = NULL) {
        return GetFromHeadOfList( rData, dwTimeout, pInterrupt);
    };

    DWORD Status(void) {
        return CMclLinkedList<T>::Status();
    };
};

template <class T>
class CMclStack : protected CMclLinkedList<T> {
public:
    CMclStack() {};

    // copying and passing by copy are not allowed...
    // this prevents confusion of internal object ownership...
    CMclStack(CMclStack &cpy);

    // assigning one object to another is not allowed,
    // this prevents confusion of internal object ownership...
    CMclStack & operator= (CMclStack & rhs);

    virtual BOOL Push( T & rData) {
        return PutOnHeadOfList(rData);
    };

    virtual BOOL Pop( T & rData, DWORD dwTimeout = INFINITE, CMclEvent *pInterrupt = NULL) {
        return GetFromHeadOfList( rData, dwTimeout, pInterrupt);
    };

    DWORD Status(void) {
        return CMclLinkedList<T>::Status();
    };
};

#ifdef _MSC_VER
#   pragma warning ( pop )
#endif

#endif

