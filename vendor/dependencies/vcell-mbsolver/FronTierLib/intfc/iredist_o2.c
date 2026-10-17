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
*				iredist_o2.c:
*
*       Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Containing function of redistribute interface using high order algorithm.
*
*/


#include <intfc/int.h>

#define DEBUG_STRING "i_redistribute"

LOCAL  	void  polyfit2d_interp_edge(double*,double*,int,double*,int,double*,int,double*,int,
				    double,int,int);
LOCAL  	void  polyfit2d_interp_vertex(double*,double*,int,double*,int,double*,double,
				     int,int);
LOCAL   void  quadratic_mid_point_on_edge2d(double*,CURVE*,BOND*,int,double);
LOCAL  	int   get_neighbor_on_curve(CURVE*,BOND*,int,double (*)[2],double (*)[2], boolean);
LOCAL  	int   eval_vander_univar(double*,int,double*,int*,int,double*,int,int,int);

#define ct_max(e1,e2) (((e2) > (e1)) ? (e2): (e1))
#define ct_min(e1,e2) (((e2) < (e1)) ? (e2): (e1))
#define ct_set_max(e1,e2,e3)  (e1 = e2, (e1 < e3 ? e1 = e3 : e1))
#define ct_set_min(e1,e2,e3)  (e1 = e2, (e1 > e3 ? e1 = e3 : e1))



enum _TRI_STATUS {
        SMALL = 1,
        LARGE,
        BAD_ANGLE,
        GOOD_ANGLE
};
typedef enum _TRI_STATUS TRI_STATUS;
enum _SPQ_FLAG {
        SHORTEST = 0,
        LONGEST  = 1
};
typedef enum _SPQ_FLAG SPQ_FLAG;

/* 3D redistribute surface functions */


LOCAL   TRI_STATUS tri_status(TRI*,RECT_GRID*,TRI_REDIST_PARAMS);
LOCAL   int     find_scaled_extrem_edge(TRI*,RECT_GRID*,SPQ_FLAG);
LOCAL   boolean    is_tri_in_queue(TRI*,POINTER_Q*);
LOCAL   boolean    delete_min_side_of_tri_o2(TRI*,int,SURFACE*,POINTER_Q**,
                                        INTERFACE*);
LOCAL   void    sort_pointer_queue(POINTER_Q*,INTERFACE*,SPQ_FLAG);
LOCAL	int	remove_tris_and_seal_o2(TRI**,TRI**,int,SURFACE*,POINTER_Q**,
 					INTERFACE*);
LOCAL 	void 	exchange_queues(POINTER_Q*,POINTER_Q*);
LOCAL 	boolean 	compare_pointers(POINTER_Q*,POINTER_Q*);

LOCAL   void    quadratic_mid_point_on_edge3d(double*,TRI*,int,
					      HYPER_SURF_ELEMENT*,
					      HYPER_SURF*,int);
LOCAL   void    polyfit3d_walf_edge(double*,double*,int,double*,int,double*,int,double*,int
				      ,double,int,int);
LOCAL   void    polyfit3d_walf_vertex(double*,double*,int,double*,int,double*,int,int);
LOCAL   int     eval_vander_bivar_cmf(double*,int,double*,int*,int,double*,int,int,int);
LOCAL   void    backsolve_bivar_safeguarded(double*,int,int,double*,int*,int,int,double*,int);
LOCAL   void    compute_qtb(double*,int,int,double*,int*,int);
LOCAL   void    qr_safeguarded(double*,int*,int*,double*,int*,int*,int,int*,int);
LOCAL   void    rescale_matrix(double*,int*,double*,int*,int*,int,int*,int);
LOCAL   void    construct_vander_bivar(double*,int*,int*,double*,int,int,int,int*,int,int);
LOCAL   void    compute_cmf_weights(double*,int*,int*,double*,double*,int,double*,int,double,int,double);
LOCAL   double  compute_resolution(double*,int);
LOCAL   boolean 	act_delete = NO;


/*
*			equi_redist_curve_seg_o2():
*
*	Second order redistribute, by Duo Wang
*/

EXPORT	void equi_redist_curve_seg_o2(
       	CURVE		*c,
	BOND		*bs,
	BOND		*be,
	int		nbds,
	double		seg_len,
	double		space,
	RECT_GRID	*rgr)
{
        BOND		*b, *bstart, *bend;
	double		b_len, sc_len, offset, s;
	double		coords[MAXD];
	double		*h = rgr->h;
	int		dim = rgr->dim;
	int		new_nbds;

	int nbd=6;
	
	/* first we copy the curve, since c changes during insertion */
	NODE *n_start = i_copy_node(c->start);
	NODE *n_end = i_copy_node(c->end);
	CURVE *c_old = i_copy_curve(c,n_start,n_end);
	BOND *b_old = c_old->first;

	if (debugging("high_order_redist"))
	{
	    printf("Entering equi_redist_curve_seg_o2()\n");
	    printf("redist_order = %d\n",c->redist_order);
	}

	DEBUG_ENTER(equi_redist_curve_seg)
	  if (nbds <= 0)
	    {
	      new_nbds = (int)(ceil((double)seg_len/(double)space));
	      b_len = seg_len/(double)new_nbds;
	    }
	  else
	    {
	      new_nbds = nbds;
	      b_len = seg_len/(double)new_nbds;
	    }
	if (new_nbds <= 1)
	{
	    if (is_closed_curve(c) && bs == c->first && be == c->last)
	    {
	    	new_nbds = c->num_points - 1;
	    	if (new_nbds > 1)
	            equi_redist_curve_seg(c,bs,be,new_nbds,
					  seg_len,space,rgr);
	    }
	    else
	        replace_curve_seg_by_bond(c,bs,be);

	    DEBUG_LEAVE(equi_redist_curve_seg)
	    return;
	}

	offset = b_len;
	bstart = bs;		bend = be->next;
	
	while (bstart != bend)
	  {  
	    b = bstart;
	    while ((sc_len = scaled_bond_length(b,h,dim)) < offset)
	      {
	    	if (b->next == bend)
		  {
	    	    replace_curve_seg_by_bond(c,bstart,b);
	    	    goto leave;
		  }
	    	offset -= sc_len;
	    	b = b->next;
		b_old = b_old->next;		
	      }				
		if ((b->next != bend) ||
		    (sc_len >= offset + MIN_SC_SEP(c->interface)))
		  {
		    s = offset/sc_len;	
		    /* here we get neighbour points using original curve */
		    quadratic_mid_point_on_edge2d(coords,c_old,b_old,nbd,s);

		    if (insert_point_in_bond(Point(coords),b,c) !=
			FUNCTION_SUCCEEDED)
		      /* after insertion, b becomes the left of the two newly bonds */
		      {
			screen("ERROR in equi_redist_curve_seg(), "
			       "insert_point_in_bond failed\n");
			clean_up(ERROR);
		      }
		  }
		replace_curve_seg_by_bond(c,bstart,b);
		/* after replace, bstart connects bstart->start and b->end, b is abondoned */
		bstart=bstart->next;
		offset=b_len;
	  }

 leave:
	DEBUG_LEAVE(equi_redist_curve_seg)
	  i_delete_curve(c_old);
	return;
}		/*end equi_redist_curve_seg*/
void  quadratic_mid_point_on_edge2d(
        double *coords,
	CURVE *c,
	BOND  *b,
	int   nbd,
	double s)
{
        int pos,i;
	double ngbpts[6][2],nrms[6][2];
	double ngbpts1[5][2],ngbpts2[5][2];
	double nrms1[5][2],nrms2[5][2];
	
	pos=get_neighbor_on_curve(c,b,nbd,ngbpts,nrms,1);
	/* pos is the position of the bond, equals nbd/2 except near boundary */
	ngbpts1[0][0]=ngbpts[pos-1][0];
	ngbpts1[0][1]=ngbpts[pos-1][1];
	nrms1[0][0]=nrms[pos-1][0];
	nrms1[0][1]=nrms[pos-1][1];
	ngbpts2[0][0]=ngbpts[pos][0];
	ngbpts2[0][1]=ngbpts[pos][1];
	nrms2[0][0]=nrms[pos][0];
	nrms2[0][1]=nrms[pos][1];
	
	for (i=1;i<pos;i++)
	  {
	    ngbpts1[i][0]=ngbpts[i-1][0];
	    ngbpts1[i][1]=ngbpts[i-1][1];
	    nrms1[i][0]=nrms[i-1][0];
	    nrms1[i][1]=nrms[i-1][1];
	    
	    ngbpts2[i][0]=ngbpts[i][0];
	    ngbpts2[i][1]=ngbpts[i][1];
	    nrms2[i][0]=nrms[i][0];
	    nrms2[i][1]=nrms[i][1];

	  }
	for (i=pos;i<nbd-1;i++)
	  {
	    ngbpts1[i][0]=ngbpts[i][0];
	    ngbpts1[i][1]=ngbpts[i][1];
	    nrms1[i][0]=nrms[i][0];
	    nrms1[i][1]=nrms[i][1];

	    ngbpts2[i][0]=ngbpts[i+1][0];
	    ngbpts2[i][1]=ngbpts[i+1][1];
	    nrms2[i][0]=nrms[i+1][0];
	    nrms2[i][1]=nrms[i+1][1];
	    
	  }
	/* strip set 1 is necessary */
	polyfit2d_interp_edge(coords,*ngbpts1,nbd-1,*nrms1,nbd-1,*ngbpts2,nbd-1,*nrms2,nbd-1,s,2,1);
}
	

