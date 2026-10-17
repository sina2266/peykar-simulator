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
*				funtan2d.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	The routines in this file untangle a interface.
*/


#define DEBUG_STRING	"hypunr"
#include <front/fdecs.h>

#define Cross_pro(p0,p1,p2)						   \
	((Coords(p1)[0] - Coords(p0)[0])*(Coords(p2)[1] - Coords(p0)[1]) - \
	 (Coords(p1)[1] - Coords(p0)[1])*(Coords(p2)[0] - Coords(p0)[0]))

#define Dot_pro(p0,p1,p2)						   \
	((Coords(p1)[0] - Coords(p0)[0])*(Coords(p2)[0] - Coords(p0)[0]) + \
	 (Coords(p1)[1] - Coords(p0)[1])*(Coords(p2)[1] - Coords(p0)[1]))

struct _Snlist {
	CURVE		**nc;
	NODE    	**nopp, *sn;
	FT_POINT		**pt;
	double		*ang;
	ORIENTATION	*orient;
	int		num_c;
	int		nc_set, nopp_set, pt_set, ang_set, orient_set;
} ;
typedef struct _Snlist Snlist;

struct _BOX_INFO
{
	CRXING ***x_crx;		/* crossing of x edges */
	CRXING ***y_crx;		/* crossing of y edges */
	CRXING *crx_store;		/* memory storage array of crossings */
	int **num_x_crx;		/* number of x-edge crossings */
	int **num_y_crx;		/* number of y-edge crossings */
	int num_crx;
	COMPONENT **comp;		/* components at box grid corner */
	BOND **in_bonds;		/* reconstructed bonds in box */
	CURVE **in_curves;		/* curves of the bonds in box */
	int num_in_bonds;		/* number of reconstructed bonds */
	BOND **out_bonds;		/* bonds cut by the box boundary */
	CURVE **out_curves;		/* curves of the cut bonds */
	int num_out_bonds;		/* number of cut bonds */
	CURVE **curves;			/* curves associated with the box */
	boolean *is_entirely_inside;	/* flag for entirely inside curve */
	int num_curves;			/* number of curves of above */
};
typedef struct _BOX_INFO BOX_INFO;

struct _CELL_INFO
{
	COMPONENT comp[4];	/* comps at four corners: SW,SE,NE,NW */
	CRXING *crxs[4];	/* crxings at four edges: S, E, N, W  */
	int num_cell_crx;	/* number of crxings in a cell: 0,2,4 */
};
typedef struct _CELL_INFO CELL_INFO;

	/* LOCAL Function Declarations */
LOCAL	boolean	check_phys_loops_on_snlist(Snlist*);
LOCAL	boolean	correct_and_identify_node_types(Front*);
LOCAL	boolean	is_node_in_node_list(NODE*,NNLIST*);
LOCAL	boolean	pass1_delete_unphys_curves(Front*,NNLIST**,double,int);
LOCAL	boolean	pass2_delete_unphys_curves(Front*,NNLIST**,double,int);
LOCAL	double	Pangle(FT_POINT*,FT_POINT*,FT_POINT*);
LOCAL	double	area_of_loop(NNLIST*,int);
LOCAL	int	total_number_curves_at_node(NODE*);
LOCAL	void	alloc_sn_list(Snlist*);
LOCAL	void	check_physical_loops_at_sink_nodes(INTERFACE*);
LOCAL	void	delete_from_nnlist(NNLIST*,NNLIST**);
LOCAL	void	free_sn_list(Snlist*);
LOCAL	void	print_NNLIST_struct(NNLIST*);
LOCAL	void	print_Snlist(Snlist*);
LOCAL	void	set_curve_info_in_new_node_list(NNLIST*);
LOCAL	void	set_curve_list_for_node(Snlist*);
LOCAL	void	set_index_for_phys_sectors(NNLIST*);
/*NEW*/
LOCAL	void 	set_2d_boxes(CROSS*,RECT_GRID*,RECT_BOX**);
LOCAL	boolean 	connected_boxes(RECT_BOX*,RECT_BOX*);
LOCAL 	boolean 	box_untangle(Front*,RECT_BOX*,CROSS**);
LOCAL	boolean 	set_box_comp_and_crx(INTERFACE*,RECT_BOX*,BOX_INFO*);
LOCAL	boolean 	reconstruct_box_bonds(INTERFACE*,RECT_BOX*,BOX_INFO*);
LOCAL	boolean 	reconnect_box_bonds(INTERFACE*,RECT_BOX*,BOX_INFO*);
LOCAL	boolean 	make_cell_bonds(CELL_INFO*,BOX_INFO*);
LOCAL	boolean 	check_and_reorganize_curves(INTERFACE*,BOX_INFO*);
LOCAL	void 	install_box_crossings(INTERFACE*,RECT_BOX*,BOX_INFO*);
LOCAL	void 	insert_box_crossings_in_curves(INTERFACE*,RECT_BOX*,BOX_INFO*);
LOCAL	void 	check_make_inside_loop_curve(INTERFACE*,BOX_INFO*,RECT_BOX*);
LOCAL	void 	check_make_outside_loop_curve(INTERFACE*,BOX_INFO*,RECT_BOX*);
LOCAL	void 	rotate_cell(CELL_INFO*);
LOCAL	void 	reclt_after_grid_based_untangle(CROSS*,BOX_INFO*);
LOCAL	void	count_box_crossings(INTERFACE*,RECT_BOX*,BOX_INFO*);
LOCAL	void 	xgraph_tangled_box(const char*,INTERFACE*,RECT_BOX*,int);
LOCAL	void 	xgraph_box_bonds(const char*,BOX_INFO*,RECT_BOX*,int);
LOCAL	void 	show_box_topology(RECT_BOX*,int**,int**,COMPONENT**);
LOCAL 	void 	set_corner_components(INTERFACE*,RECT_BOX*,BOX_INFO*);
LOCAL 	void 	set_side_components(INTERFACE*,RECT_BOX*,BOX_INFO*);
LOCAL 	void 	set_interior_components(INTERFACE*,RECT_BOX*,BOX_INFO*);
LOCAL 	void 	remove_box_unphysical_crx(INTERFACE*,RECT_BOX*,BOX_INFO*);
LOCAL	boolean 	bond_in_box(BOND*,INTERFACE*,RECT_BOX*);
LOCAL	boolean 	bond_enclose_point(BOND*,FT_POINT*);
LOCAL	boolean 	out_box_bond(BOND*,RECT_BOX*);
LOCAL	boolean 	out_box_point(FT_POINT*,RECT_BOX*);
LOCAL	boolean 	point_on_box_bdry(FT_POINT*,RECT_BOX*);
LOCAL	boolean 	in_clockwise_order(BOND*,CURVE*,BOND*,CURVE*,RECT_BOX*);
LOCAL 	boolean 	new_closed_loop(BOND*,int*,NODE*);
LOCAL   boolean    seg_cross_bond(int,BOND*,double,double,double,FT_POINT*);
LOCAL	CURVE 	*copy_curve_without_geom(CURVE*,NODE*,NODE*);


/*
*			is_vector_vector_cross():
*/

EXPORT	boolean	is_vector_vector_cross(
	CROSS		*cr)
{
	if (	wave_type(cr->c1) >= FIRST_VECTOR_PHYSICS_WAVE_TYPE &&
		wave_type(cr->c2) >= FIRST_VECTOR_PHYSICS_WAVE_TYPE)
		return YES;
	return NO;
}		/*end is_vector_vector_cross*/

/*
*			is_scalar_vector_cross():
*/

EXPORT	boolean	is_scalar_vector_cross(
	CROSS		*cr)
{
	if (is_vector_vector_cross(cr) == YES)
		return NO;
	if (	wave_type(cr->c1) < FIRST_PHYSICS_WAVE_TYPE ||
		wave_type(cr->c2) < FIRST_PHYSICS_WAVE_TYPE)
		return NO;
	if (	wave_type(cr->c1) >= FIRST_VECTOR_PHYSICS_WAVE_TYPE ||
		wave_type(cr->c2) >= FIRST_VECTOR_PHYSICS_WAVE_TYPE)
		return YES;
	return NO;
}		/*end is_scalar_vector_cross*/


/*
*			scalar_unravel():
*
*	The main routine for untangling a scalar interface.
*	This routine assumes that all node types on the incoming
*	interface are known and thus after the crossing points
*	are inserted they are the only nodes with unknown type.
*/


EXPORT	int scalar_unravel(
	Front		*fr,
	CROSS		**crx,
	int		flag)
{
	INTERFACE	*intfc = fr->interf;
	INTERFACE	*hold_intfc;
	CROSS		*cross;
	NNLIST		*new_node_list, *nnlist, *nl;
	CROSS		*cr, *crb;
	CURVE		**c;
	CURVE		*curves1[2], *curves2[2];
        NODE		*m, **n;
	double		*h = fr->rect_grid->h;
	double		hx = h[0], hy = h[1];
	double		min_area = 2.0*hx*hy;
	int		dim = fr->rect_grid->dim;
	int		j, num;

	DEBUG_ENTER(scalar_unravel)
	cross = *crx;

	if (cross == NULL)
	{
	    DEBUG_LEAVE(scalar_unravel)
	    return CURVES_UNTANGLED;
	}

	hold_intfc = current_interface();
	set_current_interface(intfc);
	if (DEBUG)
	{
	    (void) printf("In scalar_unravel(), intfc %llu cross %p\n",
			  interface_number(intfc),(POINTER)cross);
	    for (c = intfc->curves; c && *c;  c++)
		    print_bond_list(*c);

	    /* output debugging information */

	    for (num = 0, cr = cross;  cr;  num++, cr = cr->next)
		    ;

	    (void) printf("\nThe hyp interface is tangled at %d Point(s)\n",
			  num);
	    (void) printf("Here is the cross list of intersections:\n\n");

	    for (cr = cross;  cr;  cr = cr->next)
		    print_cross(cr);

	    (void) output();
	    (void) printf("\t\tHERE IS THE HYP INTERFACE TO BE UNTANGLED\n\n");
	    if (debugging("states"))
		    show_intfc_states(intfc);
	    else
		    print_interface(intfc);
	}

/* PREPROCESS the cross_list to spot crossings very close to each other */

label_1:
	if (cross == NULL)
	{
	    set_current_interface(hold_intfc);
	    DEBUG_LEAVE(scalar_unravel)
	    return CURVES_UNTANGLED;
	}
	for  (cr = cross; cr->next; cr = cr->next)
	{
	    for (crb = cr->next; crb; crb = crb->next)
	    {
		double	sclen;

		sclen = scaled_separation(cr->p,crb->p,h,dim);
		if (DEBUG)
		{
		    (void) printf("Testing crosses %p %p for close crosses, "
		                  "Scaled separation = %g\n",
				  (POINTER)cr,(POINTER)crb,sclen);
		}

#define are_redundant_crosses(cr1,cr2,dim)				\
	((separation((cr1)->p,(cr2)->p,(dim)) <				\
		 			EPSILON*bond_length((cr1)->b1))	\
			 	&&					\
	 (((cr1)->b1 == (cr2)->b1) || ((cr1)->b1 == (cr2)->b2) ||	\
	  ((cr1)->b2 == (cr2)->b1) || ((cr1)->b2 == (cr2)->b2)))

		if (are_redundant_crosses(cr,crb,dim) && NO)
		{
	    	    char xg_name[100];
		    double *center = Coords(cr->p);
		    double radius = 3.0*h[0];
	    	    sprintf(xg_name,"untangle");
#if defined __MPI__
	    	    sprintf(xg_name,"%s-%d",xg_name,pp_mynode());
#endif /* defined __MPI__ */
		    xgraph_2d_intfc_within_range(xg_name,intfc,center,radius,
						NO);
		    (void) printf("WARNING in scalar_unravel(), "
		                  "redundant crosses detected\n");
		    set_current_interface(hold_intfc);
		    DEBUG_LEAVE(scalar_unravel)
		    return ERROR_IN_UNTANGLE;
		}
		else if (sclen < MIN_SC_SEP(intfc))
		{
		    BOND	*cb1, *cb2, *cbb1, *cbb2,
		    		*cbb1n, *cbb1p, *cbb2n, *cbb2p;

				/* RESOLVE THE TWO CROSSES */

		    if (DEBUG)
			(void) printf("eliminate two close crosses %p, %p \n",
				      (POINTER)cr,(POINTER)crb);


		    cb1   = cr->b1;		cbb1  = crb->b1;
		    cb2   = cr->b2;		cbb2  = crb->b2;


		    if (cr->c1 == crb->c1 && is_closed_curve(cr->c1))
		    {
			if ((cb1->prev == NULL) && (cbb1->next == NULL))
			    (void) move_closed_loop_node(cr->c1,cb1->next);
			if ((cb1->next == NULL) && (cbb1->prev == NULL))
			    (void) move_closed_loop_node(cr->c1,cbb1->next);
		    }
		    if (cr->c1 == crb->c2 && is_closed_curve(cr->c1))
		    {
			if ((cb1->prev == NULL) && (cbb2->next == NULL))
			    (void) move_closed_loop_node(cr->c1,cb1->next);
			if ((cb1->next == NULL) && (cbb2->prev == NULL))
			    (void) move_closed_loop_node(cr->c1,cbb2->next);
		    }
		    if (cr->c2 == crb->c1 && is_closed_curve(cr->c2))
		    {
			if ((cb2->prev == NULL) && (cbb1->next == NULL))
			    (void) move_closed_loop_node(cr->c2,cb2->next);
			if ((cb2->next == NULL) && (cbb1->prev == NULL))
			    (void) move_closed_loop_node(cr->c2,cbb1->next);
		    }
		    if (cr->c2 == crb->c2 && is_closed_curve(cr->c2))
		    {
			if ((cb2->prev == NULL) && (cbb2->next == NULL))
			    (void) move_closed_loop_node(cr->c2,cb2->next);
			if ((cb2->next == NULL) && (cbb2->prev == NULL))
			    (void) move_closed_loop_node(cr->c2,cbb2->next);
		    }

		    cbb1p = cbb1->prev;		cbb2p = cbb2->prev;
		    cbb1n = cbb1->next;		cbb2n = cbb2->next;

		    if     (cb1 == cbb1n)
			(void) delete_start_of_bond(cb1, cr->c1);
		    else if (cb1 == cbb1p)
			(void) delete_start_of_bond(cbb1,crb->c1);
		    else if (cb2 == cbb2n)
			(void) delete_start_of_bond(cb2, cr->c2);
		    else if (cb2 == cbb2p)
			(void) delete_start_of_bond(cbb2,crb->c2);
		    else if (cb2 == cbb1n)
			(void) delete_start_of_bond(cb2, cr->c2);
		    else if (cb2 == cbb1p)
			(void) delete_start_of_bond(cbb1,crb->c1);
		    else if (cb1 == cbb2n)
			(void) delete_start_of_bond(cb1, cr->c1);
		    else if (cb1 == cbb2p)
			(void) delete_start_of_bond(cbb2,crb->c2);
		    else
		    {
			(void) printf("WARNING in scalar_unravel(), "
			              "Nothing done\n");
			set_current_interface(hold_intfc);
			DEBUG_LEAVE(scalar_unravel)
			return ERROR_IN_UNTANGLE;
		    }
					
		    if (DEBUG)
		    {
			(void) printf("Near crosses found and "
			              "resolution attempted\n");
			(void) printf("Recalling intersections\n");
		    }

		    if (intersections(intfc,crx,NO) == FUNCTION_FAILED)
		    {
			(void) printf("WARNING in scalar_unravel(), "
			              "intersections() failed\n");
			set_current_interface(hold_intfc);
			DEBUG_LEAVE(scalar_unravel)
			return ERROR_IN_UNTANGLE;
		    }
		    cross = *crx;
		    goto label_1;
		}
	    }
	}
/* END OF PREPROCESSING */

	if (cross == NULL)
	{
	    set_current_interface(hold_intfc);
	    DEBUG_LEAVE(scalar_unravel)
	    return CURVES_UNTANGLED;
	}

	new_node_list = (NNLIST *)Store(sizeof(NNLIST));
	new_node_list->prev = NULL;
	new_node_list->next = NULL;
	nnlist = new_node_list;
	nnlist->num_phys = 0;
	for (j = 0; j < 4; j++)
	    nnlist->area[j] = HUGE_VAL;

/* 
* PRIMARY LOOP ON CROSS LIST, in which the cross points are replaced by nodes 
*/
	
	if (DEBUG)
	    (void) printf("CROSS LOOP\n");
	for (cr = cross;  cr != NULL;  cr = cr->next)
	{
	    if (DEBUG)
	    	(void) printf("cr %p\n",(POINTER)cr);
		
	    split_curves_at_cross(cr,fr,&m,curves1,NULL,NULL,curves2,NULL,
			          NULL,MIN_SC_SEP(intfc),NULL);
	    nnlist->m = m;


	    if (cr->next != NULL)
	    {
	    	nnlist->next = (NNLIST *)Store(sizeof(NNLIST));
	    	nnlist->next->prev = nnlist;
	    	nnlist = nnlist->next;
	    	nnlist->next = NULL;
	    	nnlist->num_phys = 0;
	    	for (j = 0; j < 4; j++)
	    	    nnlist->area[j] = HUGE_VAL;
	    }
		/* New_node list is now generated */

	}

/* END OF PRIMARY LOOP */

		/*  Eliminate redundant CLOSED nodes */
redundant_delete:
	for (n = intfc->nodes; *n; n++)
	{
	    if (DEBUG)
	       (void) printf("Check node %llu to eliminate\n",node_number(*n));
	    if (node_type(*n) != CLOSED_NODE)
		continue;
	    if (delete_redundant_node(*n,NULL,NULL,fr))
		goto redundant_delete;
	}

		/* Insert curve info in new_node_list */

	set_curve_info_in_new_node_list(new_node_list);

	if (DEBUG)
	{
	    (void) printf("HYP INTERFACE AFTER CROSS LIST LOOP\n");
	    if (debugging("states"))
	    	show_intfc_states(intfc);
	    else
	    	print_interface(intfc);
	}

		/* First pass at identifying physical sectors */

	set_index_for_phys_sectors(new_node_list);

	if (DEBUG)
	{
	    (void) printf("Node list after first set index\n");
	    for (nl = new_node_list; nl != NULL; nl = nl->next)
	    	print_NNLIST_struct(nl);
	}

	/* loop over new_node_list; identify and delete unphysical loops */

	if (pass1_delete_unphys_curves(fr,&new_node_list,min_area,flag) == NO)
	{
	    (void) printf("WARNING in scalar_unravel(), "
	                  "pass1_delete_unphys_curves() failed\n");
	    set_current_interface(hold_intfc);
	    DEBUG_LEAVE(scalar_unravel)
	    return ERROR_IN_UNTANGLE;
	}

	if (DEBUG)
	{
	    (void) printf("Node list after first delete "
	                  "unphysical curve loop\n");
	    for (nl = new_node_list; nl != NULL; nl = nl->next)
		print_NNLIST_struct(nl);
	}


	/* loop over new_node_list again to identify and delete unphysical
	   loops, resolve if possible which of the two unphysical loops
	   should be deleted if this fails delete the one with smallest area */

	if (pass2_delete_unphys_curves(fr,&new_node_list,min_area,flag) == NO)
	{
	    (void) printf("WARNING in scalar_unravel(), "
	                  "pass2_delete_unphys_curves() failed\n");
	    set_current_interface(hold_intfc);
	    DEBUG_LEAVE(scalar_unravel)
	    return ERROR_IN_UNTANGLE;
	}

	if (DEBUG)
	{
	    (void) printf("before corr \n");
	    if (debugging("states"))
	    	show_intfc_states(intfc);
	    else
	    	print_interface(intfc);
	}

	if (correct_and_identify_node_types(fr) == NO)
	{
	    (void) printf("WARNING in scalar_unravel(), "
	                  "correct_and_identify_node_types() failed\n");
	    set_current_interface(hold_intfc);
	    DEBUG_LEAVE(scalar_unravel)
	    return ERROR_IN_UNTANGLE;
	}
	if (DEBUG)
	{
	    (void) printf("After correct_and_identify_nodes\n");
	    if (debugging("states"))
	    	show_intfc_states(intfc);
	    else
	    	print_interface(intfc);
	}

		/* Check all loops remaining at sink nodes are physical */

	check_physical_loops_at_sink_nodes(intfc);

		/* update points_on_curve count -- should be unnecessary */
	
	update_num_points(intfc);

	if (DEBUG)
	{
	    (void) output();
	    (void) printf("UNTANGLED HYP INTERFACE:\n");
	    if (debugging("states"))
	    	show_intfc_states(intfc);
	    else
	    	print_interface(intfc);
	}

	set_current_interface(hold_intfc);
	DEBUG_LEAVE(scalar_unravel)
	return CURVES_UNTANGLED;
}		/*end scalar_unravel*/


