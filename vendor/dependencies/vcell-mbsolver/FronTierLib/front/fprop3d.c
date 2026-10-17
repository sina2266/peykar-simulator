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
*				fprop3d.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*/


#define DEBUG_STRING	"tprop3d"

#include <front/fdecs.h>              /* includes int.h, table.h */

	/* LOCAL Function Declarations */

LOCAL	boolean	set_tangent_space_projection(Front*,Tparams*,TRI**,int);
LOCAL	boolean	tn_common_vertex(int,int,TRI*,const TN*,boolean);
LOCAL 	boolean 	update_state_in_tan_direction(Front*,const Tparams*,FT_POINT*,
                                              double);
LOCAL	void	copy_stencil_in_direction(Tan_stencil*,int,Front*);
LOCAL	void	set_tn_on_tri_side(TN*,TRI*,int);
LOCAL	void	set_tn_on_tri_vertex(TN*,TRI*,int);
LOCAL	int	set_posn_and_states(Tan_stencil*,TRI*,HYPER_SURF*,double*,
				    int,int,Front*);
LOCAL	int	set_rest_of_states(Tan_stencil*,TRI*,HYPER_SURF*,double*,
				    int,int,Front*);
LOCAL	boolean    is_forward_pn(double*,double*,double*);
LOCAL	boolean	check_record_tri(TRI*,TRI**,int*);

#define copy_posn(p1,p2)	(p1)[0] = (p2)[0]; 		\
				(p1)[1] = (p2)[1]; 		\
				(p1)[2] = (p2)[2]


/*ARGSUSED*/

EXPORT	boolean f_tan_point_propagate(
	Front              *fr,
	FT_POINT              *p,
	FT_POINT              *newp,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF         *hs,
	double dt,
	int   dir)
{
	double kappa;
	static 	Tparams tp[2];
	RECT_GRID *gr=computational_grid(fr->interf);
	double  *h = gr->h;

	/*tol for plane_side_intersection. */
	set_tol_for_tri_sect(1.0e-6*min3(h[0], h[1], h[2]));

	if (Boundary_point(p)) 
	    return YES;
	
	tp[0].dt = tp[1].dt = dt;
	if (!set_up_tangent_params(fr,p,hse,hs,tp))
	    return NO;
	
        kappa = p->curvature;
	if (dir == 0)
	{
	    if (!update_state_in_tan_direction(fr,tp,newp,kappa)) 
	    {
	    	screen("Tangential fails update_state in direction 0\n");
	    	return NO;
	    }
            if (fr->parab == YES)
                npt_parab_tan_solver3d(fr,tp,tp+1,newp);
	}
	else
	{
	    if (!update_state_in_tan_direction(fr,tp+1,newp,kappa)) 
	    {
	    	screen("Tangential fails update_state in direction 1\n");
	    	return NO;
	    }
            if (fr->parab == YES)
                npt_parab_tan_solver3d(fr,tp+1,tp,newp);
	}
	return YES;

}		/*end f_tan_point_propagate*/

EXPORT	boolean set_up_tangent_params(
	Front			*fr,
	FT_POINT			*p,
	HYPER_SURF_ELEMENT	*hse,
	HYPER_SURF		*hs,
	Tparams			*tp)
{
	INTERFACE *intfc = hs->interface;
	static TRI	  *tris[100];
	TRI 	  **ptris;
	double	  *t1 = tp[0].tan;	/* double[3] */
	double	  *t2 = tp[1].tan;	/* double[3] */
	double	  nor[3];
	boolean      folded;
	int	  i,nt;

	normal(p,hse,hs,nor,fr);
	nt = set_tri_list_around_point(p,Tri_of_hse(hse),&ptris,intfc);
	for (i = 0; i < nt; ++i)
	    tris[i] = ptris[i];

	tp[0].hs  = tp[1].hs  = hs;
	tp[0].p   = tp[1].p   = p;
	tp[0].nor = tp[1].nor = nor;

	principal_tangent(fr,p,hse,hs,nor,t1);
	Cross3d(nor,t1,t2);
	folded = set_tangent_space_projection(fr,tp,tris,nt);

	if (folded ||
	     ( ((tp[0].tnr.tri == NULL) && (tp[0].tnl.tri == NULL)) ||
	       ((tp[1].tnr.tri == NULL) && (tp[1].tnl.tri == NULL)) ) )
	{
	    /* A cusped geometry has been detected, recompute the normal   */
	    /* vector so that the local triangles project in a regular way */
	    /* onto the tangent space                                      */
	    omit_vertex_in_plane_fit();
	    plane_fit_normal3d(p,hse,hs,nor);
	    nt = set_tri_list_around_point(p,Tri_of_hse(hse),&ptris,intfc);
	    for (i = 0; i < nt; ++i)
		tris[i] = ptris[i];
	    principal_tangent(fr,p,hse,hs,nor,t1);
	    Cross3d(nor,t1,t2);
	    folded = set_tangent_space_projection(fr,tp,tris,nt);
	}

	if (folded ||
	     ((tp[0].tnr.tri == NULL) && (tp[0].tnl.tri == NULL)) ||
	     ((tp[1].tnr.tri == NULL) && (tp[1].tnl.tri == NULL)) )
	{
	    if (debugging("tparams"))
	    {
	    	(void) printf("WARNING in set_up_tangent_params(), "
			  "didn't set tris, nt = %d\n",nt);
	    	(void) printf("p = %llu\n",point_number(p));
	    	(void) printf("Tri_of_hse(hse)\n");
	    	print_tri(Tri_of_hse(hse),intfc);
		clean_up(ERROR);
	    }
	    return NO;
	}
	else
	    return YES;
}		/* end set_up_tangent_params */

