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
*				fmap.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*/

#include <front/fdecs.h>		/* includes int.h, table.h */

LOCAL 	boolean build_linear_element(INTRP_CELL*,double*);
LOCAL 	void collect_cell_ptst(INTRP_CELL*,int*,COMPONENT,Front*,double*,
			double (*func)(Locstate));
LOCAL	boolean test_point_in_seg(double*,double**);
LOCAL	boolean test_point_in_tri(double*,double**);
LOCAL 	boolean test_point_in_tetra(double*,double**);
LOCAL	Tan_stencil **FrontGetTanStencils2d(Front*,FT_POINT*,int);
LOCAL	Tan_stencil **FrontGetTanStencils3d(Front*,FT_POINT*,int);
LOCAL 	void FrontPreAdvance2d(Front*);
LOCAL 	void FrontPreAdvance3d(Front*);
LOCAL   boolean new_vtx_is_closer(double,double,double*,double*,int);

EXPORT	void FT_Propagate(
	Front *front)
{
	double dt_frac;
	Front *newfront;

	FrontAdvance(front->dt,&dt_frac,front,&newfront,
                                (POINTER)NULL);
	if (front->grid_intfc != NULL)
	    FT_FreeGridIntfc(front);
        assign_interface_and_free_front(front,newfront);
	FT_MakeGridIntfc(front);
}	/* end FT_Propagate */

EXPORT	void FT_Propagate2(
	Front *front,
	Front **newfront)
{
	double dt_frac;
	FT_MakeGridIntfc(front);
	FrontAdvance(front->dt,&dt_frac,front,newfront,
                                (POINTER)NULL);
	FT_FreeGridIntfc(front);
}	/* end FT_Propagate */

EXPORT	void FrontSwapAndFree(
	Front *front,
	Front *newfront)
{
        assign_interface_and_free_front(front,newfront);
}	/* end FT_Propagate */

EXPORT	int FrontAdvance(
        double    dt,
        double    *dt_frac,
        Front    *front,
        Front    **newfront,
        POINTER  wave)
{
	int status,count;
	double start_dt = dt;

	*dt_frac = 1.0;
	front->dt_frac = dt_frac;
        status = advance_front(front->dt,dt_frac,front,newfront,wave);

        count = 0;
        while (status == MODIFY_TIME_STEP || status == REPEAT_TIME_STEP)
        {
	    if (status == MODIFY_TIME_STEP)
            	front->dt = (*dt_frac)*start_dt;
            status = advance_front(front->dt,dt_frac,front,newfront,wave);
            count++;
            if (count > 15) 
	    {
		screen("ERROR: in FrontAdvance() modified step 10 times\n");
	    	clean_up(ERROR);
	    }
        }
}	/* end FrontAdvance */

EXPORT	double FrontHypTimeStep(
	Front *front)
{
	double fcrds[MAXD];
	double max_dt;

	/* f_max_front_time_step */
	max_dt = (*front->max_front_time_step)(front,fcrds);
#if defined(__MPI__)
	pp_global_min(&max_dt,1);
#endif /* defined(__MPI__) */
	return max_dt;
}	/* end FrontTimeStep */

EXPORT	double FrontOutputTimeControl(
	Front *front,
	boolean *is_movie_time,
	boolean *is_print_time,
	boolean *time_limit_reached,
	int *im,		/* Printing number */
	int *ip)		/* Movie Frame number */
{
	double time = front->time;
	double dt = front->dt;
	double new_dt;
	double dt1,dt2,dt3;

	*is_movie_time = *is_print_time = *time_limit_reached = NO;

	dt1 = (*im)*front->movie_frame_interval - time;
	dt2 = (*ip)*front->print_time_interval - time;
	dt3 = front->max_time - time;
	new_dt = min3(dt1,dt2,dt3);

	if (front->step+1 >= front->max_step)
            *time_limit_reached = YES;

	if (new_dt > dt) 
	    return dt;

	if (dt1 == new_dt)
	{
	    *is_movie_time = YES;
	    (*im)++;
	}
        if (dt2 == new_dt)
        {
            *is_print_time = YES;
	    (*ip)++;
        }
        if (dt3 == new_dt)
        {
            *time_limit_reached = YES;
        }
	return new_dt;
}	/* end FrontOutputTimeControl */

EXPORT  void FT_RedistMesh(
        Front *fr)
{
        INTERFACE *intfc = fr->interf;
        CURVE **c;
        SURFACE **s;
        int dim = intfc->dim;
        boolean force_redistribute = YES;
	boolean sav_intrp_state = interpolate_intfc_states(intfc);

	interpolate_intfc_states(intfc) = YES;
        switch (dim)
        {
        case 2:
            Curve_redistribute(fr,&force_redistribute);
            break;
        case 3:
	    reset_normal_on_intfc(fr->interf);
            Surface_redistribute(fr,&force_redistribute);
        }
	interpolate_intfc_states(intfc) = sav_intrp_state;
}       /* end FT_RedistMesh */

EXPORT void FT_RedistSurfWithWaveType(
	Front *front,
	SURFACE *surf,
	TRI_REDIST_PARAMS redist_params,
	int w_type)
{
}	/* end FT_RedistSurfWithWaveType */

static boolean	equi_bond_curve_redist(Front*,CURVE*);
EXPORT void FT_RedistCurveWithEqualBonds(
	Front *front,
	CURVE *c)
{
	equi_bond_curve_redist(front,c);
}	/* end FT_RedistCurveWithWaveType */

EXPORT  void FT_SetCurveSpacing(
        Front *fr,
        double spacing)
{
        Front_spacing(fr,GENERAL_WAVE) = spacing;
}       /* end FrontSetSpacing */

EXPORT	void FrontSetTriParams(
	Front *fr,
	double max_tri_area_fac,
	double min_tri_area_fac,
	double min_angle_at_vertex,	/* In unit of degree */
	double max_scaled_tri_side)
{
	double sslen,std_area,hmin,a;

	hmin = fr->rect_grid->h[0];
	if (hmin > fr->rect_grid->h[1])
	    hmin = fr->rect_grid->h[1];
	if (hmin > fr->rect_grid->h[2])
	    hmin = fr->rect_grid->h[2];

	sslen = Front_spacing(fr,GENERAL_WAVE);
	std_area = 0.25*sqrt(3.0)*sqr(sslen)*sqr(hmin);
	Max_tri_sqr_area(fr,GENERAL_WAVE) = sqr(max_tri_area_fac*std_area);
	Min_tri_sqr_area(fr,GENERAL_WAVE) = sqr(min_tri_area_fac*std_area);
	Max_bond_len(fr,GENERAL_WAVE) = sslen*sqrt(max_tri_area_fac)*hmin;
	Min_bond_len(fr,GENERAL_WAVE) = sslen*sqrt(min_tri_area_fac)*hmin;
	a = radians(min_angle_at_vertex);
	Aspect_ratio_tolerance(fr,GENERAL_WAVE) = 0.5*sin(a)/(2.0 - cos(a));
	Max_scaled_tri_side_sqr_length(fr) = sqr(max_scaled_tri_side);
}	/* end FrontSetTriParams */

EXPORT 	void FrontResetTriParams(
	Front *fr)
{
	double max_tri_area_fac = 2.0;
	double min_tri_area_fac = 0.5;
	double min_angle_at_vertex = radians(15.0);
	double max_scaled_tri_side = 1.3;

	FrontSetTriParams(fr,max_tri_area_fac,
			     min_tri_area_fac,
			     min_angle_at_vertex,
			     max_scaled_tri_side);
}	/* end FrontResetTriParams */


EXPORT	void	FT_Init(
	int		argc,
	char		**argv,
	F_BASIC_DATA 	*f_basic)
{
	char *in_name      = f_basic->in_name;
	char *out_name     = f_basic->out_name;
	char *restart_name = f_basic->restart_name;
	int  *subdomains   = f_basic->subdomains;
        int i,total_num_proc = 1;
	char dirname[256];
	char file_name[256];

	f_basic->ReadFromInput = NO;
	f_basic->RestartRun = NO;

        pp_init(&argc,&argv);

	argc--;
	argv++;
	/* Set for default */
	strcpy(out_name,"intfc");
	for (i = 0; i < MAXD; ++i)
            subdomains[i] = 1;
	f_basic->coord_system = IDENTITY_REMAP;
	while (argc >= 1)
	{
	    if (argv[0][0] != '-')
	    {
		printf("Usage: example -d dimension -i input -o output\n");
		exit(1);
	    }
	    switch(argv[0][1]) {
	    case 'i':
	    case 'I':
	    	f_basic->ReadFromInput = YES;
	    	zero_scalar(in_name,200);
                strcpy(in_name,argv[1]);
                argc -= 2;
		argv += 2;
		break;
	    case 'r':
	    case 'R':
	    	f_basic->RestartRun = YES;
	    	zero_scalar(restart_name,200);
                strcpy(restart_name,argv[1]);
                argc -= 2;
		argv += 2;
		break;
	    case 't':
	    case 'T':
	    	f_basic->RestartStep = atoi(argv[1]);
                argc -= 2;
                argv += 2;
                break;
	    case 'd':
	    case 'D':
	    	f_basic->dim = atoi(argv[1]);
                argc -= 2;
                argv += 2;
                break;
	    case 'c':
	    case 'C':
		switch (argv[1][0])
		{
		case 'c':
		case 'C':
		    f_basic->coord_system = CYLINDRICAL_REMAP;
		    break;
		case 's':
		case 'S':
		    f_basic->coord_system = SPHERICAL_REMAP;
		    break;
		default:
		    f_basic->coord_system = IDENTITY_REMAP;
		}
                argc -= 2;
                argv += 2;
                break;
	    case 'o':
	    case 'O':
		zero_scalar(dirname,200);
		strcpy(dirname,argv[1]);
		if (pp_min_status(create_directory(dirname,NO)) == NO)
		{
		    screen("Cannot create directory %s\n",dirname);
		    clean_up(ERROR);    
		}
		if (pp_numnodes() > 1)
                    sprintf(file_name,"%s/run-output.%d",dirname,pp_mynode());
		else
                    sprintf(file_name,"%s/run-output",dirname);
		freopen(file_name,"w",stdout);
		zero_scalar(out_name,200);
		strcpy(out_name,argv[1]);
		argc -= 2;
		argv += 2;
		break;
#if defined(__MPI__)
            case 'p':
            case 'P':
                for (i = 0; i < MAXD; ++i)
                {
                    if (argc < 2 || argv[1][0] == '-') break;
                    argc -= 1;
                    argv += 1;
                    subdomains[i] = atoi(argv[0]);
                    total_num_proc *= subdomains[i];
                }
                argc -= 1;
                argv += 1;
                if (total_num_proc != pp_numnodes())
                {
                    printf("total number of processors for the partition %d "
                           "does not equal to requested np %d\n",
                           total_num_proc,pp_numnodes());
                    clean_up(ERROR);
                }
		break;
#endif /* defined(__MPI__) */
	    default:
		argc -= 2;
		argv += 2;
	    }
	}
}	/* end FrontInitStatndardIO */

EXPORT	void FT_AddMovieFrame(
	Front *front,
	char *out_name,
	boolean print_in_binary)
{
	show_front_output(front,out_name,print_in_binary);
}	/* end FT_AddMovieFrame */

EXPORT	void FT_Save(
	Front *front,
	char *out_name)
{
	print_front_output(front,out_name);
}	/* end FT_Save */

EXPORT	void FrontFreeAll(
	Front *front)
{
	free_front(front);
}	/* end FrontFreeAll */

EXPORT	void FT_MakeGridIntfc(
	Front *front)
{
	front->grid_intfc = make_grid_intfc(front->interf,
			EXPANDED_DUAL_GRID,NULL);
}	/* end FT_MakeGridIntfc */

EXPORT	void FT_FreeGridIntfc(
	Front *front)
{
	free_grid_intfc(front->grid_intfc);
	front->grid_intfc = NULL;
}	/* end FT_FreeGridIntfc */

EXPORT	void FT_ParallelExchIntfcBuffer(
	Front *front)
{
	scatter_front(front);
}	/* end FT_ParallelExchIntfcBuffer */

EXPORT	void FT_ParallelExchGridArrayBuffer(
	double *grid_array,
	Front *front)
{
	scatter_top_grid_float_array(grid_array,front);
}	/* end FT_ParallelExchGridArrayBuffer */

EXPORT	HYPER_SURF *FT_HyperSurfAtGridCrossing(
	Front *front,
	int *icoords,
	GRID_DIRECTION dir,
	int w_type)
{
	static CRXING *crxs[MAX_NUM_CRX];
	INTERFACE *grid_intfc = front->grid_intfc;
	int nc,crx_index;

	nc = GridSegCrossing(crxs,icoords,dir,grid_intfc);
	if (nc == 0) 
	    return NULL;
	if (dir == EAST || dir == NORTH || dir == UPPER)
	    crx_index = 0;
	else
	    crx_index = nc - 1;

	if (crxs[crx_index]->hs != NULL)
	{
	    if (w_type == ANY_WAVE_TYPE)
		return crxs[crx_index]->hs;
	    else if (w_type == wave_type(crxs[crx_index]->hs))
		return crxs[crx_index]->hs;
	    else 
		return NULL;
	}
	return NULL;
}	/* end FT_HyperSurfAtGridCrossing */