/* get neighbor points coordiantes and normals into arrays */
LOCAL   int get_neighbor_on_curve(
        CURVE* c,
	BOND* b, 
	int ngb, 
	double ngbpts[][2],
	double nrms[][2],
	boolean closed)
{
        int nleft=(ngb-2)/2;
	int pos=nleft+1;
	int nright=ngb-nleft-2;
	int i,j;
	double dist = 0;
	BOND *bleft, *bright, *bcurrent;
	/* todo if (c->num_points<nbd) */
	
	/* bond end points*/
	bcurrent=b;       
	if (closed)
	  {
	    /* left neighbors */
	    bleft=b;
	    for (i=nleft; i>=0; i--)
	      { 
		ngbpts[i][0]=Coords(bleft->start)[0];
		ngbpts[i][1]=Coords(bleft->start)[1];
		if (bleft != c->first) /* not start of the curve */
		  {
		    bleft=bleft->prev; /* gets the normal */
		    nrms[i][0]= -(Coords(bleft->next->end)[1]-Coords(bleft->start)[1]);
		    nrms[i][1]= Coords(bleft->next->end)[0]-Coords(bleft->start)[0];
		  }

		else			/* start of the curve */
		  {
		    bleft=c->last;/* gets the normal */
		    nrms[i][0]= -(Coords(c->first->end)[1]-Coords(bleft->start)[1]);
		    nrms[i][1]= Coords(c->first->end)[0]-Coords(bleft->start)[0];
		  }
		dist = sqrt(nrms[i][0]*nrms[i][0]+nrms[i][1]*nrms[i][1]);
		nrms[i][0] = nrms[i][0]/dist;
		nrms[i][1] = nrms[i][1]/dist;		
	      }
	    /* right neighbors */
	    bright=b;
	    for (i=nleft+1; i<ngb; i++)
	      { 
		ngbpts[i][0]=Coords(bright->end)[0];
		ngbpts[i][1]=Coords(bright->end)[1];
		if (bright != c->last) /* not end of the curve */
		  {
		    bright=bright->next;
		    nrms[i][0]= -(Coords(bright->end)[1]-Coords(bright->prev->start)[1]);
		    nrms[i][1]= Coords(bright->end)[0]-Coords(bright->prev->start)[0];		  
		  }
		else			/* end of the curve */
		  {
		    bright=c->first;
		    nrms[i][0]= -(Coords(bright->end)[1]-Coords(c->last->start)[1]);
		    nrms[i][1]= Coords(bright->end)[0]-Coords(c->last->start)[0];		  
		  }
		dist = sqrt(nrms[i][0]*nrms[i][0]+nrms[i][1]*nrms[i][1]);
		nrms[i][0] = nrms[i][0]/dist;
		nrms[i][1] = nrms[i][1]/dist;		
	      }
	  }
	else
	  {		
	    bleft=b;
	    for (i=nleft; i>0; i--)
	      {
		if (bleft == c->first)
		  {
		    /* if bond is near boundary, we shift the bond b */
		    /* and the return value pos can give the information */
		    for (j=0;j<i;j++)
		      {
			b=b->next;
			pos=pos-1;
		      }
		    break;
		  }
		bleft=bleft->prev;
	      }
	    bright=b;
	    for (i=nright; i>0; i--)
	      {
		if (bright == c->last)
		  {
		    for (j=0;j<i;j++)
		      {
			b=b->prev;
			pos=pos+1;
		      }
		    break;
		  }
		bright=bright->next;
	      }
	    get_neighbor_on_curve(c,b,6,ngbpts,nrms,1);
	    b=bcurrent;  
	  }
	return pos;			/* return the bond index */
}
/*************************************************************
 *
 * FUNCTION: polyfit2d_interp_edge
 *
 * Calculate the position of the point in the high-order surface 
 *    The function constructs the three fittings using points ngbpnts1,
 *    ngbpnts2 and ngbpnts3, respectively, then using
 *    (1-param, param)  as the barycentric coordinate
 *    
 * Input:
 * ngbpnts1-2:Input points of size mx3, Its first column is x-coordinates,
 *            and its second column is y-coordinates. The first vertex will
 *            be used as the origin of the local coordinate system.
 * nrms1-2:   The normals at ngbptns
 * param:     The parameter 
 *            along the tangent direction.
 * deg:       The degree of polynomial to fit, from 0 to 6
 * strip:     If 1, the fit is interpolatory at ngbpnts(1,:) (in other words,
 *            the fit passes through point ngbpnts(1,:)). If 0, the fit does not
 *            pass the point ngbpnts(1,:), useful for a noisy inputs.
 * Output:
 * pnt:    The interpolated coordinates in the global coordinate system
 *************************************************************/

void  polyfit2d_interp_edge(
   double pnt_out[2], 
   double *ngbpnts1, 
   int ngbpnts1_dim1, 
   double *nrms1, 
   int nrms1_dim1, 
   double *ngbpnts2, 
   int ngbpnts2_dim1, 
   double *nrms2, 
   int nrms2_dim1, 
   double param, 
   int deg, 
   int strip)
{
   int deg_1;
   double vec[2];
   double vec_1[2], pnt1[2], pnt2[2];
   double s;

   deg_1 = deg;
   /* ct_assert(size(ngbpnts1,2)==2); */
   /* ct_assert(size(ngbpnts1,1)>3 && size(ngbpnts1,1)<=MAXPNTS); */
   /* ct_assert(size(ngbpnts2, 2)==2); */
   /* ct_assert(size(ngbpnts2,1)>3 && size(ngbpnts2,1)<=MAXPNTS); */
   /* ct_assert(deg>=2 && deg<=6); */
   /* Use linear fitting by default */
   /* Use approximation by default */
   /* Compute edge tangent vector and parameter */

   vec[0] =  ngbpnts2[0] -  ngbpnts1[0];
   vec[1] =  ngbpnts2[1] -  ngbpnts1[1];
   s = sqrt( vec[0] *  vec[0] +  vec[1] *  vec[1]);
   vec_1[0] =  vec[0] / s;
   vec_1[1] =  vec[1] / s;
   /* Interpolate using vertex-based polynomial fittings at two vertices */

   polyfit2d_interp_vertex(pnt1, ngbpnts1, ngbpnts1_dim1, nrms1, nrms1_dim1, vec_1, param * s, deg_1, strip);
   polyfit2d_interp_vertex(pnt2, ngbpnts2, ngbpnts2_dim1, nrms2, nrms2_dim1, vec_1, (0.0 - (1.0 - param)) * s, deg_1, strip);
   /* Compute weighted average of the two points */

   pnt_out[0] = (1.0 - param) *  pnt1[0] + param *  pnt2[0];
   pnt_out[1] = (1.0 - param) *  pnt1[1] + param *  pnt2[1];
   return;
}

/*************************************************************
 *
 * FUNCTION: polyfit2d_interp_vertex
 *
 * Construct a local polynomial fitting and then interpolate.
 *    The function constructs the fitting using points pnts(:,1:3) in
 *    a local uv coordinate system with pnts(1,1:3) as the origin
 *    and nrms(1,1:3) vertical axis, and then interpolates to the point
 *    with u=param to obtain its coordinates.
 * Input:
 * pnts:   Input points of size mx3, Its first column is x-coordinates,
 *         and its second column is y-coordinates. The first vertex will
 *         be used as the origin of the local coordinate system.
 * nrms:   The normals at pnts
 * vec:    The unit direction vector
 * param:  The distance of the point to be interpolated from pnts(1,:)
 *         along the tangent direction.
 * deg:    The degree of polynomial to fit, from 0 to 6
 * strip:  If 1, the fit is interpolatory at pnts(1,:) (in other words,
 *         the fit passes through point pnts(1,:)). If 0, the fit does not
 *         pass the point pnts(1,:), useful for a noisy inputs.
 * Output:
 * pnt:    The interpolated coordinates in the global coordinate system
 *         for the point with u=param in the local coordinate system.
 * deg can be 2-6
 *************************************************************/

void  polyfit2d_interp_vertex(
   double pnt_out[2], 
   double *pnts, 
   int pnts_dim1, 
   double *nrms, 
   int nrms_dim1, 
   double vec[2], 
   double param, 
   int deg, 
   int strip)
{
   double us[13];
   int us_dim1;
   double bs[13];
   int bs_dim1;
   double ws_row[13];
   int ws_row_dim1;
   int deg_1, nverts, k, i;
   double nrm[2], tng[2];
   double uu[2];
   double dist, height, dtemp_0, dtemp_1, dtemp_2;

   deg_1 = deg;
   /*     ct_assert( size(pnts,2)==2); */
   /*     ct_assert( size(pnts,1)>3 && size(pnts,1)<=MAXPNTS); */
   /*     ct_assert( deg>=2 && deg<=6);     */
   /* Use linear fitting by default */
   /* Use approximation by default */
   /* First, determine local orthogonal cordinate system. */
   /* tng is the local u coordinate, nrm is the local v coordinate */

   nrm[0] = nrms[0];
   nrm[1] = nrms[1];
   tng[0] = nrm[1];
   tng[1] = - nrm[0];
   /* Project onto local coordinate system */

   nverts = pnts_dim1;
   us_dim1 = nverts - strip;
   if (nverts < strip) {
      us_dim1 = 0;
   }
   memset(us, 0, (nverts - strip) * 8);
   ct_set_max(bs_dim1, nverts - strip, 0);
   memset(bs, 0, (nverts - strip) * 8);
   ws_row_dim1 = nverts - strip;
   if (nverts < strip) {
      ws_row_dim1 = 0;
   }
   for (i=0; i<= nverts - 1 - strip; i+=1) {
      ws_row[i] = 1.0;
   }
   us[0] = 0.0;
   ws_row[0] = 1.0;
   k = 0;
   for (i=0; i<=nverts - 1; i+=1) {
      if ((strip)&&(!i)) {
         continue;
      }
      else {
         k = 1 + k;
      }
      uu[0] =  pnts[i << 1] -  pnts[0];
      uu[1] =  pnts[1 + (i << 1)] -  pnts[1];
      us[k - 1] =  uu[0] *  tng[0] +  uu[1] *  tng[1];
      bs[k - 1] =  uu[0] *  nrm[0] +  uu[1] *  nrm[1];
      /* Compute normal-based weights */

      dtemp_0 =  nrms[i << 1] *  nrm[0] +  nrms[1 + (i << 1)] *  nrm[1];
      dtemp_1 = ct_max(dtemp_0, 0.0);
      ws_row[i - strip] = dtemp_1;
   }
   /* Compute the coefficients and store into bs */

   deg_1 = eval_vander_univar(us, us_dim1, bs, &bs_dim1, deg_1, ws_row, ws_row_dim1, strip, 0);
   dist = param *  vec[0] *  tng[0];
   dist = dist + param *  vec[1] *  tng[1];
   if (strip) {
      height = 0.0;
   }
   else {
      height = bs[0];
   }
   for (i=0; i<=deg_1 - 1; i+=1) {
     dtemp_2 = pow(dist, 1 + i);
     height = height +  bs[ 1 - strip + i] * dtemp_2;
   }
   /*% Change back to global coordinate system. */

   pnt_out[0] =  pnts[0] + dist *  tng[0] + height *  nrm[0];
   pnt_out[1] =  pnts[1] + dist *  tng[1] + height *  nrm[1];
   /*%  */

   return;
}

/*************************************************************
 *
 * FUNCTION: eval_vander_univar
 *
 *EVAL_VANDER_UNIVAR Evaluate Vandermonde matrix V.
 * [BS,DEGREE] = EVAL_VANDER_UNIVAR(US,BS,DEGREE,WS,STRIP) Evaluates 
 * Vandermonde matrix V and solves V\BS. It supports up to degree 6.
 * 
 * If strip is true, then the fitting is forced to pass through origin.
 * If usederiv is true, then bs will contain the derivatives of the 
 *     polynomial. Otherwise, bs will contain the coefficents.
 * See also EVAL_VANDER_BIVAR
 *************************************************************/

