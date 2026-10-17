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
*			triprint.c
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Storage allocation/free routines, print routines
*	for the trigrid construction
*/

#include <tri/trilocaldecs.h>

	/* LOCAL function prototypes*/
#if defined(TWOD)
LOCAL	void	print_BLK_EL1(BLK_EL1*);
LOCAL	void	print_BLK_NODE_list(BLK_NODE*);
LOCAL	void	print_BLK_NODE(BLK_NODE*);
LOCAL	void	print_PCS_ELEMENT(POINT_COMP_ST*,TRI_GRID*);
#endif /* defined(TWOD) */

EXPORT	void print_LINEAR_ELEMENT(
	LINEAR_ELEMENT	*lin,
	TRI_GRID	*grid)
{
	int		i, dim;
	char		s[20];

	if (lin == NULL)
	{
	    (void) printf("lin NULL\n");
	    return;
	}
	(void) printf("lin %p comp %d\n",(POINTER)lin,lin->comp);

	dim = grid->comp_grid.dim;
	for (i = 0; i <= dim; ++i)
	{
	    (void) printf("sides: %d: ",i);
	    switch (lin->side[i])
	    {
	    case F_SIDE:
	    	(void) printf("F");
	    	break;
	    case E_SIDE:
	    	(void) printf("E");
	    	break;
	    case I_SIDE:
	    	(void) printf("I");
	    	break;
	    default:
	    	(void) printf("not set");
	    	break;
	    }
	}
	(void) printf("\n");

	(void) printf("Points -\n");
	for (i = 0; i <= dim; ++i)
	{
	    (void) sprintf(s,"\t%d - ",i);
	    print_general_vector(s,Coords(lin->p[i]),dim,"\n");
	}
	(void) printf("End Points\n");
}		/*end print_LINEAR_ELEMENT*/

EXPORT	void print_BILINEAR_ELEMENT(
	BILINEAR_ELEMENT *bilin,
	TRI_GRID	*ntg)
{
	int		i, dim, npts;
	char		s[20];

	if (bilin == NULL)
	{
	    (void) printf("Bilinear element NULL\n");
	    return;
	}

	dim = ntg->comp_grid.dim;

	(void) printf("bilin %p comp %d\n",(POINTER)bilin,bilin->comp);

	npts = (1<<ntg->comp_grid.dim);
	(void) printf("Points -\n");
	for (i = 0; i < npts; ++i)
	{
	    (void) sprintf(s,"\t%d - ",i);
	    print_general_vector(s,Coords(bilin->p[i]),dim,"\n");
	}
	(void) printf("End Points\n");
}		/*end print_BILINEAR_ELEMENT*/

EXPORT	void print_components(
	TRI_GRID	*grid)
{

	Table *T = table_of_interface(grid->grid_intfc);
	(void) printf("\n\nTRI_GRID:  COMPONENTS\n\n");
	switch (grid->rect_grid.dim)
	{
#if defined(ONED)
	case 1:	/* TODO */
	    break;
#endif /* defined(ONED) */
#if defined(TWOD)
	case 2:
	    {
	        COMPONENT *components = T->components;
	        RECT_GRID *gr = &grid->rect_grid;
	        int	  xmax, ymax;
	        int	  ix, iy, ind0;
	        xmax = gr->gmax[0];	ymax = gr->gmax[1];
	        for (iy = ymax;  iy >= 0;  --iy)
	        {
	            ind0 = iy * (xmax+1);
	            for (ix = 0;  ix <= xmax;  ++ix)
	            {
	                (void) printf("%-5d ",components[ind0+ix]);
	            }
	            (void) printf("\n");
	        }
	    }
	    break;
#endif /* defined(TWOD) */
#if defined(THREED)
	case 3:	/* TODO */
	    break;
#endif /* defined(THREED) */
	}
}		/*end print_components*/

