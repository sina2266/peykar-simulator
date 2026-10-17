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
*				fadv.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*/

#define DEBUG_STRING    "fadv"
#include <front/fdecs.h>		/* includes int.h, table.h */

	/* LOCAL Function Declarations */
LOCAL	boolean	BothSidesActive(HYPER_SURF*,Front*);

LOCAL	int	advance_front1d(double,double*,Front*,Front**,POINTER);
LOCAL	void	EnforceFlowSpecifedStates1d(Front*);

LOCAL	int	advance_front2d(double,double*,Front*,Front**,POINTER);
LOCAL	int	delete_phys_curves_at_old_node(NODE*,int);
LOCAL	int	node_modify_time_step(NODE*,Front*,double*,int);
LOCAL	void	EnforceFlowSpecifedStates2d(Front*);
LOCAL	void	check_bond_lengths(INTERFACE*);

LOCAL 	int 	advance_front3d_tracking_control(double,double*,Front*,
						 Front**,POINTER);

LOCAL 	int 	hybrid_advance_front3d(double,double*,Front*,Front**,POINTER);
LOCAL 	int 	mixed_advance_front3d(double,double*,Front*,Front**,POINTER);
LOCAL	int	preserve_front_advance_front3d(double,double*,Front*,
					       Front**,POINTER);
LOCAL	int	propagate_3d_front(POINTER,Front*,Front*,double,double*,boolean);
LOCAL   void    init_intfc_curvature3d1(Front*,INTERFACE*); /*for serial mode */
LOCAL   void    init_intfc_curvature3d2(Front*,INTERFACE*);
LOCAL   int	propagate_node_points(Front*,Front*,POINTER,double,double*);
LOCAL 	int 	propagate_points_tangentially(Front*,Front*,int,double,double*,
						  int);
LOCAL	int	reconstruct_front_advance_front3d(double,double*,Front*,
						  Front**,POINTER);
LOCAL	void	EnforceFlowSpecifedStates3d(Front*);
LOCAL	void	debug_propagate_3d_front(Front*);
LOCAL	void	propagate_curve_points(Front*,Front*,POINTER,double);
LOCAL	void	unfold_interface_section(FT_POINT*,FT_POINT*,TRI*,TRI*,
	                                 SURFACE*,SURFACE*);
LOCAL   void    propagate_surface_points(Front*,Front*,POINTER,double,double*);

LOCAL	void	detach_and_propagate_curves(Front*,Front*,POINTER,double);
LOCAL	void 	check_2d_orientation(INTERFACE*);

EXPORT	void	set_advance_front(
	INIT_DATA	*init,
	Front		*front)
{
	int dim = front->rect_grid->dim;
	boolean efss,mvfm;

	front->_EnforceFlowSpecifedStates = NULL;
	efss = ((init==NULL) || (enforce_flow_specified_states(init)==YES)) ?
		YES : NO;
	front->movingframe = ((init==NULL) || (movingframe(init)==YES)) 
				? YES : NO;
	switch (dim)
	{
	case 1:
	    front->_pre_advance_front = NULL;
	    front->_advance_front = advance_front1d;
	    if (efss == YES)
	        front->_EnforceFlowSpecifedStates = EnforceFlowSpecifedStates1d;
	    break;
	case 2:
#if defined(USE_OVERTURE)
            front->_normal_advance_front = normal_advance_front2d;
            front->_tangnt_advance_front = tangnt_advance_front2d;
            front->_redist_advance_front = redist_advance_front2d;
#else /* if defined(USE_OVERTURE) */
	    front->_advance_front = advance_front2d;
#endif /* if defined(USE_OVERTURE) */
	    front->_pre_advance_front = FrontPreAdvance;
	    if (efss == YES)
	        front->_EnforceFlowSpecifedStates = EnforceFlowSpecifedStates2d;
	    break;
	case 3:
	    front->_pre_advance_front = NULL;
	    front->_advance_front = advance_front3d_tracking_control;
	    if (efss == YES)
	        front->_EnforceFlowSpecifedStates = EnforceFlowSpecifedStates3d;
	    break;
	}
}		/*end set_advance_front*/


EXPORT	int	return_advance_front(
	Front		*front,
	Front		**newfront,
	int		status,
	const char	*fname)
{
#if defined(USE_OVERTURE)
        status = status;
#else   /* if defined(USE_OVERTURE)  */
	if (front->pp_grid && front->pp_grid->nn > 1)
 	    status = syncronize_time_step_status(status,front->pp_grid);
#endif /*  if defined(USE_OVERTURE)  */

	if (debugging("final_front"))
	{
	    print_Front_structure(front);
	    print_Front_structure(*newfront);
	}
	if (status != GOOD_STEP)
	{
	    free_front(*newfront);
	    *newfront = NULL;
	}
	else if (front->_EnforceFlowSpecifedStates != NULL)
	{
	    (*front->_EnforceFlowSpecifedStates)(*newfront);
	}
	debug_front("final_front","after EnforceFlowSpecifedStates():",
	           *newfront);
	debug_print("front","Left %s\n",fname);
	return status;
}		/*end return_advance_front*/

LOCAL	void	EnforceFlowSpecifedStates1d(
	Front	*fr)
{
	INTERFACE	*intfc;
	HYPER_SURF	*hs;
	FT_POINT		**p;
	Locstate	sl, sr;

	if ((fr==NULL) || (Fsr_list(fr)==NULL) || (fr->rect_grid->dim!=1))
	    return;

	intfc = fr->interf;
	for (p = intfc->points; p && *p; ++p)
	{
	    hs = Hyper_surf(*p);
	    if (BothSidesActive(hs,fr) == NO)
	    {
	       	slsr(*p,NULL,Hyper_surf(*p),&sl,&sr);
	    	(void) RegionIsFlowSpecified(sl,NULL,Coords(*p),
	                                 negative_component(hs),NO_COMP,fr);
	    	(void) RegionIsFlowSpecified(sr,NULL,Coords(*p),
	                                 positive_component(hs),NO_COMP,fr);
	    }
	}
}		/*end EnforceFlowSpecifedStates1d*/

LOCAL	void	EnforceFlowSpecifedStates2d(
	Front	*fr)
{
	INTERFACE	*intfc;
	CURVE		**c;
	FT_POINT		*p;
	HYPER_SURF	*hs;
	BOND		*b;
	Locstate	sl, sr;

	if ((fr==NULL) || (Fsr_list(fr)==NULL) || (fr->rect_grid->dim!=2))
	    return;

	intfc = fr->interf;
	for (c = intfc->curves; c && *c; ++c)
	{
	    hs = Hyper_surf(*c);
	    if (is_subdomain_boundary(hs) || is_passive_boundary(hs))
	        continue;
	    if (BothSidesActive(hs,fr) == NO)
	    {
	    	b = (*c)->first;
	    	p = b->start;
	       	slsr(p,Hyper_surf_element(b),hs,&sl,&sr);
	        (void) RegionIsFlowSpecified(sl,NULL,Coords(p),
	                                 negative_component(hs),NO_COMP,fr);
	    	(void) RegionIsFlowSpecified(sr,NULL,Coords(p),
	                                 positive_component(hs),NO_COMP,fr);
		for (; b != NULL; b = b->next)
		{
		    p = b->end;
	    	    slsr(p,Hyper_surf_element(b),hs,&sl,&sr);
	    	    (void) RegionIsFlowSpecified(sl,NULL,Coords(p),
	                                 negative_component(hs),NO_COMP,fr);
	    	    (void) RegionIsFlowSpecified(sr,NULL,Coords(p),
	                                 positive_component(hs),NO_COMP,fr);
		}
	    }
	}
}		/*end EnforceFlowSpecifedStates2d*/

LOCAL	void	EnforceFlowSpecifedStates3d(
	Front	*fr)
{
	HYPER_SURF		*hs;
	INTERFACE		*intfc;
	SURFACE			**s;
	FT_POINT			*p;
	TRI			*t;
	Locstate		sl, sr;
	int			i;

	if ((fr==NULL) || (Fsr_list(fr)==NULL) || (fr->rect_grid->dim!=3) )
	    return;

	intfc = fr->interf;
	for (s = intfc->surfaces; s && *s; ++s)
	{
	    hs = Hyper_surf(*s);
	    if (is_subdomain_boundary(hs) || is_passive_boundary(hs))
	        continue;
	    if (BothSidesActive(hs,fr) == YES)
	    	continue;
	    for (t = first_tri(*s); !at_end_of_tri_list(t,*s); t = t->next)
	    {
		for (i = 0; i < 3; ++i)
		{
		    p = Point_of_tri(t)[i];
	    	    slsr(p,Hyper_surf_element(t),hs,&sl,&sr);
	    	    (void) RegionIsFlowSpecified(sl,NULL,Coords(p),
	                                         negative_component(hs),
						 NO_COMP,fr);
	    	    (void) RegionIsFlowSpecified(sr,NULL,Coords(p),
	                                         positive_component(hs),
						 NO_COMP,fr);
		}
	    }
	}
}		/*end EnforceFlowSpecifedStates3d*/

LOCAL	boolean	BothSidesActive(
	HYPER_SURF	*hs,
	Front		*front)
{
	FlowSpecifiedRegion     *fsr;
	INTERFACE	*intfc = front->interf;
	COMPONENT	pcomp = positive_component(hs);
	COMPONENT	ncomp = negative_component(hs);

	if (hs == NULL)
	    return NO;

	for (fsr = Fsr_list(front); fsr != NULL; fsr = fsr->next)
	{
	    if (equivalent_comps(fsr->comp,pcomp,intfc))
	    	return NO;
	    if (equivalent_comps(fsr->comp,ncomp,intfc))
	    	return NO;
	}
	return YES;
}		/*end BothSidesActive*/


EXPORT	INTERFACE	*pp_copy_interface(
	INTERFACE	*intfc)
{
	INTERFACE	*new_intfc;
	boolean		stat;
	boolean		delete_status;

	DEBUG_ENTER(pp_copy_interface)

	new_intfc = copy_interface(intfc);
	stat = (new_intfc != NULL) ? YES : NO;

	if (stat == NO)
	{
	    (void) printf("WARNING in pp_copy_interface(), "
		          "unable to copy interface");
	    if (pp_numnodes() > 1)
		(void) printf(" on processor %d\n",pp_mynode());
	    else
		(void) printf("\n");
	}

	delete_status = YES;
	if (pp_min_status(stat) == NO)
	{
	    if (stat == YES)
	    {
	    	(void) printf("WARNING in pp_copy_interface(), "
		              "unable to copy interface "
		              "on a remote processor\n");
	        delete_status = (delete_interface(new_intfc)) ? YES : NO;
		if (delete_status == NO)
		{
		    screen("ERROR in pp_copy_interface() "
			   "unable to delete interface ");
	            if (pp_numnodes() > 1)
		        screen(" on processor %d\n",pp_mynode());
	            else
		        screen("\n");
		}
	        new_intfc = NULL;
	    }
	}
	if (pp_min_status(delete_status) == NO)
	{
	    if (delete_status == YES)
	    {
	        screen("ERROR in pp_copy_interface(), unable to delete "
		       "interface on a remote processor\n");
	    }
	    clean_up(ERROR);
	}
	DEBUG_LEAVE(pp_copy_interface)
	return new_intfc;
}	/*end pp_copy_interface*/


/*ARGSUSED**/
EXPORT	void set_propagation_limits(
	Front		*front,
	Front		*newfront)
{
	HYPER_SURF_ELEMENT *hse;
	HYPER_SURF         *hs;
	FT_POINT	           *p;

	(void) next_point(newfront->interf,NULL,NULL,NULL);
	while (next_point(newfront->interf,&p,&hse,&hs))
	{
	    n_pt_propagated(p) = NO;
	    t_pt_propagated(p) = NO;
	}
	if (front->interf->dim == 2)
	{
	    NODE **n;
	    CURVE **c;
	    int  i;
	    for (n = front->interf->nodes; *n; ++n) 
	        propagation_status(*n) = PROPAGATED_NODE;
	    for (n = newfront->interf->nodes; *n; ++n) 
	    {
	        for (i = 0; i < 2; ++i)
	            Node_vel(*n)[i] = 0.0;
	        propagation_status(*n) = UNPROPAGATED_NODE;
	    }
	    for (c = newfront->interf->curves; c && *c; ++c)
	    {
		if (!is_closed_curve(*c)) 
		    (*c)->orientation = 0;
		else
		    (*c)->orientation = (area_of_closed_curve(*c) > 0.0) ?
						1 : -1;
	    }
	}
	initialize_max_front_speed(front);
}		/*end set_propagation_limits*/