/*
*			set_curve_info_in_new_node_list():
*
*	Insert curve information in node list
*/

LOCAL	void set_curve_info_in_new_node_list(
	NNLIST		*new_node_list)
{
	NNLIST		*nl;
	CURVE		**ci, **co, *c1;
	FT_POINT 		*pt[4];
        NODE    	*m;
	int		i, j;
	ORIENTATION	orient;
	double		a, an[4];

	DEBUG_ENTER(set_curve_info_in_new_node_list)
	for (nl = new_node_list; nl != NULL; nl = nl->next)
	{
		if (DEBUG)
			(void) printf("Inserting curve info in node_list\n");

		/* check if necessary to flip c1 and c3 */

		m = nl->m;
		ci = m->in_curves;	co = m->out_curves;
		nl->nc[0] = *ci;	nl->orient[0] = NEGATIVE_ORIENTATION;
		nl->nc[2] = *co;	nl->orient[2] = POSITIVE_ORIENTATION;
		ci++;			co++;
		nl->nc[1] = *ci;	nl->orient[1] = NEGATIVE_ORIENTATION;
		nl->nc[3] = *co;	nl->orient[3] = POSITIVE_ORIENTATION;

		for (i = 0; i < 4; i++)
		{
			nl->nn[i] = Node_of(nl->nc[i],
					Opposite_orient(nl->orient[i]));
			pt[i] = Point_adjacent_to_node(nl->nc[i],
						       nl->orient[i]);
		}

		if (DEBUG)
		{
		    (void) printf("Ordering the curves here ");
		    (void) printf("c[i] = %llu %llu %llu %llu\n",
			       curve_number(nl->nc[0]),curve_number(nl->nc[1]),
			       curve_number(nl->nc[2]),curve_number(nl->nc[3]));
		}

		for (i = 0; i < 3; i++)
			an[i] = Pangle(m->posn,pt[0],pt[i+1]);

		if (DEBUG)
			(void) printf("angles %g %g %g\n",an[0],an[1],an[2]);

			/* Order curves by angle */

		for (i = 0; i < 2; i++)
		{
			for (j = i+1; j < 3; j++)
			{
				if (an[j] < an[i])
				{
					a = an[i];
					m = nl->nn[i+1];
					c1 = nl->nc[i+1];
					orient = nl->orient[i+1];
					an[i] = an[j];	
					nl->nc[i+1] = nl->nc[j+1];
					nl->nn[i+1] = nl->nn[j+1];
					nl->orient[i+1] = nl->orient[j+1];
					an[j] = a;
					nl->nn[j+1] = m;
					nl->nc[j+1] = c1;
					nl->orient[j+1] = orient;
				}
			}
		}
		for (i = 0; i < 4; i++)
		{
		    nl->comp[i] = (nl->orient[i] == POSITIVE_ORIENTATION)?
			    negative_component(nl->nc[i]) :
				    positive_component(nl->nc[i]);
		    if (DEBUG)
		    {
		      (void) printf("Setting components for nl\n");
		      (void) printf("i %d nc %llu comps: l %d r %d  comp[] %d\n",
				    i,curve_number(nl->nc[i]),
				    negative_component(nl->nc[i]),
				    positive_component(nl->nc[i]),nl->comp[i]);
	            }
		}

		if (DEBUG)
			print_NNLIST_struct(nl);
	}
	DEBUG_LEAVE(set_curve_info_in_new_node_list)
}		/*end set_curve_info_in_new_node_list*/

/*
*			set_index_for_phys_sectors():
*
*	Set index for physical sectors using algorithm described in
*	Glimm, Grove, Lindquist, Mcbryan, Tryggvasson.
*/

LOCAL	void set_index_for_phys_sectors(
	NNLIST		*new_node_list)
{
	NNLIST		*nl;
	int		i;

	DEBUG_ENTER(set_index_for_phys_sectors)
	for (nl = new_node_list; nl != NULL; nl = nl->next)
	{
	    for (i = 0; i < 4; i++)
		nl->ni[i] = UNPHYSICAL;

	    for (i = 0; i < 4; i++)
	    {
	    	if (nl->nn[i] != nl->nn[(i+1)%4])
	    	    nl->ni[i] = PHYSICAL;
	    	else
	    	{
	    	    if ((nl->nn[i] == nl->m) && (nl->nc[i] == nl->nc[(i+5)%4]))
	    	    {
	    	    	nl->ni[(i+3)%4] = PHYSICAL;
	    	    	nl->ni[(i+1)%4] = PHYSICAL;
	    	    }
	    	}
	    }
	}
	DEBUG_LEAVE(set_index_for_phys_sectors)
}		/*end set_index_for_phys_sectors*/


/*
*			pass1_delete_unphys_curves():
*
*	First loop over new_node_list - identify and delete unphysical loops
*	First loop algorithm described in Glimm, Grove, Lindquist, McBryan,
*	Tryggvasson.
*/

LOCAL	boolean pass1_delete_unphys_curves(
	Front		*fr,
	NNLIST		**new_node_list,
	double		min_area,
	int		flag)
{
	NNLIST		*nl, *ml;
	CURVE		**c, *newc;
	int		j, i, count;

	DEBUG_ENTER(pass1_delete_unphys_curves)
restart_loop:

	for (nl = (*new_node_list); nl != NULL; nl = nl->next)
	{
		/* Node with less than 4 curves. Delete from new node list */
restart_pass:
	    count = 0;
	    for (c = nl->m->in_curves;  c && *c; c++) count++;
	    for (c = nl->m->out_curves; c && *c; c++) count++;

	    if  (count < 4)
	    {
	    	ml = nl;
	    	nl = nl->next;
	    	delete_from_nnlist(ml,new_node_list);
	    	if (nl == NULL) break;
	    	goto restart_pass;
	    }

	    nl->num_phys = 0;
	    j = 0;
	    for (i = 0; i < 4; i++)
	    {
	    	if (nl->ni[i] == PHYSICAL)
		    nl->num_phys++;
	    	else
		    j = i;
	    }

	    if (nl->num_phys == 3)     /* one unphys. loop, (loop j) */
	    {
	    	nl->area[j] = area_of_loop(nl,j);
	    	if (replace_unphys_loop(nl,new_node_list,&newc,fr,
	    			j,min_area,flag) == FUNCTION_FAILED)
	    	{
	    	    DEBUG_LEAVE(pass1_delete_unphys_curves)
	    	    return NO;
	    	}
	    	goto restart_loop;
	    }
	    if (nl->num_phys == 4)
	    {
	        (void) printf("WARNING in pass1_delete_unphys_curves(), ");
	        (void) printf("all sectors physical\n");
	        return NO;
	    }
	}
	DEBUG_LEAVE(pass1_delete_unphys_curves)
	return YES;
}		/*end pass1_delete_unphys_curves*/


/*
*			pass2_delete_unphys_curves():
*
*	Second loop over new_node_list - identify and delete unphysical loops
*	Second loop algorithm described in Glimm, Grove, Lindquist, Mcbryan,
*	Tryggvasson.
*/

LOCAL	boolean pass2_delete_unphys_curves(
	Front		*fr,
	NNLIST		**new_node_list,
	double		min_area,
	int		flag)
{
	CURVE		*newc, *cc;
	INTERFACE	*intfc = fr->interf;
	NNLIST		*nl, *ml;
	int		i, j, stat;
	double		area, ai[4];

	DEBUG_ENTER(pass2_delete_unphys_curves)
	nl = *new_node_list;

	while (nl != NULL)
	{
	    /* if the node has only two in and/or out curves, */
	    /* 	  delete it from the new node list 	  */

	    if (total_number_curves_at_node(nl->m) < 4)
	    {
		if (DEBUG)
			(void) printf("go to next_node_list\n");
		goto next_node_list;
	    }
		

	    for (i = 0; i < 4; i++) ai[i] = 0.0;

	    if (DEBUG)
		(void) printf("after set ai = 0\n");

	    for (i = 0; i < 4; i++)
	    {
		if (DEBUG)
			(void) printf("find ai\n");

		if (nl->ni[i] == UNPHYSICAL)
		{
			ai[i] = nl->area[i] = area_of_loop(nl,i);
		}
		else	ai[i] = HUGE_VAL;

		if (ai[i] < 0.0)
		{
			if (nl->nc[i]->start == nl->nc[i]->end)
				ai[i] = -ai[i];
			else
				ai[i] = HUGE_VAL;
		}
	    }

	    if (DEBUG)
	    {
		(void) printf(" the ai's found:\n");
		for (j = 0; j < 4; j++)
		{
			if (ai[j] == HUGE_VAL)
				(void) printf("\tai[%d] = UNASSIGNED\n",j);
			else
				(void) printf("\tai[%d] = %g\n",j,ai[j]);
		}
	    }

	    if (nl->num_phys == 2)
	    {
		INTERFACE *sav_intfc, *tmp_intfc;
		COMPONENT interior_comp[2], comp0, comp1, ext_comp;
		NODE	  *corr_m;
		boolean	  sav_interpolate;
		int	  unphys[2], k0, k1, k0n, k1n;

		for (i = 0, j = 0; i < 4; i++)
		{
			if (nl->ni[i] == UNPHYSICAL) unphys[j++] = i;
		}

			/*
			*	check for and handle this case
			*
			*	  -------------------------
			*    P	/  U          a          U  \  P
			*   ---------------------------------------
			*    P	\  U          b          U  /  P
			*	  -------------------------
			*  
			*	where loops 'a' and 'b' are both unphysical
			*	by deleting loop 'a' ('b') if its area is
			*	much smaller than loop 'b' ('a').
			*/

		k0 = unphys[0];		k0n = (k0 + 1) % 4;
		k1 = unphys[1];		k1n = (k1 + 1) % 4;
		if ((nl->nn[k0] == nl->nn[k0n]) && (nl->nn[k1] == nl->nn[k1n]))
		{
		    if (DEBUG)
		    {
			(void) printf("\nchecking bananna loops ");
			(void) printf("k0 %d k0n %d  k1 %d k1n %d\n",
				      k0,k0n,k1,k1n);
		    }
		    if (fabs(ai[k0]) < 0.01*fabs(ai[k1]))
		    {
			stat = replace_unphys_loop(nl,new_node_list,&newc,
						   fr,k0,min_area,flag);
			if (stat == FUNCTION_FAILED)
			{
			    DEBUG_LEAVE(pass2_delete_unphys_curves)
			    return NO;
			}
			if (DEBUG)
				(void) printf("deleting loop k0 k0n\n");
			goto next_node_list;
		    }
		    else if (fabs(ai[k1]) < 0.01*fabs(ai[k0]))
		    {
			stat = replace_unphys_loop(nl,new_node_list,&newc,
						   fr,k1,min_area,flag);
			if (stat == FUNCTION_FAILED)
			{
			    DEBUG_LEAVE(pass2_delete_unphys_curves)
			    return NO;
			}
			if (DEBUG)
				(void) printf("deleting loop k1 k1n\n");
			goto next_node_list;
		    }
		}

		for (j = 0; j < 2; j++)
		{
		    k0 = unphys[j];		k1 = (k0 + 1) % 4;

		    if ((nl->nn[k0] == nl->nn[k1])
		            &&
			(is_node_in_node_list(nl->nn[k0],*new_node_list) == NO)
		            &&
			comps_consistent_at_node(nl->nn[k0]))
		    {
			stat = replace_unphys_loop(nl,new_node_list,&newc,
					fr,unphys[(j+1)%2],min_area,flag);
			if (stat == FUNCTION_FAILED)
			{
			    DEBUG_LEAVE(pass2_delete_unphys_curves)
			    return NO;
			}
			goto next_node_list;
		    }
		    if (nl->orient[k0] == POSITIVE_ORIENTATION)
		    {
			comp0 = negative_component(nl->nc[k0]);
		    }
		    else
		    {
			comp0 = positive_component(nl->nc[k0]);
		    }
		    if (nl->orient[k1] == POSITIVE_ORIENTATION)
		    {
			comp1 = positive_component(nl->nc[k1]);
		    }
		    else
		    {
			comp1 = negative_component(nl->nc[k1]);
		    }
		    if (comp0 != comp1)
		    {
			if (DEBUG)
			{
			    (void) printf("Inconsistent components ");
			    (void) printf("inside loop %d of nl %p\n",
					  k0,(POINTER)nl);
			    stat = replace_unphys_loop(nl,new_node_list,&newc,
						       fr,k0,min_area,flag);
			    if (stat == FUNCTION_FAILED)
			    {
				DEBUG_LEAVE(pass2_delete_unphys_curves)
				return NO;
			    }
			    goto next_node_list;
			}
		    }
		    interior_comp[j] = comp0;
		}
		sav_intfc = current_interface();
		sav_interpolate = interpolate_intfc_states(intfc);
		interpolate_intfc_states(intfc) = NO;
		set_copy_intfc_states(NO);
		tmp_intfc = copy_interface(intfc);
		cc = correspond_curve(nl->nc[0]);
		corr_m = Node_of(cc,nl->orient[0]);
		for (i = 0; i < 4; i++)
		{
		    for (j = 0; j < i; j++)
		    {
			if (nl->nc[i] == nl->nc[j])
			    goto already_deleted;
		    }
		    (void) delete_curve(correspond_curve(nl->nc[i]));

		    already_deleted:
		    	;
		}
		(void) delete_node(corr_m);
		ext_comp = long_component(Coords(nl->m->posn),tmp_intfc);
		(void) delete_interface(tmp_intfc);
		set_current_interface(sav_intfc);
		set_copy_intfc_states(YES);
		interpolate_intfc_states(intfc) = sav_interpolate;
		set_correspond_hyper_surfaces_to_NULL(intfc);

		if ((interior_comp[0] == ext_comp) &&
		    (interior_comp[1] != ext_comp))
		{
		    stat = replace_unphys_loop(nl,new_node_list,&newc,
					       fr,unphys[0],min_area,flag);
		    if (stat == FUNCTION_FAILED)
		    {
			DEBUG_LEAVE(pass2_delete_unphys_curves)
			return NO;
		    }
		    goto next_node_list;
		}
		else if ((interior_comp[0] != ext_comp) &&
			 (interior_comp[1] == ext_comp))
		{
		    stat = replace_unphys_loop(nl,new_node_list,&newc,
					       fr,unphys[1],min_area,flag);
		    if (stat == FUNCTION_FAILED)
		    {
			DEBUG_LEAVE(pass2_delete_unphys_curves)
			return NO;
		    }
		    goto next_node_list;
		}
	    }
		/* find the loop with the smallest area */

	    if (DEBUG)
		(void) printf("find the smallest area\n");

	    area = HUGE_VAL;
	    i = -1;
	    for (j = 0; j < 4; j++)
	    {
		    if (ai[j] < area)
		    {
			    area = ai[j];
			    i = j;
		    }
	    }

	    if (i == -1)
	    {
		(void) printf("WARNING in pass2_delete_unphys_curves(), ");
		(void) printf("all areas HUGE_VAL\n");
		DEBUG_LEAVE(pass2_delete_unphys_curves)
		return NO;
	    }
	    if (DEBUG)
		(void) printf("smallest area for i %d\n",i);

	    stat = replace_unphys_loop(nl,new_node_list,&newc,
				       fr,i,min_area,flag);
	    if (stat == FUNCTION_FAILED)
	    {
		DEBUG_LEAVE(pass2_delete_unphys_curves)
		return NO;
	    }

	    if (DEBUG)
		(void) printf("after delete\n");

next_node_list:
	    if (new_node_list == NULL) break;
	    ml = nl;
	    nl = nl->next;
	    delete_from_nnlist(ml,new_node_list);
	    if (DEBUG)
		(void) printf("after delete from nnl\n");
	}
	DEBUG_LEAVE(pass2_delete_unphys_curves)
	return YES;
}		/*end pass2_delete_unphys_curves*/

LOCAL	int total_number_curves_at_node(
	NODE *node)
{
	CURVE **c;
	int count = 0;

	for (c = node->in_curves;  c && *c; c++) count++;
	for (c = node->out_curves; c && *c; c++) count++;

	if (DEBUG)
	      (void) printf("Curve count %d for node %llu \n",
			    count,node_number(node));
	return count;
}		/*end total_number_curves_at_node*/


/*ARGSUSED*/
EXPORT	boolean f_delete_loop(
	NNLIST		*nl,
	NNLIST		**new_node_list,
	CURVE		**newc,
	Front		*fr,
	int		i,
	double		min_area,
	int		flag)
{
	int		j; 

	DEBUG_ENTER(f_delete_loop)
	*newc = NULL;
	if (DEBUG)
	{
	    (void) printf("f_delete_loop() for sector i %d\n",i);
	    print_NNLIST_struct(nl);
	}

	if (fabs(nl->area[i]) > min_area)
	{
	    (void) printf("WARNING in f_delete_loop(), "
	                  "large loop unphysical\n");
	    if (flag != LAST_ATTEMPT_TO_UNTANGLE)
	    {
	        DEBUG_LEAVE(f_delete_loop)
	        return FUNCTION_FAILED;
	    }
	    else
	    {
	        (void) printf("Deleting large unphysical loop on third "
	                      "attempt to untangle front\n");
	    }
	}
	j = (i + 1) % 4;
	(void) delete_curve(nl->nc[j]);
	if (nl->nc[j] != nl->nc[i])
	    (void) delete_curve(nl->nc[i]);
	if (DEBUG)
	    (void) printf("Leaving f_delete_loop()\n");
	
	DEBUG_LEAVE(f_delete_loop)
	return FUNCTION_SUCCEEDED;
}		/*end f_delete_loop*/

