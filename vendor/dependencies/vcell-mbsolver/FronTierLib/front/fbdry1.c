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
*				fbdry1.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	This file contains routines to determine, impose or untangle
*	front boundaries and boundary conditions. Specifically:
*			f_impose_bc()
*			nearest_linked_bdry_like_curve()
*			boundary_data_type()
*			nearest_boundary_curve()
*			next_boundary()
*			f_boundary_untangle()
*/

#if defined(TWOD)

#include <front/fdecs.h>

	/* LOCAL Function Declarations */
LOCAL	boolean	interior_curve_crosses_boundary(CROSS*);
LOCAL	int	assign_states_of_boundary_cross(Front*,CROSS*,ORIENTATION,
						ANGLE_DIRECTION,
						CURVE**,CURVE**,int,RPROBLEM*);
LOCAL	void	eliminate_redundant_closed_node_at_bdry_tangle(CROSS*,
							       CURVE**,
							       ORIENTATION);
LOCAL	void	map_states_at_node(Locstate,CURVE*,ORIENTATION,SIDE,Front*);
LOCAL	void	nearest_boundary_curve(double*,INTERFACE*,double*,BOND**,
				       CURVE**,COMPONENT*,int*);
LOCAL	void	nearest_linked_bdry_like_curve(FT_POINT*,BOND*,CURVE*,double*,
					       BOND**,CURVE**,int);


/*
*			next_boundary():
*
*	Finds the next boundary curve and its orientation, defined by a
*	given curve and orientation, hence a given curve and node.
*	We assume that we are not interested in boundary curves with
*	excluded comps on both sides.  Some such curves are passive
*	boundaries, and some are subdomain boundaries.
*/

EXPORT int next_boundary(
	CURVE		*c1,
	ORIENTATION	o1,
	CURVE		**c2,
	ORIENTATION	*o2)
{
	CURVE		**c;
	INTERFACE	*intfc = c1->interface;

#define bdry_of_interior_region(cur,intfc)				\
	( (wave_type(cur) != PASSIVE_BOUNDARY) &&			\
	  (wave_type(cur) < FIRST_PHYSICS_WAVE_TYPE) &&			\
	  (								\
	    ( !is_excluded_comp(positive_component(cur),intfc) 		\
	    ||								\
	      !is_excluded_comp(negative_component(cur),intfc) 		\
	    )								\
	  )								\
	)

	*c2 = NULL;
	*o2 = ORIENTATION_NOT_SET;
	for (c = Node_of(c1,o1)->in_curves; c && *c; ++c)
	{
	    if ((o1 == POSITIVE_ORIENTATION || c1 != *c) &&
	    	bdry_of_interior_region(*c,intfc))
	    {
	    	*c2 = *c;
	    	*o2 = NEGATIVE_ORIENTATION;
		if (is_bdry(c1) == is_bdry(*c2))
	    	    return YES;
	    }
	}
	for (c = Node_of(c1,o1)->out_curves; c && *c; ++c)
	{
	    if ((o1 == NEGATIVE_ORIENTATION || c1 != *c) &&
	    	 bdry_of_interior_region(*c,intfc))
	    {
	    	*c2 = *c;
	    	*o2 = POSITIVE_ORIENTATION;
		if (is_bdry(c1) == is_bdry(*c2))
	    	    return YES;
	    }
	}
	return (*c2 != NULL) ? YES : NO;
}		/*end next_boundary*/




/*
*			f_impose_bc():
*    
*	If is_normal == YES, the vector  w  is projected to be othogonal
*	to the closest boundary of the computational rectangle.
*	Otherwise w is projected to be tangent to the closest boundary.
*	If the flag normalize is YES, the returned vector will be normalized
*	to a unit uni_array.
*
*    	It is assumed that the POINT *p is a point on the boundary of the
*	computational square.
*/

/*ARGSUSED*/
EXPORT void f_impose_bc(
	FT_POINT		*p,
	BOND		*b,
	CURVE		*c,
	double		*w,
	Front		*front,
	boolean		is_normal,
	boolean		normalize)
{
	COMPONENT	comp;
	BOND		*bbdry;
	CURVE		*cbdry;
	double		t, tgnt[MAXD], db[MAXD], wtan;
	int		i, dim = front->rect_grid->dim;
	int		w_type = NEUMANN_BOUNDARY;

	if (wave_type(c) < FIRST_PHYSICS_WAVE_TYPE)
	    return;
	if (debugging("fibc"))
	{
	    print_general_vector("imposing bc on ",Coords(p),dim,"");
	    (void) printf(" curve %llu\nbond - ",curve_number(c));
	    print_bond(b);
	}
	cbdry = NULL;
	if ((p == c->start->posn) || (p == c->end->posn))
	{
	    nearest_linked_bdry_like_curve(p,b,c,&t,&bbdry,&cbdry,w_type);
	    if (debugging("fibc"))
	    {
	    	(void) printf("nlblc id bc as %llu  t %g bbdry - ",
			      curve_number(cbdry),t);
		print_bond(bbdry);
	    }
	}
	if (cbdry == NULL)
	    nearest_boundary_curve(Coords(p),front->interf,&t,&bbdry,
			           &cbdry,&comp,&w_type);
	if (debugging("fibc"))
	{
	    (void) printf("id bc as %llu  t %g bbdry - ",
			  curve_number(cbdry),t);
	    print_bond(bbdry);
	}
	if (cbdry == NULL)
	    return;

	if (is_normal == YES)
	{
	    if (p == bbdry->start || p == bbdry->end)
	    	normal(p,Hyper_surf_element(bbdry),Hyper_surf(cbdry),
		       tgnt,front);
	    else if (t > 0.5)
	    	normal(bbdry->end,Hyper_surf_element(bbdry),Hyper_surf(cbdry),
		       tgnt,front);
	    else
	    	normal(bbdry->start,Hyper_surf_element(bbdry),Hyper_surf(cbdry),
		       tgnt,front);
	}
	else
	{
	    for (i = 0; i < dim; ++i)
	       tgnt[i] = (Coords(bbdry->end)[i] - Coords(bbdry->start)[i])/
	    		 bond_length(bbdry);
	}

	wtan = scalar_product(tgnt,w,dim);

	if (debugging("fibc"))
	{
	    print_general_vector("tgnt ",tgnt,dim,"");
	    (void) printf("wtan %g\n",wtan);
	}

	if (normalize == YES)
	{
	    if (wtan > 0.0)
	    {
	    	for (i = 0; i < dim; ++i)
		    w[i] = tgnt[i];
		return;
	    }
	    else if (wtan < 0.0)
	    {
	    	for (i = 0; i < dim; ++i)
		    w[i] = -tgnt[i];
		return;
	    }
	    if (is_normal == YES)
	    {
	    	for (i = 0; i < dim; ++i)
	    	   db[i] = Coords(b->end)[i] - Coords(b->start)[i];
		wtan = scalar_product(tgnt,db,dim);

	    }
	    else
	    {
	    	if (p == b->end)
	    	{
	    	    for (i = 0; i < dim; ++i)
	    	       db[i] = Coords(b->end)[i] - Coords(b->start)[i];
	    	    wtan = scalar_product(tgnt,db,dim);
	    	}
	    	else if (p == b->start)
	    	{
	    	    for (i = 0; i < dim; ++i)
	    	       db[i] = Coords(b->start)[i] - Coords(b->end)[i];
	    	    wtan = scalar_product(tgnt,db,dim);
	    	}
	    }
	    if (wtan < 0.0)
	    {
	    	for (i = 0; i < dim; ++i)
		    w[i] = -tgnt[i];
	    	return;
	    }
	    else
	    {
	    	for (i = 0; i < dim; ++i)
		    w[i] = tgnt[i];
		return;
	    }
	}
	else
	{
	    for (i = 0; i < dim; ++i)
	    	w[i] = wtan * tgnt[i];
	}
}		/*end f_impose_bc*/

