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
*				frp1.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*/

#if defined(TWOD)

#include <front/fdecs.h>		/* includes int.h, table.h */

	/* LOCAL Function Declarations */
LOCAL	int	is_curve_near_linear(FT_POINT*,FT_POINT*,CURVE*,RECT_GRID*);
LOCAL	int	non_null_bdry_curve_at_node(RPROBLEM*,NODE*,O_CURVE*);
LOCAL	int	test_newc_for_null(CURVE*,RP_NODE*,RP_NODE*,RPROBLEM*);
LOCAL	void	delete_null_bdry_curves_at_node(RPROBLEM*,RP_NODE*,Front*);
LOCAL	void	delete_from_rp_node_list(RPROBLEM*,RP_NODE*);
LOCAL	void	f_free_rp_node(RP_NODE*rpn,RPROBLEM*);
LOCAL	void	f_set_phys_ocurves_to_null(RP_NODE*,RPROBLEM*);
LOCAL	void	f_user_free_rp_node(RP_NODE*,RPROBLEM*);
LOCAL	void	f_user_print_rp_node(RP_NODE*,RPROBLEM*);
LOCAL	void	f_user_print_rproblem(RPROBLEM*);
LOCAL	void	init_bdry_ocurves(RP_NODE*);
LOCAL	void	make_ang_ordered_list(RPROBLEM*,O_CURVE*,O_CURVE_FAMILY**);
LOCAL	void	roclists_after_delete(RPROBLEM*,CURVE*);
LOCAL	void	substitute_boundary_continuation(O_CURVE**,RPROBLEM*);
LOCAL 	void 	f_init_physical_ocurves(RPROBLEM*);
LOCAL 	void 	f_replace_null_curves_in_family(O_CURVE_FAMILY**,RPROBLEM*);
LOCAL	void 	f_print_rp_statistical_data(RPROBLEM*);

#if !defined(__INTEL_COMPILER)
#pragma	noinline	is_curve_near_linear
#pragma	noinline	non_null_bdry_curve_at_node
#pragma	noinline	test_newc_for_null
#pragma	noinline	delete_null_bdry_curves_at_node
#pragma	noinline	delete_o_curve_with_curve
#pragma	noinline	delete_from_rp_node_list
#pragma	noinline	f_set_phys_ocurves_to_null
#pragma	noinline	f_user_free_rp_node
#pragma	noinline	f_user_print_rp_node
#pragma	noinline	f_user_print_rproblem
#pragma	noinline	init_bdry_ocurves
#pragma	noinline	make_ang_ordered_list
#pragma	noinline	roclists_after_delete
#pragma	noinline	substitute_boundary_continuation
#endif /*!defined(__INTEL_COMPILER)*/

LOCAL	F_USER_RPROBLEM Rp_user_hook = {
	sizeof(F_RPROBLEM),
	sizeof(RP_NODE),
	f_init_rp_nodes,
	f_delete_curve_from_rp_node,
	f_free_rp_node,
	f_user_free_rp_node,
	f_print_rp_node,
	f_user_print_rp_node,
	f_print_rproblem,
	f_user_print_rproblem,
	f_set_rp_statistics,
	f_set_phys_ocurves_to_null
};


EXPORT	F_USER_RPROBLEM* rp_user_hook(void)
{
	return &Rp_user_hook;
}		/*end rp_user_hook*/

/*ARGSUSED*/
LOCAL	void f_set_phys_ocurves_to_null(
	RP_NODE		*rpn,
	RPROBLEM	*rp)
{
}		/*end f_set_phys_ocurves_to_null*/

/*ARGSUSED*/
LOCAL	void	f_user_print_rproblem(
	RPROBLEM	*rp)
{
}		/*end f_user_print_rproblem*/

/*
*			augment_rproblem_list():
*
*	Given a current RPROBLEM and a set of interacting nodes, this
*	routine will allocate a new RPROBLEM for the interacting nodes
*	and update the current pointer. If the interacting nodes overlap
*	with any previous RPROBLEM, the RPROBLEMs are merged and the extra
*	is deleted.
*/


EXPORT void augment_rproblem_list(
	RPROBLEM	**curr_rp,
	NODE		**interact_nodes,
	double		dt,
	double		dt_frac,
	INTERFACE	*old_intfc,
	INTERFACE	*new_intfc,
	Front		*fr,
	POINTER		wave)
{
	F_USER_RPROBLEM *rpuh = rp_user_hook();
	RPROBLEM	*rp,*rp1;
	NODE		**n, *n1, *n1old;
	int		size_rproblem = rpuh->size_rproblem;

	    /* install new rproblem */

	debug_print("rproblem","Entered augment_rproblem_list()\n");
	scalar(&rp,size_rproblem);
	if (debugging("rproblem"))
	    (void) printf("curr_rp %p rp %p\n",(POINTER)*curr_rp,(POINTER)rp);
	rp->old_intfc = old_intfc;
	rp->new_intfc = new_intfc;
	rp->fr = fr;
	rp->wave = wave;
	rp->prev = *curr_rp;
	f_user_rproblem(rp) = *rpuh;
	if (*curr_rp) 
	{
	    rp->next = (*curr_rp)->next;
	    if ((*curr_rp)->next)
	    	(*curr_rp)->next->prev = rp;
	    (*curr_rp)->next = rp;
	}
	if (debugging("rproblem"))
	{
	    NODE	**n;
	    int	num;
	    for (num = 0, n = interact_nodes; n && *n; num++, n++);
	    num /= 2;
	    (void) printf("Start node list\n");
	    (void) printf("num = %d\n",num);
	}
	for (n = interact_nodes; *n; n++) 
	{
	    n1 = *n;
	    n++;
	    n1old = *n;
	    (void) add_to_rp_node_list(rp,n1,n1old);
	}
	rp->dt = dt;
	rp->dt_frac = dt_frac;
	*curr_rp = rp;

	    /* Check if interaction belongs to some previous rproblem */

	rp = (*curr_rp)->prev;
	while(rp) 
	{
	    rp1 = rp->prev;
	    merge_and_delete_overlaps(*curr_rp,rp);
	    rp = rp1;
	}

	if (debugging("rproblem"))
	    print_rproblem(*curr_rp);
	debug_print("rproblem","Left augment_rproblem_list()\n");
}		/*end augment_rproblem_list*/

/*
*			merge_and_delete_overlaps():
*
*	Merges rp2 into rp1 and deletes rp2.
*/

EXPORT void merge_and_delete_overlaps(
	RPROBLEM	*rp1,
	RPROBLEM	*rp2)
{
	RP_NODE		*rpn1, *rpn2;

	debug_print("rproblem",
	      "Entered merge_and_delete_overlaps(rp1 %d rp2 %d)\n",rp1,rp2);
	for (rpn1 = rp1->first_rp_node; rpn1 != NULL; rpn1 = rpn1->next)
	{
	    for (rpn2 = rp2->first_rp_node; rpn2 != NULL; rpn2 = rpn2->next)
	    {
	    	if (rpn1->node == rpn2->node) 
	    	{
	    	    merge_rproblems(rp1,rp2);
	    	    debug_print("rproblem","Left merge_and_delete_overlaps()\n");
	    	    return;
	    	}
	    }
	}
	debug_print("rproblem","Left merge_and_delete_overlaps()\n");
}		/*end merge_and_delete_overlaps*/

EXPORT void merge_rproblems(
	RPROBLEM	*rp1,
	RPROBLEM	*rp2)
{
	RP_NODE		*rpn1, *rpn2;

	debug_print("rproblem","Entered merge_rproblems(rp1 %d rp2 %d)\n",rp1,rp2);
	for (rpn1 = rp1->first_rp_node; rpn1 != NULL; rpn1 = rpn1->next)
	{
	    for (rpn2 = rp2->first_rp_node; rpn2 != NULL; rpn2 = rpn2->next)
	    {
	    	if (rpn2->node == rpn1->node)
	    	    delete_from_rp_node_list(rp2,rpn2);
	    }
	}
	if (rp2->first_rp_node)
	{
	    if (rp1->last_rp_node)
	    {
	    	rp1->last_rp_node->next = rp2->first_rp_node;
	    	rp2->first_rp_node->prev = rp1->last_rp_node;
	    }
	    else
	    	rp1->first_rp_node = rp2->first_rp_node;
	    rp1->last_rp_node = rp2->last_rp_node;
	}
	rp2->first_rp_node = rp2->last_rp_node = NULL;
	if (rp2->prev)
	    rp2->prev->next = rp2->next;
	if (rp2->next)
	    rp2->next->prev = rp2->prev;
	rp1->dt_frac = min(rp1->dt_frac,rp2->dt_frac);
	free_rp(rp2);
	debug_print("rproblem","Left merge_rproblems()\n");
}		/*end merge_rproblems*/

/*
*			find_curves_with_wave_type():
*/

EXPORT	void find_curves_with_wave_type(
	NODE		*n,
	CURVE		**c1,
	ORIENTATION	*orient1,
	CURVE		**c2,
	ORIENTATION	*orient2,
	int		type)
{
	CURVE		**c_beg, **c;
	int		i;
	ORIENTATION	c_or;

	*c1 = *c2 = NULL;

	debug_print("rproblem","Entered find_curves_with_wave_type()\n");
	if (debugging("rproblem"))
	      (void) printf("\t\tnode %llu curve type %d\n",node_number(n),type);

	for (i = 0, c_beg = n->in_curves,  c_or = NEGATIVE_ORIENTATION;
	     i < 2;
	     i++,   c_beg = n->out_curves, c_or = POSITIVE_ORIENTATION)
	{
	    for (c = c_beg;  c && *c;  c++)
	    {
	    	if (wave_type(*c) == type)
	    	{
	    	    if (*c1 == NULL)
	    	    {
	    		*c1 = *c;
	    		*orient1 = c_or;
	    	    }
	    	    else
	    	    {
	    		*c2 = *c;
	    		*orient2 = c_or;
	                debug_print("rproblem","Left find_curves_with_wave_type()\n");
	    		return;
	    	    }
	    	}
	    }
	}
	if (debugging("rproblem"))
	{
	    (void) printf("\t\t");
	    if (*c1)
	    	(void) printf("c1 %llu, ",curve_number(*c1));
	    else
	    	(void) printf("c1 NULL, ");

	    if (*c2)
	    	(void) printf("c2 %llu\n",curve_number(*c2));
	    else
	    	(void) printf("c2 NULL\n");
	}
	debug_print("rproblem","Left find_curves_with_wave_type()\n");
}		/*end find_curves_with_wave_type*/

/*
*			init_cfamily():
*/

