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
*				fredist2d.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*/


#define DEBUG_STRING    "redistribute"
#include <front/fdecs.h>		/* includes int.h, table.h */

	/* LOCAL Function Declarations */
LOCAL	INTERFACE	*make_save_intfc(INTERFACE*);
LOCAL	INTERFACE	*reset_interface_of_front(Front*,INTERFACE**);
LOCAL	boolean  attach_sink_nodes(Front*);
LOCAL	boolean  check_for_intersections(INTERFACE*,CROSS**,const boolean);
LOCAL	boolean  vector_wave_interaction(CROSS*);
LOCAL	boolean  too_many_tangled_points(CROSS*,double*,int);
LOCAL	boolean  outside_reflecting_bdry(FT_POINT*,INTERFACE*);
LOCAL 	boolean  topologically_close_crosses(CROSS*,CROSS*);
LOCAL	double redistribution_spacing(BOND*,CURVE*,double,Front*);
LOCAL	void  prepare_to_leave_redistribute2d(INTERFACE**);
LOCAL	void  set_force_tangle(boolean);
LOCAL	boolean elastic_tangle(CROSS*);


#if defined(DEBUG_STRING)
#define debug_bond_cross(cross,fr)					\
	if (cross != NULL && debugging("bond_cross"))			\
		print_crossing_elements(cross,fr->interf);
#define	DEBUG_FRONT(mesg,fr)	 debug_front(DEBUG_STRING,mesg,fr);
#else /* defined(DEBUG_STRING) */
#define debug_bond_cross(cross,fr)
#define	DEBUG_FRONT(mesg,fr)
#endif /* defined(DEBUG_STRING) */

LOCAL	boolean _force_tangle = NO;

EXPORT	boolean	force_tangle(void)
{
    return _force_tangle;
}		/*end force_tangle*/

LOCAL	void	set_force_tangle(
	boolean	y_or_n)
{
	_force_tangle = y_or_n;
}		/*end set_force_tangle*/

/*
*			redistribute2d():
*
*	The redistribution leaves the endpoints of CURVES fixed, arrang-
*	ing that the distance along the front between points is approx.
*	equal to the constant spacing scaled by a factor depending on the
*	local interface curvature. The meaning of 'approximately'
*	is determined by the value of the paramater type_redistribute.
*	As described at the top of this file, three different redistribution
*	algorithms are available, depending on the this value of paramater.
*
*	After a redistribution, the routine intersections() determines
*	whether the new front is self-intersecting.  If so, a physics
*	dependent routine untangle_front() will be called to deal with
*	the intersections, and redistribute should be called again.
*
*	Returns one of the following values
*
*		GOOD_REDISTRIBUTION
*		UNABLE_TO_UNTANGLE
*		BAD_REDISTRIBUTION
*		MODIFY_TIME_STEP_REDISTRIBUTE
*
*	If the do_redist flag is NO, the redistribution step
*	is omitted, leaving only the attachment of sinks and the
*	untangle step.
*
*	Redistribute() is designed to interact with the function
*	advance_front(). If do_redist is YES (the usual case)
*	and the untangle routines fail, then advance_front() will
*	be restarted with the do_redist flag equals NO.
*	The reason for this behavior is that it is possible for
*	the redistribution algorithm to produce unphysical tangles
*	that were not present in the original interface.  If on
*	this second attempt untangle again fails, them the error
*	condition BAD_REDISTRIBUTION is returned.
*/

