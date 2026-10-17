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
*				iprotos.h:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*/

#if !defined(_IPROTOS_H)
#define _IPROTOS_H

#include <intfc/int.h>

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif

 /* libintfc.a EXPORTED Function Declarations*/

/*	comp.c*/
IMPORT	COMPONENT	component(double*,INTERFACE*);
IMPORT	COMPONENT	long_component(double*,INTERFACE*);
IMPORT	COMPONENT	nearest_interior_comp(boolean,COMPONENT,
					      double*,INTERFACE*);
IMPORT	COMPONENT	new_component(COMPONENT);
IMPORT	boolean	is_excluded_comp(COMPONENT,INTERFACE*);
IMPORT	int	check_comps_at_nodes(INTERFACE*,O_NODE**);
IMPORT	int	comps_consistent_at_node(NODE*);
IMPORT	void	exclude_comp(COMPONENT,INTERFACE*);
IMPORT	void	set_topological_grid(INTERFACE*,RECT_GRID*);
IMPORT	void	show_COMP(FILE*,INTERFACE*);

/*	iblkb.c*/
IMPORT	int construct_bdry_blk(BLK_CRX*,BLK_TRI*);

/*	iecomps.c*/
IMPORT	boolean	equivalent_comps(COMPONENT,COMPONENT,INTERFACE*);
IMPORT	const COMPONENT	*equivalent_components_list(COMPONENT,int*,INTERFACE*);
IMPORT  void	fprint_comp_equiv_lists(FILE*,INTERFACE*);
IMPORT  void	fprint_comp_equiv_lists(FILE*,INTERFACE*);
IMPORT	void	set_equivalent_comps(COMPONENT,COMPONENT,INTERFACE*);

/*	ifourier.c*/
IMPORT  FOURIER_POLY    *get_fourier_bubble(double*,double*,int,const char*);
IMPORT  FOURIER_POLY    *get_fourier_multi_mode(double*,double*,int,const char*);
IMPORT  FOURIER_POLY    *get_fourier_mixed(double*,double*,int,const char*);
IMPORT  FOURIER_POLY    *get_fourier_random(double*,double*,int,const char*);
IMPORT	FOURIER_POLY	*allocate_fourier_poly(int,int,ALIGN*);
IMPORT	FOURIER_POLY	*get_fourier_coeffs(double*,double*,int,const char*);
IMPORT	LEGENDRE_POLY	*get_legendre_coeffs(double,const char*);
IMPORT  LEGENDRE_POLY   *get_legendre_mixed(double,const char*);
IMPORT  LEGENDRE_POLY   *get_legendre_multi_mode(double,const char*,const char*);
IMPORT  LEGENDRE_POLY   *get_legendre_random(double,const char*);
IMPORT  LEGENDRE_POLY   *allocate_legendre_poly(int,ALIGN*);
IMPORT	double		fourier_poly(double*,FOURIER_POLY*);
IMPORT	double		legendre_poly(double,LEGENDRE_POLY*);
IMPORT	int		random_bubble_num_modes(const char*,int*,int*,int);
IMPORT	int		spherical_num_modes(const char*,int*,int*,double*);
IMPORT	void		init_random_modes(int,int,int,int,FOURIER_POLY*,
					  double*,double*);

/*	intfc.c*/
IMPORT	BOND	*i_Bond(FT_POINT*,FT_POINT*);
IMPORT	CURVE	**split_curve(FT_POINT*,BOND*,CURVE*,
			      COMPONENT,COMPONENT,COMPONENT,COMPONENT);
IMPORT	CURVE	*curve_of_bond(BOND*,INTERFACE*);
IMPORT	CURVE	*i_copy_curve(CURVE*,NODE*,NODE*);
IMPORT	CURVE	*i_make_curve(COMPONENT,COMPONENT,NODE*,NODE*);
IMPORT	CURVE	*join_curves(CURVE*,CURVE*,COMPONENT,COMPONENT,BOND**);
IMPORT	INTERFACE	*i_copy_interface(INTERFACE*);
IMPORT	INTERFACE	*current_interface(void);
IMPORT	INTERFACE	*make_interface(int);
IMPORT	INTERFACE	*read_print_interface(INIT_DATA*,const IO_TYPE*,
                                              boolean,int*);
IMPORT	INTERFACE	*read_interface(void);
IMPORT	HYPER_SURF *i_make_hypersurface(COMPONENT,COMPONENT);
IMPORT	HYPER_SURF_BDRY	*i_make_hypersurface_boundary(void);
IMPORT	NODE	*i_copy_node(NODE*);
IMPORT	NODE	*i_make_node(FT_POINT*);
IMPORT	NODE	*node_of_point(FT_POINT*,INTERFACE*);
IMPORT	FT_POINT	*i_Point(double*);
IMPORT	FT_POINT	*i_Static_point(INTERFACE*);
IMPORT	FT_POINT	*i_copy_point(FT_POINT*);
IMPORT	FT_POINT	*i_make_point(double*,COMPONENT,COMPONENT);
IMPORT	POINTER	init_table_Store(size_t,INIT_DATA*);
IMPORT	POINTER	Store(size_t);
IMPORT	POINTER	store(size_t);
IMPORT	boolean	exists_interface(INTERFACE*);
IMPORT	boolean	i_delete_end_of_bond(BOND*,CURVE*);
IMPORT	boolean	i_delete_node(NODE*);
IMPORT	boolean	i_delete_start_of_bond(BOND*,CURVE*);
IMPORT	boolean	i_insert_point_in_bond(FT_POINT*,BOND*,CURVE*);
IMPORT	boolean	next_bond(INTERFACE*,BOND**,CURVE**);
IMPORT	boolean	next_curve(INTERFACE*,CURVE**);
IMPORT	boolean	next_hypersurface1d(INTERFACE*,HYPER_SURF**);
IMPORT	const char *i_boundary_type_as_string(int);
IMPORT	double	i_random01(INTERFACE*);
IMPORT	int	i_delete_curve(CURVE*);
IMPORT	int	i_delete_interface(INTERFACE*);
IMPORT	int	i_delete_point(FT_POINT*);
IMPORT	struct	Table	*interface_table_list(void);
IMPORT	struct	Table	*table_of_interface(INTERFACE*);
IMPORT	uint64_t	bond_number(BOND*,INTERFACE*);
IMPORT	uint64_t	curve_number(CURVE*);
IMPORT	uint64_t	hypersurface_boundary_number(HYPER_SURF_BDRY*);
IMPORT	uint64_t	hypersurface_element_number(HYPER_SURF_ELEMENT*,
						    INTERFACE*);
IMPORT	uint64_t	hypersurface_number(HYPER_SURF*);
IMPORT	uint64_t	interface_number(INTERFACE*);
IMPORT	uint64_t	node_number(NODE*);
IMPORT	uint64_t	point_number(FT_POINT*);
IMPORT	uint64_t	table_number(struct Table*);
IMPORT	int	i_read_boundary_type_from_string(const char*);
IMPORT	void	delete_from_cross_list(CROSS*);
IMPORT	void	fprint_hypersurface(FILE*,HYPER_SURF*);
IMPORT	void	i_fprint_interface(FILE*,INTERFACE*);
IMPORT	void	print_hypersurface(HYPER_SURF*);
IMPORT	void	print_hypersurface_boundaries(HYPER_SURF_BDRY**);
IMPORT	void	set_current_interface(INTERFACE*);
IMPORT	FT_POINT *i_average_points(boolean,FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*,
	                             FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*);