EXPORT void init_cfamily(
	O_CURVE_FAMILY	**cfamily,
	CURVE		*c,
	ORIENTATION	orient)
{
	scalar(cfamily,sizeof(O_CURVE_FAMILY));
	scalar(&(*cfamily)->first,sizeof(O_CURVE));

	(*cfamily)->last = (*cfamily)->first;
	(*cfamily)->first->curve = c;
	(*cfamily)->first->orient = orient;
	(*cfamily)->first->prev = NULL;
	(*cfamily)->first->next = NULL;
}		/*end init_cfamily*/

/*
*			join_cfamilies():
*
*	Appends a copy of cf2 onto the end of cf1, if cf2 is null then nothing 
*	is done.  If cf1 is null and cf2 is non null then storage for cf1 
*	will be allocated and cf2 will be copied onto cf1.
*	The family cf2 returns unchanged.
*/

EXPORT	void join_cfamilies(
	O_CURVE_FAMILY	**cf1,
	O_CURVE_FAMILY	*cf2)
{
	O_CURVE		*oc, *oc1;

	if (!cf2)
	    return;
	for (oc = cf2->first; oc != NULL; oc = oc->next)
	{
	    init_o_curve(&oc1,oc->curve,oc->orient);
	    add_oc_curve_to_family(oc1,cf1);
	}
}		/*end join_cfamilies*/

/*
*			add_oc_curve_to_family():
*/

EXPORT	void add_oc_curve_to_family(
	O_CURVE		*oc,
	O_CURVE_FAMILY	**cfamily)
{
	if (!oc)
	    return;
	if (!*cfamily) 
	{
	    scalar(cfamily,sizeof(O_CURVE_FAMILY));
	    (*cfamily)->first = (*cfamily)->last = oc;
	    oc->prev = oc->next = NULL;
	    return;
	}
	else if (!(*cfamily)->last)
	{
	    (*cfamily)->first = (*cfamily)->last = oc;
	    oc->prev = oc->next = NULL;
	    return;
	}
	oc->next = (*cfamily)->last->next;
	if (oc->next)
	    oc->next->prev = oc;
	(*cfamily)->last->next = oc;
	oc->prev = (*cfamily)->last;
	(*cfamily)->last = oc;
}		/*end add_oc_curve_to_family*/


/*
*			init_o_curve():
*/

EXPORT void init_o_curve(
	O_CURVE		**ocurve,
	CURVE		*c,
	ORIENTATION	orient)
{
	scalar(ocurve,sizeof(O_CURVE));
	(*ocurve)->curve = c;
	(*ocurve)->orient = orient;
	(*ocurve)->prev = NULL;
	(*ocurve)->next = NULL;
}		/*end init_o_curve*/


/*
*			is_bdry_type():
*
*	Determines if a riemann problem arises from interaction of one
*	or more boundary nodes. Records the number of neumann or dirichlet
*	nodes, number of fixed nodes, number of source/sink nodes,
*	and the types of boundary curves bracketing the interaction length
*	along the boundary.
*/


EXPORT 	int is_bdry_type(
	RPROBLEM	*rp)
{
	RP_NODE		*rp_node;
	
	if (!rp)
	    return NO;
	for (rp_node = rp->first_rp_node; rp_node; rp_node = rp_node->next) 
	{
	    if (rp_node->neumann1 || rp_node->dirichlet1 || rp_node->subdomain1)
	        return YES;
	}
	return NO;
}		/*end is_bdry_type*/


EXPORT 	void f_set_rp_statistics(
	RPROBLEM	*rp)
{
	RP_NODE		*rp_node;
	
	if (!rp)
	    return;
	rp->bdry_type1 = rp->bdry_type2 = UNKNOWN_BOUNDARY_TYPE;
	rp->num_nd = 0;
	rp->num_fxd = 0;
	rp->num_srce = 0;
	rp->num_nod = 0;
	rp->num_bdry_nod = 0;
	rp->num_phys = 0;
	for (rp_node = rp->first_rp_node; rp_node; rp_node = rp_node->next) 
	{
	    rp->num_nod++;
	    if (rp_node->neumann1 || rp_node->dirichlet1 || rp_node->subdomain1) 
	    {
	    	rp->num_bdry_nod++;
	    	if (rp_node->neumann1)
	    	{
	    	    rp->bdry_type1 = NEUMANN_BOUNDARY;
	    	    if (rp_node->neumann2) 
	    	    	rp->bdry_type2 = NEUMANN_BOUNDARY;
	    	    else if (rp_node->dirichlet1) 
	    	    	rp->bdry_type2 = DIRICHLET_BOUNDARY;
	    	    else if (rp_node->subdomain1) 
	    	    	rp->bdry_type2 = SUBDOMAIN_BOUNDARY;
	    	}
		else if (rp_node->dirichlet1) 
		{
		    rp->bdry_type1 = DIRICHLET_BOUNDARY;
		    if (rp_node->dirichlet2) 
			rp->bdry_type2 = DIRICHLET_BOUNDARY;
		    else if (rp_node->subdomain1) 
			rp->bdry_type2 = SUBDOMAIN_BOUNDARY;
		}
		else if (rp_node->subdomain1) 
		{
		    rp->bdry_type1 = SUBDOMAIN_BOUNDARY;
		    if (rp_node->subdomain1) 
			rp->bdry_type2 = SUBDOMAIN_BOUNDARY;
		}
	    }
	    if (   (node_type(rp_node->node) == NEUMANN_NODE)
	         || (node_type(rp_node->node) == DIRICHLET_NODE))
	    	rp->num_nd++;
	    else if (is_fixed_node(rp_node->node))
	    	rp->num_fxd++;
	    else if (is_source_sink_node(rp_node->node))
	    	rp->num_srce++;
	    else if (node_type(rp_node->node) >= FIRST_PHYSICS_NODE_TYPE)
	    	rp->num_phys++;
	}
	
	rp_num_incident(rp) = 0;
	for (rp_node = rp->first_rp_node; rp_node; rp_node = rp_node->next) 
	{
	    NODE *n = rp_node->node;
	    CURVE **c;
	    for (c = n->in_curves; c && *c; ++c)
	    {
		if (((*c)->start == n) && (start_status(*c) == INCIDENT))
		    rp_num_incident(rp)++;
		else if (((*c)->end == n) && (end_status(*c) == INCIDENT))
		    rp_num_incident(rp)++;
	    }
	    for (c = n->out_curves; c && *c; ++c)
	    {
		if (((*c)->start == n) && (start_status(*c) == INCIDENT))
		    rp_num_incident(rp)++;
		else if (((*c)->end == n) && (end_status(*c) == INCIDENT))
		    rp_num_incident(rp)++;
	    }
	}
}		/*end f_set_rp_statistics*/


/*
*			f_init_rp_nodes():
*
*	Initializes the RP_NODEs associated with a given RPROBLEM.
*/

EXPORT void f_init_rp_nodes(
	RPROBLEM	*rp)
{
	RP_NODE		*rpn;
	O_CURVE		*bdryoc[2];

	/* Allocate storage; label curves at nodes */

	debug_print("2drp","Entered init_rp_nodes\n");
	for (rpn = rp->first_rp_node; rpn != NULL; rpn = rpn->next)
	{
	    if ((!rpn->node) || is_passive_node(rpn->node)) 
	    	continue;
	    init_bdry_ocurves(rpn);
	}

	/* Replace null bdry curves by continuations */

	for (rpn = rp->first_rp_node; rpn; rpn = rpn->next)
	{
	    substitute_boundary_continuation(&rpn->neumann1,rp);
	    substitute_boundary_continuation(&rpn->neumann2,rp);
	    substitute_boundary_continuation(&rpn->dirichlet1,rp);
	    substitute_boundary_continuation(&rpn->dirichlet2,rp);
	    substitute_boundary_continuation(&rpn->subdomain1,rp);
	    substitute_boundary_continuation(&rpn->subdomain2,rp);

#define save(ptr,store)							\
	if ((ptr) && !(store)[0])					\
	    (store)[0] = (ptr);						\
	else if (ptr)							\
	    (store)[1] = (ptr);						\
	(ptr) = NULL;

	    bdryoc[0] = bdryoc[1] = NULL;
	    save(rpn->neumann1,bdryoc);
	    save(rpn->neumann2,bdryoc);
	    save(rpn->dirichlet1,bdryoc);
	    save(rpn->dirichlet2,bdryoc);
	    save(rpn->subdomain1,bdryoc);
	    save(rpn->subdomain2,bdryoc);

#define locate(oc)							\
	switch (wave_type((oc)->curve))					\
	{								\
	case NEUMANN_BOUNDARY:						\
	    if (rpn->neumann1)						\
		rpn->neumann2 = oc;					\
	    else							\
		rpn->neumann1 = oc;					\
	    break;							\
	case DIRICHLET_BOUNDARY:					\
	    if (rpn->dirichlet1)					\
		rpn->dirichlet2 = oc;					\
	    else							\
		rpn->dirichlet1 = oc;					\
	    break;							\
	case SUBDOMAIN_BOUNDARY:					\
	    if (rpn->subdomain1)					\
		rpn->subdomain2 = oc;					\
	    else							\
		rpn->subdomain1 = oc;					\
	    break;							\
	}

	    if (bdryoc[0] != NULL)
		locate(bdryoc[0])
	    if (bdryoc[1] != NULL)
		locate(bdryoc[1])
	}
	debug_print("2drp","Left init_rp_nodes\n");
}		/*end init_rp_nodes*/

EXPORT	RP_NODE *add_to_rp_node_list(
	RPROBLEM	*rp,
	NODE		*n,
	NODE		*oldn)
{
	RP_NODE		*rpn;
	int		size_rp_node = f_user_rproblem(rp).size_rp_node;

	if (rp == NULL)
	    return NULL;

	/* Are nodes already in list */

	for (rpn = rp->first_rp_node; rpn; rpn = rpn->next) 
	    if (rpn->old_node == oldn && rpn->node == n)
		return rpn;

	scalar(&rpn,size_rp_node);

	rpn->next = NULL;	rpn->prev = rp->last_rp_node;
	if (rp->last_rp_node)
	    rp->last_rp_node->next = rpn;
	else
	    rp->first_rp_node = rpn;
	rp->last_rp_node = rpn;

	rpn->node = n;		rpn->old_node = oldn;

	rpn->states_assigned_at_node = NO;

	rpn->neumann1 = NULL;	rpn->neumann2 = NULL;
	rpn->dirichlet1 = NULL;	rpn->dirichlet2 = NULL;
	rpn->subdomain1 = NULL;	rpn->subdomain2 = NULL;

	set_phys_ocurves_to_null(rpn,rp);
	return rpn;
}		/*end add_to_rp_node_list*/

