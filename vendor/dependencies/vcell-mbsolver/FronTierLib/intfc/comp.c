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
*				comp.c:
*
*
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*			CODE FOR COMPONENT COMPUTATIONS:
*
*		The main user routines for COMPONENT computation is
*	component()  which determines the COMPONENT at a point relative
*	to an INTERFACE.
*
*		Most of the COMPONENT code is related to making these
*	operations as efficient as possible when called many times on
*	the same INTERFACE.   This is done by constructing local lists
*	of BONDS and CURVES which intersect each grid block of a
*	rectangular grid.   The cost of the above operations then is
*	related only to the size of the local lists rather than to the
*	number of BONDS in the INTERFACE.   
*
*		The underlying rectangular grid may either be specified,
*	or if not, the code chooses the grid itself - it selects the
*	smallest rectangle that contains all POINTS of the INTERFACE
*	and imposes a grid on that.   To specify the underlying topological
*	grid for an INTERFACE, call the routine set_topological_grid().
*
*		Whenever component() is called and either the underlying
*	grid for the INTERFACE has been changed, or the INTERFACE itself
*	has been modified, then the code automatically reconstructs the
*	bond/comp lists.   This can also be done by calling the routine 
*	make_bond_comp_lists().
*
*		In addition to the above routines, various other routines
*	are supplied for accessing BOND and COMPONENT information relative
*	to an inderlying grid.   We give here the calling sequence for
*	all of these functions.   
*
*		
*
*
*		COMPONENT component(coords,intfc)
*		double *coords;
*		INTERFACE *intfc;
*
*	Returns the COMPONENT relative to INTERFACE intfc at point coords.
*
*
*		void set_topological_grid(intfc,grid)
*		INTERFACE *intfc;
*		RECT_GRID *grid;
*
*	Sets an underlying regular rectangular grid for intfc.   If
*	grid is NULL, the code chooses the grid itself.   Otherwise
*	the RECT_GRID pointed to by grid is used.   Note that the
*	RECT_GRID of an INTERFACE is copied by  copy_interface().
*	Thus it is rarely necessary to specify the underlying grid
*	except at the start.
*
*
*		boolean make_bond_comp_lists(intfc)
*		INTERFACE *intfc;
*
*	Creates lists of BONDS, CURVES and COMPONENTS passing through
*	each grid block of the underlying grid for intfc.   If there
*	is no underlying grid, one will be created first.   This routine
*	is rarely needed since it is called automatically by either
*	component()  or  nearest_interface_point()  after any change to an
*	INTERFACE (or its grid).   The various lists created are accessed via
*	the following set of functions:
*
*
*
*
*		COMPONENT max_component(intfc)
*		INTERFACE *intfc;
*
*	Returns the highest COMPONENT value for intfc.
*
*
*
*
*		COMPONENT min_component(intfc)
*		INTERFACE *intfc;
*
*	Returns the lowest COMPONENT value for intfc.
*
*		COMPONENT exterior_component(intfc)
*		INTERFACE *intfc;
*
*	Returns the COMPONENT value of the exterior of intfc.
*
*		int number_of_labeled_components(intfc)
*		INTERFACE *intfc;
*
*	Returns the ordered array of COMPONENT values intersecting the 
*	ONFRONT grid-block  icoords.
*
*/




#include <intfc/iloc.h>


/* LOCAL Function prototypes */


/*
*				component():
*
*	See comments concerning component2d() and component3d() 
*	in files comp2d.c and comp3d.c.
*
*/

/*ARGSUSED*/
EXPORT COMPONENT component(
	double		*coords,
	INTERFACE	*intfc)
{
	switch(intfc->dim)
	{
	case 1:
	    return component1d(coords,intfc);
	case 2:
	    return component2d(coords,intfc);
	case 3:
	    return component3d(coords,intfc);
	}
	return NO_COMP;
}		/*end component*/

