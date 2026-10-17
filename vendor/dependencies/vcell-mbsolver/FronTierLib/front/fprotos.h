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
*				fprotos.h
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*/
#if !defined(_FPROTOS_H)
#define _FPROTOS_H

#include <front/fdecs.h>

		/* Front IMPORTED Function Prototypes*/

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif

	/* fadv.c*/
IMPORT	void	set_advance_front(INIT_DATA*,Front*);
IMPORT  INTERFACE  *pp_copy_interface(INTERFACE*);
IMPORT  void    set_propagation_limits(Front*,Front*);
IMPORT  void    init_intfc_curvature3d(Front*,INTERFACE*);
IMPORT  int     return_advance_front(Front*,Front**,int,const char*);

IMPORT  void    copy_hypersurface_flags(INTERFACE*);

IMPORT  void    reset_hs_flags_on_intfc(INTERFACE*);
IMPORT  void    set_node_doubly_linked_list(INTERFACE*);
IMPORT  void    set_corresponds_for_node_prop(INTERFACE*,INTERFACE*);
IMPORT  void    print_linked_node_list(INTERFACE*);
IMPORT  NODE    *adv_node_loop_after_good_prop(NODE*,NODE*,RPROBLEM**);
IMPORT  NODE    *reorder_node_loop(NODE*,NODE*);
IMPORT  boolean    consistent_propagated_loop_orientations(double,double*,
                     Front*,POINTER);
IMPORT  int     rp_modify_time_step(RPROBLEM*,Front*,int);
IMPORT  double   limit_dt_frac(double,Front*);

	/* fbdry1.c*/
IMPORT	int	f_boundary_untangle(Front*,CROSS**,RPROBLEM*,NODE*,int);
IMPORT	int	is_bdry_interior_cross(Front*,CROSS*,ORIENTATION*,
				       ANGLE_DIRECTION*,
				       COMPONENT*,COMPONENT*);
IMPORT	int	is_c_in_another_bdry_cr(CROSS*,CURVE*);
IMPORT	int	next_boundary(CURVE*,ORIENTATION,CURVE**,ORIENTATION*);
IMPORT	int	split_curves_at_bdry_cross(CROSS*,Front*,ORIENTATION,
					   ANGLE_DIRECTION,CURVE**,
					   CURVE**,CURVE**,COMPONENT*,
					   COMPONENT*,int,RPROBLEM*);
IMPORT	void	classify_bdry_crosses(CROSS*,int*);
IMPORT	void	f_impose_bc(FT_POINT*,BOND*,CURVE*,double*,Front*,boolean,boolean);
IMPORT	void	map_phys_cur_states_to_bdry_cur(CURVE*,ORIENTATION,SIDE,
						CURVE*,ORIENTATION,SIDE,
						int,int,INTERFACE*,Front*);
IMPORT	int	modify_exterior_curve(CURVE*,CURVE*,CURVE**,ORIENTATION,
				      ANGLE_DIRECTION,NODE*,
				      RPROBLEM*,CROSS*,int,Front*);
IMPORT  void    debug_show_boundary_curve_states(const char*,
                                       CURVE*,ORIENTATION);

	/* fbdry2.c*/
IMPORT	SIDE	physical_side_of_bdry_curve(CURVE*);
IMPORT	int	is_curve_in_cross_list(CROSS*,CURVE*);

	/* fbdry3.c*/
IMPORT	boolean	correct_for_exterior_curves(Front*);
IMPORT	int	curve_exits_parallel_to_bdry(Front*,POINTER,RPROBLEM*);
IMPORT	void	replace_cphys_by_cbdry(CURVE*,CURVE*,Front*);

	/* fbdry4.c*/
IMPORT	boolean	all_pts_of_c_are_exterior_like(CURVE*,INTERFACE*);
IMPORT	boolean	all_pts_on_c_are_ext_to_rect(CURVE*,RECT_GRID*);
IMPORT	boolean	f_delete_phys_remn_on_bdry(Front*);
IMPORT	boolean	c_parallel_to_bdry_like_curves(CURVE*,double,double);
IMPORT	boolean	shortest_connecting_bdry_like_path(CURVE*,ORIENTATION,
						   CURVE**,ORIENTATION*,
						   boolean*,boolean*,double*,
						   boolean*,boolean*,double*);
IMPORT	void	f_delete_exterior_curves(Front*,INTERFACE*);
IMPORT	void	shift_c_states_to_bdry_curves(CURVE*,CURVE*,Front*);

	/* fcorrspnd.c*/
IMPORT	HYPER_SURF	*find_correspond_hyper_surface(HYPER_SURF*,
				       HYPER_SURF_BDRY**,HYPER_SURF_BDRY**,
				       Front*,INTERFACE*);
IMPORT	boolean	rst_cor_after_delete_hyper_surf(HYPER_SURF*);
IMPORT	boolean	rst_cor_after_make_hyper_surf(HYPER_SURF*);
IMPORT	boolean	set_correspondence_between_interfaces(INTERFACE*,INTERFACE*);
IMPORT	void	show_crspd_between_two_intfc(INTERFACE*,INTERFACE*);
IMPORT	void	print_correspond_hyper_surf_list(INTERFACE*);
IMPORT	void	remove_corresponds_to_deleted_interface(INTERFACE*);
IMPORT	void	rst_cor_after_delete_interface(INTERFACE*);
IMPORT	void	set_add_to_correspond_list(boolean);
IMPORT	void	set_correspond_hyper_surf_bdrys_to_NULL(INTERFACE*);
IMPORT	void	set_correspond_hyper_surfaces_to_NULL(INTERFACE*);
IMPORT	void	zero_corr_of_hyper_surf(HYPER_SURF*);
IMPORT	int	rst_cor_after_join_hypersurfaces(HYPER_SURF*,HYPER_SURF*,
						 HYPER_SURF*);
IMPORT	CURVE	*find_correspond_curve(CURVE*,NODE*,NODE*,Front*,INTERFACE*);
IMPORT	NODE	*node_corresponding_to(NODE*,Front*);
IMPORT	boolean	find_correspond_of_oriented_curve(O_CURVE*,O_CURVE*,NODE*,
						  Front*,INTERFACE*);
IMPORT	boolean	rst_cor_after_attach_curve_to_node(CURVE*,CURVE*);
IMPORT	boolean	rst_cor_after_invert_curve(CURVE*);
IMPORT	boolean	rst_cor_after_split_curve(CURVE*,CURVE**);

	/* fcrosscur.c*/
IMPORT	boolean	intersection_of_two_o_curves(O_CURVE*,O_CURVE*,O_CURVE*,
					     O_CURVE*,BOND**,BOND**,FT_POINT**,
					     double*,double*,Front*,POINTER,
					     double,NODE_FLAG);
IMPORT	int	check_cross(double,BOND*,O_CURVE*,double,BOND*,O_CURVE*,FT_POINT*,
			    double*,double*,int);
IMPORT	int	crossing_of_a_propagated_curve_and_circle(O_CURVE*,O_CURVE*,
				double,FT_POINT*,FT_POINT*,BOND**,double*,Front*,
				POINTER,RPROBLEM**,double,double*,NODE_FLAG);
IMPORT	int	crossing_of_two_propagated_curves(O_CURVE*,O_CURVE*,O_CURVE*,
				O_CURVE*,FT_POINT*,BOND**,BOND**,double*,double*,
				Front*,POINTER,RPROBLEM**,double,double*,
				NODE_FLAG);
IMPORT	int	set_node_velocity(FT_POINT*,FT_POINT*,NODE*,O_CURVE*,O_CURVE*,
				  double*,double*,Front*,double,double*);
IMPORT	void	reverse_states_at_point(FT_POINT*,Front*);
IMPORT	void	init_curve_for_crossing(FT_POINT*,FT_POINT*,BOND*,O_CURVE*,
					O_CURVE*,NODE**,BOND**,Front*,
					POINTER,double,double*,NODE_FLAG);
IMPORT	void	set_vel_of_crossing_node(BOND*,BOND*,BOND*,BOND*,int,int,
					 NODE*,NODE*,double,Front*);
