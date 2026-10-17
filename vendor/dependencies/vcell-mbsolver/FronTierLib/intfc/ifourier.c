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
*				ifourier.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Contains intialization and support routines for Fourier polynomials.
*/

#include <intfc/iloc.h>

	/* FOURIER POLYNOMIAL Function Declarations */
LOCAL	void	        prompt_for_fourier_multi_modes(int,int,FOURIER_POLY*,
						      double*,double*);
LOCAL	void	        prompt_for_legendre_multi_modes(double,int,int,
							double*,int*);


EXPORT	FOURIER_POLY  *allocate_fourier_poly(
	int		nmodes,
	int		dim,
	ALIGN		*fpstore)
{
	FOURIER_POLY	*fpoly;
	double		*nu_store;
	size_t		naFP, naA, naP, naNUP, naNUS;
	size_t		size;
	int		i;

	if (nmodes == 0)
	    return NULL;

	naFP = num_aligns(sizeof(FOURIER_POLY));
	naA = num_aligns(nmodes*FLOAT);
	naP = num_aligns(nmodes*FLOAT);
	naNUP = num_aligns(nmodes*sizeof(double*));
	naNUS = num_aligns(nmodes*(dim-1)*FLOAT);
	size = sizeof(ALIGN)*(naFP + naA + naP + naNUP + naNUS);

	if (fpstore == NULL)
	    scalar(&fpstore,size);
	else
	    zero_scalar(fpstore,size);

	fpoly = (FOURIER_POLY*)fpstore;
	fpoly->A = (double*)(fpstore + naFP);
	fpoly->phase = (double*)(fpstore + naFP + naA);
	fpoly->nu = (double**)(fpstore + naFP + naA + naP);
	nu_store = (double*)(fpstore + naFP + naA + naP + naNUP);

	for (i = 0; i < nmodes; ++i)
	    fpoly->nu[i] = nu_store +i*(dim-1);

	fpoly->dim = dim;
	fpoly->num_modes = nmodes;
	return fpoly;
}		/*end allocate_fourier_poly*/


EXPORT	double fourier_poly(
	double		*coords,
	FOURIER_POLY	*fpoly)
{
	double		z;
	double		*A = fpoly->A;
	double		**nu = fpoly->nu;
	double		*phi = fpoly->phase;
	int		N = fpoly->num_modes;
	int		k, dim;

	z = fpoly->z0;
	dim = fpoly->dim;
	for (k = 0; k < N; ++k)
	{
	    z += A[k] * sin(scalar_product(nu[k],coords,dim-1)+phi[k]);
	}
	return z;
}		/*end fourier_poly*/

EXPORT	FOURIER_POLY	*get_fourier_coeffs(
	double		*L,
	double		*U,
	int		dim,
	const char	*mesg)
{
	int		i, len;
	char		choice[Gets_BUF_SIZE];
	FOURIER_POLY	*fpoly = NULL;

	screen("Enter the choice of interface description\n"
	       "The following types of descriptions are supported\n"
	       "\tMultiple mode description (M, default),\n"
	       "\t\tDirect input of Fourier polynomial defined by\n"
	       "\t\ty = sum_i A_i sin(2*PI*nu_i*(x-XL)/(XU-XL) + phi_i)\n"
	       "\tRandom mode description (R).\n"
	       "\t\tUser specifies range of frequencies, and the amplitudes\n");
	screen("\t\tand phases are choosen as Gaussian random variables\n"
	       "\tMixed random and user input modes (MR),\n"
	       "\tMultiple bubble description (B),\n"
	       "\t\tInterface shape is choosen using a truncated Fourier\n"
	       "\t\tseries for a superposition of step functions\n"
	       "\tEnter choice: ");

	(void) Gets(choice);
	len = (int) strlen(choice);
	for (i = 0; i < len; ++i)
	    choice[i] = tolower(choice[i]);

	if (strncmp(choice,"mr",2) == 0)
	    fpoly = get_fourier_mixed(L,U,dim,mesg);
	else if (strncmp(choice,"r",1) == 0)
	    fpoly = get_fourier_random(L,U,dim,mesg);
	else if (strncmp(choice,"b",1) == 0)
	    fpoly = get_fourier_bubble(L,U,dim,mesg);
	else
	    fpoly = get_fourier_multi_mode(L,U,dim,mesg);

	/* make sure all amplitudes are positive */
	for (i=0; i<fpoly->num_modes; ++i)
	{
	    if (fpoly->A[i] < 0) 
	    {
	    	fpoly->A[i] *= -1;
	    	fpoly->phase[i] += PI;
	    }
	}

	return	fpoly;
}		/*end get_fourier_coeffs*/

