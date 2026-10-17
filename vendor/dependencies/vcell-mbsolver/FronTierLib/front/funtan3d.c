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
*				 funtan3d.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	The routines in this file untangle a 3D interface.
*/

#if defined(THREED)

#define DEBUG_STRING "untan3d"

#include <front/fdecs.h>

enum {
	MAX_C_CURVES_ON_TRI =  5,
	MAX_SIZE_SIDE_VTS   =  6,
	MAX_SIZE_VTS	    = 10,
	MAX_SIZE_C_EDGES    = 100
};

struct _Vertex {
    	FT_POINT       *point;
	double       x, y;
	int         index;
	int         cdt_side_index[3];
	int         tri_vertex_index;
	int         tri_side_index;
	double       d2;
};
typedef struct  _Vertex  Vertex;

struct _Cdt_Side {
	Vertex  *vertices[MAX_SIZE_SIDE_VTS];
	TRI     *tris[MAX_SIZE_SIDE_VTS-1];
	int     tri_facing_side[MAX_SIZE_SIDE_VTS-1];
	int     num_verts;
	int     num_tris;
	boolean    done_stiching;
};
typedef struct  _Cdt_Side  Cdt_Side;

struct _Cdt {
	struct _Cdt *next;
	C_BOND      **c_bond_list;
	Cdt_Side    side[3];
	SURFACE     *surf;
	TRI         *tri;
	int         isurf;
};
typedef struct _Cdt  Cdt;

struct _NCSURF {
    SURFACE     *s;
    ORIENTATION orient;
    boolean     physical;
    double       area;
};
typedef struct _NCSURF NCSURF;

struct _NCLIST {
	struct _NCLIST *prev, *next;

	NCSURF         NS[4];
	C_CURVE        *c_curve;
	boolean           _physical_region[4];
	int            num_comps;
	int            num_physical_regions;
	int            num_surfaces;
};
typedef struct _NCLIST NCLIST;

#define physical_region(ncl,i)	((ncl)->_physical_region[(i)])
#define physical_surf(ncl,i)	((ncl)->NS[(i)].physical)


#define Point_vertex_pointer(_pnt_) ((Vertex*)opaque_pointer(_pnt_))
#define Tri_side_of_cdt(_cdt_)      ((_cdt_)->side)
#define Tri_color(tri) Tri_index(tri)

#define cdt_of_tri(tri) (Cdt *)Tri_workspace(tri)

#define c_bond_start_is_tri_vertex(cb)					\
	((is_start_on_bdry(cb,0) && is_start_to_vertex(cb,0)) 		\
	    		 &&					        \
	 (is_start_on_bdry(cb,1) && is_start_to_vertex(cb,1)))

#define c_bond_end_is_tri_vertex(cb)					\
	((is_end_on_bdry(cb,0) && is_end_to_vertex(cb,0))     		\
	    			&&				      	\
	 (is_end_on_bdry(cb,1) && is_end_to_vertex(cb,1)))

	/* LOCAL Function Prototypes */
LOCAL	NCLIST	*set_nclist(INTERFACE*);
LOCAL	boolean	delete_detected_unphysical_surfaces(NCLIST**);
LOCAL	boolean	delete_unphysical_surfaces_at_c_curve(NCLIST**,NCLIST**);
LOCAL	boolean	c_bonds_intersect(C_BOND*,C_BOND*);
LOCAL	boolean	c_curve_out_comp_domain(C_CURVE*);
LOCAL	boolean	cdt_triangulate_one_tri(Cdt*);
LOCAL	boolean	cdt_retriangulate_surface_along_c_curves(SURFACE*);
LOCAL	boolean	curve_is_in_c_curve_list(CURVE*);
LOCAL	boolean	delete_inconsistent_surfs(INTERFACE*);
LOCAL	boolean	prepare_v_and_e_for_cdt(Cdt*,Vertex*,int*,int*,int*);
LOCAL	boolean	split_surfaces_at_c_curves(INTERFACE*);
LOCAL	int	paint_component(TRI*,int);
LOCAL	void	print_cdt(Cdt*);
LOCAL	void	print_cdt_side(Cdt_Side*,INTERFACE*);
LOCAL	void	print_nclist(NCLIST*,INTERFACE*);
LOCAL	void	print_ncsurf(NCSURF*);
LOCAL	void	print_vertex(const char*,Vertex*);
LOCAL	void	insert_curve_from_c_curve(C_CURVE*);
LOCAL	void	install_tris_from_dtris(Cdt*,Vertex*,triangulateio*);
LOCAL	void	set_physical_surfaces_at_curve(CURVE*,NCLIST*);
LOCAL	void	set_states_at_cross_points(Front*,C_CURVE*);
LOCAL	void	set_states_at_crosses(Front*);

/*
*			scalar_unravel_3d():
*
*	The main routine for untangling a 3D scalar interface.
*	This routine assumes that different topological components
*	have different indices. Such indices are used to identify the
*	two sides of surfaces bounding these components. Surfaces are
*	first split at c_curves and then crosses are resolved using
*	these component information.
*/

EXPORT	boolean scalar_unravel_3d(
	Front  *front,
	CROSS  **crosses)
{
	INTERFACE      *intfc = front->interf;
	INTERFACE      *hold_intfc;
	C_CURVE        **pc;
	SURFACE        **ps;
	CROSS          *cross;

		/* prelimianry setups */

	DEBUG_ENTER(scalar_unravel_3d)

	hold_intfc = current_interface();
	set_current_interface(intfc);

	cross = *crosses;
	if (cross == NULL)
	{
	    set_current_interface(hold_intfc);
	    DEBUG_LEAVE(scalar_unravel_3d)
	    return FUNCTION_SUCCEEDED;
	}

	if (DEBUG)
	{
	    (void) printf("Tangled interface\n");
	    print_interface(intfc);
	    (void) printf("checking consistency of input interface\n");
	    if (!consistent_interface(intfc))
	    {
		screen("ERROR in scalar_unravel_3d(), "
		       "inconsistent input interface\n");
		clean_up(ERROR);
	    }
	    else
		(void) printf("input interface is consistent\n");
	}

	/* Delete those c_curves completely outside comp domain */

	for (pc = intfc->c_curves; pc && *pc; ++pc)
	{	    
	    if (c_curve_out_comp_domain(*pc))
	    {
		C_CURVE *cc = *pc;
		SURFACE *s0 = cc->s[0], *s1 = cc->s[1];
		(void) delete_from_pointers(cc,&intfc->c_curves);
		(void) delete_from_pointers(cc,&s0->c_curves);
		if (s0 != s1)
		    (void) delete_from_pointers(cc,&s1->c_curves);
		pc = intfc->c_curves;
	    }
	}

	set_states_at_crosses(front);

	/* Make constrained delaunay triangulation on   */
	/* surfaces for given cross points on triangles */

	start_clock("constraint_delaunay_tri");
	for (ps = intfc->surfaces; ps && *ps; ++ps)
	{
	    if (!cdt_retriangulate_surface_along_c_curves(*ps))
	    {
	        set_current_interface(hold_intfc);
		(void) printf("WARNING in scalar_unravel_3d(), "
		              "cdt_retriangulate_surface_along_c_curves() "
			      "failed\n");
	        DEBUG_LEAVE(scalar_unravel_3d)
		return FUNCTION_FAILED;
	    }
	}
	stop_clock("constraint_delaunay_tri");

	/* Make curves linking crossing surfaces */
	/* and split surfaces at crossing curve  */

	start_clock("split_surfaces_at_c_curves");
	if (!split_surfaces_at_c_curves(intfc))
	{
	    set_current_interface(hold_intfc);
	    (void) printf("WARNING in scalar_unravel_3d(), "
	                  "split_surfaces_at_c_curves() "
	    	      "failed\n");
	    DEBUG_LEAVE(scalar_unravel_3d)
	    return FUNCTION_FAILED;
	}
	stop_clock("split_surfaces_at_c_curves");

	if (DEBUG)
	{
	    (void) printf("checking consistency of tris on surface after\n"
	                  "constrained delaunay triangulation, stitching "
			  "and curve insertion\n");
	    if (!consistent_interface(intfc))
	    {
		screen("ERROR in scalar_unravel_3d(), "
		       "retriangulation produces an inconsistent interface\n");
		clean_up(ERROR);
	    }
	    else
		(void) printf("interface is consistent after "
	                      "retriangulation, stitching "
			      "and curve insertion\n");
	}


	start_clock("delete_inconsistent_surf");
	if (!delete_inconsistent_surfs(intfc))
	{
	    set_current_interface(hold_intfc);
	    (void) printf("WARNING in scalar_unravel_3d(), "
		          "delete_inconsistent_surfs() failed\n");
	    DEBUG_LEAVE(scalar_unravel_3d)
	    return FUNCTION_FAILED;
	}
	stop_clock("delete_inconsistent_surf");

	if (DEBUG)
	{
	    (void) printf("Untangled interface\n");
	    print_interface(intfc);
	    (void) printf("Check interface consistency before leaving\n");
	    if (!consistent_interface(intfc))
	    {
		screen("ERROR in scalar_unravel_3d(), "
		       "inconsistent interface at end of untangle\n");
		clean_up(ERROR);
	    }
	    (void) printf("Interface is consistent!\n");
	}

	set_current_interface(hold_intfc);
	DEBUG_LEAVE(scalar_unravel_3d)
	return FUNCTION_SUCCEEDED;
} 	/* end scalar_unravel_3d */

