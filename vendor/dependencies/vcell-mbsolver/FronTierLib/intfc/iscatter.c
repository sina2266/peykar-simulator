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
*				iscatter.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*/


#include <intfc/iloc.h>

/*
*		find_Cartesian_coordinates():
*
*	Identifies the coordinates of the subdomain with pp node id.  The
*	identification of the id and subdomain coordinates are given by
*
*	id = icoords[0] + icoords[1]*gmax[0] + icoords[2]*gmax[0]*gmax[1].
*
*	This function is the logical inverse of the function domain_id().
*
*	This defines the natural lexographical ft_assignment of pp id numbers
*	to the subdomains, as illustrated for this 4x3 partition:
*
*	----------------------------------------
*	|        |         |         |         |
*	|   8    |    9    |   10    |   11    |
*	|        |         |         |         |
*	| (0,2)  |  (1,2)  |  (2,2)  |  (3,2)  |
*	----------------------------------------       -----------
*	|        |         |         |         |       |         |
*	|   4    |    5    |    6    |    7    |       |   id    | 
*	|        |         |         |         |       |         |
*	| (0,1)  |  (1,1)  |  (2,1)  |  (3,1)  |       | icoords |
*	----------------------------------------       -----------
*	|        |         |         |         |
*	|   0    |    1    |    2    |    3    |
*	|        |         |         |         |
*	| (0,0)  |  (1,0)  |  (2,0)  |  (3,0)  |
*	----------------------------------------
*
*/


EXPORT	void find_Cartesian_coordinates(
	int		id,
	PP_GRID		*pp_grid,
	int		*icoords)
{
	int 	dim =  pp_grid->Global_grid.dim;
	int 	d, G;

	for (d = 0; d < dim; d++)
	{
	    G = pp_grid->gmax[d];
	    icoords[d] = id % G;
	    id = (id - icoords[d])/G;
	}
}		/*end find_Cartesian_coordinates*/

/*
*			neighbor_id():
*
*	Finds the cartesian coordinate, him, and process id of the subdomain
*	on side, side, in the direction dir of the subdomain with
*	coordinates me.
*/

EXPORT	int	neighbor_id(
	int		*him,
	int		*me,
	int		dir,
	int		side,
	PP_GRID		*pp_grid)
{
	int		*G = pp_grid->gmax;
	int		i, dim = pp_grid->Global_grid.dim;

	for (i = 0; i < dim; i++)
	    him[i] = me[i];

	him[dir] = (me[dir] + 2*side - 1);
	if (him[dir] < 0)
	    him[dir] = G[dir] - 1;
	if (him[dir] >= G[dir])
	    him[dir] = 0;

	return domain_id(him,G,dim);
}		/*end neighbor_id*/

/*
*			domain_id():
*
*	Translates the integer coordinates of a subdomain into the pp id
*	of that domain.  This is the logical inverse of the function
*	find_Cartesian_coordinates().
*/

EXPORT  int domain_id(
	int		*icoords,
	int		*G,
	int		dim)
{
	int		tmpid;
	int		i;

	tmpid = icoords[dim-1];
	for (i = dim-2; i >= 0; i--)
	    tmpid = icoords[i] + G[i]*tmpid;
	return tmpid;
}		/*end domain_id*/


EXPORT	void print_PP_GRID_structure(
	PP_GRID		*pp_grid)
{
	int		i, j;
	int		dim;

	(void) printf("\n\t\tPP_GRID %p structure\n",(POINTER)pp_grid);
	if (pp_grid == NULL)
	{
		(void) printf("NULL PP_GRID");
		return;
	}
	dim = pp_grid->Global_grid.dim;
	(void) printf("\n\t1. Frame:\n\n");
	for (i = 0; i < dim; i++)
	{
		(void) printf("L[%d] = %g, U[%d] = %g, gmax[%d] = %d\n",
				i,pp_grid->dom[i][0],i,
				pp_grid->dom[i][pp_grid->gmax[i]],
				i,pp_grid->gmax[i]);
	}

	(void) printf("\n\t2. Subdomain Corners:\n\n");
	for (i = 0; i < pp_grid->Global_grid.dim; i++)
	{
		(void) printf("%d - direction:\n",i);
		for (j = 0; j < pp_grid->gmax[i]+1; j++)
			(void) printf("%-10g ",pp_grid->dom[i][j]);
		(void) printf("\n\n");
	}
 
 
	(void) printf("Global_grid\n");
	print_rectangular_grid(&pp_grid->Global_grid);
	(void) printf("Zoom_grid\n");
	print_rectangular_grid(&pp_grid->Zoom_grid);
 
	(void) printf("\t\tEnd PP_GRID %p structure\n\n",(POINTER)pp_grid);
}		/*end print_PP_GRID_structure*/


EXPORT	PP_GRID *set_pp_grid(
	INIT_DATA	*init,
	RECT_GRID       *comp_glbgr)
{
	double		L[MAXD], U[MAXD], *GL, *GU;
	double		*h = comp_glbgr->h;
	int		lbuf[MAXD], ubuf[MAXD];
	int		gmax[MAXD];
	int		icoords[MAXD];
	int		i, dim = comp_glbgr->dim;
	int		myid = pp_mynode();
	static PP_GRID	Pp_grid;
	PP_GRID		*pp_grid = &Pp_grid;
	char		s[1028];

	debug_print("init_pp_grid","Entered set_pp_grid():\n");

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

	if (debugging("init_pp_grid"))
	{
	    (void) printf("pp_grid after set_pp_grid()\n");
	    (void) print_PP_GRID_structure(pp_grid);
	}
	debug_print("init_pp_grid","Left i_set_pp_grid():\n");
	return pp_grid;
}		/*end set_pp_grid*/
