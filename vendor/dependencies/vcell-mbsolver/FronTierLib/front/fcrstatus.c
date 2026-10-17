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
*				fcrstatus.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Contains routines for setting cross status when curve crossing
*	functions fail.  This information is used in setting up
*	rproblems.
*/

#if defined(TWOD)

#include <front/fdecs.h>

	/* LOCAL Function Declarations */
LOCAL	int	are_curves_close(CURVE*,CURVE*,RECT_GRID*);
LOCAL	int	degen_cr_tr(double**,int,double*,double,double*,
			    FT_POINT*,FT_POINT*,FT_POINT*);
LOCAL	int	robust_H_extend_cross_trace(RECT_GRID*,FT_POINT*,FT_POINT*,BOND*,
					BOND*,FT_POINT*,double*,int,double*,FT_POINT*);
LOCAL	int	robust_cross_curves_trace(BOND*,BOND*,BOND*,BOND*,
					  ORIENTATION,ORIENTATION,
					  BOND*,BOND*,BOND*,BOND*,
					  ORIENTATION,ORIENTATION,
					  double*,FT_POINT*,Front*);
LOCAL	int	unpropagated_nodes_remain_on_interface(NODE*,NODE*,INTERFACE*);
LOCAL	void	find_partial_time_cross(BOND*,BOND*,BOND*,BOND*,
					ORIENTATION,ORIENTATION,
					NODE*,NODE*,FT_POINT*,double*,FT_POINT*,
					NODE**,Front*);
LOCAL	void	find_partial_time_cross_by_extension(O_CURVE*,O_CURVE*,BOND*,
						     NODE*,NODE*,FT_POINT*,double*,
						     FT_POINT*,NODE**,Front*);
LOCAL	void	leaving_robust_cross_trace(int,double,FT_POINT*);

LOCAL const double SC_TOL = 1.5; /*TOLERANCE*/

/*ARGSUSED*/
EXPORT int find_cross_status(
	int		cr_stat,
	O_CURVE		*oldc1,
	O_CURVE		*newc1,
	O_CURVE		*oldc2,
	O_CURVE		*newc2,
	FT_POINT		*p1_opp,
	FT_POINT		*p2_opp,
	BOND		*b1virtual,
	BOND		*b2virtual,
	FT_POINT		*pans,
	NODE		**interact_node1,
	NODE		**interact_node2,
	Front		*fr,
	POINTER		wave,
	double		dt,
	double		*dt_frac)
{
	RECT_GRID	*rgr = fr->rect_grid;
	NODE		*endn_newc1,*endn_newc2;
	NODE		*endn_c1,*endn_c2;
	BOND		*b,*newb;
	BOND		Btmp;
	double		dt_tmp;
	int		dim = rgr->dim;
	int		sc1,sc2;      /* YES for short curves; otherwise NO */
	static	FT_POINT	*ptmp = NULL;

	debug_print("status","Entered find_cross_status()\n");

	if (ptmp == NULL) 
	{
		ptmp = Static_point(fr->interf);
	}
	*interact_node1 = NULL;
	*interact_node2 = NULL;
	endn_c1 = Opp_node_of_o_curve(oldc1);
	endn_c2 = Opp_node_of_o_curve(oldc2);
	endn_newc1 = Opp_node_of_o_curve(newc1);
	endn_newc2 = Opp_node_of_o_curve(newc2);

	if (unpropagated_nodes_remain_on_interface(Node_of_o_curve(newc1),
		Node_of_o_curve(newc2),newc1->curve->interface))
	{
	    debug_print("status","Left find_cross_status(), status = %s\n",
			   "PSEUDOCROSS_NODE_NODE");
	    return PSEUDOCROSS_NODE_NODE;
	}
	sc1 = is_short_curve(oldc1->curve,oldc1->orient,rgr,SC_TOL);
	sc2 = is_short_curve(oldc2->curve,oldc2->orient,rgr,SC_TOL);
	if (debugging("status"))
	    (void) printf("is_short_curve: sc1 %d  sc2 %d\n",sc1,sc2);


	if (!sc1 && !sc2) 
	{
	    BOND *oldb1s, *newb1s, *oldb1e, *newb1e;
	    BOND *oldb2s, *newb2s, *oldb2e, *newb2e;


	    if (debugging("status"))
	    {
	        (void) printf("In find_cross_status\n");
	        (void) printf("WARNING: No intersection of long curves\n");
	    }
	    oldb1s = Bond_at_node_of_o_curve(oldc1);
	    newb1s = b1virtual;
	    oldb1e = Bond_at_opp_node_of_o_curve(oldc1);
	    newb1e = Bond_at_opp_node_of_o_curve(newc1);
	    oldb2s = Bond_at_node_of_o_curve(oldc2);
	    newb2s = b2virtual;
	    oldb2e = Bond_at_opp_node_of_o_curve(oldc2);
	    newb2e = Bond_at_opp_node_of_o_curve(newc2);

	    if (cr_stat == NO_CROSS &&
	    	robust_cross_curves_trace(oldb1s,newb1s,oldb1e,newb1e,
					  oldc1->orient,newc1->orient,
					  oldb2s,newb2s,oldb2e,newb2e,
					  oldc2->orient,newc2->orient,
					  dt_frac,pans,fr))
	    {
	    	*interact_node1 = endn_newc1;
	    	*interact_node2 = endn_newc2;
	    	debug_print("status","Left find_cross_status(), status = %s\n",
			       "CROSS_PAST_CURVE_NODE");
		return CROSS_PAST_CURVE_NODE;
	    }
	    else
	    {
		(void) printf("WARNING in find_cross_status(), "
			      "ERROR_NODE status found at long curves\n");
	    	debug_print("status","Left find_cross_status(), status = %s\n",
				"ERROR_NODE");
		return ERROR_NODE;
	    }
	}

	    /* Compute the cross time and nodes */

	if (sc1) 
	{
	    b = Bond_at_node_of_o_curve(oldc2);
	    newb = b2virtual;
	    for (; b && newb; b = Following_bond(b,oldc2->orient),
			newb = Following_bond(newb,newc2->orient)) 
	    {
	    	if (robust_cross_trace(rgr,endn_c1->posn,p1_opp,
				       b,newb,&dt_tmp,ptmp))
		{
		    *interact_node1 = endn_newc1;
		    if (dt_tmp <= *dt_frac)
		    {
			*dt_frac = min(dt_tmp,*dt_frac);
			Coords(pans)[0] = Coords(ptmp)[0];
			Coords(pans)[1] = Coords(ptmp)[1];
		    }
		}
	    }
	    if (b == NULL && newb != NULL)
	    {
		Btmp.start = Btmp.end = Opp_node_of_o_curve(oldc2)->posn;
		set_bond_length(&Btmp,dim);
		for (; newb != NULL; newb = Following_bond(newb,newc2->orient)) 
		{
		    if (robust_cross_trace(rgr,endn_c1->posn,p1_opp,&Btmp,newb,
					   &dt_tmp,ptmp))
		    {
			*interact_node1 = endn_newc1;
			if (dt_tmp <= *dt_frac)
			{
			    *dt_frac = min(dt_tmp,*dt_frac);
			    Coords(pans)[0] = Coords(ptmp)[0];
			    Coords(pans)[1] = Coords(ptmp)[1];
			}
		    }
		}
	    }
	    else if (newb == NULL && b != NULL)
	    {
		Btmp.start = Btmp.end = Opp_node_of_o_curve(newc2)->posn;
		set_bond_length(&Btmp,dim);
		for (; b != NULL; b = Following_bond(b,oldc2->orient)) 
		{
		    if (robust_cross_trace(rgr,endn_c1->posn,p1_opp,b,&Btmp,
				           &dt_tmp,ptmp))
		    {
			*interact_node1 = endn_newc1;
			if (dt_tmp <= *dt_frac)
			{
			    *dt_frac = min(dt_tmp,*dt_frac);
			    Coords(pans)[0] = Coords(ptmp)[0];
			    Coords(pans)[1] = Coords(ptmp)[1];
			}
		    }
		}
	    }
	    if (debugging("status"))
	    {
		(void) printf("c1 short: interact_node %llu "
		              "pans %g %g dt_frac %g\n",
			      node_number(*interact_node1),
			      Coords(pans)[0],Coords(pans)[1],*dt_frac);
	    }
	}
	if (sc2) 
	{
	    b = Bond_at_node_of_o_curve(oldc1);
	    newb = b1virtual;
	    for (; b && newb; b = Following_bond(b,oldc1->orient),
	    	newb = Following_bond(newb,newc1->orient)) 
	    {
	    	if (robust_cross_trace(rgr,endn_c2->posn,p2_opp,
				       b,newb,&dt_tmp,ptmp))
		{
		    *interact_node2 = endn_newc2;
		    if (dt_tmp <= *dt_frac)
		    {
			*dt_frac = min(dt_tmp,*dt_frac);
			Coords(pans)[0] = Coords(ptmp)[0];
			Coords(pans)[1] = Coords(ptmp)[1];
		    }
		}
	    }
	    if (b == NULL && newb != NULL)
	    {
	    	Btmp.start = Btmp.end = Opp_node_of_o_curve(oldc1)->posn;
		set_bond_length(&Btmp,dim);
		for (; newb != NULL; newb = Following_bond(newb,newc1->orient)) 
		{
		    if (robust_cross_trace(rgr,endn_c2->posn,p2_opp,&Btmp,newb,
					   &dt_tmp,ptmp))
		    {
			*interact_node2 = endn_newc2;
			if (dt_tmp <= *dt_frac)
			{
			    *dt_frac = min(dt_tmp,*dt_frac);
			    Coords(pans)[0] = Coords(ptmp)[0];
			    Coords(pans)[1] = Coords(ptmp)[1];
			}
		    }
		}
	    }
	    else if (newb == NULL && b != NULL)
	    {
		Btmp.start = Btmp.end = Opp_node_of_o_curve(newc1)->posn;
		set_bond_length(&Btmp,dim);
		for (; b != NULL; b = Following_bond(b,oldc1->orient)) 
		{
		    if (robust_cross_trace(rgr,endn_c2->posn,p2_opp,b,&Btmp,
					   &dt_tmp,ptmp))
		    {
			*interact_node2 = endn_newc2;
			if (dt_tmp <= *dt_frac)
			{
			    *dt_frac = min(dt_tmp,*dt_frac);
			    Coords(pans)[0] = Coords(ptmp)[0];
			    Coords(pans)[1] = Coords(ptmp)[1];
			}
		    }
		}
	    }
	    if (debugging("status"))
	    {
		(void) printf("c2 short: interact_node %llu "
		              "pans %g %g dt_frac %g\n",
			      node_number(*interact_node2),
			      Coords(pans)[0],Coords(pans)[1],*dt_frac);
	    }
	}
	if (!*interact_node1 && !*interact_node2) 
	{
	    (void) printf("WARNING in find_cross_status(), ");
	    (void) printf("No cross for partial time propagation\n");
	    (void) printf("ERROR_NODE status returned\n");
	    if (debugging("find_cross_status")) 
	    {
	    	(void) printf("dt_tmp *dt_frac = %g %g\n",dt_tmp,*dt_frac);
		(void) printf("sc1 sc2 = %d %d\n",sc1,sc2);
		(void) printf("b1virtual = %llu %g %g -> %g %g\n",
			      bond_number(b1virtual,fr->interf),
			      Coords(b1virtual->start)[0],
			      Coords(b1virtual->start)[1],
			      Coords(b1virtual->end)[0],
			      Coords(b1virtual->end)[1]);
		(void) printf("old,new c1:\n");
		print_o_curve(oldc1);
		print_o_curve(newc1);
		(void) printf("b2virtual = %llu %g %g -> %g %g\n",
			      bond_number(b2virtual,fr->interf),
			      Coords(b2virtual->start)[0],
			      Coords(b2virtual->start)[1],
			      Coords(b2virtual->end)[0],
			      Coords(b2virtual->end)[1]);
		(void) printf("old,new c2:\n");
		print_o_curve(oldc2);
		print_o_curve(newc2);
	    }
	    debug_print("status","Left find_cross_status(), status = %s\n",
				"ERROR_NODE");
	    return ERROR_NODE;
	}
	if (*interact_node1 == *interact_node2)
	    *interact_node2 = NULL;
	debug_print("status","Left find_cross_status(), status = CROSS_NODE_NODE\n");
	return CROSS_NODE_NODE;
}		/*end find_cross_status*/