LOCAL	void delete_from_rp_node_list(
	RPROBLEM	*rp,
	RP_NODE		*rpn)
{
	if (!rpn)
	    return;

	if (rpn == rp->first_rp_node && rpn == rp->last_rp_node)
	{
	    rp->first_rp_node = rp->last_rp_node = NULL;
	}
	else if (rpn == rp->first_rp_node)
	{
	    rp->first_rp_node = rpn->next;
	    rpn->next->prev   = rpn->prev;
	    if (rpn->prev)
		rpn->prev->next = rpn->next;
	}
	else if (rpn == rp->last_rp_node)
	{
	    rp->last_rp_node = rpn->prev;
	    rpn->prev->next  = rpn->next;
	    if (rpn->next)
		rpn->next->prev = rpn->prev;
	}
	else
	{
	    rpn->prev->next = rpn->next;
	    rpn->next->prev = rpn->prev;
	}
	free_rp_node(rpn,rp);
}		/*end delete_from_rp_node_list*/


LOCAL void init_bdry_ocurves(
	RP_NODE		*rp_node)
{
	CURVE		*c1,*c2;
	ORIENTATION	orient1,orient2;

		/* Default settings */

	if (rp_node->neumann1)
	    free(rp_node->neumann1);
	if (rp_node->neumann2)
	    free(rp_node->neumann2);
	rp_node->neumann1   = rp_node->neumann2   = NULL;

	if (rp_node->dirichlet1)
	    free(rp_node->dirichlet1);
	if (rp_node->dirichlet2)
	    free(rp_node->dirichlet2);
	rp_node->dirichlet1 = rp_node->dirichlet2 = NULL;

	if (rp_node->subdomain1)
	    free(rp_node->subdomain1);
	if (rp_node->subdomain2)
	    free(rp_node->subdomain2);
	rp_node->subdomain1  = rp_node->subdomain2  = NULL;
	
	find_curves_with_wave_type(rp_node->node,&c1,&orient1,&c2,&orient2,
				   NEUMANN_BOUNDARY);
	if (c1)
	    init_o_curve(&rp_node->neumann1,c1,orient1);
	if (c2)
	    init_o_curve(&rp_node->neumann2,c2,orient2);

	find_curves_with_wave_type(rp_node->node,&c1,&orient1,&c2,&orient2,
				   DIRICHLET_BOUNDARY);
	if (c1)
	    init_o_curve(&rp_node->dirichlet1,c1,orient1);
	if (c2)
	    init_o_curve(&rp_node->dirichlet2,c2,orient2);

	find_curves_with_wave_type(rp_node->node,&c1,&orient1,&c2,&orient2,
				   SUBDOMAIN_BOUNDARY);
	if (c1)
	    init_o_curve(&rp_node->subdomain1,c1,orient1);
	if (c2)
	    init_o_curve(&rp_node->subdomain2,c2,orient2);
}		/*end init_bdry_ocurves*/


/*
*				is_null_curve():
*
*	A null curve has distinct nodes for ends, both of which belong to
*	the Riemann problem rp.  Perhaps there should also be a check that
*	the length of the curve is small relative to a mesh spacing, so that
*	loops and null curves can be distinguished.
*/

EXPORT int is_null_curve(
	CURVE		*curve,
	RPROBLEM	*rp)
{
	RP_NODE		*rpn1, *rpn2;

	if (!curve || is_closed_curve(curve))
	    return NO;

	if (curve->interface == rp->new_intfc)
	{
	    for (rpn1 = rp->first_rp_node; rpn1 != NULL; rpn1 = rpn1->next)
	    {
	        if (rpn1->node == curve->start)
	        {
	    	    for (rpn2=rp->first_rp_node; rpn2!=NULL; rpn2=rpn2->next)
		    {
		        if (rpn2->node == curve->end)
		        {
		    	return test_newc_for_null(curve,rpn1,rpn2,rp);
		        }
		    }
	        }
	    }
	}
	else if (curve->interface == rp->old_intfc)
	{
	    FT_POINT *p0 = curve->start->posn, *p1 = curve->end->posn;
	    RECT_GRID *gr = computational_grid(rp->old_intfc);

	    for (rpn1 = rp->first_rp_node; rpn1 != NULL; rpn1 = rpn1->next)
	    {
	    	if (rpn1->old_node == curve->start)
	    	{
	    	    for (rpn2=rp->first_rp_node; rpn2!=NULL; rpn2=rpn2->next)
		    {
		        if (rpn2->old_node == curve->end)
		        {
			    return is_curve_near_linear(p0,p1,curve,gr);
		        }
		    }
		}
	    }
	}
	return NO;
}		/*end is_null_curve*/


/*
*			test_newc_for_null():
*
*	Given a curve both of whose nodes belong to the RPROBLEM rp,
*	this function performs a series of test to determine if the
*	curve is indeed null with respect to the set of interacting
*	nodes.  The primary purpose of this function is to distinguish
*	between truly null curves, and non-trivial loops whose endpoints
*	participate in the interaction, but are not to be deleted.
*/

LOCAL	int test_newc_for_null(
	CURVE		*cur,
	RP_NODE		*rpn1,
	RP_NODE		*rpn2,
	RPROBLEM	*rp)
{
	Front		*fr = rp->fr;
	POINTER		wave = rp->wave;
	RECT_GRID	*gr = computational_grid(cur->interface);
	FT_POINT		*p0, *p1;
	BOND		*b;
	O_CURVE		Oc, Oldoc;
	double		*L = gr->L, *U = gr->U;
	double		V[MAXD];
	int		status = YES;
	int		dim = gr->dim;
	static	FT_POINT	*newp0 = NULL, *newp1 = NULL;
	
	if (newp1 == NULL)
	{
	    newp0 = Static_point(cur->interface);
	    newp1 = Static_point(cur->interface);
	}

	/* Test #1  have all interior points of cur propagated out of bounds? */

	for (b = cur->first; b != cur->last; b = b->next)
	{
	    if (!outside_point(Coords(b->end),L,U,dim))
	    {
	    	status = NO;
	    	break;
	    }
	}

	if (status == YES)
	    return status;

	/* Test #2 is curve close to line segment connecting nodes? */


	Oc.curve = cur;		Oc.orient = POSITIVE_ORIENTATION;
	if (propagation_status(cur->start) == PROPAGATED_NODE)
	    p0 = cur->start->posn;
	else if (find_correspond_of_oriented_curve(&Oc,&Oldoc,rpn1->old_node,
	    	                                   fr,rp->old_intfc))
	{
	    p0 = newp0;
	    point_propagate(fr,wave,Node_of_o_curve(&Oldoc)->posn,newp0,
			    Bond_at_node_of_o_curve(&Oldoc),Oldoc.curve,
			    rp->dt,V);
	}
	else
	{
	    p0 = newp0;
	    Coords(newp0)[0] = Coords(rpn1->old_node->posn)[0] +
			       rp->dt * Node_vel(rpn1->node)[0];
	    Coords(newp0)[1] = Coords(rpn1->old_node->posn)[1] +
			       rp->dt * Node_vel(rpn1->node)[1];
	}

	Oc.curve = cur;		Oc.orient = NEGATIVE_ORIENTATION;
	if (propagation_status(cur->end) == PROPAGATED_NODE)
	    p1 = cur->end->posn;
	else if (find_correspond_of_oriented_curve(&Oc,&Oldoc,rpn2->old_node,
			                           fr,rp->old_intfc))
	{
	    p1 = newp1;
	    point_propagate(fr,wave,Node_of_o_curve(&Oldoc)->posn,
			    newp1,Bond_at_node_of_o_curve(&Oldoc),Oldoc.curve,
			    rp->dt,V);
	}
	else
	{
	    p1 = newp1;
	    Coords(newp1)[0] = Coords(rpn1->old_node->posn)[0] +
			       rp->dt * Node_vel(rpn1->node)[0];
	    Coords(newp1)[1] = Coords(rpn1->old_node->posn)[1] +
			       rp->dt * Node_vel(rpn1->node)[1];
	}

	status = is_curve_near_linear(p0,p1,cur,gr);

	return status;
}		/*end test_newc_for_null*/

LOCAL	int is_curve_near_linear(
	FT_POINT		*p0,
	FT_POINT		*p1,
	CURVE		*cur,
	RECT_GRID	*gr)
{
	BOND		*b;
	double		len;
	double		pmid[MAXD], d10[MAXD], d[MAXD];
	double		cp[MAXD];
	double		tan_dist, nor_dist;
	double		H[MAXD], Htan, Hnor;
	int		i, dim = gr->dim;
	int		short_seg;

	short_seg = YES;
	for (i = 0; i < dim; i++)
	{
	    H[i] = min(gr->h[i],0.25 * (gr->U[i] - gr->L[i]));
	    d10[i] = Coords(p1)[i] - Coords(p0)[i];
	    pmid[i] = 0.5 * (Coords(p0)[i] + Coords(p1)[i]);
	    if (fabs(d10[i]) >= H[i])
	        short_seg = NO;
	}

	if (short_seg == YES)
	{
	    for (b = cur->first; b != cur->last; b = b->next)
	    {
	    	for (i = 0; i < dim; i++)
	    	{
	    	    d[i] = fabs(Coords(b->end)[i] - pmid[i]);
	    	    if (d[i] >= H[i])
		        return NO;
	    	}
	    }
	    return YES;
	}

	len = mag_vector(d10,dim);
	for (i = 0; i < dim; i++) d10[i] /= len;
	Htan = 0.5*len + fabs(scalar_product(d10,H,dim));
	Hnor = fabs(vector_product(d10,H,cp,dim));

	for (b = cur->first; b != cur->last; b = b->next)
	{
	    for (i = 0; i < dim; i++)
	    	d[i] = Coords(b->end)[i] - pmid[i];
	    tan_dist = fabs(scalar_product(d10,d,dim));
	    nor_dist = fabs(vector_product(d10,d,cp,dim));
	    if (nor_dist >= Hnor || tan_dist >= Htan)
	        return NO;
	}
	return YES;
}		/*end is_curve_near_linear*/

/*
*			substitute_boundary_continuation():
*
*	Substitutes the boundary curve which continues the boundary past a
*	given oriented curve, assumed to be a boundary, provided it is null 
*	with respect to the riemann problem, meaning that its start and end 
*	nodes belong to the riemann problem.
*/

