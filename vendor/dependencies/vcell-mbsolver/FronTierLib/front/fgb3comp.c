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
*                               tricrx.c:
*
*       Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*/


#define DEBUG_STRING "crx_intfc"

#include <front/fdecs.h>

#define  is_moving_curve(c)    (curve_type(c) == NEUMANN_CURVE_P || \
                                curve_type(c) == NEUMANN_CURVE_W )


	/*LOCAL Function Prototypes*/
LOCAL	boolean	fill_comp_from_prev_intfc(INTERFACE *, int *, int *);
LOCAL	boolean	fill_comp_from_solid_intfc(Front *, int *, int *);
LOCAL	void    set_wall_flag_storage(INTERFACE *);
LOCAL	void    free_wall_flag_storage(INTERFACE *);
LOCAL	boolean    set_wall_crossings3d(INTERFACE *, int *, int *);
LOCAL	boolean    fix_crx_on_wall(CRXING*,INTERFACE*,int,int);
LOCAL	boolean    make_wall_surfaces(INTERFACE *, int *, int *);
LOCAL	int     count_grid_curve_crossings3d(INTERFACE *, int *, int *);
LOCAL	void    set_curve_crx_storage(INTERFACE *, int *, int *);
LOCAL	void    free_curve_crx_storage(INTERFACE *);
LOCAL   void    insert_curve_face_crossings(INTERFACE*,int*,GRID_DIRECTION,int*);
LOCAL	int 	insert_grid_based_curve_crossings3d(INTERFACE *, int *, int *);
LOCAL	int 	make_wall_curves(INTERFACE *, int *, int *);
LOCAL   void    adjust_face_crossing(double *, INTERFACE *, int *, GRID_DIRECTION);
LOCAL   void    strip_wall_moving_curves(INTERFACE *);
LOCAL   void    insert_fixed_curve_crossings(INTERFACE*,int*,int*, int*);
LOCAL   void    print_face_crossings(INTERFACE *);
LOCAL   void    print_curve_face_crossings(INTERFACE *, int*, GRID_DIRECTION);
LOCAL   void    tecplot_tris_in_box(char*, int, int, int, INTERFACE *);


LOCAL   void  tecplot_tris_in_box(
	char       *fname,
	int 	   ibx, 
	int 	   iby, 
	int 	   ibz, 
	INTERFACE  *intfc)
{
	struct Table    *T = table_of_interface(intfc);
	TRI		**tris;
	SURFACE		**surfs;
	int		num_tris, i, j;
	double		*p;
	FILE		*fp;
	char		fname1[100];

	tris = T->tris[ibz][iby][ibx];
	surfs = T->surfaces[ibz][iby][ibx];
	num_tris = T->num_of_tris[ibz][iby][ibx];

	if(num_tris == 0)
	    return;
	
	if(fname != NULL)
	{
	    sprintf(fname1, "%s_%d.plt", fname, pp_mynode());
	    fp = fopen(fname1, "w");
	    if(fp == NULL)
	    {
	        printf("tecplot_tris_in_box: can not open %s\n", fname1);
		return;
	    }
	}
	else
	    return; 

	fprintf(fp, "ZONE  N=%d  E=%d\n", 3*num_tris, num_tris);
	fprintf(fp, "F=FEPOINT, ET=TRIANGLE\n");

	for(i=0; i<num_tris; i++)
	{
	    if(surfs[i]->number == ON_WALL)
	        continue;
	    for(j=0; j<3; j++)
	    {
	        p = Coords(Point_of_tri(tris[i])[j]);
	        fprintf(fp, "%12.8e  %12.8e  %12.8e\n", p[0], p[1], p[2]);
	    }
	}
	
	for(i=0; i<num_tris; i++)
	{
	    fprintf(fp, "%d  %d  %d\n", 3*i+1, 3*i+2, 3*i+3);
	}

	fclose(fp);
}




/*
*		rebuild_intfc_at_crossings3d3():
*/


EXPORT	boolean	rebuild_intfc_at_crossings3d3(
	Front     *front)
{
	INTERFACE *intfc = front->interf;
	boolean   	  sav_intrp = interpolate_intfc_states(intfc);
	int 	  i, smin[3], smax[3];
	size_t	  sizest = front->sizest;
	RECT_GRID Dual_grid, *comp_grid = front->rect_grid;
	RECT_GRID gr_save = topological_grid(intfc);
	RECT_GRID *gr;

	DEBUG_ENTER(rebuild_intfc_at_crossings3d)

	set_dual_grid(&Dual_grid,comp_grid);
	set_size_of_intfc_state(sizest);
	set_copy_intfc_states(YES);
	
	print_storage("Entering rebuild_intfc_at_crossings3d","crx_store");

	set_expanded_grid(&Dual_grid,&topological_grid(intfc));
	
	gr = &topological_grid(intfc);
	/* set reconstruction boundary and tolerance */

	for (i = 0; i < 3; ++i)
	{
	    smin[i] = 0;	
	    smax[i] = topological_grid(intfc).gmax[i]; /*expanded_dual_grid*/
            if (Dual_grid.lbuf[i] != 0)
		smin[i] += Dual_grid.lbuf[i] - 1;
	    else if (curves_on_bdry_side(i,0,intfc) == YES)
	        smin[i] += 1;
            if (Dual_grid.ubuf[i] != 0)
		smax[i] -= Dual_grid.ubuf[i] - 1;
	    else if (curves_on_bdry_side(i,1,intfc) == YES)
	        smax[i] -= 1;
	}

	/*do not need this function for 3 comp code */
	/*only deal with the subdomain curves, they are far from reconstruction */
	/*region. Deactivate this part. */
	/*extend_boundary_side(intfc,smin,smax,gr); */

	make_interface_topology_lists(intfc);

	/*should alloc components first count crx after fill_comp_from.. */
        set_crx_storage_for_reconstruction(intfc,NULL);
	
	start_clock("insert_grid_crossings3d");
	interpolate_intfc_states(intfc) = YES;
	insert_grid_intfc_crossings(intfc);
	stop_clock("insert_grid_crossings3d");
	print_storage("After insert_grid_crossings3d","crx_store");
	
	set_wall_flag_storage(intfc);
	set_use_wall_edge(NO);
	
	/*set wall flag */
	set_wall_flag_for_surface(front->interf);

	/*should before make_interface_topology_lists */
	fill_comp_from_solid_intfc(front, smin, smax);
	
	/*assume compon3d of previous interface is ft_assigned */
	fill_comp_from_prev_intfc(intfc, smin, smax);

	set_use_wall_edge(YES);

	/*printf("#sec call remove_unphysical_crossings3d\n"); */
	start_clock("remove_unphysical_crossings3d");
	
	if (!remove_unphysical_crossings3d(intfc,smin,smax))
	{
	    interpolate_intfc_states(intfc) = sav_intrp;
	    DEBUG_LEAVE(rebuild_intfc_at_crossings3d)
	    return NO;
	}

	/*consistent_interface will fail after here because new surface  */
	/*without tris is made */
	set_curve_crx_storage(intfc, smin, smax);
	make_wall_surfaces(front->interf, smin, smax);
	/*WARNNING print_surface will fail after this function */
	
	set_wall_crossings3d(intfc,smin,smax);
	
	insert_grid_based_curve_crossings3d(intfc,smin,smax);

	/*delete tmp wall curves */
	strip_wall_moving_curves(intfc);
	
	make_wall_curves(intfc, smin, smax);
	/*WARNNING print_curve will fail after this function */
	
	/*test orient. */
	/*check_wall_crx_orient(intfc, smin, smax); */

	set_use_wall_edge(NO);

	stop_clock("remove_unphysical_crossings3d");
	strip_subdomain_bdry_curves(intfc);

	start_clock("reconstruct_intfc3d_in_box");
	intfc->modified = NO;
	
	if (!reconstruct_intfc3d_in_box(intfc,smin,smax,YES,NULL))
	{
	    interpolate_intfc_states(intfc) = sav_intrp;
	    DEBUG_LEAVE(rebuild_intfc_at_crossings3d)
	    return NO;
	}
	stop_clock("reconstruct_intfc3d_in_box");
	print_storage("After reconstruct_intfc3d_in_box","crx_store");
	
	interpolate_intfc_states(intfc) = NO;
	
	null_sides_are_consistent();

	install_subdomain_bdry_curves(intfc);
	interpolate_intfc_states(intfc) = sav_intrp;

	identify_detached_surface_curve_pair(intfc);

	reset_intfc_num_points(intfc);
	topological_grid(intfc) = gr_save;
     
	free_crx_storage(intfc);
	
	free_curve_crx_storage(intfc);
	free_wall_flag_storage(intfc);

	front->interf = copy_interface(intfc);
	
	delete_interface(intfc);
	
	interface_reconstructed(front->interf) = YES;
	print_storage("After copy_interface","crx_store");
	DEBUG_LEAVE(rebuild_intfc_at_crossings3d)
	return YES;
}	/*end rebuild_intfc_at_crossings3d */


EXPORT  boolean  is_wall_side(TRI *t, int side)
{
BOND_TRI    *btri;
CURVE	    *c;

	if(!is_side_bdry(t, side))
	    return NO;
	btri = Bond_tri_on_side(t, side);
	if(btri == NULL)
	    return YES;
	c = btri->curve;
	
	if(curve_type(c) == NEUMANN_CURVE || 
	   curve_type(c) == NEUMANN_CURVE_W)
	    return NO;
	
	return YES;
}

EXPORT  TRI*  Tri_on_side_along_wall(
	int		*is,
	TRI 		*t, 
	int 		side)
{
BOND_TRI    **btris;
BOND	    *b;
TRI	    *n_t;
FT_POINT	    *p;
int	    i;

	if(!is_side_bdry(t, side))
	{
	    n_t = Tri_on_side(t, side);
	    p = Point_of_tri(t)[Next_m3(side)];
	    *is = Vertex_of_point(n_t, p);
	    return n_t;
	}
	if(is_wall_side(t, side))
	    return NULL;

	/*t, side is NEUMAN_CURVE or NEUMANN_CURVE_W */
	b = Bond_on_side(t, side);

	n_t = NULL;
	for(btris=Btris(b); btris && *btris; btris++)
	{
	    /*ASSUME  wall_surface flag is ft_assigned */
	    if( (*btris)->tri == t || (!is_wall_surface((*btris)->surface)) )
	        continue;
	    n_t = (*btris)->tri;
	    break;
	}
	if(n_t == NULL)
	{
	    printf("ERROR Tri_on_side_along_wall, no wall tri found\n");
	    clean_up(ERROR);
	}

	for(i=0; i<3; i++)
	{
	    if(!is_side_bdry(n_t, i))
	        continue;
	    if(Bond_on_side(n_t, i) == b)
	    {
	        *is = i;
		break;
	    }
	}

	printf("#across curve  %p  %d\n", n_t, *is);

	return n_t;
}