LIB_LOCAL	void print_trigrid_alloc_status(
	const char *mesg,
	TRI_GRID   *ntg)
{
	if (mesg == NULL || mesg[0] == '\0')
		(void) printf("\n\tAllocation status\n");
	else
		(void) printf("\n\t%s allocation status\n",mesg);
	(void) printf("\t\t%-21s %d\n","node_points",ntg->alloc.node_points);
	(void) printf("\t\t%-21s %d\n","front_points",ntg->alloc.front_points);
	(void) printf("\t\t%-21s %d\n","states",ntg->alloc.states);
	(void) printf("\t\t%-21s %d\n","rect_state_storage",
		      ntg->alloc.rect_state_storage);
	(void) printf("\t\t%-21s %d\n","blk_els0",ntg->alloc.blk_els0);
	(void) printf("\t\t%-21s %d\n","lin_els",ntg->alloc.lin_els);
	(void) printf("\t\t%-21s %d\n","bilin_els",ntg->alloc.bilin_els);
	(void) printf("\t\t%-21s %d\n","blk_els1",ntg->alloc.blk_els1);
	(void) printf("\n");
}		/*end print_trigrid_alloc_status*/

#if defined(TWOD) || defined(THREED)

/*ARGSUSED*/
LIB_LOCAL	void print_crx_store(
	TRI_GRID	*ntg,
	int		n_crx)
{
	Table *T = table_of_interface(ntg->grid_intfc);
	int		i;
	CRXING		*cross;

	if (T->crx_store == 0)
	{
	    (void) printf("crx_store[] not allocated\n");
	    return;
	}

	(void) printf("\tcrx_store[] listing:\nncrx\n");
	for (i = 0;  i < n_crx;  ++i)
	{
	    (void) printf("%d ",i);
	    cross = &(T->crx_store[i]);
	    print_crxings(cross,NO);
	}
	(void) printf("\n");
}		/*end print_crx_store*/

#endif /* defined(TWOD) || defined(THREED) */

#if defined(TWOD)

LOCAL	void print_PCS_ELEMENT(
	POINT_COMP_ST	*pcs,
	TRI_GRID	*grid)
{
	int dim = grid->comp_grid.dim;
	if (pcs == NULL)
	{
	    (void) printf("pcs NULL\n");
	    return;
	}
	print_general_vector("position - ",Coords(pcs->p),dim,"\n");
	(void) printf("pcs %p comp %d %d\n",
		(POINTER)pcs,pcs->comp[0],pcs->comp[1]);
}		/*end print_PCS_ELEMENT*/


/*
*			print_triangulation():
*
*	Prints the triangulated grid in a format suitable for input
*	to the program "tri".  To obtain the plot, use "tri" to
*	generate a "color" plot, but view the resulting plot on
*	a device not capable of doing color fills (so that the
*	polygon primitive draws the boundary of the polygon).
*	Alternatively, do a 3d mesh plot with "tri".
*
*	TODO:
*		Right now nodes with component ext_comp are shifted
*		to the boundary; this is so that superposition of
*		an interface is feasible using tri, remaplot, and mergeplot.
*		Remove this when it is possible to dictate a window to tri.
*/