LOCAL	boolean split_surfaces_at_c_curves(
	INTERFACE *intfc)
{
	BOND           *b;
	BOND_TRI       **bts;
	C_CURVE        **pc;
	TRI            *tri;
	boolean        sav_intrp;
	int            n, color, ncolors;
	static SURFACE **news = NULL, **olds = NULL;
	static int     *colors = NULL, *num_tris_of_color = NULL;
	static size_t  max_ncolors = 0;

	DEBUG_ENTER(split_surfaces_at_c_curves)

	if (DEBUG)
	{
	    (void) printf("Interface into split_surfaces_at_c_curves()\n");
	    print_interface(intfc);
	}

	for (pc = intfc->c_curves; pc && *pc; ++pc)
	    insert_curve_from_c_curve(*pc);

	/* Reset triangle index on surface */
	null_tri_array_numbers(intfc);/*Sets all colors to 0*/
	if (colors == NULL)
	{
	    max_ncolors = 100;
	    uni_array(&colors,max_ncolors,INT);
	    uni_array(&num_tris_of_color,max_ncolors,INT);
	    uni_array(&news,max_ncolors,sizeof(SURFACE*));
	    uni_array(&olds,max_ncolors,sizeof(SURFACE*));
	}
	for (ncolors = 0, color = 1, pc = intfc->c_curves; pc && *pc; ++pc)
	{
	    b = (*pc)->first->bond;

	    for (bts = Btris(b); bts && *bts; ++bts)
	    {
	        tri = (*bts)->tri;
		if (Tri_color(tri) == 0)
		{
		    if (ncolors >= max_ncolors)
		    {
			SURFACE **new_olds;
			int     i;
			int     *new_colors, *new_num_tris_of_color;
			size_t  new_max_ncolors = 2*max_ncolors;

	                uni_array(&new_colors,new_max_ncolors,INT);
	                uni_array(&new_num_tris_of_color,new_max_ncolors,INT);
	                uni_array(&new_olds,new_max_ncolors,sizeof(SURFACE*));
			for (i = 0; i < ncolors; ++i)
			{
			    new_colors[i] = colors[i];
			    new_num_tris_of_color[i] = num_tris_of_color[i];
			    new_olds[i]  = olds[i];
			}
			free_these(4,colors,num_tris_of_color,olds,news);
	                uni_array(&news,max_ncolors,sizeof(SURFACE*));
			colors = new_colors;
			num_tris_of_color = new_num_tris_of_color;
			olds = new_olds;
		    }
		    olds[ncolors] = (*bts)->surface;
		    num_tris_of_color[ncolors] = paint_component(tri,color);
		    colors[ncolors++] = color++;
		}
	    }
	}

	if (DEBUG)
	    (void) printf("%d colors used\n",ncolors);

	/* Create new surfaces, one for each color */
	for (n = 0; n < ncolors; ++n)
	{
	    CURVE       *c, **neg, **pos;
	    TRI         *ntri;

	    color = colors[n];
	    neg = pos = NULL;
	    for (pc = intfc->c_curves; pc && *pc; ++pc)
	    {
		b = (*pc)->first->bond;
	        for (bts = Btris(b); bts && *bts; ++bts)
	        {
		    if (Tri_color((*bts)->tri) == color)
		    {
			tri = (*bts)->tri;
			c = (*pc)->curve;
		        switch ((*bts)->orient)
		        {
		        case POSITIVE_ORIENTATION:
			    if (!unique_add_to_pointers(c,&pos))
			    {
			        screen("ERROR in split_surfaces_at_c_curves(), "
				       "unique_add_to_pointers() failed for "
				       "POSITIVE_ORIENTATION\n");
				clean_up(ERROR);
			    }
			    break;
		        case NEGATIVE_ORIENTATION:
			    if (!unique_add_to_pointers(c,&neg))
			    {
			        screen("ERROR in split_surfaces_at_c_curves(), "
				       "unique_add_to_pointers() failed for "
				       "NEGATIVE_ORIENTATION\n");
				clean_up(ERROR);
			    }
			    break;
		        case ORIENTATION_NOT_SET:
		        default:
			    (void) printf("WARNING in "
					  "split_surfaces_at_c_curves(), "
				          "inconsistent curve orientation\n");
	                    DEBUG_LEAVE(split_surfaces_at_c_curves)
	                    return NO;
		        }
		    }
	        }
	    }
	    news[n] = copy_surface(olds[n],neg,pos,NO);
	    /* Move tris from s to news[n] */
	    for (tri=first_tri(olds[n]); !at_end_of_tri_list(tri,olds[n]);
		 tri=ntri)
	    {
		ntri = tri->next;
		if (Tri_color(tri) == color)
		{
		    remove_tri_from_surface(tri,olds[n],YES);
		    insert_tri_at_tail_of_list(tri,news[n]);
		}
	    }
	}

	if (DEBUG)
	{
	    for (n = 0; n < ncolors; ++n)
		(void) printf("New surface %llu has color %d, "
			      "old surface = %llu\n",
			      surface_number(news[n]),colors[n],
			      surface_number(olds[n]));
	}

	/* Delete old surfaces */
	for (n = 0; n < ncolors; ++n)
	{
	    if (olds[n]->num_tri == 0)
	    {
		if (pointer_is_in_array(olds[n],intfc->surfaces))
		{
		    while(olds[n]->pos_curves != NULL)
		        (void) remove_curve_from_surface_bdry(olds[n],
				olds[n]->pos_curves[0],POSITIVE_ORIENTATION);
		    while(olds[n]->neg_curves != NULL)
		        (void) remove_curve_from_surface_bdry(olds[n],
				olds[n]->neg_curves[0],NEGATIVE_ORIENTATION);
		    if (!delete_surface(olds[n]))
		    {
		        (void) printf("WARNING in "
				      "split_surfaces_at_c_curves(), "
			              "can't delete old surface %llu\n",
				      surface_number(olds[n]));
	                DEBUG_LEAVE(split_surfaces_at_c_curves)
		        return NO;
		    }
		}
	    }
	    else
	    {
		(void) printf("WARNING in split_surfaces_at_c_curves(), "
			      "triangles remain on surface to be deleted\n");
	        DEBUG_LEAVE(split_surfaces_at_c_curves)
		return NO;
	    }
	}
	sav_intrp = interpolate_intfc_states(intfc);
	interpolate_intfc_states(intfc) = YES;
	if (!sort_bond_tris(intfc))
	{
	    (void) printf("WARNING in split_surfaces_at_c_curves(), "
			  "sort_bond_tris() failed\n");
	    DEBUG_LEAVE(split_surfaces_at_c_curves)
	    return NO;
	}
	interpolate_intfc_states(intfc) = sav_intrp;
	DEBUG_LEAVE(split_surfaces_at_c_curves)
	return YES;
}		/*end split_surfaces_at_c_curves*/

