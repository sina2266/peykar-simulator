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
*				fvelo.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*/

#include <front/fdecs.h>

#ifdef WIN32
#define _USE_MATH_DEFINES
#include <math.h>
#include <windows_math_func.h>
#endif

EXPORT void init_translation_params(
	POINTER *params,
	int dim)
{
	int i;
	static TRANS_PARAMS *trans_params;

	stat_scalar(&trans_params,sizeof(TRANS_PARAMS));
	trans_params->dim = dim;
	for (i = 0; i < dim; ++i)
	{
	    screen("Enter the velocity component in direction %d: ",i);
	    Scanf("%f\n",&trans_params->vel[i]);
	}

	*params = (POINTER)trans_params;

}	/* end init_translation_params */

EXPORT void init_sine_motion_params(
	POINTER *params,
	int dim)
{
	static SINE_MOTION_PARAMS *s_params;

	stat_scalar(&s_params,sizeof(SINE_MOTION_PARAMS));
	s_params->dim = dim;
	screen("Enter amplitude of sinusoidal velocity: ");
	Scanf("%f\n",&s_params->vmax);
	screen("Enter the wave length of the velocity field: ");
	Scanf("%f\n",&s_params->wave_length);
	screen("Enter the phase of the velocity field: ");
	Scanf("%f\n",&s_params->phase);

	*params = (POINTER)s_params;

}	/* end init_sine_motion_params */

EXPORT void init_radial_motion_params(
	POINTER *params,
	int dim)
{
	static RADIAL_MOTION_PARAMS *r_params;
	int i;

	stat_scalar(&r_params,sizeof(RADIAL_MOTION_PARAMS));
	r_params->dim = dim;
	screen("Enter the velocity center coordinates: ");
	for (i = 0; i < dim; ++i)
	{
	    Scanf("%f ",&r_params->cen[i]);
	}
	Scanf("\n");
	screen("Enter the velocity magnitude: ");
	Scanf("%f\n",&r_params->speed);

	*params = (POINTER)r_params;

}	/* end init_radial_motion_params */

EXPORT void init_shear_motion_params(
	POINTER *params,
	int dim)
{
	static SHEAR_MOTION_PARAMS *s_params;
	double l;
	int i;

	stat_scalar(&s_params,sizeof(SHEAR_MOTION_PARAMS));
	s_params->dim = dim;
	screen("Veolcity is perpendicular to its gradient\n");
	screen("Enter the direction of velocity gradient: ");
	for (i = 0; i < dim; ++i)
	{
	    Scanf("%f ",&s_params->dir[i]);
	}
	Scanf("\n");
	screen("Enter the coordinates of a point for zero velocity: ");
	for (i = 0; i < dim; ++i)
	{
	    Scanf("%f ",&s_params->coords[i]);
	}
	Scanf("\n");
	screen("Enter the magnitude of velocity gradient: ");
	Scanf("%f\n",&s_params->dvdh);
	l = mag_vector(s_params->dir,2);
	s_params->dir[0] /= l;
	s_params->dir[1] /= l;

	*params = (POINTER)s_params;
}	/* end init_shear_motion_params */

EXPORT void init_circular_rotation_params(
	POINTER *params,
	int dim)
{
	static CIRCULAR_ROTATION_PARAMS *c_params;
	double l;
	int i;

	stat_scalar(&c_params,sizeof(CIRCULAR_ROTATION_PARAMS));
	c_params->dim = dim;

	screen("Rotation axis is the z-axis\n");
	screen("Enter the x and y coordinates of rotation axis: ");
	Scanf("%f %f\n",&c_params->x_0,&c_params->y_0);
	screen("Enter angular velocity at rotation center: ");
	Scanf("%f\n",&c_params->omega_0);
	screen("Enter gradient of angular velocity d_omega/dr: ");
	Scanf("%f\n",&c_params->grad);

	*params = (POINTER)c_params;
}	/* end init_circular_rotation_params */

