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
*				fuserintfc.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*
*	User definable hooks to the interface libary.
*/




#include <front/fdecs.h>


	/* LOCAL Function Declarations */
LOCAL	Locstate f_read_print_state_data(INIT_DATA*,const IO_TYPE*,
                                         Locstate,INTERFACE*);
LOCAL	double	f_cross_tolerance(INTERFACE*);
LOCAL	void	f_fprint_intfc_state(FILE*,Locstate,INTERFACE*);
LOCAL	void	f_fprint_state_data(FILE*,Locstate,INTERFACE*);
LOCAL	void	f_reflect_state(Locstate,INTERFACE*,double*,double*,double*);
LOCAL	void	f_tangent(FT_POINT*,BOND*,CURVE*,double*,Front*);
LOCAL	void	f_fshow_intfc_states1d(FILE*,INTERFACE*);
LOCAL	void	normal1d(FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*,double*,Front*);
LOCAL	void	slsr1d(FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*,
		       Locstate*,Locstate*);
LOCAL	void	state_at_point(COMPONENT,double*,HYPER_SURF_ELEMENT*,
			       HYPER_SURF*,Locstate);
LOCAL 	void 	assign_boundary_node_type(NODE*,INTERFACE*,RECT_GRID*,double);
LOCAL 	void 	assign_boundary_curve_wave_type(CURVE*,INTERFACE*,RECT_GRID*,
			double);
LOCAL   void    assign_boundary_point_wave_type(FT_POINT*,INTERFACE*,RECT_GRID*,
                       	double);
LOCAL   double   lagrangian_n_pt(int,double,double*,double*);
LOCAL	boolean	f_set_boundary1d(INTERFACE*,RECT_GRID*,COMPONENT,double);
LOCAL	boolean	f_set_boundary2d(INTERFACE*,RECT_GRID*,COMPONENT,double);
LOCAL	void	f_fshow_intfc_states2d(FILE*,INTERFACE*);
LOCAL	void	f_lagrangian_tangent(FT_POINT*,BOND*,CURVE*,double*,Front*);
LOCAL	void	f_wlsp_tangent(FT_POINT*,BOND*,CURVE*,double*,Front*);
/* linpack dependent LOCAL	void	f_spline_tangent(FT_POINT*,BOND*,CURVE*,double*,Front*); */
LOCAL	void	first_order_normal2d(FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*,
			double*,Front*);
LOCAL	void	f_normal2d(FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*,
			double*,Front*);
LOCAL	void	slsr2d(FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*,
		       Locstate*,Locstate*);
LOCAL	void	state_along_bond(COMPONENT,double*,HYPER_SURF_ELEMENT*,
				 HYPER_SURF*,Locstate);
LOCAL	void	f_area_weighted_normal3d(FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*,
	                                 double*,Front*);
LOCAL	void	f_sine_weighted_normal3d(FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*,
	                                 double*,Front*);
LOCAL	void	f_plane_fit_normal3d(FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*,
	                             double*,Front*);
LOCAL	void	f_wlsp_normal(FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*,
	                             double*,Front*);
LOCAL	boolean	f_set_boundary3d(INTERFACE*,RECT_GRID*,COMPONENT,double);
LOCAL	void	f_fshow_intfc_states3d(FILE*,INTERFACE*);
LOCAL 	void 	f_WLSP_set_intfc_geom(Front*,INTERFACE*);
LOCAL	void	slsr3d(FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*,
		       Locstate*,Locstate*);
LOCAL	void	state_in_tri(COMPONENT,double*,HYPER_SURF_ELEMENT*,
			     HYPER_SURF*,Locstate);

EXPORT	F_USER_INTERFACE *f_user_hook(
	int		dim)
{
	static F_USER_INTERFACE Fuser_hooks[3];
	static boolean first = YES;

	if (first == YES)
	{
	    int i;
	    static F_INTERFACE_TOLERANCES Itol;

	    first = NO;

	    /* Set default values for F_INTERFACE_TOLERANCES */
	    Itol._DtReductionFac      = 0.8;

	    /* Set default values for Fuser_hooks fields*/

	    /* fields valid for all dimensions */

	    for (i = 0; i < 3; ++i)
	    {
	        zero_scalar(&Fuser_hooks[i]._computational_grid,
			    sizeof(RECT_GRID));
	        Fuser_hooks[i]._bstates = NULL;
	        Fuser_hooks[i]._num_bstates = 0;
	        Fuser_hooks[i]._first_node = 0;
	        Fuser_hooks[i]._last_node = 0;
	        Fuser_hooks[i]._sizest = 0;
	        Fuser_hooks[i]._interpolate_intfc_states = NO;
	        Fuser_hooks[i]._mono_comp_curves = NO;
	        Fuser_hooks[i]._fprint_wave_type = f_fprint_wave_type;
	        Fuser_hooks[i]._wave_type_as_string = f_wave_type_as_string;
	        Fuser_hooks[i]._read_wave_type_from_string =
		    f_read_wave_type_from_string;
	        Fuser_hooks[i]._bi_interpolate_intfc_states =
		    linear_state_interpolator;
	        Fuser_hooks[i]._fprint_state_data = f_fprint_state_data;
	        Fuser_hooks[i]._read_print_state_data = f_read_print_state_data;
	        Fuser_hooks[i]._nearest_intfc_state = f_nearest_intfc_state;
	        Fuser_hooks[i]._reflect_state = f_reflect_state;
	        Fuser_hooks[i]._fprint_intfc_state = f_fprint_intfc_state;
	        Fuser_hooks[i]._alloc_state = f_alloc_state;
	        Fuser_hooks[i]._alloc_intfc_state = f_alloc_intfc_state;
	        Fuser_hooks[i]._clear_state = f_clear_state;
	        Fuser_hooks[i]._obstacle_state = f_clear_state;
	        Fuser_hooks[i]._FInterfaceTolerances = Itol;
	        Fuser_hooks[i]._default_perform_redistribution_function =
		    f_perform_redistribution;
	        Fuser_hooks[i]._merge_hs_flags = f_merge_hs_flags;
	        Fuser_hooks[i]._interface_tangent_function._tangent = f_tangent;
	        Fuser_hooks[i]._interface_tangent_function._tangent_name =
		    strdup("f_tangent");
	        Fuser_hooks[i]._set_tangent_function = f_set_tangent_function;
	        Fuser_hooks[i]._set_normal_function = f_set_normal_function;
	        Fuser_hooks[i]._alloc_MaxFrontSpeed = f_alloc_MaxFrontSpeed;
	    }

	    /* fields valid for both 2D and 3D */
	    for (i = 1; i < 3; ++i)
	    {
	        Fuser_hooks[i]._fprint_hsbdry_type = f_fprint_hsbdry_type;
	        Fuser_hooks[i]._read_hsbdry_type_from_string =
		    f_read_hsbdry_type_from_string;
	        Fuser_hooks[i]._tri_interpolate_intfc_states =
		    linear_tri_state_interpolator;
	        Fuser_hooks[i]._read_print_boundary_state_data =
		    f_read_print_boundary_state_data;
	    }

	    /* Dimension specific fields */
	    Fuser_hooks[0]._fprint_hsbdry_type = NULL;
	    Fuser_hooks[0]._read_hsbdry_type_from_string = NULL;
	    Fuser_hooks[0]._slsr = slsr1d;
	    Fuser_hooks[0]._tri_interpolate_intfc_states = NULL;
	    Fuser_hooks[0]._state_along_hypersurface_element = state_at_point;
	    Fuser_hooks[0]._form_subintfc_via_communication =
			f_intfc_communication1d;
	    Fuser_hooks[0]._fshow_intfc_states = f_fshow_intfc_states1d;

	    Fuser_hooks[0]._mean_curvature_at_point = NULL;
	    Fuser_hooks[0]._interface_normal_function._normal = normal1d;
	    Fuser_hooks[0]._interface_normal_function._normal_name =
	        	strdup("normal1d");

	    Fuser_hooks[1]._slsr = slsr2d;
	    Fuser_hooks[1]._state_along_hypersurface_element = state_along_bond;
	    Fuser_hooks[1]._form_subintfc_via_communication =
			f_intfc_communication2d;

	    Fuser_hooks[1]._fshow_intfc_states = f_fshow_intfc_states2d;
	    Fuser_hooks[1]._mean_curvature_at_point =
			f_wlsp_curvature;
	    Fuser_hooks[1]._interface_normal_function._normal = 
			f_wlsp_normal;
	    Fuser_hooks[1]._interface_normal_function._normal_name =
	        	strdup("first_order_normal2d");

	    Fuser_hooks[2]._slsr = slsr3d;
	    Fuser_hooks[2]._state_along_hypersurface_element = state_in_tri;
	    Fuser_hooks[2]._form_subintfc_via_communication =
			f_intfc_communication3d;
	    Fuser_hooks[2]._fshow_intfc_states = f_fshow_intfc_states3d;
	    Fuser_hooks[2]._mean_curvature_at_point =
			f_wlsp_curvature;
	    Fuser_hooks[2]._interface_normal_function._normal =
	        	f_wlsp_normal;
	    Fuser_hooks[2]._interface_normal_function._normal_name =
	        	strdup("f_wlsp_normal");
	}
	if (dim < 1 || dim > 3)
	{
	    screen("ERROR in f_user_hook(), invalid dim %d\n",dim);
	    clean_up(ERROR);
	    return NULL;
	}
	else
	    return Fuser_hooks + dim - 1;
}		/*end f_user_hook*/