IMPORT	void	set_virtual_bond_at_node(FT_POINT*,BOND*,CURVE*,ORIENTATION,Front*,
					 int,NODE_FLAG);

	/* fcrossext.c*/
IMPORT	int	cross_or_extend_to_cross_two_propagated_curves(O_CURVE*,
				O_CURVE*,O_CURVE*,O_CURVE*,FT_POINT**,BOND**,
				BOND**,double*,double*,Front*,POINTER,
				RPROBLEM**,double,double*,NODE_FLAG,boolean*);
IMPORT	int	D_extend_crossing_of_two_propagated_curves(O_CURVE*,O_CURVE*,
				O_CURVE*,O_CURVE*,O_CURVE*,O_CURVE*,COMPONENT,
				COMPONENT,FT_POINT*,BOND**,BOND**,double*,double*,
				Front*,POINTER,RPROBLEM**,double,double*,
				NODE_FLAG);
IMPORT	int	H_extend_crossing_of_two_propagated_curves(O_CURVE*,O_CURVE*,
				O_CURVE*,O_CURVE*,COMPONENT,COMPONENT,FT_POINT*,
				BOND**,BOND**,double*,double*,Front*,POINTER,
				RPROBLEM**,double,double*,NODE_FLAG);
IMPORT	void	find_bonds_for_extension_direction(BOND*,O_CURVE*,O_CURVE*,
						   BOND*,BOND*,Front*);
IMPORT	void	set_use_circle_D_extend(boolean);
IMPORT	void	set_use_normal_D_extend(boolean);

	/* fcrstatus.c*/
IMPORT	int	find_D_extend_status(O_CURVE*,O_CURVE*,FT_POINT*,BOND*,BOND*,
				     FT_POINT*,Front*,POINTER,double,double*);
IMPORT	int	find_H_extend_status(O_CURVE*,O_CURVE*,FT_POINT*,O_CURVE*,
				     O_CURVE*,double*,int,BOND*,FT_POINT*,Front*,
				     POINTER,double,double*);
IMPORT	int	find_circle_cross_status(O_CURVE*,O_CURVE*,FT_POINT*,double,
					 FT_POINT*,Front*,double*);
IMPORT	int	find_cross_or_extend_to_cross_status(int,O_CURVE*,O_CURVE*,
				O_CURVE*,O_CURVE*,FT_POINT*,FT_POINT*,BOND*,BOND*,
				BOND*,BOND*,FT_POINT*,NODE**,NODE**,Front*,
				POINTER,double,double*);
IMPORT	int	find_cross_status(int,O_CURVE*,O_CURVE*,O_CURVE*,O_CURVE*,
				  FT_POINT*,FT_POINT*,BOND*,BOND*,FT_POINT*,NODE**,
				  NODE**,Front*,POINTER,double,double*);
IMPORT	int	robust_circle_cross_trace(FT_POINT*,FT_POINT*,FT_POINT*,FT_POINT*,
					  double,double*,FT_POINT*);
IMPORT	int	robust_cross_trace(RECT_GRID*,FT_POINT*,FT_POINT*,BOND*,BOND*,
				   double*,FT_POINT*);
IMPORT	void	set_prop_status_for_pseudo_cross_node(O_CURVE*,O_CURVE*,
				                      O_CURVE*,O_CURVE*,
						      Front*,POINTER,double,
						      NODE_FLAG);

	/* fgb2d.c*/
IMPORT  INTERFACE *make_emb_grid_intfc(INTERFACE*);

	/* fgb3d.c*/
IMPORT	boolean repair_intfc_at_crossings3d(Front*);
IMPORT	boolean rebuild_intfc_at_crossings3d(Front*);
IMPORT	boolean rebuild_intfc_at_crossings3d2(Front*);
IMPORT  boolean check_extension_of_surface_global(SURFACE*);
IMPORT  boolean check_normal_on_intfc(INTERFACE*);
IMPORT	boolean check_degenerated_loop(TRI**,int*,FT_POINT**,int);

	/*fgb3dutil.c */
IMPORT	void	adjust_crossings(int*,int*,INTERFACE*);
IMPORT	boolean	next_ip_in_dir(const int*,int,int*,int*,int*);
IMPORT	boolean	check_and_repair_crx(INTERFACE*,int*,int*);
IMPORT	void 	fill_comps_in_box(int*,int*,int*,INTERFACE*);
IMPORT	void 	fill_physical_comps(int*,int*,int*,INTERFACE*);
IMPORT	void 	fill_comp_with_component3d(int*,int*,int*,INTERFACE*);
IMPORT	void 	show_component_along_line(int,int,int*,int*,int,INTERFACE*);
IMPORT	void 	remove_unphysical_crxings(int*,int*,int*,INTERFACE*,CRX_TYPE);
IMPORT	boolean	remove_unphysical_crossings3d(INTERFACE*,int*,int*);
IMPORT  SURFACE* find_surf_with_comp(INTERFACE*, int, int);
IMPORT	boolean	curves_on_bdry_side(int,int,INTERFACE*);

IMPORT	boolean 	adjacent_cell(int*,int*);
IMPORT	int 	record_unphysical_ips(int*,int*,INTERFACE*,int**);
IMPORT  boolean	reconstruct_intfc3d_in_box_lgb(INTERFACE*,int*,int*,boolean,VOLUME_FRAC*);
IMPORT  boolean	reconstruct_intfc3d_in_box(INTERFACE*,int*,int*,boolean,VOLUME_FRAC*);
IMPORT	int	count_grid_intfc_crossings3d(INTERFACE*);
IMPORT 	int	insert_grid_intfc_crossings3d(INTERFACE*);
IMPORT	void	set_expanded_grid(RECT_GRID*,RECT_GRID*);
IMPORT  void	set_crx_storage_for_reconstruction(INTERFACE*,VOLUME_FRAC*);
IMPORT	void	free_crx_storage(INTERFACE*);
IMPORT	void	linear_interp_coefs_three_pts(double*,double*,double*,double*,double*);
IMPORT	boolean	track_comp_through_crxings3d(int*,int*,int*,INTERFACE*,CRX_TYPE);
IMPORT	void    interpolate_crx_pt_states_on_tri(INTERFACE*,FT_POINT*,TRI*,SURFACE*);
IMPORT	void    interpolate_crx_pt_states_on_edge(INTERFACE*,FT_POINT*,TRI*,
                                SURFACE*,int);
IMPORT  void    check_surface_curve(SURFACE *);
IMPORT  void    check_intfc_curve_connect(INTERFACE *);

	/*fgb3comp.c */
IMPORT  boolean  is_wall_side(TRI*,int);
IMPORT  TRI*  Tri_on_side_along_wall(int*,TRI*,int);
IMPORT  boolean  is_wall_vertex(TRI*,int);
IMPORT  int   tri_list_along_wall(FT_POINT*,TRI*,TRI***,INTERFACE*);
IMPORT	int   check_wall_crx_orient(INTERFACE *, int *, int *);
IMPORT  boolean  edge_index_of_face(int *,int,int,int,GRID_DIRECTION,int*);
IMPORT  boolean  vertex_index_of_face(int *,int,int,int,GRID_DIRECTION,int*);
IMPORT	boolean  rebuild_intfc_at_crossings3d3(Front *);

        /* fgrid.c*/
IMPORT	int  	count_grid_intfc_crossings(INTERFACE*);
IMPORT 	int 	insert_grid_intfc_crossings(INTERFACE*);
IMPORT  int     set_grid_intfc_components(INTERFACE*,INTERFACE*);
IMPORT  INTERFACE *make_grid_intfc(INTERFACE*,GRID_TYPE,VOLUME_FRAC*);
IMPORT	void	free_grid_intfc(INTERFACE*);
IMPORT	void	show_grid_components(int*,int*,int,INTERFACE*);
IMPORT	void	show_line_components3d(int*, int*, int, INTERFACE*);
IMPORT	void 	show_the_grid_comp(const char*,INTERFACE*);
IMPORT	void	adjust_grid_intfc_points(INTERFACE*);
	
	/* finit.c*/