int eval_vander_univar(
   double *us, 
   int us_dim1, 
   double *bs, 
   int *bs_dim1, 
   int degree, 
   double *ws, 
   int ws_dim1, 
   int strip, 
   int usederiv)
{
   static double V[91];
   int V_dim1, V_dim2;
   double ws1[13];
   double ts[13];
   int ncols;
   double v[13];
   int v_dim1;
   double dtemp_0[13];
   static double dtemp_1[91];
   int dtemp_1_dim1;
   int degree_1_out, npnts, rnk, ncols_sub, n;
   double t, t2, vnrm2, dtemp_2, dtemp_3;
   static double weights[7] = 
             {
                1.0, 1.0, 2.0, 6.0, 24.0, 120.0, 720.0
             };
   static int ncoeffs[6] = 
             {
                2, 3, 4, 5, 6, 7
             };
   int itemp_0, itemp_1, itemp_2, itemp_3, i;
   int j, i3;
   double dtemp_4;

   degree_1_out = degree;
   /* Determine degree of fitting */

   npnts = us_dim1;
   /* ct_assert( degree<=length(ncoeffs) && degree>=1); */
   /* Determine degree of polynomial */

   ncols =  ncoeffs[degree - 1] - strip;
   while (( ncoeffs[degree_1_out - 1] - strip > npnts)&&(degree_1_out > 1)) {
      degree_1_out = degree_1_out - 1;
      ncols =  ncoeffs[degree_1_out - 1] - strip;
   }
   /*% Construct matrix */

   V_dim1 = ct_max(npnts, 0);
   ct_set_max(V_dim2, ncols, 0);
   memset(V, 0, npnts * ncols * 8);
   /* Allocate the maximum size */

   for (i=0; i<=V_dim1 - 1; i+=1) {
      V[V_dim2 * i] = 1.0;
   }
   for (i=0; i<=us_dim1 - 1; i+=1) {
      dtemp_0[i] = us[i];
   }
   itemp_0 = ct_max(V_dim1, 0);
   itemp_3 = 0;
   if (us_dim1 == 1) {
      for (i=0; i<=itemp_0 - 1; i+=1) {
         V[ 1 + V_dim2 * i - strip] = dtemp_0[0];
      }
   }
   else {
      for (i=0; i<=itemp_0 - 1; i+=1) {
         V[ 1 + V_dim2 * i - strip] = dtemp_0[itemp_3];
         itemp_3 = 1 + itemp_3;
      }
   }
   for (i=3 - strip; i<=ncols; i+=1) {
      itemp_1 = ct_max(V_dim1, 0);
      if (itemp_1 == 1) {
         dtemp_1_dim1 = us_dim1;
         for (j=0; j<=us_dim1 - 1; j+=1) {
            dtemp_1[j] =  V[i - 2] *  us[j];
         }
      }
      else {
         dtemp_1_dim1 = itemp_1;
         if (us_dim1 == 1) {
            for (j=0; j<=dtemp_1_dim1 - 1; j+=1) {
               dtemp_1[j] =  V[ V_dim2 * j - 2 + i] *  us[0];
            }
         }
         else {
            for (j=0; j<=dtemp_1_dim1 - 1; j+=1) {
               dtemp_1[j] =  V[ V_dim2 * j - 2 + i] *  us[j];
            }
         }
      }
      itemp_2 = ct_max(V_dim1, 0);
      itemp_3 = 0;
      if (dtemp_1_dim1 == 1) {
         for (j=0; j<=itemp_2 - 1; j+=1) {
            V[ V_dim2 * j - 1 + i] = dtemp_1[0];
         }
      }
      else {
         for (j=0; j<=itemp_2 - 1; j+=1) {
            V[ V_dim2 * j - 1 + i] = dtemp_1[itemp_3];
            itemp_3 = 1 + itemp_3;
         }
      }
   }
   /*% Scale rows to assign different weights to different points */

   if (degree_1_out > 2) {
      /* Scale weights to be inversely proportional to distance */

      dtemp_2 = 0.0;
      for (i=0; i<=us_dim1 - 1; i+=1) {
         dtemp_4 =  us[i] *  us[i];
         dtemp_2 = dtemp_2 + dtemp_4;
         ws1[i] = dtemp_4;
      }
      for (i=0; i<=us_dim1 - 1; i+=1) {
         ws1[i] =  ws1[i] + 0.01 * (dtemp_2 / (((double)npnts)));
      }
      if (degree_1_out < 4) {
         for (i=0; i<=npnts - 1; i+=1) {
            ws1[i] =  ws[i] /  sqrt(ws1[i]);
         }
      }
      else {
         for (i=0; i<=npnts - 1; i+=1) {
            ws1[i] =  ws[i] /  ws1[i];
         }
      }
   }
   else {
      for (i=0; i<=ws_dim1 - 1; i+=1) {
         ws1[i] = ws[i];
      }
   }

   for (i=0; i<=npnts - 1; i+=1) {
      for (j=0; j<=V_dim2 - 1; j+=1) {
         V[ V_dim2 * i + j] =  V[ V_dim2 * i + j] *  ws1[i];
      }
      bs[i] =  bs[i] *  ws1[i];
   }
   /*% Scale columns to reduce condition number */

   memset(ts, 0, ncols * 8);
   for (i=0; i<=ncols - 1; i+=1) {
      t = 0.0;
      for (j=0; j<=npnts - 1; j+=1) {
         t = t +  V[ V_dim2 * j + i] *  V[ V_dim2 * j + i];
      }
      dtemp_3 = sqrt(t);
      for (j=0; j<=V_dim1 - 1; j+=1) {
         V[ V_dim2 * j + i] =  V[ V_dim2 * j + i] / dtemp_3;
      }
      ts[i] = dtemp_3;
   }
   /*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
   /* Perform Householder QR factorization to compute */
   /* [Q, V(1:n,1:n)] = qr(V(1:m,1:n),0); bs = Q'*bs; rnk=n; */

   rnk = ncols;
   for (i=0; i<=ncols - 1; i+=1) {
      v_dim1 = npnts - i;
      if (npnts < i) {
         v_dim1 = 0;
      }
      for (j=0; j<= npnts - 1 - i; j+=1) {
         v[j] = V[ V_dim2 * (i + j) + i];
      }
      t2 = 0.0;
      for (j=0; j<=v_dim1 - 1; j+=1) {
         t2 = t2 +  v[j] *  v[j];
      }
      t = sqrt(t2);
      if ( v[0] >= 0.0) {
         vnrm2 = sqrt(2.0 * (t2 +  v[0] * t));
         v[0] =  v[0] + t;
      }
      else {
         vnrm2 = sqrt(2.0 * (t2 -  v[0] * t));
         v[0] =  v[0] - t;
      }
      if (vnrm2 > 0.0) {
         for (j=0; j<=v_dim1 - 1; j+=1) {
            v[j] =  v[j] / vnrm2;
         }
      }
      /* Optimized version for  */
      /* V(k:npnts,k:ncols) = V(k:npnts,k:ncols) - 2*v*(v'*V(k:npnts,k:ncols)); */

      for (j=1 + i; j<=ncols; j+=1) {
         t2 = 0.0;
         for (i3=0; i3<=v_dim1 - 1; i3+=1) {
            t2 = t2 +  v[i3] *  V[ V_dim2 * (i + i3) - 1 + j];
         }
         t2 = t2 + t2;
         for (i3=0; i3<=v_dim1 - 1; i3+=1) {
            V[ V_dim2 * (i + i3) - 1 + j] =  V[ V_dim2 * (i + i3) - 1 + j] - t2 *  v[i3];
         }
      }
      /* Estimate rank of matrix */

      if (( fabs(V[ V_dim2 * i + i]) < 0.001)&&(rnk == ncols)) {
         rnk = i;
         break;
      }
      /* Optimized version for  */
      /* bs(k:npnts,:) = bs(k:npnts,:) - 2*v*(v'*bs(k:npnts,:)); */

      t2 = 0.0;
      for (j=0; j<=v_dim1 - 1; j+=1) {
         t2 = t2 +  v[j] *  bs[i + j];
      }
      t2 = t2 + t2;
      for (j=0; j<=v_dim1 - 1; j+=1) {
         bs[i + j] =  bs[i + j] - t2 *  v[j];
      }
   }
   /*%%%%%%%%%%%%%%%%%%%%% */

   ncols_sub = ncols;
   while (rnk < ncols_sub) {
      degree_1_out = degree_1_out - 1;
      if (!degree_1_out) {
         /* Matrix is singular. Consider curve as flat. */

         for (i=0; i<=(*bs_dim1) - 1; i+=1) {
            bs[i] = 0.0;
         }
         return degree_1_out;
      }
      ncols_sub =  ncoeffs[degree_1_out - 1] - strip;
   }
   /*%%%%%%%%%%%%%%%%%%%%% */
   /* Perform backward substitution to compute  */
   /* bs = triu(R(1:ncols,1:ncols))\bs; */

   for (i=ncols_sub; i>=1; i+=-1) {
      for (j=1 + i; j<=ncols_sub; j+=1) {
         bs[i - 1] =  bs[i - 1] -  V[ V_dim2 * (i - 1) - 1 + j] *  bs[j - 1];
      }
      if ( V[ V_dim2 * (i - 1) - 1 + i] == 0.0) {
         /* Singular matrix */

         printf("Matrix is singular.");
         /*#ok<WNTAG> */

         bs[i - 1] = 0.0;
      }
      else {
         bs[i - 1] =  bs[i - 1] /  V[ V_dim2 * (i - 1) - 1 + i];
      }
   }
   /*%%%%%%%%%%%%%%%%%%%%% */

   n = ct_min(7 - strip, ncols_sub);
   if (usederiv) {
      /* Force weights to be double precision. */

      for (i=0; i<=n - 1; i+=1) {
         bs[i] =  bs[i] *  weights[strip + i] /  ts[i];
      }
   }
   else {
      for (i=0; i<=n - 1; i+=1) {
         bs[i] =  bs[i] /  ts[i];
      }
   }
   return degree_1_out;
}

LOCAL	const int	Num_pqs_in_block = 1000;
struct _TRI_SURF {
  TRI	*tri;
  SURFACE *surf;
  CURVE   *c01, *c12, *c20;
  double   sqr_norm, dist;
  int     side;
};
typedef struct _TRI_SURF	TRI_SURF; 
#define Tri_surf(p)			((TRI_SURF *) (p)->pointer)
#define PQ_for_tri(tri)			((POINTER_Q *) Tri_workspace(tri))
#define tri_surface_from_queue(tri)	(Tri_surf(PQ_for_tri(tri)))
#define Bond_of_q(pq)		        ((BOND *)(pq)->pointer)
#define Tri_of_q(pq)		        (Tri_surf(pq)->tri)


LOCAL POINTER_Q *dequeue(
			 TRI       *tri,
			 POINTER_Q *pq)
{
  if (PQ_for_tri(tri))
    {
      if (head_of_pointer_queue(PQ_for_tri(tri))
	  == head_of_pointer_queue(pq))
	{
	  pq = delete_from_pointer_queue(PQ_for_tri(tri));
	  Tri_workspace(tri) = NULL;
	}
    }
  return pq;
}		/*end dequeue*/

LOCAL   POINTER_Q *alloc_and_add_to_queue(
	TRI       *t,
	SURFACE   *s,
	POINTER_Q *pq,
	int       nside)
{
        TRI_SURF    *ts;
	const double *nor;
	int	    i, j;

	pq = add_to_pointer_queue(NULL,pq);
	Tri_workspace(t) = (POINTER) pq;
	ts = tri_surface_from_queue(t);
	if (ts == NULL)
	  {
	    screen("ERROR in alloc_and_add_to_queue(), "
		   "tri_surface_from_queue() returns NULL\n");
	    clean_up(ERROR);
	  }
	ts->tri = t;
	nor = Tri_normal(t);
	ts->sqr_norm = Dot3d(nor,nor);

	/* a distant for further check, should be shift invariant */
	ts->dist = 0.0;
	for(i=0; i<3; i++)
	  for(j=0; j<3; j++)
	    ts->dist += Coords(Point_of_tri(t)[i])[j];
	
	ts->surf = s;
	ts->c01 = ts->c12 = ts->c20 = NULL;
	ts->side = nside;
	return pq;
}		/*end alloc_and_add_to_queue*/

LOCAL   void  tecplot_tri_queue(
	const char	*msg,
	FILE		*file,
	POINTER_Q 	*p_q)
{
        POINTER_Q 	*q;
	TRI_SURF 	*t_surf;
	FT_POINT		*p;
	TRI 		*tri;
	int		k, i, cnt = 0;
 
	double	pt[3] = { 0.9733961893900565,     -0.301512040795133,     -15.64756169180028  };
	double	pt1[3] = { -1.026603810609944,     -0.301512040795133,     -15.64756169180028 };
		
 
	if (p_q == NULL) 
	  {
	    (void) printf("tecplot_tri_queue NULL POINTER_Q %s\n", msg);
	    return;
	  }

	q = head_of_pointer_queue(p_q);
	while (q != tail_of_pointer_queue(p_q))
	  {
	    t_surf = Tri_surf(q);
	    tri = t_surf->tri;
	    q = q->next;
	    cnt++;
	  }
	(void) fprintf(file, "ZONE T=\"%s\" N=%d E=%d\nF=FEPOINT, ET=TRIANGLE\n",
		       msg, 3*cnt, cnt);

	k = 0;
	q = head_of_pointer_queue(p_q);
	while (q != tail_of_pointer_queue(p_q))
	  {
	    t_surf = Tri_surf(q);
	    tri = t_surf->tri;
	    for (i = 0; i < 3; i++)
	      {
		p = Point_of_tri(tri)[i];
		fprintf(file,"%-9g %-9g %-9g\n",Coords(p)[0],
			Coords(p)[1],Coords(p)[2]);
		
		if(distance_between_positions(Coords(Point_of_tri(tri)[i]), pt, 3)<0.05  || 
		   distance_between_positions(Coords(Point_of_tri(tri)[i]), pt1, 3)<0.05 )
		  {
		    /*printf("#de tri sort  k = %d\n", k); */
		    /*printf("sqr_norm = %24.16e, dist = %24.16e\n", t_surf->sqr_norm, t_surf->dist); */
		    /*print_tri(tri, t_surf->surf->interface); */
		  }
	      }
	    k++;
	    q = q->next;
	  }
	
	for(i=0; i<cnt; i++)
	  {
	    fprintf(file, "%d %d %d\n", 3*i+1, 3*i+2, 3*i+3);
	  }
}

