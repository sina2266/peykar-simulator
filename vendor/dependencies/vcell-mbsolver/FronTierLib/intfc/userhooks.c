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
*				userhooks.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*			User Supplied Operations
*/

#include <intfc/iloc.h>

/* User interface hooks switching functions */
/* TODO:  upon upgrade to C++ these should be declared inline */

/* Interface topology lookup */

EXPORT	boolean nearest_interface_point_within_range(
	double		   *coords,
	COMPONENT	   comp,
	INTERFACE	   *intfc,
	USE_BOUNDARIES     bdry,
	HYPER_SURF	   *hs,
	double		   *ans,
	double		   *a,
	HYPER_SURF_ELEMENT **phse,
	HYPER_SURF	   **phs,
	int 		   range)	/* Range in unit of grid size */
{
	int icrds[MAXD];
	struct Table *T;

	if (intfc == NULL)
	    return NO;
	switch (intfc->dim)	
	{
	case 2: 
	    return nearest_interface_point_within_range2d(coords,comp,intfc,
					bdry,hs,ans,a,phse,phs,range);
	case 3:
	    return nearest_interface_point_within_range3d(coords,comp,intfc,
					bdry,hs,ans,a,phse,phs,range);
	}
}		/*end nearest_interface_point_within_range*/

EXPORT	boolean nearest_interface_point(
	double		   *coords,
	COMPONENT	   comp,
	INTERFACE	   *intfc,
	USE_BOUNDARIES     bdry,
	HYPER_SURF	   *hs,
	double		   *ans,
	double		   *a,
	HYPER_SURF_ELEMENT **phse,
	HYPER_SURF	   **phs)
{
	if (intfc == NULL)
	    return NO;
	return (*i_user_interface(intfc)._nip)(coords,comp,intfc,bdry,
			                       hs,ans,a,phse,phs);
}		/*end nearest_interface_point*/

EXPORT	boolean nearest_similar_interface_point(
	double		   *crds,
	COMPONENT	   cmpp,
	COMPONENT	   cmpn,
	INTERFACE	   *intfc,
	USE_BOUNDARIES	   bdry,
	HYPER_SURF	   *hs,
	double		   *ans,
	double		   *a,
	HYPER_SURF_ELEMENT **phse,
	HYPER_SURF	   **phs)
{
	if (intfc == NULL)
	    return NO;
	return (*i_user_interface(intfc)._nsip)(crds,cmpp,cmpn,intfc,
			                        bdry,hs,ans,a,phse,phs);
}		/*end nearest_similar_interface_point*/

EXPORT	boolean long_nearest_interface_point(
	double		   *coords,
	COMPONENT	   comp,
	INTERFACE	   *intfc,
	USE_BOUNDARIES	   bdry,
	HYPER_SURF	   *hs,
	double		   *ans,
	double		   *a,
	HYPER_SURF_ELEMENT **phse,
	HYPER_SURF	   **phs)
{
	if (intfc == NULL)
	    return NO;
	return (*i_user_interface(intfc)._lnip)(coords,comp,intfc,bdry,
			                        hs,ans,a,phse,phs);
}		/*end long_nearest_interface_point*/

EXPORT	boolean long_nearest_similar_interface_point(
	double		   *crds,
	COMPONENT	   cmpp,
	COMPONENT	   cmpn,
	INTERFACE	   *intfc,
	USE_BOUNDARIES	   bdry,
	HYPER_SURF	   *hs,
	double		   *ans,
	double		   *a,
	HYPER_SURF_ELEMENT **phse,
	HYPER_SURF	   **phs)
{
	if (intfc == NULL)
	    return NO;
	return (*i_user_interface(intfc)._lnsip)(crds,cmpp,cmpn,intfc,
			                         bdry,hs,ans,a,phse,phs);
}		/*end long_nearest_similar_interface_point*/


EXPORT	boolean next_point(
	INTERFACE	   *intfc,
	FT_POINT		   **pp,
	HYPER_SURF_ELEMENT **phse,
	HYPER_SURF	   **phs)
{
	boolean status;
	if (intfc == NULL)
	    return NO;
	status = (*i_user_interface(intfc)._next_point)(intfc,pp,phse,phs);
	if (status && pp && *pp)
	{
	    (*pp)->hse = *phse;
	    (*pp)->hs = *phs;
	}
	return status;
}		/*end next_point*/

EXPORT	boolean next_hypersurface(
	INTERFACE	*intfc,
	HYPER_SURF	**phs)
{
	if (intfc == NULL)
	    return NO;
	return (*i_user_interface(intfc)._next_hypersurface)(intfc,phs);
}		/*end next_hypersurface*/


/* Boundary type printing */

EXPORT	void fprint_boundary_type(
	FILE		*file,
	const char	*mesg1,
	int		b_type,
	const char	*mesg2,
	INTERFACE	*intfc)
{
	if (intfc == NULL)
	    return;
	(*i_user_interface(intfc)._fprint_boundary_type)(file,mesg1,
		                                         b_type,mesg2,intfc);
}		/*end fprint_boundary_type*/

EXPORT	int read_boundary_type_from_string(
	const char	*type,
	INTERFACE	*intfc)
{
	if (intfc == NULL)
	    return NO;
	return (*i_user_interface(intfc)._read_boundary_type_from_string)(type);
}		/*end read_boundary_type_from_string*/



/* Interface utilities */
EXPORT	void user_make_interface(
	INTERFACE	*intfc)
{
	if (intfc == NULL)
	    return;
	(*i_user_interface(intfc)._user_make_interface)(intfc);
}		/*end user_make_interface*/

