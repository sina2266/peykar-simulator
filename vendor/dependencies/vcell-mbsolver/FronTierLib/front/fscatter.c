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
*				fscatter.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*/



#define DEBUG_STRING    "fscatter"
#include <front/fdecs.h>    	/* includes int.h, table.h */

		/* LOCAL Function Declarations */

LOCAL 	void  	bundle_array_buffer(int,int*,int*,int*,double*,byte*);
LOCAL 	void  	unbundle_array_buffer(int,int*,int*,int*,double*,byte*);
LOCAL 	int 	set_send_buffer_limits(int,int*,int*,int,int,int*,int*,int*);
LOCAL 	int 	set_recv_buffer_limits(int,int*,int*,int,int,int*,int*,int*);
LOCAL   void    reflect_array_buffer(int,int,int,int*,int*,int*,double*);

/*
*			scatter_front():
*	
*	This routine does three things:
*	1. cut large intfc according to the size of physical
*	   subdomain;
*	2. extend subdomain intfc and rect_grid up to buffer region;
*	3. fix boundary curves onto extended subdomain ----
*	   virtual domain.
*
*/


EXPORT boolean scatter_front(
	Front		*front)
{
	COMPONENT	max_comp;
	INTERFACE	*intfc = front->interf;
	RECT_GRID	*gr = front->rect_grid;
	boolean		status;
	boolean		sav_copy = copy_intfc_states();
	boolean		sav_intrp = interpolate_intfc_states(intfc);
	int		i, dim = gr->dim;
	DEBUG_ENTER(scatter_front)

	if ((dim == 3) && debugging("consistency"))
	{
	    (void) printf("Check consistency of interface "
	                  "before scatter_front()\n");
	    if (!consistent_interface(intfc))
	    {
		screen("ERROR in scatter_front(),  input interface is "
		       "inconsistent\n");
		clean_up(ERROR);
	    }
	    (void) printf("Interface into scatter_front is consistent\n");
	}
	if (dim == 2)
	{
	    for (i = 0; i < dim; ++i)
		if ((gr->lbuf[i] > 0) || (gr->ubuf[i] > 0))
		     break;
	    if (i == dim)
	    {
		DEBUG_LEAVE(scatter_front)
		status = FUNCTION_SUCCEEDED; /* No subdomains to process */
		return pp_min_status(status);
	    }
	}

	set_copy_intfc_states(YES);
	interpolate_intfc_states(intfc) = NO;

	status = form_subintfc_via_communication(front);
	
	if(dim == 3)
	    delete_outside_surface(front->interf);
	
	max_comp = max_component(intfc);
	pp_global_imax(&max_comp,1L);
	max_component(intfc) = max_comp;
	intfc->normal_unset = YES;

	interpolate_intfc_states(intfc) = sav_intrp;
	set_copy_intfc_states(sav_copy);

	if ((status) && (dim == 3) && debugging("consistency"))
	{
	    (void) printf("Check consistency of interface ");
	    (void) printf("after scatter_front()\n");
	    if (!consistent_interface(intfc))
	    {
		screen("ERROR in scatter_front(),  output interface is "
		       "inconsistent\n");
		clean_up(ERROR);
	    }
	}

	if (!status)
	    printf("In scatter_front() status = FAILED mynode = %d\n",
				pp_mynode());
	DEBUG_LEAVE(scatter_front)
	return pp_min_status(status);
}		/*end scatter_front*/


EXPORT	INTERFACE *f_zoom_interface(
	INTERFACE	*intfc,
	RECT_GRID	*gr,
	double		*L,	/* Lower coord of clip box */
	double		*U,	/* upper coord of clip box */
	double		**Q)	/* Rotation matrix */
{
	INTERFACE	*zoom_intfc;

	DEBUG_ENTER(f_zoom_interface)
	zoom_intfc = i_zoom_interface(intfc,gr,L,U,Q);
	copy_rect_grid(computational_grid(intfc),gr);
	rotate_and_zoom_rect_grid(computational_grid(zoom_intfc),L,U,Q);
	DEBUG_LEAVE(f_zoom_interface)
	return zoom_intfc;
}		/*end f_zoom_interface*/

EXPORT	void	delete_subdomain_boundaries(
	INTERFACE	*intfc)
{

	DEBUG_ENTER(delete_subdomain_boundaries)
	switch (intfc->dim)
	{
	case 1:
	    {
	        FT_POINT **p;
	        for (p = intfc->points; p && *p; ++p)
	        {
		    if (is_subdomain_boundary(Hyper_surf(*p)))
	            {
		        delete_point(*p);
		        if (intfc->points == NULL)
		            break;
		        p = intfc->points-1;
	            }
	        }
	    }
	    break;
	case 2:
	    delete_subdomain_curves(intfc);
	    break;
	case 3:/*TODO implement delete_subdomain_boundaries*/
	    break;
	}
	DEBUG_LEAVE(delete_subdomain_boundaries)
}		/*end delete_subdomain_boundaries*/


EXPORT	void clip_front_to_subdomain(
	Front 		*front)
{
	INTERFACE *intfc = front->interf;
	PP_GRID	  *pp_grid = front->pp_grid;
	int	   i, dim = intfc->dim;
	int	   icoords[MAXD];
	int	   myid = pp_mynode();
	int        rbt[3][2];

	debug_print("init","Entered clip_front_to_subdomain()\n");
	find_Cartesian_coordinates(myid,pp_grid,icoords);
	for (i = 0; i < dim; ++i)
	{
	    rbt[i][0] = rect_boundary_type(intfc,i,0);
	    rbt[i][1] = rect_boundary_type(intfc,i,1);
	    if (icoords[i] > 0)
	        rect_boundary_type(intfc,i,0) = SUBDOMAIN_BOUNDARY;
	    if (icoords[i] < (pp_grid->gmax[i]-1))
	        rect_boundary_type(intfc,i,1) = SUBDOMAIN_BOUNDARY;
	}

	if (dim == 3)
	{
	    RECT_GRID *t_gr = &topological_grid(intfc);
	    double     *h = front->rect_grid->h;
	    int       *buf = pp_grid->buf;

	    /* Reset topological grid GL and GU */
	    for (i = 0; i < dim; ++i)
	    {
	        if (buffered_boundary_type(rbt[i][0]))
		    t_gr->GL[i] -= buf[i]*h[i];
	        if (buffered_boundary_type(rbt[i][1]))
		    t_gr->GU[i] += buf[i]*h[i];
	    }
	    
	    delete_outside_surface(intfc);
	    debug_print("init","Left clip_front_to_subdomain()\n");
	    /*return; */
	}

	if (debugging("init"))
	{
	    (void) printf("Interface before scatter_front()\n");
	    print_interface(front->interf);
	}
	pp_clip_rect_grids(front,rbt);
	if (!scatter_front(front))
	{
	    screen("ERROR in clip_front_to_subdomain(), "
	           "scatter_front failed\n");
	    print_interface(front->interf);
	    clean_up(ERROR);
	}
	if (debugging("init"))
	{
	    (void) printf("Interface after scatter_front()\n");
	    print_interface(front->interf);
	}
	return;
}		/*end clip_front_to_subdomain*/

