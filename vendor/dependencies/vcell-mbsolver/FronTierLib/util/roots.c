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
*				roots.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Simple root finding programs for user defined functions.
*
*		bisection_find_root()
*		find_separation_point()
*		find_root()
*/

#include <cdecs.h>
#include <vmalloc.h>

enum {NUM_ITER = 50};

/*
*			bisection_find_root():
*
*	This routine finds a root of the equation y = f(x)
*	with x between x0 and x1 using the bisection method.
*
*	The solution if it exists will either satisfy
*	fabs(f(*px) - y) < fabs(epsilon) or there exists a root
*	with fabs(root - *px) < fabs(delta).
*/


EXPORT boolean bisection_find_root(
	boolean (*f)(double,double*,POINTER),
	POINTER f_params,
	double y,
	double *px,
	double x0,
	double x1,
	double epsilon,
	double delta)
{
	double a,b,xnew,fa,fb,fnew;
	double xmin, fxmin, s;
	int i;

#if defined(DEBUG_FIND_ROOT)
	if (debugging("find_root")) 
	{
		double fx0, fx1;

		(void) printf("Entering bisection_find_root()\n");
		if ( ((*f)(x0,&fx0,f_params)==FUNCTION_FAILED) ||
			((*f)(x1,&fx1,f_params)==FUNCTION_FAILED) )
		{
			(void) printf("WARNING in bisection_find_root(), ");
			(void) printf("Unable to evaluate function\n");
			return FUNCTION_FAILED;
		}
		(void) printf("y = %g\n",y);
		(void) printf("x0 = %g, f(%g) - %g = %g\n",x0,x0,y,fx0 - y);
		(void) printf("x1 = %g, f(%g) - %g = %g\n",x1,x1,y,fx1 - y);
	}
#endif /* defined(DEBUG_FIND_ROOT) */

	xmin = ERROR_FLOAT;
	delta = fabs(delta);
	epsilon = fabs(epsilon);
	a = x1;
	if ((*f)(a,&fa,f_params) == FUNCTION_FAILED)
	{
		(void) printf("WARNING in bisection_find_root(), ");
		(void) printf("Unable to evaluate function\n");
		return FUNCTION_FAILED;
	}
	xnew = a;
	fnew = fa;
	b = x0;
	if ((*f)(b,&fb,f_params) == FUNCTION_FAILED)
	{
		(void) printf("WARNING in bisection_find_root(), ");
		(void) printf("Unable to evaluate function\n");
		return FUNCTION_FAILED;
	}
	if (fabs(fb - y) <= epsilon)
	{
		*px = b;
		return FUNCTION_SUCCEEDED;
	}
	for (i = 0; i < NUM_ITER; ++i) 
	{
		if (fabs(b-a) <= delta || fabs(fnew - y) <= epsilon) 
		{
			if (fabs(fnew - y) <= epsilon)
				*px = xnew;
			else
			{
			    double xmid, fxmid, erra, errb, errmid;

			    xmid = 0.5*(a + b);
			    if ((*f)(xmid,&fxmid,f_params) == FUNCTION_FAILED)
			    {
				(void) printf("WARNING ");
				(void) printf("in bisection_find_root(), ");
				(void) printf("Unable to evaluate function\n");
				return FUNCTION_FAILED;
			    }
			    erra = fabs(fa - y);
			    errb = fabs(fb - y);
			    errmid = fabs(fxmid - y);
			    if (erra < min(errb,errmid))	*px = a;
			    else if (errb < min(erra,errmid))	*px = b;
			    else				*px = xmid;
			}
#if defined(DEBUG_FIND_ROOT)
			if (debugging("find_root")) 
			{
			    double fans;

			    (void) printf("Left bisection_find_root() ");
			    (void) printf("status = FUNCTION_SUCCEEDED\n");
			    (void) printf("root = %g, ",*px);
			    if ((*f)(*px,&fans,f_params) == FUNCTION_FAILED)
			    {
				(void) printf("WARNING ");
				(void) printf("in bisection_find_root(), ");
				(void) printf("Unable to evaluate function\n");
				return FUNCTION_FAILED;
			    }
			    (void) printf("y = %g, f(%g) = %g, ",y,*px,fans);
			    (void) printf("err = %g\n",fabs(y - fans));
			}
#endif /* defined(DEBUG_FIND_ROOT) */
			return FUNCTION_SUCCEEDED;
		}
		if ( (fa - y)*(fb - y) <= 0.0 ) 
		{
			s = (fa - y)/(fa - fb);
			if (s < 0.1)      s = 0.2;
			else if (s > 0.9) s = 0.8;
			xnew = a + s*(b - a);
			if ((*f)(xnew,&fnew,f_params) == FUNCTION_FAILED)
			{
			   (void) printf("WARNING in bisection_find_root(), ");
			   (void) printf("Unable to evaluate function\n");
			   return FUNCTION_FAILED;
			}
			if ( (fa - y)*(fnew - y) <= 0 ) 
			{
				b = xnew;
				fb = fnew;
			}
			else 
			{
				a = xnew;
				fa = fnew;
			}
		}
		else 
		{

			if (!find_separation_point(f,f_params,y,&a,b,a,
					&xmin,&fxmin,epsilon)) 
			{
			   (void) printf("WARNING in bisection_find_root(), ");
			   (void) printf("No change of sign\n");
#if defined(DEBUG_FIND_ROOT)
			   debug_print("find_root",
				 "Left bisection_find_root() ");
			   debug_print("find_root","status = FUNCTION_FAILED\n");
#endif /* defined(DEBUG_FIND_ROOT) */
				*px = xmin;

				return (fabs(fxmin - y) < epsilon) ?
					FUNCTION_SUCCEEDED : FUNCTION_FAILED;
			}
			if ((*f)(a,&fa,f_params) == FUNCTION_FAILED)
			{
			   (void) printf("WARNING in bisection_find_root(), ");
			   (void) printf("Unable to evaluate function\n");
			   return FUNCTION_FAILED;
			}
			xnew = a;
			fnew = fa;
		}
	}
	(void) printf("WARNING in bisection_find_root(), no convergence\n");
#if defined(DEBUG_FIND_ROOT)
	debug_print("find_root",
	      "Left bisection_find_root() status = FUNCTION_FAILED\n");
#endif /* defined(DEBUG_FIND_ROOT) */
	return FUNCTION_FAILED;
}		/*end bisection_find_root*/

