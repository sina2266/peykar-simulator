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
*				triel1.c
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*/

#if defined(TWOD)

#include <tri/trilocaldecs.h>

	/* LOCAL Function Declarations */
LOCAL	boolean	triangulate_blk(TRI_GRID*,int*,
				LINEAR_ELEMENT**,BILINEAR_ELEMENT**);
LOCAL	boolean	triangulate_el00(TRI_GRID*,int,int*,int,int,int*,int,int,
				 int*,int,int,int*,int,LINEAR_ELEMENT**,int*);
LOCAL	void	set_element_side(CRXING*,CRXING*,char*);
LOCAL	void	triangulate_el10(TRI_GRID*,CRXING*,int,int*,CRXING*,CRXING*,
				 int,int*,CRXING*,CRXING*,int,int*,
				 CRXING*,CRXING*,int,int*,
				 CRXING*,LINEAR_ELEMENT**,int*);
LOCAL	void	triangulate_el3(TRI_GRID*,CRXING*,int,int*,CRXING*,CRXING*,
				int,int*,CRXING*,LINEAR_ELEMENT**,int*);
LOCAL	void	triangulate_el5(TRI_GRID*,CRXING*,int,int*,int,int,int*,
				CRXING*,CRXING*,int,int*,CRXING*,
				LINEAR_ELEMENT**,int*);
LOCAL	void	triangulate_el6(TRI_GRID*,CRXING*,int,int*,int,int,int*,
				CRXING*,CRXING*,int,int*,int,int,int*,
				CRXING*,LINEAR_ELEMENT**,int*);
LOCAL	void	triangulate_el7(TRI_GRID*,CRXING*,int,int*,int,int,int*,
				int,int,int*,CRXING*,CRXING*,int,int*,
				CRXING*,LINEAR_ELEMENT**,int*);
LOCAL	void	triangulate_el8(TRI_GRID*,CRXING*,int,int*,CRXING*,CRXING*,
				int,int*,CRXING*,CRXING*,int,int*,CRXING*,
				LINEAR_ELEMENT**,int*);
LOCAL	void	triangulate_el9(TRI_GRID*,CRXING*,int,int*,int,int,int*,
				CRXING*,CRXING*,int,int*,CRXING*,CRXING*,
				int,int*,CRXING*,LINEAR_ELEMENT**,int*);

#if !defined(__INTEL_COMPILER)
#pragma	noinline	triangulate_blk
#pragma	noinline	triangulate_el00
#pragma	noinline	set_element_side
#pragma	noinline	triangulate_el10
#pragma	noinline	triangulate_el3
#pragma	noinline	triangulate_el5
#pragma	noinline	triangulate_el6
#pragma	noinline	triangulate_el7
#pragma	noinline	triangulate_el8
#pragma	noinline	triangulate_el9
#endif /*!defined(__INTEL_COMPILER)*/

LOCAL	int	ext_comp;

#define Test_crx_contiguity(s_crx,s_num,e_crx,e_num)			\
{									\
	if (s_crx->hs  != e_crx->hs) continue;				\
	if ((e_num != s_num+1) && (e_num != s_num-1))			\
	{								\
		if (!s_crx->end) continue;				\
		if (!e_crx->end) continue;				\
		if (!is_closed_curve(Curve_of_hs(s_crx->hs)))		\
			continue;					\
	}								\
}

#define set_element_comp(cx_s,cx_w,side,comp);				\
	set_element_side(cx_s,cx_w,&side);				\
	if (side == 'L') comp = negative_component(cx_s->hs);		\
	else		  comp = positive_component(cx_s->hs);		\

#define	set_element_states(cx_s,cx_w,st_s,st_w,comp);			\
	if (negative_component(cx_s->hs) == comp)			\
	{								\
		st_s = left_state(cx_s->pt);				\
		st_w = left_state(cx_w->pt);				\
	}								\
	else								\
	{								\
		st_s = right_state(cx_s->pt);				\
		st_w = right_state(cx_w->pt);				\
	}



EXPORT	int triangulate_mesh2d(
	TRI_GRID	*ntg)
{
	int		xmax = ntg->rect_grid.gmax[0];
	int		ymax = ntg->rect_grid.gmax[1];
	int		ix, iy;
	int		icoords[MAXD];
	LINEAR_ELEMENT  *lins;
	BILINEAR_ELEMENT *bilins;

#if defined(DEBUG_TRI_GRID)
	debug_print("triangulate","Entered triangulate_mesh2d()\n");
#endif /* defined(DEBUG_TRI_GRID) */

	ntg->_locate_on_trigrid = tg_locate;

	ntg->area   = tg_area;

	lins  = ntg->lin_els;		ntg->n_lin_els  = 0;
	bilins = ntg->bilin_els;	ntg->n_bilin_els = 0;

	ext_comp = exterior_component(ntg->grid_intfc);

	for (iy = 0;  iy < ymax;  ++iy)
	{
	    icoords[1] = iy;
	    for (ix = 0;  ix < xmax;  ++ix)
	    {
	    	icoords[0] = ix;
	    	if (blk_triangulate(ntg,icoords,&lins,&bilins)==FUNCTION_FAILED)
		{
		    (void) printf("WARNING in triangulate_mesh(), "
		                  "blk_triangulate() failed\n");
		    if (ntg->alloc.blk_els1)
		        free_ntg_blk_node_lists(ntg,ntg->rect_grid.gmax);
#if defined(DEBUG_TRI_GRID)
	            debug_print("triangulate","Left triangulate_mesh2d(), "
		                        "status = MODIFY_TIME_STEP\n");
#endif /* defined(DEBUG_TRI_GRID) */
		    return MODIFY_TIME_STEP;
		}
	    }
	}

#if defined(DEBUG_TRI_GRID)
	if (debugging("triangulate"))
	{
	    (void) printf("\nmesh triangulated\n");
	    (void) printf("%d triangles,   %d quadrilaterals generated\n",
	    	          ntg->n_lin_els,ntg->n_bilin_els);
	    print_blk_els0(ntg);
	    print_blk_els1(ntg);
	}
	if (debugging("alloc_view"))
	{
	    (void) printf("After triangulate mesh\n");
	    long_alloc_view(stdout);
	}
	if (debugging("plot_tri_grid"))
	{
	    static  FILE    *ntgrid = NULL;
	    static  boolean first = YES;
	    static  char    filename[15];

	    if (first)
	    {
		int nn = pp_numnodes();
	    	first = NO;
	    	if (nn == 1)
	    	    (void) sprintf(filename,"ntgrid.plt");
		else
		{
		    int nd, myid = pp_mynode();
		    for (nd = 0; nn != 0; nn /=10, ++nd);
		    (void) sprintf(filename,"ntgrid.plt.%s",
					    right_flush(myid,nd));
		}

		ntgrid = fopen(filename,"w");
		if (debugging("nobuf"))
		    setbuf(ntgrid,NULL);
		print_machine_parameters(ntgrid);
	    }
	    print_triangulation(ntgrid,ntg);
	}
#endif /* defined(DEBUG_TRI_GRID) */

	if (ntg->alloc.blk_els1)
	    free_ntg_blk_node_lists(ntg,ntg->rect_grid.gmax);
#if defined(DEBUG_TRI_GRID)
	debug_print("triangulate","Left triangulate_mesh2d(), status = GOOD_STEP\n");
#endif /* defined(DEBUG_TRI_GRID) */
	return GOOD_STEP;
}		/*end triangulate_mesh2d*/


EXPORT	boolean fast_blk_triangulate(
	TRI_GRID	*ntg,
	int		*icoords,
	LINEAR_ELEMENT	**tri,
	BILINEAR_ELEMENT **qd)
{
	if (Blk_el1(icoords,ntg).interior_node == 0)
		return triangulate_blk(ntg,icoords,tri,qd);
	else
		return triangulate_blk_with_nodes(ntg,icoords,tri,qd);
}		/*end fast_blk_triangulate*/


LOCAL	boolean triangulate_blk(
	TRI_GRID	*ntg,
	int		*icoords,
	LINEAR_ELEMENT	**lin,
	BILINEAR_ELEMENT **bilin)
{
	BLK_EL0		*blk_el0;
	boolean		status = NO;
	int		ncs, ncw, ncn, nce;
	int		*t_num;
	int		*s_list, *e_list, *n_list, *w_list;
 	int		jse, jne, jnw, jsw;

	retrieve_crossing_lists(ntg,icoords,&ncs,&ncw,&ncn,&nce,
		&jsw,&jse,&jnw,&jne,&s_list,&w_list,&n_list,&e_list);


	blk_el0 = &Blk_el0(icoords,ntg);
	if (ncs && (ncw || ncn || nce))
	{
		blk_el0_linear_els(blk_el0) = *lin;
		t_num = &num_lin_els_in_blk(blk_el0);

		status = triangulate_el0(ntg,ncs,s_list,jse,nce,e_list,jne,
				ncn,n_list,jnw,ncw,w_list,jsw,lin,t_num);
		if (status == FUNCTION_SUCCEEDED)
			return status;
	}

	if (nce && (ncw || ncn || ncs))
	{
		blk_el0_linear_els(blk_el0) = *lin;
		t_num = &num_lin_els_in_blk(blk_el0);

		status = triangulate_el0(ntg,nce,e_list,jne,ncn,n_list,jnw,
				ncw,w_list,jsw,ncs,s_list,jse,lin,t_num);
		if (status == FUNCTION_SUCCEEDED)
			return status;
	}

	if (ncn && (ncw || ncs || nce))
	{
		blk_el0_linear_els(blk_el0) = *lin;
		t_num = &num_lin_els_in_blk(blk_el0);

		status = triangulate_el0(ntg,ncn,n_list,jnw,ncw,w_list,jsw,
				ncs,s_list,jse,nce,e_list,jne,lin,t_num);
		if (status == FUNCTION_SUCCEEDED)
			return status;
	}
	
	if (ncw && (ncs || ncn || nce))
	{
		blk_el0_linear_els(blk_el0) = *lin;
		t_num = &num_lin_els_in_blk(blk_el0);

		status = triangulate_el0(ntg,ncw,w_list,jsw,ncs,s_list,jse,
				nce,e_list,jne,ncn,n_list,jnw,lin,t_num);
		if (status == FUNCTION_SUCCEEDED)
			return status;
	}

		/* This case is topologically indistinguishable */
		/* from a rectangle with no cuts as far as the	*/
		/* crxing list can discern			*/

	set_quad(ntg,icoords,jsw,jse,jnw,jne,bilin);
	return FUNCTION_SUCCEEDED;
}		/*end triangulate_blk*/