IMPORT	ORIENTATION orientation_of_bond_at_tri(BOND*,TRI*);
IMPORT	boolean	delete_side_of_tri(TRI*,SURFACE*,int);
IMPORT	boolean	delete_vertex_of_tri(FT_POINT*pt,TRI*,SURFACE*);
EXPORT	boolean	flip_diagonal(TRI*,int);
EXPORT	boolean	retriangulate_polygon(FT_POINT**,int,FT_POINT**,int,const double*,
				      TRI**,int,SURFACE*,BOND_TRI**,TRI**,
				      TRI***,int*);
IMPORT	int side_of_tri_with_bond(BOND*,TRI*);
IMPORT	uint64_t	bond_tri_number(BOND_TRI*,INTERFACE*);
IMPORT	uint64_t	surface_number(SURFACE*);
IMPORT	uint64_t	tri_number(TRI*,INTERFACE*);
IMPORT  void 	order_interface(INTERFACE *); 
IMPORT  void 	delete_scn(SURFACE *); 
IMPORT  boolean    change_node_of_closed_curve(FT_POINT*, CURVE*);
IMPORT  void    reset_nodes_posn(INTERFACE *);


/*	irefl.c*/
IMPORT	void	i_reflect_node(NODE*,double*,double*);
IMPORT	void	i_reflect_curve(CURVE*,double*,double*);
IMPORT	void	i_reflect_point(FT_POINT*,double*,double*,INTERFACE*);
IMPORT	void	i_reflect_surface(SURFACE*,double*,double*);

/*	iscatter.c*/
IMPORT	int	domain_id(int*,int*,int);
IMPORT	int	neighbor_id(int*,int*,int,int,PP_GRID*);
IMPORT	void	find_Cartesian_coordinates(int,PP_GRID*,int*);
IMPORT	void	print_PP_GRID_structure(PP_GRID*);
IMPORT  PP_GRID *set_pp_grid(INIT_DATA*,RECT_GRID*);

/*	isub.c*/
IMPORT  const char *grid_direction_name(GRID_DIRECTION);
IMPORT  void    init_seg_crx_lists(INTERFACE*,int,int);
IMPORT	BDRY_SIDE rect_bdry_side_for_curve(int*,int*,CURVE*,RECT_GRID*);
IMPORT	P_LINK	*add_to_hash_table(POINTER,POINTER,P_LINK*,int);
IMPORT	POINTER	find_from_hash_table(POINTER,P_LINK*,int);
IMPORT	boolean	i_is_comp_in_list(COMPONENT,COMP_LIST*);
IMPORT	double	scalar_product_on_bonds(BOND*,BOND*,int);
IMPORT	double	scaled_bond_length(BOND*,double*,int);
IMPORT	void	scaled_tri_params(TRI*,double*,double*,double*);
#if !defined(separation)
IMPORT double separation(FT_POINT*,FT_POINT*,int);
#endif /* !defined(separation) */
IMPORT	int	curve_in_curve_list(CURVE*,CURVE**);
IMPORT	boolean	is_c_on_intfc(CURVE*);
IMPORT	boolean	is_b_on_curve(CURVE*,BOND*);
IMPORT	boolean    pointer_in_list(POINTER,int,POINTER*);
IMPORT	boolean    integer_in_list(int,int,int*);
IMPORT	void	i_add_comp_to_list(COMPONENT,COMP_LIST*,INTERFACE*);
IMPORT	void	rect_bdry_side_for_hyper_surf(int*,int*,HYPER_SURF*,
					      RECT_GRID*);
IMPORT	void	reset_hash_table(P_LINK*,int);
IMPORT	void	reset_intfc_num_points(INTERFACE*);
IMPORT	void	vector_product_on_bonds(BOND*,BOND*,int,double*);
IMPORT	double	area_of_closed_curve(CURVE*);
IMPORT	ANGLE_DIRECTION	c1_to_c2_direction(O_CURVE*,O_CURVE*);
IMPORT	boolean	robust_cross_bonds(BOND*,int,BOND*,int,double*,double*,
				   RECT_GRID*,FT_POINT*);
IMPORT	int	cross_sign(BOND*,BOND*);
IMPORT	int	intersect_bond_with_curve_segment(BOND*,BOND*,BOND*,O_CURVE*,
					  BOND**,FT_POINT*,RECT_GRID*);
IMPORT	int	is_short_curve(CURVE*,ORIENTATION,RECT_GRID*,double);
IMPORT	int	robust_cross_bond_circle(BOND*,FT_POINT*,double,double*,FT_POINT*);
IMPORT	int	robust_extend_bond_to_cross_bond(BOND*,ORIENTATION,BOND*,
					double*,double*,FT_POINT*,double*,int);
IMPORT	int	robust_extend_bonds_to_cross(BOND*,ORIENTATION,
					     int,BOND*,ORIENTATION,int,
					     FT_POINT*,double*,double*,FT_POINT*,
					     RECT_GRID*);
IMPORT	int	robust_quad_roots_in_interval(double*,double,double,double,
					      double,double,double);
IMPORT	void	big_angle(BOND*,CURVE*,BOND*,CURVE*,double*,double*,RECT_GRID*);
IMPORT 	int  	seg_index2d(int,int,GRID_DIRECTION,int*);
IMPORT	int	set_tri_list_around_point(FT_POINT*,TRI*,TRI***,INTERFACE*);
IMPORT	int	tri_list_computed_by_normal(FT_POINT*,TRI*,TRI***,INTERFACE*);
IMPORT	const double* const *side_vector(const TRI*);
IMPORT	const double        *vector_on_tri_side(const TRI*,int,double*);
IMPORT	const double        *length_side(const TRI*);
IMPORT	const double	   *Tri_normal(const TRI*);
IMPORT	double              length_of_tri_side(const TRI*,int);
IMPORT	double              sqr_norm(const TRI*);
IMPORT	void	area_weighted_normal3d(FT_POINT*,HYPER_SURF_ELEMENT*,
                                       HYPER_SURF*,double*);
IMPORT	void	sine_weighted_normal3d(FT_POINT*,HYPER_SURF_ELEMENT*,
                                       HYPER_SURF*,double*);
IMPORT	void	average_position_of_surface(double*,SURFACE*);
IMPORT	void	omit_vertex_in_plane_fit(void);
IMPORT	void	plane_fit_normal3d(FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*,
				   double*);
IMPORT	void	reset_normal_on_intfc(INTERFACE*);
IMPORT	void	set_normal_of_tri(TRI*);

IMPORT	void	PointArrayRing1(FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*,int*,
				FT_POINT**);
IMPORT	void	PointArrayRing2(FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*,int*,
				int*, FT_POINT**,FT_POINT**);
IMPORT	void	BondAndNeighbors(HYPER_SURF_ELEMENT*,HYPER_SURF*,int*,BOND**,
				int);
IMPORT	void	TriAndFirstRing(HYPER_SURF_ELEMENT*,HYPER_SURF*,int*,TRI**);
IMPORT	void	TriAndFirstTwoRings(HYPER_SURF_ELEMENT*,HYPER_SURF*,int*,TRI**);
IMPORT  int     two_points_share_side(FT_POINT*,TRI*,FT_POINT*,INTERFACE*);
IMPORT 	int  	seg_index3d(int,int,int,GRID_DIRECTION,int*);
IMPORT 	int  	face_index3d(int,int,int,GRID_DIRECTION,int*);
IMPORT  void    init_face_crx_lists(INTERFACE*,int,int);
IMPORT  void    set_normal_from_tris(FT_POINT*,TRI**,int,double*);
IMPORT  void    plane_fit_normal3d_along_wall(double*, FT_POINT*, TRI**,int,TRI**,int);

