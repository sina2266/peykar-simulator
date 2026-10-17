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
*				finit.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Contains the initialization routines
*
*		init_front()
*		init_front_states()
*/

#include <front/fdecs.h>		/* includes int.h, table.h */


	/* LOCAL Function Declarations */
LOCAL	boolean	f_read_print_FlowSpecifiedRegion_data(INIT_DATA*,const IO_TYPE*,
						      FlowSpecifiedRegion**);
LOCAL	boolean	read_print_ConstantFlowRegion(INIT_DATA*,const IO_TYPE*,
                                              INTERFACE*,
					      FlowSpecifiedRegion*,
					      FlowSpecifiedRegion**);
LOCAL	void	init_front_time_step_control(INIT_DATA*);
LOCAL	void	f_set_front_time_step_control_default(INIT_DATA*);
LOCAL	void	prompt_for_front_spacing(INIT_DATA*);
LOCAL	void	read_print_FlowSpecifiedRegion_list(INIT_DATA*,const IO_TYPE*);
LOCAL	void 	init_rect_bdry_type(int**,RECT_GRID*);

LOCAL	void	clear_curve_redistribution_info(CURVE_REDISTRIBUTE*);
LOCAL	void	init_redistribution_frequency(INIT_DATA*);
LOCAL	void	set_redistribution_frequency(INIT_DATA*,Front*);

LOCAL void	init_1d_front_states(Front*,INIT_DATA*,
				     void(*)(FT_POINT*,HYPER_SURF_ELEMENT*,
					     HYPER_SURF*,Locstate,Locstate,
					     INIT_DATA*));

LOCAL	void	f_init_curve_redistribution_parameters(INIT_DATA*,Front*);
LOCAL	void	f_prompt_for_curve_redist_options(INIT_DATA*);
LOCAL 	void	init_2d_front_states(Front*,INIT_DATA*,
				     void (*)(FT_POINT*,HYPER_SURF_ELEMENT*,
					      HYPER_SURF*,Locstate,Locstate,
					      INIT_DATA*));
LOCAL 	void    set_boundary_node_type(NODE*,INTERFACE*);
LOCAL 	void 	FT_InitIntfc1d(Front*,LEVEL_FUNC_PACK*);
LOCAL 	void 	FT_InitIntfc2d(Front*,LEVEL_FUNC_PACK*);
LOCAL 	void 	FT_InitIntfc3d(Front*,LEVEL_FUNC_PACK*);

LOCAL	void	f_init_surface_redistribution_parameters(INIT_DATA*,Front*);
LOCAL	void    init_3d_front_states(Front*,INIT_DATA*,
				     void (*)(FT_POINT*,HYPER_SURF_ELEMENT*,
					      HYPER_SURF*,Locstate,Locstate,
					      INIT_DATA*));
LOCAL	void	f_prompt_for_surface_redist_options(INIT_DATA*);
LOCAL 	void 	read_print_front_time_and_step(Front*,FILE*);

/*
*			prompt_for_front_options():
*
*	Prompts for user selected front options.
*
*/

EXPORT void f_prompt_for_front_options(
	INIT_DATA *init,
	Front     *front)
{
	char	s[Gets_BUF_SIZE];
	int	dim = Comp_grid(init).dim;

	debug_print("init","Entered f_prompt_for_front_options()\n");

	InitialMaxFrontSpeed(init) =
	    alloc_MaxFrontSpeed(InitialMaxFrontSpeed(init),
	                        i_intfc(init),StateSize(init));

	if (restart_io_type(init) != NULL)
	    read_print_front_options(init,front);

	set_redistribution_defaults(init);
	prompt_for_redistribute(init);

	    /* Choose Constant to Increase/Decrease Time-Steps */

	set_front_time_step_control_default(init);
	init_front_time_step_control(init);

	if (StateSize(init) != 0)
	{
	    screen("\n\t\tflow specified state enforcement at fronts\n\n");

	    enforce_flow_specified_states(init) = YES;
	    screen("Enforce flow specified states at fronts (dflt=%s): ",
	       (enforce_flow_specified_states(init)==YES)?"yes":"no");
	    (void) Gets(s);
	    if (s[0] == 'y' || s[0] == 'Y')
	    	enforce_flow_specified_states(init) = YES;
	    else if (s[0] == 'n' || s[0] == 'N')
	    	enforce_flow_specified_states(init) = NO;

	    (void) printf("\n");

	    movingframe(init) = NO;
	    screen("Type yes to propagate front in moving frame (dflt = %s): ",
	       		(movingframe(init)==YES)?"yes":"no");
	    (void) Gets(s);
	    if (s[0] == 'y' || s[0] == 'Y')
	    	movingframe(init) = YES;
	    else if (s[0] == 'n' || s[0] == 'N')
	    	movingframe(init) = NO;

	    (void) printf("\n");
	}

	tangent_method(init) = WLSP_TANGENT;
	if (restart_io_type(init) != NULL)
	    tangent_method(init) = TANGENT_METHOD_FROM_RESTART;
	normal3d_method(init) = WLSP_NORMAL;
	if (restart_io_type(init) != NULL)
	    normal3d_method(init) = NORMAL_METHOD_FROM_RESTART;

	if (dim == 2) /*Only 2D currently supports multiple tangent algorithms*/
	{
	    static const char *dflt = ", default";
	    screen("Select tangent computation algorithm, choices are\n"
	           "\tWLSP_TANGENT (WLSP%s)\n"
	           "\tLinear centered SECANT uni_arrays (SECANT%s)\n"
	           "\tFourth order LANGRANGIAN interpolation (LANGRANGIAN%s)\n"
	           "\tCubic SPLINE fit (SPLINE%s)\n",
		   (tangent_method(init)==WLSP_TANGENT) ? dflt : "",
		   (tangent_method(init)==LINEAR_SECANT) ? dflt : "",
		   (tangent_method(init)==LANGRANGIAN_INTERPOLANT) ? dflt : "",
		   (tangent_method(init)==CUBIC_SPLINE) ? dflt : "");
	    if (restart_io_type(init) != NULL)
	        screen("\tTangent method from restart file (restart%s)\n",
		       (tangent_method(init)==TANGENT_METHOD_FROM_RESTART) ?
		           dflt : "");
	    screen("Enter choice: ");
	    (void) Gets(s);
	    if (s[0] != '\0')
	    {
		if (strcasecmp(s,"wlsp")==0)
	            tangent_method(init) = WLSP_TANGENT;
		if (strcasecmp(s,"secant")==0)
	            tangent_method(init) = LINEAR_SECANT;
		if (strcasecmp(s,"langrangian")==0)
	            tangent_method(init) = LANGRANGIAN_INTERPOLANT;
		if (strcasecmp(s,"spline")==0)
	            tangent_method(init) = CUBIC_SPLINE;
		if (strcasecmp(s,"restart")==0)
	            tangent_method(init) = TANGENT_METHOD_FROM_RESTART;
	    }
	    if (restart_io_type(init) != NULL)
	    {
	        F_USER_INTERFACE *fuh;
	        set_tangent_operator(tangent_method(init),dim);
		fuh = f_user_hook(dim);
		interface_tangent_function(restart_intfc(init)) =
		    fuh->_interface_tangent_function;
	    }
	}
	if (dim == 3)
	{
	    static const char *dflt = ", default";
	    screen("Select normal computation algorithm, choices are\n"
	           "\tWLSP normals (WLSP%s)\n"
	           "\tArea weighted normals (AREA%s)\n"
	           "\tSine weighted normals (SINE%s)\n"
	           "\tLeast squares plane fit (PLANE%s)\n",
		   (normal3d_method(init)==WLSP_NORMAL) ? dflt : "",
		   (normal3d_method(init)==AREA_WEIGHTED_NORMAL) ? dflt : "",
		   (normal3d_method(init)==SINE_WEIGHTED_NORMAL) ? dflt : "",
		   (normal3d_method(init)==PLANE_FIT_NORMAL) ? dflt : "");
	    if (restart_io_type(init) != NULL)
	        screen("\tNormal method from restart file (restart%s)\n",
		       (normal3d_method(init)==NORMAL_METHOD_FROM_RESTART) ?
		           dflt : "");
	    screen("Enter choice: ");
	    (void) Gets(s);
	    if (s[0] != '\0')
	    {
		if (strcasecmp(s,"wlsp")==0)
	            normal3d_method(init) = WLSP_NORMAL;
		if (strcasecmp(s,"area")==0)
	            normal3d_method(init) = AREA_WEIGHTED_NORMAL;
		if (strcasecmp(s,"sine")==0)
	            normal3d_method(init) = SINE_WEIGHTED_NORMAL;
		if (strcasecmp(s,"plane")==0)
	            normal3d_method(init) = PLANE_FIT_NORMAL;
		if (strcasecmp(s,"restart")==0)
	            normal3d_method(init) = NORMAL_METHOD_FROM_RESTART;
	    }
	}

	debug_print("init","Left f_prompt_for_front_options()\n");
}		/*end f_prompt_for_front_options*/


EXPORT	void	set_front_hooks(
	INIT_DATA	*init)
{
	F_INIT_DATA	*f_init = f_init_data(init);

	f_init->_set_redistribution_defaults = f_set_redistribution_defaults;
	f_init->_set_front_time_step_control_default = 
	    f_set_front_time_step_control_default;
	f_init->_copy_redistribution_values = f_copy_redistribution_values;
	f_init->_prompt_for_redistribute = f_prompt_for_redistribute;
	f_init->_read_print_FlowSpecifiedRegion_data = 
	    f_read_print_FlowSpecifiedRegion_data;
	f_init->_prompt_for_front_options = f_prompt_for_front_options;
	f_init->_read_print_front_options = f_read_print_front_options;
}		/*end set_front_hooks*/

