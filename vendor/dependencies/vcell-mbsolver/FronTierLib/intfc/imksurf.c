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
*				imksurf.c:
*
*       Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Containing function of rebuilding interface within a mesh block.
*
*/

#if defined(THREED)

#define DEBUG_STRING "i_make_surf"

#include <intfc/int.h>
#include <intfc/iprotos.h>
#include <fprotos.h>

LOCAL   void 	assign_blk_crx(BLK_CRX*,int,int,int,const EG_CRX*,boolean);
LOCAL	void 	assign_two_comp_domain(double (*func)(POINTER,double*),POINTER,
			COMPONENT***,RECT_GRID,COMPONENT,COMPONENT);
LOCAL	boolean 	install_bdry_objects(INTERFACE*,NODE****,CURVE****,
			SURFACE***,RECT_GRID*,RECT_GRID*);
LOCAL   void    stitch_curves_of_blk(int*,BLK_TRI****,CURVE**,int);
LOCAL	boolean 	is_positive_bdry_curve(int,int,int,NODE*,NODE*);
LOCAL   boolean    face_crx_in_dir(double (*func1)(POINTER,double*),POINTER,
                        double (*func2)(POINTER,double*),POINTER,
			double*,double*,double*,int);
LOCAL   int     count_bdry_coner_crx(int*,COMPONENT***);
LOCAL   int     install_bdry_corner_crx(INTERFACE*,EG_CRX*,RECT_GRID,double*,double*,
			NODE****,CURVE****,SURFACE***);
LOCAL	int 	count_side_comp(COMPONENT,COMPONENT,COMPONENT,COMPONENT);

/*	Initialization functions for level surfaces */

LOCAL 	POINTER init_plane_params(RECT_GRID*);
LOCAL 	POINTER init_ellipsoid_params(RECT_GRID*);
LOCAL 	POINTER init_multi_ellipsoid_params(RECT_GRID*);
LOCAL 	POINTER init_hyperboloid_params(RECT_GRID*);
LOCAL 	POINTER init_paraboloid_params(RECT_GRID*);
LOCAL 	POINTER init_sine_wave_params(RECT_GRID*);
LOCAL 	POINTER init_i_random_pert_params(RECT_GRID*);
LOCAL 	POINTER init_dumbbell_params(RECT_GRID*);

/*	Level surface functions */
LOCAL   double hyperboloid_func(POINTER,double*);
LOCAL   double paraboloid_func(POINTER,double*);
LOCAL   double i_random_pert_func(POINTER,double*);
LOCAL   double multi_ellipsoid_func(POINTER,double*);

EXPORT	boolean make_bdry_surfaces(
	INTERFACE *intfc,
	RECT_GRID *rgr)
{
	int 		i, j, k, l, num_crx,*gmax;
	int             ns, nc;
	RECT_GRID   	dual_gr;
	NODE		****corners;
	CURVE		****edges;
	SURFACE		***faces;
	EG_CRX		Eg_crx;
	BDRY_BOX_PARAMS bp;
	COMPONENT	compin,compout;
	BLK_INFO	blk_info;
	static BLK_CRX  *blk_crx;
	NODE **n;

	DEBUG_ENTER(make_bdry_surfaces)

	if (blk_crx == NULL)
	    blk_crx = alloc_blk_crx(NO);
	set_grid_for_surface_construction(&dual_gr,rgr);
	gmax = dual_gr.gmax;

	tri_array(&corners,2,2,2,sizeof(NODE*));
	tri_array(&edges,3,2,2,sizeof(CURVE*));
	bi_array(&faces,3,2,sizeof(SURFACE*));

	if (!install_bdry_objects(intfc,corners,edges,faces,rgr,&dual_gr))
	    return YES;

	tri_array(&Eg_crx.comp,gmax[0]+1,gmax[1]+1,gmax[2]+1,sizeof(COMPONENT));
	reset_domain_comp(Eg_crx.comp,dual_gr);

	bp.L = rgr->VL; 	bp.U = rgr->VU;
	bp.intfc = intfc;
	for (i = 0; i < 3; ++i)
	{
	    if (rgr->lbuf[i] != 0) bp.L[i] -= rgr->h[i];
	    if (rgr->ubuf[i] != 0) bp.U[i] += rgr->h[i];
	}
	compin = NO_COMP;
	compout = exterior_component(intfc);
	assign_two_comp_domain(bdry_box_func,(POINTER)&bp,Eg_crx.comp,
				dual_gr,compout,compin);
	num_crx = count_crx_through_comp(gmax,Eg_crx.comp);
	num_crx += count_bdry_coner_crx(gmax,Eg_crx.comp);

	alloc_grid_crx_mem(&Eg_crx,gmax,num_crx,YES);

	Eg_crx.num_curves = 0;
	for (i = 0; i < 3; ++i)
	    for (j = 0; j < 2; ++j)
	    	for (k = 0; k < 2; ++k)
		    if (edges[i][j][k] != NULL)
		    	++Eg_crx.num_curves;
	uni_array(&Eg_crx.curves,Eg_crx.num_curves,sizeof(CURVE*));
	for (l = 0, i = 0; i < 3; ++i)
	    for (j = 0; j < 2; ++j)
	    	for (k = 0; k < 2; ++k)
		    if (edges[i][j][k] != NULL)
		    	Eg_crx.curves[l++] = edges[i][j][k];

	num_crx = install_bdry_corner_crx(intfc,&Eg_crx,dual_gr,bp.L,bp.U,
			corners,edges,faces);

	blk_info.num_surfs = blk_info.num_curves = 0;
	for (i = 0; i < 3; ++i)
	{
	    for (j = 0; j < 2; ++j)
	    {
	    	for (k = 0; k < 2; ++k)
		{
		    if (edges[i][j][k] != NULL)
		    	++blk_info.num_curves;
		}
		if (faces[i][j] != NULL)
		    ++blk_info.num_surfs;
	    }
	}
	uni_array(&blk_info.surfs,blk_info.num_surfs,sizeof(SURFACE*));
	uni_array(&blk_info.curves,blk_info.num_curves,sizeof(CURVE*));
	uni_array(&blk_info.cur_tris,blk_info.num_surfs,sizeof(TRI*));

	for (ns = 0, nc = 0, i = 0; i < 3; ++i)
	{
	    for (j = 0; j < 2; ++j)
	    {
	    	for (k = 0; k < 2; ++k)
		{
		    if (edges[i][j][k] != NULL)
		    	blk_info.curves[nc++] = edges[i][j][k];
		}
		if (faces[i][j] != NULL)
		    blk_info.surfs[ns++] = faces[i][j];
	    }
	}

	for (i = 0; i < blk_info.num_surfs; ++i)
	    first_tri(blk_info.surfs[i]) = last_tri(blk_info.surfs[i]) = NULL;

	blk_crx->comps[0] = compout;
	blk_crx->comps[1] = compin;
	blk_crx->blk_info = &blk_info;

	make_grid_surfaces(blk_crx,&Eg_crx,gmax,YES);
	for (n = intfc->nodes; n && *n; ++n)
	{
	    if ((*n)->in_curves == NULL && (*n)->out_curves == NULL)
	    {
	    	delete_node(*n);
	    	n = intfc->nodes;
	    }
	}
	for (i = 0; i < blk_info.num_surfs; ++i)
        {
	    last_tri(blk_info.surfs[i]) = blk_info.cur_tris[i];
	    last_tri(blk_info.surfs[i])->next = 
	    		tail_of_tri_list(blk_info.surfs[i]);
	    first_tri(blk_info.surfs[i])->prev = 
	    		head_of_tri_list(blk_info.surfs[i]);
	}

	install_subdomain_bdry_curves(intfc);
	reset_intfc_num_points(intfc);

	free_grid_crx_mem(&Eg_crx,YES);
	free_these(4,Eg_crx.comp,corners,edges,faces);
	free_these(3,blk_info.surfs,blk_info.curves,blk_info.cur_tris);

	DEBUG_LEAVE(install_faces)
	return YES;
}	/* end make_bdry_surfaces */


/*******************************************************************
*       This function make a surface described by the function     *
*       func = 0. The negative side of the surface has neg_comp    *
*       and the positive side of the surface has pos_comp.         *
*******************************************************************/

EXPORT boolean make_level_surface(
	RECT_GRID   *rgr,
	INTERFACE   *intfc,
	COMPONENT   neg_comp,
	COMPONENT   pos_comp,
	double       (*func)(POINTER,double*),
        POINTER     func_params,
	SURFACE     **s)
{
	int		i,num_crx, *gmax;
	RECT_GRID	dual_gr;
	SURFACE		*surf;
	EG_CRX		Eg_crx;
	BLK_INFO	blk_info;
	static BLK_CRX  *blk_crx;
        INTERFACE       *save_intfc;

        save_intfc = current_interface();
        set_current_interface(intfc);
	if (blk_crx == NULL)
	    blk_crx = alloc_blk_crx(NO);

	zero_scalar(&Eg_crx,sizeof(EG_CRX));

	set_grid_for_surface_construction(&dual_gr,rgr);
	gmax = dual_gr.gmax;
	tri_array(&Eg_crx.comp,gmax[0]+1,gmax[1]+1,gmax[2]+1,
			sizeof(COMPONENT));

	reset_domain_comp(Eg_crx.comp,dual_gr);
	assign_two_comp_domain(func,func_params,Eg_crx.comp,
				dual_gr,neg_comp,pos_comp);
	num_crx = count_crx_through_comp(gmax,Eg_crx.comp);

	alloc_grid_crx_mem(&Eg_crx,gmax,num_crx,NO);

	num_crx = install_grid_crx(func,func_params,&Eg_crx,dual_gr,
				neg_comp,pos_comp);

	surf = make_surface(neg_comp,pos_comp,NULL,NULL);

	first_tri(surf) = last_tri(surf) = NULL;

	blk_info.num_surfs = 1;
	uni_array(&blk_info.surfs,1,sizeof(SURFACE*));
	uni_array(&blk_info.cur_tris,1,sizeof(TRI*));

	blk_info.cur_tris[0] = NULL;
	blk_info.surfs[0] = surf;
	surf->num_tri = 0;

	blk_crx->comps[0] = pos_comp;
	blk_crx->comps[1] = neg_comp;
	blk_crx->blk_info = &blk_info;

	for (i = 0; i < num_crx; ++i)
	    Eg_crx.crx_store[i].s = surf;
	
	make_grid_surfaces(blk_crx,&Eg_crx,gmax,NO);

	if (first_tri(surf) != NULL)
	{
	    last_tri(surf) = blk_info.cur_tris[0];
	    last_tri(surf)->next = tail_of_tri_list(surf);
	    first_tri(surf)->prev = head_of_tri_list(surf);
	    reset_intfc_num_points(surf->interface);
	    interface_reconstructed(surf->interface) = YES;
	    *s = surf;
	}
	else
	{
	    delete_surface(surf);
	    surf = NULL;
	}
	interface_reconstructed(intfc) = YES;
	intfc->modified = YES;

	free_grid_crx_mem(&Eg_crx,NO);
	free_these(3,Eg_crx.comp,blk_info.surfs,blk_info.cur_tris);
	set_current_interface(save_intfc);
	return YES;
}	/* end make_level_surface */

/*******************************************************************
*       This function make a surface described by the function     *
*       func = 0. The negative side of the surface has neg_comp    *
*       and the positive side of the surface has pos_comp.         *
*******************************************************************/

EXPORT boolean make_comp3_surfaces(
	RECT_GRID   *rgr,
	COMPONENT   comp0,
	COMPONENT   comp1,
	COMPONENT   comp2,
	double       (*func1)(POINTER,double*),
        POINTER     func1_params,
	double       (*func2)(POINTER,double*),
        POINTER     func2_params,
	SURFACE     ***s,
	CURVE	    **c)
{
	int		i,num_crx, *gmax;
	RECT_GRID	dual_gr;
	SURFACE		*surf;
	EG_CRX		Eg_crx;
	BLK_INFO	blk_info;
	static BLK_CRX  *blk_crx;

	if (blk_crx == NULL)
	    blk_crx = alloc_blk_crx(NO);

	zero_scalar(&Eg_crx,sizeof(EG_CRX));
	set_grid_for_surface_construction(&dual_gr,rgr);
	gmax = dual_gr.gmax;
	tri_array(&Eg_crx.comp,gmax[0]+1,gmax[1]+1,gmax[2]+1,
			sizeof(COMPONENT));

	reset_domain_comp(Eg_crx.comp,dual_gr);
	print_rectangular_grid(&dual_gr);
	assign_negative_comp(func1,func1_params,Eg_crx.comp,
				dual_gr,comp0);
	assign_intersection_comp(func1,func1_params,func2,func2_params,
				Eg_crx.comp,dual_gr,comp1,
				POSITIVE_SIDE,NEGATIVE_SIDE);
	assign_positive_comp(func2,func2_params,Eg_crx.comp,
				dual_gr,comp2);
	num_crx = count_crx_through_comp(gmax,Eg_crx.comp);
	printf("num_crx = %d\n",num_crx);

	alloc_grid_crx_mem(&Eg_crx,gmax,num_crx,YES);

	free_grid_crx_mem(&Eg_crx,YES);
	return YES;
}	/* end make_comp3_surfaces */

LOCAL	boolean install_bdry_objects(
	INTERFACE *intfc,
	NODE ****corners,
	CURVE ****edges,
	SURFACE ***faces,
	RECT_GRID *gr,
	RECT_GRID *dual_gr)
{
	int i,j,k,ii,jj;
	double coords[3];
	double corner_coords[3][2];
	NODE *nodes[2][2],*ns,*ne;
	COMPONENT ext_comp = exterior_component(intfc);
	boolean bdry_face = NO;
	int bdry_index_offset = 100;
	boolean edge_flag[3][2][2];
	boolean node_flag[2][2][2];
	int dir,bdry_side[2];
	int surf_count;


	for (i = 0; i < 3; ++i)
	{
	    for (j = 0; j < 2; ++j)
	    {
	    	for (k = 0; k < 2; ++k)
		{
		    if (i != 2) 
		    	corners[i][j][k] = NULL;
		    edges[i][j][k] = NULL;
		}
		faces[i][j] = NULL;
	    }
	}

	for (i = 0; i < 3; ++i)
	for (j = 0; j < 2; ++j)
	for (k = 0; k < 2; ++k)
	{
	    edge_flag[i][j][k] = YES;
            if (buffered_boundary_type(intfc->rect_bdry_type[(i+1)%3][j]) ||
		intfc->rect_bdry_type[(i+1)%3][j] == MIXED_TYPE_BOUNDARY)
	    	edge_flag[i][j][k] = NO;
            if (buffered_boundary_type(intfc->rect_bdry_type[(i+2)%3][k]) ||
		intfc->rect_bdry_type[(i+2)%3][k] == MIXED_TYPE_BOUNDARY)
	    	edge_flag[i][j][k] = NO;
	}
	for (i = 0; i < 2; ++i)
	for (j = 0; j < 2; ++j)
	for (k = 0; k < 2; ++k)
	{
	    surf_count = 3;
	    node_flag[i][j][k] = YES;
            if (buffered_boundary_type(intfc->rect_bdry_type[0][i]) ||
		intfc->rect_bdry_type[0][i] == MIXED_TYPE_BOUNDARY)
		surf_count--;
            if (buffered_boundary_type(intfc->rect_bdry_type[1][j]) ||
		intfc->rect_bdry_type[1][j] == MIXED_TYPE_BOUNDARY)
		surf_count--;
            if (buffered_boundary_type(intfc->rect_bdry_type[2][k]) ||
		intfc->rect_bdry_type[2][k] == MIXED_TYPE_BOUNDARY)
		surf_count--;
	    if (surf_count < 2) 
		node_flag[i][j][k] = NO;
	}

	for (i = 0; i < 3; ++i)
	{
	    if (!buffered_boundary_type(intfc->rect_bdry_type[i][0]))
		corner_coords[i][0] = gr->L[i];
	    else
	    	corner_coords[i][0] = dual_gr->L[i];
	    if (!buffered_boundary_type(intfc->rect_bdry_type[i][1]))
		corner_coords[i][1] = gr->U[i];
	    else
	    	corner_coords[i][1] = dual_gr->U[i];
	}
	for (i = 0; i < 3; ++i)
	{
	    for (j = 0; j < 2; ++j)
	    {
	        if ((!buffered_boundary_type(intfc->rect_bdry_type[i][j])) &&
                    (intfc->rect_bdry_type[i][j] != MIXED_TYPE_BOUNDARY))
	        {
		    coords[i] = corner_coords[i][j];
		    for (ii = 0; ii < 2; ++ii)
		    {
		        for (jj = 0; jj < 2; ++jj)
		        {
		    	    coords[(i+1)%3] = corner_coords[(i+1)%3][ii];
		    	    coords[(i+2)%3] = corner_coords[(i+2)%3][jj];
			    switch (i)
			    {
			    case 0:
				/*
				if (corners[j][ii][jj] == NULL &&
				    node_flag[j][ii][jj] == YES)
				*/
				if (corners[j][ii][jj] == NULL) 
				    corners[j][ii][jj] = 
				    	make_node(Point(coords));
			    	nodes[ii][jj] = corners[j][ii][jj];
				break;
			    case 1:
				/*
				if (corners[jj][j][ii] == NULL &&
				    node_flag[jj][j][ii] == YES)
				*/
				if (corners[jj][j][ii] == NULL) 
				    corners[jj][j][ii]
					= make_node(Point(coords));
			    	nodes[ii][jj] = corners[jj][j][ii]; 
				break;
			    case 2:
				/*
				if (corners[ii][jj][j] == NULL &&
				    node_flag[ii][jj][j] == YES)
				*/
				if (corners[ii][jj][j] == NULL)
				    corners[ii][jj][j]
					= make_node(Point(coords));
			    	nodes[ii][jj] = corners[ii][jj][j]; 
			    }
			    if (nodes[ii][jj] != NULL)
			    {
			      if (j == 0)
			    	set_bdry_side(Boundary(nodes[ii][jj]),
				    		i,BOTTOM);
			      else
			    	set_bdry_side(Boundary(nodes[ii][jj]),
				    		i,TOP);
			    }
		        }
		    }
		    faces[i][j] = make_surface(ext_comp,NO_COMP,
		    		(CURVE**)NULL,(CURVE**)NULL);
		    Hyper_surf_index(faces[i][j]) = bdry_index_offset + 2*i + j;

		    user_install_faces(faces[i][j],2*i+j);
		    set_is_bdry(faces[i][j]);
		    if (j == 0)
		    	set_bdry_side(Boundary(faces[i][j]),i,BOTTOM);
		    else
		    	set_bdry_side(Boundary(faces[i][j]),i,TOP);

		    if (edge_flag[(i+1)%3][0][j] == YES)
		    {
		      if (edges[(i+1)%3][0][j] == NULL)
		      {
			ns = nodes[0][0]; ne = nodes[1][0];
		        edges[(i+1)%3][0][j] = make_curve(NO_COMP,
					NO_COMP,ns,ne);
			if (j == 0)
			{
			    set_bdry_side(Boundary(edges[(i+1)%3][0][j]),
			    		i,BOTTOM);
			    bdry_side[1] = 0;
			}
			else
			{
			    set_bdry_side(Boundary(edges[(i+1)%3][0][j]),
			    		i,TOP);
			    bdry_side[1] = 1;
			}
			set_bdry_side(Boundary(edges[(i+1)%3][0][j]),
			    		(i+2)%3,BOTTOM);
			dir = (i+1)%3;
			bdry_side[0] = 0;
		    	assign_curve_boundary_type(edges[(i+1)%3][0][j],
					dir,bdry_side);
		      }
		      else
		      {
		        ns = edges[(i+1)%3][0][j]->start;
		        ne = edges[(i+1)%3][0][j]->end;
		      }
		      if (is_positive_bdry_curve(i,j,0,ns,ne))
		      {
			add_to_pointers((POINTER)edges[(i+1)%3][0][j],
				(POINTER**)&faces[i][j]->pos_curves);
			add_to_pointers((POINTER)faces[i][j],(POINTER**)
				&edges[(i+1)%3][0][j]->pos_surfaces);
		      }
		      else
		      {
			add_to_pointers((POINTER)edges[(i+1)%3][0][j],
				(POINTER**)&faces[i][j]->neg_curves);
			add_to_pointers((POINTER)faces[i][j],(POINTER**)
				&edges[(i+1)%3][0][j]->neg_surfaces);
		      }
		    }

		    if (edge_flag[(i+2)%3][j][1] == YES)
		    {
		      if (edges[(i+2)%3][j][1] == NULL)
		      {
			ns = nodes[1][0]; ne = nodes[1][1];
		    	edges[(i+2)%3][j][1] = make_curve(NO_COMP,
					NO_COMP,ns,ne);
			if (j == 0)
			{
			    set_bdry_side(Boundary(edges[(i+2)%3][j][1]),
			    		i,BOTTOM);
			    bdry_side[0] = 0;
			}
			else
			{
			    set_bdry_side(Boundary(edges[(i+2)%3][j][1]),
			    		i,TOP);
			    bdry_side[0] = 1;
			}
			set_bdry_side(Boundary(edges[(i+2)%3][j][1]),
			    		(i+1)%3,TOP);
			dir = (i+2)%3;
			bdry_side[1] = 1;
		    	assign_curve_boundary_type(edges[(i+2)%3][j][1],
					dir,bdry_side);
		      }
		      else
		      {
		    	ns = edges[(i+2)%3][j][1]->start;
		    	ne = edges[(i+2)%3][j][1]->end;
		      }
		      if (is_positive_bdry_curve(i,j,1,ns,ne))
		      {
			add_to_pointers((POINTER)edges[(i+2)%3][j][1],
				(POINTER**)&faces[i][j]->pos_curves);
			add_to_pointers((POINTER)faces[i][j],(POINTER**)
				&edges[(i+2)%3][j][1]->pos_surfaces);
		      }
		      else
		      {
			add_to_pointers((POINTER)edges[(i+2)%3][j][1],
				(POINTER**)&faces[i][j]->neg_curves);
			add_to_pointers((POINTER)faces[i][j],(POINTER**)
				&edges[(i+2)%3][j][1]->neg_surfaces);
		      }
		    }

		    if (edge_flag[(i+1)%3][1][j] == YES)
		    {
		      if (edges[(i+1)%3][1][j] == NULL) 
		      {
			ns = nodes[1][1]; ne = nodes[0][1];
		        edges[(i+1)%3][1][j] = make_curve(NO_COMP,
					NO_COMP,ns,ne);
			if (j == 0)
			{
			    set_bdry_side(Boundary(edges[(i+1)%3][1][j]),
			    		i,BOTTOM);
			    bdry_side[1] = 0;
			}
			else
			{
			    set_bdry_side(Boundary(edges[(i+1)%3][1][j]),
			    		i,TOP);
			    bdry_side[1] = 1;
			}
			set_bdry_side(Boundary(edges[(i+1)%3][1][j]),
			    		(i+2)%3,TOP);
			dir = (i+1)%3;
			bdry_side[0] = 1;
		    	assign_curve_boundary_type(edges[(i+1)%3][1][j],
					dir,bdry_side);
		      }
		      else
		      {
		    	ns = edges[(i+1)%3][1][j]->start;
		    	ne = edges[(i+1)%3][1][j]->end;
		      }
		      if (is_positive_bdry_curve(i,j,2,ns,ne))
		      {
			add_to_pointers((POINTER)edges[(i+1)%3][1][j],
				(POINTER**)&faces[i][j]->pos_curves);
			add_to_pointers((POINTER)faces[i][j],(POINTER**)
				&edges[(i+1)%3][1][j]->pos_surfaces);
		      }
		      else
		      {
			add_to_pointers((POINTER)edges[(i+1)%3][1][j],
				(POINTER**)&faces[i][j]->neg_curves);
			add_to_pointers((POINTER)faces[i][j],(POINTER**)
				&edges[(i+1)%3][1][j]->neg_surfaces);
		      }
		    }

		    if (edge_flag[(i+2)%3][j][0] == YES)
		      {
		      if (edges[(i+2)%3][j][0] == NULL) 
		      {
			ns = nodes[0][1]; ne = nodes[0][0];
		    	edges[(i+2)%3][j][0] = make_curve(NO_COMP,
					NO_COMP,ns,ne);
			if (j == 0)
			{
			    set_bdry_side(Boundary(edges[(i+2)%3][j][0]),
			    		i,BOTTOM);
			    bdry_side[0] = 0;
			}
			else
			{
			    set_bdry_side(Boundary(edges[(i+2)%3][j][0]),
			    		i,TOP);
			    bdry_side[0] = 1;
			}
			set_bdry_side(Boundary(edges[(i+2)%3][j][0]),
			    		(i+1)%3,BOTTOM);
			dir = (i+2)%3;
			bdry_side[1] = 0;
		    	assign_curve_boundary_type(edges[(i+2)%3][j][0],
					dir,bdry_side);
		      }
		      else
		      {
		    	ns = edges[(i+2)%3][j][0]->start;
		    	ne = edges[(i+2)%3][j][0]->end;
		      }
		      if (is_positive_bdry_curve(i,j,3,ns,ne))
		      {
			add_to_pointers((POINTER)edges[(i+2)%3][j][0],
				(POINTER**)&faces[i][j]->pos_curves);
			add_to_pointers((POINTER)faces[i][j],(POINTER**)
				&edges[(i+2)%3][j][0]->pos_surfaces);
		      }
		      else
		      {
			add_to_pointers((POINTER)edges[(i+2)%3][j][0],
				(POINTER**)&faces[i][j]->neg_curves);
			add_to_pointers((POINTER)faces[i][j],(POINTER**)
				&edges[(i+2)%3][j][0]->neg_surfaces);
		      }
		    }
	        }
	    }
	}
	for (i = 0; i < 3; ++i)
	{
	    for (j = 0; j < 2; ++j)
	    {
	    	if (faces[i][j] != NULL)
		    bdry_face = YES;
	    }
	}
	return bdry_face;
}	/* end install_bdry_objects */

