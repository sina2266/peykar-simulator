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
*				fcrosscur.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Contains the routines
*
*		crossing_of_two_propagated_curves()
*		crossing_of_a_propagated_curve_and_circle()
*		intersection_of_two_o_curves()
*
*	for use by node propagate routines.
*/

#if defined(TWOD)

#include <front/fdecs.h>

	/* LOCAL Function Prototypes */
#if defined(DEBUG_CROSSING)
LOCAL	void	debug_print_improper_cross(FT_POINT*,double,double,BOND*,CURVE*,
					   BOND*,CURVE*,double*,double*,Front*);
#endif /* defined(DEBUG_CROSSING) */
LOCAL	int	found_crossing_of_a_propagated_curve_and_circle(BOND**,BOND*,
                                                                BOND*,O_CURVE*,
								FT_POINT*,double,
								Front*);
LOCAL	int	found_crossing_of_two_propagated_curves(BOND**,BOND**,BOND*,
                                                        BOND*,BOND*,BOND*,
							NODE*,NODE*,O_CURVE*,
							O_CURVE*,O_CURVE*,
							O_CURVE*,FT_POINT*,FT_POINT*,
							double,double,double*,
							double*,double,double*,
							Front*);
LOCAL	int	leave_crossing_of_a_propagated_curve_and_circle(BOND*,O_CURVE*,
                                                                NODE*,int);
LOCAL	int	leave_crossing_of_two_propagated_curves(int,O_CURVE*,O_CURVE*,
                                                        BOND*,BOND*,
							NODE*,NODE*);
LOCAL	boolean	leave_intersection_of_two_o_curves(boolean,O_CURVE*,O_CURVE*,
	                                           O_CURVE*,O_CURVE*,BOND*,
						   BOND*,BOND**,BOND**,
						   NODE*,NODE*);
LOCAL	int	no_crossing_of_two_propagated_curves(int,O_CURVE*,O_CURVE*,
                                                     O_CURVE*,O_CURVE*,
						     FT_POINT*,FT_POINT*,BOND*,BOND*,
						     BOND*,BOND*,NODE*,NODE*,
						     FT_POINT*,Front*,POINTER,
						     double,double*,RPROBLEM**);
LOCAL	void	no_cross_of_propagated_curve_and_circle(int,O_CURVE*,O_CURVE*,
	                                                BOND*,FT_POINT*,FT_POINT*,
							FT_POINT*,double,double,
							double*,Front*,POINTER,
							RPROBLEM**);
LOCAL	void	set_vel_of_circle_crossing_node(BOND*,ORIENTATION,
						BOND*,ORIENTATION,NODE*,
						double,double,int);
LOCAL	void	set_virtual_opp_node(FT_POINT*,O_CURVE*,O_CURVE*,NODE**,BOND**,
				     Front*,POINTER,double);

/*
*               crossing_of_two_propagated_curves():
*
*	Finds the intersection of two curves newc1 and newc2 that
*	have been propagated except possibly at their nodes.
*       If there is a crossing GOOD_NODE is returned and the crossing point 
*	and bonds are found; otherwise an appropriate diagnostic is returned.
*/

EXPORT int crossing_of_two_propagated_curves(
	O_CURVE		*oldc1,
	O_CURVE		*newc1,
	O_CURVE		*oldc2,
	O_CURVE		*newc2,
	FT_POINT		*pc,		/* crossing point */
	BOND		**newb1cr,	/* the crossing bond on newc1 */
	BOND		**newb2cr,	/* the crossing bond on newc2 */
	double		*s1,		/* fract dist on bond to cross */
	double		*s2,		/* fract dist on bond to cross */
	Front		*fr,
	POINTER		wave,
	RPROBLEM	**rp,
	double		dt,		/* time step */
	double		*dt_frac,	/* part of time step achieved */
	NODE_FLAG	flag)
{
	RECT_GRID	*gr = fr->rect_grid;
	BOND		*b1, *b2, *newb1, *newb2;
	BOND		*b1virtual, *b2virtual;
	BOND		B1, B2;			/* virtual bonds */
	BOND		*follower;
	NODE		*oppn1,*oppn2;
	BOND		*oppb1,*oppb2;
	double		*h = gr->h;
	double		low[MAXD], high[MAXD];
	double		v1[MAXD], v2[MAXD];
	int		wt1 = wave_type(newc1->curve);
	int		wt2 = wave_type(newc2->curve);
	int		cr_stat = NO_CROSS;
	int		status;
	int		i, dim = gr->dim;
	static	FT_POINT   *p1 = NULL, *p2 = NULL, *p1_opp = NULL, *p2_opp = NULL;
	static	FT_POINT	*oldp = NULL;
	int		on_b1,on_b2;
	int		count_b1,count_b2;

	on_b1 = (wt1 < FIRST_SCALAR_PHYSICS_WAVE_TYPE) ? YES : NO;
	on_b2 = (wt2 < FIRST_SCALAR_PHYSICS_WAVE_TYPE) ? YES : NO;

	debug_print("crossing","Entered crossing_of_two_propagated_curves()\n");

	if (p1 == NULL) 
	{
	    p1 = Static_point(fr->interf);
	    p2 = Static_point(fr->interf);
	    p1_opp = Static_point(fr->interf);
	    p2_opp = Static_point(fr->interf);
	    oldp = Static_point(fr->interf);
	}

	for (i = 0; i < dim; i++)
	{
	    low[i] = fr->rect_grid->VL[i] - MIN_SCALED_LENGTH(fr->interf)*h[i];
	    high[i] = fr->rect_grid->VU[i] + MIN_SCALED_LENGTH(fr->interf)*h[i];
	}

	/* propagate the node to two new positions using both curves */

	newb1 = b1virtual = &B1;	newb2 = b2virtual = &B2;
	init_curve_for_crossing(p1,p1_opp,b1virtual,oldc1,newc1,
		                &oppn1,&oppb1,fr,wave,dt,v1,flag);
	init_curve_for_crossing(p2,p2_opp,b2virtual,oldc2,newc2,
		                &oppn2,&oppb2,fr,wave,dt,v2,flag);
	
#if defined(DEBUG_CROSSING)
	if (debugging("crossing")) 
	{
		(void) printf("Old and new curves in ");
		(void) printf("crossing_of_two_propagated_curves():\n");
		(void) printf("OLDC1\n");	print_o_curve(oldc1);
		(void) printf("NEWC1\n");	print_o_curve(newc1);
		(void) printf("OLDC2\n");	print_o_curve(oldc2);
		(void) printf("NEWC2\n");	print_o_curve(newc2);
		(void) printf("Virtual bonds:\n");
		(void) printf("b1virtual\n");	print_bond(b1virtual);	
		(void) printf("b2virtual\n");	print_bond(b2virtual);

	}
#endif /* defined(DEBUG_CROSSING) */

	if (node_velocity_preset(flag) == YES)
	{
	    for (i = 0; i < dim; i++)
	        Coords(pc)[i] = Coords(Node_of_o_curve(oldc2)->posn)[i] +
				dt * Node_vel(Node_of_o_curve(newc2))[i];
	    *newb1cr = Bond_at_opp_node_of_o_curve(newc1);
	    *newb2cr = Bond_at_opp_node_of_o_curve(newc2);
	    *s1 = (newc1->orient == POSITIVE_ORIENTATION) ? 1.0 : 0.0;
	    *s2 = (newc2->orient == POSITIVE_ORIENTATION) ? 1.0 : 0.0;
	    status = GOOD_NODE;
	    return leave_crossing_of_two_propagated_curves(status,newc1,newc2,
	                                                   oppb1,oppb2,oppn1,
						           oppn2);
	}

	/* find the crossing of two partially propagated curves */

	count_b1 = count_b2 = 0;
	for ( ; ; ) 
	{
	    b2 = b2virtual;
	    for ( ; ; ) 
	    {
	        if (robust_cross_bonds(newb1,on_b1,b2,on_b2,s1,s2,gr,pc)) 
	        {
	    	    *newb1cr = newb1;	*newb2cr = b2;

	    	    cr_stat = check_cross(*s1,newb1,newc1,*s2,b2,newc2,pc,
					  low,high,dim);
		    if (cr_stat == GOOD_CROSS)
		    {
	                return found_crossing_of_two_propagated_curves(newb1cr,
			        newb2cr,b1virtual,b2virtual,oppb1,oppb2,
				oppn1,oppn2,oldc1,oldc2,newc1,newc2,
				oldp,pc,*s1,*s2,v1,v2,dt,dt_frac,fr);
		    }
	    	    else
	    	    {
#if defined(DEBUG_CROSSING)
	    		debug_print_improper_cross(pc,*s1,*s2,*newb1cr,
	    			                   newc1->curve,*newb2cr,
						   newc2->curve,low,high,fr);
#endif /* defined(DEBUG_CROSSING) */
	                return no_crossing_of_two_propagated_curves(cr_stat,
			         oldc1,oldc2,newc1,newc2,p1_opp,p2_opp,
				 b1virtual,b2virtual,oppb1,oppb2,oppn1,oppn2,
			         pc,fr,wave,dt,dt_frac,rp);
	    	    }
	        }
	        if (b2 == newb2)
		    break;
	        b2 = Following_bond(b2,newc2->orient);
	    }
	    if ((follower = Following_bond(newb2,newc2->orient)) != NULL)
	    {
	    	newb2 = follower;
		if (count_b2 > 10) break;
		count_b2++;
	    }
	    else if (Following_bond(newb1,newc1->orient) == NULL) 
	    {
#if defined(DEBUG_CROSSING)
	    	if (debugging("crossing"))
	    	    (void) printf("Curves don't cross\n");
#endif /* defined(DEBUG_CROSSING) */
	        break;
	    }

	    b1 = b1virtual;
	    for ( ; ; ) 
	    {
	        if (robust_cross_bonds(b1,on_b1,newb2,on_b2,s1,s2,gr,pc)) 
	        {
	    	    *newb1cr = b1;			*newb2cr = newb2;
	    	    cr_stat = check_cross(*s1,b1,newc1,*s2,newb2,newc2,pc,
					  low,high,dim);
		    if (cr_stat == GOOD_CROSS)
		    {
	                return found_crossing_of_two_propagated_curves(newb1cr,
			        newb2cr,b1virtual,b2virtual,oppb1,oppb2,
				oppn1,oppn2,oldc1,oldc2,newc1,newc2,
				oldp,pc,*s1,*s2,v1,v2,dt,dt_frac,fr);
		    }
	    	    else
	    	    {
#if defined(DEBUG_CROSSING)
	    		debug_print_improper_cross(pc,*s1,*s2,
	    				*newb1cr,newc1->curve,*newb2cr,
	    				newc2->curve,low,high,fr);
#endif /* defined(DEBUG_CROSSING) */
	                return no_crossing_of_two_propagated_curves(cr_stat,
			         oldc1,oldc2,newc1,newc2,p1_opp,p2_opp,
				 b1virtual,b2virtual,oppb1,oppb2,oppn1,oppn2,
			         pc,fr,wave,dt,dt_frac,rp);
	    	    }
	        }
	        if (b1 == newb1)
		    break;
	        b1 = Following_bond(b1,newc1->orient);
	    }
	    if ((follower = Following_bond(newb1,newc1->orient)) != NULL)
	    {
	    	newb1 = follower;
		if (count_b1 > 10) break;
		count_b1++;
	    }
	    else if (Following_bond(newb2,newc2->orient) == NULL) 
	    {
#if defined(DEBUG_CROSSING)
		if (debugging("crossing"))
		    (void) printf("Curves don't cross\n");
#endif /* defined(DEBUG_CROSSING) */
		break;
	    }
	}

	return no_crossing_of_two_propagated_curves(cr_stat,oldc1,oldc2,
	                                            newc1,newc2,p1_opp,p2_opp,
						    b1virtual,b2virtual,
						    oppb1,oppb2,oppn1,oppn2,
						    pc,fr,wave,dt,dt_frac,rp);
}		/*end crossing_of_two_propagated_curves*/