/*
*			set_default_front_options():
*
*	set default front options.
*
*/

EXPORT void set_default_front_options(
	INIT_DATA *init,
	Front     *front)
{
	char	s[Gets_BUF_SIZE];
	int	dim = front->rect_grid->dim;

	debug_print("init","Entered set_default_front_options()\n");

	InitialMaxFrontSpeed(init) =
	    alloc_MaxFrontSpeed(InitialMaxFrontSpeed(init),
	                        i_intfc(init),StateSize(init));

	if (restart_io_type(init) != NULL)
	    read_print_front_options(init,front);

	set_redistribution_defaults(init);

	    /* Choose Constant to Increase/Decrease Time-Steps */

	set_front_time_step_control_default(init);

	tangent_method(init) = LINEAR_SECANT;
	if (restart_io_type(init) != NULL)
	    tangent_method(init) = TANGENT_METHOD_FROM_RESTART;
	if (dim == 3)
	{
	    normal3d_method(init) = AREA_WEIGHTED_NORMAL;
	    if (restart_io_type(init) != NULL)
	        normal3d_method(init) = NORMAL_METHOD_FROM_RESTART;
	}

	if (dim == 2) /*Only 2D currently supports multiple tangent algorithms*/
	{
	    if (restart_io_type(init) != NULL)
	    {
	        F_USER_INTERFACE *fuh;
	        set_tangent_operator(tangent_method(init),dim);
		fuh = f_user_hook(dim);
		interface_tangent_function(restart_intfc(init)) =
		    fuh->_interface_tangent_function;
	    }
	}
	debug_print("init","Left set_default_front_options()\n");
}		/*end set_default_front_options*/

EXPORT	void	f_read_print_front_options(
	INIT_DATA *init,
	Front     *front)
{
	const IO_TYPE *io_type = restart_io_type(init);
	FILE    *file = io_type->file;
	int	redis_flag;

	front->interf = restart_intfc(init);
	if (next_output_line_containing_string(file,
					 "REDISTRIBUTION INFORMATION:") != NULL)
	{
	    (void) fgetstring(file,"redistribution count = ");
	    (void) fscanf(file,"%d",&redistribution_count(init));
	    
	    (void) fgetstring(file,"redis_flag = ");
	    (void) fscanf(file,"%d",&redis_flag);
	    redis_flag(init) = redis_flag == 0 ? NO : YES;
	}
	else
	{
	    redistribution_count(init) = 0;
	    redis_flag(init) = NO;
	}

	if (next_output_line_containing_string(file,"FRONT SPEEDS:"))
	{
	    if (!initial_read_print_max_front_speed_info(init,front))
	    {
		MAX_FRONT_SPEED *mfs = InitialMaxFrontSpeed(init);
	        int dim = Comp_grid(init).dim;

		/*Old Style output*/
	        if (fgetstring(file,"front->spfr ="))
	        {
	            int		i, c;
		    if ((c = getc(file)) != '\f')
		    {		/* NOBINARY */
			(void) ungetc(c,file);
			for (i = 0; i <= dim; ++i)
		            (void) fscan_float(file,mfs->_spfr+i);
		    }
		    else
		    {		/* BINARY */
			(void) getc(file);
			(void) read_binary_real_array(mfs->_spfr,dim+1,
			                              io_type);
		    }
	        }
	    }
	}
	read_print_FlowSpecifiedRegion_list(init,io_type);
}		/*end f_read_print_front_options*/

/*
*			     init_front():
*
*	Initialization of a front involves setting various parameters 
*	such as the type of redistribute algorithm to be used, 
*	the front spacing, the time step factor.
*/


EXPORT void init_front(
	INIT_DATA     *init,
	Front	      *front)
{
	debug_print("init","Entered init_front()\n");

	Clear_redistribution_parameters(front);
	front->redis_flag = NO;
	front->dismin = front->dismax = front->distance = 0.0;
        front->hmin_fre = front->hmax_fre = 0.0;
	if (restart_io_type(init) != NULL)
	    read_print_front(init,front);

		/* Choose Redistribution Method */

	Init_redistribution(init,front);

	    /* Choose Constant to Increase/Decrease Time-Steps */

	front->Tstep = time_step_control_options(init);

	debug_print("init","Left init_front()\n");
}		/*end init_front*/


/*
*		initial_front_redistribute():
*
*	Also, the redistribution
*	of the initial interface front->interf is performed.
*
*	IMPORTANT NOTE:  For restarts,  the interface is never 
*	redistributed here.
*/

/*ARGSUSED*/
EXPORT void initial_front_redistribute(
	Front	      *front,
	const IO_TYPE *restart_io_type)
{
		/* Redistribute Initial Front */
	debug_print("init","Entered initial_front_redistribute()\n");

	if (front->rect_grid->dim == 1)
	{
	    debug_print("init","Left initial_front_redistribute()\n");
	    return;
	}

	if (debugging("init"))
	{
	    (void) printf("Interface before redistribution\n");
	    print_interface(front->interf);
	}

 	if (restart_io_type == NULL)
	{
	    if (redistribute(front,YES,NO) != GOOD_REDISTRIBUTION)
	    {
	    	screen("ERROR in initial_front_redistribute(), "
	    	       "redistribute failed\n");
	    	clean_up(ERROR);
	    }
	}

	measure_front(front);

	if (debugging("Front")) 
	    print_Front_structure(front);
	debug_print("init","Left initial_front_redistribute()\n");
}		/*end initial_front_redistribute*/


LOCAL	void	f_set_front_time_step_control_default(
	INIT_DATA *init)
{
	char	s[Gets_BUF_SIZE];
	int	dim = Comp_grid(init).dim;
	TSTEP_CONTROL	*tstep;

	debug_print("init","Entered set_front_time_step_control_default()\n");

	tstep = &time_step_control_options(init);
	tstep->time_step_factor   = 0.75; /*DEFAULT TOLERANCE*/
	tstep->apply_cfl_at_nodes = YES;  /*DEFAULT*/
	tstep->apply_cfl_at_nodes = (debugging("NoNdCFL")) ? NO : YES;
	tstep->max_sep            = 2.0;  /*DEFAULT TOLERANCE*/
	tstep->cfl_fudge          = 1.1;  /*DEFAULT TOLERANCE*/
	tstep->frac_floor         = 0.75; /*DEFAULT TOLERANCE*/
	tstep->frac_ceil          = 1.25; /*DEFAULT TOLERANCE*/

	debug_print("init","Left set_front_time_step_control_default()\n");
}	/* end f_set_front_time_step_control_default */

LOCAL	void	init_front_time_step_control(
	INIT_DATA *init)
{
	char	s[Gets_BUF_SIZE];
	int	dim = Comp_grid(init).dim;
	TSTEP_CONTROL	*tstep;

	tstep = &time_step_control_options(init);
	debug_print("init","Entered init_front_step_control()\n");
	screen("\n\t\ttime step size control\n\n");

	screen("\nThe current defaults for the front time step control are\n");
	screen("\tTime step factor = %g\n",tstep->time_step_factor);
	screen("\tApply CFL at nodes = %s\n",y_or_n(tstep->apply_cfl_at_nodes));
	screen("\tMaximum node separation at untangle = %g\n",tstep->max_sep);
	screen("\tCFL increase factor = %g\n",tstep->cfl_fudge);
	screen("\tMinimum time step modification factor = %g\n",
	       tstep->frac_floor);
	screen("\tMaximum time step modification factor = %g\n",
	       tstep->frac_ceil);

	screen("Use defaults for front time step control (default = y): ");
	(void) Gets(s);

	if (s[0] != 'n' && s[0] != 'N')
	    return;

	screen("Enter the time step factor (fraction of CFL condition - "
	       "default %g): ",tstep->time_step_factor);
	(void) Gets(s);
	if (s[0] != '\0')
	{
	    int n;
	    n = sscan_float(s,&tstep->time_step_factor);
	    if (n != 1)
	    {
		screen("ERROR in init_front_time_step_control(), "
		       "couldn't scan Time_step_factor\n");
		clean_up(ERROR);
	    }
	}

	if (dim == 2)
	{
	    screen("Use node velocity to restrict CFL condition "
	           "(default %s): ",y_or_n(tstep->apply_cfl_at_nodes));
	    (void) Gets(s);
	    if (s[0] != '\0')
	    {
	    	if (tstep->apply_cfl_at_nodes == YES)
	    	{
	   	    if (s[0] == 'N' || s[0] == 'n')
		        tstep->apply_cfl_at_nodes = NO;
		}
		else
		{
		    if (s[0] == 'Y' || s[0] == 'y')
		        tstep->apply_cfl_at_nodes = YES;
		}
	    }
	    if (tstep->apply_cfl_at_nodes == YES)
	    {
		screen("Enter the maximum node separation at "
		       "tangles (default %g): ",tstep->max_sep);
		(void) Gets(s);
		if (s[0] != '\0')
		    (void) sscan_float(s,&tstep->max_sep);
	    }
	}

	screen("Enter the CFL increase factor (default %g): ",tstep->cfl_fudge);
	(void) Gets(s);
	if (s[0] != '\0')
	    (void) sscan_float(s,&tstep->cfl_fudge);

	screen("Enter the minimum time step modification factor "
	       "(default %g): ",tstep->frac_floor);
	(void) Gets(s);
	if (s[0] != '\0')
	    (void) sscan_float(s,&tstep->frac_floor);

	screen("Enter the maximum time step modification factor "
	       "(default %g): ",tstep->frac_ceil);
	(void) Gets(s);
	if (s[0] != '\0')
	    (void) sscan_float(s,&tstep->frac_ceil);
	debug_print("init","Left init_front_step_control()\n");
}		/*end init_front_time_step_control*/