EXPORT	void	pp_clip_rect_grids(
	Front *front,
	int   rbt[3][2])
{
	INTERFACE	*intfc = front->interf;
	PP_GRID		*pp_grid = front->pp_grid;
	RECT_GRID	*zoom_gr = &pp_grid->Zoom_grid;
	RECT_GRID	 *t_gr = &topological_grid(intfc);
	RECT_GRID	 *c_gr = computational_grid(intfc), Dual_grid;
	int		dim = front->rect_grid->dim;
	int		i, tgmax[MAXD];

	for (i = 0; i < dim; ++i)
	{
	    if (buffered_boundary_type(rect_boundary_type(intfc,i,0)))
	    	zoom_gr->lbuf[i] = pp_grid->buf[i];
	    else
		zoom_gr->lbuf[i] = 0;
	    if (buffered_boundary_type(rect_boundary_type(intfc,i,1)))
	    	zoom_gr->ubuf[i] = pp_grid->buf[i];
	    else
		zoom_gr->ubuf[i] = 0;
	}
	set_rect_grid(zoom_gr->L,zoom_gr->U,zoom_gr->GL,zoom_gr->GU,
		      zoom_gr->lbuf,zoom_gr->ubuf,zoom_gr->gmax,dim,
		      &c_gr->Remap,zoom_gr);

	for (i = 0; i < dim; ++i)
	{
	    if (zoom_gr->gmax[i] <= zoom_gr->lbuf[i] ||
		zoom_gr->gmax[i] <= zoom_gr->ubuf[i])
	    {
	    	screen("ERROR in pp_clip_rect_grids(), "
	    	       "grid should have more interior mesh zones "
	    	       "than buffer mesh zones.\n"
	    	       "dir = %d, gmax = %d, lbuf = %d, ubuf = %d\n",
	    	       i,zoom_gr->gmax[i],zoom_gr->lbuf[i],zoom_gr->ubuf[i]);
	    	clean_up(ERROR);
	    }
	}

	for (i = 0; i < dim; ++i)
	{
	    double h = t_gr->h[i];
	    tgmax[i] = irint((zoom_gr->VU[i] - zoom_gr->VL[i])/h);
	    t_gr->GL[i] = zoom_gr->GL[i];
	    if (buffered_boundary_type(rbt[i][0]))
		t_gr->GL[i] -= pp_grid->buf[i]*zoom_gr->h[i];
	    t_gr->GU[i] = zoom_gr->GU[i];
	    if (buffered_boundary_type(rbt[i][1]))
		t_gr->GU[i] += pp_grid->buf[i]*zoom_gr->h[i];
	}
	
	/*Assume the grid is square in 3d, and top grid is the dual of comp grid. */
	if(dim == 3)
	{
	    intfc->table->new_grid = YES;
	    copy_rect_grid(c_gr,zoom_gr);
	    copy_rect_grid(front->rect_grid,zoom_gr);
	    set_dual_grid(&Dual_grid,c_gr);
	    set_expanded_grid(&Dual_grid,t_gr);
	    /*(void) adjust_top_grid_for_square(t_gr,zoom_gr);*/
	}
	else
	{
	    set_rect_grid(zoom_gr->VL,zoom_gr->VU,t_gr->GL,t_gr->GU,
		      NOBUF,NOBUF,tgmax,dim,&t_gr->Remap,t_gr);
	    intfc->table->new_grid = YES;
	    /*(void) adjust_top_grid_for_square(t_gr,zoom_gr);*/
	    copy_rect_grid(c_gr,zoom_gr);
	    copy_rect_grid(front->rect_grid,zoom_gr);
	    intfc->modified = YES;
	}
}		/*end pp_clip_rect_grids*/


EXPORT	void set_front_pp_grid(
	INIT_DATA	*init,
	Front		*front)
{
	RECT_GRID	*comp_glbgr = front->rect_grid;
	double		L[MAXD], U[MAXD], *GL, *GU;
	double		*h = comp_glbgr->h;
	int		lbuf[MAXD], ubuf[MAXD];
	int		gmax[MAXD];
	int		icoords[MAXD];
	int		i, dim = comp_glbgr->dim;
	int		myid = pp_mynode();
	static PP_GRID	Pp_grid;
	PP_GRID		*pp_grid = &Pp_grid;

	debug_print("init_pp_grid","Entered set_front_pp_grid():\n");

	front->pp_grid = pp_grid;
	copy_rect_grid(&pp_grid->Global_grid,comp_glbgr);

	pp_grid->nn = 1;
	for (i = 0; i < dim; ++i)
	{
	    int	Gmax, Pmax, k;
	    int	basic_slices, extra_slices;

	    pp_grid->buf[i] = buffer_zones(init)[i];
	    Pmax = pp_grid->gmax[i] = subdomains(init)[i];
	    pp_grid->nn *= Pmax;

	    uni_array(&pp_grid->dom[i],Pmax + 1,FLOAT);

	    pp_grid->dom[i][0]    = comp_glbgr->L[i];
	    pp_grid->dom[i][Pmax] = comp_glbgr->U[i];
	    Gmax = comp_glbgr->gmax[i];

	    basic_slices = Gmax / Pmax;
	    extra_slices = Gmax % Pmax;

	    for (k = 1; k < Pmax; ++k)
	    {
	    	if (k < extra_slices)
	            pp_grid->dom[i][k] = k*(basic_slices + 1)*h[i]
	        			 + pp_grid->dom[i][0];
	        else
	            pp_grid->dom[i][k] = (k*basic_slices + extra_slices)*h[i]
	        			 + pp_grid->dom[i][0];
	    }
	}

	/* Clip rectangular grid to subdomain */

	GL = pp_grid->Global_grid.L;    GU = pp_grid->Global_grid.U;
	find_Cartesian_coordinates(myid,pp_grid,icoords);
	for (i = 0; i < dim; ++i)
	{
	    L[i] = pp_grid->dom[i][icoords[i]];
	    U[i] = pp_grid->dom[i][icoords[i] + 1];
	    gmax[i] = irint((U[i] - L[i])/h[i]);
	    switch (dim) /* TODO Unify 2 and 3 D */
	    {
	    case 1:
	    case 2:
	    	lbuf[i] = (icoords[i] > 0) ? pp_grid->buf[i] : 0;
	    	ubuf[i] =  (icoords[i]<(pp_grid->gmax[i]-1))?pp_grid->buf[i]:0;
	    	break;
	    case 3:
	    	lbuf[i] = pp_grid->buf[i];
	    	ubuf[i] = pp_grid->buf[i];
	    	break;
	    }
	}
	set_rect_grid(L,U,GL,GU,lbuf,ubuf,gmax,dim,&comp_glbgr->Remap,
		      &pp_grid->Zoom_grid);

	if (debugging("set_pp_grid"))
	{
	    print_PP_GRID_structure(pp_grid);
	}

	debug_print("init_pp_grid","Left set_front_pp_grid():\n");
}		/*end set_front_pp_grid*/