EXPORT	boolean f_replace_unphys_loop(
	NNLIST		*nl,
	NNLIST		**new_node_list,
	CURVE		**newc,
	Front		*fr,
	int		i,
	double		min_area,
	int		flag)
{

	NNLIST		*nlopp;
	Locstate	st;
	int		im1, ip1, ip2;
	int		k, km1, kp2;
	size_t		szst = fr->sizest;

	DEBUG_ENTER(f_replace_unphys_loop)
	*newc = NULL;
	if (DEBUG)
	{
	    (void) printf("f_replace_unphys_loop() for sector i %d\n",i);
	    print_NNLIST_struct(nl);
	}

	if (fabs(nl->area[i]) > min_area)
	{
	    (void) printf("WARNING in f_replace_unphys_loop(), "
	                  "large loop unphysical\n");
	    if (flag != LAST_ATTEMPT_TO_UNTANGLE)
	    {
	        DEBUG_LEAVE(f_replace_unphys_loop)
	        return FUNCTION_FAILED;
	    }
	    else
	    {
	        (void) printf("Replacing large unphysical loop on third "
	                      "attempt to untangle front\n");
	    }
	}
		/* Test whether to replace loop by single curve */

	im1 = (i+3)%4;	ip1 = (i+1)%4;	ip2 = (i+2)%4;

	if (DEBUG)
	{
	    (void) printf("test for replacement by single curve\n");
	    (void) printf("im1 %d  i %d  ip1 %d  ip2 %d\n",im1,i,ip1,ip2);
	}

		/* WARNING: tests on nl->comp[] are not reliable     */
		/*	    enough for the replacement decision      */
		/*	    OR for deciding on component ft_assignments */
		/*	    Note how nl->comp[] was ft_assigned !!	  */

	nl->comp[ip1] = (nl->orient[ip2] == POSITIVE_ORIENTATION)?
			positive_component(nl->nc[ip2]) :
			negative_component(nl->nc[ip2]);
	nl->comp[im1] = (nl->orient[im1] == POSITIVE_ORIENTATION)?
			negative_component(nl->nc[im1]) :
			positive_component(nl->nc[im1]);

	if (((nl->comp[ip1] != (nl->comp[im1]))) &&
	    (nl->nc[ip1] != nl->nc[i]) &&
	    (wave_type(nl->nc[ip1]) == wave_type(nl->nc[i])))
	{
	    for (nlopp = *new_node_list; nlopp != NULL; nlopp = nlopp->next)
	    	if (nlopp->m == nl->nn[i]) break;

	    if (nlopp == NULL)
	    {
	    	(void) printf("WARNING in f_replace_unphys_loop(), "
	    	              "Unable to find opposite node list\n");
	    	DEBUG_LEAVE(f_replace_unphys_loop)
	    	return f_delete_loop(nl,new_node_list,newc,
	    			     fr,i,min_area,flag);
	    }
	    if (total_number_curves_at_node(nlopp->m) < 4)
	    {
	    	(void) printf("WARNING in f_replace_unphys_loop(), "
		              "Opposite node list has < 4 curves\n");
		DEBUG_LEAVE(f_replace_unphys_loop)
		return FUNCTION_FAILED;
	    }

	    for (k = 0; k < 4; k++)
	    	if (nlopp->nc[k] == nl->nc[ip1]) break;

	    km1 = (k+3) % 4;	kp2 = (k + 2) % 4;

	    *newc = make_curve(nl->comp[ip1],nl->comp[im1],nl->m,nl->nn[i]);
	    wave_type(*newc) = wave_type(nl->nc[ip1]);

	    st = (nl->orient[im1] == POSITIVE_ORIENTATION) ?
					left_start_state(nl->nc[im1]) :
					right_end_state(nl->nc[im1]);
	    ft_assign(right_start_state(*newc),st,szst);

	    st = (nl->orient[ip2] == POSITIVE_ORIENTATION) ?
					right_start_state(nl->nc[i]) :
					left_end_state(nl->nc[i]);
	    ft_assign(left_start_state(*newc),st,szst);

	    st = (nlopp->orient[kp2] == POSITIVE_ORIENTATION) ?
					right_start_state(nlopp->nc[kp2]) :
					left_end_state(nlopp->nc[kp2]);
	    ft_assign(right_end_state(*newc),st,szst);

	    st = (nlopp->orient[km1] == POSITIVE_ORIENTATION) ?
					left_start_state(nlopp->nc[km1]) :
					right_end_state(nlopp->nc[km1]);
	    ft_assign(left_end_state(*newc),st,szst);

	    delete_from_nnlist(nlopp,new_node_list);
	}
	(void) delete_curve(nl->nc[ip1]);
	if (nl->nc[ip1] != nl->nc[i])
	    (void) delete_curve(nl->nc[i]);
	
	DEBUG_LEAVE(f_replace_unphys_loop)
	return FUNCTION_SUCCEEDED;
}		/*end f_replace_unphys_loop*/


LOCAL   void delete_from_nnlist(
	NNLIST		*nl,
	NNLIST		**new_node_list)
{
		/* NNLIST members created by calls to Store(). */
		/* All storage will be freed when appropriate  */
		/* interface is deleted. Merely unlink NNLIST  */
		/* 		    element		       */

	DEBUG_ENTER(delete_from_nnlist)
	if ((nl->next == NULL) && (nl->prev == NULL))
	{
		*new_node_list = NULL;
		DEBUG_LEAVE(delete_from_nnlist)
		return;
	}
	if (nl->next == NULL)
	{
		nl->prev->next = NULL;
	} 
 	else if (nl->prev == NULL)
	{
		nl->next->prev = NULL;
		*new_node_list = nl->next;
	}
	else
	{
		nl->next->prev = nl->prev;
		nl->prev->next = nl->next;
	}

	DEBUG_LEAVE(delete_from_nnlist)
}		/*end delete_from_nnlist*/

LOCAL	boolean is_node_in_node_list(
	NODE		*node,
	NNLIST		*nnlist)
{
	NNLIST		*nl;

	for (nl = nnlist; nl != NULL; nl = nl->next)
	    if (nl->m == node)
		    return YES;
	
	return NO;
}		/*end is_node_in_node_list*/

LOCAL	void print_NNLIST_struct(
	NNLIST		*nl)
{
	int		i;

	(void) printf("NNLIST %p:  prev %p next %p\n",(POINTER)nl,
		      (POINTER)nl->prev,(POINTER)nl->next);
	(void) printf("\tnode m %llu  pos: %g %g\n\n",
		      node_number(nl->m),
		      Coords(nl->m->posn)[0],Coords(nl->m->posn)[1]);
	for (i = 0; i < 4; i++)
	{
		(void) printf("\ti %d   nc %llu   nn %llu   comp %d\n",
			      i,curve_number(nl->nc[i]),
			      node_number(nl->nn[i]),nl->comp[i]);
		(void) printf("\torient %s   ni %s\n",
			      (nl->orient[i] == POSITIVE_ORIENTATION) ?
			      "POSITIVE ORIENTATION" : "NEGATIVE ORIENTATION",
			      (nl->ni[i] == PHYSICAL) ?
			      		"PHYSICAL" : "UNPHYSICAL");
		if (nl->area[i] == HUGE_VAL)
			(void) printf("\tArea of loop  UNASSIGNED\n\n");
		else
			(void) printf("\tArea of loop %g\n\n",nl->area[i]);
	}
	(void) printf("\tNumber of physical regions %d\n",nl->num_phys);
	(void) printf("\n");
}		/*end print_NNLIST_struct*/

LOCAL 	double area_of_loop(
	NNLIST		*nl,
	int		i)
{
	double		area, a;
	int		j;
	BOND		*b;
	FT_POINT		*p0;

	DEBUG_ENTER(area_of_loop)

	j = (i+1)%4;
	p0 = nl->m->posn;
	area = 0.0;
	a = 0.0;

	for (b = nl->nc[i]->first; b; b = b->next)
		area = area + Cross_pro(p0,b->start,b->end);
	if (nl->nc[i]->first->start != nl->m->posn)
		area = -area;


	if (DEBUG)
		(void) printf("Area %g  curve %llu\n",
			      area,curve_number(nl->nc[i]));

	if (nl->nc[i] != nl->nc[j])
	{
		for (b = nl->nc[j]->first; b; b = b->next)
			a = a + Cross_pro(p0,b->start,b->end);
		if (nl->nc[j]->last->end != nl->m->posn)
			a = -a;

		if (DEBUG)
			(void) printf("A %g  curve %llu\n",
				      a,curve_number(nl->nc[j]));
	}
	area = area + a;

	if (DEBUG)
	 	(void) printf("leaving area_of_loop  area %g\n",area);

	DEBUG_LEAVE(area_of_loop)
	return area;
}		/*end area_of_loop*/



LOCAL 	double Pangle(
	FT_POINT		*p0,
	FT_POINT		*p1,
	FT_POINT		*p2)
{
	double		x,y,a;

	DEBUG_ENTER(Pangle)
	if (DEBUG)
	{
		(void) printf("Entered Pangle\n");
		(void) printf("p0 %llu  x %g  y %g\n",
			      point_number(p0),Coords(p0)[0],Coords(p0)[1]);
		(void) printf("p1 %llu  x %g  y %g\n",
			      point_number(p1),Coords(p1)[0],Coords(p1)[1]);
		(void) printf("p2 %llu  x %g  y %g\n",
			      point_number(p2),Coords(p2)[0],Coords(p2)[1]);
	}

	y = Dot_pro(p0,p1,p2);
	x = Cross_pro(p0,p1,p2);
	a = atan2(x,y);


	if (DEBUG)
		(void) printf("Dot(x): %g  Cross(y): %g  a: %g\n",x,y,a);

	if (a < 0.0)
		a = 2.*PI + a;

	if (DEBUG)
		(void) printf("Return %g\n",a);

	DEBUG_LEAVE(Pangle)
	return a;
}		/*end Pangle*/




LOCAL	boolean correct_and_identify_node_types(
	Front		*fr)
{
	INTERFACE	*intfc = fr->interf;
	NODE		**n;
	CURVE 		*c1, *c2, *cur;
	boolean		sav_interp;

	DEBUG_ENTER(correct_and_identify_node_types)
restart_loop:
	for (n = intfc->nodes;  n && *n;  n++)
	{
		if (DEBUG)
		{
			(void) printf("Resolving node type at node %llu\n",
				      node_number(*n));
			print_node(*n);
		}

		if ((node_type(*n) != ERROR) &&
		    (node_type(*n) != CLOSED_NODE) &&
		    (node_type(*n) != UNKNOWN_NODE_TYPE))
			continue;
		
			/* identify probable cases */

			/* closed loops */
		if (is_node_of_closed_curve_only(*n))
			node_type(*n) = CLOSED_NODE;

		else if (((*n)->in_curves== NULL) &&
			((*n)->out_curves == NULL))
		{
			(void) delete_node(*n);
			goto restart_loop;
		} 

			/* two shock nodes */
		else if (    (*n)->in_curves && (*n)->out_curves &&
			(*n)->in_curves[0]  && (!(*n)->in_curves[1])
		 && (*n)->out_curves[0] && (!(*n)->out_curves[1]))
		{
		    c1 = (*n)->in_curves[0];
		    c2 = (*n)->out_curves[0];

		    if ((negative_component(c1) != negative_component(c2))
			    			||
			(positive_component(c1) != positive_component(c2)))
		    {
			(void) printf("WARNING in ");
			(void) printf("correct_and_identify_node_types(), ");
			(void) printf("Inconsistent components\n");
			if (DEBUG)
				print_interface(intfc);
			DEBUG_LEAVE(correct_and_identify_node_types)
			return NO;
		    }
		    sav_interp = interpolate_intfc_states(intfc);
		    interpolate_intfc_states(intfc) = YES;
		    cur = join_curves(c1,c2,negative_component(c1),
				      positive_component(c2),(BOND **)NULL);
		    interpolate_intfc_states(intfc) = sav_interp;
		    if (cur == NULL)
		    {
			(void) printf("WARNING in ");
			(void) printf("correct_and_identify_node_types(), ");
			(void) printf("join_curves() returns NULL\n");
		        DEBUG_LEAVE(correct_and_identify_node_types)
			return NO;
		    }
		    (void) delete_node(*n);
		    goto restart_loop;
		}
		else if ((!(*n)->in_curves) &&
			 ((*n)->out_curves[0] && (*n)->out_curves[1]
					 && (!(*n)->out_curves[2])))
		{
		    c1 = (*n)->out_curves[0];
		    c2 = (*n)->out_curves[1];
		    invert_curve(c1);

		    if ((negative_component(c1) != negative_component(c2))
			    			||
			(positive_component(c1) != positive_component(c2)))
		    {
			(void) printf("WARNING in ");
			(void) printf("correct_and_identify_node_types(), ");
			(void) printf("Inconsistent components\n");
			if (DEBUG)
				print_interface(intfc);
			DEBUG_LEAVE(correct_and_identify_node_types)
			return NO;
		    }
		    sav_interp = interpolate_intfc_states(intfc);
		    interpolate_intfc_states(intfc) = YES;
		    cur = join_curves(c1,c2,negative_component(c1),
				      positive_component(c1),(BOND **)NULL);
		    interpolate_intfc_states(intfc) = sav_interp;
		    if (cur == NULL)
		    {
			(void) printf("WARNING in ");
			(void) printf("correct_and_identify_node_types(), ");
			(void) printf("join_curves() returns NULL\n");
		        DEBUG_LEAVE(correct_and_identify_node_types)
			return NO;
		    }
		    (void) delete_node(*n);
		    goto restart_loop;
		}
		else if ((!(*n)->out_curves) &&
			 ((*n)->in_curves[0] && (*n)->in_curves[1]
					 && (!(*n)->in_curves[2])))
		{
		    c1 = (*n)->in_curves[0];
		    c2 = (*n)->in_curves[1];
		    invert_curve(c2);

		    if ((negative_component(c1) != negative_component(c2))
			    			||
			(positive_component(c1) != positive_component(c2)))
		    {
			(void) printf("WARNING in ");
			(void) printf("correct_and_identify_node_types(), ");
			(void) printf("Inconsistent components\n");
			if (DEBUG)
				print_interface(intfc);
			DEBUG_LEAVE(correct_and_identify_node_types)
			return NO;
		    }
		    sav_interp = interpolate_intfc_states(intfc);
		    interpolate_intfc_states(intfc) = YES;
		    cur = join_curves(c1,c2,negative_component(c1),
				      positive_component(c1),(BOND **)NULL);
		    interpolate_intfc_states(intfc) = sav_interp;
		    if (cur == NULL)
		    {
			(void) printf("WARNING in ");
			(void) printf("correct_and_identify_node_types(), ");
			(void) printf("join_curves() returns NULL\n");
		        DEBUG_LEAVE(correct_and_identify_node_types)
			return NO;
		    }
		    (void) delete_node(*n);
		    goto restart_loop;
		}
		else
			(*fr->identify_physical_node)(*n);
	}
	DEBUG_LEAVE(correct_and_identify_node_types)
	return YES;
}		/*end correct_and_identify_node_types*/


/* 
*			eliminate_small_loops():
*
*	This routine finds and eliminates small loops generated by a self-
*	intersecting loop. It is quite crude, and slow, and calls
*	intersections()	after each loop has been eliminated. This routine
*	can be called in the beginning of untangle and reduces the
*	possibility of double crossings. If double crossings don't occur
*	it is a slow and painful way of doing only a part of what the
*	unravel routine can do.
*/

EXPORT	void eliminate_small_loops(
	INTERFACE	*intfc,
	double		hx,
	double		hy,
	CROSS		**cross)
{
	CURVE		*c;
	BOND		*b1, *b2, *bb;
	FT_POINT		*p0;
	CROSS		*cr;
	double		area, min_area = hx*hy;

	DEBUG_ENTER(eliminate_small_loops)
restart:
	if (*cross == NULL)
	{
	    DEBUG_LEAVE(eliminate_small_loops)
	    return;
	}

	for (cr = *cross;  cr->next;  cr = cr->next)
	{
	    if (cr->c1 == cr->c2)
	    {
	    	c = cr->c1;
		p0 = cr->p;

		/* Make sure b2 follows b1 in bond list of curve */

		for (bb = c->first;  bb != c->last;  bb = bb->next)
		{
		    if (bb == cr->b1)
		    {
		    	b1 = cr->b1;	b2 = cr->b2;
		    	break;
		    }
		    if (bb == cr->b2)
		    {
		    	b1 = cr->b2;	b2 = cr->b1;
		    	break;
		    }
		}

		/* Compute area of loop */

		for (area = 0.0, bb = b1->next; bb != b2; bb = bb->next)
		    area += Cross_pro(p0,bb->start,bb->end);
		area = fabs(area);
		if (area > min_area)
		    continue;
			
		/* Delete loop with small area */

		interpolate_intfc_states(c->interface) = YES;
		if (insert_point_in_bond(p0,b2,c) != FUNCTION_SUCCEEDED)
	        {
	            screen("ERROR in eliminate_small_loops(), "
		           "insert_point_in_bond() failed\n");
	            clean_up(ERROR);
	        }
		for (bb = b1->next; bb != b2->next; bb = bb->next)
		    (void) delete_start_of_bond(bb,c);
		if (intersections(intfc,cross,NO) == FUNCTION_FAILED)
		{
		    screen("ERROR in eliminate_small_loops(), "
		           "intersections() failed\n");
		    clean_up(ERROR);
		}
		goto restart;
	    }
	} 
	DEBUG_LEAVE(eliminate_small_loops)
}		/*end eliminate_small_loops*/


LOCAL	void alloc_sn_list(
	Snlist	*snlist)
{
	uni_array(&snlist->nc    ,snlist->num_c,sizeof(POINTER));
	uni_array(&snlist->nopp  ,snlist->num_c,sizeof(POINTER));
	uni_array(&snlist->pt    ,snlist->num_c,sizeof(POINTER));
	uni_array(&snlist->ang   ,snlist->num_c,FLOAT);
	uni_array(&snlist->orient,snlist->num_c,INT);
	snlist->nc_set = 0;	snlist->nopp_set = 0;
	snlist->pt_set = 0;	snlist->ang_set = 0;
	snlist->orient_set = 0;
}		/*end alloc_sn_list*/

LOCAL	void free_sn_list(
	Snlist	*snlist)
{
	free(snlist->nc);
	free(snlist->nopp);
	free(snlist->pt);
	free(snlist->ang);
	free(snlist->orient);
	snlist->sn = NULL;
	snlist->num_c = 0;
	snlist->nc_set = 0;	snlist->nopp_set = 0;
	snlist->pt_set = 0;	snlist->ang_set = 0;
	snlist->orient_set = 0;
}		/*end free_sn_list*/

LOCAL	void print_Snlist(
	Snlist		*snlist)
{
	FT_POINT		*p;
	int		i;

	if (snlist == NULL)
	{
		(void) printf("Snlist unallocated\n");
		return;
	}
	if (snlist->num_c <= 0)
	{
		(void) printf("Snlist empty\n");
		return;
	}
	(void) printf("Snlist at node %llu\n",node_number(snlist->sn));
	print_node(snlist->sn);
	for (i = 0;  i < snlist->num_c;  i++)
	{
		if (snlist->nc_set)
			(void) printf("curve %llu ",curve_number(snlist->nc[i]));
		else
			(void) printf("curve NULL ");
		if (snlist->orient_set)
			(void) printf("orient %d ",snlist->orient[i]);
		else
			(void) printf("orient NULL ");
		if (snlist->ang_set)
			(void) printf("ang %g ",snlist->ang[i]);
		else
			(void) printf("ang NULL ");
		if (snlist->nopp_set)
		{
			p = (snlist->nopp[i])->posn;
			(void) printf("opp_node %g %g ",
				      Coords(p)[0],Coords(p)[1]);
		}
		else
			(void) printf("nopp NULL ");
		if (snlist->pt_set)
		{
			p = snlist->pt[i];
			(void) printf("pt %g %g\n",Coords(p)[0],Coords(p)[1]);
		}
		else
			(void) printf("pt NULL\n");
	}
}		/*end print_Snlist*/


