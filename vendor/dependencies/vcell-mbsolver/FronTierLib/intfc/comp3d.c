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
*				comp3d.c:
*
*
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	This file is a three dimensional companion to comp.c.  See the latter
*	for documentation.  There are two main parts to this file.  The first
*	is the computation of the closest point on a given surface or interface
*	to a given point.  There are several versions and related routines, all
*	of which serve to answer the question, where is a point x, y, z located
*	relative to a given interface.  The second main part of this file is to
*	maintain a hashed table of triangles and surfaces, to allow the rapid
*	computations for the first part, and for intersections (is the
*	interface really non self intersecting?).
*
*	For a more detailed documentation, see the file comp.c.
*/

#if defined(THREED)

#define DEBUG_STRING    "comp3d"

#include <intfc/iloc.h>

typedef struct {
	double   pt[3];		/* Location being projected on tri */
	double   dpt[3];		/* vector pt -> closest point; */
	double   a[3];		/* Parametric coordinates of projection point */
	double   d, d2;		/* distance and square distance pt to tri */
	double   nor_d2;		/* Square of distance pt to plane of tri */
	SIDE    side;		/* side of tri upon which pt lies */
	int     side_index;	/* used when side == ONEDGE or ONVERTEX */
	int     vertex_index;	/* used when side == ONVERTEX */
	FT_POINT   *pv, *p1, *p2;	/* Vertices marking boundary closest to pt */
	TRI     *tri;		/* Triangle being projected upon */
	SURFACE *s;		/* Surface containing tri */
	int	idir;		/* used in directional projection */
} TRI_PROJECTION;

	/* LOCAL Function Declarations */
LOCAL	COMPONENT	set_comp_on_block(int*,INTERFACE*);
LOCAL	COMPONENT	comp_at_closest(TRI_PROJECTION*);
LOCAL	COMPONENT	component_wrt_icoords3d(double*,int*,INTERFACE*);
LOCAL	boolean	block_recorded(int**,int*,int*);
LOCAL	boolean	comp_is_on_surface(SURFACE*,const COMPONENT*,int);
LOCAL	boolean	new_tri_is_closer(TRI_PROJECTION*,TRI_PROJECTION*);
LOCAL	boolean	old_tri_on_vertex(TRI_PROJECTION*,TRI_PROJECTION*);
LOCAL	boolean	old_tri_on_edge(TRI_PROJECTION*,TRI_PROJECTION*);
LOCAL	boolean	set_tri_and_surface_list_pointers(struct Table*,int,int,int);
LOCAL	int	**add_to_bls_list(int**,int*,int*,int*);
LOCAL	int	block_dimension(int*);
LOCAL	void	blocks_on_grid_based_tri(TRI*,int***,COMPONENT***,RECT_GRID*, 
				INTERFACE *);
LOCAL	void	blocks_on_tri(TRI*,int***,COMPONENT***,RECT_GRID*,INTERFACE*);
LOCAL	void	fill_tri_and_surface_lists(int***,TRI*****,SURFACE*****,
	        			   INTERFACE*);
LOCAL	void	nearest_on_front_grid_block(int*,int*,INTERFACE*);
LOCAL  	boolean    nearest_local_on_front_grid_block(int*,int*,int,INTERFACE*);
LOCAL	void	set_off_front_comp3d(INTERFACE*);
LOCAL	void	set_x_face_comps(int,INTERFACE*);
LOCAL	void	set_y_face_comps(int,INTERFACE*);
LOCAL	void	set_z_face_comps(int,INTERFACE*);
LOCAL	void	shortest_distance3d(TRI_PROJECTION*);
LOCAL	void	show_TRI_list(INTERFACE*);
LOCAL	void 	set_tri_list_tolerance(RECT_GRID*);
LOCAL   double   shortest_2line_dist_dir(int,double*,double*,double*,double*,
				double*);
LOCAL   COMPONENT component_wrt_icoords3d_vertex(double*,int*,FT_POINT*,
				INTERFACE*);
LOCAL	int 	compare_tris(const void*,const void*);

LOCAL	double	crx_tol;/*TOLERANCE*/
LOCAL	double	crx_tolv;
LOCAL	double	crx_toll;
LOCAL	double	ctol;
LOCAL	const double	blk_tol = 1.0e-6;

LOCAL	boolean	comp_is_on_surface(
	SURFACE         *surf,
	const COMPONENT *comps,
	int             n_eq)
{
	int i;
	COMPONENT pcomp, ncomp;

	if (comps == NULL)
	    return YES;

	pcomp = positive_component(surf);
	ncomp = negative_component(surf);

	for (i = 0; i < n_eq; ++i)
	{
	    if (pcomp == comps[i])
		return YES;
	    else if (ncomp == comps[i])
		return YES;
	}
	return NO;
}		/*end comp_is_on_surface*/

#define comps_are_on_surface(s,l_comp,r_comp)                         \
	comps_are_on_hyper_surf(Hyper_surf(s),l_comp,r_comp)

/*
*				component3d():
*
*	Determines the topological COMPONENT relative to a given
*	INTERFACE which contains the point x,y,z.
*
*	If the INTERFACE has been modified since last call, then
*	the tri, surface, comp lists are recreated.   This imposes
*	a rectangular grid structure on the INTERFACE, by default
*	over the smallest rectangle that contains all of the
*	INTERFACE points.   The list of tris and surfaces passing
*	through each grid block is then computed, tris[iz][iy][ix]
*	surfaces[iz][iy][ix], and also the array of off-front component
*	values  compon3d[iz][iy][ix].   For on-front blocks, the compon3d[][][]
*	array is given the flag value ONFRONT.
*
*	component() determines the grid block containing the point
*	x,y,z and if it is off-front, returns the value compon3d[iz][iy][ix].
*	If on-front, it locates the closest INTERFACE surface within
*	the grid block (and determines which side of the surface it
*	is on) by looping over the local tris[iz][iy][ix].  It then
*	returns the appropriate left or right COMPONENT value of
*	that SURFACE.
*
*	Returns component value if successful, or ERROR on error.
*	This assumes that ERROR is not a possible COMPONENT value.
*	ERROR occurs if enough space cannot be found for the
*	allocation of tri_list arrays.
*	Note:  Distances below are actually squared distances.
*/

LIB_LOCAL COMPONENT component3d(
	double		*coords,
	INTERFACE	*intfc)
{
	TRI_PROJECTION	Closest, Tri_proj;
	TRI_PROJECTION	*closest = &Closest, *tri_proj = &Tri_proj;
	TRI		**t;
	SURFACE		**s;
	int		icoords[MAXD], ix, iy, iz, nt;
	int		i, k;
	int             ixmin, ixmax, iymin, iymax, izmin, izmax;
	int		dim = intfc->dim;
	struct Table	*T = intfc->table;
	COMPONENT	comp;

	DEBUG_ENTER(component3d)
	 
	/* Check for no surfaces in the interface (interior only) */
	if (intfc->surfaces == NULL)
	{
	    DEBUG_LEAVE(component3d)
	    return intfc->default_comp;
	}

	if (intfc->modified || T->new_grid)
	{
	    if (make_tri_comp_lists(intfc) == FUNCTION_FAILED)
	    {
	    	(void) printf("WARNING in component3d(), "
	    	              "make_tri_comp_lists failed\n");
		DEBUG_LEAVE(component3d)
		return ERROR;
	    }
	}

	if (debugging("tst_comp3d"))
	{
	    print_general_vector("#comp3d debug ",coords,3,"\n");
	    show_COMP(stdout, intfc);
	}

	if (rect_in_which(coords,icoords,&T->rect_grid) == FUNCTION_FAILED)
	{
	    DEBUG_LEAVE(component3d)
	    return exterior_component(intfc);	/* Point Outside */
	}
	for (i = 0; i < dim; ++i)
	{
	    if (icoords[i] < 0) icoords[i] = 0;
	    if (icoords[i] >= T->rect_grid.gmax[i]) 
		icoords[i] = T->rect_grid.gmax[i] - 1;
	}
	ix = icoords[0];
	iy = icoords[1];
	iz = icoords[2];

	if (T->compon3d[iz][iy][ix] != ONFRONT)
	{
	    DEBUG_LEAVE(component3d)
	    return T->compon3d[iz][iy][ix];	/* Off Front */
	}
	/* On Front: */

	/* Find Closest Point on Front: */

	Closest.tri = NULL;
	Closest.s = NULL;
	Closest.pv = Closest.p1 = Closest.p2 = NULL;
	Closest.d2 = Closest.d = HUGE_VAL;
	Closest.nor_d2 = HUGE_VAL;
	for (i = 0; i < dim; ++i)
	    Tri_proj.pt[i] = coords[i];
	ixmin = (ix > 0) ? ix-1 : ix;
	ixmax = ((ix+1) < T->rect_grid.gmax[0]) ? ix+1 : ix;
	iymin = (iy > 0) ? iy-1 : iy;
	iymax = ((iy+1) < T->rect_grid.gmax[1]) ? iy+1 : iy;
	izmin = (iz > 0) ? iz - 1 : iz;
	izmax = ((iz+1) < T->rect_grid.gmax[2]) ? iz+1 : iz;

	ixmin = ixmax = ix;
	iymin = iymax = iy;
	izmin = izmax = iz;

	if(debugging("fill_comp"))
	{
	    tecplot_show_box_tris("crx_input", T->tris[iz][iy][ix], 
		T->num_of_tris[iz][iy][ix], &T->rect_grid, icoords);
	}

	for (iz = izmin; iz <= izmax; ++iz)
	{
	    for (iy = iymin; iy <= iymax; ++iy)
	    {
	        for (ix = ixmin; ix <= ixmax; ++ix)
		{
	            if (T->compon3d[iz][iy][ix] != ONFRONT)
			continue;
	            t = T->tris[iz][iy][ix];
	            nt = T->num_of_tris[iz][iy][ix];

		    for (k = 0; k < nt; ++k, ++t, ++s)
			Tri_projection_computed(*t) = NO;
		}
	    }
	}
	for (iz = izmin; iz <= izmax; ++iz)
	{
	    for (iy = iymin; iy <= iymax; ++iy)
	    {
	        for (ix = ixmin; ix <= ixmax; ++ix)
		{
	            if (T->compon3d[iz][iy][ix] != ONFRONT)
			continue;
	            t = T->tris[iz][iy][ix];
	            s = T->surfaces[iz][iy][ix];
	            nt = T->num_of_tris[iz][iy][ix];

	            for (k = 0; k < nt; ++k, ++t, ++s)
	            {
			if (Tri_projection_computed(*t) != YES)
			{
	                    Tri_proj.tri = *t;
	                    Tri_proj.s = *s;
	                    shortest_distance3d(tri_proj);
	                    if (new_tri_is_closer(tri_proj,closest) == YES)
				Closest = Tri_proj;
			}
			Tri_projection_computed(*t) = YES;
	            }
		}
	    }
	}

	if (Closest.tri == NULL)  /*There are no adjacent onfront blocks */
	{
            int icrds[3];

	    nearest_on_front_grid_block(icoords,icrds,intfc);
	    comp = component_wrt_icoords3d(coords,icrds,intfc);
	}
	else
	    comp = comp_at_closest(&Closest);
	
	DEBUG_LEAVE(component3d)
	return comp;
}		/*end component3d*/



/*
* 			component_wrt_icoords3d():
*
* 	functionally identical to component3d() except component_wrt_icoords3d()
*	works in one mesh block labeled as icoords. See comment for
*	component3d() for details.
*
*/

LOCAL COMPONENT component_wrt_icoords3d(
	double		*coords,
	int		*icoords,
	INTERFACE	*intfc)
{
	COMPONENT	comp, comp0;
	TRI_PROJECTION	Closest, Tri_proj;
	TRI_PROJECTION	*closest = &Closest, *tri_proj = &Tri_proj;
	TRI		**t;
	SURFACE		**s;
	FT_POINT		*p;
	int		i, nt, num = 0;
	int		dim = intfc->dim;
	int		ix = icoords[0], iy = icoords[1], iz = icoords[2];
	int             ixmin, ixmax, iymin, iymax, izmin, izmax;
	int		*gmax = (&topological_grid(intfc))->gmax;
	struct Table	*T = intfc->table;
        

	ixmin = (ix > 0) ? ix-1 : ix;
	ixmax = ((ix+1) < gmax[0]) ? ix+1 : ix;
	iymin = (iy > 0) ? iy-1 : iy;
	iymax = ((iy+1) < gmax[1]) ? iy+1 : iy;
	izmin = (iz > 0) ? iz - 1 : iz;
	izmax = ((iz+1) < gmax[2]) ? iz+1 : iz;

	 
	/* initialization for the shortest_distance3d() */

	Closest.tri = NULL;
	Closest.s = NULL;
	Closest.pv = Closest.p1 = Closest.p2 = NULL;
	Closest.d2 = HUGE_VAL;
	Closest.nor_d2 = HUGE_VAL;
	for (i = 0; i < dim; ++i)
	    Tri_proj.pt[i] = coords[i];

	for (iz = izmin; iz <= izmax; ++iz)
	{
	    for (iy = iymin; iy <= iymax; ++iy)
	    {
	        for (ix = ixmin; ix <= ixmax; ++ix)
		{
	            if (T->compon3d[iz][iy][ix] != ONFRONT)
			continue;
	            t = T->tris[iz][iy][ix];
	            nt = T->num_of_tris[iz][iy][ix];
	            for (i = 0; i < nt; ++i)
			Tri_projection_computed(t[i]) = NO;
		}
	    }
	}
	for (iz = izmin; iz <= izmax; ++iz)
	{
	    for (iy = iymin; iy <= iymax; ++iy)
	    {
	        for (ix = ixmin; ix <= ixmax; ++ix)
		{
	            if (T->compon3d[iz][iy][ix] != ONFRONT)
			continue;

	            /* find the number of tris and surfs in icoords */

	            t = T->tris[iz][iy][ix];
	            nt = T->num_of_tris[iz][iy][ix];
	            s = T->surfaces[iz][iy][ix];

	            /* loop over the tris in block at icoords to find the min */

	            for (i = 0; i < nt; ++i)
	            {   
			if (Tri_projection_computed(t[i]) != YES)
			{
			    ++num;
	                    Tri_proj.tri = t[i];
	                    Tri_proj.s = s[i];
	                    shortest_distance3d(tri_proj);
	                    if (new_tri_is_closer(tri_proj,closest) == YES)
	    	                Closest = Tri_proj;
			    
			    if(debugging("tst_comp3d"))
			    {
			        print_general_vector("pt", Tri_proj.pt, 3, "\n");
				printf("tri=%p\n", t[i]);
				print_tri_coords(t[i]);
				printf("d2=%24.16e  %d\n", Tri_proj.d2, Tri_proj.side);
			    }
			}
			Tri_projection_computed(t[i]) = YES;
	            }
	        }
	    }
	}
	if (Closest.tri == NULL)/*There are no adjacent onfront blocks*/
	{
	    int icrds[3];
	    /* Find nearest ONFRONT grid block */
	    nearest_on_front_grid_block(icoords,icrds,intfc);
	    return component_wrt_icoords3d(coords,icrds,intfc);
	}
	comp0 = -1;
	if(Closest.side == ONVERTEX)
	{
	    p = Point_of_tri(Closest.tri)[Closest.vertex_index];
	    comp0 = component_wrt_icoords3d_vertex(coords, icoords, p, intfc);
	}

	comp = comp_at_closest(&Closest);
	if(debugging("tst_comp3d"))
	{
	    printf("#proj_tri=%p  %d  d2=%24.16e comp=%d  comp0=%d\n", 
			    Closest.tri, Closest.side, Closest.d2, comp, comp0);
	}

	/*TMP just use the new alg. */
	if(comp0 != -1 && comp != comp0)
	{
	     /*DEBUG_TMP printf("WARNING, component_wrt_icoords3d " */
	          /*DEBUG_TMP "prev alg. and new alg. gives different comp %d  %d\n",  */
		  /*DEBUG_TMP comp, comp0); */
	     comp = comp0;
	}
	return comp;
}		/*end component_wrt_icoords3d*/


LOCAL	void	nearest_on_front_grid_block(
	int		*icoords,
	int		*icrds,
	INTERFACE	*intfc)
{
	RECT_GRID	*gr = &topological_grid(intfc);
	int		xmax = gr->gmax[0];
	int		ymax = gr->gmax[1];
	int		zmax = gr->gmax[2];
	int		ix = icoords[0], iy = icoords[1], iz = icoords[2];
	int		i, j, k;
	int             ix_closest, iy_closest, iz_closest;
	int		ir, irmin;
	struct Table	*T = intfc->table;

	/*
	*  At the expense of a considerably more complicated
	*  looping structure,  the following construction can be
	*  reduced from an O(n^3) algorithm to one that is O(n^3)
	*  only in the worst case.  The basic idea would be to loop
	*  over successive L infinity balls centered at icoords
	*  of integer radius ir until an ONFRONT grid cell is
	*  discovered.  Unfortunately this loop is complicated by
	*  requirement that each of these balls be modified by
	*  intersecting them with the interior of the computational
	*  domain.  This makes the looping structure quite complicated
	*  (it is already complicated in the unbounded domain case),
	*  but it should in general discover the closest ONFRONT grid
	*  cell in as few iterations as possible.
	*/

	irmin = sqr(xmax) + sqr(ymax) + sqr(zmax);
	ix_closest = iy_closest = iz_closest = -1;
	for (i = 0; i < xmax; ++i)
	{
	    for (j = 0; j < ymax; ++j)
	    {
	    	for (k = 0; k < zmax; ++k)
	    	{
	            if ((T->compon3d[k][j][i] != ONFRONT) ||
	                (T->num_of_tris[k][j][i] == 0))
	    	    	continue;
	    	    ir = sqr(i-ix)+sqr(j-iy)+sqr(k-iz);
	    	    if (ir < irmin)
	    	    {
	    	    	irmin = ir;
	    	    	ix_closest = i;
	    	    	iy_closest = j;
	    	    	iz_closest = k;
	    	    }
	    	}
	    }
	}
	if ((ix_closest < 0) || (iy_closest < 0) || (iz_closest < 0))
	{
	    screen("ERROR in nearest_on_front_grid_block(), "
	           "no front block near icoords (%d, %d, %d) ",ix,iy,iz);
	    (void) printf("not found\n");
	    clean_up(ERROR);
	}
	icrds[0] = ix_closest;
	icrds[1] = iy_closest;
	icrds[2] = iz_closest;
}		/*end nearest_on_front_grid_block*/