EXPORT	INTERFACE *copy_interface(
	INTERFACE	*intfc)
{
	if (intfc == NULL)
	    return NULL;
	return (*i_user_interface(intfc)._copy_interface)(intfc);
}		/*end copy_interface*/

EXPORT	int user_read_print_interface(
	INIT_DATA     *init,
	const IO_TYPE *io_type,
	INTERFACE     *intfc,
	boolean          overlay)
{
	if (intfc == NULL)
	    return NO;
	return (*i_user_interface(intfc)._user_read_print_interface)(init,
	                                                             io_type,
								     intfc,
								     overlay);
}		/*end user_read_print_interface*/

EXPORT	void	print_interface(
	INTERFACE	*intfc)
{
	fprint_interface(stdout,intfc);
}		/*end print_interface*/

EXPORT	void fprint_interface(
	FILE		*file,
	INTERFACE	*intfc)
{
	if (intfc == NULL)
	    return;
	(*i_user_interface(intfc)._fprint_interface)(file,intfc);
}		/*end fprint_interface*/

EXPORT	void user_fprint_interface(
	FILE		*file,
	INTERFACE	*intfc)
{
	if (intfc == NULL)
	    return;
	(*i_user_interface(intfc)._user_fprint_interface)(file,intfc);
}		/*end user_fprint_interface*/

EXPORT	void gview_plot_interface(
	const char *dname,
	INTERFACE  *intfc)
{
	if (intfc == NULL)
	    return;
	(*i_user_interface(intfc)._gview_plot_interface)(dname,intfc);
}		/*end gview_plot_interface*/

EXPORT	int delete_interface(
	INTERFACE	*intfc)
{
	if (intfc == NULL) /*Nothing to do*/
	    return 1;
	return (*i_user_interface(intfc)._delete_interface)(intfc);
}		/*end delete_interface*/


/* Node utilities */

EXPORT	NODE *make_node(
	FT_POINT		*p)
{
	INTERFACE *intfc = current_interface();

	if (intfc == NULL)
	    return NULL;
	return (*i_user_interface(current_interface())._make_node)(p);
}		/*end make_node*/

EXPORT	NODE *copy_node(
	NODE		*n)
{
	if (n == NULL || n->interface == NULL)
	    return NULL;
	return (*i_user_interface(n->interface)._copy_node)(n);
}		/*end copy_node*/

EXPORT	boolean delete_node(
	NODE		*n)
{
	if (n == NULL || n->interface == NULL)
	    return FUNCTION_FAILED;
	if (n->in_curves!=NULL || n->out_curves!=NULL)
	    return FUNCTION_FAILED;
	if (n->interface->nodes==NULL)
	    return FUNCTION_FAILED;

	return (*i_user_interface(n->interface)._delete_node)(n);
}		/*end delete_node*/

EXPORT	void	print_node(
	NODE		*node)
{
	fprint_node(stdout,node);
}		/*end print_node*/

EXPORT	void fprint_node(
	FILE		*file,
	NODE		*n)
{
	if (n == NULL || n->interface == NULL)
	    return;
	(*i_user_interface(n->interface)._fprint_node)(file,n);
}		/*end fprint_node*/

EXPORT	void user_fprint_node(
	FILE		*file,
	NODE		*n)
{
	if (n == NULL || n->interface == NULL)
	    return;
	(*i_user_interface(n->interface)._user_fprint_node)(file,n);
}		/*end user_fprint_node*/

EXPORT	NODE *read_node(
	INTERFACE	*intfc,
	int		i)
{
	if (intfc == NULL)
	    return NULL;
	return (*i_user_interface(intfc)._read_node)(intfc,i);
}		/*end read_node*/

EXPORT	int user_read_node(
	NODE		*n)
{
	if (n == NULL || n->interface == NULL)
	    return NO;
	return (*i_user_interface(n->interface)._user_read_node)(n);
}		/*end user_read_node*/


EXPORT	void user_read_print_node(
	NODE          *n,
	const IO_TYPE *io_type,
	boolean          overlay)
{
	if (n == NULL || n->interface == NULL)
	    return;
	(*i_user_interface(n->interface)._user_read_print_node)(n,io_type,
	                                                        overlay);
}		/*end user_read_print_node*/



/* Rect grid utilities */
EXPORT	void fprint_intfc_rect_grids(
	FILE		*file,
	INTERFACE	*intfc)
{
	if (intfc == NULL)
	    return;
	(*i_user_interface(intfc)._fprint_intfc_rect_grids)(file,intfc);
}		/*end fprint_intfc_rect_grids*/

EXPORT	void user_fprint_intfc_rect_grids(
	FILE		*file,
	INTERFACE	*intfc)
{
	if (intfc == NULL)
	    return;
	(*i_user_interface(intfc)._user_fprint_intfc_rect_grids)(file,intfc);
}		/*end user_fprint_intfc_rect_grids*/

EXPORT	int read_print_intfc_rect_grids(
	const IO_TYPE *io_type,
	INTERFACE     *intfc,
	REMAP         *remap)
{
	if (intfc == NULL)
	    return NO;
	return (*i_user_interface(intfc)._read_print_intfc_rect_grids)(io_type,
								       intfc,
								       remap);
}		/*end read_print_intfc_rect_grids*/

