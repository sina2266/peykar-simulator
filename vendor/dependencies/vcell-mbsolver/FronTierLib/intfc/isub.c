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
*				isub.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	This file contains elementary routines for the computation of
*	angles, areas, vector and scalar products.
*/

#include <intfc/iloc.h>

	/* LOCAL Function Prototypes */
LOCAL	BDRY_SIDE rect_bdry_side_for_surface(int*,int*,SURFACE*,RECT_GRID*);
LOCAL	void  set_area_weighted_normal(TRI**,int,double*);
LOCAL   void  set_nel_max_normal(FT_POINT*,TRI**,int,double*);
LOCAL	void  debug_print_tri_list_around_point(FT_POINT*,TRI*,INTERFACE*,TRI**,
	                                            int,const char*);
LOCAL   void  reflect(double,double,double,double,double*,double*,RECT_GRID*);
LOCAL   void  set_cross_position(double,double,int,double,double,int,FT_POINT *);

EXPORT	double separation(
	FT_POINT		*p,
	FT_POINT		*q,
	int		dim)
{
	int		i;
	double		sep;

	sep = 0.0;
	for (i = 0; i < dim; ++i)
	    sep += sqr(Coords(q)[i] - Coords(p)[i]);
	return sqrt(sep);
}		/*end separation*/


/*
*			vector_product_on_bonds():
*
*	This routine computes the vector product in 2D and 3D.
*	The answer is placed in Pout and is a vector for three dimensions and
*	a single double (scalar) in two dimensions. 
*
*	Pout = bonds1 X bonds2.
*
*/

EXPORT 	void 	vector_product_on_bonds(
	BOND		*b1,
	BOND		*b2,
	int		dim,
	double		*Pout)
{
	double		vect1[MAXD],vect2[MAXD];
	int		i;

	for (i = 0; i < dim; ++i)
	{
	    vect1[i] = Coords(b1->end)[i] - Coords(b1->start)[i];
	    vect2[i] = Coords(b2->end)[i] - Coords(b2->start)[i];
	}

	switch (dim)
	{
	case 2:
	    *Pout = vect1[0]*vect2[1]-vect1[1]*vect2[0] ;
	    break;
	case 3:
	    Pout[0] = vect1[1]*vect2[2]-vect1[2]*vect2[1];
	    Pout[1] = vect1[2]*vect2[0]-vect1[0]*vect2[2];
	    Pout[2] = vect1[0]*vect2[1]-vect1[1]*vect2[0];
	    break;
	default:
	    break;
	}
	return;
}		/*end vector_product_on_bonds*/


/*
*			scalar_product_on_bonds():
*
* 	This function returns the scalar product of two bonds 
*	The dimension must be specified when this function is called. 
*	
*/
EXPORT double scalar_product_on_bonds(
	BOND		*b1,
	BOND		*b2,
	int		dim)
{
	double		sp;
	int		i;

	sp = 0.0;
	for (i = 0; i < dim; ++i) 
	    sp += (Coords(b1->end)[i] - Coords(b1->start)[i])  *
		  (Coords(b2->end)[i] - Coords(b2->start)[i]);
	return sp;
}		/*end scalar_product_on_bonds*/

EXPORT	double scaled_bond_length(
	BOND		*b,
	double		*h,
	int		dim)
{
	double tmp, ans = 0.0;
	int   i;

	for (i = 0; i < dim; ++i) 
	{
	    tmp = (Coords(b->end)[i] - Coords(b->start)[i])/h[i];
	    ans += sqr(tmp);
	}
	return sqrt(ans);
}		/*end scaled_bond_length*/

EXPORT	void scaled_tri_params(
	TRI *t,
	double *h,
	double *scaled_area,
	double *scaled_aspect_ratio)
{
	double *p0 = Coords(Point_of_tri(t)[0]);
	double *p1 = Coords(Point_of_tri(t)[1]);
	double *p2 = Coords(Point_of_tri(t)[2]);

        double           s00, s01, s02;
        double           s10, s11, s12;
        double           s20, s21, s22;
        double           N0, N1, N2;
        double           h0 = h[0], h1 = h[1], h2 = h[2];
        double           sqr_area;
	double		len[3];


        s00 = (p1[0]-p0[0])/h0; s01 = (p1[1]-p0[1])/h1; s02 = (p1[2]-p0[2])/h2;
        s10 = (p2[0]-p1[0])/h0; s11 = (p2[1]-p1[1])/h1; s12 = (p2[2]-p1[2])/h2;
        s20 = (p0[0]-p2[0])/h0; s21 = (p0[1]-p2[1])/h1; s22 = (p0[2]-p2[2])/h2;
        QCross3d(s0,s2,N);

        sqr_area = 0.25*QDot3d(N,N);
	len[0] = QDot3d(s0,s0);
        len[1] = QDot3d(s1,s1);
        len[2] = QDot3d(s2,s2);

	*scaled_area = sqrt(sqr_area);
	*scaled_aspect_ratio = (*scaled_area)/sqr(len[0]+len[1]+len[2]);
}	/* end scaled_tri_params */



/*
*			area_of_closed_curve():
*
*       Returns the SIGNED area of a closed curve.  The
*       area will be negative if the inside is on the
*       right side of the curve.
*
*       Returns ERROR_FLOAT if the curve is not closed.
*/

EXPORT	double area_of_closed_curve(
	CURVE		*c)
{
	double		area = 0.0, cp[MAXD];
	BOND		*b;
	double		*p0;
	int		dim = c->interface->dim;

	if (!is_closed_curve(c))
	    return ERROR_FLOAT;

	if (c->num_points < 3)
	    return 0.0;

	p0 = Coords(c->start->posn);
	area = 0.0;
	for (b = c->first->next; b && (b != c->last) ; b = b->next)
	{
	    (void) vector_product_on_points(p0,Coords(b->start),
					    Coords(b->end),dim,cp);
	    area += cp[0];
	}

	area *= 0.5;
	return area;
}		/*end area_of_closed_curve*/

EXPORT int  seg_index2d(
	int            nix,
	int            niy,
	GRID_DIRECTION dir,
	int 	       *gmax)
{
	int xmax = gmax[0];
	int ymax = gmax[1];
	if (nix < 0 || niy < 0 || nix > xmax || niy > ymax)
	    return INT_MIN;
	switch (dir)
	{
	case WEST:
	    if ((nix != 0))
	    	return 2*niy*xmax + niy + nix-1;
	    break;
	case EAST:
	    if ((nix != xmax))
	    	return 2*niy*xmax + niy + nix;
	    break;
	case SOUTH:
	    if ((niy != 0))
	    	return 2*niy*xmax + niy - 1 + nix - xmax;
	    break;
	case NORTH:
	    if ((niy != ymax))
	    	return 2*niy*xmax + niy + xmax + nix;
	    break;
	default:
	    screen("ERROR in seg_index2d(), invalid direction %d\n",dir);
	    clean_up(ERROR);
	}
	return INT_MIN;
}		/*end seg_index2d*/


EXPORT	P_LINK *add_to_hash_table(
	POINTER	pl,
	POINTER	pr,
	P_LINK	*hash_table,
	int	h_size)
{
	P_LINK   *link;
	uint64_t i, N, i_init;

	i_init = ptr2ull(pl) % h_size;
	link = hash_table+i_init;
	if ((link->pl == NULL) || (link->pl == pl))
	{
	    link->pl = pl;
	    link->pr = pr;
	    return link;
	}
	N = min(h_size-i_init,i_init+1);
	for (i = 1; i < N; ++i)
	{
	    link = hash_table + (i_init + i);
	    if ((link->pl == NULL) || (link->pl == pl))
	    {
	        link->pl = pl;
	        link->pr = pr;
	        return link;
	    }
	    link = hash_table + (i_init - i);
	    if ((link->pl == NULL) || (link->pl == pl))
	    {
	        link->pl = pl;
	        link->pr = pr;
	        return link;
	    }
	}
	for (i = i_init + N; i < h_size; ++i)
	{
	    link = hash_table + i;
	    if ((link->pl == NULL) || (link->pl == pl))
	    {
	        link->pl = pl;
	        link->pr = pr;
	        return link;
	    }
	}
	if (N <= i_init)
	{
	    for (i = 0; i <= i_init - N; ++i)
	    {
	        link = hash_table + i;
	        if ((link->pl == NULL) || (link->pl == pl))
	        {
	            link->pl = pl;
	            link->pr = pr;
	            return link;
	        }
	    }
	}
	return NULL;
}		/*end add_to_hash_table*/


EXPORT 	POINTER  find_from_hash_table(
	POINTER	pl,
	P_LINK	*hash_table,
	int	h_size)
{
	P_LINK   *link;
	uint64_t i, N, i_init;

	i_init = ptr2ull(pl) % h_size;
	link = hash_table+i_init;
	if (link->pl == pl)
	    return link->pr;
	if (link->pl == NULL)
	    return NULL;
	N = min(h_size-i_init,i_init+1);
	for (i = 1; i < N; ++i)
	{
	    link = hash_table + (i_init + i);
	    if (link->pl == pl)
	        return link->pr;
	    if (link->pl == NULL)
	        return NULL;
	    link = hash_table + (i_init - i);
	    if (link->pl == pl)
	        return link->pr;
	    if (link->pl == NULL)
	        return NULL;
	}
	for (i = i_init + N; i < h_size; ++i)
	{
	    link = hash_table + i;
	    if (link->pl == pl)
	        return link->pr;
	    if (link->pl == NULL)
	        return NULL;
	}
	if (N <= i_init)
	{
	    for (i = 0; i <= N - i_init; ++i)
	    {
	        link = hash_table + i;
	        if (link->pl == pl)
	            return link->pr;
	        if (link->pl == NULL)
	            return NULL;
	    }
	}
	return NULL;
}		/*end find_from_hash_table*/

EXPORT	void reset_hash_table(
	P_LINK		*hash_table,
	int		h_size)
{
	zero_scalar(hash_table,h_size*sizeof(P_LINK));
}		/*end reset_hash_table*/


EXPORT	BDRY_SIDE rect_bdry_side_for_curve(
	int		*idir,
	int		*iside,
	CURVE		*c,
	RECT_GRID	*gr)
{
	INTERFACE	*intfc = c->interface;
	double		*ps, *pe, d, m, min_d, p;
	int		i, dim = intfc->dim;
	int		lidir, liside;

	if (!is_bdry(c))
	    return NOT_A_BDRY;
	ps = Coords(c->start->posn);
	pe = Coords(c->end->posn);
	min_d = HUGE_VAL;
	for (i = 0; i < dim; ++i)
	{
	    d = fabs(ps[i] - pe[i]);
	    if (d < min_d)
	    {
	    	min_d = d;
	    	lidir = i;
	    }
	}
	p = 0.5*(ps[lidir] + pe[lidir]);
	m = 0.5*(gr->L[lidir] + gr->U[lidir]);
	liside = (p < m) ? 0 : 1;

	if (idir != NULL)
	    *idir = lidir;
	if (iside != NULL)
	    *iside = liside;
	switch (lidir)
	{
	case 0:
	    return (liside == 0) ? LEFT_BDRY : RIGHT_BDRY;
	case 1:
	    return (liside == 0) ? LOWER_BDRY : UPPER_BDRY;
	case 2:
	    return (liside == 0) ? ZMIN_BDRY : ZMAX_BDRY;
	}
	return NOT_A_BDRY;
}		/*end rect_bdry_side_for_curve*/

/*ARGSUSED*/
LOCAL	BDRY_SIDE rect_bdry_side_for_surface(
	int		*idir,
	int		*iside,
	SURFACE		*s,
	RECT_GRID	*gr)
{
	double		*L = gr->GL, *U = gr->GU;
	double		pbar[3];
	double		d, min_d;
	int		i;
	int		lidir, liside;

	average_position_of_surface(pbar,s);

	min_d = HUGE_VAL;
	liside = -1;
	for (i = 0; i < 3; ++i)
	{
	    d = fabs(L[i] - pbar[i]);
	    if (d < min_d)
	    {
	    	min_d = d;
	    	lidir = i;
		liside = 0;
	    }
	    d = fabs(U[i] - pbar[i]);
	    if (d < min_d)
	    {
	    	min_d = d;
	    	lidir = i;
		liside = 1;
	    }
	}

	if (idir != NULL)
	    *idir = lidir;
	if (iside != NULL)
	    *iside = liside;
	switch (lidir)
	{
	case 0:
	    return (liside == 0) ? LEFT_BDRY : RIGHT_BDRY;
	case 1:
	    return (liside == 0) ? LOWER_BDRY : UPPER_BDRY;
	case 2:
	    return (liside == 0) ? ZMIN_BDRY : ZMAX_BDRY;
	}
	return NOT_A_BDRY;
}		/*end rect_bdry_side_for_surface*/

EXPORT	void	average_position_of_surface(
	double	*pbar,
	SURFACE *s)
{
	TRI	*tri;
	double	*p0, *p1, *p2;
	int	i, nt;

	pbar[0] = pbar[1] = pbar[2] = 0.0;
	for (nt = 0, tri = first_tri(s); !at_end_of_tri_list(tri,s);
	     ++nt, tri = tri->next)
	{
	    p0 = Coords(Point_of_tri(tri)[0]);
	    p1 = Coords(Point_of_tri(tri)[1]);
	    p2 = Coords(Point_of_tri(tri)[2]);
	    for (i = 0; i < 3; ++i)
		pbar[i] += (p0[i] + p1[i] + p2[i])/3.0;
	}
	pbar[0] /= nt; pbar[1] /= nt; pbar[2] /= nt;
}		/*end average_position_of_surface */

/*
*		set_tri_list_around_point():
*
*	Locates a triangles directly connected to the give triangle
*	at the vertex v.  The list stops if a boundary bond is detected.
*/

EXPORT  int set_tri_list_around_point(
	FT_POINT     *p,
	TRI       *tri,
	TRI       ***ptris,
	INTERFACE *intfc)
{
	static TRI	**tris = NULL;
	static int	max_num_tris = 0;
	TRI		*nbtri;
	int		j, n;

	if (tris == NULL)
	{
	    max_num_tris = 24;
	    uni_array(&tris,max_num_tris,sizeof(TRI *));
	}

	nbtri = tri;
	n = 0;
	do {
	  if ((n+1) >= max_num_tris)
	  {
	    TRI **tmp_tris;
	    int i, nmax = max_num_tris + 24;

	    uni_array(&tmp_tris,nmax,sizeof(TRI *));
	    for (i = 0; i < max_num_tris; ++i)
	      tmp_tris[i] = tris[i];
	    free(tris);
	    tris = tmp_tris;
	    max_num_tris = nmax;
	  }
	  tris[n++] = nbtri;
	  if ((nbtri = Prev_tri_at_vertex(nbtri,p)) != NULL)
	  {
	    if (Next_tri_at_vertex(nbtri,p) != tris[n-1])
	    {
	      screen("ERROR in set_tri_list_around_point(), "
		     "inconsistent neighboring tris\n");
	      /*return -1; */
	      debug_print_tri_list_around_point(p,tri,intfc,tris,n,
			                        "set_tri_list_around_point");
	      (void) printf("nbtri\n");
	      print_tri(nbtri,intfc);
	      clean_up(ERROR);
	    }
	    if (nbtri == tri)
	    {
	      tris[n] = NULL;
	      *ptris = tris;
	      return n;
	    }
	    for (j = 0; j < n; ++j)
	    {
	      if (nbtri == tris[j])
	      {
	        screen("ERROR in set_tri_list_around_point(), "
		       "nbtri reappeared in tri list before tri\n");
		return -1;
	        debug_print_tri_list_around_point(p,tri,intfc,tris,n,
			                          "set_tri_list_around_point");
		clean_up(ERROR);
	      }
	    }
	  }
	} while (nbtri != NULL);

	/* Hit boundary */
	for (nbtri = tri; Next_tri_at_vertex(nbtri,p) != NULL;
				nbtri = Next_tri_at_vertex(nbtri,p));

	if (nbtri == tri) /* Previously generated list okay */
	{
	    tris[n] = NULL;
	    *ptris = tris;
	    return n;
	}

	n = 0;
	do {
	  if ((n+1) >= max_num_tris)
	  {
	    TRI **tmp_tris;
	    int i, nmax = max_num_tris + 24;

	    uni_array(&tmp_tris,nmax,sizeof(TRI *));
	    for (i = 0; i < max_num_tris; ++i)
	      tmp_tris[i] = tris[i];
	    free(tris);
	    tris = tmp_tris;
	    max_num_tris = nmax;
	  }
	  tris[n++] = nbtri;
	  if ((nbtri = Prev_tri_at_vertex(nbtri,p)) != NULL)
	  {
	    if (Next_tri_at_vertex(nbtri,p) != tris[n-1])
	    {
	      screen("ERROR in set_tri_list_around_point(), "
		     "inconsistent neighboring tris\n");
	      /*return -1; */
	      debug_print_tri_list_around_point(p,tri,intfc,tris,n,
			                        "set_tri_list_around_point");
	      (void) printf("nbtri\n");
	      print_tri(nbtri,intfc);
	      clean_up(ERROR);
	    }
	    for (j = 0; j < n; ++j)
	    {
	      if (nbtri == tris[j])
	      {
		screen("ERROR in set_tri_list_around_point(), "
		       "nbtri reappeared in tri list before tri\n");
		return -1;
		debug_print_tri_list_around_point(p,tri,intfc,tris,n,
			                          "set_tri_list_around_point");
		clean_up(ERROR);
	      }
	    }
	  }
	} while (nbtri != NULL);

	tris[n] = NULL;
	*ptris = tris;
	return n;
}		/*end set_tri_list_around_point*/

LOCAL	void	debug_print_tri_list_around_point(
	FT_POINT      *p,
	TRI        *tri,
	INTERFACE  *intfc,
	TRI        **tris,
	int        n,
	const char *func)
{
	double BBL[3], BBU[3];
	int   i;
	char  s[256];

	(void) printf("Current number of tris = %d\n",n);
	set_tri_list_bounding_box(tris,n,BBL,BBU,NO,YES);
	(void) sprintf(s,"%s-tris",func);
	gview_plot_triangle_list("",s,tris,n,
				 0.1,0.0,0.0,0.9,0.0,0.0,0.5,BBL,BBU);
	(void) sprintf(s,"%s-p",func);
	gview_plot_vertices("",s,&p,1,BBL,BBU);
	(void) printf("point %llu\n",point_number(p));
	print_general_vector("Coords(p) = ",Coords(p),3,"\n");
	(void) printf("tri\n");
	print_tri(tri,intfc);
	for (i = 0; i < n; ++i)
	{
	    (void) printf("tris[%d]\n",i);
	    print_tri(tris[i],intfc);
	}
}		/*end debug_print_tri_list*/

EXPORT int  seg_index3d(
	int            nix,
	int            niy,
	int            niz,
	GRID_DIRECTION dir,
	int 	       *gmax)
{
	int xmax = gmax[0];
	int ymax = gmax[1];
	int zmax = gmax[2];
	switch (dir)
	{
	case WEST:
	    return (niz*(ymax+1) + niy)*xmax + nix-1;
	case EAST:
	    return (niz*(ymax+1) + niy)*xmax + nix;
	case SOUTH:
	    return (niz*ymax + niy-1)*(xmax+1) + nix + xmax*(ymax+1)*(zmax+1);
	case NORTH:
	    return (niz*ymax + niy)*(xmax+1) + nix + xmax*(ymax+1)*(zmax+1);
	case LOWER:
	    return ((niz-1)*(ymax+1) + niy)*(xmax+1) + nix +
		   xmax*(ymax+1)*(zmax+1) + (xmax+1)*ymax*(zmax+1);
	case UPPER:
	    return (niz*(ymax+1) + niy)*(xmax+1) + nix +
		   xmax*(ymax+1)*(zmax+1) + (xmax+1)*ymax*(zmax+1);
	default:
	    screen("ERROR in seg_index3d(), invalid direction %d\n",dir);
	    clean_up(ERROR);
	}
	return INT_MIN;
}		/*end seg_index3d*/

