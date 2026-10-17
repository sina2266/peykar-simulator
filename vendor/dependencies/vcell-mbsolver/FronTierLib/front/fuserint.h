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
*				fuserint.h:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*			User Supplied Operations
*/

#if !defined(_FUSERINT_H)
#define _FUSERINT_H

#include <front/fdecs.h>

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif


struct _F_POINT {
	FT_POINT		point;
	Locstate	_left_state;
	Locstate	_right_state;
};
typedef struct _F_POINT F_POINT;

	/* F_POINT access macros */
#define f_point(point)			((F_POINT *) (point))
#define left_state(point)		(f_point(point)->_left_state)
#define right_state(point)		(f_point(point)->_right_state)
#define	n_pt_propagated(point)		(Point_flags(point)._user0)
#define	t_pt_propagated(point)		(Point_flags(point)._user1)

struct _F_BOND_TRI {
	BOND_TRI	bond_tri;
	Locstate	_left_start_btri_state;
	Locstate	_right_start_btri_state;
	Locstate	_left_end_btri_state;
	Locstate	_right_end_btri_state;
};
typedef struct _F_BOND_TRI F_BOND_TRI;

	/* F_BOND_TRI access macros */
#define f_bond_tri(btri)		((F_BOND_TRI *) (btri))
#define	left_start_btri_state(btri)	(f_bond_tri(btri)->_left_start_btri_state)
#define	right_start_btri_state(btri)	(f_bond_tri(btri)->_right_start_btri_state)
#define	left_end_btri_state(btri)	(f_bond_tri(btri)->_left_end_btri_state)
#define	right_end_btri_state(btri)	(f_bond_tri(btri)->_right_end_btri_state)

struct _TANGENT_FUNCTION {
	void       (*_tangent)(FT_POINT*,BOND*,CURVE*,double*,Front*);
	const char *_tangent_name;
};
typedef struct _TANGENT_FUNCTION TANGENT_FUNCTION;

struct _NORMAL_FUNCTION {
	void       (*_normal)(FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*,
	                      double*,Front*);
	const char *_normal_name;
};
typedef struct _NORMAL_FUNCTION NORMAL_FUNCTION;

struct _F_CURVE {
	CURVE	         curve;
	Locstate         _left_start_state,_left_end_state;
	Locstate         _right_start_state,_right_end_state;
	TANGENT_FUNCTION _curve_tangent_function;
        int _start_status;
        int _end_status;
};
typedef struct _F_CURVE F_CURVE;

	/* F_CURVE access macros */
#define	f_curve(curve)			((F_CURVE *) (curve))
#define left_start_state(curve)		(f_curve(curve)->_left_start_state)
#define left_end_state(curve)		(f_curve(curve)->_left_end_state)
#define right_start_state(curve)	(f_curve(curve)->_right_start_state)
#define right_end_state(curve)		(f_curve(curve)->_right_end_state)
#define curve_tangent_function(curve)					\
    f_curve(curve)->_curve_tangent_function
#define curve_tangent(curve)						\
    curve_tangent_function(curve)._tangent
#define curve_tangent_name(curve)					\
    curve_tangent_function(curve)._tangent_name
#define start_status(curve)     (f_curve(curve)->_start_status)
#define end_status(curve)       (f_curve(curve)->_end_status)
#define status_at_node(curve,orient)                                    \
        (((orient) == POSITIVE_ORIENTATION) ? start_status(curve) :     \
        end_status(curve))

struct _F_C_BOND {
	C_BOND cbond;
	Locstate _left_start_c_bond_state[2];
	Locstate _right_start_c_bond_state[2];
	Locstate _left_end_c_bond_state[2];
	Locstate _right_end_c_bond_state[2];
};
typedef struct _F_C_BOND F_C_BOND;

	/* F_C_BOND access macros */