LOCAL const double EXTEND_FAC = 100.0; /*TOLERANCE*/

/*ARGSUSED*/
EXPORT int find_D_extend_status(
	O_CURVE		*oldc1,
	O_CURVE		*newc1,
	FT_POINT		*p1_opp,
	BOND		*oldb2,
	BOND		*newb2,
	FT_POINT		*pans,
	Front		*fr,
	POINTER		wave,
	double		dt,
	double		*dt_frac)
{
	NODE		*endn_oldc1;
	BOND		B2;
	FT_POINT		Ps,Pe;
	RECT_GRID	*rgr = fr->rect_grid;
	double		d2[MAXD];
	double		dt_tmp;
	int		sc1;	      /* YES for short curves; otherwise NO */

	debug_print("status","Entered find_D_extend_status\n");

	endn_oldc1 = Opp_node_of_o_curve(oldc1);
	if (unpropagated_nodes_remain_on_interface(Node_of_o_curve(newc1),
		(NODE *)NULL,newc1->curve->interface))
	{
	    debug_print("status","Left find_D_extend_status(), status = \n",
				"PSEUDOCROSS_NODE_NODE");
	    return PSEUDOCROSS_NODE_NODE;
	}
	sc1 = is_short_curve(oldc1->curve,oldc1->orient,rgr,SC_TOL);


	if (!sc1) 
	{
	    if (debugging("status"))
	    {
		(void) printf("WARNING in find_D_extend_status(), ");
		(void) printf("No intersection of long curves\n");
		(void) printf("oldb2 -\n");	print_bond(oldb2);
		(void) printf("newb2 -\n");	print_bond(newb2);
		(void) printf("oldc1 -\n");	print_o_curve(oldc1);
		(void) printf("newc1 -\n");	print_o_curve(newc1);
		(void) printf("old interface\n");
		print_interface(oldc1->curve->interface);
	    }
	    debug_print("status","Left find_D_extend_status(), status = "
			   "NO_CROSS_NODE\n");
	    return NO_CROSS_NODE;
	}

		/* Compute the cross time and nodes */

	B2.start = &Ps;
	B2.end = &Pe;
	d2[0] = Coords(newb2->end)[0] - Coords(newb2->start)[0];
	d2[1] = Coords(newb2->end)[1] - Coords(newb2->start)[1];
	if (scaled_hypot(d2,rgr->h,rgr->dim) < MIN_SCALED_LENGTH(fr->interf))
	{
	    *dt_frac = 0.0;
	    debug_print("status","Left find_D_extend_status(), status = "
			   "CROSS_NODE_NODE\n");
	    return CROSS_NODE_NODE;
	}
	COORDS(Ps)[0] = Coords(newb2->start)[0] - EXTEND_FAC*d2[0];
	COORDS(Ps)[1] = Coords(newb2->start)[1] - EXTEND_FAC*d2[1];
	COORDS(Pe)[0] = Coords(newb2->end)[0]   + EXTEND_FAC*d2[0];
	COORDS(Pe)[1] = Coords(newb2->end)[1]   + EXTEND_FAC*d2[1];
	if (!robust_cross_trace(rgr,endn_oldc1->posn,p1_opp,oldb2,&B2,
				   &dt_tmp,pans)) 
	{
	    (void) printf("WARNING in find_D_extend_status(), "
	                  "No intersection for partial time prop\n");
	    (void) printf("ERROR_NODE status being returned\n");
	    if (debugging("status"))
	    {
		(void) printf("dt_tmp %g *dt_frac %g\n",dt_tmp,*dt_frac);
		(void) printf("oldb2 -\n");	print_bond(oldb2);
		(void) printf("newb2 -\n");	print_bond(newb2);
		(void) printf("oldc1 -\n");	print_o_curve(oldc1);
		(void) printf("newc1 -\n");	print_o_curve(newc1);
	    }
	    debug_print("status","Left find_D_extend_status(), status = "
				"ERROR_NODE\n");
	    return ERROR_NODE;
	}
	*dt_frac = min(dt_tmp,*dt_frac);
	debug_print("status","Left find_D_extend_status(), status = %s\n",
				"CROSS_NODE_NODE");
	return CROSS_NODE_NODE;
}		/*end find_D_extend_status*/