EXPORT int redistribute2d(
	Front		*fr,
	boolean		do_redist,
	boolean		restart_init)
{
	CROSS		*cross;
	INTERFACE	*intfc = fr->interf;
	INTERFACE	*save_intfc[2];
	O_NODE		*onode_list;
	double		*h = fr->rect_grid->h;
	double		hx = h[0], hy = h[1];
	double		dt, *dt_frac, min_dt_frac;
	int		status;
	boolean		do_scatter = NO;
	int		small_loops_deleted;
	int		redist_status;
	boolean		force_redistribute;
	boolean		delete_small_loops_before_untangle;
	boolean		istatus;
	boolean		any_redist;
	int		num_untangles = 0;
	int		flag = NORMAL_ATTEMPT_TO_UNTANGLE;
	int		dim = fr->rect_grid->dim;
	static const int       MAX_NUM_UNTANGLES = 10;/*TOLERANCE*/
	static boolean	first = YES;
	static double	dt_max, dt_min;
	DEBUG_ENTER(redistribute2d)

	if (first)
	{
	    first = NO;
	    dt_max = HUGE_VAL;
	    dt_min = 0.0;
	}

	if (fr->interf->curves == NULL)
	{
	    DEBUG_LEAVE(redistribute2d)
	    return GOOD_REDISTRIBUTION;
	}

	if (DEBUG)
	{
	    if (Frequency_of_redistribution(fr,GENERAL_WAVE) > 0)
	    {
	        (void) printf("Count redistribute(%d) %% "
	                      "general curve frequency redistribute(%d) = %d\n",
	    		      Redistribution_count(fr),
			      Frequency_of_redistribution(fr,GENERAL_WAVE),
			      Redistribution_count(fr) % 
	    		      Frequency_of_redistribution(fr,GENERAL_WAVE));
	    }
	    if (Frequency_of_redistribution(fr,VECTOR_WAVE) > 0)
	    {
	        (void) printf("Count redistribute(%d) %% "
	                      "uni_array curve frequency redistribute(%d) = %d\n",
	    		      Redistribution_count(fr),
			      Frequency_of_redistribution(fr,VECTOR_WAVE),
	    		      Redistribution_count(fr) %
	    		      Frequency_of_redistribution(fr,VECTOR_WAVE));
	    }
	    DEBUG_FRONT("at start of redistribute2d()",fr)
	    (void) printf("Interface before redistribute2d\n");
	    print_interface(intfc);
	}

	save_intfc[0] = make_save_intfc(intfc);
	save_intfc[1] = NULL;

	print_storage("before redistribute","REDIST_storage");
	start_clock("redistribute");

        if (!pp_min_status(attach_sink_nodes(fr)))
        {
            (void) printf("WARNING - in redistribute2d(), "
                          "attach_sink_nodes() failed\n");
            (void) reset_interface_of_front(fr,save_intfc);
            prepare_to_leave_redistribute2d(save_intfc);
            DEBUG_LEAVE(redistribute2d)
            return BAD_REDISTRIBUTION;
        }

	DEBUG_FRONT("after attach_sink_nodes",fr)

	    /* Redistribute Interface */

	Interface_redistributed(fr) = NO;
	small_loops_deleted = NO;
	force_redistribute = NO;
	delete_small_loops_before_untangle = NO;

redistribute_interface:

	if (do_redist)
	{
	    boolean status;

	    status = (intfc->curves == NULL) ? YES :
	    		(Interface_redistributed(fr)) ? YES :
	    		Curve_redistribute(fr,&force_redistribute);

	    status = closed_curve_node_redistribute(intfc,status);

            if (!pp_min_status(status))
            {
                (void) printf("WARNING in redistribute2d(), "
                              "redistribution failed\n");
                (void) reset_interface_of_front(fr,save_intfc);
                prepare_to_leave_redistribute2d(save_intfc);
                DEBUG_LEAVE(redistribute2d)
                return BAD_REDISTRIBUTION;
            }
	}

        any_redist = pp_max_status(Interface_redistributed(fr));

	if (any_redist)
	{
	    /* redistribute or untangle may produce 2 and 3 bond loops */

	    delete_small_loops(fr);
	    if (!restart_init)
	    {
	    	print_storage("before copy/delete intfc",
	    		      "REDIST_storage");
	    	start_clock("copy/delete intfc");
	    	set_size_of_intfc_state(size_of_state(fr->interf));
	    	set_copy_intfc_states(YES); /* Ensure states copied */
	    	fr->interf = copy_interface(intfc);
	    	(void) delete_interface(intfc);
	    	intfc = fr->interf;
	    	set_copy_intfc_states(YES);
	    	stop_clock("copy/delete intfc");
	    	print_storage("after copy/delete intfc","REDIST_storage");
	    }
	    do_scatter = YES;
	}

	DEBUG_FRONT("before intersections check",fr)

	    	/* Check for Intersections in Front */
	if (DEBUG) (void) printf("Checking for intersections\n");
	istatus = check_for_intersections(intfc,&cross,YES);
	if (!istatus)
	{
	    if (debugging("pionfail"))
	    	print_interface(intfc);
	    (void) reset_interface_of_front(fr,save_intfc);
	    prepare_to_leave_redistribute2d(save_intfc);
	    DEBUG_LEAVE(redistribute2d)
	    return BAD_REDISTRIBUTION;
	}

	if (DEBUG) (void) printf("Intersections check completed\n");

	if (interface_is_tangled(cross))
	{
	    static const char *redist_mesg =   "redistributed hyp ";
	    static const char *unredist_mesg = "unredistributed hyp ";
	    int	              num_tangles;

	    ++num_untangles;
	    if (DEBUG)
	    {
	    	(void) printf("Entering untangle block, "
	    	              "attempt number %d.\n",num_untangles);
	    }

	    num_tangles = print_number_of_tangles(
	    	(Interface_redistributed(fr)==YES)?redist_mesg:unredist_mesg,
		fr->interf,cross);

	    if (num_untangles > MAX_NUM_UNTANGLES)
	    {
	    	(void) printf("WARNING in redistribute2d(), too many attempts "
			      "(%d)  to untangle without success.\n",
			      num_untangles);
	    	if (DEBUG && (cross != NULL))
	    	    print_intersections(cross,intfc);
	    	(void) reset_interface_of_front(fr,save_intfc);
	    	prepare_to_leave_redistribute2d(save_intfc);
	    	DEBUG_LEAVE(redistribute2d)
	    	return BAD_REDISTRIBUTION;
	    }
	    if ((save_intfc[0] != NULL) && any_redist &&
	        vector_wave_interaction(cross))
	    {
	    	CROSS	   *ur_cross = NULL;
		CROSS      *cr;
		HYPER_SURF *hs;
	    	int	   unredist_num_tangles;

	    	/* Check to see if redistribute created uni_array
	    	 * tangles.  We do not allow this, as the
	    	 * untangle code may not be able to handle the
	    	 * resulting (possibly unphysical) configuration. */

	    	if (DEBUG)
	    	    (void) printf("Second intersections check\n");
	    	istatus = check_for_intersections(save_intfc[0],&ur_cross,YES);
	    	if (!istatus)
	    	{
	    	    (void) printf("Second intersections check "
	    	                  "detects bad status.\n");
	    	    if (debugging("pionfail"))
	    	    {
	    	        (void) printf("Unredistributed Interface:\n\n");
	    	        print_interface(save_intfc[0]);
	    	        (void) printf("Redistributed Interface:\n\n");
	    	        print_interface(intfc);
	    	    }
	    	    (void) reset_interface_of_front(fr,save_intfc);
	    	    prepare_to_leave_redistribute2d(save_intfc);
	    	    DEBUG_LEAVE(redistribute2d)
	    	    return BAD_REDISTRIBUTION;
	    	}
	    	if (DEBUG)
	    	    (void) printf("Second intersections check completed\n");
	        if (!interface_is_tangled(ur_cross))
		{
		    /*
		     * The unredistributed interface is untangled
		     * Omit redistribution on this time step
		     */
	    	    (void) reset_interface_of_front(fr,save_intfc);
		    intfc = fr->interf;
	            save_intfc[0] = make_save_intfc(intfc);
	            any_redist = NO;
		    cross = NULL;
		}
		else
		{
		    boolean repeat_redistribute = NO;
	    	    unredist_num_tangles = print_number_of_tangles(
	    			unredist_mesg,save_intfc[0],ur_cross);
	    	    if (unredist_num_tangles != num_tangles)
	    	    {
			repeat_redistribute = YES;
	    		(void) printf("WARNING in redistribute2d(), "
	    		              "redistribute causes vector tangle.\n");
		        for (cr = cross; cr != NULL; cr = cr->next)
		        {
			    if (wave_type(cr->c1) >=
				FIRST_VECTOR_PHYSICS_WAVE_TYPE)
			    {
				hs = Hyper_surf(cr->c1);
			        do_not_redistribute(hs) = YES;
				hs = correspond_hyper_surf(hs);
				if (hs != NULL)
			            do_not_redistribute(hs) = YES;
				else
				    repeat_redistribute = NO;
			    }
			    if (wave_type(cr->c2) >=
				FIRST_VECTOR_PHYSICS_WAVE_TYPE)
			    {
				hs = Hyper_surf(cr->c2);
			        do_not_redistribute(cr->c2) = YES;
				hs = correspond_hyper_surf(hs);
				if (hs != NULL)
			            do_not_redistribute(hs) = YES;
				else
				    repeat_redistribute = NO;
			    }
		        }
		    }
		
                    if (pp_max_status(repeat_redistribute))
                    {
                        (void) reset_interface_of_front(fr,save_intfc);
                        prepare_to_leave_redistribute2d(save_intfc);
                        status = redistribute2d(fr,do_redist,restart_init);
                        DEBUG_LEAVE(redistribute2d)
                        return status;
                    }
	    	}
	    }
	}
	if (interface_is_tangled(cross))
	{
	    if (restart_init) 
	    {
	    	(void) printf("WARNING in redistribute2d(), "
	    	              "Restart interface tangled, cannot continue\n");
	    	(void) reset_interface_of_front(fr,save_intfc);
	    	prepare_to_leave_redistribute2d(save_intfc);
	    	DEBUG_LEAVE(redistribute2d)
	    	return BAD_REDISTRIBUTION;
	    }

	    set_force_tangle(NO);

	    if (elastic_tangle(cross) && fr->elastic_untangle != NULL)
	    {
	        status = (*fr->elastic_untangle)(fr,&cross);
	        status = synchronize_untangle_status(status);
		if (status != CURVES_UNTANGLED)
		{
	    	    (void) printf("WARNING in redistribute2d(), "
	    	              "grid based untangle failed\n");
	    	    redist_status = (do_redist &&
	    			(Interface_redistributed(fr))) ?
	    		     UNABLE_TO_UNTANGLE : BAD_REDISTRIBUTION;
	    	    (void) reset_interface_of_front(fr,save_intfc);
	    	    prepare_to_leave_redistribute2d(save_intfc);
	    	    DEBUG_LEAVE(redistribute2d)
	    	    return BAD_REDISTRIBUTION;
		}
	    }

	    if (too_many_tangled_points(cross,h,dim))
	    {
	        status = (*fr->grid_based_untangle)(fr,&cross);
	        status = synchronize_untangle_status(status);
		if (status != CURVES_UNTANGLED)
		{
	    	    (void) printf("WARNING in redistribute2d(), "
	    	              "grid based untangle failed\n");
	    	    redist_status = (do_redist &&
	    			(Interface_redistributed(fr))) ?
	    		     UNABLE_TO_UNTANGLE : BAD_REDISTRIBUTION;
	    	    (void) reset_interface_of_front(fr,save_intfc);
	    	    prepare_to_leave_redistribute2d(save_intfc);
	    	    DEBUG_LEAVE(redistribute2d)
	    	    return BAD_REDISTRIBUTION;
		}
	    }

	    if (DEBUG)
	    {
	    	(void) printf("Checking to delete small ");
	    	(void) printf("loops before untangle\n");
	    }

            if (pp_max_status(delete_small_loops_before_untangle))
            {
                eliminate_small_loops(intfc,hx,hy,&cross);
                small_loops_deleted = YES;
            }

	    /* Make backup copy of interface in case untangle fails */

	    save_intfc[1] = make_save_intfc(intfc);


	    	/* Boundary untangle */

	    if (fr->fr_bdry_untangle)
	    {
	        if (DEBUG)
	    	    (void) printf("Calling boundary untangle\n");
	        status = (cross == NULL) ? CURVES_UNTANGLED :
	    			           (*fr->fr_bdry_untangle)(fr,&cross,
	    		                                           NULL,NULL,
								   flag);
	        status = synchronize_untangle_status(status);

	        switch (status)
	        {
	        case CURVES_UNTANGLED:
	    	    break;
	        case MODIFY_TIME_STEP_TO_UNTANGLE:
	    	    (void) printf("WARNING in redistributed2d, "
	    	                  "bdry_untangle returns \n"
	    	                  "\t\tMODIFY_TIME_STEP_TO_UNTANGLE\n");
	    	    (void) reset_interface_of_front(fr,save_intfc);
	    	    prepare_to_leave_redistribute2d(save_intfc);
	    	    DEBUG_LEAVE(redistribute2d)
	    	    return MODIFY_TIME_STEP_REDISTRIBUTE;
	        case MODIFY_TIME_STEP_TO_UNTANGLE_BUT_FORCE_TANGLE:
	    	    (void) printf("WARNING in redistributed2d, "
	    	                  "bdry_untangle returns \n"
	    	                  "\t\tMODIFY_TIME_STEP_TO_"
	    	                  "UNTANGLE_BUT_FORCE_TANGLE\n");
	    	    dt_frac = fr->dt_frac;
	    	    set_force_tangle(YES);
	    	    dt_max = dt = fr->dt;
	    	    min_dt_frac = dt_min/dt;
	    	    if (*dt_frac < min_dt_frac)
	    		*dt_frac = 0.5*(min_dt_frac + 1.0);
	    	    (void) reset_interface_of_front(fr,save_intfc);
	    	    prepare_to_leave_redistribute2d(save_intfc);
	    	    DEBUG_LEAVE(redistribute2d)
	    	    return MODIFY_TIME_STEP_REDISTRIBUTE;
	        case ERROR_IN_UNTANGLE:
	        default:
	    	    if (do_redist && 
	    	        (Curve_redistribution_function(fr) != NULL) && 
	    	        (!Interface_redistributed(fr)))
	    	    {
	    	        (void) printf("WARNING in redistribute2d(), "
	    	                      "unable to untangle front and"
	    	                      " boundary on first attempt\n"
	    	                      "Redistributing and trying again\n");
	    	        force_redistribute = YES;
	    	        intfc = reset_interface_of_front(fr,save_intfc+1);
	    	        goto redistribute_interface;
	    	    }
	    	    else if (!small_loops_deleted) 
	    	    {
	    	        (void) printf("WARNING in redistribute2d(), "
	    	                      "unable to untangle front and "
	    	                      "boundary\n"
	    	                      "Deleting small loops and "
	    	                      "trying again\n");
	    	        intfc = reset_interface_of_front(fr,save_intfc+1);
	    	        delete_small_loops_before_untangle = YES;
	    	        goto redistribute_interface;
	    	    }
	    	    (void) printf("WARNING in redistribute2d(), "
	    	                  "unable to untangle front and bdry\n");
	    	    (void) reset_interface_of_front(fr,save_intfc);
	    	    redist_status = (do_redist &&
	    			Interface_redistributed(fr)) ?
	    		UNABLE_TO_UNTANGLE : BAD_REDISTRIBUTION;
	    	    prepare_to_leave_redistribute2d(save_intfc);
	    	    DEBUG_LEAVE(redistribute2d)
	    	    return redist_status;
	        }
	        DEBUG_FRONT("after boundary untangle:",fr)
	    }

	    if (debugging("intersect"))
	    {
	        (void) printf("After fr_bdry_untangle.\n");
	        if (cross)
	        {
	           (void) printf("Printing interior intersections only:\n");
	           print_intersections(cross,intfc);
	        }
	        else
	           (void) printf("No interior intersections\n");
	    }
	    debug_bond_cross(cross,fr)

	/* Interior untangle */

	    if (interface_is_tangled(cross))
	    {
	        if (DEBUG)
	    	    (void) printf("Calling untangle front\n");
	        status = (cross == NULL) ? CURVES_UNTANGLED :
	    		(fr->untangle_front) ?
	                        (*fr->untangle_front)(fr,&cross,flag) :
	                        ERROR_IN_UNTANGLE;

	        status = synchronize_untangle_status(status);

	        switch (status)
	        {
	        case CURVES_UNTANGLED:
	            break;
	        case MODIFY_TIME_STEP_TO_UNTANGLE:
	    	    (void) printf("WARNING in redistributed2d, "
	    	                  "untangle returns "
	    	                  "MODIFY_TIME_STEP_TO_UNTANGLE\n");
	    	    (void) reset_interface_of_front(fr,save_intfc);
	            prepare_to_leave_redistribute2d(save_intfc);
	    	    DEBUG_LEAVE(redistribute2d)
	            return MODIFY_TIME_STEP_REDISTRIBUTE;
	        case MODIFY_TIME_STEP_TO_UNTANGLE_BUT_FORCE_TANGLE:
	    	    (void) printf("WARNING in redistributed2d, "
	    	                  "untangle returns "
	    	                  "MODIFY_TIME_STEP_TO_UNTANGLE_");
	    	    (void) printf("BUT_FORCE_TANGLE\n");
	    	    set_force_tangle(YES);
	    	    dt_frac = fr->dt_frac;
	    	    dt_max = dt = fr->dt;
	    	    min_dt_frac = dt_min/dt;
	    	    if (*dt_frac < min_dt_frac)
	    		*dt_frac = 0.5*(min_dt_frac + 1.0);
	    	    (void) reset_interface_of_front(fr,save_intfc);
	    	    prepare_to_leave_redistribute2d(save_intfc);
	    	    DEBUG_LEAVE(redistribute2d)
	    	    return MODIFY_TIME_STEP_REDISTRIBUTE;
	        case ERROR_IN_UNTANGLE:
	    	    if (do_redist && 
	    	        (Curve_redistribution_function(fr) != NULL) && 
	    	        (!Interface_redistributed(fr)))
	            {
	                (void) printf("WARNING in redistribute2d(), "
	                              "unable to untangle interior"
	                              " crosses on first attempt\n");
	                (void) printf("Redistributing and trying again\n");
	                force_redistribute = YES;
	                flag = DIFFICULT_ATTEMPT_TO_UNTANGLE;
	    	        intfc = reset_interface_of_front(fr,save_intfc+1);
	                goto redistribute_interface;
	            }
	            else if (!small_loops_deleted) 
	            {
	                (void) printf("WARNING in redistribute2d(), "
	                              "unable to untangle interior crosses\n");
	    	        (void) printf("Deleting small loops and ");
	                (void) printf("trying again\n");
	                flag = LAST_ATTEMPT_TO_UNTANGLE;
	    	        intfc = reset_interface_of_front(fr,save_intfc+1);
	                delete_small_loops_before_untangle = YES;
	                goto redistribute_interface;
	            }
	            (void) printf("WARNING in redistribute2d(), "
	                          "unable to unravel the interface\n");
	    	    (void) reset_interface_of_front(fr,save_intfc);
	    	    redist_status = (do_redist &&
	    			Interface_redistributed(fr)) ?
	    		UNABLE_TO_UNTANGLE : BAD_REDISTRIBUTION;
	    	    prepare_to_leave_redistribute2d(save_intfc);
	    	    DEBUG_LEAVE(redistribute2d)
	    	    return redist_status;
	        }
	    }
	    else
	    {
	    	(void) delete_interface(save_intfc[1]);
	    	save_intfc[1] = NULL;
	    }
	    dt_min = 0.0;	dt_max = HUGE_VAL;

	    /* Untangle may produce zero length bonds */

	    if (DEBUG)
	    	(void) printf("Deleting very short bonds\n");
	    intfc_delete_very_short_bonds(fr);

	    if (!pp_min_status(intfc_delete_fold_back_bonds(fr)))
	    {
	    	(void) printf("WARNING in redistribute2d(), "
	    	              "unable to delete fold back bonds\n");
	    	(void) reset_interface_of_front(fr,save_intfc);
	    	prepare_to_leave_redistribute2d(save_intfc);
	    	DEBUG_LEAVE(redistribute2d)
	    	return BAD_REDISTRIBUTION;
	    }

	    if (DEBUG)
	    	(void) printf("Calling scatter front\n");

	    if (!pp_min_status(correct_for_exterior_curves(fr)))
	    {
	    	(void) printf("WARNING in redistribute2d(), "
	                  "unable to correct for exterior curves\n");
	    	(void) reset_interface_of_front(fr,save_intfc);
	    	prepare_to_leave_redistribute2d(save_intfc);
	    	DEBUG_LEAVE(redistribute2d)
	    	return BAD_REDISTRIBUTION;
	    }
	    if (!scatter_front(fr))
	    {
	    	(void) printf("WARNING in redistributed2d, "
	    	              "scatter_front() failed, "
	    	              "MODIFY_TIME_STEP_TO_UNTANGLE\n");
	    	(void) reset_interface_of_front(fr,save_intfc);
	    	prepare_to_leave_redistribute2d(save_intfc);
	    	dt_frac = fr->dt_frac;
	    	*dt_frac = Max_time_step_modification_factor(fr);
	    	DEBUG_LEAVE(redistribute2d)
	    	return MODIFY_TIME_STEP_REDISTRIBUTE;
	    }

	    do_scatter = NO;
	    DEBUG_FRONT("after interior untangle:",fr)
	    if (save_intfc[1] != NULL)
	    {
	    	delete_interface(save_intfc[1]);
		save_intfc[1] = NULL;
	    }
	    goto redistribute_interface;
	}
	else if (force_tangle() && fr->dt > 0.0)
	{
	    dt_frac = fr->dt_frac;
	    dt = fr->dt;
	    dt_min = dt;
	    min_dt_frac = 0.5*(dt_max + dt)/dt;
	    *dt_frac = max(*dt_frac,min_dt_frac);
	    (void) reset_interface_of_front(fr,save_intfc);
	    prepare_to_leave_redistribute2d(save_intfc);
	    DEBUG_LEAVE(redistribute2d)
	    return MODIFY_TIME_STEP_REDISTRIBUTE;
	}

	if (!pp_min_status(correct_for_exterior_curves(fr)))
	{
	    (void) printf("WARNING in redistribute2d(), "
	                  "unable to correct for exterior curves\n");
	    (void) reset_interface_of_front(fr,save_intfc);
	    prepare_to_leave_redistribute2d(save_intfc);
	    DEBUG_LEAVE(redistribute2d)
	    return BAD_REDISTRIBUTION;
	}

	DEBUG_FRONT("after correction for exterior curves",fr)
	if (fr->interf->modified)
	    do_scatter = YES;

        if (pp_max_status(do_scatter))
	{

	    if (!scatter_front(fr))
	    {
	    	(void) printf("WARNING in redistributed2d, "
	    	              "scatter_front() failed, "
	    	              "\t\tMODIFY_TIME_STEP_TO_UNTANGLE\n");
	    	(void) reset_interface_of_front(fr,save_intfc);
	    	prepare_to_leave_redistribute2d(save_intfc);
	    	dt_frac = fr->dt_frac;
	    	*dt_frac = Max_time_step_modification_factor(fr);
	    	DEBUG_LEAVE(redistribute2d)
	    	return MODIFY_TIME_STEP_REDISTRIBUTE;
	    }
	}

	DEBUG_FRONT("after redistribute:",fr)
	prepare_to_leave_redistribute2d(save_intfc);
	DEBUG_LEAVE(redistribute2d)
	return GOOD_REDISTRIBUTION;
}		/*end redistribute2d*/