#define	f_c_bond(cbond)		     ((F_C_BOND *) (cbond))
#define left_start_c_bond_state(cb)  (f_c_bond(cb)->_left_start_c_bond_state)
#define right_start_c_bond_state(cb) (f_c_bond(cb)->_right_start_c_bond_state)
#define left_end_c_bond_state(cb)    (f_c_bond(cb)->_left_end_c_bond_state)
#define right_end_c_bond_state(cb)   (f_c_bond(cb)->_right_end_c_bond_state)

	/* Possible values for propagation_status of a NODE */

enum _NODE_PROPAGATION_STATUS {
	PROPAGATION_STATUS_UNSET = INT_MIN,
	UNPROPAGATED_NODE = 1,
	VEL_COMPUTED_NODE,
	PROPAGATED_NODE,
	DELETED_NODE
};
typedef enum _NODE_PROPAGATION_STATUS  NODE_PROPAGATION_STATUS;

struct _F_NODE {
	NODE	                node;
	NODE	                *_prev;
	NODE	                *_next;
	NODE_PROPAGATION_STATUS	_propagation_status;
	double	                _v[MAXD];
	boolean                    _preserve_position_as_point;
};
typedef struct _F_NODE F_NODE;

	/* F_NODE access macros */
#define	f_node(node)			((F_NODE *) (node))
#define	prev_node(node)			(f_node(node)->_prev)
#define	next_node(node)			(f_node(node)->_next)
#define	propagation_status(node)	(f_node(node)->_propagation_status)
#define Node_vel(node)			(f_node(node)->_v)
#define preserve_position_as_point(node)				\
    (f_node(node)->_preserve_position_as_point)

struct _BOUNDARY_STATE {
	Locstate _boundary_state;
	void	 (*_boundary_state_function)(double*,HYPER_SURF*,Front*,
						POINTER,Locstate);
	POINTER  _boundary_state_data;
	void     (*_fprint_boundary_state_data)(FILE*,INTERFACE*,
					        struct _BOUNDARY_STATE*);
	char     *_boundary_state_function_name;
	POINTER  *_boundary_state_function_params;
};
typedef struct _BOUNDARY_STATE BOUNDARY_STATE;

struct _F_USER_INTERFACE {
	RECT_GRID	_computational_grid;
	BOUNDARY_STATE	**_bstates;
	int	_num_bstates;
	NODE	*_first_node;
	NODE	*_last_node;
	size_t	_sizest;
	boolean	_interpolate_intfc_states;
	boolean	_mono_comp_curves;
	void	(*_fprint_hsbdry_type)(FILE*,const char*,int,const char*,
				       INTERFACE*);
	int	(*_read_hsbdry_type_from_string)(const char*,INTERFACE*);
	void	(*_fprint_wave_type)(FILE*,const char*,int,const char*,
	                             INTERFACE*);
	const char *(*_wave_type_as_string)(int);
	int	(*_read_wave_type_from_string)(const char*);
	void	(*_fprint_state_data)(FILE*,Locstate,INTERFACE*);
	Locstate (*_read_print_state_data)(INIT_DATA*,const IO_TYPE*,
	                                   Locstate,INTERFACE*);
	boolean	(*_nearest_intfc_state)(double*,COMPONENT,INTERFACE*,Locstate,
					double*,HYPER_SURF**);
	void	(*_slsr)(FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*,
			 Locstate*,Locstate*);
	boolean	(*_tri_interpolate_intfc_states)(double,double,double,double*,
						 Locstate,double*,Locstate,
						 double*,Locstate,
						 RECT_GRID*,Locstate);
	void	(*_bi_interpolate_intfc_states)(double,double,double*,Locstate,
						double*,Locstate,RECT_GRID*,
						Locstate);
	void	(*_state_along_hypersurface_element)(COMPONENT,double*,
						     HYPER_SURF_ELEMENT*,
						     HYPER_SURF*,Locstate);
	boolean	(*_form_subintfc_via_communication)(Front*);

#if defined(USE_OVERTURE)
        boolean    (*_form_patch_subintfc_via_cut)(Front*);
        boolean    (*_form_patch_subintfc)(Front*,COMPONENT);
        void    (*_assembly_fine_patch_fronts_to_one)(Front**,Front*);
#endif /* if defined(USE_OVERTURE) */

