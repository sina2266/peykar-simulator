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
*				tri3dutils.c:
*
*       Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Three dimensional specific tetrazation functions.
*/

#if defined(THREED)

#include <tri/tri3ddefs.h>

enum {
	MAX_NUM_TETRAS = 200, /*TOLERANCE*/
	MAX_NUM_SIDES  = 300, /*TOLERANCE*/
	MAX_G_SIZE     =  80, /*TOLERANCE*/
	MAX_N_COMP     =   8, /*TOLERANCE*/
        MAX_BLK_ELS    =  30   /*TOLERANCE*/
};

	/* LOCAL Function Declarations */
LOCAL	void	make_blk_tg_pt_list(TRI_GRID*,BLK_BIN***,P_LINK*,int);
LOCAL	boolean	common_face(FACE*,FACE*);
LOCAL	boolean	init_delaunay_tetra(PT_ST*,TETRA*,int,int*,HULL*,TRI_GRID*);
LOCAL	boolean	is_coplanar_element(LINEAR_ELEMENT*,TRI_GRID*);
LOCAL   boolean 	is_in_positive_face(FACE*,double*,int,TRI_GRID*);
LOCAL	boolean	inside_sphere(TG_PT*,TETRA*);
LOCAL	boolean	point_inside_hull(TG_PT*,HULL*,TRI_GRID*);
LOCAL	boolean	visible_face(TG_PT*,FACE*,boolean,TRI_GRID*);
LOCAL	int	group_point_list(PT_ST*,int,BLK_BIN*,COMPONENT);
LOCAL	void	add_regular_node_to_blk_list(TRI_GRID*,BLK_BIN***);
LOCAL	void	add_tg_pt_from_tri(BLK_BIN*,TRI*,SURFACE*,P_LINK*,int);
LOCAL	void	assign_face(TETRA*,FACE*,int,int,TRI_GRID*);
LOCAL	void	delaunay_triangulation(TRI_GRID*,PT_ST*,int,COMPONENT,BLK_EL0*);
LOCAL	void	connect_pt_and_faces(TETRA*,TETRA,PT_ST*,int*,TRI_GRID*);
LOCAL	void	connect_visible_vtx(PT_ST*,TETRA*,HULL*,int*,TRI_GRID*);
LOCAL	void	increment_delaunay_tetra(PT_ST*,TETRA*,int*,HULL*,TRI_GRID*);
LOCAL	void	make_blk_tetra(TRI_GRID*,BLK_BIN*,BLK_EL0*);
LOCAL	void	make_blk_tg_pt_list(TRI_GRID*,BLK_BIN***,P_LINK*,int);
LOCAL	void	make_cube(TRI_GRID*,BLK_BIN*,BLK_EL0*);
LOCAL	void	reconstruct_delaunay_tetra(TETRA*,TETRA*,PT_ST*,int*,int,
					   TRI_GRID*);
LOCAL	void	reform_tetras_with_exterior_pt(PT_ST*,HULL*,TETRA*,int*,
					       TETRA*,int,TRI_GRID*);
LOCAL	void	remove_common_faces(TETRA*,TETRA*,TRI_GRID*);
LOCAL	void	remove_hull_overlaps(HULL*,int,int,boolean*);
LOCAL	void	set_tetra_geometric_params(TETRA*,TRI_GRID*);
LOCAL	void	shift_hull(HULL*,boolean*);
LOCAL	void	shift_tetra_list(TETRA*,int*,int*);
LOCAL	void	sort_PT_STs(PT_ST*,int,BLK_BIN*);
#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */
LOCAL	int	PtStcompr(const void*,const void*);
#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */
#if defined(DEBUG_TRI_GRID)
LOCAL	double	tetra_volume(TETRA*);
LOCAL	void	print_blk_bin(BLK_BIN*);
LOCAL	void	print_hull(HULL*);
LOCAL	void	print_tetra(TETRA*);
#endif /* defined(DEBUG_TRI_GRID) */

#if defined(DEBUG_TRI_GRID)
LOCAL int max_num_faces;
LOCAL int max_num_tetra;
LOCAL int max_num_enc_tetra;
LOCAL int total_num_cubes;
LOCAL int total_num_tetra;
#endif /* defined(DEBUG_TRI_GRID) */

LOCAL	int	  num_3d_bilin_els;
LOCAL	int	  num_3d_lin_els;
LOCAL	TG_PT	  **el_list_store = NULL;
LOCAL	COMPONENT *pos_comp_store = NULL, *neg_comp_store = NULL;
LOCAL	Locstate  *pos_st_store = NULL, *neg_st_store = NULL;


#define	installed(pt,npts,pt_list)					\
	pointer_in_list((POINTER)(pt),(npts),(POINTER*)(pt_list))

EXPORT	int triangulate_mesh3d(
	TRI_GRID	*ntg)
{
	int		ix, iy, iz, nt, ***num_tris;
	struct Table	*T;
	BLK_BIN		***blk_bin;
	BLK_BIN		*bbzyx;
	BLK_EL0		*blk_el0;
	P_LINK		*hash_table;
	int             xmax, ymax, zmax;
	int		h_size;


	xmax = ntg->rect_grid.gmax[0];
	ymax = ntg->rect_grid.gmax[1];
	zmax = ntg->rect_grid.gmax[2];
#if defined(DEBUG_TRI_GRID)
	max_num_faces = 0;
	max_num_tetra = 0;
	total_num_tetra = 0;
	total_num_cubes = 0;
	max_num_enc_tetra = 0;
#endif /* defined(DEBUG_TRI_GRID) */

	ntg->_locate_on_trigrid = tg_locate;
	set_tri3d_tolerances(ntg);

	T = table_of_interface(ntg->grid_intfc);
	num_3d_bilin_els  = 0;
	num_3d_lin_els = 0;

	    /* Allocate space for hashing table */

	h_size = (ntg->grid_intfc->num_points)*4+1; 
	uni_array(&hash_table,h_size,sizeof(P_LINK)); 

	    /* Assign temperary block storage bin */

	tri_array(&blk_bin,zmax,ymax,xmax,sizeof(BLK_BIN));

	    /* Prepare trigrid points for each block */

	start_clock("prepare_for_triangulate");
	copy_tg_pts_from_intfc(ntg,hash_table,h_size);
	copy_tg_pts_from_regular_grid(ntg);
	make_blk_tg_pt_list(ntg,blk_bin,hash_table,h_size);
	add_regular_node_to_blk_list(ntg,blk_bin);
	stop_clock("prepare_for_triangulate");

	    /* Triangulate each mesh block */

	start_clock("triangulate_blocks");
	num_3d_bilin_els  = 0;
	num_3d_lin_els = 0;
	blk_el0 = ntg->blk_els0;
	num_tris = T->num_of_tris;
	for (iz = 0; iz < zmax; ++iz)
	{
	    for (iy = 0; iy < ymax; ++iy)
	    {
		for (ix = 0; ix < xmax; ++ix)
		{
		    nt = num_tris[iz][iy][ix];
		    bbzyx = &blk_bin[iz][iy][ix];
		    if (nt == 0)	
		    {
			make_cube(ntg,bbzyx,blk_el0);
#if defined(DEBUG_TRI_GRID)
			if (debugging("tri_mesh"))
			    ++total_num_cubes;
#endif /* defined(DEBUG_TRI_GRID) */
		    }
		    else	
		    {
		    	make_blk_tetra(ntg,bbzyx,blk_el0);
#if defined(DEBUG_TRI_GRID)
			if (debugging("tri_mesh"))
			    total_num_tetra += num_lin_els_in_blk(blk_el0);
#endif /* defined(DEBUG_TRI_GRID) */
		    }
		    ++blk_el0;
		}
	    }
	}
	stop_clock("triangulate_blocks");
#if defined(DEBUG_TRI_GRID)
	if (debugging("tri_mesh"))
	{
	    (void) printf("Total number of allocated bilinear elements = %d\n",
			  ntg->n_bilin_els);
	    (void) printf("Total number of produced bilinear elements = %d\n",
			  total_num_cubes);
	    (void) printf("Total number of allocated linear elements = %d\n",
			  ntg->n_lin_els);
	    (void) printf("Total number of produced linear elements = %d\n",
			  total_num_tetra);
	    (void) printf("MAX_NUM_SIDES = %d\n",MAX_NUM_SIDES);
	    (void) printf("MAX_NUM_TETRA = %d\n",MAX_NUM_TETRAS);
	    (void) printf("Maximum number of faces used: %d\n",max_num_faces);
	    (void) printf("Maximum number of tetra used: %d\n",max_num_tetra);
	    (void) printf("Maximum number of encircling tetra: %d\n",
			  max_num_enc_tetra);
	}
#endif /* defined(DEBUG_TRI_GRID) */
	free_these(7,blk_bin,hash_table,el_list_store,
		   pos_comp_store,neg_comp_store,pos_st_store,neg_st_store);
	return GOOD_STEP;
}		/*end triangulate_mesh3d*/

