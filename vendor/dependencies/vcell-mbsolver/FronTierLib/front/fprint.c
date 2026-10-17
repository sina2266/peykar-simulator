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
*				fprint.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Contains printing routines for front debugging and for all front
*	structures.
*
*			debug_front(debug_string,title,front)
*			fshow_curve_states(file,curve)
*			show_curve_states(curve)
*			fshow_surface_states(file,surface)
*			show_surface_states(surface)
*			print_bond_and_states(b,c,fr)
*			f_print_front(fr,file)
*			f_print_Front_structure(fr)
*			print_AFLIN(aflin)
*			f_print_rproblem(rp)
*			f_print_rp_node(rpn,rp)
*
*/


#include <front/fdecs.h>		/* includes int.h, table.h */
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef USE_CGNS
#include "pak/cgnslib_3.0/cgnslib.h"
#endif

#define MAX_NVARS 100

	/* LOCAL Function Prototypes */
LOCAL	void	   fprint_FlowSpecifiedRegion_list(FILE*,Front*);
LOCAL	void	   show_phys_curve_states(INTERFACE*);
LOCAL	void	   show_states_at_point_on_tri(FILE*,int,TRI*,SURFACE*);
#if defined(USE_HDF)
LOCAL	int32	dfnt_size_float(void);
LOCAL	long	fill_hdf_values1d(POINTER,Front*,HDF_plot_data*,double*,double*,
				  double*,double*,int*);
LOCAL	long	fill_hdf_values2d(POINTER,Front*,HDF_plot_data*,double*,double*,
				  double*,double*,int*);
LOCAL	long	fill_hdf_values3d(POINTER,Front*,HDF_plot_data*,double*,double*,
				  double*,double*,int*);
LOCAL	void	pp_collect_hdf_data(long,HDF_plot_data*);
LOCAL	void	print_int32_vector(const char*,int32*,int32,const char*);
LOCAL	void	print_raster_data(INTERFACE*,HDF_frame_data*,
                                  int*,HDF_plot_data*);
LOCAL	void	print_sds_data(Front*,double*,double*,
			       HDF_frame_data*,HDF_plot_data*);
LOCAL	void	set_HDF_plotting_range(Front*,HDF_plot_data*,
				     int*,double*,double*);
LOCAL	void	hdf_plot_comp2d(Front*,char*,boolean);
LOCAL	void	hdf_plot_comp3d(Front*,char*,boolean);
LOCAL	void	hdf_plot_cross_sectional_comp(Front*,char*,int,boolean);
LOCAL	boolean	write_sds_slab(boolean,int32,int32,int32*,int32*,int32*,
			       double**,const char**,const char*,int32,
			       comp_coder_t,comp_info*,VOIDP);
LOCAL	void	hdf_plot_var2d(Front*,char*,char*,double*,COMPONENT, 
		double (*get_state_var)(Locstate),boolean);
LOCAL	void	hdf_plot_var3d(Front*,char*,char*,double*,COMPONENT, 
		double (*get_state_var)(Locstate),int,boolean);
#endif /* defined(USE_HDF) */

#ifdef USE_CGNS
LOCAL	void	show_front_cgns(Front*,char*);
LOCAL 	void 	cgns_plot_vector_field(const char*,Front*);
#endif


#if defined(__GD__)
LOCAL	void 	FrontGDMovie(char*,Front*);
LOCAL	void 	FrontGDMovie1d(char*,Front*);
LOCAL	void 	FrontGDMovie2d(char*,Front*);
LOCAL	void 	gd_plot_var(INTERFACE*,char*,double,char*,double*, /* for 1d */
		double (*get_state_var)(Locstate),int,boolean);
#endif /* defined(__GD__) */
LOCAL   void    xgraph_plot_var(INTERFACE*,char*,double,char*,double*, 
                double (*get_state_var)(Locstate),int); /* for 1d */
LOCAL 	void 	fprint_front_time_stamp(FILE*,Front*);
LOCAL 	void 	vtk_plot_vector_field(const char*,Front*);
LOCAL	void	show_front_gd(Front*,char*);
LOCAL	void	show_front_xg(Front*,char*);
LOCAL	void	show_front_hdf(Front*,char*);
LOCAL	void	show_front_vtk(Front*,char*,boolean);
LOCAL	void	show_front_sdl(Front*,char*);

/*******************************************************************************
*									       *
*		    Printout for Structures in fdecs.h			       *
*									       *
*******************************************************************************/



EXPORT	void	f_fprint_front(
	Front	*front,
	FILE	*file)
{
	(void) fprintf(file,"  ");/*PRINT VERSION = NUMBER OF BLANKS*/
	fprint_interface(file,front->interf);

	if (debugging("grfrsts"))
	    fgraph_front_states(file,front);
	
	(void) fprintf(file,"\n\n");
	(void) foutput(file);
	(void) fprintf(file,"\t\t\tREDISTRIBUTION INFORMATION:\n");
	(void) fprintf(file,"redistribution count = %d\n",
			     Redistribution_count(front));

	(void) foutput(file);
	(void) fprintf(file,"\t\t\tFRONT SPEEDS:\n");
	print_max_front_speed_info(file,front);
	fprint_FlowSpecifiedRegion_list(file,front);
}		/*end f_fprint_front*/


/*
*			f_print_Front_structure():
*/

EXPORT	void f_print_Front_structure(
	Front		*fr)
{
	(void) printf("\n\n\n\t\tFront %p structure\n",fr);
	if (fr == NULL)
	{
	    (void) printf("\t\tstructure not yet allocated\n");
	    (void) printf("\n\t\tEnd Front %p structure\n\n",fr);
	    return;
	}

	print_RECT_GRID_structure(fr->rect_grid);

	(void) printf("Redistribution parameters:\n\t");
	(void) printf("init_redistribute() %p, curve_redist_func() %p\n\t",
		      Init_redistribution_function(fr),
		      Curve_redistribution_function(fr));
	(void) printf("forward_cur_redist() %p backward_cur_redist() %p\n\t",
		      Forward_curve_redistribute_function(fr),
		      Backward_curve_redistribute_function(fr));
	(void) printf("frequency (general %d vector %d node %d) ",
		      Frequency_of_redistribution(fr,GENERAL_WAVE),
		      Frequency_of_redistribution(fr,VECTOR_WAVE),
		      Frequency_of_redistribution(fr,GENERAL_NODE));
	(void) printf("\tnode_redistribute() %p\n",
		      Node_redistribute_function(fr));
	(void) printf("count %d\n\t",Redistribution_count(fr));
	(void) printf("spacing (general %g vector %g) ",
		      Front_spacing(fr,GENERAL_WAVE),
		      Front_spacing(fr,VECTOR_WAVE));
	(void) printf("cos_big_angle (general %g vector %g)\n\t",
		      Cosine_big_angle(fr,GENERAL_WAVE),
		      Cosine_big_angle(fr,VECTOR_WAVE));
	(void) printf("length %g\n",Front_length(fr));

	(void) printf("timestep control:\ttime_step_factor %g\n",
		      Time_step_factor(fr));
	(void) printf("                :\tapply_cfl_at_nodes %s\n",
		      y_or_n(Apply_CFL_at_nodes(fr)));
	(void) printf("                :\tmax_sep %g\n",
		      Max_new_node_separation(fr));
	(void) printf("                :\tcfl_fudge %g\n",
		      Time_step_increase_factor(fr));
	(void) printf("                :\tfrac_floor %g\n",
		      Min_time_step_modification_factor(fr));
	(void) printf("                :\tfrac_ceil %g\n",
		      Max_time_step_modification_factor(fr));
	(void) printf("\n");
	print_max_front_speed_info(stdout,fr);
	(void) printf("\n");

	(void) printf("\nfront movement:\thyperbolic %d\n",fr->hyperbolic);
	(void) printf("\t\t[NO_STATES %d FULL_STATES %d",NO_STATES,FULL_STATES);
	(void) printf("]\n");
	(void) printf("\tnode_propagate() %p\tcurve_propagate() %p\n",
		      fr->node_propagate,fr->curve_propagate);
	(void) printf("\tpoint_propagate() %p\tbond_propagate() %p\n",
		      fr->_point_propagate,fr->bond_propagate);
	(void) printf("\tsnd_node_propagate() %p\ttan_curve_propagate() %p\n",
		      fr->snd_node_propagate,fr->tan_curve_propagate);
	(void) printf("\t_npt_tang_solver() %p\t\n",fr->_npt_tang_solver);
	(void) printf("\t_one_side_npt_tang_solver() %p\t\n",
		      fr->_one_side_npt_tang_solver);
	(void) printf("\timpose_bc() %p\n",fr->impose_bc);

	(void) printf("\nstate variables:\tsizest %d\tnfloats %d\n",
		      (int)fr->sizest,fr->nfloats);
	(void) printf("_state_interpolator() %p  print_state() %p\n",
		      fr->_state_interpolator,fr->print_state);
	(void) printf("_fgraph_front_states() %p\n",
		      fr->_fgraph_front_states);
	(void) printf("_fprint_header_for_graph_curve_states() %p\n",
		      fr->_fprint_header_for_graph_curve_states);
	(void) printf("_fgraph_curve_states() %p\n",
		      fr->_fgraph_curve_states);
	(void) printf("transform_state() %p\n",fr->transform_state);

	(void) printf("interactions:\tfr_bdry_untangle() %p"
		      "\tuntangle_front() %p\n",
		      fr->fr_bdry_untangle,fr->untangle_front);
	(void) printf("\tB_node_bifurcation() %p\ttwodrproblem() %p\n",
		      fr->B_node_bifurcation,fr->twodrproblem);

	(void) printf("\ninterface:\tinterf %llu\n",
		      interface_number(fr->interf));

	(void) printf("\n\t\tEnd Front %p structure\n\n",fr);

}		/*end f_print_Front_structure*/



EXPORT	void print_AFLIN(
	FILE		*file,
	AFLIN		*aflin,
	int		dim)
{
	int		i, j;

	(void) fprintf(file,"\n\n\n\t\tAFLIN %p structure\n",aflin);
	if (aflin == NULL)
	{
	    (void) fprintf(file,"\t\tstructure not yet allocated\n");
	    (void) fprintf(file,"\n\t\tEnd AFLIN %p structure\n\n",aflin);
	    return;
	}
	for (i = 0; i < dim; ++i)
	{
	    for (j = 0; j < dim; ++j)
	    	(void) fprintf(file,"a[%d][%d] %g ",i,j,aflin->a[i][j]);
	    (void) fprintf(file,"\n");
	}
	(void) fprintf(file,"\n");
	for (i = 0; i < dim; ++i)
	    (void) fprintf(file,"b[%d] %g ",i,aflin->b[i]);
	(void) fprintf(file,"det %g\n",aflin->det);
	(void) fprintf(file,"\n\t\tEnd AFLIN %p structure\n\n",aflin);
}		/*end print_AFLIN*/

EXPORT	void	print_time_step_status(
	const char *mesg,
	int	   status,
	const char *end)
{
	(void) printf("%s%s%s",mesg,time_step_status_as_string(status),end);
}		/*end print_time_step_status*/

EXPORT	const char* time_step_status_as_string(
	int	   status)
{
	static char s[120];
	switch (status)
	{
	case GOOD_STEP:
	    return "GOOD_STEP";
	case MODIFY_TIME_STEP:
	    return "MODIFY_TIME_STEP";
	case REPEAT_TIME_STEP:
	    return "REPEAT_TIME_STEP";
	case ERROR_IN_STEP:
	    return "ERROR_IN_STEP";
	default:
	    (void) sprintf(s,"UNKNOWN %d",status);
	    return s;
	}
}		/*end time_step_status_as_string*/

EXPORT	void	f_fprint_max_front_speed_info(
	FILE	*file,
	Front	*front)
{
	int	i, j, dim = front->rect_grid->dim;

	(void) fprintf(file,"Maximum Front Speed Information\n");
	(void) fprintf(file,"Spfr = ");
	if (is_binary_output() == YES)
	{
	    (void) fprintf(file,"\f%c",dim+1);
	    (void) fwrite((const void *) Spfr(front),sizeof(double),dim+1,file);
	}
	else
	{
	    for (i = 0; i < dim; ++i)
	    	(void) fprintf(file,"%"FFMT", ",Spfr(front)[i]);
	    (void) fprintf(file,"%"FFMT,Spfr(front)[dim]);
	}
	(void) fprintf(file,"\n");
	for (i = 0; i <= dim; ++i)
	{
	    (void) fprintf(file,"MaxFrontSpeedCoords[%d] = ",i);
	    if (is_binary_output() == YES)
	    {
	        (void) fprintf(file,"\f%c",dim);
	    	(void) fwrite((const void *) MaxFrontSpeedCoords(front)[i],
			      sizeof(double),dim,file);
	    }
	    else
	    {
	    	(void) fprintf(file,"%"FFMT,MaxFrontSpeedCoords(front)[i][0]);
		for (j = 1; j < dim; ++j)
		    (void) fprintf(file,", %"FFMT,
				   MaxFrontSpeedCoords(front)[i][j]);
	    }
	    (void) fprintf(file,"\n");
	}
	for (i = 0; i <= dim; ++i)
	{
	    (void) fprintf(file,"MaxFrontSpeedState[%d] = ",i);
	    fprint_state_data(file,MaxFrontSpeedState(front)[i],front->interf);
	}
}		/*end f_fprint_max_front_speed_info*/


#include <sys/types.h>
#include <sys/stat.h>

/*
*			debug_front():
*
*	Prints the interface of a front if debugging is requested.
*	There is a dual control structure.  The first involves 
*	debug_string, which controls whether any debugging is printed
*	based on the location of the call to debug_front().  The 
*	second determines what kind of information to print out.
*
*	The current choices for debug_string from advance_front() are:
*		old_front	initial front into advance_front
*		cp_front	after normal curve propagation
*		2drp_front	after rp loop
*		node_front	after node loop
*		ztc_front	before/after zero timestep correction
*		dec_front	after delete_exterior_curves()
*		dvsb_front	after delete_very_short_bonds()
*		dfbb_front	after delete_fold_back_bonds()
*		snd_front	after snd_node_prop()
*		tcp_front	after tan curve propagate
*		dspr_front	after delete_phys_remn_on_bdry()
*		redist_front	after redistribution
*		dsloop_front	after delete_small_loops()
*		ERROR_front	after any error in the front propagate
*	Debugging on front basically turns on all of these.
*
*	The current choices for type of output are:
*		front_intfc	call print_interface()
*		graph_front_states	call graph_front_states()
*		front_states	call show_intfc_states()
*		phys_fr_states	call show_phys_curve_states()
*		plot_front	call plot_interface()
*
*	NOTE:  to anyone who adds a call(s) to this function, PLEASE
*		update these lists.
*
*	NOTE 2: It is not sufficient to debug on a string from the first
*		list.  You must also debug on at least one string from
*		the second list in order to get any output.
*/

EXPORT void debug_front(
	const char	*debug_string,
	const char	*title,
	Front		*fr)
{
	boolean deb = (debugging(debug_string) || debugging("front")) ? YES : NO;

	if (debugging("time_step"))
	{
	    (void) printf("Spfr(fr) %s = ",title);
	    print_general_vector("",Spfr(fr),fr->rect_grid->dim+1,"\n");
	}

	if (!((strcmp(debug_string,"ERROR_front")==0) || deb))
	    return;

	if (deb && debugging("front_intfc"))
	{
	    (void) output();
	    (void) printf("\t\tFront %s\n",title);
	    print_interface(fr->interf);
	}
	if (deb && debugging("graph_fr")) 
	{
	    (void) output();
	    (void) printf("\t\tGraph of front states %s\n",title);
	    graph_front_states(fr);
	}
	if (deb && debugging("front_states")) 
	{
	    (void) output();
	    (void) printf("\t\tFront states %s\n",title);
	    show_intfc_states(fr->interf);
	}
	if (deb && debugging("phys_fr_states") && fr->rect_grid->dim==2) 
	{
	    (void) output();
	    (void) printf("\t\tPhysical curve states %s\n",title);
	    show_phys_curve_states(fr->interf);
	}
	if (deb && debugging("plot_front"))
	{
	    static int     ctr = 0;
	    static int     step = -1;
	    static boolean first = YES;
	    char           fname[1024];
	    static const char     *dname = "debug_front";
	    static const char     *funcname = "debug_front()";

	    if (first == YES)
	    {
	    	first = NO;
		if (create_directory(dname,YES) == FUNCTION_FAILED)
	    	{
	    	    screen("ERROR in %s directory %s doesn't exist ",
		           "and can't be made\n",funcname,dname);
		    clean_up(ERROR);
		}
	    }
	    if (step < 0 || fr->step != step)
	    {
	    	step = fr->step;
	    	ctr = 0;
	    }
	    (void) sprintf(fname,"%s/plt",dname);
	    plot_interface(fr->interf,fname,&step,&ctr,title);
	    ++ctr;
	}
}		/*end debug_front*/


LOCAL	void	fprint_FlowSpecifiedRegion_list(
	FILE	*file,
	Front	*fr)
{
	FlowSpecifiedRegion *fsr;
	int n;

	(void) foutput(file);
	(void) fprintf(file,"FLOW SPECIFIED REGIONS DATA LIST\n");
	for (n = 0, fsr = Fsr_list(fr); fsr != NULL; ++n, fsr = fsr->next)
	    ;
	(void) fprintf(file,"Number of regions = %d\n",n);

	for (fsr = Fsr_list(fr); fsr != NULL; fsr = fsr->next)
	    fprint_FlowSpecifiedRegion_data(file,fsr,fr);

	(void) foutput(file);
	(void) fprintf(file,"END FLOW SPECIFIED REGIONS DATA LIST\n");
}		/*end fprint_FlowSpecifiedRegion_list*/