EXPORT void init_norv_params(
	POINTER *params,
        int dim)
{
	static NORV_PARAMS *norv_params;

	stat_scalar(&norv_params,sizeof(NORV_PARAMS));
	norv_params->dim = dim;
	screen("Velocity in the normal direction\n");
	screen("Enter the velocity coefficient: ");
	Scanf("%f\n",&norv_params->coeff);

	*params = (POINTER)norv_params;
}	/* end init_norv_params */

EXPORT void init_curvature_params(
	POINTER *params,
        int dim)
{
	static NORV_PARAMS *norv_params;

	stat_scalar(&norv_params,sizeof(NORV_PARAMS));
	norv_params->dim = dim;
	screen("Velocity in the normal direction\n");
	screen("Velocity magnitude = V0 - epsilon*kappa\n");
	screen("Enter V0 and epsilon: ");
	Scanf("%f %f\n",&norv_params->coeff,&norv_params->epsilon);

	*params = (POINTER)norv_params;
}	/* end init_curvature_params */


EXPORT void init_flame_params(
	POINTER *params,
        int dim)
{
        static FLAME_PARAMS *flame_params;
        CIRCULAR_ROTATION_PARAMS *c_params;
        char s[10];
        double wind[MAXD],x0,y0,omega_0,domega_dr;
        static POINTER params2;
        stat_scalar(&flame_params,sizeof(FLAME_PARAMS));
        screen("\n\t\tSpecifying wind vector field\n\n");
        screen("Supported wind vector fields are \n"
               "\tConstant (c)\n"
               "\tCircular (r)\n"
               "\tTime-dependent (t)\n"
               "Enter choice : ");
        (void)Scanf("%s\n",s);
        switch(s[0])
        {
        case 'C':
        case 'c':
                screen("Enter two components for the direction vector : ");
                Scanf("%f %f\n",
			&flame_params->wind[0],&flame_params->wind[1]);
                flame_params->problem_type =0;
        break;
                case 'R':
                case 'r':
                init_circular_rotation_params(&flame_params->stuff,dim);
                flame_params->problem_type = 1;
        break;
        case 'T':
        case 't':
                screen("Enter two components for the first position vector : ");
                Scanf("%f %f\n",&flame_params->wind[0],
			&flame_params->wind[1]);
                screen("Enter two components for the dirction"
		       "uni_array changing as time : ");
                Scanf("%f %f\n",&flame_params->a,&flame_params->b);
                flame_params->problem_type = 2;

        break;
        }

        flame_params->dim = dim;
        screen("Enter alpha: ");
        Scanf("%f\n",&flame_params->alpha);
        screen("Enter U: ");
        Scanf("%f\n",&flame_params->U);
        screen("Enter m: ");
        Scanf("%f\n",&flame_params->m);
        screen("Enter c: ");
        Scanf("%f\n",&flame_params->c);
        screen("Enter epsilon: ");
        Scanf("%f\n",&flame_params->epsilon);
        *params = (POINTER)flame_params;

}	/* end init_flame_params */

EXPORT void init_burgers_params(
	POINTER *params,
        int dim)
{
	static BURGERS_PARAMS *b_params;

	stat_scalar(&b_params,sizeof(BURGERS_PARAMS));
	b_params->dim = dim;
	*params = (POINTER)b_params;
}	/* end init_burgers_params */

EXPORT void init_bipolar_params(
	POINTER *params,
	int dim)
{
	int i;
	static BIPOLAR_PARAMS *bipolar_params;
	char s[10];

	stat_scalar(&bipolar_params,sizeof(BIPOLAR_PARAMS));
	bipolar_params->dim = dim;
	bipolar_params->reverse_time = -1;

	screen("Enter the center of the first pole: ");
	Scanf("%f %f\n",&bipolar_params->cen1[0],&bipolar_params->cen1[1]);
	screen("Enter the strength of the first pole: ");
	Scanf("%f\n",&bipolar_params->i1);

	screen("Enter the center of the second pole: ");
	Scanf("%f %f\n",&bipolar_params->cen2[0],&bipolar_params->cen2[1]);
	screen("Enter the strength of the second pole: ");
	Scanf("%f\n",&bipolar_params->i2);

	screen("Do reversal run after certain time [y or n]: ");
        Scanf("%s\n",s);
	if (s[0] == 'Y' || s[0] == 'y')
	{
	    screen("Enter a time for reversal: ");
            Scanf("%f\n",&bipolar_params->reverse_time);
	}
	*params = (POINTER)bipolar_params;

}	/* end init_translation_params */

