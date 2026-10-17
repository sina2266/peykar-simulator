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
*				tripcs.c
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*/

#if defined(TWOD) || defined(THREED)

#include <tri/trilocaldecs.h>

/* offsets:
*   in order to be able to work consistently with the 
*    bilinear interpolation routines, it is important
*    that offsets are defined correctly:
* 
*    TWOD:
* 	
* 	      vertices on bilinear elements are numbered as follows
* 	
* 	                                 x
* 	      	      1-------2          ^
* 	              |       |          |
* 	              |       |          |
* 	              |       |          |
* 	              0-------3          -------->y
* 	                                 
* 	
*    THREED:
* 	
* 	      vertices on bilinear elements are numbered as follows
* 	
* 	                                 z
* 	      	      1-------3          ^
* 	             /|      /|          |
* 	            / |     / |          |
* 	           /  |    /  |          |
* 	          /   0---/---2          -------->y
* 	         /   /   /   /          /
*	        5-------7   /          /
*	        |  /    |  /          /
* 	        | /     | /          /
* 	        |/      |/         \/
* 	        4-------6          x
* 	
* 
*/ 
	/* LOCAL Function Declarations */
#if defined(TWOD)
LOCAL	void    add_intfc_blk_pcs2d(int,BOND**,CURVE**,BLK_EL0*,P_LINK*,int);
LOCAL   int     count_num_pcs2d(TRI_GRID*);
#endif /* defined(TWOD) */
 
#if defined(THREED)
LOCAL	void    add_intfc_blk_pcs3d(int,TRI**,SURFACE**,BLK_EL0*,P_LINK*,int);

#endif /* defined(THREED) */

LIB_LOCAL 	void copy_tg_pts_from_intfc(
	TRI_GRID *ntg,
	P_LINK   *hash_table,
	int      h_size)
{
#if defined(TWOD) || defined(THREED)
	TG_PT     *fr_pt;
	INTERFACE *intfc = ntg->grid_intfc;
	FT_POINT     *point;
#endif /* defined(TWOD) || defined(THREED) */

	reset_hash_table(hash_table,h_size);

#if defined(TWOD)
	if (intfc->dim == 2)
	{
	    BOND  *b;
	    CURVE **c;
	    NODE  **n;

	    fr_pt = ntg->front_points;
	    for (n = intfc->nodes; n && *n; ++n)
	    {
		point = (*n)->posn;
		Coords(fr_pt)[0] = Coords(point)[0];
		Coords(fr_pt)[1] = Coords(point)[1];
		(void) add_to_hash_table((POINTER)point,(POINTER)fr_pt,
			                 hash_table,h_size);
		++fr_pt;
	    }
	    for (c = intfc->curves; c && *c; ++c)
	    {
		for (b = (*c)->first; b != (*c)->last; b = b->next)
		{
		   point = b->end;
		   Coords(fr_pt)[0] = Coords(point)[0];
		   Coords(fr_pt)[1] = Coords(point)[1];
		   (void) add_to_hash_table((POINTER)point,(POINTER)fr_pt,
				            hash_table,h_size);
		   ++fr_pt;
		}
	    }
	}
#endif /* defined(TWOD) */
#if defined(THREED)
	if (intfc->dim == 3)
	{
	    HYPER_SURF         *hs;
	    HYPER_SURF_ELEMENT *hse;

	    fr_pt = ntg->front_points;
	    (void) next_point(intfc,NULL,NULL,NULL);
	    while (next_point(intfc,&point,&hse,&hs))
	    {
	          Coords(fr_pt)[0] = Coords(point)[0];
	          Coords(fr_pt)[1] = Coords(point)[1];
	          Coords(fr_pt)[2] = Coords(point)[2];
	          (void) add_to_hash_table((POINTER)point,(POINTER)fr_pt,
				           hash_table,h_size);
	          ++fr_pt;
	    }
	}
#endif /* defined(THREED) */
}		/*end copy_tg_pts_from_intfc*/

#if defined(TWOD)

