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
*				iprt3d.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Contains printing routines for front debugging and for all front
*	structures.
*
*/

#if defined(THREED)

#include <plotdecs.h>
#include <intfc/iloc.h>


typedef struct {
	double coord[3];
} GPT;

typedef struct {
	GPT points[3];
} GTRI;

typedef struct {
	GPT points[2];
} GLINE;

#undef PLOTFILE
#define PLOTFILE plotfile
#define close_frame erase()

#define is_draw_point(a,sign) (((a > 0 && sign > 0) || \
	(a < 0 && sign < 0)) ? YES : NO)

	/* LOCAL Function Declarations */
LOCAL	double	dot_prod(GPT, GPT);
LOCAL	int	common_lseg(GLINE,GTRI,double);
LOCAL	int	common_point(GLINE*,GTRI,int*,double);
LOCAL	void	draw_tri(GTRI,int,int,int,double);
LOCAL	int	inconsistent_sign(double, double);
LOCAL	int	inside_tri(GPT, GTRI);
LOCAL	int	intersection(double, double, double, double, double, double, double, double, double*, double*);
LOCAL	int	lseg_before_tri(GLINE, GTRI);
LOCAL	void	lseg_clip(GLINE*,int*,GTRI,double);
LOCAL	int	out_of_limit(GLINE, GTRI);
LOCAL	int	point_before_tri(GPT, GTRI);
LOCAL	void	project_point(GPT, double*, double*);
LOCAL	void	reassign_lseg_end(GLINE*, GTRI, int);
LOCAL	int	same_point(GPT,GPT,double);
LOCAL	int	test_and_bifurcate(GLINE*, GTRI, GLINE*);
LOCAL	int	visible(GPT*);
LOCAL	int	within(double);
LOCAL	void	Rotation_matrix(void);
LOCAL	void	assign_point(GPT, GPT*);
LOCAL	void	cross_prod(GPT, GPT, GPT*);
LOCAL	void	find_proj_vector(void);
LOCAL	void	find_window_limit(RECT_GRID*, double*, double*, double*, double*);
LOCAL	void	init_3d_graph_params(void);
LOCAL	void	normalize(GPT*);
LOCAL	void	open_3d_plot_file(void);
LOCAL	void	process_lseg(GLINE,int,int,double);
LOCAL	void	project_lseg(GLINE);
LOCAL	void	shift_l_seg(GLINE*, int, int);
LOCAL	void	three_D_pt_trans(GPT, GPT*);
LOCAL	void	three_D_tri_list_trans(void);
LOCAL	void	vec_sub(GPT, GPT, GPT*);

LOCAL int num_tri = 0;
LOCAL GTRI *tri_list = NULL;
LOCAL int HR;
LOCAL double dir_num[3];
LOCAL FILE *plotfile = NULL;

EXPORT	void	print_c_curve(
	C_CURVE   *c_curve,
	INTERFACE *intfc)
{
	C_BOND *cb;
	int    i;
	(void) printf("C_CURVE structure 0x%p\n",c_curve);
	(void) printf("interface = %llu\n",interface_number(c_curve->interface));
	(void) printf("start node - "); print_node(c_curve->start);
	(void) printf("end node - "); print_node(c_curve->end);
	(void) printf("s[0] = %llu, s[1] = %llu\n",
		      surface_number(c_curve->s[0]),
		      surface_number(c_curve->s[1]));
	(void) printf("curve = %llu\n",curve_number(c_curve->curve));
	(void) printf("num_points = %d\n",c_curve->num_points);
	(void) printf("C_BOND's first = 0x%p last = 0x%p\n",
		      c_curve->first,c_curve->last);
	for (i = 0, cb = c_curve->first; cb != NULL; ++i, cb = cb->next);
	(void) printf("%d C_BOND's on C_CURVE\n",i);
	(void) printf("\nPoint list\n");
	for (cb = c_curve->first; cb != NULL; cb = cb->next)
	{
	    FT_POINT *ps = cb->start, *pe = cb->end;
	    double *cs = Coords(ps), *ce = Coords(pe);
	    (void) printf("prev 0x%p next 0x%p, (%llu) %g %g %g -> "
	                  "(%llu) %g %g %g\n",cb->prev,cb->next,
			  point_number(ps),cs[0],cs[1],cs[2],
			  point_number(pe),ce[0],ce[1],ce[2]);
	}
	(void) printf("\n");
	for (cb = c_curve->first; cb != NULL; cb = cb->next)
	{
	    (void) printf("\n");
	    print_c_bond(cb,intfc);
	}
	(void) printf("End C_CURVE structure 0x%p\n",c_curve);
}		/*end print_c_curve*/