EXPORT void init_vortex_params(
        POINTER *params,
        int dim)
{
        static VORTEX_PARAMS *vortex_params;
        char rev_time[100], comp_error[100];
        stat_scalar(&vortex_params,sizeof(VORTEX_PARAMS));
        vortex_params->dim = dim;

        screen("\nSupported Vortex Types are\n"
	       "\tSingle   (s)\n"
	       "\tMultiple (m)\n"
	       "Enter selection here: ");

        Scanf("%s\n",&vortex_params->type);

        screen("\nSupported vortex time dependency types are\n"
	       "\tNone        (n)\n"
	       "\tcos(pi*t/T) (c)\n"
	       "\treversal    (r)\n"
	       "Enter selection here: ");
	 Scanf("%s\n",rev_time);
	 vortex_params->time = -1;
	 vortex_params->cos_time = -1;
	 vortex_params->coeff = 1.0;
	 switch(rev_time[0])
	 {
	     case 'n':
	     case 'N':
	         break;
             case 'c':
	     case 'C':
	         screen("\nEnter T for cos(pi*t/T) coeff: ");
	         Scanf("%f\n",&(vortex_params->cos_time));
	         break;
	     case 'r':
	     case 'R':
	         screen("\nEnter a time for reversal: ");
	         Scanf("%f\n",&(vortex_params->time));
	         break;
	     default:
	         screen("Undefined time dependency type!\n");
	         clean_up(ERROR);
	}
	 
	*params = (POINTER)vortex_params;
}       /* end init_vortex_params */


/*
*	Velocity functions for point propagation
*/

EXPORT int sine_motion_vel(
	POINTER params,
	Front *front,
	FT_POINT *p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF *hs,
	double *vel)
{
	SINE_MOTION_PARAMS *sine_params;
	int i,dim;
	double *coords = Coords(p);

	sine_params = (SINE_MOTION_PARAMS*)params;

	dim = sine_params->dim;
	for (i = 0; i < dim-1; ++i)
	    vel[i] = 0.0;
	vel[dim-1] = sine_params->vmax*sin(2.0*PI*coords[0]/
		sine_params->wave_length - sine_params->phase);
}	/* end translation_vel */

EXPORT int translation_vel(
	POINTER params,
	Front *front,
	FT_POINT *p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF *hs,
	double *vel)
{
	TRANS_PARAMS *trans_params;
	int i;
	double *coords = Coords(p);

	trans_params = (TRANS_PARAMS*)params;

	for (i = 0; i < trans_params->dim; ++i)
	    vel[i] = trans_params->vel[i];
}	/* end translation_vel */

EXPORT int radial_motion_vel(
	POINTER params,
	Front *front,
	FT_POINT *p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF *hs,
	double *vel)
{
	RADIAL_MOTION_PARAMS *r_params;
	double *cen,speed,dist;
	int i;
	double *coords = Coords(p);

	r_params = (RADIAL_MOTION_PARAMS*)params;
	cen = r_params->cen;
	speed = r_params->speed;

	dist = 0;
	for (i = 0; i < r_params->dim; ++i)
	    dist += sqr(coords[i]-cen[i]);
	dist = sqrt(dist);
	for (i = 0; i < r_params->dim; ++i)
	    vel[i] = (coords[i]-cen[i])/dist*speed;
}	/* end radial_motion_vel */