/*ARGSUSED*/
EXPORT	void	f_fprint_FlowSpecifiedRegion_data(
	FILE			*file,
	FlowSpecifiedRegion	*fsr,
	Front			*fr)
{
	(void) fprintf(file,"\nData for FlowSpecifiedRegion structure %p\n",
		       fsr);
	(void) fprintf(file,"comp = %d\n",fsr->comp);
	(void) fprintf(file,"type = %s\n",fsr->type);
}		/*end f_fprint_FlowSpecifiedRegion_data*/


EXPORT	void	fprint_Tan_stencil(
	FILE		*file,
	Front		*front,
	Tan_stencil	*sten)
{
	int		i;
	int		dim = front->rect_grid->dim;
	int		nrad = sten->npts/2;
	char		s[80];

	(void) fprintf(file,"Data for Tan_stencil %p\n",sten);
	(void) sprintf(s,"Tangent direction: ");
	fprint_general_vector(file,s,sten->dir,dim,"\n");
	(void) fprintf(file,"npts = %d\n",sten->npts);
	for (i = -nrad; i <= nrad; ++i)
	{
	    (void) sprintf(s,"Coords(sten->p[%2d]) = ",i);
	    fprint_general_vector(file,s,Coords(sten->p[i]),dim,"\n");
	}
	for (i = -nrad; i <= nrad; ++i)
	{
	    (void) fprintf(file,"sten->t[%2d] = %g\n",i,sten->t[i]);
	}
	(void) fprintf(file,"Left states:\n");
	for (i = -nrad; i <= nrad; ++i)
	{
	    fprint_state_data(file,sten->leftst[i],front->interf);
	}
	(void) fprintf(file,"Right states:\n");
	for (i = -nrad; i <= nrad; ++i)
	{
	    fprint_state_data(file,sten->rightst[i],front->interf);
	}
	(void) fprintf(file,"End of stencil\n\n");
}		/*end fprint_Tan_stencil*/

EXPORT	void	fprint_Nor_stencil(
	FILE		*file,
	Front		*front,
	Nor_stencil	*sten)
{
	int		i;
	int		dim = front->rect_grid->dim;
	int		nrad = sten->npts/2;
	char		s[80];

	(void) fprintf(file,"Data for Nor_stencil %p\n",sten);
	(void) sprintf(s,"Nor direction: ");
	fprint_general_vector(file,s,sten->nor,dim,"\n");
	(void) fprintf(file,"component = %d  curvature = %f\n",sten->comp,
				sten->curvature);
	(void) fprintf(file,"npts = %d\n",sten->npts);
	for (i = 0; i <= nrad; ++i)
	{
	    (void) sprintf(s,"sten->pts[%d] = ",i);
	    fprint_general_vector(file,s,sten->pts[i],dim,"\n");
	}
	(void) fprintf(file,"End of stencil\n\n");
}		/*end fprint_Nor_stencil*/

EXPORT	void	print_Tan_stencil(
	Front		*front,
	Tan_stencil	*sten)
{
	fprint_Tan_stencil(stdout,front,sten);
}		/*end print_Tan_stencil*/

EXPORT	void	print_Nor_stencil(
	Front		*front,
	Nor_stencil	*sten)
{
	fprint_Nor_stencil(stdout,front,sten);
}		/*end print_Tan_stencil*/

/*
*			f_print_rproblem():
*/

EXPORT void f_print_rproblem(
	RPROBLEM	*rp)
{
	RP_NODE		*rpn;

	(void) printf("\n\t\tRPROBLEM %p:\n",rp);
	if (!rp)
	{
	    (void) printf("\tEND OF RPROBLEM %p:\n\n",rp);
	    return;
	}

	(void) printf("prev %p  next %p\n",rp->prev,rp->next);
	(void) printf("new_intfc %llu  old_intfc %llu  ",
		      interface_number(rp->new_intfc),
		      interface_number(rp->old_intfc));
	(void) printf("dt %g  dt_frac %g\n\n",rp->dt,rp->dt_frac);

	/* 
	*	Even though these sets of interacting nodes are
	*	reprinted in the rp_node printout below, it is
	*	convenient to print the node list separately in 
	*	order to see the list of interacting nodes as
	*	a single object.  The problem is that the
	*	print_rp_node() function prints so much that
	*	it is difficult to remember what are the other
	*	interacting nodes in the list.
	*/

	(void) printf("Old Interacting Nodes\n");
	for (rpn = rp->first_rp_node; rpn; rpn = rpn->next)
	    print_node(rpn->old_node);
	(void) printf("End Old Interacting Nodes\n");

	(void) printf("New Interacting Nodes\n");
	for (rpn = rp->first_rp_node; rpn; rpn = rpn->next)
	{
	    print_node(rpn->node);
	    print_propagation_status(rpn->node);
	    (void) printf("\n\n");
	}
	(void) printf("End New Interacting Nodes\n");

	(void) printf("\t\tRp_nodes:\n");
	for (rpn = rp->first_rp_node; rpn; rpn = rpn->next)
	    print_rp_node(rpn,rp);
	(void) printf("\tEnd Rp_nodes\n\n");

	(void) printf("\tAngle ordered curves:\n");
	print_o_curve_family(rp->ang_ordered_curves);
	(void) printf("\tEnd Angle ordered curves\n\n");

	(void) printf("\tOld Angle ordered curves:\n");
	print_o_curve_family(rp->old_ang_ordered_curves);
	(void) printf("\tEnd Old Angle ordered curves:\n\n");

	(void) printf("\t\tBoundary curves:\n");
	print_o_curve_family(rp->bdry_curves);
	(void) printf("\tEnd Boundary curves\n\n");

	(void) printf("\tOld Boundary curves:\n");
	print_o_curve_family(rp->old_bdry_curves);
	(void) printf("\tEnd Old Boundary curves\n\n");

	(void) printf("\tStatistical information\n");
	(void) printf("bdry_type1 %d  bdry_type2 %d\n",
		      rp->bdry_type1,rp->bdry_type2);
	(void) printf("num_nd %d  num_fxd %d  num_srce %d\n",
		      rp->num_nd,rp->num_fxd,rp->num_srce);
	(void) printf("num_nod %d  num_bdry_nod %d  num_phys %d\n",
		      rp->num_nod,rp->num_bdry_nod,rp->num_phys);
	(void) printf("\tEnd Statistical information\n\n");

	user_print_rproblem(rp);
	(void) printf("\tEND OF RPROBLEM %p:\n\n",rp);
}		/*end f_print_rproblem*/


/*
*			f_print_rp_node():
*/

/*ARGSUSED*/
EXPORT void f_print_rp_node(
	RP_NODE		*rpn,
	RPROBLEM	*rp)
{
	(void) printf("\t\tRp_node %p:\n",rpn);
	if (!rpn)
	{
	    (void) printf("\tEnd of Rp_node %p:\n\n",rpn);
	    return;
	}

	(void) printf("prev %p  next %p\n\n",rpn->prev,rpn->next);

	(void) printf("(new) node - "); print_node(rpn->node);
	(void) printf("old node ");	 print_node(rpn->old_node);
	(void) printf("states_assigned_at_node %s\n",
		      (rpn->states_assigned_at_node) ? "YES" : "NO");

	(void) printf("\t\tBoundary curves:\n");
	(void) printf("\tNeumann1:\n");	print_o_curve(rpn->neumann1);
	(void) printf("\tNeumann2:\n");	print_o_curve(rpn->neumann2);
	(void) printf("\tDirichlet1:\n");	print_o_curve(rpn->dirichlet1);
	(void) printf("\tDirichlet2:\n");	print_o_curve(rpn->dirichlet2);
	(void) printf("\tSubdomain1:\n");	print_o_curve(rpn->subdomain1);
	(void) printf("\tSubdomain2:\n");	print_o_curve(rpn->subdomain2);
	(void) printf("\tEnd Boundary curves\n\n");
	
	user_print_rp_node(rpn,rp);
	(void) printf("\tEnd of Rp_node %p:\n\n",rpn);
}		/*end f_print_rp_node*/


EXPORT	void	print_untangle_status(
	int		status)
{
	(void) printf("status = %s\n",untangle_status_as_string(status));
}		/*end print_untangle_status*/

EXPORT	void print_propagation_status(
	NODE		*node)
{
	(void) printf("Propagation status of node %llu = ",node_number(node));
	if (node == NULL)
	{
	    (void) printf("NULL NODE\n");
	    return;
	}
	(void) printf("%s\n",
		      propagation_status_as_string(propagation_status(node)));
}		/*end print_propagation_status*/


EXPORT	const char *untangle_status_as_string(
	int		status)
{
	static char s[120];
	switch (status)
	{
	case CURVES_UNTANGLED:
	    return "CURVES_UNTANGLED";
	case MODIFY_TIME_STEP_TO_UNTANGLE:
	    return "MODIFY_TIME_STEP_TO_UNTANGLE";
	case ERROR_IN_UNTANGLE:
	    return "ERROR_IN_UNTANGLE";
	default:
	    (void) sprintf(s,"UNKNOWN UNTANGLE STATUS %d",status);
	    return s;
	}
}		/*end untangle_status_as_string*/

EXPORT	const char *propagation_status_as_string(
	NODE_PROPAGATION_STATUS	status)
{
	static char s[120];
	switch (status)
	{
	case UNPROPAGATED_NODE:
	    return "UNPROPAGATED_NODE";
	case VEL_COMPUTED_NODE:
	    return "VEL_COMPUTED_NODE";
	case PROPAGATED_NODE:
	    return "PROPAGATED_NODE";
	case DELETED_NODE:
	    return "DELETED_NODE";
	default:
	    (void) sprintf(s,"UNKNOWN PROPAGATION STATUS %d",status);
	    return s;
	}
}		/*end propagation_status_as_string*/


LOCAL	void show_phys_curve_states(
	INTERFACE	*intfc)
{
	CURVE		**c;

	if (intfc->dim != 2) return;

	if (size_of_state(intfc) == 0)
	{
	    (void) printf("No states ft_assigned on intfc\n");
	    return;
	}

	(void) printf("\t\tSTATES ON THE FRONT\n\n");
	for (c = intfc->curves;  *c != NULL;  ++c)
	{
	    if (wave_type(*c) < FIRST_PHYSICS_WAVE_TYPE)
	        continue;
	    show_curve_states(*c);
	}
	(void) printf("\n\n");
	(void) printf("\t\tEND OF STATES ON THE FRONT\n\n");
}		/*end show_phys_curve_states*/


/*
*		show_curve_states():
*		fshow_curve_states():
*
*	Prints states on a curve using print_intfc_state().
*/

EXPORT	void	show_curve_states(
	CURVE		*c)
{
	fshow_curve_states(stdout,c);
}		/*end show_curve_states*/

EXPORT	void fshow_curve_states(
	FILE		*file,
	CURVE		*c)
{
	INTERFACE	*intfc;
	Locstate	sl, sr;
	BOND		*b;
	size_t		sizest;
	const char	**Dnm = computational_grid(c->interface)->Remap.Dnm;

	if (c == NULL)
	    return;
	intfc = c->interface;

	sizest = size_of_state(intfc);

	if (sizest == 0)
	{
	    (void) fprintf(file,"No states ft_assigned on curve\n");
	    return;
	}

	(void) fprintf(file,"\t\tSTATES ON CURVE\n\n");
	(void) fprintf(file,"    %-10s %-10s %-10s %llu ",Dnm[0],Dnm[1],
		       "STATES on CURVE_",curve_number(c));
	(void) fprintf(file,"%s\n",(is_bdry(c)) ? "Boundary curve" :
						  "Interior curve");

	if (sizest == FLOAT)
	{
	    (void) fprintf(file,"\t\t\t    l_st\tr_st\n");

	    sl = left_start_state(c);       sr = right_start_state(c);
	    (void) fprintf(file,"start curve states ");
	    (void) fprintf(file,"%-11g %-11g\n",
	    	           ((double *)sl)[0],((double *)sr)[0]);
 
	    sl =  left_state(c->start->posn);
	    sr = right_state(c->start->posn);
	    (void) fprintf(file,"start posn states ");
	    (void) fprintf(file,"%-11g %-11g\n",
			   ((double *)sl)[0],((double *)sr)[0]);
	    (void) fprintf(file,"\n");

	    b = c->first;
	    slsr(b->start,Hyper_surf_element(b),Hyper_surf(c),&sl,&sr);

	    (void) fprintf(file,"%-11g %-11g  ",
	    	           Coords(b->start)[0],Coords(b->start)[1]);
	    (void) fprintf(file,"%-11g %-11g\n",
	    	           ((double *)sl)[0],((double *)sr)[0]);

	    while (b != NULL)
	    {
	    	slsr(b->end,Hyper_surf_element(b),Hyper_surf(c),&sl,&sr);

		(void) fprintf(file,"%-11g %-11g  ",
				    Coords(b->end)[0],Coords(b->end)[1]);
		(void) fprintf(file,"%-11g %-11g\n",
				    ((double *)sl)[0],((double *)sr)[0]);

		b = b->next;
	    }
	    (void) fprintf(file,"\n");

	    sl = left_end_state(c);         sr = right_end_state(c);
	    (void) fprintf(file,"end curve states ");
	    (void) fprintf(file,"%-11g %-11g\n",
			   ((double *)sl)[0],((double *)sr)[0]);

	    sl =  left_state(c->end->posn);
	    sr = right_state(c->end->posn);
	    (void) fprintf(file,"end posn states ");
	    (void) fprintf(file,"%-11g %-11g\n",
			   ((double *)sl)[0],((double *)sr)[0]);
	    (void) fprintf(file,"\n");
	}
	else if (sizest == 2 * FLOAT)
	{
	    (void) fprintf(file,"\t\t\t    l_st\t\tr_st\n");
	    b = c->first;
	    slsr(b->start,Hyper_surf_element(b),Hyper_surf(c),&sl,&sr);

	    (void) fprintf(file,"%-11g %-11g  ",
	    	                Coords(b->start)[0],Coords(b->start)[1]);
	    (void) fprintf(file,"%-11g %-11g " ,
			        ((double *)sl)[0],((double *)sl)[1]);
	    (void) fprintf(file,"%-11g %-11g\n",
			        ((double *)sr)[0],((double *)sr)[1]);

	    while (b != NULL)
	    {
	    	slsr(b->end,Hyper_surf_element(b),Hyper_surf(c),&sl,&sr);

		(void) fprintf(file,"%-11g %-11g  ",
				    Coords(b->end)[0],Coords(b->end)[1]);
		(void) fprintf(file,"%-11g %-11g ",
				    ((double *)sl)[0],((double *)sl)[1]);
		(void) fprintf(file,"%-11g %-11g\n",
				    ((double *)sr)[0],((double *)sr)[1]);

		b = b->next;
	    }
	    (void) fprintf(file,"\n");
	}
	else
	{
	    b = c->first;
	    sl = left_start_state(c);
	    sr = right_start_state(c);

	    (void) fprintf(file,"%g %g\n",
			        Coords(b->start)[0],Coords(b->start)[1]);
	    (void) fprintf(file,"\t\tl_st ");
	    fprint_intfc_state(file,sl,intfc);
	    (void) fprintf(file,"\t\tr_st ");
	    fprint_intfc_state(file,sr,intfc);

	    for (; b->next != NULL; b = b->next)
	    {
	    	slsr(b->end,Hyper_surf_element(b),Hyper_surf(c),&sl,&sr);

		(void) fprintf(file,"%g %g\n",
				    Coords(b->end)[0],Coords(b->end)[1]);
		(void) fprintf(file,"\t\tl_st ");
		fprint_intfc_state(file,sl,intfc);
		(void) fprintf(file,"\t\tr_st ");
		fprint_intfc_state(file,sr,intfc);
	    }

	    sl = left_end_state(c);
	    sr = right_end_state(c);

	    (void) fprintf(file,"%g %g\n",Coords(b->end)[0],Coords(b->end)[1]);
	    (void) fprintf(file,"\t\tl_st ");
	    fprint_intfc_state(file,sl,intfc);
	    (void) fprintf(file,"\t\tr_st ");
	    fprint_intfc_state(file,sr,intfc);

	    (void) fprintf(file,"\n");
	}
	(void) fprintf(file,"\n");
	(void) fprintf(file,"\n");
	(void) fprintf(file,"\t\tEND OF STATES ON CURVE %llu\n\n",
		       curve_number(c));
}		/*end fshow_curve_states*/


EXPORT void print_bond_and_states(
	BOND		*b,
	CURVE		*c,
	Front		*fr)
{
	if (b == NULL)
	{
	    (void) printf("bond = NULL\n");
	    return;
	}
	(void) printf("bond (%llu):  %g %g -> %g %g\n",
		      bond_number(b,fr->interf),
		      Coords(b->start)[0],Coords(b->start)[1],
		      Coords(b->end)[0],Coords(b->end)[1]);
	(void) printf("prev = %llu next = %llu\n",bond_number(b->prev,fr->interf),
		      bond_number(b->next,fr->interf));
	if (fr->sizest)
	{
	    (void) printf("left/right states at b->start:\n");
	    (*fr->print_state)(left_state_at_point_on_curve(b->start,b,c));
	    (*fr->print_state)(right_state_at_point_on_curve(b->start,b,c));
	    (void) printf("left/right states at b->end:\n");
	    (*fr->print_state)(left_state_at_point_on_curve(b->end,b,c));
	    (*fr->print_state)(right_state_at_point_on_curve(b->end,b,c));
	}
}		/*end print_bond_and_states*/

EXPORT	void	fprint_redistribution_direction(
	FILE                     *file,
	const char               *mesg,
	REDISTRIBUTION_DIRECTION dir,
	const char               *end)
{
	(void) fprintf(file,"%s%s%s",mesg,
				     redistribution_direction_as_string(dir),
				     end);
}		/*end fprint_redistribution_direction*/

