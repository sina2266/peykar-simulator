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
*				ilocprotos.h:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*/

#if !defined(_ILOCPROTOS_H)
#define _ILOCPROTOS_H

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif

 /* libintfc.a EXPORTED Function Declarations Internal to Interface*/

/*	comp.c*/

/*	iecomps.c*/
IMPORT	POINTER	copy_e_comps(INTERFACE*);

/*	iblkb.c*/
IMPORT	int construct_bdry_blk(BLK_CRX*,BLK_TRI*);

/*	igrid.c*/

/*	intfc.c*/
IMPORT	CURVE	*i_read_curve(INTERFACE*,int);
IMPORT	FT_POINT	*i_read_print_point(INTERFACE*,const IO_TYPE*,boolean);
IMPORT	NODE	*i_read_node(INTERFACE*,int);
IMPORT	FT_POINT	*i_read_point(INTERFACE*,int);
IMPORT	boolean	next_hypersurface2d(INTERFACE*,HYPER_SURF**);
IMPORT  boolean next_point1d(INTERFACE*,FT_POINT**,
			  HYPER_SURF_ELEMENT**,HYPER_SURF**);
IMPORT  boolean next_point2d(INTERFACE*,FT_POINT**,
		       HYPER_SURF_ELEMENT**,HYPER_SURF**);
IMPORT	int	i_read_print_intfc_rect_grids(const IO_TYPE*,INTERFACE*,REMAP*);
IMPORT	int	winding_number(double*,double*,int);
IMPORT	struct	Chunk	*new_chunk(struct Table*);
IMPORT	void	i_fprint_boundary_type(FILE*,const char*,int,const char*,
                                       INTERFACE*);
IMPORT	void	i_fprint_curve(FILE*,CURVE*);
IMPORT	void	i_fprint_intfc_rect_grids(FILE*,INTERFACE*);
IMPORT	void	i_fprint_point(FILE*,FT_POINT*);
IMPORT	void	i_fprint_node(FILE*,NODE*);
IMPORT	void	print_table(struct Table*);
IMPORT	void	print_table_list(void);

/*	irefl.c*/
IMPORT	void	i_reflect_interface1d(INTERFACE*,double*,double*);
IMPORT	void	i_reflect_interface2d(INTERFACE*,double*,double*);
IMPORT	void	i_reflect_interface3d(INTERFACE*,double*,double*);

/*	iscatter.c*/

/*	isub.c*/
IMPORT	int	i_print_number_of_tangles(const char*,INTERFACE*,CROSS*);
IMPORT	void	set_tri_storage_type(TRI_STORAGE_TYPE type);

/*	iuserintfc.c*/

/*	ppcopy.c*/
IMPORT	void	i_reconstruct_surface_pointers(SURFACE*,INTERFACE*,INTERFACE*,
					       POINTER*,POINTER*,int);
IMPORT	void	i_reconstruct_tri_pointers(TRI*,INTERFACE*,INTERFACE*,
					   POINTER*,POINTER*,int);

/*	shift.c*/

/*	top.c*/
IMPORT	CURVE	*adjacent_curve_along_surface(CURVE*,ORIENTATION,
					      ANGLE_DIRECTION,
					      ORIENTATION*,double*,int);
IMPORT	boolean	i_is_subdomain_boundary(HYPER_SURF*);
IMPORT	boolean	i_is_subdomain_node(NODE*);
IMPORT	boolean	i_is_virtual_fixed_node(NODE*);
IMPORT	void	fprint_bond(FILE*,BOND*);
IMPORT	void	set_curves_at_onode(O_NODE*);

/*	zoom.c*/

/*	comp1d.c*/
IMPORT	COMPONENT	component1d(double*,INTERFACE*);
IMPORT	boolean	i_intersections1d(INTERFACE*,CROSS**,const boolean);
IMPORT	boolean	long_nearest_interface_point1d(double*,COMPONENT,INTERFACE*,
					       USE_BOUNDARIES,HYPER_SURF*,
					       double*,double*,
					       HYPER_SURF_ELEMENT**,
					       HYPER_SURF**);
IMPORT	boolean	nearest_interface_point1d(double*,COMPONENT,INTERFACE*,
					  USE_BOUNDARIES,HYPER_SURF*,double*,
					  double*,HYPER_SURF_ELEMENT**,
					  HYPER_SURF**);
IMPORT	void	i_print_intersections1d(CROSS*,INTERFACE*);
IMPORT void	show_COMP_1d(FILE*,INTERFACE*);
IMPORT	void	show_point_comp_lists(INTERFACE*);


/*	comp2d.c*/
IMPORT	COMPONENT	component2d(double*,INTERFACE*);
IMPORT	COMPONENT	long_component2d(double*,INTERFACE*);
IMPORT	boolean	long_nearest_interface_point2d(double*,COMPONENT,INTERFACE*,
					       USE_BOUNDARIES,HYPER_SURF*,
					       double*,double*,
					       HYPER_SURF_ELEMENT**,
					       HYPER_SURF**);
IMPORT	boolean	long_nearest_similar_interface_point2d(double*,COMPONENT,
				                       COMPONENT,INTERFACE*,
						       USE_BOUNDARIES,
						       HYPER_SURF*,double*,
						       double*,
						       HYPER_SURF_ELEMENT**,
						       HYPER_SURF**);