EXPORT int burgers_vel(
	POINTER params,
	Front *front,
	FT_POINT *p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF *hs,
	double *vel)
{
	BURGERS_PARAMS *b_params = (BURGERS_PARAMS*)params;
	double *coords = Coords(p);
	int i,dim = b_params->dim;

	for (i = 0; i < dim-1; ++i)
	    vel[i] = coords[dim-1];
	vel[dim-1] = 0.0;
}	/* end burgers_vel */

EXPORT int shear_motion_vel(
	POINTER params,
	Front *front,
	FT_POINT *p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF *hs,
	double *vel)
{
	SHEAR_MOTION_PARAMS *s_params;
	double dh,s;
	double nor[MAXD],tangt[MAXD];
	double *coords = Coords(p);

	/*
	s_params = (SHEAR_MOTION_PARAMS*)params;
	tangt[0] = -s_params->dir[1];
	tangt[1] = s_params->dir[0];
	
	dh = (coords[0] - s_params->x_0)*s_params->dir[0] +
	     (coords[1] - s_params->y_0)*s_params->dir[1];
	s = s_params->dvdh*dh;
	vel[0] = s*tangt[0];
	vel[1] = s*tangt[1];
	*/
}	/* end shear_motion_vel */


EXPORT int circular_rotation_vel(
	POINTER params,
	Front *front,
	FT_POINT *p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF *hs,
	double *vel)
{
	CIRCULAR_ROTATION_PARAMS *c_params;
	double x0,y0;
	double omega_0,domega_dr;
	double rad,V;
	double xcomp,ycomp;
	double *coords = Coords(p);

	c_params = (CIRCULAR_ROTATION_PARAMS*)params;

	x0 = c_params->x_0;
	y0 = c_params->y_0;
	omega_0 = c_params->omega_0;
	domega_dr = c_params->grad;

	rad = sqrt(sqr(coords[0] - x0) + sqr(coords[1] - y0));
	if (rad == 0.0)
	{
	    vel[0] = vel[1] = vel[2] = 0.0;
	    return 1;
	}
	xcomp = fabs(coords[1]-y0)/rad;
	ycomp = fabs(coords[0]-x0)/rad;
	V = rad*(omega_0 + domega_dr*rad);

	vel[2] = 0.0;
	if (coords[0]-x0 >= 0.0 && coords[1]-y0 >= 0.0) /*1st quadrant*/ 
	{
	    vel[0] = -V*xcomp;
	    vel[1] =  V*ycomp;
	}
	else if (coords[0]-x0 <= 0.0 && coords[1]-y0 >= 0.0) /*2nd quadrant*/ 
	{
	    vel[0] = -V*xcomp;
	    vel[1] = -V*ycomp;
	}
	else if (coords[0]-x0 <= 0.0 && coords[1]-y0 <= 0.0) /*3rd quadrant*/ 
	{
	    vel[0] =  V*xcomp;
	    vel[1] = -V*ycomp;
	}
	else if (coords[0]-x0 >= 0.0 && coords[1]-y0 <= 0.0) /*4th quadrant*/ 
	{
	    vel[0] =  V*xcomp;
	    vel[1] =  V*ycomp;
	}
}	/* end shear_motion_vel */

EXPORT int normal_vel(
	POINTER params,
	Front *front,
	FT_POINT *p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF *hs,
	double *vel)
{
	NORV_PARAMS *norv_params;
	int i;
	double coeff;
	double curvature;
	double nor[MAXD];

	norv_params = (NORV_PARAMS*)params;
	coeff = norv_params->coeff;

	normal(p,hse,hs,nor,front);
	for (i = 0; i < norv_params->dim; ++i)
	{
	    vel[i] = nor[i]*coeff;
	}
}	/* end normal_vel */