LOCAL	void set_curve_list_for_node(
	Snlist		*snl)
{
	CURVE		**ci, **co, *c1;
        NODE		*sn, *m;
	FT_POINT		*p;
	int		i, j;
	ORIENTATION	orient;
	double		a;

	DEBUG_ENTER(set_curve_list_for_node)
	sn = snl->sn;		snl->num_c = 0;
	for (ci = sn->in_curves;  ci && *ci;  ci++)
		(snl->num_c)++;
	for (co = sn->out_curves;  co && *co;  co++)
		(snl->num_c)++;

	if (DEBUG)
	{
		(void) printf("Inserting curve info in Snlist for node -\n");
		print_node(sn);
		(void) printf("Num curves %d\n",snl->num_c);
	}

	alloc_sn_list(snl);

	i = 0;
	for (ci = sn->in_curves;  ci && *ci;  ci++)
	{
		snl->nc[i] = *ci;
		snl->orient[i] = NEGATIVE_ORIENTATION;
		snl->nopp[i] = (*ci)->start;
		snl->pt[i] = (*ci)->last->start;
		i++;
	}
	for (co = sn->out_curves;  co && *co;  co++)
	{
		snl->nc[i] = *co;
		snl->orient[i] = POSITIVE_ORIENTATION;
		snl->nopp[i] = (*co)->end;
		snl->pt[i] = (*co)->first->end;
		i++;
	}
	snl->nc_set = 1;	snl->orient_set = 1;
	snl->nopp_set = 1;	snl->pt_set = 1;
	if (DEBUG)
	{
		(void) printf("Initial Snlist load\n");
		print_Snlist(snl);
	}

	snl->ang[0] = 0.0;
	for (i = 1;  i < snl->num_c;  i++)
		snl->ang[i] = Pangle(sn->posn,snl->pt[0],snl->pt[i]);
	snl->ang_set = 1;

	if (DEBUG)
	{
		(void) printf("After setting angles\n");
		print_Snlist(snl);
	}

		/* Order curves by angle */

	for (i = 1;  i < snl->num_c-1;  i++)
	{
		for (j = i+1;  j < snl->num_c;  j++)
		{
			if (snl->ang[j] < snl->ang[i])
			{
				a      = snl->ang[i];
				m      = snl->nopp[i];
				p      = snl->pt[i];
				c1     = snl->nc[i];
				orient = snl->orient[i];

				snl->ang[i]    = snl->ang[j];	
				snl->nopp[i]   = snl->nopp[j];
				snl->pt[i]     = snl->pt[j];
				snl->nc[i]     = snl->nc[j];
				snl->orient[i] = snl->orient[j];

				snl->ang[j]    = a;
				snl->nopp[j]   = m;
				snl->pt[j]     = p;
				snl->nc[j]     = c1;
				snl->orient[j] = orient;
			}
		}
	}

	if (DEBUG)
	{
		(void) printf("After ordering angles\n");
		print_Snlist(snl);
	}
	DEBUG_LEAVE(set_curve_list_for_node)
}		/*end set_curve_list_for_node*/


/*
*			check_phys_loops_on_snlist():
*
*	Returns YES if all loops physical. Returns NO if unphysical
*	loop found. Also deletes unphysical loop.
*/

LOCAL	boolean check_phys_loops_on_snlist(
	Snlist		*snl)
{
	int		i, j, k;

	DEBUG_ENTER(check_phys_loops_on_snlist)
	if (snl->num_c <= 2)
	{
		if (DEBUG)
		{
			(void) printf(" <= 2 curves attached to S_node. ");
			(void) printf("Nothing checked\n");
		}
		DEBUG_LEAVE(check_phys_loops_on_snlist)
		return YES;
	}
	for (i = 0;  i < snl->num_c - 1;  i++)
	{
	    j = (i+1) % snl->num_c;
	    if (DEBUG)
		    (void) printf("i %d j %d ",i,j);

	    if (snl->nc[i] != snl->nc[j])
	    {
		    if (DEBUG)
			    (void) printf("no loop\n");
		    continue;
	    }
	    k = (i+2) % snl->num_c;
	    if (DEBUG)
	    {
		(void) printf("k %d\n",k);
		(void) printf("curve %llu ",curve_number(snl->nc[j]));
		(void) printf("negative component %d positive component %d\n",
			      negative_component((snl->nc[j])),
			      positive_component((snl->nc[j])));
		(void) printf("curve %llu ",curve_number(snl->nc[k]));
		(void) printf("negative component %d positive component %d\n",
			      negative_component((snl->nc[k])),
			      positive_component((snl->nc[k])));
	    }
	    if (snl->orient[j] == POSITIVE_ORIENTATION)
	    {
		if (snl->orient[k] == POSITIVE_ORIENTATION)
		{
		    if (negative_component((snl->nc[j]))
			!= positive_component((snl->nc[k])))
		    {
			(void) delete_curve(snl->nc[i]);
			if (DEBUG)
				(void) printf("Unphys loop %d deleted\n",i);
			DEBUG_LEAVE(check_phys_loops_on_snlist)
			return NO;
		    }
	        }
		else
		{
		    if (negative_component((snl->nc[j]))
			!= negative_component((snl->nc[k])))
		    {
			(void) delete_curve(snl->nc[i]);
			if (DEBUG)
				(void) printf("Unphys loop %d deleted\n",i);
			DEBUG_LEAVE(check_phys_loops_on_snlist)
			return NO;
		    }
	        }
	    }
	    else
	    {
		if (snl->orient[k] == POSITIVE_ORIENTATION)
		{
		    if (positive_component((snl->nc[j]))
			!= positive_component((snl->nc[k])))
		    {
			(void) delete_curve(snl->nc[i]);
			if (DEBUG)
				(void) printf("Unphys loop %d deleted\n",i);
			DEBUG_LEAVE(check_phys_loops_on_snlist)
			return NO;
		    }
	        }
		else
		{
		    if (positive_component((snl->nc[j]))
			!= negative_component((snl->nc[k])))
		    {
			(void) delete_curve(snl->nc[i]);
			if (DEBUG)
				(void) printf("Unphys loop %d deleted\n",i);
			DEBUG_LEAVE(check_phys_loops_on_snlist)
			return NO;
		    }
	        }
	    }
	}
	if (DEBUG)
	   (void) printf("All loops physical\n");
	DEBUG_LEAVE(check_phys_loops_on_snlist)
	return YES;
}		/*end check_phys_loops_on_snlist*/



LOCAL	void check_physical_loops_at_sink_nodes(
	INTERFACE	*intfc)
{
	Snlist		Snlist_store;
	Snlist		*snl = &Snlist_store;
	NODE		**n;

	DEBUG_ENTER(check_physical_loops_at_sink_nodes)
	for (n = intfc->nodes;  n && *n;  n++)
	{
		if (node_type(*n) != SINK_NODE)
			continue;

	reloop_node:
		snl->sn = *n;
		set_curve_list_for_node(snl);
		if (check_phys_loops_on_snlist(snl) == NO)
		{
			free_sn_list(snl);
			goto reloop_node;
		}
		else
			free_sn_list(snl);
	}
	DEBUG_LEAVE(check_physical_loops_at_sink_nodes)
}		/*end check_physical_loops_at_sink_nodes*/




#define GEN_CURVE
#if defined(GEN_CURVE)

/*
*	A generalized curve is a linked group of CURVES satisfying the 
*	following conditions:
*		1) each CURVE has the same wave type
*		2) each interior node has the same node type
*		3) each interior node has exactly 2 (not necessarily distinct)
*		   CURVEs attached to it - the previous CURVE of the
*		   generalized curve and the next one.  (PASSIVE_BOUNDARY
*		   curves at a node are not counted in this total unless
*		   the generalized curve is a PASSIVE_BOUNDARY.)
*		4) the CURVEs have a consistent orientation - i.e.
*		      c1->start => c1->end = c2->start => c2->end ...  or
*		      c1->end <= c1->start = c2->end <= c2->start ... 
*		   Usually the left and right components of the generalized
*		   curve are just the left and right components, respectively,
*		   of any CURVE within it.  This may not be TRUE when 
*		   PASSIVE_BOUNDARYs are present at interior nodes.
*
*	The idea is that a generalized curve would be a single CURVE except
*	that simple NODEs had to be added for some reason (e.g. to preserve the
*	shape of the curve).  Hopefully the above conditions will 
*	identify all such curves and not any unwanted curves.  
*
*	In the future it may be desirable to require consistency of other
*	fields in a CURVE (e.g. start and end states, boundary function,...)
*	Consistency of the boundary flag of CURVEs is intentionally not
*	included since situations like the following should be considered
*	generalized curves:    bdry-->-x  - passive bdry -  x->-bdry
*				       |                    |   
*				       |		    ^
*				       -->--internal bdry ->-
*	
*/

/*
*			opp_node_of_gen_curve():
*
*	Given a generalized curve which has orientation 'orient'
*	relative to one end node (the "beginning") and with "beginning" curve
*	'c', this routine returns the node at the opposite end (the "end") of 
*	the generalized curve.
*	(For a generalized curve containing a single curve, it is
*	equivalent to Node_of(c,Opposite_orient(orient))).
*
*	On error the routine returns NULL.
*/

EXPORT NODE *opp_node_of_gen_curve(
	CURVE		*c,
	ORIENTATION	orient)
{
	int		n_type;	/* node type of interior nodes of gen curve */
	NODE		*endn=NULL;/* node at "end" of each component curve */
	CURVE		*nextc,*prevc;

		/* preliminary error checking */

	DEBUG_ENTER(opp_node_of_gen_curve)
	if (DEBUG)
	{
		(void) printf("Entered opp_node_of_gen_curve()\n");
		(void) printf("c %llu orient %d\n",curve_number(c),orient);
	}
	if (!c) goto Exit;

	/**** check if c really is at beginning?
	      If c is not at the beginning, this routine may find the end
	      of the wrong generalized curve.  Problems can be prevented
	      by initializing n_type properly (not to UNKOWN) when c is not at
	      the beginning.  
	n_type = node_type(Node_of(c,orient));
	prevc = next_curve_of_gen_curve(c,Opposite_orient(orient),
					&n_type,&endn);
	if (!prevc) n_type = UNKNOWN_NODE_TYPE;
	*******/

		/* loop through the curves in the generalized curve 	*/
		/*   until the curve ends or closes back on itself 	*/

	prevc = c;
	nextc = NULL;
	n_type = UNKNOWN_NODE_TYPE;
	while (nextc != c)
	{			/* until gen curve closes */

		nextc = next_curve_of_gen_curve(prevc,orient,&n_type,&endn);
		if (nextc == NULL) break;	/* reached end of gen curve */
		prevc = nextc;
	}

Exit:
	if (DEBUG)
		(void) printf("Leaving opp_node_of_gen_curve() -  endn %llu\n",
			      node_number(endn));
	DEBUG_LEAVE(opp_node_of_gen_curve)
	return endn;

}		/*end opp_node_of_gen_curve*/


/*
*			next_curve_of_gen_curve():
*
*	This routine attempts to find the next curve after curve 'c' in
*	a generalized curve which has orientation 'orient' relative
*	to the "beginning" and has interior node type 'n_type'.
*	The node type at the "beginning" of the continuation curve will
*	not be checked if n_type = UNKNOWN_NODE_TYPE.
*
*	ON EXIT:
*		n_type	- node type of next_n, UNKNOWN_NODE_TYPE if c is NULL
*		next_n	- node at "end" of 'c', NULL if c is NULL
*		Returns the continuation if it is found, NULL otherwise.
*				  (NOTE: c closed => no continuation)
*/

EXPORT CURVE *next_curve_of_gen_curve(
	CURVE		*c,
	ORIENTATION	orient,
	int		*n_type,    /* node type of interior nodes on curve */
	NODE		**next_n) 		/* node at "end" of c */
{
	CURVE		*next_c = NULL;
	CURVE		**cp,**c_array;
	int		nn_type;
	int		nw_type,w_type;
	int		ignore_passive=YES;

		/* preliminary error checking */

	DEBUG_ENTER(next_curve_of_gen_curve)

	*next_n = NULL;
	if (DEBUG)
		(void) printf("c %llu orient %d n_type %d\n",
			      curve_number(c),orient,*n_type);
	if (!c)
	{
		*n_type = UNKNOWN_NODE_TYPE;
		DEBUG_LEAVE(next_curve_of_gen_curve)
		return NULL;
	}

		/* find the end node of c and check (or set) the node type */

	*next_n = Node_of(c,Opposite_orient(orient));
	nn_type = node_type(*next_n);
	if (DEBUG)
		(void) printf("next_n %llu nn_type %d\n",
			      node_number(*next_n),nn_type);
	if (*n_type != UNKNOWN_NODE_TYPE && *n_type != nn_type)
	{
		DEBUG_LEAVE(next_curve_of_gen_curve)
		return NULL;
	}
	*n_type = nn_type;

		/* check for closed curve */

	if (is_closed_curve(c))
	{
		if (DEBUG)
			(void) printf("curve is closed - returning NO\n");
		DEBUG_LEAVE(next_curve_of_gen_curve)
		return NULL;
	}
		
		/* loop through the in_curves or out_curves (depending on */
		/*         orient) to find the next valid curve  	  */

	w_type = wave_type(c);
	if (w_type == PASSIVE_BOUNDARY) ignore_passive=NO;
	if (DEBUG)
		(void) printf("w_type %d ignore_passive %d\n",
			      w_type,ignore_passive);
	if (orient == POSITIVE_ORIENTATION)
	{
		c_array = (*next_n)->out_curves;
		if (DEBUG)
		    (void) printf("searching through out_curves for next_c\n");
	}
	else
	{
		c_array = (*next_n)->in_curves;
		if (DEBUG)
		    (void) printf("searching through in_curves for next_c\n");
	}
	for (cp = c_array; cp && *cp; cp++)
	{
			/* ignore c and (conditionally) passive boundaries */

		if (c == *cp) continue;	/* really unneeded since c not closed*/
		nw_type = wave_type(*cp);
		if (nw_type == PASSIVE_BOUNDARY && ignore_passive) continue;

			/* found a curve - check that it is the only	*/ 
			/* curve at the node and check wave type	*/

		if (DEBUG)
			(void) printf("found curve %llu wave type %d\n",
				      curve_number(*cp),nw_type);
		if (next_c)
		{
			DEBUG_LEAVE(next_curve_of_gen_curve)
			return NULL;
		}
		next_c = *cp;
		if (nw_type != w_type)
		{
			DEBUG_LEAVE(next_curve_of_gen_curve)
			return NULL;
		}
	}
	if (!next_c) 	/* no curves found with proper orient*/
	{
		DEBUG_LEAVE(next_curve_of_gen_curve)
		return NULL;
	}

		/* look through remaining curves at node to make sure there
		*  are no more (non-ignored) curves
		*/

	if (orient == POSITIVE_ORIENTATION)
	{
		c_array = (*next_n)->in_curves;
		if (DEBUG)
			(void) printf("now checking in_curves for extras\n");
	}
	else
	{
		c_array = (*next_n)->out_curves;
		if (DEBUG)
			(void) printf("now checking out_curves for extras\n");
	}
	for (cp = c_array; cp && *cp; cp++)
	{

			/* ignore c and (conditionally) passive boundaries */

		if (*cp == c) continue;
		nw_type = wave_type(*cp);
		if (nw_type == PASSIVE_BOUNDARY && ignore_passive) continue;

		if (DEBUG)
			(void) printf("found extra curve %llu w_type %d\n",
				      curve_number(*cp),nw_type);
		DEBUG_LEAVE(next_curve_of_gen_curve)
		return NULL;
	}
		/* continuation curve found */

	if (DEBUG)
	{
		(void) printf("next_n %llu *n_type %d next_c %llu\n",
			     node_number(*next_n),*n_type,curve_number(next_c));
	}
	DEBUG_LEAVE(next_curve_of_gen_curve)
	return next_c;
}		/*end next_curve_of_gen_curve*/

#else /* defined(GEN_CURVE) */

	/*
	*	The following are versions of the above routines
	*	when "generalized" curves are not allowed.  That is,
	*	when generalized curves are just single CURVEs.
	*/

EXPORT NODE *opp_node_of_gen_curve(
	CURVE		*c,
	ORIENTATION	orient)
{
	return Node_of(c,Opposite_orient(orient));
}		/*end opp_node_of_gen_curve*/

/*ARGSUSED*/
EXPORT CURVE *next_curve_of_gen_curve(
	CURVE		*c,
	ORIENTATION	orient,
	int		*n_type,    /* node type of interior nodes on curve */
	NODE		**next_n)		/* node at "end" of c */
{
	/* no continuation if generalized curve is a single curve */

	*next_n = NULL;
	n_type = UNKNOWN_NODE_TYPE;
	return NULL;
}		/*end next_curve_of_gen_curve*/

#endif /* defined(GEN_CURVE) */

/*NEW*/

/*
*			f_grid_based_untangle():
*
*	This is a trial function using grid based method to untagle
*	complicated topological changes. It divide tangled section
*	into rectangular boxes, record interface grid crossings with
*	the grid segments in each box, remove unphysical crossings,
*	reconstruct curves in each box and then connect them to the
*	outside world.
*/


EXPORT	int f_grid_based_untangle(
	Front	*fr,
	CROSS	**crx)
{
	CROSS		*cr;
	INTERFACE 	*intfc = fr->interf;
	RECT_GRID 	*gr = &topological_grid(intfc);
	RECT_GRID	save_grid;
	RECT_GRID	Dual_grid;
	RECT_BOX 	*boxes,*box;
	boolean		status;
	boolean		sav_intrp = interpolate_intfc_states(intfc);
	int 		i;

	if (debugging("lgb2d"))
	    (void) printf("Entering f_grid_based_untangle()\n");
	if (*crx == NULL)
	{
	    if (debugging("lgb2d"))
		(void) printf("Leaving f_grid_based_untangle()\n");
	    return CURVES_UNTANGLED;
	}

	save_grid = *gr;
	set_dual_grid(&Dual_grid,gr);
	topological_grid(intfc) = Dual_grid;
	gr = &Dual_grid;

	if (debugging("lgb2d"))
	{
	    (void) printf("Check consistency of interface:\n");
	    if (consistent_interface(intfc))
	    	(void) printf("Entering f_grid_based_untangle(), "
			      "interface is consistent\n");
	}

	interpolate_intfc_states(intfc) = YES;
	make_interface_topology_lists(intfc);
	set_2d_boxes(*crx,gr,&boxes);
	if (debugging("lgb2d"))
	{
	    (void) printf("Before untangle:\n");
	    for (cr = *crx; cr; cr = cr->next)
	    	(void) printf("cr = %p\n",cr);
	}
	status = FUNCTION_SUCCEEDED;
	for (box = boxes; box; box = box->next)
	{
	    if (box->num_cross <= 2) continue;
	    if (debugging("lgb2d"))
	    {
		(void) printf("Untangling box:\n");
	    	(void) printf("box->bmin = %d %d\n",box->bmin[0],
				  		box->bmin[1]);
	    	(void) printf("box->bmax = %d %d\n",box->bmax[0],
				  		box->bmax[1]);
		(void) printf("Before untangle box %p:\n",box);
		for (cr = *crx; cr; cr = cr->next)
	    	    (void) printf("cr = %p\n",cr);
	    }
	    status = box_untangle(fr,box,crx);
	    if (status == FUNCTION_SUCCEEDED)
	    {
		if (debugging("lgb2d"))
		{
		    (void) printf("After untangle box %p:\n",box);
		    for (cr = *crx; cr; cr = cr->next)
	    	        (void) printf("cr = %p\n",cr);
		}
	    }
	    else
		goto leave;
	}
	if (debugging("lgb2d"))
	{
	    (void) printf("After untangle:\n");
	    for (cr = *crx; cr; cr = cr->next)
	    	(void) printf("cr = %p\n",cr);
	    (void) printf("Check consistency of interface:\n");
	    if (consistent_interface(intfc))
	    	(void) printf("Leaving f_grid_based_untangle(), "
			      "interface is consistent\n");
	}

leave:
	interpolate_intfc_states(intfc) = sav_intrp;
	topological_grid(intfc) = save_grid;
	make_interface_topology_lists(intfc);
	if (debugging("lgb2d"))
	    (void) printf("Leaving f_grid_based_untangle()\n");
	return (status == FUNCTION_SUCCEEDED) ? CURVES_UNTANGLED :
		    ERROR_IN_UNTANGLE;
}	/* end grid_based_untangle */

