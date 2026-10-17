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
*			iecomps.c
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	All structures and functions in this file deal with equivalence
*	classes of components.  This file is more or less independent from
*	the rest of the front tracking code.
*
*	In order to make the memory allocation for new lists slightly more
*	efficient, they are allocated in lengths which are multiples of
*	the constant EQUIV_COMPS_LEN.  This leads to the need to compute
*	the number of such blocks we have or need for a given number of
*	elements.  The formula is as follows (using integer division):
*		n = (n_equiv/EQUIV_COMPS_LEN + 1);
*/

#include <intfc/iloc.h>

#define	E_comps(intfc)		((EQUIV_COMPS *) (intfc)->e_comps)


        /* LOCAL Function Declarations */
LOCAL  EQUIV_COMPS     *append_to_comp_equiv_list(COMPONENT,EQUIV_COMPS*);
LOCAL  EQUIV_COMPS     *new_equiv_comps(void);
LOCAL  COMP_EQUIV_LIST in_comp_equiv_list(COMPONENT,COMPONENT, EQUIV_COMPS*);
LOCAL  void     merge_comp_equiv_lists(EQUIV_COMPS*,EQUIV_COMPS**,COMPONENT);



/*
*			set_equivalent_comps():
*
*	Takes a pair of components and puts them in the same equivalence
*	class in the intfc->e_comps list.  If there are no lists, a new one
*	is created containing comp1 and comp2.  If lists already exist, they
*	are parsed to see if one of the components is already contained in
*	some equivalence class.  If so, the other comp is appended to that
*	list.  If after this append we find the other comp again, we merge
*	the two classes.  If neither comp appears in any class, we create
*	a new class and append this to the e_comps list.
*/

EXPORT void set_equivalent_comps(
	COMPONENT	comp1,
	COMPONENT	comp2,
	INTERFACE	*intfc)
{
	EQUIV_COMPS	*e_comps, *e_class = NULL;

	debug_print("equiv_comp","Entering set_equivalent_comps()\n");
	if (debugging("equiv_comp"))
	    (void) printf("comp1 = %d, comp2 = %d\n",comp1, comp2);

	if (comp1 == comp2)
	{
	    debug_print("equiv_comp","Leaving set_equivalent_comps()\n");
	    return;
	}

	if (E_comps(intfc) == NULL)
	{
	    e_comps = new_equiv_comps();

	    (void) append_to_comp_equiv_list(comp1,e_comps);
	    (void) append_to_comp_equiv_list(comp2,e_comps);

	    intfc->e_comps = (POINTER) e_comps;

	    if (debugging("equiv_comp"))
	    	fprint_comp_equiv_lists(stdout,intfc);
	    debug_print("equiv_comp","Leaving set_equivalent_comps()\n");
	    return;
	}

	/* e_class stores the e_comps structure when one comp has already been
	 * found.  If the second is then found in another list, this is the
	 * signal to merge. */

	for (e_comps = E_comps(intfc); ; e_comps = e_comps->next)
	{
	    switch (in_comp_equiv_list(comp1,comp2,e_comps))
	    {
	    case NEITHER_FOUND:
	        break;
	    case COMP1_FOUND:
	        if (e_class == NULL)
	            e_class = append_to_comp_equiv_list(comp2,e_comps);
	        else 
	            merge_comp_equiv_lists(e_class,&e_comps,comp1);
	        break;
	    case COMP2_FOUND:
	        if (e_class == NULL)
	            e_class = append_to_comp_equiv_list(comp1,e_comps);
	        else 
	            merge_comp_equiv_lists(e_class,&e_comps,comp2);
	        break;
	    case BOTH_FOUND:
	        debug_print("equiv_comp","Leaving set_equivalent_comps()\n");
	        return;
	    }
		
	    /* The loop is structured this way so we can hang onto the last
	     * e_comps and append a new structure when neither comp is
	     * found. */
	    if (e_comps->next == NULL)
	    	break;
	}


	if (e_class == NULL)
	{
	    e_class = new_equiv_comps();
	    e_comps->next = e_class;
	    e_class->prev = e_comps;

	    (void) append_to_comp_equiv_list(comp1, e_class);
	    (void) append_to_comp_equiv_list(comp2, e_class);
	}

	if (debugging("equiv_comp"))
	    fprint_comp_equiv_lists(stdout,intfc);
	debug_print("equiv_comp","Leaving set_equivalent_comps()\n");
	return;
}		/*end set_equivalent_comps*/


/*
*			equivalent_comps():
*
*	This function parses the e_comps lists in intfc to see if a given
*	pair of components are already in the same equivalence class.
*/