EXPORT boolean fast_point_in_tetra(
	double		*pt,
	LINEAR_ELEMENT	*el,
	COMPONENT	comp,
	TRI_GRID	*ntg)
{
	double		*p0, *p1, *p2, *p3;
	double		a[3],b[3],c[3],v[3];
	double		D,Dp;
	int		i,j;

	if(debugging("line_pj"))
		printf("\n");
	
	for (i = 0; i < 4; i++)
	{
	    p0 = Coords(el->p[i]);
	    p1 = Coords(el->p[(i+1)%4]);
	    p2 = Coords(el->p[(i+2)%4]);
	    p3 = Coords(el->p[(i+3)%4]);

	    for (j = 0; j < 3; j++)
	    {
	    	a[j] = p1[j] - p0[j];
	    	b[j] = p2[j] - p0[j];
	    	c[j] = p3[j] - p0[j];
	    	v[j] = pt[j] - p0[j];
	    }
	    D = Det3d(a,b,c);
	    
	    if(debugging("line_pj"))
		    printf("D %24.16e  vtol  %24.16e\n", D, vtol(ntg));

	    if (fabs(D) < vtol(ntg)) return NO;
	    Dp = Det3d(a,b,v);
    
	    if(debugging("line_pj"))
		    printf("D %24.16e  Dp  %24.16e  D*Dp %24.16e\n", D, Dp, D*Dp);
	    
	    /*#bjet2 pt is on the plane. */
	    if (fabs(Dp) < vtol(ntg)) continue;
	
	    if ((D > 0.0 && Dp < 0.0) ||
	        (D < 0.0 && Dp > 0.0))
		return NO;
	}
	return YES;
}	/* end fast_point_in_tetra */

EXPORT boolean point_in_tetra(
	double		*pt,
	LINEAR_ELEMENT	*el,
	COMPONENT	comp,
	TRI_GRID	*ntg)
{
	static HULL	*cvx_hull = NULL;
	static TETRA	tetra;
	int		i;

	if (comp != el->comp)
	    return NO;

	if (is_coplanar_element(el,ntg) == YES)
	    return NO;

	if (cvx_hull == NULL)
	{
	    scalar(&cvx_hull,sizeof(HULL));
	    uni_array(&(cvx_hull->faces),4,sizeof(FACE));
	    cvx_hull->num_faces = 4;
	}

	for (i = 0; i < 4; ++i)
	{
	    tetra.el.p[i] = el->p[i];
	    tetra.el.side[i] = el->side[i];
	}
	for (i = 0; i < 4; ++i)
	{
	    assign_face(&tetra,cvx_hull->faces+i,i,YES,ntg);
	}
	for (i = 0; i < 4; ++i)
	{
	    if (is_in_positive_face(cvx_hull->faces+i,pt,NO,ntg) == YES)
	    	return NO;
	}
	return YES;
}		/*end point_in_tetra*/

LIB_LOCAL int count_num_pcs3d(
	TRI_GRID	*ntg)
{
	int		ix, iy, iz, k, l, j;
	int		nt,***num_tris;
	TRI		*****tris,**blk_tris,*t;
	FT_POINT		*p;
	BOND		*b;
	BOND_TRI	**btris;
	struct Table	*T;
	int		n_pcs = 0;
	int             xmax, ymax, zmax;

	xmax = topological_grid(ntg->grid_intfc).gmax[0];
	ymax = topological_grid(ntg->grid_intfc).gmax[1];
	zmax = topological_grid(ntg->grid_intfc).gmax[2];
	T = table_of_interface(ntg->grid_intfc);
	num_tris = T->num_of_tris;
	tris = T->tris;
	for (iz = 0; iz < zmax; ++iz)
	{
	    for (iy = 0; iy < ymax; ++iy)
	    {
		for (ix = 0; ix < xmax; ++ix)
		{
		    nt = num_tris[iz][iy][ix];
		    if (nt == 0)
		    	n_pcs += 1;
		    else
		    {
		    	n_pcs += 8;
			blk_tris = tris[iz][iy][ix];
		    	for (l = 0; l < nt; ++l)
		    	{
			    t = blk_tris[l];
			    for (k = 0; k < 3; ++k)
			        sorted(Point_of_tri(t)[k]) = NO;
		    	}
		
			/*see add_intfc_blk_pcs3d for using */
			/*#bjet2 count boundary points */
			for (l = 0; l < nt; ++l)
			{
			    t = blk_tris[l];
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
		    
		    		    for(btris = Btris(b); btris && *btris; btris++)
		    		    {
			    	        ++n_pcs;
		    		    }/*for btris */
				}/* for j, two points for a side */
	    		    }/* for k, sides for t */
			}

			/*count interior points */
		    	for (l = 0; l < nt; ++l)
		    	{
			    t = blk_tris[l];
			    for (k = 0; k < 3; ++k)
			    {
			        if (sorted(Point_of_tri(t)[k]) == NO)
			        {
			    	    sorted(Point_of_tri(t)[k]) = YES;
			    	    ++n_pcs;
			        }
			    }
		    	}
		    }
		}
	    }
	}
	return n_pcs;
}	/* end count_num_pcs3d */


LIB_LOCAL	void copy_tg_pts_from_regular_grid(
	TRI_GRID	*ntg)
{
	RECT_GRID	*gr = &ntg->rect_grid;
	double		**edges = gr->edges, *xedges, *yedges, *zedges;
	int		xmaxx, ymaxx, zmaxx;
	TG_PT		*node_pts;
	int		i,j,k;
	int		n_node_points = 0;

	xmaxx = ntg->rect_grid.gmax[0]+1;
       	ymaxx = ntg->rect_grid.gmax[1]+1;
       	zmaxx = ntg->rect_grid.gmax[2]+1;

	node_pts = ntg->node_points;
	zedges = edges[2];
	for (k = 0; k < zmaxx; ++k)
	{
	    yedges = edges[1];
	    for (j = 0; j < ymaxx; ++j)
	    {
		xedges = edges[0];
		for (i = 0; i < xmaxx; ++i)
		{
		    Coords(node_pts)[0] = *xedges;
		    Coords(node_pts)[1] = *yedges;
		    Coords(node_pts)[2] = *zedges;
		    ++node_pts;
		    ++n_node_points;
		    if (i < xmaxx-1)
			++xedges;
		}
	        if (j < ymaxx-1)
		    ++yedges;
	    }
	    if (k < zmaxx-1)
		++zedges;
	}
}

