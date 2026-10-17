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
*			fscat3d2.c
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*/

#define DEBUG_STRING    "fscatter"
#include <front/fdecs.h>
#include <intfc/idiagnostic.h>

	/* LOCAL Function Declarations */
LOCAL	INTERFACE *cut_buf_interface2(INTERFACE*,int,int,int*,int*);
LOCAL	boolean      append_adj_intfc_to_buffer2(INTERFACE*,INTERFACE*,RECT_GRID*,
					      RECT_GRID*,int,int);
LOCAL	boolean      append_adj_intfc_to_buffer2_old(INTERFACE*,INTERFACE*,RECT_GRID*,
					      RECT_GRID*,int,int);
LOCAL	boolean      append_buffer_surface2(SURFACE*,SURFACE*,RECT_GRID*,RECT_GRID*,
				         int,int,P_LINK*,int);
LOCAL	boolean      buffer_extension3d2(INTERFACE*,INTERFACE*,int,int,boolean);
LOCAL	boolean      tri_in_matching_strip(TRI*,double,double,int);
LOCAL	boolean      append_reflected_surface(SURFACE*,SURFACE*,RECT_GRID*,
				           RECT_GRID*,int,int,P_LINK*,int);
LOCAL	boolean      append_rfl_intfc_to_buffer(INTERFACE*,INTERFACE*,
				    	     RECT_GRID*,RECT_GRID*,int,int);
LOCAL	boolean      is_reflected_tri(TRI*,TRI*,int,int,RECT_GRID*,int,int*);
LOCAL	boolean      match_tris_in_block(SURFACE*,SURFACE*,RECT_GRID*,TRI**,TRI**,
				      int,int,P_LINK*,int);
LOCAL	boolean      matching_tris(TRI*,TRI*,int*,double*);
LOCAL	boolean      point_on_same_edge(FT_POINT*,FT_POINT*,double*);
LOCAL	boolean      reconstruct_tris_in_rfl_block(SURFACE*,TRI**,TRI**,int*,
	 				        RECT_GRID*,int,int);
LOCAL	boolean      reflect_buffer_interface(INTERFACE*,int,int,boolean);
LOCAL	boolean      tri_on_bound(TRI*,double,int);
LOCAL	boolean      tri_out_domain2(TRI*,double*,double*,int,int);
LOCAL	boolean      tri_side_on_bound(TRI*,double,int,int*);
LOCAL	double     dist2_between_tris(TRI*,TRI*,int*);
LOCAL	void      clip_intfc_at_grid_bdry2(INTERFACE*);
LOCAL	void      open_null_sides2(INTERFACE*,double*,double*,int,int);
LOCAL	void      set_floating_point_tolerance2(RECT_GRID*);
LOCAL	void      detach_tri_side(TRI*,int);
LOCAL	void      merge_block_tris(SURFACE*,TRI**,TRI**,int);
LOCAL	void      stitch_blocks(TRI**,int,TRI**,int);
LOCAL   boolean      is_reflected_point_pair(FT_POINT*,FT_POINT*,double,int,double);
/*#bjet2  functions for merge_curve */
LOCAL   BOND  *find_match_bond(BOND*, CURVE*);
LOCAL   boolean  seal_closed_curve(CURVE*, int);
LOCAL   void  set_btri_on_bond(BOND*,CURVE*);
LOCAL   void  check_bond_comp(const char*, BOND*, BOND*);
LOCAL   void  merge_overlap_curves(INTERFACE*,INTERFACE*);
LOCAL	boolean is_buffer_curve(CURVE*,INTERFACE*);

LOCAL	double	ltol[3];/*LINE TOLERANCE*/
/*TMP*/
LOCAL void print_intfc_nodes(INTERFACE*);
LOCAL void print_intfc_curves(INTERFACE*);
LOCAL void search_the_bond(INTERFACE *intfc);


/*ARGSUSED*/
EXPORT boolean f_intfc_communication3d2(
	Front		*fr)
{
	INTERFACE	*intfc = fr->interf;
	INTERFACE	*adj_intfc[2], *sav_intfc, *buf_intfc;
	PP_GRID		*pp_grid = fr->pp_grid;
	int		me[MAXD], him[MAXD];
	int		myid, dst_id;
	int		*G;
	int		i,j,k;
	int		dim = intfc->dim;
	boolean		sav_copy;
	boolean            status = FUNCTION_SUCCEEDED;

	DEBUG_ENTER(f_intfc_communication3d2)
	if (debugging("trace"))
	{
	    (void) printf("Entering f_intfc_communication3d2()\n");
	}

	set_floating_point_tolerance2(fr->rect_grid);
	sav_copy = copy_intfc_states();
	sav_intfc = current_interface();
	set_copy_intfc_states(YES);

	myid = pp_mynode();
	G = pp_grid->gmax;
	find_Cartesian_coordinates(myid,pp_grid,me);

	if (DEBUG)
	{
	    (void) printf("myid = %d, ",myid);
	    print_int_vector("me = ",me,dim,"\n");
	    print_PP_GRID_structure(pp_grid);
	    (void) printf("Input interface:\n");
	    print_interface(intfc);
	}

		/* Extend interface in three directions */
	
	clip_intfc_at_grid_bdry2(intfc);

	for (i = 0; i < dim; ++i)
	{
	    for (j = 0; j < 2; ++j)
	    {
	    	pp_gsync();
		for (k = 0; k < dim; ++k)
		    him[k] = me[k];

		if (rect_boundary_type(intfc,i,j) == SUBDOMAIN_BOUNDARY)
	        {
		    him[i] = me[i] + 2*j - 1;
		    him[i] = (him[i]+G[i])%G[i];
	        }
		else if (rect_boundary_type(intfc,i,j) == REFLECTION_BOUNDARY)
		{
		    status = reflect_buffer_interface(intfc,i,j,status);
		    set_current_interface(intfc);
		}

	        /*Send buffer region to adjacent domain if necessary*/
                if (rect_boundary_type(intfc,i,j) == SUBDOMAIN_BOUNDARY)
                {
		    dst_id = domain_id(him,G,dim);
		    if (!Is_outside(him,G,i))
		    {
			buf_intfc = cut_buf_interface2(intfc,i,j,me,him);
		
			if (me[i] == him[i])
			    adj_intfc[(j+1)%2] = buf_intfc;
		    	else
			{
			    send_interface(buf_intfc,dst_id);
			    (void) delete_interface(buf_intfc);
			}
			if (DEBUG)
			{
			    (void) printf("Interface sent to dst_id = %d,",
					  dst_id);
			    print_int_vector("him = ",him,dim,"\n");
			}
		    }
	        }

		/*printf("#send after %d %d\n", i, j); */
		/*Receive adjacent buffer region if necessary*/
                if (rect_boundary_type(intfc,i,(j+1)%2) == SUBDOMAIN_BOUNDARY)
                {
		    him[i] = me[i] - 2*j + 1;
		    him[i] = (him[i]+G[i])%G[i];
		    dst_id = domain_id(him,G,dim);
		    if (!Is_outside(him,G,i))
		    {
		        if (me[i] != him[i])
			    adj_intfc[(j+1)%2] = receive_interface(dst_id);

		        if (DEBUG)
		        {
			    (void) printf("Interface received from "
					  "dst_id = %d,",dst_id);
			    print_int_vector("him = ",him,dim,"\n");
		        }
		    }
	        }
	    }
	  
	    for (j = 0; j < 2; ++j)
	    {
		if (rect_boundary_type(intfc,i,j) == SUBDOMAIN_BOUNDARY)
	        {
		    him[i] = me[i] - 2*j + 1;
		    him[i] = (him[i]+G[i])%G[i];
		    if (!Is_outside(him,G,i))
		    {
		        status = buffer_extension3d2(intfc,adj_intfc[j],
						     i,j,status);
			
			(void) delete_interface(adj_intfc[j]);
		        set_current_interface(intfc);

			if (!status)
			{
			  (void) printf("WARNING in "
					"f_intfc_communication3d2 "
					"buffer_extension3d2 failed for "
					"i = %d, j = %d\n",i,j);
			    
			    clean_up(ERROR);
			    goto stat_comm;
			}
		    }
	        }
	    }
stat_comm:
	    if (pp_min_status(status) == NO)
	        goto  exit_comm;
	    
	    reset_intfc_num_points(intfc);
	}


exit_comm: 
	if (status == FUNCTION_SUCCEEDED)
	{
	    install_subdomain_bdry_curves(intfc);
	    reset_intfc_num_points(intfc);
	    if (DEBUG)
	    {
	    	(void) printf("Final intfc:\n");
	    	print_interface(intfc);
	    }
	}
	else
	{
	    (void) printf("WARNING in f_intfc_communication3d2(), "
			  "failure status produced\n");
	    /*clean_up(ERROR); */
	}

	set_copy_intfc_states(sav_copy);
	set_current_interface(sav_intfc);
	DEBUG_LEAVE(f_intfc_communication3d2)
	if (debugging("trace"))
	    (void) printf("Leaving f_intfc_communication3d2()\n");
	return status;
}	/*end f_intfc_communication3d2 */

EXPORT void clip_front_for_output(
        Front           *fr,
	RECT_GRID       *zoom_grid)
{
	INTERFACE    *intfc = fr->interf;
        INTERFACE    *sav_intfc;
        boolean         sav_copy;
	double	     *VL = zoom_grid->VL;
	double	     *VU = zoom_grid->VU;
	double	     *h = zoom_grid->h;
        double        l[MAXD],u[MAXD];
        int          nb,dir, dim = intfc->dim;

	sav_copy = copy_intfc_states();
        set_copy_intfc_states(YES);
        sav_intfc = current_interface();
        strip_subdomain_bdry_curves(intfc);
        set_current_interface(intfc);

	if (!interface_reconstructed(intfc))
	{
	    set_floating_point_tolerance1(fr->rect_grid->h);
	    for (dir = 0; dir < dim; ++dir)
            {
            	for (nb = 0; nb < 2; ++nb)
                    open_null_sides1(intfc,VL,VU,dir,nb);
            }
	}
	else
	{
	    set_floating_point_tolerance2(fr->rect_grid);
	    for (dir = 0; dir < dim; ++dir)
	    {
	    	l[dir] = zoom_grid->VL[dir] - 0.5*h[dir];
	    	u[dir] = zoom_grid->VU[dir] + 0.5*h[dir];
	    }
	    for (dir = 0; dir < dim; ++dir)
            {
            	for (nb = 0; nb < 2; ++nb)
                    open_null_sides2(intfc,l,u,dir,nb);
            }
	}
	cut_out_curves_in_buffer(intfc);
	reset_intfc_num_points(intfc);
	set_current_interface(sav_intfc);
	install_subdomain_bdry_curves(intfc);
        reset_intfc_num_points(intfc);
	set_copy_intfc_states(sav_copy);
}	/* end clip_front_for_output */


