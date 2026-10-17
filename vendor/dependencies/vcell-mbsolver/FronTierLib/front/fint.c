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
*				fint.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Contains global functions needed for the support of the
*	user include program fragments in fuserint.h.
*
*	This file should provide the minimal additional source
*	needed for the compilation of programs that use the
*	front user includes, fusertypes.h,  fuserint.h, and
*	fusertop.h
*
*/

#include <front/fdecs.h>


	/* LOCAL Function Declarations */
LOCAL	int	old_read_print_boundary_state_data_list(INIT_DATA*,
                                                        const IO_TYPE*,
                                                        INTERFACE*);
LOCAL	int	read_print_boundary_state_data_list(INIT_DATA*,const IO_TYPE*,
                                                    INTERFACE*);
LOCAL	void	copy_excluded_comps_lists(COMP_LIST*,COMP_LIST*);
LOCAL	void	old_read_print_boundary_state_data(INIT_DATA*,const IO_TYPE*,
                                                   INTERFACE*,int);

LOCAL	CURVE*	f_user_2d_make_curve(CURVE*);
LOCAL	INTERFACE*	f_user_2d_copy_interface(INTERFACE*,INTERFACE*);
LOCAL	REDISTRIBUTION_DIRECTION read_redistribution_direction_from_string(
	    const char *rd);
LOCAL	boolean	f_user_2d_join_curves(CURVE*,CURVE*,CURVE*);
LOCAL	boolean	f_user_2d_split_curve(int,FT_POINT*,BOND*,CURVE*,CURVE**);
LOCAL	boolean	f_user_2d_insert_point_in_bond(FT_POINT*,BOND*,CURVE*);
LOCAL	int	f_user_2d_delete_curve(CURVE*);
LOCAL	void	f_user_2d_reconstruct_curve_pointers(CURVE*,INTERFACE*,
						     INTERFACE*,POINTER*,
						     POINTER*,int);
LOCAL	void	set_2d_rect_bdry_types_from_curves(INTERFACE*);

LOCAL	INTERFACE  *f_user_3d_copy_interface(INTERFACE*,INTERFACE*);
LOCAL	boolean	   f_user_3d_join_curves(CURVE*,CURVE*,CURVE*);
LOCAL	boolean    f_user_3d_insert_point_in_bond(BOND*);
LOCAL	void	   interpolation_coefs_in_tri(double*,double*,TRI*);
LOCAL	void 	f_read_print_Dirichlet_bdry_states(INIT_DATA*,const IO_TYPE*,
					INTERFACE*);
LOCAL	void 	f_fprint_Dirichlet_bdry_states(FILE*,INTERFACE*);

	/* LOCAL external variables */
LOCAL size_t	size_of_intfc_state = 0;
LOCAL boolean	copyintfcstates = YES;
LOCAL boolean      set_copy_from_to_lists = NO;
/*#bjet2 */
LOCAL boolean      usewalledge = NO;

LOCAL  void  printf_local_varible(char  *s)
{
    printf("#%s %lu  %d  %d\n", s, size_of_intfc_state,
              copyintfcstates,set_copy_from_to_lists);
}

EXPORT void set_use_wall_edge(
	boolean		y_or_n)
{
	usewalledge = y_or_n;
}

EXPORT boolean use_wall_edge(void)
{
	return usewalledge;
}

/*
*		set_size_of_intfc_state():
*
*	To be called before all calls to make_interface().
*/

EXPORT void set_size_of_intfc_state(
	size_t		size)
{
	size_of_intfc_state = size;
}		/*end set_size_of_intfc_state*/

/*#bjet2 */
EXPORT int get_size_of_intfc_state(void)
{
	return size_of_intfc_state;
}		/*end get_size_of_intfc_state*/

/*
*		set_copy_intfc_states():
*
*	To be called with argument NO before a call to
*	copy_interface() if no copying of states is to be done;
*	also should be called with argument YES after the copying.
*/

EXPORT void set_copy_intfc_states(
	boolean		yes_no)
{
	copyintfcstates = yes_no;
}		/*end set_copy_intfc_states*/

EXPORT boolean copy_intfc_states(void)
{
	return copyintfcstates;
}		/*end copy_intfc_states*/


EXPORT	void set_computational_grid(
	INTERFACE	*intfc,
	RECT_GRID	*grid)
{
	if (grid == NULL)
	    return;
	copy_rect_grid(computational_grid(intfc),grid);
}		/*end set_computational_grid*/



/*ARGSUSED*/
EXPORT	INTERFACE *f_copy_interface(
	INTERFACE	*intfc)
{
    	HYPER_SURF      **hs;
	HYPER_SURF_BDRY **hsb;
	INTERFACE	*new_intfc;
	int		dim = intfc->dim;

	set_copy_from_to_lists = YES;
	new_intfc = i_copy_interface(intfc);
	set_copy_from_to_lists = NO;

	set_computational_grid(new_intfc,computational_grid(intfc));
	f_user_interface(new_intfc) = f_user_interface(intfc);
	copy_excluded_comps_lists(&excluded_comps(new_intfc),
				  &excluded_comps(intfc));

	switch (dim)
	{
	case 1:
	    if (!set_correspondence_between_interfaces(intfc,new_intfc))
	    {
	        (void) printf("WARNING in f_copy_interface(), "
	                      "can't set interface correspondence\n");
	        (void) delete_interface(new_intfc);
	        return NULL;
	    }
	    break;
	case 2:
	    new_intfc = f_user_2d_copy_interface(new_intfc,intfc);
	    return new_intfc;
	case 3: 
	    /*#bjet bond_tri states are copied in copy_tris */
	    new_intfc = f_user_3d_copy_interface(new_intfc,intfc);
	    return new_intfc;
	}

	/* Clear the copied from and copied to fields*/
	for (hs = hyper_surf_list(intfc); hs && *hs; ++hs)
	    hs_copied_from(*hs) = hs_copied_to(*hs) = NULL;
	for (hs = hyper_surf_list(new_intfc); hs && *hs; ++hs)
	    hs_copied_from(*hs) = hs_copied_to(*hs) = NULL;
	for (hsb = hyper_surf_bdry_list(intfc); hsb && *hsb; ++hsb)
	    hsb_copied_from(*hsb) = hsb_copied_to(*hsb) = NULL;
	for (hsb = hyper_surf_bdry_list(new_intfc); hsb && *hsb; ++hsb)
	    hsb_copied_from(*hsb) = hsb_copied_to(*hsb) = NULL;
	return new_intfc;
}		/*end f_copy_interface*/



EXPORT	int f_delete_interface(
	INTERFACE	*intfc)
{
	int status;

	remove_corresponds_to_deleted_interface(intfc);
	rst_cor_after_delete_interface(intfc);
	status = i_delete_interface(intfc);
	return status;
}		/*end f_delete_interface*/




EXPORT	void	f_user_fprint_interface(
	FILE		*file,
	INTERFACE	*intfc)
{
	int		i;
	int		print_bstates = NO;

	(void) fprintf(file,"\n");
	(void) foutput(file);
	(void) fprintf(file,"Interface normal and tangent operators\n");
	(void) fprintf(file,"Interface normal function = %s\n",
	               interface_normal_name(intfc));
	(void) fprintf(file,"Interface tangent function = %s\n",
	               interface_tangent_name(intfc));
	(void) fprintf(file,"End interface normal and tangent operators\n");

	for (i = 0; i < num_bstates(intfc); ++i)
	{
	    if (bstate_list(intfc)[i] != NULL)
	    {
	    	print_bstates = YES;
	    	break;
	    }
	}

	(void) fprintf(file,"\n");
	if (print_bstates)
	{
	    BOUNDARY_STATE *bstate;

	    (void) foutput(file);
	    (void) fprintf(file,"Boundary state data for interface %llu\n",
		                interface_number(intfc));
	    (void) fprintf(file,"num_bstates = %d\n",num_bstates(intfc));
	    for (i = 0; i < num_bstates(intfc); ++i)
	    {
		bstate = bstate_list(intfc)[i];
		if (bstate != NULL)
		{
	    	    (void) fprintf(file,"Boundary state data for index %d\n",i);
		    (*bstate->_fprint_boundary_state_data)(file,intfc,bstate);
	    	    (void) fprintf(file,
				  "\nEnd Boundary state data for index %d\n",i);
		}
		else
	    	    (void) fprintf(file,
				"No boundary state data for index %d\n",i);
	    }
	    (void) foutput(file);
	    (void) fprintf(file,"End Boundary state data for interface %llu\n",
		    interface_number(intfc));
        }

	(void) fprintf(file,"\n");
	(void) foutput(file);
	(void) fprintf(file,"Excluded Components List:\n");
	(void) fprintf(file,"Number of excluded components = %d\n",
		       excluded_comps(intfc).ncomps);
	for (i = 0; i < excluded_comps(intfc).ncomps; ++i)
	    (void) fprintf(file,"Excluded component %d = %d\n",i,
			   excluded_comps(intfc).comps[i]);
	(void) foutput(file);
	(void) fprintf(file,"End Excluded Components List:\n");
	f_fprint_Dirichlet_bdry_states(file,intfc);
}		/*end f_user_fprint_interface*/

EXPORT	int	add_bstate_to_list(
	BOUNDARY_STATE	*bstate,
	INTERFACE	*intfc,
	int		index)
{
	BOUNDARY_STATE	*new_bstate;
	size_t		sizest = size_of_state(intfc);

	if (bstate == NULL)
	    return -1;

	if (index < 0)	/* add bs to end */
	    index = num_bstates(intfc);
	if (index >= num_bstates(intfc))
	{
	    int i, N = index + 1;
	    BOUNDARY_STATE **new_bstates;
	    BOUNDARY_STATE **old_bstates = bstate_list(intfc);

	    uni_array(&new_bstates,N+1,sizeof(BOUNDARY_STATE*));
	    ++new_bstates;
	    for (i = -1; i < num_bstates(intfc); ++i)
	    	new_bstates[i] = old_bstates[i];
	    for (; i < N; ++i)
	    	new_bstates[i] = NULL;
	    bstate_list(intfc) = new_bstates;
	    num_bstates(intfc) = N;
	    free(old_bstates-1);
	}
	if (bstate_list(intfc)[index] == NULL)
	{
	    scalar(&bstate_list(intfc)[index],sizeof(BOUNDARY_STATE));
	}
	new_bstate = bstate_list(intfc)[index];
	*new_bstate = *bstate;
	if (bstate->_boundary_state != NULL)
	{
	    alloc_state(intfc,&new_bstate->_boundary_state,sizest);
	    ft_assign(new_bstate->_boundary_state,bstate->_boundary_state,
				sizest);
            new_bstate->_fprint_boundary_state_data =
                                bstate->_fprint_boundary_state_data;
	}
	return index;
}		/*end add_bstate_to_list*/

EXPORT	void	f_fprint_boundary_state_data(
	FILE		*file,
	INTERFACE	*intfc,
	BOUNDARY_STATE	*bstate)
{
	(void) fprintf(file,"Boundary state:");
	if (bstate->_boundary_state == NULL)
	    (void) fprintf(file," NONE\n");
	else
	{
	    (void) fprintf(file,"\n");
	    fprint_state_data(file,bstate->_boundary_state,intfc);
	}
	(void) fprintf(file,"Boundary state function = ");
	if (bstate->_boundary_state_function == NULL)
	    (void) fprintf(file,"NONE\n");
	else
	    (void) fprintf(file,"%s\n",bstate->_boundary_state_function_name);
}		/*end f_fprint_boundary_state_data*/

EXPORT	void	f_user_fprint_intfc_rect_grids(
	FILE		*file,
	INTERFACE	*intfc)
{
	(void) fprintf(file," \n\t\t\tInterface Computational Grid:\n\n");
	fprint_rectangular_grid(file,computational_grid(intfc));
	(void) fprintf(file,"\n");
}		/*end f_user_fprint_intfc_rect_grids*/

/*ARGSUSED*/
EXPORT	int	f_user_read_print_interface(
	INIT_DATA     *init,
	const IO_TYPE *io_type,
	INTERFACE     *intfc,
	boolean          overlay)
{
	FILE       *file = io_type->file;
	const char *search_string;
	int	   i;

	if (next_output_line_containing_string(file,
	        "Interface normal and tangent operators"))
	{
	    if (fgetstring(file,"Interface normal function = "))
	    {
	        F_USER_INTERFACE *fuh;
	        char s[2048];
	        (void) fgets(s,2046,file);
	        s[strlen(s)-1] = '\0';
	        set_normal_function(s,&interface_normal_function(intfc),intfc);
	        fuh = f_user_hook(intfc->dim);
	        fuh->_interface_normal_function =
	            interface_normal_function(intfc);
	    }
	    if (fgetstring(file,"Interface tangent function = "))
	    {
	        F_USER_INTERFACE *fuh;
	        char s[2048];
	        (void) fgets(s,2046,file);
	        s[strlen(s)-1] = '\0';
	        set_tangent_function(s,&interface_tangent_function(intfc),
		                     intfc);
	        fuh = f_user_hook(intfc->dim);
	        fuh->_interface_tangent_function =
	            interface_tangent_function(intfc);
	    }
	}
	if (size_of_state(intfc) != 0 &&
	    !read_print_boundary_state_data_list(init,io_type,intfc))
	{
	    screen("ERROR in f_user_read_print_interface(), "
	           "can't find End Dirichlet boundary state data\n");
	    return NO;
	}

	search_string = "Excluded Components List";
	if (next_output_line_containing_string(file,search_string))
	{
	    COMPONENT	comp;
	    int		ncomps;

	    (void) fgetstring(file,"Number of excluded components = ");
	    (void) fscanf(file,"%d",&ncomps);
	    for (i = 0; i < ncomps; ++i)
	    {
	    	(void) fscanf(file,"%*s %*s %*d %*s %d",&comp);
	    	exclude_comp(comp,intfc);
	    }
	    search_string = "End Excluded Components List";
	    if (next_output_line_containing_string(file,search_string) == NULL)
	    {
	    	screen("ERROR in f_user_read_print_interface(), "
	    	       "can't find End Excluded Components List\n");
	    	return NO;
	    }
	}
	f_read_print_Dirichlet_bdry_states(init,io_type,intfc);

	return YES;
}		/*end f_user_read_print_interface*/

