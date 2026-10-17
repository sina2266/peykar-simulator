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
*			arrayutils.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*
*	This file contains routines for manipulating dynamic arrays.
*
*	The code uses a peculiar data structure called
*	a set to represent dynamically growing and shrinking
*	sets of pointers.   These sets of pointers are
*	manipulated using the routines  add_to_pointers()  and  
*	delete_from_pointers(), which in turn use a lower-level
*	allocator called expand_set().
*
*	A set consists of an ordered pair of integers together
*	with an allocated block of storage.  The first integer
*	contains the number (in bytes) of storage currently
*	used in the block's storage,  while the second integer
*	contains the block's total length (in bytes).  These
*	three objects are stored contiguously in memory with the
*	two integers immediately preceeding the blocks storage
*	as indicated below.
*
*		{num[0]}{num[1]}{block of storage}
*
*	num[0] = number of bytes used in block
*	num[1] = total number of bytes allocated for block.
*
*			POINTER_Q lists
*
*	A POINTER_Q list is a doubly linked list where each element contains
*	an address of a data structure.  The functions
*
*	add_to_pointer_queue()
*	delete_from_pointer_queue()
*
*	are used to add and remove elements from the pointer list.  A NULL
*	argument to add_to_pointer_queue() initiates a new list. The
*	main benefit to be obtained by the use of these objects is that
*	they allow the blocking of storage allocation,  which can substantially
*	improve the execution time required to manipulate these lists.
*
*	The following functions are provided to query list imformation:
*
*	head_of_pointer_queue() returns the first element in the queue,
*	tail_of_pointer_queue() returns the last element in the queue,
*	queue_length()	returns the number of elements in the queue.
*
*	The following options to control the queue functions can be set
*
*	PQ_DEFAULTS		Reset all options to their default values
*	PQ_BLOCK_SIZE		number of elements contained in a storage block
*	PQ_ALLOC_TYPE		storage allocator type
*	PQ_ALLOC_SIZE_FOR_POINTERS	size of data structure of pointer
*
*	The current default values are
*
*	PQ_BLOCK_SIZE			20
*	PQ_ALLOC_TYPE			"vmalloc"
*	PQ_ALLOC_SIZE_FOR_POINTERS	0
*
*
*	If PQ_ALLOC_SIZE_FOR_POINTERS is positive and the pointer argument
*	is NULL then instead of adding the argument pointer to the queue,
*	a pointer to a block of this size is added to the queue.  This storage
*	is taken from an internally maintained list of storage blocks.
*
*	The PQ_ALLOC_TYPE option controls the type of storage allocator
*	setting the option to "vmalloc" will use the Vmalloc allocators
*	while using the value "store" will use the current interface
*	storage allocator.  If "vmalloc" is used,  then all storage allocated
*	to the queue will be freed if the queue becomes empty. Note the if
*	"store" is used,  the storage allocated for the pointer queue will
*	be freed when the interface is deleted,  and not when the queue
*	is empty.
*
*	These options are set by calls the the function set_pointer_queue_opts()
*	Options must be set before a queue is initiated.  Once initiated
*	any change to the queue options will only affect subsequent queues.
*	Set_pointer_queue_opts() uses an XVIEW model to set options.  It
*	is a variable argument function with options of the form (flag, value).
*	For example
*	
*	set_pointer_queue_opts(PQ_DEFAULTS,0);
*	
*	restores all default values.  While
*
*	set_pointer_queue_opts(PQ_BLOCK_SIZE,100,PQ_ALLOC_TYPE,"store",0);
*
*	sets the block size to 100 and allocation type to store.
*
*	IMPORTANT:  all calls to set_pointer_queue_opts() must terminate
*	with a final argument of NULL.  Leaving this option off can
*	cause unexpected effects.
*/

#include <stdarg.h>
#include <intfc/iloc.h>

enum _PQ_ALLOCATOR {
	USE_STORE_FOR_ALLOC   = 1,
	USE_VMALLOC_FOR_ALLOC,
	DEFAULT_ALLOC	      = USE_VMALLOC_FOR_ALLOC
};
typedef enum _PQ_ALLOCATOR PQ_ALLOCATOR;