LIB_LOCAL boolean	nearest_local_on_front_grid_block(
	int		*icoords,
	int		*icrds,		/*output */
	int		width,
	INTERFACE	*intfc)
{
	RECT_GRID	*gr = &topological_grid(intfc);
	int		xmax = gr->gmax[0];
	int		ymax = gr->gmax[1];
	int		zmax = gr->gmax[2];
	int		ixmin, ixmax, iymin, iymax, izmin, izmax;
	int		ix = icoords[0], iy = icoords[1], iz = icoords[2];
	int		i, j, k;
	int             ix_closest, iy_closest, iz_closest;
	int		ir, irmin;
	struct Table	*T = intfc->table;

	irmin = sqr(xmax) + sqr(ymax) + sqr(zmax);
	
	ix_closest = -1;
	iy_closest = -1;
	iz_closest = -1;

	ixmin = max(icoords[0]-width, 0);
	ixmax = min(icoords[0]+width+1, xmax);
	
	iymin = max(icoords[1]-width, 0);
	iymax = min(icoords[1]+width+1, ymax);
	
	izmin = max(icoords[2]-width, 0);
	izmax = min(icoords[2]+width+1, zmax);

	for (i = ixmin; i < ixmax; ++i)
	{
	    for (j = iymin; j < iymax; ++j)
	    {
	    	for (k = izmin; k < izmax; ++k)
	    	{
	            if ((T->compon3d[k][j][i] != ONFRONT) ||
	                (T->num_of_tris[k][j][i] == 0))
	    	    	continue;

	    	    ir = sqr(i-ix)+sqr(j-iy)+sqr(k-iz);
	    	    if (ir < irmin)
	    	    {
	    	    	irmin = ir;
	    	    	ix_closest = i;
	    	    	iy_closest = j;
	    	    	iz_closest = k;
	    	    }
	    	}
	    }
	}

	if(ix_closest == -1)
		return NO;

	icrds[0] = ix_closest;
	icrds[1] = iy_closest;
	icrds[2] = iz_closest;
	
	return YES;

}		/*end nearest_on_front_grid_block*/




/*
*			long_component3d():
*
*	Determines the topological COMPONENT relative to a given
*	INTERFACE containing a point with coordinates coords.
*
*	This version is much less efficient for many calls on
*	the same unmodified INTERFACE than is function component().
*	For a small number of calls it is much more efficient.
*
*
*	Differs from  component()  in that the local comp/tri lists
*	are not constructed.   A loop over all TRIS of the INTERFACE
*	is performed to determine the closest one and the appropriate
*	side.   The corresponding CURVE or SURFACE COMPONENT is then returned.
*	Thus the cost of the function is proportional to the total
*	number of TRIS on the INTERFACE.
*
*	Note:  Distances below are actually squared distances.
*/

LIB_LOCAL COMPONENT long_component3d(
	double		*coords,
	INTERFACE	*intfc)
{
	TRI_PROJECTION	Closest, Tri_proj;
	TRI_PROJECTION	*closest = &Closest, *tri_proj = &Tri_proj;
	TRI		*t;
	SURFACE		**s;
	int		i;
	int		dim = intfc->dim;
	COMPONENT	comp;

	/* Find Closest Point on Front: */
	DEBUG_ENTER(long_component3d)

	Closest.tri = NULL;
	Closest.s = NULL;
	Closest.pv = Closest.p1 = Closest.p2 = NULL;
	Closest.d2 = HUGE_VAL;
	Closest.nor_d2 = HUGE_VAL;
	for (i = 0; i < dim; ++i)
	    Tri_proj.pt[i] = coords[i];
	for (s = intfc->surfaces; s && *s; ++s)
	{
	    Tri_proj.s = *s;
	    for (t = first_tri(*s); !at_end_of_tri_list(t,*s); t = t->next)
	    {
	    	Tri_proj.tri = t;
	    	shortest_distance3d(tri_proj);
	    	if (new_tri_is_closer(tri_proj,closest) == YES)
	    		Closest = Tri_proj;
	    }
	}
	comp = comp_at_closest(&Closest);
	DEBUG_LEAVE(long_component3d)
	return comp;
}		/*end long_component3d*/

LIB_LOCAL COMPONENT dir_long_component3d(
	double		*coords,
	INTERFACE	*intfc)
{
	TRI_PROJECTION	Closest, Tri_proj;
	TRI		*t;
	SURFACE		**s;
	int		idir,i,iv,ie,dim = intfc->dim;
	COMPONENT	comp;
	RECT_GRID	gr = topological_grid(intfc);
	double		crds_crx[MAXD],h[MAXD];

	/* Find Closest Point on Front: */
	DEBUG_ENTER(long_component3d)

	for (idir = 0; idir < dim; ++idir)
	    h[idir] = gr.U[idir] - gr.L[idir];

	Closest.tri = NULL;
	Closest.s = NULL;
	Closest.pv = Closest.p1 = Closest.p2 = NULL;
	Closest.d = HUGE_VAL;
	for (s = intfc->surfaces; s && *s; ++s)
	{
	    Tri_proj.s = *s;
	    for (t = first_tri(*s); !at_end_of_tri_list(t,*s); t = t->next)
	    {
	    	Tri_proj.tri = t;
		for (idir = 0; idir < 3; ++idir)
		{
		    for (i = 0; i < dim; ++i)
		    	Tri_proj.pt[i] = coords[i];
	    	    if (tri_edge_crossing(t,coords,Tri_proj.pt,idir,
		    		&iv,&ie,h))
		    {
		    	Tri_proj.tri = t;
			Tri_proj.s = *s;
			Tri_proj.pv = (iv == ERROR) ? NULL : 
				Point_of_tri(t)[iv];
			Tri_proj.vertex_index = iv;
			Tri_proj.p1 = (ie == ERROR) ? NULL : 
				Point_of_tri(t)[ie];
			Tri_proj.p2 = (ie == ERROR) ? NULL : 
				Point_of_tri(t)[Next_m3(ie)];
			Tri_proj.side_index = iv;
		    	Tri_proj.d = fabs(crds_crx[idir] - coords[idir]);
			Tri_proj.idir = idir;
		    }
		}
	    	if (Tri_proj.d < Closest.d)
	    		Closest = Tri_proj;
	    }
	}
	if (Closest.pv != NULL)
	{
	}
	else if (Closest.p1 != NULL)
	{
	    double d1 = coords[idir] - Closest.pt[idir];
	    double d2 = Tri_normal(Closest.tri)[idir];
	    if (same_sign(d1,d2))
	    	comp = positive_component(Closest.s);
	    else
	    	comp = negative_component(Closest.s);
	}
	else
	{
	    double d1 = coords[idir] - Closest.pt[idir];
	    double d2 = Tri_normal(Closest.tri)[idir];
	    if (same_sign(d1,d2))
	    	comp = positive_component(Closest.s);
	    else
	    	comp = negative_component(Closest.s);
	}
	DEBUG_LEAVE(long_component3d)
	return comp;
}		/*end long_component3d*/


/*
*			nearest_interface_point3d():
*
*	Given a point with coordinates coords and a COMPONENT comp,
*	this routine locates the closest point of the INTERFACE which
*	borders COMPONENT comp. The coordinates of this point are ans.
*	Also returns the bond and curve containing p, and the paramatric
*	location of the point on this bond.
*	Returns value 1 or 0 if successful or not in finding a
*	closest point.
*
*	This routine is O(1) if called on an ONFRONT gridblock,
*	but is otherwise O(N).
*
*	If surface is not NULL then component and boundary are ignored,
*	and instead the routine finds the closest point on this surface.
*/


LIB_LOCAL boolean nearest_interface_point3d(
	double		   *coords,
	COMPONENT	   comp,
	INTERFACE	   *intfc,
	USE_BOUNDARIES	   bdry,
	HYPER_SURF	   *hs,
	double		   *ans,
	double		   *a,
	HYPER_SURF_ELEMENT **phse,
	HYPER_SURF	   **phs)
{
        const COMPONENT *eq_comps;
	int             n_eq;
	TRI_PROJECTION	Closest, Tri_proj;
	TRI_PROJECTION  *closest = &Closest, *tri_proj = &Tri_proj;
	TRI		**t;
	SURFACE		**s;
	int		ix, iy, iz;	/* Grid square containing coords */
	int		icoords[MAXD];	/* Grid square containing x,y */
	int		ix1, ix2, iy1, iy2, iz1, iz2;
	int		i, k;
	int		dim = intfc->dim;
	struct Table	*T = intfc->table;

	if (intfc->modified || T->new_grid)
	{
	    if (make_tri_comp_lists(intfc) == FUNCTION_FAILED)
	    {
	        (void) printf("WARNING in nearest_interface_point3d(), "
	                      "make_tri_comp_lists() failed\n");
	        print_interface(intfc);
	        return NO;
	    }
	}

	if ((point_in_buffer(coords,&T->rect_grid) == YES) ||
	    (rect_in_which(coords,icoords,&T->rect_grid) == FUNCTION_FAILED) ||
	    (T->compon3d[icoords[2]][icoords[1]][icoords[0]] != ONFRONT))
	{
		if(!nearest_local_on_front_grid_block(icoords,icoords,10,intfc))
		{
		    if (debugging("interpolate"))
			print_rectangular_grid(&T->rect_grid);
	            return long_nearest_interface_point3d(coords,comp,intfc,
					bdry,hs,ans,a,phse,phs);
		}
        }

	eq_comps = equivalent_components_list(comp,&n_eq,intfc);

	/* On Front: */

	/* Find Closest Point on Front: */

	Closest.tri = NULL;
	Closest.s = NULL;
	Closest.pv = Closest.p1 = Closest.p2 = NULL;
	Closest.d2 = HUGE_VAL;
	Closest.nor_d2 = HUGE_VAL;
	for (i = 0; i < dim; ++i)
	    Tri_proj.pt[i] = coords[i];

	ix = icoords[0];
	iy = icoords[1];
	iz = icoords[2];
	ix1 = ix - 1;
	if (ix1 < 0)
	    ix1 = 0;
	ix2 = ix + 1;
	if (ix2 >= T->rect_grid.gmax[0])
	    ix2 = T->rect_grid.gmax[0] - 1;
	iy1 = iy - 1;
	if (iy1 < 0)
	    iy1 = 0;
	iy2 = iy + 1;
	if (iy2 >= T->rect_grid.gmax[1])
	    iy2 = T->rect_grid.gmax[1] - 1;
	iz1 = iz - 1;
	if (iz1 < 0)
	    iz1 = 0;
	iz2 = iz + 1;
	if (iz2 >= T->rect_grid.gmax[2])
	    iz2 = T->rect_grid.gmax[2] - 1;
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
	                if (hs)
	                {
	                    if (Surface_of_hs(hs) != *s)
	                        continue;
	                }
	                else
	                {
			    if (!comp_is_on_surface(*s,eq_comps,n_eq))
	                        continue;
			    if (skip_boundary_hs(Hyper_surf(*s),bdry))
	                        continue;

	                }
	        	Tri_proj.tri = *t;
	        	Tri_proj.s = *s;
	                shortest_distance3d(tri_proj);
			
			if(debugging("line_tri"))
			{
			    printf("#nearest pt  %d   %p  %24.16e\n",
			        tri_proj->side, tri_proj->tri, tri_proj->d2);
			    print_tri_coords(tri_proj->tri);
			}
	        	if (new_tri_is_closer(tri_proj,closest) == YES)
	        	    Closest = Tri_proj;
	            }
	        }
	    }
	}

	if (Closest.tri == NULL)
	    return long_nearest_interface_point3d(coords,comp,intfc,bdry,hs,
	        				  ans,a,phse,phs);

	*phse = Hyper_surf_element(Closest.tri);
	*phs = Hyper_surf(Closest.s);
	for (i = 0; i < dim; ++i)
	{
	    int j;

	    ans[i] = 0.;
	    for (j = 0; j < 3; ++j)
	    	ans[i] += Closest.a[j]*Coords(Point_of_tri(Closest.tri)[j])[i];
	    a[i] = Closest.a[i];
	}
	return YES;
}		/*end nearest_interface_point3d*/

/*
*			nearest_similar_interface_point3d():
*
*	Given a point and  COMPONENTs compn,compp locates the closest
*	point of the INTERFACE on a surface with same COMPONENTs.
*	(and same orientation)
*	Also returns the tri and surface containing p, and the parametric
*	location of the point on this tri.
*	Returns value 1 or 0 if succesful or not in finding a
*	closest point.
*
*	This routine is O(1) if called on an ONFRONT gridblock,
*	but is otherwise O(N).
*
*	If surface is not NULL then component and boundary are ignored,
*	and instead the routine finds the closest point on the surface.
*/

LIB_LOCAL boolean nearest_similar_interface_point3d(
	double		   *coords,
	COMPONENT	   compp,
	COMPONENT	   compn,
	INTERFACE	   *intfc,
	USE_BOUNDARIES	   bdry,
	HYPER_SURF	   *hs,
	double		   *ans,
	double		   *a,
	HYPER_SURF_ELEMENT **phse,
	HYPER_SURF	   **phs)
{
	TRI_PROJECTION	Closest, Tri_proj;
	TRI_PROJECTION	*closest = &Closest, *tri_proj = &Tri_proj;
	TRI		**t;
	SURFACE		**s;
	int		ix, iy, iz;		/* Grid square containing x,y */
	int		icoords[MAXD];
	int		ix1, ix2, iy1, iy2, iz1, iz2;
	int		i, k;
	int		dim = intfc->dim;
	struct Table	*T = intfc->table;

	if (intfc->modified || T->new_grid)
	    if (make_tri_comp_lists(intfc) == FUNCTION_FAILED)
	    {
	        return NO;
	    }
	/* Test for Off Front */
	if ((rect_in_which(coords, icoords, &T->rect_grid) == FUNCTION_FAILED)
	    || (T->compon3d[icoords[2]][icoords[1]][icoords[0]] != ONFRONT))
	{
	    return long_nearest_similar_interface_point3d(coords,compp,compn,
							  intfc,bdry,hs,ans,
							  a,phse,phs);
	}

	/* On Front */

	/* Find Closest Point on Front */

	Closest.tri = NULL;
	Closest.s = NULL;
	Closest.pv = Closest.p1 = Closest.p2 = NULL;
	Closest.d2 = HUGE_VAL;
	Closest.nor_d2 = HUGE_VAL;
	for (i = 0; i < dim; ++i)
	    Tri_proj.pt[i] = coords[i];

	ix = icoords[0];
	iy = icoords[1];
	iz = icoords[2];
	ix1 = ix - 1;
	if (ix1 < 0)
	    ix1 = ix;
	ix2 = ix + 1;
	if (ix2 == T->rect_grid.gmax[0])
	    ix2 = ix;
	iy1 = iy - 1;
	if (iy1 < 0)
	    iy1 = iy;
	iy2 = iy + 1;
	if (iy2 == T->rect_grid.gmax[1])
	    iy2 = iy;
	iz1 = iz - 1;
	if (iz1 < 0)
	    iz1 = iz;
	iz2 = iz + 1;
	if (iz2 == T->rect_grid.gmax[2])
	    iz2 = iz;

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
	                if (hs)
	                {
	                    if (Surface_of_hs(hs) != *s)
	                        continue;
	                }
	                else
	                {
			    if (!comps_are_on_surface(*s,compp,compn))
	                        continue;
			    if (skip_boundary_hs(Hyper_surf(*s),bdry))
	                        continue;
	                }
	        	Tri_proj.tri = *t;
	        	Tri_proj.s = *s;
	                shortest_distance3d(tri_proj);
	        	if (new_tri_is_closer(tri_proj,closest) == YES)
	        	    Closest = Tri_proj;
	            }
	        }
	    }
	}

	if (Closest.tri == NULL)
	{
	    if (DEBUG)
	    {
	    	(void) printf("nearest_similar_interface_point3d(), "
	    	              "Closest.tri == NULL\n");
	        (void) printf("calling long_nearest_similar_");
	        (void) printf("interface_point3d()\n");
	    }
	    return long_nearest_similar_interface_point3d(coords,compp,compn,
							    intfc,bdry,hs,ans,
							    a,phse,phs);
	}

	*phse = Hyper_surf_element(Closest.tri);
	*phs = Hyper_surf(Closest.s);
	for (i = 0; i < dim; ++i)
	{
	    ans[i] = Closest.a[0] * Coords(Point_of_tri(Closest.tri)[0])[i] +
		     Closest.a[1] * Coords(Point_of_tri(Closest.tri)[1])[i] +
		     Closest.a[2] * Coords(Point_of_tri(Closest.tri)[2])[i];
	    a[i] = Closest.a[i];
	}
	return YES;
}		/*end nearest_similar_interface_point3d*/


