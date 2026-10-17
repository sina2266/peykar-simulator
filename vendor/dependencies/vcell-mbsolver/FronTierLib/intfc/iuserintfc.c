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
*				iuserintfc.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*
*	User definable hooks to the interface library.
*
*	IMPORTANT NOTE: All of the i_user_... functions below are NO-OP
*	functions, i.e. they perform no operations.  These hooks are
*	simply default place holders for the interface hook functions.
*	The reason these functions are NO-OPs is that any operation they
*	perform would be meaningful at the intfc library level and so should
*	properly be included in the actual operation function rather than
*	the user hook function.
*/




#include <intfc/iloc.h>


	/* LOCAL Function Declarations */
LOCAL	boolean	i_is_excluded_comp(COMPONENT,COMP_LIST*,INTERFACE*);
LOCAL	double	i_cross_tolerance(INTERFACE*);
LOCAL	int	i_user_read_print_interface(INIT_DATA*,const IO_TYPE*,
                                            INTERFACE*,boolean);
LOCAL	void	i_fset_hyper_surf_color(FILE*,HYPER_SURF*);
LOCAL	void	i_user_fprint_interface(FILE*,INTERFACE*);
LOCAL	void	i_user_fprint_intfc_rect_grids(FILE*,INTERFACE*);
LOCAL	void	i_user_make_interface(INTERFACE*);
LOCAL	void	i_user_read_print_intfc_rect_grids(const IO_TYPE*,INTERFACE*,
						   boolean,REMAP*);
LOCAL	void	i_user_copy_hyper_surf(HYPER_SURF*,HYPER_SURF*);

LOCAL	boolean	i_user_join_curves(CURVE*,CURVE*,CURVE*);
LOCAL	boolean	i_user_split_curve(int,FT_POINT*,BOND*,CURVE*,CURVE**);
LOCAL	int	i_user_read_node(NODE*);
LOCAL	void	i_user_fprint_curve(FILE*,CURVE*);
LOCAL	void	i_user_fprint_node(FILE*,NODE*);
LOCAL	void	i_user_read_curve(CURVE*);
LOCAL	boolean	i_user_read_print_curve(CURVE*,const IO_TYPE*,boolean);
LOCAL	void	i_user_read_print_node(NODE*,const IO_TYPE*,boolean);

LOCAL	void	i_user_fprint_point(FILE*,FT_POINT*);
LOCAL	void	i_user_read_point(INTERFACE*,FT_POINT*);
LOCAL	void	i_user_read_print_point(FT_POINT*,const IO_TYPE*,boolean);

LOCAL	void	i_gview_plot_interface(const char*,INTERFACE*);
LOCAL	void	i_user_install_faces(SURFACE*,int);
LOCAL	void	i_assign_curve_boundary_flag(CURVE*);
LOCAL	void	i_user_read_print_surface(SURFACE*,const IO_TYPE*,boolean);
LOCAL	void	i_user_read_surface(SURFACE*);
LOCAL	void	i_user_fprint_surface(FILE*,SURFACE*);