EXPORT int curvature_vel(
	POINTER params,
	Front *front,
	FT_POINT *p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF *hs,
	double *vel)
{
	NORV_PARAMS *norv_params;
	int i;
	double coeff,epsilon;
	double kappa;
	double nor[MAXD];

	norv_params = (NORV_PARAMS*)params;
	coeff = norv_params->coeff;
	epsilon = norv_params->epsilon;

	normal(p,hse,hs,nor,front);
	kappa = mean_curvature_at_point(p,hse,hs,front);
	for (i = 0; i < norv_params->dim; ++i)
	{
	    vel[i] = nor[i]*(coeff - epsilon*kappa);
	}
}	/* end curvature_vel */

EXPORT int flame_vel(
	POINTER params,
	Front *front,
	FT_POINT *p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF *hs,
	double *vel)
{
        FLAME_PARAMS *flame_params;
        CIRCULAR_ROTATION_PARAMS *c_params;
        int problem_type;
        int i;
        double a, b;
        double theta, nor[MAXD], wind[MAXD];
        double ssin,scos;
        double speed;
        double alpha, U, c, m, epsilon;
        double t,temp;
        flame_params = (FLAME_PARAMS*)params;
        U = flame_params->U;
        c = flame_params->c;
        m = flame_params->m;
        alpha = flame_params->alpha;
        epsilon = flame_params->epsilon;
        wind[0] = flame_params->wind[0];
        wind[1] = flame_params->wind[1];
        problem_type = flame_params->problem_type;
        a = flame_params->a;
        b = flame_params->b;
        if(problem_type == 1)
        {
        c_params = (CIRCULAR_ROTATION_PARAMS*)flame_params->stuff;
        circular_rotation_vel(c_params,front,p,hse,hs,wind);
        }

        if(problem_type == 2)
        {
        t=front->time;
        wind[0]=a*t+wind[0];
        wind[1]=b*t+wind[1];
        }

        temp=sqrt(pow(wind[0],2)+pow(wind[1],2));
        normal(p,hse,hs,nor,front);
        nor[0] = nor[0];
        nor[1] = nor[1];
        scos=(wind[0]*nor[0]+wind[1]*nor[1])/temp;
        theta = acos(scos);
        ssin=sqrt(1-pow(scos,2));
        if(isgreater(M_PI/2,theta))
                speed = epsilon + c*sqrt(U)*pow(fabs(scos),m);
        else
            speed = epsilon*(alpha + (1.0 - alpha)*fabs(ssin));
        vel[0] = speed*nor[0];
        vel[1] = speed*nor[1];

}	/* end flame_vel */

EXPORT int bipolar_vel(
	POINTER params,
	Front *front,
	FT_POINT *p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF *hs,
	double *vel)
{
	BIPOLAR_PARAMS *bipolar_params = (BIPOLAR_PARAMS*)params;
	double *coords = Coords(p);
	double d1,d2;
	double s1,s2;
	double *cen1 = bipolar_params->cen1;
	double *cen2 = bipolar_params->cen2;
	double dx1,dy1;
	double dx2,dy2;
	double rev_time = bipolar_params->reverse_time;

	dx1 = coords[0] - cen1[0]; 
	dy1 = coords[1] - cen1[1];
	dx2 = coords[0] - cen2[0]; 
	dy2 = coords[1] - cen2[1];

	d1 = sqrt(sqr(dx1) + sqr(dy1));
	d2 = sqrt(sqr(dx2) + sqr(dy2));

	s1 = bipolar_params->i1/2.0/PI/d1;
	s2 = bipolar_params->i2/2.0/PI/d2;

	if ( rev_time >= 0.0 &&front->time > rev_time)
	{
	    s1 *= -1;
	    s2 *= -1;
	}

	vel[0] =  s1*dy1/d1 + s2*dy2/d2;
	vel[1] = -s1*dx1/d1 - s2*dx2/d2;
}	/* end bipolar_vel */