LOCAL	void set_2d_boxes(
	CROSS *cross,
	RECT_GRID *gr,
	RECT_BOX **boxes)
{
	RECT_BOX Box,*box,*nbox,*tmp_box;
	int i,j;
	int ip[MAXD];
	CROSS *cr;

	Box.prev = Box.next = NULL;
        box = &Box;

	for (cr = cross; cr; cr = cr->next)
	{
	    if (is_bdry(cr->c1) || is_bdry(cr->c2)) 
		continue;	/* this function does not solve for
				   boundary untangle */
	    rect_in_which(Coords(cr->p),ip,gr);
	    if (ip[0] < 2 || ip[0] >= gr->gmax[0] - 2) continue;
	    if (ip[1] < 2 || ip[1] >= gr->gmax[1] - 2) continue;
	    scalar(&box->next,sizeof(RECT_BOX));
	    box->next->prev = box;
	    box->next->next = NULL;
	    box = box->next;
	    box->num_cross = 0;
	    for (i = 0; i < 2; ++i)
	    {
		box->bmin[i] = ip[i]-1;
		box->bmax[i] = ip[i] + 2;
	    }
	    box->grid = gr;
	    box->cross[box->num_cross++] = cr;
	    if (debugging("lgb2d"))
	    {
	    	(void) printf("cr = %p cr->p = %f %f\n",cr,Coords(cr->p)[0],
			      Coords(cr->p)[1]);
	    	(void) printf("box->bmin = %d %d\n",box->bmin[0],box->bmin[1]);
	    	(void) printf("box->bmax = %d %d\n",box->bmax[0],box->bmax[1]);
	    }
	}
	/* merge boxes */
	for (box = Box.next; box != NULL; box = box->next)
	{
	    for (nbox = box->next; nbox != NULL; nbox = nbox->next)
	    {
		if (connected_boxes(box,nbox))
		{
		    if (debugging("lgb2d"))
		    {
		    	(void) printf("Merging connected boxes:\n");
	    	    	(void) printf(" box->bmin = %d %d\n",box->bmin[0],
				      	box->bmin[1]);
	    	    	(void) printf(" box->bmax = %d %d\n",box->bmax[0],
				      	box->bmax[1]);
	    	    	(void) printf("nbox->bmin = %d %d\n",nbox->bmin[0],
				      	nbox->bmin[1]);
	    	    	(void) printf("nbox->bmax = %d %d\n",nbox->bmax[0],
				      	nbox->bmax[1]);
		    	for (j = 0; j < box->num_cross; ++j)
			    (void) printf("box->cross[%d] = %p\n",
					  j,box->cross[j]);
		    	for (j = 0; j < nbox->num_cross; ++j)
			    (void) printf("nbox->cross[%d] = %p\n",
					  j,nbox->cross[j]);
		    }
		    for (i = 0; i < 2; ++i)
		    {
		    	box->bmin[i] = min(box->bmin[i],nbox->bmin[i]);
		    	box->bmax[i] = max(box->bmax[i],nbox->bmax[i]);
		    }
		    for (j = 0; j < nbox->num_cross; ++j)
			box->cross[box->num_cross+j] = nbox->cross[j];
		    box->num_cross += nbox->num_cross;
		    tmp_box = nbox->prev;
		    tmp_box->next = nbox->next;
		    if (nbox->next) nbox->next->prev = tmp_box;
		    free(nbox);
		    nbox = tmp_box;
		    if (debugging("lgb2d"))
		    {
		    	(void) printf("After merging:\n");
	    	    	(void) printf(" box->bmin = %d %d\n",box->bmin[0],
				      	box->bmin[1]);
	    	    	(void) printf(" box->bmax = %d %d\n",box->bmax[0],
				      	box->bmax[1]);
		    	for (j = 0; j < box->num_cross; ++j)
			    (void) printf("box->cross[%d] = %p\n",
					  j,box->cross[j]);
		    }
		}
	    }
	}
	*boxes = Box.next;
}	/* end set_2d_boxes */

LOCAL boolean box_untangle(
	Front *fr,
	RECT_BOX *box,
	CROSS **cross)
{
	INTERFACE *intfc = fr->interf;
	int i,*gmax = box->smax;
	boolean status;
	BOX_INFO box_info;
	NODE **n;
	CROSS *cr;
	static int count = 0;	/* for debugging purpose */

	if (debugging("lgb2d"))
	    (void) printf("Entering box_untangle()\n");
	for (i = 0; i < 2; ++i)
	    gmax[i] = box->bmax[i] - box->bmin[i];

	uni_array(&box_info.in_bonds,gmax[0]*gmax[1]*2,sizeof(BOND*));
	uni_array(&box_info.in_curves,gmax[0]*gmax[1]*2,sizeof(CURVE*));
	uni_array(&box_info.out_bonds,(gmax[0]+gmax[1])*4,sizeof(BOND*));
	uni_array(&box_info.out_curves,(gmax[0]+gmax[1])*4,sizeof(CURVE*));
	uni_array(&box_info.curves,10,sizeof(CURVE*));
	uni_array(&box_info.is_entirely_inside,10,INT);
	bi_array(&box_info.x_crx,gmax[0],gmax[1]+1,sizeof(CRXING*));
	bi_array(&box_info.y_crx,gmax[0]+1,gmax[1],sizeof(CRXING*));
	bi_array(&box_info.num_x_crx,gmax[0],gmax[1]+1,INT);
	bi_array(&box_info.num_y_crx,gmax[0]+1,gmax[1],INT);
	bi_array(&box_info.comp,gmax[0]+1,gmax[1]+1,sizeof(COMPONENT));

	if (debugging("lgb2d"))
	    xgraph_tangled_box("step-1",intfc,box,count);
	count_box_crossings(intfc,box,&box_info);
	uni_array(&box_info.crx_store,box_info.num_crx,sizeof(CRXING));
	install_box_crossings(intfc,box,&box_info);
	insert_box_crossings_in_curves(intfc,box,&box_info);
	if (debugging("lgb2d"))
	    xgraph_tangled_box("step-2",intfc,box,count);

	status = set_box_comp_and_crx(intfc,box,&box_info);
	if (status == FUNCTION_FAILED) 
	    goto leave_box_untangle;
	status = reconstruct_box_bonds(intfc,box,&box_info);
	if (debugging("lgb2d"))
	    xgraph_box_bonds("step-3",&box_info,box,count);
	if (status == FUNCTION_FAILED) 
	    goto leave_box_untangle;
	status = reconnect_box_bonds(intfc,box,&box_info);
	if (status == FUNCTION_SUCCEEDED)
	{
	    for (cr = *cross; cr; cr = cr->next)
	    {
		for (i = 0; i < box->num_cross; ++i)
		{
		    if (cr == box->cross[i])
		    {
			if (cr->prev) cr->prev->next = cr->next;
			else *cross = cr->next;
			if (cr->next) cr->next->prev = cr->prev;
		    }
		}
	    }
	    reclt_after_grid_based_untangle(*cross,&box_info);
	    for (n = intfc->nodes; n && *n; ++n)
	    	delete_redundant_node(*n,*cross,NULL,fr);
	    for (cr = *cross; cr; cr = cr->next)
	    {
	    	boolean is_b1_on_c1 = is_b_on_curve(cr->c1,cr->b1);
	    	boolean is_b2_on_c2 = is_b_on_curve(cr->c2,cr->b2);
		/*
	    	if (!is_b1_on_c1 || !is_b2_on_c2)
		{
		    if (cr->prev) cr->prev->next = cr->next;
		    else *cross = cr->next;
		    if (cr->next) cr->next->prev = cr->prev;
		}
		*/
		if (!is_b1_on_c1 && !is_b2_on_c2)
                {
                    if (cr->prev) cr->prev->next = cr->next;
                    else *cross = cr->next;
                    if (cr->next) cr->next->prev = cr->prev;
                }
                else if (!is_b1_on_c1 || !is_b2_on_c2)
                {
                    (void) printf("WARNING: bond of cross not on curve\n");
                    status = FUNCTION_FAILED;
                    goto leave_box_untangle;
                }
	    }
	}
	if (debugging("lgb2d"))
	{
	    xgraph_tangled_box("step-4",intfc,box,count);
	    count++;
	}

leave_box_untangle:
	free_these(10,box_info.in_bonds,box_info.in_curves,
		   box_info.out_bonds,box_info.out_curves,
		   box_info.x_crx,box_info.y_crx,
		   box_info.num_x_crx,box_info.num_y_crx,
		   box_info.comp,box_info.crx_store);
	if (debugging("lgb2d"))
	    (void) printf("Leaving box_untangle()\n");
	return status;
}	/* end box_untangle */

LOCAL void insert_box_crossings_in_curves(
	INTERFACE *intfc,
	RECT_BOX *box,
	BOX_INFO *box_info)
{
	int i,j,num_crx = box_info->num_crx;
	int num_out_bond = 0;
	CRXING *crx_store = box_info->crx_store;
	FT_POINT *p;
	BOND *b,*bout;
	CURVE *c;

	if (debugging("lgb2d"))
	    (void) printf("Entering insert_box_crossings_in_curves()\n");

	box_info->num_out_bonds = 0;
	for (i = 0; i < num_crx; ++i)
	{
	    p = crx_store[i].pt;
	    b = crx_store[i].bond;
	    c = Curve_of_hs(crx_store[i].hs);
	    if (Coords(p)[0] == Coords(b->start)[0] &&
	    	Coords(p)[1] == Coords(b->start)[1])
	    {
		crx_store[i].pt = b->start;
		b = crx_store[i].bond = b->prev;
	    }
	    else if (Coords(p)[0] == Coords(b->end)[0] &&
	    	Coords(p)[1] == Coords(b->end)[1])
		crx_store[i].pt = b->end;
	    else
	    	insert_point_in_bond(p,b,c);
	    if (out_box_bond(b,box) && point_on_box_bdry(p,box))
	    {
		box_info->out_curves[box_info->num_out_bonds] = c;
		box_info->out_bonds[box_info->num_out_bonds++] = b;
		crx_store[i].bond = b;
	    }
	    else if (out_box_bond(b->next,box) && point_on_box_bdry(p,box))
	    {
		box_info->out_curves[box_info->num_out_bonds] = c;
		box_info->out_bonds[box_info->num_out_bonds++] = b->next;
		crx_store[i].bond = b->next;
	    }
	    for (j = i+1; j < num_crx; ++j)
	    {
		if (b == crx_store[j].bond)
		{
		    crx_store[j].bond = 
			    (bond_enclose_point(b,crx_store[j].pt) == YES) ?
			    b : b->next;
		}
	    }
	}
	if (debugging("lgb2d"))
	    (void) printf("Leaving insert_box_crossings_in_curves()\n");
}	/* end insert_box_crossings_in_curves */

LOCAL	boolean make_cell_bonds(
	CELL_INFO *cell,
	BOX_INFO *box_info)
{
	int i,j;
	CURVE *c;
	CRXING *crx1,*crx2;

	if (cell->num_cell_crx == 0) return YES;
	else if (cell->num_cell_crx == 2)
	{
	    for (i = 0; i < 3; ++i)
	    {
		if ((crx1 = cell->crxs[0]) != NULL)
		{
		    c = Curve_of_hs(crx1->hs);
		    for (j = 1; j < 4; ++j)
			if (cell->crxs[j] != NULL)
			    crx2 = cell->crxs[j];
		    box_info->in_curves[box_info->num_in_bonds] = c;
		    if (cell->comp[0] == negative_component(c))
		    	box_info->in_bonds[box_info->num_in_bonds++] =
					Bond(crx1->pt,crx2->pt);
		    else
		    	box_info->in_bonds[box_info->num_in_bonds++] =
					Bond(crx2->pt,crx1->pt);
		    return YES;
		}
		else rotate_cell(cell);
	    }
	}
	else if (cell->num_cell_crx == 4)
	{
	    double d, d_shortest = HUGE;
	    int i_rot;
	    for (i = 0; i < 4; ++i)
	    {
		rotate_cell(cell);
		crx1 = cell->crxs[0];
		crx2 = cell->crxs[1];
		d = distance_between_positions(Coords(crx1->pt),
				Coords(crx2->pt),2);
		if (d < d_shortest)
		{
		    i_rot = i;
		    d_shortest = d;
		}
	    }
	    for (i = 0; i <= i_rot; ++i)
		rotate_cell(cell);
	    crx1 = cell->crxs[0];
	    crx2 = cell->crxs[1];
	    c = Curve_of_hs(crx1->hs);
	    box_info->in_curves[box_info->num_in_bonds] = c;
	    if (cell->comp[0] == negative_component(c))
		box_info->in_bonds[box_info->num_in_bonds++] =
					Bond(crx1->pt,crx2->pt);
	    else
		box_info->in_bonds[box_info->num_in_bonds++] =
					Bond(crx2->pt,crx1->pt);
	    rotate_cell(cell);
	    rotate_cell(cell);
	    crx1 = cell->crxs[0];
	    crx2 = cell->crxs[1];
	    c = Curve_of_hs(crx1->hs);
	    box_info->in_curves[box_info->num_in_bonds] = c;
	    if (cell->comp[0] == negative_component(c))
		box_info->in_bonds[box_info->num_in_bonds++] =
					Bond(crx1->pt,crx2->pt);
	    else
		box_info->in_bonds[box_info->num_in_bonds++] =
					Bond(crx2->pt,crx1->pt);
	    return YES;
	}
	else
	{
	    screen("ERROR: odd number of crxings in cell\n");
	    return NO;
	}
}	/* end make_cell_bonds */

LOCAL	void rotate_cell(CELL_INFO *cell)
{
	int i;
	CRXING *crx_tmp = cell->crxs[0];
	COMPONENT comp_tmp = cell->comp[0];
	for (i = 0; i < 3; ++i)
	{
	    cell->crxs[i] = cell->crxs[i+1];
	    cell->comp[i] = cell->comp[i+1];
	}
	cell->crxs[3] = crx_tmp;
	cell->comp[3] = comp_tmp;
}	/* rotate_cell */

LOCAL	void count_box_crossings(
	INTERFACE *intfc,
	RECT_BOX *box,
	BOX_INFO *box_info)
{
	struct Table *T = table_of_interface(intfc);
	int i,j,ii,jj,k,l,nb;
	BOND **bonds,*b,*tmp_b;
	CURVE **curves,**c,**bc;
	FT_POINT *ps,*pe,*p;
	double coords[3] = {0,0,0};
	int num_crx = 0;
	double *L = box->grid->L;
	double *h = box->grid->h;
	int **num_x_crx = box_info->num_x_crx;
	int **num_y_crx = box_info->num_y_crx;
	double   low,upp,seg;

	ps = Point(coords);
	pe = Point(coords);
	p  = Point(coords);
	tmp_b = Bond(ps,pe);

	curves = box_info->curves;
	box_info->num_curves = 0;
	for (j = box->bmax[1]; j >= box->bmin[1]; --j)
	{
	    jj = j - box->bmin[1];
	    for (i = box->bmin[0]; i < box->bmax[0]; ++i)
	    {
	    	ii = i - box->bmin[0];
		nb = (j == box->bmax[1]) ? T->num_of_bonds[j-1][i]: 
				T->num_of_bonds[j][i];
		bonds = (j == box->bmax[1]) ? T->bonds[j-1][i]: T->bonds[j][i];
		bc = (j == box->bmax[1]) ? T->curves[j-1][i]: 
					T->curves[j][i];
		num_x_crx[ii][jj] = 0;
		for (k = 0; k < nb; ++k)
		{
		    boolean in_list;
		    b = bonds[k];
		    for (c = intfc->curves; c && *c; ++c)
		    {
			/* Tables may have been changed in last
			 * box operation, this is the least expensive
			 * way to make sure curves are updated */
		    	if (is_b_on_curve(*c,b))
			{
			    bc[k] = *c;
			    break;
			}
		    }
		    in_list = NO;
		    for (l = 0; l < box_info->num_curves; ++l)
		    {
		    	if (bc[k] == curves[l]) 
			    in_list = YES;
		    }
		    if (!in_list)
			curves[box_info->num_curves++] = bc[k];
                    low = Coords(tmp_b->start)[0] = L[0] + i*h[0];
                    upp = Coords(tmp_b->end)[0] = L[0] + (i+1)*h[0];
                    seg = Coords(tmp_b->start)[1] = Coords(tmp_b->end)[1]
                                        = L[1] + j*h[1];
		    set_bond_length(tmp_b,2);
		    if (seg_cross_bond(0,b,low,upp,seg,p))
		    {
			num_x_crx[ii][jj]++;
			num_crx++;
		    }
		}
	    }
	}
	for (j = box->bmax[1]-1; j >= box->bmin[1]; --j)
	{
	    jj = j - box->bmin[1];
	    for (i = box->bmin[0]; i <= box->bmax[0]; ++i)
	    {
	    	ii = i - box->bmin[0];
		nb = (i == box->bmax[0]) ? T->num_of_bonds[j][i-1]: 
				T->num_of_bonds[j][i];
		bonds = (i == box->bmax[0]) ? T->bonds[j][i-1]: T->bonds[j][i];
		num_y_crx[ii][jj] = 0;
		for (k = 0; k < nb; ++k)
		{
		    b = bonds[k];
                    low = Coords(tmp_b->start)[1] = L[1] + j*h[1];
                    upp = Coords(tmp_b->end)[1] = L[1] + (j+1)*h[1];
                    seg = Coords(tmp_b->start)[0] = Coords(tmp_b->end)[0]
                                        = L[0] + i*h[0];
		    set_bond_length(tmp_b,2);
		    if (seg_cross_bond(1,b,low,upp,seg,p))
		    {
			num_y_crx[ii][jj]++;
			num_crx++;
		    }
		}
	    }
	}
	for (i = 0; i < box_info->num_curves; ++i)
	{
	    CURVE *c = curves[i];
	    boolean out_box_pt_found = NO;
	    box_info->is_entirely_inside[i] = YES;
	    if (is_closed_curve(c) && 
	 	!out_box_point(c->start->posn,box))
	    {
		for (b = c->first; b != NULL; b = b->next)
		{
		    if (out_box_point(b->start,box))
		    {
			move_closed_loop_node(c,b);
			out_box_pt_found = YES;
	    		box_info->is_entirely_inside[i] = NO;
			break;
		    }
		}
		if (!out_box_pt_found && debugging("lgb2d"))
		    (void) printf("closed curve entirely inside box!\n");
	    }
	    else
	    	box_info->is_entirely_inside[i] = NO;
	}
	box_info->num_crx = num_crx;
}	/* end count_box_crossings */