LOCAL	void add_intfc_blk_pcs2d(
	int num_bonds,
	BOND **bond,
	CURVE **curve,
	BLK_EL0 *blk_el0,
	P_LINK *hash_table,
	int h_size)
{
	BOND		*b;
	CURVE		*c;
	FT_POINT		*p;
	int		i;
	TG_PT		*tp;
	POINT_COMP_ST	*pcs = blk_el0_pcs_els(blk_el0);
	Locstate	sl,sr;

	/* reset points in block bonds */

	for (i = 0; i < num_bonds; ++i)
	{
	    b = bond[i];
	    sorted(b->start) = NO;
	    sorted(b->end) = NO;
	}

	/* add interface points inside cell */

	for (i = 0; i < num_bonds; ++i)
	{
	    b = bond[i];	c = curve[i];
	    p = b->start;
	    if (sorted(p) == NO)
	    {
		sorted(p) = YES;
		tp = (TG_PT*)find_from_hash_table((POINTER)p,hash_table,
				                  h_size);
		slsr(p,Hyper_surf_element(b),Hyper_surf(c),&sl,&sr);
		pcs[num_pcs_els_in_blk(blk_el0)].p = tp;
		pcs[num_pcs_els_in_blk(blk_el0)].comp[0] =
			negative_component(c);
		pcs[num_pcs_els_in_blk(blk_el0)].comp[1] =
			positive_component(c);
		pcs[num_pcs_els_in_blk(blk_el0)].s[0] = sl;
		pcs[num_pcs_els_in_blk(blk_el0)].s[1] = sr;
		++num_pcs_els_in_blk(blk_el0);
	    }
	    p = b->end;
	    if (sorted(p)==NO)
	    {
		sorted(p) = YES;
		tp = (TG_PT*)find_from_hash_table((POINTER)p,hash_table,
				                  h_size);
		slsr(p,Hyper_surf_element(b),Hyper_surf(c),&sl,&sr);
		pcs[num_pcs_els_in_blk(blk_el0)].p = tp;
		pcs[num_pcs_els_in_blk(blk_el0)].comp[0] =
			negative_component(c);
		pcs[num_pcs_els_in_blk(blk_el0)].comp[1] =
			positive_component(c);
		pcs[num_pcs_els_in_blk(blk_el0)].s[0] = sl;
		pcs[num_pcs_els_in_blk(blk_el0)].s[1] = sr;
		++num_pcs_els_in_blk(blk_el0);
	    }
	    
	}
	if (debugging("add_intfc_blk_pcs2d"))
	{
	    int i;
	    
	    (void) printf("\nblk_el0 %llu\n", ptr2ull(blk_el0));
	    (void) printf("\t num_pcs_els_in_blk = %d\n",
		          num_pcs_els_in_blk(blk_el0));
	    for (i=0; i< num_pcs_els_in_blk(blk_el0);++i)
	    {
		(void) printf("%d: (%g,%g),", i,
			      Coords(pcs[i].p)[0],Coords(pcs[i].p)[1]);
		(void) printf("comp = (%d,%d)\n",
			      pcs[i].comp[0],pcs[i].comp[1]);
	    }
	}
		       
	    
}	/* end add_intfc_blk_pcs2d */