	void	(*_reflect_state)(Locstate,INTERFACE*,double*,double*,double*);
	void	(*_fprint_intfc_state)(FILE*,Locstate,INTERFACE*);
	void	(*_fshow_intfc_states)(FILE*,INTERFACE*);
	void	(*_read_print_boundary_state_data)(INIT_DATA*,const IO_TYPE*,
	                                           INTERFACE*,int);
	double	(*_mean_curvature_at_point)(FT_POINT*,HYPER_SURF_ELEMENT*,
					    HYPER_SURF*,Front*);
	void	(*_alloc_state)(Locstate*,size_t);
	Locstate	(*_alloc_intfc_state)(size_t);
	void	(*_clear_state)(Locstate,size_t);
	void	(*_obstacle_state)(Locstate,size_t);
	boolean	(*_default_perform_redistribution_function)(HYPER_SURF*,Front*,
							    boolean);
	boolean (*_merge_hs_flags)(HYPER_SURF*,HYPER_SURF*);

		/* Front geometry */
	NORMAL_FUNCTION _first_order_normal_function;
	NORMAL_FUNCTION _interface_normal_function;
	void (*_set_normal_function)(const char*,NORMAL_FUNCTION*,INTERFACE*);
	TANGENT_FUNCTION _interface_tangent_function;
	void (*_set_tangent_function)(const char*,TANGENT_FUNCTION*,INTERFACE*);

	MAX_FRONT_SPEED	*(*_alloc_MaxFrontSpeed)(MAX_FRONT_SPEED*,INTERFACE*,
	                                         size_t);

	struct _F_INTERFACE_TOLERANCES {
		/*
		 * When a time step reduction is selected,  the fractional
		 * time step is always changed by at least this factor.
		 */
		double	_DtReductionFac;

		/*
		 * Curves with few than this number of points are always
		 * tagged as short
		 */
		int	_ShortCurveNumPoints;
	} _FInterfaceTolerances;

};
typedef struct _F_USER_INTERFACE F_USER_INTERFACE;

#if defined(__cplusplus)
typedef F_USER_INTERFACE::_F_INTERFACE_TOLERANCES F_INTERFACE_TOLERANCES;
#else /* defined(__cplusplus) */
typedef struct _F_INTERFACE_TOLERANCES F_INTERFACE_TOLERANCES;
#endif /* defined(__cplusplus) */

struct _F_INTERFACE {
	I_INTERFACE i_intfc;
	F_USER_INTERFACE f_user_intfc;
};
typedef struct _F_INTERFACE F_INTERFACE;

	/* F_INTERFACE access macros */
#define f_interface(intfc)		((F_INTERFACE *) (intfc))
#define f_user_interface(intfc)		(f_interface(intfc)->f_user_intfc)
#define Computational_grid(intfc)					\
				f_user_interface(intfc)._computational_grid
#define computational_grid(intfc)	(&Computational_grid(intfc))
#define	first_node(intfc)		(f_user_interface(intfc)._first_node)
#define	last_node(intfc)		(f_user_interface(intfc)._last_node)
#define size_of_state(intfc)		(f_user_interface(intfc)._sizest)
#define interpolate_intfc_states(intfc)	(f_user_interface(intfc)._interpolate_intfc_states)
#define mono_comp_curves(intfc)		(f_user_interface(intfc)._mono_comp_curves)
#define default_perform_redistribution_function(intfc)			\
    f_user_interface(intfc)._default_perform_redistribution_function

#define	interface_normal_function(intfc)				\
    f_user_interface(intfc)._interface_normal_function
#define interface_normal(intfc)						\
    interface_normal_function(intfc)._normal
#define interface_normal_name(intfc)					\
    interface_normal_function(intfc)._normal_name

#define	interface_tangent_function(intfc)				\
    f_user_interface(intfc)._interface_tangent_function
#define interface_tangent(intfc)					\
    interface_tangent_function(intfc)._tangent
#define interface_tangent_name(intfc)					\
    interface_tangent_function(intfc)._tangent_name

