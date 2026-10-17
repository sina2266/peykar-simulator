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
*				fcrossext.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Contains the routines
*
*		D_extend_crossing_of_two_propagated_curves()
*		H_extend_crossing_of_two_propagated_curves()
*		cross_or_extend_to_cross_two_propagated_curves()
*
*	for use by node propagate routines.
*/


#include <front/fdecs.h>

typedef struct {
	O_CURVE *oldc, *newc;
	double v[MAXD];
	FT_POINT *p, *p_opp;
	BOND *bvirtual, *oppb;
	NODE *oppn;
} VIRTUAL_PROPAGATED_CURVE;

typedef struct {
	FT_POINT	*pc;		/* crossing point */
	BOND	**newbacr,	/* the crossing bond on the newca */
		**newb2cr;	/* the crossing bond on the newc2 */
	double	*sa,*s2;	/* fract dist on bond to cross */
	RPROBLEM **rp;
	double	*dt_frac;
} D_EXTEND_OUTPUT;

	/* LOCAL Function Declarations */
LOCAL	BOND	*circle_D_extend(VIRTUAL_PROPAGATED_CURVE*,
				 VIRTUAL_PROPAGATED_CURVE*,
				 VIRTUAL_PROPAGATED_CURVE*,D_EXTEND_OUTPUT*,
				 BOND*,COMPONENT,int*,double*,double*,
				 NODE_FLAG,Front*);
LOCAL	BOND	*linear_D_extend(VIRTUAL_PROPAGATED_CURVE*,
				 VIRTUAL_PROPAGATED_CURVE*,
				 VIRTUAL_PROPAGATED_CURVE*,D_EXTEND_OUTPUT*,
				 BOND*,COMPONENT,int*,double*,double*,
				 NODE_FLAG,Front*);
LOCAL	BOND	*normal_D_extend(VIRTUAL_PROPAGATED_CURVE*,
				 VIRTUAL_PROPAGATED_CURVE*,
				 VIRTUAL_PROPAGATED_CURVE*,
				 D_EXTEND_OUTPUT*,BOND*,COMPONENT,int*,
				 double*,double*,NODE_FLAG,Front*);
LOCAL	int	check_H_extend_cross(BOND*,BOND*,O_CURVE*,O_CURVE*,double*,int,
				     COMPONENT,COMPONENT,boolean*);
LOCAL	int	find_circle_through_points(FT_POINT*,FT_POINT*,FT_POINT*,FT_POINT*,
					   double*,double*,int);
LOCAL	int	found_D_extend_cross(VIRTUAL_PROPAGATED_CURVE*,
				     VIRTUAL_PROPAGATED_CURVE*,
				     VIRTUAL_PROPAGATED_CURVE*,
				     D_EXTEND_OUTPUT*,BOND*,Front*,double);
LOCAL	int	found_H_ext_cr_2_pc(O_CURVE*,O_CURVE*,O_CURVE*,O_CURVE*,
                                    BOND*,BOND*,BOND**,BOND**,BOND*,
				    BOND*,BOND*,BOND*,FT_POINT*,FT_POINT*,double,double,
				    NODE*,NODE*,double,double*,double*,double*,
				    Front*);
LOCAL	int	found_c_or_e_2_pc(BOND**,BOND**,BOND*,BOND*,BOND*,BOND*,
                                  O_CURVE*,O_CURVE*,O_CURVE*,O_CURVE*,
				  BOND*,BOND*,NODE*,NODE*,NODE_FLAG,
				  FT_POINT**,FT_POINT*,double,double,double,
				  double*,double*,double*,Front*,boolean*);
LOCAL	int	leave_D_extend(VIRTUAL_PROPAGATED_CURVE*,
			       VIRTUAL_PROPAGATED_CURVE*,
			       VIRTUAL_PROPAGATED_CURVE*,int);
LOCAL	int	leave_H_ext_cr_2_pc(int,O_CURVE*,O_CURVE*,BOND*,BOND*,
                                    NODE*,NODE*);
LOCAL	int	leave_cross_or_extend_to_cross_two_propagated_curves(int,
                                                                     O_CURVE*,
								     O_CURVE*,
								     BOND*,
								     BOND*,
								     NODE*,
								     NODE*);
LOCAL	boolean	no_D_extend_cross(VIRTUAL_PROPAGATED_CURVE*,
				  VIRTUAL_PROPAGATED_CURVE*,D_EXTEND_OUTPUT*,
				  int*,int,BOND*,BOND*,Front*,POINTER,double);
LOCAL	int	no_H_ext_cr_2_pc(O_CURVE*,O_CURVE*,O_CURVE*,O_CURVE*,COMPONENT,
                                 COMPONENT,FT_POINT*,FT_POINT*,FT_POINT*,FT_POINT*,FT_POINT*,
			         FT_POINT*,BOND**,BOND**,BOND*,BOND*,BOND*,BOND*,
			         BOND*,BOND*,BOND*,NODE*,NODE*,double*,double*,
			         double*,int,Front*,POINTER,RPROBLEM**,
			         double,double*,double*,double*,boolean,int,NODE_FLAG);
LOCAL	int	no_c_or_e_2_pc(int,O_CURVE*,O_CURVE*,O_CURVE*,O_CURVE*,BOND*,
                               BOND*,BOND*,BOND*,BOND*,BOND*,NODE*,NODE*,
			       FT_POINT*,FT_POINT*,FT_POINT**,Front*,POINTER,
			       double,double*,RPROBLEM**);
LOCAL	void	modify_D_extend_list(boolean,
				     BOND*(*)(VIRTUAL_PROPAGATED_CURVE*,
					      VIRTUAL_PROPAGATED_CURVE*,
					      VIRTUAL_PROPAGATED_CURVE*,
					      D_EXTEND_OUTPUT*,BOND*,
					      COMPONENT,int*,double*,double*,
					      NODE_FLAG,Front*));
LOCAL	void	swap_virtual_prop_curves(VIRTUAL_PROPAGATED_CURVE*,
					 VIRTUAL_PROPAGATED_CURVE*);

/*
*               D_extend_crossing_of_two_propagated_curves():
*
*	The curve newc2 is extended from the node newn to its intersection 
*	with newca, which is also found.
*	This function is used in B_node_propagate().
*       If there is a crossing GOOD_NODE is returned and the crossing point 
*	and bonds are found a crossing; otherwise a diagnostic signal is 
*	returned.
*/

#define NORMAL_D_EXTEND 1 /* 1 = USE,  0 = DONT USE */
#define CIRCLE_D_EXTEND 0 /* 1 = USE,  0 = DONT USE */
	
LOCAL   BOND *(*D_extend[4])(VIRTUAL_PROPAGATED_CURVE*,
                             VIRTUAL_PROPAGATED_CURVE*,
                             VIRTUAL_PROPAGATED_CURVE*,D_EXTEND_OUTPUT*,
                             BOND*,COMPONENT,int*,double*,double*,
                             NODE_FLAG,Front*) =
#if NORMAL_D_EXTEND && CIRCLE_D_EXTEND
        {circle_D_extend, normal_D_extend,linear_D_extend, NULL};
#elif CIRCLE_D_EXTEND
        {circle_D_extend, linear_D_extend, NULL, NULL};
#elif NORMAL_D_EXTEND
        {normal_D_extend, linear_D_extend, NULL, NULL};
#else /* NORMAL_D_EXTEND && CIRCLE_D_EXTEND */
        {linear_D_extend, NULL, NULL, NULL};
#endif /* NORMAL_D_EXTEND && CIRCLE_D_EXTEND */

LOCAL	BOND *(*scalar_D_extend[4])(VIRTUAL_PROPAGATED_CURVE*,
			     VIRTUAL_PROPAGATED_CURVE*,
			     VIRTUAL_PROPAGATED_CURVE*,D_EXTEND_OUTPUT*,
			     BOND*,COMPONENT,int*,double*,double*,
			     NODE_FLAG,Front*) =
	{circle_D_extend, normal_D_extend, linear_D_extend, NULL};

LOCAL	BOND *(*vector_D_extend[4])(VIRTUAL_PROPAGATED_CURVE*,
			     VIRTUAL_PROPAGATED_CURVE*,
			     VIRTUAL_PROPAGATED_CURVE*,D_EXTEND_OUTPUT*,
			     BOND*,COMPONENT,int*,double*,double*,
			     NODE_FLAG,Front*) =
	{linear_D_extend, normal_D_extend,NULL, NULL};


EXPORT	void set_use_circle_D_extend(
	boolean	yn)
{
	modify_D_extend_list(yn,circle_D_extend);
}		/*end set_use_circle_D_extend*/

EXPORT	void set_use_normal_D_extend(
	boolean	yn)
{
	modify_D_extend_list(yn,normal_D_extend);
}		/*end set_use_normal_D_extend*/

LOCAL	void modify_D_extend_list(
	boolean	yn,
	BOND	*(*func) (VIRTUAL_PROPAGATED_CURVE*,VIRTUAL_PROPAGATED_CURVE*,
			  VIRTUAL_PROPAGATED_CURVE*,D_EXTEND_OUTPUT*,BOND*,
			  COMPONENT,int*,double*,double*,NODE_FLAG,Front*))
{
	int i, j;

	for (i = 0; D_extend[i] != NULL; ++i)
	{
	    if (D_extend[i] == func)
	    {
	    	for (j = i; D_extend[j] != NULL; ++j)
	    	    D_extend[j] = D_extend[j+1];
	    }
	}
	if (yn == YES)
	{
	    for (i = 0; D_extend[i] != NULL; ++i);
	    for (; i > 0; --i)
		D_extend[i] = D_extend[i-1];
	    D_extend[0] = func;
	}
}		/*end modify_D_extend_list*/