/*YES mean vertex is not on NEUMANN_CURVE_P and NEUMANN_CURVE_W */
EXPORT  boolean  is_wall_vertex(TRI *t, int iv)
{
SURFACE      *surf;
FT_POINT	     *p;
TRI	     **tris;
int	     i, nt;
	
	surf = t->surf;
	if(surf == NULL)
	{
	    printf("ERROR is_wall_vertex tri is not related to surf.\n");
	    clean_up(ERROR);
	}
	p = Point_of_tri(t)[iv];
	nt = set_tri_list_around_point(p, t, &tris, surf->interface);
	
	for(i=0; i<nt; i++)
	{
	    iv = Vertex_of_point(tris[i], p);
	    if( is_wall_side(tris[i],iv) || 
	        is_wall_side(tris[i],Prev_m3(iv)) )
		return YES;
	}
	return NO;
}

EXPORT  int tri_list_along_wall(
	FT_POINT     *p,
	TRI       *tri,
	TRI       ***ptris,
	INTERFACE *intfc)
{
static TRI	**tris = NULL;
TRI		*t, *n_t, **ptris0;
int		iv, is, i, nt, nt1;

	if (tris == NULL)
	{
	    uni_array(&tris,50,sizeof(TRI *));
	}

	nt = set_tri_list_around_point(p, tri, &ptris0, intfc);
	if(nt == -1)
	{
	    printf("ERROR tri_list_along_wall set_tri_list_around fails.\n");
	    clean_up(ERROR);
	}
	for(i=0; i<nt; i++)
	    tris[i] = ptris0[i];

	t = ptris0[0];
	iv = Next_side_at_vertex(t, p);
	if(is_side_bdry(t, iv))
	{
	    n_t = Tri_on_side_along_wall(&is, t, iv);
	    if(n_t != NULL)
	    {
	        nt1 = set_tri_list_around_point(p, n_t, &ptris0, intfc);
		if(nt1 == -1)
		{
	    	    printf("ERROR tri_list_along_wall set_tri_list_around fails 1.\n");
	    	    clean_up(ERROR);
		}
		for(i=0; i<nt1; i++)
	            tris[i+nt] = ptris0[i];
		*ptris = tris;
		return nt+nt1;
	    }
	}
	
	t = ptris0[nt-1];
	iv = Prev_side_at_vertex(t, p);
	if(is_side_bdry(t, iv))
	{
	    n_t = Tri_on_side_along_wall(&is, t, iv);
	    if(n_t != NULL)
	    {
	        nt1 = set_tri_list_around_point(p, n_t, &ptris0, intfc);
		if(nt1 == -1)
		{
	    	    printf("ERROR tri_list_along_wall set_tri_list_around fails 2.\n");
	    	    clean_up(ERROR);
		}
		for(i=0; i<nt1; i++)
	            tris[i+nt] = ptris0[i];
		*ptris = tris;
		return nt+nt1;
	    }
	}

	*ptris = tris;
	return nt;
}

EXPORT boolean edge_index_of_face(
	int 	*ie, 
	int 	ix,
	int	iy,
	int	iz, 
	GRID_DIRECTION  dir,
	int 	*gmax)
{	
	switch (dir)
	{
	case WEST:
	    ix--;
	case EAST:
	    ie[0] = seg_index3d(ix,iy,iz,NORTH,gmax);
	    ie[1] = seg_index3d(ix,iy,iz+1,NORTH,gmax);
	    ie[2] = seg_index3d(ix,iy,iz,UPPER,gmax);
	    ie[3] = seg_index3d(ix,iy+1,iz,UPPER,gmax);
	    return YES;
	case SOUTH:
	    iy--;
	case NORTH:
	    ie[0] = seg_index3d(ix,iy,iz,EAST,gmax);
	    ie[1] = seg_index3d(ix,iy,iz+1,EAST,gmax);
	    ie[2] = seg_index3d(ix,iy,iz,UPPER,gmax);
	    ie[3] = seg_index3d(ix+1,iy,iz,UPPER,gmax);
	    return YES;
	case LOWER:
	    iz--;
	case UPPER:
	    ie[0] = seg_index3d(ix,iy,iz,EAST,gmax);
	    ie[1] = seg_index3d(ix,iy+1,iz,EAST,gmax);
	    ie[2] = seg_index3d(ix,iy,iz,NORTH,gmax);
	    ie[3] = seg_index3d(ix+1,iy,iz,NORTH,gmax);
	    return YES;
	default:
	    screen("ERROR in edge_index_of_face(), invalid direction %d\n",dir);
	    clean_up(ERROR);
	}
	return NO;
}

EXPORT boolean vertex_index_of_face(
	int 	*iv, 
	int 	ix,
	int	iy,
	int	iz, 
	GRID_DIRECTION  dir,
	int 	*gmax)
{	
	switch (dir)
	{
	case WEST:
	    ix--;
	case EAST:
	    iv[0] = d_index3d(ix,iy,iz,gmax);
	    iv[1] = d_index3d(ix,iy+1,iz,gmax);
	    iv[2] = d_index3d(ix,iy+1,iz+1,gmax);
	    iv[3] = d_index3d(ix,iy,iz+1,gmax);
	    return YES;
	case SOUTH:
	    iy--;
	case NORTH:
	    iv[0] = d_index3d(ix,iy,iz,gmax);
	    iv[1] = d_index3d(ix+1,iy,iz,gmax);
	    iv[2] = d_index3d(ix+1,iy,iz+1,gmax);
	    iv[3] = d_index3d(ix,iy,iz+1,gmax);
	    return YES;
	case LOWER:
	    iz--;
	case UPPER:
	    iv[0] = d_index3d(ix,iy,iz,gmax);
	    iv[1] = d_index3d(ix+1,iy,iz,gmax);
	    iv[2] = d_index3d(ix+1,iy+1,iz,gmax);
	    iv[3] = d_index3d(ix,iy+1,iz,gmax);
	    return YES;
	default:
	    screen("ERROR in vertex_index_of_face(), invalid direction %d\n",dir);
	    clean_up(ERROR);
	}
	return NO;
}

LOCAL  void strip_wall_moving_curves(
	INTERFACE *intfc)
{
	DEBUG_ENTER(strip_wall_moving_curves)

	/*ASSUME  two kinds of curves is moving */
	/*init part is in detach_moving_curves */
	strip_bdry_curves(intfc, NEUMANN_CURVE_P);
	strip_bdry_curves(intfc, NEUMANN_CURVE_W);

	DEBUG_LEAVE(strip_wall_moving_curves)
}

/*djust_for_min_spacing */
LOCAL void adjust_face_crossing(
	double		*pt,
	INTERFACE	*intfc,
	int		*icrds,
	GRID_DIRECTION  dir)
{
RECT_GRID       *gr = &topological_grid(intfc);
double 		*L = gr->L,  *h = gr->h, smin, smax;
double		tol = 0.0039;
int		i, j, k;

	if(dir == EAST  ||  dir == WEST)
	    k = 0;
	if(dir == NORTH  ||  dir == SOUTH)
	    k = 1;
	if(dir == UPPER  ||  dir == LOWER)
	    k = 2;

	for(i=1; i<=2; i++)
	{
	    j = (k+i)%3;
	    smin = L[j] + icrds[j]*h[j];
	    smax = smin + h[j];

	    if( pt[j]-smin < tol*h[j] )
	    {
	        /*printf("#adj curve %12.8e  %12.8e %d %d\n",pt[j]-smin,tol*h[j], k, j); */
	        pt[j] = smin + tol*h[j];
	    }
	    else  if( pt[j]-smax > -tol*h[j] )
	    {
	        /*printf("#adj curve1 %12.8e  %12.8e %d %d\n",pt[j]-smax,tol*h[j], k, j); */
	        pt[j] = smax - tol*h[j];
	    }
	}

}

#define  MAX_CURVE_CRX_SORT    100

typedef  struct {
	double    crx[3];
	TRI      *tri;
	SURFACE  *surf;
}    CURVE_CRX_SORT;

/*WARN3  arrange.  */
int compare_crx(const void *a, const void *b)
{
CURVE_CRX_SORT  *c1=(CURVE_CRX_SORT*)a, *c2=(CURVE_CRX_SORT*)b;
double	*crx1 = c1->crx, *crx2 = c2->crx;
int	i;

       if(crx1[0] + crx1[1] + crx1[2] < crx2[0] + crx2[1] + crx2[2])
           return -1;
       else
           return 1;

}

LOCAL boolean  tst_line_tri_crossing_on_face(
	double	   	*crx,
	TRI		**ptri,
	SURFACE		**psurf,
	double	   	*pt1,	/*two points of the line, should be on face icoords. */
	double	   	*pt2,
	int	   	*icoords,
	INTERFACE  	*intfc,
	double		tol)
{
struct Table    *T = table_of_interface(intfc);
int		*gmax = T->rect_grid.gmax;
TRI		**t;
SURFACE		**s;
int		i, k, num, ix, iy, iz;	
int		ix1, ix2, iy1, iy2, iz1, iz2;
static double	**fbox = NULL;
static CURVE_CRX_SORT    *c_crx = NULL;

	if(fbox == NULL)
	    bi_array(&fbox, 2, 3, FLOAT);
	if(c_crx == NULL)
	    uni_array(&c_crx, MAX_CURVE_CRX_SORT, sizeof(CURVE_CRX_SORT));

	for(i=0; i<3; i++)
	{
	    fbox[0][i] = min(pt1[i], pt2[i]) - tol;
	    fbox[1][i] = max(pt1[i], pt2[i]) + tol;
	}

	ix = icoords[0];
	iy = icoords[1];
	iz = icoords[2];
	
	ix1 = ix - 1;
	if (ix1 < 0)
	    ix1 = ix;
	ix2 = ix + 1;
	if (ix2 == gmax[0])
	    ix2 = ix;
	iy1 = iy - 1;
	if (iy1 < 0)
	    iy1 = iy;
	iy2 = iy + 1;
	if (iy2 == gmax[1])
	    iy2 = iy;
	iz1 = iz - 1;
	if (iz1 < 0)
	    iz1 = iz;
	iz2 = iz + 1;
	if (iz2 == gmax[2])
	    iz2 = iz;

	num = 0;
	for (ix = ix1; ix <= ix2; ++ix)
	{
	    for (iy = iy1; iy <= iy2; ++iy)
	    {
	        for (iz = iz1; iz <= iz2; ++iz)
	        {
	            if (T->compon3d[iz][iy][ix] != ONFRONT)
	                continue;
	            t = T->tris[iz][iy][ix];
	            s = T->surfaces[iz][iy][ix];
	            for (k = 0; k < T->num_of_tris[iz][iy][ix]; ++k, ++t, ++s)
	            {
		        if(is_wall_surface(*s))
	        	    continue;
 
		        if(is_tri_outside_box(*t, fbox))
			    continue;
           	
			if(debugging("line_tri"))
	    		{
	        	    printf("\n %d %d %d\n", ix, iy, iz);
	        	    print_tri(*t, intfc);
	    		}
	    		if(line_tri_crossing(crx,*t, pt1, pt2, tol))
	    		{
			    *ptri = *t;
			    *psurf = *s;
			    
			    /*printf("#crx found\n"); */
			    /*print_general_vector("crx = ", crx, 3, "\n"); */

			    ft_assign(c_crx[num].crx, crx, 3*FLOAT);
			    c_crx[num].tri = *t;
			    c_crx[num].surf = *s;
	    		    num++;
			}
		    }
		}
	    }
	}

	qsort(c_crx, num, sizeof(CURVE_CRX_SORT), compare_crx);

	/*printf("#sort af num=%d\n", num); */
	for(k=0; k<num; k++)
	{
	    print_tri(c_crx[k].tri, (c_crx[k].surf)->interface);
	    print_general_vector("crx=", c_crx[k].crx, 3, "\n");
	    printf("\n");
	}

        printf("#all crx\n");
        for(k=0; k<num; k++)
        {
            print_general_vector("crx=", c_crx[k].crx, 3, "\n");
        }

	printf("#sort end\n");

	return NO;
}


