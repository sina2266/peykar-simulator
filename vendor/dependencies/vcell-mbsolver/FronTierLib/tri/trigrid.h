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
*			trigrid.h:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	External definitions for the triangulated grid construction.
*/


#if !defined(_TRIGRID_H)
#define _TRIGRID_H

#include <front/fdecs.h>

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif

	/* grid corner directions */

#if defined(TWOD)
enum _GRID_CORNER {
	NORTH_EAST = NORTH | EAST,
	NORTH_WEST = NORTH | WEST,
	SOUTH_EAST = SOUTH | EAST,
	SOUTH_WEST = SOUTH | WEST
};
typedef enum _GRID_CORNER GRID_CORNER;

#endif /* defined(TWOD) */

 /* Side types for a linear element (2d tri, 3d tetra) */

enum _LIN_EL_FACE_TYPE {
#if defined(THREED)
	TETRA_CREATED = 0x0,
	CREATE_TETRA  = 0x1,
#endif /* defined(THREED) */
	F_SIDE = 0x1,	/* side formed by front */
	E_SIDE = 0x2,	/* side formed by mesh block edge */
	I_SIDE = 0x4	/* side interior to mesh block - meets another * tri */
};
typedef enum _LIN_EL_FACE_TYPE LIN_EL_FACE_TYPE;

struct _BLK_CORNER {
	GRID_CORNER sides;
	int         index;
	TG_PT       *nd;
	Locstate    state;
	COMPONENT   comp;
};
typedef struct _BLK_CORNER BLK_CORNER;

struct _BLK_CRX_LIST {
	struct _BLK_CRX_LIST	*prev;
	struct _BLK_CRX_LIST	*next;
	struct _BLK_CRX_LIST	*opp_crxl;
	NODE			*opp_node;
	CRXING			*crx;
	CURVE			*blk_cur;
	ORIENTATION		blk_cur_orient;
	BLK_CORNER		*corner;
	GRID_DIRECTION		side;
	int			*crx_index;
};
typedef struct _BLK_CRX_LIST BLK_CRX_LIST;

typedef struct {
	TG_PT	         *p[MAXD+1];
	Locstate         s[MAXD+1];
	LIN_EL_FACE_TYPE side[MAXD+1];	/*side facing vortex*/
	COMPONENT        comp;
} LINEAR_ELEMENT;

typedef struct {
	TG_PT	         **p;
	Locstate         *s;
	double		 *x;
	int		 dim;
	int		 nc;		/* number of columns */
	int		 nr;		/* number of rows */
	int		 i_order;
	COMPONENT        comp;
} LEAST_SQR_CLUSTER;

typedef struct {
	TG_PT	  *p[(1<<MAXD)];
	COMPONENT comp;
} BILINEAR_ELEMENT;

typedef struct {
	TG_PT	  *p;
	COMPONENT comp[2];
	Locstate s[2];
} POINT_COMP_ST;

typedef union {
	LINEAR_ELEMENT	 *_lin;		/* linear element  list for block */
	BILINEAR_ELEMENT *_bilin;	/* bilinear element list for block */
	POINT_COMP_ST	 *_pcs;		/* point-component-state triplet */
} TG_LIST;

typedef struct {
	int	_num_els;
	TG_LIST	_tg_list;
} BLK_EL0;

#define	set_bilinear_blk_el0(blk_el0)	((blk_el0)->_num_els = 0)
#define	blk_el0_is_bilinear(blk_el0)	((blk_el0)->_num_els == 0)
#define blk_el0_bilinear_el(blk_el0)	((blk_el0)->_tg_list)._bilin

#define blk_el0_linear_els(blk_el0)	((blk_el0)->_tg_list)._lin
#define	num_lin_els_in_blk(blk_el0)	(blk_el0)->_num_els

#define blk_el0_pcs_els(blk_el0)	((blk_el0)->_tg_list._pcs)
#define	num_pcs_els_in_blk(blk_el0)     ((blk_el0)->_num_els)