LIB_LOCAL int  max_num_3d_lin_els(
	INTERFACE	*intfc)
{
	FT_POINT		*p;
	TRI		**blk_tris;
	TRI		*****tz, ****tzy, ***tzyx;
	int		***ntz, **ntzy, *ntzyx;
	int		num_blk_tris;
	int		l,m,np;
	int		i,j,k;
	int		num_tol_pts;
	int		num_lin_els;
	int		xmax, ymax, zmax;
	struct Table	*T;
	static FT_POINT	**p_list = NULL;

	T = table_of_interface(intfc);
	if (p_list == NULL)
		uni_array(&p_list,MAX_BLK_ELS,sizeof(FT_POINT *));

	xmax = topological_grid(intfc).gmax[0];
	ymax = topological_grid(intfc).gmax[1];
	zmax = topological_grid(intfc).gmax[2];

	num_tol_pts = 0;
	for (k = 0, ntz = T->num_of_tris, tz = T->tris;
				k < zmax; ++k, ++ntz, ++tz)
	{
	    for (j = 0, ntzy = *ntz, tzy = *tz;
				j < ymax; ++j, ++ntzy, ++tzy)
	    {
		for (i = 0, ntzyx = *ntzy, tzyx = *tzy;
				i < xmax; ++i, ++ntzyx, ++tzyx)
		{
		    if ((num_blk_tris = *ntzyx) == 0) continue;
		    blk_tris = *tzyx;
		    np = 0;
		    for (l = 0; l < num_blk_tris; ++l)
		    {
			for (m = 0; m < 3; ++m)
			{
			    p = Point_of_tri(blk_tris[l])[m];
			    if (installed(p,np,p_list) == NO)
			    {
				p_list[np++] = p;
			    }
			}
		    }
		    np = 2*np + 8;
		    num_tol_pts += np;
		}
	    }
	}
	num_lin_els = 3*num_tol_pts;
	return num_lin_els;
}		/*end max_num_3d_lin_els*/


LOCAL	void make_blk_tg_pt_list(
	TRI_GRID	*ntg,
	BLK_BIN		***blk_bin,
	P_LINK		*hash_table,
	int		h_size)
{
	TRI		*****tz, ****tzy, ***tzyx;
	SURFACE		*****sz, ****szy, ***szyx;
	TRI		**blk_tris;
	SURFACE		**blk_surfs;
	BLK_BIN		***bbz, **bbzy, *bbzyx;
	int		i,j,k,l, indx;
	int		num_blk_tri;
	int		***ntz, **ntzy, *ntzyx;
	int             xmax, ymax, zmax;
	struct Table	*T;

	xmax = topological_grid(ntg->grid_intfc).gmax[0];
	ymax = topological_grid(ntg->grid_intfc).gmax[1];
	zmax = topological_grid(ntg->grid_intfc).gmax[2];
	T = table_of_interface(ntg->grid_intfc);
	for (indx = 0, k = 0, ntz = T->num_of_tris; k < zmax; ++k, ++ntz)
	{
	    for (j = 0, ntzy = *ntz; j < ymax; ++j, ++ntzy)
	    {
		for (i = 0, ntzyx = *ntzy; i < xmax; ++i, ++ntzyx)
		{
		    indx += (*ntzyx != 0) ? MAX_BLK_ELS : 8;
		}
	    }
	}

	uni_array(&el_list_store,indx,sizeof(TG_PT *));
	uni_array(&pos_comp_store,indx,sizeof(COMPONENT));
	uni_array(&neg_comp_store,indx,sizeof(COMPONENT));
	uni_array(&pos_st_store,indx,sizeof(Locstate));
	uni_array(&neg_st_store,indx,sizeof(Locstate));

	indx = 0;
	for (k=0, ntz=T->num_of_tris, bbz=blk_bin, tz=T->tris, sz=T->surfaces;
	     k < zmax; ++k, ++ntz, ++bbz, ++tz, ++sz)
	{
	    for (j = 0, ntzy = *ntz, bbzy = *bbz, tzy = *tz, szy = *sz;
				j < ymax; ++j, ++ntzy, ++bbzy, ++tzy, ++szy)
	    {
		for (i = 0,
			ntzyx = *ntzy,
			bbzyx = *bbzy,
			tzyx = *tzy,
			szyx = *szy;
				i < xmax; ++i, ++ntzyx, ++bbzyx, ++tzyx, ++szyx)
		{
		    num_blk_tri = *ntzyx;
		    bbzyx->num_els = 0;
		    if (num_blk_tri != 0)
		    {
			bbzyx->el_list = el_list_store + indx;
			bbzyx->pos_comp = pos_comp_store + indx;
			bbzyx->neg_comp = neg_comp_store + indx;
			bbzyx->pos_st = pos_st_store + indx;
			bbzyx->neg_st = neg_st_store + indx;
			indx += MAX_BLK_ELS;
		    	blk_tris = *tzyx; blk_surfs = *szyx;
		    	for (l = 0; l < num_blk_tri; ++l)
		    	{
			    add_tg_pt_from_tri(bbzyx,blk_tris[l],
					blk_surfs[l],hash_table,h_size);
		    	}
		    }
		    else
		    {
			bbzyx->el_list = el_list_store + indx;
			bbzyx->pos_comp = pos_comp_store + indx;
			bbzyx->neg_comp = neg_comp_store + indx;
			bbzyx->pos_st = pos_st_store + indx;
			bbzyx->neg_st = neg_st_store + indx;
			indx += 8;
		    }
		}
	    }
	}
}		/*end make_blk_tg_pt_list*/


LOCAL void add_tg_pt_from_tri(
	BLK_BIN		*blk_bin,
	TRI		*tri,
	SURFACE		*surf,
	P_LINK		*hash_table,
	int		h_size)
{
	int		i,num_els;
	TG_PT		*pt;
	TG_PT		**el_list;
	FT_POINT		*point;
	Locstate	sl,sr;

	num_els = blk_bin->num_els;
	el_list = blk_bin->el_list;
	for (i = 0; i < 3; ++i)
	{
	    point = Point_of_tri(tri)[i];
	    pt = (TG_PT*)find_from_hash_table((POINTER)point,
			                      hash_table,h_size);
	    if (!installed(pt,num_els,el_list))
	    {
	    	el_list[num_els] = pt;
	    	blk_bin->pos_comp[num_els] = positive_component(surf);
	    	blk_bin->neg_comp[num_els] = negative_component(surf);
	    	slsr(point,Hyper_surf_element(tri),Hyper_surf(surf),&sl,&sr);
		blk_bin->neg_st[num_els] = sl;
		blk_bin->pos_st[num_els] = sr;
		++num_els;
	    }
	}
	blk_bin->num_els = num_els;
}		/*end add_tg_pt_from_tri*/

