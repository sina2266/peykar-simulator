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
*				trigrid1.c:
*
*       Copyright 1999 by The University at Stony Brook, All rights reserved.
*
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

#if defined(DEBUG_TRI_GRID)
#define DEBUG_STRING    "tri_grid"
#endif /* defined(DEBUG_TRI_GRID) */

#include <tri/trilocaldecs.h>

	/* LOCAL Function Declarations */
LOCAL	void	free_crx_lists(TRI_GRID*);
LOCAL   int     orig_construct_tri_grid(TRI_GRID*,RECT_GRID*,Front*);


#if defined(DEBUG_TRI_GRID)
LOCAL	void	end_frame(void);
LOCAL	void	open_plot_file(void);
LOCAL	void	plot_crossings2d(TRI_GRID*);
LOCAL	void	plot_grid_lines(RECT_GRID*);
LOCAL	void	plot_intfc(INTERFACE*);
LOCAL	void	set_plot_window(RECT_GRID*);
#endif /* defined(DEBUG_TRI_GRID) */

/*
*			t_construct_tri_grid()
*
*	Driver for the actual triangulation of the grid.
*	TODO:
*		1. Triangulate only specific component(s)
*		2. Check on bad aspect ratios of triangles.
*
*	The construction is described in (for now see Notes by Lindquist)
*	The construction routines are in triels.c . All triangles/rectangles
*	are oriented counter-clockwise.
*/

EXPORT	int ERROR_RETURN_FROM_CONSTRUCT_TRI_GRID(
	const char *mesg,
	int	   status,
	INTERFACE  *cintfc,
	TRI_GRID   *grid)
{
	(void) printf("WARNING in construct_tri_grid(), %s\n",mesg);
	set_current_interface(cintfc);
	free_grid_lines(&grid->rect_grid);
	return status;
}		/*end ERROR_RETURN_FROM_CONSTRUCT_TRI_GRID*/

EXPORT  int t_construct_tri_grid(
	register TRI_GRID *grid,
	RECT_GRID         *rgr,
	Front             *front)
{
	INTERFACE       *intfc = front->interf;
	int		status = ERROR_IN_STEP;
	DEBUG_ENTER(t_construct_tri_grid)

	switch (intfc->dim)
	{
	case 1:
	case 2:
	    status = orig_construct_tri_grid(grid,rgr,front);
	    break;
#if defined(THREED)
	case 3:
	    status = (interface_reconstructed(intfc) == NO) ?
	                 orig_construct_tri_grid(grid,rgr,front) :
	                 reconstruct_intfc_and_tri_grid(grid,rgr,front);
	    break;
#endif /* defined(THREED) */
	}
	DEBUG_LEAVE(t_construct_tri_grid)
	return status;
}       /* end t_construct_tri_grid */