EXPORT	I_USER_INTERFACE *i_user_hook(
	int		dim)
{
	int                     i;
	static boolean             first = YES;
	static I_USER_INTERFACE User_hooks[3];

	if (first == YES)
	{
	    static COMP_LIST Default_excluded_comps = { 0,
							0,
							NULL,
							i_is_excluded_comp,
							i_add_comp_to_list
						      };
	    first = NO;
	    for (i = 0; i < MAXD; ++i)
	    {
	        User_hooks[i].size_interface = sizeof(I_INTERFACE);
	        User_hooks[i].size_point = sizeof(FT_POINT);
	        User_hooks[i].size_hyper_surf = sizeof(HYPER_SURF);
	        User_hooks[i]._read_boundary_type_from_string =
		    i_read_boundary_type_from_string;
	        User_hooks[i]._fprint_boundary_type = i_fprint_boundary_type;
	        User_hooks[i]._user_make_interface = i_user_make_interface;
	        User_hooks[i]._copy_interface = i_copy_interface;
	        User_hooks[i]._user_copy_hyper_surf = i_user_copy_hyper_surf;
	        User_hooks[i]._user_read_print_interface =
		    i_user_read_print_interface;
	        User_hooks[i]._fprint_interface = i_fprint_interface;
	        User_hooks[i]._user_fprint_interface = i_user_fprint_interface;
	        User_hooks[i]._delete_interface = i_delete_interface;
	        User_hooks[i]._make_hypersurface = i_make_hypersurface;
	        User_hooks[i]._make_hypersurface_boundary =
		    i_make_hypersurface_boundary;
	        User_hooks[i]._fprint_intfc_rect_grids =
		    i_fprint_intfc_rect_grids;
	        User_hooks[i]._user_fprint_intfc_rect_grids =
		    i_user_fprint_intfc_rect_grids;
		User_hooks[i]._read_print_intfc_rect_grids =
		    i_read_print_intfc_rect_grids;
	        User_hooks[i]._user_read_print_intfc_rect_grids =
		    i_user_read_print_intfc_rect_grids;
		User_hooks[i]._Static_point = i_Static_point;
	        User_hooks[i]._average_points = i_average_points;
	        User_hooks[i]._copy_point = i_copy_point;
	        User_hooks[i]._send_interface = i_send_interface;
	        User_hooks[i]._receive_interface = i_receive_interface;
	        User_hooks[i]._reconstruct_interface_pointers = 
		    i_reconstruct_interface_pointers;
	        User_hooks[i]._reconstruct_point_pointers = 
		    i_reconstruct_point_pointers;
	        User_hooks[i]._print_number_of_tangles =
		    i_print_number_of_tangles;
	        User_hooks[i]._is_subdomain_boundary = i_is_subdomain_boundary;
	        User_hooks[i]._fset_hyper_surf_color = i_fset_hyper_surf_color;
	        User_hooks[i]._zoom_interface = i_zoom_interface;
	        User_hooks[i]._reflect_point = i_reflect_point;
	        User_hooks[i]._InterfaceTolerances._Parallel =
		    0.00000762939453125; /*PARALLEL 2^-17 */
	        User_hooks[i]._InterfaceTolerances._Min_sin_sqr = 1.0e-6;
		User_hooks[i]._InterfaceTolerances._MinScaledLength =
		    pow(2.0,-10.0);
		User_hooks[i]._InterfaceTolerances._MinScaledSeparation = 
		    pow(2.0,-7.0);
		User_hooks[i]._InterfaceTolerances._EndOfCurve =
		    1.0 - User_hooks[i]._InterfaceTolerances.
		    _MinScaledSeparation;
		User_hooks[i]._InterfaceTolerances._StartOfCurve =
		    User_hooks[i]._InterfaceTolerances._MinScaledSeparation;
		User_hooks[i]._InterfaceTolerances._TolFac = 32.0;
		User_hooks[i]._InterfaceTolerances._RcbMinScaledSep =
		    User_hooks[i]._InterfaceTolerances._MinScaledLength;
		User_hooks[i]._InterfaceTolerances._RobustFac = 0.01;
		User_hooks[i]._InterfaceTolerances._RcbMacTol = 1.0e-5;
		User_hooks[i]._InterfaceTolerances._RcbcRobustFac =
		    User_hooks[i]._InterfaceTolerances._MinScaledLength;
		User_hooks[i]._InterfaceTolerances._ReflectTol = 0.0001;
		User_hooks[i]._InterfaceTolerances._ShortCurveNumPoints = 3;
	        User_hooks[i]._cross_tolerance = i_cross_tolerance;
	        User_hooks[i]._excluded_comps = Default_excluded_comps;
		User_hooks[i]._random01_seed[0] = 0xab83;
		User_hooks[i]._random01_seed[1] = 0x0023;
		User_hooks[i]._random01_seed[2] = 0x3eaa;
		User_hooks[i]._random01 = i_random01;
	    }
	    User_hooks[0]._ChunkSize = 1250*sizeof(ALIGN);
	    User_hooks[0]._nip = nearest_interface_point1d;
	    User_hooks[0]._lnip = long_nearest_interface_point1d;
	    User_hooks[0]._next_point = next_point1d;
	    User_hooks[0]._next_hypersurface = next_hypersurface1d;
	    User_hooks[0]._make_point = i_make_point;
	    User_hooks[0]._Point = i_Point;
	    User_hooks[0]._delete_point = i_delete_point;
	    User_hooks[0]._fprint_point = i_fprint_point;
	    User_hooks[0]._user_fprint_point = i_user_fprint_point;
	    User_hooks[0]._read_point = i_read_point;
	    User_hooks[0]._user_read_point = i_user_read_point;
	    User_hooks[0]._read_print_point = i_read_print_point;
	    User_hooks[0]._user_read_print_point = i_user_read_print_point;
	    User_hooks[0]._set_boundary = i_set_boundary1d;
	    User_hooks[0]._intersections = i_intersections1d;
	    User_hooks[0]._print_intersections = i_print_intersections1d;
	    User_hooks[0]._reflect_interface = i_reflect_interface1d;
	    User_hooks[0]._make_interface_topology_lists =
		make_point_comp_lists;

	    for (i = 1; i < MAXD; ++i)
	    {
	        User_hooks[i].size_bond = sizeof(BOND);
	        User_hooks[i].size_curve = sizeof(CURVE);
	        User_hooks[i].size_node = sizeof(NODE);
	        User_hooks[i].size_hyper_surf_element =
		    sizeof(HYPER_SURF_ELEMENT);
	        User_hooks[i].size_hyper_surf_bdry = sizeof(HYPER_SURF_BDRY);
	        User_hooks[i].size_o_node = sizeof(O_NODE);
	        User_hooks[i]._make_node = i_make_node;
	        User_hooks[i]._copy_node = i_copy_node;
	        User_hooks[i]._delete_node = i_delete_node;
	        User_hooks[i]._fprint_node = i_fprint_node;
	        User_hooks[i]._user_fprint_node = i_user_fprint_node;
	        User_hooks[i]._read_node = i_read_node;
	        User_hooks[i]._user_read_node = i_user_read_node;
	        User_hooks[i]._user_read_print_node = i_user_read_print_node;
	        User_hooks[i]._make_curve = i_make_curve;
	        User_hooks[i]._copy_curve = i_copy_curve;
	        User_hooks[i]._delete_curve = i_delete_curve;
	        User_hooks[i]._fprint_curve = i_fprint_curve;
	        User_hooks[i]._user_fprint_curve = i_user_fprint_curve;
	        User_hooks[i]._read_curve = i_read_curve;
	        User_hooks[i]._user_read_curve = i_user_read_curve;
	        User_hooks[i]._user_read_print_curve = i_user_read_print_curve;
	        User_hooks[i]._user_split_curve = i_user_split_curve;
	        User_hooks[i]._user_join_curves = i_user_join_curves;
	        User_hooks[i]._Bond = i_Bond;
	        User_hooks[i]._Point = i_Point;
	        User_hooks[i]._insert_point_in_bond = i_insert_point_in_bond;
	        User_hooks[i]._delete_start_of_bond = i_delete_start_of_bond;
	        User_hooks[i]._delete_end_of_bond = i_delete_end_of_bond;
	        User_hooks[i]._reconstruct_node_pointers = 
		    i_reconstruct_node_pointers;
	        User_hooks[i]._reconstruct_bond_pointers = 
		    i_reconstruct_bond_pointers;
	        User_hooks[i]._reconstruct_curve_pointers = 
		    i_reconstruct_curve_pointers;
	        User_hooks[i]._attach_curve_to_node = i_attach_curve_to_node;
	        User_hooks[i]._invert_curve = i_invert_curve;
	        User_hooks[i]._reverse_curve = i_reverse_curve;
	        User_hooks[i]._move_closed_loop_node = i_move_closed_loop_node;
	        User_hooks[i]._is_subdomain_node = i_is_subdomain_node;
	        User_hooks[i]._reflect_curve = i_reflect_curve;
	        User_hooks[i]._reflect_node = i_reflect_node;
	    }

	    User_hooks[1]._ChunkSize = 12500*sizeof(ALIGN);
	    User_hooks[1]._nip = nearest_interface_point2d;
	    User_hooks[1]._lnip = long_nearest_interface_point2d;
	    User_hooks[1]._nsip = nearest_similar_interface_point2d;
	    User_hooks[1]._lnsip = long_nearest_similar_interface_point2d;
	    User_hooks[1]._next_point = next_point2d;
	    User_hooks[1]._next_hypersurface = next_hypersurface2d;
	    User_hooks[1]._set_boundary = i_set_boundary2d;
	    User_hooks[1]._intersections = i_intersections2d;
	    User_hooks[1]._print_intersections = i_print_intersections2d;
	    User_hooks[1]._print_crossing_elements =
		i_print_crossing_elements2d;
	    User_hooks[1]._is_virtual_fixed_node = i_is_virtual_fixed_node;
	    User_hooks[1]._reflect_interface = i_reflect_interface2d;
	    User_hooks[1]._make_interface_topology_lists = 
		make_bond_comp_lists;
	    User_hooks[1]._gview_plot_interface = i_gview_plot_interface;
	    User_hooks[1]._consistent_interface = i_consistent_interface;
	    User_hooks[2].size_bond_tri = sizeof(BOND_TRI);
	    User_hooks[2].size_tri = sizeof(TRI);
	    if (debugging("MinTStor"))
	        set_tri_storage_type(MIN_TRI_STORAGE);
	    else if (debugging("FullTGeo"))
	        set_tri_storage_type(FULL_TRI_GEOMETRY);
	    else
	        set_tri_storage_type(TRI_PLUS_NORMAL);
	    User_hooks[2].size_surface = sizeof(SURFACE);
	    User_hooks[2]._ChunkSize = 50000*sizeof(ALIGN);
	    User_hooks[2]._nip = nearest_interface_point3d;
	    User_hooks[2]._lnip = long_nearest_interface_point3d;
	    User_hooks[2]._nsip = nearest_similar_interface_point3d;
	    User_hooks[2]._lnsip = long_nearest_similar_interface_point3d;
	    User_hooks[2]._next_point = next_point3d;
	    User_hooks[2]._next_hypersurface = next_hypersurface3d;
	    User_hooks[2]._gview_plot_interface = i_gview_plot_interface;
	    User_hooks[2]._link_tri_to_bond = i_link_tri_to_bond;
	    User_hooks[2]._reverse_bond = i_reverse_bond;
	    User_hooks[2]._reorder_curve_link_list = i_reorder_curve_link_list;
	    User_hooks[2]._insert_point_in_tri = i_insert_point_in_tri;
	    User_hooks[2]._insert_point_in_tri_side = 
		i_insert_point_in_tri_side;
	    User_hooks[2]._undo_insert_point_in_tri =
		i_undo_insert_point_in_tri;
	    User_hooks[2]._undo_insert_point_in_tri_side = 
		i_undo_insert_point_in_tri_side;
	    User_hooks[2]._make_tri = i_make_tri;
	    User_hooks[2]._CBond = i_CBond;
	    User_hooks[2]._make_surface = i_make_surface;
	    User_hooks[2]._join_surfaces = i_join_surfaces;
	    User_hooks[2]._copy_surface = i_copy_surface;
	    User_hooks[2]._delete_surface = i_delete_surface;
	    User_hooks[2]._fprint_surface = i_fprint_surface;
	    User_hooks[2]._user_fprint_surface = i_user_fprint_surface;
	    User_hooks[2]._read_surface = i_read_surface;
	    User_hooks[2]._user_read_surface = i_user_read_surface;
	    User_hooks[2]._user_read_print_surface = i_user_read_print_surface;
	    User_hooks[2]._reconstruct_surface_pointers = 
		i_reconstruct_surface_pointers;
	    User_hooks[2]._reconstruct_tri_pointers = 
		i_reconstruct_tri_pointers;
	    User_hooks[2]._set_boundary = i_set_boundary3d;
	    User_hooks[2]._user_install_faces = i_user_install_faces;
	    User_hooks[2]._assign_curve_boundary_flag = 
		i_assign_curve_boundary_flag;
	    User_hooks[2]._intersections = i_intersections3d;
	    User_hooks[2]._print_intersections = i_print_intersections3d;
	    User_hooks[2]._print_crossing_elements =
		i_print_crossing_elements3d;
	    User_hooks[2]._is_virtual_fixed_node = i_is_virtual_fixed_node;
	    User_hooks[2]._reflect_interface = i_reflect_interface3d;
	    User_hooks[2]._reflect_surface = i_reflect_surface;
	    User_hooks[2]._make_interface_topology_lists = make_tri_comp_lists;
	    User_hooks[2]._consistent_interface = i_consistent_interface;
	    User_hooks[2]._sort_bond_tris = i_sort_bond_tris;
	}
	if (dim < 1 || dim > 3)
	{
	    screen("ERROR in i_user_hook(), invalid dim %d\n",dim);
	    clean_up(ERROR);
	    return NULL;
	}
	else
	    return User_hooks + dim - 1;
}		/*end i_user_hook*/

