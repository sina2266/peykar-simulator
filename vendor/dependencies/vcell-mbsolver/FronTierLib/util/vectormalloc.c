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
*			uni_arraymalloc.c:
*
*		A STORAGE ALLOCATOR FOR MULTIPLE ARRAYS:
*
*
*	Described below is a general purpose storage allocator which
*	has a clean interface to the user.
*
*	NOTE: All vmalloc function return clean storage that has been
*	initialized to all bits 0.
*
*	The higher level allocation routine and calling sequence is:
*
*		POINTER array_T(&A,order,dim1,dim2,...,size,type)
*		POINTER *A;
*		int 	order, dim1,dim2,..., size, type;	
*
*	This returns a pointer to a multiple array of order  order  with
*	respective dimensions  dim1, dim2, ...of objects of size
*	size  bytes.   Here  dim1  is the slowest varying dimension.
*
*	The higher level freeing routines are:  
*			free_these(n,A1,..,An)
*			free_from_T(A),
*	The first of these frees the named objects, the second frees all
*	objects allocated since A was allocated.
*	These routines can be used across arbitrary levels of (even recursive)
*	function calls from the points of allocation.  
*
*	The storage allocation routines all return NULL if enough space
*	cannot be found or an internal error occurs.   Otherwise they
*	return a pointer to the allocated storage.
*
*	In addition, internal errors cause a message to be printed indicating 
*	the source of the error.
*
*	Files using these routines should include the file  vmalloc.h
*	which defines the quantities  CHAR, INT, FLOAT, DOUBLE and provides
*	macro definitions for the convenient functions: 
*				scalar_T(), 
*				vector_T(),   
*				matrix_T(), 
*				tri_array_T(),
*				quad_array_T(),
*				quin_array_T(),
*				sex_array_T(),
*	which allocate respectively (pointer to)scalar, uni_array, bi_array
*	triple, quadruple, .. etc. arrays of objects of given size.
*
*	In addition two sets of macros are provided for
*	allocation and freeing in simpler programs.   These routines are
*	called as above except that the 'type' paramater is not given.
*
*	scalar(), uni_array(), bi_array(), ...,  free_from()
*
*	Examples:
*
*	To allocate an m by n array  A of doubles, the required code
*	is:
*			double **A;
*			A = (double **) bi_array(&A,m,n,DOUBLE);
*
*	The array may later be freed by the call:
*			free(A);
*
*	Similiarly, to allocate an n dimensional vector V of floats, use:
*			double *V;
*			V = (double *) uni_array(&V,n,FLOAT);
*
*	The vector can be freed by the call:
*			free(V);
*
*	Both arrays could have been freed by either of the calls:
*			free_these(2,A,V);
*			free_from(A);
*
*	Disaster will ensue if an attempt is made to free a variable
*	not previously allocated.   Allocated variables should be
*	treated as declared arrays.  Thus
*			A[i][j] = 5.;
*	is a legitimate operation, but  
*			p = A++;
*	will cause later disaster since it modifies A.
*
*	Calls to array_T(), matrix_T(), vector_T(), free,  or the other 
*	free routines may be made in any order whatever.
*	
*	Two functions called alloc_view() and long_alloc_view()  are
*	available to provide a formatted view of the internal allocated
*	and free blocks.   Alloc_view() shows only the start and end
*	of each block, whereas long_alloc_view() shows all of the
*	information available about each block.
*
*	Debugging may be controlled externally by calling the routine:
*				set_vmalloc_debug(level)
*	where level is a non-negative integer.   If level is 0, the default,
*	then the routines in this file generate no messages, except for
*	a warning when ALL memory is exhausted.  If level is greater than
*	or equal to 1,  then the functions array_T() and f_ree test the
*	integrity of the header blocks each time they are called.  If a
*	corrupted header is found, an immediate fatal error is generated.
*	If level is 2 or greater, then the location of the top of allocated
*	storage is printed after each allocation.   If level is 3 or greater,
*	the internal settings of all pointers are shown as they are set.
*	If level is 4, alloc_view() is called before and after every request
*	to allocate or free while if level is 5 then long_alloc_view() is
*	called.   Other debugging output is also generated in these cases.
*
*	Original Author: Oliver McBryan, New York University.
*	Current Author: John W. Grove, Univerisity at Stony Brook
*
*	History:  Original by O. McBryan.  This version included a
*	private free list and multiple storage types.
*
*	Modified version produced by J. Grove,  1994.  This version
*	removed the private free and alloc lists and eliminated separate
*	storage types.  Debugging to test header integrity was also added.
*/

