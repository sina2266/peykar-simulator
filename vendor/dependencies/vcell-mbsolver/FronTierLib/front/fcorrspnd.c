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
*				fcorrspnd.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Contains functions for the maintainence of the correspondence
*	between curves on different interfaces.
*
*	Given two hypersurfaces oldhs and newhs, the list of interfaces in
*	correspondence is searched until the correspondence structure involving
*	oldhs->interface and newhs->interface is found.	 If newhs is the ith
*	hypersurface in the the newhs hypersurface list and oldhs is the jth
*	hypersurface in the oldhs hypersurface list, then
*	corr_possible(incmtrx[i][j]) is true if a correspondence is possible
*	and FALSE otherwise.  In cases of possible correspondence,
*	orients_agree(incmtrx[i][j]) is true  when the orientations of the hyper
*	surfaces agree and FALSE otherwise.  The orientation of the hyper
*	surfaces in possible correspondence will agree unless newhs has been
*	subject to an odd number of applications of orientation reversal.  This
*	includes the case where newhs is part of a split object that was
*	inverted before it was split.
*
*/


#define DEBUG_STRING	"correspond"
#include <front/fdecs.h>		/* includes int.h, table.h */

struct _HS_LIST {
	HYPER_SURF *hs;
	double dist;
	int orients_match;
	struct _HS_LIST *next, *prev;
};

typedef struct _HS_LIST HS_LIST;

struct _CORRESPOND {
	struct _CORRESPOND *prev,*next;
	INTERFACE *newintfc, *oldintfc;
	HYPER_SURF **newhs, **oldhs;
	int num_newhs, num_oldhs;
	int **incmtrx;
	int newhs_space, oldhs_space;	/* space available for newhs, oldhs */
};

typedef struct _CORRESPOND CORRESPOND;

	/* masks (octal) and related macros for correspondence matrix */

LOCAL const int NOT_SET	      = 0x0;
LOCAL const int CORR_POSSIBLE = 0x1;
LOCAL const int ORIENTS_AGREE = 0x2;

#define corr_possible(entry)	((entry) & CORR_POSSIBLE)
#define orients_agree(entry)	((entry) & ORIENTS_AGREE)
#define Unset_flag(entry,flag)	entry &= ~(flag) 
#define Set_flag(entry,flag)	entry |= (flag) 
#define Invert_flag(entry,flag)		\
		{							\
			if ((entry) & (flag)) Unset_flag(entry,flag); \
			else			Set_flag(entry,flag);	\
		}

	/* start of linked correspondence list */

LOCAL CORRESPOND *first_cor = NULL, *last_cor = NULL;


	/* LOCAL Function Prototypes */
LOCAL	CORRESPOND	*correspondence_for_interfaces(INTERFACE*,INTERFACE*);
LOCAL	CORRESPOND	*new_correspond_struct(void);
LOCAL	HYPER_SURF	*closest_hyper_surf_in_list(HYPER_SURF*,HS_LIST*,
						    INTERFACE*);
LOCAL	int		add_newhs_to_correspond(CORRESPOND*,HYPER_SURF*);
LOCAL	int		add_oldhs_to_correspond(CORRESPOND*,HYPER_SURF*);
LOCAL	int		copy_correspondence(CORRESPOND*,INTERFACE*,
					    INTERFACE*,int);
LOCAL	int		expand_correspondence_struct(CORRESPOND*,int,int);
LOCAL	int		index_of_hs_in_crspd(HYPER_SURF*,CORRESPOND*);
LOCAL	int		init_correspondence_struct(CORRESPOND**,INTERFACE*,
						   INTERFACE*,int,int);
LOCAL	void		delete_correspond_from_list(CORRESPOND*);
LOCAL	void		delete_newhs_from_correspond(CORRESPOND*,int);
LOCAL	void		delete_oldhs_from_correspond(CORRESPOND*,int);
LOCAL	void		error_in_find_correspond_hyper_surface(int,
				HYPER_SURF*,CORRESPOND*,HYPER_SURF_BDRY**,
				HYPER_SURF_BDRY**,Front*,INTERFACE*);
LOCAL	void		free_correspondence_struct(CORRESPOND*);
LOCAL	void		insert_in_correspond_list(CORRESPOND*);
LOCAL	void		print_correspondence(CORRESPOND*);
#if defined(TWOD) || defined(THREED)
LOCAL	int		corresponding_boundaries_are_consistent(HYPER_SURF*,
			      HYPER_SURF_BDRY**,HYPER_SURF_BDRY**,INTERFACE*);
#endif /* defined(TWOD) || defined(THREED) */
#if defined(TWOD)
LOCAL	CURVE		*closest_curve_in_list(CURVE*,HS_LIST*,INTERFACE*);
LOCAL	int		correspond_curves_agree_in_orient(CURVE*,CURVE*);
#endif /* defined(TWOD) */

/*
*			set_add_to_correspond_list():
*
*	Sets the values of the local variable add_to_correspond_list
*	to control the addition of a copied interface into the
*	CORRESPOND data list.  The default value of this variable
*	is NO and must be set to YES before a call to copy_interface()
*	or remap_interface() if the dynamic correspondence between
*	the interface curves is to be recorded in the
*	incidence matrix structures.  The usual notions of 
*	correspond_curve() are preserved in both cases.
*/

LOCAL	boolean add_to_correspond_list = NO;

EXPORT	void show_crspd_between_two_intfc(
	INTERFACE *intfc1,
	INTERFACE *intfc2)
{
	CORRESPOND *crspd;

	crspd = correspondence_for_interfaces(intfc1,intfc2);
	print_correspondence(crspd);
}	/* end show_crspd_between_two_intfc */

EXPORT	void set_add_to_correspond_list(
	boolean	y_or_n)
{
	add_to_correspond_list = y_or_n;
}		/*end set_add_to_correspond_list*/


/*
*			find_correspond_hyper_surface():
*
*	Given hypersurface hs, this function attempts to find the hypersurface
*	corr_hs on the interface intfc which corresponds to hs.
*	The physics dependent function correspondence_is_possible()
*	is used to elminate candidates.
*	In addition if the list p_hsb or n_hsb are non-null and non empty
*	then the corresponding hypersurface will be required to include
*	the elements of these lists in its list of positive or negative
*	hypersurface boundaries (NOTE for 2D:  if p_hsb != NULL,  then
*	*p_hsb should equal the start node of the curve found by the
*	correspondence,	 similarly *n_hsb should be the end node.)
*	provided that the orientation between the two hypersurfaces in possible
*	correspondence agree.  If the orientations are reversed, the
*	roles of p_hsb and n_hsb are interchanged.
*
*	Returns the corresponding hypersurface if successful, NULL otherwise.
*/

