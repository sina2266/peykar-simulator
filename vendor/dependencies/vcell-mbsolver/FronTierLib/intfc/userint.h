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
*				userint.h:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*			User Supplied Operations
*/

#if !defined(_USERINT_H)
#define _USERINT_H

#include <intfc/int.h>

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif

struct _I_USER_INTERFACE {

	/* Sizes of interface data structures */
	size_t	size_interface;
	size_t	size_point;
	size_t	size_bond;
	size_t	size_curve;
	size_t	size_node;
	size_t	size_bond_tri;
	size_t	size_tri;
	size_t	size_surface;
	size_t	size_hyper_surf;
	size_t	size_hyper_surf_element;
	size_t	size_hyper_surf_bdry;
	size_t	size_o_node;

	/* Variables for interface store*/
	size_t	_ChunkSize;

	/* Dimensionally dependent functions for topology indentification */

 	/* nearest_interface_point() */
	boolean	(*_nip)(double*,COMPONENT,INTERFACE*,USE_BOUNDARIES,HYPER_SURF*,
			double*,double*,HYPER_SURF_ELEMENT**,HYPER_SURF**);

	/* long_nearest_interface_point() */
	boolean	(*_lnip)(double*,COMPONENT,INTERFACE*,USE_BOUNDARIES,HYPER_SURF*,
			 double*,double*,HYPER_SURF_ELEMENT**,HYPER_SURF**);

	/* nearest_similar_interface_point() */
	boolean	(*_nsip)(double*,COMPONENT,COMPONENT,INTERFACE*,USE_BOUNDARIES,
			 HYPER_SURF*,double*,double*,HYPER_SURF_ELEMENT**,
			 HYPER_SURF**);

	/* long_nearest_similar_interface_point() */
	boolean	(*_lnsip)(double*,COMPONENT,COMPONENT,struct _INTERFACE*,
			  USE_BOUNDARIES,HYPER_SURF*,double*,double*,
			  HYPER_SURF_ELEMENT**,HYPER_SURF**);

	/* Dimensionally dependent functions for interface element looping */

	boolean	(*_next_point)(struct _INTERFACE*,FT_POINT**,
			       HYPER_SURF_ELEMENT**,HYPER_SURF**);
	boolean	(*_next_hypersurface)(struct _INTERFACE*,HYPER_SURF**);

	/* Functions used for I/O of boundary type data */

	int	(*_read_boundary_type_from_string)(const char*);
	void	(*_fprint_boundary_type)(FILE*,const char*,int,const char*,
	                                 struct _INTERFACE*);

	/* User defined hooks to interface utility functions */

	/* Interface utility functions */
	void	(*_user_make_interface)(INTERFACE*);
	INTERFACE	*(*_copy_interface)(INTERFACE*);
	int	(*_user_read_print_interface)(INIT_DATA*,const IO_TYPE*,
	                                      INTERFACE*,boolean);
	void	(*_fprint_interface)(FILE*,INTERFACE*);
	void	(*_gview_plot_interface)(const char*,INTERFACE*);
	void	(*_user_fprint_interface)(FILE*,INTERFACE*);
	int	(*_delete_interface)(INTERFACE*);

	/* Hypersurface utility functions */
	HYPER_SURF	*(*_make_hypersurface)(COMPONENT,COMPONENT);
	void		(*_user_copy_hyper_surf)(HYPER_SURF*,HYPER_SURF*);

	/* Hypersurface boundary utility functions */
	HYPER_SURF_BDRY	*(*_make_hypersurface_boundary)(void);

	/* Node utility functions */
	NODE	*(*_make_node)(FT_POINT*);
	NODE	*(*_copy_node)(NODE*);
	boolean	(*_delete_node)(NODE*);
	void	(*_fprint_node)(FILE*,NODE*);
	void	(*_user_fprint_node)(FILE*,NODE*);
	NODE	*(*_read_node)(INTERFACE*,int);
	int	(*_user_read_node)(NODE*);
	void	(*_user_read_print_node)(NODE*,const IO_TYPE*,boolean);