/*
*			nearest_boundary_curve():
*
*	Finds the nearest position on a boundary curve to the point coords.
*	The optional integer type is provided for extra checking
*	in the case of corners.
*	The boundary curve returned will lie on the same side of the
*	computational rectangle as the point x,y.
*	If these criteria are not matched NULL is returned for the boundary
*	curve.
*	Note: rectangular boundary geometry is assumed through use of
*	the nearest_boundary side logic.
*/

/*ARGSUSED*/
LOCAL void nearest_boundary_curve(
	double		*coords,
	INTERFACE	*intfc,
	double		*t,
	BOND		**b,
	CURVE		**c,
	COMPONENT	*comp,
	int		*type)
{
	RECT_GRID	*gr = &topological_grid(intfc);
	HYPER_SURF	*hs2;
	HYPER_SURF_ELEMENT *hse;
	CURVE		**c1;
	FT_POINT		*ps, *pe;
	double		coords_c1[MAXD];
	double		l1s, l1e;
	double		residual = HUGE_VAL, new_residual;
	double		d1,d2,d3;
	double		dim = intfc->dim;
	int		side;
	int		i;

	side = nearest_boundary(coords,gr);
	*c = NULL;
	for (c1 = intfc->curves; *c1; ++c1) 
	{
		if (!is_bdry(*c1)) continue;
		if (type && wave_type(*c1) != *type) continue;

		ps = (*c1)->start->posn;	pe = (*c1)->end->posn;
		for (i = 0; i < dim; ++i)
			coords_c1[i] = 0.5*(Coords(ps)[i] + Coords(pe)[i]);
		if (side != nearest_boundary(coords_c1,gr)) continue;
		l1s = l1e = d3 = 0.0;
		for (i = 0; i < dim; ++i)
		{
			l1s += fabs(coords[i] - Coords(ps)[i]);
			l1e += fabs(coords[i] - Coords(pe)[i]);
			d3 += fabs(Coords(pe)[i] - Coords(ps)[i]);
		}
		d1 = min(l1s, l1e);
		d2 = max(l1s, l1e);
		new_residual = d1 + d2 - d3;
		if (!*c || new_residual < residual) 
		{
			*c = *c1;
			residual = new_residual;
		}
		*comp = (is_interior_comp(negative_component(*c),intfc)) ?
			 negative_component(*c): positive_component(*c);
	}

	if ((*c) == NULL) return;

	/*
	*	This function is called as part of a series of
	*	modifications to the interface, so it is a
	*	waste of time to generate a new bond_comp list.
	*	Thus use long_nearest_interface_point() if
	*	the interface is modified.
	*/

	if (intfc->modified)
	{
	    if (long_nearest_interface_point(coords,*comp,intfc,
					     INCLUDE_BOUNDARIES,
					     Hyper_surf(*c),coords_c1,t,
					     &hse,&hs2) != YES)
	    {
		screen("ERROR in nearest_boundary_curve(), "
		       "long_nearest_interface_point() failed\n");
		clean_up(ERROR);
	    }
	}
	else
	{
	    if (nearest_interface_point(coords,*comp,intfc,INCLUDE_BOUNDARIES,
					Hyper_surf(*c),coords_c1,t,
					&hse,&hs2) != YES)
	    {
		screen("ERROR in nearest_boundary_curve(), "
		       "nearest_interface_point() failed\n");
		clean_up(ERROR);
	    }
	}
	*b = Bond_of_hse(hse);
}		/*end nearest_boundary_curve*/


/*
*		nearest_linked_bdry_like_curve():
*
*	Given node point p on bond b curve c, this routine finds
*	the nearest (in terms of angular distance) boundary like
*	curve of given w_type that is joined to p.
*	It make no assumptions that the boundary curves must be
*	vertical or horizontal.
*/

LOCAL	void nearest_linked_bdry_like_curve(
	FT_POINT		*p,
	BOND		*b,
	CURVE		*c,
	double		*bt,
	BOND		**bb,
	CURVE		**bc,
	int		wv_type)
{
	O_NODE		*ond;
	CURVE		*tc;
	int		i, hld_i;
	double		c_ang, del_ang, min_ang;

	*bc = NULL;		*bb = NULL;
	if ((wave_type(c) == wv_type) && (is_bdry_like_curve(c)))
	{
		*bc = c;	*bb = b;
		if      (p == b->start) *bt = 0.0;
		else if (p == b->end  ) *bt = 1.0;
		else
		{
			*bt = hypot(Coords(p)[0] - Coords(b->start)[0],
				    Coords(p)[1] - Coords(b->start)[1]) /
							bond_length(b);
		}
		return;
	}

	if      (p == c->start->posn) ond = make_onode(c->start);
	else if (p == c->end->posn  ) ond = make_onode(c->end);
	else return;

		/* find boundary curve of correct wave_type */
		/*   having closest angular distance to c   */

	for (i = 0;  i < ond->num_c;  ++i)
	{
		if (ond->nc[i] == c) break;
	}
	c_ang = ond->ang[i];


	hld_i = -1;		min_ang = 2.1 * PI;
	for (i = 0;  i < ond->num_c;  ++i)
	{
		tc = ond->nc[i];
		if (tc == c ) continue;
		if (wave_type(tc) != wv_type)   continue;
		if (!is_bdry_like_curve(tc)) continue;
		del_ang = fabs(ond->ang[i] - c_ang);
		if (del_ang < min_ang)
		{
			min_ang = del_ang;
			hld_i = i;
		}
	}
	if (hld_i >= 0)
	{
		*bc = ond->nc[hld_i];
		if (ond->orient[hld_i] == POSITIVE_ORIENTATION)
		{
			*bb = (*bc)->first;
			*bt = 0.0;
		}
		else
		{
			*bb = (*bc)->last;
			*bt = 1.0;
		}
	}
}		/*end nearest_linked_bdry_like_curve*/


/*
*			f_boundary_untangle():
*
*	This is the principal routine for resolving intersections
*	between boundary and interior curves. In the case of a
*	Dirichlet boundary, the portion of the interior curve which
*	lies on the exterior side of the boundary is removed.
*	In the case of a Neumann boundary the
*	exterior portion is removed and then a physics routine must
*	determine where to place a reflected wave, or whether there is
*	to be a reflected wave.
*	The exterior side of the boundary curve is defined as follows:
*	If the boundary curve (defined as such by its wave_type) is 
*	on the boundary of the computational domain
*	(ie if is_bdry(curve) ) then the exterior side is the
*	side whose component is the exterior_component of the interface.
*	Otherwise a line from the boundary curve to the boundary of the
*	computational domain is drawn.
*	Each crossing of this line by a nonpassive boundary curve reflects a 
*	change from interior to exterior.  Here we make the convention that 
*	each boundary curve has one interior side and one exterior side,
*	except for passive curves, which have two exterior sides.
*/