/*ref: nearest_interface_point3d */
LOCAL boolean  line_tri_crossing_on_face(
	double	   	*crx,
	TRI		**ptri,
	SURFACE		**psurf,
	double	   	*pt1,	/*two points of the line, should be on face icoords. */
	double	   	*pt2,
	int	   	*icoords,
	INTERFACE  	*intfc,
	double		tol)
{
struct Table    	 *T = table_of_interface(intfc);
int			 *gmax = T->rect_grid.gmax;
TRI			 **t;
SURFACE			 **s;
int			 i, k, num, ix, iy, iz;	
int			 ix1, ix2, iy1, iy2, iz1, iz2;
static double		 **fbox = NULL;
static CURVE_CRX_SORT    *c_crx = NULL;

	if(fbox == NULL)
	    bi_array(&fbox, 2, 3, FLOAT);
	if(c_crx == NULL)
	    uni_array(&c_crx, MAX_CURVE_CRX_SORT, sizeof(CURVE_CRX_SORT));

	for(i=0; i<3; i++)
	{
	    fbox[0][i] = min(pt1[i], pt2[i]) - tol;
	    fbox[1][i] = max(pt1[i], pt2[i]) + tol;
	}

	ix = icoords[0];
	iy = icoords[1];
	iz = icoords[2];
	
	ix1 = ix - 1;
	if (ix1 < 0)
	    ix1 = ix;
	ix2 = ix + 1;
	if (ix2 == gmax[0])
	    ix2 = ix;
	iy1 = iy - 1;
	if (iy1 < 0)
	    iy1 = iy;
	iy2 = iy + 1;
	if (iy2 == gmax[1])
	    iy2 = iy;
	iz1 = iz - 1;
	if (iz1 < 0)
	    iz1 = iz;
	iz2 = iz + 1;
	if (iz2 == gmax[2])
	    iz2 = iz;

	num = 0;
	for (ix = ix1; ix <= ix2; ++ix)
	{
	    for (iy = iy1; iy <= iy2; ++iy)
	    {
	        for (iz = iz1; iz <= iz2; ++iz)
	        {
	            if (T->compon3d[iz][iy][ix] != ONFRONT)
	                continue;
	            t = T->tris[iz][iy][ix];
	            s = T->surfaces[iz][iy][ix];
	            for (k = 0; k < T->num_of_tris[iz][iy][ix]; ++k, ++t, ++s)
	            {
		        if(is_wall_surface(*s))
	        	    continue;
 
		        if(is_tri_outside_box(*t, fbox))
			    continue;
           	
			if(debugging("line_tri"))
	    		{
	        	    printf("\n %d %d %d\n", ix, iy, iz);
	        	    print_tri(*t, intfc);
	        	    /*print_tri_coords(tris[i]); */
	    		}

	    		if(line_tri_crossing(crx,*t, pt1, pt2, tol))
	    		{
			    ft_assign(c_crx[num].crx, crx, 3*FLOAT);
			    c_crx[num].tri = *t;
			    c_crx[num].surf = *s;
	    		    num++;
	    		}
		    }
		}
	    }
	}

	if(num > 0)
	{
	    qsort(c_crx, num, sizeof(CURVE_CRX_SORT), compare_crx);
	    
	    ft_assign(crx, c_crx[0].crx, 3*FLOAT);
	    *ptri = c_crx[0].tri;
	    *psurf = c_crx[0].surf;
	    return YES;
	}
	
	return NO;
}


LOCAL  boolean  insert_fixed_curve_crx_on_face(
	FT_POINT	   *p,
	BOND	   *b,
	CURVE      *curve, 
	int	   *smin, 
	int	   *smax, 
	INTERFACE  *intfc, 
	int	   *index)
{
	struct Table    *T = table_of_interface(intfc);
	RECT_GRID       *rgr = &topological_grid(intfc);
	size_t 		sizest = size_of_state(intfc);
	CRXING		*crx_list;
	int		*face_list;
	TRI		*tri;
	SURFACE		*surf, *s;
	FT_POINT		*newp;
	BOND_TRI	**btris;
	Locstate  	sl, sr;
	double		face_crds[3], *coords, d, dmin;
	int		i, j, k;
	int		nc[1], icrds[3], icrds1[3];
	int		*gmax = rgr->gmax;
	double		*h = rgr->h;
	static  GRID_DIRECTION  dir, DIR[3] = {EAST, NORTH, UPPER};

	for(i=0, btris=Btris(b); btris && *btris; btris++, i++)
	{
	    if(curve != (*btris)->curve)
	    {
	        printf("ERROR insert_fixed_curve_crossings, "
		       "curve inconsistent.\n");
		clean_up(ERROR);
	    }

	    /*find a surface which has physical states on one side. */
	    s = (*btris)->surface;
	    if(i == 0)
	    {
	        tri = (*btris)->tri;
		surf = s;
	    }
	    else  if( wave_type(s) > wave_type(surf) )
	    {
	        tri = (*btris)->tri;
		surf = s;
	    }
	}

	coords = Coords(p);
	if(!rect_in_which(coords, icrds, rgr))
	{
	    return NO;
	}
	
	for(i=0; i<3; i++)
	    face_crds[i] = cell_edge(icrds[i], i, rgr);

	dmin = HUGE_VAL;
	for(i=0; i<3; i++)
	    for(j=0; j<2; j++)
	    {
	        d = fabs(coords[i] - face_crds[i] - j*h[i]);
		if(d < dmin)
		{
		    dmin = d;
		    for(k=0; k<3; k++)
		        icrds1[k] = i==k ? icrds[k] + j : icrds[k];
		    dir = DIR[i];
		}
	    }
	

	/*count_grid_curve_crossings3d case */
	for(i=0; i<3; i++)
	{
	    if(icrds1[i]<smin[i] || icrds1[i]>smax[i])
	        return NO;
	}
	if (icrds1[0] == smax[0] && (dir == NORTH || dir == UPPER))
	    return NO;
	if (icrds1[1] == smax[1] && (dir == EAST || dir == UPPER))
	    return NO;
	if (icrds1[2] == smax[2] && (dir == EAST || dir == NORTH))
	    return NO;

	k = face_index3d(icrds1[0],icrds1[1],icrds1[2],dir,gmax);

	if(T->curve_crx_count[k] == 0)
	{
	    printf("ERROR insert_fixed_curve_crx_on_face, invalid fixed curve crx.\n");
	    clean_up(ERROR);
	}
	if(T->curve_crx_type[k] != 0)
	{
	    printf("ERROR insert_fixed_curve_crx_on_face, multi curve crx, invalid for grid based.\n");
	    clean_up(ERROR);
	}

	T->curve_crx_type[k]++;
	face_list = T->curve_crx_lists[k];
	crx_list = T->curve_crx_store + *index;
	
	/*only one curve crx for grid based case */
	*nc = 0;

	newp = crx_list[*nc].pt = Point(coords);
	intfc->modified = NO;
	Index_of_point(newp) = k;
	    
	/*printf("#fixed crx0  %d  %d\n", *index, crx_list); */
	/*print_general_vector("#curve crds0= ", coords, 3, "\n"); */
	    
	crx_list[*nc].hsb = Hyper_surf_bdry(curve); 
	crx_list[*nc].tri = tri;
	/*find states from  surf */
	slsr(p, Hyper_surf_element(tri), Hyper_surf(surf), &sl, &sr);
	ft_assign(left_state(newp), sl, sizest);
	ft_assign(right_state(newp), sr, sizest);
	crx_list[*nc].lcomp = negative_component(surf);
	crx_list[*nc].ucomp = positive_component(surf);
	
	if(debugging("curve_fix_crx"))
	{
	    print_wall_curve_crx("fix curve_fix", icrds, (int) dir, k, &crx_list[*nc]);
	    /*remove_from_debug("curve_fix_crx"); */
	}

	crx_list[*nc].crx_num = 0;
	face_list[*nc] = *index;
	++(*nc);    /*useless for grid based */
	++(*index);

	return YES;
}

LOCAL void  insert_fixed_curve_crossings(
	INTERFACE	*intfc, 
	int		*smin,
	int		*smax, 
	int		*index)
{	
	BOND		*b;
	CURVE		**c;

	for(c=intfc->curves; c && *c; c++)
	{
	    if(is_moving_curve(*c) || curve_type(*c) == SUBDOMAIN_HSBDRY)
	        continue;
	    
	    /*NODE: since smin>0 and smax<gmax, node on boundary  will not be inserted in faces,  */
	    /*but the node of a closed curve should be included. */
	    for(b = (*c)->first; b; b = b->next)
	        insert_fixed_curve_crx_on_face(b->start, b, *c, smin, smax, intfc, index);
	    /*this line is useless since (*c)->last->end is out of smin smax. */
	    if(!is_closed_curve(*c))
	        insert_fixed_curve_crx_on_face((*c)->last->end, (*c)->last, *c, smin, smax, intfc, index);
	}
}

LOCAL	void   print_face_crossings(
	INTERFACE       *intfc)
{
	int  icrds[3] = {15, 17, 13};
	GRID_DIRECTION    dir = EAST;

	print_curve_face_crossings(intfc, icrds, dir);
}