EXPORT	void	f_preserve_user_hooks(
	int                     dim,
	PRESERVE_USER_HOOKS	flag)
{
	F_USER_INTERFACE        *fuh;
	static F_USER_INTERFACE Sav_fuh;
	static F_USER_INTERFACE *sav_fuh = NULL;

	i_preserve_user_hooks(dim,flag);
	switch (flag)
	{
	case SAVE_HOOKS:
	    if (sav_fuh != NULL)
	    {
		screen("ERROR in f_preserve_user_hooks(), "
		       "attempt to save without prior restore\n");
		clean_up(ERROR);
	    }
	    fuh = f_user_hook(dim);
	    sav_fuh = &Sav_fuh;
	    *sav_fuh = *fuh;
	    break;
	case RESTORE_HOOKS:
	    if (sav_fuh == NULL)
	    {
		screen("ERROR in f_preserve_user_hooks(), "
		       "attempt to restore without prior save\n");
		clean_up(ERROR);
	    }
	    fuh = f_user_hook(dim);
	    *fuh = *sav_fuh;
	    sav_fuh = NULL;
	    break;
	}
}		/*end f_preserve_user_hooks*/

EXPORT	void f_set_interface_hooks(
	int		dim,
	INIT_DATA       *init)
{
	I_USER_INTERFACE *iuh = i_user_hook(dim);
	F_USER_INTERFACE *fuh = f_user_hook(dim);
	int	         i;

	/* Front extended structure sizes */
	iuh->size_interface = sizeof(F_INTERFACE);
	iuh->size_point = sizeof(F_POINT);
	iuh->size_curve = sizeof(F_CURVE);
	iuh->size_node = sizeof(F_NODE);
	iuh->size_hyper_surf = sizeof(F_HYPER_SURF);
	iuh->size_hyper_surf_bdry = sizeof(F_HYPER_SURF_BDRY);
	switch (dim)
	{
	case 1:
	    break;
	case 2:
	    break;
	case 3:
	    iuh->size_bond_tri = sizeof(F_BOND_TRI);
	    break;
	}

	/* Front extended function pointers */
	iuh->_read_boundary_type_from_string = f_read_wave_type_from_string;
	iuh->_fprint_boundary_type = f_fprint_wave_type;
	iuh->_user_make_interface = f_user_make_interface;
	iuh->_copy_interface = f_copy_interface;
	iuh->_user_read_print_interface = f_user_read_print_interface;
	iuh->_user_fprint_interface = f_user_fprint_interface;
	iuh->_delete_interface = f_delete_interface;
	iuh->_user_fprint_intfc_rect_grids = f_user_fprint_intfc_rect_grids;
	iuh->_user_read_print_intfc_rect_grids =
	    f_user_read_print_intfc_rect_grids;
	iuh->_Point = f_Point;
	iuh->_Static_point = f_Static_point;
	iuh->_average_points = f_average_points;
	iuh->_copy_point = f_copy_point;
	iuh->_reconstruct_interface_pointers = f_reconstruct_interface_pointers;
	iuh->_fset_hyper_surf_color = f_fset_hyper_surf_color;
	iuh->_zoom_interface = f_zoom_interface;
	iuh->_reflect_point = f_reflect_point;
	iuh->_make_hypersurface = f_make_hypersurface;
	iuh->_user_copy_hyper_surf = f_user_copy_hyper_surf;
	iuh->_make_hypersurface_boundary = f_make_hypersurface_boundary;
	iuh->_make_node = f_make_node;
	iuh->_copy_node = f_copy_node;
	iuh->_delete_node = f_delete_node;
	iuh->_user_fprint_node = f_user_fprint_node;
	iuh->_user_read_node = f_user_read_node;
	iuh->_user_read_print_node = f_user_read_print_node;
	iuh->_make_curve = f_make_curve;
	iuh->_copy_curve = f_copy_curve;
	iuh->_delete_curve = f_delete_curve;
	iuh->_user_fprint_curve = f_user_fprint_curve;
	iuh->_user_read_curve = f_user_read_curve;
	iuh->_user_read_print_curve = f_user_read_print_curve;
	iuh->_user_split_curve = f_user_split_curve;
	iuh->_user_join_curves = f_user_join_curves;
	iuh->_insert_point_in_bond = f_insert_point_in_bond;
	iuh->_delete_start_of_bond = f_delete_start_of_bond;
	iuh->_delete_end_of_bond = f_delete_end_of_bond;
	iuh->_reconstruct_point_pointers = f_reconstruct_point_pointers;
	iuh->_reconstruct_node_pointers = f_reconstruct_node_pointers;
	iuh->_reconstruct_bond_pointers = f_reconstruct_bond_pointers;
	iuh->_reconstruct_curve_pointers = f_reconstruct_curve_pointers;
	iuh->_invert_curve = f_invert_curve;
	iuh->_reverse_curve = f_reverse_curve;
	iuh->_is_subdomain_boundary = f_is_subdomain_boundary;
	iuh->_cross_tolerance = f_cross_tolerance;
	iuh->_receive_interface = f_receive_interface;
	switch (dim)
	{
	case 1:
	    iuh->_make_point = f_make_point;
	    iuh->_set_boundary = f_set_boundary1d;
	    break;
	case 2:
	    iuh->_reflect_node = f_reflect_node2d;
	    iuh->_reflect_curve = f_reflect_curve2d;
	    iuh->_attach_curve_to_node = f_attach_curve_to_node;
	    iuh->_move_closed_loop_node = f_move_closed_loop_node;
	    iuh->_is_subdomain_node = f_is_subdomain_node;
	    iuh->_is_virtual_fixed_node = f_is_virtual_fixed_node;
	    iuh->_set_boundary = f_set_boundary2d;
	    break;
	case 3:
	    iuh->_reflect_surface = f_reflect_surface;
	    iuh->_CBond = f_CBond;
	    iuh->_insert_point_in_tri = f_insert_point_in_tri;
	    iuh->_insert_point_in_tri_side = f_insert_point_in_tri_side;
	    iuh->_link_tri_to_bond = f_link_tri_to_bond;
	    iuh->_reverse_bond = f_reverse_bond;
	    iuh->_reorder_curve_link_list = f_reorder_curve_link_list;
	    iuh->_join_surfaces = f_join_surfaces;
	    iuh->_make_surface = f_make_surface;
	    iuh->_copy_surface = f_copy_surface;
	    iuh->_delete_surface = f_delete_surface;
	    iuh->_user_fprint_surface = f_user_fprint_surface;
	    iuh->_user_read_surface = f_user_read_surface;
	    iuh->_user_read_print_surface = f_user_read_print_surface;
	    iuh->_user_install_faces = f_user_install_faces;
	    iuh->_assign_curve_boundary_flag = f_assign_curve_boundary_flag;
	    iuh->_assign_curve_boundary_type = f_assign_curve_boundary_type;
	    iuh->_set_boundary = f_set_boundary3d;
	    iuh->_gview_plot_interface = f_gview_plot_interface;
	    iuh->_consistent_interface = f_consistent_interface;
	    iuh->_sort_bond_tris = f_sort_bond_tris;
            /*#bjet2 */
	    iuh->_assign_btri_states = f_assign_btri_states;
	    iuh->_detach_one_surface = f_detach_one_surface;
	    break;
	}

	/* Allocated boundary state array */
	fuh->_num_bstates = 6;
	uni_array(&fuh->_bstates,fuh->_num_bstates+1,sizeof(BOUNDARY_STATE*));
	++fuh->_bstates;
	for (i = -1; i < fuh->_num_bstates; ++i)
	    fuh->_bstates[i] = NULL;
}		/*end f_set_interface_hooks*/


/*
*			f_reflect_state():
*
*	This is a no operation function to be used as a front based
*	default for the operation reflect_state().  It is appropriate
*	for any physics whose states have no vector quantities.
*/

