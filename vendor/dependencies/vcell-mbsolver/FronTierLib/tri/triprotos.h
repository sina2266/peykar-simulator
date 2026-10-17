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
*			triprotos.h
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*/

#if !defined(_TRIPROTOS_H)
#define _TRIPROTOS_H

#include <tri/trigrid.h>

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif

	/* Trigrid EXPORTED Function Declarations*/
#if defined(ONED)
/*	tri1d.c*/
IMPORT	int     set_components1d(TRI_GRID*,INTERFACE*);
#endif /* defined(ONED) */

#if defined(TWOD)
/*	tri2d.c*/
IMPORT	int	set_components2d(TRI_GRID*,INTERFACE*);
#endif /* defined(TWOD) */

#if defined(THREED)
/*	tri3d.c*/
IMPORT	void	set_tri3dv0_tri_grid_hooks(TRI_GRID_HOOKS*);
IMPORT	void	init_triangulation_options(TRI_GRID_HOOKS*,int);
IMPORT	void	insert_grid_crossings3d(TRI_GRID*,Front*);

/*	tri3dutils.c*/
IMPORT	boolean	point_in_tetra(double*,LINEAR_ELEMENT*,COMPONENT,TRI_GRID*);
IMPORT	boolean	fast_point_in_tetra(double*,LINEAR_ELEMENT*,COMPONENT,TRI_GRID*);
IMPORT	int	triangulate_mesh3d(TRI_GRID*);

#endif /* defined(THREED) */

/*	tricpy.c*/
IMPORT	void	copy_tri_grid(TRI_GRID*,TRI_GRID*,size_t);
IMPORT	void	free_copy_tri_grid(TRI_GRID**);

/*	triel1.c*/
IMPORT	boolean	fast_blk_triangulate(TRI_GRID*,int*,
				     LINEAR_ELEMENT**,BILINEAR_ELEMENT**);
IMPORT	int	triangulate_mesh2d(TRI_GRID*);

/*	triel2.c*/
IMPORT	boolean	exact_blk_triangulate(TRI_GRID*,int*,LINEAR_ELEMENT**,
				      BILINEAR_ELEMENT**);

/*	triel3.c*/

/*	trigrid1.c*/
IMPORT	BLK_EL0	*blk_el0_for_coords(double*,TRI_GRID*);
IMPORT	TRI_GRID	*allocate_tri_grid(TRI_GRID_HOOKS*);
IMPORT	int	**set_node_index_list(TRI_GRID*);
IMPORT	int	t_construct_tri_grid(TRI_GRID*,RECT_GRID*,Front*);
IMPORT	void	free_hyp_tri_grid(TRI_GRID**);
IMPORT	void	free_tri_grid(TRI_GRID*);
IMPORT	void	set_dual_interface_topology(TRI_GRID*);
IMPORT	void	set_tri_grid_rect_grids(TRI_GRID*,RECT_GRID*,INTERFACE*);
IMPORT	double	area_of_domain_with_comp(Front*,COMPONENT);

/*	trigrid2.c*/

/*	triloc.c*/
IMPORT	int	crossings_in_direction(CRXING**,int*,GRID_DIRECTION,TRI_GRID*);
#if defined(ONED)
IMPORT	CRXING *nearest_crossing1d(int*,GRID_DIRECTION,TRI_GRID*);
#endif /* defined(ONED) */
#if defined(TWOD)
IMPORT	CRXING	*nearest_crossing2d(int*,GRID_DIRECTION,TRI_GRID*);
IMPORT	boolean	point_in_triangle(double*,LINEAR_ELEMENT*,COMPONENT,TRI_GRID*);
#endif /* defined(TWOD) */
#if defined(THREED)
IMPORT	CRXING	*nearest_crossing3d(int*,GRID_DIRECTION,TRI_GRID*);
IMPORT	boolean	nearest_crossing_state_with_comp(int*,double*,COMPONENT,
					TRI_GRID*,Locstate*);
#endif /* defined(THREED) */
#if defined(DEBUG_TRI_LOC)
IMPORT	void	set_debug_tri_loc(int);
#endif /* defined(DEBUG_TRI_LOC) */

/*	tripcs.c*/
#if defined(TWOD)
IMPORT	int	collect_pcs_in_mesh2d(TRI_GRID*);
#endif /* defined(TWOD) */
#if defined(THREED)
IMPORT	int	collect_pcs_in_mesh3d(TRI_GRID*);
#endif /* defined(THREED) */

/*	triprint.c*/
IMPORT	void	print_BILINEAR_ELEMENT(BILINEAR_ELEMENT*,TRI_GRID*);
IMPORT	void	print_LINEAR_ELEMENT(LINEAR_ELEMENT*,TRI_GRID*);
IMPORT  void    print_components(TRI_GRID*);
#if defined(TWOD)
IMPORT	void	print_triangulation(FILE*,TRI_GRID*);
IMPORT	void	print_blk_els0(TRI_GRID*);
IMPORT	void	print_blk_els1(TRI_GRID*);
#endif /* defined(TWOD) */

/*	triuserintfc.c*/
IMPORT	void	t_set_interface_hooks(int,INIT_DATA*);

#if defined(c_plusplus) || defined(__cplusplus)
}
#endif

#endif /* !defined(_TRIPROTOS_H) */
