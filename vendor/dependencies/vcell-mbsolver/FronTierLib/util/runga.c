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
*				runga.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Simple 4th Order Runga-Kutta Method
*/

#include <cdecs.h>
#include <vmalloc.h>


	/* LOCAL Function Declarations */
LOCAL	boolean	rk1(double,double*,double*,int,double,
		    boolean(*)(double,double*,double*,int,POINTER),POINTER);


/*
*			runga_kutta():
*	
*	Solves the initial value problem
*
*		 dy
*		----   =  f(x,y)	y(x0) = y0;
*		 dx
*
*	where y and f are dimension n uni_arrays,
*	using a fourth order Runga-Kutta with variable step.
*	This function returns the value of y at x1.  Since it
*	integrates past x1 and then interpolates the value at
*	x1 it is important that the problem be well defined
*	up to and past x1.
*	The parameter eps is allowable truncation error.  The step
*	size will be adjusted so that this error is approximately
*	satified.  If the truncation estimate of the local truncation
*	error becomes very small, the step sized will be increased.
*
*	Storage is allocated the first time this function is called.
*	If it is used later with a different dimension n the old
*	storage will be freed and new storage of the proper dimension
*	will be allocated.
*
*	The function f(x,y) is evaluated by the call to the boolean valued
*	function feval(x,y,f,n,parameters).  Where x is a scalar, y and f are
*	n dimensional uni_arrays and parameters is a pointer which
*	can be cast as a appropriate structure pointer to recover
*	any addition parameters which f may depend on.
*	The function feval should also return a value, YES if the
*	evaluation is sucessful, NO if the evaluation is unsucessful
*	but the program should continue with reduced step size, or
*	ERROR if the program should be terminated.
*
*	The input parameter H is a pointer to an initial guess for 
*	the step size.	It will be automatically increased or decreased 
*	to the correct size. Its value on return will be the last
*	step size used.
*/

#if defined(DEBUG_RUNGA_KUTTA)
#define rk_warning(s)	(void) printf("WARNING in runga_kutta(), %s\n",(s))
#else /* defined(DEBUG_RUNGA_KUTTA) */
#define rk_warning(s)
#endif /* defined(DEBUG_RUNGA_KUTTA) */


#define set_error_message(mesg,fname)					\
	{								\
		char s[80];						\
		(void) sprintf(s,"%s at line %d",mesg,__LINE__);	\
		if (fname != NULL)					\
		{							\
			(void) strcat(s," in ");			\
			(void) strcat(s,fname);				\
			(void) strcat(s,"()");				\
		}							\
		rk_warning(s);						\
	}

#define adjust_step_for_status1(stat,H,HMIN)				\
	switch ((stat))							\
	{								\
	case YES:							\
	    break;							\
	case NO:							\
	    if (fabs((H)) > HMIN)					\
	    {								\
	    	(H) *= 0.5;						\
	    	goto restart_loop;					\
	    }								\
	    else							\
	    	return NO;						\
	}

#define adjust_step_for_status2(stat,H,HOLD,HMIN)			\
	switch ((stat))							\
	{								\
	case YES:							\
	    break;							\
	case NO:							\
	    if (fabs((H)) > HMIN)					\
	    {								\
	    	(H) = (HOLD);						\
	    	goto advance_solution;					\
	    }								\
	    else							\
	    	return NO;						\
	}