LOCAL	boolean set_tangent_space_projection(
	Front		   *fr,
	Tparams		   *tp,
	TRI	           **tris,
	int                num_tris)
{
	FT_POINT *p;
    	boolean  folded;
	double *t[2], *plane[2];
	double *p0;
	double *h = fr->rect_grid->h;
	double pc[3], pdir[3];
	int   iv, iside, ivtx;
	int   i, j;


	p = tp[0].p;
	t[0] = tp[0].tan;	/* double[3] */
	t[1] = tp[1].tan;	/* double[3] */
	plane[0] = tp[0].plane;	/* double[4] */
	plane[1] = tp[1].plane;	/* double[4] */
	for (i = 0; i < 3; ++i)
	{
	    plane[0][i] =  t[1][i];
	    plane[1][i] = -t[0][i];
	}

	p0 = Coords(p);
	for (i = 0; i < 2; ++i)
	{
	  plane[i][3] = plane[i][0]*p0[0]+plane[i][1]*p0[1]+plane[i][2]*p0[2];
	  tp[i].ds = sqr(t[i][0]/h[0]) + sqr(t[i][1]/h[1]) + sqr(t[i][2]/h[2]);
	  tp[i].ds = 1/sqrt(tp[i].ds);
	  zero_scalar(&tp[i].tnl,sizeof(TN));
	  zero_scalar(&tp[i].tnr,sizeof(TN));
	}
	folded = NO;
	for (i = 0; i < num_tris; ++i)
	{
	  iv = Vertex_of_point(tris[i],p);
	  iside = Next_m3(iv);
	  for (j = 0; j < 2; ++j)
	  {
	    if (plane_side_intersection(plane[j],tris[i],iside,pc,&ivtx))
	    {
	      difference(pc,Coords(p),pdir,3);
	      if (Dot3d(pdir,t[j]) < 0.0)
	      {
	        if (tp[j].tnl.tri != NULL)
		{
	          folded = tn_common_vertex(iv,ivtx,tris[i],&tp[j].tnl,folded);
		}
	        else
	        {
	          copy_posn(tp[j].tnl.pc,pc);
		  if (ivtx == -1)
		    set_tn_on_tri_side(&tp[j].tnl,tris[i],iside);
		  else
		    set_tn_on_tri_vertex(&tp[j].tnl,tris[i],ivtx);
	        }
	      }
	      else
	      {
	        if (tp[j].tnr.tri != NULL)
		{
	          folded = tn_common_vertex(iv,ivtx,tris[i],&tp[j].tnr,folded);
		}
	        else
	        {
	          copy_posn(tp[j].tnr.pc,pc);
	          if (ivtx == -1)
	            set_tn_on_tri_side(&tp[j].tnr,tris[i],iside);
		  else
		    set_tn_on_tri_vertex(&tp[j].tnr,tris[i],ivtx);
	        }
	      }
	    }
	  }
	}
	if ( (tp[0].tnr.tri == NULL) || (tp[0].tnl.tri == NULL) ||
	     (tp[1].tnr.tri == NULL) || (tp[1].tnl.tri == NULL) ) 
	{
	  int iv0, ivl;
	  int side0, sidel;
	  /* Check if p is on a boundary curve */
	  iv0 = Vertex_of_point(tris[0],p);
	  ivl = Vertex_of_point(tris[num_tris-1],p);
	  side0 = sidel = -1;
	  if (num_tris == 1)
	  {
	      if (is_side_bdry(tris[0],iv0) ||
		      (Tri_on_side(tris[0],iv0) == NULL))
	          side0 = iv0;
	      if (is_side_bdry(tris[0],Prev_m3(iv0)) ||
		   (Tri_on_side(tris[0],Prev_m3(iv0)) == NULL))
	          sidel = Prev_m3(iv0);
	  }
	  else if (is_side_bdry(tris[0],iv0) ||
		  (Tri_on_side(tris[0],iv0) == NULL))
	      side0 = iv0;
	  else if (is_side_bdry(tris[0],Prev_m3(iv0)) ||
		   (Tri_on_side(tris[0],Prev_m3(iv0)) == NULL))
	      side0 = Prev_m3(iv0);
	  if (is_side_bdry(tris[num_tris-1],ivl) ||
		  (Tri_on_side(tris[num_tris-1],ivl) == NULL))
	      sidel = ivl;
	  else if (is_side_bdry(tris[num_tris-1],Prev_m3(ivl)) ||
		   (Tri_on_side(tris[num_tris-1],Prev_m3(ivl)) == NULL))
	      sidel = Prev_m3(ivl);
	  if ((side0 != -1) && (sidel != -1))
	  {
	    for (i = 0; i < 2; ++i)
	    {
	      if ((tp[i].tnr.tri == NULL) && (tp[i].tnl.tri != NULL))
	      {
	        tp[i].tnr.tri = tp[i].tnl.tri;
	        tp[i].tnr.is_vertex = YES;
	        tp[i].tnr.iv = Vertex_of_point(tp[i].tnr.tri,p);
	      }
	      else if ((tp[i].tnr.tri != NULL) && (tp[i].tnl.tri == NULL))
	      {
	        tp[i].tnl.tri = tp[i].tnr.tri;
	        tp[i].tnl.is_vertex = YES;
	        tp[i].tnl.iv = Vertex_of_point(tp[i].tnl.tri,p);
	      }
	      else if ((tp[i].tnr.tri == NULL) && (tp[i].tnl.tri == NULL))
	      {
		FT_POINT *popp;
		double d0, dl;
		/* all tris are on one side of the plane */
		popp = (side0 == iv0) ?  Point_of_tri(tris[0])[Next_m3(iv0)] :
		                         Point_of_tri(tris[0])[Prev_m3(iv0)];
	        difference(Coords(popp),Coords(p),pdir,3);
		d0 = Dot3d(pdir,t[i]);
		popp = (sidel == ivl) ?  Point_of_tri(tris[num_tris-1])
			                              [Next_m3(ivl)] :
		                         Point_of_tri(tris[num_tris-1])
					              [Prev_m3(ivl)];
	        difference(Coords(popp),Coords(p),pdir,3);
		dl = Dot3d(pdir,t[i]);
		if (d0 > dl)
		{
	            tp[i].tnr.tri = tris[0];
	            tp[i].tnl.tri = tris[num_tris-1];
		}
		else
		{
	            tp[i].tnl.tri = tris[0];
	            tp[i].tnr.tri = tris[num_tris-1];
		}

	        tp[i].tnr.is_vertex = YES;
	        tp[i].tnr.iv = Vertex_of_point(tp[i].tnr.tri,p);

	        tp[i].tnl.is_vertex = YES;
	        tp[i].tnl.iv = Vertex_of_point(tp[i].tnl.tri,p);
	      }
	    }
	  }
	}
	if (folded ||
	     ( ((tp[0].tnr.tri == NULL) && (tp[0].tnl.tri == NULL)) ||
	       ((tp[1].tnr.tri == NULL) && (tp[1].tnl.tri == NULL)) ) )
	{
	    /* A cusped geometry has been detected, recompute the normal   */
	    /* vector so that the local triangles project in a regular way */
	    /* onto the tangent space                                      */
	    if (debugging("tparams"))
	    {
		FT_POINT      *p;
		double      BBL[3], BBU[3];
		double      unor[3], ut1[3], ut2[3];
		double      len;
		char       s[256];
		const char *scnt;
		static int cnt = 0;

		len = Mag3d(tp[0].nor);
		unor[0] = tp[0].nor[0]/len;
		unor[1] = tp[0].nor[1]/len;
		unor[2] = tp[0].nor[2]/len;
		len = Mag3d(t[0]);
		ut1[0] = t[0][0]/len;
		ut1[1] = t[0][1]/len;
		ut1[2] = t[0][2]/len;
		len = Mag3d(t[1]);
		ut2[0] = t[1][0]/len;
		ut2[1] = t[1][1]/len;
		ut2[2] = t[1][2]/len;

		(void) printf("WARNING in set_tangent_space_projection(), "
			      "Interface folded\n");
	        set_tri_list_bounding_box(tris,num_tris,BBL,BBU,NO,YES);
	        len = sqrt(sqr(BBU[0]-BBL[0]) +
			   sqr(BBU[1]-BBL[1]) +
			   sqr(BBU[2]-BBL[2]));
		unor[0] *= len; unor[1] *= len; unor[2] *= len;
		 ut1[0] *= len;  ut1[1] *= len;  ut1[2] *= len;
		 ut2[0] *= len;  ut2[1] *= len;  ut2[2] *= len;
	        p = tp[0].p;
		set_vector_bounding_box(Coords(p),unor,-1.0,BBL,BBU,YES,YES);
		set_vector_bounding_box(Coords(p),unor,1.0,BBL,BBU,YES,YES);
		set_vector_bounding_box(Coords(p),ut1,-1.0,BBL,BBU,YES,YES);
		set_vector_bounding_box(Coords(p),ut1,1.0,BBL,BBU,YES,YES);
		set_vector_bounding_box(Coords(p),ut2,-1.0,BBL,BBU,YES,YES);
		set_vector_bounding_box(Coords(p),ut2,1.0,BBL,BBU,YES,YES);
		scnt = right_flush(cnt,6);
		(void) sprintf(s,"tan_params-axes.%s",scnt);
		gview_plot_axes("",s,BBL,BBU,BBL,BBU);
		(void) sprintf(s,"tan_params-coords-axes.%s",scnt);
		gview_plot_coord_sys("",s,Coords(p),
			             unor,ut1,ut2,BBL,BBU);
		(void) sprintf(s,"tan_params-tris.%s",scnt);
		gview_plot_triangle_list("",s,tris,num_tris,
			                 0.1,0.0,0.0,0.9,0.0,0.0,0.5,BBL,BBU);
		++cnt;
	    }
	}
	return folded;
}		/*end set_tangent_space_projection*/