LOCAL	int	old_style_boundary_data_list;

LOCAL	int	read_print_boundary_state_data_list(
	INIT_DATA     *init,
	const IO_TYPE *io_type,
	INTERFACE     *intfc)
{
	FILE       *file = io_type->file;
	HYPER_SURF **hs;
	char	   s[2048];
	const char *search_string;
	int	   index, nbstates;

	for (hs = hyper_surf_list(intfc); hs && *hs; ++hs)
	    if (wave_type(*hs) == DIRICHLET_BOUNDARY)
		break;

	if (hs == NULL || *hs == NULL) /* No Dirichlet boundaries */
	    return YES;

	search_string = "Boundary state data for interface";
	if (next_output_line_containing_string(file,search_string) == NULL)
	{
	    old_style_boundary_data_list = YES;
	    return old_read_print_boundary_state_data_list(init,io_type,intfc);
	}
	old_style_boundary_data_list = NO;
	(void) fgetstring(file,"num_bstates =");
	(void) fscanf(file,"%d",&nbstates);
	(void) getc(file);/*trailing newline*/
	for (index = 0; index < nbstates; ++index)
	{
	    (void) fgets(s,2046,file);
	    if (s[0] == 'N')
		continue;
	    (*read_print_boundary_state_data(intfc))(init,io_type,intfc,index);
	    (void) fgetstring(file,"End Boundary state data for index");
	    (void) fscanf(file,"%*d");
	    (void) getc(file);/*trailing newline*/
	}
	return YES;
}		/*end read_print_boundary_state_data_list*/

LOCAL	int	old_read_print_boundary_state_data_list(
	INIT_DATA     *init,
	const IO_TYPE *io_type,
	INTERFACE     *intfc)
{
	FILE       *file = io_type->file;
	const char *search_string;
	int	   dim = intfc->dim;
	int	   i, j, index;

	/* Check for restart from old style file, only needed for TWOD */
	if (dim == 2)
	{
	    for (i = 0; i < dim; ++i)
	    {
	        for (j = 0; j < 2; ++j)
	        {
	            if (rect_boundary_type(intfc,i,j) == UNKNOWN_BOUNDARY_TYPE)
	            {
	                set_2d_rect_bdry_types_from_curves(intfc);
	    	        return YES;
	            }
	        }
	    }
	}

	search_string = "Dirichlet rectangular boundary state information ";

	if (next_output_line_containing_string(file,search_string) == NULL)
	{
	    return YES;
	}
	for (i = 0; i < dim; ++i)
	{
	    for (j = 0; j < 2; ++j)
	    {
	        if (rect_boundary_type(intfc,i,j) != DIRICHLET_BOUNDARY)
	            continue;
		index = 2*i + j;
		(void) fgetstring(file,
				  "Rect boundary state information for the ");
		(*read_print_boundary_state_data(intfc))(init,io_type,
		                                         intfc,index);
	    }
	}
	search_string = "End Dirichlet rectangular boundary state information";
	if (next_output_line_containing_string(file,search_string) == NULL)
	{
	    screen("ERROR in old_read_print_boundary_state_data_list(), "
	           "can't find End Dirichlet boundary state data\n");
	    return NO;
	}
	return YES;
}		/*end old_read_print_boundary_state_data_list*/

EXPORT	void	f_read_print_boundary_state_data(
	INIT_DATA     *init,
	const IO_TYPE *io_type,
	INTERFACE     *intfc,
	int	      index)
{
	FILE           *file = io_type->file;
	BOUNDARY_STATE Bstate;
	char	       c, s[120];

	if (old_style_boundary_data_list == YES)
	{
	    old_read_print_boundary_state_data(init,io_type,intfc,index);
	    return;
	}
	Bstate._fprint_boundary_state_data = f_fprint_boundary_state_data;
	Bstate._boundary_state_data = NULL;
	Bstate._boundary_state_function = NULL;
	(void) fgetstring(file,"Boundary state:");
	c = getc(file);
	if (c == '\n')
	{
	    Bstate._boundary_state = read_print_state_data(init,io_type,
	                                                   NULL,intfc);
	}
	else
		Bstate._boundary_state = NULL;

	(void) fgetstring(file,"Boundary state function = ");

	(void) fscanf(file,"%s",s);
	if (strcmp(s,"NONE") == 0 || strcmp(s,"none") == 0 ||
		    strcmp(s,"(NULL)") == 0 || strcmp(s,"(null)") == 0)
	{
	    Bstate._boundary_state_function_name = NULL;
	}
	else
	{
	    uni_array(&Bstate._boundary_state_function_name,strlen(s)+1,
		   sizeof(char));
	    (void) strcpy(Bstate._boundary_state_function_name,s);
	    if (strcmp(Bstate._boundary_state_function_name,
	    				"fixed_boundary_state") == 0)
	    	Bstate._boundary_state_function = fixed_boundary_state;
	}
	(void) add_bstate_to_list(&Bstate,intfc,index);
}		/*end f_read_print_boundary_state_data*/

LOCAL	void	old_read_print_boundary_state_data(
	INIT_DATA     *init,
	const IO_TYPE *io_type,
	INTERFACE     *intfc,
	int	      index)
{
	FILE	  *file = io_type->file;
	BOUNDARY_STATE	Bstate;
	char		s[120];

	Bstate._fprint_boundary_state_data = f_fprint_boundary_state_data;
	Bstate._boundary_state_data = NULL;
	Bstate._boundary_state =
	    read_print_state_data(init,io_type,NULL,intfc);
	(void) fgetstring(file,"Rect boundary state function = ");
	(void) fscanf(file,"%s",s);
	if (strcmp(s,"NULL") == 0 || strcmp(s,"null") == 0 ||
		       strcmp(s,"(NULL)") == 0 || strcmp(s,"(null)") == 0)
	{
	    Bstate._boundary_state_function = NULL;
	    Bstate._boundary_state_function_name = NULL;
	}
	else
	{
	    Bstate._boundary_state_function_name = s;
	    if (strcmp(s,"fixed_boundary_state") == 0)
	    	Bstate._boundary_state_function = fixed_boundary_state;
	}
	(void) add_bstate_to_list(&Bstate,intfc,index);
}		/*end old_read_print_boundary_state_data*/


/*ARGSUSED*/
EXPORT	void f_user_read_print_intfc_rect_grids(
	const IO_TYPE *io_type,
	INTERFACE     *intfc,
	boolean	      oldstyle,
	REMAP     *remap)
{
	if (oldstyle == YES)
	{
	    return;
	}
	else
	{
	    (void) fgetstring(io_type->file,"Interface Computational Grid:");
	    read_rectangular_grid(io_type,computational_grid(intfc),NO,remap);
	}
}		/*end f_user_read_print_intfc_rect_grids*/

/*ARGSUSED*/
EXPORT	void	f_user_make_interface(
	INTERFACE	*intfc)
{
	F_USER_INTERFACE *fuh;
	int		dim;

	dim = intfc->dim;
	fuh = f_user_hook(dim);
	f_user_interface(intfc) = *fuh;
	size_of_state(intfc) = size_of_intfc_state;
	set_size_of_intfc_state(0);
}		/*end f_user_make_interface*/

EXPORT  INTERFACE *f_receive_interface(
        int     src_id)
{
        INTERFACE *intfc = current_interface();

        set_size_of_intfc_state(size_of_state(intfc));
        return i_receive_interface(src_id);
}       /* end f_receive_interface */

EXPORT	FT_POINT *f_Point(
	double		*coords)
{
	FT_POINT		*p;
	INTERFACE	*intfc = current_interface();
	size_t		sizest;

	if ((p = i_Point(coords)) == NULL)
	    return NULL;
	if ((sizest = size_of_state(intfc)) != 0)
	{
	    left_state(p) = alloc_intfc_state(intfc,sizest);
	    right_state(p) = alloc_intfc_state(intfc,sizest);
	    if (right_state(p) == NULL)
	        return NULL;
	}
	else
	{
	    left_state(p)  = NULL;
	    right_state(p) = NULL;
	}
	return p;
}		/*end f_Point*/



EXPORT	FT_POINT *f_Static_point(
	INTERFACE	*intfc)
{
	FT_POINT		*p;
	size_t		sizest;

	p = i_Static_point(intfc);
	if ((sizest = size_of_state(intfc)) != 0)
	{
	    alloc_state(intfc,&left_state(p),sizest);
	    alloc_state(intfc,&right_state(p),sizest);
	    if (right_state(p) == NULL)
	        return NULL;
	}
	return p;
}		/*end f_Static_point*/

EXPORT	FT_POINT *f_copy_point(
	FT_POINT		*p)
{
	FT_POINT		*newp = i_copy_point(p);
	size_t		sizest;
	INTERFACE	*intfc = current_interface();
	int i,dim = intfc->dim;

	if ((copy_intfc_states() == YES) &&
	    ((sizest = size_of_state(intfc)) != 0))
	{
	    ft_assign(left_state(newp),left_state(p),sizest);
	    ft_assign(right_state(newp),right_state(p),sizest);
	}
	for (i = 0; i < dim; ++i)
	{
	    newp->vel[i] = p->vel[i];
	}
	return newp;
}		/*end f_copy_point*/

EXPORT	void f_reconstruct_interface_pointers(
	INTERFACE	*nintfc,
	struct Table	*otbl,
	POINTER		*ocad,
	POINTER		*ncad)
{
	INTERFACE	*ointfc;
	int		nchks;

	nchks = otbl->num_chunks;
	ointfc = (INTERFACE *)new_address(nintfc,otbl->interface,
					  ocad,ncad,nchks);
	copy_rect_grid(computational_grid(nintfc),computational_grid(ointfc));

	i_reconstruct_interface_pointers(nintfc,otbl,ocad,ncad);
	ointfc = otbl->interface;
	first_node(nintfc) =
		(NODE *)new_address(nintfc,first_node(ointfc),ocad,ncad,nchks);
	last_node(nintfc) =
		(NODE *)new_address(nintfc,last_node(ointfc),ocad,ncad,nchks);
	excluded_comps(nintfc).comps =
		(COMPONENT *)new_address(nintfc,excluded_comps(nintfc).comps,
					 ocad,ncad,nchks);
}		/*end f_reconstruct_interface_pointers*/

/*ARGSUSED*/
EXPORT	void f_reconstruct_point_pointers(
	FT_POINT		*p,
	INTERFACE	*nintfc,
	INTERFACE	*ointfc,
	POINTER		*ocad,
	POINTER		*ncad,
	int		nchks)
{
	boolean    recon_states = (size_of_state(nintfc) != 0) ? YES : NO;

	debug_print("pp_mode","Entered f_reconstruct_point_pointers()\n");
	i_reconstruct_point_pointers(p,nintfc,ointfc,ocad,ncad,nchks);
	if (nintfc->dim == 3)
	{
	    if (sorted(p) == YES)
	        recon_states = NO;
	}
	if (recon_states == YES)
	{
	    left_state(p) = (Locstate) new_address(nintfc,left_state(p),
					           ocad,ncad,nchks);
	    right_state(p) = (Locstate) new_address(nintfc,right_state(p),
					            ocad,ncad,nchks);
	}
	debug_print("pp_mode","Left f_reconstruct_point_pointers()\n");
}		/*end f_reconstruct_point_pointers*/

LOCAL	void copy_excluded_comps_lists(
	COMP_LIST	*new_list,
	COMP_LIST	*list)
{
	int		i, ncomps;
	int		max_ncomps;

	*new_list = *list;
	max_ncomps = list->max_ncomps;
	ncomps = list->ncomps;
	if (max_ncomps != 0)
	{
	    COMPONENT *new_comps, *comps;

	    comps = list->comps;
	    new_list->comps = new_comps =
	    	(COMPONENT *) store(max_ncomps*sizeof(COMPONENT));
	    for (i = 0; i < ncomps; ++i)
	    	new_comps[i] = comps[i];
	    for (i = list->ncomps; i < max_ncomps; ++i)
	    	new_comps[i] = ERROR;

	}
	else
	    new_list->comps = NULL;
}		/*end copy_excluded_comps_list*/


/*
*			f_nearest_intfc_state():
*
*	Finds nearest interface state to x,y,comp
*	Upgraded from macro to function to provide a return status. (4/1/89)
*	Upgraded to provide more error checking.		    (4/1/89)
*/