EXPORT HYPER_SURF *find_correspond_hyper_surface(
	HYPER_SURF	*hs,
	HYPER_SURF_BDRY	**p_hsb,
	HYPER_SURF_BDRY	**n_hsb,
	Front		*fr,
	INTERFACE	*intfc)
{
	CORRESPOND	*crspd;
	HS_LIST		Hsl;		/* Dummy start of hypersurface list */
	HS_LIST		*hsl;
	HYPER_SURF	*corr_hs;
	HYPER_SURF	**chs;
	HYPER_SURF_BDRY	**or_p_hsb, **or_n_hsb;
	int		**incmtrx;
	int		entry;
	int		hsi, chsj;
#if defined(TWOD) || defined(THREED)
	HYPER_SURF	*corr_hyp_surf;
#endif /* defined(TWOD) || defined(THREED) */
	
	DEBUG_ENTER(find_correspond_hyper_surface)
	if (DEBUG)
	{
	    (void) printf("hs = %llu, intfc = %llu\n",
	    	          hypersurface_number(hs),interface_number(intfc));
	    (void) printf("*p_hsb = %llu, *n_hsb = %llu\n",
	    	          hypersurface_boundary_number(*p_hsb),
	    	          hypersurface_boundary_number(*n_hsb));
	}
	if (hs == NULL)
	{
	    corr_hs = NULL;
	    DEBUG_LEAVE(find_correspond_hyper_surface)
	    return corr_hs;
	}
	if (intfc == hs->interface)
	{
	    corr_hs = hs;
	    DEBUG_LEAVE(find_correspond_hyper_surface)
	    return corr_hs;
	}
	corr_hs = NULL;


	corr_hyp_surf = correspond_hyper_surf(hs);
	if (corresponding_boundaries_are_consistent(corr_hyp_surf,
						    p_hsb,n_hsb,intfc))
	{
	    corr_hs = corr_hyp_surf;
	    DEBUG_LEAVE(find_correspond_hyper_surface)
	    return corr_hs;
	}

	crspd = correspondence_for_interfaces(hs->interface,intfc);
	if (crspd == NULL) 
	{
	    if (DEBUG)
	    	error_in_find_correspond_hyper_surface(1,hs,crspd,
						       p_hsb,n_hsb,fr,intfc);
	    DEBUG_LEAVE(find_correspond_hyper_surface)
	    return corr_hs;
	}
	if (DEBUG)
	{
	    (void) printf("Correspond structure\n");
	    print_correspondence(crspd);
	}
	if ((hsi = index_of_hs_in_crspd(hs,crspd)) == ERROR)
	{
	    if (DEBUG)
	    	error_in_find_correspond_hyper_surface(2,hs,crspd,
						       p_hsb,n_hsb,fr,intfc);
	    DEBUG_LEAVE(find_correspond_hyper_surface)
	    return corr_hs;
	}
	if (DEBUG)
	    (void) printf("Index of hypersurface %llu in correspond %p = %d\n",
			  hypersurface_number(hs),(POINTER)crspd,hsi);

	/* Set up candidate list */

	incmtrx = crspd->incmtrx;
	Hsl.next = Hsl.prev = NULL;
	hsl = &Hsl;
	if (intfc == crspd->oldintfc)
	{
	    for (chsj = 0, chs = crspd->oldhs; chs && *chs; ++chsj, ++chs)
	    {
	    	if (DEBUG)
	    	    (void) printf("Testing hypersurface %llu\n",
				  hypersurface_number(*chs));
		entry = incmtrx[hsi][chsj];
		if (!corr_possible(entry)) continue;
		if (DEBUG)
		{
		    (void) printf("Possible correspondence\n");
		    (void) printf("Adding %llu to hypersurface list\n",
				  hypersurface_number(*chs));
		}
		scalar(&hsl->next,sizeof(HS_LIST));
		hsl->next->prev = hsl;
		hsl = hsl->next;
		hsl->next = NULL;
		hsl->hs = *chs;
		hsl->orients_match = orients_agree(entry);
	    }
	}
	else
	{
	    for (chsj = 0, chs = crspd->newhs; chs && *chs; ++chsj, ++chs)
	    {
	    	if (DEBUG)
	    	    (void) printf("Testing hypersurface %llu\n",
	    	    	          hypersurface_number(*chs));
	    	entry = incmtrx[chsj][hsi];
	    	if (!corr_possible(entry)) continue;
	    	if (DEBUG)
	    	{
	    	    (void) printf("Possible correspondence\n");
	    	    (void) printf("Adding %llu to hypersurface list\n",
	    			  hypersurface_number(*chs));
	    	}
	    	scalar(&hsl->next,sizeof(HS_LIST));
	    	hsl->next->prev = hsl;
	    	hsl = hsl->next;
	    	hsl->next = NULL;
	    	hsl->hs = *chs;
	    	hsl->orients_match = orients_agree(entry);
	    }
	}

	if (Hsl.next == NULL) 
	{
	    if (DEBUG)
	    	error_in_find_correspond_hyper_surface(3,hs,crspd,
						       p_hsb,n_hsb,fr,intfc);
	    DEBUG_LEAVE(find_correspond_hyper_surface)
	    return corr_hs;
	}
	else if (Hsl.next->next == NULL) /* Unique candidate */
	{
	    corr_hs = Hsl.next->hs;
	    free(Hsl.next);
	    DEBUG_LEAVE(find_correspond_hyper_surface)
	    return corr_hs;
	}

	/* Test for hypersurface bdry and physics dependent compatibility */

	for (hsl = Hsl.next; hsl; hsl = hsl->next)
	{
	    if (DEBUG)
	    	(void) printf("Testing hypersurface %llu\n",
	    		      hypersurface_number(hsl->hs));
	    or_p_hsb = (hsl->orients_match) ? p_hsb : n_hsb;
	    or_n_hsb = (hsl->orients_match) ? n_hsb : p_hsb;
#if defined(TWOD) || defined(THREED)
	    if (!corresponding_boundaries_are_consistent(hsl->hs,or_p_hsb,
							    or_n_hsb,intfc))
	    {
	    	if (hsl->prev) hsl->prev->next = hsl->next;
	    	if (hsl->next) hsl->next->prev = hsl->prev;
	    	free(hsl);
	    	continue;
	    }
	    if (DEBUG)
	       (void) printf("Hypersurface boundary information agrees\n");
#endif /* defined(TWOD) || defined(THREED) */
	    if (!correspondence_is_possible(hs,hsl->hs,
	    				       or_p_hsb,or_n_hsb,fr))
	    {
	    	if (hsl->prev) hsl->prev->next = hsl->next;
	    	if (hsl->next) hsl->next->prev = hsl->prev;
	    	free(hsl);
	    	continue;
	    }
	    if (DEBUG)
	           (void) printf("Physics dependent information agrees\n");
	}

	if (Hsl.next == NULL) 
	{
	    if (DEBUG)
	    	error_in_find_correspond_hyper_surface(3,hs,crspd,
						       p_hsb,n_hsb,fr,intfc);
	    DEBUG_LEAVE(find_correspond_hyper_surface)
	    return corr_hs;
	}
	else if (Hsl.next->next == NULL) /* Unique candidate */
	{
	    corr_hs = Hsl.next->hs;
	    free(Hsl.next);
	    DEBUG_LEAVE(find_correspond_hyper_surface)
	    return corr_hs;
	}

	corr_hs = closest_hyper_surf_in_list(hs,&Hsl,intfc);

	if (corr_hs == NULL)
	{
	    if (DEBUG)
	    	error_in_find_correspond_hyper_surface(4,hs,crspd,p_hsb,
						       n_hsb,fr,intfc);
	}
	for (hsl = Hsl.next; hsl; hsl = hsl->next) free(hsl);
	DEBUG_LEAVE(find_correspond_hyper_surface)
	return corr_hs;
}		/*end find_correspond_hyper_surface*/


/*
*		set_correspondence_between_interfaces():
*
*	Sets up the correspondence data structures between
*	the two interfaces oldintfc and newintfc.  Assumes that
*	newintfc is a copy or remap of oldintfc so the curves are in perfect
*	correspondence.
*	Called in f_USER_copy_interface.
*/

EXPORT boolean set_correspondence_between_interfaces(
	INTERFACE	*oldintfc,
	INTERFACE	*newintfc)
{

	CORRESPOND	*corrspnd, *crspd1;
	HYPER_SURF	**oldhs, **newhs;
	HYPER_SURF_BDRY	**oldhsb, **newhsb;
	int		i, num_hs, num_oldhs, num_newhs;

	DEBUG_ENTER(set_correspondence_between_interfaces)
	if (DEBUG)
	{
	  (void) printf("Setting correspondence between oldintfc %llu and ",
			interface_number(oldintfc));
	  (void) printf("newintfc %llu\n",interface_number(newintfc));
	}


	/* Set correspond hypersurface boundaries */
	for (oldhsb = hyper_surf_bdry_list(oldintfc);oldhsb && *oldhsb; ++oldhsb)
	    correspond_hyper_surf_bdry(*oldhsb) = hsb_copied_to(*oldhsb);
	for (newhsb = hyper_surf_bdry_list(newintfc);newhsb && *newhsb; ++newhsb)
	    correspond_hyper_surf_bdry(*newhsb) = hsb_copied_from(*newhsb);

	/* Set correspond hypersurfaces and count number of hypersurfaces */

	for (oldhs = hyper_surf_list(oldintfc), num_oldhs = 0;
		oldhs && *oldhs; ++oldhs, ++num_oldhs)
	    correspond_hyper_surf(*oldhs) = hs_copied_to(*oldhs);
	if (DEBUG)
	    (void) printf("number of old hypersurfaces = %d\n",num_oldhs);

	for (newhs = hyper_surf_list(newintfc), num_newhs = 0;
		newhs && *newhs; ++newhs, ++num_newhs)
	    correspond_hyper_surf(*newhs) = hs_copied_from(*newhs);
	if (DEBUG)
	    (void) printf("number of new hypersurfaces = %d\n",num_newhs);

	if (num_oldhs != num_newhs)
	{
	    screen("ERROR in set_correspondence_between_interfaces(), "
		   "different number of hyper surfaces on interfaces\n");
	    (void) printf("num_oldhs = %d, num_newhs = %d\n",
			  num_oldhs,num_newhs);
	    clean_up(ERROR);
	}
	num_hs = num_oldhs;

	if (!add_to_correspond_list)
	{
	    DEBUG_LEAVE(set_correspondence_between_interfaces)
	    return YES;
	}
	add_to_correspond_list = NO; /* Default value for next copy */

	if (!init_correspondence_struct(&corrspnd,oldintfc,
					   newintfc,num_oldhs,num_newhs))
	{
	    DEBUG_LEAVE(set_correspondence_between_interfaces)
	    return NO;
	}
	for (i = 0; i < num_hs; ++i)
	{
	    Set_flag(corrspnd->incmtrx[i][i],CORR_POSSIBLE | ORIENTS_AGREE);
	}
	if (DEBUG)
	{
	    (void) printf("New CORRESPOND structure\n");
	    print_correspondence(corrspnd);
	}

	/*Copy correspondence from other interfaces corresponding to oldintfc*/

	for (crspd1 = first_cor; crspd1 != corrspnd; crspd1 = crspd1->next)
	{
	    if ((crspd1->oldintfc==oldintfc) || (crspd1->newintfc==oldintfc))
	    {
	    	if (!copy_correspondence(crspd1,oldintfc,newintfc,num_hs))
		{
		    DEBUG_LEAVE(set_correspondence_between_interfaces)
		    return NO;
		}
	    }
	}
	DEBUG_LEAVE(set_correspondence_between_interfaces)
	return YES;
}		/*end set_correspondence_between_interfaces*/


