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
*				geomutils.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Contains elementary functions for the computation of geometry.
*/

#include <intfc/geom.h>

EXPORT	double	EPSILON	= 1.e-4;	/* TOLERANCE */

	/* LOCAL Function Prototypes */

/*
*			vector_product_on_points():
*
*	This routine computes the vector product in two or three dimensions.
*	The answer is placed in Pout and is a vector for three dimensions and
*	a single double (scalar) in two dimensions. Po is the origin of 
*	coordinates for the uni_arrays P1 and P2. Thus Pout = (P1 - Po)x(P2 - Po).
* 	The returned value is 0 for two dimensions and in three diemsnions is 
*       the area of the triangle formed by these two uni_arrays.
*
*	BUG:  The values returned by this function are inconsistent
*	in 2 and 3 dimensions.
*/

EXPORT 	double 	vector_product_on_points(
	const double *p0,
	const double *p1,
	const double *p2,
	int         dim,
	double       *Pout)
{
	double    vect1[3],vect2[3];
	double    area;
	int    i;

	for (i = 0; i < dim; ++i)
	{
	    vect1[i] = p1[i] - p0[i];
	    vect2[i] = p2[i] - p0[i];
	}

	switch (dim)
	{
	case 2:
	    *Pout = vect1[0]*vect2[1]-vect1[1]*vect2[0] ;
	    area = fabs(*Pout);
	    break;
	case 3:
	    Pout[0] = vect1[1]*vect2[2]-vect1[2]*vect2[1];
	    Pout[1] = vect1[2]*vect2[0]-vect1[0]*vect2[2];
	    Pout[2] = vect1[0]*vect2[1]-vect1[1]*vect2[0];
	    area = 0.5 * sqrt( sqr(Pout[0]) + sqr(Pout[1]) + sqr(Pout[2]));
	    break;
	default:
	    area = ERROR_FLOAT;
	}
	return area;
}		/*end vector_product_on_points*/


EXPORT	double vector_product(
	const double *v1,
	const double *v2,
	double       *vprod,
	int         dim)
{
	double    magsqr;
	double    vtmp[3];
	int      i, j, k;

	if (vprod == NULL)
	    vprod = vtmp;
	switch (dim)
	{
	case 1:
	    vprod[0] = 0.0;
	    return fabs(*vprod);
	case 2:
	    vprod[0] = v1[0]*v2[1] - v1[1]*v2[0];
	    return fabs(*vprod);
	case 3:	/* vprod = v1 X v2  right-hand rule */
	    magsqr = 0.0;
	    for (i = 0; i < 3; ++i)
	    {
	        j = Next_m3(i);    k = Prev_m3(i);
	        vprod[i] = v1[j]*v2[k] - v1[k]*v2[j];
	        magsqr += vprod[i]*vprod[i];
	    }
	    return sqrt(magsqr);
	}
	return ERROR_FLOAT;
}		/*end vector_product*/

EXPORT	double scalar_product(
	const double	*v1,
	const double	*v2,
	const int	dim)
{
	double    scalar_prod = 0.0;
	int      i;
	
	for (i = 0; i < dim; ++i)
	    scalar_prod += v1[i]*v2[i];
	return scalar_prod;
}		/*end scalar_product*/

EXPORT double triple_product(
	const double *v1,
	const double *v2,
	const double *v3,
	int         dim)
{
	double    v[3];
	double    cpd;

	(void) vector_product(v1,v2,v,dim);
	cpd = scalar_product(v,v3,dim);
	return cpd;
}		/*end triple_product*/


EXPORT	double mag_vector(
	const double *v,
	int         dim)
{
	int i;
	double sp;

	switch (dim)
	{
	case 1:
	    return fabs(v[0]);
	case 2:
	case 3:
	    sp = 0.0;
	    for (i = 0; i < dim; ++i)
	        sp += v[i]*v[i];
	    return sqrt(sp);
	}
	return 0.0;
}		/*end mag_vector*/