/*ARGSUSED*/
EXPORT int D_extend_crossing_of_two_propagated_curves(
	O_CURVE		*oldca,
	O_CURVE		*newca,
	O_CURVE		*oldcb,
	O_CURVE		*newcb,
	O_CURVE		*oldc2,
	O_CURVE		*newc2,
	COMPONENT	ahead_comp,
	COMPONENT	propagation_comp,
	FT_POINT		*pc,		/* crossing point */
	BOND		**newbacr,	/* the crossing bond on the newca */
	BOND		**newb2cr,	/* the crossing bond on the newc2 */
	double		*sa, double *s2,	/* fract dist on bond to cross */
	Front		*fr,
	POINTER		wave,
	RPROBLEM	**rp,
	double		dt,		/* time step */
	double		*dt_frac,
	NODE_FLAG	flag)
{
	VIRTUAL_PROPAGATED_CURVE CA, CB, C2;
	D_EXTEND_OUTPUT DOUT;
	BOND		*newba;
	BOND		Ba, Bb, B2, OLDB2DIR, NEWB2DIR;
	double		*h = fr->rect_grid->h;
	double		low[MAXD], high[MAXD];
	int		cr_stat = NO_CROSS;
	int		status;
	int		i, dim = fr->interf->dim;
	static	FT_POINT	*pa = NULL, *pb = NULL, *p2 = NULL, *pa_opp = NULL,
	                *pb_opp = NULL, *p2_opp = NULL;
	BOND *(**Extend)(VIRTUAL_PROPAGATED_CURVE*,
			     VIRTUAL_PROPAGATED_CURVE*,
			     VIRTUAL_PROPAGATED_CURVE*,D_EXTEND_OUTPUT*,
			     BOND*,COMPONENT,int*,double*,double*,
			     NODE_FLAG,Front*);

	debug_print("D_extend",
	    "Entered D_extend_crossing_of_two_propagated_curves()\n");

	if (pa == NULL) 
	{
	    pa = Static_point(fr->interf);
	    pb = Static_point(fr->interf);
	    p2 = Static_point(fr->interf);
	    pa_opp = Static_point(fr->interf);
	    pb_opp = Static_point(fr->interf);
	    p2_opp = Static_point(fr->interf);
	}

	for (i = 0; i < dim; ++i)
	{
	    low[i] = fr->rect_grid->VL[i] - MIN_SCALED_LENGTH(fr->interf)*h[i];
	    high[i] = fr->rect_grid->VU[i] + MIN_SCALED_LENGTH(fr->interf)*h[i];
	}

	/* Set internal Data structures */

	DOUT.pc = pc;
	DOUT.newbacr = newbacr;
	DOUT.newb2cr = newb2cr;
	DOUT.sa = sa;
	DOUT.s2 = s2;
	DOUT.rp = rp;
	DOUT.dt_frac = dt_frac;

	CA.newc = newca;	CB.newc = newcb;	C2.newc = newc2;
	CA.oldc = oldca;	CB.oldc = oldcb;	C2.oldc = oldc2;
	CA.p = pa;		CB.p = pb;		C2.p = p2;
	CA.p_opp = pa_opp;	CB.p_opp = pb_opp;	C2.p_opp = p2_opp;
	CA.bvirtual = &Ba;	CB.bvirtual = &Bb;	C2.bvirtual = &B2;

	init_curve_for_crossing(CA.p,CA.p_opp,CA.bvirtual,CA.oldc,CA.newc,
		                &CA.oppn,&CA.oppb,fr,wave,dt,CA.v,flag);
	init_curve_for_crossing(CB.p,CB.p_opp,CB.bvirtual,CB.oldc,CB.newc,
		                &CB.oppn,&CB.oppb,fr,wave,dt,CB.v,flag);
	init_curve_for_crossing(C2.p,C2.p_opp,C2.bvirtual,C2.oldc,C2.newc,
		                &C2.oppn,&C2.oppb,fr,wave,dt,C2.v,flag);

	    /* Fix a direction from a c2 bond for crossing with ca */

	find_bonds_for_extension_direction(C2.bvirtual,oldc2,newc2,&NEWB2DIR,
					   &OLDB2DIR,fr);

	if (debugging("D_extend"))
	{
	    (void) printf("Virtual and direction bonds\n");
	    (void) printf("CA.bvirtual\n");	print_bond(CA.bvirtual);
	    (void) printf("C2.bvirtual\n");	print_bond(C2.bvirtual);
	    (void) printf("OLDB2DIR\n");	print_bond(&OLDB2DIR);
	    (void) printf("NEWB2DIR\n");	print_bond(&NEWB2DIR);
	}

	*s2 = (newc2->orient == POSITIVE_ORIENTATION) ? 0.0 : 1.0;
	if (node_velocity_preset(flag) == YES)
	{
	    for (i = 0; i < dim; ++i)
	    {
	        Coords(pc)[i] = Coords(Node_of_o_curve(oldc2)->posn)[i] +
				dt * Node_vel(Node_of_o_curve(newc2))[i];
	    }
	    *newbacr = Bond_at_opp_node_of_o_curve(newca);
	    *newb2cr = Bond_at_node_of_o_curve(newc2);
	    *sa = (newca->orient == POSITIVE_ORIENTATION) ? 1.0 : 0.0;
	    status = GOOD_NODE;
	    return leave_D_extend(&CA,&CB,&C2,status);
	}

	if (wave_type(newc2->curve) >= FIRST_VECTOR_PHYSICS_WAVE_TYPE)
	    Extend = vector_D_extend;
	else
	    Extend = scalar_D_extend;
	for (i = 0; Extend[i] != NULL; ++i)
	{
	    newba = (*Extend[i])(&CA,&CB,&C2,&DOUT,&NEWB2DIR,ahead_comp,
			           &cr_stat,low,high,flag,fr);
	    if (newba != NULL && cr_stat == GOOD_CROSS)
	    {
	    	status = found_D_extend_cross(&CA,&CB,&C2,&DOUT,newba,fr,dt);
	    	if (status == GOOD_NODE)
	    	    return leave_D_extend(&CA,&CB,&C2,status);
	    }
	}


	if (no_D_extend_cross(&CA,&C2,&DOUT,&status,cr_stat,
		              &OLDB2DIR,&NEWB2DIR,fr,wave,dt) == YES)
	{
	    return leave_D_extend(&CA,&CB,&C2,status);
	}
	status = found_D_extend_cross(&CA,&CB,&C2,&DOUT,newba,fr,dt);
	return leave_D_extend(&CA,&CB,&C2,status);
}		/*end D_extend_crossing_of_two_propagated_curves*/

/*ARGSUSED*/
LOCAL	BOND *linear_D_extend(
	VIRTUAL_PROPAGATED_CURVE *ca,
	VIRTUAL_PROPAGATED_CURVE *cb,
	VIRTUAL_PROPAGATED_CURVE *c2,
	D_EXTEND_OUTPUT *dout,
	BOND		*newb2dir,
	COMPONENT	ahead_comp,
	int		*cr_stat,
	double		*low,
	double		*high,
	NODE_FLAG	flag,
	Front		*fr)
{
	BOND		*newba;
	FT_POINT		*pc = dout->pc;
	RECT_GRID	*rgr = fr->rect_grid;
	double		*h = rgr->h;
	double		q[MAXD],u[MAXD],v[MAXD],w[MAXD],vmw[MAXD];
	double		U_cross_V,U_cross_VMW;
	double		ulen;	
	double		para,alpha,alpha_min,alpha_max,delta;
	double		scaled_len;
	double		*sa = dout->sa;
	int		i, dim = rgr->dim;

	debug_print("D_extend","Entered linear_D_extend()\n");
	if (c2->newc->orient == POSITIVE_ORIENTATION) 
	{
	    for (i = 0; i < dim; ++i)
	    {
	    	q[i] = Coords(newb2dir->end)[i];
	    	u[i] = Coords(newb2dir->start)[i] - q[i];
	    }
	}
	else 
	{
	    for (i = 0; i < dim; ++i)
	    {
	    	q[i] = Coords(newb2dir->start)[i];
	    	u[i] = Coords(newb2dir->end)[i] - q[i];
	    }
	}
	if (scaled_hypot(u,h,dim) < MIN_SCALED_LENGTH(fr->interf))
	{
	    if (debugging("D_extend"))
	    {
		(void) printf("WARNING in "
		              "D_extend_crossing_of_two_propagated_curves(), "
		              "Unable to find normalized tangent to newc2\n");
	    }
	    debug_print("D_extend","Left linear_D_extend()\n");
	    return NULL;
	}
	ulen = mag_vector(u,dim);
	for (i = 0; i < dim; ++i) u[i] /= ulen;
	
	/* Find cross point of ca with c2 direction found above */
	/* Extension is by a line */

	newba = ca->bvirtual;
	for ( ; ; ) 
	{
	    if (debugging("D_extend"))
	    {
	        (void) printf("Loop over newba\n");
	        print_bond(newba);
	    }

	    for (i = 0; i < dim; ++i)
	    {
		v[i] = Coords(newba->start)[i] - q[i];
		w[i] = Coords(newba->end)[i] - q[i];
		vmw[i] = v[i] - w[i];
	    }
	    scaled_len = scaled_hypot(vmw,h,dim);
	    delta = (scaled_len < sqr(EPSILON)) ? 0.0 : 0.01/scaled_len;
	    alpha_min = (newba->prev == NULL) ?
	    		- delta : -MIN_SCALED_LENGTH(fr->interf)*delta;
	    alpha_max = (newba->next == NULL) ?
	    		1.0 + delta : 1.0 + MIN_SCALED_LENGTH(fr->interf)*delta;
	    (void) vector_product(u,v,&U_cross_V,dim);
	    (void) vector_product(u,vmw,&U_cross_VMW,dim);

	    if (fabs(U_cross_V - 0.5*(alpha_max + alpha_min)*U_cross_VMW) <=
	        	0.5*fabs(alpha_max - alpha_min)*fabs(U_cross_VMW)) 
	    {
	    	if (fabs(U_cross_VMW) > 0.)
	            alpha = U_cross_V/(U_cross_VMW);
	    	else
	            alpha = 0.;
	    	para = (1. - alpha)*scalar_product(u,v,dim) +
	            	      alpha*scalar_product(u,w,dim);
	    	if (para >= 0.) 
	    	{
	            if (alpha < 0.0)
			alpha = 0.0;
	            if (alpha > 1.0)
			alpha = 1.0;
		    for (i = 0; i < dim; ++i)
		    {
		       	Coords(pc)[i] = Coords(newba->start)[i] +
		    	alpha*(Coords(newba->end)[i] - Coords(newba->start)[i]);
		    }
	            *sa = alpha;
	            *cr_stat = check_cross(*sa,newba,ca->newc,-1.0,NULL,NULL,
					   pc,low,high,dim);
	            debug_print("D_extend","Left linear_D_extend()\n");
	            return (*cr_stat == GOOD_CROSS) ? newba : NULL;
	    	}
	    }
	    newba = Following_bond(newba,ca->newc->orient);
	    if (newba == NULL)
		break;
	}
	debug_print("D_extend","Left linear_D_extend()\n");
	return newba;
}		/*end linear_D_extend*/

/*ARGSUSED*/
LOCAL	BOND *circle_D_extend(
	VIRTUAL_PROPAGATED_CURVE *ca,
	VIRTUAL_PROPAGATED_CURVE *cb,
	VIRTUAL_PROPAGATED_CURVE *c2,
	D_EXTEND_OUTPUT *dout,
	BOND		*newb2dir,
	COMPONENT	ahead_comp,
	int		*cr_stat,
	double		*low,
	double		*high,
	NODE_FLAG	flag,
	Front		*fr)
{
	RECT_GRID	*rgr = fr->rect_grid;
	BOND		*newba;
	NODE		*fn;
	FT_POINT		*p0c, *pac, *p2c;
	FT_POINT		Pcenter;
	FT_POINT		*pc = dout->pc;
	double		Rsq;
	double		*sa = dout->sa;
	double		len_extend,hmax,*h = rgr->h;
	int		i, dim = rgr->dim;

	debug_print("D_extend","Entered circle_D_extend()\n");
	fn = Node_of(ca->oldc->curve,ca->oldc->orient);
	hmax = h[0];
	for (i = 0; i < dim; ++i)
	    if (hmax < h[i]) hmax = h[i];
	if ((continue_past_fixed_node(flag) == YES) && is_fixed_node(fn))
	{
	    /*
	     * The fundamental idea of this algorithm is that each fixed
	     * node acts as point source of Hugoniot wavelets. Thus the
	     * circle of wavelets should have as their center the fixed
	     * node. The radius of the circle is taken to be from the
	     * fixed node to the image of the propagated physcial node.
	     */

	    p0c = Point_of_bond(newb2dir,c2->newc->orient);
	    Rsq = 0.0;
	    for (i = 0; i < dim; ++i)
	    {
	    	Coords(&Pcenter)[i] = Coords(fn->posn)[i];
	    	Rsq += sqr(Coords(p0c)[i] - Coords(&Pcenter)[i]);
	    }

	    if (debugging("D_extend"))
	    {
	    	(void) printf("circle_D_extend: ");
	    	(void) printf("fixed node - ");	print_node(fn);
	    	(void) printf("center fn %g %g p0c %g %g radius %g\n",
	    		      Coords(&Pcenter)[0],Coords(&Pcenter)[1],
	    		      Coords(p0c)[0],Coords(p0c)[1],sqrt(Rsq));
	    }

	    len_extend = 0.0;
	    for (newba = ca->bvirtual; ;)
	    {
	        if (robust_cross_bond_circle(newba,&Pcenter,Rsq,sa,pc))
		{
	            *cr_stat = check_cross(*sa,newba,ca->newc,-1.0,NULL,NULL,
					   pc,low,high,dim);
	            debug_print("D_extend","Left circle_D_extend()\n");
		    return (*cr_stat != GOOD_CROSS) ? NULL : newba;
		}
		len_extend += bond_length(newba);
		if (len_extend > hmax) break;
		newba = Following_bond(newba,ca->newc->orient);
		if (newba == NULL)
		{    
		    debug_print("D_extend","Left circle_D_extend()\n");
		    return NULL;
		}
	    }
	}
	else if (Following_bond(newb2dir,c2->newc->orient) != NULL)
	{
	    /* 
	     * Find cross point of the circle through the three
	     * points on ca nearest the node with c2.
	     */

	    p0c = Point_of_bond(newb2dir,c2->newc->orient);
	    pac = Point_of_bond(newb2dir,Opposite_orient(c2->newc->orient));
	    p2c = Point_of_bond(Following_bond(newb2dir,c2->newc->orient),
			        Opposite_orient(c2->newc->orient));
		
	    if (find_circle_through_points(p0c,pac,p2c,&Pcenter,&Rsq,h,dim))
	    {
		if (debugging("D_extend"))
		{
		    (void) printf("circle_D_extend: ");
		    (void) printf("center %g %g radius %g\n",
				  Coords(&Pcenter)[0],
				  Coords(&Pcenter)[1],sqrt(Rsq));
		}

	    	len_extend = 0.0;
	        for (newba = ca->bvirtual; ;) 
	        {
	            if (robust_cross_bond_circle(newba,&Pcenter,Rsq,sa,pc))
	            {
	    	        *cr_stat = check_cross(*sa,newba,ca->newc,-1.0,NULL,
					       NULL,pc,low,high,dim);
	    	        debug_print("D_extend","Left circle_D_extend()\n");
	                return (*cr_stat == GOOD_CROSS) ? newba : NULL;
	            }
		    len_extend += bond_length(newba);
		    if (len_extend > hmax) break;
		    newba = Following_bond(newba,ca->newc->orient);
		    if (newba == NULL)
		    {
	    	        debug_print("D_extend","Left circle_D_extend()\n");
	    	        return NULL;
	    	    }
		}
	    }
	}
	debug_print("D_extend","Left circle_D_extend()\n");
	return NULL;
}		/*end circle_D_extend*/