EXPORT	void	f_set_redistribution_defaults(
	INIT_DATA *init)
{
	static const double STANDARD_SCALED_LENGTH = 0.75;/*TOLERANCE*/

	front_redist_mode(init) =             FULL_REDIST;
	full_curve_redist_version(init) =     EQUI_BOND_REDISTRIBUTE;
	use_rect_bdry_redistribution(init) =  NO;
	cosine_big_angle(init,GENERAL_WAVE) = 1.0;
	cosine_big_angle(init,VECTOR_WAVE) =  1.0;
	
	redistribution_grid_size(init) = 3.0/4;           /*DEFAULT*/
	redistribution_frequency(init,GENERAL_WAVE) = 5; /*DEFAULT*/
	redistribution_frequency(init,VECTOR_WAVE) = 5;   /*DEFAULT*/
	redistribution_frequency(init,GENERAL_NODE) = 5; /*DEFAULT*/
	reconstruction_frequency(init) = 10;              /*DEFAULT*/

	tracking_algorithm(init) = LOCALLY_GRID_BASED_TRACKING;/*For 3D only */

	front_spacing(init,GENERAL_WAVE) = STANDARD_SCALED_LENGTH;
	front_spacing(init,VECTOR_WAVE) = STANDARD_SCALED_LENGTH;

	/*The default values are all TOLERANCES*/
	maximum_triangle_area_factor(init,GENERAL_WAVE) = 2.0;
	minimum_triangle_area_factor(init,GENERAL_WAVE) = 0.5;
	maximum_triangle_area_factor(init,VECTOR_WAVE) = 2.0;
	minimum_triangle_area_factor(init,VECTOR_WAVE) = 0.5;
	minimum_angle_at_triangle_vertex(init,GENERAL_WAVE) = radians(15.0);
	minimum_angle_at_triangle_vertex(init,VECTOR_WAVE) = radians(15.0);
	maximum_scaled_triangle_side_length(init) = 1.3;
}		/*end f_set_redistribution_defaults*/

EXPORT void f_copy_redistribution_values(
	INIT_DATA	*init,
	Front		*front)
{
	set_redistribution_defaults(init);

	front_redist_mode(init) = Redistribution_mode(front);
	curve_redist_options(init) = CurveRedistributionOptions(front);

	redistribution_grid_size(init) =
	    Grid_size_of_redistribution(front);
	redistribution_frequency(init,GENERAL_WAVE) =
	    Frequency_of_redistribution(front,GENERAL_WAVE);
	redistribution_frequency(init,VECTOR_WAVE) =
	    Frequency_of_redistribution(front,VECTOR_WAVE);
	redistribution_frequency(init,GENERAL_NODE) =
	    Frequency_of_redistribution(front,GENERAL_NODE);

	reconstruction_frequency(init) = Frequency_of_reconstruction(front);

	redistribution_count(init) = Redistribution_count(front);
	redis_flag(init) = front->redis_flag;

	tracking_algorithm(init) = Tracking_algorithm(front);

	front_spacing(init,GENERAL_WAVE) = Front_spacing(front,GENERAL_WAVE);
	front_spacing(init,VECTOR_WAVE) = Front_spacing(front,VECTOR_WAVE);

	surface_redist_options(init) = SurfaceRedistributionOptions(front);
}		/*end f_copy_redistribution_values*/

/*
*			f_prompt_for_redistribute():
*
*	Prompt for redistribution parameters.
*/

EXPORT void f_prompt_for_redistribute(
	INIT_DATA   *init)
{
	int     dim = Comp_grid(init).dim;

	debug_print("init","Entered f_prompt_for_redistribute()\n");
	if (dim == 1)
	    return;

	f_prompt_for_curve_redist_options(init);   

	/* Choose Approximate Spacing of Points on Front */
	prompt_for_front_spacing(init);

	if (dim == 3)
	    f_prompt_for_surface_redist_options(init);

	debug_print("init","Left f_prompt_for_redistribute()\n");
}		/* end to f_prompt_for_redistribute() */


/*
*			f_init_redistribute():
*
*	Provides for the initialization of redistribution parameters
* 	in a physics independent manner.  It is assumed that the calling
*	routine decides whether to call Clear_redistribution_parameters().
*	This is to preserve any values that are set during pause or restart.
*/

EXPORT void f_init_redistribute(
	INIT_DATA	*init,
	Front		*front)
{
	debug_print("init","Entered f_init_redistribute()\n");

	if (front->rect_grid->dim == 1)
	{
	    debug_print("init","Left f_init_redistribute()\n");
	    return;
	}

	Redistribution_mode(front) = front_redist_mode(init);
	f_init_curve_redistribution_parameters(init,front);

	Front_spacing(front,GENERAL_WAVE) = front_spacing(init,GENERAL_WAVE);
	Front_spacing(front,VECTOR_WAVE) = front_spacing(init,VECTOR_WAVE);

	if (front->rect_grid->dim == 3)
	    f_init_surface_redistribution_parameters(init,front);
	
	debug_print("init","Left f_init_redistribute()\n");
}		/*end f_init_redistribute*/

EXPORT	void	set_dflt_cur_redist_params(
	Front	*front)
{
	Curve_redistribution_function(front) = full_redistribute;
	Forward_curve_redistribute_function(front) = equi_curve_redistribute;
	Backward_curve_redistribute_function(front) =
	    backward_equi_curve_redistribute;
	Node_redistribute_function(front) = NULL;
	Delete_small_loops_function(front) = f_delete_small_loops;
	Delete_fold_back_bonds_function(front) = f_delete_fold_back_bonds;
	Delete_exterior_curves_function(front) = f_delete_exterior_curves;
	Delete_phys_remn_on_bdry_function(front) = f_delete_phys_remn_on_bdry;
	Delete_point_adjacent_to_node_function(front) =
	    f_delete_point_adjacent_to_node;
	Use_rect_boundary_redistribution(front) = NO;
	Cosine_big_angle(front,GENERAL_WAVE) =
	    Cosine_big_angle(front,VECTOR_WAVE) = 1.0;
	Front_length(front) = -HUGE_VAL;
}		/*end set_dflt_cur_redist_params*/

/*
*		f_prompt_for_curve_redist_options():
*
*	Prompt for curve redistribution options.
*/

LOCAL void f_prompt_for_curve_redist_options(
	INIT_DATA   *init)
{
	char		s[Gets_BUF_SIZE];
	double		big_angle;
	boolean		use_big_angle = NO;
	int		dim = Comp_grid(init).dim;	

	debug_print("init","Entered f_prompt_for_curve_redist_oprtions()\n");

	screen("\n\t\tCurve Redistribution Control\n\n");
	screen("Enter the mode of curve redistribution --\n\t");
	screen("`none'%s, `expansion'%s, or `full'%s: ",
	    (front_redist_mode(init) == NO_REDIST) ? " (dflt)" : "",
	    (front_redist_mode(init) == EXPANSION_REDIST) ? " (dflt)" : "",
	    (front_redist_mode(init) == FULL_REDIST) ? " (dflt)" : "");
	(void) Gets(s);
	switch (s[0])
	{
	case 'n':					/* None */
	case 'N':
	    front_redist_mode(init) = NO_REDIST;
	    break;

	case 'e':					/* Expansion */
	case 'E':
	    front_redist_mode(init) = EXPANSION_REDIST;
	    break;

	case 'f':					/* Full */
	case 'F':
	    front_redist_mode(init) = FULL_REDIST;
	    break;

	default:
	    break;
	}

	switch (front_redist_mode(init))
	{
	case EXPANSION_REDIST:
	    if (dim == 2)
	    	init_redistribution_frequency(init);
	    break;

	case FULL_REDIST:
	    screen("Enter version of full curve redistribution\n");
	    screen("\tordinary full curve redistribution [o%s]\n",
	    	   (full_curve_redist_version(init) == ORDINARY_REDISTRIBUTE) ?
	           "(default)" : "");
	    screen("\tequi-bond curve redistribution [e%s]\n",
	    	   (full_curve_redist_version(init) == EQUI_BOND_REDISTRIBUTE) ?
	               "(default)" : "");
	    screen("Enter choice: ");
	    (void) Gets(s);
	    switch (s[0])
	    {
	    case 'o':
	    case 'O':
	    	use_big_angle = YES;
		full_curve_redist_version(init) = ORDINARY_REDISTRIBUTE;
            break;
	    case 'e':
	    case 'E':
	    	full_curve_redist_version(init) = EQUI_BOND_REDISTRIBUTE;
		break;
	    default:
		break;
	    }

	    if (dim == 2)
	    	init_redistribution_frequency(init);

	    if (use_big_angle == YES)
	    {
	    	big_angle = acos(cosine_big_angle(init,GENERAL_WAVE));
	    	screen("Enter the hyperbolic big angle for general "
	    	       "curves (in degrees) (dflt = %g): ",degrees(big_angle));
		(void) Gets(s);
		if (s[0] != '\0')
		{
		    (void) sscan_float(s,&big_angle);
		    big_angle = radians(big_angle);
		    cosine_big_angle(init,GENERAL_WAVE) = cos(big_angle);
	        }

	        big_angle = acos(cosine_big_angle(init,VECTOR_WAVE));
	        screen("Enter the hyperbolic big angle for vector  "
		       "curves (in degrees) (dflt = %g): ",degrees(big_angle));
		(void) Gets(s);
		if (s[0] != '\0')
		{
		    (void) sscan_float(s,&big_angle);
		    big_angle = radians(big_angle);
		    cosine_big_angle(init,VECTOR_WAVE) = cos(big_angle);
		}
	    }
	    break;

	case NO_REDIST:
	default:
	    break;
	}

	if ((dim == 2) && (front_redist_mode(init) != NO_REDIST))
	{
	    screen("Type 'y' for rect grid based redistribution "
	           "of rectangular boundaries: ");
	    (void) Gets(s);
	    if (s[0] == 'y' || s[0] == 'Y')
	    	use_rect_bdry_redistribution(init) = YES;
	}
	debug_print("init","Left f_prompt_for_curve_redist_oprtions()\n");
}		/* end to f_prompt_for_curve_redist_options() */									