/*
*		outside_point():
*
*	Determines which of the 27 segments the given p lies in
*	with respect to the rectangle with lower corner L and upper
*	corner U.
*
*		     |         |
*		  7  |    6    |  8
*		     |         |
*		-----+---------+-----
*		     |         |
*		  1  |    0    |  2 
*		     |         |
*		-----+---------+-----
*		     |         |
*		  4  |    3    |  5
*		     |         |
*
*	outside_point() returns
*		0    if point `p' lies inside domain 0
*	      1 -> 8 otherwise
*
*/

EXPORT	int outside_point(
	const double *p,
	const double *L,
	const double *U,
	int         dim)
{
	int    i, p_out;
	int    n;

	for (p_out = 0, i = 0, n = 1;  i < dim;  ++i, n *= 3)
	{
	    p_out += ((p[i] < L[i]) ?   n : 0);
	    p_out += ((p[i] > U[i]) ? 2*n : 0);
	}
	return p_out;
}	/* end outside_point */


EXPORT	double	distance_between_positions(
	const double    *p,
	const double    *q,
	int            dim)
{
	int    i;
	double  d, sep;

	sep = 0.0;
	for (i = 0; i < dim; ++i)
	{
	    d = p[i] - q[i];
	    sep += sqr(d);
	}
	return sqrt(sep);
}		/*end distance_between_positions*/

EXPORT	double dscalar_product(
	const double *v1,
	const double *v2,
	int          dim)
{
	double    scalar_prod = 0.0;
	int       i;
	
	for (i = 0; i < dim; ++i)
	    scalar_prod += v1[i]*v2[i];
	return scalar_prod;
}		/*end dscalar_product*/


EXPORT	double	grid_size_in_direction(
	const double *t,
	const double *h,
	int         dim)
{
	double    ds;

	ds = 1.0/scaled_hypot(t,h,dim);
	return ds;
}		/*end grid_size_in_direction*/

EXPORT	double scaled_hypot(
	const double *p,
	const double *h,
	int         dim)
{
	double    tmp, ans = 0.0;
	int      i;

	for (i = 0; i < dim; ++i) 
	{
	    tmp = p[i]/h[i];
	    ans += sqr(tmp);
	}
	return sqrt(ans);
}		/*end scaled_hypot*/

EXPORT	double dscaled_hypot(
	const double *p,
	const double *h,
	int          dim)
{
	double    tmp, ans = 0.0;
	int       i;

	for (i = 0; i < dim; ++i) 
	{
	    tmp = p[i]/h[i];
	    ans += sqr(tmp);
	}
	return sqrt(ans);
}		/*end dscaled_hpot*/

EXPORT	double _scaled_separation(
	const double *p,
	const double *q,
	const double *h,
	int         dim)
{
	double    tmp, ans = 0.0;
	int      i;

	for (i = 0; i < dim; ++i) 
	{
	    tmp = (p[i] - q[i])/h[i];
	    ans += sqr(tmp);
	}
	return sqrt(ans);
}		/*end _scaled_separation*/


/*
*			cal_angle():
*
*	Finds the angle between the two uni_arrays a = p1 - p2 and
*	b = p3 - p2.  If branch_cut == PLUS_CUT, the angle is normalized to 
*       lie in the interval 0 <= angle < 2.0*PI.  Otherwise, the angle is 
*       normalized to lie in the interval -PI <= angle < PI.  
*/


EXPORT 	double cal_angle(
	const double *p0,
	const double *p1,
	const double *p2,
	int         branch_cut,
	int         dim,
	const double *nor)
{
	double    v0[3], v1[3];
	double    C[3],u,v,prod;
	int      i;

	for (i = 0; i < dim; ++i)
	{
	    v0[i] = p2[i] - p1[i];
	    v1[i] = p0[i] - p1[i];
	}

	u = scalar_product(v0,v1,dim);
	v = vector_product(v0,v1,C,dim);

	switch (dim)
	{
	case 2:
	    v = C[0];
	    break;
	case 3:
	    prod = scalar_product(C,nor,dim);
	    if (fabs(prod) < .9*v)
	    {
	    	(void) printf("ERROR in cal_angle(), ");
	    	(void) printf("Inconsistant normal direction\n");
	        print_general_vector("nor = ",nor,dim,"");
	        print_general_vector("C = ",C,dim,"\n");
	        print_general_vector("p0 = ",p0,dim,"");
	        print_general_vector("p1 = ",p1,dim,"");
	        print_general_vector("p2 = ",p2,dim,"\n");
	    	(void) printf("v = %g, c.n = %g\n",v,prod);
	    	clean_up(ERROR);
	    }
	    if(prod < 0.0)	v = -v;
	    break;
	default:
	    (void) printf("ERROR in cal_angle(), dim = %d",dim);
	    clean_up(ERROR);
	}

	return (branch_cut == PLUS_CUT) ? angle(u,v) : atan2(v,u);
}		/*end cal_angle*/


