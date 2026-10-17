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
*				Interface Table:
*
*	Records Storage and other information for INTERFACES in use.
*	Records the information as a linked list of Tables, one for
*	each Interface.
*	Storage for the linked list is generated dynamically as
*	needed.   This allows programs to manipulate varying
*	numbers of interfaces without difficulty.
*
*	Within each Table, a further linked list of
*	ChunkSize(intfc)-sized blocks of storage is maintained - this is the
*	actual area where bonds, curves etc are stored for that interface.
*	Individual Chunks should be large enough so that large parts of an
*	interface are stored contiguously.   The use of Chunks makes it easy
*	to deal with a dynamically changing interface without incurring major
*	loss of contiguousness.   The lowest level storage allocator
*	is only called occasionaly - whenever a new Chunk is required.
*	Thus there is little overhead incurred by the low level storage
*	scheme.   The actual dispensing of pieces of a chunk is done
*	by the routine  store().
*
*	The definition of the Table structure is in file table.h
*
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*/

#if !defined(_TABLE_H)
#define _TABLE_H

#include <intfc/int.h>

#define ChunkTop(chunk) (&chunk->_ChunkTop)

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif


struct Chunk {
	struct Chunk 	*next;		/* Pointer to Next Chunk */
	struct Chunk 	*prev;		/* Pointer to Previous Chunk */
	ALIGN		_ChunkTop;
};



struct Table {
	struct Table 	*next; 		/* Pointer to Next Table */
	struct Table 	*prev; 		/* Pointer to Previous Table */
	struct Chunk 	*first_chunk;	/* List of Allocated Chunks */
	struct Chunk	*last_chunk;	
	struct Chunk    *big_chunks;    /* Oversized chunks */
	ALIGN		*top;		/* Low Free location in Top Chunk */
	size_t		_ChunkSize;	/* Size of interface chunks in ALIGNS*/
	size_t		remainder;	/* Space left in Top Chunk in ALIGNS */
	int		num_chunks;	/* Number of allocated chunks */

	INTERFACE 	*interface;	/* The Associated Interface */

			/* Quantities used by component(): */

	RECT_GRID 	rect_grid;	/* Regular Rectangular Grid */
	boolean		fixed_grid;	/* Fixed Grid supplied Previously */
	boolean		new_grid;	/* Flags a change of grid. */
	COMPONENT	max_comp;	/* Largest COMPONENT value */
	COMPONENT	min_comp;	/* Smallest COMPONENT value */
	COMPONENT	ext_comp;	/* Exterior COMPONENT value */
	int		max_pp_indx;	/* Largest pp_index */

			/* Interface-rect_grid structure */
	int		n_crx;
	int		n_segs;
	int             *seg_crx_count;
        int             **seg_crx_lists;
        int             *seg_crx_lists_store;
        CRXING          *crx_store;
	COMPONENT       *components;
	COMPONENT       *cg_comps;

	int     *edge_flag;
        int     n_curve_crx;
        int     *curve_crx_count;
        int     **curve_crx_lists;
        int     *curve_crx_lists_store;
        CRXING  *curve_crx_store;
        int     *curve_crx_type;

	/* ONED */
	COMPONENT       *compon1d;        /* COMPONENT in each Grid Block */
	int             *num_of_points; /* Number POINTS in each Grid zone */
	FT_POINT           ***pts_in_zone; /* points in each zone */
	/* TWOD */
	COMPONENT	**compon2d;	/* COMPONENT in each Grid Block */
	int		**num_of_bonds;	/* Number BONDS in each Grid Block */
	BOND		****bonds;	/* BOND lists in each Grid Block */
	CURVE		****curves;	/* CURVE lists in each Grid Block */

			/* Quantities used by loc_comp_list: */
	
	BOND		**bondstore;	/* Storage for BOND Lists */
	CURVE		**curvestore;	/* Storage for CURVE Lists */