IMPORT  void print_crxings(CRXING*,boolean);
IMPORT  const char *crossing_direction_name(CROSSING_DIRECTION);
IMPORT boolean WLSP_compute_normal2d(FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*);
IMPORT boolean WLSP_compute_normal3d(FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*);
IMPORT boolean WLSP_compute_normal3d0(FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*);
IMPORT void reset_surface_points(SURFACE*);
IMPORT void PointAndFirstRingTris(FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*,int*,
			TRI**);

/*	iuserintfc.c*/
IMPORT	I_USER_INTERFACE	*i_user_hook(int);
IMPORT	void	SetChunkSize(size_t);
IMPORT	void	i_preserve_user_hooks(int,PRESERVE_USER_HOOKS);

/*	ppcopy.c*/
IMPORT	INTERFACE	*i_receive_interface(int);
IMPORT	POINTER	_new_address(INTERFACE*,POINTER,POINTER*,POINTER*,int);
IMPORT	void	i_reconstruct_bond_pointers(BOND*,INTERFACE*,INTERFACE*,
					    POINTER*,POINTER*,int);
IMPORT	void	i_reconstruct_curve_pointers(CURVE*,INTERFACE*,INTERFACE*,
					     POINTER*,POINTER*,int);
IMPORT	void	i_reconstruct_interface_pointers(INTERFACE*,struct Table*,
						 POINTER*,POINTER*);
IMPORT	void	i_reconstruct_node_pointers(NODE*,INTERFACE*,INTERFACE*,
					    POINTER*,POINTER*,int);
IMPORT	void	i_reconstruct_point_pointers(FT_POINT*,INTERFACE*,INTERFACE*,
					     POINTER*,POINTER*,int);
IMPORT	void	i_send_interface(INTERFACE*,int);

/*	shift.c*/
IMPORT	INTERFACE *remap_interface(INTERFACE*,
				   void (*)(FT_POINT*,BOND*,CURVE*,FT_POINT*,BOND*,
					   CURVE*,boolean,RECT_GRID*,POINTER),
				   void (*)(INTERFACE*,INTERFACE*,
					   void (*remap)(FT_POINT*,BOND*,CURVE*,
							FT_POINT*,BOND*,CURVE*,
							boolean,RECT_GRID*,
							POINTER),
					   POINTER),
				   POINTER);

/*	top.c*/
IMPORT	BDRY_SIDE boundary_side(const double*,const RECT_GRID*,double);
IMPORT	BDRY_SIDE nearest_boundary(double*,RECT_GRID*);
IMPORT	BOND	*random_bond_on_curve(CURVE*);
IMPORT	CURVE	*adjacent_curve(CURVE*,ORIENTATION,ANGLE_DIRECTION,
				ORIENTATION*);
IMPORT	CURVE	*i_attach_curve_to_node(CURVE*,FT_POINT*,BOND*,NODE*);
IMPORT	CURVE	*i_make_fourier_curve(int,double,double,FOURIER_POLY*,
				      COMPONENT,COMPONENT);
IMPORT	CURVE	*read_curve_from_file(COMPONENT,COMPONENT,NODE*,NODE*,char*);
IMPORT	O_NODE	*make_onode(NODE*);
IMPORT	boolean	bonds_in_strict_order(BOND*,BOND*);
IMPORT	boolean	points_in_strict_order(FT_POINT*,BOND*,FT_POINT*,BOND*,int);
IMPORT	const char *bdry_side_as_string(BDRY_SIDE);
IMPORT	double	curve_length(CURVE*);
IMPORT	int	intersect_ray_with_boundary(double*,double*,double*,double*,
					    double*,int);
IMPORT	int	intersect_ray_with_curve(FT_POINT*,double*,BOND*,BOND*,
					 CURVE*,ORIENTATION,BOND**,FT_POINT*);
IMPORT	int	intersect_ray_with_sector(FT_POINT*,FT_POINT*,double*,double**,
					  double*,int);
IMPORT	boolean	i_move_closed_loop_node(CURVE*,BOND*);
IMPORT	int	num_curves_at_node(NODE*,int*,int*);
IMPORT	void	change_node_of_curve(CURVE*,ORIENTATION,NODE*);
IMPORT	void	i_cut_curve(FT_POINT*,BOND*,CURVE*,ORIENTATION);
IMPORT	void	copy_o_curve(O_CURVE*,O_CURVE*);
IMPORT	void	delete_list(POINTER**);
IMPORT	void	i_invert_curve(CURVE*);
IMPORT	void	i_reverse_curve(CURVE*);
IMPORT	void	invert_bond(BOND*);
IMPORT	void	merge_and_delete_nodes(NODE*,NODE*);
IMPORT	void	nearest_boundary_point(double*,double*,RECT_GRID*);
IMPORT	void	plot_interface(INTERFACE*,const char*,int*,int*,const char*);
IMPORT	void	print_bdry_side(const char*,BDRY_SIDE,const char*);
IMPORT	void	print_bond(BOND*);
IMPORT	void	print_bond_list(CURVE*);
IMPORT	void	print_curve_with_orient(CURVE*,ORIENTATION);
IMPORT	void	print_int_vector(const char*,const int*,int,const char*);
IMPORT	void	print_o_curve(O_CURVE*);
IMPORT	void	print_o_curve_family(O_CURVE_FAMILY*);
IMPORT	void	print_onode(O_NODE*);
IMPORT	void	print_onode_list(O_NODE**);
IMPORT	void	rbl_after_move_node(NODE*);
IMPORT	void	set_bond_length(BOND*,int);
IMPORT	void	set_point_of_bond(FT_POINT*,BOND*,ORIENTATION,int);
IMPORT	void	update_num_points(INTERFACE*);
IMPORT	int	num_points_on_curve(CURVE*);
IMPORT	void	invert_tri(TRI*);
IMPORT	void	i_invert_surface(SURFACE*);

/*	zoom.c*/
IMPORT	INTERFACE	*i_zoom_interface(INTERFACE*,RECT_GRID*,
					  double*,double*,double**);
IMPORT	void    clip_interface2d(INTERFACE*);
IMPORT	void	rotate_and_zoom_rect_grid(RECT_GRID*,double*,double*,double**);

/*	comp1d.c*/
IMPORT	boolean consistent_components1d(INTERFACE*);
IMPORT	boolean	make_point_comp_lists(INTERFACE*);
IMPORT	void	show_point_comp_lists(INTERFACE*);
IMPORT  void    reset_intfc_components(INTERFACE*);

/*	setb1d.c*/


/*	comp2d.c*/
IMPORT	boolean	make_bond_comp_lists(INTERFACE*);
IMPORT	double	shortest_distance2d(double*,BOND*,FT_POINT**,double*,SIDE*);
IMPORT  void    show_box_comp_crx(int*,int*,int*,COMPONENT*,int*);

/*	cross2d.c*/
IMPORT	int	find_companion_cross(CROSS*,CROSS**,ORIENTATION*,ORIENTATION*,
				     ORIENTATION*,ORIENTATION*);
IMPORT	void	add_to_cross_list(CROSS**,CURVE*,BOND*,CURVE*,BOND*,FT_POINT*);
IMPORT	void	insert_in_cross_list(CROSS*,CROSS*);
IMPORT	void	print_cross(CROSS*);
IMPORT	void	print_cross_list(CROSS*);
IMPORT	void	rcl_after_delete_bond_fragment_at_node(CROSS*,FT_POINT*,
						       CURVE*,ORIENTATION);
IMPORT	void	rcl_after_insert_point(CROSS*,FT_POINT*,BOND*);
IMPORT	void	rcl_after_join(CROSS*,CURVE*,CURVE*,CURVE*);
IMPORT	void	rcl_after_split(CROSS*,FT_POINT*,BOND*,CURVE*,CURVE**);