/*
*          		 normalized_angle():
*
*       Normalize the given angle to be between 0 and 2*PI.
*/

EXPORT  double  normalized_angle(
	double    ang)
{
	int    d;
	static	double	twopi = 2.0*PI;

	while (ang < 0.0)
	    ang += twopi;
	d = (int)(ang/twopi);
	return (ang - d*twopi);
}		/*end normalized_angle*/


/*
*			avg_angle_and_normalize()
*
*	Average two angles,  where the average is defined as the
*	directional angle of the average of the two unit uni_arrays
*	with angle directions a1 and a2;
*
*	tan(avg_ang) = (sin(a1) + sin(a2))/(cos(a1) + cos(a2))
*/

EXPORT	double avg_angle_and_normalize(
	double    a1,
	double    a2)
{
	double    ang;

	a1 = normalized_angle(a1);
	a2 = normalized_angle(a2);
	ang = fabs(a1 - a2);
	return normalized_angle( 0.5*(a1+a2) + ((ang > PI) ? PI : 0.0));
}		/*end avg_angle_and_normalize*/


/*
*		            angle():
*	Returns the normalized angle (from 0 to 2*PI) of the given uni_array,
*	defined as the counterclockwise (positive) angle from the positive
*	x axis to the given uni_array.
*
*/

EXPORT  double  angle(
	double    vx,
	double    vy)
{
	double    ang;
	static	double	twopi = 2.0*PI;

#if defined(cray)
	ang = (vx == 0.0 && vy == 0.0) ? 0.0 : atan2(vy,vx);
#else /* defined(cray) */
	ang = atan2(vy,vx);
#endif /* defined(cray) */
	return (ang < 0.0) ? (ang + twopi) : ang;
}		/*end angle*/

/*
*		is_new_angle_smaller():
*/

EXPORT int is_new_angle_smaller(
	double    _sin,
	double    _cos,
	double    oldsin,
	double    oldcos,
	int      dir)
{
	int    quadrant, oldquadrant;
	int    ans = NO;

	quadrant    = (  _sin > 0.) ?
	        	(  _cos > 0. ? 1 : 2) : (  _cos > 0. ? 4 : 3);
	oldquadrant = (oldsin > 0.) ?
	        	(oldcos > 0. ? 1 : 2) : (oldcos > 0. ? 4 : 3);
	if (quadrant < oldquadrant)
	    ans = YES;
	else if (oldquadrant == quadrant && 
	    	(quadrant == 1 || quadrant == 4) && _sin < oldsin)
	    ans = YES;
	else if (oldquadrant == quadrant &&
	    	(quadrant == 2 || quadrant == 3) && _sin > oldsin)
	    ans = YES;
	if (dir == CLOCKWISE)
	    ans = (ans == YES) ? NO : YES;
	return ans;
}		/*end is_new_angle_smaller*/


EXPORT void print_angle_direction(
	const char      *message,
	ANGLE_DIRECTION	ang_dir,
	const char      *end)
{
	fprint_angle_direction(stdout,message,ang_dir,end);
}		/*end print_angle_direction*/


EXPORT void fprint_angle_direction(
	FILE            *file,
	const char      *mesg,
	ANGLE_DIRECTION	ang_dir,
	const char      *end)
{
	(void) fprintf(file,"%s %s%s",mesg,angle_direction_name(ang_dir),end);
}		/*end fprint_angle_direction*/