struct _BLK_NODE {
	struct _BLK_NODE *prev;
	struct _BLK_NODE *next;
	NODE	  *n;
	TG_PT	  *nd;
};
typedef struct _BLK_NODE BLK_NODE;

typedef struct {
	int	 interior_node;
	BLK_NODE *blk_node;
} BLK_EL1;

typedef enum {COMPLETE_TRIANGULATION,ELEMENT_ON_THE_FLY} TRIANGULATION_METHOD;

struct _TRI_GRID {
	/*
	*		Comments on TRI_GRID RECT_GRIDs
	*
	* There are up to four grids associated with the TRI_GRID structure.
	* The comp_grid is a copy of the computational grid of the interface
	* whose triangulated grid is being constructed.  Two other grids
	* may be specified by the user.  However the mapping between grid
	* coordinates on the separate grids generally assumes that the two
	* grids tg_grid and rect_grid are dual to comp_grid in the sense
	* the cell centers of the comp_grid correspond to cell edges of the
	* the other two grids and the origin of the the comp_grid is positioned
	* at the cell center of the tg_grid.  Thus the user should be careful
	* about assumptions made about the index mapping between the two grids.
	* The grid tg_grid and rect_grid are related by a buffer zone
	* offset.  Corresponding points on the two grids tg_grid and
	* rect_grid are related by the formula
	*
	* position on tg_grid(icoords) = position on rect_grid(icoords+lbuf),
	*
	* where the position refers to the position of the lattice points,
	* and lbuf is the lower buffer zone width of tg_grid.
	*
	* The fourth grid is the topological grid of the interface in
	* the field grid_intfc.  It is generated from the rect_grid grid.
	* This grid is currently only used in the three dimensional 
	* front tracking code.  It is a copy of the tri_grid rect_grid.
	*/

	RECT_GRID comp_grid;	/* Computational grid */
	RECT_GRID tg_grid;	/* Unexpanded rect grid for trigrid */
	RECT_GRID rect_grid;	/* Expanded rect grid for trigrid */
	struct _TRI_GRID *old_tri_grid;

	boolean	(*_locate_on_trigrid)(double*,COMPONENT,struct _TRI_GRID*,
				      BILINEAR_ELEMENT**,LINEAR_ELEMENT**lin,
				      LEAST_SQR_CLUSTER**lsq);
	double	(*area)(int*,COMPONENT,struct _TRI_GRID*);

	int	nnx, nny, nnz;	/* Number of nodes on rect grid in direction */
	int	node_offset;	
	int	cell_offset;
	int     offset[8];      /* Offsets for rectangular mesh */

	int	n_nodes;
	int	n_crx;
	int     n_segs;
	/*int	*seg_crx_count; */
	/*int	**seg_crx_lists; */
	/*int	*seg_crx_lists_store; */
	int	 n_lin_els, n_bilin_els, n_pcs;
	/*CRXING	*crx_store; */
	LINEAR_ELEMENT	 *lin_els;
	BILINEAR_ELEMENT *bilin_els;
	POINT_COMP_ST *pcs;

	int	guessed_num_lin;
	double	num_lin_guess_factor;

	BLK_EL1	 *blk_els1, *cg_blk_els1;
	BLK_EL0	 *blk_els0, *cg_blk_els0;

	int	n_node_points, n_tg_pts, n_fr_pts;
	int	n_reg_nodes;

	TG_PT	 *node_points, *cg_npts;
	TG_PT	 *front_points;

	/*COMPONENT *components, *cg_comps; */
	COMPONENT *cg_comps;

	Locstate *states, *cg_states;
	byte     *rect_state_storage;

	INTERFACE *grid_intfc;