EXPORT int  face_index3d(
	int            nix,
	int            niy,
	int            niz,
	GRID_DIRECTION dir,
	int 	       *gmax)
{
	int xmax = gmax[0];
	int ymax = gmax[1];
	int zmax = gmax[2];
	switch (dir)
	{
	case WEST:
	    return (niz*ymax + niy)*(xmax+1) + nix-1;
	case EAST:
	    return (niz*ymax + niy)*(xmax+1) + nix;
	case SOUTH:
	    return (niz*(ymax+1) + niy-1)*xmax + nix + (xmax+1)*ymax*zmax;
	case NORTH:
	    return (niz*(ymax+1) + niy)*xmax + nix + (xmax+1)*ymax*zmax;
	case LOWER:
	    return ((niz-1)*ymax + niy)*xmax + nix +
		   xmax*(ymax+1)*zmax + (xmax+1)*ymax*zmax;
	case UPPER:
	    return (niz*ymax + niy)*xmax + nix +
		   xmax*(ymax+1)*zmax + (xmax+1)*ymax*zmax;
	default:
	    screen("ERROR in face_index3d(), invalid direction %d\n",dir);
	    clean_up(ERROR);
	}
	return INT_MIN;
}		/*end seg_index3d*/


EXPORT	void rect_bdry_side_for_hyper_surf(
	int		*idir,
	int		*iside,
	HYPER_SURF	*hs,
	RECT_GRID	*gr)
{
	INTERFACE	*intfc = hs->interface;

	switch (intfc->dim)
	{
	case 1:
	{
	    double L = gr->L[0];
	    double U = gr->U[0];
	    FT_POINT *p = (FT_POINT *) hs;
	    *idir = 0;
	    if (fabs(Coords(p)[0] - L) < fabs(Coords(p)[0] - U))
	    {
	        *iside = 0;
	        return;
	    }
	    else
	    {
	        *iside = 1;
	        return;
	    }
	}
	case 2:
	    (void) rect_bdry_side_for_curve(idir,iside,Curve_of_hs(hs),gr);
	    return;
	case 3:
	    (void) rect_bdry_side_for_surface(idir,iside,Surface_of_hs(hs),gr);
	    return;
	}
}		/*end rect_bdry_side_for_hyper_surf*/

EXPORT	void i_add_comp_to_list(
	COMPONENT	comp,
	COMP_LIST	*comp_list,
	INTERFACE	*intfc)
{
	INTERFACE	*cur_intfc;
	COMPONENT	*comps = comp_list->comps;
	int		ncomps = comp_list->ncomps;
	int		max_ncomps = comp_list->max_ncomps;
	int		i;
	static int	increment = 10;

	/* Check if already in the list */
	for (i = 0; i < ncomps; ++i)
		if (comp == comps[i]) return;

	if (comps == NULL)
	{
		cur_intfc = current_interface();
		if (cur_intfc != intfc) set_current_interface(intfc);
		max_ncomps = increment;
		comp_list->max_ncomps = max_ncomps;
		comp_list->comps = comps =
			(COMPONENT *) store(max_ncomps*sizeof(COMPONENT));
		if (cur_intfc != intfc)
			set_current_interface(cur_intfc);
	}
	else if (ncomps+1 > max_ncomps)
	{
		COMPONENT  *tmp_comps = comps;

		cur_intfc = current_interface();
		if (cur_intfc != intfc) set_current_interface(intfc);
		max_ncomps += increment;
		comp_list->max_ncomps = max_ncomps;
		comp_list->comps = comps =
			(COMPONENT *) store(max_ncomps*sizeof(COMPONENT));
		for (i = 0; i < ncomps; ++i) comps[i] = tmp_comps[i];
		if (cur_intfc != intfc)
			set_current_interface(cur_intfc);
	}
	comps[comp_list->ncomps++] = comp;
}		/*end i_add_to_comp_list*/

EXPORT	boolean i_is_comp_in_list(
	COMPONENT	comp,
	COMP_LIST	*comp_list)
{
	int		i;

	for (i = 0; i < comp_list->ncomps; ++i)
	    if (comp == comp_list->comps[i])
		return YES;
	return NO;
}		/*end i_is_comp_in_list*/

EXPORT	int curve_in_curve_list(
	CURVE		*c,
	CURVE		**c_list)
{
	CURVE		**cc;

	for (cc = c_list; cc && *cc ; ++cc)
	    if (*cc == c) 
	    	return YES;
	return NO;
}		/*end curve_in_curve_list*/

EXPORT  boolean is_c_on_intfc(
        CURVE   *c)
{         
	INTERFACE       *intfc;
        CURVE           **cc;

        if (c == NULL) return NO;
        if ((intfc = c->interface) == NULL) return NO;
        for (cc = intfc->curves;  cc && *cc;  ++cc)
        {
            if (c == *cc)
                return YES;
        }
        return NO;
}               /*end is_c_in_intfc*/

EXPORT	boolean is_b_on_curve(
	CURVE *c,
	BOND *b)
{
	BOND *bb;
	if (c == NULL || b == NULL) return NO;
	for (bb = c->first; bb != NULL; bb = bb->next)
	    if (bb == b) return YES;
	return NO;
}	/* end is_b_on_curve */

EXPORT void reset_intfc_num_points(
	INTERFACE	*intfc)
{
	HYPER_SURF	*hs;
	HYPER_SURF_ELEMENT *hse;
	FT_POINT		*p;
	int		np = 0;

	(void) next_point(intfc,NULL,NULL,NULL);
	while (next_point(intfc,&p,&hse,&hs))
	    ++np;
	intfc->num_points = np;
}	/* end reset_intfc_num_points */

LIB_LOCAL	int	i_print_number_of_tangles(
	const char	*mesg,
	INTERFACE	*intfc,
	CROSS		*cross)
{
	int		num, dim = intfc->dim;
	CROSS		*cr;
	static const char *iname[] = {"","wave interaction","point","curve"};

	for (num = 0, cr = cross; cr != NULL; ++num, cr = cr->next);
	if (debugging("untan2d"))
	{
	    (void) printf("The %s%sinterface is tangled at ",
		      (pp_numnodes()>1)?"local ":"",mesg);
	    (void) printf("%d %s%s\n",num,iname[dim],(num!=1) ? "s" : "");
	}
	if (num > 0)
	{
	    if (debugging("intersect"))
	    	print_intersections(cross,intfc);
	    if (debugging("bond_cross"))
	    	print_crossing_elements(cross,intfc);
	    if (debugging("rcl"))
	    	print_cross_list(cross);
	}
	return num;
}		/*end i_print_number_of_tangles*/


struct	_TRI_LIST_AT_VERTEX
{
	FT_POINT	*p;
	TRI	**tris;
	int	num_tris;
};
typedef struct  _TRI_LIST_AT_VERTEX TRI_LIST_AT_VERTEX;
LOCAL	TRI_LIST_AT_VERTEX Tri_list_at_vertex;


/*
*			plane_fit_normal3d():
*
*	Returns the Normal to the hypersurface hs at the point p on
*	hypersurface element hse of hs.  The normal points from the
*	negative side to the positive side of the hypersurface.
*
*	The normal is computed by a least squares plane fit to the point
*	p and the points adjacent to p.
*
*	NOTE: In some instances, such as when the point p is the vertex
*	of a cone-like structure,  it is desirable to omit the use of p
*	in the planar fit. The function omit_vertex_in_plane_fit() turns
*	on a flag that excludes the use of p in the fitting structure. This
*	flag is then reset to the standard value of including p in the plane
*	fit.
*/

LOCAL	boolean	OmitVertexInPlaneFit = NO;

EXPORT	void	omit_vertex_in_plane_fit(void)
{
	OmitVertexInPlaneFit = YES;
}		/*end omit_vertex_in_plane_fit*/

EXPORT  void plane_fit_normal3d(
	FT_POINT		   *p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF	   *hs,
	double		   *nor)
{
	INTERFACE    *intfc = hs->interface;
	FT_POINT        *pp;
	TRI          *tri;
	int          n, i, v;
	double        pbar[3], lambda[3];
	static double **pts = NULL;
	static double **r = NULL;
	static int   num_pts = 0;

	if ((intfc->modified == NO) && (normal_at_point(p)[0] != HUGE_VAL))
	{
	    Tri_list_at_vertex.p = NULL;
	    Tri_list_at_vertex.num_tris = 0;
	    Tri_list_at_vertex.tris = NULL;
	    nor[0] = normal_at_point(p)[0];
	    nor[1] = normal_at_point(p)[1];
	    nor[2] = normal_at_point(p)[2];
	    return;
	}
	if (r == NULL)
	    bi_array(&r,3,3,FLOAT);
	tri = Tri_of_hse(hse);
	Tri_list_at_vertex.p = p;
	Tri_list_at_vertex.num_tris =
	    set_tri_list_around_point(p,Tri_of_hse(hse),
	                              &Tri_list_at_vertex.tris,intfc);
	if ((Tri_list_at_vertex.num_tris+2) > num_pts)
	{
	    if (pts != NULL)
	        free(pts);
	    num_pts = 2*(Tri_list_at_vertex.num_tris+2);
	    uni_array(&pts,num_pts,sizeof(double*));
	}
	n = 0;
	if (OmitVertexInPlaneFit)
	    OmitVertexInPlaneFit = NO;
	else
	    pts[n++] = Coords(p);
	v = Vertex_of_point(Tri_list_at_vertex.tris[0],p);
	if (is_side_bdry(Tri_list_at_vertex.tris[0],Prev_m3(v)))
	{
	    pp = Point_of_tri(Tri_list_at_vertex.tris[0])[Prev_m3(v)];
	    pts[n++] = Coords(pp);
	}
	for (i = 0; i < Tri_list_at_vertex.num_tris; ++i)
	{
	    v = Vertex_of_point(Tri_list_at_vertex.tris[i],p);
	    pp = Point_of_tri(Tri_list_at_vertex.tris[i])[Next_m3(v)];
	    pts[n++] = Coords(pp);
	}
	affine_fit((const double* const*)pts,3,n,Tri_normal(tri),pbar,r,lambda);

	normal_at_point(p)[0] = nor[0] = r[2][0];
	normal_at_point(p)[1] = nor[1] = r[2][1];
	normal_at_point(p)[2] = nor[2] = r[2][2];
}		/*end plane_fit_normal3d*/



#define   MAX_PTS    50

/*tris1 and tris2 must come from set_tri_list_around_point */
EXPORT  void plane_fit_normal3d_along_wall(
	double		   *nor,
	FT_POINT		   *p,
	TRI		   **tris1,
	int		   nt1,
	TRI		   **tris2,
	int		   nt2)

{
	FT_POINT        *pp;
	int          n,i,j, v;
	double        pbar[3], lambda[3], dv[3], lenv;
	double        *pts[MAX_PTS]; 
	static double **r = NULL;

	if (r == NULL)
	    bi_array(&r,3,3,FLOAT);

	n = 0;
	/*add all points in tris1 including the two bdry points */
	v = Vertex_of_point(tris1[0],p);
	if (is_side_bdry(tris1[0],Prev_m3(v)))
	{
	    pp = Point_of_tri(tris1[0])[Prev_m3(v)];
	    pts[n++] = Coords(pp);
	}
	for (i = 0; i < nt1; ++i)
	{
	    v = Vertex_of_point(tris1[i],p);
	    pp = Point_of_tri(tris1[i])[Next_m3(v)];
	    pts[n++] = Coords(pp);
	}
	
	/*add all points in tris2 excluding the two bdry points */
	for (i = 0; i < nt2-1; ++i)
	{
	    v = Vertex_of_point(tris2[i],p);
	    pp = Point_of_tri(tris2[i])[Next_m3(v)];
	    pts[n++] = Coords(pp);
	}

	affine_fit((const double* const*)pts,3,n,Tri_normal(tris1[0]),pbar,r,lambda);

	nor[0] = r[2][0];
	nor[1] = r[2][1];
	nor[2] = r[2][2];
}		/*end plane_fit_normal3d*/




/*
*			area_weighted_normal3d():
*
*	Returns the Normal to the hypersurface hs at the point p on
*	hypersurface element hse of hs.  The normal points from the
*	negative side to the positive side of the hypersurface.
*
*	The normal is computed as the area weighted average of the normals
*	to the triangles that surround the point p. This normal vector has
*	the property that the sum of the signed areas of the triangled onto
*	a plane with this normal is a maxium.
*/

/*ARGSUSED*/
EXPORT  void area_weighted_normal3d(
	FT_POINT		   *p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF	   *hs,
	double		   *nor)
{
	INTERFACE    *intfc = hs->interface;
	if ((intfc->modified == NO) && (normal_at_point(p)[0] != HUGE_VAL))
	{
	    Tri_list_at_vertex.p = NULL;
	    Tri_list_at_vertex.num_tris = 0;
	    Tri_list_at_vertex.tris = NULL;
	    nor[0] = normal_at_point(p)[0];
	    nor[1] = normal_at_point(p)[1];
	    nor[2] = normal_at_point(p)[2];
	    return;
	}
	Tri_list_at_vertex.p = p;
	Tri_list_at_vertex.num_tris =
	    set_tri_list_around_point(p,Tri_of_hse(hse),
	                              &Tri_list_at_vertex.tris,intfc);
	set_area_weighted_normal(Tri_list_at_vertex.tris,
	                         Tri_list_at_vertex.num_tris,nor);
	normal_at_point(p)[0] = nor[0];
	normal_at_point(p)[1] = nor[1];
	normal_at_point(p)[2] = nor[2];
}		/*end area_weighted_normal3d*/


EXPORT	int	tri_list_computed_by_normal(
	FT_POINT     *p,
	TRI       *tri,
	TRI       ***ptris,
	INTERFACE *intfc)
{
	if (Tri_list_at_vertex.p == NULL)
	{
	    Tri_list_at_vertex.p = p;
	    Tri_list_at_vertex.num_tris =
	        set_tri_list_around_point(p,tri,&Tri_list_at_vertex.tris,intfc);
	}

	*ptris = Tri_list_at_vertex.tris;
	return Tri_list_at_vertex.num_tris;
}		/*end tri_list_computed_by_normal*/

EXPORT	void	PointArrayRing1(
	FT_POINT *p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF *hs,
	int *npts1,
	FT_POINT **pts1)
{
	INTERFACE *intfc = hs->interface;
	TRI *t,*tri = Tri_of_hse(hse);
	int i,j,nt;
	FT_POINT *pp;
	boolean pp_in_list;

	nt = set_tri_list_around_point(p,tri,&Tri_list_at_vertex.tris,intfc);
	*npts1 = 0;
	for (i = 0; i < nt; ++i)
	{
	    t = Tri_list_at_vertex.tris[i];
	    for (j = 0; j < 3; ++j)
	    {
	    	pp = Point_of_tri(t)[j];
		if (pp == p) continue;
		pp_in_list = pointer_in_list((POINTER)pp,*npts1,(POINTER*)pts1);
		if (!pp_in_list) 
		{
		    pp->hse = Hyper_surf_element(t);
		    pts1[(*npts1)++] = pp;
		}
	    }
	}
}	/* end PointArrayRing1 */

	
EXPORT	void	PointArrayRing2(
	FT_POINT *p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF *hs,
	int *npts1,
	int *npts2,
	FT_POINT **pts1,
	FT_POINT **pts2)
{
	INTERFACE *intfc = hs->interface;
	int i,j,k,l,nt;
	FT_POINT *pp;
	TRI *t;
	boolean pp_in_lists;

	PointArrayRing1(p,hse,hs,npts1,pts1);
	*npts2 = 0;
	for (i = 0; i < *npts1; ++i)
	{
	    nt = set_tri_list_around_point(pts1[i],Tri_of_hse(pts1[i]->hse),
	    		&Tri_list_at_vertex.tris,intfc);
	    for (j = 0; j < nt; ++j)
	    {
	    	t = Tri_list_at_vertex.tris[j];
		for (k = 0; k < 3; ++k)
		{
		    pp = Point_of_tri(t)[k];
		    pp_in_lists = NO;
		    if (pp == p) continue;
		    pp_in_lists = pointer_in_list((POINTER)pp,*npts1,
		    			(POINTER*)pts1);
		    if (pp_in_lists) continue;
		    pp_in_lists = pointer_in_list((POINTER)pp,*npts2,
		    			(POINTER*)pts2);
		    if (!pp_in_lists)
		    {
		    	pp->hse = Hyper_surf_element(t);
			pts2[(*npts2)++] = pp;
		    }
		}
	    }
	}
}	/* end PointArrayRing2 */

LOCAL	void	set_area_weighted_normal(
	TRI		**tris,
	int		num_tris,
	double		*nor)
{
	int	    i,j;
	double	    area_tri,length;
	const double *tnor;

	nor[0] = nor[1] = nor[2] = 0.0;
	for (i = 0; i < num_tris; ++i)
	{
	    tnor = Tri_normal(tris[i]);
	    area_tri = 0.5*Mag3d(tnor);
	    for (j = 0; j < 3; ++j)
	    {
	    	nor[j] += tnor[j]*area_tri;
	    }
	}
	length = Mag3d(nor);
	for (i = 0; i < 3; ++i)
	    nor[i] /= length;
}		/*end set_area_weighted_normal*/

/*
*			sine_weighted_normal3d():
*	
*	This function uses the method introduced by Nelson Max
*       "Weights for Computing Vertex Normals from Facet Normals",
*       Journal of Graphics Tools, 4 (2) (1999), pp. 1-6.
*
*/

void	tecplot_tris(const char*, TRI **, int);

/*ARGSUSED*/
EXPORT  void sine_weighted_normal3d(
	FT_POINT		   *p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF	   *hs,
	double		   *nor)
{
	INTERFACE    *intfc = hs->interface;
	
	if(debugging("db_nor"))
	{
	    printf("#db_nor test\n");
	    print_general_vector("pt=", Coords(p), 3, "\n");
	    printf("modified = %d  nor[0] = %24.16e\n", intfc->modified, normal_at_point(p)[0]);
	    print_general_vector("nor=", normal_at_point(p), 3, "\n");
	}

	if(debugging("db_nor"))
	{
	    printf("db_nor reach\n");
	}

	Tri_list_at_vertex.p = p;
	Tri_list_at_vertex.num_tris =
	    set_tri_list_around_point(p,Tri_of_hse(hse),
	                              &Tri_list_at_vertex.tris,intfc);
	set_nel_max_normal(p,Tri_list_at_vertex.tris,
				      Tri_list_at_vertex.num_tris,nor);
	normal_at_point(p)[0] = nor[0];
	normal_at_point(p)[1] = nor[1];
	normal_at_point(p)[2] = nor[2];
	
	if(debugging("db_nor"))
	{
	    printf("calc af\n");
	    /*tecplot_tris("db_nor", Tri_list_at_vertex.tris, Tri_list_at_vertex.num_tris); */
	    print_general_vector("nor=", normal_at_point(p), 3, "\n");
	}
}		/*end sine_weighted_normal3d*/

EXPORT  void set_normal_from_tris(
	FT_POINT        *p,
	TRI          **tris,
	int          nt,
	double        *nor)
{
	set_nel_max_normal(p, tris,  nt, nor);
}


LOCAL   void set_nel_max_normal(
        FT_POINT        *p,
	TRI          **tris,
	int          nt,
	double        *nor)

{
        int     i,j,v;
	double   dtheta,length;
	double   wm0[3],wm1[3],tmp_wm0,tmp_wm1;
	const double   *tnor;
	TRI     *tri;
	FT_POINT   *pn0,*pn1;
	
	nor[0] = nor[1] = nor[2] =0;
        for(i = 0; i < nt; ++i)
	{
	    tri = tris[i];
	    tnor = Tri_normal(tri);
	    length = Mag3d(tnor);
	    
	    if(debugging("db_nor"))
	    {
		print_general_vector("tnor=", tnor, 3, "\n");
	    }

	    v = Vertex_of_point(tri,p);
	    pn0 = Point_of_tri(tri)[Next_m3(v)];
            pn1 = Point_of_tri(tri)[Prev_m3(v)];
            for (j = 0; j < 3; ++j)
	    {
	        wm0[j] = Coords(pn0)[j] - Coords(p)[j];
                wm1[j] = Coords(pn1)[j] - Coords(p)[j];
            }
            tmp_wm0 = Mag3d(wm0);
	    tmp_wm1 = Mag3d(wm1);
            for (j = 0; j < 3; ++j)
	    {
	         wm0[j] =  wm0[j]/tmp_wm0;
		 wm1[j] =  wm1[j]/tmp_wm1;
            }
	    dtheta = acos(scalar_product(wm1,wm0,3));
            for (j = 0; j < 3; ++j)
	    {
	       nor[j] += tnor[j]*sin(dtheta)/(tmp_wm0*tmp_wm1*length);
	    }
         }
	 length = Mag3d(nor);
         for (i = 0; i < 3; ++i)
	     nor[i] /= length;
}	/* end set_nel_max_normal */