LOCAL	boolean is_positive_bdry_curve(
	int dir,
	int nb,
	int n,
	NODE *ns,
	NODE *ne)
{
	double ns_coord,ne_coord;

	if (n == 0 || n == 2)
	{
	    ns_coord = Coords(ns->posn)[(dir+1)%3];
	    ne_coord = Coords(ne->posn)[(dir+1)%3];
	}
	else
	{
	    ns_coord = Coords(ns->posn)[(dir+2)%3];
	    ne_coord = Coords(ne->posn)[(dir+2)%3];
	}
	if (nb == 0)
	{
	    if (n == 0 || n == 1)
	    {
	    	if (ns_coord < ne_coord)
		    return YES;
	    	else
		    return NO;
	    }
	    else
	    {
	    	if (ns_coord < ne_coord)
		    return NO;
	    	else
		    return YES;
	    }
	}
	else
	{
	    if (n == 0 || n == 1)
	    {
	    	if (ns_coord < ne_coord)
		    return NO;
	    	else
		    return YES;
	    }
	    else
	    {
	    	if (ns_coord < ne_coord)
		    return YES;
	    	else
		    return NO;
	    }
	}
}	/* end is_positive_bdry_curve */

EXPORT	int install_grid_crx(
	double (*func)(POINTER,double*),
        POINTER func_params,
	EG_CRX *eg_crx,
	RECT_GRID grid,
	COMPONENT comp0,
	COMPONENT comp1)
{
	double coords1[3];
	double coords2[3];
	double crds_crx[3];
	double *L = grid.L;
	double *h = grid.h;
	int *gmax = grid.gmax;
	int dim = grid.dim;
	int i,j,k;
	int n_crx = 0;
	BBI_POINT ****x_crx = eg_crx->x_crx;
	BBI_POINT ****y_crx = eg_crx->y_crx;
	BBI_POINT ****z_crx = eg_crx->z_crx;
	BBI_POINT *crx_store = eg_crx->crx_store;
	COMPONENT ***comp = eg_crx->comp;
	double crx_mid;

	/* install x-crossings */

	for (j = 0; j <= gmax[1]; ++j)
	{
	    coords1[1] = coords2[1] = L[1] + j*h[1];
	    for (k = 0; k <= gmax[2]; ++k)
	    {
		coords1[2] = coords2[2] = L[2] + k*h[2];
		for (i = 0; i < gmax[0]; ++i)
		{
		    /*x_crx[i][j][k] = NULL; */
		    if (((comp[i][j][k] == comp0) && (comp[i+1][j][k] == comp1)) ||
                        ((comp[i][j][k] == comp1) && (comp[i+1][j][k] == comp0)))
		    {
		        coords1[0] = L[0] + i*h[0];
		        coords2[0] = L[0] + (i+1)*h[0];
			if (! grid_line_crx_in_dir(func,func_params,
				dim,coords1,coords2,crds_crx,0))
			{
			    screen("ERROR: in install_grid_crx(), no x-crxing!");
			    clean_up(ERROR);
			}
			if (crds_crx[0] - coords1[0] < 0.004*h[0])
			    crds_crx[0] = coords1[0] + 0.004*h[0];
			if (coords2[0] - crds_crx[0] < 0.004*h[0])
			    crds_crx[0] = coords2[0] - 0.004*h[0];
			crx_mid = 0.5*(coords1[0] + coords2[0]);
			if (fabs(crds_crx[0] - crx_mid) < 0.004*h[0])
			{
			    if (crds_crx[0] < crx_mid)
				crds_crx[0] = crx_mid - 0.004*h[0];
			    else
				crds_crx[0] = crx_mid + 0.004*h[0];
			}
			crx_store[n_crx].p = Point(crds_crx);
			x_crx[i][j][k] = &crx_store[n_crx++];
		    }
		}
	    }
	}

	/* install y-crossings */

	for (i = 0; i <= gmax[0]; ++i)
	{
	    coords1[0] = coords2[0] = L[0] + i*h[0];
	    for (k = 0; k <= gmax[2]; ++k)
	    {
		coords1[2] = coords2[2] = L[2] + k*h[2];
		for (j = 0; j < gmax[1]; ++j)
		{
		    /*y_crx[i][j][k] = NULL; */
		    if (((comp[i][j][k] == comp0) && (comp[i][j+1][k] == comp1)) ||
                        ((comp[i][j][k] == comp1) && (comp[i][j+1][k] == comp0)))
		    {
		        coords1[1] = L[1] + j*h[1];
		        coords2[1] = L[1] + (j+1)*h[1];
			if (! grid_line_crx_in_dir(func,func_params,
				dim,coords1,coords2,crds_crx,1))
			{
			    screen("ERROR: in install_grid_crx(), no y-crxing!");
			    clean_up(ERROR);
			}
			if (crds_crx[1] - coords1[1] < 0.004*h[1])
			    crds_crx[1] = coords1[1] + 0.004*h[1];
			if (coords2[1] - crds_crx[1] < 0.004*h[1])
			    crds_crx[1] = coords2[1] - 0.004*h[1];
			crx_mid = 0.5*(coords1[1] + coords2[1]);
			if (fabs(crds_crx[1] - crx_mid) < 0.004*h[1])
			{
			    if (crds_crx[1] < crx_mid)
				crds_crx[1] = crx_mid - 0.004*h[1];
			    else
				crds_crx[1] = crx_mid + 0.004*h[1];
			}
			crx_store[n_crx].p = Point(crds_crx);
			crx_store[n_crx].p = Point(crds_crx);
			y_crx[i][j][k] = &crx_store[n_crx++];
		    }
		}
	    }
	}

	/* install z-crossings */

	for (i = 0; i <= gmax[0]; ++i)
	{
	    coords1[0] = coords2[0] = L[0] + i*h[0];
	    for (j = 0; j <= gmax[1]; ++j)
	    {
		coords1[1] = coords2[1] = L[1] + j*h[1];
		for (k = 0; k < gmax[2]; ++k)
		{
		    /*z_crx[i][j][k] = NULL; */
		    if (((comp[i][j][k] == comp0) && (comp[i][j][k+1] == comp1)) ||
                        ((comp[i][j][k] == comp1) && (comp[i][j][k+1] == comp0)))
		    {
		        coords1[2] = L[2] + k*h[2];
		        coords2[2] = L[2] + (k+1)*h[2];
			if (! grid_line_crx_in_dir(func,func_params,
				dim,coords1,coords2,crds_crx,2))
			{
			    screen("ERROR: in install_grid_crx(), no z-crxing!");
			    clean_up(ERROR);
			}
			if (crds_crx[2] - coords1[2] < 0.004*h[2])
			    crds_crx[2] = coords1[2] + 0.004*h[2];
			if (coords2[2] - crds_crx[2] < 0.004*h[2])
			    crds_crx[2] = coords2[2] - 0.004*h[2];
			crx_mid = 0.5*(coords1[2] + coords2[2]);
			if (fabs(crds_crx[2] - crx_mid) < 0.004*h[2])
			{
			    if (crds_crx[2] < crx_mid)
				crds_crx[2] = crx_mid - 0.004*h[2];
			    else
				crds_crx[2] = crx_mid + 0.004*h[2];
			}
			crx_store[n_crx].p = Point(crds_crx);
			crx_store[n_crx].p = Point(crds_crx);
			z_crx[i][j][k] = &crx_store[n_crx++];
		    }
		}
	    }
	}
	return n_crx;
}	/* end install_grid_crx */

LOCAL void assign_blk_crx(
	BLK_CRX      *blk_crx,
	int          i,
	int          j,
	int          k,
	const EG_CRX *eg_crx,
	boolean         include_curve_crx)
{
	int ic,num_curve_crx,ii,jj,kk;
	BBI_POINT ****x_crx = eg_crx->x_crx;
	BBI_POINT ****y_crx = eg_crx->y_crx;
	BBI_POINT ****z_crx = eg_crx->z_crx;
	BBI_POINT ****x_curve_crx = eg_crx->x_curve_crx;
	BBI_POINT ****y_curve_crx = eg_crx->y_curve_crx;
	BBI_POINT ****z_curve_crx = eg_crx->z_curve_crx;
	BBI_POINT ****node_crx = eg_crx->node_crx;
	COMPONENT c,***comp = eg_crx->comp;

	blk_crx->num_comps = 0;
	for (ii = 0; ii < 2; ++ii)
	{
	    for (jj = 0; jj < 2; ++jj)
	    {
		for (kk = 0; kk < 2; ++kk)
		{
		    c = comp[i+ii][j+jj][k+kk];
		    blk_crx->ix[ii][jj][kk] = ii;
		    blk_crx->iy[ii][jj][kk] = jj;
		    blk_crx->iz[ii][jj][kk] = kk;
		    for (ic = 0; ic < blk_crx->num_comps; ++ic)
		    {
		    	if (c == blk_crx->comps[ic])
			{
			    ++blk_crx->nv[ic];
			    break;
			}
		    }
		    if (ic == blk_crx->num_comps)
		    {
		    	blk_crx->comps[ic] = c;
			blk_crx->nv[ic] = 1;
			++blk_crx->num_comps;
		    }
		    blk_crx->comp[ii][jj][kk] = c;
		}
		blk_crx->crx[0][ii][jj] = x_crx[i][j+ii][k+jj];
		blk_crx->crx[1][ii][jj] = y_crx[i+jj][j][k+ii];
		blk_crx->crx[2][ii][jj] = z_crx[i+ii][j+jj][k];
	    }
	}
	for (ii = 0; ii < blk_crx->num_comps-1; ++ii)
	{
	    for (jj = 1; jj < blk_crx->num_comps; ++jj)
	    {
	    	if (blk_crx->nv[ii] > blk_crx->nv[jj])
		{
		    int nv_tmp;
		    COMPONENT c_tmp;
		    nv_tmp = blk_crx->nv[ii];
		    blk_crx->nv[ii] = blk_crx->nv[jj];
		    blk_crx->nv[jj] = nv_tmp;
		    c_tmp = blk_crx->comps[ii];
		    blk_crx->comps[ii] = blk_crx->comps[jj];
		    blk_crx->comps[jj] = c_tmp;
		}
	    }
	}
	if (! include_curve_crx)
	{
	    blk_crx->blk_type = COMP2_BLOCK;
	    return;
	}
	num_curve_crx = 0;
	for (ii = 0; ii < 2; ++ii)
	{
	    blk_crx->curve_crx[0][ii] = x_curve_crx[i+ii][j][k];
	    if (x_curve_crx[i+ii][j][k] != NULL)
		++num_curve_crx;
	    blk_crx->curve_crx[1][ii] = y_curve_crx[i][j+ii][k];
	    if (y_curve_crx[i][j+ii][k] != NULL)
		++num_curve_crx;
	    blk_crx->curve_crx[2][ii] = z_curve_crx[i][j][k+ii];
	    if (z_curve_crx[i][j][k+ii] != NULL)
		++num_curve_crx;
	}
	blk_crx->node_crx = node_crx[i][j][k];
	if (blk_crx->num_comps == 2)
	{
	    if (num_curve_crx == 0)
	    	blk_crx->blk_type = COMP2_BLOCK;
	    else if (num_curve_crx != 0)
	    	blk_crx->blk_type = BDRY_BLOCK;
	}
	else if (blk_crx->num_comps == 3)
	    blk_crx->blk_type = COMP3_BLOCK;
}	/* end assign_blk_crx */

EXPORT	boolean onfront_block(
	int          i,
	int          j,
	int          k,
	const EG_CRX *eg_crx)
{
	int ii,jj;

	for (ii = 0; ii < 2; ++ii)
	{
	    for (jj = 0; jj < 2; ++jj)
	    {
		if (eg_crx->x_crx[i][j+ii][k+jj] != NULL)
		    return YES;
		if (eg_crx->y_crx[i+jj][j][k+ii] != NULL)
		    return YES;
		if (eg_crx->z_crx[i+ii][j+jj][k] != NULL)
		    return YES;
	    }
	}
	return NO;
}	/* end onfront_block */

LOCAL int count_bdry_coner_crx(
	int *gmax,
	COMPONENT ***comp)
{
	int i,num_crx;

	num_crx = 0;
	for (i = 0; i <= gmax[0]; ++i)
	{
	    if (comp[i][1][1] != comp[i][0][1] &&
	    	comp[i][1][1] != comp[i][1][0])
		++num_crx;
	    if (comp[i][gmax[1]-1][1] != comp[i][gmax[1]][1] &&
	    	comp[i][gmax[1]-1][1] != comp[i][gmax[1]-1][0])
		++num_crx;
	    if (comp[i][gmax[1]-1][gmax[2]-1] != comp[i][gmax[1]][gmax[2]-1] &&
	    	comp[i][gmax[1]-1][gmax[2]-1] != comp[i][gmax[1]-1][gmax[2]])
		++num_crx;
	    if (comp[i][1][gmax[2]-1] != comp[i][0][gmax[2]-1] &&
	    	comp[i][1][gmax[2]-1] != comp[i][1][gmax[2]])
		++num_crx;
	}
	for (i = 0; i <= gmax[1]; ++i)
	{
	    if (comp[1][i][1] != comp[0][i][1] &&
	    	comp[1][i][1] != comp[1][i][0])
		++num_crx;
	    if (comp[gmax[0]-1][i][1] != comp[gmax[0]][i][1] &&
	    	comp[gmax[0]-1][i][1] != comp[gmax[0]-1][i][0])
		++num_crx;
	    if (comp[gmax[0]-1][i][gmax[2]-1] != comp[gmax[0]][i][gmax[2]-1] &&
	    	comp[gmax[0]-1][i][gmax[2]-1] != comp[gmax[0]-1][i][gmax[2]])
		++num_crx;
	    if (comp[1][i][gmax[2]-1] != comp[0][i][gmax[2]-1] &&
	    	comp[1][i][gmax[2]-1] != comp[1][i][gmax[2]])
		++num_crx;
	}
	for (i = 0; i <= gmax[2]; ++i)
	{
	    if (comp[1][1][i] != comp[0][1][i] &&
	    	comp[1][1][i] != comp[1][0][i])
		++num_crx;
	    if (comp[gmax[0]-1][1][i] != comp[gmax[0]][1][i] &&
	    	comp[gmax[0]-1][1][i] != comp[gmax[0]-1][0][i])
		++num_crx;
	    if (comp[gmax[0]-1][gmax[1]-1][i] != comp[gmax[0]][gmax[1]-1][i] &&
	    	comp[gmax[0]-1][gmax[1]-1][i] != comp[gmax[0]-1][gmax[1]][i])
		++num_crx;
	    if (comp[1][gmax[1]-1][i] != comp[0][gmax[1]-1][i] &&
	    	comp[1][gmax[1]-1][i] != comp[1][gmax[1]][i])
		++num_crx;
	}
	if (comp[1][1][1] != comp[0][1][1] &&
	    comp[1][1][1] != comp[1][0][1] &&
	    comp[1][1][1] != comp[1][1][0]) 
	    ++num_crx;
	if (comp[gmax[0]-1][1][1] != comp[gmax[0]][1][1] &&
	    comp[gmax[0]-1][1][1] != comp[gmax[0]-1][0][1] &&
	    comp[gmax[0]-1][1][1] != comp[gmax[0]-1][1][0]) 
	    ++num_crx;
	if (comp[1][gmax[1]-1][1] != comp[0][gmax[1]-1][1] &&
	    comp[1][gmax[1]-1][1] != comp[1][gmax[1]][1] &&
	    comp[1][gmax[1]-1][1] != comp[1][gmax[1]-1][0]) 
	    ++num_crx;
	if (comp[1][1][gmax[2]-1] != comp[0][1][gmax[2]-1] &&
	    comp[1][1][gmax[2]-1] != comp[1][0][gmax[2]-1] &&
	    comp[1][1][gmax[2]-1] != comp[1][1][gmax[2]]) 
	    ++num_crx;
	if (comp[gmax[0]-1][gmax[1]-1][1] != comp[gmax[0]][gmax[1]-1][1] &&
	    comp[gmax[0]-1][gmax[1]-1][1] != comp[gmax[0]-1][gmax[1]][1] &&
	    comp[gmax[0]-1][gmax[1]-1][1] != comp[gmax[0]-1][gmax[1]-1][0]) 
	    ++num_crx;
	if (comp[1][gmax[1]-1][gmax[2]-1] != comp[0][gmax[1]-1][gmax[2]-1] &&
	    comp[1][gmax[1]-1][gmax[2]-1] != comp[1][gmax[1]][gmax[2]-1] &&
	    comp[1][gmax[1]-1][gmax[2]-1] != comp[1][gmax[1]-1][gmax[2]]) 
	    ++num_crx;
	if (comp[gmax[0]-1][1][gmax[2]-1] != comp[gmax[0]][1][gmax[2]-1] &&
	    comp[gmax[0]-1][1][gmax[2]-1] != comp[gmax[0]-1][0][gmax[2]-1] &&
	    comp[gmax[0]-1][1][gmax[2]-1] != comp[gmax[0]-1][1][gmax[2]]) 
	    ++num_crx;
	if (comp[gmax[0]-1][gmax[1]-1][gmax[2]-1] != 
	    comp[gmax[0]][gmax[1]-1][gmax[2]-1] &&
	    comp[gmax[0]-1][gmax[1]-1][gmax[2]-1] != 
	    comp[gmax[0]-1][gmax[1]][gmax[2]-1] &&
	    comp[gmax[0]-1][gmax[1]-1][gmax[2]-1] != 
	    comp[gmax[0]-1][gmax[1]-1][gmax[2]]) 
	    ++num_crx;

	return num_crx;
}	/* end count_bdry_coner_crx */


