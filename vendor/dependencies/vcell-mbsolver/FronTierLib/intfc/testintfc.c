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
*				testintfc.c:
*
*			Test Driver for Interface Package:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*/

#include <signal.h>
#include <intfc/iloc.h>

#define  NUM_INTERFACES  20


	/* LOCAL Function Declarations */

LOCAL	void	check_component(INTERFACE*);
LOCAL	void	start_up(int,char**,INIT_DATA*);
#if defined(ONED)
LOCAL	void	test1d(void);
#endif /* defined(ONED) */
#if defined(TWOD)
LOCAL	void test2d(void);
LOCAL	void untangle_and_delete_loops(INTERFACE*, CROSS*);
LOCAL	void check_delete_degenerate_node(INTERFACE*);
LOCAL 	void move_bdry_node(NODE*,CURVE*,INTERFACE*,
		int (*vfunc)(POINTER,double*,double*),POINTER,double);
LOCAL 	void init_translation_params(POINTER*,int);
LOCAL 	void init_radial_motion_params(POINTER*,int);
LOCAL 	void init_shear_motion_params(POINTER*,int);
LOCAL 	void init_sine_motion_params(POINTER*,int);
LOCAL 	void move_interface(INTERFACE*,int (*vfunc)(POINTER,double*,double*),
			POINTER,double);
LOCAL 	INTERFACE *make_line_interface(RECT_GRID*);
LOCAL 	INTERFACE *make_elliptic_interface(RECT_GRID*);
LOCAL 	boolean bond_cross_bdry_curve_at_node(BOND*,NODE*,FT_POINT*,
		CURVE**,BOND**);
LOCAL 	int translation_vel(POINTER,double*,double*);
LOCAL 	int radial_motion_vel(POINTER,double*,double*);
LOCAL 	int shear_motion_vel(POINTER,double*,double*);
LOCAL 	int sine_motion_vel(POINTER,double*,double*);
LOCAL 	double max_front_speed(RECT_GRID,int (*vfunc)(POINTER,double*,double*),
			POINTER);
#endif /* defined(TWOD) */
#if defined(THREED)
LOCAL	void test3d(void);
LOCAL 	INTERFACE *make_plane_interface(RECT_GRID*);
LOCAL 	INTERFACE *make_ellipsoid_interface(RECT_GRID*);
LOCAL 	INTERFACE *make_hyperboloid_interface(RECT_GRID*);
LOCAL 	INTERFACE *make_paraboloid_interface(RECT_GRID*);
LOCAL 	INTERFACE *make_bdry_interface(RECT_GRID*);
LOCAL 	INTERFACE *make_random_pert_interface(RECT_GRID*);
LOCAL   INTERFACE *make_temp_comp3_interface(RECT_GRID*);
LOCAL	double hyperboloid_func(POINTER,double*);
LOCAL	double paraboloid_func(POINTER,double*);
LOCAL	double random_pert_func(POINTER,double*);
#endif /* defined(THREED) */



int main(int argc, char **argv)
{
	static I_INIT_DATA	Init;
	int dim = ERROR;
	start_up(argc,argv,init_data(&Init));
	set_binary_output(NO);

#if defined(ONED) && !defined(TWOD) && !defined(THREED)
	dim = 1;
#elif defined(TWOD) && !defined(ONED) && !defined(THREED)
	dim = 2;
#elif defined(THREED) && !defined(ONED) && !defined(TWOD)
	dim = 3;
#endif /* defined(ONED) && !defined(TWOD) && !defined(THREED) */

	if (dim == ERROR)
	{
	    screen("Enter the interface dimension: ");
	    (void) Scanf("%d\n",&dim);
	}

	switch (dim)
	{
#if defined(ONED)
	case 1:
	    test1d();
	    break;
#endif /* defined(ONED) */
#if defined(TWOD)
	case 2:
	    test2d();
	    break;
#endif /* defined(TWOD) */
#if defined(THREED)
	case 3:
	    test3d();
	    break;
#endif /* defined(THREED) */
	}
	clean_up(0);
	return 0;
}



/*ARGSUSED*/
LOCAL	void	start_up(
	int		argc,
	char		**argv,
	INIT_DATA	*init)
{
#if defined(PP_MODE)
	PP_GRID *pp_grid = NULL;
#endif /* defined(PP_MODE) */
	IMPORT boolean suppress_prompts;

	suppress_prompts = YES;
 
	setbuf(stdin,NULL);
	init_clean_up(NULL,NULL);
	screen("Welcome to the World of Partial Interfaces!\n");
	 
	init_prompting_and_debugging(init);
}


/*
*				check_component():
*
*		Debugs the component() routine.
*
*/