	/* Rect grid utility functions */
		/*Writing*/
	void	(*_fprint_intfc_rect_grids)(FILE*,INTERFACE*);
	void	(*_user_fprint_intfc_rect_grids)(FILE*,INTERFACE*);
		/*Reading*/
	int	(*_read_print_intfc_rect_grids)(const IO_TYPE*,INTERFACE*,
	                                        REMAP*);
	void	(*_user_read_print_intfc_rect_grids)(const IO_TYPE*,INTERFACE*,
						     boolean,REMAP*);

	/* Curve utility functions */
	CURVE	*(*_make_curve)(COMPONENT,COMPONENT,NODE*,NODE*);
	CURVE	*(*_copy_curve)(CURVE*,NODE*,NODE*);
	int	(*_delete_curve)(CURVE*);
	void	(*_fprint_curve)(FILE*,CURVE*);
	void	(*_user_fprint_curve)(FILE*,CURVE*);
	CURVE	*(*_read_curve)(INTERFACE*,int);
	void	(*_user_read_curve)(CURVE*);
	boolean	(*_user_read_print_curve)(CURVE*,const IO_TYPE*,boolean);
	boolean	(*_user_split_curve)(int,FT_POINT*,BOND*,CURVE*,CURVE**);
	boolean	(*_user_join_curves)(CURVE*,CURVE*,CURVE*);

	/* Bond utilities */
	BOND	*(*_Bond)(FT_POINT*,FT_POINT*);
	BOND_TRI *(*_link_tri_to_bond)(BOND_TRI*,TRI*,SURFACE*,BOND*,CURVE*);
	void 	(*_reverse_bond)(BOND*);
	void 	(*_reorder_curve_link_list)(CURVE*);

	/* Point utility functions */
	FT_POINT	*(*_Point)(double*);
	FT_POINT	*(*_Static_point)(INTERFACE*);
	FT_POINT	*(*_copy_point)(FT_POINT*);
	FT_POINT   *(*_average_points)(boolean,FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*,
		                         FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*);
	FT_POINT	*(*_make_point)(double*,COMPONENT,COMPONENT);
	int	(*_delete_point)(FT_POINT*);
	void	(*_fprint_point)(FILE*,FT_POINT*);
	void	(*_user_fprint_point)(FILE*,FT_POINT*);
	FT_POINT	*(*_read_point)(INTERFACE*,int);
	void	(*_user_read_point)(INTERFACE*,FT_POINT*);
	FT_POINT	*(*_read_print_point)(INTERFACE*,const IO_TYPE*,boolean);
	void	(*_user_read_print_point)(FT_POINT*,const IO_TYPE*,boolean);
	boolean	(*_insert_point_in_bond)(FT_POINT*,BOND*,CURVE*);
	boolean	(*_delete_start_of_bond)(BOND*,CURVE*);
	boolean	(*_delete_end_of_bond)(BOND*,CURVE*);
	SURFACE *(*_join_surfaces)(CURVE*);
	boolean	(*_insert_point_in_tri)(FT_POINT*,TRI*,SURFACE*);
	boolean	(*_insert_point_in_tri_side)(FT_POINT*,int,TRI*,SURFACE*);
	boolean	(*_undo_insert_point_in_tri)(FT_POINT*,TRI*,SURFACE*);
	boolean	(*_undo_insert_point_in_tri_side)(FT_POINT*,int,TRI*,SURFACE*);

	/* Tri utility functions */
	TRI *(*_make_tri)(FT_POINT*,FT_POINT*,FT_POINT*,
			  POINTER,POINTER,POINTER,int);