LOCAL boolean tn_common_vertex(
	int      iv,
	int      ivtx,
	TRI      *tri,
	const TN *tn,
	boolean     folded)
{
	if (tn->is_vertex && (ivtx != -1))
	{
	    /*
	     * Both instances occur at a vertex, are the
	     * triangles neighbor's ?
	     */
	    if (ivtx == Next_m3(iv))
	    {
		if (Tri_on_side(tri,iv) != tn->tri)
		    folded = YES;
	    }
	    else if (ivtx == Prev_m3(iv))
	    {
		if (Tri_on_side(tri,ivtx) != tn->tri)
		    folded = YES;
	    }
	    else
	    {
		screen("ERROR in to_common_vertex(), inconsistent indices\n");
		clean_up(ERROR);
	    }
	}
	else
	    folded = YES;
	return folded;
}		/*end tn_common_vertex*/

LOCAL	boolean update_state_in_tan_direction(
	Front         *fr,
	const Tparams *tp,
	FT_POINT         *newp,
	double         kappa)
{
	Locstate 	    ansl, ansr;
	static  Tan_stencil *sten = NULL;

	if (sten == NULL)
	    sten = alloc_tan_stencil(fr,fr->npts_tan_sten/2);

	set_up_tangent_stencil(fr,sten,tp,newp,kappa);

	ansl = left_state(newp);
	ansr = right_state(newp);
	npt_tang_solver(tp->ds,tp->dt,sten,ansl,ansr,fr);
}	/* end update_state_in_tan_direction */

EXPORT	boolean set_up_tangent_stencil(
	Front         *fr,
	Tan_stencil *sten,
	const Tparams *tp,
	FT_POINT         *newp,
	double         kappa)
{
	int                 il, ir;
	Locstate            sl,sr;

	ir =  sten->npts/2 + 1;
	il = -sten->npts/2 - 1;

	if (tp->tnl.tri != NULL)
	    sten->hse[0] = Hyper_surf_element(tp->tnl.tri);
	else if (tp->tnr.tri != NULL)
	    sten->hse[0] = Hyper_surf_element(tp->tnr.tri);
	else
	{
	    (void) printf("WARNING in set_up_tangent_stencil(), "
			  "can't set sten->hse[0]\n");
	    return NO;
	}
	sten->hs[0] = tp->hs;
	sten->p[0] = tp->p;

	slsr(tp->p,Hyper_surf_element(tp->tnl.tri),tp->hs,&sl,&sr);
	ft_assign(sten->leftst[0],sl,fr->sizest);
	ft_assign(sten->rightst[0],sr,fr->sizest);

	fill_stencil_in_direction(fr,sten,-1,il,tp);
	fill_stencil_in_direction(fr,sten,1,ir,tp);

	sten->newhs = tp->hs;
	sten->dir = tp->tan;
	sten->curvature = kappa;
	return YES;
}		/*end set_up_tangent_stencil*/


/*
*
*
*                   set_weight_for_tri_interpolation():
*
*       This function takes a point p in the interior of a triangle TRI 
*	and computes the coefficients of the linear combination
*
*		p = f[0]*p0 + f[1]*p1 + f[2]*p2
*
*	where p0, p1, and p3 are the verticies of tri, and
*	f[0] + f[1] + f[2] = 1.
*/

/*ARGSUSED*/
EXPORT	void set_weight_for_tri_interpolation(
	double	  *p,
	TRI	  *tri,
	double	  *f,
	double	  *h,
	INTERFACE *intfc)
{
	static const double WEIGHT_TOL = 1.0e-5;	/*TOLERANCE*/
	double	           *p0 = Coords(Point_of_tri(tri)[0]);
	double	           *p1 = Coords(Point_of_tri(tri)[1]);
	double	           *p2 = Coords(Point_of_tri(tri)[2]);
	double	           p0p[3], p1p[3], p2p[3], x[3];
	double	           den;
	boolean	           renormalize;
	const double        *nor = Tri_normal(tri);

	den = Dot3d(nor,nor); /* NOTE: by construction tri->nor 
			      * equals (p1-p0) x (p2-p0) */
	if (den == 0.0)
	{
	    f[0] = 1.0/3.0;
	    f[1] = 1.0/3.0;
	    f[2] = 1.0/3.0;

	    (void) printf("WARNING in "
	                  "set_weight_for_tri_interpolation(), "
	                  "degenerate tri found\n");
	    print_tri(tri,intfc);
	    return;
	}

	difference(p0,p,p0p,3);   /* Note: difference(a,b,c) is c=a-b*/
	difference(p1,p,p1p,3);
	difference(p2,p,p2p,3);

	Cross3d(p0p,nor,x);     /* Note: Cross3d(a,b,c) is c=axb*/

	f[1] =  Dot3d(p2p,x);
	f[2] = -Dot3d(p1p,x);
	f[0] = den - (f[1] + f[2]);

	if (f[0] < 0.0)
	{
	    Cross3d(p1p,p2p,x);
	    f[0] = Dot3d(nor,x);
	    den = f[0] + f[1] + f[2];
	}
	f[0] /= den;
	f[1] /= den;
	f[2] /= den;

	renormalize = NO;
	if (f[0] < 0.0 && f[0] > -WEIGHT_TOL)
	{
	    f[0] = 0.0;
	    renormalize = YES;
	}
	if (f[1] < 0.0 && f[1] > -WEIGHT_TOL)
	{
	    f[1] = 0.0;
	    renormalize = YES;
	}
	if (f[2] < 0.0 && f[2] > -WEIGHT_TOL)
	{
	    f[2] = 0.0;
	    renormalize = YES;
	}
	if (f[0] < 0.0 || f[1] < 0.0 || f[2] < 0.0)
	{
	    screen("ERROR in set_weight_for_tri_interpolation(), "
	           "negative weight beyond tolerance\n");

	    (void) printf("f = (%g %g %g)\n",f[0],f[1],f[2]);
	    (void) printf("p = (%g %g %g)\n",p[0],p[1],p[2]);
	    (void) printf("p0 = (%g %g %g)\n",p0[0],p0[1],p0[2]);
	    (void) printf("p1 = (%g %g %g)\n",p1[0],p1[1],p1[2]);
	    (void) printf("p2 = (%g %g %g)\n",p2[0],p2[1],p2[2]);
	    (void) printf("Tri_normal(tri) = (%g %g %g)\n",nor[0],nor[1],nor[2]);
	    (void) printf("sqr_norm(tri) = %g\n",sqr_norm(tri));
	    (void) printf("p0 - p = (%g %g %g)\n",
	    	          p0[0]-p[0],p0[1]-p[0],p0[2]-p[2]);
	    (void) printf("p1 - p = (%g %g %g)\n",
			  p1[0]-p[0],p1[1]-p[0],p1[2]-p[2]);
	    (void) printf("p2 - p = (%g %g %g)\n",
	    	          p2[0]-p[0],p2[1]-p[0],p2[2]-p[2]);
	    clean_up(ERROR);
	}
	if (renormalize)
	{
	    den = f[0] + f[1] + f[2];
	    f[0] /= den;
	    f[1] /= den;
	    f[2] /= den;
	}

}		/*end set_weight_for_tri_interpolation*/