LOCAL	int no_crossing_of_two_propagated_curves(
	int	 cr_stat,
	O_CURVE	 *oldc1,
	O_CURVE	 *oldc2,
	O_CURVE	 *newc1,
	O_CURVE	 *newc2,
	FT_POINT	 *p1_opp,
	FT_POINT	 *p2_opp,
	BOND	 *b1virtual,
	BOND	 *b2virtual,
	BOND     *oppb1,
	BOND     *oppb2,
	NODE     *oppn1,
	NODE     *oppn2,
	FT_POINT	 *pc,
	Front	 *fr,
	POINTER	 wave,
	double	 dt,
	double	 *dt_frac,
	RPROBLEM **rp)
{
	NODE  *interact_node1,*interact_node2;
	NODE  *interact_nodes[9];
	double dt_frac_tmp = 1.0;
	int   wt1 = wave_type(newc1->curve);
	int   wt2 = wave_type(newc2->curve);
	int   k;
	int   status;

	status = find_cross_status(cr_stat,oldc1,newc1,oldc2,newc2,
				   p1_opp,p2_opp,b1virtual,b2virtual,pc,
				   &interact_node1,&interact_node2,
				   fr,wave,dt,&dt_frac_tmp);

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
	case CROSS_PAST_CURVE_NODE:
	    if (status == CROSS_NODE_NODE)
	    {
	        set_vel_of_crossing_node(Bond_at_node_of_o_curve(oldc1),
			                 Bond_at_node_of_o_curve(oldc2),
					 b1virtual,b2virtual,wt1,wt2,
					 Node_of_o_curve(oldc2),
			                 Node_of_o_curve(newc2),dt,fr);
	    }
	    *dt_frac = min(*dt_frac,dt_frac_tmp);
	    k = 0;
	    interact_nodes[k++] = Node_of_o_curve(newc1);
	    interact_nodes[k++] = Node_of_o_curve(oldc1);
	    if (Node_of_o_curve(newc2) != Node_of_o_curve(newc1))
	    {
	        interact_nodes[k++] = Node_of_o_curve(newc2);
	        interact_nodes[k++] = Node_of_o_curve(oldc2);
	    }
	    interact_nodes[k++] = interact_node1 ? interact_node1 :
	    	                               interact_node2;
	    interact_nodes[k++] = interact_node1 ?
	    	Opp_node_of_o_curve(oldc1) : interact_node2 ?
	    	Opp_node_of_o_curve(oldc2) : NULL;
	    interact_nodes[k++] = interact_node1 && interact_node2 ?
	    	interact_node2 : NULL;
	    interact_nodes[k++] = interact_node1 && interact_node2 ?
	    	Opp_node_of_o_curve(oldc2) : NULL;
	    interact_nodes[k++] = NULL;
	    augment_rproblem_list(rp,interact_nodes,dt,dt_frac_tmp,
	    		      oldc1->curve->interface,
	    		      newc1->curve->interface,fr,wave);
	    break;
	}
	return leave_crossing_of_two_propagated_curves(status,newc1,newc2,
	                                               oppb1,oppb2,oppn1,
						       oppn2);
}		/*end no_crossing_of_two_propagated_curves*/

/*ARGSUSED*/
LOCAL	int found_crossing_of_two_propagated_curves(
	BOND    **newb1cr,
	BOND    **newb2cr,
	BOND    *b1virtual,
	BOND    *b2virtual,
	BOND    *oppb1,
	BOND    *oppb2,
	NODE    *oppn1,
	NODE    *oppn2,
	O_CURVE *oldc1,
	O_CURVE *oldc2,
	O_CURVE *newc1,
	O_CURVE *newc2,
	FT_POINT   *oldp,
	FT_POINT   *pc,
	double   s1,
	double   s2,
	double   *v1,
	double   *v2,
	double   dt,
	double   *dt_frac,
	Front   *fr)
{
	FT_POINT *oldp1, *oldp2;
	int   i, dim = fr->rect_grid->dim;
	int   status;

	if (*newb1cr == b1virtual)
	    *newb1cr = Bond_at_node_of_o_curve(newc1);
	if (*newb2cr == b2virtual)
	    *newb2cr = Bond_at_node_of_o_curve(newc2);

#if defined(DEBUG_CROSSING)
	if (debugging("crossing")) 
	{
	    (void) printf("pc (%llu): (%g, %g)  s1 = %g s2 = %g\n",
			  point_number(pc),Coords(pc)[0],Coords(pc)[1],s1,s2);
	    (void) printf("*newb1cr:\n");
	    print_bond_and_states(*newb1cr,newc1->curve,fr);
	    (void) printf("*newb2cr:\n");
	    print_bond_and_states(*newb2cr,newc2->curve,fr);
	}
#endif /* defined(DEBUG_CROSSING) */

	/* Compute node velocity */

	oldp1 = Node_of_o_curve(oldc1)->posn;
	oldp2 = Node_of_o_curve(oldc2)->posn;
	for (i = 0; i < dim; i++)
	    Coords(oldp)[i] = 0.5*(Coords(oldp1)[i]+Coords(oldp2)[i]);
	status = set_node_velocity(pc,oldp,Node_of_o_curve(newc2),oldc1,oldc2,
			         v1,v2,fr,dt,dt_frac);
	return leave_crossing_of_two_propagated_curves(status,newc1,newc2,
	                                               oppb1,oppb2,oppn1,oppn2);
}		/*end found_crossing_of_two_propagated_curves*/