EXPORT int vortex_vel(
        POINTER params,
        Front *front,
        FT_POINT *p,
        HYPER_SURF_ELEMENT *hse,
        HYPER_SURF *hs,
        double *vel)
{
        VORTEX_PARAMS *vortex_params;
        int i, dim;
        double coeff,coeff2,xtemp,ytemp;
        char *type;
        double *coords = Coords(p);
	double x,y,z;
        vortex_params = (VORTEX_PARAMS*)params;

	dim = vortex_params->dim;
        type = vortex_params->type;

        coeff2 = 1.0;
        if(vortex_params->time > 0 && front->time >= vortex_params->time)
            coeff2 = -1.0;
        if(vortex_params->cos_time > 0 )
            coeff2 = cos((front->time*PI)/vortex_params->cos_time);
	x = coords[0];
	y = coords[1];
	if (dim == 3)
	    z = coords[2];

	if (dim == 2)
	{
	    switch(type[0])
	    {
	    case 'm':
	    case 'M':
		/*four vortex work */
		xtemp = 4*PI*(x+0.5);
		ytemp = 4*PI*(y+0.5);
		vel[0] = coeff2*sin(xtemp)*sin(ytemp); 
		vel[1] = coeff2*cos(xtemp)*cos(ytemp); 
		/*end four vortex work */
		break;
	    case 's':
	    case 'S':
		/*2D single vortex motion */
		vel[0] = -coeff2*sin(PI*x)*sin(PI*x)*sin(2*PI*y);
		vel[1] = coeff2*sin(2*PI*x)*sin(PI*y)*sin(PI*y);
		break;
	    default:
		screen("Undefined vortex type!\n");
		clean_up(ERROR);
	    }
	}
	else if (dim == 3)
	{
	    switch(type[0])
	    {
	    case 'm':
	    case 'M':
		/*double vortex work */
	        vel[0] = coeff2*2*sin(PI*x)*sin(PI*x)*sin(2*PI*y)*sin(2*PI*z);
	        vel[1] = -coeff2*sin(2*PI*x)*sin(PI*y)*sin(PI*y)*sin(2*PI*z);
	        vel[2] = -coeff2*sin(2*PI*x)*sin(2*PI*y)*sin(PI*z)*sin(PI*z);
		break;
	    case 's':
	    case 'S':
		/*shearing flow motion */
		vel[0] = coeff2*sin(PI*x)*sin(PI*x)*sin(2*PI*y);
		vel[1] = -coeff2*sin(2*PI*x)*sin(PI*y)*sin(PI*y);
		vel[2] = coeff2*sqr(1-2*sqrt((x-0.5)*(x-0.5)+(y-0.5)*(y-0.5)));
		break;
	    default:
	        screen("Undefined time dependency type!\n");
	        clean_up(ERROR);
	    }
	}    
}       /* end vortex_vel */

EXPORT int double_vortex_vel(
        POINTER params,
        Front *front,
        FT_POINT *p,
        HYPER_SURF_ELEMENT *hse,
        HYPER_SURF *hs,
        double *vel)
{
        BIPOLAR_PARAMS *dv_params = (BIPOLAR_PARAMS*)params;
        double *coords = Coords(p);
        double d1,d2;
        double s1,s2;
        double *cen1 = dv_params->cen1;
        double *cen2 = dv_params->cen2;
        double dx1,dy1;
        double dx2,dy2;

        dx1 = coords[0] - cen1[0];
        dy1 = coords[1] - cen1[1];
        dx2 = coords[0] - cen2[0];
        dy2 = coords[1] - cen2[1];

        d1 = sqrt(sqr(dx1) + sqr(dy1));
        d2 = sqrt(sqr(dx2) + sqr(dy2));

        s1 = dv_params->i1/2.0/PI/d1;
        s2 = dv_params->i2/2.0/PI/d2;

        vel[0] =  s1*dy1/d1 + s2*dy2/d2;
        vel[1] = -s1*dx1/d1 - s2*dx2/d2;
	vel[1] -= 0.3;
}       /* end double_vortex_vel */