LOCAL	void install_box_crossings(
	INTERFACE *intfc,
	RECT_BOX *box,
	BOX_INFO *box_info)
{
	struct Table *T = table_of_interface(intfc);
	int i,j,ii,jj,k,l,nb,nc;
	BOND **bonds,*b,*tmp_b;
	CURVE **curves,*c;
	FT_POINT *ps,*pe,*p,*pt;
	double coords[3] = {0,0,0};
	int num_crx = 0;
	double *L = box->grid->L;
	double *h = box->grid->h;
	CRXING ***x_crx = box_info->x_crx;
	CRXING ***y_crx = box_info->y_crx;
	CRXING *crx_store = box_info->crx_store;
	int **num_x_crx = box_info->num_x_crx;
	int **num_y_crx = box_info->num_y_crx;
	int *gmax = box->smax;
	double   low,upp,seg;

	num_crx = 0;
	for (i = 0; i < gmax[0]; ++i)
	{
	    for (j = 0; j <= gmax[1]; ++j)
	    {
		x_crx[i][j] = crx_store + num_crx;
		num_crx += num_x_crx[i][j];
	    }
	}
	for (i = 0; i <= gmax[0]; ++i)
	{
	    for (j = 0; j < gmax[1]; ++j)
	    {
		y_crx[i][j] = crx_store + num_crx;
		num_crx += num_y_crx[i][j];
	    }
	}
	ps = Point(coords);
	pe = Point(coords);
	p  = Point(coords);
	tmp_b = Bond(ps,pe);

	for (j = box->bmin[1]; j <= box->bmax[1]; ++j)
	{
	    jj = j - box->bmin[1];
	    for (i = box->bmin[0]; i < box->bmax[0]; ++i)
	    {
	    	ii = i - box->bmin[0];
		nb = (j == box->bmax[1]) ? T->num_of_bonds[j-1][i]: 
				T->num_of_bonds[j][i];
		bonds = (j == box->bmax[1]) ? T->bonds[j-1][i]: 
					T->bonds[j][i];
		curves = (j == box->bmax[1]) ? T->curves[j-1][i]: 
					T->curves[j][i];
		nc = 0;
		for (k = 0; k < nb; ++k)
		{
		    b = bonds[k];
		    c = curves[k];
		    low = Coords(tmp_b->start)[0] = L[0] + i*h[0];
		    upp = Coords(tmp_b->end)[0] = L[0] + (i+1)*h[0];
		    seg = Coords(tmp_b->start)[1] = Coords(tmp_b->end)[1] 
			    		= L[1] + j*h[1];
		    set_bond_length(tmp_b,2);
		    if (seg_cross_bond(0,b,low,upp,seg,p))
		    {
			x_crx[ii][jj][nc].pt = Point(Coords(p));
			x_crx[ii][jj][nc].bond = b;
			x_crx[ii][jj][nc].hs = Hyper_surf(c);
			if (Coords(b->start)[1] < Coords(b->end)[1])
			{
			    x_crx[ii][jj][nc].lcomp =
				    negative_component(c);
			    x_crx[ii][jj][nc].ucomp =
				    positive_component(c);
			}
			else
			{
			    x_crx[ii][jj][nc].lcomp =
				    positive_component(c);
			    x_crx[ii][jj][nc].ucomp =
				    negative_component(c);
			}
			nc++;
			num_crx++;
		    }
		}
		for (k = 0; k < nc-1; ++k)
		{
		    for (l = k+1; l < nc; ++l)
		    {
			if (Coords(x_crx[ii][jj][k].pt)[0] >
			    Coords(x_crx[ii][jj][l].pt)[0])
			{
			    CRXING tmp_crx;
			    tmp_crx = x_crx[ii][jj][l];
			    x_crx[ii][jj][l] = x_crx[ii][jj][k];
			    x_crx[ii][jj][k] = tmp_crx;
			}
		    }
		}
	    }
	}
	for (j = box->bmin[1]; j < box->bmax[1]; ++j)
	{
	    jj = j - box->bmin[1];
	    for (i = box->bmin[0]; i <= box->bmax[0]; ++i)
	    {
	    	ii = i - box->bmin[0];
		nb = (i == box->bmax[0]) ? T->num_of_bonds[j][i-1]: 
				T->num_of_bonds[j][i];
		bonds = (i == box->bmax[0]) ? T->bonds[j][i-1]: 
					T->bonds[j][i];
		curves = (i == box->bmax[0]) ? T->curves[j][i-1]: 
					T->curves[j][i];
		nc = 0;
		for (k = 0; k < nb; ++k)
		{
		    b = bonds[k];
		    c = curves[k];
		    low = Coords(tmp_b->start)[1] = L[1] + j*h[1];
		    upp = Coords(tmp_b->end)[1] = L[1] + (j+1)*h[1];
		    seg = Coords(tmp_b->start)[0] = Coords(tmp_b->end)[0] 
			    		= L[0] + i*h[0];
		    set_bond_length(tmp_b,2);
		    if (seg_cross_bond(1,b,low,upp,seg,p))
		    {
			y_crx[ii][jj][nc].pt = Point(Coords(p));
			y_crx[ii][jj][nc].bond = b;
			y_crx[ii][jj][nc].hs = Hyper_surf(c);
			if (Coords(b->start)[0] < Coords(b->end)[0])
			{
			    y_crx[ii][jj][nc].lcomp =
				    positive_component(c);
			    y_crx[ii][jj][nc].ucomp =
				    negative_component(c);
			}
			else
			{
			    y_crx[ii][jj][nc].lcomp =
				    negative_component(c);
			    y_crx[ii][jj][nc].ucomp =
				    positive_component(c);
			}
			nc++;
			num_crx++;
		    }
		}
		for (k = 0; k < nc-1; ++k)
		{
		    for (l = k+1; l < nc; ++l)
		    {
			if (Coords(y_crx[ii][jj][k].pt)[1] >
			    Coords(y_crx[ii][jj][l].pt)[1])
			{
			    CRXING tmp_crx;
			    tmp_crx = y_crx[ii][jj][l];
			    y_crx[ii][jj][l] = y_crx[ii][jj][k];
			    y_crx[ii][jj][k] = tmp_crx;
			}
		    }
		}
	    }
	}
}	/* end install_box_crossings */

LOCAL	boolean set_box_comp_and_crx(
	INTERFACE *intfc,
	RECT_BOX *box,
	BOX_INFO *box_info)
{
	COMPONENT **comp = box_info->comp;
	COMPONENT cur_comp;
	int **num_x_crx = box_info->num_x_crx;
	int **num_y_crx = box_info->num_y_crx;
	CRXING ***x_crx = box_info->x_crx;
	CRXING ***y_crx = box_info->y_crx;
	int i,j,k,l,nc,ncx,ncy,*gmax = box->smax;
	int icoords[MAXD];

	if (debugging("lgb2d"))
	    (void) printf("Entering set_box_comp_and_crx()\n");

	/* Reset components */
	for (i = 0; i <= gmax[0]; ++i)
	    for (j = 0; j <= gmax[1]; ++j)
		comp[i][j] = NO_COMP;

	set_corner_components(intfc,box,box_info);

	if (debugging("lgb2d"))
	{
	    (void) printf("After setting corner:\n");
	    show_box_topology(box,num_x_crx,num_y_crx,comp);
	}

	set_side_components(intfc,box,box_info);

	if (debugging("lgb2d"))
	{
	    (void) printf("After setting boundary sides:\n");
	    show_box_topology(box,num_x_crx,num_y_crx,comp);
	}

	set_interior_components(intfc,box,box_info);

	if (debugging("lgb2d"))
	{
	    (void) printf("After setting interior and pack/shake:\n");
	    show_box_topology(box,num_x_crx,num_y_crx,comp);
	}

	remove_box_unphysical_crx(intfc,box,box_info);

	if (debugging("lgb2d"))
	{
	    (void) printf("After remove unphysical crossings:\n");
	    show_box_topology(box,num_x_crx,num_y_crx,comp);
	}

	/* Check consistency of grid components with crossings */
	for (i = 0; i <= gmax[0]; ++i)
	{
	    for (j = 0; j <= gmax[1]; ++j)
	    {
		if (i != gmax[0]) 	/* check east side */
		{
		    if ((num_x_crx[i][j] == 1 && 
			comp[i][j] != x_crx[i][j][0].lcomp) ||
			(num_x_crx[i][j] == 0 &&
		    	comp[i][j] != comp[i+1][j]))
		    {
			(void) printf("At grid point %d %d\n",i,j);
			(void) printf("component east side not consistent!\n");
			return FUNCTION_FAILED;
		    }
		}
		if (j != gmax[1]) 	/* check north side */
		{
		    if ((num_y_crx[i][j] == 1 && 
			comp[i][j] != y_crx[i][j][0].lcomp) ||
			(num_y_crx[i][j] == 0 &&
		    	comp[i][j] != comp[i][j+1]))
		    {
			(void) printf("At grid point %d %d\n",i,j);
			(void) printf("component north side not consistent!\n");
			return FUNCTION_FAILED;
		    }
		}
		if (i != 0) 		/* check west side */
		{
		    if ((num_x_crx[i-1][j] == 1 && 
			comp[i][j] != x_crx[i-1][j][0].ucomp) ||
			(num_x_crx[i-1][j] == 0 &&
		    	comp[i][j] != comp[i-1][j]))
		    {
			(void) printf("At grid point %d %d\n",i,j);
			(void) printf("component west side not consistent!\n");
			return FUNCTION_FAILED;
		    }
		}
		if (j != 0) 		/* check south side */
		{
		    if ((num_y_crx[i][j-1] == 1 && 
			comp[i][j] != y_crx[i][j-1][0].ucomp) ||
			(num_y_crx[i][j-1] == 0 &&
		    	comp[i][j] != comp[i][j-1]))
		    {
			(void) printf("At grid point %d %d\n",i,j);
			(void) printf("component south side not consistent!\n");
			return FUNCTION_FAILED;
		    }
		}
	    }
	}

	if (debugging("lgb2d"))
	    (void) printf("Leaving set_box_comp_and_crx()\n");
	return FUNCTION_SUCCEEDED;
}	/* end set_box_comp_and_crx */

LOCAL	boolean reconnect_box_bonds(
	INTERFACE *intfc,
	RECT_BOX *box,
	BOX_INFO *box_info)
{
	int num_in_bonds = box_info->num_in_bonds;
	int num_out_bonds = box_info->num_out_bonds;
	BOND **in_bonds = box_info->in_bonds;
	BOND **out_bonds = box_info->out_bonds;
	CURVE **in_curves = box_info->in_curves;
	CURVE **out_curves = box_info->out_curves;
	FT_POINT *ps,*pe;
	BOND *b;
	BOND *b1,*b2;
	CURVE *c1,*c2;
	int i,j;

	if (debugging("lgb2d"))
	    (void) printf("Entering reconnect_box_bonds()\n");

	if (debugging("lgb2d"))
	{
	    (void) printf("num_in_bonds = %d\n",num_in_bonds);
	    (void) printf("num_out_bonds = %d\n",num_out_bonds);
	    for (i = 0; i < num_in_bonds; ++i)
	    	(void) printf("in_curves[%d] = %p\n",i,
			      box_info->in_curves[i]);
	    for (i = 0; i < num_out_bonds; ++i)
	    	(void) printf("out_curves[%d] = %p\n",i,
			      box_info->out_curves[i]);
	    printf("Original curves:\n");
	    for (i = 0; i < box_info->num_curves; ++i)
	    {
		printf("box_info->curves[%d] = %p\n",i,box_info->curves[i]);
	    }
	}

	/* Prepare out_bonds for connection:
	 * (1). disconnect from old bonds inside the box;
	 * (2). arrange the out_bonds in clockwise order.
	 */
	for (i = 0; i < num_out_bonds; ++i)
	{
	    if (out_bonds[i]->next && 
		     !out_box_bond(out_bonds[i]->next,box))
		out_bonds[i]->next = NULL;
	    else if (out_bonds[i]->prev && 
		     !out_box_bond(out_bonds[i]->prev,box))
		out_bonds[i]->prev = NULL;
	}
	for (i = 0; i < num_out_bonds-1; ++i)
	{
	    for (j = i+1; j < num_out_bonds; ++j)
	    {
		if (!in_clockwise_order(out_bonds[i],out_curves[i],
				out_bonds[j],out_curves[j],box))
		{
		    CURVE *c_tmp = out_curves[i];
		    BOND *b_tmp = out_bonds[i];
		    out_curves[i] = out_curves[j];
		    out_curves[j] = c_tmp;
		    out_bonds[i] = out_bonds[j];
		    out_bonds[j] = b_tmp;
		}
	    }
	}
	if (debugging("lgb2d"))
	{
	    (void) printf("out_bonds before connection:\n");
	    for (j = 0; j < num_out_bonds; ++j)
	    {
	    	b1 = out_bonds[j];
	    	c1 = out_curves[j];
	    	if (b1 != c1->first && b1->prev == NULL) 
		{
		    (void) printf("out_bonds[%d]  prev  NULL  start  %p\n",
		    		j,b1->start);
		    (void) printf("%f  %f\n",Coords(b1->start)[0],
		    			Coords(b1->start)[1]);
		}
		else if (b1 != c1->last && b1->next == NULL)
		{
		    (void) printf("out_bonds[%d]  next  NULL  end    %p\n",
		    		j,b1->end);
		    (void) printf("%f  %f\n",Coords(b1->end)[0],
		    			Coords(b1->end)[1]);
		}
		else
		    (void) printf("out_bonds[%d]\n",j);
	    }
	    (void) printf("open in_bonds before connection:\n");
	    for (i = 0; i < num_in_bonds; ++i)
	    {
		if (in_bonds[i]->prev != NULL && in_bonds[i]->next != NULL)
		    continue;
	    	b1 = in_bonds[i];
	    	c1 = in_curves[i];
	    	if (b1 != c1->first && b1->prev == NULL) 
		    (void) printf("in_bonds[%d]  prev  NULL  start  %p\n",
		    		i,b1->start);
		else if (b1 != c1->last && b1->next == NULL)
		    (void) printf("in_bonds[%d]  next  NULL  end    %p\n",
		    		i,b1->end);
	    }
	}

	/* Connecting in_bonds and out_bonds at box boundary */
	for (i = 0; i < num_in_bonds; ++i)
	{
	    for (j = 0; j < num_out_bonds; ++j)
	    {
		if (in_bonds[i]->start == out_bonds[j]->end)
		{
		    if (in_bonds[i]->prev != NULL || out_bonds[j]->next != NULL)
		    {
			(void) printf("in_bond start and out_bond end match\n");
			(void) printf("but connecting bonds not null\n");
			return FUNCTION_FAILED;
		    }
		    in_bonds[i]->prev = out_bonds[j];
		    out_bonds[j]->next = in_bonds[i];
		}
		else if (in_bonds[i]->end == out_bonds[j]->start)
		{
		    if (in_bonds[i]->next != NULL || out_bonds[j]->prev != NULL)
		    {
			(void) printf("in_bond end and out_bond start match\n");
			(void) printf("but connecting bonds not null\n");
			return FUNCTION_FAILED;
		    }
		    in_bonds[i]->next = out_bonds[j];
		    out_bonds[j]->prev = in_bonds[i];
		}
	    }
	}

	/* Connect the left-over cut bonds from outside */
	for (i = 0; i < num_out_bonds; ++i)
	{
	    b1 = out_bonds[i];
	    c1 = out_curves[i];
	    if ((b1 != c1->first && b1->prev == NULL) ||
                (b1 != c1->last && b1->next == NULL))
		break;
	}
	if (debugging("lgb2d"))
	{
	    for (j = 0; j < num_out_bonds; ++j)
	    {
	    	b1 = out_bonds[j];
	    	c1 = out_curves[j];
	    	if (b1 != c1->first && b1->prev == NULL) 
		    (void) printf("out_bonds[%d]  prev  NULL\n",j);
		else if (b1 != c1->last && b1->next == NULL)
		    (void) printf("out_bonds[%d]  next  NULL\n",j);
		else
		    (void) printf("out_bonds[%d]\n",j);
	    }
	    (void) printf("start index = %d\n",i);
	}
	for (j = 0; j < num_out_bonds; ++j)
	{
	    b1 = out_bonds[(j+i)%num_out_bonds];
	    b2 = out_bonds[(j+i+1)%num_out_bonds];
	    c1 = out_curves[(j+i)%num_out_bonds];
	    c2 = out_curves[(j+i+1)%num_out_bonds];
	    if (b1 != c1->first && b1->prev == NULL &&
		b2 != c2->last && b2->next == NULL)
	    {
		ps = b2->end;
		pe = b1->start;
		b = Bond(ps,pe);
		b1->prev = b;
		b2->next = b;
		b->next = b1;
		b->prev = b2;
		++j;
	    }
	    if (b1 != c1->last && b1->next == NULL &&
		b2 != c2->first && b2->prev == NULL)
	    {
		ps = b1->end;
		pe = b2->start;
		b = Bond(ps,pe);
		b1->next = b;
		b2->prev = b;
		b->prev = b1;
		b->next = b2;
		++j;
	    }
	}
	/* Check unconnected in or out bonds */
	for (i = 0; i < num_in_bonds; ++i)
	{
	    if ((in_bonds[i] != in_curves[i]->first && 
		 in_bonds[i]->prev == NULL) ||
		(in_bonds[i] != in_curves[i]->last &&
		 in_bonds[i]->next == NULL))
	    {
		screen("Warning: unconnected in_bond found:\n");
		(void) print_bond(in_bonds[i]);
		return FUNCTION_FAILED;
	    }
	}
	for (j = 0; j < num_out_bonds; ++j)
	{
	    if ((out_bonds[j] != out_curves[j]->first &&
		out_bonds[j]->prev == NULL) ||
		(out_bonds[j] != out_curves[j]->last &&
		out_bonds[j]->next == NULL))
	    {
		screen("Warning: unconnected out_bond[%d] found:\n",j);
		(void) print_bond(out_bonds[j]);
		return FUNCTION_FAILED;
	    }
	}
	check_make_outside_loop_curve(intfc,box_info,box);
	if (!check_and_reorganize_curves(intfc,box_info))
	    return FUNCTION_FAILED;

	if (debugging("lgb2d"))
	    (void) printf("Leaving reconnect_box_bonds()\n");
	return FUNCTION_SUCCEEDED;
}	/* end reconnect_box_bonds */

LOCAL	boolean reconstruct_box_bonds(
	INTERFACE *intfc,
	RECT_BOX *box,
	BOX_INFO *box_info)
{
	FT_POINT *ps,*pe;
	int *gmax = box->smax;
	int **num_x_crx = box_info->num_x_crx;
	int **num_y_crx = box_info->num_y_crx;
	CRXING ***x_crx = box_info->x_crx;
	CRXING ***y_crx = box_info->y_crx;
	CRXING *crx_s,*crx_e;
	BOND **bonds = box_info->in_bonds;
	CURVE **curves = box_info->in_curves;
	COMPONENT **comp = box_info->comp;
	int i,j,nb;
	boolean ps_to_pe;
	CURVE *c;
	CELL_INFO cell;

	box_info->num_in_bonds = 0;
	for (i = 0; i < gmax[0]; ++i)
	{
	    for (j = 0; j < gmax[1]; ++j)
	    {
		cell.comp[0] = comp[i][j];
		cell.comp[1] = comp[i+1][j];
		cell.comp[2] = comp[i+1][j+1];
		cell.comp[3] = comp[i][j+1];
		cell.num_cell_crx = 0;
		if (num_x_crx[i][j] == 1)
		{
		    cell.crxs[0] = &x_crx[i][j][0];
		    cell.num_cell_crx++;
		}
		else
		    cell.crxs[0] = NULL;
		if (num_y_crx[i+1][j] == 1)
		{
		    cell.crxs[1] = &y_crx[i+1][j][0];
		    cell.num_cell_crx++;
		}
		else
		    cell.crxs[1] = NULL;
		if (num_x_crx[i][j+1] == 1)
		{
		    cell.crxs[2] = &x_crx[i][j+1][0];
		    cell.num_cell_crx++;
		}
		else
		    cell.crxs[2] = NULL;
		if (num_y_crx[i][j] == 1)
		{
		    cell.crxs[3] = &y_crx[i][j][0];
		    cell.num_cell_crx++;
		}
		else
		    cell.crxs[3] = NULL;
		if (!make_cell_bonds(&cell,box_info))
		    return FUNCTION_FAILED;
	    }
	}
	nb = box_info->num_in_bonds;
	for (i = 0; i < nb-1; ++i)
	{
	    for (j = i+1; j < nb; ++j)
	    {
		if (bonds[i]->start == bonds[j]->end)
		{
		    bonds[i]->prev = bonds[j];
		    bonds[j]->next = bonds[i];
		}
		else if (bonds[i]->end == bonds[j]->start)
		{
		    bonds[j]->prev = bonds[i];
		    bonds[i]->next = bonds[j];
		}
	    }
	}
	check_make_inside_loop_curve(intfc,box_info,box);
	return FUNCTION_SUCCEEDED;
}	/* end reconstruct_box_bonds */