/*
*			rst_cor_after_delete_interface():
*
*	Removes the CORRESPOND structures containing intfc from
*	the list AND frees the storage used.
*/

EXPORT	void rst_cor_after_delete_interface(
	INTERFACE	*intfc)
{
	CORRESPOND	*crspd, *next_crspd = NULL;

	for (crspd = first_cor; crspd != NULL; crspd = next_crspd)
	{
		next_crspd = crspd->next;
		if ((intfc == crspd->oldintfc) || (intfc == crspd->newintfc))
		{
		    if (DEBUG)
		    {
			(void) printf("In rst_cor_after_delete_interface(),");
			(void) printf("\n\tintfc = %llu,",
				      interface_number(intfc));
			(void) printf(" deleting correspond = %p\n",
				      (POINTER)crspd);
		    }
		    delete_correspond_from_list(crspd);
		    free_correspondence_struct(crspd);
		}
	}
}		/*end rst_cor_after_delete_interface*/


EXPORT void remove_corresponds_to_deleted_interface(
	INTERFACE	*intfc)
{
	struct Table	*IT = interface_table_list();
	struct Table	*T;
	HYPER_SURF	**hs;
	HYPER_SURF_BDRY **hsb;

	for (T = IT; T != NULL; T = T->next)
	{
		if (T->interface == intfc) continue;
		for (hs = hyper_surf_list(T->interface); hs && *hs; ++hs)
		{
			if (!correspond_hyper_surf(*hs)) continue;
			if (correspond_hyper_surf(*hs)->interface == intfc)
				correspond_hyper_surf(*hs) = NULL;
		}
		for (hsb = hyper_surf_bdry_list(T->interface);
							hsb && *hsb; ++hsb)
		{
		    if (!correspond_hyper_surf_bdry(*hsb)) continue;
		    if (correspond_hyper_surf_bdry(*hsb)->interface == intfc)
			    correspond_hyper_surf_bdry(*hsb) = NULL;
		}
	}
}		/*end remove_corresponds_to_deleted_interface*/


EXPORT	void	set_correspond_hyper_surfaces_to_NULL(
	INTERFACE	*intfc)
{
	HYPER_SURF	**hs;

	hs = hyper_surf_list(intfc);
	for (; hs && *hs; ++hs) correspond_hyper_surf(*hs) = NULL;
}		/*end set_correspond_hyper_surfaces_to_NULL*/


EXPORT	void	set_correspond_hyper_surf_bdrys_to_NULL(
	INTERFACE	*intfc)
{
	HYPER_SURF_BDRY **hsb;

	hsb = hyper_surf_bdry_list(intfc);
	for (; hsb && *hsb; ++hsb) correspond_hyper_surf_bdry(*hsb) = NULL;
}		/*end set_correspond_hyper_surf_bdrys_to_NULL*/


/*
*			zero_corr_of_hyper_surf():
*
*	This function is used to zero out all correspondences of a given
*	hyper surface.  For split_curve(), for example, the reset function
*	belows says both new hyper surfaces correspond to the old one.
*	This function provides the facility of setting the correspondences
*	of one or both of the new hyper surfaces to null.  Unlike the reset
*	for delete, hs retains its CORRESPOND entries in the new/old hs lists
*	and incmtrx.
*/

EXPORT	void zero_corr_of_hyper_surf(
	HYPER_SURF	*hs)
{
	CORRESPOND	*crspd;
	INTERFACE	*intfc = hs->interface;
	int		index, corr_index;

	if (correspond_hyper_surf(hs))
	    correspond_hyper_surf(correspond_hyper_surf(hs)) = NULL;
	correspond_hyper_surf(hs) = NULL;
	for (crspd = first_cor; crspd != NULL; crspd = crspd->next)
	{
	    if (intfc == crspd->newintfc) 
	    {
	    	index = index_of_hs_in_crspd(hs,crspd);
	    	for (corr_index=0; corr_index < crspd->num_oldhs; ++corr_index)
	    	{
	    		crspd->incmtrx[index][corr_index] = NOT_SET;
	    	}
	    	if (DEBUG) print_correspondence(crspd);
	    }
	    else if (intfc == crspd->oldintfc) 
	    {
	    	index = index_of_hs_in_crspd(hs,crspd);
	    	for (corr_index=0; corr_index < crspd->num_newhs; ++corr_index)
	    	{
	    	    crspd->incmtrx[corr_index][index] = NOT_SET;
	    	}
	    	if (DEBUG) print_correspondence(crspd);
	    }
	}
}		/*end zero_corr_of_hyper_surf*/


EXPORT	boolean rst_cor_after_delete_hyper_surf(
	HYPER_SURF	*hs)
{
	CORRESPOND	*crspd;
	HYPER_SURF	**newhs, **oldhs;
	INTERFACE	*intfc = hs->interface;
	int		index;

	DEBUG_ENTER(rst_cor_after_delete_hyper_surf)
	if (DEBUG)
	    print_hypersurface(hs);

	if (correspond_hyper_surf(hs))
	    correspond_hyper_surf(correspond_hyper_surf(hs)) = NULL;
	for (crspd = first_cor; crspd != NULL; crspd = crspd->next)
	{
	    if (intfc == crspd->newintfc) 
	    {
	    	for (newhs = crspd->newhs, index = 0;
	    	     newhs && *newhs; ++newhs, ++index)
	    	{
	    	    if (*newhs != hs)
		        continue;
	    	    delete_newhs_from_correspond(crspd,index);
	    	    if (DEBUG)
	    	    	print_correspondence(crspd);
	    	    break;
	    	}
	    }
	    else if (intfc == crspd->oldintfc) 
	    {
	    	for (oldhs = crspd->oldhs, index = 0;
	    	     oldhs && *oldhs; ++oldhs, ++index)
	    	{
	    	    if (*oldhs != hs)
		        continue;
	    	    delete_oldhs_from_correspond(crspd,index);
	    	    if (DEBUG)
	    	    	print_correspondence(crspd);
	    	    break;
	    	}
	    }
	}
	DEBUG_LEAVE(rst_cor_after_delete_hyper_surf)
	return YES;
}		/*end rst_cor_after_delete_hyper_surf*/


EXPORT	boolean rst_cor_after_make_hyper_surf(
	HYPER_SURF	*hs)
{
	CORRESPOND	*crspd;
	INTERFACE	*intfc = hs->interface;

	correspond_hyper_surf(hs) = NULL;
	for (crspd = first_cor; crspd != NULL; crspd = crspd->next)
	{
	    if (intfc == crspd->newintfc)
	    {
	    	if (!add_newhs_to_correspond(crspd,hs))
	    	    return NO;
	    }
	    else if (intfc == crspd->oldintfc)
	    {
	    	if (!add_oldhs_to_correspond(crspd,hs))
	    	    return NO;
	    }
	}
	return YES;
}		/*end rst_cor_after_make_hyper_surf*/