EXPORT	const char *redistribution_direction_as_string(
	REDISTRIBUTION_DIRECTION direction)
{
	static char s[120];
	switch (direction)
	{
	case FORWARD_REDISTRIBUTION:
	    return "FORWARD_REDISTRIBUTION";
	case BACKWARD_REDISTRIBUTION:
	    return "BACKWARD_REDISTRIBUTION";
	default:		    
	    (void) sprintf(s,"UNKNOWN REDISTRIBUTION DIRECTION %d",direction);
	    return s;
	}
}		/*end redistribution_direction_as_string*/

/*
*		show_surface_states():
*		fshow_surface_states():
*
*	Prints states on a surface using print_intfc_state().
*/

EXPORT	void show_surface_states(
	SURFACE		*s)
{
	fshow_surface_states(stdout,s);
}		/*end show_surface_states*/

EXPORT	void fshow_surface_states(
	FILE		*file,
	SURFACE		*s)
{
	INTERFACE	*intfc;
	TRI		*tri;
	size_t		sizest;
	int		i;

	if (s == NULL)
	    return;
	intfc = s->interface;

	sizest = size_of_state(intfc);

	if (sizest == 0)
	{
	    (void) fprintf(file,"No states ft_assigned on surface\n");
	    return;
	}

	(void) fprintf(file,"\t\tSTATES ON %s SURFACE %llu, ",
		     (is_bdry(s)) ? "BOUNDARY" : "INTERIOR",surface_number(s));
	fprint_wave_type(file,"WAVE TYPE = ",wave_type(s),"\n",intfc);
	(void) fprintf(file,
		"\tPositive Component = %-4d   Negative Component = %-4d    ",
		positive_component(s),negative_component(s));
	(void) fprintf(file,"%s\n\n",
		       is_bdry(s) ? "Boundary Surface" : "Interior Surface");

	(void) fprintf(file,"States on Tris\n");
	for (i = 0, tri = first_tri(s); !at_end_of_tri_list(tri,s);
	     					++i, tri = tri->next)
	{
	    (void) fprintf(file,"States on Tri %d, Boundary = %d\n",
	    	                i,Boundary_tri(tri));

	    show_states_at_point_on_tri(file,0,tri,s);
	    show_states_at_point_on_tri(file,1,tri,s);
	    show_states_at_point_on_tri(file,2,tri,s);

	    (void) fprintf(file,"End States on Tri %d\n",i);
	}
	(void) fprintf(file,"\n");
	(void) fprintf(file,"\t\tEND OF STATES ON SURFACE\n\n");

}		/*end fshow_surface_states*/


LOCAL	void	show_states_at_point_on_tri(
	FILE	*file,
	int	i,
	TRI	*tri,
	SURFACE	*s)
{
	INTERFACE	*intfc = s->interface;
	Locstate	sl, sr;
	FT_POINT		*p = Point_of_tri(tri)[i];

	(void) fprintf(file,"tri->p%d %g %g %g, Boundary = %d\n",i+1,
		      Coords(p)[0],Coords(p)[1],Coords(p)[2],Boundary_point(p));
	slsr(p,Hyper_surf_element(tri),Hyper_surf(s),&sl,&sr);
	(void) fprintf(file,"\t\tl_st ");
	fprint_intfc_state(file,sl,intfc);
	(void) fprintf(file,"\t\tr_st ");
	fprint_intfc_state(file,sr,intfc);
}		/*end show_states_at_point_on_tri*/

EXPORT	void print_tri_states(
	TRI                *tri,
	HYPER_SURF	   *hs)
{
	INTERFACE *intfc = hs->interface;
	HYPER_SURF_ELEMENT *hse = Hyper_surf_element(tri);
	Locstate  sl, sr;
	FT_POINT     *p;
	int       i;

	for (i = 0; i < 3; ++i)
	{
	    p = Point_of_tri(tri)[i];
            /*slsr(p,hse,hs,&sl,&sr); */

	    sl = left_state(p);
	    sr = right_state(p);

	    (void) printf("left state at p[%d]\n",i);
	    print_state_data(sl,intfc);
	    (void) printf("right state at p[%d]\n",i);
	    print_state_data(sr,intfc);
	}
}		/*end print_tri_states*/

EXPORT  void    f_gview_plot_interface(
	const char *dname,
	INTERFACE  *intfc)
{
	RECT_GRID *gr = computational_grid(intfc);
	geomview_interface_plot(dname,intfc,gr);
}               /*end f_default_gview_plot_interface*/

EXPORT	void print_node_status(
	const char *mesg,
	int	   status,
	const char *end)
{
	(void) printf("%s%s%s",mesg,node_status_as_string(status),end);
}		/*end print_node_status*/

EXPORT	const char	*node_status_as_string(
	int	status)
{
	static char s[120];
	switch (status)
	{
	case GOOD_NODE:
	    return "GOOD_NODE";
	case PSEUDOCROSS_NODE_NODE:
	    return "PSEUDOCROSS_NODE_NODE";
	case CROSS_NODE_NODE:
	    return "CROSS_NODE_NODE";
	case CROSS_PAST_CURVE_NODE:
	    return "CROSS_PAST_CURVE_NODE";
	case PSEUDOCROSS_CURVE_NODE:
	    return "PSEUDOCROSS_CURVE_NODE";
	case CROSS_CURVE_NODE:
	    return "CROSS_CURVE_NODE";
	case NO_CROSS_NODE:	 
	    return "NO_CROSS_NODE";
	case BIFURCATION_NODE:
	    return "BIFURCATION_NODE";
	case ERROR_NODE:	
	    return "ERROR_NODE";
	case NO_STORAGE_NODE:	
	    return "NO_STORAGE_NODE";
	case MODIFY_TIME_STEP_NODE:
	    return "MODIFY_TIME_STEP_NODE";
	case REPEAT_TIME_STEP_NODE: 
	    return "REPEAT_TIME_STEP_NODE";
	default:		    
	    (void) sprintf(s,"UNKNOWN NODE STATUS %d",status);
	    return s;
	}
}		/*end node_status_as_string*/

EXPORT void print_front_output(
	Front *front,
	char *out_name)
{
	FILE *out_file;
	char dirname[200];
	char intfc_name[200],comp_name[200];
	int step = front->step;
	int dim = front->rect_grid->dim;
	int numnodes = pp_numnodes();
	boolean save_binary_output = is_binary_output();

	sprintf(dirname,"%s/gv.ts%s",out_name,right_flush(step,7));
	if (numnodes > 1)
	    sprintf(dirname,"%s-nd%s",dirname,right_flush(pp_mynode(),4));

	if (dim != 1)
	    gview_plot_interface(dirname,front->interf);
	sprintf(intfc_name,"%s/intfc-ts%s",out_name,right_flush(step,7));
	if (numnodes > 1)
	    sprintf(intfc_name,"%s-nd%s",intfc_name,right_flush(pp_mynode(),4));

	out_file = fopen(intfc_name,"w");
	print_title(out_file,"");
	fprint_front_time_stamp(out_file,front);
	fprintf(out_file,"\n#");
	fprint_interface(out_file,front->interf);
	fclose(out_file);
	if (front->rect_grid->dim == 2)
	{
	    sprintf(comp_name,"%s/comp.ts%s",out_name,right_flush(step,7));
	    if (numnodes > 1)
	        sprintf(comp_name,"%s-nd%s",comp_name,right_flush(pp_mynode(),4));
	    out_file = fopen(comp_name,"w");
	    (void) make_bond_comp_lists(front->interf);
	    show_COMP(out_file,front->interf);
	    fclose(out_file);
	}
	set_binary_output(save_binary_output);
}	/* end print_front_output */

LOCAL	void show_front_vtk(
	Front *front,
        char *out_name,
        boolean print_in_binary)
{
	char dirname[256];
        int step = front->step; 
	int dim = front->rect_grid->dim;

	if (dim == 1) return;

	/* Create vtk directories */

        sprintf(dirname,"%s/vtk.ts%s",out_name,right_flush(step,7));
	if (pp_numnodes() > 1)
            sprintf(dirname,"%s-nd%s",dirname,right_flush(pp_mynode(),4));
	if (!create_directory(dirname,YES))
	{
	    screen("Cannot create directory %s\n",dirname);
	    clean_up(ERROR);
	}
        vtk_interface_plot(dirname,front->interf,print_in_binary,
					front->time,front->step);
	if (front->vtk_movie_var != NULL)
	    vtk_plot_vector_field(dirname,front);
}	/* end show_front_vtk */

LOCAL	void show_front_hdf(
	Front *front,
        char *out_name)
{
	char dirname[256];
        int step = front->step; 
	static boolean first = YES;
	int dim = front->rect_grid->dim;

	if (dim == 1) return;
#if defined(USE_HDF)
	/* Create HDF directory */
	sprintf(dirname,"%s/hdf",out_name);
	if (first && pp_mynode() == 0)
	{
	    if (!create_directory(dirname,NO))
	    {
	    	screen("Cannot create directory %s\n",dirname);
	    	clean_up(ERROR);
	    }
	}
        if (dim == 2)
        {
	    hdf_plot_comp2d(front,dirname,first);
	    if (front->hdf_movie_var != NULL)
	    {
		HDF_MOVIE_VAR *hdf_movie_var = front->hdf_movie_var;
		int i,num_var = hdf_movie_var->num_var;
		for (i = 0; i < num_var; ++i)
		{
	    	    hdf_plot_var2d(front,dirname,hdf_movie_var->var_name[i],
				hdf_movie_var->top_var[i],
				hdf_movie_var->obstacle_comp[i],
				hdf_movie_var->get_state_var[i],first);
		}
	    }
        }
	else if (dim == 3 && pp_numnodes() == 1 &&
		 front->hdf_movie_var != NULL)
	{
	    HDF_MOVIE_VAR *hdf_movie_var = front->hdf_movie_var;
	    if (hdf_movie_var->plot_comp)
	    	hdf_plot_comp3d(front,dirname,first);
	    if (front->hdf_movie_var != NULL)
	    {
		int i,num_var = hdf_movie_var->num_var;
		for (i = 0; i < num_var; ++i)
		{
	    	    hdf_plot_var3d(front,dirname,hdf_movie_var->var_name[i],
				hdf_movie_var->top_var[i],
				hdf_movie_var->obstacle_comp[i],
				hdf_movie_var->get_state_var[i],
				hdf_movie_var->idir[i],first);
		}
	    }
	}
#endif /* defined(USE_HDF) */
	first = NO;
	return;
}	/* end show_front_hdf */

LOCAL	void show_front_xg(
	Front *front,
        char *out_name)
{
	char dirname[256];
        int dim = front->rect_grid->dim;
	static boolean first = YES;
	HDF_MOVIE_VAR *hdf_movie_var = front->hdf_movie_var;
        int i,num_var;

	if (hdf_movie_var != NULL)
	    num_var = hdf_movie_var->num_var;
	else
	    return;

	if (dim > 1) return; /* not for 2,3-D */

	sprintf(dirname,"%s/xg",out_name);
	if (first)
	{
	    if (!create_directory(dirname,NO))
	    {
	    	screen("Cannot create directory %s\n",dirname);
	    	clean_up(ERROR);
	    }
	}
        for (i = 0; i < num_var; ++i)
        {
            xgraph_plot_var(front->grid_intfc,dirname,front->time,
                        hdf_movie_var->var_name[i],hdf_movie_var->top_var[i],
                        hdf_movie_var->get_state_var[i],front->step);
        }
	
	first = NO;
	return;
}	/* end show_front_xg */

LOCAL   void xgraph_plot_var(
        INTERFACE *intfc,
        char *dirname,
        double time,
        char *var_name,
        double *var,
        double (*get_state_var)(Locstate),
	int step)
{
        RECT_GRID *rg = &topological_grid(intfc);
        int *gmax = rg->gmax;
        double *L = rg->L;
        double *U = rg->U;
        double *h = rg->h;
        int i,index0;
        static double *x,*c;
	char fname[100];
	FILE *xfile;
	FT_POINT *p;
        HYPER_SURF *hs;
        HYPER_SURF_ELEMENT *hse;
        Locstate sl,sr;
        int count;
        boolean tracked_interior_point;
	
	if (x == NULL)
        {
            FT_VectorMemoryAlloc((POINTER*)&x,gmax[0]+1,sizeof(double));
            FT_VectorMemoryAlloc((POINTER*)&c,gmax[0]+1,sizeof(double));
        }
	sprintf(fname,"%s/%s-%s.xg",dirname,var_name,
			right_flush(step,7));
	xfile = fopen(fname,"w");

	for (i = 1; i < gmax[0]; ++i)
        {
            index0 = d_index1d(i,gmax);
            x[i] = L[0] + i*h[0];
            c[i] = var[index0];
        }
	tracked_interior_point = NO;
        next_point(intfc,NULL,NULL,NULL);
        while (next_point(intfc,&p,&hse,&hs))
        {
            if (is_bdry(p)) continue;
            tracked_interior_point = YES;
            break;
        }
        slsr(p,hse,hs,&sl,&sr);
	count = 0;
        for (i = 1; i < gmax[0]; ++i)
        {
            x[count] = L[0] + i*h[0];
            index0 = d_index1d(i,gmax);
            c[count] = var[index0];
            if (tracked_interior_point && x[count] > Coords(p)[0])
                break;
            count++;
        }
	if (tracked_interior_point)
        {
            x[count] = Coords(p)[0];
            c[count] = get_state_var(sl);
            count++;
        }
	fprintf(xfile,"\"%s section 1\"\n",var_name);
	for (i = 0; i < count; ++i)
	    fprintf(xfile,"%20.14f  %20.14f\n",x[i],c[i]);

	if (tracked_interior_point)
        {
            count = 0;
            x[count] = Coords(p)[0];
            c[count++] = get_state_var(sr);
            for (i = 1; i < gmax[0]; ++i)
            {
                if (L[0] + i*h[0] <= Coords(p)[0]) continue;
                x[count] = L[0] + i*h[0];
                index0 = d_index1d(i,gmax);
                c[count] = var[index0];
                count++;
            }
	    fprintf(xfile,"\n");
	    fprintf(xfile,"\"%s section 2\"\n",var_name);
	    for (i = 0; i < count; ++i)
	    	fprintf(xfile,"%20.14f  %20.14f\n",x[i],c[i]);
	    fclose(xfile);
        }

}	/* end xgraph_plot_var */

LOCAL	void show_front_gd(
	Front *front,
        char *out_name)
{
	char dirname[256];
        int dim = front->rect_grid->dim;
	static boolean first = YES;

#if defined(__GD__)
	if (dim > 2 || pp_numnodes() > 1) return; /* not for 1,3-D */
						   /* not for parallel */
	sprintf(dirname,"%s/gd",out_name);
	if (first)
	{
	    if (!create_directory(dirname,NO))
	    {
	    	screen("Cannot create directory %s\n",dirname);
	    	clean_up(ERROR);
	    }
	}
	FrontGDMovie(dirname,front);
#endif /* defined(__GD__) */
	first = NO;
	return;
}	/* end show_front_gd */


LOCAL	void show_front_sdl(
	Front *front,
        char *out_name)
{
	char dirname[256];
	int step = front->step;

	if (!front->print_sdl_file)
	    return;
	sprintf(dirname,"%s/sdl.ts%s",out_name,right_flush(step,7));
	if (pp_numnodes() > 1)
            sprintf(dirname,"%s-nd%s",dirname,right_flush(pp_mynode(),4));
	if (!create_directory(dirname,NO))
	{
	    screen("Cannot create directory %s\n",dirname);
	    clean_up(ERROR);
	}
	sdl_interface_plot(dirname,front->interf);
}	/* end show_front_sdl */

EXPORT  void show_front_output(
        Front *front,
        char *out_name,
	boolean print_in_binary) 
{   
	show_front_gd(front,out_name);
	show_front_xg(front,out_name);
	show_front_hdf(front,out_name);
	
	#ifdef USE_CGNS
	show_front_cgns(front,out_name);
	#else
	show_front_vtk(front,out_name,print_in_binary);	
        #endif 
	show_front_sdl(front,out_name);
}       /* end show_front_output */

