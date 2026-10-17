/************************************************************************************
FronTier is a set of libraries that implements differnt types of Front Traking algorithms.
Front Tracking is a numerical method for the solution of partial differential equations 
whose solutions have discontinuities.  


Copyright (C) 1999 by The University at Stony Brook. 
 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

******************************************************************************/


/************************************************************************************
FronTier is a set of libraries that implements differnt types of Front Traking algorithms.
Front Tracking is a numerical method for the solution of partial differential equations 
whose solutions have discontinuities.  


Copyright (C) 1999 by The University at Stony Brook. 
 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

******************************************************************************/


/*
*			fnamedebug.h
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*/

#if !defined(_FNAMEDEBUG_H)
#define _FNAMEDEBUG_H
#include <cdecs.h>

#if defined(IGNORE_ERRORS)
#define	Check_return(func,fname)					\
	(void) (func);
#else /* defined(IGNORE_ERRORS) */
#define	Check_return(func,fname)					 \
	{								 \
	    if (! (func))						 \
	    {							         \
	    	(void) printf("ERROR in %s(), %s failed\n",#fname,#func);\
	    	clean_up(ERROR);				         \
	    }							         \
	}
#endif /* defined(IGNORE_ERRORS) */

#if defined(DEBUG_STRING)
#define DEBUG_ENTER(fname)						\
	debug_print(DEBUG_STRING,"Entered %s()\n",#fname);
#define DEBUG_LEAVE(fname)						\
	debug_print(DEBUG_STRING,"Left %s()\n",#fname);
#undef DEBUG
#define	DEBUG		debugging(DEBUG_STRING)
#else /* defined(DEBUG_STRING) */
#define DEBUG_ENTER(fname)
#define DEBUG_LEAVE(fname)
#undef DEBUG
#define	DEBUG		NO
#endif /* defined(DEBUG_STRING) */

#include <navdecs.h>

#endif /* !defined(_FNAMEDEBUG_H) */