/*	isect2d.c*/
IMPORT	boolean	bond_crosses_curve(BOND*,CURVE*,FT_POINT*,BOND**,ORIENTATION);
IMPORT	boolean	cross_bonds(BOND*,BOND*,FT_POINT*);

/*	isurgery.c */
IMPORT	void	identify_detached_surface_curve_pair(INTERFACE*);
IMPORT	int 	next_null_sided_tri(TRI*,FT_POINT*,TRI**);
IMPORT	int 	prev_null_sided_tri(TRI*,FT_POINT*,TRI**);

/*	setb1d.c*/
IMPORT  boolean    i_set_boundary1d(INTERFACE*,RECT_GRID*,COMPONENT,double);
/*	setb2d.c*/
IMPORT	boolean	i_set_boundary2d(INTERFACE*,RECT_GRID*,COMPONENT,double);
/*      igview.c */
IMPORT  void    geomview_intfc_plot2d(const char*,INTERFACE*,RECT_GRID*);
IMPORT  void    tecplot_interface(const char*,INTERFACE*);
IMPORT  void    tecplot_blk_intfc_plot(const char *,BLK_TRI *);
IMPORT  void    tecplot_curve(const char*,FILE*,CURVE*);
IMPORT  void    tecplot_surface(const char*,FILE*,SURFACE*);
IMPORT  void    tecplot_surface_in_ball(const char*,SURFACE*);
IMPORT  void    tecplot_box(const char*,FILE*,double*,double*);
IMPORT	void	tecplot_tris(const char *,TRI **, int);
IMPORT  void    tecplot_show_tris(const char*, TRI **, int, FILE *);
IMPORT	void	tecplot_show_box_tri(const char*,RECT_BOX*,TRI**,int,FILE*);
IMPORT	void	tecplot_show_box_tris(const char*,TRI**,int,RECT_GRID*,int*);
IMPORT	void    tecplot_interface_in_box(const char*,FILE*,int*,int*,
				INTERFACE*);
IMPORT	void	set_shift_for_tecplot(double, double, double);
IMPORT	void    tecplot_surface_in_ball(const char *, SURFACE*);
IMPORT	void	set_tst_posn(double*);
IMPORT	void	vtk_interface_plot(const char*,INTERFACE*,boolean,double,int);
IMPORT	void	sdl_interface_plot(const char*,INTERFACE*);
#if defined __GD__
IMPORT  void    gd_initplot(char*,char*,double,double,double,double,int);
IMPORT  void    gd_appendplot(char*,char*,double,double,double,double,int);
IMPORT  void    gd_plotdata(int,double*,double*);
IMPORT  void    gd_plotframe(char*);
IMPORT  void    gd_closeplot();
IMPORT  void    gd_2d_intfc(char*,char*,INTERFACE*,RECT_GRID*,int,boolean);
#endif /* defined __GD__ */
IMPORT	char	*get_vtk_file_name(char*,const char*,const char*,size_t*);
IMPORT	char	*get_cgns_file_name(char*,const char*,const char*,size_t*);

/*	comp3d.c*/
IMPORT	boolean	make_tri_comp_lists(INTERFACE*);
IMPORT	boolean	within_tri(const double*,const double*,const double*,
	                   const double*,const double*,double);
IMPORT	boolean	i_insert_point_in_tri(FT_POINT*,TRI*,SURFACE*);
IMPORT	boolean	i_insert_point_in_tri_side(FT_POINT*,int,TRI*,SURFACE*);
IMPORT	boolean	i_undo_insert_point_in_tri(FT_POINT*,TRI*,SURFACE*);
IMPORT	boolean	i_undo_insert_point_in_tri_side(FT_POINT*,int,TRI*,SURFACE*);
IMPORT  boolean    tri_edge_crossing(TRI*,double*,double*,int,int*,int*,double*);
IMPORT	void	assign_tri_icoords(RECT_GRID*,TRI*);
IMPORT  boolean    line_tri_crossing(double*,TRI*,double*,double*,double);
IMPORT  boolean    line_point_projection(double*,int*,double*,double*,double*,double);
IMPORT  boolean    is_tri_outside(INTERFACE*,TRI*,RECT_GRID*);
IMPORT  boolean    is_tri_outside_box(TRI*, double **);
IMPORT  boolean    is_outside_surface(INTERFACE*,SURFACE*,RECT_GRID*);
IMPORT  boolean    is_outside_surfaces(INTERFACE*,RECT_GRID*);
IMPORT  void    delete_outside_surface(INTERFACE *);

/* 	idiagnostic.c*/
IMPORT	int  	index_of_pointer(POINTER*,POINTER);
IMPORT  int 	points_on_surface(SURFACE*);
IMPORT 	void 	print_blk_tri(BLK_TRI*);
IMPORT	void 	print_bond(BOND*);
IMPORT  void 	points_of_interface(INTERFACE*);
IMPORT	boolean	the_tri(TRI*);
IMPORT	boolean	the_side(TRI*);
IMPORT	boolean	the_BOND(BOND*);
IMPORT	boolean	the_point(FT_POINT*);
IMPORT	boolean	the_pt(double *);
IMPORT 	boolean	check_tri_and_neighbor(TRI*);
IMPORT  boolean 	search_the_tri_in_intfc(INTERFACE*);
IMPORT  boolean 	search_the_tri_in_surf(SURFACE*);
IMPORT	void	print_tri_coords(TRI*);
IMPORT  void 	find_blk_tri(BLK_TRI *);

	/*igview.c*/
IMPORT	void	gview_bounding_box(FILE*,const double*,const double*,
	                           int,const char*);
IMPORT  void    gview_cube(FILE*,const double*,const double*);
IMPORT  void 	gview_local_surface(SURFACE*,const char*,const char*,
				    SURFACE_COLOR,const double*,double);
IMPORT	void	gview_plot_axes(const char*,const char*,const double*,
				const double*,const double*,const double*);
IMPORT	void	gview_plot_coord_sys(const char*,const char*,const double*,
	                             const double*,const double*,const double*,
				     const double*,const double*);
IMPORT	void	gview_plot_polyline(const char*,const char*,FT_POINT**,
	                            int,boolean,double,double,double,double,
				    const double*,const double*);
IMPORT	void	gview_plot_triangle_list(const char*,const char*,TRI**,int,
	                                 double,double,double,double,double,double,
					 double,const double*,const double*);
IMPORT	void	gview_plot_vertices(const char*,const char*,FT_POINT**,
	                            int,const double*,const double*);
IMPORT	void	gview_plot_c_curve(const C_CURVE*,int,const char*);
IMPORT  void    gview_plot_curve(const CURVE*,const char*,const char*,
				 SURFACE_COLOR,int);
IMPORT	void	gview_plot_tri_and_point_list(const char*,TRI**,
	                                      const double*,int,double* const*,
					      SURFACE_COLOR,double,int,int);
IMPORT	void	gview_plot_tri_list(const char*,TRI**,int);
IMPORT	void	gview_intfc_within_range(const char*,INTERFACE*,
					double*,double);
IMPORT  void    gview_polyline(const char*,const char*,double* const*,
			       int,SURFACE_COLOR,int);
IMPORT  void 	gview_surface(SURFACE*,const char*,SURFACE_COLOR);
IMPORT	void	geomview_interface_plot(const char*,INTERFACE*,RECT_GRID*);
IMPORT	void	set_point_list_bounding_box(FT_POINT**,int,double*,
	                                    double*,boolean,boolean);
IMPORT	void	set_tri_list_bounding_box(TRI**,int,double*,double*,
	                                  boolean,boolean);
IMPORT	void	set_vector_bounding_box(const double*,const double*,double,
	                                double*,double*,boolean,boolean);