EXPORT	boolean f_nearest_intfc_state(
	double	    *coords,
	COMPONENT    comp,
	INTERFACE   *intfc,
	Locstate     state,
	double	    *coords_on,
	HYPER_SURF **hs_on)
{
	RECT_GRID	*gr;
	HYPER_SURF	*hs;
	HYPER_SURF_ELEMENT *hse;
	double		t[MAXD];
	double		coords_in[MAXD], coords_on_tmp[MAXD];
	double		sep;
	boolean         status;
	int		i, j, dim = intfc->dim;
	int		side;

	DEBUG_ENTER(f_nearest_intfc_state)

	if (debugging("interpolate"))
	{
	    printf("Entering f_nearest_intfc_state()\n");
	}
	if (coords_on == NULL)
	    coords_on = coords_on_tmp;
	gr = computational_grid(intfc);
	for (i = 0; i < dim; ++i)
	{
	    coords_in[i] = coords[i];
	    if (coords[i] < gr->VL[i] || coords[i] > gr->VU[i])
	    	break;
	}
	if (i < dim)
	{
	    side = nearest_boundary(coords,gr);
	    switch (side)
	    {
	    case LEFT_BDRY:	i = 0; j = 0; break;
	    case RIGHT_BDRY:	i = 0; j = 1; break;
	    case LOWER_BDRY:	i = 1; j = 0; break;
	    case UPPER_BDRY:	i = 1; j = 1; break;
	    case ZMIN_BDRY:	i = 2; j = 0; break;
	    case ZMAX_BDRY:	i = 2; j = 1; break;
	    }
	    switch (rect_boundary_type(intfc,i,j))
	    {
	    case SUBDOMAIN_BOUNDARY:
	    case REFLECTION_BOUNDARY:
#if defined(USE_OVERTURE)
            case AMR_SUBDOMAIN_BOUNDARY:
#endif /* if defined(USE_OVERTURE) */
	        for (i = 0; i < dim; ++i)
		    coords_in[i] = coords[i];
	        break;
	    default:
	        nearest_boundary_point(coords,coords_in,gr);
	    }
	}

	if ((status = nearest_interface_point(coords_in,comp,intfc,
					      NO_SUBDOMAIN,NULL,
					      coords_on,t,&hse,&hs)) != YES)
	{
	    (void) printf("WARNING in f_nearest_intfc_state(), "
	                  "nearest_interface_point failed\n");
	    
	    DEBUG_LEAVE(f_nearest_intfc_state)
	    return status;
	}

	if ((hs != NULL) && (hse != NULL) && size_of_state(intfc) != 0)
	{
	    TRI  *tri = Tri_of_hse(hse);
	    SURFACE   **s;
	
	    if(debugging("line_tri"))
	    {
	        printf("nearest\n");
		print_tri(tri, intfc);
	        print_general_vector("t= ", t, 3, "\n");
		printf("#nearest  %d | %p  %p | %d  %d\n", comp, 
			Surface_of_hs(hs), tri->surf,
	            	negative_component(hs), positive_component(hs));
	    }
    
	    /*state_in_tri */
	    state_along_hypersurface_element(comp,t,hse,hs,state);
	
	    if(debugging("line_tri"))
	    {
		printf("#line_tri\n");
	        for(s=intfc->surfaces; s && *s; s++)
	            printf("#s %p    %d  %d\n", *s, positive_component(*s), 
					negative_component(*s));
	        print_general_vector("t= ", t, 3, "\n");
	    }
	}
	if (hs_on != NULL)
	    *hs_on = hs;
	sep = _scaled_separation(coords,coords_on,gr->h,dim);
	if (sep > 2.0 && debugging("not_close"))/*TOLERANCE*/
	{
	    (void) printf("WARNING in f_nearest_intfc_state(), "
	                  "coords_on not close to coords\n");
	    print_general_vector("coords = ",coords,dim,"\n");
	    print_general_vector("coords_on = ",coords_on,dim,"\n");
	    (void) printf("scaled separation = %g\n",sep);
	}
	
	DEBUG_LEAVE(f_nearest_intfc_state)
	return status;
}		/*end f_nearest_intfc_states*/



/* Default front functions for reading and printing wave types */
EXPORT	const char *f_wave_type_as_string(
	int		w_type)
{
	const  char *type;
	switch(w_type) 
	{
	case PASSIVE_BOUNDARY:
	    return "PASSIVE_BOUNDARY";
	case NEUMANN_BOUNDARY:
	    return "NEUMANN_BOUNDARY";
	case DIRICHLET_BOUNDARY:
	    return "DIRICHLET_BOUNDARY";
	case MOVABLE_BODY_BOUNDARY:
	    return "MOVABLE_BODY_BOUNDARY";
	case GROWING_BODY_BOUNDARY:
	    return "GROWING_BODY_BOUNDARY";
	case ELASTIC_BOUNDARY:
	    return "ELASTIC_BOUNDARY";
	case FIRST_PHYSICS_WAVE_TYPE:
	    return "FIRST_PHYSICS_WAVE_TYPE";
	case FIRST_SCALAR_PHYSICS_WAVE_TYPE:
	    return "FIRST_SCALAR_PHYSICS_WAVE_TYPE";
	case FIRST_VECTOR_PHYSICS_WAVE_TYPE:
	    return "FIRST_VECTOR_PHYSICS_WAVE_TYPE";
	case UNKNOWN_WAVE_TYPE:
	    return "UNKNOWN_WAVE_TYPE";
#if defined(USE_OVERTURE)
        case AMR_SUBDOMAIN_BOUNDARY:
            return "AMR_SUBDOMAIN_BOUNDARY";
#endif /* if defined(USE_OVERTURE) */
	default:
	    type = i_boundary_type_as_string(w_type);
	    if (strstr(type," -- ** UNKNOWN boundary type**") != NULL)
	    {
	        static char s[256];
	        (void) sprintf(s,"%d -- ** UNKNOWN wave type**",w_type);
	        return s;
	    }
	    return type;
	}
}		/*end f_wave_type_as_string*/


EXPORT	void f_fprint_wave_type(
	FILE		*file,
	const char	*mesg1,
	int		w_type,
	const char	*mesg2,
	INTERFACE       *intfc)
{
	if (mesg1 != NULL)
	    (void) fprintf(file,"%s",mesg1);
	(void) fprintf(file,"%s",wave_type_as_string(w_type,intfc));
	if (mesg2 != NULL)
	    (void) fprintf(file,"%s",mesg2);
}		/*end f_fprint_wave_type*/

EXPORT	int f_read_wave_type_from_string(
	const char *type)
{
	int i;
	int w_type = UNKNOWN_WAVE_TYPE;
	static struct { const char *name; int type; } wave_type_map[] = {
	    {"PASSIVE_BOUNDARY",   PASSIVE_BOUNDARY},
	    {"PA",                 PASSIVE_BOUNDARY},
	    {"NEUMANN_BOUNDARY",   NEUMANN_BOUNDARY},
	    {"N",                  NEUMANN_BOUNDARY},
	    {"MOVABLE_BODY_BOUNDARY",   MOVABLE_BODY_BOUNDARY},
	    {"M",                  MOVABLE_BODY_BOUNDARY},
	    {"GROWING_BODY_BOUNDARY",   GROWING_BODY_BOUNDARY},
	    {"G",                  GROWING_BODY_BOUNDARY},
	    {"ELASTIC_BOUNDARY",   ELASTIC_BOUNDARY},
	    {"E",                  ELASTIC_BOUNDARY},
	    {"DIRICHLET_BOUNDARY", DIRICHLET_BOUNDARY},
	    {"D",                  DIRICHLET_BOUNDARY},
	    {"FIRST_PHYSICS_WAVE_TYPE", FIRST_PHYSICS_WAVE_TYPE},
	    {"FIRST_SCALAR_PHYSICS_WAVE_TYPE", FIRST_SCALAR_PHYSICS_WAVE_TYPE},
	    {"FIRST_VECTOR_PHYSICS_WAVE_TYPE", FIRST_VECTOR_PHYSICS_WAVE_TYPE},
	    {NULL,                 UNKNOWN_WAVE_TYPE}
	};
	for (i = 0; wave_type_map[i].name != NULL; ++i)
	{
	    if (strcasecmp(type,wave_type_map[i].name) == 0)
	    {
	        w_type = wave_type_map[i].type;
		break;
	    }
	}
	if (w_type != UNKNOWN_WAVE_TYPE)
	    return w_type;

	w_type = i_read_boundary_type_from_string(type);
	return (w_type != UNKNOWN_BOUNDARY_TYPE) ? w_type : UNKNOWN_WAVE_TYPE;
}		/*end f_read_wave_type_from_string*/

/*
*			f_fprint_hsbdry_type():
*
*	 Default front functions for reading and printing hypersurface
*	 boundary types
*/

/*ARGSUSED*/
EXPORT	void f_fprint_hsbdry_type(
	FILE		*file,
	const char	*mesg1,
	int		hsb_type,
	const char	*mesg2,
	INTERFACE	*intfc)
{
	const char *suffix;

	switch (intfc->dim)
	{
	case 3:
	    suffix = "HSBDRY";
	    break;
	case 2:
	    suffix = "NODE";
	    break;
	case 1:
	    screen("ERROR in f_fprint_hsbdry_type(), "
	           "invalid dimension %d\n",intfc->dim);
	           clean_up(ERROR);
	}
	if (mesg1 != NULL)
	    (void) fprintf(file,"%s",mesg1);
	switch(hsb_type) 
	{
	case PASSIVE_HSBDRY:
	    (void) fprintf(file,"PASSIVE_%s",suffix);
	    break;
	case FIXED_HSBDRY:
	    (void) fprintf(file,"FIXED_%s",suffix);
	    break;
	case CLOSED_HSBDRY:
	    (void) fprintf(file,"CLOSED_%s",suffix);
	    break;
	case NEUMANN_HSBDRY:
	    (void) fprintf(file,"NEUMANN_%s",suffix);
	    break;
	case DIRICHLET_HSBDRY:
	    (void) fprintf(file,"DIRICHLET_%s",suffix);
	    break;
	case SOURCE_HSBDRY:
	    (void) fprintf(file,"SOURCE_%s",suffix);
	    break;
	case SINK_HSBDRY:
	    (void) fprintf(file,"SINK_%s",suffix);
	    break;
	case SUBDOMAIN_HSBDRY:
	    (void) fprintf(file,"SUBDOMAIN_%s",suffix);
	    break;
	case MONO_COMP_HSBDRY:
	    (void) fprintf(file,"MONO_COMP_%s",suffix);
	    break;
	case UNKNOWN_HSBDRY_TYPE:
	    (void) fprintf(file,"UNKNOWN_HSBDRY_TYPE\t\t");
	    break;
	default:
	    (void) fprintf(file,"ERROR\t\t");
	    break;
	}
	if (mesg2 != NULL)
	    (void) fprintf(file,"%s",mesg2);
}		/*end f_fprint_hsbdry_type*/

/*ARGSUSED*/
EXPORT	int f_read_hsbdry_type_from_string(
	const char *type,
	INTERFACE  *intfc)
{
	int		hsb_type = UNKNOWN_HSBDRY_TYPE;

	switch(type[0]) 
	{
	case 'P':
	    if (type[1] == 'A')
	    	hsb_type = PASSIVE_HSBDRY;
	    else if (type[1] == 'E')
	    	hsb_type = SUBDOMAIN_HSBDRY; /* Old style */
	    break;
	case 'F':
	    hsb_type = FIXED_HSBDRY;
	    break;
	case 'N':
	    switch (type[8])
	    {
	    case 'H':
	    case 'N':		/* NEUMANN_NODE */
	        hsb_type = NEUMANN_HSBDRY;
	        break;
	    case 'C':
	        hsb_type = NEUMANN_CURVE;
	        break;
	    default:
	        break;
	    }
	    break;
	case 'M':
	    hsb_type = MONO_COMP_HSBDRY;
	    break;
	case 'C':
	    hsb_type = CLOSED_HSBDRY;
	    break;
	case 'D':
	    hsb_type = DIRICHLET_HSBDRY;
	    break;
	case 'S':
	    switch (type[1])
	    {
	    case '0':
	    	hsb_type = SOURCE_HSBDRY;
	    	break;
	    case 'I':
	    	hsb_type = SINK_HSBDRY;
	    	break;
	    case 'U':
	    	hsb_type = SUBDOMAIN_HSBDRY;
	    	break;
	    default:
	    	break;
	    }
	    break;
	case 'U':
	case 'u':
	    hsb_type = UNKNOWN_HSBDRY_TYPE;
	    break;
	default:
	    break;
	}
	if (hsb_type == UNKNOWN_HSBDRY_TYPE)
	{
	    screen("ERROR in f_read_hsbdry_type_from_string() "
	           "unknown hypersurface boundary type %s\n",type);
	    clean_up(ERROR);
	}
	return hsb_type;
}		/*end f_read_hsbdry_type_from_string*/

EXPORT	void	f_user_copy_hyper_surf(
	HYPER_SURF	*new_hs,
	HYPER_SURF	*old_hs)
{
	wave_type(new_hs) = wave_type(old_hs);
	Hs_flag(new_hs) = Hs_flag(old_hs);
	bstate_index(new_hs) = bstate_index(old_hs);
	redistribution_direction(new_hs) = redistribution_direction(old_hs);
	perform_redistribution_function(new_hs) =
	    perform_redistribution_function(old_hs);
	hypersurface_normal_function(new_hs) =
	    hypersurface_normal_function(old_hs);
	if (set_copy_from_to_lists)
	{
	    hs_copied_from(new_hs) = old_hs;
	    hs_copied_to(old_hs) = new_hs;
	}
	if (wave_type(old_hs) == MOVABLE_BODY_BOUNDARY)
	{
	    int i;
	    body_index(new_hs) = body_index(old_hs);
            mom_inertial(new_hs) = mom_inertial(old_hs);
            total_mass(new_hs) = total_mass(old_hs);
            angular_velo(new_hs) = angular_velo(old_hs);
            motion_type(new_hs) = motion_type(old_hs);
            for (i = 0; i < old_hs->interface->dim; ++i)
            {
                center_of_mass(new_hs)[i] = center_of_mass(old_hs)[i];
                center_of_mass_velo(new_hs)[i] =
                                center_of_mass_velo(old_hs)[i];
                rotation_direction(new_hs)[i] = rotation_direction(old_hs)[i];
                rotation_center(new_hs)[i] = rotation_center(old_hs)[i];
            }
	}
}		/*end f_user_copy_hyper_surf*/