EXPORT	void print_c_bond(
	C_BOND    *cb,
	INTERFACE *intfc)
{
	(void) printf("C_BOND structure 0x%p\n",cb);
	(void) printf("prev = 0x%p, next = 0x%p\n",cb->prev,cb->next);
	(void) printf("start %llu %g %g %g\n",point_number(cb->start),
					     Coords(cb->start)[0],
					     Coords(cb->start)[1],
					     Coords(cb->start)[2]);
	(void) printf("end %llu %g %g %g\n",point_number(cb->end),
					     Coords(cb->end)[0],
					     Coords(cb->end)[1],
					     Coords(cb->end)[2]);
	(void) printf("bond - ");
	print_bond(cb->bond);
	(void) printf("s[0] - ");
	print_c_surf(cb->s,intfc);
	(void) printf("s[1] - ");
	print_c_surf(cb->s+1,intfc);
	(void) printf("c_curve = 0x%p\n",cb->c_curve);
	(void) printf("End C_BOND structure 0x%p\n",cb);
}		/*end print_c_bond*/

EXPORT	void print_c_surf(
	C_SURF    *cs,
	INTERFACE *intfc)
{
	(void) printf("C_SURF structure 0x%p\n",cs);
	(void) printf("t - ");
	print_tri(cs->t,intfc);
	(void) printf("prev_t = %llu, next_t = %llu\n",
		tri_number(cs->prev_t,intfc),tri_number(cs->next_t,intfc));
	(void) printf("start flag - ");
	print_c_surf_flag(&cs_flag_start(*cs));
	(void) printf("end flag - ");
	print_c_surf_flag(&cs_flag_end(*cs));
	(void) printf("End C_SURF structure 0x%p\n",cs);
}		/*end print_c_surf*/

EXPORT	void	print_c_surf_flag(
	C_SURF_FLAG *flag)
{
	(void) printf("on bdry = %s\n",y_or_n(cs_on_bdry(*flag)));
	(void) printf("edge vertex = %s\n",y_or_n(cs_edge_vertex(*flag)));
	(void) printf("tri side index = %d\n",cs_tri_side_index(*flag));
}		/*end print_c_surf_flag*/

LIB_LOCAL void threed_interface_plot(
	INTERFACE *intfc,
	RECT_GRID *grid)
{
	SURFACE        **s;
	TRI            *tri;
	int            i,j,k;
	int	       dim = grid->dim;
	double          *h = grid->h;
	double          ls;
	static double   xl,yl,xu,yu;
	static boolean first = YES;

	num_tri = 0;
	for (s = intfc->surfaces; *s && s; s++)
		num_tri += (*s)->num_tri;

	if (first == YES)
	{
	    first = NO;
	    open_3d_plot_file();
	    init_3d_graph_params();
	    Rotation_matrix();
	    find_proj_vector();
	}
	find_window_limit(grid,&xl,&xu,&yl,&yu);

	for (ls = 0.0, i = 0; i < dim; ++i)
	    ls += h[i]*h[i];
	ls = sqrt(ls);

	uni_array(&tri_list,num_tri,sizeof(GTRI));

	i = 0;
	for (s = intfc->surfaces; s && *s; s++)
		for (tri = first_tri(*s); !at_end_of_tri_list(tri,*s);
			tri = tri->next)
	{
	    for (j = 0; j < 3; ++j)
	    {
	    	for (k = 0; k < 3; ++k)
	    	{
	    	    tri_list[i].points[j].coord[k] =
			Coords(Point_of_tri(tri)[j])[k];
	    	}
	    }
	    ++i;
	}

	/* Draw three-dimensional interface */

	viewport(0,0,1,1);
	window(xl,yl,xu,yu);

	three_D_tri_list_trans();

	for (i = 0; i < num_tri; ++i)
	    draw_tri(tri_list[i],i,NO,HR,ls);
	close_frame;

	free(tri_list);
}		/* end threed_interface_plot*/