LOCAL	NCLIST *set_nclist(
	INTERFACE *intfc)
{
	BOND_TRI    **bts;
	BOND        *b;
	C_CURVE     **pc;
	COMPONENT   complist[8], ucomplist[8];
	CURVE       *pcc;
	NCLIST      *nclist, *ncl;
	NCSURF      NS[4];
	TRI         *tri;
	int         i, j, ncomp;
	size_t      nc_curves;
	double       mag, bdir[3], tdir[4][3], nor[4][3], ang[4];
	const double *tnor;

	/* Set up NCLIST structures */
	nc_curves = size_of_pointers(intfc->c_curves);
	nclist = (NCLIST*)store(nc_curves*sizeof(NCLIST));
	for (ncl = nclist, i = 0; i < nc_curves; ++ncl, ++i)
	{
	    ncl->next = ncl + 1;
	    ncl->prev = ncl - 1;
	}
	nclist[0].prev = NULL;
	nclist[nc_curves-1].next = NULL;
	for (ncl = nclist, pc = intfc->c_curves; pc && *pc; ++ncl, ++pc)
	{
	    pcc = (*pc)->curve;
	    ncl->c_curve = *pc;
	    b = pcc->first;
	    for (i = 0; i < 3; ++i)
		bdir[i]=(Coords(b->end)[i]-Coords(b->start)[i])/bond_length(b);
	    for (i = 0, bts = Btris(b); bts && *bts; ++i, ++bts)
	    {
		tri = (*bts)->tri;
		NS[i].s = (*bts)->surface;
		NS[i].physical = NO;
		NS[i].orient = (*bts)->orient;
		NS[i].area = -HUGE_VAL;
		tnor = Tri_normal(tri);
		mag = Mag3d(tnor);
		for (j = 0; j < 3; ++j)
		    nor[i][j] = tnor[j]/mag;
		switch (NS[i].orient)
		{
		case POSITIVE_ORIENTATION:
		    Cross3d(bdir,nor[i],tdir[i]);
		    break;
		case NEGATIVE_ORIENTATION:
		    Cross3d(nor[i],bdir,tdir[i]);
		    break;
		case ORIENTATION_NOT_SET:
		    (void) printf("WARNING in set_nclist(), "
			          "unset orientation\n");
		    return NULL;
		}
	    }
	    if (i < 4)
	    {
	        screen("ERROR in set_nclist(), unexpected case %d < 4 "
		       "surfaces\n",i);
		clean_up(ERROR);
	    }
	    for (i = 0; i < 4; ++i)
	    {
		double x = Dot3d(tdir[i],tdir[0]);
		double y = Dot3d(tdir[i],nor[0]);
		ang[i] = angle(x,y);
	    }

	    /* Sort ncl->ns */
	    ncl->NS[0] = NS[0];
	    if (ang[1] <= ang[2])
	    {
		if (ang[1] <= ang[3])
		{
		    ncl->NS[1] = NS[1];
		    if (ang[2] <= ang[3])
		    {
			ncl->NS[2] = NS[2];
			ncl->NS[3] = NS[3];
		    }
		    else
		    {
			ncl->NS[2] = NS[3];
			ncl->NS[3] = NS[2];
		    }
		}
		else
		{
		    ncl->NS[1] = NS[3];
		    ncl->NS[2] = NS[1];
		    ncl->NS[3] = NS[2];
		}
	    }
	    else
	    {
		if (ang[2] <= ang[3])
		{
		    ncl->NS[1] = NS[2];
		    if (ang[1] <= ang[3])
		    {
			ncl->NS[2] = NS[1];
			ncl->NS[3] = NS[3];
		    }
		    else
		    {
			ncl->NS[2] = NS[3];
			ncl->NS[3] = NS[1];
		    }
		}
		else
		{
		    ncl->NS[1] = NS[3];
		    ncl->NS[2] = NS[2];
		    ncl->NS[3] = NS[1];
		}
	    }
	    ncl->num_surfaces = 4;
	    ncl->num_physical_regions = 0;
	    for (i = 0; i < 4; ++i)
	    {
		complist[2*i] = positive_component(NS[i].s);
		complist[2*i+1] = negative_component(NS[i].s);
		physical_region(ncl,i) = NO;
		physical_surf(ncl,i) = NO;
	    }
	    for (ncomp = 0, i = 0; i < 8; ++i)
	    {
		for (j = 0; j < ncomp; ++j)
		    if (complist[i] == ucomplist[j])
			break;
		if (j == ncomp)
		    ucomplist[ncomp++] = complist[i];
	    }
	    ncl->num_comps = ncomp;
	}
	return nclist;
}		/*end set_nclist*/


LOCAL void insert_curve_from_c_curve(
	C_CURVE *c_curve)
{
	BOND      *b;
	BOND_TRI  **bts;
	C_BOND    *cb;
	CURVE     *curve;
	INTERFACE *intfc = c_curve->interface;
	NODE      *ns, *ne;
	SURFACE   *s;
	TRI       *tri;

	if (c_curve->curve != NULL)
	    return;
	cb = c_curve->first;
	if ((ns = node_of_point(cb->start,intfc)) == NULL)
	    ns = make_node(cb->start);
	cb = c_curve->last;
	if ((ne = node_of_point(cb->end,intfc)) == NULL)
	    ne = make_node(cb->end);
	c_curve->curve = curve = make_curve(NO_COMP,NO_COMP,ns,ne);
	curve->first = c_curve->first->bond;
	for (cb = c_curve->first; cb != NULL; cb = cb->next)
	{
	    if (cb->next)
	        cb->bond->next = cb->next->bond;
	    if (cb->prev)
	        cb->bond->prev = cb->prev->bond;
	}
	curve->last = c_curve->last->bond;
	b = c_curve->first->bond;
	for (bts = Btris(b); bts && *bts; ++bts)
	{
	    ORIENTATION orient;

	    s = (*bts)->surface;
	    tri = (*bts)->tri;
	    if ((s == c_curve->s[0]) || (s == c_curve->s[1]))
	    {
		orient = (*bts)->orient;
	        install_curve_in_surface_bdry(s,curve,orient);
	    }
	}
	for (b = curve->first; b != NULL; b = b->next)
	{
	    for (bts = Btris(b); bts && *bts; ++bts)
	    {
	        s = (*bts)->surface;
	        tri = (*bts)->tri;
	        if ((s == c_curve->s[0]) || (s == c_curve->s[1]))
		    (void) link_tri_to_bond(*bts,tri,s,b,curve);
	    }
	}
}		/*end insert_curve_from_c_curve*/


/*   
*
*			paint_component():
*
*	The algorithm used in the following is a simple breadth first search
*	algorithm on the dual graph of the triangulated surface.  A description
*	of breath first search can be found, for example, in "Introduction to 
*	Algorithms," by Corman, Leiserson, Rivest (MIT Press, 1991).
*    
*	The algorithm will find all triangles reachable from tri and paint
*	them with the given color.  The algorithm runs in linear time in the 
*	size k of the connected component of tri.  The maximal queue size needed
*	is bounded by worst case k/2 which corresponds to the maximal number of 
*	leaves in a binary tree of size k. The topology of this problem,
*	however, makes k/4 a more reasonable bound on the queue size.
*
*/

LOCAL int paint_component(
	TRI *tri, 
	int color)
{
	TRI              *t;
	POINTER_Q        *tri_queue = NULL, *head;
	int              i, num_tris;
	static const int TRI_BLOCK_SIZE = 100;/*TOLERANCE*/
	
		/* Initialize queue */

	set_pointer_queue_opts(PQ_BLOCK_SIZE,TRI_BLOCK_SIZE,PQ_ALLOC_TYPE,
	                       "store",PQ_ALLOC_SIZE_FOR_POINTERS,
			       sizeof(TRI*),0);
	
	num_tris = 1;
	Tri_color(tri) = color;
	tri_queue = add_to_pointer_queue(tri,tri_queue);
	
	while(tri_queue)
	{
	    head = head_of_pointer_queue(tri_queue);
	    t = (TRI *)head->pointer;
	    for(i = 0; i < 3; ++i)
	    {
		if (is_side_bdry(t,i))
		    continue;
		if (Tri_on_side(t,i) == NULL)
		{
		    screen("ERROR in paint_component(): "
		           "NULL pointer found along tri edge\n");
		    clean_up(ERROR);
		}
		if (Tri_color(Tri_on_side(t,i)) == color)
		    continue;
	        Tri_color(Tri_on_side(t,i)) = color;
	        ++num_tris;
	        tri_queue = add_to_pointer_queue(Tri_on_side(t,i),tri_queue);
	    }
	    tri_queue = delete_from_pointer_queue(head);
	}
	 
	return num_tris;
}	/* end paint_component */