IMPORT	void	f_read_print_front_options(INIT_DATA*,Front*);
IMPORT	boolean	f_read_print_max_front_speed_info(INIT_DATA*,const IO_TYPE*,
                                                  Front*,MAX_FRONT_SPEED*);
IMPORT	void	f_copy_redistribution_values(INIT_DATA*,Front*);
IMPORT	void	f_init_redistribute(INIT_DATA*,Front*);
IMPORT	void	f_prompt_for_front_options(INIT_DATA*,Front*);
IMPORT	void	f_prompt_for_redistribute(INIT_DATA*);
IMPORT	void	f_read_print_front(INIT_DATA*,Front*);
IMPORT	void	f_set_redistribution_defaults(INIT_DATA*);
IMPORT	void	init_front(INIT_DATA*,Front*);
IMPORT  void    initial_front_redistribute(Front*,const IO_TYPE*);
IMPORT	void	init_front_states(Front*,INIT_DATA*,
				  void (*)(FT_POINT*,HYPER_SURF_ELEMENT*,
				           HYPER_SURF*,Locstate,Locstate,
					   INIT_DATA*));
IMPORT	void	set_dflt_cur_redist_params(Front*);
IMPORT	void	set_front_hooks(INIT_DATA*);
IMPORT 	void 	set_default_front_options(INIT_DATA*, Front*);
IMPORT  void    set_test_front(F_INIT_DATA*,Front*);
IMPORT  void    set_default_front(F_INIT_DATA*,Front*);

	/* fint.c*/
IMPORT	HYPER_SURF_BDRY	*f_make_hypersurface_boundary(void);
IMPORT	HYPER_SURF      *f_make_hypersurface(COMPONENT,COMPONENT);
IMPORT	INTERFACE       *f_copy_interface(INTERFACE*);
IMPORT	INTERFACE       *f_receive_interface(int);
IMPORT	FT_POINT	*f_Point(double*);
IMPORT	FT_POINT	*f_Static_point(INTERFACE*);
IMPORT	FT_POINT	*f_copy_point(FT_POINT*);
IMPORT	boolean	copy_intfc_states(void);
IMPORT	boolean f_merge_hs_flags(HYPER_SURF*,HYPER_SURF*);
IMPORT	int	add_bstate_to_list(BOUNDARY_STATE*,INTERFACE*,int);
IMPORT	int	f_delete_interface(INTERFACE*);
IMPORT	boolean	f_nearest_intfc_state(double*,COMPONENT,INTERFACE*,
				      Locstate,double*,HYPER_SURF**);
IMPORT	const char *f_wave_type_as_string(int);
IMPORT	int	f_read_hsbdry_type_from_string(const char*,INTERFACE*);
IMPORT	int	f_read_wave_type_from_string(const char*);
IMPORT	int	f_user_read_print_interface(INIT_DATA*,const IO_TYPE*,
                                            INTERFACE*,boolean);
IMPORT	void	f_fprint_boundary_state_data(FILE*,INTERFACE*,BOUNDARY_STATE*);
IMPORT	void	f_fprint_hsbdry_type(FILE*,const char*,int,
				     const char*,INTERFACE*);
IMPORT	void	f_fprint_wave_type(FILE*,const char*,int,const char*,
                                   INTERFACE*);
IMPORT	void	f_read_print_boundary_state_data(INIT_DATA*,const IO_TYPE*,
                                                 INTERFACE*,int);
IMPORT	void	f_reconstruct_interface_pointers(INTERFACE*,struct Table *,
						 POINTER*,POINTER*);
IMPORT	void	f_reconstruct_point_pointers(FT_POINT*,INTERFACE*,INTERFACE*,
					     POINTER*,POINTER*,int);
IMPORT	void	f_user_copy_hyper_surf(HYPER_SURF*,HYPER_SURF*);
IMPORT	void	f_user_fprint_interface(FILE*,INTERFACE*);
IMPORT	void	f_user_fprint_intfc_rect_grids(FILE*,INTERFACE*);
IMPORT	void	f_user_make_interface(INTERFACE*);
IMPORT	void	f_user_read_print_intfc_rect_grids(const IO_TYPE*,INTERFACE*,
						   boolean,REMAP*);
IMPORT	void	fixed_boundary_state(double*,HYPER_SURF*,Front*,
				     POINTER,Locstate);
IMPORT	void	set_computational_grid(INTERFACE*,RECT_GRID*);
IMPORT	void	set_copy_intfc_states(boolean);
IMPORT	void	set_size_of_intfc_state(size_t);
/*#bjet2 */
IMPORT	int	get_size_of_intfc_state(void);
IMPORT  void    set_use_wall_edge(boolean);
IMPORT  boolean    use_wall_edge(void);
IMPORT	CURVE	*f_copy_curve(CURVE*,NODE*,NODE*);
IMPORT	CURVE	*f_make_curve(COMPONENT,COMPONENT,NODE*,NODE*);
IMPORT	NODE	*f_copy_node(NODE*);
IMPORT	NODE	*f_make_node(FT_POINT*);
IMPORT	boolean	interpolate_states_at_split_curve_node(void);
IMPORT	boolean	f_delete_end_of_bond(BOND*,CURVE*);
IMPORT	boolean	f_delete_node(NODE*);
IMPORT	boolean	f_delete_start_of_bond(BOND*,CURVE*);
IMPORT	boolean	f_insert_point_in_bond(FT_POINT*,BOND*,CURVE*);
IMPORT	boolean	f_user_join_curves(CURVE*,CURVE*,CURVE*);
IMPORT	boolean	f_user_read_print_curve(CURVE*,const IO_TYPE*,boolean);
IMPORT	boolean	f_user_split_curve(int,FT_POINT*,BOND*,CURVE*,CURVE**);
IMPORT	int	f_delete_curve(CURVE*);
IMPORT	int	f_user_read_node(NODE*);
IMPORT	void	f_reconstruct_bond_pointers(BOND*,INTERFACE*,INTERFACE*,
					    POINTER*,POINTER*,int);
IMPORT	void	f_reconstruct_curve_pointers(CURVE*,INTERFACE*,INTERFACE*,
					     POINTER*,POINTER*,int);
IMPORT	void	f_reconstruct_node_pointers(NODE*,INTERFACE*,INTERFACE*,
					    POINTER*,POINTER*,int);
IMPORT	void	f_user_fprint_curve(FILE*,CURVE*);
IMPORT	void	f_user_fprint_node(FILE*,NODE*);
IMPORT	void	f_user_read_curve(CURVE*);
IMPORT	void	f_user_read_print_node(NODE*,const IO_TYPE*,boolean);
IMPORT	void	set_interpolate_states_at_split_curve_node(boolean);
/*#bjet2 */
IMPORT SURFACE *f_detach_one_surface(SURFACE *);
IMPORT	FT_POINT	*f_make_point(double*,COMPONENT,COMPONENT);
IMPORT	void	test_for_mono_comp_curves(INTERFACE*);
IMPORT	BOND_TRI *f_link_tri_to_bond(BOND_TRI*,TRI*,SURFACE*,BOND*,CURVE*);
IMPORT	C_BOND	*f_CBond(C_BOND*,FT_POINT*,FT_POINT*,TRI*,TRI*);
IMPORT	SURFACE	*f_join_surfaces(CURVE*);
IMPORT	SURFACE	*f_make_surface(COMPONENT,COMPONENT,CURVE**,CURVE**);
IMPORT	SURFACE	*f_copy_surface(SURFACE*,CURVE**,CURVE**,boolean);
IMPORT	boolean	f_insert_point_in_tri(FT_POINT*,TRI*,SURFACE*);
IMPORT	boolean	f_insert_point_in_tri_side(FT_POINT*,int,TRI*,SURFACE*);
IMPORT	int	f_delete_surface(SURFACE*);
IMPORT	void 	f_reverse_bond(BOND*);
IMPORT	void 	f_reorder_curve_link_list(CURVE*);
IMPORT	void	f_assign_curve_boundary_flag(CURVE*);
IMPORT	void	f_assign_curve_boundary_type(CURVE*,int,int*);
IMPORT	void	f_user_fprint_surface(FILE*,SURFACE*);
IMPORT	void	f_user_install_faces(SURFACE*,int);
IMPORT	void	f_user_read_print_surface(SURFACE*,const IO_TYPE*,boolean);
IMPORT	void	f_user_read_surface(SURFACE*);

	/* fnode.c*/
