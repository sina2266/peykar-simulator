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
*				fredist3d.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	This file contains the high-level routine for the re-distribution 
*	of triangles on given surfaces according to given criteria,
*
*			redistribute_surface()
*
*	and several elementary routines for splitting and joining
*	triangles.  Consider two adjacent triangles with a common side.
*	The two elementary operations, which are regarded as inverses of
*	each other, are to either split the common side by insertion of
*	a new point (and thus to split the triangles) or to shrink the
*	common side, and reduce it to a single point (and thus to reduce the
*	two triangles to a pair of edges).  A further elementary operation is
*	to flip the diagonal of a pair of adjacent triangles, ie of a
*	elementary quadralateral.
*
*	Also in this file are the three D additions to insert and delete_
*	point_in_bond.	With these three D additions, these two D routines
*	applied to curves of a three D interface, will modify the triangles
*	of the surfaces bounded by the curves and bonds, so that a valid
*	interface is produced.	These subroutines must not be called 
*	independently.
*
*	Each has the functionality as stated above.
*/


#define DEBUG_STRING	"redist3d"
#include <front/fdecs.h>		/* includes int.h, table.h */

	/* LOCAL Function Declarations */
LOCAL	void	redistribute_curve3d(CURVE*,Front*);
LOCAL	void	change_buffer_for_intfc(INTERFACE*);
LOCAL  	void    detect_and_move_points(SURFACE*);
LOCAL 	boolean need_to_redist_surface(Front*);

#if defined(DEBUG_STRING)
#define	DEBUG_FRONT(mesg,fr)	 debug_front(DEBUG_STRING,mesg,fr);
#else /* defined(DEBUG_STRING) */
#define	DEBUG_FRONT(mesg,fr)
#endif /* defined(DEBUG_STRING) */

LOCAL	boolean act_delete = NO;

/*
*			redistribute3d():
*
*	Returns one of the following values
*
*	BAD_REDISTRIBUTION  	
*		if Surface_redistribute() fails.
*	      	if intersections() fails after Surface_redistribute() succeeds. 
*		if restart_init is set and the restart interface is tangled.
*						
*	UNABLE_TO_UNTANGLE	
*		if redistributed intfc is tangled and scalar_unravel fails.
*		if Surface_redistribute() fails after scalar_unravel succeeds.
*		if intersections() fails after Surface_redistribute() succeeds.
*		if new tangle found after old tangle was successfully removed.
*
*	GOOD_REDISTRIBUTION	
*		otherwise.
*
*	If the do_redist flag is NO, the redistribution step
*	is omitted.
*/

#define MAX_SMOOTH_PARA  500



double	smooth_min_tri_area = 0.0;

LOCAL	void	set_tol_for_smooth(Front *fr)
{
	smooth_min_tri_area = 1.0e-2*sqrt(Min_tri_sqr_area(fr,GENERAL_WAVE));
}