/*
*		f_init_curve_redistribution_parameters():
*
*	Provides for the initialization of redistribution parameters
* 	in a physics independent manner.
*/

LOCAL void f_init_curve_redistribution_parameters(
	INIT_DATA *init,
	Front	  *front)
{
	int	       dim = front->rect_grid->dim;
	CURVE_CLEANERS Sav_cleaners;

	debug_print("init","Entered f_init_curve_redistribution_parameters()\n");

		/* DEFAULT values of redistribution parameters */
	CurveRedistributionOptions(front) = curve_redist_options(init);
	clear_curve_redistribution_info(&Curve_redistribution_info(front));
	if (dim == 1)
	    return;

	set_dflt_cur_redist_params(front);
	Sav_cleaners = Curve_redistribution_info(front).Cleaners;

	switch (front_redist_mode(init))
	{
	case NO_REDIST:
	    clear_curve_redistribution_info(&Curve_redistribution_info(front));
	    Curve_redistribution_info(front).Cleaners = Sav_cleaners;
	    break;

	case EXPANSION_REDIST:
	    clear_curve_redistribution_info(&Curve_redistribution_info(front));
	    Curve_redistribution_function(front) = expansion_redistribute;
	    Curve_redistribution_info(front).Cleaners = Sav_cleaners;
	    break;

	case FULL_REDIST:
	    Curve_redistribution_function(front) = full_redistribute;
	    switch (full_curve_redist_version(init))
	    {
	    case ORDINARY_REDISTRIBUTE:
	        Forward_curve_redistribute_function(front) =
	            full_inc_redist_cur;
	        Backward_curve_redistribute_function(front) =
	            full_dec_redist_cur;
	        break;

	    case EQUI_BOND_REDISTRIBUTE:
	        Forward_curve_redistribute_function(front) =
	            equi_curve_redistribute;
	        Backward_curve_redistribute_function(front) =
	            backward_equi_curve_redistribute;
	        break;

	    default:
	        screen("ERROR in f_init_curve_redistribution_parameters(), "
	               "invalid curve redistribution version%d\n",
	               full_curve_redist_version(init));
	        clean_up(ERROR);
	    }
	    Cosine_big_angle(front,GENERAL_WAVE) =
	        cosine_big_angle(init,GENERAL_WAVE);
	    Cosine_big_angle(front,VECTOR_WAVE) =
	        cosine_big_angle(init,VECTOR_WAVE);
	    break;
	    
	default:
	    screen("ERROR in f_init_curve_redistribution_parameters(), "
	           "invalid redistribution mode %d\n",front_redist_mode(init));
	    clean_up(ERROR);
	}

	if ((dim == 2) && (Curve_redistribution_function(front) != NULL))
	{
	    set_redistribution_frequency(init,front);
	    Use_rect_boundary_redistribution(front) =
	        use_rect_bdry_redistribution(init);
	}
	
	debug_print("init","Left f_init_curve_redistribution_parameters()\n");
}		/*end f_init_curve_redistribution_parameters*/

LOCAL	void	clear_curve_redistribution_info(
	CURVE_REDISTRIBUTE *cdi)
{
	zero_scalar(cdi,sizeof(CURVE_REDISTRIBUTE));
}		/*end clear_curve_redistribution_info*/


LOCAL	void	prompt_for_front_spacing(
	INIT_DATA *init)
{
	char	    s[Gets_BUF_SIZE];
	int	    dim = Comp_grid(init).dim;
	static const char  *name[] = {"", "", "curves", "surfaces"};


	if (dim == 1)
	    return;

	screen("\n\t\tfront spacing control\n\n");

	screen("Enter the spacing for general %s ",name[dim]);
	screen("in dimensionless\n\tlength/mesh units (dflt = %g): ",
	       front_spacing(init,GENERAL_WAVE));
	(void) Gets(s);
	if (s[0] != '\0')
	{
	    (void) sscan_float(s,&front_spacing(init,GENERAL_WAVE));
	    if (front_spacing(init,GENERAL_WAVE) >= 1.0)	/*TOLERANCE*/
	    	front_spacing(init,GENERAL_WAVE) = 0.999;	/*TOLERANCE*/
	}

	screen("Enter the spacing for vector type %s ",name[dim]);
	screen("in dimensionless\n\tlength/mesh units (dflt = %g): ",
		front_spacing(init,VECTOR_WAVE));
	(void) Gets(s);
	if (s[0] != '\0')
	{
	    (void) sscan_float(s,&front_spacing(init,VECTOR_WAVE));
	    if (front_spacing(init,VECTOR_WAVE) >= 1.0)		/*TOLERANCE*/
	    	front_spacing(init,VECTOR_WAVE) = 0.999;	/*TOLERANCE*/
	}
}		/*end prompt_for_front_spacing*/


LOCAL	void	init_redistribution_frequency(
	INIT_DATA *init)
{
	char	s[Gets_BUF_SIZE];
	int	dim = Comp_grid(init).dim;

	debug_print("init","Entered init_redistribution_frequency()\n");

	if(restart_io_type(init) == NULL)
	    redis_flag(init) = NO;
	
	if (tracking_algorithm(init) == GRID_BASED_TRACKING  ||  
	    tracking_algorithm(init) == THREE_COMP_GRID_BASED_TRACKING)  
	{
	  redistribution_frequency(init,GENERAL_WAVE) = INT_MAX;
	  redistribution_frequency(init,VECTOR_WAVE) = INT_MAX;
	  redistribution_frequency(init,GENERAL_NODE) = INT_MAX;
	  reconstruction_frequency(init) = INT_MAX;
	  redistribution_count(init) = 0;
	}
	else
	{
	  screen("\n\t\tRedistribution Frequency Control\n\n");

 	  screen("Enter the frequency of redistribution for general curves "
	         "(dflt = %d): ",redistribution_frequency(init,GENERAL_WAVE));
	  (void) Gets(s);
	  if (s[0] != '\0')
 	    (void) sscanf(s,"%d",&redistribution_frequency(init,GENERAL_WAVE));

 	  screen("Enter the frequency of redistribution for vector curves "
	         "(dlft = %d): ",redistribution_frequency(init,VECTOR_WAVE));
	  (void) Gets(s);
	  if (s[0] != '\0')
 	    (void) sscanf(s,"%d",&redistribution_frequency(init,VECTOR_WAVE));

	  if (dim == 2)
	  {
	      screen("Enter the frequency of node redistribution "
                     "(dflt = %d): ",
		     redistribution_frequency(init,GENERAL_NODE));
	      (void) Gets(s);
	      if (s[0] != '\0')
	      {
	    	(void) sscanf(s,"%d",
	            &redistribution_frequency(init,GENERAL_NODE));
	      }
	  }
	  if (tracking_algorithm(init) == MIXED_TRACKING)
	  {
	    screen("Enter the frequency of reconstruction (dlft = %d): ",
		   reconstruction_frequency(init));
	    (void) Gets(s);
	    if (s[0] != '\0')
		(void) sscanf(s,"%d",&reconstruction_frequency(init));
	  }

		/* Determine new count_redist */

	  screen("Enter the redistribute count (default = %d): ",
		  redistribution_count(init));
	  (void) Gets(s);
	  if (s[0] != '\0')
	      (void) sscanf(s,"%d",&redistribution_count(init));
	}

	debug_print("init","Left init_redistribution_frequency()\n");
}		/*end init_redistribution_frequency*/

LOCAL void  set_redistribution_frequency(
	INIT_DATA *init,
	Front     *front)
{
	debug_print("init","Entered set_redistribution_frequency()\n");
	Frequency_of_redistribution(front,GENERAL_WAVE) =
	    redistribution_frequency(init,GENERAL_WAVE);
	Frequency_of_redistribution(front,VECTOR_WAVE) =
	    redistribution_frequency(init,VECTOR_WAVE);
	Frequency_of_redistribution(front,GENERAL_NODE) =
	    redistribution_frequency(init,GENERAL_NODE);
	Grid_size_of_redistribution(front) =
	    redistribution_grid_size(init);

	if (tracking_algorithm(init) == MIXED_TRACKING)
	    Frequency_of_reconstruction(front) = reconstruction_frequency(init);

	Redistribution_count(front) = redistribution_count(init);
	front->redis_flag = redis_flag(init);

	debug_print("init","Left set_redistribution_frequency()\n");
}		/*end to set_redistribution_frequency()*/

