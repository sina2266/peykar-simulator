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
*                               fuserhooks.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Front Extensions to Interface and Rproblem User Supplied Operations
*/

#include <front/fdecs.h>


/* Front extensions to intfc user interface hooks functions */

EXPORT	void	fprint_hsbdry_type(
	FILE       *file,
	const char *mesg1,
	int        hsb_type,
	const char *mesg2,
	INTERFACE  *intfc)
{
	if (intfc == NULL)
	    return;
	(*f_user_interface(intfc)._fprint_hsbdry_type)(
		file,mesg1,hsb_type,mesg2,intfc);
}		/*end fprint_hsbdry_type*/

EXPORT	int	read_hsbdry_type_from_string(
	char* type,
	INTERFACE* intfc)
{
	if (intfc == NULL)
	    return ERROR;
	return (*f_user_interface(intfc)._read_hsbdry_type_from_string)(type,
								intfc);
}		/*end read_hsbdry_type_from_string*/

EXPORT	void	fprint_wave_type(
	FILE* file,
	const char *mesg1,
	int        w_type,
	const char *mesg2,
	INTERFACE* intfc)
{
	if (intfc == NULL)
	    return;
	(*f_user_interface(intfc)._fprint_wave_type)(file,mesg1,w_type,
	                                             mesg2,intfc);
}		/*end fprint_wave_type*/

EXPORT	const char *wave_type_as_string(
	int       w_type,
	INTERFACE *intfc)
{
	if (intfc == NULL)
	    return "INTERFACE IS NULL, UNKNOWN_WAVE_TYPE";
	return (*f_user_interface(intfc)._wave_type_as_string)(w_type);
}		/*end wave_type_as_string*/


EXPORT	int	read_wave_type_from_string(
	const char* type,
	INTERFACE* intfc)
{
	if (intfc == NULL)
	    return ERROR;
	return (*f_user_interface(intfc)._read_wave_type_from_string)(type);
}		/*end read_wave_type_from_string*/

EXPORT	boolean	nearest_intfc_state(
	double *coords,
	COMPONENT comp,
	INTERFACE *intfc,
	Locstate state,
	double *coords_on,
	HYPER_SURF **hs_on)
{
	if (intfc == NULL)
	    return NO;
	return (*f_user_interface(intfc)._nearest_intfc_state)(coords,comp,
							       intfc,state,
							       coords_on,hs_on);
}		/*end nearest_intfc_state*/

EXPORT	void	bi_interpolate_intfc_states(
	INTERFACE *intfc,
	double alpha,
	double beta,
	double *coords0,
	Locstate s0,
	double *coords1,
	Locstate s1,
	Locstate ans)
{
	if (intfc == NULL)
	    return;
	(*f_user_interface(intfc)._bi_interpolate_intfc_states)(alpha,beta,
				coords0,s0,coords1,s1,
				computational_grid(intfc),ans);
}		/*end bi_interpolate_intfc_states*/

EXPORT	boolean	tri_interpolate_intfc_states(
	INTERFACE *intfc,
	double alpha,
	double beta,
	double gamma,
	double *coords0,
	Locstate s0,
	double *coords1,
	Locstate s1,
	double *coords2,
	Locstate s2,
	Locstate ans)
{
	if (intfc == NULL)
	    return FUNCTION_FAILED;
	return (*f_user_interface(intfc)._tri_interpolate_intfc_states)(
			alpha,beta,gamma,coords0,s0,coords1,s1,coords2,s2,
			computational_grid(intfc),ans);
}		/*end tri_interpolate_intfc_states*/

EXPORT	void	slsr(
	FT_POINT			*p,
	HYPER_SURF_ELEMENT	*hse,
	HYPER_SURF		*hs,
	Locstate		*sl,
	Locstate		*sr)
{
	if (hs == NULL || hs->interface == NULL)
	{
	    *sl = *sr = NULL;
	    return;
	}
	(*f_user_interface(hs->interface)._slsr)(p,hse,hs,sl,sr);
}		/*end slsr*/

EXPORT	void	state_along_hypersurface_element(
	COMPONENT comp,
	double* t,
	HYPER_SURF_ELEMENT* hse,
	HYPER_SURF* hs,
	Locstate state)
{
	if (hs == NULL || hs->interface == NULL)
	    return;
	f_user_interface(hs->interface)._state_along_hypersurface_element(
	    comp,t,hse,hs,state);
}		/*end state_along_hypersurface_element*/