IMPORT	int	f_node_propagate(Front*,POINTER,NODE*,NODE*,RPROBLEM**,
				 double,double*,NODE_FLAG,POINTER);
IMPORT	int	B_node_propagate(Front*,POINTER,NODE*,NODE*,RPROBLEM**,
				 double,double*,NODE_FLAG);
IMPORT	int	closed_node_propagate(Front*,POINTER,NODE*,NODE*,double);
IMPORT	int	fixed_node_propagate(Front*,POINTER,NODE*,NODE*,double);
IMPORT	int	pp_node_propagate(Front*,POINTER,NODE*,NODE*,RPROBLEM**,
				  double,double*);
IMPORT  int	set_node_states_and_continue(NODE*,NODE*,Front*);
IMPORT	void	reset_fixed_node_states(NODE*,Front*);
IMPORT	void	assign_states_on_passive_curves_at_node(NODE*);

	/* fnodesub.c*/
IMPORT	void	find_tangent_to_curve(FT_POINT*,BOND*,CURVE*,ORIENTATION,
				      double*,Front*);
IMPORT	void	bond_secant_to_curve(FT_POINT*,BOND*,CURVE*,ORIENTATION,BOND*,
				     Front*,double);
IMPORT	void	find_secant_to_curve(FT_POINT*,BOND*,CURVE*,ORIENTATION,double*,
				     Front*,double);
IMPORT	CURVE	*find_physical_curve_at_node(NODE*,ORIENTATION*);
IMPORT	SIDE	find_propagation_side(O_CURVE*,FT_POINT*,SIDE,Front*);
IMPORT	boolean	delete_redundant_node(NODE*,CROSS*,RPROBLEM*,Front*);
IMPORT	boolean	f_check_delete_redundant_node(NODE*,CURVE*,CURVE*);
IMPORT	int	bdry_node_type(int);
IMPORT	ANGLE_DIRECTION	f_find_i_to_prop_dir(Front*,POINTER,NODE*,CURVE*,
					     ORIENTATION,double,
					     COMPONENT*,FT_POINT*,double*);
IMPORT	int	modify_B_node(NODE*,NODE*,O_CURVE*,O_CURVE*,O_CURVE*,O_CURVE*,
			      O_CURVE*,O_CURVE*,O_CURVE*,O_CURVE*,O_CURVE*,
			      O_CURVE*,FT_POINT*,BOND*,BOND*,ANGLE_DIRECTION,
			      double,double,RPROBLEM**,Front*,POINTER,
			      double,double*,NODE_FLAG);
IMPORT	int	velocity_satisfies_CFL(NODE*,double,double*,Front*);
IMPORT	void	assign_interacting_states(FT_POINT*,CURVE*,ORIENTATION,Front*,
					  Locstate,Locstate);
IMPORT	void	cut_curve(FT_POINT*,BOND*,CURVE*,ORIENTATION,
			  Front*,Locstate,Locstate);
IMPORT	void	find_propagation_orientation(Front*,POINTER,NODE*,NODE*,FT_POINT*,
					     O_CURVE*,double,ANGLE_DIRECTION*,
					     O_CURVE*,O_CURVE*,O_CURVE*,
					     O_CURVE*,SIDE*,SIDE*,
					     COMPONENT*,COMPONENT*);
IMPORT	void	find_tangent_to_propagated_curve(FT_POINT*,BOND*,O_CURVE*,
						 O_CURVE*,double*,Front*,
						 POINTER,double);
IMPORT	void	init_redundant_node_for_deletion(NODE*,NODE*,Front*,
						 POINTER,double);
IMPORT	void	insert_point_adjacent_to_node(FT_POINT*,CURVE*,ORIENTATION);
IMPORT	void	propagated_tangent_bond_at_node(BOND*,CURVE*,ORIENTATION,Front*,
						POINTER,double);
IMPORT	void	shift_node(FT_POINT*,BOND*,CURVE*,ORIENTATION,
			   CURVE*,ORIENTATION,NODE*,Front*,
			   Locstate,Locstate,Locstate,Locstate);
IMPORT	void	shift_node_past(FT_POINT*,BOND*,CURVE*,ORIENTATION,
				CURVE*,ORIENTATION,ANGLE_DIRECTION,NODE*,
				Front*,NODE_FLAG,Locstate,Locstate,
				Locstate,Locstate);

	/* fdiagnostic.c */
IMPORT  void 	detail_of_curve(CURVE*);
IMPORT	void	summary_of_interface(INTERFACE*);
IMPORT  void	summarize_interface(const char*,const char*,INTERFACE*,
				    const COORDINATE_PLANE,
				    const char*,const char*);

	/* fprint.c*/
IMPORT	const char	*time_step_status_as_string(int);
IMPORT	void	f_fprint_front(Front*,FILE*);
IMPORT	void	f_print_Front_structure(Front*);
IMPORT	void	f_fprint_FlowSpecifiedRegion_data(FILE*,FlowSpecifiedRegion*,
						  Front*);
IMPORT	void	fprint_Tan_stencil(FILE*,Front*,Tan_stencil*);
IMPORT	void	fprint_Nor_stencil(FILE*,Front*,Nor_stencil*);
IMPORT	void	print_AFLIN(FILE*,AFLIN*,int);
IMPORT	void	print_Tan_stencil(Front*,Tan_stencil*);
IMPORT	void	print_Nor_stencil(Front*,Nor_stencil*);
IMPORT  void    print_time_step_status(const char*,int,const char*);
IMPORT	void	debug_front(const char*,const char*,Front*);
IMPORT	void	f_fprint_max_front_speed_info(FILE*,Front*);
IMPORT  void    print_front_output(Front*,char*);
IMPORT	void	show_front_output(Front*,char*,boolean);
#if defined(USE_HDF)
IMPORT	void	plot_hdf_data(POINTER,Front*,HDF_plot_data*);
#endif /* defined(USE_HDF) */
IMPORT	const char *propagation_status_as_string(NODE_PROPAGATION_STATUS);
IMPORT	const char *redistribution_direction_as_string(REDISTRIBUTION_DIRECTION);
IMPORT	const char *untangle_status_as_string(int);
IMPORT	void	f_print_rp_node(RP_NODE*,RPROBLEM*);
IMPORT	void	f_print_rproblem(RPROBLEM*);
IMPORT	void	fprint_redistribution_direction(FILE*,const char*,
						REDISTRIBUTION_DIRECTION,
						const char*);
IMPORT	void	fshow_curve_states(FILE*,CURVE*);
IMPORT	void	print_bond_and_states(BOND*,CURVE*,Front*);
IMPORT	void	print_propagation_status(NODE*);
IMPORT  void    print_untangle_status(int);
IMPORT	void	show_curve_states(CURVE*);
IMPORT  void    f_gview_plot_interface(const char*,INTERFACE*);
IMPORT	void	fshow_surface_states(FILE*,SURFACE*);
IMPORT	void	print_tri_states(TRI*,HYPER_SURF*);
IMPORT	void	show_surface_states(SURFACE*);
IMPORT	void	print_node_status(const char*,int,const char*);
IMPORT	const char *node_status_as_string(int);

	/* fprop2d.c*/
IMPORT	void	f_curve_propagate2d(Front*,POINTER,CURVE*,CURVE*,double);
IMPORT	void	f_tan_curve_propagate(Front*,Front*,INTERFACE*,CURVE*,
				CURVE*,double);
IMPORT	void	oblique_propagate_at_node(Front*,POINTER,FT_POINT*,O_CURVE*,
			 	O_CURVE*,double*,double);
IMPORT	void	set_no_tan_propagate(CURVE *);
IMPORT	void	f_second_order_intfc_propagate(Front*,POINTER,INTERFACE*,
				INTERFACE*,double);

	/* fprop3d.c*/