EXPORT	void	f_read_print_front(
	INIT_DATA	*init,
	Front		*front)
{
	Redistribution_count(front) = redistribution_count(init);
	front->redis_flag = redis_flag(init);
	MaxFrontSpeed(front) = InitialMaxFrontSpeed(init);
	if (debugging("restart"))
	{
	    (void) printf("Read value of MAX_FRONT_SPEED\n");
	    print_max_front_speed_info(stdout,front);
	}
}		/*end f_read_print_front*/

EXPORT	boolean	f_read_print_max_front_speed_info(
	INIT_DATA       *init,
	const IO_TYPE   *io_type,
	Front	        *front,
	MAX_FRONT_SPEED	*mfs)
{
	INTERFACE *intfc = front->interf;
	FILE	  *file = io_type->file;
	int	  i, j, dim = intfc->dim;
	int	  c;
	char	  s[80];

	debug_print("restart","Entered f_read_print_max_front_speed_info()\n");

	if (fgetstring(file,"Maximum Front Speed Information")==FUNCTION_FAILED)
	{
	    (void) printf("WARNING in f_read_print_max_front_speed_info(), "
			  "can't find Maximum Front Speed Information\n");
	    debug_print("restart","Left f_read_print_max_front_speed_info()\n");
	    return FUNCTION_FAILED;
	}
	if (fgetstring(file,"Spfr = ") == FUNCTION_FAILED)
	{
	    (void) printf("WARNING in f_read_print_max_front_speed_info(), "
			  "can't find Spfr = \n");
	    debug_print("restart","Left f_read_print_max_front_speed_info()\n");
	    return FUNCTION_FAILED;
	}
	if ((c = getc(file)) == '\f') /*BINARY*/
	{
	    (void) getc(file);
	    (void) read_binary_real_array(mfs->_spfr,dim+1,io_type);
	}
	else
	{
	    (void) ungetc(c,file);
	    (void) fscan_float(file,mfs->_spfr);
	    for (i = 1; i <= dim; ++i)
	    {
	    	(void) getc(file);/*Grab comma*/
	    	(void) fscan_float(file,mfs->_spfr+i);
	    }
	}
	if (debugging("restart"))
	    (void) print_general_vector("mfs->_spfr = ",mfs->_spfr,dim+1,"\n");
	for (i = 0; i <= dim; ++i)
	{
	    (void) sprintf(s,"MaxFrontSpeedCoords[%d] = ",i);
	    if (fgetstring(file,s) == FUNCTION_FAILED)
	    {
	        (void) printf("WARNING in f_read_print_max_front_speed_info(), "
			      "can't find %s\n",s);
	        debug_print("restart","Left f_read_print_max_front_speed_info()\n");
	        return FUNCTION_FAILED;
	    }
	    if ((c = getc(file)) == '\f') /*BINARY*/
	    {
	    	(void) getc(file);
		(void) read_binary_real_array(mfs->_coords[i],dim,io_type);
	    }
	    else
	    {
	    	(void) ungetc(c,file);
	    	(void) fscan_float(file,mfs->_coords[i]);
	    	for (j = 1; j < dim; ++j)
	    	{
	    	    (void) getc(file);/*Grab comma*/
	    	    (void) fscan_float(file,mfs->_coords[i]+j);
	    	}
	    }
	}
	if (size_of_state(intfc) > 0)
	{
	    for (i = 0; i <= dim; ++i)
	    {
		(void) sprintf(s,"MaxFrontSpeedState[%d] = ",i);
		if (fgetstring(file,s) == FUNCTION_FAILED)
		{
	            (void) printf("WARNING in "
				  "f_read_print_max_front_speed_info(), "
			          "can't find %s\n",s);
	            debug_print("restart",
			  "Left f_read_print_max_front_speed_info()\n");
	            return FUNCTION_FAILED;
		}
		(void) read_print_state_data(init,io_type,
		                             mfs->_mxspst[i],intfc);
	    }
	}
	MaxFrontSpeed(front) = mfs;
	debug_print("restart","Left f_read_print_max_front_speed_info()\n");
	return FUNCTION_SUCCEEDED;
}		/*end f_read_print_max_front_speed_info*/

LOCAL	void	read_print_FlowSpecifiedRegion_list(
	INIT_DATA     *init,
	const IO_TYPE *io_type)
{
	FlowSpecifiedRegion *fsr;
	FILE                *file = io_type->file;
	int                 n, num_regions;

	if (next_output_line_containing_string(file,
			"FLOW SPECIFIED REGIONS DATA LIST") == NULL)
		return;

	(void) fgetstring(file,"Number of regions = ");
	(void) fscanf(file,"%d",&num_regions);

	/* Initialize doubly linked list */
	for (n = 0; n < num_regions; ++n)
	{
	    if (!read_print_FlowSpecifiedRegion_data(init,io_type,&fsr))
	    {
	        screen("ERROR in read_print_FlowSpecifiedRegion_list(), "
	               "can't read data printout\n");
	        clean_up(ERROR);
	    }
	}
}		/*end read_print_FlowSpecifiedRegion_list*/

LOCAL	boolean	f_read_print_FlowSpecifiedRegion_data(
	INIT_DATA	    *init,
	const IO_TYPE       *io_type,
	FlowSpecifiedRegion **pfsr)
{
	FILE	                   *file = io_type->file;
	static FlowSpecifiedRegion Fsr;

	(void) fgetstring(file,"comp = ");
	(void) fscanf(file,"%d",&Fsr.comp);
	(void) fgetstring(file,"type = ");
	(void) fscanf(file,"%s",Fsr.type);

	if (strcmp(Fsr.type,"CONSTANT_REGION") == 0)
	    return read_print_ConstantFlowRegion(init,io_type,
	                                         restart_intfc(init),
						 &Fsr,pfsr);
	if (strcmp(Fsr.type,"SKIP_COMPONENT_REGION") == 0)
	{
	    *pfsr = SetSkipComponentRegion(Fsr.comp);
	    return YES;
	}
	if (strcmp(Fsr.type,"SKIP_ALL_COMPONENTS") == 0)
	{
	    *pfsr = SetSkipAllComponents();
	    return YES;
	}

	*pfsr = &Fsr;
	return NO;
}		/*end f_read_print_FlowSpecifiedRegion_data*/

LOCAL	boolean	read_print_ConstantFlowRegion(
	INIT_DATA           *init,
	const IO_TYPE       *io_type,
	INTERFACE	    *intfc,
	FlowSpecifiedRegion *fsr,
	FlowSpecifiedRegion **pfsr)
{
	Locstate state = read_print_state_data(init,io_type,NULL,intfc);

	*pfsr = &SetConstantFlowRegion(fsr->comp,state,intfc)->Fsr;

	return YES;
}		/*end read_print_ConstantFlowRegion*/