LOCAL boolean buffer_extension3d2(
	INTERFACE	*intfc,
	INTERFACE	*adj_intfc,
	int		dir,
	int		nb,
	boolean		status)
{
	BOND		*b;
	NODE		**n;
	CURVE		**c;
	HYPER_SURF	*hs;
	HYPER_SURF_ELEMENT *hse;
	FT_POINT		*p;
	SURFACE		**s;
	TRI		*t;
	RECT_GRID	*gr = computational_grid(intfc);
	RECT_GRID	*adj_gr = computational_grid(adj_intfc);
	RECT_GRID	dual_gr;
	double		T[MAXD];
	int		i;
	int		dim = intfc->dim;

	DEBUG_ENTER(buffer_extension3d2)

	set_current_interface(intfc);

		/* Set periodic shift */

	for (i = 0; i < dim; ++i)
	    T[i] = 0;
	if (nb == 0)				/* Lower neighbor */
	    T[dir] = gr->L[dir] - adj_gr->U[dir];
	else					/* Upper neighbor */
	    T[dir] = gr->U[dir] - adj_gr->L[dir];

		/* Shift points on interface */

	(void) next_point(adj_intfc,NULL,NULL,NULL);
	for (s = adj_intfc->surfaces; s && *s; ++s)
	{
	    for (t = first_tri(*s); !at_end_of_tri_list(t,*s); t = t->next)
	    {
	        for(i=0; i<3; i++)
		{
		    p = Point_of_tri(t)[i];
		    if(!sorted(p))
		    {
		        Coords(p)[dir] += T[dir];
			sorted(p) = YES;
		    }
		}
	    }
	}

	set_dual_grid(&dual_gr,gr);

		/* Patch tris from adj_intfc to intfc */
	if (debugging("scatter_old"))
	{
	    if(!append_adj_intfc_to_buffer2_old(intfc,adj_intfc,gr,&dual_gr,
				dir,nb))
	    {
	        status = FUNCTION_FAILED;
	        (void) printf("WARNING: in buffer_extension3d2(), "
	    	          "append_adj_intfc_to_buffer2_old() failed\n");
	    }
	    DEBUG_LEAVE(buffer_extension3d2)
	    return status;
	}

	if(!append_adj_intfc_to_buffer2(intfc,adj_intfc,gr,&dual_gr,dir,nb))
	{
	    status = FUNCTION_FAILED;
	    (void) printf("WARNING: in buffer_extension3d2(), "
	    	          "append_adj_intfc_to_buffer2() failed\n");
	}
	DEBUG_LEAVE(buffer_extension3d2)
	return status;
}	/*end buffer_extension3d2*/

LOCAL   boolean  seal_closed_curve(CURVE *c, int n)
{
	if(c->first->start != c->last->end)
	{
	    printf("WARNING in seal_closed_curve: curve is not closed.\n");
	    return NO;
	}
   
	/*delete end node */
	if(n == 0)
	{
	    if(!delete_from_pointers(c, &c->end->in_curves))
	    {
	        printf("ERROR in seal_closed_curve, ");
		printf("delete from_end node failed.\n");
		clean_up(ERROR);
	    }
	    delete_node(c->end);
	    c->end=c->start;
	    if(!add_to_pointers(c, &c->end->in_curves))
	    {
	        printf("ERROR in seal_closed_curve, ");
		printf("add_to_end node failed.\n");
		clean_up(ERROR);
	    }
	    return YES;
	}

	/*delete start node */
	if(n == 1)
	{
	    if(!delete_from_pointers(c, &c->start->out_curves))
	    {
		printf("ERROR in seal_closed_curve, ");
		printf("delete from_start node failed.\n");
		clean_up(ERROR);
	    }
	    delete_node(c->start);
	    c->start=c->end;
	    if(!add_to_pointers(c, &c->start->out_curves))
	    {
		printf("ERROR in seal_closed_curve, ");
		printf("add_to_start node failed.\n");
		clean_up(ERROR);
	    }
	    return YES;
	}

	return NO;
}

LOCAL   BOND * find_match_bond(
	BOND *b, 
	CURVE *c)
{
	BOND    *b1;

	for(b1=c->first; b1; b1=b1->next )
	    if(b1->start == b->start && b1->end == b->end)
	        return b1;
	    else  if(b1->start == b->end && b1->end == b->start)
	    {
	        printf("ERROR: find_match_bond, ");
		printf("try to merge reverse order curve.\n");
		clean_up(ERROR);
	    }
	return NULL;
}

LOCAL   void  set_btri_on_bond(
	BOND *b, 
	CURVE *c)
{
	BOND_TRI  **bt;
	
	for(bt = Btris(b); bt && *bt; bt++)
	{
	    (*bt)->curve = c;
	    (*bt)->surface = (*bt)->tri->surf;
	}

}

LOCAL   void  check_bond_comp(
	const char    	*msg,
	BOND    	*b1,
	BOND		*b2)
{
	if(b1->start != b2->start || b1->end != b2->end)
	{
	    printf("ERROR: %s, bonds do not match.\n", msg);
	    clean_up(ERROR);
	}
}

/*b2 is a bond of curve, link btrs on b1 to b2 */
/*delete merged bond connected b1 */
EXPORT   void merge_btris(
	BOND           *b1,
	BOND   	       *b2,
	CURVE	       *curve,
	ORIENTATION    orient,
	INTERFACE      *intfc)
{
	BOND_TRI  **btris, **btris1, *bt;
	int       found;

	while(b1 != NULL && b2 != NULL)
	{
	    check_bond_comp("merge_btris", b1, b2);

	    if(debugging("merge_btris"))
	    {
	        printf("#bond b1 \n");
	        print_bond(b1);
	        for(btris = Btris(b1); btris && *btris; btris++)
	            print_tri((*btris)->tri, intfc);
	    
	        printf("#bond b2\n");
	        print_bond(b2);
	        for(btris = Btris(b2); btris && *btris; btris++)
	            print_tri((*btris)->tri, intfc);
	    }

	    for(btris = Btris(b1); btris && *btris; btris++)
	    {
	        found = NO;
	        for(btris1 = Btris(b2); btris1 && *btris1; btris1++)
		    if((*btris1)->tri == (*btris)->tri)
		    {
		        found = YES;
			break;
		    }
	        if(!found)
		{
		    bt = link_tri_to_bond(NULL,(*btris)->tri,(*btris)->surface,b2,curve);
		    assign_btri_states(bt, *btris);
		}
	    }
	    
	    if(debugging("merge_btris"))
	    {
	        printf("\n#bond af\n");
	        print_bond(b1);
	        print_bond(b2);
	    }

	    if(orient == NEGATIVE_ORIENTATION)
	    {
	        b1 = b1->prev;
	        b2 = b2->prev;
	    }
	    else
	    {
	        b1 = b1->next;
	        b2 = b2->next;
	    }

	}
	
	if(b1 != NULL)
	{
	    printf("ERROR merge_btris, merged curve is longer, need to be preprocessed.\n");
	    clean_up(ERROR);
	}
	
	return;
}

LOCAL  void  merge_overlap_curves(	
	INTERFACE *intfc,
	INTERFACE *adj_intfc)
{
	CURVE     **c, **curve;
	BOND      *b1, *b2;
	BOND_TRI  **btris, **btris1, *bt;
	int       found;


    /*printf("#merge_over bf\n"); */

merge_over_curve:
    for (c = intfc->curves; c && *c; c++)
    {
        if(is_closed_curve(*c))
	    continue;
	if (is_buffer_curve(*c,adj_intfc))
	    continue;
        for(curve = intfc->curves; curve && *curve; curve++) 	
        {
	    if(*curve == *c)
	        continue;
	    
	    b1 = (*c)->first;
	    b2 = find_match_bond(b1, *curve);
	    if(b2 == NULL)
	        continue;
 	    
	    if(is_closed_curve(*c) && is_closed_curve(*curve))
	    {
	        printf("ERROR: merge two closed curve, need to update the code.\n");
		clean_up(ERROR);
	    }
	   
	    /**c is not closed and is included in *curve */
	    while(b1 != NULL)
	    {
	        /*merge bond_tri on bond */
	        for(btris = Btris(b1); btris && *btris; btris++)
		{
		    found = NO;
	            for(btris1 = Btris(b2); btris1 && *btris1; btris1++)
		        if((*btris1)->tri == (*btris)->tri)
			{
			    found = YES;
			    break;
			}
		    if(!found)
		    {
		        bt = link_tri_to_bond(NULL,(*btris)->tri,(*btris)->surface,b2,*curve);
		        assign_btri_states(bt, *btris);
		    }
		}

		if(b1->next == NULL)
		    break;
		else
		    b1 = b1->next;
	        if(b2->next == NULL)
		{
		    if(is_closed_curve(*curve))
		        b2 = (*curve)->first;
		    else
		    {
		        printf("ERROR: merge_overlap_curve, curves do not match.\n");
			clean_up(ERROR);
		    }
		}
		else
		    b2 = b2->next;
		check_bond_comp("merge_overlap_curve", b1, b2);
	    }

	    delete_curve(*c);
	    goto  merge_over_curve;
	}
    }

    /*printf("#merge_over af\n"); */
}