LOCAL	boolean check_for_intersections(
	INTERFACE *intfc,
	CROSS     **cross,
	const boolean   bdry)
{
	CROSS *cr;
	boolean istatus;
	RECT_GRID *gr = computational_grid(intfc);

	istatus = intersections(intfc,cross,bdry);
	if (!istatus)
	{
	    (void) printf("WARNING in check_for_intersections(), "
	                  "intersections() failed");
	    if (pp_numnodes() > 1)
		(void) printf(" on node %d\n",pp_mynode());
	    else
		(void) printf("\n");
	}
	if (!pp_min_status(istatus))
	{
	    if (istatus)
	    {
	        (void) printf("WARNING in redistribute2d(), "
	                      "intersections() failed on remote node\n");
	    }
	    return FUNCTION_FAILED;
	}

	if (*cross != NULL)
	{
	    for (cr = *cross; cr; cr = cr->next)
	    {
	        if (is_passive_boundary(cr->c1) || is_passive_boundary(cr->c2))
		    /*|| outside_reflecting_bdry(cr->p,intfc)) */
	        {
		    if (*cross == cr)
		        *cross = cr->next;
	    	    delete_from_cross_list(cr);
	        }
	    }
	}
	return FUNCTION_SUCCEEDED;
}		/*end check_for_intersections*/

