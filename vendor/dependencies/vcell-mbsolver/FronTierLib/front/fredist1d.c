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
*				fredist1d.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	The routines in this file untangle a interface.
*/

#if defined(ONED)

#include <front/fdecs.h>

/* LOCAL Function Prototypes */

EXPORT	int	redistribute1d(
	Front *fr)
{
	CROSS     *cross;
	INTERFACE *intfc = fr->interf;
	boolean      istatus;
	int       flag = NORMAL_ATTEMPT_TO_UNTANGLE;
	int       status;

	debug_print("redist1d","Entered redistribute1d()\n");
	istatus = intersections(intfc,&cross,YES);
	if (istatus == FUNCTION_FAILED)
	{
	    (void) printf("WARNING in redistribute1d(), "
	                  "intersections() failed\n");
	}
	if (pp_min_status(istatus) == FUNCTION_FAILED)
	{
	    if (istatus == FUNCTION_SUCCEEDED)
	    {
	        (void) printf("WARNING in redistribute1d(), "
	                      "intersections() failed on remote node\n");
	    }
	    if (debugging("redist1d"))
	    {
	        (void) printf("WARNING in redistribute1d(), "
		              "intersections() failed\n");
	    }
	    debug_print("redist1d","Left redistribute1d()\n");
	    return BAD_REDISTRIBUTION;
	}
	if (interface_is_tangled(cross) == NO)
	{
	    debug_print("redist1d","Left redistribute1d()\n");
	    return GOOD_REDISTRIBUTION;
	}

	(void) print_number_of_tangles("",intfc,cross);

	if (fr->fr_bdry_untangle)
	{
	    if (debugging("redist1d"))
	        (void) printf("Attempting to untangle boundary interactions\n");
	    status = (cross == NULL) ? CURVES_UNTANGLED :
				       (*fr->fr_bdry_untangle)(fr,&cross,
							       NULL,NULL,flag);
	    status = synchronize_untangle_status(status);
	    if (status != CURVES_UNTANGLED)
	    {
		(void) printf("WARNING in redistribute1d(), "
		              "unable to untangle boundary tangles\n");
	        debug_print("redist1d","Left redistribute1d()\n");
		return BAD_REDISTRIBUTION;
	    }
	}
	if (interface_is_tangled(cross) == NO)
	{
	    debug_print("redist1d","Left redistribute1d()\n");
	    return GOOD_REDISTRIBUTION;
	}
	if (fr->untangle_front)
	{
	    if (debugging("redist1d"))
	        (void) printf("Attempting to untangle interior interactions\n");
	    status = (cross==NULL) ? CURVES_UNTANGLED :
				     (*fr->untangle_front)(fr,&cross,flag);
	    status = synchronize_untangle_status(status);
	    switch (status)
	    {
	    case CURVES_UNTANGLED:
		break;
	    case MODIFY_TIME_STEP_TO_UNTANGLE:
		(void) printf("WARNING in redistributed1d, "
	    	              "fr->untangle_front returns \n"
	    	              "\t\tMODIFY_TIME_STEP_TO_UNTANGLE\n");
	        debug_print("redist1d","Left redistribute1d()\n");
	    	return MODIFY_TIME_STEP_REDISTRIBUTE;
	    default:
		(void) printf("WARNING in redistribute1d(), "
		              "unable to untangle interior tangles\n");
	        debug_print("redist1d","Left redistribute1d()\n");
		return BAD_REDISTRIBUTION;
	    }
	}
	if (interface_is_tangled(cross) == YES)
	{
	    (void) printf("WARNING in redistribute1d(), "
	                  "unable to untangle interface\n");
	    print_intersections(cross,intfc);
	    debug_print("redist1d","Left redistribute1d()\n");
	    return BAD_REDISTRIBUTION;
	}
	if (consistent_components1d(intfc) == NO)
	{
	    screen("ERROR in redistribute1d(), "
		   "inconsistent components\n");
	    print_interface(intfc);
	    clean_up(ERROR);
	    debug_print("redist1d","Left redistribute1d()\n");
	    return BAD_REDISTRIBUTION;
	}
	if (debugging("redist1d"))
	{
	    (void) printf("Untangled interface\n");
	    print_interface(fr->interf);
	}
	debug_print("redist1d","Left redistribute1d()\n");
	return GOOD_REDISTRIBUTION;
}		/*end redistribute1d*/
#endif /* defined(ONED) */
