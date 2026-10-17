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
*				imkcurve.c:
*
*       Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Containing function of rebuilding interface within a mesh block.
*
*/


#define DEBUG_STRING "i_make_curve"

#include <intfc/int.h>

struct  _EG_CRX_2D {
        BBI_POINT ***x_crx;
        BBI_POINT ***y_crx;
        BBI_POINT ***x_curve_crx;
        BBI_POINT ***y_curve_crx;
        BBI_POINT ***node_crx;
        BBI_POINT *crx_store;
        COMPONENT **comp;
        CURVE     **nodes;
        int       num_nodes;
};
typedef struct _EG_CRX_2D EG_CRX_2D;

struct _BLK_CRX_2D {
        COMPONENT       **comp;
        int             **ix;
        int             **iy;
        int             num_comps;
        int             nv[4];
        COMPONENT       comps[4];
        BBI_POINT       ***crx;
        BBI_POINT       *node_crx;
        BBI_POINT       *crx_store;
        BLK_INFO        *blk_info;
        BLK_TYPE        blk_type;
};
typedef struct _BLK_CRX_2D BLK_CRX_2D;

LOCAL	boolean onfront_block2d(int,int,const EG_CRX_2D*);
LOCAL	boolean increment_theta(double*,double,double,double,ELLIP_PARAMS*,
			double*,double);
LOCAL	void reset_domain_comp2d(COMPONENT**,RECT_GRID*);
LOCAL   void assign_two_comp_domain2d(double (*func)(POINTER,double*),POINTER,
                        COMPONENT**,RECT_GRID*,COMPONENT,COMPONENT);
LOCAL	void make_grid_curves(BLK_CRX_2D*,const EG_CRX_2D*,int*,BOND**,int*);
LOCAL 	void assign_blk_crx2d(BLK_CRX_2D*,int,int,const EG_CRX_2D*);
LOCAL	void connect_adj_blk(BLK_BOND*,BLK_BOND*);
LOCAL	int  count_crx_through_comp2d(int*,COMPONENT**);
LOCAL   int  install_grid_crx2d(double (*func)(POINTER,double*),POINTER,
                        EG_CRX_2D*,RECT_GRID*);
LOCAL	int  construct_comp2_blk2d(BLK_CRX_2D*,BLK_BOND*);
LOCAL	BLK_CRX_2D *alloc_blk_crx2d(boolean);

/*	Initialization for level function parameters */
LOCAL	POINTER init_line_params(RECT_GRID*);
LOCAL	POINTER init_multi_circle_params(RECT_GRID*);
LOCAL	POINTER init_sine_wave_params(RECT_GRID*);
LOCAL	POINTER init_test_disk_params(RECT_GRID*);

/*	Level functions for curves */

LOCAL   double line_func(POINTER,double*);

#define		MAX_NUM_SEGMENTS		100


EXPORT CURVE *make_elliptic_curve(
	ELLIP_PARAMS *ellip,
	COMPONENT compin,
	COMPONENT compout,
	double space)
{
	int	    i;
	boolean	    done;
	ORIENTATION nor_or = ellip->nor_orient;
	int	    closed = ellip->closed;
	double	    theta[MAXD-1];
	RECT_GRID   *gr = ellip->gr;
	double	    *h = gr->h, *L = gr->L, *U = gr->U;
	double	    coords[MAXD];
	double	    sgn;
	double	    ThetaS = ellip->ThetaS[0], ThetaE = ellip->ThetaE[0];
	CURVE	    *cur;
	NODE	    *ns, *ne;
	COMPONENT   l_comp,r_comp;

	if (nor_or == NEGATIVE_ORIENTATION)
	{
	    sgn = -1.0;
	    l_comp = compout;
	    r_comp = compin;
	}
	else
	{
	    sgn = 1.0;
	    l_comp = compin;
	    r_comp = compout;
	}
	if (closed == YES)
	{
	    if (nor_or == NEGATIVE_ORIENTATION)
	    {
		ThetaS = normalized_angle(ThetaS);
		ThetaE = ThetaS - 2.0*PI;
	    }
	    else
	    {
		ThetaS = normalized_angle(ThetaS);
		ThetaE = ThetaS + 2.0*PI;
	    }
	    coords_on_ellips(ellip->ThetaS,coords,ellip);
	    ns = make_node(Point(coords));
	    ne = ns;
	}
	else
	{
	}
	cur = make_curve(l_comp,r_comp,ns,ne);

	done = NO;
	theta[0] = ThetaS;
	while (done == NO)
	{
	    done = increment_theta(theta,ThetaS,ThetaE,sgn,ellip,h,space);
	    if (done == YES)
		break;
	    coords_on_ellips(theta,coords,ellip);
	    if (insert_point_in_bond(Point(coords),cur->last,cur) !=
		FUNCTION_SUCCEEDED)
	    {
	        screen("ERROR in g_make_ellipse(), "
		       "insert_point_in_bond() failed\n");
	        clean_up(ERROR);
	    }
	}
	return cur;
}	/* end make_elliptic_curve */


EXPORT	void coords_on_ellips(
	double		*theta,
	double		*coords,
	ELLIP_PARAMS	*ellip)
{
	double		*cen = ellip->cen, *rad = ellip->rad;
	FOURIER_POLY	*fpoly = ellip->fpoly;
	LEGENDRE_POLY	*lpoly = ellip->lpoly;
	int		dim = ellip->dim;
	double		Dr[MAXD];
	double		er[3];
	double		r;
	int		i, j;

	switch (dim)
	{
	case 2:
	    er[0] = cos(theta[0]);
	    er[1] = sin(theta[0]);
	    r = 1.0/hypot(er[0]/rad[0],er[1]/rad[1]);
	    break;
#if defined(THREED)
	case 3:
	    er[0] = cos(theta[0])*sin(theta[1]);
	    er[1] = sin(theta[0])*sin(theta[1]);
	    er[2] = cos(theta[1]);
	    r = 1.0/sqrt(sqr(er[0]/rad[0])+sqr(er[1]/rad[1])+sqr(er[2]/rad[2]));
	    break;
#endif /* defined(THREED) */
	default:
	    r = ERROR_FLOAT;
	    screen("ERROR in coords_on_ellips(), "
	           "invalid dimension %d\n",dim);
	    clean_up(ERROR);
	}
	if (fpoly != NULL)
	    r += fourier_poly(theta,fpoly);
	if ((dim == 2) && (lpoly != NULL))
	    r += legendre_poly(er[1],lpoly);

	for (i = 0; i < dim; i++)
            Dr[i] = 1.0*r*er[i];
	for (i = 0; i < dim; i++)
	    coords[i] = cen[i] + Dr[i];
}	/*end coords_on_ellips */


/*
*			increment_theta():
*
*	Computes the incremented theta in the polar coordinated
*	of the z = constant cross section of an ellipsoid.
*	The difference new_theta - theta is determined so that
*	the displacement of the two consecutive points on the
*	ellipsoid are approximately separated by the distance
*	space in the scaled metric with scale vector h.
*	This function assumes that if sgn and theta_e - theta_s
*	have the same algebraic sign.
*/

LOCAL	boolean increment_theta(
	double		*theta,
	double		theta_s,
	double		theta_e,
	double		sgn,
	ELLIP_PARAMS	*ellip,
	double		*h,
	double		space)
{
	double		d_theta, dl;
	double		new_theta;
	double		rad[MAXD], r;
	double		er[2];
	boolean		done;
	rad[0] = ellip->rad[0];	rad[1] = ellip->rad[1];
	dl = space*hypot(h[0],h[1]);
	er[0] = cos(*theta);	er[1] = sin(*theta);
	r = 1.0/hypot(er[0]/rad[0],er[1]/rad[1]);
	d_theta = dl/(r*r*r*hypot(er[0]/sqr(rad[0]),er[1]/sqr(rad[1])));

	new_theta = theta[0] + sgn*d_theta;
	done = NO;
	if (!Between(new_theta,theta_s,theta_e))
	{
	    new_theta = theta_e;
	    done = YES;
	}

	*theta = new_theta;
	return done;
}		/*end increment_theta*/