IMPORT  void    fill_stencil_in_direction(Front*,Tan_stencil*,int,int,
                                const Tparams*);
IMPORT	void	set_weight_for_tri_interpolation(double*,TRI*,double*,double*,
	                        INTERFACE*);
IMPORT  void    f_surface_propagate(Front*,Front*,POINTER,double,double*);
IMPORT  void    fourth_order_point_propagate(Front*,POINTER, FT_POINT*, FT_POINT*,
		                HYPER_SURF_ELEMENT*,HYPER_SURF*,double,double*);
IMPORT  void    second_order_point_propagate(Front*,POINTER, FT_POINT*, FT_POINT*,
		                HYPER_SURF_ELEMENT*,HYPER_SURF*,double,double*);
IMPORT  void    first_order_point_propagate(Front*,POINTER, FT_POINT*, FT_POINT*,
		                HYPER_SURF_ELEMENT*,HYPER_SURF*,double,double*);
IMPORT	void    find_position_along_wall(double*,TN*,double,const Tparams*,
				Front*);
IMPORT  void    print_TN(TN*);
IMPORT  void    print_Tparams(const char*, Tparams*);
IMPORT	boolean	f_tan_point_propagate(Front*,FT_POINT*,FT_POINT*,
				HYPER_SURF_ELEMENT*,HYPER_SURF*,double,int);
IMPORT  boolean set_up_tangent_params(Front*,FT_POINT*,HYPER_SURF_ELEMENT*,
                                HYPER_SURF*,Tparams*);
IMPORT	boolean set_up_wall_tangent_params(Tparams*,FT_POINT*,BOND*,CURVE*,double*,
				SURFACE**,TRI**,INTERFACE*,Front*);
IMPORT	boolean fill_tan_stencil_along_wall(Tan_stencil*,Front*,const Tparams*,
				TRI**,int);
IMPORT  boolean set_up_tangent_stencil(Front*,Tan_stencil*,const Tparams*,
                                FT_POINT*,double);

	/* fvelo.c*/
/*	Initialization of velocity field params */
IMPORT 	void 	init_translation_params(POINTER*,int);
IMPORT 	void 	init_radial_motion_params(POINTER*,int);
IMPORT 	void 	init_shear_motion_params(POINTER*,int);
IMPORT 	void 	init_sine_motion_params(POINTER*,int);
IMPORT 	void 	init_circular_rotation_params(POINTER*,int);
IMPORT 	void 	init_norv_params(POINTER*,int);
IMPORT 	void 	init_curvature_params(POINTER*,int);
IMPORT 	void 	init_flame_params(POINTER*,int);
IMPORT 	void 	init_burgers_params(POINTER*,int);
IMPORT  void    init_bipolar_params(POINTER*,int);
IMPORT  void    init_vortex_params(POINTER*,int);
/*	Functions of velocity field */
IMPORT 	int 	translation_vel(POINTER,Front*,FT_POINT*,HYPER_SURF_ELEMENT*,
			HYPER_SURF*,double*);
IMPORT 	int 	radial_motion_vel(POINTER,Front*,FT_POINT*,HYPER_SURF_ELEMENT*,
			HYPER_SURF*,double*);
IMPORT 	int 	shear_motion_vel(POINTER,Front*,FT_POINT*,HYPER_SURF_ELEMENT*,
                        HYPER_SURF*,double*);
IMPORT 	int 	sine_motion_vel(POINTER,Front*,FT_POINT*,HYPER_SURF_ELEMENT*,
			HYPER_SURF*,double*);
IMPORT 	int 	circular_rotation_vel(POINTER,Front*,FT_POINT*,HYPER_SURF_ELEMENT*,
			HYPER_SURF*,double*);
IMPORT 	int 	normal_vel(POINTER,Front*,FT_POINT*,HYPER_SURF_ELEMENT*,
			HYPER_SURF*,double*);
IMPORT 	int 	curvature_vel(POINTER,Front*,FT_POINT*,HYPER_SURF_ELEMENT*,
			HYPER_SURF*,double*);
IMPORT 	int 	flame_vel(POINTER,Front*,FT_POINT*,HYPER_SURF_ELEMENT*,
			HYPER_SURF*,double*);
IMPORT 	int 	burgers_vel(POINTER,Front*,FT_POINT*,HYPER_SURF_ELEMENT*,
			HYPER_SURF*,double*);
IMPORT  int     bipolar_vel(POINTER,Front*,FT_POINT*,HYPER_SURF_ELEMENT*,
                        HYPER_SURF*,double*);
IMPORT  int     vortex_vel(POINTER,Front*,FT_POINT*,HYPER_SURF_ELEMENT*,
		                        HYPER_SURF*,double*);
IMPORT  int     double_vortex_vel(POINTER,Front*,FT_POINT*,HYPER_SURF_ELEMENT*,
		                        HYPER_SURF*,double*);

	/*	fredist.c*/
IMPORT	boolean	f_perform_redistribution(HYPER_SURF*,Front*,boolean);
IMPORT	boolean	interface_is_tangled(CROSS*);
IMPORT	boolean	redist_needed(Front*,int);
IMPORT	int	redistribute(Front*,boolean,boolean);
IMPORT	void	Clear_redistribution_parameters(Front*);

	/* fredist1d.c*/
IMPORT	int	redistribute1d(Front*);

	/* fredist2d.c*/
IMPORT	boolean	backward_equi_curve_redistribute(Front*,CURVE*,boolean);
IMPORT	boolean	equi_curve_redistribute(Front*,CURVE*,boolean);
IMPORT	boolean	expand_redist_cur(Front*,CURVE*);
IMPORT	boolean	expansion_redistribute(Front*,boolean*);
IMPORT	boolean	force_tangle(void);
IMPORT	boolean	full_dec_redist_cur(Front*,CURVE*,boolean);
IMPORT	boolean	full_inc_redist_cur(Front*,CURVE*,boolean);
IMPORT	boolean	full_redistribute(Front*,boolean*);
IMPORT	int	join_curves_at_closed_nodes(INTERFACE*);
IMPORT	int	redistribute2d(Front*,boolean,boolean);

	/* fredist3d.c*/
IMPORT	int	redistribute3d(Front*,boolean,boolean);
IMPORT	int     recon_repeat();
IMPORT	boolean	surface_redistribute(Front*,boolean*);
IMPORT	void    set_repeat(int);
IMPORT	boolean    compute_smooth_para(SMOOTH_PARA*,FT_POINT*,TRI*,SURFACE*,
				SMOOTH_TOL*);
IMPORT  boolean    point_outside_open_bdry(int*,double*,FT_POINT*,INTERFACE*);
IMPORT	void	smooth_curve(CURVE*);
IMPORT	void 	triangle_height_vec(double*,double*,double*,double*);
IMPORT	boolean    compute_average_point(SMOOTH_PARA*,FT_POINT*,TRI*,SURFACE*,
					SMOOTH_TOL*);
IMPORT	double	min_null_pair_angle(double*,double*,double*,double*);
IMPORT	void	tecplot_interface_in_ball(const char*,INTERFACE*);



	/* frp1.c */
IMPORT	F_USER_RPROBLEM	*rp_user_hook(void);
IMPORT	RP_NODE	*add_to_rp_node_list(RPROBLEM*,NODE*,NODE*);
IMPORT	boolean	f_untrack_curve(O_CURVE*,O_CURVE*,COMPONENT,double,Front*,
				POINTER,RPROBLEM*,UNTRACK_FLAG);
IMPORT	int	is_bdry_type(RPROBLEM*);
IMPORT	int	is_null_curve(CURVE*,RPROBLEM*);
IMPORT	void	add_oc_curve_to_family(O_CURVE*,O_CURVE_FAMILY**);
IMPORT	void	delete_curve_from_o_curve_family(CURVE*,O_CURVE_FAMILY**);
IMPORT	void	delete_null_physical_curves(RPROBLEM*);
IMPORT	void	delete_o_curve_with_curve(O_CURVE**,CURVE*);
IMPORT	void	find_corr_cur_in_rp(O_CURVE*,O_CURVE*,Front*,RPROBLEM*);
IMPORT	void	find_curves_with_wave_type(NODE*,CURVE**,ORIENTATION*,
					   CURVE**,ORIENTATION*,int);