/*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*/

/* LINTLIBRARY */


#include <cdecs.h>
#include <stdarg.h>
#include <sys/types.h>
#include <stdlib.h>
#ifndef __clang__
# include <malloc.h>
#endif

	    /* Debugging Control: */

EXPORT    int  vmalloc_debug_on = 0;

#define   SHOW_ALLOC \
	switch (vmalloc_debug_on)    \
	{                            \
	case 4:                      \
	    alloc_view(stdout);      \
	    break;                   \
	case 5:                      \
	    long_alloc_view(stdout); \
	    break;                   \
	}

#if defined(__cplusplus)
LOCAL const unsigned int MAX_ORDER = 10;
#else /* defined(__cplusplus) */
enum { MAX_ORDER = 10 };
#endif /* defined(__cplusplus) */

LOCAL	int	total_alloc = 0;

LOCAL	ALIGN	EndOfBlockTag;

	    /* Basic Storage Allocator Data Structure: */
struct _ALLOC_HEADER {
	POINTER	             block;
	ALIGN                *aligns;
	ALIGN                *EndOfBlock;
	const char           *name;
	size_t               space;
	unsigned int         size;
	unsigned int         order;
	unsigned int         dim[MAX_ORDER];
	struct _ALLOC_HEADER *prev, *next;
};
typedef struct _ALLOC_HEADER ALLOC_HEADER;

#define	BLOCK_CORRUPTED(h)	( ((h)->block != block_from_header(h)) || \
                                  (*(h)->EndOfBlock != EndOfBlockTag) )

LOCAL const int HEADER_OFFSET = sizeof(ALIGN)*num_aligns(sizeof(ALLOC_HEADER));

#define	block_from_header(h)	((POINTER)((byte*)(h) + HEADER_OFFSET))
#define	header_from_block(b)	((ALLOC_HEADER *)((byte*)(b) - HEADER_OFFSET))

LOCAL	ALLOC_HEADER	*first_header = NULL, *last_header = NULL;

	/* LOCAL function Prototypes */
LOCAL	void	print_alloc_header(ALLOC_HEADER*);
LOCAL	void	set_pointers(int,int*,int,POINTER*,POINTER,int);
LOCAL	void	test_vmalloc_integrity(void);

/*
*				array_T():
*
*	Allocates an array of arbitrary order and dimensions.
*	Usage:
*
*		array_T(name_A,pA,order,dim1,dim2,...,size,type)
*
*	The order must be at least 1.
*	The array is order fold with dimensions dim1, dim2, .. in
*	the various directions, with dim1 varying slowest.
*	
*	Returns a pointer to the allocated array or NULL if there
*	is not sufficient space.   This pointer may be treated
*	exactly like a C multiple array of pointers.
*
*
*	Details:
*
*	From the given argument list the order and dimensions of the
*	array are determined, and from these the total storage
*	required for the array is computed - sum of element storage
*	and the nested pointer arrays needed.
*
*	The storage allocator Alloc() is then called to request a
*	storage block of this size, and of the appropriate type.
*
*	Next the nested arrays of pointers are set recursively
*	by the routine set_pointers().
*
*	Finally the fields  var_ptr, num_pointers, var_name are set
*	in the block header.
*/