EXPORT	boolean runga_kutta(
	double		x0,
	double		*y0,
	double		x1,
	double		*y1,
	double		*H,
	int		n,
	boolean		(*feval)(double,double*,double*,int,POINTER),
	double		eps,
	POINTER		parameters)
{
	boolean		status;
	double		h, xi, xim1, errmax, fmax;
	double		errk, H_old;
	int		k;
	int		i;
	static const	int	MAX_NUM_LOOPS = 10000;/*TOLERANCE*/
	static const	double  HMIN = 1.e-10;/*TOLERANCE*/
	static const    double  Epsilon = 1.e-6; /* TOLERANCE */
	static double	*f, *etaH, *etaH22, *etaH2, *tmp, *yi, *yim1;
	static int	storage_allocated = NO;
	static int	olddim;
	
	if (!storage_allocated)
	{
	    storage_allocated = YES;
	    uni_array(&f,n,DOUBLE);
	    uni_array(&etaH,n,DOUBLE);
	    uni_array(&etaH22,n,DOUBLE);
	    uni_array(&etaH2,n,DOUBLE);
	    uni_array(&tmp,n,DOUBLE);
	    uni_array(&yi,n,DOUBLE);
	    uni_array(&yim1,n,DOUBLE);
	    olddim = n;
	}
	else if (n != olddim)
	{
	    free_these(7,f,etaH,etaH22,etaH2,tmp,yi,yim1);
	    uni_array(&f,n,DOUBLE);
	    uni_array(&etaH,n,DOUBLE);
	    uni_array(&etaH22,n,DOUBLE);
	    uni_array(&etaH2,n,DOUBLE);
	    uni_array(&tmp,n,DOUBLE);
	    uni_array(&yi,n,DOUBLE);
	    uni_array(&yim1,n,DOUBLE);
	    olddim = n;
	}
	if ((*feval)(x0,y0,f,n,parameters) == FUNCTION_FAILED)
	{
		rk_warning("unable to evalutate function");
		return FUNCTION_FAILED;
	}
	fmax = fabs(f[0]);
	for (i = 1; i < n; i++)
		fmax = max(fmax,fabs(f[i]));
	if (fmax*fabs(x0 - x1) <= Epsilon*eps)
	{
		*H = 0.0;
		for (i = 0; i < n; i++)
			y1[i] = y0[i] + f[i]*(x1 - x0);
		return FUNCTION_SUCCEEDED;
	}
	*H = (x1 < x0) ? -fabs(*H) : fabs(*H);
	xi = x0;
	for (k = 0; k < n; k++)
		yi[k] = y0[k];

	for (i = 0; i < MAX_NUM_LOOPS; i++)
	{
		xim1 = xi;
		for (k = 0; k < n; k++)	yim1[k] = yi[k];

		status = rk1(xi,yi,etaH,n,*H,feval,parameters);
		adjust_step_for_status1(status,*H,HMIN);
		status = rk1(xi,yi,etaH22,n,0.5*(*H),feval,parameters);
		adjust_step_for_status1(status,*H,HMIN);
		status = rk1(xi,etaH22,etaH2,n,0.5*(*H),feval,parameters);
		adjust_step_for_status1(status,*H,HMIN);

		errmax = fabs(etaH[0] - etaH2[0]);
		for (k = 1; k < n; k++)
		{
			if ((errk = fabs(etaH[k] - etaH2[k])) > errmax)
				errmax = errk;
		}
		if (errmax < .9375*eps)
		{

	/* Check to see if step size can be increased */

			H_old = *H;
			if (0.034*eps < errmax)
				h = *H/pow(errmax/(eps*.9375),0.2);
			else
				h = 2.0*(*H);
			*H = h;
			status = rk1(xi,yi,etaH,n,*H,feval,parameters);
			adjust_step_for_status2(status,*H,H_old,HMIN);
			status = rk1(xi,yi,etaH22,n,0.5*(*H),feval,parameters);
			adjust_step_for_status2(status,*H,H_old,HMIN);
			status = rk1(xi,etaH22,tmp,n,0.5*(*H),feval,parameters);
			adjust_step_for_status2(status,*H,H_old,HMIN);
		
		/* Compute local error at new step */

			errmax = fabs(etaH[0] - tmp[0]);
			for (k = 1; k < n; k++)
			{
				if ((errk = fabs(etaH[k] - tmp[k])) > errmax)
					errmax = errk;
			}

		/* Test for allowable error */

			if (errmax <= 259.2*eps)
			{
				for (k = 0; k < n; k++)
					etaH2[k] = tmp[k];
				goto advance_solution;
			}
			else
			{
				*H = H_old;
				goto advance_solution;
			}
		}


		while (errmax > 228.*eps)
		{

	/* Reduce step size */

			h = *H/pow(errmax/(eps*.9375),0.2);
			*H = 2.0*h;
			if (fabs(*H) < HMIN)
			{
				rk_warning("step size too small");
				return FUNCTION_FAILED;
			}

			status = rk1(xi,yi,etaH,n,*H,feval,parameters);
			adjust_step_for_status1(status,*H,HMIN);
			status = rk1(xi,yi,etaH22,n,0.5*(*H),feval,parameters);
			adjust_step_for_status1(status,*H,HMIN);
			status = rk1(xi,etaH22,etaH2,n,0.5*(*H),
				feval,parameters); 
			adjust_step_for_status1(status,*H,HMIN);

			errmax = fabs(etaH[0] - etaH2[0]);
			for (k = 1; k < n; k++)
			{
				if ((errk = fabs(etaH[k] - etaH2[k])) > errmax)
					errmax = errk;
			}
		}
advance_solution:
		xi += *H;
		for (k = 0; k < n; k++)
			yi[k] = etaH2[k];
		if (!Between(xi,x0,x1)) break;
restart_loop:	;
	}
	if (i >= MAX_NUM_LOOPS)
	{
		rk_warning("too many steps");
		return FUNCTION_FAILED;
	}
	for (k = 0; k < n; k++)
		y1[k] = ((x1 - xim1)*yi[k] + (xi - x1)*yim1[k])/(*H);

	return FUNCTION_SUCCEEDED;
}		/*end runga_kutta*/