LIB_LOCAL boolean nearest_interface_point_within_range3d(
	double		   *coords,
	COMPONENT	   comp,
	INTERFACE	   *intfc,
	USE_BOUNDARIES	   bdry,
	HYPER_SURF	   *hs,
	double		   *ans,
	double		   *a,
	HYPER_SURF_ELEMENT **phse,
	HYPER_SURF	   **phs,
	int 		   range)
{
        const COMPONENT *eq_comps;
	int             n_eq;
	TRI_PROJECTION	Closest, Tri_proj;
	TRI_PROJECTION  *closest = &Closest, *tri_proj = &Tri_proj;
	TRI		**t;
	SURFACE		**s;
	int		ix, iy, iz;	/* Grid square containing coords */
	int		icoords[MAXD];	/* Grid square containing x,y */
	int		ix1, ix2, iy1, iy2, iz1, iz2;
	int		i, k;
	int		dim = intfc->dim;
	struct Table	*T = intfc->table;

	if (intfc->modified || T->new_grid)
	{
	    if (make_tri_comp_lists(intfc) == FUNCTION_FAILED)
	    {
	        (void) printf("WARNING in "
			      "nearest_interface_point_within_range3d(), "
	                      "make_tri_comp_lists() failed\n");
	        print_interface(intfc);
	        return NO;
	    }
	}

	if ((rect_in_which(coords,icoords,&T->rect_grid) == FUNCTION_FAILED) ||
	    (T->compon3d[icoords[2]][icoords[1]][icoords[0]] != ONFRONT))
	{
	    if(!nearest_local_on_front_grid_block(icoords,icoords,range,intfc))
	        return NO;
        }

	eq_comps = equivalent_components_list(comp,&n_eq,intfc);

	/* On Front: */

	/* Find Closest Point on Front: */

	Closest.tri = NULL;
	Closest.s = NULL;
	Closest.pv = Closest.p1 = Closest.p2 = NULL;
	Closest.d2 = HUGE_VAL;
	Closest.nor_d2 = HUGE_VAL;
	for (i = 0; i < dim; ++i)
	    Tri_proj.pt[i] = coords[i];

	ix = icoords[0];
	iy = icoords[1];
	iz = icoords[2];
	ix1 = ix - 1;
	if (ix1 < 0)
	    ix1 = ix;
	ix2 = ix + 1;
	if (ix2 == T->rect_grid.gmax[0])
	    ix2 = ix;
	iy1 = iy - 1;
	if (iy1 < 0)
	    iy1 = iy;
	iy2 = iy + 1;
	if (iy2 == T->rect_grid.gmax[1])
	    iy2 = iy;
	iz1 = iz - 1;
	if (iz1 < 0)
	    iz1 = iz;
	iz2 = iz + 1;
	if (iz2 == T->rect_grid.gmax[2])
	    iz2 = iz;
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
	                if (hs)
	                {
	                    if (Surface_of_hs(hs) != *s)
	                        continue;
	                }
	                else
	                {
			    if (!comp_is_on_surface(*s,eq_comps,n_eq))
	                        continue;
			    if (skip_boundary_hs(Hyper_surf(*s),bdry))
	                        continue;

	                }
	        	Tri_proj.tri = *t;
	        	Tri_proj.s = *s;
	                shortest_distance3d(tri_proj);
			
			if(debugging("line_tri"))
			{
			    printf("#nearest pt  %d   %p  %24.16e\n",
			        tri_proj->side, tri_proj->tri, tri_proj->d2);
			    print_tri_coords(tri_proj->tri);
			}
	        	if (new_tri_is_closer(tri_proj,closest) == YES)
	        	    Closest = Tri_proj;
	            }
	        }
	    }
	}

	if (Closest.tri == NULL)
	    return NO;

	*phse = Hyper_surf_element(Closest.tri);
	*phs = Hyper_surf(Closest.s);
	for (i = 0; i < dim; ++i)
	{
	    int j;

	    ans[i] = 0.;
	    for (j = 0; j < 3; ++j)
	    	ans[i] += Closest.a[j]*Coords(Point_of_tri(Closest.tri)[j])[i];
	    a[i] = Closest.a[i];
	}
	return YES;
}		/*end nearest_interface_point_within_range3d*/

/*
*			long_nearest_interface_point3d():
*
*	Given a coordinates (coords) and a COMPONENT comp, locates the closest
*	point ans of the INTERFACE which borders COMPONENT comp.
*	Also returns the tri and surface containing ans, and the paramatric
*	location of the point on this ans.
*	Returns value 1 or 0 if succesful or not in finding a
*	closest point.
*
*	If surface is not NULL then component and boundary are ignored,
*	and instead the routine finds the closest point along curve.
*/

LIB_LOCAL boolean long_nearest_interface_point3d(
	double		*coords,
	COMPONENT	comp,
	INTERFACE	*intfc,
	USE_BOUNDARIES	bdry,
	HYPER_SURF	*hs,
	double		*ans,
	double		*a,
	HYPER_SURF_ELEMENT **phse,
	HYPER_SURF	**phs)
{
        const COMPONENT *eq_comps;
	int             n_eq;
	TRI_PROJECTION	Closest, Tri_proj;
	TRI_PROJECTION	*closest = &Closest, *tri_proj = &Tri_proj;
	TRI		*t;
	SURFACE		**s;
	int		i;
	int		dim = intfc->dim;

	printf("#long_nearest_interface_point3d is called\n");
	
	/* Find Closest Point on Front: */
	Closest.tri = NULL;
	Closest.s = NULL;
	Closest.pv = Closest.p1 = Closest.p2 = NULL;
	Closest.d2 = HUGE_VAL;
	Closest.nor_d2 = HUGE_VAL;
	for (i = 0; i < dim; ++i)
	    Tri_proj.pt[i] = coords[i];

	eq_comps = equivalent_components_list(comp,&n_eq,intfc);
	for (s = intfc->surfaces; s && *s; ++s)
	{
	    /* new made wall surface has no tris, should be excluded */
	    if(first_tri(*s) == NULL)
	        continue;
	    
	    if (hs)
	    {
	    	if (Surface_of_hs(hs) != *s)
	    	    continue;
	    	/*
	         * Note: In this case the for statement is redundant but it
	    	 * does provide a check that the surface is on the interface
	    	 * intfc
	         */
	    }
	    else
	    {
		if (!comp_is_on_surface(*s,eq_comps,n_eq))
	    	    continue;
		if (skip_boundary_hs(Hyper_surf(*s),bdry))
	    	    continue;
	    }
	    Tri_proj.s = *s;
	    for (t = first_tri(*s); !at_end_of_tri_list(t,*s); t = t->next)
	    {
	        Tri_proj.tri = t;
	        shortest_distance3d(tri_proj);
	        if (new_tri_is_closer(tri_proj,closest) == YES)
	            Closest = Tri_proj;
	    }
	}

	if (Closest.tri == NULL)
	{ 
	    return NO;
	}
	*phse = Hyper_surf_element(Closest.tri);
	*phs = Hyper_surf(Closest.s);

	for (i = 0; i < dim; ++i)
	{
	    ans[i] = Closest.a[0] * Coords(Point_of_tri(Closest.tri)[0])[i] +
		     Closest.a[1] * Coords(Point_of_tri(Closest.tri)[1])[i] +
		     Closest.a[2] * Coords(Point_of_tri(Closest.tri)[2])[i];
	    a[i] = Closest.a[i];
	}
	return YES;
}		/*end long_nearest_interface_point3d*/

/*
*			long_nearest_similar_interface_point3d():
*
*	Given a point and COMPONENTs  compp,compn locates the closest
*	point ans of the INTERFACE  and curve with same COMPONENTs .
*       (and same orientation)
*	Also returns the tri and surface containing ans, and the paramatric
*	location of the point on this tri.
*	Returns value 1 or 0 if succesful or not in finding a
*	closest point.
*
*	If surface is not NULL then component and boundary are ignored,
*	and instead the routine finds the closest point on the surface.
*/

LIB_LOCAL boolean long_nearest_similar_interface_point3d(
	double		*coords,
	COMPONENT	compp,
	COMPONENT	compn,
	INTERFACE	*intfc,
	USE_BOUNDARIES	bdry,
	HYPER_SURF	*hs,
	double		*ans,
	double		*a,
	HYPER_SURF_ELEMENT **phse,
	HYPER_SURF	**phs)
{
	TRI_PROJECTION	Closest, Tri_proj;
	TRI_PROJECTION	*closest = &Closest, *tri_proj = &Tri_proj;
	TRI		*t;
	SURFACE		**s;
	int		i;
	int		dim = intfc->dim;

	/* Find Closest Point on Front: */
	Closest.tri = NULL;
	Closest.s = NULL;
	Closest.pv = Closest.p1 = Closest.p2 = NULL;
	Closest.d2 = HUGE_VAL;
	Closest.nor_d2 = HUGE_VAL;
	for (i = 0; i < dim; ++i)
	    Tri_proj.pt[i] = coords[i];

	for (s = intfc->surfaces; *s; ++s)
	{
	    if (hs)
	    {
	    	if (Surface_of_hs(hs) != *s)
	    		continue;
	    	/* Note: In this case for statement is redundant but it
	    	 * does provide a check that the surface is on interface
	    	 * intfc */
	    }
	    else
	    {
		if (!comps_are_on_surface(*s,compp,compn))
	    		continue;
		if (skip_boundary_hs(Hyper_surf(*s),bdry))
	    		continue;
	    }
	    Tri_proj.s = *s;
	    for (t = first_tri(*s); !at_end_of_tri_list(t,*s); t = t->next)
	    {
	    	Tri_proj.tri = t;
	                shortest_distance3d(tri_proj);
	    	if (new_tri_is_closer(tri_proj,closest) == YES)
	    	{
	    	    Closest = Tri_proj;
	    	}
	    }
	}

	*phse = Hyper_surf_element(Closest.tri);
	*phs = Hyper_surf(Closest.s);
	if (Closest.tri == NULL)
	{
	    return NO;
	}
	for (i = 0; i < dim; ++i)
	{
	    ans[i] = Closest.a[0] * Coords(Point_of_tri(Closest.tri)[0])[i] +
		     Closest.a[1] * Coords(Point_of_tri(Closest.tri)[1])[i] +
		     Closest.a[2] * Coords(Point_of_tri(Closest.tri)[2])[i];
	    a[i] = Closest.a[i];
	}
	return YES;
}		/*end long_nearest_similar_interface_point3d*/


/*
*			shortest_distance3d():
*
*	Computes the distance of a position pt = tri_proj->pt from
*	a triangle tri. The algorithm is based on simple geometry.
*	We decompose pt in the form
*
*	pt = N + a0*p0 + a1*p1 + a2*p2, where a0 + a1 + a2 = 1
*
*	where p0, p1, and p2 are the verticies of tri,  and N is a uni_array
*	normal to the plane of tri.  The formulas for N, a0, a1, and a2
*	are elementary.  Letting <.> denote the usual scalar product,
*	and n = (p1 - p0) x (p2 - p0) be the area weighted normal uni_array
*	to the triangle.
*
*	N = <pt - p0,n/|n|> n/|n|,  |n| = sqrt(<n,n>)
*	a0 = triple_product(pt-p1),(pt-p2),n)/<n,n>
*	a1 = triple_product(pt-p2),(pt-p0),n)/<n,n>
*	a2 = triple_product(pt-p0),(pt-p1),n)/<n,n>
*
*	It is easy to check that a0 + a1 + a2 = 1.
*
*	The computation for the closest point is then based on the following
*	geometric observations. For each i,  let H_i denote the half space
*	containing tri and bounded by the plane generated by n and the side of
*	tri opposite the vertex pi. Then ai > 0 if and only if pt is in H_i.
*	It is immediately clear that the closest point in tri to pt lies in
*	the interior of tri if and only if all of the ai are positive.  
*	Otherwise the closest point in tri to pt must lie on the boundary
*	of tri.  Furthermore,  if ai <= 0,  then the closest point in tri
*	to pt lies on the side of tri opposite pi.
*
*	Input:
*		tri_proj->pt     -	position of point
*		tri_proj->tri    -	triangle
*		tri_proj->s      -	surface
*
*	Output:
*		tri_proj->d2     -	square of distance(pt,tri)
*		tri_proj->nor_d2 -	square of distance(pt,plane of tri)
*		tri_proj->side   -	side of tri on which pt lies,
*                                       if pt is a vertex, v, then this side
*                                       makes the smaller angle with respect
*                                       to the displacment vector from the
*                                       vertex to pt. Note that if the closest
*                                       point on tri to pt is a vertex of tri,
*                                       then tri must lie entirely on the
*                                       opposite side from pt of the plane
*                                       with normal the displacement uni_array
*                                       from that vertex to pt.  In this case
*                                       the cos of all uni_arrays from points in
*                                       tri to v and the vector from v to pt
*                                       must be negative (angle bigger than 90
*                                       degrees, and hence the closest side
*                                       is the one with the smaller cosine in
*                                       absolute value.
*		tri_proj->pv     -	if the closest point in tri to pt
*			                is a vertex,  then the address of
*			                of this vertex is returned in pv,
*			                otherwise pv is null.
*	        tri_proj->a      -      Projection coefficients.
*		tri_proj->p1     -	If the point on tri closest to pt
*		tri_proj->p2   		lies on the interior of a side of tri,
*			                then the two vertices bounding this
*			                side are returned in p1 and p2.
*/

LOCAL void shortest_distance3d(
	TRI_PROJECTION	*tri_proj)
{
	TRI   *tri = tri_proj->tri;
	double pt0, pt1, pt2;
	double a0, a1, a2;
	FT_POINT *P0, *P1, *P2;	/* vertices of tri */
	double *p0, *p1, *p2;	/* pi = Coords(Pi)  */
	double dp00, dp01, dp02;	/* dpij = ptj - pi[j] */
	double dp10, dp11, dp12;
	double dp20, dp21, dp22;
	const double *nor;       /* normal to tri       */
	double n0, n1, n2;	/* components of nor   */
	double np, x, x0, x1, x2;/* temporary variables */
	double L;		/* L = 2*area(tri)^2   */
	double ps;
	double x01,  x12,  x20;
	double v100, v101, v102, lv10s;
	double v210, v211, v212, lv21s;
	double v020, v021, v022, lv02s;
	double q00, q01, q02,	/* Displacement uni_arrays from p to     */
	      q10, q11, q12,    /* the closest point on each tri side */
	      q20, q21, q22;
	double d0_2, d1_2, d2_2; /* Square distance from pt to tri sides */
	double cos0, cos1, cos2;
	const double* const *s;
	const double *l;

	pt0 = tri_proj->pt[0];
	pt1 = tri_proj->pt[1];
	pt2 = tri_proj->pt[2];
	tri_proj->pv = tri_proj->p1 = tri_proj->p2 = NULL;
	P0 = Point_of_tri(tri)[0]; p0 = Coords(P0);
	P1 = Point_of_tri(tri)[1]; p1 = Coords(P1);
	P2 = Point_of_tri(tri)[2]; p2 = Coords(P2);
	dp00 = pt0 - p0[0]; dp01 = pt1 - p0[1]; dp02 = pt2 - p0[2];
	dp10 = pt0 - p1[0]; dp11 = pt1 - p1[1]; dp12 = pt2 - p1[2];
	dp20 = pt0 - p2[0]; dp21 = pt1 - p2[1]; dp22 = pt2 - p2[2];

	l = length_side(tri);
	lv10s = l[0]*l[0];
	lv21s = l[1]*l[1];
	lv02s = l[2]*l[2];

	ps = (lv10s + lv21s + lv02s)/3.0;

	nor = Tri_normal(tri);
	n0 = nor[0]; n1 = nor[1]; n2 = nor[2];
	a0 = QDet3d(dp1,dp2,n); a1 = QDet3d(dp2,dp0,n); a2 = QDet3d(dp0,dp1,n);
	L = Dot3d(nor,nor);
	np = QDot3d(dp0,n);
	if (L <= sqr(EPSILON*ps))	/*TOLERANCE*/
	{
	    /*Zero area triangle*/
	    tri_proj->side = UNKNOWN_SIDE;
	    tri_proj->d2 = HUGE_VAL;
	    return;
	}
	a0 /= L; a1 /= L; a2 /= L;
	x = a0 + a1 + a2;
	a0 /= x;
	a1 /= x;
	a2 /= x;
	tri_proj->nor_d2 = np*np/L;
	if ((0.0 < a0) && (0.0 < a1) && (0.0 < a2))
	{
	    /* The point is strictly inside the triangle */
	    if (np > 0.0)
	        tri_proj->side = POSITIVE_SIDE;
	    else if (np < 0.0)
	        tri_proj->side = NEGATIVE_SIDE;
	    else
	        tri_proj->side = COPLANAR;/* pt is coplanar with tri */
	    tri_proj->d2 = tri_proj->nor_d2;
	    tri_proj->dpt[0] = a0*dp00 + a1*dp10 + a2*dp20;
	    tri_proj->dpt[1] = a0*dp01 + a1*dp11 + a2*dp21;
	    tri_proj->dpt[2] = a0*dp02 + a1*dp12 + a2*dp22;
	}
	else
	{
	    /*
	     * The point is exterior to the triangle or on the triangle
	     * boundary, compute the nearest projection onto the triangle
	     */ 

	    /* Determine the interior angles of the triangle */
	    s = side_vector(tri);
	    v100 = s[0][0]; v101 = s[0][1]; v102 = s[0][2];
	    v210 = s[1][0]; v211 = s[1][1]; v212 = s[1][2];
	    v020 = s[2][0]; v021 = s[2][1]; v022 = s[2][2];

	    /* Compute projections of p onto the triangle sides */
	    cos0 = -QDot3d(v10,v02)/(l[0]*l[2]);
            x01 = (a1*l[0] + a2*l[2]*cos0)/l[0];
	    if (x01 < 0.0) x01 = 0.0;
	    if (1.0 < x01) x01 = 1.0;
	    q00 = (1.0 - x01)*dp00 + x01*dp10;
	    q01 = (1.0 - x01)*dp01 + x01*dp11;
	    q02 = (1.0 - x01)*dp02 + x01*dp12;
	    d0_2 = QDot3d(q0,q0);

	    cos1 = -QDot3d(v10,v21)/(l[0]*l[1]);
	    x12 = (a2*l[1] + a0*l[0]*cos1)/l[1];
	    if (x12 < 0.0) x12 = 0.0;
	    if (1.0 < x12) x12 = 1.0;
	    q10 = (1.0 - x12)*dp10 + x12*dp20;
	    q11 = (1.0 - x12)*dp11 + x12*dp21;
	    q12 = (1.0 - x12)*dp12 + x12*dp22;
	    d1_2 = QDot3d(q1,q1);

	    cos2 = -QDot3d(v02,v21)/(l[1]*l[2]);
	    x20 = (a0*l[2] + a1*l[1]*cos2)/l[2];
	    if (x20 < 0.0) x20 = 0.0;
	    if (1.0 < x20) x20 = 1.0;
	    q20 = (1.0 - x20)*dp20 + x20*dp00;
	    q21 = (1.0 - x20)*dp21 + x20*dp01;
	    q22 = (1.0 - x20)*dp22 + x20*dp02;
	    d2_2 = QDot3d(q2,q2);

	    if ((d0_2 <= d1_2) && (d0_2 <= d2_2))
	    {
		/* Closest point is on the side 0 = p0 -> p1 */
	        tri_proj->d2 = d0_2;
		tri_proj->dpt[0] = q00;
		tri_proj->dpt[1] = q01;
		tri_proj->dpt[2] = q02;
		if (x01 <= 0.0)
		{
		    /* Closest point is p0 */
	    	    tri_proj->pv = P0;
	    	    a0 = 1.0;
		    a1 = 0.0;
		    a2 = 0.0;
		    tri_proj->side = ONVERTEX;
		    tri_proj->vertex_index = 0;
		    x0 = QDot3d(q0,v10);
		    x0 = (x0*x0)/(d0_2*lv10s);
		    x2 = QDot3d(q0,v02);
		    x2 = (x2*x2)/(d0_2*lv02s);
		    if (x0 < x2)
		    {
		        tri_proj->side_index = 0;
		    }
		    else
		    {
		        tri_proj->side_index = 2;
		    }
		}
		else if (1.0 <= x01)
		{
		    /* Closest point is p1 */
		    tri_proj->pv = P1;
		    a0 = 0.0;
	    	    a1 = 1.0;
		    a2 = 0.0;
		    tri_proj->side = ONVERTEX;
		    tri_proj->vertex_index = 1;
		    x1 = QDot3d(q0,v10);
		    x1 = x1*x1/(d0_2*lv10s);
		    x2 = QDot3d(q0,v21);
		    x2 = x2*x2/(d0_2*lv21s);
		    if (x1 < x2)
		    {
		        tri_proj->side_index = 1;
		    }
		    else
		    {
		        tri_proj->side_index = 2;
		    }
		}
		else
		{
		    /* Closest point is normal projection onto side 0 */
	    	    tri_proj->p1 = P0;
	    	    tri_proj->p2 = P1;
		    a0 = 1.0 - x01;
		    a1 = x01;
		    a2 = 0.0;
		    tri_proj->side = ONEDGE;
		    tri_proj->side_index = 0;
		}
	    }
	    else if ((d1_2 <= d0_2) && (d1_2 <= d2_2))
	    {
		/* Closest point is on the side 1 = p1 -> p2 */
	        tri_proj->d2 = d1_2;
		tri_proj->dpt[0] = q10;
		tri_proj->dpt[1] = q11;
		tri_proj->dpt[2] = q12;
		if (x12 <= 0.0)
		{
		    /* Closest point is p1 */
		    tri_proj->pv = P1;
		    a0 = 0.0;
	    	    a1 = 1.0;
		    a2 = 0.0;
		    tri_proj->side = ONVERTEX;
		    tri_proj->vertex_index = 1;
		    x1 = QDot3d(q1,v10);
		    x1 = x0*x0/(d1_2*lv10s);
		    x2 = QDot3d(q1,v21);
		    x2 = x2*x2/(d1_2*lv21s);
		    if (x1 < x2)
		    {
		        tri_proj->side_index = 1;
		    }
		    else
		    {
		        tri_proj->side_index = 2;
		    }
		}
		else if (1.0 <= x12)
		{
		    /* Closest point is p2 */
	    	    tri_proj->pv = P2;
		    a0 = 0.0;
		    a1 = 0.0;
	    	    a2 = 1.0;
		    tri_proj->side = ONVERTEX;
		    tri_proj->vertex_index = 2;
		    x1 = QDot3d(q1,v21);
		    x1 = x1*x1/(d1_2*lv21s);
		    x2 = QDot3d(q1,v02);
		    x2 = x2*x2/(d1_2*lv02s);
		    /*x2 = x0*x0/(d1_2*lv02s);  This looks like an obvious err. */
		    if (x1 < x2)
		    {
		        tri_proj->side_index = 1;
		    }
		    else
		    {
		        tri_proj->side_index = 2;
		    }
		}
		else
		{
		    /* Closest point is normal projection onto side 1 */
	            tri_proj->p1 = P1;
	            tri_proj->p2 = P2;
		    a0 = 0.0;
	            a1 = 1.0 - x12;
	            a2 = x12;
		    tri_proj->side = ONEDGE;
	            tri_proj->side_index = 1;
		}
	    }
	    else if ((d2_2 <= d0_2) && (d2_2 <= d1_2))
	    {
		/* Closest point is on the side 2 = p2 -> p0 */
	        tri_proj->d2 = d2_2;
		tri_proj->dpt[0] = q20;
		tri_proj->dpt[1] = q21;
		tri_proj->dpt[2] = q22;
		if (x20 <= 0.0)
		{
		    /* Closest point is p2 */
	    	    tri_proj->pv = P2;
		    a0 = 0.0;
		    a1 = 0.0;
	    	    a2 = 1.0;
		    tri_proj->side = ONVERTEX;
		    tri_proj->vertex_index = 2;
		    x1 = QDot3d(q2,v21);
		    x1 = x1*x1/(d2_2*lv21s);
		    x2 = QDot3d(q2,v02);
		    x2 = x0*x0/(d2_2*lv02s);
		    if (x1 < x2)
		    {
		        tri_proj->side_index = 1;
		    }
		    else
		    {
		        tri_proj->side_index = 2;
		    }
		}
		else if (1.0 <= x20)
		{
		    /* Closest point is p0 */
	    	    tri_proj->pv = P0;
	    	    a0 = 1.0;
		    a1 = 0.0;
		    a2 = 0.0;
		    tri_proj->side = ONVERTEX;
		    tri_proj->vertex_index = 0;
		    x0 = QDot3d(q2,v10);
		    x0 = x0*x0/(d2_2*lv10s);
		    x2 = QDot3d(q2,v02);
		    x2 = x2*x2/(d2_2*lv02s);
		    if (x0 < x2)
		    {
		        tri_proj->side_index = 0;
		    }
		    else
		    {
		        tri_proj->side_index = 2;
		    }
		}
		else
		{
		    /* Closest point is normal projection onto side 2 */
	    	    tri_proj->p1 = P2;
	    	    tri_proj->p2 = P0;
	    	    a0 = x20;
		    a1 = 0.0;
	    	    a2 = 1.0 - x20;
		    tri_proj->side = ONEDGE;
	            tri_proj->side_index = 2;
		}
	    }
	    else
	    {
		screen("ERROR in shortest_distance3d(), impossible case "
		       "no minimum distance to triangle side. \n");
		printf("d= %24.16e  %24.16e  %24.16e\n", d0_2, d1_2, d2_2);
		printf("a= %24.16e  %24.16e  %24.16e\n", a0, a1, a2);
		print_general_vector("l=", l, 3, "\n");
		print_tri(tri, tri->surf->interface);

		clean_up(ERROR);
	    }
	}
	tri_proj->a[0] = a0; tri_proj->a[1] = a1; tri_proj->a[2] = a2;
	tri_proj->d = sqrt(tri_proj->d2);
}		/*end shortest_distance3d*/