#define	interface_curvature(intfc)				\
    f_user_interface(intfc)._mean_curvature_at_point

#define	FInterfaceTolerances(intfc)					\
	(f_user_interface(intfc)._FInterfaceTolerances)
#define	TIME_STEP_REDUCTION_FACTOR(intfc)				\
	FInterfaceTolerances(intfc)._DtReductionFac

		/* Boundary state control structure macros */
#define	num_bstates(intfc)						\
	(f_user_interface(intfc)._num_bstates)
#define	bstate_list(intfc)						\
	(f_user_interface(intfc)._bstates)
#define	rect_bstate(intfc,i,j)						\
	(bstate_list(intfc)[2*i+j])
#define rect_boundary_state(intfc,i,j)					\
	(rect_bstate(intfc,i,j)->_boundary_state)
#define rect_boundary_state_function(intfc,i,j)				\
	(rect_bstate(intfc,i,j)->_boundary_state_function)
#define rect_boundary_state_data(intfc,i,j)				\
	(rect_bstate(intfc,i,j)->_boundary_state_data)
#define fprint_rect_boundary_state_data(intfc,i,j)			\
	(rect_bstate(intfc,i,j)->_fprint_boundary_state_data)
#define rect_boundary_state_function_name(intfc,i,j)			\
	(rect_bstate(intfc,i,j)->_boundary_state_function_name)
#define read_print_boundary_state_data(intfc)				\
	(f_user_interface(intfc)._read_print_boundary_state_data)

		/* Printing hyper surface boundary type */
#define	print_hsbdry_type(mesg1,hsb_type,mesg2,intfc)		\
		fprint_hsbdry_type(stdout,mesg1,hsb_type,mesg2,intfc)
#define	fprint_node_type(file,mesg1,n_type,mesg2,intfc)			\
		fprint_hsbdry_type(file,mesg1,n_type,mesg2,intfc)
#define	print_node_type(mesg1,n_type,mesg2,intfc)			\
		fprint_node_type(stdout,mesg1,n_type,mesg2,intfc)
#define read_node_type_from_string(type,intfc)				\
		read_hsbdry_type_from_string(type,intfc)

		/* Printing hypersurface wave type */
#define	print_wave_type(mesg1,w_type,mesg2,intfc)			\
		fprint_wave_type(stdout,mesg1,w_type,mesg2,intfc)

#define print_state_data(state,intfc)	fprint_state_data(stdout,state,intfc)

		/* Locstate allocation*/
#define	alloc_state(intfc,sp,sizest)					\
		(f_user_interface(intfc)._alloc_state(sp,sizest))
#define	alloc_intfc_state(intfc,sizest)					\
		(f_user_interface(intfc)._alloc_intfc_state(sizest))
#define	clear_state(intfc,s,sizest)					\
		(f_user_interface(intfc)._clear_state(s,sizest))
#define	obstacle_state(intfc,s,sizest)					\
		(f_user_interface(intfc)._obstacle_state(s,sizest))

enum _REDISTRIBUTION_DIRECTION {
	FORWARD_REDISTRIBUTION,
	BACKWARD_REDISTRIBUTION
};
typedef enum _REDISTRIBUTION_DIRECTION REDISTRIBUTION_DIRECTION;

enum _MOTION_TYPE {
	FREE_MOTION		= 0,
	VERTICAL_MOTION,
	HORIZONTAL_MOTION,
	ROTATION,
	PRESET_MOTION
};
typedef enum _MOTION_TYPE MOTION_TYPE;
	

struct _HS_FLAG {
	boolean _do_not_redistribute;
	boolean _never_redistribute;
	boolean _untracked_hyper_surface;
	boolean _redistribute_by_time_step_frequency;
	boolean _redistribute_hyper_surface;
	boolean _redistributed;
};
typedef struct _HS_FLAG HS_FLAG;