LOCAL   void  tri_queue_test(
        const char	*msg,
        POINTER_Q 	*p_q)
{
        POINTER_Q 	*q;
	TRI_SURF 	*t_surf;
	FT_POINT		*p;
	TRI 		*tri;
	int		k, i;
	double		tst_pt[3] = {  0.0,     0.8985,                 1.47066};
	double		tst_pt1[3] = { 1.0,     0.8985,                 1.47066};
	double		tol = 1.0/40.0;
 
	if (p_q == NULL) 
	  {
	    (void) printf("tri_queue_test NULL POINTER_Q %s\n", msg);
	    return;
	  }
	
	k = 0;
	q = head_of_pointer_queue(p_q);
	while (q != tail_of_pointer_queue(p_q))
	  {
	    t_surf = Tri_surf(q);
	    tri = t_surf->tri;

	    for (i = 0; i < 3; i++)
	      {
		p = Point_of_tri(tri)[i];
		
		if(distance_between_positions(Coords(Point_of_tri(tri)[i]), tst_pt, 3)<tol  || 
		   distance_between_positions(Coords(Point_of_tri(tri)[i]), tst_pt1, 3)<tol )
		  {
		    printf("#de tri sort  k = %d\n", k);
		    printf("sqr_norm = %24.15e, dist = %24.15e\n", t_surf->sqr_norm, t_surf->dist);
		    print_tri(tri, t_surf->surf->interface);
		    
		    break;
		  }
	      }

	    k++;
	    q = q->next;
	  }
}

/*
 *       Second order surface redistribution function
 */
EXPORT  boolean redistribute_surface_o2(
	SURFACE		*s,
	RECT_GRID 	*gr,
	TRI_REDIST_PARAMS tri_params)
{
        HYPER_SURF *hs = Hyper_surf(s);
	HYPER_SURF_ELEMENT *hse;

	INTERFACE *intfc;
	FT_POINT	  *midp, *p;
	POINTER_Q *insert_queue, *delete_queue;
	TRI	  *tri, *oppt;
	boolean      status;
	int       i, nside, nf, nl, ns, nt;
	int	  dim;
	FILE	  *db_file;
	double	  coords[3];

	DEBUG_ENTER(redistribute_surface_o2)
	  
        set_pointer_queue_opts(PQ_BLOCK_SIZE,Num_pqs_in_block,PQ_ALLOC_TYPE,
				 "vmalloc",PQ_ALLOC_SIZE_FOR_POINTERS,
				 sizeof(TRI_SURF),0);

	if (debugging("high_order_redist"))
	  {
	    printf("Entering redistribute_surface_o2()\n");
	    printf("redist_order = %d\n",s->redist_order);
	  }
	intfc = s->interface;
	dim = intfc->dim;
	
	/*set the tolerance for tri_status */
	
	status = YES;
	insert_queue = delete_queue = NULL;
	nt = nf = nl = ns = 0;
	
	for (tri=first_tri(s); !at_end_of_tri_list(tri,s); tri=tri->next)
	  {
	    /* set normal, stupid way, but i do not know how to traverse vertex of a surface */
	    hse = Hyper_surf_element(tri);	    
	    for(i = 0; i < 3; i++) {
	      p = Point_of_tri(tri)[i];
	      WLSP_compute_normal3d0(p,hse,hs);
	    }  
	    /* end set normal */
	    nt++;
	    switch (tri_status(tri,gr,tri_params))
	      {
	      case BAD_ANGLE:	
		++nf;
	      case LARGE:
		++nl;
		insert_queue = alloc_and_add_to_queue(tri,s,insert_queue,nside);
		break;
	      case SMALL:
		++ns;
		delete_queue = alloc_and_add_to_queue(tri,s,delete_queue,nside);
		break;
	      case GOOD_ANGLE:
	      default:
		Tri_workspace(tri) = NULL;
		break;
	      }
	  }
	
	if(insert_queue==NULL && delete_queue==NULL)
	  {
	    DEBUG_ENTER(redistribute_surface_o2)
	      return YES;
	  }

	if(debugging("prt_que"))
	  {
	    char  ch[60];

	    sprintf(ch, "queue_tri_%d.plt", pp_mynode());
	    db_file = fopen(ch, "w");
	    (void) fprintf(db_file,"TITLE = \"tecplot surface\"\n"
			   "VARIABLES = \"x\", \"y\", \"z\"\n");
	    printf("#queue_tri open \n");

	    tecplot_tri_queue("insert", db_file, insert_queue);
	    tecplot_tri_queue("delete", db_file, delete_queue);
	    fclose(db_file);
	  }

	sort_pointer_queue(insert_queue,intfc,LONGEST);
	if(debugging("pt_surface"))
	  {
	    tecplot_surface_in_ball("in_bf", s);
	  }

	while (insert_queue)
	  {
	    insert_queue = head_of_pointer_queue(insert_queue);
	    tri = Tri_of_q(insert_queue);
	    
	    nside = find_scaled_extrem_edge(tri,gr,LONGEST);
	    insert_queue = dequeue(tri,insert_queue);
	    
	    if (is_side_bdry(tri,nside))
	      continue;
	    
	    oppt = Tri_on_side(tri,nside);	    
	    if(is_tri_in_queue(oppt,insert_queue))
	      insert_queue = dequeue(oppt,insert_queue);
	    if(is_tri_in_queue(oppt,delete_queue))
	      delete_queue = dequeue(oppt,delete_queue);

	    if(skip_bdry_tri(oppt) || skip_bdry_tri(tri))
	      continue;

	    /* find and make tri side mid point(quadratic mid point) */
	    hse = Hyper_surf_element(tri);	    
	    quadratic_mid_point_on_edge3d(coords,tri,nside,hse,hs,2);
	    midp = Point(coords);

	    if (!insert_point_in_tri_side(midp,nside,tri,s))
	      {
		printf("WARNING redistribute_surface_o2, "
		       "insert_point_in_tri_side fails.\n");
		status = NO;
	      }
	  }
	    
	sort_pointer_queue(delete_queue,intfc,SHORTEST);
	if(debugging("pt_surface"))
	  {
	    tecplot_surface_in_ball("de_bf", s);
	    tri_queue_test("deletei_bf_test",  delete_queue);
	  }

	while (delete_queue)
	  {
	    delete_queue = head_of_pointer_queue(delete_queue);
	    tri = Tri_of_q(delete_queue);

	    nside = find_scaled_extrem_edge(tri,gr,SHORTEST);
	    /*add_to_debug("delete_dup"); */
		
	    delete_queue = dequeue(tri, delete_queue);
	    if (act_delete)
	      continue;

	    if(!delete_min_side_of_tri_o2(tri,nside,s,&delete_queue,intfc))
	    {
		printf("WARNING, redistribute_surface_o2, "
		       "delete_min_side_of_tri_o2 fails.\n");
		status = NO;
	    }
	}
	    
	interface_reconstructed(intfc) = NO;
	
	DEBUG_LEAVE(redistribute_surface_o2)
	return status;
}		/*end redistribute_surface_o2*/

LOCAL   int find_scaled_extrem_edge(
	TRI		*tri,
	RECT_GRID	*grid,
	SPQ_FLAG	to_find)
{
	const double* const *s;
	double	h0 = grid->h[0], h1 = grid->h[1], h2 = grid->h[2];
	double	s00, s01, s02;
	double	s10, s11, s12;
	double	s20, s21, s22;
	double	len0, len1, len2;

	s = side_vector(tri);
	s00 = s[0][0]/h0; s01 = s[0][1]/h1; s02 = s[0][2]/h2;
	s10 = s[1][0]/h0; s11 = s[1][1]/h1; s12 = s[1][2]/h2;
	s20 = s[2][0]/h0; s21 = s[2][1]/h1; s22 = s[2][2]/h2;
	len0 = QDot3d(s0,s0); len1 = QDot3d(s1,s1); len2 = QDot3d(s2,s2);

	switch (to_find)
	{
	case LONGEST:
	    return (len0<len1) ? ((len1<len2) ? 2:1) : ((len0<len2) ? 2:0);
	case SHORTEST:
	    return (len0>len1) ? ((len1>len2) ? 2:1) : ((len0>len2) ? 2:0);
	default:
	    return -1;
	}
}		/*end find_scaled_extrem_edge*/

/*
*				tri_status():
*
*	Determines whether redistribution of a triangle is needed by
*	comparing the triangles normalized area with two tolerances.
*	In addition, this routine also checks the squared edge lengths and
*	determines their aspect ratios.
*	This routine has the following return values:
*
*	if (norm_area > max_sqr_area) return LARGE;
*	if (norm_area < min_sqr_area) return SMALL;
*	if (aspect ratio < aspect_tol) return BAD_ANGLE;
*	return GOOD_ANGLE;
*
*	The aspect ratio of a triangle is defined as A/(l0^2+l1^2+l2^2)
*	where A is the area of the triangle in the grid scaled metric
*	and the li are the lengths of the sides of the triangle in
*	grid scaled metric.
*/