/*ARGSUSED*/
EXPORT int f_boundary_untangle(
	Front		*front,
	CROSS		**cross,
	RPROBLEM	*rp,
	NODE		*n_shift,
	int		flag)
{
	CROSS		*cr, *crprev = NULL;
	CROSS		*crx, *crx_prev;
	CURVE		*cexterior,*cinterior;
	CURVE		*bdrycurves[2];
	ORIENTATION	cphys_orient;
	ANGLE_DIRECTION	cb_to_cp_dir;
	int		i, num_cr;
	int		*is_irregular_bdry_cross;
	int		status;
	COMPONENT	bdryncomp[2], bdrypcomp[2];
	int		deb_fbu = debugging("f_boundary_untangle");

	debug_print("f_boundary_untangle","Entered f_boundary_untangle()\n");
	if (deb_fbu)
	{
	    (void) printf("Cross list for f_boundary_untangle()\n");
	    print_cross_list(*cross);
	}
	
	if (!cross || !*cross)
	{
	    debug_print("f_boundary_untangle","Left f_boundary_untangle()\n");
	    return CURVES_UNTANGLED;
	}
	start_clock("f_boundary_untangle");
	if (deb_fbu && rp)
	{
	    (void) printf("Old interface\n");
	    print_interface(rp->old_intfc);
	    (void) printf("New interface\n");
	    print_interface(rp->new_intfc);
	}

	for (cr = *cross, num_cr = 0; cr; cr = cr->next)
	{
	    if (is_passive_boundary(cr->c1) || is_passive_boundary(cr->c2)) 
	    {
	        if (*cross = cr)
		    *cross = cr->next;
	    	delete_from_cross_list(cr);
	    }
	    else
	    	++num_cr;
	}
	if (num_cr == 0)
	{
	    *cross = NULL;
	    stop_clock("f_boundary_untangle");
	    debug_print("f_boundary_untangle","Left f_boundary_untangle()\n");
	    return CURVES_UNTANGLED;
	}
	if (deb_fbu)
	{
	    (void) printf("After is_passive_boundary test\n");
	    print_cross_list(*cross);
	}
	uni_array(&is_irregular_bdry_cross,num_cr,INT);
	classify_bdry_crosses(*cross,is_irregular_bdry_cross);
	if (deb_fbu)
	{
	    (void) printf("After classify_boundary_crosses\n");
	    print_cross_list(*cross);
	}
	for  (i = 0, cr = *cross; cr; ++i, cr = cr->next)
	{
	    if (deb_fbu)
	    {
	    	(void) printf("At head of main for loop\n");
	    	print_cross_list(*cross);
		(void) printf("cr - %p\n",(POINTER)cr);
	    }

	    if (!is_bdry_interior_cross(front,cr,&cphys_orient,
					   &cb_to_cp_dir,bdryncomp,bdrypcomp)) 
	    {
	    	if (deb_fbu)
	    	    (void) printf("was NOT bdry_interior_cross\n");
		crprev = cr;
		continue;
	    }

	    if (deb_fbu) 
	    {
	    	print_orientation("cphys_orient =",cphys_orient,"\n");
	    	(void) printf("bdrycomp: l0 %d r0 %d ",
			      bdryncomp[0],bdrypcomp[0]);
		(void) printf("l1 %d r1 %d\n",bdryncomp[1],bdrypcomp[1]);
		(void) printf("is_irregular_bdry_cross[%d] = %s\n",i,
			      (is_irregular_bdry_cross[i] == YES) ? "YES" :
			      ((is_irregular_bdry_cross[i] == NO) ? "NO" :
			      "ERROR"));
	    }

	    	/* Delete cr from cross list */

	    if (crprev)
		crprev->next = cr->next;
	    else
		*cross = cr->next;

	    if (cr->next)
		cr->next->prev = crprev ? crprev : NULL;


		/* Split curves */

	    if (!split_curves_at_bdry_cross(cr,front,cphys_orient,
					       cb_to_cp_dir,&cexterior,
					       &cinterior,bdrycurves,
					       bdryncomp,bdrypcomp,
					       is_irregular_bdry_cross[i],rp))
	    {
		(void) printf("WARNING in f_boundary_untangle(), "
		              "split_curves_at_bdry_cross() failed\n");
		free(is_irregular_bdry_cross);
		stop_clock("f_boundary_untangle");
		debug_print("f_boundary_untangle","Left f_boundary_untangle()\n");
		return ERROR_IN_UNTANGLE;
	    }

	    	/* Delete or move exterior curve */

	    if (deb_fbu)
	    	(void) printf("Start delete exterior curve\n");
	    status = modify_exterior_curve(cinterior,cexterior,
	    			           bdrycurves,cphys_orient,
					   cb_to_cp_dir,n_shift,rp,cr,
					   is_irregular_bdry_cross[i],front);
		
	    if (status != CURVES_UNTANGLED)
	    {
	    	if (deb_fbu)
	    	{
	    	    (void) printf("modify_exterior_curve returns ");
		    print_untangle_status(status);
		}
		if (status == ERROR_IN_UNTANGLE)
		{
		    (void) printf("WARNING in f_boundary_untangle(),  "
				  "modify_exterior_curve() failed ");
		}
		free(is_irregular_bdry_cross);
		stop_clock("f_boundary_untangle");
		debug_print("f_boundary_untangle","Left f_boundary_untangle()\n");
		return status;
	    }
	}
	free(is_irregular_bdry_cross);

		/*    It is entirely possible that    */
		/*   intersections() has found other  */
		/*   crosses on cexterior that are    */
		/* not bdry-interior crosses AND are  */
		/* 	NOT in the interior !!!	      */
		/* These crosses must be removed from */
		/*	    the cross list.	      */
		/* Because of the ++i counter in the  */
		/* main controlling loop of this code */
		/* it is impossible to do this inside */
		/*          the main loop.	      */
		/* A correction is introduced before  */
		/*	    this routine returns      */

	if (deb_fbu)
	{
	    (void) printf("Cross list before delete_curve correction\n");
	    print_cross_list(*cross);
	}
	crx_prev = NULL;	crx = *cross;
	while (crx)
	{
	    if (is_c_on_intfc(crx->c1) && is_c_on_intfc(crx->c2))
	    {
	    	crx_prev = crx;		crx = crx->next;
	    }
	    else
	    {
       	    	if (deb_fbu)
	       	    (void) printf("removing cross %p  crx_prev %p\n",
				  (POINTER)crx,(POINTER)crx_prev);

		if (crx->next)
		    crx->next->prev = crx_prev ? crx_prev : NULL;

		if (crx_prev)
		{
		    crx_prev->next = crx->next;
		    crx = crx->next;
		}
		else
		{
		    *cross = crx->next;
		    crx = *cross;
		}
	    }
	}
	if (deb_fbu)
	{
	    (void) printf("Cross list after delete_curve correction\n");
	    print_cross_list(*cross);
	    (void) printf("Interface after f_boundary_untangle()\n");
	    print_interface(front->interf);
	    (void) printf("Periodic continuations after "
	                  "f_boundary_untangle()\n");
	    print_correspond_hyper_surf_list(front->interf);
	}
	stop_clock("f_boundary_untangle");
	debug_print("f_boundary_untangle","Left f_boundary_untangle()\n");
	return CURVES_UNTANGLED;
}		/*end f_boundary_untangle*/