LIB_LOCAL	void retrieve_crossing_lists(
	TRI_GRID	*ntg,
	int		*icoords,
	int		*pncs,
	int		*pncw,
	int		*pncn,
	int		*pnce,
	int		*jsw,
	int		*jse,
	int		*jnw,
	int		*jne,
	int		**ps_list,
	int		**pw_list,
	int		**pn_list,
	int		**pe_list)
{
	static	int	tmp_n_store[MAX_NUM_CRX], tmp_w_store[MAX_NUM_CRX];
	int		ix = icoords[0], iy = icoords[1];
	int		xmax = ntg->rect_grid.gmax[0];
	int		*s_list, *e_list, *n_list, *w_list;
	int		ncs, ncw, ncn, nce;
	int		k_s, k_w, k_n, k_e;
	int 		i, j, xmp1;
	Table		*T = table_of_interface(ntg->grid_intfc);

	xmp1 = xmax + 1;
	k_s = 2*iy*xmax + iy + ix;	ncs = T->seg_crx_count[k_s];
	s_list = T->seg_crx_lists[k_s];
	k_w = k_s + xmax;		ncw = T->seg_crx_count[k_w];
	w_list = tmp_w_store;
	k_n = k_w + xmax + 1;   ncn = T->seg_crx_count[k_n];
	n_list = tmp_n_store;
	k_e = k_w + 1;          nce = T->seg_crx_count[k_e];
	e_list = T->seg_crx_lists[k_e];

	*jsw = iy * xmp1 + ix;		*jse = *jsw + 1;
	*jnw = (iy+1) * xmp1 + ix;	*jne = *jnw + 1;

#if defined(DEBUG_TRI_GRID)
	if (debugging("exact_tri"))
	{
 		TG_PT	*pse, *pne, *pnw, *psw;

		psw = &(ntg->node_points[*jsw]);
		pse = &(ntg->node_points[*jse]);
		pnw = &(ntg->node_points[*jnw]);
		pne = &(ntg->node_points[*jne]);

		(void) printf("\ntriangulating ix %d iy %d\n",ix,iy);
		(void) printf("\tk_s %d k_w %d k_n %d k_e %d ",
			      k_s,k_w,k_n,k_e);
		(void) printf("ncs %d ncw %d ncn %d nce %d\n",ncs,ncw,ncn,nce);
		(void) printf("\tpse: %g  %g   pne: %g %g\n",
			      Coords(pse)[0],Coords(pse)[1],
			      Coords(pne)[0],Coords(pne)[1]);
		(void) printf("\tpnw: %g  %g   psw: %g %g\n",
			      Coords(pnw)[0],Coords(pnw)[1],
			      Coords(psw)[0],Coords(psw)[1]);
	}
#endif /* defined(DEBUG_TRI_GRID) */

			/* move seg_crx_lists_store for N and W */
			/* mesh sides to tmp storage and invert */
			/* order such that crx_lists are given  */
			/* in a counter_clockwise manner        */

	if ((ncn > MAX_NUM_CRX) || (ncw > MAX_NUM_CRX))
	{
		screen("> MAX_NUM_CRX = %d crossings found on ",MAX_NUM_CRX);
		screen("N or W side of block ix %d iy %d.\n",ix,iy);
		screen("Increase limits in exact_blk_triangulate().\n");
		if (ncn > MAX_NUM_CRX) n_list = (int *) Store(ncn*INT);
		if (ncw > MAX_NUM_CRX) w_list = (int *) Store(ncw*INT);
	}

	for (i = 0, j = ncn-1;  i < ncn;  ++i, --j)
	{
		n_list[i] = *(T->seg_crx_lists[k_n] + j);
	}

	for (i = 0, j = ncw-1;  i < ncw;  ++i, --j)
	{
		w_list[i] = *(T->seg_crx_lists[k_w] + j);
	}

#if defined(DEBUG_TRI_GRID)
	if (debugging("invert"))
	{
		(void) printf("\tInverted n_list:\n");
		for (i = 0;  i < ncn;  ++i)
			(void) printf("list: %d\n",n_list[i]);
		(void) printf("\tInverted w_list:\n");
		for (i = 0;  i < ncw;  ++i)
			(void) printf("list: %d\n",w_list[i]);
		(void) printf("\ts_list:\n");
		for (i = 0;  i < ncs;  ++i)
			(void) printf("list: %d\n",s_list[i]);
		(void) printf("\te_list:\n");
		for (i = 0;  i < nce;  ++i)
			(void) printf("list: %d\n",e_list[i]);
	}
#endif /* defined(DEBUG_TRI_GRID) */

	*pn_list = n_list;	*ps_list = s_list;
	*pe_list = e_list;	*pw_list = w_list;

	*pncn = ncn;		*pncs = ncs;
	*pnce = nce;		*pncw = ncw;
}		/*end retrieve_crossing_lists*/

LIB_LOCAL	void set_quad(
	TRI_GRID	*ntg,
	int		*icoords,
	int		jsw,
	int		jse,
	int		jnw,
	int		jne,
	BILINEAR_ELEMENT **bilin)
{
 	TG_PT		*pse, *pne, *pnw, *psw;
	BLK_EL0		*blk_el0;
	Table		*T = table_of_interface(ntg->grid_intfc);

	psw = &(ntg->node_points[jsw]);
	pse = &(ntg->node_points[jse]);
	pnw = &(ntg->node_points[jnw]);
	pne = &(ntg->node_points[jne]);

	(*bilin)->p[0] = psw;		(*bilin)->p[1] = pse;
	(*bilin)->p[2] = pne;		(*bilin)->p[3] = pnw;
	(*bilin)->comp = T->components[jsw];

	blk_el0 = &Blk_el0(icoords,ntg);
	blk_el0_bilinear_el(blk_el0) = *bilin;

#if defined(DEBUG_TRI_GRID)
	if (debugging("triangulate")) print_BILINEAR_ELEMENT(*bilin,ntg);
#endif /* defined(DEBUG_TRI_GRID) */

	set_bilinear_blk_el0(blk_el0);
	++ntg->n_bilin_els;
	++(*bilin);
}		/*end set_quad*/

LIB_LOCAL	boolean triangulate_el0(
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
	LINEAR_ELEMENT	**tri,
	int		*t_num)
{
	boolean		status;

	status = triangulate_el00(ntg,ncs,s_list,jse,nce,e_list,jne,
		 		ncn,n_list,jnw,ncw,w_list,jsw,tri,t_num);
	if (status == FUNCTION_SUCCEEDED)
		return status;

#if defined(DEBUG_TRI_GRID)
	if (debugging("triangulate")) 
		(void) printf("Failed triangulation from south side\n");
#endif /* defined(DEBUG_TRI_GRID) */

	if (nce > 0)
	{
		status = triangulate_el00(ntg,nce,e_list,jne,ncn,n_list,jnw,
				ncw,w_list,jsw,ncs,s_list,jse,tri,t_num);
		if (status == FUNCTION_SUCCEEDED)
			return status;

#if defined(DEBUG_TRI_GRID)
		if (debugging("triangulate"))
			(void) printf("Failed triangulation from east side\n");
#endif /* defined(DEBUG_TRI_GRID) */

	}
	if (ncn > 0)
	{
		status = triangulate_el00(ntg,ncn,n_list,jnw,ncw,w_list,jsw,
				ncs,s_list,jse,nce,e_list,jne,tri,t_num);
		if (status == FUNCTION_SUCCEEDED)
			return status;

#if defined(DEBUG_TRI_GRID)
		if (debugging("triangulate"))
		       (void) printf("Failed triangulation from north side\n");
#endif /* defined(DEBUG_TRI_GRID) */

	}
	if (ncw > 0)
	{
		status = triangulate_el00(ntg,ncw,w_list,jsw,ncs,s_list,jse,
				nce,e_list,jne,ncn,n_list,jnw,tri,t_num);
		if (status == FUNCTION_SUCCEEDED)
			return status;

#if defined(DEBUG_TRI_GRID)
		if (debugging("triangulate"))
			(void) printf("Failed triangulation from west side\n");
#endif /* defined(DEBUG_TRI_GRID) */

	}

	/* This case is topologically indistinguishable */
	/* from a rectangle with no cuts as far as the	*/
	/* crxing list can discern			*/
	
	return FUNCTION_FAILED;
}		/*end triangulate_el0*/

LOCAL	boolean triangulate_el00(
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
	LINEAR_ELEMENT	**tri,
	int		*t_num)
{
	Table		*T = table_of_interface(ntg->grid_intfc);
	CRXING		*s_crx, *e_crx, *n_crx, *w_crx;
	int		sc, ec, nc, wc;
	int		s_num, e_num, n_num, w_num;
	int		sc1, ec1, nc1, wc1;

#if defined(DEBUG_TRI_GRID)
	if (debugging("triangulate"))
	{
		TG_PT	*pse, *pne, *pnw, *psw;

		psw = &(ntg->node_points[jsw]);
		pse = &(ntg->node_points[jse]);
		pnw = &(ntg->node_points[jnw]);
		pne = &(ntg->node_points[jne]);

		print_el0(ntg,ncs,s_list,pse,nce,e_list,
				pne,ncn,n_list,pnw,ncw,w_list,psw);
	}
#endif /* defined(DEBUG_TRI_GRID) */

	for (sc = 0;  sc < ncs;  ++sc)
	{
		s_crx = &(T->crx_store[*(s_list+sc)]);
		s_num = s_crx->crx_num;
		sc1   = sc+1;
		if (nce > 0)
		{
			for (ec = 0;  ec < nce;  ++ec)
			{
				e_crx = &(T->crx_store[*(e_list+ec)]);
				e_num = e_crx->crx_num;
				Test_crx_contiguity(s_crx,s_num,e_crx,e_num)

				ec1   = ec+1;
				triangulate_el1(ntg,s_crx,ncs-sc1,s_list+sc1,
					jse,ec,e_list,e_crx,tri,t_num);
				triangulate_el4(ntg,e_crx,nce-ec1,e_list+ec1,
					jne,ncn,n_list,jnw,ncw,w_list,
					jsw,sc,s_list,s_crx,tri,t_num);
				return FUNCTION_SUCCEEDED;
			}
		}
		if (ncn > 0)
		{
			for (nc = 0;  nc < ncn;  ++nc)
			{
				n_crx = &(T->crx_store[*(n_list+nc)]);
				n_num = n_crx->crx_num;
				Test_crx_contiguity(s_crx,s_num,n_crx,n_num)

				nc1   = nc+1;
				triangulate_el2(ntg,s_crx,ncs-sc1,s_list+sc1,
						jse,nce,e_list,jne,
						nc,n_list,n_crx,tri,t_num);
				triangulate_el2(ntg,n_crx,ncn-nc1,n_list+nc1,
						jnw,ncw,w_list,jsw,
						sc,s_list,s_crx,tri,t_num);
				return FUNCTION_SUCCEEDED;
			}
		}
		if (ncw > 0)
		{
			for (wc = 0;  wc < ncw;  ++wc)
			{
				w_crx = &(T->crx_store[*(w_list+wc)]);
				w_num = w_crx->crx_num;
				Test_crx_contiguity(s_crx,s_num,w_crx,w_num)

				wc1   = wc+1;
				triangulate_el1(ntg,w_crx,ncw-wc1,w_list+wc1,
					jsw,sc,s_list,s_crx,tri,t_num);
				triangulate_el4(ntg,s_crx,ncs-sc1,s_list+sc1,
					jse,nce,e_list,jne,ncn,n_list,
					jnw,wc,w_list,w_crx,tri,t_num);
				return FUNCTION_SUCCEEDED;
			}
		}
	}
			/* Reach here - unable to triangulate */
	return FUNCTION_FAILED;
}		/*end triangulate_el00*/