LOCAL int install_bdry_corner_crx(
	INTERFACE *intfc,
	EG_CRX    *eg_crx,
	RECT_GRID dual_gr,
	double     *L,
	double     *U,
	NODE      ****corners,
	CURVE     ****edges,
	SURFACE   ***faces)
{
	int i,j,k;
	BBI_POINT ****x_crx = eg_crx->x_crx;
	BBI_POINT ****y_crx = eg_crx->y_crx;
	BBI_POINT ****z_crx = eg_crx->z_crx;
	BBI_POINT ****x_curve_crx = eg_crx->x_curve_crx;
	BBI_POINT ****y_curve_crx = eg_crx->y_curve_crx;
	BBI_POINT ****z_curve_crx = eg_crx->z_curve_crx;
	BBI_POINT ****node_crx = eg_crx->node_crx;
	BBI_POINT *crx_store = eg_crx->crx_store;
	COMPONENT ***comp = eg_crx->comp;
	double coords[3];
	double *DL = dual_gr.L;
	double *h  = dual_gr.h;
	int *gmax = dual_gr.gmax;
	int m,n,num_crx = 0;

	/* install x-crossings */

	for (j = 0; j <= gmax[1]; ++j)
	{
	    coords[1] = DL[1] + j*h[1];
	    for (k = 0; k <= gmax[2]; ++k)
	    {
		coords[2] = DL[2] + k*h[2];
		if ((comp[0][j][k] != comp[1][j][k]) &&
		    (!buffered_boundary_type(intfc->rect_bdry_type[0][0])) &&
		    (intfc->rect_bdry_type[0][0] != MIXED_TYPE_BOUNDARY))
		{
		    coords[0] = L[0];
		    crx_store[num_crx].p = Point(coords);
		    crx_store[num_crx].s = faces[0][0];
		    x_crx[0][j][k] = &crx_store[num_crx];
		    if (j == 0 || j == gmax[1])
		    {
			m = (j == 0) ? 0 : gmax[1]-1;
			n = (j == 0) ? 0 : 1;
		    	z_curve_crx[0][m][k] = x_crx[0][j][k];
		    	z_curve_crx[0][m][k]->c = 
				(z_curve_crx[0][m][k]->c == NULL) ? 
				edges[2][0][n] : NULL;
		    }
		    if (k == 0 || k == gmax[2])
		    {
			m = (k == 0) ? 0 : gmax[2]-1;
			n = (k == 0) ? 0 : 1;
		    	y_curve_crx[0][j][m] = x_crx[0][j][k];
		    	y_curve_crx[0][j][m]->c = 
				(y_curve_crx[0][j][m]->c == NULL) ?
				edges[1][n][0] : NULL;
		    }
		    if ((j == 0 || j == gmax[1]) &&
		    	(k == 0 || k == gmax[2]))
		    {
		    	m = (j == 0) ? 0 : 1;
		    	n = (k == 0) ? 0 : 1;
			crx_store[num_crx].p = corners[0][m][n]->posn;
		    }
		    ++num_crx;
		}
		if ((comp[gmax[0]-1][j][k] != comp[gmax[0]][j][k]) &&
		    (!buffered_boundary_type(intfc->rect_bdry_type[0][1])) &&
		    (intfc->rect_bdry_type[0][1] != MIXED_TYPE_BOUNDARY))
		{
		    coords[0] = U[0];
		    crx_store[num_crx].p = Point(coords);
		    crx_store[num_crx].s = faces[0][1];
		    x_crx[gmax[0]-1][j][k] = &crx_store[num_crx];
		    if (j == 0 || j == gmax[1])
		    {
			m = (j == 0) ? 0 : gmax[1]-1;
			n = (j == 0) ? 0 : 1;
		    	z_curve_crx[gmax[0]-1][m][k] = x_crx[gmax[0]-1][j][k];
		    	z_curve_crx[gmax[0]-1][m][k]->c = 
				(z_curve_crx[gmax[0]-1][m][k]->c == NULL) ?
				edges[2][1][n] : NULL;
		    }
		    if (k == 0 || k == gmax[2])
		    {
			m = (k == 0) ? 0 : gmax[2]-1;
			n = (k == 0) ? 0 : 1;
		    	y_curve_crx[gmax[0]-1][j][m] = x_crx[gmax[0]-1][j][k];
		    	y_curve_crx[gmax[0]-1][j][m]->c = 
				(y_curve_crx[gmax[0]-1][j][m]->c == NULL) ?
				edges[1][n][1] : NULL;
		    }
		    if ((j == 0 || j == gmax[1]) &&
		    	(k == 0 || k == gmax[2]))
		    {
		    	m = (j == 0) ? 0 : 1;
		    	n = (k == 0) ? 0 : 1;
			crx_store[num_crx].p = corners[1][m][n]->posn;
		    }
		    ++num_crx;
		}
	    }
	}

	/* install y-crossings */

	for (i = 0; i <= gmax[0]; ++i)
	{
	    coords[0] = DL[0] + i*h[0];
	    for (k = 0; k <= gmax[2]; ++k)
	    {
		coords[2] = DL[2] + k*h[2];
		if ((comp[i][0][k] != comp[i][1][k]) &&
		    (!buffered_boundary_type(intfc->rect_bdry_type[1][0])) &&
		    (intfc->rect_bdry_type[1][0] != MIXED_TYPE_BOUNDARY))
		{
		    coords[1] = L[1];
		    crx_store[num_crx].p = Point(coords);
		    crx_store[num_crx].s = faces[1][0];
		    y_crx[i][0][k] = &crx_store[num_crx];
		    if (i == 0 || i == gmax[0])
		    {
			m = (i == 0) ? 0 : gmax[0]-1;
			n = (i == 0) ? 0 : 1;
		    	z_curve_crx[m][0][k] = y_crx[i][0][k];
		    	z_curve_crx[m][0][k]->c = 
				(z_curve_crx[m][0][k]->c == NULL) ?
				edges[2][n][0] : NULL;
		    }
		    if (k == 0 || k == gmax[2])
		    {
			m = (k == 0) ? 0 : gmax[2]-1;
			n = (k == 0) ? 0 : 1;
		    	x_curve_crx[i][0][m] = y_crx[i][0][k];
		    	x_curve_crx[i][0][m]->c = 
				(x_curve_crx[i][0][m]->c == NULL) ?
				edges[0][0][n] : NULL;
		    }
		    if ((i == 0 || i == gmax[0]) &&
		    	(k == 0 || k == gmax[2]))
		    {
		    	m = (i == 0) ? 0 : 1;
		    	n = (k == 0) ? 0 : 1;
			crx_store[num_crx].p = corners[m][0][n]->posn;
		    }
		    ++num_crx;
		}
		if ((comp[i][gmax[1]-1][k] != comp[i][gmax[1]][k]) &&
		    (!buffered_boundary_type(intfc->rect_bdry_type[1][1])) &&
		    (intfc->rect_bdry_type[1][1] != MIXED_TYPE_BOUNDARY))
		{
		    coords[1] = U[1];
		    crx_store[num_crx].p = Point(coords);
		    crx_store[num_crx].s = faces[1][1];
		    y_crx[i][gmax[1]-1][k] = &crx_store[num_crx];
		    if (i == 0 || i == gmax[0])
		    {
			m = (i == 0) ? 0 : gmax[0]-1;
			n = (i == 0) ? 0 : 1;
		    	z_curve_crx[m][gmax[1]-1][k] = y_crx[i][gmax[1]-1][k];
		    	z_curve_crx[m][gmax[1]-1][k]->c = 
				(z_curve_crx[m][gmax[1]-1][k]->c == NULL) ?
				edges[2][n][1] : NULL;
		    }
		    if (k == 0 || k == gmax[2])
		    {
			m = (k == 0) ? 0 : gmax[2]-1;
			n = (k == 0) ? 0 : 1;
		    	x_curve_crx[i][gmax[1]-1][m] = y_crx[i][gmax[1]-1][k];
		    	x_curve_crx[i][gmax[1]-1][m]->c = 
				(x_curve_crx[i][gmax[1]-1][m]->c == NULL) ?
				edges[0][1][n] : NULL;
		    }
		    if ((i == 0 || i == gmax[0]) &&
		    	(k == 0 || k == gmax[2]))
		    {
		    	m = (i == 0) ? 0 : 1;
		    	n = (k == 0) ? 0 : 1;
			crx_store[num_crx].p = corners[m][1][n]->posn;
		    }
		    ++num_crx;
		}
	    }
	}

	/* install z-crossings */

	for (i = 0; i <= gmax[0]; ++i)
	{
	    coords[0] = DL[0] + i*h[0];
	    for (j = 0; j <= gmax[1]; ++j)
	    {
		coords[1] = DL[1] + j*h[1];
		if ((comp[i][j][0] != comp[i][j][1]) &&
		    (!buffered_boundary_type(intfc->rect_bdry_type[2][0])) &&
		    (intfc->rect_bdry_type[2][0] != MIXED_TYPE_BOUNDARY))
		{
		    coords[2] = L[2];
		    crx_store[num_crx].p = Point(coords);
		    crx_store[num_crx].s = faces[2][0];
		    z_crx[i][j][0] = &crx_store[num_crx];
		    if (i == 0 || i == gmax[0])
		    {
		    	m = (i == 0) ? 0 : gmax[0]-1;
		    	n = (i == 0) ? 0 : 1;
			y_curve_crx[m][j][0] = z_crx[i][j][0];
			y_curve_crx[m][j][0]->c = 
				(y_curve_crx[m][j][0]->c == NULL) ? 
				edges[1][0][n] : NULL;
		    }
		    if (j == 0 || j == gmax[1])
		    {
		    	m = (j == 0) ? 0 : gmax[1]-1;
		    	n = (j == 0) ? 0 : 1;
			x_curve_crx[i][m][0] = z_crx[i][j][0];
			x_curve_crx[i][m][0]->c = 
				(x_curve_crx[i][m][0]->c == NULL) ?
				edges[0][n][0] : NULL;
		    }
		    if ((i == 0 || i == gmax[0]) &&
		    	(j == 0 || j == gmax[1]))
		    {
		    	m = (i == 0) ? 0 : 1;
		    	n = (j == 0) ? 0 : 1;
			crx_store[num_crx].p = corners[m][n][0]->posn;
		    }
		    ++num_crx;
		}
		if ((comp[i][j][gmax[2]-1] != comp[i][j][gmax[2]]) &&
		    (!buffered_boundary_type(intfc->rect_bdry_type[2][1])) &&
		    (intfc->rect_bdry_type[2][1] != MIXED_TYPE_BOUNDARY))
		{
		    coords[2] = U[2];
		    crx_store[num_crx].p = Point(coords);
		    crx_store[num_crx].s = faces[2][1];
		    z_crx[i][j][gmax[2]-1] = &crx_store[num_crx];
		    if (i == 0 || i == gmax[0])
		    {
		    	m = (i == 0) ? 0 : gmax[0]-1;
		    	n = (i == 0) ? 0 : 1;
			y_curve_crx[m][j][gmax[2]-1] = z_crx[i][j][gmax[2]-1];
			y_curve_crx[m][j][gmax[2]-1]->c = 
				(y_curve_crx[m][j][gmax[2]-1]->c == NULL) ?
				edges[1][1][n] : NULL;
		    }
		    if (j == 0 || j == gmax[1])
		    {
		    	m = (j == 0) ? 0 : gmax[1]-1;
		    	n = (j == 0) ? 0 : 1;
			x_curve_crx[i][m][gmax[2]-1] = z_crx[i][j][gmax[2]-1];
			x_curve_crx[i][m][gmax[2]-1]->c = 
				(x_curve_crx[i][m][gmax[2]-1]->c == NULL) ?
				edges[0][n][1] : NULL;
		    }
		    if ((i == 0 || i == gmax[0]) &&
		    	(j == 0 || j == gmax[1]))
		    {
		    	m = (i == 0) ? 0 : 1;
		    	n = (j == 0) ? 0 : 1;
			crx_store[num_crx].p = corners[m][n][1]->posn;
		    }
		    ++num_crx;
		}
	    }
	}

	/* install curves crxings */

	for (i = 0; i <= gmax[0]; ++i)
	{
	    coords[0] = DL[0] + i*h[0];
	    if (comp[i][1][1] != comp[i][0][1] &&
	    	comp[i][1][1] != comp[i][1][0] &&
		((!buffered_boundary_type(intfc->rect_bdry_type[1][0]) &&
		 (intfc->rect_bdry_type[1][0] != MIXED_TYPE_BOUNDARY)) &&
		 (!buffered_boundary_type(intfc->rect_bdry_type[2][0]) &&
		 (intfc->rect_bdry_type[2][0] != MIXED_TYPE_BOUNDARY))))
	    {
		if (i == 0)
		    crx_store[num_crx].p = corners[0][0][0]->posn;
		else if (i == gmax[0])
		    crx_store[num_crx].p = corners[1][0][0]->posn;
		else
		{
	    	    coords[1] = L[1]; coords[2] = L[2];
		    crx_store[num_crx].p = Point(coords);
		}
		crx_store[num_crx].c = edges[0][0][0];
		x_curve_crx[i][0][0] = &crx_store[num_crx++];
	    }
	    else
		x_curve_crx[i][0][0] = NULL;
	    if (comp[i][gmax[1]-1][1] != comp[i][gmax[1]][1] &&
	    	comp[i][gmax[1]-1][1] != comp[i][gmax[1]-1][0] &&
		((!buffered_boundary_type(intfc->rect_bdry_type[1][1]) &&
		 (intfc->rect_bdry_type[1][1] != MIXED_TYPE_BOUNDARY)) &&
		 (!buffered_boundary_type(intfc->rect_bdry_type[2][0]) &&
		 (intfc->rect_bdry_type[2][0] != MIXED_TYPE_BOUNDARY))))
	    {
		if (i == 0)
		    crx_store[num_crx].p = corners[0][1][0]->posn;
		else if (i == gmax[0])
		    crx_store[num_crx].p = corners[1][1][0]->posn;
		else
		{
	    	    coords[1] = U[1]; coords[2] = L[2];
		    crx_store[num_crx].p = Point(coords);
		}
		crx_store[num_crx].c = edges[0][1][0];
		x_curve_crx[i][gmax[1]-1][0] = &crx_store[num_crx++];
	    }
	    else
		x_curve_crx[i][gmax[1]-1][0] = NULL;
	    if (comp[i][1][gmax[2]-1] != comp[i][0][gmax[2]-1] &&
	    	comp[i][1][gmax[2]-1] != comp[i][1][gmax[2]] &&
		((!buffered_boundary_type(intfc->rect_bdry_type[1][0]) &&
		 (intfc->rect_bdry_type[1][0] != MIXED_TYPE_BOUNDARY)) &&
		 (!buffered_boundary_type(intfc->rect_bdry_type[2][1]) &&
		 (intfc->rect_bdry_type[2][1] != MIXED_TYPE_BOUNDARY))))
	    {
		if (i == 0)
		    crx_store[num_crx].p = corners[0][0][1]->posn;
		else if (i == gmax[0])
		    crx_store[num_crx].p = corners[1][0][1]->posn;
		else
		{
	    	    coords[1] = L[1]; coords[2] = U[2];
		    crx_store[num_crx].p = Point(coords);
		}
		crx_store[num_crx].c = edges[0][0][1];
		x_curve_crx[i][0][gmax[2]-1] = &crx_store[num_crx++];
	    }
	    else
		x_curve_crx[i][0][gmax[2]-1] = NULL;
	    if (comp[i][gmax[1]-1][gmax[2]-1] != comp[i][gmax[1]][gmax[2]-1] &&
	    	comp[i][gmax[1]-1][gmax[2]-1] != comp[i][gmax[1]-1][gmax[2]] &&
		((!buffered_boundary_type(intfc->rect_bdry_type[1][1]) &&
		 (intfc->rect_bdry_type[1][1] != MIXED_TYPE_BOUNDARY)) &&
		 (!buffered_boundary_type(intfc->rect_bdry_type[2][1]) &&
		 (intfc->rect_bdry_type[2][1] != MIXED_TYPE_BOUNDARY))))
	    {
		if (i == 0)
		    crx_store[num_crx].p = corners[0][1][1]->posn;
		else if (i == gmax[0])
		    crx_store[num_crx].p = corners[1][1][1]->posn;
		else
		{
	    	    coords[1] = U[1]; coords[2] = U[2];
		    crx_store[num_crx].p = Point(coords);
		}
		crx_store[num_crx].c = edges[0][1][1];
		x_curve_crx[i][gmax[1]-1][gmax[2]-1] = &crx_store[num_crx++];
	    }
	    else
		x_curve_crx[i][gmax[1]-1][gmax[2]-1] = NULL;
	}
	for (i = 0; i <= gmax[1]; ++i)
	{
	    coords[1] = DL[1] + i*h[1];
	    if (comp[1][i][1] != comp[0][i][1] &&
	    	comp[1][i][1] != comp[1][i][0] &&
		((!buffered_boundary_type(intfc->rect_bdry_type[0][0]) &&
		 (intfc->rect_bdry_type[0][0] != MIXED_TYPE_BOUNDARY)) &&
		 (!buffered_boundary_type(intfc->rect_bdry_type[2][0]) &&
		 (intfc->rect_bdry_type[2][0] != MIXED_TYPE_BOUNDARY))))
	    {
		if (i == 0)
		    crx_store[num_crx].p = corners[0][0][0]->posn;
		else if (i == gmax[1])
		    crx_store[num_crx].p = corners[0][1][0]->posn;
		else
		{
	    	    coords[0] = L[0]; coords[2] = L[2];
		    crx_store[num_crx].p = Point(coords);
		}
		crx_store[num_crx].c = edges[1][0][0];
		y_curve_crx[0][i][0] = &crx_store[num_crx++];
	    }
	    else
		y_curve_crx[0][i][0] = NULL;
	    if (comp[gmax[0]-1][i][1] != comp[gmax[0]][i][1] &&
	    	comp[gmax[0]-1][i][1] != comp[gmax[0]-1][i][0] &&
		((!buffered_boundary_type(intfc->rect_bdry_type[0][1]) &&
		 (intfc->rect_bdry_type[0][1] != MIXED_TYPE_BOUNDARY)) &&
		 (!buffered_boundary_type(intfc->rect_bdry_type[2][0]) &&
		 (intfc->rect_bdry_type[2][0] != MIXED_TYPE_BOUNDARY))))
	    {
		if (i == 0)
		    crx_store[num_crx].p = corners[1][0][0]->posn;
		else if (i == gmax[1])
		    crx_store[num_crx].p = corners[1][1][0]->posn;
		else
		{
	    	    coords[0] = U[0]; coords[2] = L[2];
		    crx_store[num_crx].p = Point(coords);
		}
		crx_store[num_crx].c = edges[1][0][1];
		y_curve_crx[gmax[0]-1][i][0] = &crx_store[num_crx++];
	    }
	    else
		y_curve_crx[gmax[0]-1][i][0] = NULL;
	    if (comp[1][i][gmax[2]-1] != comp[0][i][gmax[2]-1] &&
	    	comp[1][i][gmax[2]-1] != comp[1][i][gmax[2]] &&
		((!buffered_boundary_type(intfc->rect_bdry_type[0][0]) &&
		 (intfc->rect_bdry_type[0][0] != MIXED_TYPE_BOUNDARY)) &&
		 (!buffered_boundary_type(intfc->rect_bdry_type[2][1]) &&
		 (intfc->rect_bdry_type[2][1] != MIXED_TYPE_BOUNDARY))))
	    {
		if (i == 0)
		    crx_store[num_crx].p = corners[0][0][1]->posn;
		else if (i == gmax[1])
		    crx_store[num_crx].p = corners[0][1][1]->posn;
		else
		{
	    	    coords[0] = L[0]; coords[2] = U[2];
		    crx_store[num_crx].p = Point(coords);
		}
		crx_store[num_crx].c = edges[1][1][0];
		y_curve_crx[0][i][gmax[2]-1] = &crx_store[num_crx++];
	    }
	    else
		y_curve_crx[0][i][gmax[2]-1] = NULL;
	    if (comp[gmax[0]-1][i][gmax[2]-1] != comp[gmax[0]][i][gmax[2]-1] &&
	    	comp[gmax[0]-1][i][gmax[2]-1] != comp[gmax[0]-1][i][gmax[2]] &&
		((!buffered_boundary_type(intfc->rect_bdry_type[0][1]) &&
		 (intfc->rect_bdry_type[0][1] != MIXED_TYPE_BOUNDARY)) &&
		 (!buffered_boundary_type(intfc->rect_bdry_type[2][1]) &&
		 (intfc->rect_bdry_type[2][1] != MIXED_TYPE_BOUNDARY))))
	    {
		if (i == 0)
		    crx_store[num_crx].p = corners[1][0][1]->posn;
		else if (i == gmax[1])
		    crx_store[num_crx].p = corners[1][1][1]->posn;
		else
		{
	    	    coords[0] = U[0]; coords[2] = U[2];
		    crx_store[num_crx].p = Point(coords);
		}
		crx_store[num_crx].c = edges[1][1][1];
		y_curve_crx[gmax[0]-1][i][gmax[2]-1] = &crx_store[num_crx++];
	    }
	    else
		y_curve_crx[gmax[0]-1][i][gmax[2]-1] = NULL;
	}
	for (i = 0; i <= gmax[2]; ++i)
	{
	    coords[2] = DL[2] + i*h[2];
	    if (comp[1][1][i] != comp[0][1][i] &&
	    	comp[1][1][i] != comp[1][0][i] &&
		((!buffered_boundary_type(intfc->rect_bdry_type[0][0]) &&
		 (intfc->rect_bdry_type[0][0] != MIXED_TYPE_BOUNDARY)) &&
		 (!buffered_boundary_type(intfc->rect_bdry_type[1][0]) &&
		 (intfc->rect_bdry_type[1][0] != MIXED_TYPE_BOUNDARY))))
	    {
		if (i == 0)
		    crx_store[num_crx].p = corners[0][0][0]->posn;
		else if (i == gmax[2])
		    crx_store[num_crx].p = corners[0][0][1]->posn;
		else
		{
	    	    coords[0] = L[0]; coords[1] = L[1];
		    crx_store[num_crx].p = Point(coords);
		}
		crx_store[num_crx].c = edges[2][0][0];
		z_curve_crx[0][0][i] = &crx_store[num_crx++];
	    }
	    else
		z_curve_crx[0][0][i] = NULL;
	    if (comp[gmax[0]-1][1][i] != comp[gmax[0]][1][i] &&
	    	comp[gmax[0]-1][1][i] != comp[gmax[0]-1][0][i] &&
		((!buffered_boundary_type(intfc->rect_bdry_type[0][1]) &&
		 (intfc->rect_bdry_type[0][1] != MIXED_TYPE_BOUNDARY)) &&
		 (!buffered_boundary_type(intfc->rect_bdry_type[1][0]) &&
		 (intfc->rect_bdry_type[1][0] != MIXED_TYPE_BOUNDARY))))
	    {
		if (i == 0)
		    crx_store[num_crx].p = corners[1][0][0]->posn;
		else if (i == gmax[2])
		    crx_store[num_crx].p = corners[1][0][1]->posn;
		else
		{
	    	    coords[0] = U[0]; coords[1] = L[1];
		    crx_store[num_crx].p = Point(coords);
		}
		crx_store[num_crx].c = edges[2][1][0];
		z_curve_crx[gmax[0]-1][0][i] = &crx_store[num_crx++];
	    }
	    else
		z_curve_crx[gmax[0]-1][0][i] = NULL;
	    if (comp[1][gmax[1]-1][i] != comp[0][gmax[1]-1][i] &&
	    	comp[1][gmax[1]-1][i] != comp[1][gmax[1]][i] &&
		((!buffered_boundary_type(intfc->rect_bdry_type[0][0]) &&
		 (intfc->rect_bdry_type[0][0] != MIXED_TYPE_BOUNDARY)) &&
		 (!buffered_boundary_type(intfc->rect_bdry_type[1][1]) &&
		 (intfc->rect_bdry_type[1][1] != MIXED_TYPE_BOUNDARY))))
	    {
		if (i == 0)
		    crx_store[num_crx].p = corners[0][1][0]->posn;
		else if (i == gmax[2])
		    crx_store[num_crx].p = corners[0][1][1]->posn;
		else
		{
	    	    coords[0] = L[0]; coords[1] = U[1];
		    crx_store[num_crx].p = Point(coords);
		}
		crx_store[num_crx].c = edges[2][0][1];
		z_curve_crx[0][gmax[1]-1][i] = &crx_store[num_crx++];
	    }
	    else
		z_curve_crx[0][gmax[1]-1][i] = NULL;
	    if (comp[gmax[0]-1][gmax[1]-1][i] != comp[gmax[0]][gmax[1]-1][i] &&
	    	comp[gmax[0]-1][gmax[1]-1][i] != comp[gmax[0]-1][gmax[1]][i] &&
		((!buffered_boundary_type(intfc->rect_bdry_type[0][1]) &&
		 (intfc->rect_bdry_type[0][1] != MIXED_TYPE_BOUNDARY)) &&
		 (!buffered_boundary_type(intfc->rect_bdry_type[1][1]) &&
		 (intfc->rect_bdry_type[1][1] != MIXED_TYPE_BOUNDARY))))
	    {
		if (i == 0)
		    crx_store[num_crx].p = corners[1][1][0]->posn;
		else if (i == gmax[2])
		    crx_store[num_crx].p = corners[1][1][1]->posn;
		else
		{
	    	    coords[0] = U[0]; coords[1] = U[1];
		    crx_store[num_crx].p = Point(coords);
		}
		crx_store[num_crx].c = edges[2][1][1];
		z_curve_crx[gmax[0]-1][gmax[1]-1][i] = &crx_store[num_crx++];
	    }
	    else
		z_curve_crx[gmax[0]-1][gmax[1]-1][i] = NULL;
	}

	/* install node crxings */

	if (comp[1][1][1] != comp[0][1][1] &&
	    comp[1][1][1] != comp[1][0][1] &&
	    comp[1][1][1] != comp[1][1][0]) 
	{
	    crx_store[num_crx].p = corners[0][0][0]->posn;
	    node_crx[0][0][0] = &crx_store[num_crx++];
	}
	if (comp[gmax[0]-1][1][1] != comp[gmax[0]][1][1] &&
	    comp[gmax[0]-1][1][1] != comp[gmax[0]-1][0][1] &&
	    comp[gmax[0]-1][1][1] != comp[gmax[0]-1][1][0]) 
	{
	    crx_store[num_crx].p = corners[1][0][0]->posn;
	    node_crx[gmax[0]-1][0][0] = &crx_store[num_crx++];
	}
	if (comp[1][gmax[1]-1][1] != comp[0][gmax[1]-1][1] &&
	    comp[1][gmax[1]-1][1] != comp[1][gmax[1]][1] &&
	    comp[1][gmax[1]-1][1] != comp[1][gmax[1]-1][0]) 
	{
	    crx_store[num_crx].p = corners[0][1][0]->posn;
	    node_crx[0][gmax[1]-1][0] = &crx_store[num_crx++];
	}
	if (comp[1][1][gmax[2]-1] != comp[0][1][gmax[2]-1] &&
	    comp[1][1][gmax[2]-1] != comp[1][0][gmax[2]-1] &&
	    comp[1][1][gmax[2]-1] != comp[1][1][gmax[2]]) 
	{
	    crx_store[num_crx].p = corners[0][0][1]->posn;
	    node_crx[0][0][gmax[2]-1] = &crx_store[num_crx++];
	}
	if (comp[gmax[0]-1][gmax[1]-1][1] != comp[gmax[0]][gmax[1]-1][1] &&
	    comp[gmax[0]-1][gmax[1]-1][1] != comp[gmax[0]-1][gmax[1]][1] &&
	    comp[gmax[0]-1][gmax[1]-1][1] != comp[gmax[0]-1][gmax[1]-1][0]) 
	{
	    crx_store[num_crx].p = corners[1][1][0]->posn;
	    node_crx[gmax[0]-1][gmax[1]-1][0] = &crx_store[num_crx++];
	}
	if (comp[1][gmax[1]-1][gmax[2]-1] != comp[0][gmax[1]-1][gmax[2]-1] &&
	    comp[1][gmax[1]-1][gmax[2]-1] != comp[1][gmax[1]][gmax[2]-1] &&
	    comp[1][gmax[1]-1][gmax[2]-1] != comp[1][gmax[1]-1][gmax[2]]) 
	{
	    crx_store[num_crx].p = corners[0][1][1]->posn;
	    node_crx[0][gmax[1]-1][gmax[2]-1] = &crx_store[num_crx++];
	}
	if (comp[gmax[0]-1][1][gmax[2]-1] != comp[gmax[0]][1][gmax[2]-1] &&
	    comp[gmax[0]-1][1][gmax[2]-1] != comp[gmax[0]-1][0][gmax[2]-1] &&
	    comp[gmax[0]-1][1][gmax[2]-1] != comp[gmax[0]-1][1][gmax[2]]) 
	{
	    crx_store[num_crx].p = corners[1][0][1]->posn;
	    node_crx[gmax[0]-1][0][gmax[2]-1] = &crx_store[num_crx++];
	}
	if (comp[gmax[0]-1][gmax[1]-1][gmax[2]-1] != 
	    comp[gmax[0]][gmax[1]-1][gmax[2]-1] &&
	    comp[gmax[0]-1][gmax[1]-1][gmax[2]-1] != 
	    comp[gmax[0]-1][gmax[1]][gmax[2]-1] &&
	    comp[gmax[0]-1][gmax[1]-1][gmax[2]-1] != 
	    comp[gmax[0]-1][gmax[1]-1][gmax[2]]) 
	{
	    crx_store[num_crx].p = corners[1][1][1]->posn;
	    node_crx[gmax[0]-1][gmax[1]-1][gmax[2]-1] = &crx_store[num_crx++];
	}

	return num_crx;
}	/* end install_bdry_corner_crx */


