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
*				fprop2d.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*/


#include <front/fdecs.h>

LOCAL 	void 	f_second_order_intfc_propagate2d(Front*,POINTER,INTERFACE*,
				INTERFACE*,double);
LOCAL 	void 	f_second_order_intfc_propagate3d(Front*,POINTER,INTERFACE*,
				INTERFACE*,double);
LOCAL	void 	set_propagation_bounds(Front*,double*,double*);
LOCAL	boolean	out_of_bound(FT_POINT*,double*,double*,int);

/*
*			f_tan_curve_propagate():
*
*	Performs the tangential sweep update of states on the front.
*	All points with t_pt_propagated(p) == NO are updated.
*/

/* ARGSUSED */
EXPORT	void f_tan_curve_propagate(
	Front		*fr,
	Front		*newfr,
	INTERFACE	*tempintfc,
	CURVE		*tempc,
	CURVE		*newc,
	double		dt)
{
	BOND		    *tempb, *newb;
	Locstate	    ansl, ansr;
	boolean		    curveIsClosed;
	double		    *h = fr->rect_grid->h;
	double		    tngt[MAXD], ds, sbl;
	int		    i, dim = fr->rect_grid->dim;
	static	int	    nrad = 0;
	static	Tan_stencil *sten = NULL;

	debug_print("f_tan_prop","Entered f_tan_curve_propagate()\n");
	if (debugging("f_tan_prop"))
	{
	    (void) printf("tempc %llu  newc %llu\n",curve_number(tempc),
	    	          curve_number(newc));
	    (void) printf("tempc\n");	print_curve(tempc);
	    (void) printf("\nnewc\n");	print_curve(newc);
	}

	if (sten == NULL) 
	{
	    nrad = fr->npts_tan_sten/2;
	    sten = alloc_tan_stencil(fr,nrad);
	}

	switch (wave_type(tempc))
	{
	case PASSIVE_BOUNDARY:
	case SUBDOMAIN_BOUNDARY:
	    return;
	default:
	    break;
	}

	curveIsClosed = (is_closed_node(newc->end)) ? YES : NO;

	tempb = tempc->first;		newb  = newc->first;

		/* Check if zero length curve */

	if (tempc->first == tempc->last)
	{
	    sbl = scaled_bond_length(tempc->first,h,dim);
	    if (sbl < MIN_SC_SEP(tempintfc))
	    {
	    	debug_print("f_tan_prop","Left f_tan_curve_propagate()\n");
	    	return;
	    }
	}

	for (;  newb;  tempb = tempb->next, newb = newb->next)
	{
	    if (t_pt_propagated(newb->end))
	        continue;

	    /* stop at tempc->last if no continuation */

	    if ((tempb == tempc->last) &&
	        !curveIsClosed && !is_fixed_node(newc->end))
	    {
	    	break;
	    }

	    /* calculate tangential displacement */

	    /*
	     *  TODO:  the upgrade of this function
	     *  to 3 dimensions is non-trivial.
	     *  There will need to be either two
	     *  operator splitting sweeps,  or one
	     *  unsplit solver.  There is arbitrary
	     *  choice of tangent directions and this
	     *  will have to be resolved.
	     */

	    tangent(newb->end,newb,newc,tngt,newfr);

	    ds = grid_size_in_direction(tngt,h,dim);

	    /* find the stencil states */

	    states_at_distance_along_curve(tempb->end,tempb,tempc,
			                   NEGATIVE_ORIENTATION,ds,nrad,
			                   sten->leftst-1,sten->rightst-1,
			                   sten->hs-1,sten->hse-1,sten->t-1,
					   sten->p-1,newfr);

	    if (tempb->next != NULL)
	    {
	    	ansl  = left_state(newb->end);
	    	ansr  = right_state(newb->end);
	    }
	    else
	    {
	    	ansl  = left_end_state(newc);
	    	ansr  = right_end_state(newc);
	    }

	    states_at_distance_along_curve(tempb->end,tempb,tempc,
			                   POSITIVE_ORIENTATION,ds,nrad,
			                   sten->leftst+1,sten->rightst+1,
			                   sten->hs+1,sten->hse+1,sten->t+1,
					   sten->p+1,newfr);

	    sten->p[0] = tempb->end;
	    sten->hse[0] = Hyper_surf_element(tempb);
	    sten->hs[0] = Hyper_surf(tempc);
	    sten->t[0] = 1.0;
	    sten->curvature = mean_curvature_at_point(sten->p[0],sten->hse[0],
	                                              sten->hs[0],fr);

	    if (debugging("f_tan_prop")) 
	    {
	    	int        j;
	    	static const char *xyz[3] = { "x", "y", "z" };

	        (void) printf("state locations\n");
		(void) printf("%-8s"," ");
		for (i = 0; i < dim; ++i)
		    (void) printf("%-14s",xyz[i]);
		(void) printf("\n");
		for (j = -nrad; j <= nrad; ++j)
		{
		    for (i = 0; i < dim; ++i)
			(void) printf("%-14g",Coords(sten->p[j])[i]);
		    (void) printf("\n");
		}
		(void) printf("\n");
		(void) printf("State values\n");
		for (j = -nrad; j <= nrad; ++j)
		{
		    (void) printf("left state[%d] at ",j);
		    print_general_vector("",Coords(sten->p[j]),dim,"\n");
		    (*fr->print_state)(
			left_state_at_point_on_curve(sten->p[j],
						     Bond_of_hse(sten->hse[j]),
					             Curve_of_hs(sten->hs[j])));
		    (void) printf("right state[%d] at ",j);
		    print_general_vector("",Coords(sten->p[j]),dim,"\n");
		    (*fr->print_state)(
	                right_state_at_point_on_curve(sten->p[j],
						      Bond_of_hse(sten->hse[j]),
					              Curve_of_hs(sten->hs[j])));
		    (void) printf("\n");
		}
	    }

	    /* update using n-point stencil tangential op */

	    sten->newhs = Hyper_surf(newc);
	    sten->dir = tngt;
	    npt_tang_solver(ds,dt,sten,ansl,ansr,fr);
            if (fr->parab == YES)
                npt_parab_tan_solver2d(ds,dt,sten,ansl,ansr,fr);
	    t_pt_propagated(newb->end) = YES;

	    if (debugging("f_tan_prop"))
	    {
		(void) printf("answers: left right\n");
		(*newfr->print_state)(ansl);
		(*newfr->print_state)(ansr);
		(void) printf("\n");
	    }
	}

	if (curveIsClosed)
	{
	    /* assign start states to end states */
	    ft_assign(left_start_state(newc),left_end_state(newc),fr->sizest);
	    ft_assign(right_start_state(newc),right_end_state(newc),fr->sizest);
	}
	debug_print("f_tan_prop","Left f_tan_curve_propagate()\n");
}		/*end f_tan_curve_propagate*/