LIB_LOCAL	void triangulate_el1(
	TRI_GRID	*ntg,
	CRXING		*cx_s,
	int		ncs,
	int		*s_list,
	int		jse,
	int		nce,
	int		*e_list,
	CRXING		*cx_e,
	LINEAR_ELEMENT	**tri,
	int		*t_num)
{
	Table		*T = table_of_interface(ntg->grid_intfc);
	COMPONENT	comp;
	CRXING		*s_crx, *e_crx;
	Locstate	st_s, st_se, st_e;
	TG_PT		*pse;
	char		side;
	int		sc, ec, s_num, e_num, sc1, ec1;

	pse = &(ntg->node_points[jse]);

#if defined(DEBUG_TRI_GRID)
	if (debugging("triangulate"))
		print_el1(ntg,cx_s,ncs,s_list,pse,nce,e_list,cx_e);
#endif /* defined(DEBUG_TRI_GRID) */

	for (sc = 0;  sc < ncs;  ++sc)
	{
		s_crx = &(T->crx_store[*(s_list+sc)]);
		s_num = s_crx->crx_num;
		sc1   = sc+1;
		for (ec = 0;  ec < nce;  ++ec)
		{
			e_crx = &(T->crx_store[*(e_list+ec)]);
			e_num = e_crx->crx_num;
			Test_crx_contiguity(s_crx,s_num,e_crx,e_num)

			ec1   = ec+1;
			triangulate_el3(ntg,cx_s,sc,s_list,s_crx,e_crx,
					nce-ec1,e_list+ec1,cx_e,tri,t_num);
			triangulate_el1(ntg,s_crx,ncs-sc1,s_list+sc1,
					jse,ec,e_list,e_crx,tri,t_num);
			return;
		}
	}
			/* reach here if crossings dictate that */
			/* topology is indistinguishable from   */
			/* 	    no crossings case		*/

	set_element_comp(cx_s,cx_e,side,comp);
	if (side == 'L')
	{
		st_s = left_state(cx_s->pt);
		st_e = left_state(cx_e->pt);
	}
	else
	{
		st_s = right_state(cx_s->pt);
		st_e = right_state(cx_e->pt);
	}
	st_se = ntg->states[jse];

	set_tri(ntg,tri,t_num,cx_s->nd,pse,cx_e->nd,st_s,st_se,st_e,
						E_SIDE,E_SIDE,F_SIDE,comp);
}		/*end triangulate_el1*/

LIB_LOCAL	void triangulate_el2(
	TRI_GRID	*ntg,
	CRXING		*cx_s,
	int		ncs,
	int		*s_list,
	int		jse,
	int		nce,
	int		*e_list,
	int		jne,
	int		ncn,
	int		*n_list,
	CRXING		*cx_n,
	LINEAR_ELEMENT	**tri,
	int		*t_num)
{
	Table		*T = table_of_interface(ntg->grid_intfc);
	COMPONENT	comp;
	CRXING		*s_crx, *e_crx, *n_crx;
	Locstate	st_s, st_se, st_ne, st_n;
	TG_PT		*pse, *pne;
	char		side;
	int		sc, ec, nc;
	int		s_num, e_num, n_num;
	int		sc1, ec1, nc1;

	pse = &(ntg->node_points[jse]);		pne = &(ntg->node_points[jne]);

#if defined(DEBUG_TRI_GRID)
	if (debugging("triangulate"))
		print_el2(ntg,cx_s,ncs,s_list,pse,nce,
				e_list,pne,ncn,n_list,cx_n);
#endif /* defined(DEBUG_TRI_GRID) */

	for (sc = 0;  sc < ncs;  ++sc)
	{
		s_crx = &(T->crx_store[*(s_list+sc)]);
		s_num = s_crx->crx_num;
		sc1   = sc+1;
		for (ec = 0;  ec < nce;  ++ec)
		{
			e_crx = &(T->crx_store[*(e_list+ec)]);
			e_num = e_crx->crx_num;
			Test_crx_contiguity(s_crx,s_num,e_crx,e_num)

			ec1   = ec+1;
			triangulate_el5(ntg,e_crx,nce-ec1,e_list+ec1,jne,ncn,
				n_list,cx_n,cx_s,sc,s_list,s_crx,tri,t_num);
			triangulate_el1(ntg,s_crx,ncs-sc1,s_list+sc1,
				jse,ec,e_list,e_crx,tri,t_num);
			return;
		}
		for (nc = 0;  nc < ncn;  ++nc)
		{
			n_crx = &(T->crx_store[*(n_list+nc)]);
			n_num = n_crx->crx_num;
			Test_crx_contiguity(s_crx,s_num,n_crx,n_num)

			nc1   = nc+1;
			triangulate_el3(ntg,cx_s,sc,s_list,s_crx,n_crx,
					nce-nc1,n_list+nc1,cx_n,tri,t_num);
			triangulate_el2(ntg,s_crx,ncs-sc1,s_list+sc1,jse,
				nce,e_list,jne,nc,n_list,n_crx,tri,t_num);
			return;
		}
	}
	for (ec = 0;  ec < nce;  ++ec)
	{
		e_crx = &(T->crx_store[*(e_list+ec)]);
		e_num = e_crx->crx_num;
		ec1   = ec+1;
		for (nc = 0;  nc < ncn;  ++nc)
		{
			n_crx = &(T->crx_store[*(n_list+nc)]);
			n_num = n_crx->crx_num;
			Test_crx_contiguity(e_crx,e_num,n_crx,n_num)

			nc1   = nc+1;
			triangulate_el5(ntg,cx_s,ncs,s_list,jse,ec,e_list,e_crx,
		   		n_crx,ncn-nc1,n_list+nc1,cx_n,tri,t_num);
			triangulate_el1(ntg,e_crx,nce-ec1,e_list+ec1,
					jne,nc,n_list,n_crx,tri,t_num);
			return;
		}
	}
			/* reach here if crossings dictate that */
			/* topology is indistinguishable from   */
			/* 	    no crossings case		*/

	set_element_comp(cx_s,cx_n,side,comp);
	if (side == 'L')
	{
		st_s = left_state(cx_s->pt);
		st_n = left_state(cx_n->pt);
	}
	else
	{
		st_s = right_state(cx_s->pt);
		st_n = right_state(cx_n->pt);
	}
	st_se = ntg->states[jse];
	st_ne = ntg->states[jne];

	set_tri(ntg,tri,t_num,cx_s->nd,pse,pne,     st_s,st_se,st_ne,
						E_SIDE,E_SIDE,I_SIDE,comp);
	set_tri(ntg,tri,t_num,cx_s->nd,pne,cx_n->nd,st_s,st_ne,st_n,
						I_SIDE,E_SIDE,F_SIDE,comp);
}		/*end triangulate_el2*/

LOCAL	void triangulate_el3(
	TRI_GRID	*ntg,
	CRXING		*cx_s1,
	int		ncs,
	int		*s_list,
	CRXING		*cx_s2,
	CRXING		*cx_a1,
	int		nca,
	int		*a_list,
	CRXING		*cx_a2,
	LINEAR_ELEMENT	**lin,
	int		*t_num)
{
	Table		*T = table_of_interface(ntg->grid_intfc);
	COMPONENT	comp;
	CRXING		*s_crx, *a_crx;
	Locstate	st_s1, st_s2, st_a1, st_a2;
	char		side;
	int		sc, ac, s_num, a_num, sc1, ac1;

#if defined(DEBUG_TRI_GRID)
	if (debugging("triangulate"))
		print_el3(ntg,cx_s1,ncs,s_list,cx_s2,cx_a1,nca,a_list,cx_a2);
#endif /* defined(DEBUG_TRI_GRID) */

	for (sc = 0;  sc < ncs;  ++sc)
	{
		s_crx = &(T->crx_store[*(s_list+sc)]);
		s_num = s_crx->crx_num;
		sc1   = sc+1;
		for (ac = 0;  ac < nca;  ++ac)
		{
			a_crx = &(T->crx_store[*(a_list+ac)]);
			a_num = a_crx->crx_num;
			Test_crx_contiguity(s_crx,s_num,a_crx,a_num)

			ac1   = ac+1;
			triangulate_el3(ntg,cx_s1,sc,s_list,s_crx,a_crx,nca-ac1,
						a_list+ac1,cx_a2,lin,t_num);
			triangulate_el3(ntg,s_crx,ncs-sc1,s_list+sc1,cx_s2,
					cx_a1,ac,a_list,a_crx,lin,t_num);
			return;
		}
	}
			/* reach here if crossings dictate that */
			/* topology is indistinguishable from   */
			/* 	    no crossings case		*/

	set_element_comp(cx_s1,cx_a2,side,comp);
	if (side == 'L')
	{
		st_s1 = left_state(cx_s1->pt);
		st_a2 = left_state(cx_a2->pt);
	}
	else
	{
		st_s1 = right_state(cx_s1->pt);
		st_a2 = right_state(cx_a2->pt);
	}
	set_element_states(cx_a1,cx_s2,st_a1,st_s2,comp);

	set_tri(ntg,lin,t_num,cx_s1->nd,cx_a1->nd,cx_a2->nd,st_s1,st_a1,st_a2,
						I_SIDE,E_SIDE,F_SIDE,comp);
	set_tri(ntg,lin,t_num,cx_s1->nd,cx_s2->nd,cx_a1->nd,st_s1,st_s2,st_a1,
						E_SIDE,F_SIDE,I_SIDE,comp);
}		/*end triangulate_el3*/

