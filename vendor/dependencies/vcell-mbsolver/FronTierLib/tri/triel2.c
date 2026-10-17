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
*			triel2.c
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Triangulation of mesh blocks containing intfc nodes.
*/


#if defined(TWOD)
#include <tri/trilocaldecs.h>

	/* LOCAL Function Declarations */
LOCAL	ORIENTATION	find_bond_and_orient_at_crx(CRXING*,int,BOND**);
LOCAL	boolean	crxl_in_blk_crx_list(BLK_CRX_LIST*,BLK_CRX_LIST*);
LOCAL	int	is_proper_elN1(INTERFACE*,BOND***);
LOCAL	int	point_in_crx_list(FT_POINT*,BOND*,CRXING**,CRXING**);
LOCAL	boolean	triangulate_elN1(TRI_GRID*,int*,int,int*,int,int,int*,int,int,
				 int*,int,int,int*,int,int,NODE**,TG_PT**,
				 LINEAR_ELEMENT**,BILINEAR_ELEMENT**,
				 INTERFACE*);
LOCAL	void	add_corner_to_blk_crx_list(BLK_CRX_LIST**,int,GRID_CORNER,
                                           TRI_GRID*);
LOCAL	void	add_crx_to_blk_crx_list(TRI_GRID*,int*,BLK_CRX_LIST**,
                                        GRID_DIRECTION,INTERFACE*,NODE**,int);
LOCAL	void	find_nearest_crx_connected_to_node(BLK_CRX_LIST*,
						   BLK_CRX_LIST**,
						   BLK_CRX_LIST*,
						   BLK_CRX_LIST*,
						   ANGLE_DIRECTION);
LOCAL	void	free_blk_node_list(BLK_EL1*);
LOCAL	void	free_next_blk_node(BLK_NODE*);
LOCAL	void	insert_cur_seg(NODE*,BOND*,CURVE*,ORIENTATION,
			       CRXING***,CURVE***,INTERFACE*);
LOCAL	void	install_blk_curve(BOND*,BOND*,NODE*,NODE*,CURVE*,ORIENTATION,
				  INTERFACE*);
LOCAL	void	make_blk_crx_lists(TRI_GRID*,int,int*,int,int,int*,int,int,
				   int*,int,int,int*,int,int,NODE**,INTERFACE*,
				   BOND**,BLK_CRX_LIST**,BLK_CRX_LIST**,
				   BLK_CRX_LIST**,BLK_CRX_LIST**,
				   BLK_CRX_LIST**);
LOCAL	void	saw_off_blk_corner(BLK_CRX_LIST*,BLK_CRX_LIST*,BLK_CRX_LIST**);
LOCAL	void	set_interior_blk_node_list(NODE***,TG_PT***,
					   int,int*,TRI_GRID*);
LOCAL	void	state_and_comp_at_crx_connected_to_node(BLK_CRX_LIST*,int,int,
							Locstate*,COMPONENT*);
LOCAL	void	triangulate_corner_el(TRI_GRID*,BLK_CRX_LIST*,
				      LINEAR_ELEMENT**,int*);
LOCAL	void	triangulate_single_node_el(TRI_GRID*,TG_PT*,BLK_CRX_LIST*,
					   LINEAR_ELEMENT**,int*);
#if defined(DEBUG_TRI_GRID)
LOCAL	void	print_blk_corner(BLK_CORNER*);
LOCAL	void	print_blk_crx_list(BLK_CRX_LIST*,const char*);
LOCAL	void	print_blk_crx_list_element(BLK_CRX_LIST*);
#endif /* defined(DEBUG_TRI_GRID) */

/*ARGSUSED*/
LIB_LOCAL	boolean triangulate_blk_with_nodes(
	TRI_GRID	*ntg,
	int		*icoords,
	LINEAR_ELEMENT	**tri,
	BILINEAR_ELEMENT **qd)
{
	INTERFACE	*save_intfc;
	INTERFACE	*blk_intfc = NULL;
	NODE		**node;
 	TG_PT		**nd;
	int		num_nodes = Blk_el1(icoords,ntg).interior_node;
	int		ncs, ncw, ncn, nce;
	int		*s_list, *e_list, *n_list, *w_list;
 	int		jse, jne, jnw, jsw;
	boolean		status = NO;

#if defined(DEBUG_TRI_GRID)
	debug_print("blk_with_nodes","Entered triangulate_blk_with_nodes()\n");
	if (debugging("blk_with_nodes"))
	{
    	(void) printf("\nblk ix %d iy %d has %d interior nodes\n",
		      icoords[0],icoords[1],num_nodes);
	}
#endif /* defined(DEBUG_TRI_GRID) */

	retrieve_crossing_lists(ntg,icoords,&ncs,&ncw,&ncn,&nce,&jsw,&jse,&jnw,
				&jne,&s_list,&w_list,&n_list,&e_list);

	set_interior_blk_node_list(&node,&nd,num_nodes,icoords,ntg);

	if (num_nodes == 1)
	{
	    save_intfc = current_interface();
	    blk_intfc = make_blk_intfc(ntg,ncs,s_list,jse,nce,e_list,
				       jne,ncn,n_list,jnw,ncw,w_list,jsw,
				       num_nodes,node);
	    status = triangulate_elN1(ntg,icoords,ncs,s_list,jse,nce,e_list,
				      jne,ncn,n_list,jnw,ncw,w_list,jsw,
				      num_nodes,node,nd,tri,qd,blk_intfc);
	    (void) delete_interface(blk_intfc);
	    blk_intfc = NULL;
	    set_current_interface(save_intfc);
	}

	if (status == FUNCTION_FAILED)
	{
	    status = exact_triangulate_el3(ntg,node,num_nodes,icoords,
				           ncs,ncw,ncn,nce,jsw,jse,jnw,jne,
				           s_list,w_list,n_list,e_list,tri);
	    if (status == FUNCTION_FAILED)
	    {
	        (void) printf("WARNING in triangulate_blk_with_nodes(), "
	                      "exact_triangulate_el3() failed\n");
	    }
	}
	if (node != NULL)
	    free(node);
	if (nd != NULL)
	    free(nd);
#if defined(DEBUG_TRI_GRID)
	debug_print("blk_with_nodes","Left triangulate_blk_with_nodes()\n");
#endif /* defined(DEBUG_TRI_GRID) */
	return status;
}		/*end triangulate_blk_with_nodes*/


EXPORT	boolean exact_blk_triangulate(
	TRI_GRID	*ntg,
	int		*icoords,
	LINEAR_ELEMENT	**lin,
	BILINEAR_ELEMENT **bilin)
{
	NODE		**node;
	boolean		status;
	int		num_nodes = Blk_el1(icoords,ntg).interior_node;
	int		ncs, ncw, ncn, nce;
	int		*s_list, *e_list, *n_list, *w_list;
 	int		jse, jne, jnw, jsw;

#if defined(DEBUG_TRI_GRID)
	debug_print("exact_tri","Entered exact_blk_triangulate()\n");
#endif /* defined(DEBUG_TRI_GRID) */

	retrieve_crossing_lists(ntg,icoords,&ncs,&ncw,&ncn,&nce,&jsw,&jse,&jnw,
				&jne,&s_list,&w_list,&n_list,&e_list);

	/* Check for grid block not containing interface curves */
	if ((num_nodes + ncs + ncw + ncn + nce) == 0)
	{
	    set_quad(ntg,icoords,jsw,jse,jnw,jne,bilin);
	    return YES;
	}

	set_interior_blk_node_list(&node,NULL,num_nodes,icoords,ntg);

	status = exact_triangulate_el3(ntg,node,num_nodes,icoords,
			               ncs,ncw,ncn,nce,jsw,jse,jnw,jne,s_list,
			               w_list,n_list,e_list,lin);

	if (status == FUNCTION_FAILED)
	{
	    (void) printf("WARNING in exact_blk_triangulate(), "
		          "exact_triangulate_el3() failed\n");
	}

	if (node != NULL)
	    free(node);
#if defined(DEBUG_TRI_GRID)
	debug_print("exact_tri","Left exact_blk_triangulate()\n");
#endif /* defined(DEBUG_TRI_GRID) */
	return status;
}		/*end exact_blk_triangulate*/