EXPORT	boolean	compute_smooth_para(
	SMOOTH_PARA	*smooth_para,
	FT_POINT		*p,
	TRI		*tri,
	SURFACE		*s,
	SMOOTH_TOL	*stol)
{
	TRI	*tri2, *tri1, **ptris;
	int	n, j, k;
	FT_POINT	*p1, *p2, *p3;
	double	hv1[3], hv2[3], ang, tri_area;
	double	dist, lenk, max_cos, avep[3];
	
	if(Boundary_point(p))
	    return NO;

	for(k=0; k<3; k++)
	    avep[k] = 0.0;
	
	lenk = 0.0;
	max_cos = -1.0;

	n = set_tri_list_around_point(p, tri, &ptris, s->interface);

	for(j=0; j<n; j++)
	{
	    tri1 = ptris[j];
	    p1 = Point_of_tri(tri1)[Next_m3(Vertex_of_point(tri1,p))];
	    p2 = Point_of_tri(tri1)[Prev_m3(Vertex_of_point(tri1,p))];
		        
	    for(k=0; k<3; k++)
		avep[k] += Coords(p1)[k];
			
	    /*compute cone dist */
	    lenk += distance_between_positions(Coords(p1), Coords(p2), 3);

	    /*compute the smallest angle between two triangles in an edge. */
	    tri2 = ptris[(j-1+n)%n];
	    p3 = Point_of_tri(tri2)[Next_m3(Vertex_of_point(tri2,p))];
	    triangle_height_vec(hv1, Coords(p), Coords(p2), Coords(p1));
	    
	    /*if a very small tri is found, just skip it,  */
	    /*delete_min_side_of_tri will deal with this case. */
	    tri_area = 0.5*distance_between_positions(Coords(p),Coords(p1),
	    			3)*Mag3d(hv1);
	    if(tri_area < smooth_min_tri_area)
		return NO;

	    triangle_height_vec(hv2, Coords(p), Coords(p3), Coords(p1));
	    ang = Dot3d(hv1,hv2)/(Mag3d(hv1)*Mag3d(hv2));
	    if(ang > max_cos)
		max_cos = ang;
	}
		    
	for(k=0; k<3; k++)
	    avep[k] /= n;
		    
	dist = distance_between_positions(avep, Coords(p), 3);
		    
	/* dist/lenk, max_cos to test bad point */
	if(dist/lenk > stol->cone_ratio || max_cos > stol->max_cos)
	{
	    smooth_para->pt = p;
	    smooth_para->tri = tri;
	    ft_assign(smooth_para->avep, avep, 3*FLOAT);
	    smooth_para->cor = dist/lenk > stol->cone_ratio ? dist/lenk : -1.0;
	    smooth_para->cos = max_cos > stol->max_cos ? max_cos : -1.0;
	    
	    return YES;
	}

	return NO;
}	/* end compute_smooth_para */

EXPORT	boolean	compute_average_point(
	SMOOTH_PARA	*smooth_para,
	FT_POINT		*p,
	TRI		*tri,
	SURFACE		*s,
	SMOOTH_TOL	*stol)
{
	TRI	*tri1, **ptris;
	int	j, k, n;
	FT_POINT	*p1;
	double	avep[3];
	
	if(Boundary_point(p))
	    return NO;

	for(k=0; k<3; k++)
	    avep[k] = 0.0;
	
	n = set_tri_list_around_point(p, tri, &ptris, s->interface);

	for(j=0; j<n; j++)
	{
	    tri1 = ptris[j];
	    p1 = Point_of_tri(tri1)[Next_m3(Vertex_of_point(tri1,p))];
		        
	    for(k=0; k<3; k++)
		avep[k] += Coords(p1)[k];
	}
		    
	for(k=0; k<3; k++)
	    avep[k] /= n;
		    
	smooth_para->pt = p;
	smooth_para->tri = tri;
	ft_assign(smooth_para->avep, avep, 3*FLOAT);
	smooth_para->cor = -1.0;
	smooth_para->cos = -1.0;
	    
	return YES;
}

LOCAL  void    detect_and_move_points(
	SURFACE		*s)
{
	TRI		*tri;
	FT_POINT		*p;
	int		i, num;
	SMOOTH_PARA	smooth_que[MAX_SMOOTH_PARA];
	SMOOTH_TOL	stol;

	if (!(first_tri(s)))
	    return;

	/*smooth paramaters. */
	stol.cone_ratio = 0.2;
	stol.max_cos = 0.7;
	stol.alpha = sqrt(0.65);

	for (tri=first_tri(s); !at_end_of_tri_list(tri,s); tri=tri->next)
	{
	    for (i = 0; i < 3; i++)
	    {
		Index_of_point(Point_of_tri(tri)[i]) = -1;
	    }
	}

	num = 0;
	/*Compute the the parameters in each points */
	for (tri=first_tri(s); !at_end_of_tri_list(tri,s); tri=tri->next)
	{
	    for (i = 0; i < 3; i++)
	    {
		p = Point_of_tri(tri)[i];
		if(Boundary_point(p) || Index_of_point(p) != -1)
		    continue;

		Index_of_point(p) = 1;
		if(!compute_smooth_para(&smooth_que[num], p,tri,s,&stol))
		    continue;
		
		num++;
		if(num >= MAX_SMOOTH_PARA)
		{
	 	    printf("ERROR check_and_move_points, array is too large.\n");
		    clean_up(ERROR);
		}
	    }
	}

	if(num > 0)
	{
	    s->interface->modified = YES;
	}
	
	/*Apply Laplacian smooth */
	for(i=0; i<num; i++)
	    compute_point_smooth(&smooth_que[i], &stol, s->interface);

}	/* end tecplot_surface_states */