		/* Allocation Status: */
	struct {
	    /*int seg_crx_count; */
	    /*int seg_crx_lists; */
	    /*int seg_crx_lists_store; */
	    int node_points;
	    int front_points;
	    int components;
	    int states;
	    int rect_state_storage;
	    /*int crx_store; */
	    int lin_els;
	    int bilin_els;
	    int pcs;
	    int blk_els0;
	    int blk_els1;
	} alloc;

#if defined(THREED)
	struct _THREED_ELEMENT_TOLERANCE {
	    double	_vtol;	/* General volume tolerance */
	    double	_stol;	/* General area tolerance */
	    double	_ltol;	/*General dist tolerance */
	} _vsl_tol;
#endif /* defined(THREED) */

	struct _TRI_GRID_HOOKS {
	    int	(*_construct_tri_grid)(struct _TRI_GRID*,RECT_GRID*,Front*);
	    boolean (*_blk_triangulate)(struct _TRI_GRID*,int*,
					LINEAR_ELEMENT**,
					BILINEAR_ELEMENT**);
	    boolean (*_pt_in_lin_el)(double*,LINEAR_ELEMENT*,
					 COMPONENT,struct _TRI_GRID*);
	    int	 (*_triangulate_mesh)(struct _TRI_GRID*);
	    int	 (*_set_components)(struct _TRI_GRID*,INTERFACE*);
#if defined(THREED)
	    int  (*_count_grid_crossings_on_intfc3d)(struct _TRI_GRID*);
	    void (*_set_crx_structure_storage3d)(struct _TRI_GRID*);
	    void (*_set_interpolation_storage3d)(struct _TRI_GRID*);
	    void (*_insert_grid_crossings3d)(struct _TRI_GRID*,Front*);
	    void (*_set_tri3d_tolerances)(struct _TRI_GRID*);
#endif /* defined(THREED) */
	    TRIANGULATION_METHOD _method; /*PROMPTED*/
	    boolean _use_least_square;
	} tri_grid_hooks;

#if defined(USE_OVERTURE)
        POINTER              cg_over; /* POINTER of CompositeGrid */
        POINTER              cg_over_function; /* pointer of doubleCompositeGridFunction */
        int                  patch_number;
        int                  patch_level;
        int                  NumberOfLevels;
        int                  use_overture_state;
        int                  overture_init_step;
        COMPONENT            patch_component; /* it's for patches without interface */
#endif /* defined(USE_OVERTURE) */
};
typedef struct _TRI_GRID TRI_GRID;
#if defined(__cplusplus)
typedef struct TRI_GRID::_TRI_GRID_HOOKS TRI_GRID_HOOKS;
#else /* defined(__cplusplus) */
typedef struct _TRI_GRID_HOOKS TRI_GRID_HOOKS;
#endif /* defined(__cplusplus) */

#define	interpolation_method(tg) (tg)->tri_grid_hooks._method
#define	use_least_square(tg) (tg)->tri_grid_hooks._use_least_square
#define	construct_tri_grid(tg,rgr,front)				\
	(tg)->tri_grid_hooks._construct_tri_grid(tg,rgr,front)
#define	triangulate_mesh(tg)	(*(tg)->tri_grid_hooks._triangulate_mesh)(tg)
#define	set_components(tg,ifc)	(*(tg)->tri_grid_hooks._set_components)(tg,ifc)
#define	point_in_linear_element(crds,et,comp,tg)			\
			(*(tg)->tri_grid_hooks._pt_in_lin_el)(crds,et,comp,tg)
#define	blk_triangulate(tg,ic,lins,blins)				\
				(*(tg)->tri_grid_hooks._blk_triangulate) \
						(tg,ic,lins,blins)
#if defined(THREED)
#define set_crx_structure_storage3d(tg)					\
    (*(tg)->tri_grid_hooks._set_crx_structure_storage3d)(tg)

#define count_grid_crossings_on_intfc3d(tg)				\
    (*(tg)->tri_grid_hooks._count_grid_crossings_on_intfc3d)(tg)