LOCAL	void set_interior_blk_node_list(
	NODE		***pnode,
	TG_PT		***pnd,
	int		num_nodes,
	int		*icoords,
	TRI_GRID	*ntg)
{
	BLK_NODE	*blkn;
	NODE		**node;
 	TG_PT		**nd;
	int		i;

	if (num_nodes == 0)
	{
	    if (pnode != NULL)
	        *pnode = NULL;
	    if (pnd != NULL)
	        *pnd = NULL;
	    return;
	}

	if (pnode != NULL)
	{
	    uni_array(pnode,num_nodes,sizeof(NODE *));
	    node = *pnode;
	}
	else
	    node = NULL;
	if (pnd != NULL)
	{
	    uni_array(pnd,num_nodes,sizeof(TG_PT *));
	    nd = *pnd;
	}
	else
	    nd = NULL;
	for (blkn = Blk_el1(icoords,ntg).blk_node, i = 0; blkn != NULL;
	     blkn = blkn->next, ++i)
	{
	    if (node != NULL)
		node[i] = blkn->n;
	    if (nd != NULL)
		nd[i]   = blkn->nd;
	}

#if defined(DEBUG_TRI_GRID)
	if (debugging("exact_tri"))
	{
	    (void) printf("node list -\n");
	    if (nd != NULL && node != NULL)
	    {
	    	for (i = 0;  i < num_nodes;  ++i)
	    	{
	    	    (void) printf("i %d  nd: %g %g  node:\n",
	    			  i,Coords(nd[i])[0],Coords(nd[i])[1]);
	    	    print_node(node[i]);
	    	}
	    }
	    else if (node != NULL)
	    {
	    	for (i = 0;  i < num_nodes;  ++i)
	    	{
	    	    (void) printf("i %d  node:\n",i);
	    	    print_node(node[i]);
	    	}
	    }
	    else if (nd != NULL)
	    {
	    	for (i = 0;  i < num_nodes;  ++i)
	    	{
	    	    (void) printf("i %d  nd: %g %g\n",
	    			  i,Coords(nd[i])[0],Coords(nd[i])[1]);
	    	}
	    }
	}
#endif /* defined(DEBUG_TRI_GRID) */
}		/*end set_interior_blk_node_list*/



/*
*			triangulate_elN1():
*
*	Attempts to triangulate a block containing a single node.
*	Will fail if the sub grid interface becomes tangled when
*	all of the sub grid curves are replaced by single bonds.
*/


LOCAL boolean triangulate_elN1(
	TRI_GRID	*ntg,
	int		*icoords,
	int		ncs,
	int		*s_list,
	int		jse,
	int		nce,
	int		*e_list,
	int		jne,
	int		ncn,
	int		*n_list,
	int		jnw,
	int		ncw,
	int		*w_list,
	int		jsw,
	int		num_nodes,
	NODE		**node,
	TG_PT		**nd,
	LINEAR_ELEMENT	**lin,
	BILINEAR_ELEMENT **bilin,
	INTERFACE	*blk_intfc)
{
	BOND		**blk_bonds;
	BLK_CRX_LIST	*nd_crxlst;
	BLK_CRX_LIST	*sw_crxlst, *se_crxlst, *ne_crxlst, *nw_crxlst;
	BLK_EL0		*blk_el0;
	int		*t_num;
	Table		*T = table_of_interface(ntg->grid_intfc);

#if defined(DEBUG_TRI_GRID)
	debug_print("blk_with_nodes","Entered triangulate_elN1()\n");
#endif /* defined(DEBUG_TRI_GRID) */

	if (!is_proper_elN1(blk_intfc,&blk_bonds)) 
	{
#if defined(DEBUG_TRI_GRID)
	    debug_print("blk_with_nodes","Left triangulate_elN1(), answer = NO\n");
#endif /* defined(DEBUG_TRI_GRID) */
	    return NO;
	}

	/* Make a linked list of the crosses */

	make_blk_crx_lists(ntg,ncs,s_list,jse,nce,e_list,jne,ncn,n_list,jnw,ncw,
			   w_list,jsw,num_nodes,node,blk_intfc,blk_bonds,
		           &nd_crxlst,&sw_crxlst,&se_crxlst,&ne_crxlst,
			   &nw_crxlst);
	
	blk_el0 = &Blk_el0(icoords,ntg);
	if (nd_crxlst) 
	{
	    blk_el0_linear_els(blk_el0) = *lin;
	    t_num = &num_lin_els_in_blk(blk_el0);
	    triangulate_single_node_el(ntg,nd[0],nd_crxlst,lin,t_num);
	    if (sw_crxlst)
	    	triangulate_corner_el(ntg,sw_crxlst,lin,t_num);
	    if (se_crxlst)
	    	triangulate_corner_el(ntg,se_crxlst,lin,t_num);
	    if (ne_crxlst)
	    	triangulate_corner_el(ntg,ne_crxlst,lin,t_num);
	    if (nw_crxlst)
	    	triangulate_corner_el(ntg,nw_crxlst,lin,t_num);
	}
	else /* Node doesn't affect triangulation */
	{
	    boolean status = FUNCTION_FAILED;

	    if (ncs && (ncw || ncn || nce))
	    {
	    	blk_el0_linear_els(blk_el0) = *lin;
	    	t_num = &num_lin_els_in_blk(blk_el0);

	    	status = triangulate_el0(ntg,ncs,s_list,jse,
	    				 nce,e_list,jne,ncn,n_list,jnw,
	    				 ncw,w_list,jsw,lin,t_num);

	    	if (status == FUNCTION_SUCCEEDED)
	    	    return YES;
	    }
		
	    if (nce && (ncw || ncn || ncs))
	    {
	    	blk_el0_linear_els(blk_el0) = *lin;
	    	t_num = &num_lin_els_in_blk(blk_el0);

	    	status = triangulate_el0(ntg,nce,e_list,jne,ncn,n_list,jnw,ncw,
					 w_list,jsw,ncs,s_list,jse,lin,t_num);

	    	if (status == FUNCTION_SUCCEEDED)
	    	    return YES;
	    }
		
	    if (ncn && (ncw || ncs || nce))
	    {
	    	blk_el0_linear_els(blk_el0) = *lin;
	    	t_num = &num_lin_els_in_blk(blk_el0);

	    	status = triangulate_el0(ntg,ncn,n_list,jnw,
					 ncw,w_list,jsw,ncs,s_list,jse,
					 nce,e_list,jne,lin,t_num);

		if (status == FUNCTION_SUCCEEDED)
		    return YES;
	    }
		
	    if (ncw && (ncs || ncn || nce))
	    {
	    	blk_el0_linear_els(blk_el0) = *lin;
	    	t_num = &num_lin_els_in_blk(blk_el0);

	    	status = triangulate_el0(ntg,ncw,w_list,jsw,
					 ncs,s_list,jse,nce,e_list,jne,
					 ncn,n_list,jnw,lin,t_num);

		if (status == FUNCTION_SUCCEEDED)
		    return YES;
	    }
		
		/* This case is topologically indistinguishable */
		/* from a rectangle with no cuts as far as the	*/
		/* crxing list can discern			*/

	    (*bilin)->p[0] = &(ntg->node_points[jsw]);		
	    (*bilin)->p[1] = &(ntg->node_points[jse]);
	    (*bilin)->p[2] = &(ntg->node_points[jne]);
	    (*bilin)->p[3] = &(ntg->node_points[jnw]);
	    (*bilin)->comp = T->components[jsw];

	    blk_el0_bilinear_el(blk_el0) = *bilin;

#if defined(DEBUG_TRI_GRID)
	    if (debugging("triangulate"))
	    	print_BILINEAR_ELEMENT(*bilin,ntg);
#endif /* defined(DEBUG_TRI_GRID) */

	    set_bilinear_blk_el0(blk_el0);
	    ++ntg->n_bilin_els;
	    ++(*bilin);

	    return YES;
	}
#if defined(DEBUG_TRI_GRID)
	debug_print("blk_with_nodes","Left triangulate_elN1(), answer = YES\n");
#endif /* defined(DEBUG_TRI_GRID) */
	return YES;
}		/*end triangulate_elN1*/

