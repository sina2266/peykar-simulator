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
*			tricpy.c
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*/


#include <tri/trilocaldecs.h>


	/* LOCAL Function Declarations */
#if defined(TWOD)
LOCAL	void	copy_tri_storage2d(TRI_GRID*,TRI_GRID*,size_t);
#endif /* defined(TWOD) */
#if defined(THREED)
LOCAL	void	copy_tri_storage3d(TRI_GRID*,TRI_GRID*,size_t);
#endif /* defined(THREED) */
LOCAL	void	set_triangulation_storage(TRI_GRID*,size_t);

EXPORT	void copy_tri_grid(
	TRI_GRID	*ogrid,
	TRI_GRID	*ngrid,
	size_t		sizest)
{
	int		dim = ogrid->rect_grid.dim;
	int 		i;

	ngrid->tri_grid_hooks = ogrid->tri_grid_hooks;
	copy_rect_grid(&ngrid->rect_grid,&ogrid->rect_grid);
	copy_rect_grid(&ngrid->comp_grid,&ogrid->comp_grid);
	copy_rect_grid(&ngrid->tg_grid,&ogrid->tg_grid);
	ngrid->_locate_on_trigrid     = ogrid->_locate_on_trigrid;
	ngrid->area       = ogrid->area;
	ngrid->grid_intfc = ogrid->grid_intfc;
	ngrid->node_offset = ogrid->node_offset;
	ngrid->cell_offset = ogrid->cell_offset;
	for (i = 0; i < 1<<dim; ++i)
	    ngrid->offset[i] = ogrid->offset[i];
	ngrid->nnx = ogrid->nnx;
	ngrid->nny = ogrid->nny;
	ngrid->nnz = ogrid->nnz;

	ngrid->n_crx   = ogrid->n_crx;
	ngrid->n_reg_nodes = ogrid->n_reg_nodes;
	ngrid->n_nodes = ogrid->n_nodes;
	/*ngrid->seg_crx_count = ogrid->seg_crx_count; */
	/*ngrid->seg_crx_lists = ogrid->seg_crx_lists; */
	/*ngrid->seg_crx_lists_store = ogrid->seg_crx_lists_store; */
	/*ngrid->crx_store     = ogrid->crx_store; */

	ngrid->n_lin_els  = ogrid->n_lin_els;
	ngrid->n_bilin_els = ogrid->n_bilin_els;
	ngrid->n_pcs = ogrid->n_pcs;
	ngrid->n_tg_pts = ogrid->n_tg_pts;
	ngrid->n_fr_pts = ogrid->n_fr_pts;

	
		/* alloc statements below dangerous as two copies */
		/*  now point to same storage. Handle with care.  */

	/*ngrid->alloc.seg_crx_count = 1; */
	/*ngrid->alloc.seg_crx_lists = 1; */
	/*ngrid->alloc.seg_crx_lists_store = 1; */
	/*ngrid->alloc.crx_store     = 1; */

#if defined(USE_OVERTURE)
        set_AMR_triangulation_storage(ngrid,sizest);
#else
        set_triangulation_storage(ngrid,sizest);
#endif /* defined(USE_OVERTURE) */

	copy_node_points(ngrid,ogrid);
	ngrid->front_points = ogrid->front_points;
	ngrid->alloc.front_points = YES;
	ngrid->cg_comps = ogrid->cg_comps;
	/*copy_trigrid_components(ngrid,ogrid); */

	ngrid->bilin_els = ogrid->bilin_els;
	if (ngrid->bilin_els != NULL)
		ngrid->alloc.bilin_els = YES;

	switch (dim)
	{
#if defined(ONED)
	case 1: /* TODO */
		break;
#endif /* defined(ONED) */
#if defined(TWOD)
	case 2:
#if defined(USE_OVERTURE)
                copy_AMR_tri_storage2d(ogrid,ngrid,sizest);
#else
                copy_tri_storage2d(ogrid,ngrid,sizest);
#endif /* defined(USE_OVERTURE) */
		break;
#endif /* defined(TWOD) */
#if defined(THREED)
	case 3:
		copy_tri_storage3d(ogrid,ngrid,sizest);
		break;
#endif /* defined(THREED) */
	}

}		/*end copy_tri_grid*/