EXPORT	void print_triangulation(
	FILE		*file,
	TRI_GRID	*ntg)
{
	register int	n;
	register TG_PT	*node, *p_0;

	BILINEAR_ELEMENT *bilin;
	LINEAR_ELEMENT  *lin;
	RECT_GRID	*r_gr = &ntg->rect_grid;
	boolean		binary_output = is_binary_output();
	double		tolx, toly;
	double           coords_on[MAXD];
	double		zero = 0.0;
	RECT_GRID       *gr = computational_grid(ntg->grid_intfc);

	tolx = 0.01 * r_gr->h[0]; /*TOLERANCE*/
	toly = 0.01 * r_gr->h[1]; /*TOLERANCE*/

	n = ntg->n_node_points;
	node = ntg->node_points;
	(void) fprintf(file,"NODES %d 1 %d 1\n",n,binary_output);
	if (binary_output == YES)
	{
	    while (n--)
	    {
		if ((Coords(node)[0] < r_gr->L[0] + tolx) ||
		    (Coords(node)[0] > r_gr->U[0] - tolx) ||
		    (Coords(node)[1] < r_gr->L[1] + toly) ||
		    (Coords(node)[1] > r_gr->U[1] - toly))
		{
		    nearest_boundary_point(Coords(node),coords_on,gr);
	            (void) fwrite((const void*) coords_on,sizeof(double),2,file);
	            (void) fwrite((const void*) &zero,sizeof(double),1,file);
		    ++node;
		    continue;
		}
	        (void) fwrite((const void*) Coords(node),sizeof(double),2,file);
	        (void) fwrite((const void*) &zero,sizeof(double),1,file);
		++node;
	    }
	}
	else
	{
	    while (n--)
	    {
	    	if ((Coords(node)[0] < r_gr->L[0] + tolx) ||
	    	    (Coords(node)[0] > r_gr->U[0] - tolx) ||
	    	    (Coords(node)[1] < r_gr->L[1] + toly) ||
	    	    (Coords(node)[1] > r_gr->U[1] - toly))
	    	{
	    	    nearest_boundary_point(Coords(node),coords_on,gr);
	    	    (void) fprintf(file,"%g %g 0\n",coords_on[0],coords_on[1]);
	    	    ++node;
	    	    continue;
	    	}
	    	(void) fprintf(file,"%g %g 0\n",
			       Coords(node)[0],Coords(node)[1]);
	    	++node;
	    }
	}

	p_0 = ntg->node_points;

	n = ntg->n_bilin_els;
	bilin = ntg->bilin_els;
	(void) fprintf(file,"RECTANGLES %d 0 %d 1\n",n,binary_output);
	if (binary_output == YES)
	{
	    while (n--)
	    {
	    	int ia[4];

	    	ia[0] = (int)(bilin->p[0]-p_0);
	    	ia[1] = (int)(bilin->p[1]-p_0);
	    	ia[2] = (int)(bilin->p[3]-p_0);
	    	ia[3] = (int)(bilin->p[2]-p_0);
	    	(void) fwrite((const void *) ia,sizeof(int),4,file);
	    	++bilin;
	    }
	}
	else
	{
	    while (n--)
	    {
	    	(void) fprintf(file,"%llu %llu %llu %llu\n",
	    		       ptr2ull(bilin->p[0]-p_0),
	    		       ptr2ull(bilin->p[1]-p_0),
	    		       ptr2ull(bilin->p[3]-p_0),
	    		       ptr2ull(bilin->p[2]-p_0));
	    	++bilin;
	    }
	}

	n = ntg->n_lin_els;
	lin = ntg->lin_els;
	(void) fprintf(file,"TRIANGLES %d 0 %d 1\n",n,binary_output);
	if (binary_output == YES)
	{
	    while (n--)
	    {
	    	int ia[3];

	    	ia[0] = (int)(lin->p[0]-p_0);
	    	ia[1] = (int)(lin->p[1]-p_0);
	    	ia[2] = (int)(lin->p[2]-p_0);
	    	(void) fwrite((const void *) ia,sizeof(int),3,file);
	    	++lin;
	    }
	}
	else
	{
	    while (n--)
	    {
	    	(void) fprintf(file,"%llu %llu %llu\n",
	    		       ptr2ull(lin->p[0]-p_0),
	    		       ptr2ull(lin->p[1]-p_0),
	    		       ptr2ull(lin->p[2]-p_0));
	    	++lin;
	    }
	}

	(void) fprintf(file,"END\n");
}		/*end print_triangulation*/


LIB_LOCAL	void print_side_crx_list(
	TRI_GRID	*ntg,
	char		side,
	int		n,
	int		*n_list)
{
	int		i;

	Table *T = table_of_interface(ntg->grid_intfc);
	(void) printf("%c crx list: %d crossings\n",side,n);
	for (i = 0;  i < n;  ++i)
	    print_crxings(T->crx_store + n_list[i],NO);
}		/*end print_side_crx_list*/