#if defined(USE_HDF)
EXPORT void plot_hdf_data(
	POINTER		wave,
	Front		*front,
	HDF_plot_data	*hdf_data)
{
	COMPONENT         *comp;
	HDF_PRINT_OPTIONS *opts = &HDF_print_opts(hdf_data);
	RECT_GRID	  *gr = front->rect_grid;
	const char	  *type_name;
	int		  nvars = hdf_num_vars(opts);
	int		  i, dim = hdf_data->dim;
	int		  no_pixels;
	long		  count;
	int		  var;
	int		  pixels[3];
	double		  L[3], U[3];
	double		  l[3], u[3];
	static	long (*fill_hdf_values[4])(POINTER,Front*,HDF_plot_data*,
					   double*,double*,double*,double*,int*) =
					{ NULL,
					  fill_hdf_values1d,
					  fill_hdf_values2d,
					  fill_hdf_values3d};
	debug_print("HDF","Entered plot_hdf_data().\n");

	set_HDF_plotting_range(front,hdf_data,pixels,L,U);

	no_pixels = 1;
	for (i = 0; i < dim; ++i)
	{
	    if (pixels[i] == 0)
	    {
	        if (debugging("HDF"))
		    (void) printf("pixels[%d] = 0\n",i);
		no_pixels = 0;
	    	break;
	    }
	}
	pp_global_imin(&no_pixels,1L);
	if (no_pixels == 0)
	{
	    if (debugging("HDF"))
	    {
		(void) printf("zero size image detected\n");
	    }
	    debug_print("HDF","Left plot_hdf_data().\n");
	    return;
	}

	for (i = 0; i < dim; ++i)
	{
	    l[i] = gr->L[i] /*old had: - 0.5*gr->lbuf[i]*gr->h[i]*/;
	    u[i] = gr->U[i] /*old had: + 0.5*gr->ubuf[i]*gr->h[i]*/;
	}

	count = (*fill_hdf_values[dim])(wave,front,hdf_data,l,u,L,U,pixels);

	comp = hdf_data->comp;
	for (var = 0; var < nvars; ++var)
	{
	    double	min_val, max_val;
	    double	*values = hdf_data->frame_data[var].values;
	    long	k;

	    min_val = HUGE_VAL, max_val = -HUGE_VAL;
	    for (k = 0; k < count; ++k)
	    {
	        if (!is_excluded_comp(comp[k],front->interf) &&
		    values[k] != -HUGE_VAL)
		{
		    min_val = min(min_val,values[k]);
		    max_val = max(max_val,values[k]);
	        }
	    }
	    hdf_data->frame_data[var].current_time_min = min_val;
	    hdf_data->frame_data[var].current_time_max = max_val;
	    if (hdf_data->frame_data[var].cumulative_time_min > min_val)
		hdf_data->frame_data[var].cumulative_time_min = min_val;
	    if (hdf_data->frame_data[var].cumulative_time_max < max_val)
		hdf_data->frame_data[var].cumulative_time_max = max_val;
	}

	if (is_io_node(pp_mynode()))
	{
	    if (hdf_data_type(opts) == HDF_RASTER)
	    {
	        for (var = 0; var < nvars; ++var)
	            print_raster_data(front->interf,hdf_data->frame_data+var,
		                      pixels,hdf_data);
	    }
	    if (hdf_data_type(opts) == HDF_SDS)
	    {
	        for (var = 0; var < nvars; ++var)
		    print_sds_data(front,L,U,
				   hdf_data->frame_data+var,hdf_data);
	    }
	}
	pp_gsync();

	type_name = (hdf_data_type(opts) == HDF_RASTER) ? "HDF" : "SDS";
	(void) printf("\n\n\n");
	(void) output();
	(void) printf("\t\t\t%s GENERATED STATISTICS:\n\n",type_name);
	(void) printf("Min/max values up to time %g\n\n",front->time);
	(void) printf("%-20s %-20s %-20s %-20s %-20s\n",
		      "STATE_VARIABLE","CURRENT_MIN","CURRENT_MAX",
		      "CUMULATIVE_MIN","CUMULATIVE_MAX");
	for (var = 0; var < nvars; ++var)
	{
	    (void) printf("%-20s %-20g %-20g %-20g %-20g\n",
			  HDF_frame_plot_name(hdf_data->frame_data[var]),
			  hdf_data->frame_data[var].current_time_min,
			  hdf_data->frame_data[var].current_time_max,
			  hdf_data->frame_data[var].cumulative_time_min,
			  hdf_data->frame_data[var].cumulative_time_max);
	}
	(void) printf("\n\n");
	(void) output();
	(void) printf("\t\t\tEND %s GENERATED STATISTICS:\n\n",type_name);
	debug_print("HDF","Left plot_hdf_data().\n");
}		/*end plot_hdf_data*/ 

/*ARGSUSED*/
LOCAL	long	fill_hdf_values1d(
	POINTER		wave,
	Front		*front,
	HDF_plot_data	*hdf_data,
	double		*l,
	double		*u,
	double		*L,
	double		*U,
	int		*pixels)
{
	COMPONENT	*comp;
	double		coords[3];
	double		*step = hdf_data->step;
	int		nvars = HDF_num_vars(HDF_print_opts(hdf_data));
	int		width = pixels[0];
	int		nn = pp_numnodes();
	int		var, i;
	long		count, indx;
	static Locstate	state = NULL;

	debug_print("HDF","Entered fill_hdf_values1d().\n");

	if (state == NULL)
	    alloc_state(front->interf,&state,front->sizest);

	if (nn > 1)
	{
	    comp = hdf_data->comp;
	    for (count = 0, i = 0; i < width; ++i, ++count, ++comp)
	    {
		*comp = NO_COMP;
	        for (var = 0; var < nvars; ++var)
	            hdf_data->frame_data[var].values[count] = -HUGE_VAL;
	    }
	}
	else
	    count = width;
	comp = hdf_data->comp;

	/*Fill scale array*/

	for (i = 0; i < width; i++)
	    hdf_data->scale[1][i] = L[0] + (i+.5)*step[0];

	for (indx = 0, i = 0; i < width; ++i, ++indx, ++comp)
	{
	    coords[0] = hdf_data->scale[1][i];
	    if ((nn > 1) && ((coords[0] < l[0]) || (coords[0] > u[0])))
		continue;
	    *comp = component(coords,front->interf);
	    if (is_excluded_comp(comp[indx],front->interf))
		obstacle_state(front->interf,state,front->sizest);
	    else
	    {
	        if (is_exterior_comp(*comp,front->interf))
		    *comp = nearest_interior_comp(YES,NO_COMP,
		                                  coords,front->interf);
	        hyp_solution(coords,*comp,NULL,UNKNOWN_SIDE,
		             front,wave,state,NULL);
	    }

	    for (var = 0; var < nvars; ++var)
	    {
	        hdf_data->frame_data[var].values[indx] =
	            (*HDF_frame_plot_filter(hdf_data->frame_data[var]))(
			(*HDF_frame_plot_function(hdf_data->frame_data[var]))(
	    			                  coords,front,wave,
						  *comp,state));
	    }
	}
	pp_collect_hdf_data(count,hdf_data);
	debug_print("HDF","Left fill_hdf_values1d().\n");
	return count;
}		/*end fill_hdf_values1d*/

LOCAL	long	fill_hdf_values2d(
	POINTER		wave,
	Front		*front,
	HDF_plot_data	*hdf_data,
	double		*l,
	double		*u,
	double		*L,
	double		*U,
	int		*pixels)
{
	COMPONENT	*comp;
	double		coords[3];
	double		*step = hdf_data->step;
	int		nvars = HDF_num_vars(HDF_print_opts(hdf_data));
	int		width = pixels[0];
	int		height = pixels[1];
	int		nn = pp_numnodes();
	int		var, i, j;
	long		count,indx;
	int             p = 0, q = 0;
	double          intfc_jump[1];
	double          sum[2][MAX_NVARS];
	static Locstate	state = NULL;
	double		hl,hu,vl,vu;

	debug_print("HDF","Entered fill_hdf_values2d().\n");
	assert( nvars<=MAX_NVARS);

	for (i = 0; i < nvars; i++)
	{
	    sum[0][i] = 0.0;
	    sum[1][i] = 0.0;
	}
	intfc_jump[0] = 0.0;

	if (state == NULL)
	    alloc_state(front->interf,&state,front->sizest);

	if (nn > 1)
	{
	    comp = hdf_data->comp;
	    for (count = 0, j = 0; j < height; ++j)
	    for (i = 0; i < width; ++i, ++count, ++comp)
	    {
		*comp = NO_COMP;
	        for (var = 0; var < nvars; ++var)
	            hdf_data->frame_data[var].values[count] = -HUGE_VAL;
	    }
	}
	else
	    count = width*height;

	for (i = 0; i < width; i++)
	    hdf_data->scale[1][i] = L[0] + (i+.5)*step[0];
	for (j = 0; j < height; j++)
	    hdf_data->scale[2][j] = L[1] + (j+.5)*step[1];
	if (HDF_subdomain_div(HDF_print_opts(hdf_data)))
	{
	    hl = l[0] + 0.6*step[0]; 	hu = u[0] - 0.6*step[0];
	    vl = l[1] + 0.6*step[1]; 	vu = u[1] - 0.6*step[1];
	}
	else
	{
	    hl = l[0]; 			hu = u[0];
	    vl = l[1]; 			vu = u[1];
	}

	for (j = 0, comp = hdf_data->comp; j < height; ++j)
	{
	    coords[1] = hdf_data->scale[2][j];
	    if ((nn > 1) && ((coords[1] < vl) || (coords[1] > vu)))
		continue;
	    for (i = 0; i < width; ++i)
	    {
	        coords[0] = hdf_data->scale[1][i];
	        if ((nn > 1) && ((coords[0] < hl) || (coords[0] > hu)))
		    continue;
		/*indx = i + j*width;  old, upside down */
		indx = i + (height-j-1)*width; 
	        comp[indx] = component(coords,front->interf);
		if (is_excluded_comp(comp[indx],front->interf))
		    obstacle_state(front->interf,state,front->sizest);
		else
		{
		    if (is_exterior_comp(comp[indx],front->interf))
		        comp[indx] = nearest_interior_comp(YES,NO_COMP,coords,
						           front->interf);
	            hyp_solution(coords,comp[indx],NULL,UNKNOWN_SIDE,
			         front,wave,state,NULL);
		}

	        for (var = 0; var < nvars; ++var)
	        {
	            hdf_data->frame_data[var].values[indx] =
	              (*HDF_frame_plot_filter(hdf_data->frame_data[var]))(
		        (*HDF_frame_plot_function(hdf_data->frame_data[var]))(
	    				          coords,front,wave,
						  comp[indx],state));
		    sum[0][var] = sum[0][var] +  
		      hdf_data->frame_data[var].values[indx] - 
		      (*intfc_jump);
		    sum[1][var] = sum[1][var] + (*intfc_jump);
		    if (var == 0) /* so we don't count more than once */
		    {
			if (*intfc_jump == 0.0)
			  p++;		
			else if (*intfc_jump != 0.0)
			  q++;
		    }
	        }
	    }
	}
	pp_collect_hdf_data(count,hdf_data);
	debug_print("HDF","Left fill_hdf_values2d().\n");
	return count;
}		/*end fill_hdf_values2d*/

LOCAL	long	fill_hdf_values3d(
	POINTER		wave,
	Front		*front,
	HDF_plot_data	*hdf_data,
	double		*l,
	double		*u,
	double		*L,
	double		*U,
	int		*pixels)
{
	COMPONENT	*comp;
	double		coords[3];
	double		*step = hdf_data->step;
	int		nvars = HDF_num_vars(HDF_print_opts(hdf_data));
	double          sum[2][MAX_NVARS], lsum[2][MAX_NVARS];
	int		width = pixels[0];
	int		length = pixels[1];
	int		height = pixels[2];
	int		nn = pp_numnodes();
	int             mn = pp_mynode();
	int		var, i, j, k;
	int             p = 0, q = 0;
	double          intfc_jump[1];
	long		count, indx;
	static Locstate	state = NULL;

	debug_print("HDF","Entered fill_hdf_values3d().\n");
	assert( nvars<=MAX_NVARS);

	for (i = 0; i < nvars; i++)
        {
	    sum[0][i] = 0.0;
	    sum[1][i] = 0.0;
	}
	intfc_jump[0] = 0.0;

	if (state == NULL)
	    alloc_state(front->interf,&state,front->sizest);

	if (nn > 1)
	{
	    comp = hdf_data->comp;
	    for (count = 0, k = 0; k < height; ++k)
	    for (           j = 0; j < length; ++j)
	    for (           i = 0; i < width;  ++i, ++count, ++comp)
	    {
		*comp = NO_COMP;
	        for (var = 0; var < nvars; ++var)
	            hdf_data->frame_data[var].values[count] = -HUGE_VAL;
	    }
	}
	else
	    count = width*length*height;

	for (i = 0; i < width; i++)
	    hdf_data->scale[1][i] = U[0] - (i+.5)*step[0];
	for (j = 0; j < length; j++)
	    hdf_data->scale[2][j] = L[1] + (j+.5)*step[1];
	for (k = 0; k < height; k++)
	    hdf_data->scale[3][k] = L[2] + (k+.5)*step[2];
	

	for (k = 0, comp = hdf_data->comp; k < height; ++k)
	{
	    coords[2] = hdf_data->scale[3][k];
	    if ((nn > 1) && ((coords[2] < l[2]) || (coords[2] > u[2])))
		continue;
	    for (j = 0; j < length; ++j)
	    {
	        coords[1] = hdf_data->scale[2][j];
	        if ((nn > 1) && ((coords[1] < l[1]) || (coords[1] > u[1])))
		    continue;
	        for (i = 0; i < width; ++i)
	        {
	            coords[0] = hdf_data->scale[1][i];
	            if ((nn > 1) && ((coords[0] < l[0]) || (coords[0] > u[0])))
		        continue;
		    indx = i + j*width + k*width*length;
	            comp[indx] = component(coords,front->interf);
		    if (is_excluded_comp(comp[indx],front->interf))
		        obstacle_state(front->interf,state,front->sizest);
		    else
		    {
		        if (is_exterior_comp(comp[indx],front->interf))
		            comp[indx] = nearest_interior_comp(YES,NO_COMP,
			                                       coords,
							       front->interf);
	                hyp_solution(coords,comp[indx],NULL,UNKNOWN_SIDE,
				     front,wave,state,NULL);
		    }

	            for (var = 0; var < nvars; ++var)
	            {
	              hdf_data->frame_data[var].values[indx] =
	                (*HDF_frame_plot_filter(hdf_data->frame_data[var]))(
			  (*HDF_frame_plot_function(hdf_data->frame_data[var]))(
	    				            coords,front,wave,
						    comp[indx],state));
			sum[0][var] = sum[0][var] +  
			  hdf_data->frame_data[var].values[indx] - 
			  (*intfc_jump);
			sum[1][var] = sum[1][var] + (*intfc_jump);
			if (*intfc_jump == 0.0)
			    p++;		
			else if (*intfc_jump != 0.0)
			    q++;
	            }
	        }
	    }
	}
	
	printf("total number of pixels is %d.\n",p+q);
	printf("number of interior pixels is %d = %lf percent of pixels.\n",
	       p,(double)p/(p+q));
	printf("number of interface pixels is %d = %lf percent of pixels .\n",
	       q,(double)q/(p+q));
	for (var = 0; var < nvars; var++)
	{
	    pp_global_sum(sum[0],var);
	    pp_global_sum(sum[1],var);
	    printf("Total var%d = %lf.\n", var, sum[0][var] + sum[1][var]);
	    printf("Interior total for var%d = %lf, = %lf of total.\n",
		   var, sum[0][var], sum[0][var]/(sum[0][var] + sum[1][var]) );
	    printf("Interface total for var%d = %lf, = %lf of total.\n\n",
		   var, sum[1][var], sum[1][var]/(sum[0][var] + sum[1][var]) );
	}
	
	pp_collect_hdf_data(count,hdf_data);
	debug_print("HDF","Left fill_hdf_values3d().\n");
	return count;
}		/*end fill_hdf_values3d*/

LOCAL	void	pp_collect_hdf_data(
	long		count,
	HDF_plot_data	*hdf_data)
{
	int nvars = HDF_num_vars(HDF_print_opts(hdf_data));
	int var;

	if (pp_numnodes() == 1)
	    return;

	if (debugging("HDF"))
	    (void) printf("PP %d has count = %ld\n",pp_mynode(),count);

	pp_global_imax(hdf_data->comp,hdf_data->num_values);
	for (var = 0; var < nvars; ++var)
	    pp_global_max(hdf_data->frame_data[var].values,
			  hdf_data->num_values);

}		/*end pp_collect_hdf_data*/

LOCAL	void	set_HDF_plotting_range(
	Front		*front,
	HDF_plot_data	*hdf_data,
	int		*pixels,
	double		*L,
	double		*U)
{
	HDF_PRINT_OPTIONS	*opts = &HDF_print_opts(hdf_data);
	int	i, dim = hdf_data->dim;

	debug_print("HDF","Entered set_HDF_plotting_range().\n");
	for (i = 0; i < dim; ++i)
	{
	    pixels[i] = hdf_pixels(opts)[i];
	    L[i] = hdf_L0(opts)[i] + front->time*hdf_V(opts)[i];	
	    L[i] = max(L[i],front->rect_grid->GL[i]);
	    U[i] = hdf_U0(opts)[i] + front->time*hdf_V(opts)[i];	
	    U[i] = min(U[i],front->rect_grid->GU[i]);
	    if (hdf_data_type(opts) == HDF_RASTER)
		pixels[i] = ((U[i] - L[i]) > 0.0) ? 
			    irint(pixels[i]*(U[i] - L[i])/hdf_len(opts)[i]) :
			    0;
	}
	debug_print("HDF","Left set_HDF_plotting_range().\n");
}		/*end set_HDF_plotting_range*/

LOCAL	void	print_raster_data(
	INTERFACE       *intfc,
	HDF_frame_data	*frame_data,
	int		*pixels,
	HDF_plot_data	*hdf_data)
{
	COMPONENT         *comp = hdf_data->comp;
	HDF_PRINT_OPTIONS *opts = &HDF_print_opts(hdf_data);
	double	          max_val, min_val;
	double	          *f_val;
	double	          vrng;
	double	          fmax_color;
	uint32		  comp_code = ras_compression_type(opts);
	int	          i, j, k;
	int	          width = pixels[0];
	int	          height = pixels[1];
	uint8	          line_color = frame_data->line_color;
	uint8	          barrier_color = frame_data->line_color-1;
	uint8	          num_table_colors = frame_data->num_table_colors;
	uint8	          max_color = frame_data->num_colors-1;
	uint8	          *r_val = hdf_data->raster_data;;

	fmax_color = max_color;
	f_val = frame_data->values;
	if (hdf_frame_dflt_scale(frame_data) == YES)
	{
	    hdf_frame_scale_min(frame_data) = min_val =
		frame_data->current_time_min;
	    hdf_frame_scale_max(frame_data) = max_val =
		frame_data->current_time_max;
	}
	else
	{
	    min_val = hdf_frame_scale_min(frame_data);
	    max_val = hdf_frame_scale_max(frame_data);
	}
	f_val = frame_data->values;
	vrng = max_val-min_val;
	for (i = 0; i < width; ++i)
	    r_val[i] = r_val[i+height-1] = line_color;
	for (j = 1; j < height-1; ++j)
	    r_val[width*j] = r_val[width*(j+1)-1] = line_color;
	for (j = 1; j < height-1; ++j)
	for (i = 1; i < width-1; ++i)
	{
	    k = j*width+i;
	    if ( (comp[k] != comp[k-1]) || (comp[k] != comp[k-width]) ||
	      (comp[k] != comp[k-width-1]) )
	      r_val[k] = line_color;
	    else if (f_val[k] < min_val)
		r_val[k] = num_table_colors;
	    else if (f_val[k] > max_val)
		r_val[k] = num_table_colors + max_color;
	    else
	        r_val[k] = num_table_colors + 
	    	    (uint8)irint(fmax_color*(f_val[k]-min_val)/vrng);
	}

	(void) DFR8setpalette(frame_data->palette);
	if ((frame_data->first == YES) && (frame_data->append == NO))
	    (void) DFR8putimage(frame_data->file_name,r_val,width,height,
				comp_code);
	else
	    (void) DFR8addimage(frame_data->file_name,r_val,width,height,
				comp_code);
	frame_data->first = NO;
}		/*end print_raster_data*/