LOCAL	int orig_construct_tri_grid(
	TRI_GRID  *grid,
	RECT_GRID *rgr,
	Front     *front)
{
	INTERFACE	*intfc = front->interf;
	INTERFACE	*cintfc;
	int		dim = rgr->dim;
	int		status = ERROR_IN_STEP;
	int             stat;
	size_t		sizest = front->sizest;
#if defined(TWOD) || defined(THREED)
	int		**ic_of_node;
#endif /* defined(TWOD) || defined(THREED) */
	DEBUG_ENTER(orig_construct_tri_grid)

	start_clock("orig_construct_tri_grid");

	set_tri_grid_rect_grids(grid,rgr,intfc);
#if defined(THREED)
	if (dim == 3)
	    set_tri3d_tolerances(grid);
#endif /* defined(THREED) */

#if defined(DEBUG_TRI_GRID)
	if (debugging("plot_tri_grid"))
	{
	    open_plot_file();
	    set_plot_window(&grid->rect_grid);
	    plot_grid_lines(&grid->rect_grid);
	    plot_intfc(intfc);
	    end_frame();
	}
#endif /* defined(DEBUG_TRI_GRID) */

	    /* copy grid_intfc from intfc and insert grid crossings, */
	    /* storing the information in the crx_store[] array */

	cintfc = current_interface();

	set_size_of_intfc_state(sizest);
	set_copy_intfc_states(YES);
	if ((grid->grid_intfc = copy_interface(intfc)) == NULL)
	{
	    stat = ERROR_RETURN_FROM_CONSTRUCT_TRI_GRID(
                        "copy_interface() failed",
			ERROR_IN_STEP,cintfc,grid);
	    DEBUG_LEAVE(orig_construct_tri_grid )
	    return stat;
	}
	no_topology_lists(grid->grid_intfc) = YES;

	    /*
	    *	Set correspond curve on intfc to null, but leave
	    *	the curve correspondence from grid_intfc to intfc
	    *	intact.
	    */

	set_correspond_hyper_surfaces_to_NULL(intfc);

	delete_subdomain_boundaries(grid->grid_intfc);
/*	delete_passive_boundaries(grid->grid_intfc); */

	interpolate_intfc_states(grid->grid_intfc) = YES;

#if defined(DEBUG_TRI_GRID)
	if (DEBUG)
	{
	    (void) printf("Grid interface before insert_grid_crossings()\n");
	    print_interface(grid->grid_intfc);
	}
#endif /* defined(DEBUG_TRI_GRID) */

	start_clock("insert_grid_crossings");
	switch (dim)
	{
#if defined(ONED)
	case 1:
	    init_triangulation_storage(grid,NULL,sizest);
	    status = insert_grid_crossings1d(grid);
	    break;
#endif /* defined(ONED) */
#if defined(TWOD)
	case 2:
	    ic_of_node = set_node_index_list(grid);
	    init_triangulation_storage(grid,ic_of_node,sizest);
	    status = insert_grid_crossings2d(grid,ic_of_node);
	    free(ic_of_node);
	    break;
#endif /* defined(TWOD) */
#if defined(THREED)
	case 3:
	    ic_of_node = set_node_index_list(grid);
	    init_triangulation_storage(grid,ic_of_node,sizest);
	    insert_grid_crossings3d(grid,front);
	    set_interpolation_storage3d(grid);
	    free(ic_of_node);
	    status = GOOD_STEP;
	    break;
#endif /* defined(THREED) */
	}
	stop_clock("insert_grid_crossings");

	if (status != GOOD_STEP)
	{
	    stat = ERROR_RETURN_FROM_CONSTRUCT_TRI_GRID(
	        "can't insert grid crossings",status,cintfc,grid);
	    DEBUG_LEAVE(orig_construct_tri_grid)
	    return stat;
	}

#if defined(DEBUG_TRI_GRID)
	if (debugging("plot_tri_grid"))
	{
	    open_plot_file();
	    set_plot_window(&grid->rect_grid);
	    plot_grid_lines(&grid->rect_grid);
	    plot_intfc(grid->grid_intfc);
	    end_frame();
	    
	    set_plot_window(&grid->rect_grid);
	    plot_intfc(grid->grid_intfc);
#if defined(TWOD)
	    if (dim == 2)
	        plot_crossings2d(grid);
#endif /* defined(TWOD) */
	    end_frame();
	}
#endif /* defined(DEBUG_TRI_GRID) */

	start_clock("set_components");
	status = set_components(grid,intfc);
#if defined(DEBUG_TRI_GRID)
	if (status == GOOD_STEP && debugging("tri_comps"))
	    print_components(grid);
#endif /* defined(DEBUG_TRI_GRID) */
	stop_clock("set_components");

	if (status != GOOD_STEP)
	{
	    return ERROR_RETURN_FROM_CONSTRUCT_TRI_GRID("set_components failed",
							status,cintfc,grid);
	}

	    /* set up grid nodes, triangles, and quadrangles */

	start_clock("triangulate_mesh");
	status = triangulate_mesh(grid);
	stop_clock("triangulate_mesh");

	if (status != GOOD_STEP)
        {
	    stat = ERROR_RETURN_FROM_CONSTRUCT_TRI_GRID(
		    "triangulate_mesh() failed",status,cintfc,grid);
	    DEBUG_LEAVE(orig_construct_tri_grid)
	    return stat;
	}

	free_grid_lines(&grid->rect_grid);
	set_current_interface(cintfc);

	stop_clock("orig_construct_tri_grid");
	debug_print("tri_grid","Left orig_construct_tri_grid()\n");
	DEBUG_LEAVE(orig_construct_tri_grid)
	return status;
}		/*end orig_construct_tri_grid*/


EXPORT	TRI_GRID *allocate_tri_grid(
	TRI_GRID_HOOKS  *tri_grid_hooks)
{
	TRI_GRID *ntg;
	DEBUG_ENTER(allocate_tri_grid )

	scalar(&ntg,sizeof(TRI_GRID));
	if (ntg == NULL)
	{
	    DEBUG_LEAVE(allocate_tri_grid )
	    return ntg;
	}
	if (tri_grid_hooks != NULL)
	    ntg->tri_grid_hooks = *tri_grid_hooks;
	ntg->num_lin_guess_factor = 1.5;	/*TOLERANCE*/
	DEBUG_LEAVE(allocate_tri_grid )
	return ntg;
}		/*end allocate_tri_grid*/