EXPORT	const char *angle_direction_name(
	ANGLE_DIRECTION ang_dir)
{
	switch (ang_dir)
	{
	case COUNTER_CLOCK:
	    return "COUNTER_CLOCK";
	case CLOCKWISE:
	    return "CLOCKWISE";
	case ANGLE_DIRECTION_NOT_SET:
	    return "ANGLE_DIRECTION_NOT_SET";
	default:
	    screen("ERROR in angle_direction_name(), "
	           "invalid angle direction %d\n",ang_dir);
	    clean_up(ERROR);
	}
	return "ANGLE_DIRECTION_NOT_SET";
}		/*end angle_direction_name*/

EXPORT void print_orientation(
	const char    *message,
	ORIENTATION   orient,
	const char    *end)
{
	fprint_orientation(stdout,message,orient,end);
}		/*end print_orientation*/

EXPORT void fprint_orientation(
	FILE           *file,
	const char     *message,
	ORIENTATION    orient,
	const char     *end)
{
	(void) fprintf(file,"%s %s%s",message,orientation_name(orient),end);
}		/*end fprint_orientation*/

EXPORT const char *orientation_name(
	ORIENTATION    orient)
{
	switch (orient)
	{
	case POSITIVE_ORIENTATION:
	    return "POSITIVE_ORIENTATION";
	case NEGATIVE_ORIENTATION:
	    return "NEGATIVE_ORIENTATION";
	case ORIENTATION_NOT_SET:
	    return "ORIENTATION_NOT_SET";
	default:
	    screen("ERROR in orientation_name(), "
	           "invalid orientation %d\n",orient);
	    clean_up(ERROR);
	}
	return "ORIENTATION_NOT_SET";
}		/*end orientation_name*/

EXPORT	SIDE Opposite_side(
	const SIDE side)
{
	switch (side)
	{
	case NEGATIVE_SIDE:
	    return POSITIVE_SIDE;
	case POSITIVE_SIDE:
	    return NEGATIVE_SIDE;
	case ONEDGE:
	    return ONEDGE;
	case UNKNOWN_SIDE:
	default:
	    screen("ERROR in Opposite_side(), invalid side\n");
	    clean_up(ERROR);
	    return side;
	}
}		/*end Opposite_side*/

EXPORT void print_side(
	const char *message,
	SIDE       side,
	const char *end)
{
	(void) printf("%s %s%s",message,side_name(side),end);
}		/*end print_side*/

EXPORT const char *side_name(
	SIDE       side)
{
	switch (side)
	{
	case NEGATIVE_SIDE:
	    return "NEGATIVE_SIDE";
	case POSITIVE_SIDE:
	    return "POSITIVE_SIDE";
	case ONEDGE:
	    return "ONEDGE";
	case ONVERTEX:
	    return "ONVERTEX";
	case COPLANAR:
	    return "COPLANAR";
	case UNKNOWN_SIDE:
	    return "UNKNOWN_SIDE";
	default:
	    screen("ERROR in side_name(), invalid side %d\n",side);
	    clean_up(ERROR);
	}
	return "UNKNOWN_SIDE";
}		/*end side_name*/

EXPORT	void	print_general_vector(
	const char  *mesg,
	const double *v,
	int	    dim,
	const char  *end)
{
	fprint_general_vector(stdout,mesg,v,dim,end);
}		/*end print_general_vector*/

EXPORT	void fprint_general_vector(
	FILE        *file,
	const char  *mesg,
	const double *v,
	int         dim,
	const char  *end)
{
	int    i;

	if (mesg != NULL)
	    (void) fprintf(file,"%s",mesg);
	(void) fprintf(file,"(");
	for (i = 0; i < dim; ++i)
	    (void) fprintf(file,"%"FFMT"%s",v[i],(i==(dim-1)) ? ")" : ", ");
	if (end != NULL)
	    (void) fprintf(file,"%s",end);
}		/*end fprint_general_vector*/