/*
*		fixed_boundary_state():
*	
*	Sets the state to the boundary_state of the given hypersurface
*	to the value of the fixed state boundary_state(hs).
*
*/

/*ARGSUSED*/
EXPORT	void fixed_boundary_state(
	double		*coords,
	HYPER_SURF	*hs,
	Front		*front,
	POINTER		wave,
	Locstate	state)
{
	ft_assign(state,boundary_state(hs),front->sizest);
}		/*end fixed_boundary_state*/


/*ARGSUSED*/
EXPORT	void f_reconstruct_node_pointers(
	NODE		*n,
	INTERFACE	*nintfc,
	INTERFACE	*ointfc,
	POINTER		*ocad,
	POINTER		*ncad,
	int		nchks)
{
	debug_print("pp_mode","Entered f_reconstruct_node_pointers()\n");
	i_reconstruct_node_pointers(n,nintfc,ointfc,ocad,ncad,nchks);
	prev_node(n) = (NODE *)new_address(nintfc,prev_node(n),
					   ocad,ncad,nchks);
	next_node(n) = (NODE *)new_address(nintfc,next_node(n),
					   ocad,ncad,nchks);
	/* Node correspondence not supported */
	if (ointfc->dim == 2)
		correspond_hyper_surf_bdry(n) = NULL;
	debug_print("pp_mode","Left f_reconstruct_node_pointers()\n");
}		/*end f_reconstruct_node_pointers*/

EXPORT	void f_reconstruct_curve_pointers(
	CURVE		*c,
	INTERFACE	*nintfc,
	INTERFACE	*ointfc,
	POINTER		*ocad,
	POINTER		*ncad,
	int		nchks)
{									     
	i_reconstruct_curve_pointers(c,nintfc,ointfc,ocad,ncad,nchks);
	switch (nintfc->dim)
	{
	case 2:
	    f_user_2d_reconstruct_curve_pointers(c,nintfc,ointfc,
						 ocad,ncad,nchks);
	    break;
	case 3:
	    break;/*TODO*/
	}
}		/*end f_reconstruct_curve_pointers*/

/*ARGSUSED*/
EXPORT	void f_reconstruct_bond_pointers(
	BOND		*b,
	INTERFACE	*nintfc,
	INTERFACE	*ointfc,
	POINTER		*ocad,
	POINTER		*ncad,
	int		nchks)
{
	BOND_TRI	**btris;

	i_reconstruct_bond_pointers(b,nintfc,ointfc,ocad,ncad,nchks);


	if (size_of_state(nintfc) == 0)
	    return;

	/*for (btris = Btris(b); btris && *btris; ++btris) */
	/*{ */
	/*    printf("%d  %d |", left_start_btri_state(*btris), right_start_btri_state(*btris)); */
	/*} */
	/*printf("\n"); */

	/*see i_send_interface for consistence */
	for (btris = Btris(b); btris && *btris; ++btris)
	{
	    left_start_btri_state(*btris) =
	    	(Locstate) new_address(nintfc,left_start_btri_state(*btris),
	    			       ocad,ncad,nchks);
	    right_start_btri_state(*btris) =
	    	(Locstate) new_address(nintfc,right_start_btri_state(*btris),
	    			       ocad,ncad,nchks);
	    left_end_btri_state(*btris) =
	    	(Locstate) new_address(nintfc,left_end_btri_state(*btris),
	    			       ocad,ncad,nchks);
	    right_end_btri_state(*btris) =
	    	(Locstate) new_address(nintfc,right_end_btri_state(*btris),
	    			       ocad,ncad,nchks);
	}

}		/*end f_reconstruct_bond_pointers*/

EXPORT	boolean f_insert_point_in_bond(
	FT_POINT		*p,
	BOND		*b,
	CURVE		*c)
{
	boolean status = FUNCTION_FAILED;
	if (i_insert_point_in_bond(p,b,c) != FUNCTION_SUCCEEDED)
	    return status;
	switch (c->interface->dim)
	{
	case 2:
	    status = f_user_2d_insert_point_in_bond(p,b,c);
	    break;
	case 3:
	    status = f_user_3d_insert_point_in_bond(b);
	    break;
	}
	return status;
}		/*end f_insert_point_in_bond*/


EXPORT	void f_user_read_curve(
	CURVE		*curve)
{
	INTERFACE	*intfc = curve->interface;
	char		type[120];

	switch (intfc->dim)
	{
	case 1:
	    break;
	case 2:
	    if (is_mono_comp_curve(curve))
	        mono_comp_curves(intfc) = YES;
	    screen("Enter wave type for curve: ");
	    (void) Scanf("%s\n",type);
	    wave_type(curve) = read_wave_type_from_string(type,intfc);
	    break;
	case 3:
	    screen("Enter the hypersurface boundary type for curve: ");
	    (void) Scanf("%s\n",type);
	    hsbdry_type(curve) = read_hsbdry_type_from_string(type,intfc);
	    break;
	}
}		/*end f_user_read_curve*/

/*ARGSUSED*/
EXPORT	boolean f_user_read_print_curve(
	CURVE	      *curve,
	const IO_TYPE *io_type,
	boolean          overlay)
{
	INTERFACE *intfc = curve->interface;
	FILE      *file = io_type->file;
	char	  type[120];
	int	  i;

	switch (intfc->dim)
	{
	case 1:
	    break;
	case 2:
	    if (is_mono_comp_curve(curve))
		mono_comp_curves(intfc) = YES;
	    if (!fgetstring(file,"curve->wave_type = "))
	    {
	        (void) printf("WARNING in f_user_read_print_curve(), "
		              "can't find wave_type\n");
		return NO;
	    }
	    if (fscanf(file,"%s",type) != 1)
	    {
	        (void) printf("WARNING in f_user_read_print_curve(), "
		              "can't read wave type\n");
		return NO;
	    }
	    wave_type(curve) = read_wave_type_from_string(type,intfc);
	    (void) fgetstring(file,"Hs_flag(curve) = "); 

            /* FIXME. Implementation of boolean is compiler dependent. */
            (void) fscanf(file,"%d",&Hs_flag(curve));
	    if (fgetstring(file,"redistribution_direction = ") ==
		FUNCTION_SUCCEEDED)
	    {
	        if (fscanf(file,"%s",type) != 1)
		{
	            (void) printf("WARNING in f_user_read_print_curve(), "
		                  "can't read redistribution direction\n");
		    return NO;
		}
		redistribution_direction(curve) =
		    read_redistribution_direction_from_string(type);
	    }
	    if (fgetstring(file,"Specialized curve normal function = "))
	    {
	        char s[2048];
	        (void) fgets(s,2046,file);
	        s[strlen(s)-1] = '\0';
	        set_normal_function(s,&hypersurface_normal_function(curve),
		                    intfc);
	    }
	    if (wave_type(curve) == MOVABLE_BODY_BOUNDARY)
            {
                fgetstring(file,"Body index = ");
            	(void) fscanf(file,"%d",&body_index(Hyper_surf(curve)));
                total_mass(Hyper_surf(curve)) =
                        read_print_float("Total mass = ",1.0,io_type);
                mom_inertial(Hyper_surf(curve)) =
                        read_print_float("Moment of inertial = ",1.0,io_type);
                fgetstring(file,"Center of Mass = ");
	    	for (i = 0; i < 2; ++i)
                    center_of_mass(Hyper_surf(curve))[i] =
                            fread_float(NULL,io_type);
                fgetstring(file,"Center of mass velocity = ");
	    	for (i = 0; i < 2; ++i)
		    center_of_mass_velo(Hyper_surf(curve))[i] =
                            fread_float(NULL,io_type);
	    	fgetstring(file,"Rotation center = ");
	    	for (i = 0; i < 2; ++i)
            	    rotation_center(Hyper_surf(curve))[i] =
                            fread_float(NULL,io_type);
                angular_velo(Hyper_surf(curve)) =
                            read_print_float("Angular velocity = ",1.0,io_type);
                fgetstring(file,"Motion type = ");
                /* FIXME: the size of enum is compiler dependent */
            	(void) fscanf(file,"%d",&motion_type(Hyper_surf(curve)));
	    }
	    break;
	case 3:
	    if (!fgetstring(file,"Hypersurface boundary type = "))
	    {
	        (void) printf("WARNING in f_user_read_print_curve(), "
		              "can't find Hypersurface boundary type\n");
		return NO;
	    }
	    if (fscanf(file,"%s",type) != 1)
	    {
	        (void) printf("WARNING in f_user_read_print_curve(), "
		              "can't read Hypersurface boundary type\n");
		return NO;
	    }
	    hsbdry_type(curve) = read_hsbdry_type_from_string(type,intfc);
	    break;
	}
	if (fgetstring(file,"Specialized curve tangent function = "))
	{
	    char s[2048];
	    (void) fgets(s,2046,file);
	    s[strlen(s)-1] = '\0';
	    set_tangent_function(s,&curve_tangent_function(curve),intfc);
	}
	return YES;
}		/*end f_user_read_print_curve*/

EXPORT	int f_user_read_node(
	NODE		*node)
{
	char		type[120];
	static int	depth = 0;

	if (node->interface->dim == 3)
	    return YES;/*nothing to do */
	screen("Enter node type: ");
	(void) Scanf("%s\n",type);
	node_type(node) = read_node_type_from_string(type,node->interface);
	if (node_type(node) == UNKNOWN_NODE_TYPE)
	{
	    if (depth++ < 5)
	    {
	    	(void) printf("WARNING in f_user_read_node(), "
			      "Invalid node type, enter another value\n");
	    	return f_user_read_node(node);
	    }
	    return NO;
	}
	depth = 0;
	return YES;
}		/*end f_user_read_node*/

/*ARGSUSED*/
EXPORT	void f_user_read_print_node(
	NODE          *node,
	const IO_TYPE *io_type,
	boolean          overlay)
{
	FILE *file = io_type->file;
	char type[120];
	int  dim = node->interface->dim;

	if (dim == 2)
	{
	    (void) fgetstring(file,"node->type = ");
	    (void) fscanf(file,"%s",type);        
	    node_type(node) = read_node_type_from_string(type,node->interface);
	}
}		/*end f_user_read_print_node*/

EXPORT	boolean f_delete_start_of_bond(
	BOND		*b,
	CURVE		*c)
{
	boolean status;

	if (c->interface->dim == 3)
	{
	    BOND_TRI **bt, **bpt;
	    for (bt = Btris(b), bpt = Btris(b->prev); bt && *bt; ++bt, ++bpt)
	    {
	    	left_end_btri_state(*bpt) = left_end_btri_state(*bt);
	    	right_end_btri_state(*bpt) = right_end_btri_state(*bt);
	    }
	}
	status = i_delete_start_of_bond(b,c);
	return status;
}		/*end f_delete_start_of_bond*/

EXPORT	boolean f_delete_end_of_bond(
	BOND		*b,
	CURVE		*c)
{
	boolean status;
	if (c->interface->dim == 3)
	{
	    BOND_TRI **bt, **bnt;
	    for (bt = Btris(b), bnt = Btris(b->next); bt && *bt; ++bt, ++bnt)
	    {
	        left_start_btri_state(*bnt) = left_start_btri_state(*bt);
	        right_start_btri_state(*bnt) = right_start_btri_state(*bt);
	    }
	}
	status = i_delete_end_of_bond(b,c);
	return status; 
}		/*end f_delete_end_of_bond*/

/*
*		set_interpolate_states_at_split_curve_node():
*
*/

LOCAL boolean	SplitCurveNodeStates = YES;

EXPORT void set_interpolate_states_at_split_curve_node(boolean yes_no)
{
	SplitCurveNodeStates = yes_no;
}		/*end set_interpolate_states_at_split_curve_node*/

EXPORT boolean interpolate_states_at_split_curve_node(void)
{
	return SplitCurveNodeStates;
}		/*end interpolate_states_at_split_curve_node*/

/*ARGSUSED*/
EXPORT	boolean f_user_split_curve(
	int		is_a_node,
	FT_POINT		*p,
	BOND		*bond,
	CURVE		*curve,
	CURVE		**curves)
{
	boolean status = NO;

	switch (curve->interface->dim)
	{
	case 2:
	    status = f_user_2d_split_curve(is_a_node,p,bond,curve,curves);
	    break;
	case 3:
	    hsbdry_type(curves[0]) = hsbdry_type(curve);
	    hsbdry_type(curves[1]) = hsbdry_type(curve);
	    status = YES;
	    break;
	}
	curve_tangent_function(curves[0]) = curve_tangent_function(curve);
	curve_tangent_function(curves[1]) = curve_tangent_function(curve);
	return status;
}		/*end f_user_split_curve*/


EXPORT	boolean f_user_join_curves(
	CURVE		*curve,
	CURVE		*curve1,
	CURVE		*curve2)
{
	boolean status = NO;

	switch (curve->interface->dim)
	{
	case 2: 
	    status = f_user_2d_join_curves(curve,curve1,curve2);
	    start_status(curve) = start_status(curve1);
	    end_status(curve) = end_status(curve2);
	    break;
	case 3:
	    status = f_user_3d_join_curves(curve,curve1,curve2);
	    break;
	}
	if (curve_tangent(curve1) == curve_tangent(curve2))
	    curve_tangent_function(curve) = curve_tangent_function(curve1);
	return status;
}		/*end f_user_join_curves*/