/* VARARGS */
EXPORT POINTER array_T(
	const char	*name_A,
	POINTER		*pA,
	int		order,
	 ...)
{
	va_list		ap;
	int		i;
	int		size, num_elements, num_pointers;
	size_t   	space;
	int		dim[MAX_ORDER];	/* Array of Dimension magnitudes */
	size_t          naligns;
	ALLOC_HEADER	*h;

#if defined(_SGI_SOURCE) && !defined(__GNUC__) && !defined(SPEEDSHOP)
	{
	    static boolean first = YES;

	    if (first == YES)
	    {
	    	first = NO;
	    	mallopt(M_FREEHD,1);
	    }
	}
#endif /* defined(_SGI_SOURCE) && !defined(__GNUC__) && !defined(SPEEDSHOP) */

	if (vmalloc_debug_on)
	{
	    test_vmalloc_integrity();
	    if (vmalloc_debug_on>1)
	    {
	    	(void) printf("Entered array_T()\n");
                print_call_stack(" FROM array_T");
	    }
	}
	if ((order < 0) || (order > MAX_ORDER))
	{
	    screen("ERROR in array_T(), ");
	    if (order < 0)
	    	screen("order (%d) negative\n",order);
	    else
	    	screen("order too large\n");
	    clean_up(ERROR);
	    return NULL;
	}

	    /* Get Dimension, size and type arguments: */

	va_start(ap, order);
	for (i = 0; i < order; ++i)
	{
	    dim[i] = va_arg(ap,int);
	    if (dim[i] < 0)
	    {
	    	screen("ERROR in array_T(), negative dim[%d] = %d\n",i,dim[i]);
	    	clean_up(ERROR);
	    	return NULL;
	    }
	}
	size = va_arg(ap,int);
	if (size < 0)
	{
	    screen("ERROR in array_T(), negative size = %d\n",size);
	    clean_up(ERROR);
	    return NULL;
	}
	va_end(ap);

	if (vmalloc_debug_on > 1)
	{
	    (void) printf("array_T(%s,%llu,order=%d,dim=(",
	    	          name_A,ptr2ull(pA),order);
	    if (order > 0)
	    	(void) printf("%d",dim[0]);
 	    for(i = 1; i < order; ++i)
	    	(void) printf(",%d",dim[i]);
	    (void) printf("),size=%d)\n",size);
	}

	    /* Count Number of Elements and Pointers: */

	num_elements = 1;
	num_pointers = 0;
	for(i = 0; i < order-1; ++i)
	{
	    num_elements *= dim[i];
	    num_pointers += num_elements;
	}
	if (order > 0)
	    num_elements *= dim[order-1];


	    /* Allocate Space and assign pointers recursively: */
	    /* NOTE: pointer area is padded so data area will  */
	    /*       be aligned properly 			   */

	space = HEADER_OFFSET +  /* Size of header in bytes */
	        num_aligns(num_pointers*sizeof(POINTER))*sizeof(ALIGN) +
		                 /* Size of storage for row pointers in bytes */
		num_elements*size + /* Size of storage for data in bytes */
		sizeof(ALIGN);      /* Storage for tail block */
	naligns = num_aligns(space);
	space = sizeof(ALIGN)*naligns;
	if ((h = (ALLOC_HEADER*) malloc(space)) == NULL)
	{
	    screen("ERROR in array_T(), "
	           "array_T unable to get enough space: %s\n",strerror(errno));
	    (void) printf("array_T(%s,%llu,order=%d,dim=(",
	    	          name_A,ptr2ull(pA),order);
	    if (order > 0)
	    	(void) printf("%d",dim[0]);
	    for(i = 1; i < order; ++i)
	    	(void) printf(",%d",dim[i]);
	    (void) printf("),size=%d)\n",size);
	    clean_up(ERROR);
	    return NULL;
	}
	/*Ensure storage is initialized to zero*/
	zero_scalar(h,space);

	*pA = block_from_header(h);
	set_pointers(order,dim,dim[0],(POINTER *)*pA,
	             ((byte *)(*pA)) + dim[0]*sizeof(POINTER),size);

	    /* Store Array Variable Pointer and Array Pointer Number: */

	total_alloc += space;
	if (name_A[0] == '&')
	    ++name_A;
	h->block = *pA;
	h->aligns = (ALIGN*)h;
	h->EndOfBlock = h->aligns + naligns-1;
	*h->EndOfBlock = EndOfBlockTag;
	h->name = name_A;
	h->space = space;
	h->size = size;
	h->order = order;
	for (i = 0; i < order; ++i)
	    h->dim[i] = dim[i];
	h->next = NULL;
	if (last_header == NULL)
	{
	    first_header = last_header = h;
	    h->prev = NULL;
	}
	else
	{
	    last_header->next = h;
	    h->prev = last_header;
	    last_header = h;
	}
	
	if (vmalloc_debug_on)
	{
	    test_vmalloc_integrity();
	    if (vmalloc_debug_on > 1)
	    {
	        (void) printf("Allocated array %s, Returning address = %p\n",
	        	      name_A,*pA);
	        SHOW_ALLOC;
	    }
	}
	return *pA;
}		/*end array_T*/


