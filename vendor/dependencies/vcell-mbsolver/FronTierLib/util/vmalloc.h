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


			/* Definitions for Storage Allocators: */
/*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*/

#if !defined(_VMALLOC_H)
#define _VMALLOC_H

#include <cdecs.h>

#define	  CHAR	  sizeof(char)
#define   INT	  sizeof(int)
#define	  FLOAT   sizeof(double)
#define   DOUBLE  sizeof(double)

#define   free(x) f_ree((POINTER)(x),#x)/* Avoids clash with C library free() */

#define	  scalar(a,b)			(void) array_T(#a,(POINTER*)a,1,1,b)

#define CHECK_ALLOC_SIZE

#if defined(CHECK_ALLOC_SIZE)

#define check_array_alloc_size(x,sz,str)				\
  ((sizeof(x) > (sz)) ? printf("WARNING %s alloc in file %s line %d, "	\
                               "sizeof(%s) = %lu > %lu\n",		\
	                       str,__FILE__,__LINE__,#x,		\
			       (size_t)sizeof(x),(size_t)(sz)) :	\
			       0)

#define uni_array(a,b,c)							\
    ((void)check_array_alloc_size(**a,c,"uni_array"),  			\
     (void) array_T(#a,(POINTER*)a,1,b,c))

#define bi_array(a,b,c,d)							\
    ((void)check_array_alloc_size(***a,d,"bi_array"), 			\
     (void) array_T(#a,(POINTER*)a,2,b,c,d))

#define tri_array(a,b,c,d,e)						\
    ((void)check_array_alloc_size(****a,e,"tri_array"),			\
     (void) array_T(#a,(POINTER*)a,3,b,c,d,e))

#define quad_array(a,b,c,d,e,f)						\
    ((void)check_array_alloc_size(*****a,f,"quad_array"),		\
     (void) array_T(#a,(POINTER*)a,4,b,c,d,e,f))

#define quin_array(a,b,c,d,e,f,g)					\
    ((void)check_array_alloc_size(******a,g,"quin_array"),		\
     (void) array_T(#a,(POINTER*)a,5,b,c,d,e,f,g))

#define sex_array(a,b,c,d,e,f,g,h)					\
    ((void)check_array_alloc_size(******a,h,"sex_array"),		\
     (void) array_T(#a,(POINTER*)a,6,b,c,d,e,f,g,h))

#else /* defined(CHECK_ALLOC_SIZE) */ 

#define	  uni_array(a,b,c) 		(void) array_T(#a,(POINTER*)a,1,b,c)
#define   bi_array(a,b,c,d)		(void) array_T(#a,(POINTER*)a,2,b,c,d)
#define	  tri_array(a,b,c,d,e) 		(void) array_T(#a,(POINTER*)a,3,b,c,d,e)
#define	  quad_array(a,b,c,d,e,f)	(void) array_T(#a,(POINTER*)a,4,b,c,d,e,f)
#define	  quin_array(a,b,c,d,e,f,g) 	(void) array_T(#a,(POINTER*)a,5,b,c,d,e,f,g)
#define	  sex_array(a,b,c,d,e,f,g,h) 	(void) array_T(#a,(POINTER*)a,6,b,c,d,e,f,g,h)

#endif /* defined(CHECK_ALLOC_SIZE) */ 

#define   free_from(a)			free_from_T((POINTER)a)



IMPORT int vmalloc_debug_on;

#define  vmalloc_debug set_vmalloc_debug
#define  set_vmalloc_debug(value)  (vmalloc_debug_on = value)


/*	For convenience of complex structures containing allocated arrays: */

#define  set_alloc(str,pointer) 					    \
	((str)->alloc.pointer=YES)
#define  set_no_alloc(str,pointer)					    \
	((str)->alloc.pointer=NO)
#define	 Set_free(str,ptr)						    \
	if ((str)->alloc.ptr) set_free(str,ptr)
#define  set_free(str,pointer)						    \
	(free((str)->pointer),(str)->alloc.pointer=NO)
#define VECTOR(str,var,dim,size) 					    \
	(set_alloc(str,var),uni_array(&((str)->var),dim,size))
#define MATRIX(str,var,dim1,dim2,size) 					    \
	(set_alloc(str,var),bi_array(&((str)->var),dim1,dim2,size))
#define TRI_ARRAY(str,var,dim1,dim2,dim3,size) 				    \
	(set_alloc(str,var),tri_array(&((str)->var),dim1,dim2,dim3,size))
#define	ASSIGN_ARRAY_POINTER(str,var,ptr)				    \
	(set_no_alloc(str,var),(str)->var = ptr)

/*
*		Obsolete Macros for general type storage allocator:
*/

#define	  scalar_T(a,b,t)			array_T(#a,(POINTER*)a,1,1,b)
#define	  vector_T(a,b,c,t) 			array_T(#a,(POINTER*)a,1,b,c)
#define   matrix_T(a,b,c,d,t)			array_T(#a,(POINTER*)a,2,b,c,d)
#define	  tri_array_T(a,b,c,d,e,t) 		array_T(#a,(POINTER*)a,3,b,c,d,e)
#define	  quad_array_T(a,b,c,d,e,f,t) 		array_T(#a,(POINTER*)a,4,b,c,d,e,f)
#define	  quin_array_T(a,b,c,d,e,f,g,t) 	array_T(#a,(POINTER*)a,5,b,c,d,e,f,g)
#define	  sex_array_T(a,b,c,d,e,f,g,h,t)	array_T(#a,(POINTER*)a,6,b,c,d,e,f,g,h)
#define	  stat_scalar(a,b)			(void) array_T(#a,(POINTER*)a,1,1,b)
#define	  stat_vector(a,b,c) 			(void) array_T(#a,(POINTER*)a,1,b,c)
#define   stat_matrix(a,b,c,d)			(void) array_T(#a,(POINTER*)a,2,b,c,d)
#define	  stat_tri_array(a,b,c,d,e) 		(void) array_T(#a,(POINTER*)a,3,b,c,d,e)
#define	  stat_quad_array(a,b,c,d,e,f)		(void) array_T(#a,(POINTER*)a,4,b,c,d,e,f)
#define	  stat_quin_array(a,b,c,d,e,f,g)	(void) array_T(#a,(POINTER*)a,5,b,c,d,e,f,g)
#define	  stat_sex_array(a,b,c,d,e,f,g,h)	(void) array_T(#a,(POINTER*)a,6,b,c,d,e,f,g,h)
#define   stat_free_from(a)			free_from_T((POINTER)a)

#define STAT_VECTOR(str,var,dim,size) \
	(set_alloc(str,var),uni_array(&(str->var),dim,size))
#define STAT_MATRIX(str,var,dim1,dim2,size) \
	(set_alloc(str,var),bi_array(&(str->var),dim1,dim2,size))
#define STAT_TRI_ARRAY(str,var,dim1,dim2,dim3,size) \
	(set_alloc(str,var),tri_array(&(str->var),dim1,dim2,dim3,size))

#endif /* !defined(_VMALLOC_H) */