EXPORT   void  merge_curves(
	INTERFACE *intfc,
	INTERFACE *adj_intfc)
{
	CURVE     **c, **curve;
	SURFACE	  *surf;
	BOND      *b, *b1, *b2, *bc, *bo;
	BOND_TRI  **bt;
	NODE      **n;
	int       found_node;
	boolean	  first_match,last_match;
 
merge_curve:
	for (c = intfc->curves; c && *c; c++)
	{
	    if(is_closed_curve(*c))
		continue;

	    if (is_buffer_curve(*c,adj_intfc))
		continue;

	    for(curve=intfc->curves; curve && *curve; curve++) 	
	    {
		if(is_closed_curve(*curve) || *curve == *c)
		    continue;
       
		/* find merged curve in the head of the curve */
		first_match = last_match = NO;
		bo = (*c)->first;
		bc = find_match_bond(bo, *curve);
		if (bc != NULL)
		    first_match = YES;
		else
		{
		    bo = (*c)->last;
		    bc = find_match_bond(bo, *curve);
		    if (bc != NULL)
			last_match = YES;
		}
            
		if(first_match)
		{
		    /*check if c is included in curve, b1 \in c, b2 \in curve */
		    for(b2 = bc, b1=bo; b2 && b1; b2 = b2->next, b1 = b1->next)
			check_bond_comp("merge_curves", b1, b2);
		    if(b1 == NULL)
		    continue;

		    /*now, c is longer than curve in the positive direction */
		    b1 = bo;
		    merge_btris(bc, b1, *c, POSITIVE_ORIENTATION, intfc);
		    for(b2 = bc; b2 != NULL; b2 = b2->prev)
                    {
			b1->prev = b2->prev;
			if(b2->prev != NULL)
			{
			    (*c)->first = b1->prev;
			    (*c)->first->next = b1;
			}
			b1 = (*c)->first;

			/*change the bond tri */
			set_btri_on_bond(b2, *c);
                          
			/*change the starting node */
			(*c)->start->posn = (*c)->first->start;
			if((*c)->first->start == (*c)->last->end)
			{
			    set_btri_on_bond((*c)->first, *c);
			    merge_btris(b1->prev,(*c)->last,*c,
					NEGATIVE_ORIENTATION, intfc);
			    seal_closed_curve(*c, 1);
			    break;
			}
		    }
		    (*c)->first->prev = NULL;
		    delete_curve(*curve);
		    goto  merge_curve;
		}  /* if (first_match) */
		else if (last_match)
		{
		    /*check if c is included in curve, b1 \in c, b2 \in curve */
		    for(b2 = bc, b1=bo; b2 && b1; b2 = b2->prev, b1 = b1->prev)
			check_bond_comp("merge_curves", b1, b2);
		    if(b1 == NULL)
		    continue;

		    /*now, c is longer than curve in the positive direction */
		    b1 = bo;
		    merge_btris(bc, b1, *c, NEGATIVE_ORIENTATION, intfc);
		    for(b2 = bc; b2 != NULL; b2 = b2->next)
                    {
			b1->next = b2->next;
			if(b2->next != NULL)
			{
			    (*c)->last = b1->next;
			    (*c)->last->prev = b1;
			}
			b1 = (*c)->last;

			/*change the bond tri */
			set_btri_on_bond(b2, *c);
                          
			/*change the starting node */
			(*c)->end->posn = (*c)->last->end;
			if((*c)->last->end == (*c)->first->start)
			{
			    set_btri_on_bond((*c)->last, *c);
			    merge_btris(b1->next,(*c)->first,*c,
					POSITIVE_ORIENTATION, intfc);
			    seal_closed_curve(*c, 1);
			    break;
			}
		    }
		    (*c)->last->next = NULL;
		    delete_curve(*curve);
		    goto  merge_curve;
		}  /* else if (last_match) */
	    }   /*for  curve */
	}   /*for c */
    
	for (c = intfc->curves; c && *c; c++)
	    (*c)->num_points = num_points_on_curve(*c);

	/*printf("#merge_curve after\n"); */
	merge_overlap_curves(intfc,adj_intfc);
	/*printf("#merge_overlap_curves after\n"); */
	
	/* delete the nodes which have no curve related */
	for (n = intfc->nodes; n && *n; ++n)
	{
	    found_node = NO;
	    for(c = intfc->curves; c && *c; c++)
	    {
		if((*n) == (*c)->start || (*n) == (*c)->end)
		    found_node = YES;
	    }
	    if(!found_node)
	    {
		delete_node(*n);
		n--;
	    }
	}

	/* BTRIs which are not in the buffer zone are not set */
        
	/*in append_buffer_surface2 (copy_buffer_surface  i_copy_surface),  */
	/*bt->surface = adj_surf */
	/*see function    */
	/* append_buffer_surface2 */
	/*     copy_buffer_surface */
	/*         i_copy_surface */
	/*             copy_tris */
	/*        newbtri = link_tri_to_bond(NULL,newtri,news,nbond,*newc); */
	/* link_tri_list_to_surface   */
	/* this function relinks the tri->surf to surface */
	/* Therefore, We need to reset the bond tri field */
	/* */
	/* btri->tri, btri->bond, btri->orient are correct. */
	/* case 1. the buffer curve is connected with one curve. */
	/* btri->curve is reset in the above merge_curve step,  */
	/* btri->surface will be reset in the following step.  */
	/* case 2. the buffer curve is not connected with one curve. */
	/* btri->curve is correct and do not need to be reset. */
	/* btri->surface will be reset in the following step.  */
	
	for(c = intfc->curves; c && *c; c++)
	{
	    for (b = (*c)->first; b; b = b->next)
	    {
		for(bt = Btris(b); bt && *bt; bt++)
		{
		    surf = (*bt)->tri->surf;
		    if((*bt)->surface != surf)
			(*bt)->surface = surf;
		}
	    }
	}
	
	/*#bjet2 fix the order of btri so it can pass f_consistent_interface */
	order_interface(intfc);
	for (c = intfc->curves; c && *c; c++)
	    reorder_curve_link_list(*c);
}

/*#bjet2 */
/*Make sure, before this function, curve states are connected */
/*see check_surface_curve for the check. the connection is tested before this func */
EXPORT  void average_btris(
	TRI       *ts,
	SURFACE   *ss,
	TRI	  *ta,
	SURFACE   *sa)
{
	INTERFACE *intfc = ss->interface;
	TRI	  *tris[2];
	BOND_TRI  *btris[2];
	static Locstate stmp = NULL;
	Locstate  s1, s2;
	double	  crdss[2][3], crdse[2][3];
	size_t	  sizest = size_of_state(intfc);
	int	  i, j;

	if (stmp == NULL)
	    alloc_state(intfc,&stmp,sizest);

	tris[0] = ts;
	tris[1] = ta;
	
	/*printf("#average_btris\n"); */
	/*print_tri(ts, ss->interface); */
	/*print_tri(ta, sa->interface); */

	for(i=0; i<3; i++)
	{
	    if(Boundary_point(Point_of_tri(ts)[i]) != 
	       Boundary_point(Point_of_tri(ta)[i]))
	    {
	        printf("ERROR Boundary_point inconsistent, impossible for grid based.\n");
	        clean_up(ERROR);
	    }
	    if(is_side_bdry(ts,i) != is_side_bdry(ta,i))
	    {
	        printf("ERROR boundary side inconsistent, impossible for grid based.\n");
		printf("Triangle ts:\n");
		print_tri(ts,intfc);
		printf("Triangle ta:\n");
		print_tri(ta,intfc);
		clean_up(ERROR);
	    }

	    if(is_side_bdry(ts,i))
	    {
	        for(j=0; j<2; j++)
		{
		    btris[j] = Bond_tri_on_side(tris[j],i);
		
		    /*two points corresponding to start and end states */
		    if(btris[j]->orient == POSITIVE_ORIENTATION)
		    {
		        ft_assign(crdss[j], Coords(Point_of_tri(tris[j])[i]), 3*FLOAT);
		        ft_assign(crdse[j], Coords(Point_of_tri(tris[j])[Next_m3(i)]), 3*FLOAT);
		    }
		    else
		    {
		        ft_assign(crdse[j], Coords(Point_of_tri(tris[j])[i]), 3*FLOAT);
		        ft_assign(crdss[j], Coords(Point_of_tri(tris[j])[Next_m3(i)]), 3*FLOAT);
		    }
		}

		s1 = left_start_btri_state(btris[0]);
		s2 = left_start_btri_state(btris[1]);
		bi_interpolate_intfc_states(intfc,0.5,0.5,crdss[0],s1,crdss[1],s2,stmp);
	        ft_assign(s1,stmp,sizest);
	        ft_assign(s2,stmp,sizest);
		
		s1 = left_end_btri_state(btris[0]);
		s2 = left_end_btri_state(btris[1]);
		bi_interpolate_intfc_states(intfc,0.5,0.5,crdse[0],s1,crdse[1],s2,stmp);
	        ft_assign(s1,stmp,sizest);
	        ft_assign(s2,stmp,sizest);
		
		s1 = right_start_btri_state(btris[0]);
		s2 = right_start_btri_state(btris[1]);
		bi_interpolate_intfc_states(intfc,0.5,0.5,crdss[0],s1,crdss[1],s2,stmp);
	        ft_assign(s1,stmp,sizest);
	        ft_assign(s2,stmp,sizest);
		
		s1 = right_end_btri_state(btris[0]);
		s2 = right_end_btri_state(btris[1]);
		bi_interpolate_intfc_states(intfc,0.5,0.5,crdse[0],s1,
					crdse[1],s2,stmp);
	        ft_assign(s1,stmp,sizest);
	        ft_assign(s2,stmp,sizest);
	    }
	}
}

LOCAL 	boolean append_adj_intfc_to_buffer2(
	INTERFACE	*intfc,		/* my interface 	       */
	INTERFACE	*adj_intfc,	/* received interface	       */
	RECT_GRID	*grid,		/* Rectangular grid for region */
	RECT_GRID	*dual_gr,	/* Dual grid for region        */
	int		dir,
	int		nb)
{
	INTERFACE *cur_intfc;
	P_LINK	  *p_table;	/* Table of matching points on intfc
					 * and adj_intfc*/
	SURFACE	  **s, **as, *surf;
        CURVE     **c, **ac; 
	int	  p_size;		/*Size of space allocated for p_table*/
	boolean      status;
	boolean	  corr_surf_found;
        BOND      *b;
        BOND_TRI  **bt;

	DEBUG_ENTER(append_adj_intfc_to_buffer2)

	cur_intfc = current_interface();
	set_current_interface(intfc);

	p_size = 4*(adj_intfc->num_points) + 1;
	uni_array(&p_table,p_size,sizeof(P_LINK));
	reset_hash_table(p_table,p_size);
	
	/* Begin patching adj_intfc to  current interface */
	status = YES;
        	
	for (as = adj_intfc->surfaces; as && *as; ++as)
	{
	    corr_surf_found = NO;
	    for (s = intfc->surfaces; s && *s; ++s)
	    {
		if (surfaces_matched(*as,*s))
		{
		    if(debugging("append_buf"))
		    {
		        printf("surface %p [%d %d] of intfc (%p)\n",
                             *s,negative_component(*s),positive_component(*s),
			     intfc);
			printf("join buf surf %p [%d %d] of adj_intfc (%p)\n",
                             *as,negative_component(*as),
			     positive_component(*as),adj_intfc);
		    }

	            if (!append_buffer_surface2(*s,*as,grid,dual_gr,dir,nb,
					p_table,p_size))
		    {
			(void) printf("WARNING in append_adj_intfc_to_buffer2(), "
		              "append surface failed\n");
			free(p_table);
			set_current_interface(cur_intfc);
			
			DEBUG_LEAVE(append_adj_intfc_to_buffer2)
			return NO;

		    }
		    else
		    {
		        corr_surf_found = YES;
		        break;
		    }

		}
	    }

	    if (!corr_surf_found)
	    {
		SURFACE *surf;
		
		surf = copy_buffer_surface(*as,p_table,p_size);
		Hyper_surf_index(surf) = Hyper_surf_index((*as));
	    }
	}
	
	free(p_table);
	
	merge_curves(intfc,adj_intfc);
	
	set_current_interface(cur_intfc);
	
	if (debugging("consistency"))
	{
	    null_sides_are_consistent();
	    if (!f_consistent_interface(intfc))
	    {
		screen("ERROR in append_adj_intfc_to_buffer2(), "
		       "intfc is inconsistent\n");
		clean_up(ERROR);
	    }
	    else
	        printf("#check after append_adj_intfc_to_buffer2, intfc con.\n");
	}
	
	DEBUG_LEAVE(append_adj_intfc_to_buffer2)
	return status;
}		/*end append_adj_intfc_to_buffer2*/