LOCAL	TRI_STATUS tri_status(
	TRI *tri,
	RECT_GRID *gr,
	TRI_REDIST_PARAMS tri_params)
{
	double	*p0 = Coords(Point_of_tri(tri)[0]);
	double	*p1 = Coords(Point_of_tri(tri)[1]);
	double	*p2 = Coords(Point_of_tri(tri)[2]);
	double	s00, s01, s02;
	double	s10, s11, s12;
	double	s20, s21, s22;
	double	N0, N1, N2;
	double	h0 = gr->h[0], h1 = gr->h[1], h2 = gr->h[2];
	double	sqr_area;
	double	a2,len[3],tri_area;
	const double *nor = Tri_normal(tri);

	sqr_area = 0.25*Dot3d(nor,nor); 
	if (sqr_area <= tri_params.min_sqr_area)
	    return SMALL;

	s00 = (p1[0]-p0[0])/h0; s01 = (p1[1]-p0[1])/h1; s02 = (p1[2]-p0[2])/h2;
	s10 = (p2[0]-p1[0])/h0; s11 = (p2[1]-p1[1])/h1; s12 = (p2[2]-p1[2])/h2;
	s20 = (p0[0]-p2[0])/h0; s21 = (p0[1]-p2[1])/h1; s22 = (p0[2]-p2[2])/h2;
	QCross3d(s0,s2,N);

	tri_area = 0.5*sqrt(QDot3d(N,N));

			/* Check aspect ratio	*/
	len[0] = QDot3d(s0,s0);
	len[1] = QDot3d(s1,s1);
	len[2] = QDot3d(s2,s2);

	a2 = len[0]+len[1]+len[2];
	a2 = tri_area/sqr(a2);
	if (a2 < tri_params.aspect_tol2)
	    return BAD_ANGLE;

	if ((len[0] > tri_params.max_sqr_length) || 
	    (len[1] > tri_params.max_sqr_length) ||
	    (len[2] > tri_params.max_sqr_length))
	    return LARGE;

	if (sqr_area >= tri_params.max_sqr_area)
	    return LARGE;
	return GOOD_ANGLE;
}		/*end tri_status*/
LOCAL   boolean delete_min_side_of_tri_o2(
	TRI	  *tri,
	int	  side,
	SURFACE	  *s,
	POINTER_Q **pq,
	INTERFACE *intfc)
{
	TRI	*nbtri, *t, *nbt, **tmp_tris;
	TRI	*new_tris[500], *in_tris[200], *tris[2][100];
	FT_POINT	*p[4], *pt, *pmid, *plist[2][100];
	int	i, j, k, nt, np[2], nside, ntris[2];
	boolean	rm_flag;
	static	int	cnt = 0;
	FILE	*file;
	char	fname[100];

	DEBUG_ENTER(delete_min_side_of_tri_o2)
	  
	  /*printf("#tri side %d %d\n", tri, side);*/
	
	
	/* using the quadratic point coordinate */
	HYPER_SURF* hs = Hyper_surf(s);
	HYPER_SURF_ELEMENT* hse = Hyper_surf_element(tri);
	double coords[3];	    
	quadratic_mid_point_on_edge3d(coords,tri,side,hse,hs,2);

	p[0] = Point_of_tri(tri)[side];
	p[1] = Point_of_tri(tri)[Next_m3(side)];

	if(Boundary_point(p[0]) || Boundary_point(p[1]))
	{
	    DEBUG_LEAVE(delete_min_side_of_tri_o2)
	    return YES;
	}

	nbtri = Tri_on_side(tri,side);
	for(nside=0; nside<3; nside++)
	    if (Tri_on_side(nbtri,nside) == tri)
		break;
	/* nside now is the side of nbtri neighboring tri */
	p[2] = Point_of_tri(tri)[Prev_m3(side)];
	p[3] = Point_of_tri(nbtri)[Prev_m3(nside)];

	for(k=0; k<2; k++)
	{
	    ntris[k] = set_tri_list_around_point(p[k],tri,&tmp_tris,intfc);
	    for(i=0; i<ntris[k]; i++)
	    {
		tris[k][i] = tmp_tris[i];
	        *pq = dequeue(tris[k][i],*pq);
	    }

	    np[k] = 0;
	    /*finding bounding points except the 4 common points. */
	    for(i=0; i<ntris[k]; i++)
	    {
		t = tris[k][i];
		j = Vertex_of_point(t, p[k]);
		pt = Point_of_tri(t)[Prev_m3(j)];
		
		for(j=0; j<4; j++)
		    if(pt == p[j])
			break;
		if(j < 4)
		    continue;

		plist[k][np[k]] = pt;
		np[k]++;
	    }
	}

	/*skip the bdry case. */
	if(Boundary_point(p[2]) || Boundary_point(p[3]))
	{
	    DEBUG_LEAVE(delete_min_side_of_tri_o2)
	    return YES;
	}
	for(i=0; i<np[0]; i++)
	    if(Boundary_point(plist[0][i]))
	    {
		DEBUG_LEAVE(delete_min_side_of_tri_o2)
		return YES;
	    }
	for(i=0; i<np[1]; i++)
	    if(Boundary_point(plist[1][i]))
	    {
		DEBUG_LEAVE(delete_min_side_of_tri_o2)
		return YES;
	    }

	/*check if there are duplicate points in the bounding tris. */
	rm_flag = NO;
	if(np[0] > 0 && np[1] > 0)
	{
	    /*the general case, test duplicate points */
	    for(i=0; i<np[0]; i++)
		for(j=0; j<np[1]; j++)
		    if(plist[0][i] == plist[1][j])
			rm_flag = YES;
	}
	else if(np[0] == 0 && np[1] == 0)
	{
	    /*the tetrahedron case */
	    rm_flag = YES;
	}

	/*printf("#shape np %3d  %3d %3d\n", rm_flag, np[0], np[1]); */
	if(rm_flag)
	{
	    nt = 0;
	    for(k=0; k<2; k++)
		nt = merge_tris_set(in_tris, nt, tris[k], ntris[k]);
	    if(debugging("delete_dup"))
	    {
		sprintf(fname,"dup_min%d_%d.plt",pp_mynode(),cnt);
		cnt++;
		printf("debug file %s\n", fname);
		
		file = fopen(fname,"w");
		tecplot_show_tris("in_tris", in_tris, nt, file);
		fclose(file);
	    }
    
	    nt = remove_tris_and_seal_o2(new_tris, in_tris, nt, s, pq, intfc);
	    	 
	    if(debugging("delete_dup"))
	    {
		file = fopen(fname,"a");
		tecplot_show_tris("new_tris", new_tris, nt, file);
		fclose(file);
	    }
	    
	    DEBUG_LEAVE(delete_min_side_of_tri_o2)
	    return   nt==-1 ? NO : YES;
	}

	/*collapse two tris. */
	pmid = average_points(YES,p[0],Hyper_surf_element(tri),Hyper_surf(s),
				  p[1],Hyper_surf_element(tri),Hyper_surf(s));

	/* begin: added by duowang */
	Coords(pmid)[0] = coords[0];
	Coords(pmid)[1] = coords[1];
	Coords(pmid)[2] = coords[2];
	/* end: added by duowang */

	/*change the point for the surrounding tris. */
	for (i = 0; i < 2; ++i)
	{
	    for (j = 0; j < ntris[i]; ++j)
	    {
		t = tris[i][j];
		k = Vertex_of_point(t,p[i]);
		Point_of_tri(t)[k] = pmid;
		if ((t != tri) && (t != nbtri))
		    set_normal_of_tri(t);
	    }
	}

	/*change tri neighbor for tri. */
	nbt = Tri_on_side(tri,Next_m3(side));
	t = Tri_on_side(tri,Prev_m3(side));
	for(i=0; i<3; i++)
	{
	    if (Tri_on_side(t,i) == tri)
		Tri_on_side(t,i) = nbt;
	    if (Tri_on_side(nbt,i) == tri)
		Tri_on_side(nbt,i) = t;
	}
	
	/*change tri neighbor for nbtri. */
	nbt = Tri_on_side(nbtri,Next_m3(nside));
	t = Tri_on_side(nbtri,Prev_m3(nside));
	for (i = 0; i < 3; ++i)
	{
	    if (Tri_on_side(t,i) == nbtri)
		Tri_on_side(t,i) = nbt;
	    if (Tri_on_side(nbt,i) == nbtri)
		Tri_on_side(nbt,i) = t;
	}
	
	remove_tri_from_surface(tri,s,YES);
	remove_tri_from_surface(nbtri,s,YES);
	
	DEBUG_LEAVE(delete_min_side_of_tri_o2)
	return YES;
}

/*ARGSUSED*/
LOCAL   void sort_pointer_queue(
	POINTER_Q	*pq,
	INTERFACE	*intfc,
	SPQ_FLAG	flag)
{
	POINTER_Q	*pq1,*pq2;
	
	if (pq == NULL)
	    return;

	pq1 = head_of_pointer_queue(pq);
	while (pq1 != tail_of_pointer_queue(pq))
	{
	    pq2 = pq1->next;
	    if (flag == SHORTEST)
	    {
		if (compare_pointers(pq1, pq2))
	    	    exchange_queues(pq1,pq2);
	        
		while (pq2 != tail_of_pointer_queue(pq))
	        {
	    	    pq2 = pq2->next;
		    if (compare_pointers(pq1, pq2))
		    {
	    	        exchange_queues(pq1,pq2);
	    	    }
	        }
	    }
	    else if (flag == LONGEST)
	    {
		if (!compare_pointers(pq1, pq2))
	    	    exchange_queues(pq1,pq2);
	        
		while (pq2 != tail_of_pointer_queue(pq))
	        {
	    	    pq2 = pq2->next;
		    if (!compare_pointers(pq1, pq2))
		    {
	    	        exchange_queues(pq1,pq2);
	    	    }
	        }
	    }
	    pq1 = pq1->next;
	}
}		/*end sort_pointer_queue*/


LOCAL	int	remove_tris_and_seal_o2(
	TRI		**new_tris,
	TRI		**tris,
	int		nt,
	SURFACE		*s,
	POINTER_Q	**pq,
	INTERFACE	*intfc)
{
	TRI	*out_tris[500], **new_out_tris;
	int	i, num_out_tris, num_new_tris;

	DEBUG_ENTER(remove_tris_and_seal_o2);
	
	num_out_tris = bound_tris_set(out_tris, tris, nt);

	/*tris are already dequeue in delete_min_side_of_tri_o2 */
	for(i=0; i<nt; i++)
	    remove_tri_from_surface(tris[i], s, NO);
	    
	if(num_out_tris == 0)
	{
	    DEBUG_LEAVE(remove_tris_and_seal_o2);
	    return 0;
	}

	sep_common_point_from_loop(out_tris, num_out_tris, NULL, NULL, intfc);
	
	/*new null side tris can be added into out_tris */
	num_out_tris = sep_common_edge_from_tris(&new_out_tris, 
				out_tris, num_out_tris, intfc);

	/*since a smooth_null_loop is applied above, the positions of  */
	/*3 vertics of a tri is changed, all the bound tris should be  */
	/*removed from the que. */
	
	for(i=0; i<num_out_tris; i++)
	    *pq = dequeue(new_out_tris[i], *pq);

	num_new_tris = 0;
	nt = seal_all_loops_wo_constraint(new_tris, &num_new_tris, 
			new_out_tris, num_out_tris, 1, NO);
	/* if the 5th parameter is 0, then a centroid point will be introduced */
	nt = merge_tris_set(new_tris, num_new_tris, new_out_tris, nt);

	DEBUG_LEAVE(remove_tris_and_seal_o2);
	return nt;
}	/* end remove_tris_and_seal_o2 */

LOCAL	void	exchange_queues(
	POINTER_Q *pq1,
	POINTER_Q *pq2)
{
	TRI_SURF *ts1, *ts2, T;

	ts1 = Tri_surf(pq1);
	ts2 = Tri_surf(pq2);
	T = *ts1;
	*ts1 = *ts2;
	*ts2 = T;
	Tri_workspace(ts1->tri) = (POINTER) pq1;
	Tri_workspace(ts2->tri) = (POINTER) pq2;
}		/*end exchange_queues*/

LOCAL 	boolean 	compare_pointers(
	POINTER_Q	*pq1,
	POINTER_Q	*pq2)
{
	double	ave_norm, norm1, norm2;
	double	tol = 1.0e-8;

	norm1 = Tri_surf(pq1)->sqr_norm;
	norm2 = Tri_surf(pq2)->sqr_norm;
	ave_norm = (norm1 + norm2)*0.5;

	/*two tris have very similar area, compare the postions. */
	if( fabs(norm1 - norm2) < ave_norm*tol ) 
	    return   Tri_surf(pq1)->dist > Tri_surf(pq2)->dist;
	else
	    return   norm1 > norm2;
}

LOCAL   boolean    is_tri_in_queue(
       	TRI		*tri,
	POINTER_Q		*pq)
{
	POINTER_Q		*tri_q;

	for (tri_q = head_of_pointer_queue(pq); tri_q; tri_q = tri_q->next)
	{
	    if (Tri_of_q(tri_q) == tri)
		return YES;
	}
	return NO;
}		/*end is_tri_in_queue*/

