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
*				fnode.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Contains physics-independent routines for the propagation
*	of nodes:
*
*			fixed_node_propagate()
*			closed_node_propagate()
*			B_node_propagate()
*/


#include <front/fdecs.h>

	/* LOCAL Function Declarations */
#if defined(DEBUG_NODE_PROPAGATE)
LOCAL	void	debug_print_B_node_curves(const char*,
					  O_CURVE*,O_CURVE*,O_CURVE*,
					  O_CURVE*,O_CURVE*);
#endif /* defined(DEBUG_NODE_PROPAGATE) */
LOCAL	void copy_state_to_node(Locstate,COMPONENT,NODE*,size_t);
LOCAL	boolean close_to_subdomain_bdry(O_CURVE,O_CURVE,O_CURVE);
LOCAL   int  free_end_node_propagate(Front*,POINTER,NODE*,NODE*,double);

/*   
*		fixed_node_propagate():
*
*	Updates the states at a fixed node.
*
*	Allows for physical curve attachments to a fixed node.
*	Performs a point_propagate at the appropriate end of each
*	attached curve, which can lead to neighboring curve
*	start_ or end_ states being different. This routine then ensures
*	that such neighboring states are set equal. (This is important
*	for such algorithms as the hyp tri_grid construction). The setting
*	of equal states is done as follows:
*
*	If:	 the fixed node has boundary states that are non-zero Neumann,
*		 the injection state is used for all relevent curve states
*		 at the node.
*
*	else if: a curve has physical wave type, its state values alone are
*		 used.
*
*	else if: neither of two neighboring curves has physical wave type,
*		 their state values are averaged.
*	
*/

EXPORT	int fixed_node_propagate(
	Front		*fr,
	POINTER		wave,
	NODE		*oldn,
	NODE		*newn,
	double		dt)
{
	INTERFACE	*n_intfc;
	O_NODE		*o_ond;
	CURVE		*oldc, *newc;
	BOND		*oldb;
	FT_POINT		*o_posn, *n_posn;
	Locstate	lst, rst;
	double		V[MAXD];
	int		i, dim = fr->rect_grid->dim;
	ORIENTATION	oc_or;
	size_t		sizest;
	DEBUG_ENTER(fixed_node_propagate)

	debug_print("fixed_node","Entered fixed_node_propagate()\n");

	n_intfc = newn->interface;	sizest = fr->sizest;
	o_posn = oldn->posn;		n_posn = newn->posn;

	o_ond = make_onode(oldn);

#if defined(DEBUG_NODE_PROPAGATE)
	if (debugging("fixed_node"))
	    print_onode(o_ond);
#endif /* defined(DEBUG_NODE_PROPAGATE) */

	for (i = 0;  i < o_ond->num_c;  ++i)
	{
	    oldc = o_ond->nc[i];
	    oc_or = o_ond->orient[i];

	    if (wave_type(oldc) == PASSIVE_BOUNDARY)
		continue;

	    newc = (oc_or == POSITIVE_ORIENTATION) ?
	    find_correspond_curve(oldc,newn,(NODE *)NULL,fr,n_intfc) :
	    find_correspond_curve(oldc,(NODE *)NULL,newn,fr,n_intfc);

#if defined(DEBUG_NODE_PROPAGATE)
	    if (debugging("fixed_node"))
	    {
	    	lst =  Left_state_at_node(oldc,oc_or);
	    	rst = Right_state_at_node(oldc,oc_or);
	    	if (wave_type(oldc) >= FIRST_PHYSICS_WAVE_TYPE)
	    	    (void) printf("oldc %llu physical\n",curve_number(oldc));
	    	else
	    	    (void) printf("oldc %llu\n",curve_number(oldc));
	    	(void) printf("\t left - ");	(*fr->print_state)(lst);
	    	(void) printf("\tright - ");	(*fr->print_state)(rst);
	    }
#endif /* defined(DEBUG_NODE_PROPAGATE) */

	    oldb = Bond_at_node(oldc,oc_or);
	    o_posn->hse = Hyper_surf_element(oldb);
	    o_posn->hs = Hyper_surf(oldc);
	    point_propagate(fr,wave,o_posn,n_posn,oldb,oldc,dt,V);
	    Coords(n_posn)[0] = Coords(o_posn)[0];
	    Coords(n_posn)[1] = Coords(o_posn)[1];

	    lst =  Left_state_at_node(newc,oc_or);
	    rst = Right_state_at_node(newc,oc_or);
	    ft_assign(lst, left_state(n_posn),sizest);
	    ft_assign(rst,right_state(n_posn),sizest);

#if defined(DEBUG_NODE_PROPAGATE)
	    if (debugging("fixed_node"))
	    {
	    	if (wave_type(newc) >= FIRST_PHYSICS_WAVE_TYPE)
	    	    (void) printf("newc %llu physical\n",curve_number(newc));
	    	else
	    	    (void) printf("newc %llu\n",curve_number(newc));
	    	(void) printf("\t left - ");	(*fr->print_state)(lst);
	    	(void) printf("\tright - ");	(*fr->print_state)(rst);
	    	(void) printf("\n");
	    }
#endif /* defined(DEBUG_NODE_PROPAGATE) */
	}

		/* Correct neigboring states at node */

	reset_fixed_node_states(newn,fr);

	for (i = 0; i < dim; ++i)
	    Node_vel(newn)[i] = 0.0;
	propagation_status(newn) = PROPAGATED_NODE;

	debug_print("fixed_node","Left fixed_node_propagate()\n");
	DEBUG_LEAVE(fixed_node_propagate)
	return GOOD_NODE;
}		/*end fixed_node_propagate*/


#define IS_nzn_bdry(fr,x,y,comp,ijc) \
	((fr->is_nzn_bdry) && (*fr->is_nzn_bdry)(x,y,comp,ijc))