/*ARGSUSED*/
EXPORT	int modify_exterior_curve(
	CURVE		*cinterior,
	CURVE		*cexterior,
	CURVE		**bdrycurves,
	ORIENTATION	cphys_orient,
	ANGLE_DIRECTION	cb_to_cp_dir,
	NODE		*n_shift,
	RPROBLEM	*rp,
	CROSS		*cr,
	int		is_irreg_bdry_crx,
	Front		*front)
{
	NODE		*oppn;
	int		cext_in_bdry_cr;
	int		status;

	debug_print("f_boundary_untangle","Entered modify_exterior_curve()\n");

	oppn = Node_of(cexterior,Opposite_orient(cphys_orient));
	cext_in_bdry_cr = is_c_in_another_bdry_cr(cr->next,cexterior);
	if (!cext_in_bdry_cr) 
	{
	    if ((wave_type(cr->c2) == NEUMANN_BOUNDARY) &&
	        (wave_type(cinterior) >= FIRST_VECTOR_PHYSICS_WAVE_TYPE))
	    {
	        if (front->fr_vec_bdry_untangle == NULL)
	        {
	            /* reflected wave needed */
	            screen("ERROR in modify_exterior_curve(), "
	                   "reflected wave code needed\n");
	            clean_up(ERROR);
	        }
	        else
	        {
	            status = (*front->fr_vec_bdry_untangle)(cinterior,
	        	                                    cexterior,
							    bdrycurves,
	        	                                    cphys_orient,
							    cb_to_cp_dir,
	        	                                    is_irreg_bdry_crx,
							    front);

	    	    if (debugging("f_boundary_untangle"))
		    {
		        (void) printf("fr_vec_bdry_untangle() returns ");
			print_untangle_status(status);
		    }
		    if (status != CURVES_UNTANGLED)
		    {
		        if (status == ERROR_IN_UNTANGLE)
		        {
		            (void) printf("WARNING in modify_exterior_curve(), "
		                          "fr_vec_bdry_untangle failed \n");
		        }
			debug_print("f_boundary_untangle",
			      "Left modify_exterior_curve()\n");
		    	return status;
		    }
		}
	    }
	    else
	    {
		(void) delete_curve(cexterior);
		if (front->phys_set_node_types)
		    (*front->phys_set_node_types)(front);
	    }
	}
	if (!is_curve_in_cross_list(cr->next,cinterior) &&
	                              (wave_type(cr->c2) == PASSIVE_BOUNDARY))
	    (void) delete_curve(cinterior);

		/* Remove possible redundant node */

	if (is_irreg_bdry_crx && (oppn->interface != NULL) &&
	                          (node_type(oppn) < FIRST_PHYSICS_NODE_TYPE))
	    (void) delete_redundant_node(oppn,cr,rp,front);

	debug_print("f_boundary_untangle","Left modify_exterior_curve()\n");
	return CURVES_UNTANGLED;
}		/*end modify_exterior_curve*/

EXPORT	int is_c_in_another_bdry_cr(
	CROSS		*cross,
	CURVE		*c)
{
	CROSS		*cr;

	for (cr = cross;  cr;  cr = cr->next)
	{
	    if ((cr->c1 == c) || (cr->c2 == c))
	    {
	    	if (interior_curve_crosses_boundary(cr) == YES)
	    	    return YES;
	    }
	}
	return NO;
}		/*end is_c_in_another_bdry*/


EXPORT int split_curves_at_bdry_cross(
	CROSS		*cr,
	Front		*front,
	ORIENTATION	cphys_orient,
	ANGLE_DIRECTION	cb_to_cp_dir,
	CURVE		**cexterior,
	CURVE		**cinterior,
	CURVE		**bdrycurves,
	COMPONENT	*bdryncomp,
	COMPONENT	*bdrypcomp,
	int		set_opp_bdry_node_states,
	RPROBLEM	*rp)
{
	NODE		*node;
	CURVE		*physcurves[2];
	int		deb_fbu = debugging("f_boundary_untangle");

	debug_print("f_boundary_untangle","Entered split_curves_at_bdry_cross()\n");

		/* Split curves */

	if  (deb_fbu)
	{
	    (void) printf("Before split\n");

	    (void) printf("Physical curve %llu\n",curve_number(cr->c1));
	    if (debugging("curve_states"))
	    	show_curve_states(cr->c1);
	    else
	    	print_curve(cr->c1);

	    (void) printf("Boundary curve %llu\n",curve_number(cr->c2));
	    if (debugging("curve_states"))
	    	show_curve_states(cr->c2);
	    else
	    	print_curve(cr->c2);
	    print_correspond_hyper_surf_list(front->interf);
	}

	split_curves_at_cross(cr,front,&node,physcurves,NULL,NULL,bdrycurves,
		              bdryncomp,bdrypcomp,0.0,(POINTER)rp);

	if (deb_fbu && front->sizest) 
	{
	    (void) printf("Curves from phys split:\n");
	    print_curve(physcurves[0]);
	    print_curve(physcurves[1]);
	    if (debugging("curve_states"))
	    {
	    	show_curve_states(physcurves[0]);
	    	show_curve_states(physcurves[1]);
	    }
	    (void) printf("States at common node of physcurves:\n");
	    (void) printf("Left end state physcurves[0]:\n");
	    (*front->print_state)(left_end_state(physcurves[0]));
	    (void) printf("Right end state physcurves[0]:\n");
	    (*front->print_state)(right_end_state(physcurves[0]));
	    (void) printf("Left start state physcurves[1]:\n");
	    (*front->print_state)(left_start_state(physcurves[1]));
	    (void) printf("Right start state physcurves[1]:\n");
	    (*front->print_state)(right_start_state(physcurves[1]));
	    (void) printf("Curves from bdry split:\n");
	    print_curve(bdrycurves[0]);
	    print_curve(bdrycurves[1]);
	    if (debugging("curve_states"))
	    {
		show_curve_states(bdrycurves[0]);
	        show_curve_states(bdrycurves[1]);
	    }
	    print_correspond_hyper_surf_list(front->interf);
	}

		/* Set curve flags */

	node_type(node) = bdry_node_type(wave_type(cr->c2));
	if (node_type(node) == UNKNOWN_NODE_TYPE)
	{
	    (void) printf("WARNING in split_curves_at_bdry_cross(), "
	                  "can't set boundary node type\n");
	    debug_print("f_boundary_untangle","Left split_curves_at_bdry_cross()\n");
	    return NO;
	}
		/* Move bdry nodes to bdry */

	if (is_bdry(node))
	    nearest_boundary_point(Coords(node->posn),Coords(node->posn),
			           front->rect_grid);

		/* Set physics dependent curve flags */
	if (front->phys_split_bdry_cross)
	    (*front->phys_split_bdry_cross)(physcurves,bdrycurves);

	if (deb_fbu)
	    (void) printf("After start/end status\n");

		/* Set boundary states */

	bstate_index(bdrycurves[0]) = bstate_index(cr->c2);
	bstate_index(bdrycurves[1]) = bstate_index(cr->c2);
	if (front->sizest)
	{
	    if (!assign_states_of_boundary_cross(front,cr,cphys_orient,
			                        cb_to_cp_dir,physcurves,
						bdrycurves,
			                        set_opp_bdry_node_states,rp))
	    {
	    	(void) printf("WARNING in split_curves_at_bdry_cross(), "
	                  "assign_states_of_bdry_cross() returns NO\n");
	    	return NO;
	    }
	}

	    /* Check for and eliminate redundant CLOSED nodes on physcurves */

	if (physcurves[0] && node_type(physcurves[0]->start) == CLOSED_NODE)
	    eliminate_redundant_closed_node_at_bdry_tangle(
		cr,physcurves,POSITIVE_ORIENTATION);
	if (physcurves[1] && node_type(physcurves[1]->end) == CLOSED_NODE)
	    eliminate_redundant_closed_node_at_bdry_tangle(
		cr,physcurves,NEGATIVE_ORIENTATION);

	if (cphys_orient == POSITIVE_ORIENTATION)
	{
	    *cexterior = physcurves[1];
	    *cinterior = physcurves[0];
	}
	else
	{
	    *cexterior = physcurves[0];
	    *cinterior = physcurves[1];
	}

	if (deb_fbu)
	    (void) printf("cext %llu  cint %llu\n",
			  curve_number(*cexterior),curve_number(*cinterior));
	debug_print("f_boundary_untangle","Left split_curves_at_bdry_cross()\n");
	return YES;
}		/*end split_curves_at_bdry_cross*/