	/* THREED */
	COMPONENT	***compon3d;	/* COMPONENT in each Grid Block */
	int		***num_of_tris;	/* Number TRIS in each Grid Block */
	TRI		*****tris;	/* TRI lists in each Grid Block */
	SURFACE		*****surfaces;	/* SURFACE lists in each Grid Block */
	double		***area;	/* Surface area in each Grid Block */
	double		***vol_frac;	/* Volume frac in each Grid Block */

			/* Quantities used by loc_comp_list: */
	
	TRI		**tristore;	/* Storage for TRI Lists */
	SURFACE		**surfacestore;	/* Storage for SURFACE Lists */

			/* Quantities used by next_point(): */
	FT_POINT		**cur_point;
	NODE		**cur_node;	/* Current NODE for next_point() */
	CURVE		**cur_curve;	/* Current CURVE for next_point() */
	BOND		*cur_bond;	/* Current BOND for next_point() */
	TRI		*cur_tri;
	SURFACE		**cur_surface;
	int		np_do_p_curs;	/* Used by next_point3d() */
	int		np_do_n_curs;	/* Used by next_point3d() */

	/* This part is for elliptic solver */
	RECT_GRID	aug_comp_grid;
	BLK_EDGE	**blk_edge;
	int       	n_c_crx;         
	int       	n_c_segs;
	int		*c_seg_crx_count;
	int		**c_seg_crx_lists;
	CRXING		*c_crx_store;
	COMPONENT	*c_components;
	COMPONENT	*c_cg_comps;  /* On aug_comp_grid  */
	TG_PT     	*c_node_points,*c_cg_npts;
	int c_nnx,c_nny,c_nnz; 	 /* Number of nodes on aug_comp_grid */
	int c_node_offset;       /* same meaning as node_offset, on */
				 /* aug_comp_grid */
        int c_cell_offset;       /* same meaning as cell_offset, on */
				 /* aug_comp_grid */
	int       n_c_node_points;
	int	  n_c_tg_pts;
	int	  n_c_fr_pts;
	int       n_c_reg_nodes;       /* count on aug_comp_grid,
                                      * same meaning as n_node_points, etc.. */
	int       *c_seg_crx_lists_store;
	int       *blk_type;
	/* End part for elliptic solver */

	boolean         _no_topology_lists;/*Disables topology construction*/
};

typedef struct Table Table;



struct Table *table_of_interface(INTERFACE *intfc);




/*	
*	If DEBUG is nonzero, then considerable debugging code is compiled in.
*	If DEBUG is 0, the debugging code is omitted at compilation.
*	A convenient value of DEBUG for controlled debugging is the value
*	debugging("interface")  which is nonzero only if debugging has been
*	requested for "interface".
*/

#if !defined(DEBUG)
#define  DEBUG   0
#endif /* !defined(DEBUG) */

enum {
	EXTERIOR_COMP     = 0,
	MIN_INTERIOR_COMP = 1
};

#if defined(c_plusplus) || defined(__cplusplus)
}
#endif

		/* Macros for Processing Table Data: */

#define	number_of_labeled_components(intfc) \
		((intfc)->table->max_comp - (intfc)->table->min_comp + 1)

#define	max_component(intfc)	((intfc)->table->max_comp)

#define	min_component(intfc)	((intfc)->table->min_comp)

#define	exterior_component(intfc)	((intfc)->table->ext_comp)

#define	no_topology_lists(intfc)	((intfc)->table->_no_topology_lists)

#define is_exterior_comp(comp,intfc)					\
	(equivalent_comps((comp), exterior_component(intfc), (intfc)))

#define is_interior_comp(comp,intfc)					\
	(!equivalent_comps((comp), exterior_component(intfc), (intfc)))

#define	max_pp_index(intfc)	((intfc)->table->max_pp_indx)

#define topological_grid(intfc) ((intfc)->table->rect_grid)  /* NOT A POINTER */
#endif /* !defined(_TABLE_H) */