EXPORT	void reset_fixed_node_states(
	NODE		*newn,
	Front		*fr)
{
	static	Locstate tmpst = NULL;
	static	size_t	 sizest = 0;

	COMPONENT	comp;
	O_NODE		*n_ond;
	CURVE		*ic, *jc;
	Locstate	ist, jst;
	int		i, j, num_c;
	ORIENTATION	ic_or, jc_or;
	int		wtj, wti;
	double		x, y;
	DEBUG_ENTER(reset_fixed_node_states)

	if (tmpst == NULL)
	{
	    sizest = fr->sizest;
	    alloc_state(fr->interf,&tmpst,sizest);
	}
	if (sizest == 0) return;

	n_ond = make_onode(newn);
	num_c = n_ond->num_c;

	for (j = 0;  j < num_c;  ++j)
	{
	    jc = n_ond->nc[j];
	    jc_or = n_ond->orient[j];

	    comp = (jc_or == POSITIVE_ORIENTATION) ? positive_component(jc)
	    				           : negative_component(jc);

	    if (is_excluded_comp(comp,jc->interface))
		continue;


	    i = (j == 0) ? num_c - 1 : j - 1;
	    ic = n_ond->nc[i];		ic_or = n_ond->orient[i];

	    wtj = wave_type(jc);	wti = wave_type(ic);
	    if (wtj == SUBDOMAIN_BOUNDARY || wti == SUBDOMAIN_BOUNDARY)
	    	continue;

	    if ((wtj == PASSIVE_BOUNDARY) || (wti == PASSIVE_BOUNDARY))
	    	continue;

	    jst = (jc_or == POSITIVE_ORIENTATION) ? right_start_state(jc)
						  : left_end_state(jc);
	    ist = (ic_or == POSITIVE_ORIENTATION) ? left_start_state(ic)
	    				          : right_end_state(ic);

	    x = Coords(newn->posn)[0];	y = Coords(newn->posn)[1];
	    if ((wtj == NEUMANN_BOUNDARY) && IS_nzn_bdry(fr,x,y,comp,jc))
	    {
	    	ft_assign(ist,jst,sizest);
	    }
	    else if ((wti == NEUMANN_BOUNDARY) && IS_nzn_bdry(fr,x,y,comp,ic))
	    {
	    	ft_assign(jst,ist,sizest);
	    }

	    /* Feb 28 2003: Wonho: Corner point correction
	     * will enforce Dirichlet boundary condition when
	     * two conditions are competing at the corner
	     */
	    else if ((wti == NEUMANN_BOUNDARY) && wtj == DIRICHLET_BOUNDARY )
	    {
	    	ft_assign(ist,jst,sizest);
	    }
	    else if ((wtj == NEUMANN_BOUNDARY) && wti == DIRICHLET_BOUNDARY )
	    {
	    	ft_assign(jst,ist,sizest);
	    }

	    else if ((wave_type(jc) >= FIRST_PHYSICS_WAVE_TYPE) &&
	    	     (wave_type(ic) <  FIRST_PHYSICS_WAVE_TYPE))
	    {
	    	ft_assign(ist,jst,sizest);
	    }
	    else if ((wave_type(ic) >= FIRST_PHYSICS_WAVE_TYPE) &&
		     (wave_type(jc) <  FIRST_PHYSICS_WAVE_TYPE))
	    {
	    	ft_assign(jst,ist,sizest);
	    }
	    else
	    {
	    	interpolate_states(fr,0.5,0.5,Coords(newn->posn),ist,
				   Coords(newn->posn),jst,tmpst);
		ft_assign(ist,tmpst,sizest);
		ft_assign(jst,tmpst,sizest);
	    }
	}
	DEBUG_LEAVE(reset_fixed_node_states)
}		/*end reset_fixed_node_states*/


/*   
*			closed_node_propagate():
*
*	Propagates the node of a closed curve.
*/

EXPORT int closed_node_propagate(
	Front		*fr,
	POINTER		wave,
	NODE		*oldn,
	NODE		*newn,
	double		dt)
{
	CURVE		*oldc, *newc;
	FT_POINT		*np;
	Locstate	lst, rst;
	double		V[MAXD];
	size_t 		szst = fr->sizest;
	int		i, dim = fr->interf->dim;

	if (propagation_status(newn) == PROPAGATED_NODE)
	    return GOOD_NODE;
	debug_print("closed_node","Entered closed_node_propagate()\n");
#if defined(DEBUG_NODE_PROPAGATE)
	if (debugging("closed_node")) 
	{
	    (void) printf("\n\tOLD NODE:\n");
	    print_node(oldn);
	}
#endif /* defined(DEBUG_NODE_PROPAGATE) */

	oldc = oldn->out_curves[0];
#if defined(DEBUG_NODE_PROPAGATE)
	if (debugging("closed_node")) 
	{
	    (void) printf("\t\tOLD CURVE:\n");
	    print_curve(oldc);
	}
#endif /* defined(DEBUG_NODE_PROPAGATE) */

	np = newn->posn;
	oldn->posn->hse = Hyper_surf_element(oldc->first);
	oldn->posn->hs = Hyper_surf(oldc);
	point_propagate(fr,wave,oldn->posn,np,oldc->first,oldc,dt,V);

	newc = find_correspond_curve(oldc,newn,newn,fr,newn->interface);

	lst = left_state(np);		rst = right_state(np);
	ft_assign( left_start_state(newc),lst,szst);
	ft_assign(right_start_state(newc),rst,szst);
	ft_assign(   left_end_state(newc),lst,szst);
	ft_assign(  right_end_state(newc),rst,szst);

	set_bond_length(newc->first,dim);
	set_bond_length(newc->last,dim);

	propagation_status(newn) = PROPAGATED_NODE;

	    /* WARNING: Node_vel of closed nodes is undefined */
	for (i = 0; i < dim; ++i)
	    Node_vel(newn)[i] = ERROR_FLOAT;

#if defined(DEBUG_NODE_PROPAGATE)
	if (debugging("closed_node")) 
	{
	    (void) printf("\n\tNEW NODE:\n");
	    print_node(newn);
	    (void) printf("\t\tNEW CURVE:\n");
	    print_curve(newc);
	}
#endif /* defined(DEBUG_NODE_PROPAGATE) */
	debug_print("closed_node","Left closed_node_propagate()\n");
	return GOOD_NODE;
}		/*end closed_node_propagate*/

/*
*                       free_end_node_propagate():
*
*       Propagates the free end node of a curve.
*/

LOCAL int free_end_node_propagate(
        Front           *fr,
        POINTER         wave,
        NODE            *oldn,
        NODE            *newn,
        double           dt)
{
        BOND            *oldb;
        CURVE           *oldc, *newc;
        FT_POINT           *np;
        Locstate        lst, rst;
        double           V[MAXD];
        size_t          szst = fr->sizest;
        int             i, dim = fr->interf->dim;

        debug_print("free_end_node","Entered free_end_node_propagate()\n");
#if defined(DEBUG_NODE_PROPAGATE)
        if (debugging("free_end_node"))
        {
            (void) printf("\n\tOLD NODE:\n");
            print_node(oldn);
        }
#endif /* defined(DEBUG_NODE_PROPAGATE) */

        oldc = (oldn->out_curves != NULL) ? oldn->out_curves[0] :
                oldn->in_curves[0];

        np = newn->posn;
        oldb = (oldn == oldc->start) ? oldc->first : oldc->last;
        oldn->posn->hse = Hyper_surf_element(oldb);
        oldn->posn->hs = Hyper_surf(oldc);
        point_propagate(fr,wave,oldn->posn,np,oldb,oldc,dt,V);

        newc = find_correspond_curve(oldc,newn,newn,fr,newn->interface);

        lst = left_state(np);           rst = right_state(np);
        if (oldn == oldc->start)
        {
            ft_assign(left_start_state(newc),lst,szst);
            ft_assign(right_start_state(newc),rst,szst);

            set_bond_length(newc->first,dim);
        }
        else
        {
            ft_assign(left_end_state(newc),lst,szst);
            ft_assign(right_end_state(newc),rst,szst);
            set_bond_length(newc->last,dim);
        }

        propagation_status(newn) = PROPAGATED_NODE;

            /* WARNING: Node_vel of closed nodes is undefined */
        for (i = 0; i < dim; ++i)
            Node_vel(newn)[i] = ERROR_FLOAT;

#if defined(DEBUG_NODE_PROPAGATE)
        if (debugging("free_end_node"))
        {
            (void) printf("\n\tNEW NODE:\n");
            print_node(newn);
        }
#endif /* defined(DEBUG_NODE_PROPAGATE) */
        debug_print("free_end_node","Left free_end_node_propagate()\n");
        return GOOD_NODE;
}               /*end closed_node_propagate*/