EXPORT	void free_hyp_tri_grid(
	TRI_GRID	**ptri_grid)
{
	if (ptri_grid == NULL || *ptri_grid == NULL) return;
	free_tri_grid(*ptri_grid);
	free(*ptri_grid);
	*ptri_grid = NULL;
}		/*end free_hyp_tri_grid*/



/*
*	Frees a TRI_GRID structure.
*/

EXPORT	void free_tri_grid(
	TRI_GRID	*grid)
{
	if (grid == NULL) return;

	free_crx_lists(grid);
	if (grid->grid_intfc != NULL)
		(void) delete_interface(grid->grid_intfc);

	free_triangulation_storage(grid);
}		/*end free_tri_grid*/

LOCAL	void free_crx_lists(
	TRI_GRID	*ntg)
{
	Table *T = table_of_interface(ntg->grid_intfc);
	print_storage("before free_crx_lists","TRI_storage");
#if defined(DEBUG_TRI_GRID)
	if (debugging("seg_crx"))
	{
	    (void) printf("\n\tBefore free cross storage\n");
	    print_trigrid_alloc_status("",ntg);
	}
#endif /* defined(DEBUG_TRI_GRID) */

	if (T != NULL)
	{
	    free_these(5,T->seg_crx_count,T->seg_crx_lists,
	    		T->seg_crx_lists_store,T->crx_store,T->components);
	}

	/*
	Set_free(ntg,seg_crx_count); ntg->seg_crx_count = NULL;
	Set_free(ntg,seg_crx_lists); ntg->seg_crx_lists = NULL;
	Set_free(ntg,seg_crx_lists_store); ntg->seg_crx_lists_store = NULL;
	Set_free(ntg,crx_store); ntg->crx_store = NULL;
	*/
	print_storage("after free_crx_lists","TRI_storage");
}		/*end free_crx_lists*/

LIB_LOCAL	void free_triangulation_storage(
	TRI_GRID	*ntg)
{
	print_storage("before free_triangulation_storage","TRI_storage");
#if defined(DEBUG_TRI_GRID)
	if (debugging("TRI_storage"))
	{
	    (void) printf("\n\tBefore free triangulation storage\n");
	    print_trigrid_alloc_status("",ntg);
	}
#endif /* defined(DEBUG_TRI_GRID) */

	switch (ntg->rect_grid.dim)
	{
#if defined(ONED)
	case 1:
	    break;
#endif /* defined(ONED) */
#if defined(TWOD)
	case 2:
	    if (ntg->alloc.blk_els1)
	    	free_ntg_blk_node_lists(ntg,ntg->rect_grid.gmax);
	    Set_free(ntg,blk_els1); ntg->blk_els1 = ntg->cg_blk_els1 = NULL;
	    break;
#endif /* defined(TWOD) */
#if defined(THREED)
	case 3:
	    break;
#endif /* defined(THREED) */
	}

	Set_free(ntg,lin_els);		ntg->lin_els = NULL;
	Set_free(ntg,bilin_els);	ntg->bilin_els = NULL;
	Set_free(ntg,pcs);		ntg->pcs = NULL;
	Set_free(ntg,blk_els0);	ntg->blk_els0 = ntg->cg_blk_els0 = NULL;
	Set_free(ntg,blk_els1);	ntg->blk_els1 = ntg->cg_blk_els1 = NULL;
	Set_free(ntg,node_points); ntg->node_points = ntg->cg_npts = NULL;
	Set_free(ntg,front_points);ntg->front_points = NULL;
	Set_free(ntg,states); ntg->states = ntg->cg_states = NULL;
	Set_free(ntg,rect_state_storage); ntg->rect_state_storage = NULL;
	print_storage("after free_triangulation_storage","TRI_storage");
}		/*end free_triangulation_storage*/

EXPORT	BLK_EL0 *blk_el0_for_coords(
	double		*coords,
	TRI_GRID	*ntg)
{
	int		icoords[MAXD];

	if (rect_in_which(coords,icoords,&ntg->rect_grid) == FUNCTION_FAILED)
	{
		screen("ERROR in blk_el0_for_coords(), ");
		screen("point not on grid\n");
		clean_up(ERROR);
	}
	return &Blk_el0(icoords,ntg);
}		/*end blk_el0_for_coords*/