LOCAL void substitute_boundary_continuation(
	O_CURVE		**oc,
	RPROBLEM	*rp)
{
	CURVE		*curve;
	ORIENTATION	orient;

	if (!*oc || !rp)
	    return;
	curve = (*oc)->curve;
	orient = (*oc)->orient;
	if (wave_type(curve) >= FIRST_PHYSICS_WAVE_TYPE)
	{
	    screen("ERROR in substitute_boundary_continuation(), "
	           "physical wave type\n");
	    clean_up(ERROR);
	}
	if (!is_null_curve(curve,rp))
	    return;
	if (debugging("2drp"))
	{
	    (void) printf("Null curve found\n");	print_curve(curve);
	}
	if (next_boundary(curve,Opposite_orient(orient),&(*oc)->curve,
			  &(*oc)->orient))
	{
	    substitute_boundary_continuation(oc,rp);
	    return;
	}
	free(*oc);
	*oc = NULL;
}		/*end substitute_boundary_continuation*/


/*
*			delete_oc_curve_from_family():
*/

EXPORT void delete_oc_curve_from_family(
	O_CURVE		**poc,
	O_CURVE_FAMILY	**cfamily)
{
	O_CURVE		*oc;

	if (poc == NULL)
	    return;
	oc = *poc;
	if (oc == NULL || cfamily == NULL || *cfamily == NULL)
	    return;
	if ((oc == (*cfamily)->first) && (oc == (*cfamily)->last))
	{
	    free(*cfamily);
	    *cfamily = NULL;
	}
	else
	{
	    if (oc->prev)
	        oc->prev->next = oc->next;
	    if (oc == (*cfamily)->first)
	        (*cfamily)->first = oc->next;
	    if (oc->next)
	        oc->next->prev = oc->prev;
	    if (oc == (*cfamily)->last)
	        (*cfamily)->last = oc->prev;
	}
	free(oc);
	*poc = NULL;
}		/*end delete_oc_curve_from_family*/

/*
*			f_delete_curve_from_rp_node();
*/

/*ARGSUSED*/
EXPORT void f_delete_curve_from_rp_node(
	CURVE		*curve,
	RP_NODE		*rpn,
	RPROBLEM	*rp)
{
	if (curve == NULL || rpn == NULL)
	    return;
	if (is_bdry_like_curve(curve))
	{
	    if (rpn->neumann1)
	    	delete_o_curve_with_curve(&rpn->neumann1,curve);
	    if (rpn->neumann2)
	    	delete_o_curve_with_curve(&rpn->neumann2,curve);
	    if (rpn->dirichlet1)
	    	delete_o_curve_with_curve(&rpn->dirichlet1,curve);
	    if (rpn->dirichlet2)
	    	delete_o_curve_with_curve(&rpn->dirichlet2,curve);
	    if (rpn->subdomain1)
	    	delete_o_curve_with_curve(&rpn->subdomain1,curve);
	    if (rpn->subdomain2)
	    	delete_o_curve_with_curve(&rpn->subdomain2,curve);
	}

}		/*end f_delete_curve_from_rp_node*/

/*
*		delete_o_curve_with_curve():
*/

EXPORT	void delete_o_curve_with_curve(
	O_CURVE		**o_curve,
	CURVE		*curve)
{
	if (curve==NULL || o_curve==NULL ||
	    *o_curve==NULL || (*o_curve)->curve!=curve)
	    return;

	if ((*o_curve)->prev)
	    (*o_curve)->prev->next = (*o_curve)->next;
	if ((*o_curve)->next)
	    (*o_curve)->next->prev = (*o_curve)->prev;
	free(*o_curve);
	*o_curve = NULL;
}		/*end delete_ocurve_with_curve*/

/*
*		delete_curve_from_o_curve_family():
*/

EXPORT	void delete_curve_from_o_curve_family(
	CURVE		*curve,
	O_CURVE_FAMILY	**cfamily)
{
	O_CURVE		*oc;

start:
	if (cfamily == NULL || *cfamily == NULL)
	    return;
	if ((*cfamily)->first == NULL || (*cfamily)->last == NULL)
	{
	    *cfamily = NULL;
	    return;
	}
	for (oc = (*cfamily)->first; oc; oc = oc->next) 
	{
	    if (oc->curve == curve)	
	    {
	    	delete_oc_curve_from_family(&oc,cfamily);
	    	goto	start;
	    }
	    if (oc == (*cfamily)->last)
		break;
	}
}		/*end delete_curve_from_ocurve_family*/


EXPORT void delete_null_boundary_curves(
	RPROBLEM	*rp,
	Front		*front,
	POINTER		wave)
{
	NODE		*n;
	RP_NODE		*rpn;
	O_CURVE		*oldboc, *boc;
	CURVE		*bc;
	Locstate	sl, sr;
	double		V[MAXD];
	ORIENTATION	bc_orient;
	static	FT_POINT	*pnew = NULL;

	debug_print("2drp","Entered delete_null_boundary_curves\n");
	if (debugging("2drp"))
	{
	    (void) printf("Interface before delete_null_boundary_curves()\n");
	    print_interface(rp->new_intfc);
	}
	if (pnew == NULL)
	{
	    pnew = Static_point(front->interf);
	}

	/* Set states at nodes of non-null boundary curves */

	boc = (rp->bdry_curves) ? rp->bdry_curves->first : NULL;
	oldboc = (rp->old_bdry_curves) ? rp->old_bdry_curves->first : NULL;
	for (; boc && oldboc; boc = boc->next, oldboc = oldboc->next)
	{
	    n = Node_of_o_curve(boc);
	    if ((is_fixed_node(n)) || (is_passive_node(n)) ||
	        (is_source_sink_node(n)) ||
	        (propagation_status(n) == PROPAGATED_NODE))
	        continue;
	    if (!rp_node_with_node(&rpn,rp,Node_of_o_curve(boc)))
	    {
	    	screen("ERROR in delete_null_boundary_curves(), "
	    	       "Unable to find rp_node\n");
	    	clean_up(ERROR);
	    }
	    if (rpn->states_assigned_at_node)
		continue;
	    point_propagate(front,wave,Node_of_o_curve(oldboc)->posn,pnew,
			    Bond_at_node_of_o_curve(oldboc),
			    oldboc->curve,rp->dt,V);
	    sl = Left_state_at_node_of_o_curve(boc);
	    sr = Right_state_at_node_of_o_curve(boc);
	    if (boc->orient == oldboc->orient)
	    {
	    	ft_assign(sl,left_state(pnew),front->sizest);
	    	ft_assign(sr,right_state(pnew),front->sizest);
	    }
	    else
	    {
	    	ft_assign(sl,right_state(pnew),front->sizest);
	    	ft_assign(sr,left_state(pnew),front->sizest);
	    }
	}

	for (rpn = rp->first_rp_node; rpn; rpn = rpn->next)
	{
	    if ((rpn->node == NULL) || (is_fixed_node(rpn->node)) ||
		(is_passive_node(rpn->node)) ||
		(is_source_sink_node(rpn->node)))
		continue;
	    delete_null_bdry_curves_at_node(rp,rpn,front);
	}

	/* Delete interior points of null boundary curves */

	boc = (rp->bdry_curves) ? rp->bdry_curves->first : NULL;
	if (boc == NULL)
	{
	    debug_print("2drp","Left delete_null_boundary_curves\n");
	    return;
	}
	Check_return(next_boundary(boc->curve,boc->orient,&bc,&bc_orient),
		     delete_null_boundary_curves)
	while (is_null_curve(bc,rp))
	{
	    if (!(is_fixed_node(bc->start) && is_fixed_node(bc->end)))
	    	delete_interior_points_of_curve(front,bc);
	    bc_orient = Opposite_orient(bc_orient);
	    Check_return(next_boundary(bc,bc_orient,&bc,&bc_orient),
	    	         delete_null_boundary_curves)
	}
	debug_print("2drp","Left delete_null_boundary_curves\n");
}		/*end delete_null_boundary_curves*/