EXPORT	void free_copy_tri_grid(
	TRI_GRID	**ptri_grid)
{
	TRI_GRID	*tri_grid;

	if (ptri_grid == NULL) return;
	tri_grid = *ptri_grid;
	if (tri_grid == NULL) return;

	tri_grid->grid_intfc = NULL;

	/*
	*  The node_points, front_points, bilin_els, and components  
	*  storage are reused in the newwave and should not be freed here
	*/
	tri_grid->node_points = NULL;	tri_grid->cg_npts = NULL;
	tri_grid->alloc.node_points = NO;
	tri_grid->front_points = NULL;
	tri_grid->alloc.front_points = NO;
	tri_grid->bilin_els = NULL;
	tri_grid->alloc.bilin_els = NO;
	/*tri_grid->components = NULL;	tri_grid->cg_comps = NULL; */
	tri_grid->alloc.components = NO;

	free_triangulation_storage(tri_grid);
	free(tri_grid);
	*ptri_grid = NULL;
}		/*end free_copy_tri_grid*/


LOCAL	void set_triangulation_storage(
	TRI_GRID	*ntg,
	size_t		sizest)
{
	register RECT_GRID	*r_gr = &ntg->rect_grid;
	register Locstate	*state;
	register byte	 	*storage;

	int		n_reg_nodes;
	int		i, dim = r_gr->dim;
	int		*gmax = r_gr->gmax;
#if defined(TWOD) || defined(THREED)
	BLK_EL0		*blk_el0;
	static BLK_EL0	CLEAR_BLK_EL0;
#endif /* defined(TWOD) || defined(THREED) */
#if defined(TWOD)
	BLK_EL1		*blk_el1;
	static BLK_EL1	CLEAR_BLK_EL1;
#endif /* defined(TWOD) */

	print_storage("before set_triangulation_storage","TRI_storage");
#if defined(DEBUG_TRI_GRID)
	if (debugging("TRI_storage"))
	{
	    (void) printf("\n\tBefore set triangulation storage\n");
	    print_trigrid_alloc_status("",ntg);
	}
#endif /* defined(DEBUG_TRI_GRID) */

	n_reg_nodes = gmax[0] + 1;
	for (i = 1; i < dim; ++i) n_reg_nodes *= (gmax[i] + 1);
#if defined(DEBUG_TRI_GRID)
	if (debugging("TRI_storage"))
	{
	    print_int_vector("gmax = ",gmax,dim,"\n");
	    (void) printf("n_reg_nodes = %d\n",n_reg_nodes);
	}
#endif /* defined(DEBUG_TRI_GRID) */


			/* allocate comp/state storage */

	alloc_states_array(ntg,n_reg_nodes);
	VECTOR(ntg,rect_state_storage,n_reg_nodes,sizest);

	switch (dim)
	{
#if defined(ONED)
	case 1:
	{
		int xmax = gmax[0];
		int ix;

		    /* storage for regular points */
		/* associate storage with states at regular grid nodes */

		state = ntg->states;
		storage = ntg->rect_state_storage;
		for (ix = 0;  ix <= xmax;  ++ix)
		{
			*state++ = (Locstate) storage;
			storage += sizest;
		}
	}
		break;
#endif /* defined(ONED) */
#if defined(TWOD)
	case 2:
	{
	    int xmax = gmax[0], ymax = gmax[1];
	    int ix, iy;

	    VECTOR(ntg,lin_els,ntg->n_lin_els,sizeof(LINEAR_ELEMENT));
	    if (interpolation_method(ntg) == ELEMENT_ON_THE_FLY)
	    	VECTOR(ntg,pcs,ntg->n_pcs,sizeof(POINT_COMP_ST));
	    alloc_blk_els0(ntg,ymax*xmax);
	    alloc_blk_els1(ntg,ymax*xmax);
#if defined(DEBUG_TRI_GRID)
	    if (debugging("TRI_storage"))
	    {
	    	(void) printf("n_crx %d, ",ntg->n_crx);
	    	(void) printf("storage set for %d triangles\n",ntg->n_lin_els);
	    }
#endif /* defined(DEBUG_TRI_GRID) */

		    /* storage for regular points */
	/* associate storage with states at regular grid nodes */

	    state = ntg->states;
	    storage = ntg->rect_state_storage;
	    blk_el0 = ntg->blk_els0;	blk_el1 = ntg->blk_els1;
	    for (iy = 0;  iy <= ymax;  ++iy)
	    {
	    	for (ix = 0;  ix <= xmax;  ++ix)
	    	{
	    	    *state++ = (Locstate) storage;
	    	    storage += sizest;

	    	    if ((ix == xmax) || (iy == ymax))
	    	    	continue;
	    	    *blk_el1++ = CLEAR_BLK_EL1;
	    	    *blk_el0++ = CLEAR_BLK_EL0;
	    	}
	    }
	}
	    break;
#endif /* defined(TWOD) */
#if defined(THREED)
	case 3:
	{
		int xmax = gmax[0], ymax = gmax[1], zmax = gmax[2];
		int ix, iy, iz;

		if (interpolation_method(ntg) == COMPLETE_TRIANGULATION)
		    VECTOR(ntg,lin_els,ntg->n_lin_els,sizeof(LINEAR_ELEMENT));
		if (interpolation_method(ntg) == ELEMENT_ON_THE_FLY)
		    VECTOR(ntg,pcs,ntg->n_pcs,sizeof(POINT_COMP_ST));
		alloc_blk_els0(ntg,zmax*ymax*xmax);
#if defined(DEBUG_TRI_GRID)
		if (debugging("TRI_storage"))
		{
			(void) printf("n_crx %d, ",ntg->n_crx);
			(void) printf("storage set for %d tetras\n",
				      ntg->n_lin_els);
		}
#endif /* defined(DEBUG_TRI_GRID) */

		    /* storage for regular points */
		/* associate storage with states at regular grid nodes */

		state = ntg->states;
		storage = ntg->rect_state_storage;
		blk_el0 = ntg->blk_els0;
		for (iz = 0;  iz <= zmax;  ++iz)
		{
			for (iy = 0;  iy <= ymax;  ++iy)
			{
				for (ix = 0;  ix <= xmax;  ++ix)
				{
					*state++ = (Locstate) storage;
					storage += sizest;
					if ((ix == xmax) || (iy == ymax)
							 || (iz == zmax))
						continue;
					*blk_el0++ = CLEAR_BLK_EL0;
				}
			}
		}
	}
		break;
#endif /* defined(THREED) */
	}
	print_storage("after set_triangulation_storage","TRI_storage");
}		/*end set_triangulation_storage*/