IMPORT	boolean	nearest_interface_point2d(double*,COMPONENT,INTERFACE*,
					  USE_BOUNDARIES,HYPER_SURF*,double*,
					  double*,HYPER_SURF_ELEMENT**,
					  HYPER_SURF**);
IMPORT	boolean	nearest_similar_interface_point2d(double*,COMPONENT,COMPONENT,
						  INTERFACE*,USE_BOUNDARIES,
						  HYPER_SURF*,double*,double*,
						  HYPER_SURF_ELEMENT**,
						  HYPER_SURF**);
IMPORT	boolean	nearest_interface_point2d(double*,COMPONENT,INTERFACE*,
					  USE_BOUNDARIES,HYPER_SURF*,double*,
					  double*,HYPER_SURF_ELEMENT**,
					  HYPER_SURF**);
IMPORT	void	show_COMP_2d(FILE*,INTERFACE*);
IMPORT	boolean	nearest_interface_point_within_range2d(double*,COMPONENT,
			INTERFACE*,USE_BOUNDARIES,HYPER_SURF*,double*,double*,
			HYPER_SURF_ELEMENT**,HYPER_SURF**,int);

/*	cross2d.c*/

/*	isect2d.c*/
IMPORT	boolean	i_intersections2d(INTERFACE*,CROSS**,const boolean);
IMPORT	void	i_print_crossing_elements2d(CROSS*,INTERFACE*);
IMPORT	void	i_print_intersections2d(CROSS*,INTERFACE*);

/*	setb2d.c*/


/*	comp3d.c*/
IMPORT	COMPONENT	component3d(double*,INTERFACE*);
IMPORT	COMPONENT	long_component3d(double*,INTERFACE*);
IMPORT	COMPONENT	dir_long_component3d(double*,INTERFACE*);
IMPORT	boolean	make_tri_lists(INTERFACE*);
IMPORT	boolean	long_nearest_interface_point3d(double*,COMPONENT,INTERFACE*,
			USE_BOUNDARIES,HYPER_SURF*,double*,double*,
			HYPER_SURF_ELEMENT**,HYPER_SURF**);
IMPORT	boolean	long_nearest_similar_interface_point3d(double*,COMPONENT,
		        COMPONENT,INTERFACE*,USE_BOUNDARIES,HYPER_SURF*,
			double*,double*,HYPER_SURF_ELEMENT**,HYPER_SURF**);
IMPORT	boolean	nearest_interface_point3d(double*,COMPONENT,INTERFACE*,
			USE_BOUNDARIES,HYPER_SURF*,double*,double*,
			HYPER_SURF_ELEMENT**,HYPER_SURF**);
IMPORT	boolean	nearest_interface_point_within_range3d(double*,COMPONENT,
			INTERFACE*,USE_BOUNDARIES,HYPER_SURF*,double*,double*,
			HYPER_SURF_ELEMENT**,HYPER_SURF**,int);
IMPORT	boolean	nearest_similar_interface_point3d(double*,COMPONENT,COMPONENT,
			INTERFACE*,USE_BOUNDARIES,HYPER_SURF*,double*,double*,
			HYPER_SURF_ELEMENT**,HYPER_SURF**);
IMPORT	void	show_COMP_3d(FILE*,INTERFACE*);

/*	int3d.c*/
IMPORT	SURFACE *read_print_surface(INTERFACE*,const IO_TYPE*,int,
                                    INTERFACE_ADDRESSES*,boolean);
IMPORT	boolean	next_hypersurface3d(INTERFACE*,HYPER_SURF**);
IMPORT	boolean	next_point3d(INTERFACE*,FT_POINT**,HYPER_SURF_ELEMENT**,
			     HYPER_SURF**);
IMPORT	void	copy_all_surfaces(INTERFACE*,INTERFACE*);
IMPORT	void	fprint_tri(FILE*,TRI*,INTERFACE*);
IMPORT	void	fprint_tris_on_curve(FILE*,CURVE*);
IMPORT	void	i_fprint_surface(FILE*,SURFACE*);
IMPORT	void	print_tris_on_surface(SURFACE*);
IMPORT	void	read_print_tris_on_curve(FILE*,CURVE*,int,
					 INTERFACE_ADDRESSES*);
IMPORT	void	set_tri_array_numbers(INTERFACE*,int);

/*	iprt3d.c*/
IMPORT	void	threed_interface_plot(INTERFACE*,RECT_GRID*);

/*	isect3d.c*/
IMPORT	boolean	i_intersections3d(INTERFACE*,CROSS**,const boolean);
IMPORT	void	i_print_crossing_elements3d(CROSS*,INTERFACE*);

/*	map.c*/
IMPORT	boolean	is_interior_vertex(TRI*,FT_POINT*,TRI**,INTERFACE*);

/*	setb3d.c*/


/*	icheck3d.c*/
IMPORT void	check_tri_and_bond(TRI*,BOND*,const char*,INTERFACE*);

/*	ineighborhood.c */

#if defined(c_plusplus) || defined(__cplusplus)
}
#endif

#endif /* !defined(_ILOCPROTOS_H) */