LOCAL void delete_null_bdry_curves_at_node(
	RPROBLEM	*rp,
	RP_NODE		*rpn,
	Front		*front)
{
	RP_NODE		*opprpn1;
	NODE		*n,*oppn1;
	CURVE		**c,*bc1,*bc2,*newbc,*ctmp;
	ORIENTATION	bc1_orient,bc2_orient,ctmp_orient;
	Locstate	sl,sr;
	FT_POINT		*p2new;

	debug_print("2drp","Entered delete_null_bdry_curves_at_node()\n");
	if (debugging("2drp"))
	{
	    (void) printf("Deleting null boundary curves at rp node %p\n",
	    	          (POINTER)rpn);
	    print_rp_node(rpn,rp);
	}

		/* Determine nonpassive boundary curves */

	bc1 = bc2 = NULL;
	for (c = rpn->node->in_curves; c && *c; c++) 
	{
	    if (wave_type(*c) >= FIRST_PHYSICS_WAVE_TYPE
	     || wave_type(*c) == PASSIVE_BOUNDARY)
	    {
	        if (debugging("2drp")) 
	    	    (void) printf("Physical or passive curve found\n");
		debug_print("2drp","Left delete_null_bdry_curves_at_node()\n");
	        return;
	    }
	    else if (!bc1) 
	    {
	    	bc1 = *c;
	    	bc1_orient = NEGATIVE_ORIENTATION;
	    }
	    else if (!bc2) 
	    {
	    	bc2 = *c;
	    	bc2_orient = NEGATIVE_ORIENTATION;
	    }
	}
	for (c = rpn->node->out_curves; c && *c; c++) 
	{
	    if (wave_type(*c) >= FIRST_PHYSICS_WAVE_TYPE
	    	|| wave_type(*c) == PASSIVE_BOUNDARY)
	    {
	        if (debugging("2drp")) 
	    	    (void) printf("Physical or passive curve found\n");
		debug_print("2drp","Left delete_null_bdry_curves_at_node()\n");
	        return;
	    }
	    else if (!bc1) 
	    {
	    	bc1 = *c;
	    	bc1_orient = POSITIVE_ORIENTATION;
	    }
	    else if (!bc2) 
	    {
	    	bc2 = *c;
	    	bc2_orient = POSITIVE_ORIENTATION;
	    }
	}
	if (bc1 == NULL && bc2 == NULL)
	{
	    if (debugging("2drp")) 
	    	(void) printf("No boundary curves at node\n");
	    debug_print("2drp","Left delete_null_bdry_curves_at_node()\n");
	    return;
	}
	if (is_null_curve(bc2,rp)) 
	{
	    ctmp = bc1;
	    ctmp_orient = bc1_orient;
	    bc1 = bc2;
	    bc1_orient = bc2_orient;
	    bc2 = ctmp;
	    bc2_orient = ctmp_orient;
	}
	if (!is_null_curve(bc1,rp)) 
	{
	    if (is_fixed_node(rpn->node) || is_passive_node(rpn->node) ||
		is_source_sink_node(rpn->node))
	    {
	    	if (debugging("2drp")) 
	    	{
	    	    (void) printf("Fixed, passive, source or sink"
				  "node found\n");
	    	}
	    	debug_print("2drp","Left delete_null_bdry_curves_at_node()\n");
		return;
	    }
	    if (bc2_orient != Opposite_orient(bc1_orient))
	    {
		if (debugging("2drp"))
		{
		    (void) printf("bc2_orient != "
				  "Opposite_orient(bc1_orient)\n");
		    print_orientation("bc1_orient = ",bc1_orient," ");
		    print_orientation("bc2_orient = ",bc2_orient,"\n");
		}
	    	invert_curve(bc2);
	    	roclists_after_invert(rp,bc2,(O_CURVE *)NULL);
	    	bc2_orient = Opposite_orient(bc1_orient);
	    }
	    interpolate_intfc_states(rp->new_intfc) = YES;
	    if (bc1_orient == POSITIVE_ORIENTATION)
	    {
		if (debugging("2drp"))
		{
		    (void) printf("Joining curves bc2 && bc1\n");
		    (void) printf("bc2\n");
		    print_curve(bc2);
		    (void) printf("bc1\n");
		    print_curve(bc1);
		}
	    	newbc = join_curves(bc2,bc1,negative_component(bc1),
				    positive_component(bc1),NULL);
	    	roclists_after_join(rp,bc2,NULL,bc1,NULL,newbc);
	    	(void) delete_node(bc2->end);
	    }
	    else
	    {
		if (debugging("2drp"))
		{
		    (void) printf("Joining curves bc1 && bc2\n");
		    (void) printf("bc1\n");
		    print_curve(bc1);
		    (void) printf("bc2\n");
		    print_curve(bc2);
		}
	    	newbc = join_curves(bc1,bc2,negative_component(bc1),
				    positive_component(bc1),NULL);
	    	roclists_after_join(rp,bc1,NULL,bc2,NULL,newbc);
	    	(void) delete_node(bc1->end);
	    }
	    if (newbc == NULL)
	    {
	    	screen("ERROR in delete_null_bdry_curves_at_node(), "
	    	       "join_curves() returns NULL");
	    	clean_up(ERROR);
	    }
	    delete_curve_from_rp_node(bc1,rpn,rp);
	    delete_curve_from_rp_node(bc2,rpn,rp);
	    debug_print("2drp","Left delete_null_bdry_curves_at_node()\n");
	    return;
	}

		/* Remove node with no physical or passive curves */

	n = Node_of(bc1,bc1_orient);
	sl = Left_state_at_node(bc2,bc2_orient);
	sr = Right_state_at_node(bc2,bc2_orient);

	if (is_null_curve(bc2,rp))
	    delete_interior_points_of_curve(front,bc2);
	else
	{
	    set_copy_intfc_states(YES);
	    p2new = Point(Coords(n->posn));
	    ft_assign(left_state(p2new),sl,front->sizest);
	    ft_assign(right_state(p2new),sr,front->sizest);
	    interpolate_intfc_states(rp->new_intfc) = NO;
	    insert_point_adjacent_to_node(p2new,bc2,bc2_orient);
	    interpolate_intfc_states(rp->new_intfc) = YES;
	}
	oppn1 = Node_of(bc1,Opposite_orient(bc1_orient));
	if (!rp_node_with_node(&opprpn1,rp,oppn1))
	{
	    screen("ERROR in delete_null_bdry_curves_at_node(), "
		   "rp_node_with_node() failed\n");
	    clean_up(ERROR);
	}
	if (opprpn1->states_assigned_at_node)
	{
	    if (bc1_orient != bc2_orient)
	    {
	    	ft_assign(sl,Left_state_at_node(bc1,
		       Opposite_orient(bc1_orient)),front->sizest);
	    	ft_assign(sr,Right_state_at_node(bc1,
		       Opposite_orient(bc1_orient)),front->sizest);
	    }
	    else
	    {
	    	ft_assign(sl,Right_state_at_node(bc1,
		       Opposite_orient(bc1_orient)),front->sizest);
	    	ft_assign(sr,Left_state_at_node(bc1,
		       Opposite_orient(bc1_orient)),front->sizest);
	    }
	}
	(void) delete_curve(bc1);
	change_node_of_curve(bc2,bc2_orient,oppn1);
	(void) delete_node(n);
	debug_print("2drp","Left delete_null_bdry_curves_at_node\n");
}		/*end delete_null_bdry_curves_at_node*/

/*
*			free_rp_list():
*/

EXPORT void free_rp_list(
	RPROBLEM	**rp)
{
	RPROBLEM	*rp1,*rp2;

	debug_print("2drp","Entered free_rp_list\n");
	if ((rp == NULL) || (*rp == NULL))
	{
	    debug_print("2drp","Left free_rp_list\n");
	    return;
	}
	rp2 = (*rp)->next;
	for (rp1 = *rp; rp1->prev;) 
	{
	    rp1 = rp1->prev;
	    free_rp(rp1->next);
	}
	free_rp(rp1);
	for (; rp2 && rp2->next;) 
	{
	    rp2 = rp2->next;
	    free_rp(rp2->prev);
	}
	if (rp2) free_rp(rp2);
	*rp = NULL;
	debug_print("2drp","Left free_rp_list\n");
}		/*end free_rp_list*/


/*
*			free_rp():
*/

EXPORT void free_rp(
	RPROBLEM	*rp)
{
	RP_NODE		*rp_node;
	RP_NODE		*next_rp_node;
	
	if (!rp)
	    return;
	free_o_curve_family(rp->bdry_curves);
	free_o_curve_family(rp->old_bdry_curves);
	free_o_curve_family(rp->ang_ordered_curves);
	free_o_curve_family(rp->old_ang_ordered_curves);
	for (rp_node = rp->first_rp_node; rp_node; rp_node = next_rp_node) {
		next_rp_node = rp_node->next; //gcw 2/3/15 - read "next" before freeing
	    free_rp_node(rp_node,rp);
	}
	free(rp);
}		/*end free_rp*/

/*ARGSUSED*/
LOCAL void f_free_rp_node(
	RP_NODE		*rpn,
	RPROBLEM	*rp)
{
	if (!rpn)
	    return;
	if (rpn->neumann1)
	    free(rpn->neumann1);
	if (rpn->neumann2)
	    free(rpn->neumann2);
	if (rpn->dirichlet1)
	    free(rpn->dirichlet1);
	if (rpn->dirichlet2)
	    free(rpn->dirichlet2);
	if (rpn->subdomain1)
	    free(rpn->subdomain1);
	if (rpn->subdomain2)
	    free(rpn->subdomain2);

	user_free_rp_node(rpn,rp);
	free(rpn);
}		/*end f_free_rp_node*/

/*ARGSUSED*/
LOCAL void f_user_free_rp_node(
	RP_NODE		*rpn,
	RPROBLEM	*rp)
{
}		/*end f_user_free_rp_node*/

/*ARGSUSED*/
LOCAL void f_user_print_rp_node(
	RP_NODE		*rpn,
	RPROBLEM	*rp)
{
}		/*end f_user_print_rp_node*/

/*
*			free_o_curve_family():
*/

EXPORT void free_o_curve_family(
	O_CURVE_FAMILY	*ocf)
{
	if (!ocf)
	    return;
	if (ocf->first)
	{
	    O_CURVE *oc, *ocnext = NULL;

	    ocf->first->prev = ocf->last->next = NULL;
	    for (oc = ocf->first; oc; oc = ocnext)
	    {
	        ocnext = oc->next;
		free(oc);
	    }
	}
	free(ocf);
}		/*end free_o_curve_family*/



/*
*		delete_null_physical_curves():
*/

EXPORT	void delete_null_physical_curves(
	RPROBLEM	*rp)
{
	CURVE		**c;
	RP_NODE		*rpn;

	debug_print("2drp","Entered delete_null_physical_curves()\n");
	if (debugging("2drp"))
	{
	    (void) printf("Interface before delete_null_physical_curves()\n");
	    print_interface(rp->new_intfc);
	}
	for (rpn = rp->first_rp_node; rpn; rpn = rpn->next)
	{
	    if (is_fixed_node(rpn->node))
		continue;
delete_in_curve:
	    for (c = rpn->node->in_curves; c && *c; c++)
	    {
	    	if (is_null_curve(*c,rp) &&
	    	    wave_type(*c) >= FIRST_PHYSICS_WAVE_TYPE) 
	    	{
	    	    if (debugging("2drp"))
	    		(void) printf("Deleting curve %llu\n",curve_number(*c));
	    	    (void) delete_curve(*c);
	    	    goto delete_in_curve;
	    	}
	    }
delete_out_curve:
	    for (c = rpn->node->out_curves; c && *c; c++)
	    {
	    	if (is_null_curve(*c,rp) &&
	    	    wave_type(*c) >= FIRST_PHYSICS_WAVE_TYPE) 
	    	{
	    	    if (debugging("2drp"))
	    		(void) printf("Deleting curve %llu\n",curve_number(*c));
	    	    (void) delete_curve(*c);
	    	    goto delete_out_curve;
	    	}
	    }
	}
	if (debugging("2drp"))
	{
	    (void) printf("Interface after delete_null_physical_curves()\n");
	    print_interface(rp->new_intfc);
	}
	debug_print("2drp","Left delete_null_physical_curves()\n");
}		/*end delete_null_physical_curves*/


/*
*		reset_component_of_loop():
*/

EXPORT void reset_component_of_loop(
	CURVE		*c,
	ORIENTATION	c_orient,
	ANGLE_DIRECTION	ang_dir,
	COMPONENT	new_comp,
	Front		*front)
{
	INTERFACE	*intfc = c->interface;
	O_CURVE_FAMILY	*loop;
	O_CURVE		*oc;

	debug_print("loop_comp","Entered reset_component_of_loop()\n");
	loop = find_loop(c,c_orient,ang_dir);
	if (debugging("loop_comp"))
	{
	    int i = 0;
	    (void) printf("Curves in loop before components are reset\n");
	    for (oc = loop->first; oc; oc = oc->next)
	    {
	    	(void) printf("Curve[%d]\n",i++);
	    	print_curve(oc->curve);
	    }
	}
	for (oc = loop->first; oc; oc = oc->next)
	{
	    if (curve_ang_oriented_l_to_r(ang_dir,oc->orient))
	    {
	    	set_equivalent_comps(negative_component(oc->curve),
				     new_comp,intfc);
		negative_component(oc->curve) = new_comp;
	    }
	    else
	    {
	    	set_equivalent_comps(positive_component(oc->curve),
				     new_comp,intfc);
		positive_component(oc->curve) = new_comp;
	    }
	}
	if (debugging("loop_comp"))
	{
	    int i = 0;
	    (void) printf("Curves in loop after components are reset\n");
	    for (oc = loop->first; oc; oc = oc->next)
	    {
	    	(void) printf("Curve[%d]\n",i++);
	    	print_curve(oc->curve);
	    }
	}
	SetActiveFlowComponent(new_comp,front);
	free_o_curve_family(loop);
	debug_print("loop_comp","Left reset_component_of_loop()\n");
}		/*end reset_component_of_loop*/