LOCAL   void open_3d_plot_file(void)
{
	char name[20];
	int myid, nn, nd;

	if( plotfile != NULL )
	    return;

	nn = pp_numnodes();
	myid = pp_mynode();
	for (nd = 0; nn != 0; nn /=10, nd++);
	(void) strcpy(name,"plots/test");
	(void) sprintf(name,"%s.%s",name,right_flush(myid,nd));
	if( (plotfile = fopen(name,"w")) == NULL )
	{
	    screen("ERROR in open_3d_plot_file(), can't open %s\n",name);
	    clean_up(ERROR);
	}
	if (debugging("nobuf")) setbuf(plotfile,NULL);
	openpl();
}		/* end open_3d_plot_file*/



LOCAL void find_window_limit(
	RECT_GRID *grid,
	double *xl,
	double *xu,
	double *yl,
	double *yu)
{
	double xxl,yyl,xxu,yyu;
	double XC[2],YC[2],ZC[2];
	double xwidth,ywidth;
	double delta;
	int i,j,k;


	XC[0] = grid->VL[0];	XC[1] = grid->VU[0];
	YC[0] = grid->VL[1];	YC[1] = grid->VU[1];
	ZC[0] = grid->VL[2];	ZC[1] = grid->VU[2];

	xxl = yyl =  1000.0;
	xxu = yyu = -1000.0;
	for (i = 0; i < 2; ++i)
	{
	    for (j = 0; j < 2; ++j)
	    {
		for (k = 0; k < 2; ++k)
		{
		    GPT p,pt;
		    double xp,yp;
		    p.coord[0] = XC[i];   
		    p.coord[1] = YC[j];   
		    p.coord[2] = ZC[k];
		    three_D_pt_trans(p,&pt);
		    project_point(pt,&xp,&yp);
		    if (xp > xxu) xxu = xp;
		    if (yp > yyu) yyu = yp;
		    if (xp < xxl) xxl = xp;
		    if (yp < yyl) yyl = yp;
		}
	    }
	}
	xwidth = xxu - xxl;
	ywidth = yyu - yyl;
	if (xwidth > ywidth)
	{
	    delta = 0.1*xwidth;
	    *xl = xxl - delta;		*xu = xxu + delta;
	    *yl = 0.5*(yyl + yyu) - 0.5*xwidth - delta;
	    *yu = 0.5*(yyl + yyu) + 0.5*xwidth + delta;
	}
	else
	{
	    delta = 0.05*xwidth;
	    *yl = yyl - delta;		*yu = yyu + delta;
	    *xl = 0.5*(xxl + xxu) - 0.5*ywidth - delta;
	    *xu = 0.5*(xxl + xxu) + 0.5*ywidth + delta;
	}
	return;
}


LOCAL void init_3d_graph_params(void)
{
	HR = YES;
	dir_num[0] = 0.6;
	dir_num[1] = 0.9;
	dir_num[2] = 0.7;
}

LOCAL double M[3][3];
LOCAL GPT VX,VY;


LOCAL int visible(GPT *f)
{
	GPT   p1,p2,dop,norm_vec;
	int	vis;

	vec_sub(f[0],f[1],&p1);
	vec_sub(f[1],f[2],&p2);
	cross_prod(p1,p2,&norm_vec);
	dop.coord[0]=-1.0;
	dop.coord[1]=-1.0;
	dop.coord[2]=-1.0;
	if (dot_prod(dop,norm_vec)<0)
        {
       		vis=YES;
        }
	else
       	{
         	vis=NO;
       	}
	return vis;
}

