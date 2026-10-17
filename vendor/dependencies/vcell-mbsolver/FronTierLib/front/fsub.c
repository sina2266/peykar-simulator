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
*				fsub.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Contains the miscellaneous subroutines
*
*		angle_from_c1_c2_at_common_node()
*		measure_front()
*
*		assign_front_interface()
*
*		f_max_front_time_step()
*
*		robust_quad_roots_in_interval()
*/


#include <front/fdecs.h>		/* includes int.h, table.h */


	/* LOCAL Function Declarations */
LOCAL	Front*	f_copy_front(Front*);
#if defined(USE_OVERTURE)
LOCAL   Front*  f_deep_copy_front(Front*);
LOCAL   void    f_deep_free_front(Front*);
#endif /* if defined(USE_OVERTURE) */

LOCAL	void	f_copy_MaxFrontSpeed(MAX_FRONT_SPEED*,MAX_FRONT_SPEED*,Front*);
LOCAL	void	f_destroy_MaxFrontSpeed(Front*);
LOCAL	void	f_free_front(Front*);
LOCAL	void	f_include_max_front_speed_info(MAX_FRONT_SPEED*,Front*);

LOCAL   double   compute_curvature2d(double*,double*,double,int);
LOCAL	void	delete_passive_curves(INTERFACE*);
LOCAL 	void 	f_identify_physical_node(NODE*);

/*Moved from testfront.c by Eric--------------Aug. 06,2005*/
LOCAL void      set_boundary_node_type(NODE*,INTERFACE*);
LOCAL   SURFACE *prompt_make_NWGrid_surface(INTERFACE*);

/*----------------------End by Eric-----------------------*/
/*TMP*/
LOCAL void    no_continuation_along_curve(int,ORIENTATION,int,CURVE*,double*,
                                FT_POINT**,BOND**,Front*);
LOCAL void    point_at_distance_along_curve(FT_POINT*,ORIENTATION,double,int,
				FT_POINT**,BOND**,double*,Front*);


/*
*			f_principal_tangent():
*
*	A simple function for computing a particular tangent vector to
*	a hypersurface.  In two space dimensions this will simply rotate
*	the normal vector by 90 degrees.  In 3D it flips (with a change of sign)
*	the largest and smallest components of the normal uni_array.
*/

/*ARGSUSED*/
EXPORT	void	f_principal_tangent(
	FT_POINT			*p,
	HYPER_SURF_ELEMENT	*hse,
	HYPER_SURF		*hs,
	double			*nor,
	double			*vdir)
{
	int	i, imax, imin;
	int	dim = hs->interface->dim;
	double	len;

	for (i = 0; i < dim; ++i)
	    vdir[i] = 0.0;
	if (dim == 1)
	    return;

	imax = 0;	imin = dim-1;
	for (i = 1; i < dim; ++i)
	{
	    if (nor[i] > nor[imax])
	    	imax = i;
	    if (nor[i] < nor[imin])
	    	imin = i;
	}
	vdir[imax] = -nor[imin];
	vdir[imin] =  nor[imax];
	len = mag_vector(vdir,dim);
	for (i = 0; i < dim; ++i)
	    vdir[i] /= len;
}		/*end f_principal_tangent*/


/*
*                       f_deep_copy_front():
*
*       Basic default function for copying a front structure.
*       Allocates storage for the new front and copies the
*       argument into the new structure.
*/
#if defined(USE_OVERTURE)
LOCAL   Front *f_deep_copy_front(
        Front           *fr)
{
        Front           *newfr;

        scalar(&newfr,sizeof(Front));
        copy_into_front(newfr,fr);
        scalar(&(newfr->rect_grid), sizeof(RECT_GRID));
        *(newfr->rect_grid) = *(fr->rect_grid);
        scalar(&(newfr->pd_flag),sizeof(Patch_bdry_flag));
        return newfr;
}               /*end f_deep_copy_front*/
#endif /* if defined(USE_OVERTURE)  */


/*
*			f_copy_front():
*
*	Basic default function for copying a front structure.
*	Allocates storage for the new front and copies the
*	argument into the new structure.
*/

LOCAL	Front *f_copy_front(
	Front		*fr)
{
	Front		*newfr;

	scalar(&newfr,sizeof(Front));
	copy_into_front(newfr,fr);
	return newfr;
}		/*end f_copy_front*/

/*
*			f_copy_into_front():
*
*	Copies fr into newfr.  Assumes newfr is already allocated.
*/

EXPORT	void f_copy_into_front(
	Front		*newfr,
	Front		*fr)
{
	*newfr = *fr;
}		/*end f_copy_into_front*/

/*
*			f_free_front():
*
*	Basic front destructor.  Deletes fr->interf and then frees the
*	corresponding front.  Should only be used on fronts that were
*	created by f_copy_front.
*/

LOCAL	void	f_free_front(
	Front		*fr)
{
	if (fr == NULL)
	    return;
	if (fr->interf != NULL)
	    (void) delete_interface(fr->interf);
	free(fr);
}		/*end f_free_front*/


/*
*                       f_deep_free_front():
*
*       Basic front destructor.  Deletes fr->interf and then frees the
*       corresponding front.  Should only be used on fronts that were
*       created by f_copy_front.
*/
#if defined(USE_OVERTURE)
LOCAL   void    f_deep_free_front(
        Front           *fr)
{
        if (fr == NULL)
            return;
        if (fr->interf != NULL)
            (void) delete_interface(fr->interf);
        free(fr->rect_grid);
        free(fr->pd_flag);
        free(fr);
}               /*end f_deep_free_front*/
#endif /* if defined(USE_OVERTURE)  */

/*
*			f_set_default_front_parameters():
*
*	Sets default values for most fields in the front structure.  Fields
*	not initialized in this function are indicated by comments below.  As
*	of this writing, the order below reflects the declaration of the Front
*	structure in fdecs.h as much as possible.
*	Note: this function assumes that fr->rect_grid->dim and fr->sizest
*	have already been set.
*/