EXPORT void find_corr_cur_in_rp(
	O_CURVE		*c,
	O_CURVE		*corr_c,
	Front		*front,
	RPROBLEM	*rp)
{
	RP_NODE		*rpn;
	NODE		*corr_node;
	INTERFACE	*corr_intfc;

	debug_print("rp_correspond","Entered find_corr_cur_in_rp()\n");
	if (debugging("rp_correspond"))
	{
	    (void) printf("Looking for curve corresponding to curve %llu\n",
		          curve_number(c->curve));
	    print_o_curve(c);
	}
	if (!rp_node_with_node(&rpn,rp,Node_of_o_curve(c)))
	{
	    screen("ERROR in find_corr_cur_in_rp(), Unable to find rp node\n");
	    clean_up(ERROR);
	}
	if (debugging("rp_correspond"))
	{
	    (void) printf("Rp node connected to Node_of_o_curve(c)\n");
	    print_rp_node(rpn,rp);
	}
	if (c->curve->interface == rp->new_intfc)
	{
	    corr_node = rpn->old_node;
	    corr_intfc = rp->old_intfc;
	}
	else if (c->curve->interface == rp->old_intfc)
	{
	    corr_node = rpn->node;
	    corr_intfc = rp->new_intfc;
	}
	else
	{
	    screen("ERROR in find_corr_cur_in_rp(), "
		   "Interface of curve not in rproblem\n");
	    (void) printf("Input curve c = %llu, ",curve_number(c->curve));
	    (void) printf("c->curve->interface = %llu\n",
			  interface_number(c->curve->interface));
	    (void) printf("Interface of c\n");
	    print_interface(c->curve->interface);
	    print_rproblem(rp);
	    (void) printf("Old interface of rp\n");
	    print_interface(rp->old_intfc);
	    (void) printf("New interface of rp\n");
	    print_interface(rp->new_intfc);
	    clean_up(ERROR);
	}
	if (!find_correspond_of_oriented_curve(c,corr_c,corr_node,front,
						  corr_intfc))
	{
	    /* Presumably, the new curve has been untracked for some
	     * reason beyond the scope of this rp.  WARNING, this
	     * may not be the correct thing to do in every case, &&
	     * may cause problems elsewhere. */
	    corr_c->curve = NULL;
	}
	else if (debugging("rp_correspond"))
	{
	    (void) printf("Correspond curve %llu found\n",
			  curve_number(corr_c->curve));
	    print_o_curve(corr_c);
	}
	debug_print("rp_correspond","Left find_corr_cur_in_rp()\n");
}		/*end find_corr_cur_in_rp*/

EXPORT void relocate_null_pointer(
	POINTER		*p1,
	POINTER		*p2)
{
	if (!p1 || !p2)
	    return;
	if (*p2 && !*p1) 
	{
	    *p1 = *p2;
	    *p2 = NULL;
	}
}		/*end relocate_null_pointer*/

/*
*			init_ocurve_lists():
*
*	Initializes the angle order curve lists for the rproblem
*	rp.  Also initializes the boundary curve list that consists
*	of the non-null boundary curves in the rproblem.
*/

EXPORT	void init_ocurve_lists(
	RPROBLEM	*rp,
	Front		*front)
{
	RP_NODE		*rpn;
	O_CURVE		Oldc0, Newc0, Newc;
	O_CURVE		*oc, *noc;

	debug_print("2drp","Entered init_ocurve_lists()\n");

	free_o_curve_family(rp->ang_ordered_curves);
	rp->ang_ordered_curves = NULL;

	free_o_curve_family(rp->old_ang_ordered_curves);
	rp->old_ang_ordered_curves = NULL;

	free_o_curve_family(rp->bdry_curves);
	rp->bdry_curves = NULL;

	free_o_curve_family(rp->old_bdry_curves);
	rp->old_bdry_curves = NULL;

	Oldc0.curve = NULL;	Newc0.curve = NULL;

	for (rpn = rp->first_rp_node; rpn != NULL; rpn = rpn->next)
	{
	    if (non_null_bdry_curve_at_node(rp,rpn->old_node,&Oldc0))
	        break;
	}

	if (Oldc0.curve != NULL)
	{
	    make_ang_ordered_list(rp,&Oldc0,&rp->old_ang_ordered_curves);
	    for (oc=rp->old_ang_ordered_curves->first; oc!=NULL; oc=oc->next)
	    {
	    	find_corr_cur_in_rp(oc,&Newc,front,rp);
	    	init_o_curve(&noc,Newc.curve,Newc.orient);
	    	add_oc_curve_to_family(noc,&rp->ang_ordered_curves);
	    }
	}
	else
	{
	    for (rpn = rp->first_rp_node; rpn != NULL; rpn = rpn->next)
	    {
	    	if (non_null_bdry_curve_at_node(rp,rpn->node,&Newc0))
	    	    break;
	    }
	    make_ang_ordered_list(rp,&Newc0,&rp->ang_ordered_curves);
	}


	/* Make list circular */

	if (rp->old_ang_ordered_curves != NULL)
	{
	    rp->old_ang_ordered_curves->first->prev = 
					rp->old_ang_ordered_curves->last;
	    rp->old_ang_ordered_curves->last->next = 
					rp->old_ang_ordered_curves->first;
	}

	rp->ang_ordered_curves->first->prev = rp->ang_ordered_curves->last;
	rp->ang_ordered_curves->last->next = rp->ang_ordered_curves->first;

	
	if (Oldc0.curve != NULL &&
			wave_type(Oldc0.curve) < FIRST_PHYSICS_WAVE_TYPE)
	{
	    oc = rp->old_ang_ordered_curves->first->next;
	    if (wave_type(oc->curve) < FIRST_PHYSICS_WAVE_TYPE &&
	    	oc != rp->old_ang_ordered_curves->last)
	    {
	    	rp->old_ang_ordered_curves->first = oc;
	    	rp->old_ang_ordered_curves->last = oc->prev;

	    	oc = rp->ang_ordered_curves->first->next;
	    	rp->ang_ordered_curves->first = oc;
	    	rp->ang_ordered_curves->last = oc->prev;
	    }

	    /* Set old boundary curve lists */

	    init_o_curve(&oc,rp->old_ang_ordered_curves->first->curve,
			 rp->old_ang_ordered_curves->first->orient);
	    add_oc_curve_to_family(oc,&rp->old_bdry_curves);
	    init_o_curve(&oc,rp->old_ang_ordered_curves->last->curve,
			 rp->old_ang_ordered_curves->last->orient);
	    add_oc_curve_to_family(oc,&rp->old_bdry_curves);

	    /* Set new boundary curve lists */

	    init_o_curve(&oc,rp->ang_ordered_curves->first->curve,
			 rp->ang_ordered_curves->first->orient);
	    add_oc_curve_to_family(oc,&rp->bdry_curves);
	    init_o_curve(&oc,rp->ang_ordered_curves->last->curve,
	    		 rp->ang_ordered_curves->last->orient);
	    add_oc_curve_to_family(oc,&rp->bdry_curves);
	}
	else if (Newc0.curve != NULL &&
			wave_type(Newc0.curve) < FIRST_PHYSICS_WAVE_TYPE)
	{
	    oc = rp->ang_ordered_curves->first->next;
	    if (wave_type(oc->curve) < FIRST_PHYSICS_WAVE_TYPE &&
	    	oc != rp->ang_ordered_curves->last)
	    {
	    	rp->ang_ordered_curves->first = oc;
	    	rp->ang_ordered_curves->last = oc->prev;
	    }

	    /* Set new boundary curve lists */

	    init_o_curve(&oc,rp->ang_ordered_curves->first->curve,
	    		 rp->ang_ordered_curves->first->orient);
	    add_oc_curve_to_family(oc,&rp->bdry_curves);
	    init_o_curve(&oc,rp->ang_ordered_curves->last->curve,
	    		 rp->ang_ordered_curves->last->orient);
	    add_oc_curve_to_family(oc,&rp->bdry_curves);
	}
	debug_print("2drp","Left init_ocurve_lists()\n");
}		/*end init_ocurve_lists*/

/*
*			non_null_bdry_curve_at_node():
*
*	Tests for the existence of a nonpassive boundary curve
*	at NODE node that is not null with respect to the 
*	RPROBLEM rp.  If such a curve is found, it and its
*	orientation with respect to node are recorded in oc,
*	and the function returns YES.  Otherwise the function
*	returns NO.  If oc->curve is NULL then some non-null
*	physical curve at node will be recorded in oc.
*/

LOCAL	int non_null_bdry_curve_at_node(
	RPROBLEM	*rp,
	NODE		*node,
	O_CURVE		*oc)
{
	CURVE		**c;

	if (node == NULL)
	    return NO;
	for (c = node->in_curves; c && *c; c++)
	{
	    if ((wave_type(*c) == PASSIVE_BOUNDARY) || (is_null_curve(*c,rp)))
		continue;
	    else if (wave_type(*c) < FIRST_PHYSICS_WAVE_TYPE)
	    {
	    	oc->curve = *c;
	    	oc->orient = NEGATIVE_ORIENTATION;
	    	return YES;
	    }
	    else if (oc->curve == NULL)
	    {
	    	oc->curve = *c;
	    	oc->orient = NEGATIVE_ORIENTATION;
	    }
	}
	for (c = node->out_curves; c && *c; c++)
	{
	    if ((wave_type(*c) == PASSIVE_BOUNDARY) || (is_null_curve(*c,rp)))
		continue;
	    else if (wave_type(*c) < FIRST_PHYSICS_WAVE_TYPE)
	    {
	    	oc->curve = *c;
	    	oc->orient = POSITIVE_ORIENTATION;
	    	return YES;
	    }
	    else if (oc->curve == NULL)
	    {
	    	oc->curve = *c;
	    	oc->orient = POSITIVE_ORIENTATION;
	    }
	}
	return NO;
}		/*end non_null_bdry_curve_at_node*/