LIB_LOCAL	int	set_untracked_components(
	TRI_GRID	*ntg,
	INTERFACE	*intfc)
{
	Table		*T = table_of_interface(ntg->grid_intfc);
	RECT_GRID	*gr = &ntg->rect_grid;
	RECT_GRID	*c_gr;
	COMPONENT	ext_comp = exterior_component(ntg->grid_intfc);
	COMPONENT	icomp;
	COMPONENT	*comp;
	COMPONENT	*comps = T->components;
	double		coords[MAXD];
	int		xmax = 0, ymax = 0, zmax = 0;
	int		xmaxx = 0, ymaxx = 0;
	int		i, j, k, dim = gr->dim;
	int		n_reg_nodes = ntg->n_reg_nodes;

	c_gr = computational_grid(intfc);
	for (i = 0; i < dim; ++i)
	    coords[i] = grid_center_coord(i,c_gr);
	icomp = component(coords,intfc);
	comp = comps;
	for (i = 0; i < n_reg_nodes; ++i)
	    *comp++ = icomp;

	switch (dim)
	{
#if defined(ONED)
	case 1:
	    xmax = gr->gmax[0];
	    break;
#endif /* defined(ONED) */
#if defined(TWOD)
	case 2:
	    ymax = gr->gmax[1];
	    xmax = gr->gmax[0];	xmaxx = xmax+1;
	    break;
#endif /* defined(TWOD) */
#if defined(THREED)
	case 3:
	    zmax = gr->gmax[2];
	    ymax = gr->gmax[1];	ymaxx = ymax+1;
	    xmax = gr->gmax[0];	xmaxx = xmax+1;
	    break;
#endif /* defined(THREED) */
	}

	if (!buffered_boundary_type(rect_boundary_type(intfc,0,0)))
	{
	    for (k = 0; k <= zmax; ++k)
	    {
	    	comp = comps + k*xmaxx*ymaxx;
	    	for (j = 0; j <= ymax; ++j, comp += xmaxx)
	    	    *comp = ext_comp;
	    }
	}
	if (!buffered_boundary_type(rect_boundary_type(intfc,0,1)))
	{
	    for (k = 0; k <= zmax; ++k)
	    {
	    	comp = comps + xmax + k*xmaxx*ymaxx;
	    	for (j = 0; j <= ymax; ++j, comp += xmaxx)
		    *comp = ext_comp;
	    }
	}

#if defined(ONED)
	if (dim == 1)
	    return GOOD_STEP;
#endif /* defined(ONED) */

#if defined(TWOD) || defined(THREED)
	if (!buffered_boundary_type(rect_boundary_type(intfc,1,0)))
	{
	    for (k = 0; k <= zmax; ++k)
	    {
	    	comp = comps + k*xmaxx*ymaxx;
	    	for (i = 0; i <= xmax; ++i, comp += 1)
	    	    *comp = ext_comp;
	    }
	}
	if (!buffered_boundary_type(rect_boundary_type(intfc,1,1)))
	{
	    for (k = 0; k <= zmax; ++k)
	    {
	    	comp = comps + ymax*xmaxx + k*xmaxx*ymaxx;
	    	for (i = 0; i <= xmax; ++i, comp += 1)
	    	    *comp = ext_comp;
	    }
	}
#endif /* defined(TWOD) || defined(THREED) */

#if defined(TWOD)
	if (dim == 2)
	    return GOOD_STEP;
#endif /* defined(TWOD) */

#if defined(THREED)
	if (!buffered_boundary_type(rect_boundary_type(intfc,2,0)))
	{
	    comp = comps;
	    for (j = 0; j <= ymax; ++j)
	    	for (i = 0; i <= xmax; ++i, comp += 1)
	    	    *comp = ext_comp;
	}
	if (!buffered_boundary_type(rect_boundary_type(intfc,2,1)))
	{
	    comp = comps + zmax*xmaxx*ymaxx;
	    for (j = 0; j <= ymax; ++j)
	    	for (i = 0; i <= xmax; ++i, comp += 1)
	    	    *comp = ext_comp;
	}
#endif /* defined(THREED) */

	return GOOD_STEP;
}		/*end set_untrack_components*/