EXPORT	void f_user_fprint_curve(
	FILE		*file,
	CURVE		*curve)
{
	switch (curve->interface->dim) 
	{ 
	case 1: 
	    return; 
	case 2:
	    fprint_wave_type(file,"\n\tcurve->wave_type = ",
			     wave_type(curve),"\n",curve->interface);
            /* FIXME. Implementation of boolean is compiler dependent. */
	    fprintf(file,"\tHs_flag(curve) = %d\n",Hs_flag(curve));
	    fprint_redistribution_direction(file,
					    "\n\tredistribution_direction = ",
					    redistribution_direction(curve),
					    "\n");
	    (void) fprintf(file,"\tSpecialized curve normal function = ");
	    if (hypersurface_normal(curve))
	        (void) fprintf(file,"%s\n",hypersurface_normal_name(curve));
	    else
	        (void) fprintf(file,"none\n");
	    if (wave_type(curve) == MOVABLE_BODY_BOUNDARY)
            {
                (void) fprintf(file,"\tBody index = ");
                (void) fprintf(file,"%d\n",body_index(Hyper_surf(curve)));
                (void) fprintf(file,"\tTotal mass = ");
                (void) fprintf(file,"%"FFMT"\n",total_mass(Hyper_surf(curve)));
                (void) fprintf(file,"\tMoment of inertial = ");
                (void) fprintf(file,"%"FFMT"\n",
                                mom_inertial(Hyper_surf(curve)));
                (void) fprintf(file,"\tCenter of Mass = ");
                (void) fprintf(file,"%"FFMT" %"FFMT"\n",
                                center_of_mass(Hyper_surf(curve))[0],
                                center_of_mass(Hyper_surf(curve))[1]);
                (void) fprintf(file,"\tCenter of mass velocity = ");
                (void) fprintf(file,"%"FFMT" %"FFMT"\n",
                                center_of_mass_velo(Hyper_surf(curve))[0],
                                center_of_mass_velo(Hyper_surf(curve))[1]);
                (void) fprintf(file,"\tRotation center = ");
                (void) fprintf(file,"%"FFMT" %"FFMT"\n",
                                rotation_center(Hyper_surf(curve))[0],
                                rotation_center(Hyper_surf(curve))[1]);
                (void) fprintf(file,"\tAngular velocity = ");
                (void) fprintf(file,"%"FFMT"\n",
                                angular_velo(Hyper_surf(curve)));
                (void) fprintf(file,"\tMotion type = %d\n",
                               motion_type(Hyper_surf(curve)));
	    }
	    break;
	case 3:
	    fprint_hsbdry_type(file,"\n\tHypersurface boundary type = ",
			       hsbdry_type(curve),"\n",curve->interface);
	    break; 
	}
	(void) fprintf(file,"\tSpecialized curve tangent function = ");
	if (curve_tangent_name(curve))
	    (void) fprintf(file,"%s\n",curve_tangent_name(curve));
	else
	    (void) fprintf(file,"none\n");
}		/*end f_user_fprint_curve*/

EXPORT	void f_user_fprint_node(
	FILE		*file,
	NODE		*node)
{
	if (node->interface->dim == 2)
	    fprint_node_type(file,"\t\tnode->type = ",node_type(node),"\n",
	    		     node->interface);
}		/*end f_user_fprint_node*/

EXPORT HYPER_SURF *f_make_hypersurface(
	COMPONENT	neg_comp,
	COMPONENT	pos_comp)
{
	HYPER_SURF	*hs = i_make_hypersurface(neg_comp,pos_comp);

	wave_type(hs) = ERROR;
	/* Defaults for Hs_flag(hs) */
	do_not_redistribute(hs)                 = NO;
	never_redistribute(hs)                  = NO;
	untracked_hyper_surf(hs)                = NO;
	redistribute_by_time_step_frequency(hs) = YES;
	redistribute_hyper_surface(hs)          = NO;
	perform_redistribution_function(hs) =
	    default_perform_redistribution_function(hs->interface);
	redistribution_direction(hs) = FORWARD_REDISTRIBUTION;
	hypersurface_normal(hs) = NULL;
	hypersurface_normal_name(hs) = NULL;
	hs_copied_from(hs) = NULL;
	hs_copied_to(hs) = NULL;

	return hs;
}		/*end f_make_hypersurface*/

EXPORT	HYPER_SURF_BDRY	*f_make_hypersurface_boundary(void)
{
	HYPER_SURF_BDRY	*hsb = i_make_hypersurface_boundary();

	hsbdry_type(hsb) = ERROR;
	hsb_copied_from(hsb) = NULL;
	hsb_copied_to(hsb) = NULL;
	return hsb;
}		/*end f_make_hypersurface_boundary*/


EXPORT CURVE *f_make_curve(
	COMPONENT	left_c,
	COMPONENT	right_c,
	NODE		*start,
	NODE		*end)
{
	CURVE		*curve;

	curve = i_make_curve(left_c,right_c,start,end);

	if (curve == NULL) 
	    return NULL;
	switch (curve->interface->dim)
	{
	case 2:
	    curve = f_user_2d_make_curve(curve);
	    break;
	case 3:
	    break;
	}
	curve_tangent(curve) = NULL;
	curve_tangent_name(curve) = NULL;
	return curve;
}		/*end f_make_curve*/



EXPORT	CURVE	*f_copy_curve(
	CURVE		*curve,
	NODE		*start,
	NODE		*end)
{
	CURVE		*new_curve;
	INTERFACE	*intfc;
        int i;

	new_curve = i_copy_curve(curve, start, end);

	if (curve == NULL)
	    return NULL;
	intfc = new_curve->interface;
	if (intfc->dim == 2)
	{
	    if ((copy_intfc_states() == YES) && (size_of_state(intfc) != 0))
	    {
	        size_t	sizest = size_of_state(intfc);
	    	ft_assign(left_start_state(new_curve),
		       left_start_state(curve),sizest);
	    	ft_assign(right_start_state(new_curve),
	    	       right_start_state(curve),sizest);
	    	ft_assign(left_end_state(new_curve),
	    	       left_end_state(curve),sizest);
	    	ft_assign(right_end_state(new_curve),
	    	       right_end_state(curve),sizest);
	    }
	    if (is_mono_comp_curve(new_curve))
	    	mono_comp_curves(new_curve->interface) = YES;
            start_status(new_curve) = start_status(curve);
            end_status(new_curve)   = end_status(curve);
	}
	if (intfc->dim == 3)
	{
	    hsbdry_type(new_curve) = hsbdry_type(curve);
	    if (set_copy_from_to_lists)
	    {
	        HYPER_SURF_BDRY *old_hsb, *new_hsb;
	        old_hsb = Hyper_surf_bdry(curve);
	        new_hsb = Hyper_surf_bdry(new_curve);
	        hsb_copied_from(new_hsb) = old_hsb;
	        hsb_copied_to(old_hsb) = new_hsb;
	    }
	}
	curve_tangent_function(new_curve) = curve_tangent_function(curve);
	return new_curve;
}		/*end f_copy_curve*/


EXPORT	int	f_delete_curve(
	CURVE		*curve)
{
	int		status = 1;

	switch (curve->interface->dim)
	{
	case 2:
	    status = f_user_2d_delete_curve(curve);
	    break;
	case 3:
	    break;/*TODO*/
	}
	status *= i_delete_curve(curve);
	return status;
}		/*end f_delete_curve*/

EXPORT	NODE	*f_make_node(
	FT_POINT		*posn)
{
	NODE		*newnod;
	int		i, dim;

	newnod = i_make_node(posn);
	if (newnod == NULL)
	    return NULL;
	dim = newnod->interface->dim;
	prev_node(newnod) = NULL;
	next_node(newnod) = NULL;
	if (dim == 2)
	    correspond_hyper_surf_bdry(newnod) = NULL;
	propagation_status(newnod) = PROPAGATION_STATUS_UNSET;
	preserve_position_as_point(newnod) = NO;
	for (i = 0; i < dim; ++i)
	    Node_vel(newnod)[i] = 0.0;

	return newnod;
}		/*end f_make_node*/

EXPORT	NODE	*f_copy_node(
	NODE		*old_node)
{
	NODE		*new_node;
	int		i, dim;

	if (old_node == NULL)
	    return NULL;

	if ((new_node = i_copy_node(old_node)) == NULL)
	    return NULL;

	dim = old_node->interface->dim;

	for (i = 0; i < dim; ++i)
	    Node_vel(new_node)[i] = Node_vel(old_node)[i];
	if (dim == 2)
	{
	    node_type(new_node) = node_type(old_node);
	    if (set_copy_from_to_lists)
	    {
	        HYPER_SURF_BDRY *old_hsb, *new_hsb;
	        old_hsb = Hyper_surf_bdry(old_node);
	        new_hsb = Hyper_surf_bdry(new_node);
	        hsb_copied_from(new_hsb) = old_hsb;
	        hsb_copied_to(old_hsb) = new_hsb;
	    }
	}
	preserve_position_as_point(new_node) =
	    preserve_position_as_point(old_node);
	return new_node;
}		/*end f_copy_node*/

EXPORT	boolean	f_delete_node(
	NODE		*node)
{
	INTERFACE	*intfc = node->interface;
	boolean    	status;

	if (first_node(intfc) == node)
	    first_node(intfc) = next_node(node);
	if (last_node(intfc) == node)
	    last_node(intfc) = prev_node(node);
	if (prev_node(node))
	    next_node(prev_node(node)) = next_node(node);
	if (next_node(node))
	    prev_node(next_node(node)) = prev_node(node);
	if (intfc->dim == 2)
	{
	    if (correspond_hyper_surf_bdry(node))
	    	correspond_hyper_surf_bdry(correspond_hyper_surf_bdry(node)) =
		    NULL;
	    correspond_hyper_surf_bdry(node) = NULL;
	}
	prev_node(node) = next_node(node) = NULL;
	propagation_status(node) = DELETED_NODE;
	status = i_delete_node(node);
	return status;
}		/*end f_delete_node*/

EXPORT	FT_POINT *f_make_point(
	double		*coords,
	COMPONENT	left_c,
	COMPONENT	right_c)
{
	FT_POINT		*p;

	if ((p = i_make_point(coords,left_c,right_c)) == NULL)
	    return NULL;

	bstate_index(p) = -1;
	return p;
}		/*end f_make_point*/


LOCAL	INTERFACE *f_user_2d_copy_interface(
	INTERFACE	*new_intfc,
	INTERFACE	*intfc)
{
	NODE		**oldn, **newn;

	mono_comp_curves(new_intfc) = mono_comp_curves(intfc);

	if (!set_correspondence_between_interfaces(intfc,new_intfc))
	    return NULL;
	first_node(new_intfc) = (first_node(intfc) != NULL) ?
	                        correspond_node(first_node(intfc)) : NULL;
	last_node(new_intfc) = (last_node(intfc) != NULL) ?
	                       correspond_node(last_node(intfc)) : NULL;
	oldn = intfc->nodes;	newn = new_intfc->nodes;
	for (;oldn && *oldn; ++oldn, ++newn)
	{
	    prev_node(*newn) = (prev_node(*oldn) != NULL) ?
	    	               correspond_node(prev_node(*oldn)) : NULL;
	    next_node(*newn) = (next_node(*oldn) != NULL) ?
	    	               correspond_node(next_node(*oldn)) : NULL;
	}
	set_correspond_hyper_surf_bdrys_to_NULL(intfc);
	set_correspond_hyper_surf_bdrys_to_NULL(new_intfc);

	return new_intfc;
}		/*end f_user_2d_copy_interface */


LOCAL	CURVE	*f_user_2d_make_curve(
	CURVE		*curve)
{
	size_t		sizest;
	INTERFACE	*intfc = curve->interface;

	if ((sizest = size_of_state(intfc)) != 0)
	{
	    left_start_state(curve) = alloc_intfc_state(intfc,sizest);
	    left_end_state(curve) = alloc_intfc_state(intfc,sizest);
	    right_start_state(curve) = alloc_intfc_state(intfc,sizest);
	    right_end_state(curve) = alloc_intfc_state(intfc,sizest);
	    if (right_end_state(curve) == NULL)
	        return NULL;
	}
	else 
	{
	    left_start_state(curve) = NULL;
	    left_end_state(curve) = NULL;
	    right_start_state(curve) = NULL;
	    right_end_state(curve) = NULL;
	}
	if (!rst_cor_after_make_hyper_surf(Hyper_surf(curve)))
	    return NULL;
	bstate_index(curve) = -1;
	if (is_mono_comp_curve(curve))
	    mono_comp_curves(curve->interface) = YES;
	wave_type(curve) = UNKNOWN_WAVE_TYPE;
	return curve;
}		/*end f_user_2d_make_curve*/

LOCAL	int	f_user_2d_delete_curve(
	CURVE		*curve)
{
	if (!rst_cor_after_delete_hyper_surf(Hyper_surf(curve)))
	    return 0;
	test_for_mono_comp_curves(curve->interface);
	return 1;
}		/*end f_user_2d_delete_curve*/

