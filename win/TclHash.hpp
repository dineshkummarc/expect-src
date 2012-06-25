/*
 ------------------------------------------------------------------------------
 * TclHash.hpp --
 *
 *   Tcl's hash table done as a template.
 *
 * Copyright (c) 1999-2001 TomaSoft Engineering
 *
 * See the file "license.txt" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * RCS: @(#) $Id: TclHash.hpp,v 1.1.2.4 2002/08/30 02:49:52 davygrvy Exp $
 ------------------------------------------------------------------------------
 */

#ifndef INC_tclhash_hpp__
#define INC_tclhash_hpp__

#ifndef INC_config_hpp__
#   include "config.hpp"
#endif

#include "tcl.h"

CPPTCL_BEGIN_NAMESPACE(Tcl)

template <class T, int keytype = TCL_STRING_KEYS>
    class Hash
{
public:
    Hash () { Tcl_InitHashTable(&HashTbl, keytype); }
    ~Hash () { Tcl_DeleteHashTable(&HashTbl); }
    Tcl_Obj *Stats ();
    int Add (const void *key, T result);
    int Find (const void *key, T *result);
    int Extract (const void *key, T *result);
    int Delete (const void *key);
    int Top (T *result, const void **key = 0L);
    int Next (T *result, const void **key = 0L);

protected:
    Tcl_HashSearch HashSrch;
    Tcl_HashTable HashTbl;
};

template <class T, int keytype>
    Tcl_Obj *Hash<T, keytype>::Stats ()
{
    const char *stats;
    Tcl_Obj *oStats;

    stats = Tcl_HashStats(&HashTbl);
    oStats = Tcl_NewStringObj(stats, -1);
    ckfree((char *)stats);
    return oStats;
}

template <class T, int keytype>
    int Hash<T, keytype>::Add (const void *key, T result)
{
    int created;
    Tcl_HashEntry *entryPtr;

    entryPtr = Tcl_CreateHashEntry(&HashTbl, static_cast<const char *>(key),
	&created);

    if (!created) {
	return TCL_ERROR;
    }
    Tcl_SetHashValue(entryPtr, result);
    return TCL_OK;
}

template <class T, int keytype>
    int Hash<T, keytype>::Find (const void *key, T *result)
{
    Tcl_HashEntry *entryPtr;

    entryPtr = Tcl_FindHashEntry(&HashTbl, static_cast<const char *>(key));
    if (entryPtr == 0L) {
	return TCL_ERROR;
    }
    if (result != 0L) {
	*result = reinterpret_cast<T>(Tcl_GetHashValue(entryPtr));
    }
    return TCL_OK;
}

template <class T, int keytype>
    int Hash<T, keytype>::Delete (const void *key)
{
    Tcl_HashEntry *entryPtr;

    entryPtr = Tcl_FindHashEntry(&HashTbl, static_cast<const char *>(key));
    if (entryPtr == 0L) {
	return TCL_ERROR;
    }
    Tcl_DeleteHashEntry(entryPtr);
    return TCL_OK;
}

template <class T, int keytype>
    int Hash<T, keytype>::Top (T *result, const void **key)
{
    Tcl_HashEntry *entryPtr;

    entryPtr = Tcl_FirstHashEntry(&HashTbl, &HashSrch);
    if (entryPtr == 0L) {
	return TCL_ERROR;
    }
    if (result != 0L) {
	*result = reinterpret_cast<T>(Tcl_GetHashValue(entryPtr));
    }
    if (key != 0L) {
	*key = Tcl_GetHashKey(&HashTbl, entryPtr);
    }
    return TCL_OK;
}

template <class T, int keytype>
    int Hash<T, keytype>::Next (T *result, const void **key)
{
    Tcl_HashEntry *entryPtr;

    entryPtr = Tcl_NextHashEntry(&HashSrch);
    if (entryPtr == 0L) {
	return TCL_ERROR;
    }
    if (result != 0L) {
	*result = reinterpret_cast<T>(Tcl_GetHashValue(entryPtr));
    }
    if (key != 0L) {
	*key = Tcl_GetHashKey(&HashTbl, entryPtr);
    }
    return TCL_OK;
}

template <class T, int keytype>
    int Hash<T, keytype>::Extract (const void *key, T *result)
{
    Tcl_HashEntry *entryPtr;

    entryPtr = Tcl_FindHashEntry(&HashTbl, static_cast<const char *>(key));
    if (entryPtr == 0L) {
	return TCL_ERROR;
    }
    if (result != 0L) {
	*result = reinterpret_cast<T>(Tcl_GetHashValue(entryPtr));
    }
    Tcl_DeleteHashEntry(entryPtr);
    return TCL_OK;
}

CPPTCL_END_NAMESPACE

#endif	// #ifndef INC_tclhash_hpp__

