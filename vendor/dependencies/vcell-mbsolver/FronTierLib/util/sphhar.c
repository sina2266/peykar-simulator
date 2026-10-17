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
*				sphhar.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Computes spherical harmonics and associated Legendre polynomials.
*
*	Translated into C from the NCAR spherepack3.0 fortran functions
*	dnlft and dnlft, NCAR holds the original copyright to the fortran
*	version of these functions.
*/

#include <cdecs.h>
#include <vmalloc.h>

/* LOCAL function prototypes */
LOCAL	double dnlft(int,int,double,double*);
LOCAL	void   dnlfk(int,int,double*);

/*
*			NALegendre():
*
*	Computes the normalized associated legendre function
*	NALegendre(m,n,phi) = pbar(m,n,phi), where
*
*	pbar(m,n,phi) = sqrt((2*n+1)*factorial(n-m)/(2*factorial(n+m)))
*	                    *sin(phi)**m/(2**n*factorial(n)) times the
*	                    (n+m)th derivative of (x**2-1)**n with respect
*	                     to x=cos(phi)
*/

EXPORT	double	NALegendre(
	int    m,
	int    n,
	double phi)
{
	static double *cp = NULL;
	static int    cp_len = 0;

	if (cp_len < (n/2+1))
	{
	    cp_len = max(2*cp_len,256);
	    cp_len = max(n/2+1,cp_len);
	    uni_array(&cp,cp_len,sizeof(double));
	}
	dnlfk(m,n,cp);
	return dnlft(m,n,phi,cp);
}		/*end NALegendre*/

/*
*			SphericalHarmonic():
*
*	Computes the spherical harmonic
*	SphericalHarmonic(m,n,phi,theta) = Y(m,n,phi,theta), where
*
*	Y(m,n,phi,theta) = ((-1)^m/sqrt(4*pi)) pbar(m,n,phi) exp(i*m*theta)
*/

EXPORT	double	*SphericalHarmonic(
	double *Yval,
	int    m,
	int    n,
	double phi,
	double theta)
{
	double pb = NALegendre(m,n,phi);
	double sgn = (m%2 == 0) ? 1.0 : -1.0;
	static double Ybuf[2];

	if (Yval == NULL)
	    Yval = Ybuf;
	Yval[0] = sgn*pb*cos(m*theta);
	Yval[1] = sgn*pb*sin(m*theta);
	return Yval;
}		/*end SphericalHarmonic*/

/*
*			SphericalHarmonic_r():
*
*	Returns the real part of the spherical harmonic Y(m,n,phi,theta)
*	define by
*
*	Y(m,n,phi,theta) = ((-1)^m/sqrt(4*pi)) pbar(m,n,phi) exp(i*m*theta)
*/

EXPORT	double	SphericalHarmonic_r(
	int    m,
	int    n,
	double phi,
	double theta)
{
	double pb = NALegendre(m,n,phi);
	double sgn = (m%2 == 0) ? 1.0 : -1.0;
	return sgn*pb*cos(m*theta);
}		/*end SphericalHarmonic_r*/

/*
*			Yi():
*
*	Returns the imaginary part of the spherical harmonic Y(m,n,phi,theta)
*	define by
*
*	Y(m,n,phi,theta) = ((-1)^m/sqrt(4*pi)) pbar(m,n,phi) exp(i*m*theta)
*/

EXPORT	double	SphericalHarmonic_i(
	int    m,
	int    n,
	double phi,
	double theta)
{
	double pb = NALegendre(m,n,phi);
	double sgn = (m%2 == 0) ? 1.0 : -1.0;
	return sgn*pb*sin(m*theta);
}		/*end SphericalHarmonic_i*/

/*
*			SphericalHarmonic_s():
*
*	Returns the real phase shifted spherical harmonic Y(m,n,phi,theta,phase)
*	define by
*
*	Y(m,n,phi,theta) = ((-1)^m/sqrt(4*pi)) pbar(m,n,phi) sin(m*theta+phase)
*/

EXPORT	double	SphericalHarmonic_s(
	int    m,
	int    n,
	double phi,
	double theta,
	double phase)
{
	double pb = NALegendre(m,n,phi);
	double sgn = (m%2 == 0) ? 1.0 : -1.0;
	return sgn*pb*sin(m*theta+phase);
}		/*end SphericalHarmonic_s*/