LIB_LOCAL	void print_seg_crx_lists(
	TRI_GRID	*ntg)
{
	int		xmax = ntg->rect_grid.gmax[0];
	int		ymax = ntg->rect_grid.gmax[1];
	int		i, j, nx, msk;
	int		*crx;
	Table		*T = table_of_interface(ntg->grid_intfc);

	if (T->seg_crx_lists == NULL)
	{
	    (void) printf("seg_crx_lists[] not allocated\n");
	    return;
	}

	(void) printf("\tseg_crx_lists[] listing:");
	(void) printf("\nseg\tn_crx\tstore\tcrx list\n");
	msk = xmax*(ymax+1) + ymax*(xmax+1);
	for (i = 0;  i < msk;  ++i)
	{
	    nx  = T->seg_crx_count[i];
	    crx = T->seg_crx_lists[i];
	    (void) printf("%d\t%d\t%p",i,nx,(POINTER)crx);
	    for (j = 0; j < nx;  ++j)
	    	(void) printf(" %d",*(crx+j));
	    (void) printf("\n");
	}
	(void) printf("\n");
}		/*end print_seg_crx_lists*/

LIB_LOCAL	void print_seg_crx_count(
	TRI_GRID	*ntg,
	int		n_crx)
{
	int		xmax = ntg->rect_grid.gmax[0];
	int		ymax = ntg->rect_grid.gmax[1];
	int		num_seg;
	int		i;
	Table		*T = table_of_interface(ntg->grid_intfc);

	if (T->seg_crx_count == NULL)
	{
	    (void) printf("seg_crx_count[] not allocated\n");
	    return;
	}

	(void) printf("\n\tseg_crx_count[] - non zero entries\n");
	(void) printf("%-14s %-14s\n","mesh seg","number crosses");
	num_seg = ymax*(xmax+1) + xmax*(ymax+1);
	for (i = 0;  i < num_seg;  ++i)
	{
	    if (T->seg_crx_count[i] == 0)
	    	continue;
	    (void) printf("%-14d %-14d\n",i,T->seg_crx_count[i]);
	}
	(void) printf("\nTotal crossings %d\n\n",n_crx);
}		/*end print_seg_crx_count*/

LIB_LOCAL	void print_seg_crx_lists_store(
	TRI_GRID	*ntg,
	int		n_crx)
{
	int		i;
	Table		*T = table_of_interface(ntg->grid_intfc);

	if (T->seg_crx_lists_store == NULL)
	{
		(void) printf("seg_crx_lists_store[] not allocated\n");
		return;
	}

	(void) printf("\tseg_crx_lists_store[] listing:\ni\tn_crx\n");
	for (i = 0;  i < n_crx;  ++i)
		(void) printf("%d\t%d\n",i,T->seg_crx_lists_store[i]);
	(void) printf("\n");
}		/*end print_seg_crx_lists_store*/

/*
*	printout of elements in triel1.c
*/

LIB_LOCAL	void print_el0(
	TRI_GRID	*ntg,
	int		ncs,
	int		*s_list,
	TG_PT		*pse,
	int		nce,
	int		*e_list,
	TG_PT		*pne,
	int		ncn,
	int		*n_list,
	TG_PT		*pnw,
	int		ncw,
	int		*w_list,
	TG_PT		*psw)
{
	(void) printf("\nelement 0: %g %g -> %g %g -> %g %g -> %g %g\n",
		      Coords(pse)[0],Coords(pse)[1],
		      Coords(pne)[0],Coords(pne)[1],
		      Coords(pnw)[0],Coords(pnw)[1],
		      Coords(psw)[0],Coords(psw)[1]);

	if (debugging("element_side_lists"))
	{
	    print_side_crx_list(ntg,'S',ncs,s_list);
	    print_side_crx_list(ntg,'E',nce,e_list);
	    print_side_crx_list(ntg,'N',ncn,n_list);
	    print_side_crx_list(ntg,'W',ncw,w_list);
	}
}		/*end print_el0*/

LIB_LOCAL	void print_el1(
	TRI_GRID	*ntg,
	CRXING		*cx_s,
	int		ncs,
	int		*s_list,
	TG_PT		*pse,
	int		nce,
	int		*e_list,
	CRXING		*cx_e)
{
	TG_PT		*ps, *pe;

	ps = cx_s->nd;		pe = cx_e->nd;
	(void) printf("\nelement 1: %g %g -> %g %g -> %g %g\n",
		      Coords(ps)[0],Coords(ps)[1],
		      Coords(pse)[0],Coords(pse)[1],
		      Coords(pe)[0],Coords(pe)[1]);
	if (debugging("element_side_lists"))
	{
	    print_side_crx_list(ntg,'S',ncs,s_list);
	    print_side_crx_list(ntg,'E',nce,e_list);
	}
}		/*end print_el1*/