EXPORT double multi_sine_mode_func(
	POINTER func_params, 
        double *coords)
{
	FOURIER_POLY *sine_params = (FOURIER_POLY*)func_params;
	double z0,dist;
	double *nu,A,phase;	
	double *L = sine_params->L;
	double *U = sine_params->U;
	int i,j;
	int dim = sine_params->dim;

	
	z0 = sine_params->z0;
	for (i = 0; i < sine_params->num_modes; ++i)
	{
	    A = sine_params->A[i];
	    nu = sine_params->nu[i];
	    phase = sine_params->phase[i];
	    for (j = 0; j < dim-1; ++j)
	    {
		phase += nu[j]*(coords[j] - L[j])*2.0*PI/(U[j] - L[j]);
	    }
	    z0 += A*sin(phase);
	}
	dist = coords[dim-1] - z0;
	return dist;
}	/* end multi_sine_mode_func */


/*******************************************************************
*       This function make a curve described by the function       *
*       func = 0. The negative side of the curve has right_comp    *
*       and the positive side of the curve has left_comp.          *
*******************************************************************/

EXPORT CURVE **make_level_curves(
	RECT_GRID   *rgr,
	INTERFACE   *intfc,
	COMPONENT   left_comp,
	COMPONENT   right_comp,
	double       (*func)(POINTER,double*),
        POINTER     func_params,
	boolean	    use_dual_grid,
	int	    *num_segs)
{
	int		i,num_crx,np,*gmax;
	RECT_GRID	dual_gr,*grid;
	CURVE		*curve;
	EG_CRX_2D	Eg_crx;
	BLK_INFO	blk_info;
	static BLK_CRX_2D  *blk_crx;
	double		coords[2] = {0.0,0.0};
	NODE		*ns,*ne;
	BOND 		*b,*first[MAX_NUM_SEGMENTS];
        INTERFACE       *save_intfc;
	static 	CURVE 	**curves;

        save_intfc = current_interface();
        set_current_interface(intfc);

	if (blk_crx == NULL)
	    blk_crx = alloc_blk_crx2d(NO);

	zero_scalar(&Eg_crx,sizeof(EG_CRX_2D));
	if (use_dual_grid)
	{
	    set_grid_for_surface_construction(&dual_gr,rgr);
	    grid = &dual_gr;
	}
	else
	    grid = rgr;
	gmax = grid->gmax;
	bi_array(&Eg_crx.comp,gmax[0]+1,gmax[1]+1,sizeof(COMPONENT));

	reset_domain_comp2d(Eg_crx.comp,grid);
	assign_two_comp_domain2d(func,func_params,Eg_crx.comp,
				grid,right_comp,left_comp);
	num_crx = count_crx_through_comp2d(gmax,Eg_crx.comp);
	if (num_crx == 0)
	{
	    intfc->default_comp = Eg_crx.comp[0][0];
	    free_these(1,Eg_crx.comp);
	    return NULL;
	}

	bi_array(&Eg_crx.x_crx,gmax[0],gmax[1]+1,sizeof(BBI_POINT*));
        bi_array(&Eg_crx.y_crx,gmax[0]+1,gmax[1],sizeof(BBI_POINT*));
        uni_array(&Eg_crx.crx_store,num_crx,sizeof(BBI_POINT));

	num_crx = install_grid_crx2d(func,func_params,&Eg_crx,grid);

	ns = make_node(Point(coords));
	ne = make_node(Point(coords));
	curve = make_curve(left_comp,right_comp,ns,ne);

	curve->first = curve->last = NULL;

	blk_info.num_surfs = 1;
	uni_array(&blk_info.curves,1,sizeof(CURVE*));
	uni_array(&blk_info.cur_bonds,1,sizeof(BOND*));

	blk_info.cur_bonds[0] = NULL;
	blk_info.curves[0] = curve;
	curve->num_points = 0;

	blk_crx->comps[0] = right_comp;
	blk_crx->comps[1] = left_comp;
	blk_crx->blk_info = &blk_info;

	for (i = 0; i < num_crx; ++i)
	    Eg_crx.crx_store[i].c = curve;

	make_grid_curves(blk_crx,&Eg_crx,gmax,first,num_segs);
	curve->num_points = 2;
	for (b = curve->first; b->next != NULL; b = b->next)
	    curve->num_points++;
	curve->last = b;
	if (curve->start->posn == curve->end->posn)
	{
	    change_node_of_curve(curve,NEGATIVE_ORIENTATION,curve->start);
	    delete_node(ne);
	}
	if (*num_segs != 0)
	{
	    CURVE *c;
	    uni_array(&curves,*num_segs,sizeof(CURVE*));
	    for (i = 0; i < *num_segs; ++i)
	    {
	    	if (first[i]->start == curve->start->posn)
		{
		    curves[i] = curve;
		    continue;
		}
		np = 2;
		for (b = first[i]; b->next != NULL; b = b->next) ++np;
		if (b->end == first[i]->start)
		{
		    ns = ne = make_node(first[i]->start);
		}
		else
		{
		    ns = make_node(first[i]->start);
		    ne = make_node(b->end);
		}
		c = make_curve(left_comp,right_comp,ns,ne);
		curves[i] = c;
		c->first = first[i];
		c->last = b;
		c->num_points = np;
	    }
	}

	reset_intfc_num_points(intfc);
	free_these(3,Eg_crx.x_crx,Eg_crx.y_crx,Eg_crx.crx_store);
	free_these(3,Eg_crx.comp,blk_info.curves,blk_info.cur_bonds);
	set_current_interface(save_intfc);
	if (*num_segs >= 1) return curves;
	else return NULL;
}	/* end make_level_surface */


/**********************************************************************
*	This function reset components of the domain to NO_COMP       *
**********************************************************************/

LOCAL	void reset_domain_comp2d(
	COMPONENT **comp,
	RECT_GRID *gr)
{
	int i,j;
	int *gmax = gr->gmax;

	for (i = 0; i <= gmax[0]; ++i)
	{
	    for (j = 0; j <= gmax[1]; ++j)
	    {
	    	comp[i][j] = NO_COMP;
	    }
	}
}	/* end reset_domain_comp */


LOCAL	int count_crx_through_comp2d(
	int *gmax,
	COMPONENT **comp)
{
	int i,j,num_crx,num_comp;

	num_crx = 0;
	for (i = 0; i <= gmax[0]; ++i)
	{
	    for (j = 0; j <= gmax[1]; ++j)
	    {
		if (i != gmax[0])
		{
		    if (comp[i][j] != comp[i+1][j])
			    ++num_crx;
		}
		if (j != gmax[1])
		{
		    if (comp[i][j] != comp[i][j+1])
			    ++num_crx;
		}
	    }
	}
	return num_crx;
}	/* end count_crx_through_comp */


LOCAL	void assign_two_comp_domain2d(
	double (*func)(POINTER,double*),
	POINTER func_params,
	COMPONENT **comp,
	RECT_GRID *gr,
	COMPONENT left_comp,
	COMPONENT right_comp)
{
	int i,j,k;
	int *gmax = gr->gmax;
	double *L = gr->L;
	double *h = gr->h;
	double coords[MAXD];

	for (i = 0; i <= gmax[0]; ++i)
	{
	    coords[0] = L[0] + i*h[0];
	    for (j = 0; j <= gmax[1]; ++j)
	    {
	    	coords[1] = L[1] + j*h[1];
		if (comp[i][j] != NO_COMP) continue;
		if ((*func)(func_params,coords) > 0)
		    comp[i][j] = left_comp;
		else 
		    comp[i][j] = right_comp;
	    }
	}
}	/* end assign_two_comp_domain */