LOCAL void Rotation_matrix(void)
{
	double M1[3][3],M2[3][3];
	double rsin,rcos,length;
	int i,j,l;

	/* Rotation matrix about y-axis */

	length = sqrt(dir_num[0]*dir_num[0]+dir_num[2]*dir_num[2]);
	rsin = dir_num[0]/length;
	rcos = dir_num[2]/length;

	M1[0][0] = rcos;
	M1[0][1] = 0.0;
	M1[0][2] = rsin;
	M1[1][0] = 0.0;
	M1[1][1] = 1.0;
	M1[1][2] = 0.0;
	M1[2][0] = -rsin;
	M1[2][1] = 0.0;
	M1[2][2] = rcos;

	/* Rotation matrix about x-axis */

	length = sqrt(dir_num[1]*dir_num[1]+dir_num[2]*dir_num[2]);
	rsin = dir_num[1]/length;
	rcos = dir_num[2]/length;

	M2[0][0] = 1.0;
	M2[0][1] = 0.0;
	M2[0][2] = 0.0;
	M2[1][0] = 0.0;
	M2[1][1] = rcos;
	M2[1][2] = -rsin;
	M2[2][0] = 0.0;
	M2[2][1] = rsin;
	M2[2][2] = rcos;

	/* Compute the rotation matrix */

	for (i = 0; i < 3; ++i)
	{
	    for (j = 0; j < 3; ++j)
	    {
		M[i][j] = 0.0;
		for (l = 0; l < 3; l++)
		{
		    M[i][j] += M1[i][l]*M2[l][j];
		}
	    }
	}
}

LOCAL void three_D_pt_trans(GPT p, GPT *p_t)
{
	int i,l;

	for (i = 0; i < 3; ++i)
	{
	    p_t->coord[i] = 0.0;
	    for (l = 0; l < 3; l++)
	    {
		p_t->coord[i] += M[i][l]*p.coord[l];
	    }
	}
}

LOCAL void find_proj_vector(void)
{
	GPT v;

	v.coord[0] = 0.0;
	v.coord[1] = 0.0;
	v.coord[2] = 1.0;

	three_D_pt_trans(v,&VY);
	VY.coord[2] = 0.0;
	normalize(&VY);
	cross_prod(v,VY,&VX);
}

LOCAL void process_lseg(
	GLINE lseg,
	int   i_tri,
	int   HR,
	double ls)
{
	int i;
	static GLINE l_seg[10];
	int num_seg;

	if (HR == NO)
	{
	    project_lseg(lseg);
	    return;
	}

	/* Line clipping with tri_list */

	assign_point(lseg.points[0],&(l_seg[0].points[0]));
	assign_point(lseg.points[1],&(l_seg[0].points[1]));

	num_seg = 1;
	for (i = 0; i < num_tri; ++i)
	{
	    if (i == i_tri) continue;
	    if (out_of_limit(lseg,tri_list[i]) == YES)
		continue;
	    if (lseg_before_tri(lseg,tri_list[i]) == YES)
		continue;
	    if (common_lseg(lseg,tri_list[i],ls) == YES)
	    {
		if (i_tri > i) 
		{
		    return;
		}
		else continue;
	    }
	    lseg_clip(l_seg,&num_seg,tri_list[i],ls);
	    if (num_seg == 0) return;
	}
	for (i = 0; i < num_seg; ++i)
	{
	    if (same_point(l_seg[i].points[0],l_seg[i].points[1],ls))
		continue;
	    project_lseg(l_seg[i]);
	}
}		/*end process_lseg*/

LOCAL void lseg_clip(
	GLINE *l_seg,
	int   *num_seg,
	GTRI  tri,
	double ls)
{
	int i;
	int i_in;
	int num_in;
	int shift;
	int ns;

	ns = *num_seg;
	for (i = 0; i < *num_seg; ++i)
	{
	    if (common_point(&(l_seg[i]),tri,&shift,ls) == YES)
	    {
		if (shift == YES)
		{
		    shift_l_seg(l_seg,i,ns);
		    --ns;
		    --*num_seg;
		    --i;
		}
	    }
	    else
	    {
		num_in = 0;
	    	if (inside_tri(l_seg[i].points[0],tri) == YES)
	    	{
	    	    num_in++;
		    i_in = 0;
	    	}
	    	if (inside_tri(l_seg[i].points[1],tri) == YES)
	    	{
	    	    num_in++;
		    i_in = 1;
	    	}
	    	if (num_in == 2)
	    	{
		    if ((point_before_tri(l_seg[i].points[0],tri) == NO)
		       || (point_before_tri(l_seg[i].points[1],tri) == NO))
		    {
		        shift_l_seg(l_seg,i,ns);
		        --ns;
		        --*num_seg;
		        --i;
		    }
	        }
	        else if (num_in == 1)
	        {
		    if (point_before_tri(l_seg[i].points[i_in],tri) == NO)
		    {
		        reassign_lseg_end(&(l_seg[i]),tri,i_in);
		    }
	        }
	        else if (num_in == 0)
	        {
		    if ((point_before_tri(l_seg[i].points[0],tri) == NO)
		       || (point_before_tri(l_seg[i].points[1],tri) == NO))
		    {
		        GLINE l_b;
		        if (test_and_bifurcate(&(l_seg[i]),tri,&(l_b)) != NO)
		        {
			    l_seg[ns].points[0].coord[0] 
				= l_b.points[0].coord[0];
			    l_seg[ns].points[0].coord[1] 
				= l_b.points[0].coord[1];
			    l_seg[ns].points[0].coord[2] 
				= l_b.points[0].coord[2];
			    l_seg[ns].points[1].coord[0] 
				= l_b.points[1].coord[0];
			    l_seg[ns].points[1].coord[1] 
				= l_b.points[1].coord[1];
			    l_seg[ns].points[1].coord[2] 
				= l_b.points[1].coord[2];
			    ++ns;
		        }
		    }
	        }
	    }
	}
	*num_seg = ns;
}