EXPORT	boolean FT_StateVarAtGridCrossing(
	Front *front,
	int *icoords,
	GRID_DIRECTION dir,
	COMPONENT comp,
	double (*state_func)(Locstate),
	double *ans,
	double *crx_coords)
{
	Locstate state;
	HYPER_SURF *hs;

	if (!FT_StateStructAtGridCrossing(front,icoords,dir,comp,
		&state,&hs,crx_coords))
	    return NO;
	*ans = (*state_func)(state);
	return YES;
}	/* end FT_StateVarAtGridCrossing */

EXPORT	boolean FT_NormalAtGridCrossing(
	Front *front,
	int *icoords,
	GRID_DIRECTION dir,
	COMPONENT comp,
	double *nor,
	HYPER_SURF **hs,
	double *crx_coords)
{
        int j;
	int crx_index;
	INTERFACE *grid_intfc = front->grid_intfc;
	static CRXING *crxs[MAX_NUM_CRX];
	int i,nc,dim = grid_intfc->dim;
	FT_POINT *p;
	BOND *b;
	TRI *t;
	double len;
	const double *tnor;

	nc = GridSegCrossing(crxs,icoords,dir,grid_intfc);
	if (nc == 0) return NO;

	if (dir == EAST || dir == NORTH || dir == UPPER)
	    crx_index = 0;
	else
	    crx_index = nc - 1;

	*hs = crxs[crx_index]->hs;
	p = crxs[crx_index]->pt;
	/* if comp is not associated with hs or not specified, normal
	   is the natural normal of the surface.
	if (comp == negative_component(*hs) ||
	    comp == positive_component(*hs))
	*/
	{
	    for (i = 0; i < dim; ++i)
		crx_coords[i] = Coords(p)[i];
	    switch (dim)
	    {
	    case 2:
		b = crxs[crx_index]->bond;
		if (b == NULL)
		    b = crxs[crx_index]->bond = Bond_of_hse(p->hse);
		normal(p,Hyper_surf_element(b),*hs,nor,front);
		if (comp == negative_component(*hs))
		{
	    	    for (i = 0; i < dim; ++i)
			nor[i] *= -1.0;
		}
		break;
	    case 3:
		t = crxs[crx_index]->tri;
		len = sqr_norm(t);
		len = sqrt(len);
		tnor = Tri_normal(t);
	    	for (i = 0; i < dim; ++i)
		    nor[i] = tnor[i]/len;
		if (comp == negative_component(*hs))
		{
	    	    for (i = 0; i < dim; ++i)
			nor[i] *= -1.0;
		}
		break;
	    default: 
	    	screen("ERROR: In FT_NormalAtGridCrossing(),"
			"unsupported dimension dim = %d\n",dim);
		return NO;
	    }
	}
	return YES;
}	/* end FT_NormalAtGridCrossing */

#define         MAX_NUM_VERTEX_IN_CELL          20
EXPORT	boolean FT_StateStructAtGridCrossing(
	Front *front,
	int *icoords,
	GRID_DIRECTION dir,
	COMPONENT comp,
	Locstate *state,
	HYPER_SURF **hs,
	double *crx_coords)
{
        int j;
	int crx_index;
	INTERFACE *grid_intfc = front->grid_intfc;
	static CRXING *crxs[MAX_NUM_CRX];
	int i,nc,dim = grid_intfc->dim;

	crx_index = 0;
	nc = GridSegCrossing(crxs,icoords,dir,grid_intfc);
	if (nc == 0) return NO;
	if (dir == EAST || dir == NORTH || dir == UPPER)
	    crx_index = 0;
	else
	    crx_index = nc - 1;

	*hs = crxs[crx_index]->hs;
	if (comp == negative_component(*hs))
	    *state = left_state(crxs[crx_index]->pt);
	else if (comp == positive_component(*hs))
	    *state = right_state(crxs[crx_index]->pt);
	else
	{
	    *state = NULL;
	    screen("ERROR: In FT_StateVarAtGridCrossing(),"
			"component does not match\n");
	    return NO;
	}
	for (i = 0; i < dim; ++i)
	    crx_coords[i] = Coords(crxs[crx_index]->pt)[i];
	return YES;
}	/* end FT_StateStructAtGridCrossing */

EXPORT	boolean FT_StateStructAtGridCrossing2(
	Front *front,
	int *icoords,
	GRID_DIRECTION dir,
	COMPONENT comp,
	Locstate *state,
	HYPER_SURF **hs,
	HYPER_SURF_ELEMENT **hse,
	double *crx_coords)
{
        int j;
	int crx_index;
	INTERFACE *grid_intfc = front->grid_intfc;
	static CRXING *crxs[MAX_NUM_CRX];
	int i,nc,dim = grid_intfc->dim;

	crx_index = 0;
	nc = GridSegCrossing(crxs,icoords,dir,grid_intfc);
	if (nc == 0) return NO;
	if (dir == EAST || dir == NORTH || dir == UPPER)
	    crx_index = 0;
	else
	    crx_index = nc - 1;

	*hs = crxs[crx_index]->hs;
	if (comp == negative_component(*hs))
	    *state = left_state(crxs[crx_index]->pt);
	else if (comp == positive_component(*hs))
	    *state = right_state(crxs[crx_index]->pt);
	else
	{
	    *state = NULL;
	    screen("ERROR: In FT_StateVarAtGridCrossing(),"
			"component does not match\n");
	    return NO;
	}
	for (i = 0; i < dim; ++i)
	    crx_coords[i] = Coords(crxs[crx_index]->pt)[i];
	if (dim == 2)
	{
	    *hse = Hyper_surf_element(crxs[crx_index]->bond);
	}
	else if (dim == 3)
	{
	    *hse = Hyper_surf_element(crxs[crx_index]->tri);
	}
	return YES;
}	/* end FT_StateStructAtGridCrossing2 */

LOCAL double lin_cell_tol;

EXPORT	boolean FT_IntrpStateVarAtCoords(
	Front *front,
	COMPONENT comp,
	double *coords,
	double *grid_array,
	double (*get_state)(Locstate),
	double *ans,
	double *default_ans)
{
	int icoords[MAXD];
	INTERFACE *grid_intfc = front->grid_intfc;
	static INTRP_CELL *blk_cell;
	RECT_GRID *gr = &topological_grid(grid_intfc);
	int i,dim = gr->dim;

	if (blk_cell == NULL)
	{
	    scalar(&blk_cell,sizeof(INTRP_CELL));
	    uni_array(&blk_cell->var,MAX_NUM_VERTEX_IN_CELL,sizeof(double));
	    bi_array(&blk_cell->coords,MAX_NUM_VERTEX_IN_CELL,MAXD,
						sizeof(double));
	    bi_array(&blk_cell->p_lin,MAXD+1,MAXD,sizeof(double));
	    uni_array(&blk_cell->var_lin,MAXD+1,sizeof(double));
	    lin_cell_tol = 1.0;
	    for (i = 0; i < dim; ++i)
	    	lin_cell_tol *= 0.00001*gr->h[i];
	}

	if (!rect_in_which(coords,icoords,gr))
	{
	    *ans = 0.0;
	    return NO;
	}
	collect_cell_ptst(blk_cell,icoords,comp,front,grid_array,get_state);
	if (blk_cell->is_bilinear)
	{
	    if (debugging("the_pt"))
		printf("Bilinear cell interpolate\n");
	    *ans = FrontBilinIntrp(coords,blk_cell,NO);
	    return YES;
	}
	else if (build_linear_element(blk_cell,coords))
	{
	    if (debugging("the_pt"))
		printf("Linear cell interpolate\n");
	    *ans = FrontLinIntrp(coords,blk_cell,NO);
	    return YES;
	}
	else if (default_ans != NULL)
	{
	    if (debugging("the_pt"))
		printf("Using default interpolate\n");
	    *ans = *default_ans;
	}
	else
	{
	    static Locstate state;
	    if (state == NULL)
		scalar(&state,front->sizest);
	    nearest_intfc_state(coords,comp,front->interf,state,NULL,NULL);
	    *ans = get_state(state);
	    return YES;
	}
}	/* end FT_IntrpStateVarAtCoords */

EXPORT boolean FrontNearestIntfcState(
	Front *front,
	double *coords,
	COMPONENT comp,
	Locstate state)
{
	return nearest_intfc_state(coords,comp,front->interf,state,NULL,NULL);
}	/* end FrontNearestIntfcState */

LOCAL boolean build_linear_element(
	INTRP_CELL *blk_cell,
	double *coords)
{
	int dim = blk_cell->dim;
	double **ps = blk_cell->coords;
	double *vars = blk_cell->var;
	double **p = blk_cell->p_lin;
	double *var = blk_cell->var_lin;
	int i,j,k,l,nv = blk_cell->nv;
	double dist[MAX_NUM_VERTEX_IN_CELL];
	double dp[MAX_NUM_VERTEX_IN_CELL][MAXD];

	for (i = 0; i < nv; i++)
	{
	    dist[i] = 0.0;
	    for (j = 0; j < dim; ++j)
	    {
	   	dist[i] += sqr(coords[j] - ps[i][j]);
	   	dp[i][j] = fabs(coords[j] - ps[i][j]);
	    }
	}
	for (i = 0; i < nv; ++i)
	{
	    for (j = i+1; j < nv; ++j)
	    {
		/*if (dist[i] > dist[j])*/
		if (new_vtx_is_closer(dist[i],dist[j],dp[i],dp[j],dim))
		{
		    double tmp;
		    tmp = dist[j];
		    dist[j] = dist[i];
		    dist[i] = tmp;
		    tmp = vars[j];
		    vars[j] = vars[i];
		    vars[i] = tmp;
		    for (k = 0; k < dim; ++k)
		    {
		    	tmp = ps[j][k];
		    	ps[j][k] = ps[i][k];
		    	ps[i][k] = tmp;
		    }
		}
	    }
	}
	switch(dim)
	{
	case 1:
	    for (i = 0; i < nv; i++)
	    {
	    	p[0] = blk_cell->coords[i];
	    	var[0] =  blk_cell->var[i];
	    	for (j = i+1; j < nv; j++)
		{
	    	    p[1] = blk_cell->coords[j];
	    	    var[1] = blk_cell->var[j];
		    if (test_point_in_seg(coords,p) == YES)
	    		return FUNCTION_SUCCEEDED;
		}
	    }
	    break;
	case 2:
	    for (i = 0; i < nv; i++)
	    {
	    	p[0] = blk_cell->coords[i];
	    	var[0] =  blk_cell->var[i];
	    	for (j = i+1; j < nv; j++)
		{
	    	    p[1] = blk_cell->coords[j];
	    	    var[1] = blk_cell->var[j];
		    for (k = j+1; k < nv; k++)
		    {
	    	    	p[2] = blk_cell->coords[k];
	    	    	var[2] = blk_cell->var[k];
			if (test_point_in_tri(coords,p) == YES)
	    		    return FUNCTION_SUCCEEDED;
		    }
		}
	    }
	    break;
	case 3:
	    for (i = 0; i < nv; i++)
	    {
	    	p[0] = blk_cell->coords[i];
	    	var[0] = blk_cell->var[i];
	    	for (j = i+1; j < nv; j++)
		{
	    	    p[1] = blk_cell->coords[j];
	    	    var[1] = blk_cell->var[j];
		    for (k = j+1; k < nv; k++)
		    {
	    	    	p[2] = blk_cell->coords[k];
	    	    	var[2] = blk_cell->var[k];
			for (l = k+1; l < nv; l++)
			{
	    	    	    p[3] = blk_cell->coords[l];
	    	    	    var[3] = blk_cell->var[l];
			    if (test_point_in_tetra(coords,p) == YES)
			    {
	    		        return FUNCTION_SUCCEEDED;
			    }
			}
		    }
		}
	    }
	}
	return FUNCTION_FAILED;
}	/* end build_linear_element */