struct _TRI_Plus_normal {
	TRI Tri;
	double _nor[3];	          /* normal to the tri plane	     */
	double _sqr_norm;	  /* sqr(n1) + sqr(n2) +sqr(n3)      */
};
typedef struct _TRI_Plus_normal TRI_Plus_normal;

struct _TRI_FullGeometry {
	TRI_Plus_normal TriPN;
	double _length_side[3];    /* length of sides                 */
	double _sv_store[3][3];    /* storage for _side_vector        */
	double *_side_vector[3];	  /* side uni_arrays v[i] = p[i+1]-p[i] */
};
typedef struct _TRI_FullGeometry TRI_FullGeometry;

LOCAL	TRI_STORAGE_TYPE tri_storage_type = TRI_PLUS_NORMAL;

#define Tri_normal_vector(_tri_)  (((TRI_Plus_normal*)(_tri_))->_nor)
#define sqr_normal_vector(_tri_)  (((TRI_Plus_normal*)(_tri_))->_sqr_norm)
#define fg_sv_store(_tri_)	  (((TRI_FullGeometry*)(_tri_))->_sv_store)
#define fg_side_vector(_tri_)     (((TRI_FullGeometry*)(_tri_))->_side_vector)
#define fg_length_side(_tri_)	  (((TRI_FullGeometry*)(_tri_))->_length_side)

LIB_LOCAL	void	set_tri_storage_type(
	TRI_STORAGE_TYPE type)
{
	I_USER_INTERFACE *iuh = i_user_hook(3);
	switch (type)
	{
	case MIN_TRI_STORAGE:
	    tri_storage_type = type;
	    iuh->size_tri = sizeof(TRI);
	    break;
	case FULL_TRI_GEOMETRY:
	    tri_storage_type = type;
	    iuh->size_tri = sizeof(TRI_FullGeometry);
	    break;
	case TRI_PLUS_NORMAL:
	    tri_storage_type = type;
	    iuh->size_tri = sizeof(TRI_Plus_normal);
	    break;
	default:
	    screen("ERROR in set_tri_storage_type(), unknown value %d\n",type);
	    clean_up(ERROR);
	}
}		/*end set_tri_storage_type*/

EXPORT void reset_normal_on_intfc(
	INTERFACE	*intfc)
{
	SURFACE		**s;
	TRI		*t;


	switch (tri_storage_type)
	{
	case MIN_TRI_STORAGE:
	    break;
	case FULL_TRI_GEOMETRY:
	case TRI_PLUS_NORMAL:
	    for (s = intfc->surfaces; s && *s; ++s)
	        for (t = first_tri(*s); !at_end_of_tri_list(t,*s); t = t->next)
		    set_normal_of_tri(t);
	    break;
	default:
	    screen("ERROR in reset_normal_on_intfc(), unknown value %d\n",
		   tri_storage_type);
	    clean_up(ERROR);
	}
}		/*end reset_normal_on_intfc*/

/*ARGSUSED*/
EXPORT	void set_normal_of_tri(
	TRI	*tri)
{
	double *p[3];
	double *n;
	double **s, s0, s1, s2;
	int   i;

	if (tri_storage_type == MIN_TRI_STORAGE)
	    return;

	p[0] = Coords(Point_of_tri(tri)[0]);
	p[1] = Coords(Point_of_tri(tri)[1]);
	p[2] = Coords(Point_of_tri(tri)[2]);

	if (tri_storage_type == FULL_TRI_GEOMETRY)
	{
	    s = fg_side_vector(tri);
	    for (i = 0; i < 3; ++i)
	    {
	        s[i] = fg_sv_store(tri)[i];
	        s[i][0] = s0 = p[Next_m3(i)][0] - p[i][0];
	        s[i][1] = s1 = p[Next_m3(i)][1] - p[i][1];
	        s[i][2] = s2 = p[Next_m3(i)][2] - p[i][2];
	        fg_length_side(tri)[i] = sqrt(s0*s0 + s1*s1 + s2*s2);
	    }
	}
	else
	{
	    static double **sstore;
	    if (sstore == NULL)
		bi_array(&sstore,3,3,FLOAT);
	    s = sstore;
	    for (i = 0; i < 3; ++i)
	    {
	        s[i][0] = p[Next_m3(i)][0] - p[i][0];
	        s[i][1] = p[Next_m3(i)][1] - p[i][1];
	        s[i][2] = p[Next_m3(i)][2] - p[i][2];
	    }
	}

	n = Tri_normal_vector(tri);
	n[0] = s[2][1]*s[0][2] - s[2][2]*s[0][1];
	n[1] = s[2][2]*s[0][0] - s[2][0]*s[0][2];
	n[2] = s[2][0]*s[0][1] - s[2][1]*s[0][0];
	sqr_normal_vector(tri) = n[0]*n[0] + n[1]*n[1] + n[2]*n[2];
}		/*end set_normal_of_tri*/

EXPORT	const double	*Tri_normal(
	const TRI *tri)
{
	if (tri_storage_type == MIN_TRI_STORAGE)
	{
	    const double  *p[3];
	    double        s[3][3];
	    static double n[3];
	    int   i;

	    p[0] = Coords(Point_of_tri(tri)[0]);
	    p[1] = Coords(Point_of_tri(tri)[1]);
	    p[2] = Coords(Point_of_tri(tri)[2]);

	    for (i = 0; i < 3; ++i)
	    {
	        s[i][0] = p[Next_m3(i)][0] - p[i][0];
	        s[i][1] = p[Next_m3(i)][1] - p[i][1];
	        s[i][2] = p[Next_m3(i)][2] - p[i][2];
	    }

	    n[0] = s[2][1]*s[0][2] - s[2][2]*s[0][1];
	    n[1] = s[2][2]*s[0][0] - s[2][0]*s[0][2];
	    n[2] = s[2][0]*s[0][1] - s[2][1]*s[0][0];
	    return n;
	}
	else
	    return Tri_normal_vector(tri);
}		/*end Tri_normal*/

EXPORT	double	sqr_norm(
	const TRI *tri)
{
	if (tri_storage_type == MIN_TRI_STORAGE)
	{
	    const double *n = Tri_normal(tri);
	    return n[0]*n[0] + n[1]*n[1] + n[2]*n[2];
	}
	else
	    return sqr_normal_vector(tri);
}		/*end sqr_norm*/

EXPORT	const double* const* side_vector(
	const TRI *tri)
{
	if (tri_storage_type == FULL_TRI_GEOMETRY)
	    return (const double* const*)fg_side_vector(tri);
	else
	{
	    double *p[3];
	    int   i;
	    static double *s[3];
	    static double ss[3][3];

	    p[0] = Coords(Point_of_tri(tri)[0]);
	    p[1] = Coords(Point_of_tri(tri)[1]);
	    p[2] = Coords(Point_of_tri(tri)[2]);

	    for (i = 0; i < 3; ++i)
	    {
	        s[i] = ss[i];
	        ss[i][0] = p[Next_m3(i)][0] - p[i][0];
	        ss[i][1] = p[Next_m3(i)][1] - p[i][1];
	        ss[i][2] = p[Next_m3(i)][2] - p[i][2];
	    }
	    return (const double* const*)s;
	}
}		/*end side_vector*/

EXPORT	const double *vector_on_tri_side(
	const TRI *tri,
	int       side,
	double     *v)
{
	if (v == NULL)
	{
	    if (tri_storage_type == FULL_TRI_GEOMETRY)
	        return fg_side_vector(tri)[side];
	    else
	    {
	        double *np = Coords(Point_of_tri(tri)[Next_m3(side)]);
	        double *p = Coords(Point_of_tri(tri)[side]);
	        static double sv[3];

	        sv[0] = np[0] - p[0];
	        sv[1] = np[1] - p[1];
	        sv[2] = np[2] - p[2];
	        return sv;
	    }
	}
	else
	{
	    if (tri_storage_type == FULL_TRI_GEOMETRY)
	    {
	        v[0] = fg_side_vector(tri)[side][0];
	        v[1] = fg_side_vector(tri)[side][1];
	        v[2] = fg_side_vector(tri)[side][2];
	    }
	    else
	    {
	        double *np = Coords(Point_of_tri(tri)[Next_m3(side)]);
	        double *p = Coords(Point_of_tri(tri)[side]);

	        v[0] = np[0] - p[0];
	        v[1] = np[1] - p[1];
	        v[2] = np[2] - p[2];
	    }
	    return v;
	}
}		/*end vector_on_tri_side*/

EXPORT	const double	*length_side(
	const TRI *tri)
{
	if (tri_storage_type == FULL_TRI_GEOMETRY)
	    return fg_length_side(tri);
	else
	{
	    static double l[3];
	    double        *p, *np;
	    int          side;

	    for (side = 0; side < 3; ++side)
	    {
	        p = Coords(Point_of_tri(tri)[side]);
	        np = Coords(Point_of_tri(tri)[Next_m3(side)]);
	        l[side] = sqrt(sqr(np[0]-p[0])+sqr(np[1]-p[1])+sqr(np[2]-p[2]));
	    }
	    return l;
	}
}		/*end length_side*/

EXPORT	double	length_of_tri_side(
	const TRI *tri,
	int       side)
{
	if (tri_storage_type == FULL_TRI_GEOMETRY)
	    return fg_length_side(tri)[side];
	else
	{
	    double *p = Coords(Point_of_tri(tri)[side]);
	    double *np = Coords(Point_of_tri(tri)[Next_m3(side)]);
	    return sqrt(sqr(np[0]-p[0])+sqr(np[1]-p[1])+sqr(np[2]-p[2]));
	}
}		/*end length_of_tri_side*/

EXPORT 	int two_points_share_side(
	FT_POINT	  *p1,
	TRI	  *tri1,
	FT_POINT     *p2,
	INTERFACE *intfc)
{
	TRI       **tris;
	int	  i,j,num_tris;

        num_tris = set_tri_list_around_point(p1,tri1,&tris,intfc);
	if (num_tris == -1)
	    return -1;
	for (i = 0; i < num_tris; ++i)
	{
	    for (j = 0; j < 3; ++j)
	    {
	    	if (Point_of_tri(tris[i])[j] == p2 &&
				Tri_on_side(tris[i],j) != NULL)
		    return 1;
	    }
	}
	return 0;

}	/*end two_points_share_side*/		

#define NO_ROBUST_CROSS_BONDS

#if defined(NO_ROBUST_CROSS_BONDS)

/*
*	               robust_cross_bonds():
*
*	Actually the non robust version of the robust_cross_bonds()
*	the robust version can be obtained by resetting the compiler
*	directive NO_ROBUST_CROSS_BONDS.  The only robustness in
*	this case is with respect to machine tolerance times a
*	a tolerance factor TOL_FAC(current_interface()).
*/

EXPORT	boolean	robust_cross_bonds(
	BOND		*b1,
	int		on_b1,
	BOND		*b2,
	int		on_b2,
	double		*tcr1,
	double		*tcr2,
	RECT_GRID	*gr,
	FT_POINT		*p)
{
	FT_POINT		*p1s = b1->start,	*p1e = b1->end;
	FT_POINT		*p2s = b2->start,	*p2e = b2->end;
	double		b1sx = Coords(p1s)[0],	b1sy = Coords(p1s)[1];
	double		b1ex = Coords(p1e)[0],	b1ey = Coords(p1e)[1];
	double		b2sx = Coords(p2s)[0],	b2sy = Coords(p2s)[1];
	double		b2ex = Coords(p2e)[0],	b2ey = Coords(p2e)[1];
	double		xx1, yy1, xx2, yy2;
	double		*h = gr->h;
	double		parallel = PARALLEL(current_interface());
	double		x0 = (double)b1sx - (double)b2sx;
	double		y0 = (double)b1sy - (double)b2sy;
	double		x1 = (double)b1ex - (double)b1sx;
	double		y1 = (double)b1ey - (double)b1sy;
	double		x2 = (double)b2ex - (double)b2sx;
	double		y2 = (double)b2ey - (double)b2sy;
	double		num1, num2, den, len;
	double		ux, uy, a0, a1, a2;
	double		lb1, lb2, lb12;
	double 		t1, t2;
	double		t1min, t1max, t2min, t2max;
	double		rcb_min_sc_sep = RCB_MIN_SC_SEP(current_interface());
	int		long_bond;
	int		dim = gr->dim;
	static boolean	first = YES;
	static double	mac_tol;

#if defined(DEBUG_CROSS_BONDS)
	debug_print("rcb","Entered robust_cross_bonds()\n");
	if (debugging("rcb"))
	{
	    (void) printf("Bonds into robust_cross_bonds()\n");
	    (void) printf("b1 - ");	print_bond(b1);
	    (void) printf("b2 - ");	print_bond(b2);
	}
#endif /* defined(DEBUG_CROSS_BONDS) */

	if (first == YES)
	{
	    first = NO;

	    /*mac_tol = TOL_FAC(current_interface())*MACH_EPS;*/
	    mac_tol = RCB_MACH_TOL(current_interface());
	}

	/* Set robustness factor to zero except at nodes */

	t1min = (b1->prev == NULL) ?      -mac_tol : 0.0;
	t1max = (b1->next == NULL) ? 1.0 + mac_tol : 1.0;
	t2min = (b2->prev == NULL) ?      -mac_tol : 0.0;
	t2max = (b2->next == NULL) ? 1.0 + mac_tol : 1.0;

	den  = x2*y1  -  y2*x1;
	num1 = x0*y2  -  y0*x2;
	num2 = x0*y1  -  y0*x1;
	lb1  = (double) bond_length(b1);	lb2 = (double) bond_length(b2);

#if defined(DEBUG_CROSS_BONDS)
	if (debugging("rcb"))
	{
	    (void) printf("den %g num1 %g num2 %g lb1 %g lb2 %g\n",
	    		  den,num1,num2,lb1,lb2);
	}
#endif /* defined(DEBUG_CROSS_BONDS) */

	if (fabs(den) <= parallel*lb1*lb2) /* Parallel lines */
	{
	    lb12 = separation(p1s,p2s,gr->dim);

#if defined(DEBUG_CROSS_BONDS)
	    if (debugging("rcb"))
	    {
	        (void) printf("Bonds are parallel\n");
	        (void) printf("lb12 %g eps12 %g eps122 %g eps121 %g\n",
	        	      lb12,parallel*lb1*lb2,parallel*lb12*lb2,
	        	      parallel*lb12*lb1);
	    }
#endif /* defined(DEBUG_CROSS_BONDS) */

	    if ((fabs(num1) > parallel*lb12*lb2) 
	         || (fabs(num2) > parallel*lb12*lb1)   )
	    {
#if defined(DEBUG_CROSS_BONDS)
	        debug_print("rcb","Leaving robust_cross_bonds() NO CROSS\n");
#endif /* defined(DEBUG_CROSS_BONDS) */
	        return NO;
	    }

	    if (max(lb1,lb2) < mac_tol)
	    {

	    /* both b1 and b2 have zero length, cross if p1s and p2s close */

	        if (scaled_separation(p1s,p2s,h,dim) > rcb_min_sc_sep)
	        {
#if defined(DEBUG_CROSS_BONDS)
	            debug_print("rcb","Leaving robust_cross_bonds() NO CROSS\n");
#endif /* defined(DEBUG_CROSS_BONDS) */
	            return NO;
	        }
	        *tcr1 = *tcr2 = 0.0;
	        set_cross_position(b1sx,b1sy,on_b1,b2sx,b2sy,on_b2,p);
#if defined(DEBUG_CROSS_BONDS)
	        debug_print("rcb","Leaving robust_cross_bonds() BONDS CROSS\n");
#endif /* defined(DEBUG_CROSS_BONDS) */
	        return YES;
	    }

	    if (lb1 > lb2) 
	    {
	        long_bond = 1;
	        ux = x1;		uy = y1;
	        len = (double)bond_length(b1);
	        ux /= len;		uy /= len;
	    }
	    else
	    {
	        long_bond = 2;
	        ux = x2;		uy = y2;
	        len = (double)bond_length(b2);
	        ux /= len;		uy /= len;
	    }

	    a0 = x0*ux  +  y0*uy;
	    a1 = x1*ux  +  y1*uy;
	    a2 = x2*ux  +  y2*uy;

#if defined(DEBUG_CROSS_BONDS)
	    if (debugging("rcb"))
	    {
	        double A0, A1, A2;
	        (void) printf("long_bond %d ux %g uy %g a0 %g a1 %g a2 %g\n",
	        	      long_bond,ux,uy);

	        if (long_bond == 1)
	        {
	            A0 = a0/a1;	A2 = a2/a2;
	            (void) printf("a0 %g a2 %g\n",a0/a1,a2/a1);
	            (void) printf("tests: t1min %g t1max       %g\n",
	        		  t1min,t1max);
	            (void) printf("       -a0   %g t2min*a2-a0 %g\n",
    				  -A0,t2min*A2-A0);
	            (void) printf("       a2-a0 %g t2max*a2-a0 %g\n",
	        		  A2-A0,t2max*A2-A0);
	        }
	        else
	        {
	            A0 = a0/a2;	A1 = a1/a2;
	            (void) printf("a0 %g a1 %g\n",A0,A1);
	            (void) printf("tests: t2min %g t2max       %g\n",
	            	          t2min,t2max);
	            (void) printf("          a0 %g a0+t1min*a1 %g\n",
	            		  A0,A0+t1min*A1);
	            (void) printf("       a0+a1 %g a0+t1max*a1 %g\n",
	        		  A0+A1,A0+t1max*A1);
	        }
	    }
#endif /* defined(DEBUG_CROSS_BONDS) */

	    if (long_bond == 1) 
	    {
	        a0 /= a1;               a2 /= a1;
	        if ((t1min<-a0 && -a0<t1max) ||
	            (t2min<0.0 && (t1min<t2min*a2-a0 && t2min*a2-a0<t1max)))
	        {
	            t2 = 0.0;	t1 = -a0;
	        }
	        else if ((t1min<a2-a0 && a2-a0<t1max) ||
	        	 (t2max>1.0 &&
	        	     (t1min<t2max*a2-a0 && t2max*a2-a0 < t1max)))
	        {
	            t2 = 1.0;	t1 = a2 - a0;
	        }
	        else 
	        {
#if defined(DEBUG_CROSS_BONDS)
	            debug_print("rcb","Leaving robust_cross_bonds() NO CROSS\n");
#endif /* defined(DEBUG_CROSS_BONDS) */
	            return NO;
	        }
	    }
	    else 
	    {
	        a0 /= a2;               a1 /= a2;
	        if ((t2min < a0 && a0 < t2max) ||
	            (t1min < 0.0 && (t2min < a0+t1min*a1
	        		      && a0+t1min*a1 < t2max)))
	        {
	            t1 = 0.0;	t2 = a0;
	        }
	        else if ((t2min < a0+a1 && a0+a1 < t2max) ||
	        	 (t1max > 1.0 &&
	        	     (t2min < a0+t1max*a1 && a0+t1max*a1 < t2max)))
	        {
	            t1 = 1.0;	t2 = a0 + a1;
	        }
	        else 
	        {
#if defined(DEBUG_CROSS_BONDS)
	            debug_print("rcb","Leaving robust_cross_bonds() NO CROSS\n");
#endif /* defined(DEBUG_CROSS_BONDS) */
	            return NO;
	        }
	    }
	}

	else
	{
	    /* Should not have to worry that fabs(den) is small */
	    /* as this taken care of in previous if() test. */

	    t1 = num1/den;			t2 = num2/den;

#if defined(DEBUG_CROSS_BONDS)
	    if( debugging("rcb") )
	    {
	    	(void) printf("Bonds not parallel, t1 %g, t2 %g\n",t1,t2);
	    	(void) printf("t1min %g, t1max %g, t2min %g, t2max %g\n",
	    		      t1min,t1max,t2min,t2max);
	    }
#endif /* defined(DEBUG_CROSS_BONDS) */

	    if ((t1 < t1min || t1 > t1max) || (t2 < t2min || t2 > t2max))
	    {
#if defined(DEBUG_CROSS_BONDS)
	        debug_print("rcb","Leaving robust_cross_bonds() NO CROSS\n");
#endif /* defined(DEBUG_CROSS_BONDS) */
	    	return NO;
	    }
	}

	if (t1 < 0.0)
	    t1 = 0.0;
	if (t1 > 1.0)
	    t1 = 1.0;
	if (t2 < 0.0)
	    t2 = 0.0;
	if (t2 > 1.0)
	    t2 = 1.0;

#if defined(DEBUG_CROSS_BONDS)
	if (debugging("rcb"))
	    (void) printf("tcr: 1 %g  2 %g\n",t1,t2);
#endif /* defined(DEBUG_CROSS_BONDS) */

	*tcr1 = (double)t1;		*tcr2 = (double)t2;
	xx1 = b1sx + t1*x1;		yy1 = b1sy + t1*y1;
	xx2 = b2sx + t2*x2;		yy2 = b2sy + t2*y2;
	set_cross_position(xx1,yy1,on_b1,xx2,yy2,on_b2,p);

#if defined(DEBUG_CROSS_BONDS)
	debug_print("rcb","Leaving robust_cross_bonds() BONDS CROSS\n");
#endif /* defined(DEBUG_CROSS_BONDS) */
	return YES;
}		/*end robust_cross_bonds*/