LOCAL	INTERFACE *reset_interface_of_front(
	Front		*fr,
	INTERFACE	**save_intfc)
{
	if (save_intfc == NULL || *save_intfc == NULL)
	    return fr->interf;
	(void) delete_interface(fr->interf);
	fr->interf = *save_intfc;
	set_current_interface(fr->interf);
	*save_intfc = NULL;
	return fr->interf;
}		/*end reset_interface_of_front*/

LOCAL	void prepare_to_leave_redistribute2d(
	INTERFACE	**save_intfc)
{
	if (save_intfc[0] != NULL)
	    (void) delete_interface(save_intfc[0]);
	if (save_intfc[1] != NULL)
	    (void) delete_interface(save_intfc[1]);
	stop_clock("redistribute");
	print_storage("after redistribute","REDIST_storage");
}		/*end prepare_to_leave_redistribute2d*/


/*
*			make_save_intfc():
*
*	Creates a backup copy of intfc and leaves the current interface 
*	unaltered.
*/

LOCAL	INTERFACE *make_save_intfc(
	INTERFACE	*intfc)
{
	INTERFACE	*sav_intfc, *cur_intfc;
	boolean		sav_interp;
	
	print_storage("before make save intfc","REDIST_storage");
	start_clock("make save intfc");
	cur_intfc = current_interface();
	sav_interp = interpolate_intfc_states(intfc);
	set_add_to_correspond_list(YES);
	set_size_of_intfc_state(size_of_state(intfc));
	set_copy_intfc_states(YES); /* Ensure states copied */
	if ((sav_intfc = copy_interface(intfc)) == NULL) 
	{
	    set_current_interface(cur_intfc);
	    return NULL;
	}
	interpolate_intfc_states(sav_intfc) = sav_interp;
	set_current_interface(cur_intfc);
	stop_clock("make save intfc");
	print_storage("after make save intfc","REDIST_storage");
	return sav_intfc;
}		/*end make_save_intfc*/

/*
*			expansion_redistribute():
*
*	Defines a redistribution of an interface which never drops
*	points.   New points are added to keep the maximum front
*	spacing below the quantity space.
*/