EXPORT	FOURIER_POLY 	*get_fourier_random(
	double		*L,
	double		*U,
	int		dim,
	const char	*mesg)
{
	FOURIER_POLY	*fpoly = NULL;
	int		min_n, max_n, num_modes;

	num_modes = random_bubble_num_modes(mesg,&min_n,&max_n,dim);
	fpoly = allocate_fourier_poly(num_modes,dim,NULL);

	init_random_modes(0,min_n,max_n,num_modes,fpoly,L,U);
	
	return fpoly;
}		/*end get_fourier_random*/

EXPORT	FOURIER_POLY 	*get_fourier_mixed(
	double		*L,
	double		*U,
	int		dim,
	const char	*mesg)
{
	FOURIER_POLY	*fpoly = NULL;
	char		s[256];
	int		num_modes_e;
	int		min_n_r, max_n_r, num_modes_r;

	screen("Enter the number of explicit modes %s: ",mesg);
	(void) Scanf("%d\n",&num_modes_e);

	(void) sprintf(s,"for random modes %s",mesg);
	num_modes_r = random_bubble_num_modes(s,&min_n_r,&max_n_r,dim);

	fpoly = allocate_fourier_poly(num_modes_e+num_modes_r,dim,NULL);

	prompt_for_fourier_multi_modes(0,num_modes_e,fpoly,L,U);
	init_random_modes(num_modes_e,min_n_r,max_n_r,num_modes_r,fpoly,L,U);
	
	return fpoly;
}		/*end get_fourier_mixed*/

/*
*			get_fourier_bubble():
*
*	This function calculates the Fourier coefficients
*	for the superposition of the individual bubbles
*	for the multiple bubble compressible Rayleigh-Taylor
*	simulation.   In this context,  a single "bubble"
*	with center c,  diameter d and amplitude A.  Is
*	the Fourier polynomial of degree nmode obtained
*	by truncating the Fourier series for the function
*
*		  _
*		 /  
*		 |  - 0.5 * A * (1 + cos(2*PI*(x-c)/d))  for |x - c| < d/2
*		/
*	f(x) = <
*		\
*		 |  0 					 otherwise
*		  \
*		   -
*
*	Each "bubble" may be modified by an additive constant
*	to adjust its mean position to the desired value.
*
*	TODO THREED
*/

EXPORT 	FOURIER_POLY	*get_fourier_bubble(
	double		*L,
	double		*U,
	int		dim,
	const char	*mesg)
{
	FOURIER_POLY	*fpoly = NULL;
	int		num_modes = 40;
	double 		**F_wave_number, *F_amplitude, *F_phase, X, XL;
	int 		i, j, nb;
	double 		*x, *a, *l;
	double 		A, B, amp, c;

	X = U[0]-L[0], XL = L[0];
	fpoly = allocate_fourier_poly(num_modes,dim,NULL);
	F_amplitude = fpoly->A;
	F_phase = fpoly->phase;
	F_wave_number = fpoly->nu;

	screen("Enter the number of bubbles %s: ",mesg);
	(void) Scanf("%d\n",&nb);

	uni_array(&x,nb,FLOAT); uni_array(&a,nb,FLOAT); uni_array(&l,nb,FLOAT);
	for (i = 0; i < nb; ++i)
	{
	    screen("Enter the center of bubble %d: ",i+1);
	    (void) Scanf("%f\n",&x[i]);
	    screen("Enter the amplitude of bubble %d: ",i+1);
	    (void) Scanf("%f\n",&a[i]);
	    screen("Enter the diameter of bubble %d: ",i+1);
	    (void) Scanf("%f\n",&l[i]);
	}

	for (i = 1; i <= num_modes; ++i) 
	{
	    A = B = 0.0;
	    for (j = 0; j < nb; ++j)
	    {
	    	c = i * l[j] / X;
	    	if ( fabs(c - 1.0) < EPSILON)
	            amp = -0.5*a[j]*l[j]/X;
		else
		    amp = a[j]*l[j]*sin(PI*c)/(PI*X*c*(sqr(c)-1.0));
		A += amp*cos(2.0*PI*i*(x[j] -XL)/X);
		B += amp*sin(2.0*PI*i*(x[j] -XL)/X);
	    }
	    F_amplitude[i-1]   = hypot(A,B);
	    F_wave_number[i-1][0] = 2.0*PI*i/X;
	    F_phase[i-1]       = 2.0*PI*i*XL/X + atan2(-A,B);
	}

	free_these(3,x,a,l);
	return fpoly;
}		/*end get_fourier_bubble*/