/*ARGSUSED*/
EXPORT int find_H_extend_status(
	O_CURVE		*oldc1,
	O_CURVE		*newc1,
	FT_POINT		*p1_opp,
	O_CURVE		*oldc2,
	O_CURVE		*newc2,
	double		*moments,
	int		npts,
	BOND		*newb2,
	FT_POINT		*pans,
	Front		*fr,
	POINTER		wave,
	double		dt,
	double		*dt_frac)
{
	NODE		*endn_oldc1;
	BOND		B2;
	FT_POINT		Ps, Pe;
	int		i, dim = fr->rect_grid->dim;
	RECT_GRID	*rgr = fr->rect_grid;
	double		d2[MAXD];
	int		sc1;	      /* YES for short curves; otherwise NO */

	debug_print("H_extend","Entered find_H_extend_status()\n");
	if (debugging("H_extend_status"))
	{
	    (void) printf("newc1 -\n");	print_o_curve(newc1);
	    (void) printf("newb2 -\n");	print_bond(newb2);
	    (void) printf("newc2 -\n");	print_o_curve(newc2);
	}

	endn_oldc1 = Opp_node_of_o_curve(oldc1);
	if (unpropagated_nodes_remain_on_interface(Node_of_o_curve(newc1),
		Node_of_o_curve(newc2),newc1->curve->interface))
	{
	    debug_print("H_extend","Left find_H_extend_status(), status = %s\n",
				"PSEUDOCROSS_NODE_NODE");
	    return PSEUDOCROSS_NODE_NODE;
	}
	sc1 = is_short_curve(oldc1->curve,oldc1->orient,rgr,SC_TOL);


	if (!sc1) 
	{
	    (void) printf("WARNING in find_H_extend_status(), ");
	    (void) printf("No intersection of long curves\n");
	    if (debugging("H_extend"))
	    {
	    	(void) printf("oldc1 -\n");	print_o_curve(oldc1);
	    	(void) printf("newc1 -\n");	print_o_curve(newc1);
	    	(void) printf("newb2 -\n");	print_bond(newb2);
	    	(void) printf("oldc2 -\n");	print_o_curve(oldc2);
	    	(void) printf("newc2 -\n");	print_o_curve(newc2);
	    }
	    debug_print("H_extend","Left find_H_extend_status(), status = %s\n",
				"NO_CROSS_NODE");
	    return NO_CROSS_NODE;
	}

	    /* Extend newb2 to ensure cross in case PTS are colinear */

	B2.start = &Ps;
	B2.end = &Pe;
	for (i = 0; i < dim; i++)
	{
	    d2[i] = Coords(newb2->end)[i] - Coords(newb2->start)[i];
	    Coords(&Ps)[i] = Coords(newb2->start)[i] - EXTEND_FAC*d2[i];
	    Coords(&Pe)[i] = Coords(newb2->end)[i]   + EXTEND_FAC*d2[i];
	}

	    /* Compute the cross time and nodes */

	if (!robust_H_extend_cross_trace(rgr,endn_oldc1->posn,p1_opp,
		Bond_at_node_of_o_curve(oldc2),&B2,
		Node_of_o_curve(oldc2)->posn,moments,npts,dt_frac,pans)) 
	{
	    (void) printf("WARNING in find_H_extend_status(), "
	                  "No intersection for partial time prop\n");
	    (void) printf("ERROR_NODE status being returned\n");
	    if (debugging("H_extend"))
	    {
	    	(void) printf("*dt_frac %g\n",*dt_frac);
	    	(void) printf("oldc1 -\n");	print_o_curve(oldc1);
	    	(void) printf("newc1 -\n");	print_o_curve(newc1);
	    	(void) printf("oldc2 -\n");	print_o_curve(oldc2);
	    	(void) printf("newc2 -\n");	print_o_curve(newc2);
	    }
	    debug_print("H_extend","Left find_H_extend_status(), status = %s\n",
				"ERROR_NODE");
	    return ERROR_NODE;
	}
	debug_print("H_extend","Left find_H_extend_status(), status = %s\n",
				"CROSS_NODE_NODE");
	return CROSS_NODE_NODE;
}		/*end find_H_extend_status*/


/*ARGSUSED*/
EXPORT int find_cross_or_extend_to_cross_status(
	int		cr_stat,
	O_CURVE		*oldc1,
	O_CURVE		*newc1,
	O_CURVE		*oldc2,
	O_CURVE		*newc2,
	FT_POINT		*p1_opp,
	FT_POINT		*p2_opp,
	BOND		*b1virtual,
	BOND		*b2virtual,
	BOND		*newb1dir,
	BOND		*newb2dir,
	FT_POINT		*pans,
	NODE		**interact_node1,
	NODE		**interact_node2,
	Front		*fr,
	POINTER		wave,
	double		dt,
	double		*dt_frac)
{
	NODE		*endn_newc1,*endn_newc2;
	NODE		*endn_oldc1,*endn_oldc2;
	int		sc1,sc2;      /* YES for short curves; otherwise NO */
	int		curves_close = NO; /* YES if long curves are close */
	double		len,dx,dy;
	double		*h = fr->rect_grid->h;
	int		dim = fr->rect_grid->dim;

	debug_print("status","Entered find_cross_or_extend_to_cross_status\n");

	*interact_node1 = NULL;
	*interact_node2 = NULL;
	endn_oldc1 = Opp_node_of_o_curve(oldc1);
	endn_oldc2 = Opp_node_of_o_curve(oldc2);
	endn_newc1 = Opp_node_of_o_curve(newc1);
	endn_newc2 = Opp_node_of_o_curve(newc2);
	sc1 = is_short_curve(oldc1->curve,oldc1->orient,fr->rect_grid,SC_TOL);
	sc2 = is_short_curve(oldc2->curve,oldc2->orient,fr->rect_grid,SC_TOL);

	if (unpropagated_nodes_remain_on_interface(Node_of_o_curve(newc1),
		                                   Node_of_o_curve(newc2),
						   newc1->curve->interface))
	{
	    debug_print("status","Left find_cross_or_extend_to_cross_status() "
			"PSEUDOCROSS_NODE_NODE\n");
	    return PSEUDOCROSS_NODE_NODE;
	}
	if (!sc1 && !sc2) 
	{
	    curves_close = are_curves_close(oldc1->curve,oldc2->curve,
					    fr->rect_grid);
	}

	if (debugging("status")) 
	{
	    (void) printf("sc1 = %s, sc2 = %s, curves_close = %s\n",
			  (sc1) ? "YES" : "NO",(sc2) ? "YES" : "NO",
			  (curves_close) ? "YES" : "NO");
	    if (cr_stat == NO_CROSS)
		(void) printf("cr_stat = NO_CROSS\n");
	    else
	    {
	    	(void) printf("cr_stat %d, ",cr_stat);
	    	if (cr_stat & END_OF_CURVE_CROSS)
	    	    (void) printf("END_OF_CURVE_CROSS");
		if (cr_stat & OUT_OF_BOUNDS_CROSS)
		    (void) printf("OUT_OF_BOUNDS_CROSS");
		(void) printf("\n");
	    }
	}

	if (!sc1 && !sc2 && !curves_close)
	{
	    BOND *oldb1s, *newb1s, *oldb1e, *newb1e;
	    BOND *oldb2s, *newb2s, *oldb2e, *newb2e;

	    (void) printf("WARNING in find_cross_or_extend_to_cross_");
	    (void) printf("status(), No intersection of long curves\n");

	    /* Extend virtual bonds */

	    len = scaled_bond_length(b1virtual,h,dim);
	    if (newc1->orient == POSITIVE_ORIENTATION)
	    {
	    	dx = Coords(b1virtual->start)[0] - Coords(b1virtual->end)[0];
		dy = Coords(b1virtual->start)[1] - Coords(b1virtual->end)[1];
		Coords(b1virtual->start)[0] += len*dx;
		Coords(b1virtual->start)[1] += len*dy;
	    }
	    else
	    {
	    	dx = Coords(b1virtual->end)[0] - Coords(b1virtual->start)[0];
		dy = Coords(b1virtual->end)[1] - Coords(b1virtual->start)[1];
		Coords(b1virtual->end)[0] += len*dx;
		Coords(b1virtual->end)[1] += len*dy;
	    }
	    len = scaled_bond_length(b2virtual,h,dim);
	    if (newc2->orient == POSITIVE_ORIENTATION)
	    {
	    	dx = Coords(b2virtual->start)[0] - Coords(b2virtual->end)[0];
		dy = Coords(b2virtual->start)[1] - Coords(b2virtual->end)[1];
		Coords(b2virtual->start)[0] += len*dx;
		Coords(b2virtual->start)[1] += len*dy;
	    }
	    else
	    {
	    	dx = Coords(b2virtual->end)[0] - Coords(b2virtual->start)[0];
		dy = Coords(b2virtual->end)[1] - Coords(b2virtual->start)[1];
		Coords(b2virtual->end)[0] += len*dx;
		Coords(b2virtual->end)[1] += len*dy;
	    }
	    oldb1s = Bond_at_node_of_o_curve(oldc1);
	    newb1s = b1virtual;
	    oldb1e = Bond_at_opp_node_of_o_curve(oldc1);
	    newb1e = Bond_at_opp_node_of_o_curve(newc1);
	    oldb2s = Bond_at_node_of_o_curve(oldc2);
	    newb2s = b2virtual;
	    oldb2e = Bond_at_opp_node_of_o_curve(oldc2);
	    newb2e = Bond_at_opp_node_of_o_curve(newc2);
	    if (cr_stat == NO_CROSS &&
	    	robust_cross_curves_trace(oldb1s,newb1s,oldb1e,newb1e,
					  oldc1->orient,newc1->orient,oldb2s,
					  newb2s,oldb2e,newb2e,oldc2->orient,
					  newc2->orient,dt_frac,pans,fr))
	    {
	    	debug_print("status","Left find_cross_or_extend_to_cross_status(), "
			       "CROSS_PAST_CURVE_NODE\n");
		return CROSS_PAST_CURVE_NODE;
	    }
	    else
	    {
		(void) printf("WARNING in "
			      "find_cross_or_extend_to_cross_status(), "
			      "ERROR_NODE detected\n");
		debug_print("status","Left find_cross_or_extend_to_cross_status() "
			       "ERROR_NODE\n");
		return ERROR_NODE;
	    }
	}

	    /* Compute the cross time and nodes */

	if (sc1 || curves_close) 
	{
	    find_partial_time_cross(Bond_at_node_of_o_curve(oldc2),
	    	                    b2virtual,NULL,NULL,oldc2->orient,
				    newc2->orient,endn_oldc1,endn_newc1,
			            p1_opp,dt_frac,pans,interact_node1,fr);
	    if (!*interact_node1) 
	    {
	    	find_partial_time_cross_by_extension(oldc2,newc2,newb2dir,
						     endn_oldc1,endn_newc1,
						     p1_opp,dt_frac,
				                     pans,interact_node1,fr);
	    }
	}
	if (sc2 || curves_close) 
	{
	    find_partial_time_cross(Bond_at_node_of_o_curve(oldc1),b1virtual,
				    NULL,NULL,oldc1->orient,newc1->orient,
				    endn_oldc2,endn_newc2,p2_opp,dt_frac,pans,
				    interact_node2,fr);
	    if (!*interact_node2) 
	    {
	    	find_partial_time_cross_by_extension(oldc1,newc1,newb1dir,
						     endn_oldc2,endn_newc2,
						     p2_opp,dt_frac,pans,
						     interact_node2,fr);
	    }
	}

	if (!*interact_node1 && !*interact_node2) 
	{
	    (void) printf("WARNING in find_cross_or_extend_to_cross_status(), "
	                  "No cross for partial time propagation\n");
	    (void) printf("ERROR_NODE status being returned\n");
	    if (debugging("status"))
	    {
	    	(void) printf("*dt_frac = %g\n",*dt_frac);
	    	(void) printf("sc1 = %s, sc2 = %s, curves_close = %s\n",
	    		      (sc1) ? "YES" : "NO",(sc2) ? "YES" : "NO",
	    		      (curves_close) ? "YES" : "NO");
	    	(void) printf("b1virtual = %llu %g %g -> %g %g\n",
	    		      bond_number(b1virtual,fr->interf),
	    		      Coords(b1virtual->start)[0],
	    		      Coords(b1virtual->start)[1],
	    		      Coords(b1virtual->end)[0],
	    		      Coords(b1virtual->end)[1]);
	    	(void) printf("newb1dir = %llu %g %g -> %g %g\n",
	    		      bond_number(newb1dir,fr->interf),
	    		      Coords(newb1dir->start)[0],
	    		      Coords(newb1dir->start)[1],
	    		      Coords(newb1dir->end)[0],
	    		      Coords(newb1dir->end)[1]);
	    	(void) printf("oldc1\n");	print_o_curve(oldc1);
	    	(void) printf("newc1\n");	print_o_curve(newc1);
	    	(void) printf("b2virtual\n");	print_bond(b2virtual);
	    	(void) printf("newb2dir\n");	print_bond(newb2dir);
	    	(void) printf("oldc2\n");	print_o_curve(oldc2);
	    	(void) printf("newc2\n");	print_o_curve(newc2);
	    }
	    debug_print("status","Left find_cross_or_extend_to_cross_status(), ",
			   "ERROR_NODE\n");
	    return ERROR_NODE;
	}

	if (*interact_node1 == *interact_node2)
	    *interact_node2 = NULL;

	debug_print("status","Left find_cross_or_extend_to_cross_status(), "
		       "CROSS_NODE_NODE\n");

	return CROSS_NODE_NODE;
}		/*end find_cross_or_extend_to_cross_status*/