#define	MAX_CURVE_PTS	2000

/*must be called after sep_common_pt_for_open_bdry is called */

EXPORT  void	smooth_curve(
	CURVE	*c)
{
	FT_POINT	*p, *prevp, *nextp, *pt[MAX_CURVE_PTS];
	BOND	*b;
	int	i, k, num;
	double	nor[4], smo_fac = sqrt(0.6);
	double	newpt[MAX_CURVE_PTS][3];

	num = 0;
	for(b=c->first; b != NULL; b=b->next)
	{
	    p = b->start;
	    if(!point_outside_open_bdry(&k, nor, p, c->interface))
		continue;
	    
	    nextp = b->end;
	    
	    prevp = b->prev != NULL ? b->prev->start : NULL;
	    if(prevp == NULL)
		if(is_closed_curve(c))
		    prevp = c->last->start;
		else
		    continue;
	    
	    for(i=0; i<3; i++)
		newpt[num][i] = Coords(p)[i]*smo_fac + 
		    (Coords(prevp)[i] + Coords(nextp)[i])*(1.0-smo_fac)*0.5;
	    pt[num] = p;

	    num++;
	    if(num >= MAX_CURVE_PTS)
	    {
		printf("ERROR smooth_curves, too many points.\n");
		clean_up(ERROR);
	    }
	}

	for(i=0; i<num; i++)
	    ft_assign(Coords(pt[i]), newpt[i], 3*FLOAT);

}

EXPORT	double	min_null_pair_angle(
	double	*p0,
	double	*p1,
	double	*p2,
	double	*p3)
{
	double	hv1[3], hv2[3], ang;
	
	triangle_height_vec(hv1, p1, p0, p2);
	triangle_height_vec(hv2, p1, p3, p2);
	ang = Dot3d(hv1,hv2)/(Mag3d(hv1)*Mag3d(hv2));
	return	ang;
}

EXPORT	void	set_increased_buffer_grid(
	RECT_GRID       *rgr,
	const RECT_GRID *gr,
	int		inc,
	INTERFACE	*intfc)
{
	int		dlbuf[MAXD], dubuf[MAXD];
	int		i;

	if (rgr == NULL || gr == NULL)
	    return;
	rgr->dim = gr->dim;
	for (i = 0; i < gr->dim; ++i)
	{
	    dlbuf[i] = gr->lbuf[i];
	    rgr->GL[i] = gr->GL[i];
	    if(dlbuf[i] != 0 && 
	       rect_boundary_type(intfc,i,0) == SUBDOMAIN_BOUNDARY)
	    {
		dlbuf[i] += inc;
		rgr->GL[i] = gr->GL[i] - inc*gr->h[i];
	    }

	    dubuf[i] = gr->ubuf[i];
	    rgr->GU[i] = gr->GU[i];
	    if(dubuf[i] != 0 &&
	       rect_boundary_type(intfc,i,1) == SUBDOMAIN_BOUNDARY)
	    {
		dubuf[i] += inc;
		rgr->GU[i] = gr->GU[i] + inc*gr->h[i];
	    }
	    
	    rgr->gmax[i] = gr->gmax[i];
	    rgr->L[i] = gr->L[i];
	    rgr->U[i] = gr->U[i];
	}
	set_rect_grid(rgr->L,rgr->U,rgr->GL,rgr->GU,dlbuf,dubuf,
		      rgr->gmax,gr->dim,&gr->Remap,rgr);
}