EXPORT	void fill_stencil_in_direction(
	Front*		fr,
	Tan_stencil*	sten,
	int		index,
	int		limit,
	const Tparams	*tp)
{
	TRI 		*n_t, *o_t;
	FT_POINT 		*ptmp;
	TN              tn;
	double		l,dist;
	double		po[3], pn[3], posn[3],dir[3];
	int		i, iv, iside, loop_index;
	boolean            flag;
	const double	*plane = tp->plane;
	double		ds = tp->ds;
	HYPER_SURF	*hs = tp->hs;
	static	TRI	*tris[50];
	int		num_tris = 0;

	tn = (limit < 0) ? tp->tnl : tp->tnr;

	n_t = tn.tri;
	copy_posn(po,Coords(tp->p));
	copy_posn(pn,tn.pc);
	if (n_t == NULL)
	{
	    copy_stencil_in_direction(sten,limit,fr);
	    return;
	}
	num_tris = 0;

	loop_index = 0;
	dist = ds;
	while (index != limit)
	{
	    ++loop_index;
	    if (loop_index >= 50) 
	    {
		screen("ERROR in fill_stencil_in_direction(), "
		       "loop_index exceeds 50\n");
		print_general_vector("point - ",Coords(sten->p[0]),
				3,"\n");
		clean_up(ERROR);
	    }

	    l = distance_between_positions(po,pn,3);
	    difference(pn,po,dir,3);
            while (index != limit && l >= dist)
            {
		posn[0] = po[0] + dist*dir[0]/l;
		posn[1] = po[1] + dist*dir[1]/l;
		posn[2] = po[2] + dist*dir[2]/l;

		index = set_posn_and_states(sten,n_t,hs,posn,index,limit,fr);
		dist += ds;
	    }

	    dist -= l;
	    if(debugging("tst_tan_sten"))
	    {
	        printf("dist=%25.16e  l=%25.16e\n", dist, l);
	    }

	    l = 0.0;

	    if (((!tn.is_vertex) && (is_side_bdry(n_t,tn.side)))
		||
	        ((tn.is_vertex) && (Boundary_point(Point_of_tri(n_t)[tn.iv]))) 
		||
	        ((check_record_tri(n_t,tris,&num_tris)) && (loop_index != 1)))
	    {
		index = set_rest_of_states(sten,n_t,hs,pn,index,limit,fr);

		return;
	    }

	    copy_posn(po,pn);

	    if (!tn.is_vertex)
	    {
		int is[2];

		o_t = n_t;
		n_t = Tri_on_side(o_t,tn.side);

		/*if (n_t == NULL)	*/				/*TEST*/
		/*{			*/				/*TEST*/
		/*    index = set_rest_of_states(sten,o_t,hs,	*/	/*TEST*/
		/*			       po,index,limit,fr);*/	/*TEST*/
		/*    return;		*/				/*TEST*/
		/*}		*/					/*TEST*/

		ptmp = Point_of_tri(o_t)[tn.side];
		is[0] = Vertex_of_point(n_t,ptmp);
		is[1] = Next_m3(is[0]);

		for (i = 0; i < 2; ++i)
		{
		    flag = plane_side_intersection(plane,n_t,is[i],pn,&iv);
		    if (flag)
	            {
			if (iv == -1)
			    set_tn_on_tri_side(&tn,n_t,is[i]);
			else
			    set_tn_on_tri_vertex(&tn,n_t,iv);
			break;
		    }
		}
		if(debugging("tst_tan_sten"))
		{
		    printf("side i=%d  iv=%d  \n", i, iv);
		    if(n_t != NULL)
		        print_tri_coords(n_t);
		}
	    }
	    else
	    {
		o_t = n_t;
		ptmp = Point_of_tri(n_t)[tn.iv];
		n_t = Next_tri_at_vertex(n_t,ptmp);
		while (n_t != o_t)
		{
		    iv = Vertex_of_point(n_t,ptmp);
		    iside = Next_m3(iv);
		    flag = plane_side_intersection(plane,n_t,iside,pn,&iv);
		    if (flag && is_forward_pn(po,pn,dir))
		    {
			if (iv == -1)
			    set_tn_on_tri_side(&tn,n_t,iside);
			else
			    set_tn_on_tri_vertex(&tn,n_t,iv);
			break;
		    }
		    n_t = Next_tri_at_vertex(n_t,ptmp);
		}
		
		if(debugging("tst_tan_sten"))
		{
		    printf("vertex iside=%d  iv=%d  \n", iside, iv);
		    if(n_t != NULL)
		        print_tri_coords(n_t);
		}

		if (o_t == n_t)
		{
		    index = set_rest_of_states(sten,o_t,hs,po,index,limit,fr);

		    return;
		}
	    }
	}

}		/*end fill_stencil_in_direction*/

LOCAL	boolean is_forward_pn(
	double *po,
	double *pn,
	double *t)
{
	double pdir[3];

	difference(pn,po,pdir,3);
	return (Dot3d(pdir,t) > 0.0) ? YES : NO;
}		/* end is_forward_pn */


LOCAL	void copy_stencil_in_direction(
	Tan_stencil *sten,
	int limit,
	Front *fr)
{
	int di,N,i,j;
	Locstate sl, sr, s;


	if (limit > 0)
	{
	    N = limit;
	    di = 1;
	}
	else
	{
	    N = -limit;
	    di = -1;
	}
	sl = sten->leftst[0];
	sr = sten->rightst[0];
	for (i = 0; i < N; ++i)
	{
	    j = di*i;
	    sten->hse[j] = sten->hse[0];
	    sten->hs[j] = sten->hs[0];
	    Coords((sten->p)[j])[0] = Coords((sten->p)[0])[0];
	    Coords((sten->p)[j])[1] = Coords((sten->p)[0])[1];
	    Coords((sten->p)[j])[2] = Coords((sten->p)[0])[2];
	    s = sten->leftst[j];
	    ft_assign(s,sl,fr->sizest);
	    s = sten->rightst[j];
	    ft_assign(s,sr,fr->sizest);
	}
}	/* copy_stencil_in_direction */