EXPORT	void copy_hypersurface_flags(
	INTERFACE	*intfc)
{
	HYPER_SURF	**hs;

	for (hs = hyper_surf_list(intfc); hs && *hs; ++hs)
	{
	    if (correspond_hyper_surf(*hs) != NULL)
	    	Hs_flag(*hs) = Hs_flag(correspond_hyper_surf(*hs));
	}
}		/*end copy_hypersurface_flags*/



/*ARGSUSED*/
LOCAL int advance_front1d(
	double		dt,
	double		*dt_frac,
	Front		*front,
	Front		**newfront,
	POINTER		wave)
{
	FT_POINT              *oldp, *newp;
	HYPER_SURF_ELEMENT *oldhse, *newhse;
	HYPER_SURF         *oldhs, *newhs;
	INTERFACE          *intfc_old, *intfc_new;
	int		   status;
	double              V[MAXD];
	boolean		   has_tracked_points;
	static const char	   *fname = "advance_front1d";

	debug_print("front","Entered %s(step %d time %g dt %g)\n",fname,
				front->step,front->time,dt);
	debug_front("old_front","into advance front",front);

	*newfront = copy_front(front);
	Interface_redistributed(*newfront) = NO;

	has_tracked_points = (front->interf->points != NULL) ? YES : NO;
	if (pp_max_status(has_tracked_points) == NO)
	{
	    set_size_of_intfc_state(size_of_state(front->interf));
	    set_copy_intfc_states(YES);
	    (*newfront)->interf = pp_copy_interface(front->interf);
	    status = ((*newfront)->interf != NULL) ? GOOD_STEP : ERROR_IN_STEP;
	    return return_advance_front(front,newfront,status,fname);
	}

	start_clock("propagate");

		/* Initialize Newfront */

	start_clock("init_new_front");
	set_size_of_intfc_state(size_of_state(front->interf));
	set_copy_intfc_states(NO);
	set_add_to_correspond_list(YES);
	(*newfront)->interf = pp_copy_interface(front->interf);
	if ((*newfront)->interf == NULL)
	{
	    (void) printf("ERROR in advance_front1d(), "
			  "unable to copy interface\n");
	    return return_advance_front(front,newfront,ERROR_IN_STEP,fname);
	}
	stop_clock("init_new_front");

		/* Propagate the points */

	set_propagation_limits(front,*newfront);
	set_copy_intfc_states(YES);
	intfc_old = front->interf;
	intfc_new = (*newfront)->interf;

	(void) next_point(intfc_old,NULL,NULL,NULL);
	(void) next_point(intfc_new,NULL,NULL,NULL);
	while (next_point(intfc_old,&oldp,&oldhse,&oldhs) && 
	       next_point(intfc_new,&newp,&newhse,&newhs))
	{
	    point_propagate(front,wave,oldp,newp,oldhse,oldhs,dt,V);
	}
	copy_hypersurface_flags(intfc_new);
	debug_front("pt_front","after point propagate",*newfront);

	switch (redistribute(*newfront,YES,NO)) 
	{
	case	GOOD_REDISTRIBUTION:
	    status = GOOD_STEP;
	    break;
	
	case	MODIFY_TIME_STEP_REDISTRIBUTE:
	    (void) printf("WARNING in advance_front1d(), redistribution "
			  "of front failed, reducing time step\n");
	    debug_front("ERROR_front","after error",*newfront);
	    *dt_frac = max(Min_time_step_modification_factor(front),*dt_frac);
	    status = MODIFY_TIME_STEP;
	    break;

	case	UNABLE_TO_UNTANGLE:
	case	BAD_REDISTRIBUTION:
	default:
	    (void) printf("WARNING in advance_front1d(), "
	                  "redistribution of front failed\n");
	    debug_front("ERROR_front","after error",*newfront);
	    *dt_frac = Min_time_step_modification_factor(front);
	    status = ERROR_IN_STEP;
	    break;
	}
	debug_front("redist_front","after redistribute",*newfront);
	if (status != GOOD_STEP)
	    return return_advance_front(front,newfront,status,fname);
	if (!scatter_front(*newfront))
	{
	    (void) printf("WARNING in advance_front1d(), "
	    	          "scatter_front() failed for "
	    	          "normally propagated front\n");
	    return return_advance_front(front,newfront,ERROR_IN_STEP,fname);
	}

	(*newfront)->step = front->step + 1;
	(*newfront)->time = front->time + dt;
	interpolate_intfc_states(intfc_new) = YES;
	set_size_of_intfc_state(size_of_state(intfc_new));

	if (intfc_new->modified)
	    (void) make_point_comp_lists(intfc_new);

	stop_clock("propagate");
	debug_front("new_front","from advance front",*newfront);
	return return_advance_front(front,newfront,status,fname);
}		/*end advance_front1d*/

/*
*			advance_front2d():
*
*	Advances the Front structure front by copying its interface
*	to the interface for newfront and calling
*		
*		(*front->node_propagate)() for each node
*		(*front->curve_propagate)() for each curve
*		point_propagate() for (certain) points on each curve
*		(*front->bond_propagate)() for (certain) bonds on each curve
*		(*front->snd_node_propagate)() for each node
*		(*front->tan_curve_propagate)() for each curve
*	and
*		redistribute()
*
*	in this order to modify the new interface.
*	
*	To define the points and bonds to be propagated in the normal or
*	tangential propagation loop, the flags n_pt_propagated(p)
*	and t_pt_propagated(p) respectively are set to NO.  As each point
*	is processed by the respective propagate step the corresponding
*	flag is set to YES.  To supress a given propagation step,  the
*	the corresponding flag can be turned on prior to propagation.
*
*	Returns 1 if successful, or 0 if copying the interface fails or
*	one of the propagation, or redistribution routines returns 0.
*
*	For documentation of debug_front(), see the documentation for that
*	function in another file (currently fprint.c).
*/


#define is_bad_status(status)   (					\
				((status) == BIFURCATION_NODE) ||	\
				((status) == CROSS_PAST_CURVE_NODE) ||	\
				((status) == MODIFY_TIME_STEP_NODE) ||	\
				((status) == REPEAT_TIME_STEP_NODE) ||	\
				((status) == NO_CROSS_NODE) ||		\
				((status) == ERROR_NODE)		)