/*ARGSUSED*/
LOCAL	BOND *normal_D_extend(
	VIRTUAL_PROPAGATED_CURVE *ca,
	VIRTUAL_PROPAGATED_CURVE *cb,
	VIRTUAL_PROPAGATED_CURVE *c2,
	D_EXTEND_OUTPUT *dout,
	BOND		*newb2dir,
	COMPONENT	ahead_comp,
	int		*cr_stat,
	double		*low,
	double		*high,
	NODE_FLAG	flag,
	Front		*fr)
{
	RECT_GRID	*rgr = fr->rect_grid;
	FT_POINT		Pacr, Pbcr;
	FT_POINT		*p, *ptmp;
	FT_POINT		*p2 = Point_of_bond(c2->bvirtual,
				            Opposite_orient(c2->newc->orient));
	FT_POINT		*pc = dout->pc;
	HYPER_SURF 	*hs;
	HYPER_SURF_ELEMENT *hse_acr, *hse_bcr;
	BOND		*newba;
	double		tmpsa, tmpsb;
	double		sav_coords[MAXD];
	double		*sa = dout->sa;
	int		i, dim = rgr->dim;

	debug_print("D_extend","Entered normal_D_extend()\n");

	if (wave_type(ca->newc->curve) != NEUMANN_BOUNDARY ||
	    wave_type(c2->newc->curve) >= FIRST_VECTOR_PHYSICS_WAVE_TYPE)
	{
	    debug_print("D_extend","Left normal_D_extend()\n");
	    return NULL;
	}


	p = Node_of_o_curve(ca->newc)->posn;
	ptmp = Point_of_bond(ca->bvirtual,ca->newc->orient);
	for (i = 0; i < dim; ++i)
	{
	    sav_coords[i] = Coords(p)[i];
	    Coords(p)[i] = Coords(ptmp)[i];
	}
	if (long_nearest_interface_point(Coords(p2),ahead_comp,
					 ca->newc->curve->interface,
					 INCLUDE_BOUNDARIES,
					 Hyper_surf(ca->newc->curve),
		                         Coords(&Pacr),&tmpsa,
					 &hse_acr,&hs) != YES)
	{
	    screen("ERROR in normal_D_extend(), "
	           "long_nearest_interface_point() failed\n");
	    clean_up(ERROR);
	}
	for (i = 0; i < dim; ++i) Coords(p)[i] = sav_coords[i];

	p = Node_of_o_curve(cb->newc)->posn;
	ptmp = Point_of_bond(cb->bvirtual,cb->newc->orient);
	for (i = 0; i < dim; ++i)
	{
	    sav_coords[i] = Coords(p)[i];
	    Coords(p)[i] = Coords(ptmp)[i];
	}
	if (long_nearest_interface_point(Coords(p2),ahead_comp,
		                         cb->newc->curve->interface,
					 INCLUDE_BOUNDARIES,
					 Hyper_surf(cb->newc->curve),
		                         Coords(&Pbcr),&tmpsb,
					 &hse_bcr,&hs) != YES)
	{
	    screen("ERROR in normal_D_extend(), "
	           "long_nearest_interface_point() failed\n");
	    clean_up(ERROR);
	}
	for (i = 0; i < dim; ++i) Coords(p)[i] = sav_coords[i];

	if (separation(p2,&Pacr,dim) <= separation(p2,&Pbcr,dim))
	{
	    *sa = tmpsa;
	    *dout->newbacr = newba = Bond_of_hse(hse_acr);
	    for (i = 0; i < dim; ++i)
	    	Coords(pc)[i] = Coords(&Pacr)[i];
	    *cr_stat = check_cross(*sa,newba,ca->newc,-1.0,NULL,NULL,
				   pc,low,high,dim);
	    if (*cr_stat == GOOD_CROSS)
	    {
	    	debug_print("D_extend","Left normal_D_extend()\n");
	    	return newba;
	    }
	}
	else
	{
	    *sa = tmpsb;
	    *dout->newbacr = newba = Bond_of_hse(hse_bcr);
	    for (i = 0; i < dim; ++i)
	    	Coords(pc)[i] = Coords(&Pbcr)[i];

	    swap_virtual_prop_curves(ca,cb);
	    *cr_stat = check_cross(*sa,newba,ca->newc,-1.0,NULL,NULL,pc,
				   low,high,dim);
	    if (*cr_stat == GOOD_CROSS)
	    {
	    	debug_print("D_extend","Left normal_D_extend()\n");
	    	return newba;
	    }
	    swap_virtual_prop_curves(ca,cb);
	}
	debug_print("D_extend","Left normal_D_extend()\n");
	return NULL;
}		/*end normal_D_extend*/

LOCAL	boolean no_D_extend_cross(
	VIRTUAL_PROPAGATED_CURVE *ca,
	VIRTUAL_PROPAGATED_CURVE *c2,
	D_EXTEND_OUTPUT *dout,
	int		*status,
	int		cr_stat,
	BOND		*oldb2dir,
	BOND		*newb2dir,
	Front		*fr,
	POINTER		wave,
	double		dt)
{
	NODE		*interact_nodes[7];
	double		dt_frac_tmp = 1.0;
	int		wta, wt2;

	debug_print("D_extend","Entered no_D_extend_cross()\n");

	*status = find_D_extend_status(ca->oldc,ca->newc,ca->p_opp,oldb2dir,
		                       newb2dir,dout->pc,fr,wave,dt,
				       &dt_frac_tmp);

	wta = wave_type(ca->newc->curve);
	wt2 = wave_type(c2->newc->curve);
	switch (*status)
	{
	case PSEUDOCROSS_NODE_NODE:
	    set_vel_of_crossing_node(Bond_at_node_of_o_curve(ca->oldc),
			             Bond_at_node_of_o_curve(c2->oldc),
				     ca->bvirtual,c2->bvirtual,wta,wt2,
				     Node_of_o_curve(c2->oldc),
			             Node_of_o_curve(c2->newc),dt,fr);
		break;

	case CROSS_NODE_NODE:
	    set_vel_of_crossing_node(Bond_at_node_of_o_curve(ca->oldc),
			             Bond_at_node_of_o_curve(c2->oldc),
				     ca->bvirtual,c2->bvirtual,wta,wt2,
				     Node_of_o_curve(c2->oldc),
			             Node_of_o_curve(c2->newc),dt,fr);
	    interact_nodes[0] = Node_of_o_curve(ca->newc);
	    interact_nodes[1] = Node_of_o_curve(ca->oldc);
	    interact_nodes[2] = Opp_node_of_o_curve(ca->newc);
	    interact_nodes[3] = Opp_node_of_o_curve(ca->oldc);
	    if (scaled_bond_length(newb2dir,fr->rect_grid->h,
	    	    fr->rect_grid->dim) < MIN_SCALED_LENGTH(fr->interf))
	    {
	    	interact_nodes[4] = Opp_node_of_o_curve(c2->newc);
	    	interact_nodes[5] = Opp_node_of_o_curve(c2->oldc);
	    	interact_nodes[6] = NULL;
	    }
	    else
	    	interact_nodes[4] = NULL;
	    augment_rproblem_list(dout->rp,interact_nodes,dt,dt_frac_tmp,
	    			  ca->oldc->curve->interface,
	    			  ca->newc->curve->interface,fr,wave);
	    break;

	case ERROR_NODE:
	    if (debugging("D_extend"))
		(void) printf("ERROR_NODE status\n");
	    debug_print("D_extend","Left no_D_extend_cross(), return value = YES\n");
	    return YES;

	default:
	    if (cr_stat != NO_CROSS)
	    {
		if (debugging("D_extend"))
		    (void) printf("Unknown cross status\n");
	        debug_print("D_extend","Left no_D_extend_cross(), "
				 "return value = YES\n");
		return YES;
	    }
	    *dout->dt_frac = min(*dout->dt_frac,dt_frac_tmp);
	}
	if (debugging("D_extend")) 
	{
	    (void) printf("ca->newc - ");	print_o_curve(ca->newc);
	    (void) printf("ca->p = %g %g\n",
	    	          Coords(ca->p)[0],Coords(ca->p)[1]);
	    (void) printf("c2->newc - ");	print_o_curve(c2->newc);
	    (void) printf("c2->p = %g %g\n",
			  Coords(c2->p)[0],Coords(c2->p)[1]);
	}
	debug_print("D_extend","Left no_D_extend_cross(), return value = YES\n");
	return YES;
}		/*end no_D_extend_cross*/

LOCAL	int  found_D_extend_cross(
	VIRTUAL_PROPAGATED_CURVE *ca,
	VIRTUAL_PROPAGATED_CURVE *cb,
	VIRTUAL_PROPAGATED_CURVE *c2,
	D_EXTEND_OUTPUT *dout,
	BOND		*newba,
	Front		*fr,
	double		dt)
{
	NODE		*oldna, *oldn2;
	int		status = GOOD_NODE;
	int		i, dim = fr->rect_grid->dim;
	static FT_POINT	*oldp = NULL;

	if (oldp == NULL) 
	    oldp = Static_point(fr->interf);

	*dout->newbacr = (newba==ca->bvirtual) ?
			Bond_at_node_of_o_curve(ca->newc) : newba;
	*dout->newb2cr = Bond_at_node_of_o_curve(c2->newc);

	if (debugging("D_extend")) 
	{
	    (void) printf("pc (%llu): (%g, %g)  *sa = %g *s2 = %g\n",
	    	          point_number(dout->pc),
	    	          Coords(dout->pc)[0],Coords(dout->pc)[1],
	    	          *dout->sa,*dout->s2);
	    (void) printf("*dout->newbacr:\n");
	    print_bond_and_states(*dout->newbacr,ca->newc->curve,fr);
	    (void) printf("*dout->newb2cr:\n");
	    print_bond_and_states(*dout->newb2cr,c2->newc->curve,fr);
	}

	/* Compute node velocity */

	oldna = Node_of_o_curve(ca->oldc);
	oldn2 = Node_of_o_curve(c2->oldc);
	for (i = 0; i < dim; ++i)
	{
	    Coords(oldp)[i] =
		0.5*(Coords(oldna->posn)[i] + Coords(oldn2->posn)[i]);
	}
	status = set_node_velocity(dout->pc,oldp,Node_of_o_curve(c2->newc),
			           ca->oldc,c2->oldc,cb->v,c2->v,fr,dt,
				   dout->dt_frac);
	return status;
}		/*end found_D_extend_cross*/

