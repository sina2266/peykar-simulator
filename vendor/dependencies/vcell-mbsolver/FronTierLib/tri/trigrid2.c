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
*				trigrid2.c:
*
*       Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*/


#if defined(THREED)

#include <tri/trilocaldecs.h>
#include <tri/tri3ddefs.h>

	/* LOCAL Function Declarations */
LOCAL	int	ERROR_RETURN_FROM_RECONSTRUCT_INTFC_AND_TRI_GRID(const char*,
								 int,
								 INTERFACE*,
								 TRI_GRID*);

LOCAL	int ERROR_RETURN_FROM_RECONSTRUCT_INTFC_AND_TRI_GRID(
	const char *mesg,
	int	   status,
	INTERFACE  *cintfc,
	TRI_GRID   *grid)
{
	(void) printf("WARNING in reconstruct_intfc_and_tri_grid(), %s\n",mesg);
	set_current_interface(cintfc);
	free_grid_lines(&grid->rect_grid);
	return status;
}		/*end ERROR_RETURN_FROM_RECONSTRUCT_INTFC_AND_TRI_GRID*/


/*rgr is the dual of computational grid */
LIB_LOCAL	int reconstruct_intfc_and_tri_grid(
	TRI_GRID  *grid,
	RECT_GRID *rgr,
	Front     *front)
{
	INTERFACE	*intfc = front->interf;
	size_t		sizest = front->sizest;
	INTERFACE	*cintfc;
	int		status = ERROR_IN_STEP;
	int		stat;
	DEBUG_ENTER(reconstruct_intfc_and_tri_grid)

	debug_print("tri_grid","Entered construct_tri_grid()\n");

	if (intfc->dim != 3)
	{
	    screen("ERROR in reconstruct_intfc_and_tri_grid(), "
		   "dim == %d != 3 not supported\n",intfc->dim);
	    clean_up(ERROR);
	}
	start_clock("construct_tri_grid");

	set_tri_grid_rect_grids(grid,rgr,intfc);
	set_tri3d_tolerances(grid);

	/* copy grid_intfc from intfc and insert grid crossings, */
	/* storing the information in the crx_store[] array */

	cintfc = current_interface();

	/*printf("#reconstruct_intfc_and_tri_grid\n"); */
	
	set_size_of_intfc_state(sizest);
	set_copy_intfc_states(YES);
	if ((grid->grid_intfc = copy_interface(intfc)) == NULL)
	{
	    stat = ERROR_RETURN_FROM_RECONSTRUCT_INTFC_AND_TRI_GRID(
			"copy_interface() failed",
			ERROR_IN_STEP,cintfc,grid);
	    DEBUG_LEAVE(reconstruct_intfc_and_tri_grid)
	    return stat;
	}
	no_topology_lists(grid->grid_intfc) = YES;
	
	/*DEBUG_TMP check_print_intfc("Construct grid_intfc", NULL, 'g',  */
	       /*DEBUG_TMP grid->grid_intfc, 0, -1, NO); */

	/*
	*	Set correspond curve on intfc to null, but leave
	*	the curve correspondence from grid_intfc to intfc
	*	intact.
	*/

	set_correspond_hyper_surfaces_to_NULL(intfc);

	delete_subdomain_boundaries(grid->grid_intfc);
	delete_passive_boundaries(grid->grid_intfc);

	interpolate_intfc_states(grid->grid_intfc) = YES;

	start_clock("insert_grid_crossings3d");
	init_triangulation_storage(grid,NULL,sizest);
	insert_grid_crossings3d(grid,front);
	set_interpolation_storage3d(grid);
	stop_clock("insert_grid_crossings3d");
	
	/*printf("#set_comp bf\n"); */
	/*add_to_debug("interp_grid"); */

	start_clock("set_components");
	status = set_components(grid,intfc);

	stop_clock("set_components");

	if (status != GOOD_STEP)
	{
	     stat = ERROR_RETURN_FROM_RECONSTRUCT_INTFC_AND_TRI_GRID(
			"set_components failed",
			status,cintfc,grid);
	     DEBUG_LEAVE(reconstruct_intfc_and_tri_grid)
	     return stat;
	}

	/* set up grid nodes, triangles, and quadrangles */

	start_clock("triangulate_mesh");
	status = triangulate_mesh(grid);
	stop_clock("triangulate_mesh");

	if (status != GOOD_STEP)
	{ 
	    stat = ERROR_RETURN_FROM_RECONSTRUCT_INTFC_AND_TRI_GRID(
			"triangulate_mesh() failed",status,cintfc,grid);
	    DEBUG_LEAVE(reconstruct_intfc_and_tri_grid)
	    return stat;
	}

	free_grid_lines(&grid->rect_grid);
	set_current_interface(cintfc);

	stop_clock("construct_tri_grid");
	debug_print("tri_grid","Left construct_tri_grid()\n");
	DEBUG_LEAVE(reconstruct_intfc_and_tri_grid)
	return status;
}		/*end reconstruct_intfc_and_tri_grid*/

#endif /* defined(THREED) */