LOCAL	void	change_buffer_for_intfc(
	INTERFACE	*intfc)
{
	RECT_GRID	Dual_grid, c_gr, *comp_grid;

	c_gr = Computational_grid(intfc);
	comp_grid = computational_grid(intfc);
	set_increased_buffer_grid(comp_grid, &c_gr, 3, intfc);
	set_dual_grid(&Dual_grid, comp_grid);
	set_expanded_grid(&Dual_grid, &topological_grid(intfc));
}

LOCAL	int	repeat_count = 0;

EXPORT	void	set_repeat(int	cnt)
{
	repeat_count = cnt;
}
EXPORT	int	recon_repeat()
{
	return repeat_count;
}

EXPORT int redistribute3d(
	Front		*fr,
	boolean		do_redist,
	boolean		restart_init)
{
	CROSS		*cross;
	INTERFACE	*intfc;
	boolean		status;
	boolean		force_redistribute = YES;
	int		k, i;
	static int	cnt = 0;
	boolean		do_auto_redist;

	DEBUG_ENTER(redistribute3d)
	start_clock("redistribute");

	do_auto_redist = NO;
	if (fr->Auto_Redist)
	{
	    do_auto_redist = pp_max_status(need_to_redist_surface(fr));
	}
	intfc = fr->interf;
	act_delete = NO;
	
	if(fr->step == -1)
	{
	    char  sn[30];
	    sprintf(sn, "pt_surface_bf%d", fr->step);
	    tecplot_interface_in_ball(sn, fr->interf);
	}

	
	if(fr->step > 0)
	{
	    SURFACE		**s;
	    CURVE		**c;

	    set_tol_for_smooth(fr);
	    cnt++;

	    for(i=0; i<2; i++)
	    {
		for (s = intfc->surfaces; s && *s; ++s)
		{
	    	    if(wave_type(*s) != FIRST_SCALAR_PHYSICS_WAVE_TYPE)
			continue;
	    
		    detect_and_move_points(*s);
		}
	
		if(NO)
		{
		    char  sn[30];
		    sprintf(sn, "pt_surface_bf%d_%d", fr->step, i);
		    tecplot_interface_in_ball(sn, fr->interf);
		}

		status = scatter_front(fr);

		if(!status)
		{
		    printf("ERROR in redistribute3d, scatter_front fails "
		    	   "after detect_and_move_points.\n");
		    clean_up(ERROR);
		}
	    }
	}

	if(fr->step == 0)
	    null_sides_are_consistent();

	if ((do_redist && fr->step > 0) || pp_max_status(fr->redis_flag) ||
	     do_auto_redist)
	{
	    int		num_iter=2;
	    
	    if(fr->step%10 == 0 || pp_max_status(fr->redis_flag))
		act_delete = NO;

	    for(i=0; i<num_iter; i++)
	    {
	        force_redistribute = YES;
	       
		status = (Interface_redistributed(fr) == YES) ?
		        FUNCTION_SUCCEEDED :
		        Surface_redistribute(fr,&force_redistribute);

		if(i != num_iter-1)
		    Interface_redistributed(fr) = NO;
		
		/*There are two cases,  */
		/*    1. redistribute_surface fails for one proc, intfc is ok, */
		/*    2. the scatter_front in surface_redistribute fails,  */
		/*    intfc is changed and is not consistent. */
	    	if (status == FUNCTION_FAILED)
		{
		    printf("WARNING in redistribute3d, Surface_redistribute "
		    	   "fails. %d  %d\n", fr->step, i);
		    
		    stop_clock("redistribute");
		    DEBUG_LEAVE(redistribute3d)
		    return BAD_REDISTRIBUTION;
		}

		if(debugging("pt_surface"))
		    clean_up(0);
	    }

	    intfc = fr->interf;
	    fr->redis_flag = NO;
	}

	if (Tracking_algorithm(fr) == LOCALLY_GRID_BASED_TRACKING)
	{

	    if(fr->step == 0)
	        null_sides_are_consistent();

	    if(recon_repeat() >= 1)
	    {
	    RECT_GRID	sav_c_gr, sav_t_gr, *sav_fr_gr;

	    sav_c_gr = Computational_grid(intfc);
	    sav_t_gr = topological_grid(intfc);
	    
	    /*use a large grid, scatter_front will construct a large  */
	    /*buffer for intfc. */
	    change_buffer_for_intfc(intfc);
	    
	    status = scatter_front(fr);
	
	    if(fr->step == 0)
	        null_sides_are_consistent();
	
	    sav_fr_gr = fr->rect_grid;
	    fr->rect_grid = computational_grid(intfc);
   
	    /*repair_intfc_at_crossings3d */
	    status = repair_front_at_grid_crossing(fr);
	
	    /*recover the previous grid. */
	    fr->rect_grid = sav_fr_gr;
	    Computational_grid(intfc) = sav_c_gr;
	    topological_grid(intfc) = sav_t_gr;
	
	    Computational_grid(fr->interf) = sav_c_gr;
	    topological_grid(fr->interf) = sav_t_gr;
	    }
	    else
	    {
		status = repair_front_at_grid_crossing(fr);
	    }

	    if(fr->step == 0)
	        null_sides_are_consistent();

	    if (!status)
                return UNABLE_TO_UNTANGLE;

	    start_clock("scatter_front");
	    
	    status = scatter_front(fr);
	    
	    stop_clock("scatter_front");
	
	    /*if(fr->step == 1615) */
		/*clean_up(0); */
	    
	    if (!status)
	    {
		printf("WARNING, scatter_front fails after "
		       "locally reconstruction in redistribute3d.\n");
		
		stop_clock("redistribute");
		DEBUG_LEAVE(redistribute3d)
		return INCONSISTENT_RECONSTRUCTION;
	    }
	    
	    if(fr->step == 0)
	        null_sides_are_consistent();

	    stop_clock("redistribute");
	    DEBUG_LEAVE(redistribute3d)
	    return GOOD_REDISTRIBUTION;
	}

	/*for LGB belowing is never reached. */

	if (debugging("gvrdst3d"))
	{
	    char s[120];

	    (void) sprintf(s,"before-intersect-ts%d",fr->step);
	    gview_plot_interface(s,fr->interf);
	}
	DEBUG_FRONT("before intersections check",fr)

		/* Check for Intersections in Front */

	  /* intersections does one of  the following:
	     1) returns NO if topology construction fails;
	     2) crashes if add_to_pointers() fails;
	     3) returns YES otherwise.
	  */

	if (pp_min_status(intersections(intfc,&cross,YES)) == FUNCTION_FAILED)
	{
	    stop_clock("redistribute");
	    (void) printf("WARNING in redistribute3d(), "
	                  "intersections() failed\n");
	    print_interface(intfc);
	    DEBUG_LEAVE(redistribute3d)
	    return BAD_REDISTRIBUTION;
	}

	if (debugging("gvrdst3d"))
	{
	    char s[120];

	    (void) sprintf(s,"before-untangle-ts%d",fr->step);
	    gview_plot_interface(s,fr->interf);
	}

	if (interface_is_tangled(cross))
	{
	    static const int Max_nattemps = 3;
	    int              nattemps;
	    (void) print_number_of_tangles("",intfc,cross);
	    start_clock("untangle");
	    if (restart_init) 
	    {
		stop_clock("untangle");
		stop_clock("redistribute");
		(void) printf("WARNING in redistribute(), "
		              "Restart interface tangled, cannot continue\n");
		DEBUG_LEAVE(redistribute3d)
		return BAD_REDISTRIBUTION;
	    }

	    nattemps = 0;
	    while (cross) 
	    {
		++nattemps;
		if (!scalar_unravel_3d(fr,&cross))
		{
		    stop_clock("untangle");
		    stop_clock("redistribute");
		    (void) printf("WARNING in redistribute3d(), "
		                  "scalar_unravel_3d() failed\n");
		    DEBUG_LEAVE(redistribute3d)
		    return UNABLE_TO_UNTANGLE;
		}
		force_redistribute = YES;
		if (!Surface_redistribute(fr,&force_redistribute))
		{
		    stop_clock("untangle");
		    stop_clock("redistribute");
		    (void) printf("WARNING in redistribute3d(), after "
		                  "untangling Surface_redistribute() failed\n");
		    DEBUG_LEAVE(redistribute3d)
		    return UNABLE_TO_UNTANGLE;
		}
		intfc = fr->interf;
		if (!pp_min_status(intersections(intfc,&cross,YES)))
		{
		    (void) printf("WARNING in redistribute3d(), "
		                  "After untangle, intersections() failed\n");
		    DEBUG_LEAVE(redistribute3d)
		    return UNABLE_TO_UNTANGLE;
		}
		if (interface_is_tangled(cross))
		{
		    if (nattemps>=Max_nattemps)
		    {
		        (void) printf("WARNING in redistribute3d(), "
				      "After untangle, intersections() finds "
				      "new tangle, too many attemps\n");
		        DEBUG_LEAVE(redistribute3d)
		        return UNABLE_TO_UNTANGLE;
		    }
		    else
		    {
		        (void) printf("WARNING in redistribute3d(), "
				      "After untangle, intersections() finds "
				      "new tangle, will attempt to untangle\n");
		    }
		}
		
	    }
	    stop_clock("untangle");
	}

	if (debugging("gvrdst3d"))
	{
	    char s[120];

	    (void) sprintf(s,"after-untangle-ts%d",fr->step);
	    gview_plot_interface(s,fr->interf);
	}

	stop_clock("redistribute");
	DEBUG_LEAVE(redistribute3d)
	return GOOD_REDISTRIBUTION;
}		/*end redistribute3d*/