LOCAL	void   print_curve_face_crossings(
	INTERFACE	*intfc,
	int		*icrds,             /* node  icoords     */
	GRID_DIRECTION  dir)
{
	struct Table    *T = table_of_interface(intfc);
	RECT_GRID       *rgr = &topological_grid(intfc);
	CRXING		*crx;
	SURFACE		*s;
	int		i, j, k, ie[4], ix,iy,iz;
	int		*gmax = rgr->gmax;

	ix = icrds[0];
	iy = icrds[1];
	iz = icrds[2];

	edge_index_of_face(ie,ix,iy,iz,dir,gmax);

	/*(1)moving curve: find 2 wall crx and 1 fluid crx on 4 edges of the face. */
	/*   fixed curve: find 3 wall crx on 4 edges of the face. */
	
	for(i=0; i<4; i++)
	{
	    for(j=0; j<T->seg_crx_count[ie[i]]; j++)
	    {
	        k = T->seg_crx_lists[ie[i]][j];
	        crx = &T->crx_store[k];

	        s = Surface_of_hs(crx->hs);
	        printf("%d %d   %d  %d \n", i, j, T->seg_crx_count[ie[i]], 
	            is_wall_surface(s) );
	        print_wall_crx("w", icrds, -1, -1, crx);
	    }
	}   
}


/*ref: LOCAL void insert_block_crossings */
/*
 *       case 1		   case 2
 *   f2---c-----f1	f1--------f1
 *    | 	|	|	  |
 *    |		|	|	  c
 *    x		x	x	  |
 *    |		|	|	  |
 *    w---------w	w-----x---f2
 *    
 *    x: wall  crx
 *    c: fluid crx
*/
LOCAL void insert_curve_face_crossings(
	INTERFACE	*intfc,
	int		*icrds, 
	GRID_DIRECTION  dir,
	int		*index)
{
	struct Table    *T = table_of_interface(intfc);
	RECT_GRID       *rgr = &topological_grid(intfc);
	size_t 		sizest = size_of_state(intfc);
	CRXING		*crx_list, *crx0, *crx;
	int		*face_list;
	TRI		*tri;
	SURFACE		*surf, *crx_surf;
	HYPER_SURF	*hs_on;
	FT_POINT		*newp;
	Locstate  	sl, sr;
	double		crds_crx[MAXD], *pt1, *pt2, *coords;
	double           *h = rgr->h, tol;
	int		i,j,k,l,iv,ie[4], c0, c1, fixed;
	int		ix,iy,iz;
	int		*gmax = rgr->gmax;
	int		nc[1];
	boolean		found;

	tol = min3(h[0], h[1], h[2])*1.0e-5;
	ix = icrds[0];
	iy = icrds[1];
	iz = icrds[2];

	edge_index_of_face(ie,ix,iy,iz,dir,gmax);

	/*(1)moving curve: find 2 wall crx and 1 fluid crx on 4 edges of the face. */
	/*   fixed curve: find 3 wall crx on 4 edges of the face. */
	
	j = 0;
	l = 0;
	for(i=0; i<4; i++)
	{
	    if(T->seg_crx_count[ie[i]] == 0)
	        continue;
	    k = T->seg_crx_lists[ie[i]][0];
	    crx = &T->crx_store[k];
	    
	    if(is_wall_surface(Surface_of_hs(crx->hs)))
	    {
	        /*2 wall crx */
	        if(j == 0)
	            pt1 = Coords(crx->pt);
	        else  if(j == 1)
	            pt2 = Coords(crx->pt);
	        else  if(j == 2)
		{
		    print_general_vector("third wall crx", Coords(crx->pt), 3, "\n");
		}
	        else
		{
	            printf("ERROR in insert_curve_crossings ");
		    printf("4 wall crossings in one face, impossible for grid based.\n");
		    clean_up(ERROR);
	        }
		j++;
	    }
	    else
	    {
	        /*1 fluid crx */
	        crx0 = crx;
		if(l==1)
		{
		    printf("ERROR in insert_curve_crossings ");
		    printf("2 fluid crossings in one face, impossible for grid based.\n");
		    
		    print_int_vector("icrds=", icrds, 3, "\n");
		    printf("dir = %d\n", dir);
		    
		    print_curve_face_crossings(intfc, icrds, dir);
		    clean_up(ERROR);
		}
		l++;
	    }
	}
	
	/*3 wall crxs, must be wall fixed curve case, shoud be inserted before this part */
	if(j == 3)
	{
	    printf("ERROR in insert_curve_crossings, fixed wall crx happens");
	    printf(" wall crossings=%d fluid crossings=%d in one face. \n", j, l);
	    clean_up(ERROR);
	}
	/*must be 2 wall crxs, 1 fluid crx case, possible moving curve case */
	if(j != 2 && l != 1)
	{
	    printf("ERROR in insert_curve_crossings, impossible crx ");
	    printf("wall crossings=%d fluid crossings=%d in one face. \n", j, l);
	    clean_up(ERROR);
	}

	/*(3) insert curve crx, ref: add_to_crx_list */
	k = face_index3d(ix,iy,iz,dir,gmax);
	face_list = T->curve_crx_lists[k];
	crx_list = T->curve_crx_store + *index;

	T->curve_crx_type[k]++;

	/*only one curve crx for grid based case */
	*nc = 0;

	if(debugging("line_tri"))
	    found = tst_line_tri_crossing_on_face(crds_crx,&tri,&surf,
	        pt1,pt2,icrds,intfc,tol);

	found = line_tri_crossing_on_face(crds_crx,&tri,&surf,
	    pt1,pt2,icrds,intfc,tol);
	
	/*printf("#found %d\n", found); */
	if(found)
	{
	    /*pt position */
	    newp = crx_list[*nc].pt = Point(crds_crx);
	    intfc->modified = NO;
	    Index_of_point(newp) = k;
	    
	    /*printf("#kcrx0  %d  %d\n", *index, crx_list); */
	    /*print_general_vector("#curve crds0= ", crds_crx, 3, "\n"); */
	    
	    interpolate_crx_pt_states_on_tri(intfc,newp,tri,surf);
	    crx_list[*nc].hsb = NULL;
	    crx_list[*nc].tri = tri;
	    /*physical surf */
	    crx_list[*nc].lcomp = negative_component(surf);
	    crx_list[*nc].ucomp = positive_component(surf);
	}
	else
	{
	    /*using the projection of the fluid crx as the curve crx */
	    /*ref: fill_missing_crx */
	    /*pt position */
	    line_point_projection(crds_crx,&iv,Coords(crx0->pt),pt1,pt2,tol);
	    newp = crx_list[*nc].pt = Point(crds_crx);
	    intfc->modified = NO;
	    
	    Index_of_point(newp) = k;

	    crx_surf = Surface_of_hs(crx0->hs);
	    c0 = negative_component(crx_surf);
	    c1 = positive_component(crx_surf);
	
	    /*pt state */
	    coords = Coords(newp);
	    intfc->modified = NO;
	    nearest_intfc_state(coords,c0,intfc,left_state(newp),NULL,&hs_on);
	    nearest_intfc_state(coords,c1,intfc,right_state(newp),NULL,&hs_on);

	    if(debugging("line_proj"))
	    {
	        print_general_vector("pt=", Coords(crx0->pt), 3, "\n");
	        print_tri(crx0->tri, intfc); 
	    }
	    /*printf("#proj crds %d  %d  %d \n", *index, c0, c1); */
	    /*print_general_vector("crds= ", crds_crx, 3, "\n"); */

	    crx_list[*nc].hsb = NULL; /*Hyper_surf(crx_surf); */
	    crx_list[*nc].tri = NULL;
	    /*physical surf */
	    crx_list[*nc].lcomp = c0;
	    crx_list[*nc].ucomp = c1;
	}

	adjust_face_crossing(Coords(newp),intfc,icrds,dir);

	if(debugging("curve_fix_crx"))
	{
	    print_wall_curve_crx("curve_fix", icrds, (int) dir, k, &crx_list[*nc]);
	    remove_from_debug("curve_fix_crx");
	}

	crx_list[*nc].crx_num = 0;
	face_list[*nc] = *index;
	++(*nc);    /*useless for grid based */
	++(*index);

	remove_from_debug("line_tri");
}


/* ref: insert_grid_intfc_crossings3d( */
LOCAL	int insert_grid_based_curve_crossings3d(
	INTERFACE	*intfc, 
	int		*smin,
	int		*smax)
{
	RECT_GRID	*rgr = &topological_grid(intfc);
	GRID_DIRECTION  dir[3] = {EAST,NORTH,UPPER};
	struct Table	*T = table_of_interface(intfc);
	int		*curve_crx_count = T->curve_crx_count;
	int		*gmax = rgr->gmax;
	int		ix,iy,iz, i,k, ip[3];
	int             crx_index=0;

        DEBUG_ENTER(insert_grid_based_curve_crossings3d)
	
	if (intfc->surfaces == NULL  ||
	   is_outside_surfaces(intfc, rgr)) 
		return GOOD_STEP;

	insert_fixed_curve_crossings(intfc,smin,smax, &crx_index);

	for (iz = smin[2]; iz <= smax[2]; ++iz)
	{
	    ip[2] = iz;
	    for (iy = smin[1]; iy <= smax[1]; ++iy)
	    {
	        ip[1] = iy;
	        for (ix = smin[0]; ix <= smax[0]; ++ix)
	        {
		    ip[0] = ix;
	            for (i = 0; i < 3; ++i)
	            {
	                if (ix == smax[0] && (dir[i] == NORTH || dir[i] == UPPER))
	                    continue;
	                if (iy == smax[1] && (dir[i] == EAST || dir[i] == UPPER))
	                    continue;
	                if (iz == smax[2] && (dir[i] == EAST || dir[i] == NORTH))
	                    continue;
			
	                /*Assume grid based crx */
			k = face_index3d(ix,iy,iz,dir[i],gmax);
			if(curve_crx_count[k] == 0)
			    continue;
			/*fixed curve crx excluded. */
			if(T->curve_crx_type[k] > 0)
			    continue;

			insert_curve_face_crossings(intfc,ip,dir[i],&crx_index);
		    }
		}
	    }
	}

	DEBUG_LEAVE(insert_grid_based_curve_crossings3d)
	return GOOD_STEP;
}