LOCAL boolean delete_inconsistent_surfs(
	INTERFACE *intfc)
{
	COMPONENT compi, compj;
	CURVE     **c;
	NCLIST	  *ncl, *nclist, *l;
	int       i, j, k, n;

	DEBUG_ENTER(delete_inconsistent_surfs)
	if (DEBUG)
	{
	    (void) printf("Interface into delete_inconsistent_surfs()\n");
	    print_interface(intfc);
	}

	if ((nclist = set_nclist(intfc)) == NULL)
	{
	    (void) printf("WARNING in delete_inconsistent_surfs(), "
	                  "set_nclist() failed\n");
	    DEBUG_LEAVE(delete_inconsistent_surfs)
	    return FUNCTION_FAILED;
	}

	if (DEBUG)
	{
	    for (i = 0, ncl = nclist; ncl != NULL; ++i, ncl = ncl->next);
	    (void) printf("detected %d tangles to resolve\n",i);
	    for (ncl = nclist; ncl != NULL; ncl = ncl->next)
	        print_nclist(ncl,intfc);
	}

	for (c = intfc->curves; c && *c; ++c)
	{
	    if (!curve_is_in_c_curve_list(*c))
		set_physical_surfaces_at_curve(*c,nclist);
	}

	/* First pass delete surfaces */

	if (!delete_detected_unphysical_surfaces(&nclist))
	{
	    (void) printf("WARNING in delete_inconsistent_surfs(), "
			  "delete_detected_unphysical_surfaces() "
			  "failed on first pass\n");
	    DEBUG_LEAVE(delete_inconsistent_surfs)
	    return NO;
	}
	if (nclist == NULL) /* We are done */
	{
	    DEBUG_LEAVE(delete_inconsistent_surfs)
	    return YES;
	}

	for (ncl = nclist; ncl != NULL; ncl = ncl->next)
	{
	    if ((ncl->num_comps == 2) && (ncl->num_physical_regions == 2))
	    {
		for (i = 0; i < 4; ++i)
		{
		    if (physical_region(ncl,i) == NO)
		    {
			j = (i+1)%4;
			compi = (ncl->NS[i].orient == POSITIVE_ORIENTATION) ?
				positive_component(ncl->NS[i].s) :
				negative_component(ncl->NS[i].s);
			compj = (ncl->NS[j].orient == NEGATIVE_ORIENTATION) ?
				positive_component(ncl->NS[j].s) :
				negative_component(ncl->NS[j].s);
			if (compi != compj)
			{
			    physical_surf(ncl,i) = YES;
			    for (l = nclist; l != NULL; l = l->next)
			    {
				for (k = 0; k < 4; ++k)
				{
				    if (l->NS[k].s == ncl->NS[i].s)
				    {
					if (physical_surf(l,k) == NO)
					    physical_surf(l,k) = YES;
					if (physical_region(l,k) == NO)
					{
					    physical_region(l,k) = YES;
					    ++l->num_physical_regions;
					}
					n = (k+3)%4;
					if (physical_region(l,n) == NO)
					{
					    physical_region(l,n) = YES;
					    ++l->num_physical_regions;
					}
				    }
				}
			    }
			}
		    }
		}
	    }
	}

	/* Second pass delete surfaces */

	if (!delete_detected_unphysical_surfaces(&nclist))
	{
	    (void) printf("WARNING in delete_inconsistent_surfs(), "
			  "delete_detected_unphysical_surfaces() "
			  "failed on second pass\n");
	    DEBUG_LEAVE(delete_inconsistent_surfs)
	    return NO;
	}
	if (nclist == NULL) /* We are done */
	{
	    DEBUG_LEAVE(delete_inconsistent_surfs)
	    return YES;
	}

	/* Compute remaining surface areas */
	for (ncl = nclist; ncl != NULL; ncl = ncl->next)
	{
	    for (i = 0; i < 4; ++i)
	    {
		if (ncl->NS[i].s != NULL)
		{
		    SURFACE *s = ncl->NS[i].s;
		    TRI     *t;
		    double   area;
		    area = 0.0;
	            for (t = first_tri(s); !at_end_of_tri_list(t,s); t = t->next)
		    {
		        const double *nor = Tri_normal(t);
			area += 0.5*Mag3d(nor);
		    }
		    ncl->NS[i].area = area;
		}
	    }
	}

	for (i = 0, ncl = nclist; ncl != NULL; ++i, ncl = ncl->next);
	(void) printf("WARNING in delete_inconsistent_surfs(), "
		      "%d unresolved tangles\n",i);
	(void) printf("Tangled interface\n");
	print_interface(intfc);
	for (ncl = nclist; ncl != NULL; ncl = ncl->next)
	    print_nclist(ncl,intfc);

	/* TODO */
	DEBUG_LEAVE(delete_inconsistent_surfs)
	return NO;
}		/* end delete_inconsistent_surfs */

LOCAL	boolean	delete_detected_unphysical_surfaces(
	NCLIST **pnclist)
{
	NCLIST *nclist = *pnclist;
	NCLIST *ncl;

	DEBUG_ENTER(delete_detected_unphysical_surfaces)
	ncl = nclist;
	while ((nclist != NULL) && (ncl != NULL))
	{
	    if ((ncl->num_physical_regions == 3) &&
		(!delete_unphysical_surfaces_at_c_curve(&ncl,&nclist)))
	    {
		(void) printf("WARNING in delete_inconsistent_surfs(), "
			      "delete_unphysical_surfaces_at_c_curve "
			      "failed\n");
		DEBUG_LEAVE(delete_detected_unphysical_surfaces)
		return NO;
	    }
	    else if (ncl != NULL)
		ncl = ncl->next;
	}
	*pnclist = nclist;
	DEBUG_LEAVE(delete_detected_unphysical_surfaces)
	return YES;
}		/*end delete_detected_unphysical_surfaces*/

LOCAL	boolean curve_is_in_c_curve_list(
	CURVE *c)
{
	C_CURVE **pc;

	for (pc = c->interface->c_curves; pc && *pc; ++pc)
	    if ((*pc)->curve == c)
		return YES;
	return NO;
}		/*end curve_is_in_c_curve_list*/

LOCAL	void set_physical_surfaces_at_curve(
	CURVE *c,
	NCLIST *nclist)
{
	NCLIST  *ncl;
	SURFACE **s, **slist[2];
	int     i, j;

	DEBUG_ENTER(set_physical_surfaces_at_curve)
	slist[0] = c->pos_surfaces; slist[1] = c->neg_surfaces;
	for (j = 0; j < 2; ++j)
	{
	    for (s = slist[j]; s && *s; ++s)
	    {
	        for (ncl = nclist; ncl != NULL; ncl = ncl->next)
	        {
		    for (i = 0; i < 4; ++i)
		    {
		        if (ncl->NS[i].s == *s)
		        {
			    physical_surf(ncl,i) = YES;
			    if (physical_region(ncl,i) == NO)
			    {
			        physical_region(ncl,i) = YES;
			        ++ncl->num_physical_regions;
			    }
			    if (physical_region(ncl,(i+3)%4) == NO)
			    {
			        physical_region(ncl,(i+3)%4) = YES;
			        ++ncl->num_physical_regions;
			    }
		        }
		    }
	        }
	    }
	}
	DEBUG_LEAVE(set_physical_surfaces_at_curve)
}		/*end set_physical_surfaces_at_curve*/

LOCAL	boolean delete_unphysical_surfaces_at_c_curve(
	NCLIST **pncl,
	NCLIST **pnclist)
{
	NCLIST *ncl = *pncl;
	NCLIST *nclist = *pnclist;
	NCLIST  *l, *nl, *ll;
	SURFACE *s, *news;
	int     i, j, pj;

	DEBUG_ENTER(delete_unphysical_surfaces_at_c_curve)

	/* UPGRADE HERE to support leaving a separating surface */
	for (i = 0; i < 4; ++i)
	{
	    if ((physical_surf(ncl,i) == NO) && (ncl->NS[i].s != NULL))
	    {
		s = ncl->NS[i].s;
		ncl->NS[i].area = -HUGE_VAL;
	        ncl->NS[i].orient = ORIENTATION_NOT_SET;
		ncl->NS[i].physical = NO;
		(void) delete_surface(s);
		for (l = nclist; l != NULL; l = l->next)
		{
		    for (j = 0; j < 4; ++j)
		    {
			if (l->NS[j].s == s)
			{
			    l->NS[j].s = NULL;
			    l->NS[j].area = -HUGE_VAL;
			    l->NS[j].orient = ORIENTATION_NOT_SET;
			    l->NS[j].physical = NO;
		            l->num_surfaces--;
			    pj = (j+3)%4;
			    if ((physical_region(l,j) == NO) &&
				(physical_region(l,pj) == YES))
			    {
				physical_region(l,j) = YES;
				++l->num_physical_regions;
			    }
			    if ((physical_region(l,pj) == NO) &&
				(physical_region(l,j) == YES))
			    {
				physical_region(l,pj) = YES;
				++l->num_physical_regions;
			    }
			}
		    }
		}
	    }
	}

	/* Join at curves with only two remaining surfaces */
	for (l = nclist; l != NULL; l = nl)
	{
	    nl = l->next;
	    if (l->num_surfaces == 2)
	    {
		SURFACE *sn, *sp;
		if (!find_surfaces_to_join_at_curve(l->c_curve->curve,&sn,&sp))
		{
	            DEBUG_LEAVE(delete_unphysical_surfaces_at_c_curve)
		    return NO;
		}
		if ((news = join_surfaces(l->c_curve->curve))==NULL)
		{
	            DEBUG_LEAVE(delete_unphysical_surfaces_at_c_curve)
		    return NO;
		}
	        /* unlink l */
	        if (l->prev)
	            l->prev->next = l->next;
	        else
	            *pnclist = nclist = l->next;
	        if (l->next)
	            l->next->prev = l->prev;
		/* Reset surface pointers after surface deletion */
	        for (ll = nclist; ll != NULL; ll = ll->next)
		{
		    for (i = 0; i < 4; ++i)
			if ((ll->NS[i].s == sn) || (ll->NS[i].s == sp))
			    ll->NS[i].s = news;
		}
	    }
	}
	*pncl = *pnclist;
	DEBUG_LEAVE(delete_unphysical_surfaces_at_c_curve)
	return YES;
}		/*end delete_unphysical_surfaces_at_c_curve*/



/*
*		cdt_retriangulate_surface_along_c_curves():
*	   			
*	This function retriangulates surface surf along c_curves.
*	The main steps involved are the following: (1) Re-triangulate
*	(find the cdt of) each tris passed through by the c_curves; (2)
*	stitch neighboring cdts together. 
*/