/*
* 		surface_redistribute()
*
* 	The choice of name surface_redistribute() clashes in an obvious 
* 	way with the name redistribute_surface().  
*/


EXPORT boolean surface_redistribute(
	Front		*fr,
	boolean		*force_redist)
{
	SURFACE		**s;
	CURVE		**c;
	boolean		status, min_status;
	boolean		redist_non_vec_cur, redist_vec_cur;
	boolean		called_redistribute;	/* for parallel sychronization */
	TRI_REDIST_PARAMS tri_params;
	int wc;

	DEBUG_ENTER(surface_redistribute)

		/* Check on redistribution conditions */

	if (*force_redist)
	{
	    redist_non_vec_cur = redist_vec_cur = YES;
	    *force_redist = NO;
	}
	else if (Redistribution_count(fr) < 0)
	{
	    redist_non_vec_cur = redist_vec_cur = NO;
	}
	else
	{
	    redist_vec_cur = redist_needed(fr,VECTOR_WAVE);
	    redist_non_vec_cur = redist_needed(fr,GENERAL_WAVE);
	}
	++Redistribution_count(fr);

		/* Redistribute vector surfaces */
	
	set_size_of_intfc_state(size_of_state(fr->interf));
	set_copy_intfc_states(YES);
	set_current_interface(fr->interf);

	status = YES;
	if (redist_vec_cur == YES)
	{
	    for (s = fr->interf->surfaces; s && *s ; ++s) 
	      {   
	    	if ((!omit_redistribution(*s)) &&
		    (wave_type(*s) >= FIRST_VECTOR_PHYSICS_WAVE_TYPE))
		{
		    wc = wave_type(*s)< FIRST_VECTOR_PHYSICS_WAVE_TYPE ?
					GENERAL_WAVE : VECTOR_WAVE;
		    tri_params.max_sqr_area = Max_tri_sqr_area(fr,wc);
		    tri_params.min_sqr_area = Min_tri_sqr_area(fr,wc);
		    tri_params.max_sqr_length = 
				Max_scaled_tri_side_sqr_length(fr);
		    tri_params.aspect_tol2 = 
				sqr(Aspect_ratio_tolerance(fr,wc));
	    	    if (!redistribute_surface(*s,fr->rect_grid,tri_params))
			status = NO;
		}
	    }
	    interface_reconstructed(fr->interf) = NO;
	}
	
	make_interface_topology_lists(fr->interf);
	
	if (redist_non_vec_cur == YES)
	{
	    for (s = fr->interf->surfaces; s && *s ; ++s) 
	    {
	    	if ((!omit_redistribution(*s)) &&
	    	    (wave_type(*s) >= FIRST_PHYSICS_WAVE_TYPE ||
		     wave_type(*s) == GROWING_BODY_BOUNDARY) &&
	    	    (wave_type(*s) < FIRST_VECTOR_PHYSICS_WAVE_TYPE) &&
		    (wave_type(*s) != ELASTIC_BOUNDARY))
		{
		    start_clock("redistribute_surface");
		    wc = wave_type(*s)< FIRST_VECTOR_PHYSICS_WAVE_TYPE ?
					GENERAL_WAVE : VECTOR_WAVE;
		    tri_params.max_sqr_area = Max_tri_sqr_area(fr,wc);
		    tri_params.min_sqr_area = Min_tri_sqr_area(fr,wc);
		    tri_params.max_sqr_length = 
				Max_scaled_tri_side_sqr_length(fr);
		    tri_params.aspect_tol2 = 
				sqr(Aspect_ratio_tolerance(fr,wc));
		    if (!redistribute_surface(*s,fr->rect_grid,tri_params))
			status = NO;
		    stop_clock("redistribute_surface");
		}
	    }
	    interface_reconstructed(fr->interf) = NO;
	}
	
	make_interface_topology_lists(fr->interf);

	/*insert_point_in_tri_side fails or delete_min_side_of_tri fails. */
	if(status == NO)
	{
	    printf("WARNING after #surface_redistribute, i"
	    	   "status is NO in step %d\n", fr->step);
	}
	min_status = pp_min_status(status);
	if (min_status == NO)
	{
	    (void) printf("WARNING in surface_redistribute(), "
		          "redistribute_surface(), failed\n");
	    DEBUG_LEAVE(surface_redistribute)
	    return min_status;
	}

	start_clock("scatter_front");
	status = scatter_front(fr);
	stop_clock("scatter_front");
	
	if (!status)
	{
	    (void) printf("WARNING in surface_redistribute(), "
	    	          "scatter_front() failed\n");
	    DEBUG_LEAVE(surface_redistribute)
	    return status;
	}

	set_current_interface(fr->interf);

	Interface_redistributed(fr) =
	    ((redist_vec_cur==YES) || (redist_non_vec_cur==YES)) ? YES : NO;

	DEBUG_LEAVE(surface_redistribute)
	return status;
}		/*end surface_redistribute*/