/*
*			new_tri_is_closer():
*
*	This geometric function decides whether or not the new triangle
*	whose projected position is contained in the TRI_PROJECTION structure
*	ntp is closer to the given point than the triangle described in the
*	TRI_PROJECTION structure tp.
*
*	Input:
*		TRI_PROJECTION *ntp = New triangle information
*		TRI_PROJECTION *tp  = Currently accepted closest triangle
*
*        Output:
*		return YES is ntp is determined to be closer to the point
*		NO otherwise.
*
*
*	Algorithm:
*		The TRI_PROJECTION structure contains as described in its
*		declaration found a the beginning of this file. The algorithm
*		procedes as follows. In the following let pt denote the point
*		for which we are trying to answer the question as to whether
*	        tp->tri or ntp->tri is closer to pt. There are two square
*		distances, the minimum square distance d2 and the minimum
*		square of normal distance nor_d2. If the closest point to
*		pt lays in the interior, these two square distances are
*		the same, else nor_d2 < d2.
*
*		1. If one or the other of ntp and tp has a NULL tri
*		   while the other tri is not NULL, then the selection with
*		   with the non-NULL tri is closer.
*
*		2. For all non-degenerated cases, we only need to compare
*                  tp->d2 and ntp->d2, the shorter one is closer.
*
*               3. Two cases are considered as degenerate cases. The first
*                  is when tp->side and ntp->side are both ONEDGE, and the
*                  the edges in tp and ntp are the same edge. The second
*                  is when tp->side and ntp->side are both ONVERTEX and the
*                  the closest vertices of tp and ntp are the same point.
*                  In both of the two cases, we divide the space into 4
*                  quadrants using the planes of the two triangles
*
*                                    \    III   /
*                                     \        /
*                                      \      /
*                                       \    /
*                                        \  /
*                                   IV    \/     II
*                                         /\
*                                        /  \
*                         tp->tri side  /    \  ntp->tri side
*                                      /      \
*                                     /        \
*                                    /     I    \
*
*                 
* 		3.1 For the ONEDGE case with the closest points being
*		    on the common edge for tp and ntp, if the other 
*                   point of tp and pt are on the different side of the
*		    plane of the triangle ntp->tri, while the other point 
*		    of ntp and pt are on the same side of the plane of 
*		    the triangle tp->tri, we choose npt. It happens in 
*                    quadrant II and IV. We can use the dot products of 
*		    sv(see old_tri_on_edge()) and normals to dertermine this.
*		    In quadrant I and III, we can use normal distance
*		    tp->nor_d2 and ntp->nor_d2 to decide the correct one
*		    we want. We choose the triangle with the longer normal
*		    distance.
*
*		3.2 For the ONVERTEX case with the closest point being
*                   on the same vertex, we have the same agorithm as 
*                   ONEDGE case. The only difference is that we need 
*		    to dertermine if the  other two points of ntp are 
*		    on the same or different side of the plane of the 
*		    triangle tp with pt.
*/

LOCAL	boolean	new_tri_is_closer(
	TRI_PROJECTION *ntp,
	TRI_PROJECTION *tp)
{

	if ((tp->tri == NULL) && (ntp->tri != NULL))
	    return YES;
	if ((ntp->tri == NULL) && (tp->tri != NULL))
	    return NO;

	switch (tp->side)
	{
	case POSITIVE_SIDE:
	case NEGATIVE_SIDE:
	    return (ntp->d2 < tp->d2) ? YES : NO;
	case ONVERTEX:
	    return old_tri_on_vertex(ntp,tp);
	case ONEDGE:
	    return old_tri_on_edge(ntp,tp);
	case COPLANAR:
	    return NO;
	case UNKNOWN_SIDE:
	default:
	    return (ntp->d2 <= tp->d2) ? YES : NO;
	}
}		/*end new_tri_is_closer*/

LOCAL	boolean old_tri_on_vertex(
	TRI_PROJECTION *ntp,
	TRI_PROJECTION *tp)
{
	FT_POINT *p, *np;
	int  vi, nvi, si, nsi;
	FT_POINT *op, *nop;
	int   opi, nopi;
        double dtol = 10*MACH_EPS;	

	switch (ntp->side)
	{
	case POSITIVE_SIDE:
	case NEGATIVE_SIDE:
	case ONEDGE:
	    return (ntp->d2 <= tp->d2) ? YES : NO;
	case ONVERTEX:
	    vi = tp->vertex_index;
	    p = Point_of_tri(tp->tri)[vi];
	    nvi = ntp->vertex_index;
	    np = Point_of_tri(ntp->tri)[nvi];
	    if (p == np) /* Closest point is a common vertex */
	    {
		double dot,ndot,dots,dotns,ndots,ndotns;
		FT_POINT *po, *npo;
		int i;
		double sv[3], nsv[3];
		dot  = Dot3d(tp->dpt,Tri_normal(tp->tri));
		ndot = Dot3d(ntp->dpt,Tri_normal(ntp->tri));
		po = Point_of_tri(tp->tri)[Next_m3(vi)];
		npo = Point_of_tri(tp->tri)[Prev_m3(vi)];
		for (i = 0; i < 3; i++)
		{
		    sv[i] = Coords(po)[i] - Coords(p)[i];
		    nsv[i] = Coords(npo)[i] - Coords(p)[i];
		}
		dots = Dot3d(sv,Tri_normal(ntp->tri));
		dotns = Dot3d(nsv,Tri_normal(ntp->tri));
		po = Point_of_tri(ntp->tri)[Next_m3(nvi)];
		npo = Point_of_tri(ntp->tri)[Prev_m3(nvi)];
		for (i = 0; i < 3; i++)
		{
		    sv[i] = Coords(po)[i] - Coords(p)[i];
		    nsv[i] = Coords(npo)[i] - Coords(p)[i];
		}
		ndots = Dot3d(sv,Tri_normal(tp->tri));
		ndotns = Dot3d(nsv,Tri_normal(tp->tri));
		if (((dots*ndot <= -dtol) && (dotns*ndot <= -dtol)) ||
		      ((fabs(dots*ndot) <= dtol) && (dotns*ndot <= 0)) ||
		      ((dots*ndot <= 0) && fabs(dotns*ndot) <= dtol))
		{
		    if (((ndots*dot <= -dtol) && (ndotns*dot <= -dtol)) ||
			((fabs(ndots*dot) <= dtol) && (ndotns*dot <= 0)) ||
			((ndots*dot <= 0) && fabs(ndotns*dot) <= dtol))
	                return (ntp->nor_d2 > tp->nor_d2)? YES: NO;
		    else
		        return YES;
		 }
		 else
		 {
		     if (((ndots*dot <= -dtol) && (ndotns*dot <= -dtol)) ||
			 ((fabs(ndots*dot) <= dtol) && (ndotns*dot <= 0)) ||
			 ((ndots*dot <= 0) && fabs(ndotns*dot) <= dtol))
			 return NO;
		     else
		     {
		         if (((ndots*dot >= dtol) && (ndotns*dot <= -dtol)) ||
			     ((ndots*dot <= -dtol) && (ndotns*dot >= dtol)))
			     return NO;
			 if (((dots*ndot <= -dtol) && (dotns*ndot >= dtol)) ||
			     ((dots*ndot >= dtol) && (dotns*ndot <= -dtol)))
			     return YES;
			 else
			     return (ntp->nor_d2 < tp->nor_d2)? YES: NO;
	             }
	         }
             }		  
	     else
		 return (ntp->d2 < tp->d2) ? YES : NO;
	case COPLANAR:
	    return YES;
	case UNKNOWN_SIDE:
	default:
	    return NO;
	}
}	/* old_tri_on_vertex */

LOCAL	boolean old_tri_on_edge(
	TRI_PROJECTION *ntp,
	TRI_PROJECTION *tp)
{
        FT_POINT *p, *np;
        int i, vi, nvi;
	double sv[3], nsv[3];
	switch (ntp->side)
	{
	case POSITIVE_SIDE:
	case NEGATIVE_SIDE:
	    return (ntp->d2 <= tp->d2) ? YES : NO;
	case ONEDGE:
	    if ( (tp->p1 == ntp->p1 && tp->p2 == ntp->p2) ||
	         (tp->p1 == ntp->p2 && tp->p2 == ntp->p1) )
            {
	       double dot,ndot,dots,dotns;
	        vi = tp->side_index;
		nvi = ntp->side_index;
		p = Point_of_tri(tp->tri)[Prev_m3(vi)];
		np = Point_of_tri(ntp->tri)[Prev_m3(nvi)];
		dot  = Dot3d(tp->dpt,Tri_normal(tp->tri));
		ndot = Dot3d(ntp->dpt,Tri_normal(ntp->tri));
		for (i = 0; i < 3; i++)
		{
		    sv[i] = Coords(p)[i] - Coords(tp->p1)[i];
		    nsv[i] = Coords(np)[i] - Coords(ntp->p1)[i];
		}
		dots = Dot3d(sv,Tri_normal(ntp->tri));
		dotns = Dot3d(nsv,Tri_normal(tp->tri));
		if(dots*ndot<=0)
		{
		    if(dotns*dot<=0)
		        return (ntp->nor_d2 >= tp->nor_d2)? YES: NO;
		     else
		         return YES;
		}
	        else
		{
		    if(dotns*dot<=0)
		        return NO;
		     else
		        return (ntp->nor_d2 >= tp->nor_d2)? YES: NO; 
		}
	     }	
	    else
		return (ntp->d2 < tp->d2) ? YES : NO;
	case ONVERTEX:
	    return (ntp->d2 < tp->d2) ? YES : NO;
	case COPLANAR:
	    return YES;
	case UNKNOWN_SIDE:
	default:
	    return NO;
	}
}	/* old_tri_on_edge */


/*
*			make_tri_comp_lists():
*
*	This function determines the local component and tri list arrays
*	compon3d[][][], num_of_tris[][][], and tris[][][]
*	for an INTERFACE relative to a grid.
*
*	Returns 1 if successful or 0 on error (insufficient space).
*/


EXPORT boolean make_tri_comp_lists(
	INTERFACE	*intfc)
{
	DEBUG_ENTER(make_tri_comp_lists)

	if (no_topology_lists(intfc) == YES)
	{
	    screen("ERROR in make_tri_comp_lists(), "
		   "illegal attempt to construct interface topology\n"
		   "no_topology_lists(intfc) == YES\n");
	    clean_up(ERROR);
	}

	if (make_tri_lists(intfc) != YES)
	{
	    (void) printf("WARNING in make_tri_comp_lists(), "
	                  "make_tri_lists() failed\n");
	    DEBUG_LEAVE(make_tri_comp_lists)
	    return FUNCTION_FAILED;
	}
	intfc->modified = NO;
	intfc->table->new_grid = NO;

	DEBUG_LEAVE(make_tri_comp_lists)
	return FUNCTION_SUCCEEDED;
}		/*end make_tri_comp_lists*/




/*
*			make_tri_lists():
*
*	Computes the number of TRIS num_of_tris[iz][iy][ix] of INTERFACE
*	traversing each grid block of RECT_GRID, and constructs the
*	lists tris[iz][iy][ix] and surfaces[iz][iy][ix] of traversing TRIS
*	and SURFACES for each grid block:
*
*	Initializes  compon3d[iz][iy][ix]  to ONFRONT for blocks traversed
*	by INTERFACE and to the component value elsewhere.
*
*
*	Returns 1 if successful or 0 on error (if insufficient
*	storage is available, or there is a problem in rect_in_which()).
*
*
*				Details:
*
*	This is a three-pass construction over the INTERFACE with an
*	intermediate pass over the RECT_GRID.
*
*	On the first pass, the array tri_blocks is filled in along with
*	the number of tris in each mesh-square num_of_tris[][][].  This
*	is accomplished by calling the routine blocks_on_tri()
*	consecutively on each TRI of INTERFACE.
*
*	tri_blocks will contain for each TRI of INTERFACE the list
*	of (ix,iy,iz) integer triples for all grid blocks of the grid which are
*	traversed by TRI.   The end of the integer-triple list for a
*	TRI is marked with the integer  END_TRI.
*
*	The second pass, set_tri_list_pointers(), initializes the
*	pointers tris[iz][iy][ix] to point to the correct offsets in
*	the array tristore[] using the previously constructed
*	num_of_tris[][][] array.   Similarly for surfaces[iz][iy][ix].
*
*	The third pass, fill_tri_and_curve_lists(), is a loop over the
*	TRIS of INTERFACE.   For each TRI, the grid square information
*	previously stored in tri_blocks is transferred to the tris[][][]
*	array by adding TRI to tris[iz][iy][ix] for each (ix,iy,iz) triple
*	in the tri_block list for that TRI.   Similarly for surfaces[][][].
*
*	Storage Requirements:
*
*	Three arrays of unknown size must be allocated here: tristore,
*	surfacestore and tri_blocks.  To estimate the storage for tri_blocks,
*	we use the assumption that the tris are small relative to the mesh
*	spacing, so that as an upper bound (exact or probabilistic) we can
*	assume that the triangle only passes through adjacent blocks.  Thus
*	we want to know the number of orthants which are generically cut by
*	a hyperplane. In dimension d, this number is 1 + 2 + 2**2 + ...
*	+ 2**(d-1) = 2**d -1, and for d = 3, the number is 7.  Thus in the
*	worst case, a triangle meets 7 of the 8 contingous octants. The storage
*	requirement is 3 integers per block (octant) and one additional integer
*	to mark the end of the list of blocks. Thus the storage is bounded by
*	(3 * 7 + 1) * num_tris = 22 * num_tris.  The corresponidng two
*	dimensional estimate is (2 * 3 + 1) * num_tris = 7 * num_tris. On a
*	probabilistic basis, a considerably smaller amount of storage would do.
*
*	Estimates of tristore and surfacestore are not needed, as the
*	num_of_tris[][][] and num_of_surfaces[][][] are computed exactly
*	before this storage is needed, giving exact values for the tristore and
*	surfacestore.
*/