/*ARGSUSED*/
LOCAL	boolean f_user_2d_split_curve(
	int		is_a_node,
	FT_POINT		*p,
	BOND		*bond,
	CURVE		*curve,
	CURVE		**curves)
{
	INTERFACE	*cur_intfc = current_interface();
	size_t		sizest = size_of_state(cur_intfc);
	double		length,alpha;

	if ((interpolate_intfc_states(cur_intfc) ||
	     (copy_intfc_states() == YES)) && (sizest != 0))
	{
	    if (interpolate_states_at_split_curve_node() == YES)
	    {
		alpha = bond_length(bond);
	        length = alpha + bond_length(bond->next);
	        if (length < EPSILON)
		    alpha = 0.5;
	        else
		    alpha /= length;
	        bi_interpolate_intfc_states(cur_intfc,alpha,1.-alpha,
	                Coords(bond->start),
	            	left_state_at_point_on_curve(bond->start,bond,curve),
	            	Coords(bond->next->end),
	            	left_state_at_point_on_curve(bond->next->end,
	            	                             bond->next,curve),
	            	left_state(p));
	        bi_interpolate_intfc_states(cur_intfc,
	            	alpha,1.-alpha,
	            	Coords(bond->start),
	            	right_state_at_point_on_curve(bond->start,bond,curve),
	            	Coords(bond->next->end),
	            	right_state_at_point_on_curve(bond->next->end,
	            	                              bond->next,curve),
	            	right_state(p));
	    }
	    else
	    {
	        ft_assign(left_end_state(curves[0]),left_state(p),sizest);
	        ft_assign(right_end_state(curves[0]),right_state(p),sizest);
	        ft_assign(left_start_state(curves[1]),left_state(p),sizest);
	        ft_assign(right_start_state(curves[1]),right_state(p),sizest);
	    }
	}
	bstate_index(curves[0]) = bstate_index(curve);
	bstate_index(curves[1]) = bstate_index(curve);
	wave_type(curves[0]) = wave_type(curve);
	wave_type(curves[1]) = wave_type(curve);
	hypersurface_normal_function(curves[0]) =
	    hypersurface_normal_function(curve);
	hypersurface_normal_function(curves[1]) =
	    hypersurface_normal_function(curve);
	redistribution_direction(curves[0]) = redistribution_direction(curve);
	redistribution_direction(curves[1]) = redistribution_direction(curve);
	if (!rst_cor_after_split_curve(curve,curves))
	    return NO;
	if (is_mono_comp_curve(curves[0]) || is_mono_comp_curve(curves[1]))
	    mono_comp_curves(curves[0]->interface) = YES;
	return YES;
}		/*end f_user_2d_split_curve*/

/*ARGSUSED*/
LOCAL	void f_user_2d_reconstruct_curve_pointers(
	CURVE		*c,
	INTERFACE	*nintfc,
	INTERFACE	*ointfc,
	POINTER		*ocad,
	POINTER		*ncad,
	int		nchks)
{
	if (size_of_state(nintfc) != 0)					     
	{								     
	    left_start_state(c) =					     
	    	(Locstate)new_address(nintfc,left_start_state(c),
	    			      ocad,ncad,nchks);    
	    right_start_state(c) =					     
	    	(Locstate)new_address(nintfc,right_start_state(c),
	    			      ocad,ncad,nchks);    
	    left_end_state(c) =					     
	    	(Locstate)new_address(nintfc,left_end_state(c),
	    			      ocad,ncad,nchks);    
	    right_end_state(c) =					     
	    	(Locstate)new_address(nintfc,right_end_state(c),
	    			      ocad,ncad,nchks);    
	}								     
	/* Curve correspondence not supported */  
	correspond_hyper_surf(c) = NULL;
}		/*end f_user_2d_reconstruct_curve_pointers*/

/*ARGSUSED*/
LOCAL	boolean f_user_2d_insert_point_in_bond(
	FT_POINT		*p,
	BOND		*b,
	CURVE		*c)
{
	BOND		*bnew;
	Locstate	start_left_state,start_right_state;
	Locstate	end_left_state,end_right_state;
	double		len,total_len,para;
	INTERFACE	*intfc;

	bnew = b->next;
	intfc = current_interface();
	if (interpolate_intfc_states(intfc) && size_of_state(intfc) != 0)
	{
	    if (b->prev == NULL)
	    {
	    	start_left_state = left_start_state(c);
	    	start_right_state = right_start_state(c);
	    }
	    else
	    {
	    	start_left_state = left_state(b->start);
	    	start_right_state = right_state(b->start);
	    }
	    if (bnew->next == NULL)
	    {
	    	end_left_state = left_end_state(c);
	    	end_right_state = right_end_state(c);
	    }
	    else
	    {
	    	end_left_state = left_state(bnew->end);
	    	end_right_state = right_state(bnew->end);
	    }
	    len = bond_length(b);
	    total_len = len + bond_length(bnew);
	    para = (total_len > 0.0) ? len/total_len : 0.5;
	    bi_interpolate_intfc_states(intfc,1.0-para,para,
			                Coords(b->start),start_left_state,
			                Coords(bnew->end),end_left_state,
			                left_state(b->end));
	    bi_interpolate_intfc_states(intfc,1.0-para,para,
			                Coords(b->start),start_right_state,
			                Coords(bnew->end),end_right_state,
			                right_state(b->end));
	}
	return FUNCTION_SUCCEEDED;
}		/*end f_user_2d_insert_point_in_bond*/

LOCAL	boolean f_user_2d_join_curves(
	CURVE		*curve,
	CURVE		*curve1,
	CURVE		*curve2)
{
	INTERFACE	*intfc = curve->interface;
	size_t		sizest = size_of_state(intfc);

	if (interpolate_intfc_states(intfc) && (sizest != 0))
	{
	    ft_assign(left_start_state(curve),left_start_state(curve1),sizest);
	    ft_assign(right_start_state(curve),right_start_state(curve1),sizest);
	    bi_interpolate_intfc_states(intfc,0.5,0.5,
			                Coords(curve1->end->posn),
					left_end_state(curve1),
			                Coords(curve2->start->posn),
					left_start_state(curve2),
			                left_state(curve1->end->posn));
	    bi_interpolate_intfc_states(intfc,0.5,0.5,
	    	                        Coords(curve1->end->posn),
	    			        right_end_state(curve1),
	    	                        Coords(curve2->start->posn),
	    			        right_start_state(curve2),
	    	                        right_state(curve1->end->posn));
	    ft_assign(left_end_state(curve),left_end_state(curve2),sizest);
	    ft_assign(right_end_state(curve),right_end_state(curve2),sizest);
	}
	if (wave_type(curve1) != wave_type(curve2))
	{
	    (void) printf("WARNING in f_user_2d_join_curves(), "
		          "wave_type mismatch()\n");
	    return NO;
	}
	user_copy_hyper_surf(Hyper_surf(curve),Hyper_surf(curve1));
	if (!rst_cor_after_join_hypersurfaces(Hyper_surf(curve1),
						 Hyper_surf(curve2),
						 Hyper_surf(curve)))
	    return NO;
	if (is_mono_comp_curve(curve))
	    mono_comp_curves(curve->interface) = YES;

	if (hs_flags_equal(curve1,curve2) ||
	    merge_hs_flags(Hyper_surf(curve1),Hyper_surf(curve2)))
		Hs_flag(curve) = Hs_flag(curve1);
	else
	{
	    screen("ERROR in f_user_2d_join_curves(), "
	           "trying to join two curves with different Hs_flag.\n");
	    clean_up(ERROR);
	}

	if (redistribution_direction(curve1)!=redistribution_direction(curve2))
	{
	    redistribution_direction(curve) = (rand()%2) ?
		redistribution_direction(curve1) :
		redistribution_direction(curve2);
	}
	else
	    redistribution_direction(curve) = redistribution_direction(curve1);
	if (hypersurface_normal(curve1) == hypersurface_normal(curve2))
	    hypersurface_normal_function(curve) =
	        hypersurface_normal_function(curve1);
	return YES;
}		/*end f_user_2d_join_curves*/


/*
*			merge_hs_flags():
*
*	This function attempts to merge two hyper-surface flags by 
*	promoting one to match the other.  The supposed goal is to merge
*	two hyper-surfaces with these respective flags.  The resulting
*	flag should be the most specialized union of the two.
*/

EXPORT boolean f_merge_hs_flags(
	HYPER_SURF	*hs1,
	HYPER_SURF	*hs2)
{
	if (do_not_redistribute(hs1) || do_not_redistribute(hs2))
	{
	    do_not_redistribute(hs1) = YES;
	    do_not_redistribute(hs2) = YES;
	}
	if (never_redistribute(hs1) || never_redistribute(hs2))
	{
	    never_redistribute(hs1) = YES;
	    never_redistribute(hs2) = YES;
	}

	if (untracked_hyper_surf(hs1) || untracked_hyper_surf(hs2))
	{
	    untracked_hyper_surf(hs1) = YES;
	    untracked_hyper_surf(hs2) = YES;
	}

	if (redistribute_by_time_step_frequency(hs1) ||
	    redistribute_by_time_step_frequency(hs2))
	{
	    redistribute_by_time_step_frequency(hs1) = YES;
	    redistribute_by_time_step_frequency(hs2) = YES;
	}

	if (redistribute_hyper_surface(hs1) || redistribute_hyper_surface(hs2))
	{
	    redistribute_hyper_surface(hs1) = YES;
	    redistribute_hyper_surface(hs2) = YES;
	}

	if (redistributed(hs1) || redistributed(hs2))
	{
	    redistributed(hs1) = YES;
	    redistributed(hs2) = YES;
	}

	return (hs_flags_equal(hs1,hs2)) ? YES : NO;
}		/*end f_merge_hs_flags*/


/*
*		test_for_mono_comp_curves():
*
*	Tests for the existence of curves with the same
*	component value on both sides of the curve.
*/

/*ARGSUSED*/
EXPORT void test_for_mono_comp_curves(
	INTERFACE	*intfc)
{
	CURVE		**c;

	mono_comp_curves(intfc) = NO;
	for (c = intfc->curves; c && *c; ++c)
	    if (is_mono_comp_curve(*c))
	    	mono_comp_curves(intfc) = YES;
}		/*end test_for_mono_comp_curves*/

LOCAL	void	set_2d_rect_bdry_types_from_curves(
	INTERFACE	*intfc)
{
	RECT_GRID	*tgr = &topological_grid(intfc);
	CURVE		**c;
	int		dim = intfc->dim;
	int		i, j, ic, jc;
	int		w_type;

	for (i = 0; i < dim; ++i)
	{
	    for (j = 0; j < 2; ++j)
	    {
	        w_type = UNKNOWN_WAVE_TYPE;
	        for (c = intfc->curves; c && *c; ++c)
	        {
	            if (!is_bdry(*c))
			continue;
	            (void) rect_bdry_side_for_curve(&ic,&jc,*c,tgr);
	            if (ic != i || jc != j)
			continue;
	            if (w_type == UNKNOWN_WAVE_TYPE)
			w_type = wave_type(*c);
	            if (w_type != wave_type(*c))
	            {
	                w_type = UNKNOWN_WAVE_TYPE;
	                break;
	            }
	            if (w_type == DIRICHLET_BOUNDARY)
	            {
			/*
			*  Without additional prompting it is not
			*  possible to distinguish homogeneous 
			*  Dirichlet boundaries from non-homogeneous
			*  rectangular boundaries that consist of a
			*  single curve.  The code can continue to
			*  run correctly with rectangular type unknown
			*  so that is what is the default case for now.
			*/
	                w_type = UNKNOWN_WAVE_TYPE;
			break;
	            }
	        }
	        if (w_type != UNKNOWN_WAVE_TYPE)
	        {
	            rect_boundary_type(intfc,i,j) = w_type;
	            if (w_type == DIRICHLET_BOUNDARY)
	            {
	                rect_boundary_type(intfc,i,j) = UNKNOWN_BOUNDARY_TYPE;
	            }
	        }
	    }
	}
}		/*end set_2d_rect_bdry_types_from_curves*/

LOCAL	REDISTRIBUTION_DIRECTION read_redistribution_direction_from_string(
	    const char *rd)
{
	if (rd == NULL)
	    return BACKWARD_REDISTRIBUTION;
	else if (strcasecmp(rd,"FORWARD_REDISTRIBUTION") == 0)
	    return FORWARD_REDISTRIBUTION;
	else if (strcasecmp(rd,"BACKWARD_REDISTRIBUTION") == 0)
	    return BACKWARD_REDISTRIBUTION;
	return BACKWARD_REDISTRIBUTION;
}		/*read_redistribution_direction_from_string*/



LOCAL boolean f_user_3d_join_curves(
	CURVE *curve,
	CURVE *curve1,
	CURVE *curve2)
{
	if (hsbdry_type(curve1) != hsbdry_type(curve2))
	{
	    screen("ERROR in f_user_3d_join_curves(), attempt to join "
		   "curves of different hsbdry_types\n");
	    clean_up(ERROR);
	}
	hsbdry_type(curve) = hsbdry_type(curve1);
	return YES;
}		/*end f_user_3d_join_curves*/

/*ARGSUSED*/
LOCAL	INTERFACE *f_user_3d_copy_interface(
	INTERFACE	*new_intfc,
	INTERFACE	*intfc)
{
	BOND		*b, *newb;
	BOND_TRI	**btri, **newbtri;
	CURVE		**pc, **pnc;
	size_t		sizest;

/*#bjet2  in  */
/*copy_tris   */
/*    assign_btri_states */

	return new_intfc;
}		/*end f_user_3d_copy_interface*/


LOCAL	boolean f_user_3d_insert_point_in_bond(
	BOND		*b)
{
	INTERFACE	*intfc;
	BOND		*bnew;
	BOND_TRI	*btri,**btris;
	BOND_TRI	*nbtri,**nbtris;
	double		len,total_len,para;
	Locstate	start_left_state,end_left_state,mid_left_state;
	Locstate	start_right_state,end_right_state,mid_right_state;

	bnew = b->next;
	intfc = current_interface();
	if (interpolate_intfc_states(intfc) && size_of_state(intfc) != 0)
	{
	    len = bond_length(b);
	    total_len = len + bond_length(bnew);
	    para = (total_len <= .00001) ? 0.5 : len / (total_len);
	    for (btris = Btris(b), nbtris = Btris(bnew); btris && *btris;
			++btris, ++nbtris)
	    {
		btri = *btris;
		nbtri = *nbtris;

		start_left_state = left_start_btri_state(btri);
		mid_left_state = left_end_btri_state(btri);
		end_left_state = left_end_btri_state(nbtri);

		start_right_state = right_start_btri_state(btri);
		mid_right_state = right_end_btri_state(btri);
		end_right_state = right_end_btri_state(nbtri);

		bi_interpolate_intfc_states(intfc,1.-para,para,
			                    Coords(b->start),start_left_state,
			                    Coords(bnew->end),end_left_state,
			                    mid_left_state);
		bi_interpolate_intfc_states(intfc,1.-para,para,
			                    Coords(b->start),start_right_state,
			                    Coords(bnew->end),end_right_state,
			                    mid_right_state);
	    }
	}
	return FUNCTION_SUCCEEDED;
}		/*end f_user_3d_insert_point_in_bond */