LOCAL	int set_rest_of_states(
	Tan_stencil*	sten,
	TRI*		t,
	HYPER_SURF*	hs,
	double*		p,
	int		index,
	int		limit,
	Front*		fr)
{
	int	i, j;
	int	N, di, pindex;
	Locstate sl, sr, s;

	if (limit > 0)
	{
	    N = limit - index;
	    di = 1;
	    pindex = index - 1;
	}
	else
	{
	    N = index - limit;
	    di = -1;
	    pindex = index + 1;
	}
	sl = sten->leftst[pindex];
	sr = sten->rightst[pindex];
	for (i = 0; i < N; ++i)
	{
	    j = index + di*i;
	    sten->hse[j] = Hyper_surf_element(t);
	    sten->hs[j] = hs;
	    Coords((sten->p)[j])[0] = p[0];
	    Coords((sten->p)[j])[1] = p[1];
	    Coords((sten->p)[j])[2] = p[2];
	    s = sten->leftst[j];
	    ft_assign(s,sl,fr->sizest);
	    s = sten->rightst[j];
	    ft_assign(s,sr,fr->sizest);
	}

	return limit;
}		/*end set_rest_of_states*/

LOCAL	int set_posn_and_states(
	Tan_stencil*	sten,
	TRI*		t,
	HYPER_SURF*	hs,
	double*		p,
	int		index,
	int		limit,
	Front*		fr)
{
	Locstate	sl1, sl2, sl3;
	Locstate	sr1, sr2, sr3;
	FT_POINT		*p1,*p2,*p3;
	double		f[3];


	set_weight_for_tri_interpolation(p,t,f,fr->rect_grid->h,hs->interface);

	Coords(sten->p[index])[0] = p[0];
	Coords(sten->p[index])[1] = p[1];
	Coords(sten->p[index])[2] = p[2];
	sten->hse[index] = Hyper_surf_element(t);
	sten->hs[index] = hs;

	p1 = Point_of_tri(t)[0];
	p2 = Point_of_tri(t)[1];
	p3 = Point_of_tri(t)[2];
	slsr(p1,Hyper_surf_element(t),hs,&sl1,&sr1);
	slsr(p2,Hyper_surf_element(t),hs,&sl2,&sr2);
	slsr(p3,Hyper_surf_element(t),hs,&sl3,&sr3);
	if ((tri_interpolate_states(fr,f[0],f[1],f[2],Coords(p1),sl1,
		                    Coords(p2),sl2,Coords(p3),sl3,
				    sten->leftst[index])
					!= FUNCTION_SUCCEEDED)
	    ||
	    (tri_interpolate_states(fr,f[0],f[1],f[2],Coords(p1),sr1,
		                    Coords(p2),sr2,Coords(p3),sr3,
				    sten->rightst[index])
					!= FUNCTION_SUCCEEDED))
	{
	    screen("ERROR in set_posn_and_states(), "
		   "tri_interpolate_states() failed\n");
	}

	return (limit < 0) ? --index : ++index;
}		/*end set_posn_and_states*/

LOCAL	void	set_tn_on_tri_side(
	TN  *tn,
	TRI *tri,
	int is)
{
	tn->tri = tri;
	tn->is_vertex = NO;
	tn->side = is;
}		/*end set_tn_on_tri_side*/

LOCAL	void	set_tn_on_tri_vertex(
	TN  *tn,
	TRI *tri,
	int iv)
{
	tn->tri = tri;
	tn->is_vertex = YES;
	tn->iv = iv;
}

LOCAL	boolean check_record_tri(
	TRI *tri,
	TRI **tris,
	int *num_tris)
{
	int i;

	for (i = 0; i < *num_tris; ++i)
	{
	    if (tris[i] == tri)
		return YES;
	}
	tris[(*num_tris)++] = tri;
	return NO;
}

EXPORT void f_surface_propagate(
	Front 		*front, 
	Front 		*newfront, 
	POINTER		wave,
	double		dt,
	double		*V)
{
	INTERFACE 		*intfc_old = front->interf;
	INTERFACE		*intfc_new = newfront->interf;
	HYPER_SURF		*oldhs, *newhs;
	HYPER_SURF_ELEMENT 	*oldhse, *newhse;
	FT_POINT			*oldp, *newp;
	DEBUG_ENTER(f_surface_propagate)

	start_clock("surface_propagate");
	(void) next_point(intfc_old,NULL,NULL,NULL);
	(void) next_point(intfc_new,NULL,NULL,NULL);
	while (next_point(intfc_old,&oldp,&oldhse,&oldhs) && 
	       next_point(intfc_new,&newp,&newhse,&newhs))
	{	
	    point_propagate(front,wave,oldp,newp,oldhse,oldhs,dt,V);
		 	
	    if (Boundary_point(newp))
	    {
		Locstate newsl,newsr;
		
		slsr(newp,newhse,newhs,&newsl,&newsr);
		ft_assign(newsl,left_state(newp),front->sizest);
		ft_assign(newsr,right_state(newp),front->sizest);
	    }
	}	

	stop_clock("surface_propagate");
	DEBUG_LEAVE(f_surface_propagate)
}		/*end f_surface_propagate*/

EXPORT void second_order_point_propagate(
        Front *front,
        POINTER wave,
        FT_POINT *oldp,
        FT_POINT *newp,
        HYPER_SURF_ELEMENT *oldhse,
        HYPER_SURF         *oldhs,
        double              dt,
        double              *V)
{
        double vel[MAXD],vm1[MAXD],s;
        int i, dim = front->rect_grid->dim;

        if (wave_type(oldhs) < MOVABLE_BODY_BOUNDARY)
        {
            for (i = 0; i < dim; ++i)
	    {
                Coords(newp)[i] = Coords(oldp)[i];
	    }
            return;
        }

        /* Use fourth order Runge Kutta method */

        (*front->vfunc)(front->vparams,front,oldp,oldhse,oldhs,vel);
        for (i = 0; i < dim; ++i)
            Coords(newp)[i] = Coords(oldp)[i] + dt*vel[i];
        (*front->vfunc)(front->vparams,front,newp,oldhse,oldhs,vm1);

        for (i = 0; i < dim; ++i)
        {
	    V[i] = 0.5*(vel[i] + vm1[i]);
            Coords(newp)[i] = Coords(oldp)[i] + dt*V[i];
            set_max_front_speed(i,fabs(V[i]),NULL,Coords(newp),front);
        }
	s = mag_vector(V,dim);
	set_max_front_speed(dim,s,NULL,Coords(newp),front);
}       /* second_order_point_propagate */


EXPORT void fourth_order_point_propagate(
        Front *front,
        POINTER wave,
        FT_POINT *oldp,
        FT_POINT *newp,
        HYPER_SURF_ELEMENT *oldhse,
        HYPER_SURF         *oldhs,
        double              dt,
        double              *V)
{
        double vel[MAXD],vm1[MAXD],vm2[MAXD],vm3[MAXD],s;
        int i, dim = front->rect_grid->dim;

        if (wave_type(oldhs) < MOVABLE_BODY_BOUNDARY)
        {
            for (i = 0; i < dim; ++i)
	    {
                Coords(newp)[i] = Coords(oldp)[i];
	    }
            return;
        }

        /* Use fourth order Runge Kutta method */

        (*front->vfunc)(front->vparams,front,oldp,oldhse,oldhs,vel);
        for (i = 0; i < dim; ++i)
            Coords(newp)[i] = Coords(oldp)[i] + 0.5*dt*vel[i];
        (*front->vfunc)(front->vparams,front,newp,oldhse,oldhs,vm1);
        for (i = 0; i < dim; ++i)
            Coords(newp)[i] = Coords(oldp)[i] + 0.5*dt*vm1[i];
        (*front->vfunc)(front->vparams,front,newp,oldhse,oldhs,vm2);
        for (i = 0; i < dim; ++i)
            Coords(newp)[i] = Coords(oldp)[i] + dt*vm2[i];
        (*front->vfunc)(front->vparams,front,newp,oldhse,oldhs,vm3);

        for (i = 0; i < dim; ++i)
        {
	    V[i] = (vel[i] + 2.0*vm1[i] + 2.0*vm2[i] + vm3[i])/6.0;
            Coords(newp)[i] = Coords(oldp)[i] + dt*V[i];
            set_max_front_speed(i,fabs(V[i]),NULL,Coords(newp),front);
        }
	s = mag_vector(V,dim);
	set_max_front_speed(dim,s,NULL,Coords(newp),front);
}       /* fourth_order_point_propagate */