LOCAL void check_component(INTERFACE *intfc)
{
	char s[Gets_BUF_SIZE];
	double coords[3];
	int i, dim = intfc->dim;
	static char *s1[3] = {"x", "x, y", "x, y, z"};
	static char *s2[3] = {"", " pairs", " triples"};
	
	    
	screen("Test of component(coords,intfc) vs long_component()\n");
	screen("Enter %s%s, or a <RETURN> to terminate test\n\n",s1[dim-1],
	    s2[dim-1]);
	while ((1)) {
	    screen("\nEnter point %s: ",s1);
	    (void) Gets(s);
	    if (s[0]=='\0') break;
	    for (i = 0; i < dim; i++) (void) sscan_float(s,coords+i);
	    screen("long_Component relative to Interface  = %d\n",
	    			long_component(coords,intfc));
	    screen("Component relative to Interface  = %d\n",
	    			component(coords,intfc));
	}
	(void) printf("\n\n");
}

#if defined(ONED)
LOCAL	void	test1d(void)
{
	INTERFACE  *intfc, *new_intfc;
	FT_POINT *p;
	double x;
	COMPONENT left, right;
	
	/*
	oldintfc = read_intfc(stdin);
	*/
	intfc = make_interface(1);
	x = 7.7; left = 3; right = 4;
	make_point(&x,left,right);
	x = 7.7; left = 4; right = 5;
	make_point(&x,left,right);
	x = 5.5; left = 2; right = 3;
	make_point(&x,left,right);
	x = 3.3; left = 1; right = 2;
	make_point(&x,left,right);
	print_interface(intfc);

	print_interface(intfc);

	(void) printf ("delete point x = 5.5\n");
	p = *(intfc->points+1);
	delete_point(p);
	reset_intfc_components(intfc);
	print_interface(intfc);

	(void) printf ("\n\ncopy interface\n\n");
	new_intfc = copy_interface(intfc);
	print_interface(new_intfc);
}
#endif /* defined(ONED) */

#if defined(TWOD)
LOCAL	void test2d(void)
{
	int i;
	INTERFACE  *intfc;
	RECT_GRID rgr;
	FT_POINT *p;
	BOND *b, *bond;
	CURVE *c;
	RECT_GRID grid;
	CROSS *cross;
	double coords[MAXD];
	double L[MAXD],U[MAXD];
	double vel[MAXD];
	int (*vfunc)(POINTER,double*,double*);
	const double eps = 10.0*MACH_EPS;
	static int gmax[2] = {1, 1};
	char s[10];
	POINTER params;
	double print_interval,max_speed,dt,time,max_time;
	int num_print;

	rgr.dim = 2;
	i_init_remap_and_rect_grid(&rgr);

	screen("Supported interface types are \n"
	       "\tStraight line (l)\n"
	       "\tEllipse (e)\n"
	       "\tManually created interface (m)\n"
	       "Enter choice: ");
	(void) Scanf("%s\n",s);
	switch (s[0])
	{
	case 'L':
	case 'l':
	    intfc = make_line_interface(&rgr);
	    break;
	case 'E':
	case 'e':
	    intfc = make_elliptic_interface(&rgr);
	    break;
	case 'M':
	case 'm':
	    intfc = read_interface();
	    break;
	default:
	    screen("ERROR: unsupported interface type\n");
	    clean_up(ERROR);
	}

	set_topological_grid(intfc,&rgr);
	(void) set_boundary(intfc,&topological_grid(intfc),
			    MIN_INTERIOR_COMP,eps);
	rect_bdry_redist2d(intfc,&rgr,0);
	printf("Initial interface:\n");
	print_interface(intfc);
	(void) make_bond_comp_lists(intfc);
	show_COMP(stdout,intfc);

	screen("Supported velocity fields are \n"
	       "\tTranslation (t)\n"
	       "\tRadial motion (r)\n"
	       "\tShear motion (s)\n"
	       "\tSinusiodal motion (w)\n"
	       "Enter choice: ");
	(void) Scanf("%s\n",s);
	switch (s[0])
	{
	case 'T':
	case 't':
	    vfunc = translation_vel;
	    init_translation_params(&params,2);
	    break;
	case 'R':
	case 'r':
	    vfunc = radial_motion_vel;
	    init_radial_motion_params(&params,2);
	    break;
	case 'S':
	case 's':
	    vfunc = shear_motion_vel;
	    init_shear_motion_params(&params,2);
	    break;
	case 'W':
	case 'w':
	    vfunc = sine_motion_vel;
	    init_sine_motion_params(&params,2);
	    break;
	}
	screen("Enter final time of the interface motion: ");
	Scanf("%f\n",&max_time);
	screen("Enter number of intervals for printing: ");
	Scanf("%d\n",&num_print);

	max_speed = max_front_speed(rgr,vfunc,params);
	time = 0.0;
	print_interval = max_time/num_print;
	i = 1;
	for (;;)
	{
	    dt = 0.75*rgr.h[0]/max_speed;
	    if (time+dt > i*print_interval) 
	    	dt = 1.000000001*(i*print_interval - time);
	    if (time+dt > max_time) 
	    	dt = 1.000000001*(max_time - time);
	    move_interface(intfc,vfunc,params,dt);
	    intfc->modified = YES;
	    intersections(intfc,&cross,YES);
	    if (cross)
	    	untangle_and_delete_loops(intfc,cross);
	    rect_bdry_redist2d(intfc,&rgr,0);
	    time += dt;
	    if (time >= i*print_interval)
	    {
	        (void) printf("\n\nInterface after %d-th time interval"
			      "\tTime = %f\n\n\n",
				i,time);
	        print_interface(intfc);
	        (void) make_bond_comp_lists(intfc);
	        show_COMP(stdout,intfc);
		++i;
	    }

	    if (time >= max_time) break;

	}

	(void) delete_interface(intfc);
}	/* end test2d */