EXPORT	void	fprint_state_data(
	FILE* file,
	Locstate st,
	INTERFACE* intfc)
{
	if (intfc == NULL)
	    return;
	(*f_user_interface(intfc)._fprint_state_data)(file,st,intfc);
}		/*end fprint_state_data*/

EXPORT	Locstate read_print_state_data(
	INIT_DATA     *init,
	const IO_TYPE *io_type,
	Locstate      state,
	INTERFACE     *intfc)
{
	if (intfc == NULL)
	    return NULL;
	return (*f_user_interface(intfc)._read_print_state_data)(init,io_type,
	                                                         state,intfc);
}		/*end read_print_state_data*/

EXPORT	boolean	form_subintfc_via_communication(
	Front* fr)
{
	if (fr == NULL || fr->interf == NULL)
	    return FUNCTION_FAILED;
	return (*f_user_interface(fr->interf)._form_subintfc_via_communication)(fr);
}		/*end form_subintfc_via_communication*/

EXPORT	void	reflect_state(
	Locstate	state,	/* state being reflected */
	INTERFACE	*intfc,	/* interface containing state */
	double		*pt,	/* position of state being reflected */
	double		*p,	/* point on reflection plane */
	double		*n)	/* normal vector to reflection plane */
{
	if (intfc == NULL)
	    return;
	(*f_user_interface(intfc)._reflect_state)(state,intfc,pt,p,n);
}		/*end reflect_state*/

EXPORT	void	fprint_intfc_state(
	FILE		*file,
	Locstate	state,	/* state being printed */
	INTERFACE	*intfc)	/* interface containing state */
{
	if (intfc == NULL)
	    return;
	(*f_user_interface(intfc)._fprint_intfc_state)(file,state,intfc);
}		/*end fprint_intfc_state*/

EXPORT	void	print_intfc_state(
	Locstate	state,	/* state being printed */
	INTERFACE	*intfc)	/* interface containing state */
{
	if (intfc == NULL)
	    return;
	(*f_user_interface(intfc)._fprint_intfc_state)(stdout,state,intfc);
}		/*end print_intfc_state*/

EXPORT	void	fshow_intfc_states(
	FILE		*file,
	INTERFACE	*intfc)
{
	if (intfc == NULL)
	    return;
	(*f_user_interface(intfc)._fshow_intfc_states)(file,intfc);
}		/*end fshow_intfc_states*/

EXPORT	void	show_intfc_states(
	INTERFACE	*intfc)
{
	if (intfc == NULL)
	    return;
	(*f_user_interface(intfc)._fshow_intfc_states)(stdout,intfc);
}		/*end show_intfc_states*/

EXPORT	void	init_rp_nodes(
	RPROBLEM* rp)
{
	if (rp == NULL)
	    return;
	(*f_user_rproblem(rp)._init_rp_nodes)(rp);
}		/*end init_rp_nodes*/

EXPORT	void	delete_curve_from_rp_node(
	CURVE* c,
	RP_NODE* rpn,
	RPROBLEM* rp)
{
	if (rp == NULL)
	    return;
	(*f_user_rproblem(rp)._delete_curve_from_rp_node)(c,rpn,rp);
}		/*end delete_curve_from_rp_node*/

EXPORT	void	free_rp_node(
	RP_NODE* rpn,
	RPROBLEM* rp)
{
	if (rp == NULL)
	    return;
	(*f_user_rproblem(rp)._free_rp_node)(rpn,rp);
}		/*end free_rp_node*/

EXPORT	void	user_free_rp_node(
	RP_NODE* rpn,
	RPROBLEM* rp)
{
	if (rp == NULL)
	    return;
	(*f_user_rproblem(rp)._user_free_rp_node)(rpn,rp);
}		/*end user_free_rp_node*/

EXPORT	void	print_rp_node(
	RP_NODE* rpn,
	RPROBLEM* rp)
{
	if (rp == NULL)
	    return;
	(*f_user_rproblem(rp)._print_rp_node)(rpn,rp);
}		/*end print_rp_node*/

EXPORT	void	user_print_rp_node(
	RP_NODE* rpn,
	RPROBLEM* rp)
{
	if (rp == NULL)
	    return;
	(*f_user_rproblem(rp)._user_print_rp_node)(rpn,rp);
}		/*end user_print_rp_node*/

EXPORT	void	print_rproblem(
	RPROBLEM* rp)
{
	if (rp == NULL)
	    return;
	(*f_user_rproblem(rp)._print_rproblem)(rp);
}		/*end print_rproblem*/

EXPORT	void	user_print_rproblem(
	RPROBLEM* rp)
{
	if (rp == NULL)
	    return;
	(*f_user_rproblem(rp)._user_print_rproblem)(rp);
}		/*end user_print_rproblem*/