EXPORT	void user_read_print_intfc_rect_grids(
	const IO_TYPE *io_type,
	INTERFACE     *intfc,
	boolean          oldstyle,
	REMAP         *remap)
{
	if (intfc == NULL)
	    return;
	(*i_user_interface(intfc)._user_read_print_intfc_rect_grids)(io_type,
								     intfc,
								     oldstyle,
								     remap);
}		/*end user_read_print_intfc_rect_grids*/


/*Hypersurface utilities*/
EXPORT	HYPER_SURF *make_hypersurface(
	COMPONENT	left_c,
	COMPONENT	right_c)
{
	INTERFACE *intfc = current_interface();

	if (intfc == NULL)
	    return NULL;
	return (*i_user_interface(intfc)._make_hypersurface)(left_c,right_c);
}		/*end make_hypersurface*/

EXPORT	void user_copy_hyper_surf(
	HYPER_SURF *new_hs,
	HYPER_SURF *old_hs)
{
    	INTERFACE *intfc;
	if ((old_hs == NULL) || (old_hs->interface == NULL))
	    return;
	intfc = old_hs->interface;
	(*i_user_interface(intfc)._user_copy_hyper_surf)(new_hs,old_hs);
}		/*end user_copy_hyper_surf*/


/*Hypersurface boundary utilities*/
EXPORT	HYPER_SURF_BDRY *make_hypersurface_boundary(void)
{
	INTERFACE *intfc = current_interface();

	if (intfc == NULL)
	    return NULL;
	return (*i_user_interface(intfc)._make_hypersurface_boundary)();
}		/*end make_hypersurface_boundary*/



/* Curve utilities */
EXPORT	CURVE *make_curve(
	COMPONENT	left_c,
	COMPONENT	right_c,
	NODE		*start,
	NODE		*end)
{
	INTERFACE *intfc = current_interface();

	if (intfc == NULL)
	    return NULL;
	return (*i_user_interface(intfc)._make_curve)(left_c,right_c,start,end);
}		/*end make_curve*/

EXPORT	CURVE *copy_curve(
	CURVE		*c,
	NODE		*start,
	NODE		*end)
{
	if (c == NULL || c->interface == NULL)
	    return NULL;
	return (*i_user_interface(c->interface)._copy_curve)(c,start,end);
}		/*end copy_curve*/

EXPORT	int delete_curve(
	CURVE		*c)
{
	if (c == NULL || c->interface == NULL)
	    return NO;
	return (*i_user_interface(c->interface)._delete_curve)(c);
}		/*end delete_curve*/

EXPORT	void print_curve(
	CURVE		*curve)
{
	fprint_curve(stdout,curve);
}		/*end print_curve*/

EXPORT	void fprint_curve(
	FILE		*file,
	CURVE		*c)
{
	if (c == NULL || c->interface == NULL)
	    return;
	(*i_user_interface(c->interface)._fprint_curve)(file,c);
}		/*end fprint_curve*/

EXPORT	void user_fprint_curve(
	FILE		*file,
	CURVE		*c)
{
	if (c == NULL || c->interface == NULL)
	    return;
	(*i_user_interface(c->interface)._user_fprint_curve)(file,c);
}		/*end user_fprint_curve*/

EXPORT	CURVE *read_curve(
	INTERFACE	*intfc,
	int		i)
{
	if (intfc == NULL)
	    return NULL;
	return (*i_user_interface(intfc)._read_curve)(intfc,i);
}		/*end read_curve*/

EXPORT	void user_read_curve(
	CURVE		*c)
{
	if (c == NULL || c->interface == NULL)
	    return;
	(*i_user_interface(c->interface)._user_read_curve)(c);
}		/*end user_read_curve*/

EXPORT	boolean user_read_print_curve(
	CURVE         *c,
	const IO_TYPE *io_type,
	boolean          overlay)
{
	if (c == NULL || c->interface == NULL)
	    return NO;
	return (*i_user_interface(c->interface)._user_read_print_curve)(c,
	                                                       io_type,overlay);
}		/*end user_read_print_curve*/

EXPORT	boolean user_split_curve(
	int		is_a_node,
	FT_POINT		*p,
	BOND		*bond,
	CURVE		*curve,
	CURVE		**curves)
{
	if (curve == NULL || curve->interface == NULL)
	    return NO;
	return (*i_user_interface((curve)->interface)._user_split_curve)(
		is_a_node,p,bond,curve,curves);
}		/*end user_split_curve*/

EXPORT	boolean user_join_curves(
	CURVE		*curve,
	CURVE		*curve1,
	CURVE		*curve2)
{
	if (curve == NULL || curve->interface == NULL)
	    return NO;
	return (*i_user_interface((curve)->interface)._user_join_curves)(
			curve,curve1,curve2);
}		/*end user_join_curves*/


/* BOND utilities */
EXPORT	BOND *Bond(
	FT_POINT	*start,
	FT_POINT	*end)
{
	INTERFACE *intfc = current_interface();

	if (intfc == NULL)
	    return NULL;
	return (*i_user_interface(intfc)._Bond)(start,end);
}		/*end Bond*/

EXPORT	BOND_TRI *link_tri_to_bond(
	BOND_TRI *btri,
	TRI	 *tri,
	SURFACE  *s,
	BOND	 *b,
	CURVE    *c)
{
	INTERFACE *intfc = current_interface();

	if (intfc == NULL)
	    return NULL;
	return (*i_user_interface(intfc)._link_tri_to_bond)(btri,tri,s,b,c);
}		/*end link_tri_to_bond*/