EXPORT boolean equivalent_comps(
	COMPONENT	comp1,
	COMPONENT	comp2,
	INTERFACE	*intfc)
{
	EQUIV_COMPS	*e_comps;

	if (comp1 == comp2)
	    return YES;

	for (e_comps = E_comps(intfc); e_comps != NULL; e_comps = e_comps->next)
	    if (in_comp_equiv_list(comp1,comp2,e_comps) == BOTH_FOUND)
	    	return	YES;

	return NO;
}		/*end equivalent_comps*/

/*
*			equivalent_components_list():
*
*	Returns a list of components that are equivalent to the input component
*	number.
*	NOTE!!! This list may be stored as an internal static array and thus
*	is rewritten upon each subsequent call to this function.
*/

EXPORT	const COMPONENT	*equivalent_components_list(
	COMPONENT comp,
	int       *num_equivalent,
	INTERFACE *intfc)
{
	EQUIV_COMPS	 *e_comps;
	int              i, n;
	static COMPONENT comps[1];

	if (comp == NO_COMP)
	{
	    *num_equivalent = 0;
	    return NULL;
	}
	for (e_comps = E_comps(intfc); e_comps != NULL; e_comps = e_comps->next)
	{
	    n = e_comps->n_equiv;
	    for (i = 0; i < n; ++i)
	    {
	        if (e_comps->comp[i] == comp)
		{
	            *num_equivalent = e_comps->n_equiv;
		    return e_comps->comp;
		}
	    }
	}
	comps[0] = comp;
	*num_equivalent = 1;
	return comps;
}		/*end equivalent_components_list*/


/*
*			copy_e_comps():
*
*	This function copies the entire intfc->e_comps list into a newly
*	allocated block of storage.  A pointer to the new storage is returned,
*	and the old version is not modified.
*/

LIB_LOCAL POINTER copy_e_comps(
	INTERFACE	*intfc)
{
	EQUIV_COMPS	*e_comps, *new_e_comps;
	EQUIV_COMPS	*last_new_e_comps = NULL, *top = NULL;
	int		i;
	int		n;

	for (e_comps = E_comps(intfc); e_comps != NULL; e_comps = e_comps->next)
	{
	    new_e_comps = new_equiv_comps();
	    new_e_comps->n_equiv = e_comps->n_equiv;

	    n = (e_comps->n_equiv/EQUIV_COMPS_LEN + 1)*EQUIV_COMPS_LEN;
	    new_e_comps->comp = (COMPONENT *) Store(n*sizeof(COMPONENT));
	    for (i = 0; i < n; ++i)
	    	new_e_comps->comp[i] = e_comps->comp[i];

	    if (top == NULL)
	    {
	    	top = last_new_e_comps = new_e_comps;
	    }
	    else
	    {
	    	last_new_e_comps->next = new_e_comps;
	    	new_e_comps->prev = last_new_e_comps;
	    	last_new_e_comps = new_e_comps;
	    }
	}
	return	(POINTER) top;
}		/*end copy_e_comps*/



/*
*			new_equiv_comps():
*
*	This function allocated a new e_comps structure and returns a pointer
*	to it.
*/

LOCAL EQUIV_COMPS *new_equiv_comps(void)
{
	EQUIV_COMPS	*e_comps;

	e_comps = (EQUIV_COMPS *) Store(sizeof(EQUIV_COMPS));

	e_comps->n_equiv = 0;
	e_comps->comp = NULL;
	e_comps->prev = NULL;
	e_comps->next = NULL;

	return	e_comps;
}		/*end new_equiv_comps*/


/*
*			append_to_comp_equiv_list():
*
*	This function adds a given component to a given equivalence class
*	array.  New storage is allocated if necessary, and the given
*	component is appended to the end of the list.  e_comps is assumed
*	to be non-NULL.
*/

LOCAL EQUIV_COMPS *append_to_comp_equiv_list(
	COMPONENT	comp,
	EQUIV_COMPS	*e_comps)
{
	COMPONENT       *list;
	int		i;
	int		n;

	/* Is comp already in the list */
	n = e_comps->n_equiv;
	for (i = 0; i < n; ++i)
	    if (comp == e_comps->comp[i])
		return e_comps;
	n = e_comps->n_equiv;
	if (n % EQUIV_COMPS_LEN == 0)	/* be sure the array is long enough */
	{
	    list = e_comps->comp;
	    e_comps->comp =
		(COMPONENT *) Store((n+EQUIV_COMPS_LEN)*sizeof(COMPONENT));

	    for (i = 0; i < n; ++i)
	    	e_comps->comp[i] = list[i];
	    for (i = n; i < n + EQUIV_COMPS_LEN; ++i)
	    	e_comps->comp[i] = NO_COMP;	/* invalid comp for init */

	    /* should free "list" if not done automatically */
	}
	e_comps->comp[e_comps->n_equiv++] = comp;
	return	e_comps;
}		/*end append_to_comp_equiv_list*/