struct _F_HYPER_SURF {
	HYPER_SURF	hyper_surf;
	int		_wave_type;
	HS_FLAG		_hs_flag;
	HYPER_SURF*	_correspond_hyper_surf;
	HYPER_SURF*	_hs_copied_from; /*Used internally in f_copy_interface */
	HYPER_SURF*	_hs_copied_to;   /*These fields are set to NULL at the */
	                                 /*end of f_copy_interface             */
	int		_hs_bstate_index;
	boolean	        (*_perform_redistribution)(HYPER_SURF*,Front*,boolean);
	NORMAL_FUNCTION _hypersurface_normal_function;
	REDISTRIBUTION_DIRECTION _redistribution_direction;
	/* Application related parameters */
	/* for fluid physics */
	double           _surface_tension;
	/* for rigid body mechanics */
	int	body_index;		/* To identify different body */
        double  mass;			/* Total mass */
        double  moment_of_inertial;	/* Moment of inertial about the axis */
        double  center_of_mass[MAXD];	/* Center of mass */
	double	rotation_dir[MAXD];	/* Direction of rotation */
	double	rotation_cen[MAXD];	/* Center of rotation */
        double  cm_velo[MAXD];		/* Center of mass velocity */
        double  angular_velo;		/* Angular velocity of rotation */
	MOTION_TYPE motion_type;
};
typedef struct _F_HYPER_SURF F_HYPER_SURF;

	/* F_HYPER_SURF access macros */
#define	f_hyper_surf(hs)	((F_HYPER_SURF *) Hyper_surf(hs))
#define wave_type(hs)		(f_hyper_surf(hs)->_wave_type)
#define Hs_flag(hs)		(f_hyper_surf(hs)->_hs_flag)
#define correspond_hyper_surf(hs)					\
				(f_hyper_surf(hs)->_correspond_hyper_surf)
#define hs_copied_from(hs)	(f_hyper_surf(hs)->_hs_copied_from)
#define hs_copied_to(hs)	(f_hyper_surf(hs)->_hs_copied_to)
#define surface_tension(hs)     (f_hyper_surf(hs)->_surface_tension)

#define body_index(hs)          (f_hyper_surf(hs)->body_index)
#define total_mass(hs)          (f_hyper_surf(hs)->mass)
#define mom_inertial(hs)        (f_hyper_surf(hs)->moment_of_inertial)
#define center_of_mass(hs)      (f_hyper_surf(hs)->center_of_mass)
#define angular_velo(hs)        (f_hyper_surf(hs)->angular_velo)
#define center_of_mass_velo(hs) (f_hyper_surf(hs)->cm_velo)
#define rotation_direction(hs)  (f_hyper_surf(hs)->rotation_dir)
#define rotation_center(hs)  	(f_hyper_surf(hs)->rotation_cen)
#define motion_type(hs)         (f_hyper_surf(hs)->motion_type)


#define	perform_redistribution_function(hs)				\
    f_hyper_surf(hs)->_perform_redistribution
#define perform_redistribution(hs,fr,force)				\
	(*perform_redistribution_function(hs))(Hyper_surf(hs),fr,force)
#define redistribution_direction(hs)					\
    f_hyper_surf(hs)->_redistribution_direction

#define	hypersurface_normal_function(hs)				\
    f_hyper_surf(hs)->_hypersurface_normal_function
#define hypersurface_normal(hs)						\
    hypersurface_normal_function(hs)._normal
#define hypersurface_normal_name(hs)					\
    hypersurface_normal_function(hs)._normal_name

	/* F_HYPER_SURF Boundary state control */
#define	bstate_index(hs)						\
	(f_hyper_surf(hs)->_hs_bstate_index)
#define	hs_bstate(hs)							\
	(f_user_interface((hs)->interface)._bstates[bstate_index(hs)])
#define boundary_state(hs)						\
	(hs_bstate(hs)->_boundary_state)
#define boundary_state_function(hs)					\
	(hs_bstate(hs)->_boundary_state_function)
#define boundary_state_data(hs)						\
	(hs_bstate(hs)->_boundary_state_data)
#define fprint_boundary_state_data(hs)					\
	(hs_bstate(hs)->_fprint_boundary_state_data)