LIB_LOCAL	void triangulate_el4(
	TRI_GRID	*ntg,
	CRXING		*cx_s,
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
	CRXING		*cx_w,
	LINEAR_ELEMENT	**tri,
	int		*t_num)
{
	Table		*T = table_of_interface(ntg->grid_intfc);
	COMPONENT	comp;
	CRXING		*s_crx, *e_crx, *n_crx, *w_crx;
	Locstate	st_s, st_se, st_ne, st_nw, st_w;
	TG_PT		*pse, *pne, *pnw;
	char		side;
	int		sc, ec, nc, wc;
	int		s_num, e_num, n_num, w_num;
	int		sc1, ec1, nc1, wc1;

	pse = &(ntg->node_points[jse]);
	pnw = &(ntg->node_points[jnw]);
	pne = &(ntg->node_points[jne]);

#if defined(DEBUG_TRI_GRID)
	if (debugging("triangulate"))
		print_el4(ntg,cx_s,ncs,s_list,pse,nce,e_list,pne,
			   		ncn,n_list,pnw,ncw,w_list,cx_w);
#endif /* defined(DEBUG_TRI_GRID) */

	for (sc = 0;  sc < ncs;  ++sc)
	{
		s_crx = &(T->crx_store[*(s_list+sc)]);
		s_num = s_crx->crx_num;
		sc1   = sc+1;
		for (ec = 0;  ec < nce;  ++ec)
		{
			e_crx = &(T->crx_store[*(e_list+ec)]);
			e_num = e_crx->crx_num;
			Test_crx_contiguity(s_crx,s_num,e_crx,e_num)

			ec1   = ec+1;
			triangulate_el7(ntg,e_crx,nce-ec1,e_list+ec1,jne,ncn,
					n_list,jnw,ncw,w_list,cx_w,cx_s,
						sc,s_list,s_crx,tri,t_num);
			triangulate_el1(ntg,s_crx,ncs-sc1,s_list+sc1,
					jse,ec,e_list,e_crx,tri,t_num);
			return;
		}
		for (nc = 0;  nc < ncn;  ++nc)
		{
			n_crx = &(T->crx_store[*(n_list+nc)]);
			n_num = n_crx->crx_num;
			Test_crx_contiguity(s_crx,s_num,n_crx,n_num)

			nc1   = nc+1;
			triangulate_el5(ntg,n_crx,ncn-nc1,n_list+nc1,jnw,ncw,
				w_list,cx_w,cx_s,sc,s_list,s_crx,tri,t_num);
			triangulate_el2(ntg,s_crx,ncs-sc1,s_list+sc1,jse,
				nce,e_list,jne,nc,n_list,n_crx,tri,t_num);
			return;
		}
		for (wc = 0;  wc < ncw;  ++wc)
		{
			w_crx = &(T->crx_store[*(w_list+wc)]);
			w_num = w_crx->crx_num;
			Test_crx_contiguity(s_crx,s_num,w_crx,w_num)

			wc1   = wc+1;
			triangulate_el3(ntg,cx_s,sc,s_list,s_crx,w_crx,
					ncw-wc1,w_list+wc1,cx_w,tri,t_num);
			triangulate_el4(ntg,s_crx,ncs-sc1,s_list+sc1,jse,
					nce,e_list,jne,nc,n_list,jnw,wc,
					w_list,w_crx,tri,t_num);
			return;
		}
	}
	for (ec = 0;  ec < nce;  ++ec)
	{
		e_crx = &(T->crx_store[*(e_list+ec)]);
		e_num = e_crx->crx_num;
		ec1   = ec+1;
		for (nc = 0;  nc < ncn;  ++nc)
		{
			n_crx = &(T->crx_store[*(n_list+nc)]);
			n_num = n_crx->crx_num;
			Test_crx_contiguity(e_crx,e_num,n_crx,n_num)

			nc1   = nc+1;
			triangulate_el6(ntg,cx_s,ncs,s_list,jse,ec,e_list,e_crx,
		   			n_crx,ncn-nc1,n_list+nc1,jnw,ncw,
						w_list,cx_w,tri,t_num);
			triangulate_el1(ntg,e_crx,nce-ec1,e_list+ec1,
					jne,nc,n_list,n_crx,tri,t_num);
			return;
		}
		for (wc = 0;  wc < ncw;  ++wc)
		{
			w_crx = &(T->crx_store[*(w_list+wc)]);
			w_num = w_crx->crx_num;
			Test_crx_contiguity(e_crx,e_num,w_crx,w_num)

			wc1   = wc+1;
			triangulate_el5(ntg,cx_s,ncs,s_list,jse,ec,e_list,e_crx,
				w_crx,ncw-wc1,w_list+wc1,cx_w,tri,t_num);
			triangulate_el2(ntg,e_crx,nce-ec1,e_list+ec1,jne,ncn,
				n_list,jnw,wc,w_list,w_crx,tri,t_num);
			return;
		}
	}
	for (nc = 0;  nc < ncn;  ++nc)
	{
		n_crx = &(T->crx_store[*(n_list+nc)]);
		n_num = n_crx->crx_num;
		nc1   = nc+1;
		for (wc = 0;  wc < ncw;  ++wc)
		{
			w_crx = &(T->crx_store[*(w_list+wc)]);
			w_num = w_crx->crx_num;
			Test_crx_contiguity(n_crx,n_num,w_crx,w_num)

			wc1   = wc+1;
			triangulate_el7(ntg,cx_s,ncs,s_list,jse,nce,e_list,jne,
					nc,n_list,n_crx,w_crx,ncw-wc1,
						w_list+wc1,cx_w,tri,t_num);
			triangulate_el1(ntg,n_crx,ncn-nc1,n_list+nc1,
					jnw,wc,w_list,w_crx,tri,t_num);
			return;
		}
	}
			/* reach here if crossings dictate that */
			/* topology is indistinguishable from   */
			/* 	    no crossings case		*/

	set_element_comp(cx_s,cx_w,side,comp);
	if (side == 'L')
	{
		st_s = left_state(cx_s->pt);
		st_w = left_state(cx_w->pt);
	}
	else
	{
		st_s = right_state(cx_s->pt);
		st_w = right_state(cx_w->pt);
	}
	st_se = ntg->states[jse];
	st_ne = ntg->states[jne];
	st_nw = ntg->states[jnw];

	set_tri(ntg,tri,t_num,pne,pnw     ,cx_w->nd,st_ne,st_nw,st_w,
						E_SIDE,E_SIDE,I_SIDE,comp);
	set_tri(ntg,tri,t_num,pne,cx_w->nd,cx_s->nd,st_ne,st_w ,st_s,
						I_SIDE,F_SIDE,I_SIDE,comp);
	set_tri(ntg,tri,t_num,pne,cx_s->nd,pse     ,st_ne,st_s,st_se,
						I_SIDE,E_SIDE,E_SIDE,comp);
}		/*end triangulate_el4*/

LOCAL	void triangulate_el5(
	TRI_GRID	*ntg,
	CRXING		*cx_s,
	int		ncs,
	int		*s_list,
	int		jse,
	int		nce,
	int		*e_list,
	CRXING		*cx_e,
	CRXING		*cx_a1,
	int		nca,
	int		*a_list,
	CRXING		*cx_a2,
	LINEAR_ELEMENT	**tri,
	int		*t_num)
{
	Table		*T = table_of_interface(ntg->grid_intfc);
	COMPONENT	comp;
	CRXING		*s_crx, *e_crx, *a_crx;
	Locstate	st_s, st_se, st_e, st_a1, st_a2;
	TG_PT		*pse;
	char		side;
	int		sc, ec, ac;
	int		s_num, e_num, a_num;
	int		sc1, ec1, ac1;

	pse = &(ntg->node_points[jse]);

#if defined(DEBUG_TRI_GRID)
	if (debugging("triangulate"))
		print_el5(ntg,cx_s,ncs,s_list,pse,nce,e_list,cx_e,
						cx_a1,nca,a_list,cx_a2);
#endif /* defined(DEBUG_TRI_GRID) */

	for (sc = 0;  sc < ncs;  ++sc)
	{
		s_crx = &(T->crx_store[*(s_list+sc)]);
		s_num = s_crx->crx_num;
		sc1   = sc+1;
		for (ec = 0;  ec < nce;  ++ec)
		{
			e_crx = &(T->crx_store[*(e_list+ec)]);
			e_num = e_crx->crx_num;
			Test_crx_contiguity(s_crx,s_num,e_crx,e_num)

			ec1   = ec+1;
			triangulate_el8(ntg,cx_s,sc,s_list,s_crx,e_crx,nce-ec1,
						e_list+ec1,cx_e,cx_a1,nca,
						a_list,cx_a2,tri,t_num);
			triangulate_el1(ntg,s_crx,ncs-sc1,s_list+sc1,
					jse,ec,e_list,e_crx,tri,t_num);
			return;
		}
		for (ac = 0;  ac < nca;  ++ac)
		{
			a_crx = &(T->crx_store[*(a_list+ac)]);
			a_num = a_crx->crx_num;
			Test_crx_contiguity(s_crx,s_num,a_crx,a_num)

			ac1   = ac+1;
			triangulate_el3(ntg,cx_s,sc,s_list,s_crx,a_crx,
					nca-ac1,a_list+ac1,cx_a2,tri,t_num);
			triangulate_el5(ntg,s_crx,ncs-sc1,s_list+sc1,jse,ec,
				e_list,cx_e,cx_a1,ac,a_list,a_crx,tri,t_num);
			return;
		}
	}
	for (ec = 0;  ec < nce;  ++ec)
	{
		e_crx = &(T->crx_store[*(e_list+ec)]);
		e_num = e_crx->crx_num;
		ec1   = ec+1;
		for (ac = 0;  ac < nca;  ++ac)
		{
			a_crx = &(T->crx_store[*(a_list+ac)]);
			a_num = a_crx->crx_num;
			Test_crx_contiguity(e_crx,e_num,a_crx,a_num)

			ac1   = ac+1;
			triangulate_el5(ntg,cx_s,ncs,s_list,jse,ec,e_list,e_crx,
				a_crx,nca-ac1,a_list+ac1,cx_a2,tri,t_num);
			triangulate_el3(ntg,e_crx,nce-ec1,e_list+ec1,cx_e,cx_a1,
						ac,a_list,a_crx,tri,t_num);
			return;
		}
	}
			/* reach here if crossings dictate that */
			/* topology is indistinguishable from   */
			/* 	    no crossings case		*/

	set_element_comp(cx_s,cx_a2,side,comp);
	if (side == 'L')
	{
		st_s  = left_state(cx_s->pt);
		st_a2 = left_state(cx_a2->pt);
	}
	else
	{
		st_s  = right_state(cx_s->pt);
		st_a2 = right_state(cx_a2->pt);
	}
	set_element_states(cx_a1,cx_e,st_a1,st_e,comp);
	st_se = ntg->states[jse];

	set_tri(ntg,tri,t_num,cx_s->nd,pse      ,cx_e->nd ,st_s,st_se,st_e,
						E_SIDE,E_SIDE,I_SIDE,comp);
	set_tri(ntg,tri,t_num,cx_s->nd,cx_e->nd ,cx_a1->nd,st_s,st_e ,st_a1,
						I_SIDE,F_SIDE,I_SIDE,comp);
	set_tri(ntg,tri,t_num,cx_s->nd,cx_a1->nd,cx_a2->nd,st_s,st_a1,st_a2,
						I_SIDE,E_SIDE,F_SIDE,comp);
}		/*end triangulate_el5*/