LOCAL	void make_ang_ordered_list(
	RPROBLEM	*rp,
	O_CURVE		*oc0,
	O_CURVE_FAMILY	**ang_ordered_curves)
{
	O_CURVE		Oc, *oc;

	copy_o_curve(&Oc,oc0);
	do
	{
	    if (is_null_curve(Oc.curve,rp))
	    {
	    	Oc.orient = Opposite_orient(Oc.orient);
	    }
	    else if (wave_type(Oc.curve) != PASSIVE_BOUNDARY)
	    {
	    	init_o_curve(&oc,Oc.curve,Oc.orient);
	    	add_oc_curve_to_family(oc,ang_ordered_curves);
	    }
	    Oc.curve = adjacent_curve(Oc.curve,Oc.orient,
				      COUNTER_CLOCK,&Oc.orient);

	} while (Oc.curve != oc0->curve || Oc.orient != oc0->orient);
}		/*end make_ang_ordered_list*/

EXPORT	void rrpnlist_after_delete_node(
	RPROBLEM	*rp,
	NODE		*node)
{
	RP_NODE		*rpn;

	for (rpn = rp->first_rp_node; rpn != NULL; rpn = rpn->next)
	    if (rpn->node == node)
		rpn->node = NULL;
}		/*end rrpnlist_after_delete_node*/

EXPORT	void roclists_after_split(
	RPROBLEM	*rp,
	CURVE		*curve,
	CURVE		**curves,
	int		add_newn_to_rp)
{
	RP_NODE		*rpn;
	O_CURVE_FAMILY	*ocl;
	O_CURVE		*oc;
	NODE		*newn;

	if (!rp)
	    return;
	ocl = rp->ang_ordered_curves;
	newn = curves[0]->end;
	if (add_newn_to_rp && !rp_node_with_node(&rpn,rp,newn))
	    (void) add_to_rp_node_list(rp,newn,NULL);
	if (ocl && ocl->first)
	{
	    for (oc = ocl->first; oc != NULL; oc = oc->next)
	    {
	    	if (oc->curve == curve)
	    	{
	    	    oc->curve = (oc->orient==POSITIVE_ORIENTATION) ?
	    				curves[1] : curves[0];
	    	}
	    	if (oc == ocl->last)
		    break;
	    }
	}
	if (wave_type(curve) >= FIRST_PHYSICS_WAVE_TYPE)
	    return;
	ocl = rp->bdry_curves;
	if (ocl && ocl->first)
	{
	    /*
	     *  Check if single boundary curve in rproblem splits.
	     *  By construction the ang_ordered_curves list always
	     *  begins and ends at boundary curves if such exist.
	     */

	    if (ocl->first == ocl->last)
	    {
	    	oc = ocl->first;
	    	oc->curve = rp->ang_ordered_curves->first->curve;
	    	oc->orient = rp->ang_ordered_curves->first->orient;
	    	init_o_curve(&oc,rp->ang_ordered_curves->last->curve,
	    		     rp->ang_ordered_curves->last->orient);
	    	add_oc_curve_to_family(oc,&ocl);
	    	return;
	    }

	    for (oc = ocl->first; oc != NULL; oc = oc->next)
	    {
	    	if (oc->curve == curve)
	    	{
	    	    oc->curve = (oc->orient==POSITIVE_ORIENTATION) ?
	    				curves[1] : curves[0];
	    	}
	    	if (oc == ocl->last)
	    	    break;
	    }
	}
}		/*end roclists_after_split*/

EXPORT	void roclists_after_join(
	RPROBLEM	*rp,
	CURVE		*curve1,
	O_CURVE		*oc1,
	CURVE		*curve2,
	O_CURVE		*oc2,
	CURVE		*curve)
{
	O_CURVE_FAMILY	*ocl;
	O_CURVE		*oc, *joc1, *joc2;
	O_CURVE		*tmpoc;

	if (!rp)
	    return;
	ocl = rp->ang_ordered_curves;
	if (ocl && ocl->first)
	{
	    joc1 = joc2 = NULL;
	    for (oc = ocl->first; oc != NULL; oc = oc->next)
	    {
	    	if (oc->curve == curve1)
	    	{
	    	    if (joc1 == NULL)
	    	        joc1 = oc;
	    	    else if (joc2 == NULL)
			joc2 = oc;
	    	    oc->curve = curve;
	    	    if (oc1 && oc1 != oc)
			oc1->curve = oc->curve;
	    	}
	    	if (oc->curve == curve2)
		{
			if (joc1 == NULL)
			    joc1 = oc;
			else if (joc2 == NULL)
			    joc2 = oc;
			oc->curve = curve;
			if (oc2 && oc2 != oc)
			    oc2->curve = oc->curve;
		}
		if (oc == ocl->last)
		    break;
	    }
	    if (joc2 != NULL)
	    {
	    	delete_oc_curve_from_family(&joc2,&ocl);
	    }
	}

	ocl = rp->bdry_curves;
	if (ocl && ocl->first) 
	{
	    for (oc = ocl->first; oc != NULL; oc = oc->next)
	    {
	    	if (oc->curve == curve1)
	    	{
	    	    oc->curve = curve;
	    	    if (oc1 && oc1 != oc)
			oc1->curve = oc->curve;
	    	}
	    	if (oc->curve == curve2)
	    	{
	    	    oc->curve = curve;
	    	    if (oc2 && oc2 != oc)
			oc2->curve = oc->curve;
	    	}
	    	if (oc == ocl->last)
		    break;
	    }

	    /* Check if all boundary curves are merged
	     * Question, Should the boundary ocurve list be
	     * removed if all boundary curves are merged?
	     */

	    if (ocl->first->curve == ocl->last->curve)
	    {
	    	tmpoc = ocl->last;
	    	delete_oc_curve_from_family(&tmpoc,&ocl);
	    }
	}
}		/*end roclists_after_join*/


EXPORT	void roclists_after_invert(
	RPROBLEM	*rp,
	CURVE		*curve,
	O_CURVE		*ocurve)
{
	O_CURVE_FAMILY	*ocl;
	O_CURVE		*oc;

	if (!rp)
	    return;
	ocl = rp->ang_ordered_curves;
	if (ocl && ocl->first)
	{
	    for (oc = ocl->first; oc != NULL; oc = oc->next)
	    {
	    	if (oc->curve == curve)
	    	{
	    	    oc->orient = Opposite_orient(oc->orient);
	    	    if (ocurve && ocurve != oc)
	    	    	ocurve->orient = oc->orient;
	    	}
	    	if (oc == ocl->last)
	    	    break;
	    }
	}

	ocl = rp->bdry_curves;
	if (ocl && ocl->first)
	{
	    for (oc = ocl->first; oc != NULL; oc = oc->next)
	    {
	    	if (oc->curve == curve)
	    	{
	    	    oc->orient = Opposite_orient(oc->orient);
	    	    if (ocurve && ocurve != oc)
	    		ocurve->orient = oc->orient;
	    	}
	    	if (oc == ocl->last)
	    	    break;
	    }
	}
}		/*end roclists_after_invert*/

LOCAL	void roclists_after_delete(
	RPROBLEM	*rp,
	CURVE		*curve)
{
	if (!rp)
	    return;

	delete_curve_from_o_curve_family(curve,&rp->ang_ordered_curves);
	delete_curve_from_o_curve_family(curve,&rp->bdry_curves);
}		/*end roclists_after_delete*/


EXPORT	boolean f_untrack_curve(
	O_CURVE		*oc,
	O_CURVE		*oldoc,
	COMPONENT	newcomp,
	double		dt,
	Front		*fr,
	POINTER		wave,
	RPROBLEM	*rp,
	UNTRACK_FLAG	flag)
{
	COMPONENT	lcomp, rcomp;
	CURVE		**c;
	INTERFACE	*intfc = oc->curve->interface;
	NODE		*node = Node_of_o_curve(oc);
	NODE		*oppn = Opp_node_of_o_curve(oc);
	ORIENTATION	orient = oc->orient;
	ORIENTATION	opp_or = Opposite_orient(orient);
	int		num_in, num_out, num_total;
	boolean		LisFSR, RisFSR;
	boolean		sav_intrp = interpolate_intfc_states(intfc);

	debug_print("untrack","Entered f_untrack_curve()\n");

	lcomp = negative_component(oc->curve);
	rcomp = positive_component(oc->curve);
	LisFSR = ComponentIsFlowSpecified(lcomp,fr);
	RisFSR = ComponentIsFlowSpecified(rcomp,fr);
	/*
	 * A flow specified component adjacent to an active flow component must
	 * be set to active if the intervening curve is untracked.
	 */
	if ((LisFSR == YES) && (RisFSR == NO))
	{
	    SetActiveFlowComponent(lcomp,fr);
	    (void) printf("WARNING in f_untrack_curve(), "
			  "COMPONENT %d set to active.\n",lcomp);
	}
	if ((LisFSR == NO) && (RisFSR == YES))
	{
	    SetActiveFlowComponent(rcomp,fr);
	    (void) printf("WARNING in f_untrack_curve(), "
			  "COMPONENT %d set to active.\n",rcomp);
	}
	if (lcomp != rcomp)
	{
	    set_equivalent_comps(lcomp,rcomp,intfc);
	    reset_component_of_loop(oc->curve,orient,CLOCKWISE,newcomp,fr);
	    reset_component_of_loop(oc->curve,orient,COUNTER_CLOCK,newcomp,fr);
	}

	(void) delete_curve(oc->curve);
	for (c = intfc->curves; c && *c; c++)
	{
	    if (negative_component((*c)) == lcomp)
	    	negative_component((*c)) = newcomp;
	    if (negative_component((*c)) == rcomp)
	    	negative_component((*c)) = newcomp;
	    if (positive_component((*c)) == lcomp)
	    	positive_component((*c)) = newcomp;
	    if (positive_component((*c)) == rcomp)
	    	positive_component((*c)) = newcomp;
	}
	roclists_after_delete(rp,oc->curve);
	num_total = num_curves_at_node(node,&num_in,&num_out);
	interpolate_intfc_states(intfc) = YES;
	if ((states_set_at_node(flag,orient) == NO) && (num_total == 2) && 
			(oldoc != NULL))
	{
	    init_redundant_node_for_deletion(node,Node_of_o_curve(oldoc),
					     fr,wave,dt);
	}
	(void) delete_redundant_node(node,NULL,rp,fr);

	num_total = num_curves_at_node(oppn,&num_in,&num_out);
	if ((states_set_at_node(flag,opp_or) == NO) && (num_total == 2) &&
			(oldoc != NULL))
	{
	    init_redundant_node_for_deletion(oppn,Opp_node_of_o_curve(oldoc),
					     fr,wave,dt);
	}
	(void) delete_redundant_node(oppn,NULL,rp,fr);

	interpolate_intfc_states(intfc) = sav_intrp;

	if (debugging("untrack"))
	{
	    (void) printf("Interface at end of f_untrack_curve()\n");
	    print_interface(intfc);
	}
	debug_print("untrack","Left f_untrack_curve()\n");
	return YES;
}		/*end f_untrack_curve*/