/*
*			oblique_propagate_at_node():
*/

EXPORT void oblique_propagate_at_node(
	Front		*fr,
	POINTER		wave,
	FT_POINT		*newp,
	O_CURVE		*oldc,
	O_CURVE		*newc,
	double		*nor,
	double		dt)
{
	BOND		*oldb;
	FT_POINT		*oldn_posn, *oldp;
	double		save[MAXD], dposn[MAXD];
	double		len, t[MAXD];
	double		V[MAXD];
	int		i, dim = fr->rect_grid->dim;
	void		(*save_impose_bc)(FT_POINT*,BOND*,CURVE*,double*,Front*,
					  boolean,boolean);

	oldb = Bond_at_node_of_o_curve(oldc);
	oldn_posn = Node_of_o_curve(oldc)->posn;
	oldp = Point_adjacent_to_node(oldc->curve,oldc->orient);
	for (i = 0; i < dim; ++i)
	{
		save[i] = Coords(oldp)[i];
		dposn[i] = save[i] - Coords(oldn_posn)[i];
	}
	t[0] = -nor[1];		t[1] =  nor[0];
	len = mag_vector(t,dim);
	for (i = 0; i < dim; ++i) t[i] /= len;
	if (scalar_product(t,dposn,dim) < 0.0)
	{
		for (i = 0; i < dim; ++i) t[i] = -t[i];
	}
	len = 0.0;
	for (i = 0; i < dim; ++i) len += fabs(t[i]*fr->rect_grid->h[i]);
	for (i = 0; i < dim; ++i)
		Coords(oldp)[i] = Coords(oldn_posn)[i] + len * t[i];
	save_impose_bc = fr->impose_bc;
	fr->impose_bc = NULL;
	point_propagate(fr,wave,oldn_posn,newp,oldb,oldc->curve,dt,V);
	if (newc->orient != oldc->orient) reverse_states_at_point(newp,fr);
	for (i = 0; i < dim; ++i) Coords(oldp)[i] = save[i];
	fr->impose_bc = save_impose_bc;
}		/*end oblique_propagate_at_node*/