/*
*				set_pointers():
*
*	This recursive routine is basic to the allocation of arbitrary
*	dimensional arrays of objects of size size.   The pointers
*	are set starting at storage location  pointers, and point to
*	locations starting at storage location  location.   A total of
*	 num_ptrs  pointers will be set at the current level.
*
*	At the lowest recursion level, the pointers are set to objects
*	of size  size_object  bytes, while at higher levels the pointers
*	are set to point to other pointers. At the lowest level the 
*	storage location adjusted, if necessary, to provide proper 
*	data alignment.
*/
	
LOCAL void set_pointers(
	int		order,
	int		*dim,
	int		num_ptrs,
	POINTER		*pointers,
	POINTER		location,
	int		size_object)
{
	int	 i,size;
	uint64_t offset;

	if (vmalloc_debug_on>2)
	{
	    (void) printf(
	       "set_pointers(ord=%d dim=(%d,%d) num_po=%d po=%p lo=%p so=%d\n",
	        	  order,dim[0],order<2?0:dim[1],num_ptrs,
	        	  pointers,location,size_object);
	}
	if (order < 2)
	    return;
	else if (order == 2) 
	{
	    size = size_object;

	    	/* guarantee alignment of data */
	    if ((offset = (ptr2ull(location) % sizeof(ALIGN))) != 0)
	    {
	        offset = (offset != 0) ? sizeof(ALIGN) - offset: 0;
	        location = ((byte *) location) + offset;
	        if (vmalloc_debug_on > 2)
	        {
	    	    (void) printf("lo adjusted %llu bytes to %llu "
	    	                  "to guarantee data alignment\n",
	    			  offset,ptr2ull(location));
	        }
	    }
	}
	else size = sizeof(POINTER);

	for (i = 0; i < num_ptrs; ++i) 
	    pointers[i] = ((byte *) location) + i*dim[1]*size;
	if (vmalloc_debug_on > 2)
	{
	    (void) printf("Set %d pointers to:",num_ptrs);
	    for(i = 0; i < num_ptrs; ++i) 
	    	(void) printf("%c%10llu",i%7?' ':'\n',
	        	      ptr2ull(pointers[i]));
	    (void) printf("\n");
	}

	set_pointers(order-1,dim+1,dim[1]*num_ptrs,pointers+num_ptrs,
	    	     ((byte *) location) + dim[1]*num_ptrs*size,size_object);

}		/*end set_pointers*/


