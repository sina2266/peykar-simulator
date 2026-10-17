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
*				example0.c:
*
*		User initialization example for Front Package:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*	
*	This example shows a circle in a double vortex field. It demonstrates
*	the resolution of the front tracking method.
*
*/

#include <vector>
#include <FronTier.h>

static double size=2.5;
static int Nx = 640;
double vx = 0.0;
double e = 0.1;

char *in_name,*restart_state_name,*restart_name,*out_name;
boolean RestartRun;
int RestartStep;
boolean binary = YES;

static double example2d_init_surface_func(
        POINTER func_params,
        double *coords)
{
	double phi = atan2(coords[1], coords[0]);
	double dist = sqrt(sqr(coords[0]) + sqr(coords[1])) - 
		(1 + e*cos(5 * phi) + 2 * e* cos(7 * phi));
	return dist;
}

/********************************************************************
 *	Sample (rotation) velocity function for the front    *
 ********************************************************************/

static int example2d_vel_func(
	POINTER params,
	Front *front,
	FT_POINT *p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF *hs,
	double *vel)
{	
	double *coords = Coords(p);
	
	double Time = front->time;
	double X = coords[0];
	double y = coords[1];

    double Center_x = vx*Time;
    Center_x = Center_x - 2 * size * floor((Center_x + size)/ (2*size));	
    double Center_X = Center_x - 2 * size * (X - Center_x <= -size) + 2 * size * (X - Center_x >=size);
	//if (fabs(X) > size)
	//	printf("X=%lf Center_X=%lf\n", X, Center_X);
    double Center_y = 0;
    double fi = atan2(coords[1]-Center_y, coords[0]-Center_X);
    double R_dot = -e * 5 * sin(5*(fi+Time)) - 2 * e * 7 * 0.5 * sin( 7 * (fi + 0.5 * Time));
    vel[0] = R_dot*cos(fi)+vx;
    vel[1] = R_dot*sin(fi);

	return 0;
}

static void printPoints(Front* front) {
	if (front->interf->curves != NULL) {
		int  dim = front->rect_grid->dim;
		CURVE* curve = front->interf->curves[0];
		FT_POINT		*p;
		for (BOND* bond = curve->first; bond != NULL; bond = bond->next)
		{
			p = bond->start;
			if (p == NULL)
				(void) fprintf(stderr,"NULL ->\n");
			else
			{
				(void) fprintf(stderr,"%lf ",front->time);
				for (int k = 0; k < dim; ++k)
					(void) fprintf(stderr,"%lf ",Coords(p)[k]);
				(void) fprintf(stderr,"\n");
			}
		}

		if ((curve->last != NULL) && (curve->last->end != NULL))
		{
			p = curve->last->end;
			(void) fprintf(stderr,"%lf ",front->time);
			for (int k = 0; k < dim; ++k)
				(void) fprintf(stderr,"%lf ",Coords(p)[k]);
			(void) fprintf(stderr,"\n");
		}
	}
}