/*ARGSUSED*/
LOCAL	void	f_reflect_state(
	Locstate	state,	/* state being reflected */
	INTERFACE	*intfc,	/* interface of point */
	double		*pt,	/* position of state being reflected */
	double		*p,	/* point on reflection plane */
	double		*n)	/* normal to plane */
{
}		/*end f_reflect_state*/

/*
*			f_fprint_intfc_state():
*
*	Default function for simple Locstate printing.  Assumes that
*	Locstate represents an array of floating point numbers.
*/

/*ARGSUSED*/
LOCAL	void	f_fprint_intfc_state(
	FILE		*file,   /* output file */
	Locstate	state,	/* state being printed */
	INTERFACE	*intfc)	/* interface of state */
{
	double		*st = (double*)state;
	size_t		i, nfloats = size_of_state(intfc)/sizeof(double);

	(void) fprintf(file,"State %p =",(POINTER)state);
	for (i = 0; i < nfloats; ++i)
		(void) fprintf(file," %-14g",st[i]);
}		/*end f_fprint_intfc_states*/


/*
*		linear_state_interpolator():
*
*	Default version of the state interpolator. Uses component-wise
*	linear interpolation on the Locstate's treated as arrays of
*	floats.
*/

/*ARGSUSED*/
EXPORT void linear_state_interpolator(
	double		alpha,
	double		beta,
	double		*crds1,
	Locstate	st1,
	double		*crds2,
	Locstate	st2,
	RECT_GRID	*gr,
	Locstate	answer)
{
	double	  *s1 = (double*)st1, *s2 = (double*)st2;
	double	  *ans = (double*)answer;
	size_t	  i, nfloats;
	INTERFACE *cur_intfc;

	cur_intfc = current_interface();
	nfloats = size_of_state(cur_intfc)/sizeof(double);

	for (i = 0; i < nfloats; ++i)
		ans[i] = alpha * s1[i] +  beta * s2[i];
}		/*end linear_state_interpolator*/




LOCAL	void	f_fprint_state_data(
	FILE		*file,
	Locstate	state,
	INTERFACE	*intfc)
{
	double	*st = (double*)state;
	size_t	i, nfloats;
	size_t	sizest = size_of_state(intfc);

	(void) fprintf(file,"State information for state %p -",(POINTER)state);
	if (is_binary_output() == YES)
	{
	    (void) fprintf(file,"\f%c",1);
	    (void) fwrite((const void *) state,sizest,1,file);
	}
	else
	{
	    nfloats = sizest/sizeof(double);
	    for (i = 0; i < nfloats; ++i)
	    	(void) fprintf(file," %-"FFMT,st[i]);
	}
	(void) fprintf(file,"\n");
}		/*end f_fprint_state_data*/

/*ARGSUSED*/
LOCAL	Locstate f_read_print_state_data(
	INIT_DATA     *init,
	const IO_TYPE *io_type,
	Locstate      state,
	INTERFACE     *intfc)
{
	FILE	*file = io_type->file;
	double	*x;
	int	c;
	size_t	i, nfloats;
	size_t	sizest = size_of_state(intfc);

	if (state == NULL)
	    state = (Locstate) store(sizest);
	x = (double *) state;
	(void) fgetstring(file,"State information for state");
	(void) fscanf(file,"%*s%*s");
	nfloats = sizest/sizeof(double);
	if ((c = getc(file)) == '\f')
	{
	    (void) getc(file);
	    (void) read_binary_real_array(x,nfloats,io_type);
	}
	else
	{
	    (void) ungetc(c,file);
	    for (i = 0; i < nfloats; ++i)
	    {
	    	(void) fscan_float(file,x+i);
	    }
	}
	return state;
}		/*end f_read_print_state_data*/


/*
*			f_tangent():
*
*	Finds the unit tangent vector to the curve c at the point p
*	on bond b.  The vector t points in the direction of c,
*	from start towards end.  The algorithm is to compute an effective
*	secant vector joining the opposite endpoints of the bonds meeting
*	at the point p.
*/

/*ARGSUSED*/
LOCAL	void	f_tangent(
	FT_POINT		*p,
	BOND		*b,
	CURVE		*c,
	double		*t,
	Front		*front)
{
	int		dim = front->interf->dim;
	FT_POINT		*p1, *p2;
	double		length;
	int		i;
	static	BOND  	*bdir1 = NULL, *bdir2 = NULL;
	/*NOTE: the following variables are newly added */
	BOND		*b1,*b2;	
	double		dp1,dp2,l1,l2,t1[MAXD],t2[MAXD];

	if (dim == 1)
	{
	    t[0] = 1.0;
	    return;
	} 

	if (bdir2 == NULL)
	{
	    scalar(&bdir1,sizeof(BOND));
	    bdir1->start = Static_point(front->interf);
	    bdir1->end = Static_point(front->interf);
	    scalar(&bdir2,sizeof(BOND));
	    bdir2->start = Static_point(front->interf);
	    bdir2->end = Static_point(front->interf);
	}
	if (is_bdry(c)) /*Rectangular boundary*/
	{
	    for (i = 0; i < dim; ++i)
	        t[i] = Coords(c->end->posn)[i] - Coords(c->start->posn)[i];
	    length = mag_vector(t,dim);
	    for (i = 0; i < dim; ++i)
	        t[i] /= length;
	    return;
	}
	if (p == c->end->posn)             /* End Node */
	{
	    if (is_closed_curve(c))
	    {
	        bond_tangent_to_curve(p,c->last,c,NEGATIVE_ORIENTATION,
				      bdir1,front);
	        p1 = bdir1->start;
		b1 = c->last;
	        bond_tangent_to_curve(p,c->first,c,POSITIVE_ORIENTATION,
				      bdir2,front);
	        p2 = bdir2->end;
		b2 = c->first;
	    }
	    else
	    {
	        for (i = 0; i < dim; ++i)
	        {
	            t[i] = (Coords(c->last->end)[i] - 
		    		Coords(c->last->start)[i])
				/bond_length(c->last);
	        }
	        if ((dim == 2)
		    &&
		    (wave_type(c) >= FIRST_PHYSICS_WAVE_TYPE)
	            &&
		    (is_bdry_like_node(c->end) || is_fixed_node(c->end))
	            &&
		    (front->impose_bc != NULL))
	        {
	            (*front->impose_bc)(p,b,c,t,front,YES,YES);
	        }
	        return;
	    }
	}
	else if (p == c->start->posn)             /* Start Node */
	{
	    if (is_closed_curve(c))
	    {
	        bond_tangent_to_curve(p,c->last,c,NEGATIVE_ORIENTATION,
	                              bdir1,front);
	        p1 = bdir1->start;
		b1 = c->last;
	        bond_tangent_to_curve(p,c->first,c,POSITIVE_ORIENTATION,
	                              bdir2,front);
	        p2 = bdir2->end;
		b2 = c->first;
	    }
	    else
	    {
	        for (i = 0; i < dim; ++i)
	        {
	            t[i] = (Coords(c->first->end)[i] - 
		    		Coords(c->first->start)[i])
				/bond_length(c->first);
	        }
	        if ((dim == 2)
		    &&
		    (wave_type(c) >= FIRST_PHYSICS_WAVE_TYPE)
		    &&
		    (is_bdry_like_node(c->start) || is_fixed_node(c->start))
		    &&
		    (front->impose_bc != NULL))
	        {
	            (*front->impose_bc)(p,b,c,t,front,YES,YES);
	        }
	        return;
	    }
	}
	else if (p == b->end)
	{
	    bond_tangent_to_curve(p,b,c,NEGATIVE_ORIENTATION,bdir1,front);
	    p1 = bdir1->start;
	    b1 = b;
	    bond_tangent_to_curve(p,b->next,c,POSITIVE_ORIENTATION,bdir2,front);
	    p2 = bdir2->end;
	    b2 = b->next;
	}
	else if (p == b->start)
	{
	    bond_tangent_to_curve(p,b->prev,c,NEGATIVE_ORIENTATION,bdir1,front);
	    p1 = bdir1->start;
	    b1 = b->prev;
	    bond_tangent_to_curve(p,b,c,POSITIVE_ORIENTATION,bdir2,front);
	    p2 = bdir2->end;
	    b2 = b;
	}
	else
	{
	    double t0[3], t1[3];
	    double dp[3], db[3];
	    double alpha;

	    /*
	     * Calling routine has faked a point. Define the tangent field
	     * along the bond so that the tangent varies continuously from the
	     * bond start to bond end.
	     */

	    tangent(b->start,b,c,t0,front);
	    tangent(b->end,b,c,t1,front);
	    p1 = b->start;
	    p2 = b->end;
	    for (i = 0; i < dim; ++i)
	    {
	        db[i] = Coords(p2)[i] - Coords(p1)[i];
	        dp[i] = Coords(p)[i] - Coords(p1)[i];
	    }
	    length = mag_vector(db,dim);
	    alpha = scalar_product(dp,db,dim)/sqr(length);
	    alpha = max(0.0,alpha);
	    alpha = min(1.0,alpha);
	    for (i = 0; i < dim; ++i)
	        t[i] = (1.0 - alpha)*t0[i] + alpha*t1[i];
	    length = mag_vector(t,dim);
	    for (i = 0; i < dim; ++i)
	        t[i] /= length;
	    return;
	}

	for (i = 0; i < dim; ++i)
	    t[i] = Coords(p2)[i] - Coords(p1)[i];
	/*NOTE: subject to critique */
	dp1 = (Coords(p)[0] - Coords(b1->start)[0])*t[0] +
	      (Coords(p)[1] - Coords(b1->start)[1])*t[1];
	dp2 = (Coords(b2->end)[0] - Coords(p)[0])*t[0] +
	      (Coords(b2->end)[1] - Coords(p)[1])*t[1];
	/*TMP*/
	if (dp1 < 0.0 || dp2 < 0.0)
	{
	    for (i = 0; i < dim; i++)
	    {
		t1[i] = Coords(b2->end)[i] - Coords(p)[i];
		t2[i] = Coords(p)[i] - Coords(b1->start)[i];
	    }
	    l1 = mag_vector(t1,dim);
	    l2 = mag_vector(t2,dim);
	    if (l1 != 0.0 && l2 != 0.0)
	    {
	    	for (i = 0; i < dim; i++)
		    t[i] = t1[i]/l1 + t2[i]/l2;
	    }
	    else if (l1 != 0.0) 
	    {
	    	for (i = 0; i < dim; i++)
		    t[i] = t1[i]/l1;
	    }
	    else if (l2 != 0.0)
	    {
	    	for (i = 0; i < dim; i++)
		    t[i] = t2[i]/l2;
	    }
	}
	/*NOTE: end of newly added */
	length = mag_vector(t,dim);
	/*On very small, closed loops it can happen that p1 and p2 are
	          at the same location.*/
	if (length == 0.0)
	{
	    p1 = b->start;          p2 = b->end;
	    for (i = 0; i < dim; ++i)
	        t[i] = Coords(p2)[i] - Coords(p1)[i];
	    length = mag_vector(t,dim);
	}
	for (i = 0; i < dim; ++i)
	    t[i] /= length;
}		/*end f_tangent*/
	