EXPORT void scatter_top_grid_float_array(
	double *solute,
	Front *front)
{
	INTERFACE *intfc = front->grid_intfc;
	PP_GRID	*pp_grid = front->pp_grid;
	int dim = Dimension(intfc);
	static byte *storage;
	int i,j,k,dir,side,len;
	int bmin[3],bmax[3];
	int myid,dst_id,*G;
	int me[3],him[3];
	RECT_GRID *comp_grid,*top_grid;
	int lbuf[MAXD],ubuf[MAXD],*gmax;
	static int max_buf = 0;
	static int storage_size;
	static int min_gmax;

	start_clock("scatter_top_grid_float_array");
	myid = pp_mynode();
	G = pp_grid->gmax;
	find_Cartesian_coordinates(myid,pp_grid,me);

	comp_grid = computational_grid(intfc);
	top_grid = &topological_grid(intfc);
	gmax = top_grid->gmax;
	for (i = 0; i < dim; ++i)
	{
	    lbuf[i] = comp_grid->lbuf[i];
	    ubuf[i] = comp_grid->ubuf[i];
	}

	if (storage == NULL)
	{
	    min_gmax = gmax[0];
	    for (i = 0; i < dim; i++)
	    {
	    	if (lbuf[i] > max_buf)
		    max_buf = lbuf[i];
	    	if (ubuf[i] > max_buf)
		    max_buf = ubuf[i];
		if (min_gmax > gmax[i])
		    min_gmax = gmax[i];
	    }
	    storage_size = max_buf*FLOAT;
	    for (i = 0; i < dim; i++)
	    	storage_size *= (gmax[i] + 1);
	    storage_size /= (min_gmax + 1);
	    uni_array(&storage,storage_size,sizeof(byte));
	}

	for (dir = 0; dir < dim; ++dir)
	{
	    for (side = 0; side < 2; ++side)
	    {
		for (k = 0; k < dim; ++k)
		    him[k] = me[k];
	    	if (rect_boundary_type(intfc,dir,side) == SUBDOMAIN_BOUNDARY)
		{
		    him[dir] = (me[dir] + 2*side - 1 + G[dir])%G[dir];
		    dst_id = domain_id(him,G,dim);
	    	    len = set_send_buffer_limits(dim,bmin,bmax,dir,side,lbuf,
		    			ubuf,gmax);
		    bundle_array_buffer(dim,bmin,bmax,gmax,solute,
		    			storage);
		    if (dst_id != myid)
		    	pp_send(array_id(0),storage,len,dst_id);
		}
                else if (rect_boundary_type(intfc,dir,side) ==
                                REFLECTION_BOUNDARY)
                {
                    reflect_array_buffer(dim,dir,side,gmax,lbuf,ubuf,solute);
                }
		if (rect_boundary_type(intfc,dir,(side+1)%2) ==
					SUBDOMAIN_BOUNDARY)
		{
		    him[dir] = (me[dir] - 2*side + 1 + G[dir])%G[dir];
		    dst_id = domain_id(him,G,dim);
	    	    len = set_recv_buffer_limits(dim,bmin,bmax,dir,(side+1)%2,
		    			lbuf,ubuf,gmax);
		    if (dst_id != myid)
		    	pp_recv(array_id(0),dst_id,storage,len);
		    unbundle_array_buffer(dim,bmin,bmax,gmax,solute,
		    			storage);
		}
	    }
	}
	stop_clock("scatter_top_grid_float_array");
}	/* end scatter_dual_grid_array */

LOCAL	void bundle_array_buffer(
	int dim,
	int *bmin,
	int *bmax,
	int *gmax,
	double *array,
	byte *storage)
{
	byte *variable = storage;
	int i,j,k,ic;
	switch (dim)
	{
	case 2:
	    for (i = bmin[0]; i < bmax[0]; ++i)
	    for (j = bmin[1]; j < bmax[1]; ++j)
	    {
	    	ic = d_index2d(i,j,gmax);
		ft_assign(variable,(POINTER)&array[ic],FLOAT);
		variable += FLOAT;
	    }
	    break;
	case 3:
	    for (i = bmin[0]; i < bmax[0]; ++i)
	    for (j = bmin[1]; j < bmax[1]; ++j)
	    for (k = bmin[2]; k < bmax[2]; ++k)
	    {
	    	ic = d_index3d(i,j,k,gmax);
		ft_assign(variable,array+ic,FLOAT);
		variable += FLOAT;
	    }
	}
}	/* end bundle_array_buffer */

LOCAL	void unbundle_array_buffer(
	int dim,
	int *bmin,
	int *bmax,
	int *gmax,
	double *array,
	byte *storage)
{
	byte *variable = storage;
	int i,j,k,ic;
	switch (dim)
	{
	case 2:
	    for (i = bmin[0]; i < bmax[0]; ++i)
	    for (j = bmin[1]; j < bmax[1]; ++j)
	    {
	    	ic = d_index2d(i,j,gmax);
		ft_assign(array+ic,variable,FLOAT);
		variable += FLOAT;
	    }
	    break;
	case 3:
	    for (i = bmin[0]; i < bmax[0]; ++i)
	    for (j = bmin[1]; j < bmax[1]; ++j)
	    for (k = bmin[2]; k < bmax[2]; ++k)
	    {
	    	ic = d_index3d(i,j,k,gmax);
		ft_assign(array+ic,variable,FLOAT);
		variable += FLOAT;
	    }
	}
}	/* end unbundle_array_buffer */


LOCAL	int set_send_buffer_limits(
	int dim,
	int *bmin,
	int *bmax,
	int dir,
	int side,
	int *lbuf,
	int *ubuf,
	int *gmax)
{
	int i,len;
	for (i = 0; i < dim; ++i)
	{
	    bmin[i] = 0;
	    bmax[i] = gmax[i] + 1;
	}
	if (side == 0)
	{
	    bmin[dir] = lbuf[dir];
	    bmax[dir] = 2*lbuf[dir];
	}
	else
	{
	    bmin[dir] = gmax[dir] - 2*ubuf[dir] + 1;
	    bmax[dir] = gmax[dir] - ubuf[dir] + 1;
	}
	len = 1;
	for (i = 0; i < dim; ++i)
	    len *= (bmax[i] - bmin[i]);
	return len*FLOAT;
}	/* end set_send_buffer_limits */


LOCAL	int set_recv_buffer_limits(
	int dim,
	int *bmin,
	int *bmax,
	int dir,
	int side,
	int *lbuf,
	int *ubuf,
	int *gmax)
{
	int i,len;
	for (i = 0; i < dim; ++i)
	{
	    bmin[i] = 0;
	    bmax[i] = gmax[i] + 1;
	}
	if (side == 0)
	{
	    bmin[dir] = 0;
	    bmax[dir] = lbuf[dir];
	}
	else
	{
	    bmin[dir] = gmax[dir] - ubuf[dir] + 1;
	    bmax[dir] = gmax[dir] + 1;
	}
	len = 1;
	for (i = 0; i < dim; ++i)
	    len *= (bmax[i] - bmin[i]);
	return len*FLOAT;
}	/* end set_recv_buffer_limits */


/*	
 *	This function adjusts the boundary according to the relative
 *	CPU balance between mynode and neighbors. The lower boundary 
 *	is expanded if mynode CPU is 5 percent smaller than the lower
 *	neighbor, shrinked if it is 5 percent larger; the same for
 *	the upper side of the boundary. The input cpu_time is the total
 *	CPU time consumed by mynode in the past time step.
 *
 */