LOCAL 	void make_blk_tetra(
	TRI_GRID	*ntg,
	BLK_BIN		*blk_bin,
	BLK_EL0		*blk_el0)
{
	static	PT_ST	*g_pts = NULL;
	static COMPONENT *comp = NULL;
	int		i,num_comp,gnp;

	if (g_pts == NULL)
	{
	    uni_array(&g_pts,MAX_G_SIZE,sizeof(PT_ST));
	    uni_array(&comp,MAX_N_COMP,sizeof(COMPONENT));
	}

	num_comp = 0;
	for (i = 0; i < blk_bin->num_els - 8; ++i)
	{
	    if (integer_in_list((blk_bin->pos_comp[i]),num_comp,comp) == NO)
	    {
	    	comp[num_comp++] = blk_bin->pos_comp[i];
	    }
	    if (integer_in_list((blk_bin->neg_comp[i]),num_comp,comp) == NO)
	    {
	    	comp[num_comp++] = blk_bin->neg_comp[i];
	    }
	}
#if defined(DEBUG_TRI_GRID)
	if (debugging("delaunay_tri"))
	{
	    (void) printf("Block Input: \n");
	    print_blk_bin(blk_bin);
	    (void) printf("Total number of components: %d\n",num_comp);
	    for (i = 0; i < num_comp; ++i) 
	    	(void) printf("comp[%d] = %d\n",i,comp[i]);
	}
#endif /* defined(DEBUG_TRI_GRID) */
	blk_el0_linear_els(blk_el0) = ntg->lin_els + num_3d_lin_els;
	num_lin_els_in_blk(blk_el0) = 0;

	for (i = 0; i < num_comp; ++i)
	{
	    gnp = 0;
	    gnp = group_point_list(g_pts,gnp,blk_bin,comp[i]);

#if defined(DEBUG_TRI_GRID)
	    if (debugging("delaunay_tri"))
	    {
	        (void) printf("Group delaunay triangulation: \n");
	        (void) printf("Group component: %d\n",comp[i]);
	        (void) printf("Total number of points in group: %d\n",gnp);
	    }
#endif /* defined(DEBUG_TRI_GRID) */
	    delaunay_triangulation(ntg,g_pts,gnp,comp[i],blk_el0);
	}
}		/*end make_blk_tetra*/


LOCAL	void make_cube(
	TRI_GRID	*ntg,
	BLK_BIN		*blk_bin,
	BLK_EL0		*blk_el0)
{
	BILINEAR_ELEMENT *cube;
	int		 i;

	cube = blk_el0_bilinear_el(blk_el0) =
	    &(ntg->bilin_els[num_3d_bilin_els++]);

	for (i = 0; i < 8; ++i)
	{
	    cube->p[i] = blk_bin->el_list[i];
	}
	cube->comp = blk_bin->pos_comp[0];
	set_bilinear_blk_el0(blk_el0);
	blk_bin->num_els = 0;
#if defined(DEBUG_TRI_GRID)
	if (debugging("make_cube"))
	{
	    (void) printf("Block Input: \n");
	    print_blk_bin(blk_bin);
	    (void) printf("Block Cube: \n");
	    print_BILINEAR_ELEMENT(blk_el0_bilinear_el(blk_el0),ntg);
	}
#endif /* defined(DEBUG_TRI_GRID) */
}		/*end make_cube*/


LOCAL	void add_regular_node_to_blk_list(
	TRI_GRID	*ntg,
	BLK_BIN		***blk_bin)
{
	TG_PT		*node_pts;
	BLK_BIN		***bbz, **bbzy, *bbzyx;
	int		i,j,k,ii;
	int		num_els;
	int		*offset = ntg->offset;
	int             xmax, ymax, zmax;
	COMPONENT	*comp;
	Locstate	*states;
	Table		*T = table_of_interface(ntg->grid_intfc);
	
	/*
	*	verticies on bilinear elements are numbered as follows
	*
	*                                  z
	*		1-------3          ^
	*	       /|      /|          |
	*	      / |     / |          |
	*	     /  |    /  |          |
	*	    /   0---/---2          -------->y
	*          /   /   /   /          /
	*         5-------7   /          /
	*	  |  /    |  /          /
	*	  | /     | /          /
	*	  |/      |/         \/
	*         4-------6          x
	*/

	xmax = ntg->rect_grid.gmax[0];
	ymax = ntg->rect_grid.gmax[1];
	zmax = ntg->rect_grid.gmax[2];


	comp = T->components;
	states = ntg->states;
	node_pts = ntg->node_points;
	for (k = 0, bbz = blk_bin; k < zmax; ++k)
	{
	    for (j = 0, bbzy = *bbz; j < ymax; ++j)
	    {
	        for (i = 0, bbzyx = *bbzy; i < xmax; ++i)
	        {
		    num_els = bbzyx->num_els;
		    for (ii = 0; ii < 8; ++ii)
		    {
		    	bbzyx->el_list[num_els+ii] = node_pts + offset[ii];
		    	bbzyx->pos_st[num_els+ii] = states[offset[ii]];
		    	bbzyx->pos_comp[num_els+ii] = comp[offset[ii]];
		    	bbzyx->neg_comp[num_els+ii] = NO_COMP;
		    }
		    bbzyx->num_els += 8;
		    ++node_pts;
		    ++states;
		    ++comp;
		    if (i < xmax-1)
		        ++bbzyx;
	        }
		++node_pts;
		++states;
		++comp;
		if (j < ymax-1)
		    ++bbzy;
	    }
	    if (k < zmax-1)
	    {
	    	node_pts += xmax+1;
	    	states += xmax+1;
	    	comp += xmax+1;
		++bbz;
	    }
	}
}		/*end add_regular_node_to_blk_list*/

LOCAL void delaunay_triangulation(
	TRI_GRID	*ntg,
	PT_ST		*g_pts,
	int		num_pts,
	COMPONENT comp,
	BLK_EL0		*blk_el0)
{
	int		i,num_tetras;
	static TETRA	*tetras = NULL;
	static HULL	*cvx_hull = NULL;

	debug_print("delaunay_tri","Entered delaunay_triangulation\n");

	if (tetras == NULL)
	{
	    uni_array(&tetras,MAX_NUM_TETRAS,sizeof(TETRA));
	    scalar(&cvx_hull,sizeof(HULL));
	    uni_array(&(cvx_hull->faces),MAX_NUM_SIDES,sizeof(FACE));
	}

	/* Make the first tetra */

	if (init_delaunay_tetra(g_pts,&tetras[0],num_pts,&num_tetras,
				cvx_hull,ntg) == NO)
	{
#if defined(DEBUG_TRI_GRID)
	    if (debugging("delaunay_tri"))
	    {
	    	(void) printf("Co-planar group of points, ");
	    	(void) printf("no tetrahedral created\n");
	    }
#endif /* defined(DEBUG_TRI_GRID) */
	    debug_print("delaunay_tri","Left delaunay_triangulation\n");
	    return;
	}

	/* Adding point to the delaunay */
	/* tetra using incremental method */

	for (i = 4; i < num_pts; ++i)
	    increment_delaunay_tetra(g_pts+i,tetras,&num_tetras,cvx_hull,ntg);

	for (i = 0; i < num_tetras; ++i)
	{
	    if (tetras[i].coplanar)
		continue;
	    tetras[i].el.comp = comp;
	    ntg->lin_els[num_3d_lin_els++] = tetras[i].el;
	    ++num_lin_els_in_blk(blk_el0);
	}
#if defined(DEBUG_TRI_GRID)
	if (debugging("delaunay_tri"))
	{
	    int num_non_cp;
	    double volume = 0.0;

	    for (i = 0, num_non_cp = 0; i < num_tetras; ++i)
	    	if (!tetras[i].coplanar)
		    ++num_non_cp;

	    (void) printf("Number of tetrahedrals created: %d\n",num_tetras);
	    for (i = 0; i < num_tetras; ++i)
	    {
	    	print_tetra(&tetras[i]);
	    	volume += tetra_volume(&tetras[i]);
	    }
	    (void) printf("Number of non-coplanar tetras: %d\n",num_non_cp);
	    (void) printf("Total volume = %g\n",volume);
	    (void) printf("Convex Hull: \n");
	    print_hull(cvx_hull);
	}
	if (debugging("tri_mesh"))
	    if (max_num_tetra < num_tetras)
		max_num_tetra = num_tetras;
#endif /* defined(DEBUG_TRI_GRID) */
	debug_print("delaunay_tri","Left delaunay_triangulation\n");
}		/*end delaunay_triangulation*/