EXPORT  void first_order_point_propagate(
        Front *front,
        POINTER wave,
        FT_POINT *oldp,
        FT_POINT *newp,
        HYPER_SURF_ELEMENT *oldhse,
        HYPER_SURF         *oldhs,
        double              dt,
        double              *V)
{
        double vel[MAXD],s;
        int i, dim = front->rect_grid->dim;

        if (wave_type(oldhs) < MOVABLE_BODY_BOUNDARY)
        {
            for (i = 0; i < dim; ++i)
	    {
                Coords(newp)[i] = Coords(oldp)[i];
	    }
            return;
        }

        (*front->vfunc)(front->vparams,front,oldp,oldhse,oldhs,vel);
        for (i = 0; i < dim; ++i)
        {
            Coords(newp)[i] = Coords(oldp)[i] + dt*vel[i];
            set_max_front_speed(i,fabs(vel[i]),NULL,Coords(newp),front);
        }
	s = mag_vector(vel,dim);
	set_max_front_speed(dim,s,NULL,Coords(newp),front);
}       /* first_order_point_propagate */

/******************prop along wall for 3d***********************/
#define  MAX_TRIS   50
#define  MAX_TAN_TRIS  50

/*#bjet2 used for tangent curve prop. */
LOCAL	boolean set_wall_tangent_space(
	Front		   *fr,
	Tparams		   *tp,
	double		   *t1,
	TRI	           **tris,
	int                num_tris)
{
	FT_POINT *p;
    	boolean  folded;
	double *t, *plane;
	double *p0;
	double *h = fr->rect_grid->h;
	double pc[3], pdir[3];
	int   iv, iside, ivtx;
	int   i, j;

	p = tp->p;
	t = tp->tan;		/* double[3] */
	plane = tp->plane;	/* double[4] */
	
	for (i = 0; i < 3; ++i)
	    plane[i] =  t1[i];

	p0 = Coords(p);

	plane[3] = plane[0]*p0[0]+plane[1]*p0[1]+plane[2]*p0[2];
	tp->ds = sqr(t[0]/h[0]) + sqr(t[1]/h[1]) + sqr(t[2]/h[2]);
	tp->ds = 1/sqrt(tp->ds);
	zero_scalar(&tp->tnl,sizeof(TN));
	zero_scalar(&tp->tnr,sizeof(TN));
	
	folded = NO;
	for (i = 0; i < num_tris; ++i)
	{
	    iv = Vertex_of_point(tris[i],p);
	    iside = Next_m3(iv);
	    if (plane_side_intersection(plane,tris[i],iside,pc,&ivtx))
	    {
	        if(debugging("set_tan"))
		{
		    print_general_vector("pc", pc, 3, "\n");
		    printf("iv %d  iside  %d ivtx  %d\n",iv,  iside, ivtx);
		    print_tri(tris[i], tp->hs->interface);
		}
	        difference(pc,p0,pdir,3);
	        if (Dot3d(pdir,t) < 0.0)
	        {
	            if (tp->tnl.tri != NULL)  /*two tris intersect with plane */
	                folded = tn_common_vertex(iv,ivtx,tris[i],&tp->tnl,folded);
	            else
	            {
	                copy_posn(tp->tnl.pc, pc);
		        if (ivtx == -1)
		            set_tn_on_tri_side(&tp->tnl,tris[i],iside);
		        else
		            set_tn_on_tri_vertex(&tp->tnl,tris[i],ivtx);
	            }
	        }
	        else
	        {
	            if (tp->tnr.tri != NULL)
	                folded = tn_common_vertex(iv,ivtx,tris[i],&tp->tnr,folded);
	            else
	            {
	                copy_posn(tp->tnr.pc,pc);
	                if (ivtx == -1)
	                    set_tn_on_tri_side(&tp->tnr,tris[i],iside);
		        else
		            set_tn_on_tri_vertex(&tp->tnr,tris[i],ivtx);
	            }
	        }
	    }
	}

	if (folded)
	{
	    printf("WARNING: the wall is folded, need to improve the quanlity of the wall. \n");
	}
	
	return folded;
}		/*end set_tangent_space_projection*/

EXPORT  void   print_TN(TN  *tn)
{
int	i;

	if(tn->tri == NULL)
	{
	    printf("tn->tri == NULL\n");
	    return;
	}
	print_tri_coords(tn->tri);
	for(i=0; i<3; i++)
	    printf(" %p  ", Point_of_tri(tn->tri)[i]);
	printf("\n");
	print_general_vector("pc=", tn->pc, 3, "\n");
	printf("#v  %d  %d  #side %d\n", tn->is_vertex, tn->iv, tn->side);
}

EXPORT  void   print_Tparams(
	char const *msg, 
	Tparams *tp)
{
	double	v[3];

	printf("%s\n", msg);
	printf("#hs %p (%d  %d) p %p  ds=%15.8e\n", tp->hs, 
	    negative_component(tp->hs), positive_component(tp->hs),
	    tp->p, tp->ds);

	printf("#tnl\n");
	print_TN(&tp->tnl);
	if(tp->tnl.tri != NULL)
	{
	    difference(tp->tnl.pc, Coords(tp->p), v, 3);
	    printf("#dot tan  %15.8e  dot plane %15.8e\n", 
	          Dot3d(v, tp->tan), Dot3d(v, tp->plane));
	}

	printf("#tnr\n");
	print_TN(&tp->tnr);
	if(tp->tnr.tri != NULL)
	{
	    difference(tp->tnr.pc, Coords(tp->p), v, 3);
	    printf("#dot tan  %15.8e  dot plane %15.8e\n", 
	          Dot3d(v, tp->tan), Dot3d(v, tp->plane));
	}

}