/*
*			print_sds_data():
*
*	Prints a FronTier HDF scientific data set for a single scalar variable.
*
*	File Format:
*
*	The computation domain occupies the dim dimensional rectangle with lower and upper
*	corners L and U respectively.  The ploting window occupies the region defined by
*	the dim dimensional rectangle with lower and upper corners wL_i and wU_i respectively for
*	i = 1,..,N where N is the number of frames printed in the file.
*
*	Set Name                      rank  dims                start   edges     data
*       1   "Plot Name"	              1     (strlen(name)+1)    (0)     (dims[0]) Character string
*
*                                                                                 -                 -
*	2   "Computational Domain"    2     (2 dim)             (0 0)   (dims)    | L[0]...L[dim-1] |
*                                                                                 | U[0]...I[dim-1] |
*						                                  -                 -
*
*                                                                                 -                       -
*	3   "Current Plotting Window" 3     (N 2 dim)           (0 0 0) (dims)    | wL_0[0]...wL_0[dim-1] |
*                                                                                 | wU_0[0]...wU_0[dim-1] |
*                                                                                 -                       -
*								         		  ...
*                                                                                 -                               -
*                                                                                 | wL_{N-1}[0]...wL_{N-1}[dim-1] |
*                                                                                 | wU_{N-1}[0]...wU_{N-1}[dim-1] |
*                                                                                 -                               -
*
*	4   "Time"                    1     (N)                 (0)     (dims)    ( T_0,...T_{N-1} )
*
*                                                                                 -                                                 -
*	5  "Max/Min Values"           2     (N 4)               (0 0)   (dims)    | min_0 max_0 cum_min_0 cum_max_0                 |
*                                                                                 |               ...                               |
*	                                                                          | min_{N-1} max_{N-1} cum_min_{N-1} cum_max_{N-1} |
*                                                                                 -                                                 -
*       Note: min_i and max_i refer to the minimum and maximum of the scalar field being printed over the data in the 
*       corresponding frame (ie the max and min over each frames data).  The data in cum_min and cum_max refer the
*	cummulative min and max over the history of the run, that is the cummulative min and max over a series of time steps.
*
*	6   "COMPONENTS"              dim+1 (N l[dim-1]...l[0]) (0 0 0) (dims)    Each frame contains a component number array
*                                                                                 of the current state of the computation, sampled
*                                                                                 onto a rectangular lattice.  This lattice is
*                                                                                 stored as an up to 3 dimensional bi_array.  The
*									          X (or R in cylindrical or spherical geometry) values
*                                                                                 are stored in increasing order as consecutive data
*                                                                                 along a row.  X or R coordinates correspond to the
*                                                                                 last dimension in dims.  Y (in 2D cylindrical Z)
*                                                                                 are stored in decreasing (in direction) order along
*                                                                                 the second to last dimension in dims.  Thus for
*                                                                                 a fixed frame number and Z coordinate slice, X and Y
*                                                                                 are indexed in the geometrically natural order so
*                                                                                 that the image of the matrix with X varing along
*                                                                                 the columns and Y along the rows corresponds to
*                                                                                 the natural coordinate system with the highest Y
*                                                                                 value occuring along the first (ie top) row.  In 3D
*                                                                                 Z coordinates are added as consecutive planes 
*                                                                                 perpendicular to the Z-axis with the Z coordinate
*                                                                                 decreasing with plane index.
*
*	7   the name stored as the    dim+1 (N l[dim-1]...l[0]) (0 0 0) (dims)    This field contains the values of the scalar
*           value of the "Plot Name"                                              field being printed.  Its layout is the same as
*           field                                                                 described for the component field above.
*/

LOCAL	void	print_sds_data(
	Front		*front,
	double		*L,
	double		*U,
	HDF_frame_data	*frame_data,
	HDF_plot_data	*hdf_data)
{
	COMPONENT	  *comp = hdf_data->comp;
	HDF_PRINT_OPTIONS *opts = &HDF_print_opts(hdf_data);
	boolean		  newfile, status;
	comp_info         *c_info = &hdf_compression_info(opts);
	char		  file_name[1024];
	double		  mm[4];
	double		  domain[6];
	int32		  sd_id;
	int32		  rank;
	int32		  dims[4], start[4], edges[4];
	comp_coder_t	  comp_code = sds_compression_type(opts);
	int		  dim = hdf_data->dim;
	int		  *pixels = HDF_pixels(HDF_print_opts(hdf_data));
	int		  i;
	int		  step = front->step;
	int32		  access_mode;
	const char	  **Dnm = front->rect_grid->Remap.Dnm;
	const char	  *dir_names[4];
	static const char *win_dim_names[] = {
						"Window Print Step",
						"Window Lower Bound",
						"Window Upper Bound"
					   };
	static const char *time_dim_name[] = {"Time"};

	static int32	  size_float = -1;

	debug_print("HDF","Entered print_sds_data().\n");

	newfile = YES;
	access_mode = (newfile == YES) ? DFACC_CREATE : DFACC_RDWR;

	if (debugging("HDF"))
	{
	    (void) printf("In print_sds_data(), access_mode = %s\n",
			  (access_mode==DFACC_CREATE)?"DFACC_CREATE":
			  (access_mode==DFACC_RDWR)?"DFACC_RDWR":"UNKNOWN");
	}

	if (size_float == -1)
	    size_float = dfnt_size_float();

	(void) sprintf(file_name,"%s-ts%s.hdf",frame_data->file_name,
		       right_flush(step,TSTEP_FIELD_WIDTH));
	sd_id = SDstart(file_name,access_mode);

	if (newfile == YES)
	{
	    int32	sds_id, dim_id;

	    /*Write data name*/
	    rank = 1;
	    dims[0] = (int32)strlen(hdf_frame_plot_name(frame_data))+1;
	    start[0] = 0;
	    edges[0] = dims[0];
	    sds_id = SDcreate(sd_id,"Plot Name",DFNT_CHAR,rank,dims);
	    if (SDwritedata(sds_id,start,NULL,edges,
			    (VOIDP)hdf_frame_plot_name(frame_data)) != SUCCEED)
	    {
		(void) printf("WARNING in print_sds_data(), SDwritedata failed "                              "writing plot_name for new file\n"
			      "plot name = %s\n",
			      hdf_frame_plot_name(frame_data));
	    }
	    if (SDendaccess(sds_id) != SUCCEED)
	    {
	        (void) printf("WARNING in print_sds_data(), "
			      "SDendaccess failed after writing "
			      "plot_name for new file\n"
			      "plot name = %s\n",
			      hdf_frame_plot_name(frame_data));
	    }

	    /*Write computational domain*/
	    rank = 2;
	    dims[0] = 2;
	    dims[1] = dim;
	    start[0] = 0;
	    start[1] = 0;
	    edges[0] = 2;
	    edges[1] = dim;
	    for (i = 0; i < dim; ++i)
	    {
	    	domain[i] = front->rect_grid->L[i];
	    	domain[i+dim] = front->rect_grid->U[i];
	    }
	    sds_id = SDcreate(sd_id,"Computational Domain",
			      size_float,rank,dims);
	    dim_id = SDgetdimid(sds_id,0);
	    if (SDsetdimname(dim_id,"Domain Lower Bounday") != SUCCEED)
	    {
	        (void) printf("WARNING in print_sds_data(), SDsetdimname "
			      "failed for Domain Lower Bounday\n"
			      "plot name = %s\n",
			      hdf_frame_plot_name(frame_data));
	    }
	    dim_id = SDgetdimid(sds_id,1);
	    if (SDsetdimname(dim_id,"Domain Upper Bounday") != SUCCEED)
	    {
	        (void) printf("WARNING in print_sds_data(), SDsetdimname "
			      "failed for Domain Upper Bounday\n"
			      "plot name = %s\n",
			      hdf_frame_plot_name(frame_data));
	    }
	    if (SDwritedata(sds_id,start,NULL,edges,(VOIDP)domain) != SUCCEED)
	    {
	        (void) printf("WARNING in print_sds_data(), SDwritedata failed "
		              "writing edges for the Computational Domain\n"
			      "plot name = %s\n",
			      hdf_frame_plot_name(frame_data));
	    }
	    if (SDendaccess(sds_id) != SUCCEED)
	    {
	        (void) printf("WARNING in print_sds_data(), SDendaccess failed "
			      "for the Computational Domain\n"
			      "plot name = %s\n",
			      hdf_frame_plot_name(frame_data));
	    }
	}

	/*Write current plotting window*/
	rank = 3;
	dims[0] = SD_UNLIMITED;
	dims[1] = 2;
	dims[2] = dim;
	start[1] = 0;
	start[2] = 0;
	edges[1] = 2;
	edges[2] = dim;
	for (i = 0; i < dim; ++i)
	{
	    domain[i] = L[i];
	    domain[i+dim] = U[i];
	}
	if (write_sds_slab(newfile,sd_id,rank,start,edges,dims,NULL,
			   win_dim_names,"Current Plotting Window",
			   size_float,comp_code,
			   c_info,(VOIDP)domain) != FUNCTION_SUCCEEDED)
	{
	    (void) printf("WARNING in print_sds_data(), write_sds_slab failed "
			  "for the Current Plotting Window\n"
			  "plot name = %s\n",hdf_frame_plot_name(frame_data));
	}

	/*Record current time*/
	rank = 1;
	dims[0] = SD_UNLIMITED;
	if (write_sds_slab(newfile,sd_id,rank,start,edges,dims,NULL,
			   time_dim_name,"Time",size_float,comp_code,
			   c_info,(VOIDP)&front->time) != FUNCTION_SUCCEEDED)
	{
	    (void) printf("WARNING in print_sds_data(), write_sds_slab failed "
			  "for Time\n"
			  "plot name = %s\n",hdf_frame_plot_name(frame_data));
	}

	/* Write min/max values */
	rank = 2;
	dims[0] = SD_UNLIMITED;
	dims[1] = 4;
	start[1] = 0;
	edges[1] = 4;
	mm[0] = frame_data->current_time_min;
	mm[1] = frame_data->current_time_max;
	mm[2] = frame_data->cumulative_time_min;
	mm[3] = frame_data->cumulative_time_max;

	if (write_sds_slab(newfile,sd_id,rank,start,edges,dims,NULL,NULL,
		           "Max/Min Values",size_float,comp_code,
			   c_info,(VOIDP)mm) != FUNCTION_SUCCEEDED)
	{
	    (void) printf("WARNING in print_sds_data(), write_sds_slab failed "
			  "for Max/Min Values\n"
			  "plot name = %s\n",hdf_frame_plot_name(frame_data));
	}

	/* Set rank and dims for values and comps arrays */
	rank = dim + 1;
	dims[0] = SD_UNLIMITED;
	for (i = 1; i <= dim; ++i)
	{
	    dims[i] = pixels[dim-i];
	    start[i] = 0;
	    edges[i] = dims[i];
	}
	dir_names[0] = "Time Slice";
	for (i = 1; i <= dim; ++i)
	    dir_names[i] = Dnm[dim-i];

	if (debugging("HDF"))
	{
	    (void) printf("Dimensions of sds data\n");
	    print_int32_vector("dims = ",dims+1,dim,"\n");
	    print_int32_vector("start = ",start+1,dim,"\n");
	    print_int32_vector("edges = ",edges+1,dim,"\n");
	}

	if (sizeof(COMPONENT) != sizeof(int32))
	{
	    static int32 *i32comp;
	    if (!i32comp)
	        uni_array(&i32comp,hdf_data->num_values,sizeof(int32));
	    for (i = 0; i < hdf_data->num_values; ++i)
	        i32comp[i] = comp[i];
	    status = write_sds_slab(newfile,sd_id,rank,start,edges,dims,
	                            hdf_data->scale,dir_names,"COMPONENTS",
				    DFNT_INT32,comp_code,c_info,(VOIDP)i32comp);
	}
	else
	{
	    status = write_sds_slab(newfile,sd_id,rank,start,edges,dims,
	                            hdf_data->scale,dir_names,"COMPONENTS",
				    DFNT_INT32,comp_code,c_info,(VOIDP)comp);
	}
	if (!status)
	{
	    (void) printf("WARNING in print_sds_data(), write_sds_slab failed "
			  "for COMPONENTS\n"
			  "plot name = %s\n",hdf_frame_plot_name(frame_data));
	}

	/*Write values array*/
	if (write_sds_slab(newfile,sd_id,rank,start,edges,dims,hdf_data->scale,
		           dir_names,hdf_frame_plot_name(frame_data),size_float,
		           comp_code,c_info,
			   (VOIDP)frame_data->values) != FUNCTION_SUCCEEDED)
	{
	    (void) printf("WARNING in print_sds_data(), write_sds_slab failed "
			  "for the values array\n"
			  "plot name = %s\n",hdf_frame_plot_name(frame_data));
	}

	if (SDend(sd_id) != SUCCEED)
	    (void) printf("WARNING in print_sds_data(), SDend failed\n");

	frame_data->first = NO;
	debug_print("HDF","Left print_sds_data().\n");
}		/*end print_sds_data*/


LOCAL	boolean	write_sds_slab(
	boolean	     newfile,
	int32	     sd_id,
	int32	     rank,
	int32	     *start,
	int32	     *edges,
	int32	     *dims,
	double	     **scale,
	const char   **dim_names,
	const char   *name,
	int32	     size_data,
	comp_coder_t comp_code,
	comp_info    *c_info,
	VOIDP	     values)
{
	boolean status = FUNCTION_SUCCEEDED;
	int32	sds_id, sds_idx, dim_id;
	int32	num_type, num_attrs;
	intn	i;

	debug_print("HDF","Entered write_sds_slab().\n");

	if (newfile == YES)
	{
	    start[0] = 0;
	    edges[0] = 1;
	    sds_id = SDcreate(sd_id,name,size_data,rank,dims);
	    if (dim_names != NULL)
	    {
	        for (i = 0; i < rank; ++i)
	        {
	    	    dim_id = SDgetdimid(sds_id,i);
	    	    if (SDsetdimname(dim_id,dim_names[i]) != SUCCEED)
	    	    {
	                (void) printf("WARNING in write_sds_slab(), "
			              "SDsetdimname failed, name = %s\n",name);
			status = FUNCTION_FAILED;
	    	    }
		    if (debugging("HDF"))
		    {
			(void) printf("Dimension name %d for %s set to %s\n",
				      i,name,dim_names[i]);
		    }
	        }
	    }
	}
	else
	{
	    sds_idx = SDnametoindex(sd_id,name);
	    sds_id = SDselect(sd_id,sds_idx);
	    dim_id = SDgetdimid(sds_id,0);
	    if (SDdiminfo(dim_id,NULL,start,&num_type,&num_attrs) != SUCCEED)
	    {
		(void) printf("WARNING in write_sds_slab(), "
			      "SDdiminfo failed, name = %s\n",name);
		status = FUNCTION_FAILED;
	    }
	    ++start[0];
	    edges[0] = 1;
	}
	if (scale != NULL)
	{
	    static int32 size_float = -1;

	    if (size_float == -1)
		size_float = dfnt_size_float();

	    if (debugging("HDF"))
		(void) printf("Setting dimscale for %s\n",name);
	    for (i = 0; i < rank; ++i)
	    {
		if (scale[i] != NULL)
		{
	    	    dim_id = SDgetdimid(sds_id,i);
	    	    if (SDsetdimscale(dim_id,dims[i],size_float,scale[i])
								   != SUCCEED)
	    	    {
	                (void) printf("WARNING in write_sds_slab(), "
			              "SDsetdimscale failed, name = %s\n",name);
			(void) printf("dim_id = %d, dims[%d] = %d, "
				      "size_data = %d, scale[%d] = %p\n",
				      dim_id,i,dims[i],size_data,
				      i,(POINTER)scale[i]);
			status = FUNCTION_FAILED;
	    	    }
		}
	    }
	}
	switch (comp_code)
	{
	case COMP_CODE_RLE:
	    if (debugging("HDF"))
		(void) printf("Setting RLE compression\n");
	    if (SDsetcompress(sds_id,comp_code,c_info) != SUCCEED)
	    {
		(void) printf("WARNING in write_sds_slab(), "
		              "RLE compression failed\n");
	    }
	    break;
	case COMP_CODE_DEFLATE:
	    if (debugging("HDF"))
		(void) printf("Setting GZIP compression\n");
	    if (SDsetcompress(sds_id,comp_code,c_info) != SUCCEED)
	    {
		(void) printf("WARNING in write_sds_slab(), "
		              "DEFLATE compression failed\n");
	    }
	    break;
	case COMP_CODE_SKPHUFF:
	    if (debugging("HDF"))
		(void) printf("Setting SKPHUFF compression\n");
	    c_info->skphuff.skp_size = size_data;
	    if (SDsetcompress(sds_id,comp_code,c_info) != SUCCEED)
	    {
		(void) printf("WARNING in write_sds_slab(), "
		              "SKPHUFF compression failed\n");
	    }
	    break;
	case COMP_CODE_NONE:
	default:
	    break;
	}
	if (debugging("HDF"))
	    (void) printf("Calling SDwritedata()\n");
	if (SDwritedata(sds_id,start,NULL,edges,values) != SUCCEED)
	{
	    (void) printf("WARNING in write_sds_slab(), SDwritedata failed "
		   "writing edges, name = %s\n",name);
	    status = FUNCTION_FAILED;
	}
	if (SDendaccess(sds_id) != SUCCEED)
	{
	    (void) printf("WARNING in write_sds_slab(), "
			  "SDendaccess failed, name = %s\n",name);
	    status = FUNCTION_FAILED;
	}
	debug_print("HDF","Left write_sds_slab().\n");
	return status;
}		/*end write_sds_slab*/