EXPORT	int collect_pcs_in_mesh2d(TRI_GRID *ntg)
{
        int		xmax;
        int		ymax;
        struct Table	*T;
        P_LINK		*hash_table;
        Locstate	*states;
        COMPONENT	*comp;
        TG_PT		*node_pts;
        BLK_EL0		*blk_el0;
        BOND    	****by;
        BOND    	***byx;
        CURVE   	****cy,***cyx;
        POINT_COMP_ST	*pcs;
	int		ix, iy, l;
        int		**nby, *nbyx; 
	int		*offset = ntg->offset;
	int		num_pcs = 0;
	int		h_size;
	
#if defined(DEBUG_TRI_GRID)
	debug_print("collect_pcs_in_mesh2d", "make_bond_comp_list");
#endif /* defined(DEBUG_TRI_GRID) */

	set_dual_interface_topology(ntg);
	ntg->_locate_on_trigrid = tg_build;

#if defined(DEBUG_TRI_GRID)
	debug_print("collect_pcs_in_mesh2d", "to count_num_pcs2d");
#endif /* defined(DEBUG_TRI_GRID) */

	ntg->n_pcs = count_num_pcs2d(ntg);
	VECTOR(ntg,pcs,ntg->n_pcs,sizeof(POINT_COMP_ST));


	T = table_of_interface(ntg->grid_intfc);


        xmax = ntg->rect_grid.gmax[0];
       	ymax = ntg->rect_grid.gmax[1];


		/* Allocate space for hashing table */

	/*P_LINK (int.h) pair of left and right states */

	h_size = (ntg->grid_intfc->num_points)*4+1; 
	uni_array(&hash_table,h_size,sizeof(P_LINK)); 

#if defined(DEBUG_TRI_GRID)
	debug_print("collect_pcs_in_mesh2d", "to copy_tg_pts_from_intfc");
#endif /* defined(DEBUG_TRI_GRID) */

	copy_tg_pts_from_intfc(ntg,hash_table,h_size);

		/* set the pcs's */

	comp = T->components;
	states = ntg->states;
	node_pts = ntg->node_points;
	blk_el0 = ntg->blk_els0;

#if defined(DEBUG_TRI_GRID)
	debug_print("collect_pcs_in_mesh2d", "to set the pcs's");
#endif /* defined(DEBUG_TRI_GRID) */

	for (iy = 0, nby = T->num_of_bonds, by  = T->bonds, cy  = T->curves;
	     iy < ymax; ++iy, ++nby, ++by, ++cy)
	{
	    for (ix = 0, nbyx = *nby, byx = *by, cyx = *cy;
		 ix < xmax; 
		 ++ix, ++nbyx, ++byx, ++cyx)
	    {
		pcs = blk_el0_pcs_els(blk_el0) = &(ntg->pcs[num_pcs]);
		if (*nbyx != 0)
		{
		    for (l = 0; l < 4; ++l)
		    {
			pcs[l].p = node_pts + offset[l];
			pcs[l].comp[0] = comp[offset[l]];
			pcs[l].s[0] = states[offset[l]];
			pcs[l].comp[1] = NO_COMP;
			pcs[l].s[1] = NULL;
		    }
		    num_pcs_els_in_blk(blk_el0) = 4;

		    add_intfc_blk_pcs2d(*nbyx,*byx,*cyx,blk_el0,
				hash_table,h_size);
		    num_pcs += num_pcs_els_in_blk(blk_el0);
		}
		else
		{
		    set_bilinear_blk_el0(blk_el0);
		    pcs[0].p = node_pts;
		    pcs[0].comp[0] = comp[0];
		    pcs[0].s[0] = states[0];
		    pcs[0].comp[1] = NO_COMP;
		    pcs[0].s[1] = NULL;
		    ++num_pcs;
		}
		++node_pts;
		++states;
		++comp;
		++blk_el0;
	    }
	    ++node_pts;
	    ++states;
	    ++comp;
	    
	    /*
	    if (iy < ymax -1)
	    {
		  node_pts += xmax+1;
		  states += xmax+1;
		  comp += xmax+1;
	    }
	    */
	}

	free(hash_table);
#if defined(DEBUG_TRI_GRID)
	if (debugging("collect_pcs_in_mesh2d"))
	{
	    int i;
	    int icoords[MAXD];
	    double *coords;

	    (void) printf("\t\t PRINTING PCS:\n");

	    for (i=0;i<ntg->n_pcs ;++i)
	    {   
		coords = (double*)(ntg->pcs[i].p);
		if (rect_in_which(coords,icoords,&(ntg->rect_grid)))
		{
		    (void) printf("%d: icoords[%d,%d] ",
				  i,icoords[0],icoords[1]);
		    (void) printf("comp[0] = %d, comp[1] = %d \n",
				  ntg->pcs[i].comp[0],ntg->pcs[i].comp[1]);
		}
		else
		{
		    (void) printf("rect_in_which failed for this pcs\n");
		    (void) printf("%d: comp[0] = %d, comp[1] = %d \n",
				  i,ntg->pcs[i].comp[0],ntg->pcs[i].comp[1]);
		}
		(void) printf("(%g,%g)\n", coords[0],coords[1]);
	    }
	    (void) printf("\t\n\n PRINTING BLK_EL0:\n");
	    for (icoords[0] = 0; icoords[0] < ntg->rect_grid.gmax[0];
		++icoords[0])
	    {
		for (icoords[1] = 0; icoords[1] < ntg->rect_grid.gmax[1];
		     ++icoords[1])
		{
		    blk_el0 = &Blk_el0(icoords,ntg);
		    (void) printf("blk_el0[%d,%d] = %llu\n",
				  icoords[0],icoords[1],
				  ptr2ull(blk_el0));
		    (void) printf("num_pcs_els_in_blk = %d\n",
				  num_pcs_els_in_blk(blk_el0));
		    for (i = 0; i < num_pcs_els_in_blk(blk_el0); ++i)
		    {
			(void) printf("%d: (%g, %g)\n",i,
				      Coords(blk_el0_pcs_els(blk_el0)[i].p)[0],
				      Coords(blk_el0_pcs_els(blk_el0)[i].p)[1]);
		    }
		}
	    }
	    
	}
	if (debugging("tri_grid"))
	{
	       (void) printf("Grid interface AFTER collect_pcs_in_mesh2d()\n");
	       print_interface(ntg->grid_intfc);
	}
#endif /* defined(DEBUG_TRI_GRID) */
	return GOOD_STEP;
	    
}		/*end collect_pcs_in_mesh2d*/