EXPORT	boolean	f_insert_point_in_tri(
	FT_POINT	*p,
	TRI	*tri,
	SURFACE	*s)
{
	INTERFACE *intfc = s->interface;
	Locstate  sl1, sr1, sl2, sr2, sl3, sr3;
	double	  f[3];

	if (interpolate_intfc_states(intfc) && size_of_state(intfc) != 0)
	{
	    interpolation_coefs_in_tri(Coords(p),f,tri);
	    slsr(Point_of_tri(tri)[0],Hyper_surf_element(tri),
		 Hyper_surf(s),&sl1,&sr1);
	    slsr(Point_of_tri(tri)[1],Hyper_surf_element(tri),
		 Hyper_surf(s),&sl2,&sr2);
	    slsr(Point_of_tri(tri)[2],Hyper_surf_element(tri),
		 Hyper_surf(s),&sl3,&sr3);
	}
	else
	{
	    f[0] = 0.0;
	    f[1] = 0.0;
	    f[2] = 0.0;
	    sl1 = NULL;
	    sr1 = NULL;
	    sl2 = NULL;
	    sr2 = NULL;
	    sl3 = NULL;
	    sr3 = NULL;
	}

	if (i_insert_point_in_tri(p,tri,s) != FUNCTION_SUCCEEDED)
	    return FUNCTION_FAILED;

	if (interpolate_intfc_states(intfc) && size_of_state(intfc) != 0)
	{
	    double *crds0 = Coords(Point_of_tri(tri)[0]);
	    double *crds1 = Coords(Point_of_tri(tri)[1]);
	    double *crds2 = Coords(Point_of_tri(Tri_on_side01(tri))[2]);
	    if ((tri_interpolate_intfc_states(intfc,f[0],f[1],f[2],crds0,sl1,
					      crds1,sl2,crds2,sl3,
					      left_state(p))
						  != FUNCTION_SUCCEEDED)
	        ||
		(tri_interpolate_intfc_states(intfc,f[0],f[1],f[2],crds0,sr1,
					      crds1,sr2,crds2,sr3,
					      right_state(p))
						  != FUNCTION_SUCCEEDED))
	    {
		screen("ERROR in f_insert_point_in_tri(), "
		       "tri_interpolate_intfc_states() failed\n");
		clean_up(ERROR);
	    }
	}
	return FUNCTION_SUCCEEDED;
}		/*end f_insert_point_in_tri*/

EXPORT	boolean	f_insert_point_in_tri_side(
	FT_POINT	*p,
	int	side,
	TRI	*tri,
	SURFACE	*s)
{
	INTERFACE *intfc = s->interface;

	if (!is_side_bdry(tri,side) && interpolate_intfc_states(intfc) &&
	    (size_of_state(intfc) != 0))
	{
	    Locstate sl0, sr0, sl1, sr1;
	    FT_POINT    *p0, *p1;
	    double    x0, x1;
	    p0 = Point_of_tri(tri)[side];
	    x0 = separation(p,p0,intfc->dim)/length_of_tri_side(tri,side);
	    p1 = Point_of_tri(tri)[Next_m3(side)];
	    x1 = separation(p,p1,intfc->dim)/length_of_tri_side(tri,side);
	    x0 = x0/(x0+x1);
	    x1 = x1/(x0+x1);

	    slsr(p0,Hyper_surf_element(tri),Hyper_surf(s),&sl0,&sr0);
	    slsr(p1,Hyper_surf_element(tri),Hyper_surf(s),&sl1,&sr1);

	    bi_interpolate_intfc_states(intfc,x0,x1,Coords(p0),sl0,
			                Coords(p1),sl1,left_state(p));
	    bi_interpolate_intfc_states(intfc,x0,x1,Coords(p0),sr0,
			                Coords(p1),sr1,right_state(p));
	}

	if (i_insert_point_in_tri_side(p,side,tri,s) != FUNCTION_SUCCEEDED)
	    return FUNCTION_FAILED;

	return FUNCTION_SUCCEEDED;
}		/*end f_insert_point_in_tri_side*/

LOCAL	void	interpolation_coefs_in_tri(
	double	*p,
	double	*f,
	TRI	*tri)
{
	FT_POINT **tp;
	double dp0[3], dp1[3], dp2[3];
	const double *n = Tri_normal(tri);
	double D = Dot3d(n,n);
	int   i;

	tp = Point_of_tri(tri);
	for (i = 0; i < 3; ++i)
	{
	    dp0[i] = p[i] - Coords(tp[0])[i];
	    dp1[i] = p[i] - Coords(tp[1])[i];
	    dp2[i] = p[i] - Coords(tp[2])[i];
	}
	f[0] = Det3d(dp1,dp2,n)/D;
	f[1] = Det3d(dp2,dp0,n)/D;
	f[2] = Det3d(dp0,dp1,n)/D;
}		/*end interpolation_coefs_in_tri*/

EXPORT	SURFACE *f_make_surface(
	COMPONENT neg_comp,
	COMPONENT pos_comp,
	CURVE	  **neg,
	CURVE	  **pos)
{
	SURFACE		*news; 

	news = i_make_surface(neg_comp,pos_comp,neg,pos);
	if (!rst_cor_after_make_hyper_surf(Hyper_surf(news)))
	    return NULL;
	return news;
}		/*end f_make_surface*/

EXPORT	SURFACE	*f_join_surfaces(
	CURVE *c)
{
	BOND         *b;
	HYPER_SURF   *hsn, *hsp;
	HS_FLAG      hsflag;
	TRI          *ntri, *ptri;
	INTERFACE    *intfc = c->interface;
	SURFACE      *news, *sn, *sp;
	int          nside, pside;
	size_t	     sizest = size_of_state(intfc);

	debug_print("join_surfaces","Entered f_join_surfaces\n");

	if (debugging("join_surfaces"))
	    (void) printf("joining_surfaces at curve %llu\n",curve_number(c));
	if (!find_surfaces_to_join_at_curve(c,&sn,&sp))
	{
	    debug_print("join_surfaces","Left f_join_surfaces\n");
	    return NULL;
	}
	if (debugging("join_surfaces"))
	{
	    (void) printf("joining_surfaces sn = %llu, and sp = %llu\n",
			  surface_number(sn),surface_number(sp));
	}
	if (wave_type(sn) != wave_type(sp))
	{
	    (void) printf("WARNING in f_join_surfaces(), "
			  "unequal wave types\n");
	    debug_print("join_surfaces","Left f_join_surfaces\n");
	    return NULL;
	}
	hsn = Hyper_surf(sn);
	hsp = Hyper_surf(sp);
	if (!hs_flags_equal(hsn,hsp) && !merge_hs_flags(hsn,hsp))
	{
	    (void) printf("WARNING in f_join_surfaces(), "
			  "incompatible hyper surf flags\n");
	    debug_print("join_surfaces","Left f_join_surfaces\n");
	    return NULL;
	}
	hsflag = Hs_flag(hsn);
	if (interpolate_intfc_states(intfc) && (sizest != 0))
	{
	    Locstate sln, srn, slp, srp;
	    FT_POINT    *p;
	    b = c->first;
	    if (!tris_on_side_of_bond_for_join(b,&ntri,&nside,&ptri,&pside))
	    {
	        (void) printf("WARNING in f_join_surfaces(), "
			      "can't find tris to join at c->first\n");
	        debug_print("join_surfaces","Left f_join_surfaces\n");
	        return NULL;
	    }
	    p = b->start;
	    slsr(p,Hyper_surf_element(ntri),hsn,&sln,&srn);
	    slsr(p,Hyper_surf_element(ptri),hsp,&slp,&srp);
	    bi_interpolate_intfc_states(intfc,0.5,0.5,Coords(p),sln,
			                Coords(p),slp,left_state(p));
	    bi_interpolate_intfc_states(intfc,0.5,0.5,Coords(p),srn,
			                Coords(p),srp,right_state(p));
	    for (b = c->first; b != NULL; b = b->next)
	    {
	        if (!tris_on_side_of_bond_for_join(b,&ntri,&nside,&ptri,&pside))
		{
	            (void) printf("WARNING in f_join_surfaces(), "
			          "can't find tris to join\n");
	            debug_print("join_surfaces","Left f_join_surfaces\n");
	            return NULL;
		}
	        p = b->end;
	        slsr(p,Hyper_surf_element(ntri),hsn,&sln,&srn);
	        slsr(p,Hyper_surf_element(ptri),hsp,&slp,&srp);
	        bi_interpolate_intfc_states(intfc,0.5,0.5,Coords(p),sln,
			                    Coords(p),slp,left_state(p));
	        bi_interpolate_intfc_states(intfc,0.5,0.5,Coords(p),srn,
			                    Coords(p),srp,right_state(p));
	    }
	}
	if ((news = i_join_surfaces(c)) == NULL)
	{
	    (void) printf("WARNING in f_join_surfaces(), "
			  "i_join_surfaces returns NULL\n");
	    debug_print("join_surfaces","Left f_join_surfaces\n");
	    return NULL;
	}
	if (sn != sp) /* sn and sp are now deleted surfaces */
	{
	    Hs_flag(news) = hsflag;
	    if (!rst_cor_after_join_hypersurfaces(hsn,hsp,
						     Hyper_surf(news)))
	    (void) printf("WARNING in f_join_surfaces(), "
			  "can't reset hypersurface correspondence\n");
	    debug_print("join_surfaces","Left f_join_surfaces\n");
	    return NULL;
	}
	debug_print("join_surfaces","Left f_join_surfaces\n");
	return news;
}		/*end f_join_surfaces*/

EXPORT	C_BOND *f_CBond(
	C_BOND *cb,
	FT_POINT  *start,
	FT_POINT  *end,
	TRI    *t1,
	TRI    *t2)
{
	if (cb == NULL)
	    cb = (C_BOND*)store(sizeof(F_C_BOND));
	i_CBond(cb,start,end,t1,t2);
	left_start_c_bond_state(cb)[0] = NULL;
	right_start_c_bond_state(cb)[0] = NULL;
	left_end_c_bond_state(cb)[0] = NULL;
	right_end_c_bond_state(cb)[0] = NULL;
	left_start_c_bond_state(cb)[1] = NULL;
	right_start_c_bond_state(cb)[1] = NULL;
	left_end_c_bond_state(cb)[1] = NULL;
	right_end_c_bond_state(cb)[1] = NULL;
	return cb;
}		/*end f_CBond*/


EXPORT	SURFACE *f_copy_surface(
	SURFACE	*s,
	CURVE	**pos,
	CURVE	**neg,
	boolean copy_tris)
{
	SURFACE		*news;

	news = i_copy_surface(s,pos,neg,copy_tris);

	return news;
}		/*end f_copy_surface*/

/*ARGSUSED*/
EXPORT	int f_delete_surface(
	SURFACE		*s)
{
	int status;

	if (!rst_cor_after_delete_hyper_surf(Hyper_surf(s)))
	    return 0;
	status = i_delete_surface(s);
	return status;
}		/*end f_delete_surface*/

EXPORT	void	f_user_fprint_surface(
	FILE		*file,
	SURFACE		*s)
{
	fprint_wave_type(file,"\n\tsurface->wave_type = ",
		         wave_type(s),"\n",s->interface);
	if (wave_type(s) == MOVABLE_BODY_BOUNDARY)
        {
            (void) fprintf(file,"\tBody index = ");
            (void) fprintf(file,"%d\n",body_index(Hyper_surf(s)));
            (void) fprintf(file,"\tTotal mass = ");
            (void) fprintf(file,"%"FFMT"\n",total_mass(Hyper_surf(s)));
            (void) fprintf(file,"\tMoment of inertial = ");
            (void) fprintf(file,"%"FFMT"\n",
                                mom_inertial(Hyper_surf(s)));
            (void) fprintf(file,"\tCenter of Mass = ");
            (void) fprintf(file,"%"FFMT" %"FFMT"\n",
                                center_of_mass(Hyper_surf(s))[0],
                                center_of_mass(Hyper_surf(s))[1],
                                center_of_mass(Hyper_surf(s))[2]);
            (void) fprintf(file,"\tCenter of mass velocity = ");
            (void) fprintf(file,"%"FFMT" %"FFMT"\n",
                                center_of_mass_velo(Hyper_surf(s))[0],
                                center_of_mass_velo(Hyper_surf(s))[1],
                                center_of_mass_velo(Hyper_surf(s))[2]);
            (void) fprintf(file,"\tRotation direction = ");
            (void) fprintf(file,"%"FFMT" %"FFMT"\n",
                                rotation_direction(Hyper_surf(s))[0],
                                rotation_direction(Hyper_surf(s))[1],
                                rotation_direction(Hyper_surf(s))[2]);
            (void) fprintf(file,"\tRotation center = ");
            (void) fprintf(file,"%"FFMT" %"FFMT"\n",
                                rotation_center(Hyper_surf(s))[0],
                                rotation_center(Hyper_surf(s))[1],
                                rotation_center(Hyper_surf(s))[2]);
            (void) fprintf(file,"\tAngular velocity = ");
            (void) fprintf(file,"%"FFMT"\n",
                                angular_velo(Hyper_surf(s)));
            (void) fprintf(file,"\tMotion type = %d\n",
                               motion_type(Hyper_surf(s)));
	}
	(void) fprintf(file,"\tSpecialized surface normal function = ");
	if (hypersurface_normal(s))
	    (void) fprintf(file,"%s\n",hypersurface_normal_name(s));
	else
	    (void) fprintf(file,"none\n");
}		/*end f_user_fprint_surface*/