LOCAL int are_curves_close(
	CURVE		*c1,
	CURVE		*c2,
	RECT_GRID	*gr)
{
	HYPER_SURF	*hs, *hs1 = Hyper_surf(c1), *hs2 = Hyper_surf(c2);
	HYPER_SURF_ELEMENT *hse;
	INTERFACE	*intfc = c1->interface;
	double		coords_on[MAXD], *coords, t;
	BOND		*b;
	double		max_min_sep, min_sep;
	double		*h = gr->h;
	int		dim = gr->dim;

	coords = Coords(c2->start->posn);
	if (nearest_interface_point(coords,NO_COMP,intfc,NO_BOUNDARIES,
				    hs1,coords_on,&t,&hse,&hs) != YES)
	{
	    screen("ERROR in are_curves_close(), "
	           "nearest_interface_point() failed\n");
	    clean_up(ERROR);
	}
	max_min_sep = _scaled_separation(coords,coords_on,h,dim);

	coords = Coords(c2->end->posn);
	if (nearest_interface_point(coords,NO_COMP,intfc,NO_BOUNDARIES,hs1,
				    coords_on,&t,&hse,&hs) != YES)
	{
	    screen("ERROR in are_curves_close(), "
	           "nearest_interface_point() failed\n");
	    clean_up(ERROR);
	}
	min_sep = _scaled_separation(coords,coords_on,h,dim);
	max_min_sep = max(max_min_sep,min_sep);

	for (b = c2->first; b != c2->last; b = b->next)
	{
	    coords = Coords(b->end);
	    if (
		nearest_interface_point(coords,NO_COMP,intfc,NO_BOUNDARIES,hs1,
					coords_on,&t,&hse,&hs) != YES)
	    {
	        screen("ERROR in are_curves_close(), "
	               "nearest_interface_point() failed\n");
	        clean_up(ERROR);
	    }
	    min_sep = _scaled_separation(coords,coords_on,h,dim);
	    max_min_sep = max(max_min_sep,min_sep);
	}

	coords = Coords(c1->start->posn);
	if (nearest_interface_point(coords,NO_COMP,intfc,NO_BOUNDARIES,hs2,
				    coords_on,&t,&hse,&hs) != YES)
	{
	    screen("ERROR in are_curves_close(), "
	           "nearest_interface_point() failed\n");
	    clean_up(ERROR);
	}
	min_sep = _scaled_separation(coords,coords_on,h,dim);
	max_min_sep = max(max_min_sep,min_sep);

	coords = Coords(c1->end->posn);
	if (nearest_interface_point(coords,NO_COMP,intfc,NO_BOUNDARIES,hs2,
				    coords_on,&t,&hse,&hs) != YES)
	{
	    screen("ERROR in are_curves_close(), "
	           "nearest_interface_point() failed\n");
	    clean_up(ERROR);
	}
	min_sep = _scaled_separation(coords,coords_on,h,dim);
	max_min_sep = max(max_min_sep,min_sep);

	for (b = c1->first; b != c1->last; b = b->next)
	{
	    coords = Coords(b->end);
	    if (nearest_interface_point(coords,NO_COMP,intfc,NO_BOUNDARIES,hs2,
					coords_on,&t,&hse,&hs) != YES)
	    {
	        screen("ERROR in are_curves_close(), "
	               "nearest_interface_point() failed\n");
	        clean_up(ERROR);
	    }
	    min_sep = _scaled_separation(coords,coords_on,h,dim);
	    max_min_sep = max(max_min_sep,min_sep);
	}

	return (max_min_sep < 1.5) ? YES : NO;
}		/*end are_curves_close*/

LOCAL void find_partial_time_cross_by_extension(
	O_CURVE		*oldc,
	O_CURVE		*newc,
	BOND		*newbdir,
	NODE		*oldn,
	NODE		*newn,
	FT_POINT		*newp,
	double		*dt_frac,
	FT_POINT		*pans,
	NODE		**interact_node,
	Front		*fr)
{
	static	BOND	*newb = NULL;
	static	FT_POINT	*ptmp = NULL;
	double		dx, dy, dt_tmp;

	if (ptmp == NULL)
	{
		scalar(&newb,sizeof(BOND));
		newb->start = Static_point(fr->interf);
		newb->end = Static_point(fr->interf);
		ptmp = Static_point(fr->interf);
	}

	dx = Coords(newbdir->end)[0] - Coords(newbdir->start)[0];
	dy = Coords(newbdir->end)[1] - Coords(newbdir->start)[1];
	if (newc->orient == POSITIVE_ORIENTATION) 
	{
		Coords(newb->end)[0] = Coords(newbdir->end)[0];
		Coords(newb->end)[1] = Coords(newbdir->end)[1];
		Coords(newb->start)[0] = Coords(newbdir->end)[0] - 
						EXTEND_FAC*dx;
		Coords(newb->start)[1] = Coords(newbdir->end)[1] - 
						EXTEND_FAC*dy;
	}
	else 
	{
		Coords(newb->start)[0] = Coords(newbdir->start)[0];
		Coords(newb->start)[1] = Coords(newbdir->start)[1];
		Coords(newb->end)[0] = Coords(newbdir->start)[0] + 
						EXTEND_FAC*dx;
		Coords(newb->end)[1] = Coords(newbdir->start)[1] + 
						EXTEND_FAC*dy;
	}
	if (robust_cross_trace(fr->rect_grid,oldn->posn,newp,
		Bond_at_node_of_o_curve(oldc),newb,&dt_tmp,ptmp))
	{
		*interact_node = newn;
		if (dt_tmp <= *dt_frac) 
		{
			*dt_frac = min(dt_tmp,*dt_frac);
			Coords(pans)[0] = Coords(ptmp)[0];
			Coords(pans)[1] = Coords(ptmp)[1];
		}
	}
}		/*end find_partial_time_cross_by_extension*/