LOCAL boolean init_delaunay_tetra(
	PT_ST		*g_pts,
	TETRA		*tetra0,
	int		num_pts,
	int		*num_tetras,
	HULL		*hull,
	TRI_GRID	*ntg)
{
	int		shifted = NO;
	int		i,j;

	for (i = 0; i < 4; ++i) 
	{
	    tetra0->el.p[i] = g_pts[i].pt;
	    tetra0->el.s[i] = g_pts[i].st;
	    tetra0->el.side[i] = CREATE_TETRA;
	}
	set_tetra_geometric_params(tetra0,ntg);
	if (tetra0->coplanar)
	{
	    for (j = 3; j >= 0 && shifted == NO; --j)
	    {
	        for (i = 4; i < num_pts && shifted == NO; ++i)
	        {
	    	    tetra0->el.p[j] = g_pts[i].pt;
	    	    tetra0->el.s[j] = g_pts[i].st;
	    	    set_tetra_geometric_params(tetra0,ntg);
	    	    if (!tetra0->coplanar)
	    	    {
	    		PT_ST Ptmp = g_pts[j];

	    		g_pts[j] = g_pts[i];
	    		g_pts[i] = Ptmp;
	    		shifted = YES;
	    	    }
	        }
	        if (shifted == NO)
	        {
	    	    tetra0->el.p[j] = g_pts[j].pt;
	    	    tetra0->el.s[j] = g_pts[j].st;
	        }
	    }
	    if (shifted == NO)
		return NO;
	}
	for (i = 0; i < 4; ++i)
	{
	    assign_face(tetra0,hull->faces+i,i,YES,ntg);
	}
	*num_tetras = 1;
	hull->num_faces = 4;
	return YES;
}		/*end init_delaunay_tetra*/


LOCAL void increment_delaunay_tetra(
	PT_ST		*pt,
	TETRA		*tetras,
	int		*num_tetras,
	HULL		*hull,
	TRI_GRID	*ntg)
{
	int		num_enc;
	static TETRA	*tetras_enc = NULL;
	int		i,j,i_shift[MAX_NUM_TETRAS];

	if (tetras_enc == NULL)
	{
	    uni_array(&tetras_enc,MAX_NUM_TETRAS,sizeof(TETRA));
	}
	if (point_inside_hull(pt->pt,hull,ntg) == NO)
	{
#if defined(DEBUG_TRI_GRID)
	    if (debugging("delaunay_tri"))
	    	(void) printf("New point is outside the convex hull\n");
#endif /* defined(DEBUG_TRI_GRID) */
	    num_enc = 0;
	    for (i = 0; i < *num_tetras; ++i)
	    {
	    	if (inside_sphere(pt->pt,&tetras[i]) == YES)
	    	{
	    	    for (j = 0; j < 4; ++j)
	    	    {
	    	    	tetras_enc[num_enc].el.p[j] = tetras[i].el.p[j];
	    	    	tetras_enc[num_enc].el.s[j] = tetras[i].el.s[j];
	    	    	tetras_enc[num_enc].el.side[j] = CREATE_TETRA;
	    	    }
	    	    i_shift[i] = YES;
	    	    ++num_enc;
	    	}
	    	else i_shift[i] = NO;
	    }
	    if (num_enc == 0)
	    	connect_visible_vtx(pt,tetras,hull,num_tetras,ntg);
	    else
	    {
	    	shift_tetra_list(tetras,num_tetras,i_shift);
	    	reform_tetras_with_exterior_pt(pt,hull,tetras,num_tetras,
					       tetras_enc,num_enc,ntg);
	    }

	}
	else
	{
#if defined(DEBUG_TRI_GRID)
	    if (debugging("delaunay_tri"))
	    {
	    	(void) printf("New point is inside the convex hull\n");
	    }
#endif /* defined(DEBUG_TRI_GRID) */
	    num_enc = 0;
	    for (i = 0; i < *num_tetras; ++i)
	    {
	    	if (inside_sphere(pt->pt,&tetras[i]) == YES)
	    	{
	    	    for (j = 0; j < 4; ++j)
	    	    {
	    	    	tetras_enc[num_enc].el.p[j] = tetras[i].el.p[j];
	    		tetras_enc[num_enc].el.s[j] = tetras[i].el.s[j];
	    		tetras_enc[num_enc].el.side[j] = CREATE_TETRA;
	    	    }
	    	    i_shift[i] = YES;
	    	    ++num_enc;
	    	}
	    	else i_shift[i] = NO;
	    }
	    shift_tetra_list(tetras,num_tetras,i_shift);

	    reconstruct_delaunay_tetra(tetras,tetras_enc,pt,num_tetras,
	    			       num_enc,ntg);
	}
#if defined(DEBUG_TRI_GRID)
	if (debugging("delaunay_tri"))
	{
	    (void) printf("Number of encircling tetras: %d\n",num_enc);
	}
	if (debugging("tri_mesh"))
	{
	    if (max_num_enc_tetra < num_enc)
	    	max_num_enc_tetra = num_enc;
	}
#endif /* defined(DEBUG_TRI_GRID) */
}		/*end increment_delaunay_tetra*/

LOCAL void connect_visible_vtx(
	PT_ST		*pt,
	TETRA		*tetras,
	HULL		*hull,
	int		*num_tetras,
	TRI_GRID	*ntg)
{
	int		i,j;
	int		ns_old, ns_new;
	boolean		i_shift[MAX_NUM_SIDES];

	ns_old = hull->num_faces;

	/* Determine visible face and make tetras */

	for (i = 0; i < ns_old; ++i)
	{
	    i_shift[i] = NO;
	    if (visible_face(pt->pt,hull->faces+i,NO,ntg) == YES)
	    {
	        for (j = 0; j < 3; ++j)
	        {
	            tetras[*num_tetras].el.p[j] = hull->faces[i].vertex[j];
	            tetras[*num_tetras].el.s[j] = hull->faces[i].state[j];
	        }
	        tetras[*num_tetras].el.p[3] = pt->pt;
	        tetras[*num_tetras].el.s[3] = pt->st;
	        set_tetra_geometric_params(&tetras[*num_tetras],ntg);
	        if (!tetras[*num_tetras].coplanar)
	        {
	            i_shift[i] = YES;
	            for (j = 0; j < 3; ++j)
	            {
	        	assign_face(&tetras[*num_tetras],
	        	            hull->faces + hull->num_faces++,j,YES,ntg);
	            }
	            ++(*num_tetras);
	        }
	    }
	}

	/* Identify interior overlapping faces */

	ns_new = hull->num_faces;
	remove_hull_overlaps(hull,ns_old,ns_new,i_shift);

}		/*end connect_visible_vtx*/


LOCAL void reconstruct_delaunay_tetra(
	TETRA		*tetras,
	TETRA		*tetras_enc,
	PT_ST		*pt,
	int		*num_tetras,
	int		num_enc,
	TRI_GRID	*ntg)
{
	int		i,j;

	if (num_enc == 1)
	{
	    connect_pt_and_faces(tetras,tetras_enc[0],pt,num_tetras,ntg);
	}
	else
	{
	    for (i = 0; i < num_enc; ++i)
	    {
	    	for (j = 0; j < num_enc; ++j)
	    	{
	    	    if (i == j)
			continue;
	    	    else
	    	    {
	    	    	remove_common_faces(&tetras_enc[i],&tetras_enc[j],ntg);
	    	    }
	    	}
	    }
	    for (i = 0; i < num_enc; ++i)
	    {
	    	connect_pt_and_faces(tetras,tetras_enc[i],pt,num_tetras,ntg);
	    }
	}
}		/*end reconstruct_delaunay_tetra*/