EXPORT	void	i_preserve_user_hooks(
	int                     dim,
	PRESERVE_USER_HOOKS	flag)
{
	I_USER_INTERFACE        *iuh;
	static I_USER_INTERFACE Sav_iuh;
	static I_USER_INTERFACE *sav_iuh = NULL;

	switch (flag)
	{
	case SAVE_HOOKS:
	    if (sav_iuh != NULL)
	    {
		screen("ERROR in i_preserve_user_hooks(), "
		       "attempt to save without prior restore\n");
		clean_up(ERROR);
	    }
	    iuh = i_user_hook(dim);
	    sav_iuh = &Sav_iuh;
	    *sav_iuh = *iuh;
	    break;
	case RESTORE_HOOKS:
	    if (sav_iuh == NULL)
	    {
		screen("ERROR in i_preserve_user_hooks(), "
		       "attempt to restore without prior save\n");
		clean_up(ERROR);
	    }
	    iuh = i_user_hook(dim);
	    *iuh = *sav_iuh;
	    sav_iuh = NULL;
	    break;
	}
}		/*end i_preserve_user_hooks*/


/*
*		SetChunkSize():
*
*	Sets the size of storage chunks for the allocator store.
*	This function may be called at any time, but changing the
*	value of the chunk size will only affect interfaces created
*	after the call to SetChunkSize().
*/