LOCAL void find_partial_time_cross(
	BOND		*oldbs,
	BOND		*newbs,
	BOND		*oldbe,
	BOND		*newbe,
	ORIENTATION	oldc_orient,
	ORIENTATION	newc_orient,
	NODE		*oldn,
	NODE		*newn,
	FT_POINT		*newp,
	double		*dt_frac,
	FT_POINT		*pans,
	NODE		**interact_node,
	Front		*fr)
{
	static	FT_POINT	*ptmp = NULL;

	FT_POINT		*oldp = oldn->posn;
	BOND		*b, *newb;
	RECT_GRID	*rgr = fr->rect_grid;
	double		dt_tmp;

	if (ptmp == NULL) 
	{
		ptmp = Static_point(fr->interf);
	}
	b = oldbs;
	newb = newbs;
	while (b && newb)
	{
		if (robust_cross_trace(rgr,oldp,newp,b,newb,&dt_tmp,ptmp))
		{
			*interact_node = newn;
			if (dt_tmp <= *dt_frac)
			{
				*dt_frac = min(dt_tmp,*dt_frac);
				Coords(pans)[0] = Coords(ptmp)[0];
				Coords(pans)[1] = Coords(ptmp)[1];
			}
		}
		if (b == oldbe || newb == newbe) break;
		b = Following_bond(b,oldc_orient);
		newb = Following_bond(newb,newc_orient);
	}
}		/*end find_partial_time_cross*/

EXPORT int find_circle_cross_status(
	O_CURVE		*oldc,
	O_CURVE		*newc,
	FT_POINT		*newp,
	double		radius,
	FT_POINT		*pans,
	Front		*fr,
	double		*dt_frac)
{
	FT_POINT		*oldp;
	int		sc;	      /* YES for short curves; otherwise NO */

	if (unpropagated_nodes_remain_on_interface(Node_of_o_curve(newc),NULL,
						   newc->curve->interface))
	    return PSEUDOCROSS_NODE_NODE;

	sc = is_short_curve(oldc->curve,oldc->orient,fr->rect_grid,SC_TOL);
	if (!sc) 
	{
	    (void) printf("WARNING in find_circle_cross_status(), "
	                  "No intersection of long curves\n");
	    return NO_CROSS_NODE;
	}

		/* Compute the cross time and nodes */

	oldp = Opp_node_of_o_curve(oldc)->posn;
	if (!robust_circle_cross_trace(oldp,newp,
					  Node_of_o_curve(oldc)->posn,NULL,
			                  radius,dt_frac,pans)) 
	{
	    (void) printf("WARNING in find_circle_cross_status(), "
	                  "No intersection for partial time prop\n");
	    (void) printf("ERROR_NODE status being returned\n");
	    return ERROR_NODE;
	}
	return CROSS_NODE_NODE;
}		/*end find_circle_cross_status*/




/*
*			robust_cross_trace():
*
*	Finds the intersection if any of the space time line swept out
*	by a point moving over a fixed time interval and the surface
*	swept out by a moving bond. The data are the initial and final
*	points and bonds. The equation for the intersection is quadratic
*	in the time and the distance along the bond. It is first solved for
*	one of these variables, giving a quadratic equation in the other.
*	The latter is solved by formula, and then it is determined whether
*	the solution corresponds to a real intersection in the allowed
*	intervals.
*	In the code below, h is the arclength along the bond, and one can
*	show that equality of one coordinate defines a fractional linear
*	transformation between t and h,
*	t = (ax + h*bx) / (cx + h*dx) = (ay + h*by) / (cy + h*dy)
*	for known quantities ax, etc.
*
*/

LOCAL const double RCT_MIN = -0.05; /* TOLERANCE */
LOCAL const double RCT_MAX =  1.05; /* TOLERANCE */

EXPORT int robust_cross_trace(
	RECT_GRID	*rgr,
	FT_POINT		*op,	/* old and new points */
	FT_POINT		*np,
	BOND		*ob,	/* old and new bonds  */
	BOND		*nb,
	double		*t_frac,
	FT_POINT		*pans)
{
	INTERFACE	*intfc = current_interface();
	double		p1[MAXD], np1[MAXD];
	double		p2[MAXD], np2[MAXD];
	double		p3[MAXD], np3[MAXD];
	double		h[MAXD], v01[MAXD];
	double		magsv[4];
	double		A,B,C;
	double		tmin, tmax;
	double		t[MAXD];
	double		epsilon = TOL_FAC(intfc)*MACH_EPS;/*TOLERANCE*/
	double          min_sc_sep = MIN_SC_SEP(intfc);
	double		max_mag, max_mag_sqr;
	static const double	RCT_ABS = 1.05; /* TOLERANCE */
	int		i, j, dim = rgr->dim;
	static double	**v = NULL;

	debug_print("r_cross_trace","Entered robust_cross_trace()\n");
	if (v == NULL)
	{
	    bi_array(&v,4,MAXD,DOUBLE);
	}
	for (i = 0; i < dim; i++)
	{
	    p1[i] = (double) Coords(op)[i];
	    np1[i] = (double) Coords(np)[i];
	    p2[i] = (double) Coords(ob->start)[i];
	    np2[i] = (double) Coords(nb->start)[i];
	    p3[i] = (double) Coords(ob->end)[i];
	    np3[i] = (double) Coords(nb->end)[i];
	}
	if (debugging("r_cross_trace"))
	{
	    (void) printf("op = (%g, %g), np = (%g, %g)\n",
			  p1[0],p1[1],np1[0],np1[1]);
	    (void) printf("ob = (%g, %g) -> (%g, %g)\n",
			  p2[0],p2[1],p3[0],p3[1]);
	    (void) printf("nb = (%g, %g) -> (%g, %g)\n",
			  np2[0],np2[1],np3[0],np3[1]);
	}

	*t_frac = HUGE_VAL;
	for (i = 0; i < dim; i++)
	{
	    v[0][i] = p2[i] - p1[i];	v[1][i] = p3[i] - p2[i];
	    h[i] = (double) rgr->h[i];
	    v01[i] = v[0][i] + v[1][i];
	}
	if ((dscaled_hypot(v[0],h,dim) < min_sc_sep) ||
	    (dscaled_hypot(v01,h,dim) < min_sc_sep))
	{
	    /*
	    *  This degenerate case occurs frequently and should be
	    *  eliminated at the start of the function.
	    */

	    *t_frac = 0.0;
	    for (i = 0; i < dim; i++)
		Coords(pans)[i] = Coords(op)[i];
	    leaving_robust_cross_trace(YES,*t_frac,pans);
	    return YES;

	}
	for (i = 0; i < dim; i++)
	{
	    v[2][i] = v[0][i] - (np2[i] - np1[i]);
	    v[3][i] = v[1][i] - (np3[i] - np2[i]);
	}
	max_mag_sqr = 0.0;
	for (i = 0; i < 4; i++)
	{
	    magsv[i] = dscalar_product(v[i],v[i],dim);
	    max_mag_sqr = max(max_mag_sqr,magsv[i]);
	}
	max_mag = sqrt(max_mag_sqr);
	if (debugging("r_cross_trace"))
	{
	    (void) printf("Before normalization\n");
	    for (i = 0; i < 4; i++)
	    {
	    	(void) printf("v[%d] = %g, %g, magsv[%d] = %g\n",
			      i,v[i][0],v[i][1],i,magsv[i]);
	    }
	    (void) printf("max magnitude = %g\n",max_mag);
	}
	if (debugging("r_cross_trace"))
	    (void) printf("epsilon = %g\n",epsilon);
	if (max_mag < epsilon)
	{
	    if (debugging("r_cross_trace"))
	    	(void) printf("All points at same time equal\n");
	    *t_frac = 0.0;
	    for (i = 0; i < dim; i++)
		Coords(pans)[i] = (double) p1[i];
	    leaving_robust_cross_trace(YES,*t_frac,pans);
	    return YES;
	}
	for (i = 0; i < 4; i++)
	{
	    v[i][0] /= max_mag;	v[i][1] /= max_mag;
	    magsv[i] /= max_mag_sqr;
	}
	if (debugging("r_cross_trace"))
	{
	    (void) printf("After normalization\n");
	    for (i = 0; i < 4; i++)
	    {
	    	(void) printf("v[%d] = %g, %g, magsv[%d] = %g\n",
	    		      i,v[i][0],v[i][1],i,magsv[i]);
	    }
	}

	    /*  A * h**2 + B * h + C = 0 */

	A = v[3][0] * v[1][1] - v[3][1] * v[1][0];
	B = v[3][0] * v[0][1] - v[3][1] * v[0][0] +
	    v[2][0] * v[1][1] - v[2][1] * v[1][0];
	C = v[2][0] * v[0][1] - v[2][1] * v[0][0];


	switch(robust_quad_roots_in_interval(h,A,B,C,RCT_MIN,RCT_MAX,epsilon))
	{
	case 1:
	case 2:
	    break;
	case 3:
	    if (degen_cr_tr(v,dim,magsv,epsilon,t_frac,op,np,pans))
	    {
	    	leaving_robust_cross_trace(YES,*t_frac,pans);
	    	return YES;
	    }
	    leaving_robust_cross_trace(NO,*t_frac,pans);
	    return NO;
	case 0:
	default:
	    leaving_robust_cross_trace(NO,*t_frac,pans);
	    return NO;
	}

	for (i = 0; i < 2; i++)
	{
	    double	mag_a, mag_b;
	    double	a[MAXD], b[MAXD];
	    double	max_mag;
	    double	num, den;

	    for (j = 0; j < dim; j++)
	    {
	    	a[j] = v[0][j] + h[i]*v[1][j];
	    	b[j] = v[2][j] + h[i]*v[3][j];
	    }

	    mag_a = sqrt(dscalar_product(a,a,dim));
	    mag_b = sqrt(dscalar_product(b,b,dim));
	    max_mag = max(mag_a,mag_b);

	    if (debugging("r_cross_trace"))
	    {
	    	(void) printf("mag_a = %g, mag_b = %g, max_mag = %g\n",
	    		      mag_a,mag_b,max_mag);
	    }

	    if (max_mag < epsilon)
	    	t[i] = 0.0;
	    else
	    {
		for (j = 0; j < dim; j++)
		{
		    a[j] /= max_mag;
		    b[j] /= max_mag;
		}
		mag_a /= max_mag;
		mag_b /= max_mag;
		num = dscalar_product(a,b,dim);
		den = dscalar_product(b,b,dim);
		if (mag_a > RCT_ABS*mag_b)
		    t[i] = (num >= 0.0) ? HUGE_VAL : -HUGE_VAL;
		else if (num >  RCT_MAX * den)
		    t[i] = HUGE_VAL;
		else if (num < RCT_MIN * den)
		    t[i] = -HUGE_VAL;
		else
		    t[i] = num/den;
	    }
	}
	tmax = max(t[0],t[1]);
	tmin = min(t[0],t[1]);
	if (debugging("r_cross_trace")) 
	    (void) printf("tmin tmax = %g %g\n",tmin,tmax);


	if ((tmin > RCT_MAX) || (tmax < RCT_MIN) ||
	    ((tmin < RCT_MIN) && (tmax > RCT_MAX)))
	{
	    leaving_robust_cross_trace(NO,*t_frac,pans);
	    return NO;
	}
	*t_frac = (tmin > RCT_MIN) ? (double) min(max(tmin,0.0),1.0) :
				     (double) min(max(tmax,0.0),1.0);
	for (i = 0; i < dim; i++)
	{
	    Coords(pans)[i] = p1[i] + (*t_frac)*(np1[i] - p1[i]);
	}
	leaving_robust_cross_trace(YES,*t_frac,pans);
	return YES;
}		/*end robust_cross_trace*/