LOCAL	int leave_D_extend(
	VIRTUAL_PROPAGATED_CURVE *ca,
	VIRTUAL_PROPAGATED_CURVE *cb,
	VIRTUAL_PROPAGATED_CURVE *c2,
	int		status)
{
	int		dim = c2->newc->curve->interface->dim;

	if (c2->newc->orient == POSITIVE_ORIENTATION) 
	    c2->oppb->end = c2->oppn->posn;
	else 
	    c2->oppb->start = c2->oppn->posn;
	set_bond_length(c2->oppb,dim);
	if (ca->newc->orient == POSITIVE_ORIENTATION) 
	    ca->oppb->end = ca->oppn->posn;
	else 
	    ca->oppb->start = ca->oppn->posn;
	set_bond_length(ca->oppb,dim);
	if (cb->newc->orient == POSITIVE_ORIENTATION) 
	    cb->oppb->end = cb->oppn->posn;
	else 
	    cb->oppb->start = cb->oppn->posn;
	set_bond_length(cb->oppb,dim);
	debug_print("D_extend",
	      "Left D_extend_crossing_of_two_propagated_curves()\n\t");
	if (debugging("D_extend"))
	    print_node_status("status = ",status,"\n");
	return status;
}		/*end leave_D_extend*/

LOCAL	void swap_virtual_prop_curves(
	VIRTUAL_PROPAGATED_CURVE *ca,
	VIRTUAL_PROPAGATED_CURVE *cb)
{
	VIRTUAL_PROPAGATED_CURVE Ctmp;
	O_CURVE		OCtmp;
	O_CURVE		*oldca, *newca, *oldcb, *newcb;

	oldca = ca->oldc;	oldcb = cb->oldc;
	newca = ca->newc;	newcb = cb->newc;
	OCtmp = *oldca;		*oldca = *oldcb;	*oldcb = OCtmp;
	OCtmp = *newca;		*newca = *newcb;	*newcb = OCtmp;
	Ctmp = *ca;		*ca = *cb;		*cb = Ctmp;
	ca->oldc = oldca;	cb->oldc = oldcb;
	ca->newc = newca;	cb->newc = newcb;
}		/*end swap_virtual_prop_curves*/


/*
*		H_extend_crossing_of_two_propagated_curves():
*
*	The extension is by Huyghens' principle. The curve c2 is extended
*	to cross c1.
*/

EXPORT int H_extend_crossing_of_two_propagated_curves(
	O_CURVE		*oldc1,
	O_CURVE		*newc1,
	O_CURVE		*oldc2,
	O_CURVE		*newc2,
	COMPONENT	ahead_comp,
	COMPONENT	propagation_comp,
	FT_POINT		*pc,		/* crossing point */
	BOND		**newb1cr,	/* the crossing bond on the newc1 */
	BOND		**newb2cr,	/* the crossing bond on the newc2 */
	double		*s1, double *s2,	/* fract dist on bond to cross */
	Front		*fr,
	POINTER		wave,
	RPROBLEM	**rp,
	double		dt,		/* time step */
	double		*dt_frac,
	NODE_FLAG	flag)
{
	RECT_GRID *gr = fr->rect_grid;
	FT_POINT		*q2;
	FT_POINT		*padj;
	BOND		*newb1, *newb2;
	BOND		*b1virtual,*b2virtual,*b1limit,*b2limit;
	BOND		B1, B2;
	NODE		*oppn1,*oppn2;
	NODE_FLAG	ndflag;
	BOND		*oppb1,*oppb2;
	double		low[MAXD], high[MAXD];
	double		*h = gr->h, *L = gr->VL, *U = gr->VU;
	double		residual;
	double		nor[MAXD];
	double		v1[MAXD], v2[MAXD];
	double		p2x,p2y;
	double		moments[9];
	double		sina, cosa;
	int		wt1 = wave_type(newc1->curve);
	int		wt2 = wave_type(newc2->curve);
	ANGLE_DIRECTION	c1_to_c2;
	int		count = 20;/*Used to be 60*/
	int		i,npts = 0;
	int		cr_stat = NO_CROSS;
	boolean		cross_found = NO;
	int		dim = fr->interf->dim;
	static	FT_POINT	*p1 = NULL, *p2 = NULL, *p1_opp = NULL,
	                *p2_opp = NULL, *oldp = NULL;
	static	double	**Q = NULL;
	int             on_b1,on_b2;

        on_b1 = (wt1 < FIRST_SCALAR_PHYSICS_WAVE_TYPE) ? YES : NO;
        on_b2 = (wt2 < FIRST_SCALAR_PHYSICS_WAVE_TYPE) ? YES : NO;

	debug_print("H_extend", 
	      "Entered H_extend_crossing_of_two_propagated_curves()\n");

	if (Q == NULL) 
	{
	    p1 = Static_point(fr->interf);
	    p2 = Static_point(fr->interf);
	    p1_opp = Static_point(fr->interf);
	    p2_opp = Static_point(fr->interf);
	    oldp = Static_point(fr->interf);
	    bi_array(&Q,MAXD,MAXD,FLOAT);
	}

	for (i = 0; i < dim; ++i)
	{
	    low[i] = fr->rect_grid->VL[i] - MIN_SCALED_LENGTH(fr->interf)*h[i];
	    high[i] = fr->rect_grid->VU[i] + MIN_SCALED_LENGTH(fr->interf)*h[i];
	}

	/* Initialize moments array to zero */
	
	for (i = 0; i < 9; ++i)
	    moments[i] = 0.0;

	/* propagate the node to two new positions using both curves */

	newb1 = b1virtual = &B1;
	newb2 = b2virtual = &B2;
	init_curve_for_crossing(p1,p1_opp,b1virtual,oldc1,newc1,&oppn1,&oppb1,
				fr,wave,dt,v1,flag);
	init_curve_for_crossing(p2,p2_opp,b2virtual,oldc2,newc2,&oppn2,&oppb2,
				fr,wave,dt,v2,flag);
	
	if (debugging("H_extend")) 
	{
	    (void) printf("Virtual bonds in %s\n",
			  "H_extend_crossing_of_two_propagated_curves()");
	    (void) printf("b1virtual\n");	print_bond(b1virtual);
	    if (b1virtual->prev != NULL)
	    {
	    	(void) printf("Left state b1virtual->start\n");
	    	(*fr->print_state)(left_state(b1virtual->start));
	    	(void) printf("Right state b1virtual->start\n");
	    	(*fr->print_state)(right_state(b1virtual->start));
	    }
	    else
	    {
	    	(void) printf("Left state b1virtual->start\n");
	    	(*fr->print_state)(left_start_state(newc1->curve));
	    	(void) printf("Right state b1virtual->start\n");
	    	(*fr->print_state)(right_start_state(newc1->curve));
	    }
	    if (b1virtual->next != NULL)
	    {
	    	(void) printf("Left state b1virtual->end\n");
	    	(*fr->print_state)(left_state(b1virtual->end));
	    	(void) printf("Right state b1virtual->end\n");
	    	(*fr->print_state)(right_state(b1virtual->end));
	    }
	    else
	    {
	    	(void) printf("Left state b1virtual->end\n");
	    	(*fr->print_state)(left_end_state(newc1->curve));
	    	(void) printf("Right state b1virtual->end\n");
	    	(*fr->print_state)(right_end_state(newc1->curve));
	    }
	    (void) printf("b2virtual\n");	print_bond(b2virtual);
	    if (b2virtual->prev != NULL)
	    {
	    	(void) printf("Left state b2virtual->start\n");
	    	(*fr->print_state)(left_state(b2virtual->start));
	    	(void) printf("Right state b2virtual->start\n");
	    	(*fr->print_state)(right_state(b2virtual->start));
	    }
	    else
	    {
	    	(void) printf("Left state b2virtual->start\n");
	    	(*fr->print_state)(left_start_state(newc2->curve));
	    	(void) printf("Right state b2virtual->start\n");
	    	(*fr->print_state)(right_start_state(newc2->curve));
	    }
	    if (b2virtual->next != NULL)
	    {
	    	(void) printf("Left state b2virtual->end\n");
	    	(*fr->print_state)(left_state(b2virtual->end));
	    	(void) printf("Right state b2virtual->end\n");
	    	(*fr->print_state)(right_state(b2virtual->end));
	    }
	    else
	    {
	    	(void) printf("Left state b2virtual->end\n");
	    	(*fr->print_state)(left_end_state(newc2->curve));
	    	(void) printf("Right state b2virtual->end\n");
	    	(*fr->print_state)(right_end_state(newc2->curve));
	    }
	    (void) printf("newc1, ");	print_o_curve(newc1);
	    (void) printf("newc2, ");	print_o_curve(newc2);
	}

	if (node_velocity_preset(flag) == YES)
	{
	    for (i = 0; i < dim; ++i)
	    	Coords(pc)[i] = Coords(Node_of_o_curve(oldc2)->posn)[i]
				+ dt * Node_vel(Node_of_o_curve(newc2))[i];
	    *newb1cr = Bond_at_opp_node_of_o_curve(newc1);
	    *newb2cr = Bond_at_node_of_o_curve(newc2);
	    *s1 = (newc1->orient == POSITIVE_ORIENTATION) ? 1.0 : 0.0;
	    *s2 = (newc2->orient == POSITIVE_ORIENTATION) ? 0.0 : 1.0;
	    return leave_H_ext_cr_2_pc(GOOD_NODE,newc1,newc2,oppb1,oppb2,
	                               oppn1,oppn2);
	}

	    /* Propagate c1 for arc >= one mesh length */

	residual = 1.5;
	for ( ; ; ) 
	{
	    for (i = 0; i < dim; ++i)
	    	residual -= fabs((Coords(newb1->end)[i] -
				  Coords(newb1->start)[i]) / h[i]);
	    b1limit = newb1 = Following_bond(newb1,newc1->orient);
	    if (residual <= 0. || newb1 == NULL)
		break;
	}

	    /* Propagate c2 by Huyghens' principle */

	padj = Point_adjacent_to_node(newc2->curve,newc2->orient);
	b2limit = newb2 = Following_bond(newb2,newc2->orient);
	c1_to_c2 = c1_to_c2_direction(oldc1,oldc2);
	q2 = Point_adjacent_to_node(oldc2->curve,oldc2->orient);
	nor[0] = -(Coords(q2)[1] - Coords(Node_of_o_curve(oldc2)->posn)[1]);
	nor[1] =  Coords(q2)[0] - Coords(Node_of_o_curve(oldc2)->posn)[0];
	sina = sin(PI/40.);	cosa = cos(PI/40.);
	if (c1_to_c2 == COUNTER_CLOCK)
	    sina *= -1.;
	Q[0][0] = cosa;	Q[0][1] = -sina;
	Q[1][0] = sina;	Q[1][1] =  cosa;
	while (count-- > 0) 
	{

	    /* Compute statistics for cross trace */

	    p2x = Coords(p2)[0];    p2y = Coords(p2)[1];
	    ++npts;
	    moments[0] += p2x;
	    moments[1] += p2y;
	    moments[2] += p2x*p2x;
	    moments[3] += p2x*p2y;
	    moments[4] += p2y*p2y;
	    moments[5] += p2x*p2x*p2x;
	    moments[6] += p2x*p2x*p2y;
	    moments[7] += p2x*p2y*p2y;
	    moments[8] += p2y*p2y*p2y;

	        /* Check for intersections */

	    for (newb1 = b1virtual; newb1 != b1limit;
	        		newb1 = Following_bond(newb1,newc1->orient))
	    {
	        for (newb2 = b2virtual; newb2 != b2limit;
	            		newb2 = Following_bond(newb2,newc2->orient))
	        {
	            if (robust_cross_bonds(newb1,on_b1,newb2,on_b2,s1,s2,gr,pc)) 
	            {
			cr_stat = check_cross(*s1,newb1,newc1,*s2,newb2,
					      newc2,pc,low,high,dim);
			if (cr_stat == GOOD_CROSS)
			{
			    if (!check_H_extend_cross(newb1,newb2,oldc1,
							 oldc2,h,dim,
							 ahead_comp,
							 propagation_comp,
							 &cross_found))
			    {
	                        return no_H_ext_cr_2_pc(oldc1,newc1,oldc2,newc2,
				                        ahead_comp,
							propagation_comp,pc,
							p1_opp,padj,oldp,p1,p2,
							newb1cr,newb2cr,b2limit,
							b1virtual,b2virtual,
							newb1,newb2,oppb1,oppb2,
							oppn1,oppn2,s1,s2,
							moments,npts,fr,wave,
							rp,dt,dt_frac,v1,v2,
							cross_found,count,flag);
			    }
			    else
			    {
	                        return found_H_ext_cr_2_pc(oldc1,oldc2,
				                           newc1,newc2,
							   oppb1,oppb2,
							   newb1cr,newb2cr,
							   b1virtual,b2virtual,
							   newb1,newb2,
							   pc,oldp,*s1,*s2,
							   oppn1,oppn2,dt,
							   dt_frac,v1,v2,fr);
			    }
			}
			else
			{
			    if (debugging("H_extend"))
			       (void) printf("check_cross rejects cross\n");
	                    return no_H_ext_cr_2_pc(oldc1,newc1,oldc2,newc2,
				                    ahead_comp,propagation_comp,
						    pc,p1_opp,padj,oldp,p1,p2,
						    newb1cr,newb2cr,b2limit,
						    b1virtual,b2virtual,newb1,
						    newb2,oppb1,oppb2,oppn1,
						    oppn2,s1,s2,moments,npts,
						    fr,wave,rp,dt,dt_frac,v1,v2,
						    cross_found,count,flag);
			}
	            }
	        }
	    }
	    /* Insert point from previous propagation */

	    if (dont_insert_on_h_extend(flag) != YES)
	    {
	        insert_point_adjacent_to_node(Point(Coords(p2)),newc2->curve,
		                              newc2->orient);
	        /*
		 * If newc2 started as a single bond curve and has
		 * POSITIVE_ORIENTATION,  then the above insertion modifies
		 * oppb2,  which must be reset.
		 */
		if (newc2->curve->num_points == 3 &&
				newc2->orient == POSITIVE_ORIENTATION)
		    oppb2 = Bond_at_opp_node_of_o_curve(newc2);
	        ft_assign(left_state(Point_adjacent_to_node(newc2->curve,
							 newc2->orient)),
	               left_state(p2),fr->sizest);
	        ft_assign(right_state(Point_adjacent_to_node(newc2->curve,
							  newc2->orient)),
	               right_state(p2),fr->sizest);
	    }


	        /* Propagate node obliquely */

	    if (outside_point(Coords(p2),L,U,dim))
		break;
	    rotate_vector(nor,Q,nor,dim);
	    oblique_propagate_at_node(fr,wave,p2,oldc2,newc2,nor,dt);
	    ndflag = flag;
	    set_virtuals_by_adjacent_bond(ndflag) = NO;
	    set_virtual_bond_at_node(p2,b2virtual,newc2->curve,
		                     newc2->orient,fr,dim,ndflag);

	}

	return no_H_ext_cr_2_pc(oldc1,newc1,oldc2,newc2,ahead_comp,
	                        propagation_comp,pc,p1_opp,padj,oldp,p1,p2,
			        newb1cr,newb2cr,b2limit,b1virtual,b2virtual,
			        newb1,newb2,oppb1,oppb2,oppn1,oppn2,s1,s2,
			        moments,npts,fr,wave,rp,dt,dt_frac,v1,v2,
			        cross_found,count,flag);
}		/*end H_extend_crossing_of_two_propagated_curves*/