EXPORT	void	SetChunkSize(
	size_t		csize)
{
	I_USER_INTERFACE *user_hook;
	int i;

	for (i = 0; i < 3; ++i)
	{
	    user_hook = i_user_hook(i+1);
	    user_hook->_ChunkSize = csize*sizeof(ALIGN);
	    EnsureSufficientMessageBufferSize(user_hook->_ChunkSize);
	}
}		/*end SetChunkSize*/

/*ARGSUSED*/
LOCAL	void	i_user_make_interface(
	INTERFACE	*intfc)
{
}		/*end i_user_make_interface*/

/*ARGSUSED*/
LOCAL	void	i_user_copy_hyper_surf(
	HYPER_SURF *new_hs,
	HYPER_SURF *old_hs)
{
}		/*end i_user_copy_hyper_surf*/


/*ARGSUSED*/
LOCAL	void i_fset_hyper_surf_color(
	FILE       *file,
	HYPER_SURF *hs)
{
}		/*end i_fset_hyper_surf_color*/

/*ARGSUSED*/
LOCAL	void	i_user_fprint_interface(
	FILE		*file,
	INTERFACE	*intfc)
{
}		/*end i_user_fprint_interface*/

/*ARGSUSED*/
LOCAL	void	i_user_fprint_intfc_rect_grids(
	FILE		*file,
	INTERFACE	*intfc)
{
}		/*end i_user_fprint_intfc_rect_grids*/