/*old version of append_adj_intfc.. */
LOCAL 	boolean append_adj_intfc_to_buffer2_old(
	INTERFACE	*intfc,		/* my interface 	       */
	INTERFACE	*adj_intfc,	/* received interface	       */
	RECT_GRID	*grid,		/* Rectangular grid for region */
	RECT_GRID	*dual_gr,	/* Dual grid for region        */
	int		dir,
	int		nb)
{
	INTERFACE *cur_intfc;
	P_LINK	  *p_table;	/* Table of matching points on intfc
					 * and adj_intfc*/
	SURFACE	  **s, **as;
	int	  p_size;		/*Size of space allocated for p_table*/
	boolean      status;
	boolean	  corr_surf_found;

	DEBUG_ENTER(append_adj_intfc_to_buffer2)

	cur_intfc = current_interface();
	set_current_interface(intfc);

	p_size = 4*(adj_intfc->num_points) + 1;
	uni_array(&p_table,p_size,sizeof(P_LINK));
	reset_hash_table(p_table,p_size);

	/* Begin patching adj_intfc to current interface */
	status = YES;
	for (as = adj_intfc->surfaces; as && *as; ++as)
	{
	    corr_surf_found = NO;
	    for (s = intfc->surfaces; s && *s; ++s)
	    {
		/*
		*  COMMENT -
		*  The Hyper_surf_index() function is not
		*  fully supported.  This will fail in the
		*  presence of interface changes in topology
		*  TODO: FULLY SUPPORT THIS OBJECT
		*/
		if (Hyper_surf_index(*s) == Hyper_surf_index(*as))
		{
		    corr_surf_found = YES;
		    if (!append_buffer_surface2(*s,*as,grid,dual_gr,dir,nb,
						p_table,p_size))
		    {
			(void) printf("WARNING in "
			              "append_adj_intfc_to_buffer2(), "
			              "append surface failed\n");
			status = NO;
		    }
		}
	    }
	    if (!corr_surf_found)
	    {
		SURFACE *surf;
		surf = copy_buffer_surface(*as,p_table,p_size);
		Hyper_surf_index(surf) = Hyper_surf_index((*as));
	    }
	}
	free(p_table);
	set_current_interface(cur_intfc);
	if (debugging("consistency"))
	{
	    null_sides_are_consistent();
	    if (!consistent_interface(intfc))
	    {
		screen("ERROR in append_adj_intfc_to_buffer2(), "
		       "intfc is inconsistent\n");
		clean_up(ERROR);
	    }
	}
	DEBUG_LEAVE(append_adj_intfc_to_buffer2)
	return status;
}		/*end append_adj_intfc_to_buffer2*/

LOCAL void set_floating_point_tolerance2(
	RECT_GRID *gr)
{
	int   i;
	double crx_tol;

	crx_tol = line_cross_tolerance(gr);
	for (i = 0; i < 3; ++i)
	    ltol[i] = crx_tol; /*TOLERANCE*/
}		/*end set_floating_point_tolerance2*/

EXPORT	double	line_cross_tolerance(
	RECT_GRID *gr)
{
	double		hmin;
	int		i;

	for (hmin = gr->h[0], i = 1; i < 3; ++i)
	    if (hmin > gr->h[i])
		hmin = gr->h[i];
	return 1.0e-6*hmin; /*TOLERANCE*/
}		/*end line_cross_tolerance*/



LOCAL boolean append_buffer_surface2(
	SURFACE	  *surf,
	SURFACE	  *adj_surf,
	RECT_GRID *grid,
	RECT_GRID *dual_gr,
	int	  dir,
	int	  nb,
	P_LINK	  *p_table,
	int	  p_size)
{
	TRI	**tris_s,**tris_a;
	TRI	*tri;
	int	i, ns, na, gmax[MAXD];
	double	crx_l, crx_u;
        int     idir1, idir2;
        int     i1, i2;
        int     **nbt_s, **nbt_a;
        int     *lbuf = dual_gr->lbuf;
        int     *ubuf = dual_gr->ubuf;
        TRI     **tri_store_s, **tri_store_a;
        TRI     ****blk_tri_s, ****blk_tri_a;
	CURVE   **c;

	DEBUG_ENTER(append_buffer_surface2)

	/*#bjet2 */
	if(debugging("out_surf"))
	{
	    printf("#check curve_surf\n");
	    check_surface_curve(surf);
	    printf("#check_curve adj_surf\n");
	    check_surface_curve(adj_surf);
	}

	idir1 = (dir+1)%3;
        idir2 = (dir+2)%3;
        gmax[dir] = dual_gr->gmax[dir];
        gmax[idir1] = dual_gr->gmax[idir1] + lbuf[idir1] + ubuf[idir1] + 2;
        gmax[idir2] = dual_gr->gmax[idir2] + lbuf[idir2] + ubuf[idir2] + 2;

	uni_array(&tris_s,surf->num_tri,sizeof(TRI *));
	uni_array(&tris_a,adj_surf->num_tri,sizeof(TRI *));
	bi_array(&nbt_s,gmax[idir1],gmax[idir2],INT);
        bi_array(&nbt_a,gmax[idir1],gmax[idir2],INT);
        bi_array(&blk_tri_s,gmax[idir1],gmax[idir2],sizeof(TRI**));
        bi_array(&blk_tri_a,gmax[idir1],gmax[idir2],sizeof(TRI**));

	crx_l = (nb == 0) ? grid->L[dir] - 0.5*grid->h[dir] : 
			    grid->U[dir] - 0.5*grid->h[dir];
	crx_u = (nb == 0) ? grid->L[dir] + 0.5*grid->h[dir] : 
			    grid->U[dir] + 0.5*grid->h[dir];

	ns = 0;
	for (tri = first_tri(surf); !at_end_of_tri_list(tri,surf);
		tri = tri->next)
	{
	    if (tri_in_matching_strip(tri,crx_l,crx_u,dir))
	    {
	    	tris_s[ns++] = tri;
		assign_tri_icoords(dual_gr,tri);
                i1 = Tri_icoords(tri)[idir1] + lbuf[idir1] + 1;
                i2 = Tri_icoords(tri)[idir2] + lbuf[idir2] + 1;
	    	i1 = (i1 < 0) ? 0 : i1;
	    	i2 = (i2 < 0) ? 0 : i2;
	    	i1 = (i1 >= gmax[idir1]) ? gmax[idir1] - 1 : i1;
	    	i2 = (i2 >= gmax[idir2]) ? gmax[idir2] - 1 : i2;
                ++nbt_s[i1][i2];
	    }
	}

	na = 0;
	for (tri = first_tri(adj_surf); !at_end_of_tri_list(tri,adj_surf);
		tri = tri->next)
	{
	    if (tri_in_matching_strip(tri,crx_l,crx_u,dir))
	    {
	    	tris_a[na++] = tri;
		assign_tri_icoords(dual_gr,tri);
                i1 = Tri_icoords(tri)[idir1] + lbuf[idir1] + 1;
                i2 = Tri_icoords(tri)[idir2] + lbuf[idir2] + 1;
	    	i1 = (i1 < 0) ? 0 : i1;
	    	i2 = (i2 < 0) ? 0 : i2;
	    	i1 = (i1 >= gmax[idir1]) ? gmax[idir1] - 1 : i1;
	    	i2 = (i2 >= gmax[idir2]) ? gmax[idir2] - 1 : i2;
                ++nbt_a[i1][i2];
	    }
	}

	uni_array(&tri_store_s,ns,sizeof(TRI*));
        uni_array(&tri_store_a,na,sizeof(TRI*));

	ns = na = 0;
        for (i1 = 0; i1 < gmax[idir1]; ++i1)
        {
            for (i2 = 0; i2 < gmax[idir2]; ++i2)
            {
                if (nbt_s[i1][i2] != 0)
                {
                    blk_tri_s[i1][i2] = tri_store_s+ns;
                    ns += nbt_s[i1][i2];
                    nbt_s[i1][i2] = 0;
                }
                if (nbt_a[i1][i2] != 0)
                {
                    blk_tri_a[i1][i2] = tri_store_a+na;
                    na += nbt_a[i1][i2];
                    nbt_a[i1][i2] = 0;
                }
            }
        }
	for (i = 0; i < ns; ++i)
        {
            i1 = Tri_icoords(tris_s[i])[idir1] + lbuf[idir1] + 1;
            i2 = Tri_icoords(tris_s[i])[idir2] + lbuf[idir2] + 1;
	    i1 = (i1 < 0) ? 0 : i1;
	    i2 = (i2 < 0) ? 0 : i2;
	    i1 = (i1 >= gmax[idir1]) ? gmax[idir1] - 1 : i1;
	    i2 = (i2 >= gmax[idir2]) ? gmax[idir2] - 1 : i2;
            blk_tri_s[i1][i2][nbt_s[i1][i2]] = tris_s[i];
            ++nbt_s[i1][i2];
        }
        for (i = 0; i < na; ++i)
        {
            i1 = Tri_icoords(tris_a[i])[idir1] + lbuf[idir1] + 1;
            i2 = Tri_icoords(tris_a[i])[idir2] + lbuf[idir2] + 1;
	    i1 = (i1 < 0) ? 0 : i1;
	    i2 = (i2 < 0) ? 0 : i2;
	    i1 = (i1 >= gmax[idir1]) ? gmax[idir1] - 1 : i1;
	    i2 = (i2 >= gmax[idir2]) ? gmax[idir2] - 1 : i2;
            blk_tri_a[i1][i2][nbt_a[i1][i2]] = tris_a[i];
            ++nbt_a[i1][i2];
        }

	for (i1 = 0; i1 < gmax[idir1]; ++i1)
        {
            for (i2 = 0; i2 < gmax[idir2]; ++i2)
            {
                if (nbt_s[i1][i2] != nbt_a[i1][i2])
                {
		    FT_POINT *p;
		    int j;
                    (void) printf("WARNING in append_buffer_surface2(), "
                                  "local and adjacent sides have "
                                  "different number of tris\n");
		    (void) printf("i1 = %d  i2 = %d\n",i1,i2);
		    (void) printf("Local number of tris: %d\n",
					nbt_s[i1][i2]);
		    for (i = 0; i < nbt_s[i1][i2]; ++i)
		    {
			print_tri(blk_tri_s[i1][i2][i], surf->interface);
			(void) printf("\n");
		    }
		    (void) printf("Adjacent number of tris: %d\n",
					nbt_a[i1][i2]);
		    for (i = 0; i < nbt_a[i1][i2]; ++i)
		    {
			print_tri(blk_tri_a[i1][i2][i], adj_surf->interface);
			(void) printf("\n");
		    }
	            free_these(8,nbt_s,nbt_a,blk_tri_s,blk_tri_a,tris_s,tris_a,
                               tri_store_s,tri_store_a);
		    DEBUG_LEAVE(append_buffer_surface2)
                    return FUNCTION_FAILED;
                }
                if (nbt_s[i1][i2] == 0)
		    continue;
		
                if (!match_tris_in_block(surf,adj_surf,grid,
					 blk_tri_s[i1][i2],blk_tri_a[i1][i2],
					 nbt_s[i1][i2],nb,p_table,p_size))
                {
                    (void) printf("WARNING in append_buffer_surface2(), "
                                  "match_tris_in_block() failed nb=%d i1=%d i2=%d\n",
				  nb, i1, i2);
	            free_these(8,nbt_s,nbt_a,blk_tri_s,blk_tri_a,tris_s,tris_a,
                               tri_store_s,tri_store_a);
		    DEBUG_LEAVE(append_buffer_surface2)
                    return FUNCTION_FAILED;
                }
            }
        }

	/*printf("#append_buffer_surface\n"); */
	adj_surf = copy_buffer_surface(adj_surf,p_table,p_size);
        
	for (i1 = 0; i1 < gmax[idir1]; ++i1)
        {
            for (i2 = 0; i2 < gmax[idir2]; ++i2)
            {
                nbt_s[i1][i2] = 0;
                nbt_a[i1][i2] = 0;
	    }
	}
	na = 0;
	for (tri = first_tri(adj_surf); !at_end_of_tri_list(tri,adj_surf);
		tri = tri->next)
	{
	    if (tri_in_matching_strip(tri,crx_l,crx_u,dir))
	    {
	    	tris_a[na++] = tri;
		assign_tri_icoords(dual_gr,tri);
	    }
	}
	for (i = 0; i < ns; ++i)
        {
            i1 = Tri_icoords(tris_s[i])[idir1] + lbuf[idir1] + 1;
            i2 = Tri_icoords(tris_s[i])[idir2] + lbuf[idir2] + 1;
	    i1 = (i1 < 0) ? 0 : i1;
	    i2 = (i2 < 0) ? 0 : i2;
	    i1 = (i1 >= gmax[idir1]) ? gmax[idir1] - 1 : i1;
	    i2 = (i2 >= gmax[idir2]) ? gmax[idir2] - 1 : i2;
            blk_tri_s[i1][i2][nbt_s[i1][i2]] = tris_s[i];
            ++nbt_s[i1][i2];
        }
        for (i = 0; i < na; ++i)
        {
            i1 = Tri_icoords(tris_a[i])[idir1] + lbuf[idir1] + 1;
            i2 = Tri_icoords(tris_a[i])[idir2] + lbuf[idir2] + 1;
	    i1 = (i1 < 0) ? 0 : i1;
	    i2 = (i2 < 0) ? 0 : i2;
	    i1 = (i1 >= gmax[idir1]) ? gmax[idir1] - 1 : i1;
	    i2 = (i2 >= gmax[idir2]) ? gmax[idir2] - 1 : i2;
            blk_tri_a[i1][i2][nbt_a[i1][i2]] = tris_a[i];
            ++nbt_a[i1][i2];
        }

	/*adjoin adj_surf tri list to surf tri list */
	last_tri(surf)->next = first_tri(adj_surf);
	first_tri(adj_surf)->prev = last_tri(surf);
	link_tri_list_to_surface(first_tri(surf),last_tri(adj_surf),surf);

	/*#bjet2  BEGIN curves in adj_surf should be added to surf.  */
	for(c=adj_surf->pos_curves; c && *c; c++)
	    if(! delete_from_pointers(adj_surf, &(*c)->pos_surfaces))
	    {
	        printf("ERROR: in append_buffer_surface2, "
		       "adj_surf and pos_curves are not paired.\n");
		clean_up(ERROR);
	    }
	    else
	        install_curve_in_surface_bdry(surf, *c, POSITIVE_ORIENTATION);

	for(c=adj_surf->neg_curves; c && *c; c++)
	    if(! delete_from_pointers(adj_surf, &(*c)->neg_surfaces))
	    {
	        printf("ERROR: in append_buffer_surface2, "
		       "adj_surf and neg_curves are not paired.\n");
		clean_up(ERROR);
	    }
	    else
	        install_curve_in_surface_bdry(surf, *c, NEGATIVE_ORIENTATION);
	/*#bjet2 END */

	adj_surf->pos_curves = adj_surf->neg_curves = NULL;
	(void) delete_surface(adj_surf);
	adj_surf = NULL;

	for (i1 = 0; i1 < gmax[idir1]; ++i1)
        {
            for (i2 = 0; i2 < gmax[idir2]; ++i2)
            {
                if (nbt_s[i1][i2] == 0)
		    continue;
		merge_block_tris(surf,blk_tri_s[i1][i2],
			         blk_tri_a[i1][i2],nbt_s[i1][i2]);
	    }
	}

	free_these(8,nbt_s,nbt_a,blk_tri_s,blk_tri_a,tris_s,tris_a,
                   tri_store_s,tri_store_a);
	DEBUG_LEAVE(append_buffer_surface2)
	return YES;
}		/*end append_buffer_surface2*/