IMPORT	void	gview_point_tri_rings(const char*,FT_POINT*);

/*	int3d.c*/
IMPORT	BOND_TRI *i_link_tri_to_bond(BOND_TRI*,TRI*,SURFACE*,BOND*,CURVE*);
IMPORT	SURFACE	*i_copy_surface(SURFACE*,CURVE**,CURVE**,boolean);
IMPORT	SURFACE *i_join_surfaces(CURVE*);
IMPORT	SURFACE	*i_make_surface(COMPONENT,COMPONENT,CURVE**,CURVE**);
IMPORT	SURFACE	*i_read_surface(INTERFACE*,int);
IMPORT	TRI	*i_make_tri(FT_POINT*,FT_POINT*,FT_POINT*,POINTER,POINTER,POINTER,int);
IMPORT	TRI	*Next_tri_at_vertex(TRI*,FT_POINT*);
IMPORT	TRI	*Prev_tri_at_vertex(TRI*,FT_POINT*);
IMPORT	boolean    curve_is_in_surface_bdry(SURFACE*,CURVE*,ORIENTATION*);
IMPORT	boolean    find_surfaces_to_join_at_curve(CURVE*,SURFACE**,SURFACE**);
IMPORT	boolean    i_sort_bond_tris(INTERFACE*);
IMPORT	boolean	next_tri(INTERFACE*,TRI**,SURFACE**);
IMPORT	boolean	remove_curve_from_surface_bdry(SURFACE*,CURVE*,ORIENTATION);
IMPORT	boolean	tris_on_side_of_bond_for_join(BOND*,TRI**,int*,TRI**,int*);
IMPORT	boolean	link_neighbor_tris(TRI*,TRI*);
IMPORT	int	i_delete_surface(SURFACE*);
IMPORT	void 	i_reverse_bond(BOND*);
IMPORT	void 	i_reorder_curve_link_list(CURVE*);
IMPORT	void	insert_tri_at_head_of_list(TRI*,SURFACE*);
IMPORT	void	insert_tri_at_tail_of_list(TRI*,SURFACE*);
IMPORT	void	install_curve_in_surface_bdry(SURFACE*,CURVE*,ORIENTATION);
IMPORT	void	link_tri_list_to_surface(TRI*,TRI*,SURFACE*);
IMPORT	void	null_tri_array_numbers(INTERFACE*);
IMPORT	void	print_tri(TRI*,INTERFACE*);
IMPORT	void	remove_tri_from_surface(TRI*,SURFACE*,boolean);
IMPORT	void	rotate_triangle(TRI*,int);
IMPORT  boolean    assign_btri_states(BOND_TRI*, BOND_TRI*);
IMPORT  SURFACE *detach_one_surface(SURFACE *);
IMPORT  void    print_wall_crx(const char*,int*,int,int,CRXING*);
IMPORT  void    print_wall_curve_crx(const char*,int*,int,int,CRXING*);
IMPORT  void    print_wall_curve_crx0(const char*,FT_POINT *, int,CRXING*);

/*	iprt3d.c*/
IMPORT	void	print_c_bond(C_BOND*,INTERFACE*);
IMPORT	void	print_c_curve(C_CURVE*,INTERFACE*);
IMPORT	void	print_c_surf(C_SURF*,INTERFACE*);
IMPORT	void	print_c_surf_flag(C_SURF_FLAG*);


/*	isect3d.c*/
IMPORT	C_BOND	*i_CBond(C_BOND*,FT_POINT*,FT_POINT*,TRI*,TRI*);
IMPORT	void	i_print_intersections3d(CROSS*,INTERFACE*);

/*	map.c*/
IMPORT  int     NumOfInteriorPoints(INTERFACE*);

IMPORT  void    ArrayOfSurfaces(INTERFACE*,SURFACE**);
IMPORT  void    ArrayOfCurvePoints(CURVE*,double*);
IMPORT  void    ArrayOfIntfcPoints(INTERFACE*intfc,double*);
IMPORT  void    ArrayOfSurfTris_FT(SURFACE*,TRI**);
IMPORT  void    ArrayOfSurfTris(SURFACE*,double*,int*);
IMPORT  void    ArrayOfIntfcTris_FT(INTERFACE*,TRI**);
IMPORT  void    ArrayOfIntfcTris(INTERFACE*,double*,int*);
IMPORT	int	GridSegCrossing(CRXING**,int*,GRID_DIRECTION,INTERFACE*);
IMPORT	COMPONENT *GridIntfcComp(INTERFACE*);
IMPORT	boolean	IntfcGetPointChain(FT_POINT*,FT_POINT**,int);

/*	setb3d.c*/
IMPORT	boolean	i_set_boundary3d(INTERFACE*,RECT_GRID*,COMPONENT,double);

/*	trisurf.c*/
IMPORT	void	oblique_planar_surface_triangulation(SURFACE*,RECT_GRID*); 
IMPORT	void	planar_hole_surface_triangulation(SURFACE*,RECT_GRID*,
						  FT_POINT*,FT_POINT*,FT_POINT*,FT_POINT*);
IMPORT	void	planar_surface_triangulation(SURFACE*,RECT_GRID*,const boolean);

/*	icheck3d.c*/
IMPORT	void	null_sides_are_consistent(void);
IMPORT	boolean	i_consistent_interface(INTERFACE*);
IMPORT	void	check_double_cone_point(INTERFACE*);

/*	userhooks.c */
IMPORT	BOND	*Bond(FT_POINT*,FT_POINT*);
IMPORT	CURVE	*attach_curve_to_node(CURVE*,FT_POINT*,BOND*,NODE*);
IMPORT	CURVE	*copy_curve(CURVE*,NODE*,NODE*);
IMPORT	CURVE	*make_curve(COMPONENT,COMPONENT,NODE*,NODE*);
IMPORT	CURVE	*read_curve(INTERFACE*,int);
IMPORT	HYPER_SURF *make_hypersurface(COMPONENT,COMPONENT);
IMPORT	HYPER_SURF_BDRY	*make_hypersurface_boundary(void);
IMPORT	INTERFACE	*copy_interface(INTERFACE*);
IMPORT	INTERFACE	*zoom_interface(INTERFACE*,RECT_GRID*,
					double*,double*,double**);
IMPORT	NODE	*copy_node(NODE*);
IMPORT	NODE	*make_node(FT_POINT*);
IMPORT	NODE	*read_node(INTERFACE*,int);
IMPORT	FT_POINT	*Point(double*);
IMPORT	FT_POINT	*Static_point(INTERFACE*);
IMPORT	FT_POINT	*average_points(boolean,FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*,
	                             FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*);
IMPORT	FT_POINT	*copy_point(FT_POINT*);
IMPORT	boolean	delete_end_of_bond(BOND*,CURVE*);
IMPORT	boolean	delete_node(NODE*);
IMPORT	boolean	delete_start_of_bond(BOND*,CURVE*);
IMPORT	boolean	insert_point_in_bond(FT_POINT*,BOND*,CURVE*);
IMPORT	boolean	intersections(INTERFACE*,CROSS**,const boolean);
IMPORT	boolean	is_subdomain_boundary(HYPER_SURF*);
IMPORT	boolean	is_subdomain_node(NODE*);
IMPORT	boolean	is_virtual_fixed_node(NODE*);
IMPORT	boolean	long_nearest_interface_point(double*,COMPONENT,INTERFACE*,
				USE_BOUNDARIES,HYPER_SURF*,double*,double*,
				HYPER_SURF_ELEMENT**,HYPER_SURF**);