LIB_LOCAL	void print_el2(
	TRI_GRID	*ntg,
	CRXING		*cx_s,
	int		ncs,
	int		*s_list,
	TG_PT		*pse,
	int		nce,
	int		*e_list,
	TG_PT		*pne,
	int		ncn,
	int		*n_list,
	CRXING		*cx_n)
{
	TG_PT		*ps, *pn;

	ps = cx_s->nd;		pn = cx_n->nd;
	(void) printf("\nelement 2: %g %g -> %g %g -> %g %g -> %g %g\n",
		      Coords(ps)[0],Coords(ps)[1],
		      Coords(pse)[0],Coords(pse)[1],
		      Coords(pne)[0],Coords(pne)[1],
		      Coords(pn)[0],Coords(pn)[1]);
	if (debugging("element_side_lists"))
	{
	    print_side_crx_list(ntg,'S',ncs,s_list);
	    print_side_crx_list(ntg,'E',nce,e_list);
	    print_side_crx_list(ntg,'N',ncn,n_list);
	}
}		/*end print_el2*/

LIB_LOCAL	void print_el3(
	TRI_GRID	*ntg,
	CRXING		*cx_s1,
	int		ncs,
	int		*s_list,
	CRXING		*cx_s2,
	CRXING		*cx_a1,
	int		nca,
	int		*a_list,
	CRXING		*cx_a2)
{
	TG_PT		*ps1, *ps2, *pa1, *pa2;

	ps1 = cx_s1->nd;	ps2 = cx_s2->nd;
	pa1 = cx_a1->nd;	pa2 = cx_a2->nd;
	(void) printf("\nelement 3: %g %g -> %g %g -> %g %g -> %g %g\n",
		      Coords(ps1)[0],Coords(ps1)[1],
		      Coords(ps2)[0],Coords(ps2)[1],
		      Coords(pa1)[0],Coords(pa1)[1],
		      Coords(pa2)[0],Coords(pa2)[1]);
	if (debugging("element_side_lists"))
	{
	    print_side_crx_list(ntg,'S',ncs,s_list);
	    print_side_crx_list(ntg,'A',nca,a_list);
	}
}		/*end print_el3*/

LIB_LOCAL	void print_el4(
	TRI_GRID	*ntg,
	CRXING		*cx_s,
	int		ncs,
	int		*s_list,
	TG_PT		*pse,
	int		nce,
	int		*e_list,
	TG_PT		*pne,
	int		ncn,
	int		*n_list,
	TG_PT		*pnw,
	int		ncw,
	int		*w_list,
	CRXING		*cx_w)
{
	TG_PT		*ps, *pw;

	ps = cx_s->nd;		pw = cx_w->nd;
	(void) printf("\nelement 4: %g %g -> %g %g -> ",
		      Coords(ps)[0],Coords(ps)[1],
		      Coords(pse)[0],Coords(pse)[1]);
	(void) printf("%g %g -> %g %g -> %g %g\n",
		      Coords(pne)[0],Coords(pne)[1],
		      Coords(pnw)[0],Coords(pnw)[1],
		      Coords(pw)[0],Coords(pw)[1]);
	if (debugging("element_side_lists"))
	{
	    print_side_crx_list(ntg,'S',ncs,s_list);
	    print_side_crx_list(ntg,'E',nce,e_list);
	    print_side_crx_list(ntg,'N',ncn,n_list);
	    print_side_crx_list(ntg,'W',ncw,w_list);
	}
}		/*end print_el4*/