LOCAL int advance_front2d(
	double    dt,
	double    *dt_frac,
	Front    *front,
	Front    **newfront,
	POINTER  wave)
{
	CURVE      *oldc,*tempc,*newc;
	CURVE	   **c;
	INTERFACE  *tempintfc;
	NODE 	   *oldn,*tempn,*newn;
	NODE_FLAG  flag;
	RPROBLEM   *rp;
	RPROBLEM   *rp1;
	boolean	   scatter_normally_propagated_front = YES;
	boolean	   scatter_tangentially_propagated_front = YES;
	boolean	   stat;
	boolean       do_redist;
	int        status;
	long       intfc_modified;
	long       redo_advance_front;
	static const char *fname = "advance_front2d()";
	int	   debug_flag = NO;

	debug_print("front","Entered %s(step %d time %g dt %g)\n",fname,
	        	        front->step,front->time,dt);
	debug_front("old_front","into advance front",front);

	*newfront = copy_front(front);
	Interface_redistributed(*newfront) = NO;
	do_redist = (front->num_mts == 0) ? YES : NO;

begin_advance_front2d:
	redo_advance_front = 0;
	tempintfc = NULL;
	rp = NULL;
	set_to_next_node_only(flag);

	set_node_doubly_linked_list(front->interf);

	        /* Initialize Newfront */

	start_clock("init_new_front");
	capture_waves(front);
	print_storage("before init_new_front","ADV_storage");
	        /* TODO: Remove this option!!!!! */
	if (front->init_topology_of_new_interface)
	    status = (*front->init_topology_of_new_interface)(front,*newfront);
	else 
	{
	    set_size_of_intfc_state(size_of_state(front->interf));
	    set_copy_intfc_states(NO);
	    set_add_to_correspond_list(YES);
	    (*newfront)->interf = pp_copy_interface(front->interf);
	    reset_hs_flags_on_intfc((*newfront)->interf);
	    status = ((*newfront)->interf != NULL) ? GOOD_STEP : ERROR_IN_STEP;
	    set_copy_intfc_states(YES);
	}
	if (front->pp_grid)
	    status = syncronize_time_step_status(status,front->pp_grid);
	if (status != GOOD_STEP) 
	{
	    (void) printf("WARNING in advance_front2d(), "
	                  "unable to copy interface\n");
	    status = ERROR_IN_STEP;
	    stop_clock("init_new_front");
	    return return_advance_front(front,newfront,status,fname);
	}
	print_storage("after init_new_front","ADV_storage");
	stop_clock("init_new_front");

	        /* Set Default Propagation Limits */

	set_propagation_limits(front,*newfront);

	        /* Propagate the Curves */

	if (front->intfc_propagate != NULL)
	{
	    start_clock("intfc_propagate");
	    intfc_propagate(front,wave,front->interf,(*newfront)->interf,dt);
	    debug_front("cp_front","after intfc prop",*newfront);
	    stop_clock("curve_propagate");
	}
	else if (front->curve_propagate != NULL) 
	{
	    start_clock("curve_propagate");
	    if (debugging("front"))
	    	(void) printf("Loop over Curves\n");
	    for (c = front->interf->curves; c && *c; ++c)
	    {
	        oldc = *c;
	        if (((newc = correspond_curve(oldc)) != NULL) &&
	    	     (correspond_curve(newc) != NULL))
	        {
	    	    if (debugging("propagate"))
	                (void) printf("\t\tpropagating curve %llu\n",
		                      curve_number(oldc));
		    curve_propagate(front,wave,oldc,newc,dt);
		    /*f_curve_propagate2d */
		}
	    }
	    debug_front("cp_front","after curve prop",*newfront);
	    stop_clock("curve_propagate");
	}

		/* Propagate the Nodes */

	if (debugging("front"))
	{
	    print_correspond_hyper_surf_list(front->interf);
	    print_correspond_hyper_surf_list((*newfront)->interf);
	}
	if (front->node_propagate != NULL) 
	{
	    start_clock("node_propagate");
	    set_corresponds_for_node_prop(front->interf,(*newfront)->interf);
	    oldn = first_node(front->interf);
	    while (oldn != NULL) 
	    {
	        newn = correspond_node(oldn);
	        if (debugging("crx_status"))
	            print_linked_node_list((*newfront)->interf);
	        status = (newn != NULL) ?
	            (*front->node_propagate)(front,wave,oldn,newn,&rp,
	        			     dt,dt_frac,flag,NULL) : GOOD_NODE;

	        if (debugging("crx_status"))
	        if (is_bad_status(status) &&
	          (point_in_buffer(Coords(oldn->posn),front->rect_grid) == YES))
	        {
	            print_node_status("WARNING in advance_front2d(), "
	                              "node_propagation returns ",status,"\n");
	            (void) printf("Problem occurs in buffer zone - ignoring\n");
	            if (set_node_states_and_continue(oldn,newn,front))
	                status = GOOD_NODE;
	        }

	        switch (status) 
	        {
	        case GOOD_NODE:
	            oldn = adv_node_loop_after_good_prop(oldn,newn,&rp);
	            break;
	        case PSEUDOCROSS_NODE_NODE:
	            debug_print("PSEUDOCROSS","PSEUDOCROSS case\n");
	            oldn = reorder_node_loop(oldn,newn);
	            break;
	        case CROSS_NODE_NODE:
	        case BIFURCATION_NODE:
	            debug_print("CROSS","CROSS case\n");
	            oldn = next_node(oldn);
	            break;
	        case CROSS_PAST_CURVE_NODE:
	            print_node_status("WARNING in advance_front2d(), "
	                              "node_propagate failed with status ",
				      status,"\n");
	            print_node(oldn);
	            if (debugging("CROSS_PAST"))
	            {
	                (void) printf("Cross past curve case\n"
	                              "dt_frac = %g\n",*dt_frac);
	                (void) printf("Reducing time step\n");
	            }
		    status = node_modify_time_step(oldn,front,dt_frac,
					           MODIFY_TIME_STEP);
	            free_rp_list(&rp);
	            goto sync_prop_stat1;
	        case MODIFY_TIME_STEP_NODE:
	            (void) printf("WARNING in advance_front2d(), "
	                          "node_propagate returns "
	                          "MODIFY_TIME_STEP_NODE\n");
	            free_rp_list(&rp);
		    status = node_modify_time_step(oldn,front,NULL,
						   MODIFY_TIME_STEP);
	            goto sync_prop_stat1;
	        case REPEAT_TIME_STEP_NODE:
	            (void) printf("WARNING in advance_front2d(), "
	                          "node_propagate returns "
	                          "REPEAT_TIME_STEP_NODE\n");
	            free_rp_list(&rp);
		    status = node_modify_time_step(oldn,front,NULL,
					           REPEAT_TIME_STEP);
	            goto sync_prop_stat1;
	        case NO_CROSS_NODE:
	            print_node_status("WARNING in advance_front2d(), "
	                              "node_propagate failed with status ",
	                              status,"\n");
	            print_node(oldn);
	            if (debugging("NO_CROSS"))
	            {
	                (void) printf("No cross case\n");
	                (void) printf("dt_frac = %g\n",*dt_frac);
	                (void) printf("Reducing time step\n");
	            }
	            free_rp_list(&rp);
		    status = node_modify_time_step(oldn,front,dt_frac,
					           MODIFY_TIME_STEP);
	            goto sync_prop_stat1;
	        case ERROR_NODE:
	        default:
	            print_node_status("WARNING in advance_front2d(), "
	                              "node_propagate failed with status ",
	                              status,"\n");
	            print_node(oldn);
	            if (debugging("ERROR_NODE"))
	            {
	                (void) printf("Old interface:\n");
	                print_interface(front->interf);
	                print_correspond_hyper_surf_list(front->interf);
	                (void) printf("New interface:\n");
	                print_interface((*newfront)->interf);
	                print_correspond_hyper_surf_list((*newfront)->interf);
	            }
		    status = node_modify_time_step(oldn,front,dt_frac,
					           ERROR_IN_STEP);
	            free_rp_list(&rp);
	            goto sync_prop_stat1;
	        }
	    } /* end of while (oldn != NULL) */
	    set_correspond_hyper_surf_bdrys_to_NULL(front->interf);
	    set_correspond_hyper_surf_bdrys_to_NULL((*newfront)->interf);
	    if (rp && (front->twodrproblem != NULL)) 
	    {
	        for (rp1 = rp; rp1; rp1 = rp1->prev) 
	        {
	            debug_front("2drp_front",
	                "new between node loop and rp loop",*newfront);
	                    
	            status = (*front->twodrproblem)(front,*newfront,wave,&rp1);

	            /* At this point, rp is nothing more than a valid element
	             * of the list which provides a starting point
	             * for deleting the list.  If we delete an element of
	             * the list in front->twodrproblem (presumably due to
	             * merging two RPROBLEM's), then rp may point to freed
	             * storage and will need to be updated.  rp1 should still
	             * be a valid element of the list.
	             */
	            rp = rp1;

		    if (status != GOOD_STEP)
		    {
	                print_time_step_status("WARNING in advance_front2d(), "
					       "rp failed with status = ",
					       status,"\n");
	                switch (status) 
	                {
	                case GOOD_STEP:
			    break;

		        case REPEAT_TIME_STEP:
	                    break;

	                case MODIFY_TIME_STEP:
		            status = rp_modify_time_step(rp1,front,status);
			    if (status == MODIFY_TIME_STEP)
			    {
	                        *dt_frac = rp1->dt_frac;
	                        if (debugging("2drp"))
	                        {
	                            print_rproblem(rp1);
	                            (void) printf("dt_frac %g\n",*dt_frac);
	                            (void) printf("Reducing time step\n");
	                        }
			        *dt_frac = limit_dt_frac(*dt_frac,front);
			    }
	                    break;

	                case ERROR_IN_STEP:
	                default:
	                    print_rproblem(rp1);
	                    /* Try reducing the time step */
		            status = rp_modify_time_step(rp1,front,status);
	                    if (status == MODIFY_TIME_STEP)
	                        *dt_frac *=
				    TIME_STEP_REDUCTION_FACTOR(front->interf);
	                    break;
	                }
		    }
	            if (status != GOOD_STEP)
			break;
	        }
	        free_rp_list(&rp);
	        debug_front("2drp_front","after 2drp loop",*newfront);
	    }
	    else if (rp) 
	    {
	        for (rp1 = rp; rp1; rp1 = rp1->prev) 
	            print_rproblem(rp1);
	        free_rp_list(&rp);
	        (void) printf("WARNING in advance_front2d(), "
	                      "CROSS code needed\n");
	        status = ERROR_IN_STEP;
	    }

sync_prop_stat1:
	    stop_clock("node_propagate");
	    if (front->pp_grid)
	    	status = syncronize_time_step_status(status,front->pp_grid);
	    if (status != GOOD_STEP)
	        return return_advance_front(front,newfront,status,fname);
	}

	stat = consistent_propagated_loop_orientations(dt,dt_frac,front,wave);
	if (stat == NO)
	{
	    (void) printf("WARNING in advance_front2d(), "
		          "Inconsistent orientation of propagated loop "
	                  "detected after point and node propagations");
	    if (pp_numnodes() > 1)
		(void) printf(" on processor %d\n",pp_mynode());
	    else
		(void) printf("\n");
	}
	if (pp_min_status(stat) == NO)
	{
	    if (stat == YES)
	    {
	        (void) printf("WARNING in advance_front2d(), "
		              "Inconsistent orientation of propagated loop "
	                      "detected on a remote processor "
			      "after point and node propagations ");
	    }
	    status = MODIFY_TIME_STEP;
	    goto sync_prop_stat2;
	}

	/* Make Temp Interface for Tangential Propagation */

	set_node_doubly_linked_list((*newfront)->interf);
	if (front->snd_node_propagate) 
	{
	    start_clock("snd_copy_interface");
	    print_storage("before snd_copy_interface","ADV_storage");
	    tempintfc = (*newfront)->interf;
	    set_size_of_intfc_state(size_of_state(tempintfc));
	    set_add_to_correspond_list(YES);
	    if (((*newfront)->interf = pp_copy_interface(tempintfc)) == NULL)
	    {
	    	(void) printf("WARNING in advance_front2d(), "
		              "unable to copy interface\n");
		status = ERROR_IN_STEP;
		goto sync_prop_stat2;
	    }
	    copy_hypersurface_flags((*newfront)->interf);
	    print_storage("after snd_copy_interface","ADV_storage");
	    stop_clock("snd_copy_interface");
	}
	interpolate_intfc_states((*newfront)->interf) = YES;

	/* Second Propagation for the States Around the Nodes */

	if (front->snd_node_propagate) 
	{
	    start_clock("snd_node_propagate");
	    if (debugging("front"))
	    	(void) printf("Second Loop over Nodes\n");

	    tempn = first_node(tempintfc);
	    newn = first_node((*newfront)->interf);
	    while (newn != NULL)
	    {
	    	(*front->snd_node_propagate)(front,*newfront,wave,
	    				     tempintfc,tempn,newn,dt);
	    	tempn = next_node(tempn);
	    	newn = next_node(newn);
	    }

	    debug_front("snd_front","after snd_node prop",*newfront);
	    stop_clock("snd_node_propagate");
	}

	if (tempintfc)
	    (void) delete_interface(tempintfc);
	print_storage("after delete tempintfc","ADV_storage");

		/* Redistribute the New Front */

	switch (redistribute(*newfront,do_redist,NO)) 
	{
	case	GOOD_REDISTRIBUTION:
	    status = GOOD_STEP;
	    break;
	
	case	UNABLE_TO_UNTANGLE:
	    (void) printf("WARNING in advance_front2d(), "
	                  "redistribution of front failed\n"
	                  "Restarting advance_front2d()\n");
	    *dt_frac = Min_time_step_modification_factor(front);
	    status = MODIFY_TIME_STEP;
	    break;

	case	MODIFY_TIME_STEP_REDISTRIBUTE:
	    (void) printf("WARNING in advance_front2d(), "
	                  "redistribute returns\n"
	                  "\t\tMODIFY_TIME_STEP_REDISTRIBUTE, dt_frac = %g\n",
			  *dt_frac);
	    *dt_frac = Min_time_step_modification_factor(front);
	    status = MODIFY_TIME_STEP;
	    break;
		
	case	BAD_REDISTRIBUTION:
	default:
	    (void) printf("WARNING in advance_front2d(), "
	                  "redistribution of front failed\n");
	    debug_front("ERROR_front","after error",*newfront);
	    *dt_frac = Min_time_step_modification_factor(front);
	    status = MODIFY_TIME_STEP;
	    break;
	}
	if (front->pp_grid)
	    status = syncronize_time_step_status(status,front->pp_grid);
	if (status != GOOD_STEP)
	    return return_advance_front(front,newfront,status,fname);

	Redistribution_count(front) = Redistribution_count(*newfront);
	(*newfront)->step = front->step + 1;
	(*newfront)->time = front->time + dt;
	debug_front("redist_front","after redistribution",*newfront);

	/* Communicate topologically propagated front */
	if (scatter_normally_propagated_front == YES)
	{
	    start_clock("scatter_front");
	    if (!scatter_front(*newfront))
	    {
	    	(void) printf("WARNING in advance_front2d(), "
	    	              "scatter_front() failed for "
	    	              "normally propagated front\n");
	    	scatter_normally_propagated_front = NO;
	    	scatter_tangentially_propagated_front = NO;
	    	(void) delete_interface((*newfront)->interf);
	    	(*newfront)->interf = NULL;
	    	goto begin_advance_front2d;
	    }
	    stop_clock("scatter_front");
	}

	debug_front("node_front","after node loop",*newfront);
	if (debugging("front"))
	{
	    print_correspond_hyper_surf_list(front->interf);
	    print_correspond_hyper_surf_list((*newfront)->interf);
	}

	if (front->mass_consv_diagn_driver)
	    (*front->mass_consv_diagn_driver)(front,wave,dt);

	if (debugging("bond_lengths"))
	    check_bond_lengths((*newfront)->interf);

	/* Check for the geometric orientation of loops */

	/* ONLY check loops that will not be deleted !!!! */
	delete_small_loops(*newfront);

		/* Delete non-boundary curves that lie  */
		/* fully on or exterior to the boundary */

	delete_exterior_curves(*newfront,front->interf);
	intfc_delete_fold_back_bonds(*newfront);
	debug_front("dec_front","after delete_exterior_curves:",*newfront);

	interpolate_intfc_states((*newfront)->interf) = YES;

	/* Make Temp Interface for Tangential Propagation */

	if (front->tan_curve_propagate) 
	{
	    start_clock("snd_copy_interface");
	    print_storage("before snd_copy_interface","ADV_storage");
	    tempintfc = (*newfront)->interf;
	    set_size_of_intfc_state(size_of_state(tempintfc));
	    set_add_to_correspond_list(YES);
	    if (((*newfront)->interf = pp_copy_interface(tempintfc)) == NULL)
	    {
	    	(void) printf("WARNING in advance_front2d(), "
		              "unable to copy interface\n");
		status = ERROR_IN_STEP;
		goto sync_prop_stat2;
	    }
	    copy_hypersurface_flags((*newfront)->interf);
	    interpolate_intfc_states((*newfront)->interf) = YES;
	    print_storage("after snd_copy_interface","ADV_storage");
	    stop_clock("snd_copy_interface");
	}

	/* Tangential Sweep for States on the Curves */

	if (front->tan_curve_propagate) 
	{
	    start_clock("tan_curve_propagate");
	    if (debugging("front"))
	    	(void) printf("Second Loop over Curves\n");
	    for (c = tempintfc->curves; c && *c; ++c)
	    {
	    	tempc = *c;
	    	newc = correspond_curve(tempc);
	    	(*front->tan_curve_propagate)(front,*newfront,
	    				      tempintfc,tempc,newc,dt);
	    }
	    debug_front("tcp_front","after tan_curve_propagate:",*newfront);
	    stop_clock("tan_curve_propagate");
	}
	if (tempintfc)
	    (void) delete_interface(tempintfc);
	print_storage("after delete tempintfc","ADV_storage");


		/* Provide robustness for untangle algorithms */

		/*   delete remnants of scalar physical   */
		/*  curves sticking to NEUMANN boundaries */
		/* Add to delete_exterior_curves()? */

	if (pp_min_status(delete_phys_remn_on_bdry(*newfront)) == NO)
	{
	    (void) printf("WARNING in advance_front2d(), "
	                  "delete_phys_remn_on_bdry() detected error\n");
	    debug_front("ERROR_front","after error",*newfront);
	    *dt_frac = Min_time_step_modification_factor(front);
	    status = MODIFY_TIME_STEP;
	    goto sync_prop_stat2;
	}
	debug_front("dspr_front",
		    "after 1st delete_phys_remn_on_bdry():",*newfront);

sync_prop_stat2:
	if (front->pp_grid)
	    status = syncronize_time_step_status(status,front->pp_grid);
	if (status != GOOD_STEP)
	    return return_advance_front(front,newfront,status,fname);

	/* Communicate tangentially propagated front */
	if (scatter_tangentially_propagated_front == YES)
	{
	    start_clock("scatter_front");
	    if (!scatter_front(*newfront))
	    {
	    	(void) printf("WARNING in advance_front2d(), "
	    	              "scatter_front() failed for "
	    	              "tangentially propagated front\n");
	    	scatter_normally_propagated_front = NO;
	    	scatter_tangentially_propagated_front = NO;
	    	(void) delete_interface((*newfront)->interf);
	    	(*newfront)->interf = NULL;
	    	goto begin_advance_front2d;
	    }
	    stop_clock("scatter_front");
	}

	if (status != GOOD_STEP)
	    return return_advance_front(front,newfront,status,fname);


		/* 	Post-process newfront->interf	   */
		/* Provide robustness after redistribution */
		/*   for node propagate on next time step  */

		/* Delete non-boundary curves that lie  */
		/* fully on or exterior to the boundary */

	delete_exterior_curves(*newfront,front->interf);
	debug_front("dec_front","after delete_exterior_curves:",*newfront);

		/*  delete remnants of scalar physical    */
		/* curves sticking to NEUMANN boundaries  */
		/* Add to delete_exterior_curves()? */

	if (pp_min_status(delete_phys_remn_on_bdry(*newfront)) == NO)
	{
	    (void) printf("WARNING in advance_front2d(), "
	                  "delete_phys_remn_on_bdry() detected error\n");
	    debug_front("ERROR_front","after error",*newfront);
	    *dt_frac = Min_time_step_modification_factor(front);
	    status = MODIFY_TIME_STEP;
	    return return_advance_front(front,newfront,status,fname);
	}
	debug_front("dspr_front",
		    "after 2nd delete_phys_remn_on_bdry():",*newfront);


	/* These guys keep sneaking through !! */
	/* This should be the most effective place for this call */
	/* Brent - I believe it is better to have the function at
	*  the end of advance_front2d() applied to the newfront
	*  instead of at the beginning applied to front.
	*  In general our policy should be never to modify the
	*  old interface data.
	*/
	delete_small_loops(*newfront);
	debug_front("dsloop_front","after delete_small_loops():",*newfront);

	test_for_mono_comp_curves((*newfront)->interf);

	/* Check if post processing has changed topology */

	intfc_modified = (*newfront)->interf->modified;
	pp_global_lmax(&intfc_modified,1L);
	if (intfc_modified)
	{
	    if (!scatter_front(*newfront))
	    {
	    	(void) printf("WARNING in advance_front2d(), "
	    	              "final scatter_front() failed\n");
	    	*dt_frac = Max_time_step_modification_factor(front);
	        return return_advance_front(front,newfront,
		                            MODIFY_TIME_STEP,fname);
	    }
	    stat = make_bond_comp_lists((*newfront)->interf);
	    if (pp_min_status(stat) == FUNCTION_FAILED)
	    {
	    	screen("ERROR in advance_front2d(), "
	    	       "make_bond_comp_lists() failed\n");
	    	clean_up(ERROR);
	    }
	}

	return return_advance_front(front,newfront,GOOD_STEP,fname);
}		/*end advance_front2d*/