enum { END_TRI = -1 };	/* Any integer that cannot be a grid block index */
/*
*			mark_end_of_tri():
*
*	Marks the end of the grid blocks for a TRI in the
*	tri_blocks array with a marker (the negative integer
*	END_TRI).   This allows the tri_blocks array to be
*	reprocessed later TRI by TRI.
*
*	Pass tri_blocks as macro argument.
*/

#define  mark_end_of_tri(tblocks)	 *((tblocks)++) = END_TRI
#define  end_tri(tblocks)		((tblocks) == END_TRI)

LOCAL	int total_num_of_tri_blocks;	/* sum of num_of_tris[iz][iy][ix] */
LOCAL	int *Tri_blocks = NULL;	/* Temporary Array */
LOCAL	int *tri_blocks = NULL;	/* Points to temporary array */
LOCAL	int tri_blocks_size, tri_blocks_top;

LOCAL  boolean is_point_inside_box(
	double          *coords,
	const double    *VL,
	const double    *VU)
{
int	i;

	for(i = 0; i < 3; i++)
	{
	    if(coords[i] < VL[i] || coords[i] > VU[i])
	        return  NO;
	}
	return YES;
}

/*the tri t is outside the expanded grid. */
EXPORT boolean is_tri_outside(INTERFACE *intfc, TRI *t, RECT_GRID *grid)
{
	const double *h = grid->h;
	const double *VL = grid->VL, *VU = grid->VU;
	int	    i, j;
	double	   *p1 = Coords(Point_of_tri(t)[0]);
	double	   *p2 = Coords(Point_of_tri(t)[1]);
	double	   *p3 = Coords(Point_of_tri(t)[2]);
	double	   p[3];
	int	   ib[4];
	static const double SHIFT = 1.0e-6; /* TOLERANCE see rect_in_which */

	/*blocks_on_grid_based_tri */
	if(interface_reconstructed(intfc))
	{
	    for (i = 0; i < 3; ++i)
	        p[i] = (p1[i] + p2[i] + p3[i])/3.0;
	    if (is_point_inside_box(p, VL, VU))
	        return NO;
	    return YES;
	}

	/*WARN  this case is not changed, SHIFT is very large. */
	/*grid free case */
	for(i=0; i<3; i++)
	{
	    for(j=0; j<3; j++)
	        if(Coords(Point_of_tri(t)[j])[i] >= VL[i]-SHIFT*h[i])
		    break;
	    if(j==3)
	        return YES;
	    
	    for(j=0; j<3; j++)
	        if(Coords(Point_of_tri(t)[j])[i] <= VU[i]+SHIFT*h[i])
		    break;
	    if(j==3)
	        return YES;
	}
	return NO;
}

EXPORT  boolean is_tri_outside_box(TRI *t, double **fbox)
{
	int    i, j;

	for(i=0; i<3; i++)
	{
	    for(j=0; j<3; j++)
	        if(Coords(Point_of_tri(t)[j])[i] >= fbox[0][i])
		    break;
	    if(j==3)
	        return YES;
	    
	    for(j=0; j<3; j++)
	        if(Coords(Point_of_tri(t)[j])[i] <= fbox[1][i])
		    break;
	    if(j==3)
	        return YES;
	}
	return NO;
}

EXPORT boolean    is_outside_surface(
		INTERFACE  *intfc, 
		SURFACE    *s, 
		RECT_GRID  *gr)
{
TRI        *t;

	for(t=first_tri(s); !at_end_of_tri_list(t, s); t=t->next)
	{
	    if(!is_tri_outside(intfc, t, gr))
	        return NO;
	}
	return YES;
}

/*all surfaces are outside the domain */
EXPORT boolean    is_outside_surfaces(INTERFACE *intfc, RECT_GRID *gr)
{
	SURFACE    **s;

	for(s=intfc->surfaces; s && *s; s++)
	{
	    if(!is_outside_surface(intfc, *s, gr))
	        return NO;
	}
	return YES;
}

/*make_tri_lists will fail if the entire surface is outside the domain. */
EXPORT  void    delete_outside_surface(INTERFACE *intfc)
{
	SURFACE    **s, *surf;
	CURVE      **c;
	TRI        *t;
	boolean       found;
	RECT_GRID  *top_grid = &topological_grid(intfc);
	BOND_TRI   **btris;

	reset_wall_flag_for_surface(intfc);

	/*set outside surface flag */
	/*1. for curve related surfaces.  */
	for(c=intfc->curves; c && *c; c++)
	{
	    found = NO;
	    for(btris=Btris((*c)->first); btris && *btris; btris++)
	    {
		surf = (*btris)->surface;
		
		/*is an inside surface or the surface can not be deleted */
	        if(!is_outside_surface(intfc, surf, top_grid) || 
			surf->number == INSIDE_DOMAIN)
		    found = YES;
	    }

	    /*found == YES:  no surface can be deleted */
	    /*found == NO:   all surfaces can be deleted */
	    
	    for(btris=Btris((*c)->first); btris && *btris; btris++)
	    {
		(*btris)->surface->number = found==YES ? INSIDE_DOMAIN : 
			OUTSIDE_DOMAIN;
	    }
	}

	/*2. for no curve related surfaces */
	for(s=intfc->surfaces; s && *s; s++)
	{
	    /*ignore the tested surfaces. */
	    if((*s)->number == INSIDE_DOMAIN || (*s)->number == OUTSIDE_DOMAIN)
	        continue;
		
	    if(is_outside_surface(intfc, *s, top_grid))
	        (*s)->number = OUTSIDE_DOMAIN;
	}

de_out_s:
	for(s=intfc->surfaces; s && *s; s++)
	{
	    if((*s)->number == OUTSIDE_DOMAIN)
	    {
	        delete_scn(*s);
		goto de_out_s;
	    }
	}

	/*restore the wall flag */
	set_wall_flag_for_surface(intfc);
}

void	set_sort_tol(RECT_GRID  *);
void	sort_tris_on_blocks(INTERFACE	*);

LIB_LOCAL boolean make_tri_lists(
	INTERFACE	*intfc)
{
	COMPONENT    **compiz, **compizL, **compizU;
	COMPONENT    *compiziy, *compiziyL, *compiziyU;
	RECT_GRID    *top_grid;
	SURFACE	     **s;
	TRI	     *t;
	boolean	     status;
	double	     *h, hmintol;
	int	     i, size;
	int	     ix, iy, iz, max_size, out_cnt, dim;
	register int xmax, ymax, zmax;
	struct Table *T;

	DEBUG_ENTER(make_tri_lists)

	if (no_topology_lists(intfc) == YES)
	{
	    screen("ERROR in make_tri_lists(), "
		   "illegal attempt to construct interface topology\n"
		   "no_topology_lists(intfc) == YES\n");
	    clean_up(ERROR);
	}

	start_clock("make_tri_lists");

	if ((T = table_of_interface(intfc)) == NULL)
	{
	    stop_clock("make_tri_lists");
	    DEBUG_LEAVE(make_tri_lists)
	    return NO; 
	}

	/* Free old storage */

	if (T->num_of_tris != NULL)
	{
	    free(T->num_of_tris);
	    T->num_of_tris = NULL;
	}
	if (T->tristore != NULL)
	{
	    free(T->tristore);
	    T->tristore = NULL;
	}
	if (T->surfacestore != NULL)
	{
	    free(T->surfacestore);
	    T->surfacestore = NULL;
	}
	if (T->compon3d != NULL)
	{
	    free(T->compon3d);
	    T->compon3d = NULL;
	}
	if (T->tris != NULL)
	{
	    free(T->tris);
	    T->tris = NULL;
	}
	if (T->surfaces != NULL)
	{
	    free(T->surfaces);
	    T->surfaces = NULL;
	}

	/* Create a Grid if Needed: */

	if (!T->fixed_grid)
	    set_topological_grid(intfc,NULL);

	top_grid = &topological_grid(intfc);

	/* Set tolerance for tri lists */

	set_tri_list_tolerance(top_grid);

	/* Allocate New num_of_tris[][][], compon3d[][][]: */

	xmax = top_grid->gmax[0];
	ymax = top_grid->gmax[1];
	zmax = top_grid->gmax[2];

	tri_array(&T->num_of_tris,zmax,ymax,xmax,INT);
	if (T->num_of_tris == NULL)
	{
	    stop_clock("make_tri_lists");
	    DEBUG_LEAVE(make_tri_lists)
	    return NO;
	}
	/* NOTE: 
	 * tri_array returns data initialized to 0, so
	 * T->num_of_tris[k][j][i] = 0 for all i, j, k initially
	 */
	tri_array(&T->compon3d,zmax,ymax,xmax,sizeof(COMPONENT));
	if (T->compon3d == NULL)
	{
	    stop_clock("make_tri_lists");
	    DEBUG_LEAVE(make_tri_lists)
	    return NO;
	}

	/* Initialize all blocks to NO_COMP */

	for (iz = 0; iz < zmax; ++iz)
	    for (compiz = T->compon3d[iz], iy = 0; iy < ymax; ++iy)
	    	for (compiziy = compiz[iy], ix = 0; ix < xmax; ++ix)
	    	    compiziy[ix] = NO_COMP;

	/* Set rect boundary edge values */

	compizL = T->compon3d[0];
	compizU = T->compon3d[zmax-1];
	for (iy = 0; iy < ymax; ++iy)
	{
	    compiziyL = compizL[iy];
	    compiziyU = compizU[iy];
	    for (ix = 0; ix < xmax; ++ix)
	    {
	    	compiziyL[ix] = ON_RECT_BOUNDARY;
	    	compiziyU[ix] = ON_RECT_BOUNDARY;
	    }
	}
	for (iz = 1; iz < zmax-1; ++iz)
	{
	    compiz = T->compon3d[iz];
	    compiziyL = compiz[0];
	    compiziyU = compiz[ymax-1];
	    for (ix = 0; ix < xmax; ++ix)
	    {
	    	compiziyL[ix] = ON_RECT_BOUNDARY;
	    	compiziyU[ix] = ON_RECT_BOUNDARY;
	    }
	    for (iy = 1; iy < ymax-1; ++iy)
	    {
	    	compiziy = compiz[iy];
	    	compiziy[0] = ON_RECT_BOUNDARY;
	    	compiziy[xmax-1] = ON_RECT_BOUNDARY;
	    }
	}

	/* Find Length of and Allocate Tri_blocks array: */

	dim = intfc->dim;
	h = top_grid->h;
	max_size = 0;
	out_cnt = 0;
	hmintol = blk_tol*min3(h[0], h[1], h[2]);
	
	for (s = intfc->surfaces; s && *s; ++s)
	{
	    for (t = first_tri(*s); !at_end_of_tri_list(t,*s); t = t->next)
	    {
	        double *crds0 = Coords(Point_of_tri(t)[0]);
	        double *crds1 = Coords(Point_of_tri(t)[1]);
	        double *crds2 = Coords(Point_of_tri(t)[2]);

	        if(is_tri_outside(intfc, t, top_grid))
		{
		    max_size += 2;
		    out_cnt += 2;
		    continue;
		}

	        size = 1;
	        for (i = 0; i < dim; ++i)
	        {
	    	    double a = crds0[i], b = crds1[i], c = crds2[i];

	    	    size *= 2 + (int)( (max3(a, b, c) - min3(a, b, c) + 
		    		      4.0*hmintol)/h[i]);
	        }
	        max_size += 3 * size + 1;
	    }
	}

	if (max_size != out_cnt)
	{
	    uni_array(&Tri_blocks,max_size,INT);
	    if (Tri_blocks == NULL)
	    {
	    	stop_clock("make_tri_lists");
	    	DEBUG_LEAVE(make_tri_lists)
	    	return NO;
	    }

	    tri_blocks = Tri_blocks;
	    tri_blocks_size = max_size;
	    tri_blocks_top = 0;

	    /* Fill the num_of_tris[][][] and Tri_square arrays: */

	    /* Find and record all blocks on each tri: */

	    total_num_of_tri_blocks = 0;
	    
	    if (interface_reconstructed(intfc) == YES)
            {
                for (s = intfc->surfaces; s && *s; ++s)
                    for (t = first_tri(*s); !at_end_of_tri_list(t,*s); 
				t = t->next)
		    {
                        blocks_on_grid_based_tri(t,T->num_of_tris,
				T->compon3d,top_grid,intfc);
		    }
            }
            else
	    {
	        for (s = intfc->surfaces; s && *s; ++s)
	        {
	    	    for (t = first_tri(*s); !at_end_of_tri_list(t,*s); 
				t = t->next)
		    {
	                blocks_on_tri(t,T->num_of_tris,T->compon3d,
				top_grid,intfc);
		    }
	        }
	    }
	
	    /* Assign tris[][][] and surfaces[][][]: */
	    if ((status = set_tri_and_surface_list_pointers(T,xmax,ymax,zmax)))
	    {

	    	/* Copy in the grid blocks from Tri_blocks: */

	    	fill_tri_and_surface_lists(T->num_of_tris,T->tris,T->surfaces,
	    			       intfc);
		set_sort_tol(top_grid);
		sort_tris_on_blocks(intfc);
	    	/* Set the compon3d[][] array: */

	    	set_off_front_comp3d(intfc);
	    }
	    free(Tri_blocks);
	}
	else
	{
	    for (iz = 0; iz < zmax; ++iz)
	    {
	    	for (iy = 0; iy < ymax; ++iy)
		{
	    	    for (ix = 0; ix < xmax; ++ix)
		    {
	    	    	T->num_of_tris[iz][iy][ix] = 0;
	    	    	T->compon3d[iz][iy][ix] = intfc->default_comp;
		    }
		}
	    }
	    status = YES;
	}

	if (DEBUG)
	{
	    show_COMP_3d(stdout,intfc);
	    show_TRI_list(intfc);
	    points_of_interface(intfc);
	    print_interface(intfc);
	}
	stop_clock("make_tri_lists");
	DEBUG_LEAVE(make_tri_lists)
	return status;
}		/*end make_tri_lists*/

EXPORT  void assign_tri_icoords(
	RECT_GRID *grid,
	TRI *tri)
{
	double center[3];
	int i;

	for (i = 0; i < 3; ++i)
	{
	    center[i] = (Coords(Point_of_tri(tri)[0])[i] +
	                 Coords(Point_of_tri(tri)[1])[i] +
	                 Coords(Point_of_tri(tri)[2])[i])/3.0;
	}
	Tri_icoords(tri)[0] = cell_index(center[0],0,grid);
	Tri_icoords(tri)[1] = cell_index(center[1],1,grid);
	Tri_icoords(tri)[2] = cell_index(center[2],2,grid);
}     /* end assign_tri_icoords */


/*
*			set_tri_and_surface_list_pointers():
*
*	Does what its name implies! Read documentation above for the
*	function make_tri_list().
*	Returns 1 if successful, or 0 if unable to allocate enough
*	space.
*/

LOCAL boolean set_tri_and_surface_list_pointers(
	struct Table	*T,
	int		xmax,
	int		ymax,
	int		zmax)
{
	int		ix, iy, iz;
	int		***num_tris, **num_tris_iz, *num_tris_iziy;
	TRI		**last_t;
	TRI		*****tris, ****tris_iz, ***tris_iziy;
	SURFACE		**last_s;
	SURFACE		*****surfs, ****surfs_iz, ***surfs_iziy;

	DEBUG_ENTER(set_tri_and_surface_list_pointers)

	/* Allocate tris and surfaces arrays: */

	tri_array(&T->tris,zmax,ymax,xmax,sizeof(TRI **));
	if (T->tris == NULL)
	{ 
	    DEBUG_LEAVE(set_tri_and_surface_list_pointers)
	    return NO; 
	}

	tri_array(&T->surfaces, zmax, ymax, xmax, sizeof(SURFACE **));
	if (T->surfaces == NULL)
	{ 
	    DEBUG_LEAVE(set_tri_and_surface_list_pointers)
	    return NO;
	}

	/* Allocate tristore, surfacestore arrays: */

	uni_array(&T->tristore, total_num_of_tri_blocks, sizeof(TRI *));
	if (T->tristore == NULL)
	{ 
	    DEBUG_LEAVE(set_tri_and_surface_list_pointers)
	    return NO;
	}

	uni_array(&T->surfacestore, total_num_of_tri_blocks, sizeof(SURFACE *));
	if (T->surfacestore == NULL)
	{ 
	    DEBUG_LEAVE(set_tri_and_surface_list_pointers)
	    return NO;
	}

	last_t = T->tristore;
	last_s = T->surfacestore;
	tris = T->tris;
	surfs = T->surfaces;
	num_tris = T->num_of_tris;
	/*
	for (iz = 0; iz < zmax; ++iz)
	{
	    num_tris_iz = num_tris[iz];
	    tris_iz = tris[iz];
	    surfs_iz = surfs[iz];
	    for (iy = 0; iy < ymax; ++iy)
	    {
	    	num_tris_iziy = num_tris_iz[iy];
	    	tris_iziy = tris_iz[iy];
	    	surfs_iziy = surfs_iz[iy];
	    	for (ix = 0; ix < xmax; ++ix)
	    	{
	    	    tris_iziy[ix] = last_t;
	    	    surfs_iziy[ix] = last_s;
	    	    last_t += num_tris_iziy[ix];
	    	    last_s += num_tris_iziy[ix];
	    	    num_tris_iziy[ix] = 0;
	    	}
	    }
	}
	*/
	for (iz = 0; iz < zmax; ++iz)
	for (iy = 0; iy < ymax; ++iy)
	for (ix = 0; ix < xmax; ++ix)
	{
	    tris[iz][iy][ix] = last_t;
	    surfs[iz][iy][ix] = last_s;
	    last_t += num_tris[iz][iy][ix];
	    last_s += num_tris[iz][iy][ix];
	    num_tris[iz][iy][ix] = 0;
	}

	DEBUG_LEAVE(set_tri_and_surface_list_pointers)
	return YES;
}		/*end set_tri_and_surface_list_pointers*/