LOCAL	int leave_crossing_of_two_propagated_curves(
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

	debug_print("crossing","Left crossing_of_two_propagated_curves()\n");
	if (debugging("crossing"))
	    print_node_status("status = ",status,"\n");
	return status;
}		/*end leave_crossing_of_two_propagated_curves*/






/*
*               crossing_of_a_propagated_curve_and_circle():
*
*       This routine searches for a point on a propagated curve with
*       a given distance from the unpropagated node.  Actually we are
*	looking for the position of the propagated node when its speed
*	has already been calculated.
*       If there is a crossing GOOD_NODE is returned and the crossing point 
*	and bond is found; otherwise an appropriate diagnostic is returned.
*/


EXPORT int crossing_of_a_propagated_curve_and_circle(
	O_CURVE		*oldc,
	O_CURVE		*newc,
	double		radius,	    /* radius of the circle centered at oldn */
	FT_POINT	*pcenter,   /* Center of the circle */
	FT_POINT	*pc,	    /* crossing point */
	BOND		**newbcr,   /* the crossing bond on newc */
	double		*s,	    /* fract dist on bond to cross */
	Front		*fr,
	POINTER		wave,
	RPROBLEM	**rp,
	double		dt,	    /* time step */
	double		*dt_frac,
	NODE_FLAG	flag)
{
	BOND		*newb;
	BOND		*bvirtual;
	BOND		B;	    /* virtual bond */
	NODE		*oppn;
	BOND		*oppb;
	double		*h = fr->rect_grid->h;
	double		low[MAXD], high[MAXD];
	double		V[MAXD];
	double		eps;
	double		Rsq = sqr(radius);
	int		cr_stat = NO_CROSS;
	int		status = ERROR_NODE;
	int		i, dim = fr->rect_grid->dim;
	static	FT_POINT	*p = NULL, *p_opp = NULL;

	debug_print("circle_crossing",
		"Entered crossing_of_a_propagated_curve_and_circle()\n");
	if (p == NULL) 
	{
	    p = Static_point(fr->interf);
	    p_opp = Static_point(fr->interf);
	}
	eps = .005 * min(fr->rect_grid->h[0],fr->rect_grid->h[1]);
	for (i = 0; i < dim; i++)
	{
	    low[i] = fr->rect_grid->VL[i] - MIN_SCALED_LENGTH(fr->interf)*h[i];
	    high[i] = fr->rect_grid->VU[i] + MIN_SCALED_LENGTH(fr->interf)*h[i];
	}
	/* propagate node to temporary new position using oldc */

	newb = bvirtual = &B;
	init_curve_for_crossing(p,p_opp,bvirtual,oldc,newc,
		                &oppn,&oppb,fr,wave,dt,V,flag);
#if defined(DEBUG_CROSSING)
	if (debugging("circle_crossing")) 
	{
	    (void) printf("pcenter = (%g, %g), radius = %g\n",
			  Coords(pcenter)[0],Coords(pcenter)[1],radius);
	    (void) printf("bvirtual, ");	print_bond(bvirtual);
	    (void) printf("newc, ");	print_o_curve(newc);
	}
#endif /* defined(DEBUG_CROSSING) */

	if (node_velocity_preset(flag) == YES)
	{
	    for (i = 0; i < dim; i++)
	       Coords(pc)[i] = Coords(Node_of_o_curve(oldc)->posn)[i] +
			       dt * Node_vel(Node_of_o_curve(newc))[i];
	    *newbcr = Bond_at_opp_node_of_o_curve(newc);
	    *s = (newc->orient == POSITIVE_ORIENTATION) ? 1.0 : 0.0;
	    status = GOOD_NODE;
	    return leave_crossing_of_a_propagated_curve_and_circle(oppb,newc,
	                                                           oppn,status);
	}

		/* Find crossing of curve and circle */

	if (radius > 0.0)
	{
	    for ( ; newb != NULL; newb = Following_bond(newb,newc->orient))
	    {
	    	if (robust_cross_bond_circle(newb,pcenter,Rsq,s,pc)) 
	    	{
	    	    cr_stat = check_cross(*s,newb,newc,-1.0,NULL,NULL,
					  pc,low,high,dim);
	    	    if (cr_stat == GOOD_CROSS)
		    {
	                status =
			 found_crossing_of_a_propagated_curve_and_circle(
			     newbcr,newb,bvirtual,newc,pc,*s,fr);
	                return leave_crossing_of_a_propagated_curve_and_circle(
			           oppb,newc,oppn,status);
		    }
	    	}
	    }
	    /*
	     * If cr_stat != NO_CROSS found only invalid cross(es).
	     * If cr_stat == NO_CROSS, found no crosses, and want
	     * to check for zero radius below.
	     */
	    if (cr_stat != NO_CROSS)
	    {
	        no_cross_of_propagated_curve_and_circle(status,oldc,newc,
		                                        bvirtual,p,p_opp,pc,
							radius,dt,dt_frac,
							fr,wave,rp);

	        return leave_crossing_of_a_propagated_curve_and_circle(oppb,
		                                                       newc,
								       oppn,
								       status);
	    }
	}

		/* If radius = 0 the crossing point is oldn->posn */

	if (radius <= eps) 
	{
	    for (i = 0; i < dim; i++)
	    	Coords(pc)[i] = Coords(Node_of_o_curve(oldc)->posn)[i];
	    *s = (newc->orient == POSITIVE_ORIENTATION) ? 0.0 : 1.0;
	    newb = bvirtual;
#if defined(DEBUG_CROSSING)
	    if (debugging("circle_crossing"))
	    	(void) printf("radius = %g eps = %g\n",radius,eps);
#endif /* defined(DEBUG_CROSSING) */
	    status = found_crossing_of_a_propagated_curve_and_circle(
			     newbcr,newb,bvirtual,newc,pc,*s,fr);
	    return leave_crossing_of_a_propagated_curve_and_circle(
			           oppb,newc,oppn,status);
	}

	no_cross_of_propagated_curve_and_circle(status,oldc,newc,bvirtual,
	                                        p,p_opp,pc,radius,dt,dt_frac,
						fr,wave,rp);

	return leave_crossing_of_a_propagated_curve_and_circle(oppb,newc,oppn,
	                                                       status);
}		/*end crossing_of_a_propagated_curve_and_circle*/