	/* Surface utility functions */
	SURFACE	*(*_make_surface)(COMPONENT,COMPONENT,CURVE**,CURVE**);
	SURFACE	*(*_copy_surface)(SURFACE*,CURVE**,CURVE**,boolean);
	int	(*_delete_surface)(SURFACE*);
	void	(*_fprint_surface)(FILE*,SURFACE*);
	void	(*_user_fprint_surface)(FILE*,SURFACE*);
	SURFACE	*(*_read_surface)(INTERFACE*,int);
	void	(*_user_read_surface)(SURFACE*);
	void	(*_user_read_print_surface)(SURFACE*,const IO_TYPE*,boolean);

	/* C_BOND utility functions */
	C_BOND *(*_CBond)(C_BOND*,FT_POINT*,FT_POINT*,TRI*,TRI*);
	boolean	(*_sort_bond_tris)(INTERFACE*);
        /*#bjet2 */
	boolean    (*_assign_btri_states)(BOND_TRI*,BOND_TRI*);

	/* Parallel communication utilities */
	void	(*_send_interface)(INTERFACE*,int);
	INTERFACE	*(*_receive_interface)(int);
	void	(*_reconstruct_interface_pointers)(INTERFACE*,struct Table*,
						   POINTER*,POINTER*);
	void	(*_reconstruct_point_pointers)(FT_POINT*,INTERFACE*,INTERFACE*,
					       POINTER*,POINTER*,int);
	void	(*_reconstruct_node_pointers)(NODE*,INTERFACE*,INTERFACE*,
					      POINTER*,POINTER*,int);
	void	(*_reconstruct_bond_pointers)(BOND*,INTERFACE*,INTERFACE*,
					      POINTER*,POINTER*,int);
	void	(*_reconstruct_curve_pointers)(CURVE*,INTERFACE*,INTERFACE*,
					       POINTER*,POINTER*,int);
	void	(*_reconstruct_surface_pointers)(SURFACE*,INTERFACE*,
						 INTERFACE*,POINTER*,
						 POINTER*,int);
	void	(*_reconstruct_tri_pointers)(TRI*,INTERFACE*,INTERFACE*,
					     POINTER*,POINTER*,int);

	boolean	(*_set_boundary)(INTERFACE*,RECT_GRID*,COMPONENT,double);
	void	(*_user_install_faces)(SURFACE*,int);
	void	(*_assign_curve_boundary_flag)(CURVE*);
	void	(*_assign_curve_boundary_type)(CURVE*,int,int*);
	
	/*#bjet2 */
	void    (*_check_print_intfc)(const char*,const char*,char,INTERFACE*,
				int,int,boolean);
	SURFACE *(*_detach_one_surface)(SURFACE *); 
	void    (*_print_wall_crx)(const char*,int*,int,int,CRXING*);
	void    (*_print_wall_curve_crx)(const char*,int*,int,int,CRXING*);
	void    (*_print_wall_curve_crx0)(const char*,FT_POINT *, int,CRXING*);


	boolean	(*_intersections)(INTERFACE*,CROSS**,const boolean);
	double   (*_cross_tolerance)(INTERFACE*);
	void	(*_print_intersections)(CROSS*,INTERFACE*);
	int	(*_print_number_of_tangles)(const char*,INTERFACE*,CROSS*);
	void	(*_print_crossing_elements)(CROSS*,INTERFACE*);

	CURVE	*(*_attach_curve_to_node)(CURVE*,FT_POINT*,BOND*,NODE*);
	void	(*_invert_curve)(CURVE*);
	void	(*_invert_surface)(SURFACE*);
	void	(*_reverse_curve)(CURVE*);
	boolean	(*_move_closed_loop_node)(CURVE*,BOND*);
	boolean	(*_is_subdomain_boundary)(HYPER_SURF*);
	boolean	(*_is_subdomain_node)(NODE*);
	boolean	(*_is_virtual_fixed_node)(NODE*);
	void	(*_fset_hyper_surf_color)(FILE*,HYPER_SURF*);
	INTERFACE	*(*_zoom_interface)(INTERFACE*,RECT_GRID*,
					    double*,double*,double**);
	void	(*_reflect_interface)(INTERFACE*,double*,double*);
	void	(*_reflect_surface)(SURFACE*,double*,double*);
	void	(*_reflect_curve)(CURVE*,double*,double*);
	void	(*_reflect_node)(NODE*,double*,double*);
	void	(*_reflect_point)(FT_POINT*,double*,double*,INTERFACE*);
	boolean	(*_make_interface_topology_lists)(INTERFACE*);