#define boundary_state_function_name(hs)				\
	(hs_bstate(hs)->_boundary_state_function_name)
#define boundary_state_function_params(hs)				\
	(hs_bstate(hs)->_boundary_state_function_params)


struct _F_HYPER_SURF_BDRY {
	HYPER_SURF_BDRY		hyper_surf_bdry;
	HYPER_SURF_BDRY	  	*_correspond;	/* ONLY REQUIRED IN   */
						/*  copy_interface */
						/* set up prev and next lists */
						/* also used in node          */
						/* propagation loop */
	HYPER_SURF_BDRY	  	*_hsb_copied_from;/*Used internally in        */
	HYPER_SURF_BDRY	  	*_hsb_copied_to;  /*f_copy_interface          */
	                                          /*These fields are set to   */
	                                          /*NULL at the end of        */
	                                          /*f_copy_interface          */
	int _hsbdry_type;
};
typedef struct _F_HYPER_SURF_BDRY F_HYPER_SURF_BDRY;

	/* F_HYPER_SURF_BDRY access macros */
#define	f_hyper_surf_bdry(hsb)	((F_HYPER_SURF_BDRY *) Hyper_surf_bdry(hsb))
#define correspond_hyper_surf_bdry(hsb)				\
		(f_hyper_surf_bdry(Hyper_surf_bdry(hsb))->_correspond)
#define hsb_copied_from(hsb)					\
		(f_hyper_surf_bdry(Hyper_surf_bdry(hsb))->_hsb_copied_from)
#define hsb_copied_to(hsb)					\
		(f_hyper_surf_bdry(Hyper_surf_bdry(hsb))->_hsb_copied_to)
#define hsbdry_type(hsb)					\
			(f_hyper_surf_bdry(Hyper_surf_bdry(hsb))->_hsbdry_type)

		/* macros */

	/* flag for existence of mono component curves */
#define is_mono_comp_curve(curve)					\
	((curve)->interface->dim == 2 ? 				\
	negative_component(curve) == positive_component(curve) : NO)	\

	/* states at (non-node) points on an interface */

#define state_with_comp(point,hs,comp)				\
	(((comp) == negative_component((hs))) ? left_state(point)	\
		: ((comp) == positive_component((hs))) ?		\
			right_state(point) : NULL)


	/* states at nodes */

#define Left_state_at_node(curve,orient)				\
	(((orient) == POSITIVE_ORIENTATION)				\
		? left_start_state(curve) : left_end_state(curve))
#define Right_state_at_node(curve,orient)				\
	(((orient) == POSITIVE_ORIENTATION)				\
		? right_start_state(curve) : right_end_state(curve))

#define Left_state_at_node_of_o_curve(oc)				\
	(((oc)->orient == POSITIVE_ORIENTATION)				\
		? left_start_state((oc)->curve) : left_end_state((oc)->curve))
#define Right_state_at_node_of_o_curve(oc)				\
	(((oc)->orient == POSITIVE_ORIENTATION)				\
		? right_start_state((oc)->curve) : right_end_state((oc)->curve))

#define Left_state_at_opp_node_of_o_curve(oc)				\
	(((oc)->orient == NEGATIVE_ORIENTATION)				\
		? left_start_state((oc)->curve) : left_end_state((oc)->curve))
#define Right_state_at_opp_node_of_o_curve(oc)				\
	(((oc)->orient == NEGATIVE_ORIENTATION)				\
		? right_start_state((oc)->curve) : right_end_state((oc)->curve))


	/*
	*	states at points which might be nodes
	*
	* [left/right]_state_at_point_on_curve, need b to distinguish
	* between the start and end states at the node of a closed curve.
	*/

#define left_state_at_point_on_curve(p,b,c) 				\
	(								\
		(((p) == (c)->start->posn) && ((b) == (c)->first)) ?	\
			left_start_state((c))			 	\
		: (((p) == (c)->end->posn) && ((b) == (c)->last)) ?	\
			left_end_state((c))				\
		:							\
			left_state((p))					\
	)