/*ARGSUSED*/
LOCAL	void	no_cross_of_propagated_curve_and_circle(
	int      status,
	O_CURVE  *oldc,
	O_CURVE  *newc,
	BOND     *bvirtual,
	FT_POINT  *p,
	FT_POINT  *p_opp,
	FT_POINT  *pc,
	double    radius,
	double    dt,
	double    *dt_frac,
	Front    *fr,
	POINTER  wave,
	RPROBLEM **rp)
{
	NODE  *interact_nodes[5];
	double dt_frac_tmp = 1.0;
	int   dim = fr->rect_grid->dim;

	status = find_circle_cross_status(oldc,newc,p_opp,
			radius,pc,fr,&dt_frac_tmp);
	
	switch (status)
	{
	case PSEUDOCROSS_NODE_NODE:
	    set_vel_of_circle_crossing_node(Bond_at_node_of_o_curve(oldc),
			                    oldc->orient,bvirtual,newc->orient,
			                    Node_of_o_curve(newc),
					    radius,dt,dim);
	    break;

	case CROSS_NODE_NODE:
	    set_vel_of_circle_crossing_node(Bond_at_node_of_o_curve(oldc),
			                    oldc->orient,bvirtual,newc->orient,
			                    Node_of_o_curve(newc),radius,
					    dt,dim);
	    interact_nodes[0] = Node_of_o_curve(newc);
	    interact_nodes[1] = Node_of_o_curve(oldc);
	    interact_nodes[2] = Opp_node_of_o_curve(newc);
	    interact_nodes[3] = Opp_node_of_o_curve(oldc);
	    interact_nodes[4] = NULL;
	    augment_rproblem_list(rp,interact_nodes,dt,dt_frac_tmp,
			          oldc->curve->interface,
				  newc->curve->interface,fr,wave);
	    break;

	case CROSS_PAST_CURVE_NODE:
	    *dt_frac = min(*dt_frac,dt_frac_tmp);
	    break;
	}

#if defined(DEBUG_CROSSING)
	if (debugging("circle_crossing")) 
	{
	    (void) printf("p = %g %g\n",Coords(p)[0],Coords(p)[1]);
	    print_o_curve(newc);
	}
#endif /* defined(DEBUG_CROSSING) */
}		/*end no_cross_of_propagated_curve_and_circle*/

LOCAL void set_vel_of_circle_crossing_node(
	BOND		*b,
	ORIENTATION	b_orient,
	BOND		*newb,
	ORIENTATION	newb_orient,
	NODE		*center,
	double		radius,
	double		dt,
	int		dim)
{
	FT_POINT P;
	FT_POINT *oldp,*newp;
	double t;
	const double eps = MACH_EPS;/*TOLERANCE*/
	int   i;

	propagation_status(center) = VEL_COMPUTED_NODE;
	for (i = 0; i < dim; i++)
	    Node_vel(center)[i] = 0.0;

	oldp = (b_orient == POSITIVE_ORIENTATION) ? b->end : b->start;
	newp = (newb_orient == POSITIVE_ORIENTATION) ? newb->end : newb->start;
	Check_return(
	    robust_circle_cross_trace(oldp,newp,center->posn,NULL,radius,&t,&P),
	    set_vel_of_circle_crossing_node)
	t *= dt;
	if (t < eps)/*TOLERANCE*/
	    return;
	for (i = 0; i < dim; i++)
	    Node_vel(center)[i] = (Coords(&P)[i] - Coords(center->posn)[i])/t;
}		/*end set_vel_of_circle_crossing_node*/

/*ARGSUSED*/
LOCAL	int	found_crossing_of_a_propagated_curve_and_circle(
	BOND    **newbcr,
	BOND    *newb,
	BOND    *bvirtual,
	O_CURVE *newc,
	FT_POINT   *pc,
	double   s,
	Front   *fr)
{
	*newbcr = (newb == bvirtual) ? 	Bond_at_node_of_o_curve(newc) : newb;
#if defined(DEBUG_CROSSING)
	if (debugging("circle_crossing")) 
	{
	    (void) printf("pc (%llu): %g %g  s = %g\n",point_number(pc),
		          Coords(pc)[0],Coords(pc)[1],s);
	    (void) printf("*newbcr:\n");
	    print_bond_and_states(*newbcr,newc->curve,fr);
	}
#endif /* defined(DEBUG_CROSSING) */
	return GOOD_NODE;
}		/*end found_crossing_of_a_propagated_curve_and_circle*/

LOCAL	int	leave_crossing_of_a_propagated_curve_and_circle(
	BOND    *oppb,
	O_CURVE *newc,
	NODE    *oppn,
	int     status)
{
	int dim = newc->curve->interface->dim;
	if (newc->orient == POSITIVE_ORIENTATION) 
	    oppb->end = oppn->posn;
	else 
	    oppb->start = oppn->posn;
	bond_length(oppb) = separation(oppb->start,oppb->end,dim);

	debug_print("circle_crossing",
	      "Left crossing_of_a_propagated_curve_and_circle(), status = %s\n",
	      node_status_as_string(status));
	return status;
}		/*end leave_crossing_of_a_propagated_curve_and_circle*/

/*
*		intersection_of_two_o_curves():
*
*	Given two propagated o curves oc1 and oc2, this function finds
*	the first crossing on oc1->curve with respect to oc1->orient
*	with oc2->curve.  If no crossing is found, then oc1->curve
*	will be extended at the node Node_of_o_curve(oc1)
*	and the crossing of this extension with oc2 will checked.  
*	If no cross is then found the process is repeated with c2 extended.
*	Finally if no cross found both curves are extended to find a cross.
*	If no crossing by extension is found, or if the cross by extension
*	is more than a few mesh blocks from the original nodes
*	the function returns NO. The function returns YES if a crossing is 
*	found.  In addition the states at the nodes of the two curves are 
*	ft_assigned to be the states obtained by propagating the nodes of the 
*	old curves as points.
*
*/

