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
*				iloc.h:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*			INTERFACE Structures:
*/

#if !defined(_ILOC_H)
#define _ILOC_H

#include <intfc/int.h>

typedef struct {
	int      dim;
	int      num_nodes;    /* number of nodes */
	uint64_t *nodes;       /* addresses of nodes */
	int      num_curves;   /* number of curves */
	uint64_t *curves;      /* addresses of curves */
	uint64_t *ns, *ne;	    /* addresses of start and end nodes */
	int      num_surfaces; /* number of surfaces */
	uint64_t *surfaces;    /* addresses of surfaces */
	int      *num_bonds;   /* number of bonds on curves */
	uint64_t **bonds;      /* addresses of bonds */
	int      *num_psurfs;  /* number of pos surfaces on curves */
	uint64_t **psurfs;     /* addresses of pos surfaces on curves */
	int      *num_nsurfs;  /* number of neg surfaces on curves */
	uint64_t **nsurfs;     /* addresses of neg surfaces on curves */
	int      *num_surfs;   /* number of surfaces on curves */
	uint64_t **surfs;      /* addresses of surfaces on curves */
	int      ***tris;	    /* tri numbers of tris bounding curve */
	int      *num_pcurves; /* number of pos curves on surfaces */
	uint64_t **pcurves;    /* addresses of pos curves on surfaces */
	int      *num_ncurves; /* number of pos curves on surfaces */
	uint64_t **ncurves;    /* addresses of pos curves on surfaces */
} INTERFACE_ADDRESSES;

#include <intfc/ilocprotos.h>

#endif /* !defined(_ILOC_H) */