LOCAL	int install_grid_crx2d(
	double (*func)(POINTER,double*),
        POINTER func_params,
	EG_CRX_2D *eg_crx,
	RECT_GRID *grid)
{
	double coords1[MAXD];
	double coords2[MAXD];
	double crds_crx[MAXD];
	double *L = grid->L;
	double *h = grid->h;
	int *gmax = grid->gmax;
	int dim = grid->dim;
	int i,j,n_crx;
	BBI_POINT ***x_crx = eg_crx->x_crx;
	BBI_POINT ***y_crx = eg_crx->y_crx;
	BBI_POINT *crx_store = eg_crx->crx_store;
	COMPONENT **comp = eg_crx->comp;

	n_crx = 0;

	/* install x-crossings */

	for (j = 0; j <= gmax[1]; ++j)
	{
	    coords1[1] = coords2[1] = L[1] + j*h[1];
	    for (i = 0; i < gmax[0]; ++i)
	    {
		x_crx[i][j] = NULL;
		if (comp[i][j] != comp[i+1][j])
		{
		    coords1[0] = L[0] + i*h[0];
		    coords2[0] = L[0] + (i+1)*h[0];
		    if (! grid_line_crx_in_dir(func,func_params,
				dim,coords1,coords2,crds_crx,0))
		    {
			screen("ERROR: in install_grid_crx(), no x-crxing!");
			clean_up(ERROR);
		    }
		    if (crds_crx[0] - coords1[0] < 0.004*h[0])
			crds_crx[0] = coords1[0] + 0.004*h[0];
		    if (coords2[0] - crds_crx[0] < 0.004*h[0])
			crds_crx[0] = coords2[0] - 0.004*h[0];
		    crx_store[n_crx].p = Point(crds_crx);
		    x_crx[i][j] = &crx_store[n_crx++];
		}
	    }
	}

	/* install y-crossings */

	for (i = 0; i <= gmax[0]; ++i)
	{
	    coords1[0] = coords2[0] = L[0] + i*h[0];
	    for (j = 0; j < gmax[1]; ++j)
	    {
		y_crx[i][j] = NULL;
		if (comp[i][j] != comp[i][j+1])
		{
		    coords1[1] = L[1] + j*h[1];
		    coords2[1] = L[1] + (j+1)*h[1];
		    if (!grid_line_crx_in_dir(func,func_params,
			dim,coords1,coords2,crds_crx,1))
		    {
			screen("ERROR: in install_grid_crx(), no y-crxing!");
			clean_up(ERROR);
		    }
		    if (crds_crx[1] - coords1[1] < 0.004*h[1])
			crds_crx[1] = coords1[1] + 0.004*h[1];
		    if (coords2[1] - crds_crx[1] < 0.004*h[1])
			crds_crx[1] = coords2[1] - 0.004*h[1];
		    crx_store[n_crx].p = Point(crds_crx);
		    y_crx[i][j] = &crx_store[n_crx++];
		}
	    }
	}

	return n_crx;
}	/* end install_grid_crx */


LOCAL	void make_grid_curves(
	BLK_CRX_2D      *blk_crx,
	const EG_CRX_2D *eg_crx,
	int             *gmax,
	BOND		**first,
	int		*num_segs)
{
	int i,j,k,num_blk;
	BLK_BOND *bm,***blk_mem,*blk_mem_store;
	BOND *b;

	num_blk = 0;
	for (i = 0; i < gmax[0]; ++i)
	{
	    for (j = 0; j < gmax[1]; ++j)
	    {
		    if (onfront_block2d(i,j,eg_crx))
			++num_blk;
	    }
	}
	bi_array(&blk_mem,gmax[0],gmax[1],sizeof(BLK_BOND*));
        uni_array(&blk_mem_store,num_blk,sizeof(BLK_BOND));

	num_blk = 0;
	for (i = 0; i < gmax[0]; ++i)
	{
	    for (j = 0; j < gmax[1]; ++j)
	    {
		if (onfront_block2d(i,j,eg_crx))
		{
		    bm = blk_mem[i][j] = &blk_mem_store[num_blk++];
		    bm->blk_info = blk_crx->blk_info;
		    assign_blk_crx2d(blk_crx,i,j,eg_crx);
		    switch (blk_crx->blk_type)
		    {
		    case COMP2_BLOCK:
			construct_comp2_blk2d(blk_crx,bm);
			break;
		    default:
			screen("UNKNOWN BLOCK: code needed!\n");
			clean_up(ERROR);
		    }
		    if (i != 0 && blk_mem[i-1][j] != NULL)
			connect_adj_blk(blk_mem[i-1][j],bm);
		    if (j != 0 && blk_mem[i][j-1] != NULL)
			connect_adj_blk(blk_mem[i][j-1],bm);
		}
	    }
	}
	*num_segs = 0;
	for (i = 0; i < num_blk; ++i)
	{
	    int nc = blk_mem_store[i].num_curves;
	    for (j = 0; j < nc; ++j)
	    {
	    	int nb = blk_mem_store[i].num_bonds[j];
		for (k = 0; k < nb; ++k)
		{
		    if (blk_mem_store[i].bonds[j][k]->prev == NULL)
		    	first[(*num_segs)++] = blk_mem_store[i].bonds[j][k];
		}
	    }
	}

	free_these(2,blk_mem,blk_mem_store);
}	/* end make_grid_curves */


LOCAL	boolean onfront_block2d(
	int          i,
	int          j,
	const EG_CRX_2D *eg_crx)
{
	int ii,jj;

	for (ii = 0; ii < 2; ++ii)
	{
	    for (jj = 0; jj < 2; ++jj)
	    {
		if (eg_crx->x_crx[i][j+ii] != NULL)
		    return YES;
		if (eg_crx->y_crx[i+jj][j] != NULL)
		    return YES;
	    }
	}
	return NO;
}	/* end onfront_block */


LOCAL	BLK_CRX_2D *alloc_blk_crx2d(
	boolean alloc_BBI_POINT)
{
	BLK_CRX_2D *blk_crx;

	scalar(&blk_crx,sizeof(BLK_CRX));
	bi_array(&blk_crx->comp,2,2,sizeof(COMPONENT));
	bi_array(&blk_crx->ix,2,2,sizeof(int));
	bi_array(&blk_crx->iy,2,2,sizeof(int));
	bi_array(&blk_crx->crx,2,2,sizeof(BBI_POINT*));
	if (alloc_BBI_POINT)
	{
	    int i,j,k,num_crx = 0;
	    uni_array(&blk_crx->crx_store,5,sizeof(BBI_POINT));
	    for (i = 0; i < 2; ++i)
	    for (j = 0; j < 2; ++j)
	    	blk_crx->crx[i][j] = &blk_crx->crx_store[num_crx++];
	    blk_crx->node_crx = &blk_crx->crx_store[num_crx++];
	}
	return blk_crx;
}	/*end alloc_blk_crx*/


LOCAL void assign_blk_crx2d(
	BLK_CRX_2D   *blk_crx,
	int          i,
	int          j,
	const EG_CRX_2D *eg_crx)
{
	int ic,num_curve_crx,ii,jj;
	BBI_POINT ***x_crx = eg_crx->x_crx;
	BBI_POINT ***y_crx = eg_crx->y_crx;
	COMPONENT c,**comp = eg_crx->comp;

	blk_crx->num_comps = 0;
	for (ii = 0; ii < 2; ++ii)
	{
	    for (jj = 0; jj < 2; ++jj)
	    {
		c = comp[i+ii][j+jj];
		blk_crx->ix[ii][jj] = ii;
		blk_crx->iy[ii][jj] = jj;
		for (ic = 0; ic < blk_crx->num_comps; ++ic)
		{
		    if (c == blk_crx->comps[ic])
		    {
			++blk_crx->nv[ic];
			break;
		    }
		}
		if (ic == blk_crx->num_comps)
		{
		    blk_crx->comps[ic] = c;
		    blk_crx->nv[ic] = 1;
		    ++blk_crx->num_comps;
		}
		blk_crx->comp[ii][jj] = c;
	    }
	    blk_crx->crx[0][ii] = x_crx[i][j+ii];
	    blk_crx->crx[1][ii] = y_crx[i+ii][j];
	}
	for (ii = 0; ii < blk_crx->num_comps-1; ++ii)
	{
	    for (jj = 1; jj < blk_crx->num_comps; ++jj)
	    {
	    	if (blk_crx->nv[ii] > blk_crx->nv[jj])
		{
		    int nv_tmp;
		    COMPONENT c_tmp;
		    nv_tmp = blk_crx->nv[ii];
		    blk_crx->nv[ii] = blk_crx->nv[jj];
		    blk_crx->nv[jj] = nv_tmp;
		    c_tmp = blk_crx->comps[ii];
		    blk_crx->comps[ii] = blk_crx->comps[jj];
		    blk_crx->comps[jj] = c_tmp;
		}
	    }
	}
	blk_crx->blk_type = COMP2_BLOCK;
	return;
}	/* end assign_blk_crx */