EXPORT	void f_user_read_surface(
	SURFACE		*s)
{
	char		type[120];

	screen("Enter wave type for surface: ");
	(void) Scanf("%s\n",type);
	wave_type(s) = read_wave_type_from_string(type,s->interface);
}		/*end f_user_read_surface*/

/*ARGSUSED*/
EXPORT	void f_user_read_print_surface(
	SURFACE	      *surf,
	const IO_TYPE *io_type,
	boolean          overlay)
{
	FILE *file = io_type->file;
	char type[120];
	int i;

	(void) fgetstring(file,"surface->wave_type = ");
	(void) fscanf(file,"%s",type);
	wave_type(surf) = read_wave_type_from_string(type,surf->interface);
	if (wave_type(surf) == MOVABLE_BODY_BOUNDARY)
        {
            fgetstring(file,"Body index = ");
            (void) fscanf(file,"%d",&body_index(Hyper_surf(surf)));
            total_mass(Hyper_surf(surf)) =
                            read_print_float("Total mass = ",1.0,io_type);
            mom_inertial(Hyper_surf(surf)) =
                            read_print_float("Moment of inertial = ",1.0,
			    io_type);
            fgetstring(file,"Center of Mass = ");
	    for (i = 0; i < 3; ++i)
            	center_of_mass(Hyper_surf(surf))[i] =
                            fread_float(NULL,io_type);
	    fgetstring(file,"Center of mass velocity = ");
	    for (i = 0; i < 3; ++i)
            	center_of_mass_velo(Hyper_surf(surf))[i] =
                            fread_float(NULL,io_type);
	    fgetstring(file,"Rotation direction = ");
	    for (i = 0; i < 3; ++i)
            	rotation_direction(Hyper_surf(surf))[i] =
                            fread_float(NULL,io_type);
	    fgetstring(file,"Rotation center = ");
	    for (i = 0; i < 3; ++i)
            	rotation_center(Hyper_surf(surf))[i] =
                            fread_float(NULL,io_type);
	    for (i = 0; i < 3; ++i)
            angular_velo(Hyper_surf(surf)) =
                        read_print_float("Angular velocity = ",1.0,io_type);
            fgetstring(file,"Motion type = ");
            /* FIXME: the size of enum is compiler dependent */
            (void) fscanf(file,"%d",&motion_type(Hyper_surf(surf)));
	}
	if (fgetstring(file,"Specialized surface normal function = "))
	{
	    char s[2048];
	    (void) fgets(s,2046,file);
	    s[strlen(s)-1] = '\0';
	    set_normal_function(s,&hypersurface_normal_function(surf),
		                surf->interface);
	}
}		/*end f_user_read_print_surface*/

/*#bjet2 */
EXPORT	BOND_TRI *f_link_tri_to_bond(
	BOND_TRI *btri,
	TRI	 *tri,
	SURFACE  *s,
	BOND	 *b,
	CURVE    *c)
{
	INTERFACE	*cintfc = current_interface();
	size_t		sizest = size_of_state(cintfc);
	BOND		*bp, *bn;	/*#bjet2 closed curve case */
	int		i, j;

	btri = i_link_tri_to_bond(btri,tri,s,b,c);
	if (sizest == 0)
	{
	    left_start_btri_state(btri) = NULL;
	    right_start_btri_state(btri) = NULL;
	    left_end_btri_state(btri) = NULL;
	    right_end_btri_state(btri) = NULL;
	    return btri;
	}

	for (i = 0; i < size_of_pointers(Btris(b)); ++i)
	    if (btri == Btris(b)[i])
	        break;
	if (i == size_of_pointers(Btris(b)))
	{
	    screen("ERROR in f_user_link_tri_to_bond(), "
		   "btri not in Btris(b) array\n");
	    clean_up(ERROR);
	}

	if(b->prev != NULL)
	    bp = b->prev;
	else  if(is_closed_curve(c))
	    bp = c->last;
	else
	    bp = NULL;

	if (left_start_btri_state(btri) == NULL)
	{
            if (bp != NULL)
            {
	        for (j = 0; j < size_of_pointers(Btris(bp)); ++j)
	            if (btri->surface == (Btris(bp)[j])->surface)
	                break;
                if(j < size_of_pointers(Btris(bp)))
                {
	            left_start_btri_state(btri) =
		        left_end_btri_state(Btris(bp)[j]);
                }
                else
	            left_start_btri_state(btri) = (Locstate) store(sizest);
            }
            else
	        left_start_btri_state(btri) = (Locstate) store(sizest);
	}
	if (right_start_btri_state(btri) == NULL)
	{
            if (bp != NULL)
            {
	        for (j = 0; j < size_of_pointers(Btris(bp)); ++j)
	            if (btri->surface == (Btris(bp)[j])->surface)
	                break;
                if(j < size_of_pointers(Btris(bp)))
                {
	            right_start_btri_state(btri) =
		        right_end_btri_state(Btris(bp)[j]);
                }
                else
	            right_start_btri_state(btri) = (Locstate) store(sizest);
            }
            else
	        right_start_btri_state(btri) = (Locstate) store(sizest);
	}

	if(b->next != NULL)
	    bn = b->next;
	else  if(is_closed_curve(c))
	    bn = c->first;
	else
	    bn = NULL;

	if (left_end_btri_state(btri) == NULL)
	{
            if (bn != NULL)
            {
	        for (j = 0; j < size_of_pointers(Btris(bn)); ++j)
	            if (btri->surface == (Btris(bn)[j])->surface)
	                break;
                if(j < size_of_pointers(Btris(bn)))
                {
	            left_end_btri_state(btri) =
	                left_start_btri_state(Btris(bn)[j]);
                }
                else
	            left_end_btri_state(btri) = (Locstate) store(sizest);
            }
	    else
	        left_end_btri_state(btri) = (Locstate) store(sizest);
	}
	if (right_end_btri_state(btri) == NULL)
	{
            if (bn != NULL)
            {
	        for (j = 0; j < size_of_pointers(Btris(bn)); ++j)
	            if (btri->surface == (Btris(bn)[j])->surface)
	                break;
                if(j < size_of_pointers(Btris(bn)))
                {
	            right_end_btri_state(btri) =
		        right_start_btri_state(Btris(bn)[j]);
                }
                else
	            right_end_btri_state(btri) = (Locstate) store(sizest);
            }
	    else
	        right_end_btri_state(btri) = (Locstate) store(sizest);
	}
	
	return btri;
}		/*end f_link_tri_to_bond*/



EXPORT	void f_reverse_bond(
	BOND	 *b)
{
	INTERFACE	*cintfc = current_interface();
	size_t		sizest = size_of_state(cintfc);

	BOND_TRI **btris;

	i_reverse_bond(b);

	for (btris = Btris(b); btris && *btris; btris++)
	{
	    Locstate sl_tmp,sr_tmp;

	    sl_tmp = left_end_btri_state(*btris);
	    left_end_btri_state(*btris) =
	    	right_start_btri_state(*btris);
	    right_start_btri_state(*btris) = sl_tmp;

	    sr_tmp = left_start_btri_state(*btris);
	    left_start_btri_state(*btris) =
	    	right_end_btri_state(*btris);
	    right_end_btri_state(*btris) = sr_tmp;
	}

}	/* end f_reverse_bond */

/*#bjet2 */
/* make sure the end_btri_state of the previous bond is set, otherwise */
/* the value will be overwrote. */
EXPORT  void f_reorder_curve_link_list(
        CURVE *c)
{
	BOND *b;
	BOND_TRI **btris,**bntris;

	i_reorder_curve_link_list(c);
	
	for (b = c->first; b != c->last; b = b->next)
	{
	    if (b->next != NULL)
	    {
	    	for (btris = Btris(b); btris && *btris; btris++)
		for (bntris = Btris(b->next); bntris && *bntris; bntris++)
		{
		    if ((*btris)->surface == (*bntris)->surface)
		    {
		        left_start_btri_state(*bntris) =
		    	    left_end_btri_state(*btris);
		        right_start_btri_state(*bntris) =
		    	    right_end_btri_state(*btris);
		    }
		}
	    }
	}
	
	if(is_closed_curve(c))
	{
	    for (btris = Btris(c->last); btris && *btris; btris++)
	    for (bntris = Btris(c->first); bntris && *bntris; bntris++)
	    {
	        if ((*btris)->surface == (*bntris)->surface)
	        {
	            left_start_btri_state(*bntris) =
	    	        left_end_btri_state(*btris);
	            right_start_btri_state(*bntris) =
	    	        right_end_btri_state(*btris);
	        }
	    }
	}

}	/* end f_reorder_curve_link_list */

/*#bjet2 */
EXPORT SURFACE *f_detach_one_surface(
	SURFACE  *s)
{
INTERFACE	*intfc = current_interface();
SURFACE		*surf;
P_LINK	  	*p_table;	
int	  	p_size;
boolean		sav_copy;
	
	sav_copy = copy_intfc_states();
	set_copy_intfc_states(YES);

	p_size = 4*(intfc->num_points) + 1;
	uni_array(&p_table,p_size,sizeof(P_LINK));
	reset_hash_table(p_table,p_size);

	surf = copy_buffer_surface(s, p_table, p_size);
	/*delete_scn(s); */

	free(p_table);
	set_copy_intfc_states(sav_copy);
	
	return surf;
}


EXPORT	void f_assign_curve_boundary_flag(
	CURVE		*c)
{
	INTERFACE	*intfc = c->interface;
	int		bflag = is_bdry(c);
	int		bs,msk = 0x3;
	int		i, j, dim = intfc->dim;

        if (hsbdry_type(c) > ERROR) /* already set */
	{
	    return;
	}

	for (i = 0, bs = bflag; i < dim; ++i, bs = bs >> 2)
	{
	    j = (bs) & msk;
	    if (j == 0)
	        continue;
	    j--; 
	    if (buffered_boundary_type(rect_boundary_type((intfc),i,j)))
	        hsbdry_type(c) = SUBDOMAIN_HSBDRY;
	}
	if (hsbdry_type(c) == ERROR) /* Still unset */
	    hsbdry_type(c) = FIXED_HSBDRY;
}		/*end f_assign_curve_boundary_flag*/

EXPORT	void f_assign_curve_boundary_type(
	CURVE		*c,
	int		dir,
	int 		*side)
{
	INTERFACE	*intfc = c->interface;
	int		i, j, dim = intfc->dim;

        if (hsbdry_type(c) > ERROR) /* already set */
	{
	    return;
	}
	hsbdry_type(c) = FIXED_HSBDRY;
	if (buffered_boundary_type(rect_boundary_type((intfc),
	    (dir+1)%3,side[0])))
	    hsbdry_type(c) = SUBDOMAIN_HSBDRY;
	if (buffered_boundary_type(rect_boundary_type((intfc),
	    (dir+2)%3,side[1])))
	    hsbdry_type(c) = SUBDOMAIN_HSBDRY;
}		/*end f_assign_curve_boundary_type*/

EXPORT	void f_user_install_faces(
	SURFACE		*new_s,
	int		face_num)
{
	int             idir,iside;

	idir = face_num/2;      iside = face_num%2;
	wave_type(new_s) = rect_boundary_type(new_s->interface,idir,iside);
	if (wave_type(new_s) == DIRICHLET_BOUNDARY)
		bstate_index(new_s) = 2*idir + iside;
}		/*end f_user_install_faces*/

LOCAL	void f_read_print_Dirichlet_bdry_states(
	INIT_DATA     *init,
	const IO_TYPE *io_type,
	INTERFACE     *intfc)
{
	FILE       *file = io_type->file;
	HYPER_SURF **hs;
	int	   i;

	if (next_output_line_containing_string(file,
		"Hypersurface Dirichlet boundary state information") == NULL)
	{
	    screen("old stype, no longer supported!\n");
	    clean_up(ERROR);
	}
	for (i = 0, hs = intfc->hss; hs && *hs; ++i, ++hs)
	{
	    if (wave_type(*hs) != DIRICHLET_BOUNDARY)
		continue;
	    (void) fgetstring(file,"Boundary state index for");
	    (void) fscanf(file,"%*s %*d %*s %d",&bstate_index(*hs));
	}
	(void) next_output_line_containing_string(file,
		"End hypersurface Dirichlet boundary state information");
}		/*end f_read_print_Dirichlet_bdry_states*/

LOCAL	void f_fprint_Dirichlet_bdry_states(
	FILE		*file,
	INTERFACE	*intfc)
{
	HYPER_SURF **hs;
	int	   i, dim = intfc->dim;
	static const char *hsname[] = { "point", "curve", "surface" };

	(void) fprintf(file,"\n\n");
	(void) foutput(file);
	(void) fprintf(file,
	               "Hypersurface Dirichlet boundary state information ");
	(void) fprintf(file,"for interface %llu\n",interface_number(intfc));
	for (i = 0, hs = intfc->hss; hs && *hs; ++i, ++hs)
	{
	    if (wave_type(*hs) != DIRICHLET_BOUNDARY)
	        continue;
	    (void) fprintf(file,"Boundary state index for %s %d = %d\n",
	                   hsname[dim-1],i,bstate_index(*hs));
	}
	(void) foutput(file);
	(void) fprintf(file,
	             "End hypersurface Dirichlet boundary state information ");
	(void) fprintf(file,"for interface %llu\n",interface_number(intfc));
	(void) fprintf(file,"\n\n");
}		/*end f_fprint_Dirichlet_bdry_states*/