LOCAL void move_bdry_node(
	NODE *n,
	CURVE *curve,
	INTERFACE *intfc,
	int (*vfunc)(POINTER,double*,double*),
	POINTER params,
	double dt)
{
	int i;
	FT_POINT *p1,*p2;
	double vel[MAXD];
	BOND *bi,*bb;
	CURVE **c,*cb;
	static FT_POINT *p,*pe,*pc;
	FT_POINT *ptmp;
	int extended,on_first;
	ORIENTATION orient,cb_orient;

	if (p == NULL)
	{
	    p  = Static_point(intfc);
	    pe = Static_point(intfc);
	    pc = Static_point(intfc);
	}
	for (i = 0; i < 2; ++i)
	    Coords(p)[i] = Coords(n->posn)[i];
	vfunc(params,Coords(p),vel);
	Coords(p)[0] += vel[0]*dt;
	Coords(p)[1] += vel[1]*dt;

	if (n == curve->start)
	{
	    p1 = curve->first->end;
	    p2 = p;
	    orient = POSITIVE_ORIENTATION;
	}
	else if (n == curve->end)
	{
	    p1 = curve->last->start;
	    p2 = p;
	    orient = NEGATIVE_ORIENTATION;
	}
	bi = Bond(p1,p2);
	extended = on_first = NO;
	bb = NULL;
	if (!bond_cross_bdry_curve_at_node(bi,n,pc,&cb,&bb))
	{
	    bi = (orient == POSITIVE_ORIENTATION) ?
			curve->first->next : curve->last->prev;
	    while (bi != NULL)
	    {
		if (bond_cross_bdry_curve_at_node(bi,n,pc,&cb,&bb))
		    break;
		bi = (orient == POSITIVE_ORIENTATION) ?
		    		bi->next : bi->prev;
	    }
	}
	else
	    on_first = YES;
	if (bb == NULL)
	{
	    for (i = 0; i < 2; i++)
	    	Coords(pe)[i] = Coords(p1)[i] + 100.0*
			(Coords(p2)[i] - Coords(p1)[i]);
	    p2 = pe;
	    bi = Bond(p1,p2);
	    if (!bond_cross_bdry_curve_at_node(bi,n,pc,&cb,&bb))
	    {
		screen("In move_bdry_node():\n");
	    	screen("NO crossing found for propagated node!\n");
		print_node(n);
		print_bond(bi);
		clean_up(ERROR);
	    }
	    extended = YES;
	}
	for (c = n->out_curves; c && *c; ++c)
	{
	    if (*c == cb)
	    	cb_orient = POSITIVE_ORIENTATION;
	}
	for (c = n->in_curves; c && *c; ++c)
	{
	    if (*c == cb)
	    	cb_orient = NEGATIVE_ORIENTATION;
	}
	ptmp = Point(Coords(pc));
	if (!extended && !on_first)
	{
	    i_cut_curve(ptmp,bi,curve,orient);
	}
	else
	{
	    for (i = 0; i < 2; ++i)
	    	Coords(n->posn)[i] = Coords(pc)[i];
	}
	if ((cb_orient == POSITIVE_ORIENTATION && bb != cb->first) ||
	    (cb_orient == NEGATIVE_ORIENTATION && bb != cb->last))
	{
	    i_cut_curve(ptmp,bb,cb,cb_orient);
	}
}	/* end move_bdry_node */

LOCAL boolean bond_cross_bdry_curve_at_node(
	BOND *b,
	NODE *n,
	FT_POINT *p,
	CURVE **c_crx,
	BOND **b_crx)
{
	CURVE **c;

	for (c = n->in_curves; c && *c; ++c)
	{
	    if (!is_bdry(*c)) continue;
	    if (bond_crosses_curve(b,*c,p,b_crx,NEGATIVE_ORIENTATION))
	    {
		*c_crx = *c;
		return YES;
	    }
	}
	for (c = n->out_curves; c && *c; ++c)
	{
	    if (!is_bdry(*c)) continue;
	    if (bond_crosses_curve(b,*c,p,b_crx,POSITIVE_ORIENTATION))
	    {
		*c_crx = *c;
		return YES;
	    }
	}
	return NO;
}	/* end bond_cross_bdry_curve_at_node */