LOCAL	void triangulate_el6(
	TRI_GRID	*ntg,
	CRXING		*cx_s,
	int		ncs,
	int		*s_list,
	int		jse,
	int		nce,
	int		*e_list,
	CRXING		*cx_e,
	CRXING		*cx_n,
	int		ncn,
	int		*n_list,
	int		jnw,
	int		ncw,
	int		*w_list,
	CRXING		*cx_w,
	LINEAR_ELEMENT	**tri,
	int		*t_num)
{
	Table		*T = table_of_interface(ntg->grid_intfc);
	COMPONENT	comp;
	CRXING		*s_crx, *e_crx, *n_crx, *w_crx;
	Locstate	st_s, st_se, st_e, st_n, st_nw, st_w;
	TG_PT		*pse, *pnw;
	char		side;
	int		sc, ec, nc, wc;
	int		s_num, e_num, n_num, w_num;
	int		sc1, ec1, nc1, wc1;

	pse = &(ntg->node_points[jse]);		pnw = &(ntg->node_points[jnw]);

#if defined(DEBUG_TRI_GRID)
	if (debugging("triangulate"))
		print_el6(ntg,cx_s,ncs,s_list,pse,nce,e_list,cx_e,
			   	cx_n,ncn,n_list,pnw,ncw,w_list,cx_w);
#endif /* defined(DEBUG_TRI_GRID) */

	for (sc = 0;  sc < ncs;  ++sc)
	{
		s_crx = &(T->crx_store[*(s_list+sc)]);
		s_num = s_crx->crx_num;
		sc1   = sc+1;
		for (ec = 0;  ec < nce;  ++ec)
		{
			e_crx = &(T->crx_store[*(e_list+ec)]);
			e_num = e_crx->crx_num;
			Test_crx_contiguity(s_crx,s_num,e_crx,e_num)

			ec1   = ec+1;
			triangulate_el9(ntg,cx_n,ncn,n_list,jnw,ncw,w_list,cx_w,
					cx_s,sc,s_list,s_crx,e_crx,nce-ec1,
						e_list+ec1,cx_e,tri,t_num);
			triangulate_el1(ntg,s_crx,ncs-sc1,s_list+sc1,
					jse,ec,e_list,e_crx,tri,t_num);
			return;
		}
		for (nc = 0;  nc < ncn;  ++nc)
		{
			n_crx = &(T->crx_store[*(n_list+nc)]);
			n_num = n_crx->crx_num;
			Test_crx_contiguity(s_crx,s_num,n_crx,n_num)

			nc1   = nc+1;
			triangulate_el5(ntg,n_crx,ncn-nc1,n_list+nc1,jnw,ncw,
				w_list,cx_w,cx_s,sc,s_list,s_crx,tri,t_num);
			triangulate_el5(ntg,s_crx,ncs-sc1,s_list+sc1,jse,nce,
				e_list,cx_e,cx_n,nc,n_list,n_crx,tri,t_num);
			return;
		}
		for (wc = 0;  wc < ncw;  ++wc)
		{
			w_crx = &(T->crx_store[*(w_list+wc)]);
			w_num = w_crx->crx_num;
			Test_crx_contiguity(s_crx,s_num,w_crx,w_num)

			wc1   = wc+1;
			triangulate_el3(ntg,cx_s,sc,s_list,s_crx,w_crx,
					ncw-wc1,w_list+wc1,cx_w,tri,t_num);
			triangulate_el6(ntg,s_crx,ncs-sc1,s_list+sc1,jse,ec,
					e_list,cx_e,cx_n,ncn,n_list,jnw,
						wc,w_list,w_crx,tri,t_num);
			return;
		}
	}
	for (ec = 0;  ec < nce;  ++ec)
	{
		e_crx = &(T->crx_store[*(e_list+ec)]);
		e_num = e_crx->crx_num;
		ec1   = ec+1;
		for (nc = 0;  nc < ncn;  ++nc)
		{
			n_crx = &(T->crx_store[*(n_list+nc)]);
			n_num = n_crx->crx_num;
			Test_crx_contiguity(e_crx,e_num,n_crx,n_num)

			nc1   = nc+1;
			triangulate_el6(ntg,cx_s,ncs,s_list,jse,ec,e_list,e_crx,
		   			n_crx,ncn-nc1,n_list+nc1,jnw,ncw,
						w_list,cx_w,tri,t_num);
			triangulate_el3(ntg,e_crx,nce-ec1,e_list+ec1,cx_e,cx_n,
					nc,n_list,n_crx,tri,t_num);
			return;
		}
		for (wc = 0;  wc < ncw;  ++wc)
		{
			w_crx = &(T->crx_store[*(w_list+wc)]);
			w_num = w_crx->crx_num;
			Test_crx_contiguity(e_crx,e_num,w_crx,w_num)

			wc1   = wc+1;
			triangulate_el5(ntg,cx_s,ncs,s_list,jse,ec,e_list,e_crx,
				w_crx,ncw-wc1,w_list+wc1,cx_w,tri,t_num);
			triangulate_el5(ntg,cx_n,ncn,n_list,jnw,wc,w_list,w_crx,
				e_crx,nce-ec1,e_list+ec1,cx_e,tri,t_num);
			return;
		}
	}
	for (nc = 0;  nc < ncn;  ++nc)
	{
		n_crx = &(T->crx_store[*(n_list+nc)]);
		n_num = n_crx->crx_num;
		nc1   = nc+1;
		for (wc = 0;  wc < ncw;  ++wc)
		{
			w_crx = &(T->crx_store[*(w_list+wc)]);
			w_num = w_crx->crx_num;
			Test_crx_contiguity(n_crx,n_num,w_crx,w_num)

			wc1   = wc+1;
			triangulate_el9(ntg,cx_s,ncs,s_list,jse,nce,e_list,cx_e,
					cx_n,nc,n_list,n_crx,w_crx,ncw-wc1,
						w_list+wc1,cx_w,tri,t_num);
			triangulate_el1(ntg,n_crx,ncn-nc1,n_list+nc1,
					jnw,wc,w_list,w_crx,tri,t_num);
			return;
		}
	}
			/* reach here if crossings dictate that */
			/* topology is indistinguishable from   */
			/* 	    no crossings case		*/

	set_element_comp(cx_s,cx_w,side,comp);
	if (side == 'L')
	{
		st_s = left_state(cx_s->pt);
		st_w = left_state(cx_w->pt);
	}
	else
	{
		st_s = right_state(cx_s->pt);
		st_w = right_state(cx_w->pt);
	}
	set_element_states(cx_n,cx_e,st_n,st_e,comp);
	st_se = ntg->states[jse];
	st_nw = ntg->states[jnw];

	set_tri(ntg,tri,t_num,cx_s->nd,pse     ,cx_e->nd,st_s,st_se,st_e,
						E_SIDE,E_SIDE,I_SIDE,comp);
	set_tri(ntg,tri,t_num,cx_n->nd,pnw,     cx_w->nd,st_n,st_nw,st_w,
						E_SIDE,E_SIDE,I_SIDE,comp);
	set_tri(ntg,tri,t_num,cx_s->nd,cx_e->nd,cx_n->nd,st_s,st_e ,st_n,
						I_SIDE,F_SIDE,I_SIDE,comp);
	set_tri(ntg,tri,t_num,cx_n->nd,cx_w->nd,cx_s->nd,st_n,st_w ,st_s,
						I_SIDE,F_SIDE,I_SIDE,comp);
}		/*end triangulate_el6*/