LOCAL	void	i_gview_plot_interface(
	const char	*dname,
	INTERFACE	*intfc)
{
	RECT_GRID *gr = &topological_grid(intfc);
	switch (gr->dim)
	{
	case 2:
	    geomview_intfc_plot2d(dname,intfc,gr);
	    break;
	case 3:
	    geomview_interface_plot(dname,intfc,gr);
	    break;
	}
}		/*end i_default_gview_plot_interface*/

/*ARGSUSED*/
LOCAL	void	i_user_read_print_intfc_rect_grids(
	const IO_TYPE *io_type,
	INTERFACE     *intfc,
	boolean	      oldstyle,
	REMAP         *remap)
{
}		/*end i_user_read_print_intfc_rect_grids*/

/*ARGSUSED*/
LOCAL	int	i_user_read_print_interface(
	INIT_DATA     *init,
	const IO_TYPE *io_type,
	INTERFACE     *intfc,
	boolean          overlay)
{
	return YES;
}		/*end i_user_read_print_interface*/

LOCAL	double	i_cross_tolerance(
	INTERFACE *intfc)
{
	double *h = topological_grid(intfc).h;
	double hmin;
	hmin = min(h[0],h[1]);
	hmin = min(hmin,h[2]);
	return MIN_SIN_SQR(intfc)*hmin;/*TOLERANCE*/
}		/*end i_cross_tolerance*/

