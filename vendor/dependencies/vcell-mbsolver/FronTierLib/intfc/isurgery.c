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
*				trisurgery.c:
*
*       Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*/

#if defined(THREED)

#include <intfc/int.h>

enum {SURF_STITCH,CURV_STITCH};

enum {
	MAX_TRIS_AT_VERTEX =  100
};

struct _STITCH {
  	FT_POINT 		*start_pt;
  	FT_POINT		*end_pt;
	TRI             *detached_tri;
	BOND            *detached_bond;
  	int		detached_side;
};
typedef struct _STITCH STITCH;

struct _SEAM {
  	STITCH 	*stitch;
  	SURFACE *surface;
  	int	number_of_stitches;
  	FT_POINT 	*begin_pt;
  	FT_POINT	*finish_pt;
};
typedef struct _SEAM SEAM;

struct _SURGERY {
  	SEAM 		*seam1;
  	SEAM		*seam2;
  	TRI		**new_tris;
};
typedef struct _SURGERY SURGERY;


LOCAL   void 	align_seam_with_point(SEAM*,FT_POINT*,SEAM*);
LOCAL 	void 	sew_surface_to_curve(SURGERY*);
LOCAL	int	count_null_sides_of_surf(SURFACE*);
LOCAL	int	set_surface_seam(SURFACE*,SEAM*);
LOCAL	void 	set_curve_seam(CURVE*,SEAM*,ORIENTATION);
LOCAL	void 	print_seam(SEAM*);
LOCAL	TRI 	*bdry_adjacent_tri(TRI*,FT_POINT*,int);

EXPORT void identify_detached_surface_curve_pair(
	INTERFACE 	*intfc)
{
  	CURVE 		*detached_curv;
	SURFACE 	**surf,*detached_surf=NULL;
	int		num_sides = 0;
	ORIENTATION	orientation;
	SEAM		seam;
	SEAM		surface_seam;
	FT_POINT		*curv_node_pt;
	SEAM		curve_seam;
	int		num_bonds;
	SURGERY		surgery;
	DEBUG_ENTER(identify_detached_surface_curve_pair)

	/* Assumptions:							*/
	/* The interface contains exactly one surface with tris having 	*/
	/* NULL sides.  This surface is attached to exactly one curve.	*/

	for (surf = intfc->surfaces; surf && *surf; ++surf)
	{  
	    if ((num_sides = count_null_sides_of_surf(*surf)) == 0)
	        continue;

	    detached_surf = *surf;
	    if (detached_surf->pos_curves != NULL)
            {
		detached_curv = detached_surf->pos_curves[0];
		orientation = POSITIVE_ORIENTATION;
	    }
	    else if (detached_surf->neg_curves != NULL)
	    {
		detached_curv = detached_surf->neg_curves[0];
		orientation = NEGATIVE_ORIENTATION;
	    }

	    if (debugging("surgery"))
	    {
	        (void) printf("detached surf = %p  detached curve = %p  ",
			      detached_surf,detached_curv);
		print_orientation("orientation = ",orientation,"\n");
	    }
	    break;
	}

	if (detached_surf == NULL)
	{
	    DEBUG_LEAVE(identify_detached_surface_curve_pair)
	    return;
	}

	uni_array(&surface_seam.stitch,num_sides+100,sizeof(STITCH));
	if (set_surface_seam(detached_surf,&surface_seam) < num_sides) 
        {
	    (void) printf("ERROR in identify_detached_surface_curve_pair(),\n"
			  "the seam does not contain all null sides of "
			  "surface.\n");
	    clean_up(ERROR);
	}

	curv_node_pt = detached_curv->start->posn; 
	uni_array(&seam.stitch,surface_seam.number_of_stitches,sizeof(STITCH));
	align_seam_with_point(&surface_seam,curv_node_pt,&seam);
	free(surface_seam.stitch);

	num_bonds = detached_curv->num_points - 1;
	uni_array(&curve_seam.stitch,num_bonds,sizeof(STITCH));
	set_curve_seam(detached_curv,&curve_seam,orientation);

	if (debugging("surgery"))
	{
	    (void) printf("The detached surface SEAM, after alignment.\n");
	    print_seam(&seam);
	    (void) printf("The detached curve SEAM.\n");
	    print_seam(&curve_seam);
	}

	surgery.seam1 = &seam;
	surgery.seam2 = &curve_seam;

	sew_surface_to_curve(&surgery);

	free(seam.stitch);
	free(curve_seam.stitch);
	DEBUG_LEAVE(identify_detached_surface_curve_pair)
}		/*end identify_detached_surface_curve_pair*/