LOCAL	boolean tri_in_matching_strip(
	TRI   *tri,
	double crx_l,
	double crx_u,
	int   dir)
{
	double tri_center;
	int   i;

	tri_center = 0.0;
	for (i = 0; i < 3; ++i)
	    tri_center += Coords(Point_of_tri(tri)[i])[dir];
	tri_center /= 3.0;
	if (((crx_l - ltol[dir]) <= tri_center        ) && 
	    ((        tri_center <= (crx_u + ltol[dir]))))
	    return YES;
	else
	    return NO;
}		/*end tri_in_matching_strip*/

LOCAL void open_null_sides2(
	INTERFACE	*intfc,
	double		*L,
	double		*U,
	int		dir,
	int		nb)
{
	TRI		*tri;
	SURFACE 	**s;

	DEBUG_ENTER(open_null_sides2)
	if (DEBUG)
	{
	    static const char *xyz[] = { "x", "y", "z"};
	    static const char *sname[] = { "lower", "upper"};
	    (void) printf("Clipping interface in at %s %s side\n",
			  sname[nb],xyz[dir]);
	}

	for (s = intfc->surfaces; s && *s; ++s)
	{
	    for (tri=first_tri(*s); !at_end_of_tri_list(tri,*s); tri=tri->next)
	    {
	    	if (tri_out_domain2(tri,L,U,dir,nb))
	    	{
	    	    remove_out_domain_tri(tri,*s);
	    	}
	    }
	}
	for (s = intfc->surfaces; s && *s; ++s)
	{
	    if (no_tris_on_surface(*s))
	    {
	    	(void) delete_surface(*s);
		--s;
	    }
	}
	DEBUG_LEAVE(open_null_sides2)
}		/*end open_null_sides2*/

LOCAL boolean tri_out_domain2(
	TRI		*tri,
	double		*L,
	double		*U,
	int		dir,
	int		nb)
{
	double		tri_center, LB, UB;
	int		i;

	tri_center = 0.0;
	for (i = 0; i < 3; ++i)
	    tri_center += Coords(Point_of_tri(tri)[i])[dir];
	tri_center /= 3.0;
	if (nb == 0)
	{
	    LB = L[dir] - ltol[dir];
	    if (tri_center <= LB)
		return YES;
	}
	else
	{
	    UB = U[dir] + ltol[dir];
	    if (tri_center >= UB)
		return YES;
	}
	return NO;
}	/* end tri_out_domain2 */

LOCAL void clip_intfc_at_grid_bdry2(
	INTERFACE	*intfc)
{
	INTERFACE	*cur_intfc = current_interface();
	RECT_GRID	*gr = computational_grid(intfc);
	int		dim = intfc->dim;
	int		dir,nb;
	double		L[MAXD],U[MAXD];

	DEBUG_ENTER(clip_intfc_at_grid_bdry2)

	strip_subdomain_bdry_curves(intfc);
	set_current_interface(intfc);
	for (dir = 0; dir < dim; ++dir)
	{
	    L[dir] = gr->L[dir] - 0.5*gr->h[dir];
	    U[dir] = gr->U[dir] + 0.5*gr->h[dir];
	    
	    if(rect_boundary_type(intfc,dir,0) == OPEN_BOUNDARY)
		L[dir] = gr->VL[dir] - 0.5*gr->h[dir];
	    if(rect_boundary_type(intfc,dir,1) == OPEN_BOUNDARY)
		U[dir] = gr->VU[dir] + 0.5*gr->h[dir];
	}
	
	for (dir = 0; dir < dim; ++dir)
	{
	    for (nb = 0; nb < 2; ++nb)
	    {
	    	open_null_sides2(intfc,L,U,dir,nb);
	    }
	}

	/*#bjet2  cut the 3 comp curve */
	cut_out_curves_in_buffer(intfc);
	
	reset_intfc_num_points(intfc);
	set_current_interface(cur_intfc);
	DEBUG_LEAVE(clip_intfc_at_grid_bdry2)
}		/*end clip_intfc_at_grid_bdry2*/


LOCAL	INTERFACE  *cut_buf_interface2(
	INTERFACE	*intfc,
	int		dir,
	int		nb,
	int		*me,
	int		*him)
{
	INTERFACE	*sav_intfc, *tmp_intfc, *buf_intfc;
	RECT_GRID	*gr = computational_grid(intfc);
	RECT_GRID	dual_gr;
	double		L[3], U[3];
	boolean		sav_copy = copy_intfc_states();

	set_copy_intfc_states(YES);
	sav_intfc = current_interface();

	set_size_of_intfc_state(size_of_state(intfc));
	tmp_intfc = copy_interface(intfc);
	set_dual_grid(&dual_gr,gr);

	if (nb == 0)
	{
	    L[dir] = gr->L[dir] - 0.5*gr->h[dir];
	    U[dir] = gr->L[dir] + (gr->L[dir] - dual_gr.VL[dir]) + gr->h[dir];
	}
	else
	{
	    L[dir] = gr->U[dir] - (dual_gr.VU[dir] - gr->U[dir]) - gr->h[dir];
	    U[dir] = gr->U[dir] + 0.5*gr->h[dir];
	}

	open_null_sides2(tmp_intfc,L,U,dir,(nb+1)%2);

	cut_out_curves_in_buffer(tmp_intfc);

	reset_intfc_num_points(tmp_intfc);

	if (me[dir] == him[dir])
	{
	    buf_intfc = tmp_intfc;
	}
	else
	{
	    set_size_of_intfc_state(size_of_state(intfc));
	    buf_intfc = copy_interface(tmp_intfc);
	    (void) delete_interface(tmp_intfc);
	}
	set_copy_intfc_states(sav_copy);
	set_current_interface(sav_intfc);

	if (debugging("consistency"))
	{
	    printf("#check cut_buf_interface2\n");
	    null_sides_are_consistent();
	    if (!consistent_interface(buf_intfc))
	    {
		screen("ERROR in cut_buf_interface2(), "
		       "cut interface is inconsistent\n");
		clean_up(ERROR);
	    }
	}

	return buf_intfc;

}		/*end cut_buf_interface2*/