/*ARGSUSED*/
LOCAL	boolean i_is_excluded_comp(
	COMPONENT	comp,
	COMP_LIST	*comp_list,
	INTERFACE	*intfc)
{
	if (is_exterior_comp(comp,intfc))
	    return YES;
	return i_is_comp_in_list(comp,&excluded_comps(intfc));
}		/*end i_is_excluded_comp*/

/*ARGSUSED*/
LOCAL	void	i_user_fprint_curve(
	FILE		*file,
	CURVE		*curve)
{
}		/*end i_user_fprint_curve*/

/*ARGSUSED*/
LOCAL	void	i_user_fprint_node(
	FILE		*file,
	NODE		*node)
{
}		/*end i_user_fprint_node*/

/*ARGSUSED*/
LOCAL	void	i_user_read_curve(
	CURVE		*curve)
{
}		/*end i_user_read_curve*/

/*ARGSUSED*/
LOCAL	int	i_user_read_node(
	NODE		*node)
{
	return YES;
}		/*end i_user_read_node*/

/*ARGSUSED*/
LOCAL	boolean	i_user_read_print_curve(
	CURVE	      *curve,
	const IO_TYPE *io_type,
	boolean          overlay)
{
	return YES;
}		/*end i_user_read_print_curve*/

/*ARGSUSED*/
LOCAL	void	i_user_read_print_node(
	NODE          *node,
	const IO_TYPE *io_type,
	boolean          overlay)
{
}		/*end i_user_read_print_node*/

/*ARGSUSED*/
LOCAL	boolean i_user_split_curve(
	int		is_a_node,
	FT_POINT		*p,
	BOND		*bond,
	CURVE		*curve,
	CURVE		**curves)
{
	return YES;
}		/*end i_user_split_curve*/

/*ARGSUSED*/
LOCAL	boolean i_user_join_curves(
	CURVE		*curve,
	CURVE		*curve1,
	CURVE		*curve2)
{
	return YES;
}		/*end i_user_join_curves*/

/*ARGSUSED*/
LOCAL	void	i_user_fprint_point(
	FILE		*file,
	FT_POINT		*point)
{
}		/*end i_user_fprint_point*/

/*ARGSUSED*/
LOCAL	void	i_user_read_point(
	INTERFACE	*intfc,
	FT_POINT		*p)
{
}		/*end i_user_read_point*/

/*ARGSUSED*/
LOCAL	void	i_user_read_print_point(
	FT_POINT	      *p,
	const IO_TYPE *io_type,
	boolean          overlay)
{
}		/*end i_user_read_print_point*/

/*ARGSUSED*/
LOCAL	void	i_user_read_surface(
	SURFACE		*s)
{
}		/*end i_user_read_surface*/

/*ARGSUSED*/
LOCAL	void	i_user_read_print_surface(
	SURFACE	      *s,
	const IO_TYPE *io_type,
	boolean          overlay)
{
}		/*end i_user_read_print_surface*/

/*ARGSUSED*/
LOCAL	void	i_user_fprint_surface(
	FILE		*file,
	SURFACE		*s)
{
}		/*end i_user_fprint_surface*/

/*ARGSUSED*/
LOCAL	void	i_user_install_faces(
	SURFACE		*s,
	int		face_num)
{
}		/*end i_user_install_faces*/

/*ARGSUSED*/
LOCAL	void	i_assign_curve_boundary_flag(
	CURVE		*c)
{
}		/*end i_assign_curve_boundary_flag*/

/*ARGSUSED*/
LOCAL	void	i_assign_curve_boundary_type(
	CURVE		*c,
	int		dir,
	int		*side)
{
}		/*end i_assign_curve_boundary_type*/