#define         MAX_RING_PTS            100
void  quadratic_mid_point_on_edge3d(
        double *coords,
	TRI *tri,
	int nside,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF *hs,
	int ring)
{
            int np1, np2, npts1, npts2;
	    FT_POINT *pts1[MAX_RING_PTS], *pts2[MAX_RING_PTS];
	    double ngbpts1[MAX_RING_PTS][3], ngbpts2[MAX_RING_PTS][3];
	    FT_POINT *p;
	    double nrms1[MAX_RING_PTS][3],nrms2[MAX_RING_PTS][3];
	    int i;

	    p = Point_of_tri(tri)[nside];
	    
  
	    PointArrayRing2(p,hse,hs,&np1,&np2,pts1,pts2);
	    ngbpts1[0][0] = Coords(p)[0];
	    ngbpts1[0][1] = Coords(p)[1];
	    ngbpts1[0][2] = Coords(p)[2];


	    nrms1[0][0] = p->_nor0[0];
	    nrms1[0][1] = p->_nor0[1];
	    nrms1[0][2] = p->_nor0[2];
	    if (ring == 1)
	      npts1 = 1 + np1;
	    else
	      npts1 = 1 + np1 + np2 ;
	    
	    for(i = 0; i < np1; i++)
	      {
		ngbpts1[i+1][0] = Coords(pts1[i])[0];
		ngbpts1[i+1][1] = Coords(pts1[i])[1];
		ngbpts1[i+1][2] = Coords(pts1[i])[2];
		nrms1[i+1][0] = pts1[i]->_nor0[0];
		nrms1[i+1][1] = pts1[i]->_nor0[1];
		nrms1[i+1][2] = pts1[i]->_nor0[2];
	      }
	   
	    for(i = 0; i < np2; i++)
	      {
		ngbpts1[i+np1+1][0] = Coords(pts2[i])[0];
		ngbpts1[i+np1+1][1] = Coords(pts2[i])[1];
		ngbpts1[i+np1+1][2] = Coords(pts2[i])[2];
		nrms1[i+np1+1][0] = pts2[i]->_nor0[0];
		nrms1[i+np1+1][1] = pts2[i]->_nor0[1];
		nrms1[i+np1+1][2] = pts2[i]->_nor0[2];
	      }

	    p = Point_of_tri(tri)[Next_m3(nside)];
	    
	    PointArrayRing2(p,hse,hs,&np1,&np2,pts1,pts2);

	    ngbpts2[0][0] = Coords(p)[0];
	    ngbpts2[0][1] = Coords(p)[1];
	    ngbpts2[0][2] = Coords(p)[2];

	    nrms2[0][0] = p->_nor0[0];
	    nrms2[0][1] = p->_nor0[1];
	    nrms2[0][2] = p->_nor0[2];
	    
	    if(ring == 1)
	      npts2 = 1 + np1;
	    else
	      npts2 = 1 + np1 + np2 ;
	    
	    for(i = 0; i < np1; i++)
	      {
		ngbpts2[i+1][0] = Coords(pts1[i])[0];
		ngbpts2[i+1][1] = Coords(pts1[i])[1];
		ngbpts2[i+1][2] = Coords(pts1[i])[2];
		nrms2[i+1][0] = pts1[i]->_nor0[0];
		nrms2[i+1][1] = pts1[i]->_nor0[1];
		nrms2[i+1][2] = pts1[i]->_nor0[2];

	      }
	    for(i = 0; i < np2; i++)
	      {
		ngbpts2[i+np1+1][0] = Coords(pts2[i])[0];
		ngbpts2[i+np1+1][1] = Coords(pts2[i])[1];
		ngbpts2[i+np1+1][2] = Coords(pts2[i])[2];
		nrms2[i+np1+1][0] = pts2[i]->_nor0[0];
		nrms2[i+np1+1][1] = pts2[i]->_nor0[1];
		nrms2[i+np1+1][2] = pts2[i]->_nor0[2];

	      }
	    /* set strip = 1*/
	    polyfit3d_walf_edge(coords,*ngbpts1,npts1,*nrms1,npts1,*ngbpts2,npts2,*nrms2,npts2,0.5,2,1);
}

/*end quadratic_mid_point_on_edge3d*/

/*************************************************************
 *
 * FUNCTION: polyfit3d_walf_edge
 *
 * Compute the position of a point within an edge using
 *            weighted averaging of least-squares fittings.
 * Input:
 * ngbpnts1-2:Input points of size mx3, Its first column is x-coordinates,
 *            and its second column is y-coordinates. The first vertex will
 *            be used as the origin of the local coordinate system.
 * nrms1-2:   The normals at ngbptns
 * xi:        The parameter within the tangent line of edge
 * deg:       The degree of polynomial to fit, from 1 to 6
 * interp:    If true, the fit is interpolatory at vertices.
 * Output:
 * pnt:       The reconstructed point in the global coordinate system
 * See also polyfit3d_walf_tri, polyfit3d_walf_quad, polyfit3d_cmf_edge
 *************************************************************/

void  polyfit3d_walf_edge(
   double pnt_out[3], 
   double *ngbpnts1, 
   int ngbpnts1_dim1, 
   double *nrms1, 
   int nrms1_dim1, 
   double *ngbpnts2, 
   int ngbpnts2_dim1, 
   double *nrms2, 
   int nrms2_dim1, 
   double xi, 
   int deg, 
   int interp)
{
   int deg_1, i;
   double pos[3];
   double pnt1[3], pnt2[3];

   deg_1 = deg;
   /* Use quadratic fitting by default */

   if (!deg_1) {
      deg_1 = 2;
   }
   /* Do not use interpolation by default */
   /* Compute face normal vector and the local coordinate */

   for (i=0; i<=2; i+=1) {
      pos[i] = (1.0 - xi) *  ngbpnts1[i] + xi *  ngbpnts2[i];
   }
   /* Interpolate using vertex-based polynomial fittings at two vertices */
   polyfit3d_walf_vertex(pnt1, ngbpnts1, ngbpnts1_dim1, nrms1, nrms1_dim1, pos, deg_1, interp);
   polyfit3d_walf_vertex(pnt2, ngbpnts2, ngbpnts2_dim1, nrms2, nrms2_dim1, pos, deg_1, interp);
   /* Compute weighted average of the two points */

   for (i=0; i<=2; i+=1) {
      pnt_out[i] = (1.0 - xi) *  pnt1[i] + xi *  pnt2[i];
   }
   return;
}

/*************************************************************
 *
 * FUNCTION: polyfit3d_walf_vertex
 *
 * Construct a local polynomial fitting and then interpolate.
 *    The function constructs the fitting using points pnts(:,1:3) in
 *    a local uv coordinate system with pnts(1,1:3) as the origin
 *    and nrms(1,1:3) vertical axis, and then interpolates to the point
 *    with u=param to obtain its coordinates.
 * Input:
 * pnts:   Input points of size mx3, Its first column is x-coordinates,
 *         and its second column is y-coordinates. The first vertex will
 *         be used as the origin of the local coordinate system.
 * nrms:   The normals at pnts
 * pos:    The point to be interpolated.
 * deg:    The degree of polynomial to fit, from 0 to 6
 * interp:  If 1, the fit is interpolatory at pnts(1,:) (in other words,
 *         the fit passes through point pnts(1,:)). If 0, the fit does not
 *         pass the point pnts(1,:), useful for a noisy inputs.
 * Output:
 * pnt:    The interpolated coordinates in the global coordinate system
 *         for the point with u=param in the local coordinate system.
 *************************************************************/

void  polyfit3d_walf_vertex(
   double pnt_out[3], 
   double *pnts, 
   int pnts_dim1, 
   double *nrms, 
   int nrms_dim1, 
   double pos[3], 
   int deg, 
   int interp)
{
   static double us[256];
   int us_dim1;
   static double bs[128];
   int bs_dim1;
   static double ws_row[128];
   int ws_row_dim1;
   static double us1[256];
   static double bs1[128];
   static double ws_row1[128];
   int ws_row1_dim1;
   int deg_out, nverts, k, index, jj;
   double nrm[3], absnrm[3], t1[3], t2[3];
   double h, u, v, height, dtemp_0;
   int tc;
   static double V[28];
   double dtemp_1, dtemp_2, dtemp_3, dtemp_4, dtemp_5;
   double dtemp_6[3];
   int itemp_0, i1, j;
   double dtemp_7, dtemp_8, dtemp_9;

   for (i1=0; i1<=2; i1+=1) {
      absnrm[i1] = fabs(nrms[i1]);
      nrm[i1] = nrms[i1];
   }
   if (( absnrm[0] >  absnrm[1])&&( absnrm[0] >  absnrm[2])) {
      t1[0] = 0.0;
      t1[1] = 1.0;
      t1[2] = 0.0;
   }
   else {
      t1[0] = 1.0;
      t1[1] = 0.0;
      t1[2] = 0.0;
   }
   dtemp_0 = 0.0;
   for (i1=0; i1<=2; i1+=1) {
      dtemp_0 = dtemp_0 +  t1[i1] *  nrm[i1];
   }
   dtemp_1 = 0.0;
   for (i1=0; i1<=2; i1+=1) {
      dtemp_7 =  t1[i1] - dtemp_0 *  nrm[i1];
      dtemp_1 = dtemp_1 + dtemp_7 * dtemp_7;
      t1[i1] = dtemp_7;
   }
   dtemp_2 = sqrt(dtemp_1);
   for (i1=0; i1<=2; i1+=1) {
      t1[i1] =  t1[i1] / dtemp_2;
   }
   /*CROSS_COL Efficient routine for computing cross product of two  */
   /*3-dimensional column vectors. */
   /* CROSS_COL(A,B) Efficiently computes the cross product between */
   /* 3-dimensional column vector A, and 3-dimensional column vector B. */

   t2[0] =  nrm[1] *  t1[2] -  nrm[2] *  t1[1];
   t2[1] =  nrm[2] *  t1[0] -  nrm[0] *  t1[2];
   t2[2] =  nrm[0] *  t1[1] -  nrm[1] *  t1[0];
   /*% Project onto local coordinate system */

   nverts = pnts_dim1;
   us_dim1 = nverts - interp;
   if (nverts < interp) {
      us_dim1 = 0;
   }
   memset(us, 0, ((nverts - interp) << 1) * 8);
   ct_set_max(bs_dim1, nverts - interp, 0);
   memset(bs, 0, (nverts - interp) * 8);
   us[0] = 0.0;
   us[1] = 0.0;
   for (i1=1 + interp; i1<=nverts; i1+=1) {
      k = i1 - interp;
      dtemp_3 = 0.0;
      dtemp_4 = 0.0;
      dtemp_5 = 0.0;
      for (j=0; j<=2; j+=1) {
         dtemp_9 =  pnts[ 3 * i1 - 3 + j] -  pnts[j];
         dtemp_3 = dtemp_3 + dtemp_9 *  t1[j];
         dtemp_4 = dtemp_4 + dtemp_9 *  t2[j];
         dtemp_5 = dtemp_5 + dtemp_9 *  nrm[j];
      }
      us[(k - 1) << 1] = dtemp_3;
      us[1 + ((k - 1) << 1)] = dtemp_4;
      bs[k - 1] = dtemp_5;
   }
   /* tol=1./2.0; */

   h = compute_resolution(pnts, pnts_dim1);
   for (i1=0; i1<=2; i1+=1) {
      dtemp_6[i1] = pnts[i1];
   }
   compute_cmf_weights(ws_row, &ws_row_dim1, &tc, dtemp_6, pnts, pnts_dim1, nrms, nrms_dim1, h, interp, 0.258819045102521017032159988958);
   /* Compute the coefficients and store into bs */

   if (tc) {
      memset(us1, 0, ((nverts - interp) << 1) * 8);
      memset(bs1, 0, (nverts - interp) * 8);
      ws_row1_dim1 = nverts - interp;
      if (nverts < interp) {
         ws_row1_dim1 = 0;
      }
      memset(ws_row1, 0, (nverts - interp) * 8);
      index = 0;
      for (i1=0; i1<=us_dim1 - 1; i1+=1) {
         if ( ws_row[i1] > 0.0) {
            index = 1 + index;
            us1[(index << 1) - 2] = us[i1 << 1];
            us1[(index << 1) - 1] = us[1 + (i1 << 1)];
            bs1[index - 1] = bs[i1];
            ws_row1[index - 1] = ws_row[i1];
         }
      }
      us_dim1 = index;
      bs_dim1 = index;
      for (i1=0; i1<=index - 1; i1+=1) {
         us[i1 << 1] = us1[i1 << 1];
         us[1 + (i1 << 1)] = us1[1 + (i1 << 1)];
         bs[i1] = bs1[i1];
      }
      if (ws_row1_dim1 > 1) {
         itemp_0 = index;
      }
      else {
         itemp_0 = 1;
      }
      ws_row_dim1 = itemp_0;
      for (i1=0; i1<=itemp_0 - 1; i1+=1) {
         ws_row[i1] = ws_row1[i1];
      }
      /* Filter out points with zero or negative weights. */
      /*us = us(ws_row>0,:); bs = bs(ws_row>0,:); ws_row = ws_row(ws_row>0); */
   }
   deg_out = eval_vander_bivar_cmf(us, us_dim1, bs, &bs_dim1, deg, ws_row, ws_row_dim1, interp, 1);
   /*% project the point into u-v plane and evaluate its value */

   u = 0.0;
   v = 0.0;
   for (i1=0; i1<=2; i1+=1) {
      dtemp_8 =  pos[i1] -  pnts[i1];
      u = u + dtemp_8 *  t1[i1];
      v = v + dtemp_8 *  t2[i1];
   }
   /* Evaluate the polynomial */

   memset(V, 0, 224);
   V[0] = u;
   V[1] = v;
   jj = 2;
   for (i1=2; i1<=deg_out; i1+=1) {
      jj = 1 + jj;
      V[jj - 1] =  V[ jj - 1 - i1] * u;
      for (j=0; j<=i1 - 1; j+=1) {
         jj = 1 + jj;
         V[jj - 1] =  V[ jj - 2 - i1] * v;
      }
   }
   if (interp) {
      height = 0.0;
   }
   else {
      height = bs[0];
   }
   for (i1=0; i1<=jj - 1; i1+=1) {
      height = height +  bs[ 1 - interp + i1] *  V[i1];
   }
   /*% Change back to global coordinate system. */

   for (i1=0; i1<=2; i1+=1) {
      pnt_out[i1] =  pnts[i1] + u *  t1[i1] + v *  t2[i1] + height *  nrm[i1];
   }
   return;
}