EXPORT	void	f_set_default_front_parameters(
	INIT_DATA	*init,
	Front		*fr)
{
	int		 dim = fr->rect_grid->dim;

	F_USER_INTERFACE *fuh = f_user_hook(dim);

	/*
	 * Remark: set_advance_front uses the value of Tracking_algorithm(fr)
	 * to select the appropriate advance_front function (in 3D)
	 * so the default value of this flag must be set prior to calling
	 * set_advance_front.
	 */
	Tracking_algorithm(fr) = (init != NULL) ?
	    tracking_algorithm(init) : NO_DYNAMIC_TRACKING;
	set_advance_front(init,fr);

	fr->_free_front =				f_free_front;
	fr->_copy_front =				f_copy_front;
	fr->_copy_into_front =				f_copy_into_front;

	fr->_print_Front_structure =			f_print_Front_structure;
	fr->_fprint_front =				f_fprint_front;
	fr->_read_print_front =				f_read_print_front;

#if defined(USE_OVERTURE)
        fr->_deep_free_front =                          f_deep_free_front;
        fr->_deep_copy_front =                          f_deep_copy_front;
#endif /* if defined(USE_OVERTURE) */

	/*	fr->sizest (assumed already set) */
	fr->_state_interpolator = linear_state_interpolator;
	fr->_tri_state_interpolator = linear_tri_state_interpolator;
	fr->transform_state =				NULL;

	fr->_is_correspondence_possible =		NULL; 

	/*	fr->Redist */
	Node_redistribute_function(fr) =		NULL;

	fr->max_front_time_step =			f_max_front_time_step;
	/*	fr->_MaxFrontSpeed  (see below) */
	/*	fr->Tstep */

	/*	fr->dt */
	/*	fr->dt_frac */
	/*	fr->time */
	/*	fr->step */

	fr->hyperbolic =				FULL_STATES;
	/*	fr->npts_tan_sten */
	fr->init_topology_of_new_interface =		NULL;
	f_wave_capture(fr) =				NULL;
	fr->_init_propagate =				NULL;
	fr->curve_propagate =				NULL; 
	fr->node_propagate =				NULL; 
	/*	fr->_point_propagate */
	fr->bond_propagate =				NULL;
	fr->snd_node_propagate =			NULL; 
	fr->tan_curve_propagate =			NULL; 
	/*	fr->_npt_tang_solver */
	/*	fr->_one_side_npt_tang_solver */
	fr->impose_bc =					NULL;

	fr->_untrack_point = NULL;
	fr->_untrack_curve = NULL;
	fr->_untrack_surface = NULL;

	switch (dim)
	{
	case 1:
	    fr->fr_bdry_untangle =		NULL;
	    fr->_check_delete_redundant_node =	NULL;
	    fr->_replace_unphys_loop = 		NULL;
	    fr->_untrack_point =		f_untrack_point;
	    break;
	case 2:
	    if (init != NULL)
	        set_tangent_operator(tangent_method(init),dim);
	    fr->_check_delete_redundant_node =	NULL;
	    fr->fr_bdry_untangle =		f_boundary_untangle;
	    fr->_replace_unphys_loop = 		f_replace_unphys_loop;
	    Rect_boundary_redistribute_function(fr) = rect_bdry_redist2d;
	    fr->_untrack_curve =		f_untrack_curve;
	    fr->curve_propagate =		f_curve_propagate2d; 
	    fr->node_propagate = 		f_node_propagate;
	    fr->untangle_front =		scalar_unravel; 
	    fr->grid_based_untangle =		f_grid_based_untangle;
	    fr->elastic_untangle =		f_elastic_untangle;
	    fr->intfc_propagate =		NULL; 
	    break;
	case 3:
	    if (init != NULL)
	    {
	        set_normal3d_method(normal3d_method(init),dim);
		
	    	/* Who did this and for what?
		if (fr->interf != NULL)
		    f_user_interface(fr->interf) = *fuh;
	    	*/
	    }
	    if (debugging("no_tan_prop"))
	        fr->_tan_point_propagate =		NULL;
	    else
	        fr->_tan_point_propagate =		f_tan_point_propagate;
	    fr->_check_delete_redundant_node =	NULL;
	    fr->fr_bdry_untangle =		NULL;
	    fr->_replace_unphys_loop = 		NULL;
	    fr->_reconstruct_front_at_grid_crossing =
	        	rebuild_intfc_at_crossings3d;
	    fr->_repair_front_at_grid_crossing =
                        repair_intfc_at_crossings3d;
	    fr->_untrack_surface =		f_untrack_surface;
	    fr->surface_propagate =		f_surface_propagate; 
	    fr->_principal_tangent = 		f_principal_tangent;
	    break;
	}

	fr->fr_vec_bdry_untangle =			NULL; 
	fr->B_node_bifurcation =			NULL; 
	fr->twodrproblem =				f_2drproblem; 
	fr->identify_physical_node =		f_identify_physical_node; 
	fr->init_2drproblem =				f_init_2drproblem; 
	fr->phys_split_bdry_cross =			NULL; 
	fr->phys_set_node_types =			NULL; 
	fr->parallel_refl_vec_wave =			NULL;

	fr->tan_curve_propagate =			NULL; 
	fr->_fgraph_front_states =			NULL; 
	fr->_fgraph_curve_states =			NULL; 
	fr->_fprint_header_for_graph_curve_states =	NULL; 
	fr->_find_i_to_prop_dir =			NULL;

	fr->neumann_bdry_state =			NULL;
	fr->_find_i_to_prop_dir = 			NULL;
	fr->is_nzn_bdry =				NULL;

	fr->_alloc_state = fuh->_alloc_state;
	fr->_clear_state = fuh->_clear_state;
	fr->_obstacle_state = fuh->_obstacle_state;

	MaxFrontSpeed(fr) = (init != NULL) ? InitialMaxFrontSpeed(init) : NULL;

	/*      fr->nfloats */
	fr->print_state =				NULL;
	fr->_fgraph_front_states =			NULL;
	fr->_fprint_header_for_graph_curve_states = 	NULL;
	fr->_fgraph_curve_states =			NULL;
	fr->mass_consv_diagn_driver =			NULL;

	fr->head_fsr =					AddToFsrList(NULL);

	/*	fr->FDIVIDER */
	/*	fr->interf */

}		/*end f_set_default_front_parameters*/

EXPORT	MAX_FRONT_SPEED	*f_alloc_MaxFrontSpeed(
	MAX_FRONT_SPEED	*mxsp,
	INTERFACE	*intfc,
	size_t		sizest)
{
	static MAX_FRONT_SPEED_OPERATORS DefaultOperators = {
	    f_set_max_front_speed,
	    f_include_max_front_speed_info,
	    f_initialize_max_front_speed,
	    f_fprint_max_front_speed_info,
	    f_read_print_max_front_speed_info,
	    f_copy_MaxFrontSpeed,
	    f_destroy_MaxFrontSpeed
	};
	int	i, j;
	byte    **buf;

	if (mxsp != NULL)
	    return mxsp;

	scalar(&mxsp,sizeof(MAX_FRONT_SPEED));

	mxsp->_sizest = sizest;
	if (mxsp->_sizest > 0)
	{
	    bi_array(&buf,MAXD+1,1,mxsp->_sizest);
	    mxsp->_mxspst = (Locstate*)buf;
	}
	bi_array(&mxsp->_coords,MAXD+1,MAXD,FLOAT);
	for (i = 0; i <= MAXD; ++i)
	{
	    if (mxsp->_sizest > 0)
	    	clear_state(intfc,mxsp->_mxspst[i],mxsp->_sizest);
	    for (j = 0; j < MAXD; ++j)
	    	mxsp->_coords[i][j] = HUGE;
	}
	mxsp->operators = DefaultOperators;

	return mxsp;
}		/*end f_alloc_MaxFrontSpeed*/

LOCAL	void	f_copy_MaxFrontSpeed(
	MAX_FRONT_SPEED	*nmxsp,
	MAX_FRONT_SPEED	*omxsp,
	Front		*fr)
{
	int	i, j, dim = fr->rect_grid->dim;
	
	nmxsp = alloc_MaxFrontSpeed(nmxsp,fr->interf,omxsp->_sizest);
	nmxsp->operators = omxsp->operators;
	nmxsp->_sizest = omxsp->_sizest;
	for (i = 0; i < dim+1; ++i)
	{
	    nmxsp->_spfr[i] = omxsp->_spfr[i];
	    if (nmxsp->_sizest != 0)
	        ft_assign(nmxsp->_mxspst[i],omxsp->_mxspst[i],nmxsp->_sizest);
	    for (j = 0; j < dim; ++j)
	    	nmxsp->_coords[i][j] = omxsp->_coords[i][j];
	}
}		/*end f_copy_MaxFrontSpeed*/

LOCAL	void	f_destroy_MaxFrontSpeed(
	Front	*fr)
{
	free(MaxFrontSpeedState(fr));
	free(MaxFrontSpeedCoords(fr));
	free(MaxFrontSpeed(fr));
	MaxFrontSpeed(fr) = NULL;
}		/*end f_destroy_MaxFrontSpeed*/

/*
*
*			assign_interface_and_free_front():
*
*	Copies fr->interf into newfr->interf and then frees fr.
*/

EXPORT	void	assign_interface_and_free_front(
	Front		*newfr,
	Front		*fr)
{
	assign_front_interface(newfr,fr);
	fr->interf = NULL;
	free_front(fr);
}		/*end assign_interface_and_free_front*/

/*
*			assign_front_interface():
*
*/

EXPORT void assign_front_interface(
	Front		*left,
	Front		*right)
{
	if (left != NULL)
	    (void) delete_interface(left->interf);

	left->interf = right->interf;
}		/*end assign_front_interface*/


/*
*			f_max_front_time_step():
*
*	Sets max_dt to the maximum time step allowed by the
*	Courant-Friedrichs-Levy condition for the advancing front.
*/

EXPORT	double	f_max_front_time_step(
	Front		*fr,
	double		*coords)
{
	double		max_dt = HUGE;
	double		dt[MAXD+1];
	double		*spfr = Spfr(fr);
	double		*h = fr->rect_grid->h;
	double		hmin = HUGE;
	int		i, j, dim = fr->interf->dim;

	for (i = 0; i < dim; ++i)
	{
	    if (hmin > h[i]) hmin = h[i];
	    if (spfr[i] > 0.0)
	    {
	        dt[i] = h[i]/spfr[i];
	        if (max_dt > dt[i])
	        {
	            max_dt = dt[i];
	            for (j = 0; j < dim; ++j)
	                coords[j] = MaxFrontSpeedCoords(fr)[i][j];
	        }
	    }
	    else
	        dt[i] = HUGE;
	}
	if (spfr[dim] > 0.0)
	{
	    dt[dim] = hmin/spfr[dim];
	    if (max_dt > dt[dim])
	    {
	        max_dt = dt[dim];
	        for (j = 0; j < dim; ++j)
	    	    coords[j] = MaxFrontSpeedCoords(fr)[dim][j];
	    }
	}
	else
	    dt[dim] = HUGE;
	if (debugging("time_step"))
	{
	    (void) printf("In f_max_front_time_step()\n");
	    for (i = 0; i < dim; ++i)
	    {
	        (void) printf("front: spfr[%d] %g dt[%d]  %g dx[%d] %g\n",
	                      i,spfr[i],i,dt[i],i,h[i]);
	        (void) printf("front - max_dt = %g\n",max_dt);
	    }
	    (void) printf("front: spfr[%d] %g dt[%d]  %g\n",
	                  i,spfr[i],i,dt[i]);
	    (void) printf("front - max_dt = %g\n",max_dt);
	    print_general_vector("coords = ",coords,dim,"\n");
	}
	return max_dt;
}		/*end f_max_front_time_step*/