/*
*			f_curve_propagate2d():
*
*	Propagates a curve by propagating each point from
*	oldc->first->end to oldc->last->start,
*	through a call to point_propagate.
*	and propagates each bond from old->first
*	to oldc->last through a call to fr->bond_propagate.
*/

EXPORT void f_curve_propagate2d(
	Front		*fr,
	POINTER		wave,
	CURVE		*oldc,
	CURVE		*newc,
	double		dt)
{
	BOND		*oldb = oldc->first;
	BOND		*newb = newc->first;
	double		V[MAXD];
	int		dim = fr->interf->dim;	
	double		L[MAXD],U[MAXD];	/* propagation boundary */

	debug_print("f_curve_propagate","Entered f_curve_propagate2d\n");

	if ((fr->_point_propagate == NULL)   ||
	    (oldc == NULL)                   ||
	    (newc == NULL)                   ||
	    (correspond_curve(oldc) != newc) ||
	    (correspond_curve(newc) != oldc))
	    return;

	set_propagation_bounds(fr,L,U);
	while (oldb) 
	{
	    if ((oldb != oldc->last) && (!n_pt_propagated(newb->end)))
	    {
	    	n_pt_propagated(newb->end) = YES;
		if (out_of_bound(oldb->end,L,U,dim) &&
		    wave_type(oldc) != MOVABLE_BODY_BOUNDARY) 
		{
		    Locstate newsl,newsr;
		    Locstate oldsl,oldsr;
		    slsr(newb->end,Hyper_surf_element(newb),Hyper_surf(newc),
		    		&newsl,&newsr);
		    slsr(oldb->end,Hyper_surf_element(oldb),Hyper_surf(oldc),
		    		&oldsl,&oldsr);
		    ft_assign(newsl,oldsl,fr->sizest);                 
		    ft_assign(newsr,oldsr,fr->sizest);
		    continue;
		}
	    	point_propagate(fr,wave,oldb->end,newb->end,oldb->next,
				oldc,dt,V);
	    }
	    if (fr->bond_propagate != NULL)
	    	(*fr->bond_propagate)(fr,wave,oldb,newb,oldc,dt);
	    else
	    	set_bond_length(newb,dim); /* Update new bond length */
	    if (oldb == oldc->last)
		break;
	    oldb = oldb->next;
	    newb = newb->next;
	}
	if (wave_type(oldc) == MOVABLE_BODY_BOUNDARY)
        {
            /* Propagate center of mass */
            int i,dim;
            dim = fr->rect_grid->dim;
            for (i = 0; i < dim; ++i)
	    {
                center_of_mass_velo(newc)[i] = center_of_mass_velo(oldc)[i];
                center_of_mass(newc)[i] = center_of_mass(oldc)[i] +
                        dt*center_of_mass_velo(oldc)[i];
	    }
	    angular_velo(newc) = angular_velo(oldc);
        }
	debug_print("f_curve_propagate","Leaving f_curve_propagate2d\n");
}		/*end f_curve_propagate2d*/