EXPORT boolean expansion_redistribute(
	Front		*fr,
	boolean		*force_redistribute)
{
	INTERFACE	*interf = fr->interf;
	CURVE		**c;
	boolean		save_intrp = interpolate_intfc_states(interf);

	DEBUG_ENTER(expansion_redistribute)

	interpolate_intfc_states(interf) = YES;
	for (c = interf->curves; *c; ++c) 
	{
	    if (omit_redistribution(*c))
		continue;
	    if(wave_type(*c) == MOVABLE_BODY_BOUNDARY)
		continue;
	    if(wave_type(*c) == ELASTIC_BOUNDARY)
		continue;
	    if (!expand_redist_cur(fr,*c))
	    {
	    	Interface_redistributed(fr) = NO;
		interpolate_intfc_states(interf) = save_intrp;
	    	DEBUG_LEAVE(expansion_redistribute)
	    	return NO;
	    }
	}

	Interface_redistributed(fr) = YES;
	interpolate_intfc_states(interf) = save_intrp;
	*force_redistribute = NO;
	DEBUG_LEAVE(expansion_redistribute)
	return YES;
}		/*end expansion_redistribute*/


/*
*			expand_redist_cur():
*
*	This routine continually bisects the bonds in the curve c until
*	they have (scaled) length less than
*	spacing = max(space, 2*MIN_SC_SEP(fr->interf)),
*	where space is equal to the front spacing factor divided by
*	(1 + kbar*hypot(hx,hy)) and kbar is the average of the curvature
*	at the start and end of the bond.
*	(If there are no roundoff errors,  2^N - 1 (for some N)
*	equally spaced points are added to each old bond.)
*	The resulting new bonds will have (scaled) lengths >= 0.5*spacing.
*
*	Bonds are deleted only when the scaled bond length is <
*	MIN_SC_SEP(fr->interf).
*
*	After expansion, with one exception, all bonds on the curve will 
*	have (scaled) lengths between MIN_SC_SEP(fr->interf) and "spacing".
*	The exception is that a very short curve (total scaled length
*	< MIN_SC_SEP(fr->interf)) will end up as a single very short bond.
*	(A WARNING is written in this case.)
*
*/

/*ARGSUSED*/
EXPORT boolean expand_redist_cur(
	Front		*fr,
	CURVE		*c)
{
	BOND		*b, *bprev;
	RECT_GRID	*gr = fr->rect_grid;
	double		space, spacing;
	double		*h = gr->h;
	double		distance;	/* scaled bond lengths on curve */
	double		coords[MAXD];	/* coordinates of points added */
	int		i, dim = gr->dim;
	int		 pts_added;	/* number of points added to a bond */
	double		min_space;

	DEBUG_ENTER(expand_redist_cur)
	if (DEBUG)
	{
	    (void) printf("Before adding points in expand_redist_cur()\n");
	    print_curve(c);
	}

	    /* skip passive boundary curves */

	if (wave_type(c) == PASSIVE_BOUNDARY)
	    goto Exit;

	space = Front_spacing(fr,
	    	    (wave_type(c) >= FIRST_VECTOR_PHYSICS_WAVE_TYPE) ?
	    		VECTOR_WAVE : GENERAL_WAVE);
	min_space = 0.2*space;	/* set to 1/5 of the maximun spacing */

	    /* guarantee that no bonds shorter than MIN_SC_SEP(fr->interf)
	    *  will be added
	    */

	if (space < 2.0*MIN_SC_SEP(fr->interf)) 
	{
	    (void) printf("WARNING in expand_redist_cur(), "
	                  "desired spacing (%g) too small, "
	                  "using 2.0*MIN_SC_SEP(fr->interf)\n",space);
	    space = 2.0*MIN_SC_SEP(fr->interf);
	}

	    /*  loop over bonds in the curve */ 

	b = c->first;
	bprev = b->prev;
	(void) redistribution_spacing(NULL,NULL,0.0,NULL);
	while (b != NULL) 
	{
	    distance = scaled_bond_length(b,h,dim);
	    if (DEBUG) 
	    {
	    	print_bond(b);
	    	(void) printf("distance %g, space %g\n",distance,space);
	    }

	    	/* delete very short bonds */
	
	    /*if (distance < MIN_SC_SEP(fr->interf))  */
	    if (distance < min_space)
	    {
	    	if (c->num_points <= 3) 
	    	    goto Exit;

	    	if (b == c->last) 
	    	{
	    	    b = b->prev;
	    	    bprev = b->prev;
	    	}
	    	if (DEBUG)
	    	    (void) printf("deleting point (x,y) = (%g,%g)\n",
	    			  Coords(b->end)[0],Coords(b->end)[1]);
	    	(void) delete_start_of_bond(b->next,c);
	    	continue;
	    }

	    	/* Divide long bond into n equally spaced bonds */

	    /*Reason for commenting: mean curvature is not reliable. */
	    /*spacing = redistribution_spacing(b,c,space,fr); */
	    /*if (distance > spacing) */
	    if (distance > space)
	    {
	    	double t;

	    	/*pts_added = (int) (distance/spacing); */
	    	pts_added = (int) (distance/space);
	    	while (pts_added > 0)
	    	{
	    	    t = 1.0/((double)(pts_added-- + 1));
	    	    for (i = 0; i < dim; ++i)
	    		coords[i] = (1.0 - t)*Coords(b->start)[i]
	    				 + t *Coords(b->end)[i];
	    	    if (insert_point_in_bond(Point(coords),b,c) !=
			FUNCTION_SUCCEEDED)
	    	    {
	    	        screen("ERROR in expand_redist_cur(), "
	    	               "insert_point_in_bond failed\n");
	    	        clean_up(ERROR);
	    	    }
	    	    b = b->next;
	    	}
	    }
	    bprev = b;
	    b = b->next;
	}

	if (bprev != c->last) 
	{
	    (void) printf("WARNING in expand_redist_cur(), "
	                  "loop over bonds incomplete\n");
	    DEBUG_LEAVE(expand_redist_cur)
	    return NO;
	}

Exit:
	if (DEBUG)
	{
	    (void) printf("After adding points in expand_redist_cur()\n");
	    print_curve(c);
	}
	DEBUG_LEAVE(expand_redist_cur)
	return YES;
}		/*end expand_redist_cur*/

LOCAL	double	redistribution_spacing(
	BOND	*b,
	CURVE	*c,
	double	space,
	Front	*fr)
{
	static	double	kstart;
	static	double	kend = -1.0;
	double	kappa;
	double	hx, hy;

	if (b == NULL || c == NULL || fr == NULL)
	{
	    kend = -1.0;
	    return -1.0;
	}
	hx = fr->rect_grid->h[0];
	hy = fr->rect_grid->h[1];
	kstart = (kend >= 0.0) ? kend :
		fabs(mean_curvature_at_point(b->start,
				     Hyper_surf_element(b),Hyper_surf(c),fr));

	kend = fabs(mean_curvature_at_point(b->end,
				     Hyper_surf_element(b),Hyper_surf(c),fr));

	kappa = 0.5*(kstart+kend)*hypot(hx,hy);
	return space/(1.0 + kappa);
}		/*end redistribution_spacing*/