#else /* defined(NO_ROBUST_CROSS_BONDS) */

#define DEBUG_CROSS_BONDS

/*
*			robust_cross_bonds():
*
*	Robust version of bond crossing function.  Extends each
*	bond by the scaled length factor ROBUST_FAC through nodes
*	and EPSILON*ROBUST_FAC elsewhere.
*/

EXPORT	boolean	robust_cross_bonds(
	BOND		*b1,
	int		on_b1,
	BOND		*b2,
	int		on_b2,
	double		*tcr1,
	double		*tcr2,
	RECT_GRID	*gr,
	FT_POINT		*p)
{
	FT_POINT		*p1s = b1->start,	*p1e = b1->end;
	FT_POINT		*p2s = b2->start,	*p2e = b2->end;
	double		b1sx = Coords(p1s)[0],	b1sy = Coords(p1s)[1];
	double		b1ex = Coords(p1e)[0],	b1ey = Coords(p1e)[1];
	double		b2sx = Coords(p2s)[0],	b2sy = Coords(p2s)[1];
	double		b2ex = Coords(p2e)[0],	b2ey = Coords(p2e)[1];
	double		xx1, yy1, xx2, yy2;
	double		*h = gr->h;
	double		x0 = (double)b1sx - (double)b2sx;
	double		y0 = (double)b1sy - (double)b2sy;
	double		x1 = (double)b1ex - (double)b1sx;
	double		y1 = (double)b1ey - (double)b1sy;
	double		x2 = (double)b2ex - (double)b2sx;
	double		y2 = (double)b2ey - (double)b2sy;
	double		t1, t2;
	double		num1, num2, den, len;
	double		ux, uy, a0, a1, a2;
	double		lb1, lb2, lb12;
	double		scaled_length;
	double		delta1, delta2, tcr1_min, tcr1_max, tcr2_min, tcr2_max;
	double		t1mid, dt1, t2mid, dt2;
	double		rcb_min_sc_sep = RCB_MIN_SC_SEP(current_interface());
	double		robust_fac = ROBUST_FAC(current_interface());
	int		long_bond;
	int		dim = gr->dim;

#if defined(DEBUG_CROSS_BONDS)
	debug_print("rcb","Entered robust_cross_bonds()\n");
	if (debugging("rcb"))
	{
	    (void) printf("Bonds into robust_cross_bonds()\n");
	    (void) printf("b1 - ");	print_bond(b1);
	    (void) printf("b2 - ");	print_bond(b2);
	}
#endif /* defined(DEBUG_CROSS_BONDS) */

	den  = x2*y1  -  y2*x1;
	num1 = x0*y2  -  y0*x2;
	num2 = x0*y1  -  y0*x1;
	lb1 = (double) bond_length(b1);	lb2 = (double) bond_length(b2);


	        /* Use a dimensionless delta */

	scaled_length = (double)scaled_bond_length(b1,h,dim);
	delta1 = (scaled_length < EPSILON) ? 0.0 : robust_fac*scaled_length;

	scaled_length = (double)scaled_bond_length(b2,h,dim);
	delta2 = (scaled_length < EPSILON) ? 0.0 : robust_fac*scaled_length;

	    /* delta is now stricter at curve ends */
	    /*  If this routine fails for a curve  */
	    /*    the robust_extend_() routines    */
	    /* 	  below should be called       */

	tcr1_min = (b1->prev == NULL) ? 0.0 - delta1 : 0.0 - EPSILON*delta1;
	tcr2_min = (b2->prev == NULL) ? 0.0 - delta2 : 0.0 - EPSILON*delta2;

	tcr1_max = (b1->next == NULL) ? 1.0 + delta1 : 1.0 + EPSILON*delta1;
	tcr2_max = (b2->next == NULL) ? 1.0 + delta2 : 1.0 + EPSILON*delta2;

	t1mid = 0.5*(tcr1_min + tcr1_max);
	t2mid = 0.5*(tcr2_min + tcr2_max);

	dt1 = 0.5*fabs(tcr1_max - tcr1_min);
	dt2 = 0.5*fabs(tcr2_max - tcr2_min);

#if defined(DEBUG_CROSS_BONDS)
	if (debugging("rcb"))
	{
	    (void) printf("den %g num1 %g num2 %g lb1 %g lb2 %g\n",
	    		  den,num1,num2,lb1,lb2);
	    (void) printf("bond 1: del %g min %g max %g mid %g dt %g\n",
	    		  delta2,tcr1_min,tcr1_max,t1mid,dt1);
	    (void) printf("bond 2: del %g min %g max %g mid %g dt %g\n",
	    		  delta1,tcr2_min,tcr2_max,t2mid,dt2);
	    (void) printf("tests: fabs(num1-t1mid*den) %g  dt1*fabs(den) %g\n",
	    		  fabs(num1-t1mid*den),dt1*fabs(den));
	    (void) printf("       fabs(num2-t2mid*den) %g  dt2*fabs(den) %g\n",
	    		  fabs(num2-t2mid*den),dt2*fabs(den));
	}
#endif /* defined(DEBUG_CROSS_BONDS) */


	if(    (fabs(num1 - t1mid*den) >= dt1*fabs(den))
	    || (fabs(num2 - t2mid*den) >= dt2*fabs(den))    )
	{
	    lb12 = separation(b1->start,b2->start,dim);

#if defined(DEBUG_CROSS_BONDS)
	    if (debug_print("rcb"))
	    	(void) printf("lb12 %g eps12 %g eps122 %g eps121 %g\n",
	    	              lb12,parallel*lb1*lb2,parallel*lb12*lb2,
	        	      parallel*lb12*lb1);
#endif /* defined(DEBUG_CROSS_BONDS) */

	    if ((fabs(den)  > parallel*lb1*lb2)  ||
		(fabs(num1) > parallel*lb12*lb2) ||
		(fabs(num2) > parallel*lb12*lb1))
	    {
#if defined(DEBUG_CROSS_BONDS)
	    	debug_print("rcb","Left robust_cross_bonds() NO CROSS\n");
#endif /* defined(DEBUG_CROSS_BONDS) */
	    	return NO;
	    }
	    if (lb1 > lb2) 
	    {
	    	long_bond = 1;
	    	ux = x1;		uy = y1;
	    	len = (double)bond_length(b1);
	    	ux /= len;		uy /= len;
	    }
	    else if (lb2 > 0.0)
	    {
	    	long_bond = 2;
	    	ux = x2;		uy = y2;
	    	len = (double)bond_length(b2);
	    	ux /= len;		uy /= len;
	    }
	    else
	    {

	    /* both b1 and b2 have zero length, cross if p1s and p2s close */

	    	if (scaled_separation(b1->start,b2->start,h,dim)>rcb_min_sc_sep)
	    	{
#if defined(DEBUG_CROSS_BONDS)
	    	    debug_print("rcb","Left robust_cross_bonds() NO CROSS\n");
#endif /* defined(DEBUG_CROSS_BONDS) */
	                return NO;
	    	}
	    	*tcr1 = *tcr2 = 0.0;
	    	set_cross_position(b1sx,b1sy,on_b1,b2sx,b2sy,on_b2,p);
	    	return YES;
	    }
	    a0 = x0*ux  +  y0*uy;
	    a1 = x1*ux  +  y1*uy;
	    a2 = x2*ux  +  y2*uy;

#if defined(DEBUG_CROSS_BONDS)
	    if (debugging("rcb"))
	    {
	    	(void) printf("long_bond %d ux %g uy %g a0 %g a1 %g a2 %g\n",
	        	      long_bond,ux,uy,a0,a1,a2);

	    	(void) printf("tests: fabs(a0+t1mid*a1) %g  dt1*fabs(a1) %g\n",
	        	      fabs(a0+t1mid*a1),dt1*fabs(a1));
	    	(void) printf("       fabs(a0-a2+t1mid*a1) %g\n",
	    		      fabs(a0-a2+t1mid*a1));

	        (void) printf("       fabs(a0-t2mid*a2) %g  dt2*fabs(a2) %g\n",
	        	      fabs(a0-t2mid*a2),dt2*fabs(a2));
	        (void) printf("       fabs(a0+a1-t2mid*a2) %g\n",
	        	      fabs(a0+a1-t2mid*a2));

	    }
#endif /* defined(DEBUG_CROSS_BONDS) */
	    if (long_bond == 1) 
	    {
	    	if (fabs(a0 + t1mid*a1) <= dt1*fabs(a1))
	    	{
	    	    t2 = 0.0;	t1 = -a0/a1;
	    	}
	    	else if (fabs(a0 - a2 + t1mid*a1) <= dt1*fabs(a1))
	    	{
	    	    t2 = 1.0;	t1 = (a2 - a0)/a1;
	    	}
	    	else 
	    	{
#if defined(DEBUG_CROSS_BONDS)
	    	    debug_print("rcb","Left robust_cross_bonds() NO CROSS\n");
#endif /* defined(DEBUG_CROSS_BONDS) */
	    	    return NO;
	    	}
	    }
	    else 
	    {
	    	if (fabs(a0 - t2mid*a2) <= dt2*fabs(a2))
	    	{
	    	    t1 = 0.0;	t2 = a0/a2;
	    	}
	    	else if (fabs(a0 + a1 - t2mid*a2) <= dt2*fabs(a2))
	    	{
	    	    t1 = 1.0;	t2 = (a0 + a1)/a2;
	    	}
	    	else 
	    	{
#if defined(DEBUG_CROSS_BONDS)
	    	    debug_print("rcb","Left robust_cross_bonds() NO CROSS\n");
#endif /* defined(DEBUG_CROSS_BONDS) */
	    	    return NO;
	    	}
	    }
	}
	else 
	{
	    t1 = num1/den;		t2 = num2/den;
	}

#if defined(DEBUG_CROSS_BONDS)
	if (debugging("rcb"))
	    (void) printf("tcr: 1 %g  2 %g\n",t1,t2);
#endif /* defined(DEBUG_CROSS_BONDS) */

	if (t1 < 0.0)
	    t1 = 0.0;
	if (t1 > 1.0)
	    t1 = 1.0;
	if (t2 < 0.0)
	    t2 = 0.0;
	if (t2 > 1.0)
	    t2 = 1.0;

	*tcr1 = (double)t1;		*tcr2 = (double)t2;
	xx1 = b1sx + t1*x1;		yy1 = b1sy + t1*y1;
	xx2 = b2sx + t2*x2;		yy2 = b2sy + t2*y2;
	set_cross_position(xx1,yy1,on_b1,xx2,yy2,on_b2,p);

#if defined(DEBUG_CROSS_BONDS)
	debug_print("rcb","Left robust_cross_bonds() BONDS CROSS\n");
#endif /* defined(DEBUG_CROSS_BONDS) */
	return YES;
}		/*end robust_cross_bonds*/

#endif /* defined(NO_ROBUST_CROSS_BONDS) */


LOCAL	void set_cross_position(
	double		x1,
	double		y1,
	int		on_b1,
	double		x2,
	double		y2,
	int		on_b2,
	FT_POINT		*pt)
{
	if (on_b1 == YES && on_b2 == NO)
	{
	    Coords(pt)[0] = x1;		Coords(pt)[1] = y1;
	}
	else if (on_b1 == NO && on_b2 == YES)
	{
	    Coords(pt)[0] = x2;		Coords(pt)[1] = y2;
	}
	else
	{
	    Coords(pt)[0] = 0.5*(x1 + x2);	Coords(pt)[1] = 0.5*(y1 + y2);
	}
}		/*end set_cross_position*/


EXPORT	int robust_extend_bond_to_cross_bond(
	BOND		*b1,
	ORIENTATION	extend_orient,
	BOND		*b2,
	double		*tcr1,
	double		*tcr2,
	FT_POINT		*pc,
	double		*h,
	int		dim)
{
	double		p[MAXD],u[MAXD],v[MAXD],w[MAXD],vmw[MAXD];
	double		U_cross_V,U_cross_VMW, W_cross_V;
	double		U_dot_V, U_dot_W;
	double		scaled_length;
	double		ulen,para,alpha,delta;
	double		tcr2_min, tcr2_max;
	double		t2mid, dt2;
	double		dh[MAXD];
	int		i;

#if defined(DEBUG_CROSS_BONDS)
	debug_print("rebcb","Entered robust_extend_bond_to_cross_bond()\n");
	if (debugging("rebcb"))
	{
	    (void) printf("Checking for cross by extension of bond b1 %d ",b1);
	    (void) printf("with bond b2 %d\n",b2);
	    print_orientation("extend_orient = ",extend_orient,"\n");
	    (void) printf("b1 - ");		print_bond(b1);
	    (void) printf("b2 - ");		print_bond(b2);
	}
#endif /* defined(DEBUG_CROSS_BONDS) */

	if (b1 == NULL || b2 == NULL)
	{
#if defined(DEBUG_CROSS_BONDS)
	    if (debugging("rebcb"))
	    	(void) printf("NULL bond\n");
	    debug_print("rebcb",
	          "Left robust_extend_bond_to_cross_bond() NO CROSS\n");
#endif /* defined(DEBUG_CROSS_BONDS) */
	    return NO;	/* NULL bond */
	}
	for (i = 0; i < dim; ++i) dh[i] = (double) h[i];

	/* Find unit vector in direction of extension */

	if (extend_orient == POSITIVE_ORIENTATION) 
	{
	    for (i = 0; i < dim; ++i)
	    {
	    	p[i] = (double) Coords(b1->end)[i];
	    	u[i] = ((double) Coords(b1->start)[i]) - p[i];
	    }
	    *tcr1 = 0.0;
	}
	else 
	{
	    for (i = 0; i < dim; ++i)
	    {
	    	p[i] = (double) Coords(b1->start)[i];
	    	u[i] = ((double) Coords(b1->end)[i]) - p[i];
	    }
	    *tcr1 = 1.0;
	}
	if (dscaled_hypot(u,dh,dim) < 0.001)
	{
#if defined(DEBUG_CROSS_BONDS)
	    if (debugging("rebcb"))
	    	(void) printf("b1 is too short\n");
	    debug_print("rebcb",
	          "Left robust_extend_bond_to_cross_bond() NO CROSS\n");
#endif /* defined(DEBUG_CROSS_BONDS) */
	    return NO;
	}
	ulen = hypot(u[0],u[1]);

	for (i = 0; i < dim; ++i)
	{
	    u[i] /= ulen;
	    v[i] = ((double) Coords(b2->start)[i]) - p[i];
	    w[i] = ((double) Coords(b2->end)[i])   - p[i];
	    vmw[i] = v[i] - w[i];
	}

	scaled_length = dscaled_hypot(vmw,dh,dim);

	delta = (scaled_length < EPSILON) ? 0.0 : 0.01/scaled_length;
	tcr2_min = (b2->prev == NULL) ? -delta : -0.001*delta;
	tcr2_max = (b2->next == NULL) ? 1.0 + delta : 1.0 + 0.001*delta;

	t2mid = 0.5*(tcr2_min + tcr2_max);
	dt2 = 0.5*fabs(tcr2_max - tcr2_min);

	U_cross_VMW = u[0]*vmw[1] - u[1]*vmw[0];
	U_cross_V   = u[0]*v[1] - u[1]*v[0];
	W_cross_V   = w[0]*v[1] - w[1]*v[0];
	U_dot_V     = u[0]*v[0] + u[1]*v[1];
	U_dot_W     = u[0]*w[0] + u[1]*w[1];

#if defined(DEBUG_CROSS_BONDS)
	if (debugging("rebcb"))
	{
	    (void) printf("u = <%g, %g>, v = <%g, %g>\n",
	    	          u[0],u[1],v[0],v[1]);
	    (void) printf("w = <%g, %g>, vmw = <%g, %g>\n",
	    	          w[0],w[1],vmw[0],vmw[1]);
	    (void) printf("delta = %g, tcr2_min = %g, tcr2_max = %g\n",
	    	          delta,tcr2_min,tcr2_max);
	    (void) printf("t2mid = %g, dt2 = %g\n",t2mid,dt2);
	    (void) printf("U_cross_V = %g, U_cross_VMW = %g, W_cross_V = %g\n",
	    	          U_cross_V, U_cross_VMW,W_cross_V);
	    (void) printf("U_dot_V = %g, U_dot_W = %g\n",U_dot_V,U_dot_W);
	}
#endif /* defined(DEBUG_CROSS_BONDS) */

	if ((fabs(U_cross_V - t2mid*U_cross_VMW) > dt2*fabs(U_cross_VMW)) ||
	    	(W_cross_V*U_cross_VMW < 0.0) ||
	    	(fabs(W_cross_V) < fabs(ulen*U_cross_VMW)))
	{
#if defined(DEBUG_CROSS_BONDS)
	    if (debugging("rebcb"))	
	    	(void) printf("Cross by positive extension not on b2\n");
	    debug_print("rebcb",
	          "Left robust_extend_bond_to_cross_bond() NO CROSS\n");
#endif /* defined(DEBUG_CROSS_BONDS) */
	    return NO;
	}
	
	if (fabs(U_cross_VMW) > 0.0)
	{
	    alpha = U_cross_V/(U_cross_VMW);
	    para = (1.0 - alpha)*U_dot_V + alpha*U_dot_W;
	}
	else
	{
#if defined(DEBUG_CROSS_BONDS)
	    if (debugging("rebcb"))
	    	(void) printf("Parallel bonds\n");
#endif /* defined(DEBUG_CROSS_BONDS) */

	    if (U_dot_V < 0.0 && U_dot_W < 0.0)
	    {
#if defined(DEBUG_CROSS_BONDS)
	    	if (debugging("rebcb"))	
	    	{
	    	    (void) printf("b2 lies on opposite side of b1, "
	    	                  "with respect to extension direction\n");
	    	}
	    	debug_print("rebcb","Left robust_extend_bond_to_cross_bond() %s\n",
	              "NO CROSS");
#endif /* defined(DEBUG_CROSS_BONDS) */
	        return NO;
	    }
	    else if ((U_dot_W < 0.0) || (U_dot_V < U_dot_W))
	    {
	    	alpha = 0.0;
	    	para = U_dot_V;
	    }
	    else
	    {
	    	alpha = 1.0;
	    	para = U_dot_W;
	    }
	}
	if (para >= 0.0) 
	{
	    if (alpha < 0.0)
		alpha = 0.0;
	    if (alpha > 1.0)
		alpha = 1.0;
	    Coords(pc)[0] = Coords(b2->start)[0] + 
	    	alpha*(Coords(b2->end)[0] - Coords(b2->start)[0]);
	    Coords(pc)[1] = Coords(b2->start)[1] + 
	    	alpha*(Coords(b2->end)[1] - Coords(b2->start)[1]);
	    *tcr2 = (double) alpha;
#if defined(DEBUG_CROSS_BONDS)
	    if (debugging("rebcb"))	
	    {
	    	(void) printf("Cross by extension found at <%g, %g>\n",
	    		      Coords(pc)[0],Coords(pc)[1]);
	    	(void) printf("tcr1 = %g, tcr2 = %g\n",*tcr1,*tcr2);
	    }
	    debug_print("rebcb","Left robust_extend_bond_to_cross_bond() CROSS\n");
#endif /* defined(DEBUG_CROSS_BONDS) */
	    return YES;
	}
	else
	{
#if defined(DEBUG_CROSS_BONDS)
	    if (debugging("rebcb"))	
	    {
	    	(void) printf("b2 lies on opposite side of b1, "
	    	              "with respect to extension direction\n");
	    }
	    debug_print("rebcb","Left robust_extend_bond_to_cross_bond() NO CROSS\n");
#endif /* defined(DEBUG_CROSS_BONDS) */
	    return NO;
	}
}		/*end robust_extend_bond_to_cross_bond*/