LOCAL	int node_modify_time_step(
	NODE  *oldn,
	Front *front,
	double *dt_frac,
	int   status)
{
	if (status == GOOD_STEP)
	    return status;
	if (last_time_step_modification(front) == YES)
	{
	    return delete_phys_curves_at_old_node(oldn,status);
	}
	if (dt_frac != NULL)
	{
	    if (status == ERROR_IN_STEP)
	    {
	        *dt_frac = Min_time_step_modification_factor(front);
		status = MODIFY_TIME_STEP;
	    }
	    else
	        *dt_frac *= TIME_STEP_REDUCTION_FACTOR(front->interf);
	}
	return status;
}		/*end node_modify_time_step*/

EXPORT	int rp_modify_time_step(
	RPROBLEM *rp,
	Front    *front,
	int      status)
{
	RP_NODE   *rpn;

	if (status == ERROR_IN_STEP)
	    status = MODIFY_TIME_STEP;
	if ((status==GOOD_STEP) || (last_time_step_modification(front)!=YES))
	    return status;
	for (rpn = rp->first_rp_node; rpn != NULL; rpn = rpn->next)
	    status = delete_phys_curves_at_old_node(rpn->old_node,status);
	return status;
}		/*end rp_modify_time_step*/

EXPORT	double	limit_dt_frac(
	double	dt_frac,
	Front	*front)
{
	if (dt_frac < Min_time_step_modification_factor(front))
	    dt_frac = Min_time_step_modification_factor(front);
	if (dt_frac > Max_time_step_modification_factor(front))
	    dt_frac = Max_time_step_modification_factor(front);
	return dt_frac;
}		/*end limit_dt_frac*/

LOCAL	int delete_phys_curves_at_old_node(
	NODE  *oldn,
	int   status)
{
	boolean untrack = NO;
	CURVE   **c;
	int     wtype = FIRST_VECTOR_PHYSICS_WAVE_TYPE;

	for (c = oldn->in_curves; c && *c; ++c)
	{
	    if (wave_type(*c) < wtype)
		continue;
	    untracked_hyper_surf(*c) = YES;
	    untrack = YES;
	    status = REPEAT_TIME_STEP;
	}
	for (c = oldn->out_curves; c && *c; ++c)
	{
	    if (wave_type(*c) < wtype)
		continue;
	    untracked_hyper_surf(*c) = YES;
	    untrack = YES;
	    status = REPEAT_TIME_STEP;
	}
	if (untrack == YES)
	{
	    (void) printf("WARNING in delete_phys_curves_at_old_node(), "
			  "turning off tracking of vector curves at node\n");
	    print_node(oldn);
	}
	return status;
}		/*end delete_phys_curves_at_old_node*/

EXPORT void reset_hs_flags_on_intfc(
	INTERFACE	*intfc)
{
	HYPER_SURF	**hs;

	if (intfc == NULL)
	    return;

	for (hs = hyper_surf_list(intfc); hs && *hs; ++hs)
	{
	    /*Turn off flags force || preventing redistribution*/
	    never_redistribute(*hs) = NO;
	    do_not_redistribute(*hs) = NO;
	    redistribute_hyper_surface(*hs) = NO;
	    redistributed(*hs) = NO;
	    perform_redistribution_function(*hs) =
	        default_perform_redistribution_function(intfc);
	}
}		/*end reset_hs_flags_on_intfc*/

EXPORT	NODE *adv_node_loop_after_good_prop(
	NODE		*oldn,
	NODE		*newn,
	RPROBLEM	**rp)
{
	NODE		*n;
	NODE		*next_oldn;
	RPROBLEM	*rp1;
	RP_NODE		*rpn;

	if ((rp != NULL) && (*rp != NULL) && 
	    (newn != NULL) && (node_type(newn) != SUBDOMAIN_NODE) &&
	    (!is_virtual_fixed_node(newn)))
	{
	    for (rp1 = *rp; rp1; rp1 = rp1->prev) 
	    {
	    	for (rpn = rp1->first_rp_node; rpn; rpn = rpn->next)
	    	{
	    	    if ((rpn->node == newn)) 
	    	    	break;
	    	}
	    	if (rpn == NULL)
		    continue;

	    		/* Delete rp1 from rp list */

	    	if (rp1->prev)
		    rp1->prev->next = rp1->next;
	    	if (rp1->next)
		    rp1->next->prev = rp1->prev;
	    	else
		    *rp = rp1->prev;
				
	    	    /* Push other nodes in rp1 to end of node list */

	    	for (rpn = rp1->first_rp_node; rpn; rpn = rpn->next)
	    	{
	    	    if ((rpn->node == newn)) 
	    	    	continue;

	    	    if (propagation_status(rpn->node) == PROPAGATED_NODE)
	    		continue;

	    	    /* Check if node already follows newn */

	       	    for (n = next_node(newn); n != NULL; n = next_node(n))
	    	    {
	    	    	if (n == rpn->node)
			    break;
	    	    }
	    	    if (n != NULL)
			continue;

	    	    n = rpn->node;
	    	    (void) reorder_node_loop(rpn->old_node,rpn->node);
	    	}
	    	free_rp(rp1);
	    }
	}

	/*
	*  Reset propagation status to UNPROPAGATED_NODE
	*  after successful node propagation.
	*/
	next_oldn = next_node(oldn);
	for (oldn = next_oldn; oldn != NULL; oldn = next_node(oldn))
	{
	    if ((newn = correspond_node(oldn)) == NULL)
		continue;
	    if (propagation_status(newn) != PROPAGATED_NODE)
	    	propagation_status(newn) = UNPROPAGATED_NODE;
	}
	return next_oldn;
}		/*end adv_node_loop_after_good_prop*/



/*
*			reorder_node_loop():
*
*	Rearranges the node loop so that *n is placed at the end of the list,
*	and then reset to what was the value of (*n)->next when the function
*	was entered.  Returns the next node in the list after n.
*/

EXPORT NODE *reorder_node_loop(
	NODE *oldn,
	NODE *newn)
{
	INTERFACE *intfc;
	NODE	  *next, *prev;
	NODE      *next_old_node;
	
	if (oldn != NULL)
	{
	    intfc = oldn->interface;
	    next = next_node(oldn);
	    if (next == NULL) 
	    {
	        screen("ERROR in reorder_node_loop(), "
		       "next_node(oldn) == NULL\n");
	        print_node(oldn);
		(void) printf("Old interface information\n");
	        print_linked_node_list(intfc);
	        print_interface(intfc);
		if (newn != NULL)
		{
	            intfc = newn->interface;
		    (void) printf("New interface information\n");
	            print_linked_node_list(intfc);
	            print_interface(intfc);
		}
	        clean_up(ERROR);
	    }
	    prev = prev_node(oldn);
	    prev_node(next) = prev;
	    if (prev != NULL)
	        next_node(prev) = next;
	    else
	        first_node(intfc) = next;
	    next_node(last_node(intfc)) = oldn;
	    next_node(oldn) = NULL;
	    prev_node(oldn) = last_node(intfc);
	    last_node(intfc) = oldn;
	    next_old_node = next;
	}
	else
	    next_old_node = NULL;

	if (newn != NULL)
	{
	    intfc = newn->interface;
	    next = next_node(newn);
	    if (next == NULL) 
	    {
	        screen("ERROR in reorder_node_loop(), "
		       "next_node(newn) == NULL\n");
	        print_node(newn);
		if (oldn != NULL)
		{
		    (void) printf("Old interface information\n");
	            print_linked_node_list(intfc);
	            print_interface(intfc);
		}
	        intfc = newn->interface;
		(void) printf("New interface information\n");
	        print_linked_node_list(intfc);
	        print_interface(intfc);
	        clean_up(ERROR);
	    }
	    prev = prev_node(newn);
	    prev_node(next) = prev;
	    if (prev != NULL)
	        next_node(prev) = next;
	    else
	        first_node(intfc) = next;
	    next_node(last_node(intfc)) = newn;
	    next_node(newn) = NULL;
	    prev_node(newn) = last_node(intfc);
	    last_node(intfc) = newn;
	}
	return next_old_node;
}		/*end reorder_node_loop*/


EXPORT	void set_corresponds_for_node_prop(
	INTERFACE	*ointfc,
	INTERFACE	*nintfc)
{
	NODE		**on, **nn;

	on = ointfc->nodes;	nn = nintfc->nodes;
	for (;on && *on; ++on, ++nn)
	{
	    correspond_hyper_surf_bdry(*on) = Hyper_surf_bdry(*nn);
	    correspond_hyper_surf_bdry(*nn) = Hyper_surf_bdry(*on);
	}
}		/*end set_corresponds_for_node_prop*/

EXPORT	void print_linked_node_list(
	INTERFACE	*intfc)
{
	NODE		**n, *m;
	int		i, node_count;

	n = intfc->nodes;
	if (! n)
	{
	    (void) printf("NULL node list on intfc\n");
	    return;
	}

	(void) printf("\tnode list - intfc %llu\n",interface_number(intfc));
	for (node_count = 0;  n && *n;  ++n, ++node_count)
		;

	m = first_node(intfc);
	for (i = 0;  i <= node_count + 2;  ++i) 
	{
	    if (m != NULL)
	    {
	    	(void) printf("prev %llu  m %llu  next %llu  ",
	    		      node_number(prev_node(m)),node_number(m),
	    		      node_number(next_node(m)));
	    	print_propagation_status(m);
	    }
	    else
	    	break;
	    m = next_node(m);
	}
}		/*end print_linked_node_list*/