EXPORT	void	set_tri_grid_rect_grids(
	TRI_GRID	*grid,
	RECT_GRID	*dual_grid,
	INTERFACE	*intfc)
{
	RECT_GRID *expanded_dual_grid = &grid->rect_grid;
	RECT_GRID *unexpanded_dual_grid = &grid->tg_grid;
	RECT_GRID *comp_grid = &grid->comp_grid;
	RECT_GRID *cgr = computational_grid(intfc);
	int	  gmax[3];
	int	  i, dim = dual_grid->dim;
	int       *offset = grid->offset;
        int       xmaxx,ymaxx;

	DEBUG_ENTER(set_tri_grid_rect_grids)

	copy_rect_grid(comp_grid,cgr);
	copy_rect_grid(unexpanded_dual_grid,dual_grid);

	for (i = 0; i < dim; ++i)
	    gmax[i] = dual_grid->gmax[i]+dual_grid->lbuf[i]+dual_grid->ubuf[i];

	set_rect_grid(dual_grid->VL,dual_grid->VU,dual_grid->GL,dual_grid->GU,
		      NOBUF,NOBUF,gmax,dim,&cgr->Remap,expanded_dual_grid);
	for (i = 0; i < dim; ++i)
	    expanded_dual_grid->h[i] = dual_grid->h[i];

	switch (dim)
	{
	case 3:
	    grid->nnz = gmax[2] + 1;
	    grid->nny = gmax[1] + 1;
	    grid->nnx = gmax[0] + 1;
	    grid->node_offset = 
	        dual_grid->lbuf[0]+1 + grid->nnx * (dual_grid->lbuf[1]+1) +
	        grid->nnx * grid->nny * (dual_grid->lbuf[2]+1);
	    grid->cell_offset = 
	        dual_grid->lbuf[0] + gmax[0] * dual_grid->lbuf[1] +
	        gmax[0] * gmax[1] * dual_grid->lbuf[2];
	    xmaxx = gmax[0] + 1;        ymaxx = gmax[1] + 1;
	    offset[0] = 0;              offset[2] = xmaxx;
	    offset[4] = 1;              offset[6] = offset[2] + 1;
	    offset[1] = xmaxx*ymaxx;    offset[3] = offset[1] + xmaxx;
	    offset[5] = offset[1] + 1;  offset[7] = offset[3] + 1;
	    break;
	case 2:
	    grid->nny = gmax[1] + 1;
	    grid->nnx = gmax[0] + 1;
	    grid->node_offset = dual_grid->lbuf[0] + 1 +
	        grid->nnx * (dual_grid->lbuf[1] + 1);
	    grid->cell_offset = 
	        dual_grid->lbuf[0] + gmax[0] * dual_grid->lbuf[1];
	    xmaxx = gmax[0] + 1;
            offset[0] = 0;                  offset[2] = xmaxx +1;
            offset[1] = 1;                  offset[3] = xmaxx;
	    break;
	case 1:
	    grid->nnx = gmax[0] + 1;
	    grid->node_offset = dual_grid->lbuf[0] + 1;
	    grid->cell_offset = dual_grid->lbuf[0];
	    break;
	}

	if (!set_grid_lines(expanded_dual_grid))
	{
	    screen("ERROR in set_tri_grid_rect_grids(), "
	           "set_grid_lines() failed\n");
	    clean_up(ERROR);
	}
#if defined(DEBUG_TRI_GRID)
	if (DEBUG)
	{
	    (void) printf("\nRect Grids for Tri Grid\n");
	    (void) printf("\ntri_grid->comp_grid\n");
	    print_rectangular_grid(comp_grid);
	    (void) printf("\ntri_grid->tg_grid\n");
	    print_rectangular_grid(unexpanded_dual_grid);
	    (void) printf("\ntri_grid->rect_grid\n");
	    print_rectangular_grid(expanded_dual_grid);
	}
#endif /* defined(DEBUG_TRI_GRID) */
	DEBUG_LEAVE(set_tri_grid_rect_grids)
}		/*end set_tri_grid_rect_grids*/

EXPORT	void set_dual_interface_topology(
	TRI_GRID *grid)
{
	INTERFACE *intfc = grid->grid_intfc;
	RECT_GRID *expanded_dual_grid = &grid->rect_grid;
	RECT_GRID *top_grid = &topological_grid(intfc);

	DEBUG_ENTER(set_dual_interface_topology)
	if ((intfc->modified) || no_topology_lists(intfc) ||
		(memcmp((const void*)expanded_dual_grid,
		        (const void*)top_grid,sizeof(RECT_GRID)) != 0))
	{
	    set_topological_grid(intfc,expanded_dual_grid); 
	    no_topology_lists(intfc) = NO;

	    if (!make_interface_topology_lists(intfc))
	    {
	        screen("ERROR in set_dual_interface_topology(), "
		       "make_interface_topology_lists() failed\n");
	        clean_up(ERROR);
	    }
	}
	DEBUG_LEAVE(set_dual_interface_topology)
}		/*end set_dual_interface_topology*/