EXPORT	boolean intersection_of_two_o_curves(
	O_CURVE		*newc1,
	O_CURVE		*oldc1,
	O_CURVE		*newc2,
	O_CURVE		*oldc2,
	BOND		**crossb1,
	BOND		**crossb2,
	FT_POINT		**pcr,
	double		*s1,
	double		*s2,
	Front		*fr,
	POINTER		wave,
	double		dt,
	NODE_FLAG	flag)
{
	RECT_GRID	*gr = fr->rect_grid;
	NODE		*oppn1 = NULL, *oppn2 = NULL;
	BOND		*oppb1 = NULL, *oppb2 = NULL;
	BOND		*b1, *b2;
	boolean		found_cross = NO;
	double		*h = gr->h;
	double		V[MAXD];
	int		wt1 = wave_type(newc1->curve);
	int		wt2 = wave_type(newc2->curve);
	int		i, dim = gr->dim;
	static	BOND	*bdir1 = NULL, *bdir2 = NULL;
	static	FT_POINT	*pc = NULL, *p1 = NULL, *p2 = NULL,
	                *pmid = NULL, *p1_opp = NULL, *p2_opp = NULL;
	int		on_b1,on_b2;

	on_b1 = (wt1 < FIRST_SCALAR_PHYSICS_WAVE_TYPE) ? YES : NO;
	on_b2 = (wt2 < FIRST_SCALAR_PHYSICS_WAVE_TYPE) ? YES : NO;

	debug_print("intersection","Entered intersection_of_two_o_curves()\n");
	if (pc == NULL) 
	{
	    pc = Static_point(fr->interf);
	    p1 = Static_point(fr->interf);
	    p2 = Static_point(fr->interf);
	    pmid = Static_point(fr->interf);
	    p1_opp = Static_point(fr->interf);
	    p2_opp = Static_point(fr->interf);
	    scalar(&bdir1,sizeof(BOND));
	    scalar(&bdir2,sizeof(BOND));
	    bdir1->start = Static_point(fr->interf);
	    bdir1->end = Static_point(fr->interf);
	    bdir2->start = Static_point(fr->interf);
	    bdir2->end = Static_point(fr->interf);
	}

	if (newc1->curve == newc2->curve && newc1->orient == newc2->orient)
	    return NO;

	if (node_velocity_preset(flag) == YES)
	{
	    if (oldc1 && oldc1->curve)
	    {
	        for (i = 0; i < dim; i++)
	            Coords(pc)[i] = Coords(Node_of_o_curve(oldc1)->posn)[i] +
			            dt * Node_vel(Node_of_o_curve(newc1))[i];
	    }
	    else if (oldc2 && oldc2->curve)
	    {
	        for (i = 0; i < dim; i++)
	            Coords(pc)[i] = Coords(Node_of_o_curve(oldc2)->posn)[i] +
				    dt * Node_vel(Node_of_o_curve(newc2))[i];
	    }
	    else
	    {
	        for (i = 0; i < dim; i++)
	            Coords(pc)[i] = Coords(Node_of_o_curve(newc1)->posn)[i] +
				    dt * Node_vel(Node_of_o_curve(newc1))[i];
	    }
	    *pcr = Point(Coords(pc));
	    *crossb1 = Bond_at_opp_node_of_o_curve(newc1);
	    *crossb2 = Bond_at_opp_node_of_o_curve(newc2);
	    *s1 = (newc1->orient == POSITIVE_ORIENTATION) ? 1.0 : 0.0;
	    *s2 = (newc2->orient == POSITIVE_ORIENTATION) ? 1.0 : 0.0;
	    return leave_intersection_of_two_o_curves(YES,oldc1,oldc2,
	                                              newc1,newc2,oppb1,oppb2,
						      crossb1,crossb2,
						      oppn1,oppn2);
	}
	bdir1->next = bdir1->prev = NULL;
	bdir2->next = bdir2->prev = NULL;
	*pcr = NULL;

	if (oldc1 && oldc1->curve) 
	{
	    if (phys_virtuals_preset(flag) != YES)
	    {
	    	point_propagate(fr,wave,Node_of_o_curve(oldc1)->posn,
				p1,Bond_at_node_of_o_curve(oldc1),oldc1->curve,
				dt,V);
		if (newc1->orient != oldc1->orient)
		    reverse_states_at_point(p1,fr);

		ft_assign(Left_state_at_node_of_o_curve(newc1),
		       left_state(p1),fr->sizest);
		ft_assign(Right_state_at_node_of_o_curve(newc1),
		       right_state(p1),fr->sizest);

		if (newc1->orient == POSITIVE_ORIENTATION) 
		    Bond_at_node_of_o_curve(newc1)->start = p1;
		else 
		    Bond_at_node_of_o_curve(newc1)->end = p1;
	    }

	    if (oldc1->curve == correspond_curve(newc1->curve))
	    {
	    	set_virtual_opp_node(p1_opp,oldc1,newc1,&oppn1,
				     &oppb1,fr,wave,dt);
	    }
	}
	if (oldc2 && oldc2->curve) 
	{
	    if (phys_virtuals_preset(flag) != YES)
	    {
	    	point_propagate(fr,wave,Node_of_o_curve(oldc2)->posn,
				p2,Bond_at_node_of_o_curve(oldc2),oldc2->curve,
				dt,V);
	    	if (newc2->orient != oldc2->orient)
	    	    reverse_states_at_point(p2,fr);

	    	ft_assign(Left_state_at_node_of_o_curve(newc2),
	    	       left_state(p2),fr->sizest);
	    	ft_assign(Right_state_at_node_of_o_curve(newc2),
	    	       right_state(p2),fr->sizest);

	    	if (newc2->orient == POSITIVE_ORIENTATION) 
	    	    Bond_at_node_of_o_curve(newc2)->start = p2;
	    	else 
	    	    Bond_at_node_of_o_curve(newc2)->end = p2;
	    }

	    if (oldc2->curve == correspond_curve(newc2->curve))
	    {
	    	set_virtual_opp_node(p2_opp,oldc2,newc2,&oppn2,
				&oppb2,fr,wave,dt);
	    }
	}
#if defined(DEBUG_CROSSING)
	if (debugging("intersection")) 
	{
	    if (oldc1 && oldc1->curve && debugging("states"))
	    {
	    	(void) printf("States on oldc1\n");
	    	show_curve_states(oldc1->curve);
	    }
	    if (oldc2 && oldc2->curve && debugging("states"))
	    {
	    	(void) printf("States on oldc2\n");
	    	show_curve_states(oldc2->curve);
	    }
	    (void) printf("newc1\n");
	    print_o_curve(newc1);
	    if (oldc1) 
	    {
	    	(void) printf("oldc1\n");
	    	print_o_curve(oldc1);
	    }
	    else
	    	(void) printf("oldc1 = NULL\n");
	    (void) printf("newc2\n");
	    print_o_curve(newc2);
	    if (oldc2) 
	    {
	    	(void) printf("oldc2\n");
	    	print_o_curve(oldc2);
	    }
	    else
	    	(void) printf("oldc2 = NULL\n");
	}
#endif /* defined(DEBUG_CROSSING) */
	for (b1 = Bond_at_node_of_o_curve(newc1); b1;
		b1 = Following_bond(b1,newc1->orient)) 
	{
	    for (b2 = Bond_at_node_of_o_curve(newc2); b2;
	    	b2 = Following_bond(b2,newc2->orient)) 
	    {
	    	if (b1 == b2) continue;
	    	if (b1->next == b2) continue;
	    	if (b1->prev == b2) continue;
	    	if (b1->start == b2->start) continue;
	    	if (b1->start == b2->end) continue;
	    	if (b1->end == b2->start) continue;
	    	if (b1->end == b2->end) continue;

	    	if (!robust_cross_bonds(b1,on_b1,b2,on_b2,s1,s2,gr,pc))
	    	    continue;

				/* Store Data */
			
	    	*pcr = Point(Coords(pc));
#if defined(DEBUG_CROSSING)
	    	if (debugging("intersection"))
	    	{
	    	    (void) printf("True cross found\n");
	    	    (void) printf("Cross point 1 = (%g, %g)\n",
	    			  Coords(pcr[0])[0],Coords(pcr[0])[1]);
	    	    (void) printf("Crossb1\n");	print_bond(b1);
	    	    (void) printf("Crossb2\n");	print_bond(b2);
	    	}
#endif /* defined(DEBUG_CROSSING) */
	    	*crossb1 = b1;
	    	*crossb2 = b2;
	        return leave_intersection_of_two_o_curves(YES,oldc1,oldc2,
	                                                  newc1,newc2,
							  oppb1,oppb2,
						          crossb1,crossb2,
						          oppn1,oppn2);

	    }
	}

	if (single_extend_to_cross(flag) == YES)
	{
	    BOND *endbond;

	    bond_tangent_to_curve(Node_of_o_curve(newc1)->posn,
			          Bond_at_node_of_o_curve(newc1),newc1->curve,
			          newc1->orient,bdir1,fr);
#if defined(DEBUG_CROSSING)
	    if (debugging("intersection"))
	    {
	    	(void) printf("bdir1, ");	print_bond(bdir1);
	    	print_orientation("orient = ",newc1->orient,"\n");
	    }
#endif /* defined(DEBUG_CROSSING) */
		
	    endbond = Following_bond(bdir1,Opposite_orient(newc1->orient));
	
	    for (b2 = Bond_at_node_of_o_curve(newc2); b2;
	    	 b2 = Following_bond(b2,newc2->orient)) 
	    {
	    	if (b2 == endbond) break;
	    	if (!robust_extend_bond_to_cross_bond(bdir1,newc1->orient,
							 b2,s1,s2,pc,h,dim))
	    	{
	    	    continue;
	    	}

	    	/* Store Data */
		
	    	*pcr = Point(Coords(pc));
#if defined(DEBUG_CROSSING)
	    	if (debugging("intersection"))
	    	{
	    	    (void) printf("Cross found by extending c1\n");
	    	    (void) printf("Cross point 1 = (%g, %g)\n",
	    			  Coords(pcr[0])[0],Coords(pcr[0])[1]);
	    	    (void) printf("bdir1, ");	print_bond(bdir1);
	    	    (void) printf("crossb2, ");	print_bond(b2);
	    	}
#endif /* defined(DEBUG_CROSSING) */
	    	*crossb1 = Bond_at_node_of_o_curve(newc1);
	    	*crossb2 = b2;
	        return leave_intersection_of_two_o_curves(YES,oldc1,oldc2,
	                                                  newc1,newc2,
							  oppb1,oppb2,
							  crossb1,crossb2,
							  oppn1,oppn2);
	    }

	    bond_tangent_to_curve(Node_of_o_curve(newc2)->posn,
			          Bond_at_node_of_o_curve(newc2),newc2->curve,
			          newc2->orient,bdir2,fr);

	    endbond = Following_bond(bdir2,Opposite_orient(newc2->orient));

#if defined(DEBUG_CROSSING)
	    if (debugging("intersection"))
	    {
	    	(void) printf("bdir2, ");	print_bond(bdir2);
	    	print_orientation("orient = ",newc2->orient,"\n");
	    }
#endif /* defined(DEBUG_CROSSING) */

	    for (b1 = Bond_at_node_of_o_curve(newc1); b1;
	    	b1 = Following_bond(b1,newc1->orient)) 
	    {
	    	if (b1 == endbond) break;
		
		if (!robust_extend_bond_to_cross_bond(bdir2,newc2->orient,
							 b1,s2,s1,pc,h,dim))
		{
		    continue;
		}

		/* Store Data */
		
		*pcr = Point(Coords(pc));
#if defined(DEBUG_CROSSING)
		if (debugging("intersection"))
		{
		    (void) printf("Cross found by extending c2\n");
		    (void) printf("Cross point 1 = (%g, %g)\n",
				  Coords(pcr[0])[0],Coords(pcr[0])[1]);
		    (void) printf("crossb1\n");	print_bond(b1);
		    (void) printf("bdir2:\n");	print_bond(bdir2);
		}
#endif /* defined(DEBUG_CROSSING) */
		*crossb1 = b1;
		*crossb2 = Bond_at_node_of_o_curve(newc2);
	        return leave_intersection_of_two_o_curves(YES,oldc1,oldc2,
	                                                  newc1,newc2,
							  oppb1,oppb2,
							  crossb1,crossb2,
							  oppn1,oppn2);
	    }
	}

	if (double_extend_to_cross(flag) == YES)
	{
	    for (i = 0; i < dim; i++)
	    {
	    	Coords(pmid)[i] = 0.5*(Coords(Node_of_o_curve(newc1)->posn)[i] +
	    	                       Coords(Node_of_o_curve(newc2)->posn)[i]);
	    }
#if defined(DEBUG_CROSSING)
	    if (debugging("intersection"))
	    	(void) printf("pmid = (%g, %g)\n",Coords(pmid)[0],
			      Coords(pmid)[1]);
#endif /* defined(DEBUG_CROSSING) */
	    if (robust_extend_bonds_to_cross(bdir1,newc1->orient,on_b1,bdir2,
			                     newc2->orient,on_b2,pmid,s1,s2,
					     pc,fr->rect_grid))
	    {
	    	found_cross = YES;
	    	*pcr = Point(Coords(pc));
#if defined(DEBUG_CROSSING)
	    	if (debugging("intersection"))
	    	{
	    	    (void) printf("Cross found by double extention\n");
	    	    (void) printf("Cross point 1 = (%g, %g)\n",
	    			  Coords(pcr[0])[0],Coords(pcr[0])[1]);
	    	    (void) printf("bdir1\n");	print_bond(bdir1);
	    	    (void) printf("bdir2:\n");	print_bond(bdir2);
	    	}
#endif /* defined(DEBUG_CROSSING) */
	    	*crossb1 = Bond_at_node_of_o_curve(newc1);
	    	*crossb2 = Bond_at_node_of_o_curve(newc2);
	    }
	}
	return leave_intersection_of_two_o_curves(found_cross,oldc1,oldc2,
	                                          newc1,newc2,oppb1,oppb2,
						  crossb1,crossb2,oppn1,oppn2);
}		/*end intersection_of_two_o_curves*/