LOCAL	double	f_cross_tolerance(
	INTERFACE *intfc)
{
	double *h = computational_grid(intfc)->h;
	double hmin;
	hmin = min(h[0],h[1]);
	hmin = min(hmin,h[2]);
	return MIN_SIN_SQR(intfc)*hmin;/*TOLERANCE*/
}		/*end f_cross_tolerance*/

/*
*		linear_tri_state_interpolator():
*
*	Default version of the tri state interpolator.
*	Uses component-wise linear interpolation on the Locstate's
*	treated as arrays of floats.
*/

/*ARGSUSED*/
EXPORT boolean linear_tri_state_interpolator(
	double		alpha,
	double		beta,
	double		gamma,
	double		*crds0,
	Locstate	st0,
	double		*crds1,
	Locstate	st1,
	double		*crds2,
	Locstate	st2,
	RECT_GRID	*gr,
	Locstate	answer)
{
	double	  *s0 = (double*)st0, *s1 = (double*)st1, *s2 = (double*)st2;
	double	  *ans = (double*)answer;
	size_t	  i, nfloats;
	INTERFACE *cur_intfc;

	cur_intfc = current_interface();
	nfloats = size_of_state(cur_intfc)/sizeof(double);

	for (i = 0; i < nfloats; ++i)
		ans[i] = alpha * s0[i] + beta * s1[i] + gamma * s2[i];
	return FUNCTION_SUCCEEDED;
}		/*end linear_tri_state_interpolator*/


/*
*		f_fshow_intfc_states1d():
*
*	Prints states on an interface using print_intfc_state().
*/


/*ARGSUSED*/
LOCAL	void f_fshow_intfc_states1d(
	FILE		*file,
	INTERFACE	*intfc)
{
	FT_POINT	**p;

	(void) fprintf(file,"\t\tSTATES ON THE FRONT\n\n");
	if (size_of_state(intfc) > 0)
	{
		for (p = intfc->points; p && *p;  ++p)
		{
			(void) fprintf(file,"\t\tSTATES AT POINT %g\n\n",
				Coords(*p)[0]);
			(void) fprintf(file,"\t\tl_st ");
			fprint_intfc_state(file,left_state(*p),intfc);
			(void) fprintf(file,"\t\tr_st ");
			fprint_intfc_state(file,right_state(*p),intfc);
		}
		
	}
	else
	{
		(void) fprintf(file,"No states ft_assigned on intfc\n");
		return;
	}
	(void) fprintf(file,"\n\n");
	(void) fprintf(file,"\t\tEND OF STATES ON THE FRONT\n\n");
}		/*end f_fshow_intfc_states1d*/

/*ARGSUSED*/
LOCAL	void	slsr1d(
	FT_POINT		*p,
	HYPER_SURF_ELEMENT* hse,
	HYPER_SURF	*hs,
	Locstate	*sl,
	Locstate	*sr)
{
	*sl = left_state(p);
	*sr = right_state(p);
}		/*end slsr1d*/

/*
*			state_at_point():
*
*	Returns the state matching component at a point on a 1d interface.
*/

/*ARGSUSED*/
LOCAL	void state_at_point(
	COMPONENT	   comp,
	double		   *t,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF	   *hs,
	Locstate	   state)
{
	FT_POINT	*p = Point_of_hs(hs);
	size_t	sizest = f_user_interface(hs->interface)._sizest;

	if (comp == negative_component(p))
	{
	    ft_assign(state,left_state(p),sizest);
	}
	else if (comp == positive_component(p))
	{
	    ft_assign(state,right_state(p),sizest);
	}
	else
	{
	    screen("ERROR in state_at_point(), "
	           "comp = %d not on point %d\n",comp,p);
	    (void) printf("Point p\n");
	    print_point(p);
	    (void) printf("Interface of p\n");
	    print_interface(hs->interface);
	    clean_up(ERROR);
	}
}		/*end state_at_point*/

/*
*				normal1d():
*
*	Returns the Normal to the hypersurface hs at the point p on
*	hypersurface element hse of CURVE c.  The normal points from the
*	negative side to the positive side of the hypersurface.  In one
*	space dimensions this means it points from the LEFT side TO the
*	RIGHT side.
*/

/*ARGSUSED*/
EXPORT	void normal1d(
	FT_POINT		*p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF	*hs,
	double		*nor,
	Front		*front)
{
	nor[0] = 1.0;
}		/*end normal1d*/



EXPORT	void	set_tangent_operator(
	TANGENT_METHOD tan_meth,
	int            dim)
{
	F_USER_INTERFACE *fuh = f_user_hook(dim);
	switch (tan_meth)
	{
	case LANGRANGIAN_INTERPOLANT:
	    fuh->_interface_tangent_function._tangent = f_lagrangian_tangent;
	    fuh->_interface_tangent_function._tangent_name =
	        strdup("f_lagrangian_tangent");
	    break;
	case TANGENT_METHOD_FROM_RESTART:
	    break;
	case LINEAR_SECANT:
	    fuh->_interface_tangent_function._tangent = f_tangent;
	    fuh->_interface_tangent_function._tangent_name =
	        strdup("f_tangent");
	    break;
	case WLSP_TANGENT:
	    fuh->_interface_tangent_function._tangent = f_wlsp_tangent;
	    fuh->_interface_tangent_function._tangent_name =
	        strdup("f_wlsp_tangent");
	    break;
	default:
	    break;
	}
}		/*end set_tangent_operator*/

/*ARGSUSED*/
EXPORT	void	f_set_normal_function(
	const char      *s,
	NORMAL_FUNCTION *nf,
	INTERFACE       *intfc)
{
	if (strcmp(s,"none") == 0)
	{
	    nf->_normal = NULL;
	    nf->_normal_name = NULL;
	}
	else if (strstr(s,"normal1d"))
	{
	    nf->_normal = normal1d;
	    nf->_normal_name = strdup("normal1d");
	}
	else if (strstr(s,"first_order_normal2d"))
	{
	    nf->_normal = first_order_normal2d;
	    nf->_normal_name = strdup("first_order_normal2d");
	}
	else if (strstr(s,"wlsp_normal2d"))
	{
	    nf->_normal = f_wlsp_normal;
	    nf->_normal_name = strdup("f_wlsp_normal");
	}
	else if (strstr(s,"area_weighted_normal3d"))
	{
	    nf->_normal = f_area_weighted_normal3d;
	    nf->_normal_name = strdup("f_area_weighted_normal3d");
	}
	else if (strstr(s,"plane_fit_normal3d"))
	{
	    nf->_normal = f_plane_fit_normal3d;
	    nf->_normal_name = strdup("f_plane_fit_normal3d");
	}
	else if (strstr(s,"sine_weighted_normal3d"))
        {
            nf->_normal = f_sine_weighted_normal3d;
            nf->_normal_name = strdup("f_sine_weighted_normal3d");
        }
	else if (strstr(s,"wlsp_normal3d"))
        {
            nf->_normal = f_wlsp_normal;
            nf->_normal_name = strdup("f_wlsp_normal");
        }
	else
	{
	    screen("ERROR in f_set_normal_function(), unknown normal function "
		   "%s\n",s);
	    clean_up(ERROR);
	}
}		/*end f_set_normal_function*/