/*                  
*			B_node_propagate():
*
*	A B_node is a node at which three or more curves meet, with one
*	having a physical wave type, two of the others having boundary
*	wave types, not passive, and all others being passive curves.
*
*	The physical curve is propagated, defining an ahead component,
*	hence an ahead and behind boundary, and a component into which
*	the node propagates.  If the node propagates into the ahead
*	component, the physical curve is extended to meet the ahead
*	boundary using extend_crossing_of_two_propagated_curves();
*	otherwise the intersection of the propagated physical curve
*	is found with the propagated ahead boundary.  Then the ahead
*	and behind boundaries are updated by calling shift_node(),
*	and the physical curve is updated using cut_curve().
*
*/

EXPORT int B_node_propagate(
	Front		*fr,
	POINTER		wave,
	NODE		*oldn,
	NODE		*newn,
	RPROBLEM	**rp,
	double		dt,
	double		*dt_frac,
	NODE_FLAG	flag)
{
	BOND		*crossbphys;		/* intersecting two bonds */
	BOND		*crossbahead;		/* on newcphys, newcahead */
	COMPONENT       propagation_comp;	/* comp containing newn */
	COMPONENT	ahead_comp;             /* comp on ahead side of
						   cphys wrt motion */
	CURVE		*ca;
	O_CURVE		Oldcphys, Newcphys;	/* the physical curve */
	O_CURVE		Oldcahead;              /* ahead bdry wrt oldn */
	O_CURVE		Newcahead;              /* crspd of Oldcahead */
	O_CURVE		Oldcbehind;             /* behind bdry wrt oldn */
	O_CURVE		Newcbehind;             /* crspd of Oldcbehind */
	O_CURVE		Newcaprop;              /* ahead bdry wrt newn */
	O_CURVE		Oldcaprop;              /* crspd of Newcaprop */
	O_CURVE		Newcbprop;              /* behind bdry wrt newn */
	O_CURVE		Oldcbprop;              /* crspd of Newcbprop */
	double		tcr_phys,tcr_ahead;	/* fractional dist to cross */
	int		status;			/* diagnostic return value */
	ANGLE_DIRECTION	i_to_prop_dir;          /* ang dir - inc to forward
						   wrt dir of motion */
	SIDE		propagation_side;       /* side of cahead defined by
						   newn posn*/
	SIDE		inc_side;               /* side of cahead defined by
						   cphys */
	static FT_POINT	*pc = NULL;		/* crossing point */
	static FT_POINT	*newp = NULL;		/* new node propagation point */
	boolean opposite_dir_tried = NO;
	i_to_prop_dir = ANGLE_DIRECTION_NOT_SET;
	
	debug_print("B_node","Entered B_node_propagate()\n");
#if defined(DEBUG_NODE_PROPAGATE)
	if (debugging("B_node")) 
	{
	    (void) printf("\n\tOLD NODE:\n");	print_node(oldn);
	}
#endif /* defined(DEBUG_NODE_PROPAGATE) */
	if (pc == NULL)
	{
	    pc = Static_point(fr->interf);
	    newp = Static_point(fr->interf);
	}
	zero_scalar(&Oldcphys,sizeof(O_CURVE));
	zero_scalar(&Newcphys,sizeof(O_CURVE));
	zero_scalar(&Oldcahead,sizeof(O_CURVE));
	zero_scalar(&Newcahead,sizeof(O_CURVE));
	zero_scalar(&Oldcbehind,sizeof(O_CURVE));
	zero_scalar(&Newcbehind,sizeof(O_CURVE));
	zero_scalar(&Newcaprop,sizeof(O_CURVE));
	zero_scalar(&Oldcaprop,sizeof(O_CURVE));
	zero_scalar(&Newcbprop,sizeof(O_CURVE));
	zero_scalar(&Oldcbprop,sizeof(O_CURVE));
	Oldcphys.prev = Newcphys.prev = Oldcphys.next = Newcphys.next = NULL;
	Oldcahead.prev = Newcahead.prev = NULL;
	Oldcahead.next = Newcahead.next = NULL;
	Oldcbehind.prev = Newcbehind.prev = NULL;
	Oldcbehind.next = Newcbehind.next = NULL;

	Oldcphys.curve = find_physical_curve_at_node(oldn,&Oldcphys.orient);
	if (!find_correspond_of_oriented_curve(&Oldcphys,&Newcphys,
					      newn,fr,newn->interface))
	{
	    screen("ERROR in B_node_propagate(), "
	           "find_correspond_of_oriented_curve() failed\n");
	    clean_up(ERROR);
	}
	
                /* Identify curves and components */

find_prop_orient:
	find_propagation_orientation(fr,wave,oldn,newn,newp,&Oldcphys,dt,
				     &i_to_prop_dir,&Oldcahead,&Newcahead,
				     &Oldcbehind,&Newcbehind,&inc_side,
				     &propagation_side,&ahead_comp,
				     &propagation_comp);

	/* Added by Lingling Wu, the node near subdomain
	 * boundary with only one bond on physical curve
	 * need not to be propagated, it adds complexity 
	 * and the result will be thrown away anyway.
	 */
	if (close_to_subdomain_bdry(Oldcphys,Oldcahead,Oldcbehind))
	{
	    propagation_status(newn) = PROPAGATED_NODE;
	    if (fr->sizest)
	    {
	        Locstate sl,sr;
	        sl = (Oldcphys.orient == POSITIVE_ORIENTATION) ?
		        	left_start_state(Oldcphys.curve) : 
		        	left_end_state(Oldcphys.curve);
	        sr = (Oldcphys.orient == POSITIVE_ORIENTATION) ?
		        	right_start_state(Oldcphys.curve) : 
		        	right_end_state(Oldcphys.curve);
	        copy_state_to_node(sl,negative_component(Oldcphys.curve),
				    newn,fr->sizest);
	        copy_state_to_node(sr,positive_component(Oldcphys.curve),
				    newn,fr->sizest);
	    }
	    return GOOD_NODE;
	}
	
	copy_o_curve(&Newcaprop,&Newcahead);
	copy_o_curve(&Oldcaprop,&Oldcahead);
	copy_o_curve(&Newcbprop,&Newcbehind);
	copy_o_curve(&Oldcbprop,&Oldcbehind);

	if (
	    (continue_past_fixed_node(flag) == YES) && 
	    ( 
	        is_fixed_node(Opp_node_of_o_curve(&Newcahead)) ||
		fixed_type_node(Opp_node_of_o_curve(&Newcahead))
	    )
	   )
	{
	    if (debugging("B_node"))
	        (void) printf("Continuing past fixed node\n");
	    if (!next_boundary(Newcahead.curve,
	                       Opposite_orient(Newcahead.orient),
	    	              &Newcaprop.curve,&Newcaprop.orient))
	    {
	        screen("ERROR in B_node_propagate(), "
	               "next_boundary() failed\n");
	        clean_up(ERROR);
	    }

	    do
	    {
	    	if (!next_boundary(Oldcahead.curve,
				   Opposite_orient(Oldcahead.orient),
	    		           &Oldcaprop.curve,&Oldcaprop.orient))
		{
	            screen("ERROR in B_node_propagate(), "
	                   "next_boundary() failed\n");
	            clean_up(ERROR);
		}
	    	if (Oldcahead.orient != Oldcaprop.orient)
		{
	    	    inc_side = Opposite_side(inc_side);
		}
		propagation_side = find_propagation_side(&Oldcaprop,newp,
		                                         inc_side,fr);
	    }
	    while (!is_fixed_node(Node_of_o_curve(&Oldcaprop)) &&
	           !fixed_type_node(Node_of_o_curve(&Oldcaprop)));

	    copy_o_curve(&Newcahead,&Newcbehind);
	}

fixed_node_loop:
#if defined(DEBUG_NODE_PROPAGATE)
	if (debugging("B_node")) 
	{
	    (void) printf("inc_side = %s, propagation_side = %s\n",
	                  side_name(inc_side),side_name(propagation_side));
	    debug_print_B_node_curves("OLD ",&Oldcphys,
				      &Oldcahead,&Oldcaprop,
				      &Oldcbehind,&Oldcbprop);
	    debug_print_B_node_curves("NEW ",&Newcphys,
	    			      &Newcahead,&Newcaprop,
				      &Newcbehind,&Newcbprop);

	    if (debugging("node_states"))
	    {
	    	Locstate sl, sr;

	    	(void) printf("STATES AT OLD NODE\n\n");
	    	(void) printf("\t\tOLD PHYSICAL NODE STATES:\n");
	    	slsr(Node_of_o_curve(&Oldcphys)->posn,
	    	     Hyper_surf_element(Bond_at_node_of_o_curve(&Oldcphys)),
		     Hyper_surf(Oldcphys.curve),&sl,&sr);
		(void) printf("left state at node ");  (*fr->print_state)(sl);
		(void) printf("right state at node "); (*fr->print_state)(sr);

		(void) printf("\t\tOLD AHEAD NODE STATES:\n");
		slsr(Node_of_o_curve(&Oldcahead)->posn,
		     Hyper_surf_element(Bond_at_node_of_o_curve(&Oldcahead)),
		     Hyper_surf(Oldcahead.curve),&sl,&sr);
		(void) printf("left state at node ");  (*fr->print_state)(sl);
		(void) printf("right state at node "); (*fr->print_state)(sr);

		(void) printf("\t\tOLD BEHIND NODE STATES:\n");
		slsr(Node_of_o_curve(&Oldcbehind)->posn,
		     Hyper_surf_element(Bond_at_node_of_o_curve(&Oldcbehind)),
		     Hyper_surf(Oldcbehind.curve),&sl,&sr);
		(void) printf("left state at node ");  (*fr->print_state)(sl);
		(void) printf("right state at node "); (*fr->print_state)(sr);

		(void) printf("\nEND STATES AT OLD NODE\n\n");
	     }
	}
#endif /* defined(DEBUG_NODE_PROPAGATE) */

	     /* Identify new position of node */

	if (inc_side == propagation_side)
	{
	    if ((wave_type(Oldcaprop.curve) == NEUMANN_BOUNDARY)
	       				&&
		(wave_type(Oldcphys.curve) >= FIRST_VECTOR_PHYSICS_WAVE_TYPE))
	    {

#if defined(DEBUG_NODE_PROPAGATE)
	        if (debugging("B_node"))
	            (void) printf("calling "
		              "H_extend_crossing_of_two_propagated_curves()\n");
#endif /* defined(DEBUG_NODE_PROPAGATE) */

		status = H_extend_crossing_of_two_propagated_curves(
			    &Oldcaprop,&Newcaprop,&Oldcphys,&Newcphys,
			    ahead_comp,propagation_comp,pc,
			    &crossbahead,&crossbphys,&tcr_ahead,
			    &tcr_phys,fr,wave,rp,dt,dt_frac,flag);
	        if (status == ERROR_NODE)
	        {
		    (void) printf("WARNING in B_node_propagate(), "
		        "H_extend_crossing_of_two_propagated_curves()"
			", returns ERROR_NODE\n");
	        }
	    }
	    else
	    {
#if defined(DEBUG_NODE_PROPAGATE)
	        if (debugging("B_node"))
	            (void) printf("calling "
		              "D_extend_crossing_of_two_propagated_curves()\n");
#endif /* defined(DEBUG_NODE_PROPAGATE) */

		ca = Newcaprop.curve;
		status = D_extend_crossing_of_two_propagated_curves(
			    &Oldcaprop,&Newcaprop,&Oldcbehind,&Newcbehind,
			    &Oldcphys,&Newcphys,ahead_comp,propagation_comp,
			    pc,&crossbahead,&crossbphys,&tcr_ahead,
			    &tcr_phys,fr,wave,rp,dt,dt_frac,flag);
		if (ca != Newcaprop.curve)
		{
		    /* Propagation direction reversed*/
		    i_to_prop_dir = Opposite_ang_dir(i_to_prop_dir);
		    inc_side = (curve_ang_oriented_l_to_r(
			i_to_prop_dir,Oldcaprop.orient)) ?
			    NEGATIVE_SIDE : POSITIVE_SIDE;
		}
	        if (status == ERROR_NODE)
	        {
		    (void) printf("WARNING in B_node_propagate(), "
		                  "D_extend_crossing_of_two_propagated_curves()"
			          ", returns ERROR_NODE\n");
	        }
	    }
	}
	else
	{
#if defined(DEBUG_NODE_PROPAGATE)
	    if (debugging("B_node"))
	    	(void) printf("calling crossing_of_two_propagated_curves()\n");
#endif /* defined(DEBUG_NODE_PROPAGATE) */

	    status = crossing_of_two_propagated_curves(
			&Oldcphys,&Newcphys,&Oldcaprop,&Newcaprop,
			pc,&crossbphys,&crossbahead,&tcr_phys,
			&tcr_ahead,fr,wave,rp,dt,dt_frac,flag);
	
	    if (status == ERROR_NODE)
	    {
		(void) printf("WARNING in B_node_propagate(), "
			      "crossing_of_two_propagated_curves(), returns "
			      "ERROR_NODE\n");
	    }
	}
	
	/* Check for valid cross */

	if (status == GOOD_NODE)
	{
	    INTERFACE	*intfc = oldn->interface;
	    COMPONENT	comp;
	    double		*crds;

	    crds = Coords(Point_of_bond(crossbphys,
			  Opposite_orient(Newcphys.orient)));
	    comp = component(crds,intfc);
	    if (is_excluded_comp(comp,intfc))
	    {
	    	void	(*save_impose_bc)(FT_POINT*,BOND*,CURVE*,double*,Front*,
					  boolean,boolean);

#if defined(DEBUG_NODE_PROPAGATE)
		if (debugging("B_node"))
		    (void) printf("invalid cross found, calling %s\n",
				  "crossing_of_two_propagated_curves()");
#endif /* defined(DEBUG_NODE_PROPAGATE) */

		save_impose_bc = fr->impose_bc;
		fr->impose_bc = NULL;
		status = crossing_of_two_propagated_curves(
				&Oldcphys,&Newcphys,&Oldcaprop,&Newcaprop,
				pc,&crossbphys,&crossbahead,&tcr_phys,
				&tcr_ahead,fr,wave,rp,dt,dt_frac,flag);
		fr->impose_bc = save_impose_bc;
	    }
	}

	if (
	    (status == NO_CROSS_NODE) &&
	    (continue_past_fixed_node(flag) == YES) &&
	    ( 
	        is_fixed_node(Opp_node_of_o_curve(&Oldcaprop)) ||
	        fixed_type_node(Opp_node_of_o_curve(&Newcahead))
	    )
	   )
	{
	    ORIENTATION	tmp_orient, tmp_or;
	    NODE		*tmpn;

	    tmpn = Opp_node_of_o_curve(&Oldcaprop);
	    if (!next_boundary(Oldcaprop.curve,
	                       Opposite_orient(Oldcaprop.orient),
			       &Oldcaprop.curve,&tmp_orient))
	    {
	        screen("ERROR in B_node_propagate(), next_boundary() failed\n");
	        clean_up(ERROR);
	    }
	    if (Oldcaprop.orient != tmp_orient) 
	    {
		inc_side = Opposite_side(inc_side);
	    }
	    Oldcaprop.orient =  tmp_orient;
	    propagation_side = find_propagation_side(&Oldcaprop,newp,
	                                             inc_side,fr);
	    if (!find_correspond_of_oriented_curve(&Oldcaprop,&Newcaprop,newn,
						   fr,newn->interface))
	    {
	        screen("ERROR in B_node_propagate(), "
		       "find_correspond_of_oriented_curve() failed\n");
	        clean_up(ERROR);
	    }

		/* if second fixed_node has phys curve attached */
		/* 	force a reduction in timestep.		*/
		/* Is this code still valid for reservoir ?	*/

	    if (find_physical_curve_at_node(tmpn,&tmp_or) != NULL)
	    {
	        (void) printf("B_node: next_fixed_node has phys curves\n");
	        (void) printf("        forcing timestep reduction\n");
	        status = MODIFY_TIME_STEP;
	        debug_print("B_node","Left B_node_propagate(), ");
	        if (debugging("B_node"))
	    	    print_node_status("status = ",status,"\n");
		return status;
	    }
	    (void) printf("B_node prop: next fixed_node also skipped\n");
	    goto fixed_node_loop;
	}

	if (status != GOOD_NODE) 
	{	
	    if (all_pts_on_c_are_ext_to_rect(Newcphys.curve,fr->rect_grid) ||
	    	Newcphys.curve->num_points == 2) 
		/* Comment: a curve with only two points and no crossing
		 * must be exiting the boundary of domain. Xiaolin Li */
	    {
	        propagation_status(newn) = PROPAGATED_NODE;
		if (fr->sizest)
		{
	            Locstate sl,sr;
		    if (Newcphys.curve->num_points > 2)
		    {
	            	sl = (Newcphys.orient == POSITIVE_ORIENTATION) ?
		        	left_state(Newcphys.curve->first->end) : 
		        	left_state(Newcphys.curve->last->start);
	            	sr = (Newcphys.orient == POSITIVE_ORIENTATION) ?
		        	right_state(Newcphys.curve->first->end) : 
		        	right_state(Newcphys.curve->last->start);
		    }
		    else
		    {
	            	sl = (Newcphys.orient == POSITIVE_ORIENTATION) ?
		        	left_start_state(Oldcphys.curve) : 
		        	left_end_state(Oldcphys.curve);
	            	sr = (Newcphys.orient == POSITIVE_ORIENTATION) ?
		        	right_start_state(Oldcphys.curve) : 
		        	right_end_state(Oldcphys.curve);
		    }
	            copy_state_to_node(sl,negative_component(Newcphys.curve),
				    newn,fr->sizest);
	            copy_state_to_node(sr,positive_component(Newcphys.curve),
				    newn,fr->sizest);
		}
#if defined(DEBUG_NODE_PROPAGATE)
	        if (debugging("B_node"))
	            print_curve(Newcphys.curve);
#endif /* defined(DEBUG_NODE_PROPAGATE) */
	        return GOOD_NODE;
	    }
	    debug_print("B_node","Left B_node_propagate(), ");
	    if (debugging("B_node"))
		print_node_status("status = ",status,"\n");
	    if (!opposite_dir_tried)
	    {
	    	opposite_dir_tried = YES;
		goto find_prop_orient;
	    }
	    return status;
	}
	

	    /* Modify the interface and assign the new states */

#if defined(DEBUG_NODE_PROPAGATE)
	if (debugging("B_node")) 
	{
	    (void) printf("New curves after crossing functions\n\n");
	    debug_print_B_node_curves("NEW ",&Newcphys,&Newcaprop,NULL,
	                              &Newcbehind,NULL);
	}
#endif /* defined(DEBUG_NODE_PROPAGATE) */

	status = modify_B_node(oldn,newn,&Oldcphys,&Newcphys,&Oldcahead,
			       &Newcahead,&Oldcaprop,&Newcaprop,&Oldcbehind,
			       &Newcbehind,&Oldcbprop,&Newcbprop,pc,
		               crossbphys,crossbahead,i_to_prop_dir,tcr_phys,
		               tcr_ahead,rp,fr,wave,dt,dt_frac,flag);

#if defined(DEBUG_NODE_PROPAGATE)
	if (debugging("B_node"))
	{
	    (void) printf("New node and new curves after B_node_propagate()\n");
	    (void) printf("\n\nNEW NODE:\n");	print_node(newn);
	    debug_print_B_node_curves("NEW ",&Newcphys,&Newcahead,NULL,
	                              &Newcbehind,NULL);

	    if (debugging("node_states"))
	    {
	    	Locstate sl, sr;

	    	(void) printf("STATES AT NEW NODE\n\n");
	    	(void) printf("\t\tNEW PHYSICAL NODE STATES:\n");
	    	slsr(Node_of_o_curve(&Newcphys)->posn,
	    	     Hyper_surf_element(Bond_at_node_of_o_curve(&Newcphys)),
	    	     Hyper_surf(Newcphys.curve),&sl,&sr);
	    	(void) printf("left state at node ");  (*fr->print_state)(sl);
	    	(void) printf("right state at node "); (*fr->print_state)(sr);

	    	(void) printf("\t\tNEW AHEAD NODE STATES:\n");
	    	slsr(Node_of_o_curve(&Newcahead)->posn,
	    	     Hyper_surf_element(Bond_at_node_of_o_curve(&Newcahead)),
	    	     Hyper_surf(Newcahead.curve),&sl,&sr);
	    	(void) printf("left state at node ");  (*fr->print_state)(sl);
	    	(void) printf("right state at node "); (*fr->print_state)(sr);

	    	(void) printf("\t\tNEW BEHIND NODE STATES:\n");
	    	slsr(Node_of_o_curve(&Newcbehind)->posn,
	    	     Hyper_surf_element(Bond_at_node_of_o_curve(&Newcbehind)),
	    	     Hyper_surf(Newcbehind.curve),&sl,&sr);
	    	(void) printf("left state at node ");  (*fr->print_state)(sl);
	    	(void) printf("right state at node "); (*fr->print_state)(sr);

	    	(void) printf("\nEND STATES AT NEW NODE\n\n");
	    }
	}
#endif /* defined(DEBUG_NODE_PROPAGATE) */
	propagation_status(newn) = PROPAGATED_NODE;
	debug_print("B_node","Left B_node_propagate(), ");
	if (debugging("B_node"))
	    print_node_status("status = ",status,"\n");
	return status;
}		/*end B_node_propagate*/