LOCAL	int32	dfnt_size_float(void)
{
	static int32 size_float = -1;

	if (size_float == -1)
	{
	    if (sizeof(double)==sizeof(float64))
	        size_float = DFNT_FLOAT64;
	    else if (sizeof(double)==sizeof(float32))
	        size_float = DFNT_FLOAT32;
	    else
	    {
	        screen("ERROR in print_sds_data(), "
	               "can't determine float size\n");
	        clean_up(ERROR);
	    }
	}
	return size_float;
}		/*end dfnt_size_float*/

LOCAL	void print_int32_vector(
	const char *mesg,
	int32	   *v,
	int32	   dim,
	const char *end)
{
	int32		i;

	if (mesg != NULL) (void) printf("%s",mesg);
	(void) printf("(");
	for (i = 0; i < dim; ++i)
	    (void) printf("%d%s",v[i],(i==(dim-1)) ? ")" : ", ");
	(void) printf("%s",end);
}		/*end print_int32_vector*/

static uint8	dflt_palette[3*256] = {
	    0xff,0xff,0xff,0x00,0x00,0x00,0xff,0xff,0x00,0xff,0x00,0xff,
	    0x00,0xff,0xff,0xff,0x00,0x00,0x00,0xff,0x00,0x00,0x00,0xff,
	    0x00,0x00,0xfd,0x01,0x02,0xfb,0x03,0x04,0xf9,0x04,0x06,0xf7,
	    0x06,0x08,0xf5,0x07,0x0a,0xf3,0x09,0x0c,0xf1,0x0a,0x0e,0xef,
	    0x0c,0x10,0xed,0x0e,0x12,0xeb,0x0f,0x14,0xe9,0x11,0x16,0xe7,
	    0x12,0x18,0xe4,0x14,0x1b,0xe2,0x15,0x1d,0xe0,0x17,0x1f,0xde,
	    0x18,0x21,0xdc,0x1a,0x23,0xda,0x1c,0x25,0xd8,0x1d,0x27,0xd6,
	    0x1f,0x29,0xd4,0x20,0x2b,0xd2,0x22,0x2d,0xd0,0x23,0x2f,0xce,
	    0x25,0x31,0xcb,0x27,0x34,0xc9,0x28,0x36,0xc7,0x2a,0x38,0xc5,
	    0x2b,0x3a,0xc3,0x2d,0x3c,0xc1,0x2e,0x3e,0xbf,0x30,0x40,0xbd,
	    0x31,0x42,0xbb,0x33,0x44,0xb9,0x35,0x46,0xb7,0x36,0x48,0xb5,
	    0x38,0x4a,0xb2,0x39,0x4d,0xb0,0x3b,0x4f,0xae,0x3c,0x51,0xac,
	    0x3e,0x53,0xaa,0x3f,0x55,0xa8,0x41,0x57,0xa6,0x43,0x59,0xa4,
	    0x44,0x5b,0xa2,0x46,0x5d,0xa0,0x47,0x5f,0x9e,0x49,0x61,0x9c,
	    0x4a,0x63,0x9a,0x4c,0x65,0x97,0x4e,0x68,0x95,0x4f,0x6a,0x93,
	    0x51,0x6c,0x91,0x52,0x6e,0x8f,0x54,0x70,0x8d,0x55,0x72,0x8b,
	    0x57,0x74,0x89,0x58,0x76,0x87,0x5a,0x78,0x85,0x5c,0x7a,0x83,
	    0x5d,0x7c,0x81,0x5f,0x7e,0x7e,0x60,0x81,0x7c,0x62,0x83,0x7a,
	    0x63,0x85,0x78,0x65,0x87,0x76,0x67,0x89,0x74,0x68,0x8b,0x72,
	    0x6a,0x8d,0x70,0x6b,0x8f,0x6e,0x6d,0x91,0x6c,0x6e,0x93,0x6a,
	    0x70,0x95,0x68,0x71,0x97,0x65,0x73,0x9a,0x63,0x75,0x9c,0x61,
	    0x76,0x9e,0x5f,0x78,0xa0,0x5d,0x79,0xa2,0x5b,0x7b,0xa4,0x59,
	    0x7c,0xa6,0x57,0x7e,0xa8,0x55,0x7f,0xaa,0x53,0x81,0xac,0x51,
	    0x83,0xae,0x4f,0x84,0xb0,0x4d,0x86,0xb2,0x4a,0x87,0xb5,0x48,
	    0x89,0xb7,0x46,0x8a,0xb9,0x44,0x8c,0xbb,0x42,0x8e,0xbd,0x40,
	    0x8f,0xbf,0x3e,0x91,0xc1,0x3c,0x92,0xc3,0x3a,0x94,0xc5,0x38,
	    0x95,0xc7,0x36,0x97,0xc9,0x34,0x98,0xcb,0x31,0x9a,0xce,0x2f,
	    0x9c,0xd0,0x2d,0x9d,0xd2,0x2b,0x9f,0xd4,0x29,0xa0,0xd6,0x27,
	    0xa2,0xd8,0x25,0xa3,0xda,0x23,0xa5,0xdc,0x21,0xa7,0xde,0x1f,
	    0xa8,0xe0,0x1d,0xaa,0xe2,0x1b,0xab,0xe4,0x18,0xad,0xe7,0x16,
	    0xae,0xe9,0x14,0xb0,0xeb,0x12,0xb1,0xed,0x10,0xb3,0xef,0x0e,
	    0xb5,0xf1,0x0c,0xb6,0xf3,0x0a,0xb8,0xf5,0x08,0xb9,0xf7,0x06,
	    0xbb,0xf9,0x04,0xbc,0xfb,0x02,0xbe,0xfd,0x00,0xbe,0xfb,0x00,
	    0xbf,0xf9,0x00,0xbf,0xf7,0x00,0xc0,0xf5,0x00,0xc1,0xf3,0x00,
	    0xc1,0xf1,0x00,0xc2,0xef,0x00,0xc2,0xed,0x00,0xc3,0xeb,0x00,
	    0xc3,0xe9,0x00,0xc4,0xe7,0x00,0xc4,0xe5,0x00,0xc5,0xe3,0x00,
	    0xc5,0xe1,0x00,0xc6,0xde,0x00,0xc6,0xdc,0x00,0xc7,0xda,0x00,
	    0xc7,0xd8,0x00,0xc8,0xd6,0x00,0xc8,0xd4,0x00,0xc9,0xd2,0x00,
	    0xc9,0xd0,0x00,0xca,0xce,0x00,0xca,0xcc,0x00,0xcb,0xca,0x00,
	    0xcb,0xc8,0x00,0xcc,0xc6,0x00,0xcc,0xc4,0x00,0xcd,0xc2,0x00,
	    0xcd,0xbf,0x00,0xce,0xbd,0x00,0xce,0xbb,0x00,0xcf,0xb9,0x00,
	    0xcf,0xb7,0x00,0xd0,0xb5,0x00,0xd1,0xb3,0x00,0xd1,0xb1,0x00,
	    0xd2,0xaf,0x00,0xd2,0xad,0x00,0xd3,0xab,0x00,0xd3,0xa9,0x00,
	    0xd4,0xa7,0x00,0xd4,0xa5,0x00,0xd5,0xa3,0x00,0xd5,0xa1,0x00,
	    0xd6,0x9e,0x00,0xd6,0x9c,0x00,0xd7,0x9a,0x00,0xd7,0x98,0x00,
	    0xd8,0x96,0x00,0xd8,0x94,0x00,0xd9,0x92,0x00,0xd9,0x90,0x00,
	    0xda,0x8e,0x00,0xda,0x8c,0x00,0xdb,0x8a,0x00,0xdb,0x88,0x00,
	    0xdc,0x86,0x00,0xdc,0x84,0x00,0xdd,0x82,0x00,0xdd,0x7f,0x00,
	    0xde,0x7d,0x00,0xde,0x7b,0x00,0xdf,0x79,0x00,0xdf,0x77,0x00,
	    0xe0,0x75,0x00,0xe1,0x73,0x00,0xe1,0x71,0x00,0xe2,0x6f,0x00,
	    0xe2,0x6d,0x00,0xe3,0x6b,0x00,0xe3,0x69,0x00,0xe4,0x67,0x00,
	    0xe4,0x65,0x00,0xe5,0x63,0x00,0xe5,0x61,0x00,0xe6,0x5e,0x00,
	    0xe6,0x5c,0x00,0xe7,0x5a,0x00,0xe7,0x58,0x00,0xe8,0x56,0x00,
	    0xe8,0x54,0x00,0xe9,0x52,0x00,0xe9,0x50,0x00,0xea,0x4e,0x00,
	    0xea,0x4c,0x00,0xeb,0x4a,0x00,0xeb,0x48,0x00,0xec,0x46,0x00,
	    0xec,0x44,0x00,0xed,0x42,0x00,0xed,0x3f,0x00,0xee,0x3d,0x00,
	    0xee,0x3b,0x00,0xef,0x39,0x00,0xef,0x37,0x00,0xf0,0x35,0x00,
	    0xf1,0x33,0x00,0xf1,0x31,0x00,0xf2,0x2f,0x00,0xf2,0x2d,0x00,
	    0xf3,0x2b,0x00,0xf3,0x29,0x00,0xf4,0x27,0x00,0xf4,0x25,0x00,
	    0xf5,0x23,0x00,0xf5,0x21,0x00,0xf6,0x1e,0x00,0xf6,0x1c,0x00,
	    0xf7,0x1a,0x00,0xf7,0x18,0x00,0xf8,0x16,0x00,0xf8,0x14,0x00,
	    0xf9,0x12,0x00,0xf9,0x10,0x00,0xfa,0x0e,0x00,0xfa,0x0c,0x00,
	    0xfb,0x0a,0x00,0xfb,0x08,0x00,0xfc,0x06,0x00,0xfc,0x04,0x00,
	    0xfd,0x02,0x00,0xfd,0x00,0x00,0xff,0xff,0xff,0x00,0x00,0x00
	};

LOCAL	void	hdf_plot_comp2d(
	Front	*front, 
	char	*dirname,
	boolean	first)
{
	static 	COMPONENT *comps;
	int		width;
	int		height;
	int		pwidth;
	int		pheight;
	int		i, j, k;
	double           coords[MAXD];
	double           *U;
	double           *L;
	double           *PU;
	double           *PL;
	double           hx,hy;
 	RECT_GRID       *gr = front->rect_grid;
	PP_GRID         *pp = front->pp_grid;
	RECT_GRID       *global_gr = &(pp->Global_grid);

	char		file_name[1024];
	static uint8	*r_val, *pr_val, *tmp_val, *tmp2_val;
	static uint8    line_color = 255;
	static uint8    num_table_colors = 8;
	uint8    num_colors = 256 - num_table_colors - 2;
	int 		icoords[3], index, index2;
	static uint8 *palette = dflt_palette;
	static uint8 cfunc[17] = {0,16,32,48,8,24,40,56,4,12,20,28,
					36,44,52,60,64};
	int pixel[8] = {100000,200000,300000,400000,500000,600000,
                                        700000,800000};
	int resolution_level = front->resolution_level;
	int Total_Pixels = pixel[resolution_level];
	int hdf_comp_tag = 666;

 	width = gr->gmax[0];
 	height = gr->gmax[1];
	L = gr->L;
	U = gr->U;
	PL = global_gr->L;
	PU = global_gr->U;
	pwidth = global_gr->gmax[0];
	pheight = global_gr->gmax[1];

	debug_print("HDF","Entered plot_hdf_comp2d().\n");

	pwidth = irint(sqrt(Total_Pixels*(PU[0] - PL[0])/(PU[1] - PL[1])));
	pheight = irint(sqrt(Total_Pixels*(PU[1] - PL[1])/(PU[0] - PL[0])));
	if (pwidth > 800)
	{
	    pwidth = 800;
	    pheight = irint(800*(PU[1] - PL[1])/(PU[0] - PL[0]));
	}
	else if (pheight > 800)
	{
	    pheight = 800;
	    pwidth = irint(800*(PU[0] - PL[0])/(PU[1] - PL[1]));
	}
	width = (int)((1.0*pwidth)/(1.0*pp->gmax[0])); 
	height = (int)((1.0*pheight)/(1.0*pp->gmax[1])); 
	hx = (U[0] - L[0])/(width - 1);
	hy = (U[1] - L[1])/(height - 1);
	uni_array(&r_val,width*height,sizeof(uint8));
	if (comps == NULL)
	    uni_array(&comps,width*height,sizeof(COMPONENT));
	front->hdf_comps[0] = comps;
	
	if(pp_mynode() == 0)
	{
	    uni_array(&tmp_val,width*height,sizeof(uint8));
	    uni_array(&pr_val,pwidth*pheight,sizeof(uint8));
	}
	for (j = 0; j < width; ++j)
	{
	    for (i = 0; i < height; ++i)
	    {
		coords[0] = L[0]+j*hx;
                coords[1] = L[1]+i*hy;
		k = j + (height - i - 1)*width;
	        comps[k] = component(coords,front->interf);
	    }
	}

	for (i = 0; i < width; ++i)
            r_val[i] = r_val[i+(height-2)*width] = line_color;
	for (j = 1; j < height-1; ++j)
            r_val[width*j] = r_val[width*(j+1)-1] = line_color;
	
        for (j = 0; j < width; ++j)
        for (i = 0; i < height; ++i)
	{
	       k = j + (height - i - 1)*width;

	    if (k > width && k%width != 0 && 
	    	((comps[k] != comps[k-1]) || (comps[k] != comps[k-width]) 
		|| (comps[k] != comps[k-width-1])))
	  	r_val[k] = line_color; 
	    else
	    {
		int ic = (int)comps[k];
		r_val[k] = (uint8)(num_table_colors + 
			irint(cfunc[ic]/64.0*num_colors));
            }
	}
	sprintf(file_name,"%s/comp.hdf",dirname);
	
#if defined(__MPI__)
	if(pp_mynode() != 0)
	{
	    pp_send(hdf_comp_tag, r_val,width*height*sizeof(uint8),0);
	}
#endif /* defined(__MPI__) */
	
	if(pp_mynode() == 0)
	{
	    for (i = 0; i < pp_numnodes(); i++)
	    {
#if defined(__MPI__)
	        find_Cartesian_coordinates( i, pp, icoords);
	        if (i != 0)
	            pp_recv(hdf_comp_tag,i,tmp_val, width*height*sizeof(uint8)); 
	        tmp2_val = tmp_val;
#endif /* defined(__MPI__) */
	        if(i == 0)
	            tmp2_val = r_val;
	        for (j = 0; j < width; ++j)
	        for (k = 0; k < height; ++k)
                {
		      index = j + (height - k - 1)*width;    
#if defined(__MPI__)
                     index = pwidth*(pheight - icoords[1]*height - k - 1) + icoords[0]*width + j;   
#endif /* defined(__MPI__) */
                     index2= j +(height-k-1)*width;
                   
	            pr_val[index] = tmp2_val[index2];
		}
	    }   
       	    (void) DFR8setpalette(palette);
	    if(first == YES)
            	(void) DFR8putimage(file_name,pr_val,pwidth,pheight,COMP_NONE);
	    else
            	(void) DFR8addimage(file_name,pr_val,pwidth,pheight,COMP_NONE);
	    free_these(2,tmp_val,pr_val);
	}

	free_these(1,r_val);
	debug_print("HDF","Left plot_hdf_comp2d().\n");
}	/* end plot_hdf_comp2d */

LOCAL	void	hdf_plot_comp3d(
	Front	*front, 
	char	*dirname,
	boolean	first)
{
	int i;

	for (i = 0; i < 3; ++i)
	    hdf_plot_cross_sectional_comp(front,dirname,i,first);
}	/* end hdf_plot_comp3 */