LOCAL 	void	sew_surface_to_curve(
	SURGERY *surgery)
{
  	SEAM 		*surf_seam = surgery->seam1;
	SEAM 		*curv_seam = surgery->seam2;
	FT_POINT		*surf_pt,*curv_pt,*other_pt;
	TRI		*new_tri,*prev_tri = NULL,*detached_tri;
	SURFACE		*detached_surf = surf_seam->surface;
	BOND		*detached_bond;
	BOND_TRI	**btris;
	int		s_index = 0, c_index = 0,i;
	int		work_on;
	double		surf_tri_measure,curv_tri_measure;
	int		s_stitches = surf_seam->number_of_stitches;
	int		c_stitches = curv_seam->number_of_stitches;
	int		num_tris = s_stitches + c_stitches;
	int		detached_side;
	DEBUG_ENTER(sew_surface_to_curve)

	uni_array(&surgery->new_tris,num_tris,sizeof(TRI*));

	for (i = 0; i < num_tris; ++i)
	{
	    surf_pt = surf_seam->stitch[s_index % s_stitches].start_pt;
	    curv_pt = curv_seam->stitch[c_index % c_stitches].start_pt;

	    surf_tri_measure = ((s_index < s_stitches) ?
	        distance_between_positions(Coords(curv_pt),
			Coords(surf_seam->stitch[s_index].end_pt),3) :
		HUGE_VAL);

	    curv_tri_measure = ((c_index < c_stitches) ?
		distance_between_positions(Coords(surf_pt),
			Coords(curv_seam->stitch[c_index].end_pt),3) :
	        HUGE_VAL);

	    if (surf_tri_measure < curv_tri_measure)
	        work_on = SURF_STITCH;
	    else
	        work_on = CURV_STITCH;

	    switch (work_on)
	    {
	    case SURF_STITCH:
	        other_pt = surf_seam->stitch[s_index].end_pt;
		detached_tri = surf_seam->stitch[s_index].detached_tri;
		detached_side = surf_seam->stitch[s_index++].detached_side;

		new_tri = make_tri(other_pt,surf_pt,curv_pt,
				   detached_tri,prev_tri,NULL,0);
		insert_tri_at_tail_of_list(new_tri,detached_surf);
		Tri_on_side(detached_tri,detached_side) = new_tri;
		surgery->new_tris[i] = new_tri;
		break;

	    case CURV_STITCH:
	        other_pt = curv_seam->stitch[c_index].end_pt;
		detached_bond = curv_seam->stitch[c_index++].detached_bond;

		new_tri = make_tri(surf_pt,curv_pt,other_pt,
				   prev_tri,detached_bond,NULL,2);
		insert_tri_at_tail_of_list(new_tri,detached_surf);
		surgery->new_tris[i] = new_tri;
		btris = Btris(detached_bond);
		while (*btris && ((*btris)->surface != detached_surf)) 
		    ++btris;

		(*btris)->tri = new_tri;
		break;
	    }

	    if (prev_tri != NULL)
	        Tri_on_side20(prev_tri) = new_tri;
	    prev_tri = new_tri;
	}	   

	Tri_on_side20(prev_tri) = surgery->new_tris[0];

	if (Boundary_tri(surgery->new_tris[0]))/*first new_tri was CURV_STITCH*/ 
	    Tri_on_side01(surgery->new_tris[0]) = prev_tri;

	else				       /*first new_tri was SURF_STITCH*/
	    Tri_on_side12(surgery->new_tris[0]) = prev_tri;

	free(surgery->new_tris);
	DEBUG_LEAVE(sew_surface_to_curve)
}		/*end sew_surface_to_curve*/

LOCAL   void 	align_seam_with_point(
	SEAM 		*unaligned,
	FT_POINT 		*point,
	SEAM 		*aligned)
{
  	int 	num_stitches,i,index_of_nearest_pt;
	double  	least_dist = 10000, dist;
	FT_POINT	*stitch_pt;

  	num_stitches = unaligned->number_of_stitches;
	for (i = 0; i < num_stitches; ++i)
	{
	    stitch_pt = unaligned->stitch[i].start_pt;
	    if ((dist = distance_between_positions(Coords(point),
	    	 Coords(stitch_pt),3)) < least_dist)
	    {
		least_dist = dist;
		index_of_nearest_pt = i;
	    }
	}

	for (i = 0; i < num_stitches; ++i)
	    ft_assign(&(aligned->stitch[i]),
		   &(unaligned->stitch[(i+index_of_nearest_pt)%num_stitches]),
		   sizeof(STITCH));
	    
	aligned->begin_pt = aligned->stitch[0].start_pt;
	aligned->finish_pt = aligned->stitch[num_stitches - 1].end_pt;
	aligned->number_of_stitches = num_stitches;
	aligned->surface = unaligned->surface;
}		/*end align_seam_with_point*/