/*ARGSUSED*/
EXPORT	void	f_set_tangent_function(
	const char       *s,
	TANGENT_FUNCTION *tf,
	INTERFACE        *intfc)
{
	if (strcmp(s,"none") == 0)
	{
	    tf->_tangent = NULL;
	    tf->_tangent_name = NULL;
	}
	else if (strcmp(s,"f_lagrangian_tangent") == 0)
	{
	    tf->_tangent = f_lagrangian_tangent;
	    tf->_tangent_name = strdup("f_lagrangian_tangent");
	}
	else if (strcmp(s,"f_tangent") == 0)
	{
	    tf->_tangent = f_tangent;
	    tf->_tangent_name = strdup("f_tangent");
	}
	else if (strcmp(s,"f_wlsp_tangent") == 0)
	{
	    tf->_tangent = f_wlsp_tangent;
	    tf->_tangent_name = strdup("f_wlsp_tangent");
	}
}		/*end f_set_tangent_function*/

/*
*			f_lagrangian_tangent():
*
*	Finds unit the tangent vector to the curve c at the point p
*	on bond b.  The vector t points in the direction of c,
*	from start towards end.  The algorithm is to fit a fourth order
*	lagrangian interpolation polynomial to five points centered at p.
*	The tangent is defined as the unit tangent vector of this interpolant
*	at p.
*/

/*ARGSUSED*/
LOCAL	void	f_lagrangian_tangent(
	FT_POINT		*p,
	BOND		*b,
	CURVE		*c,
	double		*t,
	Front		*front)
{
	int             dim = front->interf->dim;
	BOND		*bonds[4];
	double		length;
	double		s[4];
	int		is,ie;
	int             i, j;
	static	double	**tt = NULL;

	if (tt == NULL)
	{
	    bi_array(&tt,MAXD,4,FLOAT);
	}
	if (is_bdry(c)) /*Rectangular boundary*/
	{
	    for (i = 0; i < dim; ++i)
	        t[i] = Coords(c->end->posn)[i] - Coords(c->start->posn)[i];
	    length = mag_vector(t,dim);
	    for (i = 0; i < dim; ++i)
	        t[i] /= length;
	    return;
	}
	is = 0;		ie = 4;
	if (p == b->start)
	{
	    if (b != c->first)
	    {
	        bonds[1] = b->prev;
	        s[1] = -0.5*bond_length(bonds[1]);
	        if (bonds[1] != c->first)
	        {
	            bonds[0] = bonds[1]->prev;
	            s[0] = -bond_length(bonds[1]) - 0.5*bond_length(bonds[0]);
	        }
	        else
	        {
	            bonds[0] = NULL;
	            is = 1;
	        }
	    }
	    else if (is_closed_curve(c))
	    {
	        bonds[1] = c->last;
	        s[1] = -0.5*bond_length(bonds[1]);
	        if (bonds[1] != c->first)
	        {
	            bonds[0] = bonds[1]->prev;
	            s[0] = -bond_length(bonds[1]) - 0.5*bond_length(bonds[0]);
	        }
	        else
	        {
	            bonds[0] = NULL;
	            is = 1;
	        }
	    }
	    else
	    {
	        bonds[0] = bonds[1] = NULL;
	        is = 2;
	    }
	    bonds[2] = b;
	    s[2] = 0.5*bond_length(bonds[2]);
	    if (b != c->last)
	    {
	        bonds[3] = b->next;
	        s[3] = bond_length(bonds[2]) + 0.5*bond_length(bonds[3]);
	    }
	    else if (is_closed_curve(c))
	    {
	        bonds[3] = c->first;
	        s[3] = bond_length(bonds[2]) + 0.5*bond_length(bonds[3]);
	    }
	    else 
	    {
	        bonds[3] = NULL;
	        ie = 3;
	    }
	}
	else if (p == b->end)
	{
	    bonds[1] = b;
	    s[1] = -0.5*bond_length(bonds[1]);
	    if (b != c->first)
	    {
	        bonds[0] = b->prev;
	        s[0] = -bond_length(bonds[1]) - 0.5*bond_length(bonds[0]);
	    }
	    else if (is_closed_curve(c))
	    {
	        bonds[0] = c->last;
	        s[0] = -bond_length(bonds[1]) - 0.5*bond_length(bonds[0]);
	    }
	    else 
	    {
	        bonds[0] = NULL;
	        is = 1;
	    }
	    if (b != c->last)
	    {
	        bonds[2] = b->next;
	    	s[2] = 0.5*bond_length(bonds[2]);
	        if (bonds[2] != c->last)
	        {
	            bonds[3] = bonds[2]->next;
	            s[3] = bond_length(bonds[2]) + 0.5*bond_length(bonds[3]);
	        }
	        else if (is_closed_curve(c))
	        {
	            bonds[3] = c->first;
	            s[3] = bond_length(bonds[2]) + 0.5*bond_length(bonds[3]);
	        }
	        else
	        {
	            bonds[3] = NULL;
	            ie = 3;
	        }
	    }
	    else if (is_closed_curve(c))
	    {
	        bonds[2] = c->first;
	    	s[2] = 0.5*bond_length(bonds[2]);
	        if (bonds[2] != c->last)
	        {
	            bonds[3] = bonds[2]->next;
	            s[3] = bond_length(bonds[2]) + 0.5*bond_length(bonds[3]);
	        }
	        else
	        {
	            bonds[3] = NULL;
	            ie = 3;
	        }
	    }
	    else
	    {
	        bonds[2] = bonds[3] = NULL;
	        ie = 2;
	    }
	}
	for (i = 0; i < 4; ++i)
	{
	    if (bonds[i] == NULL)
	        continue;
	    else
	    {
	        for (j = 0; j < dim; ++j)
	        {
	            tt[j][i] = (Coords(bonds[i]->start)[j] - 
	        	   Coords(bonds[i]->end)[j])/bond_length(bonds[i]);
	        }
	    }
	}
	for (i = 0; i < dim; ++i)
	{
	    t[i] = lagrangian_n_pt(ie-is,0.0,s+is,tt[i]+is);
	}
	length = mag_vector(t,dim);
	for (i = 0; i < dim; ++i)
	    t[i] /= length;
	if (debugging("tangent"))
	{
	    double t1[3];
	    double x, y, v[3];
	    f_tangent(p,b,c,t1,front);
	    x = scalar_product(t,t1,dim);
	    y = vector_product(t,t1,v,dim); 
	    (void) printf("angle between f_tangent and "
	        	  "f_lagrangian_tangent = %g degrees, sin = %g\n",
	        	  degrees(atan2(y,x)),y);
	}
}		/*end f_lagrangian_tangent*/

LOCAL   double lagrangian_n_pt(
        int n,
        double xx,
        double *x,
        double *f)
{
        int i, j;
        double dd, soln;

        soln = 0.0;
        for (i = 0; i < n; ++i)
        {
            dd = 1.0;
            for (j = 0; j < n; ++j)
            {
                if (j == i)
	            continue;
                dd *= (xx - x[j])/(x[i] - x[j]);
            }
            soln += f[i]*dd;
        }
        return -soln;
}       /* end lagrangian_n_pt */

/*
*				f_spline_tangent():
*
*	Finds unit the tangent vector to the curve c at the point p
*	on bond b.  The vector t points in the direction of c,
*	from start towards end.  The algorithm is to fit a cubic spline
*	interpolant through the point and adjacent points on the curve.
*	The end point conditions are that the spline terms are quadratic
*	at the two extreme sections.
*/

/*ARGSUSED*/

/*
*		f_fshow_intfc_states2d():
*
*	Prints states on an interface using print_intfc_state().
*/