EXPORT void set_node_doubly_linked_list(
	INTERFACE	*intfc)
{
	NODE		**n;

	n = intfc->nodes;
	first_node(intfc) = *n;
	prev_node(*n) = NULL;
	for (; *(n+1); ++n) 
	{
	    next_node(*n) = *(n+1);
	    prev_node(*(n+1)) = (*n);
	}
	last_node(intfc) = *n;
	next_node(*n) = NULL;
}		/*end set_node_doubly_linked_list*/



/*ARGSUSED*/
EXPORT	boolean consistent_propagated_loop_orientations(
	double		dt,
	double		*dt_frac,
	Front		*fr,
	POINTER		wave)
{
	static const double AREA_FAC = 0.01;	/*TOLERANCE*/
	static const int   num_attempts = 5;	/*TOLERANCE*/
	CURVE	**c,*oldc,*newc;
	NODE	*oldn, *newn;
	double	dt_f, dT;
	double	A_old, A_new;
	double	min_area = AREA_FAC*fr->rect_grid->h[0]*fr->rect_grid->h[1];
	boolean	status = YES;
	int	i;

	debug_print("orient_consistency",
	      "Entered consistent_propagated_loop_orientations()\n");

	for (c = fr->interf->curves; c && *c; ++c)
	{
	    oldc = *c;
	    if (!is_closed_curve(oldc))
		continue;
	    newc = correspond_curve(oldc);

	    /*TODO: this needs to be solved correctly.*/
	    if ((newc == NULL) || (newc->interface == NULL) ||
	        (!is_closed_curve(newc)))
	    	continue;
	    if (newc->num_points > 20)
	    	continue;	/* sufficiently long curve cannot fully flip */

	    A_old = area_of_closed_curve(oldc);
	    A_new = area_of_closed_curve(newc);

	    if (debugging("orient_consistency"))
	    	(void) printf("A_old = %g, A_new = %g\n",A_old,A_new);

	    if (A_old*A_new > 0.0)
		continue;

	    status = NO;
	    if (debugging("orient_consistency"))
	    {
	    	(void) printf("Loop %llu inverted after propagation\n",
	    		      curve_number(oldc));
	    	(void) printf("Old loop\n");
	    	print_curve(oldc);
	    	(void) printf("Propagated loop\n");
	    	print_curve(newc);
	    }

	    /* Determine dt_frac */

	    if (A_old < min_area)
	    {
	    	*dt_frac *= 0.9;
	    	continue;
	    }

	    dt_f = 1.0;
	    oldn = oldc->start;	newn = newc->start;
	    for (i = 0; i < num_attempts; ++i)
	    {
	    	dt_f *= 0.5;
	    	dT = dt_f * dt;
	    	if (fr->curve_propagate != NULL) 
	    	    curve_propagate(fr,wave,oldc,newc,dT);
		(void) closed_node_propagate(fr,wave,oldn,newn,dT);
		A_new = area_of_closed_curve(newc);

		if (debugging("orient_consistency"))
		{
	            (void) printf("In loop, dt_f = %g, dT = %g, ",dt_f,dT);
		    (void) printf("A_old = %g, A_new = %g\n",A_old,A_new);
		}
		if (A_old*A_new >= 0.0)
		    break;
	    }
	    if (i == num_attempts)
	    {
	        (void) printf("WARNING in "
			      "consistent_propagated_loop_orientations(), "
		              "cannot produce consistent loop "
		              "orientations by reduction of time step\n");
		(void) printf("modifying interface\n");
		(void) delete_curve(newc);
		status = YES;
		continue;
	    }
	    *dt_frac = min(*dt_frac,dt_f);

		/* Restore newc to original state ? 			*/
		/* This is unnecessary if reduce time step is called,   */
		/* as the newc->interface will be deleted, but this	*/
		/* may be desirable for consistency reasons if future	*/
		/* resolutions solve this problem without having to	*/
		/* reduce the time step.  For now this section is	*/
		/* commented out.					*/

	}
	debug_print("orient_consistency",
		"Left consistent_propagated_loop_orientations()\n");
	return status;
}		/*end consistent_propagated_loop_orientation*/



LOCAL	void check_bond_lengths(
	INTERFACE	*intfc)
{
	CURVE		*c;
	BOND		*b;
	double		oldbl, newbl;

	(void) next_bond(intfc,NULL,NULL);
	while (next_bond(intfc,&b,&c))
	{
	    oldbl = bond_length(b);
	    newbl = separation(b->end,b->start,intfc->dim);

	    if (oldbl != newbl)
	    {
	    	(void) printf("WARNING in check_bond_lengths(), "
	    	              "bond lengths not correct at end of "
	    	              "advance\nold %g new %g dlen %g\n",
	    		      oldbl,newbl,oldbl-newbl);
	    	(void) printf("bond - ");
		print_bond(b);
	    	(void) printf("\n");
	    }
	    bond_length(b) = newbl;
	}
}		/*end check_bond_lengths*/


LOCAL 	int 	advance_front3d_tracking_control(
	double	dt,
	double	*dt_frac,
	Front	*front,
	Front	**newfront,
	POINTER	wave)
{
	int step_status;

	DEBUG_ENTER(advance_front3d_tracking_control)

  	switch(Tracking_algorithm(front))
	{
	case NO_DYNAMIC_TRACKING:
	    step_status = GOOD_STEP;
	    break;

	case GRID_FREE_TRACKING:
	    step_status = preserve_front_advance_front3d(dt,dt_frac,front,
							 newfront,wave);
	    break;

	case GRID_BASED_TRACKING:
	    step_status = reconstruct_front_advance_front3d(dt,dt_frac,front,
						            newfront,wave);
	    break;
	
	case THREE_COMP_GRID_BASED_TRACKING:
	    step_status = reconstruct_front_advance_front3d(dt,dt_frac,front,
						            newfront,wave);
	    break;

	case MIXED_TRACKING:
	case LOCALLY_GRID_BASED_TRACKING:
	    step_status = mixed_advance_front3d(dt,dt_frac,front,
						newfront,wave);
	    break;

	case HYBRID_TRACKING:
	    step_status = hybrid_advance_front3d(dt,dt_frac,front,
						 newfront,wave);
	    break;

	default:
	    screen("ERROR in advance_front3d_tracking_control()\n"
		   "Unknown Tracking algorithm = %d\n",
		   Tracking_algorithm(front));
	    clean_up(ERROR);
	}
	if (step_status == GOOD_STEP)
            front->redis_flag = (*newfront)->redis_flag;
	else
	    front->redis_flag = NO;
	DEBUG_LEAVE(advance_front3d_tracking_control)
	return step_status;
}		/*end advance_front3d_tracking_control*/

/*
*		reconstruct_front_advance_front3d():
*
*	   The following program is the routine for three-dimensional
*	front advance.  It is called from each of the hyp_drivers in 
*	hyp/hdriver.c, since advancing the front is the first step 
*	of the hyperbolic solution.
*	   The advance front involves three steps; copy the front and 
*	interface,  perform a propagate 3d front, and redistribute the
*	front. 
*
*	   Note - The function appears here to be LOCAL, but access is 
*	provided by a pointer set in the function set_advance_front() 
*	above.	   
*
*
*/

/*ARGSUSED*/
LOCAL int reconstruct_front_advance_front3d(
	double		dt,
	double		*dt_frac,
	Front		*front,
	Front		**newfront,
	POINTER		wave)
{
	static const char *fname = "reconstruct_front_advance_front3d";
	boolean	   has_tracked_surfaces;
	int	   status;

	DEBUG_ENTER(reconstruct_front_advance_front3d)

	debug_print("front","Entered %s(step %d time %g dt %g)\n",fname,
				front->step,front->time,dt);
	debug_front("old_front","into advance front",front);

	*newfront = copy_front(front);
	(*newfront)->redis_flag = NO;
	Interface_redistributed(*newfront) = NO;
	has_tracked_surfaces = (front->interf->surfaces != NULL) ? YES : NO;
	if (pp_max_status(has_tracked_surfaces) == NO)
	{
	    set_size_of_intfc_state(size_of_state(front->interf));
	    set_copy_intfc_states(YES);
	    (*newfront)->interf = pp_copy_interface(front->interf);
	    status = ((*newfront)->interf != NULL) ? GOOD_STEP : ERROR_IN_STEP;
	    return return_advance_front(front,newfront,status,fname);
	}

	start_clock("copy_interface");

		/* Initialize Newfront */

	set_size_of_intfc_state(size_of_state(front->interf));
	set_copy_intfc_states(NO);
	(*newfront)->interf = pp_copy_interface(front->interf);
	if ((*newfront)->interf == NULL)
	{
	    status = ERROR_IN_STEP;
	    (void) printf("ERROR in advance_3d_front(), "
		          "unable to copy interface\n");
	    return return_advance_front(front,newfront,status,fname);
	}
	stop_clock("copy_interface");

		/* Propagate points on surfaces */

	start_clock("propagate");
	set_copy_intfc_states(YES);
	status = propagate_3d_front(wave,front,*newfront,dt,dt_frac,YES);
	debug_front("final_front","after propagate_3d_front front:",*newfront);
	stop_clock("propagate");

	if (status != GOOD_STEP)
	{
	    (void) printf("WARNING in %s(), "
	                  "propagate_3d_front() failed\n",fname);
	    return return_advance_front(front,newfront,status,fname);
	}
	interface_reconstructed((*newfront)->interf) = YES;
	return return_advance_front(front,newfront,status,fname);
}		/*end reconstruct_front_advance_front3d*/

LOCAL 	int 	hybrid_advance_front3d(
	double	dt,
	double	*dt_frac,
	Front	*front,
	Front	**newfront,
	POINTER	wave)
{
	int	step_status;
	double	dt_frac_restore = *dt_frac;
	DEBUG_ENTER(hybrid_advance_front3d)

	step_status = preserve_front_advance_front3d(dt,dt_frac,front,
						     newfront,wave);
	if (debugging("hybrid"))
	{
	    (void) printf("HYBRID  preserve_front...(): ");
	    print_time_step_status("time step status = ",step_status,"\n");
	}

	if (step_status != GOOD_STEP)
	{
	    *dt_frac = dt_frac_restore;
	    step_status = reconstruct_front_advance_front3d(dt,dt_frac,front,
							    newfront,wave);
	}

	DEBUG_LEAVE(hybrid_advance_front3d)
	return step_status;
}		/*end hybrid_advance_front3d*/

LOCAL int mixed_advance_front3d(
	double		dt,
	double		*dt_frac,
	Front		*front,
	Front		**newfront,
	POINTER		wave)
{
	int status;
	double   dt_frac_restore = *dt_frac;
	DEBUG_ENTER(mixed_advance_front3d)

	if (debugging("mixed"))
	{
	    (void) printf("TIMESTEP = %d\n",front->step);
	    (void) printf("MIXED  Redistribution_count(front) = %d\n",
			  Redistribution_count(front));
	    (void) printf("MIXED  Frequency_of_reconstruction(front) = %d\n",
			  Frequency_of_reconstruction(front));
	}
	status = preserve_front_advance_front3d(dt,dt_frac,
				    front,newfront,wave);

	/*do not use grid based */

	DEBUG_LEAVE(mixed_advance_front3d)
	return status;
}		/*end mixed_advance_front3d*/

LOCAL int test_lgb_interface(
	Front		*front,
	Front		**newfront)
{
	static const char *fname = "test_lgb_interface";
	int	   status;
	boolean	   do_redist = YES;
	
	DEBUG_ENTER(test_lgb_interface)

	*newfront = copy_front(front);
	
	start_clock("copy_interface");
	set_size_of_intfc_state(size_of_state(front->interf));
	set_copy_intfc_states(YES);
	
	(*newfront)->interf = pp_copy_interface(front->interf);
	if ((*newfront)->interf == NULL)
	{
	    (void) printf("ERROR in test_lgb_interface, "
	                  "unable to copy interface\n");
	    clean_up(ERROR);
	}
	
	stop_clock("copy_interface");

	interpolate_intfc_states((*newfront)->interf) = YES;
	
	make_interface_topology_lists((*newfront)->interf);
	status = repair_front_at_grid_crossing(*newfront);
	make_interface_topology_lists((*newfront)->interf);
	
	if (status == GOOD_STEP)
	    reset_normal_on_intfc((*newfront)->interf);
	else
	{
	    printf("ERROR test_lgb_interface, "
	    	   "repair_front_at_grid_crossing fails.\n");
	    clean_up(ERROR);
	}

	/*delete_interface((*newfront)->interf); */
	free_front(*newfront);

	DEBUG_LEAVE(test_lgb_interface)
}

