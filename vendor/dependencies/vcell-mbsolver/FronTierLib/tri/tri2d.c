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
*				tri2d.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Two dimensional specific triangulation functions.
*	A triangulation of the computational region is constructed
*	from a rectangular grid and an interface.  The corners of
*	the triangles and quadrangles in the triangulation are either
*	intersections between grid lines of the rectangular grid or
*	intersections of grid lines with the interface.
*
*	TODO:
*		1. Rig up a mechanism for triangulating only selected
*			components.
*		2. Speed up area computations.
*/



#if defined(TWOD)
#include <tri/trilocaldecs.h>


	/* LOCAL Function Declarations */
LOCAL	int	add_crx_to_list2d(TRI_GRID*,int*,int,CROSSING_DIRECTION,
				  FT_POINT*,CURVE*,int*);
LOCAL	int	count_intfc_crossings_of_dual_lattice2d(TRI_GRID*,INTERFACE*,
							RECT_GRID*,int**);
LOCAL	int	find_index_of_node(NODE*,INTERFACE*);
LOCAL	int	grid_crossings_on_segment(TRI_GRID*,double,double,double,double,
					  int,int,int,int,double,double,double,
					  double,CURVE*,int*,int*,RECT_GRID*,
					  int*,FT_POINT**);
LOCAL	int	shift_from_cell_edge(double*,int,RECT_GRID*,double);
LOCAL	void	add_blk_node_to_list(BLK_EL1*,NODE*,TG_PT*);
LOCAL	void	count_crossings(TRI_GRID*,double,double,double,double,int,int,int,
			 int,double,double,double,double,CURVE*,int*,RECT_GRID*);

/*
*			insert_grid_crossings2d():
*
*	Inserts points in grid_intfc wherever a grid line intersects
*	the interface.  Stores these grid crossings in arrays indexed
*	by the appropriate grid line segment.
*
*	The algorithm for inserting grid crossings is taken from
*	merge_with_points_on_mesh_lines() in psgrid.c.  In this
*	algorithm grid_crossings_on_segment() is called for each
*	bond on the interface.  grid_crossings_on_segment() loads
*	the new_intfc_points[] array with the points to be inserted.
*	Then the insert_point_in_bond() is called to insert these
*	new points in grid_intfc.  The state associated with a new
*	point is expected to be calculated by USER_insert_point_in_bond,
*	presumably by interpolation along the bond.
*/

/*
*	Remark about tolerences in insert_grid_crossings2d().
*
*		The shifting of points on the interface to positions
*	at least some minimum distance from certain grid lines
*	has the potential of producing new tangles in the
*	hyp trigrid interface.  This problem can be expected
*	to occur when a node occupies a position that corresponds
*	to a corner of the dual lattice.  Since human nature
*	tends to think in terms of whole numbers,  it is
*	quite likely that stationary nodes such as sources and
*	sinks will be put in such locations.  In particular
*	runs that are being compared with finite difference
*	methods can be expected to have this property since
*	the wells will be placed at the centers of the computational
*	lattice and hence at the corners of the dual lattice.
*
*		An example occurs when a sink node with two curves
*	lies at a corner.  The node is shifted away from the
*	corner for the purposes of the component and triangulation
*	computation.  This shift causes the resulting grid crossing
*	of one of these curves to move out of the neighborhood of
*	of the nearest orthogonal grid line,  while the other curve 
*	crosses the same grid line as the first but lies inside this 
*	neighborhood.  This second point is then shifted by the tolerence
*	TOL*[hx or hy] to move it away from the corner of the grid
*	block,  but this shift can it past the other
*	crossing producing a new tangle.
*
*		Ideally,  the solution to this problem is to have
*	no (zero) tolerences,  but this will likely cause floating
*	problems and difficulties in ordering the grid crossings
*	around the grid blocks,  since it is possible for the
*	crossings to occur at corners (for example a 45 degree
*	Neumann boundary on a square mesh).   This problem can
*	be reduced by lowering the tolerences and requiring
*	that nodes be further shifted into the grid blocks than
*	regular grid crossings.  Other resolutions to this problem
*	will require complicated logic in order to detect and eliminate
*	(or never produce) these tangles.
*/