/*
*			is_bdry_interior_cross():
*
*	Determines if a cross comes from the crossing of a physical
*	with a boundary type curve (as defined by their wave types).
*	If the answer is yes, the variable cphys_orient is set to the
*	orientation of the physical curve when directed from interior
*	to exterior. Also cross->c1 is reset to the physical curve and 
*	cross->c2 is reset to the boundary curve in this case.
*/

EXPORT int is_bdry_interior_cross(
	Front		*front,
	CROSS		*cross,
	ORIENTATION	*cphys_orient,
	ANGLE_DIRECTION	*cb_to_cp_dir,
	COMPONENT	*bdryncomp,
	COMPONENT	*bdrypcomp)
{
	double		cp;
	SIDE		side;
	int		dim = front->rect_grid->dim;
	COMPONENT	comp0,comp1;
	int		deb_ibi = debugging("is_bdry_interior");

	debug_print("is_bdry_interior","Entered is_bdry_interior_cross()\n");
	if (!cross)
	{
	    if (deb_ibi)
		(void) printf("cross == NULL\n");
	    debug_print("is_bdry_interior","Left is_bdry_interior_cross()\n");
	    return NO;
	}

	if (interior_curve_crosses_boundary(cross) == NO)
	{
	    if (deb_ibi)
		(void) printf("interior_curve_crosses_boundary returns NO\n");
	    debug_print("is_bdry_interior","Left is_bdry_interior_cross()\n");
	    return NO;
	}

	if (debugging("is_bdry_interior"))
	    (void) printf("Bdry interior cross found\n");
	vector_product_on_bonds(cross->b2,cross->b1,dim,&cp);
	*cb_to_cp_dir = (cp > 0.0) ? COUNTER_CLOCK : CLOCKWISE;
	if (*cb_to_cp_dir == COUNTER_CLOCK)
	{
	    comp0 = negative_component(cross->c1);
	    comp1 = positive_component(cross->c1);
	}
	else 
	{
	    comp1 = negative_component(cross->c1);
	    comp0 = positive_component(cross->c1);
	}
	side = physical_side_of_bdry_curve(cross->c2);
	if (side == POSITIVE_SIDE) 
	{
	    bdryncomp[0] = bdryncomp[1] = negative_component(cross->c2);
	    bdrypcomp[0] = comp0;
	    bdrypcomp[1] = comp1;
	}
	else 
	{
	    bdrypcomp[0] = bdrypcomp[1] = positive_component(cross->c2);
	    bdryncomp[0] = comp0;
	    bdryncomp[1] = comp1;
	}
	*cphys_orient = ((*cb_to_cp_dir==COUNTER_CLOCK && side==POSITIVE_SIDE)
			 ||
			 (*cb_to_cp_dir==CLOCKWISE && side==NEGATIVE_SIDE)) ?
				 POSITIVE_ORIENTATION : NEGATIVE_ORIENTATION;
	debug_print("is_bdry_interior","Left is_bdry_interior_cross()\n");
	return YES;
}		/*end is_bdry_interior_cross*/