EXPORT	void set_default_tan_stencil(
	Tan_stencil	*stencil)
{
	int		i;

	for (i = 0; i < stencil->npts; ++i)
	{
	    stencil->hsstore[i] = NULL;
	    stencil->hsestore[i] = NULL;
	    stencil->tstore[i] = ERROR_FLOAT;
	}		
	stencil->curvature = 0.0;
}			/*end set_default_tan_stencil*/

/*
*			alloc_tan_stencil():
*
*/

EXPORT	Tan_stencil *alloc_tan_stencil(
	Front		*fr,
	int		nrad)
{
	byte		*store;
	int		npts = 2*nrad + 1;
	Tan_stencil	*sten;
	int		i;
	size_t		size;
	size_t		hs_offset, hse_offset;
	size_t		p_offset, t_offset, ls_offset, rs_offset;

	size = sizeof(Tan_stencil);
	hs_offset = (size%sizeof(HYPER_SURF*) != 0) ?
	            (size/sizeof(HYPER_SURF*) + 1)*sizeof(HYPER_SURF*) : size;
	size = hs_offset + npts*sizeof(HYPER_SURF*);
	hse_offset = (size%sizeof(HYPER_SURF_ELEMENT*) != 0) ?
	    (size/sizeof(HYPER_SURF_ELEMENT*) + 1)*sizeof(HYPER_SURF_ELEMENT*) :
	    size;
	size = hse_offset + npts*sizeof(HYPER_SURF_ELEMENT*);
	p_offset = (size%sizeof(FT_POINT*) != 0) ?
	    (size/sizeof(FT_POINT*) + 1)*sizeof(FT_POINT*) : size;
	size = p_offset + npts*sizeof(FT_POINT*);
	t_offset = (size%sizeof(double) != 0) ?
	    (size/sizeof(double) + 1)*sizeof(double) : size;
	size = t_offset + npts*sizeof(double);
	ls_offset = (size%sizeof(Locstate) != 0) ?
	    (size/sizeof(Locstate) + 1)*sizeof(Locstate) : size;
	size = ls_offset + 2*npts*sizeof(Locstate);
	rs_offset = ls_offset + npts*sizeof(Locstate);

	scalar(&store,size);
	sten = (Tan_stencil*)store;
	sten->hsstore = (HYPER_SURF**)(store + hs_offset);
	sten->hsestore = (HYPER_SURF_ELEMENT**)(store + hse_offset);
	sten->pstore = (FT_POINT**)(store + p_offset);
	sten->tstore = (double*)(store + t_offset);
	sten->leftststore = (Locstate*)(store + ls_offset);
	sten->rightststore = (Locstate*)(store + rs_offset);

	for (i = 0; i < npts; ++i)
	    sten->pstore[i] = Static_point(fr->interf);
	sten->npts = npts;
	sten->hs = sten->hsstore + nrad;
	sten->hse = sten->hsestore + nrad;
	sten->p = sten->pstore + nrad;
	sten->t = sten->tstore + nrad;
	sten->leftst = sten->leftststore + nrad;
	sten->rightst = sten->rightststore + nrad;
	for (i = 0; i < npts; ++i)
	{
	    sten->leftststore[i] = left_state(sten->pstore[i]);
	    sten->rightststore[i] = right_state(sten->pstore[i]);
	}
	return sten;
}			/*end alloc_tan_stencil*/

EXPORT	void measure_front(
	Front		*front)
{
	switch (front->rect_grid->dim)
	{
	case 1: /*TODO implement measure front*/
	    Front_length(front) = 0;
	    break;
	case 2:
	    {
	        BOND  *b;
	        CURVE *c;

	        /* Compute Length of Front */

	        Front_length(front) = 0.0;
	        (void) next_bond(front->interf,NULL,NULL);
	        while (next_bond(front->interf,&b,&c)) 
	        {
	    	    if (is_bdry(c))
			continue;
	    	    Front_length(front) += bond_length(b);
	        }
	    }
	    break;
	case 3: /*TODO implement measure front*/
	    Front_length(front) = ERROR_FLOAT;
	    break;
	}
}		/*end measure_front*/


EXPORT	int syncronize_time_step_status(
	int	 status,
	PP_GRID* pp_grid)
{
	long		max_status, min_status;

	if (pp_grid->nn == 1)
	    return status;

	min_status = status;
	pp_global_lmin(&min_status,1L);

	if (min_status == ERROR_IN_STEP)
	    return ERROR_IN_STEP;

	max_status = status;
	pp_global_lmax(&max_status,1L);

	if (max_status == MODIFY_TIME_STEP)
	    return MODIFY_TIME_STEP;

	return status;
}		/*end syncronize_time_step_status*/

EXPORT	void	f_set_max_front_speed(
	int		i,
	double		spd,
	Locstate	state,
	double		*coords,
	Front		*fr)
{
	if (fabs(spd) > Spfr(fr)[i])
	{
	    int	j, dim = fr->rect_grid->dim;
	    Spfr(fr)[i] = fabs(spd);
	    if (coords != NULL)
	    {
	    	for (j = 0; j < dim; ++j)
	    	    MaxFrontSpeedCoords(fr)[i][j] = coords[j];
	    }
	    if (state != NULL)
	    	ft_assign(MaxFrontSpeedState(fr)[i],state,fr->sizest);
	}
}		/*end f_set_max_front_speed*/

LOCAL	void	f_include_max_front_speed_info(
	MAX_FRONT_SPEED	*mxsp,
	Front		*fr)
{
	int i, dim = fr->rect_grid->dim;

	debug_print("time_step","Entered f_include_max_front_speed_info()\n");
	for (i = 0; i <= dim; ++i)
	{
	    if (fabs(mxsp->_spfr[i]) > Spfr(fr)[i])
	    {
	    	int	j;

	    	Spfr(fr)[i] = fabs(mxsp->_spfr[i]);
	    	for (j = 0; j < dim; ++j)
	    	    MaxFrontSpeedCoords(fr)[i][j] = mxsp->_coords[i][j];
	    	ft_assign(MaxFrontSpeedState(fr)[i],mxsp->_mxspst[i],fr->sizest);
	    }
	}
	debug_print("time_step","Left f_include_max_front_speed_info()\n");
}		/*end f_include_max_front_speed_info*/

EXPORT	void	f_initialize_max_front_speed(
	Front	*fr)
{
	MAX_FRONT_SPEED	*mxsp = MaxFrontSpeed(fr);
	int	i, j;

	debug_print("time_step","Entered f_initialize_max_front_speed()\n");
	for (i = 0; i <= MAXD; ++i)
	    mxsp->_spfr[i] = 0.0;
	for (i = 0; i <= MAXD; ++i)
	{
	    if (mxsp->_sizest)
	    	clear_state(fr->interf,mxsp->_mxspst[i],mxsp->_sizest);
	    for (j = 0; j < MAXD; ++j)
	    	mxsp->_coords[i][j] = HUGE;
	}
	if (debugging("time_step"))
	{
	    (void) printf("Spfr(fr) initialized to zero, ");
	    print_general_vector("Spfr(fr) = ",Spfr(fr),
	                         fr->rect_grid->dim+1,"\n");
	}
	debug_print("time_step","Left f_initialize_max_front_speed()\n");
}		/*end f_initialize_max_front_speed*/

/*#bjet2 */
LOCAL  boolean full_average = YES;
EXPORT  void   set_full_average(boolean y_or_n)
{
    full_average = y_or_n;
}

/*
*			f_average_points():
*
*	Extension of i_average_points() that updates the states at the
*	averaged points.
*/