EXPORT	int check_wall_crx_orient(
	INTERFACE	*intfc, 
	int		*smin,
	int		*smax)
{
	RECT_GRID	*rgr = &topological_grid(intfc);
	GRID_DIRECTION  dir[3] = {EAST,NORTH,UPPER};
	struct Table	*T = table_of_interface(intfc);
	COMPONENT       *comp = T->components;
	CRXING		*crx, *seg_crx[4];
	int		*gmax = rgr->gmax;
	int		ix,iy,iz, i,j,k, ind, ie[4], iv[4], fcrx;
	int		ref_orient, orient, ort, pc, nc;
	SURFACE		*surf, *ref_surf;
	CURVE		*curve;

	if (intfc->surfaces == NULL) return GOOD_STEP;
	
        DEBUG_ENTER(check_wall_crx_orient)
	
	for (iz = smin[2]; iz <= smax[2]; ++iz)
	{
	    for (iy = smin[1]; iy <= smax[1]; ++iy)
	    {
	        for (ix = smin[0]; ix <= smax[0]; ++ix)
	        {
	            for (i = 0; i < 3; ++i)
	            {
	                if (ix == smax[0] && (dir[i] == NORTH || dir[i] == UPPER))
	                    continue;
	                if (iy == smax[1] && (dir[i] == EAST || dir[i] == UPPER))
	                    continue;
	                if (iz == smax[2] && (dir[i] == EAST || dir[i] == NORTH))
	                    continue;
			
			k = face_index3d(ix,iy,iz,dir[i],gmax);
			if(T->curve_crx_count[k] == 0)
			    continue;
			
			ind = T->curve_crx_lists[k][0];
			crx = &T->curve_crx_store[ind];
			curve = Curve_of_hsb(crx->hsb);

			edge_index_of_face(ie,ix,iy,iz,dir[i],gmax);
			vertex_index_of_face(iv,ix,iy,iz,dir[i],gmax);
	
			/*finding all crxs throught 4 edges(seg_crx) and a reference surf(fcrx) */
			/*if there is a fluid crx, use the fluid crx as the reference surf. */
			fcrx = -1;
			for(k=0; k<4; k++)
			    if(T->seg_crx_count[ie[k]] != 0)
			    {
			        ind = T->seg_crx_lists[ie[k]][0];
			        seg_crx[k] = &T->crx_store[ind];
				if(!is_wall_surface(Surface_of_hs(seg_crx[k]->hs)))
				    fcrx = k;
			    }
			    else
			        seg_crx[k] = NULL;
			if(fcrx == -1)
			{
			    for(k=0; k<4; k++)
			        if(seg_crx[k] != NULL)
				    fcrx = k;
			}

			ref_surf = Surface_of_hs(seg_crx[fcrx]->hs);
			ref_orient = curve_surface_orientation(ref_surf, curve);
			pc = positive_component(ref_surf);
			nc = negative_component(ref_surf);

			printf("\n");
			printf("   %2d---%c---%2d\n", comp[iv[3]], seg_crx[1]==NULL? '-' : '1', comp[iv[2]]);
			printf("    |        |\n");
			printf("    %c        %c\n", seg_crx[2]==NULL? '|' : '2', seg_crx[3]==NULL? '|' : '3');
			printf("    |        |\n");
			printf("   %2d---%c---%2d\n", comp[iv[0]], seg_crx[0]==NULL? '-' : '0', comp[iv[1]]);

			/*printf("ref  %d  %d  %d  %d\n", ref_surf, ref_orient, pc, nc); */
			for(k=0; k<4; k++)
			    if(T->seg_crx_count[ie[k]] != 0)
			    {
			        surf = Surface_of_hs(seg_crx[k]->hs);
			
				/*printf("surf  %d  %d  %d\n", surf, positive_component(surf), negative_component(surf)); */
				/*determine the orientation of curve surface pair FROM the ref_surf. */
				orient = ORIENTATION_NOT_SET;
				if(surf == ref_surf)
				    orient = ref_orient;
				else
				{
				    if(ref_orient == POSITIVE_ORIENTATION)
				    {
				        if(positive_component(surf) == pc)
	    			            orient = NEGATIVE_ORIENTATION;
				        else  if(positive_component(surf) == nc)
	    			            orient = POSITIVE_ORIENTATION;
					else  if(negative_component(surf) == nc)
	    			            orient = NEGATIVE_ORIENTATION;
				        else  if(negative_component(surf) == pc)
	    			            orient = POSITIVE_ORIENTATION;
				    }
				    else
				    {
				        if(positive_component(surf) == pc)
	    			            orient = POSITIVE_ORIENTATION;
				        else  if(positive_component(surf) == nc)
	    			            orient = NEGATIVE_ORIENTATION;
					else  if(negative_component(surf) == nc)
	    			            orient = POSITIVE_ORIENTATION;
				        else  if(negative_component(surf) == pc)
	    			            orient = NEGATIVE_ORIENTATION;
				    }
				}
				
				if(orient == ORIENTATION_NOT_SET)
				{
				    printf("ERROR check_wall_crx_orient, orient is not set.\n");
				    clean_up(ERROR);
				}

				ort = curve_surface_orientation(surf, curve);
			        printf("%2d %p | %2d | %2d %2d | %2d %2d | %2d %2d |%10s\n",k, surf, is_wall_surface(surf),
				      negative_component(surf),positive_component(surf),
				      seg_crx[k]->lcomp, seg_crx[k]->ucomp, 
				      orient, ort, orient == ort? " " : "o_error");
			    }
		    }
		}
	    }
	}

	return GOOD_STEP;

        DEBUG_LEAVE(check_wall_crx_orient)
}


#define  MAX_3_COMP_CURV  10

LOCAL	int make_wall_curves(
	INTERFACE	*intfc, 
	int		*smin,
	int		*smax)
{
	RECT_GRID	*rgr = &topological_grid(intfc);
	GRID_DIRECTION  dir[3] = {EAST,NORTH,UPPER};
	struct Table	*T = table_of_interface(intfc);
	NODE		*ns, *ne;
	CRXING		*crx, *crx_tmp;
	SURFACE		*crx_surf[4];
	int		*gmax = rgr->gmax;
	int		ix,iy,iz, i,j,k, ind, ie[4];
	static O_SURFACE    **sarr=NULL;
	CURVE		*newc[MAX_3_COMP_CURV];
	FT_POINT		*pt[MAX_3_COMP_CURV][2];
	int             nsurf=0, scnt[MAX_3_COMP_CURV];
	

        DEBUG_ENTER(make_wall_curves)
	
	if(sarr == NULL)
	    bi_array(&sarr, MAX_3_COMP_CURV, 3, sizeof(O_SURFACE));
	
	if (intfc->surfaces == NULL  ||  
	    is_outside_surfaces(intfc, rgr)) 
		return GOOD_STEP;
	
	for (iz = smin[2]; iz <= smax[2]; ++iz)
	{
	    for (iy = smin[1]; iy <= smax[1]; ++iy)
	    {
	        for (ix = smin[0]; ix <= smax[0]; ++ix)
	        {
	            for (i = 0; i < 3; ++i)
	            {
	                if (ix == smax[0] && (dir[i] == NORTH || dir[i] == UPPER))
	                    continue;
	                if (iy == smax[1] && (dir[i] == EAST || dir[i] == UPPER))
	                    continue;
	                if (iz == smax[2] && (dir[i] == EAST || dir[i] == NORTH))
	                    continue;
			
	                /*Assume grid based crx */
			k = face_index3d(ix,iy,iz,dir[i],gmax);
			if(T->curve_crx_count[k] == 0)
			    continue;
			ind = T->curve_crx_lists[k][0];
			crx = &T->curve_crx_store[ind];
			/*exclude the fixed 3 comp curves */
			if(crx->hsb != NULL)
			    continue;
			
			edge_index_of_face(ie,ix,iy,iz,dir[i],gmax);
			
			j = 0;
			for(k=0; k<4; k++)
			    if(T->seg_crx_count[ie[k]] != 0)
			    {
			        ind = T->seg_crx_lists[ie[k]][0];
			        crx_tmp = &T->crx_store[ind];
	    			crx_surf[j] = Surface_of_hs(crx_tmp->hs);
				j++;
			    }
			if(j != 3)
			{
			    printf("ERROR make_wall_curves, crx #=%d for curve crx face.\n",j);
			    clean_up(ERROR);
			}
			k = add_to_o_surfaces(sarr,&nsurf,crx_surf);
		
			/*use crx_num to store the cruve index  */
			if(k == -1)
			{ /*new 3 comp curve found */
			    scnt[nsurf-1] = 1;
			    pt[nsurf-1][0] = crx->pt;   /*1st node posn */
			    crx->crx_num = nsurf-1;
			}
			else  if(scnt[k] == 1)
			{ /*already in the array */
			    scnt[k] = 2;
			    pt[k][1] = crx->pt;         /*2ed node posn */
		            crx->crx_num = k;
			}
			else
			    crx->crx_num = k;
		
		    }
		}
	    }
	}
	
	for(i=0; i<nsurf; i++)
	{    
	    if(scnt[i] !=2 )
	    {
	        printf("ERROR make_wall_curves, only one curve crx, impossible.\n");
	        clean_up(ERROR);
	    }
	    /*printf("#wall curve made.\n"); */
	    
	    ns = make_node(pt[i][0]);
	    ne = make_node(pt[i][1]);
	    /*make the wall curve  ASSUME  the curve is NEUMANN_CURVE */
	    newc[i] = make_curve(NO_COMP, NO_COMP, ns, ne);
	    curve_type(newc[i]) = NEUMANN_CURVE;
	    newc[i]->last = NULL;
	    newc[i]->first = NULL;
	    for(j=0; j<3; j++)
	    {
	        /*printf("#surf %d(%d %d) %d  orient %d\n", sarr[i][j].surface,  */
		/*    positive_component(sarr[i][j].surface), negative_component(sarr[i][j].surface), */
		/*    is_wall_surface(sarr[i][j].surface), sarr[i][j].orient); */
	        install_curve_in_surface_bdry(sarr[i][j].surface, newc[i],
					      sarr[i][j].orient);
	    }
	}

	/*curve has no tris  */
	intfc->modified = NO;

	/*set curve pointer for wall curve crx  */
	for (iz = smin[2]; iz <= smax[2]; ++iz)
	{
	    for (iy = smin[1]; iy <= smax[1]; ++iy)
	    {
	        for (ix = smin[0]; ix <= smax[0]; ++ix)
	        {
	            for (i = 0; i < 3; ++i)
	            {
	                if (ix == smax[0] && (dir[i] == NORTH || dir[i] == UPPER))
	                    continue;
	                if (iy == smax[1] && (dir[i] == EAST || dir[i] == UPPER))
	                    continue;
	                if (iz == smax[2] && (dir[i] == EAST || dir[i] == NORTH))
	                    continue;
			
	                /*Assume grid based crx */
			k = face_index3d(ix,iy,iz,dir[i],gmax);
			if(T->curve_crx_count[k] == 0)
			    continue;

			ind = T->curve_crx_lists[k][0];
			crx = &T->curve_crx_store[ind];
			if(crx->hsb == NULL)
			{
			    /*printf("#crx->hs %d %d %d %d %d %d\n", ix,iy,iz, dir[i], */
			    /*    crx->crx_num, Hyper_surf(newc[crx->crx_num])); */
			    crx->hsb = Hyper_surf_bdry(newc[crx->crx_num]);
		        }
		    }
		}
	    }
	}

        DEBUG_LEAVE(make_wall_curves)
	
	return GOOD_STEP;
}