LOCAL double	sort_tol = 0.0;
void	set_sort_tol(RECT_GRID  *);

void set_sort_tol(RECT_GRID  *gr)
{
	double	*h = gr->h;

	sort_tol = 1.0e-7*min3(h[0], h[1], h[2]);
}

LOCAL	int compare_tris(const void *a, const void *b)
{
	TRI	**a1=(TRI**)a, **b1=(TRI**)b;
	TRI	*t1 = *a1, *t2 = *b1;
	double	cen1[3], cen2[3];
	int	i;

	centroid_of_tri(cen1, t1);
	centroid_of_tri(cen2, t2);

	for(i=0; i<3; i++)
	    if(fabs(cen1[i]-cen2[i]) < sort_tol)
		continue;
	    else if(cen1[i] < cen2[i])
		return -1;
	    else
		return 1;

	return 0;
}

void	sort_tris_on_blocks(INTERFACE	*);

/*sort tris in each block according to the positions of the centroids of tris, make sure different */
/*procs will have the same tris. */
void	sort_tris_on_blocks(INTERFACE	*intfc)
{
	RECT_GRID	*top_grid = &topological_grid(intfc);
	struct Table	*T;
	TRI		**tris;
	SURFACE		**surfs;
	int		ix, iy, iz, i, nt, xmax, ymax, zmax;
	
	T = table_of_interface(intfc);
	xmax = top_grid->gmax[0];
	ymax = top_grid->gmax[1];
	zmax = top_grid->gmax[2];

	for(iz = 0; iz < zmax; ++iz)
	{
	    for(iy = 0; iy < ymax; ++iy)
	    {
		for(ix = 0; ix < xmax; ++ix)
		{
		    nt = T->num_of_tris[iz][iy][ix];
		    if(nt == 0)
			continue;
		    tris = T->tris[iz][iy][ix];
		    surfs = T->surfaces[iz][iy][ix];

		    qsort((POINTER)tris, nt, sizeof(TRI*), compare_tris);
		    for(i=0; i<nt; i++)
			surfs[i] = tris[i]->surf;
		}
	    }
	}
}

/*
*		fill_tri_and_surface_lists():
*
*	Does what its name implies!  Read the documentation above
*	for function  make_tri_lists().
*/

LOCAL void fill_tri_and_surface_lists(
	int		***num_of_tris,
	TRI		*****tris,
	SURFACE		*****surfaces,
	INTERFACE	*intfc)
{
	int		ix, iy, iz;
	TRI		*t;
	SURFACE		**s;

	DEBUG_ENTER(fill_tri_and_surface_lists)
	tri_blocks = Tri_blocks;

	for (s = intfc->surfaces; s && *s; ++s)
	    for (t = first_tri(*s); !at_end_of_tri_list(t,*s); t = t->next)
	{
	    while(!end_tri(*tri_blocks))
	    {
	    	int num_tris;
	    	ix = *(tri_blocks++);
	    	iy = *(tri_blocks++);
	    	iz = *(tri_blocks++);
	    	num_tris = num_of_tris[iz][iy][ix]++;
	    	tris[iz][iy][ix][num_tris] = t;
	    	surfaces[iz][iy][ix][num_tris] = *s;
	    }
	    ++tri_blocks;	/* Skip END_TRI */
	}
	
	DEBUG_LEAVE(fill_tri_and_surface_lists)
}		/*end fill_tri_and_surface_lists*/


/*
*				Store_tri():
*
*	Adds a new icoords block entry to the tri_blocks array.
*	Also increments num_of_tris[iz][iy][ix] and sets compon3d[iz][iy][ix]
*	to ONFRONT.
*
*	Pass tri_blocks, total_num_of_tri_blocks, and comp as macro arguments.
*
*	NOTE:
*	The line (num_of_tris)[iz][iy][ix]++; here seems to be redundant,
*	as this end is accomplished in fill_tri_and_surface_lists.
*	This may be TRUE only for the context I studied: I tried to
*	eliminate this line but the run crashed; needs more study. --pinezich
*/

#define Store_tri(ic,tri_blocks,total_num_of_tri_blocks,comp)		\
{									\
	int ix = (ic)[0], iy = (ic)[1], iz = (ic)[2];			\
	*((tri_blocks)++) = ix;						\
	*((tri_blocks)++) = iy;						\
	*((tri_blocks)++) = iz;						\
	++(total_num_of_tri_blocks);					\
	++(num_of_tris)[iz][iy][ix];					\
	(comp)[iz][iy][ix] = ONFRONT;					\
}


/*
*			blocks_on_tri():
*			blocks_on_grid_based_tri():
*
*	Constructs a list of all grid blocks that intersect with the
*	triangle t.  For each grid block discovered to overlap with the
*	the triangle t,  t is added to the array of triangles that
*	intersect with that grid block.  This array is indexed using
*	the natural indices associated with the rectangular grid.
*
*	The function blocks_on_grid_based_tri computes the same list but
*	is specialized to a grid reconstructed interface.
*/
/*hmintol should be less than gfac(=4e-3, defined in set_floating_point_tolerance1) */
/*because the cutting surfaces in the scatter_front part.  */
/*cut_buf_interface1 */
/*    open_null_sides1 */
/*	 tri_out_domain1 */

LOCAL void blocks_on_tri(
	TRI	  *t,
	int	  ***num_of_tris,
	COMPONENT ***comp,
	RECT_GRID *grid,
	INTERFACE *intfc)
{
	FT_POINT	**p;
	int	i,j,k;
	int	i_diff[3], inc_size;
	int	ima[3][3], imi[3][3], imin[3], imax[3], ib[3];
	int	*gmax, *lbuf, *ubuf;
	double	hmintol, ptmp[3];

	gmax = grid->gmax;
	lbuf = grid->lbuf;
	ubuf = grid->ubuf;
	hmintol = blk_tol*min3(grid->h[0], grid->h[1], grid->h[2]);

	/* if a point is on a face of a block, make sure it belongs to both blocks. */
	
	p = Point_of_tri(t);
	for(i=0; i<3; i++)
	{
	    for(j=0; j<3; j++)
		ptmp[j] = Coords(p[i])[j] + hmintol;
	    rect_in_which(ptmp, ima[i], grid);
 
	    for(j=0; j<3; j++)
		ptmp[j] = Coords(p[i])[j] - hmintol;
	    rect_in_which(ptmp, imi[i], grid);
	}

	/*determine the bound box for the tri */
	for (i = 0; i < 3; ++i)
	{
	    imin[i] = min3(imi[0][i],imi[1][i],imi[2][i]);
	    if (imin[i] < -lbuf[i]) 
	    	imin[i] = -lbuf[i];
	    else if (imin[i] >= gmax[i] + ubuf[i]) 
	    	imin[i] = gmax[i] + ubuf[i] - 1;
	    
	    imax[i] = max3(ima[0][i],ima[1][i],ima[2][i]);
	    if (imax[i] < -lbuf[i]) 
	    	imax[i] = -lbuf[i];
	    else if (imax[i] >= gmax[i] + ubuf[i]) 
	    	imax[i] = gmax[i] + ubuf[i] - 1;
	    
	    i_diff[i] = imax[i] - imin[i] + 1;
	}
	
	if(is_tri_outside(intfc, t, grid))
	    for(i=0; i<3; ++i)
		i_diff[i] = 0;

	if (i_diff[0] > 10)
	{
	    print_tri_coords(t);
	    print_int_vector("i_diff = ", i_diff, 3, "\n");
	}

	/*check if the array tri_blocks is out of range. */
	tri_blocks_top += 3*i_diff[0]*i_diff[1]*i_diff[2] + 1;
	if(tri_blocks_top > tri_blocks_size)
	{
	    print_tri(t,intfc);
	    printf("ERROR blocks_on_tri, too many blocks, %d  %d\n",
	    	   tri_blocks_top, tri_blocks_size);
	    clean_up(ERROR);
	}

	if(the_tri(t) && NO)
	{
	    printf("#tri found.\n");
	    print_tri(t,intfc);
	    print_int_vector("i_diff", i_diff, 3, "\n");
	    print_int_vector("imin", imin, 3, "\n");
	    print_int_vector("imax", imax, 3, "\n");
	    printf("posn  %24.15e %24.15e  %24.15e\n", 
		grid->L[2] + imin[2]*grid->h[2],
		grid->L[2] + (imax[2]+1)*grid->h[2], hmintol);
	    print_rectangular_grid(grid);
	}
	
	for (i = 0; i < i_diff[0]; ++i)
	{
	    for (j = 0; j < i_diff[1]; ++j)
	    {
	    	for (k = 0; k < i_diff[2]; ++k)
	    	{
	    	    ib[0] = imin[0] + i;
	    	    ib[1] = imin[1] + j;
	    	    ib[2] = imin[2] + k;
		    if (ib[0] < 0 || ib[0] >= gmax[0]) continue;
		    if (ib[1] < 0 || ib[1] >= gmax[1]) continue;
		    if (ib[2] < 0 || ib[2] >= gmax[2]) continue;
	    	    Store_tri(ib,tri_blocks,total_num_of_tri_blocks,comp);
	    	}
	    }
	}
	mark_end_of_tri(tri_blocks);
}		/*end blocks_on_tri*/

LOCAL void blocks_on_grid_based_tri(
	TRI		*t,
	int		***num_of_tris,
	COMPONENT	***comp,
	RECT_GRID	*grid,
	INTERFACE	*intfc)
{
	int	   i,num_bls;
	int	   ib[4];
	double	   *L = grid->L;
	double	   *h = grid->h;
	double	   *p1 = Coords(Point_of_tri(t)[0]);
	double	   *p2 = Coords(Point_of_tri(t)[1]);
	double	   *p3 = Coords(Point_of_tri(t)[2]);
	double	   p[3];
	static int **blocks = NULL;	 /* block index vector */
	static int MAX_BLS = 64; /* allocated number of rows in blocks */

	if (blocks == NULL)
	    bi_array(&blocks,MAX_BLS,3,INT);

	num_bls = 0;
	for (i = 0; i < 3; ++i)
	    p[i] = (p1[i] + p2[i] + p3[i])/3.0;
	if (rect_in_which(p,ib,grid) && !is_tri_outside(intfc,t,grid))
	    ib[3] = 1;
	else
	{
	    mark_end_of_tri(tri_blocks);
	    return;
	}

	for (i = 0; i < 3; ++i)
	{
	    if (ib[i] < 0 || ib[i] >= grid->gmax[i])
	    {
		mark_end_of_tri(tri_blocks);
            	return;
	    }
	}
    	Store_tri(ib,tri_blocks,total_num_of_tri_blocks,comp);
	blocks = add_to_bls_list(blocks,ib,&num_bls,&MAX_BLS);
	for (i = 0; i < 3; ++i)
	{
	    if (p1[i] == p2[i] && p1[i] == p3[i])
	    {
		if (fabs(p1[i] - (L[i] + ib[i]*h[i])) < ctol)
		{
		    --ib[i];
		    /*if (ib[i] < -grid->lbuf[i])*/
		    if (ib[i] < 0)
			continue;
    		    Store_tri(ib,tri_blocks,total_num_of_tri_blocks,comp);
		    blocks = add_to_bls_list(blocks,ib,&num_bls,&MAX_BLS);
		}
		else if (fabs(p1[i] - (L[i] + (ib[i] + 1)*h[i])) < ctol)
		{
		    ++ib[i];
		    /*if (ib[i] >= grid->gmax[i] + grid->ubuf[i])*/
		    if (ib[i] >= grid->gmax[i])
			continue;
    		    Store_tri(ib,tri_blocks,total_num_of_tri_blocks,comp);
		    blocks = add_to_bls_list(blocks,ib,&num_bls,&MAX_BLS);
		}
	    }
	}

	mark_end_of_tri(tri_blocks);
}		/*end blocks_on_grid_based_tri*/

EXPORT boolean within_tri(
	const double *p,
	const double *p1,
	const double *p2,
	const double *p3,
	const double *N,
	double	    tol)
{
	double	v1[3],v2[3],v3[3];
	double	v21[3], v13[3], v32[3];
	double	l1,l2,l3,l21,l13,l32;
	double	D1, D2, D3;
	int 	i;

	for (i = 0; i < 3; ++i)
	{
	    v21[i] = p2[i] - p1[i];
	    v13[i] = p1[i] - p3[i];
	    v32[i] = p3[i] - p2[i];
	    v1[i]  = p[i]  - p1[i];
	    v2[i]  = p[i]  - p2[i];
	    v3[i]  = p[i]  - p3[i];
	}

	l21 = sqrt(Dot3d(v21,v21));
	l13 = sqrt(Dot3d(v13,v13));
	l32 = sqrt(Dot3d(v32,v32));
	l1  = sqrt(Dot3d(v1,v1));
	l2  = sqrt(Dot3d(v2,v2));
	l3  = sqrt(Dot3d(v3,v3));

	if (l1 == 0.0 || l2 == 0.0 || l3 == 0.0)
	{
	    return YES;
	}

	for (i = 0; i < 3; ++i)
	{
	    v21[i] /= l21;
	    v13[i] /= l13;
	    v32[i] /= l32;
	    v1[i]  /= l1;
	    v2[i]  /= l2;
	    v3[i]  /= l3;
	}

	D1 = Det3d(v1,v21,N);
	D2 = Det3d(v2,v32,N);
	D3 = Det3d(v3,v13,N);
	
	if(debugging("line_tri"))
	{
	    printf("#D %24.16e  %24.16e  %24.16e\n", D1, D2, D3);
	}

	return ((D1 <= tol) && (D2 <= tol) && (D3 <= tol)) ? YES : NO;
}		/*end within_tri*/

/*
*		block_dimension():
*
*	The block dimension of the vector i_diff gives the dimension
*	of the smallest rectangular array of grid blocks that contain
*	the triangle whose vertices were used to compute i_diff. It also
*	checks whether the triangle can fit within a single 2x2x2
*	array of grid blocks,  if not then the dimension is defined to
*	be 4. In particular we have:
*
*	block_dimension (1,1,1) = 0,  triangle lies within a single grid block.
*	block_dimension (2,1,1), (1,2,1), (1,1,2) = 1, triangle lies within 
*		a pair of adjacent grid blocks.
*	block_dimension (2,2,1), (1,2,2), (2,1,2) = 2, triangle lies within 
*		a 2x2 group of coplanar grid blocks.
*	block_dimension (2,2,2) = 3,  triangle lies within a 2x2x2 group of
*		adjacent grid blocks,  and does not lie entirely within 
*		any 2x2 coplanar subset of this group.
*/

LOCAL int block_dimension(
	int		*i_diff)
{
	int		i,i1,i2;
	int		dimen;

	i1 = i2 = 0;
	for (i = 0; i < 3; ++i)
	{
	    if (i_diff[i] == 1)
	        ++i1;
	    if (i_diff[i] == 2)
	        ++i2;
	}
	if (i1+i2 != 3)
	    dimen = 4;
	else if (i2 == 3)
	    dimen = 3;
	else if (i2 == 2)
	    dimen = 2;
	else if (i2 == 1)
	    dimen = 1;
	else
	    dimen = 0;

	return dimen;
}		/*end block_dimension*/

/*
*			block_recorded():
*
*	Boolean function that returns YES or NO depending on whether
*	the index ib has been recorded in the list of indices blocks.
*/

LOCAL boolean block_recorded(
	int		**blocks,
	int		*ib,
	int		*num_bls)
{
	int		i;

	for (i = 0; i < *num_bls; ++i)
	{
	    if ((ib[0] == blocks[i][0]) && (ib[1] == blocks[i][1]) &&
	        (ib[2] == blocks[i][2]))
	    {
	        return YES;
	    }
	}
	return NO;
}		/*end block_recorded*/


LOCAL int **add_to_bls_list(
	int **blocks,
	int *ib,
	int *num_bls,
	int *MAX_BLS)
{
	if (*num_bls == *MAX_BLS)
	{
	    int **new_blocks;
	    int i;

	    *MAX_BLS *= 2;
	    bi_array(&new_blocks,*MAX_BLS,3,INT);
	    for (i = 0; i < *num_bls; ++i)
	    {
	    	new_blocks[i][0] = blocks[i][0];
	    	new_blocks[i][1] = blocks[i][1];
	    	new_blocks[i][2] = blocks[i][2];
	    }
	    free(blocks);
	    blocks = new_blocks;
	}
	blocks[*num_bls][0] = ib[0];
	blocks[*num_bls][1] = ib[1];
	blocks[*num_bls][2] = ib[2];
	++(*num_bls);
	return blocks;
}		/*end add_to_bls_list*/