EXPORT int next_null_sided_tri(
	TRI 		*start_tri,
	FT_POINT 		*vertex,
	TRI 		**next_tri)
{
	int 		next_side;
	TRI		*new_tri;
	int		count = 0;

	new_tri = start_tri;
	while (new_tri != NULL)
	{
	    next_side = Next_side_at_vertex(new_tri,vertex);
	    if (next_side == ERROR)
	    {
	    	screen("In next_null_sided_tri(), next_side is ERROR\n");
		printf("new_tri vertices: %p %p %p\n",
				Point_of_tri(new_tri)[0],
				Point_of_tri(new_tri)[1],
				Point_of_tri(new_tri)[2]);
		printf("vertex = %p\n",vertex);
		clean_up(ERROR);
	    }
	    if (Neighbor_on_side(new_tri,next_side) == NULL)
	    {
	    	*next_tri = new_tri;
		return next_side;
	    }
	    if (is_side_bdry(new_tri,next_side))
	    	new_tri = bdry_adjacent_tri(new_tri,vertex,next_side);
	    else
	    	new_tri = Tri_on_side(new_tri,next_side);
	    if (new_tri == start_tri)
	    	break;
	    if (count++ > MAX_TRIS_AT_VERTEX)
	    {
	    	screen("ERROR: dead loop in next_null_sided_tri()\n");
		clean_up(ERROR);
	    }
	}
	*next_tri = NULL;
	return ERROR;
}		/*end next_null_sided_tri*/

EXPORT int prev_null_sided_tri(
	TRI 		*start_tri,
	FT_POINT 		*vertex,
	TRI 		**prev_tri)
{
	int 		prev_side;
	TRI		*new_tri;
	int		count = 0;

	new_tri = start_tri;
	while (new_tri != NULL)
	{
	    prev_side = Prev_side_at_vertex(new_tri,vertex);
	    if (prev_side == ERROR)
	    {
	    	screen("In prev_null_sided_tri(), prev_side is ERROR\n");
		printf("new_tri vertices: %p %p %p\n",
				Point_of_tri(new_tri)[0],
				Point_of_tri(new_tri)[1],
				Point_of_tri(new_tri)[2]);
		printf("vertex = %p\n",vertex);
		clean_up(ERROR);
	    }
	    if (Neighbor_on_side(new_tri,prev_side) == NULL)
	    {
	    	*prev_tri = new_tri;
		return prev_side;
	    }
	    if (is_side_bdry(new_tri,prev_side))
	    	new_tri = bdry_adjacent_tri(new_tri,vertex,prev_side);
	    else
	    	new_tri = Tri_on_side(new_tri,prev_side);
	    if (new_tri == start_tri)
	    	break;
	    if (count++ > MAX_TRIS_AT_VERTEX)
	    {
	    	screen("ERROR: dead loop in prev_null_sided_tri()\n");
		clean_up(ERROR);
	    }
	}
	*prev_tri = NULL;
	return ERROR;
}		/*end prev_null_sided_tri*/

LOCAL	TRI *bdry_adjacent_tri(
	TRI *tri,
	FT_POINT *p,
	int side)
{
	BOND *b,*adj_b;
	BOND_TRI **btris;
	CURVE *c;

	if (!is_side_bdry(tri,side)) return NULL;

	b = Bond_on_side(tri,side);

	c = (*Btris(b))->curve;

	if (is_closed_curve(c))
	{
	    if (b == c->first && p == b->start)
	    	adj_b = c->last;
	    else if (b == c->last && p == b->end)
	    	adj_b = c->first;
	    else
	    	adj_b = (p == b->start) ? b->prev : b->next;
	}
	else
	    adj_b = (p == b->start) ? b->prev : b->next;

	if (adj_b == NULL) return NULL;

	for (btris = Btris(adj_b); btris && *btris; ++btris)
	{
	    if ((*btris)->surface == tri->surf)
	    	return (*btris)->tri;
	}
}	/* end bdry_adjacent_tri */

LOCAL	int	count_null_sides_of_surf(
	SURFACE *surf)
{
  	int 	i, num_null_sides = 0;
	TRI	*t;

	for (t=first_tri(surf); !at_end_of_tri_list(t,surf); t=t->next)
	    for (i = 0; i < 3; ++i)
		if (Tri_on_side(t,i)==NULL) ++num_null_sides;
	return num_null_sides;
}		/*end count_null_sides_of_surf*/