LOCAL int count_num_pcs2d(TRI_GRID *ntg)
{
        int	xmax;
        int	ymax;
	int	ix, iy, l;
        int	nbyx;
	BOND    *b;
        BOND    **byx;
        struct Table *T;
	int	n_pcs = 0;

#if defined(DEBUG_TRI_GRID)
	debug_print("count_num_pcs2d", "Entered count_num_pcs2d");
#endif /* defined(DEBUG_TRI_GRID) */


	T = table_of_interface(ntg->grid_intfc);

        xmax = ntg->rect_grid.gmax[0];
        ymax = ntg->rect_grid.gmax[1];

	for (iy = 0; iy < ymax;++iy)
	{
	    for (ix = 0; ix < xmax;++ix)
	    {
		nbyx = T->num_of_bonds[iy][ix];
		byx  = T->bonds[iy][ix];
		if (nbyx == 0)
		{
		    n_pcs += 1;
#if defined(DEBUG_TRI_GRID)
	            if (debugging("count_num_pcs2d"))
	            {
	                (void) printf("n_pcs (%d, %d) = 1\n", ix, iy);
	            }
#endif /* defined(DEBUG_TRI_GRID) */

		}
		else
		{
		    n_pcs += 4;
		    for (l=0; l < nbyx;++l)
		    {
			b = byx[l];
			sorted(b->start) = NO;
			sorted(b->end) = NO;
		    }
		    for (l=0; l < nbyx;++l)
		    {
			b = byx[l];
			if (sorted(b->start) == NO)
			{
			    sorted(b->start) = YES;
			    n_pcs += 1;
			}

			if (sorted(b->end) == NO)
			{
			    sorted(b->end) = YES;
			    n_pcs += 1;
			}

		    }
#if defined(DEBUG_TRI_GRID)
	            if (debugging("count_num_pcs2d"))
	            {
	                (void) printf("n_pcs (%d, %d) = %d\n", ix, iy, n_pcs);
	            }
#endif /* defined(DEBUG_TRI_GRID) */

		    
		}
	    }
	}

#if defined(DEBUG_TRI_GRID)
	debug_print("count_num_pcs2d", "Finished count_num_pcs2d");
#endif /* defined(DEBUG_TRI_GRID) */
      
	return n_pcs;
	    
}    /* end count_num_pcs2d */



#endif /* defined(TWOD) */


#if defined(THREED)