LOCAL void collect_cell_ptst(
	INTRP_CELL *blk_cell,
	int *icoords,
	COMPONENT comp,
	Front *front,
	double *grid_array,
	double (*get_state)(Locstate))
{
	INTERFACE *grid_intfc = front->grid_intfc;
	Table *T = table_of_interface(grid_intfc);
	RECT_GRID *gr = &topological_grid(grid_intfc);
	int dim = gr->dim;
	int *gmax = gr->gmax;
	double *L = gr->L;
	double *h = gr->h;
	COMPONENT *gr_comp = T->components;
	static COMPONENT cell_comp1d[2];
	static COMPONENT cell_comp2d[2][2];
	static COMPONENT cell_comp3d[2][2][2];
	int i,j,k,index,nv,nc;
	CRXING *crx,*crxs[MAX_NUM_CRX];
	GRID_DIRECTION dir;
	int ic[MAXD];
	boolean fr_crx_grid_seg;
	double state_at_crx;
	double crx_coords[MAXD];
	
	blk_cell->is_bilinear = YES;
	blk_cell->dim = dim;
	nv = 0;
	switch (dim)
	{
	case 1:
	    for (i = 0; i < 2; ++i)
	    {
	    	ic[0] = icoords[0] + i;
	    	index = d_index1d(ic[0],gmax);
	    	cell_comp1d[i] = gr_comp[index];
	    	if (gr_comp[index] == comp || comp == NO_COMP)
	    	{
	    	    blk_cell->coords[nv][0] = L[0] + ic[0]*h[0];
		    blk_cell->var[nv] = grid_array[index];
		    nv++;
	    	}
	    	else
	    	    blk_cell->is_bilinear = NO;
	    }
	    break;
	case 2:
	    for (i = 0; i < 2; ++i)
	    for (j = 0; j < 2; ++j)
	    {
	    	ic[0] = icoords[0] + i;
	    	ic[1] = icoords[1] + j;
	    	index = d_index2d(ic[0],ic[1],gmax);
	    	cell_comp2d[i][j] = gr_comp[index];
	    	if (gr_comp[index] == comp || comp == NO_COMP)
	    	{
	    	    blk_cell->coords[nv][0] = L[0] + ic[0]*h[0];
	    	    blk_cell->coords[nv][1] = L[1] + ic[1]*h[1];
		    blk_cell->var[nv] = grid_array[index];
		    nv++;
	    	}
	    	else
	    	    blk_cell->is_bilinear = NO;
	    }
	    break;
	case 3:
	    for (i = 0; i < 2; ++i)
	    for (j = 0; j < 2; ++j)
	    for (k = 0; k < 2; ++k)
	    {
	    	ic[0] = icoords[0] + i;
	    	ic[1] = icoords[1] + j;
	    	ic[2] = icoords[2] + k;
	    	index = d_index3d(ic[0],ic[1],ic[2],gmax);
	    	cell_comp3d[i][j][k] = gr_comp[index];
	    	if (gr_comp[index] == comp || comp == NO_COMP)
	    	{
	    	    blk_cell->coords[nv][0] = L[0] + ic[0]*h[0];
	    	    blk_cell->coords[nv][1] = L[1] + ic[1]*h[1];
	    	    blk_cell->coords[nv][2] = L[2] + ic[2]*h[2];
		    blk_cell->var[nv] = grid_array[index];
		    nv++;
	    	}
	    	else
	    	    blk_cell->is_bilinear = NO;
	    }
	    break;
	}
	if (blk_cell->is_bilinear == YES) 
	{
	    blk_cell->nv = nv;
	    return;
	}
	switch (dim)
	{
	case 1:
	    for (i = 0; i < 2; ++i)
	    {
	    	ic[0] = icoords[0] + i;
	    	if (cell_comp1d[i] == comp)
		{
	    	    if (cell_comp1d[(i+1)%2] != comp)
		    {
		    	dir = (i < (i+1)%2) ? EAST : WEST;
			fr_crx_grid_seg = FT_StateVarAtGridCrossing(front,ic,dir,
				comp,get_state,&state_at_crx,crx_coords);
		    	if (fr_crx_grid_seg)
		    	{
		    	    blk_cell->var[nv] = state_at_crx;
		    	    blk_cell->coords[nv][0] = crx_coords[0];
		    	    nv++;
		    	}
		    }
		}
	    }
	    break;
	case 2:
	    for (i = 0; i < 2; ++i)
	    for (j = 0; j < 2; ++j)
	    {
	    	ic[0] = icoords[0] + i;
	    	ic[1] = icoords[1] + j;
	    	if (cell_comp2d[i][j] == comp)
	    	{
	    	    if (cell_comp2d[(i+1)%2][j] != comp)
	    	    {
		    	dir = (i < (i+1)%2) ? EAST : WEST;
			fr_crx_grid_seg = FT_StateVarAtGridCrossing(front,ic,dir,
				comp,get_state,&state_at_crx,crx_coords);
		    	if (fr_crx_grid_seg)
		    	{
		    	    blk_cell->var[nv] = state_at_crx;
		    	    blk_cell->coords[nv][0] = crx_coords[0];
		    	    blk_cell->coords[nv][1] = crx_coords[1];
		    	    nv++;
		    	}
	    	    }
	    	    if (cell_comp2d[i][(j+1)%2] != comp)
	    	    {
		    	dir = (j < (j+1)%2) ? NORTH : SOUTH;
			fr_crx_grid_seg = FT_StateVarAtGridCrossing(front,ic,dir,
				comp,get_state,&state_at_crx,crx_coords);
		    	if (fr_crx_grid_seg)
		    	{
		    	    blk_cell->var[nv] = state_at_crx;
		    	    blk_cell->coords[nv][0] = crx_coords[0];
		    	    blk_cell->coords[nv][1] = crx_coords[1];
		    	    nv++;
		    	}
	    	    }
	    	}
	    }
	    break;
	case 3:
	    for (i = 0; i < 2; ++i)
	    for (j = 0; j < 2; ++j)
	    for (k = 0; k < 2; ++k)
	    {
	    	ic[0] = icoords[0] + i;
	    	ic[1] = icoords[1] + j;
	    	ic[2] = icoords[2] + k;
	    	if (cell_comp3d[i][j][k] == comp)
	    	{
	    	    if (cell_comp3d[(i+1)%2][j][k] != comp)
	    	    {
		    	dir = (i < (i+1)%2) ? EAST : WEST;
			fr_crx_grid_seg = FT_StateVarAtGridCrossing(front,ic,dir,
				comp,get_state,&state_at_crx,crx_coords);
		    	if (fr_crx_grid_seg)
		    	{
		    	    blk_cell->var[nv] = state_at_crx;
		    	    blk_cell->coords[nv][0] = crx_coords[0];
		    	    blk_cell->coords[nv][1] = crx_coords[1];
		    	    blk_cell->coords[nv][2] = crx_coords[2];
		    	    nv++;
		    	}
	    	    }
	    	    if (cell_comp3d[i][(j+1)%2][k] != comp)
	    	    {
		    	dir = (j < (j+1)%2) ? NORTH : SOUTH;
			fr_crx_grid_seg = FT_StateVarAtGridCrossing(front,ic,dir,
				comp,get_state,&state_at_crx,crx_coords);
		    	if (fr_crx_grid_seg)
		    	{
		    	    blk_cell->var[nv] = state_at_crx;
		    	    blk_cell->coords[nv][0] = crx_coords[0];
		    	    blk_cell->coords[nv][1] = crx_coords[1];
		    	    blk_cell->coords[nv][2] = crx_coords[2];
		    	    nv++;
		    	}
	    	    }
	    	    if (cell_comp3d[i][j][(k+1)%2] != comp)
	    	    {
		    	dir = (k < (k+1)%2) ? UPPER : LOWER;
			fr_crx_grid_seg = FT_StateVarAtGridCrossing(front,ic,dir,
				comp,get_state,&state_at_crx,crx_coords);
		    	if (fr_crx_grid_seg)
		    	{
		    	    blk_cell->var[nv] = state_at_crx;
		    	    blk_cell->coords[nv][0] = crx_coords[0];
		    	    blk_cell->coords[nv][1] = crx_coords[1];
		    	    blk_cell->coords[nv][2] = crx_coords[2];
		    	    nv++;
		    	}
	    	    }
	    	}
	    }
	    break;
	}
	blk_cell->nv = nv;
}	/* end collect_cell_ptst */


LOCAL 	boolean test_point_in_tetra(
	double	*coords,
	double	**p)
{
	double		*p0, *p1, *p2, *p3;
	double		a[3],b[3],c[3],v[3];
	double		D,Dp;
	int		i,j;

	for (i = 0; i < 4; i++)
	{
	    p0 = p[i];
	    p1 = p[(i+1)%4];
	    p2 = p[(i+2)%4];
	    p3 = p[(i+3)%4];

	    for (j = 0; j < 3; j++)
	    {
	    	a[j] = p1[j] - p0[j];
	    	b[j] = p2[j] - p0[j];
	    	c[j] = p3[j] - p0[j];
	    	v[j] = coords[j] - p0[j];
	    }
	    D = Det3d(a,b,c);
	    if (fabs(D) < lin_cell_tol) return NO;
	    Dp = Det3d(a,b,v);
	    if ((D > 0.0 && Dp < 0.0) ||
	        (D < 0.0 && Dp > 0.0))
		return NO;
	}
	return YES;
}	/* end test_point_in_tetra */


LOCAL	boolean	test_point_in_seg(
	double	*coords,
	double	**p)
{
	if ((p[0][0]-lin_cell_tol < coords[0] &&
	     p[1][0]+lin_cell_tol > coords[0]) ||
	    (p[1][0]-lin_cell_tol < coords[0] &&
	     p[0][0]+lin_cell_tol > coords[0])) 
	    return YES;
	else
	    return NO;
}
	/* end test_point_in_seg */

LOCAL	boolean	test_point_in_tri(
	double	*coords,
	double	**p)
{
	double	x0, y0, x1, y1, x2, y2, cp;
	double	v1[2],v2[2];

	x0 = p[0][0];	y0 = p[0][1];
	x1 = p[1][0];	y1 = p[1][1];
	x2 = p[2][0];	y2 = p[2][1];

	v1[0] = x2 - x1;	v1[1] = y2 - y1;
	v2[0] = x0 - x1;	v2[1] = y0 - y1;
	Cross2d(v1,v2,cp);

	if (fabs(cp) < lin_cell_tol) 
	    return NO;                  /* degenerated triangle */
	else if (cp > 0.0)		/* counterclockwise triangle */
	{
	    v1[0] = x1 - x0;		v1[1] = y1 - y0;
	    v2[0] = coords[0] - x0;	v2[1] = coords[1] - y0;
	    Cross2d(v1,v2,cp);
	    if (cp < -lin_cell_tol)
	    	return NO;
	    v1[0] = x2 - x1;		v1[1] = y2 - y1;
	    v2[0] = coords[0] - x1;	v2[1] = coords[1] - y1;
	    Cross2d(v1,v2,cp);
	    if (cp < -lin_cell_tol)
	    	return NO;
	    v1[0] = x0 - x2;		v1[1] = y0 - y2;
	    v2[0] = coords[0] - x2;	v2[1] = coords[1] - y2;
	    Cross2d(v1,v2,cp);
	    if (cp < -lin_cell_tol)
	    	return NO;
	}
	else				      /* clockwise triangle */
	{
	    v1[0] = x1 - x0;		v1[1] = y1 - y0;
	    v2[0] = coords[0] - x0;	v2[1] = coords[1] - y0;
	    Cross2d(v1,v2,cp);
	    if (cp > lin_cell_tol)
	    	return NO;
	    v1[0] = x2 - x1;		v1[1] = y2 - y1;
	    v2[0] = coords[0] - x1;	v2[1] = coords[1] - y1;
	    Cross2d(v1,v2,cp);
	    if (cp > lin_cell_tol)
	    	return NO;
	    v1[0] = x0 - x2;		v1[1] = y0 - y2;
	    v2[0] = coords[0] - x2;	v2[1] = coords[1] - y2;
	    Cross2d(v1,v2,cp);
	    if (cp > lin_cell_tol)
	    	return NO;
	}
	return YES;
}	/*end test_point_in_tri*/

EXPORT	double FrontLinIntrp(
	double *crds,
	INTRP_CELL *blk_cell,
	boolean reuse_coeffs)
{
	double **p = blk_cell->p_lin;
	double *var = blk_cell->var_lin;
	double ans;
	int dim = blk_cell->dim;
	static double f[MAXD+1];

	switch (dim)
	{
	case 1:
	{
	    double den = fabs(p[0][0] - p[1][0]);
	    f[0] = fabs(crds[0] - p[1][0])/den;
	    f[1] = fabs(crds[0] - p[0][0])/den;
	    if (f[0] < 0.0)
	    {
	    	f[0] = 0.0;	f[1] = 1.0;
	    }
	    else if (f[0] > 1.0)
	    {
	    	f[0] = 1.0;	f[1] = 0.0;
	    }
	    ans = f[0]*var[0] + f[1]*var[1];
	}
	break;
	case 2:
	{
	    double x0, y0, x1, y1, x2, y2;
	    double xx, yy;
	    double den;

	    x0 = p[0][0];    y0 = p[0][1];
	    x1 = p[1][0] - x0;    y1 = p[1][1] - y0;
	    x2 = p[2][0] - x0;    y2 = p[2][1] - y0;
	    xx = crds[0] - x0;        yy = crds[1] - y0;
	    den = x1*y2 - y1*x2;
	    f[1] = (xx*y2 - yy*x2) / den;
	    f[2] = (x1*yy - y1*xx) / den;
	    f[0] = 1.0 - f[1] - f[2];
	    f[0] = max(0.0,f[0]);	f[0] = min(1.0,f[0]);
	    f[1] = max(0.0,f[1]);	f[1] = min(1.0,f[1]);
	    f[2] = max(0.0,f[2]);	f[2] = min(1.0,f[2]);
	    ans = f[0]*var[0] + f[1]*var[1] + f[2]*var[2];
	}
	break;
	case 3:
	{
	    double v10, v11, v12;
	    double v20, v21, v22;
	    double v30, v31, v32;
	    double q0, q1, q2;
	    double *p0, *p1, *p2, *p3;
	    double den;

	    p0 = p[0];    p2 = p[2];
	    p1 = p[1];    p3 = p[3];
	    q0 = crds[0] - p0[0]; q1 = crds[1] - p0[1]; q2 = crds[2] - p0[2];
	    v10 = p1[0] - p0[0]; v11 = p1[1] - p0[1]; v12 = p1[2] - p0[2];
	    v20 = p2[0] - p0[0]; v21 = p2[1] - p0[1]; v22 = p2[2] - p0[2];
	    v30 = p3[0] - p0[0]; v31 = p3[1] - p0[1]; v32 = p3[2] - p0[2];
	    den = QDet3d(v1,v2,v3);
	    if (fabs(den) < MACH_EPS)
	    {
	        f[0] = 0.0;
	        f[1] = 0.0;
	        f[2] = 0.0;
	        f[3] = 1.0;
	    }

	    f[1] = QDet3d(q,v2,v3)/den;
	    f[2] = QDet3d(v1,q,v3)/den;
	    f[3] = QDet3d(v1,v2,q)/den;
	    f[0] = 1.0 - f[1] - f[2] - f[3];
	    f[0] = max(0.0,f[0]);	f[0] = min(1.0,f[0]);
	    f[1] = max(0.0,f[1]);	f[1] = min(1.0,f[1]);
	    f[2] = max(0.0,f[2]);	f[2] = min(1.0,f[2]);
	    f[3] = max(0.0,f[3]);	f[3] = min(1.0,f[3]);
	    ans = f[0]*var[0] + f[1]*var[1] + f[2]*var[2] + f[3]*var[3];
	}
	}
	return ans;
}	/* end FrontLinIntrp */