EXPORT	boolean cpu_adapt_front(
	Front *front,
	double cpu_time,
	int *lexpand,
	int *uexpand)
{
	int 		i,j,k,l,id;
	static 	double	*sub_cpu;
	int		*G;
	int		sub_icoords[MAXD];
	int		dim = front->rect_grid->dim;
	PP_GRID		*pp_grid = front->pp_grid;
	double		lambda;
	boolean		adaptation = NO;
	RECT_GRID	*gr = front->rect_grid;
	int 		status;
	int 		icoords[MAXD];
	double 		*coords;
	int  		*iperm;	
	Locstate	s_old,s_new;
	static 		double *x_average,*y_average,*z_average;
	int         	ix, iy, iz;
	int         	xmax, ymax, zmax;
	char		debug_name[100];
	static	FILE	*debug_file;


	if (pp_numnodes() == 1) return NO;

	if (debugging("adapt_subdomain") && !debugging("re_adapt_subdomain"))
	{
            (void) printf("Entering adapt_subdomain()\n");
	    sprintf(debug_name,"adapt_step-%d",pp_mynode());
	    if (debug_file == NULL)
	    	debug_file = fopen(debug_name,"w");
	    fprintf(debug_file,"Step = %d\n",front->step);
	}

	if (sub_cpu == NULL)
	    uni_array(&sub_cpu,pp_numnodes(),FLOAT);

	G = front->pp_grid->gmax;

	for (i = 0; i < pp_numnodes(); ++i)
	    sub_cpu[i] = -HUGE_VAL;

	sub_cpu[pp_mynode()] = cpu_time;
	pp_global_max(sub_cpu,pp_numnodes());

	for (i = 0; i < dim; ++i)
		lexpand[i] = uexpand[i] = 0;
	switch (dim)
	{
	case 2:
	    if (x_average == NULL)
	    {
	    	uni_array(&x_average,G[0],FLOAT);
	    	uni_array(&y_average,G[1],FLOAT);
	    }
	    for (i = 0; i < G[0]; ++i)
	    {
	    	sub_icoords[0] = i;
		x_average[i] = 0.0;
	    	for (j = 0; j < G[1]; ++j)
	    	{
		    sub_icoords[1] = j;
		    id = domain_id(sub_icoords,G,2);
		    x_average[i] += sub_cpu[id];
	    	}
	    }
	    if (G[0] > 1)
	    {
	    	find_Cartesian_coordinates(pp_mynode(),pp_grid,sub_icoords);
                if (debugging("adapt_subdomain"))
                {
                    (void) printf("G = %d %d\n",G[0],G[1]);
                    (void) printf("sub_icoords = %d %d\n",sub_icoords[0],
                                        sub_icoords[1]);
                }
		k = sub_icoords[0];
		if (k > 0)
		{
		    lambda = x_average[k]/x_average[k-1];
		    if (lambda < 1./1.05) lexpand[0] = 1;
		    else if (lambda > 1.05) lexpand[0] = -1;
		    else lexpand[0] = 0;
		    if (lexpand[0] != 0) adaptation = YES;
                    if (debugging("adapt_subdomain"))
                    {
                        (void) printf("Comparing x-average with lower side:\n");                        (void) printf("x_ave[%d] = %10.6f x_ave[%d] = %10.6f\n",                                k,x_average[k],k-1,x_average[k-1]);
                        (void) printf("lambda = %f\n",lambda);
                        (void) printf("lexpand[0] = %d\n",lexpand[0]);
                    }
		}
		if (k < G[0]-1)
		{
		    lambda = x_average[k+1]/x_average[k];
		    if (lambda > 1.05) uexpand[0] = 1;
		    else if (lambda < 1./1.05) uexpand[0] = -1;
		    else uexpand[0] = 0;
		    if (uexpand[0] != 0) adaptation = YES;
                    if (debugging("adapt_subdomain"))
                    {
                        (void) printf("Comparing x-average with upper side:\n");                        (void) printf("x_ave[%d] = %10.6f x_ave[%d] = %10.6f\n",                                k,x_average[k],k+1,x_average[k+1]);
                        (void) printf("lambda = %f\n",lambda);
                        (void) printf("uexpand[0] = %d\n",uexpand[0]);
                    }
		}
	    }
	    for (i = 0; i < G[1]; ++i)
	    {
	    	sub_icoords[1] = i;
		y_average[i] = 0.0;
	    	for (j = 0; j < G[0]; ++j)
	    	{
		    sub_icoords[0] = j;
		    id = domain_id(sub_icoords,G,2);
		    y_average[i] += sub_cpu[id];
	    	}
	    }
	    if (G[1] > 1)
	    {
	    	find_Cartesian_coordinates(pp_mynode(),pp_grid,sub_icoords);
		k = sub_icoords[1];
		if (k > 0)
		{
		    lambda = y_average[k]/y_average[k-1];
		    if (lambda < 1./1.05) lexpand[1] = 1;
		    else if (lambda > 1.05) lexpand[1] = -1;
		    else lexpand[1] = 0;
		    if (lexpand[1] != 0) adaptation = YES;
                    if (debugging("adapt_subdomain"))
                    {
                        (void) printf("Comparing y-average with lower side:\n");                        (void) printf("y_ave[%d] = %10.6f y_ave[%d] = %10.6f\n",                                k,y_average[k],k-1,y_average[k-1]);
                        (void) printf("lambda = %f\n",lambda);
                        (void) printf("lexpand[1] = %d\n",lexpand[1]);
                    }
		}
		if (k < G[1]-1)
		{
		    lambda = y_average[k+1]/y_average[k];
		    if (lambda > 1.05) uexpand[1] = 1;
		    else if (lambda < 1./1.05) uexpand[1] = -1;
		    else uexpand[1] = 0;
		    if (uexpand[1] != 0) adaptation = YES;
                    if (debugging("adapt_subdomain"))
                    {
                        (void) printf("Comparing y-average with upper side:\n");                        (void) printf("y_ave[%d] = %10.6f y_ave[%d] = %10.6f\n",                                k,y_average[k],k+1,y_average[k+1]);
                        (void) printf("lambda = %f\n",lambda);
                        (void) printf("uexpand[1] = %d\n",uexpand[1]);
                    }
		}
	    }
	    break;
	case 3:
	    if (x_average == NULL)
            {
                uni_array(&x_average,G[0],FLOAT);
                uni_array(&y_average,G[1],FLOAT);
             	uni_array(&z_average,G[2],FLOAT);
	    }
	    for (i = 0; i < G[0]; ++i)
	    {
	    	sub_icoords[0] = i;
		x_average[i] = 0.0;
	    	for (j = 0; j < G[1]; ++j)
	    	{
		    sub_icoords[1] = j;
		    for (k = 0; k < G[2]; ++k)
		    {
		    	sub_icoords[2] = k;
			id = domain_id(sub_icoords,G,3);
			x_average[i] += sub_cpu[id];
		    }
		}
	    }
	    if (G[0] > 1)
	    {
		find_Cartesian_coordinates(pp_mynode(),pp_grid,sub_icoords);
                if (debugging("adapt_subdomain"))
                {
                    (void) printf("G = %d %d %d\n",G[0],G[1],G[2]);
                    (void) printf("sub_icoords = %d %d %d\n",sub_icoords[0],
                                        sub_icoords[1],sub_icoords[2]);
                }
                l = sub_icoords[0];
                if (l > 0)
                {
                    lambda = x_average[l]/x_average[l-1];
                    if (lambda < 1./1.05) lexpand[0] = 1;
                    else if (lambda > 1.05) lexpand[0] = -1;
                    else lexpand[0] = 0;
                    if (lexpand[0] != 0) adaptation = YES;
                    if (debugging("adapt_subdomain"))
                    {
                        (void) printf("Comparing x-average with lower side:\n");                        (void) printf("x_ave[%d] = %10.6f x_ave[%d] = %10.6f\n",                                k,x_average[l],l-1,x_average[l-1]);
                        (void) printf("lambda = %f\n",lambda);
                        (void) printf("lexpand[0] = %d\n",lexpand[0]);
                    }
                }
                if (l < G[0]-1)
                {
                    lambda = x_average[l+1]/x_average[l];
                    if (lambda > 1.05) uexpand[0] = 1;
                    else if (lambda < 1./1.05) uexpand[0] = -1;
                    else uexpand[0] = 0;
                    if (uexpand[0] != 0) adaptation = YES;
                    if (debugging("adapt_subdomain"))
                    {
                        (void) printf("Comparing x-average with upper side:\n");                        (void) printf("x_ave[%d] = %10.6f x_ave[%d] = %10.6f\n",                                k,x_average[l],l+1,x_average[l+1]);
                        (void) printf("lambda = %f\n",lambda);
                        (void) printf("uexpand[0] = %d\n",uexpand[0]);
                    }
                }
            }
	    for (i = 0; i < G[1]; ++i)
            {
                sub_icoords[1] = i;
                y_average[i] = 0.0;
                for (j = 0; j < G[0]; ++j)
                {
                    sub_icoords[0] = j;
		    for (k = 0; k < G[2]; ++k)
		    {
                        sub_icoords[2] = k;
			id = domain_id(sub_icoords,G,3);
                        y_average[i] += sub_cpu[id];
		    }
                }
            }
            if (G[1] > 1)
            {
                find_Cartesian_coordinates(pp_mynode(),pp_grid,sub_icoords);
                if (debugging("adapt_subdomain"))
                {
                    (void) printf("G = %d %d %d\n",G[0],G[1],G[2]);
                    (void) printf("sub_icoords = %d %d %d\n",sub_icoords[0],
                                        sub_icoords[1],sub_icoords[2]);
                }
                l = sub_icoords[1];
                if (l > 0)
                {
                    lambda = y_average[l]/y_average[l-1];
                    if (lambda < 1./1.05) lexpand[1] = 1;
                    else if (lambda > 1.05) lexpand[1] = -1;
                    else lexpand[1] = 0;
                    if (lexpand[1] != 0) adaptation = YES;
                    if (debugging("adapt_subdomain"))
                    {
                        (void) printf("Comparing y-average with lower side:\n");                        (void) printf("y_ave[%d] = %10.6f y_ave[%d] = %10.6f\n",                                k,y_average[l],l-1,y_average[l-1]);
                        (void) printf("lambda = %f\n",lambda);
                        (void) printf("lexpand[1] = %d\n",lexpand[1]);
                    }
                }
                if (l < G[1]-1)
                {
                    lambda = y_average[l+1]/y_average[l];
                    if (lambda > 1.05) uexpand[1] = 1;
                    else if (lambda < 1./1.05) uexpand[1] = -1;
                    else uexpand[1] = 0;
                    if (uexpand[1] != 0) adaptation = YES;
                    if (debugging("adapt_subdomain"))
                    {
                        (void) printf("Comparing y-average with upper side:\n");                        (void) printf("y_ave[%d] = %10.6f y_ave[%d] = %10.6f\n",                                k,y_average[l],l+1,y_average[l+1]);
                        (void) printf("lambda = %f\n",lambda);
                        (void) printf("uexpand[1] = %d\n",uexpand[1]);
                    }
                }
            }

	    for (i = 0; i < G[2]; ++i)
            {
                sub_icoords[2] = i;
                z_average[i] = 0.0;
                for (j = 0; j < G[0]; ++j)
                {
                    sub_icoords[0] = j;
                    for (k = 0; k < G[1]; ++k)
                    {
                        sub_icoords[1] = k;
                        id = domain_id(sub_icoords,G,3);
                        z_average[i] += sub_cpu[id];
                    }
                }
            }	
	    if (G[2] > 1)
            {
                find_Cartesian_coordinates(pp_mynode(),pp_grid,sub_icoords);
                if (debugging("adapt_subdomain"))
                {
                    (void) printf("G = %d %d %d\n",G[0],G[1],G[2]);
                    (void) printf("sub_icoords = %d %d %d\n",sub_icoords[0],
                                        sub_icoords[1],sub_icoords[2]);
                }
                l = sub_icoords[2];
                if (l > 0)
                {
                    lambda = z_average[l]/z_average[l-1];
                    if (lambda < 1./1.05) lexpand[2] = 1;
                    else if (lambda > 1.05) lexpand[2] = -1;
                    else lexpand[2] = 0;
                    if (lexpand[2] != 0) adaptation = YES;
                    if (debugging("adapt_subdomain"))
                    {
                        (void) printf("Comparing z-average with lower side:\n");                        (void) printf("z_ave[%d] = %10.6f z_ave[%d] = %10.6f\n",                                k,z_average[l],l-1,z_average[l-1]);
                        (void) printf("lambda = %f\n",lambda);
                        (void) printf("lexpand[2] = %d\n",lexpand[2]);
                    }
                }
                if (l < G[2]-1)
                {
                    lambda = z_average[l+1]/z_average[l];
                    if (lambda > 1.05) uexpand[2] = 1;
                    else if (lambda < 1./1.05) uexpand[2] = -1;
                    else uexpand[2] = 0;
                    if (uexpand[2] != 0) adaptation = YES;
                    if (debugging("adapt_subdomain"))
                    {
                        (void) printf("Comparing z-average with upper side:\n");                        (void) printf("z_ave[%d] = %10.6f z_ave[%d] = %10.6f\n",                                k,z_average[l],l+1,z_average[l+1]);
                        (void) printf("lambda = %f\n",lambda);
                        (void) printf("uexpand[2] = %d\n",uexpand[2]);
                    }
                }
            }
	    break;
	}
        if (debugging("adapt_subdomain") && !debugging("re_adapt_subdomain"))
	{
	    printf("Debugging adapt_subdomain\n");
	    fprintf(debug_file,"lexpand = ");
	    for (i = 0; i < dim; ++i)
	    	fprintf(debug_file,"%d ",lexpand[i]);
	    fprintf(debug_file,"\n");
	    fprintf(debug_file,"uexpand = ");
	    for (i = 0; i < dim; ++i)
		fprintf(debug_file,"%d ",uexpand[i]);
	    fprintf(debug_file,"\n\n");
	    fflush(debug_file);
	}
	else if (debugging("re_adapt_subdomain"))
	{
	    char step_string[100];
	    printf("Debugging re_adapt_subdomain\n");
	    adaptation = NO;
	    sprintf(debug_name,"adapt_step-%d",pp_mynode());
	    debug_file = fopen(debug_name,"r");
	    sprintf(step_string,"Step = %d",front->step);
	    fgetstring(debug_file,step_string);
	    fgetstring(debug_file,"lexpand =");
	    for (i = 0; i < dim; ++i)
	    {
	    	fscanf(debug_file,"%d ",&lexpand[i]);
		if (lexpand[i] != 0) 
		    adaptation = YES;
	    }
	    fgetstring(debug_file,"uexpand =");
	    for (i = 0; i < dim; ++i)
	    {
	    	fscanf(debug_file,"%d ",&uexpand[i]);
		if (uexpand[i] != 0) 
		    adaptation = YES;
	    }
	    fclose(debug_file);
	    printf("Repeating step %d:\n",front->step);
	    printf("lexpand = ");
	    for (i = 0; i < dim; ++i)
	    	printf("%d ",lexpand[i]);
	    printf("\n");
	    printf("uexpand = ");
	    for (i = 0; i < dim; ++i)
	    	printf("%d ",uexpand[i]);
	    printf("\n\n");
	}
	if (!pp_max_status(adaptation)) 
	{
            if (debugging("adapt_subdomain"))
            {
                (void) printf("No adaptation\n");
                (void) printf("Leaving adapt_subdomain()\n");
            }
	    return NO;
	}
	for (i = 0; i < dim; ++i)
	{
	    if (lexpand[i] == -1)
	    {
	    	gr->L[i] = gr->L[i] + gr->h[i];
		gr->gmax[i] -= 1;
	    }
	    else if (lexpand[i] == 1)
	    {
	    	gr->L[i] = gr->L[i] - gr->h[i];
		gr->gmax[i] += 1;
	    }
	    if (uexpand[i] == -1)
	    {
	    	gr->U[i] = gr->U[i] - gr->h[i];
		gr->gmax[i] -= 1;
	    }
	    else if (uexpand[i] == 1)
	    {
	    	gr->U[i] = gr->U[i] + gr->h[i];
		gr->gmax[i] += 1;
	    }
	}
	set_rect_grid(gr->L,gr->U,gr->GL,gr->GU,gr->lbuf,gr->ubuf,gr->gmax,
			dim,&gr->Remap,gr);
	set_rect_grid(gr->L,gr->U,gr->GL,gr->GU,gr->lbuf,gr->ubuf,gr->gmax,
			dim,&gr->Remap,computational_grid(front->interf));
	gr = &topological_grid(front->interf);
	for (i = 0; i < dim; ++i)
	{
	    if (lexpand[i] == -1)
	    {
	    	gr->L[i] = gr->L[i] + gr->h[i];
		gr->gmax[i] -= 1;
	    }
	    else if (lexpand[i] == 1)
	    {
	    	gr->L[i] = gr->L[i] - gr->h[i];
		gr->gmax[i] += 1;
	    }
	    if (uexpand[i] == -1)
	    {
	    	gr->U[i] = gr->U[i] - gr->h[i];
		gr->gmax[i] -= 1;
	    }
	    else if (uexpand[i] == 1)
	    {
	    	gr->U[i] = gr->U[i] + gr->h[i];
		gr->gmax[i] += 1;
	    }
	}
	set_rect_grid(gr->L,gr->U,gr->GL,gr->GU,gr->lbuf,gr->ubuf,gr->gmax,
			dim,&gr->Remap,&topological_grid(front->interf));
	front->interf->modified = YES;
	if (!scatter_front(front))
	{
	    screen("scatter_front() failed in adapt_subdomain()\n");
	    clean_up(ERROR);
	}
	return YES;
}	/* end cpu_adapt_front */