/*ARGSUSED*/
LOCAL void untangle_and_delete_loops(INTERFACE *intfc, CROSS *cross)
{
	CURVE **curves1,**curves2,**c;
	CROSS *cr;

	for (cr = cross;  cr != NULL;  cr = cr->next)
	{
	    curves1 = split_curve(cr->p,cr->b1,cr->c1,
	    		negative_component(cr->c1),
	    		positive_component(cr->c1),
	    		negative_component(cr->c1),
	    		positive_component(cr->c1));
	    rcl_after_split(cr,cr->p,cr->b1,cr->c1,curves1);
	    curves2 = split_curve(cr->p,cr->b2,cr->c2,
	    		negative_component(cr->c2),
	    		positive_component(cr->c2),
	    		negative_component(cr->c2),
	    		positive_component(cr->c2));
	    rcl_after_split(cr,cr->p,cr->b2,cr->c2,curves2);
	}

	clip_interface2d(intfc);
	check_delete_degenerate_node(intfc);
}

LOCAL	void check_delete_degenerate_node(INTERFACE *intfc)
{
	int ni,no;
	NODE **n;
	CURVE *ci,*co,*curve;

	for (n = intfc->nodes; n && *n; ++n)
	{
	    if (is_bdry(*n))
	    	continue;
	    num_curves_at_node(*n,&ni,&no);
	    if (ni == 1 && no == 1)
	    {
	    	ci = *(*n)->in_curves;
	    	co = *(*n)->out_curves;
		if (negative_component(ci) != negative_component(co))
		    continue;
		if (positive_component(ci) != positive_component(co))
		    continue;
	    	curve = join_curves(ci,co,negative_component(ci),
				positive_component(ci),NULL);
	    	delete_node(*n);
	    }
	}
}	/* end check_delete_degenerate_node */

LOCAL INTERFACE *make_line_interface(
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

	infc = make_interface(2);

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
	    	screen("ERROR in make_line(), "
		       "insert_point_in_bond() failed\n");
		clean_up(ERROR);
	    }
	}
	return infc;
}	/* end make_line_interface */

LOCAL INTERFACE *make_elliptic_interface(
	RECT_GRID *gr)
{
	INTERFACE *intfc;
	COMPONENT compin,compout;
	ELLIP_PARAMS ellip;
	double *cen,*rad;
	CURVE *curve;

	intfc = make_interface(2);
	cen = ellip.cen;
	rad = ellip.rad;

	screen("Enter two integers as the in and out components: ");
	Scanf("%d %d\n",&compin,&compout);
	screen("Enter the coordinates of the elliptic center: ");
	Scanf("%f %f\n",&cen[0],&cen[1]);
	screen("Enter the radii of the ellipse: ");
	Scanf("%f %f\n",&rad[0],&rad[1]);

	ellip.cen = cen;	ellip.rad = rad;
	ellip.closed = YES;	
	ellip.nor_orient = POSITIVE_ORIENTATION;
	ellip.dim = 2;		
	ellip.gr = &topological_grid(intfc);
	ellip.fpoly = NULL;
	ellip.lpoly = NULL;
	ellip.ThetaS[0] = 0.0;
	ellip.gr = gr;
	curve = make_elliptic_curve(&ellip,compin,compout,0.5);
	return intfc;

}	/* end make_elliptic_interface */

struct _TRANS_PARAMS
{
	double vel[MAXD];
};
typedef struct _TRANS_PARAMS TRANS_PARAMS;

struct _RADIAL_MOTION_PARAMS
{
	double cen[MAXD];
	double speed;
};
typedef struct _RADIAL_MOTION_PARAMS RADIAL_MOTION_PARAMS;

struct _SHEAR_MOTION_PARAMS
{
	double dir[MAXD];
	double x_0,y_0;
	double dvdh;
};
typedef struct _SHEAR_MOTION_PARAMS SHEAR_MOTION_PARAMS;

struct _SINE_MOTION_PARAMS
{
	double wave_length;
	double vmax;
	double phase;
};
typedef struct _SINE_MOTION_PARAMS SINE_MOTION_PARAMS;

LOCAL int sine_motion_vel(
	POINTER params,
	double *coords,
	double *vel)
{
	SINE_MOTION_PARAMS *sine_params;

	sine_params = (SINE_MOTION_PARAMS*)params;

	vel[0] = 0.0;
	vel[1] = sine_params->vmax*sin(2.0*PI*coords[0]/
		sine_params->wave_length - sine_params->phase);
}	/* end translation_vel */