LIB_LOCAL	void print_el5(
	TRI_GRID	*ntg,
	CRXING		*cx_s,
	int		ncs,
	int		*s_list,
	TG_PT		*pse,
	int		nce,
	int		*e_list,
	CRXING		*cx_e,
	CRXING		*cx_a1,
	int		nca,
	int		*a_list,
	CRXING		*cx_a2)
{
	TG_PT		*ps, *pe, *pa1, *pa2;

	ps  = cx_s->nd;		pe  = cx_e->nd;
	pa1 = cx_a1->nd;	pa2 = cx_a2->nd;
	(void) printf("\nelement 5: %g %g -> %g %g -> ",
		      Coords(ps)[0],Coords(ps)[1],
		      Coords(pse)[0],Coords(pse)[1]);
	(void) printf("%g %g -> %g %g -> %g %g\n",
		      Coords(pe)[0],Coords(pe)[1],
		      Coords(pa1)[0],Coords(pa1)[1],
		      Coords(pa2)[0],Coords(pa2)[1]);
	if (debugging("element_side_lists"))
	{
	    print_side_crx_list(ntg,'S',ncs,s_list);
	    print_side_crx_list(ntg,'E',nce,e_list);
	    print_side_crx_list(ntg,'A',nca,a_list);
	}
}		/*end print_el5*/

LIB_LOCAL	void print_el6(
	TRI_GRID	*ntg,
	CRXING		*cx_s,
	int		ncs,
	int		*s_list,
	TG_PT		*pse,
	int		nce,
	int		*e_list,
	CRXING		*cx_e,
	CRXING		*cx_n,
	int		ncn,
	int		*n_list,
	TG_PT		*pnw,
	int		ncw,
	int		*w_list,
	CRXING		*cx_w)
{
	TG_PT		*ps, *pe, *pn, *pw;

	ps = cx_s->nd;		pe = cx_e->nd;
	pn = cx_n->nd;		pw = cx_w->nd;
	(void) printf("\nelement 6: %g %g -> %g %g -> %g %g ->\n",
		      Coords(ps)[0],Coords(ps)[1],
		      Coords(pse)[0],Coords(pse)[1],
		      Coords(pe)[0],Coords(pe)[1]);
	(void) printf("           %g %g -> %g %g -> %g %g\n",
		      Coords(pn)[0],Coords(pn)[1],
		      Coords(pnw)[0],Coords(pnw)[1],
		      Coords(pw)[0],Coords(pw)[1]);
	if (debugging("element_side_lists"))
	{
	    print_side_crx_list(ntg,'S',ncs,s_list);
	    print_side_crx_list(ntg,'E',nce,e_list);
	    print_side_crx_list(ntg,'N',ncn,n_list);
	    print_side_crx_list(ntg,'W',ncw,w_list);
	}
}		/*end print_el6*/

LIB_LOCAL	void print_el7(
	TRI_GRID	*ntg,
	CRXING		*cx_s,
	int		ncs,
	int		*s_list,
	TG_PT		*pse,
	int		nce,
	int		*e_list,
	TG_PT		*pne,
	int		ncn,
	int		*n_list,
	CRXING		*cx_n,
	CRXING		*cx_w1,
	int		ncw,
	int		*w_list,
	CRXING		*cx_w2)
{
	TG_PT		*ps, *pn, *pw1, *pw2;

	ps  = cx_s->nd;		pn  = cx_n->nd;
	pw1 = cx_w1->nd;	pw2 = cx_w2->nd;
	(void) printf("\nelement 7: %g %g -> %g %g -> %g %g -> %g %g ->\n",
		      Coords(ps)[0],Coords(ps)[1],
		      Coords(pse)[0],Coords(pse)[1],
		      Coords(pne)[0],Coords(pne)[1],
		      Coords(pn)[0],Coords(pn)[1]);
	(void) printf("           %g %g -> %g %g\n",
		      Coords(pw1)[0],Coords(pw1)[1],
		      Coords(pw2)[0],Coords(pw2)[1]);
	if (debugging("element_side_lists"))
	{
	    print_side_crx_list(ntg,'S',ncs,s_list);
	    print_side_crx_list(ntg,'E',nce,e_list);
	    print_side_crx_list(ntg,'N',ncn,n_list);
	    print_side_crx_list(ntg,'W',ncw,w_list);
	}
}		/*end print_el7*/