LOCAL void leaving_robust_cross_trace(
	int		ans,
	double		t_frac,
	FT_POINT		*pans)
{
	if (!debugging("r_cross_trace")) return;
	debug_print("r_cross_trace","Left robust_cross_trace()\n");
	(void) printf("Answer = %s\n",(ans == YES) ? "YES" : "NO");
	if (ans == NO) return;
	(void) printf("t_frac = %g, pans = (%g, %g)\n",t_frac,
		      Coords(pans)[0],Coords(pans)[1]);
}		/*end leaving_robust_cross_trace*/

/*
*			degen_cr_tr():
*
*	Handle the denerate case where triangle formed by the points
*	op, ob->start, ob->end is scalar multiple of the triangle formed
*	by the points np, nb->start, nb->end,  ie the respective sides
*	of the two triangles are parallel. (See robust_cross_trace() above.)
*
*	IMPORTANT NOTE:  This routine assumes that distance from op to
*	both ob->start and ob->end is greater than MIN_SC_SEP(intfc) as this
*	case is eliminated at the beginning of robust_cross_trace().
*	Note that the displacement vector from op to ob->start is the
*	uni_array v[0] while the displacement vector from
*	op to ob->end is v[0]+v[1].  We assume both of these
*	uni_arrays are non-zero in this function.
*/

LOCAL	int degen_cr_tr(
	double		**v,
	int		dim,
	double		*magsv,
	double		epsilon,
	double		*t_frac,
	FT_POINT		*op,
	FT_POINT		*np,
	FT_POINT		*pans)
{
	INTERFACE	*intfc = current_interface();
	double		crpr[6], dpr[6], max_mag_sq, max_cr_pr;
	double		a[4], nx, ny, max_mag;
	double		t[2], tmax, tmin;
	int		i, j, k, imax_mag;

	debug_print("r_cross_trace","degenerate case\n");

	/* Case 1: t = 0 root exists */

	dpr[0] = v[0][0]*v[1][0] + v[0][1]*v[1][1];
	if ((RCT_MIN*magsv[1] < -dpr[0] && -dpr[0] < RCT_MAX*magsv[1]) &&
	(fabs(v[0][0]*v[1][1] - v[0][1]*v[1][0]) <
	     sqrt(magsv[1])*MIN_SC_SEP(intfc)))
	{
		*t_frac = 0.0;
		for (i = 0; i < dim; i++)
			Coords(pans)[i] = Coords(op)[i];
		return YES;
	}

	k = 0;
	imax_mag = 0;
	max_cr_pr = 0.0;
	max_mag_sq = 0.0;
	for (i = 0; i < 4; i++)
	{
		if (magsv[i] > max_mag_sq)
		{
			imax_mag = i;
			max_mag_sq = magsv[i];
		}
		for (j = i+1; j < 4; j++, k++)
		{
			dpr[k]  = v[i][0]*v[j][0] + v[i][1]*v[j][1];
			crpr[k] = v[i][0]*v[j][1] - v[i][1]*v[j][0];
			max_cr_pr = max(max_cr_pr,fabs(crpr[k]));
		}
	}

	/* NOTE: by assumption v[0] has magnitude >= MIN_SC_SEP(intfc) */
	/* so max_mag below is positive					 */


	max_mag = sqrt(magsv[imax_mag]);
	nx = v[imax_mag][0]/max_mag;
	ny = v[imax_mag][1]/max_mag;

	/* Case 2: All uni_arrays parallel */

	if (max_cr_pr < epsilon)
	{
		double s0;
		int increasing;

		for (i = 0; i < 4; i++) a[i] = v[i][0]*nx + v[i][1]*ny;

		if (max(fabs(a[2]),fabs(a[3])) < epsilon) return NO;
		else if (max(fabs(a[1]),fabs(a[3])) < epsilon)
		{
			t[0] = a[0]/a[2];
			if (RCT_MIN < t[0] && t[0] < RCT_MAX)
			{
				*t_frac = (double) min(max(t[0],0.0),1.0);
				Coords(pans)[0] =  Coords(op)[0] + 
				   (*t_frac) * (Coords(np)[0] - Coords(op)[0]);
				Coords(pans)[1] =  Coords(op)[1] + 
				   (*t_frac) * (Coords(np)[1] - Coords(op)[1]);
				return YES;
			}
			else
				return NO;
		}
		else if (fabs(a[3]) < epsilon)
		{
			t[0] = (a[0] + RCT_MIN*a[1])/a[2];
			t[1] = (a[0] + RCT_MAX*a[1])/a[2];
			tmax = max(t[0],t[1]);
			tmin = min(t[0],t[1]);
			if (tmin > RCT_MAX || tmax < RCT_MIN) return NO;
			*t_frac = (tmin < 0.0) ? 0.0 : (double) tmin;
			Coords(pans)[0] =  Coords(op)[0] +
				(*t_frac) * (Coords(np)[0] - Coords(op)[0]);
			Coords(pans)[1] =  Coords(op)[1] +
				(*t_frac) * (Coords(np)[1] - Coords(op)[1]);
			return YES;
		}

		s0 = -a[2]/a[3];
		increasing = (a[1]*a[2] - a[3]*a[0] > 0.0) ? YES : NO;
		if (RCT_MIN < s0 && s0 < RCT_MAX)
		{
			t[0] = (a[0] + RCT_MIN*a[1])/(a[2] + RCT_MIN*a[3]);
			t[1] = (a[0] + RCT_MAX*a[1])/(a[2] + RCT_MAX*a[3]);
			tmax = max(t[0],t[1]);
			tmin = min(t[0],t[1]);
			if (tmin > RCT_MAX || tmax < RCT_MIN) return NO;
			*t_frac = (tmin < 0.0) ? 0.0 : (double) tmin;
			Coords(pans)[0] =  Coords(op)[0] + 
				(*t_frac) * (Coords(np)[0] - Coords(op)[0]);
			Coords(pans)[1] =  Coords(op)[1] + 
				(*t_frac) * (Coords(np)[1] - Coords(op)[1]);

			return YES;
		}
		else if (fabs(a[2] + RCT_MIN*a[3]) < epsilon)
		{
			t[1] = (a[0] + RCT_MAX*a[1])/(a[2] + RCT_MAX*a[3]);
			if ((increasing && t[1] < RCT_MIN) ||
					(!increasing && t[1] > RCT_MAX))
				return NO;
			*t_frac = (increasing) ? 
					0.0 : (double) min(max(t[1],0.0),1.0);
			Coords(pans)[0] =  Coords(op)[0] + 
				(*t_frac) * (Coords(np)[0] - Coords(op)[0]);
			Coords(pans)[1] =  Coords(op)[1] + 
				(*t_frac) * (Coords(np)[1] - Coords(op)[1]);
			return YES;

		}
		else if (fabs(a[2] + RCT_MAX*a[3]) < epsilon)
		{
			t[0] = (a[0] + RCT_MIN*a[1])/(a[2] + RCT_MIN*a[3]);
			if ((!increasing && t[0] < RCT_MIN) ||
					(increasing && t[0] > RCT_MAX))
				return NO;
			*t_frac = (increasing) ? 
					(double) min(max(t[0],0.0),1.0) : 0.0;
			Coords(pans)[0] =  Coords(op)[0] + 
				(*t_frac) * (Coords(np)[0] - Coords(op)[0]);
			Coords(pans)[1] =  Coords(op)[1] + 
				(*t_frac) * (Coords(np)[1] - Coords(op)[1]);
			return YES;
		}
		else
		{
			t[0] = (a[0] + RCT_MIN*a[1])/(a[2] + RCT_MIN*a[3]);
			t[1] = (a[0] + RCT_MAX*a[1])/(a[2] + RCT_MAX*a[3]);
			tmax = max(t[0],t[1]);
			tmin = min(t[0],t[1]);
			if (tmin < RCT_MIN && tmax > RCT_MAX) return NO;
			*t_frac = (tmin >= RCT_MIN) ? 0.0 :
				 		min(max(tmax,0.0),1.0);
			Coords(pans)[0] =  Coords(op)[0] + 
				(*t_frac) * (Coords(np)[0] - Coords(op)[0]);
			Coords(pans)[1] =  Coords(op)[1] + 
				(*t_frac) * (Coords(np)[1] - Coords(op)[1]);
			return YES;
		}
	}

	/* Case 3: v[2] or v[3] zero */

	if (max(magsv[2],magsv[3]) < epsilon) return NO;

	/* Final case */

	t[0] = (v[0][0]*v[2][0] + v[0][1]*v[2][1])/magsv[2];
	t[1] = ((v[0][0]+v[1][0])*(v[2][0]+v[3][0]) +
				(v[0][1]+v[1][1])*(v[2][1]+v[3][1]))/
		((v[2][0]+v[3][0])*(v[2][0]+v[3][0]) +
				(v[2][1]+v[3][1])*(v[2][1]+v[3][1]));
	tmax = max(t[0],t[1]);
	tmin = min(t[0],t[1]);
	if (tmin > RCT_MAX || tmax < RCT_MIN) return NO;
	*t_frac = (tmin < 0.0) ? 0.0 : (double) tmin;
	Coords(pans)[0] =  Coords(op)[0] + 
		(*t_frac) * (Coords(np)[0] - Coords(op)[0]);
	Coords(pans)[1] =  Coords(op)[1] + 
		(*t_frac) * (Coords(np)[1] - Coords(op)[1]);
	return YES;
}		/*end degen_cr_tr*/