EXPORT	FOURIER_POLY	*get_fourier_multi_mode(
	double		*L,
	double		*U,
	int		dim,
	const char	*mesg)
{
	FOURIER_POLY	*fpoly = NULL;
	int		num_modes;

	screen("Enter the number of modes %s: ",mesg);
	(void) Scanf("%d\n",&num_modes);
	fpoly = allocate_fourier_poly(num_modes,dim,NULL);

	prompt_for_fourier_multi_modes(0,num_modes,fpoly,L,U);

	return fpoly;
}		/*end get_fourier_multi_mode*/

LOCAL	void	prompt_for_fourier_multi_modes(
	int	offset,
	int	num_modes,
	FOURIER_POLY	*fpoly,
	double		*L,
	double		*U)
{
	double	**wv_num = fpoly->nu + offset;
	double	*A = fpoly->A + offset, *phase = fpoly->phase + offset;
	double	nu;
	int	dim = fpoly->dim;
	int	i, j;

	for (i = 0; i < num_modes; ++i) 
	{
	    screen("Enter the amplitude of mode %d: ",i);
	    (void) Scanf("%f\n",&A[i]);
	    screen("Enter the phase of mode %d (in degrees): ",i);
	    (void) Scanf("%f\n",&phase[i]);
	    phase[i] = radians(phase[i]);
	    for (j = 0; j < dim-1; ++j)
	    {
	    	if (dim > 2)
	    	    screen("For coordinate direction %d, enter ",j);
	    	else
	    	    screen("Enter ");
	    	screen("the frequency of mode %d: ",i);
	    	(void) Scanf("%f\n",&nu);
	    	wv_num[i][j] = 2.0*PI*nu/((U[j]-L[j]));
	    	phase[i] -= L[j]*wv_num[i][j];
	    }
	}
}		/*end prompt_for_fourier_multi_modes*/