/*
*		set_states_at_node_by_propagate():
*
*	Sets the states at the node of the O_CURVE newc by propagating
*	the point at the node of O_CURVE oldc.
*/

EXPORT  void set_states_at_node_by_propagate(
	Front		*front,
	POINTER		wave,
	O_CURVE		*oldc,
	O_CURVE		*newc,
	double		dt)
{
	FT_POINT		*ptmp;
	double		V[MAXD];
	size_t		sizest = front->sizest;
 
	if (oldc == NULL || wave == NULL || front == NULL)
	    return;

 	ptmp = Node_of_o_curve(newc)->posn;
 	point_propagate(front,wave,Node_of_o_curve(oldc)->posn,ptmp,
 			Bond_at_node_of_o_curve(oldc),oldc->curve,dt,V);
 	if (newc->orient != oldc->orient)
	    reverse_states_at_point(ptmp,front);
 	ft_assign(Left_state_at_node_of_o_curve(newc),left_state(ptmp),sizest);
 	clear_state(front->interf,left_state(ptmp),sizest);
 	ft_assign(Right_state_at_node_of_o_curve(newc),right_state(ptmp),sizest);
 	clear_state(front->interf,right_state(ptmp),sizest);
}		/*end set_states_at_node_by_propagate*/


/*
*			f_2drproblem():
*
*	Recognized return statuses are
*		ERROR_IN_STEP, GOOD_STEP, MODIFY_TIME_STEP
*/

EXPORT int f_2drproblem(
	Front		*front,
	Front		*newfront,
	POINTER		p2wave,
	RPROBLEM	**prp)
{
	RPROBLEM	*rp = *prp;
	int		status;

	DEBUG_ENTER(f_2drproblem)
	if (DEBUG)
	{
	    (void) printf("Rproblem into f_2drproblem()\n");
	    print_rproblem(rp);
	}
	(*front->init_2drproblem)(rp,front);
	if (is_bdry_type(rp)) 
	{
	    if (DEBUG)
	    {
	    	(void) printf("\tBdry_type Riemann Problem:\n");
	    	f_print_rp_statistical_data(rp);
	    }
	    if ((rp->bdry_type1 == SUBDOMAIN_BOUNDARY) ||
	    	rp->bdry_type2 == SUBDOMAIN_BOUNDARY)
	    {
	    	RP_NODE *rp_n;

	    	if (  (rp->bdry_type1 == DIRICHLET_BOUNDARY ||
	    	         rp->bdry_type1 == NEUMANN_BOUNDARY)
		     ||
		    ( (rp->bdry_type2 == DIRICHLET_BOUNDARY ||
	    	       rp->bdry_type2 == NEUMANN_BOUNDARY)))
	    	{
	    	    status = pp_curve_exits_at_bdry(front,newfront,p2wave,prp);
	    	}
	    	else
	    	{
	    	    if (DEBUG)
	    	    {
	    	      (void) printf("WARNING in f_2drproblem(), "
	    	                    "interaction involving subdomain "
	    	                    "boundaries\n\tCalling "
	    	                    "set_node_states_and_continue()\n");
	    	    }
	    	    for (rp_n = rp->first_rp_node;
	    	    	 rp_n != NULL; rp_n = rp_n->next)
	    	    {
	    	        if (!set_node_states_and_continue(rp_n->old_node,
							     rp_n->node,front))
	    	        {
	    	    	    DEBUG_LEAVE(f_2drproblem)
	    	    	    return ERROR_IN_STEP;
	    	        }
	    	        rp_n->states_assigned_at_node = YES;
	    	    }
	    	    status = GOOD_STEP;
	        }
	    }
	    else if (rp->num_phys == 1 && 
	    	     rp->bdry_type1 == DIRICHLET_BOUNDARY && 
		     rp->bdry_type2 == DIRICHLET_BOUNDARY)
	    {
		NODE_FLAG flag;
		clear_node_flag(flag);
	    	status = phys_node_crosses_bdry(front,newfront,p2wave,rp,flag);
	    }
	    else if (rp_num_incident(rp) == 0 && rp->num_fxd == 2)
	    {
	    	status = curve_exits_parallel_to_bdry(front,p2wave,rp);
	    }
	    else if (rp_num_incident(rp) == 0)
	    {
		delete_null_physical_curves(rp);
		delete_null_boundary_curves(rp,front,p2wave);
	    }
	    else if (rp_num_incident(rp) == 1 && rp->num_fxd == 0)
	    {
	    	status = bdry_rp_1i_0f(front,p2wave,rp);
	    }
	    else if (rp_num_incident(rp) >= 1 && rp->num_fxd == 1)
	    {
	    	status = incident_curve_crosses_fixed_node(front,newfront,
							   p2wave,rp);
	    }
	    else if (rp_num_incident(rp) == 2)
	    {
	    	status = bdry_rp_2i(front,newfront,p2wave,rp);
	    }
	    else 
	    {
	        print_rproblem(rp);
	        (void) printf("WARNING: unexpected case in f_2drproblem\n");
	        DEBUG_LEAVE(f_2drproblem)
	        return ERROR_IN_STEP;
	    }
	}
	if (is_bdry_type(rp)) 
	DEBUG_LEAVE(f_2drproblem)
	return status;
}		/*end f_2drproblem*/


/*
*			f_init_2drproblem():
*/

EXPORT void f_init_2drproblem(
	RPROBLEM	*rp,
	Front		*front)
{
	DEBUG_ENTER(g_init_2drproblem)
	f_init_rp_nodes(rp);
	init_ocurve_lists(rp,front);
	f_init_physical_ocurves(rp);
	f_set_rp_statistics(rp);
	if (DEBUG)
	{
	    (void) printf("Initialized rproblem %p\n",(POINTER)rp);
	    print_rproblem(rp);
	}
	DEBUG_LEAVE(f_init_2drproblem)
}		/*end g_init_2drproblem*/


/*
*			f_init_physical_ocurves():
*
*	Initializes the physical curves associated with a given RPROBLEM.
*/

LOCAL void f_init_physical_ocurves(
	RPROBLEM	*rp)
{
	RP_NODE		*rp_node;
	CURVE		*c1,*c2;
	ORIENTATION	orient1,orient2;

	DEBUG_ENTER(f_init_physical_ocurves)

		/* Initialize physical curves */

	for (rp_node = rp->first_rp_node; rp_node; rp_node = rp_node->next) 
	{
	    free_o_curve_family(rpn_reflected1(rp_node)); 
	    rpn_reflected1(rp_node) = NULL;
	    free_o_curve_family(rpn_reflected2(rp_node)); 
	    rpn_reflected2(rp_node) = NULL;

	    find_curves_with_status(rp_node->node,&c1,&orient1,
	    			    &c2,&orient2,INCIDENT);

	    find_curves_with_status(rp_node->node,&c1,&orient1,
	    			    &c2,&orient2,REFLECTED);
	    if (c1)
		init_cfamily(&rpn_reflected1(rp_node),c1,orient1);
	    if (c2)
		init_cfamily(&rpn_reflected2(rp_node),c2,orient2);
	}

		/* Replace null curves by continuations */

	for (rp_node = rp->first_rp_node; rp_node; rp_node = rp_node->next) 
	{
	    f_replace_null_curves_in_family(&rpn_reflected1(rp_node),rp);
	    f_replace_null_curves_in_family(&rpn_reflected2(rp_node),rp);

	    	/* Null curves and families prefered in location 2 */

	    relocate_null_pointer((POINTER *)&rpn_reflected1(rp_node),
			          (POINTER *)&rpn_reflected2(rp_node));
	}
	DEBUG_LEAVE(f_init_physical_ocurves)
}		/*end f_init_physical_ocurves*/


LOCAL void f_replace_null_curves_in_family(
	O_CURVE_FAMILY	**cfamily,
	RPROBLEM	*rp)
{
	O_CURVE		*oc;
	O_CURVE		Baseoc;

	DEBUG_ENTER(f_replace_null_curves_in_family)
		/* Test for null curves */
	
	if (!*cfamily)
	{
	    DEBUG_LEAVE(f_replace_null_curves_in_family)
	    return;
	}

	Baseoc.curve = NULL;
testnull:
	for (oc = (*cfamily)->first; oc; oc = oc->next)
	    if (is_null_curve(oc->curve,rp)) break;

	if (!oc)
	{
	    DEBUG_LEAVE(f_replace_null_curves_in_family)
	    return;
	}

	if (DEBUG)
	{
	    (void) printf("Null curve found\n");
	    print_curve(oc->curve);
	}

	if (Baseoc.curve == NULL) Baseoc = *oc;

	    /* Replace null curves */

	oc->orient = Opposite_orient(oc->orient);
	switch(node_type(Node_of(oc->curve,oc->orient))) 
	{
	case PASSIVE_NODE:
	case DIRICHLET_NODE:
	case NEUMANN_NODE:
	case FIXED_NODE:
	case SUBDOMAIN_NODE:
	    delete_oc_curve_from_family(&oc,cfamily);
	    break;
	}
	if (*cfamily) goto testnull;
	DEBUG_LEAVE(f_replace_null_curves_in_family)
}		/*end f_replace_null_curves_in_family*/


LOCAL	void f_print_rp_statistical_data(
	RPROBLEM	*rp)
{
	DEBUG_ENTER(f_print_rp_statistical_data)

	(void) printf("\nStatistical Data for rproblem %p\n",(POINTER)rp);
	(void) printf("number of incident curves = %d\n",rp_num_incident(rp));
	(void) printf("number of Neumann and Dirichlet = %d\n",rp->num_nd);
	(void) printf("number of fixed nodes = %d\n",rp->num_fxd);
	(void) printf("number of boundary nodes = %d\n",rp->num_bdry_nod);
	(void) printf("number of physical nodes = %d\n",rp->num_phys);
	(void) printf("total number of nodes = %d\n",rp->num_nod);
	print_wave_type("bdry_type1 = ",rp->bdry_type1,"\n",rp->new_intfc);
	print_wave_type("bdry_type2 = ",rp->bdry_type2,"\n",rp->new_intfc);
	(void) printf("End of Statistical Data for rproblem %p\n\n",
		      (POINTER)rp);
	DEBUG_LEAVE(f_print_rp_statistical_data)
}		/*end f_print_rp_statistical_data*/
#endif /* defined(TWOD) */