EXPORT  double FrontBilinIntrp(
        double *crds,
        INTRP_CELL *blk_cell,
        boolean reuse_coeffs)
{
	static double f[MAXD][2];
	int i,j,k,il,iu,index;
	int dim = blk_cell->dim;
	double ans;
	double l[MAXD],u[MAXD];

	if (!reuse_coeffs)
	{
	    il = 0;
	    switch (dim)
	    {
	    case 1:
		iu = 1;
		break;
	    case 2:
		iu = 3;
		break;
	    case 3:
		iu = 7;
	    }
	    for (i = 0; i < dim; ++i)
	    {
	    	l[i] = blk_cell->coords[il][i];
		u[i] = blk_cell->coords[iu][i];
	    	f[i][0] = (u[i] - crds[i])/(u[i] - l[i]);
		f[i][0] = min(f[i][0],1.0);
		f[i][0] = max(f[i][0],0.0);
	    	f[i][1] = 1.0 - f[i][0];
	    }
	}
	index = 0;
	ans = 0.0;
	switch (dim)
	{
	case 1:
	    for (i = 0; i < 2; ++i)
	    	ans += blk_cell->var[index++]*f[0][i];
	    break;
	case 2:
	    for (i = 0; i < 2; ++i)
	    for (j = 0; j < 2; ++j)
	    {
	    	ans += blk_cell->var[index++]*f[0][i]*f[1][j];
	    }
	    break;
	case 3:
	    for (i = 0; i < 2; ++i)
	    for (j = 0; j < 2; ++j)
	    for (k = 0; k < 2; ++k)
	    {
	    	ans += blk_cell->var[index++]*f[0][i]*f[1][j]*f[2][k];
	    }
	}
	return ans;

}	/* end FrontBilinIntrp */

EXPORT	boolean FrontCpuAdaptSubdomain(
	Front *front,
	double cpu_time,
	int *lexpand,
	int *uexpand)
{
	return cpu_adapt_front(front,cpu_time,lexpand,uexpand);
}	/* end FrontCpuAdaptSubdomain */

EXPORT void FT_ReadSpaceDomain(
        char *in_name,
        F_BASIC_DATA *f_basic)
{
	
	FILE *infile;
	char input_string[200],sbdry[200];
	int i;

	infile = fopen(in_name,"r");
	for (i = 0; i < f_basic->dim; ++i)
	{
            sprintf(input_string,"Domain limit in %d-th dimension:",i);
            CursorAfterString(infile,input_string);
            fscanf(infile,"%lf %lf",&f_basic->L[i],&f_basic->U[i]);
            (void) printf("%f %f\n",f_basic->L[i],f_basic->U[i]);
	}
	CursorAfterString(infile,"Computational grid:");
	for (i = 0; i < f_basic->dim; ++i)
	{
	    fscanf(infile,"%d",&f_basic->gmax[i]);
	    (void) printf("%d ",f_basic->gmax[i]);
	}
	(void) printf("\n");
	for (i = 0; i < f_basic->dim; ++i)
	{
            sprintf(input_string,"Lower boundary in %d-th dimension:",i);
            CursorAfterString(infile,input_string);
            fscanf(infile,"%s",sbdry);
	    (void) printf("%s\n",sbdry);
	    switch (sbdry[0])
	    {
	    case 'R':
	    case 'r':
	    	f_basic->boundary[i][0] = REFLECTION_BOUNDARY;
		break;
	    case 'P':
	    case 'p':
		if (sbdry[1] == 'a' || sbdry[1] == 'A')
	    	    f_basic->boundary[i][0] = PASSIVE_BOUNDARY;
		else if (sbdry[1] == 'e' || sbdry[1] == 'E')
	    	    f_basic->boundary[i][0] = PERIODIC_BOUNDARY;
		break;
	    case 'D':
	    case 'd':
	    	f_basic->boundary[i][0] = DIRICHLET_BOUNDARY;
	    	break;
	    case 'N':
	    case 'n':
	    	f_basic->boundary[i][0] = NEUMANN_BOUNDARY;
	    	break;
	    case 'M':
	    case 'm':
	    	f_basic->boundary[i][0] = MIXED_TYPE_BOUNDARY;
	    	break;
	    default:
	    	printf("Unknown boundary!\n");
		clean_up(ERROR);
	    }
            sprintf(input_string,"Upper boundary in %d-th dimension:",i);
            CursorAfterString(infile,input_string);
            fscanf(infile,"%s",sbdry);
	    (void) printf("%s\n",sbdry);
	    switch (sbdry[0])
	    {
	    case 'R':
	    case 'r':
	    	f_basic->boundary[i][1] = REFLECTION_BOUNDARY;
		break;
	    case 'P':
	    case 'p':
		if (sbdry[1] == 'a' || sbdry[1] == 'A')
	    	    f_basic->boundary[i][1] = PASSIVE_BOUNDARY;
		else if (sbdry[1] == 'e' || sbdry[1] == 'E')
	    	    f_basic->boundary[i][1] = PERIODIC_BOUNDARY;
		break;
	    case 'D':
	    case 'd':
	    	f_basic->boundary[i][1] = DIRICHLET_BOUNDARY;
	    	break;
	    case 'N':
	    case 'n':
	    	f_basic->boundary[i][1] = NEUMANN_BOUNDARY;
	    	break;
	    case 'M':
	    case 'm':
	    	f_basic->boundary[i][1] = MIXED_TYPE_BOUNDARY;
	    	break;
	    default:
	    	printf("Unknown boundary!\n");
		clean_up(ERROR);
	    }
	}
	fclose(infile);
}	/* end FT_ReadSpaceDomain */

EXPORT void FT_ReadTimeControl(
	char *in_name,
        Front *front)
{
	FILE *infile;
	char msg[100],s[100];

	infile = fopen(in_name,"r");
        CursorAfterString(infile,"Max time:");
        fscanf(infile,"%lf",&front->max_time);
        (void) printf("%f\n",front->max_time);
        CursorAfterString(infile,"Max step:");
        fscanf(infile,"%d",&front->max_step);
        (void) printf("%d\n",front->max_step);
        CursorAfterString(infile,"Print interval:");
        fscanf(infile,"%lf",&front->print_time_interval);
        (void) printf("%f\n",front->print_time_interval);
        CursorAfterString(infile,"Movie frame interval:");
        fscanf(infile,"%lf",&front->movie_frame_interval);
        (void) printf("%f\n",front->movie_frame_interval);
        CursorAfterString(infile,"CFL factor:");
        fscanf(infile,"%lf",&(Time_step_factor(front)));
        (void) printf("%f\n",Time_step_factor(front));
        CursorAfterString(infile,"Redistribution interval:");
        fscanf(infile,"%d",&(Frequency_of_redistribution(front,GENERAL_WAVE)));
        (void) printf("%d\n",Frequency_of_redistribution(front,GENERAL_WAVE));
	sprintf(msg,"Type yes to turn off auto-redistribution:");
	front->Auto_Redist = YES;	
        if (CursorAfterStringOpt(infile,msg))
	{
            fscanf(infile,"%s",s);
	    (void) printf("%s\n",s);
	    if (s[0] == 'y' || s[0] == 'Y')
		front->Auto_Redist = NO;	
	}
	fclose(infile);
}	/* end FT_ReadTimeControl */


EXPORT	boolean FrontGetRectCellIntrpCoeffs(
	double *p, 
	RECT_GRID *grid,
	int *index, 
	double *coeffs)
{
	/* locate the point */
	int dim = grid->dim;
	int *gmax = grid->gmax;
	int icoords[MAXD];
	int i,j,k;
	double c1[MAXD], c2[MAXD], denominator;

	if (!rect_in_which(p,icoords,grid))
	    return NO;

	switch (dim)
	{
	case 2:
	    i = icoords[0];
	    j = icoords[1];
	    index[0] = d_index2d(i,j,gmax);
	    index[1] = d_index2d(i,j+1,gmax);
	    index[2] = d_index2d(i+1,j,gmax);
	    index[3] = d_index2d(i+1,j+1,gmax);

	    c1[0] = grid->L[0] + i*grid->h[0];
	    c1[1] = grid->L[1] + j*grid->h[1];
	    c2[0] = grid->L[0] + (i+1)*grid->h[0];
	    c2[1] = grid->L[1] + (j+1)*grid->h[1];

	    denominator = (c2[0]-c1[0])*(c2[1]-c1[1]);
	    coeffs[0] = (c2[0]-p[0])*(c2[1]-p[1])/denominator;
	    coeffs[1] = (p[0]-c1[0])*(c2[1]-p[1])/denominator;
	    coeffs[2] = (c2[0]-p[0])*(p[1]-c1[1])/denominator;
	    coeffs[3] = (p[0]-c1[0])*(p[1]-c1[1])/denominator;
	    break;
	case 3:
	    break;
	}
	return YES;
}

/*********************************************************************
*  	Inputs: state_func, state_func_name, or state                *
*********************************************************************/

EXPORT  void FT_SetDirichletBoundary(
        Front *front,
        void (*state_func)(double*,HYPER_SURF*,Front*,POINTER,POINTER),
        const char *state_func_name,
	POINTER state_func_params,
        Locstate state,
        HYPER_SURF *hs)
{
        BOUNDARY_STATE  Bstate;
	int index;
        INTERFACE *intfc = front->interf;

	zero_scalar(&Bstate,sizeof(BOUNDARY_STATE));
	if (state_func != NULL)
	{
            Bstate._boundary_state_function = state_func;
            Bstate._boundary_state_function_name = strdup(state_func_name);
            Bstate._boundary_state_function_params = state_func_params;
	}
	if (state != NULL)
	{
            Bstate._boundary_state = state;
	}
	Bstate._fprint_boundary_state_data = f_fprint_boundary_state_data;
	index = add_bstate_to_list(&Bstate,intfc,-1);
	if (hs)
            bstate_index(hs) = index;
}       /* end FT_SetDirichletBoundary */


EXPORT HYPER_SURF *BoundaryHyperSurf(
	INTERFACE *intfc,
	int w_type,
	int dir,
	int side)
{
	FT_RectBoundaryHypSurf(intfc,w_type,dir,side);
}	/* end BoundaryHyperSurf */

EXPORT HYPER_SURF *FT_RectBoundaryHypSurf(
	INTERFACE *intfc,
	int w_type,
	int dir,
	int side)
{
	RECT_GRID *grid = computational_grid(intfc);
	int i,dim = grid->dim;
	FT_POINT **p,*pt;
	CURVE **c;
	BOND *b;
	SURFACE **s;
	TRI *tri;
	boolean hs_found;
	double bline = (side == 0) ? grid->L[dir] : grid->U[dir];

	switch (dim)
	{
	case 1:
	    for (p = intfc->points; p && *p; ++p)
	    {
		if (wave_type(*p) != w_type || !is_bdry(*p)) continue;
		if (fabs(Coords(*p)[dir]-bline) > grid_tolerance(grid))
		    continue;
		return Hyper_surf(*p);
	    }
	    break;
	case 2:
	    for (c = intfc->curves; c && *c; ++c)
	    {
		if (wave_type(*c) != w_type || !is_bdry(*c)) continue;
		hs_found = YES;
		b = (*c)->first;
		if (fabs(Coords(b->start)[dir]-bline) > grid_tolerance(grid))
		    continue;
		for (; b != NULL; b = b->next)
		{
		    if (fabs(Coords(b->end)[dir]-bline) > grid_tolerance(grid))
		    {
			hs_found = NO;
			break;
		    }
		}
		if (!hs_found) continue;
		else
		    return Hyper_surf(*c);
	    }
	    break;
	case 3:
	    for (s = intfc->surfaces; s && *s; ++s)
	    {
		if (wave_type(*s) != w_type || !is_bdry(*s)) continue;
		hs_found = YES;
		for (tri = first_tri(*s); !at_end_of_tri_list(tri,*s);
				tri = tri->next)
		{
		    for (i = 0; i < 3; ++i)
		    {
			pt = Point_of_tri(tri)[i];
		        if (fabs(Coords(pt)[dir]-bline) > grid_tolerance(grid))
		    	{
			    hs_found = NO;
			    break;
		    	}
		    }
		}
		if (!hs_found) continue;
		else
		    return Hyper_surf(*s);
	    }
	    break;
	}
	return NULL;
}	/* end FT_RectBoundaryHypSurf */