LOCAL	void triangulate_el7(
	TRI_GRID	*ntg,
	CRXING		*cx_s,
	int		ncs,
	int		*s_list,
	int		jse,
	int		nce,
	int		*e_list,
	int		jne,
	int		ncn,
	int		*n_list,
	CRXING		*cx_n,
	CRXING		*cx_w1,
	int		ncw,
	int		*w_list,
	CRXING		*cx_w2,
	LINEAR_ELEMENT	**tri,
	int		*t_num)
{
	Table		*T = table_of_interface(ntg->grid_intfc);
	COMPONENT	comp;
	CRXING		*s_crx, *e_crx, *n_crx, *w_crx;
	Locstate	st_s, st_se, st_ne, st_n, st_w1, st_w2;
	TG_PT		*pse, *pne;
	char		side;
	int		sc, ec, nc, wc;
	int		s_num, e_num, n_num;
	int		w_num, sc1, ec1, nc1, wc1;

	pse = &(ntg->node_points[jse]);		pne = &(ntg->node_points[jne]);

#if defined(DEBUG_TRI_GRID)
	if (debugging("triangulate"))
		print_el7(ntg,cx_s,ncs,s_list,pse,nce,e_list,pne,
				ncn,n_list,cx_n,cx_w1,ncw,w_list,cx_w2);
#endif /* defined(DEBUG_TRI_GRID) */

	for (sc = 0;  sc < ncs;  ++sc)
	{
		s_crx = &(T->crx_store[*(s_list+sc)]);
		s_num = s_crx->crx_num;
		sc1   = sc+1;
		for (ec = 0;  ec < nce;  ++ec)
		{
			e_crx = &(T->crx_store[*(e_list+ec)]);
			e_num = e_crx->crx_num;
			Test_crx_contiguity(s_crx,s_num,e_crx,e_num)

			ec1   = ec+1;
			triangulate_el9(ntg,e_crx,nce-ec1,e_list+ec1,jne,ncn,
				n_list,cx_n,cx_w1,ncw,w_list,cx_w2,cx_s,
					sc,s_list,s_crx,tri,t_num);
			triangulate_el1(ntg,s_crx,ncs-sc1,s_list+sc1,
					jse,ec,e_list,e_crx,tri,t_num);
			return;
		}
		for (nc = 0;  nc < ncn;  ++nc)
		{
			n_crx = &(T->crx_store[*(n_list+nc)]);
			n_num = n_crx->crx_num;
			Test_crx_contiguity(s_crx,s_num,n_crx,n_num)

			nc1   = nc+1;
			triangulate_el8(ntg,n_crx,ncn-nc1,n_list+nc1,cx_n,cx_w1,
						ncw,w_list,cx_w2,cx_s,sc,
						s_list,s_crx,tri,t_num);
			triangulate_el2(ntg,s_crx,ncs-sc1,s_list+sc1,jse,nce,
				e_list,jne,nc,n_list,n_crx,tri,t_num);
			return;
		}
		for (wc = 0;  wc < ncw;  ++wc)
		{
			w_crx = &(T->crx_store[*(w_list+wc)]);
			w_num = w_crx->crx_num;
			Test_crx_contiguity(s_crx,s_num,w_crx,w_num)

			wc1   = wc+1;
			triangulate_el3(ntg,cx_s,sc,s_list,s_crx,w_crx,
					ncw-wc1,w_list+wc1,cx_w2,tri,t_num);
			triangulate_el7(ntg,s_crx,ncs-sc1,s_list+sc1,jse,nce,
					e_list,jne,ncn,n_list,cx_n,cx_w1,
						wc,w_list,w_crx,tri,t_num);
			return;
		}
	}
	for (ec = 0;  ec < nce;  ++ec)
	{
		e_crx = &(T->crx_store[*(e_list+ec)]);
		e_num = e_crx->crx_num;
		ec1   = ec+1;
		for (nc = 0;  nc < ncn;  ++nc)
		{
			n_crx = &(T->crx_store[*(n_list+nc)]);
			n_num = n_crx->crx_num;
			Test_crx_contiguity(e_crx,e_num,n_crx,n_num)

			nc1   = nc+1;
			triangulate_el9(ntg,cx_s,ncs,s_list,jse,ec,e_list,e_crx,
				n_crx,ncn-nc1,n_list+nc1,cx_n,cx_w1,
					ncw,w_list,cx_w2,tri,t_num);
			triangulate_el1(ntg,e_crx,nce-ec1,e_list+ec1,
					jne,nc,n_list,n_crx,tri,t_num);
			return;
		}
		for (wc = 0;  wc < ncw;  ++wc)
		{
			w_crx = &(T->crx_store[*(w_list+wc)]);
			w_num = w_crx->crx_num;
			Test_crx_contiguity(e_crx,e_num,w_crx,w_num)

			wc1   = wc+1;
			triangulate_el5(ntg,cx_s,ncs,s_list,jse,ec,e_list,e_crx,
				w_crx,ncw-wc1,w_list+wc1,cx_w2,tri,t_num);
			triangulate_el5(ntg,e_crx,nce-ec1,e_list+ec1,jne,ncn,
				n_list,cx_n,cx_w1,wc,w_list,w_crx,tri,t_num);
			return;
		}
	}
	for (nc = 0;  nc < ncn;  ++nc)
	{
		n_crx = &(T->crx_store[*(n_list+nc)]);
		n_num = n_crx->crx_num;
		nc1   = nc+1;
		for (wc = 0;  wc < ncw;  ++wc)
		{
			w_crx = &(T->crx_store[*(w_list+wc)]);
			w_num = w_crx->crx_num;
			Test_crx_contiguity(n_crx,n_num,w_crx,w_num)

			wc1   = wc+1;
			triangulate_el7(ntg,cx_s,ncs,s_list,jse,nce,e_list,jne,
					nc,n_list,n_crx,w_crx,ncw-wc1,
					w_list+wc1,cx_w2,tri,t_num);
			triangulate_el3(ntg,n_crx,ncn-nc1,n_list+nc1,cx_n,cx_w1,
					wc,w_list,w_crx,tri,t_num);
			return;
		}
	}
			/* reach here if crossings dictate that */
			/* topology is indistinguishable from   */
			/* 	    no crossings case		*/

	set_element_comp(cx_s,cx_w2,side,comp);
	if (side == 'L')
	{
		st_s  = left_state(cx_s->pt);
		st_w2 = left_state(cx_w2->pt);
	}
	else
	{
		st_s  = right_state(cx_s->pt);
		st_w2 = right_state(cx_w2->pt);
	}
	set_element_states(cx_w1,cx_n,st_w1,st_n,comp);
	st_ne = ntg->states[jne];
	st_se = ntg->states[jse];

	set_tri(ntg,tri,t_num,pse,pne      ,cx_n->nd ,st_se,st_ne,st_n,
						E_SIDE,E_SIDE,I_SIDE,comp);
	set_tri(ntg,tri,t_num,pse,cx_w2->nd,cx_s->nd ,st_se,st_w2,st_s,
						I_SIDE,F_SIDE,E_SIDE,comp);
	set_tri(ntg,tri,t_num,pse,cx_w1->nd,cx_w2->nd,st_se,st_w1,st_w2,
						I_SIDE,E_SIDE,I_SIDE,comp);
	set_tri(ntg,tri,t_num,pse,cx_n->nd ,cx_w1->nd,st_se,st_n ,st_w1,
						I_SIDE,F_SIDE,I_SIDE,comp);
}		/*end triangulate_el7*/

LOCAL	void triangulate_el8(
	TRI_GRID	*ntg,
	CRXING		*cx_s1,
	int		ncs,
	int		*s_list,
	CRXING		*cx_s2,
	CRXING		*cx_e1,
	int		nce,
	int		*e_list,
	CRXING		*cx_e2,
	CRXING		*cx_n1,
	int		ncn,
	int		*n_list,
	CRXING		*cx_n2,
	LINEAR_ELEMENT	**tri,
	int		*t_num)
{
	Table		*T = table_of_interface(ntg->grid_intfc);
	COMPONENT	comp;
	CRXING		*s_crx, *e_crx, *n_crx;
	Locstate	st_s1, st_s2, st_e1, st_e2, st_n1, st_n2;
	char		side;
	int		sc, ec, nc;
	int		s_num, e_num, n_num;
	int		sc1, ec1, nc1;

#if defined(DEBUG_TRI_GRID)
	if (debugging("triangulate"))
		print_el8(ntg,cx_s1,ncs,s_list,cx_s2,cx_e1,nce,e_list,
						cx_e2,cx_n1,ncn,n_list,cx_n2);
#endif /* defined(DEBUG_TRI_GRID) */

	for (sc = 0;  sc < ncs;  ++sc)
	{
		s_crx = &(T->crx_store[*(s_list+sc)]);
		s_num = s_crx->crx_num;
		sc1   = sc+1;
		for (ec = 0;  ec < nce;  ++ec)
		{
			e_crx = &(T->crx_store[*(e_list+ec)]);
			e_num = e_crx->crx_num;
			Test_crx_contiguity(s_crx,s_num,e_crx,e_num)

			ec1   = ec+1;
			triangulate_el8(ntg,cx_s1,sc,s_list,s_crx,e_crx,nce-ec1,
						e_list+ec1,cx_e2,cx_n1,ncn,
						n_list,cx_n2,tri,t_num);
			triangulate_el3(ntg,s_crx,ncs-sc1,s_list+sc1,cx_s2,
					cx_e1,ec,e_list,e_crx,tri,t_num);
			return;
		}
		for (nc = 0;  nc < ncn;  ++nc)
		{
			n_crx = &(T->crx_store[*(n_list+nc)]);
			n_num = n_crx->crx_num;
			Test_crx_contiguity(s_crx,s_num,n_crx,n_num)

			nc1   = nc+1;
			triangulate_el3(ntg,cx_s1,sc,s_list,s_crx,n_crx,
					ncn-nc1,n_list+nc1,cx_n2,tri,t_num);
			triangulate_el8(ntg,s_crx,ncs-sc1,s_list+sc1,cx_s2,
					cx_e1,nce,e_list,cx_e2,cx_n1,nc,
					n_list,n_crx,tri,t_num);
			return;
		}
	}
	for (ec = 0;  ec < nce;  ++ec)
	{
		e_crx = &(T->crx_store[*(e_list+ec)]);
		e_num = e_crx->crx_num;
		ec1   = ec+1;
		for (nc = 0;  nc < ncn;  ++nc)
		{
			n_crx = &(T->crx_store[*(n_list+nc)]);
			n_num = n_crx->crx_num;
			Test_crx_contiguity(e_crx,e_num,n_crx,n_num)

			nc1   = nc+1;
			triangulate_el8(ntg,cx_s1,ncs,s_list,cx_s2,cx_e1,ec,
					e_list,e_crx,n_crx,ncn-nc1,
					n_list+nc1,cx_n2,tri,t_num);
			triangulate_el3(ntg,e_crx,nce-ec1,e_list+ec1,cx_e2,
					cx_n1,nc,n_list,n_crx,tri,t_num);
			return;
		}
	}
			/* reach here if crossings dictate that */
			/* topology is indistinguishable from   */
			/* 	    no crossings case		*/

	set_element_comp(cx_s1,cx_n2,side,comp);
	if (side == 'L')
	{
		st_s1 = left_state(cx_s1->pt);
		st_n2 = left_state(cx_n2->pt);
	}
	else
	{
		st_s1 = right_state(cx_s1->pt);
		st_n2 = right_state(cx_n2->pt);
	}
	set_element_states(cx_n1,cx_e2,st_n1,st_e2,comp);
	set_element_states(cx_e1,cx_s2,st_e1,st_s2,comp);

	set_tri(ntg,tri,t_num,cx_s1->nd,cx_s2->nd,cx_e1->nd,st_s1,st_s2,st_e1,
						E_SIDE,F_SIDE,I_SIDE,comp);
	set_tri(ntg,tri,t_num,cx_s1->nd,cx_e1->nd,cx_e2->nd,st_s1,st_e1,st_e2,
						I_SIDE,E_SIDE,I_SIDE,comp);
	set_tri(ntg,tri,t_num,cx_s1->nd,cx_e2->nd,cx_n1->nd,st_s1,st_e2,st_n1,
						I_SIDE,F_SIDE,I_SIDE,comp);
	set_tri(ntg,tri,t_num,cx_s1->nd,cx_n1->nd,cx_n2->nd,st_s1,st_n1,st_n2,
						I_SIDE,E_SIDE,F_SIDE,comp);
}		/*end triangulate_el8*/

