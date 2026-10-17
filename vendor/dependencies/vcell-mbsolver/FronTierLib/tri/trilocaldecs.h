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
*			trilocaldecs.h:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Trigrid library limited declaration.
*/


#if !defined(_TRILOCALDECS_H)
#define _TRILOCALDECS_H

#include <tri/trigrid.h>

	/* Trigrid private Function Declarations*/

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif

#if defined(ONED)
/*	tri1d.c*/
IMPORT	int	insert_grid_crossings1d(TRI_GRID*);
IMPORT  int	no_triangulated_mesh(TRI_GRID*);
IMPORT	void	set_crx_structure_storage1d(TRI_GRID*);
#endif /* defined(ONED) */

#if defined(TWOD)
/*	tri2d.c*/
IMPORT	int	insert_grid_crossings2d(TRI_GRID*,int**);
IMPORT	void	set_crx_structure_storage2d(TRI_GRID*,int**);
#endif /* defined(TWOD) */

#if defined(THREED)
/*	tri3dutils.c*/
IMPORT	void	copy_tg_pts_from_regular_grid(TRI_GRID*);
#endif /* defined(THREED) */

/*	tricpy.c*/
#if defined(TWOD)
#if defined(DEBUG_TRI_GRID)
IMPORT  void    debug_copy_2d_tri_grid(TRI_GRID*,TRI_GRID*,size_t);
#endif /* defined(DEBUG_TRI_GRID) */
#endif /* defined(TWOD) */

/*	triel1.c*/
IMPORT	boolean	triangulate_el0(TRI_GRID*,int,int*,int,int,int*,int,int,int*,
				int,int,int*,int,LINEAR_ELEMENT**,int*);
IMPORT	void	retrieve_crossing_lists(TRI_GRID*,int*,int*,int*,int*,int*,
					int*,int*,int*,int*,int**,int**,
					int**,int**);
IMPORT	void	set_quad(TRI_GRID*,int*,int,int,int,int,BILINEAR_ELEMENT**);
IMPORT	void	set_tri(TRI_GRID*,LINEAR_ELEMENT**,int*,TG_PT*,TG_PT*,TG_PT*,
			Locstate,Locstate,Locstate,
			LIN_EL_FACE_TYPE,LIN_EL_FACE_TYPE,LIN_EL_FACE_TYPE,
			COMPONENT);
IMPORT	void	triangulate_el1(TRI_GRID*,CRXING*,int,int*,int,int,int*,
				CRXING*,LINEAR_ELEMENT**,int*);
IMPORT	void	triangulate_el2(TRI_GRID*,CRXING*,int,int*,int,int,int*,int,
				int,int*,CRXING*,LINEAR_ELEMENT**,int*);
IMPORT	void	triangulate_el4(TRI_GRID*,CRXING*,int,int*,int,int,int*,
				int,int,int*,int,int,int*,CRXING*,
				LINEAR_ELEMENT**,int*);

/*	triel2.c*/
EXPORT	INTERFACE	*make_blk_intfc(TRI_GRID*,int,int*,int,int,int*,
					int,int,int*,int,int,int*,
					int,int,NODE**);
IMPORT	boolean	triangulate_blk_with_nodes(TRI_GRID*,int*,LINEAR_ELEMENT**,
					   BILINEAR_ELEMENT**);
IMPORT	void	free_ntg_blk_node_lists(TRI_GRID*,int*);

/*	triel3.c*/
IMPORT	boolean	exact_triangulate_el3(TRI_GRID*,NODE**,int,int*,int,int,int,
				      int,int,int,int,int,int*,int*,int*,
				      int*,LINEAR_ELEMENT**);

/*	trigrid1.c*/
IMPORT  int     set_untracked_components(TRI_GRID*,INTERFACE*);
IMPORT	void	free_triangulation_storage(TRI_GRID*);
IMPORT	void	init_triangulation_storage(TRI_GRID*,int**,size_t);
IMPORT	int 	ERROR_RETURN_FROM_CONSTRUCT_TRI_GRID(const char*,
				int,INTERFACE*,TRI_GRID*);

#if defined(THREED)
/*	trigrid2.c*/
IMPORT	int	reconstruct_intfc_and_tri_grid(TRI_GRID*,RECT_GRID*,Front*);
#endif /* defined(THREED) */

/*	triloc.c*/
IMPORT	boolean tg_build(double*,COMPONENT,TRI_GRID*,BILINEAR_ELEMENT**,
			LINEAR_ELEMENT**,LEAST_SQR_CLUSTER**);