LOCAL int assign_states_of_boundary_cross(
	Front		*fr,
	CROSS		*cross,
	ORIENTATION	cphys_orient,
	ANGLE_DIRECTION	cb_to_cp_dir,
	CURVE		**physcurves,
	CURVE		**bdrycurves,
	int		set_opp_bdry_node_states,
	RPROBLEM	*rp)
{
	SIDE		cbdry_side;	/* physical side of bdry curve */
	SIDE		cphys_side;	/* newbdry side of physical curve */
	ORIENTATION	newbdry_orient;
	NODE		*node,*bnode0,*bnode1;
	CURVE		*cexterior,*cinterior;
	CURVE		*newbdry;
	Locstate	s0,s1;

	debug_print("assign_states","Entered assign_states_of_boundary_cross()\n");
	cbdry_side = physical_side_of_bdry_curve(cross->c2);

	if (cphys_orient == POSITIVE_ORIENTATION)
	{
	    cexterior = physcurves[1];
	    cinterior = physcurves[0];
	}
	else
	{
	    cexterior = physcurves[0];
	    cinterior = physcurves[1];
	}
	if (debugging("assign_states"))
	{
	    (void) printf("Data into assign_states_of_boundary_cross()\n");
	    (void) printf("Left_state_at_node(cexterior,cphys_orient)\n");
	    (*fr->print_state)(Left_state_at_node(cexterior,cphys_orient));
	    (void) printf("Right_state_at_node(cexterior,cphys_orient)\n");
	    (*fr->print_state)(Right_state_at_node(cexterior,cphys_orient));
	    (void) printf("Left_state_at_node(cinterior,");
	    (void) printf("Opposite_orient(cphys_orient))\n");
	    (*fr->print_state)(Left_state_at_node(cinterior,
					      Opposite_orient(cphys_orient)));
	    (void) printf("Right_state_at_node(cinterior,");
	    (void) printf("Opposite_orient(cphys_orient))\n");
	    (*fr->print_state)(Right_state_at_node(cinterior,
					       Opposite_orient(cphys_orient)));
	}
	if (cb_to_cp_dir == COUNTER_CLOCK)
	{
	    s0 =  Left_state_at_node(cinterior,Opposite_orient(cphys_orient));
	    s1 = Right_state_at_node(cinterior,Opposite_orient(cphys_orient));
	}
	else 
	{
	    s1 =  Left_state_at_node(cinterior,Opposite_orient(cphys_orient));
	    s0 = Right_state_at_node(cinterior,Opposite_orient(cphys_orient));
	}
	map_states_at_node(s0,bdrycurves[0],NEGATIVE_ORIENTATION,cbdry_side,fr);
	map_states_at_node(s1,bdrycurves[1],POSITIVE_ORIENTATION,cbdry_side,fr);

	if (debugging("assign_states")) 
	{
	    int i;

	    print_angle_direction("cb_to_cp_dir = ",cb_to_cp_dir,"\n");
	    print_side("cbdry_side = ",cbdry_side,"\n");
	    print_orientation("cphys_orient = ",cphys_orient,"\n");
	    (void) printf("cext %llu  cint %llu\n",
			  curve_number(cexterior),curve_number(cinterior));
	    (void) printf("s0:  ");	(*fr->print_state)(s0);
	    (void) printf("s1:  ");	(*fr->print_state)(s1);
	    (void) printf("\n");

	    for (i = 0;  i < 2;  ++i)
	    {
	    	(void) printf("physcurve[%d]:\n",i);
	    	print_curve(physcurves[i]);
	    	if (debugging("curve_states"))
	    	    show_curve_states(physcurves[i]);

		(void) printf("bdrycurve[%d]:\n",i);
		print_curve(bdrycurves[i]);
		if (debugging("curve_states"))
		    show_curve_states(bdrycurves[i]);
		}
	}
	if (is_curve_in_cross_list(cross->next,cexterior)) 
	{
	    if (debugging("assign_states"))
		(void) printf("cexterior is in cross list\n");
	    debug_print("assign_states","Left assign_states_of_boundary_cross()\n");
	    return YES;
	}

		/*
		* The code for the rp case is correct only for
		* short null boundary curves.
		* TO DO: Do this correctly for the general case
		*/
	if (rp)
	{
	    RP_NODE *rpn;

	    if (debugging("assign_states"))
	    	(void) printf("Rproblem untangle\n");
	    if (!is_curve_in_cross_list(cross->next,bdrycurves[0]) &&
	    			is_null_curve(bdrycurves[0],rp))
	    	delete_interior_points_of_curve(fr,bdrycurves[0]);

	    if (!is_curve_in_cross_list(cross->next,bdrycurves[1]) &&
	    			is_null_curve(bdrycurves[1],rp))
	    	delete_interior_points_of_curve(fr,bdrycurves[1]);

	    if (!rp_node_with_node(&rpn,rp,bdrycurves[0]->end))
	    {
	        screen("ERROR in assign_states_of_boundary_cross() "
		       "rp_node_with_node failed\n");
		clean_up(ERROR);
	    }
	    rpn->states_assigned_at_node = YES;
	    debug_print("assign_states","Left assign_states_of_boundary_cross()\n");
	    return YES;
	}
		/* Set newbdry */

	node = Node_of(cexterior,Opposite_orient(cphys_orient));
	bnode0 = bdrycurves[0]->start;
	bnode1 = bdrycurves[1]->end;
	if (bnode0 == node && bnode1 != node)
	{
	    newbdry = bdrycurves[0];
	    newbdry_orient = NEGATIVE_ORIENTATION;/* Oriented from cross */
	}
	else if (bnode1 == node && bnode0 != node)
	{
	    newbdry = bdrycurves[1];
	    newbdry_orient = POSITIVE_ORIENTATION;/* Oriented from cross */
	}
	else
	{
	    bnode0 = opp_node_of_gen_curve(bdrycurves[0],NEGATIVE_ORIENTATION);
	    bnode1 = opp_node_of_gen_curve(bdrycurves[1],POSITIVE_ORIENTATION);
	    if (bnode0 == node && bnode1 == node)
	    {
		double area0,area1;
		area0 = f_area_of_loop(cexterior,cphys_orient,bdrycurves[0]);
		area1 = f_area_of_loop(cexterior,cphys_orient,bdrycurves[1]);

		    /*TODO: should check for small enough area */
		if (area1 > area0) 
		{
		    newbdry = bdrycurves[0];
		    newbdry_orient = NEGATIVE_ORIENTATION;
		    			/* Oriented from cross */
		    bnode1 = NULL;
		}
		else 
		{
		    newbdry = bdrycurves[1];
		    newbdry_orient = POSITIVE_ORIENTATION;	
		    			/* Oriented from cross */
		    bnode0 = NULL;
		}
	    }
	    else if (bnode0 == node)
	    {
		newbdry = bdrycurves[0];
		newbdry_orient = NEGATIVE_ORIENTATION;
						/* Oriented from cross */
	    }
	    else if (bnode1 == node)
	    {
		newbdry = bdrycurves[1];
		newbdry_orient = POSITIVE_ORIENTATION;	
						/* Oriented from cross */
	    }
	    else 
	    {
		(void) printf("WARNING in assign_states_of_boundary_cross(), ");
		(void) printf("unable to find boundary-physical curve loop ");
		return NO;
	    }
	}
	if (debugging("assign_states")) 
	{
	    (void) printf("newbdry:\n");
	    print_curve(newbdry);
	    (void) printf("node bdrycurves[0]->start ...[1]->end:\n");
	    print_node(node);
	    print_node(bdrycurves[0]->start);
	    print_node(bdrycurves[1]->end);
	}

	cphys_side = ((cb_to_cp_dir == CLOCKWISE &&     bnode0 == node) ||
		      (cb_to_cp_dir == COUNTER_CLOCK && bnode1 == node)) ?
			      POSITIVE_SIDE : NEGATIVE_SIDE;

	if (debugging("assign_states"))
	{
	    (void) printf("Data before map_phys_cur_states_to_bdry_cur() in ");
	    (void) printf("assign_states_of_boundary_cross()\n");
	    (void) printf("Left_state_at_node(cexterior,cphys_orient)\n");
	    (*fr->print_state)(Left_state_at_node(cexterior,cphys_orient));
	    (void) printf("Right_state_at_node(cexterior,cphys_orient)\n");
	    (*fr->print_state)(Right_state_at_node(cexterior,
	    				       cphys_orient));
	    (void) printf("Left_state_at_node(cinterior,");
	    (void) printf("Opposite_orient(cphys_orient))\n");
	    (*fr->print_state)(Left_state_at_node(cinterior,
	    			      Opposite_orient(cphys_orient)));
	    (void) printf("Right_state_at_node(cinterior,");
	    (void) printf("Opposite_orient(cphys_orient))\n");
	    (*fr->print_state)(Right_state_at_node(cinterior,
	    			       Opposite_orient(cphys_orient)));
	}
	map_phys_cur_states_to_bdry_cur(cexterior,cphys_orient,
		cphys_side,newbdry,newbdry_orient,cbdry_side,
		NO,set_opp_bdry_node_states,fr->interf,fr);

	if (debugging("assign_states"))
	{
	    (void) printf("Data out of assign_states_of_boundary_cross()\n");
	    (void) printf("Left_state_at_node(cexterior,cphys_orient)\n");
	    (*fr->print_state)(Left_state_at_node(cexterior,cphys_orient));
	    (void) printf("Right_state_at_node(cexterior,cphys_orient)\n");
	    (*fr->print_state)(Right_state_at_node(cexterior,cphys_orient));
	    (void) printf("Left_state_at_node(cinterior,");
	    (void) printf("Opposite_orient(cphys_orient))\n");
	    (*fr->print_state)(Left_state_at_node(cinterior,
					      Opposite_orient(cphys_orient)));
	    (void) printf("Right_state_at_node(cinterior,");
	    (void) printf("Opposite_orient(cphys_orient))\n");
	    (*fr->print_state)(Right_state_at_node(cinterior,
					       Opposite_orient(cphys_orient)));
	}
	debug_print("assign_states","Left assign_states_of_boundary_cross()\n");
	return YES;
}		/*end assign_states_of_boundary_cross*/