#if defined(TWOD)
LOCAL	void	copy_tri_storage2d(
	TRI_GRID	*ogrid,
	TRI_GRID	*ngrid,
	size_t		sizest)
{
	BLK_EL0	  	*nblk,     *oblk;
	LINEAR_ELEMENT  *nlin,     *olin;
	byte		*nstorage, *ostorage, *end_ostr;
	int		*gmax = ogrid->rect_grid.gmax;
	int		xmax = gmax[0], ymax = gmax[1];
	int		j, ix, iy;
	int		i, dim = ogrid->rect_grid.dim;

	nstorage = ngrid->rect_state_storage;
	ostorage = ogrid->rect_state_storage;
	end_ostr = ogrid->rect_state_storage + (xmax+1)*(ymax+1)*sizest;

	if (interpolation_method(ogrid) == COMPLETE_TRIANGULATION)
	{
	    for (j = 0, olin = ogrid->lin_els, nlin = ngrid->lin_els;
				j < ogrid->n_lin_els;  ++j, ++olin, ++nlin)
	    {
		nlin->comp   = olin->comp;
		for (i = 0; i < dim+1; ++i)
		{
		    nlin->side[i] = olin->side[i];
		    nlin->p[i] = olin->p[i];
		    if ((olin->s[i] >= ((Locstate) ostorage)) && 
			(olin->s[i] < ((Locstate) end_ostr)))
		    {
	                nlin->s[i] = (Locstate)(nstorage +
					(((byte *) olin->s[i]) - ostorage));
		    }
		    else
			nlin->s[i] = olin->s[i];
		}
	    }

	    nlin   = ngrid->lin_els;	olin   = ogrid->lin_els;

	    nblk = ngrid->blk_els0;
	    oblk = ogrid->blk_els0;
	    for (iy = 0;  iy < ymax;  ++iy)
	    {
	        for (ix = 0;  ix < xmax;  ++ix, ++nblk, ++oblk)
	        {
	            if (blk_el0_is_bilinear(oblk))
	            {
	    	        set_bilinear_blk_el0(nblk);
	                blk_el0_bilinear_el(nblk) = blk_el0_bilinear_el(oblk);
	            }
	            else
	            {
	                num_lin_els_in_blk(nblk) = num_lin_els_in_blk(oblk);
	        	blk_el0_linear_els(nblk) = nlin
	        			+ (blk_el0_linear_els(oblk) - olin);
	            }
	        }
	    }
	}
	if (interpolation_method(ogrid) == ELEMENT_ON_THE_FLY)
	{
	    POINT_COMP_ST    *npcs, *opcs;
	    for (j = 0, opcs = ogrid->pcs, npcs = ngrid->pcs;
	                        j < ogrid->n_pcs;  ++j, opcs++, ++npcs)
	    {
	        npcs->p = opcs->p;
	        for (i = 0; i < 2; ++i)
	        {
	            npcs->comp[i] = opcs->comp[i];
	            if ((opcs->s[i] >= ((Locstate) ostorage)) && 
	                (opcs->s[i] < ((Locstate) end_ostr)))
	            {
	                npcs->s[i] = (Locstate)(nstorage +
	                        (((byte *) opcs->s[i]) - ostorage));
	            }
	            else
	                npcs->s[i] = opcs->s[i];
	        }
	    }
	    npcs   = ngrid->pcs; opcs   = ogrid->pcs;
	    nblk = ngrid->blk_els0;
	    oblk = ogrid->blk_els0;
	    for (iy = 0; iy < ymax; ++iy)
	    for (ix = 0; ix < xmax; ++ix, ++nblk, ++oblk)
	    {
	        num_lin_els_in_blk(nblk) = num_lin_els_in_blk(oblk);
	        blk_el0_pcs_els(nblk) = npcs + (blk_el0_pcs_els(oblk) - opcs);
	    }
	}

#if defined(DEBUG_TRI_GRID)
	if (debugging("print_copy_tri"))
		debug_copy_2d_tri_grid(ogrid,ngrid,sizest);
#endif /* defined(DEBUG_TRI_GRID) */
}		/*end copy_tri_storage2d*/