EXPORT	int count_crx_through_comp(
	int *gmax,
	COMPONENT ***comp)
{
	int i,j,k,num_crx,num_comp;

	num_crx = 0;
	for (i = 0; i <= gmax[0]; ++i)
	{
	    for (j = 0; j <= gmax[1]; ++j)
	    {
	    	for (k = 0; k <= gmax[2]; ++k)
		{
		    if (i != gmax[0])
		    {
		    	if (comp[i][j][k] != comp[i+1][j][k])
			    ++num_crx;
		    	if (j != gmax[1])
			{
			    if (is_curve_crx(comp[i][j][k],comp[i+1][j][k],
			             comp[i][j+1][k],comp[i+1][j+1][k]))
			   	++num_crx;
			}
		    }
		    if (j != gmax[1])
		    {
		    	if (comp[i][j][k] != comp[i][j+1][k])
			    ++num_crx;
		    	if (k != gmax[2])
			{
			    if (is_curve_crx(comp[i][j][k],comp[i][j][k+1],
			             comp[i][j+1][k],comp[i][j+1][k+1]))
			    	++num_crx;
			}
		    }
		    if (k != gmax[2])
		    {
		    	if (comp[i][j][k] != comp[i][j][k+1])
			    ++num_crx;
		    	if (i != gmax[0])
			{
			    if (is_curve_crx(comp[i][j][k],comp[i][j][k+1],
			             comp[i+1][j][k],comp[i+1][j][k+1]))
			    	++num_crx;
			}
		    }
		}
	    }
	}
	return num_crx;
}	/* end count_crx_through_comp */

/* The criterion to judge a curve_crx is as following:
*   1. exactly 3 components are found on four vertices;
*   2. components on diagonal postions are distinct, say: 
*          comp[0][0] != comp[1][1]
*          comp[0][1] != comp[1][0]
*/

EXPORT	boolean is_curve_crx(
	COMPONENT c00,
	COMPONENT c01,
	COMPONENT c10,
	COMPONENT c11)
{
	int nc;
	if((c00 == c11) || (c01 == c10))
	    return NO;
	nc = count_side_comp(c00,c01,c10,c11);
	if (nc != 3) return NO;
	return YES;
}	/* end is_curve_crx */

LOCAL	int count_side_comp(
	COMPONENT c1,
	COMPONENT c2,
	COMPONENT c3,
	COMPONENT c4)
{
	int nc = 1;
	if (c1 != c2)
	{
	    ++nc;
	    if (c3 != c1 && c3 != c2)
	    {
	    	++nc;
		if (c4 != c1 && c4 != c2 && c4 != c3)
	    	    ++nc;
	    }
	    else if (c4 != c1 && c4 != c2)
	    	++nc;
	}
	else if (c2 != c3)
	{
	    ++nc;
	    if (c4 != c2 && c4 != c3)
                ++nc;
	}
	return nc;
}	/* end count_side_comp */

EXPORT	void alloc_grid_crx_mem(
	EG_CRX *eg_crx,
	int    *gmax,
	int    num_crx,
	boolean   include_curve_crx)
{
	tri_array(&eg_crx->x_crx,gmax[0],gmax[1]+1,gmax[2]+1,
				sizeof(BBI_POINT*));
        tri_array(&eg_crx->y_crx,gmax[0]+1,gmax[1],gmax[2]+1,
				sizeof(BBI_POINT*));
        tri_array(&eg_crx->z_crx,gmax[0]+1,gmax[1]+1,gmax[2],
				sizeof(BBI_POINT*));
	if (include_curve_crx)
	{
	    tri_array(&eg_crx->x_curve_crx,gmax[0]+1,gmax[1],gmax[2],
				sizeof(BBI_POINT*));
            tri_array(&eg_crx->y_curve_crx,gmax[0],gmax[1]+1,gmax[2],
				sizeof(BBI_POINT*));
            tri_array(&eg_crx->z_curve_crx,gmax[0],gmax[1],gmax[2]+1,
				sizeof(BBI_POINT*));
            tri_array(&eg_crx->node_crx,gmax[0],gmax[1],gmax[2],
				sizeof(BBI_POINT*));
	}
        uni_array(&eg_crx->crx_store,num_crx,sizeof(BBI_POINT));
}	/* end alloc_grid_crx_mem */

EXPORT  void free_grid_crx_mem(
	EG_CRX *eg_crx,
	boolean   include_curve_crx)
{
	free_these(4,eg_crx->x_crx,eg_crx->y_crx,eg_crx->z_crx,
		   eg_crx->crx_store);
	if (include_curve_crx)
        {
	    /*#bjet2 */
	    free_these(4,eg_crx->x_curve_crx,eg_crx->y_curve_crx,
	    		eg_crx->z_curve_crx,eg_crx->node_crx);
	}
}	/* end free_grid_crx_mem */


/**********************************************************************
*	This function reset components of the domain to NO_COMP       *
**********************************************************************/

EXPORT	void reset_domain_comp(
	COMPONENT ***comp,
	RECT_GRID gr)
{
	int i,j,k;
	int *gmax = gr.gmax;

	for (i = 0; i <= gmax[0]; ++i)
	{
	    for (j = 0; j <= gmax[1]; ++j)
	    {
	    	for (k = 0; k <= gmax[2]; ++k)
		{
	    	    comp[i][j][k] = NO_COMP;
		}
	    }
	}
}	/* end reset_domain_comp */


/**********************************************************************
*	This function set pos_comp to the positive side of the        *
*	surface func = 0 (func > 0) domain.                           *
**********************************************************************/

EXPORT	void assign_positive_comp(
	double (*func)(POINTER,double*),
	POINTER func_params,
	COMPONENT ***comp,
	RECT_GRID gr,
	COMPONENT pos_comp)
{
	int i,j,k;
	int *gmax = gr.gmax;
	double *L = gr.L;
	double *h = gr.h;
	double coords[3];

	for (i = 0; i <= gmax[0]; ++i)
	{
	    coords[0] = L[0] + i*h[0];
	    for (j = 0; j <= gmax[1]; ++j)
	    {
	    	coords[1] = L[1] + j*h[1];
	    	for (k = 0; k <= gmax[2]; ++k)
		{
	    	    coords[2] = L[2] + k*h[2];
		    if ((*func)(func_params,coords) > 0 &&
		    	comp[i][j][k] == NO_COMP)
		    	comp[i][j][k] = pos_comp;
		}
	    }
	}
}	/* end assign_positive_comp */

/**********************************************************************
*	This function set neg_comp to the negative side of the        *
*	surface func = 0 (func < 0) domain.                           *
**********************************************************************/

EXPORT	void assign_negative_comp(
	double (*func)(POINTER,double*),
	POINTER func_params,
	COMPONENT ***comp,
	RECT_GRID gr,
	COMPONENT neg_comp)
{
	int i,j,k;
	int *gmax = gr.gmax;
	double *L = gr.L;
	double *h = gr.h;
	double coords[3];

	for (i = 0; i <= gmax[0]; ++i)
	{
	    coords[0] = L[0] + i*h[0];
	    for (j = 0; j <= gmax[1]; ++j)
	    {
	    	coords[1] = L[1] + j*h[1];
	    	for (k = 0; k <= gmax[2]; ++k)
		{
	    	    coords[2] = L[2] + k*h[2];
		    if ((*func)(func_params,coords) <= 0 &&
		    	comp[i][j][k] == NO_COMP)
		    	comp[i][j][k] = neg_comp;
		}
	    }
	}
}	/* end assign_negative_comp */

/**********************************************************************
*	This function set cross_comp to the subdomain bounded by      *
*	two surfaces func_1 = 0 and func_2 = 0. The side of each      *
*	surface is given by the input values of side1 and side2.      *
**********************************************************************/

EXPORT	void assign_intersection_comp(
	double (*func_1)(POINTER,double*),
	POINTER func_1_params,
	double (*func_2)(POINTER,double*),
	POINTER func_2_params,
	COMPONENT ***comp,
	RECT_GRID gr,
	COMPONENT cross_comp,
	SIDE side1,
	SIDE side2)
{
	int i,j,k;
	int *gmax = gr.gmax;
	double *L = gr.L;
	double *h = gr.h;
	double coords[3];
	double y1,y2;

	for (i = 0; i <= gmax[0]; ++i)
	{
	    coords[0] = L[0] + i*h[0];
	    for (j = 0; j <= gmax[1]; ++j)
	    {
	    	coords[1] = L[1] + j*h[1];
	    	for (k = 0; k <= gmax[2]; ++k)
		{
	    	    coords[2] = L[2] + k*h[2];
		    y1 = (*func_1)(func_1_params,coords);
		    y2 = (*func_2)(func_2_params,coords);
		    if (comp[i][j][k] != NO_COMP) continue;
		    if (side1 == POSITIVE_SIDE && y1 > 0)
		    {
		    	if (side2 == POSITIVE_SIDE && y2 > 0)
			    comp[i][j][k] = cross_comp;
		    	else if (side2 == NEGATIVE_SIDE && y2 <= 0)
			    comp[i][j][k] = cross_comp;
		    }
		    else if (side1 == NEGATIVE_SIDE && y1 < 0)
		    {
		    	if (side2 == POSITIVE_SIDE && y2 > 0)
			    comp[i][j][k] = cross_comp;
		    	else if (side2 == NEGATIVE_SIDE && y2 <= 0)
			    comp[i][j][k] = cross_comp;
		    }
		}
	    }
	}
}	/* end assign_intersection_comp */

/**********************************************************************
*	This function sets components for a two component domain      *
*	described by the rectangular grid gr, the side func < 0       *
*	will be ft_assigned neg_comp while the side func > 0 will be     *
*	ft_assigned pos_comp.   					      *
**********************************************************************/

LOCAL	void assign_two_comp_domain(
	double (*func)(POINTER,double*),
	POINTER func_params,
	COMPONENT ***comp,
	RECT_GRID gr,
	COMPONENT neg_comp,
	COMPONENT pos_comp)
{
	int i,j,k;
	int *gmax = gr.gmax;
	double *L = gr.L;
	double *h = gr.h;
	double coords[3];

	for (i = 0; i <= gmax[0]; ++i)
	{
	    coords[0] = L[0] + i*h[0];
	    for (j = 0; j <= gmax[1]; ++j)
	    {
	    	coords[1] = L[1] + j*h[1];
	    	for (k = 0; k <= gmax[2]; ++k)
		{
	    	    coords[2] = L[2] + k*h[2];
		    if (comp[i][j][k] != NO_COMP) continue;
		    if ((*func)(func_params,coords) > 0)
		    	comp[i][j][k] = pos_comp;
		    else 
		    	comp[i][j][k] = neg_comp;
		}
	    }
	}
}	/* end assign_two_comp_domain */


EXPORT 	double dumbbell_func(
        POINTER func_params,         
	double *coords)
{         
	DUMBBELL_PARAMS *d_params = (DUMBBELL_PARAMS*)func_params;
        double x0,x1,y,z,f0,f1,R,rr;
        double arg;

        x0 = d_params->x0;
        y = d_params->y;
        z = d_params->z;
        x1  = d_params->x1;
        R = d_params->R;
        rr  = d_params->rr;

        f0 = x0+sqrt(R*R-rr*rr);
        f1 = x1-sqrt(R*R-rr*rr); 
        if (coords[0]<f0)         
            arg = sqrt(sqr(coords[0]-x0)+sqr(coords[1]-y)+sqr(coords[2]-z))-R;
        else if(coords[0] > f1)
            arg = sqrt(sqr(coords[0]-x1)+sqr(coords[1]-y)+sqr(coords[2]-z))-R;
        else
            arg = sqrt(sqr(coords[1]-y)+sqr(coords[2]-z))-rr;
        return -arg;

}       /* end dumbbell_func */

EXPORT	double ellipsoid_func(
	POINTER func_params,
	double *coords)
{
	ELLIP_PARAMS *params;
	const double *cen,*rad;
	double arg;

	params = (ELLIP_PARAMS *)func_params;
	cen = params->cen;
	rad = params->rad;

	arg = 1.0 -
                sqr(coords[0] - cen[0])/sqr(rad[0]) -
                sqr(coords[1] - cen[1])/sqr(rad[1]) -
                sqr(coords[2] - cen[2])/sqr(rad[2]);

	return -arg;
}	/* end ellipsoid_func */

EXPORT	double bdry_box_func(
	POINTER func_params,
	double *coords)
{
	BDRY_BOX_PARAMS *params;
	double *L,*U;
	INTERFACE *intfc;
	int i;

	params = (BDRY_BOX_PARAMS *)func_params;
	L = params->L;
	U = params->U;
	intfc = params->intfc;
	for (i = 0; i < 3; ++i)
	{
	    if (rect_boundary_type(intfc,i,0) != SUBDOMAIN_BOUNDARY &&
		coords[i] < L[i])
		return -1.0;
	    if (rect_boundary_type(intfc,i,1) != SUBDOMAIN_BOUNDARY &&
	    	coords[i] > U[i])
	    	return -1.0;
	}
	return 1.0;
}	/* end bdry_box_func */