LOCAL   void reflect_array_buffer(
        int dim,
        int dir,
        int side,
        int *gmax,
        int *lbuf,
        int *ubuf,
        double *solute)
{
        int i,j,k;
        int isend,irecv;
        switch (dim)
        {
        case 1:
            if (side == 0)
            {
                for (i = 0; i < lbuf[0]; ++i)
                {
                    isend = d_index1d(lbuf[0]+i,gmax);
                    irecv = d_index1d(lbuf[0]-1-i,gmax);
                    solute[irecv] = solute[isend];
                }
            }
            else
            {
                for (i = 0; i < ubuf[0]; ++i)
                {
                    isend = d_index1d(gmax[0]-ubuf[0]-i,gmax);
                    irecv = d_index1d(gmax[0]-ubuf[0]+1+i,gmax);
                    solute[irecv] = solute[isend];
                }
            }
            break;
        case 2:
            break;
        case 3:
            break;
        }
}       /* end reflect_array_buffer */

LOCAL void pack_index_in_dir(Front*,POINTER,int*,int*,int*,int,int);
LOCAL void unpack_index_in_dir(Front*,POINTER,int*,int*,int*,int,int);
LOCAL void reflect_index_in_dir(Front*,POINTER,int*,int*,int,int);

LOCAL void pack_index_in_dir2d(Front*,int**,int*,int*,int*,int,int);
LOCAL void unpack_index_in_dir2d(Front*,int**,int*,int*,int*,int,int);
LOCAL void reflect_index_in_dir2d(Front*,int**,int*,int*,int,int);