EXPORT	COMPONENT nearest_interior_comp(
	boolean		multiple_comps,
	COMPONENT	dflt_comp,
	double		*coords,
	INTERFACE	*intfc)
{
	COMPONENT	comp;
	double		t[MAXD], coords_on[MAXD];
	HYPER_SURF	*hs;
	HYPER_SURF_ELEMENT *hse;

	if ((multiple_comps == NO) || (intfc->hss == NULL))
	    return dflt_comp;

	comp = component(coords,intfc);
	if (!is_exterior_comp(comp,intfc))
	    return comp;

	if (nearest_interface_point(coords,comp,intfc,INCLUDE_BOUNDARIES,NULL,
				    coords_on,t,&hse,&hs) != YES)
	{
	    screen("ERROR in nearest_interior_comp(), "
		   "nearest_interface_point() failed\n");
	    clean_up(ERROR);
	}
	return (is_exterior_comp(negative_component(hs),intfc)) ?
			positive_component(hs) :
			negative_component(hs);
}		/*end nearest_interior_comp*/

/*
*				long_component():
*
*	See comments concerning long_component2d() and long_component3d() 
*	in files comp2d.c and comp3d.c.
*
*/

/*ARGSUSED*/
EXPORT COMPONENT long_component(
	double		*coords,
	INTERFACE	*intfc)
{
	switch(intfc->dim)
	{
	case 2:
	    return long_component2d(coords,intfc);
	case 3:
	    return long_component3d(coords,intfc);
	}
	return NO_COMP;
}		/*end long_component*/


EXPORT void show_COMP(
	FILE		*file,
	INTERFACE	*intfc)
{
	if (intfc->modified)
	    make_interface_topology_lists(intfc);

	switch (intfc->dim)
	{
	case 1:
	    show_COMP_1d(file,intfc);
	    break;
	case 2:
	    show_COMP_2d(file,intfc);
	    break;
	case 3:
	    show_COMP_3d(file,intfc);
	    break;
	}
}		/*end show_COMP*/


/*
*			new_component():
*
*	Updates the max_component and min_component fields of intfc due
*	to the inclusion of a new component.
*
*	IMPORTANT NOTE ON PARALLELISM
*
*	The function new_component() requires a synchronization of all
*	processors.  This step is needed to ensure the global uniqueness
*	and compatibility of the component values.
*
*	THIS IS NOT CURRENTLY IMPLEMENTED AND IS A MAJOR REQUIREMENT
*	BEFORE ANY DYNAMIC MODIFICIATIONS OF PARALLEL INTERFACES IS
*	POSSIBLE.
*/

EXPORT COMPONENT new_component(
	COMPONENT	comp)
{
	COMPONENT  mincomp, maxcomp;
	HYPER_SURF **hs;
	INTERFACE    *intfc;
	struct Table *firstIT, *T;
	static COMPONENT last_reused_comp = NO_COMP;

	if (comp != NO_COMP)
	{
	    firstIT = interface_table_list();
	    switch (comp)
	    {
	    case UNUSED_COMP:
		maxcomp = INT_MIN;	mincomp = INT_MAX;
	        for (T = firstIT; T != NULL; T = T->next)
		{
		    intfc = T->interface;
		    maxcomp = max(maxcomp,max_component(intfc));
		    mincomp = min(mincomp,min_component(intfc));
		}
		++maxcomp;
		for (comp = mincomp; comp <= maxcomp; ++comp)
		{
		    if (comp == last_reused_comp)
		        continue;
		    for (T = firstIT; T != NULL; T = T->next)
		    {
		        intfc = T->interface;
		        if (is_exterior_comp(comp,intfc) ||
			    is_excluded_comp(comp,intfc))
			    break;
		        if (intfc->hss != NULL)
			{
		            for (hs = intfc->hss; *hs; ++hs)
		            {
		                if ((positive_component(*hs) == comp) ||
			            (negative_component(*hs) == comp))
			            break;
		            }
			    if (*hs)
			        break;
			}
		    }
		    if (T == NULL)
		        break;
		}
		last_reused_comp = comp;
		break;
	    case NEW_COMP:
		comp = INT_MIN;
	        for (T = firstIT; T != NULL; T = T->next)
		{
		    intfc = T->interface;
		    comp = max(comp,max_component(intfc));
		}
		++comp;
	        last_reused_comp = NO_COMP;
		break;
	    default:
	        last_reused_comp = NO_COMP;
		break;
	    }
	    for (T = firstIT; T != NULL; T = T->next)
	    {
		intfc = T->interface;
	        max_component(intfc) = max(comp,max_component(intfc));
	        min_component(intfc) = min(comp,min_component(intfc));
	    }
	}
	return comp;
}		/*end new_component*/