EXPORT HYPER_SURF **FT_InteriorHypSurfs(
	INTERFACE *intfc,
	int w_type,
	int *num_hs)
{
	int dim = intfc->dim;
	FT_POINT **p;
	CURVE **c;
	SURFACE **s;
	static HYPER_SURF **hyp_surfs;

	*num_hs = 0;
	switch (dim)
	{
	case 1:
	    for (p = intfc->points; p && *p; ++p)
	    {
		if (wave_type(*p) != w_type || is_bdry(*p)) continue;
		else (*num_hs)++;
	    }
	    uni_array(&hyp_surfs,*num_hs,sizeof(HYPER_SURF*));
	    *num_hs = 0;
	    for (p = intfc->points; p && *p; ++p)
	    {
		if (wave_type(*p) != w_type || is_bdry(*p)) continue;
		else 
		{
		    hyp_surfs[*num_hs] = Hyper_surf(*p);
		    (*num_hs)++;
		}
	    }
	    break;
	case 2:
	    for (c = intfc->curves; c && *c; ++c)
	    {
		if (wave_type(*c) != w_type || is_bdry(*c)) continue;
		else (*num_hs)++;
	    }
	    uni_array(&hyp_surfs,*num_hs,sizeof(HYPER_SURF*));
	    *num_hs = 0;
	    for (c = intfc->curves; c && *c; ++c)
	    {
		if (wave_type(*c) != w_type || is_bdry(*c)) continue;
		else 
		{
		    hyp_surfs[*num_hs] = Hyper_surf(*c);
		    (*num_hs)++;
		}
	    }
	    break;
	case 3:
	    for (s = intfc->surfaces; s && *s; ++s)
	    {
		if (wave_type(*s) != w_type || is_bdry(*s)) continue;
		else (*num_hs)++;
	    }
	    uni_array(&hyp_surfs,*num_hs,sizeof(HYPER_SURF*));
	    *num_hs = 0;
	    for (s = intfc->surfaces; s && *s; ++s)
	    {
		if (wave_type(*s) != w_type || is_bdry(*s)) continue;
		else 
		{
		    hyp_surfs[*num_hs] = Hyper_surf(*s);
		    (*num_hs)++;
		}
	    }
	}
	return hyp_surfs;
}	/* end FT_InteriorBoundaryHypSurfs */

EXPORT	Tan_stencil **FrontGetTanStencils(
	Front		*fr,
	FT_POINT		*p,
	int nrad)
{
	switch (fr->rect_grid->dim)
	{
	case 1:
	    return NULL;
	case 2:
	    return FrontGetTanStencils2d(fr,p,nrad);
	case 3:
	    return FrontGetTanStencils3d(fr,p,nrad);
	}
}	/* end FrontGetTanStencils */

LOCAL	Tan_stencil **FrontGetTanStencils3d(
	Front		*fr,
	FT_POINT		*p,
	int nrad)
{
	RECT_GRID *gr = fr->rect_grid;
	double kappa,*h = gr->h;
	int i,dim = gr->dim;
	static Tan_stencil **sten = NULL;
	static int current_nrad = 0;
	static  Tparams tp[2];

	if (nrad > current_nrad)
        {
	    if (sten == NULL)
		uni_array(&sten,1,sizeof(Tan_stencil*));
	    for (i = 0; i < dim-1; ++i)
	    {
	    	if (sten[i] != NULL) free_these(1,sten[i]);
            	sten[i] = alloc_tan_stencil(fr,nrad);
	    }
	    current_nrad = nrad;
        }
	set_tol_for_tri_sect(1.0e-6*min3(h[0], h[1], h[2]));
	if (Boundary_point(p))
	    return NULL;
	if (!set_up_tangent_params(fr,p,p->hse,p->hs,tp))
            return NULL;

	FT_CurvatureAtPoint(p,fr,&kappa);
	for (i = 0; i < dim-1; ++i)
	{
	    set_up_tangent_stencil(fr,sten[i],tp+i,p,kappa);
	}
	return sten;
}	/* end FrontGetTanStencils3d */

LOCAL	Tan_stencil **FrontGetTanStencils2d(
	Front		*fr,
	FT_POINT		*p,
	int nrad)
{
	RECT_GRID *gr = fr->rect_grid;
	double ds,*h = gr->h;
	int dim = gr->dim;
	BOND *b = Bond_of_hse(p->hse);
	CURVE *c = Curve_of_hs(p->hs);
	static double tngt[MAXD];
	static Tan_stencil **sten = NULL;
	static int current_nrad = 0;
	Locstate sl,sr;

	if (nrad > current_nrad)
        {
	    if (sten == NULL)
		uni_array(&sten,1,sizeof(Tan_stencil*));
	    if (sten[0] != NULL) free_these(1,sten[0]);
            sten[0] = alloc_tan_stencil(fr,nrad);
	    current_nrad = nrad;
        }
	tangent(p,b,c,tngt,fr);
	ds = grid_size_in_direction(tngt,h,dim);

	    /* find the stencil states */

	states_at_distance_along_curve(p,b,c,NEGATIVE_ORIENTATION,ds,nrad,
			sten[0]->leftst-1,sten[0]->rightst-1,sten[0]->hs-1,
			sten[0]->hse-1,sten[0]->t-1,sten[0]->p-1,fr);
	states_at_distance_along_curve(p,b,c,POSITIVE_ORIENTATION,ds,nrad,
			sten[0]->leftst+1,sten[0]->rightst+1,sten[0]->hs+1,
			sten[0]->hse+1,sten[0]->t+1,sten[0]->p+1,fr);

	sten[0]->p[0] = p;
	sten[0]->hse[0] = p->hse;
	sten[0]->hs[0] = p->hs;
	sten[0]->t[0] = 1.0;
	sten[0]->dir = tngt;
	slsr(p,p->hse,p->hs,&sl,&sr);
	ft_assign(sten[0]->leftst[0],sl,fr->sizest);
	ft_assign(sten[0]->rightst[0],sr,fr->sizest);

	sten[0]->curvature = mean_curvature_at_point(sten[0]->p[0],
			sten[0]->hse[0],sten[0]->hs[0],fr);
	return sten;
}	/*end FrontGetTanStencils2d */

/*
	Get the mean curvature at the point p (must be a point on 
	the interface of the front.
*/

EXPORT  void    FT_CurvatureAtPoint(
        FT_POINT   *p,
	Front	*fr,
        double   *curvature)
{
	HYPER_SURF_ELEMENT      *hse = p->hse;
        HYPER_SURF              *hs = p->hs;
	int dim = fr->rect_grid->dim;

	if (dim == 1) return;
	if (hs == NULL || hse == NULL)
	{
	    int i;
	    screen("In FT_CurvatureAtPoint(): point: ");
	    for (i = 0; i < dim; ++i)
		printf("%f ",Coords(p)[i]);
	    printf("\n");
	    screen("ERROR: Point does not have hs or hse, debug!\n");
	    clean_up(ERROR);
	}
	*curvature = mean_curvature_at_point(p,hse,hs,fr);
}	/* end FT_CurvatureAtPoint */

/*	
	Get front normal direction at a point. If comp = NO_COMP (-1)
	the normal direction will point to the positive side of the
	hyper surface the point is on. Otherwise it will point to
	the side of the given comp
*/

EXPORT	void	FT_NormalAtPoint(
	FT_POINT   *p,
        Front   *fr,
        double   *nor,
	COMPONENT comp)
{
	HYPER_SURF_ELEMENT      *hse = p->hse;
        HYPER_SURF              *hs = p->hs;
	int i,dim = fr->rect_grid->dim;
        if (dim > 1 && (hs == NULL || hse == NULL))
        {
	    screen("In FT_NormalAtPoint(): point: ");
	    for (i = 0; i < dim; ++i)
		printf("%f ",Coords(p)[i]);
	    printf("\n");
            screen("ERROR: Point does not have hs or hse, debug!\n");
            clean_up(ERROR);
        }
	normal(p,hse,hs,nor,fr);
	if (comp == negative_component(hs))
	{
	    int i,dim = fr->rect_grid->dim;
	    for (i = 0; i < dim; ++i)
		nor[i] *= -1.0;
	}
}	/* end FT_NormalAtPoint */

/*
	Get the grid step size in the direction given.
*/

EXPORT 	double	FT_GridSizeInDir(
	double	*dir,
	Front 	*front)
{
	int dim = front->rect_grid->dim;
	double *h = front->rect_grid->h;

	return grid_size_in_direction(dir,h,dim);
}	/* end FT_GridSizeInDir */	

/*
	Get coordinates of nrad points in the normal direction of the
	point p to the side of comp.
*/

EXPORT	Nor_stencil *FT_CreateNormalStencil(
	Front	*fr,
	FT_POINT	*p,
	COMPONENT comp,
	int 	npts)
{
	static Nor_stencil *sten;
	static int current_npts = 0;
	int i,j;
	int dim = fr->rect_grid->dim;
	double dn;
	
	if (sten == NULL)
	{
	    scalar(&sten,sizeof(Nor_stencil));
	    current_npts = npts;
	    bi_array(&sten->pts,npts,MAXD,FLOAT);
	}
	if (current_npts < npts)
	{
	    free_these(1,sten->pts);
	    bi_array(&sten->pts,npts,MAXD,FLOAT);
	    current_npts = npts;
	}
	sten->comp = comp;
	sten->npts = npts;
	FT_NormalAtPoint(p,fr,sten->nor,comp);
	FT_CurvatureAtPoint(p,fr,&sten->curvature);
	dn = FT_GridSizeInDir(sten->nor,fr);
	for (i = 0; i < npts; ++i)
	{
	    for (j = 0; j < dim; ++j)
	    {
		sten->pts[i][j] = Coords(p)[j] + i*dn*sten->nor[j];	
	    }
	}
	return sten;
}	/* end FT_CreateNormalStencil */


/**************************************************************************
*	This function returns a chain of points centered at point pc.     *
*	It returns NO if the curve does not have enough number of points  *
*	If the curve is not a closed curve, the chain of points is biased *
*	when the number of point on either side of pc is less than np/2   *
**************************************************************************/

EXPORT	boolean FrontGetPointChain(
	FT_POINT *pc,		/* center of the chain */
	FT_POINT **pts,		/* memory already allocated */
	int num_pts)
{
	CURVE *c = Curve_of_hs(pc->hs);
	BOND *b = Bond_of_hse(pc->hse);
	BOND *b_current,*btmp;
	int i,iprev,inext;

	if (c->num_points < num_pts) return NO;

	inext = 0;
	b_current = (pc == b->end) ? Next_bond(b,c) : b;
	while (b_current) 
	{
	    inext++;
	    if (inext == num_pts/2) break;
	    b_current = Next_bond(b_current,c);
	}

	b_current = (pc == b->end) ? b : Prev_bond(b,c);
	iprev = 0;
	while(b_current)	
	{
	    iprev++;
	    if (iprev == num_pts-inext-1) break;
	    btmp = Prev_bond(b_current,c);
	    if (btmp == NULL) break;
	    b_current = btmp;
	}
	if (iprev == 0) b_current = b;
	for (i = 0; i < num_pts && b_current != NULL; 
		++i, b_current = Next_bond(b_current,c))
	{
	    pts[i] = b_current->start;
	    if (i < num_pts-1) pts[i+1] = b_current->end;
	}

	return YES;
}	/* end FrontGetPointChain */


EXPORT void FrontPreAdvance(
	Front *front)
{
	switch (front->rect_grid->dim)
	{
	case 2:
	    FrontPreAdvance2d(front);
	    return;
	case 3:
	    FrontPreAdvance3d(front);
	    return;
	}
}	/* end FrontPreAdvance */

LOCAL void FrontPreAdvance2d(
	Front *front)
{
	INTERFACE *intfc = front->interf;
	CURVE **c;
	int i,j,index,max_body_index = -1;
	int dim = front->rect_grid->dim;
	double *torque,**force;
	double t,f[MAXD];
	double dt = front->dt;

	if (debugging("rigid_body"))
	    (void) printf("Entering FrontPreAdvance()\n");
	for (c = intfc->curves; c && *c; ++c)
	{
	    if (wave_type(*c) == MOVABLE_BODY_BOUNDARY)
		if (body_index(*c) > max_body_index)
		    max_body_index = body_index(*c);
	}
	max_body_index++;
	pp_global_imax(&max_body_index,1);
	if (max_body_index == 0) return;

	uni_array(&torque,max_body_index,FLOAT);
	bi_array(&force,max_body_index,MAXD,FLOAT);
	for (i = 0; i < max_body_index; ++i)
	{
	    torque[i] = 0.0;
	    for (j = 0; j < dim; ++j)
	    	force[i][j] = 0.0;
	}

	if (front->_compute_force_and_torque == NULL)
	{
	    screen("In FrontPreAdvance2d():\n");
	    screen("front->_compute_force_and_torque() not assigned\n");
	    clean_up(ERROR);
	}

	for (c = intfc->curves; c && *c; ++c)
	{
	    if (wave_type(*c) == MOVABLE_BODY_BOUNDARY)
	    {
		if (motion_type(*c) == PRESET_MOTION) continue;
		index = body_index(*c);
		FrontForceAndTorqueOnHs(front,Hyper_surf(*c),dt,f,&t);
		torque[index] += t;
	    	for (j = 0; j < dim; ++j)
	    	    force[index][j] += f[j];
	    }
	}
	for (i = 0; i < max_body_index; ++i)
	{
	    pp_global_sum(force[i],dim);
	    pp_global_sum(&torque[i],1);
	}
	for (c = intfc->curves; c && *c; ++c)
	{
	    if (wave_type(*c) == MOVABLE_BODY_BOUNDARY)
	    {
		index = body_index(*c);
		if (motion_type(*c) == PRESET_MOTION) 
		{
		    if (debugging("rigid_body"))
		    {
		    	printf("Body index: %d\n",index);
		    	printf("Preset motion\n");
		    	printf("angular_velo = %f\n",angular_velo(*c));
		    	printf("center_of_mass_velo = %f  %f\n",
					center_of_mass_velo(*c)[0],
					center_of_mass_velo(*c)[1]);
		    }
		    continue;
		}
		else if (motion_type(*c) == VERTICAL_MOTION)
		{
		    for (i = 0; i < dim-1; ++i)
		    	center_of_mass_velo(*c)[i] = 0.0;
		    for (i = dim-1; i < dim; ++i)
		    	center_of_mass_velo(*c)[i] +=
                        	dt*force[index][i]/total_mass(*c);
		    angular_velo(*c) = 0.0;
		}
		else if (motion_type(*c) == HORIZONTAL_MOTION)
		{
		    for (i = 0; i < dim-1; ++i)
		    	center_of_mass_velo(*c)[i] +=
                        	dt*force[index][i]/total_mass(*c);
		    for (i = dim-1; i < dim; ++i)
		    	center_of_mass_velo(*c)[i] = 0.0;
		    angular_velo(*c) = 0.0;
		}
		else if (motion_type(*c) == ROTATION) 
		{
		    for (i = 0; i < dim; ++i)
		    	center_of_mass_velo(*c)[i] = 0.0;
		    angular_velo(*c) += dt*torque[index]/mom_inertial(*c);
		}
		else
		{
		    for (i = 0; i < dim; ++i)
		    {
		    	center_of_mass_velo(*c)[i] +=
                        	dt*force[index][i]/total_mass(*c);
		    }
		    angular_velo(*c) += dt*torque[index]/mom_inertial(*c);
		}
		for (i = 0; i < dim; ++i)
                    center_of_mass(*c)[i] += dt*center_of_mass_velo(*c)[i];
		if (debugging("rigid_body"))
		{
		    printf("Body index: %d\n",index);
		    printf("total mass = %f\n",total_mass(*c));
		    printf("moment of inertial = %f\n",mom_inertial(*c));
		    printf("torque = %f\n",torque[index]);
		    printf("force = %f %f\n",force[index][0],force[index][1]);
		    printf("angular_velo = %f\n",angular_velo(*c));
		    printf("center_of_mass = %f  %f\n",
			center_of_mass(*c)[0],center_of_mass(*c)[1]);
		    printf("center_of_mass_velo = %f  %f\n",
			center_of_mass_velo(*c)[0],center_of_mass_velo(*c)[1]);
		}
	    }
	}
	free_these(2,force,torque);
	if (debugging("rigid_body"))
	    (void) printf("Leaving FrontPreAdvance()\n");
}	/* end FrontPreAdvance2d */