LOCAL	int no_H_ext_cr_2_pc(
	O_CURVE	  *oldc1,
	O_CURVE	  *newc1,
	O_CURVE	  *oldc2,
	O_CURVE	  *newc2,
	COMPONENT ahead_comp,
	COMPONENT propagation_comp,
	FT_POINT	  *pc,
	FT_POINT     *p1_opp,
	FT_POINT     *padj,
	FT_POINT     *oldp,
	FT_POINT     *p1,
	FT_POINT     *p2,
	BOND	  **newb1cr,
	BOND	  **newb2cr,
	BOND      *b2limit,
	BOND      *b1virtual,
	BOND      *b2virtual,
	BOND      *newb1,
	BOND      *newb2,
	BOND      *oppb1,
	BOND      *oppb2,
	NODE      *oppn1,
	NODE      *oppn2,
	double	  *s1,
	double     *s2,
	double     *moments,
	int       npts,
	Front	  *fr,
	POINTER	  wave,
	RPROBLEM  **rp,
	double	  dt,
	double	  *dt_frac,
	double     *v1,
	double     *v2,
	boolean      cross_found,
	int       count,
	NODE_FLAG flag)
{
	NODE_FLAG ndflag;
	NODE      *interact_nodes[5];
	double     dt_frac_tmp = 1.0;
	int       status;
	int       dim = fr->rect_grid->dim;
	int       wt1 = wave_type(newc1->curve);
	int       wt2 = wave_type(newc2->curve);

	status = find_H_extend_status(oldc1,newc1,p1_opp,oldc2,newc2,
			              moments,npts,
			              Following_bond(b2limit,
				          Opposite_orient(newc2->orient)),
			              pc,fr,wave,dt,&dt_frac_tmp);

	switch (status)
	{
	case PSEUDOCROSS_NODE_NODE:
	    set_vel_of_crossing_node(Bond_at_node_of_o_curve(oldc1),
			             Bond_at_node_of_o_curve(oldc2),
				     b1virtual,b2virtual,wt1,wt2,
				     Node_of_o_curve(oldc2),
				     Node_of_o_curve(newc2),dt,fr);
	    while (b2limit != 
		   Following_bond(Bond_at_node_of_o_curve(newc2),newc2->orient))
		(void) delete_point_adjacent_to_node(fr,newc2->curve,
					             newc2->orient);
	    break;

	case CROSS_NODE_NODE:
	    set_vel_of_crossing_node(Bond_at_node_of_o_curve(oldc1),
			             Bond_at_node_of_o_curve(oldc2),
				     b1virtual,b2virtual,wt1,wt2,
				     Node_of_o_curve(oldc2),
				     Node_of_o_curve(newc2),dt,fr);
	    while (b2limit != 
		   Following_bond(Bond_at_node_of_o_curve(newc2),newc2->orient))
	        (void) delete_point_adjacent_to_node(fr,newc2->curve,
						     newc2->orient);
	    interact_nodes[0] = Node_of_o_curve(newc1);
	    interact_nodes[1] = Node_of_o_curve(oldc1);
	    interact_nodes[2] = Opp_node_of_o_curve(newc1);
	    interact_nodes[3] = Opp_node_of_o_curve(oldc1);
	    interact_nodes[4] = NULL;
	    augment_rproblem_list(rp,interact_nodes,dt,dt_frac_tmp,
				  oldc1->curve->interface,
				  newc1->curve->interface,fr,wave);
	    break;

	case NO_CROSS_NODE:
	    if (dont_insert_on_h_extend(flag) == YES)
		break;
	    while (Point_adjacent_to_node(newc2->curve,newc2->orient) != padj)
	    {
	    	(void) delete_point_adjacent_to_node(fr,newc2->curve,
						     newc2->orient);
	    }
	    if (newc2->orient == POSITIVE_ORIENTATION) 
	    	oppb2->end = oppn2->posn;
	    else 
	    	oppb2->start = oppn2->posn;
	    set_bond_length(oppb2,dim);
	    if (newc1->orient == POSITIVE_ORIENTATION) 
	    	oppb1->end = oppn1->posn;
	    else 
	    	oppb1->start = oppn1->posn;
	    set_bond_length(oppb1,dim);
	    ndflag = flag;
	    dont_insert_on_h_extend(ndflag) = YES;
	    return H_extend_crossing_of_two_propagated_curves(oldc1,newc1,
				                              oldc2,newc2,
							      ahead_comp,
							      propagation_comp,
							      pc,newb1cr,
							      newb2cr,s1,s2,
							      fr,wave,rp,dt,
							      dt_frac,ndflag);
	default:
	    if (cross_found)
	    {
	        return found_H_ext_cr_2_pc(oldc1,oldc2,newc1,newc2,oppb1,oppb2,
	                                   newb1cr,newb2cr,b1virtual,b2virtual,
				           newb1,newb2,pc,oldp,*s1,*s2,
					   oppn1,oppn2,dt,dt_frac,v1,v2,fr);
	    }
	    *dt_frac = min(*dt_frac,dt_frac_tmp);
	}

	if (debugging("H_extend")) 
	{
	    (void) printf("p1 = %g %g\n",Coords(p1)[0],Coords(p1)[1]);
	    (void) printf("newc1, ");	print_o_curve(newc1);
	    (void) printf("p2 = %g %g\n",Coords(p2)[0],Coords(p2)[1]);
	    (void) printf("newc2, ");	print_o_curve(newc2);
	    if (count > 0 && newb1 != NULL && newb2 != NULL) 
	    {
	        (void) printf("cross_signs = %d %d\n",cross_sign(newb1,newb2),
				  cross_sign(Bond_at_node_of_o_curve(oldc1),
					     Bond_at_node_of_o_curve(oldc2)));
		(void) printf("s1,s2 = %g %g\n",*s1,*s2);
		(void) printf("newb1, ");	print_bond(newb1);
		(void) printf("newb2, ");	print_bond(newb2);
		(void) printf("pc = %g %g\n",Coords(pc)[0],Coords(pc)[1]);
	    }
	}
	return leave_H_ext_cr_2_pc(status,newc1,newc2,oppb1,oppb2,oppn1,oppn2);
}		/*end no_H_ext_cr_2_pc*/

/*ARGSUSED*/
LOCAL	int	found_H_ext_cr_2_pc(
	O_CURVE *oldc1,
	O_CURVE *oldc2,
	O_CURVE *newc1,
	O_CURVE *newc2,
	BOND    *oppb1,
	BOND    *oppb2,
	BOND    **newb1cr,
	BOND    **newb2cr,
	BOND    *b1virtual,
	BOND    *b2virtual,
	BOND    *newb1,
	BOND    *newb2,
	FT_POINT   *pc,
	FT_POINT   *oldp,
	double   s1,
	double   s2,
	NODE    *oppn1,
	NODE    *oppn2,
	double   dt,
	double   *dt_frac,
	double   *v1,
	double   *v2,
	Front   *fr)
{
	FT_POINT *oldp1, *oldp2;
	int status;
	int i, dim = fr->rect_grid->dim;
	*newb1cr = (newb1==b1virtual) ? Bond_at_node_of_o_curve(newc1) : newb1;
	*newb2cr = (newb2==b2virtual) ? Bond_at_node_of_o_curve(newc2) : newb2;

	if (debugging("H_extend")) 
	{
	    (void) printf("pc (%llu): %g %g  s1 = %g s2 = %g\n",
			  point_number(pc),Coords(pc)[0],Coords(pc)[1],s1,s2);
	    (void) printf("*newb1cr:\n");
	    print_bond_and_states(*newb1cr,newc1->curve,fr);
	    (void) printf("*newb2cr:\n");
	    print_bond_and_states(*newb2cr,newc2->curve,fr);
	}

	/* Compute node velocity */

	oldp1 = Node_of_o_curve(oldc1)->posn;
	oldp2 = Node_of_o_curve(oldc2)->posn;
	for (i = 0; i < dim; ++i)
	    Coords(oldp)[i] = 0.5*(Coords(oldp1)[i] + Coords(oldp2)[i]);
	status = set_node_velocity(pc,oldp,Node_of_o_curve(newc2),
			           oldc1,oldc2,v1,v2,fr,dt,dt_frac);

	return leave_H_ext_cr_2_pc(status,newc1,newc2,oppb1,oppb2,oppn1,oppn2);
}		/*end found_H_ext_cr_2_pc*/