LOCAL	void	hdf_plot_cross_sectional_comp(
	Front	*front, 
	char	*dirname,
	int	idir,
	boolean	first)
{
	static 	COMPONENT *hdf_comps[MAXD];
	COMPONENT	*comps;
	int		width,height,pwidth,pheight;
	int		i, j, k;
	double           coords[MAXD];
	double           L[2],U[2],PL[2],PU[2];
	double           hx,hy;
 	RECT_GRID       *gr = front->rect_grid;
	PP_GRID         *pp_grid = front->pp_grid;
	RECT_GRID       *global_gr = &(pp_grid->Global_grid);

	char		file_name[1024];
	static uint8    line_color = 255;
	static uint8    num_table_colors = 8;
	uint8		*r_val, *pr_val, *tmp_val, *tmp2_val;
	uint8    	num_colors = 256 - num_table_colors - 2;
	int 		my_ic[3],icoords[3],io_node,my_id,index,index2;
	static uint8 *palette = dflt_palette;
	static uint8 cfunc[17] = {0,16,32,48,8,24,40,56,4,12,20,28,
					36,44,52,60,64};
	int pixel[8] = {100000,200000,300000,400000,500000,600000,
                                        700000,800000};
	int resolution_level = front->resolution_level;
	int Total_Pixels = pixel[resolution_level];
	int i1,i2;
	double crds_idir;
	char crx_suffix[3][3] = {"yz","xz","xy"};
	int hdf_comp_tag = 666;

	debug_print("HDF","Entered hdf_plot_cross_sectional_comp().\n");

	my_id = pp_mynode();
#if defined(__MPI__)
	find_Cartesian_coordinates(my_id,pp_grid,my_ic);
	if (my_ic[idir] != pp_grid->gmax[idir]/2) return;
#endif /* defined(__MPI__) */
	switch (idir)
	{
	case 0:
	    i1 = 1;	i2 = 2;
	    break;
	case 1:
	    i1 = 0;	i2 = 2;
	    break;
	case 2:
	    i1 = 0;	i2 = 1;
	}
	for (i = 0; i < 3; ++i)
	    icoords[i] = 0;
	icoords[idir] = pp_grid->gmax[idir]/2;
	io_node = domain_id(icoords,pp_grid->gmax,3);

 	width  = gr->gmax[i1];
 	height = gr->gmax[i2];
	pwidth  = global_gr->gmax[i1];
	pheight = global_gr->gmax[i2];
	L[0] = gr->L[i1];	L[1] = gr->L[i2];
	U[0] = gr->U[i1];	U[1] = gr->U[i2];
	PL[0] = global_gr->L[i1];	PL[1] = global_gr->L[i2];
	PU[0] = global_gr->U[i1];	PU[1] = global_gr->U[i2];
	crds_idir = 0.5*(global_gr->L[idir] + global_gr->U[idir]);

	pwidth = irint(sqrt(Total_Pixels*(PU[0] - PL[0])/(PU[1] - PL[1])));
	pheight = irint(sqrt(Total_Pixels*(PU[1] - PL[1])/(PU[0] - PL[0])));
	if (pwidth > 800)
	{
	    pwidth = 800;
	    pheight = irint(800*(PU[1] - PL[1])/(PU[0] - PL[0]));
	}
	else if (pheight > 800)
	{
	    pheight = 800;
	    pwidth = irint(800*(PU[0] - PL[0])/(PU[1] - PL[1]));
	}
	width = (int)(pwidth/pp_grid->gmax[i1]); 
	height = (int)(pheight/pp_grid->gmax[i2]); 
	hx = (U[0] - L[0])/(width - 1);
	hy = (U[1] - L[1])/(height - 1);
	uni_array(&r_val,width*height,sizeof(uint8));
	if (hdf_comps[idir] == NULL)
	    uni_array(&hdf_comps[idir],width*height,sizeof(COMPONENT));
	comps = front->hdf_comps[idir] = hdf_comps[idir];
	
	if(my_id == io_node)
	{
	    uni_array(&tmp_val,width*height,sizeof(uint8));
	    uni_array(&pr_val,pwidth*pheight,sizeof(uint8));
	}
	coords[idir] = crds_idir;
	for (j = 0; j < width; ++j)
	for (i = 0; i < height; ++i)
	{
	    coords[i1] = L[0]+j*hx;
            coords[i2] = L[1]+i*hy;
	    k = j + (height - i - 1)*width;
	    comps[k] = component(coords,front->interf);
	}

	for (i = 0; i < width; ++i)
            r_val[i] = r_val[i+(height-2)*width] = line_color;
	for (j = 1; j < height-1; ++j)
            r_val[width*j] = r_val[width*(j+1)-1] = line_color;
	
        for (j = 0; j < width; ++j)
        for (i = 0; i < height; ++i)
	{
	     k = j + (height - i - 1)*width;

	    if (k > width && k%width != 0 && 
	    	((comps[k] != comps[k-1]) || (comps[k] != comps[k-width]) 
		|| (comps[k] != comps[k-width-1])))
	  	r_val[k] = line_color; 
	    else
	    {
		int ic = (int)comps[k];
		r_val[k] = (uint8)(num_table_colors + 
			irint(cfunc[ic]/64.0*num_colors));
            }
	}
	sprintf(file_name,"%s/comp-%s.hdf",dirname,crx_suffix[idir]);
	
#if defined(__MPI__)
	if (my_id != io_node)
	{
	    pp_send(hdf_comp_tag,r_val,width*height*sizeof(uint8),io_node);
	}
#endif /* defined(__MPI__) */
	
	if (my_id == io_node)
	{
	    for (i = 0; i < pp_numnodes(); i++)
	    {
	        tmp2_val = r_val;
#if defined(__MPI__)
	        find_Cartesian_coordinates(i,pp_grid,icoords);
		if (icoords[idir] != pp_grid->gmax[idir]/2) continue;
	        if (i != io_node)
		{
	            pp_recv(hdf_comp_tag,i,tmp_val,width*height*sizeof(uint8)); 
	            tmp2_val = tmp_val;
		}
#endif /* defined(__MPI__) */
	        for (j = 0; j < width; ++j)
	        for (k = 0; k < height; ++k)
                {
		     index = j + (height - k - 1)*width;    
#if defined(__MPI__)
                     index = pwidth*(pheight - icoords[i2]*height - k - 1) 
				+ icoords[i1]*width + j;   
#endif /* defined(__MPI__) */
                     index2= j + (height - k - 1)*width;
                   
	             pr_val[index] = tmp2_val[index2];
		}
	    }   
       	    (void) DFR8setpalette(palette);
	    if(first == YES)
            	(void) DFR8putimage(file_name,pr_val,pwidth,pheight,COMP_NONE);
	    else
            	(void) DFR8addimage(file_name,pr_val,pwidth,pheight,COMP_NONE);
	    free_these(2,tmp_val,pr_val);
	}

	free_these(1,r_val);
	debug_print("HDF","Left hdf_plot_cross_sectional_comp().\n");
}	/* end hdf_plot_cross_sectional_comp */

LOCAL	void	hdf_plot_var2d( 
	Front	*front, 
	char	*dirname,
	char	*var_name,
	double	*var,
	COMPONENT obs_comp,
	double (*get_state_var)(Locstate),
	boolean first)
{
	COMPONENT	*comps = front->hdf_comps[0];
	int		width;
	int		height;
	int		pwidth;
	int		pheight;
	int		i, j, k;
	double           coords[MAXD];
	double           *U;
	double           *L;
	double           *PU;
	double           *PL;
	double           hx,hy;
 	RECT_GRID       *gr = front->rect_grid;
	PP_GRID         *pp = front->pp_grid;
	RECT_GRID       *global_gr = &(pp->Global_grid);
	double		*var_val;
	int pixel[8] = {100000,200000,300000,400000,500000,600000,
                                        700000,800000};
	int resolution_level = front->resolution_level;
	int Total_Pixels = pixel[resolution_level];

	char		file_name[1024];
	double	min_val,max_val,vrng;
	static uint8	*r_val, *pr_val, *tmp_val, *tmp2_val;
	static uint8    line_color = 255;
	static uint8    num_table_colors = 8;
	static uint8    obs_color = 8;
	uint8    num_colors = 256 - num_table_colors - 2;
	uint8    min_colors = num_table_colors;
	uint8    max_colors = num_table_colors + num_colors;
	int 		icoords[3], index, index2;
	static uint8 *palette = dflt_palette;
	int hdf_comp_tag;
	boolean use_mid_color = NO;

 	width = gr->gmax[0];
 	height = gr->gmax[1];
	L = gr->L;
	U = gr->U;
	PL = global_gr->L;
	PU = global_gr->U;
	pwidth = global_gr->gmax[0];
	pheight = global_gr->gmax[1];
	debug_print("HDF","Entered hdf_plot_var2d().\n");

	pwidth = irint(sqrt(Total_Pixels*(PU[0] - PL[0])/(PU[1] - PL[1])));
	pheight = irint(sqrt(Total_Pixels*(PU[1] - PL[1])/(PU[0] - PL[0])));
	if (pwidth > 800)
	{
	    pwidth = 800;
	    pheight = irint(800*(PU[1] - PL[1])/(PU[0] - PL[0]));
	}
	else if (pheight > 800)
	{
	    pheight = 800;
	    pwidth = irint(800*(PU[0] - PL[0])/(PU[1] - PL[1]));
	}
	width = (int)((1.0*pwidth)/(1.0*pp->gmax[0])); 
	height = (int)((1.0*pheight)/(1.0*pp->gmax[1])); 
	hx = (U[0] - L[0])/(width - 1);
	hy = (U[1] - L[1])/(height - 1);
	uni_array(&r_val,width*height,sizeof(uint8));
	uni_array(&var_val,width*height,FLOAT);
	if(pp_mynode() == 0)
	{
	    uni_array(&tmp_val,width*height,sizeof(uint8));
	    uni_array(&pr_val,pwidth*pheight,sizeof(uint8));
	}

	max_val = -HUGE; 	min_val = HUGE;
	
	for (j = 0; j < width; ++j)
	{
	    for (i = 0; i < height; ++i)
	    {
		coords[0] = L[0] + j*hx;
                coords[1] = L[1] + i*hy;
		k = j + (height - i - 1)*width;
		if (comps[k] == obs_comp) continue;
		FT_IntrpStateVarAtCoords(front,comps[k],coords,var,
				get_state_var,var_val+k,NULL);
		if (var_val[k] < min_val) min_val = var_val[k];
		if (var_val[k] > max_val) max_val = var_val[k];
	    }
	}
	pp_global_max(&max_val,1);
	pp_global_min(&min_val,1);

	vrng = max_val - min_val;
	if (vrng < 0.0001*min_val || min_val == 0.0)
	    use_mid_color = YES;

	for (i = 0; i < width; ++i)
            r_val[i] = r_val[i+(height-2)*width] = line_color;
	for (j = 1; j < height-1; ++j)
            r_val[width*j] = r_val[width*(j+1)-1] = line_color;
	
        for (j = 0; j < width; ++j)
        for (i = 0; i < height; ++i)
	{
	       k = j + (height - i - 1)*width;

	    if (comps[k] == obs_comp)
		r_val[k] = obs_color;
	    else if (k > width && k%width != 0 && 
	    	((comps[k] != comps[k-1]) || (comps[k] != comps[k-width]) 
		|| (comps[k] != comps[k-width-1])))
	  	r_val[k] = line_color; 
	    else
	    {
		if (use_mid_color)
		    r_val[k] = (uint8)(num_table_colors + 0.5*num_colors);
		else
		    r_val[k] = (uint8)(num_table_colors + 
			(var_val[k] - min_val)/vrng*(num_colors-1));
            }
	}
	sprintf(file_name,"%s/%s.hdf",dirname,var_name);
	hdf_comp_tag = 666;
	
#if defined(__MPI__)
	if(pp_mynode() != 0)
	{
	    pp_send(hdf_comp_tag, r_val,width*height*sizeof(uint8),0);
	}
#endif /* defined(__MPI__) */
	
	if(pp_mynode() == 0)
	{
	    for (i = 0; i < pp_numnodes(); i++)
	    {
#if defined(__MPI__)
	        find_Cartesian_coordinates( i, pp, icoords);
	        if (i != 0)
	            pp_recv(hdf_comp_tag,i,tmp_val, width*height*sizeof(uint8)); 
	        tmp2_val = tmp_val;
#endif /* defined(__MPI__) */
	        if(i == 0)
	            tmp2_val = r_val;
	        for (j = 0; j < width; ++j)
	        for (k = 0; k < height; ++k)
                {
		      index = j + (height - k - 1)*width;    
#if defined(__MPI__)
                     index = pwidth*(pheight - icoords[1]*height - k - 1) + icoords[0]*width + j;   
#endif /* defined(__MPI__) */
                     index2= j +(height-k-1)*width;
                   
	            pr_val[index] = tmp2_val[index2];
		}
	    }   
	}

       	(void) DFR8setpalette(palette);
	if(first == YES)
            (void) DFR8putimage(file_name,pr_val,pwidth,pheight,COMP_NONE);
	else
            (void) DFR8addimage(file_name,pr_val,pwidth,pheight,COMP_NONE);
     
	
	free_these(2,var_val,r_val);
	if(pp_mynode() == 0)
	{
	    free_these(2,tmp_val,pr_val);
	}
	debug_print("HDF","Left hdf_plot_var2d().\n");
}	/* end hdf_plot_var2d */

LOCAL	void	hdf_plot_var3d( 
	Front	*front, 
	char	*dirname,
	char	*var_name,
	double	*var,
	COMPONENT obs_comp,
	double 	(*get_state_var)(Locstate),
	int 	idir,
	boolean 	first)
{
	COMPONENT	*comps = front->hdf_comps[idir];
	int		width;
	int		height;
	int		pwidth;
	int		pheight;
	int		i, j, k;
	double           coords[MAXD];
	double           L[2],U[2],PL[2],PU[2];
	double           hx,hy;
 	RECT_GRID       *gr = front->rect_grid;
	PP_GRID         *pp_grid = front->pp_grid;
	RECT_GRID       *global_gr = &(pp_grid->Global_grid);
	double		*var_val;
	int pixel[8] = {100000,200000,300000,400000,500000,600000,
                                        700000,800000};
	int resolution_level = front->resolution_level;
	int Total_Pixels = pixel[resolution_level];

	char		file_name[1024];
	double	min_val,max_val,vrng;
	static uint8	*r_val, *pr_val, *tmp_val, *tmp2_val;
	static uint8    line_color = 255;
	static uint8    num_table_colors = 8;
	uint8   num_colors = 256 - num_table_colors - 2;
	uint8   min_colors = num_table_colors;
	uint8   max_colors = num_table_colors + num_colors;
	int 	i1,i2,my_ic[3],icoords[3],io_node,my_id,index, index2;
	double	crds_idir;
	int 	hdf_comp_tag = 666;
	static uint8 *palette = dflt_palette;
	double ext_val[2];

	debug_print("HDF","Entered hdf_plot_var3d().\n");
        my_id = pp_mynode();
#if defined(__MPI__)
        find_Cartesian_coordinates(my_id,pp_grid,my_ic);
        if (my_ic[idir] != pp_grid->gmax[idir]/2) return;
#endif /* defined(__MPI__) */
        switch (idir)
        {
        case 0:
            i1 = 1;     i2 = 2;
            break;
        case 1:
            i1 = 0;     i2 = 2;
            break;
        case 2:
            i1 = 0;     i2 = 1;
        }
        for (i = 0; i < 3; ++i)
            icoords[i] = 0;
        icoords[idir] = pp_grid->gmax[idir]/2;
        io_node = domain_id(icoords,pp_grid->gmax,3);

        L[0] = gr->L[i1];       L[1] = gr->L[i2];
        U[0] = gr->U[i1];       U[1] = gr->U[i2];
        PL[0] = global_gr->L[i1];       PL[1] = global_gr->L[i2];
        PU[0] = global_gr->U[i1];       PU[1] = global_gr->U[i2];
        crds_idir = 0.5*(global_gr->L[idir] + global_gr->U[idir]);

	pwidth = irint(sqrt(Total_Pixels*(PU[0] - PL[0])/(PU[1] - PL[1])));
	pheight = irint(sqrt(Total_Pixels*(PU[1] - PL[1])/(PU[0] - PL[0])));
	if (pwidth > 800)
	{
	    pwidth = 800;
	    pheight = irint(800*(PU[1] - PL[1])/(PU[0] - PL[0]));
	}
	else if (pheight > 800)
	{
	    pheight = 800;
	    pwidth = irint(800*(PU[0] - PL[0])/(PU[1] - PL[1]));
	}
	width = (int)(pwidth/pp_grid->gmax[i1]); 
	height = (int)(pheight/pp_grid->gmax[i2]); 
	hx = (U[0] - L[0])/(width - 1);
	hy = (U[1] - L[1])/(height - 1);
	uni_array(&r_val,width*height,sizeof(uint8));
	uni_array(&var_val,width*height,FLOAT);
	if(my_id == io_node)
	{
	    uni_array(&tmp_val,width*height,sizeof(uint8));
	    uni_array(&pr_val,pwidth*pheight,sizeof(uint8));
	}

	max_val = -HUGE; 	min_val = HUGE;
	
	coords[idir] = crds_idir;
	for (j = 0; j < width; ++j)
	for (i = 0; i < height; ++i)
	{
	    coords[i1] = L[0] + j*hx;
            coords[i2] = L[1] + i*hy;
	    k = j + (height - i - 1)*width;
	    if (comps[k] == obs_comp) continue;
	    FT_IntrpStateVarAtCoords(front,comps[k],coords,var,
				get_state_var,var_val+k,NULL);
	    if (var_val[k] < min_val) min_val = var_val[k];
	    if (var_val[k] > max_val) max_val = var_val[k];
	}
#if defined(__MPI__)
	ext_val[0] = min_val;
	ext_val[1] = max_val;
	for (i = 0; i < pp_numnodes(); i++)
	{
	    find_Cartesian_coordinates(i,pp_grid,icoords);
	    if (icoords[idir] != pp_grid->gmax[idir]/2) continue;
	    pp_send(hdf_comp_tag,&ext_val,2*FLOAT,i);
	}
	for (i = 0; i < pp_numnodes(); i++)
	{
	    find_Cartesian_coordinates(i,pp_grid,icoords);
	    if (icoords[idir] != pp_grid->gmax[idir]/2) continue;
	    pp_recv(hdf_comp_tag,i,ext_val,2*FLOAT); 
	    if (min_val > ext_val[0]) min_val = ext_val[0];
	    if (max_val < ext_val[1]) max_val = ext_val[1];
	}
#endif /* defined(__MPI__) */

	vrng = max_val - min_val;

	for (i = 0; i < width; ++i)
            r_val[i] = r_val[i+(height-2)*width] = line_color;
	for (j = 1; j < height-1; ++j)
            r_val[width*j] = r_val[width*(j+1)-1] = line_color;
	
        for (j = 0; j < width; ++j)
        for (i = 0; i < height; ++i)
	{
	    k = j + (height - i - 1)*width;

	    if (comps[k] == obs_comp)
		r_val[k] = line_color;
	    else if (k > width && k%width != 0 && 
	    	((comps[k] != comps[k-1]) || (comps[k] != comps[k-width]) 
		|| (comps[k] != comps[k-width-1])))
	  	r_val[k] = line_color; 
	    else
	    {
		r_val[k] = (uint8)(num_table_colors + 
			(var_val[k] - min_val)/vrng*(num_colors-1));
            }
	}
	sprintf(file_name,"%s/%s.hdf",dirname,var_name);
	
#if defined(__MPI__)
	if (my_id != io_node)
	{
	    pp_send(hdf_comp_tag, r_val,width*height*sizeof(uint8),io_node);
	}
#endif /* defined(__MPI__) */
	
	if (my_id == io_node)
	{
	    for (i = 0; i < pp_numnodes(); i++)
	    {
#if defined(__MPI__)
	        find_Cartesian_coordinates(i,pp_grid,icoords);
		if (icoords[idir] != pp_grid->gmax[idir]/2) continue;
	        if (i != io_node)
		{
	            pp_recv(hdf_comp_tag,i,tmp_val, width*height*sizeof(uint8)); 
		}
	        tmp2_val = tmp_val;
#endif /* defined(__MPI__) */
	        if(i == io_node)
	            tmp2_val = r_val;
	        for (j = 0; j < width; ++j)
	        for (k = 0; k < height; ++k)
                {
		    index = j + (height - k - 1)*width;    
#if defined(__MPI__)
                    index = pwidth*(pheight - icoords[i2]*height - k - 1) + 
				icoords[i1]*width + j;   
#endif /* defined(__MPI__) */
                     index2= j +(height-k-1)*width;
                   
	            pr_val[index] = tmp2_val[index2];
		}
	    }   
       	    (void) DFR8setpalette(palette);
	    if(first == YES)
        	(void) DFR8putimage(file_name,pr_val,pwidth,pheight,COMP_NONE);
	    else
        	(void) DFR8addimage(file_name,pr_val,pwidth,pheight,COMP_NONE);
	    free_these(2,tmp_val,pr_val);
	}
	
	free_these(2,var_val,r_val);
	debug_print("HDF","Left hdf_plot_var3d()\n");
}	/* end hdf_plot_var3d */
#endif /* defined(USE_HDF) */