IMPORT	void	init_ocurve_lists(RPROBLEM*,Front*);
IMPORT	void	join_cfamilies(O_CURVE_FAMILY**,O_CURVE_FAMILY*);
IMPORT	void	roclists_after_invert(RPROBLEM*,CURVE*,O_CURVE*);
IMPORT	void	roclists_after_join(RPROBLEM*,CURVE*,O_CURVE*,CURVE*,
				    O_CURVE*,CURVE*);
IMPORT	void	roclists_after_split(RPROBLEM*,CURVE*,CURVE**,int);
IMPORT	void	rrpnlist_after_delete_node(RPROBLEM*,NODE*);
IMPORT	void	f_set_rp_statistics(RPROBLEM*);
IMPORT	void	set_states_at_node_by_propagate(Front*,POINTER,O_CURVE*,
						O_CURVE*,double);
IMPORT	void	augment_rproblem_list(RPROBLEM**,NODE**,double,double,
				      INTERFACE*,INTERFACE*,Front*,POINTER);
IMPORT	void	delete_null_boundary_curves(RPROBLEM*,Front*,POINTER);
IMPORT	void	delete_oc_curve_from_family(O_CURVE**,O_CURVE_FAMILY**);
IMPORT	void	f_delete_curve_from_rp_node(CURVE*,RP_NODE*,RPROBLEM*);
IMPORT	void	f_init_rp_nodes(RPROBLEM*);
IMPORT	void	free_o_curve_family(O_CURVE_FAMILY*);
IMPORT	void	free_rp(RPROBLEM*);
IMPORT	void	free_rp_list(RPROBLEM**);
IMPORT	void	fshow_intfc_states(FILE*,INTERFACE*);
IMPORT	void	init_cfamily(O_CURVE_FAMILY**,CURVE*,ORIENTATION);
IMPORT	void	init_o_curve(O_CURVE**,CURVE*,ORIENTATION);
IMPORT	void	merge_and_delete_overlaps(RPROBLEM*,RPROBLEM*);
IMPORT	void	merge_rproblems(RPROBLEM*,RPROBLEM*);
IMPORT	void	relocate_null_pointer(POINTER*,POINTER*);
IMPORT	void	reset_component_of_loop(CURVE*,ORIENTATION,
					ANGLE_DIRECTION,COMPONENT,Front*);
IMPORT  void 	f_init_2drproblem(RPROBLEM*,Front*);
IMPORT  int     f_2drproblem(Front*,Front*,POINTER,RPROBLEM**);

	/* frp2.c */
IMPORT	boolean	fixed_type_node(NODE*);
IMPORT	boolean	generate_boundary_cross_list(CROSS**,RPROBLEM*,Front*,POINTER);
IMPORT	boolean	rp_node_with_node(RP_NODE**,RPROBLEM*,NODE*);
IMPORT  boolean    find_curves_at_rp_with_status(O_CURVE**,O_CURVE**,
                                              int,RPROBLEM*,int);
IMPORT	int	incident_curve_crosses_fixed_node(Front*,Front*,POINTER,
						  RPROBLEM*);
IMPORT	int	join_propagated_curves(NODE**,CURVE**,O_CURVE**,O_CURVE**,
				       int,Front*,Front*,POINTER,RPROBLEM*);
IMPORT	int	phys_node_crosses_bdry(Front*,Front*,POINTER,RPROBLEM*,
				       NODE_FLAG);
IMPORT  int     bdry_rp_1i_0f(Front*,POINTER,RPROBLEM*);
IMPORT  int     bdry_rp_2i(Front*,Front*,POINTER,RPROBLEM*);
IMPORT  int     pp_curve_exits_at_bdry(Front*,Front*,POINTER,RPROBLEM**);
IMPORT	void	find_rpn_with_physical_node(RP_NODE**,RPROBLEM*,int);
IMPORT  void    find_curves_with_status(NODE*,CURVE**,ORIENTATION*,
                                        CURVE**,ORIENTATION*,int);

	/* fscatter.c*/
IMPORT	INTERFACE	*f_zoom_interface(INTERFACE*,RECT_GRID*,double*,
					  double*,double**);
IMPORT	boolean	scatter_front(Front*);
IMPORT	void	delete_subdomain_boundaries(INTERFACE*);
IMPORT	void 	clip_front_to_subdomain(Front*);
IMPORT  void    clip_front_to_rect_boundary_type(Front*);
IMPORT	void 	set_front_pp_grid(INIT_DATA*,Front*);
IMPORT  void    scatter_top_grid_float_array(double*,Front*);
IMPORT  void    scatter_cell_index(Front*,int*,int*,POINTER);
IMPORT	boolean	cpu_adapt_front(Front*,double,int*,int*);
IMPORT	boolean	f_intfc_communication1d(Front*);
IMPORT  void    pp_clip_rect_grids(Front*,int[3][2]);

	/* fscat2d.c*/
IMPORT	boolean	f_intfc_communication2d(Front*);
IMPORT	void 	cut_interface(INTERFACE*,double,int,int,boolean,boolean);
IMPORT	void	delete_subdomain_curves(INTERFACE*);
IMPORT  boolean    merge_interface(Front*,int);
IMPORT  void    clip_to_interior_region(INTERFACE*,int*,int*);
IMPORT  void    copy_interface_into(INTERFACE*,INTERFACE*);
#if defined(USE_OVERTURE)
IMPORT  void    clip_interface_with_rect(INTERFACE*,double*,double*,boolean);
IMPORT  void    set_cut_none_local_flag(int);
IMPORT  int     get_cut_none_local_flag(void);
IMPORT  void    set_min_sc_sep_val_flag(int);
IMPORT  void    set_use_delete_short_bonds_flag(void);
IMPORT  void    set_debug_cut_flag(int);  
#endif /* if defined(USE_OVERTURE) */

	/* fscat3d1.c*/
IMPORT	CURVE	*matching_curve(CURVE*,P_LINK*,int);
IMPORT	NODE	*matching_node(NODE*,P_LINK*,int);
IMPORT	SURFACE	*copy_buffer_surface(SURFACE*,P_LINK*,int);
IMPORT	boolean	curves_match(CURVE*,CURVE*,P_LINK*,int);
IMPORT	boolean	f_intfc_communication3d(Front*);
IMPORT	void	cut_out_curves_in_buffer(INTERFACE*);
IMPORT	void	install_subdomain_bdry_curves(INTERFACE*);
IMPORT	void	merge_two_tris(TRI*,TRI*,SURFACE*,SURFACE*);
IMPORT	void	remove_out_domain_tri(TRI*,SURFACE*);
IMPORT	void	strip_subdomain_bdry_curves(INTERFACE*);
IMPORT  void    set_floating_point_tolerance1(double*);
IMPORT  void    open_null_sides1(INTERFACE*,double*,double*,int,int);
IMPORT	void	communicate_default_comp(Front*);
IMPORT	void	strip_bdry_curves(INTERFACE*, int);
IMPORT	void	cut_surface(SURFACE*,boolean (*func)(POINTER,double*),POINTER,boolean);
IMPORT	void	install_hsbdry_on_surface(SURFACE*,int);
IMPORT	boolean	surfaces_matched(SURFACE*,SURFACE*);
IMPORT  void    install_subdomain_bdry_curves(INTERFACE*);
	
	/* fscat3d2.c*/
IMPORT	boolean	f_intfc_communication3d2(Front*);
IMPORT  void    clip_front_for_output(Front*,RECT_GRID*);
IMPORT	double	line_cross_tolerance(RECT_GRID*);
IMPORT  void    merge_curves(INTERFACE*,INTERFACE*);
IMPORT  void    merge_btris(BOND*,BOND*,CURVE*,ORIENTATION,INTERFACE*);
IMPORT  void    average_btris(TRI*,SURFACE*,TRI*,SURFACE*);
	
	/* fstate.c*/