LOCAL void FrontPreAdvance3d(
	Front *front)
{
	INTERFACE *intfc = front->interf;
	SURFACE **s;
	int i,j,index,max_body_index = -1;
	int dim = front->rect_grid->dim;
	double **torque,**force,torq_dir;
	double t[MAXD],f[MAXD];
	double dt = front->dt;

	if (debugging("rigid_body"))
	    (void) printf("Entering FrontPreAdvance()\n");
	for (s = intfc->surfaces; s && *s; ++s)
	{
	    if (wave_type(*s) == MOVABLE_BODY_BOUNDARY)
		if (body_index(*s) > max_body_index)
		    max_body_index = body_index(*s);
	}
	max_body_index++;
	pp_global_imax(&max_body_index,1);
	if (max_body_index == 0) return;

	bi_array(&torque,max_body_index,MAXD,FLOAT);
	bi_array(&force,max_body_index,MAXD,FLOAT);
	for (i = 0; i < max_body_index; ++i)
	{
	    for (j = 0; j < dim; ++j)
	    {
	    	torque[i][j] = 0.0;
	    	force[i][j] = 0.0;
	    }
	}

	for (s = intfc->surfaces; s && *s; ++s)
	{
	    if (wave_type(*s) == MOVABLE_BODY_BOUNDARY)
	    {
		if (motion_type(*s) == PRESET_MOTION) continue;
		index = body_index(*s);
		FrontForceAndTorqueOnHs(front,Hyper_surf(*s),dt,f,t);
	    	for (j = 0; j < dim; ++j)
		{
	    	    force[index][j] += f[j];
		    torque[index][j] += t[j];
		}
	    }
	}
	for (i = 0; i < max_body_index; ++i)
	{
	    pp_global_sum(force[i],dim);
	    pp_global_sum(torque[i],dim);
	}
	for (s = intfc->surfaces; s && *s; ++s)
	{
	    if (wave_type(*s) == MOVABLE_BODY_BOUNDARY)
	    {
		index = body_index(*s);
		if (motion_type(*s) == PRESET_MOTION) 
		{
		    if (debugging("rigid_body"))
		    {
		    	printf("Body index: %d\n",index);
		    	printf("Preset motion\n");
		    	printf("angular_velo = %f\n",angular_velo(*s));
		    	printf("center_of_mass_velo = %f  %f  %f\n",
					center_of_mass_velo(*s)[0],
					center_of_mass_velo(*s)[1],
					center_of_mass_velo(*s)[2]);
		    }
		    continue;
		}
		torq_dir = Dot3d(torque[index],rotation_direction(*s));
		angular_velo(*s) += dt*torq_dir/mom_inertial(*s);
		if (motion_type(*s) != ROTATION)
		{
		    for (i = 0; i < dim; ++i)
		    {
		    	center_of_mass_velo(*s)[i] +=
                        	dt*force[index][i]/total_mass(*s);
		    }
		}
		else 
		{
		    for (i = 0; i < dim; ++i)
		    	center_of_mass_velo(*s)[i] = 0.0;
		}
		for (i = 0; i < dim; ++i)
                    center_of_mass(*s)[i] += dt*center_of_mass_velo(*s)[i];
		if (debugging("rigid_body"))
		{
		    printf("Body index: %d\n",index);
		    printf("torque = %f %f %f\n",torque[index][0],
					torque[index][1],torque[index][2]);
		    printf("force = %f %f %f\n",force[index][0],
					force[index][1],force[index][2]);
		    printf("angular_velo = %f\n",angular_velo(*s));
		    printf("center_of_mass = %f  %f  %f\n",
					center_of_mass(*s)[0],
					center_of_mass(*s)[1],
					center_of_mass(*s)[2]);
		    printf("center_of_mass_velo = %f  %f  %f\n",
					center_of_mass_velo(*s)[0],
					center_of_mass_velo(*s)[1],
					center_of_mass_velo(*s)[2]);
		}
	    }
	}
	free_these(2,force,torque);
	if (debugging("rigid_body"))
	    (void) printf("Leaving FrontPreAdvance()\n");
}	/* end FrontPreAdvance3d */

EXPORT	boolean FrontReflectPointViaNeumannBdry(
	double		*coords,
	double		*coordsref,
	double		*nor,
	COMPONENT	int_comp,
	HYPER_SURF	*Nbdry,
	Front		*front)
{
	HYPER_SURF	   *hsbdry;
	HYPER_SURF_ELEMENT *hsebdry;
	double		   dcrds[MAXD],coordsbdry[MAXD];
	double		   ns[MAXD], ne[MAXD];
	double		   t[MAXD];
	int		   i, dim = front->rect_grid->dim;

	if (wave_type(Nbdry) != NEUMANN_BOUNDARY)
	    return NO;

	if (dim != 1)
	{
	    if (Nbdry->interface != front->interf)
	    {
	    	Nbdry = find_correspond_hyper_surface(Nbdry,NULL,NULL,
	    			                      front,front->interf);
	    	if (Nbdry == NULL)
		    return NO;
	    }
	    if (!Nbdry || Nbdry->interface != front->interf)
		return NO;
	    if (!nearest_interface_point(coords,int_comp,front->interf,
			                INCLUDE_BOUNDARIES,Nbdry,coordsbdry,t,
					&hsebdry,&hsbdry))
		return NO;
	}

	switch (dim)
	{
	case 1:
	    coordsbdry[0] = Coords(Point_of_hs(Nbdry))[0];
	    nor[0] = (coords[0] <
			0.5*(front->rect_grid->L[0]+front->rect_grid->U[0])) ?
			1.0 : -1.0;
	    break;
	case 2:
	    normal(Bond_of_hse(hsebdry)->start,hsebdry,hsbdry,ns,front);
	    normal(Bond_of_hse(hsebdry)->end,hsebdry,hsbdry,ne,front);
	    for (i = 0; i < dim; ++i)
	    	nor[i] = (1.0 - t[0])*ns[i] + t[0]*ne[i];
	    break;
	case 3:
	{
	    const double *tnor = Tri_normal(Tri_of_hse(hsebdry));
	    for (i = 0; i < dim; ++i)
	    	nor[i] = tnor[i];
	}
	    break;
	}

	for (i = 0; i < dim; ++i)
	{
	    if (int_comp == negative_component(hsbdry))
		nor[i] = -nor[i];
	    dcrds[i] = coordsbdry[i] - coords[i];
	    coordsref[i] = coords[i] + 2.0*(dcrds[i] - dcrds[i]*fabs(nor[i]));
	}
	return YES;
}		/* end FrontReflectPointVisHs */

EXPORT	boolean FT_FindNearestIntfcPointInRange(
	Front *front,
	COMPONENT comp,
	double *coords,
	double *p,
	double *t,
	HYPER_SURF_ELEMENT **phse,
	HYPER_SURF         **phs,
	int range)
{
	return nearest_interface_point_within_range(coords,comp,front->interf,
				NO_BOUNDARIES,NULL,p,t,phse,phs,range);
}	/* FrontGetNearestPoint */

EXPORT void FT_ResetTime(Front *front)
{
	front->time = 0.0;
        front->dt = 0.0;
        front->step = 0;
	front->im = front->ip = 0;
        front->is_print_time = NO;
        front->is_movie_time = NO;
        front->time_limit_reached = NO;
}	/* end FT_ResetTime */

EXPORT void FT_SetOutputCounter(Front *front)
{
	double time = front->time + MACH_EPS;
	front->ip = (int)(time/front->print_time_interval + 1.0);
	front->im = (int)(time/front->movie_frame_interval + 1.0);
	Redistribution_count(front) = front->step;
}	/* FT_SetOutputCounter */

EXPORT boolean FT_IsSaveTime(Front *front)
{
	if (front->is_print_time || front->time_limit_reached) 
	    return YES;
	else
	    return NO;
}	/* end FT_IsSaveTime */

EXPORT boolean FT_IsMovieFrameTime(Front *front)
{
	if (front->is_movie_time || front->time_limit_reached) 
	    return YES;
	else
	    return NO;
}	/* end FT_IsMovieFrameTime */

EXPORT boolean FT_TimeLimitReached(Front *front)
{
	return front->time_limit_reached;
}	/* end FT_IsSaveTime */

EXPORT	void FT_TimeControlFilter(Front *front)
{
	double time = front->time;
	double dt = front->dt;
	double new_dt;
	double dt1,dt2,dt3;

	front->is_movie_time = NO;
	front->is_print_time = NO;
	front->time_limit_reached = NO;

	dt1 = (front->im)*front->movie_frame_interval - time;
	dt2 = (front->ip)*front->print_time_interval - time;
	dt3 = front->max_time - time;
	new_dt = min3(dt1,dt2,dt3);

	if (front->step+1 >= front->max_step)
            front->time_limit_reached = YES;

	if (new_dt > dt) 
	    return;

	if (fabs(dt1 - new_dt) < 1e-15)
	{
	    front->is_movie_time = YES;
	    (front->im)++;
	}
        if (fabs(dt2 - new_dt) < 1e-15)
        {
            front->is_print_time = YES;
	    (front->ip)++;
        }
        if (fabs(dt3 - new_dt) < 1e-15)
        {
            front->time_limit_reached = YES;
        }
	front->dt = new_dt;
}	/* end FrontOutputTimeControl */

EXPORT void FT_AddTimeStepToCounter(Front *front)
{
	++(front->step);
	front->time += front->dt;
	//front->dt = 0.0;
}	/* end FT_AddTimeStepToCounter */

EXPORT	void FT_SetTimeStep(
	Front *front)
{
	double fcrds[MAXD];
	double max_dt;
	double CFL = Time_step_factor(front);

	/* f_max_front_time_step */
	max_dt = CFL*(*front->max_front_time_step)(front,fcrds);
#if defined(__MPI__)
	pp_global_min(&max_dt,1);
#endif /* defined(__MPI__) */
	if (front->dt != 0 && max_dt > front->dt * 2) {
		front->dt *= 2;
	} else {	
		front->dt = max_dt;
	}
}	/* end FT_SetTimeStep */

EXPORT	void FT_InitDebug(char *inname)
{
	FILE *infile = fopen(inname,"r");
	char string[100];

	if (!fgetstring(infile,"Enter yes for debugging:"))
	{
	    unset_debug();
	    return;
	}
	fscanf(infile,"%s",string);
	(void) printf("Enter yes for debugging: %s\n",string);
	if (string[0] == 'n' || string[0] == 'N')
	{
	    unset_debug();
	    return;
	}
	while (fgetstring(infile,"Enter the debugging string:"))
	{
	    zero_scalar(string,100*sizeof(char));	
	    fscanf(infile,"%s",string);
	    add_to_debug(string);
	    (void) printf("Enter the debugging string: %s\n",string);
	}
	fclose(infile);
	return;
}	/* end FT_InitDebug */

EXPORT void FT_RecordMaxFrontSpeed(
	int dir,
	double speed,
	POINTER state,
	double *coords,
	Front *front)
{
	set_max_front_speed(dir,speed,state,coords,front);
}	/* end FT_RecordMaxFrontSpeed */

EXPORT void FT_ParallelExchCellIndex(
	Front *front,
	int *lbuf,
	int *ubuf,
	POINTER ijk_to_I)
{
	scatter_cell_index(front,lbuf,ubuf,ijk_to_I);
}	/* end FT_ParallelExchCellIndex */

EXPORT	void FT_GetStatesAtPoint(
	FT_POINT *p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF *hs,
	POINTER *sl,
	POINTER *sr)
{
	slsr(p,hse,hs,sl,sr);
}	/* end FT_GetStatesAtPoint */