LOCAL	void f_fshow_intfc_states2d(
	FILE		*file,
	INTERFACE	*intfc)
{
	CURVE		**c;

	(void) fprintf(file,"\t\tSTATES ON THE FRONT\n\n");
	if (size_of_state(intfc) > 0)
	{
	    for (c = intfc->curves; c && *c;  ++c)
	    	fshow_curve_states(file,*c);
	}
	else
	{
	    (void) fprintf(file,"No states ft_assigned on intfc\n");
	    return;
	}
	(void) fprintf(file,"\n\n");
	(void) fprintf(file,"\t\tEND OF STATES ON THE FRONT\n\n");
}		/*end f_fshow_intfc_states2d*/

LOCAL	void state_along_bond(
	COMPONENT	   comp,
	double		   *t,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF	   *hs,
	Locstate	   state)
{
	CURVE		*c = Curve_of_hs(hs);
	BOND		*b = Bond_of_hse(hse);
	Locstate	start_state, end_state;

	if (comp == negative_component(c))
	{
	    start_state = (b == c->first) ?
			      left_start_state(c) : left_state(b->start);
	    end_state = (b == c->last) ?
			      left_end_state(c) : left_state(b->end);
	}
	else if (comp == positive_component(c))
	{
	    start_state = (b == c->first) ?
			      right_start_state(c) : right_state(b->start);
	    end_state = (b == c->last) ?
			      right_end_state(c) : right_state(b->end);
	}
	else
	{
	    screen("ERROR in state_along_bond(), comp = %d not on curve %d\n",
		   comp,c);
	    print_curve(c);
	    clean_up(ERROR);
	}

	bi_interpolate_intfc_states(c->interface,1.0-t[0],t[0],Coords(b->start),
		                    start_state,Coords(b->end),end_state,state);
}		/*end state_along_bond*/

LOCAL	void	slsr2d(
	FT_POINT		*p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF	*hs,
	Locstate	*sl,
	Locstate	*sr)
{
	BOND *b = Bond_of_hse(hse);
	CURVE *c = Curve_of_hs(hs);

	if (p == c->start->posn && b == c->first)
	{
	    *sl = left_start_state(c);
	    *sr = right_start_state(c);
	}
	else if (p == c->end->posn && b == c->last)
	{
	    *sl = left_end_state(c);
	    *sr = right_end_state(c);
	}
	else
	{
	    *sl = left_state(p);
	    *sr = right_state(p);
	}
}		/*end slsr2d*/

LOCAL boolean f_set_boundary1d(
        INTERFACE       *intfc,
        RECT_GRID       *gr,
        COMPONENT       default_comp,
        double           eps)
{
        boolean            status;
        boolean            sav_copy = copy_intfc_states();
        FT_POINT           **p;

        set_copy_intfc_states(NO);
        status = i_set_boundary1d(intfc,gr,default_comp,eps);
        for (p = intfc->points; p && *p; ++p)
        {
            if (!is_bdry(*p)) continue;
            assign_boundary_point_wave_type(*p,intfc,gr,eps);
        }
        set_copy_intfc_states(sav_copy);
        return status;
}               /*end f_set_boundary1d*/

LOCAL boolean f_set_boundary2d(
	INTERFACE	*intfc,
	RECT_GRID	*gr,
	COMPONENT	default_comp,
	double		eps)
{
	boolean		status;
	boolean		sav_copy = copy_intfc_states();
	NODE		**n;
	CURVE		**c;

	set_copy_intfc_states(NO);
	status = i_set_boundary2d(intfc,gr,default_comp,eps);
	for (c = intfc->curves; c && *c; ++c)
	{
	    if (!is_bdry(*c)) continue;
	    assign_boundary_curve_wave_type(*c,intfc,gr,eps);
	}
	for (n = intfc->nodes; n && *n; ++n)
	{
	    if (!is_bdry(*n)) continue;
	    assign_boundary_node_type(*n,intfc,gr,eps);
	}
	set_copy_intfc_states(sav_copy);
	return status;
}		/*end f_set_boundary2d*/

LOCAL void assign_boundary_node_type(
	NODE *node,
	INTERFACE *intfc,
	RECT_GRID *gr,
	double eps)
{
	double *L = gr->VL;
	double *U = gr->VU;
	int i,j;
	int idir,nb;

	i = j = -1;

	if (fabs(Coords(node->posn)[0] - L[0]) < eps)
	    i = 0;
	else if (fabs(Coords(node->posn)[0] - U[0]) < eps)
	    i = 1;
	if (fabs(Coords(node->posn)[1] - L[1]) < eps)
	    j = 0;
	else if (fabs(Coords(node->posn)[1] - U[1]) < eps)
	    j = 1;

	if (i == -1 && j == -1) /* not at rectangular boundary */
	    return;
	else if (i != -1 && j != -1) /* corner node */
	{
	    node_type(node) = FIXED_NODE;
	    return;
	}
	else if (i != -1)
	{
	    idir = 0;
	    nb = i;
	}
	else if (j != -1)
	{
	    idir = 1;
	    nb = j;
	}
	switch (rect_boundary_type(intfc,idir,nb))
	{
	case SUBDOMAIN_BOUNDARY:
	case REFLECTION_BOUNDARY: 
	    node_type(node) = SUBDOMAIN_NODE;
	    return;
	case DIRICHLET_BOUNDARY:
	    node_type(node) = DIRICHLET_NODE;
	    return;
	case NEUMANN_BOUNDARY:
	    node_type(node) = NEUMANN_NODE;
	    return;
	}
}	/* end assign_boundary_node_type */

LOCAL void assign_boundary_point_wave_type(
        FT_POINT *point,
        INTERFACE *intfc,
        RECT_GRID *gr,
        double eps)
{
        double *L = gr->L;
        double *U = gr->U;
        int idir,nb;

        if (fabs(Coords(point)[0] - L[0]) < eps)
        {
            idir = 0;   nb = 0;
        }
        else if (fabs(Coords(point)[0] - U[0]) < eps)
        {
            idir = 0;   nb = 1;
        }
        else return;

        switch (rect_boundary_type(intfc,idir,nb))
        {
        case SUBDOMAIN_BOUNDARY:
        case REFLECTION_BOUNDARY:
            wave_type(point) = SUBDOMAIN_BOUNDARY;
            return;
        case DIRICHLET_BOUNDARY:
            wave_type(point) = DIRICHLET_BOUNDARY;
            return;
        case NEUMANN_BOUNDARY:
            wave_type(point) = NEUMANN_BOUNDARY;
            return;
        }
}       /* end assign_boundary_point_wave_type */

LOCAL void assign_boundary_curve_wave_type(
	CURVE *curve,
	INTERFACE *intfc,
	RECT_GRID *gr,
	double eps)
{
	double *L = gr->L;
	double *U = gr->U;
	NODE *ns = curve->start;
	NODE *ne = curve->end;
	int idir,nb;

	if (fabs(Coords(ns->posn)[0] - L[0]) < eps &&
	    fabs(Coords(ne->posn)[0] - L[0]) < eps)
	{
	    idir = 0;	nb = 0;
	}
	else if (fabs(Coords(ns->posn)[0] - U[0]) < eps &&
	    fabs(Coords(ne->posn)[0] - U[0]) < eps)
	{
	    idir = 0;	nb = 1;
	}
	else if (fabs(Coords(ns->posn)[1] - L[1]) < eps &&
	    fabs(Coords(ne->posn)[1] - L[1]) < eps)
	{
	    idir = 1;	nb = 0;
	}
	else if (fabs(Coords(ns->posn)[1] - U[1]) < eps &&
	    fabs(Coords(ne->posn)[1] - U[1]) < eps)
	{
	    idir = 1;	nb = 1;
	}
	else return;

	switch (rect_boundary_type(intfc,idir,nb))
	{
	case SUBDOMAIN_BOUNDARY:
	case REFLECTION_BOUNDARY: 
	    wave_type(curve) = SUBDOMAIN_BOUNDARY;
	    return;
	case DIRICHLET_BOUNDARY:
	    wave_type(curve) = DIRICHLET_BOUNDARY;
	    return;
	case NEUMANN_BOUNDARY:
	    wave_type(curve) = NEUMANN_BOUNDARY;
	    return;
	}
}	/* end assign_boundary_curve_wave_type */

/*ARGSUSED*/
/*
*				first_order_normal2d():
*
*	Returns the Normal to the hypersurface hs at the point p on
*	hypersurface element hse of CURVE c.  The normal points from the
*	negative side to the positive side of the hypersurface.  In two
*	space dimensions this means it points from the LEFT=NEGATIVE_SIDE
*       side TO the RIGHT=POSITIVE_SIDE side.
*
*	Care is taken to handle the hypersurface boundaries correctly.
*	In case the hypersurface is a curve c that is closed
*	and the point in question is an endpoint,
*	then the normal is computed as if the point was an interior
*	point of the CURVE.
*/