EXPORT	boolean	is_excluded_comp(
	COMPONENT comp,
	INTERFACE* intfc)
{
	if (intfc == NULL)
	    return NO;
	return (*excluded_comps(intfc)._is_comp_in_list)(comp,
					                 &excluded_comps(intfc),
							 intfc);
}		/*end is_excluded_comp*/

EXPORT	void	exclude_comp(
	COMPONENT comp,
	INTERFACE* intfc)
{
	if (intfc == NULL)
	    return;
	(*excluded_comps(intfc)._add_comp_to_list)(comp,
					&excluded_comps(intfc),intfc);
}		/*end exclude_comp*/


/*
*			set_topological_grid():
*
*	Sets the underlying topological rectangular grid for an INTERFACE.
*	If grid is NULL, chooses  the smallest rectangle which
*	contains all POINTS in INTERFACE and then imposes a
*	regular DEFAULT_GMAX by DEFAULT_GMAX by DEFAULT_GMAX grid
*	on this rectangle.
*/


EXPORT void set_topological_grid(
	INTERFACE *intfc,
	RECT_GRID *input_grid)
{
	enum { DEFAULT_GMAX = 20 };
	HYPER_SURF	*hs;
	HYPER_SURF_ELEMENT *hse;
	FT_POINT		*p;
	RECT_GRID	*top_grid = &topological_grid(intfc);
	double		*L, *U;
	int		dim = intfc->dim;
	int		i;
	static int	dgmax[3] = {DEFAULT_GMAX, DEFAULT_GMAX, DEFAULT_GMAX};

	if (DEBUG)
	    (void) printf("\n\nEntered set_topological_grid()\n");
	intfc->table->new_grid = YES;
	if (input_grid != NULL)
	{
	    copy_rect_grid(top_grid,input_grid);
	    intfc->table->fixed_grid = YES;
	    if (DEBUG)
		(void) printf("Left set_topological_grid()\n\n");
	    return;
	}
	else
	    intfc->table->fixed_grid = NO;

			/* Find Rectangle Containing INTERFACE: */
	L = top_grid->L;
	U = top_grid->U;
	for (i = 0; i < dim; ++i)
	{
	    L[i] = HUGE_VAL;
	    U[i] = -HUGE_VAL;
	}

	(void) next_point(intfc,NULL,NULL,NULL);
	while (next_point(intfc,&p,&hse,&hs))
	{
	    for (i = 0; i < dim; ++i)
	    {
	    	L[i] = min(L[i],Coords(p)[i]);
	    	U[i] = max(U[i],Coords(p)[i]);
	    }
	}


	if (DEBUG) 
	{
	    (void) printf("Rectsolid: ");
	    print_general_vector("L = ",L,dim,"");
	    print_general_vector("U = ",L,dim,"\n");
	}

	set_rect_grid(L,U,L,U,NOBUF,NOBUF,dgmax,dim,remap_info(),top_grid);

	if (DEBUG)
	    (void) printf("Left set_topological_grid()\n\n");
	return;
}		/*end set_toplogical_grid*/


/*
*			check_comps_at_nodes():
*
*	Loops over all nodes on the interface intfc to check that
*	the components at each node are locally consistent.
*	Returns 0 if no inconsistent components are found.
*	Otherwise the numbers of nodes with inconsistent components
*	is returned together with a doubly linked list of the
*	nodes with the locally inconsistent components.
*/