EXPORT	void reverse_bond(
	BOND *b)
{
	INTERFACE *intfc = current_interface();

	if (intfc == NULL)
	    return;
	(*i_user_interface(intfc)._reverse_bond)(b);
	return;
}	/*end reverse_bond */

EXPORT	void reorder_curve_link_list(
	CURVE *c)
{
	INTERFACE *intfc = current_interface();

	if (intfc == NULL)
	    return;
	(*i_user_interface(intfc)._reorder_curve_link_list)(c);
	return;
}	/*end reorder_curve_link_list */

/* C_BOND utilities */
EXPORT	C_BOND *CBond(
	C_BOND *cb,
	FT_POINT  *start,
	FT_POINT  *end,
	TRI    *t1,
	TRI    *t2)
{
	INTERFACE *intfc = current_interface();

	if (intfc == NULL)
	    return NULL;
	return (*i_user_interface(intfc)._CBond)(cb,start,end,t1,t2);
}		/*end Bond*/


/* Point utilities */
EXPORT	FT_POINT *Point(
	double		*crds)
{
	INTERFACE	*intfc = current_interface();

	if (intfc == NULL)
	    return NULL;
	return (*i_user_interface(intfc)._Point)(crds);
}		/*end Point*/

EXPORT	FT_POINT *Static_point(
	INTERFACE	*intfc)
{
	if (intfc == NULL)
	    return NULL;
	return (*i_user_interface(intfc)._Static_point)(intfc);
}		/*end Static_point*/

EXPORT	FT_POINT *copy_point(
	FT_POINT		*p)
{
	INTERFACE *intfc = current_interface();

	if (intfc == NULL)
	    return NULL;
	return (*i_user_interface(intfc)._copy_point)(p);
}		/*end copy_point*/

EXPORT	FT_POINT *average_points(
	boolean               newpoint,
	FT_POINT		   *p1,
	HYPER_SURF_ELEMENT *hse1,
	HYPER_SURF	   *hs1,
	FT_POINT		   *p2,
	HYPER_SURF_ELEMENT *hse2,
	HYPER_SURF	   *hs2)
{
	INTERFACE *intfc;
	if (hs1->interface != NULL)
	    intfc = hs1->interface;
	else if (hs2->interface != NULL)
	    intfc = hs2->interface;
	else
	    intfc = current_interface();

	if (intfc == NULL)
	    return NULL;
	return (*i_user_interface(intfc)._average_points)(newpoint,p1,hse1,hs1,
		                                                   p2,hse2,hs2);
}		/*end average_points*/


EXPORT	FT_POINT *make_point(
	double		*crds,
	COMPONENT	ncomp,
	COMPONENT	pcomp)
{
	INTERFACE	*intfc = current_interface();

	if (intfc == NULL)
	    return NULL;
	return (*i_user_interface(intfc)._make_point)(crds,ncomp,pcomp);
}		/*end make_point*/

EXPORT	int delete_point(
	FT_POINT		*p)
{
	if (p == NULL)
	{
	    (void) printf("WARNING in delete_point(), p is NULL\n");
	    return NO;
	}
	if (p->interface == NULL)
	{
	    (void) printf("WARNING in delete_point(), p->interface is NULL\n");
	    return NO;
	}
	return (*i_user_interface(p->interface)._delete_point)(p);
}		/*end delete_point*/

EXPORT	void print_point(
	FT_POINT		*point)
{
	fprint_point(stdout,point);
}		/*end print_point*/

EXPORT	void fprint_point(
	FILE		*file,
	FT_POINT		*p)
{
	if (p == NULL || p->interface == NULL)
	    return;
	(*i_user_interface(p->interface)._fprint_point)(file,p);
}		/*end fprint_point*/

EXPORT	void user_fprint_point(
	FILE		*file,
	FT_POINT		*p)
{
	if (p == NULL || p->interface == NULL)
	    return;
	(*i_user_interface(p->interface)._user_fprint_point)(file,p);
}		/*end fprint_point*/

EXPORT	FT_POINT *read_point(
	INTERFACE	*intfc,
	int		i)
{
	if (intfc == NULL)
	    return NULL;
	return (*i_user_interface(intfc)._read_point)(intfc,i);
}		/*end read_point*/

EXPORT	void user_read_point(
	INTERFACE	*intfc,
	FT_POINT		*p)
{
	if (intfc == NULL)
	    return;
	(*i_user_interface(intfc)._user_read_point)(intfc,p);
}		/*end user_read_point*/

EXPORT	FT_POINT	*read_print_point(
	INTERFACE     *intfc,
	const IO_TYPE *io_type,
	boolean          overlay)
{
	if (intfc == NULL)
	    return NULL;
	return (*i_user_interface(intfc)._read_print_point)(intfc,io_type,
	                                                    overlay);
}		/*end read_print_point*/

EXPORT	void user_read_print_point(
	FT_POINT	      *p,
	const IO_TYPE *io_type,
	boolean          overlay)
{
	INTERFACE     *intfc;
	if ((p == NULL) || ((intfc = p->interface) == NULL))
	    return;
	(*i_user_interface(intfc)._user_read_print_point)(p,io_type,overlay);
}		/*end user_read_print_point*/


EXPORT	boolean insert_point_in_bond(
	FT_POINT		*p,
	BOND		*b,
	CURVE		*c)
{
	if (c == NULL || c->interface == NULL)
	    return FUNCTION_FAILED;
	return (*i_user_interface(c->interface)._insert_point_in_bond)(p,b,c);
}		/*end insert_point_in_bond*/