EXPORT	void sprint_general_vector(
	char        *s,
	const char  *mesg,
	const double *v,
	int         dim,
	const char  *end)
{
	int    i;

	if (mesg == NULL)
	    (void) strcpy(s,"(");
	else
	    (void) sprintf(s,"%s (",mesg);
	for (i = 0; i < dim; ++i)
	    (void) sprintf(s+strlen(s),"%"FFMT"%s",v[i],(i==(dim-1)) ? ")" : ", ");
	if (end != NULL)
	    (void) strcat(s,end);
}		/*end sprint_general_vector*/

/*
*			random_gaussian():
*
*	Returns a pseudo-independent Gaussian random number.
*/

EXPORT	double random_gaussian(
	double    mu,
	double    sigma,
	unsigned short int xsubi[3])
{
	double    t, z;
	int      num_iter;
	static double	alpha, sqrt2, erf1, logsqrtpi;
	static boolean	first = YES;

	if (first == YES)
	{
	    first = NO;
	    alpha = 2.0/sqrt(PI);
	    sqrt2 = sqrt(2.0);
	    logsqrtpi = 0.5*log(PI);
	    erf1 = erf(1.0);
	}
	t = 2.0*erand48(xsubi) - 1.0;
	if (t > erf1)
	    z =  sqrt(-logsqrtpi - log(1.0-t));
	else if (t < -erf1)
	    z = -sqrt(-logsqrtpi - log(1.0+t));
	else
	    z = t/alpha;

	for (num_iter=0; (num_iter<10) && (fabs(t-erf(z))>EPSILON); ++num_iter)
	{
	    z += (t - erf(z))/(alpha*exp(-z*z));
	}
	return sqrt2*sigma*z + mu;
}		/*end random_gaussian*/

EXPORT	ANGLE_DIRECTION	fread_angle_direction(
	FILE* file)
{
	int i;
	(void) fscanf(file,"%d",&i);
	switch (i)
	{
	case CLOCKWISE:
	    return CLOCKWISE;
	case COUNTER_CLOCK:
	    return COUNTER_CLOCK;
	case ANGLE_DIRECTION_NOT_SET:
	default:
	    return ANGLE_DIRECTION_NOT_SET;
	}
}		/*end fread_angle_direction*/

/*
*			affine_fit():
*
*	Determines the least squares affine fit for the set of points. This
*	is defined as the affine set r*(p-p0) = 0 whose distances from the
*	set of points is an extremum. Using Lagrange multipliers this
*	corresponds to finding the extrema of the functional:
*
*	F(r,lambda,p0) = -lambda*(<r,r> - 1) + (1/N)0.5*Sum_0^N sqr(<r,p_i-p0>)
*
*	where <.,.> is the standard Euclidean inner product. Assume the points
*	p are represented as column uni_arrays, the solution
*	is p0 = pbar = mean position of the points p, and
*	r an eigenvalue of the bi_array
*
*                                                       T
*		a = (1/N)Sum_0^N (p_i - pbar)(p_i - pbar)
*
*	with eigenvalue lambda. The best affine fit thus corresponds to the
*	eigenuni_array corresponding to the minimum eigenvalue of a.
*
*
*	Input:
*		p    = components of the set of points to be fit
*		dim  = dimension of the space for the p's
*		N    = number of points to be fit
*		ndir = a normalizing vector chosen so that resulting normal
*		       minium eigenvalue eigenuni_array has a positive component
*		       in the direction ndir.
*
*	Ouput:
*		pbar = average of the input points
*		r    = a matrix of eigenuni_arrays of the matrix a above
*		       sorted according to non-increasing eigenvalues
*		lambda = eigenvalues corresponding to the order of r.
*/