LOCAL int translation_vel(
	POINTER params,
	double *coords,
	double *vel)
{
	TRANS_PARAMS *trans_params;

	trans_params = (TRANS_PARAMS*)params;

	vel[0] = trans_params->vel[0];
	vel[1] = trans_params->vel[1];
}	/* end translation_vel */

LOCAL void init_translation_params(
	POINTER *params,
	int dim)
{
	TRANS_PARAMS *trans_params;

	stat_scalar(&trans_params,sizeof(TRANS_PARAMS));
	screen("Enter the velocity in x and y directions: ");
	Scanf("%f %f\n",&trans_params->vel[0],&trans_params->vel[1]);

	*params = (POINTER)trans_params;

}	/* end init_translation_params */

LOCAL void init_sine_motion_params(
	POINTER *params,
	int dim)
{
	SINE_MOTION_PARAMS *s_params;

	stat_scalar(&s_params,sizeof(SINE_MOTION_PARAMS));
	screen("Enter amplitude of sinusoidal velocity: ");
	Scanf("%f\n",&s_params->vmax);
	screen("Enter the wave length of the velocity field: ");
	Scanf("%f\n",&s_params->wave_length);
	screen("Enter the phase of the velocity field: ");
	Scanf("%f\n",&s_params->phase);

	*params = (POINTER)s_params;

}	/* end init_sine_motion_params */

LOCAL void init_radial_motion_params(
	POINTER *params,
	int dim)
{
	RADIAL_MOTION_PARAMS *r_params;

	stat_scalar(&r_params,sizeof(RADIAL_MOTION_PARAMS));
	screen("Enter the velocity center coordinates: ");
	Scanf("%f %f\n",&r_params->cen[0],&r_params->cen[1]);
	screen("Enter the velocity magnitude: ");
	Scanf("%f\n",&r_params->speed);

	*params = (POINTER)r_params;

}	/* end init_radial_motion_params */

LOCAL void init_shear_motion_params(
	POINTER *params,
	int dim)
{
	SHEAR_MOTION_PARAMS *s_params;
	double l;

	stat_scalar(&s_params,sizeof(SHEAR_MOTION_PARAMS));
	screen("Veolcity is perpendicular to its gradient\n");
	screen("Enter the direction of velocity gradient: ");
	Scanf("%f %f\n",&s_params->dir[0],&s_params->dir[1]);
	screen("Enter the point of zero velocity: ");
	Scanf("%f %f\n",&s_params->x_0,&s_params->y_0);
	screen("Enter the magnitude of velocity gradient: ");
	Scanf("%f\n",&s_params->dvdh);
	l = mag_vector(s_params->dir,2);
	s_params->dir[0] /= l;
	s_params->dir[1] /= l;

	*params = (POINTER)s_params;
}	/* end init_shear_motion_params */

LOCAL int radial_motion_vel(
	POINTER params,
	double *coords,
	double *vel)
{
	RADIAL_MOTION_PARAMS *r_params;
	double *cen,speed,dist;

	r_params = (RADIAL_MOTION_PARAMS*)params;
	cen = r_params->cen;
	speed = r_params->speed;

	dist = sqrt(sqr(coords[0]-cen[0]) + sqr(coords[1]-cen[1]));
	vel[0] = (coords[0]-cen[0])/dist*speed;
	vel[1] = (coords[1]-cen[1])/dist*speed;
}	/* end radial_motion_vel */

LOCAL int shear_motion_vel(
	POINTER params,
	double *coords,
	double *vel)
{
	SHEAR_MOTION_PARAMS *s_params;
	double dh,s;
	double nor[MAXD],tangt[MAXD];

	s_params = (SHEAR_MOTION_PARAMS*)params;
	tangt[0] = -s_params->dir[1];
	tangt[1] = s_params->dir[0];
	
	dh = (coords[0] - s_params->x_0)*s_params->dir[0] +
	     (coords[1] - s_params->y_0)*s_params->dir[1];
	s = s_params->dvdh*dh;
	vel[0] = s*tangt[0];
	vel[1] = s*tangt[1];
}	/* end shear_motion_vel */