EXPORT void FT_InitVeloFunc(
	Front *front,
	VELO_FUNC_PACK *velo_func_pack)
{
	char s[100];
	int dim = front->rect_grid->dim;

	/* Initialize front velocity field */

	if (velo_func_pack == NULL)
	{
	    screen("\n\t\tSpecifying Velocity Field\n\n");
	    screen("Supported velocity fields are \n"
	       "\tTranslation (t)\n"
	       "\tRadial motion (r)\n"
	       "\tShear motion (s)\n"
	       "\tSinusiodal motion (w)\n"
	       "\tCircular rotation (c)\n"
	       "\tNormal motion (n)\n"
	       "\tCurvature dependent motion (k)\n"
	       "\tFlame motion (f)\n"
	       "\tBurgers equation solver (b)\n"
	       "\tBi-Polar velocity (h)\n"
	       "\tVortex velocity (v)\n"
	       "\tDouble vortex velocity (d)\n"
	       "Enter choice: ");
	    (void) Scanf("%s\n",s);
	    switch (s[0])
	    {
	    case 'T':
	    case 't':
	    	front->vfunc = translation_vel;
	    	init_translation_params(&front->vparams,dim);
	    	front->_point_propagate = fourth_order_point_propagate;
	    	break;
	    case 'R':
	    case 'r':
	    	front->vfunc = radial_motion_vel;
	    	init_radial_motion_params(&front->vparams,dim);
	    	front->_point_propagate = fourth_order_point_propagate;
	    	break;
	    case 'S':
	    case 's':
	    	front->vfunc = shear_motion_vel;
	    	init_shear_motion_params(&front->vparams,dim);
	    	front->_point_propagate = fourth_order_point_propagate;
	    	break;
	    case 'W':
	    case 'w':
	    	front->vfunc = sine_motion_vel;
	    	init_sine_motion_params(&front->vparams,dim);
	    	front->_point_propagate = fourth_order_point_propagate;
	    break;
	    case 'C':
	    case 'c':
	    	front->vfunc = circular_rotation_vel;
	    	init_circular_rotation_params(&front->vparams,dim);
	    	front->_point_propagate = fourth_order_point_propagate;
	    	break;
	    case 'B':
	    case 'b':
	    	front->vfunc = burgers_vel;
	    	init_burgers_params(&front->vparams,dim);
	    	front->_point_propagate = fourth_order_point_propagate;
	    break;
	    case 'H':
	    case 'h':
	    	front->vfunc = bipolar_vel;
	    	init_bipolar_params(&front->vparams,dim);
	    	front->_point_propagate = fourth_order_point_propagate;
	    	break;
	    case 'N':
	    case 'n':
	    	front->vfunc = normal_vel;
	    	init_norv_params(&front->vparams,dim);
	    	front->_point_propagate = first_order_point_propagate;
	    	break;
	    case 'K':
	    case 'k':
	    	front->vfunc = curvature_vel;
	    	init_curvature_params(&front->vparams,dim);
	    	front->_point_propagate = first_order_point_propagate;
	    	break;
	    case 'F':
	    case 'f':
	    	front->vfunc = flame_vel;
	    	init_flame_params(&front->vparams,dim);
	    	front->_point_propagate = first_order_point_propagate;
	    	break;
	    case 'V':
	    case 'v':
	    	front->vfunc = vortex_vel;
	    	init_vortex_params(&front->vparams,dim);
	    	front->_point_propagate = fourth_order_point_propagate;
	    	break;
	    case 'D':
	    case 'd':
	    	front->vfunc = double_vortex_vel;
	    	init_bipolar_params(&front->vparams,dim);
	    	front->_point_propagate = fourth_order_point_propagate;
	    	break;
	    default:
	    	screen("Unsupported velocity field for first "	
		       "order point propagation\n");
	    	clean_up(ERROR);
	    }
	}
	else
	{
	    front->vfunc = velo_func_pack->func;
	    front->vparams = velo_func_pack->func_params;
	    if (velo_func_pack->point_propagate != NULL)
	    	front->_point_propagate = velo_func_pack->point_propagate;
	    else
	    	front->_point_propagate = first_order_point_propagate;
	}
}	/* end FT_InitVeloFunc_function */