EXPORT	void init_random_modes(
	int		offset,
	int		min_n,
	int		max_n,
	int		nmodes,
	FOURIER_POLY	*fpoly,
	double		*L,
	double		*U)
{
	char		s[Gets_BUF_SIZE];
	double		A_sd, P_sd, av_phase;
	double		**wv_num = fpoly->nu+offset, *A = fpoly->A+offset;
	double		*phase = fpoly->phase+offset;
	double		nu;
	int		dim = fpoly->dim;
	int		i, j, n, m[2], k[3];
	int		sin_weight_amplitudes;
	unsigned short int	xsubi_a[3], xsubi_p[3];

	screen("Enter the amplitude standard deviation: ");
	(void) Scanf("%f\n",&A_sd);
	screen("Use sine weighting on amplitudes (dflt n): ");
	(void) Gets(s);
	sin_weight_amplitudes = (s[0] == 'y' || s[0] == 'Y') ? YES : NO;
	screen("Enter the average phase: ");
	(void) Scanf("%f\n",&av_phase);
	av_phase = radians(av_phase);
	screen("Enter the bubble phase standard deviation: ");
	(void) Scanf("%f\n",&P_sd);
	P_sd = radians(P_sd);

	xsubi_a[0] = 5123;	xsubi_a[1] = 234; xsubi_a[2] = 1979;
	screen("Enter an optional three short integers for\n");
	screen("\tthe amplitude random number generator seed: ");
	(void) Gets(s);
	if (s[0] != '\0')
	{
	    (void) sscanf(s,"%d %d %d",k,k+1,k+2);
	    for (i = 0; i < 3; ++i)
		xsubi_a[i] = (unsigned short int) k[i];
	}
	xsubi_p[0] = 4857;	xsubi_p[1] = 123; xsubi_p[2] = 11001;
	screen("Enter an optional three short integers for\n");
	screen("\tthe phase random number generator seed: ");
	(void) Gets(s);
	if (s[0] != '\0')
	{
	    (void) sscanf(s,"%d %d %d",k,k+1,k+2);
	    for (i = 0; i < 3; ++i)
		xsubi_p[i] = (unsigned short int) k[i];
	}

	(void) printf("\tnumber of modes::%d\n",nmodes);
	switch (dim)
	{
	case 2:
	    for (i = 0; i <= max_n - min_n; ++i) 
	    {
	        A[i] = random_gaussian(0.0,A_sd,xsubi_a);
	        if (sin_weight_amplitudes == YES)
	    	    A[i] *= sin(PI*((double)(i+1))/((double)(max_n-min_n+2)));
		(void) printf("\tAmplitude for mode %d::%g\n",i,A[i]);
		phase[i] = random_gaussian(av_phase,P_sd,xsubi_p);
		(void) printf("\tPhase for mode %d::%g\n",i,degrees(phase[i]));
		nu = (double) (min_n+i);
		(void) printf("\tfrequency for mode %d direction 0::%g\n",i,nu);
		wv_num[i][0] = 2.0*PI*nu/((U[0]-L[0]));
		phase[i] += L[0]*wv_num[i][0];
	    }
	    break;
	case 3:
	    i = 0;
	    for (n = min_n; n <= max_n; ++n)
	    {
	        for (m[0] = 0; m[0] <= n; ++m[0])
	        {
	            m[1] = n - m[0];
	            A[i] = random_gaussian(0.0,A_sd,xsubi_a);
	            (void) printf("\tAmplitude for mode %d::%g\n",i,A[i]);
	            phase[i] = random_gaussian(av_phase,P_sd,xsubi_p);
	            (void) printf("\tPhase for mode %d::%g\n",
				  i,degrees(phase[i]));
		    for (j = 0; j < 2; ++j)
		    {
		        nu = (double) m[j];
		        (void) printf("\tfrequency for mode %d ",i);
		        (void) printf("direction %d::%g\n",j,nu);
		        wv_num[i][j] = 2.0*PI*nu/((U[j]-L[j]));
		        phase[i] += L[j]*wv_num[i][j];
		    }
		    ++i;
		}
	    }
	    break;
	default:
	    screen("ERROR in init_random_mode(), invalid dim = %d\n",dim);
	    clean_up(ERROR);
	    break;
	}
}		/*end init_random_modes*/

EXPORT	int random_bubble_num_modes(
	const char	*mesg,
	int		*min_n,
	int		*max_n,
	int		dim)
{
	char s[Gets_BUF_SIZE];
	static const int dflt_n_modes = 1;

	*max_n = *min_n = dflt_n_modes;
	screen("Enter the number of modes or the minimum and maximum mode\n"
	       "\tnumbers %s (dflt = %d): ",mesg,dflt_n_modes);
	(void) Gets(s);
	if (s[0] != '\0')
	{
	    int n = sscanf(s,"%d %d\n",min_n,max_n);
	    if (n == 1)
		max_n = min_n;
	}

	switch (dim)
	{
	case 2:
	    return *max_n - *min_n + 1;

	case 3:
	    return (*max_n+2)*(*max_n+1)/2 - *min_n*(*min_n+1)/2;

	default:
	    screen("ERROR in random_bubble_num_modes(), invalid dim = %d\n",
		   dim);
	    clean_up(ERROR);
	}
	return -1;
}		/*end random_bubble_num_modes*/

