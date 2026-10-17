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
*				testfront.c:
*
*			Test Driver for Front Package:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*/

#include <front/fdecs.h>

	/*  Function Declarations */

static void test_propagate(Front*);
static void init_io( int,char**);
char *in_name,*out_name,sc_name[100];

int main(int argc, char **argv)
{
	static Front front;
	static RECT_GRID comp_grid;
	static F_BASIC_DATA f_basic;

	FT_Init(argc,argv,&f_basic);
	front.sizest = 0;
        in_name                 = f_basic.in_name;
        out_name                = f_basic.out_name;
	sprintf(sc_name,"%s.sc",in_name);
        stripcomm(sc_name,in_name);
        stdin = freopen(sc_name,"r",stdin);
	FT_StartUp(&front,NULL);

	/* Start testing */

	front.step = 0;
	FT_InitIntfc(&front,NULL);
	FT_InitVeloFunc(&front,NULL);

	test_propagate(&front);

	clean_up(0);
	return 0;
}

#define is_output_time(front,i,interval)			\
	((front)->time+front->dt >= (i)*(interval))		

static	void test_propagate(
	Front *front)
{
	int ip,im,status,count;
	Front *newfront;
	double max_time,dt,dt_frac,CFL;
	double print_time_interval,movie_frame_interval;
	int max_step;
	boolean is_print_time, is_movie_time, time_limit_reached;
	char s[10];
	double fcrds[MAXD];
	int  dim = front->rect_grid->dim;
	
	screen("\n\t\tPrinting Control\n\n");
	screen("Enter maximum time and maximum step of the propagation: ");
	Scanf("%f %d\n",&max_time,&max_step);
	screen("Enter time interval for printing: ");
	Scanf("%f\n",&print_time_interval);
	screen("Enter frame interval for movie: ");
	Scanf("%f\n",&movie_frame_interval);

	redistribute(front,YES,NO);

	front->time = 0.0;

	print_front_output(front,out_name);
	show_front_output(front,out_name,NO);

	front->dt = 0.0;
	status = advance_front(front->dt,&dt_frac,front,&newfront,
				(POINTER)NULL);
	ip = im = 1;
	CFL = Time_step_factor(front);
	time_limit_reached = NO;

	for (;;)
	{
	    front->dt = CFL*(*front->max_front_time_step)(front,fcrds);

	    is_print_time = is_movie_time = NO;
	    if (is_output_time(front,im,movie_frame_interval))
	    {
		front->dt = im*movie_frame_interval - front->time;
		is_movie_time = YES;
	    }
	    if (is_output_time(front,ip,print_time_interval))
	    {
		front->dt = ip*print_time_interval - front->time;
		is_print_time = YES;
	    }
	    if (front->time+front->dt >= max_time)
	    {
		    front->dt = max_time - front->time;
		    time_limit_reached = YES;
	    }
	    status = advance_front(front->dt,&dt_frac,front,&newfront,
				(POINTER)NULL);
	    count = 0;
	    while (status == MODIFY_TIME_STEP)
	    {
		    front->dt *= CFL;
		    status = advance_front(front->dt,&dt_frac,front,&newfront,
				(POINTER)NULL);
		    count++;
		    if (count > 10) clean_up(ERROR);
	    }
	    assign_interface_and_free_front(front,newfront);

	    ++front->step;
	    front->time += front->dt;
	    
	    printf("\ntime = %f   step = %5d   dt = %f\n",
			front->time,front->step,front->dt);
	    if (is_print_time ||
		time_limit_reached || front->step >= max_step) 
	    {
		    print_front_output(front,out_name);
		    ++ip;
	    }
	    if (is_movie_time ||
		time_limit_reached || front->step >= max_step) 
	    {
		    show_front_output(front,out_name,NO);
		    ++im;
	    }

	    fflush(stdout);
	    if (time_limit_reached || front->step >= max_step) 
		    break;
	}
	(void) delete_interface(front->interf);
}	/* end test_propagate */