EXPORT boolean full_redistribute(
	Front		*fr,
	boolean		*force_redist)
{
	CURVE	**c;
	boolean	status = YES;
	boolean	force;
	int	redist_nodes;

	DEBUG_ENTER(full_redistribute)
	if (DEBUG)
	{
	    (void) printf("Unredistributed interface\n");
	    (void) printf("*force_redist = %s\n",y_or_n(*force_redist));
	    print_interface(fr->interf);
	}

	    /* Check on redistribution conditions */

	force = *force_redist;
	*force_redist = NO; /*set force_redist flag back to NO*/
	if (force)
	    redist_nodes = YES;
	else if (Redistribution_count(fr) < 0)
	    redist_nodes = NO;
	else
	    redist_nodes = redist_needed(fr,GENERAL_NODE);

	    /* Redistribute rect boundary curves */

	if (Use_rect_boundary_redistribution(fr))
	    rect_boundary_redistribute(fr->interf,fr->rect_grid,fr->step,fr);

	    /* Redistribute vector curves */

	for (c = fr->interf->curves; *c ; ++c) 
	{
	    if (wave_type(*c) == MOVABLE_BODY_BOUNDARY)
		continue;
	    if (wave_type(*c) == ELASTIC_BOUNDARY)
		continue;
	    if (perform_redistribution(*c,fr,force))
	    {
		switch (redistribution_direction(*c))
		{
		case FORWARD_REDISTRIBUTION:
	            status = Forward_curve_redistribute(fr,*c,status);
		    if (status)
		    {
		        redistributed(*c) = YES;
			redistribution_direction(*c) = BACKWARD_REDISTRIBUTION;
		    }
		    break;
		case BACKWARD_REDISTRIBUTION:
		    status = Backward_curve_redistribute(fr,*c,status);
		    if (status)
		    {
		        redistributed(*c) = YES;
			redistribution_direction(*c) = FORWARD_REDISTRIBUTION;
		    }
		    break;
		}
	    }
	}
	Redistribution_count(fr)++;

		/* Redistribute non-uni_array curves */

	if (redist_nodes)
	    status = Node_redistribute(fr,status);

	for (c = fr->interf->curves; *c ; ++c) 
	    if (redistributed(*c))
	        Interface_redistributed(fr) = YES;

	if (debugging("redistribute"))
	{
	    if (Interface_redistributed(fr))
	    {
	    	(void) printf("Redistributed interface\n");
	    	print_interface(fr->interf);
	    }
	    else
	    	(void) printf("Interface left unredistributed\n");
	}
	if (debugging("ck_b_len"))
	{
	    BOND *bb;
	    CURVE *cc;
	    double len;

	    (void) printf("Checking bond lengths after redistribution\n");
	    (void) next_bond(fr->interf,NULL,NULL);
	    while (next_bond(fr->interf,&bb,&cc))
	    {
	    	print_bond(bb);
	    	len = separation(bb->start,bb->end,fr->rect_grid->dim);
	    	(void) printf("Computed length = %g, "
	    	              "Computed length - bond_length = %g\n\n",
		              len,len - bond_length(bb));
	    }
	}
	DEBUG_LEAVE(full_redistribute)
	return status;
}		/*end full_redistribute*/

EXPORT boolean full_inc_redist_cur(
	Front		*fr,
	CURVE		*c,
	boolean		status)
{
	double		space, spacing;
	double		cos_big_angle;
	RECT_GRID	*gr = fr->rect_grid;
	BOND		*b;
	BOND		*next_delete;	/* bond whose starting vertex is to be
					 * deleted next. This variable allows
					 * insertions to be done before 
					 * deletions, ie out of the order set 
					 * by order of points along curve */
	double		*h = gr->h;
	double		distance;   /* Distance between pts of fr */
	double		offset;	    /* Next newfr pt at offset from fr pt */
	double		slopex;	    /* Unit uni_array, joining pair of fr pts */
	double		slopey;
	double		cos_angle;  /* sin, cos of exterior angle */
	double		sin_angle;
	double		scaled_length;  /* scaling for mismatched h */
	double		p[MAXD];	/* Loop starts from p */
	double		coords[MAXD];
	int		i, dim = gr->dim;

	DEBUG_ENTER(full_inc_redist_cur)

		/* Conditionally skip boundary curves */
	if (wave_type(c) == PASSIVE_BOUNDARY)
	{
	    DEBUG_LEAVE(full_inc_redist_cur)
	    return status;
	}

	if (wave_type(c) >= FIRST_VECTOR_PHYSICS_WAVE_TYPE)
	{
	    space = Front_spacing(fr,VECTOR_WAVE);
	    cos_big_angle = Cosine_big_angle(fr,VECTOR_WAVE);
	}
	else
	{
	    space = Front_spacing(fr,GENERAL_WAVE);
	    cos_big_angle = Cosine_big_angle(fr,GENERAL_WAVE);
	}

	    /* Skip short non-boundary curves with few points */

	if (!is_bdry_like_curve(c) && c->num_points <= 5) 
	{
	    scaled_length = 0.;
	    for (b = c->first; b; b = b->next)
	    	scaled_length += scaled_bond_length(b,h,dim);
	    if (scaled_length < 1.5 * space * (c->num_points -1))
	    {
	    	DEBUG_LEAVE(full_inc_redist_cur)
	    	return status;
	    }
	}

	    /* Set initial parameters for curve */

	offset = 0.5*space;
	for (i = 0; i < dim; ++i)
	    p[i] = Coords(c->first->start)[i];
	(void) redistribution_spacing(NULL,NULL,0.0,NULL);
	b = c->first;
	next_delete = NULL;

	    /* Process the next bond */

Loop:

	if (b == NULL) 
	{
	    status = NO;
	    (void) printf("WARNING in full_inc_redist_cur(), NULL BOND\n");
	    DEBUG_LEAVE(full_inc_redist_cur)
	    return status;
	}

	distance = _scaled_separation(Coords(b->end),p,h,dim);
	spacing = redistribution_spacing(b,c,space,fr);


	    /* Delete small bonds */

	if (b != c->last) 
	{
	    double	next_scaled_length;

	    if (distance < 0.1 * spacing)
	    {
	    	if (next_delete != NULL)
	    	    (void) delete_start_of_bond(next_delete,c);
		next_delete = b->next;
		b = b->next;
		goto Loop;
	    }
	    next_scaled_length = scaled_bond_length(b->next,h,dim);
	    if (next_scaled_length < 0.1 * spacing)
	    {
	    	if (next_delete != NULL)
	    	    (void) delete_start_of_bond(next_delete,c);
		if (next_delete != b->next)
		    (void) delete_start_of_bond(b->next,c);
		if (next_delete == b)
		    b = b->prev;
		next_delete = NULL;
		goto Loop;
	    }
	    big_angle(b,c,b->next,c,&cos_angle,&sin_angle,gr);
	}

	    /* Main loop for inserting points in a single bond */

	slopex = (Coords(b->end)[0] - p[0]) / distance;
	slopey = (Coords(b->end)[1] - p[1]) / distance;

	for ( ; offset <= distance; offset += spacing,b = b->next) 
	{
	    coords[0] = p[0] + slopex*offset;
	    coords[1] = p[1] + slopey*offset;
	    if (insert_point_in_bond(Point(coords),b,c) !=
		FUNCTION_SUCCEEDED)
	    {
		status = NO;
	    	screen("ERROR in full_inc_redist_cur(), "
	    	       "insert_point_in_bond failed\n");
	    	clean_up(ERROR);
		return status;
	    }
	}

	for (i = 0; i < dim; ++i)
	    p[i] = Coords(b->end)[i];
	offset -= distance;
	

		/* End of Curve */
	if (b == c->last) 
	{
	    scaled_length = scaled_bond_length(b,h,dim);

	    	/* Remove Short or Long Segment */

	    if (scaled_length <= 0.1 * spacing)
	    {
	    	(void) delete_start_of_bond(b,c);
	    	if (next_delete != NULL && next_delete != b)
	    	    (void) delete_start_of_bond(next_delete,c);
	    }
	    else if (next_delete != NULL)
	    	(void) delete_start_of_bond(next_delete,c);

	    b = c->last;
	    scaled_length = scaled_bond_length(b,h,dim);
	    if (scaled_length >= 0.5 * spacing)
	    {
	    	coords[0] = 0.5 * (Coords(b->start)[0] + Coords(b->end)[0]);
		coords[1] = 0.5 * (Coords(b->start)[1] + Coords(b->end)[1]);
		if (insert_point_in_bond(Point(coords),b,c) !=
		    FUNCTION_SUCCEEDED)
		{
		    status = NO;
		    screen("ERROR in full_inc_redist_cur(), "
		           "insert_point_in_bond failed\n");
		    clean_up(ERROR);
		    return status;
		}
	    }
	    DEBUG_LEAVE(full_inc_redist_cur)
	    return status;
	}

	/* Delete original vertex at end of b provided ... */

	scaled_length = scaled_bond_length(b,h,dim);

	if (
	    /* Initial segment of redist. b->next will be small */

	    (offset <= 0.1 * spacing)

	    /* b, ie final segment of redist. b, is small */

	    || (scaled_length <= 0.1 * spacing)

	    /* Exterior angle at vertex at end of b is small */

	    || (cos_angle >= cos_big_angle)
	) 
	{
	    if (next_delete != NULL)
	    	(void) delete_start_of_bond(next_delete,c);
	    next_delete = b->next;
	    b = b->next;
	}

	/* Else increment b; truncate if sharp bend at vertex */

	else 
	{

	    offset = scaled_length;

	    if (cos_angle <= -0.3) 
	    {
	    	coords[0] = 0.9*Coords(b->end)[0] + 0.1*Coords(b->start)[0];
		coords[1] = 0.9*Coords(b->end)[1] + 0.1*Coords(b->start)[1];
		if (insert_point_in_bond(Point(coords),b,c) !=
		    FUNCTION_SUCCEEDED)
		{
		    status = NO;
		    screen("ERROR in full_inc_redist_cur(), "
		           "insert_point_in_bond failed\n");
		    clean_up(ERROR);
		    return status;
		}
		b = b->next;
		(void) delete_start_of_bond(b->next,c);
		coords[0] = 0.1*Coords(b->end)[0] + 0.9*p[0];
		coords[1] = 0.1*Coords(b->end)[1] + 0.9*p[1];
		if (insert_point_in_bond(Point(coords),b,c) !=
		    FUNCTION_SUCCEEDED)
		{
		    status = NO;
		    screen("ERROR in full_inc_redist_cur(), "
		           "insert_point_in_bond failed\n");
		    clean_up(ERROR);
		    return status;
		}
	    }
	    b = b->next;
	}

	goto Loop;
}		/*end full_inc_redist_cur*/