LOCAL int preserve_front_advance_front3d(
	double		dt,
	double		*dt_frac,
	Front		*front,
	Front		**newfront,
	POINTER		wave)
{
	static const char *fname = "preserve_front_advance_front3d";
	boolean	   has_tracked_surfaces;
	int	   status;
	boolean	   do_redist = YES;
	int	   i;

	DEBUG_ENTER(preserve_front_advance_front3d)

	debug_print("front","Entered %s(step %d time %g dt %g)\n",fname,
				front->step,front->time,dt);
	debug_front("old_front","into advance front",front);

	do_redist = NO;
	if (front->step % Frequency_of_redistribution(front,GENERAL_WAVE) == 0)
	    do_redist = YES;

	*newfront = copy_front(front);
	(*newfront)->redis_flag = front->redis_flag;
	Interface_redistributed(*newfront) = NO;
	has_tracked_surfaces = (front->interf->surfaces != NULL) ? YES : NO;
	if (pp_max_status(has_tracked_surfaces) == NO)
	{
	    set_size_of_intfc_state(size_of_state(front->interf));
	    set_copy_intfc_states(YES);
	    (*newfront)->interf = pp_copy_interface(front->interf);
	    status = ((*newfront)->interf != NULL) ? GOOD_STEP : ERROR_IN_STEP;
	    return return_advance_front(front,newfront,status,fname);
	}

	start_clock("copy_interface");

		/* Initialize Newfront */

	set_size_of_intfc_state(size_of_state(front->interf));
	set_copy_intfc_states(NO);
	(*newfront)->interf = pp_copy_interface(front->interf);
	if ((*newfront)->interf == NULL)
	{
	    (void) printf("WARNING in advance_3d_front(), "
	                  "unable to copy interface\n");
	    return return_advance_front(front,newfront,ERROR_IN_STEP,fname);
	}
	stop_clock("copy_interface");

		/* Propagate points on surfaces */

	start_clock("propagate");
	set_copy_intfc_states(YES);

	status = propagate_3d_front(wave,front,*newfront,dt,dt_frac,NO);

	if (status != GOOD_STEP)
	{
	    (void) printf("WARNING in preserve_front_advance_front3d(), "
			  "status from propagate_3d_front() != GOOD_STEP\n");
	    print_time_step_status("time step status = ",status,"\n");
	    return return_advance_front(front,newfront,status,fname);
	}

	debug_front("final_front","after scatter front:",*newfront);
	stop_clock("propagate");

		/* Redistribute the New Front */

	interpolate_intfc_states((*newfront)->interf) = YES;
	switch (redistribute(*newfront,do_redist,NO)) 
	{
	case GOOD_REDISTRIBUTION:
	    Redistribution_count(front) = Redistribution_count(*newfront);
	    status = GOOD_STEP;
	    break;
	
	case UNABLE_TO_UNTANGLE:
	    *dt_frac *= TIME_STEP_REDUCTION_FACTOR(front->interf);
	    status = MODIFY_TIME_STEP;
	    (void) printf("WARNING in %s(), UNABLE_TO_UNTANGLE, "
	                  "redistribution of front failed\n",fname);
	    (void) printf("dt_frac set to %g\n",*dt_frac);
	    break;

	case MODIFY_TIME_STEP_REDISTRIBUTE: /* Not supported in 3d */
	    *dt_frac *= TIME_STEP_REDUCTION_FACTOR(front->interf);
	    status = MODIFY_TIME_STEP;
	    (void) printf("WARNING in %s(), "
			  "MODIFY_TIME_STEP_REDISTRIBUTE, ",fname);
	    (void) printf("dt_frac set to %g\n",*dt_frac);
	    break;
		
	case BAD_REDISTRIBUTION:
	    *dt_frac = Min_time_step_modification_factor(front);
	    status = MODIFY_TIME_STEP;
	    (void) printf("WARNING in %s(), BAD_REDISTRIBUTION, ",fname);
	    (void) printf("dt_frac set to %g\n",*dt_frac);
	    break;
	
	case INCONSISTENT_RECONSTRUCTION:
	    status = REPEAT_TIME_STEP;
	    (void) printf("WARNING in %s(), INCONSISTENT_RECONSTRUCTION\n",
	    		   fname);
	    break;

	default:
	    (void) printf("WARNING in %s(), "
	                  "unknown redistribution status\n",fname);
	    debug_front("ERROR_front","after error",*newfront);
	    clean_up(ERROR);
	    break;
	}
	if (status == GOOD_STEP)
	    reset_normal_on_intfc((*newfront)->interf);
	if (debugging("final_front"))
	    print_Front_structure(front);
	
	if(status == REPEAT_TIME_STEP)
	{
	    if(recon_repeat() >= 1)
	    {
		printf("ERROR preserve_front_advance_front3d, "
		       "repeat time step twice.\n");
		clean_up(ERROR);
	    }
	    else
	    {
		set_repeat(1);
	    }
	}
	else
	    set_repeat(0);

	return return_advance_front(front,newfront,status,fname);
}		/*end preserve_front_advance_front3d*/

/*
*
*
*			propagate_3d_front():
*
*	   This function performs the actual propagation of the front
*	to the new front location.  It consists of both a normal 
*	propagate and a tangential propagate, both of which are followed
*	by a scatter front step; for a total of four steps altogether.
*
*	Returns one of the following:
*		GOOD_STEP
*		ERROR_IN_STEP  (only from failure in node prop)
*		MODIFY_TIME_STEP
*/

void	tecplot_interface_states(const char *, INTERFACE *);

void    g_check_params_comp(POINTER, INTERFACE*);

LOCAL int propagate_3d_front(
	POINTER		wave,
	Front		*front,
	Front		*newfront,
	double		dt,
	double		*dt_frac,
	boolean		reconstruct_front)
{
	INTERFACE	*intfc_old;
	double		V[MAXD];
	int             step_status, tangent_status;
	DEBUG_ENTER(propagate_3d_front)

	debug_print("front","Entered propagate_3d_front().\n");

	intfc_old = front->interf;

 	if (front->step == 0)
            init_intfc_curvature3d2(newfront,newfront->interf);

	/*after redistribute or restart, the curvature is not calculated. */
        init_intfc_curvature3d(front, front->interf);
	
	if (front->_point_propagate != NULL)
	{
	    boolean tri_list_status;
	    start_clock("normal_propagate");

	    tri_list_status = 
	        make_tri_comp_lists(intfc_old) == FUNCTION_FAILED ? NO : YES;

	    if (pp_min_status(tri_list_status) == NO)
	    {
	        stop_clock("normal_propagate");
	    	screen("ERROR in propagate_3d_front(), "
	    	       "make_tri_comp_lists() failed\n");
	    	clean_up(ERROR);
	    }
	    
	    /* Set Default Propagation Limits */

	    set_propagation_limits(front,newfront);

	    start_clock("init_propagate");
	    init_propagate(front);
	    stop_clock("init_propagate");

	    /*set_wall_flag_for_surface(front->interf); */

	    if (front->_point_propagate != NULL)
                propagate_surface_points(front,newfront,wave,dt,V);
	    else if (front->surface_propagate != NULL)
	        surface_propagate(front,newfront,wave,dt,V);
	    
	    printf("front->curve_propagate = %d\n",front->curve_propagate);
	    printf("front->node_propagate = %d\n",front->node_propagate);
	    if (front->curve_propagate != NULL)
	        propagate_curve_points(front,newfront,wave,dt);
	
	    if (front->node_propagate != NULL)
	    {
	        step_status = propagate_node_points(front,newfront,
						    wave,dt,dt_frac);
		if (step_status != GOOD_STEP)
		{
		    DEBUG_LEAVE(propagate_3d_front)
		    return step_status;
		}
	    }
	    reset_normal_on_intfc(newfront->interf);
	    debug_front("np_front","after normal propagation",newfront);
	    stop_clock("normal_propagate");
	}
	
	debug_propagate_3d_front(newfront);

	interface_reconstructed(newfront->interf) = NO;
	/*prev_interface(newfront->interf) = front->interf; */

	if (reconstruct_front == YES)
	{
	    boolean reconstruct_status;
	    start_clock("reconstruct_front");
	    
	    reconstruct_status = reconstruct_front_at_grid_crossing(newfront);
	    
	    reconstruct_status = pp_min_status(reconstruct_status);
	    if (!reconstruct_status)
	    {
	    	stop_clock("reconstruct_front");
	    	(void) printf("WARNING in propagate_3d_front(), "
	    	              "reconstruct_front failed, "
	    	              "MODIFY_TIME_STEP\n");
	    	*dt_frac *= TIME_STEP_REDUCTION_FACTOR(front->interf);
		(void) printf("dt_frac = %g after scaling by "
			      "TIME_STEP_REDUCTION_FACTOR(front->interf)"
			      " = %g\n",*dt_frac,
			      TIME_STEP_REDUCTION_FACTOR(front->interf));
		DEBUG_LEAVE(propagate_3d_front)
		return MODIFY_TIME_STEP; 
	    }
	    stop_clock("reconstruct_front");
	}

	start_clock("scatter_front");
	if (!scatter_front(newfront))
	{
	    stop_clock("scatter_front");
	    (void) printf("WARNING in propagate_3d_front(), "
	                  "1st scatter_front() failed\n"
	                  "MODIFY_TIME_STEP\n");  
	    *dt_frac *= TIME_STEP_REDUCTION_FACTOR(front->interf);
	    DEBUG_LEAVE(propagate_3d_front)
	    return MODIFY_TIME_STEP; 
	}
	stop_clock("scatter_front");

        init_intfc_curvature3d(newfront,newfront->interf);
	if (front->_tan_point_propagate != NULL)
	{
	    start_clock("tangentiall");
	    
            front->tan_sec = NO;
	    tangent_status = propagate_points_tangentially(front,newfront,
							   reconstruct_front,
							   dt,dt_frac,
							   (front->step)%2);
	    if (tangent_status != GOOD_STEP)
	    {
		DEBUG_LEAVE(propagate_3d_front)
		return tangent_status;
	    }
	    front->tan_sec = YES;
	    tangent_status = propagate_points_tangentially(front,newfront,
							   reconstruct_front,
							   dt,dt_frac,
							   (front->step+1)%2);
	    if (tangent_status != GOOD_STEP)
	    {
		DEBUG_LEAVE(propagate_3d_front)
		return tangent_status;
	    }

	    stop_clock("tangentiall");
	}
	else if (front->tan_surface_propagate != NULL)
	{
	    INTERFACE *intfc = front->interf;
	    INTERFACE *new_intfc = newfront->interf;
	    SURFACE **olds,**news;

	    start_clock("tangentiall");
	    for (olds = intfc->surfaces, news = new_intfc->surfaces; 
			olds && *olds; ++olds, ++news)
            {
                (*front->tan_surface_propagate)(front,newfront,
                                              intfc,*olds,*news,dt);
            }
	    stop_clock("tangentiall");
	}

	debug_print("front","Left propagate_3d_front()\n");
	DEBUG_LEAVE(propagate_3d_front)
	return GOOD_STEP; 
}		/*end propagate_3d_front*/


EXPORT void   init_intfc_curvature3d(
       Front               *front,
       INTERFACE           *intfc)
{
       HYPER_SURF              *hs;
       HYPER_SURF_ELEMENT      *hse;
       FT_POINT                   *p;
     
      
       	(void) next_point(intfc,NULL,NULL,NULL);
       	while (next_point(intfc,&p,&hse,&hs))
           p->curvature = mean_curvature_at_point(p,hse,hs,front);
       
       	/*fix 3 comp curve case and also a nan bug */
       	(void) next_point(intfc,NULL,NULL,NULL);
       	while (next_point(intfc,&p,&hse,&hs))
       	{
	   if(Boundary_point(p))
	   {
               p->curvature = 0.0;
	       continue;
	   }
	   
	   if(isnan(p->curvature))
	   {
		int	i, nt;
		TRI	**ptris;

		/*in this case, triangle with 0 area may appear. */
		printf("WARNING init_intfc_curvature3d, "
	       	       "p->curvature=%24.16e, set to 0.0\n", p->curvature);
		print_general_vector("p=", Coords(p), 3, "\n");
		nt = set_tri_list_around_point(p,Tri_of_hse(hse),&ptris,intfc);
		p->curvature = 0.0;
		for(i=0; i<nt; i++)
		    print_tri(ptris[i], intfc);
		tecplot_tris("cur_tst", ptris, nt);
	   }
        }
	intfc->normal_unset = NO;
        intfc->curvature_unset = NO;
}      /* end init_intfc_curvature3d */

