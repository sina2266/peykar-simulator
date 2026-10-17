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
char in_name[100],out_name[100],sc_name[100];

int main(int argc, char **argv)
{
	static Front front;
	static RECT_GRID comp_grid;

	init_io(argc,argv);
	front_start_up(&front,NULL);

	/* Start testing */

	front.step = 0;
	screen("Restart interface from file: ");
	Scanf("%s\n",s);
        if (s[0] == 'Y' || s[0] == 'y')
        {
            restart = YES;
            screen("Enter the name of the restart file: ");
            Scanf("%s\n",restart_name);
            screen("Enter the step for restart: ");
            Scanf("%d\n",&step);
            sprintf(s,"%s.ts%s",restart_name,right_flush(step,5));
            io_type.file = fopen(s,"r");
            i_intfc(&Init) = front.interf =
                read_print_interface(init_data(&Init),&io_type,NO,&grSet);
            front.step = step;
        }
	else
	    f_init_interface(&front,NULL);
	init_front_velocity(&front,NULL);

	test_propagate(&front);

	clean_up(0);
	return 0;
}


/*ARGSUSED*/
static	void init_io(
	int		argc,
	char		**argv)
{
	argc--;
	argv++;
	strcpy(out_name,"intfc");
	while (argc >= 1)
	{
	    if (argv[0][0] != '-')
	    {
		printf("Usage: testfront -i input -o output\n");
		exit(1);
	    }
	    switch(argv[0][1]) {
	    case 'i':
		strcpy(in_name,argv[1]);
		sprintf(sc_name,"%s.sc",in_name);
		stripcomm(sc_name,in_name);
		stdin = freopen(sc_name,"r",stdin);
		argc -= 2;
		argv += 2;
		break;
	    case 'o':
		zero_scalar(out_name,100);
		strcpy(out_name,argv[1]);
		stdout = freopen(out_name,"w",stdout);
		argc -= 2;
		argv += 2;
		break;
	    }
	}
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
	char c;
	double fcrds[MAXD];
	int  dim = front->rect_grid->dim;
	boolean print_in_binay = NO;

	screen("\n\t\tPrinting Control\n\n");
	screen("Enter maximum time and maximum step of the propagation: ");
	Scanf("%f %d\n",&max_time,&max_step);
	screen("Enter time interval for printing: ");
	Scanf("%f\n",&print_time_interval);
	screen("Enter frame interval for movie: ");
	Scanf("%f\n",&movie_frame_interval);
	Scanf("Print vtk files in binary? (y/n): ");
	Scanf("%c\n",&c);

	if(c == 'y' || c == 'Y')
	    print_in_binary = YES;
	
	redistribute(front,YES,NO);

	printf("dim = %d\n", dim);
	front->time = 0.0;

	print_front_output(front,out_name);
	show_front_output(front,out_name,print_in_binary);

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
	    if (front->time+front->dt > max_time)
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
		    show_front_output(front,out_name,print_in_binary);
		    ++im;
	    }

	    fflush(stdout);
	    if (time_limit_reached || front->step >= max_step) 
		    break;
	}
	(void) delete_interface(front->interf);
}	/* end test_propagate */