EXPORT int check_comps_at_nodes(
	INTERFACE	*intfc,
	O_NODE		**onode_list)
{
	NODE		**n;
	O_NODE		O_node;
	O_NODE		*onode, *on;
	COMPONENT	compi, compj;
	int		i, j;
	int		num_inconsistent = 0;

	debug_print("ccn","Entered check_comps_at_nodes()\n");
	O_node.prev = O_node.next = NULL;
	on = &O_node;
	
	if (intfc->dim != 2)
	    return 0;
	for (n = intfc->nodes; n && *n; ++n)
	{
	    onode = make_onode(*n);
	    for (i = 0; i < onode->num_c; ++i)
	    {
	    	j = (i + 1) % onode->num_c;
	    	if (onode->orient[i] == POSITIVE_ORIENTATION)
	    	    compi = negative_component(onode->nc[i]);
	    	else
	    	    compi = positive_component(onode->nc[i]);
	    	if (onode->orient[j] == POSITIVE_ORIENTATION)
	    	    compj = positive_component(onode->nc[j]);
	    	else
	    	    compj = negative_component(onode->nc[j]);
	    	
	    	if (compi != compj)
	    	{
		    if (debugging("inconsis"))
		    {
		    	char xname[100];
			double radius = 3.0*topological_grid(intfc).h[0];
			print_node(*n);
		    	sprintf(xname,"inconsis_comp-%d-%d",pp_mynode(),
					num_inconsistent);
			xgraph_2d_intfc_within_range(xname,intfc,
					Coords((*n)->posn),radius,NO);
		    }
	    	    ++num_inconsistent;
	    	    on->next = onode;
	    	    onode->prev = on;
	    	    on = onode;
	    	    break;
	    	}
	    }
	}
	if (onode_list!= NULL)
	{
	    *onode_list = O_node.next;
	    if (*onode_list)
	    	(*onode_list)->prev = NULL;
	}
	if ((num_inconsistent > 0) && debugging("ccn"))
	{
	    (void) printf("Inconsistent components found at nodes\n");
	    for (onode = *onode_list; onode != NULL; onode = onode->next)
	    	print_onode(onode);
	    print_interface(intfc);
	}
	debug_print("ccn","Left check_comps_at_nodes(), num_inconsistent = %d\n",
	      num_inconsistent);
	return num_inconsistent;
}		/*end check_comps_at_node*/


EXPORT int comps_consistent_at_node(
	NODE		*node)
{
    	INTERFACE     *intfc =  node->interface;
	COMPONENT     compi, compj;
	CURVE	      **c;
	int	      i, j;
	int	      ans;
	static O_NODE On;
	static int    alloc_num_c = 0;

	/* Don't check for consistency on subdomain boundaries */
	if (is_pp_node(node))
	    return YES;
	ans = YES;
	On.node = node;
	On.num_c = 0;
	On.prev = On.next = NULL;
	for (c = node->in_curves; c && *c; ++c)
	    ++On.num_c;
	for (c = node->out_curves; c && *c; ++c)
	    ++On.num_c;
	if (On.num_c == 0)
	    return YES;
	
	if ((alloc_num_c == 0) || (alloc_num_c < On.num_c))
	{
	    if (alloc_num_c > 0)
	        free_these(5,On.nc,On.nopp,On.pt,On.ang,On.orient);
	    uni_array(&On.nc,On.num_c,sizeof(CURVE *));
	    uni_array(&On.nopp,On.num_c,sizeof(NODE *));
	    uni_array(&On.pt,On.num_c,sizeof(FT_POINT *));
	    uni_array(&On.ang,On.num_c,FLOAT);
	    uni_array(&On.orient,On.num_c,INT);
	    alloc_num_c = On.num_c;
	}

	set_curves_at_onode(&On);
	for (i = 0; i < On.num_c; ++i)
	{
	    j = (i + 1) % On.num_c;
	    if (On.orient[i] == POSITIVE_ORIENTATION)
	    	compi = negative_component(On.nc[i]);
	    else
	    	compi = positive_component(On.nc[i]);
	    if (On.orient[j] == POSITIVE_ORIENTATION)
	    	compj = positive_component(On.nc[j]);
	    else
	    	compj = negative_component(On.nc[j]);
		
	    if (!equivalent_comps(compi,compj,intfc))
	    {
	    	ans = NO;
	    	break;
	    }
	}
	return ans;
}		/*end comps_consistent_at_node*/