EXPORT	FT_POINT *f_average_points(
	boolean               newpoint,
	FT_POINT		   *p1,
	HYPER_SURF_ELEMENT *hse1,
	HYPER_SURF	   *hs1,
	FT_POINT		   *p2,
	HYPER_SURF_ELEMENT *hse2,
	HYPER_SURF	   *hs2)
{
	INTERFACE *intfc = hs1->interface;
	Locstate  sl1, sl2, sr1, sr2;
	Locstate  sl, sr;
	FT_POINT     *pmid;
	double	  crds1[3];
	double	  crds2[3];
	size_t	  sizest = size_of_state(intfc);
	int	  i, dim = hs1->interface->dim;

	for (i = 0; i < dim; ++i)
	{
	    crds1[i] = Coords(p1)[i];
	    crds2[i] = Coords(p2)[i];
	}
	pmid = i_average_points(newpoint,p1,hse1,hs1,p2,hse2,hs2);

	if (sizest != 0)
	{
	  if (pmid != NULL)
	  {
	    slsr(p1,hse1,hs1,&sl1,&sr1);
	    slsr(p2,hse2,hs2,&sl2,&sr2);
	    sl = left_state(pmid);
	    sr = right_state(pmid);
	    bi_interpolate_intfc_states(intfc,0.5,0.5,crds1,sl1,crds2,sl2,sl);
	    bi_interpolate_intfc_states(intfc,0.5,0.5,crds1,sr1,crds2,sr2,sr);
	  }
	  else
	  {
	    static Locstate stmp = NULL;
	    if (stmp == NULL)
	        alloc_state(intfc,&stmp,sizest);

	    /*#bjet2 */
	    /*for 3d only apply the direct average to interior points */
	    if(full_average || ( !Boundary_point(p1) && !Boundary_point(p2) ))
	    {
	        slsr(p1,hse1,hs1,&sl1,&sr1);
	        slsr(p2,hse2,hs2,&sl2,&sr2);
	        bi_interpolate_intfc_states(intfc,0.5,0.5,crds1,sl1,crds2,sl2,stmp);
	        ft_assign(sl1,stmp,sizest);
	        ft_assign(sl2,stmp,sizest);
	        bi_interpolate_intfc_states(intfc,0.5,0.5,crds1,sr1,crds2,sr2,stmp);
	        ft_assign(sr1,stmp,sizest);
	        ft_assign(sr2,stmp,sizest);
	    }

	    if ((hs1 == hs2) && (hse1 == hse2))
	    {
	      /* Update all instances of states at the two points */
	      if (dim == 2)
	      {
		BOND  *b = Bond_of_hse(hse1); /* recall hse1 == hse2 */
		CURVE *c = Curve_of_hs(hs1);  /* recall hs1 == hs2   */
		if ( ( (b->prev == NULL) && (b->next == NULL) ) &&
		     ( ((p1 == b->start) && (p2 == b->end)) ||
		       ((p1 == b->end)   && (p2 == b->start)) ) )
		{
		  /* Do other curves share p1 and p2 as node positions? */
		  NODE  *ns, *ne;
		  CURVE **cc;
		  ns = c->start;
		  ne = c->end;
		  for (cc = intfc->curves; cc && *cc; ++cc)
		  {
		    if (*cc == c)
		      continue;
		    if (((*cc)->start == ns) && ((*cc)->end == ne))
		    {
	              slsr(p1,Hyper_surf_element((*cc)->first),
			      Hyper_surf(*cc),&sl1,&sr1);
	              slsr(p2,Hyper_surf_element((*cc)->last),
			      Hyper_surf(*cc),&sl2,&sr2);
	              bi_interpolate_intfc_states(intfc,0.5,0.5,crds1,sl1,
				                  crds2,sl2,stmp);
	              ft_assign(sl1,stmp,sizest);
	              ft_assign(sl2,stmp,sizest);
	              bi_interpolate_intfc_states(intfc,0.5,0.5,crds1,sr1,
				                  crds2,sr2,stmp);
	              ft_assign(sr1,stmp,sizest);
	              ft_assign(sr2,stmp,sizest);
		    }
		    if (((*cc)->end == ns) && ((*cc)->start == ne))
		    {
	              slsr(p1,Hyper_surf_element((*cc)->last),
			      Hyper_surf(*cc),&sl1,&sr1);
	              slsr(p2,Hyper_surf_element((*cc)->first),
			      Hyper_surf(*cc),&sl2,&sr2);
	              bi_interpolate_intfc_states(intfc,0.5,0.5,crds1,sl1,
				                  crds2,sl2,stmp);
	              ft_assign(sl1,stmp,sizest);
	              ft_assign(sl2,stmp,sizest);
	              bi_interpolate_intfc_states(intfc,0.5,0.5,crds1,sr1,
				                  crds2,sr2,stmp);
	              ft_assign(sr1,stmp,sizest);
	              ft_assign(sr2,stmp,sizest);
		    }
		  }
		}
	      }
	      if ((dim == 3) && Boundary_point(p1) && Boundary_point(p2))
	      {
		TRI     *tri = Tri_of_hse(hse1); /* recall hse1 == hse2 */
		SURFACE *s = Surface_of_hs(hs1); /* recall hs1 == hs2   */
		int     v1, v2, side;

		v1 = Vertex_of_point(tri,p1);
		v2 = Vertex_of_point(tri,p2);
		if ((v1 == ERROR) || (v2 == ERROR))
		{
		  screen("ERROR in f_average_points(), invalid vertex\n");
		  clean_up(ERROR);
		}
		side = (v2 == Next_m3(v1)) ? v1 : v2;
		if (is_side_bdry(tri,side))
		{
		  BOND *b = Bond_on_side(tri,side);
		  BOND_TRI **btris;
		  for (btris = Btris(b); btris && *btris; ++btris)
		  {
		    if ((*btris)->surface != s) /*s already done*/
		    {
	              slsr(p1,Hyper_surf_element((*btris)->tri),
			      Hyper_surf((*btris)->surface),&sl1,&sr1);
	              slsr(p2,Hyper_surf_element((*btris)->tri),
			      Hyper_surf((*btris)->surface),&sl2,&sr2);
	              bi_interpolate_intfc_states(intfc,0.5,0.5,crds1,sl1,
				                  crds2,sl2,stmp);
	              ft_assign(sl1,stmp,sizest);
	              ft_assign(sl2,stmp,sizest);
	              bi_interpolate_intfc_states(intfc,0.5,0.5,crds1,sr1,
				                  crds2,sr2,stmp);
	              ft_assign(sr1,stmp,sizest);
	              ft_assign(sr2,stmp,sizest);
		    }
		  }
		}
	      }
	    }
	  }
	}
	return pmid;
}		/*end f_average_points*/

EXPORT	void	delete_passive_boundaries(
	INTERFACE	*intfc)
{

	DEBUG_ENTER(delete_passive_boundaries)
	switch (intfc->dim)
	{
	case 1:
	    {
	        FT_POINT **p;
	        for (p = intfc->points; p && *p; ++p)
	        {
		    if (is_passive_boundary(Hyper_surf(*p)))
	            {
		        delete_point(*p);
		        if (intfc->points == NULL)
		            break;
		        p = intfc->points-1;
	            }
	        }
	    }
	    break;
	case 2:
	    delete_passive_curves(intfc);
	    break;
	case 3:/*TODO implement delete_passive_boundaries*/
	    break;
	}
	DEBUG_LEAVE(delete_passive_boundaries)
}		/*end delete_passive_boundaries*/


LOCAL	void	delete_passive_curves(
	INTERFACE *intfc)
{
	CURVE	**delete_curves, **c;
	NODE	**delete_nodes, **join_nodes, **n;
	boolean    sav_intrp;

	DEBUG_ENTER(delete_passive_curves)
	sav_intrp = interpolate_intfc_states(intfc);
	interpolate_intfc_states(intfc) = YES;
	delete_curves = NULL;
	for (c = intfc->curves; c && *c; ++c)
	{
	    if (is_passive_boundary(Hyper_surf(*c)))
	    {
	    	if (!add_to_pointers(*c,&delete_curves))
		{
		    screen("ERROR in delete_passive_curves(), "
			   "add_to_pointers() failed\n");
		    clean_up(ERROR);
		}
	    }
	}
	for (c = delete_curves; c && *c; ++c)
	    (void) delete_curve(*c);

	join_nodes = NULL;
	for (n = intfc->nodes; n && *n; ++n)
	{
	    if ((size_of_pointers((*n)->in_curves) == 1) &&
	        (size_of_pointers((*n)->out_curves) == 1) &&
		(wave_type((*n)->in_curves[0]) ==
		    wave_type((*n)->out_curves[0])) &&
		(is_bdry((*n)->in_curves[0]) == is_bdry((*n)->out_curves[0])) &&
		(positive_component((*n)->in_curves[0]) ==
		    positive_component((*n)->out_curves[0])) &&
		(negative_component((*n)->in_curves[0]) ==
		    negative_component((*n)->out_curves[0])) )
	    {
		boolean join;
	        CURVE *cin, *cout;
		int   idir, iside;
		RECT_GRID *gr;

		join = YES;

	    	/* Check for corner nodes */
		gr = computational_grid(intfc);
		cin = (*n)->in_curves[0];
		cout = (*n)->out_curves[0];
		if (is_bdry(*n) && is_bdry(cin) && is_bdry(cout) &&
		    (rect_bdry_side_for_curve(&idir,&iside,cin,gr)
		     != rect_bdry_side_for_curve(&idir,&iside,cout,gr)))
		     join = NO;

		if (wave_type(cin) == DIRICHLET_BOUNDARY)
		{
		    HYPER_SURF *hsin, *hsout;

		    /*Dont join boundaries with different boundary states */
		    hsin = Hyper_surf(cin);
		    hsout = Hyper_surf(cout);

		    if (hs_bstate(hsin) && boundary_state_function(hsin) !=
		                    boundary_state_function(hsout))
		    	join = NO;
		    if (hs_bstate(hsin) && boundary_state(hsin) != 
		    		    boundary_state(hsout))
		    {
		        if ((boundary_state(hsin) == NULL) ||
			    (boundary_state(hsout) == NULL))
			    join = NO;
			else
			{
			    size_t sizest = size_of_state(intfc);
			    if (!hs_bstate(hsin) || 
			        (memcmp(boundary_state(hsin),
			               boundary_state(hsout),sizest) != 0))
			        join = NO;
			}
		    }
		}

		if (join)
		{
	            if (!add_to_pointers(*n,&join_nodes))
		    {
		        screen("ERROR in delete_passive_curves(), "
			       "add_to_pointers() failed\n");
		        clean_up(ERROR);
		    }
	        }
	    }
	}
	for (n = join_nodes; n && *n; ++n)
	{
	    join_curves((*n)->in_curves[0],(*n)->out_curves[0],
	                negative_component((*n)->in_curves[0]),
	                positive_component((*n)->in_curves[0]),
			NULL);
	}

	delete_nodes = NULL;
	for (n = intfc->nodes; n && *n; ++n)
	{
	    if (((*n)->in_curves == NULL) && ((*n)->out_curves == NULL))
	    {
	    	if (!add_to_pointers(*n,&delete_nodes))
		{
		    screen("ERROR in delete_passive_curves(), "
			   "add_to_pointers() failed\n");
		    clean_up(ERROR);
		}
	    }
	}
	for (n = delete_nodes; n && *n; ++n)
	    (void) delete_node(*n);
	interpolate_intfc_states(intfc) = sav_intrp;

	DEBUG_LEAVE(delete_passive_curves)
}		/*end delete_passive_curves*/