LIB_LOCAL	void print_el8(
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
	CRXING		*cx_n2)
{
	TG_PT		*ps1, *ps2, *pe1, *pe2, *pn1, *pn2;

	ps1 = cx_s1->nd;	ps2 = cx_s2->nd;
	pe1 = cx_e1->nd;	pe2 = cx_e2->nd;
	pn1 = cx_n1->nd;	pn2 = cx_n2->nd;
	(void) printf("\nelement 8: %g %g -> %g %g -> %g %g -> %g %g->\n",
		      Coords(ps1)[0],Coords(ps1)[1],
		      Coords(ps2)[0],Coords(ps2)[1],
		      Coords(pe1)[0],Coords(pe1)[1],
		      Coords(pe2)[0],Coords(pe2)[1]);
	(void) printf("           %g %g -> %g %g\n",
		      Coords(pn1)[0],Coords(pn1)[1],
		      Coords(pn2)[0],Coords(pn2)[1]);
	if (debugging("element_side_lists"))
	{
	    print_side_crx_list(ntg,'S',ncs,s_list);
	    print_side_crx_list(ntg,'E',nce,e_list);
	    print_side_crx_list(ntg,'N',ncn,n_list);
	}
}		/*end print_el8*/

LIB_LOCAL	void print_el9(
	TRI_GRID	*ntg,
	CRXING		*cx_s,
	int		ncs,
	int		*s_list,
	TG_PT		*pse,
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
	CRXING		*cx_w2)
{
	TG_PT		*ps, *pe, *pn1, *pn2, *pw1, *pw2;

	ps  = cx_s->nd;		pe  = cx_e->nd;
	pn1 = cx_n1->nd;	pn2 = cx_n2->nd;
	pw1 = cx_w1->nd;	pw2 = cx_w2->nd;
	(void) printf("\nelement 9: %g %g -> %g %g -> %g %g ->\n",
		      Coords(ps)[0],Coords(ps)[1],
		      Coords(pse)[0],Coords(pse)[1],
		      Coords(pe)[0],Coords(pe)[1]);
	(void) printf("           %g %g -> %g %g -> %g %g -> %g %g\n",
		      Coords(pn1)[0],Coords(pn1)[1],
		      Coords(pn2)[0],Coords(pn2)[1],
		      Coords(pw1)[0],Coords(pw1)[1],
		      Coords(pw2)[0],Coords(pw2)[1]);
	if (debugging("element_side_lists"))
	{
	    print_side_crx_list(ntg,'S',ncs,s_list);
	    print_side_crx_list(ntg,'E',nce,e_list);
	    print_side_crx_list(ntg,'N',ncn,n_list);
	    print_side_crx_list(ntg,'W',ncw,w_list);
	}
}		/*end print_el9*/

LIB_LOCAL	void print_el10(
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
	CRXING		*cx_w2)
{
	TG_PT		*ps1, *ps2, *pe1, *pe2, *pn1, *pn2, *pw1, *pw2;

	ps1 = cx_s1->nd;	ps2 = cx_s2->nd;
	pe1 = cx_e1->nd;	pe2 = cx_e2->nd;
	pn1 = cx_n1->nd;	pn2 = cx_n2->nd;
	pw1 = cx_w1->nd;	pw2 = cx_w2->nd;
	(void) printf("\nelement 10: %g %g -> %g %g -> %g %g -> %g %g ->\n",
		      Coords(ps1)[0],Coords(ps1)[1],
		      Coords(ps2)[0],Coords(ps2)[1],
		      Coords(pe1)[0],Coords(pe1)[1],
		      Coords(pe2)[0],Coords(pe2)[1]);
	(void) printf("           %g %g -> %g %g -> %g %g -> %g %g\n",
		      Coords(pn1)[0],Coords(pn1)[1],
		      Coords(pn2)[0],Coords(pn2)[1],
		      Coords(pw1)[0],Coords(pw1)[1],
		      Coords(pw2)[0],Coords(pw2)[1]);
	if (debugging("element_side_lists"))
	{
	    print_side_crx_list(ntg,'S',ncs,s_list);
	    print_side_crx_list(ntg,'E',nce,e_list);
	    print_side_crx_list(ntg,'N',ncn,n_list);
	    print_side_crx_list(ntg,'W',ncw,w_list);
	}
}		/*end print_el10*/