EXPORT	LEGENDRE_POLY  *allocate_legendre_poly(
	int		max_degree,
	ALIGN		*lpstore)
{
	LEGENDRE_POLY	*lpoly;
	size_t		naLP, naA;
	size_t		size;

	if (max_degree == 0)
	    return NULL;

	naLP = num_aligns(sizeof(LEGENDRE_POLY));
	naA = num_aligns((max_degree+1)*FLOAT);
	size = sizeof(ALIGN)*(naLP + naA);

	if (lpstore == NULL)
	    scalar(&lpstore,size);
	else
	    zero_scalar(lpstore,size);

	lpoly = (LEGENDRE_POLY*)lpstore;
	lpoly->max_degree = max_degree;
	lpoly->A = (double*)(lpstore + naLP);
	return lpoly;
}		/*end allocate_legendre_poly*/

/*
*			legendre_poly():
*
*	Returns a linear combination of the normalized Legendre polynomials.
*	Given the array of amplitudes A = lpoly->A, the degree
*	N = lpoly->max_degree, and a value x,  this
*	functions returns the combination.
*
*
*	             N
*		    ---
*		    \
*		r = /     A[k]*sqrt(k+0.5)*P(x)
*	            ---                     k
*		   k = 0
*
*	where P(x) is the kth degree Legendre polynomial.  The normalization
* 	       k
*	factor sqrt(k + 0.5) is chosen so that the L2 norm of r on [-1,1]
*	is the sum of the squares of the coefficients A[k].
*/

EXPORT	double legendre_poly(
	double		x,
	LEGENDRE_POLY	*lpoly)
{
	double		r, Pk, Pkm1, Pkm2;
	double		*A = lpoly->A;
	int		k, N = lpoly->max_degree;

	Pkm2 = 0.0;
	Pkm1 = 1.0;
	r = A[0]*sqrt(0.5);
	for (k = 1; k <= N; ++k)
	{
	    Pk = (2.0*x*Pkm1 - Pkm2) - (x*Pkm1 - Pkm2)/k;
	    r += A[k]*Pk*sqrt(k + 0.5);
	    Pkm2 = Pkm1;
	    Pkm1 = Pk;
	}
	return r;
}		/*end legendre_poly*/

EXPORT	LEGENDRE_POLY	*get_legendre_coeffs(
	double      a0,
	const char *mesg)
{
	int		i, len;
	char		choice[Gets_BUF_SIZE];
	LEGENDRE_POLY	*lpoly = NULL;

	screen("Enter the choice of interface description\n"
	       "The following types of descriptions are supported\n"
	       "\tMultiple mode description (M, default),\n"
	       "\t\tDirect input of Legendre polynomial defined by\n"
	       "\t\tr = sum_i A_i sqrt(n+0.5) P_n(x)\n"
	       "\tRandom mode description (R).\n"
	       "\t\tUser specifies range of wave numbers and amplitudes\n"
	       "\t\tare choosen as Gaussian random variables\n"
	       "\tMixed random and user input modes (MR),\n"
	       "\tEnter choice: ");

	(void) Gets(choice);
	len = (int) strlen(choice);
	for (i = 0; i < len; ++i)
	    choice[i] = tolower(choice[i]);

	if (strncmp(choice,"mr",2) == 0)
	    lpoly = get_legendre_mixed(a0,mesg);
	else if (strncmp(choice,"r",1) == 0)
	    lpoly = get_legendre_random(a0,mesg);
	else
	    lpoly = get_legendre_multi_mode(a0,"",mesg);

	return	lpoly;
}		/*end get_legendre_coeffs*/