EXPORT	double plane_func(
	POINTER func_params,
	double *coords)
{
	PLANE_PARAMS *params;
	const double *N,*P;
	double d;

	params = (PLANE_PARAMS*)func_params;
	N = params->N;	P = params->P;
	d = N[0]*(coords[0] - P[0]) + N[1]*(coords[1] - P[1])
			+ N[2]*(coords[2] - P[2]);
	return d;
}	/* end plane_func */

LOCAL	boolean face_crx_in_dir(
	double (*func1)(POINTER,double*),
	POINTER func1_params,
	double (*func2)(POINTER,double*),
	POINTER func2_params,
	double *L,
	double *U,
	double *crx_crds,
	int dir)
{
	double coords[3],coords1[3],coords2[3];
	boolean lower_set,upper_set;
	int i,k,num_iter = 8;
	double f1,f2;

	lower_set = upper_set = NO;
	for (i = 0; i < 3; i++)
	    coords[i] = L[i];
	f1 = (*func1)(func1_params,coords);
	f2 = (*func2)(func2_params,coords);
	if (f1 < 0 && f2 < 0)
	{
	    lower_set = YES;
	    for (i = 0; i < 3; i++)
	    	coords1[i] = coords[i];
	}
	else if (f1 >= 0 && f2 >= 0)
	{
	    upper_set = YES;
	    for (i = 0; i < 3; i++)
	    	coords2[i] = coords[i];
	}
	coords[(dir+1)%3] = U[(dir+1)%3];
	if (f1 < 0 && f2 < 0)
	{
	    lower_set = YES;
	    for (i = 0; i < 3; i++)
	    	coords1[i] = coords[i];
	}
	else if (f1 >= 0 && f2 >= 0)
	{
	    upper_set = YES;
	    for (i = 0; i < 3; i++)
	    	coords2[i] = coords[i];
	}
	coords[(dir+2)%3] = U[(dir+2)%3];
	if (f1 < 0 && f2 < 0)
	{
	    lower_set = YES;
	    for (i = 0; i < 3; i++)
	    	coords1[i] = coords[i];
	}
	else if (f1 >= 0 && f2 >= 0)
	{
	    upper_set = YES;
	    for (i = 0; i < 3; i++)
	    	coords2[i] = coords[i];
	}
	coords[(dir+1)%3] = L[(dir+1)%3];
	if (f1 < 0 && f2 < 0)
	{
	    lower_set = YES;
	    for (i = 0; i < 3; i++)
	    	coords1[i] = coords[i];
	}
	else if (f1 >= 0 && f2 >= 0)
	{
	    upper_set = YES;
	    for (i = 0; i < 3; i++)
	    	coords2[i] = coords[i];
	}
	if (!(lower_set && upper_set))
	    return NO;
	for (i = 0; i < num_iter; ++i)
	{
	    for (k = 0; k < 3; ++k)
	    	coords[k] = coords1[k];
	    coords[(dir+1)%3] = 0.5*(coords1[(dir+1)%3] + 
	    		coords2[(dir+1)%3]);
	    f1 = (*func1)(func1_params,coords);
	    f2 = (*func2)(func2_params,coords);
	    if (f1 < 0 && f2 < 0)
	    	coords1[(dir+1)%3] = coords[(dir+1)%3];
	    else if ( f1 >= 0 && f2 >= 0)
	    	coords2[(dir+1)%3] = coords[(dir+1)%3];

	    for (k = 0; k < 3; ++k)
	    	coords[k] = coords1[k];
	    coords[(dir+2)%3] = 0.5*(coords1[(dir+2)%3] + 
	    		coords2[(dir+1)%3]);
	    f1 = (*func1)(func1_params,coords);
	    f2 = (*func2)(func2_params,coords);
	    if (f1 < 0 && f2 < 0)
	    	coords1[(dir+1)%3] = coords[(dir+1)%3];
	    else if ( f1 >= 0 && f2 >= 0)
	    	coords2[(dir+1)%3] = coords[(dir+1)%3];
	}

}	/* end face_crx_in_dir */

/*#bjet1 */
#define  MAX_NUM_CURVES   100
EXPORT	void make_grid_surfaces(
	BLK_CRX      *blk_crx,
	EG_CRX 	     *eg_crx,
	int          *gmax,
	boolean         include_curve_crx)
{
	int 	      i,j,k,num_blk;
	BLK_TRI	      *bm,****blk_mem,*blk_mem_store;
	static CURVE  **new_c=NULL;
	static int    tnc, *n_c = NULL;
	    

	num_blk = 0;
	for (i = 0; i < gmax[0]; ++i)
	{
	    for (j = 0; j < gmax[1]; ++j)
	    {
		for (k = 0; k < gmax[2]; ++k)
		{
		    if (onfront_block(i,j,k,eg_crx))
			++num_blk;
		}
	    }
	}
	tri_array(&blk_mem,gmax[2],gmax[1],gmax[0],sizeof(BLK_TRI*));
        uni_array(&blk_mem_store,num_blk,sizeof(BLK_TRI));

	num_blk = 0;
	blk_crx->blk_info->do_volume_frac = NO;
	for (i = 0; i < gmax[0]; ++i)
	{
	    for (j = 0; j < gmax[1]; ++j)
	    {
		for (k = 0; k < gmax[2]; ++k)
		{
		    if (onfront_block(i,j,k,eg_crx))
		    {
			bm = blk_mem[k][j][i] = &blk_mem_store[num_blk++];
			bm->blk_info = blk_crx->blk_info;
			assign_blk_crx(blk_crx,i,j,k,eg_crx,include_curve_crx);
			switch (blk_crx->blk_type)
			{
			case COMP2_BLOCK:
			    construct_comp2_blk(blk_crx,bm);
			    break;
			case COMP3_BLOCK:
			    construct_comp3_blk(blk_crx,bm);
			    break;
			case BDRY_BLOCK:
			    construct_bdry_blk(blk_crx,bm);
			    break;
			default:
			    screen("UNKNOWN BLOCK: code needed!\n");
			    clean_up(ERROR);
			}
			if (i != 0)
			    stitch_adj_blk(blk_mem[k][j][i-1],bm);
			if (j != 0)
			    stitch_adj_blk(blk_mem[k][j-1][i],bm);
			if (k != 0)
			    stitch_adj_blk(blk_mem[k-1][j][i],bm);
		    }
		    else
		        blk_mem[k][j][i] = NULL;
		}
	    }
	}

	/*#bjet2 */
	/*stitch_curves_of_blk(gmax,blk_mem,eg_crx->curves,eg_crx->num_curves); */
	if(n_c == NULL)
	{
	    uni_array(&new_c, MAX_NUM_CURVES,  sizeof(CURVE*));
	    uni_array(&n_c, MAX_NUM_CURVES, sizeof(int));
	}
    
	tnc = make_curves_from_blk(new_c, n_c, gmax,blk_mem,eg_crx->curves,
					eg_crx->num_curves);
	
	/* tnc >= eg_crx->num_curves, if they are not equal,  */
	/* at least one curve is cut into two pieces. */
	if(tnc != eg_crx->num_curves)
	{
	    free(eg_crx->curves);
	    uni_array(&eg_crx->curves,tnc,sizeof(CURVE*));
	    for(i = 0; i < tnc; i++)
		eg_crx->curves[i] = new_c[i];
	    eg_crx->num_curves = tnc;
	}
	
	for (i = 0; i < gmax[0]; ++i)
	{
	    for (j = 0; j < gmax[1]; ++j)
	    {
		for (k = 0; k < gmax[2]; ++k)
		{
		    if ((bm = blk_mem[k][j][i]) != NULL)
		    {
			if (i != gmax[0]-1)
			    remove_null_pair(bm,blk_mem[k][j][i+1],0);
			if (j != gmax[1]-1)
			    remove_null_pair(bm,blk_mem[k][j+1][i],1);
			if (k != gmax[2]-1)
			    remove_null_pair(bm,blk_mem[k+1][j][i],2);
		    }
		}
	    }
	}
	
	/*#bjet  */
	for (i = 0; i < eg_crx->num_curves; ++i)
	    reorder_curve_link_list(eg_crx->curves[i]);

	free_these(2,blk_mem,blk_mem_store);
}	/* end make_grid_surfaces  */

/*#bjet2 */
/* use b as the first bond, link bonds in newb to b, then set the linked bond in  */
/* newb as NULL */
LOCAL  void   stitch_bonds(
	CURVE  *c,
	BOND   *b,
	BOND   **newb,
	int    num_bonds)
{
BOND    *firstb, *lastb;
int	k, j;

	firstb = lastb = b;
	firstb->next = firstb->prev = NULL;
	
	/*two for loops, because after one bond is added to the curve,  */
	/*the curve is changed. */
	for(j=0; j < num_bonds; j++)
	{
	    for(k =0; k < num_bonds; k++)
	    {
                if(newb[k] != NULL && newb[k] != b)
	        {
	            /*(1) new bond is added in the tail of the curve */
                    if(lastb->end == newb[k]->start)
		    {
	                lastb->next = newb[k];
	                newb[k]->prev = lastb;
	                newb[k]->next = NULL;
	                lastb = newb[k];
	                newb[k] = NULL;
	                break;
	            }
	 	    if(lastb->end == newb[k]->end)
		    {
		        reverse_bond(newb[k]);
		        lastb->next = newb[k];
		        newb[k]->prev = lastb;
		        newb[k]->next = NULL;
		        lastb = newb[k];
		        newb[k] = NULL;
		        break;
		    }
	 	    /* (2) new bond is added in the head of the curve */
		    if(firstb->start == newb[k]->end)
		    {
		        firstb->prev = newb[k];
		        newb[k]->next = firstb;
		        newb[k]->prev = NULL;
		        firstb = newb[k];
		        newb[k] = NULL;
		        break;
		    }
		    if(firstb->start == newb[k]->start)
		    {
		        reverse_bond(newb[k]);
		        firstb->prev = newb[k];
		        newb[k]->next = firstb;
		        newb[k]->prev = NULL;
		        firstb = newb[k];
		        newb[k] = NULL;
		        break;
		    }
	        }
	    }
	}

	c->first = firstb;
	c->last = lastb;
	
	c->num_points = num_points_on_curve(c);
}

LOCAL  void    reset_nodes(CURVE *c)
{
BOND  *firstb, *lastb;

	firstb = c->first;
	lastb = c->last;

	/*bonds are cyclic, nodes are not the same */
	if(firstb->start == lastb->end &&
           c->start != c->end)
        {
            if(!delete_from_pointers(c, &(c->start)->out_curves))
            {
                screen("ERROR in reset_nodes(), ");
                screen("delete_from_pointers() 1 failed\n");
                clean_up(ERROR);
            }
            
	    delete_node(c->start);
            c->start = c->end;
            
	    if (!add_to_pointers(c, &(c->start)->out_curves))
            {
                screen("ERROR in reset_nodes(), ");
                screen("add_from_pointers() 1 failed\n");
                clean_up(ERROR);
            }
        }

	/*node is cyclic, points are not the same */
        if(firstb->start != lastb->end &&
               c->start == c->end)
        {
            if(!delete_from_pointers(c, &(c->start)->in_curves))
            {
                screen("ERROR in reset_nodes(), ");
                screen("delete_from_pointers() 2 failed\n");
                clean_up(ERROR);
            }
            if(!delete_from_pointers(c,&(c->end)->out_curves))
            {
                screen("ERROR in reset_nodes(), ");
                screen("delete_from_pointers() 3 failed\n");
                clean_up(ERROR);
            }
            
	    c->start = make_node(firstb->start);
           
	    if (!add_to_pointers(c, &(c->start)->out_curves))
            {
                screen("ERROR in reset_nodes(), ");
                screen("add_from_pointers() 2 failed\n");
                clean_up(ERROR);
            }
            if (!add_to_pointers(c, &(c->end)->in_curves))
            {
                screen("ERROR in reset_nodes(), ");
                screen("add_from_pointers() 3 failed\n");
                clean_up(ERROR);
            }
        }
	
	c->start->posn = firstb->start;
	c->end->posn = lastb->end;
}

/*#bjet1 */
EXPORT  int  make_curves_from_blk(
	CURVE      **new_curves,
	int	   *n_c,
	int        *gmax,
	BLK_TRI    ****blk_mem,
	CURVE      **curves,
	int        num_curves)
{
	int    nc, i, j, k, c_ind, ind;
	int    num_bonds, alloc_bn;
	BOND   *b, *tmpb, **newb, *firstb, *lastb;
	BOND_TRI **btris;
	SURFACE  **s;
	CURVE  *c;
	NODE   *ns, *ne;

	DEBUG_ENTER(stitch_curves_of_blk) 

	alloc_bn = max(gmax[0],max(gmax[1],gmax[2]));
	uni_array(&newb, alloc_bn*alloc_bn, sizeof(BOND*));

	c_ind = 0;
	for(nc = 0; nc < num_curves; nc++)
	{
	    if(curves[nc] == NULL)
	        continue;
            num_bonds = 0;
	    newb[0] = NULL;
	    for (i = 0; i < gmax[2]; ++i)
	    {
	        for (j = 0; j < gmax[1]; ++j)
	        {
	    	    for (k = 0; k < gmax[0]; ++k)
		    {
                        if(blk_mem[i][j][k] != NULL)
			{
			   if(blk_mem[i][j][k]->bonds[nc] != NULL) 
			   {
			       newb[num_bonds] = blk_mem[i][j][k]->bonds[nc];	  
			       /*printf("#newb %d   %d %d %d \n", newb[num_bonds], i, j, k); */
			       
			       num_bonds ++;
			       if(num_bonds == alloc_bn)
			       {
				   BOND  **newb2;    
				   alloc_bn += 100;
				   uni_array(&newb2, alloc_bn, sizeof(BOND*));
				   ft_assign(newb2, newb,sizeof(BOND*)*num_bonds);
				   free(newb);
				   newb = newb2; 
			       }
			   }
			   
			   /*#bjetbond The second bond for the curve. */
			   if(blk_mem[i][j][k]->bonds1[nc] != NULL) 
			   {
			       newb[num_bonds] = blk_mem[i][j][k]->bonds1[nc];	   
			       /*printf("#newba %d   %d %d %d \n", newb[num_bonds], i, j, k); */
			       
			       num_bonds ++;
			       if(num_bonds == alloc_bn)
			       {
				   BOND  **newb2;    
				   alloc_bn += 100;
				   uni_array(&newb2, alloc_bn, sizeof(BOND*));
				   ft_assign(newb2, newb,sizeof(BOND*)*num_bonds);
				   free(newb);
				   newb = newb2; 
			       }
			   }
			} /*if blk_mem */
                    }
		}
            }

	    /*printf("#curves %llu #num bond = %d\n", curve_number(curves[nc]), num_bonds); */
	    
	    if(newb[0] == NULL)
	    {
	        curves[nc]->num_points = 0;
		curves[nc]->first = curves[nc]->last = NULL;
                continue;
	    }

	    /*curve[nc] may be cutted by one processor */
	    /*see split_curve */
	    ind = 0;
	    n_c[nc] = 0;
	    while(ind < num_bonds)
	    {
	        tmpb = newb[ind];
		newb[ind] = NULL;
		
		if(ind == 0)
		{
		    c = curves[nc];
		    stitch_bonds(c, tmpb, newb, num_bonds);	
	            reset_nodes(c);
		}
		else
		{
		    ns = make_node(tmpb->start);
		    ne = make_node(tmpb->end);
		    
		    c = copy_curve(curves[nc],ns,ne);
                    /*keep this infomation for later use */
		    c->number = curves[nc]->number;
		    
		    c->start->posn = tmpb->start;
                    c->end->posn = tmpb->end;
		    
		    stitch_bonds(c, tmpb, newb, num_bonds);	
	            reset_nodes(c);
	
	            for (s = curves[nc]->pos_surfaces; s && *s; ++s)
		        install_curve_in_surface_bdry(*s,c,POSITIVE_ORIENTATION);
	            for (s = curves[nc]->neg_surfaces; s && *s; ++s)
		        install_curve_in_surface_bdry(*s,c,NEGATIVE_ORIENTATION);

	            /*reset the curve pointers in BOND_TRI */
	            for (b = c->first; b; b = b->next)
	            {
		        for (btris = Btris(b); btris && *btris; ++btris)
		            if((*btris)->curve != NULL)
		    	        (*btris)->curve = c;
	            }
		}

		new_curves[c_ind] = c;
		c_ind++;
		n_c[nc]++;
	       
	        /*printf("#c_ind %d  n_pt %d\n", c_ind, c->num_points); */
		for(ind=0; ind < num_bonds; ind++)
	            if(newb[ind] != NULL)
		        break;
	    }
	}
	
	free(newb);
	
	DEBUG_LEAVE(stitch_curves_of_blk) 
	return c_ind;
}	/* end stitch_curves_of_blk */


LOCAL void stitch_curves_of_blk(
	int        *gmax,
	BLK_TRI    ****blk_mem,
	CURVE      **curves,
	int        num_curves)
{
	int    nc, i, j, k;
	int    num_bonds, alloc_bn;
	BOND   *b, **newb, *firstb, *lastb;
        
	DEBUG_ENTER(stitch_curves_of_blk) 

	alloc_bn = max(gmax[0],max(gmax[1],gmax[2]));
	uni_array(&newb, alloc_bn, sizeof(BOND*));

	for(nc = 0; nc < num_curves; nc++)
	{
            num_bonds = 0;
	    newb[0] = NULL;
	    for (i = 0; i < gmax[0]; ++i)
	    {
	        for (j = 0; j < gmax[1]; ++j)
	        {
	    	    for (k = 0; k < gmax[2]; ++k)
		    {
                        if(blk_mem[i][j][k] != NULL)
			{
			   if(blk_mem[i][j][k]->bonds[nc] != NULL) 
			   {
			       newb[num_bonds] = blk_mem[i][j][k]->bonds[nc];	   
			       num_bonds ++;
			       if(num_bonds == alloc_bn)
			       {
				   BOND  **newb2;    
				   alloc_bn += 10;
				   uni_array(&newb2, alloc_bn, sizeof(BOND*));
				   ft_assign(newb2, newb,sizeof(BOND*)*num_bonds);
				   free(newb);
				   newb = newb2; 
			       }
			       /* xcjia fall2006 */
			       {
				   BOND *tb = blk_mem[i][j][k]->bonds[nc];
			       }
			       /* end */
			   }
			   
			}
                    }
		}
            }

	    if(newb[0] == NULL)
	    {
	        curves[nc]->num_points = 0;
		curves[nc]->first = curves[nc]->last = NULL;
                continue;
	    }

	    firstb = lastb = newb[0];
	    firstb->next = firstb->prev = NULL;
	    
	    for(j =1; j < num_bonds; j++)
	    {
		for(k =1; k < num_bonds; k++)
		{
	            if(newb[k] != NULL)
		    {
	                if(lastb->end == newb[k]->start)
		        {
	                    lastb->next = newb[k];
	                    newb[k]->prev = lastb;
	                    newb[k]->next = NULL;
	                    lastb = newb[k];
	                    newb[k] = NULL;
	                    break;
	                 }
			 if(lastb->end == newb[k]->end)
			 {
			     reverse_bond(newb[k]);
		             lastb->next = newb[k];
			     newb[k]->prev = lastb;
			     newb[k]->next = NULL;
			     lastb = newb[k];
			     newb[k] = NULL;
			     break;
			 }
			 /* NEW CASE  */
			 if(lastb->start == newb[k]->start)
			 {
		             if(lastb->prev == NULL)
			     {
			         reverse_bond(newb[k]);
			         firstb = newb[k];
		                 lastb->prev = newb[k];
			         newb[k]->next = lastb;
			         newb[k] = NULL;
			         break;
			     }
			     else
			     {
			         screen("ERROR: stitch_curves_of_blk\n");
				 screen("last bond's prev not NULL\n");
				 clean_up(ERROR);
			     }
			 }
			 if(lastb->start == newb[k]->end)
			 {
		             if(lastb->prev == NULL)
			     {
			         firstb = newb[k];
		                 lastb->prev = newb[k];
			         newb[k]->next = lastb;
			         newb[k] = NULL;
			         break;
			     }
			     else
			     {
			         screen("ERROR: stitch_curves_of_blk\n");
				 screen("last bond's prev not NULL, 2\n");
				 clean_up(ERROR);
			     }
			 }
                         /* END NEW CASE */
			 if(firstb->start == newb[k]->end)
			 {
			     firstb->prev = newb[k];
			     newb[k]->next = firstb;
			     newb[k]->prev = NULL;
			     firstb = newb[k];
			     newb[k] = NULL;
			     break;
			 }
			 if(firstb->start == newb[k]->start)
		         {
			     reverse_bond(newb[k]);
			     firstb->prev = newb[k];
			     newb[k]->next = firstb;
			     newb[k]->prev = NULL;
			     firstb = newb[k];
			     newb[k] = NULL;
			     break;
			 }
			 /* NEW CASE */
			 if(firstb->end == newb[k]->start)
			 {
			     if(firstb->next == NULL)
			     {
				 lastb = newb[k];
			         firstb->next = newb[k];
			         newb[k]->prev = firstb;
			         newb[k] = NULL;
			         break;
			     }
			     else
			     {
			         screen("ERROR: stitch_curves_of_blk\n");
				 screen("first bond's next not NULL\n");
				 clean_up(ERROR);
			     }
			 }
			 if(firstb->end == newb[k]->end)
		         {
			     if(firstb->next == NULL)
			     {
			         reverse_bond(newb[k]);
				 lastb = newb[k];
			         firstb->next = newb[k];
			         newb[k]->prev = firstb;
			         firstb = newb[k];
			         newb[k] = NULL;
			         break;
			     }
			     else
			     {
			         screen("ERROR: stitch_curves_of_blk\n");
				 screen("first bond's next not NULL\n");
				 clean_up(ERROR);
			     }
			 }
			 /* END NEW CASE */
		    }
		}
	    }
	    curves[nc]->first = firstb;
	    curves[nc]->last = lastb;

            /*NEW */
            if(firstb->start == lastb->end &&
               curves[nc]->start != curves[nc]->end)
            {
                if(!delete_from_pointers(curves[nc],
                     &(curves[nc]->start)->out_curves))
                {
                    screen("ERROR in stitch_curves_of_blk(), ");
                    screen("delete_from_pointers() 1 failed\n");
                    clean_up(ERROR);
                }
                delete_node(curves[nc]->start);
                curves[nc]->start = curves[nc]->end;
                if (!add_to_pointers(curves[nc],
                      &(curves[nc]->start)->out_curves))
                {
                    screen("ERROR in stitch_curves_of_blk(), ");
                    screen("add_from_pointers() 1 failed\n");
                    clean_up(ERROR);
                }
            }
            if(firstb->start != lastb->end &&
               curves[nc]->start == curves[nc]->end)
            {
                if(!delete_from_pointers(curves[nc],
                     &(curves[nc]->start)->in_curves))
                {
                    screen("ERROR in stitch_curves_of_blk(), ");
                    screen("delete_from_pointers() 1 failed\n");
                    clean_up(ERROR);
                }
                if(!delete_from_pointers(curves[nc],
                     &(curves[nc]->end)->out_curves))
                {
                    screen("ERROR in stitch_curves_of_blk(), ");
                    screen("delete_from_pointers() 3 failed\n");
                    clean_up(ERROR);
                }
                curves[nc]->start = make_node(firstb->start);
                if (!add_to_pointers(curves[nc],
                     &(curves[nc]->start)->out_curves))
                {
                    screen("ERROR in stitch_curves_of_blk(), ");
                    screen("add_from_pointers() 2 failed\n");
                    clean_up(ERROR);
                }
                if (!add_to_pointers(curves[nc],
                     &(curves[nc]->end)->in_curves))
                {
                    screen("ERROR in stitch_curves_of_blk(), ");
                    screen("add_from_pointers() 3 failed\n");
                    clean_up(ERROR);
                }
            }
	    curves[nc]->start->posn = firstb->start;
	    curves[nc]->end->posn = lastb->end;

	    i = 0;
	    for(firstb = curves[nc]->first; firstb; firstb = firstb->next)
	        i++;
	    if(i != num_bonds)
	    {
		printf("i = %d  num_bonds = %d\n",i,num_bonds);
	        screen("ERROR:stitch_curves_of_blk, "
		       "curve[%d]'s bonds left unhandled\n", nc);
		(void) print_curve(curves[nc]);
		clean_up(ERROR);
	    }

	    curves[nc]->num_points = num_bonds+1;
	}
	free(newb);

	DEBUG_LEAVE(stitch_curves_of_blk) 
}	/* end stitch_curves_of_blk */