LOCAL	boolean  reflect_buffer_interface(
	INTERFACE *intfc,
	int dir,
	int nb,
	boolean status)
{
	INTERFACE	*rfl_intfc,*sav_intfc;
	RECT_GRID       *gr = computational_grid(intfc);
	RECT_GRID       dual_gr;
	double           L[3],U[3];
	boolean         sav_copy = copy_intfc_states();
	double		nor[3],posn[3];
	int		i,dim = intfc->dim;

	DEBUG_ENTER(reflect_buffer_interface)

	set_copy_intfc_states(YES);
	sav_intfc = current_interface();
	set_size_of_intfc_state(size_of_state(intfc));
	set_dual_grid(&dual_gr,gr);
	rfl_intfc = copy_interface(intfc);
	if (nb == 0)
	{
	    L[dir] = dual_gr.L[dir];
	    U[dir] = dual_gr.L[dir] + (dual_gr.ubuf[dir] + 2)*dual_gr.h[dir];
	}
	else
	{
	    L[dir] = dual_gr.U[dir] - (dual_gr.lbuf[dir] + 2)*dual_gr.h[dir];
	    U[dir] = dual_gr.U[dir];
	}
	open_null_sides2(rfl_intfc,L,U,dir,(nb+1)%2);
	if (rfl_intfc->surfaces == NULL)
	{
	    delete_interface(rfl_intfc);
	    set_copy_intfc_states(sav_copy);
	    set_current_interface(sav_intfc);
	    DEBUG_LEAVE(reflect_buffer_interface);
	    return FUNCTION_SUCCEEDED;
	}

	reset_intfc_num_points(rfl_intfc);

	/* Reflect buffer interface */

	for (i = 0; i < dim; ++i)
	    nor[i] = posn[i] = 0.0;

	/* Reflect buffer interface */

	nor[dir] = 1.0;
	posn[dir] = (nb == 0) ? gr->L[dir] : gr->U[dir];
	reflect_interface(rfl_intfc,posn,nor);

	if (!append_rfl_intfc_to_buffer(intfc,rfl_intfc,gr,&dual_gr,dir,nb))
	{
	    status = FUNCTION_FAILED;
	    (void) printf("WARNING in reflect_buffer_interface(), "
	                  "append_rfl_intfc_to_buffer() failed\n");
	    return status;
	}
	delete_interface(rfl_intfc);
	set_copy_intfc_states(sav_copy);
	set_current_interface(sav_intfc);

	DEBUG_LEAVE(reflect_buffer_interface)
	return status;
}	/* end reflect_buffer_interface */

LOCAL	boolean append_rfl_intfc_to_buffer(
	INTERFACE *intfc,
	INTERFACE *rfl_intfc,
	RECT_GRID *gr,
	RECT_GRID *dual_gr,
	int	  dir,
	int	  nb)
{
	INTERFACE	*cur_intfc;
	P_LINK		*p_table;	/* Table of matching points on intfc
					 * and rfl_intfc*/
	SURFACE		**s, **rs;
	int		p_size;		/*Size of space allocated for p_table*/

	DEBUG_ENTER(append_rfl_intfc_to_buffer)

	cur_intfc = current_interface();
	set_current_interface(intfc);

	p_size = 4*(rfl_intfc->num_points) + 1;
	uni_array(&p_table,p_size,sizeof(P_LINK));
	reset_hash_table(p_table,p_size);

	/* Begin patching rfl_intfc to current interface */
	for (s = intfc->surfaces; s && *s; ++s)
	{
	    for (rs = rfl_intfc->surfaces; rs && *rs; ++rs)
	    {
		/*
		*	COMMENT -
		*	The Hyper_surf_index() function is not
		*	fully supported.  This will fail in the
		*	presences of interface changes in topology
		*	TODO: FULLY SUPPORT THIS OBJECT
		*/
		if (Hyper_surf_index(*s) == Hyper_surf_index(*rs))
		{
		    if (!append_reflected_surface(*s,*rs,gr,dual_gr,dir,nb,
					p_table,p_size))
		    {
			set_current_interface(cur_intfc);
			(void) printf("WARNING in ");
			(void) printf("append_rfl_intfc_to_buffer(), ");
			(void) printf("append surface failed\n");
			DEBUG_LEAVE(append_rfl_intfc_to_buffer)
			return FUNCTION_FAILED;
		    }
		}
	    }
	}
	free(p_table);
	set_current_interface(cur_intfc);
	DEBUG_LEAVE(append_rfl_intfc_to_buffer)
	return FUNCTION_SUCCEEDED;
}	/* end append_rfl_intfc_to_buffer */

LOCAL	boolean append_reflected_surface(
	SURFACE		*surf,
	SURFACE         *rfl_surf,
	RECT_GRID       *gr,
	RECT_GRID       *dual_gr,
	int             dir,
	int             nb,
	P_LINK          *p_table,
	int             p_size)
{
	TRI             **tris_s,**tris_r;
	TRI             *tri;
	int             i,ns,nr,gmax[MAXD];
	double           crx_l,crx_u;
	int		idir1,idir2;
	int		i1,i2;
	int		**nbt_s,**nbt_r;
	int		*lbuf = dual_gr->lbuf;
	int		*ubuf = dual_gr->ubuf;
	TRI		**tri_store_s,**tri_store_r;
	TRI		****blk_tri_s,****blk_tri_r;
	TRI             *tris1[8],*tris2[8];

	DEBUG_ENTER(append_reflected_surface)

	idir1 = (dir+1)%3;
	idir2 = (dir+2)%3;
	gmax[dir] = dual_gr->gmax[dir];
	gmax[idir1] = dual_gr->gmax[idir1] + lbuf[idir1] + ubuf[idir1] + 2;
	gmax[idir2] = dual_gr->gmax[idir2] + lbuf[idir2] + ubuf[idir2] + 2;

	uni_array(&tris_s,rfl_surf->num_tri,sizeof(TRI *));
	uni_array(&tris_r,rfl_surf->num_tri,sizeof(TRI *));
	bi_array(&nbt_s,gmax[idir1],gmax[idir2],INT);
	bi_array(&nbt_r,gmax[idir1],gmax[idir2],INT);
	bi_array(&blk_tri_s,gmax[idir1],gmax[idir2],sizeof(TRI**));
	bi_array(&blk_tri_r,gmax[idir1],gmax[idir2],sizeof(TRI**));

	crx_l = (nb == 0) ? gr->L[dir] - 0.5*gr->h[dir] :
			gr->U[dir] - 0.5*gr->h[dir];
	crx_u = (nb == 0) ? gr->L[dir] + 0.5*gr->h[dir] :
			gr->U[dir] + 0.5*gr->h[dir];

	rfl_surf = copy_buffer_surface(rfl_surf,p_table,p_size);

	ns = 0;
	for (tri = first_tri(surf); !at_end_of_tri_list(tri,surf);
			tri = tri->next)
	{
	    if (tri_in_matching_strip(tri,crx_l,crx_u,dir))
	    {
		tris_s[ns++] = tri;
		assign_tri_icoords(dual_gr,tri);
		i1 = Tri_icoords(tri)[idir1] + lbuf[idir1] + 1;
		i2 = Tri_icoords(tri)[idir2] + lbuf[idir2] + 1;
	    	i1 = (i1 < 0) ? 0 : i1;
	    	i2 = (i2 < 0) ? 0 : i2;
	    	i1 = (i1 >= gmax[idir1]) ? gmax[idir1] - 1 : i1;
	    	i2 = (i2 >= gmax[idir2]) ? gmax[idir2] - 1 : i2;
		++nbt_s[i1][i2];
	    }
	}
	nr = 0;
	for (tri = first_tri(rfl_surf); !at_end_of_tri_list(tri,rfl_surf);
			tri = tri->next)
	{
	    if (tri_in_matching_strip(tri,crx_l,crx_u,dir))
	    {
		tris_r[nr++] = tri;
		assign_tri_icoords(dual_gr,tri);
		i1 = Tri_icoords(tri)[idir1] + lbuf[idir1] + 1;
		i2 = Tri_icoords(tri)[idir2] + lbuf[idir2] + 1;
	    	i1 = (i1 < 0) ? 0 : i1;
	    	i2 = (i2 < 0) ? 0 : i2;
	    	i1 = (i1 >= gmax[idir1]) ? gmax[idir1] - 1 : i1;
	    	i2 = (i2 >= gmax[idir2]) ? gmax[idir2] - 1 : i2;
		++nbt_r[i1][i2];
	    }
	}
	uni_array(&tri_store_s,ns,sizeof(TRI*));
	uni_array(&tri_store_r,nr,sizeof(TRI*));

	/* adjoin rfl_surf tri list to surf tri list */

	last_tri(surf)->next = first_tri(rfl_surf);
	first_tri(rfl_surf)->prev = last_tri(surf);
	link_tri_list_to_surface(first_tri(surf),last_tri(rfl_surf),surf);
	rfl_surf->pos_curves = rfl_surf->neg_curves = NULL;
	(void) delete_surface(rfl_surf);
	rfl_surf = NULL;

	ns = nr = 0;
	for (i1 = 0; i1 < gmax[idir1]; ++i1)
	{
	    for (i2 = 0; i2 < gmax[idir2]; ++i2)
	    {
		if (nbt_s[i1][i2] != 0)
		{
		    blk_tri_s[i1][i2] = &tri_store_s[ns];
		    ns += nbt_s[i1][i2];
		    nbt_s[i1][i2] = 0;
		}
		if (nbt_r[i1][i2] != 0)
		{
		    blk_tri_r[i1][i2] = &tri_store_r[nr];
		    nr += nbt_r[i1][i2];
		    nbt_r[i1][i2] = 0;
		}
	    }
	}
	for (i = 0; i < ns; ++i)
	{
	    i1 = Tri_icoords(tris_s[i])[idir1] + lbuf[idir1] + 1;
	    i2 = Tri_icoords(tris_s[i])[idir2] + lbuf[idir2] + 1;
	    i1 = (i1 < 0) ? 0 : i1;
	    i2 = (i2 < 0) ? 0 : i2;
	    i1 = (i1 >= gmax[idir1]) ? gmax[idir1] - 1 : i1;
	    i2 = (i2 >= gmax[idir2]) ? gmax[idir2] - 1 : i2;
	    blk_tri_s[i1][i2][nbt_s[i1][i2]] = tris_s[i];
	    ++nbt_s[i1][i2];
	}
	for (i = 0; i < nr; ++i)
	{
	    i1 = Tri_icoords(tris_r[i])[idir1] + lbuf[idir1] + 1;
	    i2 = Tri_icoords(tris_r[i])[idir2] + lbuf[idir2] + 1;
	    i1 = (i1 < 0) ? 0 : i1;
	    i2 = (i2 < 0) ? 0 : i2;
	    i1 = (i1 >= gmax[idir1]) ? gmax[idir1] - 1 : i1;
	    i2 = (i2 >= gmax[idir2]) ? gmax[idir2] - 1 : i2;
	    blk_tri_r[i1][i2][nbt_r[i1][i2]] = tris_r[i];
	    ++nbt_r[i1][i2];
	}

	for (i1 = 0; i1 < gmax[idir1]; ++i1)
	{
	    for (i2 = 0; i2 < gmax[idir2]; ++i2)
	    {
		if (nbt_s[i1][i2] != nbt_r[i1][i2])
		{
		    (void) printf("WARNING in append_reflected_surface(), "
				  "local and reflected sides have "
		                  "different number of tris\n");
		    return FUNCTION_FAILED;
		}
		if (nbt_s[i1][i2] == 0) continue;
		if (!reconstruct_tris_in_rfl_block(surf,blk_tri_s[i1][i2],
			blk_tri_r[i1][i2],&nbt_s[i1][i2],gr,dir,nb))
		{
		    (void) printf("WARNING in append_reflected_surface(), "
				  "reconstruct_tris_in_rfl_block() failed\n");
		    return FUNCTION_FAILED;
		}
	    }
	}
	for (i1 = 0; i1 < gmax[idir1]; ++i1)
	{
	    for (i2 = 0; i2 < gmax[idir2]; ++i2)
	    {
		if (nbt_s[i1][i2] == 0) continue;
		if ((i1 != gmax[idir1] -1) && (nbt_s[i1+1][i2] != 0))
		{
		    ns = 0;
		    for (i = 0; i < nbt_s[i1][i2]; ++i)
		    {
			tris1[ns++] = blk_tri_s[i1][i2][i];
			tris1[ns++] = blk_tri_r[i1][i2][i];
		    }
		    nr = 0;
		    for (i = 0; i < nbt_s[i1+1][i2]; ++i)
		    {
			tris2[nr++] = blk_tri_s[i1+1][i2][i];
			tris2[nr++] = blk_tri_r[i1+1][i2][i];
		    }
		    stitch_blocks(tris1,ns,tris2,nr);
		}
		if ((i2 != gmax[idir2] -1) && (nbt_s[i1][i2+1] != 0))
		{
		    ns = 0;
		    for (i = 0; i < nbt_s[i1][i2]; ++i)
		    {
			tris1[ns++] = blk_tri_s[i1][i2][i];
			tris1[ns++] = blk_tri_r[i1][i2][i];
		    }
		    nr = 0;
		    for (i = 0; i < nbt_s[i1][i2+1]; ++i)
		    {
			tris2[nr++] = blk_tri_s[i1][i2+1][i];
			tris2[nr++] = blk_tri_r[i1][i2+1][i];
		    }
		    stitch_blocks(tris1,ns,tris2,nr);
		}
	    }
	}

	free_these(8,nbt_s,nbt_r,blk_tri_s,blk_tri_r,tris_s,tris_r,
			tri_store_s,tri_store_r);

	DEBUG_LEAVE(append_reflected_surface)
	return FUNCTION_SUCCEEDED;
}	/* end append_reflected_surface */