LOCAL	void triangulate_corner_el(
	TRI_GRID	*ntg,
	BLK_CRX_LIST	*crxlst,
	LINEAR_ELEMENT	**tri,
	int		*t_num)
{
	BLK_CRX_LIST	*first, *last;
	BLK_CRX_LIST	*corner[3];
	CRXING		*cx_s, *cx_e, *cx_n, *cx_w;
	int		*s_list, *e_list, *n_list, *w_list;
	int		ncs, nce, ncn, ncw;
	int		nc[4];
	int		jse, jne, jnw;
	int		num_corners;

	first = crxlst;
	num_corners = 0;
	nc[0] = nc[1] = nc[2] = nc[3] = 0;
	for (last = first; last; last = last->next)
	{
	    if (last->corner)
	    {
	    	corner[num_corners] = last;
	    	++num_corners;
	    }
	    else
	    	++nc[num_corners];
	    if (!last->next)
	        break;
	}

	cx_s = first->crx;
	jse = corner[0]->corner->index;
	ncs = nc[0] - 1;
	s_list = (ncs > 0) ? first->next->crx_index : NULL;
	switch (num_corners)
	{
	case 1:
	    cx_e = last->crx;
	    nce = nc[1] - 1;
	    e_list = (nce > 0) ? corner[0]->next->crx_index : NULL;
	    triangulate_el1(ntg,cx_s,ncs,s_list,jse,nce,e_list,cx_e,
			    tri,t_num);
	    return;

	case 2:
	    nce = nc[1];
	    e_list = (nce > 0) ? corner[0]->next->crx_index : NULL;
	    cx_n = last->crx;
	    jne = corner[1]->corner->index;
	    ncn = nc[2] - 1;
	    n_list = (ncn > 0) ? corner[1]->next->crx_index : NULL;
	    triangulate_el2(ntg,cx_s,ncs,s_list,jse,nce,e_list,jne,
			    ncn,n_list,cx_n,tri,t_num);
	    return;

	case 3:
	    nce = nc[1];
	    e_list = (nce > 0) ? corner[0]->next->crx_index : NULL;
	    ncn = nc[2];
	    n_list = (ncn > 0) ? corner[1]->next->crx_index : NULL;
	    jne = corner[1]->corner->index;
	    cx_w = last->crx;
	    jnw = corner[2]->corner->index;
	    ncw = nc[3] - 1;
	    w_list = (ncw > 0) ? corner[2]->next->crx_index : NULL;
	    triangulate_el4(ntg,cx_s,ncs,s_list,jse,nce,e_list,jne,
			    ncn,n_list,jnw,ncw,w_list,cx_w,tri,t_num);
	    return;

	default:
	    screen("ERROR in triangulate_corner_el(), Invalid number "
		   "of corners\n");
	    clean_up(ERROR);
	}
}		/*end triangulate_corner_el*/

#define crx_connects_to_node(cxl)					\
		((cxl)->crx && ((cxl)->opp_node == 			\
		Node_of((cxl)->blk_cur,Opposite_orient((cxl)->blk_cur_orient))))

LOCAL	void triangulate_single_node_el(
	TRI_GRID	*ntg,
	TG_PT		*tg_node,
	BLK_CRX_LIST	*nd_crxlst,
	LINEAR_ELEMENT	**tri,
	int		*t_num)
{
	BLK_CRX_LIST	 *cxl1, *cxl2, *cxl11, *cxl22, *last;
	COMPONENT	 comp0, comp1, comp2;
	Locstate	 st0, st1, st2;
	TG_PT		 *p0, *p1, *p2;
	double		 dx1, dy1, dx2, dy2, dx11, dy11, dx22, dy22, ang1, ang2;
	LIN_EL_FACE_TYPE side0, side1, side2;

#if defined(DEBUG_TRI_GRID)
	debug_print("blk_with_nodes","Entered triangulate_single_node_el()\n");
#endif /* defined(DEBUG_TRI_GRID) */

	for (last = nd_crxlst; last && last->next; last = last->next);
	cxl1 = last;
	for (cxl2 = nd_crxlst; cxl2; cxl1 = cxl2, cxl2 = cxl2->next)
	{
	    if (cxl1->crx && cxl2->crx && cxl1->blk_cur == cxl2->blk_cur)
	    	side0 = F_SIDE;
	    else if (cxl1->side == cxl2->side)
	    	side0 = E_SIDE;
	    else
	    	side0 = I_SIDE;

	    if (crx_connects_to_node(cxl1))
	    {
	    	p0 = cxl1->crx->nd;
	    	side2 = F_SIDE;
	    	state_and_comp_at_crx_connected_to_node(cxl1,1,1,&st0,&comp0);
	    }
	    else if (cxl1->crx)
	    {
	    	p0 = cxl1->crx->nd;
	    	side2 = I_SIDE;
	    	if (((cxl1->blk_cur == cxl2->blk_cur) &&
	    	     (cxl1->blk_cur_orient == POSITIVE_ORIENTATION))
		    ||
		    ((cxl1->blk_cur != cxl2->blk_cur) &&
		     (cxl1->blk_cur_orient == NEGATIVE_ORIENTATION)))
	    	{
	    	    comp0 = negative_component(cxl1->blk_cur);
	    	    st0 = Left_state_at_node(cxl1->blk_cur,
	    			             cxl1->blk_cur_orient);
	    	}
	    	else
	    	{
	    	    comp0 = positive_component(cxl1->blk_cur);
	    	    st0 = Right_state_at_node(cxl1->blk_cur,
	    			              cxl1->blk_cur_orient);
	    	}
	    }
	    else if (cxl1->corner)
	    {
	    	p0 = cxl1->corner->nd;
	    	side2 = I_SIDE;
	    	comp0 = cxl1->corner->comp;
	    	st0 = cxl1->corner->state;
	    }
	    else
	    {
	        screen("ERROR in triangulate_single_node_el(), "
	               "Neither crx nor corner at crx list element\n");
	        clean_up(ERROR);
	    }

	    if (crx_connects_to_node(cxl2))
	    {
	    	p1 = cxl2->crx->nd;
	    	side1 = F_SIDE;
	    	state_and_comp_at_crx_connected_to_node(cxl2,2,2,&st1,&comp1);
	    }
	    else if (cxl2->crx)
	    {
	    	p1 = cxl2->crx->nd;
	    	side1 = I_SIDE;
	    	if (((cxl2->blk_cur == cxl1->blk_cur) &&
	    	     (cxl2->blk_cur_orient == NEGATIVE_ORIENTATION))
		    ||
		    ((cxl2->blk_cur != cxl1->blk_cur) &&
		     (cxl2->blk_cur_orient == POSITIVE_ORIENTATION)))
		{
		    comp1 = negative_component(cxl2->blk_cur);
		    st1 = Left_state_at_node(cxl2->blk_cur,
				             cxl2->blk_cur_orient);
		}
		else
		{
		    comp1 = positive_component(cxl2->blk_cur);
		    st1 = Right_state_at_node(cxl2->blk_cur,
				              cxl2->blk_cur_orient);
		}
	    }
	    else if (cxl2->corner)
	    {
	    	p1 = cxl2->corner->nd;
	    	side1 = I_SIDE;
	    	comp1 = cxl2->corner->comp;
	    	st1 = cxl2->corner->state;
	    }
	    else
	    {
	        screen("ERROR in triangulate_single_node_el(), "
	               "Neither crx nor corner at crx list element\n");
	        clean_up(ERROR);
	    }

	    p2 = tg_node;
	    if (crx_connects_to_node(cxl1) && crx_connects_to_node(cxl2))
	    {
		static int num_times = 1;
			
	    	if (num_times++ % 2)
	    	{
	    	    state_and_comp_at_crx_connected_to_node(cxl1,3,1,
							    &st2,&comp2);
	    	}
	    	else
	    	{
	    	    state_and_comp_at_crx_connected_to_node(cxl2,3,2,
							    &st2,&comp2);
	    	}
	    }
	    else if (crx_connects_to_node(cxl1))
	    {
	    	state_and_comp_at_crx_connected_to_node(cxl1,3,1,&st2,&comp2);
	    }
	    else if (crx_connects_to_node(cxl2))
	    {
	    	state_and_comp_at_crx_connected_to_node(cxl2,3,2,&st2,&comp2);
	    }
	    else
	    {
	    	find_nearest_crx_connected_to_node(cxl1,&cxl11,nd_crxlst,last,
						   CLOCKWISE);
		find_nearest_crx_connected_to_node(cxl2,&cxl22,nd_crxlst,last,
						   COUNTER_CLOCK);

		dx11 = Coords(cxl11->crx->nd)[0] - Coords(tg_node)[0];
		dy11 = Coords(cxl11->crx->nd)[1] - Coords(tg_node)[1];
		dx22 = Coords(cxl22->crx->nd)[0] - Coords(tg_node)[0];
		dy22 = Coords(cxl22->crx->nd)[1] - Coords(tg_node)[1];
		if (cxl1->crx)
		{
		    dx1 = Coords(cxl1->crx->nd)[0] - Coords(tg_node)[0];
		    dy1 = Coords(cxl1->crx->nd)[1] - Coords(tg_node)[1];
		}
		else
		{
		    dx1 = Coords(cxl1->corner->nd)[0] - Coords(tg_node)[0];
		    dy1 = Coords(cxl1->corner->nd)[1] - Coords(tg_node)[1];
		}
		if (cxl2->crx)
		{
		    dx2 = Coords(cxl2->crx->nd)[0] - Coords(tg_node)[0];
		    dy2 = Coords(cxl2->crx->nd)[1] - Coords(tg_node)[1];
		}
		else
		{
		    dx2 = Coords(cxl2->corner->nd)[0] - Coords(tg_node)[0];
		    dy2 = Coords(cxl2->corner->nd)[1] - Coords(tg_node)[1];
		}
		ang1 = angle(dx11*dx1+dy11*dy1,dx11*dy1-dy11*dx1);
		ang2 = angle(dx22*dx2+dy22*dy2,dx2*dy22-dy2*dx22);
		if (ang1 < ang2)
		{
		    state_and_comp_at_crx_connected_to_node(cxl11,3,1,&st2,
							    &comp2);
		}
		else
		{
		    state_and_comp_at_crx_connected_to_node(cxl22,3,2,&st2,
							    &comp2);
		}
	    }

		/* This comp test should be unnecessary. 
		   Used for debugging new code */

	    if (comp0 != comp1 || comp0 != comp2 || comp1 != comp2)
	    {
	        screen("ERROR in triangulate_single_node_el(), "
	               "Inconsistent components\n");
	        clean_up(ERROR);
	    }

	    set_tri(ntg,tri,t_num,p0,p1,p2,st0,st1,st2,side0,side1,side2,comp0);
	}
#if defined(DEBUG_TRI_GRID)
	debug_print("blk_with_nodes","Left triangulate_single_node_el()\n");
#endif /* defined(DEBUG_TRI_GRID) */
}		/*end triangulate_single_node_el*/