/*
*		angle_from_c1_c2_at_common_node():
*
*	Finds the angle between the curves c1 and c2 at their common node.
*	The angle is normalized to be in -PI to PI and is positive if
*	counter clockwise.
*/

EXPORT	double	angle_from_c1_to_c2_at_common_node(
	CURVE		*c1,
	ORIENTATION	c1_orient,
	CURVE		*c2,
	ORIENTATION	c2_orient,
	Front		*fr)
{
	double		a1[MAXD], a2[MAXD], ang;

	find_tangent_to_curve(Node_of(c1,c1_orient)->posn,
	        Bond_at_node(c1,c1_orient),c1,c1_orient,a1,fr);
	find_tangent_to_curve(Node_of(c2,c2_orient)->posn,
	        Bond_at_node(c2,c2_orient),c2,c2_orient,a2,fr);
	ang = normalized_angle(angle(a2[0],a2[1]) - angle(a1[0],a1[1]));
	if (ang < PI)
	    return ang;
	else 
	    return ang - 2*PI;
}		/*end angle_from_c1_to_c2_at_common_node*/

LOCAL	double compute_curvature2d(
	double           *v1,
        double           *v2,
        double           length,
        int             dim)
{
	double vprod[MAXD];
        double dtheta,kappa;

        (void) vector_product(v1,v2,vprod,dim);
        dtheta = asin(vprod[0]);
        kappa = dtheta/length;

	return kappa;
}		/*end compute_curvature2d*/

/*ARGSUSED*/
EXPORT	double	f_mean_curvature_at_point2d(
	FT_POINT		*p,
	HYPER_SURF_ELEMENT	*hse,
	HYPER_SURF		*hs,
	Front		*front)
{
	double		curvature = 0.0;
	double           curvaturep = 0.0, curvaturen = 0.0;
	CURVE		*c = Curve_of_hs(hs);
	BOND		*bnext,*b = Bond_of_hse(hse);
	FT_POINT           *pprev,*pnext;
	int		i, dim = c->interface->dim;
	double		v[MAXD],vn[MAXD], vp[MAXD];
	double           max_curvature;
	static FT_POINT	*p_prev,*p_next;
	static BOND	*b_prev,*b_next;
	static double 	ds,t[20];

	debug_print("curvature","Entered f_mean_curvature_at_point2d()\n");
	if (debugging("curvature"))
	{
	    (void) printf("Point, Bond and Curve\n");
	    print_general_vector("p = ",Coords(p),dim,"\n");
	    print_bond(b);
	}
	max_curvature = 2.0/front->rect_grid->h[0];
	if (p_prev == NULL)
	{
	    p_prev = Static_point(front->interf);
	    p_next = Static_point(front->interf);
	    stat_scalar(&b_prev,sizeof(BOND));
	    stat_scalar(&b_next,sizeof(BOND));
	    ds = front->rect_grid->h[0];
	}

	if (b == NULL || c == NULL)
	    return 0.0;

	if (p == b->start && b->prev == NULL)
	{
	    if (b->next == NULL)
	    {
	        if (debugging("curvature"))
	    	    (void) printf("curvature is FLAT\n");
	        debug_print("curvature","Left f_mean_curvature_at_point2d()\n");
	    	return 0.0;
	    }
	    bnext = b->next;
	    pnext = b->end;
	    find_tangent_to_curve(p,b,c,POSITIVE_ORIENTATION,v,front);
	    find_tangent_to_curve(pnext,bnext,c,POSITIVE_ORIENTATION,vn,front);
	    if (b->length != 0.0)
	    	curvature = compute_curvature2d(v,vn,b->length,dim);
	    else
	    	curvature = 0.0;
	    debug_print("curvature","Left f_mean_curvature_at_point2d()\n");
	    return curvature;
	}
	if (p == b->end && b->next == NULL)
	{
	    if (b->prev == NULL)
	    {
	        debug_print("curvature","Left f_mean_curvature_at_point2d()\n");
	        return 0.0;
	    }
	    bnext = b->prev;
	    pprev = b->start;
	    find_tangent_to_curve(p,b,c,NEGATIVE_ORIENTATION,v,front);
	    find_tangent_to_curve(pprev,bnext,c,NEGATIVE_ORIENTATION,vp,front);
	    if (b->length != 0.0)
	    	curvature = compute_curvature2d(vp,v,b->length,dim);
	    else
	    	curvature = 0.0;
	    debug_print("curvature","Left f_mean_curvature_at_point2d()\n");
	    return curvature;
	}
	
	if (p == b->end && b->next != NULL)
	{
	    pprev = b->start;
	    bnext = b->next;
	    pnext = bnext->end;
	    find_tangent_to_curve(p,bnext,c,POSITIVE_ORIENTATION,v,front);
	    find_tangent_to_curve(pprev,b,c,POSITIVE_ORIENTATION,vp,front);
	    if (b->length != 0.0)
	    	curvaturep = compute_curvature2d(vp,v,b->length,dim);
	    else
	    	curvaturep = 0.0;
	    
	    find_tangent_to_curve(p,b,c,NEGATIVE_ORIENTATION,v,front);
	    find_tangent_to_curve(pnext,bnext,c,NEGATIVE_ORIENTATION,vn,front);
	    if (bnext->length != 0.0)
	    	curvaturen = compute_curvature2d(v,vn,bnext->length,dim);
	    else
	    	curvaturen = 0.0;
	    curvature = 0.5*(curvaturep + curvaturen);
	}
        else if (p == b->start && b->prev != NULL)
	{
	    pnext = b->end;
	    bnext = b->prev;
	    pprev = bnext->start;
	    find_tangent_to_curve(p,b,c,POSITIVE_ORIENTATION,v,front);
	    find_tangent_to_curve(pprev,bnext,c,POSITIVE_ORIENTATION,vp,front);
	    if (bnext->length != 0.0)
	    	curvaturep = compute_curvature2d(vp,v,bnext->length,dim);
	    else 
	    	curvaturep = 0.0;
	   
	    find_tangent_to_curve(p,bnext,c,NEGATIVE_ORIENTATION,v,front);
	    find_tangent_to_curve(pnext,b,c,NEGATIVE_ORIENTATION,vn,front);
	    if (b->length != 0.0)
            	curvaturen = compute_curvature2d(v,vn,b->length,dim);
	    else 
	    	curvaturen = 0.0;
	
	    curvature = 0.5*(curvaturep + curvaturen);
	}
	if (isnan(curvature))
        {
            if (debugging("curvature"))
                printf("\n WARNING, curvature = nan, Coords is %f %f %f\n",
                        Coords(p)[0],Coords(p)[1],Coords(p)[2]);
            curvature = 0.0;
            if (debugging("curvature"))
                printf("Setting to 0.0:\n");
        }
	else if (fabs(curvature) > max_curvature)
	{
            if (debugging("curvature"))
                printf("\n WARNING, curvature > max_curvature, "
		       "Coords is %f %f %f\n",
                        Coords(p)[0],Coords(p)[1],Coords(p)[2]);
            curvature = curvature/fabs(curvature)*max_curvature;
	}
	debug_print("curvature","Left f_mean_curvature_at_point2d()\n");
	return curvature;
}		/*end f_mean_curvature_at_point2d*/


