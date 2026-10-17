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
*				tri3dv0.c:
*
*       Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Three dimensional specific tetrazation functions.
*/

#if defined(THREED)

#include <tri/tri3ddefs.h>

	/* LOCAL Function Declarations */
LOCAL	int	count_grid_crossings3d(TRI_GRID*);
LOCAL	int	set_components3d(TRI_GRID*,INTERFACE*);
LOCAL	void	set_crx_structure_storage3dv0(TRI_GRID*);
LOCAL	void	set_interpolation_storage3dv0(TRI_GRID*);
LOCAL	void	set_tri3d_tolerancesv0(TRI_GRID*);

EXPORT  void init_triangulation_options(
	TRI_GRID_HOOKS  *tri_grid_hooks,
	int dim)
{
	char s[Gets_BUF_SIZE];

	debug_print("init","Entered init_triangulation_options()\n");
	zero_scalar(tri_grid_hooks,sizeof(TRI_GRID_HOOKS));
	tri_grid_hooks->_method = COMPLETE_TRIANGULATION;
	tri_grid_hooks->_construct_tri_grid = t_construct_tri_grid;
	switch (dim)
	{
#if defined(ONED)
	case 1:
	    tri_grid_hooks->_set_components = set_components1d;
	    tri_grid_hooks->_triangulate_mesh = no_triangulated_mesh;
	    tri_grid_hooks->_pt_in_lin_el = NULL;
	    break;
#endif /* defined(ONED) */
#if defined(TWOD)
	case 2:
	    tri_grid_hooks->_set_components = set_components2d;
	    tri_grid_hooks->_triangulate_mesh = collect_pcs_in_mesh2d;
	    tri_grid_hooks->_method = ELEMENT_ON_THE_FLY;
	    tri_grid_hooks->_blk_triangulate = NULL;
	    tri_grid_hooks->_pt_in_lin_el = point_in_triangle;
	    screen("Select triangulation option, "
	           "exact [e], fast [f], or pcs [p, dflt]: ");
	    (void) Gets(s);
	    switch (s[0])
	    {
	    case 'f':
	    case 'F':
		tri_grid_hooks->_method = COMPLETE_TRIANGULATION;
	        tri_grid_hooks->_blk_triangulate = fast_blk_triangulate;
	    	tri_grid_hooks->_triangulate_mesh = triangulate_mesh2d;
	        break;
	    case 'e':
	    case 'E':
		tri_grid_hooks->_method = COMPLETE_TRIANGULATION;
		tri_grid_hooks->_blk_triangulate = exact_blk_triangulate;
	    	tri_grid_hooks->_triangulate_mesh = triangulate_mesh2d;
	        break;
	    case 'p':
	    case 'P':
	    default:
	    	break;
	    }
	    break;
#endif /* defined(TWOD) */
#if defined(THREED)
	case 3:
	    tri_grid_hooks->_pt_in_lin_el = fast_point_in_tetra;
	    set_tri3dv0_tri_grid_hooks(tri_grid_hooks);
	    tri_grid_hooks->_method = ELEMENT_ON_THE_FLY;
	    tri_grid_hooks->_triangulate_mesh = collect_pcs_in_mesh3d;
	    screen("Construct tetragrid [dflt = n]: ");
	    (void) Gets(s);
	    if ((s[0] == 'y') || (s[0] == 'Y'))
	    {
	    	tri_grid_hooks->_method = COMPLETE_TRIANGULATION;
	    	tri_grid_hooks->_triangulate_mesh = triangulate_mesh3d;
	    }
	    else
	    {
		tri_grid_hooks->_use_least_square = NO;
	    	screen("Use least square [dflt = n]: ");
	    	(void) Gets(s);
	    	if ((s[0] == 'y') || (s[0] == 'Y'))
		    tri_grid_hooks->_use_least_square = YES;
	    }
	    break;
#endif /* defined(THREED) */
	default:
	    screen("ERROR in init_triangulation_options(), "
	           "invalid computational dimension %d\n",dim);
	    clean_up(ERROR);
	}
	debug_print("init","Left init_triangulation_options()\n");
}		/*end init_triangulation_options*/


EXPORT	void	set_tri3dv0_tri_grid_hooks(
	TRI_GRID_HOOKS *tri_grid_hooks)
{
	tri_grid_hooks->_set_components = set_components3d;
	tri_grid_hooks->_set_crx_structure_storage3d =
	    set_crx_structure_storage3dv0;
	tri_grid_hooks->_count_grid_crossings_on_intfc3d = 
	    count_grid_crossings3d;
	tri_grid_hooks->_set_interpolation_storage3d =
	    set_interpolation_storage3dv0;
	tri_grid_hooks->_set_tri3d_tolerances = set_tri3d_tolerancesv0;
}		/*end set_tri3dv0_tri_grid_hooks*/