EXPORT void print_correspond_hyper_surf_list(
	INTERFACE	*intfc)
{
	HYPER_SURF	**hs;

	(void) printf("Correspond hypersurfaces\n");
	(void) printf("for curves on interface %llu\n\n",
		      interface_number(intfc));
	(void) printf("%-15s%-15s\n","Hypersurface","Correspond hypersurface");
	for (hs = hyper_surf_list(intfc); hs && *hs; ++hs)
	{
		(void) printf("%-15llu%-15llu\n",hypersurface_number(*hs),
			      hypersurface_number(correspond_hyper_surf(*hs)));
	}
	(void) printf("\nEnd of printout of correspond hypersurfaces\n");
	(void) printf("for hypersurfaces on interface %llu\n\n",
		      interface_number(intfc));
}		/*end print_correspond_hyper_surf_list*/



LOCAL	CORRESPOND *correspondence_for_interfaces(
	INTERFACE	*intfc1,
	INTERFACE	*intfc2)
{
	CORRESPOND	*crspd;

	for (crspd = first_cor; crspd != NULL; crspd = crspd->next)
	{
		if ((intfc1 == crspd->oldintfc && intfc2 == crspd->newintfc)
					||
		    (intfc1 == crspd->newintfc && intfc2 == crspd->oldintfc))
			break;
	}
	return crspd;
}		/*end correspondence_for_interface*/

LOCAL	int index_of_hs_in_crspd(
	HYPER_SURF	*hs,
	CORRESPOND	*crspd)
{
	int		index;

	if (hs->interface == crspd->oldintfc)
	{
	    for (index = 0; index < crspd->num_oldhs; ++index)
	    	if (hs == crspd->oldhs[index]) return index;
	}
	else if (hs->interface == crspd->newintfc)
	{
	    for (index = 0; index < crspd->num_newhs; ++index)
	    	if (hs == crspd->newhs[index]) return index;
	}
	return ERROR;
}		/*end index_of_hs_in_crspd*/



LOCAL	int copy_correspondence(
	CORRESPOND	*crspd,
	INTERFACE	*oldintfc,
	INTERFACE	*newintfc,
	int		num_hs)
{
	CORRESPOND	*corrspnd;
	int		i,j;

	DEBUG_ENTER(copy_correspondence)
	if (DEBUG)
	{
		(void) printf("copying correspond structure %p\n",
			      (POINTER)crspd);
		print_correspondence(crspd);
		(void) printf("oldintfc = %llu, newintfc = %llu, num_hs = %d\n",
			      interface_number(oldintfc),
			      interface_number(newintfc),num_hs);
		(void) printf("crsdp->oldintfc = %llu, crspd->newintfc = %llu\n",
			      interface_number(crspd->oldintfc),
			      interface_number(crspd->newintfc));
	}
	if (crspd->oldintfc == oldintfc)
	{
		if (!init_correspondence_struct(&corrspnd,newintfc,
				crspd->newintfc,num_hs,crspd->num_newhs))
		{
			DEBUG_LEAVE(copy_correspondence)
			return NO;
		}
	}
	else if (crspd->newintfc == oldintfc)
	{
		if (!init_correspondence_struct(&corrspnd,crspd->oldintfc,
				newintfc,crspd->num_oldhs,num_hs))
		{
			DEBUG_LEAVE(copy_correspondence)
			return NO;
		}
	}
	for (i = 0; i < crspd->num_newhs; ++i)
	{
		for (j = 0; j < crspd->num_oldhs; ++j)
		{
			corrspnd->incmtrx[i][j] = crspd->incmtrx[i][j];
		}
	}

	if (DEBUG)
	{
		(void) printf("New CORRESPOND structure\n");
		print_correspondence(corrspnd);
	}
	DEBUG_LEAVE(copy_correspondence)
	return YES;
}		/*end copy_correspondence*/




/*
*			init_correspondence_struct():
*
*	Allocates storage for CORRESPOND structure and its members
*	and sets the all fields in the structure to the 
*	correct values except the correspondence bi_array
*	which is zeroed.  Inserts the structure in the 
*	list.
*
*	Returns NO if unable to allocate storage.
*/

LOCAL	int init_correspondence_struct(
	CORRESPOND	**corrspnd,
	INTERFACE	*oldintfc,
	INTERFACE	*newintfc,
	int		num_oldhs,
	int		num_newhs)
{
	HYPER_SURF	**intfc_hs,**crdpd_hs;

	DEBUG_ENTER(init_correspondence_struct)
	if (DEBUG)
	{
		(void) printf("Initializing correspondence struct between\n");
		(void) printf("old interface %llu and new interface %llu\n",
			      interface_number(oldintfc),
			      interface_number(newintfc));
		(void) printf("num_oldhs = %d, num_newhs = %d\n",
			      num_oldhs,num_newhs);
		print_interface(oldintfc);
		print_interface(newintfc);
	}

		/* allocate (and zero) storage	*/

	*corrspnd = new_correspond_struct();
	if (!expand_correspondence_struct(*corrspnd,num_newhs,num_oldhs))
	{
		if (DEBUG)
		{
			(void) printf("Not enough space in ");
			(void) printf("init_correspondence_struct()\n");
		}
		DEBUG_LEAVE(init_correspondence_struct)
		return NO;
	}

		/* set interface pointers */

	(*corrspnd)->oldintfc = oldintfc;
	(*corrspnd)->newintfc = newintfc;

		/* copy curve arrays from interfaces */
		/* (assumes num_oldhs/newhs correct)*/

	for (intfc_hs = hyper_surf_list(oldintfc), crdpd_hs=(*corrspnd)->oldhs;
	     intfc_hs && *intfc_hs; ++intfc_hs, ++crdpd_hs)
		*crdpd_hs = *intfc_hs;
	(*corrspnd)->num_oldhs = num_oldhs;
	(*corrspnd)->oldhs_space -= num_oldhs;

	for (intfc_hs = hyper_surf_list(newintfc), crdpd_hs=(*corrspnd)->newhs;
	     intfc_hs && *intfc_hs; ++intfc_hs, ++crdpd_hs)
		*crdpd_hs = *intfc_hs;
	(*corrspnd)->num_newhs = num_newhs;
	(*corrspnd)->newhs_space -= num_newhs;
		
		/* insert in correspondence list */

	insert_in_correspond_list(*corrspnd);
	if (DEBUG)
		(void) printf("new correspondence structure = %p\n",
			      (POINTER)*corrspnd);

	DEBUG_LEAVE(init_correspondence_struct)
	return YES;
}		/*end init_correspondence_struct*/


LOCAL	void insert_in_correspond_list(
	CORRESPOND	*corrspnd)
{
	corrspnd->prev = last_cor;
	corrspnd->next = NULL;
	if (last_cor != NULL) last_cor->next = corrspnd;
	last_cor = corrspnd;
	if (first_cor == NULL) first_cor = corrspnd;
}		/*end insert_in_correspond_list*/


LOCAL	void delete_correspond_from_list(
	CORRESPOND	*crspd)
{
	if (crspd->prev != NULL)	crspd->prev->next = crspd->next;
	else				first_cor = crspd->next;

	if (crspd->next != NULL)	crspd->next->prev = crspd->prev;
	else				last_cor = crspd->prev;
}		/*end delete_correspond_from_list*/


/*
*			new_correspond_struct():
*
*	Returns the address of a zeroed CORRESPOND structure.
*	Returns NULL if unable to allocate storage.
*/

LOCAL CORRESPOND *new_correspond_struct(void)
{
	CORRESPOND	*crspd;

	scalar(&crspd,sizeof(CORRESPOND));

	return crspd;
}		/*end new_correspond_struct*/







/*
*			expand_correspondence_struct():
*
*	Expands the correspondence structure to hold nrow/ncol
*	more rows (newhs) and columns (oldhs).	The extra space is
*	all zeroed (set to NULL and NOT_SET as appropriate).
*
*	To reduce the number of memory allocation requests (at the expense of
*	possibly wasting some memory), each memory allocation request provides
*	space for GROWING_SPACE extra rows and columns. 
*
*	Returns NO if unable to allocate storage or if the structure is not
*	allocated.
*
*	NOTE: The incidence matrix is allocated as a fixed matrix not as a 
*	dynamic array of pointers.  The construct
*	for (row = incmtrx; *row; ++row) will NOT work.
*	This construct will work for the newhs/oldhs arrays (see below).
*/