struct _PQ_HEADER {
	POINTER_Q *head, *tail;	/* Head and Tail of linked list */
	POINTER_Q *pq_str; /* Linked list of POINTER_Q storage blocks */
	POINTER_Q *p2str;  /* Linked list of storage blocks for pointer */
	int pq_indx; /* Index of next free block in POINTER_Q storage list */
	int p2_indx; /* Index of next free block in pointer storage list */
	int n_queue; /* Number of elements in the list */
	PQ_ALLOCATOR alloc_type; /* Storage allocation type */
	size_t p2sz; /* Size of structure to which pointer points */
	int blk_len; /* Block length for pq_str and p2str */
};
typedef struct _PQ_HEADER PQ_HEADER;

struct _PTR_LIST {
	POINTER_Q P_q;
	PQ_HEADER *pq_header; /* Private part of POINTER_Q structure */
};
typedef struct _PTR_LIST PTR_LIST;

	/* LOCAL Function Declarations */
LOCAL	POINTER_Q	*new_pointer_queue(POINTER);
LOCAL	POINTER	zero_array(int);
LOCAL	boolean	expand_set(POINTER*,int);
LOCAL	void	alloc_next_pointer_queue_storage_block(PQ_HEADER *);
LOCAL	void	alloc_next_pointer_2_storage_block(PQ_HEADER *);

#define   end_of_set(array)  ((int *)array)[-2]

/*
*				_add_to_pointers():
*
*	Adds a new pointer to the end of a dynamically varying array of 
*	pointers. Note that the second argument is a pointer to the pointer 
*	array. Returns FUNCTION_SUCCEEDED if successful, or FUNCTION_FAILED
*	on failure.
*/

EXPORT  boolean _add_to_pointers(
	POINTER	newp,
	POINTER	**parray)
{
	if (expand_set((POINTER *)parray,2*sizeof(POINTER)) == FUNCTION_FAILED)
	    return FUNCTION_FAILED;
	(*parray)[end_of_set(*parray)/sizeof(POINTER)] = newp;
	end_of_set(*parray) += sizeof(POINTER);
	return FUNCTION_SUCCEEDED;
}		/*end _add_to_pointers*/

/*
*			_unique_add_to_pointers():
*
*	Adds a new pointer to the end of a dynamically varying array of 
*	pointers if the pointer is not already present. Note that the second
*	argument is a pointer to the pointer array. Returns 
*	FUNCTION_SUCCEEDED if successful, or FUNCTION_FAILED on failure.
*/

EXPORT  boolean _unique_add_to_pointers(
	POINTER	newp,
	POINTER	**parray)
{
	if (parray && pointer_is_in_array(newp,*parray))
	    return FUNCTION_SUCCEEDED;
	return add_to_pointers(newp,parray);
}		/*end _unique_add_to_pointers*/

/*
*			_add_to_ordered_pointers():
*
*	Adds a new pointer at a location following the first occurrence of
*	a specified pointer within a dynamically varying array  of pointers. 
*	This makes most sense for arrays in which there are no repeated
*	entries.  
*	Note that the third argument is a pointer to the pointer array. 
*	Returns FUNCTION_SUCCEEDED if successful, or FUNCTION_FAILED on failure.
*
*	NOTE:  If antep is NULL newp is added at the end of the array.
*/

EXPORT  boolean _add_to_ordered_pointers(
	POINTER		newp,
	POINTER		antep,
	POINTER		**parray)
{
	int	j, k;

	if (expand_set((POINTER *)parray,2*sizeof(POINTER)) == FUNCTION_FAILED)
	    return FUNCTION_FAILED;
	if (antep == NULL)
	    j = (int)size_of_pointers(*parray)-1;
	else
	{
	    if (!index_of_pointer_in_array(antep,*parray,&j))
	    	return FUNCTION_FAILED;
	    for (k = (int)size_of_pointers(*parray); k > j; --k)
	        (*parray)[k+1] = (*parray)[k];
	}
	(*parray)[j+1] = newp;
	end_of_set(*parray) += sizeof(POINTER);
	return FUNCTION_SUCCEEDED;
}		/*end _add_to_ordered_pointers*/

/*
*				_delete_from_pointers():
*
*	Deletes the first instance of a pointer from a dynamic set of pointers.
*	Note that the second argument is a pointer to the array	of pointers.
*	Returns YES if successful, and NO otherwise.
*/