EXPORT	int robust_extend_bonds_to_cross(
	BOND		*b1,
	ORIENTATION	c1_orient,
	int		on_b1,
	BOND		*b2,
	ORIENTATION	c2_orient,
	int		on_b2,
	FT_POINT		*oldp,
	double		*tcr1,
	double		*tcr2,
	FT_POINT		*p,
	RECT_GRID	*gr)
{
	double		hx = (double)(gr->h[0]);
	double		hy = (double)(gr->h[1]);
	double		ax = (double)(Coords(b1->start)[0] - 
	        		      Coords(b2->start)[0]);
	double		ay = (double)(Coords(b1->start)[1] - 
	        		      Coords(b2->start)[1]);
	double		bx = (double)(Coords(b1->end)[0] -
	        		      Coords(b1->start)[0]);
	double		by = (double)(Coords(b1->end)[1] -
	        		      Coords(b1->start)[1]);
	double		cx = (double)(Coords(b2->end)[0] -
	        		      Coords(b2->start)[0]);
	double		cy = (double)(Coords(b2->end)[1] -
	        		      Coords(b2->start)[1]);
	double		dx = (double)(0.5*(Coords(b1->start)[0] + 
	        			   Coords(b2->start)[0]));
	double		dy = (double)(0.5*(Coords(b1->start)[1] + 
	        			   Coords(b2->start)[1]));
	double		acc, acb, ccb, cdb;
	double		p10x, p10y, p11x, p11y;
	double		p20x, p20y, p21x, p21y;
	double		pmidx, pmidy;
	double		ex, ey, bce, bde, cce, cde;
	double		oldx = (double) Coords(oldp)[0];
	double		oldy = (double) Coords(oldp)[1];

#if defined(DEBUG_CROSS_BONDS)
	debug_print("rebcb","Entered robust_extend_bonds_to_cross()\n");
	if (debugging("rebcb"))
	{
	    (void) printf("Checking for cross by extension of bond b1 %d ",b1);
	    (void) printf("with bond b2 %d\n",b2);
	    print_orientation("c1_orient = ",c1_orient,"\n");
	    print_orientation("c2_orient = ",c2_orient,"\n");
	    (void) printf("b1 - ");		print_bond(b1);
	    (void) printf("b2 - ");		print_bond(b2);
	}
#endif /* defined(DEBUG_CROSS_BONDS) */

	if (robust_cross_bonds(b1,on_b1,b2,on_b2,tcr1,tcr2,gr,p))
	{
#if defined(DEBUG_CROSS_BONDS)
	    if (debugging("rebcb"))
	    	(void) printf("Bonds cross\n");
	    debug_print("rebcb","Left robust_extend_bonds_to_cross(), CROSS\n");
#endif /* defined(DEBUG_CROSS_BONDS) */
	    return YES;
	}

	ccb = cx*by - cy*bx;	acc = ax*cy - ay*cx;	acb = ax*by - ay*bx;

	/* Check for cross in wrong direction */

	if ((c1_orient == POSITIVE_ORIENTATION && ccb*acc > 0.0)         ||
	    (c1_orient == NEGATIVE_ORIENTATION && ccb*(ccb - acc) > 0.0) ||
	    (c2_orient == POSITIVE_ORIENTATION && ccb*acb > 0.0)         ||
	    (c2_orient == NEGATIVE_ORIENTATION && ccb*(ccb - acb) > 0.0))
	{
#if defined(DEBUG_CROSS_BONDS)
	    if (debugging("rebcb")) 
	    	(void) printf("No cross in required direction\n");
	    debug_print("rebcb","Left robust_extend_bonds_to_cross(), NO CROSS\n");
#endif /* defined(DEBUG_CROSS_BONDS) */
	    return NO;
	}

	/* Check for parallel bonds or cross far from oldp */

	if ((hx*fabs(ccb) <= fabs(ccb*(dx - oldx) + 0.5*(acc*bx + acb*cx))) ||
	    (hy*fabs(ccb) <= fabs(ccb*(dy - oldy) + 0.5*(acc*by + acb*cy))))
	{
#if defined(DEBUG_CROSS_BONDS)
	    if (debugging("rebcb"))
	    	(void) printf("parallel bonds\n");
#endif /* defined(DEBUG_CROSS_BONDS) */
	    cdb = cx*bx + cy*by;
	    if (fabs(ccb) > sqr(EPSILON)*fabs(cdb)) /* Vectors not || */
	    	return NO;
	    if (c1_orient == POSITIVE_ORIENTATION)
	    {
	    	p10x = (double) Coords(b1->start)[0];
	    	p10y = (double) Coords(b1->start)[1];
	    	p11x = (double) Coords(b1->end)[0];
	    	p11y = (double) Coords(b1->end)[1];
	    }
	    else
	    {
	    	p10x = (double) Coords(b1->end)[0];
	    	p10y = (double) Coords(b1->end)[1];
	    	p11x = (double) Coords(b1->start)[0];
	    	p11y = (double) Coords(b1->start)[1];
	    }
	    if (c2_orient == POSITIVE_ORIENTATION)
	    {
	    	p20x = (double) Coords(b2->start)[0];
	    	p20y = (double) Coords(b2->start)[1];
	    	p21x = (double) Coords(b2->end)[0];
	    	p21y = (double) Coords(b2->end)[1];
	    }
	    else
	    {
	    	p20x = (double) Coords(b2->end)[0];
	    	p20y = (double) Coords(b2->end)[1];
	    	p21x = (double) Coords(b2->start)[0];
	    	p21y = (double) Coords(b2->start)[1];
	    }
	    ex = p20x - p10x;	ey = p20y - p10y;
	    bce = bx*ey - by*ex;	bde = bx*ex + by*ey;
	    cce = cx*ey - cy*ex;	cde = cx*ex + cy*ey;
	    if ((fabs(bce) > EPSILON*fabs(bde)) || 
	    		(fabs(cce) > EPSILON*fabs(cde)))
	    	return NO;	/* Vectors not colinear */
	    pmidx = 0.5*(p11x + p21x);
	    pmidy = 0.5*(p11y + p21y);
	    if ((fabs(oldx - pmidx) > hx) || (fabs(oldy - pmidy) > hy))
	    	return NO;
	    Coords(p)[0] = (double) pmidx;	Coords(p)[1] = (double) pmidy;
	    *tcr1 = (c1_orient == POSITIVE_ORIENTATION) ? 0.0 : 1.0;
	    *tcr2 = (c2_orient == POSITIVE_ORIENTATION) ? 0.0 : 1.0;
	}
	else
	{
	    *tcr1 = acc/ccb;	*tcr2 = acb/ccb;
	    Coords(p)[0] = (double)dx + 0.5*(*tcr1*(double)bx + *tcr2*(double)cx);
	    Coords(p)[1] = (double)dy + 0.5*(*tcr1*(double)by + *tcr2*(double)cy);

	    *tcr1 = (c1_orient == POSITIVE_ORIENTATION) ?
	    		max(*tcr1,0.0) : min(*tcr1,1.0);
	    *tcr2 = (c2_orient == POSITIVE_ORIENTATION) ?
	    		max(*tcr2,0.0) : min(*tcr2,1.0);
	}
#if defined(DEBUG_CROSS_BONDS)
	if (debugging("rebcb")) 
	{
	    (void) printf("Cross by extension found\n");
	    (void) printf("at point <%g, %g>, tcr1 = %g, tcr2 = %g\n",
	    	          Coords(p)[0],Coords(p)[1],*tcr1,*tcr2);
	}
	debug_print("rebcb","Left robust_extend_bonds_to_cross(), CROSS\n");
#endif /* defined(DEBUG_CROSS_BONDS) */
	return YES;
}		/*end robust_extend_bonds_to_cross*/



EXPORT int robust_cross_bond_circle(
	BOND		*bond,
	FT_POINT		*pcenter,
	double		Rsq,
	double		*tcr,
	FT_POINT		*p)
{
	double		ax = (double) (Coords(bond->start)[0] -
	        		       Coords(pcenter)[0]);
	double		ay = (double) (Coords(bond->start)[1] -
	        		       Coords(pcenter)[1]);
	double		bx = (double) (Coords(bond->end)[0] -
	        		       Coords(bond->start)[0]);
	double		by = (double) (Coords(bond->end)[1] -
	        		       Coords(bond->start)[1]);
	double		a,b,c;
	double 		robust, hmin, hmax, h[2];
	double		rsq = (double) Rsq;
	double		rcbc_robust_fac = RCBC_ROBUST_FAC(current_interface());
	int		status = NO;

	static	double  epsilon;
	static	boolean first = YES;

	if (first == YES)
	{
	    first = NO;
	    epsilon = TOL_FAC(current_interface())*MACH_EPS;
	}

	robust = rcbc_robust_fac*((double) bond_length(bond));
	hmin = -robust;	hmax = 1.0 + robust;

	a = sqr(bx) + sqr(by);
	b = 2.0*(ax*bx + ay*by);
	c = sqr(ax) + sqr(ay) - rsq;

	switch(robust_quad_roots_in_interval(h,a,b,c,hmin,hmax,epsilon))
	{
	case 1:
	    *tcr = h[0];
	    Coords(p)[0] = Coords(bond->start)[0] + h[0]*bx;
	    Coords(p)[1] = Coords(bond->start)[1] + h[0]*by;
	    status = YES;
	    break;
	case 2:
	    /*** Voluminous output
	    (void) printf("WARNING in robust_cross_bond_circle() - "
	                  "Circle crosses bond twice\n");
	    ***/
	    status = NO;
	    break;
	case 3:
	    /*** Voluminous output
	    (void) printf("WARNING in robust_cross_bond_circle() - "
	                  "Degenerate case, zero length bond\n"
	                  "With start of bond on circle\n"
	    ***/
	    *tcr = 0.5;
	    Coords(p)[0] = 0.5*(Coords(bond->start)[0] + Coords(bond->end)[0]);
	    Coords(p)[1] = 0.5*(Coords(bond->start)[1] + Coords(bond->end)[1]);
	    status = YES;
	    break;
	case 0:
	default:
	    /*** Voluminous output
	    (void) printf("WARNING in robust_cross_bond_circle() - "
	                  "No intersection of bond and circle\n");
	    ***/
	    status = NO;
	    break;
	}

	return status;

}		/*end robust_cross_bond_circle*/


/*
*				cross_sign():
*
*	Returns the sign of the cross product of the oriented bonds b1, b2.
*/


EXPORT int cross_sign(
	BOND		*b1,
	BOND		*b2)
{
	double		dx1 = Coords(b1->end)[0] - Coords(b1->start)[0];
	double		dy1 = Coords(b1->end)[1] - Coords(b1->start)[1];
	double		dx2 = Coords(b2->end)[0] - Coords(b2->start)[0];
	double		dy2 = Coords(b2->end)[1] - Coords(b2->start)[1];

	return (dx1*dy2 - dx2*dy1 > 0. ? 1 : -1);
}		/*end cross_sign*/

/*
*			c1_to_c2_direction():
*
*	Returns the direction (CLOCKWISE or COUNTER_CLOCK) of the 
*	interior angle (<= 180 deg) between the half lines defined
*	by two curves c1 and c2 with given orientation meeting at a common node.
*/


EXPORT ANGLE_DIRECTION c1_to_c2_direction(
	O_CURVE		*c1,
	O_CURVE		*c2)
{
	int		sign;

	sign = cross_sign(Bond_at_node_of_o_curve(c1),
	        	  Bond_at_node_of_o_curve(c2));
	if (c1->orient == NEGATIVE_ORIENTATION)
	    sign *= -1;
	if (c2->orient == NEGATIVE_ORIENTATION)
	    sign *= -1;
	return (sign == 1 ? COUNTER_CLOCK : CLOCKWISE);
}		/*end c1_to_c2_direction*/


/*
*			big_angle():
*
*	Computes angle between bonds b1,b2:  It is assumed that
*	the end of b1 coincides with the start of b2.
*	Special treatment is required for nodes and closed curves.
*	If the common point is a node, this node should be either a
*	boundary point or else the node of a closed curve.
*/

EXPORT void big_angle(
	BOND		*b1,
	CURVE		*c1,
	BOND		*b2,
	CURVE		*c2,
	double		*cos_angle,
	double		*sin_angle,
	RECT_GRID	*gr)
{
	double slopex,slopey,slopex_next,slopey_next;
	double distance,next_distance,xm,xl,xp,ym,yl,yp;

	if (b1==NULL && b2==NULL)
	{
	    screen("ERROR in big_angle(), null bonds\n");
	    clean_up(ERROR);
	}
	if (b1==NULL && (c1==c2) && is_closed_curve(c1))
	    b1 = c1->last;
	if (b2==NULL && (c1==c2) && is_closed_curve(c1))
	    b2 = c1->first;

	if (b1==NULL)				/* b2 starts curve */
	{
	    xl = Coords(b2->start)[0];	yl = Coords(b2->start)[1];
	    xp = Coords(b2->end)[0];	yp = Coords(b2->end)[1];
	    reflect(xl,yl,xp,yp,&xm,&ym,gr);
	}
	else if (b2==NULL)			/* b1 ends curve */
	{
	    xl = Coords(b1->end)[0];	yl = Coords(b1->end)[1];
	    xm = Coords(b1->start)[0];	ym = Coords(b1->start)[1];
	    reflect(xl,yl,xm,ym,&xp,&yp,gr);
	}
	else
	{
	    xm = Coords(b1->start)[0];	ym = Coords(b1->start)[1];
	    xl = Coords(b1->end)[0];	yl = Coords(b1->end)[1];
	    xp = Coords(b2->end)[0];	yp = Coords(b2->end)[1];
	}

	distance      = hypot(xm-xl,ym-yl);
	next_distance = hypot(xl-xp,yl-yp);
	slopex        = (xl - xm)/distance;
	slopey        = (yl - ym)/distance;
	slopex_next   = (xp - xl)/next_distance;
	slopey_next   = (yp - yl)/next_distance;
	*cos_angle    = slopex*slopex_next + slopey*slopey_next;
	*sin_angle    = slopex*slopey_next - slopey*slopex_next;
}		/*end big_angle*/

/*
*			reflect():
*			
*	Reflects the point x,y about the boundary, returning xr,yr.  The
*	boundary segment for reflection is determined by the condition 
*	that xl,yl lie of the boundary.
*/

LOCAL void reflect(
	double		xl,
	double		yl,
	double		x,
	double		y,
	double		*xr,
	double		*yr,
	RECT_GRID	*gr)
{
	double	rtol = RTOL(current_interface());
	if (xl < gr->L[0] + rtol*gr->h[0])
	{
	    *xr = 2*gr->L[0] - x;
	    *yr = y;
	}
	if (xl > gr->U[0] - rtol*gr->h[0])
	{ 
	    *xr = 2*gr->U[0] - x;
	    *yr = y;
	}
	if (yl < gr->L[1] + rtol*gr->h[1])
	{
	    *yr = 2*gr->L[1] - y;
	    *xr = x;
	}
	if (yl > gr->U[1] - rtol*gr->h[1])
	{
	    *yr = 2*gr->U[1] - y;
	    *xr = x;
	}
}		/*end reflect*/



/*
*		intersect_bond_with_curve_segment():
*
*	Finds the intersection if any of the bond b and the segment
*	of curve c from the bonds b1 and b2 inclusive.
*	It is assume that b2 follows b1 with respect to the given
*	orientation.
*	Returns YES if successful along with the point of intersection
*	pint and the bond bint on which the intersection lies.
*/

EXPORT	int intersect_bond_with_curve_segment(
	BOND		*b,
	BOND		*b1,
	BOND		*b2,
	O_CURVE		*oc,
	BOND		**bint,
	FT_POINT		*pint,
	RECT_GRID	*gr)
{
	double		tcr1, tcr2;
	BOND		*bb;
	ORIENTATION	orient = oc->orient;

	for (bb = b1; bb != NULL; bb = Following_bond(bb,orient))
	{
	    if (robust_cross_bonds(b,YES,bb,YES,&tcr1,&tcr2,gr,pint))
	    {
	    	*bint = bb;
	    	return YES;
	    }
	    if (bb == b2)
		break;
	}

	return NO;
}		/*end intersect_bond_with_curve_segment*/

/*
*			is_short_curve():
*
*	Defines a curve as short on two criteria, one which is numerical
*	(gr->h[0], gr->h[1]) and the other which is physical (gr->X, gr->Y).
*	The physical criterion is relevent for extremely coarse grids.
*
*	The variable len scales the metric used to measure shortness.
*/

EXPORT int is_short_curve(
	CURVE		*c,
	ORIENTATION	c_orient,
	RECT_GRID	*gr,
	double		len)
{
	BOND		*b;
	double		*p, *p0;
	double		H[MAXD];
	int		i, dim = gr->dim;

	for (i = 0; i < dim; ++i)
	    H[i] = len*min(gr->h[i],0.25*(gr->U[i] - gr->L[i]));

	if (c->num_points <= SHORT_CURVE_NUM_POINTS(c->interface))
	    return YES;

	p0 = Coords(Node_of(c,c_orient)->posn);

	for (b = Bond_at_node(c,c_orient); b; b = Following_bond(b,c_orient))
	{
	    p = (c_orient == POSITIVE_ORIENTATION) ? Coords(b->end) :
	    					     Coords(b->start);

	    for (i = 0; i < dim; ++i)
	    	if (fabs(p[i] - p0[i]) >= H[i])
	            return NO;
	}
	return YES;
}		/*end is_short_curve*/

/*
*		robust_quad_roots_in_interval():
*
*	Find the real roots of the equation Ax**2 + Bx + C = 0
*	that lie in the interval [x0, x1].
*	The robustness comes from the condition that
*	quantities < epsilon are considered to be zero.
*	Returns the number of distinct real roots, unless
*	A, B and C are all zero, in which case it returns 3.
*/


EXPORT int robust_quad_roots_in_interval(
	double		*root,
	double		A,
	double		B,
	double		C,
	double		x0,
	double		x1,
	double		epsilon)
{
	double		xmid, len;
	double		maxabc, disc, sqrtd, Y;
	double		a, b, c;
	double		z;
	int		num_roots = 0;
	static boolean	first = YES;
	static double	mac_tol;

	debug_print("quad_roots","Entered robust_quad_roots_in_interval()\n");
	if (debugging("quad_roots"))
	{
	    (void) printf("epsilon = %g\n",epsilon);
	    (void) printf("A = %.32g, B = %.32g, C = %.32g\n",A,B,C);
	    (void) printf("x0 = %.32g, x1 = %.32g\n",x0,x1);
	}
	if (first == YES)
	{
	    first = NO;

	    mac_tol = TOL_FAC(current_interface())*MACH_EPS;
	}
	len = fabs(x1 - x0);
	if (x1 < x0)
	{
	    double xtmp;

	    xtmp = x1;
	    x1 = x0;
	    x0 = xtmp;
	}
	x0 -= mac_tol*len; x1 += mac_tol*len;
	xmid = 0.5*(x0 + x1), len = 0.5*fabs(x1 - x0);
	a = A*len*len; b = (2.0*A*xmid + B)*len; c = (A*xmid + B)*xmid + C;
	if (b > 0.0)
	{
	    a = -a;
	    b = -b;
	    c = -c;
	}

	if (fabs(a) + fabs(b) + fabs(c) < epsilon)
	{
	    num_roots = 3;
	    if (debugging("quad_roots"))
	    {
	    	(void) printf("Degenerate quadratic\n"
	    	              "a = %.32g, b = %.32g, c = %.32g\n",a,b,c);
	    }
	    debug_print("quad_roots",
	          "Left robust_quad_roots_in_interval(), num_roots = %d\n",
	          num_roots);
	    return num_roots;
	}


	maxabc = max(fabs(b),fabs(c));
	maxabc = max(fabs(a),maxabc);

	a /= maxabc;	b /= maxabc;	c /= maxabc;
	disc = b*b - 4.0*a*c;
	sqrtd = sqrt(fabs(disc));
	if (debugging("quad_roots"))
	{
	    (void) printf("maxabc = %.32g\n",maxabc);
	    (void) printf("After normalization - ");
	    (void) printf("a = %.32g, b = %.32g, c = %.32g\n",a,b,c);
	    (void) printf("disc = %.32g\n",disc);
	}

	if (fabs(a + c) < fabs(b))
	{
	    if (debugging("quad_roots"))
	    	(void) printf("Single real root in interval\n");
	    z = 2.0*c/(-b + sqrtd);
	    root[0] = root[1] = xmid + len*z;
	    num_roots = 1;
	    if (debugging("quad_roots"))
	    	(void) printf("root = %.32g\n",root[0]);
	}
	else if (fabs(b) < 2.0*fabs(a) && sqrtd < 2.0*a*epsilon)
	{
	    if (debugging("quad_roots"))
	    	(void) printf("zero discriminant\n");
	    z = -0.5*b/a;
	    root[0] = root[1] = xmid + len*z;
	    num_roots = 1;
	    if (debugging("quad_roots"))
	    	(void) printf("root = %.32g\n",root[0]);
	}
	else if (disc > 0.0 && fabs(c) < fabs(a))
	{
	    if (debugging("quad_roots"))
	    	(void) printf("Two roots in interval\n");
	    num_roots = 2;
	    Y = -b + sqrtd;
	    if (a > 0.0)
	    {
	    	z = 2.0*c/Y;	root[0] = xmid + len*z;
	    	z = 0.5*Y/a;	root[1] = xmid + len*z;
	    }
	    else
	    {
	    	z = 0.5*Y/a;	root[0] = xmid + len*z;
	    	z = 2.0*c/Y;	root[1] = xmid + len*z;
	    }
	    if (debugging("quad_roots"))
	    	(void) printf("root[0] = %.32g, root[1] = %.32g\n",
	    	              root[0],root[1]);
	}
	debug_print("quad_roots",
	      "Left robust_quad_roots_in_interval(), num_roots = %d\n",
	      num_roots);
	return num_roots;
}		/*end robust_quad_roots_in_interval*/

