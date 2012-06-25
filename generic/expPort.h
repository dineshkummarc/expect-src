/*
 * expPort.h --
 *
 *	This header file handles porting issues that occur because
 *	of differences between systems.  It reads in platform specific
 *	portability files.
 *
 * RCS: @(#) $Id: expPort.h,v 1.1.4.1 2002/02/10 02:58:53 davygrvy Exp $
 */

#ifndef _EXPPORT_H__
#define _EXPPORT_H__

#define HAVE_MEMCPY

#ifdef __WIN32__
#   include "../win/expWinPort.h"
#else
#   if defined(MAC_TCL)
#	include "../mac/expPort.h"
#    else
#	include "../unix/expUnixPort.h"
#    endif
#endif

#endif /* _EXPPORT_H__ */