LOCAL	int	find_index_of_node(
	NODE		*node,
	INTERFACE	*intfc)
{
	NODE		**n;
	int		n_index = 0;
	
	for ( n = intfc->nodes; n && (*n != node); ++n)
	    ++n_index;

	return n_index;
}		/*end find_index_of_node*/

/*ARGSUSED*/
LIB_LOCAL	int insert_grid_crossings2d(
	TRI_GRID	*ntg,
	int		**ic_of_node)
{
	INTERFACE	*grid_intfc = ntg->grid_intfc;
	RECT_GRID	*rgr = &ntg->rect_grid;
	set_topological_grid(grid_intfc,rgr);
	set_grid_lines(&topological_grid(grid_intfc));
	insert_grid_intfc_crossings(grid_intfc);
	free_grid_lines(&topological_grid(grid_intfc));
	return GOOD_STEP;
}		/*end insert_grid_crossings2d*/

LOCAL	int count_intfc_crossings_of_dual_lattice2d(
	TRI_GRID	*ntg,
	INTERFACE	*intfc,
	RECT_GRID	*rgr,
	int		**ic_of_node)
{
	INTERFACE *grid_intfc = ntg->grid_intfc;
	int n_crx;
	set_topological_grid(grid_intfc,rgr);
	set_grid_lines(&topological_grid(grid_intfc));
	n_crx = count_grid_intfc_crossings(grid_intfc);
	free_grid_lines(&topological_grid(grid_intfc));
	return n_crx;
}		/*end count_intfc_crossings_of_dual_lattice2d*/


/*
*			set_components2d():
*
*	Sets the component list associated with each grid node
*	of tri_grid->rect_grid.
*/

EXPORT	int set_components2d(
	TRI_GRID	*ntg,
	INTERFACE	*intfc)
{
	COMPONENT	cS, cE, cN, cW;
	COMPONENT	*comp;
	COMPONENT	ext_comp = exterior_component(ntg->grid_intfc);
	Table		*T = table_of_interface(ntg->grid_intfc);
	CRXING	 	*cr;
	CRXING		*crx_store = T->crx_store;
	RECT_GRID	*gr = &ntg->rect_grid;
	double		coords[MAXD];
	int		xmax = gr->gmax[0], ymax = gr->gmax[1];
	int		xmaxx = xmax+1;
	int		ix, iy, ix0;
	int		k_s, k_w, k_n, k_e, ncs, ncw, ncn, nce;
	int		*s_list, *e_list, *n_list, *w_list;
	int		*seg_crx_count = T->seg_crx_count;
	int		**seg_crx_lists = T->seg_crx_lists;

#if defined(USE_OVERTURE)
        {
            if(intfc->curves == NULL)
            {
                int n_reg_nodes, i;
                n_reg_nodes = xmaxx;
                n_reg_nodes *= (ymax + 1);
                if(ntg->patch_component == NO_COMP or
                   ntg->patch_component == 0)
                {
                    printf("ERROR in set_components2d()\n");
                    printf("WHen USE_OVERTURE, patch_component[%d] of patch[%d]"
                         " is not properly set\n",ntg->patch_component,ntg->patch_number);
                    clean_up(ERROR);
                }
                for(i = 0; i < n_reg_nodes; i++)
                {
                    T->components[i] = ntg->patch_component;
                }
                debug_print("set_components",
                   "Leaving set_components2d(), status = GOOD_STEP\n");
                return GOOD_STEP;
            }
        }
#endif /* if defined(USE_OVERTURE) */

	return set_grid_intfc_components(ntg->grid_intfc,intfc);
}		/*end set_components2d*/