LOCAL	boolean rk1(
	double		x0,
	double		*y0,
	double		*eta,
	int		n,
	double		h,
	boolean		(*feval)(double,double*,double*,int,POINTER),
	POINTER		parameters)
{
	static	double	*k1, *k2, *k3, *k4, *ytmp;
	static	boolean	storage_allocated = NO;
	static	int	olddim;
	boolean		status;
	int		i;
	
	if (!storage_allocated)
	{
	    storage_allocated = YES;
	    uni_array(&k1,n,DOUBLE);
	    uni_array(&k2,n,DOUBLE);
	    uni_array(&k3,n,DOUBLE);
	    uni_array(&k4,n,DOUBLE);
	    uni_array(&ytmp,n,DOUBLE);
	    olddim = n;
	}
	else if (n != olddim)
	{
	    free_these(5,k1,k2,k3,k4,ytmp);
	    uni_array(&k1,n,DOUBLE);
	    uni_array(&k2,n,DOUBLE);
	    uni_array(&k3,n,DOUBLE);
	    uni_array(&k4,n,DOUBLE);
	    uni_array(&ytmp,n,DOUBLE);
	    olddim = n;
	}
	if ((status=(*feval)(x0,y0,k1,n,parameters))==FUNCTION_FAILED)
	    return status;
	for (i = 0; i < n; i++)
	    ytmp[i] = y0[i] + 0.5*h*k1[i];
	if ((status=(*feval)(x0+0.5*h,ytmp,k2,n,parameters))==FUNCTION_FAILED)
	    return status;
	for (i = 0; i < n; i++)
	    ytmp[i] = y0[i] + 0.5*h*k2[i];
	if ((status=(*feval)(x0+0.5*h,ytmp,k3,n,parameters))==FUNCTION_FAILED)
	    return status;
	for (i = 0; i < n; i++)
	    ytmp[i] = y0[i] + h*k3[i];
	if ((status=(*feval)(x0+h,ytmp,k4,n,parameters))==FUNCTION_FAILED)
	    return status;
	for (i = 0; i < n; i++)
	    eta[i] = y0[i] + h*(k1[i] + 2.*k2[i] + 2.*k3[i] + k4[i])/6.0;
	return FUNCTION_SUCCEEDED;
}		/*end rk1*/