	double	(*_random01)(INTERFACE*);
	unsigned short int _random01_seed[3];

	COMP_LIST	_excluded_comps;

	/* Debugging utilities */
	boolean	(*_consistent_interface)(INTERFACE*);

	struct _I_INTERFACE_TOLERANCES {
		double	_Parallel;
		double   _Min_sin_sqr;
		double	_MinScaledSeparation;
		double	_MinScaledLength;
		double   _EndOfCurve;
		double   _StartOfCurve;
		double   _RcbMinScaledSep;
		double   _RobustFac;
		double   _RcbMacTol;
		double   _RcbcRobustFac;
		double   _ReflectTol;
		double   _TolFac;
		int	_ShortCurveNumPoints;
	} _InterfaceTolerances;
};
typedef struct _I_USER_INTERFACE I_USER_INTERFACE;
#if defined(__cplusplus)
typedef I_USER_INTERFACE::_I_INTERFACE_TOLERANCES I_INTERFACE_TOLERANCES;
#else /* defined(__cplusplus) */
typedef struct _I_INTERFACE_TOLERANCES I_INTERFACE_TOLERANCES;
#endif /* defined(__cplusplus) */

struct _I_INTERFACE {
	INTERFACE Intfc;
	I_USER_INTERFACE I_user_intfc;
};
typedef struct _I_INTERFACE I_INTERFACE;

#define	i_interface(intfc)	((I_INTERFACE*) (intfc))
#define	i_user_interface(intfc)	(i_interface(intfc)->I_user_intfc)

#define	ChunkSize(intfc)	(i_user_interface(intfc)._ChunkSize)
#define	InterfaceTolerances(intfc)	(i_user_interface(intfc)._InterfaceTolerances)
#define PARALLEL(intfc)		InterfaceTolerances(intfc)._Parallel
#define MIN_SIN_SQR(intfc)	InterfaceTolerances(intfc)._Min_sin_sqr
#define MIN_SC_SEP(intfc)	InterfaceTolerances(intfc)._MinScaledSeparation
#define MIN_SCALED_LENGTH(intfc) InterfaceTolerances(intfc)._MinScaledLength
#define END_OF_CURVE(intfc)	InterfaceTolerances(intfc)._EndOfCurve
#define START_OF_CURVE(intfc)	InterfaceTolerances(intfc)._StartOfCurve
#define TOL_FAC(intfc)		InterfaceTolerances(intfc)._TolFac
#define RCB_MIN_SC_SEP(intfc)	InterfaceTolerances(intfc)._RcbMinScaledSep
#define ROBUST_FAC(intfc)	InterfaceTolerances(intfc)._RobustFac
#define RCB_MACH_TOL(intfc)	InterfaceTolerances(intfc)._RcbMacTol
#define RCBC_ROBUST_FAC(intfc)	InterfaceTolerances(intfc)._RcbcRobustFac
#define RTOL(intfc)		InterfaceTolerances(intfc)._ReflectTol
#define SHORT_CURVE_NUM_POINTS(intfc)					\
				InterfaceTolerances(intfc)._ShortCurveNumPoints
#define	excluded_comps(intfc)	(i_user_interface(intfc)._excluded_comps)
#define	Random01_seed(intfc)	i_user_interface(intfc)._random01_seed

enum _PRESERVE_USER_HOOKS {
	SAVE_HOOKS,
	RESTORE_HOOKS
};
typedef enum _PRESERVE_USER_HOOKS PRESERVE_USER_HOOKS;

#if defined(c_plusplus) || defined(__cplusplus)
}
#endif


#endif /* !defined(_USERINT_H) */