/*	Identify closed loop inside the box and make a closed
 *	curve for the identified loop.
*/
LOCAL	void check_make_inside_loop_curve(
	INTERFACE *intfc,
	BOX_INFO *box_info,
	RECT_BOX *box)
{
	NODE *n;
	BOND *b,**in_bonds = box_info->in_bonds;
	CURVE *c,**in_curves = box_info->in_curves;
	int i,j,nb,num_points;
	size_t sizest = size_of_state(intfc);
	int num_old_curves = box_info->num_curves;

	if (debugging("lgb2d"))
	    (void) printf("Entering check_make_inside_loop_curve()\n");
	nb = box_info->num_in_bonds;
	for (i = 0; i < nb; ++i)
	{
	    if (new_closed_loop(in_bonds[i],&num_points,NULL))
	    {
		if (debugging("lgb2d"))
		    (void) printf("bonds in closed loop, num_points = %d\n",
				num_points);
		n = copy_node(in_curves[i]->start);
		c = copy_curve_without_geom(in_curves[i],n,n);
		n->posn = in_bonds[i]->start;
		set_not_bdry(n);
		c->first = in_bonds[i];
		c->last = in_bonds[i]->prev;
		c->first->prev = NULL;
		c->last->next = NULL;
		ft_assign(left_state(in_bonds[i]->start),
				left_start_state(c),sizest);
		ft_assign(right_state(in_bonds[i]->start),
				right_start_state(c),sizest);
		ft_assign(left_state(in_bonds[i]->start),
				left_end_state(c),sizest);
		ft_assign(right_state(in_bonds[i]->start),
				right_end_state(c),sizest);
		c->num_points = num_points;
		box_info->curves[box_info->num_curves++] = c;
		for (b = c->first; b != NULL; b = b->next)
		{
		    for (j = 0; j < nb; ++j)
		    {
			if (b == in_bonds[j])
			{
			    in_curves[j] = c;
			}
		    }
		}
		c->orientation = (area_of_closed_curve(c) > 0.0) ? 1 : -1;
	    }
	}
	/* if old curve is entirely inside the box, delete it */
	for (i = 0; i < num_old_curves; ++i)
	{
	    c = box_info->curves[i];
	    if (box_info->is_entirely_inside[i])
	    {
		n = c->start;
		delete_curve(c);
		delete_node(n);
		for (j = i; j < box_info->num_curves-1; ++j)
		{
		    box_info->is_entirely_inside[j] 
		    		= box_info->is_entirely_inside[j+1];
		    box_info->curves[j] = box_info->curves[j+1];
		}
		box_info->num_curves--;
		num_old_curves--;
	    }
	}
	if (debugging("lgb2d"))
	    (void) printf("Leaving check_make_inside_loop_curve()\n");
}	/* end check_make_inside_loop_curve */

/*	Identify closed loop inside the box and make a closed
 *	curve for the identified loop.
*/
LOCAL	void check_make_outside_loop_curve(
	INTERFACE *intfc,
	BOX_INFO *box_info,
	RECT_BOX *box)
{
	NODE *n;
	BOND *b,**out_bonds = box_info->out_bonds;
	CURVE *c,**out_curves = box_info->out_curves;
	int i,j,nb,num_points;
	size_t sizest = size_of_state(intfc);
	int num_old_curves = box_info->num_curves;

	if (debugging("lgb2d"))
	    (void) printf("Entering check_make_outside_loop_curve()\n");
	nb = box_info->num_out_bonds;
	for (i = 0; i < nb; ++i)
	{
	    if (new_closed_loop(out_bonds[i],&num_points,
			out_curves[i]->start))
	    {
		if (debugging("lgb2d"))
		    (void) printf("bonds in closed loop, num_points = %d\n",
				num_points);
		n = copy_node(out_curves[i]->start);
		c = copy_curve_without_geom(out_curves[i],n,n);
		n->posn = out_bonds[i]->start;
		set_not_bdry(n);
		node_type(n) = CLOSED_NODE;
		c->first = out_bonds[i];
		c->last = out_bonds[i]->prev;
		c->first->prev = NULL;
		c->last->next = NULL;
		ft_assign(left_state(out_bonds[i]->start),
				left_start_state(c),sizest);
		ft_assign(right_state(out_bonds[i]->start),
				right_start_state(c),sizest);
		ft_assign(left_state(out_bonds[i]->start),
				left_end_state(c),sizest);
		ft_assign(right_state(out_bonds[i]->start),
				right_end_state(c),sizest);
		c->num_points = num_points;
		box_info->curves[box_info->num_curves++] = c;
		for (b = c->first; b != NULL; b = b->next)
		{
		    for (j = 0; j < nb; ++j)
		    {
			if (b == out_bonds[j])
			{
			    out_curves[j] = c;
			}
		    }
		}
	    }
	}
	if (debugging("lgb2d"))
	    (void) printf("Leaving check_make_outside_loop_curve()\n");
}	/* end check_make_outside_loop_curve */

LOCAL	boolean check_and_reorganize_curves(
	INTERFACE *intfc,
	BOX_INFO *box_info)
{
	CURVE *c,**curves = box_info->curves;
	int i,j,num_curves = box_info->num_curves;
	BOND *b,**last_b;
	NODE **ne;

	if (debugging("lgb2d"))
	    (void) printf("Entering check_and_reorganize_curves()\n");
	uni_array(&last_b,num_curves,sizeof(BOND*));
	uni_array(&ne,num_curves,sizeof(NODE*));
	for (i = 0; i < num_curves; ++i)
	{
	    last_b[i] = curves[i]->last;
	    ne[i] = curves[i]->end;
	    if (debugging("lgb2d"))
		(void) printf("curves[%d] = %p\n",i,curves[i]);
	    c = curves[i];
	}

	for (i = 0; i < num_curves; ++i)
	{
	    c = curves[i];
	    for (b = c->first; b->next != NULL; b = b->next) ;
	    if (last_b[i] != b)
	    {
		if (debugging("lgb2d"))
		{
		    (void) printf("Changing node for curves[%d]\n",i);
		}
		for (j = 0; j < num_curves; ++j)
		    if (b->end == ne[j]->posn && b == last_b[j])
			break;
		c->last = b;
		if (j == num_curves)
		{
		    (void) printf("WARNING: cannot find matching last bond\n");
		    return FUNCTION_FAILED;
		}
		change_node_of_curve(c,NEGATIVE_ORIENTATION,ne[j]);
		if (debugging("lgb2d"))
		{
		    (void) printf("After changing node:\n");
	    	    (void) print_curve(c);
		}
	    }
	    c->num_points = 1;
	    for (b = c->first; b != NULL; b = b->next) c->num_points++;
	}
	free_these(2,last_b,ne);
	if (debugging("lgb2d"))
	    (void) printf("Leaving check_and_reorganize_curves()\n");
	return FUNCTION_SUCCEEDED;
}	/* end check_and_reorganize_curves */

LOCAL boolean new_closed_loop(
	BOND *b,
	int *num_points,
	NODE *n)
{
	BOND *b1;
	boolean node_in_loop = NO;
	*num_points = 1;
	for (b1 = b->next; b1 != NULL; b1 = b1->next)
	{
	    if (n && (n->posn == b->start || n->posn == b->end))
		node_in_loop = YES;
	    if (b1 == b && !node_in_loop) return YES;
	    ++(*num_points);
	}
	return NO;
}	/* end new_closed_loop */

LOCAL   boolean connected_boxes(
        RECT_BOX *box1,
        RECT_BOX *box2)
{
	int i,dim = box1->grid->dim;
	int nc,nw;	

	for (i = 0; i < dim; ++i)
	{
	    nc = box1->bmin[i] + box1->bmax[i] - 
	            box2->bmin[i] - box2->bmax[i];
	    nw = box1->bmax[i] + box2->bmax[i] - 
	            box1->bmin[i] - box2->bmin[i];
	    if (abs(nc) > nw) return NO;
	}
        return YES;
}       /* end connected_boxes */

LOCAL	boolean bond_in_box(
	BOND *b,
	INTERFACE *intfc,
	RECT_BOX *box)
{
	boolean status = NO;
	double XB[2],YB[2];

	XB[0] = box->grid->L[0] + (double)box->bmin[0]*box->grid->h[0];
	XB[1] = box->grid->L[0] + (double)box->bmax[0]*box->grid->h[0];
	YB[0] = box->grid->L[1] + (double)box->bmin[1]*box->grid->h[1];
	YB[1] = box->grid->L[1] + (double)box->bmax[1]*box->grid->h[1];

	if (b == NULL) return NO;

	if (Coords(b->start)[0] >= XB[0] &&
	    Coords(b->start)[0] <= XB[1] &&
	    Coords(b->start)[1] >= YB[0] &&
	    Coords(b->start)[1] <= YB[1])
	    status = YES;
	if (Coords(b->end)[0] >= XB[0] &&
	    Coords(b->end)[0] <= XB[1] &&
	    Coords(b->end)[1] >= YB[0] &&
	    Coords(b->end)[1] <= YB[1])
	    status = YES;

	return status;
}	/* end bond_in_box */

LOCAL	boolean bond_enclose_point(
	BOND *b,
	FT_POINT *p)
{
	int i;
	double ratio;

	if (fabs(Coords(b->start)[0] - Coords(b->end)[0]) >
	    fabs(Coords(b->start)[1] - Coords(b->end)[1]))
	    i = 0;
	else
	    i = 1;
	if (fabs(Coords(b->start)[i] - Coords(b->end)[i]) == 0.0) 
	    return NO;
	ratio = (Coords(p)[i] - Coords(b->start)[i])/
		(Coords(b->end)[i] - Coords(b->start)[i]);
	if (0.0 < ratio && ratio < 1.0)
	    return YES;
	else
	    return NO;
}	/* end bond_enclose_point */

LOCAL	boolean out_box_bond(
	BOND *b,
	RECT_BOX *box)
{
	int i;
	boolean cross_bdry = NO;
	double t,crx_crds,XB[2],YB[2];

	XB[0] = box->grid->L[0] + (double)box->bmin[0]*box->grid->h[0];
	XB[1] = box->grid->L[0] + (double)box->bmax[0]*box->grid->h[0];
	YB[0] = box->grid->L[1] + (double)box->bmin[1]*box->grid->h[1];
	YB[1] = box->grid->L[1] + (double)box->bmax[1]*box->grid->h[1];
	for (i = 0; i < 2; ++i)
	{
	    if (Coords(b->start)[0] == XB[i] || Coords(b->end)[0] == XB[i])
		continue;
	    t = (XB[i] - Coords(b->start)[0])/(Coords(b->end)[0] - 
			    Coords(b->start)[0]);
	    if (t < 0.0 || t > 1.0) continue;
	    crx_crds = (Coords(b->end)[1] - Coords(b->start)[1])*t +
		    		Coords(b->start)[1];
	    if (crx_crds > YB[0] && crx_crds < YB[1])
		cross_bdry = YES;
	}
	for (i = 0; i < 2; ++i)
	{
	    if (Coords(b->start)[1] == YB[i] || Coords(b->end)[1] == YB[i])
		continue;
	    t = (YB[i] - Coords(b->start)[1])/(Coords(b->end)[1] - 
			    Coords(b->start)[1]);
	    if (t < 0.0 || t > 1.0) continue;
	    crx_crds = (Coords(b->end)[0] - Coords(b->start)[0])*t +
		    		Coords(b->start)[0];
	    if (crx_crds > XB[0] && crx_crds < XB[1])
		cross_bdry = YES;
	}
	if (cross_bdry) 
	    return NO;
	if (out_box_point(b->start,box) || out_box_point(b->end,box))
	    return YES;
	return NO;
}	/* end out_box_bond */

LOCAL	boolean out_box_point(
	FT_POINT *p,
	RECT_BOX *box)
{
	double XB[2],YB[2];

	XB[0] = box->grid->L[0] + (double)box->bmin[0]*box->grid->h[0];
	XB[1] = box->grid->L[0] + (double)box->bmax[0]*box->grid->h[0];
	YB[0] = box->grid->L[1] + (double)box->bmin[1]*box->grid->h[1];
	YB[1] = box->grid->L[1] + (double)box->bmax[1]*box->grid->h[1];

	if (Coords(p)[0] < XB[0])
	    return YES;
	else if (Coords(p)[0] > XB[1])
	    return YES;
	else if (Coords(p)[1] < YB[0])
	    return YES;
	else if (Coords(p)[1] > YB[1])
	    return YES;
	return NO;
}	/* end out_box_bond */

LOCAL	boolean point_on_box_bdry(
	FT_POINT *p,
	RECT_BOX *box)
{
	double XB[2],YB[2];

	XB[0] = box->grid->L[0] + (double)box->bmin[0]*box->grid->h[0];
	XB[1] = box->grid->L[0] + (double)box->bmax[0]*box->grid->h[0];
	YB[0] = box->grid->L[1] + (double)box->bmin[1]*box->grid->h[1];
	YB[1] = box->grid->L[1] + (double)box->bmax[1]*box->grid->h[1];

	if (Coords(p)[0] == XB[0])
	    return YES;
	else if (Coords(p)[0] == XB[1])
	    return YES;
	else if (Coords(p)[1] == YB[0])
	    return YES;
	else if (Coords(p)[1] == YB[1])
	    return YES;
	return NO;
}	/* end point_on_box_bdry */

LOCAL	boolean in_clockwise_order(
	BOND *b1,
	CURVE *c1,
	BOND *b2,
	CURVE *c2,
	RECT_BOX *box)
{
	FT_POINT *p1,*p2;
	double XB[2],YB[2];

	XB[0] = box->grid->L[0] + (double)box->bmin[0]*box->grid->h[0];
	XB[1] = box->grid->L[0] + (double)box->bmax[0]*box->grid->h[0];
	YB[0] = box->grid->L[1] + (double)box->bmin[1]*box->grid->h[1];
	YB[1] = box->grid->L[1] + (double)box->bmax[1]*box->grid->h[1];

	if (b1 != c1->first && b1->prev == NULL) p1 = b1->start;
	else if (b1 != c1->last && b1->next == NULL) p1 = b1->end;
	else p1 == NULL;
	if (b2 != c2->first && b2->prev == NULL) p2 = b2->start;
	else if (b2 != c2->last && b2->next == NULL) p2 = b2->end;
	else p2 == NULL;
	if (p1 == NULL || p2 == NULL)
	{
	    screen("ERROR in in_clockwise_order(), wrong bond\n");
	    clean_up(ERROR);
	}

	if (Coords(p1)[1] == YB[0])
	{
	    if (Coords(p2)[0] == XB[0] ||
	        Coords(p2)[1] == YB[1] ||
	        Coords(p2)[0] == XB[1])
		return YES;
	    else if (Coords(p2)[0] < Coords(p1)[0])
		return YES;
	}
	else if (Coords(p1)[0] == XB[0])
	{
	    if (Coords(p2)[1] == YB[1] ||
		Coords(p2)[0] == XB[1])
		return YES;
	    else if (Coords(p2)[0] == XB[0] && Coords(p2)[1] > Coords(p1)[1])
		return YES;
	}
	else if (Coords(p1)[1] == YB[1])
	{
	    if (Coords(p2)[0] == XB[1])
		return YES;
	    else if (Coords(p2)[1] == YB[1] && Coords(p2)[0] > Coords(p1)[0])
		return YES;
	}
	else if (Coords(p2)[0] == XB[1] && Coords(p2)[1] < Coords(p1)[1])
	    return YES;
	return NO;
}	/* end in_clockwise_order */

LOCAL	void xgraph_tangled_box(
	const char *s,
	INTERFACE *intfc,
	RECT_BOX *box,
	int count)
{
	CURVE **c;
	BOND *b;
	RECT_GRID *gr = box->grid;
	int *bmin = box->bmin;
	int *bmax = box->bmax;
	double *L = gr->L;
	double *h = gr->h;
	char open_name[100];
	FILE *xfile;
	int i;

	if (count >= 20) return;	/* too many boxes */

	sprintf(open_name,"box-case-%d-%s.%d",count,s,pp_mynode());
	(void) printf("open_name = %s\n",open_name);
	xfile = fopen(open_name,"w");

	for (c = intfc->curves; c && *c; ++c)
	{
	    int nb = 0;
	    for (b = (*c)->first; b != NULL; b = b->next)
	    {
		if (nb > (*c)->num_points) break;
		if (bond_in_box(b,intfc,box))
		{
		    nb++;
		    fprintf(xfile,"%f %f\n",Coords(b->start)[0],
				    Coords(b->start)[1]);
		    if (!bond_in_box(b->next,intfc,box))
		    {
		        fprintf(xfile,"%f %f\n",Coords(b->end)[0],
				        Coords(b->end)[1]);
		        fprintf(xfile,"\n");
		    }
		}
	    }
	}

	/* draw the mesh of the box */
	for (i = bmin[0]; i <= bmax[0]; ++i)
	{
	    fprintf(xfile,"%f %f\n",L[0]+i*h[0],L[1]+bmin[1]*h[1]);
	    fprintf(xfile,"%f %f\n",L[0]+i*h[0],L[1]+bmax[1]*h[1]);
	    fprintf(xfile,"\n");
	}
	for (i = bmin[1]; i <= bmax[1]; ++i)
	{
	    fprintf(xfile,"%f %f\n",L[0]+bmin[0]*h[0],L[1]+i*h[1]);
	    fprintf(xfile,"%f %f\n",L[0]+bmax[0]*h[0],L[1]+i*h[1]);
	    fprintf(xfile,"\n");
	}
	fclose(xfile);
}	/* end xgraph_tangled_box */

LOCAL	void xgraph_box_bonds(
	const char *s,
	BOX_INFO *box_info,
	RECT_BOX *box,
	int count)
{
	BOND *b;
	RECT_GRID *gr = box->grid;
	int *bmin = box->bmin;
	int *bmax = box->bmax;
	double *L = gr->L;
	double *h = gr->h;
	char open_name[100];
	FILE *xfile;
	int i;
	BOND **in_bonds = box_info->in_bonds;
	BOND **out_bonds = box_info->out_bonds;

	if (count >= 20) return;	/* too many boxes */

	sprintf(open_name,"box-case-%d-%s.%d",count,s,pp_mynode());
	(void) printf("open_name = %s\n",open_name);
	xfile = fopen(open_name,"w");

	/* draw the mesh of the box */
	for (i = bmin[0]; i <= bmax[0]; ++i)
	{
	    fprintf(xfile,"%f %f\n",L[0]+i*h[0],L[1]+bmin[1]*h[1]);
	    fprintf(xfile,"%f %f\n",L[0]+i*h[0],L[1]+bmax[1]*h[1]);
	    fprintf(xfile,"\n");
	}
	for (i = bmin[1]; i <= bmax[1]; ++i)
	{
	    fprintf(xfile,"%f %f\n",L[0]+bmin[0]*h[0],L[1]+i*h[1]);
	    fprintf(xfile,"%f %f\n",L[0]+bmax[0]*h[0],L[1]+i*h[1]);
	    fprintf(xfile,"\n");
	}
	for (i = 0; i < box_info->num_in_bonds; ++i)
	{
	    if (in_bonds[i]->prev == NULL)
	    {
		b = in_bonds[i];
	    	fprintf(xfile,"%f %f\n",Coords(b->start)[0],
				Coords(b->start)[1]);
		for (; b != NULL; b = b->next)
	    	    fprintf(xfile,"%f %f\n",Coords(b->end)[0],
				Coords(b->end)[1]);
	    }
	    fprintf(xfile,"\n");
	}
	for (i = 0; i < box_info->num_out_bonds; ++i)
	{
	    b = out_bonds[i];
	    fprintf(xfile,"%f %f\n",Coords(b->start)[0],Coords(b->start)[1]);
	    fprintf(xfile,"%f %f\n",Coords(b->end)[0],Coords(b->end)[1]);
	    fprintf(xfile,"\n");
	}
	fclose(xfile);
}	/* end xgraph_box_bonds */

LOCAL	void show_box_topology(
	RECT_BOX *box,
	int **num_x_crx,
	int **num_y_crx,
	COMPONENT **comp)
{
	int *gmax = box->smax;
	int i,j,k;

	for (j = gmax[1]; j >= 0; --j)
	{
	    for (i = 0; i <= gmax[0]; ++i)
	    {
		if (comp[i][j] == NO_COMP)
		    printf("N");
		else
		    printf("%d",comp[i][j]);
		if (i != gmax[0])
		    printf("--%d--",num_x_crx[i][j]);
		else
		    printf("\n");
	    }
	    if (j != 0)
	    {
	    	for (i = 0; i <= gmax[0]; ++i)
		{
		    printf("|");
		    if (i != gmax[0])
			printf("     ");
		    else
		    	printf("\n");
		}
	    	for (i = 0; i <= gmax[0]; ++i)
		{
		    printf("%d",num_y_crx[i][j-1]);
		    if (i != gmax[0])
			printf("     ");
		    else
		    	printf("\n");
		}
	    	for (i = 0; i <= gmax[0]; ++i)
		{
		    printf("|");
		    if (i != gmax[0])
			printf("     ");
		    else
		    	printf("\n");
		}
	    }
	}
	printf("\n");
}	/* end show_box_topology */