IMPORT	ConstantFlowRegion	*SetConstantFlowRegion(COMPONENT,Locstate,
						       INTERFACE*);
IMPORT	FlowSpecifiedRegion	*SetSkipComponentRegion(COMPONENT);
IMPORT	FlowSpecifiedRegion	*SetSkipAllComponents(void);
IMPORT	FlowSpecifiedRegion	*AddToFsrList(FlowSpecifiedRegion*);
IMPORT	Locstate	f_alloc_intfc_state(size_t);
IMPORT	boolean	ComponentIsFlowSpecified(COMPONENT,Front*);
IMPORT	boolean	RegionIsFlowSpecified(Locstate,Locstate,double*,COMPONENT,
				      COMPONENT,Front*);
IMPORT	void	f_alloc_state(Locstate*,size_t);
IMPORT	void	f_clear_state(Locstate,size_t);
IMPORT	void	SetActiveFlowComponent(COMPONENT,Front*);
IMPORT	void	nearest_intfc_state_and_pt(double*,COMPONENT,Front*,Front*,
					   Locstate,double*,HYPER_SURF**);
IMPORT	boolean	f_sort_bond_tris(INTERFACE*);
/*#bjet2 */
IMPORT  boolean    f_assign_btri_states(BOND_TRI*,BOND_TRI*);

	/* fcheck3d.c */
IMPORT	boolean	f_consistent_interface(INTERFACE*);

	/* fstate2d.c */
IMPORT	void	set_states_by_interpolation(CURVE*,BOND*,BOND*,SIDE,Locstate,
					    Locstate,size_t);
IMPORT	void	states_at_distance_along_curve(FT_POINT*,BOND*,CURVE*,ORIENTATION,
					       double,int,Locstate*,Locstate*,
					       HYPER_SURF**,
					       HYPER_SURF_ELEMENT**,
					       double*,FT_POINT**,Front*);

	/* fsub.c*/
IMPORT	Tan_stencil	*alloc_tan_stencil(Front*,int);
IMPORT	MAX_FRONT_SPEED	*f_alloc_MaxFrontSpeed(MAX_FRONT_SPEED*,INTERFACE*,
                                               size_t);
IMPORT	FT_POINT	*f_average_points(boolean,FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*,
			               FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*);
IMPORT	double	f_max_front_time_step(Front*,double*);
IMPORT	int	syncronize_time_step_status(int,PP_GRID*);
IMPORT	void	assign_interface_and_free_front(Front*,Front*);
IMPORT	void	delete_passive_boundaries(INTERFACE*);
IMPORT	void	f_copy_into_front(Front*,Front*);
IMPORT	void	f_initialize_max_front_speed(Front*);
IMPORT	void	f_principal_tangent(FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*,
				    double*,double*);
IMPORT	void	f_set_default_front_parameters(INIT_DATA*,Front*);
IMPORT	void	f_set_max_front_speed(int,double,Locstate,double*,Front*);
IMPORT	void	assign_front_interface(Front*,Front*);
IMPORT	void	measure_front(Front*);
IMPORT	void	set_default_tan_stencil(Tan_stencil*);
IMPORT	double	length_of_scalar_wave(Front*);
IMPORT	ANGLE_DIRECTION	c1_to_c2_direction(O_CURVE*,O_CURVE*);
IMPORT	double	angle_from_c1_to_c2_at_common_node(CURVE*,ORIENTATION,
						   CURVE*,ORIENTATION,Front*);
IMPORT	double	f_mean_curvature_at_point2d(FT_POINT*,HYPER_SURF_ELEMENT*,
					    HYPER_SURF*,Front*);
IMPORT	double	f_mean_curvature_at_point3d(FT_POINT*,HYPER_SURF_ELEMENT*,
					    HYPER_SURF*,Front*);
IMPORT	double	f_wlsp_curvature(FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*,Front*);

/*#bjet2 */
IMPORT  void    set_full_average(boolean);

	/* ftop.c*/
IMPORT	boolean	f_is_subdomain_boundary(HYPER_SURF*);
IMPORT	void	f_reflect_point(FT_POINT*,double*,double*,INTERFACE*);
IMPORT	void	f_fset_hyper_surf_color(FILE*,HYPER_SURF*);
IMPORT	int	synchronize_untangle_status(int);
IMPORT	void	f_invert_curve(CURVE*);
IMPORT	void	f_reverse_curve(CURVE*);
IMPORT	boolean	f_untrack_point(FT_POINT*,COMPONENT,Front*);
IMPORT	CURVE	*f_attach_curve_to_node(CURVE*,FT_POINT*,BOND*,NODE*);
IMPORT	O_CURVE_FAMILY	*find_loop(CURVE*,ORIENTATION,ANGLE_DIRECTION);
IMPORT	boolean	f_delete_point_adjacent_to_node(Front*,CURVE*,ORIENTATION);
IMPORT	boolean	intfc_delete_fold_back_bonds(Front*);
IMPORT	boolean	f_is_subdomain_node(NODE*);
IMPORT	boolean	f_is_virtual_fixed_node(NODE*);
IMPORT  boolean	f_move_closed_loop_node(CURVE*,BOND*);
IMPORT	boolean	is_stationary_node(NODE*);
IMPORT	double	f_area_of_loop(CURVE*,ORIENTATION,CURVE*);
IMPORT	void	curve_delete_very_short_bonds(CURVE*);
IMPORT	void	delete_interior_points_of_curve(Front*,CURVE*);
IMPORT	void	f_delete_fold_back_bonds(Front*,CURVE*,double,int*,int*);
IMPORT	void	f_delete_small_loops(Front*);
IMPORT	void	f_reflect_curve2d(CURVE*,double*,double*);
IMPORT	void	f_reflect_node2d(NODE*,double*,double*);
IMPORT	void	intfc_delete_very_short_bonds(Front*);
IMPORT	void	split_curves_at_cross(CROSS*,Front*,NODE**,CURVE**,COMPONENT*,
				      COMPONENT*,CURVE**,COMPONENT*,COMPONENT*,
				      double,POINTER);
IMPORT	void	f_reflect_surface(SURFACE*,double*,double*);
IMPORT	boolean 	f_untrack_surface(SURFACE*,COMPONENT,Front*);
IMPORT	boolean	cross_rect_grid_bdry(BOND*,RECT_GRID*);

	/* funtan2d.c*/
IMPORT	CURVE	*next_curve_of_gen_curve(CURVE*,ORIENTATION,int*,NODE**);
IMPORT	NODE	*opp_node_of_gen_curve(CURVE*,ORIENTATION);
IMPORT	boolean	f_delete_loop(NNLIST*,NNLIST**,CURVE**,Front*,int,double,int);
IMPORT	boolean	f_replace_unphys_loop(NNLIST*,NNLIST**,CURVE**,Front*,
				      int,double,int);
IMPORT	boolean	is_scalar_vector_cross(CROSS*);
IMPORT	boolean	is_vector_vector_cross(CROSS*);
IMPORT	int	scalar_unravel(Front*,CROSS**,int);
IMPORT	int	f_grid_based_untangle(Front*,CROSS**);
IMPORT	int	f_elastic_untangle(Front*,CROSS**);
IMPORT	void	eliminate_small_loops(INTERFACE*,double,double,CROSS**);

	/*funtan3d.c*/
IMPORT	boolean scalar_unravel_3d(Front*,CROSS**);

	/* fuserintfc.c */
IMPORT	F_USER_INTERFACE *f_user_hook(int);
IMPORT	void	f_set_interface_hooks(int,INIT_DATA*);
IMPORT	void	f_preserve_user_hooks(int,PRESERVE_USER_HOOKS);
IMPORT	void	f_set_normal_function(const char*,NORMAL_FUNCTION*,INTERFACE*);
IMPORT	void	f_set_tangent_function(const char*,TANGENT_FUNCTION*,
                                       INTERFACE*);
IMPORT  void    linear_state_interpolator(double,double,double*,Locstate,double*,
                                       Locstate,RECT_GRID*,Locstate);