EXPORT boolean full_dec_redist_cur(
	Front		*fr,
	CURVE		*c,
	boolean		status)
{
	invert_curve(c);
	status = full_inc_redist_cur(fr,c,status);
	invert_curve(c);
	return status;
}		/*end full_dec_redist_cur*/

EXPORT	boolean backward_equi_curve_redistribute(
	Front		*fr,
	CURVE		*c,
	boolean		status)
{
	invert_curve(c);
	status = equi_curve_redistribute(fr,c,status);
	invert_curve(c);
	return status;
}		/*end backward_equi_curve_redistribute*/

EXPORT	boolean equi_curve_redistribute(
	Front		*fr,
	CURVE		*c,
	boolean		status)
{
	BOND		*b;
	double		fr_space;
	RECT_GRID	*rgr = fr->rect_grid;
	double		*h = rgr->h;
	double		c_len;
	int		dim = rgr->dim;
	int		nbds;

	DEBUG_ENTER(equi_curve_redistribute)
		/* Conditionally skip boundary curves */

	if (dim == 2 && wave_type(c) == PASSIVE_BOUNDARY && (c->num_points > 2))
	{
	    DEBUG_LEAVE(equi_curve_redistribute)
	    return status;
	}
	if (is_subdomain_boundary(Hyper_surf(c)))
	{
	    DEBUG_LEAVE(equi_curve_redistribute)
	    return YES;
	}

	if (dim == 2)
	    fr_space = Front_spacing(fr,
			 (wave_type(c) >= FIRST_VECTOR_PHYSICS_WAVE_TYPE) ?
				VECTOR_WAVE : GENERAL_WAVE);
	else
	    fr_space = Front_spacing(fr,GENERAL_WAVE);

	if (debugging("high_order_redist"))
	{
	    if (dim == 2 && (wave_type(c) == FIRST_PHYSICS_WAVE_TYPE ||
		wave_type(c) == GROWING_BODY_BOUNDARY))
	    	printf("Entering equi_curve_redistribute()"
		       " for redistribution\n");
	}
	if (DEBUG)
	{
	    (void) printf("curve - ");
	    print_curve(c);
	}


	    /* Compute curve_length */

	c_len = 0.0;
	for (b = c->first;  b;  b = b->next)
	{
	    c_len += scaled_bond_length(b,h,dim);
	}

	    /* For short, non-boundary curves with few points - each */
	    /* such curve is equi-distributed, however the number   */
	    /* of points currently on the curve remains unchanged   */

	nbds = c->num_points - 1;
	if (DEBUG)
	    (void) printf("nbds %d\n",nbds);
	if (dim == 2 && (wave_type(c) >= FIRST_PHYSICS_WAVE_TYPE  ||
	     wave_type(c) == GROWING_BODY_BOUNDARY) && 
	    is_short_curve(c,POSITIVE_ORIENTATION,rgr,1.5))
	{
	    (void) expand_redist_cur(fr,c);
	    DEBUG_LEAVE(equi_curve_redistribute)
	    return status;
	}
	else if (dim == 2 && (!is_bdry_like_curve(c)) && (nbds <= 4))
	{
	    if (c_len < 1.5 * fr_space * nbds)
	    {
		if (DEBUG)
		    (void) printf("c_len %g < tol %g",c_len,1.5*fr_space*nbds);
		if (nbds > 1)
		{
		    if (DEBUG)
			(void) printf("nbds > 1\n");
		    equi_redist_curve_seg(c,c->first,c->last,nbds,
					  c_len,fr_space,rgr);
		    if (DEBUG)
		    {
		    	(void) printf("After equi_redist_curve_seg:\n");
		    	(void) printf("curve - ");
			print_curve(c);
		    }
		}
		DEBUG_LEAVE(equi_curve_redistribute)
		return status;
	    }
	}

	    /* Pass 1, delete very short bonds */

	curve_delete_very_short_bonds(c);
	if (DEBUG)
	{
	    (void) printf("After delete_very_short_bonds:\ncurve - ");
	    print_curve(c);
	}

	    /* Can't redistribute single bond, short_curves */

	if ((c->first == c->last) && (c_len < fr_space))
	{
	    if (DEBUG)
		(void) printf("single_bond, short curve\n");
	    DEBUG_LEAVE(equi_curve_redistribute)
	    return status;
	}

	b = c->first;
        while (b)
        {
            BOND *be,*bs = b;
            c_len = 0.0;
            for (;  b;  b = b->next)
            {
                c_len += scaled_bond_length(b,h,dim);
                be = b;
		if (cross_rect_grid_bdry(b,rgr)) break;
            }
	    b = be->next;
            equi_redist_curve_seg(c,bs,be,-1,c_len,fr_space,rgr);
        }
	if (DEBUG)
	{
	    (void) printf("After equi_redist_curve_seg:\ncurve - ");
	    print_curve(c);
	}
	DEBUG_LEAVE(equi_curve_redistribute)
	return status;
}		/*end equi_curve_redistribute*/


/*
*			attach_sink_nodes():
*
*	Attaches sink nodes to interface when interface gets sufficiently
*	close. Note this cannot be done in point_propagate routines as
*	it modifies the interface.
*/