#if defined(DEBUG_TRI_GRID)
/*
*			debug_copy_2d_tri_grid():
*
*	Copy old rect_state_storage into new_rect_state_storage 
*	and ensure that same states are printed out by accessing
*	either old or new grid structures.
*/

EXPORT	void debug_copy_2d_tri_grid(
	TRI_GRID	*ogrid,
	TRI_GRID	*ngrid,
	size_t		sizest)
{
	BLK_EL0		*nblk,     *oblk;
	LINEAR_ELEMENT  *nlin,  *olin;
	BILINEAR_ELEMENT  *nbilin, *obilin;
	RECT_GRID	*nrg = &ngrid->rect_grid;
	RECT_GRID	*org = &ogrid->rect_grid;
	TG_PT	 	*nnode, *onode;
	byte		*ostorage, *nstorage;
	int		ix, iy, j;
	int		xmax, ymax;
	int		i, imax;

	xmax = org->gmax[0];	ymax = org->gmax[1];
	ostorage = ogrid->rect_state_storage;
	nstorage = ngrid->rect_state_storage;
	imax = (int)((xmax+1)*(ymax+1)*sizest);
	for (i = 0;  i < imax;  ++i)
	    *(nstorage++) = *(ostorage++);

	(void) printf("\n\tcopy_tri_grid printout\n\n");
	(void) printf("%-21s %-10s %-10s\n","","newgrid","oldgrid");
	(void) printf("%-21s %-10p %-10p\n","rect_grid",
		      (POINTER)&ngrid->rect_grid,
		      (POINTER)&ogrid->rect_grid);
	(void) printf("%-21s %-10p %-10p\n","locate",
		      ngrid->_locate_on_trigrid,
		      ogrid->_locate_on_trigrid);
	(void) printf("%-21s %-10p %-10p\n","area",
		      ngrid->area,
		      ogrid->area);
	(void) printf("%-21s %-10p %-10p\n","intfc",
		      (POINTER)ngrid->grid_intfc,
		      (POINTER)ogrid->grid_intfc);
	(void) printf("%-21s %-10d %-10d\n","n_crx",ngrid->n_crx,ogrid->n_crx);
	(void) printf("%-21s %-10d %-10d\n","n_nodes",
		      ngrid->n_nodes,ogrid->n_nodes);
	/*
	(void) printf("%-21s %-10p %-10p\n","seg_crx_count",
		      (POINTER)ngrid->seg_crx_count,
		      (POINTER)ogrid->seg_crx_count);
	(void) printf("%-21s %-10p %-10p\n","nseg_crx_lists",
		      (POINTER)ngrid->seg_crx_lists,
		      (POINTER)ogrid->seg_crx_lists);
	(void) printf("%-21s %-10p %-10p\n","seg_crx_lists_store",
		      (POINTER)ngrid->seg_crx_lists_store,
		      (POINTER)ogrid->seg_crx_lists_store);
	(void) printf("%-21s %-10p %-10p\n","crx_store",
		      (POINTER)ngrid->crx_store,
		      (POINTER)ogrid->crx_store);
	*/
	(void) printf("%-21s %-10d %-10d\n","n_lin_els",
		      ngrid->n_lin_els,ogrid->n_lin_els);
	(void) printf("%-21s %-10d %-10d\n","n_bilin_els",
		      ngrid->n_bilin_els,ogrid->n_bilin_els);
	(void) printf("%-21s %-10p %-10p\n","lin_els",
		      (POINTER)ngrid->lin_els,
		      (POINTER)ogrid->lin_els);
	(void) printf("%-21s %-10p %-10p\n","bilin_els",
		      (POINTER)ngrid->bilin_els,
		      (POINTER)ogrid->bilin_els);
	(void) printf("%-21s %-10p %-10p\n","node_points",
		      (POINTER)ngrid->node_points,
		      (POINTER)ogrid->node_points);
	(void) printf("%-21s %-10p %-10p\n","cg_npts",
		      (POINTER)ngrid->cg_npts,
		      (POINTER)ogrid->cg_npts);
	(void) printf("%-21s %-10p %-10p\n","blk_els0",
		      (POINTER)ngrid->blk_els0,
		      (POINTER)ogrid->blk_els0);
	(void) printf("%-21s %-10p %-10p\n","cg_blk_els0",
		      (POINTER)ngrid->cg_blk_els0,
		      (POINTER)ogrid->cg_blk_els0);
	(void) printf("%-21s %-10p %-10p\n","blk_els1",
		      (POINTER)ngrid->blk_els1,
		      (POINTER)ogrid->blk_els1);
	(void) printf("%-21s %-10p %-10p\n","cg_blk_els1",
		      (POINTER)ngrid->cg_blk_els1,
		      (POINTER)ogrid->cg_blk_els1);
	/*
	(void) printf("%-21s %-10p %-10p\n","components",
		      (POINTER)ngrid->components,
		      (POINTER)ogrid->components);
	*/
	(void) printf("%-21s %-10p %-10p\n","cg_comps",
		      (POINTER)ngrid->cg_comps,
		      (POINTER)ogrid->cg_comps);
	(void) printf("%-21s %-10p %-10p\n","states",
		      (POINTER)ngrid->states,
		      (POINTER)ogrid->states);
	(void) printf("%-21s %-10p %-10p\n","cg_states",
		      (POINTER)ngrid->cg_states,
		      (POINTER)ogrid->cg_states);
	(void) printf("%-21s %-10p %-10p\n","rect_state_storage",
		      (POINTER)ngrid->rect_state_storage,
		      (POINTER)ogrid->rect_state_storage);
	
	print_trigrid_alloc_status("ngrid",ngrid);
	print_trigrid_alloc_status("ogrid",ogrid);

	(void) printf("New RECT_GRID\n");	print_rectangular_grid(nrg);
	(void) printf("\n\nOld RECT_GRID\n");	print_rectangular_grid(org);

	(void) printf("\n\tnode_points\n%-5s %-14s %-14s    %-14s %-14s\n",
		      "index","xnew","xold","ynew","yold");
	for (j = 0, nnode = ngrid->node_points, onode = ogrid->node_points;
	     j < ogrid->n_node_points;
	     ++j, ++nnode, ++onode)
		(void) printf("%-5d %-14g %-14g    %-14g %-14g\n",
			      j,Coords(nnode)[0],Coords(onode)[0],
			        Coords(nnode)[1],Coords(onode)[1]);

	(void) printf("\n	LINEAR_ELEMENT\n");
	for (j = 0, olin = ogrid->lin_els, nlin = ngrid->lin_els;
	     j < ogrid->n_lin_els;
	     ++j, ++olin, ++nlin)
	{
	    print_LINEAR_ELEMENT(nlin,ngrid);
	    print_LINEAR_ELEMENT(olin,ogrid);
	    (void) printf("\n");
	}

	(void) printf("\n	BILINEAR_ELEMENT\n");
	nnode = ngrid->node_points;	onode =  ogrid->node_points;
	for (j = 0, obilin = ogrid->bilin_els, nbilin = ngrid->bilin_els;
			j < ogrid->n_bilin_els;  ++j, ++obilin, ++nbilin)
	{
	    print_BILINEAR_ELEMENT(nbilin,ngrid);
	    print_BILINEAR_ELEMENT(obilin,ogrid);
	    (void) printf("\n");
	}

	(void) printf("\n	COMPONENTS\n");
	print_components(ngrid);
	print_components(ogrid);

	(void) printf("\n	BLK_ELS0\n");
	nblk = ngrid->blk_els0;	oblk = ogrid->blk_els0;
	for (iy = 0;  iy < ymax;  ++iy)
	{
	    for (ix = 0;  ix < xmax;  ++ix)
	    {
	        (void) printf("ix %d iy %d\n",ix,iy);
	        print_BLK_EL0(nblk++,ngrid);
	        print_BLK_EL0(oblk++,ogrid);
	        (void) printf("\n");
	    }
	}
}		/*end debug_copy_2d_tri_grid*/
#endif /* defined(DEBUG_TRI_GRID) */
#endif /* defined(TWOD) */