/*
*				dnlft():
*
*	The function dnlft uses coefficients computed by dnlfk to
*	compute the double precision normalized associated legendre
*	function at colatitude phi.
*
*	The normalized associated legendre functions are given by
*
*	pbar(m,n,phi) = sqrt((2*n+1)*factorial(n-m)/(2*factorial(n+m)))
*	                  *sin(phi)**m/(2**n*factorial(n)) times the
*	                  (n+m)th derivative of (x**2-1)**n with respect
*	                   to x=cos(phi)
*
*	where phi is colatitude.
*
*	The function dnlfk evaluates the following trigonometric expansion
*	of pbar(m,n,phi) which uses the fourier coefficients cp that
*	must be computed by subroutine dnlfk.
*
*	       1) for n even and m even, pbar(m,n,phi) =
*	          .5*cp[0] plus the sum from k=1 to k=n/2
*	          of cp[k-1]*cos(2*k*phi)
*
*	       2) for n even and m odd, pbar(m,n,phi) =
*	          the sum from k=1 to k=n/2 of
*	          cp[k-1]*sin(2*k*phi)
*
*	       3) for n odd and m even, pbar(m,n,phi) =
*	          the sum from k=1 to k=(n+1)/2 of
*	          cp[k-1]*cos((2*k-1)*phi)
*
*	       4) for n odd and m odd,  pbar(m,n,phi) =
*	          the sum from k=1 to k=(n+1)/2 of
*	          cp[k-1]*sin((2*k-1)*phi)
*
*
*	Input parameters
*
*	m      The order of pbar(n,m,phi). M can be any integer
*	       however pbar(n,m,phi) = 0  if abs(m) is greater than
*	       n and pbar(n,m,phi) = (-1)**m*pbar(n,-m,phi) for
*	       negative m.
*
*	n      Nonnegative integer specifying the degree of
*	       pbar(n,m,phi)
*
*	phi  Double precision colatitude in radians
*
*	cp     Double precision array that contains the fourier
*	       coefficients computed by routine dnlfk. The length
*	       of the array depends on the parity of m and n.
*
*	             parity            length of cp
*
*	          n even m even           n/2+1
*	          n even m odd             n/2
*	          n odd  m even          (n+1)/2
*	          n odd  m odd           (n+1)/2
*
*	output  return
*
*	pb     double precision variable containing pbar(n,m,phi)
*/

LOCAL double dnlft(
	int    m,
	int    n,
	double phi,
	double *cp)
{
	double chh, cdt;
	double cth, sdt, sth;
	double pb;
	int    mmod, nmod, k;
	int    kdo;

	cdt = cos(2.0*phi);
	sdt = sin(2.0*phi);
	nmod = n % 2;
	mmod = m % 2;
	if (nmod <= 0) /*n even*/
	{
	    if (mmod <= 0) /* n even, m even */
	    {

	        kdo = n/2;
	        pb = cp[0]*0.5;
	        if (n == 0)
		    return pb;
	        cth = cdt;
	        sth = sdt;
	        for (k = 0; k < kdo; k++)
		{
		    /* pb = pb+cp[k+1]*cos(2*(k+1)*phi) */
		    pb += cp[k+1]*cth;
		    chh = cdt*cth - sdt*sth;
		    sth = sdt*cth + cdt*sth;
		    cth = chh;
	        }
	        return pb;
	    }
	    else /* n even, m odd */
	    {
	        kdo = n/2;
	        pb = 0.0;
	        cth = cdt;
	        sth = sdt;
	        for (k = 0; k < kdo; k++)
		{
		    /* pb = pb+cp[k]*sin(2*(k+1)*phi) */
		    pb += cp[k]*sth;
		    chh = cdt*cth - sdt*sth;
		    sth = sdt*cth + cdt*sth;
		    cth = chh;
	        }
	        return pb;
	    }
	}
	else /* n odd */
	{
	    if (mmod <= 0) /* n odd, m even */
	    {
	        kdo = (n + 1)/2;
	        pb = 0.0;
	        cth = cos(phi);
	        sth = sin(phi);
	        for (k = 0; k < kdo; k++)
		{
		    /* pb = pb+cp[k]*cos((2*k+1)*phi) */
		    pb += cp[k]*cth;
		    chh = cdt*cth - sdt*sth;
		    sth = sdt*cth + cdt*sth;
		    cth = chh;
	        }
	        return pb;
	    }
	    else /* n odd, m odd */
	    {
	        kdo = (n + 1)/2;
	        pb = 0.0;
	        cth = cos(phi);
	        sth = sin(phi);
	        for (k = 0; k < kdo; k++)
		{
		    /* pb = pb+cp(k)*sin((2*k+1)*phi) */
		    pb += cp[k]*sth;
		    chh = cdt*cth - sdt*sth;
		    sth = sdt*cth + cdt*sth;
		    cth = chh;
	        }
	        return pb;
	    }
	}
}		/*end dnlft*/