LOCAL void move_interface(
	INTERFACE *intfc,
	int (*vfunc)(POINTER,double*,double*),
	POINTER params,
	double dt)
{
	CURVE **c;
	BOND *b;
	double vel[MAXD];
	NODE *ns,*ne;
	RECT_GRID *rgr = &topological_grid(intfc);
	double *L = rgr->L;
	double *U = rgr->U;
	double *h = rgr->h;
	double c_len,hmin;
	int dim = rgr->dim;

	hmin = h[0];
	if (hmin > h[1]) hmin = h[1];

	for (c = intfc->curves; c && *c; ++c)
	{
	    if (is_bdry(*c)) continue;
	    for (b = (*c)->first; b != (*c)->last; b = b->next)
	    {
	    	vfunc(params,Coords(b->end),vel);
		Coords(b->end)[0] += vel[0]*dt;
		Coords(b->end)[1] += vel[1]*dt;
	    }

	    ns = (*c)->start;
	    ne = (*c)->end;
	    if (is_bdry(ns))
	    	move_bdry_node(ns,*c,intfc,vfunc,params,dt);
	    else
	    {
	    	vfunc(params,Coords(ns->posn),vel);
	    	Coords(ns->posn)[0] += vel[0]*dt;
	    	Coords(ns->posn)[1] += vel[1]*dt;
	    }
	    if (is_bdry(ne))
	    	move_bdry_node(ne,*c,intfc,vfunc,params,dt);
	    else if (!is_closed_curve(*c))
	    {
	    	vfunc(params,Coords(ne->posn),vel);
	    	Coords(ne->posn)[0] += vel[0]*dt;
	    	Coords(ne->posn)[1] += vel[1]*dt;
	    }
	    c_len = 0.0;
	    for (b = (*c)->first; b; b = b->next)
	    	c_len += scaled_bond_length(b,h,dim);
	    equi_redist_curve_seg(*c,(*c)->first,(*c)->last,-1,c_len,
	    		0.75,rgr);
	}
	closed_curve_node_redistribute(intfc,YES);
}	/* end move_interface */


LOCAL double max_front_speed(
	RECT_GRID rgr,
	int (*vfunc)(POINTER,double*,double*),
	POINTER params)
{
	int i,j;
	int *gmax = rgr.gmax;
	double *L = rgr.L;
	double *U = rgr.U;
	double *h = rgr.h;
	double coords[MAXD],vel[MAXD];
	double max_speed = 0.0;

	for (i = 0; i <= gmax[0]; ++i)
	{
	    coords[0] = L[0] + i*h[0];
	    for (j = 0; j <= gmax[1]; ++j)
	    {
	    	coords[1] = L[1] + j*h[1];
		vfunc(params,coords,vel);
		max_speed = max(mag_vector(vel,2),max_speed);
	    }
	}
	return max_speed;
}	/* end max_front_speed */
#endif /* defined(TWOD) */

#if defined(THREED)

LOCAL	void test3d(void)
{
	int i;
	INTERFACE  *intfc;
	RECT_GRID rgr;
	double L[MAXD],U[MAXD];
	double vel[MAXD];
	int (*vfunc)(POINTER,double*,double*);
	const double eps = 10.0*MACH_EPS;
	char s[10];

	rgr.dim = 3;
	i_init_remap_and_rect_grid(&rgr);

	screen("Supported interface types are \n"
	       "\tPlane (p)\n"
	       "\tEllipse (e)\n"
	       "\tBoundary (b)\n"
	       "\tHyperboloid (h)\n"
	       "\tParaboloid (a)\n"
	       "\tRandomly perturbed interface (r)\n"
	       "Enter choice: ");
	(void) Scanf("%s\n",s);
	switch (s[0])
	{
	case 'P':
	case 'p':
	    intfc = make_plane_interface(&rgr);
	    break;
	case 'E':
	case 'e':
	    intfc = make_ellipsoid_interface(&rgr);
	    break;
	case 'H':
	case 'h':
	    intfc = make_hyperboloid_interface(&rgr);
	    break;
	case 'A':
	case 'a':
	    intfc = make_paraboloid_interface(&rgr);
	    break;
	case 'R':
	case 'r':
	    intfc = make_random_pert_interface(&rgr);
	    break;
	case 'B':
	case 'b':
	    intfc = make_bdry_interface(&rgr);
	    break;
	case 'T':
	case 't':
	    intfc = make_temp_comp3_interface(&rgr);
	    break;
	}
	set_topological_grid(intfc,&rgr);
	gview_plot_interface("test_3d_intfc",intfc);
	(void) print_interface(intfc);
}

LOCAL 	INTERFACE *make_plane_interface(
	RECT_GRID *gr)
{
	PLANE_PARAMS params;
	INTERFACE *intfc;
	COMPONENT pcomp,ncomp;
	SURFACE *surf;
	double N[MAXD],P[MAXD];

	intfc = make_interface(3);

	screen("Enter two integers as the negative and positive components: ");
	Scanf("%d %d\n",&ncomp,&pcomp);
	screen("Enter the normal vector of the plane: ");
	Scanf("%f %f %f\n",&N[0],&N[1],&N[2]);
	screen("Enter the coordinates of a point the plane passes: ");
	Scanf("%f %f %f\n",&P[0],&P[1],&P[2]);
	params.N = N;
	params.P = P;
	
	set_current_interface(intfc);
	if (!make_level_surface(gr,intfc,ncomp,pcomp,plane_func,
		(POINTER)&params,&surf))
	{
	    screen("make_plane_interface() failed!\n");
	    clean_up(ERROR);
	}
	return intfc;
}	/* end make_plane_interface */