#if defined(THREED)
LOCAL	void	copy_tri_storage3d(
	TRI_GRID	*ogrid,
	TRI_GRID	*ngrid,
	size_t		sizest)
{
	BLK_EL0		*nblk,     *oblk;
	byte		*nstorage, *ostorage, *end_ostr;
	int		*gmax = ogrid->rect_grid.gmax;
	int		xmax = gmax[0], ymax = gmax[1], zmax = gmax[2];
	int		j, ix, iy, iz;
	int		i, dim = ogrid->rect_grid.dim;

	nstorage = ngrid->rect_state_storage;
	ostorage = ogrid->rect_state_storage;
	end_ostr = ogrid->rect_state_storage + 
		(xmax+1)*(ymax+1)*(zmax+1)*sizest;

	if (interpolation_method(ogrid) == COMPLETE_TRIANGULATION)
	{
	    LINEAR_ELEMENT  *nlin,     *olin;
	    for (j = 0, olin = ogrid->lin_els, nlin = ngrid->lin_els;
				j < ogrid->n_lin_els;  ++j, ++olin, ++nlin)
	    {
		nlin->comp   = olin->comp;
		for (i = 0; i < dim+1; ++i)
		{
		    nlin->side[i] = olin->side[i];
		    nlin->p[i] = olin->p[i];
		    if ((olin->s[i] >= ((Locstate) ostorage)) && 
		        (olin->s[i] < ((Locstate) end_ostr)))
		    {
	                nlin->s[i] = (Locstate)(nstorage +
					(((byte *) olin->s[i]) - ostorage));
		    }
		    else
		    	nlin->s[i] = olin->s[i];
		}
	    }
	    nlin   = ngrid->lin_els;	olin   = ogrid->lin_els;
	    nblk = ngrid->blk_els0;
	    oblk = ogrid->blk_els0;
	    for (iz = 0; iz < zmax; ++iz)
	    for (iy = 0; iy < ymax; ++iy)
	    for (ix = 0; ix < xmax; ++ix, ++nblk, ++oblk)
	    {
	        if (blk_el0_is_bilinear(oblk))
	        {
		    set_bilinear_blk_el0(nblk);
		    blk_el0_bilinear_el(nblk) = blk_el0_bilinear_el(oblk);
	        }
	        else
		{
	            num_lin_els_in_blk(nblk) = num_lin_els_in_blk(oblk);
		    blk_el0_linear_els(nblk) =
			nlin + (blk_el0_linear_els(oblk) - olin);
		}
	    }
	}
	if (interpolation_method(ogrid) == ELEMENT_ON_THE_FLY)
	{
	    POINT_COMP_ST	*npcs,	   *opcs;
	    for (j = 0, opcs = ogrid->pcs, npcs = ngrid->pcs;
				j < ogrid->n_pcs;  ++j, ++opcs, ++npcs)
	    {
		npcs->p = opcs->p;
		for (i = 0; i < 2; ++i)
		{
		    npcs->comp[i] = opcs->comp[i];
		    if ((opcs->s[i] >= ((Locstate) ostorage)) && 
			(opcs->s[i] < ((Locstate) end_ostr)))
		    {
		   	npcs->s[i] = (Locstate)(nstorage +
				(((byte *) opcs->s[i]) - ostorage));
		    }
		    else
			npcs->s[i] = opcs->s[i];
		}
	    }
	    npcs   = ngrid->pcs;	opcs   = ogrid->pcs;
	    nblk = ngrid->blk_els0;
	    oblk = ogrid->blk_els0;
	    for (iz = 0; iz < zmax; ++iz)
	    for (iy = 0; iy < ymax; ++iy)
	    for (ix = 0; ix < xmax; ++ix, ++nblk, ++oblk)
	    {
		num_pcs_els_in_blk(nblk) = num_pcs_els_in_blk(oblk);
		blk_el0_pcs_els(nblk) = npcs + (blk_el0_pcs_els(oblk) - opcs);
	    }
	}
}		/*end copy_tri_storage3d*/
#endif /* defined(THREED) */