LOCAL void set_tetra_geometric_params(
	TETRA		*tetra,
	TRI_GRID	*ntg)
{
	TG_PT		**pts;
	double		*p0, *p1, *p2, *p3;
	double		a0, a1, a2, b0, b1, b2, c0, c1, c2, d0, d1, d2;
	double		ad0, ad1, ad2, bd0, bd1, bd2;
	Locstate	*sts;

	pts = tetra->el.p;
	p0 = Coords(pts[0]); p1 = Coords(pts[1]);
	p2 = Coords(pts[2]); p3 = Coords(pts[3]);
	sts = tetra->el.s;
	a0 = p1[0] - p0[0]; a1 = p2[0] - p0[0]; a2 = p3[0] - p0[0];
	b0 = p1[1] - p0[1]; b1 = p2[1] - p0[1]; b2 = p3[1] - p0[1];
	c0 = p1[2] - p0[2]; c1 = p2[2] - p0[2]; c2 = p3[2] - p0[2];
	tetra->D = QDet3d(a,b,c);
	if (tetra->D < 0)
	{
	    TG_PT *ptmp;
	    Locstate stmp;
	    double tmp;
	    ptmp = pts[1];
	    pts[1] = pts[2];
	    pts[2] = ptmp;
	    p1 = Coords(pts[1]);
	    p2 = Coords(pts[2]);
	    stmp = sts[1];
	    sts[1] = sts[2];
	    sts[2] = stmp;
	    tmp = a0;	a0 = a1;       a1 = tmp;
	    tmp = b0;	b0 = b1;       b1 = tmp;
	    tmp = c0;	c0 = c1;       c1 = tmp;
	    tetra->D = -tetra->D;
	}

	d0 = a0*a0 + b0*b0 + c0*c0;
	d1 = a1*a1 + b1*b1 + c1*c1;
	d2 = a2*a2 + b2*b2 + c2*c2;

	if (tetra->D < vtol(ntg))
	{
	    tetra->coplanar = YES;
	}
	else
	{
	    tetra->coplanar = NO;
	    QCross3d(a,d,ad);
	    QCross3d(b,d,bd);
	    tetra->DN[0] = -QDot3d(c,bd);
	    tetra->DN[1] =  QDot3d(c,ad);
	    tetra->DN[2] =  QDot3d(a,bd);
	}
}		/*end set_tetra_geometric_params*/

/*
*				inside_sphere():
*
*	Determines whether the point pt lies within the circumsphere
*	for the tetrahedra defined by pts.
*
*	The algorithm is quite simple,  let
*
*	p0 = pts[0],	p1 = pts[1],	p2 = pts[2],	p3 = pts[3]
*	pt0 = pt - p0,	p10 = p1 - p0,	p20 = p2 - p0,	p30 = p3 - p0.
*
*	The center of the circumsphere, pc,  is the solution to the linear
*	equation A*pc0 = 0.5*b,  where pc0 = pc - p0 and
*	A[i][j] = pi0[j],  and b[i] = ||pi0||^2. 
*
*	This function returns YES if ||pt0 - pc|| <= ||p0 - pc||,
*	which is algebraically equivalent to the test
*
*	||pt0||^2 <= <pt0,2*pc0>.  To avoid numerical difficulties with
*	nearly planar tetrahedra,  this test is cast into the form
*
*	fabs(D)*||pt0||^2 <= <pt0,fabs(D)*2*pc0>.
*/


LOCAL boolean inside_sphere(
	TG_PT		*pt,
	TETRA		*tetra)
{
	double		*p0,pt0[3];
	double		*DN = tetra->DN;
	double		D = tetra->D;

	if (tetra->coplanar) 
	    return NO;

	p0 = Coords(tetra->el.p[0]);

	pt0[0] = Coords(pt)[0] - p0[0];
	pt0[1] = Coords(pt)[1] - p0[1];
	pt0[2] = Coords(pt)[2] - p0[2];

	if (D == 0.0)
	    return NO;
	else
	{
	    double RS,RR;
	    RS = pt0[0]*DN[0] + pt0[1]*DN[1] + pt0[2]*DN[2];
	    RR = (pt0[0]*pt0[0] + pt0[1]*pt0[1] + pt0[2]*pt0[2])*D;
	    if (D > 0.0)
	    	return (RR <= RS) ? YES : NO;
	    else
	    	return (RS <= RR) ? YES : NO;
	}
}		/*end inside_sphere*/

LOCAL boolean point_inside_hull(
	TG_PT		*pt,
	HULL		*hull,
	TRI_GRID	*ntg)
{
	int		num_faces = hull->num_faces;
	int		i;

	for (i = 0; i < num_faces; ++i)
	{
	    if (is_in_positive_face(hull->faces+i,Coords(pt),YES,ntg) == YES)
	    	return NO;
	}
	return YES;
}		/*end point_inside_hull*/

LOCAL void connect_pt_and_faces(
	TETRA		*tetras,
	TETRA		tetra_enc,
	PT_ST		*pt,
	int		*num_tetras,
	TRI_GRID	*ntg)
{
	int		i,j;

	for (i = 0; i < 4; ++i)
	{
	    if (tetra_enc.el.side[i] == CREATE_TETRA)
	    {
	    	for (j = 0; j < 4; ++j)
	    	{
	    	   tetras[*num_tetras].el.p[j] = tetra_enc.el.p[j];
	    	   tetras[*num_tetras].el.s[j] = tetra_enc.el.s[j];
	    	}
	    	tetras[*num_tetras].el.p[i] = pt->pt;
	    	tetras[*num_tetras].el.s[i] = pt->st;
	    	set_tetra_geometric_params(&tetras[*num_tetras],ntg);
	    	++(*num_tetras);
	    }
	}
}		/*end connect_pt_and_faces*/

LOCAL void reform_tetras_with_exterior_pt(
	PT_ST		*pt,
	HULL		*hull,
	TETRA		*tetras,
	int		*num_tetras,
	TETRA		*tetras_enc,
	int		num_enc,
	TRI_GRID	*ntg)
{
	static FACE  *face = NULL;
	static TETRA *t = NULL;
	int	     i,j,k;
	boolean	     i_shift[MAX_NUM_SIDES];
	int	     ns_old,ns_new;

	if (face == NULL)
	{
	    scalar(&face,sizeof(FACE));
	    scalar(&t,sizeof(TETRA));
	}

	ns_old = hull->num_faces;

	/* Remove face visible on hull */
	/* and on the encircling tetra */

	for (k = 0; k < ns_old; ++k)
	{
	    i_shift[k] = NO;
	    if (visible_face(pt->pt,hull->faces+k,YES,ntg) == YES)
	    {
	    	for (i = 0; i < num_enc; ++i)
	    	{
	    	    for (j = 0; j < 4; ++j)
	    	    {
			assign_face(&tetras_enc[i],face,j,NO,ntg);
		    	if (common_face(face,hull->faces+k) == YES)
		    	{
			    tetras_enc[i].el.side[j] = TETRA_CREATED;
			    i_shift[k] = YES;
		     	}
	    	    }
	    	}
	    	if (!i_shift[k])
	    	{
		    for (i = 0; i < 3; ++i)
		    {
		    	tetras[*num_tetras].el.p[i] = hull->faces[k].vertex[i];
		    	tetras[*num_tetras].el.s[i] = hull->faces[k].state[i];
		    }
		    tetras[*num_tetras].el.p[3] = pt->pt;
		    tetras[*num_tetras].el.s[3] = pt->st;
		    set_tetra_geometric_params(&tetras[*num_tetras],ntg);
		    if (!tetras[*num_tetras].coplanar)
		    {
		        i_shift[k] = YES;
		        for (i = 0; i < 3; ++i)
		        {
		    	    assign_face(&tetras[*num_tetras],
				        &(hull->faces[(hull->num_faces)++]),
					i,YES,ntg);
		        }
		        ++(*num_tetras);
		    }
		}
	    }
	}

		/* Remove common face of encircling tetra */

	for (i = 0; i < num_enc; ++i)
	{
	    for (j = 0; j < num_enc; ++j)
	    {
		if (i == j)
		    continue;
		else
		    remove_common_faces(&tetras_enc[i],&tetras_enc[j],ntg);
	    }
	}

		/* Make tetrahedra */

	for (i = 0; i < num_enc; ++i)
	{
	    for (j = 0; j < 4; ++j)
	    {
		if (tetras_enc[i].el.side[j] == CREATE_TETRA)
		{
		    assign_face(&tetras_enc[i],face,j,NO,ntg);
		    for (k = 0; k < 3; ++k)
		    {
		    	tetras[*num_tetras].el.p[k] = face->vertex[k];
		    	tetras[*num_tetras].el.s[k] = face->state[k];
		    }
		    tetras[*num_tetras].el.p[3] = pt->pt;
		    tetras[*num_tetras].el.s[3] = pt->st;
		    set_tetra_geometric_params(&tetras[*num_tetras],ntg);
		    for (k = 0; k < 3; ++k)
		    {
		        assign_face(&tetras[*num_tetras],
			            &(hull->faces[(hull->num_faces)++]),
				    k,YES,ntg);
		    }
		    ++(*num_tetras);
		}
	    }
	}

		/* Identify interior overlapping faces */

	ns_new = hull->num_faces;
	remove_hull_overlaps(hull,ns_old,ns_new,i_shift);

}		/*end reform_tetras_with_exterior_pt*/