LOCAL	void triangulate_el9(
	TRI_GRID	*ntg,
	CRXING		*cx_s,
	int		ncs,
	int		*s_list,
	int		jse,
	int		nce,
	int		*e_list,
	CRXING		*cx_e,
	CRXING		*cx_n1,
	int		ncn,
	int		*n_list,
	CRXING		*cx_n2,
	CRXING		*cx_w1,
	int		ncw,
	int		*w_list,
	CRXING		*cx_w2,
	LINEAR_ELEMENT	**tri,
	int		*t_num)
{
	Table		*T = table_of_interface(ntg->grid_intfc);
	COMPONENT	comp;
	CRXING		*s_crx, *e_crx, *n_crx, *w_crx;
	Locstate	st_s, st_se, st_e, st_n1, st_n2, st_w1, st_w2;
	TG_PT		*pse; 
	char		side;
	int		sc, ec, nc, wc;
	int		s_num, e_num, n_num, w_num;
	int		sc1, ec1, nc1, wc1;

	pse = &(ntg->node_points[jse]);

#if defined(DEBUG_TRI_GRID)
	if (debugging("triangulate"))
		print_el9(ntg,cx_s,ncs,s_list,pse,nce,e_list,cx_e,
			cx_n1,ncn,n_list,cx_n2,cx_w1,ncw,w_list,cx_w2);
#endif /* defined(DEBUG_TRI_GRID) */

	for (sc = 0;  sc < ncs;  ++sc)
	{
		s_crx = &(T->crx_store[*(s_list+sc)]);
		s_num = s_crx->crx_num;
		sc1   = sc+1;
		for (ec = 0;  ec < nce;  ++ec)
		{
			e_crx = &(T->crx_store[*(e_list+ec)]);
			e_num = e_crx->crx_num;
			Test_crx_contiguity(s_crx,s_num,e_crx,e_num)

			ec1   = ec+1;
			triangulate_el10(ntg,cx_s,sc,s_list,s_crx,e_crx,nce-ec1,
				e_list+ec1,cx_e,cx_n1,ncn,n_list,cx_n2,
				cx_w1,ncw,w_list,cx_w2,tri,t_num);
			triangulate_el1(ntg,s_crx,ncs-sc1,s_list+sc1,
					jse,ec,e_list,e_crx,tri,t_num);
			return;
		}
		for (nc = 0;  nc < ncn;  ++nc)
		{
			n_crx = &(T->crx_store[*(n_list+nc)]);
			n_num = n_crx->crx_num;
			Test_crx_contiguity(s_crx,s_num,n_crx,n_num)

			nc1   = nc+1;
			triangulate_el8(ntg,n_crx,ncn-nc1,n_list+nc1,cx_n2,
						cx_w1,ncw,w_list,cx_w2,cx_s,
						sc,s_list,s_crx,tri,t_num);
			triangulate_el5(ntg,s_crx,ncs-sc1,s_list+sc1,jse,nce,
				e_list,cx_e,cx_n1,nc,n_list,n_crx,tri,t_num);
			return;
		}
		for (wc = 0;  wc < ncw;  ++wc)
		{
			w_crx = &(T->crx_store[*(w_list+wc)]);
			w_num = w_crx->crx_num;
			Test_crx_contiguity(s_crx,s_num,w_crx,w_num)

			wc1   = wc+1;
			triangulate_el3(ntg,cx_s,sc,s_list,s_crx,w_crx,
					ncw-wc1,w_list+wc1,cx_w2,tri,t_num);
			triangulate_el9(ntg,s_crx,ncs-sc1,s_list+sc1,jse,nce,
					e_list,cx_e,cx_n1,ncn,n_list,cx_n2,
					cx_w1,wc,w_list,cx_w2,tri,t_num);
			return;
		}
	}
	for (ec = 0;  ec < nce;  ++ec)
	{
		e_crx = &(T->crx_store[*(e_list+ec)]);
		e_num = e_crx->crx_num;
		ec1   = ec+1;
		for (nc = 0;  nc < ncn;  ++nc)
		{
			n_crx = &(T->crx_store[*(n_list+nc)]);
			n_num = n_crx->crx_num;
			Test_crx_contiguity(e_crx,e_num,n_crx,n_num)

			nc1   = nc+1;
			triangulate_el9(ntg,cx_s,ncs,s_list,jse,ec,e_list,e_crx,
					n_crx,ncn-nc1,n_list+nc1,cx_n2,cx_w1,
						ncw,w_list,cx_w2,tri,t_num);
			triangulate_el3(ntg,e_crx,nce-ec1,e_list+ec1,cx_e,cx_n1,
						nc,n_list,n_crx,tri,t_num);
			return;
		}
		for (wc = 0;  wc < ncw;  ++wc)
		{
			w_crx = &(T->crx_store[*(w_list+wc)]);
			w_num = w_crx->crx_num;
			Test_crx_contiguity(e_crx,e_num,w_crx,w_num)

			wc1   = wc+1;
			triangulate_el5(ntg,cx_s,ncs,s_list,jse,ec,e_list,e_crx,
				w_crx,ncw-wc1,w_list+wc1,cx_w2,tri,t_num);
			triangulate_el8(ntg,e_crx,nce-ec1,e_list+ec1,cx_e,cx_n1,
						ncn,n_list,cx_n2,cx_w1,wc,
						w_list,w_crx,tri,t_num);
			return;
		}
	}
	for (nc = 0;  nc < ncn;  ++nc)
	{
		n_crx = &(T->crx_store[*(n_list+nc)]);
		n_num = n_crx->crx_num;
		nc1   = nc+1;
		for (wc = 0;  wc < ncw;  ++wc)
		{
			w_crx = &(T->crx_store[*(w_list+wc)]);
			w_num = w_crx->crx_num;
			Test_crx_contiguity(n_crx,n_num,w_crx,w_num)

			wc1   = wc+1;
			triangulate_el9(ntg,cx_s,ncs,s_list,jse,nce,e_list,cx_e,
					cx_n1,nc,n_list,n_crx,w_crx,ncw-wc1,
					w_list+wc1,cx_w2,tri,t_num);
			triangulate_el3(ntg,n_crx,ncn-nc1,n_list+nc1,cx_n2,
					cx_w1,wc,w_list,w_crx,tri,t_num);
			return;
		}
	}
			/* reach here if crossings dictate that */
			/* topology is indistinguishable from   */
			/* 	    no crossings case		*/

	set_element_comp(cx_s,cx_w2,side,comp);
	if (side == 'L')
	{
		st_s  = left_state(cx_s->pt);
		st_w2 = left_state(cx_w2->pt);
	}
	else
	{
		st_s  = right_state(cx_s->pt);
		st_w2 = right_state(cx_w2->pt);
	}
	set_element_states(cx_w1,cx_n2,st_w1,st_n2,comp);
	set_element_states(cx_n1,cx_e ,st_n1,st_e ,comp);
	st_se = ntg->states[jse];

	set_tri(ntg,tri,t_num,cx_s->nd,pse      ,cx_e->nd ,st_s,st_se,st_e,
						E_SIDE,E_SIDE,I_SIDE,comp);
	set_tri(ntg,tri,t_num,cx_s->nd,cx_e->nd ,cx_n1->nd,st_s,st_e ,st_n1,
						I_SIDE,F_SIDE,I_SIDE,comp);
	set_tri(ntg,tri,t_num,cx_s->nd,cx_n1->nd,cx_n2->nd,st_s,st_n1,st_n2,
						I_SIDE,E_SIDE,I_SIDE,comp);
	set_tri(ntg,tri,t_num,cx_s->nd,cx_n2->nd,cx_w1->nd,st_s,st_n2,st_w1,
						I_SIDE,F_SIDE,I_SIDE,comp);
	set_tri(ntg,tri,t_num,cx_s->nd,cx_w1->nd,cx_w2->nd,st_s,st_w1,st_w2,
						I_SIDE,E_SIDE,F_SIDE,comp);
}		/*end triangulate_el9*/