#if defined(DEBUG_NODE_PROPAGATE)
LOCAL   void debug_print_B_node_curves(
	const char	*old_new,
	O_CURVE		*cphys,
	O_CURVE		*cahead,
	O_CURVE		*caprop,
	O_CURVE		*cbehind,
	O_CURVE         *cbprop)
{
	boolean p_show_states = (debugging("states") || debugging("p_states")) ?
							YES : NO;
	boolean b_show_states = (debugging("states") || debugging("b_states")) ?
							YES : NO;

	(void) printf("\t\t%sPHYSICAL CURVE:\n",old_new);
	if (p_show_states)   show_curve_states(cphys->curve);
	else                 print_o_curve(cphys);

	(void) printf("\t\t%sAHEAD BOUNDARY:\n",old_new);
	if (b_show_states)   show_curve_states(cahead->curve);
	else                 print_o_curve(cahead);

	if (
	    (caprop != NULL) && 
	    (
	        (caprop->curve != cahead->curve) ||
		(caprop->orient != cahead->orient)
	    )
	   )
	{
	    (void) printf("\t\t%sAHEAD PROP BOUNDARY:\n",old_new);
	    if (b_show_states)   show_curve_states(caprop->curve);
	    else                 print_o_curve(caprop);
	}
  
	(void) printf("\t\t%sBEHIND BOUNDARY:\n",old_new);
	if (b_show_states)   show_curve_states(cbehind->curve);
	else                 print_o_curve(cbehind);

	if (
	    (cbprop != NULL) && 
	    (
	        (cbprop->curve != cbehind->curve) ||
		(cbprop->orient != cbehind->orient)
	    )
	   )
	{
	    (void) printf("\t\t%sBEHIND PROP BOUNDARY:\n",old_new);
	    if (b_show_states)   show_curve_states(cbprop->curve);
	    else                 print_o_curve(cbprop);
	}
}		/*end debug_print_B_node_curves*/
#endif /* defined(DEBUG_NODE_PROPAGATE) */