EXPORT	void init_seg_crx_lists(
	INTERFACE	*intfc,
	int		n_crx,
	int		size)
{
	int		i;
	int		*scls;
	Table		*T = table_of_interface(intfc);

	n_crx += MAX_CRX_FILL;/*Storage for missing crosses*/
	uni_array(&T->seg_crx_lists,size,sizeof(int *));
	uni_array(&T->seg_crx_lists_store,n_crx,sizeof(int));
	uni_array(&T->crx_store,n_crx,sizeof(CRXING));

	scls = T->seg_crx_lists_store;
	for (i = 0;  i < size;  ++i)
	{
	    if (T->seg_crx_count[i] == 0)
	    	T->seg_crx_lists[i] = NULL;
	    else
	    {
	    	T->seg_crx_lists[i] = scls;
	    	scls += T->seg_crx_count[i];
	    }
	}	
	for (i = 0;  i < n_crx;  ++i)
	    T->seg_crx_lists_store[i] = -1;

}		/*end init_seg_crx_lists*/

EXPORT	void init_face_crx_lists(
	INTERFACE	*intfc,
	int             n_curve_crx,
	int		size)
{
	int		i;
	int		*scls;
	Table		*T = table_of_interface(intfc);

	n_curve_crx += MAX_CRX_FILL;
        uni_array(&T->curve_crx_lists,size,sizeof(int*));
        uni_array(&T->curve_crx_lists_store,n_curve_crx,sizeof(int));
        uni_array(&T->curve_crx_store,n_curve_crx,sizeof(CRXING));

	scls = T->curve_crx_lists_store;
        for (i = 0;  i < size;  ++i)
        {
            if (T->curve_crx_count[i] == 0)
                T->curve_crx_lists[i] = NULL;
            else
            {
                T->curve_crx_lists[i] = scls;
                scls += T->curve_crx_count[i];
            }
        }
	for (i = 0;  i < n_curve_crx;  ++i)
	    T->curve_crx_lists_store[i] = -1;

}	/*end init_face_crx_lists*/


EXPORT  const char *grid_direction_name(
        GRID_DIRECTION dir)
{
        switch (dir)
        {
        case NORTH:
            return "NORTH";
        case SOUTH:
            return "SOUTH";
        case EAST:
            return "EAST";
        case WEST:
            return "WEST";
        case LOWER:
            return "LOWER";
        case UPPER:
            return "UPPER";
        }
        return "UNKNOWN";
}               /*end grid_direction_name*/


EXPORT	const char *crossing_direction_name(
	CROSSING_DIRECTION dir)
{
	switch (dir)
	{
	case LEFT_TO_RIGHT:
	    return "LEFT_TO_RIGHT";
	case RIGHT_TO_LEFT:
	    return "RIGHT_TO_LEFT";
	case BELOW_TO_ABOVE:
	    return "BELOW_TO_ABOVE";
	case ABOVE_TO_BELOW:
	    return "ABOVE_TO_BELOW";
	}
	return "UNKNOWN";
}		/*end crossing_direction_name*/


/*ARGSUSED*/
EXPORT	void print_crxings(
	CRXING	*cr,
	boolean	prt_hs)
{
	INTERFACE *intfc;
	if (cr == NULL)
	{
	    (void) printf("\tCRXING NULL\n");
	    return;
	}
	(void) printf("\tCRXING %p crx_num %d ",(POINTER)cr,cr->crx_num);
	(void) printf("hyper surface %llu\n",hypersurface_number(cr->hs));
	(void) printf("\t\tpt %llu ",point_number(cr->pt));
	if (cr->pt == NULL)
	{
	    (void) printf("\n");
	    return;
	}
	intfc = cr->hs->interface;
	print_general_vector("",Coords(cr->pt),intfc->dim,"\n");
	print_int_vector("lower grid index = ",cr->icl,intfc->dim,"\n");
	switch (current_interface()->dim)
	{
	case 1:
	    break;
	case 2:
	    (void) printf("end %d",cr->end);
	    (void) printf("\n\t\t");
	    (void) printf("cr->direction = %s\n",
			  crossing_direction_name(cr->crossing_direction));
	    (void) printf("positive_component = %d  negative_component = %d\n",
			positive_component(cr->hs),negative_component(cr->hs));
	    break;
	case 3:
	    (void) printf("lcomp = %d, ucomp = %d\n",cr->lcomp,cr->ucomp);
	    (void) printf("dir = %d\n",cr->dir);
	    (void) printf("a0 = %g a1 = %g a2 = %g,  a0+a1+a2 = %g\n",
			  cr->a[0],cr->a[1],cr->a[2],
			  cr->a[0]+cr->a[1]+cr->a[2]);
	    (void) printf("dp2 = %g %g %g\n",
			  cr->dp2[0],cr->dp2[1],cr->dp2[2]);
	    (void) printf("ds2 = %g %g %g\n",
			  cr->ds2[0],cr->ds2[1],cr->ds2[2]);
	    (void) printf("index = %d\n",cr->index);
	    (void) printf("cr->tri = %llu\n",tri_number(cr->tri,intfc));
	    print_tri(cr->tri,intfc);
	    if (!is_side01_a_bond(cr->tri))
	    {
		(void) printf("Tri on side 01\n");
		print_tri(Tri_on_side01(cr->tri),intfc);
	    }
	    if (!is_side12_a_bond(cr->tri))
	    {
		(void) printf("Tri on side 12\n");
		print_tri(Tri_on_side12(cr->tri),intfc);
	    }
	    if (!is_side20_a_bond(cr->tri))
	    {
		(void) printf("Tri on side 20\n");
		print_tri(Tri_on_side20(cr->tri),intfc);
	    }
	    if (Point_on_tri(cr->tri,cr->pt))
	    {
		TRI       **tris;
		int       i, nt;

	        nt = set_tri_list_around_point(cr->pt,cr->tri,&tris,intfc);
		for (i = 0; i < nt; ++i)
		{
		    (void) printf("Triangle %d at cr\n",i);
		    print_tri(tris[i],intfc);
		    (void) printf("\n");
		}
	    }
	    break;
	}
	if ((cr->hs!=NULL) && (cr->hs->interface!=NULL) && (prt_hs==YES))
	    print_hypersurface(cr->hs);
}		/* end print_crxings */

EXPORT boolean pointer_in_list(         
	POINTER         pt,         
	int             num_pts,         
	POINTER         *pt_list) 
{         
	int             i;          
	for (i = 0; i < num_pts; ++i)
        {             
	    if (pt == pt_list[i])
                return YES;
        }
        return NO;
}               /*end pointer_in_list*/

EXPORT boolean integer_in_list(
        int       n,
        int       num_int,
        int       *n_list)
{
        int             i;

        for (i = 0; i < num_int; ++i)
        {
            if (n == n_list[i])                 
		return YES;         
	}         
	return NO; 
}       /*end integer_in_list*/


/* WLSP method for normal and curvature */
#define         MAX_RING1_PTS            20
#define         MAX_RING2_PTS            100
LOCAL void polyfit2d_lhf(double*,double*,double*,int,int,int);
LOCAL void polyfit3d_lhf(double*,int*,double*,double*,double*,int,double*,
                                        int,int,int);
LOCAL void obtain_orthbases(double*,double*);
LOCAL void eval_curvature_lhf_surf(double*,double*,double*,double H[2][2]);
LOCAL int safeqr(double*,int*,int*,double*,int*,double);
LOCAL int eval_vander_bivar(double*,int,double*,int*,int,double*,int,int);

#define ct_set_max(e1,e2,e3)  (e1 = e2, (e1 < e3 ? e1 = e3 : e1))
#define ct_set_min(e1,e2,e3)  (e1 = e2, (e1 > e3 ? e1 = e3 : e1))

/*******************************************************************
 *  Function for the curvature computation of six points with LSQ   *
 *******************************************************************/

EXPORT boolean WLSP_compute_normal2d(
        FT_POINT           *p,
        HYPER_SURF_ELEMENT      *hse,
        HYPER_SURF              *hs)
{
        double           curvature = 0.0;
        CURVE           *c = Curve_of_hs(hs);
        BOND            *b = Bond_of_hse(hse);
        double normal[MAXD];
	static FT_POINT	**pts;
        static double **ngbpts;
        int i,num_pts = 5;

        if (b == NULL || c == NULL)
            return NO;

	if (ngbpts == NULL)
	{
	    uni_array(&pts,num_pts,sizeof(FT_POINT*));
	    bi_array(&ngbpts,num_pts,2,FLOAT);
	}

	if (!IntfcGetPointChain(p,pts,num_pts))
	{
	    return NO;
	}
        /* 2*nrad+1 points fitting */
	for (i = 0; i < num_pts; ++i)
        {
            ngbpts[i][0] = Coords(pts[i])[0];
            ngbpts[i][1] = Coords(pts[i])[1];
        }
        polyfit2d_lhf(normal,&curvature,*ngbpts,num_pts,num_pts/2+1,num_pts/2);
	if (p == c->start->posn)
	{
            c->nor_start[0] = normal[0];
            c->nor_start[1] = normal[1];
            c->curvature_start = curvature;
	}
	else if (p == c->end->posn)
	{
            c->nor_end[0] = normal[0];
            c->nor_end[1] = normal[1];
            c->curvature_end = curvature;
	}
	else
	{
            p->_nor[0] = normal[0];
            p->_nor[1] = normal[1];
            p->curvature = curvature;
	}
	return YES;
}	/* end WLSP_compute_normal2d */              


/**********************************************************************
 *      This is a new function for Yanhong Zhao to experiment new     *
 *      algorithm for the calculation of curvature.    3D             *
 **********************************************************************/

LOCAL void obtain_orthbases(
        double *tang,
        double *nrm1st)
{
      	double t[3],t2[3];
      	double norm,k;
      	int i;
      	if (fabs(nrm1st[0]) > fabs(nrm1st[1]) && 
	    fabs(nrm1st[0]) > fabs(nrm1st[2]))
      	{
            t[0] = 0.0;
            t[1] = 1.0;
            t[2] = 0.0;
      	}
      	else
      	{
            t[0] = 1.0;
            t[1] = 0.0;
            t[2] = 0.0;
        }
        norm = 0.0;
        k = Dot3d(t,nrm1st);

  	for (i = 0;i < 3; i++)
      	{
            t[i] = t[i] - k*nrm1st[i];
      	}
      	norm = Mag3d(t);
      	for (i = 0;i < 3; i++)
      	{
            tang[i] = t[i]/norm;
      	}

      	Cross3d(tang,nrm1st,t2);
      	for (i = 0; i < 3; i++)
            tang[i+3] = t2[i];
}	/* end obtain_orthbases */

/*************************************************************
 *                FUNCTION: polyfit2d_lhf                    *
 *************************************************************/
LOCAL void polyfit2d_lhf(
        double normal_out[2],
        double *curvature_out,
        double *ngbpts,
        int ngbpts_dim1,
        int mid,
        int deg)
{
        static double newpts[26];
        static double AS[91];
        int AS_dim1, AS_dim2;
        double b[13];
        int b_dim1;
        double b_1[13];
        double ws[7];
        static double dtemp_0[26];
        static double dtemp_1[91];
        int npnts, ncols, rnk, n, i;
        double tng[2], nrm[2];
        double Q_rotation[2][2];
        double u_max, w, f_t, f_tt, ell;
        double dtemp_2, dtemp_3, dtemp_4, dtemp_5, dtemp_6;
        int j;
        double dtemp_7;
 	/*npnts is the number of points being fit */
        npnts = ngbpts_dim1;
        
        /*Calculate the tangent vector using edge-length weighting */

        tng[0] =  ngbpts[(mid - 1) << 1]
                 -  ngbpts[(mid - 2) << 1]
                 + ( ngbpts[mid << 1] -  ngbpts[(mid - 1) << 1]);
        tng[1] =  ngbpts[1 + ((mid - 1) << 1)]
                 -  ngbpts[1 + ((mid - 2) << 1)]
                 + ( ngbpts[1 + (mid << 1)] -  ngbpts[1 + ((mid - 1) << 1)]);
        dtemp_2 = sqrt(tng[0]*tng[0] +  tng[1]*tng[1]);
        tng[0] =  tng[0]/dtemp_2;
        tng[1] =  tng[1]/dtemp_2;
        nrm[0] = - tng[1];
        nrm[1] = tng[0];

        /*normal vector */
        Q_rotation[0][0] = tng[0];
        Q_rotation[1][0] = tng[1];
        Q_rotation[0][1] = nrm[0];
        Q_rotation[1][1] = nrm[1];

        /*the rotation matrix */
        /*Calculate the coordinates in the newly local uv-axis */

        for (i = 0; i <= npnts - 1; i += 1)
        {
            newpts[i << 1] = (ngbpts[i << 1] -  ngbpts[(mid << 1) - 2])
                            *Q_rotation[0][0] + (ngbpts[1 + (i << 1)]
                            -  ngbpts[(mid << 1) - 1])*Q_rotation[1][0];
            newpts[1 + (i << 1)] = (ngbpts[i << 1] -  ngbpts[(mid << 1) - 2])
                            *Q_rotation[0][1] + (ngbpts[1 + (i << 1)]
                            -  ngbpts[(mid << 1) - 1])*Q_rotation[1][1];
        }

        /* Calculate Vandermonde matrix */
        ncols = deg + 1;
        AS_dim1 = npnts;
        AS_dim2 = ncols;
        for (i = 0; i <= AS_dim1 - 1; i += 1)
        {
            AS[i*AS_dim2] = 1.0;
        }
        for (i = 0; i <= npnts - 1; i += 1)
        {
            dtemp_0[i] = newpts[i << 1];
        }
        for (i = 0; i <= AS_dim1 - 1; i += 1)
        {
            AS[1 + i*AS_dim2] = dtemp_0[i];
        }
        for (i = 3; i <= ncols; i += 1)
        {
            for (j = 0; j <= AS_dim1 - 1; j += 1)
            {
                dtemp_1[j] =  AS[ j*AS_dim2 - 2 + i]*newpts[j << 1];
            }
            for (j = 0; j <= AS_dim1 - 1; j += 1)
            {
                AS[j*AS_dim2 - 1 + i] = dtemp_1[j];
            }
        }

        /* Assign a weight to each columns of AS */
        u_max = -HUGE_VAL;
        b_dim1 = npnts;
        for (i = 0; i <= npnts - 1; i += 1)
        {
            u_max = max(fabs(newpts[i << 1]), u_max);
            b[i] = newpts[1 + (i << 1)];
        }

        for (i = 0; i <= npnts - 1; i += 1)
        {
            dtemp_4 = fabs(newpts[i << 1] + 0.1*u_max);
            dtemp_5 = pow(dtemp_4, deg);
            w = 1.0;
            for (j = 0; j <= AS_dim2 - 1; j += 1)
            {
                AS[i*AS_dim2 + j] =  AS[i*AS_dim2 + j]*w;
            }
            b[i] = b[i]*w;
          }

        /* Rescale Vandermonde matrix */
        for (i = 0; i <= ncols - 1; i += 1)
        {
            dtemp_3 = 0.0;
            for (j = 0; j <= AS_dim1 - 1; j += 1)
            {
                dtemp_3 = dtemp_3 +  AS[j*AS_dim2 + i]*AS[j*AS_dim2 + i];
            }
            dtemp_7 = sqrt(dtemp_3);
            for (j = 0; j <= AS_dim1 - 1; j += 1)
            {
                AS[j*AS_dim2 + i] =  AS[j*AS_dim2 + i]/dtemp_7;
            }
            ws[i] = dtemp_7;
        }
        /* Solve using QR factorization.  */
        /* Afterwards, AS contains R and b contains Q'b */
        rnk = safeqr(AS, &AS_dim1, &AS_dim2, b, &b_dim1, 0.001);
        if ((rnk < ncols) && (rnk < 3))
            rnk = 3;

        for (i = 0; i <= b_dim1 - 1; i += 1)
        {
            b_1[i] = b[i];
        }
        /* Perform backward substitution to compute  */
        /* bs = triu(R(1:ncols,1:ncols))\bs; */
        n = rnk;
        for (i = n; i >= 1; i += -1)
        {
            for (j = 1 + i; j <= n; j += 1)
            {
                b_1[i - 1] =  b_1[i - 1] - AS[(i - 1)*AS_dim2 - 1 + j]
                                   *b_1[j - 1];
            }
            b_1[i - 1] =  b_1[i - 1]/AS[(i - 1)*AS_dim2 - 1 + i];
        }
        /*Calculate normal and curvature */
        f_t =  b_1[1]/ws[1];
        f_tt = 2.0*b_1[2]/ws[2];
        ell = sqrt(1.0 + f_t*f_t);
        dtemp_6 = -f_t;
        normal_out[0] =  -Q_rotation[0][0]*(dtemp_6 / ell);
        normal_out[0] = normal_out[0] -  Q_rotation[0][1]*(1.0/ell);
        normal_out[1] =  -Q_rotation[1][0]*(dtemp_6 / ell);
        normal_out[1] = normal_out[1] -  Q_rotation[1][1]*(1.0/ell);
        *curvature_out = f_tt/sqrt(ell*ell*ell);
        return;
}	/* end polyfit2d_lhf */

/*************************************************************
 *
 * FUNCTION: polyfit3d_lhf
 *
 *POLYFIT_LHF3D Compute normal, principal curvatures, and principal direction.
 * [NRM,DEG,PRCURVS,MAXPRDIR] = POLYFIT_LHF_SURF_POINT(XS, NRMS_COOR, DEGREE, STRIP)
 * Computes normal NRM, principal curvatures 
 * xs: xs(1,1:3) is the target point and  xs(2:end,1:3)  are neighbor points
 * nrm_coor: nrm_coor(1,1:3) is the normal at target point and  nrm_coor(2:end,1:3) are normals at neighbor points
 *************************************************************/