EXPORT	LEGENDRE_POLY	*get_legendre_multi_mode(
	double      a0,
	const char *mesg1,
	const char *mesg2)
{
	LEGENDRE_POLY *lpoly = NULL;
	double	      *amplitude;
	int	      *mode_degree;
	char	      s[Gets_BUF_SIZE];
	int	      i, max_degree;
	int           num_modes;

	num_modes = 1;
	screen("Enter the number of modes %s%s (dflt = %d): ",
	       mesg1,mesg2,num_modes);
	(void) Gets(s);
	if (s[0] != '\0')
	{
	    if (sscanf(s,"%d",&num_modes) != 1)
	    {
	        screen("ERROR in get_legendre_multi_mode(), "
		       "invalid input \"%s\" of number of modes\n",s);
	        clean_up(ERROR);
		return NULL;
	    }
	    if (num_modes <= 0)
	    {
	        screen("ERROR in get_legendre_multi_mode(), "
		       "nonpositive \"%d\" number of modes\n",num_modes);
		clean_up(ERROR);
	        return NULL;
	    }
	}

	uni_array(&amplitude,num_modes,FLOAT);
	uni_array(&mode_degree,num_modes,INT);
	prompt_for_legendre_multi_modes(a0,0,num_modes,amplitude,mode_degree);

	for (max_degree = 0, i = 0; i < num_modes; ++i)
	    max_degree = max(max_degree,mode_degree[i]);

	lpoly = allocate_legendre_poly(max_degree,NULL);
	for (i = 0; i <= max_degree; ++i)
	    lpoly->A[i] = 0.0;
	for (i = 0; i < num_modes; ++i)
	    lpoly->A[mode_degree[i]] += amplitude[i];

	free_these(2,amplitude,mode_degree);
	return lpoly;
}		/*end get_legendre_multi_mode*/

LOCAL	void	prompt_for_legendre_multi_modes(
	double a0,
	int   default_mode0_degree,
	int   num_modes,
	double *amplitude,
	int   *mode_degree)
{
	char s[Gets_BUF_SIZE];
	int  i;

	for (i = 0; i < num_modes; ++i) 
	{
	    mode_degree[i] = i + default_mode0_degree;
	    screen("Enter the polynomial degree of mode %d (dflt = %d): ",i,
		   mode_degree[i]);
	    (void) Gets(s);
	    if (s[0] != '\0')
	        (void) sscanf(s,"%d",mode_degree+i);
	    amplitude[i] = (mode_degree[i] == 0) ? a0 : 0.0;
	    screen("Enter the amplitude of mode %d (dflt = %g): ",
		   i,amplitude[i]);
	    (void) Gets(s);
	    if (s[0] != '\0')
	    {
		if (sscan_float(s,amplitude+i) != 1)
		{
		    screen("ERROR in prompt_for_legendre_multi_modes(), "
		           "invalid \"%s\" input of amplitude\n",s);
		    clean_up(ERROR);
		}
	    }
	}
}		/*end prompt_for_legendre_multi_modes*/

EXPORT	LEGENDRE_POLY 	*get_legendre_random(
	double      a0,
	const char *mesg)
{
	LEGENDRE_POLY      *lpoly = NULL;
	char               s[Gets_BUF_SIZE];
	double              *A, A_sd;
	int	           i, min_n, max_n;
	int	           k[3];
	unsigned short int xsubi[3];

	min_n = 0;
	max_n = 0;
	screen("Enter the minimum degree of the Legendre\n"
	       "\tpolynomials %s (dflt = %d): ",mesg,min_n);
	(void) Gets(s);
	if (s[0] != '\0')
	    (void) sscanf(s,"%d",&min_n);

	screen("Enter the maximum degree of the random amplitude Legendre\n"
	       "\tpolynomials %s (dflt = %d): ",mesg,max_n);
	(void) Gets(s);
	if (s[0] != '\0')
	    (void) sscanf(s,"%d",&max_n);

	if ((min_n < 0) || (max_n < min_n))
	{
	    screen("ERROR in get_legendre_random(), "
		   "invalid degree range\n");
	    clean_up(ERROR);
	}
	lpoly = allocate_legendre_poly(max_n,NULL);
	A = lpoly->A;
	A[0] = a0;
	for (i = 1; i <= max_n; ++i)
	    A[i] = 0.0;

	A_sd = 0.0;
	screen("Enter the amplitude standard deviation (dflt = %g): ",A_sd);
	(void) Gets(s);
	if (s[0] != '\0')
	    (void) sscan_float(s,&A_sd);
	xsubi[0] = 2345; xsubi[1] = 3572; xsubi[2] = 13789;
	screen("Enter an optional three short integers for\n"
	       "\tthe phase random number generator seed: ");
	(void) Gets(s);
	if (s[0] != '\0')
	{
	    (void) sscanf(s,"%d %d %d",k,k+1,k+2);
	    for (i = 0; i < 3; ++i)
		xsubi[i] = (unsigned short int) k[i];
	}
	for (i = min_n; i <= max_n; ++i)
	{
	    A[i] = random_gaussian(A[i],A_sd,xsubi);
	}
	(void) printf("\n");
	for (i = 0; i <= max_n; ++i)
	    (void) printf("\tAmplitude for degree %d::%g\n",i,A[i]);
	(void) printf("\n");

	return lpoly;
}		/*end get_legendre_random*/