/*ARGSUSED*/
EXPORT	int pp_node_propagate(
	Front		*fr,
	POINTER		wave,
	NODE		*oldn,
	NODE		*newn,
	RPROBLEM	**rp,
	double		dt,
	double		*dt_frac)
{
	CURVE		*oldc, *newc;
	Locstate	n_lst, n_rst;
	NODE		*on, *nn;
	NODE		*interact_nodes[5];
	O_NODE		On, Nn;
	O_NODE		*old_on, *new_on;
	O_NODE		*old_oppon, *new_oppon;
	FT_POINT		*p;
	double		*op, *np, *oop, *nop;
	double		V[MAXD];
	double		od[MAXD], nd[MAXD];
	double		m, l, f;
	int		dim = fr->rect_grid->dim;
	int		i, j, k;
	ORIENTATION	oc_or;
	int		num_c;
	DEBUG_ENTER(pp_node_propagate)

	debug_print("pp_node","Entered pp_node_propagate()\n");

	if (propagation_status(newn) == PROPAGATED_NODE)
	{
	    debug_print("pp_node","Left pp_node_propagate()\n");
	    DEBUG_LEAVE(pp_node_propagate)
	    return GOOD_NODE;
	}
	if (debugging("pp_node"))
	{
	    (void) printf("Interfaces into pp_node_propagate\n");
	    (void) printf("Old Interface\n");
	    print_interface(oldn->interface);
	    (void) printf("New Interface\n");
	    print_interface(newn->interface);
	}

	old_on = &On; new_on = &Nn;
	for (on=oldn, nn=newn; on && nn; on=next_node(on), nn=next_node(nn))
	{
	    if (!(is_subdomain_node(on) || is_virtual_fixed_node(on)))
	    	continue;
	    if (debugging("pp_node"))
	    	(void) printf("propagating pp_node, oldn %llu, newn %llu\n",
			      node_number(on),node_number(nn));
	    old_on->next = make_onode(on);
	    old_on->next->prev = old_on;
	    old_on = old_on->next;

	    new_on->next = (O_NODE *) Store(sizeof(O_NODE));
	    new_on->next->prev = new_on;
	    new_on = new_on->next;
	    new_on->next = NULL;
	    new_on->node = nn;
	    new_on->num_c = num_c = old_on->num_c;
	    new_on->nc     = (CURVE **) Store(num_c * sizeof(CURVE *));
	    new_on->nopp   = (NODE **)  Store(num_c * sizeof(NODE *));
	    new_on->pt     = (FT_POINT **) Store(num_c * sizeof(FT_POINT *));
	    new_on->ang    = NULL;
	    new_on->orient = (ORIENTATION *)    Store(num_c * INT);
	    for (i = 0;  i < old_on->num_c;  ++i)
	    {
	    	oldc = old_on->nc[i];	oc_or = old_on->orient[i];
	    	if (oc_or == POSITIVE_ORIENTATION)
	    	{
	    	    newc = find_correspond_curve(oldc,nn,NULL,fr,nn->interface);
	    	    new_on->nopp[i] = newc->end;
	    	}
	    	else
	    	{
	    	    newc = find_correspond_curve(oldc,NULL,nn,fr,nn->interface);
	    	    new_on->nopp[i] = newc->start;
	    	}
		new_on->pt[i] = p = Point((double*)NULL);
		new_on->nc[i] = newc;	new_on->orient[i] = oc_or;
		if (wave_type(oldc) != SUBDOMAIN_BOUNDARY)
		{
		    on->posn->hse = Hyper_surf_element(Bond_at_node(oldc,
						oc_or));
		    on->posn->hs = Hyper_surf(oldc);
		    point_propagate(fr,wave,on->posn,p,
		    		    Bond_at_node(oldc,oc_or),oldc,dt,V);
		    n_lst =  Left_state_at_node(newc,oc_or);
		    n_rst = Right_state_at_node(newc,oc_or);
		    ft_assign(n_lst,left_state(p),fr->sizest);
		    ft_assign(n_rst,right_state(p),fr->sizest);
		}
	    }
	    propagation_status(nn) = PROPAGATED_NODE;
	    if (debugging("pp_node"))
	    {
		(void) printf("Onodes created\n");
		(void) printf("old_on\n");	print_onode(old_on);
		(void) printf("new_on\n");	print_onode(new_on);
	    }
	}
	On.next->prev = Nn.next->prev = NULL;
	for (old_on = On.next, new_on = Nn.next; old_on != NULL;
		old_on = old_on->next, new_on = new_on->next)
	{
	    if (debugging("pp_node"))
	    {
	    	(void) printf("checking onodes old_on ");
	    	(void) printf("%p and new_on %p for crossings\n",
			      (POINTER)old_on,(POINTER)new_on);
	    }
	    op = Coords(old_on->node->posn);
	    for (k = 0; k < old_on->num_c; ++k)
	        if (wave_type(old_on->nc[k]) >= FIRST_PHYSICS_WAVE_TYPE)
	    	    break;
	    np = (k < old_on->num_c) ? Coords(new_on->pt[k]) : op;
	    for (i = 0;  i < old_on->num_c;  ++i)
	    {
	    	if (wave_type(old_on->nc[i]) != SUBDOMAIN_BOUNDARY)
	    	    continue;
	    	for (old_oppon=old_on->next, new_oppon=new_on->next;
		     old_oppon != NULL;
		     old_oppon = old_oppon->next, new_oppon = new_oppon->next)
		{
		    if (old_on->nopp[i] == old_oppon->node)
		        break;
		}
		if (old_oppon == NULL)
		    continue;
		oop = Coords(old_oppon->node->posn);
		for (k = 0; k < old_oppon->num_c; ++k)
		    if (wave_type(old_oppon->nc[k]) >= FIRST_PHYSICS_WAVE_TYPE)
			break;
		nop = (k < old_oppon->num_c) ? Coords(new_oppon->pt[k]) : oop;
		for (j = 0; j < dim; ++j)
		{
		    od[j] = oop[j] - op[j];
		    nd[j] = nop[j] - np[j];
		}
		l = scalar_product(od,nd,dim);
		if (debugging("pp_node"))
		{
		    (void) printf("old_oppon %p new_oppon %p\n",
				  (POINTER)old_oppon,(POINTER)new_oppon);
		    print_general_vector("op = ",op,dim,"");
		    print_general_vector(", np = ",np,dim,"");
		    print_general_vector(", oop = ",oop,dim,"");
		    print_general_vector(", nop = ",nop,dim,"\n");
		    print_general_vector("od = ",oop,dim,"");
		    print_general_vector(", nd = ",nop,dim,"");
		    (void) printf(", l = %g\n",l);
		}
		if (l < 0.0)
		{
		    /* Nodes cross during time propagation */
		    m = scalar_product(od,od,dim);
		    f = m/(m - l);
		    *dt_frac = min(*dt_frac,f);
		    interact_nodes[0] = new_on->node;
		    interact_nodes[1] = old_on->node;
		    interact_nodes[2] = new_oppon->node;
		    interact_nodes[3] = old_oppon->node;
		    interact_nodes[4] = NULL;
		    augment_rproblem_list(rp,interact_nodes,dt,f,
					  oldn->interface,newn->interface,
					  fr,wave);
		    if (debugging("pp_node"))
		    {
		    	(void) printf("Nodes cross, ");
		    	(void) printf("f = %g, dt_frac = %g\n",f,*dt_frac);
			print_general_vector("Coords(old_on->node->posn) = ",
				             Coords(old_on->node->posn),
				             dim,", ");
			print_general_vector("Coords(old_oppon->node->posn) = ",
				             Coords(old_oppon->node->posn),
				             dim,"\n");
			print_general_vector("Coords(new_on->node->posn) = ",
				             Coords(new_on->node->posn),
				             dim,", ");
			print_general_vector("Coords(new_oppon->node->posn) = ",
				             Coords(new_oppon->node->posn),
				             dim,"\n");
		    }
		}
	    }
	}
	debug_print("pp_node","Left pp_node_propagate()\n");
	return GOOD_NODE;
}		/*end pp_node_propagate*/