LOCAL void  polyfit3d_lhf(
	double nrm_out[3], 
	int *deg_out, 
	double prcurvs_out[2], 
	double maxprdir_out[3], 
	double *xs, 
	int xs_dim1, 
	double *nrm_coor, 
	int nrm_coor_dim1, 
	int degree, 
	int strip)
{
   	static double us[3075];
   	int us_dim1;
   	static double bs[1025];
   	int bs_dim1;
   	static double nrms_v[3075];
   	int nrms_v_dim1;
   	static double ws_row[1025];
   	int ws_row_dim1;
   	double cs[6];
   	int cs_dim1;
   	static double dtemp_0[3075];
   	static double dtemp_1[3075];
   	int dtemp_1_dim1;
   	static double dtemp_2[3075];
   	int strip_1, nverts, n, itemp_0, itemp_1;
   	double absnrm[3], t1_1[3], t2[3], nrm_l[3], maxprdir_l[3];
   	int t1[3];
   	double grad[2];
   	double P[3][3];
   	double H[2][2];
   	static int maxprdir_1[3] = {0, 0, 0};
   	static int t1_2[3] = {1, 0, 0};
   	static int t1_3[3] = {0, 1, 0};
   	double dtemp_3[3];
   	double dtemp_4, dtemp_5, dtemp_6, dtemp_7, dtemp_8;
   	double dtemp_9, dtemp_10, dtemp_11, dtemp_12, dtemp_13;
   	double dtemp_14, dtemp_15, dtemp_16, dtemp_17, dtemp_18;
   	int itemp_2, i, j;
   	double dtemp_19;

   	strip_1 = strip;
   	/* First, compute the rotation matrix */

   	for (i=0; i<=2; i+=1) 
	{
      	    absnrm[i] = fabs(nrm_coor[i]);
      	    nrm_out[i] = nrm_coor[i];
   	}
   	if (( absnrm[0] >  absnrm[1])&&( absnrm[0] >  absnrm[2])) 
      	    for (i=0; i<=2; i+=1) 
         	t1[i] = t1_3[i];
   	else 
      	    for (i=0; i<=2; i+=1) 
         	t1[i] = t1_2[i];
   	dtemp_4 = 0.0;
   	for (i=0; i<=2; i+=1) 
      	    dtemp_4 = dtemp_4 + (((double) t1[i])) *  nrm_out[i];
   	dtemp_5 = 0.0;
   	for (i=0; i<=2; i+=1) 
	{
      	    dtemp_18 = (((double) t1[i])) - dtemp_4 *  nrm_out[i];
      	    dtemp_5 = dtemp_5 + dtemp_18 * dtemp_18;
      	    t1_1[i] = dtemp_18;
   	}
   	dtemp_6 = sqrt(dtemp_5);
   	for (i=0; i<=2; i+=1) 
      	    t1_1[i] =  t1_1[i] / dtemp_6;
   	/* CROSS: Eficient routine for computing cross product */

   	t2[0] =  nrm_out[1] *  t1_1[2] -  nrm_out[2] *  t1_1[1];
   	t2[1] =  nrm_out[2] *  t1_1[0] -  nrm_out[0] *  t1_1[2];
   	t2[2] =  nrm_out[0] *  t1_1[1] -  nrm_out[1] *  t1_1[0];
   	nverts = xs_dim1;
   	/* Evaluate local coordinate system */

   	us_dim1 = nverts - strip;
   	if (nverts < strip) 
      	    us_dim1 = 0;
   	memset(us, 0, ((nverts - strip) << 1) * 8);
   	ct_set_max(bs_dim1, nverts - strip, 0);
   	memset(bs, 0, (nverts - strip) * 8);
   	us[0] = 0.0;
   	us[1] = 0.0;
   	/* compute weight based on angles of normals */

   	nrms_v_dim1 = max(nverts - 1, 0);
   	for (i=0; i<=nverts - 2; i+=1) 
	{
      	    dtemp_7 = 0.0;
      	    dtemp_8 = 0.0;
      	    dtemp_9 = 0.0;
      	    for (j=0; j<=2; j+=1) 
	    {
           	dtemp_19 =  xs[ (1 + i) * 3 + j] -  xs[j];
           	dtemp_7 = dtemp_7 + dtemp_19 *  t1_1[j];
           	dtemp_8 = dtemp_8 + dtemp_19 *  t2[j];
           	dtemp_9 = dtemp_9 + dtemp_19 *  nrm_out[j];
      	    }
      	    us[ 2 - (strip << 1) + (i << 1)] = dtemp_7;
      	    us[ 3 - (strip << 1) + (i << 1)] = dtemp_8;
      	    bs[ 1 - strip + i] = dtemp_9;
      	    for (j=0; j<=3 - 1; j+=1) 
         	nrms_v[ i * 3 + j] = nrm_coor[ (1 + i) * 3 + j];
   	}
   	if (!strip_1) 
	{
      	    strip_1 = 0;
      	    for (i=0; i<=nrms_v_dim1 - 1; i+=1) 
         	dtemp_0[i] = nrms_v[i*3]*nrm_out[0] + nrms_v[1+i*3]*nrm_out[1];
      	    itemp_0 = max(nrms_v_dim1, 0);
      	    if (nrms_v_dim1 == 1) 
	    {
         	dtemp_1_dim1 = itemp_0;
         	for (i=0; i<=itemp_0 - 1; i+=1) 
            	    dtemp_1[i] =  dtemp_0[0] +  nrms_v[2 + i * 3] *  nrm_out[2];
      	    }
      	    else 
	    {
         	dtemp_1_dim1 = nrms_v_dim1;
         	if (itemp_0 == 1) 
            	    for (i=0; i<=nrms_v_dim1 - 1; i+=1) 
               		dtemp_1[i] = dtemp_0[i] + nrms_v[2]*nrm_out[2];
         	else 
            	    for (i=0; i<=dtemp_1_dim1 - 1; i+=1) 
               		dtemp_1[i] = dtemp_0[i] + nrms_v[2+i*3]*nrm_out[2];
      	    }
      	    itemp_1 = 0;
      	    if (dtemp_1_dim1) 
         	itemp_1 = dtemp_1_dim1;
      	    ws_row_dim1 = itemp_1 + 1;
      	    ws_row[0] = 1.0;
      	    for (i=0; i<=dtemp_1_dim1 - 1; i+=1) 
	    {
         	dtemp_2[i] = max(dtemp_1[i], 0.0);
         	ws_row[1 + i] = dtemp_2[i];
      	    }
   	}
   	else 
	{
      	    ws_row_dim1 = nrms_v_dim1;
      	    for (i=0; i<=nrms_v_dim1 - 1; i+=1) 
	    {
         	dtemp_17 = nrms_v[i*3]*nrm_out[0] + nrms_v[1 + i*3]*nrm_out[1] 
			+ nrms_v[2+i*3]*nrm_out[2];
         	dtemp_10 = max(dtemp_17, 0.0);
         	ws_row[i] = dtemp_10;
      	    }
   	}
   	/* Compute the coefficients */

   	*deg_out = eval_vander_bivar(us,us_dim1,bs,&bs_dim1,degree,ws_row,
			ws_row_dim1, strip_1);
   	/* Convert coefficients into normals and curvatures */

   	if ((*deg_out) == 1) 
      	    n = 3 - strip_1;
   	else 
      	    n = 6 - strip_1;
   	itemp_2 = max(strip_1 - 1 + n, 0);
   	if (bs_dim1 > 1) 
      	    cs_dim1 = itemp_2;
   	else 
      	    cs_dim1 = 1;
   	for (i=0; i<=cs_dim1 - 1; i+=1) 
      	    cs[i] = bs[ 1 - strip_1 + i];
   	grad[0] = cs[0];
   	grad[1] = cs[1];
   	dtemp_3[0] = - grad[0];
   	dtemp_3[1] = - grad[1];
   	dtemp_3[2] = 1.0;
   	dtemp_5 = sqrt(1.0 + (grad[0]*grad[0] + grad[1]*grad[1]));
   	for (i=0; i<=2; i+=1) 	
	{
      	    nrm_l[i] =  dtemp_3[i] / dtemp_5;
      	    P[i][0] = t1_1[i];
      	    P[i][1] = t2[i];
      	    P[i][2] = nrm_out[i];
   	}
   	/* nrm = P * nrm_l; */

   	dtemp_11 = 0.0;
   	dtemp_12 = 0.0;
   	dtemp_13 = 0.0;
   	for (i=0; i<=2; i+=1) 
	{
      	    dtemp_11 = dtemp_11 +  P[0][i] *  nrm_l[i];
      	    dtemp_12 = dtemp_12 +  P[1][i] *  nrm_l[i];
      	    dtemp_13 = dtemp_13 +  P[2][i] *  nrm_l[i];
   	}
   	nrm_out[0] = dtemp_11;
   	nrm_out[1] = dtemp_12;
   	nrm_out[2] = dtemp_13;
   	if ((cs_dim1 >= 5)&&((*deg_out) > 1)) 
	{
      	    H[0][0] = cs[2];
      	    H[0][1] = cs[3];
      	    H[1][0] = cs[3];
      	    H[1][1] = cs[4];
      	    eval_curvature_lhf_surf(prcurvs_out, maxprdir_l, grad, H);
      	    /* maxprdir = P * maxprdir_l; */

      	    dtemp_14 = 0.0;
      	    dtemp_15 = 0.0;
      	    dtemp_16 = 0.0;
      	    for (i=0; i<=2; i+=1) 
	    {
         	dtemp_14 = dtemp_14 +  P[0][i] *  maxprdir_l[i];
         	dtemp_15 = dtemp_15 +  P[1][i] *  maxprdir_l[i];
         	dtemp_16 = dtemp_16 +  P[2][i] *  maxprdir_l[i];
      	    }
            maxprdir_out[0] = dtemp_14;
      	    maxprdir_out[1] = dtemp_15;
      	    maxprdir_out[2] = dtemp_16;
   	}
   	else 
	{
      	    prcurvs_out[0] = 0.0;
      	    prcurvs_out[1] = 0.0;
      	    for (i=0; i<=2; i+=1) 
         	maxprdir_out[i] = ((double) maxprdir_1[i]);
   	}
   	return;
}	/* end polyfit3d_lhf */

/*************************************************************
 *
 * FUNCTION: eval_curvature_lhf_surf
 *
 *EVAL_CURVATURE_LHF_SURF Compute principal curvature, principal direction 
 *and pseudo-inverse.
 * [CURVS,DIR,JINV] = EVAL_CURVATURE_LHF_SURF(GRAD,H) Computes principal 
 * curvature in 2x1 CURVS, principal direction of maximum curvature in 3x2 
 * DIR, and pseudo-inverse of J in 2x3 JINV.  Input arguments are the
 * gradient of the height function in 2x1 GRAD, and the Hessian of the
 * height function in 2x2 H with a local coordinate frame.
 * See also EVAL_CURVATURE_LHFINV_SURF, EVAL_CURVATURE_PARA_SURF
 *************************************************************/

LOCAL void  eval_curvature_lhf_surf(
   	double curvs_out[2], 
   	double dir_out[3], 
   	double grad[2], 
   	double H[2][2])
{
   	double grad_sqnorm, grad_norm, ell, ell2, ell3;
   	double c, s, kH2, tmp, dtemp_0;
   	double v[2], W1[2];
   	double W[2][2];
   	double U[3][2];
   	double d1[2];
   	int i;

   	grad_sqnorm =  grad[0]*grad[0] + grad[1]*grad[1];
   	grad_norm = sqrt(grad_sqnorm);
   	/* Compute key parameters */

   	ell = sqrt(1.0 + grad_sqnorm);
   	ell2 = 1.0 + grad_sqnorm;
   	ell3 = ell*(1.0 + grad_sqnorm);
   	if (grad_norm == 0.0) 
	{
      	    c = 1.0;
      	    s = 0.0;
   	}
   	else 
	{
      	    c =  grad[0]/grad_norm;
      	    s =  grad[1]/grad_norm;
   	}
   	/* Compute mean curvature and Gaussian curvature */
   	/* kH2 = (H(1,1)+H(2,2))/ell - grad*H*grad'/ell3; */
   	/* kG =  (H(1,1)*H(2,2)-H(1,2)^2)/ell2^2; */
   	/* Solve quadratic equation to compute principal curvatures */

   	v[0] = c*H[0][0] + s*H[0][1];
   	v[1] = c*H[0][1] + s*H[1][1];
   	W1[0] = (v[0]*c +  v[1]*s)/ell3;
   	W1[1] = (v[0]*(-s) +  v[1]*c)/ell2;
   	W[0][0] = W1[0];
   	W[0][1] = W1[1];
   	W[1][0] = W1[1];
   	W[1][1] = ((c*H[0][1] - s*H[0][0])*(-s) + 
			(c*H[1][1] - s*H[0][1])*c)/ell;
   	/* Lambda = eig(W); */

   	kH2 =  W[0][0] +  W[1][1];
   	tmp = sqrt(( W[0][0] -  W[1][1])*( W[0][0] -  W[1][1]) + 
			4.0*W[0][1]*W[0][1]);
   	if (kH2 > 0.0) 
	{
      	    curvs_out[0] = (kH2 + tmp)/2.0;
      	    curvs_out[1] = (kH2 - tmp)/2.0;
   	}
   	else 
	{
      	    curvs_out[0] = (kH2 - tmp)/2.0;
      	    curvs_out[1] = (kH2 + tmp)/2.0;
   	}
   /* Compute principal directions, first with basis of left  */
   /* singular vectors of Jacobian */
   /* Compute principal directions in 3-D space */

   	U[0][0] = c/ell;
   	U[0][1] = -s;
   	U[1][0] = s/ell;
   	U[1][1] = c;
   	U[2][0] = grad_norm/ell;
   	U[2][1] = 0.0;
   	if (curvs_out[0] ==  curvs_out[1]) 
      	    for (i=0; i<=2; i+=1) 
         	dir_out[i] = U[i][0];
   	else 
	{
      	    if (fabs(W[0][0] -  curvs_out[1]) > fabs(W[0][0] -  curvs_out[0])) 
	    {
         	d1[0] =  W[0][0] -  curvs_out[1];
         	d1[1] = W[0][1];
      	    }
      	    else 
	    {
         	d1[0] = - W[0][1];
         	d1[1] =  W[0][0] -  curvs_out[0];
      	    }
      	    dtemp_0 = sqrt( d1[0] *  d1[0] +  d1[1] *  d1[1]);
      	    d1[0] =  d1[0] / dtemp_0;
      	    d1[1] =  d1[1] / dtemp_0;
      	    dir_out[0] =  U[0][0] *  d1[0] +  U[0][1] *  d1[1];
      	    dir_out[1] =  U[1][0] *  d1[0] +  U[1][1] *  d1[1];
      	    dir_out[2] =  U[2][0] *  d1[0] +  U[2][1] *  d1[1];
   	}
   	return;
}	/* end eval_curvature_lhf_surf */

/*************************************************************
 *
 * FUNCTION: eval_vander_bivar
 *
 *EVAL_VANDER_BIVAR Evaluate generalized Vandermonde matrix.
 * [BS,DEGREE] = EVAL_VANDER_BIVAR(US,BS,DEGREE,WS,STRIP) Evaluates
 * generalized Vandermonde matrix V, and solve V\BS. It supports up to
 * degree 6.
 * See also EVAL_VANDER_UNIVAR
 *************************************************************/