#define set_interpolation_storage3d(tg)					\
    (*(tg)->tri_grid_hooks._set_interpolation_storage3d)(tg)

#define set_tri3d_tolerances(tg)					\
    (*(tg)->tri_grid_hooks._set_tri3d_tolerances)(tg)
#endif /* defined(THREED) */

#if defined(THREED)
#define	vtol(ntg)	(ntg)->_vsl_tol._vtol
#define	stol(ntg)	(ntg)->_vsl_tol._stol
#define	ltol(ntg)	(ntg)->_vsl_tol._ltol
#endif /* defined(THREED) */


		/* Macros */

#define Trigrid_cell_area(ic,comp,tg)					\
	(*(tg)->area)(ic,comp,tg)

#define Locate_on_trigrid(coords,comp,tg,bilin,lin,lsq)			\
	(*(tg)->_locate_on_trigrid)(coords,comp,tg,bilin,lin,lsq)

#define	tg_pt_state(pt,tg)		((tg)->states[(pt)-(tg)->node_points])

	/*
	*  n_indx and c_indx are reserved
	*  for use by the below macros and may not be used outside of
	*  this context.  Their usage is dependent on the setting of the
	*  pointer offsets in the storage fields of the trigrid structure.
	*/

#define n_indx1d(ic,tg)							\
	( (ic)[0] )

#define n_indx2d(ic,tg)							\
	( (ic)[1]*(tg)->nnx + (ic)[0] )

#define n_indx3d(ic,tg)							\
	( ( (ic)[2]*(tg)->nny + (ic)[1] )*(tg)->nnx	 + (ic)[0] )

#define c_indx1d(ic,tg)							\
	( (ic)[0] )

#define c_indx2d(ic,tg)							\
	( (ic)[1]*(((tg)->rect_grid.gmax)[0]) + (ic)[0] )

#define c_indx3d(ic,tg)							\
	( ( (ic)[2]*(((tg)->rect_grid.gmax)[1]) +			\
			(ic)[1] )*(((tg)->rect_grid.gmax)[0])	 +	\
			(ic)[0] )

#if defined(ONED) && defined(TWOD) && defined(THREED)

#define n_indx(ic,tg)							\
	( (((tg)->rect_grid.dim) == 2) ?	n_indx2d(ic,tg) : 	\
	  (((tg)->rect_grid.dim) == 3) ?	n_indx3d(ic,tg) :	\
						n_indx1d(ic,tg)	)

#define c_indx(ic,tg)							\
	( (((tg)->rect_grid.dim) == 2) ?	c_indx2d(ic,tg) :	\
	  (((tg)->rect_grid.dim) == 3) ?	c_indx3d(ic,tg) :	\
						c_indx1d(ic,tg)	)

#elif defined(TWOD) && defined(THREED)

#define n_indx(ic,tg)							\
	( (((tg)->rect_grid.dim) == 2) ?	n_indx2d(ic,tg) :	\
						n_indx3d(ic,tg))

#define c_indx(ic,tg)							\
	( (((tg)->rect_grid.dim) == 2) ?	c_indx2d(ic,tg) :	\
						c_indx3d(ic,tg))

#elif defined(ONED) && defined(TWOD)

#define n_indx(ic,tg)							\
	( (((tg)->rect_grid.dim) == 2) ?	n_indx2d(ic,tg) :	\
						n_indx1d(ic,tg))

#define c_indx(ic,tg)							\
	( (((tg)->rect_grid.dim) == 2) ?	c_indx2d(ic,tg) :	\
						c_indx1d(ic,tg))

#elif defined(ONED) && defined(THREED)

#define n_indx(ic,tg)							\
	( (((tg)->rect_grid.dim) == 3) ?	n_indx3d(ic,tg) :	\
						n_indx1d(ic,tg))