/* use unknown bdry as subdomain bdry */
LOCAL void set_off_front_comp3d(
	INTERFACE	*intfc)
{
	register int	ixmax, iymax, izmax;
	register int	ixmin, iymin, izmin;
	COMPONENT	c;
	COMPONENT	**compon_iz;
	COMPONENT	*compon_iziy;
	COMPONENT	***compon = intfc->table->compon3d;
	COMPONENT       **compk, *compj;
	RECT_GRID	*grid = &topological_grid(intfc);
	double           p[MAXD];
	int             ip[MAXD];
	int             i, j, k;
	int             kmin, jmin, imin;
	int             kmax, jmax, imax;
	int		xmax, ymax, zmax;
	int		ix, iy, iz;
	boolean		bdry_flag[3][2];

	DEBUG_ENTER(set_off_front_comp3d)

	ixmin = 0;		iymin = 0;		izmin = 0;
	xmax = grid->gmax[0];	ymax = grid->gmax[1];	zmax = grid->gmax[2];
	ixmax = xmax-1;		iymax = ymax-1;		izmax = zmax-1;

	/* deal with bdry types which do not have surfaces. */
	for(i=0; i<3; i++)
	    for(j=0; j<2; j++)
	        bdry_flag[i][j] = buffered_boundary_type(rect_boundary_type(intfc,i,j)) || 
		             rect_boundary_type(intfc,i,j) == UNKNOWN_BOUNDARY_TYPE;

	if (bdry_flag[0][0])
	    ++ixmin;
	if (bdry_flag[0][1])
	    --ixmax;
	if (bdry_flag[1][0])
	    ++iymin;
	if (bdry_flag[1][1])
	    --iymax;
	if (bdry_flag[2][0])
	    ++izmin;
	if (bdry_flag[2][1])
	    --izmax;

	/* Set remaining interior cells */
	for (iz = izmin; iz <= izmax; ++iz)
	{
	    compon_iz = compon[iz];
	    for (iy = iymin; iy <= iymax; ++iy)
	    {
	    	compon_iziy = compon_iz[iy];
	    	for (ix = ixmin; ix <= ixmax; ++ix)
	    	{
	    	    switch (compon_iziy[ix])
	    	    {
	    	    case ONFRONT:
		    case ON_RECT_BOUNDARY:
	    	        continue;
	    	    case NO_COMP:
	    	    {
	    	        ip[0] = ix; ip[1] = iy; ip[2] = iz;
	    	        p[0] = cell_center(ix,0,grid);
	    	        p[1] = cell_center(iy,1,grid);
	    	        p[2] = cell_center(iz,2,grid);
	    		compon_iziy[ix] = component_wrt_icoords3d(p,ip,intfc);
			
			if(debugging("init_comp"))
			{
			    printf("#pt0 %d \n", compon[14][iymax-9][9]);
			    printf("#init_comp %d %d %d %d\n", ix, iy, iz, compon_iziy[ix]);
			    show_COMP(stdout, intfc);
			}
	        	break;
	            }
	            default:
	            	break;
	            }
	            c = compon_iziy[ix];
		    kmin = iz;
		    kmax = min(izmax,iz+1);
		    jmin = iy;
		    jmax = min(iymax,iy+1);
		    imin = ix;
		    imax = min(ixmax,ix+1);

		    for (k = kmin; k <= kmax; ++k)
		    {
		      compk = compon[k];
		      for (j = jmin; j <= jmax; ++j)
		      {
	    	        compj = compk[j];
		        for (i = imin; i <= imax; ++i)
		        {
		          if (compj[i] == NO_COMP)
		          {
			    if(debugging("init_comp") && k == 14)
			    {
			        printf("#pt %d  %d  %d | %d  %d  %d | %d\n", 
				    imin, jmin, kmin, i, j, k, c);
			    }
			    compj[i] = c;
		          }
		        }
		      }
		    }
		    if(debugging("init_comp") && ( iz == 13 ))
		    {
		        printf("#izchk  %d \n", iz);
		        show_COMP(stdout, intfc);
	            }

		}
	    }
	}

	if(!bdry_flag[2][0])
	{
	    compon_iz = compon[0];
	    for (iy = 0; iy < ymax; ++iy)
	    {
	    	compon_iziy = compon_iz[iy];
	        for (ix = 0; ix < xmax; ++ix)
	        {
	    	    if (compon_iziy[ix] == ON_RECT_BOUNDARY)
			compon_iziy[ix] = ONFRONT;
	        }
	    }
	}
	if(!bdry_flag[2][1])
	{
	    compon_iz = compon[zmax-1];
	    for (iy = 0; iy < ymax; ++iy)
	    {
	    	compon_iziy = compon_iz[iy];
	        for (ix = 0; ix < xmax; ++ix)
	        {
	    	    if (compon_iziy[ix] == ON_RECT_BOUNDARY)
			compon_iziy[ix] = ONFRONT;
	        }
	    }
	}
	if(!bdry_flag[1][0])
	{
	    for (iz = 0; iz < zmax; ++iz)
	    {
	        compon_iz = compon[iz];
	    	compon_iziy = compon_iz[0];
	        for (ix = 0; ix < xmax; ++ix)
	        {
	    	    if (compon_iziy[ix] == ON_RECT_BOUNDARY)
			compon_iziy[ix] = ONFRONT;
	        }
	    }
	}
	if(!bdry_flag[1][1])
	{
	    for (iz = 0; iz < zmax; ++iz)
	    {
	        compon_iz = compon[iz];
	    	compon_iziy = compon_iz[ymax-1];
	        for (ix = 0; ix < xmax; ++ix)
	        {
	    	    if (compon_iziy[ix] == ON_RECT_BOUNDARY)
			compon_iziy[ix] = ONFRONT;
	        }
	    }
	}
	if(!bdry_flag[0][0])
	{
	    for (iz = 0; iz < zmax; ++iz)
	    {
	        compon_iz = compon[iz];
	        for (iy = 0; iy < ymax; ++iy)
		{
	    	    compon_iziy = compon_iz[iy];
	    	    if (compon_iziy[0] == ON_RECT_BOUNDARY)
			compon_iziy[0] = ONFRONT;
		}
	    }
	}
	if(!bdry_flag[0][1])
	{
	    for (iz = 0; iz < zmax; ++iz)
	    {
	        compon_iz = compon[iz];
	        for (iy = 0; iy < ymax; ++iy)
		{
	    	    compon_iziy = compon_iz[iy];
	    	    if (compon_iziy[xmax-1] == ON_RECT_BOUNDARY)
			compon_iziy[xmax-1] = ONFRONT;
		}
	    }
	}

	if(bdry_flag[0][0])
	    set_x_face_comps(0,intfc);
	if(bdry_flag[0][1])
	    set_x_face_comps(xmax-1,intfc);
	if(bdry_flag[1][0])
	    set_y_face_comps(0,intfc);
	if(bdry_flag[1][1])
	    set_y_face_comps(ymax-1,intfc);
	if(bdry_flag[2][0])
	    set_z_face_comps(0,intfc);
	if(bdry_flag[2][1])
	    set_z_face_comps(zmax-1,intfc);

	DEBUG_LEAVE(set_off_front_comp3d)
}		/*end set_off_front_comp3d*/


/*
*			set_x_face_comps():
*			set_y_face_comps():
*			set_z_face_comps():
*
*	These three functions are each loop over an appropriate face of
*	the toplogical grid cube,  setting the components of the grid
*	blocks at the extreme edges of the domain.  The three functions
*	are essentially the same,  but indexing makes it more convenenient
*	to write them separately.  These functions are necessary due to
*	the non-tracking of the faces of the topological domain at
*	subdomain boundaries.
*/

LOCAL	void	set_x_face_comps(
	int		ix,
	INTERFACE	*intfc)
{
	register int iymax, izmax;
	COMPONENT    c;
	COMPONENT    **compon_iz, **compon_izp1;
	COMPONENT    *compon_iziy;
	COMPONENT    ***compon = intfc->table->compon3d;
	RECT_GRID    *grid = &topological_grid(intfc);
	int	     iy, iz;
	int          ip[MAXD];

	iymax = grid->gmax[1]-1;	izmax = grid->gmax[2]-1;

	for (iz = 0; iz <=  izmax; ++iz)
	{
	    compon_iz = compon[iz];
	    compon_izp1 = (iz < izmax) ? compon[iz + 1] : NULL;
	    for (iy = 0; iy <= iymax; ++iy)
	    {
	        compon_iziy = compon_iz[iy];
	        switch (compon_iziy[ix])
	        {
	        case ONFRONT:
	            continue;
	        case NO_COMP:
		case ON_RECT_BOUNDARY:
	            ip[0] = ix; ip[1] = iy; ip[2] = iz;
	            c = set_comp_on_block(ip,intfc);
	            compon_iziy[ix] = c;
	            if (c == ONFRONT)
			continue;
	            break;
	        default:
	            break;
	        }
	        c = compon_iziy[ix];
	        if (iz < izmax && (compon_izp1[iy][ix] == NO_COMP))
    	            compon_izp1[iy][ix] = c;
	        if (iy < iymax && (compon_iz[iy+1][ix] == NO_COMP))
	            compon_iz[iy + 1][ix] = c;
	        if (iy < iymax && iz < izmax &&
	                (compon_izp1[iy + 1][ix] == NO_COMP))
	            compon_izp1[iy + 1][ix] = c;
	    }
	}
}		/*end set_x_face_comps*/

LOCAL	void	set_y_face_comps(
	int		iy,
	INTERFACE	*intfc)
{
	register int ixmax, izmax;
	COMPONENT    c;
	COMPONENT    **compon_iz, **compon_izp1;
	COMPONENT    *compon_iziy;
	COMPONENT    ***compon = intfc->table->compon3d;
	RECT_GRID    *grid = &topological_grid(intfc);
	int	    ix, iz;
	int         ip[MAXD];

	ixmax = grid->gmax[0]-2;	izmax = grid->gmax[2]-1;

	for (iz = 0; iz <=  izmax; ++iz)
	{
	    compon_iz = compon[iz];
	    compon_izp1 = (iz < izmax) ? compon[iz + 1] : NULL;
	    compon_iziy = compon_iz[iy];
	    for (ix = 1; ix <= ixmax; ++ix)
	    {
	        switch (compon_iziy[ix])
	        {
	        case ONFRONT:
	            continue;
	        case NO_COMP:
		case ON_RECT_BOUNDARY:
	            ip[0] = ix; ip[1] = iy; ip[2] = iz;
	            c = set_comp_on_block(ip,intfc);
	            compon_iziy[ix] = c;
	            if (c == ONFRONT)
			continue;
	            break;
	        default:
	            break;
	        }
	        c = compon_iziy[ix];
	        if (iz < izmax && (compon_izp1[iy][ix] == NO_COMP))
	            compon_izp1[iy][ix] = c;
	        if (ix < ixmax && (compon_iziy[ix + 1] == NO_COMP))
	            compon_iz[iy][ix + 1] = c;
	        if (iz < izmax && ix < ixmax &&
	                (compon_izp1[iy][ix + 1] == NO_COMP))
	            compon_izp1[iy][ix + 1] = c;
	    }
	}
}		/*end set_y_face_comps*/

LOCAL	void	set_z_face_comps(
	int		iz,
	INTERFACE	*intfc)
{
	register int ixmax, iymax;
	COMPONENT    c;
	COMPONENT    **compon_iz;
	COMPONENT    *compon_iziy;
	COMPONENT    ***compon = intfc->table->compon3d;
	RECT_GRID    *grid = &topological_grid(intfc);
	int	    ix, iy;
	int         ip[MAXD];

	ixmax = grid->gmax[0]-2;	iymax = grid->gmax[1]-2;

	compon_iz = compon[iz];
	for (iy = 1; iy <=  iymax; ++iy)
	{
	    compon_iziy = compon_iz[iy];
	    for (ix = 1; ix <= ixmax; ++ix)
	    {
	        switch (compon_iziy[ix])
	        {
	        case ONFRONT:
	            continue;
	        case NO_COMP:
		case ON_RECT_BOUNDARY:
	            ip[0] = ix; ip[1] = iy; ip[2] = iz;
	            c = set_comp_on_block(ip,intfc);
	            compon_iziy[ix] = c;
	            if (c == ONFRONT)
			continue;
	            break;
	        default:
	            break;
	        }
	        c = compon_iziy[ix];
	        if (iy < iymax && (compon_iz[iy+1][ix] == NO_COMP))
	            compon_iz[iy + 1][ix] = c;
	        if (ix < ixmax && (compon_iziy[ix + 1] == NO_COMP))
	            compon_iz[iy][ix + 1] = c;
	        if (ix < ixmax && iy < iymax &&
	                (compon_iz[iy + 1][ix + 1] == NO_COMP))
	            compon_iz[iy + 1][ix + 1] = c;
	    }
	}
}		/*end set_z_face_comps*/

/*
*			set_comp_on_block():
*
*	In 3d,  the fact that no surfaces are installed at subdomain or
*	periodic boundaries can cause FALSE reports of exterior
*	components or inconsistent components on the block.  Therefore when
*	seeking to set the component at a given location we must examine not
*	just that single grid block,  but all adjacent grid blocks.  If there
*	is any inconsistency in the components computed for these grid blocks,
*	then the grid block ip must be regarded as being ONFRONT even if
*	that block does not contain any surface elements.
*/


LOCAL COMPONENT set_comp_on_block(
	int		*ip,
	INTERFACE	*intfc)
{
	COMPONENT	c, c_last;
	COMPONENT	***compon;
	RECT_GRID	*grid;
	double		pn[3];
	int		xmax, ymax, zmax;
	int		ix, iy, iz;
	int		ixmin, ixmax, iymin, iymax, izmin, izmax;
	int		ipn[3];

	compon = intfc->table->compon3d;
	grid = &topological_grid(intfc);
	xmax = grid->gmax[0]; ymax = grid->gmax[1]; zmax = grid->gmax[2];
	izmin = ip[2]-1;	izmin = max(0,izmin);
	izmax = ip[2]+2;	izmax = min(zmax,izmax);
	iymin = ip[1]-1;	iymin = max(0,iymin);
	iymax = ip[1]+2;	iymax = min(ymax,iymax);
	ixmin = ip[0]-1;	ixmin = max(0,ixmin);
	ixmax = ip[0]+2;	ixmax = min(xmax,ixmax);
	c_last = NO_COMP;
	c = ONFRONT;
	for (iz = izmin; iz < izmax; ++iz)
	{
	    ipn[2] = iz;
	    pn[2] = cell_center(iz,2,grid);
	    for (iy = iymin; iy < iymax; ++iy)
	    {
	        ipn[1] = iy;
	        pn[1] = cell_center(iy,1,grid);
	        for (ix = ixmin; ix < ixmax; ++ix)
	        {
	            ipn[0] = ix;
	            pn[0] = cell_center(ix,0,grid);
	            switch (compon[iz][iy][ix])
	            {
	            case ONFRONT:
		    case ON_RECT_BOUNDARY:
	                continue;
	            case NO_COMP:
	                c = component_wrt_icoords3d(pn,ipn,intfc);
	                break;
	            default:
	                c = compon[iz][iy][ix];
	                break;
	            }
	            if (c_last == NO_COMP)
	        	c_last = c;
	            else if (c != c_last)
	            {
	        	return ONFRONT;
	            }
	        }
	    }
	}
	return c;
}		/*end set_comp_on_block*/


/*
*				show_COMP_3d():
*
*	Shows the COMPONENT values for Grid block of an INTERFACE.
*	The ONFRONT blocks are indicated by the word ON.
*/

LIB_LOCAL void show_COMP_3d(
	FILE		*file,
	INTERFACE	*intfc)
{
	COMPONENT comp;
	int	  ix, iy, iz;
	int	  ixmax, iymax, izmax;

	ixmax = topological_grid(intfc).gmax[0];
	iymax = topological_grid(intfc).gmax[1];
	izmax = topological_grid(intfc).gmax[2];
	(void) fprintf(file,"\n\nCOMPONENTS by Grid Block for "
	               "INTERFACE %llu = %p:\n",
	               interface_number(intfc),intfc);
	for (iz = 0; iz < izmax; ++iz)
	{
	    (void) fprintf(file, "\tPlane iz = %d\n\n", iz);
	    for (iy = iymax - 1; iy >= 0; --iy)
	    {
	        for (ix = 0; ix < ixmax; ++ix)
		{
		    
		    comp = intfc->table->compon3d[iz][iy][ix];
		    switch (comp)
		    {
		    case ONFRONT:
			(void) fprintf(file,"ON ");
			break;
		    case ON_RECT_BOUNDARY:
			(void) fprintf(file,"RB ");
			break;
		    default:
	                (void) fprintf(file,"%2d ",comp);
			break;
		    }
		}
	        (void) fprintf(file, "\n");
	    }
	    (void) fprintf(file, "\n\n");
	}
	(void) fprintf(file, "\n\n");
}		/*end show_COMP_3d*/



/*
*			show_TRI_list():
*
*		Displays the trilists of an INTERFACE:
*/

LOCAL void show_TRI_list(
	INTERFACE	*intfc)
{
	int		ix, iy, iz;
	int		ixmax, iymax, izmax;

	ixmax = topological_grid(intfc).gmax[0];
	iymax = topological_grid(intfc).gmax[1];
	izmax = topological_grid(intfc).gmax[2];

	(void) printf("\n\nTri Numbers for INTERFACE %llu:\n",
	              interface_number(intfc));
	for (iz = 0; iz < izmax; ++iz)
	{
	    (void) printf("\tPlane iz = %d\n\n", iz);
	    for (iy = iymax - 1; iy >= 0; --iy)
	    {
	        for (ix = 0; ix < ixmax; ++ix)
	            (void) printf("%3d ",intfc->table->num_of_tris[iz][iy][ix]);
	        (void) printf("\n");
	    }
	    (void) printf("\n\n");
	}
	(void) printf("\n\n");
}		/*end show_TRI_list*/

LOCAL	void set_tri_list_tolerance(RECT_GRID *rgr)
{
	double hmin;
	int i;

	hmin = HUGE_VAL;
	for (i = 0; i < 3; ++i)
            if (hmin > rgr->h[i])
	        hmin = rgr->h[i];

	/*crx_tol = hmin*1.0e-6;*/ 
	crx_tol = hmin*1.0e-10;/*TOLERANCE*/
	crx_tolv = sqr(crx_tol);
        /*crx_toll = 1.0e3*sqr(crx_tol); */
	crx_toll = 0.1*sqr(crx_tol);
	ctol = hmin*1.0e-5;
}		/*end set_tri_list_tolerance*/

LOCAL	COMPONENT comp_at_closest(
	TRI_PROJECTION *closest)
{
	double	    ip;
	const double *tnor;

	switch (closest->side)
	{
	case POSITIVE_SIDE:
	    return positive_component(closest->s);

	case NEGATIVE_SIDE:
	    return negative_component(closest->s);

	case ONVERTEX:
	case ONEDGE:
	    tnor = Tri_normal(closest->tri);
	    ip = Dot3d(closest->dpt,tnor);
	    if (ip > 0.0)
		return positive_component(closest->s);
	    if (ip < 0.0)
	        return negative_component(closest->s);
	    else
	    {
		if (debugging("comp3d"))
	            (void) printf("WARNING in comp_at_closest(), the component of "
		              "a point in the plane of a triangle is "
			      "undefined\n");
	        return negative_component(closest->s);/*ARBITRARY CHOICE*/
	    }

	case COPLANAR:
	    if (debugging("comp3d"))
	        (void) printf("WARNING in comp_at_closest(), the component of an "
		          "interior point of a triangle is undefined\n");
	    return negative_component(closest->s);/*ARBITRARY CHOICE*/

	case UNKNOWN_SIDE:
	default:
	    screen("ERROR in comp_at_closest(), unknown side\n");
	    return NO_COMP;
	    clean_up(ERROR);
	    return NO_COMP;
	}
}		/* end comp_at_closest */