LOCAL	void triangulate_el10(
	TRI_GRID	*ntg,
	CRXING		*cx_s1,
	int		ncs,
	int		*s_list,
	CRXING		*cx_s2,
	CRXING		*cx_e1,
	int		nce,
	int		*e_list,
	CRXING		*cx_e2,
	CRXING		*cx_n1,
	int		ncn,
	int		*n_list,
	CRXING		*cx_n2,
	CRXING		*cx_w1,
	int		ncw,
	int		*w_list,
	CRXING		*cx_w2,
	LINEAR_ELEMENT	**tri,
	int		*t_num)
{
	Table		*T = table_of_interface(ntg->grid_intfc);
	COMPONENT	comp;
	CRXING		*s_crx, *e_crx, *n_crx, *w_crx;
	Locstate	st_s1, st_s2, st_e1, st_e2, st_n1, st_n2, st_w1, st_w2;
	char		side;
	int		sc, ec, nc, wc;
	int		s_num, e_num, n_num, w_num;
	int		sc1, ec1, nc1, wc1;

#if defined(DEBUG_TRI_GRID)
	if (debugging("triangulate"))
		print_el10(ntg,cx_s1,ncs,s_list,cx_s2,cx_e1,nce,e_list,cx_e2,
			 	cx_n1,ncn,n_list,cx_n2,cx_w1,ncw,w_list,cx_w2);
#endif /* defined(DEBUG_TRI_GRID) */

	for (sc = 0;  sc < ncs;  ++sc)
	{
		s_crx = &(T->crx_store[*(s_list+sc)]);
		s_num = s_crx->crx_num;
		sc1   = sc+1;
		for (ec = 0;  ec < nce;  ++ec)
		{
			e_crx = &(T->crx_store[*(e_list+ec)]);
			e_num = e_crx->crx_num;
			Test_crx_contiguity(s_crx,s_num,e_crx,e_num)

			ec1   = ec+1;
			triangulate_el10(ntg,cx_s1,sc,s_list,s_crx,e_crx,
				nce-ec1,e_list+ec1,cx_e2,cx_n1,ncn,n_list,
				cx_n2,cx_w1,ncw,w_list,cx_w2,tri,t_num);
			triangulate_el3(ntg,s_crx,ncs-sc1,s_list+sc1,cx_s2,
					cx_e1,ec,e_list,e_crx,tri,t_num);
			return;
		}
		for (nc = 0;  nc < ncn;  ++nc)
		{
			n_crx = &(T->crx_store[*(n_list+nc)]);
			n_num = n_crx->crx_num;
			Test_crx_contiguity(s_crx,s_num,n_crx,n_num)

			nc1   = nc+1;
			triangulate_el8(ntg,s_crx,ncs-sc1,s_list+sc1,cx_s2,
						cx_e1,nce,e_list,cx_e2,cx_n1,
						nc,n_list,n_crx,tri,t_num);
			triangulate_el8(ntg,n_crx,ncn-nc1,n_list+nc1,cx_n2,
						cx_w1,ncw,w_list,cx_w2,cx_s1,
						sc,s_list,s_crx,tri,t_num);
			return;
		}
		for (wc = 0;  wc < ncw;  ++wc)
		{
			w_crx = &(T->crx_store[*(w_list+wc)]);
			w_num = w_crx->crx_num;
			Test_crx_contiguity(s_crx,s_num,w_crx,w_num)

			wc1   = wc+1;
			triangulate_el3(ntg,cx_s1,sc,s_list,s_crx,w_crx,ncw-wc1,
					w_list+wc1,cx_w2,tri,t_num);
			triangulate_el10(ntg,s_crx,ncs-sc1,s_list+sc1,cx_s2,
				cx_e1,nce,e_list,cx_e2,cx_n1,ncn,n_list,cx_n2,
					cx_w1,wc,w_list,w_crx,tri,t_num);
			return;
		}
	}
	for (ec = 0;  ec < nce;  ++ec)
	{
		e_crx = &(T->crx_store[*(e_list+ec)]);
		e_num = e_crx->crx_num;
		ec1   = ec+1;
		for (nc = 0;  nc < ncn;  ++nc)
		{
			n_crx = &(T->crx_store[*(n_list+nc)]);
			n_num = n_crx->crx_num;
			Test_crx_contiguity(e_crx,e_num,n_crx,n_num)

			nc1   = nc+1;
			triangulate_el10(ntg,cx_s1,ncs,s_list,cx_s2,cx_e1,ec,
				e_list,e_crx,n_crx,ncn-nc1,n_list+nc1,cx_n2,
					cx_w1,ncw,w_list,cx_w2,tri,t_num);
			triangulate_el3(ntg,e_crx,nce-ec1,e_list+ec1,cx_e2,
					cx_n1,nc,n_list,n_crx,tri,t_num);
			return;
		}
		for (wc = 0;  wc < ncw;  ++wc)
		{
			w_crx = &(T->crx_store[*(w_list+wc)]);
			w_num = w_crx->crx_num;
			Test_crx_contiguity(e_crx,e_num,w_crx,w_num)

			wc1   = wc+1;
			triangulate_el8(ntg,e_crx,nce-ec1,e_list+ec1,cx_e2,
						cx_n1,ncn,n_list,cx_n2,cx_w1,
						wc,w_list,w_crx,tri,t_num);
			triangulate_el8(ntg,w_crx,ncw-wc1,w_list+wc1,cx_w2,
						cx_s1,ncs,s_list,cx_s2,cx_e1,
						ec,e_list,e_crx,tri,t_num);
			return;
		}
	}
	for (nc = 0;  nc < ncn;  ++nc)
	{
		n_crx = &(T->crx_store[*(n_list+nc)]);
		n_num = n_crx->crx_num;
		nc1   = nc+1;
		for (wc = 0;  wc < ncw;  ++wc)
		{
			w_crx = &(T->crx_store[*(w_list+wc)]);
			w_num = w_crx->crx_num;
			Test_crx_contiguity(n_crx,n_num,w_crx,w_num)

			wc1   = wc+1;
			triangulate_el10(ntg,cx_s1,ncs,s_list,cx_s2,cx_e1,nce,
				e_list,cx_e2,cx_n1,nc,n_list,n_crx,w_crx,
				ncw-wc1,w_list+wc1,cx_w2,tri,t_num);
			triangulate_el3(ntg,n_crx,ncn-nc1,n_list+nc1,cx_n2,
					cx_w1,wc,w_list,w_crx,tri,t_num);
			return;
		}
	}
			/* reach here if crossings dictate that */
			/* topology is indistinguishable from   */
			/* 	    no crossings case		*/

	set_element_comp(cx_s1,cx_w2,side,comp);
	if (side == 'L')
	{
		st_s1 = left_state(cx_s1->pt);
		st_w2 = left_state(cx_w2->pt);
	}
	else
	{
		st_s1 = right_state(cx_s1->pt);
		st_w2 = right_state(cx_w2->pt);
	}
	set_element_states(cx_w1,cx_n2,st_w1,st_n2,comp);
	set_element_states(cx_n1,cx_e2,st_n1,st_e2,comp);
	set_element_states(cx_e1,cx_s2,st_e1,st_s2,comp);

	set_tri(ntg,tri,t_num,cx_s1->nd,cx_s2->nd,cx_e1->nd,st_s1,st_s2,st_e1,
						E_SIDE,F_SIDE,I_SIDE,comp);
	set_tri(ntg,tri,t_num,cx_s1->nd,cx_e1->nd,cx_e2->nd,st_s1,st_e1,st_e2,
						I_SIDE,E_SIDE,I_SIDE,comp);
	set_tri(ntg,tri,t_num,cx_s1->nd,cx_e2->nd,cx_n1->nd,st_s1,st_e2,st_n1,
						I_SIDE,F_SIDE,I_SIDE,comp);
	set_tri(ntg,tri,t_num,cx_s1->nd,cx_n1->nd,cx_n2->nd,st_s1,st_n1,st_n2,
						I_SIDE,E_SIDE,I_SIDE,comp);
	set_tri(ntg,tri,t_num,cx_s1->nd,cx_n2->nd,cx_w1->nd,st_s1,st_n2,st_w1,
						I_SIDE,F_SIDE,I_SIDE,comp);
	set_tri(ntg,tri,t_num,cx_s1->nd,cx_w1->nd,cx_w2->nd,st_s1,st_w1,st_w2,
						I_SIDE,E_SIDE,F_SIDE,comp);
}		/*end triangulate_el10*/



/*
*			set_element_side():
*
*	A curve segment from a south border crxing to a west border crxing of
*	a mesh block defines a region to the northeast of itself.
*	This routine determines on which side of the oriented curve this
*	region is found.
*/

LOCAL	void set_element_side(
	CRXING		*cx_s,
	CRXING		*cx_w,
	char		*side)
{
	int		s_num, w_num;
	HYPER_SURF	*hs;

	s_num = cx_s->crx_num;		w_num = cx_w->crx_num;
	hs = cx_s->hs;

	if (w_num > s_num)
	{
	    if (w_num == s_num+1)
	    	*side = 'R';
	    else if (is_closed_curve(Curve_of_hs(hs)) &&
	        cx_s->end && cx_w->end && (cx_w->hs == hs))
	    	*side = 'L';
	    else
	    {
	    	(void) printf("ERROR in tri_grid side assigment:\n");
	    	print_crxings(cx_s,YES);
	    	print_crxings(cx_w,YES);
	    	*side = 'R';
	    }
	}
	else
	{
	    if (w_num == s_num-1)
	    	*side = 'L';
	    else if (is_closed_curve(Curve_of_hs(hs)) &&
	    	 cx_s->end && cx_w->end && (cx_w->hs == hs))
	    	*side = 'R';
	    else
	    {
	    	(void) printf("ERROR in tri_grid side assigment:\n");
	    	print_crxings(cx_s,YES);
	    	print_crxings(cx_w,YES);
	    	*side = 'L';
	    }
	}
}		/*end set_element_side*/



LIB_LOCAL	void set_tri(
	TRI_GRID	 *ntg,
	LINEAR_ELEMENT	 **lin,
	int		 *t_num,
	TG_PT		 *p0,
	TG_PT		 *p1,
	TG_PT		 *p2,
	Locstate	 st0,
	Locstate	 st1,
	Locstate	 st2,
	LIN_EL_FACE_TYPE sd0,
	LIN_EL_FACE_TYPE sd1,
	LIN_EL_FACE_TYPE sd2,
	COMPONENT	 comp)
{
	BLK_EL0		*blk_el0;
	int		xmax = ntg->rect_grid.gmax[0];
	int		ymax = ntg->rect_grid.gmax[1];

	if (comp == ext_comp) return; /* don't triangulate exterior */

#if defined(DEBUG_TRI_GRID)
	if (debugging("triangulate"))
		(void) printf("Entered set_tri()\n");
#endif /* defined(DEBUG_TRI_GRID) */

	
	++(*t_num);		++ntg->n_lin_els;	(*lin)->comp = comp;
	(*lin)->p[0]   = p0;	(*lin)->p[1]   = p1;	(*lin)->p[2]   = p2;
	(*lin)->s[0]   = st0;	(*lin)->s[1]   = st1;	(*lin)->s[2]   = st2;
	(*lin)->side[0] = sd0;	(*lin)->side[1] = sd1;	(*lin)->side[2] = sd2;

#if defined(DEBUG_TRI_GRID)
	if (debugging("triangulate"))
	{
		(void) printf("setting lin %p\n",(POINTER)*lin);
		print_LINEAR_ELEMENT(*lin,ntg);
	}
#endif /* defined(DEBUG_TRI_GRID) */

	++(*lin);

	if (ntg->n_lin_els == ntg->guessed_num_lin)
	{
	    LINEAR_ELEMENT	 *hold_lin_els, *tmplin, *endlin;
	    int	 i, ix, iy;

#if defined(DEBUG_TRI_GRID)
	    if (debugging("triangulate"))
	    {
	        (void) printf("Finding more lins than storage alloted\n");
	        (void) printf("number lins alloted %d  ",ntg->guessed_num_lin);
		(void) printf("allocation factor %g\n",
				  ntg->num_lin_guess_factor);
	    }
#endif /* defined(DEBUG_TRI_GRID) */

	    ntg->num_lin_guess_factor += 0.2;
	    ntg->guessed_num_lin =
		2*((int)(ntg->num_lin_guess_factor * ntg->n_crx));

#if defined(DEBUG_TRI_GRID)
	    if (debugging("triangulate"))
	    {
	    	(void) printf("Resetting storage for %d lins\n",
	    		      ntg->guessed_num_lin);
	    	(void) printf("New allocation factor %g\n",
	    		      ntg->num_lin_guess_factor);

	    	(void) printf("storage pointers: hold_lin_els %p ",
	    		      (POINTER)ntg->lin_els);
	    	(void) printf("current lin %p\n",(POINTER)*lin);
	    	(void) printf("test add hold_ + num %p\n",
	    		      (POINTER)(ntg->lin_els+ntg->n_lin_els));
	    }
#endif /* defined(DEBUG_TRI_GRID) */

	    hold_lin_els = ntg->lin_els;
	    VECTOR(ntg,lin_els,ntg->guessed_num_lin,sizeof(LINEAR_ELEMENT));

#if defined(DEBUG_TRI_GRID)
	    if (debugging("triangulate"))
	    {
	        (void) printf("new lin_els %p  test add start + num %p\n",
	    		  (POINTER)ntg->lin_els,
	    		  (POINTER)(ntg->lin_els+ntg->n_lin_els));
	    }
#endif /* defined(DEBUG_TRI_GRID) */

	    *lin = ntg->lin_els;
	    for (i = 0;  i < ntg->n_lin_els;  ++i)
	    {
	    	ntg->lin_els[i] = hold_lin_els[i];
	    	++(*lin);
	    }
#if defined(DEBUG_TRI_GRID)
	    if (debugging("triangulate")) 
	    	(void) printf("next lin storage %p\n",(POINTER)*lin);
#endif /* defined(DEBUG_TRI_GRID) */

	    endlin = hold_lin_els + (ntg->n_lin_els - 1);
	    blk_el0 = ntg->blk_els0;
	    for (iy = 0;  iy < ymax;  ++iy)
	    for (ix = 0;  ix < xmax;  ++ix, ++blk_el0)
	    {
	        tmplin = blk_el0_linear_els(blk_el0);
	        if ((tmplin == NULL) || (tmplin < hold_lin_els)
	    		             || (tmplin > endlin))
	        {
		    continue;
		}

		blk_el0_linear_els(blk_el0) =
		    ntg->lin_els + (tmplin-hold_lin_els);
	    }

	    free(hold_lin_els);
	}
#if defined(DEBUG_TRI_GRID)
	if (debugging("triangulate"))
		(void) printf("Leaving set_tri()\n");
#endif /* defined(DEBUG_TRI_GRID) */
}		/*end set_tri*/


#endif /* defined(TWOD) */