EXPORT	void	set_rp_statistics(
	RPROBLEM* rp)
{
	if (rp == NULL)
	    return;
	(*f_user_rproblem(rp)._set_rp_statistics)(rp);
}		/*end set_rp_statistics*/

EXPORT	void	set_phys_ocurves_to_null(
	RP_NODE* rpn,
	RPROBLEM* rp)
{
	if (rp == NULL)
	    return;
	(*f_user_rproblem(rp)._set_phys_ocurves_to_null)(rpn,rp);
}		/*end set_phys_ocurves_to_null*/

EXPORT	void	GetFrontCurvature(
        FT_POINT                   *p,
        HYPER_SURF_ELEMENT      *hse,         
	HYPER_SURF              *hs,
	double			*curvature,
        Front                   *fr) 
{
	*curvature = mean_curvature_at_point(p,hse,hs,fr);
}	/* end GetFrontCurvature */

EXPORT	double	mean_curvature_at_point(
	FT_POINT			*p,
	HYPER_SURF_ELEMENT	*hse,
	HYPER_SURF		*hs,
	Front			*fr)
{
	INTERFACE *intfc;
	if ((hs == NULL) || (hs->interface == NULL))
	    return 0.0;
	intfc = hs->interface;
	if (f_user_interface(intfc)._mean_curvature_at_point == NULL)
	    return 0.0;
	return (*f_user_interface(intfc)._mean_curvature_at_point)(p,hse,hs,fr);
}		/*end mean_curvature_at_point*/


EXPORT	MAX_FRONT_SPEED	*alloc_MaxFrontSpeed(
	MAX_FRONT_SPEED *mfs,
	INTERFACE       *intfc,
	size_t          sizest)
{
	if (intfc == NULL)
	    return NULL;
	return (*f_user_interface(intfc)._alloc_MaxFrontSpeed)(mfs,intfc,
	                                                       sizest);
}		/*end alloc_MaxFrontSpeed*/

EXPORT boolean merge_hs_flags(
	HYPER_SURF	*hs1,
	HYPER_SURF	*hs2)
{
	if (hs1->interface != hs2->interface)
	    return NO;
	if (hs1->interface == NULL)
	    return NO;
	return (*f_user_interface(hs1->interface)._merge_hs_flags)(hs1,hs2);
}		/*end merge_hs_flags*/

EXPORT	void  set_normal_function(
	const char      *s,
	NORMAL_FUNCTION *nf,
	INTERFACE       *intfc)
{
	if (intfc != NULL)
	    (*f_user_interface(intfc)._set_normal_function)(s,nf,intfc);   
}		/*end set_tangent_function*/

EXPORT	void  set_tangent_function(
	const char       *s,
	TANGENT_FUNCTION *tf,
	INTERFACE        *intfc)
{
	if (intfc != NULL)
	    (*f_user_interface(intfc)._set_tangent_function)(s,tf,intfc);   
}		/*end set_tangent_function*/

EXPORT	void	normal(
	FT_POINT              *p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF         *hs,
	double              *nor,
	Front              *front)
{
	INTERFACE *intfc;
	if ((hs == NULL) || (hs->interface == NULL))
	{
	    screen("ERROR in normal(), hs == NULL || hs->interface == NULL\n");
	    clean_up(ERROR);
	    return;
	}
	intfc = hs->interface;
	if (hypersurface_normal(hs) != NULL)
	    (*hypersurface_normal(hs))(p,hse,hs,nor,front);
	else if (interface_normal(intfc) != NULL)
	    (*interface_normal(intfc))(p,hse,hs,nor,front);
	else
	{
	    screen("ERROR in normal(), no function hook available\n");
	    clean_up(ERROR);
	}
}		/*end normal*/

EXPORT	void	tangent(
	FT_POINT *p,
	BOND  *b,
	CURVE *c,
	double *tgnt,
	Front *front)
{
	INTERFACE *intfc;
	if ((c == NULL) || (c->interface == NULL))
	{
	    screen("ERROR in tangent(), c == NULL || c->interface == NULL\n");
	    clean_up(ERROR);
	    return;
	}
	intfc = c->interface;
	if (curve_tangent(c) != NULL)
	    (*curve_tangent(c))(p,b,c,tgnt,front);
	else if (interface_tangent(intfc) != NULL)
	    (*interface_tangent(intfc))(p,b,c,tgnt,front);
	else
	{
	    screen("ERROR in tangent(), no function hook available\n");
	    clean_up(ERROR);
	}
}		/*end tangent*/