IMPORT	boolean tg_locate(double*,COMPONENT,TRI_GRID*,BILINEAR_ELEMENT**,
			LINEAR_ELEMENT**,LEAST_SQR_CLUSTER**);
#if defined(TWOD)
IMPORT	double	tg_area(int*,COMPONENT,TRI_GRID*);
#endif /* defined(TWOD) */

#if defined(TWOD) || defined(THREED)
/*	tripcs.c*/
IMPORT	void	copy_tg_pts_from_intfc(TRI_GRID*,P_LINK*,int);
#endif /* defined(TWOD) || defined(THREED) */

/*	triprint.c*/
IMPORT	void	print_trigrid_alloc_status(const char*,TRI_GRID*);
#if defined(TWOD)
IMPORT	void	print_BLK_EL0(BLK_EL0*,TRI_GRID*);
IMPORT	void	print_el0(TRI_GRID*,int,int*,TG_PT*,int,int*,TG_PT*,int,int*,
			  TG_PT*,int,int*,TG_PT*);
IMPORT	void	print_el1(TRI_GRID*,CRXING*,int,int*,TG_PT*,int,int*,CRXING*);
IMPORT	void	print_el10(TRI_GRID*,CRXING*,int,int*,CRXING*,CRXING*,
			   int,int*,CRXING*,CRXING*,int,int*,CRXING*,CRXING*,
			   int,int*,CRXING*);
IMPORT	void	print_el2(TRI_GRID*,CRXING*,int,int*,TG_PT*,int,int*,TG_PT*,
			  int,int*,CRXING*);
IMPORT	void	print_el3(TRI_GRID*,CRXING*,int,int*,CRXING*,CRXING*,
			  int,int*,CRXING*);
IMPORT	void	print_el4(TRI_GRID*,CRXING*,int,int*,TG_PT*,int,int*,TG_PT*,
			  int,int*,TG_PT*,int,int*,CRXING*);
IMPORT	void	print_el5(TRI_GRID*,CRXING*,int,int*,TG_PT*,int,int*,
			  CRXING*,CRXING*,int,int*,CRXING*);
IMPORT	void	print_el6(TRI_GRID*,CRXING*,int,int*,TG_PT*,int,int*,
			  CRXING*,CRXING*,int,int*,TG_PT*,int,int*,CRXING*);
IMPORT	void	print_el7(TRI_GRID*,CRXING*,int,int*,TG_PT*,int,int*,TG_PT*,
			  int,int*,CRXING*,CRXING*,int,int*,CRXING*);
IMPORT	void	print_el8(TRI_GRID*,CRXING*,int,int*,CRXING*,CRXING*,int,int*,
			  CRXING*,CRXING*,int,int*,CRXING*);
IMPORT	void	print_el9(TRI_GRID*,CRXING*,int,int*,TG_PT*,int,int*,CRXING*,
			  CRXING*,int,int*,CRXING*,CRXING*,int,int*,CRXING*);
IMPORT	void	print_seg_crx_count(TRI_GRID*,int);
IMPORT	void	print_seg_crx_lists(TRI_GRID*);
IMPORT	void	print_seg_crx_lists_store(TRI_GRID*,int);
IMPORT	void	print_side_crx_list(TRI_GRID*,char,int,int*);
#endif /* defined(TWOD) */
#if defined(TWOD) || defined(THREED)
IMPORT	void	print_crx_store(TRI_GRID*ntg,int n_crx);
#endif /* defined(TWOD) || defined(THREED) */
#if defined(THREED) && defined(NAVIGATOR)
IMPORT  void 	gview_blk_tri(BLK_TRI*,int,int,int,RECT_GRID*);
IMPORT  void    gview_block_and_tris(TRI**,int,double*,double*,int);
#endif /* defined(THREED) && defined(NAVIGATOR) */

#if defined(USE_OVERTURE)
IMPORT  void    use_overture_init_triangulation_storage(TRI_GRID*,int**,size_t);
IMPORT  void    set_AMR_triangulation_storage(TRI_GRID*,size_t);
IMPORT  void    copy_AMR_tri_storage2d(TRI_GRID*,TRI_GRID*,size_t);
#endif /* if defined(USE_OVERTURE) */

#if defined(c_plusplus) || defined(__cplusplus)
}
#endif

#endif /* !defined(_TRILOCALDECS_H) */