LOCAL void pack_index_in_dir3d(Front*,int***,int*,int*,int*,int,int);
LOCAL void unpack_index_in_dir3d(Front*,int***,int*,int*,int*,int,int);
LOCAL void reflect_index_in_dir3d(Front*,int***,int*,int*,int,int);

EXPORT 	void scatter_cell_index(
	Front *fr,
	int *lbuf,
	int *ubuf,
	POINTER ijk_to_I)
{
	INTERFACE *intfc = fr->interf;
	int       me[MAXD], him[MAXD];
	int       myid, dst_id;
	PP_GRID   *pp_grid = fr->pp_grid;
	RECT_GRID *gr = fr->rect_grid;
	int       *G = pp_grid->gmax;
	int       i, j, k;
        int       dim = gr->dim;
	int       *gmax = gr->gmax;
	int       *bfs,*bfr;
	int	  size,max_size,max_buf;
	int	  index_tag = 8;

	max_size = max_buf = 0;
	for (i = 0; i < dim; ++i) 
	{
	    if (max_size < gmax[i] + lbuf[i] + ubuf[i]) 
	    	max_size = gmax[i] + lbuf[i] + ubuf[i];
	    if (max_buf < lbuf[i]) max_buf = lbuf[i];
	    if (max_buf < ubuf[i]) max_buf = ubuf[i];
	}
	size = max_buf;
	for (i = 0; i < dim-1; ++i) size *= max_size;

	uni_array(&bfs,size,sizeof(int));
        uni_array(&bfr,size,sizeof(int));

	find_Cartesian_coordinates(pp_mynode(),pp_grid,me);
	for (i = 0; i < dim; ++i)
	{
	    for (j = 0; j < 2; ++j)
	    {
		for (k = 0; k < dim; ++k)
		    him[k] = me[k];

		size = 1;
		for (k = 1; k < dim; ++k)
		    size *= (gmax[(i+k)%dim] + lbuf[(i+k)%dim] + 
				ubuf[(i+k)%dim]);
		size = (j == 0) ? size*lbuf[i] : size*ubuf[i];

	    	if (rect_boundary_type(intfc,i,j) == SUBDOMAIN_BOUNDARY)
		{
		    him[i] = (me[i] + 2*j - 1 + G[i])%G[i];
		    dst_id = domain_id(him,G,dim);
		    pack_index_in_dir(fr,ijk_to_I,lbuf,ubuf,bfs,i,j);
		    pp_send(index_tag,bfs,size*INT,dst_id);
		}
		else if (rect_boundary_type(intfc,i,j) == REFLECTION_BOUNDARY)
		{
		    reflect_index_in_dir(fr,ijk_to_I,lbuf,ubuf,i,j);
		}
	    }
	    pp_gsync();
	    for (j = 0; j < 2; ++j)
	    {
		for (k = 0; k < dim; ++k)
		    him[k] = me[k];

		size = 1;
		for (k = 1; k < dim; ++k)
		    size *= (gmax[(i+k)%dim] + lbuf[(i+k)%dim] + 
				ubuf[(i+k)%dim]);
		size = ((j+1)%2 == 0) ? size*lbuf[i] : size*ubuf[i];

		if (rect_boundary_type(intfc,i,(j+1)%2) == SUBDOMAIN_BOUNDARY)
		{
		    him[i] = (me[i] - 2*j + 1 + G[i])%G[i];
		    dst_id = domain_id(him,G,dim);
		    pp_recv(index_tag,dst_id,bfr,size*INT);
		    unpack_index_in_dir(fr,ijk_to_I,lbuf,ubuf,bfr,i,(j+1)%2);
		}
	    }
	}
	free_these(2,bfs,bfr);
}	/* end scatter_index */

LOCAL 	void pack_index_in_dir(
	Front *fr,
	POINTER IJK_to_I,
	int *lbuf,
	int *ubuf,
	int *bfs,
	int dir,
	int nb)
{
	int dim = fr->rect_grid->dim;
	switch(dim)
	{
	case 2:
	    pack_index_in_dir2d(fr,(int**)IJK_to_I,lbuf,ubuf,bfs,dir,nb);
	    return;
	case 3:
	    pack_index_in_dir3d(fr,(int***)IJK_to_I,lbuf,ubuf,bfs,dir,nb);
	    return;
	}
}	/* end pack_index_in_dir */

LOCAL 	void unpack_index_in_dir(
	Front *fr,
	POINTER IJK_to_I,
	int *lbuf,
	int *ubuf,
	int *bfr,
	int dir,
	int nb)
{
	int dim = fr->rect_grid->dim;
	switch(dim)
	{
	case 2:
	    unpack_index_in_dir2d(fr,(int**)IJK_to_I,lbuf,ubuf,bfr,dir,nb);
	    return;
	case 3:
	    unpack_index_in_dir3d(fr,(int***)IJK_to_I,lbuf,ubuf,bfr,dir,nb);
	    return;
	}
}	/* end unpack_index_in_dir */

LOCAL 	void reflect_index_in_dir(
	Front *fr,
	POINTER IJK_to_I,
	int *lbuf,
	int *ubuf,
	int dir,
	int nb)
{
	int dim = fr->rect_grid->dim;
	switch(dim)
	{
	case 2:
	    reflect_index_in_dir2d(fr,(int**)IJK_to_I,lbuf,ubuf,dir,nb);
	    return;
	case 3:
	    reflect_index_in_dir3d(fr,(int***)IJK_to_I,lbuf,ubuf,dir,nb);
	    return;
	}
}	/* end reflect_index_in_dir */