EXPORT	void f_ree(
	POINTER	   p,
	const char *name)
{
	ALLOC_HEADER	*h;

	if (p == NULL)
	    return;
	h = header_from_block(p);

	if (vmalloc_debug_on)
	{
	    ALLOC_HEADER *hh;
	    if (vmalloc_debug_on > 1)
	    {
	        (void) printf("Request to free %llu (0x%p) (%s)\n",
			      ptr2ull(p),p,name);
		print_call_stack(" FROM f_ree");
	    }
	    test_vmalloc_integrity();
	    for (hh = first_header; hh != NULL; hh = hh->next)
		if (hh == h)
		    break;
	    if (hh == NULL)
	    {
		screen("ERROR in f_ree(), attempt to free unallocated "
		       "storage\n");
		(void) printf("p = %llu (0x%p) (%s)\n",ptr2ull(p),p,name);
		print_call_stack(" FROM f_ree");
		print_alloc_header(h);
	    	long_alloc_view(stdout);
	    	clean_up(ERROR);
	    }
	}

	total_alloc -= h->space;
	if (h->prev)
	    h->prev->next = h->next;
	else
	    first_header = h->next;
	if (h->next)
	    h->next->prev = h->prev;
	else
	    last_header = h->prev;

	free(h);
	SHOW_ALLOC;
}		/*end f_ree*/


/* VARARGS */
EXPORT void free_these(
	int	n,
	...)
{
	va_list	ap;
	int	i;

	if (vmalloc_debug_on > 1)
	{
	    va_start(ap, n);
	    (void) printf("Request to free_these: ");
	    for(i = 0; i < n; ++i)
	    	(void) printf("%p ",va_arg(ap,POINTER));
	    (void) printf("\n");
	    va_end(ap);
	}

	va_start(ap, n);
	for (i = 0; i < n; ++i)
	    f_ree(va_arg(ap,POINTER),"free_these");
	va_end(ap);

	SHOW_ALLOC;
	return;
}		/*end free_these*/


/*
*				free_from_T():
*
*	Frees all blocks of a specified type that were allocated since
*	the named pointer was allocated.   Returns the number of freed
*	pointers or -1 on error.   It is not necessary that  ptr  be
*	of the specified type.
*/

EXPORT int free_from_T(
	POINTER		ptr)
{
	ALLOC_HEADER	*h;
	int		n = 0;

	if (ptr == NULL)
	    return 0;
	h = header_from_block(ptr);
	for (n = 1; h->next != NULL; ++n)
	    f_ree(h->next->block,"free_from_T");
	f_ree(h->block,"free_from_T h->block");
	return n;
}		/*end free_from_T*/

EXPORT void alloc_view(
	FILE		*file)
{
	ALLOC_HEADER	*h;
	byte		*p;
	const char	*ERR;
	static int	show_alloc_view = YES;

	if (show_alloc_view == NO)
	    return;
	ERR = "\n\nERROR in alloc_view(), corrupted header\n\n";
	(void) fprintf(file,"\n\t\tView of Storage Allocator Blocks:\n");
	(void) fprintf(file,
	               "\nTotal of %d Bytes of Storage in Alloc Lists\n\n",
	               total_alloc);

	(void) fprintf(file,"Alloc List ->\n");
	(void) fprintf(file,"%29s %29s %12s %s\n","From","To","Size","Name");
	for (h = first_header; h != NULL; h = h->next)
	{
	    if (BLOCK_CORRUPTED(h))
	    {
	    	screen(ERR);
	    	if ((file != stdout) && (file != stderr))
	    	    (void) fprintf(file,"%s", ERR);
	    	show_alloc_view = NO;
	    	clean_up(ERROR);
	    }
	    p = (byte*) h->block;
	    (void) fprintf(file,"%12llu (0x%12p) %12llu (0x%12p) %12lu %s\n",
			   ptr2ull(p),p,
	        	   (ptr2ull(p+(h->space-HEADER_OFFSET)))-1,
	        	   (p+(h->space-HEADER_OFFSET))-1,
	        	   h->space-HEADER_OFFSET,h->name);
	}
	(void) fprintf(file,"\n\n");
}		/*end alloc_view*/