LOCAL 	INTERFACE *make_ellipsoid_interface(
	RECT_GRID *gr)
{
	ELLIP_PARAMS params;
	INTERFACE *intfc;
	COMPONENT compin,compout;
	SURFACE *surf;
	double *cen,*rad;

	intfc = make_interface(3);
	cen = params.cen;
	rad = params.rad;

	screen("Enter two integers as the inside and outside components: ");
	Scanf("%d %d\n",&compin,&compout);
	screen("Enter the coordinates of the ellipsoid center: ");
	Scanf("%f %f %f\n",&cen[0],&cen[1],&cen[2]);
	screen("Enter the three radii of the ellipsoid: ");
	Scanf("%f %f %f\n",&rad[0],&rad[1],&rad[2]);
	params.cen = cen;
	params.rad = rad;
	
	set_current_interface(intfc);
	if (!make_level_surface(gr,intfc,compin,compout,ellipsoid_func,
		(POINTER)&params,&surf))
	{
	    screen("make_ellipsoid_interface() failed!\n");
	    clean_up(ERROR);
	}
	return intfc;
}	/* end make_ellipsoid_interface */

LOCAL	INTERFACE *make_temp_comp3_interface(
	RECT_GRID *gr)
{
	ELLIP_PARAMS params1,params2;
	INTERFACE *intfc;
	COMPONENT comp0, comp1, comp2;
	SURFACE **surf;
	CURVE   *curv;
	double *cen1, *rad1;
	double *cen2, *rad2;

	intfc = make_interface(3);
	cen1 = params1.cen;	cen2 = params2.cen;
	rad1 = params1.rad;	rad2 = params2.rad;

	screen("Enter three integers as three components: ");
	Scanf("%d %d %d\n",&comp0,&comp1,&comp2);
	
	screen("Enter the coordinates of the first ellipsoid center: ");
	Scanf("%f %f %f\n",&cen1[0],&cen1[1],&cen1[2]);
	screen("Enter the three radii of the first ellipsoid: ");
	Scanf("%f %f %f\n",&rad1[0],&rad1[1],&rad1[2]);

	screen("Enter the coordinates of the second ellipsoid center: ");
	Scanf("%f %f %f\n",&cen2[0],&cen2[1],&cen2[2]);
	screen("Enter the three radii of the second ellipsoid: ");
	Scanf("%f %f %f\n",&rad2[0],&rad2[1],&rad2[2]);
	params1.cen = cen1;
	params1.rad = rad1;
	params2.cen = cen2;
	params2.rad = rad2;

	set_current_interface(intfc);
	if (!make_comp3_surfaces(gr,comp0,comp1,comp2,ellipsoid_func,
		(POINTER)&params1,ellipsoid_func,(POINTER)&params2,
		&surf,&curv))
	{
	    screen("make_comp3_ellipsoid_interface() failed!\n");
	    clean_up(ERROR);
	}
	return intfc;
}	/* end make_temp_comp3_interface */
	

LOCAL 	INTERFACE *make_hyperboloid_interface(
	RECT_GRID *gr)
{
	ELLIP_PARAMS params;
	INTERFACE *intfc;
	COMPONENT compin,compout;
	SURFACE *surf;
	double *cen,*rad;

	intfc = make_interface(3);
	cen = params.cen;
	rad = params.rad;

	screen("Enter two integers as the inside and outside components: ");
	Scanf("%d %d\n",&compin,&compout);
	screen("Enter the coordinates of the hyperboloid center: ");
	Scanf("%f %f %f\n",&cen[0],&cen[1],&cen[2]);
	screen("The hyperboloid equation is x^2/a^2 + y^2/b^2 - z^2/c^2 = 1\n");
	screen("Enter the three parameters a, b, and c: ");
	Scanf("%f %f %f\n",&rad[0],&rad[1],&rad[2]);
	params.cen = cen;
	params.rad = rad;
	
	set_current_interface(intfc);
	if (!make_level_surface(gr,intfc,compin,compout,hyperboloid_func,
		(POINTER)&params,&surf))
	{
	    screen("make_hyperboloid_interface() failed!\n");
	    clean_up(ERROR);
	}
	return intfc;
}	/* end make_hyperboloid_interface */

