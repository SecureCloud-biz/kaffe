/*
 * i386/win32/cygnus/md.h
 * Windows'95 (Cygnus GNU C) i386 configuration information.
 *
 * Copyright (c) 1996, 1997
 *	Transvirtual Technologies, Inc.  All rights reserved.
 *
 * See the file "license.terms" for information on usage and redistribution 
 * of this file. 
 */

#ifndef __i386_win32_cygnus_md_h
#define __i386_win32_cygnus_md_h

/**/
/* Thread handling */
/**/
#include "i386/common.h"
#include "i386/threads.h"

/* Redefine the stack pointer offset */
#undef SP_OFFSET
#define	SP_OFFSET	7

#if defined(TRANSLATOR)
#include "jit-md.h"
#endif

#endif