/*
*			find_separation_point():
*
*	Tries to find a value px so that y - f(x0) and y - f(px)
*	have opposite signs.
*/


EXPORT boolean find_separation_point(
	boolean (*f)(double,double*,POINTER),
	POINTER f_params,
	double y,
	double *px,
	double x0,
	double x1,
	double *xmin,
	double *fmin,
	double epsilon)
{
	double a1;
	double fx0, fa1;
	double delta,f_of_x0_minus_y;
	int i,j,N = 1;
	static const int MAX_J = 1;

#if defined(DEBUG_FIND_ROOT)
	debug_print("find_root","Entering find_separation_point()\n");
#endif /* defined(DEBUG_FIND_ROOT) */
	if ((*f)(x0,&fx0,f_params) == FUNCTION_FAILED)
	{
	    (void) printf("WARNING in find_separation_point(), "
	                  "Unable to evaluate function\n");
	    return FUNCTION_FAILED;
	}
	*xmin = x0;	*fmin = fx0;
	f_of_x0_minus_y = fx0 - y;
	for (j = 0; j <= MAX_J; ++j)
	{
	    N *= 10;
	    delta = (x1 - x0)/N;
	    for( i = 0, a1 = x0; i < N; ++i)
	    {
	        a1 += delta;
	        if (i % 10 == 0 && j != 0)
		    continue;
	        if (!(*f)(a1,&fa1,f_params))
	        {
	    	    (void) printf("WARNING in find_separation_point(), "
	    	                  "Unable to evaluate function\n");
	    	    return FUNCTION_FAILED;
	        }
		if (fabs(fa1 - y) < fabs(*fmin - y))
		{
		    *xmin = a1;
		    *fmin = fa1;
		}
		if ( f_of_x0_minus_y * (fa1 - y) <= 0.0 )
		{
		    *px = a1;
#if defined(DEBUG_FIND_ROOT)
		    debug_print("find_root","Left find_separation_point() "
				      "ans = FUNCTION_SUCCEEDED\n");
#endif /* defined(DEBUG_FIND_ROOT) */
		    return FUNCTION_SUCCEEDED;
		}
		if (fabs(*fmin - y) < epsilon)
		    return FUNCTION_FAILED;
	    }
	}
#if defined(DEBUG_FIND_ROOT)
	debug_print("find_root",
	      "Left find_separation_point() ans = FUNCTION_FAILED\n");
#endif /* defined(DEBUG_FIND_ROOT) */
	return FUNCTION_FAILED;
}		/*end find_separation_point*/