LOCAL 	INTERFACE *make_paraboloid_interface(
	RECT_GRID *gr)
{
	ELLIP_PARAMS params;
	INTERFACE *intfc;
	COMPONENT compin,compout;
	SURFACE *surf;
	double *cen,*rad;

	intfc = make_interface(3);
	cen = params.cen;
	rad = params.rad;

	screen("Enter two integers as the inside and outside components: ");
	Scanf("%d %d\n",&compin,&compout);
	screen("Enter the coordinates of the hyperboloid center: ");
	Scanf("%f %f %f\n",&cen[0],&cen[1],&cen[2]);
	screen("The paraboloid equation is x^2/a^2 + y^2/b^2 - z = 0\n");
	screen("Enter the two parameters a, and b: ");
	Scanf("%f %f %f\n",&rad[0],&rad[1]);
	params.cen = cen;
	params.rad = rad;
	
	set_current_interface(intfc);
	if (!make_level_surface(gr,intfc,compin,compout,paraboloid_func,
		(POINTER)&params,&surf))
	{
	    screen("make_paraboloid_interface() failed!\n");
	    clean_up(ERROR);
	}
	return intfc;
}	/* end make_paraboloid_interface */

LOCAL 	INTERFACE *make_random_pert_interface(
	RECT_GRID *gr)
{
	FOURIER_POLY *fpoly;
	INTERFACE *intfc;
	COMPONENT compu,compl;
	SURFACE *surf;
	double z0;

	intfc = make_interface(3);

	screen("The surface is horizontal to the z-direction\n");
	screen("Enter two integers as the upper and lower components: ");
	Scanf("%d %d\n",&compu,&compl);
	screen("Enter the mean height of the interface: ");
	Scanf("%f\n",&z0);
	fpoly = get_fourier_random(gr->L,gr->U,3,"");
	fpoly->z0 = z0;
	
	set_current_interface(intfc);
	if (!make_level_surface(gr,intfc,compu,compl,random_pert_func,
		(POINTER)fpoly,&surf))
	{
	    screen("make_paraboloid_interface() failed!\n");
	    clean_up(ERROR);
	}
	return intfc;
}	/* end make_random_pert_interface */

LOCAL 	INTERFACE *make_bdry_interface(
	RECT_GRID *gr)
{
	BDRY_BOX_PARAMS params;
	INTERFACE *intfc;
	char *dir_name[] = {"X","Y","Z"};
	char *side_name[] = {"lower","upper"};
	int dir,side;
	char s[10];
	

	intfc = make_interface(3);


	for (dir = 0; dir < 3; ++dir)
	{
	    for (side = 0; side < 2; ++side)
	    {
	    	screen("Enter yes to make boundary surface on the %s side "
		       "of %s-direction: ",side_name[side],dir_name[dir]);
		Scanf("%s\n",s);
	    	if (s[0] == 'Y' || s[0] == 'y')
		{
		    rect_boundary_type(intfc,dir,side) = UNKNOWN_BOUNDARY_TYPE;
		}
		else
		{
		    rect_boundary_type(intfc,dir,side) = SUBDOMAIN_BOUNDARY;
		    if (side == 0)
		    	gr->lbuf[dir] += 2;
		    else if (side == 1)
		    	gr->ubuf[dir] += 2;
		}
	    }
	}
	set_rect_grid(gr->L,gr->U,gr->GL,gr->GU,gr->lbuf,gr->ubuf,
			gr->gmax,gr->dim,&gr->Remap,gr);
	if (!make_bdry_surfaces(intfc,gr))
	{
	    screen("make_bdry_interface() failed!\n");
	    clean_up(ERROR);
	}
	return intfc;
}	/* end make_bdry_interface */

LOCAL	double hyperboloid_func(
	POINTER func_params,
	double *coords)
{
	ELLIP_PARAMS *params;
	const double *cen,*rad;
	double arg;

	params = (ELLIP_PARAMS *)func_params;
	cen = params->cen;
        rad = params->rad;

	arg = 1.0 -
                sqr(coords[0] - cen[0])/sqr(rad[0]) -
                sqr(coords[1] - cen[1])/sqr(rad[1]) +
                sqr(coords[2] - cen[2])/sqr(rad[2]);

	return -arg;
}	/* end hyperboloid_func */


LOCAL	double paraboloid_func(
	POINTER func_params,
	double *coords)
{
	ELLIP_PARAMS *params;
	const double *cen,*rad;
	double arg;

	params = (ELLIP_PARAMS *)func_params;
	cen = params->cen;
        rad = params->rad;

	arg = sqr(coords[0] - cen[0])/sqr(rad[0]) +
              sqr(coords[1] - cen[1])/sqr(rad[1]) -
              (coords[2] - cen[2]);

	return -arg;
}	/* end hyperboloid_func */

LOCAL	double random_pert_func(
	POINTER func_params,
	double *coords)
{
	FOURIER_POLY *fpoly = (FOURIER_POLY*)func_params;
	double z = fourier_poly(coords,fpoly);

	return coords[2] - z;
}	/* end random_pert_func */

#endif /* defined(THREED) */