/* 
*		set_node_states_and_continue():
*
*	If we are going to ignore an error at a node we should copy
*	all the states from the old node so that there are valid
*	states at the new node, even if they don't make sense.
*	Also, set the status of the node to PROPAGATED.
*	
*/

EXPORT int set_node_states_and_continue(
	NODE		*oldn,
	NODE		*newn,
	Front		*fr)
{
	CURVE		**newc, *oldc;
	size_t		sizest = size_of_state(oldn->interface);

	debug_print("sns_and_c","Entered set_node_states_and_continue()\n");

	for (newc = newn->in_curves; newc && *newc; ++newc)
	{
	    oldc = find_correspond_curve(*newc,NULL,oldn,
					 fr,oldn->interface);
	    if (oldc != NULL)
	    {
	    	ft_assign(left_end_state(*newc),left_end_state(oldc),sizest);
		ft_assign(right_end_state(*newc),right_end_state(oldc),sizest);
	    }
	    else
	    {
	    	(void) printf("WARNING in set_node_states_and_continue(), ");
	    	(void) printf("unable to find correspond curve\n");
	    	print_curve(*newc);
	    	return NO;
	    }
	}

	for (newc = newn->out_curves; newc && *newc; ++newc)
	{
	    oldc = find_correspond_curve(*newc,oldn,NULL,
					 fr,oldn->interface);
	    if (oldc != NULL)
	    {
	    	ft_assign(left_start_state(*newc),left_start_state(oldc),sizest);
	    	ft_assign(right_start_state(*newc),right_start_state(oldc),sizest);
	    }
	    else
	    {
	    	(void) printf("ERROR in set_node_states_and_continue(), ");
	    	(void) printf("unable to find correspond curve\n");
	    	print_curve(*newc);
	    	return NO;
	    }
	}

	propagation_status(newn) = PROPAGATED_NODE;
	debug_print("sns_and_c","Left set_node_states_and_continue()\n");
	return YES;
}		/*end set_node_states_and_continue*/