/*************************************************************
 *
 * FUNCTION: eval_vander_bivar_cmf
 *
 *EVAL_VANDER_BIVAR_CMF Evaluate generalized Vandermonde matrix.
 * [BS,DEGREE] = EVAL_VANDER_BIVAR_CMF(US,BS,DEGREE,WS, INTERP, SAFEGUARD) 
 * Evaluates generalized Vandermonde matrix V, and solve V\BS.
 * It supports up to degree 6.
 * 
 * If interp is true, then the fitting is forced to pass through origin.
 * Note: the only difference from EVAL_VANDER_UNIVAR is ws is not 
 *       computed inside this function
 * See also EVAL_VANDER_BIVAR
 *************************************************************/

int eval_vander_bivar_cmf(
   double *us, 
   int us_dim1, 
   double *bs, 
   int *bs_dim1, 
   int degree, 
   double *ws, 
   int ws_dim1, 
   int interp, 
   int safeguard)
{
   static int V[3584];
   int V_dim1, V_dim2;
   static double V_1[3584];
   int V_1_dim1, V_1_dim2;
   int ts[28];
   int ts_dim1;
   static double ts_1[28];
   int ts_1_dim1;
   int D[28];
   int D_dim1;
   static double D_1[28];
   int D_1_dim1;
   int degree_1_out, npnts, ncols, rnk, ncols_sub;
   int x, y, x_1, y_1, x_2;
   int y_2, i1, j;

   degree_1_out = degree;
   /* Determine degree of fitting */

   npnts = us_dim1;
   /* Determine degree of polynomial */

   x_2 = ((degree + 2) * (degree + 1)) >> 1;
   y_2 = x_2;
   ncols = y_2 - interp;
   while ((npnts < ncols)&&(degree_1_out > 1)) {
      degree_1_out = degree_1_out - 1;
      x_1 = ((2 + degree_1_out) * (1 + degree_1_out)) >> 1;
      y_1 = x_1;
      ncols = y_1 - interp;
   }
   /*% Construct matrix */

   V_dim1 = ct_max(npnts, 0);
   ct_set_max(V_dim2, ncols, 0);
   memset(V, 0, npnts * ncols * 4);
   /* Allocate the maximum size */

   construct_vander_bivar(V_1, &V_1_dim1, &V_1_dim2, us, us_dim1, degree_1_out, interp, V, V_dim1, V_dim2);
   /*% Scale rows to assign different weights to different points */

   for (i1=0; i1<=npnts - 1; i1+=1) {
      for (j=0; j<=V_1_dim2 - 1; j+=1) {
         V_1[ V_1_dim2 * i1 + j] =  V_1[ V_1_dim2 * i1 + j] *  ws[i1];
      }
      bs[i1] =  bs[i1] *  ws[i1];
   }
   /*% Scale columns to reduce condition number */

   ct_set_max(ts_dim1, ncols, 0);
   memset(ts, 0, ncols * 4);
   rescale_matrix(ts_1, &ts_1_dim1, V_1, &V_1_dim1, &V_1_dim2, ncols, ts, ts_dim1);
   /*% Perform Householder QR factorization */

   ct_set_max(D_dim1, ncols, 0);
   memset(D, 0, ncols * 4);
   qr_safeguarded(D_1, &D_1_dim1, &rnk, V_1, &V_1_dim1, &V_1_dim2, ncols, D, D_dim1);
   /*% Adjust degree of fitting */

   ncols_sub = ncols;
   while (rnk < ncols_sub) {
      degree_1_out = degree_1_out - 1;
      if (!degree_1_out) {
         /* Matrix is singular. Consider surface as flat. */

         for (i1=0; i1<=(*bs_dim1) - 1; i1+=1) {
            bs[i1] = 0.0;
         }
         return degree_1_out;
      }
      x = ((2 + degree_1_out) * (1 + degree_1_out)) >> 1;
      y = x;
      ncols_sub = y - interp;
   }
   /*% Compute Q'bs */

   compute_qtb(V_1, V_1_dim1, V_1_dim2, bs, bs_dim1, ncols_sub);
   /*% Perform backward substitution and scale the solutions. */

   for (i1=0; i1<=ncols_sub - 1; i1+=1) {
      V_1[ V_1_dim2 * i1 + i1] = D_1[i1];
   }
   backsolve_bivar_safeguarded(V_1, V_1_dim1, V_1_dim2, bs, bs_dim1, degree_1_out, interp, ts_1, ts_1_dim1);
   return degree_1_out;
}

/*************************************************************
 *
 * FUNCTION: backsolve_bivar_safeguarded
 *
 * Perform backward substitution with safeguards to downgrade the order.
 *     [bs,deg_out] = backsolve_bivar_safeguarded(R, bs, degree, interp, ws)
 *************************************************************/

void  backsolve_bivar_safeguarded(
   double *R, 
   int R_dim1, 
   int R_dim2, 
   double *bs, 
   int *bs_dim1, 
   int degree, 
   int interp, 
   double *ws, 
   int ws_dim1)
{
   static double tb[128];
   double tols[8];
   int cstart, cend, ncols, x, y;
   int cond;
   int x_1, y_1, x_2, y_2, x_3;
   int y_3, i, j, k;
   double dtemp_0, dtemp_1;

   /* First, solve for reference solutions for each degree */

   for (i=0; i<=(*bs_dim1) - 1; i+=1) {
      tb[i] = bs[i];
   }
   if (interp) {
      tb[0] =  tb[0] /  R[0];
   }
   cstart = 2 - interp;
   for (i=0; i<=degree - 1; i+=1) {
      x_3 = ((2 + i) * (3 + i)) >> 1;
      y_3 = x_3;
      cend = y_3 - interp;
      /* Solve for tb for its current block alone */

      for (j=cend; j>=cstart; j+=-1) {
         for (k=1 + j; k<=cend; k+=1) {
            tb[j - 1] =  tb[j - 1] -  R[ R_dim2 * (j - 1) - 1 + k] *  tb[k - 1];
         }
         cond =  R[ R_dim2 * (j - 1) - 1 + j] != 0.0;
         
         tb[j - 1] =  tb[j - 1] /  R[ R_dim2 * (j - 1) - 1 + j];
      }
      cstart = 1 + cend;
   }
   /* Compute tolerances from solutions. For each degree, we set the */
   /* tolerance to be the maximum absolute value of the standalone */
   /* solutions of this degree and lower. */
   /* Each right-hand side column has its own tolerance. */

   memset(tols, 0, degree * 8);
   cstart = 2 - interp;
   for (i=0; i<=degree - 1; i+=1) {
      x_2 = ((2 + i) * (3 + i)) >> 1;
      y_2 = x_2;
      cend = y_2 - interp;
      if (i > 0) {
         tols[i] = tols[i - 1];
      }
      for (j=cstart; j<=cend; j+=1) {
         dtemp_1 = fabs(tb[j - 1]);
         dtemp_0 = ct_max(tols[i], dtemp_1);
         tols[i] = dtemp_0;
      }
      cstart = 1 + cend;
   }
   /* Second, solve for each degree in decending order */

   x_1 = ((degree + 2) * (degree + 1)) >> 1;
   y_1 = x_1;
   ncols = y_1 - interp;
   cend = ncols;
   for (i=degree; i>=((int)interp); i+=-1) {
      x = (i * (1 + i)) >> 1;
      y = x;
      cstart = 1 - interp + y;
      for (j=cend; j>=cstart; j+=-1) {
         for (k=1 + j; k<=ncols; k+=1) {
            bs[j - 1] =  bs[j - 1] -  R[ R_dim2 * (j - 1) - 1 + k] *  bs[k - 1];
         }
         bs[j - 1] =  bs[j - 1] /  R[ R_dim2 * (j - 1) - 1 + j];
      }
      /* Check whether the degree should be decreased. */
      /* For degree>=3, we can decrease the degree down to 2. */
      /* ust 0.1*tol as the tolerance for now */

      if ((degree > 2)&&(i >= 2)) {
         for (j=cstart; j<=cend; j+=1) {
            if ( fabs( bs[j - 1] -  tb[j - 1]) > 0.1 *  tols[i - 1]) {
               for (k=0; k<=cend - cstart; k+=1) {
                  bs[ cstart - 1 + k] = tb[ cstart - 1 + k];
               }
               for (k=0; k<= (*bs_dim1) - 1 - cend; k+=1) {
                  bs[cend + k] = 0.0;
               }
               break;
            }
         }
      }
      cend = cstart - 1;
   }
   /* Scale bs back if tb is given. */

   for (i=0; i<=ncols - 1; i+=1) {
      bs[i] =  bs[i] /  ws[i];
   }
   return;
}

/*************************************************************
 *
 * FUNCTION: compute_qtb
 *
 *************************************************************/