LOCAL void state_and_comp_at_crx_connected_to_node(
	BLK_CRX_LIST	*cxl,
	int		vertex,
	int		opp_vertex,
	Locstate	*st,
	COMPONENT	*comp)
{
	switch (vertex)
	{
	case 1:
	    if (cxl->blk_cur_orient == POSITIVE_ORIENTATION)
	    {
	    	*comp = positive_component(cxl->blk_cur);
	    	*st = right_start_state(cxl->blk_cur);
	    }
	    else
	    {
	    	*comp = negative_component(cxl->blk_cur);
	    	*st = left_end_state(cxl->blk_cur);
	    }
	    return;

	case 2:
	    if (cxl->blk_cur_orient == POSITIVE_ORIENTATION)
	    {
	    	*comp = negative_component(cxl->blk_cur);
	    	*st = left_start_state(cxl->blk_cur);
	    }
	    else
	    {
	    	*comp = positive_component(cxl->blk_cur);
	    	*st = right_end_state(cxl->blk_cur);
	    }
	    return;

	case 3:
	    if (opp_vertex == 1)
	    {
	    	if (cxl->blk_cur_orient == POSITIVE_ORIENTATION)
	    	{
	    	    *comp = positive_component(cxl->blk_cur);
	    	    *st = right_end_state(cxl->blk_cur);
	    	}
	    	else
	    	{
	    	    *comp = negative_component(cxl->blk_cur);
	    	    *st = left_start_state(cxl->blk_cur);
	    	}
	    }
	    else
	    {
	    	if (cxl->blk_cur_orient ==POSITIVE_ORIENTATION)
	    	{
	    	    *comp = negative_component(cxl->blk_cur);
	    	    *st = left_end_state(cxl->blk_cur);
	    	}
	    	else
	    	{
	    	    *comp = positive_component(cxl->blk_cur);
	    	    *st = right_start_state(cxl->blk_cur);
	    	}
	    }
	    return;
	default:
	    screen("ERROR in set_state_and_comp_at_crx_connected_to_node(), "
	           "No such vertex number\n");
	    clean_up(ERROR);
	}
}		/*end state_and_comp_at_crx_connected_to_node*/

LOCAL void find_nearest_crx_connected_to_node(
	BLK_CRX_LIST	*cxl,
	BLK_CRX_LIST	**ans,
	BLK_CRX_LIST	*first,
	BLK_CRX_LIST	*last,
	ANGLE_DIRECTION	ang_dir)
{
	if (ang_dir == CLOCKWISE)
	{
	    for (*ans = cxl->prev; *ans; *ans = (*ans)->prev)
	    	if (crx_connects_to_node(*ans))
		    break;
	    if (*ans == NULL)
	    {
	    	for (*ans = last; *ans != cxl; *ans = (*ans)->prev)
	    	    if (crx_connects_to_node(*ans))
			break;
	    }
	}
	else
	{
	    for (*ans = cxl->next; *ans; *ans = (*ans)->next)
	    	if (crx_connects_to_node(*ans))
		    break;
	    if (*ans == NULL)
	    {
	    	for (*ans = first; *ans != cxl; *ans = (*ans)->next)
	    	    if (crx_connects_to_node(*ans))
			break;
	    }
	}
	if (*ans == NULL || !crx_connects_to_node(*ans))
	{
	    screen("ERROR in find_nearest_crx_connected_to_node(), "
	           "Unable to find nearest crx\n");
	    clean_up(ERROR);
	}
}		/*end find_nearest_crx_connected_to_node*/


