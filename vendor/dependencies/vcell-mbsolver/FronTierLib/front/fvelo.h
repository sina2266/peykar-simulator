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
*				fvelo.h
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Contains structures related to velocity functions
*/
#if !defined(_FVELO_H)
#define _FVELO_H


#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif

struct _TRANS_PARAMS
{
	int dim;
	double vel[MAXD];
};
typedef struct _TRANS_PARAMS TRANS_PARAMS;

struct _RADIAL_MOTION_PARAMS
{
	int dim;
	double cen[MAXD];
	double speed;
};
typedef struct _RADIAL_MOTION_PARAMS RADIAL_MOTION_PARAMS;

struct _SHEAR_MOTION_PARAMS
{
	int dim;
	double dir[MAXD];
	double coords[MAXD];
	double dvdh;
};
typedef struct _SHEAR_MOTION_PARAMS SHEAR_MOTION_PARAMS;

struct _SINE_MOTION_PARAMS
{
	int dim;
	double wave_length;
	double vmax;
	double phase;
};
typedef struct _SINE_MOTION_PARAMS SINE_MOTION_PARAMS;

struct _CIRCULAR_ROTATION_PARAMS
{
	int dim;
	double x_0,y_0;
	double omega_0;
	double grad;
};
typedef struct _CIRCULAR_ROTATION_PARAMS CIRCULAR_ROTATION_PARAMS;

struct _NORV_PARAMS
{
	int dim;
	double coeff;
	double epsilon;
};
typedef struct _NORV_PARAMS NORV_PARAMS;

struct _FLAME_PARAMS
{
        POINTER stuff;
        int problem_type;
        int dim;
        double alpha;
        double U;
        double m;
        double c;
        double epsilon;
        double wind[MAXD];
        double a,b;
};
typedef struct _FLAME_PARAMS FLAME_PARAMS;

struct _BURGERS_PARAMS
{
	int dim;
	double cen[MAXD];
	double speed;
};
typedef struct _BURGERS_PARAMS BURGERS_PARAMS;

struct _BIPOLAR_PARAMS
{
	int dim;
	double i1,i2;
	double cen1[MAXD],cen2[MAXD];
	double reverse_time;
};
typedef struct _BIPOLAR_PARAMS BIPOLAR_PARAMS;

struct _VORTEX_PARAMS
{
    int dim;
    double coeff;
    double time;
    double cos_time;
    char type[10];
    double cen[MAXD];
    double rad;
};
typedef struct _VORTEX_PARAMS VORTEX_PARAMS;

#if defined(c_plusplus) || defined(__cplusplus)
}
#endif


#endif	/* !defined(_FVELO_H) */