EXPORT boolean _delete_from_pointers(
	POINTER		p,
	POINTER		**parray)
{
	POINTER		*q;

	if (*parray == NULL)
	    return FUNCTION_FAILED;
	for (q = *parray; *q; ++q)
	    if (*q == p)
		break;
	if (*q == NULL) /* Pointer p not in array */
	    return FUNCTION_FAILED;

	*q = (*parray)[end_of_set(*parray)/sizeof(POINTER) - 1];
	(*parray)[end_of_set(*parray)/sizeof(POINTER)-1] = NULL;
	end_of_set(*parray) -= sizeof(POINTER);
	if (end_of_set(*parray)==0)
	    *parray = NULL;
	return FUNCTION_SUCCEEDED;
}		/*end _delete_from_pointers*/

/*
*		_delete_from_pointers_if_present():
*
*	Deletes the first instance of a pointer from a dynamic set of pointers
*	if that pointer is in the array. Note that the second argument is a
*	pointer to the array of pointers.
*	Returns YES if successful, and NO otherwise.
*/

EXPORT boolean _delete_from_pointers_if_present(
	POINTER		p,
	POINTER		**parray)
{
	if (parray && !pointer_is_in_array(p,*parray))
	    return FUNCTION_SUCCEEDED;
	return delete_from_pointers(p,parray);
}		/*end _delete_from_pointers_if_present*/

/*
*			_delete_from_ordered_pointers():
*
*	Deletes all instances of a given pointer from a dynamic set 
*	of ordered pointers.  Note that the second argument is
*	a pointer to the array	of ordered pointers.
*	Returns YES if successful, and NO otherwise.
*/

EXPORT boolean _delete_from_ordered_pointers(
	POINTER		p,
	POINTER		**parray)
{
	POINTER	*q, *array;
	int     j;

	if ((parray == NULL) || (*parray == NULL))
	{
	    (void) printf("WARNING in delete_from_ordered_pointers(), "
			  "parray is NULL\n");
	    return NO;
	}
	array = *parray;
	if (!index_of_pointer_in_array(p,array,&j))
	{
	    (void) printf("WARNING in delete_from_ordered_pointers(), "
			  "p not found in parray\n");
	    return NO;
	}
	for (q = array+j; *q; ++q)
	    *q = *(q+1);

	end_of_set(array) -= sizeof(POINTER);
	if (end_of_set(array)==0)
	    *parray = NULL;

	return YES;
}		/*end _delete_from_ordered_pointers*/


/*
*		_delete_from_ordered_pointers_at_location():
*
*	Deletes the pointer at a given index in a dynamic set of
*	ordered pointers. 
*/

EXPORT boolean _delete_from_ordered_pointers_at_location(
	int		i,
	POINTER		**parray)
{
	POINTER		*q;

	if ((*parray == NULL) || (i > size_of_pointers(*parray)))
	    return FUNCTION_FAILED;
	for (q = *parray + i; *q; ++q)
	    *q = *(q + 1);
	end_of_set(*parray) -= sizeof(POINTER);
	if (end_of_set(*parray)==0)
	    *parray = NULL;
	return FUNCTION_SUCCEEDED;
}		/*end _delete_from_ordered_pointers_at_location*/

/*
*			_pointer_is_in_array():
*
*	Returns YES if the pointer p is in the pointer array parray.
*	Returns NO otherwise.
*/

EXPORT  boolean _pointer_is_in_array(
	POINTER	p,
	POINTER	*parray)
{
	POINTER		*q;

	if (parray == NULL)
	    return NO;
	for (q = parray; *q; ++q)
	    if (*q == p)
		return YES;
	return NO;
}		/*end _pointer_is_in_array*/

/*
*			_index_of_pointer_in_array():
*
*	Returns YES if the pointer p is in the pointer array parray and
*	returns the index of p via the interface *pi.
*	Returns NO otherwise.
*/

EXPORT  boolean _index_of_pointer_in_array(
	POINTER	p,
	POINTER	*parray,
	int     *pi)
{
	POINTER	*q;

	*pi = -1;
	if (parray == NULL)
	    return NO;
	for (q = parray; *q; ++q)
	{
	    if (*q == p)
	    {
		*pi = (int)(q - parray);
		return YES;
	    }
	}
	return NO;
}		/*end _index_of_pointer_in_array*/


/*
*				zero_array():
*
*	Allocates space for an array of length  size  bytes from the
*	current interface storage, and for an additional two integers
*	to precede it.   The whole vector is zeroed and the first
*	integer set to zero, the second to the number of bytes
*	that can be stored in the array.
*
*	Returns a POINTER to the array part, or NULL if no space.
*/