/*
*			f_mean_curvature_at_point3d():
*   In this function we use quatric fitting to calculate mean curvature
*   at point P of a triangulated surface. It is composed of three steps.
*   First, the normal to the surface at P is estimated by using 
*   weighted method proposed by Nelson Max. 

*   Second, using P as the origin, the normal as the Z direction and two 
*   uni_arrays on tangent plane as X and Y direction, a local coordinate 
*   system is constructed.The neighboring points around P are mapped 
*   into this local coordinate system. 

*   Finally,a quadric surface Z = AX*X + BY*Y + CX*Y + D*x + E*y in above
*   local coordinate system is constructed as the least square fitting of 
*   these mapped points. And the mean curvature of the quadric surface 
*   at the origin is used as the estimation of the mean curvature of the 
*   triangulated surface at P.
*
*
*/
/*ARGSUSED*/


EXPORT	double	f_mean_curvature_at_point3d( 
        FT_POINT		   *p,
        HYPER_SURF_ELEMENT *hse, 
        HYPER_SURF	   *hs, 
        Front		   *fr)
{ 
        int      nt_old,sign,i,j,k,m,n,nt;
	int      counter2 = 0,counter = 0;
        
	double	 tmp_p,curvature,tmpx,tmpy; 
	double    tmp[3],x[5],nor[3],local_x[3],local_y[3];
        double    length_x,**new_coords = NULL;
       
	static double    *unit_vec=NULL,*limit=NULL,*height = NULL;
	static double    **rotation=NULL,**tmp2 = NULL,*tmp1 = NULL;
	static double    **tmp_matrix = NULL,**least = NULL;
        static FT_POINT    **p_list,**p_listold;
	static double	max_curvature;
       
	FT_POINT    *pn;
	TRI	 *t; 
	TRI      **tris; 

	
	debug_print("curvature","Entered f_mean_curvature_at_point3d()\n");

	limit = fr->rect_grid->h;
        tmp_p = limit[0];
	
	for(i = 0; i < 2; ++i)
	    tmp_p = (tmp_p < limit[i+1])?tmp_p:limit[i+1];

	bi_array(&new_coords,48,3,FLOAT);
	
	if(height == NULL)
	{
	    bi_array(&tmp2,48,5,FLOAT);
            bi_array(&least,48,5,FLOAT);
	    bi_array(&rotation,3,3,FLOAT);
            bi_array(&tmp_matrix,48,5,FLOAT);
	    uni_array(&tmp1,48,FLOAT);
	    uni_array(&unit_vec,48,FLOAT);
	    uni_array(&height,48,FLOAT);
	    uni_array(&p_listold,48,sizeof(FT_POINT*));
	    uni_array(&p_list,48,sizeof(FT_POINT*));
	
	    for(i = 0; i < 48; ++i)
	    {
	        if(i == 0)
		    unit_vec[i] = 1;
		else
		    unit_vec[i] = 0;
	    }
	    max_curvature = 1.0/fr->rect_grid->h[0];
	}
	
	normal(p,hse,hs,nor,fr);
	nt_old = tri_list_computed_by_normal(p,Tri_of_hse(hse),
	                             &tris,hs->interface);
	   /*determine  nearest platelet point around p*/
	for (n = 0; n < nt_old; ++n) 
	{ 
	    t = tris[n];
	    if(t == NULL)
	        break;
	    k = Vertex_of_point(t,p);
	    pn = Point_of_tri(t)[Next_m3(k)];
	    p_listold[n] = p_list[n] = pn;
	}
	
	nt_old = counter2 = n;
	/*determine those next nearest pallete point around p
	          if necessary */
	if(counter2 < 6)
	{
	    for (n = 0; n < nt_old; ++n) 
       	    { 
	       t = tris[n];
	       while(Next_tri_at_vertex(t,p_listold[n])!=tris[n]&&
	           Next_tri_at_vertex(t,p_listold[n])!=NULL)
	       {
		   t = Next_tri_at_vertex(t,p_listold[n]);
		   k = Vertex_of_point(t,p_listold[n]);
		   pn = Point_of_tri(t)[Next_m3(k)];
		   for(j = 0; j < nt_old; ++j)
		   {
		       if(pn == p_listold[j] || pn ==p )
			  break;
		   }
		   if(j == nt_old)
		   {
		      for(i = 0; i < counter; ++i)
		      {
		         if(pn == p_list[counter2+i])
			    break;
		      }	 
		      if(i == counter)
		      {
			 p_list[counter2 + counter] = pn;
			 counter++;
		      }
		   }
                }
	     }
	}
	nt = counter2 + counter;

	if (nt > 48)
	{
	    printf("The number of triangles is more than expected\n");
	    printf("nt = %d\n",nt);
	}
	nt = min(nt,48);
	if(nt < 6)
	{
	   free(new_coords);
	   return 0.0;
        }

	/*project vector X(pn-p) onto the tangent plane at p 
	     and use its projection as direction of x axis*/
        
	pn = p_list[0];
	for(i = 0;i < 3;++i)
	    local_x[i] = Coords(pn)[i] - Coords(p)[i];
	
	tmp_p = scalar_product(local_x,nor,3);
  	
	for(i = 0; i < 3; ++i) 
	    local_x[i] = local_x[i] - tmp_p*nor[i]; 
	    
	local_y[0] = local_x[2]*nor[1] - local_x[1]*nor[2]; 
	local_y[1] = local_x[0]*nor[2] - local_x[2]*nor[0]; 
	local_y[2] = local_x[1]*nor[0] - local_x[0]*nor[1]; 
	tmpx = Mag3d(local_x); 
	tmpy = Mag3d(local_y);
	for(i = 0; i < 3; ++i) 
	{
	    local_x[i] /=tmpx; 
	    local_y[i] /=tmpy; 
	}
	/*find the rotation matrix from global to local coordinates*/
	for(i = 0; i < 3; ++i)
        {
	    rotation[0][i] = local_x[i];
            rotation[1][i] = local_y[i];
	    rotation[2][i] = nor[i];
	}
	    /*find relative coordinates of all pallete points
	         by transformation X = R(X'-Xp)*/
	for(n = 0; n < nt; ++n)
	{
	    for(i = 0; i < 3; ++i)
	    {
	        for(j = 0; j < 3; ++j)
		    new_coords[n][i]+=rotation[i][j]*(Coords(p_list[n])[j] -
		     Coords(p)[j]);
	    }
	    
	    least[n][0] = new_coords[n][0]*new_coords[n][0]; 
	    least[n][2] = new_coords[n][1]*new_coords[n][1]; 
	    least[n][1] = new_coords[n][0]*new_coords[n][1]; 
	    least[n][3] = new_coords[n][0];
	    least[n][4] = new_coords[n][1];
	    height[n] =  -new_coords[n][2]; 
	}
	  /*solve the least square problem Ax = b by QR factorization*/
	for(k = 0; k < 5; ++k)
	{
	    double tmp4 = 0,length_v = 0;
            double *tmp3 = NULL;

	    uni_array(&tmp3,5-k,FLOAT);

	    for(i = 0; i < nt-k;++i)
	    {
	        tmp1[i] = least[i+k][k];
	    }
	    length_x = sqrt(scalar_product(tmp1,tmp1,nt-k));
	    if(tmp1[0] >=0)
	        sign = 1;
            else 
	        sign = -1;
	    for(i = 0; i < nt; ++i)
	    {
		if(i < nt - k)
	           tmp2[i][k] = length_x*unit_vec[i]*sign + tmp1[i];
	        else
		   tmp2[i][k] = 0;
	    }
	    for(i = 0; i < nt; ++i)
	    {
	        length_v += tmp2[i][k]*tmp2[i][k];
	    }
	    length_v = sqrt(length_v);
	    for(i = 0; i < nt; ++i)
	    {
	        tmp2[i][k] /= length_v;
	    }
	    
	    for(j = 0; j < 5-k; ++j)
            {
	        for(i = 0; i < nt-k; ++i)
		{
                    tmp3[j] += tmp2[i][k]*least[i+k][j+k];
		}
	    }
	    for(i = 0; i < nt-k;++i)
	    {
	        for(j = 0; j < 5-k; ++j)
                {
		    tmp_matrix[i][j] = tmp2[i][k]*tmp3[j];
		    least[i+k][j+k] = least[i+k][j+k] - 2*tmp_matrix[i][j];
		}
            }
            
	    for(i = 0; i < nt-k; ++i)
	    {
                tmp4 += tmp2[i][k]*height[i+k];
	    }
	    for(i = 0; i < nt-k; ++i)
	    {
	        height[i+k] = height[i+k] - 2.0*tmp4*tmp2[i][k];
	    }
	    free(tmp3);
	}
	   /*Since Rx = b is known, solve the system by back substition*/
	for(i = 4; i >=0;--i)
	{
	    for(j = i+1; j < 5; ++j)
	        height[i] -= least[i][j]*x[j];
	    x[i] = height[i]/least[i][i];
	}
	curvature = x[0]+x[2]+x[2]*x[3]*x[3]+x[0]*x[4]*x[4]-x[1]*x[3]*x[4];
        tmp_p = 1+x[3]*x[3]+x[4]*x[4];
	tmp_p = pow(tmp_p,1.5);
	curvature /=tmp_p;
	
	if (isnan(curvature))
	{
	    printf("\n WARNING, curvature = nan, Coords is %f %f %f\n",
	        Coords(p)[0],Coords(p)[1],Coords(p)[2]);
	    curvature = 0.0;
	    printf("Setting to 0.0:\n");
	}
	else if (fabs(curvature) >= max_curvature)
	{ 
	    printf("\n WARNING, large curvature = %f,Coords is %f %f %f\n",
	        curvature,Coords(p)[0],Coords(p)[1],Coords(p)[2]);
	    curvature = max_curvature*curvature/fabs(curvature);   
	    printf("Setting to signed limite: %f\n",curvature);
	}
	free(new_coords);
	return curvature;
}		/*end f_mean_curvature_at_point3d*/