LOCAL  boolean  cdt_retriangulate_surface_along_c_curves(
	SURFACE	*surf)
{				 
	int	  i;
	int	  is, ie, isurf;
	TRI	  *t;
	C_BOND	  *cb;
	C_CURVE	  **pc;
	Cdt	  *cdt, *cdt_tris;

	DEBUG_ENTER(cdt_retriangulate_surface_along_c_curves)

	if (surf->c_curves == NULL)
	{
	    DEBUG_LEAVE(cdt_retriangulate_surface_along_c_curves)
	    return FUNCTION_SUCCEEDED;
	}

	    /* allocate Cdts */

	cdt_tris = NULL;

	/* We loop over all c_curves in the given surface... */

	for (pc = surf->c_curves; pc && *pc; ++pc)
	{	    
	    if (DEBUG)
	    	(void) printf("Setting up cdt's along c_curve %p of surface "
			      "%llu\n",(POINTER)*pc,surface_number(surf));

	    if (surf==(*pc)->s[0] && surf==(*pc)->s[1])
	    {
		is = 0;		ie = 2;
	    }
	    else if (surf == (*pc)->s[0])  
	    {
		is = 0;		ie = 1;
	    }
	    else if (surf == (*pc)->s[1])
	    {
		is = 1;		ie = 2;
	    }
	    else 
	    {
		screen("ERROR in cdt_retriangulate_surface_along_c_curves(), "
		       "surfaces on c_curve do not match surf\n");
		clean_up(ERROR);
	    }

	        
	    /* We now list the tris to be cdt'ed.  We allocate a linked list
	     * of Cdt data structures, each of which corresponds to a triangle.
	    */

	    for (cb = (*pc)->first; cb != NULL; cb = cb->next)
	    {
		for (isurf = is; isurf < ie; ++isurf)
		{
		    cb->c_curve = *pc;
		    t = cb->s[isurf].t;
         
		    if (DEBUG)
		    {
		        (void) printf("c_bond %p TRI %llu  c_bond count = %d\n",
			              cb,tri_number(t,surf->interface),
			   (int)size_of_pointers((POINTER *)Tri_cross_list(t)));
		    }

		    if (!Tri_cross_list(t))
			continue;
		    if (!(cdt = (Cdt *)store(sizeof(Cdt))))
		    {
			screen("ERROR in cdt_retriangulate_surface_"
			       "along_c_curves(), cannot get memory\n");
			clean_up(ERROR);
		    }
		
		    cdt->next = cdt_tris;
		    cdt->tri = t;
		    cdt->surf = surf;
		    cdt->isurf = isurf;
		    cdt->c_bond_list = Tri_cross_list(t);
		    Tri_cross_list(t) = NULL;
		    cdt_tris = cdt;
		}
	    }
	}

	    /* re-triangulate cdt_tris */

	for (cdt = cdt_tris; cdt; cdt = cdt->next)
	{	         
	    Tri_workspace(cdt->tri) = (POINTER)cdt;
	    if (!cdt_triangulate_one_tri(cdt))
	    {
		(void) printf("WARNING in "
		              "in cdt_retriangulate_surface_along_c_curves(), "
		              "cdt_triangulate_one_tri() failed!\n");
	        DEBUG_LEAVE(cdt_retriangulate_surface_along_c_curves)
		return FUNCTION_FAILED;
	    }
	}	         
		         

	    /* stitch cdts together on surface */

	for (cdt = cdt_tris;  cdt;  cdt = cdt->next)
	{
	    TRI   *tri = cdt->tri;
	    FT_POINT **p = Point_of_tri(tri);
	    int   side;
	    if (DEBUG)
	    {
		(void) printf("Stiching tri - ");
		print_tri(tri,surf->interface);
	    }
	    for (side = 0;  side < 3;  ++side)
	    {	        
	        Cdt_Side *tri_side = Tri_side_of_cdt(cdt) + side;
		int      s;
		if (tri_side->done_stiching)
		    continue;
		if (is_side_bdry(tri,side))
		{
		    BOND_TRI *bt = Bond_tri_on_side(tri,side);
		    if (tri_side->num_tris > 1)
		    {
			screen("ERROR in "
			       "cdt_retriangulate_surface_along_c_curves() "
			       "Code needed for tri-bond intersection\n");
			clean_up(ERROR);
		    }
		    t = tri_side->tris[0];
		    s = tri_side->tri_facing_side[0];
		    (void) link_tri_to_bond(bt,t,surf,bt->bond,bt->curve);
		}
		else
		{
		    Cdt	  *ncdt;
		    TRI   *ntri = Tri_on_side(tri,side);
	    	    FT_POINT **np = Point_of_tri(ntri);
		    int   nside;

		    for (nside = 0; nside < 3; ++nside)
		        if (Tri_on_side(ntri,nside) == tri)
			    break;
		    if (nside == 3)
		    {
		        screen("ERROR in "
			        "cdt_retriangulate_surface_along_c_curves(), "
			        "inconsistent neighbors on side %d\n",side);
		        (void) printf("tri - ");
		        print_tri(tri,surf->interface);
		        (void) printf("Neighbor on side %d - ",side);
		        print_tri(ntri,surf->interface);
		        clean_up(ERROR);
		    }
	            if (DEBUG)
	            {
		        (void) printf("Neighbor on side %d, nside = %d - ",
				      side,nside);
		        print_tri(ntri,surf->interface);
	            }
		    if ((ncdt = cdt_of_tri(ntri)) != NULL)
		    {
			Cdt_Side *ntri_side = Tri_side_of_cdt(ncdt) + nside;
		        if (p[side] == np[Next_m3(nside)])
		        {
		            int num_t = tri_side->num_tris;
		            for (i = 0; i < num_t; ++i)
		            {
				TRI *nt;
				int ns;
			        t = tri_side->tris[i];
			        s = tri_side->tri_facing_side[i];
			        nt = ntri_side->tris[num_t-1-i];
			        ns = ntri_side->tri_facing_side[num_t-1-i];
			        if (!is_side_bdry(t,s))
			            Tri_on_side(t,s) = nt;
			        if (!is_side_bdry(nt,ns))
			            Tri_on_side(nt,ns) = t;
		            }
		        }
		        else if (p[side] == np[nside])
		        {
		            screen("ERROR in "
			         "cdt_retriangulate_surface_along_c_curves(), "
			         "inconsistent normals for neighboring tris\n");
		            (void) printf("tri - ");
		            print_tri(tri,surf->interface);
		            (void) printf("Neighbor on side %d - ",side);
		            print_tri(ntri,surf->interface);
		            (void) printf("nside = %d\n",nside);
		            clean_up(ERROR);
		        }
		        else
		        {
		            screen("ERROR in "
			         "cdt_retriangulate_surface_along_c_curves(), "
			         "inconsistent neighboring points\n");
		            (void) printf("tri - ");
		            print_tri(tri,surf->interface);
		            (void) printf("Neighbor on side %d - ",side);
		            print_tri(ntri,surf->interface);
		            (void) printf("nside = %d\n",nside);
		            clean_up(ERROR);
		        }
		        ntri_side->done_stiching = YES;
		    }
		    else
		    {
		        t = tri_side->tris[0];
		        s = tri_side->tri_facing_side[0];
			Tri_on_side(t,s) = ntri;
			Tri_on_side(ntri,nside) = t;
		    }
		}
		tri_side->done_stiching = YES;
	    }	         
	}	         
	for (cdt = cdt_tris;  cdt;  cdt = cdt->next)
	    remove_tri_from_surface(cdt->tri,surf,NO);

	DEBUG_LEAVE(cdt_retriangulate_surface_along_c_curves)
	return FUNCTION_SUCCEEDED;
}		/*end cdt_retriangulate_surface_along_c_curves*/


/*
*		   c_bonds_intersect():
*
*	Check for intersection of two c_bonds known to lie in the sametri.
*/

LOCAL 	boolean  c_bonds_intersect(
	C_BOND *cb0,
	C_BOND *cb1)
{
	FT_POINT *p0s = cb0->start;
	FT_POINT *p0e = cb0->end;
	FT_POINT *p1s = cb1->start;
	FT_POINT *p1e = cb1->end;
	double *c0s, *c0e, *c1s, *c1e;
	double d00, d01, d02;
	double d10, d11, d12;
	double d0, d1, d2;
	double t0, t1, a00, a01, a10, a11, b0, b1;
	double D;

	if ((p0s==p1s) || (p0s==p1e) || (p0e==p1s) || (p0e==p1e))
	    return NO;
	c0s = Coords(p0s);
	c0e = Coords(p0e);
	c1s = Coords(p1s);
	c1e = Coords(p1e);
	d0  = c0s[0] - c1s[0];
	d1  = c0s[1] - c1s[1];
	d2  = c0s[2] - c1s[2];
	d00 = c0e[0] - c0s[0];
	d01 = c0e[1] - c0s[1];
	d02 = c0e[2] - c0s[2];
	d10 = c1e[0] - c1s[0];
	d11 = c1e[1] - c1s[1];
	d12 = c1e[2] - c1s[2];

	a00 = -QDot3d(d0,d0); a01 =  QDot3d(d0,d1); b0  =  QDot3d(d,d0);
	a10 = -a01;           a11 =  QDot3d(d1,d1); b1  =  QDot3d(d,d1);

	D =  (a00*a11 - a10*a01);
	if (D == 0.0) /*FLOATING POINT TOLERANCE TEST*/
	    return NO;
	t0 = (b0*a11 - b1*a01)/D;
	t1 = (b0*a10 - b1*a00)/D;
	if ((0.0 < t0 && t0 < 1.0) && (0.0 < t1 && t1 < 1.0))
	    return YES;
	return NO;
}		/*end c_bonds_intersect*/