LOCAL 	void pack_index_in_dir3d(
	Front *fr,
	int ***IJK_to_I,
	int *lbuf,
	int *ubuf,
	int *bfs,
	int dir,
	int nb)
{
	int index = 0;
	int i,j,k,m,imax,jmax,kmax;
	int *gmax = fr->rect_grid->gmax;

	imax = gmax[0] + lbuf[0] + ubuf[0];
	jmax = gmax[1] + lbuf[1] + ubuf[1];
	kmax = gmax[2] + lbuf[2] + ubuf[2];
	m = (nb == 0) ? lbuf[dir] : ubuf[dir];

	switch (dir)
	{
	case 0:
	    if (nb == 0)
	    {
		for (j = 0; j < jmax; ++j)
		for (k = 0; k < kmax; ++k)
		for (i = 0; i < m; ++i)
		{
		    bfs[index++] = IJK_to_I[lbuf[0]+i][j][k];
		}
	    }
	    else if (nb == 1)
	    {
		for (j = 0; j < jmax; ++j)
		for (k = 0; k < kmax; ++k)
		for (i = 0; i < m; ++i)
		{
		    bfs[index++] = IJK_to_I[gmax[0]+lbuf[0]-ubuf[0]+i][j][k];
		}
	    }
	    break;
	case 1:
	    if (nb == 0)
	    {
		for (i = 0; i < imax; ++i)
		for (k = 0; k < kmax; ++k)
		for (j = 0; j < m; ++j)
		{
		    bfs[index++] = IJK_to_I[i][lbuf[1]+j][k];
		}
	    }
	    else if (nb == 1)
	    {
		for (i = 0; i < imax; ++i)
		for (k = 0; k < kmax; ++k)
		for (j = 0; j < m; ++j)
		{
		    bfs[index++] = IJK_to_I[i][gmax[1]+lbuf[1]-ubuf[1]+j][k];
		}
	    }
	    break;
	case 2:
	    if (nb == 0)
	    {
		for (i = 0; i < imax; ++i)
		for (j = 0; j < jmax; ++j)
		for (k = 0; k < m; ++k)
		{
		    bfs[index++] = IJK_to_I[i][j][lbuf[2]+k];
		}
	    }
	    else if (nb == 1)
	    {
		for (i = 0; i < imax; ++i)
		for (j = 0; j < jmax; ++j)
		for (k = 0; k < m; ++k)
		{
		    bfs[index++] = IJK_to_I[i][j][gmax[2]+lbuf[2]-ubuf[2]+k];
		}
	    }
	}
}	/* end pack_index_in_dir3d */

LOCAL 	void unpack_index_in_dir3d(
	Front *fr,
	int ***IJK_to_I,
	int *lbuf,
	int *ubuf,
	int *bfs,
	int dir,
	int nb)
{
	int index = 0;
	int i,j,k,m,imax,jmax,kmax;
	int *gmax = fr->rect_grid->gmax;

	imax = gmax[0] + lbuf[0] + ubuf[0];
	jmax = gmax[1] + lbuf[1] + ubuf[1];
	kmax = gmax[2] + lbuf[2] + ubuf[2];
	m = (nb == 0) ? lbuf[dir] : ubuf[dir];

	switch (dir)
	{
	case 0:
	    if (nb == 0)
	    {
		for (j = 0; j < jmax; ++j)
		for (k = 0; k < kmax; ++k)
		for (i = 0; i < m; ++i)
		{
		    IJK_to_I[i][j][k] = bfs[index++];
		}
	    }
	    else if (nb == 1)
	    {
		for (j = 0; j < jmax; ++j)
		for (k = 0; k < kmax; ++k)
		for (i = 0; i < m; ++i)
		{
		    IJK_to_I[gmax[0]+lbuf[0]+i][j][k] = bfs[index++];
		}
	    }
	    break;
	case 1:
	    if (nb == 0)
	    {
		for (i = 0; i < imax; ++i)
		for (k = 0; k < kmax; ++k)
		for (j = 0; j < m; ++j)
		{
		    IJK_to_I[i][j][k] = bfs[index++];
		}
	    }
	    else if (nb == 1)
	    {
		for (i = 0; i < imax; ++i)
		for (k = 0; k < kmax; ++k)
		for (j = 0; j < m; ++j)
		{
		    IJK_to_I[i][gmax[1]+lbuf[1]+j][k] = bfs[index++];
		}
	    }
	    break;
	case 2:
	    if (nb == 0)
	    {
		for (i = 0; i < imax; ++i)
		for (j = 0; j < jmax; ++j)
		for (k = 0; k < m; ++k)
		{
		    IJK_to_I[i][j][k] = bfs[index++];
		}
	    }
	    else if (nb == 1)
	    {
		for (i = 0; i < imax; ++i)
		for (j = 0; j < jmax; ++j)
		for (k = 0; k < m; ++k)
		{
		    IJK_to_I[i][j][gmax[2]+lbuf[2]+k] = bfs[index++];
		}
	    }
	}
}	/* end unpack_index_in_dir3d */

LOCAL 	void reflect_index_in_dir3d(
	Front *fr,
	int ***IJK_to_I,
	int *lbuf,
	int *ubuf,
	int dir,
	int nb)
{
	int index = 0;
	int i,j,k,m,imax,jmax,kmax;
	int *gmax = fr->rect_grid->gmax;

	imax = gmax[0] + lbuf[0] + ubuf[0];
	jmax = gmax[1] + lbuf[1] + ubuf[1];
	kmax = gmax[2] + lbuf[2] + ubuf[2];
	m = (nb == 0) ? lbuf[dir] : ubuf[dir];

	switch (dir)
	{
	case 0:
	    if (nb == 0)
	    {
		for (j = 0; j < jmax; ++j)
		for (k = 0; k < kmax; ++k)
		for (i = 0; i < m; ++i)
		{
		    IJK_to_I[i][j][k] = IJK_to_I[2*m-i-1][j][k];
		}
	    }
	    else if (nb == 1)
	    {
		for (j = 0; j < jmax; ++j)
		for (k = 0; k < kmax; ++k)
		for (i = 0; i < m; ++i)
		{
		    IJK_to_I[gmax[0]+lbuf[0]+i][j][k] = 
				IJK_to_I[gmax[0]+lbuf[0]-i-1][j][k];
		}
	    }
	case 1:
	    if (nb == 0)
	    {
		for (i = 0; i < imax; ++i)
		for (k = 0; k < kmax; ++k)
		for (j = 0; j < m; ++j)
		{
		    IJK_to_I[i][j][k] = IJK_to_I[i][2*m-j-1][k];
		}
	    }
	    else if (nb == 1)
	    {
		for (i = 0; i < imax; ++i)
		for (k = 0; k < kmax; ++k)
		for (j = 0; j < m; ++j)
		{
		    IJK_to_I[i][gmax[1]+lbuf[1]+j][k] = 
				IJK_to_I[i][gmax[1]+lbuf[1]-j-1][k];
		}
	    }
	case 2:
	    if (nb == 0)
	    {
		for (i = 0; i < imax; ++i)
		for (j = 0; j < jmax; ++j)
		for (k = 0; k < m; ++k)
		{
		    IJK_to_I[i][j][k] = IJK_to_I[i][j][2*m-k-1];
		}
	    }
	    else if (nb == 1)
	    {
		for (i = 0; i < imax; ++i)
		for (j = 0; j < jmax; ++j)
		for (k = 0; k < m; ++k)
		{
		    IJK_to_I[i][j][gmax[2]+lbuf[2]+k] = 
				IJK_to_I[i][j][gmax[2]+lbuf[2]-k-1];
		}
	    }
	}
}	/* end reflect_index_in_dir3d */