EXPORT	boolean delete_start_of_bond(
	BOND		*b,
	CURVE		*c)
{
	if (c == NULL || c->interface == NULL)
	    return NO;
	return (*i_user_interface(c->interface)._delete_start_of_bond)(b,c);
}		/*end delete_start_of_bond*/

EXPORT	boolean delete_end_of_bond(
	BOND		*b,
	CURVE		*c)
{
	if (c == NULL || c->interface == NULL)
	    return NO;
	return (*i_user_interface(c->interface)._delete_end_of_bond)(b,c);
}		/*end delete_end_of_bond*/




EXPORT	boolean	insert_point_in_tri(
	FT_POINT	*p,
	TRI	*tri,
	SURFACE	*s)
{
	if (s == NULL || s->interface == NULL)
	    return FUNCTION_FAILED;
	return (*i_user_interface(s->interface)._insert_point_in_tri)(p,tri,s);
}		/*end insert_point_in_tri*/

EXPORT	boolean	insert_point_in_tri_side(
	FT_POINT	*p,
	int	side,
	TRI	*tri,
	SURFACE	*s)
{
	INTERFACE *intfc;
	if ((s == NULL) || ((intfc = s->interface) == NULL))
	    return FUNCTION_FAILED;
	return (*i_user_interface(intfc)._insert_point_in_tri_side)(p,side,tri,
								    s);
}		/*end insert_point_in_tri_side*/

EXPORT	boolean	undo_insert_point_in_tri(
	FT_POINT	*p,
	TRI	*tri,
	SURFACE	*s)
{
	INTERFACE *intfc;
	if ((s == NULL) || ((intfc = s->interface) == NULL))
	    return FUNCTION_FAILED;
	return (*i_user_interface(intfc)._undo_insert_point_in_tri)(p,tri,s);
}		/*end undo_insert_point_in_tri*/

EXPORT	boolean	undo_insert_point_in_tri_side(
	FT_POINT	*p,
	int	side,
	TRI	*tri,
	SURFACE	*s)
{
	INTERFACE *intfc;
	if ((s == NULL) || ((intfc = s->interface) == NULL))
	    return FUNCTION_FAILED;
	return (*i_user_interface(intfc)._undo_insert_point_in_tri_side)(p,side,
									 tri,s);
}		/*end undo_insert_point_in_tri_side*/

/* Tri utilities */
EXPORT	TRI	*make_tri(
	FT_POINT		*p0,
	FT_POINT		*p1,
	FT_POINT		*p2,
	POINTER		neighbor01,
	POINTER		neighbor12,
	POINTER		neighbor20,
	int		bdry)
{
	INTERFACE	*intfc = current_interface();

	if (intfc == NULL)
	    return NULL;
	return (*i_user_interface(intfc)._make_tri)(p0,p1,p2,neighbor01,
						   neighbor12,neighbor20,bdry);
}		/*end make_tri*/

/* Surface utilities */
EXPORT	SURFACE *make_surface(
	COMPONENT	nc,
	COMPONENT	pc,
	CURVE		**neg,
	CURVE		**pos)
{
	INTERFACE	*intfc = current_interface();

	if (intfc == NULL)
	    return NULL;
	return (*i_user_interface(intfc)._make_surface)(nc,pc,neg,pos);
}		/*end make_surface*/

EXPORT	SURFACE *copy_surface(
	SURFACE	*s,
	CURVE	**pos,
	CURVE	**neg,
	boolean copy_tris)
{
	if (s == NULL || s->interface == NULL)
	    return NULL;
	return (*i_user_interface(s->interface)._copy_surface)(s,pos,neg,
							       copy_tris);
}		/*end copy_surface*/

EXPORT	int delete_surface(
	SURFACE		*s)
{
	if (s == NULL || s->interface == NULL)
	    return NO;
	return (*i_user_interface(s->interface)._delete_surface)(s);
}		/*end delete_surface*/

EXPORT	SURFACE *join_surfaces(
	CURVE *c)
{
	if (c == NULL || c->interface == NULL)
	    return NULL;
	return (*i_user_interface(c->interface)._join_surfaces)(c);
}		/*end join_surfaces*/

EXPORT	void	 print_surface(
	SURFACE		*s)
{
	fprint_surface(stdout,s);
}		/*end print_surface*/

EXPORT	void fprint_surface(
	FILE		*file,
	SURFACE		*s)
{
	if (s == NULL || s->interface == NULL)
	    return;
	(*i_user_interface(s->interface)._fprint_surface)(file,s);
}		/*end fprint_surface*/

EXPORT	void user_fprint_surface(
	FILE		*file,
	SURFACE		*s)
{
	if (s == NULL || s->interface == NULL)
	    return;
	(*i_user_interface(s->interface)._user_fprint_surface)(file,s);
}		/*end user_fprint_surface*/

EXPORT	SURFACE *read_surface(
	INTERFACE	*intfc,
	int		i)
{
	if (intfc == NULL)
	    return NULL;
	return (*i_user_interface(intfc)._read_surface)(intfc,i);
}		/*end read_surface*/

EXPORT	void user_read_surface(
	SURFACE		*s)
{
	if (s == NULL || s->interface == NULL)
	    return;
	(*i_user_interface(s->interface)._user_read_surface)(s);
}		/*end user_read_surface*/

