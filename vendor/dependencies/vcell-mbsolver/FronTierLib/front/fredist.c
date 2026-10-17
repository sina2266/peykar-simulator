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
*				fredist.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*/

#if defined(TWOD) || defined(THREED)

#include <front/fdecs.h>		/* includes int.h, table.h */



/*
*			redistribute():
*
*	Controls high level dimensionally independent parts of
*	the redistribution of tracked waves.  See comments for
*	dimensionally specific parts for more information.
*
*	TODO:  At present this function is just a switch shell
*	implement a truly high level redistribution driver
*	to replace dimensionally dependent drivers.
*/

EXPORT int redistribute(
	Front		*fr,
	boolean		do_redist,
	boolean		restart_init)
{
	int		dim = fr->rect_grid->dim;
	int		status = BAD_REDISTRIBUTION;
	
	switch(dim)
	{
#if defined(ONED)
	case 1:
	    status = redistribute1d(fr);
	    break;
#endif /* defined(ONED) */
#if defined(TWOD)
	case 2:
	    status = redistribute2d(fr,do_redist,restart_init);
	    break;
#endif /* defined(TWOD) */
#if defined(THREED)
	case 3:
	    status = redistribute3d(fr,do_redist,restart_init);
	    /*In this case scatter_front fails,  */
	    /*all procs will have the same value for status. */
	    if(status == INCONSISTENT_RECONSTRUCTION)
	    	return status;
	    break;
#endif /* defined(THREED) */
	}


	if (fr->pp_grid && fr->pp_grid->nn > 1)
	{
	    long gs;

	    gs = (status == BAD_REDISTRIBUTION) ? 1 : 0;
#if defined(USE_OVERTURE)
#else /* if defined(USE_OVERTURE) */
            pp_global_lmax(&gs,1L);
#endif /* if defined(USE_OVERTURE) */

	    if (gs == 1)
		return BAD_REDISTRIBUTION;

	    gs = (status == MODIFY_TIME_STEP_REDISTRIBUTE) ? 1 : 0;
#if defined(USE_OVERTURE)
#else /* if defined(USE_OVERTURE) */
            pp_global_lmax(&gs,1L);
#endif /* if defined(USE_OVERTURE) */
	    if (gs == 1)
		return MODIFY_TIME_STEP_REDISTRIBUTE;
		
	    gs = (status == UNABLE_TO_UNTANGLE) ? 1 : 0;
#if defined(USE_OVERTURE)
#else /* if defined(USE_OVERTURE) */
            pp_global_lmax(&gs,1L);
#endif /* if defined(USE_OVERTURE) */

	    if (gs == 1)
		return UNABLE_TO_UNTANGLE; 

	    gs = (status == GOOD_REDISTRIBUTION) ? 0 : 1;
#if defined(USE_OVERTURE)
#else /* if defined(USE_OVERTURE) */
	    pp_global_lmax(&gs,1L);
#endif /* if defined(USE_OVERTURE) */

	    if (gs == 1)
		return BAD_REDISTRIBUTION; 
	}
	return status;
}		/*end redistribute*/

EXPORT	boolean	f_perform_redistribution(
	HYPER_SURF *hs,
	Front      *front,
	boolean       force)
{
	boolean do_redist;

	debug_print("redistribute",
	      "Entered f_perform_redistribution(), force = %s\n",
	      y_or_n(force));
	if (Redistribution_count(front) < 0)
	    do_redist = NO;
	else if (omit_redistribution(hs))
	    do_redist = NO;
	else if (is_bdry_hs(hs) && (Use_rect_boundary_redistribution(front)))
	    do_redist = NO;
	else if (force || redistribute_hyper_surface(hs))
	{
	    if (debugging("redistribute"))
	        (void) printf("redistribute_hyper_surface(hs) = %s\n",
		              y_or_n(redistribute_hyper_surface(hs)));
	    do_redist = YES;
	}
	else
	{
	    if (wave_type(hs) >= FIRST_VECTOR_PHYSICS_WAVE_TYPE)
	        do_redist = redist_needed(front,VECTOR_WAVE);
	    else
	        do_redist = redist_needed(front,GENERAL_WAVE);

	}
	debug_print("redistribute",
	      "Left f_perform_redistribution(), do_redist = %s\n",
	      y_or_n(do_redist));
	return do_redist;
}		/*end f_perform_redistribution*/

EXPORT	boolean	redist_needed(
	Front *fr,
	int   wfam)
{
	boolean do_redist;

	if (Interface_redistributed(fr) == YES)
	    do_redist = NO;
	else if (Redistribution_count(fr) % 
		 Frequency_of_redistribution(fr,wfam))
	    do_redist = NO;
	else
	    do_redist = YES;
	return do_redist;
}		/*end redist_needed*/

EXPORT	boolean interface_is_tangled(
	CROSS	*cross)
{
  	boolean	status;

	status = (cross != NULL) ? YES : NO;

#if defined(USE_OVERTURE)
        return status;
#else
	return pp_max_status(status);
#endif /* if defined(USE_OVERTURE) */
}		/*end interface_is_tangled*/

EXPORT	void	Clear_redistribution_parameters(
	Front *fr)
{
 	boolean (*fnd)(Front*,boolean);
	void (*fid)(INIT_DATA*,Front*);

	fid = Init_redistribution_function(fr);
	fnd = Node_redistribute_function(fr);
	zero_scalar(&Redistribution_info(fr),sizeof(REDIST_CONTROL));
	Init_redistribution_function(fr) = fid;
	Node_redistribute_function(fr) = fnd;				\
}		/*end Clear_redistribution_parameters*/

#endif /* defined(TWOD) || defined(THREED) */