LOCAL	void add_intfc_blk_pcs3d(
	int     num_tris,
	TRI     **tris,
	SURFACE **surfs,
	BLK_EL0 *blk_el0,
	P_LINK  *hash_table,
	int     h_size)
{
	POINT_COMP_ST *pcs = blk_el0_pcs_els(blk_el0);
	TRI           *t, *tri;
	SURFACE       *s;
	BOND	      *b;
	BOND_TRI      **btris;
	FT_POINT         *p;
	int           i,j,k, npcs;
	TG_PT         *tp;
	Locstate      sl,sr;

	/* reset points in block tris */

	for (i = 0; i < num_tris; ++i)
	{
	    t = tris[i];
	    for (k = 0; k < 3; ++k)
		sorted(Point_of_tri(t)[k]) = NO;
	}

	/*see count_num_pcs3d for alloc */
	/*#bjet2  add points in the boundary of the surfaces  */
	
	for (i = 0; i < num_tris; ++i)
	{
	    t = tris[i];
	    for (k = 0; k < 3; ++k)
	    {
	        if(!is_side_bdry(t, k))
	            continue;

		b = Bond_on_side(t, k);
		for(j = 0; j < 2; j++)
		{    
		    p = Point_of_tri(t)[(k+j)%3]; /*two points on side k */
		    if(sorted(p))
		        continue;
		    sorted(p) = YES;
		    
		    tp = (TG_PT*)find_from_hash_table((POINTER)p,
				    hash_table, h_size);
		    for(btris = Btris(b); btris && *btris; btris++)
		    {
			tri = (*btris)->tri;
			s = (*btris)->surface;
		        slsr(p,Hyper_surf_element(tri),Hyper_surf(s),&sl,&sr);
			
			npcs = num_pcs_els_in_blk(blk_el0);
			pcs[npcs].p = tp;
			pcs[npcs].comp[0] = negative_component(s);
			pcs[npcs].comp[1] = positive_component(s);
			pcs[npcs].s[0] = sl;
			pcs[npcs].s[1] = sr;
			++num_pcs_els_in_blk(blk_el0);
			
			if(debugging("pcs_cell"))
			{
		    	    printf("npcsc %d  comp %d  %d\n", npcs, 
				    pcs[npcs].comp[0], pcs[npcs].comp[1]);
		    	    print_general_vector("p ", Coords(tp), 3, "\n");
			}
		    }  /*for btris */
		}  /* for j, two points for a side */
	    }  /* for k, sides for t */
	}

	/* add points in the interior of the surface  */

	for (i = 0; i < num_tris; ++i)
	{
	    t = tris[i];	s = surfs[i];
	    for (k = 0; k < 3; ++k)
	    {
		p = Point_of_tri(t)[k];
		if (sorted(p) == YES)
		    continue;
		sorted(p) = YES;

		tp = (TG_PT*)find_from_hash_table((POINTER)p,hash_table,
				                  h_size);
		slsr(p,Hyper_surf_element(t),Hyper_surf(s),&sl,&sr);
		
		npcs = num_pcs_els_in_blk(blk_el0);
		pcs[npcs].p = tp;
		pcs[npcs].comp[0] = negative_component(s);
		pcs[npcs].comp[1] = positive_component(s);
		pcs[npcs].s[0] = sl;
		pcs[npcs].s[1] = sr;
		++num_pcs_els_in_blk(blk_el0);
		
		if(debugging("pcs_cell"))
		{
		    printf("npcs %d  comp %d  %d\n", npcs, 
				    pcs[npcs].comp[0], pcs[npcs].comp[1]);
		    print_general_vector("p ", Coords(tp), 3, "\n");
		}
	    }
	}
}	/* end add_intfc_blk_pcs3d */