LOCAL	int construct_comp2_blk2d(
	BLK_CRX_2D *blk_crx,
	BLK_BOND *blk_mem)
{
	int       i,j;
	COMPONENT **comp = blk_crx->comp;
	int       num_crx;
	CURVE 	  *c = blk_crx->blk_info->curves[0];
	COMPONENT left_c  = negative_component(c);
	COMPONENT right_c = positive_component(c);
	FT_POINT	  *p1,*p2;

	if (blk_crx->nv[0] == 1)
	{
	    blk_mem->num_bonds[0] = 1;
	    if (comp[0][0] == blk_crx->comps[0])
	    {
	    	p1 = blk_crx->crx[0][0]->p;
		p2 = blk_crx->crx[1][0]->p;
		blk_mem->bonds[0][0] = (comp[0][0] == left_c) ?
			Bond(p1,p2) : Bond(p2,p1);
	    }
	    else if (comp[0][1] == blk_crx->comps[0])
	    {
	    	p1 = blk_crx->crx[1][0]->p;
		p2 = blk_crx->crx[0][1]->p;
		blk_mem->bonds[0][0] = (comp[0][1] == left_c) ?
			Bond(p1,p2) : Bond(p2,p1);
	    }
	    else if (comp[1][0] == blk_crx->comps[0])
	    {
	    	p1 = blk_crx->crx[1][1]->p;
		p2 = blk_crx->crx[0][0]->p;
		blk_mem->bonds[0][0] = (comp[1][0] == left_c) ?
			Bond(p1,p2) : Bond(p2,p1);
	    }
	    else if (comp[1][1] == blk_crx->comps[0])
	    {
	    	p1 = blk_crx->crx[0][1]->p;
		p2 = blk_crx->crx[1][1]->p;
		blk_mem->bonds[0][0] = (comp[1][1] == left_c) ?
			Bond(p1,p2) : Bond(p2,p1);
	    }
	}
	else if (blk_crx->nv[0] == 2)
	{
	    if (comp[0][0] == comp[1][0])
	    {
	    	blk_mem->num_bonds[0] = 1;
	    	p1 = blk_crx->crx[1][1]->p;
	    	p2 = blk_crx->crx[1][0]->p;
		blk_mem->bonds[0][0] = (comp[0][0] == left_c) ?
			Bond(p1,p2) : Bond(p2,p1);
	    }
	    else if (comp[0][0] == comp[0][1])
	    {
	    	blk_mem->num_bonds[0] = 1;
	    	p1 = blk_crx->crx[0][0]->p;
	    	p2 = blk_crx->crx[0][1]->p;
		blk_mem->bonds[0][0] = (comp[0][0] == left_c) ?
			Bond(p1,p2) : Bond(p2,p1);
	    }
	    else if (comp[0][0] == comp[1][1])
	    {
	    	blk_mem->num_bonds[0] = 2;
	    	p1 = blk_crx->crx[0][0]->p;
		p2 = blk_crx->crx[1][0]->p;
		blk_mem->bonds[0][0] = (comp[0][0] == left_c) ?
			Bond(p1,p2) : Bond(p2,p1);
	    	p1 = blk_crx->crx[0][1]->p;
		p2 = blk_crx->crx[1][1]->p;
		blk_mem->bonds[0][1] = (comp[1][1] == left_c) ?
			Bond(p1,p2) : Bond(p2,p1);
	    }
	}
	blk_mem->num_curves = 1;
	blk_mem->curves[0] = c;
	for (i = 0; i < blk_mem->num_curves; ++i)
	for (j = 0; j < blk_mem->num_bonds[i]; ++j)
	{
	    blk_mem->bonds[i][j]->prev = blk_mem->bonds[i][j]->next = NULL;
	}

	return FUNCTION_SUCCEEDED;
}	/* end construct_comp2_blk2d */


LOCAL	void connect_adj_blk(
	BLK_BOND *bm1,
	BLK_BOND *bm2)
{
	int i1,j1,nc1 = bm1->num_curves;
	int i2,j2,nc2 = bm2->num_curves;
	BOND *b1,*b2;
	CURVE *c;

	for (i1 = 0; i1 < nc1; ++i1)
	{
	    for (i2 = 0; i2 < nc2; ++i2)
	    {
	    	for (j1 = 0; j1 < bm1->num_bonds[i1]; ++j1)
		{
		    for (j2 = 0; j2 < bm2->num_bonds[i2]; ++j2)
		    {
		    	b1 = bm1->bonds[i1][j1];
		    	b2 = bm2->bonds[i2][j2];
			c = bm1->curves[i1];
			if (b1->end == b2->start)
			{
			    b1->next = b2;
			    b2->prev = b1;
			    c = bm1->curves[i1];
			    while (b1 != NULL)
			    {
			    	c->first = b1;
				c->start->posn = b1->start;
				if (b1 == b2) /* closed curve */
				{
				    c->last = b1->prev;
				    c->last->next = NULL;
				    c->first->prev = NULL;
				    c->end->posn = b1->start;
				    break;
				}
				b1 = b1->prev;
			    }
			    while (b2 != NULL && b1 != b2)
			    {
			    	c->last = b2;
				c->end->posn = b2->end;
				b2 = b2->next;
			    }
			}
			else if (b2->end == b1->start)
			{
			    b2->next = b1;
			    b1->prev = b2;
			    while (b2 != NULL)
			    {
			    	c->first = b2;
				c->start->posn = b2->start;
				if (b1 == b2) /* closed curve */
				{
				    c->last = b2->prev;
				    c->last->next = NULL;
				    c->first->prev = NULL;
				    c->end->posn = b2->start;
				    break;
				}
				b2 = b2->prev;
			    }
			    while (b1 != NULL && b1 != b2)
			    {
			    	c->last = b1;
				c->end->posn = b1->end;
				b1 = b1->next;
			    }
			}
		    }
		}
	    }
	}
}	/* end connect_adj_blk */

EXPORT 	CURVE *prompt_make_linear_curve(
	INTERFACE *intfc,
	RECT_GRID *gr)
{
	INTERFACE       *infc;
	double coords1[2],coords2[2],coords[2];
	FT_POINT *p1,*p2;
	NODE *ns,*ne;
	CURVE *curve;
	COMPONENT left_c,right_c;
	int i,num_pts;
	double dx,dy;

	screen("Enter two integers as the left and right components: ");
	Scanf("%d %d\n",&left_c,&right_c);
	screen("Enter the coordinates of the start node: ");
	Scanf("%f %f\n",&coords1[0],&coords1[1]);
	screen("Enter the coordinates of the end node: ");
	Scanf("%f %f\n",&coords2[0],&coords2[1]);

	p1 = Point(coords1);
	ns = make_node(p1);
	p2 = Point(coords2);
	ne = make_node(p2);
	curve = make_curve(left_c,right_c,ns,ne);

	screen("Enter number of interior points of the line: ");
	Scanf("%d\n",&num_pts);
	dx = (coords2[0] - coords1[0])/(num_pts+1);
	dy = (coords2[1] - coords1[1])/(num_pts+1);
	for (i = 0; i < num_pts; ++i)
	{
	    coords[0] = coords1[0] + (i+1)*dx;
	    coords[1] = coords1[1] + (i+1)*dy;
	    if (insert_point_in_bond(Point(coords),curve->last,curve) !=
	                    FUNCTION_SUCCEEDED)
	    {
	    	screen("ERROR in prompt_make_linear_curve(), "
		       "insert_point_in_bond() failed\n");
		clean_up(ERROR);
	    }
	}
	return curve;
}	/* end prompt_make_linear_curve */

