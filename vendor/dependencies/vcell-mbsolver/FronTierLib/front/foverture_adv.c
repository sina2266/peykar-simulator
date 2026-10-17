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
*                               foverture_adv.c:
*
*       Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*
*       this file includes functions set_patch_front, set_patch_topo_grid,
*                       and set_patch_comput_grid
*/

#define DEBUG_STRING    "foverture_adv"

#include <front/fdecs.h>

#if defined(USE_OVERTURE)

/*
*			normal_advance_front2d():
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

LOCAL   int    second_node_propagate2d(double,double*,Front*,Front**,POINTER);  
LOCAL   char   *get_list_file_name(char*,const char*,const char*,size_t*); 
LOCAL   void   gview_plot_box2d(FILE*,double*,double*,double*);  
LOCAL   void   gview_plot_grid2d(FILE*,RECT_GRID*,double*); 
LOCAL   void   gview_plot_curve2d(FILE*,CURVE*,double*);  
LOCAL   char   *set_ppfname(char*,const char*,size_t*); 

EXPORT int normal_advance_front2d(
	double    dt,
	double    *dt_frac,
	Front    *front,
	Front    **newfront,
	POINTER  wave)
{
	CURVE      *oldc,*newc;
	CURVE	   **c;
	NODE 	   *oldn,*newn;
	RPROBLEM   *rp,*rp1;
	int        status, node_stat;
        NODE_FLAG  flag;  
	const char *fname = "normal_advance_front2d()";

	debug_print("front","Entered %s(step %d time %g dt %g)\n",fname,
	        	        front->step,front->time,dt);
	debug_front("old_front","into advance front",front);

	*newfront = copy_front(front);
	Interface_redistributed(*newfront) = NO;

        if(front->interf->nodes == NULL)
        {
            boolean       sav_copy;
            INTERFACE  *sav_intfc;  
            sav_intfc = current_interface();
            sav_copy = copy_intfc_states();
            set_size_of_intfc_state(size_of_state(front->interf));
            set_copy_intfc_states(YES);
            (*newfront)->interf = copy_interface(front->interf);
            set_current_interface(sav_intfc);
            set_copy_intfc_states(sav_copy);
            return return_advance_front(front,newfront,GOOD_STEP,fname);  
        }

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
            /* 
            If USE_OVERTURE, can not syncronize_time_step at here 
	    (*newfront)->interf = pp_copy_interface(front->interf);
            */ 
	    (*newfront)->interf = copy_interface(front->interf);
	    reset_hs_flags_on_intfc((*newfront)->interf);
	    status = ((*newfront)->interf != NULL) ? GOOD_STEP : ERROR_IN_STEP;
	    set_copy_intfc_states(YES);
	}
        if (status != GOOD_STEP)
        {
            (void) printf("ERROR in normal_advance_front2d(), "
                          "unable to copy interface\n");
            print_storage("after init_new_front","ADV_storage");
            clean_up(ERROR);
        }
	print_storage("after init_new_front","ADV_storage");
	stop_clock("init_new_front");

	        /* Set Default Propagation Limits */

	set_propagation_limits(front,*newfront);

	        /* Propagate the Curves */

	if (front->curve_propagate != NULL) 
	{
	    start_clock("curve_propagate");
	    if (debugging("front"))
	    	(void) printf("Loop over Curves\n");
	    for (c = front->interf->curves; c && *c; c++)
	    {
	        oldc = *c;
	        if (((newc = correspond_curve(oldc)) != NULL) &&
	    	     (correspond_curve(newc) != NULL))
	        {
	    	    if (debugging("propagate"))
	                (void) printf("\t\tpropagating curve %lu\n",
		                      curve_number(oldc));
		    curve_propagate(front,wave,oldc,newc,dt);
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
                if(DEBUG)
                {
                    /* 
                    printf("IN normal_advance_front2d\n");  
                    printf("node propagate\n");
                    print_node(oldn);
                    print_node(newn); 
                    printf("oldnode is virtual fixed = %s\n",
                       is_virtual_fixed_node(oldn) == YES ?
                       "YES" : "NO"); 
                    printf("newnode is virtual fixed = %s\n",
                       is_virtual_fixed_node(newn) == YES ?
                       "YES" : "NO"); 
                    printf("End of print new and old nodes\n");  
                    */  
                } 
	        status = (newn != NULL) ?
	            (*front->node_propagate)(front,wave,oldn,newn,&rp,
	        			     dt,dt_frac,flag,NULL) : GOOD_NODE;
	        if (is_bad_status(status) &&
	          (point_in_buffer(Coords(oldn->posn),front->rect_grid) == YES))
	        {
	            (void) printf("WARNING in normal_advance_front2d(), "
	                          "node_propagation returns ");
                    print_node_status("WARNING in normal_advance_front2d(), "
                                      "node_propagation returns ",status,"\n"); 
                    /* 
	            print_node_status(status);
                    */  
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
	            (void) printf("WARNING in normal_advance_front2d(), ");
	            (void) printf("node_propagate failed, ");
                    print_node_status("WARNING in normal_advance_front2d(), "
                                      "node_propagate failed with status ",
                                      status,"\n");
	            print_node(oldn);
	            if (debugging("CROSS_PAST"))
	            {
	                (void) printf("Cross past curve case\n"
	                              "dt_frac = %g\n",*dt_frac);
	                (void) printf("Reducing time step\n");
	            }
	            *dt_frac *= TIME_STEP_REDUCTION_FACTOR(front->interf);
	            free_rp_list(&rp);
	            status = MODIFY_TIME_STEP;
	            goto sync_prop_stat1;
	        case MODIFY_TIME_STEP_NODE:
	            (void) printf("WARNING in normal_advance_front2d(), "
	                          "node_propagate returns "
	                          "MODIFY_TIME_STEP_NODE\n");
	            free_rp_list(&rp);
	            status = MODIFY_TIME_STEP;
	            goto sync_prop_stat1;
	        case REPEAT_TIME_STEP_NODE:
	            (void) printf("WARNING in normal_advance_front2d(), "
	                          "node_propagate returns "
	                          "REPEAT_TIME_STEP_NODE\n");
	            free_rp_list(&rp);
	            status = REPEAT_TIME_STEP;
	            goto sync_prop_stat1;
	        case NO_CROSS_NODE:
	            (void) printf("WARNING in normal_advance_front2d(), ");
	            (void) printf("node_propagate failed, ");
                    print_node_status("WARNING in normal_advance_front2d(), "
                                      "node_propagate failed with status ",
                                      status,"\n"); 
	            print_node(oldn);
	            if (debugging("NO_CROSS"))
	            {
	                (void) printf("No cross case\n");
	                (void) printf("dt_frac = %g\n",*dt_frac);
	                (void) printf("Reducing time step\n");
	            }
	            *dt_frac *= TIME_STEP_REDUCTION_FACTOR(front->interf);
	            free_rp_list(&rp);
	            status = MODIFY_TIME_STEP;
	            goto sync_prop_stat1;
	        case ERROR_NODE:
	        default:
	            (void) printf("WARNING in normal_advance_front2d(), ");
	            (void) printf("node_propagate failed, ");
                    print_node_status("WARNING in normal_advance_front2d(), "
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
	            *dt_frac = Min_time_step_modification_factor(front);
	            free_rp_list(&rp);
	            status = MODIFY_TIME_STEP;
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
	        {
	            print_rproblem(rp1);
	        }
	        free_rp_list(&rp);
	        (void) printf("WARNING in normal_advance_front2d(), ");
	        (void) printf("CROSS code needed\n");
	        status = ERROR_IN_STEP;
	    }
	}
        /* 061003 closed, since the correspondence is reset. 
         * The second node prop. is done in tangential step now. 
        node_stat = second_node_propagate2d(dt,dt_frac,front,newfront,wave);
        if(GOOD_STEP != node_stat) 
        {
            (void) printf("WARNING in normal_advance_front2d(), "
                      "second node_propagation returns stat= %d", node_stat);
            clean_up(ERROR);  
        }  
        */
sync_prop_stat1:
	return return_advance_front(front,newfront,GOOD_STEP,fname);
}		/*end normal_advance_front2d*/


LOCAL int second_node_propagate2d(
	double    dt,
	double    *dt_frac,
	Front    *front,
	Front    **newfront,
	POINTER  wave)
{
	NODE 	   *tempn,*newn;
	INTERFACE  *tempintfc;
	boolean	   stat;
	int        status = GOOD_STEP;
	long       intfc_modified;
	const char *fname = "second_node_propagate2d()";

	debug_print("front","Entered %s(step %d time %g dt %g)\n",fname,
	        	        front->step,front->time,dt);

	/* Make Temp Interface for Second/Tangential Propagation */


	if ( front->interf == NULL || front->interf->nodes == NULL )
	    return GOOD_STEP;

	set_node_doubly_linked_list((*newfront)->interf);

	if (front->snd_node_propagate) 
	{
	    start_clock("snd_copy_interface");
	    print_storage("before snd_copy_interface","ADV_storage");
	    tempintfc = (*newfront)->interf;
	    set_size_of_intfc_state(size_of_state(tempintfc));
	    set_add_to_correspond_list(YES);
	    (*newfront)->interf = copy_interface(tempintfc);
	    copy_hypersurface_flags((*newfront)->interf);
	    interpolate_intfc_states((*newfront)->interf) = YES;
	    print_storage("after snd_copy_interface","ADV_storage");
	    stop_clock("snd_copy_interface");
	}
      
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

	if (status != GOOD_STEP)
	    return return_advance_front(front,newfront,status,fname);
	return return_advance_front(front,newfront,GOOD_STEP,fname);
}		/*end tangnt_advance_front2d*/

EXPORT int tangnt_advance_front2d(
	double    dt,
	double    *dt_frac,
	Front    *front,
	Front    **newfront,
	POINTER  wave)
{
	CURVE      *tempc,*newc;
	CURVE	   **c;
	NODE 	   *tempn,*newn;
	INTERFACE  *tempintfc;
	boolean	   stat;
        boolean       sav_copy;
	int        status = GOOD_STEP;
	long       intfc_modified;
	const char *fname = "tangnt_advance_front2d()";

	debug_print("front","Entered %s(step %d time %g dt %g)\n",fname,
	        	        front->step,front->time,dt);

	if ( front->interf == NULL || front->interf->nodes == NULL )
	    return return_advance_front(front,newfront,GOOD_STEP,fname);

        /* 050703 added, The interface regularization is performed here. */
        /* In advance_front2d(), these functions are called after
           normal_scatter_front, we call them here */ 
        delete_small_loops(*newfront); 

	stat = consistent_propagated_loop_orientations(dt,dt_frac,*newfront,wave);
	if(debugging("tangnt_advance_front2d"))
	    printf("In tangnt_advance_front2d() for patch %d\n", front->patch_number);
	if (stat == NO)
	{
	    (void) printf("WARNING in tangnt_advance_front2d(), "
		          "Inconsistent orientation of propagated loop "
	                  "detected after point and node propagations");
            if (pp_numnodes() > 1)
                (void) printf(" on processor %d\n",pp_mynode());
            else
                (void) printf("\n");
            status = MODIFY_TIME_STEP; 
            goto sync_prop_stat2;  
	}
        delete_exterior_curves(*newfront,front->interf);  
        intfc_delete_very_short_bonds(*newfront);
        stat = intfc_delete_fold_back_bonds(*newfront);
        if(stat == FUNCTION_FAILED)
        {
            (void) printf("WARNING in tangnt_advance_front2d(), "
                          "intfc_delete_fold_back_bonds() loop "
                          "detected error");
            if (pp_numnodes() > 1)
                (void) printf(" on processor %d\n",pp_mynode());
            else
                (void) printf("\n");
            status = MODIFY_TIME_STEP; 
            goto sync_prop_stat2;  
        } 
        delete_small_loops(*newfront); 
        /* 060303 added */ 
        measure_front(*newfront);
         
        /* Make Temp Interface for Second/Tangential Propagation */

        interpolate_intfc_states((*newfront)->interf) = YES;   
	set_node_doubly_linked_list((*newfront)->interf);
	if (front->snd_node_propagate || front->tan_curve_propagate) 
	{
	    start_clock("snd_copy_interface");
	    print_storage("before snd_copy_interface","ADV_storage");
	    tempintfc = (*newfront)->interf;
	    set_size_of_intfc_state(size_of_state(tempintfc));
	    set_add_to_correspond_list(YES);

            /* 060303, added copy_interface flag */
            sav_copy = copy_intfc_states();
            set_copy_intfc_states(YES);

	    (*newfront)->interf = copy_interface(tempintfc);
	    copy_hypersurface_flags((*newfront)->interf);
	    interpolate_intfc_states((*newfront)->interf) = YES;

            /* 060303, added copy_interface flag */
            set_copy_intfc_states(sav_copy);
	    print_storage("after snd_copy_interface","ADV_storage");
	    stop_clock("snd_copy_interface");
	}

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

	/* Tangential Sweep for States on the Curves */

	if (front->tan_curve_propagate) 
	{
	    start_clock("tan_curve_propagate");
	    if (debugging("front"))
	    	(void) printf("Second Loop over Curves\n");
	    for (c = tempintfc->curves; c && *c; c++)
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

        /* 060303, delete_phys_remn_on_bdry() added */
        /* Provide robustness for untangle algorithms */
        /*   delete remnants of scalar physical   */
        /*  curves sticking to NEUMANN boundaries */
        /* Add to delete_exterior_curves()? */

        if (delete_phys_remn_on_bdry(*newfront) == NO)
        {
            (void) printf("WARNING in tangnt_advance_front2d(), "
                          "delete_phys_remn_on_bdry() detected error\n");
            debug_front("ERROR_front","after error",*newfront);
            *dt_frac = Min_time_step_modification_factor(front);
            status = MODIFY_TIME_STEP;
            goto sync_prop_stat2;
        }
        debug_front("dspr_front",
                    "after 1st delete_phys_remn_on_bdry():",*newfront);

sync_prop_stat2:  
	if (status != GOOD_STEP)
	    return return_advance_front(front,newfront,status,fname);
	return return_advance_front(front,newfront,GOOD_STEP,fname);
}		/*end tangnt_advance_front2d*/


EXPORT int redist_advance_front2d(
	double    dt,
	double    *dt_frac,
	Front    *front,
	Front    **newfront,
	POINTER  wave)
{
	int        status, do_redist = YES;
	const char *fname = "redist_advance_front2d()";
	
	switch (redistribute(*newfront,do_redist,NO)) 
	{
	case	GOOD_REDISTRIBUTION:
	    status = GOOD_STEP;
	    break;
	
	case	UNABLE_TO_UNTANGLE:
	    (void) printf("WARNING in redist_advance_front2d(), "
	                  "redistribution of front failed\n"
	                  "Restarting redist_advance_front2d()\n");
	    (void) delete_interface((*newfront)->interf);
	    (*newfront)->interf = NULL;
	    do_redist = NO;
	    status = ERROR_IN_STEP;
	    break;

	case	MODIFY_TIME_STEP_REDISTRIBUTE:
	    (void) printf("WARNING in redist_advance_front2d(), "
	                  "redistribute returns\n"
	                  "\t\tMODIFY_TIME_STEP_REDISTRIBUTE\n");
	    status = MODIFY_TIME_STEP;
	    break;
		
	case	BAD_REDISTRIBUTION:
	default:
	    (void) printf("WARNING in redist_advance_front2d(), "
	                  "redistribution of front failed\n");
	    debug_front("ERROR_front","after error",*newfront);
	    *dt_frac = Min_time_step_modification_factor(front);
	    status = MODIFY_TIME_STEP;
	    break;
	}

	if (status != GOOD_STEP)
	    return return_advance_front(front,newfront,status,fname);

	Redistribution_count(front) = Redistribution_count(*newfront);
	(*newfront)->step = front->step + 1;
	(*newfront)->time = front->time + dt;
	debug_front("redist_front","after redistribution",*newfront);

	/* Delete non-boundary curves that lie  */
	/* fully on or exterior to the boundary */
	delete_exterior_curves(*newfront,front->interf);
    
        if(delete_phys_remn_on_bdry(*newfront) == NO)
        {
            (void) printf("WARNING in redist_advance_front2d(), "
                          "delete_phys_remn_on_bdry() detected error\n");
            debug_front("ERROR_front","after error",*newfront);
            status = MODIFY_TIME_STEP;
            return return_advance_front(front,newfront,status,fname);
        }  

	delete_small_loops(*newfront);

	measure_front(*newfront);

	test_for_mono_comp_curves((*newfront)->interf);

        /* 
        if((*newfront)->step > 5111) 
        {
            printf("LEAVING redist_advance_front2d, front[%d], level[%d]\n",
             (*newfront)->patch_number, (*newfront)->patch_level);
            for(CURVE **c = (*newfront)->interf->curves; c && *c; c++)
            {
                if(wave_type(*c) < FIRST_PHYSICS_WAVE_TYPE)
                    continue;
                print_curve(*c);
            }        
        }
        */ 
	return return_advance_front(front,newfront,GOOD_STEP,fname);
}		/*end redist_advance_front2d*/


EXPORT void geomview_amr_fronts_plot2d(
        const char    *dname,
        Front         **frs,
        int           num_patches,
        Wv_on_pc      **redistr_table,
        int           max_n_patch)
{
        FILE              *fp;
        int               i; 
        char              fmt[256];
        static const char *indent = "    ";
        static char       *fname = NULL, *ppfname = NULL;
        static size_t     fname_len = 0, ppfname_len = 0;
        INTERFACE         *intfc = frs[0]->interf; 
        INTERFACE         *tmpintfc;  
        CURVE             **c;
        INTERFACE         *sav_intfc;
        boolean              sav_copy; 
        double             **clrmap = NULL; 
        double             ccolor[4] = {0.0, 0.0, 0.0, 1.0};
        int               myid, numnodes;
        const char        *nstep;
        char              outname[256],outdir[256];

        myid = pp_mynode(); numnodes = pp_numnodes();
        sprintf(outdir,"%s/%s",dname,"geomintfc"); 
        ppfname = set_ppfname(ppfname,"intfc",&ppfname_len);
        nstep = right_flush(frs[0]->step,7);
        sprintf(outname,"%s.ts%s",ppfname,nstep);

        if (create_directory(dname,YES) == FUNCTION_FAILED)
        {
            (void) printf("WARNING in geomview_intfc_plot2d(), directory "
                          "%s doesn't exist and can't be created\n",dname);
            return;
        }
        if (create_directory(outdir,YES) == FUNCTION_FAILED)
        {
            (void) printf("WARNING in geomview_intfc_plot2d(), directory "
                         "%s doesn't exist and can't be created\n",outdir);
            return;
        }

        sav_intfc = current_interface();
        sav_copy = copy_intfc_states();
        set_size_of_intfc_state(size_of_state(intfc));
        set_copy_intfc_states(YES);
        tmpintfc = copy_interface(intfc);
  
        /* 
        clip_to_interior_region(tmpintfc,
            frs[0]->rect_grid->lbuf,frs[0]->rect_grid->ubuf); 
        */

        uni_array(&clrmap,6,sizeof(double*));  
        for(i = 0; i < 6; i++)
            uni_array(&clrmap[i],4,sizeof(double)); 

        i = 0;  
        clrmap[i][0] = 0.098; clrmap[i][1] = 0.647;
        clrmap[i][2] = 0.400; clrmap[i][3] = 1.000;
        i++;
        clrmap[i][0] = 0.898; clrmap[i][1] = 0.400;
        clrmap[i][2] = 0.000; clrmap[i][3] = 1.000;
        i++;
        clrmap[i][0] = 0.500; clrmap[i][1] = 1.000;
        clrmap[i][2] = 0.500; clrmap[i][3] = 1.000;
        i++;
        clrmap[i][0] = 1.000; clrmap[i][1] = 0.000;
        clrmap[i][2] = 1.000; clrmap[i][3] = 1.000;
        i++;
        clrmap[i][0] = 0.000; clrmap[i][1] = 0.800;
        clrmap[i][2] = 1.000; clrmap[i][3] = 1.000;
        i++;
        clrmap[i][0] = 0.250; clrmap[i][1] = 0.808;
        clrmap[i][2] = 0.098; clrmap[i][3] = 1.000;
        i++;

        
        fname = get_list_file_name(fname,outdir,outname,&fname_len); 
        if ((fp = fopen(fname,"w")) == NULL)
        {
            (void) printf("WARNING in gview_plot_intfc2d(), "
                           "can't open %s\n",fname);
            delete_interface(tmpintfc);  
            set_current_interface(sav_intfc);
            set_copy_intfc_states(sav_copy);
            return;
        }
        fprintf(fp,"{ LIST \n");
        /* beginning of writting Vect to file */
        for(c = tmpintfc->curves; c and *c;  c++)
            gview_plot_curve2d(fp,*c,ccolor); 

        for(int i = 0; i < num_patches; i++)
        {
           int use_clr;  
           /* 
           gview_plot_box2d(fp, frs[i]->rect_grid->L,
                frs[i]->rect_grid->U,clrmap[i%3]); 
           */ 
           if(NULL != redistr_table)
               use_clr = redistr_table[myid][i].pc_id % 6;
           else
               use_clr = 1;  
           gview_plot_grid2d(fp,frs[i]->rect_grid,clrmap[use_clr]);  
        }  

        /* end of LIST OBJ */
        fprintf(fp,"}\n");
        fclose(fp); 

        set_current_interface(sav_intfc);
        set_copy_intfc_states(sav_copy);
        delete_interface(tmpintfc);  

        for(int i = 0; i < 6; i++)
            free(clrmap[i]); 
        free(clrmap);  
}

LOCAL   char *set_ppfname(
        char       *ppfname,
        const char *fname,
        size_t     *ppfname_len)
{
        size_t len;

        if (pp_numnodes() > 1 )
        {
            const char   *nd;

            nd = right_flush(pp_mynode(),4);
            if (fname == NULL)
                fname = "";
            len = strlen(fname)+1+strlen(nd)+1;
            if (*ppfname_len < len)
            {
                *ppfname_len = len;
                if (ppfname != NULL)
                    free(ppfname);
                uni_array(&ppfname,*ppfname_len,CHAR);
            }
            (void) sprintf(ppfname,"%s.%s",fname,nd);
        }
        else
        {
            if (fname == NULL)
                fname = "";
            len = strlen(fname)+1;
            if (*ppfname_len < len)
            {
                *ppfname_len = len;
                if (ppfname != NULL)
                    free(ppfname);
                uni_array(&ppfname,*ppfname_len,CHAR);
            }
            (void) strcpy(ppfname,fname);
        }
        return ppfname;
}               /*end set_ppfname*/

LOCAL   char   *get_list_file_name(
        char       *fname,
        const char *dname,
        const char *name,
        size_t     *fname_len)
{
        size_t len;
        if (dname == NULL)
            dname = "";
        len = strlen(dname)+1+strlen(name)+6;
        if (*fname_len < len)
        {
            *fname_len = len;
            if (fname != NULL)
                free(fname);
            uni_array(&fname,*fname_len,CHAR);
        }
        if (strlen(dname) != 0)
            (void) sprintf(fname,"%s/%s.list",dname,name);
        else
            (void) sprintf(fname,"%s.list",name);
        return fname;
}

LOCAL void gview_plot_box2d(
	FILE      *fp,
        double     *L,
        double     *U,
        double     *color)
{
        fprintf(fp,"{ VECT\n");
        fprintf(fp,"%1d %1d %1d\n", 1, 5, 1);
        fprintf(fp,"%1d\n%1d\n", 5,1);
        (void) fprintf(fp,"%f %f %f \t",L[0],L[1],1.0);
        (void) fprintf(fp,"%f %f %f \t",U[0],L[1],1.0);
        (void) fprintf(fp,"%f %f %f \t",U[0],U[1],1.0);
        (void) fprintf(fp,"%f %f %f \t\n",L[0],U[1],1.0);
        (void) fprintf(fp,"%f %f %f \t",L[0],L[1],1.0);
        fprintf(fp,"%f %f %f %f \t\n", 
                  color[0], color[1], color[2], color[3]);
        fprintf(fp,"}\n");

}  

LOCAL void gview_plot_curve2d(
        FILE      *fp,
        CURVE     *c,
        double     *color)
{
            BOND *b;

            b = c->first;
            fprintf(fp,"{ \n");
            fprintf(fp,"appearance{*linewidth 2}\n");
	    fprintf(fp,"VECT\n");
            fprintf(fp,"%1d %1d %1d\n", 1, c->num_points, 1);
            fprintf(fp,"%1d\n%1d\n", c->num_points,1);

            while (b)
            {
                fprintf(fp,"%f %f %f \n",  Coords(b->start)[0],
                                Coords(b->start)[1] , 1.0);
                if (b == c->last)
                    break;
                b = b->next;
            }
            fprintf(fp,"%f %f %f \t\n",  Coords(b->end)[0],
                  Coords(b->end)[1] , 1.0);
            fprintf(fp,"%f %f %f %f \t\n",color[0],color[1],color[2],color[3]);
            fprintf(fp,"}\n");

}

LOCAL void gview_plot_grid2d(
	FILE      *fp,
        RECT_GRID *gr, 
        double     *color)
{
        int           i, j;
        int           imin[MAXD], imax[MAXD];
        int           *gmax = gr->gmax;
        int           n_x = 0, n_y = 0, num;
        double         *L = gr->L; 
        double         *U = gr->U;  
        int           icrds1[MAXD], icrds2[MAXD];
        double         crds1[MAXD+1], crds2[MAXD+1];
        double         dh[MAXD]; 

        for (i = 0; i < gr->dim; i++)
        {
            imin[i] = 0;
            imax[i] = gmax[i];
            dh[i] = gr->h[i];  
        }

        n_x = imax[0]-imin[0]+1;
        n_y = imax[1]-imin[1]+1;
        num = 2*(n_x + n_y);
 
        for(i=0; i<n_x; i++)
        {
            icrds1[0] = imin[0]+i;
            icrds1[1] = imin[1];
            icrds2[0] = imin[0]+i;
            icrds2[1] = imax[1];

            crds1[0] = L[0]+icrds1[0]*dh[0];
            crds1[1] = L[1];
            crds2[0] = L[0]+icrds2[0]*dh[0];
            crds2[1] = U[1];

            fprintf(fp,"{ VECT\n");
            fprintf(fp,"%1d %1d %1d\n", 1, 2, 1);
            fprintf(fp,"%1d %1d\n", 2, 1);
 
            fprintf(fp,"%f %f %f \t",crds1[0],crds1[1],1.0);
            fprintf(fp,"%f %f %f \t",crds2[0],crds2[1],1.0);
            fprintf(fp,"%f %f %f %f \t\n", 
                  color[0], color[1], color[2], color[3]);
            fprintf(fp,"}\n");
        }
 
        for(i=0; i<n_y; i++)
        {
            icrds1[0] = imin[0];
            icrds1[1] = imin[1]+i;
            icrds2[0] = imax[0];
            icrds2[1] = imin[1]+i;

            crds1[0] = L[0];
            crds1[1] = L[1]+icrds1[1]*dh[1];
            crds2[0] = U[0];
            crds2[1] = L[1]+icrds2[1]*dh[1];

            fprintf(fp,"{ VECT\n");
            fprintf(fp,"%1d %1d %1d\n", 1, 2, 1);
            fprintf(fp,"%1d %1d\n", 2, 1);
            fprintf(fp,"%f %f %f \t",crds1[0],crds1[1],1.0);
            fprintf(fp,"%f %f %f \t",crds2[0],crds2[1],1.0);
            fprintf(fp,"%f %f %f %f \t\n", 
                  color[0], color[1], color[2], color[3]);
            fprintf(fp,"}\n");
        }

}  
#endif  /* if defined(USE_OVERTURE) */