EXPORT	int collect_pcs_in_mesh3d(TRI_GRID *ntg)
{
	struct Table	*T;
	P_LINK		*hash_table;
	Locstate	 *states;
	COMPONENT	 *comp;
	TG_PT		 *node_pts;
	BLK_EL0		*blk_el0;
	TRI		**tris;
	SURFACE		**surfs;
	POINT_COMP_ST	*pcs;
	int		ix, iy, iz, l, nt, ***num_tris;
	int		*offset = ntg->offset;
	int		num_pcs = 0;
	int		h_size;
	int		xmax, ymax, zmax;

        xmax = ntg->rect_grid.gmax[0];
       	ymax = ntg->rect_grid.gmax[1];
       	zmax = ntg->rect_grid.gmax[2];

	ntg->_locate_on_trigrid = tg_build;
	set_tri3d_tolerances(ntg);

	T = table_of_interface(ntg->grid_intfc);

	/*orig_construct_tri_grid */
	/*reconstruct_intfc_and_tri_grid */
	/*    init_triangulation_storage */
	/*        components, states */
	/*        set_crx_structure_storage3dv0 */
	/*            ntg->n_bilin_els = xmax*ymax*zmax;    expanded_topological_grid */
	/*	      ntg->n_node_points = (xmax+1)*(ymax+1)*(zmax+1); */
	/*	       */
	/*	      alloc_node_points(ntg,ntg->n_node_points); */
	/*	          node_points */
	/*	      alloc_blk_els0(ntg,ntg->n_bilin_els); */
	/*	          blk_els0 */
	/*    set_interpolation_storage3dv0 */
	/*        count_num_pcs3d */
	/*        n_pcs, pcs, front_points */
	
		/* Allocate space for hashing table */

	h_size = (ntg->grid_intfc->num_points)*4+1; 
	uni_array(&hash_table,h_size,sizeof(P_LINK)); 

	copy_tg_pts_from_intfc(ntg,hash_table,h_size);
	/*front_points */
	copy_tg_pts_from_regular_grid(ntg);
	/*node_points */

		/* Triangulate each mesh block */

	comp = T->components;
	states = ntg->states;
	node_pts = ntg->node_points;
	blk_el0 = ntg->blk_els0;
	num_tris = T->num_of_tris;
	for (iz = 0; iz < zmax; ++iz)
	{
	    for (iy = 0; iy < ymax; ++iy)
	    {
		for (ix = 0; ix < xmax; ++ix)
		{
		    /*debug with tg_build  line_pj */
		    remove_from_debug("pcs_cell");
		    /*if((ix == 3  && iy == 5 && iz == 0 && pp_mynode() == 0) ||  */
		    /*     (ix == 33 && iy == 5 && iz == 0 && pp_mynode() == 2) ) */
		    /*    add_to_debug("pcs_cell"); */
		    
		    nt = num_tris[iz][iy][ix];
		    pcs = blk_el0_pcs_els(blk_el0) = &(ntg->pcs[num_pcs]);
		    if (nt != 0)
		    {
			tris  = T->tris[iz][iy][ix];
			surfs = T->surfaces[iz][iy][ix];
			
			if(debugging("pcs_cell"))
			{
			    int nd;
			    printf("#pcs_cell  %d\n", nt);

			    for(nd = 0; nd < nt; nd++)
			    {
			        print_tri(tris[nd], surfs[nd]->interface);
				printf("%d  (%d %d)\n", surfs[nd], 
						negative_component(surfs[nd]), 
						positive_component(surfs[nd]));
			    }
			}
		    	for (l = 0; l < 8; ++l)
		    	{
			    pcs[l].p = node_pts + offset[l];
			    pcs[l].comp[0] = comp[offset[l]];
			    pcs[l].s[0] = states[offset[l]];
			    pcs[l].comp[1] = NO_COMP;
			    pcs[l].s[1] = NULL;
		    	}
		        num_pcs_els_in_blk(blk_el0) = 8;
		    	add_intfc_blk_pcs3d(nt,tris,surfs,blk_el0,
				hash_table,h_size);
		        num_pcs += num_pcs_els_in_blk(blk_el0);
		    }
		    else
		    {
		        set_bilinear_blk_el0(blk_el0);
			pcs[0].p = node_pts;
			pcs[0].comp[0] = comp[0];
			pcs[0].s[0] = states[0];
			pcs[0].comp[1] = NO_COMP;
			pcs[0].s[1] = NULL;
			++num_pcs;
		    }
		    ++node_pts;
		    ++states;
		    ++comp;
		    ++blk_el0;
		}
		++node_pts;
		++states;
		++comp;
	    }
	    if (iz < zmax-1)
	    {   /*node_pts, states, comp are defined on the nodes of the topological grid. */
		node_pts += xmax+1;
		states += xmax+1;
		comp += xmax+1;
	    }
	}

	/*printf("#collect_pcs_in_mesh3d af: num_pcs = %d  ntg->n_pcs=%d\n",  */
	/*		num_pcs, ntg->n_pcs); */
	if(num_pcs != ntg->n_pcs)
	{
	    printf("ERROR: num_pcs != ntg->n_pcs\n");
	    clean_up(ERROR);
	}
	
	free(hash_table);
	return GOOD_STEP;
}		/*end collect_pcs_in_mesh3d*/

#endif /* defined(THREED) */
#endif /* defined(TWOD) || defined(THREED) */