LOCAL void   init_intfc_curvature3d2(
       Front               *front,
       INTERFACE           *intfc)
{
       HYPER_SURF              *hs;
       HYPER_SURF_ELEMENT      *hse;
       FT_POINT                   *p;
       int                     i;

       (void) next_point(intfc,NULL,NULL,NULL);
        while (next_point(intfc,&p,&hse,&hs))
        {
            p->curvature = 0.0;
        }
}


	  
/*
*			propagate_points_tangentially():
*
*	This is the main driver program for the tangential sweep in 3D.
*	The procedure is to copy the incoming interface of newfront
*	into intfc_new, perform a loop over the two (geometrically identical)
*	interfaces intfc_tmp = newfront->interf,  and intfc_new and install
*	the output from the pointwise tan_point_propagate function into the
*	states on intfc_new.  At the end of the loop the original
*	newfront->interf = intfc_tmp is deleted and intfc_new is installed
*	as newfront->interf.  Finally the updated interface is marked as
*	reconstructed (since it is a copy of a reconstructed interface), and
*	the new front states are scattered across processors.
*
*	NOTE: The current tangential sweep function, f_tan_point_propagate,
*	implicitly assumes that the triangles about a given point project onto
*	the tangent plane defined by the discrete normal vector in such a
*	way as to produce a regular non-intersecting polygon.  This assumption
*	can be violated when a point is the vertex of a cone-like object.
*	Whether this occurs or not depends on the choice of the normal uni_array.
*	Internally, f_tan_point_propagate will attempt to compute an alternative
*	normal vector that does not produce a tangled projection.  If this is
*	not possible,  then f_tan_point_propagate will return a failure status.
*	If this occurs propagate_points_tangentially will remove the
*	offending point,  rescatter the modified interfaces,  recopy intfc_new,
*	and restart the tangential sweep loop.  Needless to say,  this is a
*	rather inefficient solution to the problem.  The hope is that such
*	irregular cases will be rare and this problem will only occur
*	infrequently in a given run.  Thus the goal is to have a robust,  if
*	costly,  way of treating such geometric degeneracies.
*/

LOCAL int propagate_points_tangentially(
	Front		*front,
	Front		*newfront,
	int		reconstruct_front,
	double		dt,
	double		*dt_frac,
	int		dir)
{
	INTERFACE               *intfc_tmp, *intfc_new;
	HYPER_SURF		*tmphs;
	HYPER_SURF_ELEMENT 	*tmphse;
	SURFACE                 **s_tmp, **s_new;
	FT_POINT			*tmpp, *newp;
	TRI                     *tri_tmp, *tri_new;
	int                     i;
	boolean                    modified;
	DEBUG_ENTER(propagate_points_tangentially)

	start_clock("copy_interface");
	intfc_tmp = newfront->interf;
	set_size_of_intfc_state(size_of_state(front->interf));
	if ((intfc_new = pp_copy_interface(intfc_tmp)) == NULL)
	{
	    screen("ERROR in propagate_points_tangentially(), "
		   "unable to copy interface\n");
	    clean_up(ERROR);
	}
	stop_clock("copy_interface");
	if (debugging("consistency") && !consistent_interface(intfc_new))
	{
	    screen("ERROR in propagate_points_tangentially(), "
		   "intfc_new is inconsistent\n");
	    clean_up(ERROR);
	}
	
	start_clock("tan_propagate");

	modified = NO;
	/* Reset sort status for points on intfc_tmp and intfc_new */
	(void) next_point(intfc_tmp,NULL,NULL,NULL);
	(void) next_point(intfc_new,NULL,NULL,NULL);
	for (s_tmp = intfc_tmp->surfaces, s_new = intfc_new->surfaces;
		s_tmp && *s_tmp && s_new && *s_new; ++s_tmp, ++s_new)
	{
	  tmphs = Hyper_surf(*s_tmp);
	  for (tri_tmp = first_tri(*s_tmp), tri_new = first_tri(*s_new);
	       !at_end_of_tri_list(tri_tmp,*s_tmp) &&
	       !at_end_of_tri_list(tri_new,*s_new);
	       tri_tmp = tri_tmp->next, tri_new = tri_new->next)
	  {
	    tmphse = Hyper_surf_element(tri_tmp);
	    for (i = 0; i < 3; ++i)
	    {
	      tmpp = Point_of_tri(tri_tmp)[i];
	      newp = Point_of_tri(tri_new)[i];
	      if (!sorted(tmpp) && !sorted(newp))
	      {
	        if (tan_point_propagate(front,tmpp,newp,tmphse,tmphs,dt,dir))
		  sorted(tmpp) = sorted(newp) = YES;
		else
		{
		  sorted(tmpp) = sorted(newp) = YES;
		}
	      }
	      else if (!sorted(tmpp) || !sorted(newp))
	      {
	        screen("ERROR in propagate_points_tangentially(), "
		       "point lists in intfc_tmp and intfc_new "
		       "inconsistent\n");
	        clean_up(ERROR);
	      }
	    }
	  }
	}
	stop_clock("tan_propagate");

	(void) delete_interface(intfc_tmp);
	newfront->interf = intfc_new;
	debug_front("tp_front","after tangential propagation",newfront);

	start_clock("scatter_front");
	start_clock("copy_interface");
	set_size_of_intfc_state(size_of_state(front->interf));
	if ((intfc_new = pp_copy_interface(newfront->interf)) == NULL)
	{
	    screen("ERROR in propagate_points_tangentially(), "
		   "unable to copy interface\n");
	    clean_up(ERROR);
	}
	stop_clock("copy_interface");

        init_intfc_curvature3d(newfront,newfront->interf);
	
	if(NO)
	{
	    char    s[50], sn[50];

	    sprintf(sn, "%s", right_flush(pp_mynode(),PP_NODE_FIELD_WIDTH));
	    sprintf(s, "curvature%s_%s", right_flush(newfront->step, 
	    			TSTEP_FIELD_WIDTH), sn);
	    /*tecplot_interface_states(s, newfront->interf);*/
	}
	
	for (i = 0; i < 2; ++i)
	{
	  if (scatter_front(newfront))
	      break;
	  else
	  {
	    printf("\n entered else part!");
	    delete_interface(newfront->interf);
	    newfront->interf = intfc_new;
	    start_clock("copy_interface");
	    set_size_of_intfc_state(size_of_state(front->interf));
	    if ((intfc_new = pp_copy_interface(newfront->interf)) == NULL)
	    {
	      screen("ERROR in propagate_points_tangentially(), "
		     "unable to copy interface\n");
	      clean_up(ERROR);
	    }
	    stop_clock("copy_interface");
	    interface_reconstructed(newfront->interf) = NO;
	    if (i == 1)
	    {
	      if (redistribute(newfront,YES,NO) != GOOD_REDISTRIBUTION) 
	      {
		  i = 2;
		  break;
	      }
	    }
	  }
	}
	if (intfc_new != NULL)
	    delete_interface(intfc_new);
	if (i == 2)
	{
	  (void) printf("WARNING in propagate_points_tangentially(), "
			"2nd scatter_front() failed\n"
			"MODIFY_TIME_STEP_NODE\n");
	  *dt_frac *= TIME_STEP_REDUCTION_FACTOR(front->interf);
	  DEBUG_LEAVE(propagate_points_tangentially)
	  return MODIFY_TIME_STEP; 
	}
	stop_clock("scatter_front");
	DEBUG_LEAVE(propagate_points_tangentially)
	return GOOD_STEP;
}		/*end propagate_points_tangentially*/

/*
*			unfold_interface_section():
*
*	This function called when tan_point_propagate() fails.  The presumed
*	reason for this failure is the the interface near the point being
*	propagated is folded,  i.e. the projection of the interface onto
*	the tangent plane at the point is tangled.  This function assumes
*	that the interfaces corresponding to s_tmp and s_new are copies of
*	one another so that the triangle and points lists are in exact
*	one-to-one correspondence with IDENTICAL positions and that the
*	surface tri lists are synchronized.  The procedure is to perform a
*	local redistribution of the interface sections about the points.
*	The procedure is as follows:
*
*	1. Search for very small tris about the points,  merge this tris with
*	   their neighbors.
*	2. If no small tris are found,  then delete the points tmpp and newp
*	   from their respective interfaces.
*
*/

LOCAL void unfold_interface_section(
	FT_POINT   *tmpp,
	FT_POINT   *newp,
	TRI     *tri_tmp,
	TRI     *tri_new,
	SURFACE *s_tmp,
	SURFACE *s_new)
{
	INTERFACE    *intfc_tmp = s_tmp->interface;
	INTERFACE    *intfc_new = s_new->interface;
	TRI          **tris;
	boolean         repeat, modified;
	double        lbar, lmin;
	int          i, imin, v;
	int          nt, nt_tmp, nt_new;
	static TRI   **tris_tmp = NULL, **tris_new = NULL;
	static double *l;
	static int   max_num_tris = 0;
	const double  ltol = 0.01;

	debug_print("unfold","Entered unfold_interface_section\n");
	if (debugging("unfold") && debugging("consistency"))
	{
	    if (!consistent_interface(intfc_tmp))
	    {
	        screen("ERROR in unfold_interface_section(), "
		       "intfc_tmp is inconsistent\n");
	        clean_up(ERROR);
	    }
	    if (!consistent_interface(intfc_new))
	    {
	        screen("ERROR in unfold_interface_section(), "
		       "intfc_new is inconsistent\n");
	        clean_up(ERROR);
	    }
	}
	modified = NO;
	do
	{
	  repeat = NO;
	  nt_tmp = set_tri_list_around_point(tmpp,tri_tmp,&tris,intfc_tmp);
	  if (nt_tmp > max_num_tris)
	  {
	    if (tris_tmp)
	      free_these(3,tris_tmp,tris_new,l);
	    max_num_tris = 2*nt_tmp+1;
	    uni_array(&tris_tmp,max_num_tris,sizeof(TRI*));
	    uni_array(&tris_new,max_num_tris,sizeof(TRI*));
	    uni_array(&l,max_num_tris,FLOAT);
	  }
	  for (i = 0; i < nt_tmp; ++i)
	    tris_tmp[i] = tris[i];
	  tris_tmp[nt_tmp] = NULL;
	  nt_new = set_tri_list_around_point(newp,tri_new,&tris,intfc_new);
	  if (nt_new != nt_tmp)
	  {
	    screen("ERROR in unfold_interface_section(), "
		   "inconsisent tri lists on tmp and new interfaces\n");
	    clean_up(ERROR);
	  }
	  nt = nt_tmp;
	  for (i = 0; i < nt; ++i)
	    tris_new[i] = tris[i];
	  tris_new[nt] = NULL;

	  for (i = 0, lbar = 0.0; i < nt; ++i)
	  {
	    v = Vertex_of_point(tris_tmp[i],tmpp);
	    l[i] = length_of_tri_side(tris_tmp[i],Next_m3(v));
	    lbar += l[i];
	  }
	  lbar /= nt;
	  if (debugging("unfold"))
	  {
	      (void) printf("lbar = %"FFMT"\n",lbar);
	      for (i = 0; i < nt; ++i)
	      {
	          (void) printf("l[%d] = %"FFMT", l[%d]/lbar = %"FFMT"\n",
				i,l[i],i,l[i]/lbar);
	      }
	  }

	  imin = -1;
	  lmin = HUGE_VAL;
	  for (i = 0; i < nt; ++i)
	  {
	    if (l[i] < ltol*lbar)
	    {
	      if (l[i] < lmin)
	      {
		lmin = l[i];
		imin = i;
	      }
	    }
	  }
	  if (imin != -1)
	  {
	    if (debugging("unfold"))
	    {
	      (void) printf("deleting tri_tmp(%llu) side %d\n",
			    tri_number(tri_tmp,intfc_tmp),imin);
	      print_tri(tri_tmp,intfc_tmp);
	      (void) printf("deleting tri_new(%llu) side %d\n",
			    tri_number(tri_new,intfc_new),imin);
	      print_tri(tri_new,intfc_new);
	    }
	    if (!delete_side_of_tri(tris_new[imin],s_new,imin) ||
	        !delete_side_of_tri(tris_tmp[imin],s_tmp,imin))
	    {
		screen("ERROR in unfold_interface_section(), "
			"delete_side_of_tri() failed\n");
		clean_up(ERROR);
	    }
	    repeat = YES;
	    modified = YES;
	  }
	} while(repeat);
	
	if (!modified)
	{
	  /* Are the tri lists closed? */
	  if ((nt_tmp >= 3) &&
	      (Next_tri_at_vertex(tris_tmp[0],tmpp) == tris_tmp[nt_tmp-1]) &&
	      (Prev_tri_at_vertex(tris_tmp[nt_tmp-1],tmpp) == tris_tmp[0]) &&
	      (Next_tri_at_vertex(tris_new[0],newp) == tris_new[nt_new-1]) &&
	      (Prev_tri_at_vertex(tris_new[nt_new-1],newp) == tris_new[0]))
	  {
	    if (debugging("unfold"))
	    {
		(void) printf("deleting vertex tmpp(%llu) of tri_tmp(%llu)\n",
			      point_number(tmpp),tri_number(tri_tmp,intfc_tmp));
		(void) printf("deleting vertex newp(%llu) of tri_new(%llu)\n",
			      point_number(newp),tri_number(tri_new,intfc_tmp));
	    }
	    if (!delete_vertex_of_tri(tmpp,tri_tmp,s_tmp) ||
	        !delete_vertex_of_tri(newp,tri_new,s_new))
	    {
	      screen("ERROR in unfold_interface_section(), "
		     "delete_vertex_of_tri() failed\n");
	      clean_up(ERROR);
	    }
	  }
	  else
	  {
	    double BBL[3], BBU[3];

	    set_tri_list_bounding_box(tris_tmp,nt_tmp,BBL,BBU,NO,YES);
	    set_tri_list_bounding_box(tris_new,nt_new,BBL,BBU,YES,YES);

	    screen("ERROR in unfold_interface_section(), can't unfold\n");
	    (void) printf("Boundary_point(tmpp) = %d\n",Boundary_point(tmpp));
	    (void) printf("nt_tmp = %d\n",nt_tmp);
	    for (i = 0; i < nt_tmp; ++i)
	    {
	      (void) printf("tris_tmp[%d]\n",i);
	      print_tri(tris_tmp[i],intfc_tmp);
	    }
	    gview_plot_triangle_list("","tris_tmp",tris_tmp,nt_tmp,
		                     0.1,0.0,0.0,0.9,0.0,0.0,0.5,BBL,BBU);
	    gview_plot_vertices("","tmpp",&tmpp,1,BBL,BBU);

	    (void) printf("Boundary_point(newp) = %d\n",Boundary_point(newp));
	    (void) printf("nt_new = %d\n",nt_new);
	    for (i = 0; i < nt_new; ++i)
	    {
	      (void) printf("tris_new[%d]\n",i);
	      print_tri(tris_new[i],intfc_new);
	    }
	    gview_plot_triangle_list("","tris_new",tris_new,nt_new,
		                     0.1,0.0,0.0,0.9,0.0,0.0,0.5,BBL,BBU);
	    gview_plot_vertices("","newp",&newp,1,BBL,BBU);
	    clean_up(ERROR);
	  }
	}
	debug_print("unfold","Left unfold_interface_section\n");
}		/*end unfold_interface_section*/