/*ARGSUSED*/
LOCAL	boolean leave_intersection_of_two_o_curves(
	boolean    found_cross,
	O_CURVE *oldc1,
	O_CURVE *oldc2,
	O_CURVE *newc1,
	O_CURVE *newc2,
	BOND    *oppb1,
	BOND    *oppb2,
	BOND    **crossb1,
	BOND    **crossb2,
	NODE    *oppn1,
	NODE    *oppn2)
{
	if (oldc1 && oldc1->curve)
	{
	    int dim = oldc1->curve->interface->dim;
	    if (newc1->orient == POSITIVE_ORIENTATION) 
	    {
	    	Bond_at_node_of_o_curve(newc1)->start =
		    Node_of_o_curve(newc1)->posn;
	    	if (oppb1 != NULL && oppn1 != NULL)
	    	    oppb1->end = oppn1->posn;
	    }
	    else 
	    {
	    	Bond_at_node_of_o_curve(newc1)->end =
	    	    Node_of_o_curve(newc1)->posn;
	    	if (oppb1 != NULL && oppn1 != NULL)
	    	    oppb1->start = oppn1->posn;
	    }
		set_bond_length(Bond_at_node_of_o_curve(newc1),dim);
		set_bond_length(oppb1,dim);
	}
	if (oldc2 && oldc2->curve)
	{
	    int dim = oldc2->curve->interface->dim;
	    if (newc2->orient == POSITIVE_ORIENTATION) 
	    {
	    	Bond_at_node_of_o_curve(newc2)->start =
	    	    Node_of_o_curve(newc2)->posn;
	    	if (oppb2 != NULL && oppn2 != NULL)
	    	    oppb2->end = oppn2->posn;
	    }
	    else 
	    {
	    	Bond_at_node_of_o_curve(newc2)->end =
	    	    Node_of_o_curve(newc2)->posn;
	    	if (oppb2 != NULL && oppn2 != NULL)
	    	{
	    	    oppb2->start = oppn2->posn;
	    	}
	    }
	    set_bond_length(Bond_at_node_of_o_curve(newc2),dim);
	    set_bond_length(oppb2,dim);
	}

#if defined(DEBUG_CROSSING)
	if (debugging("intersection")) 
	{
	    if (found_cross == YES)
	    {
	    	(void) printf("Crossing bonds at end of "
	    	              "intersection_of_two_o_curves\n");
	    	(void) printf("Crossb1\n");
	    	print_bond(*crossb1);
	    	(void) printf("Crossb2\n");
	    	print_bond(*crossb2);
	    }
	    else
	    	(void) printf("found_cross = NO\n");
	    (void) printf("Left intersection_of_two_o_curves()\n");
	}
#endif /* defined(DEBUG_CROSSING) */
	return found_cross;
}		/*end leave_intersection_of_two_o_curves*/


/*
*			init_curve_for_crossing():
*/

EXPORT void init_curve_for_crossing(
	FT_POINT		*p,
	FT_POINT		*p_opp,
	BOND		*bvirtual,
	O_CURVE		*oldc,
	O_CURVE		*newc,
	NODE		**oppn,
	BOND		**oppb,
	Front		*fr,
	POINTER		wave,
	double		dt,
	double		*V,
	NODE_FLAG	flag)
{
	Locstate	lst, rst;
	int		i, dim = fr->interf->dim;

	if (propagation_status(Node_of_o_curve(newc)) == PROPAGATED_NODE)
	{
	    for (i = 0; i < dim; i++)
	    {
	    	Coords(p)[i] = Coords(Node_of_o_curve(newc)->posn)[i];
	    	V[i] = 0.0;
	    }
	    ft_assign(left_state(p),Left_state_at_node_of_o_curve(newc),
		   fr->sizest);
	    ft_assign(right_state(p),
	    	   Right_state_at_node_of_o_curve(newc),fr->sizest);

		/* init_curve_for_crossing() must ensure  */
		/* that these values are set upon return. */
		/* See comment regarding the Celerity in  */
		/*  crossing_of_two_propagated_curves()   */
	}
	else
	{
	    point_propagate(fr,wave,Node_of_o_curve(oldc)->posn,p,
			    Bond_at_node_of_o_curve(oldc),oldc->curve,dt,V);
	}

	if (oldc->orient != newc->orient)
	    reverse_states_at_point(p,fr);
	set_virtual_opp_node(p_opp,oldc,newc,oppn,oppb,fr,wave,dt);
	set_virtual_bond_at_node(p,bvirtual,newc->curve,newc->orient,
		                 fr,dim,flag);

	/* Set default node states */

	if (propagation_status(Node_of_o_curve(newc)) != PROPAGATED_NODE)
	{
	    lst = Left_state_at_node_of_o_curve(newc);
	    ft_assign(lst,left_state(p),fr->sizest);
	    rst = Right_state_at_node_of_o_curve(newc);
	    ft_assign(rst,right_state(p),fr->sizest);
	}
}		/*end init_curve_for_crossing*/


/*
*			set_virtual_bond_at_node():
*
*	Sets a bond to coincide with first bond of a curve, with displaced
*	node postion at the given point. States at node are ft_assigned.
*/