IMPORT	boolean	long_nearest_similar_interface_point(double*,COMPONENT,
				COMPONENT,INTERFACE*,USE_BOUNDARIES,HYPER_SURF*,
				double*,double*,HYPER_SURF_ELEMENT**,
				HYPER_SURF**);
IMPORT	boolean	move_closed_loop_node(CURVE*,BOND*);
IMPORT	boolean	nearest_interface_point(double*,COMPONENT,INTERFACE*,
				USE_BOUNDARIES,HYPER_SURF*,double*,double*,
				HYPER_SURF_ELEMENT**,HYPER_SURF**);
IMPORT	boolean	nearest_interface_point_within_range(double*,COMPONENT,
				INTERFACE*,USE_BOUNDARIES,HYPER_SURF*,double*,
				double*,HYPER_SURF_ELEMENT**,HYPER_SURF**,int);
IMPORT	boolean	nearest_similar_interface_point(double*,COMPONENT,COMPONENT,
				INTERFACE*,USE_BOUNDARIES,HYPER_SURF*,double*,
				double*,HYPER_SURF_ELEMENT**,HYPER_SURF**);
IMPORT	boolean	make_interface_topology_lists(INTERFACE*);
IMPORT	boolean	next_point(INTERFACE*,FT_POINT**,HYPER_SURF_ELEMENT**,
			   	HYPER_SURF**);
IMPORT	boolean	next_hypersurface(INTERFACE*,HYPER_SURF**);
IMPORT	boolean	set_boundary(INTERFACE*,RECT_GRID*,COMPONENT,double);
IMPORT	boolean	user_join_curves(CURVE*,CURVE*,CURVE*);
IMPORT	boolean	user_split_curve(int,FT_POINT*,BOND*,CURVE*,CURVE**);
IMPORT	double	random01(INTERFACE*);
IMPORT	double	cross_tolerance(INTERFACE*);
IMPORT	int	delete_curve(CURVE*);
IMPORT	int	delete_interface(INTERFACE*);
IMPORT	int	print_number_of_tangles(const char*,INTERFACE*,CROSS*);
IMPORT	int	read_boundary_type_from_string(const char*,INTERFACE*);
IMPORT	int	read_print_intfc_rect_grids(const IO_TYPE*,INTERFACE*,REMAP*);
IMPORT	int	user_read_node(NODE*);
IMPORT	int	user_read_print_interface(INIT_DATA*,const IO_TYPE*,
                               	INTERFACE*,boolean);
IMPORT	void	fprint_boundary_type(FILE*,const char*,int,const char*,
			     	INTERFACE*);
IMPORT	void	fprint_curve(FILE*,CURVE*);
IMPORT	void	fprint_interface(FILE*,INTERFACE*);
IMPORT	void	fprint_intfc_rect_grids(FILE*,INTERFACE*);
IMPORT	void	fprint_node(FILE*,NODE*);
IMPORT	void	invert_curve(CURVE*);
IMPORT	void	invert_surface(SURFACE*);
IMPORT	void	reverse_curve(CURVE*);
IMPORT	void	print_curve(CURVE*);
IMPORT	void	print_interface(INTERFACE*);
IMPORT	void	print_node(NODE*);
IMPORT	void	print_intersections(CROSS*,INTERFACE*);
IMPORT	void	print_crossing_elements(CROSS*,INTERFACE*);
IMPORT	void	reflect_interface(INTERFACE*,double*,double*);
IMPORT	void	reflect_node(NODE*,double*,double*);
IMPORT	void	reflect_curve(CURVE*,double*,double*);
IMPORT	void	reflect_point(FT_POINT*,double*,double*,INTERFACE*);
IMPORT	void	fset_hyper_surf_color(FILE*,HYPER_SURF*);
IMPORT	void	user_copy_hyper_surf(HYPER_SURF*,HYPER_SURF*);
IMPORT	void	user_fprint_curve(FILE*,CURVE*);
IMPORT	void	user_fprint_interface(FILE*,INTERFACE*);
IMPORT	void	user_fprint_intfc_rect_grids(FILE*,INTERFACE*);
IMPORT	void	user_fprint_node(FILE*,NODE*);
IMPORT	void	user_make_interface(INTERFACE*);
IMPORT	void	user_read_curve(CURVE*);
IMPORT	boolean	user_read_print_curve(CURVE*,const IO_TYPE*,boolean);
IMPORT	void	user_read_print_intfc_rect_grids(const IO_TYPE*,INTERFACE*,
				boolean,REMAP*);
IMPORT	void	user_read_print_node(NODE*,const IO_TYPE*,boolean);
IMPORT	FT_POINT	*make_point(double*,COMPONENT,COMPONENT);
IMPORT	FT_POINT	*read_point(INTERFACE*,int);
IMPORT	FT_POINT	*read_print_point(INTERFACE*,const IO_TYPE*,boolean);
IMPORT	int	delete_point(FT_POINT*);
IMPORT	void	fprint_point(FILE*,FT_POINT*);
IMPORT	void	print_point(FT_POINT*);
IMPORT	void	user_fprint_point(FILE*,FT_POINT*);
IMPORT	void	user_read_print_point(FT_POINT*,const IO_TYPE*,boolean);
IMPORT	void	user_read_point(INTERFACE*,FT_POINT*);
IMPORT	INTERFACE	*receive_interface(int);
IMPORT	void	reconstruct_bond_pointers(BOND*,INTERFACE*,INTERFACE*,
					  POINTER*,POINTER*,int);
IMPORT	void	reconstruct_curve_pointers(CURVE*,INTERFACE*,INTERFACE*,
					   POINTER*,POINTER*,int);
IMPORT	void	reconstruct_interface_pointers(INTERFACE*,struct Table*,
					       POINTER*,POINTER*);
IMPORT	void	reconstruct_node_pointers(NODE*,INTERFACE*,INTERFACE*,
					  POINTER*,POINTER*,int);
IMPORT	void	reconstruct_point_pointers(FT_POINT*,INTERFACE*,INTERFACE*,
					   POINTER*,POINTER*,int);
IMPORT	void	send_interface(INTERFACE *,int);
IMPORT	BOND_TRI *link_tri_to_bond(BOND_TRI*,TRI*,SURFACE*,BOND*,CURVE*);
IMPORT	C_BOND	*CBond(C_BOND*,FT_POINT*,FT_POINT*,TRI*,TRI*);
IMPORT	SURFACE *copy_surface(SURFACE*,CURVE**,CURVE**,boolean);
IMPORT	SURFACE *join_surfaces(CURVE*);
IMPORT	SURFACE *make_surface(COMPONENT,COMPONENT,CURVE**,CURVE**);
IMPORT	SURFACE *read_surface(INTERFACE*,int);
IMPORT	TRI	*make_tri(FT_POINT*,FT_POINT*,FT_POINT*,POINTER,POINTER,POINTER,int);
IMPORT	boolean	consistent_interface(INTERFACE*);
IMPORT	boolean	insert_point_in_tri(FT_POINT*,TRI*,SURFACE*);
IMPORT	boolean	insert_point_in_tri_side(FT_POINT*,int,TRI*,SURFACE*);
IMPORT	boolean sort_bond_tris(INTERFACE*);
IMPORT	boolean	undo_insert_point_in_tri(FT_POINT*,TRI*,SURFACE*);
IMPORT	boolean	undo_insert_point_in_tri_side(FT_POINT*,int,TRI*,SURFACE*);
IMPORT	int	delete_surface(SURFACE*);
IMPORT	void 	reverse_bond(BOND*);
IMPORT	void 	reorder_curve_link_list(CURVE*);
IMPORT	void	assign_curve_boundary_flag(CURVE*);
IMPORT	void	gview_plot_interface(const char*,INTERFACE*);
IMPORT	void	gview_plot_color_interface(const char*,INTERFACE*,boolean);
IMPORT	void	fprint_surface(FILE*,SURFACE*);
IMPORT	void	print_surface(SURFACE*);
IMPORT	void	reconstruct_surface_pointers(SURFACE*,INTERFACE*,INTERFACE*,
					     POINTER*,POINTER*,int);