EXPORT void assign_states_on_passive_curves_at_node(
	NODE		*node)
{
	CURVE		**c;

	for (c = node->in_curves; c && *c; ++c)
	{
	    if (wave_type(*c) == PASSIVE_BOUNDARY)
	    {
	    	obstacle_state(node->interface,left_end_state(*c),
	    		       size_of_state(node->interface));
	    	obstacle_state(node->interface,right_end_state(*c),
	    	               size_of_state(node->interface));
	    }
	}

	for (c = node->out_curves; c && *c; ++c)
	{
	    if (wave_type(*c) == PASSIVE_BOUNDARY)
	    {
	    	obstacle_state(node->interface,left_start_state(*c),
	    	               size_of_state(node->interface));
	    	obstacle_state(node->interface,right_start_state(*c),
	    	               size_of_state(node->interface));
	    }
	}
}		/*end assign_on_passive_curves_at_node*/

EXPORT int f_node_propagate(
	Front		*fr,
	POINTER		p2wave,
	NODE		*oldn,
	NODE		*newn,
	RPROBLEM	**rp,
	double		dt,
	double		*dt_frac,
	NODE_FLAG	flag,
	POINTER		user)
{
	int		status = ERROR_NODE;
	int		i, dim = fr->rect_grid->dim;

	debug_print("node_propagate","Entered f_node_propagate()\n");

#if defined(DEBUG_NODE_PROPAGATE)
	if (debugging("node_propagate")) print_node(oldn);
#endif /* defined(DEBUG_NODE_PROPAGATE) */

	if (oldn != NULL && oldn->in_curves==NULL && oldn->out_curves==NULL) 
	{
	    status = GOOD_NODE;
	    return status;
	}

		/* Propagate node according to its type */

	switch (node_type(newn)) 
	{
	case PASSIVE_NODE:
	    assign_states_on_passive_curves_at_node(newn);
	    status = GOOD_NODE;
	    propagation_status(newn) = PROPAGATED_NODE;
	    for (i = 0; i < dim; i++)
	        Node_vel(newn)[i] = 0.0;
	    break;

	case FIXED_NODE:
	    status = fixed_node_propagate(fr,p2wave,oldn,newn,dt);
	    break;

	case SUBDOMAIN_NODE:
	    status = pp_node_propagate(fr,p2wave,oldn,newn,rp,dt,dt_frac);
	    break;

	case CLOSED_NODE:
	    status = closed_node_propagate(fr,p2wave,oldn,newn,dt);
	    break;
	
        case MONO_COMP_NODE:
            status = free_end_node_propagate(fr,p2wave,oldn,newn,dt);
            break;

	case DIRICHLET_NODE:
	    status = B_node_propagate(fr,p2wave,oldn,newn,rp,dt,dt_frac,flag);
	    break;

	case NEUMANN_NODE: 
	    status = B_node_propagate(fr,p2wave,oldn,newn,rp,dt,dt_frac,flag);
	    break;

	default:
	    screen("ERROR in f_node_propagate(), "
	           "unknown node type %d\n",node_type(newn));
	    clean_up(ERROR);
	    break;
	}

	debug_print("node_propagate","Left f_node_propagate(), \n");
#if defined(DEBUG_NODE_PROPAGATE)
	if (debugging("node_propagate")) print_node(newn);
#endif /* defined(DEBUG_NODE_PROPAGATE) */
	return status;
}	/*end f_node_propagate*/