LOCAL 	void fprint_front_time_stamp(
	FILE *file,
	Front *front)
{
        if (file == NULL)
            return;
        (void) fprintf(file,"\n\n\n\n");
        (void) foutput(file);
        (void) fprintf(file,"\tTime Data:  t = %-"FFMT" j = %-10d dt = %-"FFMT"\n",
                       front->time,front->step,front->dt);	
}	/* end fprint_front_time_stamp */

#if defined(__GD__)
LOCAL	void FrontGDMovie(
	char *dirname,
	Front *front)
{
	switch(front->rect_grid->dim)
	{
	case 1:
	    FrontGDMovie1d(dirname,front);
	    return;
	case 2:
	    FrontGDMovie2d(dirname,front);
	    return;
	default:
	    screen("ERROR: FrontGDMovie() does not support dim = %d\n",
				front->rect_grid->dim);
	    clean_up(ERROR);
	}
}	/* end FrontGDMovie */

LOCAL	void FrontGDMovie1d(
	char *dirname,
	Front *front)
{
	HDF_MOVIE_VAR *hdf_movie_var = front->hdf_movie_var;
	int i,num_var = hdf_movie_var->num_var;
	static boolean first = YES;

	for (i = 0; i < num_var; ++i)
	{
	    gd_plot_var(front->grid_intfc,dirname,front->time,
			hdf_movie_var->var_name[i],hdf_movie_var->top_var[i],
			hdf_movie_var->get_state_var[i],i,first);
	}
	first = NO;
}	/* end FrontGDMovie1d */

#define		MAX_COUNT		10

LOCAL	void gd_plot_var(
	INTERFACE *intfc,
	char *dirname,
	double time,
	char *var_name,
	double *var,
	double (*get_state_var)(Locstate),
	int ivar,
        boolean first)
{
	RECT_GRID *rg = &topological_grid(intfc);
	int *gmax = rg->gmax;
	double *L = rg->L;
	double *U = rg->U;
	double *h = rg->h;
	int i,index0;
        static double *x,*c;
        char movie_caption[100];
        char time_label[100];
        char gd_name[200];
        static double xmin,xmax,cmin[MAX_COUNT],cmax[MAX_COUNT];
	double height;
	FT_POINT *p;
        HYPER_SURF *hs;
        HYPER_SURF_ELEMENT *hse;
        Locstate sl,sr;
	int count;
	boolean tracked_interior_point;

	if (debugging("trace"))
	    printf("Entering FrontGDMovie1d()\n");

	if (x == NULL)
	{
	    FT_VectorMemoryAlloc((POINTER*)&x,gmax[0]+1,sizeof(double));
	    FT_VectorMemoryAlloc((POINTER*)&c,gmax[0]+1,sizeof(double));
	}

	for (i = 1; i < gmax[0]; ++i)
	{
	    index0 = d_index1d(i,gmax);
	    x[i] = L[0] + i*h[0];
	    c[i] = var[index0];
	}

	if (first)
	{
	    for (i = 1; i < gmax[0]; ++i)
	    {
	    	if (cmin[ivar] > c[i]) cmin[ivar] = c[i];
            	if (cmax[ivar] < c[i]) cmax[ivar] = c[i];
	    }
	    height = cmax[ivar] - cmin[ivar];
	    xmin = L[0];	xmax = U[0];
	    cmin[ivar] -= 0.15*height;    cmax[ivar] += 0.30*height;
	}
	sprintf(movie_caption,"%s vs. x",var_name);
        sprintf(gd_name,"%s/%s.gif",dirname,var_name);
	if (first)
            gd_initplot(gd_name,movie_caption,xmin,xmax,
				cmin[ivar],cmax[ivar],3);
	else
            gd_appendplot(gd_name,movie_caption,xmin,xmax,
				cmin[ivar],cmax[ivar],3);

	tracked_interior_point = NO;
	next_point(intfc,NULL,NULL,NULL);
	while (next_point(intfc,&p,&hse,&hs))
	{
	    if (is_bdry(p)) continue;
	    tracked_interior_point = YES;
	    break;
	}
	slsr(p,hse,hs,&sl,&sr);

	count = 0;
	for (i = 1; i < gmax[0]; ++i)
	{
	    x[count] = L[0] + i*h[0];
	    index0 = d_index1d(i,gmax);
	    c[count] = var[index0];
	    if (tracked_interior_point && x[count] > Coords(p)[0]) 
		break;
	    count++;
	}
	if (tracked_interior_point)
	{
	    x[count] = Coords(p)[0];
	    c[count] = get_state_var(sl);
	    count++;
	}
	gd_plotdata(count,x,c);

	if (tracked_interior_point)
	{
	    count = 0;
	    x[count] = Coords(p)[0];
	    c[count++] = get_state_var(sr);
	    for (i = 1; i < gmax[0]; ++i)
	    {
	    	if (L[0] + i*h[0] <= Coords(p)[0]) continue;
	    	x[count] = L[0] + i*h[0];
		index0 = d_index1d(i,gmax);
	    	c[count] = var[index0];
	    	count++;
	    }
	    gd_plotdata(count,x,c);
	}

	sprintf(time_label,"Time = %6.3f",time);
	gd_plotframe(time_label);
	if (debugging("trace"))
	    printf("Leaving FrontGDMovie1d()\n");
}	/* end FrontGDMovie1d */

LOCAL	void FrontGDMovie2d(
	char *dirname,
	Front *front)
{
	INTERFACE *copy_intfc;
	int *lbuf = front->rect_grid->lbuf;
	int *ubuf = front->rect_grid->ubuf;
	char label[100];
	boolean plot_bullet = NO;

	if (front->hdf_movie_var != NULL)
	    plot_bullet = front->hdf_movie_var->plot_bullet;

	set_size_of_intfc_state(size_of_state(front->interf));
	set_copy_intfc_states(NO);
	copy_intfc = copy_interface(front->interf);

	delete_subdomain_curves(copy_intfc);
        delete_passive_boundaries(copy_intfc);
	clip_to_interior_region(copy_intfc,lbuf,ubuf);
	sprintf(label,"Time = %10.4f",front->time);
	gd_2d_intfc(dirname,label,copy_intfc,front->rect_grid,
			front->resolution_level,plot_bullet);
	delete_interface(copy_intfc);
}	/* end FrontGDMovie2d */
#endif /* defined(__GD__) */

LOCAL void vtk_plot_vector_field(
	const char *dname,
	Front *front)
{
	INTERFACE *grid_intfc = front->grid_intfc;
	RECT_GRID *gr = &topological_grid(grid_intfc);
	int gmax[MAXD],icoords[MAXD];
	double h[MAXD],L[MAXD],vec[MAXD];
	VTK_MOVIE_VAR *vtk_movie_var = front->vtk_movie_var;
	double **top_var = vtk_movie_var->top_var[0];
	char *vname = vtk_movie_var->var_name[0];
	static char *fname = NULL;
	FILE *vfile;
	size_t fname_len = 0;
	int i,j,k,l,index,dim = grid_intfc->dim;
	double time = front->time;

	vname = vtk_movie_var->var_name[0];
	fname = get_vtk_file_name(fname,dname,vname,&fname_len);
	if (create_directory(dname,YES) == FUNCTION_FAILED)
        {
            (void) printf("WARNING in vtk_interface_plot(), directory "
                          "%s doesn't exist and can't be created\n",dname);
            return;
        }
	vfile = fopen(fname,"w");
	for (i = 0; i < 3; ++i)
	{
	    gmax[i] = 1; L[i] = 0.0; h[i] = 0.0;
	}
	for (i = 0; i < dim; ++i)
	{
	    gmax[i] = gr->gmax[i]; L[i] = gr->L[i]; h[i] = gr->h[i];
	}

	fprintf(vfile,"# vtk DataFile Version 2.0\n");
	fprintf(vfile,"%s\n",vname);
	fprintf(vfile,"ASCII\n");
	fprintf(vfile,"DATASET STRUCTURED_POINTS\n");
	fprintf(vfile,"DIMENSIONS %d %d %d\n",gmax[0]+1,gmax[1]+1,gmax[2]+1);
	fprintf(vfile,"SPACING %f %f %f\n",h[0], h[1],h[2]);
	fprintf(vfile,"ORIGIN %f %f %f\n",L[0], L[1],L[2]);
	fprintf(vfile,"POINT_DATA %d\n",(gmax[0]+1)*(gmax[1]+1)*(gmax[2]+1));
	fprintf(vfile,"VECTORS VELOCITY double\n");

	for (l = 0; l < MAXD; ++l) vec[l] = 0.0;
	for (k = 0; k <= gmax[2]; k++)
	for (j = 0; j <= gmax[1]; j++)
	for (i = 0; i <= gmax[0]; i++)
	{		
	    icoords[0] = i;
	    icoords[1] = j;
	    icoords[2] = k;
	    index  = d_index(icoords,gmax,dim);
	    for (l = 0; l < dim; ++l)
		vec[l] = top_var[l][index];
	    fprintf(vfile,"%f %f %f\n",vec[0],vec[1],vec[2]);
	}
	fclose(vfile);
}	/* end vtk_plot_vector_field */

#ifdef USE_CGNS
LOCAL	void show_front_cgns(
	Front *front,
        char *out_name)
{
	/*****************New with CGNS ****************/
	char dirname_cgns[256];
	int step = front->step;
	int dim = front->rect_grid->dim;
	boolean print_in_binary = YES;
	
	if (dim == 1) return;

	/* Create cgns directories */

	sprintf(dirname_cgns,"%s/cgns.ts%s",out_name,right_flush(step,7));
	if (pp_numnodes() > 1)
	    sprintf(dirname_cgns,"%s-nd%s",dirname_cgns,
		right_flush(pp_mynode(),4));
	if (!create_directory(dirname_cgns,YES))
	{
	    screen("Cannot create directory %s\n",dirname_cgns);
	    clean_up(ERROR);
	}
    	cgns_interface_plot(dirname_cgns,front->interf,print_in_binary, 
	        front->time,front->step);
	if (front->vtk_movie_var != NULL)
	    cgns_plot_vector_field(dirname_cgns,front);
  
  	/***********************************************/
	/*return;*/
}	/* end show_front_cgns */

LOCAL void cgns_plot_vector_field(
	const char *dname,
	Front *front)
{
	INTERFACE *grid_intfc = front->grid_intfc;
	RECT_GRID *gr = &topological_grid(grid_intfc);
	int gmax[MAXD],icoords[MAXD];
	double h[MAXD],L[MAXD],vec[MAXD];
	VTK_MOVIE_VAR *vtk_movie_var = front->vtk_movie_var;
	double **top_var = vtk_movie_var->top_var[0];
	char *vname;
	static char *fname = NULL;
	FILE *vfile;
	size_t fname_len = 0;
	int i,j,k,l,index,dim = grid_intfc->dim;
	double time = front->time;
	int step = front->step;
 	/********* New with CGNS *********************/
   	static double ***xs_velocity = NULL;
	static double ***ys_velocity = NULL;
	static double ***zs_velocity = NULL;
	static double ***xs = NULL;
	static double ***ys = NULL;
	static double ***zs = NULL;
	static char *filename = NULL;
   	int isize2d[3][2], isize3d[3][3];
   	int ni,nj,nk;
   	int index_file,icelldim,iphysdim;
   	int index_base,index_zone,index_coord;
	int index_flow,index_velocity;
   	char basename[33],zonename[33],solname[33];
	/*********************************************/

	vname = vtk_movie_var->var_name[0];
	filename = get_cgns_file_name(fname,dname,vname,&fname_len);
	cg_open(filename,CG_MODE_WRITE,&index_file); /* New with CGNS*/
	for (i = 0; i < 3; ++i)
	{
	    gmax[i] = 1; L[i] = 0.0; h[i] = 0.0;
	}
	for (i = 0; i < dim; ++i)
	{
	    gmax[i] = gr->gmax[i]; L[i] = gr->L[i]; h[i] = gr->h[i];
	}

	/* create base (user can give any name) */
   	strcpy(basename,"Base");
   	icelldim=3;
   	iphysdim=3;
   	cg_base_write(index_file,basename,icelldim,iphysdim,&index_base);
	/* define zone name (user can give any name) */
   	strcpy(zonename,"Zone 1");
	/* vertex size */
   	isize3d[0][0]=gmax[0]+2;
   	isize3d[0][1]=gmax[1]+2;
	isize3d[0][2]=gmax[2]+2;
	/* cell size */
   	isize3d[1][0]=isize3d[0][0]-1;
   	isize3d[1][1]=isize3d[0][1]-1;
	isize3d[1][2]=isize3d[0][2]-1;
	/* boundary vertex size (always zero for structured grids) */
   	isize3d[2][0]=0;
   	isize3d[2][1]=0;
	isize3d[2][2]=0;
	/* create zone */
   	cg_zone_write(index_file,index_base,zonename,*isize3d,Structured,&index_zone);

	/* create gridpoints */
	tri_array(&xs,gmax[2]+2,gmax[1]+2,gmax[0]+2,FLOAT);
	tri_array(&ys,gmax[2]+2,gmax[1]+2,gmax[0]+2,FLOAT);
        tri_array(&zs,gmax[2]+2,gmax[1]+2,gmax[0]+2,FLOAT);

   	for (k=0; k <= gmax[0]+1; k++)
      	for (j=0; j <= gmax[1]+1; j++)
        for (i=0; i <= gmax[2]+1; i++)
	{
	  xs[i][j][k] = L[0] + (k-.5)*h[0];
	  ys[i][j][k] = L[1] + (j-.5)*h[1];
	  zs[i][j][k] = L[2] + (i-.5)*h[2];
   	}

	/* write grid coordinates (user must use SIDS-standard names here) */
   	cg_coord_write(index_file,index_base,index_zone,RealDouble,"CoordinateX",
      	    **xs,&index_coord);
   	cg_coord_write(index_file,index_base,index_zone,RealDouble,"CoordinateY",
      	    **ys,&index_coord);
   	cg_coord_write(index_file,index_base,index_zone,RealDouble,"CoordinateZ",
      	    **zs,&index_coord);
	
	/* create flow solution */
	strcpy(solname,"FlowSolution");
	cg_sol_write(index_file,index_base,index_zone,solname,CellCenter,&index_flow);

	/* create velocities */
	tri_array(&xs_velocity,gmax[2]+1,gmax[1]+1,gmax[0]+1,FLOAT);
	tri_array(&ys_velocity,gmax[2]+1,gmax[1]+1,gmax[0]+1,FLOAT);
        tri_array(&zs_velocity,gmax[2]+1,gmax[1]+1,gmax[0]+1,FLOAT);
	
	for (l = 0; l < dim; ++l) vec[l] = 0.0;

	for (k = 0; k <= gmax[0]; k++)
	for (j = 0; j <= gmax[1]; j++)
	for (i = 0; i <= gmax[2]; i++)
	{		
	    icoords[2] = i;
	    icoords[1] = j;
	    icoords[0] = k;
	    index  = d_index(icoords,gmax,dim);
	    for (l = 0; l < dim; ++l)
		vec[l] = top_var[l][index];
	    /*fprintf(vfile,"%lf %lf %lf\n",vec[0],vec[1],vec[2]);*/
	    xs_velocity[i][j][k] = vec[0];
	    ys_velocity[i][j][k] = vec[1];
	    zs_velocity[i][j][k] = vec[2];
	}
	/* write velocities (user must use SIDS-standard names here) */
	cg_field_write(index_file,index_base,index_zone,index_flow,RealDouble,
	    "VelocityX",**xs_velocity,&index_velocity);
	cg_field_write(index_file,index_base,index_zone,index_flow,RealDouble,
	    "VelocityY",**ys_velocity,&index_velocity);
	cg_field_write(index_file,index_base,index_zone,index_flow,RealDouble,
	    "VelocityZ",**zs_velocity,&index_velocity);
	cg_close(index_file);
}	/* end cgns_plot_vector_field */
#endif /*end USE_CGNS */