/*
*			robust_H_extend_cross_trace():
*
*	A set of points is given which are obtained by obliquely propagating
*	a single node in several directions.  By Huyghen's principal it is 
*	assumed that these points will all lie on a circle which is the 
*	outwardly propagating locus of a wave starting at the given point opc.
*	The radius and center of this circle are calculated by a regression.
*	Namely the center npc = (xcen,ycen) and the radius r are assumed to be
*	such that the function
*
*	number of points
*	-----
*	\
*	/	sqr(sqr(xi - xcen) + sqr(yi - ycen) - sqr(r))
*	-----
*	i = 1
*
*	is a minimun.  ((xi,yi) are the given points)
*	Actually the positions of the propagated points themselves are not
*	passed to this function.  Rather a set of sufficient statistics,
*	the moments up to order three of the coordinates of the points,
*	that is the sums of x, y, x*x, y*y, x*y, x*x*x, y*y*y, x*x*y and
*	x*y*y over the given set of points and the number of points is supplied.
*	Once the center and radius are computed a cross trace is calculated
*	for the line segement connecting the two points op and np, with
*	the locus formed by the expanding circle which starts with center
*	op and radius zero and finishes with center npc and
*	radius r.  The fuction returns YES if successful together with
*	the coordinates of the intersection point and the fractional
*	time of crossing, NO otherwise.
*	If the given set of points are colinear then a robust cross trace
*	is calculated with repect to op, np, ob and nb;
*
*	The moments in the moments array are stored as:
*
*		moments[0] = sum of x,
*		moments[1] = sum of y,
*		moments[2] = sum of x*x,
*		moments[3] = sum of x*y,
*		moments[4] = sum of y*y,
*		moments[5] = sum of x*x*x,
*		moments[6] = sum of x*x*y,
*		moments[7] = sum of x*y*y,
*		moments[8] = sum of y*y*y,
*
*	where the sum is over the given set of propagated points.
*/

LOCAL	int	robust_H_extend_cross_trace(
	RECT_GRID	*rgr,
	FT_POINT		*op,
	FT_POINT		*np,
	BOND		*ob,
	BOND		*nb,
	FT_POINT		*opc,
	double		*moments,
	int		npts,
	double		*t,
	FT_POINT		*pans)
{
	FT_POINT		NPC;
	double		r,xcen,ycen,numxc,numyc,den,xbar,ybar,rsqr;
	double		a1,a2,a3,a4,a5,a6,a7;

	debug_print("H_extend","Entered robust_H_extend_cross_trace()\n");
	if (npts <= 1) 
	{
		(void) printf("WARNING: in robust_H_extend_cross_trace(), ");
		(void) printf("Not enough points for calculation\n");
		(void) printf("Using robust_cross_trace()\n");
		return robust_cross_trace(rgr,op,np,ob,nb,t,pans);
	}
	xbar = moments[0]/npts;	ybar = moments[1]/npts;
	a1 = moments[2] - sqr(moments[0])/npts;
	a2 = moments[4] - sqr(moments[1])/npts;
	a3 = moments[3] - moments[0]*moments[1]/npts;
	a4 = moments[5] - 3.*moments[0]*moments[2]/npts;
	a5 = moments[8] - 3.*moments[1]*moments[4]/npts;
	a6 = moments[6] - moments[2]*moments[1]/npts - 
		2.*moments[3]*moments[0]/npts +
		2.*sqr(moments[0])*moments[1]/sqr(npts);
	a7 = moments[7] - moments[4]*moments[0]/npts -
		2.*moments[3]*moments[1]/npts +
		2.*sqr(moments[1])*moments[0]/sqr(npts);
	den = 2.*(a1*a2 - sqr(a3));
	numxc = a2*(a4 + a7) - a3*(a6 + a5);
	numyc = a1*(a6 + a5) - a3*(a4 + a7);
	if (debugging("H_extend"))
		(void) printf("numxc = %g\tnumyc = %g\tden = %g\n",
			      numxc,numyc,den);
	if (1.e-8*min(fabs(numxc),fabs(numyc)) > 1.e8*fabs(den)) 
	{
		(void) printf("WARNING in robust_H_extend_cross_trace(), ");
		(void) printf("Propagated points are colinear\n");
		(void) printf("Using robust_cross_trace()\n");
		return robust_cross_trace(rgr,op,np,ob,nb,t,pans);
	}
	COORDS(NPC)[0] = xcen = xbar + numxc/den;
	COORDS(NPC)[1] = ycen = ybar + numyc/den;
	rsqr = (moments[2] + moments[4])/npts + sqr(xcen) + sqr(ycen) -
		2.*(xbar*xcen + ybar*ycen);
	r = sqrt(rsqr);
	if (debugging("H_extend"))
		(void) printf("xcen = %g\tycen = %g\tr = %g\n",xcen,ycen,r);
	debug_print("H_extend","Left robust_H_extend_cross_trace()\n");
	return robust_circle_cross_trace(op,np,opc,&NPC,r,t,pans);
}		/*end robust_H_extend_cross_trace*/

/*
*			robust_circle_cross_trace():
*
*	Given a point moving from op to np in unit time, and a
*	expanding circle whose center moves from opc to npc
*	and radius increases from zero to radius in unit time.
*	the time of intersection is determined. YES returned if that 
*	time is in [0,1] and the time and position of the intersection
*	is determined.  Otherwise NO is returned.
*/