EXPORT	SURFACE *prompt_make_level_surface(
	INTERFACE  *intfc,
	RECT_GRID *gr)
{
        COMPONENT pos_comp,neg_comp;
	POINTER func_params;
	double (*func)(POINTER,double*);
	char s[20];
	SURFACE *surf;

        screen("Enter two integers as the negative and positive components: ");
        Scanf("%d %d\n",&neg_comp,&pos_comp);
                                                                                
	screen("Enter the level surface options\n");
	screen("Supported level interface types are \n"
	       "\tDumbbell (d)\n"
	       "\tPlane (p)\n"
	       "\tEllipse (e)\n"
	       "\tMultiple ellipsoids (m)\n"
	       "\tBoundary (b)\n"
	       "\tHyperboloid (h)\n"
	       "\tParaboloid (a)\n"
	       "\tRandomly perturbed interface (r)\n"
	       "\tSine waves (s)\n"
	       "Enter choice: ");
	(void) Scanf("%s\n",s);
	switch (s[0])
	{
	case 'D':
	case 'd':
	    func_params = init_dumbbell_params(gr);
	    func = dumbbell_func;
	    break;
	case 'P':
	case 'p':
	    func_params = init_plane_params(gr);
	    func = plane_func;
	    break;
	case 'M':
	case 'm':
	    func_params = init_multi_ellipsoid_params(gr);
	    func = multi_ellipsoid_func;
	    break;
	case 'E':
	case 'e':
	    func_params = init_ellipsoid_params(gr);
	    func = ellipsoid_func;
	    break;
	case 'H':
	case 'h':
	    func_params = init_hyperboloid_params(gr);
	    func = hyperboloid_func;
	    break;
	case 'A':
	case 'a':
	    func_params = init_paraboloid_params(gr);
	    func = paraboloid_func;
	    break;
	case 'R':
	case 'r':
	    func_params = init_i_random_pert_params(gr);
	    func = i_random_pert_func;
	    break;
	case 'S':
	case 's':
	    func_params = init_sine_wave_params(gr);
	    func = multi_sine_mode_func;
	    break;
	default:
	    screen("Unknown level surface type, please add code.\n");
	    clean_up(ERROR);
	}
	if (!make_level_surface(gr,intfc,pos_comp,neg_comp,func,
                func_params,&surf))
        {
            screen("prompt_make_level_surface() failed!\n");
            clean_up(ERROR);
        }
	interface_reconstructed(intfc) = YES;
	return surf;
}	/* end prompt_make_level_surface */

LOCAL 	POINTER init_dumbbell_params(
	RECT_GRID *gr)
{
	static DUMBBELL_PARAMS params;

	screen("The axis of the dumbbell is along x-axis: ");
	screen("Enter x-coordinates for the centers of two spheres: ");
	Scanf("%f %f\n",&params.x0,&params.x1);
	screen("Enter the y and z coordinates of the axis: ");
	Scanf("%f %f\n",&params.y,&params.z);
	screen("Enter the radius of the two spehers: ");
	Scanf("%f\n",&params.R);
	screen("Enter the radius of the cylinder: ");
	Scanf("%f\n",&params.rr);
	return (POINTER)&params;
}	/* end init_dumbbell_params */

LOCAL 	POINTER init_plane_params(
	RECT_GRID *gr)
{
	static PLANE_PARAMS params;

	screen("Enter the normal vector of the plane: ");
	Scanf("%f %f %f\n",&params.N[0],&params.N[1],&params.N[2]);
	screen("Enter the coordinates of a point the plane passes: ");
	Scanf("%f %f %f\n",&params.P[0],&params.P[1],&params.P[2]);
	return (POINTER)&params;
}	/* end init_plane_params */

LOCAL 	POINTER init_ellipsoid_params(
	RECT_GRID *gr)
{
	static ELLIP_PARAMS params;

	uni_array(&params.cen,MAXD,FLOAT);
	uni_array(&params.rad,MAXD,FLOAT);
	screen("Enter the coordinates of the ellipsoid center: ");
	Scanf("%f %f %f\n",&params.cen[0],&params.cen[1],&params.cen[2]);
	screen("Enter the three radii of the ellipsoid: ");
	Scanf("%f %f %f\n",&params.rad[0],&params.rad[1],&params.rad[2]);
	
	return (POINTER)&params;
}	/* end init_ellipsoid_params */

LOCAL 	POINTER init_multi_ellipsoid_params(
	RECT_GRID *gr)
{
	static M_ELLIP_PARAMS params;
	int i;

	screen("Enter number of ellipoids: ");
	Scanf("%d\n",&params.num_ellip);
	bi_array(&params.cen,params.num_ellip,MAXD,FLOAT);
	bi_array(&params.rad,params.num_ellip,MAXD,FLOAT);

	for (i = 0; i < params.num_ellip; ++i)
	{
	    screen("Enter the center coordinates of ellipsoid %d: ",i);
	    Scanf("%f %f %f\n",&params.cen[i][0],&params.cen[i][1],
				&params.cen[i][2]);
	    screen("Enter the three radii of ellipsoid %d: ",i);
	    Scanf("%f %f %f\n",&params.rad[i][0],&params.rad[i][1],
				&params.rad[i][2]);
	}

	return (POINTER)&params;
}	/* end init_multi_ellipsoid_params */

LOCAL 	POINTER init_hyperboloid_params(
	RECT_GRID *gr)
{
	static ELLIP_PARAMS params;

	uni_array(&params.cen,MAXD,FLOAT);
	uni_array(&params.rad,MAXD,FLOAT);
	screen("Enter the coordinates of the hyperboloid center: ");
	Scanf("%f %f %f\n",&params.cen[0],&params.cen[1],&params.cen[2]);
	screen("The hyperboloid equation is x^2/a^2 + y^2/b^2 - z^2/c^2 = 1\n");
	screen("Enter the three parameters a, b, and c: ");
	Scanf("%f %f %f\n",&params.rad[0],&params.rad[1],&params.rad[2]);
	
	return (POINTER)&params;
}	/* end init_hyperboloid_params */

LOCAL 	POINTER init_paraboloid_params(
	RECT_GRID *gr)
{
	static ELLIP_PARAMS params;

	uni_array(&params.cen,MAXD,FLOAT);
	uni_array(&params.rad,MAXD,FLOAT);
	screen("Enter the coordinates of the hyperboloid center: ");
	Scanf("%f %f %f\n",&params.cen[0],&params.cen[1],&params.cen[2]);
	screen("The paraboloid equation is x^2/a^2 + y^2/b^2 - z = 0\n");
	screen("Enter the two parameters a, and b: ");
	Scanf("%f %f %f\n",&params.rad[0],&params.rad[1]);
	
	return (POINTER)&params;
}	/* end init_paraboloid_params */

LOCAL 	POINTER init_sine_wave_params(
	RECT_GRID *gr)
{
	static FOURIER_POLY fpoly;
	double z0;
	int i,j,dim;

	fpoly.dim = dim = gr->dim;
	fpoly.L = gr->L;
	fpoly.U = gr->U;
	screen("The surface is horizontal to the z-direction\n");
	screen("Enter the mean height of the interface: ");
	Scanf("%f\n",&z0);
	fpoly.z0 = z0;
	screen("Enter number of modes: ");
        Scanf("%d\n",&fpoly.num_modes);
        bi_array(&fpoly.nu,fpoly.num_modes,1,FLOAT);
        uni_array(&fpoly.A,fpoly.num_modes,FLOAT);
        uni_array(&fpoly.phase,fpoly.num_modes,FLOAT);
        for (i = 0; i < fpoly.num_modes; ++i)
        {
	    for (j = 0; j < dim-1; ++j)
	    {
                screen("Enter the frequency of mode %d in direction %d: ",i,j);
                Scanf("%f\n",&fpoly.nu[i][j]);
	    }
            screen("Enter the amplitude of mode %d: ",i);
            Scanf("%f\n",&fpoly.A[i]);
            screen("Enter the phase of mode %d: ",i);
            Scanf("%f\n",&fpoly.phase[i]);
            fpoly.phase[i] *= PI/180.0;
        }
	
	return (POINTER)&fpoly;
}	/* end init_sine_wave_params */

LOCAL 	POINTER init_i_random_pert_params(
	RECT_GRID *gr)
{
	static FOURIER_POLY *fpoly;
	double z0;
	int i,j;

	screen("The surface is horizontal to the z-direction\n");
	screen("Enter the mean height of the interface: ");
	Scanf("%f\n",&z0);
	fpoly = get_fourier_random(gr->L,gr->U,3,"");
	fpoly->z0 = z0;
	
	return (POINTER)fpoly;
}	/* end init_random_pert_params */

LOCAL	double hyperboloid_func(
	POINTER func_params,
	double *coords)
{
	ELLIP_PARAMS *params;
	const double *cen,*rad;
	double arg;

	params = (ELLIP_PARAMS *)func_params;
	cen = params->cen;
        rad = params->rad;

	arg = 1.0 -
                sqr(coords[0] - cen[0])/sqr(rad[0]) -
                sqr(coords[1] - cen[1])/sqr(rad[1]) +
                sqr(coords[2] - cen[2])/sqr(rad[2]);

	return -arg;
}	/* end hyperboloid_func */


LOCAL	double paraboloid_func(
	POINTER func_params,
	double *coords)
{
	ELLIP_PARAMS *params;
	const double *cen,*rad;
	double arg;

	params = (ELLIP_PARAMS *)func_params;
	cen = params->cen;
        rad = params->rad;

	arg = sqr(coords[0] - cen[0])/sqr(rad[0]) +
              sqr(coords[1] - cen[1])/sqr(rad[1]) -
              (coords[2] - cen[2]);

	return -arg;
}	/* end hyperboloid_func */

LOCAL	double i_random_pert_func(
	POINTER func_params,
	double *coords)
{
	FOURIER_POLY *fpoly = (FOURIER_POLY*)func_params;
	double z = fourier_poly(coords,fpoly);

	return coords[2] - z;
}	/* end random_pert_func */

LOCAL	double multi_ellipsoid_func(
	POINTER func_params,
	double *coords)
{
	M_ELLIP_PARAMS *params;
	int num_ellip,i,imin;
	static double **cen,**rad;
	static double *d;
	double arg,min_d;

	params = (M_ELLIP_PARAMS *)func_params;
	num_ellip = params->num_ellip;
	if (d == NULL)
	{
	    uni_array(&d,num_ellip,FLOAT);
	    cen = params->cen;
	    rad = params->rad;
	}

	min_d = HUGE;
	for (i = 0; i < num_ellip; ++i)
	{
	    d[i] = sqr(coords[0] - cen[i][0]) + sqr(coords[1] - cen[i][1])
		+ sqr(coords[2] - cen[i][2]);
	    if (d[i] < min_d)
	    {
		min_d = d[i];
		imin = i;
	    }
	}
	arg = 1.0 -
                sqr(coords[0] - cen[imin][0])/sqr(rad[imin][0]) -
                sqr(coords[1] - cen[imin][1])/sqr(rad[imin][1]) -
                sqr(coords[2] - cen[imin][2])/sqr(rad[imin][2]);

	return -arg;
}	/* end multi_ellipsoid_func */


EXPORT	void prompt_make_comp3_surfaces(
	INTERFACE *intfc,
	RECT_GRID *gr)
{
	ELLIP_PARAMS params1,params2;
	COMPONENT comp0, comp1, comp2;
	SURFACE **surf;
	CURVE   *curv;
	double *cen1, *rad1;
	double *cen2, *rad2;

	cen1 = params1.cen;
	rad1 = params1.rad;
	cen2 = params2.cen;
	rad2 = params2.rad;

	screen("Enter three integers as three components: ");
	Scanf("%d %d %d\n",&comp0,&comp1,&comp2);
	
	screen("Enter the coordinates of the first ellipsoid center: ");
	Scanf("%f %f %f\n",&cen1[0],&cen1[1],&cen1[2]);
	screen("Enter the three radii of the first ellipsoid: ");
	Scanf("%f %f %f\n",&rad1[0],&rad1[1],&rad1[2]);

	screen("Enter the coordinates of the second ellipsoid center: ");
	Scanf("%f %f %f\n",&cen2[0],&cen2[1],&cen2[2]);
	screen("Enter the three radii of the second ellipsoid: ");
	Scanf("%f %f %f\n",&rad2[0],&rad2[1],&rad2[2]);

	set_current_interface(intfc);
	if (!make_comp3_surfaces(gr,comp0,comp1,comp2,ellipsoid_func,
		(POINTER)&params1,ellipsoid_func,(POINTER)&params2,
		&surf,&curv))
	{
	    screen("prompt_make_comp3_surfaces() failed!\n");
	    clean_up(ERROR);
	}
}	/* end prompt_make_comp3_surfaces */
	

EXPORT 	void prompt_make_bdry_surfaces(
	INTERFACE *intfc,
	RECT_GRID *gr)
{
	BDRY_BOX_PARAMS params;
	const char *dir_name[] = {"X","Y","Z"};
	const char *side_name[] = {"lower","upper"};
	int dir,side;
	char s[10];

	for (dir = 0; dir < 3; ++dir)
	{
	    for (side = 0; side < 2; ++side)
	    {
	    	screen("Enter yes to make boundary surface on the %s side "
		       "of %s-direction: ",side_name[side],dir_name[dir]);
		Scanf("%s\n",s);
	    	if (s[0] == 'Y' || s[0] == 'y')
		{
		    rect_boundary_type(intfc,dir,side) = UNKNOWN_BOUNDARY_TYPE;
		}
		else
		{
		    rect_boundary_type(intfc,dir,side) = SUBDOMAIN_BOUNDARY;
		    if (side == 0)
		    	gr->lbuf[dir] += 2;
		    else if (side == 1)
		    	gr->ubuf[dir] += 2;
		}
	    }
	}
	set_rect_grid(gr->L,gr->U,gr->GL,gr->GU,gr->lbuf,gr->ubuf,
			gr->gmax,gr->dim,&gr->Remap,gr);
	if (!make_bdry_surfaces(intfc,gr))
	{
	    screen("make_bdry_surface() failed!\n");
	    clean_up(ERROR);
	}
}	/* end prompt_make_bdry_surfaces */


EXPORT  void print_blk_crx(
        const BLK_CRX *blk_crx)
{
         int i,j,k;
	 for( i = 0; i < 2; ++i)
	 {
	     (void) printf("blk_crx->comp[0][0][%d] = %d\n",
	     			i,blk_crx->comp[0][0][i]);
	     (void) printf("blk_crx->comp[0][1][%d] = %d\n",
	     			i,blk_crx->comp[0][1][i]);
	     (void) printf("blk_crx->comp[1][0][%d] = %d\n",
	     			i,blk_crx->comp[1][0][i]);
	     (void) printf("blk_crx->comp[1][1][%d] = %d\n",
	     			i,blk_crx->comp[1][1][i]);
	 }
	 printf("crx in the block\n");
	 for(k =0; k< 3; k++)
	     for(i = 0; i < 2; i++)
                 for(j =0; j < 2; j++ )
	         {
		      if(blk_crx->crx[k][i][j]->p != NULL)
		      {
		          (void) printf("blk_crx->crx[%d][%d][%d]->p ",k,i,j);
			  (void) printf("on the surface[%llu]\n",
				surface_number(blk_crx->crx[k][i][j]->s));  
		          print_general_vector("p",
			  	Coords(blk_crx->crx[k][i][j]->p),3,"\n");	  
		      }
		  }
	     
         (void) printf("curve_crx in the block\n");
	 for(i =0; i < 3; i++)
	     for(j =0; j < 2; j++)
	     {
		 if(blk_crx->curve_crx[i][j]->p != NULL)
		 {
		     (void) printf("blk_crx->curve_crx[%d][%d]->p ",
		     		i,j);
		     (void) printf("on the curve[%llu]\n",
			   	curve_number(blk_crx->curve_crx[i][j]->c));
		     print_general_vector("p",
		     		Coords(blk_crx->curve_crx[i][j]->p),3,"\n");
		 }
	     }
}	/* end print_blk_crx */
#endif /* defined(THREED) */

EXPORT	boolean grid_line_crx_in_dir(
	double (*func)(POINTER,double*),
	POINTER func_params,
	int dim,
	double *crds1,
	double *crds2,
	double *crx_crds,
	int dir)
{
	double f1,f2,fm,a1,a2, gtol = 10*MACH_EPS;
	int i,num_iter = 50;
	double coords1[MAXD],coords2[MAXD];

	for (i = 0; i < dim; ++i)
	{
	    crx_crds[i] = coords1[i] = crds1[i];
	    coords2[i] = crds2[i];
	}

	f1 = (*func)(func_params,coords1);
	f2 = (*func)(func_params,coords2);
	if (fabs(f1) <= gtol)
	{
	    crx_crds[dir] = coords1[dir];
	    return YES;
	}
	else if (fabs(f2) <= gtol)
	{
	    crx_crds[dir] = coords2[dir];
	    return YES;
	}

	if ((f1 > 0.0 && f2 > 0.0) ||
	    (f1 < 0.0 && f2 < 0.0))
	    return NO;

	for (i = 0; i < num_iter; ++i)
	{
	    crx_crds[dir] = 0.5*(coords1[dir] + coords2[dir]);
	    fm = (*func)(func_params,crx_crds);
	    if (fabs(fm) <= gtol)
	    {
	    	return YES;
	    }
	    else if ((f1 > 0.0 && fm > 0.0) ||
	    	     (f1 < 0.0 && fm < 0.0))
	    {
	    	coords1[dir] = crx_crds[dir];
		f1 = fm;
	    }
	    else
	    {
	    	coords2[dir] = crx_crds[dir];
		f2 = fm;
	    }
	}
	a1 = (fm - f1)/(f2 - f1);
	a2 = (f2 - fm)/(f2 - f1);
	crx_crds[dir] = a1*coords2[dir] + a2*coords1[dir];
	return YES;
}	/* end grid_line_crx_in_dir */

EXPORT 	void set_grid_for_surface_construction(
	RECT_GRID *dual_gr,
	RECT_GRID *rgr)
{
	int i,gmax[MAXD];
	double L[MAXD],U[MAXD];
	int dim = rgr->dim;

	set_dual_grid(dual_gr,rgr);
	for (i = 0; i < dim; ++i)
	{
	    L[i] = dual_gr->VL[i];
	    U[i] = dual_gr->VU[i];
	    gmax[i] = dual_gr->gmax[i];
	    if (dual_gr->lbuf[i] != 0)
	    {
	    	L[i] -= dual_gr->h[i];
		gmax[i] += dual_gr->lbuf[i] + 1;
	    }
	    if (dual_gr->ubuf[i] != 0)
	    {
	    	U[i] += dual_gr->h[i];
		gmax[i] += dual_gr->ubuf[i] + 1;
	    }
	}
	set_rect_grid(L,U,dual_gr->GL,dual_gr->GU,NULL,NULL,gmax,
			dim,&rgr->Remap,dual_gr);
}	/* end set_grid_for_surface_construction */