static  void test_propagate(
        Front *front)
{
        double CFL;

	front->max_time = 25;
    // so test runs quickly, changing max_time to 0.005
    front->max_time = 0.0051;
	front->max_step = 40000;
	front->print_time_interval = 25;
	front->movie_frame_interval = 25;

        CFL = Time_step_factor(front);
	Frequency_of_redistribution(front,GENERAL_WAVE) = 1000;

	printf("CFL = %f\n",CFL);
	printf("Frequency_of_redistribution(front,GENERAL_WAVE) = %d\n",
		Frequency_of_redistribution(front,GENERAL_WAVE));

	if (!RestartRun)
	{
            FT_RedistMesh(front);
	    FT_ResetTime(front);

	    // Always output the initial interface.
	    FT_Save(front,out_name);
            FT_AddMovieFrame(front,out_name,binary);

	    // This is a virtual propagation to get maximum front 
	    // speed to determine the first time step.

            FT_Propagate(front);
            FT_SetTimeStep(front);
            FT_SetOutputCounter(front);
	}
	else
	{
            FT_SetOutputCounter(front);
	}

	FT_TimeControlFilter(front);

		printPoints(front);
        for (;;)
        {
	    /* Propagating interface for time step dt */

            FT_Propagate(front);
            FT_AddTimeStepToCounter(front);

	    //Next time step determined by maximum speed of previous
	    //step, assuming the propagation is hyperbolic and
	    //is not dependent on second order derivatives of
	    //the interface such as curvature, and etc.

            FT_SetTimeStep(front);

            printf("\ntime = %f   step = %5d   next dt = %f\n",
                        front->time,front->step,front->dt);
            fflush(stdout);
			printPoints(front);

            if (FT_IsSaveTime(front))
                FT_Save(front,out_name);
            if (FT_IsMovieFrameTime(front))
                FT_AddMovieFrame(front,out_name,binary);

            if (FT_TimeLimitReached(front))
                    break;

	    /* Output section, next dt may be modified */

	    FT_TimeControlFilter(front);
        }
        (void) delete_interface(front->interf);
}       /* end test_propagate */

int localMain(int argc, char **argv)
{
	static Front front;
	static RECT_GRID comp_grid;
	static F_BASIC_DATA f_basic;
	static LEVEL_FUNC_PACK level_func_pack;
	static VELO_FUNC_PACK velo_func_pack;

	f_basic.dim = 2;	
	FT_Init(argc,argv,&f_basic);

	/* Initialize basic computational data */

	f_basic.L[0] = -size;	f_basic.L[1] = -size;
	f_basic.U[0] = size;	f_basic.U[1] = size;
	f_basic.gmax[0] = Nx;	f_basic.gmax[1] = Nx;
	f_basic.boundary[0][0] = f_basic.boundary[0][1] = PERIODIC_BOUNDARY;
	f_basic.boundary[1][0] = f_basic.boundary[1][1] = PERIODIC_BOUNDARY;

        in_name                 = f_basic.in_name;
        restart_state_name      = f_basic.restart_state_name;
        out_name                = f_basic.out_name;
        restart_name            = f_basic.restart_name;
        RestartRun              = f_basic.RestartRun;
        RestartStep             = f_basic.RestartStep;

        sprintf(restart_name,"%s.ts%s",restart_name,right_flush(RestartStep,7));
#if defined(__MPI__)
        sprintf(restart_name,"%s-nd%s",restart_name,right_flush(pp_mynode(),4));
#endif /* defined(__MPI__) */

	FT_StartUp(&front,&f_basic);

	if (!RestartRun)
	{
	    /* Initialize interface through level function */

	    level_func_pack.neg_component = 1;
	    level_func_pack.pos_component = 2;
	    level_func_pack.func_params = NULL;
	    level_func_pack.func = example2d_init_surface_func;
	    level_func_pack.wave_type = FIRST_PHYSICS_WAVE_TYPE;
	    FT_InitIntfc(&front,&level_func_pack);
		if (f_basic.dim < 3)
                FT_ClipIntfcToSubdomain(&front);
	}


	/* Initialize velocity field function */

	velo_func_pack.func_params = NULL;
	velo_func_pack.func = example2d_vel_func;
	velo_func_pack.point_propagate = fourth_order_point_propagate;

	FT_InitVeloFunc(&front,&velo_func_pack);

	/* Propagate the front */

	test_propagate(&front);

	//clean_up(0);
	return 0;
}

// FronTier's cdecs.h defines isnan(x) as isnand(x), which breaks libc++'s <format>
// (pulled in by gtest) wherever std::format of floating point is available.
#ifdef isnan
#undef isnan
#endif
#include <gtest/gtest.h>
TEST(frontier,example2D) {
	std::vector<std::string> argv_storage = {"demo"};
	std::vector<char*> args;
	args.reserve(argv_storage.size() + 1);

	for (std::string& arg : argv_storage) {
		args.push_back(arg.data());
	}
	args.push_back(nullptr);

	localMain(static_cast<int>(argv_storage.size()), args.data());
}