EXPORT	void FT_ScalarMemoryAlloc(
	POINTER *a,
	int size)
{
	(void) array_T("scalar",a,1,1,size);
	zero_scalar(*a,size);
}	/* FT_ScalarMemoryAlloc */

EXPORT	void FT_VectorMemoryAlloc(
	POINTER *a,
	int n1,
	int size)
{
	(void) array_T("vector",a,1,n1,size);
}	/* FT_VectorMemoryAlloc */

EXPORT	void FT_MatrixMemoryAlloc(
	POINTER *a,
	int n1,
	int n2,
	int size)
{
	(void) array_T("matrix",a,2,n1,n2,size);
}	/* FT_MatrixMemoryAlloc */

EXPORT	void FT_TriArrayMemoryAlloc(
	POINTER *a,
	int n1,
	int n2,
	int n3,
	int size)
{
	(void) array_T("tri_array",a,3,n1,n2,n3,size);
}	/* FT_TriArrayMemoryAlloc */

EXPORT	void FT_QuadArrayMemoryAlloc(
	POINTER *a,
	int n1,
	int n2,
	int n3,
	int n4,
	int size)
{
	(void) array_T("quad_array",a,4,n1,n2,n3,n4,size);
}	/* FT_QuadArrayMemoryAlloc */

EXPORT	void FT_QuinArrayMemoryAlloc(
	POINTER *a,
	int n1,
	int n2,
	int n3,
	int n4,
	int n5,
	int size)
{
	(void) array_T("quin_array",a,5,n1,n2,n3,n4,n5,size);
}	/* FT_QuinArrayMemoryAlloc */

EXPORT	void FT_SexArrayMemoryAlloc(
	POINTER *a,
	int n1,
	int n2,
	int n3,
	int n4,
	int n5,
	int n6,
	int size)
{
	(void) array_T("sex_array",a,6,n1,n2,n3,n4,n5,n6,size);
}	/* FT_SexArrayMemoryAlloc */

LOCAL   boolean new_vtx_is_closer(
        double dist_old,
        double dist_new,
        double *dp_old,
        double *dp_new,
        int dim)
{
        int i;
        if (dist_new < dist_old) return YES;
        if (dist_new > dist_old) return NO;
        for (i = 0; i < dim; ++i)
            if (dp_new[i] < dp_old[i]) return YES;
        return NO;
}       /* end vtx_is_closer */

EXPORT	POINTER *FT_CreateLevelHyperSurfs(
	RECT_GRID *rgr,
	INTERFACE *intfc,
	COMPONENT neg_comp,
	COMPONENT pos_comp,
	double    (*func)(POINTER,double*),
	POINTER   func_params,
	int	  w_type,
	int       *num_hs)
{
	static CURVE	**curves;
	static SURFACE *surf;
	int i,dim = rgr->dim;

	switch(dim)
	{
	case 2:
	    curves = make_level_curves(rgr,intfc,neg_comp,pos_comp,func,
				func_params,NO,num_hs);
	    for (i = 0; i < *num_hs; ++i)
	    {
		wave_type(curves[i]) = w_type;	
		if (curves[i]->start == curves[i]->end)
                    node_type(curves[i]->start) = CLOSED_NODE;
	    }
	    return (POINTER*)curves;
	case 3:
	    if (make_level_surface(rgr,intfc,neg_comp,pos_comp,func,
				func_params,&surf))
	    {
		wave_type(surf) = w_type;
		*num_hs = 1;
		return (POINTER*)&surf;
	    }
	    else
	    {
		*num_hs = 0;
		return NULL;
	    }
	}
}	/* end FT_CreateLevelHyperSurfs */

static boolean is_rect_side_node(RECT_GRID*,NODE*,int,int);
static boolean is_rect_side_curve(RECT_GRID*,CURVE*,int,int);

EXPORT void FT_PromptSetMixedTypeBoundary2d(
	char *in_name,
        Front *front)
{
	FILE *infile;
	INTERFACE *intfc = front->interf;
	RECT_GRID *rgr = front->rect_grid;
	NODE **n,*nodes[21],*ntmp;
	CURVE **c,*curves[20];
	int n_nodes,n_curves;
	int i,j,dim,idir,nb;
	char input_string[100],s[100];

	dim = intfc->dim;
	if (dim != 2) return;	/* 3D not implemented */
	infile = fopen(in_name,"r");

	for (idir = 0; idir < dim; ++idir)
	for (nb = 0; nb < 2; ++nb)
	{
	    n_nodes = n_curves = 0;
	    if (rect_boundary_type(intfc,idir,nb) != MIXED_TYPE_BOUNDARY)
	    	continue;
	    for (n = intfc->nodes; n && *n; ++n)
	    {
	    	if (is_rect_side_node(rgr,*n,idir,nb))
		   nodes[n_nodes++] = *n; 
	    }
	    for (c = intfc->curves; c && *c; ++c)
	    {
	    	if (is_rect_side_curve(rgr,*c,idir,nb))
		   curves[n_curves++] = *c; 
	    }
	    for (i = 0; i < n_nodes-1; ++i)
	    for (j = i+1; j < n_nodes; ++j)
	    	if (Coords(nodes[i]->posn)[(idir+1)%dim] > 
		    Coords(nodes[j]->posn)[(idir+1)%dim])
		{
		    ntmp = nodes[i];
		    nodes[i] = nodes[j];
		    nodes[j] = ntmp;
		}
	    (void) printf("Direction %d side %d is MIXED_TYPE_BOUNDARY\n",
	    			idir,nb);
	    (void) printf("Total number of nodes %d\n",n_nodes);
	    (void) printf("Nodes coordinates are:\n");
	    for (i = 0; i < n_nodes; ++i)
	    {
	    	(void) printf("%f %f\n",Coords(nodes[i]->posn)[0],
					Coords(nodes[i]->posn)[1]);
	    }
	    for (i = 0; i < n_nodes; ++i)
	    {
		sprintf(input_string,"Enter type of node %d:",i+1);
            	CursorAfterString(infile,input_string);
		fscanf(infile,"%s",s);
		(void) printf("%s\n",s);
		switch (s[0])
		{
		case 'F':
		case 'f':
		    node_type(nodes[i]) = FIXED_NODE;
		    break;
		case 'D':
		case 'd':
		    node_type(nodes[i]) = DIRICHLET_NODE;
		    break;
		case 'N':
		case 'n':
		    node_type(nodes[i]) = NEUMANN_NODE;
		    break;
		default:
		    (void) printf("In FT_PromptSetMixedTypeBoundary2d():\n");
		    (void) printf("Node type not implemented()");
		    clean_up(ERROR);
		}
	    }
	    for (i = 0; i < n_nodes-1; ++i)
	    {
		CURVE *curve = NULL;
	    	for (j = 0; j < n_curves; ++j)
		{
		    if ((curves[j]->start == nodes[i] && 
		         curves[j]->end == nodes[i+1]) ||
		    	(curves[j]->start == nodes[i+1] && 
		         curves[j]->end == nodes[i])) 
		    {
			 curve = curves[j];
			 break;
		    }
		}
		sprintf(input_string,
		    	"Enter wave type of the curve between nodes %d%d:",
		    	i+1,i+2);
		if (curve == NULL)
		{
		    printf("curve not found!\n");
		    clean_up(ERROR);
		}
            	CursorAfterString(infile,input_string);
		fscanf(infile,"%s",s);
		(void) printf("%s\n",s);
		switch (s[0])
		{
		case 'D':
		case 'd':
		    wave_type(curve) = DIRICHLET_BOUNDARY;
		    break;
		case 'N':
		case 'n':
		    wave_type(curve) = NEUMANN_BOUNDARY;
		    break;
		case 'P':
		case 'p':
		    wave_type(curve) = PASSIVE_BOUNDARY;
		    break;
		default:
		    (void) printf("In FT_PromptSetMixedTypeBoundary2d():\n");
		    (void) printf("curve wave type not implemented()");
		    clean_up(ERROR);
		}
	    }
	}
}	/* end FT_PromptSetMixedTypeBoundary2d */

static boolean is_rect_side_node(
	RECT_GRID *rgr,
	NODE *n,
	int idir,
	int nb)
{
	double *L = rgr->L;
	double *U = rgr->U;
	const double eps = 10.0*MACH_EPS;
	if (!is_bdry(n)) return NO;
	if (nb == 0)
	{
	    if (fabs(Coords(n->posn)[idir] - L[idir]) > eps)
	    	return NO;
	}
	else if (nb == 1)
	{
	    if (fabs(Coords(n->posn)[idir] - U[idir]) > eps)
	    	return NO;
	}
	else
	    return NO;
	return YES;
}	/* end is_rect_side_node */

static boolean is_rect_side_curve(
	RECT_GRID *rgr,
	CURVE *c,
	int idir,
	int nb)
{
	double bline = (nb == 0) ? rgr->L[idir] : rgr->U[idir];

	if (!is_bdry(c)) return NO;
	if (fabs(Coords(c->start->posn)[idir] - bline) > grid_tolerance(rgr))
	    return NO;
	if (fabs(Coords(c->end->posn)[idir] - bline) > grid_tolerance(rgr))
	    return NO;
	return YES;
}	/* end is_rect_side_curve */

static boolean is_rect_side_surface(
	RECT_GRID *rgr,
	SURFACE *s,
	int idir,
	int nb)
{
	TRI *tri;
	FT_POINT *p;
	double bline = (nb == 0) ? rgr->L[idir] : rgr->U[idir];
	int i;

	if (!is_bdry(s)) return NO;
	for (tri = first_tri(s); !at_end_of_tri_list(tri,s); tri = tri->next)
	for (i = 0; i < 3; ++i)
	{
	    p = Point_of_tri(tri)[i];
	    if (fabs(Coords(p)[idir] - bline) > grid_tolerance(rgr))
	    	return NO;
	}
	return YES;
}	/* end is_rect_side_surface */

EXPORT HYPER_SURF **FT_MixedBoundaryHypSurfs(
	INTERFACE *intfc,
	int idir,
	int nb,
	int w_type,
	int *num_hs)
{
	RECT_GRID *rgr = computational_grid(intfc);
	CURVE **c;
	SURFACE **s;
	static HYPER_SURF **hyp_surfs;
	int i,j,dim = rgr->dim;

	*num_hs = 0;
	if (rect_boundary_type(intfc,idir,nb) != MIXED_TYPE_BOUNDARY)
	    return NULL;
	switch (dim)
	{
	case 2:
	    for (c = intfc->curves; c && *c; ++c)
	    {
		if (!is_rect_side_curve(rgr,*c,idir,nb)) continue;
		else if (wave_type(*c) != w_type) continue;
		else (*num_hs)++;
	    }
	    uni_array(&hyp_surfs,*num_hs,sizeof(HYPER_SURF*));
	    *num_hs = 0;
	    for (c = intfc->curves; c && *c; ++c)
	    {
		if (!is_rect_side_curve(rgr,*c,idir,nb)) continue;
		else if (wave_type(*c) != w_type) continue;
		else 
		{
		    hyp_surfs[*num_hs] = Hyper_surf(*c);
		    (*num_hs)++;
		}
	    }
	    break;
	case 3:
	    for (s = intfc->surfaces; s && *s; ++s)
	    {
		if (!is_rect_side_surface(rgr,*s,idir,nb)) continue;
		else if (wave_type(*s) != w_type) continue;
		else (*num_hs)++;
	    }
	    uni_array(&hyp_surfs,*num_hs,sizeof(HYPER_SURF*));
	    *num_hs = 0;
	    for (s = intfc->surfaces; s && *s; ++s)
	    {
		if (!is_rect_side_surface(rgr,*s,idir,nb)) continue;
		else if (wave_type(*s) != w_type) continue;
		else 
		{
		    hyp_surfs[*num_hs] = Hyper_surf(*s);
		    (*num_hs)++;
		}
	    }
	}
	return hyp_surfs;
}	/* end FT_MixedBoundaryHypSurfs */

#include <stdarg.h>
#define   SHOW_ALLOC \
        switch (vmalloc_debug_on)    \
	{                            \
	case 4:                      \
	    alloc_view(stdout);      \
	    break;                   \
	case 5:                      \
	    long_alloc_view(stdout); \
	    break;                   \
	}


EXPORT void FT_FreeThese(
	int	n,
	...)
{
	va_list	ap;
	int	i;

	if (vmalloc_debug_on > 1)
	{
	    va_start(ap, n);
	    (void) printf("Request to free_these: ");
	    for(i = 0; i < n; ++i)
	    	(void) printf("%p ",va_arg(ap,POINTER));
	    (void) printf("\n");
	    va_end(ap);
	}

	va_start(ap, n);
	for (i = 0; i < n; ++i)
	    f_ree(va_arg(ap,POINTER),"free_these");
	va_end(ap);

	SHOW_ALLOC;
	return;
}		/*end FT_FreeThese*/