LOCAL void remove_hull_overlaps(
	HULL		*hull,
	int		ns_old,
	int		ns_new,
	boolean		*i_shift)
{
	int		i,j;

	for (i = ns_old; i < ns_new; ++i) 
	    i_shift[i] = NO;

	for (i = ns_old; i < ns_new; ++i)
	{
	    for (j = i+1; j < ns_new; ++j)
	    {
		if (common_face(hull->faces+i,hull->faces+j) == YES)
		{
		    i_shift[i] = i_shift[j] = YES;
		}
	    }
	}
	shift_hull(hull,i_shift);
}		/*end remove_hull_overlaps*/

LOCAL void remove_common_faces(
	TETRA		*tetra1,
	TETRA		*tetra2,
	TRI_GRID	*ntg)
{
	int		i,j;
	static FACE *face1 = NULL,
	            *face2 = NULL;

	if (face2 == NULL)
	{
	    scalar(&face1,sizeof(FACE));
	    scalar(&face2,sizeof(FACE));
	}

	for (i = 0; i < 4; ++i)
	{
	    assign_face(tetra1,face1,i,NO,ntg);
	    for (j = 0; j < 4; ++j)
	    {
	    	assign_face(tetra2,face2,j,NO,ntg);
	    	if (common_face(face1,face2) == YES)
	    	{
	    	    tetra1->el.side[i] = tetra2->el.side[j] = TETRA_CREATED;
	    	}
	    }
	}
}		/*end remove_common_faces*/

LOCAL void assign_face(
	TETRA		*tetra,
	FACE		*face,
	int		i,
	int		flag,
	TRI_GRID	*ntg)
{
	int		j;

	for (j = 0; j < 3; ++j)
	{
	    face->vertex[j] = tetra->el.p[j];
	    face->state[j] = tetra->el.s[j];
	}
	if (i < 3)
	{
	    face->vertex[i] = tetra->el.p[3];
	    face->state[i] = tetra->el.s[3];
	}

		/* When topological information is needed */

	if (flag)
	{
	    double v1[3],v2[3];
	    double *v0 = Coords(face->vertex[0]);
	    for (j = 0; j < 3; ++j)
	    {
	    	v1[j] = Coords(face->vertex[1])[j] - v0[j];
	    	v2[j] = Coords(face->vertex[2])[j] - v0[j];
	    }
	    Cross3d(v1,v2,face->norm);
	    if (is_in_positive_face(face,Coords(tetra->el.p[i]),NO,ntg) == YES)
	    {
	    	TG_PT *ptmp;
	    	Locstate stmp;
	    	ptmp = face->vertex[2];
	    	face->vertex[2] = face->vertex[1];
	    	face->vertex[1] = ptmp;
	    	stmp = face->state[2];
	    	face->state[2] = face->state[1];
	    	face->state[1] = stmp;
	    	for (j = 0; j < 3; ++j)
	    	    face->norm[j] = -face->norm[j];
	    }
	}
}		/*end assign_face*/

LOCAL boolean is_in_positive_face(
	FACE		*face,
	double		*point,
	int		use_tol,
	TRI_GRID	*ntg)
{
	double		v3[3],psign;
	double		*norm = face->norm;
	double		tolerance;

	tolerance = use_tol ? vtol(ntg) : 0.0;
	v3[0] = point[0] - Coords(face->vertex[0])[0];
	v3[1] = point[1] - Coords(face->vertex[0])[1];
	v3[2] = point[2] - Coords(face->vertex[0])[2];
	psign = Dot3d(v3,norm);

	return (psign > -tolerance) ? YES : NO;
}		/*end is_in_positive_face*/

LOCAL boolean common_face(
	FACE		*s1,
	FACE		*s2)
{
	int		i,j;
	int		common_pt;

	for (i = 0; i < 3; ++i)
	{
	    common_pt = NO;
	    for (j = 0; j < 3; ++j)
	    {
	    	if (s1->vertex[i] == s2->vertex[j])
	    	    common_pt = YES;
	    }
	    if (common_pt == NO)
		return NO;
	}
	return YES;
}		/*end common_face*/

LOCAL boolean visible_face(
	TG_PT		*pt,
	FACE		*face,
	boolean		use_tol,
	TRI_GRID	*ntg)
{
	double		v[3],psign;
	double		tolerance;
	double		*norm = face->norm;

	tolerance = use_tol ? vtol(ntg) : 0.0;

	v[0] = Coords(pt)[0] - Coords(face->vertex[2])[0];
	v[1] = Coords(pt)[1] - Coords(face->vertex[2])[1];
	v[2] = Coords(pt)[2] - Coords(face->vertex[2])[2];

	psign = v[0]*norm[0] + v[1]*norm[1] + v[2]*norm[2];
	if (psign <= -tolerance)
	    return NO;
	return YES;
}		/*end visible_face*/

LOCAL	boolean is_coplanar_element(
	LINEAR_ELEMENT	*el,
	TRI_GRID	*ntg)
{
	double		*p0, *p1, *p2, *p3;
	double		a0, a1, a2, b0, b1, b2, c0, c1, c2;
	double		D;

	p0 = Coords(el->p[0]);
	p1 = Coords(el->p[1]);
	p2 = Coords(el->p[2]);
	p3 = Coords(el->p[3]);

	a0 = p1[0] - p0[0]; a1 = p2[0] - p0[0]; a2 = p3[0] - p0[0];
	b0 = p1[1] - p0[1]; b1 = p2[1] - p0[1]; b2 = p3[1] - p0[1];
	c0 = p1[2] - p0[2]; c1 = p2[2] - p0[2]; c2 = p3[2] - p0[2];
	D = QDet3d(a,b,c);

	return (fabs(D) < vtol(ntg)) ? YES : NO;
}	/* end is_coplanar_element */