LOCAL int eval_vander_bivar(
   	double *us, 
   	int us_dim1, 
   	double *bs, 
   	int *bs_dim1, 
   	int degree, 
   	double *ws, 
   	int ws_dim1, 
   	int strip)
{
   	static double V[28672];
   	int V_dim1, V_dim2;
   	static double ws1[1024];
   	static double ts[1024];
   	int ncols;
   	static double v[1024];
   	int degree_1_out, npnts, jj, rnk, ncols_sub;
   	double t2, t, vnrm2, s, w;
   	int ncoeff, itemp_0, itemp_1, i, j;
   	static int ncoeffs[6] = {3,6,10,15,21,28};
   	static double weights[10] = {1.,1.0,1.0,2.0,1.0,2.0,6.0,2.0,2.0,6.0};
   	double dtemp_0, dtemp_1, dtemp_2, dtemp_3, dtemp_4;
   	int i3;

   	degree_1_out = degree;
   	/* Determine degree of fitting */

   	npnts = us_dim1;
   	/* Determine degree of polynomial */

   	ncols =  ncoeffs[degree - 1] - strip;
   	while ((ncoeffs[degree_1_out - 1] - strip > npnts)
		&&(degree_1_out > 1)) 
	{
      	    degree_1_out = degree_1_out - 1;
      	    ncols =  ncoeffs[degree_1_out - 1] - strip;
   	}
   	/*% Construct matrix */

   	V_dim1 = max(npnts, 0);
   	ct_set_max(V_dim2, ncols, 0);
   	memset(V, 0, npnts * ncols * 8);
   	/* Allocate the maximum size */

   	for (i=0; i<=npnts - 1; i+=1) 
	{
      	    V[i * V_dim2] = 1.0;
      	    jj = 2 - strip;
      	    V[ i * V_dim2 - 1 + jj] = us[i << 1];
      	    jj = 1 + jj;
      	    V[ i * V_dim2 - 1 + jj] = us[1 + (i << 1)];
      	    for (j=2; j<=degree_1_out; j+=1) 
	    {
         	jj = 1 + jj;
         	V[i*V_dim2-1+jj] =  V[i*V_dim2-1+jj-j]*us[i << 1];
         	for (i3=0; i3<=j - 1; i3+=1) 
		{
            	    jj = 1 + jj;
            	    V[i*V_dim2-1+jj] = V[i*V_dim2-2+jj-j]*us[1+(i << 1)];
         	}
      	    }
   	}
   	/*% Scale rows to assign different weights to different points */

   	if (degree_1_out > 1) {
      	/* Scale weights to be inversely proportional to distance */

      	dtemp_1 = 0.0;
      	for (i=0; i<=us_dim1 - 1; i+=1) 
	{
            dtemp_4 = us[i << 1]*us[i << 1] + us[1+(i << 1)]*us[1+(i << 1)];
            dtemp_1 = dtemp_1 + dtemp_4;
            ws1[i] = dtemp_4;
      	}
      	for (i=0; i<=us_dim1 - 1; i+=1) 
	{
             ws1[i] =  ws1[i] + 0.01 * (dtemp_1 / (((double)npnts)));
        }
        if (degree_1_out < 4) 
	{
            for (i=0; i<=npnts - 1; i+=1) 
	    {
            	if ( ws1[i] != 0.0) 
		{
               	    ws1[i] = ws[i]/sqrt(ws1[i]);
            	}
            	else 
		{
               	    ws1[i] = ws[i];
            	}
	    }
      	}
      	else 
	{
         for (i=0; i<=npnts - 1; i+=1) {
            if ( ws1[i] != 0.0) {
               ws1[i] =  ws[i] /  ws1[i];
            }
            else {
               ws1[i] = ws[i];
            }
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
         V[ i * V_dim2 + j] =  V[ i * V_dim2 + j] *  ws1[i];
      }
      bs[i] =  bs[i] *  ws1[i];
   }
   /*% Scale columns to reduce condition number */

   memset(ts, 0, ncols * 8);
   for (i=0; i<=ncols - 1; i+=1) {
      /*NORM2_VEC Computes the 2-norm of a vector. */
      /* NORM2_VEC(V) Computes the 2-norm of column vector V, */
      /*       taking care not to cause unnecessary overflow. */
      /* See also SQNORM2_VEC */

      w = 0.0;
      for (j=0; j<=V_dim1 - 1; j+=1) {
         dtemp_2 = fabs(V[ j * V_dim2 + i]);
         w = max(dtemp_2, w);
         v[j] = V[ j * V_dim2 + i];
      }
      s = 0.0;
      if (w == 0.0) {
         /* W can be zero for max(0,nan,...) */
         /* adding all three entries together will make sure */
         /* NaN will not disappear. */

         for (j=0; j<=V_dim1 - 1; j+=1) {
            s = s +  v[j];
         }
      }
      else {
         for (j=0; j<=V_dim1 - 1; j+=1) {
            dtemp_0 =  v[j] / w;
            s = s + dtemp_0 * dtemp_0;
         }
         s = w *  sqrt(s);
      }
      dtemp_3 = s;
      if (dtemp_3 == 0.0) {
         dtemp_3 = 1.0;
      }
      else {
         for (j=0; j<=V_dim1 - 1; j+=1) {
            V[ j * V_dim2 + i] =  V[ j * V_dim2 + i] / dtemp_3;
         }
      }
      ts[i] = dtemp_3;
   }
   /*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
   /* Perform Householder QR factorization to compute */
   /* [Q, V(1:n,1:n)] = qr(V(1:m,1:n),0); bs = Q'*bs; rnk=n; */

   rnk = ncols;
   for (i=0; i<=ncols - 1; i+=1) {
      itemp_1 = npnts - i;
      if (npnts < i) {
         itemp_1 = 0;
      }
      for (j=0; j<= npnts - 1 - i; j+=1) {
         v[j] = V[ (i + j) * V_dim2 + i];
      }
      /* We don't need to worry about overflow, since V has been rescaled. */

      t2 = 0.0;
      for (j=0; j<=itemp_1 - 1; j+=1) {
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
         for (j=0; j<=itemp_1 - 1; j+=1) {
            v[j] =  v[j] / vnrm2;
         }
      }
      /* Optimized version for */
      /* V(k:npnts,k:ncols) = V(k:npnts,k:ncols) - 2*v*(v'*V(k:npnts,k:ncols)); */

      for (j=1 + i; j<=ncols; j+=1) {
         t2 = 0.0;
         for (i3=0; i3<=itemp_1 - 1; i3+=1) {
            t2 = t2 +  v[i3] *  V[ (i + i3) * V_dim2 - 1 + j];
         }
         t2 = t2 + t2;
         for (i3=0; i3<=itemp_1 - 1; i3+=1) {
            V[ (i + i3) * V_dim2 - 1 + j] =  V[ (i + i3) * V_dim2 - 1 + j] - t2 *  v[i3];
         }
      }
      /* Estimate rank of matrix */

      if (( fabs(V[ i * V_dim2 + i]) < 0.001)&&(rnk == ncols)) {
         rnk = i;
         if (rnk > 3) {
            break;
         }
      }
      /* Optimized version for */
      /* bs(k:npnts,:) = bs(k:npnts,:) - 2*v*(v'*bs(k:npnts,:)); */

      t2 = 0.0;
      for (j=0; j<=itemp_1 - 1; j+=1) {
         t2 = t2 +  v[j] *  bs[i + j];
      }
      t2 = t2 + t2;
      for (j=0; j<=itemp_1 - 1; j+=1) {
         bs[i + j] =  bs[i + j] - t2 *  v[j];
      }
   }
   /*%%%%%%%%%%%%%%%%%%%%% */

   ncols_sub = ncols;
   while ((rnk < ncols_sub)&&(degree_1_out > 1)) {
      degree_1_out = degree_1_out - 1;
      ncols_sub =  ncoeffs[degree_1_out - 1] - strip;
   }
   /*%%%%%%%%%%%%%%%%%%%%% */
   /* Perform backward substitution to compute */
   /* bs = triu(R(1:ncols,1:ncols))\bs; */

   for (i=ncols_sub; i>=1; i+=-1) {
      for (j=1 + i; j<=ncols_sub; j+=1) {
         bs[i - 1] =  bs[i - 1] -  V[ (i - 1) * V_dim2 - 1 + j] *  bs[j - 1];
      }
      if ( V[ (i - 1) * V_dim2 - 1 + i] == 0.0) {
         /* Singular matrix */

         /*__CATALYTIC_warning("Matrix is singular.", 19);*/
	if (debugging("wlsp"))
	    printf("Matrix is singular.\n");
         /*#ok<WNTAG> */

         bs[i - 1] = 0.0;
      }
      else {
         bs[i - 1] =  bs[i - 1] /  V[ (i - 1) * V_dim2 - 1 + i];
      }
   }
   /*%%%%%%%%%%%%%%%%%%%%% */
   /* Force weights to be double precision. */

   ncoeff = 6 - strip;
   itemp_0 = min(ncols_sub, ncoeff);
   for (i=0; i<=itemp_0 - 1; i+=1) {
      bs[i] =  bs[i] *  weights[strip + i] /  ts[i];
   }
   return degree_1_out;
}	/* end eval_vander_bivar */

/*This function is particularly for normal of first order */
EXPORT  boolean  WLSP_compute_normal3d0(
        FT_POINT              *p,
        HYPER_SURF_ELEMENT *hse,
        HYPER_SURF         *hs)
{
        TRI *tri,**tris;
        FT_POINT *pc,*pp; /*current point */
        int np1,np2,i,j,nt,num;
        double nor_f[3],norm=0; /*normal of tri*/
        const double  *fnor;
        double t1[3],t2[3];
        INTERFACE *intfc = p->hs->interface;
        TRI **ptris,*t,*tris1[20];

	tri = Tri_of_hse(p->hse);
	nt = set_tri_list_around_point(p,tri,&Tri_list_at_vertex.tris,intfc);
	/* nt1 = set_tri_list_around_point(p,tri,&ptris,intfc);*/
        for(i = 0; i < 3; i++)
        {
            nor_f[i] = 0;
        }

        for(i = 0; i < nt; i++)
        {
            t = Tri_list_at_vertex.tris[i];
            fnor = Tri_normal(t);
            for(j = 0; j < 3; j++)
            {
                nor_f[j] = nor_f[j] + fnor[j];
            }
        }
        for(i = 0; i < 3; i++)
        {
            norm = norm + nor_f[i]*nor_f[i];
        }
        norm = sqrt(norm);
        for(i = 0; i < 3; i++)
        {
            p->_nor0[i] = nor_f[i]/norm;
        }
	return YES;
}	/* end WLSP_compute_normal3d0 */

/*This function is particularly for normal of second order */
EXPORT boolean WLSP_compute_normal3d(
        FT_POINT              *p,
        HYPER_SURF_ELEMENT *hse,
        HYPER_SURF         *hs)
{
        TRI **tris;
        FT_POINT *pts_ring1[MAX_RING2_PTS];
        FT_POINT *pts_ring2[MAX_RING2_PTS];
        FT_POINT *pc,*pt;
        int np1,np2;
        int i,j;

        double tang[6],nrm1st[3],nrm[3],nrm_t[3],nrm_v[50][3];
        double xs_v[50][3],xss_v[50][2],nrm_vs[3];
        double PT[3][3],xss_r[50][2];
        double ws_row[50],w;

        int degree = 2;
        double temp,ncoeffs[]={3,6,10,15,21,28};
        double bs[50][2];
        double x_t[3],coeffs[20];
        double cs[10][2]={0.0},cond;
        double curvature;
        int xs_dim, nrm_coor_dim;
        double xs[60][3],prcurvs[2],maxprdir[3];
        int deg;
        double nrm_coor[60][3];       
 
        pc = p; /*point to current point */
        PointArrayRing2(pc,hse,hs,&np1,&np2,pts_ring1,pts_ring2);

        for(i = 0; i < 3; i++)
        {
            nrm1st[i] = p->_nor0[i];
        }

        for(i = 0; i <3; i++)
            nrm1st[i] = -nrm1st[i];

        xs_dim = np1 + np2 + 1;
        nrm_coor_dim = np1 + np2 + 1;
        
        xs[0][0] = Coords(pc)[0];
        xs[0][1] = Coords(pc)[1];
        xs[0][2] = Coords(pc)[2];

        nrm_coor[0][0] = pc->_nor0[0];
        nrm_coor[0][1] = pc->_nor0[1];
        nrm_coor[0][2] = pc->_nor0[2];

        for(i = 0; i < np1; i++)
        {
            xs[i+1][0] = Coords(pts_ring1[i])[0];
            xs[i+1][1] = Coords(pts_ring1[i])[1];
            xs[i+1][2] = Coords(pts_ring1[i])[2];
            nrm_coor[i+1][0] = pts_ring1[i]->_nor0[0];
            nrm_coor[i+1][1] = pts_ring1[i]->_nor0[1];
            nrm_coor[i+1][2] = pts_ring1[i]->_nor0[2];
        
        }

        for(i = 0; i < np2; i++)
        {
            xs[i+np1+1][0] = Coords(pts_ring2[i])[0];
            xs[i+np1+1][1] = Coords(pts_ring2[i])[1];
            xs[i+np1+1][2] = Coords(pts_ring2[i])[2];
            nrm_coor[i+np1+1][0] = pts_ring2[i]->_nor0[0];
            nrm_coor[i+np1+1][1] = pts_ring2[i]->_nor0[1];
            nrm_coor[i+np1+1][2] = pts_ring2[i]->_nor0[2];            
        }

        polyfit3d_lhf(nrm,&deg,prcurvs,maxprdir,*xs,xs_dim,*nrm_coor,
			nrm_coor_dim,2,1);


        for(i = 0; i < 3; i++)
        {
            pc->_nor[i] = nrm[i];
        }
        pc->curvature = -(prcurvs[0]+prcurvs[1])/2;
	return YES;
}	/* end WLSP_compute_normal3d */


/************** Least square 2D *************/

/*******************************************************************
 * Function for the curvature computation of seven points with LSQ *
 *******************************************************************/
/* Prototypes for local functions. */
/*************************************************************
 *
 *    FUNCTION: safeqr
 *
 *    Perform Householder QR factorization to compute
 *    [Q, A(1:n,1:n)] = qr(A(1:m,1:n),0); b = Q'*b; rnk=n;
 **************************************************************/
LOCAL int safeqr(
        double *A,
        int *A_dim1,
        int *A_dim2,
        double *b,
        int *b_dim1,
        double tol)
{
        double v[13];
        int v_dim1;
        static double dtemp_0[91];
        int dtemp_0_dim1;
        double dtemp_1[13];
        int dtemp_1_dim1;
        int rnk_out, m, n, itemp_0, itemp_1;
        double vnrm2, dtemp_2, dtemp_3, dtemp_4, dtemp_5;
        int itemp_2, itemp_3, itemp_4, itemp_5, itemp_6;
        int itemp_7, itemp_8, itemp_9, itemp_10, itemp_11;
        int itemp_12, itemp_13, itemp_14, itemp_15, itemp_16;
        int itemp_17, itemp_18, itemp_19, itemp_20, itemp_21;
        int itemp_22, itemp_23, itemp_24, itemp_25, itemp_26;
        int itemp_27, i, j, i3;

        n = *A_dim2;
        m = *A_dim1;
        rnk_out = n;
        itemp_0 = n;
        for(i = 0; i <= itemp_0 - 1; i += 1)
        {
            ct_set_max(v_dim1, m - i, 0);
            for(j = 0; j <= m - 1 - i; j += 1)
            {
                v[j] = A[(i + j)*(*A_dim2) + i];
            }
            if(v[0] >= 0.0)
            {
                ct_set_max(itemp_11, m - i, 0);
                if(v_dim1 > 1)
                {
                    itemp_1 = itemp_11;
                }
                else
                {
                    itemp_1 = 1;
                }
                ct_set_max(itemp_12, m - i, 0);
                if(v_dim1 > 1)
                {
                    itemp_2 = itemp_12;
                }
                else
                {
                    itemp_2 = 1;
                }
                dtemp_3 = 0.0;

                for(j = 0; j <= itemp_1 - 1; j += 1)
                {
                    dtemp_3 = dtemp_3 + v[j]*v[j];
                }
                v[0] =  v[0] + sqrt(dtemp_3);
            }
            else
            {
                ct_set_max(itemp_13, m - i, 0);
                if (v_dim1 > 1)
                {
                    itemp_3 = itemp_13;
                }
                else
                {
                    itemp_3 = 1;
                }

                ct_set_max(itemp_14, m - i, 0);
                if (v_dim1 > 1)
                {
                    itemp_4 = itemp_14;
                }
                else
                {
                    itemp_4 = 1;
                }
                dtemp_4 = 0.0;
                for(j = 0; j <= itemp_3 - 1; j += 1)
                {
                    dtemp_4 = dtemp_4 + v[j]*v[j];
                }
                v[0] = v[0] - sqrt(dtemp_4);
            }
            ct_set_max(itemp_15, m - i, 0);
            if(v_dim1 > 1)
            {
                itemp_5 = itemp_15;
            }
            else
            {
                itemp_5 = 1;
            }
            ct_set_max(itemp_16, m - i, 0);
            if(v_dim1 > 1)
            {
                itemp_6 = itemp_16;
            }
            else
            {
                itemp_6 = 1;
            }
            dtemp_5 = 0.0;
            for(j = 0; j <= itemp_5 - 1; j += 1)
            {
                dtemp_5 = dtemp_5 + v[j]*v[j];
            }
            vnrm2 = sqrt(dtemp_5);
            if(vnrm2 > 0.0)
            {
                for(j = 0; j <= v_dim1 - 1; j += 1)
                {
                    v[j] = v[j]/vnrm2;
                }
            }
            for(j = 1 + i; j <= n; j += 1)
            {
                ct_set_max(itemp_17, m - i, 0);
                if(v_dim1 > 1)
                {
                    itemp_7 = itemp_17;
                }
                else
                {
                    itemp_7 = 1;
                }
                ct_set_max(itemp_18, m - i, 0);
                dtemp_2 = 0.0;
                for(i3 = 0; i3 <= itemp_7 - 1; i3 += 1)
                {
                    dtemp_2 = dtemp_2 + v[i3] * A[(i + i3)*(*A_dim2) - 1 + j];
                }
                ct_set_max(itemp_19, m - i, 0);
                ct_set_max(itemp_20, m - i, 0);
                if(v_dim1 > 1)
                {
                    itemp_8 = itemp_20;
                }
                else
                {
                    itemp_8 = 1;
                }
                if(itemp_19 == 1)
                {
                    dtemp_0_dim1 = itemp_8;
                    for(i3 = 0; i3 <= dtemp_0_dim1 - 1; i3 += 1)
                    {
                        dtemp_0[i3] = A[i*(*A_dim2) - 1 + j]
                                            - 2.0*v[i3]*dtemp_2;
                    }
                }
                else
                {
                    dtemp_0_dim1 = itemp_19;
                    if (itemp_8 == 1)
                    {
                        for(i3 = 0; i3 <= dtemp_0_dim1 - 1; i3 += 1)
                        {
                            dtemp_0[i3] = A[(i + i3)*(*A_dim2) - 1 + j]
                                                 - 2.0*v[0]*dtemp_2;
                        }
                    }
                    else
                    {
                        for(i3 = 0; i3 <= dtemp_0_dim1 - 1; i3 += 1)
                        {
                            dtemp_0[i3] = A[(i + i3)*(*A_dim2) - 1 + j]
                                                 - 2.0*v[i3] *dtemp_2;
                        }
                    }
                }
                ct_set_max(itemp_21, m - i, 0);
                itemp_27 = 0;
                if(dtemp_0_dim1 == 1)
                {
                    for(i3=0; i3 <= itemp_21 - 1; i3 += 1)
                    {
                        A[(i + i3)*(*A_dim2) - 1 + j] = dtemp_0[0];
                    }
                }
                else
                {
                    for(i3 = 0; i3 <= itemp_21 - 1; i3 += 1)
                    {
                        A[(i + i3)*(*A_dim2) - 1 + j] = dtemp_0[itemp_27];
                        itemp_27 = 1 + itemp_27;
                    }
                }
            }
            /* Estimate rank of matrix */
            if((fabs(A[i*(*A_dim2) + i]) < tol)&&(rnk_out == n))
            {
                rnk_out = i;
                if(rnk_out > 3)
                {
                    return rnk_out;
                }
            }
            ct_set_max(itemp_22, m - i, 0);
            if(v_dim1 > 1)
            {
                itemp_9 = itemp_22;
            }
            else
            {
                itemp_9 = 1;
            }
            ct_set_max(itemp_23, m - i, 0);
            dtemp_2 = 0.0;
            for(j = 0; j <= itemp_9 - 1; j += 1)
            {
                dtemp_2 = dtemp_2 + v[j]*b[i + j];
            }
            ct_set_max(itemp_24, m - i, 0);
            ct_set_max(itemp_25, m - i, 0);
            if (v_dim1 > 1)
            {
                itemp_10 = itemp_25;
            }
            else
            {
                itemp_10 = 1;
            }
            if (itemp_24 == 1)
            {
                dtemp_1_dim1 = itemp_10;
                for (j = 0; j <= dtemp_1_dim1 - 1; j += 1)
                {
                    dtemp_1[j] = b[i] - 2.0*v[j]*dtemp_2;
                }
            }
            else
            {
                dtemp_1_dim1 = itemp_24;
                if (itemp_10 == 1)
                {
                    for (j = 0; j <= dtemp_1_dim1 - 1; j += 1)
                    {
                        dtemp_1[j] =  b[i + j] - 2.0*v[0]*dtemp_2;
                    }
                }
                else
                {
                    for (j = 0; j <= dtemp_1_dim1 - 1; j += 1)
                    {
                        dtemp_1[j] =  b[i + j] - 2.0*v[j]*dtemp_2;
                    }
                }
            }
            ct_set_max(itemp_26, m - i, 0);
            itemp_27 = 0;
            if (dtemp_1_dim1 == 1)
            {
                for (j = 0; j <= itemp_26 - 1; j += 1)
                {
                    b[i + j] = dtemp_1[0];
                }
            }
            else
            {
                for (j = 0; j <= itemp_26 - 1; j += 1)
                {
                    b[i + j] = dtemp_1[itemp_27];
                    itemp_27 = 1 + itemp_27;
                }
            }
        }
        return rnk_out;
}	/* end safeqr */

EXPORT	void PointAndFirstRingTris(
	FT_POINT *p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF *hs,
	int *nt,
	TRI **tris)
{
	INTERFACE *intfc = hs->interface;
	int i;
	*nt = set_tri_list_around_point(p,Tri_of_hse(hse),
				&Tri_list_at_vertex.tris,intfc);
	for (i = 0; i < *nt; ++i)
	    tris[i] = Tri_list_at_vertex.tris[i];
}	/* end PointAndFirstRingTri */

EXPORT	void	TriAndFirstRing(
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF *hs,
	int *nt,
	TRI **tris)
{
	INTERFACE *intfc = hs->interface;
	TRI *t,*tri = Tri_of_hse(hse);
	int i,j,n;
	FT_POINT *pp;
	boolean tri_in_list;

	*nt = 0;
	tris[(*nt)++] = tri;
	for (i = 0; i < 3; ++i)
	{
	    pp = Point_of_tri(tri)[i];
	    n = set_tri_list_around_point(pp,tri,&Tri_list_at_vertex.tris,
					intfc);
	    for (j = 0; j < n; ++j)
	    {
	    	t = Tri_list_at_vertex.tris[j];
		tri_in_list = pointer_in_list((POINTER)t,*nt,(POINTER*)tris);
		if (!tri_in_list) 
		{
		    tris[(*nt)++] = t;
		}
	    }
	}
}	/* end TriAndFirstRing */

EXPORT	void	TriAndFirstTwoRings(
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF *hs,
	int *nt,
	TRI **tris)
{
	INTERFACE *intfc = hs->interface;
	TRI *t,*tri = Tri_of_hse(hse);
	int i,j,k,n,np1;
	FT_POINT *p0,*p1;
	boolean tri_in_list;
	boolean p_in_list;
	FT_POINT *pts[30];

	*nt = np1 = 0;
	tris[(*nt)++] = tri;
	for (i = 0; i < 3; ++i)
	{
	    p0 = Point_of_tri(tri)[i];
	    n = set_tri_list_around_point(p0,tri,&Tri_list_at_vertex.tris,
					intfc);
	    for (j = 0; j < n; ++j)
	    {
	    	t = Tri_list_at_vertex.tris[j];
		tri_in_list = pointer_in_list((POINTER)t,*nt,(POINTER*)tris);
		if (!tri_in_list) 
		{
		    tris[(*nt)++] = t;
		    for (k = 0; k < 3; ++k)
		    {
			p1 = Point_of_tri(t)[k];
			p_in_list = pointer_in_list((POINTER)p1,np1,
					(POINTER*)pts);
			if (!p_in_list)
			    pts[np1++] = p1;
		    }
		}
	    }
	}
	for (i = 0; i < np1; ++i)
	{
	    p1 = pts[i];
	    n = set_tri_list_around_point(p1,tri,&Tri_list_at_vertex.tris,
					intfc);
	    for (j = 0; j < n; ++j)
	    {
	    	t = Tri_list_at_vertex.tris[j];
		tri_in_list = pointer_in_list((POINTER)t,*nt,(POINTER*)tris);
		if (!tri_in_list) 
		{
		    tris[(*nt)++] = t;
		}
	    }
	}
}	/* end TriAndFirstTwoRings */
	
/*	
	Create a list of bonds which are prev and next neighbours of the
	bond of hse. The order argument is the number of bonds on each
	side.
*/

EXPORT	void	BondAndNeighbors(
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF *hs,
	int *nb,
	BOND **bonds,
	int order)
{
	BOND *pb,*b = Bond_of_hse(hse);
	CURVE *c = Curve_of_hs(hs);
	boolean b_in_list;
	int i;

	*nb = 0;
	bonds[(*nb)++] = b;
	pb = b;
	for (i = 0; i < order; ++i)
	{
	    pb = Prev_bond(pb,c);
	    if (pb == NULL) break;
	    b_in_list = pointer_in_list((POINTER)pb,*nb,(POINTER*)bonds);
	    if (!b_in_list) bonds[(*nb)++] = pb;
	}
	pb = b;
	for (i = 0; i < order; ++i)
	{
	    pb = Next_bond(pb,c);
	    if (pb == NULL) break;
	    b_in_list = pointer_in_list((POINTER)pb,*nb,(POINTER*)bonds);
	    if (!b_in_list) bonds[(*nb)++] = pb;
	}
}	/* end BondAndNeighbors */

EXPORT   void   reset_surface_points(
        SURFACE         *s)
{
        TRI *t;

        for (t = first_tri(s); !at_end_of_tri_list(t,s); t = t->next)
        {
            sorted(Point_of_tri(t)[0]) = NO;
            sorted(Point_of_tri(t)[1]) = NO;
            sorted(Point_of_tri(t)[2]) = NO;
        }
}               /*end reset_surface_points*/