LOCAL	int count_grid_curve_crossings3d(
	INTERFACE	*grid_intfc, 
	int		*smin,
	int		*smax)
{
	RECT_GRID	*rgr = &topological_grid(grid_intfc);
	GRID_DIRECTION  dir[3] = {EAST,NORTH,UPPER};
	struct Table	*T = table_of_interface(grid_intfc);
	COMPONENT       *comp = T->components;
	int		*curve_crx_count = T->curve_crx_count;
	int		*gmax = rgr->gmax;
	int		ix,iy,iz, i,j,k, c[4],d[4];
	int             n_curve_crx=0;

	if (grid_intfc->surfaces == NULL || 
	    is_outside_surfaces(grid_intfc, rgr)) 
		return 0;

	for (iz = smin[2]; iz <= smax[2]; ++iz)
	{
	    for (iy = smin[1]; iy <= smax[1]; ++iy)
	    {
	        for (ix = smin[0]; ix <= smax[0]; ++ix)
	        {
	            for (i = 0; i < 3; ++i)
	            {
	                if (ix == smax[0] && (dir[i] == NORTH || dir[i] == UPPER))
	                    continue;
	                if (iy == smax[1] && (dir[i] == EAST || dir[i] == UPPER))
	                    continue;
	                if (iz == smax[2] && (dir[i] == EAST || dir[i] == NORTH))
	                    continue;
			
			vertex_index_of_face(d,ix,iy,iz,dir[i],gmax);
			for(j=0; j<4; j++)
			    c[j] = comp[d[j]];
		
	                /*Assume grid based crx */
			k = face_index3d(ix,iy,iz,dir[i],gmax);
	
			if(is_curve_crx(c[0],c[3],c[1],c[2]))
			{
			    n_curve_crx++;
			    curve_crx_count[k] = 1;
			}
			else
			    curve_crx_count[k] = 0;
		    }
		}
	    }
	}

	return n_curve_crx;
}

LOCAL	void free_curve_crx_storage(
        INTERFACE *intfc)
{
        Table *T = table_of_interface(intfc);

	free_these(5,T->curve_crx_count,T->curve_crx_type,T->curve_crx_lists,
	             T->curve_crx_lists_store, T->curve_crx_store);

}       /* end free_crx_storage */

/*WARNING should be called after the component is set. */
LOCAL	void set_curve_crx_storage(
	INTERFACE *intfc, 
	int	  *smin,
	int	  *smax)
{
	RECT_GRID *gr = &topological_grid(intfc);
	int *gmax = gr->gmax;
	int dim = gr->dim;
	Table *T = table_of_interface(intfc);
	int i, n_curve_crx, n_faces;

	n_faces = 0;
	switch (dim)
	{
	case 2:
	    break;
	case 3:
	    for (i = 0; i < dim; ++i)
	    {
	    	n_faces += gmax[i]*gmax[(i+1)%3]*(gmax[(i+2)%3] + 1);
	    }
	    break;
	}

	uni_array(&T->curve_crx_count,n_faces,INT);
	uni_array(&T->curve_crx_type,n_faces,INT);
	
	for (i = 0; i < n_faces; ++i)
	{
	    T->curve_crx_type[i] = 0;
	    T->curve_crx_count[i] = 0;
	}

	n_curve_crx = count_grid_curve_crossings3d(intfc, smin, smax);
        T->n_curve_crx = n_curve_crx;

	/*printf("#n_curve_crx   %d\n",  n_curve_crx); */

	init_face_crx_lists(intfc,n_curve_crx,n_faces);

}	/*end set_crx_storage_for_reconstruction*/


LOCAL	boolean	make_wall_surfaces(
	INTERFACE *intfc,
	int       *smin,
	int       *smax)
{
	INTERFACE  *sav_intfc = current_interface();
	int        sav_siz_st = get_size_of_intfc_state();
	boolean	   sav_copy_st = copy_intfc_states();
	GRID_DIRECTION   dir[3] = {EAST,NORTH,UPPER};
	RECT_GRID  *gr = &topological_grid(intfc);
	Table	   *T = table_of_interface(intfc);
	COMPONENT  *comp = T->components;
	int	   *ef = T->edge_flag;
	SURFACE	   *s, *ws, **ss;
	int	   *gmax = gr->gmax;
	int	   ea[50][3],nea,c1,c2;
	int	   i,j,k,ix,iy,iz,ip[3],ipn[3];
	int        comp1, comp2;
	boolean	   found;

	DEBUG_ENTER(make_wall_surfaces);

	/*#bjet ASSUME */
	get_default_fluid_comp(&comp1,&comp2,intfc);
	
	if(!use_wall_edge())
	{
	    DEBUG_LEAVE(make_wall_surfaces);
	    return NO;
	}

	set_current_interface(intfc);
	
	/*ea[j]  the jth wall crxing pair */
	/*assume ea[j][0] is the fluid comp  */
	/*       ea[j][1] is the wall comp */
	/*nea is the number of different wall crxs */
	/*eg:   ea[0][0] = 2 ea[0][1]=1  ea[0][2]=1   */
	/*         f(2)----x----w(1)    surface exists */
	/*      ea[1][0] = 3 ea[1][1]=1  ea[1][2]=0   */
	/*         f(3)----x----w(1)    no such surface */
	/*	nea = 2 */
	
	nea = 0;
	for (iz = smin[2]; iz <= smax[2]; ++iz)
	{
	    ip[2] = iz;
	    for (iy = smin[1]; iy <= smax[1]; ++iy)
	    {
	        ip[1] = iy;
	        for (ix = smin[0]; ix <= smax[0]; ++ix)
	        {
	            ip[0] = ix;
	            for (i = 0; i < 3; ++i)
	            {
	                if (ix == smax[0] && dir[i] == EAST)
	                    continue;
	                if (iy == smax[1] && dir[i] == NORTH)
	                    continue;
	                if (iz == smax[2] && dir[i] == UPPER)
	                    continue;
	                k = seg_index3d(ix,iy,iz,dir[i],gmax);
			if(ef[k] != ON_WALL)
			    continue;
			
			c1 = comp[d_index3d(ix,iy,iz,gmax)];
			next_ip_in_dir(ip,dir[i],ipn,smin,smax);
			c2 = comp[d_index3d(ipn[0],ipn[1],ipn[2],gmax)];
			for(j=0; j<nea; j++)
			    if((ea[j][0] == c1 && ea[j][1] == c2) ||
			       (ea[j][0] == c2 && ea[j][1] == c1))
			        break;
			if(j == nea)
			{
			    ea[j][2] = 0;
			    /*use assumption ea[j][0] is in fluid */
			    if(c1 == comp1 || c1 == comp2)
			    {	
			        ea[j][0] = c1;
				ea[j][1] = c2;
			    }
			    else
			    {
			        ea[j][0] = c2;
				ea[j][1] = c1;
			    }
			    nea++;
			}
	            }
		}
	    }
	}

	/*printf("#nea=%d\n", nea); */
	for(i=0; i<nea; i++)
	{
	    s = find_surf_with_comp(intfc, ea[i][0], ea[i][1]);
	    /*see if the wall surface with comp ea[i][0] and ea[i][1]  */
	    /*already exists */
	    if(s != NULL && is_wall_surface(s))
	        ea[i][2] = 1;
	}
	
	for(i=0; i<nea; i++)
	    if(ea[i][2] == 0)
	    {
	        printf("#wall surf  new wall surface made %d  %d\n", ea[i][1], ea[i][0]);
	        ws = make_surface(ea[i][1], ea[i][0], NULL, NULL);
		
		/*ASSUME   */
		found = NO;
		for(ss=intfc->surfaces; ss && *ss; ss++)
		{
		    if(negative_component(*ss) == ea[i][1] && 
		      (positive_component(*ss) == comp1 || positive_component(*ss) == comp2) )
		    {
			/*WARN correct only for NEUMANN and DIRICHLET FT. */
		        found = YES;
			wave_type(ws) = wave_type(*ss);
                        bstate_index(ws) = bstate_index(*ss);
			printf("#type %d  index %d\n", wave_type(ws), bstate_index(ws));
		    }
		}

		if(!found)	
		{
		    printf("ERROR make_wall_surfaces, can not find paired wall surface\n");
		    clean_up(ERROR);
		}

		first_tri(ws) = last_tri(ws) = NULL;
		ws->num_tri = 0;
		set_is_wall_surface(ws);
	    }

	/*the new surf has no tris */
	intfc->modified = NO;

	set_size_of_intfc_state(sav_siz_st);
	set_copy_intfc_states(sav_copy_st);
	set_current_interface(sav_intfc);
	
	DEBUG_LEAVE(make_wall_surfaces);
	return YES;
}

LOCAL	void  set_wall_flag_storage(INTERFACE *intfc)
{
	RECT_GRID gr = topological_grid(intfc);
	int 	  n_segs, *gmax = gr.gmax;
	Table	  *T = table_of_interface(intfc);

	n_segs = gmax[0]*(gmax[1]+1)*(gmax[2]+1)
	      + gmax[1]*(gmax[2]+1)*(gmax[0]+1)
	      + gmax[2]*(gmax[0]+1)*(gmax[1]+1);

	uni_array(&T->edge_flag,n_segs,INT);
}

LOCAL	void  free_wall_flag_storage(INTERFACE *intfc)
{
	RECT_GRID gr = topological_grid(intfc);
	Table	  *T = table_of_interface(intfc);

	free(T->edge_flag);
}


LOCAL   boolean fix_crx_on_wall(
	CRXING     *crx,   /*This is a wall crx */
	INTERFACE  *intfc,
	int	   c0, 
	int        c1)
{
	FT_POINT 		*p;
	SURFACE		*s;
	HYPER_SURF	*hs_on;
	Locstate	sl, sr;
	double		*coords;
	int		cn;

	if(crx->lcomp == c0 && crx->ucomp == c1)
	    return YES;
			    
	s = find_surf_with_comp(intfc, c0, c1);
	if(s == NULL)
	{
	    printf("ERROR in fix_crx_on_wall: ");
	    printf("no wall surface with comp %d  %d\n",c0, c1);
	    clean_up(ERROR);
	}

	cn = 0;
	if(crx->lcomp != c0)
	{
	    crx->lcomp = c0;
	    crx->hs = Hyper_surf(s);
	    cn++;
	}
	if(crx->ucomp != c1)
	{
	    crx->ucomp = c1;
	    crx->hs = Hyper_surf(s);
	    cn++;
	}
	if(cn == 2)
	{
	    printf("ERROR in fix_crx_on_wall: ");
	    printf("wall inconsistent %d  %d\n",c0, c1);
	    clean_up(ERROR);
	}
	
	coords = Coords(crx->pt);
	/*the p above can be a point on new surf, since the following will change the states */
	/*on point, a new point should be made. */
	p = crx->pt = Point(coords);
	sl = left_state(p);
	sr = right_state(p);

	if(negative_component(s) == c0)
	{
	    intfc->modified = NO;
	    nearest_intfc_state(coords,c0,intfc,sl,NULL,&hs_on);
	    nearest_intfc_state(coords,c1,intfc,sr,NULL,&hs_on);
	}
	else
	{
	    intfc->modified = NO;
	    nearest_intfc_state(coords,c0,intfc,sr,NULL,&hs_on);
	    nearest_intfc_state(coords,c1,intfc,sl,NULL,&hs_on);
	}

	/*remove_from_debug("line_tri"); */
 
	return YES;
}