LOCAL void shift_tetra_list(
	TETRA		*tetras,
	int		*num_tetras,
	int		*i_shift)
{
	int		i,j;

	for (i = 0; i < *num_tetras; ++i)
	{
	    if (i_shift[i])
	    {
	    	for (j = i; j < *num_tetras - 1; ++j)
	    	{
	    	    tetras[j] = tetras[j+1];
	    	    i_shift[j] = i_shift[j+1];
	    	}
	    	--(*num_tetras);
	    	--i;
	    }
	}
}		/*end shift_tetra_list*/

LOCAL void shift_hull(
	HULL		*hull,
	boolean		*i_shift)
{
	int		i,j;
	int		num_faces = hull->num_faces;
#if defined(DEBUG_TRI_GRID)
	if (debugging("tri_mesh"))
	{
		if (max_num_faces < hull->num_faces)
			max_num_faces = hull->num_faces;
	}
#endif /* defined(DEBUG_TRI_GRID) */

	for (i = 0; i < num_faces; ++i)
	{
	    if (i_shift[i])
	    {
	    	for (j = i; j < num_faces - 1; ++j)
	    	{
	    	    hull->faces[j] = hull->faces[j+1];
	    	    i_shift[j] = i_shift[j+1];
	    	}
	    	--num_faces;
	    	--i;
	    }
	}
	hull->num_faces = num_faces;
}		/*end shift_hull*/

LOCAL	int group_point_list(
	PT_ST		*g_pts,
	int		gnp,
	BLK_BIN		*blk_bin,
	COMPONENT	comp)
{
	int		num_els = blk_bin->num_els;
	TG_PT		**pt_list = blk_bin->el_list;
	Locstate	*pos_st = blk_bin->pos_st;
	Locstate	*neg_st = blk_bin->neg_st;
	int		i;

	for (i = 0; i < num_els - 8; ++i)
	{
	    if (blk_bin->pos_comp[i] == comp)
	    {
	    	g_pts[gnp].st = pos_st[i];
	    	g_pts[gnp++].pt = pt_list[i];
	    }
	    else if (blk_bin->neg_comp[i] == comp)
	    {
	    	g_pts[gnp].st = neg_st[i];
	    	g_pts[gnp++].pt = pt_list[i];
	    }
	}
	for (i = num_els - 8; i < num_els; ++i)
	{
	    if (blk_bin->pos_comp[i] == comp)
	    {
	    	g_pts[gnp].st = pos_st[i];
	    	g_pts[gnp++].pt = pt_list[i];
	    }
	}
	sort_PT_STs(g_pts,gnp,blk_bin);
	return gnp;
}		/*end group_point_list*/

LOCAL	void	sort_PT_STs(
	PT_ST		*g_pts,
	int		gnp,
	BLK_BIN		*blk_bin)
{
	TG_PT		Center;
	TG_PT		*pt, **pt_list = blk_bin->el_list;
	int		i;
	int		num_els = blk_bin->num_els;

	COORDS(Center)[0] = 0.0;
	COORDS(Center)[1] = 0.0;
	COORDS(Center)[2] = 0.0;
	for (i = num_els - 8; i < num_els; ++i)
	{
	    pt = pt_list[i];
	    COORDS(Center)[0] += Coords(pt)[0];
	    COORDS(Center)[1] += Coords(pt)[1];
	    COORDS(Center)[2] += Coords(pt)[2];
	}
	COORDS(Center)[0] *= 0.125; /*TOLERANCE*/
	COORDS(Center)[1] *= 0.125; /*TOLERANCE*/
	COORDS(Center)[2] *= 0.125; /*TOLERANCE*/
	for (i = 0; i < gnp; ++i)
	{
	    g_pts[i].dist = sqr(Coords(g_pts[i].pt)[0] - COORDS(Center)[0]) +
			    sqr(Coords(g_pts[i].pt)[1] - COORDS(Center)[1]) +
			    sqr(Coords(g_pts[i].pt)[2] - COORDS(Center)[2]);
	}
	qsort((POINTER)g_pts,gnp,sizeof(PT_ST),PtStcompr);
}		/*end sort_PT_STs*/


#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

LOCAL	int	PtStcompr(
	const void	*p1,
	const void	*p2)
{
	double dist1 = ((PT_ST*)p1)->dist;
	double dist2 = ((PT_ST*)p2)->dist;

	return (dist1 < dist2) ? -1 : (dist2 < dist1) ? 1 : 0;
}		/*end PtStcompr*/

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

#if defined(DEBUG_TRI_GRID)
LOCAL void print_hull(
	HULL	*hull)
{
	int	i;
	double	area = 0.0;

	for (i = 0; i < hull->num_faces; ++i)
	{
	    (void) printf("FACE %d: %p %p %p\n",i,
			  (POINTER)hull->faces[i].vertex[0],
			  (POINTER)hull->faces[i].vertex[1],
			  (POINTER)hull->faces[i].vertex[2]);
	    (void) printf("NORM %d: %g %g %g\n",i,
			  hull->faces[i].norm[0],hull->faces[i].norm[1],
			  hull->faces[i].norm[2]);
	    area += 0.5*sqrt(hull->faces[i].norm[0]*hull->faces[i].norm[0] +
			     hull->faces[i].norm[1]*hull->faces[i].norm[1] +
			     hull->faces[i].norm[2]*hull->faces[i].norm[2]);
	}
	(void) printf("Hull area = %g\n",area);
}		/*end print_hull*/




LOCAL void print_blk_bin(
	BLK_BIN 	*blk_bin)
{
	int		num_els = blk_bin->num_els;
	int		i;

	(void) printf("BLK_BIN: %p  num_els = %d\n",(POINTER)blk_bin,
		      blk_bin->num_els);
	for (i = 0; i < num_els; ++i)
	{
	    (void) printf("blk_els[%d] = %p ",
			  i,(POINTER)blk_bin->el_list[i]);
	    (void) printf("pos_comp[%d] = %d  neg_comp[%d] = %d\n",
			  i,blk_bin->pos_comp[i],i,blk_bin->neg_comp[i]);
	}
}		/*end print_blk_bin*/


LOCAL void print_tetra(
	TETRA	*tetra)
{
	int	i;

	(void) printf("Tetra %p\n",(POINTER)tetra);
	for (i = 0; i < 4; ++i)
	{
	    (void) printf("Point %p: %g %g %g\n",(POINTER)tetra->el.p[i],
	    	          Coords(tetra->el.p[i])[0],
	    	          Coords(tetra->el.p[i])[1],
	    	          Coords(tetra->el.p[i])[2]);
	}
}		/*end print_tetra*/

LOCAL double tetra_volume(
	TETRA	*tetra)
{
	double	v1[3],v2[3],v3[3],volume;

	v1[0] = Coords(tetra->el.p[1])[0] - Coords(tetra->el.p[0])[0];
	v1[1] = Coords(tetra->el.p[1])[1] - Coords(tetra->el.p[0])[1];
	v1[2] = Coords(tetra->el.p[1])[2] - Coords(tetra->el.p[0])[2];

	v2[0] = Coords(tetra->el.p[2])[0] - Coords(tetra->el.p[0])[0];
	v2[1] = Coords(tetra->el.p[2])[1] - Coords(tetra->el.p[0])[1];
	v2[2] = Coords(tetra->el.p[2])[2] - Coords(tetra->el.p[0])[2];

	v3[0] = Coords(tetra->el.p[3])[0] - Coords(tetra->el.p[0])[0];
	v3[1] = Coords(tetra->el.p[3])[1] - Coords(tetra->el.p[0])[1];
	v3[2] = Coords(tetra->el.p[3])[2] - Coords(tetra->el.p[0])[2];

	volume = Det3d(v1,v2,v3)/6.0;
	return fabs(volume);
}		/*end tetra_volume*/

#endif /* defined(DEBUG_TRI_GRID) */

#endif /* defined(THREED) */