LOCAL int out_of_limit(GLINE lseg, GTRI tri)
{
	double lcmax[2],lcmin[2];
	double tcmax[2],tcmin[2];
	int i,j;

	for (i = 0; i < 2; ++i)
	{
	    lcmax[i] = tcmax[i] = -HUGE_VAL;
	    lcmin[i] = tcmin[i] =  HUGE_VAL;
	    for (j = 0; j < 2; ++j)
	    {
		if (lseg.points[j].coord[i] > lcmax[i])
		    lcmax[i] = lseg.points[j].coord[i];
		if (lseg.points[j].coord[i] < lcmin[i])
		    lcmin[i] = lseg.points[j].coord[i];
	    }
	    for (j = 0; j < 3; ++j)
	    {
		if (tri.points[j].coord[i] > tcmax[i])
		    tcmax[i] = tri.points[j].coord[i];
		if (tri.points[j].coord[i] < tcmin[i])
		    tcmin[i] = tri.points[j].coord[i];
	    }
	    if (lcmax[i] < tcmin[i] || lcmin[i] > tcmax[i])
	    return YES;
	}
	return NO;

}

LOCAL int lseg_before_tri(GLINE lseg, GTRI tri)
{
	double lmin;
	double tmax;
	int i;

	lmin =  HUGE_VAL;
	for (i = 0; i < 2; ++i)
	{
	    if (lseg.points[i].coord[2] < lmin)
		lmin = lseg.points[i].coord[2];
	}
	tmax = -HUGE_VAL;
	for (i = 0; i < 3; ++i)
	{
	    if (tri.points[i].coord[2] > tmax)
		tmax = tri.points[i].coord[2];
	}
	if (lmin > tmax) return YES;
	else return NO;
}


LOCAL void reassign_lseg_end(GLINE *lseg, GTRI tri, int i_in)
{
	double t_lseg,t_tri; 	
	double xl1,xl2,yl1,yl2,zl1,zl2;
	double xt1,xt2,yt1,yt2,zt1,zt2;
	double zl,zt;
	int intersect_status,i;

	xl1 = lseg->points[0].coord[0];
	xl2 = lseg->points[1].coord[0];
	yl1 = lseg->points[0].coord[1];
	yl2 = lseg->points[1].coord[1];
	zl1 = lseg->points[0].coord[2];
	zl2 = lseg->points[1].coord[2];

	for (i = 0; i < 3; ++i)
	{
	    xt1 = tri.points[i%3].coord[0];
	    xt2 = tri.points[(i+1)%3].coord[0];
	    yt1 = tri.points[i%3].coord[1];
	    yt2 = tri.points[(i+1)%3].coord[1];
	    zt1 = tri.points[i%3].coord[2];
	    zt2 = tri.points[(i+1)%3].coord[2];
	    intersect_status = intersection(xl1,xl2,yl1,yl2,xt1,xt2,yt1,yt2,
		&t_lseg,&t_tri);
	    if (intersect_status == YES)
	    {
	    	zl = zl1 + (zl2 - zl1)*t_lseg;
	    	zt = zt1 + (zt2 - zt1)*t_tri;
	    	if (zl > zt) return;
	    	else
	    	{
		    lseg->points[i_in].coord[0] = xl1 + (xl2 - xl1)*t_lseg;
		    lseg->points[i_in].coord[1] = yl1 + (yl2 - yl1)*t_lseg;
		    lseg->points[i_in].coord[2] = zl;
		    return;
	    	}
	    }
	}
}