LOCAL	void make_blk_crx_lists(
	TRI_GRID	*ntg,
	int		ncs,
	int		*s_list,
	int		jse,
	int		nce,
	int		*e_list,
	int		jne,
	int		ncn,
	int		*n_list,
	int		jnw,
	int		ncw,
	int		*w_list,
	int		jsw,
	int		num_nodes,
	NODE		**node,
	INTERFACE	*blk_intfc,
	BOND		**blk_bonds,
	BLK_CRX_LIST	**nd_crxlst,
	BLK_CRX_LIST	**sw_crxlst,
	BLK_CRX_LIST	**se_crxlst,
	BLK_CRX_LIST	**ne_crxlst,
	BLK_CRX_LIST	**nw_crxlst)
{
	BLK_CRX_LIST	Crxlst, *crxl, *crxl1, *last;
	BLK_CRX_LIST	*sw_cor, *se_cor, *ne_cor, *nw_cor;
	NODE		**blk_node;
	int		i, dim = ntg->grid_intfc->dim;

#if defined(DEBUG_TRI_GRID)
	debug_print("blk_with_nodes","Entered make_blk_crx_lists()\n");
#endif /* defined(DEBUG_TRI_GRID) */


	/* Make list of interior nodes on blk_intfc */

	blk_node = (NODE **) Store(num_nodes*sizeof(NODE *));
	for (i = 0; i < num_nodes; ++i)
	    blk_node[i] = node_of_point(node[i]->posn,blk_intfc);

	/* Make basic crx list */

	crxl = &Crxlst;
	Crxlst.next = Crxlst.prev = NULL;
	add_corner_to_blk_crx_list(&crxl,jsw,SOUTH_WEST,ntg);
	sw_cor = crxl;
	for (i = 0; i < ncs; ++i)
	{
	    add_crx_to_blk_crx_list(ntg,s_list+i,&crxl,SOUTH,blk_intfc,
				    blk_node,num_nodes);
	}
	add_corner_to_blk_crx_list(&crxl,jse,SOUTH_EAST,ntg);
	se_cor = crxl;
	for (i = 0; i < nce; ++i)
	{
	    add_crx_to_blk_crx_list(ntg,e_list+i,&crxl,EAST,blk_intfc,
				    blk_node,num_nodes);
	}
	add_corner_to_blk_crx_list(&crxl,jne,NORTH_EAST,ntg);
	ne_cor = crxl;
	for (i = 0; i < ncn; ++i)
	{
	    add_crx_to_blk_crx_list(ntg,n_list+i,&crxl,NORTH,blk_intfc,
				    blk_node,num_nodes);
	}
	add_corner_to_blk_crx_list(&crxl,jnw,NORTH_WEST,ntg);
	nw_cor = crxl;
	for (i = 0; i < ncw; ++i)
	{
	    add_crx_to_blk_crx_list(ntg,w_list+i,&crxl,WEST,blk_intfc,
				    blk_node,num_nodes);
	}

	/* Check that some curve from the node reaches the grid boundary */

	for (crxl = Crxlst.next; crxl; crxl = crxl->next)
	    if (crxl->opp_node)
		break;
	if (crxl == NULL) /* Node's curves don't reach grid boundary */
	{
	    *nd_crxlst = NULL;
	    *sw_crxlst = NULL;	*se_crxlst = NULL;
	    *ne_crxlst = NULL;	*nw_crxlst = NULL;

#if defined(DEBUG_TRI_GRID)
	    if (debugging("blk_with_nodes"))
	    {
	    	(void) printf("Block cross lists produced by "
	    	              "make_blk_crx_lists()\n");
	    	print_blk_crx_list(*nd_crxlst,"NODE CROSS LIST");
	    	print_blk_crx_list(*sw_crxlst,"SW CROSS LIST");
	    	print_blk_crx_list(*se_crxlst,"SE CROSS LIST");
	    	print_blk_crx_list(*ne_crxlst,"NE CROSS LIST");
	    	print_blk_crx_list(*nw_crxlst,"NW CROSS LIST");
	    	(void) printf("Left make_blk_crx_lists()\n");
	    }
#endif /* defined(DEBUG_TRI_GRID) */
	    return;
	}


	/* Find opposite crosses */

	for (crxl = Crxlst.next; crxl; crxl = crxl->next)
	{
	    if (crxl->corner || crxl->opp_node || crxl->opp_crxl)
		continue;
	    for (crxl1 = crxl->next; crxl1; crxl1 = crxl1->next)
	    {
	        if (crxl1->corner || crxl1->opp_node || crxl1->opp_crxl)
	    	    continue;
	        if (crxl->blk_cur == crxl1->blk_cur)
	        {
	            crxl->opp_crxl = crxl1;
	            crxl1->opp_crxl = crxl;
	            break;
	        }
	    }
	}

	/* Delete pairs of crosses on the same grid side */

	for (crxl = Crxlst.next; crxl; crxl = crxl->next)
	{
	    if (crxl->corner || crxl->opp_node)
		continue;
	    if (crxl->side != crxl->opp_crxl->side)
		continue;
	    crxl->prev->next = crxl->next;
	    if (crxl->next)
	    	crxl->next->prev = crxl->prev;
	}

	/* If no corners are cut off set star pattern and return */

	for (last = Crxlst.next; last && last->next; last = last->next);
	for (crxl = Crxlst.next; crxl; crxl = crxl->next)
	    if (crxl->opp_crxl)
		break;

	if (crxl == NULL) /* All crosses connect to node */
	{
	    for (crxl = Crxlst.next; crxl; crxl = crxl->next)
	    {
	        if (crxl->corner)
	    	crxl->opp_node = blk_node[0];
	    }
	    *nd_crxlst = Crxlst.next;

	    if (*nd_crxlst)
		(*nd_crxlst)->prev = NULL;
	    *sw_crxlst = NULL;
	    *se_crxlst = NULL;
	    *ne_crxlst = NULL;
	    *nw_crxlst = NULL;
#if defined(DEBUG_TRI_GRID)
	    if (debugging("blk_with_nodes"))
	    {
	    	(void) printf("Block cross lists produced by ");
	    	(void) printf("make_blk_crx_lists()\n");
	    	print_blk_crx_list(*nd_crxlst,"NODE CROSS LIST");
	    	print_blk_crx_list(*sw_crxlst,"SW CROSS LIST");
	    	print_blk_crx_list(*se_crxlst,"SE CROSS LIST");
	    	print_blk_crx_list(*ne_crxlst,"NE CROSS LIST");
	    	print_blk_crx_list(*nw_crxlst,"NW CROSS LIST");
	    	(void) printf("Left make_blk_crx_lists()\n");
	    }
#endif /* defined(DEBUG_TRI_GRID) */
	    return;
	}

	/*
	*  The case that follows is relatively rare, but must be
	*  handled correctly when it occurs.  In this case
	*  some of the crosses on the grid boundary connect to the
	*  interior node, while some cut off sections of the grid.
	*  Find all crosses that can be connected to the interior node
	*  without producing tangles with the blk_bonds
	*/

	for (crxl = Crxlst.next; crxl; crxl = crxl->next)
	{
	    BOND Btmp;
	    FT_POINT Ptmp, Pc;
	    FT_POINT *nd_posn = blk_node[0]->posn;
	    BOND **bb;

	    if (crxl->opp_node)
		continue;
	    if (crxl->crx)
	    {
	    	Btmp.start = crxl->crx->pt;
	    	Btmp.end = nd_posn;
	    }
	    else if (crxl->corner)
	    {
	    	COORDS(Ptmp)[0] = Coords(crxl->corner->nd)[0];
	    	COORDS(Ptmp)[1] = Coords(crxl->corner->nd)[1];
	    	Btmp.start = &Ptmp;
	    	Btmp.end = nd_posn;
	    }
	    else
	    {	/* For debugging */
	        screen("ERROR in make_blk_crx_lists(), "
	               "Cross list element is not cross or corner\n");
	        clean_up(ERROR);
	    }
	    set_bond_length(&Btmp,dim);
	    for (bb = blk_bonds; bb && *bb; ++bb)
	    {
	    	if (((*bb)->start == Btmp.start) ||
	    	    ((*bb)->start == Btmp.end) ||
		    ((*bb)->end == Btmp.start) ||
		    ((*bb)->end == Btmp.end))
		    continue;
		if (cross_bonds(*bb,&Btmp,&Pc))
		    break;
	    }
	    if (!*bb) /* No intersections */
	    {
	    	crxl->opp_node = blk_node[0];
	    }
	}

	/* Saw off corner elements */

	if (!sw_cor->opp_node)
	    saw_off_blk_corner(sw_cor,&Crxlst,sw_crxlst);
	else
	    *sw_crxlst = NULL;

	if ((!se_cor->opp_node) && 
	    (crxl_in_blk_crx_list(se_cor,*sw_crxlst) == NO))
	{
	    saw_off_blk_corner(se_cor,&Crxlst,se_crxlst);
	}
	else
	    *se_crxlst = NULL;

	if ((!ne_cor->opp_node) && 
	    (crxl_in_blk_crx_list(ne_cor,*sw_crxlst) == NO) &&
	    (crxl_in_blk_crx_list(ne_cor,*se_crxlst) == NO))
	{
	    saw_off_blk_corner(ne_cor,&Crxlst,ne_crxlst);
	}
	else
	    *ne_crxlst = NULL;

	if ((!nw_cor->opp_node) && 
		(crxl_in_blk_crx_list(nw_cor,*sw_crxlst) == NO) &&
		(crxl_in_blk_crx_list(nw_cor,*se_crxlst) == NO) &&
		(crxl_in_blk_crx_list(nw_cor,*ne_crxlst) == NO))
	{
	    saw_off_blk_corner(nw_cor,&Crxlst,nw_crxlst);
	}
	else
	    *nw_crxlst = NULL;

	*nd_crxlst = Crxlst.next;
	if (*nd_crxlst)
	    (*nd_crxlst)->prev = NULL;

#if defined(DEBUG_TRI_GRID)
	if (debugging("blk_with_nodes"))
	{
	    (void) printf("Block cross lists produced by ");
	    (void) printf("make_blk_crx_lists()\n");
	    print_blk_crx_list(*nd_crxlst,"NODE CROSS LIST");
	    print_blk_crx_list(*sw_crxlst,"SW CROSS LIST");
	    print_blk_crx_list(*se_crxlst,"SE CROSS LIST");
	    print_blk_crx_list(*ne_crxlst,"NE CROSS LIST");
	    print_blk_crx_list(*nw_crxlst,"NW CROSS LIST");
	    (void) printf("Left make_blk_crx_lists()\n");
	}
#endif /* defined(DEBUG_TRI_GRID) */
}		/*end make_blk_crx_lists*/

LOCAL void saw_off_blk_corner(
	BLK_CRX_LIST	*cor,
	BLK_CRX_LIST	*crxl_head,
	BLK_CRX_LIST	**cor_crxlst)
{
	BLK_CRX_LIST	*crxl0, *crxl1, *cxl0, *cxl1, *last;

	for (crxl0 = cor; crxl0; crxl0 = crxl0->next)
	    if (crxl0->opp_node)
		break;
	if (!crxl0 || !crxl0->opp_crxl)
	{
	    screen("ERROR in saw_off_blk_corner(), "
	           "Unexpected case\n");
	}
	crxl1 = crxl0->opp_crxl;
	cxl0 = (BLK_CRX_LIST *)Store(sizeof(BLK_CRX_LIST));
	cxl1 = (BLK_CRX_LIST *)Store(sizeof(BLK_CRX_LIST));
	cxl0->crx = crxl0->crx;
	cxl0->side = crxl0->side;
	cxl0->crx_index = crxl0->crx_index;
	cxl0->opp_node = NULL;
	cxl0->opp_crxl = cxl1;
	cxl0->blk_cur = crxl0->blk_cur;
	cxl0->blk_cur_orient = crxl0->blk_cur_orient;
	cxl0->corner = crxl0->corner;
	cxl0->next = cxl1;
	cxl0->prev = crxl0->prev;

	cxl1->crx = crxl1->crx;
	cxl1->side = crxl1->side;
	cxl1->crx_index = crxl1->crx_index;
	cxl1->opp_node = NULL;
	cxl1->opp_crxl = cxl0;
	cxl1->blk_cur = crxl1->blk_cur;
	cxl1->blk_cur_orient = crxl1->blk_cur_orient;
	cxl1->corner = crxl1->corner;
	cxl1->prev = cxl0;
	cxl1->next = crxl1->next;

	if (crxl0->prev)
	    crxl0->prev->next = cxl0;
	if (crxl1->next)
	    crxl1->next->prev = cxl1;
	if (cor == crxl_head->next)
	{
	    cor->prev = NULL;
	    crxl0->prev = crxl_head;
	    crxl1->next = NULL;
	    crxl_head->next = crxl0;
	}
	else
	{
	    crxl0->prev = crxl1;
	    crxl1->next = crxl0;
	}
	if (cor->prev)
	    cor->prev->next = NULL;
	cor->prev = NULL;

	/* Reset cross list for correct rotation */

	for (last = cxl1; last; last = last->next)
	    if (!last->next)
		break;
	
	cor->prev = last;
	last->next = cor;
	cxl0->next = NULL;
	cxl1->prev = NULL;
	*cor_crxlst = cxl1;
}		/*end saw_off_blk_corner*/