EXPORT	int **set_node_index_list(
	TRI_GRID	*grid)
{
	INTERFACE	*intfc = grid->grid_intfc;
	NODE		**n;
	RECT_GRID	*expanded_dual_grid = &grid->rect_grid;
	double		s, *posn, *h = expanded_dual_grid->h;
	double		ntol[MAXD];
	int		**ic_of_node = NULL;
	int		is, i, n_index;
	int		dim = intfc->dim;
	DEBUG_ENTER(set_node_index_list)

	/* count the nodes */
	for (n_index = 0, n = intfc->nodes; n && *n; ++n)
	    ++n_index;
	bi_array(&ic_of_node,n_index,dim,INT);

	for (i = 0; i < dim; ++i)
	    ntol[i] = IG_NTOL * h[i];
	for (n_index = 0, n = intfc->nodes;  n && *n;  ++n, ++n_index)
	{
	    posn = Coords((*n)->posn);
	    for (i = 0; i < dim; ++i)
	    {
	        is = ic_of_node[n_index][i] = 
		    cell_index(posn[i],i,expanded_dual_grid);
		s = (posn[i] - expanded_dual_grid->edges[i][is]) /
		    cell_width(is,i,expanded_dual_grid);
		if (s < IG_NTOL)
		{
		    posn[i] += ntol[i];/*TOLERANCE*/
		    if (debugging("tolerance"))
		        (void) printf("TOLERANCE set_node_index_list() "
				      "IG_NTOL = %g\n",IG_NTOL);
		}
		else if (s > IG_ONEMNTOL)
		{
		    posn[i] -= ntol[i];/*TOLERANCE*/
		    if (debugging("tolerance"))
		        (void) printf("TOLERANCE set_node_index_list() "
				      "IG_ONEMNTOL = %g\n",IG_ONEMNTOL);
		}
	    }
	}
	DEBUG_LEAVE(set_node_index_list)
	return ic_of_node;
}		/*end set_node_index_list*/