LOCAL boolean need_to_redist_surface(
	Front *fr)
{
	SURFACE **s;
	RECT_GRID *gr = fr->rect_grid;
	TRI *tri;
	TRI_REDIST_PARAMS tri_params;
	int wc;

	for (s = fr->interf->surfaces; s && *s ; ++s) 
	{
	    if ((!omit_redistribution(*s)) &&
	    	(wave_type(*s) >= FIRST_PHYSICS_WAVE_TYPE ||
		 wave_type(*s) == GROWING_BODY_BOUNDARY) &&
	    	 (wave_type(*s) < FIRST_VECTOR_PHYSICS_WAVE_TYPE))
	    {
		wc = GENERAL_WAVE;
		tri_params.max_sqr_area = Max_tri_sqr_area(fr,wc);
		tri_params.min_sqr_area = Min_tri_sqr_area(fr,wc);
		tri_params.max_sqr_length = 
				Max_scaled_tri_side_sqr_length(fr);
		tri_params.aspect_tol2 = 
				sqr(Aspect_ratio_tolerance(fr,wc));
		if (surface_needs_redist(*s,fr->rect_grid,tri_params))
		    return YES;
	    }
	}
	return NO;
}

LOCAL void redistribute_curve3d(
	CURVE		*c,
	Front		*fr)
{

	BOND		*b;
	static boolean	first = YES;
	static double	max_b_length, min_b_length;


	if (first == YES)
	{
	    first = NO;
	    max_b_length = Max_bond_len(fr,GENERAL_WAVE);
	    min_b_length = Min_bond_len(fr,GENERAL_WAVE);
	}
       
	if (debugging("b_length"))
	{
	    (void) printf("max_b_length = %g, min_b_length = %g\n",
	                  max_b_length,min_b_length);
	    detail_of_curve(c);
	}
	
	for (b = c->first; b != NULL; b = b->next)
	{
	    if (b->length >= max_b_length)
	    {
	        FT_POINT *pm = Point(NULL);
		int   i;
	        for (i = 0; i < 3; ++i)
		    Coords(pm)[i] = 0.5*(Coords(b->start)[i]+Coords(b->end)[i]);
	        (void) insert_point_in_bond(pm,b,c);
		b = b->next;
	    }
	    else if (hsbdry_type(c) < FIRST_PHYSICS_HSBDRY_TYPE)
	    {
	        if (b->length <= min_b_length)
		{
		    BOND *bp, *bn;
		    if ((bp = b->prev))
		    {
			if (!delete_start_of_bond(b,c))
		        {
			    screen("ERROR in redistribute_curve3d(), "
			           "delete_start_of_bond() failed\n");
			    clean_up(ERROR);
		        }
			b = bp;
		    }
		    else if ((bn = b->next))
		    {
			if (!delete_end_of_bond(b,c))
		        {
			    screen("ERROR in redistribute_curve3d(), "
			           "delete_end_of_bond() failed\n");
			    clean_up(ERROR);
		        }
			b = bn;
		    }
		}
	    }
	}
	if (debugging("redist_curve"))
	{
	    detail_of_curve(c);
	    summarize_interface("redist_curve3d","exit",c->interface,
				XY_PLANE,"redist_curve3d","exit");
	}
}		/*end redistribute_curve3d */

