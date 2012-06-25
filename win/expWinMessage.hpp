/* ----------------------------------------------------------------------------
 * expWinMessage.hpp --
 *
 *	Declare the Message class.  This is what is passed over the thread-safe
 *	event queue.
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
 * Copyright (c) 2003 ActiveState Corporation
 *	Work by David Gravereaux <davygrvy@pobox.com> for any Win32 OS.
 *	Based on work by Gordon Chaffee <chaffee@bmrc.berkeley.edu>
 *
 * ----------------------------------------------------------------------------
 * URLs:    http://expect.nist.gov/
 *	    http://expect.sf.net/
 *	    http://bmrc.berkeley.edu/people/chaffee/expectnt.html
 * ----------------------------------------------------------------------------
 * RCS: @(#) $Id: expWinMessage.hpp,v 1.1.2.10 2003/08/26 20:46:52 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#ifndef INC_expWinMessage_hpp__
#define INC_expWinMessage_hpp__

#include <windows.h>

class Message
{
public:
    Message();
    ~Message();
    Message(Message &);

    enum Mode {
	TYPE_BLANK,
	TYPE_NORMAL,
	TYPE_ERROR,
	TYPE_WARNING,
	TYPE_INSTREAM,
	TYPE_INRECORD,
	TYPE_ENTERINTERACT,
	TYPE_EXITINTERACT,
	TYPE_SLAVEDONE
    };

    Mode    type;
    SIZE_T  length;
    PBYTE   bytes;
    DWORD   status;
};

#endif