LOCAL int expand_correspondence_struct(
	CORRESPOND	*crspd,
	int		nrow,
	int		ncol)
{
	HYPER_SURF	**new_oldhs, **new_newhs;
	int		**new_incmtrx;
	int		i,j,new_nrow,new_ncol;
	static const int GROWING_SPACE	= 20;

	DEBUG_ENTER(expand_correspondence_struct)

	if (crspd == NULL)
	{
		DEBUG_LEAVE(expand_correspondence_struct)
		return NO;
	}
	if ((nrow < crspd->newhs_space) && (ncol < crspd->oldhs_space))
	{
		DEBUG_LEAVE(expand_correspondence_struct)
		return YES;
	}

		/* NOTE: strict < to guarantee that the construct	*/
		/*	 { for (c=oldhs; c && *c; ++c) } will terminate */
		/* allocate larger uni_arrays/bi_array */

	print_storage("expand_correspondence_struct (start)","CORR_storage");
	new_nrow = crspd->num_newhs + nrow + GROWING_SPACE;
	new_ncol = crspd->num_oldhs + ncol + GROWING_SPACE;
	uni_array(&new_oldhs, new_ncol, sizeof(HYPER_SURF *));
	uni_array(&new_newhs, new_nrow, sizeof(HYPER_SURF *));
	bi_array(&new_incmtrx, new_nrow, new_ncol,sizeof(int));
	if (new_incmtrx == NULL)
		return NO;
	print_storage("expand_correspondence_struct (middle)","CORR_storage");
	
		/* copy data to larger uni_arrays/bi_array (zeroing extra data) */

	for (i = 0; i < crspd->num_oldhs; ++i)
	    new_oldhs[i] = crspd->oldhs[i];
	for (i = crspd->num_oldhs; i < new_ncol; ++i)
	    new_oldhs[i] = NULL;

	for (i = 0; i < crspd->num_newhs; ++i)
	{
		new_newhs[i] = crspd->newhs[i];
		for (j = 0; j < crspd->num_oldhs; ++j)
		{
			new_incmtrx[i][j] = crspd->incmtrx[i][j];
		}
	}
	for (i = crspd->num_newhs; i < new_nrow; ++i)
	{
		new_newhs[i] = NULL;
		for (j = crspd->num_oldhs; j < new_ncol; ++j)
		{
			new_incmtrx[i][j] = NOT_SET;
		}
	}

		/* free old storage and replace with larger uni_arrays/bi_array */

	if (crspd->incmtrx) free(crspd->incmtrx);
	crspd->incmtrx = new_incmtrx;
	if (crspd->newhs) free(crspd->newhs);
	crspd->newhs = new_newhs;
	if (crspd->oldhs) free(crspd->oldhs);
	crspd->oldhs = new_oldhs;
		
		/* correct space counters */
	
	crspd->oldhs_space = new_ncol - crspd->num_oldhs;
	crspd->newhs_space = new_nrow - crspd->num_newhs;

	print_storage("expand_correspondence_struct (end)","CORR_storage");
	DEBUG_LEAVE(expand_correspondence_struct)
	return YES;
}		/*end expand_correspondence_struct*/


/*
*		free_correspondence_struct():
*
*	Frees all the storage associated with the CORRESPOND
*	structure corrspnd.
*/

LOCAL void free_correspondence_struct(
	CORRESPOND	*corrspnd)
{
	if (corrspnd == NULL) return;

	if (corrspnd->incmtrx) free(corrspnd->incmtrx);
	if (corrspnd->newhs)   free(corrspnd->newhs);
	if (corrspnd->oldhs)   free(corrspnd->oldhs);
	free(corrspnd);
}		/*end free_correspondence_struct*/



/*
*		delete_oldhs_from_correspond():
*
*	Removes the curve references from the curve array and correspondence 
*	bi_array.	 It simply moves the last entry data to the location
*	of the entry to be deleted.  The end of the data is then
*	zeroed and the counts are updated.
*/

LOCAL void delete_oldhs_from_correspond(
	CORRESPOND	*crspd,
	int		index)
{
	HYPER_SURF	**carray = crspd->oldhs;
	int		**row;
	int		last_i,i;

	last_i = crspd->num_oldhs - 1;
	if (index < 0 || index > last_i) return;

		/* remove curve from curve list */

	carray[index] = carray[last_i];
	carray[last_i] = NULL;

		/* remove entries from correspondence matrix */

	for (i = 0, row = crspd->incmtrx; i < crspd->num_newhs; ++i, ++row)
	{
		(*row)[index] = (*row)[last_i];
		(*row)[last_i] = NOT_SET;
	}

		/* update counters */

	--crspd->num_oldhs;
	++crspd->oldhs_space;
}		/*end delete_oldhs_from_correspond*/



LOCAL void delete_newhs_from_correspond(
	CORRESPOND	*crspd,
	int		index)
{
	HYPER_SURF	**carray = crspd->newhs;
	int		*row,*last_row;
	int		last_i,i;

	last_i = crspd->num_newhs - 1;
	if ((index < 0) || (index > last_i)) return;

		/* remove curve from curve list */

	carray[index] = carray[last_i];
	carray[last_i] = NULL;

		/* remove entries from correspondence matrix */

	row = crspd->incmtrx[index];
	last_row = crspd->incmtrx[last_i];
	for (i = 0; i < crspd->num_oldhs; ++i)
	{
		row[i] = last_row[i];
		last_row[i] = NOT_SET;
	}

		/* update counters */

	--crspd->num_newhs;
	++crspd->newhs_space;
}		/*end delete_newhs_from_correspond*/


/*
*			add_newhs_to_correspond():
*
*	Adds the curve references to the curve array and correspondence 
*	bi_array.	 If enough space is available in the array and bi_array
*	for another curve, the counters are updated and the curve is
*	added to the curve array.  (The matrix data is not touched since it 
*	was set to NOT_SET during initialization.)
*	
*	If the matrix and array are not large enough, larger copies are first 
*	made and the old storage is freed.  Returns NO if a larger copy
*	cannot be made.
*/

LOCAL int add_newhs_to_correspond(
	CORRESPOND	*crspd,
	HYPER_SURF	*newhs)
{
	if (!expand_correspondence_struct(crspd,1,0)) return NO;

	/* add curve to list and update counters */

	crspd->newhs[crspd->num_newhs] = newhs;
	++crspd->num_newhs;
	--crspd->newhs_space;
	return YES;
}		/*end add_newhs_to_correspond*/


LOCAL int add_oldhs_to_correspond(
	CORRESPOND	*crspd,
	HYPER_SURF	*oldhs)
{
	if (!expand_correspondence_struct(crspd,0,1)) return NO;

	/* add curve to list and update counters */

	crspd->oldhs[crspd->num_oldhs] = oldhs;
	++crspd->num_oldhs;
	--crspd->oldhs_space;
	return YES;
}		/*end add_oldhs_to_correspond*/

/*ARGSUSED*/
LOCAL	HYPER_SURF *closest_hyper_surf_in_list(
	HYPER_SURF	*hs,
	HS_LIST		*Hsl,
	INTERFACE	*intfc)
{
	switch (intfc->dim)
	{
#if defined(ONED)
	case 1:/* TODO */
		break;
#endif /* defined(ONED) */
#if defined(TWOD)
	case 2:
		return Hyper_surf(
			closest_curve_in_list(Curve_of_hs(hs),Hsl,intfc));
#endif /* defined(TWOD) */
#if defined(THREED)
	case 3:/* TODO */
		break;
#endif /* defined(THREED) */
	}
	return NULL;
}		/*end closest_hyper_surf_in_list*/

LOCAL void print_correspondence(
	CORRESPOND	*crspd)
{
	int		i, j;

	if (crspd == NULL)
	{
		(void) printf("NULL CORRESPOND structure\n");
		return;
	}
	(void) printf("\n\nCORRESPOND structure %p\n",(POINTER)crspd);
	(void) printf("prev = %p, next = %p\n",
		      (POINTER)crspd->prev,(POINTER)crspd->next);
	(void) printf("oldintfc = %llu, newintfc = %llu\n",
		      interface_number(crspd->oldintfc),
		      interface_number(crspd->newintfc));
	(void) printf("num_oldhs = %d, num_newhs = %d\n",crspd->num_oldhs,
		      crspd->num_newhs);
	(void) printf("oldhs_space = %d, newhs_space = %d\n",
		      crspd->oldhs_space,crspd->newhs_space);
	print_correspond_hyper_surf_list(crspd->oldintfc);
	print_correspond_hyper_surf_list(crspd->newintfc);
	(void) printf("\nOld hypersurfaces\n");
	for (i = 0; i < crspd->num_oldhs; ++i)
		(void) printf("oldhs[%d] = %llu\n",
			      i,hypersurface_number(crspd->oldhs[i]));
	(void) printf("\n");
	(void) printf("\nNew hypersurfaces\n");
	for (i = 0; i < crspd->num_newhs; ++i)
		(void) printf("newhs[%d] = %llu\n",i,
			      hypersurface_number(crspd->newhs[i]));

	(void) printf("\nIncidence bi_array: row = new, col = old\n");
	(void) printf("   : ");
	for (j = 0; j < crspd->num_oldhs; ++j)
		(void) printf("%3d",j);
	(void) printf("\n\n");
	for (i = 0; i < crspd->num_newhs; ++i)
	{
		(void) printf("%3d: ",i);
		for (j = 0; j < crspd->num_oldhs; ++j)
		{
			(void) printf("%3d",crspd->incmtrx[i][j]);
		}
		(void) printf("\n");
	}
	
	(void) printf("\n\nEnd of CORRESPOND structure %p\n\n",(POINTER)crspd);
}		/*end print_correspondence*/