EXPORT	boolean	point_outside_open_bdry(
	int		*k,
	double		*nor,
	FT_POINT		*p,
	INTERFACE	*intfc)
{
	int		i;
	RECT_GRID	*gr = computational_grid(intfc);
	double		tol = 1.0e-4;

	zero_scalar(nor, 3*FLOAT);
	for(i=0; i<3; i++)
	{
	    if(rect_boundary_type(intfc,i,0) == OPEN_BOUNDARY && 
	       Coords(p)[i] < gr->VL[i] + gr->h[i])
	    {
		*k = i;
		nor[i] = -1.0;
		nor[3] = (gr->VL[i] - Coords(p)[i])/gr->h[i];
		return YES;
	    }
	    if(rect_boundary_type(intfc,i,1) == OPEN_BOUNDARY && 
	       Coords(p)[i] > gr->VU[i] - gr->h[i])
	    {
		*k = i;
		nor[i] = 1.0;
		nor[3] = (Coords(p)[i] - gr->VU[i])/gr->h[i];
		return YES;
	    }
	}
	return NO;
}	/* end point_outside_open_bdry */


/*height vector in edge p p2 */
EXPORT	void	triangle_height_vec(
	double		*hv,
	double		*p,
	double		*p1,
	double		*p2)
{
	double	v[3], nor[3], len;
	int	k;

	difference(p1, p, v, 3);
	difference(p2, p, nor, 3);
	len = Dot3d(v,nor)/Dot3d(nor,nor);
	for(k=0; k<3; k++)
	    hv[k] = v[k] - len*nor[k];
}


EXPORT  void    tecplot_interface_in_ball(
	const char	*bname,
	INTERFACE	*intfc)
{
	SURFACE	**s;

	for (s = intfc->surfaces; s && *s; ++s)
	{
	    if(wave_type(*s) < FIRST_PHYSICS_WAVE_TYPE)
	        continue;
	    printf("#show surface in ball\n");
	    tecplot_surface_in_ball(bname,*s);
	    break;
	}

}	/* end tecplot_interface */