#define right_state_at_point_on_curve(p,b,c) 				\
	(								\
		(((p) == (c)->start->posn) && ((b) == (c)->first)) ?	\
			right_start_state((c))			 	\
		: (((p) == (c)->end->posn) && ((b) == (c)->last)) ?	\
			right_end_state((c))				\
		:							\
			right_state((p))				\
	)



	/* states obtained by interpolation */
	/* NOTE: the variable "state" must point to storage */

#define left_state_along_bond(t,b,c,state)				\
	bi_interpolate_intfc_states((c)->interface,1.0-(t),(t),		\
		Coords((b)->start),					\
		((b) != (c)->first) ? left_state((b)->start)		\
				    : left_start_state(c),		\
		Coords((b)->end),					\
		((b) != (c)->last ) ? left_state((b)->end)		\
				    : left_end_state(c),state)

#define right_state_along_bond(t,b,c,state)				\
	bi_interpolate_intfc_states((c)->interface,1.0-(t),(t),		\
		Coords((b)->start),					\
		((b) != (c)->first) ? right_state((b)->start)		\
				    : right_start_state(c),		\
		Coords((b)->end),					\
		((b) != (c)->last ) ? right_state((b)->end)		\
				    : right_end_state(c),state)


	/* macros for hypersurface flag */

#define do_not_redistribute(hs)						\
    (Hs_flag(hs)._do_not_redistribute)
#define never_redistribute(hs)						\
    (Hs_flag(hs)._never_redistribute)
#define untracked_hyper_surf(hs)					\
    (Hs_flag(hs)._untracked_hyper_surface)
#define redistribute_by_time_step_frequency(hs)				\
    (Hs_flag(hs)._redistribute_by_time_step_frequency)
#define redistribute_hyper_surface(hs)					\
    (Hs_flag(hs)._redistribute_hyper_surface)
#define redistributed(hs)						\
    (Hs_flag(hs)._redistributed)
#define hs_flags_equal(a,b)						\
    (memcmp((CPOINTER)&Hs_flag(a),(CPOINTER)&Hs_flag(b),sizeof(HS_FLAG)) == 0)

#define omit_redistribution(hs)						\
    (do_not_redistribute(hs) || never_redistribute(hs))

	/* Possible values for the wave_type of a CURVE */

enum {
	UNKNOWN_WAVE_TYPE             = UNKNOWN_BOUNDARY_TYPE,
	ANY_WAVE_TYPE                 = UNKNOWN_BOUNDARY_TYPE,
	PASSIVE_BOUNDARY	      = FIRST_USER_BOUNDARY_TYPE,
	DIRICHLET_BOUNDARY,
	NEUMANN_BOUNDARY,
	MOVABLE_BODY_BOUNDARY,
	GROWING_BODY_BOUNDARY,
	FIRST_PHYSICS_WAVE_TYPE,
	ELASTIC_BOUNDARY,
	FIRST_SCALAR_PHYSICS_WAVE_TYPE = FIRST_PHYSICS_WAVE_TYPE + 10,
	FIRST_VECTOR_PHYSICS_WAVE_TYPE = FIRST_SCALAR_PHYSICS_WAVE_TYPE + 100
};

	/* possible values for the start/end_status of a CURVE */

enum {
        UNKNOWN_CURVE_STATUS 		= -3,
	FIXED				= 0,
	PASSIVE,
        VIRTUAL,
        INCIDENT,
	REFLECTED,
	FIRST_PHYSICS_CURVE_STATUS
};


#define is_passive_boundary(curve) ( wave_type(curve) == PASSIVE_BOUNDARY )

	/* correspondence of curves */
#define correspond_curve(curve)						\
	((correspond_hyper_surf(Hyper_surf(curve))!=NULL) ?		\
		Curve_of_hs(correspond_hyper_surf(Hyper_surf(curve))) : NULL)

	/* correspondence of nodes */