LOCAL	int	shift_from_cell_edge(
	double		*p,
	int		idir,
	RECT_GRID	*rgr,
	double		tol)
{
	int		ic;
	double		sfrac;

	ic = cell_index(p[idir],idir,rgr);
	sfrac = (p[idir] - rgr->edges[idir][ic]) / cell_width(ic,idir,rgr);
	if (sfrac < IG_TOL)     		p[idir] += tol;
	else if (sfrac > IG_ONEMTOL)	p[idir] -= tol;
	return ic;
}		/*end shift_from_cell_edge*/


LIB_LOCAL	void set_crx_structure_storage2d(
	TRI_GRID	*ntg,
	int		**ic_of_node)
{
	RECT_GRID	*r_gr = &ntg->rect_grid;
	INTERFACE	*intfc = ntg->grid_intfc;
	BLK_EL0		*blk_el0;
	BLK_EL1		*blk_el1;
	NODE		**node;
	int		n_crx, ii, n_segs;
	int		ix, iy;
	int		n_intfc_nodes;
	static	BLK_EL0 CLEAR_BLK_EL0;
	static	BLK_EL1 CLEAR_BLK_EL1;
	Table	*T = table_of_interface(ntg->grid_intfc);
	double		XL,XU,YL,YU,hx,hy;
	int		xmax,ymax;

	xmax = r_gr->gmax[0];	ymax = r_gr->gmax[1];
	XL   = r_gr->L[0];	YL   = r_gr->L[1];
	XU   = r_gr->U[0];	YU   = r_gr->U[1];
	hx   = r_gr->h[0];	hy   = r_gr->h[1];

	n_segs = xmax*(ymax+1) + ymax*(xmax+1);
	uni_array(&T->seg_crx_count,n_segs,INT);
	for (ii = 0;  ii < n_segs;  ++ii) T->seg_crx_count[ii] = 0;

	set_topological_grid(ntg->grid_intfc,r_gr);
	n_crx = count_intfc_crossings_of_dual_lattice2d(ntg,ntg->grid_intfc,
			r_gr,ic_of_node);

	ntg->n_crx = n_crx;
	init_seg_crx_lists(intfc,n_crx,n_segs);

	n_intfc_nodes = 0;
	for (node = intfc->nodes; node && *node;  ++node)
		++n_intfc_nodes;

	if (interpolation_method(ntg) != ELEMENT_ON_THE_FLY)
	{
	    int nlin;
	    nlin = ntg->guessed_num_lin =
		2*((int)(ntg->num_lin_guess_factor*n_crx));
	    VECTOR(ntg,lin_els,nlin,sizeof(LINEAR_ELEMENT));
	    VECTOR(ntg,bilin_els,xmax*ymax,sizeof(BILINEAR_ELEMENT));
	    ntg->n_fr_pts = intfc->num_points;
	}
	else
	    ntg->n_fr_pts = intfc->num_points + n_crx;
	ntg->n_node_points += n_crx+n_intfc_nodes;
	alloc_node_points(ntg,ntg->n_node_points);
	VECTOR(ntg,front_points,ntg->n_fr_pts,sizeof(TG_PT));

	ntg->n_tg_pts = 0;

#if defined(DEBUG_TRI_GRID)
	if (debugging("TRI_storage"))
	{
		int n_reg_nodes = (xmax+1)*(ymax+1);
		(void) printf("Storage set for %d triangles\n",
			      ntg->guessed_num_lin);
		(void) printf("gmax %d %d\n",xmax,ymax);
		(void) printf("n_intfc_nodes %d n_reg_nodes %d n_crx %d\n",
					n_intfc_nodes,n_reg_nodes,n_crx);
	}
#endif /* defined(DEBUG_TRI_GRID) */

	alloc_blk_els0(ntg,ymax*xmax);
	alloc_blk_els1(ntg,ymax*xmax);
	blk_el0 = ntg->blk_els0;	blk_el1 = ntg->blk_els1;
	for (iy = 0; iy < ymax; ++iy)
	{
	    for (ix = 0; ix < xmax; ++ix)
	    {
	    	*blk_el0++ = CLEAR_BLK_EL0;
	    	*blk_el1++ = CLEAR_BLK_EL1;
	    }
	}
}		/*end set_crx_structure_storage2d*/

#endif /* defined(TWOD) */