LOCAL	int set_components3d(
	TRI_GRID	*ntg,
	INTERFACE	*intfc)
{
	return set_grid_intfc_components(ntg->grid_intfc,intfc);
}		/*end set_components3d*/


LOCAL	void set_crx_structure_storage3dv0(
	TRI_GRID	*ntg)
{
	int		n_crx,n_segs;
	int             xmax, ymax, zmax;
	INTERFACE	*intfc = ntg->grid_intfc;
	Table		*T = table_of_interface(intfc);
	DEBUG_ENTER(set_crx_structure_storage3dv0)

	xmax = ntg->rect_grid.gmax[0];	
	ymax = ntg->rect_grid.gmax[1];	
	zmax = ntg->rect_grid.gmax[2];

	n_segs =     xmax   *(ymax+1)*(zmax+1) +
	            (xmax+1)* ymax   *(zmax+1) +
		    (xmax+1)*(ymax+1)* zmax;
	uni_array(&T->seg_crx_count,n_segs,INT);

	n_crx = count_grid_crossings3d(ntg);
	ntg->n_crx = n_crx;
	/*#bjet2 */
	T->n_crx = n_crx;

	init_seg_crx_lists(intfc,n_crx,n_segs);

	ntg->n_bilin_els = xmax*ymax*zmax;
	ntg->n_node_points = (xmax+1)*(ymax+1)*(zmax+1);
	alloc_node_points(ntg,ntg->n_node_points);
	alloc_blk_els0(ntg,ntg->n_bilin_els);

#if defined(DEBUG_TRI_GRID)
	if (debugging("TRI_storage"))
	{
	    (void) printf("gmax %d %d %d\n",xmax,ymax,zmax);
	}
#endif /* defined(DEBUG_TRI_GRID) */
	DEBUG_LEAVE(set_crx_structure_storage3dv0)
}		/*end set_crx_structure_storage3dv0*/


LOCAL int count_grid_crossings3d(
	TRI_GRID	*ntg)
{
	RECT_GRID	*rgr = &ntg->rect_grid;
	INTERFACE	*intfc = ntg->grid_intfc;

        /*printf("#count_grid_crossings3d: grid_intfc\n"); */
	set_dual_interface_topology(ntg);
	return count_grid_intfc_crossings(intfc);
}	/* end count_grid_crossings3d */

LOCAL void set_interpolation_storage3dv0(
        TRI_GRID *ntg)
{
        INTERFACE *intfc = ntg->grid_intfc;
        /*printf("#set_interpolation_storage3dv0: grid_intfc\n"); */

	set_dual_interface_topology(ntg);
        if (interpolation_method(ntg) == COMPLETE_TRIANGULATION)
        {
            ntg->n_lin_els = max_num_3d_lin_els(intfc);
            VECTOR(ntg,bilin_els,ntg->n_bilin_els,sizeof(BILINEAR_ELEMENT));
            VECTOR(ntg,lin_els,ntg->n_lin_els,sizeof(LINEAR_ELEMENT));
            ntg->n_fr_pts = intfc->num_points;
        }
        if (interpolation_method(ntg) == ELEMENT_ON_THE_FLY)
        {
            ntg->n_pcs = count_num_pcs3d(ntg);
            VECTOR(ntg,pcs,ntg->n_pcs,sizeof(POINT_COMP_ST));
            ntg->n_fr_pts = intfc->num_points;
        }
        VECTOR(ntg,front_points,ntg->n_fr_pts,sizeof(TG_PT));
}               /*end set_interpolation_storage3dv0*/


/*ARGSUSED*/
EXPORT void insert_grid_crossings3d(
	TRI_GRID	*ntg,
	Front		*front)
{
	INTERFACE	*intfc = ntg->grid_intfc;
	RECT_GRID	*rgr = &ntg->rect_grid;
	DEBUG_ENTER(insert_grid_crossings3d)

	set_dual_interface_topology(ntg);
	insert_grid_intfc_crossings3d(intfc);
	DEBUG_LEAVE(insert_grid_crossings3d)
}		/*end insert_grid_crossings3d*/


LOCAL void set_tri3d_tolerancesv0(
	TRI_GRID	*ntg)
{
	double	     *h = ntg->rect_grid.h;
	double	     hmin;
	int	     i;

	hmin = h[0];
	for (i = 1; i < 3; ++i)
	{
	    if (hmin > h[i])
		hmin = h[i];
	}

	ltol(ntg) = 0.001*hmin*IG_TOL; /*TOLERANCE*/
	stol(ntg) = ltol(ntg)*hmin;
	vtol(ntg) = stol(ntg)*hmin;
}		/*end set_tri3d_tolerancesv0*/
#endif /* defined(THREED) */
