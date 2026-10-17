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
*				array.h
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*/

#if !defined(_ARRAY_H)
#define _ARRAY_H

#include <cdecs.h>

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif

struct _POINTER_Q {
	struct _POINTER_Q *prev, *next;
	POINTER pointer;
};
typedef struct _POINTER_Q POINTER_Q;

#define add_to_pointer_queue(ptr,pq)				\
	generic_add_to_pointer_queue((POINTER)(ptr),(pq))
#define pointer_is_in_array(p,pa)				\
	_pointer_is_in_array((POINTER)(p),(POINTER*)(pa))
#define index_of_pointer_in_array(p,pa,pi)			\
	_index_of_pointer_in_array((POINTER)(p),(POINTER*)(pa),pi)
#define add_to_pointers(p,ppa)					\
	_add_to_pointers((POINTER)(p),(POINTER**)(ppa))
#define delete_from_pointers(p,ppa)				\
	_delete_from_pointers((POINTER)(p),(POINTER**)(ppa))
#define	add_to_ordered_pointers(np,ap,ppa)			\
	_add_to_ordered_pointers((POINTER)(np),(POINTER)(ap),(POINTER**)(ppa))
#define	delete_from_ordered_pointers(p,ppa)			\
	_delete_from_ordered_pointers((POINTER)(p),(POINTER**)(ppa))
#define	delete_from_ordered_pointers_at_location(i,ppa)		\
	_delete_from_ordered_pointers_at_location(i,(POINTER**)(ppa))
#define unique_add_to_pointers(p,ppa)				\
	_unique_add_to_pointers((POINTER)(p),(POINTER**)(ppa))
#define delete_from_pointers_if_present(p,ppa)			\
	_delete_from_pointers_if_present((POINTER)(p),(POINTER**)(ppa))

enum {
	PQ_DEFAULTS		    = 0,
	PQ_BLOCK_SIZE,
	PQ_ALLOC_TYPE,
	PQ_ALLOC_SIZE_FOR_POINTERS,
	N_PQ_OPTS
};


/*
*				size_of_pointers():
*
*	Returns the number of Pointers in a Pointer set:
*/
#define size_of_pointers(pset) 						\
		((pset) ? (((int *)(pset))[-2])/sizeof(POINTER) : 0 )

	/* arrayutils.c EXPORTED Function Prototypes */

IMPORT	POINTER_Q *delete_from_pointer_queue(POINTER_Q*);
IMPORT	POINTER_Q *generic_add_to_pointer_queue(POINTER,POINTER_Q*);
IMPORT	POINTER_Q *head_of_pointer_queue(POINTER_Q*);
IMPORT	POINTER_Q *tail_of_pointer_queue(POINTER_Q*);
IMPORT	boolean	  _add_to_ordered_pointers(POINTER,POINTER,POINTER**);
IMPORT	boolean	  _add_to_pointers(POINTER, POINTER**);
IMPORT	boolean	  _delete_from_ordered_pointers(POINTER,POINTER**);
IMPORT	boolean	  _delete_from_ordered_pointers_at_location(int,POINTER**);
IMPORT	boolean	  _delete_from_pointers(POINTER,POINTER**);
IMPORT	boolean	  _delete_from_pointers_if_present(POINTER,POINTER**);
IMPORT	boolean	  _pointer_is_in_array(POINTER, POINTER*);
IMPORT	boolean	  _index_of_pointer_in_array(POINTER, POINTER*,int*);
IMPORT	boolean	  _unique_add_to_pointers(POINTER, POINTER**);
IMPORT	void	  set_pointer_queue_opts(int,...);

#if defined(c_plusplus) || defined(__cplusplus)
}
#endif


#endif /* !defined(_ARRAY_H) */