/***********************new make surface alg. ********************/
LOCAL   boolean    grid_crx_from_comp(int (*func)(POINTER,double*),
                        POINTER,int,double*,double*,double*,int);
LOCAL	int     install_grid_crx_from_comp(
	int       (*func)(POINTER,double*), POINTER, EG_CRX*, RECT_GRID,int);
LOCAL   int     install_grid_curve_crx_from_comp(
	int       (*func)(POINTER,double*), POINTER, EG_CRX*, RECT_GRID,int*);
LOCAL 	void    assign_comp(int (*func)(POINTER,double*),POINTER,COMPONENT***,RECT_GRID);
LOCAL	int	make_surfaces_from_crx(INTERFACE*,EG_CRX*,int*,int);
LOCAL	int	make_curves_from_crx(INTERFACE*,EG_CRX*,int*,int);

#define  MAX_CURVE   50

LOCAL   void    fix_seg_crx_in_dir(
	double   *crx,
	double   *c1,
	double   *c2,
	double   *h,
	int	dir)
{
double	tol = 0.004;

	if(crx[dir] - c1[dir] < tol*h[dir])
		crx[dir] = c1[dir] + tol*h[dir];
	if(c2[dir] - crx[dir] < tol*h[dir])
		crx[dir] = c2[dir] - tol*h[dir];
}

LOCAL   void    fix_face_crx_in_dir(
	double   *crx,
	double   **crds,
	double   *h,
	int	dir)
{
double	tol = 0.004;
int	dir1;

	crx[dir] = crds[0][dir];

	dir1 = (dir+1)%3;
	if(crx[dir1] - crds[0][dir1] < tol*h[dir1])
	    crx[dir1] = crds[0][dir1] + tol*h[dir1];
	if(crds[2][dir1] - crx[dir1] < tol*h[dir1])
	    crx[dir1] = crds[2][dir1] - tol*h[dir1];
	
	dir1 = (dir+2)%3;
	if(crx[dir1] - crds[0][dir1] < tol*h[dir1])
	    crx[dir1] = crds[0][dir1] + tol*h[dir1];
	if(crds[2][dir1] - crx[dir1] < tol*h[dir1])
	    crx[dir1] = crds[2][dir1] - tol*h[dir1];
}

/*set up p field for  BBI_POINT */
LOCAL	int install_grid_crx_from_comp(
	int       (*func)(POINTER,double*),
        POINTER   func_params,
	EG_CRX    *eg_crx,
	RECT_GRID grid,
	int	  n_crx)
{
	double coords1[3];
	double coords2[3];
	double crds_crx[3];
	double *L = grid.L;
	double *h = grid.h;
	int *gmax = grid.gmax;
	int dim = grid.dim;
	int i,j,k;
	BBI_POINT ****x_crx = eg_crx->x_crx;
	BBI_POINT ****y_crx = eg_crx->y_crx;
	BBI_POINT ****z_crx = eg_crx->z_crx;
	BBI_POINT *crx_store = eg_crx->crx_store;
	COMPONENT ***comp = eg_crx->comp;

	/* install x-crossings  */

	for (j = 0; j <= gmax[1]; ++j)
	{
	    coords1[1] = coords2[1] = L[1] + j*h[1];
	    for (k = 0; k <= gmax[2]; ++k)
	    {
		coords1[2] = coords2[2] = L[2] + k*h[2];
		for (i = 0; i < gmax[0]; ++i)
		{
		    if(comp[i][j][k] != comp[i+1][j][k])
		    {
		        coords1[0] = L[0] + i*h[0];
		        coords2[0] = L[0] + (i+1)*h[0];
			if (! grid_crx_from_comp(func,func_params,
				dim,coords1,coords2,crds_crx,0))
			{
			    screen("ERROR: in install_grid_crx(), no x-crxing!");
			    clean_up(ERROR);
			}
			fix_seg_crx_in_dir(crds_crx, coords1, coords2, h, 0);
		
			crx_store[n_crx].p = Point(crds_crx);
			x_crx[i][j][k] = &crx_store[n_crx++];
		    }
		}
	    }
	}

	/* install y-crossings  */

	for (i = 0; i <= gmax[0]; ++i)
	{
	    coords1[0] = coords2[0] = L[0] + i*h[0];
	    for (k = 0; k <= gmax[2]; ++k)
	    {
		coords1[2] = coords2[2] = L[2] + k*h[2];
		for (j = 0; j < gmax[1]; ++j)
		{
		    if(comp[i][j][k] != comp[i][j+1][k])
		    {
		        coords1[1] = L[1] + j*h[1];
		        coords2[1] = L[1] + (j+1)*h[1];
			if (! grid_crx_from_comp(func,func_params,
				dim,coords1,coords2,crds_crx,1))
			{
			    screen("ERROR: in install_grid_crx(), no y-crxing!");
			    clean_up(ERROR);
			}
			fix_seg_crx_in_dir(crds_crx, coords1, coords2, h, 1);
			
			crx_store[n_crx].p = Point(crds_crx);
			y_crx[i][j][k] = &crx_store[n_crx++];
		    }
		}
	    }
	}

	/* install z-crossings  */

	for (i = 0; i <= gmax[0]; ++i)
	{
	    coords1[0] = coords2[0] = L[0] + i*h[0];
	    for (j = 0; j <= gmax[1]; ++j)
	    {
		coords1[1] = coords2[1] = L[1] + j*h[1];
		for (k = 0; k < gmax[2]; ++k)
		{
		    if(comp[i][j][k] != comp[i][j][k+1])
		    {
		        coords1[2] = L[2] + k*h[2];
		        coords2[2] = L[2] + (k+1)*h[2];
			if (! grid_crx_from_comp(func,func_params,
				dim,coords1,coords2,crds_crx,2))
			{
			    screen("ERROR: in install_grid_crx(), no z-crxing!");
			    clean_up(ERROR);
			}
			fix_seg_crx_in_dir(crds_crx, coords1, coords2, h, 2);
			
			crx_store[n_crx].p = Point(crds_crx);
			z_crx[i][j][k] = &crx_store[n_crx++];
		    }
		}
	    }
	}

	return n_crx;

}	/* end install_grid_crx  */

LOCAL	boolean grid_crx_from_comp(
	int      (*func)(POINTER,double*),
	POINTER  func_params,
	int      dim,
	double    *crds1,
	double    *crds2,
	double    *crx_crds,
	int      dir)
{
	double   f1,f2,fm;
	int     i,num_iter = 50;
	double   coords1[MAXD], coords2[MAXD];

	ft_assign(coords1, crds1, dim*FLOAT);
	ft_assign(coords2, crds2, dim*FLOAT);
	ft_assign(crx_crds, crds1, dim*FLOAT);
	
	f1 = (*func)(func_params,coords1);
	f2 = (*func)(func_params,coords2);

	if(f1 == f2)
	    return NO;

	for (i = 0; i < num_iter; ++i)
	{
	    crx_crds[dir] = 0.5*(coords1[dir] + coords2[dir]);
	    fm = (*func)(func_params, crx_crds);
	    
	    if(fm == f1)
	    	coords1[dir] = crx_crds[dir];
	    else  if(fm == f2)
	    	coords2[dir] = crx_crds[dir];
	    else
	    {
	        printf("ERROR grid_crx_from_comp: more than 2 comp in line\n");
		clean_up(ERROR);
	    }
	}
	
	return YES;
}	/* end grid_line_crx_in_dir  */

LOCAL	void assign_comp(
	int (*func)(POINTER,double*),
	POINTER func_params,
	COMPONENT ***comp,
	RECT_GRID gr)
{
	int i,j,k;
	int *gmax = gr.gmax;
	double *L = gr.L;
	double *h = gr.h;
	double coords[3];

	for (i = 0; i <= gmax[0]; ++i)
	{
	    coords[0] = L[0] + i*h[0];
	    for (j = 0; j <= gmax[1]; ++j)
	    {
	    	coords[1] = L[1] + j*h[1];
	    	for (k = 0; k <= gmax[2]; ++k)
		{
	    	    coords[2] = L[2] + k*h[2];
		    if(comp[i][j][k] == NO_COMP)
		    	comp[i][j][k] = (*func)(func_params, coords);
		}
	    }
	}
}	/* end assign_positive_comp  */

/*ASSUME parallel to the surface. */
LOCAL   boolean face_crx_from_comp(
	double    face_crx[],
	int      (*func)(POINTER,double*),
	POINTER  func_params,
	double    **coords,
	int	 dir)
{
int	i, dir1, dir2;
double	crds1[3];

	for(i=0; i<4; i++)
	    if( (*func)(func_params, coords[i]) == 
	        (*func)(func_params, coords[(i+1)%4]) )
	        break;

	if(i == 4)
	{
	    printf("ERROR face_crx_from_comp, do not have face crx.\n");
	    clean_up(ERROR);
	}

	/*ASSUME direction   0-1 is in (dir+1)%3 direction  */
	if(i == 0 || i == 2)
	{
	    dir1 = (dir + 1)%3;
	    dir2 = (dir + 2)%3;
	}
	else
	{
	    dir1 = (dir + 2)%3;
	    dir2 = (dir + 1)%3;
	}
	
	if(!grid_crx_from_comp(func, func_params, 3, 
			       coords[(i+3)%4], coords[i], crds1, dir2))
	{
	    printf("ERROR face_crx_from_comp, no crx in first direction %d\n", dir2);
	    printf("dir = %d, i = %d\n", dir, i);
	    clean_up(ERROR);
	}
	if(!grid_crx_from_comp(func, func_params, 3, 
			       coords[(i+2)%4], coords[(i+3)%4], face_crx, dir1))
	{
	    printf("ERROR face_crx_from_comp, no crx in second direction %d\n", dir1);
	    clean_up(ERROR);
	}

	face_crx[dir2] = crds1[dir2];
	
	if(debugging("face_crx_from_comp"))
	{
	    printf("%d  %d  %d\n",i, dir1, dir2);
	    print_general_vector("crds1", crds1, 3, "\n");
	    print_general_vector("face_crx", face_crx, 3, "\n");
	    for(i=0; i<4; i++)
	    {
	        print_general_vector("coords", coords[i], 3, "\n");
	    }
	}

	return YES;
}

LOCAL   void set_face_coords(
	double    **crds,
	int	 *ip,
	double    *L,
	double	 *h,
	int	 dir)
{
int	inc[4][3] = {{0,0,0},{0,1,0},{0,1,1},{0,0,1}};
int	i, dir1, dir2;

	dir1 = (dir + 1)%3;
	dir2 = (dir + 2)%3;

	for(i=0; i<4; i++)
	{
	    crds[i][dir]  = L[dir]  +  (ip[dir] + inc[i][0])*h[dir]; 
	    crds[i][dir1]  = L[dir1]  +  (ip[dir1] + inc[i][1])*h[dir1]; 
	    crds[i][dir2]  = L[dir2]  +  (ip[dir2] + inc[i][2])*h[dir2]; 
	}

}

LOCAL  int install_grid_curve_crx_from_comp(
        int       (*func)(POINTER,double*),
	POINTER   func_params,
	EG_CRX    *eg_crx,
	RECT_GRID grid,
	int       *n_crx)
{
	double      **face_coords, face_crds_crx[3];
	double      *L = grid.L;
	double      *h = grid.h;
	int        *gmax = grid.gmax;
	int        i,j,k;
	int        n_curve_crx, ip[3], cc[4];
	BBI_POINT  ****x_curve_crx = eg_crx->x_curve_crx;
	BBI_POINT  ****y_curve_crx = eg_crx->y_curve_crx;
	BBI_POINT  ****z_curve_crx = eg_crx->z_curve_crx;
	BBI_POINT  *crx_store = eg_crx->crx_store;
	COMPONENT  ***comp = eg_crx->comp;
	
	bi_array(&face_coords, 4, 3, FLOAT);

	n_curve_crx = 0;

	/* install x-crossings  */
        for (k = 0; k < gmax[2]; ++k)
        {
	    ip[2] = k;
            for (j = 0; j < gmax[1]; ++j)
            {
	        ip[1] = j;
		for (i = 0; i <= gmax[0]; ++i)
		{
		    ip[0] = i;

		    if (is_curve_crx(comp[i][j][k],comp[i][j+1][k],
		            comp[i][j][k+1],comp[i][j+1][k+1]))
		    {
			set_face_coords(face_coords, ip, L, h, 0);
			face_crx_from_comp(face_crds_crx,func,func_params,face_coords,0);

			fix_face_crx_in_dir(face_crds_crx, face_coords, h, 0);

			crx_store[*n_crx].p = Point(face_crds_crx);
			x_curve_crx[i][j][k] = &crx_store[(*n_crx)++];
			n_curve_crx++;
		    }
		}
	    }
	}

	/* install y-crossings  */
        for (k = 0; k < gmax[2]; ++k)
        {
            ip[2] = k;
 	    for (i = 0; i < gmax[0]; ++i)
            {
	        ip[0] = i;
		for (j = 0; j <= gmax[1]; ++j)
		{
	            ip[1] = j;
		    
		    if (is_curve_crx(comp[i][j][k],comp[i][j][k+1],
		            comp[i+1][j][k],comp[i+1][j][k+1]))
		    {
		        set_face_coords(face_coords, ip, L, h, 1);
			face_crx_from_comp(face_crds_crx,func,func_params,face_coords,1);
			
			fix_face_crx_in_dir(face_crds_crx, face_coords, h, 1);
			
			crx_store[*n_crx].p = Point(face_crds_crx);
    	 	        y_curve_crx[i][j][k] = &crx_store[(*n_crx)++];
			n_curve_crx++;
		    }
		}
	    }
	}

	/* install z-crossings  */
	for (i = 0; i < gmax[0]; ++i)
	{
	    ip[0] = i;
	    for (j = 0; j < gmax[1]; ++j)
	    {
		ip[1] = j;
		for (k = 0; k <= gmax[2]; ++k)
		{
		    ip[2] = k;
		    
		    if (is_curve_crx(comp[i][j][k],comp[i+1][j][k],
		            comp[i][j+1][k],comp[i+1][j+1][k]))
		    {   
			set_face_coords(face_coords, ip, L, h, 2);
			face_crx_from_comp(face_crds_crx,func,func_params,face_coords,2);
			
			fix_face_crx_in_dir(face_crds_crx, face_coords, h, 2);
	
			crx_store[*n_crx].p = Point(face_crds_crx);
			z_curve_crx[i][j][k] = &crx_store[(*n_crx)++];
			n_curve_crx++;
		    }
		}
	    }
	}

	free(face_coords);
	return n_curve_crx;

}	/* end install_curve_crx  */


LOCAL	int	make_surfaces_from_crx(
	INTERFACE  *intfc,
	EG_CRX	   *eg_crx,
	int        *gmax,
	int	   num_crx)
{
	INTERFACE  *sav_intfc = current_interface();
	GRID_DIRECTION   dir[3] = {EAST,NORTH,UPPER};
	SURFACE	   *news[50];
	COMPONENT  ***comp = eg_crx->comp;
	BBI_POINT  ****x_crx = eg_crx->x_crx;
	BBI_POINT  ****y_crx = eg_crx->y_crx;
	BBI_POINT  ****z_crx = eg_crx->z_crx;
	BBI_POINT  *s_crx;
	int	   ea[50][3],nea,c1,c2, ctmp, **mem_ind;
	int	   i,j,k,ix,iy,iz, num,d;
	int        comp1, comp2;

	DEBUG_ENTER(make_surfaces_from_crx);

	bi_array(&mem_ind, num_crx, 5, INT);
	/*#bjet ASSUME */
	get_default_fluid_comp(&comp1,&comp2,intfc);
	
	set_current_interface(intfc);

	num = 0;
	nea = 0;
	for (iz = 0; iz <= gmax[2]; ++iz)
	{
	    for (iy = 0; iy <= gmax[1]; ++iy)
	    {
	        for (ix = 0; ix <= gmax[0]; ++ix)
	        {
	            for (i = 0; i < 3; ++i)
	            {
	                if (ix == gmax[0] && dir[i] == EAST)
	                    continue;
	                if (iy == gmax[1] && dir[i] == NORTH)
	                    continue;
	                if (iz == gmax[2] && dir[i] == UPPER)
	                    continue;
		
			c1 = comp[ix][iy][iz];
			c2 = dir[i] == EAST ? comp[ix+1][iy][iz] : 
			     dir[i] == NORTH ? comp[ix][iy+1][iz] : 
			     comp[ix][iy][iz+1];
			if(c1 == c2)	
			    continue;

			for(j=0; j<nea; j++)
			    if((ea[j][0] == c1 && ea[j][1] == c2) ||
			       (ea[j][0] == c2 && ea[j][1] == c1))
			        break;
			if(j == nea)
			{
			    ea[j][2] = 0;
			    /*use assumption ea[j][0] is in fluid */
			  
			    /*c2: pos  c1: neg */
			    if(c2 < c1)
			    {
			        ctmp = c2;
				c2 = c1;
				c1 = ctmp;
			    }
			    if((c2 != comp1 && c2 != comp2) &&
			       (c1 == comp1 || c1 == comp2) )
			    {
			        ctmp = c2;
				c2 = c1;
				c1 = ctmp;
			    }
		   
			    ea[j][0] = c2;
			    ea[j][1] = c1;

			    nea++;
			}

			mem_ind[num][0] = ix;
			mem_ind[num][1] = iy;
			mem_ind[num][2] = iz;
			mem_ind[num][3] = dir[i];
			mem_ind[num][4] = j;
			num++;
	            }
		}
	    }
	}

	/*printf("#nea_init=%d   %d %d\n", nea, num, num_crx); */
	for(i=0; i<nea; i++)
	{
	    /*printf("#making surf %d %d\n", ea[i][1], ea[i][0]); */
	    news[i] = make_surface(ea[i][1], ea[i][0], NULL, NULL);
	    first_tri(news[i]) = last_tri(news[i]) = NULL;
	    news[i]->num_tri = 0;
	}

	/*set the s field for crxs */
	for(i=0; i<num; i++)
	{
	    ix = mem_ind[i][0];
	    iy = mem_ind[i][1];
	    iz = mem_ind[i][2];
	    d = mem_ind[i][3];
	    
	    s_crx = d == EAST ? x_crx[ix][iy][iz] : 
		    d == NORTH ? y_crx[ix][iy][iz] : z_crx[ix][iy][iz];
	    
	    s_crx->s = news[mem_ind[i][4]];
	}

	/*Assign eg_crx */
	if(nea == 0)
	{
	    eg_crx->num_surfaces = 0;
	    eg_crx->surfaces = NULL;
	}
	else
	{
	    eg_crx->num_surfaces = nea;
	    uni_array(&eg_crx->surfaces, nea, sizeof(SURFACE *));
	    for(i=0; i<nea; i++)
	        eg_crx->surfaces[i] = news[i];
	}
	
	free(mem_ind);

	set_current_interface(sav_intfc);
	
	DEBUG_LEAVE(make_surfaces_from_crx);

	return nea;
}