EXPORT 	CURVE *prompt_make_elliptic_curve(
	INTERFACE *intfc,
	RECT_GRID *gr)
{
	COMPONENT compin,compout;
	ELLIP_PARAMS ellip;
	double *cen,*rad;
	CURVE *curve;

	cen = ellip.cen;	rad = ellip.rad;
	screen("Enter two integers as the in and out components: ");
	Scanf("%d %d\n",&compin,&compout);
	screen("Enter the coordinates of the elliptic center: ");
	Scanf("%f %f\n",&cen[0],&cen[1]);
	screen("Enter the radii of the ellipse: ");
	Scanf("%f %f\n",&rad[0],&rad[1]);

	ellip.closed = YES;	
	ellip.nor_orient = POSITIVE_ORIENTATION;
	ellip.dim = 2;		
	ellip.gr = &topological_grid(intfc);
	ellip.fpoly = NULL;
	ellip.lpoly = NULL;
	ellip.ThetaS[0] = 0.0;
	curve = make_elliptic_curve(&ellip,compin,compout,0.5);
	return curve;

}	/* end prompt_make_elliptic_curve */

EXPORT void prompt_make_level_curves(
	INTERFACE *intfc,
	RECT_GRID *gr,
	COMPONENT *left_c,
	COMPONENT *right_c)
{
	POINTER func_params;
	double (*func)(POINTER,double*);
	char s[20];
	int num_segs;

	screen("Enter two integers as the left and right components: ");
	Scanf("%d %d\n",left_c,right_c);

	screen("Enter the level curve options\n");
	screen("Supported level curve types are \n"
	       "\tStraight line (l)\n"
	       "\tEllipse (e)\n"
	       "\tMultiple circles (m)\n"
	       "\tSine waves (s)\n"
	       "\tTest disk (t)\n"
	       "Enter choice: ");
	(void) Scanf("%s\n",s);
	switch (s[0])
	{
	case 'l':
	case 'L':
	    func_params = init_line_params(gr);
	    func = line_func;
	    break;
	case 'e':
	case 'E':
	    func_params = init_ellipse_params(gr);
	    func = ellipse_func;
	    break;
	case 'm':
	case 'M':
	    func_params = init_multi_circle_params(gr);
	    func = multi_circle_func;
	    break;
	case 's':
	case 'S':
	    func_params = init_sine_wave_params(gr);
	    func = multi_sine_mode_func;
	    break;
	case 't':
	case 'T':
	    func_params = init_test_disk_params(gr);
	    func = slotted_disk_func;
	    break;
	}
	make_level_curves(gr,intfc,*left_c,*right_c,func,func_params,NO,
			&num_segs);
}	/* end prompt_make_level_interface */


/*	Initialization of curve parameters
*/

LOCAL	POINTER init_line_params(RECT_GRID *gr)
{
	static LINE_PARAMS lparams;
	screen("The equation of line is a*x + b*y = c\n");
	screen("Enter a, b, and c: ");
	Scanf("%f %f %f\n",&lparams.a,&lparams.b,&lparams.c);
	return (POINTER)&lparams;
}	/* end init_line_params */

EXPORT	POINTER init_ellipse_params(RECT_GRID *gr)
{
	static ELLIP2D_PARAMS eparams;
	screen("\tThe equation of ellipse is "
	    	   "(x-x0)^2/a^2 + (y-y0)^2/b^2 = 1\n");
	screen("\tEnter center x0, and y0: ");
	Scanf("%f %f\n",&eparams.x0,&eparams.y0);
	screen("\tEnter radii a, and b: ");
	Scanf("%f %f\n",&eparams.a,&eparams.b);
	return (POINTER)&eparams;
}	/* end init_ellipse_params */

EXPORT  POINTER init_ellipse_tilt_params(RECT_GRID *gr)
{
        static ELLIP2D_TILT_PARAMS etparams;
        screen("The equation of ellipse is "
                   "(x-x0)^2/a^2 + (y-y0)^2/b^2 = 1\n");
        screen("\tEnter center x0, and y0: ");
        Scanf("%f %f\n",&etparams.x0,&etparams.y0);
        screen("\tEnter radii a, and b: ");
        Scanf("%f %f\n",&etparams.a,&etparams.b);
        screen("\tEnter the angle for tilt(degree): ");
        Scanf("%f \n",&etparams.theta);
        return (POINTER)&etparams;
}       /* end init_ellipse_tilt_params */

EXPORT  POINTER init_triangle_params(RECT_GRID *gr)
{
        static TRIANGLE_PARAMS tparams;
        screen("Enter three points for triangle\n");
        screen("\tEnter the first point x0, and y0: ");
        Scanf("%f %f\n",&tparams.x[0],&tparams.y[0]);
        screen("\tEnter the second point x1, and y1: ");
        Scanf("%f %f\n",&tparams.x[1],&tparams.y[1]);
        screen("\tEnter the third point x2, and y2: ");
        Scanf("%f %f\n",&tparams.x[2],&tparams.y[2]);
        return (POINTER)&tparams;
}       /* end init_triangle_params */

EXPORT  POINTER init_rectangle_params(RECT_GRID *gr)
{
        static RECTANGLE_PARAMS rparams;
        screen("Enter the left down point first\n");
        screen("\tEnter the first point x0, and y0: ");
        Scanf("%f %f\n",&rparams.x0,&rparams.y0);
        screen("\tEnter the length of horizontal: ");
        Scanf("%f \n",&rparams.a);
        screen("\tEnter the length of vertical: ");
        Scanf("%f \n",&rparams.b);
        return (POINTER)&rparams;
}       /* end init_rectangle_params */

EXPORT  POINTER init_cosmos_params(RECT_GRID *gr)
{
        static COSMOS_PARAMS cparams;
        screen("\tEnter the x0: ");
        Scanf("%f \n",&cparams.x0);
        screen("\tEnter the x1: ");
        Scanf("%f \n",&cparams.x1);
	screen("\tEnter the x2: ");
        Scanf("%f \n",&cparams.x2);
	screen("\tEnter the y: ");
        Scanf("%f \n",&cparams.y);
	screen("\tEnter the r0: ");
        Scanf("%f \n",&cparams.r0);
	screen("\tEnter the r1: ");
        Scanf("%f \n",&cparams.r1);
	screen("\tEnter the r2: ");
        Scanf("%f \n",&cparams.r2);
        return (POINTER)&cparams;
}

EXPORT  POINTER init_taegeuk_params(RECT_GRID *gr)
{
        static TAEGEUK_PARAMS tgparams;
        screen("\tEnter the x0: ");
        Scanf("%f \n",&tgparams.x0);
        screen("\tEnter the x1: ");
        Scanf("%f \n",&tgparams.x1);
        screen("\tEnter the x2: ");
        Scanf("%f \n",&tgparams.x2);
	screen("\tEnter the x3: ");
        Scanf("%f \n",&tgparams.x3);
	screen("\tEnter the x4: ");
        Scanf("%f \n",&tgparams.x4);
        screen("\tEnter the y: ");
        Scanf("%f \n",&tgparams.y);
        screen("\tEnter the r0: ");
        Scanf("%f \n",&tgparams.r0);
        screen("\tEnter the r1: ");
        Scanf("%f \n",&tgparams.r1);
        return (POINTER)&tgparams;
}

EXPORT  POINTER init_wing_params(RECT_GRID *gr)
{
	static WING_PARAMS wparams;
	screen("\tEnter the x0: ");
	Scanf("%f \n",&wparams.x0);
	screen("\tEnter the y0: ");
        Scanf("%f \n",&wparams.y0);
	screen("\tEnter the x1: ");
        Scanf("%f \n",&wparams.x1);
	screen("\tEnter the y1: ");
        Scanf("%f \n",&wparams.y1);
	screen("\tEnter the a0: ");
        Scanf("%f \n",&wparams.a0);
	screen("\tEnter the b0: ");
        Scanf("%f \n",&wparams.b0);
	screen("\tEnter the a1: ");
        Scanf("%f \n",&wparams.a1);
	screen("\tEnter the b1: ");
        Scanf("%f \n",&wparams.b1);
	return (POINTER)&wparams;
}