LOCAL	boolean attach_sink_nodes(
	Front		*fr)
{
	CURVE		**cc;
	INTERFACE	*intfc = fr->interf;
	NODE		**nn, *m, *n, **sink_node_list;
	BOND		*b;
	double		min_sc_sep = MIN_SC_SEP(fr->interf);
	double		*h = computational_grid(intfc)->h;
	int		dim = intfc->dim;
	int		flag;
	int		i, j;

	flag = 0;

	for (nn = intfc->nodes, i = 0;  *nn;  ++nn)
	{
	    if (node_type(*nn) == SINK_NODE) ++i;
	}
	if (i == 0)
	    return YES;

	uni_array(&sink_node_list,i,sizeof(NODE *));

	for (nn = intfc->nodes, i = 0;  *nn;  ++nn)
	{
	    if (node_type(*nn) == SINK_NODE)
	    {
	    	sink_node_list[i] = *nn;
	    	++i;
	    }
	}

tmprst:
	for (cc = intfc->curves;  cc && *cc;  ++cc)
	{
	    for (b = (*cc)->first;  b != (*cc)->last;  b = b->next)
	    {
		for (j = 0;  j < i;  ++j)
		{
		    m = sink_node_list[j];
		    if (scaled_separation(m->posn,b->end,h,dim) <= min_sc_sep)
		    {
		       (void) attach_curve_to_node(*cc,b->end,b,m);
		       node_type(m) = SINK_NODE;
		       flag = 1;
		       goto tmprst;
		    }
		}
	    }

			/* Check for breakthrough at closed nodes */

	    if (!is_closed_node((*cc)->start)) continue;
	    for (j = 0;  j < i;  ++j)
	    {
		m = sink_node_list[j];
		n = (*cc)->start;
		if (scaled_separation(m->posn,n->posn,h,dim) <= min_sc_sep)
		{
		    merge_and_delete_nodes(m,n);
		    node_type(m) = SINK_NODE;
		    flag = 1;
		    goto tmprst;
		}
	    }
	}
			/* eliminate redundant CLOSED_NODES */
	if(flag == 1)
	{
	    intfc_delete_very_short_bonds(fr);
	    if (!join_curves_at_closed_nodes(intfc))
	    {
	    	(void) printf("WARNING in attach_sink_nodes(), "
	    	              "can't join closed curves\n");
	    	return NO;
	    }
	}
	free(sink_node_list);
	return YES;
}		/*end attach_sink_nodes*/

EXPORT int join_curves_at_closed_nodes(
	INTERFACE	*intfc)
{
	NODE		**nn;
	CURVE		*c1, *c2;
	boolean		sav_interp;

	sav_interp = interpolate_intfc_states(intfc);
	interpolate_intfc_states(intfc) = YES;
	for(nn = intfc->nodes; *nn; ++nn)
	{
	    if((node_type(*nn) == CLOSED_NODE) &&
	    	 ((*nn)->in_curves[0] != (*nn)->out_curves[0]))
	    {
	    	c1 = (*nn)->in_curves[0];
	    	c2 = (*nn)->out_curves[0];
	    	if (join_curves(c1,c2,negative_component(c1),
	    		        positive_component(c1),NULL) == NULL)
	    	{
		    interpolate_intfc_states(intfc) = sav_interp;
	    	    return NO;
	    	}
	    	(void) delete_node(*nn);
	    } 
	}
	interpolate_intfc_states(intfc) = sav_interp;
	return YES;
}		/*end join_curves_at_closed_nodes*/

LOCAL	boolean	outside_reflecting_bdry(
	FT_POINT 	  *p,
	INTERFACE *intfc)
{
	RECT_GRID *gr = computational_grid(intfc);
	int i,dim = gr->dim;

	for (i = 0; i < dim; ++i)
	{
	    if (rect_boundary_type(intfc,i,0) == REFLECTION_BOUNDARY &&
		Coords(p)[i] < gr->L[i])
		return YES;
	    if (rect_boundary_type(intfc,i,1) == REFLECTION_BOUNDARY &&
		Coords(p)[i] > gr->U[i])
		return YES;
	}
	return NO;
}	/* end outside_reflecting_bdry */

LOCAL	boolean	vector_wave_interaction(
	CROSS		*cross)
{
	boolean		status = NO;
	CROSS		*cr;

	for (cr = cross; cr != NULL; cr = cr->next)
	{
	    if (is_scalar_vector_cross(cr) || is_vector_vector_cross(cr))
	    {
	        status = YES;
	        break;
	    }
	}
        return pp_max_status(status);
}		/*end vector_wave_interaction*/


LOCAL	boolean too_many_tangled_points(
	CROSS		*cross,
	double		*h,
	int		dim)
{
	CROSS	  *cr, *cr1;
	int	  ncc, mncc;
	static const int MAX_TANGLED_POINTS = 2;
	static const double MIN_CROSS_SEP = 3.0; /*TOLERANCE*/
	boolean	  status;

	mncc = 0;

	for (cr = cross; cr; cr = cr->next)
	{
	    ncc = 1;
	    for (cr1 = cr->next; cr1; cr1 = cr1->next)
	    {
	        if (scaled_separation(cr->p,cr1->p,h,dim) < MIN_CROSS_SEP &&
		     topologically_close_crosses(cr,cr1))
		    ++ncc;
	    }
	    if (ncc > mncc) mncc = ncc;
	}
	status = NO;
	if (mncc > MAX_TANGLED_POINTS)
	{
	    (void) printf("Too many (%d > %d) tangled points in a small area\n",
			  mncc,MAX_TANGLED_POINTS);
	    if (DEBUG) print_cross_list(cross);
	    status = YES;
	}

	return pp_max_status(status);
}		/*end too_many_tangled_points*/

LOCAL boolean topologically_close_crosses(
	CROSS *cr1,
	CROSS *cr2)
{
	int num_bonds;
	BOND *b;
	if (cr1->c1 == cr2->c1)
	{
	    num_bonds = 0;
	    for (b = cr1->b1; b != NULL; b = b->prev, ++num_bonds)
		if (b == cr2->b1 && num_bonds < 3) 
		    return YES;
	    num_bonds = 0;
	    for (b = cr1->b1; b != NULL; b = b->next, ++num_bonds)
		if (b == cr2->b1 && num_bonds < 3) 
		    return YES;
	}
	if (cr1->c1 == cr2->c2)
	{
	    num_bonds = 0;
	    for (b = cr1->b1; b != NULL; b = b->prev, ++num_bonds)
		if (b == cr2->b2 && num_bonds < 3) 
		    return YES;
	    num_bonds = 0;
	    for (b = cr1->b1; b != NULL; b = b->next, ++num_bonds)
		if (b == cr2->b2 && num_bonds < 3) 
		    return YES;
	}
	if (cr1->c2 == cr2->c1)
	{
	    num_bonds = 0;
	    for (b = cr1->b2; b != NULL; b = b->prev, ++num_bonds)
		if (b == cr2->b1 && num_bonds < 3) 
		    return YES;
	    num_bonds = 0;
	    for (b = cr1->b2; b != NULL; b = b->next, ++num_bonds)
		if (b == cr2->b1 && num_bonds < 3) 
		    return YES;
	}
	if (cr1->c2 == cr2->c2)
	{
	    num_bonds = 0;
	    for (b = cr1->b2; b != NULL; b = b->prev, ++num_bonds)
		if (b == cr2->b2 && num_bonds < 3) 
		    return YES;
	    num_bonds = 0;
	    for (b = cr1->b2; b != NULL; b = b->next, ++num_bonds)
		if (b == cr2->b2 && num_bonds < 3) 
		    return YES;
	}
	return NO;
}	/* end topologically_close_crosses */

LOCAL	boolean elastic_tangle(
	CROSS		*cross)
{
	CROSS *cr;
	boolean	  status;

	status = NO;
	for (cr = cross; cr; cr = cr->next)
	{
	    if (wave_type(cr->c1) == ELASTIC_BOUNDARY &&
	        wave_type(cr->c2) == ELASTIC_BOUNDARY)
	        status = YES;
	}

	return pp_max_status(status);
}		/*end elastic_tangle*/