LOCAL	int	leave_H_ext_cr_2_pc(
	int     status,
	O_CURVE *newc1,
	O_CURVE *newc2,
	BOND    *oppb1,
	BOND    *oppb2,
	NODE    *oppn1,
	NODE    *oppn2)
{
	int dim = newc1->curve->interface->dim;
	if (newc2->orient == POSITIVE_ORIENTATION) 
	    oppb2->end = oppn2->posn;
	else 
	    oppb2->start = oppn2->posn;
	set_bond_length(oppb2,dim);
	if (newc1->orient == POSITIVE_ORIENTATION) 
	    oppb1->end = oppn1->posn;
	else 
	    oppb1->start = oppn1->posn;
	set_bond_length(oppb1,dim);

	debug_print("H_extend", 
	      "Left H_extend_crossing_of_two_propagated_curves(), "
	      "status = %d\n",status);
	return status;
}		/*end leave_H_ext_cr_2_pc*/

/*ARGSUSED*/
LOCAL int check_H_extend_cross(
	BOND		*newb1,
	BOND		*newb2,
	O_CURVE		*oldc1,
	O_CURVE		*oldc2,
	double		*h,
	int		dim,
	COMPONENT	ahead_comp,
	COMPONENT	propagation_comp,
	boolean		*cross_found)
{
	double		len;

	if (cross_sign(newb1,newb2) ==
		cross_sign(Bond_at_node_of_o_curve(oldc1),
			   Bond_at_node_of_o_curve(oldc2)))
	{
	    if (debugging("H_extend"))
		(void) printf("cross signs agree\n");
	    *cross_found = YES;
	    /***
	        if (ahead_comp != propagation_comp)
	    {
	    	if (debugging("H_extend"))
	    	{
	    	    (void) printf("cross rejected since ");
	    	    (void) printf("ahead_comp != propagation_comp\n");
	    	}
	    	return NO;
	    }
	    ***/
	    return YES;
	}
	set_bond_length(newb1,dim);
	set_bond_length(newb2,dim);
	len = 0.01 * (h[0] + h[1]);
	if (bond_length(newb1) < len || bond_length(newb2) < len)
	{
	    *cross_found = YES;
	    if (debugging("H_extend"))
		(void) printf("one bond short\n");
	    /***
	    if (ahead_comp != propagation_comp)
	    {
	    	if (debugging("H_extend"))
	    	{
	    	    (void) printf("cross rejected since ");
	    	    (void) printf("ahead_comp != propagation_comp\n");
	    	}
	    	return NO;
	    }
	    ***/
	    return YES;
	}
	if (debugging("H_extend"))
	    (void) printf("cross rejected\n");
	return NO;
}		/*end check_H_extend_cross*/


/*
*               cross_or_extend_to_cross_two_propagated_curves():
*
*	The routine checks for a forward crossing of the two propagated curves
*	newc1 and newc2, if this fails it then checks for a crossing
*	of the backward extension of the two propagated curves.
*       If there is a crossing GOOD_NODE is returned and the crossing point 
*	and bonds are found a crossing; otherwise a diagnostic signal is 
*	returned.
*/

#define Preceding_bond(bond,orient)					\
	(((orient) == POSITIVE_ORIENTATION) ? (bond)->prev : (bond)->next)

