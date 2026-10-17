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
*			fscat1d.c
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*/

#if defined(ONED)

#define DEBUG_STRING	"fscatter"
#include <front/fdecs.h>

	/* LOCAL Function Declarations */
LOCAL	boolean duplicate_point(FT_POINT*,INTERFACE*);

/*
*		f_intfc_communication1d():
*
*	This function drives the interface communication.  Communication is
*	only performed if necessary.  Non-reflecting boundaries are processed
*	first.  One coordinate direction is considered at a time, and the
*	interface merged before going to the other direction.  Also, a
*	consistency check is performed on the components, which should catch
*	any errors made in the interface reconstruction.
*/

EXPORT boolean f_intfc_communication1d(
	Front		*fr)
{
	COMPONENT	i_comp;
	INTERFACE	*intfc = fr->interf;
	INTERFACE	*sav_intfc;
	INTERFACE       *send[2], *receive[2];
	INTERFACE	*tmp_intfc;
	PP_GRID      	*pp_grid = fr->pp_grid;
	int          	*G = pp_grid->gmax;
	FT_POINT		*pt, **p;
	RECT_GRID	*gr = fr->rect_grid;
	int		myid=0, nb_id[2], src, nn;
	int          	me[MAXD], him[MAXD];
	int		i;
	double           T, cut, nor[3];
	boolean		status = FUNCTION_SUCCEEDED;
	boolean         	sav_copy;

	DEBUG_ENTER(f_intfc_communication1d)
	sav_intfc = current_interface();
	set_current_interface(intfc);

	if (DEBUG)
	{
	    (void) printf("Input interface\n");
	    print_interface(intfc);
	}

	i_comp = positive_component(intfc->points[0]);

	find_Cartesian_coordinates(myid, pp_grid, me);
	for (i = 0; i < 2; i++)
	    nb_id[i] = neighbor_id(him,me,0,i,pp_grid);

	/* Delete points in buffer regions */

	for (p = intfc->points; p && *p; ++p)
	{
	    if ((Coords(*p)[0] < gr->L[0]) || (Coords(*p)[0] > gr->U[0]) ||
		is_subdomain_boundary(Hyper_surf(*p)))
	    {
		delete_point(*p);
		if (intfc->points == NULL)
		    break;
		p = intfc->points-1;
	    }
	}

	if (DEBUG)
	{
	    (void) printf("Interface after deletion in buffer zones\n");
	    print_interface(intfc);
	}

	/* Set up interfaces to send */

	sav_copy = copy_intfc_states();
	for (i = 0; i < 2; ++i)
	{
	    switch (rect_boundary_type(intfc,0,i))
	    {
	    case SUBDOMAIN_BOUNDARY:
	    case REFLECTION_BOUNDARY:
	        if (DEBUG)
		    (void) printf("%s on side %d\n",
		                  wave_type_as_string(
				      rect_boundary_type(intfc,0,i),intfc),i);
	        set_size_of_intfc_state(size_of_state(intfc));
	        set_copy_intfc_states(YES);
	        send[i] = copy_interface(intfc);
	        if (send[i] == NULL)
	        {
		    screen("ERROR in f_intfc_communication1d(), "
		           "copy_interface() failed\n");
		    clean_up(ERROR);
	        }
		break;
	    default:
	        send[i] = NULL;
		break;
	    }
	}
	set_copy_intfc_states(sav_copy);

	/* Clip send interfaces */

	if (send[0] != NULL)
	{
	    set_current_interface(send[0]);
	    cut = gr->L[0] + (gr->L[0] - gr->VL[0]);
	    for (p = send[0]->points; p && *p; ++p)
	    {
	        if (Coords(*p)[0] <= gr->L[0] || Coords(*p)[0] > cut)
	        {
		    delete_point(*p);
		    if (send[0]->points == NULL)
		        break;
		    p = send[0]->points-1;
	        }
	    }
	    nor[0] = 1.0;  nor[1] = nor[2] = 0.0;
	    if (rect_boundary_type(intfc,0,0) == REFLECTION_BOUNDARY)
	        reflect_interface(send[0],gr->L,nor);
	    else if (me[0] == 0)
	    {
		RECT_GRID *sgr = computational_grid(send[0]);
		T = sgr->GU[0] - sgr->GL[0];
		sgr->L[0] += T;
		sgr->U[0] += T;
		set_rect_grid(sgr->L,sgr->U,sgr->GL,sgr->GU,sgr->lbuf,sgr->ubuf,
	                          sgr->gmax,sgr->dim,&sgr->Remap,sgr);
	    	for (p = send[0]->points; p && *p; ++p)
		{
		    Coords(*p)[0] += T;
		}
	    }
	}
	if (send[1] != NULL)
	{
	    set_current_interface(send[1]);
	    cut = gr->U[0] + (gr->U[0] - gr->VU[0]);
	    for (p = send[1]->points; p && *p; ++p)
	    {
	        if (Coords(*p)[0] < cut || Coords(*p)[0] > gr->U[0])
	        {
		    delete_point(*p);
		    if (send[1]->points == NULL)
		        break;
		    p = send[1]->points-1;
	        }
	    }
	    nor[0] = -1.0;  nor[1] = nor[2] = 0.0;
	    if (rect_boundary_type(intfc,0,1) == REFLECTION_BOUNDARY)
	        reflect_interface(send[1],gr->U,nor);
	    else if (me[0] == G[0]-1)
	    {
		RECT_GRID *sgr = computational_grid(send[1]);
		T = sgr->GU[0] - sgr->GL[0];
		sgr->L[0] -= T;
		sgr->U[0] -= T;
		set_rect_grid(sgr->L,sgr->U,sgr->GL,sgr->GU,sgr->lbuf,sgr->ubuf,
	                          sgr->gmax,sgr->dim,&sgr->Remap,sgr);
	    	for (p = send[1]->points; p && *p; ++p)
		{
		    Coords(*p)[0] -= T;
		}
	    }
	}

	/* Compress storage of the send interfaces */

	set_current_interface(intfc);

	myid = pp_mynode();
	nn = pp_numnodes();

	receive[0] = receive[1] = NULL;
	if (rect_boundary_type(intfc,0,0) == REFLECTION_BOUNDARY)
	    receive[0] = send[0];
	else if (rect_boundary_type(intfc,0,0) == SUBDOMAIN_BOUNDARY)
	{
	    if (nb_id[0] == myid)
	    	receive[0] = send[1];
	    else
	    	send_interface(send[0],nb_id[0]);
	}
	if (rect_boundary_type(intfc,0,1) == SUBDOMAIN_BOUNDARY)
	{
	    if (nb_id[1] != myid)
	    	receive[1] = receive_interface(nb_id[1]);
	}

	if (rect_boundary_type(intfc,0,1) == REFLECTION_BOUNDARY)
	    receive[1] = send[1];
	else if (rect_boundary_type(intfc,0,1) == SUBDOMAIN_BOUNDARY)
	{
	    if (nb_id[1] == myid)
	    	receive[1] = send[0];
	    else
	    	send_interface(send[1],nb_id[1]);
	}
	if (rect_boundary_type(intfc,0,0) == SUBDOMAIN_BOUNDARY)
	{
	    if (nb_id[0] != myid)
	    	receive[0] = receive_interface(nb_id[0]);
	}

	set_current_interface(intfc);
	set_copy_intfc_states(YES);
	for (i = 0; i < 2; ++i)
	{
	    if (receive[i] == NULL)
		continue;
	    for (p = receive[i]->points; p && *p; ++p)
	    {
		if (!duplicate_point(*p,intfc))
		    (void) copy_point(*p);
	    }
	}
	if (myid == G[0]-1 &&
	    rect_boundary_type(intfc,0,1) == SUBDOMAIN_BOUNDARY)
	{
	    for (i = 1; i < intfc->num_points; ++i)
	    {
	    	FT_POINT *pt0 = intfc->points[i-1];
	    	FT_POINT *pt1 = intfc->points[i];
	    	if (negative_component(pt1) != positive_component(pt0))
	    	{
	    	    set_equivalent_comps(positive_component(pt1),
				     negative_component(pt0),intfc);
	    	    positive_component(pt1) = negative_component(pt0);
	    	}
	    }
	}
	set_copy_intfc_states(sav_copy);
	for (i = 0; i < 2; ++i)
	{
	    if (send[i] != NULL)
	        (void) delete_interface(send[i]);
	    if ((receive[i] != send[i]) && (receive[i] != NULL))
		(void) delete_interface(receive[i]);
	}

	if (!set_boundary(intfc,gr,i_comp,grid_tolerance(gr)))
	{
	    screen("ERROR in f_intfc_communication1d(), "
		   "set_boundary() failed\n");
	    clean_up(ERROR);
	    return FUNCTION_FAILED;
	}
	pt = intfc->points[0];
	if (is_bdry(pt) && (wave_type(pt) == ERROR))
	{
	    wave_type(pt) = SUBDOMAIN_BOUNDARY;
	    if (size_of_state(intfc) != 0)
	    {
		obstacle_state(intfc,left_state(pt),size_of_state(intfc));
		obstacle_state(intfc,right_state(pt),size_of_state(intfc));
	    }
	}
	pt = intfc->points[intfc->num_points-1];
	if (is_bdry(pt) && (wave_type(pt) == ERROR))
	    wave_type(pt) = SUBDOMAIN_BOUNDARY;
	if (!make_point_comp_lists(intfc))
	{
	    screen("ERROR in f_intfc_communication1d(), "
		   "make_point_comp_lists failed\n");
	    clean_up(ERROR);
	    return FUNCTION_FAILED;
	}
	set_current_interface(sav_intfc);

	if (DEBUG)
	{
	    (void) printf("Final Interface\n");
	    print_interface(intfc);
	}

	DEBUG_LEAVE(f_intfc_communication1d)
	return status;
}		/*end f_intfc_communication1d*/

LOCAL	boolean duplicate_point(
	FT_POINT *p,
	INTERFACE *intfc)
{
	RECT_GRID *gr = computational_grid(intfc);
	FT_POINT **pp;
	for (pp = intfc->points; pp && *pp; ++pp)
	{
	    if (fabs(Coords(p)[0] - Coords(*pp)[0]) > grid_tolerance(gr))
		continue;
	    if (positive_component(p) == positive_component(*pp) &&
		negative_component(p) == negative_component(*pp))
		return YES;
	}
	return NO;
}	/* end duplicate_point */
#endif /* defined(ONED) */