LOCAL boolean crxl_in_blk_crx_list(
	BLK_CRX_LIST	*crxl,
	BLK_CRX_LIST	*head)
{
	BLK_CRX_LIST	*cxl;

	for (cxl = head; cxl; cxl = cxl->next)
	    if (cxl == crxl)
		return YES;
	return NO;
}		/*end crxl_in_blk_crx_list*/

LOCAL void add_corner_to_blk_crx_list(
	BLK_CRX_LIST	**crxlst,
	int		index,
	GRID_CORNER	sides,
	TRI_GRID	*ntg)
{
	BLK_CRX_LIST	*cxl;
	Table		*T = table_of_interface(ntg->grid_intfc);

	cxl = (BLK_CRX_LIST *)Store(sizeof(BLK_CRX_LIST));
	cxl->next = (*crxlst)->next;
	cxl->prev = *crxlst;
	if ((*crxlst)->next)
	    (*crxlst)->next->prev = cxl;
	(*crxlst)->next = cxl;
	*crxlst = cxl;
	cxl->crx = NULL;
	cxl->opp_crxl = NULL;
	cxl->opp_node = NULL;
	cxl->blk_cur = NULL;
	cxl->corner = (BLK_CORNER *)Store(sizeof(BLK_CORNER));
	cxl->corner->sides = sides;
	cxl->corner->index = index;
	cxl->corner->nd = &(ntg->node_points[index]);
	cxl->corner->state = ntg->states[index];
	cxl->corner->comp = T->components[index];
}		/*end add_corner_to_blk_crx_list*/

LOCAL void add_crx_to_blk_crx_list(
	TRI_GRID	*ntg,
	int		*crx_index,
	BLK_CRX_LIST	**crxlst,
	GRID_DIRECTION	side,
	INTERFACE	*blk_intfc,
	NODE		**blk_node,
	int		num_nodes)
{
	Table		*T = table_of_interface(ntg->grid_intfc);
	BLK_CRX_LIST	*cxl;
	NODE		*n, *oppn;
	int		i;

	cxl = (BLK_CRX_LIST *)Store(sizeof(BLK_CRX_LIST));
	cxl->next = (*crxlst)->next;
	cxl->prev = *crxlst;
	if ((*crxlst)->next)
	    (*crxlst)->next->prev = cxl;
	(*crxlst)->next = cxl;
	*crxlst = cxl;
	cxl->crx = &T->crx_store[*crx_index];
	cxl->side = side;
	cxl->crx_index = crx_index;
	cxl->opp_crxl = NULL;
	cxl->opp_node = NULL;
	cxl->corner = NULL;


	/* Find opposite node if it exists */

	if ((n = node_of_point(cxl->crx->pt,blk_intfc)) == NULL)
	{
	    screen("ERROR in add_crx_to_blk_crx_list(), The point of "
		   "crx (%g %g) is not on blk_intfc\n",
	           Coords(cxl->crx->pt)[0],Coords(cxl->crx->pt)[1]);
	    clean_up(ERROR);
	}
	if (n->in_curves != NULL && n->in_curves[0] != NULL)
	{
		oppn = (n->in_curves[0])->start;
		cxl->blk_cur = n->in_curves[0];
		cxl->blk_cur_orient = NEGATIVE_ORIENTATION;
	}
	else if (n->out_curves != NULL && n->out_curves[0] != NULL)
	{
		oppn = (n->out_curves[0])->end;
		cxl->blk_cur = n->out_curves[0];
		cxl->blk_cur_orient = POSITIVE_ORIENTATION;
	}
	else
	{
	    screen("ERROR in add_crx_to_blk_crx_list(), "
	           "The node of crx point (%g, %g) "
	           "has no curve on blk_intfc\n",
	           Coords(n->posn)[0],Coords(n->posn)[1]);
	    clean_up(ERROR);
	}
	for (i = 0; i < num_nodes; ++i)
	{
	    if (oppn == blk_node[i])
	    {
	    	cxl->opp_node = oppn;
	    	break;
	    }
	}
}		/*end add_crx_to_blk_crx_list*/


/*
*			is_proper_elN1():
*
*	Given a local block interface blk_intfc, this function
*	tests for intersections on the interface formed by
*	replacing all of the curves on blk_intfc by curves
*	with single bonds.  Returns YES if the resulting interface
*	is nonintersecting, NO otherwise.
*
*	NOTE: This function does not modify blk_intfc, so
*	this data structure may safely be used in a full
*	resolution triangulation algorithm.
*/


LOCAL int is_proper_elN1(
	INTERFACE	*blk_intfc,
	BOND		***blk_bonds)
{
	BOND		*b, **b1, **b2;
	CURVE		**c;
	FT_POINT		Pc;

#if defined(DEBUG_TRI_GRID)

	debug_print("blk_with_nodes","Entered is_proper_elN1()\n");
#endif /* defined(DEBUG_TRI_GRID) */

	*blk_bonds = NULL;
	for (c = blk_intfc->curves; c && *c; ++c)
	{
	    b = Bond((*c)->start->posn,(*c)->end->posn);
	    if (add_to_pointers(b,blk_bonds) !=
		FUNCTION_SUCCEEDED)
	    {
	        screen("ERROR in is_proper_elN1(), add_to_pointers() failed\n");
	        clean_up(ERROR);
	    }
	}
	for (b1 = *blk_bonds; b1 && *b1; ++b1)
	{
	    for (b2 = b1+1; b2 && *b2; ++b2)
	    {
		if ((*b1)->start == (*b2)->start)
		    continue;
		if ((*b1)->start == (*b2)->end )
		    continue;
		if ((*b1)->end   == (*b2)->start)
		    continue;
		if ((*b1)->end   == (*b2)->end )
		    continue;
		if (cross_bonds(*b1,*b2,&Pc)) 
		{
#if defined(DEBUG_TRI_GRID)
			debug_print("blk_with_nodes",
			      "Leaving is_proper_elN1() return NO\n");
#endif /* defined(DEBUG_TRI_GRID) */
		    return NO;
		}
	    }
	}
#if defined(DEBUG_TRI_GRID)
	debug_print("blk_with_nodes","Leaving is_proper_elN1() return YES\n");
#endif /* defined(DEBUG_TRI_GRID) */

	return YES;
}		/*end is_proper_elN1*/