LOCAL 	void pack_index_in_dir2d(
	Front *fr,
	int **IJK_to_I,
	int *lbuf,
	int *ubuf,
	int *bfs,
	int dir,
	int nb)
{
	int i,j,m,imax;
	int *gmax = fr->rect_grid->gmax;

	if (dir == 0)
	{
	    imax = gmax[1] + lbuf[1] + ubuf[1];
	    switch(nb)
	    {
	    case 0:
		m = lbuf[0];
	    	for (i = 0; i < imax; ++i)
		{
		    for (j = 0; j < m; ++j)
		    	bfs[m*i+j] = IJK_to_I[lbuf[0]+j][i];
		}
		break;
	    case 1:
		m = ubuf[0];
	    	for (i = 0; i < imax; ++i)
		{
		    for (j = 0; j < m; ++j)
		    	bfs[m*i+j] = IJK_to_I[gmax[0]+lbuf[0]-ubuf[0]+j][i];
		}
	    }
	}
	else if (dir == 1)
	{
	    imax = gmax[0] + lbuf[0] + ubuf[0];
	    switch(nb)
	    {
	    case 0:
		m = lbuf[1];
	    	for (i = 0; i < imax; ++i)
		{
		    for (j = 0; j < m; ++j)
		    	bfs[m*i+j] = IJK_to_I[i][j+lbuf[1]];
		}
		break;
	    case 1:
		m = ubuf[1];
	    	for (i = 0; i < imax; ++i)
		{
		    for (j = 0; j < m; ++j)
		    	bfs[m*i+j] = IJK_to_I[i][gmax[1]+lbuf[1]-ubuf[1]+j];
		}
	    }
	}
}	/* end pack_index_in_dir2d */
	
LOCAL 	void unpack_index_in_dir2d(
	Front *fr,
	int **IJK_to_I,
	int *lbuf,
	int *ubuf,
	int *bfr,
	int dir,
	int nb)
{
	int i,j,m,imax;
	int *gmax = fr->rect_grid->gmax;

	if (dir == 0)
	{
	    imax = gmax[1] + lbuf[1] + ubuf[1];
	    switch(nb)
	    {
	    case 0:
		m = lbuf[0];
	    	for (i = 0; i < imax; ++i)
		{
		    for (j = 0; j < m; ++j)
		    	IJK_to_I[j][i] = bfr[m*i+j];
		}
		break;
	    case 1:
		m = ubuf[0];
	    	for (i = 0; i < imax; ++i)
		{
		    for (j = 0; j < m; ++j)
		    	IJK_to_I[gmax[0]+lbuf[0]+j][i] = bfr[m*i+j];
		}
	    }
	}
	else if (dir == 1)
	{
	    imax = gmax[0] + lbuf[0] + ubuf[0];
	    switch(nb)
	    {
	    case 0:
		m = lbuf[1];
	    	for (i = 0; i < imax; ++i)
		{
		    for (j = 0; j < m; ++j)
		    	IJK_to_I[i][j] = bfr[m*i+j];
		}
		break;
	    case 1:
		m = ubuf[1];
	    	for (i = 0; i < imax; ++i)
		{
		    for (j = 0; j < m; ++j)
		    	IJK_to_I[i][gmax[1]+lbuf[1]+j] = bfr[m*i+j];
		}
	    }
	}
}	/* end unpack_index_in_dir2d */
	
LOCAL 	void reflect_index_in_dir2d(
	Front *fr,
	int **IJK_to_I,
	int *lbuf,
	int *ubuf,
	int dir,
	int nb)
{
	int i,j,m,imax;
	int *gmax = fr->rect_grid->gmax;

	if (dir == 0)
	{
	    imax = gmax[1] + lbuf[1] + ubuf[1];
	    switch(nb)
	    {
	    case 0:
		m = lbuf[0];
	    	for (i = 0; i < imax; ++i)
		{
		    for (j = 0; j < m; ++j)
		    	IJK_to_I[j][i] = IJK_to_I[2*m-j-1][i];
		}
		break;
	    case 1:
		m = ubuf[0];
	    	for (i = 0; i < imax; ++i)
		{
		    for (j = 0; j < m; ++j)
		    	IJK_to_I[gmax[0]+lbuf[0]+j][i] = 
				IJK_to_I[gmax[0]+lbuf[0]-j-1][i];
		}
	    }
	}
	else if (dir == 1)
	{
	    imax = gmax[0] + lbuf[0] + ubuf[0];
	    switch(nb)
	    {
	    case 0:
		m = lbuf[1];
	    	for (i = 0; i < imax; ++i)
		{
		    for (j = 0; j < m; ++j)
		    	IJK_to_I[i][j] = IJK_to_I[i][2*m-j-1];
		}
		break;
	    case 1:
		m = ubuf[1];
	    	for (i = 0; i < imax; ++i)
		{
		    for (j = 0; j < m; ++j)
		    	IJK_to_I[i][gmax[0]+lbuf[0]+j] = 
				IJK_to_I[i][gmax[0]+lbuf[0]-j-1];
		}
	    }
	}
}	/* end reflect_index_in_dir2d */


/*
*               clip_front_to_rect_boundary_type():
*/

EXPORT	void	clip_front_to_rect_boundary_type(
	Front	*front)
{
	INTERFACE *intfc = front->interf;
	PP_GRID	  *pp_grid = front->pp_grid;
	RECT_GRID *t_gr, *c_gr, *zoom_gr, Dual_grid;
	int	  i, tgmax[MAXD],icoords[MAXD];

	zoom_gr = &pp_grid->Zoom_grid;
	find_Cartesian_coordinates(pp_mynode(),pp_grid,icoords);

	/* clip virtual boundaries */
	for (i = 0; i < intfc->dim; i++)
	{
	    if (!buffered_boundary_type(rect_boundary_type(intfc,i,0)))
		zoom_gr->lbuf[i] = 0;
	    if (!buffered_boundary_type(rect_boundary_type(intfc,i,1)))
		zoom_gr->ubuf[i] = 0;
	    if (icoords[i] > 0)
	        rect_boundary_type(intfc,i,0) = SUBDOMAIN_BOUNDARY;
	    if (icoords[i] < (pp_grid->gmax[i]-1))
	        rect_boundary_type(intfc,i,1) = SUBDOMAIN_BOUNDARY;
	}

	/* set topological and computational grids */
	t_gr = &topological_grid(intfc);
	c_gr = computational_grid(intfc);
	set_rect_grid(zoom_gr->L,zoom_gr->U,zoom_gr->GL,
		      zoom_gr->GU,zoom_gr->lbuf,
		      zoom_gr->ubuf,zoom_gr->gmax,
		      intfc->dim,&t_gr->Remap,zoom_gr);

	for (i = 0; i < intfc->dim; i++)
	{
	    double h = t_gr->h[i];
	    tgmax[i] = irint((zoom_gr->VU[i]-zoom_gr->VL[i])/h);
	}

	if(intfc->dim == 3)
	{
	    copy_rect_grid(c_gr,zoom_gr);
	    copy_rect_grid(front->rect_grid,zoom_gr);
	    set_dual_grid(&Dual_grid,c_gr);
	    set_expanded_grid(&Dual_grid,t_gr);
	    (void) adjust_top_grid_for_square(t_gr,zoom_gr);
	}
	else
	{
	    set_rect_grid(zoom_gr->VL,zoom_gr->VU,zoom_gr->GL,
	    	      zoom_gr->GU,NOBUF,NOBUF,tgmax,
	    	      intfc->dim,&t_gr->Remap,t_gr);
	    (void) adjust_top_grid_for_square(t_gr,zoom_gr);
	    copy_rect_grid(c_gr,zoom_gr);
	    copy_rect_grid(front->rect_grid,zoom_gr);
	}

	if (intfc->dim == 3 && pp_numnodes() > 1)
	{
	    delete_outside_surface(intfc);	
	    scatter_front(front);
	    communicate_default_comp(front);
	}
}		/*end clip_front_to_rect_boundary_type*/