LOCAL	boolean reconstruct_tris_in_rfl_block(
	SURFACE	*surf,
	TRI **tris_s,
	TRI **tris_r,
	int *nt,
	RECT_GRID *gr,
	int dir,
	int nb)
{
	TRI   *ts, *tr;
	FT_POINT **ps, **pr;
	double bs;
	int   side, rside;
	int   i, j, j1, j2;

	bs = (nb == 0) ? gr->L[dir] + 0.5*gr->h[dir] : 
			 gr->U[dir] - 0.5*gr->h[dir];

	for (i = 0; i < *nt; ++i)
	{
	    ts = tris_s[i];
	    if (ts == NULL)
		continue;
	    ps = Point_of_tri(ts);
	    if (tri_on_bound(ts,bs,dir))
		continue;
	    else if (tri_side_on_bound(ts,bs,dir,&side))
	    {
		for (j = 0; j < *nt; ++j)
		{
		    tr = tris_r[j];
		    if (tr == NULL)
			continue;
	            pr = Point_of_tri(tr);
		    if (is_reflected_tri(ts,tr,dir,nb,gr,side,&rside))
		    {
			ps[(side+2)%3]  = pr[(rside+1)%3];
			pr[(rside+2)%3] = ps[(side+1)%3];
			Tri_on_side(ts,( side+1)%3) = tr;
			Tri_on_side(tr,(rside+1)%3) = ts;
			detach_tri_side(ts,(side+2)%3);
			detach_tri_side(tr,(rside+2)%3);
			set_normal_of_tri(ts);
			set_normal_of_tri(tr);
			break;
		    }
		}
	    }
	    else
	    {
		for (j = 0; j < *nt; ++j)
		{
		    tr = tris_r[j];
		    if (tr == NULL) continue;
		    if (is_reflected_tri(ts,tr,dir,nb,gr,side,&rside))
		    {
			remove_tri_from_surface(tr,surf,NO);
			tris_r[j] = NULL;
			break;
		    }
		}
		remove_tri_from_surface(ts,surf,NO);
		tris_s[i] = NULL;
	    }
	}
	j1 = 0;
	for (i = 0; i < *nt; ++i)
	{
	    if (tris_s[i] != NULL)
	    {
		tris_s[j1++] = tris_s[i];
	    }
	}
	j2 = 0;
	for (i = 0; i < *nt; ++i)
	{
	    if (tris_r[i] != NULL)
	    {
	    	tris_r[j2++] = tris_r[i];
	    }
	}
	if (j1 != j2)
	{
	    (void) printf("WARNING in reconstruct_tris_in_rfl_block(), "
	                  "Final number of triangles do not match "
	                  "j1 = %d  j2 = %d\n",j1,j2);
	    return FUNCTION_FAILED;
	}
	*nt = j1;
	return FUNCTION_SUCCEEDED;
}	/* end reconstruct_tris_in_rfl_block */

LOCAL	boolean tri_on_bound(
	TRI *tri,
	double line,
	int dir)
{
	if (fabs(Coords(Point_of_tri(tri)[0])[dir] - line) < ltol[dir] &&
	    fabs(Coords(Point_of_tri(tri)[1])[dir] - line) < ltol[dir] &&
	    fabs(Coords(Point_of_tri(tri)[2])[dir] - line) < ltol[dir])
		return YES;
	else
		return NO;
}	/* end tri_on_bound */

LOCAL	boolean tri_side_on_bound(
	TRI *tri,
	double line,
	int dir,
	int *side)
{
	FT_POINT *p[3];
	int i;

	p[0] = Point_of_tri(tri)[0];
	p[1] = Point_of_tri(tri)[1];
	p[2] = Point_of_tri(tri)[2];
	for (i = 0; i < 3; ++i)
	{
	    if (fabs(Coords(p[i])[dir] - line) < ltol[dir] &&
		fabs(Coords(p[(i+1)%3])[dir]-line) < ltol[dir])
	    {
		if (fabs(Coords(p[(i+2)%3])[dir]-line) < ltol[dir])
		    return NO;	/* parallel triangle */
		else
		{
		    *side = i;
		    return YES;
		}
	    }
	}
	*side = 0;
	return NO;
}	/* end tri_side_on_bound */

LOCAL	boolean is_reflected_tri(
	TRI *ts,
	TRI *tr,
	int dir,
	int nb,
	RECT_GRID *gr,
	int side,
	int *rside)
{
	double ml = (nb == 0) ? 2.0*gr->L[dir] : 2.0*gr->U[dir];
	FT_POINT *sp[3];
	FT_POINT *rp[3];
	double h = gr->h[dir];
	int i;

	for (i = 0; i < 3; ++i)
	{
	    sp[i] = Point_of_tri(ts)[(i+side)%3];
	    rp[i] = Point_of_tri(tr)[i];
	}
	for (i = 0; i < 3; ++i)
	{
	    if (is_reflected_point_pair(sp[0],rp[(i+1)%3],ml,dir,h) &&
		is_reflected_point_pair(sp[1],rp[i],ml,dir,h) &&
		is_reflected_point_pair(sp[2],rp[(i+2)%3],ml,dir,h))
	    {
		*rside = i;
		return YES;
	    }
	}
	return NO;
}	/* end is_reflected_tri */

LOCAL	boolean is_reflected_point_pair(
	FT_POINT *ps,
	FT_POINT *pr,
	double ml,
	int dir,
	double h)
{
	if (fabs(ml - Coords(ps)[dir] - Coords(pr)[dir]) < 0.00001*h &&
	    fabs(Coords(ps)[(dir+1)%3] - Coords(pr)[(dir+1)%3]) < 0.00001*h &&
	    fabs(Coords(ps)[(dir+2)%3] - Coords(pr)[(dir+2)%3]) < 0.00001*h)
	    return YES;
	else return NO;
}	/* end is_reflected_point_pair */

LOCAL	void stitch_blocks(
	TRI **tris1,
	int nt1,
	TRI **tris2,
	int nt2)
{
	TRI *tri1, *tri2;
	int i,j;

	for (i = 0; i < nt1; ++i)
	{
	    tri1 = tris1[i];
	    for (j = 0; j < nt2; ++j)
	    {
	    	tri2 = tris2[j];
		link_neighbor_tris(tri1,tri2);
	    }
	}
}	/* end stitch_blocks */

LOCAL	void detach_tri_side(
	TRI *tri,
	int side)
{
	if (!is_side_bdry(tri,side))
	{
	    TRI *nbtri = Tri_on_side(tri,side);
	    if (nbtri == NULL) return;
	    else if (Tri_on_side01(nbtri) == tri)
		Tri_on_side01(nbtri) = NULL;
	    else if (Tri_on_side12(nbtri) == tri)
		Tri_on_side12(nbtri) = NULL;
	    else if (Tri_on_side20(nbtri) == tri)
		Tri_on_side20(nbtri) = NULL;
	}
	else
	{
	    BOND_TRI *bt = Bond_tri_on_side(tri,side);
	    (void) delete_from_pointers(bt,&Btris(bt->bond));
	}
	Neighbor_on_side(tri,side) = NULL;
}	/* end detach_tri_side */