LOCAL	void first_order_normal2d(
	FT_POINT		   *p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF	   *hs,
	double		   *nor,
	Front		   *front)
{
	double		t[MAXD];
	BOND		*bprev,*bnext;

	tangent(p,Bond_of_hse(hse),Curve_of_hs(hs),t,front);
	nor[0] = t[1];
	nor[1] = -t[0];
}		/*end first_order_normal2d*/


LOCAL	void f_normal2d(
	FT_POINT		   *p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF	   *hs,
	double		   *nor,
	Front		   *front)
{
	double		t[MAXD];
	BOND		*bprev,*bnext;

	f_tangent(p,Bond_of_hse(hse),Curve_of_hs(hs),t,front);
	nor[0] = t[1];
	nor[1] = -t[0];
}		/*end f_normal2d*/


/*
*		f_fshow_intfc_states3d():
*
*	Prints states on an interface using print_intfc_state().
*/

/*ARGSUSED*/
LOCAL	void f_fshow_intfc_states3d(
	FILE		*file,
	INTERFACE	*intfc)
{
	SURFACE		**s;

	(void) fprintf(file,"\t\tSTATES ON THE FRONT\n\n");
	if (size_of_state(intfc) > 0)
	{
	    for (s = intfc->surfaces; s && *s;  ++s)
	    	fshow_surface_states(file,*s);
	}
	else
	{
	    (void) fprintf(file,"No states ft_assigned on intfc\n");
	    return;
	}
	(void) fprintf(file,"\n\n");
	(void) fprintf(file,"\t\tEND OF STATES ON THE FRONT\n\n");
}		/*end f_fshow_intfc_states3d*/

LOCAL	void state_in_tri(
	COMPONENT	   comp,
	double		   *t,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF	   *hs,
	Locstate	   state)
{
	TRI		*tri = Tri_of_hse(hse);
	Locstate	sl, sr, s[3];
	FT_POINT		*p;
	int		i;

	for (i = 0; i < 3; ++i)
	{
	    p = Point_of_tri(tri)[i];
            slsr(p,hse,hs,&sl,&sr);
            s[i] = (comp == negative_component(hs)) ? sl : sr;
	}

        if (s[0] == NULL || s[1] == NULL || s[2] == NULL)
	{
	    screen("ERROR in state_in_tri(), NULL state found"
	           "s0= %p, s1= %p, s2= %p\n",
		   (POINTER)s[0],(POINTER)s[1],(POINTER)s[2]);
	    print_tri(tri,hs->interface);
	    clean_up(ERROR);
	}

	if (tri_interpolate_intfc_states(hs->interface,t[0],t[1],t[2],
		                         Coords(Point_of_tri(tri)[0]),s[0],
					 Coords(Point_of_tri(tri)[1]),s[1],
		                         Coords(Point_of_tri(tri)[2]),s[2],
					 state) != FUNCTION_SUCCEEDED)
	{
	    screen("ERROR in state_in_tri(), "
		   "tri_interpolate_intfc_states failed\n");
	    (void) printf("comp = %d\n",comp);
	    print_tri(tri,hs->interface);
	    print_tri_states(tri,hs);
	    print_hypersurface(hs);
	    print_interface(hs->interface);
	    gview_plot_interface("ERROR_in_state_in_tri",hs->interface);
	    clean_up(ERROR);
	}

	if(debugging("line_tri"))
	{
	    printf("#state_in_tri, tri states\n");
	    print_tri_states(tri,hs);
	}
}		/*end state_in_tri*/

LOCAL boolean f_set_boundary3d(
	INTERFACE	*intfc,
	RECT_GRID	*gr,
	COMPONENT	default_comp,
	double		eps)
{
	boolean		status;
	boolean		sav_copy = copy_intfc_states();

	set_copy_intfc_states(NO);
	status = i_set_boundary3d(intfc,gr,default_comp,eps);
	set_copy_intfc_states(sav_copy);
	return status;
}		/*end f_set_boundary3d*/

LOCAL void slsr3d_error_info(
	FT_POINT		   *p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF	   *hs,
	TRI		   *tri,
	TRI		   **tris,
	int		   ntris)
{
int i;

        (void) printf("p(%llu) = ",point_number(p));
        print_general_vector("",Coords(p),3,"\n");
        (void) printf("Boundary(p) = %d\n",Boundary(p));
        (void) printf("Boundary_point(p) = %d\n",Boundary_point(p));
        (void) printf("\nInput triangle\n");
        print_tri(Tri_of_hse(hse),hs->interface);
        
	if(tri == NULL)
	    return;

	(void) printf("\nBoundary triangle\n");
        (void) printf("Current tri\n");
        print_tri(tri,hs->interface);
        if (!is_side01_a_bond(tri))
        {
            (void) printf("Tri on side 01\n");
            print_tri(Tri_on_side01(tri),hs->interface);
        }
        if (!is_side12_a_bond(tri))
        {
	    (void) printf("Tri on side 12\n");
	    print_tri(Tri_on_side12(tri),hs->interface);
        }
        if (!is_side20_a_bond(tri))
        {
	    (void) printf("Tri on side 20\n");
	    print_tri(Tri_on_side20(tri),hs->interface);
        }
	(void) printf("\nadjacent tris\n");
	for (i = 0; i < ntris; ++i)
	{
	    (void) printf("tris[%d] - ",i);
	    print_tri(tris[i],hs->interface);
	}
}

LOCAL boolean  state_on_bdry_point(
	Locstate     *sl,
	Locstate     *sr,
	FT_POINT	     *p,
	TRI	     *tri,
	HYPER_SURF   *hs)
{
	int	 vertex, pside, nside;
	BOND	 *b;
	BOND_TRI *btri;

	if(!Boundary_point(p))
	    return NO;

	if((vertex = Vertex_of_point(tri,p)) == ERROR)
	{
	    printf("ERROR in state_on_bdry_point, "
		   "the point is not on the tri\n");
	    slsr3d_error_info(p,(HYPER_SURF_ELEMENT*)tri,hs,NULL,NULL,0);
	    clean_up(ERROR);
	}
	
	nside = vertex;
	pside = Prev_m3(vertex);

	btri = NULL;
	if (is_side_bdry(tri,nside))
	    btri = Bond_tri_on_side(tri,nside);
	else if (is_side_bdry(tri,pside))
	    btri = Bond_tri_on_side(tri,pside);

	if(btri == NULL)
	    return NO;
	
	b = btri->bond;
	if (p == b->start)
	{
	    *sl = left_start_btri_state(btri);
	    *sr = right_start_btri_state(btri);
	}
	else if (p == b->end)
	{
	    *sl = left_end_btri_state(btri);
	    *sr = right_end_btri_state(btri);
	}
	else
	{
	    printf("ERROR in state_on_bdry_point, "
	           "point on bond is inconsistent with point on tri.\n");
	    print_bond(b);
	    printf("p(%llu) = ",point_number(p));
	    print_tri(tri, hs->interface);
	    clean_up(ERROR);
	}

	return YES;
}

/*ARGSUSED*/
LOCAL	void	slsr3d(
	FT_POINT		   *p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF	   *hs,
	Locstate	   *sl,
	Locstate	   *sr)
{
	TRI	 *tri = Tri_of_hse(hse);
	TRI      **tris;
	int	 ntris;

        if (!Boundary_point(p))
	{ 
	    *sl = left_state(p);
	    *sr = right_state(p);
	    return;
	} 

	if(!state_on_bdry_point(sl, sr, p, tri, hs))
	{
	    /*according to the alg. in set_tri_list_around_point,  */
	    /*the first and the last tris are boundary tris. */
	    ntris = set_tri_list_around_point(p,tri,&tris,hs->interface);
	    
	    tri = tris[0];
	    if(!state_on_bdry_point(sl, sr, p, tri, hs))
	    {
	        tri = tris[ntris-1];
		if(!state_on_bdry_point(sl, sr, p, tri, hs))
		{
		    printf("ERROR in slsr3d(), couldn't find adjacent tri"
			   " with bond.\n");
	            slsr3d_error_info(p, hse, hs, tri, tris, ntris);
		    clean_up(ERROR);
		}
	    }
	}

}		/*end slsr3d*/


EXPORT	void	set_normal3d_method(
	NORMAL_METHOD nor3d_meth,
	int             dim)
{
	F_USER_INTERFACE *fuh = f_user_hook(dim);
	switch (nor3d_meth)
	{
	case PLANE_FIT_NORMAL:
	    fuh->_interface_normal_function._normal = f_plane_fit_normal3d;
	    fuh->_interface_normal_function._normal_name = 
	        strdup("f_plane_fit_normal3d");
	    break;
	case NORMAL_METHOD_FROM_RESTART:
	    break;
	case AREA_WEIGHTED_NORMAL:
	    fuh->_interface_normal_function._normal = f_area_weighted_normal3d;
	    fuh->_interface_normal_function._normal_name =
	        strdup("f_area_weighted_normal3d");
	    break;
	case SINE_WEIGHTED_NORMAL:
	    fuh->_interface_normal_function._normal = f_sine_weighted_normal3d;
	    fuh->_interface_normal_function._normal_name =
	        strdup("f_sine_weighted_normal3d");
	    break;
	case WLSP_NORMAL:
	default:
	    fuh->_interface_normal_function._normal = f_wlsp_normal;
	    fuh->_interface_normal_function._normal_name =
	        strdup("f_wlsp_normal");
	}
}		/*end set_normal3d_method*/