#define correspond_node(node)						\
	((correspond_hyper_surf_bdry(Hyper_surf_bdry(node)) != NULL) ?	\
	    Node_of_hsb(correspond_hyper_surf_bdry(Hyper_surf_bdry(node))) : \
	    NULL)


enum {

	/* Possible values for the hsbdry_type of a HYPER_SURF_BDRY */

	UNKNOWN_HSBDRY_TYPE        = -3,
	PASSIVE_HSBDRY		   =  1,
	FIXED_HSBDRY,
	CLOSED_HSBDRY,
	NEUMANN_HSBDRY,
	DIRICHLET_HSBDRY,
	SUBDOMAIN_HSBDRY,
	SOURCE_HSBDRY,
	SINK_HSBDRY,
	MONO_COMP_HSBDRY,
	FIRST_PHYSICS_HSBDRY_TYPE = 10,

	/* Possible values for the node_type of a NODE */

	UNKNOWN_NODE_TYPE       = UNKNOWN_HSBDRY_TYPE,
	PASSIVE_NODE	        = PASSIVE_HSBDRY,
	FIXED_NODE	        = FIXED_HSBDRY,
	CLOSED_NODE	        = CLOSED_HSBDRY,
	NEUMANN_NODE	        = NEUMANN_HSBDRY,
	DIRICHLET_NODE	        = DIRICHLET_HSBDRY,
	SUBDOMAIN_NODE	        = SUBDOMAIN_HSBDRY,
	SOURCE_NODE	        = SOURCE_HSBDRY,
	SINK_NODE	        = SINK_HSBDRY,
	MONO_COMP_NODE	        = MONO_COMP_HSBDRY,
	FIRST_PHYSICS_NODE_TYPE = FIRST_PHYSICS_HSBDRY_TYPE,
	/* String node type */
	MONO_STRING_NODE,

	/* Possible values for the curve_type of a BOUNDARY CURVE */
	DIRICHLET_CURVE         = DIRICHLET_HSBDRY,
        PASSIVE_CURVE           = PASSIVE_HSBDRY,
        FIXED_CURVE             = FIXED_HSBDRY,
	MONO_COMP_CURVE	        = MONO_COMP_HSBDRY,
	FIRST_PHYSICS_CURVE_TYPE = FIRST_PHYSICS_HSBDRY_TYPE,
	/*#bjet2 */
	NEUMANN_CURVE_P = FIRST_PHYSICS_HSBDRY_TYPE + 20,	/*physical moving curve  */
	NEUMANN_CURVE_W,	/*fixed curve on wall */
	NEUMANN_CURVE
};


#define node_type(node)			hsbdry_type(node)
#define curve_type(node)		hsbdry_type(node)


		/* The following macros are only valid in 2D */

#define is_physical_node(node)						\
		(node_type(node) >= FIRST_PHYSICS_NODE_TYPE)

#define is_source_sink_node(node)					\
		(node_type(node) == SOURCE_NODE || node_type(node) == SINK_NODE)

#define is_closed_node(node)	(node_type(node) == CLOSED_NODE)

#define set_closed_node(node)	(node_type(node) = CLOSED_NODE)

#define is_fixed_node(node)	(node_type(node) == FIXED_NODE)

#define is_passive_node(node)	(node_type(node) == PASSIVE_NODE)

#define is_bdry_like_node(n) \
	( is_bdry(n) || (node_type(n) == NEUMANN_NODE) )

#define is_bdry_like_curve(c) \
	( is_bdry(c) || (wave_type(c) == NEUMANN_BOUNDARY) )

		/* Possible time step control values */

enum {
	ERROR_IN_STEP = 0,
	GOOD_STEP,
	MODIFY_TIME_STEP,
	REPEAT_TIME_STEP
};

enum {
	STATE_ID = FIRST_USER_MESSAGE_ID, /* identifying interior states */
	ST_SIZE,
	TIME_STEP,
	FIRST_PHYSICS_MESSAGE_ID = FIRST_USER_MESSAGE_ID+100
};

#if defined(c_plusplus) || defined(__cplusplus)
}
#endif


#endif /* !defined(_FUSERINT_H) */