LOCAL POINTER zero_array(
	int		size)
{
	POINTER		start;
	int		*num;
	int		i;

	if ((start = store(size+2*sizeof(int))) == NULL)
	    return NULL;

	num = (int *)start;
	*num++ = 0;
	*num++ = size;
	start = (POINTER)num;
	for (i = 0; i < size; ++i)
	    ((char *)start)[i] = 0;
	return start;
}		/*end zero_array*/

/*
*				expand_set():
*
*	Expands the array pointed to by *p so as to allow it to
*	accommodate  size  more bytes.   If there was sufficient
*	space in the array, no actual expansion is needed.
*
*	Returns 1 if successful, and 0 otherwise.
*/

LOCAL boolean expand_set(
	POINTER		*p,
	int		size)
{
	int	   *oldnum, *newnum;
	int	   i;
	POINTER	   old_array,new_array;
	static const int  SMALL_ARRAY = 20; /* Increment Size for Byte Arrays */
	static int empty[2] = {0,0};

	if (*p == NULL)  
	    oldnum = empty;
	else
	{
	    oldnum = (int *)(*p);
	    oldnum -= 2;
	}
	if (oldnum[0]+size <= oldnum[1])
	    return FUNCTION_SUCCEEDED;

	old_array = *p;
	if ((new_array = zero_array(oldnum[1]+SMALL_ARRAY)) == NULL)
	    return FUNCTION_FAILED;
	*p = new_array;
	newnum = (int *)new_array;
	newnum -= 2;
	newnum[0] = oldnum[0];
	newnum[1] = oldnum[1] + SMALL_ARRAY;
	for (i=0; i<oldnum[1]; ++i)  
	    ((char *)new_array)[i] = ((char *)old_array)[i];
	return expand_set(p,size);
}		/*end expand_set*/



#define Pointer_queue_header(pq)	 ((PTR_LIST *) pq)->pq_header

#define	Pointer_q_from_pq_str(P)				\
	((POINTER_Q *) (((PTR_LIST *) (P)->pq_str->pointer) + (P)->pq_indx++))
#define P2_from_p2str(P)					\
	((POINTER) (((byte *) (P)->p2str->pointer)+((P)->p2sz*(P)->p2_indx++)))

#define	pq_block_size(size)	(size)*sizeof(PTR_LIST)

#define insert_in_doubly_linked_list(p,q) {				\
	(q)->prev = (p);						\
	(q)->next = (p)->next;						\
	if ((p)->next)							\
	    (p)->next->prev = (q);					\
	(p)->next = (q);						\
}

enum { DEFAULT_N_PQS_IN_BLK = 5 };
LOCAL	int n_pqs_in_blk = DEFAULT_N_PQS_IN_BLK;
LOCAL	PQ_ALLOCATOR pq_alloc_type = DEFAULT_ALLOC;
LOCAL	size_t p2sz = 0;


EXPORT	POINTER_Q *generic_add_to_pointer_queue(
	POINTER	  ptr,
	POINTER_Q *pq)
{
	PQ_HEADER	*pq_header;
	POINTER_Q	*new_pq;

	if (pq == NULL)
	    return new_pointer_queue(ptr);

	pq_header = Pointer_queue_header(pq);
	if (pq_header == NULL)
	{
	    (void) printf("WARNING in add_to_pointer_queue(), "
	                  "attempt to add pointer to NULL queue\n"
	                  "most likely this queue has been emptied "
			  "and flushed\n");
	    return NULL;
	}

	if (pq_header->pq_indx >= pq_header->blk_len)
	    alloc_next_pointer_queue_storage_block(pq_header);
	if (pq_header->p2_indx >= pq_header->blk_len)
	    alloc_next_pointer_2_storage_block(pq_header);

	new_pq = Pointer_q_from_pq_str(pq_header);
	Pointer_queue_header(new_pq) = pq_header;
	if (pq_header->p2sz > 0 && ptr == NULL)
	    ptr = P2_from_p2str(pq_header);
	new_pq->prev = pq;
	new_pq->next = pq->next;
	if (pq->next)
	    pq->next->prev = new_pq;
	pq->next = new_pq;
	if (pq == pq_header->tail)
	    pq_header->tail = new_pq;
	++pq_header->n_queue;

	new_pq->pointer = ptr;
	return new_pq;
}		/*end generic_add_to_pointer_queue*/