EXPORT int robust_circle_cross_trace(
	FT_POINT		*op,	/*Position at start of time step*/
	FT_POINT		*np,	/*Position at end of time step*/
	FT_POINT		*opc,   /*Center of circle at start of time step*/
	FT_POINT		*npc,   /*Center of circle at end of time step*/
	double		radius, /*Radius at end of time step*/
	double		*t,     /*Fractional time of intersection*/
	FT_POINT		*pans)  /*Intersection point*/
{
	double		x1 = (double)Coords(op)[0];
	double		y1 = (double)Coords(op)[1];
	double		nx1 = (double)Coords(np)[0];
	double		ny1 = (double)Coords(np)[1];
	double		x2 = (double)Coords(opc)[0];
	double		y2 = (double)Coords(opc)[1];
	double		nx2 = (npc != NULL) ? 
			    (double)Coords(npc)[0] : (double)Coords(opc)[0];
	double		ny2 = (npc != NULL) ? 
			    (double)Coords(npc)[1] : (double)Coords(opc)[1];
	double		ax,ay,bx,by;
	double		a,b,c,disc,sqrt_disc,t1,t2;
	double		num,den,tmin;

	debug_print("r_cir_cross_trace","Entered robust_circle_cross_trace()\n");
	ax = x1 - x2;		ay = y1 - y2;
	bx = nx1 - nx2 - ax;	by = ny1 - ny2 - ay;
	if (debugging("r_cir_cross_trace"))
	    (void) printf("ax = %g\tay = %g\t\nbx = %g\tby = %g\n",ax,ay,bx,by);

		/* find coefficients of quadratic eqn for t */
	
	a = sqr((double)radius) - bx*bx - by*by;
	b = -2.*(ax*bx + ay*by);
	c = -(ax*ax + ay*ay);

		/* solve for t and find cross point */

	disc = b*b - 4.*a*c;
	if (debugging("r_cir_cross_trace"))
		(void) printf("a = %g\tb = %g\tc = %g\tdisc = %g\n",a,b,c,disc);
	sqrt_disc = sqrt(fabs(disc));
#if defined(sun)
	den = fabs(a + b) + sqrt_disc;
	num = 2.*c + 0.5*a + b;
#endif /* defined(sun) */
	if (disc <= 0.0 && sqrt_disc < 0.02*fabs(a) && 
	    fabs(a + b) < 1.1*fabs(a))
			t1 = t2 = -b/(2.*a);
#if defined(sun)
	else if (1.1*fabs(a) <= den && fabs(num) < 0.55*den)
#else /* defined(sun) */
	else if (1.1*fabs(a) <= (den = fabs(a + b) + sqrt_disc) && 
	    fabs(num = 2.*c + 0.5*a + b) < 0.55*den) 
#endif /* defined(sun) */
	{
		if ((a + b) >= 0.0)
			t1 = t2 = 0.5 - num/den;
		else
			t1 = t2 = 0.5 + num/den;
	}
	else if (1.1*fabs(a) > den) 
	{
		t1 = (-b + sqrt_disc)/(2.*a);
		t2 = (-b - sqrt_disc)/(2.*a);
	}
	else 
	{
		debug_print("r_cir_cross_trace",
			"Left robust_circle_cross_trace(), Answer = NO\n");
		return NO;
	}
	
	if (debugging("r_cir_cross_trace"))
		(void) printf("t1 = %g\tt2 = %g\n",t1,t2);
	tmin = min(t1,t2);
	*t = (double) min(max(tmin,0.),1.);
	Coords(pans)[0] = (double) x1 + (*t) * (double) (nx1 - x1);
	Coords(pans)[1] = (double) y1 + (*t) * (double) (ny1 - y1);
	debug_print("r_cir_cross_trace",
	      "Left robust_circle_cross_trace(), Answer = YES\n");
	return YES;
}		/*end robust_circle_cross_trace*/

LOCAL	int robust_cross_curves_trace(
	BOND		*ob1s,
	BOND		*nb1s,
	BOND		*ob1e,
	BOND		*nb1e,
	ORIENTATION	oc1_or,
	ORIENTATION	nc1_or,
	BOND		*ob2s,
	BOND		*nb2s,
	BOND		*ob2e,
	BOND		*nb2e,
	ORIENTATION	oc2_or,
	ORIENTATION	nc2_or,
	double		*dt_frac,
	FT_POINT		*pans,
	Front		*fr)
{
	static	FT_POINT	*ptmp = NULL;

	BOND		*ob1, *nb1, *ob2, *nb2;
	FT_POINT		*op, *np;
	RECT_GRID	*rgr = fr->rect_grid;
	double		dt_tmp, dt_max;
	int		cross_found = NO;

	debug_print("curves_trace","Entered robust_cross_curves_trace()\n");
	if (ptmp == NULL) 
	{
		ptmp = Static_point(fr->interf);
	}
	dt_max = 0.;

	op = Point_of_bond(ob1s,oc1_or);
	np = Point_of_bond(nb1s,nc1_or);
	ob2 = ob2s;
	nb2 = nb2s;
	if (ob2 && nb2 && robust_cross_trace(rgr,op,np,ob2,nb2,&dt_tmp,ptmp))
	{
		if (dt_tmp > dt_max)
		{
			cross_found = YES;
			dt_max = dt_tmp;
			Coords(pans)[0] = Coords(ptmp)[0];
			Coords(pans)[1] = Coords(ptmp)[1];
		}
	}

	op = Point_of_bond(ob2s,oc2_or);
	np = Point_of_bond(nb2s,nc2_or);
	ob1 = ob1s;
	nb1 = nb1s;
	if (ob1 && nb1 && robust_cross_trace(rgr,op,np,ob1,nb1,&dt_tmp,ptmp))
	{
		if (dt_tmp > dt_max)
		{
			cross_found = YES;
			dt_max = dt_tmp;
			Coords(pans)[0] = Coords(ptmp)[0];
			Coords(pans)[1] = Coords(ptmp)[1];
		}
	}

	while (ob1 && nb1)
	{
		ob2 = ob2s;
		nb2 = nb2s;
		while (ob2 && nb2)
		{
			op = (oc1_or == POSITIVE_ORIENTATION) ?
				ob1->end : ob1->start;
			np = (nc1_or == POSITIVE_ORIENTATION) ?
				nb1->end : nb1->start;
			if (robust_cross_trace(rgr,op,np,ob2,nb2,&dt_tmp,ptmp))
			{
				if (dt_tmp > dt_max)
				{
					cross_found = YES;
					dt_max = dt_tmp;
					Coords(pans)[0] = Coords(ptmp)[0];
					Coords(pans)[1] = Coords(ptmp)[1];
				}
			}
			op = (oc2_or == POSITIVE_ORIENTATION) ?
				ob2->end : ob2->start;
			np = (nc2_or == POSITIVE_ORIENTATION) ?
				nb2->end : nb2->start;
			if (robust_cross_trace(rgr,op,np,ob1,nb1,&dt_tmp,ptmp))
			{
				if (dt_tmp > dt_max)
				{
					cross_found = YES;
					dt_max = dt_tmp;
					Coords(pans)[0] = Coords(ptmp)[0];
					Coords(pans)[1] = Coords(ptmp)[1];
				}
			}
			if (ob2 == ob2e || nb2 == nb2e) break;
			ob2 = Following_bond(ob2,oc2_or);
			nb2 = Following_bond(nb2,nc2_or);
		}
		if (ob1 == ob1e || nb1 == nb1e) break;
		ob1 = Following_bond(ob1,oc1_or);
		nb1 = Following_bond(nb1,nc1_or);
	}
	*dt_frac = min(dt_max,*dt_frac);

	debug_print("curves_trace",
		"Left robust_cross_curves_trace() cross_found = %s\n",
		(cross_found == YES) ? "YES" : "NO");

	return cross_found;
}		/*end robust_cross_curves_trace*/

EXPORT	void set_prop_status_for_pseudo_cross_node(
	O_CURVE		*oldc1,
	O_CURVE		*newc1,
	O_CURVE		*oldc2,
	O_CURVE		*newc2,
	Front		*fr,
	POINTER		wave,
	double		dt,
	NODE_FLAG	flag)
{
	BOND		B1, B2;
	BOND		*b1virtual = &B1, *b2virtual = &B2;
	BOND		*oppb1,*oppb2;
	NODE		*oppn1, *oppn2;
	double		v1[MAXD], v2[MAXD];
	int		dim = oldc1->curve->interface->dim;
	static	FT_POINT   *p1 = NULL, *p2 = NULL, *p1_opp = NULL, *p2_opp = NULL;

	if (p2_opp == NULL) 
	{
	    p1 = Static_point(fr->interf);
	    p2 = Static_point(fr->interf);
	    p1_opp = Static_point(fr->interf);
	    p2_opp = Static_point(fr->interf);
	}
	init_curve_for_crossing(p1,p1_opp,b1virtual,oldc1,newc1,
		                &oppn1,&oppb1,fr,wave,dt,v1,flag);
	init_curve_for_crossing(p2,p2_opp,b2virtual,oldc2,newc2,
		                &oppn2,&oppb2,fr,wave,dt,v2,flag);

	set_vel_of_crossing_node(Bond_at_node_of_o_curve(oldc1),
		                 Bond_at_node_of_o_curve(oldc2),b1virtual,
				 b2virtual,wave_type(newc1->curve),
				 wave_type(newc2->curve),
				 Node_of_o_curve(oldc2),Node_of_o_curve(newc2),
				 dt,fr);

	if (newc2->orient == POSITIVE_ORIENTATION) 
	{
	    oppb2->end = oppn2->posn;
	}
	else 
	{
	    oppb2->start = oppn2->posn;
	}
	set_bond_length(oppb2,dim);
	if (newc1->orient == POSITIVE_ORIENTATION) 
	{
	    oppb1->end = oppn1->posn;
	}
	else 
	{
	    oppb1->start = oppn1->posn;
	}
	set_bond_length(oppb1,dim);
}		/*end set_prop_status_for_pseudo_cross_node*/


LOCAL	int unpropagated_nodes_remain_on_interface(
	NODE		*n1,
	NODE		*n2,
	INTERFACE	*intfc)
{
	NODE		**n;

	for (n = intfc->nodes; n && *n; n++)
	{
	    if (*n == n1 || *n == n2)
		continue;
	    if (propagation_status(*n) == UNPROPAGATED_NODE)
		return YES;
	}
	return NO;
}		/*end unpropagated_nodes_remain_on_interface*/
#endif /* defined(TWOD) */