LOCAL   boolean match_tris_in_block(
        SURFACE   *ss,
        SURFACE   *sa,
	RECT_GRID *gr,
        TRI       **tris_s,
        TRI       **tris_a,
        int       nt,
        int       nb,
	P_LINK    *p_table,
	int       p_size)
{
	TRI                *ts, *ta;
	int                i, j, jmin, k, i_rot;
	HYPER_SURF_ELEMENT *hse, *hae;
        HYPER_SURF         *hs, *ha;
	FT_POINT              *ps, *pa,*pp;
	double              min_d2;
	static double       **dist = NULL;
	static int         **rot = NULL;
	static int         n_alloc = 0;

	DEBUG_ENTER(match_tris_in_block)
	if (nt > n_alloc)
	{
	    if (dist)
		free_these(2,dist,rot);
	    n_alloc = 2*nt;
	    bi_array(&dist,n_alloc,n_alloc,FLOAT);
	    bi_array(&rot,n_alloc,n_alloc,INT);
	}
	for (i = 0; i < nt; ++i)
	    for (j = 0; j < nt; ++j)
		dist[i][j] = dist2_between_tris(tris_s[i],tris_a[j],&rot[i][j]);
	for (i = 0; i < nt; ++i)
	{
	    ts = tris_s[i];
	    jmin = -1;
	    min_d2 = HUGE_VAL;
	    for (j = 0; j < nt; ++j)
	    {
		if (tris_a[j] != NULL)
		{
		    if (dist[i][j] < min_d2)
		    {
			min_d2 = dist[i][j];
			jmin = j;
		    }
		}
	    }
	    if (jmin == -1)
	    {
		screen("ERROR in match_tris_in_block(), "
		       "can't find closest tri to tri %llu\n",
		       tri_number(ts,ss->interface));
		(void) printf("ltol = %g %g %g\n",ltol[0],ltol[1],ltol[2]);
		print_tri(ts,ss->interface);
	        for (k = 0, j = 0; j < nt; ++j)
		    if (tris_a[j])
			++k;
		(void) printf("%d Remaining candidates\n",k);
	        for (j = 0; j < nt; ++j)
		{
		    if (tris_a[j])
			print_tri(tris_a[j],sa->interface);
		}
		clean_up(ERROR);
	    }
	    ta = tris_a[jmin];
	    if (matching_tris(ts,ta,&i_rot,gr->h))
	    {
		if (i_rot != rot[i][jmin])
		{
		    screen("ERROR in match_tris_in_block(), "
		           "inconsistent rotatations for tri ts %llu "
			   "and tri ta %llu\n",
		           tri_number(ts,ss->interface),
		           tri_number(ta,sa->interface));
		    (void) printf("i_rot = %d, rot[%d][%d] = %d\n",
				  i_rot,i,jmin,rot[i][jmin]);
		    (void) printf("dist[%d][%d] = %g\n",i,jmin,dist[i][jmin]);
		    (void) printf("ltol = %g %g %g\n",ltol[0],ltol[1],ltol[2]);
		    (void) printf("tri ts\n");
		    print_tri(ts,ss->interface);
		    (void) printf("tri ta\n");
		    print_tri(ta,sa->interface);
		    clean_up(ERROR);
		}
		if (i_rot != 0)
		    rotate_triangle(ts,(nb==0)?3-i_rot:i_rot);
		for (k = 0; k < 3; ++k)
		{
		    ps = Point_of_tri(ts)[k];
                    pa = Point_of_tri(ta)[k];
		    pp = (FT_POINT*)find_from_hash_table((POINTER)pa,
                                                      p_table,p_size);
		    if (pp == NULL)
		    {
			(void) add_to_hash_table((POINTER)pa,(POINTER)ps,
                                                 p_table,p_size);
			hse = Hyper_surf_element(ts);
                        hae = Hyper_surf_element(ta);
			hs = Hyper_surf(ss);
			ha = Hyper_surf(sa);
			
			/*#bjet2  used to deactivate the boundary point average. */
			set_full_average(NO);
			(void) average_points(NO,ps,hse,hs,pa,hae,ha);
			set_full_average(YES);
		    }
		}
		/*#bjet2  for boundary points */
		average_btris(ts, ss, ta, sa);
		/*
		if(debugging("match_tris_in_block") && the_tri(ts))
		{
		    printf("#match %p %p   %p %p\n", ts, ss, ta, sa);
		    print_tri(ts,ss->interface);
		    print_tri(ta,sa->interface);
		
		    print_tri_states(ts, Hyper_surf(ss));
		    print_tri_states(ta, Hyper_surf(sa));
		}
		*/
		set_normal_of_tri(ts);
		tris_s[i] = NULL;
		tris_a[jmin] = NULL;
	    }
	    else
	    {
		(void) printf("WARNING in match_tris_in_block(), "
			      "unmatched tri found in match_tris_in_block\n");
		(void) printf("Unmatched tri %llu\n",
			      tri_number(ts,ss->interface));
		(void) printf("rot[%d][%d] = %d\n",i,jmin,rot[i][jmin]);
		(void) printf("dist[%d][%d] = %g\n",i,jmin,dist[i][jmin]);
		(void) printf("ltol = %g %g %g\n",ltol[0],ltol[1],ltol[2]);
		print_tri(ts,ss->interface);
	        for (k = 0, j = 0; j < nt; ++j)
		    if (tris_a[j])
			++k;
		(void) printf("%d Remaining candidates\n",k);
	        for (j = 0; j < nt; ++j)
		{
		    if (tris_a[j])
			print_tri(tris_a[j],sa->interface);
		}
		(void) printf("Aborting matching\n");
	        DEBUG_LEAVE(match_tris_in_block)
		return NO;
	    }
	}
	DEBUG_LEAVE(match_tris_in_block)
	return YES;
	
}	/* end match_tris_in_block */

LOCAL	double dist2_between_tris(
	TRI *tri1,
	TRI *tri2,
	int *rot)
{
	double *p1[3], *p2[3];
	double d2, min_d2;

	if ((tri1 == NULL) || (tri2 == NULL))
	{
	    *rot = 0;
	    return HUGE_VAL;
	}

	p1[0] = Coords(Point_of_tri(tri1)[0]);
	p1[1] = Coords(Point_of_tri(tri1)[1]);
	p1[2] = Coords(Point_of_tri(tri1)[2]);

	p2[0] = Coords(Point_of_tri(tri2)[0]);
	p2[1] = Coords(Point_of_tri(tri2)[1]);
	p2[2] = Coords(Point_of_tri(tri2)[2]);

	min_d2 = sqr(p1[0][0]-p2[0][0])+sqr(p1[0][1]-p2[0][1])+sqr(p1[0][2]-p2[0][2]) +
	         sqr(p1[1][0]-p2[1][0])+sqr(p1[1][1]-p2[1][1])+sqr(p1[1][2]-p2[1][2]) +
	         sqr(p1[2][0]-p2[2][0])+sqr(p1[2][1]-p2[2][1])+sqr(p1[2][2]-p2[2][2]);
	*rot = 0;

	d2 = sqr(p1[0][0]-p2[1][0])+sqr(p1[0][1]-p2[1][1])+sqr(p1[0][2]-p2[1][2]) +
	     sqr(p1[1][0]-p2[2][0])+sqr(p1[1][1]-p2[2][1])+sqr(p1[1][2]-p2[2][2]) +
	     sqr(p1[2][0]-p2[0][0])+sqr(p1[2][1]-p2[0][1])+sqr(p1[2][2]-p2[0][2]);
	if (d2 < min_d2)
	{
	    min_d2 = d2;
	    *rot = 1;
	}
	d2 = sqr(p1[0][0]-p2[2][0])+sqr(p1[0][1]-p2[2][1])+sqr(p1[0][2]-p2[2][2]) +
	     sqr(p1[1][0]-p2[0][0])+sqr(p1[1][1]-p2[0][1])+sqr(p1[1][2]-p2[0][2]) +
	     sqr(p1[2][0]-p2[1][0])+sqr(p1[2][1]-p2[1][1])+sqr(p1[2][2]-p2[1][2]);
	if (d2 < min_d2)
	{
	    min_d2 = d2;
	    *rot = 2;
	}
	return min_d2;
}		/*end dist2_between_tris*/

LOCAL	boolean matching_tris(
	TRI   *tri1,
	TRI   *tri2,
	int   *i_rot,
	double *h)
{
	FT_POINT *p1[3], *p2[3];
	int i;

	p1[0] = Point_of_tri(tri1)[0];
	p1[1] = Point_of_tri(tri1)[1];
	p1[2] = Point_of_tri(tri1)[2];

	p2[0] = Point_of_tri(tri2)[0];
	p2[1] = Point_of_tri(tri2)[1];
	p2[2] = Point_of_tri(tri2)[2];

	for (i = 0; i < 3; ++i)
	{
	    if (point_on_same_edge(p1[0],p2[i],h) &&
		point_on_same_edge(p1[1],p2[(i+1)%3],h) &&
		point_on_same_edge(p1[2],p2[(i+2)%3],h))
	    {
		*i_rot = i;
		return YES;
	    }
		
	}
	return NO;
}	/* end matching_tris */

LOCAL	boolean point_on_same_edge(
	FT_POINT *p1,
	FT_POINT *p2,
	double *h)
{
	int i, num_same_coords;

	num_same_coords = 0;
	for (i = 0; i < 3; ++i)
	{
	    if (fabs(Coords(p1)[i] - Coords(p2)[i]) > 0.5*h[i])
		return NO;
	    else if (fabs(Coords(p1)[i] - Coords(p2)[i]) < ltol[i])
		++num_same_coords;
	}
	return (num_same_coords >= 2) ? YES : NO;
}

LOCAL	void merge_block_tris(
	SURFACE *s,
	TRI **tris_s,
	TRI **tris_a,
	int nt)
{
	int i,j;
	TRI *ts,*ta;

	for (i = 0; i < nt; ++i)
	{
	    ts = tris_s[i];
	    for (j = 0; j < nt; ++j)
	    {
		ta = tris_a[j];
		if (ta == NULL)
		    continue;
		if (Point_of_tri(ts)[0] == Point_of_tri(ta)[0] &&
		    Point_of_tri(ts)[1] == Point_of_tri(ta)[1] &&
		    Point_of_tri(ts)[2] == Point_of_tri(ta)[2])
		{
		    merge_two_tris(ts,ta,s,s);
		    tris_a[j] = NULL;
		}
	    }
	}
}	/* end merge_block_tris */

LOCAL	boolean is_buffer_curve(
	CURVE *curve,
	INTERFACE *adj_intfc)
{
	CURVE **c;
	FT_POINT *start_posn,*end_posn;
	int i,dim = adj_intfc->dim;

	for (c = adj_intfc->curves; c && *c; ++c)
	{
	    start_posn = (*c)->start->posn;
	    end_posn   = (*c)->end->posn;
	    for (i = 0; i < dim; ++i)
	    {
		if (Coords(start_posn)[i] != Coords(curve->start->posn)[i])
		    break;
		if (Coords(end_posn)[i] != Coords(curve->end->posn)[i])
		    break;
	    }
	    if (i != dim) 
		continue;
	    else 
	    {
		return YES;
	    }
	}
	return NO;
}	/* end is_buffer_curve */

/*TMP*/
static void print_intfc_nodes(
	INTERFACE *intfc)
{
	NODE **n;
	int count = 0;
	for (n = intfc->nodes; n && *n; ++n)
	{
	    count++;
	}
	printf("%d nodes:\n",count);
	for (n = intfc->nodes; n && *n; ++n)
	    print_node(*n);
}	/* end print_intfc_nodes */

static void print_intfc_curves(
	INTERFACE *intfc)
{
	CURVE **c;
	int count = 0;
	for (c = intfc->curves; c && *c; ++c)
	{
	    count++;
	}
	printf("%d curves:\n",count);
	for (c = intfc->curves; c && *c; ++c)
	{
	    print_curve(*c);
	}
}	/* end print_intfc_curves */

LOCAL void search_the_bond(INTERFACE *intfc)
{
	CURVE **c;
	BOND *b;
	for (c = intfc->curves; c && *c; ++c)
	{
	    for (b = (*c)->first; b != NULL; b = b->next)
		if (the_bond(b))
		{
		    printf("The bond found:\n");
		    print_bond(b);
		    print_curve(*c);
		}
	}
}	/* end search_the_bond */