LOCAL int intersection(double x11, double x12, double y11, double y12, double x21, double x22, double y21, double y22, double *t1, double *t2)
{
	double a1,b1,c1;
	double a2,b2,c2;

	a1 = x12 - x11;
	b1 = x21 - x22;
	c1 = x21 - x11;

	a2 = y12 - y11;
	b2 = y21 - y22;
	c2 = y21 - y11;
	
	if (a1*b2 == b1*a2) return NO;
	else
	{
		double arg1,arg2;
		double determ = a1*b2 - a2*b1;
		*t1 = (c1*b2 - c2*b1)/determ;
		*t2 = (a1*c2 - a2*c1)/determ;
		arg1 = *t1;  arg2 = *t2;
		if (within(arg1) && within(arg2)) return YES;
		else return NO;
	}
}

LOCAL int within(double arg)
{
	if (arg < 1 && arg > 0) return YES;
	else return NO;
}

LOCAL int inside_tri(GPT pt, GTRI tri)
{
	double v1[2],v2[2];
	double cpt1,cpt2;
	int i;

	v1[0] = pt.coord[0] - tri.points[2].coord[0];
	v1[1] = pt.coord[1] - tri.points[2].coord[1];
	v2[0] = tri.points[0].coord[0] - tri.points[2].coord[0];
	v2[1] = tri.points[0].coord[1] - tri.points[2].coord[1];

	Cross2d(v1,v2,cpt1);

	for (i = 0; i < 2; ++i)
	{
	    v1[0] = pt.coord[0] - tri.points[i].coord[0];
	    v1[1] = pt.coord[1] - tri.points[i].coord[1];
	    v2[0] = tri.points[i+1].coord[0] - tri.points[i].coord[0];
	    v2[1] = tri.points[i+1].coord[1] - tri.points[i].coord[1];
	    Cross2d(v1,v2,cpt2);

	    if (inconsistent_sign(cpt1,cpt2) == YES)
		return NO;
	}
	return YES;
}

LOCAL int inconsistent_sign(double x1, double x2)
{
	if ((x1 > 0 && x2 > 0) ||
	    (x1 < 0 && x2 < 0))
	    return NO;
	else return YES;
}

LOCAL int point_before_tri(GPT pt, GTRI tri)
{
	GPT norm;
	GPT v1,v2;
	double z_tri;
	double z_lseg;
	int status;

	z_lseg = pt.coord[2];
	v1.coord[0] = tri.points[1].coord[0] - tri.points[0].coord[0];
	v1.coord[1] = tri.points[1].coord[1] - tri.points[0].coord[1];
	v1.coord[2] = tri.points[1].coord[2] - tri.points[0].coord[2];
	v2.coord[0] = tri.points[2].coord[0] - tri.points[0].coord[0];
	v2.coord[1] = tri.points[2].coord[1] - tri.points[0].coord[1];
	v2.coord[2] = tri.points[2].coord[2] - tri.points[0].coord[2];
	cross_prod(v1,v2,&norm);
	z_tri = tri.points[0].coord[2] 
		- (norm.coord[0]*(pt.coord[0] - tri.points[0].coord[0])
		 + norm.coord[1]*(pt.coord[1] - tri.points[0].coord[1]))
		 /norm.coord[2];
	if (z_tri < z_lseg)  status = YES;
	else status = NO;
	return status;
}