LOCAL	boolean set_wall_crossings3d(
	INTERFACE *intfc,
	int *smin,
	int *smax)
{
	INTERFACE      *sav_intfc = current_interface();
	SURFACE        **s, *crx_surf;
	CRXING         *crx;
	GRID_DIRECTION dir[3] = {EAST,NORTH,UPPER};
	RECT_GRID      gr = topological_grid(intfc);
	Table	       *T = table_of_interface(intfc);
	COMPONENT      *comp = T->components;
	int	       *ef = T->edge_flag;
	int 	       *gmax = gr.gmax;
	int 	       ix,iy,iz,i,j,k, ip[3], ipn[3];
	int	       nc, list, c0,c1;

	if(!use_wall_edge())
	    return YES;
	
	set_current_interface(intfc);
	
	/*(*s)->number == 1 is wall surface. Check the comps of wall surfaces */
	/*for(s=intfc->surfaces; s && *s; s++) */
	/*    printf("#snum=%d %d %d %d\n", (*s)->number,  */
	/*           positive_component(*s),negative_component(*s), *s); */
	
	for (iz = smin[2]; iz <= smax[2]; ++iz)
	{
	    ip[2] = iz;
	    for (iy = smin[1]; iy <= smax[1]; ++iy)
	    {
	        ip[1] = iy;
	        for (ix = smin[0]; ix <= smax[0]; ++ix)
	        {
	            ip[0] = ix;
	            for (i = 0; i < 3; ++i)
	            {
	                if (ix == smax[0] && dir[i] == EAST)
	                    continue;
	                if (iy == smax[1] && dir[i] == NORTH)
	                    continue;
	                if (iz == smax[2] && dir[i] == UPPER)
	                    continue;
	                k = seg_index3d(ix,iy,iz,dir[i],gmax);
		
			/*(1)should be no crx  */
			if(ef[k] == OUTSIDE_WALL)
			{
			    T->seg_crx_count[k] = 0;
			    continue;
			}

			/*(2)if more than one crx or no crx, should be fixed for grid based case. */
			if(ef[k] == INSIDE_WALL)
			{
			    nc = T->seg_crx_count[k];
			    /*only one crx, no problem because remove_... already check it. */
			    if(nc == 0 || nc == 1)
			        continue;
			    
			    next_ip_in_dir(ip,dir[i],ipn,smin,smax);
			    c0 = comp[d_index3d(ip[0],ip[1],ip[2],gmax)];
			    c1 = comp[d_index3d(ipn[0],ipn[1],ipn[2],gmax)];
	                    
			    if(c0 == c1)
			    {
			        T->seg_crx_count[k] = 0;
				if(debugging("fluid_fix_crx"))
				{
				    printf("#fluid_fix %d   %d  %d\n", nc, c0, c1);
				    print_int_vector("ip", ip, 3, "\n");
				}
				continue;
			    }
			    
			    /*find one suitable crx on the edge  */
			    for(j=0; j<nc; j++)
			    {
			        list = T->seg_crx_lists[k][j];
	                        crx = &(T->crx_store[list]);
				/*ASSUME only one surface in the INSIDE_WALL edge. */
				if(crx->lcomp == c0 && crx->ucomp == c1)
				{
				    T->seg_crx_count[k] = 1;
				    T->seg_crx_lists[k][0] = list;
				    break;
				}
			    }
			    if(j == nc)
			    {
			        printf("ERROR in set_wall_crossings3d ");
			        printf("INSIDE_WALL edge have no suitable crossing %d  %d.\n",c0,c1);
				print_int_vector("ip=", ip, 3, "\n");
				printf("dir = %d\n", dir[i]);
			        clean_up(ERROR);
			    }
			    if(debugging("fluid_fix_crx"))
			    {
	    			print_wall_crx("fluid_fix", ip,dir[i],k,crx);
			    }
			    continue;
			}
			if(ef[k] != ON_WALL)
			{
			    printf("ERROR set_wall_crossings3d, edge_flag is not set.\n");
			    clean_up(ERROR);
			}
	               
		        /*(3) the crx for this case must be the wall crx. */
			/*printf("#on wall %d %d %d\n", ix, iy, iz); */
			nc = T->seg_crx_count[k];
			if (nc != 0)
			{
			    next_ip_in_dir(ip,dir[i],ipn,smin,smax);
			    c0 = comp[d_index3d(ip[0],ip[1],ip[2],gmax)];
			    c1 = comp[d_index3d(ipn[0],ipn[1],ipn[2],gmax)];
	                    
			    /*find the wall crx on the edge  */
			    for(j=0; j<nc; j++)
			    {
			        list = T->seg_crx_lists[k][j];
	                        crx = &(T->crx_store[list]);
			        crx_surf = Surface_of_hs(crx->hs);
				if(is_wall_surface(crx_surf))
				    break;
			    }
			    if(j == nc)
			    {
			        printf("ERROR in set_wall_crossings3d ");
			        printf("ON_WALL edge have no wall crossing %d  %d.\n",c0,c1);
				print_int_vector("ip=", ip, 3, "\n");
				printf("dir = %d\n", dir[i]);
			        clean_up(ERROR);
			    }
			    
			    /*set the crx on the wall is wall crx */
			    T->seg_crx_lists[k][0] = T->seg_crx_lists[k][j];
			    T->seg_crx_count[k] = 1;
			    
			    /*fix the comp and state of the wall crx  */
			    /*printf("#fix crx\n"); */
			    fix_crx_on_wall(crx,intfc,c0,c1);
			    
			    if(debugging("wall_fix_crx"))
			    {
	    			print_wall_crx("wall_fix", ip,dir[i],k,crx);
	    			remove_from_debug("wall_fix_crx");
			    }
			}
			else  
			{
			    /*nc == 0 for wall edge, it is impossible because there is */
			    /*a wall crx on the edge. */
			    printf("ERROR in set_wall_crossings3d ");
			    printf("ON_WALL edge have no crossing.\n");
			    clean_up(ERROR);
			}
	            }
	        }
	    }
	}

	set_current_interface(sav_intfc);

	return YES;
}

LOCAL	void set_edge_flag_for_wall(
	INTERFACE *intfc, 
	INTERFACE *wall_intfc,
	int	  cc,
	int       *smin,
	int       *smax)
{
	RECT_GRID      *gr = &topological_grid(intfc);
	int 	       ix,iy,iz;
	GRID_DIRECTION dir[3] = {EAST,NORTH,UPPER};
	int 	       ip[3],ipn[3],i,k,c1,c2,d;
	Table	       *T = table_of_interface(intfc);
	COMPONENT      *comp = T->components;
	Table	       *Tw = table_of_interface(wall_intfc);
	COMPONENT      *compw = Tw->components, comp1, comp2;
	int	       *gmax = gr->gmax;

	/*print_int_vector("#smin", smin, 3, "\n"); */
	/*print_int_vector("#smax", smax, 3, "\n"); */
	
	get_default_fluid_comp(&comp1,&comp2,intfc);
	
	for (iz = smin[2]; iz <= smax[2]; ++iz)
	{
	    ip[2] = iz;
	    for (iy = smin[1]; iy <= smax[1]; ++iy)
	    {
	        ip[1] = iy;
	        for (ix = smin[0]; ix <= smax[0]; ++ix)
	        {
	            ip[0] = ix;
		    d = d_index3d(ip[0],ip[1],ip[2],gmax);

		    /*WARN still has problems */
		    if(wall_intfc->surfaces != NULL)
		    {
		        if(compw[d] == cc)
		            comp[d] = NO_COMP;
		        else
		            comp[d] = compw[d];
		    }
		    else
			comp[d] = NO_COMP;
		    
		    for (i = 0; i < 3; ++i)
	            {
	                if (ix == smax[0] && dir[i] == EAST)
	                    continue;
	                if (iy == smax[1] && dir[i] == NORTH)
	                    continue;
	                if (iz == smax[2] && dir[i] == UPPER)
	                    continue;
			
			next_ip_in_dir(ip,dir[i],ipn,smin,smax);
			c1 = compw[d_index3d(ip[0],ip[1],ip[2],gmax)]; 
			c2 = compw[d_index3d(ipn[0],ipn[1],ipn[2],gmax)]; 
	                
			k = seg_index3d(ix,iy,iz,dir[i],gmax);
		
			if(wall_intfc->surfaces != NULL)
			{
			    if(c1 == cc && c2 == cc)
			        T->edge_flag[k] = INSIDE_WALL;  /*fluid and gas region */
			    else  if(c1 != c2)
			        T->edge_flag[k] = ON_WALL;  /*wall-fluid or wall-gas region */
			    else
			        T->edge_flag[k] = OUTSIDE_WALL; /*wall region */
			}
			else
			{
			    if(intfc->surfaces != NULL)
			        T->edge_flag[k] = INSIDE_WALL;
			    else
			        if(intfc->default_comp == comp1 || 
				   intfc->default_comp == comp2 )
			  	    T->edge_flag[k] = INSIDE_WALL;
			        else
			  	    T->edge_flag[k] = OUTSIDE_WALL;
			}
	            }
	        }
	    }
	}
}		/*end adjust_crossings */

LOCAL boolean intfc_near_point(
	int		*c, 
	double		*p,
	double   	h,
	INTERFACE  	*intfc)	    /*max box index */
{
RECT_GRID      *gr = &topological_grid(intfc);
Table	       *T = table_of_interface(intfc);
COMPONENT      ***comp = T->compon3d;
int	       *gmax = gr->gmax;
int	       ix, iy, iz, i, ip1[3], ip2[3];
double	       p0[3], tol = 1.0e-5;

	*c = NO_COMP;
	
	for(i=0; i<3; i++)
	    p0[i] = p[i] - h + tol*h;
	rect_in_which(p0, ip1, gr);

	for(i=0; i<3; i++)
	    p0[i] = p[i] + h - tol*h;
	rect_in_which(p0, ip2, gr);

	for(ix=ip1[0]; ix <= ip2[0]; ix++)
	{
	    if(ix < 0 || ix >= gmax[0])
	        return YES;
	    for(iy=ip1[1]; iy <= ip2[1]; iy++)
	    {
	        if(iy < 0 || iy >= gmax[1])
	            return YES;
		for(iz=ip1[2]; iz <= ip2[2]; iz++)
		{
		    if(iz < 0 || iz >= gmax[2])
		        return YES;
		    
		    if(comp[iz][iy][ix] == ONFRONT || 
		       comp[iz][iy][ix] == ON_RECT_BOUNDARY)
		        return YES;
		}
	    }
	}

	/*printf("#blk  %d  %d  %d\n",ip2[0]-ip1[0]+1,ip2[1]-ip1[1]+1,  */
	/*    ip2[2]-ip1[2]+1); */

	*c = comp[ip1[2]][ip1[1]][ip1[0]];
	
	return NO;
}