EXPORT  POINTER init_propeller_params(RECT_GRID *gr)
{
        static PROPELLER_PARAMS pparams;
        screen("\tEnter the center: ");
        Scanf("%f %f\n",&pparams.x[0],&pparams.y[0]);
	screen("\tEnter the number of Wings: ");
	Scanf("%d \n",&pparams.NofW);
	screen("\tEnter the inner radius: ");
        Scanf("%f \n",&pparams.r[0]);
        screen("\tEnter the outer radius: ");
        Scanf("%f \n",&pparams.r[1]);
        return (POINTER)&pparams;
}
	
LOCAL	POINTER init_multi_circle_params(RECT_GRID *gr)
{
	static MC_PARAMS mc_params;
	int i;
	screen("Enter number of circles: ");
	Scanf("%d\n",&mc_params.num_cir);
	uni_array(&mc_params.rad,mc_params.num_cir,FLOAT);
	bi_array(&mc_params.cen,mc_params.num_cir,2,FLOAT);
	for (i = 0; i < mc_params.num_cir; ++i)
	{
	    screen("Enter the center coords of the %d-th circle: ",i+1);
	    Scanf("%f %f\n",&mc_params.cen[i][0],&mc_params.cen[i][1]);
	    screen("Enter the radius of the %d-th circle: ",i+1);
	    Scanf("%f\n",&mc_params.rad[i]);
	}
	return (POINTER)&mc_params;
}	/* end init_multi_circle_params */

LOCAL	POINTER init_sine_wave_params(RECT_GRID *gr)
{
	static FOURIER_POLY sine_params;
	int i;
	screen("Enter mean position of the curve (in z-direction): ");
	Scanf("%f\n",&sine_params.z0);
	screen("Enter number of modes: ");
	Scanf("%d\n",&sine_params.num_modes);
	bi_array(&sine_params.nu,sine_params.num_modes,1,FLOAT);
	uni_array(&sine_params.A,sine_params.num_modes,FLOAT);
	uni_array(&sine_params.phase,sine_params.num_modes,FLOAT);
	for (i = 0; i < sine_params.num_modes; ++i)
	{
	    screen("Enter the frequency of the %d-th mode: ",i);
	    Scanf("%f\n",&sine_params.nu[i][0]);
	    screen("Enter the amplitude of the %d-th mode: ",i);
	    Scanf("%f\n",&sine_params.A[i]);
	    screen("Enter the phase of the %d-th mode: ",i);
	    Scanf("%f\n",&sine_params.phase[i]);
	    sine_params.phase[i] *= PI/180.0;
	}
	sine_params.dim = 2;
	sine_params.U = gr->U;
	sine_params.L = gr->L;
	return (POINTER)&sine_params;
}	/* end init_sine_wave_params */


LOCAL	POINTER init_test_disk_params(RECT_GRID *gr)
{
	static TDISK_PARAMS td_params;
	screen("Enter center of the disk x0, and y0: ");
	Scanf("%f %f\n",&td_params.x0,&td_params.y0);
	screen("Enter radius of the disk: ");
	Scanf("%f\n",&td_params.r);
	screen("Concave well faces up (along the y-axis)\n");
	screen("Enter the width and depth of the well: ");
	Scanf("%f %f\n",&td_params.w,&td_params.h);
	return (POINTER)&td_params;
}	/* end init_test_disk_params */

/*	Level functions for curves 
*/

LOCAL double line_func(
	POINTER func_params, 
        double *coords)
{
	LINE_PARAMS *lparams = (LINE_PARAMS*)func_params;
	double dist;
	double a = lparams->a;
	double b = lparams->b;
	double c = lparams->c;

	dist = (a*coords[0] + b*coords[1] - c)/sqrt(a*a + b*b);
	return dist;
}	/* end line_func */

EXPORT double multi_circle_func(
	POINTER func_params, 
        double *coords)
{
	MC_PARAMS *mc_params = (MC_PARAMS*)func_params;
	double **cen = mc_params->cen;
	double *rad = mc_params->rad;
	double num_cir = mc_params->num_cir;
	int dim = mc_params->dim;
	double dist,dmin;
	int   i,j,imin;

	dmin = HUGE;
	for (i = 0; i < num_cir; ++i)
	{
	    dist = 0.0;
	    for (j = 0; j < dim; ++j)
		dist += sqr(coords[j] - cen[i][j]);
	    dist = sqrt(dist);
	    if (dist - rad[i] < dmin) 
	    {
	    	dmin = dist - rad[i];
		imin = i;
	    }
	}
	dist = 0.0;
	for (j = 0; j < dim; ++j)
	    dist += sqr(coords[j] - cen[imin][j]);
	dist = sqrt(dist) - rad[imin];
	return dist;
}	/* end multi_circle_func */

EXPORT double ellipse_func(
	POINTER func_params, 
        double *coords)
{
	ELLIP2D_PARAMS *eparams = (ELLIP2D_PARAMS*)func_params;
	double dist;

	dist = sqr(coords[0] - eparams->x0)/sqr(eparams->a) +
	       sqr(coords[1] - eparams->y0)/sqr(eparams->b) - 1.0;
	return dist;
}	/* end ellipse_func */

EXPORT double ellipse_tilt_func(
        POINTER func_params,
        double *coords)
{
        ELLIP2D_TILT_PARAMS *etparams = (ELLIP2D_TILT_PARAMS*)func_params;

        double dist;
        double tcoords[2];
        double phi;

        phi = etparams->theta*PI/180.0;
        tcoords[0] = cos(-phi)*(coords[0]-etparams->x0) - 
			sin(-phi)*(coords[1]-etparams->y0) + etparams->x0;
        tcoords[1] = sin(-phi)*(coords[0]-etparams->x0) + 
			cos(-phi)*(coords[1]-etparams->y0) + etparams->y0;

        dist = sqr(tcoords[0] - etparams->x0)/sqr(etparams->a) + sqr(tcoords[1] - 
			etparams->y0)/sqr(etparams->b) - 1.0;
        return dist;
}       /* end ellipse_tilt_func JD */

EXPORT double triangle_func(
        POINTER func_params,
        double *coords)
{
        TRIANGLE_PARAMS *tparams = (TRIANGLE_PARAMS*)func_params;
	double x1[MAXD],x2[MAXD],x3[MAXD];
        double dd,dist0,dist1,dist2,dist3;
        int i;

        x1[0] = tparams->x[0];
        x1[1] = tparams->y[0];
        x2[0] = tparams->x[1];
        x2[1] = tparams->y[1];
        x3[0] = tparams->x[2];
        x3[1] = tparams->y[2];

        dist0 = (x2[1]-x1[1])*(x1[0]-x3[0])-(x1[1]-x3[1])*(x2[0]-x1[0]);
        dist0 = fabs(dist0);
        dist1 = ((x2[1]-x1[1])*(x1[0]-coords[0])-(x1[1]-coords[1])*(x2[0]-x1[0]));
        dist1 = fabs(dist1);
        dist2 = ((x2[1]-x3[1])*(x3[0]-coords[0])-(x3[1]-coords[1])*(x2[0]-x3[0]));
        dist2 = fabs(dist2);
        dist3 = ((x3[1]-x1[1])*(x1[0]-coords[0])-(x1[1]-coords[1])*(x3[0]-x1[0]));
        dist3 = fabs(dist3);
        dd = dist1+dist2+dist3-dist0;

        if(dd>0.00001) return dist0;
        if(dd<=0.00001)
        {
               if(dist1<0.00001||dist2<0.00001||dist3<0.00001)
               return 0;
               else return -dist0;
        }

}       /* end triangle_func  JD*/