/*ARGSUSED*/
EXPORT void set_virtual_bond_at_node(
	FT_POINT		*p,
	BOND		*b,
	CURVE		*c,
	ORIENTATION	orient,
	Front		*fr,
	int		dim,
	NODE_FLAG	flag)
{
	size_t		sizest = fr->sizest;
	BOND		*bond_at_node;

	bond_at_node = Bond_at_node(c,orient);
	if ((c->num_points > 3) &&
	    (set_virtuals_by_adjacent_bond(flag) == YES))
	{
	    p = Point_adjacent_to_node(c,orient);
	    bond_at_node = Following_bond(bond_at_node,orient);
	}
	if (orient == POSITIVE_ORIENTATION) 
	{
	    b->start = p;
	    b->end = bond_at_node->end;
	    b->prev = NULL;
	    b->next = bond_at_node->next;
	}
	else 
	{
	    b->end = p;
	    b->start = bond_at_node->start;
	    b->next = NULL;
	    b->prev = bond_at_node->prev;
	}
	ft_assign(Left_state_at_node(c,orient),left_state(p),sizest);
	ft_assign(Right_state_at_node(c,orient),right_state(p),sizest);
	bond_length(b) = separation(b->start,b->end,dim);
}		/*end set_virtual_bond_at_node*/


/*
*			set_virtual_opp_node():
*
*	If the opposite node along a curve is unpropagated, this routine will
*	move it to a propagated position, while returning its original x,y
*	coordinates.
*/

LOCAL void set_virtual_opp_node(
	FT_POINT		*p,
	O_CURVE		*oldc,
	O_CURVE		*newc,
	NODE		**opposite_node,
	BOND		**opposite_bond,
	Front		*fr,
	POINTER		wave,
	double		dt)
{
	BOND		*oppb;
	NODE		*oppn;
	double		V[MAXD];
	int		i, dim = fr->interf->dim;

	*opposite_node = oppn = Opp_node_of_o_curve(newc);
	oppb = Bond_at_opp_node_of_o_curve(newc);
	if (opposite_bond != NULL)
	    *opposite_bond = oppb;
	if (propagation_status(oppn) == PROPAGATED_NODE) 
	{
	    for (i = 0; i < dim; i++)
		Coords(p)[i] = Coords(oppn->posn)[i];
	    return;
	}
	else 
	{
	    point_propagate(fr,wave,Opp_node_of_o_curve(oldc)->posn,p,
			    Bond_at_opp_node_of_o_curve(oldc),oldc->curve,dt,V);
	    if (oldc->orient != newc->orient)
	    	reverse_states_at_point(p,fr);

	    ft_assign(Left_state_at_opp_node_of_o_curve(newc),
	    		left_state(p),fr->sizest);
	    ft_assign(Right_state_at_opp_node_of_o_curve(newc),
				right_state(p),fr->sizest);
	}
	if (newc->orient == NEGATIVE_ORIENTATION)
	    oppb->start = p;
	else
	    oppb->end = p;
	set_bond_length(oppb,dim);
}		/*end set_virtual_opp_node*/


/*
*			set_vel_of_crossing_node():
*
*	Determines the node velocity as defined by two old and two new
*	bonds, the latter propagated by time dt starting from the former.
*	It is assumed that for smaller dt1 < dt, the intermediate propagation
*	of the bonds, defined by linear interpolation, intersect, and define a
*	new node position for time propagation dt1. The node vel is determined
*	from this and stored in the given node.
*/


/*ARGSUSED*/
EXPORT void set_vel_of_crossing_node(
	BOND		*b1,
	BOND		*b2,
	BOND		*newb1,
	BOND		*newb2,
	int		wt1,
	int		wt2,
	NODE		*oldn,
	NODE		*newn,
	double		dt,
	Front		*fr)
{
	RECT_GRID	*gr = fr->rect_grid;
	FT_POINT		p;
	double		s1,s2;
	double		d1[MAXD], d2[MAXD];
	double		s1_hold[MAXD], s2_hold[MAXD];
	double		e1_hold[MAXD], e2_hold[MAXD];
	double		b1_len_hold;
	double		b2_len_hold;
	int		n_vel_set;
	int		i, j, dim = fr->interf->dim;
	static const double EXTEND_FACTOR = 30.0; /*TOLERANCE*/
#if defined(DEBUG_CROSSING)
	double		dt_current = dt;
#endif /* defined(DEBUG_CROSSING) */
	int		on_b1,on_b2;

	on_b1 = (wt1 < FIRST_SCALAR_PHYSICS_WAVE_TYPE) ? YES : NO;
	on_b2 = (wt2 < FIRST_SCALAR_PHYSICS_WAVE_TYPE) ? YES : NO;
	debug_print("set_vel","Entered set_vel_of_crossing_node()\n");
#if defined(DEBUG_CROSSING)
	if (debugging("set_vel"))
	{
	    (void) printf("oldn\n");
	    print_node(oldn);
	    (void) printf("newn\n");
	    print_node(newn);
	    (void) printf("Bond b1, ");	print_bond(b1);
	    (void) printf("Bond newb1, ");	print_bond(newb1);
	    (void) printf("Bond b2, ");	print_bond(b2);
	    (void) printf("Bond newb2, ");	print_bond(newb2);
	    (void) printf("dt = %g\n",dt);
	}
#endif /* defined(DEBUG_CROSSING) */
	propagation_status(newn) = VEL_COMPUTED_NODE;
	Node_vel(newn)[0] = 0.0;
	Node_vel(newn)[1] = 0.0;

		/* Extend virtual bonds newb1, newb2 to ensure cross */

	for (i = 0; i < dim; i++)
	{
	    s1_hold[i] = Coords(newb1->start)[i];
	    e1_hold[i] = Coords(newb1->end)[i];
	    d1[i] = e1_hold[i] - s1_hold[i];
	    s2_hold[i] = Coords(newb2->start)[i];
	    e2_hold[i] = Coords(newb2->end)[i];
	    d2[i] = e2_hold[i] - s2_hold[i];
	}
	b1_len_hold = bond_length(newb1);
	b2_len_hold = bond_length(newb2);

	for (i = 0; i < dim; i++)
	{
	    Coords(newb1->start)[i] -= EXTEND_FACTOR * d1[i];
	    Coords(newb1->end)[i]   += EXTEND_FACTOR * d1[i];
	    Coords(newb2->start)[i] -= EXTEND_FACTOR * d2[i];
	    Coords(newb2->end)[i]   += EXTEND_FACTOR * d2[i];
	}
	set_bond_length(newb1,dim);
	set_bond_length(newb2,dim);
#if defined(DEBUG_CROSSING)
	if (debugging("set_vel"))
	{
	    (void) printf("newb1 after extension, ");
	    print_bond(newb1);
	    (void) printf("newb2 after extension, ");
	    print_bond(newb2);
	}
#endif /* defined(DEBUG_CROSSING) */

	n_vel_set = NO;
	for (j = 0; j < 10; j++)
	{
	    if (robust_cross_bonds(newb1,on_b1,newb2,on_b2,&s1,&s2,gr,&p)) 
	    {
	        n_vel_set = YES;
	        for (i = 0; i < dim; i++)
	        {
	            Node_vel(newn)[i] =
			(Coords(&p)[i] - Coords(oldn->posn)[i]) / dt;
		}
		break;
	    }

	    for (i = 0; i < dim; i++)
	    {
	        Coords(newb1->start)[i] -= EXTEND_FACTOR * d1[i];
	        Coords(newb1->end)[i]   += EXTEND_FACTOR * d1[i];
	        Coords(newb2->start)[i] -= EXTEND_FACTOR * d2[i];
	        Coords(newb2->end)[i]   += EXTEND_FACTOR * d2[i];
	    }
	    set_bond_length(newb1,dim);
	    set_bond_length(newb2,dim);
#if defined(DEBUG_CROSSING)
	    if (debugging("set_vel"))
	    {
	        (void) printf("newb1 after extension, ");
	        print_bond(newb1);
	        (void) printf("newb2 after extension, ");
	        print_bond(newb2);
	    }
#endif /* defined(DEBUG_CROSSING) */

	}
	if (n_vel_set == NO)
	{
	    if (oldn != NULL)
	    {
	        for (i = 0; i < dim; i++)
	            Node_vel(newn)[i] = Node_vel(oldn)[i];
	    }
	}
	for (i = 0; i < dim; i++)
	{
	    Coords(newb1->start)[i] = s1_hold[i];
	    Coords(newb1->end)[i]   = e1_hold[i];

	    Coords(newb2->start)[i] = s2_hold[i];
	    Coords(newb2->end)[i]   = e2_hold[i];
	}

	bond_length(newb1) = b1_len_hold;
	bond_length(newb2) = b2_len_hold;

#if defined(DEBUG_CROSSING)
	if (debugging("set_vel"))
	{
	    (void) printf("computed node velocity = (%g, %g), ",
			  Node_vel(newn)[0],Node_vel(newn)[1]);
	    (void) printf("dt/dt_current = %g",dt/dt_current);
	    (void) printf("\nLeft set_vel_of_crossing_node()\n");
	}
#endif /* defined(DEBUG_CROSSING) */
}		/*end set_vel_of_crossing_node*/