EXPORT	void print_blk_els0(
	TRI_GRID	*ntg)
{
	BLK_EL0		*blk_el0;
	int		iy, ix;
	int		xmax = ntg->rect_grid.gmax[0];
	int		ymax = ntg->rect_grid.gmax[1];

	if (ntg->alloc.blk_els0 == 0)
	{
	    (void) printf("blk_els0 not ft_assigned\n");
	    return;
	}

	(void) printf("\telements in block list 0 %p\n",
		      (POINTER)ntg->blk_els0);
	(void) printf("iy\tix\tnum_elements\n");
	blk_el0 = ntg->blk_els0;
	for (iy = 0;  iy < ymax;  ++iy)
	{
	    for (ix = 0;  ix < xmax;  ++ix, ++blk_el0)
	    {
	    	(void) printf("%d\t%d\t",iy,ix);
	    	print_BLK_EL0(blk_el0,ntg);
	    }
	}
	(void) printf("\n\n");
}		/*end print_blk_els0*/


EXPORT	void print_blk_els1(
	TRI_GRID	*ntg)
{
	BLK_EL1		*blk_el1;
	int		xmax = ntg->rect_grid.gmax[0];
	int		ymax = ntg->rect_grid.gmax[1];
	int		iy, ix;

	if (ntg->alloc.blk_els1 == 0)
	{
	    (void) printf("blk_els1 not ft_assigned\n");
	    return;
	}

	(void) printf("\telements in block list 1 %p\n",
		      (POINTER)ntg->blk_els1);
	(void) printf("iy\tix\tinterior_node\n");
	blk_el1 = ntg->blk_els1;
	for (iy = 0;  iy < ymax;  ++iy)
	{
	    for (ix = 0;  ix < xmax;  ++ix, ++blk_el1)
	    {
	    	(void) printf("%d\t%d\t",iy,ix);
	    	print_BLK_EL1(blk_el1);
	    }
	}
	(void) printf("\n\n");
}		/*end print_blk_els1*/

LIB_LOCAL	void print_BLK_EL0(
	BLK_EL0		*blk,
	TRI_GRID	*ntg)
{
	int		k;

	if (blk_el0_is_bilinear(blk))
	{
	    print_BILINEAR_ELEMENT(blk_el0_bilinear_el(blk),ntg);
	}
	else if (ntg->_locate_on_trigrid == tg_build)
	{
	    (void) printf("%d\n",num_pcs_els_in_blk(blk));
	    for (k = 0;  k < num_pcs_els_in_blk(blk);  ++k)
	        print_PCS_ELEMENT(blk_el0_pcs_els(blk) + k,ntg);
	}
	else
	{
	    (void) printf("%d\n",num_lin_els_in_blk(blk));
	    for (k = 0;  k < num_lin_els_in_blk(blk);  ++k)
	        print_LINEAR_ELEMENT(blk_el0_linear_els(blk) + k,ntg);
	}
	(void) printf("\n");
}		/*end print_BLK_EL0*/


LOCAL	void print_BLK_EL1(
	BLK_EL1		*blk)
{
	int		num_nodes;

	num_nodes = blk->interior_node;

	(void) printf("%d\n",num_nodes);
	print_BLK_NODE_list(blk->blk_node);
}		/*end print_BLK_EL1*/

LOCAL	void print_BLK_NODE_list(
	BLK_NODE	*start)
{
	BLK_NODE	*blkn;

	if (start == NULL)
		(void) printf("BLK_NODE list unft_assigned\n\n");
	else
	{
		(void) printf("BLK_NODE list - \n");
		for (blkn = start;  blkn != NULL;  blkn = blkn->next)
			print_BLK_NODE(blkn);
		(void) printf("\n");
	}
}		/*end print_BLK_NODE_list*/

LOCAL	void print_BLK_NODE(
	BLK_NODE	*blkn)
{
	if (blkn == NULL)
		(void) printf("blkn - NULL\n");
	else
	{
		(void) printf("blkn %p  prev %p  next %p\n",(POINTER)blkn,
			      (POINTER)blkn->prev,(POINTER)blkn->next);
		(void) printf("node - \n");	print_node(blkn->n);
		(void) printf("nd: %g %g\n",
			      Coords(blkn->nd)[0],Coords(blkn->nd)[1]);
	}
}		/*end print_BLK_NODE*/

#endif /* defined(TWOD) */