LOCAL	int  make_curves_from_crx(
	INTERFACE	*intfc, 
	EG_CRX	        *eg_crx,
	int             *gmax,
	int		num_curve_crx)
{
	BBI_POINT  ****x_crx = eg_crx->x_crx;
	BBI_POINT  ****y_crx = eg_crx->y_crx;
	BBI_POINT  ****z_crx = eg_crx->z_crx;
	BBI_POINT  ****x_curve_crx = eg_crx->x_curve_crx;
	BBI_POINT  ****y_curve_crx = eg_crx->y_curve_crx;
	BBI_POINT  ****z_curve_crx = eg_crx->z_curve_crx;
	BBI_POINT  *c_crx, *s_crx;
	SURFACE    *sp[4];
	GRID_DIRECTION  dir[3] = {EAST,NORTH,UPPER};
	NODE		*ns, *ne;
	int		ix,iy,iz, i,j,k, num, d, **mem_ind;
	static O_SURFACE    **sarr=NULL;
	CURVE		*newc[MAX_CURVE];
	FT_POINT		*pt[MAX_CURVE][2];
	int             nsurf=0, scnt[MAX_CURVE];
	
        DEBUG_ENTER(make_curves_from_crx)
	
	if(sarr == NULL)
	    bi_array(&sarr, MAX_CURVE, 3, sizeof(O_SURFACE));
	bi_array(&mem_ind, num_curve_crx, 5, INT);

	set_wall_flag_for_surface(intfc);
	
	num = 0;
	for (iz = 0; iz <= gmax[2]; ++iz)
	for (iy = 0; iy <= gmax[1]; ++iy)
	for (ix = 0; ix <= gmax[0]; ++ix)
	for (i = 0; i < 3; ++i)
	{
	    if (ix == gmax[0] && (dir[i] == NORTH || dir[i] == UPPER))
	        continue;
	    if (iy == gmax[1] && (dir[i] == EAST || dir[i] == UPPER))
	        continue;
	    if (iz == gmax[2] && (dir[i] == EAST || dir[i] == NORTH))
	        continue;

		/*get the curve crx at the face and also get the 3 surfaces */
		/*at the edges of the face. */
	        k = 0;
		switch(dir[i])
		{
		case EAST:
		      c_crx = x_curve_crx[ix][iy][iz];
		      if(c_crx == NULL)   continue;
		      for(j=0; j<4; j++)
		      {
		          s_crx = j%2 == 0 ? 
			  (j == 0 ? y_crx[ix][iy][iz] : y_crx[ix][iy][iz+1]) : 
			  (j == 1 ? z_crx[ix][iy+1][iz] : z_crx[ix][iy][iz]); 
			  if(s_crx == NULL)  continue;
			  sp[k] = s_crx->s;
			  k++;
		      }
		      break;
		case NORTH:
		      c_crx = y_curve_crx[ix][iy][iz];
		      if(c_crx == NULL)   continue;
		      for(j=0; j<4; j++)
		      {
		          s_crx = j%2 == 0 ? 
			  (j == 0 ? z_crx[ix][iy][iz] : z_crx[ix+1][iy][iz]) : 
			  (j == 1 ? x_crx[ix][iy][iz+1] : x_crx[ix][iy][iz]); 
			  if(s_crx == NULL)   continue;
			  sp[k] = s_crx->s;
			  k++;
		      }
		      break;
		case UPPER:
		      c_crx = z_curve_crx[ix][iy][iz];
		      if(c_crx == NULL)   continue;
		      for(j=0; j<4; j++)
		      {
		          s_crx = j%2 == 0 ? 
			  (j == 0 ? x_crx[ix][iy][iz] : x_crx[ix][iy+1][iz]) : 
			  (j == 1 ? y_crx[ix+1][iy][iz] : y_crx[ix][iy][iz]); 
			  if(s_crx == NULL)   continue;
			  sp[k] = s_crx->s;
			  k++;
		      }
		      break;
		}
		if(k != 3)
		{
		    printf("ERROR make_curves_from_comp, "
		           "invalid number of edge crxs %d\n", k);
		    clean_up(ERROR);
		}

		k = add_to_o_surfaces(sarr,&nsurf,sp);
		
		/*use crx_num to store the cruve index  */

		if(k == -1)
		{ /*new 3 comp curve found */
		    k = nsurf-1;
		    scnt[k] = 1;
		    pt[k][0] = c_crx->p;   /*1st node posn */
		}
		else  if(scnt[k] == 1)
		{ /*already in the array */
		    scnt[k] = 2;
		    pt[k][1] = c_crx->p;   /*2ed node posn */
		}
		mem_ind[num][0] = ix;
		mem_ind[num][1] = iy;
		mem_ind[num][2] = iz;
		mem_ind[num][3] = dir[i];
		mem_ind[num][4] = k;
		num++;
	}

	for(i=0; i<nsurf; i++)
	{    
	    if(scnt[i] !=2)
	    {
	        printf("ERROR make_wall_curves, only one curve crx, "
		       "impossible.\n");
	        clean_up(ERROR);
	    }
	    
	    ns = make_node(pt[i][0]);
	    ne = make_node(pt[i][1]);
	    
	    newc[i] = make_curve(NO_COMP, NO_COMP, ns, ne);
	    newc[i]->last = NULL;
	    newc[i]->first = NULL;
	    for(j=0; j<3; j++)
	    {
		install_curve_in_surface_bdry(sarr[i][j].surface, newc[i],
					      sarr[i][j].orient);
	    }
	}

	for(i=0; i<num; i++)
	{
	    ix = mem_ind[i][0];
	    iy = mem_ind[i][1];
	    iz = mem_ind[i][2];
	    d = mem_ind[i][3];
	    
	    c_crx = d == EAST ? x_curve_crx[ix][iy][iz] : 
		    d == NORTH ? y_curve_crx[ix][iy][iz] : 
		    z_curve_crx[ix][iy][iz];
	    
	    c_crx->c = newc[mem_ind[i][4]];
	}
	
	/*Assign eg_crx */
	if(nsurf == 0)
	{
	    eg_crx->num_curves = 0;
	    eg_crx->curves = NULL;
	}
	else
	{
	    eg_crx->num_curves = nsurf;
	    uni_array(&eg_crx->curves, nsurf, sizeof(CURVE *));
	    for(i=0; i<nsurf; i++)
	        eg_crx->curves[i] = newc[i];
	}
	
	free(mem_ind);
        
	DEBUG_LEAVE(make_curves_from_crx)
	
	return nsurf;
}

EXPORT	void show_comp(
	COMPONENT ***comp,
	RECT_GRID gr)
{
	int i,j,k;
	int *gmax = gr.gmax;
	double *L = gr.L;
	double *h = gr.h;

	printf("gmax=%d %d %d\n", gmax[0], gmax[1], gmax[2]);
	
	for (i = 0; i <= gmax[0]; ++i)
	{
	    printf("ix = %d\n", i);
	    for (j = 0; j <= gmax[1]; ++j)
	    {
	    	for (k = 0; k <= gmax[2]; ++k)
		{
		    printf("%2d", comp[i][j][k]);
		}
		printf("\n");
	    }
	    printf("\n");
	}
}	/* end show_comp */


EXPORT boolean make_surfaces_from_comp(
	RECT_GRID   *rgr,
	int         (*func)(POINTER, double*),
	POINTER     func_params,
	SURFACE     **s,
	CURVE       **c,
	int         *num_surfs,
	int         *num_curves)
{
	INTERFACE	*intfc = current_interface();
	int		i,j, num_crx, num_curve_crx, *gmax;
	RECT_GRID	dual_gr;
	EG_CRX		Eg_crx;
	BLK_INFO	blk_info;
	static BLK_CRX  *blk_crx = NULL;
	int		is = 0;
	int             ic = 0;

	if (blk_crx == NULL) 
	{
	    blk_crx = alloc_blk_crx(NO);
	}
        
	zero_scalar(&Eg_crx,sizeof(EG_CRX));
	set_grid_for_surface_construction(&dual_gr,rgr);

	gmax = dual_gr.gmax;
	tri_array(&Eg_crx.comp,gmax[0]+1,gmax[1]+1,gmax[2]+1,
			sizeof(COMPONENT));
        
	reset_domain_comp(Eg_crx.comp,dual_gr);
	assign_comp(func, func_params, Eg_crx.comp, dual_gr);

	num_crx = count_crx_through_comp(gmax,Eg_crx.comp);
	
	alloc_grid_crx_mem(&Eg_crx,gmax,num_crx,YES);
	num_crx = install_grid_crx_from_comp(func,func_params,&Eg_crx,
				dual_gr,0);

	num_curve_crx = install_grid_curve_crx_from_comp(func, func_params, 
			&Eg_crx, dual_gr, &num_crx);

	is = make_surfaces_from_crx(intfc,&Eg_crx,gmax,num_crx - num_curve_crx);
	ic = make_curves_from_crx(intfc,&Eg_crx,gmax,num_curve_crx);

	blk_info.num_curves = ic;
       	if (ic != 0) 
	{
            uni_array(&blk_info.curves, ic, sizeof(CURVE*));
	    for(i = 0; i < ic; i++)
                blk_info.curves[i] = Eg_crx.curves[i];
	}

	blk_info.num_surfs = is;
	if(is != 0)
	{
	    uni_array(&blk_info.surfs, is, sizeof(SURFACE*));
	    uni_array(&blk_info.cur_tris, is, sizeof(TRI*));
	    for(i = 0; i < is; i++)
	    {
	        blk_info.cur_tris[i] = NULL;
	        blk_info.surfs[i] = Eg_crx.surfaces[i];
	    }
	}

	blk_crx->blk_info = &blk_info; 
	make_grid_surfaces(blk_crx,&Eg_crx,gmax,YES);
	
	/*after the above function, the num_curves  */
	/*and curves in Eg_crx are changed */

        for (i = 0; i < blk_info.num_surfs; ++i)
	{
	    last_tri(blk_info.surfs[i]) = blk_info.cur_tris[i];
	    last_tri(blk_info.surfs[i])->next = 
	                tail_of_tri_list(blk_info.surfs[i]);
	    first_tri(blk_info.surfs[i])->prev = 
	                head_of_tri_list(blk_info.surfs[i]);
	}

	for (i = 0; i < blk_info.num_surfs; ++i)
	    reset_intfc_num_points(blk_info.surfs[i]->interface);
	
	*num_surfs = is;
	*num_curves = Eg_crx.num_curves;
	for (i = 0; i < blk_info.num_surfs; i++)
	    s[i] = blk_info.surfs[i];
	for (i = 0; i < Eg_crx.num_curves; i++)
	    c[i] = Eg_crx.curves[i];
	
	free_grid_crx_mem(&Eg_crx,YES);
	free(Eg_crx.comp);

	if(is != 0)
	    free_these(3, Eg_crx.surfaces, blk_info.surfs, blk_info.cur_tris);
	if(ic != 0)
	    free_these(2, Eg_crx.curves, blk_info.curves);

	interface_reconstructed(intfc) = YES;

	return YES;
}

EXPORT boolean read_sdl_surface(
	INTERFACE   *intfc,
	COMPONENT   neg_comp,
	COMPONENT   pos_comp,
	char        *sdl_name,
	SURFACE	    **ps)
{
	FILE *sdl_file = fopen(sdl_name,"r");
	SURFACE *surf;
	double coords[MAXD];
	double *vertex;
	int *index;
	int i,j,k,l,m,i1,i2,i3,num_vtx,num_pts,num_point_tris;
	int max_num_vtx,max_num_pts,num_tris;
	boolean point_recorded;
	FT_POINT **points;
	TRI **tris;
	INTERFACE *sav_intfc;
	double L[MAXD],U[MAXD];
	double dist,max_side,min_side,ave_side;
	TRI **ptris;
	FT_POINT *p;
	int num_ptris;

	sav_intfc = current_interface();
	set_current_interface(intfc);
	surf = make_surface(neg_comp,pos_comp,NULL,NULL);

	num_vtx = num_pts = 0;
	max_side = -HUGE;
	min_side =  HUGE;
	ave_side = 0.0;
	for (i = 0; i < 3; ++i)
	{
	    L[i] =  HUGE;
	    U[i] = -HUGE;
	}
	num_vtx = 0;
	while (fgetstring(sdl_file,"vertex"))
	    num_vtx++;
	printf("num_vtx = %d\n",num_vtx);
	uni_array(&vertex,num_vtx,FLOAT);
	uni_array(&index,num_vtx,INT);
	fflush(stdout);
	fclose(sdl_file);
	sdl_file = fopen(sdl_name,"r");
	num_vtx = 0;
	while (fgetstring(sdl_file,"vertex"))
	{
	    fscanf(sdl_file,"%lf %lf %lf",coords,coords+1,coords+2);
	    point_recorded = NO;
	    for (i = 0; i < num_pts; ++i)
	    {
	    	if (coords[0] == vertex[3*i] &&
		    coords[1] == vertex[3*i+1] &&
		    coords[2] == vertex[3*i+2])
	    	{
		    index[num_vtx++] = i;
	    	    point_recorded = YES;
		}
	    }
	    if (!point_recorded)
	    {
	    	index[num_vtx++] = num_pts;
		for (i = 0; i < 3; ++i)
		{
		    vertex[3*num_pts+i] = coords[i];
		    if (L[i] > coords[i]) L[i] = coords[i];
		    if (U[i] < coords[i]) U[i] = coords[i];
		}
	    	num_pts++;
	    }
	}
	uni_array(&points,num_pts,sizeof(FT_POINT*));
	for (i = 0; i < num_pts; ++i)
	{
	    points[i] = Point(vertex+3*i);
	    points[i]->num_tris = 0;
	}
	num_tris = num_vtx/3;
	uni_array(&tris,num_tris,sizeof(TRI*));
	num_point_tris = 0;
	for (i = 0; i < num_tris; ++i)
	{
	    i1 = index[3*i];  i2 = index[3*i+1];  i3 = index[3*i+2];
	    tris[i] = make_tri(points[i1],points[i2],points[i3],
	    			NULL,NULL,NULL,NO);
	    tris[i]->surf = surf;
	    points[i1]->num_tris++; 
	    points[i2]->num_tris++; 
	    points[i3]->num_tris++; 
	    num_point_tris += 3;
	    for (j = 0; j < 3; ++j)
	    {
	    	dist = distance_between_positions(
			Coords(Point_of_tri(tris[i])[j]),
			Coords(Point_of_tri(tris[i])[(j+1)%3]),3);
	    	ave_side += dist;
		if (max_side < dist) max_side = dist;
		if (min_side > dist) min_side = dist;
	    }
	}
	intfc->point_tri_store = (TRI**)store(num_point_tris*sizeof(TRI*));
	ptris = intfc->point_tri_store;
	for (i = 0; i < num_pts; ++i)
	{
	    points[i]->tris = ptris;
	    ptris += points[i]->num_tris;
	    points[i]->num_tris = 0;
	}
	for (i = 0; i < num_tris; ++i)
	{
	    if (i != 0) 
	    {
	    	tris[i]->prev = tris[i-1];
	    	tris[i-1]->next = tris[i];
	    }
	    for (j = 0; j < 3; ++j)
	    {
		p = Point_of_tri(tris[i])[j];
		p->tris[p->num_tris++] = tris[i];
	    }
	}
	for (i = 0; i < num_pts; ++i)
	{
	    ptris = points[i]->tris;
	    num_ptris = points[i]->num_tris;
	    for (j = 0; j < num_ptris; ++j)
	    for (k = 0; k < j; ++k)
	    {
		TRI *tri1 = ptris[j];
		TRI *tri2 = ptris[k];
	    	for (m = 0; m < 3; ++m)
		for (l = 0; l < 3; ++l)
		{
		    if ((Point_of_tri(tri1)[m] == Point_of_tri(tri2)[(l+1)%3] &&
		         Point_of_tri(tri1)[(m+1)%3] == Point_of_tri(tri2)[l]))
		    {
		    	Tri_on_side(tri1,m) = tri2;
			Tri_on_side(tri2,l) = tri1;
		    }
		}
	    }
	}
	ave_side /= num_vtx;
	surf->num_tri = num_tris;
	first_tri(surf) = tris[0];
	last_tri(surf) = tris[num_tris-1];
	last_tri(surf)->next = tail_of_tri_list(surf);
	first_tri(surf)->prev = head_of_tri_list(surf);
	reset_intfc_num_points(surf->interface);

	if (debugging("sdl_input"))
	{
	    (void) printf("The detected boundaries:\n");
	    (void) printf("L = %f %f %f\n",L[0],L[1],L[2]);
	    (void) printf("U = %f %f %f\n",U[0],U[1],U[2]);
	    (void) printf("The triangle length:\n");
	    (void) printf("max_side = %f  min_side = %f  ave_side = %f\n",
					max_side,min_side,ave_side);
	    (void) printf("num_vtx = %d  num_pts = %d  num_tris = %d\n",
					num_vtx,num_pts,num_tris);
	    (void) gview_plot_interface("sdl",intfc);
 	    (void) print_interface(intfc);
	}

	*ps = surf;
	set_current_interface(sav_intfc);
	return YES;
}	/* end read_sdl_surface */

EXPORT boolean read_vtk_surface(
	INTERFACE   *intfc,
	COMPONENT   neg_comp,
	COMPONENT   pos_comp,
	char        *vtk_name,
	SURFACE	    **ps)
{
	FILE *vtk_file = fopen(vtk_name,"r");
	SURFACE *surf;
	double coords[MAXD];
	double *vertex;
	int *index;
	int i,j,k,l,m,i1,i2,i3,num_vtx,num_pts,num_point_tris,num_ptris;
	int max_num_vtx,max_num_pts,num_tris;
	boolean point_recorded;
	FT_POINT **points,*p;
	TRI **tris,**ptris;
	INTERFACE *sav_intfc;
	double L[MAXD],U[MAXD];
	double dist,max_side,min_side,ave_side;
	int N,num_edges;

	sav_intfc = current_interface();
	set_current_interface(intfc);
	surf = make_surface(neg_comp,pos_comp,NULL,NULL);

	max_side = -HUGE;
	min_side =  HUGE;
	ave_side = 0.0;
	for (i = 0; i < 3; ++i)
	{
	    L[i] =  HUGE;
	    U[i] = -HUGE;
	}
	fgetstring(vtk_file,"POINTS");
	fscanf(vtk_file,"%d\n",&num_vtx);
	uni_array(&vertex,3*num_vtx,FLOAT);
	uni_array(&index,num_vtx,INT);
	if (!fgetstring(vtk_file,"double"))
	{
	    (void) printf("VTK file %s cannot find \"double\"\n");
	    /* Try float */
	    rewind(vtk_file);
	    if (!fgetstring(vtk_file,"float"))
	    {
		(void) printf("VTK file %s cannot find \"float\"\n");
		clean_up(ERROR);
	    }
	}
	N = num_vtx;
	num_vtx = num_pts = 0;
	for (k = 0; k < N; ++k)
	{
	    fscanf(vtk_file,"%lf %lf %lf",coords,coords+1,coords+2);
	    point_recorded = NO;
	    for (i = 0; i < num_pts; ++i)
            {
                if (coords[0] == vertex[3*i] &&
                    coords[1] == vertex[3*i+1] &&
                    coords[2] == vertex[3*i+2])
                {
                    index[num_vtx++] = i;
                    point_recorded = YES;
                }
            }
	    if (!point_recorded)
            {
                index[num_vtx++] = num_pts;
                for (i = 0; i < 3; ++i)
                {
                    vertex[3*num_pts+i] = coords[i];
                    if (L[i] > coords[i]) L[i] = coords[i];
                    if (U[i] < coords[i]) U[i] = coords[i];
                }
                num_pts++;
            }
	}
	uni_array(&points,num_pts,sizeof(FT_POINT*));
	for (i = 0; i < num_pts; ++i)
	{
	    points[i] = Point(vertex+3*i);
	    points[i]->num_tris = 0;
	}
	if (!fgetstring(vtk_file,"CELLS"))
	{
	    (void) printf("VTK file %s cannot find \"CELLS\"\n");
	    /* Try POLYGONS */
	    rewind(vtk_file);
	    if (!fgetstring(vtk_file,"POLYGONS"))
	    {
		(void) printf("VTK file %s cannot find \"POLYGONS\"\n");
		clean_up(ERROR);
	    }
	}
	fscanf(vtk_file,"%d %d\n",&num_tris,&num_edges);
	uni_array(&tris,num_tris,sizeof(TRI*));
	num_point_tris = 0;
	for (i = 0; i < num_tris; ++i)
	{
	    int ns,ii1,ii2,ii3;
	    fscanf(vtk_file,"%d %d %d %d",&ns,&ii1,&ii2,&ii3);
	    i1 = index[ii1];  i2 = index[ii2];  i3 = index[ii3];
	    tris[i] = make_tri(points[i1],points[i2],points[i3],
	    			NULL,NULL,NULL,NO);
	    if (ns > 3)
	    {
		(void) printf("Cannot handle POLYGONS greater than 4\n");
		clean_up(ERROR);
	    }
	    tris[i]->surf = surf;
	    points[i1]->num_tris++;
	    points[i2]->num_tris++;
            points[i3]->num_tris++;
            num_point_tris += 3;
	    for (j = 0; j < 3; ++j)
	    {
	    	dist = distance_between_positions(
			Coords(Point_of_tri(tris[i])[j]),
			Coords(Point_of_tri(tris[i])[(j+1)%3]),3);
	    	ave_side += dist;
		if (max_side < dist) max_side = dist;
		if (min_side > dist) min_side = dist;
	    }
	}
	intfc->point_tri_store = (TRI**)store(num_point_tris*sizeof(TRI*));
        ptris = intfc->point_tri_store;
	for (i = 0; i < num_pts; ++i)
        {
	    points[i]->tris = ptris;
            ptris += points[i]->num_tris;
            points[i]->num_tris = 0;
        }
	for (i = 0; i < num_tris; ++i)
        {
	    if (i != 0) 
	    {
	    	tris[i]->prev = tris[i-1];
	    	tris[i-1]->next = tris[i];
	    }
	    for (j = 0; j < 3; ++j)
            {
                p = Point_of_tri(tris[i])[j];
                p->tris[p->num_tris++] = tris[i];
            }
        }
        for (i = 0; i < num_pts; ++i)
        {
            ptris = points[i]->tris;
            num_ptris = points[i]->num_tris;
            for (j = 0; j < num_ptris; ++j)
            for (k = 0; k < j; ++k)
            {
                TRI *tri1 = ptris[j];
                TRI *tri2 = ptris[k];
                for (m = 0; m < 3; ++m)
                for (l = 0; l < 3; ++l)
                {
                    if ((Point_of_tri(tri1)[m] == Point_of_tri(tri2)[(l+1)%3] &&                         Point_of_tri(tri1)[(m+1)%3] == Point_of_tri(tri2)[l]))
                    {
                        Tri_on_side(tri1,m) = tri2;
                        Tri_on_side(tri2,l) = tri1;
                    }
                }
            }
        }
	ave_side /= num_vtx;
	surf->num_tri = num_tris;
	first_tri(surf) = tris[0];
	last_tri(surf) = tris[num_tris-1];
	last_tri(surf)->next = tail_of_tri_list(surf);
	first_tri(surf)->prev = head_of_tri_list(surf);
	reset_intfc_num_points(surf->interface);

	if (debugging("vtk_input"))
	{
	    (void) printf("The detected boundaries:\n");
	    (void) printf("L = %f %f %f\n",L[0],L[1],L[2]);
	    (void) printf("U = %f %f %f\n",U[0],U[1],U[2]);
	    (void) printf("The triangle length:\n");
	    (void) printf("max_side = %f  min_side = %f  ave_side = %f\n",
			max_side,min_side,ave_side);
	    (void) printf("num_vtx = %d  num_tris = %d\n",num_vtx,num_tris);
	    (void) gview_plot_interface("vtk_read",intfc);
 	    (void) print_interface(intfc);
	}

	*ps = surf;
	fclose(vtk_file);
	set_current_interface(sav_intfc);
	return YES;
}	/* end read_vtk_surface */