EXPORT	POINTER_Q *delete_from_pointer_queue(
	POINTER_Q	*pq)
{
	PQ_HEADER	*pq_header;

	if (pq == NULL)
	    return NULL;
	pq_header = Pointer_queue_header(pq);
	if (pq_header == NULL)
	{
	    (void) printf("WARNING in delete_from_pointer_queue(), "
	                  "can't delete element,  probably already deleted\n");
	    return NULL;
	}
	if (pq_header->head == pq)
	    pq_header->head = pq->next;
	if (pq_header->tail == pq)
	    pq_header->tail = pq->prev;
	if (pq->prev)
	    pq->prev->next = pq->next;
	if (pq->next)
	    pq->next->prev = pq->prev;
	pq_header->n_queue--;
	zero_scalar(pq,sizeof(PTR_LIST));
	if (pq_header->head == NULL &&
		pq_header->alloc_type != USE_STORE_FOR_ALLOC)
	{
	    POINTER_Q *pq_str, *p2str, *prev;

	    pq_str = pq_header->pq_str;
	    while (pq_str->next) pq_str = pq_str->next;
	    do
	    {
	    	free(pq_str->pointer);
	    	prev = pq_str->prev;
	    	free(pq_str);
	    	pq_str = prev;
	    }
	    while (pq_str);
	    if ((p2str = pq_header->p2str) != NULL)
	    {
	    	while (p2str->next) p2str = p2str->next;
	    	do
	    	{
	    	    free(p2str->pointer);
	    	    prev = p2str->prev;
	    	    free(p2str);
	    	    p2str = prev;
	    	}
	    	while (p2str);
	    }
	    zero_scalar(pq_header,sizeof(PQ_HEADER));
	    free(pq_header);
	    return NULL;
	}
	return pq_header->head;
}		/*end delete_from_pointer_queue*/

EXPORT	POINTER_Q *head_of_pointer_queue(
	POINTER_Q	*pq)
{
	if (pq == NULL)
	    return NULL;
	if (Pointer_queue_header(pq) == NULL)
	    return NULL;
	return Pointer_queue_header(pq)->head;
}		/*end head_of_pointer_queue*/

EXPORT	POINTER_Q *tail_of_pointer_queue(
	POINTER_Q	*pq)
{
	if (pq == NULL)
	    return NULL;
	if (Pointer_queue_header(pq) == NULL)
	    return NULL;
	return Pointer_queue_header(pq)->tail;
}		/*end tail_of_pointer_queue*/

EXPORT	void	set_pointer_queue_opts(
	int		opt,
			...)
{
	va_list		ap;
	int		i, size;
	char		*alloc_type;

	va_start(ap,opt);

	for (i = 0; (opt != 0) && (i < N_PQ_OPTS); opt = va_arg(ap,int), ++i)
	{
	    switch (opt)
	    {
	    case PQ_BLOCK_SIZE:
	    	size = va_arg(ap,int);
	    	n_pqs_in_blk = size;
	    	break;

	    case PQ_ALLOC_TYPE:
	    	alloc_type = va_arg(ap,char *);
	    	if (alloc_type == NULL)
	    	{
		    pq_alloc_type = DEFAULT_ALLOC;
		    break;
		}
		switch (alloc_type[0])
		{
		case 's':
		case 'S':
		    pq_alloc_type = USE_STORE_FOR_ALLOC;
		    break;
		case 'v':
		case 'V':
		    pq_alloc_type = USE_VMALLOC_FOR_ALLOC;
		    break;
		default:
		    pq_alloc_type = DEFAULT_ALLOC;
		    break;
		}
		break;

	    case PQ_ALLOC_SIZE_FOR_POINTERS:
	    	size = va_arg(ap,int);
	    	p2sz = size;
	    	break;

	    case PQ_DEFAULTS:
	    default:
	    	n_pqs_in_blk = DEFAULT_N_PQS_IN_BLK;
	    	pq_alloc_type = DEFAULT_ALLOC;
	    	p2sz = 0;
	    	break;
	    }
	}
	if (pq_alloc_type == USE_STORE_FOR_ALLOC)
	{
	    INTERFACE	*intfc = current_interface();
	    int		max_blk_len;

	    max_blk_len = (int)(ChunkSize(intfc) / max(p2sz,sizeof(PTR_LIST)));

	    if (n_pqs_in_blk > max_blk_len)
	    {
	    	screen("ERROR in set_pointer_queue_opts(), "
	    	       "block size too large for store\n"
	    	       "Maximum block size = %d\n",max_blk_len);
	    	clean_up(ERROR);
	    }
	}

	va_end(ap);
}		/*end set_pointer_queue_opts*/