EXPORT double rectangle_func(
        POINTER func_params,
        double *coords)
{
        RECTANGLE_PARAMS *rparams = (RECTANGLE_PARAMS*)func_params;
        double det0,det1,det2,det3;
        double x[4], y[4];
        x[0] = rparams->x0;     y[0] = rparams->y0;
        x[1] = x[0] + rparams->a;       y[1] = y[0];
        x[2] = x[0] + rparams->a;       y[2] = y[0] + rparams->b;
        x[3] = x[0];            y[3] = y[0] + rparams->b;

        det0 = x[1]*coords[1] - y[1]*coords[0] - x[0]*coords[1] + 
				y[0]*coords[0] + x[0]*y[1] - x[1]*y[0];
        det1 = x[2]*coords[1] - y[2]*coords[0] - x[1]*coords[1] + 
				y[1]*coords[0] + x[1]*y[2] - x[2]*y[1];
        det2 = x[3]*coords[1] - y[3]*coords[0] - x[2]*coords[1] + 
				y[2]*coords[0] + x[2]*y[3] - x[3]*y[2];
        det3 = x[0]*coords[1] - y[0]*coords[0] - x[3]*coords[1] + 
				y[3]*coords[0] + x[3]*y[0] - x[0]*y[3];

	if (det0 ==0.0)
        {
            if (x[0] == x[1])
            {
                if ((y[0] <= coords[1] && y[1] >= coords[1])||
			(y[1] <= coords[1] && y[0] >= coords[1]))
                    return det0;
                else return HUGE;
            }
            else
            {
                 if ((x[0] <= coords[0] && x[1] >= coords[0])||
		 	(x[1] <= coords[0] && x[0] >= coords[0]))
                     return det0;
                 else return HUGE;
            }
        }
        else if (det1 == 0.0)
        {
            if (x[1] == x[2])
            {
                if ((y[1] <= coords[1] && y[2] >= coords[1])||
			(y[2] <= coords[1] && y[1] >= coords[1]))
                    return det1;
                else return HUGE;
            }
            else
            {
                if ((x[1] <= coords[0] && x[2] >= coords[0])||
			(x[2] <= coords[0] && x[1] >= coords[0]))
                    return det1;
                else return HUGE;
            }
        }
        else if (det2 == 0.0)
        {
            if (x[2] == x[3])
            {
                if ((y[2] <= coords[1] && y[3] >= coords[1])||
			(y[3] <= coords[1] && y[2] >= coords[1]))
                    return det2;
                else return HUGE;
            }
            else
            {
                if ((x[2] <= coords[0] && x[3] >= coords[0])||
			(x[3] <= coords[0] && x[2] >= coords[0]))
                    return det2;
                else return HUGE;
            }
        }
	else if (det3 == 0.0)
        {
            if (x[3] == x[0])
            {
                if ((y[3] <= coords[1] && y[0] >= coords[1])||
			(y[0] <= coords[1] && y[3] >= coords[1]))
                    return det3;
                else return HUGE;
            }
            else
            {
                if ((x[3] <= coords[0] && x[0] >= coords[0])||
			(x[0] <= coords[0] && x[3] >= coords[0]))
                    return det3;
                else return HUGE;
            }
        }
        else if (det0 > 0.0 && det1 > 0.0 && det2 > 0.0 && det3 > 0.0)
            return -HUGE;
        else
            return HUGE;
}       /*rectangle_func  JD*/

EXPORT double cosmos_func(
        POINTER func_params,
        double *coords)
{
        COSMOS_PARAMS *cparams = (COSMOS_PARAMS*)func_params;
	double x0,x1,x2,y,r0,r1,r2;
        double dist0,dist1,dist2;
	double tcoords[2];
	double phi;
        x0 = cparams->x0;
        x1 = cparams->x1;
        x2 = cparams->x2;
        y  = cparams->y;
        r0 = cparams->r0;
        r1 = cparams->r1;
        r2 = cparams->r2;

	phi = PI/6.0;
        tcoords[0] = cos(-phi)*(coords[0]-x2) - sin(-phi)*(coords[1]-y) + x2;
        tcoords[1] = sin(-phi)*(coords[0]-x2) + cos(-phi)*(coords[1]-y) + y;

        if(tcoords[1]<y)
        {
             dist0 = sqrt(sqr(tcoords[0]-x0)+sqr(tcoords[1]-y))-r0;
             dist1 = sqrt(sqr(tcoords[0]-x1)+sqr(tcoords[1]-y))-r1;
             if(dist0>0&&dist1<=0)
             return dist1;
             if(dist0<=0)
             return -dist0;
             else return dist0;
        }
        if(tcoords[1]>=y)
        {

             dist2 = sqrt(sqr(tcoords[0]-x2)+sqr(tcoords[1]-y))-r2;
             return(dist2);
        }
}	/*cosmos_func */

EXPORT double taegeuk_func(
        POINTER func_params,
        double *coords)
{
        TAEGEUK_PARAMS *tgparams = (TAEGEUK_PARAMS*)func_params;
        double x0,x1,x2,x3,x4,y,r0,r1;
        double dist0,dist1,dist2,dist3;
        double tcoords[2];
        double phi;

        x0 = tgparams->x0;
        x1 = tgparams->x1;
        x2 = tgparams->x2;
	x3 = tgparams->x3;
	x4 = tgparams->x4;	
        y  = tgparams->y;
        r0 = tgparams->r0;
        r1 = tgparams->r1;

	phi = PI/6.0;
        tcoords[0] = cos(-phi)*(coords[0]-x2) - sin(-phi)*(coords[1]-y) + x2;
        tcoords[1] = sin(-phi)*(coords[0]-x2) + cos(-phi)*(coords[1]-y) + y;

        if(tcoords[1]<y)
        {
             dist0 = sqrt(sqr(tcoords[0]-x0)+sqr(tcoords[1]-y))-r0;
             dist1 = sqrt(sqr(tcoords[0]-x1)+sqr(tcoords[1]-y))-r1;
             if(dist0>0&&dist1<=0)
             return dist1;
             if(dist0<=0)
             return -dist0;
             else return dist0;
        }
        if(tcoords[1]>=y)
        {
             dist2 = sqrt(sqr(tcoords[0]-x3)+sqr(tcoords[1]-y))-r1;
	     dist3 = sqrt(sqr(tcoords[0]-x4)+sqr(tcoords[1]-y))-r0;
	     if(dist3>0&&dist2<=0)
	     return dist2;
	     if(dist3<=0)
	     return -dist3;
	     else return dist3;             
        }
}       /*taegeuk_func by JD Kim*/

EXPORT double wing_func(
        POINTER func_params,
        double *coords)
{
        WING_PARAMS *wparams = (WING_PARAMS*)func_params;
	double x0,x1,y0,y1,a0,a1,b0,b1;
	double dist,t,a;
	x0 = wparams->x0;
	x1 = wparams->x1;
	y0 = wparams->y0;
	y1 = wparams->y1;
	a0 = wparams->a0;
	a1 = wparams->a1;
	b0 = wparams->b0;
	b1 = wparams->b1;
	t = sqrt((1.0-sqr((0.3-y0)/b0))*sqr(a0)) + 0.7;
	a = y1 - b1;

	if (coords[0]>x0 && coords[1]>a)
		dist = sqr((coords[0]-x0)/a0)+sqr((coords[1]-y0)/b0)-1.0;
	else if (coords[1]<=a && coords[0]>x0 && coords[0]<t)
		dist = a - coords[1];
	else if (coords[0]<=x1)
		dist = sqr((coords[0]-x1)/a1)+sqr((coords[1]-y1)/b1)-1.0;
	else dist=HUGE;

	return dist;
}