/*
*			find_root():
*
*	Uses a combination of the secant and bisection method
*	to find a root of y = f(x).
*
*	The solution if it exists will either satisfy
*	fabs(f(*px) - y) < fabs(epsilon) or there exists a root
*	with fabs(root - *px) < fabs(delta).
*
*	This function has the property that the answer returned
*	is insured to be the one obtained by the last call to
*	the function f.  This fact can be exploited to use
*	f to set other parameters of interest that are not
*	explicitly know to this function.
*
*	The function f is evaluted by the call f(x,y,f_params)
*	where y = f(x).  F_params is an opaque data structure
*	for the passage of parameter arguments to the function f.
*	If x is outside the domain of definition for f,  then
*	f will return an error status of FUNCTION_FAILED.
*
*/

EXPORT boolean find_root(
	boolean (*f)(double,double*,POINTER),
	POINTER f_params,
	double	y,
	double	*px,
	double	x0,
	double	x1,
	double	epsilon,
	double	delta)
{
	double	xmin, xmax, fxmin, fxmax;
	double	xlast, x;
	double	fxlast, fx;
	double	xmid, len;
	double	dfdx, s;
	double	fpx;
	double   min_err, x_min_err;
	int	unstable = 0;
	int	i;
	const int MAX_NUM_ITER_SECANT = 20;
	const int MAX_NUM_ITER_BISECTION = 100;
	const int UNSTABLE = 0;

#if defined(DEBUG_FIND_ROOT)
	debug_print("find_root","Entering find_root()\n");
#endif /* defined(DEBUG_FIND_ROOT) */

	delta = fabs(delta);		epsilon = fabs(epsilon);
	xlast = x0;			x = x1;

	if ( (!(*f)(xlast,&fxlast,f_params)) || (!(*f)(x,&fx,f_params)) )
	{
	    if (debugging("find_root"))
	    {
	    	(void) printf("WARNING in find_root() --  "
	    	              "unable to evaluate function.\n");
	    }
	    *px = 0.5*(x0 + x1);
	    (void) (*f)(*px,&fx,f_params);
#if defined(DEBUG_FIND_ROOT)
	    debug_print("find_root","Left find_root()\n");
#endif /* defined(DEBUG_FIND_ROOT) */
	    return FUNCTION_FAILED;
	}
	fxlast -= y;
	fx -= y;

	x_min_err = xlast;
	min_err = fabs(fxlast);
	if (fabs(fx) < min_err)
	{
	    x_min_err = x;
	    min_err = fx;
	}

	if (x0 < x1)
	{
	    xmin = x0;
	    fxmin = fxlast;
	    xmax = x1;
	    fxmax = fx;
	}
	else
	{
	    xmin = x1;
	    fxmin = fx;
	    xmax = x0;
	    fxmax = fxlast;
	}

	xmid = 0.5*(xmin + xmax);	len = 0.5*(xmax - xmin);
	for (i = 0; i < MAX_NUM_ITER_SECANT; ++i) 
	{	
#if defined(DEBUG_FIND_ROOT)
	    if (debugging("find_root"))
	    {
	        (void) printf("In secant loop, i = %d\n",i);
	        (void) printf("xlast = %g, fxlast = %g\n",xlast,fxlast);
	        (void) printf("x = %g f(x) = %g\n",x,fx);
	        (void) printf("xmin = %g, fxmin = %g\n",xmin,fxmin);
	        (void) printf("xmax = %g, fxmax = %g\n",xmax,fxmax);
	    }
#endif /* defined(DEBUG_FIND_ROOT) */

	    if (fabs(fx) <= epsilon) 
	    {
		*px = x;
#if defined(DEBUG_FIND_ROOT)
		if (debugging("find_root"))
		{
		   (void) printf("CONVERGED ON FUNCTION VALUE\n"
		                 "epsilon = %g, fx = %g\n",epsilon,fx);
		}
		debug_print("find_root","Left find_root().\n");
#endif /* defined(DEBUG_FIND_ROOT) */
		return FUNCTION_SUCCEEDED;
	    }
	    else if (fabs(x - xlast) <= delta)
	    {
	    	double xans[4], err[4];
	    	int   k;
	    	boolean  eval_f = NO;

#if defined(DEBUG_FIND_ROOT)
	    	if (debugging("find_root"))
	    	      (void) printf("CONVERGED ON INTERVAL LENGTH\n");
#endif /* defined(DEBUG_FIND_ROOT) */

	    	xans[0] = xmin;		err[0] = fabs(fxmin);
	    	xans[1] = xmax;		err[1] = fabs(fxmax);
	    	xans[2] = xlast;	err[2] = fabs(fxlast);
	    	xans[3] = x;		err[3] = fabs(fx);
	    	*px = 0.5*(xmin + xmax);
	    	if ((*f)(*px,&fx,f_params) == FUNCTION_FAILED)
	    	{
	    	    if (debugging("find_root"))
	    	    {
	    	       (void) printf("WARNING in find_root() -- "
	    	                     "unable to evaluate function.\n");
	            }
	    	    *px = x_min_err;
	            (void) (*f)(*px,&fx,f_params);
#if defined(DEBUG_FIND_ROOT)
	            debug_print("find_root","Left find_root()\n");
#endif /* defined(DEBUG_FIND_ROOT) */
	    	    return FUNCTION_FAILED;
	    	}
		if (fabs(fx - y) < min_err)
		{
	            x_min_err = *px;
	            min_err = fabs(fx - y);
		}
		for ( k = 0; k < 4; ++k)
		{
		    if (err[k] < min_err)
		    {
			x_min_err = xans[k];
			min_err = err[k];
			eval_f = YES;
		    }
		}
		if ( eval_f && (!(*f)(*px,&fx,f_params)) )
		{
		    if (debugging("find_root"))
		    {
		       (void) printf("WARNING in find_root() -- "
		                     "unable to evaluate function.\n");
		    }
		    *px = x_min_err;
	            (void) (*f)(*px,&fx,f_params);
#if defined(DEBUG_FIND_ROOT)
	            debug_print("find_root","Left find_root()\n");
#endif /* defined(DEBUG_FIND_ROOT) */
		    return FUNCTION_FAILED;
		}
		if (fabs(fx - y) < min_err)
		{
		    min_err = fabs(fx - y);
		    x_min_err = *px;
		}
		*px = x_min_err;
	        (void) (*f)(*px,&fx,f_params);
#if defined(DEBUG_FIND_ROOT)
		if (debugging("find_root"))
		{
		    (void) printf("delta = %g, fabs(x - xlast) = %g\n",
				  delta,fabs(x - xlast));
		    (void) printf("epsilon = %g, fx = %g\n",epsilon,fx);
		}
		debug_print("find_root","Left find_root()\n");
#endif /* defined(DEBUG_FIND_ROOT) */
		return FUNCTION_SUCCEEDED;
	    }

	    dfdx = (fx - fxlast)/(x - xlast);

#if defined(DEBUG_FIND_ROOT)
	    if (debugging("find_root"))
	    	(void) printf("dfdx = %g\n",dfdx);
#endif /* defined(DEBUG_FIND_ROOT) */

	    xlast = x;		fxlast = fx;

	    if (fabs(len*dfdx) <= fabs(fx - dfdx*(x - xmid)))
	    {
	    	x = (fx*dfdx > 0.0) ? xmin + 0.2 * len : xmax - 0.2 * len;
		if (++unstable > UNSTABLE)
		    break;
	    }
	    else
		x -= fx/dfdx;

	    if (!(*f)(x,&fx,f_params))
	    {
	    	if (debugging("find_root"))
	    	{
	    	   (void) printf("WARNING in find_root() -- "
	    	                 "unable to evaluate function.\n");
	    	}
	    	*px = x_min_err;
	    	(void) (*f)(*px,&fx,f_params);
#if defined(DEBUG_FIND_ROOT)
	        debug_print("find_root","Left find_root()\n");
#endif /* defined(DEBUG_FIND_ROOT) */
	    	return FUNCTION_FAILED;
	    }
	    fx -= y;
	    if (fabs(fx) < min_err)
	    {
	        x_min_err = x;
		min_err = fabs(fx);
	    }

	    if (fxmin*fx < 0.0)
	    {
	        xmax = x;	fxmax = fx;
	    }
	    else if (fxmax*fx < 0.0)
	    {
	        xmin = x;	fxmin = fx;
	    }
	    else /* Possible local max/min in interval, secant unstable */
	    	break;
	    xmid = 0.5*(xmin + xmax);	len = 0.5*(xmax - xmin);
	}

		/* Bisection method for robust alternative */

#if defined(DEBUG_FIND_ROOT)
	if (debugging("find_root"))
	{
	  (void) printf("WARNING in find_root() -- secant method doesn't "
	                "converge or is unstable.  Using bisection method.\n");
	}
	debug_print("find_root","Using bisection, unstable = %s\n",
		(unstable > UNSTABLE) ? "YES" : "NO");
#endif /* defined(DEBUG_FIND_ROOT) */

	if (fxmin*fxmax > 0.0)
	{
	    if (find_separation_point(f,f_params,y,px,xmin,xmax,
				      &x,&fx,epsilon) == FUNCTION_FAILED)
	    {
	    	if (debugging("find_root"))
	    	{
	    	    (void) printf("WARNING in find_root() -- "
	    	                  "no convergence to solution.\n");
	    	}
		if (fabs(fx - y) < min_err)
		{
		    x_min_err = x;
		    min_err = fabs(fx - y);
		}
	    	*px = x_min_err;
	        (void) (*f)(*px,&fx,f_params);
#if defined(DEBUG_FIND_ROOT)
	        debug_print("find_root","Left find_root()\n");
#endif /* defined(DEBUG_FIND_ROOT) */
	    	return FUNCTION_FAILED;
	    }
	    if (!(*f)(*px,&fpx,f_params))
	    {
	    	if (debugging("find_root"))
	    	{
	    	    (void) printf("WARNING: in find_root() -- "
	    	                  "unable to evaluate function.\n");
	    	}
	    	*px = x_min_err;
	        (void) (*f)(*px,&fx,f_params);
#if defined(DEBUG_FIND_ROOT)
	        debug_print("find_root","Left find_root()\n");
#endif /* defined(DEBUG_FIND_ROOT) */
	    	return FUNCTION_FAILED;
	    }
	    x = *px;
	    fx = fpx - y;
	    if (fabs(fx) < min_err)
	    {
	        x_min_err = x;
		min_err = fabs(fx);
	    }
	    if (fxmin*fx < 0.0)
	    {
	    	xmax = x;	fxmax = fx;
	    }
	    else if (fxmax*fx < 0.0)
	    {
	    	xmin = x;	fxmin = fx;
	    }
	}

	for (i = 0; i < MAX_NUM_ITER_BISECTION; ++i)
	{
	    s = (fabs(fxmin - fxmax) < epsilon) ? 0.5 : fxmin/(fxmin-fxmax);
	    if (s < 0.1)
	        s = 0.2;
	    else if (s > 0.9)
	        s = 0.8;
	    x = xmin + s*(xmax - xmin);

	    if (!(*f)(x,&fx,f_params))
	    {
	    	if (debugging("find_root"))
	    	{
	    	   (void) printf("WARNING in find_root() -- "
	    	                 "unable to evaluate function.\n");
	    	}
	    	*px = x_min_err;
	    	(void) (*f)(*px,&fx,f_params);
#if defined(DEBUG_FIND_ROOT)
	        debug_print("find_root","Left find_root()\n");
#endif /* defined(DEBUG_FIND_ROOT) */
	    	return FUNCTION_FAILED;
	    }
	    fx -= y;
	    if (fabs(fx) < min_err)
	    {
	        x_min_err = x;
		min_err = fabs(fx);
	    }

#if defined(DEBUG_FIND_ROOT)
	    if (debugging("find_root"))
	    {
	    	(void) printf("In bisection loop, i = %d\n",i);
	    	(void) printf("x = %g, f(x) = %g\n",x,fx);
	    	(void) printf("xmin = %g, fxmin = %g\n",xmin,fxmin);
	    	(void) printf("xmax = %g, fxmax = %g\n",xmax,fxmax);
	    }
#endif /* defined(DEBUG_FIND_ROOT) */

	    if (fabs(fx) <= epsilon) 
	    {
	    	*px = x;
#if defined(DEBUG_FIND_ROOT)
	    	if (debugging("find_root"))
	    	{
	    	    (void) printf("CONVERGED ON FUNCTION VALUE\n"
	    	                  "epsilon = %g, fx = %g\n",epsilon,fx);
	    	}
	    	debug_print("find_root","Left find_root()\n");
#endif /* defined(DEBUG_FIND_ROOT) */
		return FUNCTION_SUCCEEDED;
	    }
	    else if (fabs(xmax - xmin) <= delta)
	    {
	    	double xans[3], err[3];
	    	int   k;
	    	boolean  eval_f = NO;

#if defined(DEBUG_FIND_ROOT)
	    	if (debugging("find_root"))
	            (void) printf("CONVERGED ON INTERVAL LENGTH\n");
#endif /* defined(DEBUG_FIND_ROOT) */

		xans[0] = xmin;		err[0] = fabs(fxmin);
		xans[1] = xmax;		err[1] = fabs(fxmax);
		xans[2] = x;		err[2] = fabs(fx);

		*px = 0.5*(xmin + xmax);
		if (!(*f)(*px,&fx,f_params))
		{
		    if (debugging("find_root"))
		    {
		       (void) printf("WARNING in find_root() -- "
		                     "unable to evaluate function.\n");
		    }
		    *px = x_min_err;
	            (void) (*f)(*px,&fx,f_params);
#if defined(DEBUG_FIND_ROOT)
	            debug_print("find_root","Left find_root()\n");
#endif /* defined(DEBUG_FIND_ROOT) */
		    return FUNCTION_FAILED;
		}
		if (fabs(fx - y) < min_err)
		{
		    x_min_err = *px;
		    min_err = fabs(fx - y);
		}
		for ( k = 0; k < 3; ++k)
		{
		    if (err[k] < min_err)
		    {
		    	x_min_err = xans[k];
		    	min_err = err[k];
		    	eval_f = YES;
		    }
		}
		if ( eval_f && (!(*f)(*px,&fx,f_params)) )
		{
		    if (debugging("find_root"))
		    {
		        (void) printf("WARNING - in find_root() -- "
		                      "unable to evaluate function.\n");
		    }
		    *px = x_min_err;
	            (void) (*f)(*px,&fx,f_params);
#if defined(DEBUG_FIND_ROOT)
	            debug_print("find_root","Left find_root()\n");
#endif /* defined(DEBUG_FIND_ROOT) */
		    return FUNCTION_FAILED;
		}
		if (fabs(fx - y) < min_err)
		{
		    x_min_err = *px;
		    min_err = fabs(fx - y);
		}
		*px = x_min_err;
	        (void) (*f)(*px,&fx,f_params);
#if defined(DEBUG_FIND_ROOT)
		if (debugging("find_root"))
		{
		    (void) printf("delta = %g, fabs(xmax - xmin) = %g\n",
				  delta,fabs(xmax - xmin));
		    (void) printf("epsilon = %g, fx = %g\n",epsilon,fx);
		}
		debug_print("find_root","Left find_root()\n");
#endif /* defined(DEBUG_FIND_ROOT) */
		return FUNCTION_SUCCEEDED;
	    }

	    if (fxmin*fx < 0.0)
	    {
	    	xmax = x;	fxmax = fx;
	    }
	    else if (fxmax*fx < 0.0)
	    {
	    	xmin = x;	fxmin = fx;
	    }
	}
	if (debugging("find_root"))
	{
	    (void) printf("WARNING in find_root() -- "
	                  "No convergence to solution\n");
	}
	*px = x_min_err;
	(void) (*f)(*px,&fx,f_params);
#if defined(DEBUG_FIND_ROOT)
	debug_print("find_root","Left find_root()\n");
#endif /* defined(DEBUG_FIND_ROOT) */
	return FUNCTION_FAILED;
}		/*end find_root*/