/*ARGSUSED*/
LOCAL  void f_plane_fit_normal3d(
	FT_POINT		   *p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF	   *hs,
	double		   *nor,
	Front		   *front)
{
	plane_fit_normal3d(p,hse,hs,nor);
}		/*end f_plane_fit_normal3d*/

/*ARGSUSED*/
LOCAL  void f_area_weighted_normal3d(
	FT_POINT		   *p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF	   *hs,
	double		   *nor,
	Front		   *front)
{
	area_weighted_normal3d(p,hse,hs,nor);
}		/*end f_area_weighted_normal3d*/

/*ARGSUSED*/
LOCAL  void f_sine_weighted_normal3d(
	FT_POINT		   *p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF	   *hs,
	double		   *nor,
	Front		   *front)
{
	sine_weighted_normal3d(p,hse,hs,nor);
}		/*end f_area_weighted_normal3d*/

/*ARGSUSED*/
LOCAL  void f_wlsp_normal(
	FT_POINT		   *p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF	   *hs,
	double		   *nor,
	Front		   *front)
{
	INTERFACE *intfc = hs->interface;
	int i,dim = intfc->dim;
	CURVE *c;

        if (intfc->normal_unset)
        {
	    FT_POINT *ptmp;
	    HYPER_SURF_ELEMENT *hsetmp;
	    HYPER_SURF         *hstmp;
            f_WLSP_set_intfc_geom(front,intfc);
            intfc->normal_unset = NO;
            intfc->curvature_unset = NO;
	    /* The following is in case the caller also used next_point() */
	    next_point(intfc,NULL,NULL,NULL);
	    while (next_point(intfc,&ptmp,&hsetmp,&hstmp))
		if (p == ptmp && hsetmp == hse && hstmp == hs) break;
        }
	switch (dim)
	{
	case 2:
	    c = Curve_of_hs(hs);
	    if (p == c->start->posn)
	    {
	    	for (i = 0; i < dim; ++i)
            	    nor[i] = c->nor_start[i];
	    }
	    else if (p == c->end->posn)
	    {
	    	for (i = 0; i < dim; ++i)
            	    nor[i] = c->nor_end[i];
	    }
	    else
	    {
	    	for (i = 0; i < dim; ++i)
            	    nor[i] = p->_nor[i];
	    }
	    break;
	case 3:
	    for (i = 0; i < dim; ++i)
            	nor[i] = p->_nor[i];
	}
}		/*end f_wlsp_normal */

LOCAL void f_wlsp_tangent(
        FT_POINT *p,
        BOND  *b,
        CURVE *c,
        double *tgnt,
	Front *front)
{
        double nor[MAXD];
        f_wlsp_normal(p,Hyper_surf_element(b),Hyper_surf(c),nor,front);
        tgnt[0] = -nor[1];
        tgnt[1] = nor[0];
}       /* end f_wlsp_tangent */

EXPORT  double   f_wlsp_curvature(
        FT_POINT           *p,
        HYPER_SURF_ELEMENT      *hse,
        HYPER_SURF              *hs,
        Front           *front)
{
	INTERFACE *intfc = hs->interface;
	int dim = intfc->dim;
	CURVE *c;
        if (intfc->normal_unset)
        {
	    FT_POINT              *ptmp;
            HYPER_SURF_ELEMENT *hsetmp;
            HYPER_SURF         *hstmp;
            f_WLSP_set_intfc_geom(front,intfc);
	    /* make sure it does not confuse a possible outside loop */
	    next_point(intfc,NULL,NULL,NULL);
	    while (next_point(intfc,&ptmp,&hsetmp,&hstmp))
		if (p == ptmp) break;
            intfc->normal_unset = NO;
            intfc->curvature_unset = NO;
        }
	switch (dim)
	{
	case 2:
	    c = Curve_of_hs(hs);
	    if (p == c->start->posn)
	    	return c->curvature_end;
	    else if (p == c->end->posn)
	    	return c->curvature_start;
	    else
	    	return p->curvature;
	case 3:
	    return p->curvature;
	    /* NEED TO CONSIDER BOUNDARY */
	}
	return p->curvature;
}       /* end f_wlsp_curvature */

LOCAL void f_WLSP_set_intfc_geom(
	Front *front,
	INTERFACE *intfc)	/* intfc may not be the same as front->interf */
{
        FT_POINT              *p;
        HYPER_SURF_ELEMENT *hse;
        HYPER_SURF         *hs;
        int dim = intfc->dim;

	if (debugging("wlsp_curvature"))
            (void) printf("Entering WLSP_set_intfc_geom()\n");
        switch (dim)
        {
        case 2:
            next_point(intfc,NULL,NULL,NULL);
            while (next_point(intfc,&p,&hse,&hs))
	    {
                if (!WLSP_compute_normal2d(p,hse,hs))
		{
		    f_normal2d(p,hse,hs,p->_nor,front);
		    p->curvature = f_mean_curvature_at_point2d(p,hse,hs,front);
		    if (debugging("wlsp_curvature"))
		    {
		    	(void) printf("WLSP_compute_normal2d() failed\n");
			(void) printf("Setting first order one\n");
			(void) printf("normal = %f %f\n",p->_nor[0],p->_nor[1]);
			(void) printf("curvature = %f\n",p->curvature);
			(void) print_curve(Curve_of_hs(hs));
		    }
		}
	    }
            break;
        case 3:
            next_point(intfc,NULL,NULL,NULL);
            while (next_point(intfc,&p,&hse,&hs))
                WLSP_compute_normal3d0(p,hse,hs);
            next_point(intfc,NULL,NULL,NULL);
            while (next_point(intfc,&p,&hse,&hs))
	    {
                if (!WLSP_compute_normal3d(p,hse,hs))
		{
		    f_sine_weighted_normal3d(p,hse,hs,p->_nor,front);
		    p->curvature = f_mean_curvature_at_point3d(p,hse,hs,front);
		}
	    }
        }
	if (debugging("wlsp_curvature"))
            (void) printf("Leaving WLSP_set_intfc_geom()\n");
}       /* end f_WLSP_set_intfc_geom */

EXPORT void FrontSetGeomVarMethods(
	Front *front,
	TANGENT_METHOD t_method,
	NORMAL_METHOD n_method,
	CURVATURE_METHOD c_method)
{
	int dim = front->rect_grid->dim;
	INTERFACE *intfc = front->interf;

	if (dim == 2)
	{
	    switch(t_method)
	    {
	    case LINEAR_SECANT:
		interface_tangent(intfc) = f_tangent;
	    case LANGRANGIAN_INTERPOLANT:
		interface_tangent(intfc) = f_lagrangian_tangent;
	    case WLSP_TANGENT:
		interface_tangent(intfc) = f_wlsp_tangent;
	    }
	    switch(n_method)
	    {
	    case FIRST_ORDER_NORMAL:
		interface_normal(intfc) = first_order_normal2d;
		break;
	    case WLSP_NORMAL:
		interface_normal(intfc) = f_wlsp_normal;
		break;
	    }
	    switch(n_method)
	    {
	    case NORMAL_CURVATURE:
		interface_curvature(intfc) = f_mean_curvature_at_point2d;
		break;
	    case WLSP_CURVATURE:
		interface_curvature(intfc) = f_wlsp_curvature;
		break;
	    }
	}
	else if (dim == 3)
	{
	    switch(n_method)
	    {
	    case AREA_WEIGHTED_NORMAL:
		interface_normal(intfc) = f_area_weighted_normal3d;
		break;
	    case SINE_WEIGHTED_NORMAL:
		interface_normal(intfc) = f_sine_weighted_normal3d;
		break;
	    case PLANE_FIT_NORMAL:
		interface_normal(intfc) = f_plane_fit_normal3d;
		break;
	    case WLSP_NORMAL:
		interface_normal(intfc) = f_wlsp_normal;
		break;
	    }
	    switch(n_method)
	    {
	    case NORMAL_CURVATURE:
		interface_curvature(intfc) = f_mean_curvature_at_point3d;
		break;
	    case WLSP_CURVATURE:
		interface_curvature(intfc) = f_wlsp_curvature;
		break;
	    }
	}
}	/* end FrontSetGeomVarMethods */