LOCAL void f_prompt_for_surface_redist_options(
	INIT_DATA   *init)
{
	char		s[Gets_BUF_SIZE];
	double		msts_len, max_fac, min_fac, a;
	static const char	*FMT = "%lf %lf";

	debug_print("init","Entered f_prompt_for_surface_redist_options()\n");

	screen("\n\t\tsurface redistribution control\n\n");

	screen("Enter tracking algorithm, choices are:\n");
	screen("\tGrid free tracking(F%s),\n",
		(tracking_algorithm(init)==GRID_FREE_TRACKING)?", dflt":"");
	screen("\tGrid based tracking (G%s),\n",
		(tracking_algorithm(init)==GRID_BASED_TRACKING)?", dflt":"");
	screen("\tThree component grid based tracking (T%s),\n",
		(tracking_algorithm(init)==THREE_COMP_GRID_BASED_TRACKING)?", dflt":"");
	screen("\tMixed strategy tracking (M%s),\n",
		(tracking_algorithm(init)==MIXED_TRACKING)?", dflt":"");
	screen("\tHybrid strategy tracking (H%s),\n",
	       (tracking_algorithm(init)==HYBRID_TRACKING)?", dflt":"");
	screen("\tLocally grid based tracking (L%s),\n",
	       (tracking_algorithm(init)==LOCALLY_GRID_BASED_TRACKING)?", dflt":"");
	screen("Enter choice: ");
	(void) Gets(s);
	switch (s[0])
	{
	case 'F':
	case 'f':
	    tracking_algorithm(init) = GRID_FREE_TRACKING;
	    break;
	case 'M':
	case 'm':
	    tracking_algorithm(init) = MIXED_TRACKING;
	    break;
	case 'G':
	case 'g':
	    tracking_algorithm(init) = GRID_BASED_TRACKING;
	    break;
	case 'T':
	case 't':
	    tracking_algorithm(init) = THREE_COMP_GRID_BASED_TRACKING;
	    break;
	case 'H':
	case 'h':
	    tracking_algorithm(init) = HYBRID_TRACKING;
	    break;
	case 'L':
	case 'l':
	    tracking_algorithm(init) = LOCALLY_GRID_BASED_TRACKING;
	    break;
	default:
	    break;
	}
	if (tracking_algorithm(init) == GRID_BASED_TRACKING)
	    return;
	if (tracking_algorithm(init) == THREE_COMP_GRID_BASED_TRACKING)
	    return;

	if (front_redist_mode(init) != NO_REDIST)
	    front_redist_mode(init) = FULL_REDIST;
	screen("\nEnter the mode of surface redistribution --\n\t"
	       "`none'%s, or `full'%s: ",
	       (front_redist_mode(init) == NO_REDIST) ? " (dflt)" : "",
	       (front_redist_mode(init) == FULL_REDIST) ? " (dflt)" : "");
	(void) Gets(s);
	switch (s[0])
	{
	case 'n':					/* None */
	case 'N':
	    front_redist_mode(init) = NO_REDIST;
		break;
	case 'f':					/* Full */
	case 'F':
	    front_redist_mode(init) = FULL_REDIST;
	    break;
	default:
	    break;
	}


	screen("Enter the maximum and minimum triangle area factors\n\t"
	       "for general waves (dflt = %g %g): ",
	       maximum_triangle_area_factor(init,GENERAL_WAVE),
	       minimum_triangle_area_factor(init,GENERAL_WAVE));
	(void) Gets(s);
	if (s[0] != '\0')
	{
	    (void) sscanf(s,FMT,&max_fac,&min_fac);
	    maximum_triangle_area_factor(init,GENERAL_WAVE) = max_fac;
	    minimum_triangle_area_factor(init,GENERAL_WAVE) = min_fac;
	}

	screen("Enter the maximum and minimum triangle area factors\n\t"
	       "for vector waves (dflt = %g %g): ",
	       maximum_triangle_area_factor(init,VECTOR_WAVE),
	       minimum_triangle_area_factor(init,VECTOR_WAVE));
	(void) Gets(s);
	if (s[0] != '\0')
	{
	    (void) sscanf(s,FMT,&max_fac,&min_fac);
	    maximum_triangle_area_factor(init,VECTOR_WAVE) = max_fac;
	    minimum_triangle_area_factor(init,VECTOR_WAVE) = min_fac;
	}

	screen("Enter the minimum allowed angle (in degrees)\n\t"
	       "at a triangle vertex on a general surface (dflt = %g): ",
	       degrees(minimum_angle_at_triangle_vertex(init,GENERAL_WAVE)));
	(void) Gets(s);
	if (s[0] != '\0')
	{
	    (void) sscan_float(s,&a);
	    minimum_angle_at_triangle_vertex(init,GENERAL_WAVE) = radians(a);
	}

	screen("Enter the minimum allowed angle (in degrees)\n\t"
	       "at a triangle vertex on a vector surface (dflt=%g): ",
	       minimum_angle_at_triangle_vertex(init,VECTOR_WAVE));
	(void) Gets(s);
	if (s[0] != '\0')
	{
	    (void) sscan_float(s,&a);
	    minimum_angle_at_triangle_vertex(init,VECTOR_WAVE) = radians(a);
	}

	screen("Enter the max_scaled_tri_side_length (dflt = %g): ",
	       maximum_scaled_triangle_side_length(init));
	(void) Gets(s);
	if (s[0] != '\0')
	{
	    (void) sscan_float(s,&msts_len);
	    maximum_scaled_triangle_side_length(init) = msts_len;
	}

	init_redistribution_frequency(init);


	debug_print("init","Left f_prompt_for_surface_redist_options()\n");
}		/* end to f_prompt_for_surface_redist_options() */


LOCAL void f_init_surface_redistribution_parameters(
	INIT_DATA	*init,
	Front		*front)
{
	double		*h = front->rect_grid->h;
	double		std_area, max_fac, min_fac;
	double		a;
	double		sslen;
	double		hmin;

	debug_print("init","Entered f_init_surface_redistribution_parameters()\n");

	hmin = h[0];
	if (hmin > h[1])
	    hmin = h[1];
	if (hmin > h[2])
	    hmin = h[2];

	Tracking_algorithm(front) = tracking_algorithm(init);
	SurfaceRedistributionOptions(front) = surface_redist_options(init);

	Surface_redistribution_function(front) = NULL;

	if (Tracking_algorithm(front) == GRID_BASED_TRACKING)
	    return;
	if (Tracking_algorithm(front) == THREE_COMP_GRID_BASED_TRACKING)
	    return;

	Surface_redistribution_function(front) = surface_redistribute;
	sslen = Front_spacing(front,GENERAL_WAVE);
	std_area = 0.25*sqrt(3.0)*sqr(sslen)*sqr(hmin); /*TOLERANCE*/

	max_fac = maximum_triangle_area_factor(init,GENERAL_WAVE);
	min_fac = minimum_triangle_area_factor(init,GENERAL_WAVE);
	Max_bond_len(front,GENERAL_WAVE) = sslen*sqrt(max_fac)*hmin;
	Min_bond_len(front,GENERAL_WAVE) = sslen*sqrt(min_fac)*hmin;
	Max_tri_sqr_area(front,GENERAL_WAVE) = sqr(max_fac*std_area);
	Min_tri_sqr_area(front,GENERAL_WAVE) = sqr(min_fac*std_area);

	max_fac = maximum_triangle_area_factor(init,VECTOR_WAVE);
	min_fac = minimum_triangle_area_factor(init,VECTOR_WAVE);
	Max_bond_len(front,VECTOR_WAVE) = sslen*sqrt(max_fac)*hmin;
	Min_bond_len(front,VECTOR_WAVE) = sslen*sqrt(min_fac)*hmin;
	Max_tri_sqr_area(front,VECTOR_WAVE) = sqr(max_fac*std_area);
	Min_tri_sqr_area(front,VECTOR_WAVE) = sqr(min_fac*std_area);

	/*
	* The aspect ratio of a triangle is defined as
	* aspect ratio = area/(sum of squares of sides).
	* The formula used below is the maximum aspect ratio for a
	* triangle with angle a.  Triangles with aspect ratios greater
	* than this value will have all angles greater than a.
	*/
	a = minimum_angle_at_triangle_vertex(init,GENERAL_WAVE);
	Aspect_ratio_tolerance(front,GENERAL_WAVE) = 0.5*sin(a)/(2.0 - cos(a));

	a = minimum_angle_at_triangle_vertex(init,GENERAL_WAVE);
	Aspect_ratio_tolerance(front,VECTOR_WAVE) = 0.5*sin(a)/(2.0 - cos(a));

	/* The following is used in tri_status in fredist3d.c */
	/* TO DO:  reassess 3d redistibution parameters */

	Max_scaled_tri_side_sqr_length(front) =
            maximum_scaled_triangle_side_length(init)
            * maximum_scaled_triangle_side_length(init);

	set_redistribution_frequency(init,front);

	debug_print("init","Left f_init_surface_redistribution_parameters()\n");
}		/*end f_init_surface_redistribution_parameters*/


/*
*			init_front_states():
*
*	Initializes the states on the front by looping over all points
*	on front->interf and calling
*
*	(*front_initializer)(point,bond,curve,left_state,right_state,init)
*
*	to fill the Locstate's associated with the point on the left and
*	right of the curve.
*/

EXPORT void init_front_states(
	Front		*front,
	INIT_DATA	*init,
	void		(*front_initializer)(FT_POINT*,HYPER_SURF_ELEMENT*,
					     HYPER_SURF*,Locstate,Locstate,
					     INIT_DATA*))
{
	int dim = front->interf->dim;

	switch (dim)
	{
	case 1:
	    init_1d_front_states(front,init,front_initializer);
	    break;
	case 2:
	    init_2d_front_states(front,init,front_initializer);
	    break;
	case 3:
	    init_3d_front_states(front,init,front_initializer);
	    break;
	}
}		/*end init_front_states*/


LOCAL void init_1d_front_states(
	Front		*front,
	INIT_DATA	*init,
	void		(*front_initializer)(FT_POINT*,HYPER_SURF_ELEMENT*,
					     HYPER_SURF*,Locstate,Locstate,
					     INIT_DATA*))
{
	FT_POINT		**p;
	INTERFACE	*intfc = front->interf;

	debug_print("init","Entered init_1d_front_states()\n");
	if ((front->sizest == 0) || (front_initializer == NULL))
	{
	    debug_print("init","Left init_1d_front_states()\n");
	    return;
	}


	for (p = intfc->points; p && *p; ++p)
	{
	    debug_print("init","Initializing states on point %d\n",p);
	    (*front_initializer)(*p,(HYPER_SURF_ELEMENT *) NULL,
			         Hyper_surf(*p),left_state(*p),
				 right_state(*p),init);
	}
	debug_print("init","Left init_1d_front_states()\n");
}		/*end init_1d_front_states*/


LOCAL void init_2d_front_states(
	Front		*front,
	INIT_DATA	*init,
	void		(*front_initializer)(FT_POINT*,HYPER_SURF_ELEMENT*,
					     HYPER_SURF*,Locstate,Locstate,
					     INIT_DATA*))
{
	BOND		*b;
	CURVE		*c;
	INTERFACE	*intfc = front->interf;

	debug_print("init","Entered init_2d_front_states()\n");
	if (front->sizest == 0 || front_initializer == NULL)
	{
	    debug_print("init","Left init_2d_front_states()\n");
	    return;
	}

	(void) next_curve(intfc,NULL);
	while (next_curve(intfc,&c))
	{
	    if (debugging("init"))
	    	(void) printf("Initializing states on curve %llu\n",
	   	              curve_number(c));

	    (*front_initializer)(c->first->start,
	   	                 Hyper_surf_element(c->first),Hyper_surf(c),
	   	                 left_start_state(c),right_start_state(c),
				 init);
	    for (b = c->first; b != c->last; b = b->next)
	    	(*front_initializer)(b->end,
				     Hyper_surf_element(b),Hyper_surf(c),
	    		             left_state(b->end),right_state(b->end),
				     init);
	    (*front_initializer)(c->last->end,Hyper_surf_element(c->last),
	   	                 Hyper_surf(c),left_end_state(c),
				 right_end_state(c),init);
	}
	if (debugging("init_front_states"))
	{
	    print_interface(front->interf);
	    show_intfc_states(front->interf);
	}

	debug_print("init","Left init_2d_front_states()\n");
}		/*end init_2d_front_states*/