LOCAL boolean cdt_triangulate_one_tri(
	Cdt		*cdt)
{
	Vertex               vts[MAX_SIZE_VTS];	/* vertices for delaunay tri */
	int                  ce[2*MAX_SIZE_C_EDGES];	/* c_bond edges */
	int                  nv;		/* number of vertices for tri */
	int                  nce;		/* number of c_bond edges */
	int                  i;
	static boolean       first = YES;
	static triangulateio in;
	static triangulateio out;

	DEBUG_ENTER(cdt_triangulate_one_tri)

	if (first == YES)
	{
	    first = NO;
	    in.Opts.poly = YES;
	    in.Opts.neighbors = YES;
	    in.Opts.edgesout = YES;
	    in.Opts.steiner = -1;
	    in.Opts.order = 1;
	    in.Opts.noholes = YES;
	}

	if (!prepare_v_and_e_for_cdt(cdt,vts,&nv,ce,&nce))
	{
	    (void) printf("WARNING in cdt_triangulate_one_tri(), "
	                  "prepare_v_and_e_for_cdt() failed!\n");
	    DEBUG_LEAVE(cdt_triangulate_one_tri)
	    return FUNCTION_FAILED;
	}
	if (DEBUG)
	{
	    (void) printf("Vertex list\n");
	    for (i = 0; i < nv; ++i)
	    {
		(void) printf("vertex %d\n",i);
		print_vertex("",vts+i);
	    }
	    for (i = 0; i < nce; ++i)
		(void) printf("edge %d, %d -> %d\n",i,ce[2*i],ce[2*i+1]);
	}

	if (2*nv > in.size_pointlist)
	{
	    if (in.pointlist != NULL)
		free(in.pointlist);
	    in.size_pointlist = (size_t)2*nv;
	    uni_array(&in.pointlist,in.size_pointlist,FLOAT);
	}
	for (i = 0; i < nv; ++i)
	{
	    in.pointlist[2*i] = vts[i].x;
	    in.pointlist[2*i+1] = vts[i].y;
	}
	if (2*nce > in.size_segmentlist)
	{
	    if (in.segmentlist != NULL)
		free(in.segmentlist);
	    in.size_segmentlist = (size_t)2*nce;
	    uni_array(&in.segmentlist,in.size_segmentlist,INT);
	}
	for (i = 0; i < nce; ++i)
	{
	    in.segmentlist[2*i] = ce[2*i];
	    in.segmentlist[2*i+1] = ce[2*i+1];
	}

	in.numberofpoints = nv;
	in.numberofsegments = nce;
	triangulate(&in,&out,NULL);
	install_tris_from_dtris(cdt,vts,&out);
	if (DEBUG)
	{
	    print_triangulateio(&out);
	    (void) printf("cdt after install_tris_from_dtris\n");
	    print_cdt(cdt);
	}

	DEBUG_LEAVE(cdt_triangulate_one_tri)
	return FUNCTION_SUCCEEDED;
}		/*end cdt_triangulate_one_tri*/

LOCAL 	boolean prepare_v_and_e_for_cdt(
	Cdt		*cdt,
	Vertex		*vts,
	int		*pnum_vs,
	int		*edges,
	int		*pnedges)
{
	C_BOND		**cbp, **cbp1;
	Cdt_Side	*tri_side;
	FT_POINT		*ps, *pe;
	double		*o;
	TRI		*tri = cdt->tri;
	Vertex		*v;
	double		xa[3], ya[3], za[3], magv;
	double           *c, *cp;
	const double     *sv;
	const double     *tnor;
	int		i, j, side;
	int             num_vs, nedges;
	int		itri;

	for (cbp = cdt->c_bond_list;  cbp && *cbp;  ++cbp)
	{
	    opaque_pointer((*cbp)->start) =  NULL;
	    opaque_pointer((*cbp)->end) = NULL;
	}

	for (cbp = cdt->c_bond_list;  cbp && *cbp;  ++cbp)
	{
	    for (cbp1 = cbp+1;  cbp1 && *cbp1;  ++cbp1)
	    {
		if (c_bonds_intersect(*cbp,*cbp1))
		{
		    *pnum_vs = 0;
		    *pnedges = 0;
		    (void) printf("WARNING in prepare_v_and_e_for_cdt(), "
		                  "two c_bonds intersect with each other!\n");
		    return FUNCTION_FAILED;
		}
	    }
	}

	for (num_vs = 0; num_vs < 3; ++num_vs)
	{
	    v = vts + num_vs;
	    v->point = Point_of_tri(tri)[num_vs];
	    v->tri_vertex_index = num_vs;
	    v->tri_side_index = -1;
	    v->cdt_side_index[num_vs] = 0;
	    v->cdt_side_index[Next_m3(num_vs)] = -1;
	    v->cdt_side_index[Prev_m3(num_vs)] = -1;
	    v->d2 = -HUGE_VAL;
	    opaque_pointer(v->point) = (POINTER)v;
	    v->index = num_vs;
	    tri_side = Tri_side_of_cdt(cdt) + num_vs;
	    tri_side->num_verts = 1;
	    tri_side->vertices[0] = v;
	    tri_side->done_stiching = NO;
	}

	nedges = 0;
	for (cbp = cdt->c_bond_list;  cbp && *cbp;  ++cbp)
	{
	    if (tri == (*cbp)->s[0].t)
		itri = 0;
	    else if (tri == (*cbp)->s[1].t)
		itri = 1;
	    else
	    {
		screen("ERROR in prepare_v_and_e_for_cdt(), "
		       "c_bond is not attatched to tri!\n");
		clean_up(ERROR);
	    }
	    ps = (*cbp)->start;
	    pe = (*cbp)->end;
	    if ((v = Point_vertex_pointer(ps)) == NULL)
	    {
	        C_SURF_FLAG flag_s;
	        flag_s = cs_flag_start((*cbp)->s[itri]);
		v = vts + num_vs;
		opaque_pointer(ps) = (POINTER)v;
		v->point = ps;
		v->index = num_vs++;
	        v->cdt_side_index[0] = -1;
	        v->cdt_side_index[1] = -1;
	        v->cdt_side_index[2] = -1;
		v->tri_vertex_index = -1;
		v->tri_side_index = -1;
		if (cs_on_bdry(flag_s) && cs_edge_vertex(flag_s))
		{
		    side = cs_tri_side_index(flag_s);
		    v->tri_side_index = side;
		    c = Coords(Point_of_tri(tri)[side]);
		    cp = Coords(ps);
		    v->d2 = sqr(cp[0]-c[0])+sqr(cp[1]-c[1])+sqr(cp[2]-c[2]);
		    tri_side = Tri_side_of_cdt(cdt) + side;
		    for (i = 1; i < tri_side->num_verts; ++i)
			if (tri_side->vertices[i]->d2 > v->d2)
			    break;
		    for (j = tri_side->num_verts; j > i; j--)
			tri_side->vertices[j] = tri_side->vertices[j-1];
		    tri_side->vertices[i] = v;
		    ++tri_side->num_verts;
		}
	    }
	    edges[2*nedges] = v->index;
	    if ((v = Point_vertex_pointer(pe)) == NULL)
	    {
	        C_SURF_FLAG flag_e;
	        flag_e = cs_flag_end((*cbp)->s[itri]);
		v = vts + num_vs;
		opaque_pointer(pe) = (POINTER)v;
		v->point = pe;
		v->index = num_vs++;
	        v->cdt_side_index[0] = -1;
	        v->cdt_side_index[1] = -1;
	        v->cdt_side_index[2] = -1;
		v->tri_vertex_index = -1;
		v->tri_side_index = -1;
		if (cs_on_bdry(flag_e) && cs_edge_vertex(flag_e))
		{
		    side = cs_tri_side_index(flag_e);
		    v->tri_side_index = side;
		    c = Coords(Point_of_tri(tri)[side]);
		    cp = Coords(pe);
		    v->d2 = sqr(cp[0]-c[0])+sqr(cp[1]-c[1])+sqr(cp[2]-c[2]);
		    tri_side = Tri_side_of_cdt(cdt) + side;
		    for (i = 1; i < tri_side->num_verts; ++i)
			if (tri_side->vertices[i]->d2 > v->d2)
			    break;
		    for (j = tri_side->num_verts; j > i; j--)
			tri_side->vertices[j] = tri_side->vertices[j-1];
		    tri_side->vertices[i] = v;
		    ++tri_side->num_verts;
		}
	    }
	    edges[2*nedges+1] = v->index;
	    ++nedges;
	}

	/* Add the edges along the boundary of tri */
	for (i = 0; i < 3; ++i)
	{
	    tri_side = Tri_side_of_cdt(cdt) + i;
	    tri_side->vertices[tri_side->num_verts++] = vts + Next_m3(i);
	    tri_side->num_tris = tri_side->num_verts-1;
	    for (j = 0; j < tri_side->num_tris; ++j, ++nedges)
	    {
		tri_side->vertices[j]->cdt_side_index[i] = j;
	        edges[2*nedges] = tri_side->vertices[j]->index;
	        edges[2*nedges+1] = tri_side->vertices[j+1]->index;
	    }
	    tri_side->vertices[j]->cdt_side_index[i] = j;
	}

	/* set 2-D coordinates on the plane of triangle */

	tnor = Tri_normal(tri);
	magv = Mag3d(tnor);
	for (i = 0; i < 3; ++i)
	    za[i] = tnor[i]/magv;
	i = (length_of_tri_side(tri,0) > length_of_tri_side(tri,1)) ? 0 : 1;
	i = (length_of_tri_side(tri,i) > length_of_tri_side(tri,2)) ? i : 2;
	sv = vector_on_tri_side(tri,i,NULL);
	magv = length_of_tri_side(tri,i);
	o = Coords(Point_of_tri(tri)[i]);
	for (i = 0; i < 3; ++i)
	    xa[i] = sv[i]/magv;
	Cross3d(za,xa,ya);
	magv = sqrt(Dot3d(ya,ya));
	for (i = 0; i < 3; ++i)
	    ya[i] /= magv;

	for (i = 0; i < num_vs; ++i)
	{
	    double *pt;
	    pt = Coords(vts[i].point);
	    vts[i].x=(pt[0]-o[0])*xa[0]+(pt[1]-o[1])*xa[1]+(pt[2]-o[2])*xa[2];
	    vts[i].y=(pt[0]-o[0])*ya[0]+(pt[1]-o[1])*ya[1]+(pt[2]-o[2])*ya[2];
	}

	if (DEBUG)
	{
	    (void) printf("After prepare_v_and_e_for_cdt():\n");
	    (void) printf("number of vertices  = %d\n",num_vs);
	    (void) printf("number of crx edges = %d\n",nedges);
	    for (i = 0; i < 3; ++i)
	    {
		(void) printf("side %d: num_vertices = %d\n",i,
			      Tri_side_of_cdt(cdt)[i].num_verts);
	    }
	}
	*pnum_vs = num_vs;
	*pnedges = nedges;
	return FUNCTION_SUCCEEDED;
}		/*end prepare_v_and_e_for_cdt*/