EXPORT	int cross_or_extend_to_cross_two_propagated_curves(
	O_CURVE		*oldc1,
	O_CURVE		*newc1,
	O_CURVE		*oldc2,
	O_CURVE		*newc2,
	FT_POINT		**pc,
	BOND		**newb1cr,
	BOND		**newb2cr,
	double		*s1,
	double		*s2,
	Front		*fr,
	POINTER		wave,
	RPROBLEM	**rp,
	double		dt,
	double		*dt_frac,
	NODE_FLAG	flag,
	boolean		*c_ext)
{
	BOND		*newb1, *newb2;
	BOND		*b1, *oldb1, *b2, *oldb2;
	BOND		*preceder, *follower;
	NODE		*oppn1,*oppn2;
	BOND		*oppb1,*oppb2;
	RECT_GRID	*rgr = fr->rect_grid;
	double		*h    = rgr->h;
	double		low[MAXD];
	double		high[MAXD];
	double		v1[MAXD], v2[MAXD];
	double		dt_frac_tmp;
	int		wt1 = wave_type(newc1->curve);
	int		wt2 = wave_type(newc2->curve);
	int		cr_stat = NO_CROSS;
	int		at_end_of_c1 = NO;
	int		at_end_of_c2 = NO;
	int		i, dim = rgr->dim;
	static	BOND	*b1virtual = NULL, *b2virtual = NULL;
	static	BOND	*newb1dir = NULL, *newb2dir = NULL;
	static	FT_POINT   *p1 = NULL, *p2 = NULL, *p1_opp = NULL,
	                *p2_opp = NULL, *oldp = NULL;
	int             on_b1,on_b2;

        on_b1 = (wt1 < FIRST_SCALAR_PHYSICS_WAVE_TYPE) ? YES : NO;
        on_b2 = (wt2 < FIRST_SCALAR_PHYSICS_WAVE_TYPE) ? YES : NO;

	debug_print("cross_or_extend",
	      "Entered cross_or_extend_to_cross_two_propagated_curves()\n");
	if (debugging("cross_or_extend"))
	{
	    (void) printf("Curves into "
	    	          "cross_or_extend_to_cross_two_propagated_curves()\n");
	    (void) printf("Oldc1\n");
	    print_o_curve(oldc1);	print_bond_list(oldc1->curve);
	    (void) printf("Newc1\n");	
	    print_o_curve(newc1);	print_bond_list(newc1->curve);
	    (void) printf("Oldc2\n");	
	    print_o_curve(oldc2);	print_bond_list(oldc2->curve);
	    (void) printf("Newc2\n");	
	    print_o_curve(newc2);	print_bond_list(newc2->curve);
	}
	if (p1 == NULL) 
	{
	    p1 = Static_point(fr->interf);
	    p2 = Static_point(fr->interf);
	    p1_opp = Static_point(fr->interf);
	    p2_opp = Static_point(fr->interf);
	    oldp = Static_point(fr->interf);
	    scalar(&b1virtual,sizeof(BOND));
	    scalar(&b2virtual,sizeof(BOND));
	    scalar(&newb1dir,sizeof(BOND));
	    scalar(&newb2dir,sizeof(BOND));
	}
	for (i = 0; i < dim; ++i)
	{
	    low[i] = fr->rect_grid->VL[i] - MIN_SCALED_LENGTH(fr->interf)*h[i];
	    high[i] = fr->rect_grid->VU[i] + MIN_SCALED_LENGTH(fr->interf)*h[i];
	}
	if (c_ext != NULL)
	{
	    c_ext[0] = c_ext[1] = NO;
	}

	for (i = 0; i < dim; ++i)
	    Coords(oldp)[i] = 0.5*(Coords(Node_of_o_curve(oldc1)->posn)[i] +
	    	Coords(Node_of_o_curve(oldc2)->posn)[i]);
		
	/* Propagate the node to two new positions using both curves */

	newb1 = b1virtual;			newb2 = b2virtual;
	init_curve_for_crossing(p1,p1_opp,b1virtual,oldc1,newc1,
		                &oppn1,&oppb1,fr,wave,dt,v1,flag);
	init_curve_for_crossing(p2,p2_opp,b2virtual,oldc2,newc2,
		                &oppn2,&oppb2,fr,wave,dt,v2,flag);

	if (node_velocity_preset(flag) == YES)
	{
	    for (i = 0; i < dim; ++i)
	    	Coords(pc[0])[i] =
		    Coords(oldp)[i] + dt*Node_vel(Node_of_o_curve(newc2))[i];
	    *newb1cr = Bond_at_node_of_o_curve(newc1);
	    *newb2cr = Bond_at_node_of_o_curve(newc2);
	    *s1 = (newc1->orient == POSITIVE_ORIENTATION) ? 0.0 : 1.0;
	    *s2 = (newc2->orient == POSITIVE_ORIENTATION) ? 0.0 : 1.0;
	    if (debugging("cross_or_extend"))
	    {
	    	(void) printf("node velocity preset\n");
	    	print_general_vector("Node_vel(Node_of_o_curve(newc2)) = ",
				     Node_vel(Node_of_o_curve(newc2)),
				     dim,"\n");
	    	(void) printf("Crossing point (%g, %g)\n",
	    		      Coords(pc[0])[0],Coords(pc[0])[1]);
	    	(void) printf("s1 = %g, s2 = %g\n",*s1,*s2);
	    }
	    return leave_cross_or_extend_to_cross_two_propagated_curves(
	             GOOD_NODE,newc1,newc2,oppb1,oppb2,oppn1,oppn2);
	}

	    /* Find a directon for the propagated c1 */

	find_bonds_for_extension_direction(b1virtual,oldc1,newc1,newb1dir,
		                           NULL,fr);

	    /* Find a directon for the propagated c2 */

	find_bonds_for_extension_direction(b2virtual,oldc2,newc2,newb2dir,
		                           NULL,fr);

	if (debugging("cross_or_extend")) 
	{
	    (void) printf("Virtuals and direction bonds\n");

	    if (newb1dir == NULL) 
	    	screen("c1 is a short curve, newb1dir = NULL\n");
	    else
	    {
	    	(void) printf("newb1dir\n");
	    	print_bond(newb1dir);
	    }

	    (void) printf("b1virtual\n");	print_bond(b1virtual);
	    print_bond_and_states(b1virtual,newc1->curve,fr);

	    if (newb2dir == NULL) 
	    	screen("c2 is a short curve, newb2dir = NULL\n");
	    else
	    {
	    	(void) printf("newb2dir\n");
	    	print_bond(newb2dir);
	    }

	    (void) printf("b2virtual\n");	print_bond(b2virtual);
	    print_bond_and_states(b2virtual,newc2->curve,fr);

	}


	/* Look for crossing of two partially propagated curves */
	/* or the extension of one to cross the other           */

	while (!(at_end_of_c1 && at_end_of_c2)) 
	{
	    if (!at_end_of_c1) 
	    {
	    	for (b2 = b2virtual; ; b2 = Following_bond(b2,newc2->orient)) 
	    	{
	    	    if (robust_cross_bonds(newb1,on_b1,b2,on_b2,s1,s2,rgr,pc[0]))
	    	    {
	    	        cr_stat = check_cross(*s1,newb1,newc1,*s2,b2,
					      newc2,pc[0],low,high,dim);
			if (cr_stat == GOOD_CROSS)
			{
	    		    *newb1cr = newb1;
	    		    *newb2cr = b2;
	    		    if (debugging("cross_or_extend"))
	    		        (void) printf("New curves cross\n");
	                    return found_c_or_e_2_pc(newb1cr,newb2cr,
			                             b1virtual,b2virtual,
						     newb1dir,newb2dir,
						     newc1,newc2,oldc1,oldc2,
						     oppb1,oppb2,oppn1,oppn2,
						     flag,pc,oldp,*s1,*s2,
						     dt,dt_frac,v1,v2,fr,c_ext);
			}
	    		else
			{
	                    return found_c_or_e_2_pc(newb1cr,newb2cr,
			                             b1virtual,b2virtual,
						     newb1dir,newb2dir,
						     newc1,newc2,oldc1,oldc2,
						     oppb1,oppb2,oppn1,oppn2,
						     flag,pc,oldp,*s1,*s2,
						     dt,dt_frac,v1,v2,fr,c_ext);
			}
	    	    }
	    	    if (b2 == newb2)
			break;
		}
	    }
	    if (!at_end_of_c2) 
	    {
	    	if (newb2 != b2virtual)
	    	{
	    	    preceder = Preceding_bond(newb2,newc2->orient);
	    	    b2 = (Preceding_bond(preceder,newc2->orient)!=NULL) ?
	    		 preceder : b2virtual;
	    	    if (robust_extend_bond_to_cross_bond(newb1dir,
	    	    		                         newc1->orient,
							 b2,s1,s2,pc[0],h,dim))
	    	    {
	    	        cr_stat = check_cross(-1.0,NULL,NULL,*s2,preceder,
	    				      newc2,pc[0],low,high,dim);
	    	    	if (cr_stat != GOOD_CROSS)
			{
	                    return found_c_or_e_2_pc(newb1cr,newb2cr,
			                             b1virtual,b2virtual,
						     newb1dir,newb2dir,
						     newc1,newc2,oldc1,oldc2,
						     oppb1,oppb2,oppn1,oppn2,
						     flag,pc,oldp,*s1,*s2,
						     dt,dt_frac,v1,v2,fr,c_ext);
			}

	    		*newb1cr = newb1dir;
	    		*newb2cr = preceder;
	    		if (debugging("cross_or_extend"))
	    		{
	    		    (void) printf("Cross found by single ");
	    		    (void) printf("extension of c1\n");
	    		}
	                return found_c_or_e_2_pc(newb1cr,newb2cr,
			                         b1virtual,b2virtual,
						 newb1dir,newb2dir,
						 newc1,newc2,oldc1,oldc2,
						 oppb1,oppb2,oppn1,oppn2,
						 flag,pc,oldp,*s1,*s2,
						 dt,dt_frac,v1,v2,fr,c_ext);
	    	    }
	    	}
	    	if ((follower = Following_bond(newb2,newc2->orient))!=NULL)
		{
		    newb2 = follower;
		}
		else 
		    at_end_of_c2 = YES;

		for (b1 = b1virtual; ; b1=Following_bond(b1,newc1->orient)) 
		{
		    if (robust_cross_bonds(b1,on_b1,newb2,on_b2,s1,s2,rgr,pc[0]))
		    {
		        cr_stat = check_cross(*s1,b1,newc1,*s2,newb2,newc2,
					      pc[0],low,high,dim);
			if (cr_stat == GOOD_CROSS)
			{
			    *newb2cr = newb2;
			    *newb1cr = b1;
			    if (debugging("cross_or_extend"))
			        (void) printf("New curves cross\n");
	                    return found_c_or_e_2_pc(newb1cr,newb2cr,
			                             b1virtual,b2virtual,
						     newb1dir,newb2dir,
						     newc1,newc2,oldc1,oldc2,
						     oppb1,oppb2,oppn1,oppn2,
						     flag,pc,oldp,*s1,*s2,
						     dt,dt_frac,v1,v2,fr,c_ext);
			}
			else
			{
	                    return found_c_or_e_2_pc(newb1cr,newb2cr,
			                             b1virtual,b2virtual,
						     newb1dir,newb2dir,
						     newc1,newc2,oldc1,oldc2,
						     oppb1,oppb2,oppn1,oppn2,
						     flag,pc,oldp,*s1,*s2,
						     dt,dt_frac,v1,v2,fr,c_ext);
			}
		    }
		    if (b1 == newb1)
			break;
		}
	    }
	    if (!at_end_of_c1) 
	    { 
	    	if (newb1 != b1virtual)
	    	{
	    	    preceder = Preceding_bond(newb1,newc1->orient);
	    	    if (robust_extend_bond_to_cross_bond(newb2dir,
	    		                                 newc2->orient,
	    		    (Preceding_bond(preceder,newc1->orient)!=NULL) ?
	    		    preceder : b1virtual,s2,s1,pc[0],h,dim)) 
	    	    {
	    	        cr_stat = check_cross(*s1,preceder,newc1,-1.0,NULL,NULL,
	    				      pc[0],low,high,dim);
	    	    	if (cr_stat != GOOD_CROSS)
			{
	                    return found_c_or_e_2_pc(newb1cr,newb2cr,
			                             b1virtual,b2virtual,
						     newb1dir,newb2dir,
						     newc1,newc2,oldc1,oldc2,
						     oppb1,oppb2,oppn1,oppn2,
						     flag,pc,oldp,*s1,*s2,
						     dt,dt_frac,v1,v2,fr,c_ext);
			}
	    		*newb2cr = newb2dir;
	    		*newb1cr = preceder;
	                if (debugging("cross_or_extend"))
	    		{
	    	            (void) printf("Cross found by single "
	    		                  "extension of c2\n");
	    		}
	                return found_c_or_e_2_pc(newb1cr,newb2cr,
			                         b1virtual,b2virtual,
						 newb1dir,newb2dir,
						 newc1,newc2,oldc1,oldc2,
						 oppb1,oppb2,oppn1,oppn2,
						 flag,pc,oldp,*s1,*s2,
						 dt,dt_frac,v1,v2,fr,c_ext);
	    	    }
	    	}
		if ((follower = Following_bond(newb1,newc1->orient)) != NULL)
		{
		    newb1 = follower;
		}
		else 
		    at_end_of_c1 = YES;
	    }
	}


	if (robust_extend_bond_to_cross_bond(newb1dir,newc1->orient,
			                     newb2,s1,s2,pc[0],h,dim)) 
	{
	    cr_stat = check_cross(-1.0,NULL,NULL,*s2,newb2,
				  newc2,pc[0],low,high,dim);
	    if (cr_stat != GOOD_CROSS)
	    {
	        return found_c_or_e_2_pc(newb1cr,newb2cr,b1virtual,b2virtual,
				         newb1dir,newb2dir,newc1,newc2,
					 oldc1,oldc2,oppb1,oppb2,oppn1,oppn2,
				         flag,pc,oldp,*s1,*s2,dt,dt_frac,
					 v1,v2,fr,c_ext);
	    }
	    *newb1cr = newb1dir;
	    *newb2cr = newb2;
	    if (debugging("cross_or_extend"))
	       (void) printf("Cross found by single extension of c1\n");
	}

	else if (robust_extend_bond_to_cross_bond(newb2dir,newc2->orient,
			                           newb1,s2,s1,pc[0],h,dim)) 
	{
	    cr_stat = check_cross(*s1,newb1,newc1,-1.0,NULL,NULL,pc[0],
				  low,high,dim);
	    if (cr_stat != GOOD_CROSS)
	    {
	        return found_c_or_e_2_pc(newb1cr,newb2cr,b1virtual,b2virtual,
				         newb1dir,newb2dir,newc1,newc2,
					 oldc1,oldc2,oppb1,oppb2,oppn1,oppn2,
					 flag,pc,oldp,*s1,*s2,dt,dt_frac,
					 v1,v2,fr,c_ext);
	    }
	    *newb2cr = newb2dir;
	    *newb1cr = newb1;
	    if (debugging("cross_or_extend"))
	        (void) printf("Cross found by single extension of c2\n");
	}


	/* Look for double extension cross */

	else if (robust_extend_bonds_to_cross(newb1dir,newc1->orient,on_b1,
			                      newb2dir,newc2->orient,on_b2,
					      oldp,s1,s2,pc[0],fr->rect_grid)) 
	{
	    cr_stat = check_cross(-1.0,NULL,NULL,-1.0,NULL,NULL,pc[0],
				  low,high,dim);
	    if (cr_stat != GOOD_CROSS)
	    {
	        return found_c_or_e_2_pc(newb1cr,newb2cr,b1virtual,b2virtual,
				         newb1dir,newb2dir,newc1,newc2,
					 oldc1,oldc2,oppb1,oppb2,oppn1,oppn2,
					 flag,pc,oldp,*s1,*s2,dt,dt_frac,
					 v1,v2,fr,c_ext);
	    }

	    /* Check for cross past short curves */

	    if (is_short_curve(newc1->curve,newc1->orient,rgr,1.0)) 
	    {
		oldb2 = Bond_at_node_of_o_curve(oldc2);
		b2 = b2virtual;
		for (; oldb2 && b2; oldb2=Following_bond(oldb2,oldc2->orient),
				     b2=Following_bond(b2,newc2->orient)) 
		{
		    if (robust_cross_trace(rgr,Opp_node_of_o_curve(oldc1)->posn,
					   p1_opp,oldb2,b2,&dt_frac_tmp,pc[0]))
		    {
	                return found_c_or_e_2_pc(newb1cr,newb2cr,
			                         b1virtual,b2virtual,
						 newb1dir,newb2dir,
						 newc1,newc2,oldc1,oldc2,
						 oppb1,oppb2,oppn1,oppn2,
						 flag,pc,oldp,*s1,*s2,
						 dt,dt_frac,v1,v2,fr,c_ext);
		    }
		}
	    }
	    if (is_short_curve(newc2->curve,newc2->orient,rgr,1.0)) 
	    {
	    	oldb1 = Bond_at_node_of_o_curve(oldc1);
	    	b1 = b1virtual;
	    	for (; oldb1 && b1; oldb1=Following_bond(oldb1,oldc1->orient),
				      b1=Following_bond(b1,newc1->orient)) 
		{
		    if (robust_cross_trace(rgr,Opp_node_of_o_curve(oldc2)->posn,
				           p2_opp,oldb1,b1,&dt_frac_tmp,pc[0]))
		    {
	                return found_c_or_e_2_pc(newb1cr,newb2cr,
			                         b1virtual,b2virtual,
						 newb1dir,newb2dir,
						 newc1,newc2,oldc1,oldc2,
						 oppb1,oppb2,oppn1,oppn2,
						 flag,pc,oldp,*s1,*s2,
						 dt,dt_frac,v1,v2,fr,c_ext);
		    }
		}
	    }
	    *newb1cr = newb1dir;
	    *newb2cr = newb2dir;
	    if (debugging("cross_or_extend"))
	    	(void) printf("Cross found by double extension\n");
	}
	else 
	{		
	    return no_c_or_e_2_pc(cr_stat,oldc1,oldc2,newc1,newc2,oppb1,oppb2,
	                          b1virtual,b2virtual,newb1dir,newb2dir,
				  oppn1,oppn2,p1_opp,p2_opp,pc,fr,wave,dt,
				  dt_frac,rp);
	}

	return found_c_or_e_2_pc(newb1cr,newb2cr,b1virtual,b2virtual,
	                         newb1dir,newb2dir,newc1,newc2,oldc1,oldc2,
				 oppb1,oppb2,oppn1,oppn2,flag,pc,oldp,
				 *s1,*s2,dt,dt_frac,v1,v2,fr,c_ext);
}		/*end cross_or_extend_to_cross_two_propagated_curves*/