EXPORT	int set_node_velocity(
	FT_POINT		*newp,
	FT_POINT		*oldp,
	NODE		*newn,
	O_CURVE		*oldc1,
	O_CURVE		*oldc2,
	double		*v1,
	double		*v2,
	Front		*fr,
	double		dt,
	double		*dt_frac)
{
	double		*h = fr->rect_grid->h;
	double		d[MAXD];
	double		den, num1, num2;
	double		t1[MAXD], t2[MAXD];
	int		i, dim = fr->rect_grid->dim;
	int		status;
	
	debug_print("set_node_velocity","Entered set_node_velocity()\n");
	for (i = 0; i < dim; i++)
	    d[i] = Coords(newp)[i] - Coords(oldp)[i];
#if defined(DEBUG_CROSSING)
	if (debugging("set_node_velocity"))
	{
	    (void) printf("dt = %g, h = (%g, %g)\n",dt,h[0],h[1]);
	    (void) printf("d = (%g, %g)\n",d[0],d[1]);
	    (void) printf("v1 = (%g, %g), v2 = (%g, %g)\n",v1[0],v1[1],
			  v2[0],v2[1]);
	}
#endif /* defined(DEBUG_CROSSING) */
	if (((fabs(d[0]/h[0]) + fabs(d[1]/h[1])) < .01) ||
			dt < MIN_SCALED_LENGTH(fr->interf)*min(h[0],h[1])) 
	{
	    find_tangent_to_curve(Node_of_o_curve(oldc1)->posn,
			          Bond_at_node_of_o_curve(oldc1),oldc1->curve,
			          oldc1->orient,t1,fr);
	    find_tangent_to_curve(Node_of_o_curve(oldc2)->posn,
			          Bond_at_node_of_o_curve(oldc2),oldc2->curve,
			          oldc2->orient,t2,fr);
	    for (i = 0; i < dim; i++)
	    	Node_vel(newn)[i] = 0.5*(v1[i] + v2[i]);
	    (void) vector_product(t1,t2,&den,dim);
	    num1 = t2[0]*(v1[1] - v2[1]) - t2[1]*(v1[0] - v2[0]);
	    num2 = t1[0]*(v1[1] - v2[1]) - t1[1]*(v1[0] - v2[0]);
#if defined(DEBUG_CROSSING)
	    if (debugging("set_node_velocity"))
	    {
	        (void) printf("t1 = (%g, %g), t2 = (%g, %g)\n",
	    		      t1[0],t1[1],t2[0],t2[1]);
	        (void) printf("den = %g, num1 = %g, num2 = %g\n",den,num1,num2);
	    }
#endif /* defined(DEBUG_CROSSING) */
	    if (fabs(den) > 0.0)
	    {
	    	for (i = 0; i < dim; i++)
		    Node_vel(newn)[i] += 0.5 * (num1*t1[i]+num2*t2[i])/den;
	    }
	}
	else 
	{
	    for (i = 0; i < dim; i++)
	    	Node_vel(newn)[i] = d[i]/dt;
	}
	if (debugging("set_node_velocity"))
	    print_general_vector("Node velocity = ",Node_vel(newn),dim,"\n");
	status = velocity_satisfies_CFL(newn,dt,dt_frac,fr);
	debug_print("set_node_velocity","Left set_node_velocity()\n");
	return status;
}		/*end set_node_velocity*/

EXPORT	void reverse_states_at_point(
	FT_POINT		*p,
	Front		*fr)
{
	static Locstate	stmp = NULL;

	if (stmp == NULL)
	    alloc_state(fr->interf,&stmp,fr->sizest);

	ft_assign(stmp,left_state(p),fr->sizest);
	ft_assign(left_state(p),right_state(p),fr->sizest);
	ft_assign(right_state(p),stmp,fr->sizest);
}		/*end reverse_states_at_point*/


EXPORT	int check_cross(
	double	 s1,
	BOND	*b1,
	O_CURVE	*c1,
	double	 s2,
	BOND	*b2,
	O_CURVE	*c2,
	FT_POINT	*pc,
	double	*L,
	double	*U,
	int	 dim)
{
	int   status;

#if defined(DEBUG_CROSSING)
	debug_print("check_cross","Entered check_cross()\n");
	if (debugging("check_cross")) 
	{
	    (void) printf("Checking cross (%g, %g), s1 = %g, s2 = %g\n",
			Coords(pc)[0],Coords(pc)[1],s1,s2);
	    (void) printf("Between bonds b1 %llu and b2 %llu\n",
		       bond_number(b1,current_interface()),
		       bond_number(b2,current_interface()));
	    print_bond(b1);		print_bond(b2);
	    (void) printf("c1\n");	print_o_curve(c1);
	    (void) printf("c2\n");	print_o_curve(c2);
	    (void) printf("L = <%g, %g>, U = <%g, %g>\n",L[0],L[1],U[0],U[1]);
	}
#endif /* defined(DEBUG_CROSSING) */

	status = GOOD_CROSS;
	if ((b1 && c1 && end_of_curve(s1,b1,c1->curve,c1->orient)) ||
	    (b2 && c2 && end_of_curve(s2,b2,c2->curve,c2->orient)))
	{
	    status &= ~GOOD_CROSS;
	    status |= END_OF_CURVE_CROSS;
	}
	if (outside_point(Coords(pc),L,U,dim))
	{
	    status &= ~GOOD_CROSS;
	    status |= OUT_OF_BOUNDS_CROSS;
	}
	
#if defined(DEBUG_CROSSING)
	if (debugging("check_cross"))
	{
	    if (status == GOOD_CROSS)
	    	(void) printf("Good cross\n");
	    else if ((status & END_OF_CURVE_CROSS) && 
	    	     (status & OUT_OF_BOUNDS_CROSS))
	    	(void) printf("End of curve and out of bounds cross\n");
	    else if (status & END_OF_CURVE_CROSS)
	    	(void) printf("End of curve cross\n");
	    else if (status & OUT_OF_BOUNDS_CROSS)
	    	(void) printf("Out of bounds cross\n");
	}
	debug_print("check_cross","Left check_cross()\n");
#endif /* defined(DEBUG_CROSSING) */

	return status;
}		/*end check_cross*/

#if defined(DEBUG_CROSSING)
LOCAL	void debug_print_improper_cross(
	FT_POINT		*pc,
	double		s1,
	double		s2,
	BOND		*newb1cr,
	CURVE		*newc1,
	BOND		*newb2cr,
	CURVE		*newc2,
	double		*low,
	double		*high,
	Front		*fr)
{
	if (debugging("crossing"))
	{
		(void) printf("Cross found at end of curve or out of bounds\n");
		(void) printf("pc (%llu): %g %g  s1 = %g s2 = %g\n",
			point_number(pc),Coords(pc)[0],Coords(pc)[1],s1,s2);
		(void) printf("newb1cr:\n");
		print_bond_and_states(newb1cr,newc1,fr);
		(void) printf("newb2cr:\n");
		print_bond_and_states(newb2cr,newc2,fr);
		(void) printf("low = <%g, %g>, high = <%g, %g>\n",
			low[0],low[1],high[0],high[1]);
	}
}		/*end debug_print_improper_cross*/
#endif /* defined(DEBUG_CROSSING) */
#endif /* defined(TWOD) */