/*
*			merge_comp_equiv_lists():
*
*	This function merges two given equivalence class structures.  The 
*	elements in the "class" list are copied first, and then those
*	in e_comps are appended to this list.  New storage is allocated if
*	needed.  The second structure becomes obsolete, although the e_comps
*	name is used to return a pointer to the structure with the updated
*	list.  comp is the redundant element initially appearing in both lists.
*/

LOCAL void merge_comp_equiv_lists(
	EQUIV_COMPS	*e_class,
	EQUIV_COMPS	**e_comps,
	COMPONENT	comp)
{
	COMPONENT	*list;
	int		i;
	int		n;
	int		n1 = e_class->n_equiv, n2 = (*e_comps)->n_equiv;

	n = (n1 + n2 - 1)/EQUIV_COMPS_LEN + 1;
	if (n > n1/EQUIV_COMPS_LEN + 1) /* be sure the array is long enough */
	{
	    list = e_class->comp;
	    e_class->comp =
		(COMPONENT *) Store(n*EQUIV_COMPS_LEN*sizeof(COMPONENT));

	    for (i = 0; i < n1; ++i)
	    	e_class->comp[i] = list[i];

	    for (; i < n*EQUIV_COMPS_LEN; ++i)
	    	e_class->comp[i] = -1;	/* invalid comp for init */

		/* should free "list" if not done automatically */
	}

	e_class->n_equiv = n1 + n2 - 1;
	list = (*e_comps)->comp;
	for (i = 0; i < n2; ++i)
	    if (list[i] != comp)
	    	e_class->comp[n1+i] = list[i];
	    else
	    	n1--;	/* to compensate for increment of i w/o add */

	e_class = (*e_comps)->prev;
	e_class->next = (*e_comps)->next;
	if ((*e_comps)->next != NULL)
	    (*e_comps)->next->prev = e_class;

	/* should free "*e_comps" and "list" if not done automatically */

	*e_comps = e_class;
}		/*end merge_comp_equiv_lists*/


/*
*			in_comp_equiv_list():
*
*	Tests to see if comp1 and/or comp2 are in a given e_comps list.
*/

LOCAL COMP_EQUIV_LIST in_comp_equiv_list(
	COMPONENT	comp1,
	COMPONENT	comp2,
	EQUIV_COMPS	*e_comps)
{
	int	i, n = e_comps->n_equiv;
	boolean    comp1_found = NO;
	boolean    comp2_found = NO;

	for (i = 0; i < n; ++i)
	{
	    if (e_comps->comp[i] == comp1)
		comp1_found = YES;
	    else if (e_comps->comp[i] == comp2)
		comp2_found = YES;
	}
	if ((comp1_found == YES) && (comp2_found == YES))
	    return BOTH_FOUND;
	else if (comp1_found == YES)
	    return COMP1_FOUND;
	else if (comp2_found == YES)
	    return COMP2_FOUND;
	else
	    return NEITHER_FOUND;
}		/*end in_comp_equiv_list*/
	

/*
*			fprint_comp_equiv_lists():
*
*	This function prints out the lists of equivalent components in 
*	intfc->e_comps.
*/

EXPORT void fprint_comp_equiv_lists(
	FILE      *file,
	INTERFACE *intfc)
{
	EQUIV_COMPS	*e_comps;
	int		i, n;

	(void) foutput(file);
	(void) fprintf(file,"Equivalent Component list for interface %llu\n",
		       interface_number(intfc));
	for (n=0, e_comps=E_comps(intfc); e_comps != NULL;
 	     e_comps=e_comps->next, ++n);
	(void) fprintf(file,"%d Equivalence classes\n",n);
	for (e_comps = E_comps(intfc); e_comps != NULL; e_comps = e_comps->next)
	{
	    if (e_comps->comp != NULL)
	    {
	        n = e_comps->n_equiv;
	        (void) fprintf(file,"    %d equivalent components ",n);
	        for (i = 0; i < n; ++i)
	    	    (void) printf(" %d",e_comps->comp[i]);
	        (void) printf("\n");
	    }
	    else
	        (void) fprintf(file,"    0 equivalent components\n");
	}
	(void) fprintf(file,"End Equivalent Component list for interface %llu\n",
		       interface_number(intfc));
}		/*end print_comp_equiv_list*/