LOCAL	boolean	fill_comp_from_prev_intfc(
	INTERFACE *intfc,
	int       *smin,
	int       *smax)
{
INTERFACE      *prev_intfc;
RECT_GRID      *gr = &topological_grid(intfc);
Table	       *T = table_of_interface(intfc);
COMPONENT      *comp = T->components;
int	       *gmax = gr->gmax;
int 	       cc, d, ix,iy,iz, ip[3];
double	       h = min3(gr->h[0], gr->h[1], gr->h[2]), pt[3];


	if(prev_interface(intfc) == NULL)
	    return NO;

	/*interface in previous time step */
	prev_intfc = prev_interface(intfc);
	
	/*ix, iy, iz are node indices */
	for (iz = smin[2]; iz <= smax[2]; ++iz)
	{
	    pt[2] = cell_edge(iz, 2, gr);
	    for (iy = smin[1]; iy <= smax[1]; ++iy)
	    {
	        pt[1] = cell_edge(iy, 1, gr);
	        for (ix = smin[0]; ix <= smax[0]; ++ix)
	        {
	            pt[0] = cell_edge(ix, 0, gr);
		    
		    d = d_index3d(ix, iy, iz, gmax);

		    if( (comp[d] != NO_COMP) || 
		        intfc_near_point(&cc, pt, h, prev_intfc) )
		        continue;
		    comp[d] = cc;
		}
	    }
	}

	return YES;
}

LOCAL void change_3d_intfc_states(
	INTERFACE	*intfc,
	int		cc,
	Locstate	st,
	int		sizest)
{
	FT_POINT		*p;
	CURVE		**c;
	HYPER_SURF_ELEMENT *hse;
	HYPER_SURF	*hs;
	Locstate	sl, sr;
	BOND_TRI 	**btris;
	BOND 		*bond;

	debug_print("change st","Entered change_3d_front_states()\n");
	if (sizest == 0 || st == NULL)
	{
	    debug_print("change st","Left change_3d_front_states()\n");
	    return;
	}

	for (c = intfc->curves; c && *c; ++c)
	{
	    for (bond = (*c)->first; bond; bond = bond->next)
	    {
	        for(btris = Btris(bond); btris && *btris; btris++)
		{
		    hs = Hyper_surf((*btris)->surface);
		    
		    sl = left_start_btri_state(*btris);
		    sr = right_start_btri_state(*btris);
		    if(positive_component(hs) == cc)
		        ft_assign(sr,st,sizest);
		    if(negative_component(hs) == cc)
		        ft_assign(sl,st,sizest);

		    sl = left_end_btri_state(*btris);
		    sr = right_end_btri_state(*btris);
		    if(positive_component(hs) == cc)
		        ft_assign(sr,st,sizest);
		    if(negative_component(hs) == cc)
		        ft_assign(sl,st,sizest);
		}
	    }
	}

	(void) next_point(intfc,NULL,NULL,NULL);
	while (next_point(intfc,&p,&hse,&hs))
	{
	    slsr(p,hse,hs,&sl,&sr); 
	    if(positive_component(hs) == cc)
	        ft_assign(sr,st,sizest);
	    if(negative_component(hs) == cc)
		ft_assign(sl,st,sizest);
	}

	debug_print("change st","Left change_3d_front_states()\n");
}		

LOCAL  void  get_wall_physical_state(
	Locstate   *st,
	INTERFACE  *intfc,
	int        comp)
{
HYPER_SURF              *hs;
HYPER_SURF_ELEMENT      *hse;
FT_POINT                   *p;
Locstate  		sl, sr;

	*st = NULL;
	(void) next_point(intfc,NULL,NULL,NULL);
        while(next_point(intfc,&p,&hse,&hs))
	{
	    slsr(p,hse,hs,&sl,&sr);
	    if(hs->pos_comp == comp)
	    {
	        *st = sr;
		break;
	    }
	    if(hs->neg_comp == comp)
	    {
	        *st = sl;
		break;
	    }
	}
	if(*st == NULL)
	{
	    printf("NO ref state: in get_wall_physical_state, no suitable state.\n");
	/*    clean_up(ERROR); */
	}
}

LOCAL	INTERFACE*   make_surfaces_on_wall(
	int	  *cc,
	Front     *front,
	boolean	  copy_st)
{
	INTERFACE  *intfc, *sav_intfc = current_interface();
	int        sav_siz_st = get_size_of_intfc_state();
	boolean	   sav_copy_st = copy_intfc_states();
	SURFACE	   **s;
	Locstate   st;
	int	   sizest = front->sizest, comp;
	int        comp1, comp2;

	DEBUG_ENTER(merge_surfaces_on_wall);

	/*#bjet ASSUME */
	get_default_fluid_comp(&comp1,&comp2,front->interf);
	intfc = copy_interface(front->interf);
	
	set_current_interface(intfc);
	
	set_size_of_intfc_state(sav_siz_st);
	set_copy_intfc_states(copy_st);

	/*assume comp1 and comp2 are the fluid surface */
	for(s=intfc->surfaces; s && *s; s++)
	    if((positive_component(*s) == comp1 && negative_component(*s) == comp2) || 
	       (positive_component(*s) == comp2 && negative_component(*s) == comp1))
	    {
	        delete_scn(*s);
		break;
	    }

	/*make sure there is a wall surface which has comp1 as one side. */
	comp = comp2;
	for(s=intfc->surfaces; s && *s; s++)
	{
	    if(positive_component(*s) == comp1 || negative_component(*s) == comp1)
	    {
	        comp = comp1;
		break;
	    }
	}
	if(comp == comp2)
	{
	    comp = comp1;
	    comp1 = comp2;
	    comp2 = comp;
	}

	/*get the reference state of comp1 */
	if(copy_st)
	{
	    get_wall_physical_state(&st, intfc, comp1);
	}
	/*Now comp1 is one side of existing wall, changing comp2 wall surfaces */
	/*to comp1, if no comp1 and comp2 surfaces, comp will not be modified. */
	for(s=intfc->surfaces; s && *s; s++)
	{
	    if(positive_component(*s) == comp2) 	    
	    {
	        positive_component(*s) = comp1;
	    }
	    if(negative_component(*s) == comp2) 	    
	    {
	        negative_component(*s) = comp1;
	    }
	}

	/*change all comp1 states to reference states */
	if(copy_st)
	{
	    change_3d_intfc_states(intfc, comp1, st, sizest);
	}

	*cc = comp1;

	set_size_of_intfc_state(sav_siz_st);
	set_copy_intfc_states(sav_copy_st);
	set_current_interface(sav_intfc);
	
	DEBUG_LEAVE(merge_surfaces_on_wall)

	return intfc;

}	/*merge surfaces on wall */


	/*fill comp for solid wall from prev interface. */
LOCAL  void fill_comp_from_prev_for_wall(
	INTERFACE *intfc,
	INTERFACE *wall_intfc,
	int	  cc,
	int       *smin,
	int       *smax)
{
	RECT_GRID      *gr = &topological_grid(wall_intfc);
	Table	       *T = table_of_interface(wall_intfc);
	COMPONENT      *comp = T->components;
	int	       *gmax = gr->gmax;
	int            c0, comp1, comp2, ip[3];
	
	for (ip[2] = smin[2]; ip[2] <= smax[2]; ++ip[2])
	    for (ip[1] = smin[1]; ip[1] <= smax[1]; ++ip[1])
	        for (ip[0] = smin[0]; ip[0] <= smax[0]; ++ip[0])
		{
	    	    comp[d_index3d(ip[0],ip[1],ip[2],gmax)] = NO_COMP;
		}

	get_default_fluid_comp(&comp1,&comp2,intfc);
	prev_interface(wall_intfc) = prev_interface(intfc);
	fill_comp_from_prev_intfc(wall_intfc, smin, smax);

	for (ip[2] = smin[2]; ip[2] <= smax[2]; ++ip[2])
	    for (ip[1] = smin[1]; ip[1] <= smax[1]; ++ip[1])
	        for (ip[0] = smin[0]; ip[0] <= smax[0]; ++ip[0])
		{
		    c0 = comp[d_index3d(ip[0],ip[1],ip[2],gmax)];
		    if(c0 == comp1 || c0 == comp2)
	    	        comp[d_index3d(ip[0],ip[1],ip[2],gmax)] = cc;
		}
}

/*
 * 4 cases
 *       wall surf      physical surf    edge_flag | fluid comp  outwall comp
 *          NO              NO          IN/OUTSIDE     prev	   prev    OK
 *          NO		    YES          INSIDE        prev         ---    OK
 *          YES		    NO		IN/OUT/ON        fill from alg.
 *          YES		    YES		IN/OUT/ON	 fill from alg.
 */


LOCAL	boolean	fill_comp_from_solid_intfc(
	Front     *front,
	int       *smin,
	int       *smax)
{
	INTERFACE  *intfc, *sav_intfc = current_interface();
	int	   comp;	/*physical comp of the wall surface */
	RECT_GRID  *gr = &topological_grid(front->interf);
	int	   *gmax = gr->gmax;

	DEBUG_ENTER(fill_comp_from_solid_intfc);

	/*comp is the wall fluid comp after delete the fluid surface */
	intfc = make_surfaces_on_wall(&comp, front, YES);
	
	set_current_interface(intfc);
	
	make_interface_topology_lists(intfc);
        set_crx_storage_for_reconstruction(intfc,NULL);
	interpolate_intfc_states(intfc) = NO;
	
	/*ASSUME the wall is grid based surfaces. So the curve crxs are on the  */
	/*faces of the blocks, It's impossible for the edge to cross the curve */
	/*crx, it means double crxs on one edge are impossible. */
	insert_grid_intfc_crossings(intfc);
	
	fill_comp_from_prev_for_wall(front->interf,intfc,comp,smin,smax);

	if(!track_comp_through_crxings3d(smin,smax,gmax,intfc,SINGLE))
	{
	    printf("WARNING: fill_comp_from_solid_intfc can not make components\n");

	    free_crx_storage(intfc);
	    set_current_interface(sav_intfc);
	    
	    delete_interface(intfc);
	    clean_up(ERROR);
	    DEBUG_LEAVE(fill_comp_from_solid_intfc)
	    
	    return NO;
	}

	set_edge_flag_for_wall(front->interf,intfc,comp,smin,smax);

        free_crx_storage(intfc);
	set_current_interface(sav_intfc);
	
	delete_interface(intfc);
	
	DEBUG_LEAVE(fill_comp_from_solid_intfc)
	return YES;
}	/*end rebuild_intfc_at_crossings3d*/