LIB_LOCAL	INTERFACE	*make_blk_intfc(
	TRI_GRID	*ntg,
	int		ncs,
	int		*s_list,
	int		jse,
	int		nce,
	int		*e_list,
	int		jne,
	int		ncn,
	int		*n_list,
	int		jnw,
	int		ncw,
	int		*w_list,
	int		jsw,
	int		num_nodes,
	NODE		**node)
{
	Table		*T = table_of_interface(ntg->grid_intfc);
	INTERFACE	*blk_intfc;
	CRXING		**cx, **crxings[4];
	RECT_GRID	*gr, *ntg_gr = &ntg->rect_grid;
	NODE		*newn;
	CURVE		**c, **int_blk_curs;
	BOND		*b;
	ORIENTATION	orient;
	int		i;
	int		dim = ntg_gr->dim;
	static BOUNDARY_STATE *BstateList[7];

#if defined(DEBUG_TRI_GRID)
	debug_print("make_blk_intfc","Entered make_blk_intfc()\n");
#endif /* defined(DEBUG_TRI_GRID) */

	blk_intfc = make_interface(2);
	bstate_list(blk_intfc) = BstateList+1;
	gr = &topological_grid(blk_intfc);
	gr->L[0] = Coords(ntg->node_points+jsw)[0];
	gr->L[1] = Coords(ntg->node_points+jse)[1];
	gr->U[0] = Coords(ntg->node_points+jne)[0];
	gr->U[1] = Coords(ntg->node_points+jnw)[1];
	gr->gmax[0] = 1;
	gr->gmax[1] = 1;
	set_rect_grid(gr->L,gr->U,gr->L,gr->U,NOBUF,NOBUF,gr->gmax,dim,
		      &ntg_gr->Remap,gr);
	set_computational_grid(blk_intfc,gr);


			/* Set Crossings lists */

	int_blk_curs = NULL;
	for (i = 0; i < 4; ++i)
	    crxings[i] = NULL;
	for (i = 0; i < ncs; ++i)
	{
	    if (add_to_pointers((&T->crx_store[*(s_list+i)]),
				&crxings[0]) != FUNCTION_SUCCEEDED)
	    {
		screen("ERROR in make_blk_intfc(),  add_to_pointers failed "
		       "on south crossing %d\n",i);
		clean_up(ERROR);
	    }
	}
	for (i = 0; i < nce; ++i)
	{
	    if (add_to_pointers((&T->crx_store[*(e_list+i)]),
				&crxings[1]) != FUNCTION_SUCCEEDED)
	    {
		screen("ERROR in make_blk_intfc(),  add_to_pointers failed "
		       "on east crossing %d\n",i);
		clean_up(ERROR);
	    }
	}
	for (i = 0; i < ncn; ++i)
	{
	    if (add_to_pointers((&T->crx_store[*(n_list+i)]),
				&crxings[2]) != FUNCTION_SUCCEEDED)
	    {
		screen("ERROR in make_blk_intfc(),  add_to_pointers failed "
		       "on north crossing %d\n",i);
		clean_up(ERROR);
	    }
	
	}
	for (i = 0; i < ncw; ++i)
	{
	    if (add_to_pointers((&T->crx_store[*(w_list+i)]),
				&crxings[3]) != FUNCTION_SUCCEEDED)
	    {
		screen("ERROR in make_blk_intfc(),  add_to_pointers failed "
		       "on west crossing %d\n",i);
		clean_up(ERROR);
	    }
	}

#if defined(DEBUG_TRI_GRID)
	if (node != NULL && debugging("make_blk_intfc"))
	{
	    (void) printf("\nTHESE ARE THE NODES IN THE BLK_INTFC\n");
	    for (i = 0; i < num_nodes; ++i)
	    {
		print_general_vector("n->posn = ",Coords(node[i]->posn),
				     dim,"\n");
	    }
	    for (i = 0; i < num_nodes; ++i)
	    {
		print_general_vector("in_curves at node at ",
				     Coords(node[i]->posn),dim,"\n");
	        for (c = node[i]->in_curves;  c && *c; ++c)
	    	    print_curve(*c);
		print_general_vector("out_curves at node at ",
				     Coords(node[i]->posn),dim,"\n");
	        for (c = node[i]->out_curves; c && *c; ++c)
	    	    print_curve(*c);
	    }
	}
#endif /* defined(DEBUG_TRI_GRID) */

	/* Copy nodes and curves from nodes into blk_intfc */

	for (i = 0; i < num_nodes; ++i)
	{
	    if ((newn = node_of_point(node[i]->posn,blk_intfc)) == NULL)
	    {
	    	newn = make_node(node[i]->posn);

#if defined(DEBUG_TRI_GRID)
	    	if (debugging("make_blk_intfc"))
		    print_general_vector("Node at ",Coords(newn->posn),dim,
					 " is NOT in blk_intfc\n");
#endif /* defined(DEBUG_TRI_GRID) */

	    }

#if defined(DEBUG_TRI_GRID)
	    else if (debugging("make_blk_intfc"))
	    {
	    	if (debugging("make_blk_intfc"))
		    print_general_vector("Node at ",Coords(newn->posn),dim,
					 " IS in blk_intfc\n");
	    }
#endif /* defined(DEBUG_TRI_GRID) */

	    for (c = node[i]->in_curves; c && *c; ++c)
	    {
	    	insert_cur_seg(newn,(*c)->last,*c,NEGATIVE_ORIENTATION,
	    		       crxings,&int_blk_curs,blk_intfc);
	    }
	    for (c = node[i]->out_curves; c && *c; ++c)
	    {
	    	insert_cur_seg(newn,(*c)->first,*c,POSITIVE_ORIENTATION,
	    		       crxings,&int_blk_curs,blk_intfc);
	    }
	}

	/* Identify bonds from crossing to crossings */

	for (i = 0; i < 4; ++i)
	{
	    static int side[] = {SOUTH, EAST, NORTH, WEST};
	    while (crxings[i] != NULL)
	    {
		cx = crxings[i];
		orient = find_bond_and_orient_at_crx(*cx,side[i],&b);
		newn = make_node((*cx)->pt);
		set_is_bdry(newn);
		insert_cur_seg(newn,b,Curve_of_hs((*cx)->hs),orient,crxings,
			       &int_blk_curs,blk_intfc);
		if (delete_from_pointers(*cx,&crxings[i]) != FUNCTION_SUCCEEDED)
		{
		    screen("ERROR in make_blk_intfc(), "
		           "delete_from_pointers() failed\n");
		    clean_up(ERROR);
		}
	    }
	}

#if defined(DEBUG_TRI_GRID)
	if (debugging("make_blk_intfc"))
	{
	    (void) printf("Block interface\n");
	    print_interface(blk_intfc);
	}
	debug_print("make_blk_intfc","Left make_blk_intfc()\n");
#endif /* defined(DEBUG_TRI_GRID) */
	return blk_intfc;
}		/*end make_blk_intfc*/



LOCAL	void insert_cur_seg(
	NODE		*node_in,
	BOND		*b_in,
	CURVE		*c,
	ORIENTATION	orient,
	CRXING		***crxings,
	CURVE		***int_blk_curs,
	INTERFACE	*blk_intfc)
{
	BOND		*b_out;
	NODE		*node_out, *opp_node;
	FT_POINT		*pob;
	CRXING		*crx;
	int		side;
	ORIENTATION	opp_or = Opposite_orient(orient);

#if defined(DEBUG_TRI_GRID)
	debug_print("make_blk_intfc","Entered insert_cur_seg()\n");
#endif /* defined(DEBUG_TRI_GRID) */

	for (b_out = b_in; b_out != NULL; b_out = Following_bond(b_out,orient))
	{
	    for (side = 0; side < 4; ++side)
	    {
		if (!crxings[side])
		    continue;
		pob = Point_of_bond(b_out,opp_or);
		if (point_in_crx_list(pob,b_out,crxings[side],&crx))
		{
		    node_out = make_node(crx->pt);
		    set_is_bdry(node_out);
		    if (delete_from_pointers(crx,&crxings[side]) !=
			FUNCTION_SUCCEEDED)
		    {
		    	screen("ERROR in insert_cur_seg(), "
		    	       "delete_from_pointers() failed\n");
		    	clean_up(ERROR);
		    }
		    install_blk_curve(b_in,b_out,node_in,node_out,c,orient,
				      blk_intfc);
#if defined(DEBUG_TRI_GRID)
		    debug_print("make_blk_intfc","Left insert_cur_seg()\n");
#endif /* defined(DEBUG_TRI_GRID) */
		    return;
		}
	    }
	}

		/* Interior block curve found */

 		/* Check if already done */
	if (curve_in_curve_list(c,*int_blk_curs))
	    return;

	if (add_to_pointers(c,int_blk_curs) != FUNCTION_SUCCEEDED)
	{
	    screen("ERROR in insert_cur_seg(), add_to_pointers() failed\n");
	    clean_up(ERROR);
	}
	opp_node = Node_of(c,opp_or);

	node_out = node_of_point(opp_node->posn,blk_intfc);
	if (!node_out)
	    node_out = make_node(Node_of(c,opp_or)->posn);
	b_out = Bond_at_node(c,opp_or);

	install_blk_curve(b_in,b_out,node_in,node_out,c,orient,blk_intfc);

#if defined(DEBUG_TRI_GRID)
	debug_print("make_blk_intfc","Left insert_cur_seg()\n");
#endif /* defined(DEBUG_TRI_GRID) */
}		/*end insert_cur_seg*/