EXPORT	void user_read_print_surface(
	SURFACE	      *s,
	const IO_TYPE *io_type,
	boolean          overlay)
{
	if (s == NULL || s->interface == NULL)
	    return;
	(*i_user_interface(s->interface)._user_read_print_surface)(s,io_type,
	                                                           overlay);
}		/*end user_read_print_surface*/

	/* Parallel communication utilities */

EXPORT	void send_interface(
	INTERFACE	*intfc,
	int		dst_id)
{
	if (intfc == NULL)
	    return;
	(*i_user_interface(intfc)._send_interface)(intfc,dst_id);
}		/*end send_interface*/

EXPORT	INTERFACE *receive_interface(
	int		src_id)
{
	INTERFACE	*intfc = current_interface();
	INTERFACE	*recvd_intfc;

	if (intfc == NULL)
	    return NULL;
	recvd_intfc = (*i_user_interface(intfc)._receive_interface)(src_id);
	set_current_interface(intfc);
	return recvd_intfc;
}		/*end receive_interface*/

EXPORT	void reconstruct_interface_pointers(
	INTERFACE	*nintfc,
	struct Table	*otbl,
	POINTER		*ocad,
	POINTER		*ncad)
{
	if (nintfc == NULL)
	    return;
	(*i_user_interface(nintfc)._reconstruct_interface_pointers)(nintfc,otbl,
								    ocad,ncad);
}		/*end reconstruct_interface_pointers*/

EXPORT	void reconstruct_point_pointers(
	FT_POINT		*p,
	INTERFACE	*nintfc,
	INTERFACE	*ointfc,
	POINTER		*ocad,
	POINTER		*ncad,
	int		nchks)
{
	if (nintfc == NULL)
	    return;
	(*i_user_interface(nintfc)._reconstruct_point_pointers)(p,nintfc,
								ointfc,ocad,
								ncad,nchks);
	if (nintfc->dim == 3)
	    sorted(p) = YES;	/* This flag should be set here, XLL 8/7/08 */
}		/*end reconstruct_point_pointers*/

EXPORT	void reconstruct_node_pointers(
	NODE		*n,
	INTERFACE	*nintfc,
	INTERFACE	*ointfc,
	POINTER		*ocad,
	POINTER		*ncad,
	int		nchks)
{
	if (nintfc == NULL)
	    return;
	(*i_user_interface(nintfc)._reconstruct_node_pointers)(n,nintfc,ointfc,
							       ocad,ncad,nchks);
}		/*end reconstruct_node_pointers*/

EXPORT	void reconstruct_bond_pointers(
	BOND		*b,
	INTERFACE	*nintfc,
	INTERFACE	*ointfc,
	POINTER		*ocad,
	POINTER		*ncad,
	int		nchks)
{
	if (nintfc == NULL)
	    return;
	(*i_user_interface(nintfc)._reconstruct_bond_pointers)(b,nintfc,ointfc,
							       ocad,ncad,nchks);
}		/*end reconstruct_bond_pointers*/

EXPORT	void reconstruct_curve_pointers(
	CURVE		*c,
	INTERFACE	*nintfc,
	INTERFACE	*ointfc,
	POINTER		*ocad,
	POINTER		*ncad,
	int		nchks)
{
	if (nintfc == NULL)
	    return;
	(*i_user_interface(nintfc)._reconstruct_curve_pointers)(c,nintfc,
								ointfc,ocad,
								ncad,nchks);
}		/*end reconstructcurve_pointers*/

EXPORT	void reconstruct_surface_pointers(
	SURFACE		*s,
	INTERFACE	*nintfc,
	INTERFACE	*ointfc,
	POINTER		*ocad,
	POINTER		*ncad,
	int		nchks)
{
	if (nintfc == NULL)
	    return;
	(*i_user_interface(nintfc)._reconstruct_surface_pointers)(s,nintfc,
								  ointfc,ocad,
								  ncad,nchks);
}		/*end reconstruct_surface_pointers*/

EXPORT	void reconstruct_tri_pointers(
	TRI		*t,
	INTERFACE	*nintfc,
	INTERFACE	*ointfc,
	POINTER		*ocad,
	POINTER		*ncad,
	int		nchks)
{
	if (nintfc == NULL)
	    return;
	(*i_user_interface(nintfc)._reconstruct_tri_pointers)(t,nintfc,ointfc,
							      ocad,ncad,nchks);
}		/*end reconstruct_tri_pointers*/


EXPORT	boolean set_boundary(
	INTERFACE	*intfc,
	RECT_GRID	*gr,
	COMPONENT	i_comp,
	double		eps)
{
	if (intfc == NULL)
	    return NO;
	return (*i_user_interface(intfc)._set_boundary)(intfc,gr,i_comp,eps);
}		/*end set_boundary*/

EXPORT	void user_install_faces(
	SURFACE		*s,
	int		fn)
{
	if (s == NULL || s->interface == NULL)
	    return;
	(*i_user_interface(s->interface)._user_install_faces)(s,fn);
}		/*end user_install_faces*/

EXPORT	void assign_curve_boundary_flag(
	CURVE		*c)
{
	if (c == NULL || c->interface == NULL)
	    return;
	(*i_user_interface(c->interface)._assign_curve_boundary_flag)(c);
}		/*end assign_curve_boundary_flag*/

EXPORT	void assign_curve_boundary_type(
	CURVE		*c,
	int 		dir,
	int 		*side)
{
	if (c == NULL || c->interface == NULL)
	    return;
	(*i_user_interface(c->interface)._assign_curve_boundary_type)(c,
				dir,side);
}		/*end assign_curve_boundary_type*/