EXPORT void long_alloc_view(
	FILE		*file)
{
	ALLOC_HEADER	*h;
	byte		*p;
	const char	*ERR;
	unsigned int	i;
	static int	show_long_alloc_view = YES;

	if (show_long_alloc_view == NO)
	    return;
	ERR = "\n\nERROR in long_alloc_view(), corrupted header\n\n";
	(void) fprintf(file,"\n\t\tView of Storage Allocator Blocks:\n");
	(void) fprintf(file,
	               "\nTotal of %d Bytes of Storage in Alloc Lists\n\n",
	               total_alloc);

	(void) fprintf(file,"Alloc List ->\n");
	(void) fprintf(file,"%29s %29s %29s %12s %s %-6s %s\n",
	               "Header","From","To","Size","Order","Dim","Name");
	for (h = first_header; h != NULL; h = h->next)
	{
	    if (BLOCK_CORRUPTED(h))
	    {
	    	screen(ERR);
	    	if (file != stdout && file != stderr)
	    	    (void) fprintf(file,"%s", ERR);
	    	show_long_alloc_view = NO;
	    	clean_up(ERROR);
	    }
	    p = (byte*) h->block;
	    (void) fprintf(file,"%12llu (0x%12p) %12llu (0x%12p) "
				"%12llu (0x%12p) %12lu %5u ",
	    	          ptr2ull(h),h,
			  ptr2ull(p),p,
	    	          (ptr2ull(p+(h->space-HEADER_OFFSET)))-1,
	    	          (p+(h->space-HEADER_OFFSET))-1,
	    	          h->space-HEADER_OFFSET,h->order);
	    (void) fprintf(file,"(");
	    for (i = 0; i < h->order; ++i)
	    	(void) fprintf(file,"%d%s",h->dim[i],
	    		       (i < h->order-1) ? ", " : ") ");
	    (void) fprintf(file,"%s\n",h->name);
	}
	(void) fprintf(file,"\n\n");
}		/*end long_alloc_view*/


/*
*			get_vmalloc_storage_use():
*
*	Reports on maximum storage requested and available.
*/

EXPORT int get_vmalloc_storage_use(void)
{
	return total_alloc;
}		/*end get_vmalloc_storage_use*/

LOCAL	void	test_vmalloc_integrity(void)
{
	ALLOC_HEADER	*h;
	const char	*ERR;

	ERR = "ERROR in test_vmalloc_integrity(), corrupted header\n";
	for (h = first_header; h != NULL; h = h->next)
	{
	    if (BLOCK_CORRUPTED(h))
	    {
	    	screen(ERR);
		print_alloc_header(h);
	    	long_alloc_view(stdout);
	    	clean_up(ERROR);
	    }
	}
}		/*end test_vmalloc_integrity*/

LOCAL	void	print_alloc_header(
	ALLOC_HEADER *h)
{
	(void) printf("ALLOC_HEADER %llu (0x%p), ",ptr2ull(h),h);
	if (h != NULL)
	{
	    char name[1001];
	    int i, M;
	    (void) printf("prev = 0x%p, next = 0x%p\n",h->prev,h->next);
	    (void) printf("block = %llu (0x%p)\n",
			  ptr2ull(h->block),h->block);
	    (void) printf("EndOfBlock = %llu (0x%p)\n",
			  ptr2ull(h->EndOfBlock),h->EndOfBlock);
	    (void) strncpy(name,h->name,1000);
	    name[1000] = '\0';
	    (void) printf("name = %s\n",name);
	    (void) printf("space = %lu\n",h->space);
	    (void) printf("size = %d\n",h->size);
	    (void) printf("order = %d\n",h->order);
	    (void) printf("dim =");
	    M = min(h->order,MAX_ORDER);
	    for (i = 0; i < M; ++i)
		(void) printf(" %d",h->dim[i]);
	    (void) printf("\n");
	    (void) printf("End ALLOC_HEADER 0x%p\n",h);
	}
	else
	    (void) printf("\nNULL ALLOC_HEADER\n");
}		/*end print_alloc_header*/