LOCAL void init_3d_front_states(
	Front		*front,
	INIT_DATA	*init,
	void		(*front_initializer)(FT_POINT*,HYPER_SURF_ELEMENT*,
					     HYPER_SURF*,Locstate,Locstate,
					     INIT_DATA*))
{
	FT_POINT		*p;
	CURVE		**c;
	HYPER_SURF_ELEMENT *hse;
	HYPER_SURF	*hs;
	Locstate	sl, sr;
	INTERFACE	*intfc = front->interf;
	BOND_TRI 	**btris;
	BOND 		*bond;
	int 		i;

	debug_print("init","Entered init_3d_front_states()\n");
	if (front->sizest == 0 || front_initializer == NULL)
	{
	    debug_print("init","Left init_3d_front_states()\n");
	    return;
	}

	for (c = intfc->curves; c && *c; ++c)
	{
	    i = 0;
	    bond = (*c)->first;
	    if ((btris = Btris(bond)) == NULL) continue;
	    while (btris[i])
	    {
		p = bond->start;
		hse = Hyper_surf_element(btris[i]->tri);
		hs = Hyper_surf(Surface_of_tri(btris[i]->tri));
	    	sl = left_start_btri_state(btris[i]);
	    	sr = right_start_btri_state(btris[i]);
	    	(*front_initializer)(p,hse,hs,sl,sr,init);
		for (; bond; bond = bond->next)
		{
		    btris = Btris(bond);
		    p = bond->end;
		    hse = Hyper_surf_element(btris[i]->tri);
		    sl = left_end_btri_state(btris[i]);
		    sr = right_end_btri_state(btris[i]);
	    	    (*front_initializer)(p,hse,hs,sl,sr,init);
		}
	    	bond = (*c)->first;
		btris = Btris(bond);
		++i;
	    }
	}
	(void) next_point(intfc,NULL,NULL,NULL);
	while (next_point(intfc,&p,&hse,&hs))
	{
	    slsr(p,hse,hs,&sl,&sr); 
	    (*front_initializer)(p,hse,hs,sl,sr,init);
	}

	debug_print("init","Left init_3d_front_states()\n");
}		/*end init_3d_front_states*/

EXPORT  void    FT_StartUp(
        Front *front,
	F_BASIC_DATA *ft_basic)
{
#if defined(PP_MODE)
       	PP_GRID *pp_grid = NULL;
#endif /* defined(PP_MODE) */
       	IMPORT boolean suppress_prompts;
       	int i,j,dim;
        static const int DEFAULT_BUF_WIDTH = 3;
        static RECT_GRID comp_grid;
        static F_INIT_DATA Init;
	int **rect_bdry_type;

	start_clock("FT_StartUp");
        setbuf(stdin,NULL);
        init_clean_up(NULL,NULL);

	if(ft_basic == NULL) /* call build-in functions to prompt data */
	{
        	suppress_prompts = YES;
		Init.use_default_front_data = YES;

		screen("Welcome to the World of Partial Interfaces!\n");
		init_prompting_and_debugging(init_data(&Init));
        	set_binary_output(NO);
		screen("Enter the interface dimension: ");
		(void) Scanf("%d\n",&dim);

        	for (i = 0; i < dim; ++i)
        	{
            		subdomains(init_data(&Init))[i] = 1;
            		buffer_zones(init_data(&Init))[i] = DEFAULT_BUF_WIDTH 
					+ MAXD/3;
        	}

        	/* Init rectangular grids */
		comp_grid.dim = dim;
		i_init_remap_and_rect_grid(&comp_grid);
        	front->rect_grid = &comp_grid;

		Init._StateSize = front->sizest;
		set_size_of_intfc_state(front->sizest);
        	f_set_interface_hooks(dim,init_data(&Init));

        	/* Init front interface */

        	i_intfc(&Init) = front->interf = make_interface(dim);
		bi_array(&rect_bdry_type,MAXD,2,INT);
        	init_rect_bdry_type(rect_bdry_type,front->rect_grid);

        	copy_rect_grid(computational_grid(front->interf),
                        front->rect_grid);
        	set_test_front(&Init,front);
		delete_interface(front->interf);
		set_size_of_intfc_state(front->sizest);
        	i_intfc(&Init) = front->interf = make_interface(dim);
		for (i = 0; i < dim; ++i)
		{
		    for (j = 0; j < 2; ++j)
		    {
			rect_boundary_type(front->interf,i,j) = 
				rect_bdry_type[i][j];
		    }
		}
		free(rect_bdry_type);
	}
	else 	/* use supplied data to initialize front */
	{
		Init.use_default_front_data = NO;
		init_default_debugging(init_data(&Init));
        	set_binary_output(NO);
		dim = ft_basic->dim;
		front->out_name = ft_basic->out_name;

        	for (i = 0; i < dim; ++i)
        	{
            		subdomains(init_data(&Init))[i] = 1;
            		buffer_zones(init_data(&Init))[i] = DEFAULT_BUF_WIDTH 
					+ MAXD/3;
#if defined(__MPI__)
                        subdomains(init_data(&Init))[i] =
                                        ft_basic->subdomains[i];
#endif /* defined(__MPI__) */
        	}
		Init._StateSize = front->sizest = 
				ft_basic->size_of_intfc_state;
		set_size_of_intfc_state(front->sizest);

        	f_set_interface_hooks(dim,init_data(&Init));
		if (ft_basic->RestartRun)
		{
		    IO_TYPE io_type;
		    int grid_set;
		    FILE *rfile;
		    comp_grid.dim = dim;
		    set_remap_and_rect_grid(ft_basic->L,ft_basic->U,
			    ft_basic->gmax,IDENTITY_REMAP,&comp_grid);
        	    front->rect_grid = &comp_grid;
		    rfile = fopen(ft_basic->restart_name,"r");
		    determine_io_type(rfile,&io_type);
		    read_print_front_time_and_step(front,io_type.file);
		    i_intfc(&Init) = front->interf = 
		    	read_print_interface((INIT_DATA*)&Init,&io_type,
					NO,&grid_set);
        	    set_default_front(&Init,front);
		    set_size_of_intfc_state(front->sizest);
		}
		else
		{
        	    /* Init rectangular grids */
		    comp_grid.dim = dim;
		    set_remap_and_rect_grid(ft_basic->L,ft_basic->U,
			    ft_basic->gmax,IDENTITY_REMAP,&comp_grid);
        	    front->rect_grid = &comp_grid;

        	    /* Init front interface */

        	    i_intfc(&Init) = front->interf = make_interface(dim);

        	    copy_rect_grid(computational_grid(front->interf),
                            front->rect_grid);
        	    set_default_front(&Init,front);
		    delete_interface(front->interf);
		    set_size_of_intfc_state(front->sizest);
        	    i_intfc(&Init) = front->interf = make_interface(dim);
		    copy_rect_grid(computational_grid(front->interf),
		        	    front->rect_grid);
		    for (i = 0; i < dim; ++i)
		    {
		        for (j = 0; j < 2; ++j)
		        {
			    rect_boundary_type(front->interf,i,j) = 
				    ft_basic->boundary[i][j];
		        }
		    }
		}
	}

        /* front hook up and initialization */
        front->_tan_point_propagate = NULL;
        front->_reconstruct_front_at_grid_crossing =
                        rebuild_intfc_at_crossings3d;
        front->_repair_front_at_grid_crossing =
                        repair_intfc_at_crossings3d;

	front->hdf_movie_var = NULL;
	set_topological_grid(front->interf,front->rect_grid);
	if (ft_basic != NULL && ft_basic->RestartRun)
	{
	    int rbt[3][2];
	    for (i = 0; i < dim; ++i)
	    {
	    	rbt[i][0] = rect_boundary_type(front->interf,i,0);
	    	rbt[i][1] = rect_boundary_type(front->interf,i,1);
	    }
	    pp_clip_rect_grids(front,rbt);
	}
	stop_clock("FT_StartUp");
	return;
}       /* end FT_StartUp */

EXPORT	void FT_InitIntfc(
        Front  *front,
	LEVEL_FUNC_PACK *level_func_pack)
{
	switch (front->rect_grid->dim)
	{
	case 1:
	    FT_InitIntfc1d(front,level_func_pack);
	    break;
	case 2:
	    FT_InitIntfc2d(front,level_func_pack);
	    break;
	case 3:
	    FT_InitIntfc3d(front,level_func_pack);
	    break;
	default: 
	    screen("Unknown dimension!\n");
	    clean_up(ERROR);
	}
}	/* end FT_InitIntfc */

/*ARGSUSED*/
LOCAL   void FT_InitIntfc1d(
	Front *front,
        LEVEL_FUNC_PACK *level_func_pack)
{
	RECT_GRID *gr = front->rect_grid;
	INTERFACE *intfc = front->interf;
        double **points = level_func_pack->point_array;
        FT_POINT *p;
        const double eps = 10.0*MACH_EPS;

	if (level_func_pack->num_points == 1 &&
	    level_func_pack->point_array != NULL)
	{
            p = make_point(points[0],level_func_pack->neg_component,
                                 level_func_pack->pos_component);
                if (level_func_pack->wave_type != UNKNOWN_WAVE_TYPE)
                    wave_type(p) = level_func_pack->wave_type;
                else
                    wave_type(p) = FIRST_PHYSICS_WAVE_TYPE;
	}
	else
	    intfc->default_comp = level_func_pack->pos_component;

        set_topological_grid(intfc,computational_grid(intfc));

        (void) set_boundary(intfc,&topological_grid(intfc),
                            intfc->default_comp,eps);
}       /* end FT_InitIntfc1d */

