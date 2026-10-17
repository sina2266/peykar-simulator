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
*  	Perform a 2D FFT inplace given a complex 2D array
*  	The direction dir, 1 for forward, -1 for reverse
*  	The size of the array (nx,ny)
*  	Return FALSE if there are memory problems or
*  	the dimensions are not powers of 2
*/

#include <cdecs.h>
#include <vmalloc.h>

EXPORT boolean fft2d(
	COMPLEX **c,
	int nx,
	int ny,
	int dir)
{
    	int i,j;
       	int m,twopm;
   	double *real,*imag;

      	/* Transform the rows */

	uni_array(&real,nx,FLOAT);
	uni_array(&imag,nx,FLOAT);
	if (real == NULL || imag == NULL)
	    return(NO);
       	if (!Powerof2(nx,&m,&twopm) || twopm != nx)
      	    return(NO);
        for (j=0;j<ny;j++) 
	{
	    for (i=0;i<nx;i++) 
	    {
              	real[i] = c[i][j].real;
         	imag[i] = c[i][j].imag;
	    }
            fft(dir,m,real,imag);
            for (i=0;i<nx;i++) 
	    {
            	c[i][j].real = real[i];
           	c[i][j].imag = imag[i];
            }
      	}
        free_these(2,real,imag);

	/* Transform the columns */

	uni_array(&real,ny,FLOAT);
	uni_array(&imag,ny,FLOAT);
        if (real == NULL || imag == NULL)
     	    return(NO);
        if (!Powerof2(ny,&m,&twopm) || twopm != ny)
            return(NO);
   	for (i=0;i<nx;i++) 
	{
            for (j=0;j<ny;j++) 
	    {
	    	real[j] = c[i][j].real;
	    	imag[j] = c[i][j].imag;
            }
	    fft(dir,m,real,imag);
            for (j=0;j<ny;j++) 
	    {
            	c[i][j].real = real[j];
            	c[i][j].imag = imag[j];
       	    }
       	}
        free_these(2,real,imag);

        return(YES);
}	/* end fft2d */




/*
*  	This computes an in-place complex-to-complex FFT
*  	x and y are the real and imaginary arrays of 2^m points.
*  	dir =  1 gives forward transform
*  	dir = -1 gives reverse transform
*
*  	Formula: forward
*               N-1
*               ---
*          1   \          - j k 2 pi n / N
*  X(n) = ---   >   x(k) e                    = forward transform
*          N   /                                n=0..N-1
*               ---
*               k=0
*
*  	Formula: reverse
*               N-1
*               ---
*              \          j k 2 pi n / N
*  X(n) =       >   x(k) e                    = forward transform
*              /                                n=0..N-1
*               ---
*               k=0
*/

EXPORT boolean fft(
	int dir,
	int m,
	double *x,
	double *y)
{
   	long nn,i,i1,j,k,i2,l,l1,l2;
      	double c1,c2,tx,ty,t1,t2,u1,u2,z;

	/* Calculate the number of points */
    	nn = 1;
       	for (i=0;i<m;i++)
	    nn *= 2;

        /* Do the bit reversal */
	    i2 = nn >> 1;
	    j = 0;
	    for (i=0;i<nn-1;i++) 
	    {
	       	if (i < j) 
		{
	            tx = x[i];
	            ty = y[i];
	            x[i] = x[j];
	            y[i] = y[j];
	            x[j] = tx;
	            y[j] = ty;
	        }
	        k = i2;
	        while (k <= j) 
		{
	       	    j -= k;
		    k >>= 1;
		}
	        j += k;
	     }
	/* Compute the FFT */
   	    c1 = -1.0;
      	    c2 = 0.0;
            l2 = 1;
    	    for (l=0;l<m;l++) {
          	l1 = l2;
       		l2 <<= 1;
     		u1 = 1.0;
           	u2 = 0.0;
         	for (j=0;j<l1;j++) 
		{
          	    for (i=j;i<nn;i+=l2) 
		    {
             		i1 = i + l1;
             		t1 = u1 * x[i1] - u2 * y[i1];
           		t2 = u1 * y[i1] + u2 * x[i1];
                	x[i1] = x[i] - t1;
             		y[i1] = y[i] - t2;
                 	x[i] += t1;
             		y[i] += t2;
              	    }
        	    z =  u1 * c1 - u2 * c2;
        	    u2 = u1 * c2 + u2 * c1;
     	    	    u1 = z;
        	}
     		c2 = sqrt((1.0 - c1) / 2.0);
             	if (dir == 1)
              	    c2 = -c2;
            	c1 = sqrt((1.0 + c1) / 2.0);
	    }

        /* Scaling for forward transform */
     	if (dir == 1) 
	{
	    for (i=0;i<nn;i++) 
	    {
            	x[i] /= (double)nn;
            	y[i] /= (double)nn;
            }
      	}

	return(YES);
}	/* end fft */

EXPORT boolean Powerof2(
	int N,
	int *m,
	int *pwm)
{
	int i,NN;
	NN = N;

	i = 0;
	while (NN%2 != 1)
	{
	    NN /= 2;
	    i++;
	    if (NN == 1)
	    {
	    	*m = i;
		*pwm = N;
		return YES;
	    }
	}
	*pwm = -1;
	return NO;
}	/* end Powerof2 */