EXPORT	void affine_fit(
	const double* const  *p,
	int                 dim,
	int                 N,
	const double         *ndir,
	double               *pbar,
	double               **r,
	double               *lambda)
{
	double            sm, tresh, h, t, theta;
	double            c, s, tau, g;
	double            a[3][3], v[3][3];
	double            ll[3], b[3], z[3];
	double            sp, det;
	int              i, j, k, l, m[3];
	static const int MAXITER = 50;

	debug_print("affine","Entered affine_fit()\n");
	if ((dim < 2) || (dim > 3))
	{
	    screen("ERROR in affine_fit(), dim = %d not supported\n",dim);
	    clean_up(ERROR);
	}

	for (j = 0; j < dim; ++j)
	{
	    for (pbar[j] = 0.0, i = 0; i < N; ++i)
	        pbar[j] += p[i][j];
	    pbar[j] /= N;
	}
	for (i = 0; i < dim; ++i)
	{
	    for (j = 0; j <= i; ++j)
	    {
	        for (a[i][j] = 0.0, k = 0; k < N; ++k)
	            a[i][j] += (p[k][i] - pbar[i])*(p[k][j] - pbar[j]);
	        a[i][j] /= N;
	    }
	}
	for (i = 0; i < dim; ++i)
	    for (j = i+1; j < dim; ++j)
	        a[i][j] = a[j][i];

	for (i = 0; i < dim; ++i)
	{
	    for (j = 0; j < dim; ++j)
	        v[i][j] = 0.0;
	    v[i][i] = 1.0;
	    b[i] = ll[i] = a[i][i];
	    z[i] = 0.0;
	}
	for (k = 0; k < MAXITER; ++k)
	{
	    sm = fabs(a[0][1]) + fabs(a[0][2]) + fabs(a[1][2]);
	    if (sm == 0.0)
	        break;
	    tresh = (k < 3) ? 0.2*sm/9.0 : 0.0;
	    for (i = 0; i < 2; ++i)
	    {
		for (j = i+1; j < dim; ++j)
		{
	            g = 100.0*fabs(a[i][j]);
	            if ((k > 3) && ((fabs(ll[i])+g) == fabs(ll[i]))
	            	        && ((fabs(ll[j])+g) == fabs(ll[j])))
	            	a[i][j] = 0.0;
	            else if (fabs(a[i][j]) > tresh)
		    {
	            	h = ll[j] - ll[i];
	                if ((fabs(h)+g) == fabs(h))
	            	    t = a[i][j]/h;
	     	        else
			{
	    		    theta = 0.5*h/a[i][j];
	    		    t = 1.0/(fabs(theta) + sqrt(1.0 + theta*theta));
	    		    if (theta < 0.0)
				t = -t;
	    		}
	    		c = 1.0/sqrt(1.0 + t*t);
	    		s = t*c;
	    		tau = s/(1.0 + c);
	    		h = t*a[i][j];
	    		z[i] -= h;
	    		z[j] += h;
	    		ll[i] -= h;
	    		ll[j] += h;
	    		a[i][j] = 0.0;
	    		for (l = 0; l < i-1; ++l)
			{
	                    g = a[l][i];
			    h = a[l][j];
			    a[l][i] = g - s*(h + g*tau);
			    a[l][j] = h + s*(g - h*tau);
	    		}
	    		for (l = i+1; l < j-1; ++l)
			{
	                    g = a[i][l];
			    h = a[l][j];
			    a[i][l] = g - s*(h + g*tau);
			    a[l][j] = h + s*(g - h*tau);
	    		}
	    		for (l = j+1; l < dim; ++l)
			{
	                    g = a[i][l];
			    h = a[j][l];
			    a[i][l] = g - s*(h + g*tau);
			    a[j][l] = h + s*(g - h*tau);
	    		}
	    		for (l = 0; l < dim; ++l)
			{
			    g = v[l][i];
			    h = v[l][j];
			    v[l][i] = g - s*(h + g*tau);
			    v[l][j] = h + s*(g - h*tau);
	    		}
	    	    }
		}
	    }
	    for (i = 0; i < dim; ++i)
	    {
		b[i] += z[i];
		ll[i] = b[i];
		z[i] = 0.0;
	    }
	}
	for (i = 0; i < dim; ++i)
	    m[i] = i;
	for (i = 0; i < dim; ++i)
	{
	    for (j = i+1; j < dim; ++j)
	    {
		if (ll[m[i]] < ll[m[j]])
		{
		    k = m[i];
		    m[i] = m[j];
		    m[j] = k;
		}
	    }
	}
	for (i = 0; i < dim; ++i)
	{
	    lambda[i] = ll[m[i]];
	    for (j = 0; j < dim; ++j)
	        a[i][j] = v[j][m[i]];
	    for (; j < 3; ++j)
		a[i][j] = 0.0;
	   
	}
	for (; i < 3; ++i)
	{
	    for (j = 0; j < 3; ++j)
		a[i][j] = 0.0;
	    a[i][i] = 1.0;
	}
	for (sp = 0.0, i = 0; i < dim; ++i)
	    sp += a[dim-1][i]*ndir[i];
	if (sp < 0.0)
	{
	    for (i = 0; i < dim; ++i)
		a[dim-1][i] *= -1.0;
	}
	det = a[0][0]*a[1][1]*a[2][2] +
	      a[0][1]*a[1][2]*a[2][0] +
	      a[0][2]*a[1][0]*a[2][1] -
	      a[0][2]*a[1][1]*a[2][0] -
	      a[0][1]*a[1][0]*a[2][2] -
	      a[0][0]*a[1][2]*a[2][1];
	if (det < 0.0)
	{
	    for (i = 0; i < 3; ++i)
		a[0][i] *= -1.0;
	}
	for (i = 0; i < dim; ++i)
	{
	    for (j = 0; j < dim; ++j)
	        r[i][j] = a[i][j];
	}
	if (debugging("affine"))
	{
	    double mag_ndir = mag_vector(ndir,dim);
	    double mag_r[3];
	    double v[3];

	    (void) print_general_vector("ndir = ",ndir,dim,", ");
	    (void) printf("magnitude = %g\n",mag_ndir);
	    for (i = 0; i < dim; ++i)
	    {
		mag_r[i] = mag_vector(r[i],dim);
		(void) printf("r[%d] = ",i);
		print_general_vector("",r[i],dim,", ");
	        (void) printf("magnitude = %g\n",mag_r[i]);
		(void) printf("<r[%d],ndir>/(|r[%d]|*|ndir|) = %g\n",i,i,
			      scalar_product(r[i],ndir,dim)/
			      (mag_ndir*mag_r[i]));
	    }
	    print_general_vector("lambda = ",lambda,dim,"\n");
	    for (i = 0; i < dim; ++i)
	    {
	        for (j = 0; j <= i; ++j)
	        {
	            for (a[i][j] = 0.0, k = 0; k < N; ++k)
	                a[i][j] += (p[k][i] - pbar[i])*(p[k][j] - pbar[j]);
	            a[i][j] /= N;
	        }
	    }
	    for (i = 0; i < dim; ++i)
	        for (j = i+1; j < dim; ++j)
	            a[i][j] = a[j][i];
	    for (i = 0; i < dim; ++i)
	    {
		for (j = 0; j < dim; ++j)
		    for (v[j] = 0, k = 0; k < dim; ++k)
			v[j] += a[j][k]*r[i][k];
		(void) printf("Correlation * r[%d] = ",i);
		print_general_vector("",v,dim,", ");
		for (j = 0; j < dim; ++j)
		    v[j] -= lambda[i]*r[i][j];
		(void) printf("|Correlation * r[%d] - "
			      "lambda[%d] * r[%d]| = %g\n",i,i,i,
			      mag_vector(v,dim));
	    }
	    if (dim == 3)
	    {
		int   in, ip;
		for (i = 0; i < 3; ++i)
		{
		    in = Next_m3(i);
		    ip = Prev_m3(i);
		    (void) vector_product(r[i],r[in],v,3);
		    (void) printf("r[%d] X r[%d] = ",i,in);
		    print_general_vector("",v,3,"\n");
		    (void) printf("|r[%d] X r[%d] - r[%d]| = %g\n",i,in,ip,
			          distance_between_positions(v,r[ip],3));
		}
	    }
	}
	debug_print("affine","Left affine_fit()\n");
}		/*end affine_fit*/

/*	
*	Calculate plane angle about the center at a coordinate.
*/

EXPORT	double plane_angle(
	double *cen,
	double *coords)
{
	double R,phi;
	double rcrds[2];
	rcrds[0] = coords[0] - cen[0];
	rcrds[1] = coords[1] - cen[1];
	R = mag_vector(rcrds,2);
        if (R == 0.0) return 0.0;

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
	return phi;
}	/* end plane_angle */