LOCAL int test_and_bifurcate(GLINE *lseg, GTRI tri, GLINE *l_b)
{
	int i_intersect;
	double t_l[2],t_tri; 	
	double xl1,xl2,yl1,yl2,zl1,zl2;
	double xt1,xt2,yt1,yt2;
	int intersect_status,i;

	xl1 = lseg->points[0].coord[0];
	xl2 = lseg->points[1].coord[0];
	yl1 = lseg->points[0].coord[1];
	yl2 = lseg->points[1].coord[1];
	zl1 = lseg->points[0].coord[2];
	zl2 = lseg->points[1].coord[2];

	i_intersect = 0;
	for (i = 0; i < 3; ++i)
	{
	    xt1 = tri.points[i%3].coord[0];
	    xt2 = tri.points[(i+1)%3].coord[0];
	    yt1 = tri.points[i%3].coord[1];
	    yt2 = tri.points[(i+1)%3].coord[1];
	    intersect_status = intersection(xl1,xl2,yl1,yl2,xt1,xt2,yt1,yt2,
		&t_l[i_intersect],&t_tri);
	    if (intersect_status == YES)
	    {
		i_intersect++;
	    }
	}
	if (i_intersect < 2) return NO;
	else 
	{
	    double t_lseg;

	    l_b->points[1].coord[0] = lseg->points[1].coord[0];
	    l_b->points[1].coord[1] = lseg->points[1].coord[1];
	    l_b->points[1].coord[2] = lseg->points[1].coord[2];
	    t_lseg = min(t_l[0],t_l[1]);
	    lseg->points[1].coord[0] = xl1 + (xl2 - xl1)*t_lseg;
	    lseg->points[1].coord[1] = yl1 + (yl2 - yl1)*t_lseg;
	    lseg->points[1].coord[2] = zl1 + (zl2 - zl1)*t_lseg;
	    t_lseg = max(t_l[0],t_l[1]);
	    l_b->points[0].coord[0] = xl1 + (xl2 - xl1)*t_lseg;
	    l_b->points[0].coord[1] = yl1 + (yl2 - yl1)*t_lseg;
	    l_b->points[0].coord[2] = zl1 + (zl2 - zl1)*t_lseg;
	    return YES;
	}
}

LOCAL void project_lseg(GLINE lseg)
{
	double x0,x1,y0,y1;

	x0 = dot_prod(lseg.points[0],VX);
	y0 = dot_prod(lseg.points[0],VY);
	x1 = dot_prod(lseg.points[1],VX);
	y1 = dot_prod(lseg.points[1],VY);

	line(x0,y0,x1,y1);
}

LOCAL void cross_prod(GPT v1, GPT v2, GPT *result)
{
	result->coord[0]=v1.coord[1]*v2.coord[2]-v1.coord[2]*v2.coord[1];
	result->coord[1]=v1.coord[2]*v2.coord[0]-v1.coord[0]*v2.coord[2];
	result->coord[2]=v1.coord[0]*v2.coord[1]-v1.coord[1]*v2.coord[0];
}

LOCAL void vec_sub(GPT v1, GPT v2, GPT *v3)
{
	v3->coord[0]=v2.coord[0]-v1.coord[0];
	v3->coord[1]=v2.coord[1]-v1.coord[1];
	v3->coord[2]=v2.coord[2]-v1.coord[2];
}

LOCAL double dot_prod(GPT v1, GPT v2)
{
	double temp;

	temp=v1.coord[0]*v2.coord[0]+v1.coord[1]*v2.coord[1]
		+v1.coord[2]*v2.coord[2];
    	return temp;

}

LOCAL void project_point(GPT p, double *xp, double *yp)
{
	*xp = dot_prod(p,VX);
	*yp = dot_prod(p,VY);
}

LOCAL void draw_tri(
	GTRI  tri,
	int   i_tri,
	int   flag,
	int   HR,
	double ls)
{
	GLINE lseg;
	int i;

	if (flag == YES && visible(tri.points) == NO)
	    return;
	for (i = 0; i < 3; ++i)
	{
	    lseg.points[0].coord[0] = tri.points[i%3].coord[0];
	    lseg.points[0].coord[1] = tri.points[i%3].coord[1];
	    lseg.points[0].coord[2] = tri.points[i%3].coord[2];
	    lseg.points[1].coord[0] = tri.points[(i+1)%3].coord[0];
	    lseg.points[1].coord[1] = tri.points[(i+1)%3].coord[1];
	    lseg.points[1].coord[2] = tri.points[(i+1)%3].coord[2];
	    process_lseg(lseg,i_tri,HR,ls);
	}
}		/*end draw_tri*/