LOCAL	int	no_c_or_e_2_pc(
	int      cr_stat,
	O_CURVE  *oldc1,
	O_CURVE  *oldc2,
	O_CURVE  *newc1,
	O_CURVE  *newc2,
	BOND     *oppb1,
	BOND     *oppb2,
	BOND     *b1virtual,
	BOND     *b2virtual,
	BOND     *newb1dir,
	BOND     *newb2dir,
	NODE     *oppn1,
	NODE     *oppn2,
	FT_POINT    *p1_opp,
	FT_POINT    *p2_opp,
	FT_POINT    **pc,
	Front    *fr,
	POINTER  wave,
	double    dt,
	double    *dt_frac,
	RPROBLEM **rp)
{
	NODE  *interact_node1, *interact_node2;
	NODE  *interact_nodes[7];
	double dt_frac_tmp = 1.0;
	int   status;
	int   wt1 = wave_type(newc1->curve);
	int   wt2 = wave_type(newc2->curve);

	status = find_cross_or_extend_to_cross_status(cr_stat,oldc1,newc1,
						      oldc2,newc2,p1_opp,
						      p2_opp,b1virtual,
						      b2virtual,newb1dir,
						      newb2dir,pc[0],
				                      &interact_node1,
						      &interact_node2,
						      fr,wave,dt,
				                      &dt_frac_tmp);

	switch (status)
	{
	case PSEUDOCROSS_NODE_NODE:
	    set_vel_of_crossing_node(Bond_at_node_of_o_curve(oldc1),
			             Bond_at_node_of_o_curve(oldc2),
				     b1virtual,b2virtual,wt1,wt2,
				     Node_of_o_curve(oldc2),
			             Node_of_o_curve(newc2),dt,fr);
		break;

	case CROSS_NODE_NODE:
	    set_vel_of_crossing_node(Bond_at_node_of_o_curve(oldc1),
			             Bond_at_node_of_o_curve(oldc2),
				     b1virtual,b2virtual,wt1,wt2,
				     Node_of_o_curve(oldc2),
				     Node_of_o_curve(newc2),dt,fr);
		interact_nodes[0] = Node_of_o_curve(newc1);
		interact_nodes[1] = Node_of_o_curve(oldc1);
		interact_nodes[2] =
		interact_node1 ? interact_node1 : interact_node2;
		interact_nodes[3] = interact_node1 ? 
				Opp_node_of_o_curve(oldc1) : interact_node2 ?
				Opp_node_of_o_curve(oldc2) : NULL;
		interact_nodes[4] = interact_node1 && interact_node2 ?
				interact_node2 : NULL;
		interact_nodes[5] = interact_node1 && interact_node2 ?
				Opp_node_of_o_curve(oldc2) : NULL;
		interact_nodes[6] = NULL;
		augment_rproblem_list(rp,interact_nodes,dt,dt_frac_tmp,
				  oldc1->curve->interface,
				  newc1->curve->interface,fr,wave);
		break;

	case CROSS_PAST_CURVE_NODE:
		*dt_frac = min(*dt_frac,dt_frac_tmp);
		break;
	}
	return leave_cross_or_extend_to_cross_two_propagated_curves(status,
	         newc1,newc2,oppb1,oppb2,oppn1,oppn2);
}		/*end no_c_or_e_2_pc*/

/*ARGSUSED*/
LOCAL	int	found_c_or_e_2_pc(
	BOND      **newb1cr,
	BOND      **newb2cr,
	BOND      *b1virtual,
	BOND      *b2virtual,
	BOND      *newb1dir,
	BOND      *newb2dir,
	O_CURVE   *newc1,
	O_CURVE   *newc2,
	O_CURVE   *oldc1,
	O_CURVE   *oldc2,
	BOND      *oppb1,
	BOND      *oppb2,
	NODE      *oppn1,
	NODE      *oppn2,
	NODE_FLAG flag,
	FT_POINT     **pc,
	FT_POINT     *oldp,
	double     s1,
	double     s2,
	double     dt,
	double     *dt_frac,
	double     *v1,
	double     *v2,
	Front     *fr,
	boolean	  *c_ext)
{
	int status;
	if (*newb1cr == b1virtual)     
	{
	    *newb1cr = Bond_at_node_of_o_curve(newc1);
	    if (set_virtuals_by_adjacent_bond(flag) == YES)
	    	*newb1cr = Following_bond(*newb1cr,newc1->orient);
	}
	else if (*newb1cr == newb1dir) 
	{
	    if (c_ext != NULL)
		c_ext[0] = YES;
	    if (Following_bond(newb1dir,newc1->orient) == NULL)
	    	*newb1cr = Bond_at_opp_node_of_o_curve(newc1);
	    else if (newc1->orient == POSITIVE_ORIENTATION)
	    	*newb1cr = newb1dir->next->prev;
	    else
	    	*newb1cr = newb1dir->prev->next;
	}
	if (*newb2cr == b2virtual)     
	{
	    *newb2cr = Bond_at_node_of_o_curve(newc2);
	    if (set_virtuals_by_adjacent_bond(flag) == YES)
	    	*newb2cr = Following_bond(*newb2cr,newc2->orient);
	}
	else if (*newb2cr == newb2dir) 
	{
	    if (c_ext != NULL)
		c_ext[1] = YES;
	    if (Following_bond(newb2dir,newc2->orient) == NULL)
	    	*newb2cr = Bond_at_opp_node_of_o_curve(newc2);
	    else if (newc2->orient == POSITIVE_ORIENTATION)
	    	*newb2cr = newb2dir->next->prev;
	    else
	    	*newb2cr = newb2dir->prev->next;
	}

	if (debugging("cross_or_extend")) 
	{
	    (void) printf("pc[0] (%llu): %g %g  s1 = %g s2 = %g\n",
	    	          point_number(pc[0]),
			  Coords(pc[0])[0],Coords(pc[0])[1],s1,s2);
	    (void) printf("*newb1cr:\n");
	    print_bond_and_states(*newb1cr,newc1->curve,fr);
	    (void) printf("*newb2cr:\n");
	    print_bond_and_states(*newb2cr,newc2->curve,fr);
	}

	/* Compute node velocity */

	status = set_node_velocity(pc[0],oldp,Node_of_o_curve(newc1),
				   oldc1,oldc2,v1,v2,fr,dt,dt_frac);

	return leave_cross_or_extend_to_cross_two_propagated_curves(status,
	         newc1,newc2,oppb1,oppb2,oppn1,oppn2);
}		/*end found_c_or_e_2_pc*/

LOCAL	int leave_cross_or_extend_to_cross_two_propagated_curves(
	int     status,
	O_CURVE *newc1,
	O_CURVE *newc2,
	BOND    *oppb1,
	BOND    *oppb2,
	NODE    *oppn1,
	NODE    *oppn2)
{
	int dim = newc1->curve->interface->dim;
	if (newc2->orient == POSITIVE_ORIENTATION) 
	    oppb2->end = oppn2->posn;
	else 
	    oppb2->start = oppn2->posn;
	set_bond_length(oppb2,dim);
	if (newc1->orient == POSITIVE_ORIENTATION) 
	    oppb1->end = oppn1->posn;
	else 
	    oppb1->start = oppn1->posn;
	set_bond_length(oppb1,dim);
	if (debugging("cross_or_extend"))
	{
	    print_general_vector("Node_vel(Node_of_o_curve(newc2)) = ",
	    		         Node_vel(Node_of_o_curve(newc2)),dim,"\n");
	    (void) printf("\t\t\t\t\t");
	    print_node_status("status = ",status,"\n");
	}
	debug_print("cross_or_extend",
	      "Left cross_or_extend_to_cross_two_propagated_curves()\n");
	return status;
}		/*end cross_or_extend_to_cross_two_propagated_curves*/



LOCAL int find_circle_through_points(
	FT_POINT		*p0,
	FT_POINT		*p1,
	FT_POINT		*p2,
	FT_POINT		*pcenter,
	double		*rsqr,
	double		*h,
	int		dim)
{
	INTERFACE    *intfc = current_interface();
	double	    xm0, ym0, xm1, ym1, nx0, ny0, nx1, ny1;
	double	    xmd, ymd;
	double	    den;
	double	    t0, t1;
	double	    dx0, dy0, dx1, dy1, dx2, dy2;
	static const double MAX_SQR_RADIUS = 10000.0; /*TOLERANCE*/

	if (scaled_separation(p0,p1,h,dim) < MIN_SC_SEP(intfc) ||
    	    scaled_separation(p0,p2,h,dim) < MIN_SC_SEP(intfc) ||
    	    scaled_separation(p1,p2,h,dim) < MIN_SC_SEP(intfc))
	    return NO;

	xm0 = (double) 0.5*(Coords(p0)[0] + Coords(p1)[0]);
	ym0 = (double) 0.5*(Coords(p0)[1] + Coords(p1)[1]);
	xm1 = (double) 0.5*(Coords(p1)[0] + Coords(p2)[0]);
	ym1 = (double) 0.5*(Coords(p1)[1] + Coords(p2)[1]);
	nx0 = (double)Coords(p0)[1]-(double)Coords(p1)[1];
	ny0 = (double)Coords(p1)[0]-(double)Coords(p0)[0];
	xmd = xm1 - xm0;	ymd = ym1 - ym0;
	den = hypot(nx0,ny0);	nx0 /= den;	ny0 /= den;
	nx1 = (double)Coords(p1)[1]-(double)Coords(p2)[1];
	ny1 = (double)Coords(p2)[0]-(double)Coords(p1)[0];
	den = hypot(nx1,ny1);	nx1 /= den;	ny1 /= den;
	den = nx0*ny1 - ny0*nx1;
	if (fabs(den) < EPSILON)
	    return NO;
	t0 = (xmd*ny1 - ymd*nx1)/den;	t1 = (xmd*ny0 - ymd*nx0)/den;
	Coords(pcenter)[0] = (double) (0.5*(xm0 + xm1 + t0*nx0 + t1*nx1));
	Coords(pcenter)[1] = (double) (0.5*(ym0 + ym1 + t0*ny0 + t1*ny1));
	dx0 = Coords(p0)[0] - Coords(pcenter)[0];
	dy0 = Coords(p0)[1] - Coords(pcenter)[1];
	dx1 = Coords(p1)[0] - Coords(pcenter)[0];
	dy1 = Coords(p1)[1] - Coords(pcenter)[1];
	dx2 = Coords(p2)[0] - Coords(pcenter)[0];
	dy2 = Coords(p2)[1] - Coords(pcenter)[1];
	*rsqr = (sqr(dx0)+sqr(dy0)+sqr(dx1)+sqr(dy1)+sqr(dx2)+sqr(dy2))/3.0;
	if (*rsqr > MAX_SQR_RADIUS)
	    return NO;
	return YES;
}		/*end find_circle_through_points*/

/*
*			find_bonds_for_extension_direction():
*
*	Given a bond connected to a partially propagated curve, this routine 
*	returns a bond with nontrivial length which agrees with the given bond
*	at the start if c_orient equals POSITIVE_ORIENTATION or at the end
*	otherwise.
*
*/

EXPORT	void find_bonds_for_extension_direction(
	BOND		*newb,
	O_CURVE		*oldc,
	O_CURVE		*newc,
	BOND		*newbdir,
	BOND		*oldbdir,
	Front		*fr)
{
	BOND		*oldb;
	BOND		*follower;
	double		*h = fr->rect_grid->h;
	double		scaled_length;
	int		dim = fr->interf->dim;

	if (oldbdir != NULL) 
	{
	    oldb = Bond_at_node_of_o_curve(oldc);
	    oldbdir->next = oldb->next;
	    oldbdir->prev = oldb->prev;
	    oldbdir->start = oldb->start;
	    oldbdir->end = oldb->end;
	    bond_length(oldbdir) = bond_length(oldb);
	}
	newbdir->next = newb->next;
	newbdir->prev = newb->prev;
	newbdir->start = newb->start;
	newbdir->end = newb->end;
	bond_length(newbdir) = bond_length(newb);
	scaled_length = scaled_bond_length(newbdir,h,dim);
	while (scaled_length < 0.1 &&
	    (follower = Following_bond(newbdir,newc->orient)) != NULL && 
	    Following_bond(follower,newc->orient) != NULL) 
	{ 
	    if (newc->orient == POSITIVE_ORIENTATION) 
	    {
	    	newbdir->end = follower->end;
	    	newbdir->next = follower->next;
	    }
	    else 
	    {
	    	newbdir->start = follower->start;
	    	newbdir->prev = follower->prev;
	    }
	    set_bond_length(newbdir,dim);
	    if (oldbdir != NULL)
	    {
	    	if (oldc->orient == POSITIVE_ORIENTATION) 
	    	{
	    	    oldbdir->end = oldbdir->next->end;
	    	    oldbdir->next = oldbdir->next->next;
	    	}
	    	else
	    	{
	    	    oldbdir->start = oldbdir->prev->start;
	    	    oldbdir->prev = oldbdir->prev->prev;
	    	}
	    	set_bond_length(oldbdir,dim);
	    }
	    scaled_length += scaled_bond_length(follower,h,dim);
	}
}		/*end find_bonds_for_extension_direction*/