EXPORT	void	set_no_tan_propagate(
	CURVE	*c)
{
	BOND	*b;

	t_pt_propagated(c->first->start) = YES;
	for (b = c->first; b != NULL; b = b->next)
		t_pt_propagated(b->end) = YES;
}		/*end set_no_tan_propagate*/

LOCAL	void 	set_propagation_bounds(
	Front *fr,
	double *L,
	double *U)
{
	RECT_GRID	*gr = fr->rect_grid;
	int		i,dim = gr->dim;

	/* Set propagation boundaries */
	for (i = 0; i < dim; ++i)
	{
	    if (gr->lbuf[i] > 0)
	    	L[i] = gr->L[i] - 2.0*gr->h[i];
	    else
	    	L[i] = -HUGE_VAL;;
	    if (gr->ubuf[i] > 0)
	    	U[i] = gr->U[i] + 2.0*gr->h[i];
	    else
	    	U[i] = HUGE_VAL;;
	}
}	/* end set_propagation_bounds */

LOCAL	boolean	out_of_bound(
	FT_POINT *p,
	double *L,
	double *U,
	int dim)
{
	int i;
	for (i = 0; i < dim; ++i)
	    if (Coords(p)[i] < L[i] || Coords(p)[i] > U[i])
	    	return YES;
	return NO;
}	/* end out_of_bound */


/*
*			f_second_order_intfc_propagate():
*
*	Propagates each interior point of the interface by
*	calling point propagate. All node points (except closed
*	nodes) are not propagated. 
*/

EXPORT void f_second_order_intfc_propagate(
	Front		*fr,
	POINTER		wave,
	INTERFACE	*old_intfc,
	INTERFACE	*new_intfc,
	double		dt)
{
	switch (fr->rect_grid->dim)
	{
	case 2:
	    f_second_order_intfc_propagate2d(fr,wave,old_intfc,new_intfc,dt);
	    return;
	case 3:
	    f_second_order_intfc_propagate3d(fr,wave,old_intfc,new_intfc,dt);
	    return;
	}
}	/* end f_second_order_intfc_propagate */

LOCAL void f_second_order_intfc_propagate2d(
	Front		*fr,
	POINTER		wave,
	INTERFACE	*old_intfc,
	INTERFACE	*new_intfc,
	double		dt)
{
	INTERFACE	*tmp_intfc;
	HYPER_SURF              *oldhs, *tmphs, *newhs;
        HYPER_SURF_ELEMENT      *oldhse, *tmphse, *newhse;
	CURVE              	**oldc, **tmpc, **newc;
        BOND      		*oldb, *tmpb, *newb;
        FT_POINT                   *oldp, *tmpp, *newp;
	int		i;
	double		V[MAXD];

	set_copy_intfc_states(NO);
	tmp_intfc = pp_copy_interface(fr->interf);

	/* Compute v(x^n, t^n) */

	for (oldc = old_intfc->curves, tmpc = tmp_intfc->curves;
	     oldc && *oldc; ++oldc, ++tmpc)
	{
	    for (oldb = (*oldc)->first, tmpb = (*tmpc)->first;
		 oldb != NULL; oldb = oldb->next, tmpb = tmpb->next)
	    {
		if (oldb == (*oldc)->last && !is_closed_node((*oldc)->end))
		    continue;
		oldp = oldb->end;	tmpp = tmpb->end;
		oldhse = Hyper_surf_element(oldb);
		oldhs = Hyper_surf(*oldc);
	    	point_propagate(fr,wave,oldp,tmpp,oldhse,oldhs,dt,V);
	    }
	}

	/* Compute v(x^(n+1), t^(n+1)) */

	for (newc = new_intfc->curves, tmpc = tmp_intfc->curves;
	     newc && *newc; ++newc, ++tmpc)
	{
	    for (newb = (*newc)->first, tmpb = (*tmpc)->first;
		 newb != NULL; newb = newb->next, tmpb = tmpb->next)
	    {
		tmpp = tmpb->end;	newp = newb->end;
		tmphse = Hyper_surf_element(tmpb);
		tmphs = Hyper_surf(*tmpc);
	    	point_propagate(fr,wave,tmpp,newp,tmphse,tmphs,dt,V);
	    }
	}

	/* Compute x^(n+1) = x^n + 0.5*dt*(v(x^n,t^n) + v(x^(n+1), t^(n+1)) */

	for (oldc = old_intfc->curves, tmpc = tmp_intfc->curves, 
	     newc = new_intfc->curves;
	     oldc && *oldc; ++oldc, ++tmpc, ++newc)
	{
	    for (oldb = (*oldc)->first, tmpb = (*tmpc)->first,
		 newb = (*newc)->first; oldb != NULL; 
		 oldb = oldb->next, tmpb = tmpb->next, newb = newb->next)
	    {
		oldp = oldb->end;	
		tmpp = tmpb->end;	
		newp = newb->end;
	    	for (i = 0; i < 2; ++i)
	    	    Coords(newp)[i] = Coords(oldp)[i] + 0.5*(oldp->vel[i] +
					tmpp->vel[i])*dt;
		if (newb == (*newc)->last && is_closed_node((*newc)->end))
		{
		    propagation_status((*newc)->end) = PROPAGATED_NODE;
		    set_bond_length((*newc)->first,2);
		    set_bond_length((*newc)->last,2);
		}
		else
		    set_bond_length(newb,2);
	    }
	}

	delete_interface(tmp_intfc);
}	/* end f_second_order_intfc_propagate2d */