LOCAL	int	set_surface_seam(
	SURFACE	*surf,
	SEAM	*seam)
{
	TRI	*tri, *next_tri;
	int	side, next_side, i=0;
	STITCH	*stitch;

        for (tri=first_tri(surf); !at_end_of_tri_list(tri,surf); tri=tri->next)
	{
	    for (side = 0; side < 3; ++side)
		if (Tri_on_side(tri,side) == NULL)
		    break;
	    if (side < 3)
	        break;
	}

	seam->number_of_stitches = 1;
	seam->surface = surf;
	stitch = seam->stitch;

	stitch[0].start_pt = Point_of_tri(tri)[side];
	stitch[0].end_pt = Point_of_tri(tri)[Next_m3(side)];
	stitch[0].detached_tri = tri;
	stitch[0].detached_side = side;

	seam->begin_pt = seam->finish_pt = stitch[0].start_pt;

	while (stitch[i].end_pt != seam->finish_pt)
	{
	    FT_POINT *vertex;
	    ++i;
	    vertex = stitch[i].start_pt = stitch[i-1].end_pt;
	    next_side = next_null_sided_tri(tri,vertex,&next_tri);
	    stitch[i].end_pt = Point_of_tri(next_tri)[Next_m3(next_side)];
	    stitch[i].detached_tri = next_tri;
	    stitch[i].detached_side = next_side;
	    ++seam->number_of_stitches;
	    tri = next_tri;
	}
	return seam->number_of_stitches;
}		/*end set_surface_seam*/

LOCAL	void 	set_curve_seam(
	CURVE 	*curve,
	SEAM  	*seam,
	ORIENTATION orientation)
{
	int 	i,num_bonds;
	BOND 	*bond;
	STITCH	*stitch;

	num_bonds = curve->num_points - 1;
	seam->number_of_stitches = num_bonds;
	seam->surface = NULL;
	stitch = seam->stitch;

	switch (orientation)
	{
	case POSITIVE_ORIENTATION:
	    seam->begin_pt = curve->start->posn;
	    seam->finish_pt = curve->end->posn;

	    for (i = 0, bond = curve->first; i < num_bonds;
		 ++i, bond = bond->next)
	    {
	        stitch[i].start_pt = bond->start;
		stitch[i].end_pt = bond->end;
		stitch[i].detached_bond = bond;
		stitch[i].detached_side = -1; /*BOND*/
	    }
	    break;

	case NEGATIVE_ORIENTATION:
	    seam->begin_pt = curve->end->posn;
	    seam->finish_pt = curve->start->posn;

	    for (i = 0, bond = curve->last; i < num_bonds;
		 ++i, bond = bond->prev)
	    {
	        stitch[i].start_pt = bond->end;
		stitch[i].end_pt = bond->start;
		stitch[i].detached_bond = bond;
		stitch[i].detached_side = -1; /*BOND*/
	    }
	    break;

	case ORIENTATION_NOT_SET:
	default:
	    (void) printf("ERROR in set_curve_seam(), ORIENTATION_NOT_SET\n");
	    clean_up(ERROR);
	}
}		/*end set_curve_seam*/

LOCAL	void print_seam(
	SEAM 	*seam)
{
  	int i;
	double *start,*end;

	(void) printf("Start of SEAM %p\n",seam);
	(void) printf("  surface = %p    "
		      "number_of_stitches = %d\n",
		      seam->surface,seam->number_of_stitches);
	(void) printf("  begin_pt = ( %g %g %g )  "
		      "finish_pt = ( %g %g %g )\n",
		      Coords(seam->begin_pt)[0],Coords(seam->begin_pt)[1],
		      Coords(seam->begin_pt)[2],
		      Coords(seam->finish_pt)[0],Coords(seam->finish_pt)[1],
		      Coords(seam->finish_pt)[2]);
	(void) printf("\n");
	(void) printf("%3s  %30s  %30s  %8s\n",
		      "num","   start of stitch   ","    end of stitch    ",
		      "    nhbr  ");
	(void) printf("=");
	for (i = 0; i < 100; ++i) printf("=");
	printf("\n");
	for (i = 0; i < seam->number_of_stitches; ++i)
	{
	    start = Coords(seam->stitch[i].start_pt);
	    end = Coords(seam->stitch[i].end_pt);
	    (void) printf("%3d  ( %g %g %g )  "
			  "( %g %g %g )  ",
			  i,start[0],start[1],start[2],end[0],end[1],end[2]);
	    if (seam->stitch[i].detached_side == -1)
	        (void) printf(" BOND\n");
	    else if (seam->stitch[i].detached_side > 2)
	        (void) printf(" unknown\n");
	    else
		(void) printf(" TRI SIDE%d%d\n",
	                      seam->stitch[i].detached_side, 
	                      Next_m3(seam->stitch[i].detached_side)); 
	}
	(void) printf("End of SEAM %p.\n",seam);
	(void) printf("\n");
}		/*end print_seam*/
#endif /* defined(THREED) */