/*t1 normal to curve on the tangent plane */
/*t2 tangent to curve on the tangent plane */
LOCAL  boolean find_triad_at_point(
	double			*t1,
	double			*t2,
	double			*nor,
	FT_POINT			*p,
	BOND			*b,
	CURVE			*c,
	double			*ref_tan)
{
BOND    	*bp;
double		p0[3],p1[3],p2[3],pc[3], d0,d1;
int		i;
double		tol = 0.01;

	/*p should be b->start and bp->end */
	if(p == b->end)
	{
	    b = b->next;
	    if(b == NULL)
	        if(is_closed_curve(c))
		    b = c->first;
		else
		{
		    printf("ERROR find_triad_at_point, node point is not valid here\n");
		    clean_up(ERROR);
		}
	}
	if(p == b->start)
	{
	    bp = b->prev;
	    if(bp == NULL)
	        if(is_closed_curve(c))
		    bp = c->last;
		else
		{
		    printf("ERROR find_triad_at_point, node point is not valid here\n");
		    clean_up(ERROR);
		}
	}
	else
	{
	    printf("ERROR find_triad_at_point, curve is inconsistent\n");
	    clean_up(ERROR);
	}

	ft_assign(p0, Coords(bp->start), 3*FLOAT);
	ft_assign(p1, Coords(p), 3*FLOAT);
	ft_assign(p2, Coords(b->end), 3*FLOAT);
	
	d0 = distance_between_positions(p0, p1, 3);
	d1 = distance_between_positions(p1, p2, 3);
	for(i=0; i<3; i++)
	{
	    p0[i] = p1[i] + (p0[i] - p1[i])/d0;
	    p2[i] = p1[i] + (p2[i] - p1[i])/d1;
	    pc[i] = (p0[i] + p2[i])*0.5 - p1[i];
	}
	
	d0 = Mag3d(pc);
	if(d0 < tol)
	{
	    difference(p2, p0, t2, 3);
	    Cross3d(nor, t2, t1);
	    d0 = Mag3d(t1);
	    for(i=0; i<3; i++)
	        t1[i] /= d0;
	}
	else
	{
	    Cross3d(nor, pc, t2);
	    d0 = Mag3d(t2);
	    for(i=0; i<3; i++)
	        t2[i] /= d0;
	    Cross3d(nor, t2, t1);
	}

	/*t1 _|_ nor and |t1| = 1 */
	if(Dot3d(t1, ref_tan)<0.0)
	{
	    for(i=0; i<3; i++)
	        t1[i] *= -1.0;
	}
	Cross3d(nor, t1, t2);
	
}

EXPORT	boolean set_up_wall_tangent_params(
	Tparams			*tp,
	FT_POINT			*p,
	BOND			*b,
	CURVE			*c,
	double			*ref_tan,
	SURFACE			**surfs,
	TRI			**tris0,
	INTERFACE		*intfc,	
	Front			*fr)
{
	TRI	  **tris, *tris1[MAX_TRIS], *tris2[MAX_TRIS];
	BOND_TRI  **btris;
	double	  t1[3], t2[3], lent;
	static double   nor[3];    /*nor will be assing to tp->nor */
	boolean      folded;
	int	  i, nt1, nt2, nt;

	if(p != b->start && p != b->end)
	{
	    printf("ERROR set_up_wall_tangent_params, the point is not on the bond\n");
	    printf("%p  %p  %p\n", p, b->start, b->end);
	    clean_up(ERROR);
	}

	nt1 = set_tri_list_around_point(p, tris0[0], &tris, intfc);
	for(i=0; i<nt1; i++)
	    tris1[i] = tris[i];
	nt2 = set_tri_list_around_point(p, tris0[1], &tris, intfc);
	for(i=0; i<nt2; i++)
	{
	    tris2[i] = tris[i];
	    tris1[i+nt1] = tris2[i];
	}
	nt = nt1 + nt2;
	
	if(nt1 == -1 || nt2 == -1)
	{
	    printf("ERROR set_up_wall_tangent_params, set_tri_list_around_point fails\n");
	    clean_up(ERROR);
	}

	set_normal_from_tris(p, tris1, nt, nor);

	tp[0].hs  = Hyper_surf(surfs[0]);
	tp[1].hs  = Hyper_surf(surfs[1]);
	tp[0].p   = tp[1].p   = p;
	tp[0].nor = tp[1].nor = nor;

	/*t1 is the normal of the tangent plane. */
	tangent(p, b, c, t1, fr);
	
	/*make sure nor, t1, t2 are orthonomal */
	Cross3d(nor,t1,t2);
	lent = Mag3d(t2);
	for(i=0; i<3; i++)
	    t2[i] /= lent;

	if(Dot3d(t2, ref_tan)<0.0)
	{
	    for(i=0; i<3; i++)
	        t2[i] *= -1.0;
	}

	Cross3d(nor,t2,t1);
	
	/*nor and t2 are orthogonal, norm(t1) = 1.0 */
	/*t1: normal of the cut plane, the plane is perpendicular to the curve */
	/*t2: direction of the cut plane */
	/*nor: normal of the surface */
	copy_posn(tp[0].tan, t2);
	copy_posn(tp[1].tan, t2);

	if(NO && debugging("set_up_wall_tangent_params") && the_point(p))
	{
	    add_to_debug("set_tan");
	    print_general_vector("tan", t2, 3, "\n");
	    print_general_vector("t1", t1, 3, "\n");
	    print_general_vector("nor", nor, 3, "\n");
	}

	folded = set_wall_tangent_space(fr,&tp[0],t1,tris1,nt1);
	folded = set_wall_tangent_space(fr,&tp[1],t1,tris2,nt2);
	
	/*remove_from_debug("set_tan"); */

	if ( ((tp[0].tnr.tri == NULL) && (tp[1].tnr.tri == NULL)) || 
	     ((tp[0].tnl.tri == NULL) && (tp[1].tnl.tri == NULL)) )
	{
	    printf("WARNING set_up_wall_tangent_params "
	           "one side of the curve has no triangles.\n");
	    
	    print_general_vector("p=", Coords(p), 3, "\n");
	    printf(" %p  %p  %p  %p\n", 
	        tp[0].tnl.tri, tp[1].tnl.tri, 
	        tp[0].tnr.tri, tp[1].tnr.tri);
	    
	    /*tecplot_triad("triad", Coords(p), nor, t1, t2, tp[0].ds); */
	    /*tecplot_tris("tris1", tris1, nt1); */
	    /*tecplot_tris("tris2", tris2, nt2); */

	    plane_fit_normal3d_along_wall(nor, p, tris1, nt1, tris2, nt2);
	    find_triad_at_point(t2, t1, nor, p, b, c, ref_tan);
	   
	    /*tecplot_triad("triad1", Coords(p), nor, t1, t2, tp[0].ds); */
	
	    copy_posn(tp[0].tan, t2);
	    copy_posn(tp[1].tan, t2);
	    folded = set_wall_tangent_space(fr,&tp[0],t1,tris1,nt1);
	    folded = set_wall_tangent_space(fr,&tp[1],t1,tris2,nt2);
	    
	    printf("plane_fit  %p  %p  %p  %p\n", 
	        tp[0].tnl.tri, tp[1].tnl.tri, 
	        tp[0].tnr.tri, tp[1].tnr.tri);
	}

	return YES;

}		/* end set_up_tangent_params */

LOCAL boolean  check_hse_p(
	FT_POINT		      *p,
	HYPER_SURF_ELEMENT    *hse,
	HYPER_SURF	      *hs)
{
	int	i;
	TRI	*tri = Tri_of_hse(hse);

	for(i=0; i<3; i++)
	{
	    if(Point_of_tri(tri)[i] == p)
	        break;
	}
	if(i == 3)
	{
	    printf("ERROR check_hse_p point is inconsistent with tri.\n");
	    clean_up(ERROR);
	}
	if(tri->surf != Surface_of_hs(hs))
	{
	    printf("ERROR check_hse_p hse is inconsistent with hs.\n");
	    clean_up(ERROR);
	}
	return YES;
}