IMPORT  boolean    linear_tri_state_interpolator(double,double,double,double*,
                                       Locstate,double*,Locstate,double*,
				       Locstate,RECT_GRID*,Locstate);
IMPORT	void	set_tangent_operator(TANGENT_METHOD,int);
IMPORT	void	set_normal3d_method(NORMAL_METHOD,int);
IMPORT	void	FrontSetGeomVarMethods(Front*,TANGENT_METHOD,NORMAL_METHOD,
					CURVATURE_METHOD);

	/* fuserhooks.c */
IMPORT	MAX_FRONT_SPEED	*alloc_MaxFrontSpeed(MAX_FRONT_SPEED*,INTERFACE*,
                                             size_t);
IMPORT	Locstate read_print_state_data(INIT_DATA*,const IO_TYPE*,
                                       Locstate,INTERFACE*);
IMPORT	boolean	form_subintfc_via_communication(Front*);
IMPORT	boolean 	merge_hs_flags(HYPER_SURF*,HYPER_SURF*);
IMPORT	boolean	tri_interpolate_intfc_states(INTERFACE*,double,double,double,
					     double*,Locstate,double*,Locstate,
					     double*,Locstate,Locstate);
IMPORT	void	GetFrontCurvature(FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*,
					double*,Front*);
IMPORT	double	mean_curvature_at_point(FT_POINT*,HYPER_SURF_ELEMENT*,
					HYPER_SURF*,Front*);
IMPORT	boolean	nearest_intfc_state(double*,COMPONENT,INTERFACE*,Locstate,
				    double*,HYPER_SURF**);
IMPORT	const char *wave_type_as_string(int,INTERFACE*);
IMPORT	int	read_hsbdry_type_from_string(char*,INTERFACE*);
IMPORT	int	read_wave_type_from_string(const char*,INTERFACE*);
IMPORT	void	bi_interpolate_intfc_states(INTERFACE*,double,double,double*,
					    Locstate,double *,Locstate,Locstate);
IMPORT	void	delete_curve_from_rp_node(CURVE*,RP_NODE*,RPROBLEM*);
IMPORT	void	fprint_hsbdry_type(FILE*,const char*,int,const char*,
				   INTERFACE*);
IMPORT	void	fprint_intfc_state(FILE*,Locstate,INTERFACE*);
IMPORT	void	fprint_state_data(FILE*,Locstate,INTERFACE*);
IMPORT	void	fprint_wave_type(FILE*,const char*,int,const char*,INTERFACE*);
IMPORT	void	free_rp_node(RP_NODE*,RPROBLEM*);
IMPORT	void	init_rp_nodes(RPROBLEM*);
IMPORT	void	normal(FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*,double*,Front*);
IMPORT	void	print_intfc_state(Locstate,INTERFACE*);
IMPORT	void	print_rp_node(RP_NODE*,RPROBLEM*);
IMPORT	void	print_rproblem(RPROBLEM*);
IMPORT	void	reflect_state(Locstate,INTERFACE*,double*,double*,double*);
IMPORT	void	show_intfc_states(INTERFACE*);
IMPORT	void	set_phys_ocurves_to_null(RP_NODE*,RPROBLEM*);
IMPORT	void	set_rp_statistics(RPROBLEM*);
IMPORT	void	set_normal_function(const char*,NORMAL_FUNCTION*,INTERFACE*);
IMPORT	void	set_tangent_function(const char *s,TANGENT_FUNCTION*,
                                     INTERFACE*);
IMPORT	void	slsr(FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*,
		     Locstate*,Locstate*);
IMPORT	void	state_along_hypersurface_element(COMPONENT,double*,
						 HYPER_SURF_ELEMENT*,
						 HYPER_SURF*,Locstate);
IMPORT	void	tangent(FT_POINT*,BOND*,CURVE*,double*,Front*);
IMPORT	void	user_print_rp_node(RP_NODE*,RPROBLEM*);
IMPORT	void	user_print_rproblem(RPROBLEM*);
IMPORT	void	user_free_rp_node(RP_NODE*,RPROBLEM*);
IMPORT  void    assign_curve_boundary_type(CURVE*,int,int*);

#if defined(USE_OVERTURE)
        /* foverture_patch.c */
IMPORT  void    newfront_to_distri_table(Front*,Front*,int,Wv_on_pc**,int);
IMPORT  void    set_patch_front(Front*,Front*,RECT_GRID*,int);
IMPORT  void    set_patch_topo_grid(RECT_GRID*,RECT_GRID*);
IMPORT  boolean    clip_patch_front(Front*,int);
IMPORT  void    remove_patch_all_boundary_curves(Front*);
IMPORT  void    delete_patch_all_curves(Front*);
IMPORT  boolean    f_form_patch_subintfc_via_cut2d(Front*);
IMPORT  boolean    f_form_patch_subintfc_2d(Front*,COMPONENT);
IMPORT  void    assembly_fine_patch_fronts_to_one2d(Front**,Front*);
IMPORT  void    assembly_fine_patch_fronts_to_one2d_ver2(Front**,Front*);
IMPORT  void    assembly_fine_patch_fronts_to_one(Front**,Front*);
IMPORT  void    retrieve_boundary_curves_from_zero(Front*,INTERFACE*);
IMPORT  int     assembly_distribute_patch_fronts(Front**,int,Wv_on_pc**,
                    int,int);
IMPORT  void    send_front_misc(Front*,COMPONENT*,int);
IMPORT  void    recv_front_misc(Front*,COMPONENT*,int);
IMPORT  void    send_mini_front_misc(Front*,COMPONENT*,int);
IMPORT  void    recv_mini_front_misc(Front*,COMPONENT*,int);
IMPORT  int     set_copy_proc_frs(Front***,int,Wv_on_pc**,int,int*);
        /* foverture_adv.c */
IMPORT  int     normal_advance_front2d(double,double*,Front*,Front**,POINTER);
IMPORT  int     tangnt_advance_front2d(double,double*,Front*,Front**,POINTER);
IMPORT  int     redist_advance_front2d(double,double*,Front*,Front**,POINTER);
IMPORT  void    geomview_amr_fronts_plot2d(const char*,Front**,int,Wv_on_pc**,int);

#endif /* if defined(USE_OVERTURE) */

        /*testfront.c*/
IMPORT  void    test1d(Front*);

	/* fmap.c */
IMPORT  void FrontSwapAndFree(Front*,Front*);
IMPORT  void FT_Propagate2(Front*,Front**);
IMPORT  int FrontAdvance(double,double*,Front*,Front**,POINTER);
IMPORT  void FrontSetSpacing(Front*,double);
IMPORT  void FrontRedistMesh(Front*);
IMPORT	void FrontSetTriParams(Front*,double,double,double,double);
IMPORT	void FrontResetTriParams(Front*);
IMPORT	void FrontFreeAll(Front*);
IMPORT	boolean FrontCpuAdaptSubdomain(Front*,double,int*,int*);
IMPORT	boolean FrontNearestIntfcState(Front*,double*,COMPONENT,Locstate);
IMPORT  double FrontLinIntrp(double*,INTRP_CELL*,boolean);
IMPORT  double FrontBilinIntrp(double*,INTRP_CELL*,boolean);
IMPORT	boolean FrontGetRectCellIntrpCoeffs(double*,RECT_GRID*,int*,double*);
IMPORT HYPER_SURF *BoundaryHyperSurf(INTERFACE*,int,int,int);
IMPORT Tan_stencil **FrontGetTanStencils(Front*,FT_POINT*,int);
IMPORT boolean FrontGetPointChain(FT_POINT*,FT_POINT**,int);
IMPORT void FrontPreAdvance(Front*);
IMPORT boolean FrontReflectPointViaNeumannBdry(double*,double*,double*,
		COMPONENT,HYPER_SURF*,Front*);

IMPORT	double FrontHypTimeStep(Front*);
#if defined(c_plusplus) || defined(__cplusplus)
}
#endif

#include <front/fapi.h>
#endif /* !defined(_FPROTOS_H) */