LOCAL   void FT_InitIntfc2d(
	Front *front,
	LEVEL_FUNC_PACK *level_func_pack)
{
	RECT_GRID *gr = front->rect_grid;
	INTERFACE *intfc = front->interf;
	char s[10];
	const double eps = 10.0*MACH_EPS;
	CURVE **c,*curve;
	COMPONENT left_c,right_c;
	CURVE **curves;
	int num_segs;

	copy_rect_grid(computational_grid(intfc),gr);

	if (level_func_pack == NULL)
	{
	    prompt_make_level_curves(intfc,gr,&left_c,&right_c);
	}
	else if (level_func_pack->func != NULL)
	{
	    double (*func)(POINTER,double*);
	    POINTER func_params;
	    left_c = level_func_pack->neg_component;
	    right_c = level_func_pack->pos_component;
	    func = level_func_pack->func;
	    func_params = level_func_pack->func_params;
	    curves = make_level_curves(gr,intfc,left_c,right_c,func,
				func_params,NO,&num_segs);
	}
	else if (level_func_pack->point_array != NULL)
	{
	    double **point_array = level_func_pack->point_array;
	    int num_points = level_func_pack->num_points;
	    boolean is_closed_curve = level_func_pack->is_closed_curve;
	    left_c = level_func_pack->neg_component;
	    right_c = level_func_pack->pos_component;
	    if (!make_array_curve(intfc,left_c,right_c,num_points,
	    		point_array,is_closed_curve))
            {
            	screen("make_array_curve() failed!\n");
            	clean_up(ERROR);
	    }
	}
	else
	{
	    intfc->default_comp = level_func_pack->pos_component;
	}

	set_topological_grid(intfc,computational_grid(intfc));

	(void) set_boundary(intfc,&topological_grid(intfc),
			    intfc->default_comp,eps);
	for (c = intfc->curves; c && *c; ++c)
	{
	    if (wave_type(*c) != UNKNOWN_WAVE_TYPE) 
		continue;	/* Already assigned */
	    if (negative_component(*c) == left_c &&
		positive_component(*c) == right_c)
	    {
		if (level_func_pack == NULL) 
	    	    wave_type(*c) = FIRST_PHYSICS_WAVE_TYPE;
		else if (level_func_pack->wave_type != UNKNOWN_WAVE_TYPE)
		{
		    wave_type(*c) = level_func_pack->wave_type;
		}
		else
	    	    wave_type(*c) = FIRST_PHYSICS_WAVE_TYPE;

		if ((*c)->start == (*c)->end)
		    node_type((*c)->start) = CLOSED_NODE;
		else if (negative_component(*c) == positive_component(*c))
		{
		    node_type((*c)->start) = MONO_COMP_NODE;
                    node_type((*c)->end) = MONO_COMP_NODE;
		}
	    	else
		{
		    set_boundary_node_type((*c)->start,intfc);
		    set_boundary_node_type((*c)->end,intfc);
		}
		if (!is_bdry(*c))
		    start_status(*c) = end_status(*c) = INCIDENT;
		else
		    start_status(*c) = end_status(*c) = FIXED;
	    }
	}

	rect_bdry_redist2d(intfc,computational_grid(intfc),0);
}	/* end FT_InitIntfc2d */

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


LOCAL   void FT_InitIntfc3d(
	Front *front,
	LEVEL_FUNC_PACK *level_func_pack)
{
	RECT_GRID *gr = front->rect_grid;
	INTERFACE *intfc = front->interf;
	SURFACE **s,*surf = NULL;
	COMPONENT neg_comp,pos_comp;
	const double eps = 10.0*MACH_EPS;

	clip_front_to_subdomain(front);

	if (level_func_pack == NULL)
	{
	    surf = prompt_make_level_surface(intfc,gr);
	    interface_reconstructed(intfc) = YES;
	}
	else if (level_func_pack->read_sdl_input)
	{
	    neg_comp = level_func_pack->neg_component;
	    pos_comp = level_func_pack->pos_component;
	    if (!read_sdl_surface(intfc,neg_comp,pos_comp,
	    		level_func_pack->sdl_name,&surf))
	    {
		screen("read_sdl_surface() failed!\n");
		clean_up(ERROR);
	    }
	}
	else if (level_func_pack->read_vtk_input)
	{
	    neg_comp = level_func_pack->neg_component;
	    pos_comp = level_func_pack->pos_component;
	    if (!read_vtk_surface(intfc,neg_comp,pos_comp,
	    		level_func_pack->vtk_name,&surf))
	    {
		screen("read_sdl_surface() failed!\n");
		clean_up(ERROR);
	    }
	}
	else if (level_func_pack->is_mono_hs)
	{
	    int i;
	    for (i = 0; i < level_func_pack->num_mono_hs; ++i)
	    {
            	pos_comp = level_func_pack->pos_component;
	    	neg_comp = level_func_pack->pos_component+1;
	    	if (!make_level_surface(gr,intfc,neg_comp,pos_comp,
			level_func_pack->func,level_func_pack->func_params,
			&surf))
	    	{
		    screen("make_level_surface() failed!\n");
		    clean_up(ERROR);
	    	}
	    	cut_surface(surf,level_func_pack->constr_func,
			level_func_pack->constr_params,YES);
	    	install_hsbdry_on_surface(surf,MONO_COMP_HSBDRY);
	    	negative_component(surf) = positive_component(surf);
	    	if (level_func_pack->attach_string == YES)
	    	{
		    (*level_func_pack->string_func)(intfc,surf,
				level_func_pack->string_params,i);
	    	}
		interface_reconstructed(intfc) = NO;
	    	if (level_func_pack->wave_type != UNKNOWN_WAVE_TYPE)
		    wave_type(surf) = level_func_pack->wave_type;
		else
	    	    wave_type(surf) = FIRST_PHYSICS_WAVE_TYPE;
	    }
	}
	else if (level_func_pack->func != NULL)
	{
	    double (*func)(POINTER,double*);
	    POINTER func_params;
	    neg_comp = level_func_pack->neg_component;
	    pos_comp = level_func_pack->pos_component;
	    func = level_func_pack->func;
	    func_params = level_func_pack->func_params;
	    if (!make_level_surface(gr,intfc,neg_comp,pos_comp,func,
			func_params,&surf))
	    {
		screen("make_level_surface() failed!\n");
		clean_up(ERROR);
	    }
	}
	else
	{
	    intfc->default_comp = level_func_pack->pos_component;
	}
	if (surf)
	{
	    if (level_func_pack->wave_type != UNKNOWN_WAVE_TYPE)
	    	wave_type(surf) = level_func_pack->wave_type;
	    else
	    	wave_type(surf) = FIRST_PHYSICS_WAVE_TYPE;
	}

	set_topological_grid(intfc,computational_grid(intfc));

	scatter_front(front);
	if (level_func_pack->set_3d_bdry)
	{
	    (void) set_boundary(front->interf,front->rect_grid,
			front->interf->default_comp,eps);
	}
	scatter_front(front);
}	/* end FT_InitIntfc3d */


LOCAL	void init_rect_bdry_type(
	int **rect_boundary_type,
	RECT_GRID *rgr)
{
	char btype[256],mesg[256];
	static const char *direction[3] = { "x", "y", "z"};
        static const char *side[3][2] = { {"left", "right"},
                                          {"lower", "upper"},
                                          {"bottom", "top"}
                                        };
	int i,j;
	boolean need_pp_grid = NO;

	screen("Available rectangular boundary types are\n"
	       "\tPeriodic (p)\n"
	       "\tReflection (r)\n"
	       "\tDirichlet (d)\n"
	       "\tNeumann (n)\n");
	for (i = 0; i < rgr->dim; ++i)
	{
	    for (j = 0; j < 2; ++j)
	    {
		(void) sprintf(mesg,"for the %s boundary in the %s direction",
                               side[i][j],direction[i]);
		screen("Enter boundary type %s: ",mesg);
		Scanf("%s\n",btype);
		switch (btype[0])
		{
		case 'P':
		case 'p':
		    rect_boundary_type[i][j] = PERIODIC_BOUNDARY;
		    rect_boundary_type[i][j+1] = PERIODIC_BOUNDARY;
		    ++j;
		    break;
		case 'D':
		case 'd':
		    rect_boundary_type[i][j] = DIRICHLET_BOUNDARY;
		    break;
		case 'R':
		case 'r':
		    rect_boundary_type[i][j] = REFLECTION_BOUNDARY;
		    break;
		case 'N':
		case 'n':
		    rect_boundary_type[i][j] = NEUMANN_BOUNDARY;
		    break;
		}
	    }
	}

}	/* end init_rect_bdry_type */


LOCAL void read_print_front_time_and_step(
	Front *front,
	FILE *file)
{
	fgetstring(file,"Time Data");
	fgetstring(file,"t = ");
	fscanf(file,"%lf",&front->time);
	fgetstring(file,"j = ");
	fscanf(file,"%d",&front->step);
	fgetstring(file,"dt = ");
	fscanf(file,"%lf",&front->dt);
}	/* end read_print_front_time_and_step */