/*ref:  update_state_in_tan_direction */
EXPORT	boolean fill_tan_stencil_along_wall(
	Tan_stencil	*sten,
	Front           *fr,
	const Tparams   *tp,
	TRI		**tris,	/*tris[0] tri in left, tris[1] tri in right */
	int		dir)
{
	Locstate  sl,sr;
	int  	  nsts = sten->npts/2 + 1;

	sten->hs[0] = tp->hs;
	sten->p[0] = tp->p;

	if(NO && debugging("fill_tan_stencil_along_wall") && the_point(tp->p))
	{
	    add_to_debug("tst_tan_sten");
	}

	if(dir == -1)
	    if(tp->tnl.tri == NULL)
	        /*if tnl.tri == NULL, fill_stencil_in_direction will fill sten */
		/*using sl, tp will be useless. Using tris[0] to get the state */
	        sten->hse[0] = Hyper_surf_element(tris[0]);
	    else
	        sten->hse[0] = Hyper_surf_element(tp->tnl.tri);
	else
	    if(tp->tnr.tri == NULL)
	        sten->hse[0] = Hyper_surf_element(tris[1]);
	    else
	        sten->hse[0] = Hyper_surf_element(tp->tnr.tri);

	/*check the consistence  */
	check_hse_p(tp->p, sten->hse[0], tp->hs);

	slsr(tp->p,sten->hse[0],tp->hs,&sl,&sr);

	/*use leftst[0] and rightst[0] may be used in fill_stencil_in_direction. */
	/*states_near_location_from_WallTan */
	ft_assign(sten->leftst[0],sl,fr->sizest);
	ft_assign(sten->rightst[0],sr,fr->sizest);

	/*printf("\n#tst_tan_sten  ds=%24.16e\n", tp->ds); */
	
	/*dir == 1 use tp[1] to fill the rightst,  */
	/*tp[1] is the positive fluid side wall  */
	/*Param(sr) = Params(righst[1]) */
	/*dir == -1 use tp[0] to fill the rightst,  */
	/*tp[0] is the negative fluid side wall  */
	/*Param(sl) = Params(righst[-1]) */
	fill_stencil_in_direction(fr,sten,dir,dir*nsts,tp);

	/*remove_from_debug("tst_tan_sten"); */
	return YES;
}		/*end fill_stencil_along_wall*/

EXPORT	void find_position_along_wall(
	double		*posn,
	TN              *tn,
	double		dist,
	const Tparams	*tp,
	Front*		fr)
{
	TRI 		*n_t, *o_t, **tris_on_p;
	FT_POINT 		*ptmp;
	double		l;
	double		po[3], pn[3], dir[3];
	const double	*plane = tp->plane;
	int		num_tris, nt;
	int		isn, is[2], i, iv, iside, loop_index;
	boolean            flag;
	static	TRI	*tris[MAX_TAN_TRIS];

	*tn = (dist < 0) ? tp->tnl : tp->tnr;
	dist = fabs(dist);

	n_t = tn->tri;    /*n_t != NULL */
	copy_posn(po,Coords(tp->p));
	copy_posn(pn,tn->pc);

	/*print_general_vector("#plane", plane, 4, "\n"); */
	num_tris = 0;

	for(loop_index=0; loop_index<MAX_TAN_TRIS; loop_index++)
	{
	    l = distance_between_positions(po,pn,3);
	    difference(pn,po,dir,3);
            
	    if(debugging("find_position_along_wall"))
	    {
	        print_general_vector("#pn", pn, 3, "\n");
	        printf("#v  %d  %d  #side %d   %p  ", 
		    tn->is_vertex, tn->iv, tn->side, tn->tri);
	        if(!tn->is_vertex)
		    printf("neighbor %p\n", Tri_on_side(tn->tri, tn->side));
	    }

	    /*0. point in tri found */
	    if(l >= dist)
            {
	        for(i=0; i<3; i++)
		    posn[i] = po[i] + dist*dir[i]/l;
	        return;
	    }
	    dist -= l;

	    /*1. hit wall boundary edge */
	    if( (!tn->is_vertex) && (is_side_bdry(n_t, tn->side)) )
	    {
	        if(is_wall_side(n_t, tn->side))
		{
		    copy_posn(posn, pn);
		    return;
		}
	    }
	    /*2. hit wall boundary vertex */
	    if((tn->is_vertex) && (Boundary_point(Point_of_tri(n_t)[tn->iv])))
	    {
	        if(is_wall_vertex(n_t, tn->iv))
		{
		    copy_posn(posn, pn);
		    return;
		}
	    }
	    /*3. very small surface formed */
	    if((check_record_tri(n_t,tris,&num_tris)) && (loop_index != 0))
	    {
	        copy_posn(posn, pn);
	        return;
	    }

	    copy_posn(po,pn);

	    if(!tn->is_vertex)
	    {
		o_t = n_t;
		n_t = Tri_on_side_along_wall(&isn, o_t, tn->side);
		
		if(n_t == NULL)
		{   /*The only case is tn->side is not bdry and Tri_on_side ==NULL,  */
		    /*if n_t tn->side is case 1. the function returned before.  */
		    /*if n_t == NULL inside the above, Tri_on_side_along_wall will clean_up */
		    printf("ERROR find_position_along_wall, can not find new tri.\n");
		    clean_up(ERROR);
		}

		is[0] = Next_m3(isn);
		is[1] = Next_m3(is[0]);

		for (i = 0; i < 2; ++i)
		{
		    flag = plane_side_intersection(plane,n_t,is[i],pn,&iv);
		    if (flag)
	            {
			if (iv == -1)
			    set_tn_on_tri_side(tn, n_t, is[i]);
			else
			    set_tn_on_tri_vertex(tn, n_t, iv);
			break;
		    }
		}
	    }
	    else
	    {
		o_t = n_t;
		ptmp = Point_of_tri(n_t)[tn->iv];
		nt = tri_list_along_wall(ptmp, n_t, &tris_on_p, fr->interf);

		/*WARN: tris_on_p shoud be ordered, otherwise scatter_front may fail due to the inconsistent */
		/*between processors.  a triangle may be visited twice, shoud be improved. */
		for(i = 0; i < nt; i++)
		{
		    n_t = tris_on_p[i];
		    if(n_t == o_t)
		        continue;

		    iv = Vertex_of_point(n_t,ptmp);
		    iside = Next_m3(iv);
		    flag = plane_side_intersection(plane, n_t, iside, pn, &iv);
		    if (flag)
		    {
			if (iv == -1)
			    set_tn_on_tri_side(tn, n_t, iside);
			else
			{
			    if(ptmp == Point_of_tri(n_t)[iv])
			        continue;
			    set_tn_on_tri_vertex(tn, n_t, iv);
			}
			break;
		    }
		}
		if(i == nt)
		{
		    printf("ERROR find_position_along_wall, no new tri on vertex.\n");
		    clean_up(ERROR);
		}
	    }
	}
	
	if (loop_index == MAX_TAN_TRIS) 
	{
	    printf("ERROR in find_position_along_wall,loop_index exceeds 50\n");
	    clean_up(ERROR);
	}

}		/*end fill_stencil_in_direction*/