EXPORT  void set_test_front(
	F_INIT_DATA *Init,
	Front *front)
{
	set_front_hooks(init_data(Init));
	MaxFrontSpeed(front) = InitialMaxFrontSpeed(Init);
	Clear_redistribution_parameters(front);
	Init_redistribution_function(front) = f_init_redistribute;
	set_front_pp_grid(init_data(Init),front);
	prompt_for_front_options(init_data(Init),front);
	f_set_default_front_parameters(init_data(Init),front);
	init_front(init_data(Init),front);
}	/* end set_test_front */


EXPORT  void set_default_front(
	F_INIT_DATA *Init,
	Front *front)
{
	set_front_hooks(init_data(Init));
	MaxFrontSpeed(front) = InitialMaxFrontSpeed(Init);
	Init_redistribution_function(front) = f_init_redistribute;
	set_front_pp_grid(init_data(Init),front);
	set_default_front_options(init_data(Init),front);
	f_set_default_front_parameters(init_data(Init),front);
	init_front(init_data(Init),front);
	front->resolution_level = 3;
}	/* end set_default_front */

EXPORT	void	test1d(
	Front *front)
{
	INTERFACE  *intfc, *new_intfc;
	FT_POINT *p;
	double x;
	COMPONENT left, right;
	
	intfc = front->interf;
	x = 7.7; left = 3; right = 4;
	make_point(&x,left,right);
	x = 7.7; left = 4; right = 5;
	make_point(&x,left,right);
	x = 5.5; left = 2; right = 3;
	make_point(&x,left,right);
	x = 3.3; left = 1; right = 2;
	make_point(&x,left,right);
	print_interface(intfc);

	print_interface(intfc);

	(void) printf ("delete point x = 5.5\n");
	p = *(intfc->points+1);
	delete_point(p);
	reset_intfc_components(intfc);
	print_interface(intfc);

	(void) printf ("\n\ncopy interface\n\n");
	new_intfc = copy_interface(intfc);
	print_interface(new_intfc);
}

/*ARGSUSED*/

LOCAL	void set_boundary_node_type(
	NODE *n,
	INTERFACE *intfc)
{
	RECT_GRID *rgr = computational_grid(intfc);
	double eps = grid_tolerance(rgr);
	int i;
	for (i = 0; i < rgr->dim; ++i)
	{
	    if (fabs(Coords(n->posn)[i] - rgr->L[i]) < eps)
	    {
		switch (rect_boundary_type(intfc,i,0))
		{
		case PERIODIC_BOUNDARY:
		case REFLECTION_BOUNDARY:
		    node_type(n) = SUBDOMAIN_NODE;
		    break;
		case DIRICHLET_BOUNDARY:
		    node_type(n) = DIRICHLET_NODE;
		    break;
		case NEUMANN_BOUNDARY:
		    node_type(n) = NEUMANN_NODE;
		    break;
		}
	    }
	    else if (fabs(Coords(n->posn)[i] - rgr->U[i]) < eps)
	    {
		switch (rect_boundary_type(intfc,i,1))
		{
		case PERIODIC_BOUNDARY:
		case REFLECTION_BOUNDARY:
		    node_type(n) = SUBDOMAIN_NODE;
		    break;
		case DIRICHLET_BOUNDARY:
		    node_type(n) = DIRICHLET_NODE;
		    break;
		case NEUMANN_BOUNDARY:
		    node_type(n) = NEUMANN_NODE;
		    break;
		}
	    }
	}

}	/* end set_boundary_node_type */

EXPORT double length_of_scalar_wave(Front *front)
{
	double length;
	INTERFACE *intfc = front->interf;
	CURVE **c;

	length = 0.0;
	for (c = intfc->curves; c && *c; ++c)
	{
	    if (wave_type(*c) >= FIRST_PHYSICS_WAVE_TYPE &&
	        wave_type(*c) < FIRST_VECTOR_PHYSICS_WAVE_TYPE)
		length += curve_length(*c);
	}
	return length;
}	/* end length_of_scalar_wave */


/*
*			f_identify_physical_node():
*
*	This routine is called in scalar_unravel. Its purpose is to set the
*	node_type and curve start/end status of new nodes (especially CC_NODES)
*	created within this routine.
*/

LOCAL void f_identify_physical_node(
	NODE		*n)
{
	CURVE		**c;
	int		num_curves = 0;
	int             num_neumann = 0;
	int             num_dirichlet = 0;
	int             num_passive = 0;
	int             num_subdomain = 0;
	int             num_phys = 0;
	int             wtype;

	debug_print("identify","Entered f_identify_physical_node()\n");
	if (debugging("identify")) print_node(n);
	for (c = n->in_curves; c && *c; c++)
	{
	    wtype = wave_type(*c);
	    num_curves++;
	    if (wtype < FIRST_PHYSICS_WAVE_TYPE)
	    {
		switch (wtype)
		{
		case NEUMANN_BOUNDARY:
		    num_neumann++;
		    break;
		case DIRICHLET_BOUNDARY:
		    num_dirichlet++;
		    break;
		case PASSIVE_BOUNDARY:
		    num_passive++;
		    break;
		case SUBDOMAIN_BOUNDARY:
		case REFLECTION_BOUNDARY:
		    num_subdomain++;
		    break;
		case GROWING_BODY_BOUNDARY:
		    num_phys++;
		    break;
		}
	    }
	    else
		num_phys++;
	}
	for (c = n->out_curves; c && *c; c++)
	{
	    wtype = wave_type(*c);
	    num_curves++;
	    if (wtype < FIRST_PHYSICS_WAVE_TYPE)
	    {
		switch (wtype)
		{
		case NEUMANN_BOUNDARY:
		    num_neumann++;
		    break;
		case DIRICHLET_BOUNDARY:
		    num_dirichlet++;
		    break;
		case PASSIVE_BOUNDARY:
		    num_passive++;
		    break;
		case SUBDOMAIN_BOUNDARY:
		case REFLECTION_BOUNDARY:
		    num_subdomain++;
		    break;
		case GROWING_BODY_BOUNDARY:
		    num_phys++;
		    break;
		}
	    }
	    else
		num_phys++;
	}
	if (num_subdomain > 0)
	{
	    node_type(n) = SUBDOMAIN_NODE;
	    for (c = n->in_curves; c && *c; c++)
	    {
	        wtype = wave_type(*c);
		if (wtype == PASSIVE_BOUNDARY)
	            end_status(*c) = PASSIVE;
		else if (wtype < FIRST_PHYSICS_WAVE_TYPE)
	            end_status(*c) = FIXED;
		else
	            end_status(*c) = INCIDENT;
	    }
	    for (c = n->out_curves; c && *c; c++)
	    {
	        wtype = wave_type(*c);
		if (wtype == PASSIVE_BOUNDARY)
	            start_status(*c) = PASSIVE;
		else if (wtype < FIRST_PHYSICS_WAVE_TYPE)
	            start_status(*c) = FIXED;
		else
	            start_status(*c) = INCIDENT;
	    }
	}
	else if (num_phys == 0)
	{
	    if ((num_neumann > 0) || (num_dirichlet > 0))
	        node_type(n) = FIXED_NODE;
	    else
	        node_type(n) = PASSIVE_NODE;
	    for (c = n->in_curves; c && *c; c++)
	    {
	        wtype = wave_type(*c);
		if (wtype == PASSIVE_BOUNDARY)
	            end_status(*c) = PASSIVE;
		else
	            end_status(*c) = FIXED;
	    }
	    for (c = n->out_curves; c && *c; c++)
	    {
	        wtype = wave_type(*c);
		if (wtype == PASSIVE_BOUNDARY)
	            start_status(*c) = PASSIVE;
		else
	            start_status(*c) = FIXED;
	    }
	}
	else if (((num_neumann > 0) || (num_dirichlet > 0)) && (num_phys == 1))
	{
	    node_type(n) = (num_neumann > 0) ? NEUMANN_NODE : DIRICHLET_NODE;
	    for (c = n->in_curves; c && *c; c++)
	    {
	        wtype = wave_type(*c);
		if (wtype == PASSIVE_BOUNDARY)
	            end_status(*c) = PASSIVE;
		else if (wtype < FIRST_PHYSICS_WAVE_TYPE)
	            end_status(*c) = FIXED;
		else
	            end_status(*c) = INCIDENT;
	    }
	    for (c = n->out_curves; c && *c; c++)
	    {
	        wtype = wave_type(*c);
		if (wtype == PASSIVE_BOUNDARY)
	            start_status(*c) = PASSIVE;
		else if (wtype < FIRST_PHYSICS_WAVE_TYPE)
	            start_status(*c) = FIXED;
		else
	            start_status(*c) = INCIDENT;
	    }
	}
	else
	{
	    node_type(n) = ERROR;
	    debug_print("identify","Left f_identify_physical_node\n");
	    return;
	}
	debug_print("identify","Left f_identify_physical_node\n");
}		/*end f_identify_physical_node*/