LOCAL	CURVE *copy_curve_without_geom(
	CURVE *c,
	NODE *ns,
	NODE *ne)
{
	BOND *first,*last;
	BOND *btmp;
	CURVE *newc;
	first = c->first;
	last = c->last;
	btmp = Bond(first->start,last->end);
	c->first = c->last = btmp;
	newc = copy_curve(c,ns,ne);
	c->first = first;
	c->last = last;
	return newc;
}	/* copy_curve_without_geom */

LOCAL	void reclt_after_grid_based_untangle(
	CROSS *cross,
	BOX_INFO *box_info)
{
	CURVE **curves = box_info->curves;
	int i,nc = box_info->num_curves;
	BOND *b;
	CROSS *cr;
	for (i = 0; i < nc; ++i)
	{
	    for (b = curves[i]->first; b != NULL; b = b->next)
   	    {
		for (cr = cross; cr; cr = cr->next)
		{
		    if (cr->b1 == b) cr->c1 = curves[i];
		    if (cr->b2 == b) cr->c2 = curves[i];
		}
	    }
	}
}	/* end reclt_after_grid_based_untangle */


LOCAL	boolean	seg_cross_bond(
	int	dir,
	BOND	*b,
	double	l,
	double	u,
	double	seg,
	FT_POINT	*p)
{
	double	x1=Coords(b->start)[0];
	double	y1=Coords(b->start)[1];
	double	x2=Coords(b->end)[0];
	double	y2=Coords(b->end)[1];

	if (dir == 0)			/*x crossing */
	{
	    if (seg < min(y1,y2)) return NO;
	    if (seg > max(y1,y2)) return NO;
	    Coords(p)[1] = seg;
	    if (y1 == y2)		/*parallel */
	    {
	        if (y1 != seg) return NO;
		if (max(x1,x2) < l) return NO;
		if (min(x1,x2) > u) return NO;	/*no overlap */
		if (bond_length(b) > u-l)
		    Coords(p)[0] = Between(l,x1,x2) ? l : u;
		else
		    Coords(p)[0] = Between(x1,l,u) ? x1 : x2;
	        return YES;
	    }
	    Coords(p)[0] = x1 + (seg - y1)*(x2 - x1)/(y2 - y1);
	    if (Coords(p)[0] < l) return NO;
	    if (Coords(p)[0] > u) return NO;
	    return YES;
	}
	else				/*y crossing */
	{
	    if (seg < min(x1,x2)) return NO;
	    if (seg > max(x1,x2)) return NO;
	    Coords(p)[0] = seg;
	    if (x1 == x2)		/*parallel */
	    {
	        if (x1 != seg) return NO;
		if (max(y1,y2) < l) return NO;
		if (min(y1,y2) > u) return NO;	/*no overlap */
		if (bond_length(b) > u-l)
		    Coords(p)[1] = Between(l,y1,y2)?l:u;
		else
		    Coords(p)[1] = Between(y1,l,u)?y1:y2;
		return YES;
	    }
	    Coords(p)[1] = y1 + (seg - x1)*(y2 - y1)/(x2 - x1);
	    if (Coords(p)[1] < l) return NO;
	    if (Coords(p)[1] > u) return NO;
	    return YES;
	}
}	/* end seg_cross_bond */


LOCAL void set_corner_components(
	INTERFACE *intfc,
	RECT_BOX *box,
	BOX_INFO *box_info)
{
	COMPONENT **comp = box_info->comp;
	int **num_x_crx = box_info->num_x_crx;
	int **num_y_crx = box_info->num_y_crx;
	CRXING ***x_crx = box_info->x_crx;
	CRXING ***y_crx = box_info->y_crx;
	int i,j,nc,*gmax = box->smax;
	int icoords[MAXD];
	int nbc[2][2];

	/* Set corner components */
	nbc[0][0] = nbc[0][1] = 0;
	for (i = 0; i < gmax[0]; ++i)
	{
	    if ((nc = num_x_crx[i][0]) != 0)
	    {
	    	if (comp[0][0] == NO_COMP) comp[0][0] = x_crx[i][0][0].lcomp;
	    	comp[gmax[0]][0] = x_crx[i][0][nc-1].ucomp;
		nbc[0][0] += nc;
	    }
	    if ((nc = num_x_crx[i][gmax[1]]) != 0)
	    {
	    	if (comp[0][gmax[1]] == NO_COMP) 
			comp[0][gmax[1]] = x_crx[i][gmax[1]][0].lcomp;
	    	comp[gmax[0]][gmax[1]] = x_crx[i][gmax[1]][nc-1].ucomp;
		nbc[0][1] += nc;
	    }
	}
	nbc[1][0] = nbc[1][1] = 0;
	for (j = 0; j < gmax[1]; ++j)
	{
	    if ((nc = num_y_crx[0][j]) != 0)
	    {
	    	if (comp[0][0] == NO_COMP) comp[0][0] = y_crx[0][j][0].lcomp;
	    	comp[0][gmax[1]] = y_crx[0][j][nc-1].ucomp;
		nbc[1][0] += nc;
	    }
	    if ((nc = num_y_crx[gmax[0]][j]) != 0)
	    {
	    	if (comp[gmax[0]][0] == NO_COMP) 
			comp[gmax[0]][0] = y_crx[gmax[0]][j][0].lcomp;
	    	comp[gmax[0]][gmax[1]] = y_crx[gmax[0]][j][nc-1].ucomp;
		nbc[1][1] += nc;
	    }
	}
	/* in case some corner are not set */
	if (comp[0][0] == NO_COMP)
	{
	    if (nbc[0][0] == 0 && comp[gmax[0]][0] != NO_COMP) 
		comp[0][0] = comp[gmax[0]][0];
	    if (nbc[1][0] == 0 && comp[0][gmax[1]] != NO_COMP) 
		comp[0][0] = comp[0][gmax[1]];
	}
	if (comp[gmax[0]][0] == NO_COMP)
	{
	    if (nbc[0][0] == 0 && comp[0][0] != NO_COMP) 
		comp[gmax[0]][0] = comp[0][0];
	    if (nbc[1][1] == 0 && comp[gmax[0]][gmax[1]] != NO_COMP) 
		comp[gmax[0]][0] = comp[gmax[0]][gmax[1]];
	}
	if (comp[0][gmax[1]] == NO_COMP)
	{
	    if (nbc[1][0] == 0 && comp[0][0] != NO_COMP) 
		comp[0][gmax[1]] = comp[0][0];
	    if (nbc[0][1] == 0 && comp[gmax[0]][gmax[1]] != NO_COMP) 
		comp[0][gmax[1]] = comp[gmax[0]][gmax[1]];
	}
	if (comp[gmax[0]][gmax[1]] == NO_COMP)
	{
	    if (nbc[0][1] == 0 && comp[0][gmax[1]] != NO_COMP) 
		comp[gmax[0]][gmax[1]] = comp[0][gmax[1]];
	    if (nbc[1][1] == 0 && comp[gmax[0]][0] != NO_COMP) 
		comp[gmax[0]][gmax[1]] = comp[gmax[0]][0];
	}
	if (nbc[0][0] == 0 && nbc[0][1] == 0 && 
	    nbc[1][0] == 0 && nbc[1][1] == 0)
	{
	    /* Corner still unset */
	    double coords[MAXD];
	    COMPONENT cc[4];
	    COMPONENT c1,c2;
	    int nc1,nc2;
	    RECT_GRID *grid = box->grid;
	    double *L = grid->L;
	    double *h = grid->h;
	    coords[0] = L[0] + box->bmin[0]*h[0];
	    coords[1] = L[1] + box->bmin[1]*h[1];
	    cc[0] = component(coords,intfc);
	    coords[0] = L[0] + box->bmax[0]*h[0];
	    coords[1] = L[1] + box->bmin[1]*h[1];
	    cc[1] = component(coords,intfc);
	    coords[0] = L[0] + box->bmin[0]*h[0];
	    coords[1] = L[1] + box->bmax[1]*h[1];
	    cc[2] = component(coords,intfc);
	    coords[0] = L[0] + box->bmax[0]*h[0];
	    coords[1] = L[1] + box->bmax[1]*h[1];
	    cc[3] = component(coords,intfc);
	    nc1 = 1;	nc2 = 0;
	    c1 = cc[0];
	    for (i = 1; i < 4; ++i)
	    {
	    	if (cc[i] != c1) 
		{
		    c2 = cc[i];
		    nc2++;
		}
		else
		    nc1++;
	    }
	    if (nc1 > nc2)
	    {
	    	comp[0][0] = comp[gmax[0]][0] = comp[0][gmax[1]]
			   = comp[gmax[0]][gmax[1]] = c1;
	    }
	    else
	    {
	    	comp[0][0] = comp[gmax[0]][0] = comp[0][gmax[1]]
			   = comp[gmax[0]][gmax[1]] = c2;
	    }
	}
}	/* end set_corner_components */

LOCAL void set_side_components(
	INTERFACE *intfc,
	RECT_BOX *box,
	BOX_INFO *box_info)
{
	COMPONENT **comp = box_info->comp;
	int **num_x_crx = box_info->num_x_crx;
	int **num_y_crx = box_info->num_y_crx;
	CRXING ***x_crx = box_info->x_crx;
	CRXING ***y_crx = box_info->y_crx;
	int i,j,nc,*gmax = box->smax;
	int icoords[MAXD];

	/* Set periphery side components */
	for (i = 0; i < gmax[0]-1; ++i)
	{
	    if ((nc = num_x_crx[i][0]) != 0)
	    {
	    	comp[i+1][0] = x_crx[i][0][nc-1].ucomp;
	    }
	    else 
		comp[i+1][0] = comp[i][0];
	    if ((nc = num_x_crx[i][gmax[1]]) != 0)
	    {
	    	comp[i+1][gmax[1]] = x_crx[i][gmax[1]][nc-1].ucomp;
	    }
	    else 
		comp[i+1][gmax[1]] = comp[i][gmax[1]];
	}
	for (j = 0; j < gmax[1]-1; ++j)
	{
	    if ((nc = num_y_crx[0][j]) != 0)
	    {
	    	comp[0][j+1] = y_crx[0][j][nc-1].ucomp;
	    }
	    else 
		comp[0][j+1] = comp[0][j];
	    if ((nc = num_y_crx[gmax[0]][j]) != 0)
	    {
	    	comp[gmax[0]][j+1] = y_crx[gmax[0]][j][nc-1].ucomp;
	    }
	    else 
		comp[gmax[0]][j+1] = comp[gmax[0]][j];
	}
}	/* end set_side_components */

LOCAL void set_interior_components(
	INTERFACE *intfc,
	RECT_BOX *box,
	BOX_INFO *box_info)
{
	COMPONENT **comp = box_info->comp;
	int **num_x_crx = box_info->num_x_crx;
	int **num_y_crx = box_info->num_y_crx;
	CRXING ***x_crx = box_info->x_crx;
	CRXING ***y_crx = box_info->y_crx;
	int i,j,k,ncx,ncy,nc,*gmax = box->smax;
	int icoords[MAXD];

	/* Set interior components, first trial */
	for (i = 1; i < gmax[0]; ++i)
	{
	    for (j = 1; j < gmax[1]; ++j)
	    {
		if ((ncx = num_x_crx[i-1][j]) == 0 && 
		     	comp[i-1][j] != NO_COMP)
		    comp[i][j] = comp[i-1][j];
		else if ((ncy = num_y_crx[i][j-1]) == 0 &&
			comp[i][j-1] != NO_COMP)
		    comp[i][j] = comp[i][j-1];
		else if (x_crx[i-1][j][ncx-1].ucomp ==
			y_crx[i][j-1][ncy-1].ucomp)
		{
		    comp[i][j] = x_crx[i-1][j][ncx-1].ucomp;
		}
	    }
	}
	for (i = gmax[0]-1; i > 0; --i)
	{
	    for (j = gmax[1]-1; j > 0; --j)
	    {
		if ((ncx = num_x_crx[i][j]) == 0 && 
		     	comp[i+1][j] != NO_COMP)
		    comp[i][j] = comp[i+1][j];
		else if ((ncy = num_y_crx[i][j]) == 0 &&
			comp[i][j+1] != NO_COMP)
		    comp[i][j] = comp[i][j+1];
		else if (x_crx[i][j][0].lcomp ==
			y_crx[i][j][0].lcomp)
		{
		    comp[i][j] = x_crx[i][j][0].lcomp;
		}
	    }
	}
	/* Pack and shake */
	for (k = 0; k < 3; ++k)
	{
	    for (i = 1; i < gmax[0]; ++i)
	    {
	    	for (j = 1; j < gmax[1]; ++j)
	    	{
		    int num_inconsistency = 0;
		    COMPONENT incon_comp,connected_comp;
		    connected_comp = NO_COMP;
		    nc = num_x_crx[i-1][j];
		    if (nc == 0) connected_comp = comp[i-1][j];
		    if ((nc != 0 && comp[i][j] != x_crx[i-1][j][nc-1].ucomp) ||
			(nc == 0 && comp[i][j] != comp[i-1][j]))
		    {
			num_inconsistency++;
			incon_comp = (nc == 0) ? comp[i-1][j] :
				x_crx[i-1][j][nc-1].ucomp;
		    }
		    nc = num_x_crx[i][j];
		    if (nc == 0) connected_comp = comp[i+1][j];
		    if ((nc != 0 && comp[i][j] != x_crx[i][j][0].lcomp) ||
			(nc == 0 && comp[i][j] != comp[i+1][j]))
		    {
			num_inconsistency++;
			incon_comp = (nc == 0) ? comp[i+1][j] :
				x_crx[i-1][j][0].lcomp;
		    }
		    nc = num_y_crx[i][j-1];
		    if (nc == 0) connected_comp = comp[i][j-1];
		    if ((nc != 0 && comp[i][j] != y_crx[i][j-1][nc-1].ucomp) ||
			(nc == 0 && comp[i][j] != comp[i][j-1]))
		    {
			num_inconsistency++;
			incon_comp = (nc == 0) ? comp[i][j-1] :
				y_crx[i][j-1][nc-1].ucomp;
		    }
		    nc = num_y_crx[i][j];
		    if (nc == 0) connected_comp = comp[i][j+1];
		    if ((nc != 0 && comp[i][j] != y_crx[i][j][0].lcomp) ||
			(nc == 0 && comp[i][j] != comp[i][j+1]))
		    {
			num_inconsistency++;
			incon_comp = (nc == 0) ? comp[i][j+1] :
				y_crx[i][j][0].lcomp;
		    }
		    if (num_inconsistency >= 3)
			comp[i][j] = incon_comp;
		    else if (num_inconsistency == 2 && 
			     connected_comp != NO_COMP)
			comp[i][j] = connected_comp;
		}
	    }
	}
}	/* end set_interior_components */

LOCAL void remove_box_unphysical_crx(
	INTERFACE *intfc,
	RECT_BOX *box,
	BOX_INFO *box_info)
{
	COMPONENT **comp = box_info->comp;
	COMPONENT cur_comp;
	int **num_x_crx = box_info->num_x_crx;
	int **num_y_crx = box_info->num_y_crx;
	CRXING ***x_crx = box_info->x_crx;
	CRXING ***y_crx = box_info->y_crx;
	int i,j,k,l,ncx,ncy,nc,*gmax = box->smax;
	int icoords[MAXD];

	/* Remove unphysical crossings step 1*/
	for (k = 0; k < 3; ++k)
	{
	    for (i = 1; i < gmax[0]; ++i)
	    {
	    	for (j = 1; j < gmax[1]; ++j)
	    	{
		    nc = num_x_crx[i-1][j];
		    if (nc != 0 && comp[i][j] != x_crx[i-1][j][nc-1].ucomp) 
		    {
			num_x_crx[i-1][j]--;
		    }
		    nc = num_x_crx[i][j];
		    if (nc != 0 && comp[i][j] != x_crx[i][j][0].lcomp) 
		    {
			for (l = 0; l < nc-1; ++l)
			{
			    x_crx[i][j][l] = x_crx[i][j][l+1];
			}
			num_x_crx[i][j]--;
		    }
		    nc = num_y_crx[i][j-1];
		    if (nc != 0 && comp[i][j] != y_crx[i][j-1][nc-1].ucomp) 
		    {
			num_y_crx[i][j-1]--;
		    }
		    nc = num_y_crx[i][j];
		    if (nc != 0 && comp[i][j] != y_crx[i][j][0].lcomp) 
		    {
			for (l = 0; l < nc-1; ++l)
			{
			    y_crx[i][j][l] = y_crx[i][j][l+1];
			}
			num_y_crx[i][j]--;
		    }
		}
	    }
	}
	if (debugging("lgb2d"))
	{
	    (void) printf("After remove unphysical crossings step 1:\n");
	    show_box_topology(box,num_x_crx,num_y_crx,comp);
	}
	/* Remove unphysical crossings step 2*/
	for (i = 0; i < gmax[0]; ++i)
	{
	    for (j = 1; j < gmax[1]; ++j)
	    {
	    	if ((nc = num_x_crx[i][j]) != 0)
		{
		    cur_comp = comp[i][j];
		    for (k = 0; k < nc; ++k)
		    {
		    	if (x_crx[i][j][k].lcomp != cur_comp)
			{
			    for (l = k; l < nc-1; ++l)
			    {
			    	x_crx[i][j][l] = x_crx[i][j][l+1];
			    }
			    --nc;
			    --num_x_crx[i][j];
			}
			cur_comp = x_crx[i][j][k].ucomp;
		    }
		    if (nc && cur_comp != comp[i+1][j])
		    {
			--nc;
		    	--num_x_crx[i][j];
		    }
		}
	    }
	}
	for (i = 1; i < gmax[0]; ++i)
	{
	    for (j = 0; j < gmax[1]; ++j)
	    {
	    	if ((nc = num_y_crx[i][j]) != 0)
		{
		    cur_comp = comp[i][j];
		    for (k = 0; k < nc; ++k)
		    {
		    	if (y_crx[i][j][k].lcomp != cur_comp)
			{
			    for (l = k; l < nc-1; ++l)
			    {
			    	y_crx[i][j][l] = y_crx[i][j][l+1];
			    }
			    --nc;
			    --num_y_crx[i][j];
			}
			cur_comp = y_crx[i][j][k].ucomp;
		    }
		    if (nc && cur_comp != comp[i][j+1])
		    {
			--nc;
		    	--num_y_crx[i][j];
		    }
		}
	    }
	}
	if (debugging("lgb2d"))
	{
	    (void) printf("After remove unphysical crossings step 2:\n");
	    show_box_topology(box,num_x_crx,num_y_crx,comp);
	}

	/* Remove multiple crossings */
	for (i = 0; i < gmax[0]; ++i)
	{
	    for (j = 0; j < gmax[1]; ++j)
	    {
		if ((nc = num_x_crx[i][j+1]) != 0)
		{
		    if (nc%2 == 0)
			num_x_crx[i][j+1] = 0;
		    else if (nc > 1)
			num_x_crx[i][j+1] = 1;
		}
		if ((nc = num_y_crx[i+1][j]) != 0)
		{
		    if (nc%2 == 0)
			num_y_crx[i+1][j] = 0;
		    else if (nc > 1)
			num_y_crx[i+1][j] = 1;
		}
		if (i == 0 && (nc = num_y_crx[i][j]) != 0)
		{
		    if (nc%2 == 0)
			num_y_crx[i][j] = 0;
		    else if (nc > 1)
			num_y_crx[i][j] = 1;
		}
		if (j == 0 && (nc = num_x_crx[i][j]) != 0)
		{
		    if (nc%2 == 0)
			num_x_crx[i][j] = 0;
		    else if (nc > 1)
			num_x_crx[i][j] = 1;
		}
	    }
	}
}	/* end remove_box_unphysical_crx */

EXPORT	int f_elastic_untangle(
	Front	*fr,
	CROSS	**crx)
{
	printf("Entering f_elastic_untangle()\n");
	printf("TODO: code needed!\n");
	clean_up(ERROR);
}	/* end f_elastic_untangle */