LOCAL void install_tris_from_dtris(
	Cdt           *cdt,
	Vertex        *v,
	triangulateio *out)
{
	C_BOND        **cb;
	Cdt_Side      *tri_side;
	SURFACE	      *s = cdt->surf;
	Vertex        *v0, *v1;
	INTERFACE     *cintfc = current_interface();
	TRI	      *nt;
	int           isurf, i, j, k, indx, side;
	size_t        sizest = size_of_state(cintfc);
	static TRI    **newtris = NULL;
	static size_t max_n_newtris = 0;
	
	if (out->numberoftriangles > max_n_newtris)
	{
	    if (newtris != NULL)
		free(newtris);
	    max_n_newtris = 2*out->numberoftriangles;
	    uni_array(&newtris,max_n_newtris,sizeof(TRI*));
	}

	/* Create new triangles */
	for (i = 0; i < out->numberoftriangles; ++i)
	{
	    FT_POINT *p0, *p1, *p2;
	    p0 = v[out->trianglelist[3*i]].point;
	    p1 = v[out->trianglelist[3*i+1]].point;
	    p2 = v[out->trianglelist[3*i+2]].point;
	    newtris[i] = make_tri(p0,p1,p2,NULL,NULL,NULL,0);
	    insert_tri_at_tail_of_list(newtris[i],s);
	}

	/* Set neighbors within the new set */
	for (i = 0; i < out->numberoftriangles; ++i)
	{
	    nt = newtris[i];
	    for (j = 0; j < 3; ++j)
	    {
		if ((indx = TriangulateNeighborOnSide(i,j,*out)) >= 0)
		    Tri_on_side(nt,j) = newtris[indx];
		else
		{
		    /* Indentify side of old tri containing the vertices */
		    int i0, i1;
		    v0 = Point_vertex_pointer(Point_of_tri(nt)[j]);
		    v1 = Point_vertex_pointer(Point_of_tri(nt)[Next_m3(j)]);
		    i0 = v0->tri_vertex_index;
		    i1 = v1->tri_vertex_index;
		    if ((i0 >= 0) && (i1 >= 0)) /* nt and tri share a side */
		    {
			int side = (i1 == Next_m3(i0)) ? i0 : i1;
	                tri_side = Tri_side_of_cdt(cdt) + side;
			tri_side->tris[0] = nt;
			tri_side->tri_facing_side[0] = j;
		    }
		    else
		    {
			side = (i0<0)?v0->tri_side_index:v1->tri_side_index;
	                tri_side = Tri_side_of_cdt(cdt) + side;
			k = min(v0->cdt_side_index[side],
				v1->cdt_side_index[side]);
			tri_side->tris[k] = nt;
			tri_side->tri_facing_side[k] = j;
		    }
		}
	    }
	}

	/* Insert bonds along the C_BONDs */
	sizest = size_of_state(cintfc);
	size_of_state(cintfc) = 0;
	isurf = cdt->isurf;
	for (cb = cdt->c_bond_list;  cb && *cb;  ++cb)
	{
	    FT_POINT    *ps = (*cb)->start, *pe = (*cb)->end;
	    BOND     *b;
	    BOND_TRI *btri;
	    int      is, ie;

	    if ((b = (*cb)->bond) == NULL)
		(*cb)->bond = b = Bond(ps,pe);

	    /* Find the triangles bordering the new bond */
	    for (i = 0; i < out->numberoftriangles; ++i)
	    {
	        nt = newtris[i];
		is = ie = -1;
		for (j = 0; j < 3; ++j)
		{
		    if (Point_of_tri(nt)[j] == ps)
			is = j;
		    if (Point_of_tri(nt)[j] == pe)
			ie = j;
		}
		if ((is < 0) || (ie < 0))
		    continue;
		side = (ie==Next_m3(is)) ? is : ie;
		btri = link_tri_to_bond(NULL,nt,s,b,NULL);
		left_start_btri_state(btri) =
		    left_start_c_bond_state(*cb)[isurf];
		right_start_btri_state(btri) =
		    right_start_c_bond_state(*cb)[isurf];
		left_end_btri_state(btri) = left_end_c_bond_state(*cb)[isurf];
		right_end_btri_state(btri) = right_end_c_bond_state(*cb)[isurf];
	    }
	}
	size_of_state(cintfc) = sizest;
}		/*end install_tris_from_dtris*/

LOCAL	boolean c_curve_out_comp_domain(
	C_CURVE   *c_curve)
{
	C_BOND    *cb;
	TRI       *tri;
	RECT_GRID *cgr = computational_grid(c_curve->interface);
	int       i,j;
	double     *p;
	double     *L = cgr->L;
	double     *U = cgr->U;

	for (cb = c_curve->first; cb != NULL; cb = cb->next)
	{
	    for (i = 0; i < 2; ++i)
	    {
		tri = cb->s[i].t;
		for (j = 0; j < 3; ++j)
		{
		    p = Coords(Point_of_tri(tri)[j]);
		    if ((L[0] <= p[0] && p[0] <= U[0]) &&
		        (L[1] <= p[1] && p[1] <= U[1]) &&
		        (L[2] <= p[2] && p[2] <= U[2]))
		    {
			return NO;
		    }
		}
	    }
	}
	return YES;
}	/* end c_curve_out_comp_domain */


LOCAL	void	set_states_at_crosses(
	Front *fr)
{
	INTERFACE	*intfc = fr->interf;
	C_CURVE		**pc;

	for (pc = intfc->c_curves; pc && *pc; ++pc)
	    set_states_at_cross_points(fr,*pc);
}	/* end set_states_at_crosses */