LOCAL 	SURFACE *prompt_make_NWGrid_surface(
	INTERFACE *intfc)
{
	FILE *ifile;
	FT_POINT **pts,*pv[MAXD];
	TRI **tris,*tri;
	SURFACE *surf;
	double coords[MAXD],vel[MAXD];
	int i,j,num_points,num_tris;
	int iv[MAXD],inb[3];
	COMPONENT neg_comp,pos_comp;
	char in_name[256];

	set_current_interface(intfc);
	neg_comp = 2;
	pos_comp = 3;
	surf = make_surface(neg_comp,pos_comp,NULL,NULL);
	screen("Enter name of the input file: ");
	Scanf("%s\n",in_name);
	ifile = fopen(in_name,"r");
	if (ifile == NULL)
	{
	    screen("Cannot open input file: %s\n",in_name);
	    clean_up(ERROR);
	}

	if (!fgetstring(ifile,"Number of Nodes, Number of Elements"))
	{
	    screen("Cannot get string for nodes and elements\n");
	    clean_up(ERROR);
	}
	fscanf(ifile,"%d %d",&num_points,&num_tris);
	uni_array(&pts,num_points,sizeof(FT_POINT*));
	uni_array(&tris,num_tris,sizeof(TRI*));

	if (!fgetstring(ifile,"X,Y,Z-Coordinates, U,V,W-Velocity"))
        {
            screen("Cannot get string for coords, vel\n");
            clean_up(ERROR);
        }
	for (i = 0; i < num_points; ++i)
	{
	    int j;
	    char ctmp[100];
	    for (j = 0; j < 3; ++j)
	    {
	    	fscanf(ifile,"%s ",ctmp);
		coords[j] = atof(ctmp);
	    }
	    for (j = 0; j < 3; ++j)
	    {
	    	fscanf(ifile,"%s ",ctmp);
		vel[j] = atof(ctmp);
	    }
	    pts[i] = Point(coords);
	}

	if (!fgetstring(ifile,"Element to node connectivity"))
        {
            screen("Cannot get node connectivity\n");
            clean_up(ERROR);
        }
	for (i = 0; i < num_tris; ++i)
	{
	    fscanf(ifile,"%d %d %d",iv,iv+1,iv+2);
	    for (j = 0; j < 3; ++j)
	    	pv[j] = pts[iv[j]];
	    tris[i] = make_tri(pv[0],pv[1],pv[2],NULL,NULL,NULL,NO);
	    if (i != 0) 
	    {
		tris[i]->prev = tris[i-1];
		tris[i-1]->next = tris[i];
	    }
	}

	if (!fgetstring(ifile,"Element to element connectivity"))
        {
            screen("Cannot get element connectivity\n");
            clean_up(ERROR);
        }
	for (i = 0; i < num_tris; ++i)
	{
	    fscanf(ifile,"%d %d %d",inb,inb+1,inb+2);
	    for (j = 0; j < 3; ++j)
	    {
		if (inb[j] != -1)
		    Tri_on_side(tris[i],(j+1)%3) = tris[inb[j]];
		else 
		    Tri_on_side(tris[i],(j+1)%3) = NULL;
	    }
	}
	surf->num_tri = num_tris;
	first_tri(surf) = tris[0];
	first_tri(surf)->prev = head_of_tri_list(surf);
	last_tri(surf) = tris[num_tris-1];
	last_tri(surf)->next = tail_of_tri_list(surf);
	reset_intfc_num_points(intfc);
	return surf;
}	/* end prompt_make_NWGrid_surface */

/*---------------End by Eric-----------------*/

/*TMP*/
#define at_beginning(p,b,orient)					\
	(  (orient == POSITIVE_ORIENTATION && p == b->start)		\
 	|| (orient == NEGATIVE_ORIENTATION && p == b->end))

LOCAL void point_at_distance_along_curve(
	FT_POINT		*p,
	ORIENTATION	orient, /* direction along curve for state evaluation */
	double		ds,	/* distance along curve for state evaluation  */
	int		npts,	/* number of points on curve to load          */
	FT_POINT		**posn,
	BOND		**bvp,
	double		*t,
	Front		*fr)
{
	BOND		*b = Bond_of_hse(p->hse);
	CURVE		*c = Curve_of_hs(p->hs);
	CURVE		*cc;
	BOND		*cb, *fb;
	int		isgn, indx, i, j, dim = fr->rect_grid->dim;
	ORIENTATION	c_or;
	double		lds;

			/* initialize loop */
		/* p should be at "beginning" of cb */

	if (ds < 0.0) 
	{
	    ds = -ds;
	    orient = Opposite_orient(orient);
	}
	isgn = (orient == NEGATIVE_ORIENTATION) ? -1 : 1;

	cb = (b==NULL || at_beginning(p,b,orient)) ? b 
			: Following_bond(b,orient);
	cc = c;
	c_or = orient;

	if (cb == NULL) 	/* at termination of curve */
	{
	    if (!is_closed_node(Node_of(cc,c_or)))
	    {
	    	no_continuation_along_curve(0,c_or,npts,cc,
					    t,posn,bvp,fr);
		return;
	    }
	    cb = Bond_at_node(cc,c_or);
	}

	for (i = 0, lds = ds; i < npts; i++, lds += ds)
	{
	    if (cb == NULL)
		continue;
	    indx = i*isgn;

	    /* loop to find cb, in the middle of which is the displaced point */

	    while (bond_length(cb) < lds) 
	    {
	    	lds -= bond_length(cb);
	    	fb = Following_bond(cb,c_or);
	    	if ((fb == NULL) && (!is_closed_node(Node_of(cc,c_or))))
		{
		    cb = NULL;
		    break;
		}
		cb = (fb == NULL) ? Bond_at_node(cc,c_or) : fb;
	    }
	    if (cb != NULL)
	    {

	    	/* interpolate */

	    	t[indx] = lds / bond_length(cb);
	    	if (c_or == NEGATIVE_ORIENTATION)
	    	    t[indx] = 1.0 - t[indx];
		for (j = 0; j < dim; j++)
		{
		    Coords(posn[indx])[j] = Coords(cb->start)[j] + 
					    t[indx]*(Coords(cb->end)[j] -
					    Coords(cb->start)[j]);
		}
		if (bvp != NULL)
		{
		    bvp[indx] = cb;
		    t[indx] *= bond_length(cb);
		}
	    }
	    else
	    {
	    	/* at termination of curve */
	    	no_continuation_along_curve(indx,c_or,npts,cc,
					    t,posn,bvp,fr);
	    }
	}
}		/*end point_at_distance_along_curve*/


LOCAL	void	no_continuation_along_curve(
	int		indx,
	ORIENTATION	orient,
	int		npts,
	CURVE		*cc,
	double		*t,
	FT_POINT		**posn,
	BOND		**bvp,
	Front		*fr)
{
	FT_POINT		*curr_posn;
	int		i, j, dim = fr->rect_grid->dim;
	int		isgn = (orient == NEGATIVE_ORIENTATION) ? -1 : 1;
	size_t		sizest = fr->sizest;

			/* use states at node */

	if (orient == POSITIVE_ORIENTATION) 
	{
	    if (bvp != NULL)
	    {
	    	bvp[indx] = cc->last;
	    	t[indx] = bond_length(cc->last);
	    }
	    curr_posn = cc->last->end;
	}
	else 
	{
	    if (bvp != NULL)
	    {
	    	bvp[indx] = cc->first;
	    	t[indx] = 0.0;
	    }
	    curr_posn = cc->first->start;
	}
	for (i = 0; i < dim; i++)
	    Coords(posn[indx])[i] = Coords(curr_posn)[i];
	for (i = indx+isgn; i*isgn < npts; i += isgn)
	{
	    if (bvp != NULL)
	    {
	    	bvp[i] = bvp[indx];
	    	t[i] = t[indx];
	    }
	    for (j = 0; j < dim; j++)
		Coords(posn[i])[j] = Coords(posn[indx])[j];
	}
}		/*end no_continuation_along_curve*/