LOCAL void f_second_order_intfc_propagate3d(
	Front		*fr,
	POINTER		wave,
	INTERFACE	*old_intfc,
	INTERFACE	*new_intfc,
	double		dt)
{
	INTERFACE	*tmp_intfc;
	HYPER_SURF              *oldhs, *tmphs, *newhs;
        HYPER_SURF_ELEMENT      *oldhse, *tmphse, *newhse;
        FT_POINT                   *oldp, *tmpp, *newp;
	int		i;
	double		V[MAXD];
	printf("Entering f_second_order_intfc_propagate3d()\n");

	set_copy_intfc_states(NO);
	tmp_intfc = pp_copy_interface(fr->interf);

	(void) next_point(old_intfc,NULL,NULL,NULL);
        (void) next_point(tmp_intfc,NULL,NULL,NULL);
        while (next_point(old_intfc,&oldp,&oldhse,&oldhs) &&
               next_point(tmp_intfc,&tmpp,&tmphse,&tmphs))
        {
	    point_propagate(fr,wave,oldp,tmpp,oldhse,oldhs,dt,V);
	}

        (void) next_point(tmp_intfc,NULL,NULL,NULL);
	(void) next_point(new_intfc,NULL,NULL,NULL);
        while (next_point(tmp_intfc,&tmpp,&tmphse,&tmphs) &&
               next_point(new_intfc,&newp,&newhse,&newhs))
        {
	    point_propagate(fr,wave,tmpp,newp,tmphse,tmphs,dt,V);
	}

	(void) next_point(old_intfc,NULL,NULL,NULL);
        (void) next_point(tmp_intfc,NULL,NULL,NULL);
	(void) next_point(new_intfc,NULL,NULL,NULL);
        while (next_point(old_intfc,&oldp,&oldhse,&oldhs) &&
               next_point(tmp_intfc,&tmpp,&tmphse,&tmphs) &&
	       next_point(new_intfc,&newp,&newhse,&newhs))
        {
	    for (i = 0; i < 3; ++i)
	    	Coords(newp)[i] = Coords(oldp)[i] + 0.5*(oldp->vel[i] +
				tmpp->vel[i])*dt;
	}

	delete_interface(tmp_intfc);
	printf("Leaving f_second_order_intfc_propagate3d()\n");
}	/* end f_second_order_intfc_propagate3d */