#define c_indx(ic,tg)							\
	( (((tg)->rect_grid.dim) == 3) ?	c_indx3d(ic,tg) :	\
						c_indx1d(ic,tg))

#elif defined(TWOD)

#define n_indx(ic,tg)	n_indx2d(ic,tg)

#define c_indx(ic,tg)	c_indx2d(ic,tg)

#elif defined(THREED)

#define n_indx(ic,tg)	n_indx3d(ic,tg)	

#define c_indx(ic,tg)	c_indx3d(ic,tg)	

#elif defined(ONED)

#define n_indx(ic,tg)	n_indx1d(ic,tg)	

#define c_indx(ic,tg)	c_indx1d(ic,tg)	

#endif /* defined(ONED) && defined(TWOD) && defined(THREED) */

#define Regular_grid_node(ic,tg)	((tg)->cg_npts + n_indx((ic),(tg)))

#define Regular_grid_comp(ic,tg)	(tg)->cg_comps[n_indx((ic),(tg))]

#define Regular_grid_state(ic,tg)	((tg)->cg_states[n_indx((ic),(tg))])

#define	Regular_blk_el0(ic,tg)		((tg)->cg_blk_els0[c_indx((ic),(tg))])

#define	Regular_blk_el1(ic,tg)		((tg)->cg_blk_els1[c_indx((ic),(tg))])

#define	Blk_el0(ic,tg)			((tg)->blk_els0[c_indx((ic),(tg))])

#define	Blk_el1(ic,tg)			((tg)->blk_els1[c_indx((ic),(tg))])

#define alloc_states_array(ntg,n)					\
{									\
	VECTOR((ntg),states,(n),sizeof(Locstate));			\
	(ntg)->cg_states = (ntg)->states + (ntg)->node_offset;		\
}

#define alloc_node_points(ntg,n)					\
{									\
	VECTOR((ntg),node_points,(n),sizeof(TG_PT));			\
	(ntg)->cg_npts = (ntg)->node_points + (ntg)->node_offset;	\
}

#define	copy_node_points(ngrid,ogrid)					\
{									\
	(ngrid)->node_points = (ogrid)->node_points;			\
	(ngrid)->cg_npts = (ogrid)->cg_npts;				\
	(ngrid)->alloc.node_points = YES;				\
}

#define alloc_components_array(ntg,n)					\
{									\
	VECTOR((ntg),components,(n),sizeof(COMPONENT));			\
	(ntg)->cg_comps = (ntg)->components + (ntg)->node_offset;	\
}

#define	copy_trigrid_components(ngrid,ogrid)				\
{									\
	(ngrid)->components = (ogrid)->components;			\
	(ngrid)->cg_comps = (ogrid)->cg_comps;				\
	(ngrid)->alloc.components = YES;				\
}

#define alloc_blk_els0(ntg,n)						\
{									\
	VECTOR((ntg),blk_els0,(n),sizeof(BLK_EL0));			\
	(ntg)->cg_blk_els0 = (ntg)->blk_els0 + (ntg)->cell_offset;	\
}

#define alloc_blk_els1(ntg,n)						\
{									\
	VECTOR((ntg),blk_els1,(n),sizeof(BLK_EL1));			\
	(ntg)->cg_blk_els1 = (ntg)->blk_els1 + (ntg)->cell_offset;	\
}

/*
*	Initialization structure for tri library
*/

struct _TRI_INIT_DATA {
	F_INIT_DATA	F_init_data;
	TRI_GRID_HOOKS	Tri_grid_hooks;
};
typedef struct _TRI_INIT_DATA TRI_INIT_DATA;
#define	tri_init_data(init)	((TRI_INIT_DATA*)(init))
#define Tri_grid_hooks(init)	(tri_init_data(init)->Tri_grid_hooks)

#if defined(c_plusplus) || defined(__cplusplus)
}
#endif

#include <tri/triprotos.h>
#endif /* !defined(_TRIGRID_H) */