/*
*			map_phys_cur_states_to_bdry_cur():
*
*	Maps states and components from the cphys_side of a physical curve to  
*	cbdry_side of a (generalized) boundary curve.  The curves are assumed 
*	to meet at a common node with orientation cphys_orient and cbdry_orient.
*	The states at the end nodes of the boundary curve are set depending on 
*	the flags set_bnode_sts (common node states) and set_opp_bnode_sts. 
*
*	States on both sides of DIRICHLET boundary curves are changed.
*	States on SUBDOMAIN boundaries are unchanged.
*/

EXPORT	void map_phys_cur_states_to_bdry_cur(
	CURVE		*cphys,
	ORIENTATION	cphys_orient,
	SIDE		cphys_side,
	CURVE		*cbdry,
	ORIENTATION	cbdry_orient,
	SIDE		cbdry_side,
	int		set_bnode_sts,
	int		set_opp_bnode_sts,
	INTERFACE	*intfc,
	Front		*fr)
{
	HYPER_SURF	*hs;
	HYPER_SURF_ELEMENT *hse;
	BOND		*b, *B;
	CURVE		*C;
	CURVE		*bcur,*nextc;
	NODE		*node = NULL;
	int		n_type = UNKNOWN_NODE_TYPE;
	ORIENTATION	cbdry_opp_orient=Opposite_orient(cbdry_orient);
	Locstate	sphys, sext;
	COMPONENT	comp;
	double		coords[MAXD], t;
	static Locstate	phys_state = NULL;

	debug_print("map_p_sts","Entered map_phys_cur_states_to_bdry_cur()\n");

	if (wave_type(cbdry) == SUBDOMAIN_BOUNDARY)
	    return;
	if (phys_state == NULL) 
	{
	    alloc_state(fr->interf,&phys_state,fr->sizest);
	}

	if (debugging("map_p_sts"))
	{
	    (void) printf("states on physical curve %llu\n",curve_number(cphys));
	    show_curve_states(cphys);

	    debug_show_boundary_curve_states("before mapping", 
			cbdry,cbdry_orient);
	}
	if (next_curve_of_gen_curve(cphys,cphys_orient,&n_type,&node) != NULL)
	{
	    screen("ERROR in map_phys_cur_states_to_bdry_cur(), "
	           "cphys %llu is a generalized curve ",curve_number(cphys));
	    clean_up(ERROR);
	}

	comp = (cphys_side == NEGATIVE_SIDE) ? negative_component(cphys) :
				               positive_component(cphys);

	bcur = cbdry;
	nextc = NULL;
	n_type = UNKNOWN_NODE_TYPE;
	while (nextc != cbdry)
	{
	    if (cbdry_side == NEGATIVE_SIDE)
		negative_component(bcur) = comp;
	    else
		positive_component(bcur) = comp;

	    for (b = bcur->first; b->next; b = b->next) 
	    {
	    	if (cbdry_side == NEGATIVE_SIDE) 
	    	{
	    	    sphys = left_state(b->end);
	    	    sext = right_state(b->end);
	    	}
	    	else 
	    	{
	    	    sphys = right_state(b->end);
	    	    sext = left_state(b->end);
	    	}

		/* assign state on cphys_side of cphys to sphys*/

		if (long_nearest_interface_point(Coords(b->end),
						 comp,intfc,NO_BOUNDARIES,
			                         Hyper_surf(cphys),
						 coords,&t,&hse,&hs) != YES)
	        {
		    screen("ERROR in map_phys_cur_states_to_bdry_cur(), "
		           "long_nearest_interface_point() failed\n");
		    clean_up(ERROR);
	        }
		B = Bond_of_hse(hse);
		C = Curve_of_hs(hs);

		if (cphys_side == NEGATIVE_SIDE) 
		    left_state_along_bond(t,B,C,sphys);
		else			
		    right_state_along_bond(t,B,C,sphys);

		if (debugging("map_p_sts")) 
		{
		    (void) printf("comp %d  ",comp);
		    print_side("cphys_side is ",cphys_side,"\n");
		    (void) printf("sphys:  ");
		    (*fr->print_state)(sphys);
		    (void) printf("sext:  ");
		    (*fr->print_state)(sext);
		    (void) printf("\n");
		}
	    }

	    nextc = next_curve_of_gen_curve(bcur,cbdry_orient,&n_type,&node);
	    if (!nextc) /* end of gen curve */
	        break;

	    /* boundary curve continues - map states at interior node */
		
	    if (long_nearest_interface_point(Coords(node->posn),comp,intfc,
					     NO_BOUNDARIES,Hyper_surf(cphys),
					     coords,&t,&hse,&hs) != YES)
	    {
		screen("ERROR in map_phys_cur_states_to_bdry_cur(), "
		       "long_nearest_interface_point() failed\n");
		clean_up(ERROR);
	    }
	    B = Bond_of_hse(hse);
	    C = Curve_of_hs(hs);

	    if (cphys_side == NEGATIVE_SIDE)
	    	left_state_along_bond(t,B,C,phys_state);
	    else
	    	right_state_along_bond(t,B,C,phys_state);

	    map_states_at_node(phys_state,bcur,cbdry_opp_orient,cbdry_side,fr);
	    map_states_at_node(phys_state,nextc,cbdry_orient,cbdry_side,fr);

	    bcur = nextc;

	}

	if (set_bnode_sts)
	{
	    if (debugging("map_p_sts")) 
	    	(void) printf("Setting states at Node\n");

	    sphys = (cphys_side == NEGATIVE_SIDE) ?
	    		Left_state_at_node(cphys,cphys_orient) :
	    		Right_state_at_node(cphys,cphys_orient);
	    map_states_at_node(sphys,cbdry,cbdry_orient,cbdry_side,fr);

	}

	if (set_opp_bnode_sts)
	{
		ORIENTATION cphys_opp_orient = Opposite_orient(cphys_orient);
		if (debugging("map_p_sts")) 
			(void) printf("Setting states at Opp Node\n");

		sphys = (cphys_side == NEGATIVE_SIDE) ?
		      Left_state_at_node(cphys,cphys_opp_orient) :
		      Right_state_at_node(cphys,cphys_opp_orient);
		map_states_at_node(sphys,bcur,cbdry_opp_orient,cbdry_side,fr);
	}

	if (debugging("map_p_sts"))
	{
		debug_show_boundary_curve_states("after mapping",
			cbdry,cbdry_orient);
	}
	debug_print("map_p_sts","Left map_phys_cur_states_to_bdry_cur()\n");
}		/*end map_phys_cur_states_to_bdry_cur*/

