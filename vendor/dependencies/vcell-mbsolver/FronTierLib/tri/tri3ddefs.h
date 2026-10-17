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
*				tri3ddefs.h:
*
*       Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Three dimensional specific tetrazation functions.
*/

#if !defined(_TRI3DDEFS_H)
#define _TRI3DDEFS_H
#if defined(THREED)

#include <tri/trilocaldecs.h>

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif


typedef struct {
	LINEAR_ELEMENT	el;
	double		D,DN[3];
	boolean		coplanar;
} TETRA;

typedef struct {
	TG_PT		*vertex[3];
	Locstate        state[3];
	double		norm[3];
} FACE;

typedef struct {
	TG_PT		*ends[2];
} LINE;

typedef struct {
	FACE 	*faces;
	int 	num_faces;
} HULL;

	/* BLK_BIN--Used to store points, comps and states  */
	/* before making tetrahedrals.			    */

struct _BLK_BIN {
	int num_els;
	TG_PT **el_list;
	COMPONENT *pos_comp, *neg_comp;
	Locstate *pos_st, *neg_st;
};
typedef struct _BLK_BIN BLK_BIN;

struct	_PT_ST {
	TG_PT		*pt;
	Locstate	st;
	double		dist;
};
typedef struct _PT_ST PT_ST;

/* Prototypes */
IMPORT	int	count_num_pcs3d(TRI_GRID*);
IMPORT	int	max_num_3d_lin_els(INTERFACE*);

#if defined(c_plusplus) || defined(__cplusplus)
}
#endif


#endif /* defined(THREED) */
#endif /* !defined(_TRI3DDEFS_H) */