LOCAL	void	set_states_at_cross_points(
	Front	*fr,
	C_CURVE	*c_curve)
{
	C_BOND	   *cb;
	INTERFACE  *intfc = fr->interf;
	FT_POINT	   *p, *pt[3];
	TRI	   *tri;
	HYPER_SURF *hs[2];
	Locstate   sl[3], sr[3];
	Locstate   ans;
	double	   f[3];
	double	   *h = fr->rect_grid->h;
	int	   i, j;
	size_t     sizest = fr->sizest;

	hs[0] = Hyper_surf(c_curve->s[0]);
	hs[1] = Hyper_surf(c_curve->s[1]);
	cb = c_curve->first;
	for (i = 0; i < 2; ++i)
	{
	    tri = cb->s[i].t;
	    p = cb->start;
	    for (j = 0; j < 3; ++j)
	    {
		pt[j] = Point_of_tri(tri)[j];
		slsr(pt[j],Hyper_surf_element(tri),hs[i],&sl[j],&sr[j]);
	    }
	    set_weight_for_tri_interpolation(Coords(p),tri,f,h,hs[i]->interface);
	    ans = alloc_intfc_state(intfc,sizest);
	    left_start_c_bond_state(cb)[i] = ans;
	    if (!tri_interpolate_states(fr,f[0],f[1],f[2],
				        Coords(pt[0]),sl[0],
				        Coords(pt[1]),sl[1],
				        Coords(pt[2]),sl[2],ans))
	    {
		screen("ERROR in set_states_at_cross_points(), "
		       "tri_interpolate_states() fails on the negative side "
		       "of surface %d at start of c_curve\n",i);
		clean_up(ERROR);
	    }
	    ans = alloc_intfc_state(intfc,sizest);
	    right_start_c_bond_state(cb)[i] = ans;
	    if (!tri_interpolate_states(fr,f[0],f[1],f[2],
				        Coords(pt[0]),sr[0],
				        Coords(pt[1]),sr[1],
				        Coords(pt[2]),sr[2],ans))
	    {
		screen("ERROR in set_states_at_cross_points(), "
		       "tri_interpolate_states() fails on the positive side "
		       "of surface %d at start of c_curve\n",i);
		clean_up(ERROR);
	    }
	}
	for (; cb != NULL; cb = cb->next)
	{
	    if (cb->prev != NULL)
	    {
	        left_start_c_bond_state(cb)[0] =
		    left_end_c_bond_state(cb->prev)[0];
	        left_start_c_bond_state(cb)[1] =
		    left_end_c_bond_state(cb->prev)[1];
	        right_start_c_bond_state(cb)[0] =
		    right_end_c_bond_state(cb->prev)[0];
	        right_start_c_bond_state(cb)[1] =
		    right_end_c_bond_state(cb->prev)[1];
	    }
	    for (i = 0; i < 2; ++i)
	    {
	        tri = cb->s[i].t;
	        p = cb->end;
	        for (j = 0; j < 3; ++j)
	        {
		    pt[j] = Point_of_tri(tri)[j];
		    slsr(pt[j],Hyper_surf_element(tri),hs[i],&sl[j],&sr[j]);
	        }
	        set_weight_for_tri_interpolation(Coords(p),tri,f,h,
			                         hs[i]->interface);
	        ans = alloc_intfc_state(intfc,sizest);
	        left_end_c_bond_state(cb)[i] = ans;
	        if (!tri_interpolate_states(fr,f[0],f[1],f[2],
				            Coords(pt[0]),sl[0],
				            Coords(pt[1]),sl[1],
				            Coords(pt[2]),sl[2],ans))
	        {
		    screen("ERROR in set_states_at_cross_points(), "
		           "tri_interpolate_states() fails on the negative "
		           "side of surface %d\n",i);
		    clean_up(ERROR);
	        }
	        ans = alloc_intfc_state(intfc,sizest);
	        right_end_c_bond_state(cb)[i] = ans;
	        if (!tri_interpolate_states(fr,f[0],f[1],f[2],
				            Coords(pt[0]),sr[0],
				            Coords(pt[1]),sr[1],
				            Coords(pt[2]),sr[2],ans))
	        {
		    screen("ERROR in set_states_at_cross_points(), "
		           "tri_interpolate_states() fails on the positive "
		           "side of surface %d\n");
		    clean_up(ERROR);
	        }
	    }
	}
}	/* end set_states_at_cross_points */

LOCAL	void print_vertex(
	const char *indent,
	Vertex     *v)
{
	(void) printf("%sVertex structure 0x%p\n",indent,v);
	(void) printf("%s\tpoint = %llu %g %g %g\n"
	              "%s\tx, y = %g %g\n"
	              "%s\tindex = %d\n"
	              "%s\tcdt_side_index = %d %d %d\n"
	              "%s\ttri_vertex_index = %d\n"
	              "%s\ttri_side_index = %d\n"
	              "%s\td2 = %g\n",
		      indent,point_number(v->point),
		             Coords(v->point)[0],
		             Coords(v->point)[1],
		             Coords(v->point)[2],
		      indent,v->x,v->y,
		      indent,v->index,
		      indent,v->cdt_side_index[0],
		             v->cdt_side_index[1],
		             v->cdt_side_index[2],
		      indent,v->tri_vertex_index,
		      indent,v->tri_side_index,
		      indent,v->d2);
	(void) printf("%sEnd Vertex structure 0x%p\n",indent,v);
}		/*end print_vertex*/

LOCAL	void	print_cdt_side(
	Cdt_Side  *cdt_side,
	INTERFACE *intfc)
{
	int i;
	(void) printf("Cdt_Side structure 0x%p\n",cdt_side);
	(void) printf("\tnum_verts = %d\n",cdt_side->num_verts);
	for (i = 0; i < cdt_side->num_verts; ++i)
	    print_vertex("\t",cdt_side->vertices[i]);
	(void) printf("\tnum_tris = %d\n",cdt_side->num_tris);
	for (i = 0; i < cdt_side->num_tris; ++i)
	{
	    print_tri(cdt_side->tris[i],intfc);
	    (void) printf("tri_facing_side[%d] = %d\n",
			  i,cdt_side->tri_facing_side[i]);
	}
	(void) printf("done_stiching = %s\n",y_or_n(cdt_side->done_stiching));
	(void) printf("End Cdt_Side structure 0x%p\n",cdt_side);
}		/*end print_cdt_side*/

LOCAL	void	print_cdt(
	Cdt *cdt)
{
	INTERFACE *intfc = cdt->surf->interface;
	C_BOND    **cb;
	int       i;

	(void) printf("Cdt structure 0x%p\n",cdt);
	(void) printf("next = 0x%p\n",cdt->next);
	(void) printf("tri - ");
	print_tri(cdt->tri,cdt->surf->interface);
	(void) printf("isurf = %d\n",cdt->isurf);
	(void) printf("surf = %llu\n",surface_number(cdt->surf));
	(void) printf("Cdt_Sides's\n");
	for (i = 0; i < 3; ++i)
	{
	    (void) printf("side[%d] - ",i);
	    print_cdt_side(cdt->side+i,cdt->surf->interface);
	}
	(void) printf("C_BOND's\n");
	for (cb = cdt->c_bond_list; cb && *cb; ++cb)
	    print_c_bond(*cb,intfc);
	(void) printf("End Cdt structure 0x%p\n",cdt);
}		/*end print_cdt*/

LOCAL	void	print_nclist(
	NCLIST    *ncl,
	INTERFACE *intfc)
{
	int i;

	(void) printf("NCLIST structure 0x%p, prev 0x%p, next 0x%p\n",
		      ncl,ncl->prev,ncl->next);
	(void) printf("num_comps = %d\n",ncl->num_comps);
	(void) printf("num_physical_regions = %d\n",ncl->num_physical_regions);
	(void) printf("num_surfaces = %d\n",ncl->num_surfaces);
	(void) printf("physical_region = %s %s %s %s\n",
		      y_or_n(physical_region(ncl,0)),
		      y_or_n(physical_region(ncl,1)),
		      y_or_n(physical_region(ncl,2)),
		      y_or_n(physical_region(ncl,3)));
	(void) printf("physical_surf = %s %s %s %s\n",
		      y_or_n(physical_surf(ncl,0)),
		      y_or_n(physical_surf(ncl,1)),
		      y_or_n(physical_surf(ncl,2)),
		      y_or_n(physical_surf(ncl,3)));
	print_c_curve(ncl->c_curve,intfc);
	for (i = 0; i < 4; ++i)
	{
	    (void) printf("ncl->NS[%d] - ",i);
	    print_ncsurf(ncl->NS+i);
	}

	(void) printf("End NCLIST structure 0x%p\n",ncl);
}		/*end print_nclist*/

LOCAL	void	print_ncsurf(
	NCSURF *ncs)
{
	(void) printf("NCSURF structure 0x%p\n",ncs);
	(void) printf("surface = %llu\n",surface_number(ncs->s));
	(void) printf("orient = %s\n",orientation_name(ncs->orient));
	(void) printf("physical = %s\n",y_or_n(ncs->physical));
	(void) printf("area = %g\n",ncs->area);

	(void) printf("End NCSURF structure 0x%p\n",ncs);
}		/*end print_ncsurf*/
#endif /* defined(THREED) */