void  compute_qtb(
   double *Q, 
   int Q_dim1, 
   int Q_dim2, 
   double *bs, 
   int *bs_dim1, 
   int ncols)
{
   int nrow, i, j;
   double t2;

   nrow = Q_dim1;
   for (i=0; i<=ncols - 1; i+=1) {
      /* Optimized version for */
      /* bs(k:nrow,:) = bs(k:nrow,:) - 2*v*(v'*bs(k:nrow,:)), */
      /* where v is Q(k:npngs) */

      t2 = 0.0;
      for (j=1 + i; j<=nrow; j+=1) {
         t2 = t2 +  Q[ Q_dim2 * (j - 1) + i] *  bs[j - 1];
      }
      t2 = t2 + t2;
      for (j=1 + i; j<=nrow; j+=1) {
         bs[j - 1] =  bs[j - 1] - t2 *  Q[ Q_dim2 * (j - 1) + i];
      }
   }
   return;
}

/*************************************************************
 *
 * FUNCTION: qr_safeguarded
 *
 * Compute Householder QR factorization with safeguards.
 * It compares the diagonal entries with the given tolerance to
 * determine whether the matrix is nearly singular. It is
 * specialized for performing polynomial fittings.
 * It saves Householder reflector vectors into lower triangular part A.
 * Save diagonal part of R into D, and upper triangular part (excluding
 * diagonal) of R into upper triangular part of A.
 *************************************************************/

void  qr_safeguarded(
   double *D_1_out, 
   int *D_1_out_dim1, 
   int *rnk_out, 
   double *A, 
   int *A_dim1, 
   int *A_dim2, 
   int ncols, 
   int *D, 
   int D_dim1)
{
   static double v[3584];
   int v_dim1;
   static double dtemp_0[3584];
   double t2, t, vnrm2;
   int itemp_0, itemp_1, i, j, i3;

   *D_1_out_dim1 = D_dim1;
   for (i=0; i<=D_dim1 - 1; i+=1) {
      D_1_out[i] = ((double) D[i]);
   }
   *rnk_out = ncols;
   for (i=0; i<=ncols - 1; i+=1) {
      v_dim1 = (*A_dim1) - i;
      if ((*A_dim1) < i) {
         v_dim1 = 0;
      }
      for (j=0; j<= (*A_dim1) - 1 - i; j+=1) {
         v[j] = A[ (*A_dim2) * (i + j) + i];
      }
      /* We don't need to worry about overflow, since A has been rescaled. */

      t2 = 0.0;
      for (j=0; j<=v_dim1 - 1; j+=1) {
         t2 = t2 +  v[j] *  v[j];
      }
      t = sqrt(t2);
      if ( v[0] >= 0.0) {
         vnrm2 = sqrt(2.0 * (t2 +  v[0] * t));
         v[0] =  v[0] + t;
      }
      else {
         vnrm2 = sqrt(2.0 * (t2 -  v[0] * t));
         v[0] =  v[0] - t;
      }
      if (vnrm2 > 0.0) {
         for (j=0; j<=v_dim1 - 1; j+=1) {
            v[j] =  v[j] / vnrm2;
         }
      }
      /* Optimized version for */
      /* A(k:npnts,k:ncols) = A(k:npnts,k:ncols) - 2*v*(v'*A(k:npnts,k:ncols)); */

      for (j=1 + i; j<=ncols; j+=1) {
         t2 = 0.0;
         for (i3=0; i3<=v_dim1 - 1; i3+=1) {
            t2 = t2 +  v[i3] *  A[ (*A_dim2) * (i + i3) - 1 + j];
         }
         t2 = t2 + t2;
         for (i3=0; i3<=v_dim1 - 1; i3+=1) {
            A[ (*A_dim2) * (i + i3) - 1 + j] =  A[ (*A_dim2) * (i + i3) - 1 + j] - t2 *  v[i3];
         }
      }
      D_1_out[i] = A[ (*A_dim2) * i + i];
      for (j=0; j<=v_dim1 - 1; j+=1) {
         dtemp_0[j] = v[j];
      }
      itemp_0 = (*A_dim1) - i;
      if ((*A_dim1) < i) {
         itemp_0 = 0;
      }
      itemp_1 = 0;
      if (v_dim1 == 1) {
         for (j=0; j<=itemp_0 - 1; j+=1) {
            A[ (*A_dim2) * (i + j) + i] = dtemp_0[0];
         }
      }
      else {
         for (j=0; j<=itemp_0 - 1; j+=1) {
            A[ (*A_dim2) * (i + j) + i] = dtemp_0[itemp_1];
            itemp_1 = 1 + itemp_1;
         }
      }
      /* Estimate rank of matrix */

      if (( fabs(D_1_out[i]) < 9.99999999999999954748111825886e-07)&&((*rnk_out) == ncols)) {
         *rnk_out = i;
         break;
      }
   }
   return;
}

/*************************************************************
 *
 * FUNCTION: rescale_matrix
 *
 *% Rescale the columns of a matrix to reduce condition number
 *************************************************************/

void  rescale_matrix(
   double *ts_1_out, 
   int *ts_1_out_dim1, 
   double *V, 
   int *V_dim1, 
   int *V_dim2, 
   int ncols, 
   int *ts, 
   int ts_dim1)
{
   static double v[3584];
   double s, w, dtemp_0, dtemp_1, dtemp_2;
   int i, j;

   *ts_1_out_dim1 = ts_dim1;
   for (i=0; i<=ts_dim1 - 1; i+=1) {
      ts_1_out[i] = ((double) ts[i]);
   }
   for (i=0; i<=ncols - 1; i+=1) {
      /*NORM2_VEC Computes the 2-norm of a vector. */
      /* NORM2_VEC(V) Computes the 2-norm of column vector V, */
      /*       taking care not to cause unnecessary overflow. */
      /* See also SQNORM2_VEC */

      w = 0.0;
      for (j=0; j<=(*V_dim1) - 1; j+=1) {
         dtemp_1 = fabs(V[ (*V_dim2) * j + i]);
         w = ct_max(dtemp_1, w);
         v[j] = V[ (*V_dim2) * j + i];
      }
      s = 0.0;
      if (w == 0.0) {
         /* W can be zero for max(0,nan,...) */
         /* adding all three entries together will make sure */
         /* NaN will not disappear. */

         for (j=0; j<=(*V_dim1) - 1; j+=1) {
            s = s +  v[j];
         }
      }
      else {
         for (j=0; j<=(*V_dim1) - 1; j+=1) {
            dtemp_0 =  v[j] / w;
            s = s + dtemp_0 * dtemp_0;
         }
         s = w *  sqrt(s);
      }
      dtemp_2 = s;
      if (dtemp_2 == 0.0) {
         dtemp_2 = 1.0;
      }
      else {
         for (j=0; j<=(*V_dim1) - 1; j+=1) {
            V[ (*V_dim2) * j + i] =  V[ (*V_dim2) * j + i] / dtemp_2;
         }
      }
      ts_1_out[i] = dtemp_2;
   }
   return;
}

/*************************************************************
 *
 * FUNCTION: construct_vander_bivar
 *
 *************************************************************/

void  construct_vander_bivar(
   double *V_1_out, 
   int *V_1_out_dim1, 
   int *V_1_out_dim2, 
   double *us, 
   int us_dim1, 
   int degree, 
   int interp, 
   int *V, 
   int V_dim1, 
   int V_dim2)
{
   int npnts, jj, i, j, k;

   *V_1_out_dim1 = V_dim1;
   *V_1_out_dim2 = V_dim2;
   for (i=0; i<=V_dim1 - 1; i+=1) {
      for (j=0; j<=V_dim2 - 1; j+=1) {
         V_1_out[ V_dim2 * i + j] = ((double) V[ V_dim2 * i + j]);
      }
   }
   npnts = us_dim1;
   for (i=0; i<=npnts - 1; i+=1) {
      V_1_out[V_dim2 * i] = 1.0;
      jj = 2 - interp;
      V_1_out[ V_dim2 * i - 1 + jj] = us[i << 1];
      jj = 1 + jj;
      V_1_out[ V_dim2 * i - 1 + jj] = us[1 + (i << 1)];
      for (j=2; j<=degree; j+=1) {
         jj = 1 + jj;
         V_1_out[ V_dim2 * i - 1 + jj] =  V_1_out[ V_dim2 * i - 1 + jj - j] *  us[i << 1];
         for (k=0; k<=j - 1; k+=1) {
            jj = 1 + jj;
            V_1_out[ V_dim2 * i - 1 + jj] =  V_1_out[ V_dim2 * i - 2 + jj - j] *  us[1 + (i << 1)];
         }
      }
   }
   return;
}

/*************************************************************
 *
 * FUNCTION: compute_cmf_weights
 *
 * Compute weights for continuous moving frames.
 * [ws,toocoarse] = compute_cmf_weights( pos, pnts, nrms, h, interp,tol)
 *************************************************************/

void  compute_cmf_weights(
   double *ws_out, 
   int *ws_out_dim1, 
   int *toocoarse_out, 
   double pos[3], 
   double *pnts, 
   int pnts_dim1, 
   double *nrms, 
   int nrms_dim1, 
   double h, 
   int interp, 
   double tol)
{
   double h2, d, costheta, dtemp_0;
   int i, i2;

   /* default tolerance is cos(75 degrees); */

   h2 = h * h;
   *toocoarse_out = 0;
   *ws_out_dim1 = ct_max(pnts_dim1, 0);
   memset(ws_out, 0, pnts_dim1 * 8);
   for (i=1 + interp; i<=pnts_dim1; i+=1) {
      d = 0.0;
      costheta = 0.0;
      for (i2=0; i2<=2; i2+=1) {
         dtemp_0 =  pnts[ 3 * i - 3 + i2] -  pos[i2];
         d = d + dtemp_0 * dtemp_0;
         costheta = costheta +  nrms[ 3 * i - 3 + i2] *  nrms[i2];
      }
      if (costheta > tol) {
         ws_out[ i - 1 - interp] = costheta *  exp((0.0 - d) / h2);
      }
      else {
         *toocoarse_out = 1;
      }
   }
   return;
}

/*************************************************************
 *
 * FUNCTION: compute_resolution
 *
 * Compute a resolution parameter for a given set of points.
 *    h = compute_resolution( ngbnpts)
 * where ngbnpts is m-by-2 or m-by-3. We use the second shortest distance 
 * from the first point in ngbnpnts to the others.
 *************************************************************/

double compute_resolution(
   double *ngbnpts, 
   int ngbnpts_dim1)
{
   double h_out, sqdists_1stmin, sqdists_2ndmin, sqd, dtemp_0;
   int cond;
   int i1, j;
   double dtemp_1, dtemp_2;

   cond = ngbnpts_dim1 > 1;
   if (ngbnpts_dim1 >= 2) {
      sqdists_1stmin = HUGE_VAL;
      sqdists_2ndmin = HUGE_VAL;
      for (i1=2; i1<=ngbnpts_dim1; i1+=1) {
         sqd = 0.0;
         for (j=0; j<=2; j+=1) {
            dtemp_2 =  ngbnpts[ 3 * i1 - 3 + j] -  ngbnpts[j];
            sqd = sqd + dtemp_2 * dtemp_2;
         }
         if (sqd < sqdists_1stmin) {
            sqdists_2ndmin = sqdists_1stmin;
            sqdists_1stmin = sqd;
         }
         else {
            sqdists_2ndmin = ct_min(sqd, sqdists_2ndmin);
         }
      }
      h_out = sqrt(sqdists_2ndmin);
   }
   else {
      dtemp_0 = 0.0;
      for (i1=0; i1<=2; i1+=1) {
         dtemp_1 =  ngbnpts[3 + i1] -  ngbnpts[i1];
         dtemp_0 = dtemp_0 + dtemp_1 * dtemp_1;
      }
      h_out = sqrt(dtemp_0);
   }
   return h_out;
}