#define   MAX_NUM_CURVE   500

LOCAL	void detach_and_propagate_curves(
	Front	*front,
	Front	*newfront,
	POINTER wave,
	double	dt)
{
	INTERFACE    *sav_intfc = current_interface();
	SURFACE	     **s, *olds, *news;
	CURVE	     **new_curves[MAX_NUM_CURVE];
	CURVE	     **c, **cc, *newc, *pc;
	P_LINK	     *p_table;	
	int	     i, num_c, p_size;
	boolean	     found = NO;

	DEBUG_ENTER(detach_and_propagate_curves)

	/*store all paired curves */
	i = 0;
	for (c =  front->interf->curves, 
	     cc = newfront->interf->curves;
	     c && *c &&
	     cc && *cc; 
	     c++, cc++, i++)
	{
	    if(i >= MAX_NUM_CURVE)
	    {
		printf("ERROR detach_and_propagate_curves, too many curves.\n");
		clean_up(ERROR);
	    }
	    
	    new_curves[i] = NULL;
	    if(!add_to_pointers(*cc, &new_curves[i]))
	    {
	        printf("ERROR detach_and_propagate_curves, "
		       "can not add into new_curves.\n");
		clean_up(ERROR);
	    }
	}

	/*detach newfront surface */
	set_current_interface(newfront->interf);
	
	for(s=newfront->interf->surfaces; s && *s; s++)
	{
	    /*ASSUME the moving surface type */
	    if(wave_type(*s) >= FIRST_PHYSICS_WAVE_TYPE)
	    {
	        olds = *s;
		news = detach_one_surface(olds);
	        if(olds != news)
		    found = YES;
		break;
	    }
	}

	if(!found)
	{
	    for(i=0, c=front->interf->curves; c && *c; c++, i++)
	        curve_propagate(front, wave, *c, new_curves[i][0], dt);
	    
	    set_current_interface(sav_intfc);
	    DEBUG_LEAVE(detach_and_propagate_curves)
	    return;
	}
	
	/*printf("#detach surfce  %d to %d.\n", olds, news); */
	/*construct the olds and news pairs */
	num_c = 0;
	for(i = 0; i < 2; i++)
	{
	    c = i==0 ? news->pos_curves : news->neg_curves;
	    for(; c && *c; c++)
	        num_c++;
	}

	p_size = 4*num_c + 1;
	uni_array(&p_table,p_size,sizeof(P_LINK));
	reset_hash_table(p_table,p_size);
	
	for(i = 0; i < 2; i++)
	{
	    c =  i==0 ? olds->pos_curves : olds->neg_curves; 
	    cc = i==0 ? news->pos_curves : news->neg_curves; 

	    for( ; c  && *c && 
	           cc && *cc;  c++, cc++)
	    {
	        add_to_hash_table((POINTER)(*c),(POINTER)(*cc), p_table,p_size);
	    }
	}

	i = 0;
	for(c = front->interf->curves; c && *c; c++, i++)
	{
	    newc = new_curves[i][0];
	    pc = (CURVE*) find_from_hash_table((POINTER)newc,p_table,p_size);
	    if(pc != NULL)
	    {
	        if(curve_type(pc) != NEUMANN_CURVE_P)
		    delete_from_pointers(newc, &new_curves[i]);
		add_to_pointers(pc, &new_curves[i]);
	    }
	}
	
	free(p_table);
	/*delete the previous attached surface and associated curves */
	delete_scn(olds);

	if(debugging("detach_and_propagate_curves"))
	{
	    printf("#old curves\n");
	    for(c = front->interf->curves; c && *c; c++)
	        printf("#%p  %d\n", *c, curve_type(*c));
	
	    printf("#new curves\n");
	    for(c = newfront->interf->curves; c && *c; c++)
	        printf("#%p  %d\n", *c, curve_type(*c));

	    printf("#prop curves\n");
	    for(i=0, c = front->interf->curves; c && *c; c++, i++)
	    {
	        printf("#c  %p  %3d   ", *c, curve_type(*c));
	        for(cc = new_curves[i]; cc && *cc; cc++)
		    printf("|cc %p %3d  ", *cc, curve_type(*cc));
		printf("|\n");
	    }
	}

	set_current_interface(sav_intfc);

	for(i=0, c = front->interf->curves; c && *c; c++, i++)
	{
	    for(cc = new_curves[i]; cc && *cc; cc++)
	        curve_propagate(front, wave, *c, *cc, dt);
	}

	DEBUG_LEAVE(detach_and_propagate_curves)
}		/*end propagate_curve_points*/


LOCAL	void propagate_curve_points(
	Front	*front,
	Front	*newfront,
	POINTER wave,
	double	dt)
{
  	CURVE	**new_curves;
	
	DEBUG_ENTER(propagate_curve_points)
	start_clock("curve_propagate");

	detach_and_propagate_curves(front, newfront, wave, dt);

	order_interface(newfront->interf);
        for(new_curves = newfront->interf->curves; 
	    new_curves && *new_curves; new_curves++)
            reorder_curve_link_list(*new_curves);

	stop_clock("curve_propagate");
	
	DEBUG_LEAVE(propagate_curve_points)
}		/*end propagate_curve_points*/

LOCAL 	int	propagate_node_points(
	Front	*front,
	Front	*newfront,
	POINTER	wave,
	double	dt,
	double	*dt_frac)
{
	NODE		**on, *oldn, **nn, *newn;
	NODE_FLAG	flag;
	int		step_status, node_status;
	RPROBLEM	*rp;
	DEBUG_ENTER(propagate_node_points)

	start_clock("node_propagate");
	set_to_next_node_only(flag);
	step_status = GOOD_STEP;
	for (on = front->interf->nodes, nn = newfront->interf->nodes;
	     on && *on && nn && *nn && step_status==GOOD_STEP;
	     ++on , ++nn)
	{
	    oldn = *on;
	    newn = *nn;
	    node_status = (*front->node_propagate)(front,wave,oldn,newn,
						   &rp,dt,dt_frac,
						   flag,NULL);
	    switch(node_status)	
	    {
	    case GOOD_NODE:
	        break;
	    case ERROR_NODE:
	    default:
	        print_node_status("WARNING in propagate_node_points(), "
			          "node propagate failed with node_status ",
		                  node_status,"\n");
		step_status = ERROR_IN_STEP;
		break;
	    }
	}
	stop_clock("node_propagate");
	if (front->pp_grid)
	    step_status = syncronize_time_step_status(step_status,front->pp_grid);
	DEBUG_LEAVE(propagate_node_points)
	return step_status; 
}		/*end propagate_node_points*/
	   

LOCAL void debug_propagate_3d_front(
	Front 	*newfront)
{
	if (debugging("propagation_3d"))
	    gview_plot_interface("final_intfc_bs",newfront->interf);

	if (debugging("check_intersect"))
	{
	    CROSS  *cross;
	    boolean intersection_status;

	    intersection_status = intersections(newfront->interf,&cross,YES);
	    if (pp_min_status(intersection_status) == NO)
	    {
	    	screen("ERROR in debug_propagate_3d_front(), "
		       "intersections() failed.\n");
	    	gview_plot_interface("final_intfc_bs",newfront->interf);
	    	clean_up(ERROR);
	    }
	    intersection_status = (cross != NULL) ? NO : YES;
	    if (pp_min_status(intersection_status) == NO)
	    {
	    	screen("ERROR in debug_propagate_3d_front(), "
	    	       "before fscatter\n");
	    	gview_plot_interface("final_intfc_bs",newfront->interf);
	    	clean_up(ERROR);
	    }
	}

	if (debugging("check_tri_pt"))
	{
	    (void) printf("after propagating and before scatter_front()\n");
	    if (!consistent_interface(newfront->interf))
	    {
		screen("ERROR in debug_propagate_3d_front() "
		       "inconsistent interface pointers\n");
		clean_up(ERROR);
	    }
	}
}		/*end debug_propagate_3d_front*/

LOCAL void propagate_surface_points(
        Front           *front,
        Front           *newfront,
        POINTER         wave,
        double           dt,
        double           *V)
{
        INTERFACE               *intfc_old = front->interf;
        INTERFACE               *intfc_new = newfront->interf;
        HYPER_SURF              *oldhs, *newhs;
        HYPER_SURF_ELEMENT      *oldhse, *newhse;
        FT_POINT                   *oldp, *newp;
        DEBUG_ENTER(propagate_surface_points)

        start_clock("propagate_surface_points");

	(void) next_point(intfc_old,NULL,NULL,NULL);
        (void) next_point(intfc_new,NULL,NULL,NULL);
        while (next_point(intfc_old,&oldp,&oldhse,&oldhs) &&
             next_point(intfc_new,&newp,&newhse,&newhs))
        {
	    /*if(Boundary_point(newp))*/
	    if (Boundary_point(newp) && wave_type(oldhs) < 
			FIRST_PHYSICS_WAVE_TYPE)
	        continue;
	    point_propagate(front,wave,oldp,newp,oldhse,oldhs,dt,V);
	}

        stop_clock("propagate_surface_points");
        DEBUG_LEAVE(propagate_surface_points)
}               /*end propagate_surface_points*/

LOCAL	void check_2d_orientation(INTERFACE *intfc)
{
	CURVE **c;
	for (c = intfc->curves; c && *c; ++c)
	{
	    if (!is_closed_curve(*c)) continue;
	    (void) printf("Curve %llu: orientation = %d area = %f\n",
			curve_number(*c),(*c)->orientation,
			area_of_closed_curve(*c));
	}
}