/*ARGSUSED*/
LOCAL	void error_in_find_correspond_hyper_surface(
	int		flag,
	HYPER_SURF	*hs,
	CORRESPOND	*crspd,
	HYPER_SURF_BDRY	**p_hsb,
	HYPER_SURF_BDRY	**n_hsb,
	Front		*fr,
	INTERFACE	*intfc)
{
	screen("ERROR - in find_correspond_hyper_surface(), ");
	screen("Unable to find hypersurface corresponding to hypersurface %d ",
		hs);
	screen("on interface %d.\n",intfc);
	switch (flag)
	{
	case 0:
		screen("Null input hypersurface\n");
		break;
	case 1:
		screen("Correspond structure not found\n");
		break;
	
	case 2:
		screen("index of hypersurface %d ",hs);
		screen("in correspond structure %d = ERROR\n",crspd);
		break;

	case 3:
		screen("No correspond candidates found\n");
		break;

	case 4:
		screen("All correspond candidates eliminated\n");
		break;
	
	}
	(void) printf("Input hypersurface %llu\n",hypersurface_number(hs));
	print_hypersurface(hs);
	(void) printf("Positive hypersurface boundary list\n");
	print_hypersurface_boundaries(p_hsb);
	(void) printf("Negative hypersurface boundary list\n");
	print_hypersurface_boundaries(n_hsb);
	(void) printf("Corresponding interface\n");
	print_interface(intfc);
	if (hs == NULL) return;
	(void) printf("Interface of input hypersurface %llu\n",
		      hypersurface_number(hs));
	print_interface(hs->interface);
	print_correspondence(crspd);
}		/*end error_in_find_correspond_hyper_surface*/

#if defined(TWOD) || defined(THREED)

LOCAL	int corresponding_boundaries_are_consistent(
	HYPER_SURF	*c_hs,
	HYPER_SURF_BDRY	**p_hsb,
	HYPER_SURF_BDRY	**n_hsb,
	INTERFACE	*intfc)
{

	if (c_hs == NULL)
	    return NO;
	if (c_hs->interface != intfc)
	    return NO;

	switch (intfc->dim)
	{
#if defined(ONED)
	case 1:
	    break;
#endif /* defined(ONED) */
#if defined(TWOD)
	case 2:
	    {
	        CURVE	*c_c = Curve_of_hs(c_hs);
		if ((p_hsb != NULL) && (*p_hsb != NULL))
		{
		    if (Hyper_surf_bdry(c_c->start) != *p_hsb)
		    	return NO;
		}
		if ((n_hsb != NULL) && (*n_hsb != NULL))
		{
		    if (Hyper_surf_bdry(c_c->end) != *n_hsb)
		    	return NO;
		}
	    }
	    break;
#endif /* defined(TWOD) */
#if defined(THREED)
	case 3:
	    {
	        CURVE   **c;
	        SURFACE *c_s;

	        c_s = Surface_of_hs(c_hs);
	        for (c = (CURVE**)p_hsb; c && *c; ++c)
	        {
	    	    if (!pointer_is_in_array(*c,c_s->pos_curves))
	    	        return NO;
	        }
	        for (c = (CURVE**)n_hsb; c && *c; ++c)
	        {
	    	    if (!pointer_is_in_array(*c,c_s->neg_curves))
	    	        return NO;
	        }
	    }
	    break;
#endif /* defined(THREED) */
	}
	return YES;
}		/*end corresponding_boundaries_are_consistent*/

EXPORT	int rst_cor_after_join_hypersurfaces(
	HYPER_SURF *hs1,
	HYPER_SURF *hs2,
	HYPER_SURF *hs)
{
	CORRESPOND	*crspd;
	INTERFACE	*intfc = hs->interface;
	HYPER_SURF	**newhs, **oldhs;
	int		*rowc, *rowc1, *rowc2, **row;
	int		j, jc, jc1, jc2;

	DEBUG_ENTER(rst_cor_after_join_hypersurfaces)
	if (DEBUG)
	{
	    (void) printf("hs1:\n");
	    (void) printf("hypersurf 1 = %llu\n",hypersurface_number(hs1));
	    print_hypersurface(hs1);
	    (void) printf("hs2:\n");
	    (void) printf("hypersurf 2 = %llu\n",hypersurface_number(hs2));
	    print_hypersurface(hs2);
	    (void) printf("hs:\n");
	    (void) printf("hypersurf = %llu\n",hypersurface_number(hs));
	    print_hypersurface(hs);
	}

	if (correspond_hyper_surf(hs1))
	    correspond_hyper_surf(correspond_hyper_surf(hs1)) = NULL;
	correspond_hyper_surf(hs1) = NULL;
	if (correspond_hyper_surf(hs2))
	    correspond_hyper_surf(correspond_hyper_surf(hs2)) = NULL;
	correspond_hyper_surf(hs2) = NULL;
	correspond_hyper_surf(hs) = NULL;
	for (crspd = first_cor; crspd != NULL; crspd = crspd->next)
	{
	    if (intfc == crspd->newintfc)
	    {
	    	/* Find rows of hs1, hs2 && hs */

	    	rowc = rowc1 = rowc2 = NULL;
	    	for (newhs=crspd->newhs, row=crspd->incmtrx; newhs && *newhs;
		     ++newhs, ++row)
	    	{
	    	    if (*newhs == hs)
	    	    	rowc = *row;
	    	    else if (*newhs == hs1)
	    	    	rowc1 = *row;
	    	    else if (*newhs == hs2)
	    	    	rowc2 = *row;
	    	}
		if (rowc == NULL || rowc1 == NULL || rowc2 == NULL)
		{
		    DEBUG_LEAVE(rst_cor_after_join_hypersurfaces)
		    return NO;
		}
		for (j = 0; j < crspd->num_oldhs; ++j)
		{
		    rowc[j] = rowc1[j] | rowc2[j];
		}
		if (DEBUG)
			print_correspondence(crspd);
	    }
	    else if (intfc == crspd->oldintfc)
	    {
	    	/* Find columns of hs1, hs2 && hs */
	    	jc = jc1 = jc2 = -1;
	    	for (oldhs=crspd->oldhs, j=0; oldhs && *oldhs; ++oldhs, ++j)
	    	{
	    	    if (*oldhs == hs)
	    	    	jc = j;
	    	    else if (*oldhs == hs1)
	    	    	jc1 = j;
	    	    else if (*oldhs == hs2)
	    	    	jc2 = j;
	    	}
	    	if (jc < 0 || jc1 < 0 || jc2 < 0)
	    	{
	    	    DEBUG_LEAVE(rst_cor_after_join_hypersurfaces)
	    	    return NO;
	    	}
	    	for (row = crspd->incmtrx, j = 0;
	    	     j < crspd->num_newhs; ++j, ++row)
	    	{
	    	    (*row)[jc] = (*row)[jc1] | (*row)[jc2];
	    	}
	    	if (DEBUG)
	    	    print_correspondence(crspd);
	    }
	}
	DEBUG_LEAVE(rst_cor_after_join_hypersurfaces)
	return YES;
}		/*end rst_cor_after_join_hypersurfaces*/

#endif /* defined(TWOD) || defined(THREED) */

#if defined(TWOD)
/*
*		correspond_curves_agree_in_orient():
*
*	Checks that the orientations of possibly corresponding curves
*	are consistent.
*	Currently only valid in 2D.
*/