EXPORT	boolean FT_ReflectPointThroughBdry(
	Front		*front,
	HYPER_SURF	*hs,
	double		*coords,
	COMPONENT	int_comp,
	double		*coordsbdry,
	double		*coordsref,
	double		*nor)
{
	HYPER_SURF	   *hsbdry;
	HYPER_SURF_ELEMENT *hsebdry;
	double		   ns[MAXD], ne[MAXD];
	double		   t[MAXD];
	double		   v[MAXD],vn;
	int		   i, dim = front->rect_grid->dim;

	if (wave_type(hs) != NEUMANN_BOUNDARY &&
	    wave_type(hs) != MOVABLE_BODY_BOUNDARY &&
	    wave_type(hs) != GROWING_BODY_BOUNDARY &&
	    wave_type(hs) != ELASTIC_BOUNDARY)
	    return NO;

	if (dim != 1)
	{
	    if (!hs || !hs->interface)
		return NO;
	    if (!nearest_interface_point(coords,int_comp,hs->interface,
			                INCLUDE_BOUNDARIES,hs,coordsbdry,t,
					&hsebdry,&hsbdry))
		return NO;
	}

	switch (dim)
	{
	case 1:
	    coordsbdry[0] = Coords(Point_of_hs(hs))[0];
	    nor[0] = (coords[0] <
			0.5*(front->rect_grid->L[0]+front->rect_grid->U[0])) ?
			1.0 : -1.0;
	    break;
	case 2:
	    normal(Bond_of_hse(hsebdry)->start,hsebdry,hsbdry,ns,front);
	    normal(Bond_of_hse(hsebdry)->end,hsebdry,hsbdry,ne,front);
	    for (i = 0; i < dim; ++i)
	    	nor[i] = (1.0 - t[0])*ns[i] + t[0]*ne[i];
	    break;
	case 3:
	{
	    const double *tnor = Tri_normal(Tri_of_hse(hsebdry));
	    for (i = 0; i < dim; ++i)
	    	nor[i] = tnor[i];
	}
	    break;
	}
	if (int_comp == negative_component(hsbdry))
	{
	    for (i = 0; i < dim; ++i)
		nor[i] *= -1.0;
	}
	vn = 0.0;
	for (i = 0; i < dim; ++i)
	{
	    v[i] = coords[i] - coordsbdry[i];
	    vn += nor[i]*v[i];
	}
	for (i = 0; i < dim; ++i)
	{
	    v[i] = 2.0*vn*nor[i] - v[i];
	}

	for (i = 0; i < dim; ++i)
	    coordsref[i] = v[i] + coordsbdry[i];
	return YES;
}		/*end FT_ReflectPointThroughBdry*/

EXPORT	void FT_ClipIntfcToSubdomain(Front *front)
{
	clip_front_to_subdomain(front);
	return;
}	/* end FT_ClipIntfcToSubdomain */

EXPORT void FT_ArrayOfIntfcCurves(
	INTERFACE *intfc,
	CURVE **curves)
{
	CURVE **c;
        int n = 0;
        for (c = intfc->curves; c && *c; ++c)
            curves[n++] = *c;
}	/* end FT_ArrayOfIntfcCurves */

EXPORT int FT_NumOfSurfCurves(
	SURFACE *surf)
{
	int n = 0;
	CURVE **c;
	for (c = surf->pos_curves; c && *c; ++c)
	    ++n;
	for (c = surf->neg_curves; c && *c; ++c)
	    ++n;
	return n;
}	/* end FT_NumOfSurfCurves */

EXPORT void FT_ArrayOfSurfCurves(
	SURFACE *surf,
	CURVE **curves)
{
	int n = 0;
	CURVE **c;
	for (c = surf->pos_curves; c && *c; ++c)
	    curves[n++] = *c;
	for (c = surf->neg_curves; c && *c; ++c)
	    curves[n++] = *c;
}	/* end FT_NumOfSurfCurves */

EXPORT int FT_NumOfNodeCurves(
	NODE *node)
{
	int n = 0;
	CURVE **c;
	for (c = node->in_curves; c && *c; ++c)
	    ++n;
	for (c = node->out_curves; c && *c; ++c)
	    ++n;
	return n;
}	/* end FT_NumOfNodeCurves */

EXPORT void FT_ArrayOfNodeCurves(
	NODE *node,
	CURVE **curves)
{
	int n = 0;
	CURVE **c;
	for (c = node->in_curves; c && *c; ++c)
	    curves[n++] = *c;
	for (c = node->out_curves; c && *c; ++c)
	    curves[n++] = *c;
}	/* end FT_ArrayOfNodeCurves */

EXPORT void FT_ArrayOfCurvePoints(
	CURVE *curve,
	FT_POINT **pts)
{
	int n,dim = curve->interface->dim;
	BOND *b;

	n = 0;
	pts[n] = curve->first->start;
	if (dim == 2)
	{
	    pts[n]->hse = Hyper_surf_element(curve->first);
	    pts[n]->hs = Hyper_surf(curve);
	}
	n++;
	for (b = curve->first; b != NULL; b = b->next)
	{
	    pts[n] = b->end;
	    if (dim == 2)
	    {
	    	pts[n]->hse = Hyper_surf_element(b);
	    	pts[n]->hs = Hyper_surf(curve);
	    }
	    n++;
	}
}	/* end FT_ArrayOfCurvePoints */

EXPORT void FT_ArrayOfCurveBonds(
	CURVE *curve,
	BOND **bonds)
{
	BOND *b;
	int n = 0;

	for (b = curve->first; b != NULL; b = b->next)
	{
	    bonds[n++] = b;
	}
}	/* end FT_ArrayOfCurveBonds */

/*	Expensive function, use with caution 
*/

EXPORT CURVE *FT_CurveOfPoint(
	INTERFACE *intfc,
	FT_POINT *point)
{
	CURVE **c,*curve;
	BOND *b;
	for (c = intfc->curves; c && *c; c++)
	{
	    curve = *c;
	    if (curve->first->start == point) 
		return curve;
	    for (b = curve->first; b != NULL; b = b->next)
		if (b->end == point) 
		    return curve;
	}
	return NULL;
}	/* end FT_CurveOfPoint */


EXPORT NODE *FT_NodeOfPoint(
	INTERFACE *intfc,
	FT_POINT *point)
{
	NODE **n,*node;
	for (n = intfc->nodes; n && *n; n++)
	{
	    node = *n;
	    if (node->posn == point)
		return node;
	}
	return NULL;
}	/* end FT_NodeOfPoint */

EXPORT void FT_ArrayOfSurfPoints(
	SURFACE *surf,
	FT_POINT **pts)
{
	FT_POINT *p;
	TRI *tri;
	int i,n;

	/* reset point index */
	for(tri = first_tri(surf); !at_end_of_tri_list(tri,surf); 
			tri = tri->next)
	{
	    for(i = 0; i < 3; i++)
	    {
	        p = Point_of_tri(tri)[i];
		Index_of_point(p) = -1;		
	    }
	}		
	
	/* points	 */
	n = 0;
	for(tri = first_tri(surf); !at_end_of_tri_list(tri,surf); 
			tri = tri->next)
	{
	    for(i = 0; i < 3; i++)
	    {
	        p = Point_of_tri(tri)[i];
		if (Index_of_point(p) == -1)
		{
		    pts[n++] = p;
		    Index_of_point(p) = n;		
		}
	    }
	}		
}	/* end FT_ArrayOfSurfPoints */

/***********************************************************************
 *	FT_NumOfCurvePoints()
 * return the number of points in the given CURVE *c.
 ***********************************************************************/ 
EXPORT 	int FT_NumOfCurvePoints(CURVE *c)
{
	return c->num_points;
}	/* end FT_NumOfCurvePoints */

/***********************************************************************
 *	FT_NumOfSurfPoints(SURFACE *s)
 ***********************************************************************/
EXPORT	int FT_NumOfSurfPoints(SURFACE *surf)
{
	FT_POINT *p;
	TRI *tri;
	int i,n;

	/* reset point index */
	for(tri = first_tri(surf); !at_end_of_tri_list(tri,surf); 
			tri = tri->next)
	{
	    for(i = 0; i < 3; i++)
	    {
	        p = Point_of_tri(tri)[i];
		Index_of_point(p) = -1;		
	    }
	}		
	
	/* points	 */
	n = 0;
	for(tri = first_tri(surf); !at_end_of_tri_list(tri,surf); 
			tri = tri->next)
	{
	    for(i = 0; i < 3; i++)
	    {
	        p = Point_of_tri(tri)[i];
		if (Index_of_point(p) == -1)
		{
		    Index_of_point(p) = n++;		
		}
	    }
	}		
	
	return n;
}	/* end FT_NumOfSurfPoints */

/***********************************************************************
 *	FT_NumOfIntfcPoints()
 * return the number of points in the given INTERFACE *intfc.
 ***********************************************************************/
EXPORT  int FT_NumOfIntfcPoints(INTERFACE *intfc)
{
	int i, n, dim = intfc->dim;
	FT_POINT	*p;
	HYPER_SURF_ELEMENT *phse;
	HYPER_SURF	   *phs;
	
	n=0;
        next_point(intfc,NULL, NULL,NULL);
	for ((void)next_point(intfc, &p, &phse, &phs); p!= NULL; 
	     (void)next_point(intfc, &p, &phse, &phs))
	{
	    /*Index_of_point(p) = n/dim; */
            ++n;
	}
        return n;
}	/* end FT_NumOfIntfcPoints */


/**************************************************************************
	return intfc->num_points;        
}       /* end FT_NumOfIntfcPoints */

/***********************************************************************
 *	FT_NumOfIntfcNodes()
 * return the number of the nodes in the given INTERFACE *intfc;
 ***********************************************************************/
EXPORT	int FT_NumOfIntfcNodes(INTERFACE *intfc)
{
	NODE **n;
        int num_nodes = 0;
        for (n = intfc->nodes, num_nodes = 0; n && *n; ++n)
            ++num_nodes;
	return num_nodes;
}	/* end NumOfIntfcNodes */

/***********************************************************************
 *	FT_NumOfCurveBonds()
 * return the number of bonds in the given CURVE *c.
 ***********************************************************************/
EXPORT int FT_NumOfCurveBonds(CURVE *c)
{
	BOND *b;
	int num_bonds = 0;
	for(b = c->first; b != NULL; b = b->next)
	    num_bonds++;
	return num_bonds;
}	/* end NumOfCurveBonds */

/***********************************************************************
 *	FT_NumOfIntfcBonds()
 * return the number of bonds in the given INTERFACE *intfc.
 ***********************************************************************/
EXPORT int FT_NumOfIntfcBonds(INTERFACE *intfc)
{
	CURVE **c;
	int num_bonds = 0;
	for (c = intfc->curves; c && *c; ++c)
            num_bonds += FT_NumOfCurveBonds(*c);
        return num_bonds;
}	/* end FT_NumOfIntfcBonds */

/***********************************************************************
 *	FT_NumOfCurves()
 * return the number of curves in the given INTERFACE *intfc.
 ***********************************************************************/
EXPORT  int FT_NumOfIntfcCurves(INTERFACE *intfc)
{
        CURVE **c;
        int num_curves;
        for (c = intfc->curves, num_curves = 0; c && *c; ++c)
            ++num_curves;
        return num_curves;
}       /* end FT_NumOfIntfcCurves */

/***********************************************************************
 *	FT_NumOfSurfaces()
 * return the number of surfaces in the given INTERFACE *intfc.
 ***********************************************************************/
EXPORT  int FT_NumOfIntfcSurfaces(INTERFACE *intfc)
{
        SURFACE **s;
        int num_surfs = 0;
        for (s = intfc->surfaces; s && *s; ++s)
            ++num_surfs;
        return num_surfs;
}       /* end FT_NumOfIntfcSurfaces */

/**********************************************************************
 *	FT_NumOfSurfTris()
 * return the number of triangles in the given SURFACE *s.
 **********************************************************************/
EXPORT	int FT_NumOfSurfTris(SURFACE *s)
{
	return s->num_tri;
}	/* end FT_NumOfSurfTris */

/**********************************************************************
 *	FT_NumOfIntfcTris()
 * return the number of triangles in the given INTERFACE *intfc.
 **********************************************************************/
EXPORT	int FT_NumOfIntfcTris(INTERFACE *intfc)
{
	SURFACE **s;
        int num_tris = 0;
        for (s = intfc->surfaces; s && *s; ++s)
            num_tris += FT_NumOfSurfTris(*s);
        return num_tris;
}	/* end FT_NumOfIntfcTris */

EXPORT int FT_FirstRingTrisAroundPoint(
	FT_POINT *p,
	TRI *tri,
	TRI ***tris)	/* tris in first ring around the point p */
{
	int nt;
	nt = set_tri_list_around_point(p,tri,tris,p->interface);
	return nt;
}	/* end FT_FirstRingTrisAroundPoint */


/*TMP*/
static	boolean equi_bond_curve_redist(
	Front		*fr,
	CURVE		*c)
{
	BOND		*b;
	double		fr_space;
	RECT_GRID	*rgr = fr->rect_grid;
	double		*h = rgr->h;
	double		c_len;
	int		dim = rgr->dim;
	int		nbds;

	fr_space = Front_spacing(fr,GENERAL_WAVE);

	    /* Compute curve_length */

	c_len = 0.0;
	for (b = c->first;  b;  b = b->next)
	{
	    c_len += scaled_bond_length(b,h,dim);
	}

	    /* For short, non-boundary curves with few points - each */
	    /* such curve is equi-distributed, however the number   */
	    /* of points currently on the curve remains unchanged   */

	nbds = c->num_points - 1;

	    /* Pass 1, delete very short bonds */

	curve_delete_very_short_bonds(c);

	    /* Can't redistribute single bond, short_curves */

	if ((c->first == c->last) && (c_len < fr_space))
	{
	    return YES;
	}

	b = c->first;
        while (b)
        {
            BOND *be,*bs = b;
            c_len = 0.0;
            for (;  b;  b = b->next)
            {
                c_len += scaled_bond_length(b,h,dim);
                be = b;
		if (cross_rect_grid_bdry(b,rgr)) break;
            }
	    b = be->next;
            equi_redist_curve_seg(c,bs,be,-1,c_len,fr_space,rgr);
        }
	return YES;
}		/*end equi_bond_curve_redist*/