IMPORT	void	reconstruct_tri_pointers(TRI*,INTERFACE*,INTERFACE*,
					 POINTER*,POINTER*,int);
IMPORT	void	reflect_surface(SURFACE*,double*,double*);
IMPORT	void	user_fprint_surface(FILE*,SURFACE*);
IMPORT	void	user_install_faces(SURFACE*,int);
IMPORT	void	user_read_print_surface(SURFACE*,const IO_TYPE*,boolean);
IMPORT	void	user_read_surface(SURFACE*);
IMPORT  void    check_print_intfc(const char*,const char*,char,INTERFACE*,
				int,int,boolean);

/*      iredist.c */
IMPORT	void  	equi_redist_curve_seg(CURVE*,BOND*,BOND*,int,double,double,
					RECT_GRID*);
IMPORT	void	rect_bdry_curve_redist(CURVE*,ORIENTATION,RECT_GRID*,double*);
IMPORT  void    rect_bdry_redist2d(INTERFACE*,RECT_GRID*,int);
IMPORT  void  	replace_curve_seg_by_bond(CURVE*,BOND*,BOND*);
IMPORT 	boolean 	i_delete_point_adjacent_to_node(CURVE*,ORIENTATION);
IMPORT  boolean  	closed_curve_node_redistribute(INTERFACE*,boolean);
IMPORT	boolean	redistribute_surface(SURFACE*,RECT_GRID*,TRI_REDIST_PARAMS);
IMPORT	boolean	surface_needs_redist(SURFACE*,RECT_GRID*,TRI_REDIST_PARAMS);

/*      iredist_o2.c */
IMPORT	void  	equi_redist_curve_seg_o2(CURVE*,BOND*,BOND*,int,double,double,
					RECT_GRID*);
IMPORT	boolean  	redistribute_surface_o2(SURFACE*,RECT_GRID*,TRI_REDIST_PARAMS);


/*      imkcurve.c */
IMPORT	CURVE	*make_elliptic_curve(ELLIP_PARAMS*,COMPONENT,COMPONENT,double);
IMPORT	void	coords_on_ellips(double*,double*,ELLIP_PARAMS*);
IMPORT  double 	multi_sine_mode_func(POINTER,double*);
IMPORT	void	prompt_make_level_curves(INTERFACE*,RECT_GRID*,COMPONENT*,
				COMPONENT*);
IMPORT  CURVE 	*prompt_make_linear_curve(INTERFACE*,RECT_GRID*);
IMPORT  CURVE 	*prompt_make_elliptic_curve(INTERFACE*,RECT_GRID*);
IMPORT  CURVE	**make_level_curves(RECT_GRID*,INTERFACE*,COMPONENT,COMPONENT,
			double (*func)(POINTER,double*),POINTER,boolean,int*);
IMPORT	CURVE	*make_array_curve(INTERFACE*,COMPONENT,COMPONENT,int,
			double**,boolean);
IMPORT	POINTER init_ellipse_params(RECT_GRID*);
IMPORT	double 	ellipse_func(POINTER,double*);
IMPORT	POINTER init_ellipse_tilt_params(RECT_GRID*);
IMPORT	double 	ellipse_tilt_func(POINTER,double*);
IMPORT	POINTER init_triangle_params(RECT_GRID*);
IMPORT	double 	triangle_func(POINTER,double*);
IMPORT	POINTER init_rectangle_params(RECT_GRID*);
IMPORT	double 	rectangle_func(POINTER,double*);
IMPORT  POINTER init_cosmos_params(RECT_GRID*);
IMPORT  double   cosmos_func(POINTER,double*);
IMPORT  POINTER init_taegeuk_params(RECT_GRID*);
IMPORT  double   taegeuk_func(POINTER,double*);
IMPORT  POINTER init_wing_params(RECT_GRID*);
IMPORT  double   wing_func(POINTER,double*);
IMPORT  POINTER init_propeller_params(RECT_GRID*);
IMPORT  double   propeller_func(POINTER,double*);
IMPORT  double   multi_circle_func(POINTER,double*);
IMPORT	double 	level_wave_func(POINTER,double*);
IMPORT	double 	level_circle_func(POINTER,double*);
IMPORT	double 	slotted_disk_func(POINTER,double*);
IMPORT	double 	slotted_disk_func(POINTER,double*);
IMPORT	double 	projectile_func(POINTER,double*);


/*      imksurf.c */
IMPORT  boolean 	make_bdry_surfaces(INTERFACE*,RECT_GRID*);
IMPORT	boolean    make_level_surface(RECT_GRID*,INTERFACE*,COMPONENT,COMPONENT,
                        double (*func)(POINTER,double*),POINTER,
                        SURFACE**);
IMPORT  boolean    make_comp3_surfaces(RECT_GRID*,COMPONENT,COMPONENT,COMPONENT,
			double (*func1)(POINTER,double*),POINTER,
                        double (*func2)(POINTER,double*),POINTER,
                        SURFACE***,CURVE**);
IMPORT  boolean    grid_line_crx_in_dir(double (*func)(POINTER,double*),
                        POINTER,int,double*,double*,double*,int);
IMPORT  boolean    read_sdl_surface(INTERFACE*,COMPONENT,COMPONENT,char*,
			SURFACE**);
IMPORT  boolean    read_vtk_surface(INTERFACE*,COMPONENT,COMPONENT,char*,
			SURFACE**);
IMPORT	double	dumbbell_func(POINTER,double*);
IMPORT	double	ellipsoid_func(POINTER,double*);
IMPORT	double	plane_func(POINTER,double*);
IMPORT	double	bdry_box_func(POINTER,double*);
IMPORT  SURFACE *prompt_make_level_surface(INTERFACE*,RECT_GRID*);
IMPORT  void 	prompt_make_bdry_surfaces(INTERFACE*,RECT_GRID*);
IMPORT  void    print_blk_crx(const BLK_CRX*);
IMPORT  void    reset_domain_comp(COMPONENT***,RECT_GRID);
IMPORT  void    assign_positive_comp(double (*func)(POINTER,double*),POINTER,
                        COMPONENT***,RECT_GRID,COMPONENT);
IMPORT  void    assign_negative_comp(double (*func)(POINTER,double*),POINTER,
                        COMPONENT***,RECT_GRID,COMPONENT);
IMPORT  void    assign_intersection_comp(double (*func_1)(POINTER,double*),
                        POINTER,double (*func_2)(POINTER,double*),POINTER,
                        COMPONENT***,RECT_GRID,COMPONENT,SIDE,SIDE);
IMPORT  void    make_grid_surfaces(BLK_CRX*,EG_CRX*,int*,boolean);
IMPORT  void    alloc_grid_crx_mem(EG_CRX*,int*,int,boolean); 
IMPORT  void    free_grid_crx_mem(EG_CRX*,boolean);
IMPORT  boolean    onfront_block(int,int,int,const EG_CRX*);
IMPORT  boolean    is_curve_crx(COMPONENT,COMPONENT,COMPONENT,COMPONENT);
IMPORT  int     install_grid_crx(double (*func)(POINTER,double*),POINTER,
                        EG_CRX*,RECT_GRID,COMPONENT,COMPONENT);