/*
*			search_harder_for_root():
*
*	This function modifies the interface upon which a root is
*	being sought until either a root is found or the limits reach
*	some specified values which are defined by either a) reaching
*	min_a and reaching max_b,  or b) number of attempts reaching N.
*	This function assumes that f(x) is essentially monotone in that
*	the next interval to be checked is determined by the direction
*	of increase from f(a) to f(b).  It also assumes a < b.
*/

EXPORT	boolean	search_harder_for_root(
	boolean	(*f)(double,double*,POINTER),	/* Solve f(x) = y*/
	POINTER f_params,			/* parameters for f */
	double	y,				/* value sought */
	double	*px,				/* answer if found */
	double	a,				/* [a, b] = initial search */
	double	b,				/* interval */
	double	*amin,				/* [*amin, *bmax] = maximum */
	double	*bmax,				/* search interval used */
	double	min_a,				/* [min_a, max_b] = maximum */
	double	max_b,				/* allowed */
	int	N,				/* number of attempts */
	double	epsilon,			/* |f(*px) - y| < epsilon */
	double	delta)				/* |*px - root| < delta */
{
	double	fa, fb, l;
	double   err, min_err;
	double   x, fx;
	int	i;

	*px = x = 0.5*(a+b);
	if (!(*f)(x,&fx,f_params))
	    return FUNCTION_FAILED;
	min_err = fabs(fx - y);
	for (i = 0; i < N; ++i)
	{
	    l = b - a;
	    if ( (!(*f)(a,&fa,f_params)) || (!(*f)(b,&fb,f_params)) )
	    {
	        if ((*f)(a,&fa,f_params))
		{
		    err = fabs(fa - y);
		    if (err < min_err)
		    {
		        min_err = err;
		        *px = x = a;
		    }
		}
	        if ((*f)(b,&fb,f_params))
		{
		    err = fabs(fb - y);
		    if (err < min_err)
		    {
		        min_err = err;
		        *px = x = b;
		    }
		}
	    	break;
	    }
	    err = fabs(fa - y);
	    if (err < min_err)
	    {
	        min_err = err;
	        *px = x = a;
	    }
	    err = fabs(fb - y);
	    if (err < min_err)
	    {
	        min_err = err;
	        *px = x = b;
	    }
	    if ((b < max_b) && Between(fb,y,fa))
	    {
	    	a = b;
	    	b = a + l;
	    	*bmax = b = min(b,max_b);
	    }
	    else if ((a > min_a) && Between(fa,y,fb))
	    {
	    	b = a;
	    	a = b - l;
	    	*amin = a = max(a,min_a);
	    }
	    else
	    {
	        if ((a <= min_a) && (max_b <= b))
		    break;
	    	if (min_a < a)
		{
		    a = a - l;
	    	    *amin = a = max(a,min_a);
		}
		if (b < max_b)
		{
		    b = b + l;
	    	    *bmax = b = min(b,max_b);
		}
	    }
	    if (find_root(f,f_params,y,px,a,b,epsilon,delta))
	    	return FUNCTION_SUCCEEDED;
	    if ((*f)(*px,&fx,f_params))
	    {
	        err = fabs(fx - y);
	        if (err < min_err)
	        {
	            min_err = err;
	            x = *px;
	        }
	    }
	}
	return FUNCTION_FAILED;
}		/*end search_harder_for_root*/


/*
*			print_function_values():
*
*	Debugging function.  Prints the values of X and f(X)-y in a format
*	suitable for input to the graphs function.
*/

EXPORT void print_function_values(
	boolean    (*f)(double,double*,POINTER),
	POINTER    f_params,
	double	   y,
	double	   x0,
	double	   x1,
	int	   npts,
	const char *msg,
	FILE	   *file)
{
	double	step = (x1 - x0) / npts;
	double	X, fX;
	char	s[80];
	int	i;

	(void) sprintf(s,"%s(X)-y",msg);
	(void) fprintf(file,"\n%-14s %-14s\n","X",s);
	for (i = 0, X = x0; i <= npts; ++i, X += step)
	{
	    if ((*f)(X,&fX,f_params) == FUNCTION_FAILED)
	    {
	       (void) printf("WARNING trying to print values for "
	                     "%s() --\n\tunable to evaluate at %g\n.",msg,X);
	       return;
	    }
	    (void) fprintf(file,"%-14g %-14g\n",X,fX-y);
	}
	(void) fprintf(file,"\n");
}		/*end print_function_values*/