LOCAL	void	install_blk_curve(
	BOND		*b_in,
	BOND		*b_out,
	NODE		*node_in,
	NODE		*node_out,
	CURVE		*c,
	ORIENTATION	orient,
	INTERFACE	*blk_intfc)
{
	BOND		*b_first, *b_last, *b, *bnew;
	CURVE		*newc;
	FT_POINT		*ps, *pe;
	RECT_GRID	*gr = computational_grid(blk_intfc);
	double		tol = 0.5*IG_NTOL*(gr->h[0] + gr->h[1]);/*TOLERANCE*/

	if (orient == POSITIVE_ORIENTATION)
	{
	    b_first = b_in;
	    b_last  = b_out;
	    newc = make_curve(negative_component(c),positive_component(c),
			      node_in,node_out);
	}
	else
	{
	    b_first = b_out;
	    b_last  = b_in;
	    newc = make_curve(negative_component(c),positive_component(c),
			      node_out,node_in);
	}

	ps = b_first->start;
	pe = b_last->end;

	left_start_state(newc)  =  left_state_at_point_on_curve(ps,b_first,c);
	right_start_state(newc) = right_state_at_point_on_curve(ps,b_first,c);
	left_end_state(newc)    =  left_state_at_point_on_curve(pe,b_last,c);
	right_end_state(newc)   = right_state_at_point_on_curve(pe,b_last,c);

	if ((b_first != b_last) && (bond_length(b_first) < tol))
	    b_first = b_first->next;
	if ((b_first != b_last) && (bond_length(b_last) < tol))
	    b_last = b_last->prev;
	for (b = b_first, bnew = newc->first; b != b_last; b = b->next)
	{
	    if (bond_length(b) > tol)
	    {
	    	if (insert_point_in_bond(b->end,bnew,newc)!=FUNCTION_SUCCEEDED)
		{
		    screen("ERROR in install_blk_curve(), insert_point_in_bond "
			   "failed\n");
		    print_general_vector("b->end = ",Coords(b->end),
					 blk_intfc->dim,"\n");
		    print_bond(b);
		    clean_up(ERROR);
		}
	    	bnew = bnew->next;
	    }
	}
}		/*end install_blk_curve*/


LOCAL int point_in_crx_list(
	FT_POINT		*p,
	BOND		*b,
	CRXING		**crxings,
	CRXING		**crx)
{
	CRXING		**cx;

	for (cx = crxings; cx && *cx; ++cx)
	{
	    if ((*cx)->pt != p)
	        continue;
	    switch((*cx)->crossing_direction)
	    {
	    case BELOW_TO_ABOVE:
	    	if (Coords(b->end)[1] < Coords(b->start)[1])
		    continue;
	    	break;
	    case RIGHT_TO_LEFT:
	    	if (Coords(b->end)[0] > Coords(b->start)[0])
		    continue;
	    	break;
	    case ABOVE_TO_BELOW:
	    	if (Coords(b->end)[1] > Coords(b->start)[1])
		    continue;
	    	break;
	    case LEFT_TO_RIGHT:
	    	if (Coords(b->end)[0] < Coords(b->start)[0])
		    continue;
	    	break;
	    }
	    *crx = *cx;
	    return YES;
	}
	return NO;
}		/*end point_in_crx_list*/

LOCAL	ORIENTATION find_bond_and_orient_at_crx(
	CRXING		*crx,
	int		side,
	BOND		**b)
{
	ORIENTATION	   orient;
	CROSSING_DIRECTION direction = crx->crossing_direction;
	
	switch (side)
	{
	case SOUTH:
	    orient = (direction == BELOW_TO_ABOVE) ? POSITIVE_ORIENTATION :
						     NEGATIVE_ORIENTATION;
	    break;
	case EAST:
	    orient = (direction == RIGHT_TO_LEFT) ? POSITIVE_ORIENTATION :
						    NEGATIVE_ORIENTATION;
	    break;
	case NORTH:
	    orient = (direction == ABOVE_TO_BELOW) ? POSITIVE_ORIENTATION :
						     NEGATIVE_ORIENTATION;
	    break;
	case WEST:
	    orient = (direction == LEFT_TO_RIGHT) ? POSITIVE_ORIENTATION :
						    NEGATIVE_ORIENTATION;
	    break;
	default:
	    screen("ERROR in find_bond_and_orient_at_crx(), unknown side\n");
	    clean_up(ERROR);
	}
	for (*b = Bond_at_node(Curve_of_hs(crx->hs),orient); *b != NULL;
	     *b = Following_bond(*b,orient))
	{
	    if (crx->pt == Point_of_bond(*b,orient))
	    	break;
	}
	return orient;
}		/*end find_bond_and_orient_at_crx*/


LIB_LOCAL	void free_ntg_blk_node_lists(
	TRI_GRID	*ntg,
	int		*gmax)
{
	BLK_EL1		*blk_el1 = ntg->blk_els1;
	int		ix, xmax = gmax[0], iy, ymax = gmax[1];

	blk_el1 = ntg->blk_els1;
	for (iy = 0;  iy < ymax;  ++iy)
	{
	    for (ix = 0;  ix < xmax;  ++ix, ++blk_el1)
	    {
	    	if (blk_el1->interior_node == 0)
	    	    continue;
	    	else
	    	    free_blk_node_list(blk_el1);
	    }
	}
	Set_free(ntg,blk_els1); ntg->blk_els1 = ntg->cg_blk_els1 = NULL;
}		/*end free_ntg_blk_node_lists*/

LOCAL	void free_blk_node_list(
	BLK_EL1	*blk_el1)
{
	if (blk_el1->blk_node == NULL)
	    return;
	free_next_blk_node(blk_el1->blk_node);
	blk_el1->blk_node = NULL;
}		/*end free_blk_node_list*/

LOCAL	void free_next_blk_node(
	BLK_NODE *blk_node)
{
	if (blk_node->next != NULL)
	    free_next_blk_node(blk_node->next);
	free(blk_node);
}		/*end free_next_blk_node*/

#if defined(DEBUG_TRI_GRID)
LOCAL	void print_blk_crx_list(
	BLK_CRX_LIST	*crx_lst,
	const char	*message)
{
	BLK_CRX_LIST	*cxl;

	(void) printf("\nSTART OF %s BLOCK CROSS LIST\n\n",message);
	if (!crx_lst)
	    (void) printf("No crosses\n");

	for (cxl = crx_lst; cxl; cxl = cxl->next)
	    print_blk_crx_list_element(cxl);

	(void) printf("\nEND OF %s BLOCK CROSS LIST\n\n",message);
}		/*end print_blk_crx_list*/

LOCAL	void print_blk_crx_list_element(
	BLK_CRX_LIST *cxl)
{
	(void) printf("Cross list element %p\n",(POINTER)cxl);
	(void) printf("next = %p, prev = %p, opposite = %p\n",
		      (POINTER)cxl->next,(POINTER)cxl->prev,
		      (POINTER)cxl->opp_crxl);
	(void) printf("Opposite node\n");
	print_node(cxl->opp_node);

	(void) printf("crx %p  side %d  index %p\n",
		      (POINTER)cxl->crx,cxl->side,(POINTER)cxl->crx_index);
	if (cxl->crx)
	{
	    switch(cxl->side)
	    {
	    case SOUTH:
	        (void) printf("\tCross located on SOUTH side of block\n");
	        break;
	    case EAST:
	        (void) printf("\tCross located on EAST side of block\n");
	        break;
	    case NORTH:
	        (void) printf("\tCross located on NORTH side of block\n");
	        break;
	    case WEST:
	        (void) printf("\tCross located on WEST side of block\n");
	        break;
	    }
	    print_crxings(cxl->crx,NO);
	    (void) printf("Cross index = %p\n",(POINTER)cxl->crx_index);
	}
	else
	    (void) printf("cxl->crx = NULL\n");

	if (cxl->blk_cur)
	{
	    (void) printf("Block interface curve = %llu on interface %llu\n",
	    	          curve_number(cxl->blk_cur),
	    	          interface_number(cxl->blk_cur->interface));
	    (void) printf("orientation = %s\n",
	    	          (cxl->blk_cur_orient == POSITIVE_ORIENTATION) ?
	    	          "POSITIVE_ORIENTATION" : "NEGATIVE_ORIENTATION");
	}
	else
	    (void) printf("Block interface curve = NULL\n");
	
	if (cxl->corner)
	    print_blk_corner(cxl->corner);
	else
	    (void) printf("cxl->corner = NULL\n");

	(void) printf("End of cross list element %p\n\n",(POINTER)cxl);
}		/*end print_blk_crx_list_element*/

LOCAL	void print_blk_corner(
	BLK_CORNER *corner)
{
	(void) printf("Block corner %p\n",(POINTER)corner);
	(void) printf("corner->nd = %p, ",(POINTER)corner->nd);
	if (corner->nd != NULL)
	    (void) printf("at position (%g, %g)\n",
		          Coords(corner->nd)[0],Coords(corner->nd)[1]);
	else
	    (void) printf("\n");
	switch (corner->sides)
	{
	case SOUTH_WEST:
	    (void) printf("Corner located on SOUTH_WEST side of block\n");
	    break;
	case SOUTH_EAST:
	    (void) printf("Corner located on SOUTH_EAST side of block\n");
	    break;
	case NORTH_EAST:
	    (void) printf("Corner located on NORTH_EAST side of block\n");
	    break;
	case NORTH_WEST:
	    (void) printf("Corner located on NORTH_WEST side of block\n");
	    break;
	}
	(void) printf("corner index = %d\n",corner->index);
	(void) printf("Component of corner = %d\n",corner->comp);
}		/*end print_blk_corner*/
#endif /* defined(DEBUG_TRI_GRID) */
#endif /* defined(TWOD) */