LOCAL	void copy_state_to_node(
	Locstate state,
	COMPONENT comp,
	NODE *n,
	size_t sizest)
{
	CURVE **c;
	for (c = n->out_curves; c && *c; ++c)
	{
	    if (negative_component(*c) == comp)
		ft_assign(left_start_state(*c),state,sizest);
	    if (positive_component(*c) == comp)
		ft_assign(right_start_state(*c),state,sizest);
	}
	for (c = n->in_curves; c && *c; ++c)
	{
	    if (negative_component(*c) == comp)
		ft_assign(left_end_state(*c),state,sizest);
	    if (positive_component(*c) == comp)
		ft_assign(right_end_state(*c),state,sizest);
	}
}	/* end copy_state_to_node */

LOCAL	boolean close_to_subdomain_bdry(
	O_CURVE Oldcphys,
	O_CURVE Oldcahead,
	O_CURVE Oldcbehind)
{
	NODE *opp_node;
	CURVE **c;
	if (Oldcphys.curve->num_points == 1)
	{
	    opp_node = Opp_node_of_o_curve(&Oldcphys);
	    for (c = opp_node->in_curves; c && *c; ++c)
		if (wave_type(*c) == SUBDOMAIN_BOUNDARY)
		    return YES;
	    for (c = opp_node->out_curves; c && *c; ++c)
		if (wave_type(*c) == SUBDOMAIN_BOUNDARY)
		    return YES;
	}
	if (Oldcahead.curve->num_points == 1)
	{
	    opp_node = Opp_node_of_o_curve(&Oldcahead);
	    for (c = opp_node->in_curves; c && *c; ++c)
		if (wave_type(*c) == SUBDOMAIN_BOUNDARY)
		    return YES;
	    for (c = opp_node->out_curves; c && *c; ++c)
		if (wave_type(*c) == SUBDOMAIN_BOUNDARY)
		    return YES;
	}
	if (Oldcbehind.curve->num_points == 1)
	{
	    opp_node = Opp_node_of_o_curve(&Oldcbehind);
	    for (c = opp_node->in_curves; c && *c; ++c)
		if (wave_type(*c) == SUBDOMAIN_BOUNDARY)
		    return YES;
	    for (c = opp_node->out_curves; c && *c; ++c)
		if (wave_type(*c) == SUBDOMAIN_BOUNDARY)
		    return YES;
	}
	return NO;
}	/* end close_to_subdomain_bdry */