LOCAL void shift_l_seg(GLINE *l_seg, int i_d, int num_seg)
{
	int i,j,k;

	for (i = i_d; i < num_seg-1; ++i)
	{
	    for (j = 0; j < 2; ++j)
	    {
		for (k = 0; k < 3; ++k)
		{
		    l_seg[i].points[j].coord[k] =
			l_seg[i+1].points[j].coord[k];
		}
	    }
	}
}

LOCAL void assign_point(GPT p1, GPT *p2)
{
	int i;

	for (i = 0; i < 3; ++i)
	{
	    p2->coord[i] = p1.coord[i];
	}
}

LOCAL void three_D_tri_list_trans(void)
{
	int i,j,k;
	GPT p_tmp;

	for (i = 0; i < num_tri; ++i)
	{
	    for (j = 0; j < 3; ++j)
	    {
		three_D_pt_trans(tri_list[i].points[j],&p_tmp);
		for (k = 0; k < 3; ++k)
		{
		    tri_list[i].points[j].coord[k] = p_tmp.coord[k];
		}
	    }
	}

}

LOCAL void normalize(GPT *v)
{
	double length;
	int i;

	length = 0.0;

	for (i = 0; i < 3; ++i)
	{
	    length += (v->coord[i]) * (v->coord[i]);
	}
	length = sqrt(length);
	for (i = 0; i < 3; ++i)
	{
	    v->coord[i] = v->coord[i] / length;
	}
}

LOCAL int same_point(
	GPT   p1,
	GPT   p2,
	double ls)
{
	static const double delta = 1.0e-6; /*TOLERANCE*/
	int i;

	for (i = 0; i < 3; ++i)
	{
	    if (fabs(p1.coord[i] - p2.coord[i]) > delta*ls)
		return NO;
	}
	return YES;
}		/*end same_point*/

LOCAL int common_lseg(
	GLINE lseg,
	GTRI  tri,
	double ls)
{
	int i,j;
	int status[2];

	for (i = 0; i < 2; ++i)
	{
	    status[i] = NO;
	    for (j = 0; j < 3; ++j)
	    {
		if (same_point(lseg.points[i],tri.points[j],ls) == YES)
		    status[i] = YES;
	    }
	}
	if (status[0] == YES && status[1] == YES) return YES;
	else return NO;
}

LOCAL int common_point(
	GLINE *lseg,
	GTRI  tri,
	int   *shift,
	double ls)
{
	int i,j;

	*shift = NO;
	for (i = 0; i < 2; ++i)
	{
	    for (j = 0; j < 3; ++j)
	    {
		if (same_point(lseg->points[i],tri.points[j],ls) == YES)
		{
		    if (point_before_tri(lseg->points[(i+1)%2],tri) == NO)
		    {
		    	if (inside_tri(lseg->points[(i+1)%2],tri) == NO)
		    	{
			    double xl1 = lseg->points[0].coord[0];
			    double xl2 = lseg->points[1].coord[0];
			    double yl1 = lseg->points[0].coord[1];
			    double yl2 = lseg->points[1].coord[1];
			    double zl1 = lseg->points[0].coord[2];
			    double zl2 = lseg->points[1].coord[2];
	    		    double xt1 = tri.points[(j+1)%3].coord[0];
	    		    double xt2 = tri.points[(j+2)%3].coord[0];
	    		    double yt1 = tri.points[(j+1)%3].coord[1];
	    		    double yt2 = tri.points[(j+2)%3].coord[1];
			    double t_lseg,t_tri;
	    		    int status = intersection(xl1,xl2,yl1,yl2,xt1,xt2,
				yt1,yt2,&t_lseg,&t_tri);
			    if (status == YES)
			    {
			    	lseg->points[i].coord[0] = xl1 + 
				    (xl2 - xl1)*t_lseg;
			    	lseg->points[i].coord[1] = yl1 + 
				    (yl2 - yl1)*t_lseg;
	    	   	    	lseg->points[i].coord[2] = zl1 + 
				    (zl2 - zl1)*t_lseg;
			    }
		    	}
			else 
			{
			    *shift = YES;
			}
		    }
		    return YES;
		}
	    }
	}
	return NO;
}
#endif /* defined(THREED) */