LOCAL	POINTER_Q *new_pointer_queue(
	POINTER		ptr)
{
	PQ_HEADER	*pq_header;
	POINTER_Q	*new_pq;

	switch (pq_alloc_type)
	{
	case USE_VMALLOC_FOR_ALLOC:
	    scalar(&pq_header,sizeof(PQ_HEADER));
	    scalar(&pq_header->pq_str,sizeof(POINTER_Q));
	    scalar(&pq_header->pq_str->pointer,pq_block_size(n_pqs_in_blk));
	    if (p2sz != 0)
	    {
	    	scalar(&pq_header->p2str,sizeof(POINTER_Q));
	    	scalar(&pq_header->p2str->pointer,p2sz*n_pqs_in_blk);
	    	pq_header->p2str->next = pq_header->p2str->prev = NULL;
	    }
	    else
	    	pq_header->p2str = NULL;
	    break;
	case USE_STORE_FOR_ALLOC:
	    pq_header = (PQ_HEADER *) store(sizeof(PQ_HEADER));
	    pq_header->pq_str = (POINTER_Q *) store(sizeof(POINTER_Q));
	    pq_header->pq_str->pointer = store(pq_block_size(n_pqs_in_blk));
	    if (p2sz != 0)
	    {
	    	pq_header->p2str = (POINTER_Q *) store(sizeof(POINTER_Q));
		pq_header->p2str->pointer = store(p2sz*n_pqs_in_blk);
		pq_header->p2str->next = pq_header->p2str->prev = NULL;
	    }
	    else
	    	pq_header->p2str = NULL;
	    break;
	}
	pq_header->blk_len = n_pqs_in_blk;
	pq_header->p2sz = p2sz;
	pq_header->alloc_type = pq_alloc_type;
	pq_header->pq_str->prev = pq_header->pq_str->next = NULL;
	pq_header->pq_indx = 0;
	pq_header->p2_indx = 0;
	if (pq_header->p2sz > 0 && ptr == NULL)
	    ptr = P2_from_p2str(pq_header);
	new_pq = Pointer_q_from_pq_str(pq_header);
	new_pq->prev = new_pq->next = NULL;
	Pointer_queue_header(new_pq) = pq_header;
	pq_header->head = pq_header->tail = new_pq;
	pq_header->n_queue = 1;
	new_pq->pointer = ptr;
	return new_pq;
}		/*end new_pointer_queue*/

LOCAL	void alloc_next_pointer_queue_storage_block(
	PQ_HEADER	*pq_header)
{
	POINTER_Q	*new_pq_str;

	switch (pq_header->alloc_type)
	{
	case USE_VMALLOC_FOR_ALLOC:
	    scalar(&new_pq_str,sizeof(POINTER_Q));
	    scalar(&new_pq_str->pointer,pq_block_size(pq_header->blk_len));
	    break;
	case USE_STORE_FOR_ALLOC:
	    new_pq_str = (POINTER_Q *) store(sizeof(POINTER_Q));
	    new_pq_str->pointer =
		(POINTER) store(pq_block_size(pq_header->blk_len));
	    break;
	}

	insert_in_doubly_linked_list(pq_header->pq_str,new_pq_str);

	pq_header->pq_str = new_pq_str;
	pq_header->pq_indx = 0;
}		/*end alloc_next_pointer_queue_storage_block*/

LOCAL	void alloc_next_pointer_2_storage_block(
	PQ_HEADER	*pq_header)
{
	POINTER_Q	*new_p2str;

	if (pq_header->p2sz == 0)
	{
	    pq_header->p2str = NULL;
	    return;
	}
	switch (pq_header->alloc_type)
	{
	case USE_VMALLOC_FOR_ALLOC:
	    scalar(&new_p2str,sizeof(POINTER_Q));
	    scalar(&new_p2str->pointer,pq_header->p2sz*n_pqs_in_blk);
		break;
	case USE_STORE_FOR_ALLOC:
	    new_p2str = (POINTER_Q *) store(sizeof(POINTER_Q));
	    new_p2str->pointer = store(pq_header->p2sz*n_pqs_in_blk);
	    break;
	}

	insert_in_doubly_linked_list(pq_header->p2str,new_p2str);

	pq_header->p2str = new_p2str;
	pq_header->p2_indx = 0;
}		/*end alloc_next_pointer_2_storage_block*/