IMPORT  int     count_crx_through_comp(int*,COMPONENT***);
IMPORT  int     make_curves_from_blk(CURVE**,int*,int*,BLK_TRI ****,CURVE **,int);
/* make 3 comp surfaces from comp functions */
IMPORT	void    show_comp(COMPONENT ***,RECT_GRID);
IMPORT  boolean    make_surfaces_from_comp(RECT_GRID*,int (*func)(POINTER, double*),POINTER,
		SURFACE**,CURVE**,int*,int*);

/*    iwallsurf.c */
IMPORT  void  set_is_wall_surface(SURFACE *);
IMPORT  void  set_is_not_wall_surface(SURFACE *);
IMPORT  boolean  is_wall_surface(SURFACE *);
IMPORT  void  get_default_fluid_comp(int*, int*, INTERFACE*);
IMPORT  void  set_wall_flag_for_surface(INTERFACE *);
IMPORT  void  reset_wall_flag_for_surface(INTERFACE *);
IMPORT  void  get_default_fluid_comp(int*, int*, INTERFACE*);
IMPORT  int   add_to_o_surfaces(O_SURFACE **, int*, SURFACE *sp[4]);
IMPORT  void    set_grid_for_surface_construction(RECT_GRID*,RECT_GRID*);

/*	itrisset.c */
IMPORT	void	set_tol_for_tri_sect(double);
IMPORT	void	swap_positions(double*,double*,int);

IMPORT	boolean 	tri_recorded(TRI*,TRI**,int);
IMPORT  boolean    two_tris_share_pts(TRI*,TRI*,int);
IMPORT  boolean    two_tris_share_side(TRI*,TRI*,int);
IMPORT	boolean    point_in_crx_tri(double*,TRI*);
IMPORT	int	merge_tris_set(TRI**,int,TRI**,int);
IMPORT	int	set_tris_set_in_box(TRI**,int,int*,int*,INTERFACE*);
IMPORT	void	move_bound_inside_grid(int*,RECT_GRID*,int);

IMPORT	void	tri_bound_block(double**,TRI*);
IMPORT	void	tris_bound_box(double**,TRI**,int);
IMPORT	boolean	blocks_sect(double**,double**);
IMPORT	boolean	tris_sect(TRI*,TRI*);

IMPORT	void	plane_of_tri(double*,TRI*);
IMPORT	boolean 	plane_side_intersection(const double*,TRI*,int,double*,int*);
IMPORT	boolean	test_tris_intersection(TRI*,TRI*);
IMPORT	int	tris_intersection(TRI**,TRI**, int);
IMPORT	boolean	tangled_tris_bound_box(int*,int*,int*,int*,INTERFACE*);

IMPORT	boolean	link_neighbor_null_side_tris(TRI*,TRI*);
IMPORT	int	linking_tris_with_pairs(TRI**,int,TRI**,int,TRI**,int,
				TRI**,int);
IMPORT  void    centroid_of_tri(double*,TRI*);
IMPORT  boolean  	skip_bdry_tri(TRI*);
IMPORT  void    sort_tris_set(TRI**,int,POINTER);
IMPORT  int     bound_tris_set(TRI**,TRI**,int);
IMPORT  int     count_tris_in_box(int*,int*,INTERFACE*);
IMPORT  int     count_tris_in_top_box(int*,int*,INTERFACE*);
IMPORT  int     tris_set_in_top_box(TRI**, int, int*, int*, INTERFACE*);
IMPORT  boolean 	tri_in_grid_block(TRI*,int*,int*,RECT_GRID*);
IMPORT  int     rect_boxes_from_tangled_tris(RECT_BOX*,INTERFACE*);
IMPORT  int     remove_single_tri_null_loop(TRI**,int,boolean);
IMPORT  int     seal_all_loops_wo_constraint(TRI**,int*,TRI**,int,int,boolean);
IMPORT  boolean    check_valid_tris(TRI**,int,INTERFACE*);
IMPORT  boolean    check_valid_intfc(const char*, INTERFACE*);
IMPORT	int     sep_common_edge_from_tris(TRI***,TRI**,int,INTERFACE*);
IMPORT	boolean    sep_common_point_from_loop(TRI **,int,TRI**,int*,INTERFACE*);
IMPORT	void	set_comm_pt_fac(double);
IMPORT	double	get_comm_pt_fac();
IMPORT	void    compute_point_smooth(SMOOTH_PARA*,SMOOTH_TOL*,INTERFACE*);

/*      iblkc2.c */
IMPORT  BLK_CRX *alloc_blk_crx(boolean);
IMPORT  int     construct_comp2_blk(BLK_CRX*,BLK_TRI*);
IMPORT  void    stitch_inside_blk(BLK_TRI*);
IMPORT  void    stitch_adj_blk(BLK_TRI*,BLK_TRI*);
IMPORT  void    remove_null_pair(BLK_TRI*,BLK_TRI*,int);
IMPORT  void    create_triangle(BLK_TRI*,FT_POINT*,FT_POINT*,FT_POINT*,SURFACE*);
IMPORT  void    reorder_curve_link_list(CURVE*);
IMPORT	ORIENTATION curve_surface_orientation(SURFACE*,CURVE*);
IMPORT  void    set_debug_name(const char*);

/*      iblkc3.c */
IMPORT  int     is_surface(BLK_CRX*,SURFACE*);
IMPORT  int     construct_comp3_blk(BLK_CRX*,BLK_TRI*);

/*	ixgraph.c */
EXPORT  void 	xgraph_2d_intfc(const char*,INTERFACE*);
EXPORT  void 	xgraph_2d_intfc_within_range(const char*,INTERFACE*,double*,
				  double,boolean);
EXPORT  void 	xgraph_2d_reflection(const char*,INTERFACE*,double*,double*,
				  double*,double*);
IMPORT  FILE 	*xgraph_file_open(const char*,const char*,
				  const COORDINATE_PLANE);
IMPORT  void 	xgraph_RECT_GRID(const char*,RECT_GRID*);
IMPORT  void 	xgraph_affine_vector(FILE*,double*,double*,
				     const COORDINATE_PLANE,const char*);
IMPORT  void 	xgraph_curve(FILE*,CURVE*,const COORDINATE_PLANE);
IMPORT  void 	xgraph_interface_curves(const char*,const char*,INTERFACE*,
					const COORDINATE_PLANE);
IMPORT  void  	xgraph_interface_nodes(const char*,const char*,INTERFACE*,
				       const COORDINATE_PLANE);
IMPORT  void 	xgraph_interface_surfaces(const char*,const char*,INTERFACE*,
					  const COORDINATE_PLANE);
IMPORT  void 	xgraph_line_segment(FILE*,double*,double*,
				    const COORDINATE_PLANE,const char*);
IMPORT  void 	xgraph_new_data_set(FILE*);
IMPORT  void 	xgraph_point(FILE*,double*,const COORDINATE_PLANE,const char*);
IMPORT  void 	xgraph_tri(FILE*,TRI*,const COORDINATE_PLANE);
IMPORT  void 	xgraph_tris_list(FILE*,TRI**,int,const COORDINATE_PLANE);
IMPORT  boolean    point_within_range(FT_POINT*,double*,double,int);

#if defined(USE_OVERTURE)
/*   intfc_amr.c  */

IMPORT void set_amr_intfc_tol(INTERFACE*,double); 
#endif /* if defined(USE_OVERTURE) */

#ifdef IMESH
/*   iMesh.c auxilary */
IMPORT iBase_EntityHandle entityOfPoint(FT_POINT *p);
IMPORT iBase_EntityHandle entityOfBond(BOND *b);
IMPORT iBase_EntityHandle entityOfTri(TRI *t);
#endif /*def IMESH */

#if defined(c_plusplus) || defined(__cplusplus)
}
#endif


#endif /* !defined(_IPROTOS_H) */