EXPORT	void	debug_show_boundary_curve_states(
	const char	*msg,
	CURVE		*cbdry,
	ORIENTATION	cbdry_orient)
{
	CURVE		*bcur, *nextbc;
	NODE		*bnode = NULL;
	int		bn_type = UNKNOWN_NODE_TYPE;

	(void) printf("states on boundary curve %llu %s\n",
		      curve_number(cbdry),msg);
	nextbc = NULL;
	bcur = cbdry;
	while (nextbc != cbdry) 
	{
		show_curve_states(bcur);
		nextbc = next_curve_of_gen_curve(bcur,cbdry_orient,
					&bn_type,&bnode);
		if (!nextbc) break;
		bcur = nextbc;
	}

}		/*end debug_show_boundary_curve_states*/

/*
*			map_states_at_node():
*
*	Sets the states at Node_of(cbdry,cbdry_orient). On cbdry_side
*	the state is set to phys_state.  The setting on the other side
*	depends on the wave type of cbdry.
*/

LOCAL	void map_states_at_node(
	Locstate	phys_state,	/* state on cphys_side of cphys */
	CURVE		*cbdry,
	ORIENTATION	cbdry_orient,
	SIDE		cbdry_side,
	Front		*fr)
{
	Locstate	sphys, sext, ext_state;

	debug_print("map_p_sts","Entered map_states_at_node()\n");

	/* Don't reset states on SUBDOMAIN boundaries */
	if (wave_type(cbdry) == SUBDOMAIN_BOUNDARY)
		return;
	if (cbdry_side == NEGATIVE_SIDE) 
	{
	    sphys = Left_state_at_node(cbdry,cbdry_orient);
	    sext = Right_state_at_node(cbdry,cbdry_orient);
	    ext_state = right_state_at_point_on_curve(
				Point_adjacent_to_node(cbdry,cbdry_orient),
				Bond_at_node(cbdry,cbdry_orient),cbdry);
	}
	else 
	{
	    sphys = Right_state_at_node(cbdry,cbdry_orient);
	    sext = Left_state_at_node(cbdry,cbdry_orient);
	    ext_state = left_state_at_point_on_curve(
				Point_adjacent_to_node(cbdry,cbdry_orient),
				Bond_at_node(cbdry,cbdry_orient),cbdry);
	}

		/* assign sphys to state on cphys_side of cphys */

	ft_assign(sphys,phys_state,fr->sizest);

	if (debugging("map_p_sts")) 
	{
	    (void) printf("Reset node state\n");
	    (void) printf("sphys:\n"); (*fr->print_state)(sphys);
	}

			/* Assign state sext */

	/* Dirichlet, Passive or Neumann case - set to state at adj point*/
	ft_assign(sext,ext_state,fr->sizest);

	if (debugging("map_p_sts")) 
	{
	    (void) printf("Reset node state\n");
	    (void) printf("sext:\n"); (*fr->print_state)(sext);
	}

	debug_print("map_p_sts","Left map_states_at_node()\n");
}		/*end map_states_at_node*/

LOCAL void eliminate_redundant_closed_node_at_bdry_tangle(
	CROSS		*cr,
	CURVE		**physcurves,
	ORIENTATION	orient)
{
	INTERFACE	*intfc = current_interface();
	boolean		sav_interp;
	COMPONENT	left, right;
	CURVE		*cur, *c_in, *c_out;
	NODE		*cl_nd;
	
	if (orient == POSITIVE_ORIENTATION)
	{
		c_out = physcurves[0];
		cl_nd = c_out->start;
		c_in = cl_nd->in_curves[0];
		left = negative_component(c_out);
		right = positive_component(c_out);
	}
	else
	{
		c_in = physcurves[1];
		cl_nd = c_in->end;
		c_out = cl_nd->out_curves[0];
		left = negative_component(c_in);
		right = positive_component(c_in);
	}
	sav_interp = interpolate_intfc_states(intfc);
	interpolate_intfc_states(intfc) = YES;
	cur = join_curves(c_in,c_out,left,right,(BOND **)NULL);
	interpolate_intfc_states(intfc) = sav_interp;
	rcl_after_join(cr,cur,c_in,c_out);
	if (orient == POSITIVE_ORIENTATION)
	{
		physcurves[0] = cur;
		if (c_in == physcurves[1])
			physcurves[1] = cur;
	}
	else
	{
		physcurves[1] = cur;
		if (c_out == physcurves[0])
			physcurves[0] = cur;

	}
}		/*end eliminate_redundant_closed_node_at_bdry_tangle*/

/*
*			classify_bdry_crosses():
*
*	This routine extracts from the CROSS list those crosses that
*	correspond to intersections between boundary curves
*	and interior curves.  These boundary-interior crosses
*	are then further subdivided into two classes;  those
*	that correspond to a "regular" boundary cross where
*	a curve crosses a boundary and then returns into the
*	computational domain, and those that correspond to an
*	"irregular" boundary cross where a curve exits through a
*	boundary but does not recross the boundary.  Crosses in the
*	"irregular" class most often occur near a well where a section
*	of a curve attached to the well passes through the boundary
*	near the well.
*/

EXPORT	void classify_bdry_crosses(
	CROSS		*cross,
	int		*is_irregular_bdry_cross)
{
	CROSS		*cr, *cr0;
	int		i;
	ORIENTATION	c1_orient, c2_orient, c10_orient, c20_orient;

	if (cross == NULL) return;

	for (i = 0, cr = cross; cr != NULL; cr = cr->next)
	{
	    if (interior_curve_crosses_boundary(cr) == YES)
	    {
	    	(void) find_companion_cross(cr,&cr0,&c1_orient,
					    &c2_orient,&c10_orient,&c20_orient);

	    	if (cr0 != NULL)
	    	{
	    	    is_irregular_bdry_cross[i++] =  NO;
	    	    delete_from_cross_list(cr0);
	    	    insert_in_cross_list(cr,cr0);
	    	    is_irregular_bdry_cross[i++] =  NO;
	    	    cr = cr->next;
	    	}
	    	else
	    	    is_irregular_bdry_cross[i++] = YES;
	    }
	    else
	    	is_irregular_bdry_cross[i++] = ERROR;
	}
}		/*end classify_bdry_crosses*/

/*
*		interior_curve_crosses_boundary():
*
*	Tests wheter a cross corresponds to a boundary curve
*	interior curve intersection.   If YES, the curve and
*	bond structures in the cross are reset so that the
*	physical interior curve occupies the first position.
*/

LOCAL	boolean interior_curve_crosses_boundary(
	CROSS		*cross)
{
	BOND		*btmp;
	CURVE		*ctmp;

	if (wave_type(cross->c1) < FIRST_PHYSICS_WAVE_TYPE &&
	    wave_type(cross->c2) >= FIRST_PHYSICS_WAVE_TYPE) 
	{
	    ctmp = cross->c1;
	    btmp = cross->b1;
	    cross->c1 = cross->c2;
	    cross->b1 = cross->b2;
	    cross->c2 = ctmp;
	    cross->b2 = btmp;
	}
	if (wave_type(cross->c1) < FIRST_PHYSICS_WAVE_TYPE ||
	    wave_type(cross->c2) >= FIRST_PHYSICS_WAVE_TYPE)
	    return NO;
	return YES;
}		/*end interior_curve_crosses_boundary*/
#endif /* defined(TWOD) */