EXPORT double propeller_func(
        POINTER func_params,
        double *coords)
{
        PROPELLER_PARAMS *pparams = (PROPELLER_PARAMS*)func_params;
        double x[3],y[3];
	int NofW;	/*the number of wings (Please choose a multiple of 4 */
	double r[2];	/*r[0]=inner radius, r[1]=outer radius */
	double a[2],b[2];	/*thickness of wings */
        double dist0,dist1,dist2;
        double rcrds[MAXD],R,phi;
	double tcoords[2];
	int n;
	x[0] = pparams->x[0]; y[0] = pparams->y[0];
	NofW = pparams->NofW;  
	r[0] = pparams->r[0]; r[1] = pparams->r[1];

	x[1] = x[0]+r[0]*cos(2.0*PI/NofW);
	x[2] = x[0]+0.5*(r[0]+r[1]);
	y[1] = y[2] = y[0];

	a[0] = r[1]-r[0]*cos(2.0*PI/NofW); 
	a[1] = 0.5*(r[1]-r[0]);  
	b[0] = r[0]*sin(2.0*PI/NofW);
	b[1] = 1.5/NofW*b[0]; 	/*This comes from experiment but  */
				/*still have restriction */

	rcrds[0] = coords[0] - x[0];
        rcrds[1] = coords[1] - y[0];
        R = mag_vector(rcrds,2);
        phi = asin(fabs(rcrds[1])/R);
        if (rcrds[0] < 0.0)
        {
            if (rcrds[1] >= 0.0)
                phi = PI - phi;
            else
                phi = PI + phi;
        }
        else if (rcrds[1] < 0.0) 
            phi = 2.0*PI - phi;
        if (R == 0.0) phi = 0.0;

	n = phi/(2.0*PI)*NofW;	/*n is integer */
	phi = -n*2.0*PI/NofW;

        tcoords[0] = rcrds[0]*cos(phi) - rcrds[1]*sin(phi) + x[0];
        tcoords[1] = rcrds[1]*cos(phi) + rcrds[0]*sin(phi) + y[0];

	dist1=sqr(tcoords[0]-x[1])/sqr(a[0])+sqr(tcoords[1]-y[1])/sqr(b[0])-1.0;
	dist2=sqr(tcoords[0]-x[2])/sqr(a[1])+sqr(tcoords[1]-y[2])/sqr(b[1])-1.0;

	dist0 = (dist2 <= 0.0) ? -dist2 : dist1;
	return dist0;
}       /*propeller_func by JD Kim*/

EXPORT double slotted_disk_func(
	POINTER func_params, 
        double *coords)
{
	TDISK_PARAMS *td_params = (TDISK_PARAMS*)func_params;
	double xl,xr,yh;
	double dist1,dist2,dist3;

	xl = td_params->x0 - td_params->w/2.0;
	xr = td_params->x0 + td_params->w/2.0;
	yh = td_params->y0 + td_params->r - td_params->h;
	dist1 = sqrt(sqr(coords[0] - td_params->x0) + 
                sqr(coords[1] - td_params->y0)) - td_params->r;
	dist2 = (coords[0] < td_params->x0) ? coords[0] - xl :
		xr - coords[0];
	dist3 = coords[1] - yh;
	if (dist1 > 0.0) return dist1;
	else if (dist2 > 0.0 && dist3 > 0.0)
	    return min(dist2,dist3);
	else if (dist2 > 0 && dist3 < 0.0)
	    return max(dist1,dist3);
	else if (dist2 < 0 && dist3 > 0.0)
	    return max(dist1,dist2);
	else
	    return max(dist1,max(dist2,dist3));
}	/* end slotted_disk_func */

EXPORT double level_wave_func(
        POINTER func_params,
        double *coords)
{
        FOURIER_POLY *wave_params = (FOURIER_POLY*)func_params;
        double arg,z,k,phase,dist;
        int i,j,num_modes,dim;
        double *L = wave_params->L;
        double *U = wave_params->U;

        dim = wave_params->dim;
        num_modes = wave_params->num_modes;
        z = wave_params->z0;

        for (i = 0; i < num_modes; ++i)
        {
            arg = 0.0;
            for (j = 0; j < dim-1; ++j)
            {
                k = wave_params->nu[i][j]*2.0*PI/(U[j]-L[j]);
                arg += k*coords[j];
            }
            phase = wave_params->phase[i]*PI/180.0;
            arg -= phase;
            z += wave_params->A[i]*sin(arg);
        }
        dist = coords[dim-1] - z;
        return dist;
}	/* end level_wave_func */

EXPORT double level_circle_func(
        POINTER func_params,
        double *coords)
{
        CIRCLE_PARAMS *circle_params = (CIRCLE_PARAMS*)func_params;
        int i,dim;
        double *cen,R,H,dist,dist_h;

        dim = circle_params->dim;
        cen = circle_params->cen;
        R  = circle_params->R;
        H  = circle_params->H;
        dist = 0.0;
        for (i = 0; i < dim; ++i)
            dist += sqr(coords[i] - cen[i]);
	dist = sqrt(dist);
	if (circle_params->add_perturbation)
	{
	    if (dim == 2)
	    {
		FOURIER_POLY *fpoly = circle_params->fpoly;
		int num_modes = fpoly->num_modes;
		double theta;
	    	theta = asin(fabs(coords[1]-cen[1])/dist);
            	if (coords[0]-cen[0] < 0 &&
                    coords[1]-cen[1] > 0)
                    theta = PI - theta;
            	else if (coords[0]-cen[0] < 0 &&
                    coords[1]-cen[1] < 0)
                    theta = PI + theta;
            	else if (coords[0]-cen[0] > 0 &&
                    coords[1]-cen[1] < 0)
                    theta = 2*PI - theta;
		for (i = 0; i < num_modes; ++i)
		{
		    double nu = fpoly->nu[0][i];
		    double amp = fpoly->A[i];
		    double phase = fpoly->phase[i]/180.0*PI;
            	    R -= amp*sin(nu*theta + phase);
		}
	    }
	}
        dist -= R;
        if (circle_params->add_plan_surf)
        {
            if (H > cen[dim-1])
                dist_h = H - coords[dim-1];
            else
                dist_h = coords[dim-1] - H;
            dist = (dist_h < dist) ? dist_h : dist;
        }
        return dist;
}       /* end level_circle_func */

EXPORT CURVE *make_array_curve(
	INTERFACE   *intfc,
	COMPONENT   left_comp,
	COMPONENT   right_comp,
	int	    num_points,
	double	    **point_array,
	boolean	    is_closed_curve)
{
	int i;
	CURVE *c;
	NODE *ns,*ne;
	INTERFACE *save_intfc;
	double coords[MAXD];

	save_intfc = current_interface();
        set_current_interface(intfc);

	if (is_closed_curve)
	{
	    coords[0] = point_array[0][0];
	    coords[1] = point_array[0][1];
	    ns = ne = make_node(Point(coords));
	}
	else
	{
	    coords[0] = point_array[0][0];
	    coords[1] = point_array[0][1];
	    ns = make_node(Point(coords));
	    coords[0] = point_array[num_points-1][0];
	    coords[1] = point_array[num_points-1][1];
	    ne = make_node(Point(coords));
	    num_points--;
	}
	c = make_curve(left_comp,right_comp,ns,ne);
	for (i = 1; i < num_points; ++i)
	{
	    coords[0] = point_array[i][0];
	    coords[1] = point_array[i][1];
	    insert_point_in_bond(Point(coords),c->last,c);
	}

	set_current_interface(save_intfc);
	return c;
}	/* end make_array_curve */


EXPORT double projectile_func(
        POINTER func_params,
        double *coords)
{
	PROJECTILE_PARAMS *proj_params = (PROJECTILE_PARAMS*)func_params;
	int i,dim = proj_params->dim;
	double *cen = proj_params->cen;
	double r = proj_params->r;
	double R = proj_params->R;
	double h = proj_params->h;
	double rot_angle = proj_params->theta;
	double dist,dist2,ebdry2,d1,d2;

	d1 = sqr(coords[0] - cen[0]);
	d2 = 0.0;
	for (i = 1; i < dim; ++i)
	    d2 += sqr(coords[i] - cen[i]);
	dist2 = d1 + d2;
	if (coords[0] > cen[0])
	{
	    dist = d1/sqr(r) + d2/sqr(R) - 1.0;
	}
	else
	{
	    if (d2/d1 < sqr(R/h))
	    {
		ebdry2 = sqr(h/R) + d2/d1*sqr(h/R);
	    	dist = (d1 + d2)/sqr(R) - ebdry2;
	    }
	    else
	    {
		ebdry2 = 1.0 + d1/d2;
	    	dist = (d1 + d2)/sqr(R) - ebdry2;
	    }
	}
	return dist;
}	/* end projectile_func */