EXPORT	boolean intersections(
	INTERFACE	*intfc,
	CROSS		**cross,
	const boolean	bdry)
{
	if (intfc == NULL)
	    return NO;
	return (*i_user_interface(intfc)._intersections)(intfc,cross,bdry);
}		/*end intersections*/

EXPORT	double cross_tolerance(
	INTERFACE	*intfc)
{
	if (intfc == NULL)
	    return NO;
	return (*i_user_interface(intfc)._cross_tolerance)(intfc);
}		/*end cross_tolerance*/


EXPORT	void print_intersections(
	CROSS		*cross,
	INTERFACE	*intfc)
{
	if (intfc == NULL ||
			i_user_interface(intfc)._print_intersections == NULL)
		return;
	(*i_user_interface(intfc)._print_intersections)(cross,intfc);
}		/*end print_intersections*/

EXPORT	int print_number_of_tangles(
	const char *mesg,
	INTERFACE  *intfc,
	CROSS	   *cross)
{
	if (intfc == NULL ||
	    i_user_interface(intfc)._print_number_of_tangles==NULL)
	    return 0;
	return (*i_user_interface(intfc)._print_number_of_tangles)(mesg,
								   intfc,cross);
}		/*end print_number_of_tangles*/

EXPORT	void print_crossing_elements(
	CROSS		*cross,
	INTERFACE	*intfc)
{
	if (intfc == NULL ||
			i_user_interface(intfc)._print_crossing_elements==NULL)
		return;
	(*i_user_interface(intfc)._print_crossing_elements)(cross,intfc);
}		/*end print_crossing_elements*/

EXPORT	CURVE *attach_curve_to_node(
	CURVE		*c1,
	FT_POINT		*p,
	BOND		*b,
	NODE		*n)
{
	if (n == NULL || n->interface == NULL)
	    return NULL;
	return (*i_user_interface(n->interface)._attach_curve_to_node)(c1,p,b,n);
}		/*end attach_curve_to_node*/

EXPORT	void invert_curve(
	CURVE		*c)
{
	if (c == NULL || c->interface == NULL)
	    return;
	(*i_user_interface(c->interface)._invert_curve)(c);
}		/*end invert_curve*/

EXPORT	void invert_surface(
	SURFACE		*surf)
{
	if (surf == NULL || surf->interface == NULL)
	    return;
	(*i_user_interface(surf->interface)._invert_surface)(surf);
}		/*end invert_surface*/

EXPORT	void reverse_curve(
	CURVE		*c)
{
	if (c == NULL || c->interface == NULL)
	    return;
	(*i_user_interface(c->interface)._reverse_curve)(c);
}		/*end reverse_curve*/

EXPORT	boolean move_closed_loop_node(
	CURVE		*c,
	BOND		*b)
{
	if (c == NULL || c->interface == NULL)
	    return NO;
	return (*i_user_interface(c->interface)._move_closed_loop_node)(c,b);
}		/*end move_closed_loop_node*/

EXPORT	boolean is_subdomain_boundary(
	HYPER_SURF	*hs)
{
	if (hs == NULL || hs->interface == NULL)
	    return NO;
	return (*i_user_interface(hs->interface)._is_subdomain_boundary)(hs);
}		/*end is_subdomain_boundary*/

EXPORT	boolean is_subdomain_node(
	NODE		*n)
{
	if (n == NULL || n->interface == NULL)
	    return NO;
	return (*i_user_interface(n->interface)._is_subdomain_node)(n);
}		/*end is_subdomain_node*/

EXPORT	boolean is_virtual_fixed_node(
	NODE		*n)
{
	if (n == NULL || n->interface == NULL)
	    return NO;
	return (*i_user_interface(n->interface)._is_virtual_fixed_node)(n);
}		/*end is_virtual_fixed_node*/

EXPORT	void fset_hyper_surf_color(
	FILE            *file,
	HYPER_SURF	*hs)
{
	if (hs == NULL || hs->interface == NULL)
	    return;
	(*i_user_interface(hs->interface)._fset_hyper_surf_color)(file,hs);
}		/*end fset_hyper_surf_color*/

EXPORT	INTERFACE *zoom_interface(
	INTERFACE	*intfc,
	RECT_GRID	*gr,
	double		*L,
	double		*U,
	double		**Q)
{
	if (intfc == NULL)
		return NULL;
	return (*i_user_interface(intfc)._zoom_interface)(intfc,gr,L,U,Q);
}		/*end zoom_interface*/

EXPORT	void	reflect_interface(
	INTERFACE	*intfc,	/* Interface being reflected */
	double		*p,	/* point on reflection plane */
	double		*n)	/* normal vector to reflection plane */
{
	if (intfc == NULL)
		return;
	(*i_user_interface(intfc)._reflect_interface)(intfc,p,n);
}		/*end reflect_interface*/

EXPORT	void	reflect_node(
	NODE		*node,	/* Node being reflected */
	double		*p,	/* point on reflection plane */
	double		*n)	/* normal vector to reflection plane */
{
	if (node==NULL || node->interface==NULL ||
			i_user_interface(node->interface)._reflect_node==NULL)
		return;
	(*i_user_interface(node->interface)._reflect_node)(node,p,n);
}		/*end reflect_node*/

