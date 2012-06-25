/* ----------------------------------------------------------------------------
 * expWinInjectorIPC.hpp --
 *
 *	CMclMailbox values saved to a common include file to avoid
 *	differences in the constructor calls on either end of the IPC
 *	connection phase.
 *
 * ----------------------------------------------------------------------------
 *
 * Written by: Don Libes, libes@cme.nist.gov, NIST, 12/3/90
 * 
 * Design and implementation of this program was paid for by U.S. tax
 * dollars.  Therefore it is public domain.  However, the author and NIST
 * would appreciate credit if this program or parts of it are used.
 * 
 * Copyright (c) 2001-2002 Telindustrie, LLC
 * Copyright (c) 2003-2005 ActiveState Corporation
 *	Work by David Gravereaux <davygrvy@pobox.com> for any Win32 OS.
 *	Based on work by Gordon Chaffee <chaffee@bmrc.berkeley.edu>
 *
 * ----------------------------------------------------------------------------
 * URLs:    http://expect.nist.gov/
 *	    http://expect.sf.net/
 *	    http://bmrc.berkeley.edu/people/chaffee/expectnt.html
 * ----------------------------------------------------------------------------
 * RCS: @(#) $Id: expWinInjectorIPC.hpp,v 1.1.2.3 2003/08/26 20:46:52 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#ifndef INC_expWinInjectorIPC_hpp__
#define INC_expWinInjectorIPC_hpp__

enum IPCMsgtype {IRECORD, CTRL_EVENT};

#define IPC_MAXRECORDS 80

typedef struct {
    IPCMsgtype type;
#ifdef IPC_MAXRECORDS
    /*
     * This variant allow for grouped keypress events when writing
     * to the console.  Appears to correct bug 35440.
     */
    DWORD event;     /* This represents irecord length if type == IRECORD */
    INPUT_RECORD irecord[IPC_MAXRECORDS];
#else
    union {
	DWORD event;
	INPUT_RECORD irecord;
    };
#endif
} IPCMsg;

#define IPC_NUMSLOTS 50
#define IPC_SLOTSIZE sizeof(IPCMsg)

#endif