/*ARGSUSED*/
EXPORT	void init_triangulation_storage(
	TRI_GRID	*ntg,
	int		**ic_of_node,
	size_t		sizest)
{
	RECT_GRID	*expanded_dual_grid = &ntg->rect_grid;
	int		j,n_reg_nodes;
	int		i,dim = expanded_dual_grid->dim;
	int		*gmax = expanded_dual_grid->gmax;
	register Locstate *state;
	register byte	  *storage;
	Table		*T = table_of_interface(ntg->grid_intfc);
	DEBUG_ENTER(init_triangulation_storage)

	print_storage("before init_triangulation_storage","TRI_storage");
#if defined(DEBUG_TRI_GRID)
	if (debugging("TRI_storage"))
	{
	    (void) printf("\n\tBefore set triangulation storage\n");
	    print_trigrid_alloc_status("",ntg);
	}
#endif /* defined(DEBUG_TRI_GRID) */

#if defined(USE_OVERTURE)
        use_overture_init_triangulation_storage(ntg,ic_of_node,sizest);
        DEBUG_LEAVE(init_triangulation_storage)
        return;
#endif /* if defined(USE_OVERTURE)  */

	n_reg_nodes = gmax[0] + 1;
	for (i = 1; i < dim; ++i)
	    n_reg_nodes *= (gmax[i] + 1);
	ntg->n_node_points = ntg->n_reg_nodes = n_reg_nodes;

	switch (dim)
	{
#if defined(ONED)
	case 1:
	    set_crx_structure_storage1d(ntg);
	    break;
#endif /* defined(ONED) */
#if defined(TWOD)
	case 2:
	    set_crx_structure_storage2d(ntg,ic_of_node);
	    break;
#endif /* defined(TWOD) */
#if defined(THREED)
	case 3:
	    set_crx_structure_storage3d(ntg);
	    break;
#endif /* defined(THREED) */
	}

			/* allocate comp/state storage */
	uni_array(&T->components,n_reg_nodes,sizeof(COMPONENT));
	ntg->cg_comps = T->components + ntg->node_offset;
	if (sizest != 0)
	{
	    alloc_states_array(ntg,n_reg_nodes);
	    VECTOR(ntg,rect_state_storage,n_reg_nodes,sizest);
	}

		    /* storage for regular points */

	state = ntg->states;
	storage = ntg->rect_state_storage;
	j = 0;
	switch (dim)
	{
#if defined(ONED)
	case 1:
	{
	    register double	*xx_grid = expanded_dual_grid->edges[0];
	    int			xmax = gmax[0];
	    int			ix;

	    for (ix = 0;  ix <= xmax;  ++ix)
	    {
	    	Coords(ntg->node_points+j)[0] = xx_grid[ix];
	    	++j;
	
		if (sizest != 0)
		{
	    	    *state++ = (Locstate) storage;
	    	    storage += sizest;
		}
	    }
	}
		break;
#endif /* defined(ONED) */
#if defined(TWOD)
	case 2:
	{
	    register double y;
	    register double *xx_grid = expanded_dual_grid->edges[0];
	    register double *yy_grid = expanded_dual_grid->edges[1];
	    int		   xmax = gmax[0],ymax = gmax[1];
	    int		   ix,iy;

	    for (iy = 0;  iy <= ymax;  ++iy)
	    {
	    	y = yy_grid[iy];
	    	for (ix = 0;  ix <= xmax;  ++ix)
	    	{
	    	    Coords(ntg->node_points+j)[0] = xx_grid[ix];
	    	    Coords(ntg->node_points+j)[1] = y;
	    	    ++j;
    
		    if (sizest != 0)
		    {
    		        *state++ = (Locstate) storage;
    		        storage += sizest;
		    }
    		}
    	    }
	}
	    break;
#endif /* defined(TWOD) */
#if defined(THREED)
	case 3:
	{
	    register double y,z;
	    register double *xx_grid = expanded_dual_grid->edges[0];
	    register double *yy_grid = expanded_dual_grid->edges[1];
	    register double *zz_grid = expanded_dual_grid->edges[2];
	    int		   xmax = gmax[0],ymax = gmax[1],zmax = gmax[2];
	    int		   ix,iy,iz;

	    for (iz = 0;  iz <= zmax;  ++iz)
	    {
	    	z = zz_grid[iz];
	    	for (iy = 0;  iy <= ymax;  ++iy)
	    	{
	    	    y = yy_grid[iy];
	    	    for (ix = 0;  ix <= xmax;  ++ix)
	    	    {
	    	        Coords(ntg->node_points+j)[0] = xx_grid[ix];
	    	        Coords(ntg->node_points+j)[1] = y;
	    	        Coords(ntg->node_points+j)[2] = z;
	    	        ++j;

			if (sizest != 0)
			{
	    	            *state++ = (Locstate) storage;
	    	            storage += sizest;
			}
	    	    }
	    	}
	    }
	}
	    break;
#endif /* defined(THREED) */
	}
	print_storage("after init_triangulation_storage","TRI_storage");
	DEBUG_LEAVE(init_triangulation_storage)
}		/*end init_triangulation_storage*/


#if defined(DEBUG_TRI_GRID)


/*
*		Routines for debugging the grid construction:
*/

#define NOBINARY
#include <plotdecs.h>

#undef PLOTFILE
#define PLOTFILE plotfile

LOCAL	FILE *plotfile = NULL;

LOCAL	void open_plot_file(void)
{
	char filename[15];
	int  nn;

	if (plotfile != NULL)
		return;

	nn = pp_numnodes();
	if (pp_numnodes() == 1)
	    (void) sprintf(filename,"trigrid.plt");
	else
	{
	    int nd, myid = pp_mynode();
	    for (nd = 0; nn != 0; nn /=10, ++nd);
	    (void) sprintf(filename,"trigrid.plt.%s",right_flush(myid,nd));
	}

	if ((plotfile = fopen(filename,"w")) == NULL)
	{
	    screen("ERROR in open_plot_file(), can't open %s\n",filename);
	    clean_up(ERROR);
	}
	if (debugging("nobuf"))
	    setbuf(plotfile,NULL);
	openpl();
}		/*end open_plot_file*/



LOCAL	void set_plot_window(
	RECT_GRID	*gr)
{
	window(gr->L[0] - 0.5*cell_width(0,0,gr),
	       gr->L[1] - 0.5*cell_width(0,1,gr),
	       gr->U[0] + 0.5*cell_width(gr->gmax[0]-1,0,gr),
	       gr->U[1] + 0.5*cell_width(gr->gmax[1]-1,1,gr));
	viewport(0.05,0.05,0.95,0.95);
}		/*end set_plot_window*/