/*
*			dnlfk():
*
*	The function dnlfk computes the coefficients in the trigonometric
*	expansion of the normalized associated legendre functions:
*
*	pbar(m,n,phi) = sqrt((2*n+1)*factorial(n-m)/(2*factorial(n+m)))
*	                   *sin(phi)**m/(2**n*factorial(n)) times the
*	                   (n+m)th derivative of (x**2-1)**n with respect
*	                   to x=cos(phi)
*
*	where phi is colatitude.
*
*	subroutine dnlfk computes the coefficients cp[k] in the
*	following trigonometric expansion of pbar(m,n,phi).
*
*	       1) for n even and m even, pbar(m,n,phi) =
*	          .5*cp[0] plus the sum from k=1 to k=n/2
*	          of cp[k-1]*cos(2*k*phi)
*
*	       2) for n even and m odd, pbar(m,n,phi) =
*	          the sum from k=1 to k=n/2 of
*	          cp[k-1]*sin(2*k*phi)
*
*	       3) for n odd and m even, pbar(m,n,phi) =
*	          the sum from k=1 to k=(n+1)/2 of
*	          cp[k-1]*cos((2*k-1)*phi)
*
*	       4) for n odd and m odd,  pbar(m,n,phi) =
*	          the sum from k=1 to k=(n+1)/2 of
*	          cp[k-1]*sin((2*k-1)*phi)
*
*	input parameters
*
*	m      is the order of pbar(n,m,phi). m can be any integer
*	       however pbar(n,m,phi) = 0  if abs(m) is greater than
*	       n and pbar(n,m,phi) = (-1)**m*pbar(n,-m,phi) for
*	       negative m.
*
*	n      nonnegative integer specifying the degree of
*	       pbar(n,m,phi)
*
*	output parameters
*
*	cp     a double precision array that contains the fourier
*	       coefficients for pbar(m,n,phi). the length of the
*	       array depends on the parity of m and n
*
*	             parity            length of cp
*
*	          n even m even           n/2+1
*	          n even m odd             n/2
*	          n odd  m even          (n+1)/2
*	          n odd  m odd           (n+1)/2
*/

LOCAL void dnlfk(
	int    m,
	int    n,
	double *cp)
{
	double fden, fnmh, fnum, fnnp1;
	double a1, b1, c1, fnmsq, t1, t2;
	double fk, cp2, pm1;
	int ma;
	int nex;
	int nmms2, i, l;
	const double sc20 = 1048576.0;      /*2^20*/
	const double sc40 = 1099511627776.0;/*2^40*/

	cp[0] = 0.0;
	ma = abs(m);
	if (ma > n)
	    return;
	if (n < 1)
	{
	    cp[0] = sqrt(2.0);
	    return;
	}
	else if (n == 1)
	{
	    if (ma != 0)
	    {
	        cp[0] = sqrt(0.75);
	        if (m == -1)
		    cp[0] = -cp[0];
	    }
	    cp[0] = sqrt(1.5);
	    return;
	}
	if ((n + ma) % 2 != 0)
	{
	    nmms2 = (n - ma - 1)/2;
	    fnum = n + ma + 2;
	    fnmh = n - ma + 2;
	    pm1 = -1.0;
	}
	else
	{
	    nmms2 = (n - ma)/2;
	    fnum = n + ma + 1;
	    fnmh = n - ma + 1;
	    pm1 = 1.0;
	}
	t1 = 1.0/sc20;
	nex = 20;
	fden = 2.0;
	for (i = 0; i < nmms2; i++)
	{
	    t1 = fnum*t1/fden;
	    if (t1 > sc20)
	    {
	    	t1 /= sc40;
	    	nex += 40;
	    }
	    fnum += 2.0;
	    fden += 2.0;
	}
	t1 /= pow(2.0,n - 1 - nex);
	if (ma/2 % 2 != 0)
	    t1 = -t1;
	t2 = 1.0;
	for (i = 0; i < ma; i++)
	{
	    t2 = fnmh*t2/(fnmh + pm1);
	    fnmh += 2.0;
	}
	cp2 = t1*sqrt((n + .5)*t2);
	fnnp1 = n*(n + 1);
	fnmsq = fnnp1 - ma*2.0*ma;
	l = (n + 1)/2;
	if (n % 2 == 0 && ma % 2 == 0)
	    l++;
	cp[l-1] = cp2;
	if ((m < 0) && (ma % 2 != 0))
	    cp[l-1] = -cp[l-1];
	if (l <= 1)
	    return;
	fk = n;
	a1 = (fk - 2.0)*(fk - 1.0) - fnnp1;
	b1 = (fk*fk - fnmsq)*2.0;
	cp[l-2] = b1*cp[l-1]/a1;
	for (l--; l > 1; l--)
	{
	    fk -= 2.0;
	    a1 = (fk - 2.0)*(fk - 1.0) - fnnp1;
	    b1 = (fnmsq - fk*fk)*2.0;
	    c1 = (fk + 1.0)*(fk + 2.0) - fnnp1;
	    cp[l-2] = -(b1*cp[l-1] + c1*cp[l])/a1;
	}
}		/*end dnlfk*/