EXPORT	LEGENDRE_POLY 	*get_legendre_mixed(
	double      a0,
	const char *mesg)
{
	LEGENDRE_POLY *lpoly = NULL, *lpoly_e = NULL, *lpoly_r = NULL;
	double	      *A, *Ae, *Ar;
	int           i, max_degree, max_degree_e, max_degree_r;

	lpoly_e = get_legendre_multi_mode(a0,"explicit ",mesg);
	lpoly_r = get_legendre_random(0.0,mesg);
	max_degree_e = lpoly_e->max_degree;
	max_degree_r = lpoly_r->max_degree;
	max_degree = max(max_degree_e,max_degree_r);
	lpoly = allocate_legendre_poly(max_degree,NULL);
	A = lpoly->A;
	Ae = lpoly_e->A;
	Ar = lpoly_r->A;
	for (i = 0; i <= max_degree; ++i)
	    A[i] = 0.0;
	for (i = 0; i <= max_degree_e; ++i)
	    A[i] += Ae[i];
	for (i = 0; i <= max_degree_r; ++i)
	    A[i] += Ar[i];
	free_these(2,lpoly_e,lpoly_r);
	return lpoly;
}		/*end get_legendre_mixed*/


EXPORT	int spherical_num_modes(
	const char *mesg,
	int	   *min_n,
	int	   *max_n,
	double	   *U)
{
	int pl,pm,pmax;
	int num_modes, i;

	if (U[0] == PI)
	    pl = 1;
	else if (U[0] == 0.5*PI)
	    pl = 2;
	if (U[1] == 2*PI)
	    pm = 1;
	else if (U[1] == PI)
	    pm = 2;
	else if (U[1] == 0.5*PI)
	    pm = 4;
	pmax = (pl > pm) ? pl : pm;

	switch (pmax)
        {
        case 1:
	    screen("Enter the minimum and maximum frequency "
	           "numbers %s: ",mesg);
	    (void) Scanf("%d %d\n",min_n,max_n);
	    break;
        case 2:
	    screen("Frequency must be a muptiple of 2\n"
	           "Enter the minimum and maximum frequency "
	           "numbers %s: ",mesg);
	    (void) Scanf("%d %d\n",min_n,max_n);
	    if ((*max_n%2 != 0) || (*min_n%2 != 0))
            {
		screen("ERROR in spherical_num_modes(), "
		       "invalid frequency, pmax = %d!\n",pmax);
		clean_up(ERROR);
            }
	    break;
        case 4:
	    screen("Frequency must be a muptiple of 4\n"
	           "Enter the minimum and maximum frequency "
	           "numbers %s: ",mesg);
	    (void) Scanf("%d %d\n",min_n,max_n);
	    if ((*max_n%4 != 0) || (*min_n%4 != 0))
            {
		screen("ERROR in spherical_num_modes(), "
		       "invalid frequency, pmax = %d!\n",pmax);
		clean_up(ERROR);
            }
	    break;
	default:
	    screen("ERROR in spherical_num_modes(), unknown periodicity\n");
	    clean_up(ERROR);
        }

	num_modes = 0;
	for (i = *min_n; i <= *max_n; i = i+pl)
	    num_modes += (i+1)/pm;
	return num_modes;
}		/* end spherical_num_modes */