EXPORT boolean tri_edge_crossing(
	TRI   *tri,
	double *crds_start,
	double *crds_crx,
	int   ic,
	int   *iv,
	int   *ie,
	double *h)
{
	FT_POINT	    **p;
	const double *n;
	double	    norm[MAXD], D;
	double	    crds_end[MAXD];
	int	    i,j;
	double	    *p1,*p2,d,pe_crx[MAXD],*pt;
	double	    crds_max[MAXD],crds_min[MAXD];

	p = Point_of_tri(tri);
	for (i = 0; i < 3; ++i)
	{
	    crds_min[i] = HUGE;
	    crds_max[i] = -HUGE;
	    crds_crx[i] = crds_end[i] = crds_start[i];
	}
	for (i = 0; i < 3; ++i)
	{
	    for (j = 0; j < 3; ++j)
	    {
	    	if (Coords(p[i])[j] < crds_min[j])
		    crds_min[j] = Coords(p[i])[j];
	    	if (Coords(p[i])[j] > crds_max[j])
		    crds_max[j] = Coords(p[i])[j];
	    }
	}
	if ((crds_min[(ic+1)%3] > crds_crx[(ic+1)%3] + crx_tol) ||
	    (crds_min[(ic+2)%3] > crds_crx[(ic+2)%3] + crx_tol) ||
	    (crds_max[(ic+1)%3] < crds_crx[(ic+1)%3] - crx_tol) ||
	    (crds_max[(ic+2)%3] < crds_crx[(ic+2)%3] - crx_tol))
	    return NO;
	
	n = Tri_normal(tri); 
	
	if(debugging("tst_crx"))
	    print_general_vector("n", n, 3, "\n");

	D = Mag3d(n);
	for (i = 0; i < 3; ++i)
	    norm[i] = n[i]/D;

	if (norm[ic] == 0.0)	/* edge parallel to tri */
	    return NO;

	*iv = ERROR;
	*ie = ERROR;
	crds_end[ic] = crds_start[ic] + h[ic];

	/* check for tri vertex on grid edge */

	for (i = 0; i < 3; ++i)
	{
	    pt = Coords(p[i]);
	    d = sqr(pt[(ic+1)%3] - crds_crx[(ic+1)%3])
	      + sqr(pt[(ic+2)%3] - crds_crx[(ic+2)%3]);
	
	    if (d < crx_tolv && crds_start[ic] <= pt[ic] && 
			crds_end[ic] > pt[ic])
	    {
	        crds_crx[ic] = Coords(p[i])[ic];
		*iv = i;
	    	if(debugging("tst_crx"))
	    	{
		    printf("iv = %d\n",iv);
		    print_general_vector("pt", pt, 3, "\n");
		    print_general_vector("start ", crds_start, 3, "\n");
		    print_general_vector("end   ", crds_end, 3, "\n");
		    print_general_vector("crds_c", crds_crx, 3, "\n");
		    printf(" d= %24.16e  crx_tolv = %24.16e \n", d, crx_tolv);
		}
	        return YES;
	    }
	}

	/* check for tri edge on grid edge */

	for (i = 0; i < 3; ++i)
	{
	    p1 = Coords(p[i]);
	    p2 = Coords(p[(i+1)%3]);

	    d = shortest_2line_dist_dir(ic,crds_start,p1,p2,crds_crx,pe_crx);
	    if (d > crx_toll)
		continue;
		
	    if(debugging("tst_crx"))
	    {
		print_general_vector("p1", p1, 3, "\n");
		print_general_vector("p2", p2, 3, "\n");
		print_general_vector("start ", crds_start, 3, "\n");
		print_general_vector("end   ", crds_end, 3, "\n");
		print_general_vector("crds_c", crds_crx, 3, "\n");
		printf(" d= %24.16e  crx_toll = %24.16e \n", d, crx_toll);
	    }

	    if ((crds_start[ic] <= crds_crx[ic] && crds_end[ic] > crds_crx[ic]) 
		&& within_interval(p1[0],p2[0],pe_crx[0])  
		&& within_interval(p1[1],p2[1],pe_crx[1])  
		&& within_interval(p1[2],p2[2],pe_crx[2])) 
	    {
	    	*ie = i;
	    	return YES;
	    }
	    else
		return NO;
	}
	
	/* check for tri interior on grid edge */
	D = 0.0;
	for (i = 0; i < 3; ++i)
	    D += norm[i]*Coords(p[0])[i];
	
	crds_crx[ic] = (D - (norm[(ic+1)%3]*crds_start[(ic+1)%3] +
			     norm[(ic+2)%3]*crds_start[(ic+2)%3]))/norm[ic];

	if(debugging("tst_crx"))
	{
	    print_general_vector("start", crds_start, 3, "\n");
	    print_general_vector("end", crds_end, 3, "\n");
	    print_general_vector("crx", crds_crx, 3, "\n");
	    print_general_vector("norm", norm, 3, "\n");
	    printf("D= %24.16e \n", D);
	}

	if ( crds_start[ic] <= crds_crx[ic] && crds_end[ic] > crds_crx[ic]  &&
	     within_tri(crds_crx,Coords(p[0]),Coords(p[1]),Coords(p[2]),norm,
					0.0))
	{
	    return YES;
	}

	return NO;
}		/*end tri_edge_crossing*/

/*point   crds  on the plane of tri. */
/*triangle  tri, norm is the positive orient of tri */
LOCAL  boolean point_tri_crossing(
	double *crx,
	int   *iv,
	int   *ie,
	double *crds,
	TRI   *tri,
	double *norm,
	double tol)
{
FT_POINT	**pt = Point_of_tri(tri);
double	*p0 = Coords(pt[0]), *p1 = Coords(pt[1]), *p2=Coords(pt[2]);
int	i, iv0;

	*iv = ERROR;
	*ie = ERROR;

	/*point is on the 3 pts of the tri */
	for(i=0; i<3; i++)
	    if(distance_between_positions(Coords(pt[i]), crds, 3) < tol)
	    {
	        *iv = i;
		ft_assign(crx, Coords(pt[i]), 3*FLOAT);
		return YES;
	    }

	/*point is on the edge of the tri */
	for(i=0; i<3; i++)
	{
	    /*calc the projection of point on line, if the projection is on the  */
	    /*line within tol, return YES */
	    line_point_projection(crx,&iv0,crds,
	        Coords(pt[i]),Coords(pt[Next_m3(i)]), tol);
	    if(iv0 == ERROR)
	        if(distance_between_positions(crx, crds, 3) < tol)
		{
		    *ie = i;
		    return YES;
		}
	}

	/*point is inside the tri */
	if(within_tri(crds,p0,p1,p2,norm,0.0))
	{
	    ft_assign(crx,crds,3*FLOAT);
	    return YES;
	}
	return NO;
}

EXPORT boolean line_tri_crossing(
	double *crx,
	TRI   *tri,
	double *crds1,
	double *crds2,
	double tol)
{
FT_POINT	**pt = Point_of_tri(tri);
double	*p0 = Coords(pt[0]), *p1 = Coords(pt[1]), *p2=Coords(pt[2]);
const double *n;
double    norm[MAXD], v[MAXD], D, t1, t2;
int	i,iv,ie;

	n = Tri_normal(tri); 
	D = Mag3d(n);
	for (i = 0; i < 3; ++i)
	    norm[i] = n[i]/D;

	difference(crds1, p0, v, 3);
	t1 = Dot3d(v, norm);
	difference(crds2, p0, v, 3);
	t2 = Dot3d(v, norm);

	/*near the plane of the tri */
	if(fabs(t1) < tol)
	{
	    for(i=0; i<3; i++)
	        v[i] = crds1[i] - t1*norm[i];
	    if(point_tri_crossing(crx,&iv,&ie,v,tri,norm,tol))
		return YES;
	}
	if(fabs(t2) < tol)
	{
	    for(i=0; i<3; i++)
	        v[i] = crds2[i] - t2*norm[i];
	    if(point_tri_crossing(crx,&iv,&ie,v,tri,norm,tol))
		return YES;
	}
	
	/*In the same side of the plane */
	if(t1*t2 > 0.0)
	    return NO;
	
	/*In the different side of the plane */
	t1 = fabs(t1);
	t2 = fabs(t2);
	for(i=0; i<3; i++)
	    v[i] = (crds1[i]*t2 + crds2[i]*t1)/(t1 + t2);
	
	if(point_tri_crossing(crx,&iv,&ie,v,tri,norm,tol))
	    return YES;

	if(debugging("line_tri"))
	{
	    printf("#line_tri_crx\n");
	    print_general_vector("crx=", crx, 3, "\n");
	    printf("%24.16e  %24.16e\n", t1, t2);
	}

	return NO;
}

EXPORT boolean line_point_projection(
	double *crx,
	int   *iv,
	double *pt,
	double *crds1,
	double *crds2,
	double tol)
{
double    n[MAXD],v[MAXD], D, t1, t2;
int	 i;

	*iv = ERROR;
	
	difference(crds1,crds2,n,3);
	D = Mag3d(n);
	if(D < tol)
	{
	    for(i=0; i<3; i++)
	        crx[i] = (crds1[i] + crds2[2])*0.5;
	    *iv = 2;
	    return YES;
	}
	for(i=0; i<3; i++)
	    n[i] /= D;
	
	difference(crds1, pt, v, 3);
	t1 = Dot3d(v, n);
	difference(crds2, pt, v, 3);
	t2 = Dot3d(v, n);
	
	/*On the same side of the plane */
	if(t1*t2 > 0.0)
	    if(fabs(t1) < fabs(t2))
	    {
	        ft_assign(crx, crds1, 3*sizeof(double));
		*iv = 0;
		return YES;
	    }
	    else
	    {
	        ft_assign(crx, crds2, 3*sizeof(double));
		*iv = 1;
		return YES;
	    }
	
	/*In different sides of the plane */
	t1 = fabs(t1);
	t2 = fabs(t2);
	for(i=0; i<3; i++)
	    crx[i] = (crds1[i]*t2 + crds2[i]*t1)/(t1 + t2);
	return YES;

}

#define  MAX_ANG_COMP    50

typedef  struct {
	double         dist;
	COMPONENT     comp;
}    ANG_COMP;

/*dist decrease order. */
int compare_ang_comp(const void *a, const void *b)
{
ANG_COMP  *c1=(ANG_COMP*)a, *c2=(ANG_COMP*)b;
double	dist1 = c1->dist, dist2 = c2->dist;

	return dist1 < dist2 ? 1 : -1;
}

LOCAL COMPONENT comp_from_line_tri(
	double		*ip,
	double		*crds1,
	TRI		*tri,
	SURFACE		*s,
	int		*icoords,
	INTERFACE	*intfc, 
	double		tol)
{
	COMPONENT	comp;
	TRI		**t;
	FT_POINT		**pt = Point_of_tri(tri);
	int		ix = icoords[0], iy = icoords[1], iz = icoords[2];
	int             i, nt, ixmin, ixmax, iymin, iymax, izmin, izmax;
	int		*gmax = (&topological_grid(intfc))->gmax;
	double		dist, dist0, crds2[3], crx[3], crx0[3];
	const double     *tnor;
	struct Table	*T = intfc->table;
        
	ixmin = (ix > 0) ? ix-1 : ix;
	ixmax = ((ix+1) < gmax[0]) ? ix+1 : ix;
	iymin = (iy > 0) ? iy-1 : iy;
	iymax = ((iy+1) < gmax[1]) ? iy+1 : iy;
	izmin = (iz > 0) ? iz - 1 : iz;
	izmax = ((iz+1) < gmax[2]) ? iz+1 : iz;

	for(i=0; i<3; i++)
	    crds2[i] = Coords(pt[0])[i] + Coords(pt[1])[i] + Coords(pt[2])[i];
	for(i=0; i<3; i++)
	    crds2[i] /= 3.0;

	ft_assign(crx, crds2, 3*FLOAT);
        dist = distance_between_positions(crds1, crds2, 3); 

	for (iz = izmin; iz <= izmax; ++iz)
	{
	    for (iy = iymin; iy <= iymax; ++iy)
	    {
	        for (ix = ixmin; ix <= ixmax; ++ix)
		{
	            if (T->compon3d[iz][iy][ix] != ONFRONT)
			continue;
	            t = T->tris[iz][iy][ix];
	            nt = T->num_of_tris[iz][iy][ix];

	            for (i = 0; i < nt; ++i)
		    {
		        if(t[i] == tri)
			    continue;
			if(line_tri_crossing(crx0,t[i],crds1,crds2,tol))
			{
			    dist0 = distance_between_positions(crds1, crx0, 3); 
			    if(dist0 < dist)
			    {
			        dist = dist0;
				ft_assign(crx, crx0, 3*FLOAT);
				tri = t[i];
				s = T->surfaces[iz][iy][ix][i];
			    }
		        }
		    } /* for nt */
		}
	    }
	}
	difference(crds1, crds2, crx0, 3);
	
	tnor = Tri_normal(tri);
	*ip = Dot3d(tnor, crx0)/(Mag3d(tnor)*Mag3d(crx0));
	if (*ip > 0.0)
	    return positive_component(s);
	if (*ip < 0.0)
	    return negative_component(s);

	printf("WARNING comp_from_line_tri, ip==0.0\n");
	return negative_component(s);   /*ARBITRARY CHOICE */

}

/* resolve nearest tri is point case. */
LOCAL COMPONENT component_wrt_icoords3d_vertex(
	double		*coords,
	int		*icoords,
	FT_POINT		*p,
	INTERFACE	*intfc)
{
	COMPONENT	comp;
	TRI		**t;
	SURFACE		**s;
	RECT_GRID	*gr = &topological_grid(intfc);
	int		i, j, k, nt;
	int		ix = icoords[0], iy = icoords[1], iz = icoords[2];
	int             ixmin, ixmax, iymin, iymax, izmin, izmax;
	int		*gmax = gr->gmax;
	struct Table	*T = intfc->table;
	double		*h = gr->h, crds[3], v[3], tol, hmin, dist;
	static ANG_COMP *ang_comp = NULL;

	if(ang_comp == NULL)
	    uni_array(&ang_comp, MAX_ANG_COMP, sizeof(ANG_COMP));

	hmin = min3(h[0], h[1], h[2]);
	tol = 1.0e-8*hmin;
	difference(coords, Coords(p), v, 3);
	for(i=0; i<3; i++)
	    crds[i] = Coords(p)[i] + 1.0e-2*hmin*v[i]/Mag3d(v);

	ixmin = (ix > 0) ? ix-1 : ix;
	ixmax = ((ix+1) < gmax[0]) ? ix+1 : ix;
	iymin = (iy > 0) ? iy-1 : iy;
	iymax = ((iy+1) < gmax[1]) ? iy+1 : iy;
	izmin = (iz > 0) ? iz - 1 : iz;
	izmax = ((iz+1) < gmax[2]) ? iz+1 : iz;

	for (iz = izmin; iz <= izmax; ++iz)
	{
	    for (iy = iymin; iy <= iymax; ++iy)
	    {
	        for (ix = ixmin; ix <= ixmax; ++ix)
		{
	            if (T->compon3d[iz][iy][ix] != ONFRONT)
			continue;
	            t = T->tris[iz][iy][ix];
	            nt = T->num_of_tris[iz][iy][ix];

	            for (i = 0; i < nt; ++i)
			Tri_projection_computed(t[i]) = NO;
		}
	    }
	}

	k = 0;
	for (iz = izmin; iz <= izmax; ++iz)
	{
	    for (iy = iymin; iy <= iymax; ++iy)
	    {
	        for (ix = ixmin; ix <= ixmax; ++ix)
		{
	            if (T->compon3d[iz][iy][ix] != ONFRONT)
			continue;

	            t = T->tris[iz][iy][ix];
	            nt = T->num_of_tris[iz][iy][ix];
	            s = T->surfaces[iz][iy][ix];

	            for (i = 0; i < nt; ++i)
	            {
			if (Tri_projection_computed(t[i]) != YES)
			{
			    Tri_projection_computed(t[i]) = YES;
			    /*consider the tris around the point, do not use  */
			    /*set_tri_list_around_point because there may be 3comp */
			    /*curve at p. */
			    for(j=0; j<3; j++)
			        if(Point_of_tri(t[i])[j] == p)
				    break;
			    if(j == 3)
			        continue;

			    ang_comp[k].comp = comp_from_line_tri(&dist,
			        crds,t[i],s[i],icoords,intfc,tol);
			    ang_comp[k].dist = fabs(dist);
			    k++;
			}
	            }
	        }
	    }
	}
	if(k == 0)
	{
	    printf("ERROR component_wrt_icoords3d_vertex, no crossing tris.\n");
	    clean_up(ERROR);
	}
	
	qsort((POINTER)ang_comp, k, sizeof(ANG_COMP), compare_ang_comp);
	if(debugging("ang_comp"))
	{
	    printf("k=%d\n", k);
	    for(i=0; i<k; i++)
		printf("#ang  %24.16e  %d\n", ang_comp[i].dist, ang_comp[i].comp);
	    printf("\n");
	}

	return ang_comp[0].comp;

}		/*end component_wrt_icoords3d*/


LOCAL double shortest_2line_dist_dir(
        int   dir,
        double *crds_start,
        double *p1,
        double *p2,
        double *pl1,
        double *pl2)
{
        int i,i0,i1,i2;
        double d;
        double t1, t2;
        double denom;

        i0 = dir;
        i1 = (dir+1)%3;
        i2 = (dir+2)%3;

        denom = (p1[i1]-p2[i1])*(p1[i1]-p2[i1])+
                      (p1[i2]-p2[i2])*(p1[i2]-p2[i2]);

        if (fabs(denom) >= crx_toll)
        {
            /* parameters corresponding to the closest points */

            t2 = ((p1[i1]-crds_start[i1])*(p1[i1]-p2[i1])+
                (p1[i2]-crds_start[i2])*(p1[i2]-p2[i2]))/denom;

            t1 = p1[i0]-crds_start[i0]+(p2[i0]-p1[i0])*t2;

            /* p1: point on the first curve closest to the second curve */
            /* p2: point on the second curve closest to the first curve */

            for (i = 0; i < 3; ++i)
            {
                pl1[i] = crds_start[i];
                pl2[i] = t2*(p2[i]-p1[i])+p1[i];
            }
            pl1[i0] += t1;
        }
        else /* curves are parallel */
        {
            pl1[i0] = p1[i0];
            pl1[i1] = crds_start[i1];
            pl1[i2] = crds_start[i2];

            for (i = 0; i < 3; ++i)
                pl2[i] = p1[i];
        }
        /* shortest distance between curves */

        d = (pl1[0]-pl2[0])*(pl1[0]-pl2[0])+(pl1[1]-pl2[1])*
            (pl1[1]-pl2[1])+(pl1[2]-pl2[2])*(pl1[2]-pl2[2]);

        return d;
}       /* end shortest_2line_dist_dir */
#endif /* defined(THREED) */