LOCAL	int correspond_curves_agree_in_orient(
	CURVE		*c,
	CURVE		*corr_c)
{
	int		ci, corr_ci;
	CORRESPOND	*crspd;
	int		**incmtrx;
	int		entry;

	if (c == NULL || corr_c == NULL) return ERROR;

	crspd = correspondence_for_interfaces(c->interface,
					corr_c->interface);
	if (crspd == NULL)
	{
		screen("ERROR in correspond_curves_agree_in_orient(), ");
		screen("Unable to find correspond structure\n");
		clean_up(ERROR);
	}
	ci = index_of_hs_in_crspd(Hyper_surf(c),crspd);
	corr_ci = index_of_hs_in_crspd(Hyper_surf(corr_c),crspd);
	if (ci == ERROR || corr_ci == ERROR) 
	{
		screen("ERROR in correspond_curves_agree_in_orient(), ");
		screen("Unable to find index of curves in correspondence\n");
		clean_up(ERROR);
	}
	incmtrx = crspd->incmtrx;
	if (c->interface == crspd->oldintfc && 
					corr_c->interface == crspd->newintfc)
		entry = incmtrx[corr_ci][ci];
	else if (c->interface == crspd->newintfc && 
					corr_c->interface == crspd->oldintfc)
		entry = incmtrx[ci][corr_ci];
	
	if (!corr_possible(entry))
	{
		screen("ERROR in correspond_curves_agree_in_orient(), ");
		screen("Curves not in correspondence\n");
		clean_up(ERROR);
	}
	return	orients_agree(entry);
}		/*end correspond_curves_agree_in_orient*/

LOCAL	CURVE *closest_curve_in_list(
	CURVE		*c,
	HS_LIST		*Hsl,
	INTERFACE	*intfc)
{
	BOND		*b[2];
	CURVE		*corr_c;
	HS_LIST		*hsl;
	HYPER_SURF	*hs_on;
	HYPER_SURF_ELEMENT	*hse_on;
	double		min_dist;
	double		t_on[MAXD], coords_on[MAXD], *coords;
	int		i;
	USE_BOUNDARIES	on_bdry;

	on_bdry = (is_bdry(c)) ? INCLUDE_BOUNDARIES : NO_BOUNDARIES;

	b[0] = c->first;	b[1] = c->last;
	corr_c = NULL;
	while (b[0] && b[1])
	{
	    for (i = 0; i < 2; ++i)
	    {
		coords = (i == 0) ? Coords(b[0]->start) : Coords(b[1]->end);
		hsl = Hsl->next;
		if (nearest_interface_point(coords,negative_component(c),intfc,
					    on_bdry,hsl->hs,coords_on,t_on,
					    &hse_on,&hs_on) != YES)
	        {
		    screen("ERROR in closest_curve_in_list(), "
		           "nearest_interface_point() failed\n");
		    clean_up(ERROR);
	        }
		min_dist = hsl->dist = sqr(coords[0] - coords_on[0]) +
				       sqr(coords[1] - coords_on[1]);
		for (hsl = hsl->next; hsl; hsl = hsl->next)
		{
		    if (nearest_interface_point(coords,negative_component(c),
						intfc,on_bdry,hsl->hs,coords_on,
						t_on,&hse_on,&hs_on) != YES)
	            {
		        screen("ERROR in closest_curve_in_list(), "
		               "nearest_interface_point() failed\n");
		        clean_up(ERROR);
	            }
		    hsl->dist = sqr(coords[0] - coords_on[0]) +
				sqr(coords[1] - coords_on[1]);
		    if (hsl->dist < min_dist) min_dist = hsl->dist;
		}

		/* Eliminate hypersurface with distance > min_dist */

		for (hsl = Hsl->next; hsl; hsl = hsl->next)
		{
		    if (hsl->dist > min_dist)
		    {
			if (hsl->prev) hsl->prev->next = hsl->next;
			if (hsl->next) hsl->next->prev = hsl->prev;
			free(hsl);
		    }
		}
		if (Hsl->next == NULL) 
		{
		    return NULL;
		}
		else if (Hsl->next->next == NULL) /* Unique candidate */
		{
		    corr_c = Curve_of_hs(Hsl->next->hs);
		    free(Hsl->next);
		    Hsl->next = NULL;
		    return corr_c;
		}
	    }
	    if (b[0] == b[1] || b[0]->prev == b[1]) break;
	    b[0] = b[0]->next;	b[1] = b[1]->prev;
	}
	return corr_c;
}		/*end closest_curve_in_list*/


/*
*			find_correspond_curve():
*
*	Locates the curve on interface intfc corresponding to c.
*	Valid for 2D only.
*/

EXPORT CURVE *find_correspond_curve(
	CURVE		*c,
	NODE		*ns,
	NODE		*ne,
	Front		*fr,
	INTERFACE	*intfc)
{
	HYPER_SURF	*hs;
	HYPER_SURF_BDRY *p_hsb[2], *n_hsb[2];

	if (intfc->dim != 2) return NULL;

	p_hsb[0] = (ns != NULL) ? Hyper_surf_bdry(ns) : NULL;
	p_hsb[1] = NULL;
	n_hsb[0] = (ne != NULL) ? Hyper_surf_bdry(ne) : NULL;
	n_hsb[1] = NULL;
	hs = find_correspond_hyper_surface(Hyper_surf(c),p_hsb,n_hsb,fr,intfc);
	return (hs != NULL) ? Curve_of_hs(hs) : NULL;
}		/*end find_correspond_curve*/


EXPORT	boolean find_correspond_of_oriented_curve(
	O_CURVE		*c,
	O_CURVE		*corr_c,
	NODE		*corr_node,
	Front		*fr,
	INTERFACE	*intfc)
{
	NODE		*ns, *ne;

	corr_c->curve = NULL;

	if (intfc->dim != 2)	return NO;
	if (c->curve == NULL)	return YES;
	ns = (c->orient == POSITIVE_ORIENTATION) ? corr_node : NULL;
	ne = (c->orient == NEGATIVE_ORIENTATION) ? corr_node : NULL;
	corr_c->curve = find_correspond_curve(c->curve,ns,ne,fr,intfc);
	if (corr_c->curve == NULL) return NO;
	corr_c->orient = 
		(correspond_curves_agree_in_orient(c->curve,corr_c->curve)) ?
			c->orient : Opposite_orient(c->orient);
	return YES;
}		/*end find_correspond_of_oriented_curve*/


/*
*			node_corresponding_to():
*
*	Using the curve correspondence INTO the interface of node,
*	but not the reverse direction correspondence, a unique node
*	correspondence is determined, and the corresponding node
*	for a given interface intfc is found. If this
*	correspondence is not well defined, for example because some
*	of the curves correspond to NULL, then NULL is returned.
*
*	Note: passive boundaries are not considered in the curve
*	correspondence as they don't provide a valid link between nodes on
*	different interfaces.  For example, if a bifurcation has occurred
*	around a fixed node with a passive boundary, there may no longer
*	be any correspondence between the non-passive boundaries, which are
*	the curves of interest.  However, the passive boundaries have not
*	changed and will still be in correspondence.
*/

EXPORT NODE *node_corresponding_to(
	NODE		*n,
	Front		*fr)
{
	INTERFACE	*old_intfc = fr->interf;
	INTERFACE	*new_intfc = n->interface;
	CURVE		**c, **oldc;
	CURVE		*cc;
	NODE		*ans = NULL;

	for (c = n->in_curves; c && *c; ++c)
	{
	    if (is_passive_boundary(*c))
	        continue;
	    for (oldc = old_intfc->curves; oldc && *oldc; ++oldc)
	    {
	    	cc = find_correspond_curve(*oldc,NULL,n,fr,new_intfc);
		    if (cc == *c)
		    	break;
	    }
	    if (!*oldc)
	    	continue;
	    if (node_type((*oldc)->end) != node_type(n))
	    	continue;
	    if (!ans)
	    	ans = (*oldc)->end;
	    else if (ans != (*oldc)->end)
	    	return NULL;
	}
	for (c = n->out_curves; c && *c; ++c)
	{
	    if (is_passive_boundary(*c)) continue;
	    for (oldc = old_intfc->curves; oldc && *oldc; ++oldc)
	    {
	    	cc = find_correspond_curve(*oldc,n,NULL,fr,new_intfc);
	    	if (cc == *c)
	    	    break;
	    }
	    if (!*oldc)
	    	continue;
	    if (node_type((*oldc)->start) != node_type(n))
	    	continue;
	    if (!ans)
	    	ans = (*oldc)->start;
	    else if (ans != (*oldc)->start)
	    	return NULL;
	}
	return ans;
}		/*end node_corresponding_to*/