LOCAL	void end_frame(void)
{
	erase();
}		/*end end_frame*/



LOCAL	void plot_intfc(
	INTERFACE	*intfc)
{
	BOND	*b;
	CURVE	**c;

	switch (intfc->dim)
	{
#if defined(ONED)
	case 1:
		break;
#endif /* defined(ONED) */
#if defined(TWOD)
	case 2:
		for (c = intfc->curves; c && *c;  ++c)
		{
			set_color_from_table(negative_component(*c) + 2);
			move(Coords((*c)->first->start)[0],
			     Coords((*c)->first->start)[1]);
			for (b = (*c)->first;  b != NULL;  b = b->next)
				cont(Coords(b->end)[0],Coords(b->end)[1]);
		}
		break;
#endif /* defined(TWOD) */
#if defined(THREED)
	case 3:
		break;
#endif /* defined(THREED) */
	}
}		/*end plot_intfc*/



LOCAL	void plot_grid_lines(
	RECT_GRID	*gr)
{
	double		L, U;
	double		*p_grid;
	int		i, imax;
	int		dim = gr->dim;

	switch (dim)
	{
#if defined(ONED)
	case 1:
		break;
#endif /* defined(ONED) */
#if defined(TWOD)
	case 2:
		set_color_from_table(1);
		imax = gr->gmax[0];	p_grid = gr->edges[0];
		L = gr->L[1];		U = gr->U[1];
		for (i = 0;  i <= imax;  ++i) line(p_grid[i],L,p_grid[i],U);
		imax = gr->gmax[1]; 	p_grid = gr->edges[1];
		L = gr->L[0];		U = gr->U[0];
		for (i = 0;  i <= imax;  ++i) line(L,p_grid[i],U,p_grid[i]);
		break;
#endif /* defined(TWOD) */
#if defined(THREED)
	case 3:
		break;
#endif /* defined(THREED) */
	}
}		/*end plot_grid_lines*/


#if defined(TWOD)
LOCAL	void plot_crossings2d(
	TRI_GRID	*ntg)
{
	Table		*T = table_of_interface(ntg->grid_intfc);
	CRXING		*cr;
	int	 	i;

	if (T->crx_store == NULL)
	{
		double	x, y;

		x = 0.5 * (ntg->rect_grid.U[0] - ntg->rect_grid.L[0]);
		y = 0.5 * (ntg->rect_grid.U[1] - ntg->rect_grid.L[1]);
		move(x,y);
		label("crx_store[] not allocated\n");
		return;
	}

	for (i = 0;  i < ntg->n_crx;  ++i)
	{
		cr = &(T->crx_store[i]);
		move(Coords(cr->pt)[0],Coords(cr->pt)[1]);
		label("+");
	}
}		/*end plot_crossings2d*/
#endif /* defined(TWOD) */
#endif /* defined(DEBUG_TRI_GRID) */

EXPORT	double area_of_domain_with_comp(
	Front *front,
	COMPONENT comp)
{
	TRI_GRID *grid;
	static TRI_GRID_HOOKS tri_grid_hooks;
	static int first = YES;
	int i,j,*gmax,icoords[MAXD];
	double area;

	if (front->rect_grid->dim != 2) return 0.0;

	if (first == YES)
	{
	    zero_scalar(&tri_grid_hooks,sizeof(TRI_GRID_HOOKS));
	    tri_grid_hooks._method = COMPLETE_TRIANGULATION;
	    tri_grid_hooks._construct_tri_grid = t_construct_tri_grid;
	    tri_grid_hooks._set_components = set_components2d;
	    tri_grid_hooks._blk_triangulate = exact_blk_triangulate;
	    tri_grid_hooks._triangulate_mesh = triangulate_mesh2d;
	    first = NO;
	}
	grid = allocate_tri_grid(&tri_grid_hooks);
	construct_tri_grid(grid,front->rect_grid,front);

	gmax = grid->rect_grid.gmax;
	area = 0.0;
	for (i = 0; i < gmax[0]; ++i)
	{
	    icoords[0] = i;
	    for (j = 0; j < gmax[1]; ++j)
	    {
		icoords[1] = j;
	    	area += grid->area(icoords,comp,grid);
	    }
	}
	free_tri_grid(grid);
	return area;
}	/* end area_of_domain_with_comp */