EXPORT	void	reflect_curve(
	CURVE		*curve,	/* Curve being reflected */
	double		*p,	/* point on reflection plane */
	double		*n)	/* normal vector to reflection plane */
{
	if (curve==NULL || curve->interface==NULL ||
			i_user_interface(curve->interface)._reflect_curve==NULL)
		return;
	(*i_user_interface(curve->interface)._reflect_curve)(curve,p,n);
}		/*end reflect_curve*/

EXPORT	void	reflect_surface(
	SURFACE		*surface,/* Surface being reflected */
	double		*p,	/* point on reflection plane */
	double		*n)	/* normal vector to reflection plane */
{
	if (surface==NULL || surface->interface==NULL ||
		i_user_interface(surface->interface)._reflect_surface==NULL)
		return;
	(*i_user_interface(surface->interface)._reflect_surface)(surface,p,n);
}		/*end reflect_surface*/

EXPORT	boolean	consistent_interface(
	INTERFACE	*intfc)
{
	if (intfc==NULL ||
	    i_user_interface(intfc)._consistent_interface==NULL)
	    return NO;
	return (*i_user_interface(intfc)._consistent_interface)(intfc);
}		/*end consistent_interface*/

EXPORT	boolean	sort_bond_tris(
	INTERFACE	*intfc)
{
	if (intfc==NULL ||
	    i_user_interface(intfc)._sort_bond_tris==NULL)
	    return NO;
	return (*i_user_interface(intfc)._sort_bond_tris)(intfc);
}		/*end sort_bond_tris*/

EXPORT	boolean	assign_btri_states(
	BOND_TRI	*newbtri,
	BOND_TRI	*btri)
{
INTERFACE	*intfc = current_interface();
	
	if (intfc==NULL ||
	    i_user_interface(intfc)._assign_btri_states==NULL)
	    return NO;
	return (*i_user_interface(intfc)._assign_btri_states)(newbtri, btri);
}		/*end assign_btri_states*/

EXPORT SURFACE  *detach_one_surface(
	SURFACE  *s)
{
	INTERFACE	*intfc = current_interface();
	
	if (intfc==NULL ||
	    i_user_interface(intfc)._detach_one_surface==NULL)
	    return NO;
	return (*i_user_interface(intfc)._detach_one_surface)(s);
}

EXPORT	void  check_print_intfc(
	const char  *msg,
	const char  *fname,
	char   	    ch,
	INTERFACE   *intfc, 
	int	    step,
	int         fstep,
	boolean	    final)
{
	if(!debugging("check_print_intfc"))
	    return;

	if (intfc==NULL ||
	    i_user_interface(intfc)._check_print_intfc==NULL)
	    return;
	(*i_user_interface(intfc)._check_print_intfc)(msg,fname,ch,
					 intfc,step,fstep,final);
}

EXPORT  void  print_wall_crx(
	const char	*msg,
	int  		*ip, 
	int 		dir, 
	int 		k, 
	CRXING  	*crx)
{
	INTERFACE	*intfc = current_interface();
	
	if (intfc==NULL ||
	    i_user_interface(intfc)._print_wall_crx==NULL)
	    return;
	(*i_user_interface(intfc)._print_wall_crx)(msg,ip,dir,k,crx);
}

/*init:  insert_curve_face_crossings */
/*use:   fill_block_curve_crx */
EXPORT  void  print_wall_curve_crx(
	const	char	*msg,
	int  		*ip, 
	int 		dir, 
	int 		k, 
	CRXING  	*crx)
{
	INTERFACE	*intfc = current_interface();
	
	if (intfc==NULL ||
	    i_user_interface(intfc)._print_wall_curve_crx==NULL)
	    return;
	(*i_user_interface(intfc)._print_wall_curve_crx)(msg,ip,dir,k,crx);
}

/*init:  insert_curve_face_crossings */
/*use:   install_curve_points_state */
/*check the crx p pair */
EXPORT  void  print_wall_curve_crx0(
	const char	*msg,
	FT_POINT		*p,
	int 		k, 
	CRXING  	*crx)
{
	INTERFACE	*intfc = current_interface();
	
	if (intfc==NULL ||
	    i_user_interface(intfc)._print_wall_curve_crx0==NULL)
	    return;
	(*i_user_interface(intfc)._print_wall_curve_crx0)(msg,p,k,crx);
}


EXPORT	void	reflect_point(
	FT_POINT		*point,	/* Point being reflected */
	double		*p,	/* point on reflection plane */
	double		*n,	/* normal vector to reflection plane */
	INTERFACE	*intfc)	/* Interface being reflected */
{
	if (intfc==NULL || i_user_interface(intfc)._reflect_point==NULL)
	    return;
	(*i_user_interface(intfc)._reflect_point)(point,p,n,intfc);
}		/*end reflect_point*/

EXPORT	double random01(
	INTERFACE *intfc)
{
	if (intfc==NULL)
	    return 0.0;
	return (*i_user_interface(intfc)._random01)(intfc);
}		/*end random01*/

EXPORT	boolean	make_interface_topology_lists(
	INTERFACE	*intfc)
{
	if (intfc==NULL ||
	    i_user_interface(intfc)._make_interface_topology_lists==NULL)
	    return FUNCTION_FAILED;
	if (no_topology_lists(intfc) == YES)
	{
	    screen("ERROR in make_interface_topology_lists(), "
		   "illegal attempt to construct interface topology\n"
		   "no_topology_lists(intfc) == YES\n");
	    clean_up(ERROR);
	}
	return (*i_user_interface(intfc)._make_interface_topology_lists)(intfc);
}		/*end make_interface_topology_lists*/