EXPORT	boolean rst_cor_after_split_curve(
	CURVE		*curve,
	CURVE		**curves)
{

	CORRESPOND	*crspd;
	HYPER_SURF	**newhs, **oldhs;
	INTERFACE	*intfc = curve->interface;
	int		**row, *rowc, *rowc0, *rowc1;
	int		j, jc, jc0, jc1;

	DEBUG_ENTER(rst_cor_after_split_curve)
	if (DEBUG)
	{
	    (void) printf("curve:\n");
	    (void) printf("hypersurf of curve = %llu\n",
	    	          hypersurface_number(Hyper_surf(curve)));
	    print_curve(curve);
	    (void) printf("curves[0]:\n");
	    (void) printf("hypersurf of curves[0] = %llu\n",
	    	          hypersurface_number(Hyper_surf(curves[0])));
	    print_curve(curves[0]);
	    (void) printf("curves[1]:\n");
	    (void) printf("hypersurf of curves[1] = %llu\n",
	    	          hypersurface_number(Hyper_surf(curves[1])));
	    print_curve(curves[1]);
	}

	if (correspond_hyper_surf(curve))
	    correspond_hyper_surf(correspond_hyper_surf(curve)) = NULL;
	correspond_hyper_surf(curve) = NULL;
	correspond_hyper_surf(curves[0]) = NULL;
	correspond_hyper_surf(curves[1]) = NULL;
	for (crspd = first_cor; crspd != NULL; crspd = crspd->next)
	{
	    if (intfc == crspd->newintfc)
	    {
	    	/* Find rows of curve, curves[0] and curves[1] */

	    	rowc = rowc0 = rowc1 = NULL;
	    	for (newhs = crspd->newhs, row = crspd->incmtrx; 
	    			newhs && *newhs; ++newhs, ++row)
	    	{
	    	    if (*newhs == Hyper_surf(curve))
	    	    	rowc = *row;
	    	    else if (*newhs == Hyper_surf(curves[0]))
	    	    	rowc0 = *row;
	    	    else if (*newhs == Hyper_surf(curves[1]))
	    	    	rowc1 = *row;
	    	}
	    	if (rowc == NULL || rowc0 == NULL || rowc1 == NULL) 
	    	    return NO;

	    	/* Copy rowc into rowc0 and rowc1 */

	    	for (j = 0; j < crspd->num_oldhs; ++j)
	    	{
	    	    rowc0[j] = rowc1[j] = rowc[j];
	    	}
	    	if (DEBUG)
	    	    print_correspondence(crspd);
	    }
	    else if (intfc == crspd->oldintfc)
	    {
	    	/*Find columns of curve, curves[0] and curves[1] */

	    	jc = jc0 = jc1 = -1;
	    	for (oldhs = crspd->oldhs, j = 0; oldhs && *oldhs;
	    						++oldhs, ++j)
	    	{
	    	    if (*oldhs == Hyper_surf(curve))
	    	    	jc = j;
	    	    else if (*oldhs == Hyper_surf(curves[0]))
	    	    	jc0 = j;
	    	    else if (*oldhs == Hyper_surf(curves[1]))
	    	    	jc1 = j;
	    	}
	    	if (jc < 0 || jc0 < 0 || jc1 < 0)
		    return NO;

	    	/* Copy column of curve into columns of */
	    	/* curve[0] and curves[1]		*/

	    	for (row = crspd->incmtrx, j = 0; j < crspd->num_newhs; 
	    	     ++j, ++row)
	    	{
	    	    (*row)[jc0] = (*row)[jc1] = (*row)[jc];
	    	}
	    	if (DEBUG)
	    		print_correspondence(crspd);
	    }
	}
	DEBUG_LEAVE(rst_cor_after_split_curve)
	return YES;
}		/*end rst_cor_after_split_curve*/



EXPORT	boolean rst_cor_after_invert_curve(
	CURVE		*curve)
{

	CORRESPOND	*crspd;
	HYPER_SURF	**newhs, **oldhs;
	INTERFACE	*intfc = curve->interface;
	int		**row;
	int		i,j;

	DEBUG_ENTER(rst_cor_after_invert_curve)
	if (DEBUG)
	    print_curve(curve);

	if (correspond_hyper_surf(curve))
	    correspond_hyper_surf(correspond_hyper_surf(curve)) = NULL;
	correspond_hyper_surf(curve) = NULL;
	for (crspd = first_cor; crspd != NULL; crspd = crspd->next)
	{
	    if (intfc == crspd->newintfc)
	    {
	    	for (newhs = crspd->newhs, row = crspd->incmtrx; 
	    	     newhs && *newhs; ++newhs, ++row)
	    	{
	    	    if (*newhs != Hyper_surf(curve)) continue;
	    	    for (j = 0; j < crspd->num_oldhs; ++j)
	    	    {	
	    		if (corr_possible((*row)[j]))
	    			Invert_flag((*row)[j],ORIENTS_AGREE);
	    	    }
	    	    break;
	    	}
	    	if (DEBUG)
	    		print_correspondence(crspd);
	    }
	    else if (intfc == crspd->oldintfc)
	    {
	    	for (oldhs = crspd->oldhs, j = 0; oldhs && *oldhs; ++j, ++oldhs)
	    	{
	    	    if (*oldhs != Hyper_surf(curve)) continue;
	    	    for (row = crspd->incmtrx, i = 0; i < crspd->num_newhs;
	    		 ++i, ++row)
	    	    {
	    		if (corr_possible((*row)[j]))
	    		    Invert_flag((*row)[j],ORIENTS_AGREE);
	    	    }
	    	    break;
	    	}
	    	if (DEBUG)
	    	    print_correspondence(crspd);
	    }
	}
	DEBUG_LEAVE(rst_cor_after_invert_curve)
	return YES;
}		/*end rst_cor_after_invert_curve*/


EXPORT	boolean rst_cor_after_attach_curve_to_node(
	CURVE		*c1,
	CURVE		*c2)
{

	CORRESPOND	*crspd;
	HYPER_SURF	**newhs, **oldhs;
	INTERFACE	*intfc = c1->interface;
	int		**row, *rowc1, *rowc2;
	int		j, jc1, jc2;

	DEBUG_ENTER(rst_cor_after_attach_curve_to_node)
	if (DEBUG)
	{
	    (void) printf("c1:\n");
	    print_curve(c1);
	    (void) printf("c2:\n");
	    print_curve(c2);
	}

	if (correspond_hyper_surf(c1))
	    correspond_hyper_surf(correspond_hyper_surf(c1)) = NULL;
	correspond_hyper_surf(c1) = NULL;

	if (c2 == NULL)
	{
	    DEBUG_LEAVE(rst_cor_after_attach_curve_to_node)
	    return YES;
	}
	correspond_hyper_surf(c2) = NULL;
	for (crspd = first_cor; crspd != NULL; crspd = crspd->next)
	{
	    if (intfc == crspd->newintfc)
	    {
	    	rowc1 = rowc2 = NULL;
	    	for (newhs = crspd->newhs, row = crspd->incmtrx; 
	    			newhs && *newhs; ++newhs, ++row)
	    	{
	    	    if (*newhs == Hyper_surf(c1))
	    	    	rowc1 = *row;
	    	    else if (*newhs == Hyper_surf(c2))
	    	    	rowc2 = *row;
	    	}
	    	if (rowc1 == NULL || rowc2 == NULL)
	    	{
	    	    DEBUG_LEAVE(rst_cor_after_attach_curve_to_node)
	    	    return NO;
	    	}
	    	for (j = 0; j < crspd->num_oldhs; ++j)
	    	{
	    	    rowc2[j] = rowc1[j];
	    	}
	    	if (DEBUG)
	    	    print_correspondence(crspd);
	    }
	    else if (intfc == crspd->oldintfc)
	    {
	    	jc1 = jc2 = -1;
	    	for (oldhs=crspd->oldhs, j=0; oldhs && *oldhs; ++oldhs, ++j)
	    	{
	    	    if (*oldhs == Hyper_surf(c1))
	       	    	jc1 = j;
	    	    else if (*oldhs == Hyper_surf(c2))
	    	    	jc2 = j;
	    	}
	    	if (jc1 < 0 || jc2 < 0)
	    	{
	    	    DEBUG_LEAVE(rst_cor_after_attach_curve_to_node)
	    	    return NO;
	    	}
	    	for (row=crspd->incmtrx, j=0; j<crspd->num_newhs; ++j, ++row)
	    	{
	    	    (*row)[jc2] = (*row)[jc1];
	    	}
	    	if (DEBUG)
	    	    print_correspondence(crspd);
	    }
	}
	DEBUG_LEAVE(rst_cor_after_attach_curve_to_node)
	return YES;
}		/*end rst_cor_after_attach_curve_to_node*/
#endif /* defined(TWOD) */
