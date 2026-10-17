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
*				intfc.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*			INTERFACE handling Package:
*
*
*		1. 		Summary:
*
*		This file and the associated files int.h, comp.c
*	define a package of routines for defining, allocating and
*	manipulating complex interfaces in two dimensions.
*
*
*	Data structures defined in int.h include:
*
*		INTERFACE	Arbitrarily complex geometrical interface.
*		NODE		Curve intersections occur at nodes.
*		CURVE		Non-intersecting curves join nodes.
*		COMPONENT	Label for a connected domain (component)
*		FT_POINT		A Geometrical Point.
*		BOND		Joins a pair of points.
*
*
*
*	Routines for manipulating these data structures include:
*
*		make_interface()	Allocate an INTERFACE structure.
*		copy_interface()	Make a copy of an INTERFACE.
*		exists_interface()	Verify an INTERFACE.
*		delete_interface()	Delete and free storage for INTERFACE.
*		read_interface()	Inputs a Formatted INTERFACE
*		fprint_interface()	Outputs a Formatted INTERFACE
*		read_print_interface()	Reads output of print_interface()
*		current_interface()	Reports the curent INTERFACE
*		set_current_interface() Changes the current INTERFACE
*
*		make_node()		Allocates a NODE, adds to INTERFACE
*		copy_node()		Copies a NODE to current INTERFACE
*		delete_node()		Deletes a NODE from INTERFACE.
*		fprint_node()		Outputs a Formatted NODE.
*		is_node_of_closed_curve_only()    Checks if a curve closes there
*
*		make_curve()		Allocates a CURVE, adds to INTERFACE.
*		copy_curve()		Copies a CURVE to current INTERFACE
*		delete_curve()		Deletes a CURVE from INTERFACE
*		fprint_curve()		Outputs a Formatted CURVE.
*		is_closed_curve		Checks if a CURVE is closed.
*
*		Point()			Allocates a FT_POINT in current INTERFACE.
*		average_points		Computes the mean of two FT_POINTS
*		copy_point()		Copies a FT_POINT to current INTERFACE.
*		separation()		Computes separation of two FT_POINTS.
*
*		Bond()			Allocates a BOND in current INTERFACE.
*		bond_length()		Gives the length of a BOND that has been
*					constructed with Bond().
*		
*
*		insert_point_in_bond()	Inserts a FT_POINT in middle of a BOND.
*		delete_start_of_bond()	Deletes FT_POINT at start of BOND.
*
*
*		next_curve()		Finds next CURVE on an INTERFACE.
*		next_bond()		Finds next BOND on an INTERFACE.
*		next_point()		Finds next FT_POINT on an INTERFACE.
*
*
*		split_curve()		Splits a CURVE at a FT_POINT into 2 CURVES
*		join_curves()		Joins CURVES at a common NODE.
*		intersections()		Determines Intersections of INTERFACE.
*
*
*		component()		Finds COMPONENT of a point x,y.
*		max_component()		Finds largest COMPONENT value
*		min_component()		Finds smallest COMPONENT value
*		exterior_component()		Finds exterior COMPONENT value
*		number_of_labeled_components()  Finds Range of COMPONENT values.
*
*
*	In addition, a set of routines is available that allows the user to
*	impose a regular rectangular grid on some rectangle containing or
*	contained in the region where the interface is defined.    These
*	routines allow efficient determination of the topological components
*	which are within any grid block.   They assume that only positive
*	COMPONENT values are used, and use the negative value ONFRONT to
*	indicate grid_blocks intersected by the INTERFACE.
*
*
*		set_topological_grid()	Establishes a regular rectangular grid.
*		is_component()		Checks if a COMPONENT meets grid-block.
*
*	A related set of routines provide information about the BONDS and
*	CURVES in a grid-block.
*
*	Documentation for all of the COMPONENT, BOND and CURVE list functions
*	is at the top of file comp.c.
*
*		2. Basic Interface Data Structures:
*
*		An INTERFACE is regarded as a topological object
*	consisting of a set of NODES joined by non-intersecting
*	CURVES.   These concepts are embodied in the definitions
*	of the mutually-recursive data structures INTERFACE,
*	NODE and CURVE given in file int.h.
*
*		A NODE is basically a FT_POINT, describing its location
*	along with sets of CURVES which begin and end at that node
*	- we call these the  in_curves and  out_curves  of the
*	NODE.
*
*		A CURVE joins a pair of NODES without intersecting
*	another CURVE.  Thus an INTERFACE defines a decomposition 
*	of the plane into disjoint connected regions, which we call 
*	components.  Each component may be referenced by ft_assigning 
*	it an integer value or COMPONENT.   Thus a CURVE is something 
*	that connects a pair of NODES and has unique COMPONENTS on 
*	either side of it (because of the non_intersecting requirement).
*
*
*		A FT_POINT is the basic geometrical object - an x,y
*	coordinate pair describing a position in the plane.
*
*
*
*		The code separates as much as possible the treatment 
*	of these high-level objects, from the treatment of low_level
*	features such as the specific representation of the curve
*	a CURVE represents.   Thus it should not be difficult to
*	modify the code to handle say a CURVE of parabolic sections.
*
*
*		The current code represents the curve as piecewise-
*	linear.   Thus a curve is represented as a linked list of
*	linear BONDS.   
*
*		A BOND connects two FT_POINTS, its start and end, by
*	a line segment, and in addition has pointers to both the
*	preceding and following bonds.
*
*
*		A certain amount of redundancy is built into these
*	definitions to allow flexibility, but not at the expense of
*	significant inneficiency.
*
*		The interface package supports a form of inheritance.
*	Each data structure is associated with a size in the I_INTERFACE
*	data structure.  The allocated storage for each field is determined
*	by this size,  which is required to be a least as large as the
*	corresponding size of the structure being allocated.  This enables
*	the user to define extensions of each interface data structure to
*	include additional information specific to the problem being
*	modeled.  For example, such a construction would be of the sort
*
*	struct _MY_CURVE { CURVE Curve; MY_DATA_STRUCTURE My_struct;};
*
*	since C arranges that the alignment of the first element of a
*	structure and the structure itself are the same,  one can cast
*	a object of type struct _MY_CURVE as a curve.  This is a partial
*	implementation of the data hiding provided by C++.
*
*
*			3. Subroutines and Usage:
*
*
*		The normal sequence to create an INTERFACE calls
*	make_interface(), then creates NODES as required using
*	make_node() and lastly creates the CURVES using make_curve().
*	In fact, since CURVES connect pairs of NODES, it is essential
*	to use this strategy.   
*
*
*		FT_POINTS are created using  Point(), and are then added
*	to CURVES using  insert_point_in_bond().
*
*		The routine  read_interface()  provides an automated way 
*	of initializing an INTERFACE - it prompts interactively.   The
*	routine  fprint_interface()  provides formatted printing of
*	an INTERFACE in a readable format.   Then read_print_interface()
*	can be used to read the output of fprint_interface().
*
*
*		To modify an existing INTERFACE, use the routines
*	make_node(), delete_node(), make_curve(), delete_curve(),
*	insert_point_in_bond(), delete_start_of_bond().   When
*	working on an INTERFACE, the routine  next_point()  is
*	a convenient way to step through the FT_POINTS of the INTERFACE
*	one by one.   next_point() may be used simultaneously on
*	any number of INTERFACES.
*
*		
*		The copy_... routines all work on the current interface,
*	with the exception of copy_interface() which defines a new
*	current interface.   If it is desired to modify two or more
*	INTERFACES simultaneously, this should be done by making
*	calls to  set_current_interface()  before working on a new
*	INTERFACE.	The value of the current interface can always
*	be found using  current_interface().   Note in particular that
*	if the current interface is deleted, then the current INTERFACE
*	is set to NULL.
*
*
*		The routines  split_curve()  and  join_curves()  can
*	be used to perform surgery on an INTERFACE.   split_curve()
*	splits a CURVE at an internal FT_POINT into two CURVES with a
*	common NODE.   join_curves()  joins a pair of CURVES which
*	have a common NODE to form a single CURVE.   Both routines
*	allow the COMPONENT specifications on the sides of the
*	new CURVE(S) to be specified, facilitating changes in
*	topology.
*
*
*		All of the routines do considerable error checking.
*	In addition each routine accomplishes as much related work
*	as possible.   Thus for example  make_curve()  not only
*	allocates a  CURVE  structure, but also inserts the CURVE
*	into the current INTERFACE CURVE list, into the in_curves
*	and out_curves of each of its end-NODES and initializes
*	the first and last BONDS on the CURVE with the end-NODES.
*	Generally, each routine can be assumed to do all of the
*	obvious book-keeping automatically, but when in doubt
*	the documentation above each routine should be consulted.
*
*
*
*		Again in order to make the package more flexible,
*	each routine finishes by executing some user provided code.
*	The I_INTERFACE data structure provides a family of function
*	pointers that can be defined by the user to take additional
*	action for the given function.  Such pointers may be NULL,
*	in which case no action is taken.  An example of the usage
*	is illustrated below.
*
*	struct _MY_POINT { FT_POINT pt; double distance;};
*	then we set size_point in the I_INTERFACE structure to
*	sizeof(struct _MY_POINT);
*
*	EXPORT void my_user_Point(FT_POINT *p)
*	{
*		((struct _MY_POINT *) p)->distance =
*				hypot(Coords(p)[0],Coords(p)[1]);
*	}
*			
*
*
*
*		4.  Storage Allocation Scheme:
*
*	The routines that create and modify INTERFACES all use a 
*	common storage_allocator.   There are no restrictions 
*	anywhere in the code on the number or sizes of INTERFACES, 
*	NODES, CURVES, BONDS or FT_POINTS.   If the storage allocator
*	runs out of space at some point, the routine that called
*	it will return an error value, generally 0 or a NULL
*	pointer as appropriate.   The low-level storage allocator
*	is not called directly.   The INTERFACE code maintains a
*	higher level storage allocator which dispenses space as
*	required and only occasionaly calls the low level one.
*	The low level allocator is called  scalar()  while
*	the higher level allocator is called  store().
*
*	The code maintains the concept of a current INTERFACE.
*	All operations that create or modify INTERFACES are
*	always applied only to the current interface.   The
*	high-level storage scheme associates a chain of large
*	blocks of space with each interface.   When the blocks
*	for the current interface are exhausted and more space
*	is needed, the low_level allocator is called to allocate
*	a new large block (Chunk) of size ChunkSize(intfc).   The Chunks
*	for a given interface are stored as a linked list.
*	This procedure tends to keep each interface stored very 
*	contiguosly and prevents unnecessary fragmentation in the 
*	low_level storage allocator.
*
*	In addition, for internal purposes the code maintains a
*	list of interfaces TABLES.   Each table summarizes the
*	storage statistics of an INTERFACE, and in fact contains
*	the INTERFACE structure.   This list of TABLES is itself
*	maintained as a linked list, and as new INTERFACES are
*	created space for their TABLES is first allocated from
*	the low-level allocator.   The code is structured in
*	such a way that none of the user-visible routines ever
*	explicitly reference these TABLES.
*
*
*	Finally the code uses a peculiar data structure called
*	a set to represent dynamically growing and shrinking
*	sets of NODES and CURVES.   These sets of pointers are
*	manipulated using the routines  add_to_pointers()  and  
*	delete_from_pointers(), which in turn use a lower-level
*	allocator called expand_set().
*
*
*
*
*				MORE TO COME:
*/




#include <intfc/iloc.h>


	/* LOCAL Function Declarations */
LOCAL	CURVE	*read_print_curve(INTERFACE*,const IO_TYPE*,int,
                                  INTERFACE_ADDRESSES*,boolean);
LOCAL	NODE	*read_print_node(INTERFACE*,const IO_TYPE*,uint64_t*,boolean);
LOCAL	struct Chunk	*GetNextFreeChunk(struct Table*);
LOCAL	struct Table	*GetNextFreeTable(void);
LOCAL	struct Table	*new_table(int);
LOCAL	boolean	read_print_curve_boundary(const IO_TYPE*,int,
                                          INTERFACE_ADDRESSES*);
LOCAL	boolean	read_print_curve_points(CURVE*,const IO_TYPE*,int,
                                        INTERFACE_ADDRESSES*);
LOCAL	void	ReturnChunksToFreeList(struct Chunk*,struct Chunk*);
LOCAL	void	ReturnTableToFreeList(struct Table*);
LOCAL	void	fprint_hypersurface_boundaries(FILE*,HYPER_SURF_BDRY**);
LOCAL	void	free_interface_addresses_structure(INTERFACE_ADDRESSES*);
LOCAL 	CURVE	**split_curve2d(FT_POINT*,BOND*,CURVE*,
			      COMPONENT,COMPONENT,COMPONENT,COMPONENT);
LOCAL 	CURVE	**split_curve3d(FT_POINT*,BOND*,CURVE*,
			      COMPONENT,COMPONENT,COMPONENT,COMPONENT);
LOCAL	boolean	collapse_tri_on_side(TRI*,SURFACE*,int);
LOCAL	boolean	is_tangled_polygon(const REAL*,int);
LOCAL	boolean	reset_tris_at_deleted_bond(BOND_TRI*,ORIENTATION,BOND_TRI*);
LOCAL	boolean	single_tri_surface_on_bond(BOND*);
LOCAL	void	split_tris_at_split_bond(FT_POINT*,BOND*,CURVE*);




/*
*				Interface Table:
*
*	Records Storage and other information for INTERFACES in use.
*	Records the information as a linked list of Tables, one for
*	each Interface.
*	Storage for the linked list is generated dynamically as
*	needed.   This allows programs to manipulate varying
*	numbers of interfaces without difficulty.
*
*	Within each Table, a further linked list of
*	ChunkSize(intfc)-sized blocks of storage is maintained -
*	this is the actual area where bonds, curves etc are stored for that
*	interface.   Individual Chunks should be large enough so that
*	large parts of an interface are stored contiguosly.   The use of
*	Chunks makes it easy to deal with a dynamically changing interface
*	without incurring major loss of contiguousness.   The lowest level
*	storage allocator is only called occasionaly - whenever a new Chunk 
*	is required. Thus there is little overhead incurred by the low level
*	storage scheme.   The actual dispensing of pieces of a chunk is done
*	by the routine  store().
*
*	The definition of the Table structure is in file table.h
*
*/

LOCAL struct Table *FirstIT = NULL;	/* The Interface Table List */
LOCAL struct Table *LastIT = NULL;
	  
LOCAL struct Table *cur_IT = NULL;	/* The current interface table */

LOCAL INTERFACE *cur_intfc = NULL;	/* The current interface */


/*
*				set_current_interface():
*
*	Sets the current interface to a particular previously created
*	interface.   Returns 1 if successful or 0 on error - if the
*	interface does not exist.
*/

EXPORT void set_current_interface(
	INTERFACE	*intfc)
{
	struct Table	*T;

	if ((T = table_of_interface(intfc)) == NULL)
	{
	    screen("ERROR in set_current_interface(), "
	           "Interface doesn't exist\n");
	    clean_up(ERROR);
	}
	cur_IT = T;
	cur_intfc = intfc;
	return;
}		/*end set_current_interface*/


/*
*				current_interface():
*
*	Returns a pointer to the current interface, or NULL if there
*	is no current interface.
*/

EXPORT INTERFACE *current_interface(void)
{
	if (cur_intfc == NULL)
	{
	    screen("ERROR in current_interface(), interface is NULL\n");
	    clean_up(ERROR);
	}
	return cur_intfc;
}		/*end current_interface*/



/*
*				make_interface():
*
*	Creates a new Interface and an associated Table in the
*	Interface Table List.   Apart from the Table, no other
*	storage is allocated here.   However both the cur_intfc
*	and cur_IT are set to point to the newly created object
*	provided the allocation is successful.
*
*	Returns a pointer to the interface, or NULL if out of
*	space.
*/

EXPORT INTERFACE *make_interface(
	int		dim)
{
	I_USER_INTERFACE *uh;
	int		i, j;

	    /* Allocate a new Table Entry: */

	if (DEBUG)
	    (void) printf("\n\nEntered make_interface()\n");
	if (new_table(dim) == NULL)
	    return NULL;
	cur_intfc->dim = dim;

	cur_intfc->num_points = 0;
	for (i = 0; i < dim; ++i)
	    for (j = 0; j < 2; ++j)
	        rect_boundary_type(cur_intfc,i,j) = UNKNOWN_BOUNDARY_TYPE;

	interface_reconstructed(cur_intfc) = NO;
	uh = i_user_hook(dim);
	i_user_interface(cur_intfc) = *uh;
	user_make_interface(cur_intfc);
	cur_intfc->normal_unset = YES;
	cur_intfc->curvature_unset = YES;

	if (DEBUG)
	    (void) printf("Left make_interface()\n\n");
	return cur_intfc;
}		/*end make_interface*/



/*
*				i_copy_interface():
*
*	Creates a distinct copy of a previously created interface.
*	Sets the current interface to the new interface.
*	Returns a pointer to the copy, or NULL on failure.
*/

EXPORT INTERFACE *i_copy_interface(
	INTERFACE	*intfc)
{
	NODE		**pn;
	CURVE		**pc;
	INTERFACE	*new_intfc;
	int		i, j, dim = intfc->dim;

	debug_print("copy_interface","Entered i_copy_interface(%llu)\n",
	      interface_number(intfc));
	if ((exists_interface(intfc) != YES) ||
	        ((new_intfc = make_interface(dim)) == NULL))
	{
	    debug_print("copy_interface","Left i_copy_interface()\n");
	    return NULL;
	}

	if (dim == 1)
	{
	    FT_POINT **pp;

	    if ((pp = intfc->points) != NULL)
	        while (*pp) (void)
		    copy_point(*pp++);

	    new_intfc->num_points = intfc->num_points;
	}

	        /* Copy over the individual nodes and curves: */

	if ((pn = intfc->nodes) != NULL)
	{
	    while (*pn)
		Check_return(copy_node(*pn++),i_copy_interface)
	}

	if ((pc = intfc->curves) != NULL)
	{
	    while (*pc)
	    {
	        NODE *newstart, *newend, **newpn;

	        for (pn=intfc->nodes,newpn = new_intfc->nodes;
	        		*pn != (*pc)->start; ++pn,++newpn) ;
	        newstart = *newpn;
	        for (pn=intfc->nodes,newpn = new_intfc->nodes;
	        		*pn != (*pc)->end; ++pn,++newpn) ;
	        newend = *newpn;
	        Check_return(copy_curve(*pc,newstart,newend),i_copy_interface)
	        ++pc;
	    }
	}
	        /* Copy the Underlying Grid: */

	new_intfc->table->fixed_grid = intfc->table->fixed_grid;
	set_topological_grid(new_intfc,&topological_grid(intfc));
	new_intfc->e_comps = copy_e_comps(intfc);
	new_intfc->default_comp = intfc->default_comp;
	new_intfc->elliptic_comp = intfc->elliptic_comp;
	new_intfc->table->min_comp = intfc->table->min_comp;
	new_intfc->table->max_comp = intfc->table->max_comp;
	interface_reconstructed(new_intfc) = interface_reconstructed(intfc);

	if (dim == 3)
	{
	    new_intfc->num_points = intfc->num_points;
	    max_pp_index(new_intfc) = max_pp_index(intfc);
	    
	    copy_all_surfaces(intfc,new_intfc);
	
	    /* two subdomain contact with each other */
	    reset_nodes_posn(new_intfc);
	}
	for (i = 0; i < dim; ++i)
	    for (j = 0; j < 2; ++j)
	        rect_boundary_type(new_intfc,i,j)=rect_boundary_type(intfc,i,j);

	Random01_seed(new_intfc)[0] = Random01_seed(intfc)[0];
	Random01_seed(new_intfc)[1] = Random01_seed(intfc)[1];
	Random01_seed(new_intfc)[2] = Random01_seed(intfc)[2];
	i_user_interface(new_intfc) = i_user_interface(intfc);
	new_intfc->normal_unset = YES;
	new_intfc->curvature_unset = YES;
	debug_print("copy_interface","Left i_copy_interface()\n");
	return new_intfc;
}		/*end i_copy_interface*/


/*
*				read_interface():
*
*	Initializes an INTERFACE interactively by prompting for
*	the NODES and CURVES.
*/

EXPORT INTERFACE *read_interface(void)
{
	INTERFACE	*infc;
	int		i, n, dim;

	screen("\nWe will now create an interface.\n\n");
	screen("Enter Interface Dimension: ");
	(void) Scanf("%d\n",&dim);
	infc = make_interface(dim);
	if (dim == 1)
	{
	    screen("Enter Number of Points: ");
	    (void) Scanf("%d\n",&n);
	    if (n <= 0)
		return infc;
	    for (i = 1; i <= n; ++i)
		Check_return(read_point(infc,i),read_interface)
	    return infc;
	}

	screen("Enter Number of Nodes: ");
	(void) Scanf("%d\n",&n);
	for (i = 1; i <= n; ++i)
	    Check_return(read_node(infc,i),read_interface)

	screen("\nEnter Number of Curves: ");
	(void) Scanf("%d\n",&n);
	for (i = 1; i <= n; ++i)
	    Check_return(read_curve(infc,i),read_interface)
	if (infc->dim == 3)
	{
	    screen("\nEnter Number of Surfaces: ");
	    (void) Scanf("%d\n",&n);
	    for (i = 1; i <= n; ++i)
		Check_return(read_surface(infc,i),read_interface)
	}
	return infc;
}		/*end read_interface*/


/*ARGSUSED*/
LIB_LOCAL	FT_POINT *i_read_point(
	INTERFACE	*intfc,
	int		i)
{
	FT_POINT		*p;
	COMPONENT	left = 0, right = 0;
	double		coords[MAXD];

	screen("Enter Position x of Point number %d: ",i);
	(void) Scanf("%f\n",&coords[0]);

	screen("Enter Left and Right Components for Point %d: ",i);
	(void) Scanf("%d %d\n",&left,&right);
 
	p = make_point(coords,left,right);
	user_read_point(intfc,p);
	screen("\n");
	return p;
}		/*end i_read_point*/


LIB_LOCAL NODE *i_read_node(
	INTERFACE	*intfc,
	int		i)
{
	NODE		*node;
	double		coords[MAXD];
	int		j, dim = intfc->dim;

	screen("Enter %d coordinates for Node number %d: ",dim,i);
	for (j = 0; j < dim; ++j) (void) Scanf("%f",&coords[j]);
	(void) Scanf("\n"); /* Grab trailing newline */
	node = make_node(Point(coords));
	if (! user_read_node(node))
	    return NULL;
	screen("\n");
	return node;
}		/*end i_read_node*/

/*ARGSUSED*/
LIB_LOCAL	CURVE	*i_read_curve(
	INTERFACE	*intfc,
	int		c_index)
{
	CURVE		*curve;
	COMPONENT	left = 0, right = 0;
	double		coords[MAXD];
	int		dim = intfc->dim;
	int		i, n, n1, n2;

	switch (dim)
	{
	case 2:
	    screen("Enter Left and Right Components for Curve %d: ",c_index);
	    (void) Scanf("%d %d\n",&left,&right);
	    break;
	case 3:
	    screen("For curve %d,\n",c_index);
	    break;
	}
	screen("Enter Node numbers of start and end Nodes: ");
	(void) Scanf("%d %d\n",&n1,&n2);
	screen("Enter Number of Interior Points on Curve: ");
	(void) Scanf("%d\n",&n);
	curve = make_curve(left,right,intfc->nodes[n1-1],intfc->nodes[n2-1]);

	screen("Enter %d Coordinate Points one per line:\n",n);
	for (i = 0; i < n; ++i)
	{
	    int j;
	    screen(": ");
	    for (j = 0; j < dim; ++j) (void) Scanf("%f",&coords[j]);
	    (void) Scanf("\n"); /* Grad trailing newline */
	    if (insert_point_in_bond(Point(coords),curve->last,curve) !=
		FUNCTION_SUCCEEDED)
	    {
	        screen("ERROR in i_read_curve(), "
		       "insert_point_in_bond() failed\n");
	        clean_up(ERROR);
	    }
	}
	user_read_curve(curve);
	screen("\n");
	return curve;
}		/*end i_read_curve*/

/*
*			read_print_interface():
*
*	Initializes an INTERFACE by reading the output of a previous 
*	print_interface() call.   Reads the next INTERFACE found
*	in the file  file  and returns a pointer to the INTERFACE,
*	or NULL if none is found.
*/

EXPORT INTERFACE *read_print_interface(
	INIT_DATA     *init,
	const IO_TYPE *io_type,
	boolean          overlay,
	int	      *grids_set)
{
	FILE                *file = io_type->file;
	INTERFACE_ADDRESSES Iaddr;
	INTERFACE	    *infc;
	REMAP		    *remap, Remap;
	int		    i,j;
	const char	    *line;
	char		    s[120];
	const char          *search_string;
	int		    dim;
	int                 nread;

	debug_print("restart","Entered read_print_interface\n");

	line = next_output_line_containing_string(file,"Interface");
	if (line == NULL)
	{
	    (void) printf("WARNING in read_print_interface(), "
			  "can't find interface printout\n");
	    debug_print("restart","Left read_print_interface\n");
	    return NULL;
	}
	if (!check_output(file))
	{
	    while ((((int)strlen(line)) < 12) 
	    		|| (line[0] == '#' && !isdigit(line[11]))
	    		|| (line[0] != '#' && !isdigit(line[10])))
	    {
	        line = next_output_line_containing_string(file,"Interface");
	        if (line == NULL)
	        {
	            (void) printf("WARNING in read_print_interface(), "
			          "can't find interface printout\n");
	            debug_print("restart","Left read_print_interface\n");
	            return NULL;
	        }
	    }
	}

	nread = sscanf(line,"%*s %*d %*s %d %*s %*s %s",&dim,s);
	if (nread == -1)
	    dim = 2; /*OLD DEFAULT*/
	if (dim <= 0)
	{
	    (void) printf("WARNING in read_print_interface(), "
			  "invalid dimension\n");
	    debug_print("restart","Left read_print_interface\n");
	    return NULL;
	}
	if (nread < 2)
	{
	    remap = remap_info();
	}
	else
	{
	    remap = &Remap;
	    set_remap(dim,read_remap_from_string(s),remap);
	}
	infc = make_interface(dim);

	*grids_set = read_print_intfc_rect_grids(io_type,infc,remap);

	zero_scalar(&Iaddr,sizeof(INTERFACE_ADDRESSES));
	Iaddr.dim = dim;
	switch (dim)
	{
	case 1:
	{
	    int npoints;

	    (void) fscanf(file,"%d %*s",&npoints);
	    if (DEBUG)
		(void) printf("Got npoints = %d\n",npoints);
	    for (i=0; i<npoints; ++i)
	    {
	        if (!read_print_point(infc,io_type,overlay))
		{
	            (void) printf("WARNING in read_print_interface(), "
			          "read_print_point failed\n");
	            debug_print("restart","Left read_print_interface\n");
	            return NULL;
		}
	    }
	    (void) fgetstring(file,"End Points");
	    break;
	}
	case 2:
	        /* Process the Nodes: */

	    (void) fscanf(file,"%d %*s",&Iaddr.num_nodes);
	    if (DEBUG)
	        (void) printf("Got num_nodes = %d\n",Iaddr.num_nodes);
	    uni_array(&Iaddr.nodes,Iaddr.num_nodes,sizeof(uint64_t));
	    for (i = 0; i < Iaddr.num_nodes; ++i)
	    {
		if (!read_print_node(infc,io_type,Iaddr.nodes+i,overlay))
		{
	            (void) printf("WARNING in read_print_interface(), "
			          "read_print_node failed\n");
	            debug_print("restart","Left read_print_interface\n");
	            return NULL;
		}
	    }
	    (void) fgetstring(file,"End Nodes");

	        /* Process the Curves: */

	    (void) fscanf(file,"%d %*s",&Iaddr.num_curves);
	    uni_array(&Iaddr.curves,Iaddr.num_curves,sizeof(uint64_t));
	    uni_array(&Iaddr.ns,Iaddr.num_curves,sizeof(uint64_t));
	    uni_array(&Iaddr.ne,Iaddr.num_curves,sizeof(uint64_t));
	    for (j = 0; j < Iaddr.num_curves; ++j)
	    {
	        if (!read_print_curve(infc,io_type,j,&Iaddr,overlay))
		{
	            (void) printf("WARNING in read_print_interface(), "
			          "read_print_curve failed\n");
	            debug_print("restart","Left read_print_interface\n");
	            return NULL;
		}
	    }
	    (void) fgetstring(file,"End Curves");
	    break;
	case 3:
	        /* Process the Nodes: */

	    (void) fscanf(file,"%d %*s",&Iaddr.num_nodes);
	    if (DEBUG)
	        (void) printf("Got num_nodes = %d\n",Iaddr.num_nodes);
	    uni_array(&Iaddr.nodes,Iaddr.num_nodes,sizeof(uint64_t));
	    for (i = 0; i < Iaddr.num_nodes; ++i)  
	    {
		if (!read_print_node(infc,io_type,Iaddr.nodes+i,overlay))
		{
	            (void) printf("WARNING in read_print_interface(), "
			          "read_print_node failed\n");
	            debug_print("restart","Left read_print_interface\n");
	            return NULL;
		}
	    }
	    (void) fgetstring(file,"End Nodes");

	        /* Process the Curves: */

	    (void) fscanf(file,"%d %*s",&Iaddr.num_curves);
	    uni_array(&Iaddr.curves,Iaddr.num_curves,sizeof(uint64_t));
	    uni_array(&Iaddr.ns,Iaddr.num_curves,sizeof(uint64_t));
	    uni_array(&Iaddr.ne,Iaddr.num_curves,sizeof(uint64_t));
	    uni_array(&Iaddr.num_psurfs,Iaddr.num_curves,INT);
	    uni_array(&Iaddr.psurfs,Iaddr.num_curves,sizeof(uint64_t *));
	    uni_array(&Iaddr.num_nsurfs,Iaddr.num_curves,INT);
	    uni_array(&Iaddr.nsurfs,Iaddr.num_curves,sizeof(uint64_t *));
	    uni_array(&Iaddr.num_surfs,Iaddr.num_curves,INT);
	    uni_array(&Iaddr.surfs,Iaddr.num_curves,sizeof(uint64_t *));
	    uni_array(&Iaddr.num_bonds,Iaddr.num_curves,INT);
	    uni_array(&Iaddr.bonds,Iaddr.num_curves,sizeof(uint64_t *));
	    uni_array(&Iaddr.tris,Iaddr.num_curves,sizeof(int **));
	    for (j = 0; j < Iaddr.num_curves; ++j)
	    {
	        if (!read_print_curve(infc,io_type,j,&Iaddr,overlay))
		{
	            (void) printf("WARNING in read_print_interface(), "
			          "read_print_curve failed\n");
	            debug_print("restart","Left read_print_interface\n");
	            return NULL;
		}
	    }
	    (void) fgetstring(file,"End Curves");

	        /* Process the Surfaces */

	    (void) fscanf(file,"%d %*s",&Iaddr.num_surfaces);
	    uni_array(&Iaddr.surfaces,Iaddr.num_surfaces,sizeof(uint64_t));
	    uni_array(&Iaddr.num_pcurves,Iaddr.num_surfaces,INT);
	    uni_array(&Iaddr.num_ncurves,Iaddr.num_surfaces,INT);
	    uni_array(&Iaddr.pcurves,Iaddr.num_surfaces,sizeof(uint64_t *));
	    uni_array(&Iaddr.ncurves,Iaddr.num_surfaces,sizeof(uint64_t *));
	    for (j = 0; j < Iaddr.num_surfaces; ++j)
	    {
	        if (!read_print_surface(infc,io_type,j,&Iaddr,overlay))
		{
	            (void) printf("WARNING in read_print_interface(), "
			          "read_print_surface failed\n");
	            debug_print("restart","Left read_print_interface\n");
	            return NULL;
		}
	    }
	    (void) fgetstring(file,"End Surfaces");
	    if (!sort_bond_tris(infc))
	    {
		(void) printf("WARNING in read_print_interface(), "
			      "sort_bond_tris() failed\n");
	        debug_print("restart","Left read_print_interface\n");
	        return NULL;
	    }
	    reset_intfc_num_points(infc);
	}

	search_string = "Rectangular Boundary Types for Interface";
	if (next_output_line_containing_string(file,search_string) != NULL)
	{
	    for (i = 0; i < dim; ++i)
	    {
	        (void) fgetstring(file,"direction");
	        for (j = 0; j < 2; ++j)
	        {
	            (void) fscanf(file,"%s",s);
	            rect_boundary_type(infc,i,j) =
	                read_boundary_type_from_string(s,infc);
	        }
	    }
	}
	search_string = "Random number seeds"; 
	if (next_output_line_containing_string(file,search_string) != NULL)
	{
	    int xsubi[3];
	    (void) fgetstring(file,"random01_seed = ");
	    (void) fscanf(file,"%x %x %x",xsubi,xsubi+1,xsubi+2);
	    Random01_seed(infc)[0] = xsubi[0];
	    Random01_seed(infc)[1] = xsubi[1];
	    Random01_seed(infc)[2] = xsubi[2];
	}
	if (! user_read_print_interface(init,io_type,infc,overlay))
	{
	    (void) printf("WARNING in read_print_interface(), "
			  "user_read_print_interface() failed\n");
	    debug_print("restart","Left read_print_interface\n");
	    return NULL;
	}
	free_interface_addresses_structure(&Iaddr);
	(void) fgetstring(file,"End Interface");

	if ((dim == 3) && debugging("restart"))
	{
	    (void) printf("Interface read by read_print_interface()\n");
	    print_interface(infc);
	    (void) printf("checking consistency of read interface\n");
	    if (!consistent_interface(infc))
	    {
		screen("ERROR in read_print_interface(), "
		       "inconsistent interface\n");
		clean_up(ERROR);
	    }
	    else
		(void) printf("interface is consistent\n");
	}
	debug_print("restart","Left read_print_interface\n");
	return infc;
}		/*end read_print_interface*/

LOCAL	void	free_interface_addresses_structure(
	INTERFACE_ADDRESSES *iaddr)
{
	int		i, j;

	if (iaddr->nodes != NULL)
	    free(iaddr->nodes);
	if (iaddr->curves != NULL)
	    free(iaddr->curves);
	if (iaddr->surfaces != NULL)
	    free(iaddr->surfaces);
	if (iaddr->ns != NULL)
	    free(iaddr->ns);
	if (iaddr->ne != NULL)
	    free(iaddr->ne);
	if (iaddr->tris != NULL)
	{
	    for (i = 0; i < iaddr->num_curves; ++i)
	    {
	        for (j = 0; j < iaddr->num_bonds[i]; ++j)
	            free(iaddr->tris[i][j]);
	        free(iaddr->tris[i]);
	    }
	    free(iaddr->tris);
	}
	if (iaddr->num_bonds != NULL)
	{
	    for (i = 0; i < iaddr->num_curves; ++i)
	        free(iaddr->bonds[i]);
	    free(iaddr->bonds);
	    free(iaddr->num_bonds);
	}
	if (iaddr->num_psurfs != NULL)
	{
	    for (i = 0; i < iaddr->num_curves; ++i)
	        free(iaddr->psurfs[i]);
	    free(iaddr->psurfs);
	    free(iaddr->num_psurfs);
	}
	if (iaddr->num_nsurfs != NULL)
	{
	    for (i = 0; i < iaddr->num_curves; ++i)
	        free(iaddr->nsurfs[i]);
	    free(iaddr->nsurfs);
	    free(iaddr->num_nsurfs);
	}
	if (iaddr->num_surfs != NULL)
	{
	    for (i = 0; i < iaddr->num_curves; ++i)
	        free(iaddr->surfs[i]);
	    free(iaddr->surfs);
	    free(iaddr->num_surfs);
	}
	if (iaddr->num_pcurves != NULL)
	{
	    for (i = 0; i < iaddr->num_surfaces; ++i)
	        free(iaddr->pcurves[i]);
	    free(iaddr->pcurves);
	    free(iaddr->num_pcurves);
	}
	if (iaddr->num_ncurves != NULL)
	{
	    for (i = 0; i < iaddr->num_surfaces; ++i)
	        free(iaddr->ncurves[i]);
	    free(iaddr->ncurves);
	    free(iaddr->num_ncurves);
	}
	zero_scalar(iaddr,sizeof(INTERFACE_ADDRESSES));
}		/*end free_interface_addresses_structure*/

LIB_LOCAL	FT_POINT *i_read_print_point(
	INTERFACE     *intfc,
	const IO_TYPE *io_type,
	boolean          overlay)
{
	FILE	  *file = io_type->file;
	char      bdry[20];
	COMPONENT left = NO_COMP, right = NO_COMP;
	FT_POINT	  *point;
	double	  coords[MAXD];
	int	  c;
	int	  i, dim;

	dim = intfc->dim;

	/* Read Components flag: */

	if (fgetstring(file,"Point") == FUNCTION_FAILED)
	{
	    screen("ERROR in i_read_print_point(), "
	           "POINT printout not found\n");
	    clean_up(ERROR);
	}
	if (fgetstring(file,"\t\tPosition ") == FUNCTION_FAILED)
	{
	    screen("ERROR in i_read_print_point(), "
	           "POINT Position not found\n");
	    clean_up(ERROR);
	}
	if ((c = getc(file)) != '\f') 	/* NOBINARY */
	{
	    (void) ungetc(c,file);
	    for (i = 0; i < dim; ++i)
	    {
	        (void) fscanf(file,"%*s %*s");
	        (void) fscan_float(file,coords+i);
	    }
	}
	else
	{
	    (void) getc(file);
	    (void) read_binary_real_array(coords,dim,io_type);
	}
	(void) fgetstring(file,"Left");
	bdry[0] = '\0';
	(void) fscanf(file,"%*s %*s %d %*s %*s %*s %d %s %*s",
	              &left,&right,bdry);
	point = make_point(coords,left,right);
	user_read_print_point(point,io_type,overlay);
	if (strcmp(bdry,"Boundary") == 0)
	    set_is_bdry(point);
	else
	    set_not_bdry(point);
	if (fgetstring(file,"End Point") == FUNCTION_FAILED)
	{
	    screen("ERROR in i_read_print_point(), "
	           "End Point not found\n");
	    clean_up(ERROR);
	}
	return point;
}		/*end i_read_print_point*/

LOCAL	CURVE *read_print_curve(
	INTERFACE	    *intfc,
	const IO_TYPE       *io_type,
	int		    c_index,
	INTERFACE_ADDRESSES *iaddr,
	boolean                overlay)
{
	FILE		*file = io_type->file;
	char		bdry[20];
	int		i, dim = intfc->dim;
	COMPONENT	left = NO_COMP, right = NO_COMP;
	NODE		*ns, *ne;
	CURVE		*curve;

	    /* Read Components and Boundary flag: */

	if (!fgetstring(file,"Curve"))
	{
	    (void) printf("WARNING in read_print_curve(), "
	                  "can't find curve header\n");
	    return NULL;
	}
	if (fscanf(file,"%llu:",iaddr->curves + c_index) != 1)
	{
	    (void) printf("WARNING in read_print_curve(), "
	                  "can't read curve address as printed\n");
	    return NULL;
	}
	switch (dim)
	{
	case 2:
	    if (!fgetstring(file,"Left"))
	    {
	        (void) printf("WARNING in read_print_curve(), "
	                      "can't read component header\n");
	        return NULL;
	    }
	    if (fscanf(file,"%*s %*s %d %*s %*s %*s %d %s %*s",
	                  &left,&right,bdry) != 3)
	    {
	        (void) printf("WARNING in read_print_curve(), "
	                      "can't read components\n");
	        return NULL;
	    }
	    break;
	case 3:
	    if (!fgetstring(file,"Status:"))
	    {
	        (void) printf("WARNING in read_print_curve(), "
	                      "can't read component status\n");
	        return NULL;
	    }
	    if (fscanf(file,"%s %*s",bdry) != 1)
	    {
	        (void) printf("WARNING in read_print_curve(), "
	                      "can't read bdry\n");
	        return NULL;
	    }
	    break;
	}

	    /* Determine the boundary of the curve */

	if (!read_print_curve_boundary(io_type,c_index,iaddr))
	{
	    (void) printf("WARNING in read_print_curve(), "
	                  "read_print_curve_boundary() failed\n");
	    return NULL;
	}

	ns = NULL;
	for (i = 0; i < iaddr->num_nodes; ++i)
	{
	    if (iaddr->ns[c_index] == iaddr->nodes[i])
	    {
	        ns = intfc->nodes[i];
	        break;
	    }
	}
	if (ns == NULL)
	{
	    (void) printf("WARNING in read_print_curve(), can't find "
	                  "ns, num_nodes = %d, c_index = %d, "
			  "ns[c_index] = %llu\n",
			  iaddr->num_nodes,c_index,iaddr->ns[c_index]);
	    for (i = 0; i < iaddr->num_nodes; ++i)
	        (void) printf("\tnodes[%d] = %llu\n",i,iaddr->nodes[i]);
	    return NULL;
	}

	ne = NULL;
	for (i = 0; i < iaddr->num_nodes; ++i)
	{
	    if (iaddr->ne[c_index] == iaddr->nodes[i])
	    {
	        ne = intfc->nodes[i];
	        break;
	    }
	}
	if (ne == NULL)
	{
	    (void) printf("WARNING in read_print_curve(), can't find "
	                  "ne, num_nodes = %d, c_index = %d, "
			  "ne[c_index] = %llu\n",
			  iaddr->num_nodes,c_index,iaddr->ne[c_index]);
	    for (i = 0; i < iaddr->num_nodes; ++i)
	        (void) printf("\tnodes[%d] = %llu\n",i,iaddr->nodes[i]);
	    return NULL;
	}
	curve = make_curve(left,right,ns,ne);
	if (strcmp(bdry,"Boundary") == 0)
	    set_is_bdry(curve);
	else
	    set_not_bdry(curve);
	if (!read_print_curve_points(curve,io_type,c_index,iaddr))
	{
	    (void) printf("WARNING in read_print_curve(), "
	                  "read_print_curve_points() failed\n");
	    return NULL;
	}

	read_print_tris_on_curve(file,curve,c_index,iaddr);
	if (!user_read_print_curve(curve,io_type,overlay))
	{
	    (void) printf("WARNING in read_print_curve(), "
	                  "user_read_print_curve() failed\n");
	    return NULL;
	}
	if (!fgetstring(file,"End of Curve"))
	{
	    (void) printf("WARNING in read_print_curve(), can't find "
	                  "End of Curve\n");
	    return NULL;
	}
	return curve;
}		/*end read_print_curve*/

LOCAL	boolean read_print_curve_boundary(
	const IO_TYPE       *io_type,
	int		    c_index,
	INTERFACE_ADDRESSES *iaddr)
{
	FILE *file = io_type->file;
	if (fscanf(file,"%*s %*s %*s %llu %*s %*s %*s %llu",
	           iaddr->ns+c_index,iaddr->ne+c_index) != 2)
	{
	    (void) printf("WARNING in read_print_curve_boundary(), "
	                  "can't read start and end node addresses as "
			  "printed\n");
	    return NO;
	}

	if (iaddr->dim == 3)
	{
	    int k;

	            /* Record Bounding Surfaces */
	    if (fscanf(file,"%d %*s %*s %*s",iaddr->num_surfs+c_index) != 1)
	    {
	        (void) printf("WARNING in read_print_curve_boundary(), "
	                      "can't read num_surfs\n");
	        return NO;
	    }
	    uni_array(iaddr->surfs+c_index,
	           iaddr->num_surfs[c_index],sizeof(uint64_t));
	    for (k = 0; k < iaddr->num_surfs[c_index]; ++k)
	    {
	        if (fscanf(file,"%llu",iaddr->surfs[c_index]+k) != 1)
		{
	            (void) printf("WARNING in read_print_curve_boundary(), "
	                          "can't read surfs[%d][%d]\n",c_index,k);
	            return NO;
		}
	    }
	    if (fscanf(file,"%d %*s",iaddr->num_psurfs+c_index) != 1)
	    {
	        (void) printf("WARNING in read_print_curve_boundary(), "
	                      "can't read num_psurfs\n");
	        return NO;
	    }
	    uni_array(iaddr->psurfs+c_index,iaddr->num_psurfs[c_index],
	           sizeof(uint64_t));
	    for (k = 0; k < iaddr->num_psurfs[c_index]; ++k)
	    {
	        if (fscanf(file,"%llu",iaddr->psurfs[c_index]+k) != 1)
		{
	            (void) printf("WARNING in read_print_curve_boundary(), "
	                          "can't read psurfs[%d][%d]\n",c_index,k);
	            return NO;
		}
	    }
	    if (fscanf(file,"%d %*s",iaddr->num_nsurfs+c_index) != 1)
	    {
	        (void) printf("WARNING in read_print_curve_boundary(), "
	                      "can't read num_nsurfs\n");
	        return NO;
	    }
	    uni_array(iaddr->nsurfs+c_index,iaddr->num_nsurfs[c_index],
	           sizeof(uint64_t));
	    for (k = 0; k < iaddr->num_nsurfs[c_index]; ++k)
	    {
	        if (fscanf(file,"%llu",iaddr->nsurfs[c_index]+k) != 1)
		{
	            (void) printf("WARNING in read_print_curve_boundary(), "
	                          "can't read nsurfs[%d][%d]\n",c_index,k);
	            return NO;
		}
	    }
	    if (!fgetstring(file,"Data"))
	    {
	        (void) printf("WARNING in read_print_curve_boundary(), "
		              "can find curve Data\n");
		return NO;
	    }
	    (void) getc(file); /*  remove trailing \n */
	}
	return YES;
}		/*end read_print_curve_boundary*/

/*
*			read_print_curve_points():
*	Reads formatted points on a curve.
*/

LOCAL boolean read_print_curve_points(
	CURVE		    *curve,
	const IO_TYPE       *io_type,
	int		    c_index,
	INTERFACE_ADDRESSES *iaddr)
{
	FILE    *file = io_type->file;
	int     i,j,n, dim = curve->interface->dim;
	double   coords[MAXD];


	        /* Find All Points on Curve: */

	if (fscanf(file,"%d %*s %*s %*s",&n) != 1)	/* Number of Points */
	{
	    (void) printf("WARNING in read_print_curve_points(), "
	                  "can't read number of curve points\n");
	    return NO;
	}
	(void) getc(file);				/* Grab trailing \n */
	if (iaddr->num_bonds != NULL)
	    iaddr->num_bonds[c_index] = (is_closed_curve(curve)) ? n : n-1;

	if (getc(file) != '\f') 	/* NOBINARY */
	{
	    for (i = 0; i < n-1; ++i) 		/* Skip End Node */
	    {
	        for (j = 0; j < dim; ++j)
	        {
	            if (fscan_float(file,coords+j) != 1)
		    {
	                (void) printf("WARNING in read_print_curve_points(), "
	                              "can't read ascii coords[%d] of "
				      "point %d\n",j,i);
		        return NO;
		    }
	        }
	        (void) fscanf(file,"%*s");
	        if (i == 0) /* Skip Start Node */
		    continue;
	        if (insert_point_in_bond(Point(coords),curve->last,curve) !=
		    FUNCTION_SUCCEEDED)
	        {
	            screen("ERROR in read_print_curve_points(), "
		           "insert_point_in_bond() failed\n");
	            clean_up(ERROR);
	        }
	    }
	}
	else	/* BINARY */
	{
	    int	nread;

	    nread = getc(file);
	    for (i = 0; i < n; ++i)
	    {
	        if (nread == 0)
	        {
	            (void) getc(file);
	            nread = getc(file);
	        }
		(void) read_binary_real_array(coords,dim,io_type);
	        nread -= dim;
	        if (i == 0) 	/* Skip Start Node */
		    continue;
	        if (i == n-1) 	/* Skip End Node */
		    break;
	        if (insert_point_in_bond(Point(coords),curve->last,curve) !=
		    FUNCTION_SUCCEEDED)
	        {
	            screen("ERROR in read_print_curve_points(), "
		           "insert_point_in_bond() failed\n");
	            clean_up(ERROR);
	        }
	    }
	}
	return YES;
}		/*end read_print_curve_points*/



/*
*			i_fprint_interface():
*
*		Prints an interface in a readable format.
*/

EXPORT void i_fprint_interface(
	FILE		*file,
	INTERFACE	*infc)
{
	int		  i;
	const char *direction[3] = { "x", "y", "z"};
	RECT_GRID         *gr = &topological_grid(infc);

	(void) foutput(file);
	(void) fprintf(file,"Interface %llu ",interface_number(infc));
	if (infc == NULL)
	{
	    (void) fprintf(file,"Dimension %d\n",-1);
	    (void) fprintf(file,"End Interface\n\n");
	    return;
	}
	(void) fprintf(file,"Dimension %d Remap Geometry %s\n",
		       infc->dim,gr->Remap.remap_name);
	if (exists_interface(infc) != YES)
	{
	    (void) fprintf(file,"Invalid Interface - "
	                        "Probably already deleted\n");
	    (void) fprintf(file,"End Interface\n\n");
	    return;
	}

	fprint_intfc_rect_grids(file,infc);

	switch (infc->dim)
	{
	case 1:
	{
	    FT_POINT		**pt;

	    (void) fprintf(file,"%d Points:\n",infc->num_points);
	    if ((pt = infc->points) != NULL)
	        while (*pt) fprint_point(file,*pt++);
	    (void) fprintf(file,"End Points\n\n");
	    break;
	}
	case 2:
	{
	    CURVE		**cur;
	    NODE		**nod;

	    (void) fprintf(file,"%d Nodes:\n",
	                   (int)size_of_pointers((POINTER *)infc->nodes));
	    if ((nod = infc->nodes) != NULL)
	        while (*nod) fprint_node(file,*nod++);
	    (void) fprintf(file,"End Nodes\n\n");

	    (void) fprintf(file,"%d Curves:\n",
	                   (int)size_of_pointers((POINTER *)infc->curves));
	    if ((cur = infc->curves) != NULL)
	        while (*cur) fprint_curve(file,*cur++);
	    (void) fprintf(file,"End Curves\n\n");
	    break;
	}
	case 3:
	{
	    CURVE		**cur;
	    NODE		**nod;
	    SURFACE		**surf;

	    set_tri_array_numbers(infc,LOCAL_INDICES);

	    (void) fprintf(file,"%d Nodes:\n",
	                   (int)size_of_pointers((POINTER *)infc->nodes));
	    if ((nod = infc->nodes) != NULL)
	        while (*nod) fprint_node(file,*nod++);
	    (void) fprintf(file,"End Nodes\n\n");

	    (void) fprintf(file,"%d Curves:\n",
	                   (int)size_of_pointers((POINTER *)infc->curves));
	    if ((cur = infc->curves) != NULL)
	        while (*cur) fprint_curve(file,*cur++);
	    (void) fprintf(file,"End Curves\n\n");

	    (void) fprintf(file,"%d Surfaces:\n",
	        (int)size_of_pointers((POINTER *)infc->surfaces));
	    if ((surf = infc->surfaces) != NULL)
	        while (*surf) fprint_surface(file,*surf++);
	    (void) fprintf(file,"End Surfaces\n\n");
	    null_tri_array_numbers(infc);
	    break;
	}
	}
	(void) fprintf(file,"%d Points in Interface\n",infc->num_points);
	(void) foutput(file);
	(void) fprintf(file,"Rectangular Boundary Types for Interface %llu\n",
	           interface_number(infc));
	(void) fprintf(file,"\n\t\tLOWER BOUNDARY\t\t\tUPPER BOUNDARY\n");
	for (i = 0; i < infc->dim; ++i)
	{
	    (void) fprintf(file,"%s-direction\t",direction[i]);
	    fprint_boundary_type(file,"",rect_boundary_type(infc,i,0),"",infc);
	    (void) fprintf(file,"\t\t");
	    fprint_boundary_type(file,"",rect_boundary_type(infc,i,1),
	    	                 "\n",infc);
	}
	(void) fprintf(file,"\n");
	(void) fprintf(file,
		       "End Rectangular Boundary Types for Interface %llu\n",
	               interface_number(infc));

	(void) fprintf(file,"\n");
	(void) foutput(file);
	(void) fprintf(file,"Random number seeds\n");
	(void) fprintf(file,"random01_seed = 0x%x 0x%x 0x%x\n",
	               Random01_seed(infc)[0],
	               Random01_seed(infc)[1],
	               Random01_seed(infc)[2]);
	(void) fprintf(file,"End Random number seeds\n");
	(void) fprintf(file,"\n");
	fprint_comp_equiv_lists(file,infc);
	(void) fprintf(file,"\n");
	user_fprint_interface(file,infc);
	(void) foutput(file);
	(void) fprintf(file,"End Interface\n\n");
}		/*end i_fprint_interface*/



/*
*			i_delete_interface():
*
*	Deletes a previosly created interface.   Returns 1 if successful
*	or 0 if the given interface did not exist.
*/

EXPORT int i_delete_interface(
	INTERFACE	*intfc)
{
	struct Table	*T;

	if (DEBUG) (void) printf("Entered i_delete_interface(%llu)\n",
	    		         interface_number(intfc));
	if (intfc==NULL)
	{
	    return 0;
	}

	    	/* Find Table and Previous Table: */
	if ((T = table_of_interface(intfc)) == NULL) /* No match */
	{
	    return 0;
	}

	        /* Reset Current Interface: */
	if (intfc==cur_intfc)
	{ 
	    cur_IT = NULL; 
	    cur_intfc = NULL; 
	}

	        /* Free the Chunks: */
	ReturnChunksToFreeList(T->first_chunk,T->last_chunk);
	T->first_chunk = T->last_chunk = NULL;

		/* Free the big chunks */
	while (T->big_chunks != NULL)
	{
	    struct Chunk *chunk = T->big_chunks;
	    T->big_chunks = chunk->prev;
	    free(chunk);
	}

	        /* Free the bond, curve, component lists: */
	if (T->compon1d)
	    free(T->compon1d);
	if (T->num_of_points)
	    free(T->num_of_points);
	if (T->pts_in_zone)
	    free(T->pts_in_zone);
	if (T->compon2d)
	    free(T->compon2d);
	if (T->num_of_bonds)
	    free(T->num_of_bonds);
	if (T->bonds)
	    free(T->bonds);
	if (T->bondstore)
	    free(T->bondstore);
	if (T->curves)
	    free(T->curves);
	if (T->curvestore)
	    free(T->curvestore);
	if (T->compon3d)
	    free(T->compon3d);
	if (T->num_of_tris)
	    free(T->num_of_tris);
	if (T->tris)
	    free(T->tris);
	if (T->tristore)
	    free(T->tristore);
	if (T->surfaces)
	    free(T->surfaces);
	if (T->surfacestore)
	    free(T->surfacestore);

	        /* Unlink and Free the Table: */
	if (T->prev != NULL)
	    T->prev->next = T->next;
	else
	{
	    FirstIT = T->next;
	    if (FirstIT != NULL)
	        FirstIT->prev = NULL;
	}
	if (T->next != NULL)
	    T->next->prev = T->prev;
	else
	{
	    LastIT = T->prev;
	    if (LastIT != NULL)
	        LastIT->next = NULL;
	}

	ReturnTableToFreeList(T);

	if (cur_intfc == NULL)
	{
	    cur_IT = LastIT; 
	    if (cur_IT != NULL)
	        cur_intfc = cur_IT->interface; 
	}
	if (DEBUG)
	    (void) printf("Left i_delete_interface()\n\n");
	return 1;
}		/*end i_delete_interface*/



/*
*			i_make_node():
*
*	Allocates a node structure in the current interface and
*	installs it in the node-list of the interface.   The
*	node will have position  p  which may be NULL.
*
*	Returns a pointer to the new node, or NULL on error.
*/


EXPORT NODE *i_make_node(
	FT_POINT		*p)
{
	NODE		*newnod;
	size_t		size_node;

	size_node = i_user_interface(cur_intfc).size_node;
	if ((newnod = (NODE *)store(size_node)) == NULL)
	    return NULL;
	if (! add_to_pointers(newnod,&cur_intfc->nodes))
	    return NULL;

	newnod->extra = NULL;
	newnod->obj = newnod;
	newnod->posn = p;
	newnod->interface = cur_intfc;
	Boundary(newnod) = 0;
	switch (cur_intfc->dim)
	{
	case 2:
	    Hyper_surf_bdry(newnod) = make_hypersurface_boundary();
	    Node_of_hsb(Hyper_surf_bdry(newnod)) = newnod;
	    break;
	case 3:
	    if (p != NULL)
	    	Boundary_point(p) = 1;
	    break;
	}
	newnod->in_curves = newnod->out_curves = NULL;
	cur_intfc->modified = YES;
	return newnod;
}		/*end i_make_node*/



/*
*			i_copy_node():
*
*	Copies a node to the current interface.
*	Returns a pointer to the new node or NULL if no space.
*/


EXPORT NODE *i_copy_node(
	NODE		*node)
{
	NODE		*newnode;

	if (node == NULL)
	{
	    return NULL;
	}
	if ((newnode = make_node(copy_point(node->posn))) == NULL)
	{
	    return NULL;
	}
	Boundary(newnode) = Boundary(node);
	newnode->extra = node->extra;

	return newnode;
}		/*end i_copy_node*/



/*
*			i_delete_node():
*
*	Deletes a node and its entry in its interface.   
*	Returns 1 if successful, or 0 if the node has some associated
*	curves - in the latter case the node is not deleted.
*/


EXPORT boolean i_delete_node(
	NODE		*node)
{
	if (node==NULL || node->in_curves!=NULL || node->out_curves!=NULL)
	    return FUNCTION_FAILED;
	if (node->interface==NULL || node->interface->nodes==NULL)
	    return FUNCTION_FAILED;

	if (! delete_from_pointers(node,&node->interface->nodes))
	    return FUNCTION_FAILED;
	if ((Hyper_surf_bdry(node) != NULL) &&
	    (!delete_from_pointers(Hyper_surf_bdry(node),
	                              &hyper_surf_bdry_list(node->interface))))
	    return FUNCTION_FAILED;
	node->interface->modified = YES;

	if (node->interface->dim == 2)
	    Hyper_surf_bdry(node)->interface = NULL;
	node->interface = NULL;
	return FUNCTION_SUCCEEDED;
}		/*end i_delete_node*/



/*
*				node_of_point():
*
*	Finds node of a given point, if any. Returns NULL otherwise.
*/

EXPORT NODE *node_of_point(
	FT_POINT		*point,
	INTERFACE	*intfc)
{
	NODE		**n;

	for (n = intfc->nodes; n && *n; ++n)
	    if ((*n)->posn == point)
		return *n;
	return NULL;
}		/*end node_of_point*/



/*
*			i_fprint_node():
*
*	Prints a Node.
*/


LIB_LOCAL void i_fprint_node(
	FILE		*file,
	NODE		*node)
{
	CURVE		**cur;
	int		dim;

	(void) fprintf(file,"\tNode %llu:\n",node_number(node));
	if (node == NULL)
	{ 
	    (void) fprintf(file,"\t\tNULL Node\n\t\tEnd Node\n");
	    return;
	}
	(void) fprintf(file,"\t\t%s Node\n",
	           is_bdry(node) ? "Boundary" : "Interior");
	dim = node->interface->dim;
	(void) fprintf(file,"\t\tPosition  ");
	if (node->posn == NULL)
	    (void) fprintf(file,"NULL\n");
	else if (is_binary_output() == YES)
	{
	    (void) fprintf(file,"\f%c",dim);
	    (void) fwrite((const void *)Coords(node->posn),FLOAT,dim,file);
	    (void) fprintf(file,"\n");
	}
	else
	{
	    const char	*vname[3] = {"x =", "   y =", "   z ="};
	    int		k;

	    for (k = 0; k < dim; ++k)
	        (void) fprintf(file,"%s %"FFMT,vname[k],Coords(node->posn)[k]);
	    (void) fprintf(file,"\n");
	}
	(void) fprintf(file,"\t\tIn_curves->  ");
	if ((cur = node->in_curves) == NULL) 
	    (void) fprintf(file,"NULL");
	else 
	    while (*cur) (void) fprintf(file,"%llu ",curve_number(*cur++));
	(void) fprintf(file,"\n");

	(void) fprintf(file,"\t\tOut_curves-> ");
	if ((cur = node->out_curves) == NULL) 
	    (void) fprintf(file,"NULL");
	else
	    while (*cur) (void) fprintf(file,"%llu ",curve_number(*cur++));
	(void) fprintf(file,"\n");

	user_fprint_node(file,node);
	(void) fprintf(file,"\tEnd Node\n\n");
}		/*end i_fprint_node*/


LIB_LOCAL	void i_fprint_intfc_rect_grids(
	FILE		*file,
	INTERFACE	*infc)
{
	(void) fprintf(file,"\nInterface Rectangular Grids\n");
	(void) fprintf(file,"\n\t\t\tInterface Topological Grid:\n\n");
	fprint_rectangular_grid(file,&topological_grid(infc));
	user_fprint_intfc_rect_grids(file,infc);
	(void) fprintf(file,"End Interface Rectangular Grids\n\n");
}		/*end i_fprint_intfc_rect_grids*/

LIB_LOCAL	int i_read_print_intfc_rect_grids(
	const IO_TYPE *io_type,
	INTERFACE     *infc,
	REMAP         *remap)
{
	FILE    *file = io_type->file;
	boolean	oldstyle;
	int	c;
	int	grids_set;

	debug_print("restrt","Entered read_print_intfc_rect_grids\n");

	/* Check for old style printout */

	oldstyle = NO;
	grids_set = NO;
	while ((c = getc(file)) != EOF)
	{
	    if (isdigit(c)) /* Old style printout, rect grids not printed */
	    {
	        (void) ungetc(c,file);
	        oldstyle = YES;
	        break;
	    }
	    if (isalpha(c)) /* New style printout, rect grids printed */
	    {
	        (void) ungetc(c,file);
	        oldstyle = NO;
	        break;
	    }
	}
	if (oldstyle == NO)
	{
	    grids_set = YES;
	    (void) fgetstring(file,"Interface Topological Grid:");
	    read_rectangular_grid(io_type,&topological_grid(infc),NO,remap);
	}
	user_read_print_intfc_rect_grids(io_type,infc,oldstyle,remap);
	if (oldstyle == NO)
	    (void) fgetstring(file,"End Interface Rectangular Grids");
	debug_print("restrt","Left read_print_intfc_rect_grids\n");
	return grids_set;
}		/*end i_read_print_intfc_rect_grids*/


/*
*			read_print_node():
*
*	Reads a formatted node from a file.
*/

LOCAL NODE *read_print_node(
	INTERFACE     *intfc,
	const IO_TYPE *io_type,
	uint64_t      *node_p,
	boolean          overlay)
{
	FILE    *file = io_type->file;
	NODE    *node;
	double   coords[MAXD];
	char    bdry[20];
	int     i, c, dim;

	(void) fgetstring(file,"Node");
	(void) fscanf(file,"%llu:",node_p);
	(void) fscanf(file,"%s %*s",bdry);

	dim = intfc->dim;
	(void) fgetstring(file,"Position  ");
	if ((c = getc(file)) != '\f') 	/* NOBINARY */
	{
	    (void) ungetc(c,file);
	    for (i = 0; i < dim; ++i)
	    {
	        (void) fscanf(file,"%*s %*s");
	        (void) fscan_float(file,coords+i);
	    }
	}
	else
	{
	    (void) getc(file);
	    (void) read_binary_real_array(coords,dim,io_type);
	}
	node = make_node(Point(coords));
	if (strcmp(bdry,"Boundary") == 0)
	    set_is_bdry(node);
	else
	    set_not_bdry(node);
	user_read_print_node(node,io_type,overlay);
	(void) fgetstring(file,"End Node");
	return node;
}		/*end read_print_node*/

/*ARGSUSED*/
EXPORT HYPER_SURF *i_make_hypersurface(
	COMPONENT	neg_comp,
	COMPONENT	pos_comp)
{
	HYPER_SURF	*hs;
	size_t		size_hyper_surf;

	size_hyper_surf = i_user_interface(cur_intfc).size_hyper_surf;
	hs = (HYPER_SURF *)store(size_hyper_surf);
	if (hs == NULL)
	{
	    if (DEBUG)
	        (void) printf("i_make_hypersurface returns NULL (8)\n");
	    return NULL;
	}
	if (!add_to_pointers(hs,&hyper_surf_list(cur_intfc)))
	{
	    if (DEBUG)
	        (void) printf("i_make_hypersurface returns NULL (8)\n");
	    return NULL;
	}
	Hyper_surf(hs) = hs;
	hs->interface = cur_intfc;
	negative_component(hs) = new_component(neg_comp);
	positive_component(hs) = new_component(pos_comp);
	cur_intfc->modified = YES;

	return hs;
}		/*end i_make_hypersurface*/

EXPORT	HYPER_SURF_BDRY	*i_make_hypersurface_boundary(void)
{
	HYPER_SURF_BDRY	*hsb;
	size_t	size_hyper_surf_bdry;

	size_hyper_surf_bdry =
	        i_user_interface(cur_intfc).size_hyper_surf_bdry;
	hsb = (HYPER_SURF_BDRY *)store(size_hyper_surf_bdry);
	if (! add_to_pointers(hsb,&hyper_surf_bdry_list(cur_intfc)))
	{
	    if (DEBUG)
	    {
	        (void) printf("i_make_hypersurface_boundary returns NULL"
			      "(9)\n");
	    }
	    return NULL;
	}
	Hyper_surf_bdry(hsb) = hsb;
	hsb->interface = cur_intfc;
	return hsb;
}		/*end i_make_hypersurface_boundary*/


/*
*			i_make_curve():
*
*	Creates a new curve separating specified components
*	and joining two specified nodes.   The curve consists
*	of one bond joining the two nodes.
*
*	Returns a pointer to the created curve or NULL on error..
*/

/*ARGSUSED*/
EXPORT CURVE *i_make_curve(
	COMPONENT	neg_comp,
	COMPONENT	pos_comp,
	NODE		*start,
	NODE		*end)
{
	CURVE		*curve;
	INTERFACE	*intfc;
	size_t		size_curve;

	if (start==NULL || end==NULL) 
	{
	    if (DEBUG)
	    (void) printf("start %llu end %llu make_curve returns NULL (1)\n",
	              node_number(start),node_number(end));
	    return NULL;
	}
	if ((intfc=start->interface)==NULL || intfc!=end->interface)
	{
	    if (DEBUG)
	    {
	        (void) printf("intfc: start %llu end %llu ",
	        	  interface_number(intfc),
	        	  interface_number(end->interface));
	        (void) printf("make_curve returns NULL (2)\n");
	    }
	    return NULL;
	}
	if (intfc != cur_intfc)
	{
	    if (DEBUG)
	    {
	        (void) printf("intfc %llu cur %llu ",
	        	  interface_number(intfc),
	        	  interface_number(cur_intfc));
	        (void) printf("make_curve returns NULL (3)\n");
	    }
	    return NULL;

	}
	size_curve =  i_user_interface(intfc).size_curve;

	if ((curve = (CURVE *)store(size_curve)) == NULL)
	{
	    if (DEBUG)
		(void) printf("make_curve returns NULL (4)\n");
	    return NULL;
	}

	if (! add_to_pointers(curve,&start->out_curves))
	{
	    if (DEBUG)
		(void) printf("make_curve returns NULL (5)\n");
	    return NULL;
	}
	if (! add_to_pointers(curve,&end->in_curves))
	{
	    if (DEBUG)
		(void) printf("make_curve returns NULL (6)\n");
	    return NULL;
	}
	if (! add_to_pointers(curve,&intfc->curves))
	{
	    if (DEBUG)
		(void) printf("make_curve returns NULL (7)\n");
	    return NULL;
	}

	curve->interface = intfc;
	curve->obj = curve;
	curve->redist_order = 1;	/* default */
	switch (intfc->dim)
	{
	case 2:
	    Hyper_surf(curve) = make_hypersurface(neg_comp,pos_comp);
	    Curve_of_hs(Hyper_surf(curve)) = curve;
	    curve->interface->num_points += 2;
	    curve->orientation = 0;
	    break;
	case 3:
	    Hyper_surf_bdry(curve) = make_hypersurface_boundary();
	    Curve_of_hsb(Hyper_surf_bdry(curve)) = curve;
	    break;
	default:
	    break;
	}
	curve->start = start;
	curve->end = end;
	if ((curve->first = Bond(start->posn,end->posn)) == NULL)
	{
	    return NULL; 
	}
	if (intfc->dim == 2)
	{
	    curve->first->start->hse = Hyper_surf_element(curve->first);
	    curve->first->end->hse = Hyper_surf_element(curve->first);
	    curve->first->start->hs = Hyper_surf(curve);
	    curve->first->end->hs = Hyper_surf(curve);
	}
	cur_intfc->modified = YES;
	curve->last = curve->first;
	curve->num_points = 2;
	set_not_bdry(curve);
	return curve;
}		/*end i_make_curve*/



/*
*				i_copy_curve():
*
*	Returns a copy of a specified curve.   The copy will be allocated
*	from space in the current interface and will be installed in
*	that interface.   The node arguments are nodes in the current
*	interface which should already have been copied using copy_node().
*	Valid only for a complete curve - i.e. all bonds have valid
*	and matching points at each end.
*
*	Returns a pointer to the new curve or NULL on error.
*
*/

EXPORT CURVE *i_copy_curve(
	CURVE		*curve,
	NODE		*start,
	NODE		*end)
{
	CURVE		*new_curve;
	BOND		*b,*bnew;
	COMPONENT	neg_comp = NO_COMP, pos_comp = NO_COMP;

	if (curve->interface->dim == 2)
	{
	    neg_comp = negative_component(curve);
	    pos_comp = positive_component(curve);
	}

	        /* Allocate a New CURVE structure */

	if (curve==NULL || start==NULL || end==NULL)
	{
	    return NULL;
	}
	if (start->interface!=cur_intfc || end->interface!=cur_intfc)
	{
	    return NULL;
	}
	if ((new_curve = make_curve(neg_comp,pos_comp,start,end)) == NULL)
	{
	    return NULL;
	}

	        /* Copy over all of the POINTS: */

	for (b=curve->first,bnew=new_curve->first; 
	        	b!=curve->last; b=b->next, bnew = bnew->next)
	{
	    if (insert_point_in_bond(copy_point(b->end),bnew,new_curve) !=
		FUNCTION_SUCCEEDED)
	    {
	        screen("ERROR in i_copy_curve(), "
		       "insert_point_in_bond() failed\n");
	        clean_up(ERROR);
	    }
	    bnew->length0 = b->length0;
	}
	new_curve->last->length0 = curve->last->length0;

	if (is_bdry(curve))
	    set_is_bdry(new_curve);
	else
	    set_not_bdry(new_curve);
	if (curve->interface->dim == 2)
	{
	    user_copy_hyper_surf(Hyper_surf(new_curve),Hyper_surf(curve));
	    new_curve->orientation = curve->orientation;
	    new_curve->redist_order = curve->redist_order;
	}
	new_curve->extra = curve->extra;
	return new_curve;
}		/*end i_copy_curve*/



/*
*			i_delete_curve():
*
*	Deletes a curve and its entry in its interface.   
*	Deletes the curve from the in_curves and out_curves of its
*	end-nodes.
*	Returns 1 if successful, or 0  on error.
*/


EXPORT int i_delete_curve(
	CURVE	*curve)
{
	INTERFACE *intfc;
	int	  status = 1;

	if (curve==NULL || curve->interface==NULL || 
	        		curve->interface->curves==NULL)
	    return 0;

	intfc = curve->interface;
	if (intfc->dim == 3)
	{
	    SURFACE **surf;
	    for (surf = curve->pos_surfaces; surf && *surf; ++surf)
	    {
		(void) delete_from_pointers(curve,&(*surf)->pos_curves);
	    }
	    for (surf = curve->neg_surfaces; surf && *surf; ++surf)
	    {
		(void) delete_from_pointers(curve,&(*surf)->neg_curves);
	    }
	}

	status *= delete_from_pointers(curve,&intfc->curves);
	status *= delete_from_pointers(curve,&curve->start->out_curves);
	status *= delete_from_pointers(curve,&curve->end->in_curves);
	if (Hyper_surf(curve) != NULL)
	    status *= delete_from_pointers(Hyper_surf(curve),
	                                   &hyper_surf_list(intfc));
	if (Hyper_surf_bdry(curve) != NULL)
	    status *= delete_from_pointers(Hyper_surf_bdry(curve),
	                                   &hyper_surf_bdry_list(intfc));

	if (intfc->dim == 2)
	    intfc->num_points -= curve->num_points;
	intfc->modified = YES;

	if (intfc->dim == 2)
	    Hyper_surf(curve)->interface = NULL;
	curve->interface = NULL;
	return status;
}		/*end i_delete_curve*/

/*
*			fprint_hypersurface():
*
*		Prints a given hypersurface.
*/

EXPORT void print_hypersurface(
	HYPER_SURF	*hs)
{
	fprint_hypersurface(stdout,hs);
}		/*end print_hypersurface*/

EXPORT void fprint_hypersurface(
	FILE		*file,
	HYPER_SURF	*hs)
{
	(void) fprintf(file,"\tHypersurface %llu:\n",hypersurface_number(hs));
	if (hs == NULL)
	    (void) fprintf(file,"\t\tNULL Hypersurface\n");
	else
	{
	    switch (hs->interface->dim)
	    {
	    case 1:
	        break;
	    case 2:
	        fprint_curve(file,Curve_of_hs(hs));
	        break;
	    case 3:
	        fprint_surface(file,Surface_of_hs(hs));
	        break;
	    }
	}
	(void) fprintf(file,"\tEnd of Hypersurface\n\n");
}		/*end fprint_hypersurface*/


/*
*			fprint_hypersurface_boundaries():
*
*		Prints a given hypersurface boundary list.
*/

EXPORT	void	print_hypersurface_boundaries(
	HYPER_SURF_BDRY	**hsb)
{
	fprint_hypersurface_boundaries(stdout,hsb);
}		/*end print_hypersurface_boundaries*/

LOCAL void fprint_hypersurface_boundaries(
	FILE		*file,
	HYPER_SURF_BDRY	**hsb)
{
	(void) fprintf(file,"\tHypersurface boundary list %p:\n",(POINTER)hsb);
	if ((hsb == NULL) || (*hsb == NULL))
	    (void) fprintf(file,"\t\tNULL Hypersurface boundary list\n");
	else
	{
	    switch ((*hsb)->interface->dim)
	    {
	    case 1:
	        break;
	    case 2:
	        fprint_node(file,Node_of_hsb(*hsb));
	        break;
	    case 3:
	        {
	            int i;

	            for (i = 0; hsb[i] != NULL; ++i)
	            {
	    	        (void) fprintf(file,"\t\tBounding curve %d\n",i);
	    	        fprint_curve(file,Curve_of_hsb(hsb[i]));
	    	        (void) fprintf(file,"\t\tEnd Bounding curve %d\n\n",i);
	    	    }
	    	}
	    	break;
	    }
	}
	(void) fprintf(file,"\tEnd of Hypersurface boundary list\n\n");
}		/*end fprint_hypersurface_boundaries*/



/*
*				i_fprint_curve():
*
*		Prints a given curve, showing all of its bonds.
*/

LIB_LOCAL void i_fprint_curve(
	FILE		*file,
	CURVE		*curve)
{
	BOND		*bond;
	FT_POINT		*p;
	int		i, dim;
	int		k;
	const char	*endchar;
	SURFACE		**surf;

	(void) fprintf(file,"\tCurve %llu:\n",curve_number(curve));
	if (curve == NULL)
	{
	    (void) fprintf(file,"\t\tNULL Curve\n\tEnd of Curve\n\n");
	    return;
	}
	if (curve->interface == NULL)
	{
	    (void) fprintf(file,"\t\tDELETED Curve\n\tEnd of Curve\n\n");
	    return;
	}
	dim = curve->interface->dim;
	switch (dim)
	{
	case 2:
	    (void) fprintf(file,"\tHypersurface of curve = %llu\n",
	    	       hypersurface_number(Hyper_surf(curve)));
	    (void) fprintf(file,
	    	"\tLeft Component = %-4d   Right Component = %-4d    ",
	    negative_component(curve),positive_component(curve));
	    break;
	case 3:
	    (void) fprintf(file,"\tBoundary Status:   ");
	    break;
	}
	(void) fprintf(file,"%s\n",
	           is_bdry(curve) ? "Boundary Curve" : "Interior Curve");
	(void) fprintf(file,"\tStart Node = %-8llu   End Node = %-8llu\n",
	           node_number(curve->start),node_number(curve->end));
	if (dim == 3)
	{
	    (void) fprintf(file,"\t%d Surfaces at Curve-> ",
	    	           (int)Num_surfaces_bding_curve(curve));
	    if (Btris(curve->first) == NULL)
	    (void) fprintf(file,"NULL");
	    else
	    {
	    	BOND_TRI **btris = Btris(curve->first);

	    	for (; btris && *btris; ++btris)
	    	    (void) fprintf(file,"%llu ",
	            surface_number(Surface_of_tri((*btris)->tri)));
	    }
	    (void) fprintf(file,"\n");
	    (void) fprintf(file,"\t%d Pos_surfaces->  ",
	    	        (int)Num_pos_surfaces_of_curve(curve));
	    if ((surf = curve->pos_surfaces) == NULL) 
	    	(void) fprintf(file,"NULL");
	    else 
	    	while (*surf)
	        (void) fprintf(file,"%llu ",surface_number(*surf++));
	    (void) fprintf(file,"\n");
	    (void) fprintf(file,"\t%d Neg_surfaces->  ",
	    	        (int)Num_neg_surfaces_of_curve(curve));
	    if ((surf = curve->neg_surfaces) == NULL) 
	    (void) fprintf(file,"NULL");
	    else 
	    while (*surf)
	        (void) fprintf(file,"%llu ",surface_number(*surf++));
	    (void) fprintf(file,"\n");
	    (void) fprintf(file,"\tEnd Bounding Surface Data\n");
	}
	(void) fprintf(file,"\t%d Points on Curve\n",curve->num_points);

	if (is_binary_output() == YES)
	{
	    for (i = 0,bond = curve->first; bond != NULL; ++i,bond = bond->next)
	    {
	    	if (i%4 == 0)
	    	{
	    	    int remaining = curve->num_points - i;
	    	    (void) fprintf(file,"\f%c",dim*min(4,remaining));
	    	}
	    	(void) fwrite((const void*)Coords(bond->start),FLOAT,dim,file);
	    }
	    if (i%4 == 0)
	        (void) fprintf(file,"\f%c",dim);
	    (void) fwrite((const void*)Coords(curve->last->end),FLOAT,dim,file);
	}
	else
	{
	    int bonds_per_line;
	    bonds_per_line = (dim == 2) ? 4 : 2;
	    (void) fprintf(file,"\n\t");
	    for (i = 1,bond = curve->first; bond != NULL; bond = bond->next,++i)
	    {
	    	p = bond->start;
	    	endchar = i%bonds_per_line ? " " : "\n\t";
	    	if (p == NULL)
	    	    (void) fprintf(file,"NULL ->%s",endchar);
	        else
	        {
	            for (k = 0; k < dim; ++k)
	                (void) fprintf(file,"%- "FFMT" ",Coords(p)[k]);
	            (void) fprintf(file,"->%s",endchar);
	        }
	    }

	    if ((curve->last != NULL) && (curve->last->end != NULL))
	    {
	    	p = curve->last->end;
	    	for (k = 0; k < dim; ++k)
	    	    (void) fprintf(file,"%- "FFMT" ",Coords(p)[k]);
	    	(void) fprintf(file,"\n");
	    }
	}
	if (dim == 3)
	    fprint_tris_on_curve(file,curve);


	user_fprint_curve(file,curve);
	(void) fprintf(file,"\tEnd of Curve\n\n");
}		/*end i_fprint_curve*/

EXPORT boolean  change_node_of_closed_curve(
	FT_POINT	 *p,
	CURVE    *c)
{
	NODE	    *n;
	CURVE	    **cc;
	BOND	    *b;
	int	    k;
	boolean	    found;

	
	/*make sure node is the node of the closed curve c */
	if(!is_closed_curve(c))
	{
	    printf("ERROR change_node_of_closed_curve, ");
	    printf("curve is not closed.\n ");
	    clean_up(ERROR);
	}
	n = c->start;
	if(n != c->end)
	{
	    printf("ERROR change_node_of_closed_curve, ");
	    printf("node of curve is not closed.\n ");
	    clean_up(ERROR);
	}

	/*make sure n is used only by curve c */
	found = NO;
	for(k=0, cc=n->in_curves; cc && *cc; cc++, k++)
	    if(*cc == c)
	        found = YES;
	if(!found || k!=1)
	{
	    printf("ERROR change_node_of_closed_curve, ");
	    printf("node is tangled for in_curves.\n ");
	    clean_up(ERROR);
	}
	found = NO;
	for(k=0, cc=n->out_curves; cc && *cc; cc++, k++)
	    if(*cc == c)
	        found = YES;
	if(!found || k!=1)
	{
	    printf("ERROR change_node_of_closed_curve, ");
	    printf("node is tangled for out_curves.\n ");
	    clean_up(ERROR);
	}

	if(p == c->first->start)
	    return YES;

	found = NO;
	for(b=c->first; b != c->last; b=b->next)
	    if(b->end == p)
	    {
	        found = YES;
	        break;
	    }
	if(!found)
	{
	    printf("ERROR change_node_of_closed_curve, ");
	    printf("point is not on curve.\n ");
	    clean_up(ERROR);
	}

	n->posn = p;
	c->first->prev = c->last;
	c->last->next = c->first;
	
	c->first = b->next;
	c->first->prev = NULL;
	c->last = b;
	c->last->next = NULL;

	return YES;
}

/*
*				split_curve():
*
*	Splits a curve into two curves.   The POINT of splitting  p must
*	be on BOND b of the CURVE curve.   The point becomes a NODE
*	and two new curves are defined meeting there.   The old curve
*	is deleted.  The first curve will have COMPONENTS left1,
*	right1 on it, while the second will separate COMPONENTS
*	left2,right2.   This allows for changing topology.
*
*
*	Note: This routine can produce BONDS of zero length.
*
*	Returns a pointer to the new pair of new curves, or 
*	NULL on error.
*/

EXPORT CURVE **split_curve(
	FT_POINT	  *p,
	BOND	  *bond,
	CURVE	  *curve,
	COMPONENT ncomp1,
	COMPONENT pcomp1,
	COMPONENT ncomp2,
	COMPONENT pcomp2)
{
	switch(curve->interface->dim)
	{
	case 1:
	case 2:
	    return split_curve2d(p,bond,curve,ncomp1,pcomp1,ncomp2,pcomp2);
	case 3:    
	    return split_curve3d(p,bond,curve,ncomp1,pcomp1,ncomp2,pcomp2);
	default:
	    printf("ERROR split_curve, invalid dimension %d\n", curve->interface->dim);
	    clean_up(ERROR);
	}
}

/*ARGSUSED*/
LOCAL  CURVE **split_curve2d(
	FT_POINT	  *p,
	BOND	  *bond,
	CURVE	  *curve,
	COMPONENT ncomp1,
	COMPONENT pcomp1,
	COMPONENT ncomp2,
	COMPONENT pcomp2)
{
	int		is_a_node;
	NODE		*node;
	BOND		*c0first, *c0last, *c1first, *c1last;
	BOND		Btmp;
	static CURVE	*curves[2] = {NULL, NULL};
	int      	i;

	if (DEBUG)
	       (void) printf("Entered split_curve(%llu)\n",curve_number(curve));

	if ((bond == NULL) || (curve == NULL) ||
	    (curve->interface != cur_intfc))
	{
	    if (DEBUG)
	    {
	    	(void) printf("split_curve returning NULL\n");
	    	(void) printf("bond=%llu curve=%llu ",
	    		      bond_number(bond,curve->interface),
	    		      curve_number(curve));
	    	(void) printf("curve->interface=%llu cur_intfc=%llu\n",
	    		      interface_number(curve->interface),
	    		      interface_number(cur_intfc));
	    }
	    return NULL;
	}
	if ((p == curve->start->posn && bond == curve->first) ||
	    (p == curve->end->posn && bond == curve->last))
	{
	    if (DEBUG)
	    {
	    	(void) printf("split_curve returning NULL, "
	    	              "can't split at start or end of curve\n");
	    }
	    return NULL;	/* don't split a curve at its start or end */
	}

	    	/* Make sure bond->end == p == node->posn: */
	if (p == bond->start)
	    bond = bond->prev;
	    	/* Add point to curve and make it a node: */
	if (p != bond->end)	/* then p is not already on the bond */
	{
	    if (insert_point_in_bond(p,bond,curve) != FUNCTION_SUCCEEDED)
	    {
	    	if (DEBUG)
	    	{
	    	    (void) printf("split_curve returning NULL "
	    	                  "insert_point_in_bond failed\n");
	    	}
	    	return NULL;
	    }
	}
	is_a_node = ((node = node_of_point(p,curve->interface)) != NULL);
	if (! is_a_node && (node = make_node(p)) == NULL)
	{
	    if (DEBUG)
	    {
	    	(void) printf("split_curve returning NULL\n");
	    	(void) printf("make_node() failed\n");
	    }
	    return NULL;
	}
	if (is_bdry(curve))
	    set_is_bdry(node);
	else
	    set_not_bdry(node);

	c0first = curve->first;		c0last = bond;
	c1first = bond->next;		c1last = curve->last;
	Btmp.start = curve->first->start;
	Btmp.end = curve->last->end;
	Btmp.prev = Btmp.next = NULL;
	curve->first = curve->last = &Btmp;

	    	/* Copy curves: */
	curves[0] = copy_curve(curve,curve->start,node);
	if (curves[0] == NULL)
	{
	    if (DEBUG)
	    {
	    	(void) printf("split_curve returning NULL\n");
	    	(void) printf("curves[0]=copy_curve() returned NULL\n");
	    }
	    return NULL;
	}
	curves[0]->first = c0first;
	curves[0]->last = c0last;

	curves[1] = copy_curve(curve,node,curve->end);
	if (curves[1] == NULL)
	{
	    if (DEBUG)
	    {
	    	(void) printf("split_curve returning NULL\n");
	    	(void) printf("curves[1]=copy_curve() returned NULL\n");
	    }
	    return NULL;
	}
	curves[1]->first = c1first;
	curves[1]->last = c1last;

	if (curve->interface->dim == 2) 
	{
	    negative_component(curves[0]) = ncomp1;
	    positive_component(curves[0]) = pcomp1;
	    negative_component(curves[1]) = ncomp2;
	    positive_component(curves[1]) = pcomp2;
	}

	curve->first = c0first;	curve->last = c1last;
	if (user_split_curve(is_a_node,p,bond,curve,curves) != YES)
	{
	    return NULL;
	}
	/* Set 2D curve orientation */
	if (curve->interface->dim == 2)
	{
	    BOND *b;
	    for (i = 0; i < 2; ++i)
	    {
		if (is_closed_curve(curves[i]))
		{
		    if (curve->orientation != 0)
			curves[i]->orientation = curve->orientation;
		    else
			curves[i]->orientation = 
			    (area_of_closed_curve(curves[i]) > 0.0) ? 1 : -1;
		}
		curves[i]->first->start->hs = Hyper_surf(curves[i]);
		for (b = curves[i]->first; b != NULL; b = b->next)
		    b->end->hs = Hyper_surf(curves[i]);
	    }
	}

	(void) delete_curve(curve);
	c0last->next = c1first->prev = NULL;

	/* Count number of points on curves[0] && curves[1] */
	curves[0]->num_points = num_points_on_curve(curves[0]);
	curves[1]->num_points = num_points_on_curve(curves[1]);
	curves[0]->interface->num_points +=
	    curves[0]->num_points + curves[1]->num_points - 4;

	if (DEBUG) (void) printf("Left split_curve, returning %llu %llu\n\n",
	    		 curve_number(curves[0]),
	    		 curve_number(curves[1]));
	return curves;
}		/*end split_curve*/


LOCAL  CURVE **split_curve3d(
	FT_POINT	  *p,
	BOND	  *bond,
	CURVE	  *curve,
	COMPONENT ncomp1,
	COMPONENT pcomp1,
	COMPONENT ncomp2,
	COMPONENT pcomp2)
{
	int		is_a_node;
	NODE		*node;
	BOND		*c0first, *c0last, *c1first, *c1last;
	BOND		Btmp;
	static CURVE	*curves[2] = {NULL, NULL};
	COMPONENT	neg_comp=NO_COMP, pos_comp=NO_COMP;

	if (DEBUG)
	       (void) printf("Entered split_curve(%llu)\n",curve_number(curve));

	if ((bond == NULL) || (curve == NULL) ||
	    (curve->interface != cur_intfc))
	{
	    if (DEBUG)
	    {
	    	(void) printf("split_curve returning NULL\n");
	    	(void) printf("bond=%llu curve=%llu ",
	    		      bond_number(bond,curve->interface),
	    		      curve_number(curve));
	    	(void) printf("curve->interface=%llu cur_intfc=%llu\n",
	    		      interface_number(curve->interface),
	    		      interface_number(cur_intfc));
	    }
	    return NULL;
	}
	if ((p == curve->start->posn && bond == curve->first) ||
	    (p == curve->end->posn && bond == curve->last))
	{
	    if (DEBUG)
	    {
	    	(void) printf("split_curve returning NULL, "
	    	              "can't split at start or end of curve\n");
	    }
	    return NULL;	/* don't split a curve at its start or end */
	}

	    	/* Make sure bond->end == p == node->posn: */
	if (p == bond->start)
	    bond = bond->prev;
	    	/* Add point to curve and make it a node: */
	if (p != bond->end)	/* then p is not already on the bond */
	{
	    /*#bjet2 */
	    printf("ERROR: in split_curve: new bond made.\n");
	    clean_up(ERROR);

	    if (insert_point_in_bond(p,bond,curve) != FUNCTION_SUCCEEDED)
	    {
	    	if (DEBUG)
	    	{
	    	    (void) printf("split_curve returning NULL "
	    	                  "insert_point_in_bond failed\n");
	    	}
	    	return NULL;
	    }
	}
	is_a_node = ((node = node_of_point(p,curve->interface)) != NULL);

	if (! is_a_node && (node = make_node(p)) == NULL)
	{
	    if (DEBUG)
	    {
	    	(void) printf("split_curve returning NULL\n");
	    	(void) printf("make_node() failed\n");
	    }
	    return NULL;
	}
	if (is_bdry(curve))
	    set_is_bdry(node);
	else
	    set_not_bdry(node);

	    	/* Copy curves: */
	
	if ((curves[0] = make_curve(neg_comp,pos_comp,curve->start,node)) == NULL)
	{
	    printf("ERROR in split_curve: make_first curve fails.\n");
	    clean_up(ERROR);
	}
	curves[0]->first = curve->first;
	curves[0]->last = bond;
	
	if ((curves[1] = make_curve(neg_comp,pos_comp,node,curve->end)) == NULL)
	{
	    printf("ERROR in split_curve: make second_curve fails.\n");
	    clean_up(ERROR);
	}
	curves[1]->first = bond->next;
	curves[1]->last = curve->last;
	
	if (user_split_curve(is_a_node,p,bond,curve,curves) != YES)
	{
	    return NULL;
	}


	/*Reset bond tris and surface pointers*/
	if (curve->interface->dim == 3)
	{
	    BOND     *b;
	    BOND_TRI **btris;
	    SURFACE  **s;
	    int      i;

	    for (s = curve->pos_surfaces; s && *s; ++s)
	    {
		for (i = 0; i < 2; ++i)
		{
		    install_curve_in_surface_bdry(*s,curves[i],
						  POSITIVE_ORIENTATION);
		}
	    }
	    for (s = curve->neg_surfaces; s && *s; ++s)
	    {
		for (i = 0; i < 2; ++i)
		{
		    install_curve_in_surface_bdry(*s,curves[i],
						  NEGATIVE_ORIENTATION);
		}
	    }

	    curves[0]->last->next = NULL;
	    curves[1]->first->prev = NULL;

	    /*reset the curve pointers in BOND_TRI */
	    for (b = curves[0]->first; b; b = b->next)
	    {
		for (btris = Btris(b); btris && *btris; ++btris)
		    if((*btris)->curve != NULL)
		    	(*btris)->curve = curves[0];
	    }
	    
	    for (b = curves[1]->first; b; b = b->next)
	    {
		for (btris = Btris(b); btris && *btris; ++btris)
		    if((*btris)->curve != NULL)
		    	(*btris)->curve = curves[1];
	    }
	}

	(void) delete_curve(curve);

	/* Count number of points on curves[0] && curves[1] */
	curves[0]->num_points = num_points_on_curve(curves[0]);
	curves[1]->num_points = num_points_on_curve(curves[1]);
	
	if (DEBUG) 
	      (void) printf("Left split_curve, returning %llu %llu\n\n",
	    		 curve_number(curves[0]),
	    		 curve_number(curves[1]));

	return curves;
}		/*end split_curve*/



/*
*				join_curves():
*
*	Joins two curves that meet at a common node to form a 
*	single curve.   The NODE's position, while still on the CURVE,
*	ceases to be regarded as a NODE for that CURVE.  It
*	is now simply an ordinary point of the CURVE.
*	The resulting CURVE has COMPONENTS left, right on 
*	either side.   This allows for changing topolgy.
*
*	Returns a pointer to the new curve if succesful or NULL
*	on error.  If bond_at_join is not NULL  it will return
*	the address of the bond whose starting point coincides
*	with the position of the common node at which the
*	two curves were joined.
*/

EXPORT CURVE *join_curves(
	CURVE		*curve1,
	CURVE		*curve2,
	COMPONENT	left,
	COMPONENT	right,
	BOND		**bond_at_join)
{
	CURVE		*curve;

	debug_print("joinc","Entered join_curves(%llu,%llu)\n",
	      curve_number(curve1),curve_number(curve2));
	if (debugging("joinc"))
	{
	    (void) printf("curve1\n");
	    print_curve(curve1);
	    (void) printf("curve2\n");
	    print_curve(curve2);
	}

	if (bond_at_join != NULL)
	    *bond_at_join = NULL; /*Default*/

	    /* Check consistency of data */
	if (curve1==NULL || curve2==NULL || curve1->interface!=cur_intfc 
	    || curve1->interface!=curve2->interface
	    || curve1->end!=curve2->start)
	{
	    if (debugging("joinc"))
	    {
	        (void) printf("c1 %llu ",curve_number(curve1));
	        (void) printf("intfc %llu ",
	    		  interface_number(curve1->interface));
	        (void) printf("c2 %llu ",curve_number(curve2));
	        (void) printf("intfc %llu ",
	    		  interface_number(curve2->interface));
	        (void) printf("cur_intfc %llu\n",
	    		  interface_number(cur_intfc));
	        (void) printf("join_curves returning NULL\n");
	    }
	    debug_print("joinc","Left join_curves()\n\n");
	    return NULL;
	}

	if (( is_bdry(curve1) && !is_bdry(curve2)) ||
	    (!is_bdry(curve1) &&  is_bdry(curve2)))
	{
	    if (debugging("joinc"))
	    {
	        (void) printf("Incompatible boundaries in join curves\n");
	        (void) printf("boundary: c1 %d c2 %d\n",
	    		  is_bdry(curve1),is_bdry(curve2));
	        (void) printf("join_curves returning NULL\n");
	    }
	    debug_print("joinc","Left join_curves()\n\n");
	    return NULL;
	}
	if (curve1 == curve2)
	{
	    if (curve1->interface->dim == 2)
	    {
	    	positive_component(curve1) = right;
	    	negative_component(curve1) = left;
	    }
	    if (bond_at_join != NULL)
	    	*bond_at_join = curve2->first;
	    debug_print("joinc","Left join_curves()\n\n");
	    return curve1;
	}

	if ((curve = make_curve(left,right,curve1->start,curve2->end))==NULL)
	{
	    if (debugging("joinc"))
	        (void) printf("make_curve fails, join_curves returning NULL\n");
	    debug_print("joinc","Left join_curves()\n\n");
	    return NULL;
	}

	if (is_bdry(curve1))
	    set_is_bdry(curve);
	else
	    set_not_bdry(curve);
	curve->first = curve1->first;
	curve->last = curve2->last;
	curve1->last->next = curve2->first;
	if (bond_at_join != NULL)
	    *bond_at_join = curve2->first;
	curve2->first->prev = curve1->last;
	curve->num_points += (curve1->num_points + curve2->num_points - 3);
	curve->interface->num_points += 
	    (curve1->num_points + curve2->num_points - 3);

	if (curve1->interface->dim == 3)
	{
	    /* Reset surface && curve pointers to joined curve */

	    POINTER p, *ps, *ns, *pc, *nc;
	    SURFACE **s;

	    ps = NULL;
	    ns = NULL;
	    for (s = curve1->pos_surfaces; s && *s; ++s)
	    {
		if (!unique_add_to_pointers(*s,&ps))
		{
		    screen("ERROR in join_curves(), add_to_pointers() failed "
			   "for curve1 pos_surfaces\n");
		    clean_up(ERROR);
		}
		pc = (POINTER*)(*s)->pos_curves;
		p = (POINTER)curve1;
		if (!delete_from_pointers_if_present(p,&pc))
		{
		    screen("ERROR in join_curves(), delete_from_pointers() "
			   "for surface in curve1 pos_surfaces\n");
		    clean_up(ERROR);
		}
		p = (POINTER)curve;
		if (!unique_add_to_pointers(p,&pc))
		{
		    screen("ERROR in join_curves(), unique_add_to_pointers() "
			   "for surface in curve1 pos_surfaces\n");
		    clean_up(ERROR);
		}
		(*s)->pos_curves = (CURVE**)pc;
	    }
	    for (s = curve2->pos_surfaces; s && *s; ++s)
	    {
		if (!unique_add_to_pointers(*s,&ps))
		{
		    screen("ERROR in join_curves(), unique_add_to_pointers() "
			   "failed for curve2 pos_surfaces\n");
		    clean_up(ERROR);
		}
		pc = (POINTER*)(*s)->pos_curves;
		p = (POINTER)curve2;
		if (!delete_from_pointers_if_present(p,&pc))
		{
		    screen("ERROR in join_curves(), delete_from_pointers() "
			   "for surface in curve2 pos_surfaces\n");
		    clean_up(ERROR);
		}
		p = (POINTER)curve;
		if (!unique_add_to_pointers(p,&pc))
		{
		    screen("ERROR in join_curves(), unique_add_to_pointers() "
			   "for surface in curve2 pos_surfaces\n");
		    clean_up(ERROR);
		}
		(*s)->pos_curves = (CURVE**)pc;
	    }
	    for (s = curve1->neg_surfaces; s && *s; ++s)
	    {
		if (!unique_add_to_pointers(*s,&ns))
		{
		    screen("ERROR in join_curves(), add_to_pointers() failed "
			   "for curve1 neg_surfaces\n");
		    clean_up(ERROR);
		}
		nc = (POINTER*)(*s)->neg_curves;
		p = (POINTER)curve1;
		if (!delete_from_pointers_if_present(p,&nc))
		{
		    screen("ERROR in join_curves(), delete_from_pointers() "
			   "for surface in curve1 neg_surfaces\n");
		    clean_up(ERROR);
		}
		p = (POINTER)curve;
		if (!unique_add_to_pointers(p,&nc))
		{
		    screen("ERROR in join_curves(), unique_add_to_pointers() "
			   "for surface in curve1 neg_surfaces\n");
		    clean_up(ERROR);
		}
		(*s)->neg_curves = (CURVE**)nc;
	    }
	    for (s = curve2->neg_surfaces; s && *s; ++s)
	    {
		if (!unique_add_to_pointers(*s,&ns))
		{
		    screen("ERROR in join_curves(), unique_add_to_pointers() "
			   "failed for curve2 neg_surfaces\n");
		    clean_up(ERROR);
		}
		nc = (POINTER*)(*s)->neg_curves;
		p = (POINTER)curve2;
		if (!delete_from_pointers_if_present(p,&nc))
		{
		    screen("ERROR in join_curves(), delete_from_pointers() "
			   "for surface in curve2 neg_surfaces\n");
		    clean_up(ERROR);
		}
		p = (POINTER)curve;
		if (!unique_add_to_pointers(p,&nc))
		{
		    screen("ERROR in join_curves(), add_to_pointers() "
			   "for surface in curve2 neg_surfaces\n");
		    clean_up(ERROR);
		}
		(*s)->neg_curves = (CURVE**)nc;
	    }
	    curve->pos_surfaces = (SURFACE**)ps;
	    curve->neg_surfaces = (SURFACE**)ns;
	}
	else if (curve1->interface->dim == 2)
	{
	    BOND *b = curve->first; 
	    b->start->hse = Hyper_surf_element(b);
	    b->start->hs = Hyper_surf(curve);
	    for (; b != NULL; b = b->next)
	    {
	    	b->end->hse = Hyper_surf_element(b);
	    	b->end->hs = Hyper_surf(curve);
	    }
	    if (is_closed_curve(curve))
	    {
		if (curve1->orientation != 0)
		    curve->orientation = curve1->orientation;
		else if (curve2->orientation != 0)
		    curve->orientation = curve2->orientation;
		else
		    curve->orientation = 
				(area_of_closed_curve(curve) > 0.0) ? 1 : -1;
	    }
	}

	if (!user_join_curves(curve,curve1,curve2))
	{
	    debug_print("joinc","Left join_curves()\n\n");
	    return NULL;
	}
	(void) delete_curve(curve1);
	(void) delete_curve(curve2);

	if (debugging("joinc"))
	{
	    (void) printf("joined curve\n");
	    print_curve(curve);
	}
	debug_print("joinc","Left join_curves()\n\n");
	return curve;
}		/*end join_curves*/



/*
*			i_Bond():
*
*	Allocates and returns a pointer to a Bond in the current interface.
*/

EXPORT BOND *i_Bond(
	FT_POINT		*p1,
	FT_POINT		*p2)
{
	BOND		*b;

	b = (BOND *) store(i_user_interface(cur_intfc).size_bond);
	if (b == NULL) 
	{ 
	    return NULL;
	}
	b->prev = b->next = NULL;
	b->start = p1;
	b->end   = p2;
	p1->hse = p2->hse = Hyper_surf_element(b);
	b->length = separation(p1,p2,cur_intfc->dim);
	if (cur_intfc->dim == 3)
	{
	    Btris(b) = NULL;
	    Boundary_point(p1) = Boundary_point(p2) = 1;
	}
	return b;
}		/*end i_Bond*/


/*
*			i_make_point():
*
*       Allocates a point structure in the current interface and
*       installs it in the point-list of the interface.  The
*       point will have position  p  which may be NULL.  
*
*	This function is separate from i_Point() in order to support
*	the component arguments, although i_Point() is used as the basic
*	storage allocator.  This function can only be used in ONED.
*
*       Returns a pointer to the new point, or NULL on error.
*/

EXPORT FT_POINT *i_make_point(
	double		*coords,
	COMPONENT	neg_comp,
	COMPONENT	pos_comp)
{
	FT_POINT *newpoint;
	FT_POINT **p, *antep;
	int   i;

	if (cur_intfc->dim != 1)
	    return NULL;
	if ((newpoint = Point(coords)) == NULL)
	    return NULL;

	Hyper_surf(newpoint) = make_hypersurface(neg_comp,pos_comp);
	if (Hyper_surf(newpoint) == NULL)
	    return NULL;
	Point_of_hs(Hyper_surf(newpoint)) = newpoint;
	newpoint->obj = newpoint;
	newpoint->interface = cur_intfc;

	for (antep = NULL, p = cur_intfc->points; p && *p; ++p)
	{
	    if (coords[0] > Coords(*p)[0])
		antep = *p;
	    else
		break;
	}
	if (!add_to_ordered_pointers(newpoint,antep,&cur_intfc->points))
	    return NULL;
	if (antep == NULL)/*move new point to start of array*/
	{
	    for (i = cur_intfc->num_points; i > 0; --i)
	        cur_intfc->points[i] = cur_intfc->points[i-1];
	    cur_intfc->points[0] = newpoint;
	}
	    					    
	cur_IT->max_comp = max(cur_IT->max_comp,max(neg_comp,pos_comp));
	cur_IT->min_comp = min(cur_IT->min_comp,min(neg_comp,pos_comp));
	++cur_intfc->num_points;
	cur_intfc->modified = YES;
	return newpoint;
}		/*end i_make_point*/



/*
*			i_Point():
*
*	Used to create Storage for a point if needed.   It is assumed
*	that users will generally create their own storage for points.
*	This routine is simply provided for completeness.
*	Returns a pointer to the allocated POINT or NULL if no space.
*/

EXPORT FT_POINT *i_Point(
	double		*coords)
{
	FT_POINT		*p;
	int		j;
	size_t		size_point;

	size_point = i_user_interface(cur_intfc).size_point;
	p = (FT_POINT *)store(size_point);
	if (p == NULL)
	    return NULL;
	if (coords != NULL)
	    for (j = 0; j < cur_intfc->dim; ++j)
	    	Coords(p)[j] = coords[j];
	else
	    for (j = 0; j < cur_intfc->dim; ++j)
	    	Coords(p)[j] = 0.0;
	if (cur_intfc->dim == 3)
	{
	    normal_at_point(p)[0] = HUGE_VAL;
	    normal_at_point(p)[1] = HUGE_VAL;
	    normal_at_point(p)[2] = HUGE_VAL;
	}
	return p;
}		/*end i_Point*/


/*
*			i_Static_point():
*
*	Used to create Storage for a point which can be used locally
*	in a function.  Generally points allocated by this routine
*	should not be inserted into an interface, although this is
*	not an error.  Point() should be used whenever a new point
*	is needed to insert into an interface. 
*	This function differs from Point() in that the storage allocated
*	is static and will not be lost upon removal of an interface.
*	It thus provides a means for the allocation of point structures
*	for the use in functions as "private storage".
*/

/* ARGSUSED */
EXPORT FT_POINT *i_Static_point(
	INTERFACE	*intfc)
{
	FT_POINT		*static_alloced_point = NULL;
	size_t		size_point = 0;

	size_point = i_user_interface(intfc).size_point;
	scalar(&static_alloced_point,size_point);
	if (static_alloced_point == NULL)
	    return NULL;

	return static_alloced_point;
}		/*end i_Static_point*/


/*
*			i_copy_point():
*
*	Copies a point, allocating storage from the current interface.
*	The point is copied by structure ft_assignment.
*
*	Returns a pointer to the new point, or NULL on error.
*/

EXPORT FT_POINT *i_copy_point(
	FT_POINT		*p)
{
	FT_POINT		*newp;
	int		i;

	if (p == NULL)
	    return NULL;
	if (cur_intfc->dim == 1)
	{
	    newp = make_point(Coords(p),negative_component(p),
	    		                positive_component(p));
	    user_copy_hyper_surf(Hyper_surf(newp),Hyper_surf(p));
	}
	else
	    newp = Point(Coords(p));

	Point_flags(newp) = Point_flags(p);
	Boundary(newp) = Boundary(p);

	if (newp == NULL)
	    return NULL;
        newp->curvature = p->curvature;
        for(i = 0; i < 3; ++i)
            newp->_nor[i] = p->_nor[i];

	return newp;
}		/*end i_copy_point*/

/*
*			i_average_points():
*
*	Computes the average of two interface points. Most of the complication
*	of this function is to main consistent stored fields for bond lengths,
*	triangle normals, and other possible stored quantities.
*/

EXPORT	FT_POINT *i_average_points(
	boolean               newpoint,
	FT_POINT		   *p1,
	HYPER_SURF_ELEMENT *hse1,
	HYPER_SURF	   *hs1,
	FT_POINT		   *p2,
	HYPER_SURF_ELEMENT *hse2,
	HYPER_SURF	   *hs2)
{
	FT_POINT     *pmid;
	INTERFACE *intfc;
	double     mid[3];
	int	  i, dim;

	if (hs1 != NULL)
	    dim = hs1->interface->dim;
	else if (hs2 != NULL)
	    dim = hs2->interface->dim;
	else
	    dim = current_interface()->dim;
	for (i = 0; i < dim; ++i)
	      mid[i] = 0.5*(Coords(p1)[i] + Coords(p2)[i]);
	if (newpoint)
	    pmid = Point(mid);
	else
	{
	    pmid = NULL;
	    for (i = 0; i < dim; ++i)
	        Coords(p1)[i] = Coords(p2)[i] = mid[i];
	}

	if ((dim == 2) && !newpoint)
	{
	    BOND               *b;
	    CURVE              *c, **cc;
	    HYPER_SURF         *hs;
	    HYPER_SURF_ELEMENT *hse;
	    NODE               *n;
	    FT_POINT              *p;

	    for (i = 0; i < 2; ++i)
	    {
		if (i == 0)
		{
		    p = p1;
		    hs = hs1;
		    hse = hse1;
		}
		else
		{
		    p = p2;
		    hs = hs2;
		    hse = hse2;
		}
	        if (hse != NULL)
	        {
		    b = Bond_of_hse(hse);
		    set_bond_length(b,dim);
		    if (p == b->start)
		    {
		        if (b->prev)
		            set_bond_length(b->prev,dim);
		        else
		        {
			    c = Curve_of_hs(hs);
			    n = c->start;
			    if ((p != n->posn) || (b != c->first))
			    {
			        screen("ERROR in i_average_points(), "
				       "inconsistent hse%d and hs%d at "
				       "start node\n",i+1,i+1);
			        clean_up(ERROR);
			    }
			    for (cc = n->in_curves; cc && *cc; ++cc)
			        set_bond_length((*cc)->last,dim);
			    for (cc = n->out_curves; cc && *cc; ++cc)
			        set_bond_length((*cc)->first,dim);
		        }
		    }
		    if (p == b->end)
		    {
		        if (b->next)
		            set_bond_length(b->next,dim);
		        else
		        {
			    c = Curve_of_hs(hs);
			    n = c->end;
			    if ((p != n->posn) || (b != c->last))
			    {
			        screen("ERROR in i_average_points(), "
				       "inconsistent hse%d and hs%d at "
				       "end node\n",i+1,i+1);
			        clean_up(ERROR);
			    }
			    for (cc = n->in_curves; cc && *cc; ++cc)
			        set_bond_length((*cc)->last,dim);
			    for (cc = n->out_curves; cc && *cc; ++cc)
			        set_bond_length((*cc)->first,dim);
		        }
		    }
		}
	    }
	}
	if ((dim == 3) && !newpoint)
	{
	  TRI                *tri;
	  HYPER_SURF_ELEMENT *hse;
	  FT_POINT              *p;
	  int                nt, j;
	  TRI                **tt;
	  static int         max_num_tris = 0;
	  static             TRI **tris = NULL;

	  for (i = 0; i < 2; ++i)
	  {
	    if (i == 0)
	    {
	      p = p1;
	      hse = hse1;
	      intfc = (hs1 != NULL) ? hs1->interface : NULL;
	    }
	    else
	    {
	      p = p2;
	      hse = hse2;
	      intfc = (hs2 != NULL) ? hs2->interface : NULL;
	    }
	    if (hse != NULL)
	    {
	      tri = Tri_of_hse(hse);
	      nt = set_tri_list_around_point(p,tri,&tt,intfc);
	      if ((nt+1) > max_num_tris)
	      {
	      	if (tris != NULL)
	      	  free(tris);
	      	max_num_tris = 2*nt + 1;
	      	uni_array(&tris,max_num_tris,sizeof(TRI*));
	      }
	      for (j = 0; j < nt; ++j)
	      {
		tris[j] = tt[j];
		set_normal_of_tri(tris[j]);
	      }
	      if (Boundary_point(p))
	      {
	        BOND     *b, *bb;
	        BOND_TRI **bts;
		CURVE    **cc;
		NODE     *n;
		int      ntt, v, k, l, side;
		for (j = 0; j < nt; ++j)
		{
		  v = Vertex_of_point(tris[j],p);
		  for (l = 0; l < 2; ++l)
		  {
		    side = (l == 0) ? v : Prev_m3(v);
		    if (is_side_bdry(tris[j],side))
		    {
		      b = Bond_on_side(tris[j],side);
		      set_bond_length(b,dim);
	              for (bts = Btris(b); bts && *bts; ++bts)
		      {
	                ntt = set_tri_list_around_point(p,(*bts)->tri,&tt,intfc);
		        for (k = 0; k < ntt; ++k)
		          set_normal_of_tri(tt[k]);
		      }
		      if (p == b->start)
		      {
		        if (b->prev)
		          set_bond_length(b->prev,dim);
		        else
		        {
			  n = Btris(b)[0]->curve->start;
			  for (cc = n->in_curves; cc && *cc; ++cc)
			  {
			    bb = (*cc)->last;
			    set_bond_length(bb,dim);
	                    for (bts = Btris(bb); bts && *bts; ++bts)
		            {
	                      ntt = set_tri_list_around_point(p,(*bts)->tri,
				                              &tt,intfc);
		              for (k = 0; k < ntt; ++k)
		                set_normal_of_tri(tt[k]);
		            }
			  }
			  for (cc = n->out_curves; cc && *cc; ++cc)
			  {
			    bb = (*cc)->first;
			    set_bond_length(bb,dim);
	                    for (bts = Btris(bb); bts && *bts; ++bts)
		            {
	                      ntt = set_tri_list_around_point(p,(*bts)->tri,
				                              &tt,intfc);
		              for (k = 0; k < ntt; ++k)
		                set_normal_of_tri(tt[k]);
		            }
			  }
			}
		      }
		      if (p == b->end)
		      {
		        if (b->next)
		          set_bond_length(b->next,dim);
		        else
		        {
			  n = Btris(b)[0]->curve->end;
			  for (cc = n->in_curves; cc && *cc; ++cc)
			  {
			    bb = (*cc)->last;
			    set_bond_length(bb,dim);
	                    for (bts = Btris(bb); bts && *bts; ++bts)
		            {
	                      ntt = set_tri_list_around_point(p,(*bts)->tri,
				                              &tt,intfc);
		              for (k = 0; k < ntt; ++k)
		                set_normal_of_tri(tt[k]);
		            }
			  }
			  for (cc = n->out_curves; cc && *cc; ++cc)
			  {
			    bb = (*cc)->first;
			    set_bond_length(bb,dim);
	                    for (bts = Btris(bb); bts && *bts; ++bts)
		            {
	                      ntt = set_tri_list_around_point(p,(*bts)->tri,
				                              &tt,intfc);
		              for (k = 0; k < ntt; ++k)
		                set_normal_of_tri(tt[k]);
		            }
			  }
			}
		      }
		    }
		  }
	        }
	      }
	    }
	  }
	}
	return pmid;
}		/*end i_average_points*/



/*
*		i_delete_point():
*
*       Deletes a point and its entry in its interface.
*       Returns 1 if successful, or 0 if not.
*/
 
 
EXPORT int i_delete_point(
	FT_POINT		*point)
{
 
	INTERFACE *intfc;
	FT_POINT	  **p;
	int       n;
	boolean	  status;

	if (point==NULL)
	{
	    (void) printf("WARNING in i_delete_point(), "
			  "can't delete NULL point\n");
	    return 0;
	}
	intfc = point->interface;
	if (intfc==NULL)
	{
	    (void) printf("WARNING in i_delete_point(), "
			  "point->interface is NULL\n");
	    return 0;
	}
	if (intfc->points==NULL)
	{
	    (void) printf("WARNING in i_delete_point(), "
			  "point->interface->points is NULL\n");
	    return 0;
	}
 
	if (intfc->dim != 1)
	    return 1;

	p = intfc->points;
	for (n = 0; n < intfc->num_points; ++n)
	    if (point == p[n])
	        break;
	if (n == intfc->num_points)
	{
	    (void) printf("WARNING in i_delete_point(), "
			  "point not on interface\n");
	    return 0;
	}
	    
	status = delete_from_ordered_pointers(point,&intfc->points);
	if (Hyper_surf(point) != NULL)
	{
	    if (delete_from_pointers(Hyper_surf(point),&hyper_surf_list(intfc))
		!= FUNCTION_SUCCEEDED)
		status = FUNCTION_FAILED;
	}
	intfc->num_points -= 1;
	intfc->modified = YES;
	if (intfc->dim == 1)
	    Hyper_surf(point)->interface = NULL;
 
	point->interface = NULL;
	return (status == FUNCTION_FAILED) ? 0 : 1;
}		/*end i_delete_point*/

/*
*			i_fprint_point():
*
*       Prints a Point.
*/
 
LIB_LOCAL void i_fprint_point(
	FILE		*file,
	FT_POINT		*point)
{
	int		dim;
 
	(void) fprintf(file,"\tPoint %llu:\n",point_number(point));
	if (point == NULL)
	{
	    (void) fprintf(file,"\t\tNULL Point\n\t\tEnd Point\n");
	    return;
	}
	dim = point->interface->dim;
	(void) fprintf(file,"\t\tPosition ");
	if (is_binary_output() == YES)
	{
	    (void) fprintf(file,"\f%c",dim);
	    (void) fwrite((const void *) Coords(point),FLOAT,dim,file);
	}
	else
	{
	    static const char	*vname[3] = {"x =", "   y =", "   z ="};
	    int		k;

	    for (k = 0; k < dim; ++k)
	    	(void) fprintf(file,"%s %"FFMT,vname[k],Coords(point)[k]);
	}
	(void) fprintf(file,"\n");
	if (dim == 1)
	{
	    (void) fprintf(file,"\tHypersurface of point = %llu\n",
	    	       hypersurface_number(Hyper_surf(point)));
	    (void) fprintf(file,"\t\tLeft Component = %-4d   "
	    		        "Right Component = %-4d    ",
	    	                negative_component(point),
	    	                positive_component(point));
	    (void) fprintf(file,"%s\n",is_bdry(point) ?
	                   "Boundary Point" : "Interior Point");
	}
 
	user_fprint_point(file,point);
	(void) fprintf(file,"\tEnd Point\n\n");
}		/*end i_fprint_point*/



/*
*			i_insert_point_in_bond():
*
*	Inserts point p into a bond b on a curve c.   The bond is changed
*	into two bonds with the point at their common point.
*	Returns 1 if successful, or 0 on error.   Errors occur if p,b or 
*	c is NULL or if space cannot be allocated for a new bond.
*/

EXPORT boolean i_insert_point_in_bond(
	FT_POINT		*p,
	BOND		*b,
	CURVE		*c)
{
	BOND		*bnew;

	if ((p==NULL) || (b==NULL) || (c==NULL) || (bnew=Bond(p,b->end))==NULL) 
	    return FUNCTION_FAILED; 

	bnew->prev = b;
	if (b->next != NULL)
	    b->next->prev = bnew;
	else
	    c->last = bnew;				/* End of Curve */
	bnew->next = b->next;
	b->next = bnew;
	b->end  = p;
	b->length = separation(b->start,p,c->interface->dim);
	++c->num_points;
	++c->interface->num_points;
	if (c->interface->dim == 2)
	{
	    bnew->start->hse = bnew->end->hse = Hyper_surf_element(bnew);
	    bnew->start->hs = bnew->end->hs = Hyper_surf(c);
	}
	else if (c->interface->dim == 3)
	{
	    split_tris_at_split_bond(p,b,c);
	    Boundary_point(p) = 1;
	}
	c->interface->modified = YES;
	return FUNCTION_SUCCEEDED;
}		/*end i_insert_point_in_bond*/


/*
*			split_tris_at_split_bond():
*
*	This routine is intended as a subroutine to insert_point_in_bond.
*	It should not be called independently.  It will split each tri
*	having the split bond as one of its edges.
*/

LOCAL	void split_tris_at_split_bond(
	FT_POINT		*p,
	BOND		*b,
	CURVE		*c)
{
	BOND	  *newb;
	BOND_TRI  **btris;
	TRI	  *t, *nt, *at;
	FT_POINT	  *ps, *pe;
	FT_POINT	  *p0, *p1, *p2;
	POINTER	  n01, n12, n20;
	SURFACE	  *surf;
	INTERFACE *intfc = c->interface;
	int	  i, is, ie;
	int	  bside, aside;
	int	  bdry;
	
	if (intfc->dim != 3)
	    return;

	newb = b->next;
	/*
	*  If the point being inserted already exits on the
	*  bond,  don't split the tri.
	*/
	if ((b->start == b->end) || (newb->start == newb->end))
	    return;

	ps = b->start; pe = newb->end;
	for (btris = Btris(b); btris && *btris; ++btris)
	{
	    t = (*btris)->tri;
	    surf = Surface_of_tri(t);
	    is = Vertex_of_point(t,ps);
	    ie = Vertex_of_point(t,pe);

	    if (is == ERROR || ie == ERROR)
	    {
	    	screen("ERROR in split_tris_at_split_bond(), "
	    	       "inconsistent vertices, is = %d, ie = %d\n",is,ie);
	    	print_bond(b);
	    	print_bond(newb);
	    	print_tri(t,intfc);
	    	clean_up(ERROR);
	    }

	    bdry = 0;
	    if (ie == Next_m3(is))
	    {
	    	bside = is;
	    	aside = ie;
	    	p0 = p;
	    	p1 = pe;
	    	p2 = Point_of_tri(t)[Next_m3(ie)];

	    	n01 = (POINTER) newb;
	    	set_01_bdry(bdry,1);
	    	n12 = Neighbor_on_side(t,aside);
	    	set_12_bdry(bdry,is_side_bdry(t,aside)?1:0);
	    	n20 = (POINTER) t;
	    	set_20_bdry(bdry,0);
	    }
	    else if (ie == Prev_m3(is))
	    {
	    	bside = ie;
	    	aside = Prev_m3(bside);
	    	p0 = pe;
	    	p1 = p;
	    	p2 = Point_of_tri(t)[aside];

	    	n01 = (POINTER) newb;
	    	set_01_bdry(bdry,1);
	    	n12 = (POINTER) t;
	    	set_12_bdry(bdry,0);
	    	n20 = Neighbor_on_side(t,aside);
	    	set_20_bdry(bdry,is_side_bdry(t,aside)?1:0);
	    }
	    else
	    {
	    	screen("ERROR in split_tris_at_split_bond(), "
	    	       "inconsistent vertices, is = %d, ie = %d\n",is,ie);
	    	print_bond(b);
	    	print_bond(newb);
	    	print_tri(t,intfc);
	    	clean_up(ERROR);
	    }

	    nt = make_tri(p0,p1,p2,n01,n12,n20,bdry);
	    /*#bjet2 */
	    /*insert_tri_at_tail_of_list(nt,surf); */

	    Point_of_tri(t)[ie] = p;
	    set_normal_of_tri(t);

	    if (link_tri_to_bond(NULL,nt,surf,newb,c) == NULL)
	    {
	    	screen("ERROR in split_tris_at_split_bond(), "
		       "can't link nt to newb\n");
	    	print_bond(b);
	    	print_bond(newb);
	    	print_tri(t,intfc);
	    	clean_up(ERROR);
	    }

	    insert_tri_at_tail_of_list(nt,surf);
	    
	    if (is_side_bdry(t,aside))
	    {
		BOND_TRI *bt = Bond_tri_on_side(t,aside);
		(void) link_tri_to_bond(bt,nt,surf,bt->bond,bt->curve);
	    	set_side_bdry(Boundary_tri(t),aside,0);
	    }
	    else
	    {
	    	at = Tri_on_side(t,aside);
	    	for (i = 0; i < 3; ++i)
	    	{
	    	    if (Tri_on_side(at,i) == t)
	    	    {
	    	    	Tri_on_side(at,i) = nt;
	    	    	break;
	    	    }
	    	}
	    }
	    Tri_on_side(t,aside) = nt;
	}
}		/*end split_tris_at_split_bond*/



/*
*			i_delete_start_of_bond():
*
*	Deletes the point at the start of bond b on curve c.   
*	Deletes the whole bond b.   It is an error to delete the
*	first POINT on a CURVE (i.e. a NODE) by this routine.
*	Returns 1 on success or 0 on error - b and c must be non-NULL.
*/

EXPORT boolean i_delete_start_of_bond(
	BOND		*b,
	CURVE		*c)
{
	if (b==NULL || c==NULL || b->prev==NULL)
	    return FUNCTION_FAILED;

	if (single_tri_surface_on_bond(b))
	{
	    (void) printf("WARNING in i_delete_start_of_bond(), "
			  "can't delete single tri surface\n");
	    return FUNCTION_FAILED; 
	}

	b->prev->end = b->end;
	b->prev->next = b->next;
	set_bond_length(b->prev,c->interface->dim);

	if (b->next==NULL) /* End of Curve */
	    c->last = b->prev;
	else
	    b->next->prev = b->prev;
	--c->num_points;
	--c->interface->num_points;
	c->interface->modified = YES;
	if ((c->interface->dim == 3) && (Btris(b) != NULL))
	{
	    BOND_TRI **bt;
	    int      i;

	    for (i = 0, bt = Btris(b); bt && *bt; ++i, ++bt)
	    {
	    	if (!reset_tris_at_deleted_bond(*bt,POSITIVE_ORIENTATION,
						   Btris(b->prev)[i]))
		{
	            (void) printf("WARNING in i_delete_start_of_bond(), "
			          "can't reset tris\n");
		    return FUNCTION_FAILED;
		}
	    }
	}
	return FUNCTION_SUCCEEDED;
}		/*end i_delete_start_of_bond*/

/*
*			i_delete_end_of_bond():
*
*	Deletes the point at the end of bond b on curve c.   
*	Deletes the whole bond b.   It is an error to delete the
*	last POINT on a CURVE (i.e. a NODE) by this routine.
*	Returns 1 on success or 0 on error - b and c must be non-NULL.
*/

EXPORT boolean i_delete_end_of_bond(
	BOND		*b,
	CURVE		*c)
{

	if (b==NULL || c==NULL || b->next==NULL)
	    return FUNCTION_FAILED; 

	if (single_tri_surface_on_bond(b))
	{
	    (void) printf("WARNING in i_delete_end_of_bond(), "
			  "can't delete single tri surface\n");
	    return FUNCTION_FAILED; 
	}

	b->next->start = b->start;
	b->next->prev = b->prev;
	set_bond_length(b->next,c->interface->dim);

	if (b->prev==NULL) /* End of Curve */
	    c->first = b->next;
	else
	    b->prev->next = b->next;
	--c->num_points;
	--c->interface->num_points;
	c->interface->modified = YES;
	if ((c->interface->dim == 3) && (Btris(b) != NULL))
	{
	    BOND_TRI **bt;
	    int      i;

	    for (i = 0, bt = Btris(b); bt && *bt; ++i, ++bt)
	    {
	    	if (!reset_tris_at_deleted_bond(*bt,NEGATIVE_ORIENTATION,
						   Btris(b->next)[i]))
		{
	            (void) printf("WARNING in i_delete_end_of_bond(), "
			          "can't reset tris\n");
		    return FUNCTION_FAILED;
		}
	    }
	}
	return FUNCTION_SUCCEEDED;
}		/*end i_delete_end_of_bond*/

EXPORT	CURVE	*curve_of_bond(
	BOND	  *b,
	INTERFACE *intfc)
{
	CURVE **c;
	while (b->prev != NULL) b = b->prev;
	for (c = intfc->curves; c && *c; ++c)
	    if ((*c)->first == b)
		return *c;
	return NULL;
}		/*end curve_of_bond*/

LOCAL	boolean single_tri_surface_on_bond(
	BOND *b)
{
	BOND_TRI **bt = Btris(b);
	static const int flag = BIN_SIDE01|BIN_SIDE12|BIN_SIDE20;
	for (bt = Btris(b); bt && *bt; ++bt)
	    if (Boundary_tri((*bt)->tri) == flag)
		return YES; 
	return NO;
}		/*end single_tri_surface_on_bond*/

LOCAL	boolean	reset_tris_at_deleted_bond(
	BOND_TRI	*btri,
	ORIENTATION	orient,
	BOND_TRI        *btri_adj)
{
	BOND	         *b = btri->bond;
	FT_POINT            *pb = Point_of_bond(b,orient);
	SURFACE          *s = btri->surface;
	TRI              *tri = btri->tri;
	TRI              **tris;
	int              i, nt, nv;
	int              side;
	static BOND_TRI  **nb_bond_tri = NULL;
	static FT_POINT     **v = NULL;
	static TRI       **nb_tri = NULL;
	static TRI       **oldtris = NULL;
	static int       max_n_t = 0;

	nt = set_tri_list_around_point(pb,tri,&tris,s->interface);
	nv = nt+1;

	/* Allocate storage for scratch arrays as necessary */
	if (nt > max_n_t)
	{
	    if (v != NULL)
		free(v);
	    if (nb_bond_tri != NULL)
		free(nb_bond_tri);
	    if (nb_tri != NULL)
		free(nb_tri);
	    max_n_t = 2*nt;
	    uni_array(&v,max_n_t+1,sizeof(FT_POINT*));
	    uni_array(&nb_bond_tri,max_n_t+1,sizeof(BOND_TRI*));
	    uni_array(&nb_tri,max_n_t+1,sizeof(TRI*));
	    uni_array(&oldtris,max_n_t+1,sizeof(TRI*));
	}

	/* Indentify adjacent vertices and tri neighbors */
	for (i = 0; i < nt; ++i)
	{
	    oldtris[i] = tris[i];
	    side = Next_m3(Vertex_of_point(oldtris[i],pb));
	    if (is_side_bdry(oldtris[i],side))
	    {
		nb_bond_tri[i] = Bond_tri_on_side(oldtris[i],side);
		nb_tri[i] = NULL;
	    }
	    else
	    {
		nb_bond_tri[i] = NULL;
	        nb_tri[i] = Tri_on_side(oldtris[i],side);
	    }
	    v[i] = Point_of_tri(oldtris[i])[side];
	}
	nb_bond_tri[nt] = btri_adj;
	nb_tri[nt] = NULL;
	v[nt] = Point_of_tri(oldtris[nt-1])[Next_m3(side)];
	switch (orient)
	{
	case POSITIVE_ORIENTATION:
	    oldtris[nt] = tris[nt-1];
	    break;
	case NEGATIVE_ORIENTATION:
	    oldtris[nt] = tris[0];
	    break;
	case ORIENTATION_NOT_SET:
	default:
	    screen("ERROR in reset_tris_at_deleted_bond(), invalid orient %s\n",
		   orientation_name(orient));
	    return NO;
	}
	return retriangulate_polygon(v,nv,NULL,0,Tri_normal(tri),oldtris,nt,s,
				     nb_bond_tri,nb_tri,NULL,NULL);
}		/*end reset_tris_at_deleted_bond*/


/*
*			retriangulate_polygon():
*
*	Retriangulates a polygon bounded by the specified vertices.
*
*	Input:
*	          v - array of POINT's bounding the polygon.  The array
*		      is assumed to be cyclic about the polygon boundary
*		      so that the ith edge of the polygon runs from vertex
* 		      v[i] to vertex v[(i+1)%nv].
*
*	         nv - length of the array v
*
*	       tnor - average normal vector to the polygon,  the points in
*		      v will be projected onto a plane with normal tnor up
*		      which the constrained Delauney triangulate will be
*		      constructed.  The vector tnor need not be normalized
*		      to a unit uni_array.
*
*	    oldtris - List of triangles forming the "old" polygon being
*		      retriangulated.
*
*                nt - length of the array nt.
*
*		  s - Surface containing the polygon being retriangulated.
*
*	nb_bond_tri - Neighoring BOND_TRI's.  If the ith edge of the 
*		      polygon is a bond (ie the edge from v[i] to v[(i+1)%nv]
*		      is a BOND,  then the BOND_TRI defining the link from
*		      bond to tri is contained in nb_bond_tri[i].  Otherwise
*		      this field is NULL.  See nb_tri below.
*
*	     nb_tri - Neighoring TRI's.  If the ith edge of the
*		      polygon is a tri (ie the edge from v[i] to v[(i+1)%nv]
*                     connects two TRI's,  then the TRI neighbor of oldtris[i]
*                     on this side is contained in nb_tri[i].  Otherwise
*                     this field is NULL.  See nb_bond_tri above.
*
*	Output:
*	    pnewtris - If non-null a pointer to the array of new triangles
*		       will be returned via this pointer.
*
*	pnum_newtris - If non-null *pnum_newtris is the size of the array
*		       newtris.
*
*	Returns YES upon sucessful completion,  NO otherwise.  If successful
*	the triangles from the oldtris array are detached from the surface s.
*
*	WARNING:  The array newtris is an internal static array and will only
*	          be valid until the next call to retriangulate_polygon()
*	          upon which this array is reset.
*/

EXPORT	boolean	retriangulate_polygon(
	FT_POINT       **v,
	int         nv,
	FT_POINT       **internal_v,
	int         ninternal_v,
	const double *tnor,
	TRI         **oldtris,
	int         nt,
	SURFACE     *s,
	BOND_TRI    **nb_bond_tri,
	TRI         **nb_tri,
	TRI         ***pnewtris,
	int         *pnum_newtris)
{
	INTERFACE            *intfc = s->interface;
	boolean                 reversed, reversed_set;
	double                BBL[3], BBU[3];
	double                dp[3], pbar[3], lambda[3];
	double                ntnor[3];
	int                  i, j, k, l, ii, indx;
	int                  sd, v1, v2;
	int                  i0, i1, i2;
	int                  k1, k2;
	static FT_POINT         **pts = NULL;
	static boolean          first = YES;
	static const double   **p = NULL;
	static double         **r = NULL;
	static int           max_n_v = 0;
	static triangulateio in;
	static triangulateio out;
	static TRI           **newtris = NULL;
	static size_t        max_n_newtris = 0;
	static int           num_newtris = 0;

	debug_print("retriang","Entered retriangulate_polygon()\n");
	if (first == YES)
	{
	    first = NO;
	    in.Opts.poly = YES;
	    in.Opts.neighbors = YES;
	    in.Opts.edgesout = YES;
	    in.Opts.steiner = -1;
	    in.Opts.order = 1;
	    in.Opts.noholes = YES;
	}
	if (pnewtris)
	    *pnewtris = NULL;
	if (pnum_newtris)
	    *pnum_newtris = 0;

	if (debugging("retriang") && oldtris != NULL)
	{
	    double mag_tnor;

	    if (!consistent_interface(s->interface))
	    {
		screen("ERROR in retriangulate_polygon(), "
		       "input interface is inconsistent\n");
		clean_up(ERROR);
	    }
	    set_tri_list_bounding_box(oldtris,nt,BBL,BBU,NO,YES);
	    set_tri_list_bounding_box(nb_tri,nv,BBL,BBU,YES,YES);
	    set_point_list_bounding_box(internal_v,ninternal_v,BBL,BBU,YES,YES);
	    gview_plot_axes("","retriangulate_polygon-axes",BBL,BBU,BBL,BBU);
	    gview_plot_triangle_list("","retriangulate_polygon-oldtris",
				     oldtris,nt,0.1,0.0,0.0,0.9,0.0,0.0,
				     0.5,BBL,BBU);
	    gview_plot_triangle_list("","retriangulate_polygon-neighbors",
				     nb_tri,nv,0.0,0.1,0.0,0.0,0.9,0.0,
				     0.5,BBL,BBU);
	    gview_plot_polyline("","retriangulate_polygon-boundary",v,nv,
				YES,0.0,1.0,1.0,1.0,BBL,BBU);
	    gview_plot_vertices("","retriangulate_polygon-internal_vertices",
				internal_v,ninternal_v,BBL,BBU);

	    print_general_vector("tnor = ",tnor,3,", ");
	    mag_tnor = mag_vector(tnor,3);
	    (void) printf("magnitude = %g\n",mag_tnor);
	    for (i = 0; i < nt; ++i)
	    {
	        const double *otnor = Tri_normal(oldtris[i]);
		double mag_otnor = Mag3d(otnor);
		(void) printf("Tri_normal(oldtris[%d]) = ",i);
		print_general_vector("",otnor,3,", ");
		(void) printf("<normal,tnor/!tnor|> = %g\n",
			      scalar_product(otnor,tnor,3)/
			      (mag_tnor*mag_otnor));
	    }
	    for (i = 0; i < nv; ++i)
	    {
		if (nb_tri[i] != NULL)
		{
		    const double *nb_tnor = Tri_normal(nb_tri[i]);
		    double mag_nb_tnor = Mag3d(nb_tnor);
		    (void) printf("Tri_normal(nb_tri[%d]) = ",i);
		    print_general_vector("",nb_tnor,3,", ");
		    (void) printf("<normal,tnor/!tnor|> = %g\n",
			          scalar_product(nb_tnor,tnor,3)/
			          (mag_tnor*mag_nb_tnor));
		}
	    }
	    (void) printf("%d vertices\n",nv);
	    for (i = 0; i < nv; ++i)
	    {
		(void) printf("vertex[%d] %llu = %g %g %g\n",i,
			      point_number(v[i]),
			      Coords(v[i])[0],
			      Coords(v[i])[1],
			      Coords(v[i])[2]);
	    }
	    (void) printf("%d old triangles to be removed\n",nt);
	    for (i = 0; i < nt; ++i)
	    {
		(void) printf("oldtris[%d] - ",i);
		print_tri(oldtris[i],intfc);
	    }
	    (void) printf("%d neighbors\n",nv);
	    for (i = 0; i < nv; ++i)
	    {
		(void) printf("nb_tri[%d] - ",i);
		print_tri(nb_tri[i],intfc);
	    }
	}

	if (r == NULL)
	    bi_array(&r,3,3,FLOAT);
	if ((nv+ninternal_v) > max_n_v)
	{
	    if (pts != NULL)
		free(pts);
	    if (p != NULL)
		free(p);
	    if (in.pointlist != NULL)
		free(in.pointlist);
	    if (in.segmentlist != NULL)
		free(in.segmentlist);
	    max_n_v = 2*(nv + ninternal_v);
	    uni_array(&p,max_n_v,sizeof(double*));
	    uni_array(&pts,max_n_v,sizeof(FT_POINT*));
	    in.size_pointlist = (size_t)2*(max_n_v);
	    uni_array(&in.pointlist,in.size_pointlist,FLOAT);
	    in.size_segmentlist = (size_t)2*(max_n_v);
	    uni_array(&in.segmentlist,in.size_segmentlist,INT);
	}

	for (i = 0; i < nv; ++i)
	{
	    pts[i] = v[i];
	    p[i] = Coords(pts[i]);
	}
	for (i = 0; i < ninternal_v; ++i)
	{
	    pts[i+nv] = internal_v[i];
	    p[i+nv] = Coords(pts[i+nv]);
	}

	for (ii = 0; ii < 2; ++ii)
	{
	  /* Set up polygon bounding the region to be retriangulated */
	  affine_fit(p,3,nv+ninternal_v,tnor,pbar,r,lambda);

	  /*
	   * Project the set of points onto the plane containing pbar with
	   * normal vector r[2]. Note that since r is orthogonal,  this plane
	   * is spanned by the uni_arrays r[0], and r[1]
	   */
	  for (i = 0; i < nv; ++i)
	  {
	      for (j = 0; j < 3; ++j)
		  dp[j] = p[i][j] - pbar[j];
	      in.pointlist[2*i] = scalar_product(r[0],dp,3);
	      in.pointlist[2*i+1] = scalar_product(r[1],dp,3);
	      in.segmentlist[2*i] = i;
	      in.segmentlist[2*i+1] = (i+1)%nv;
	  }
	  if (debugging("retriang"))
	  {
	    (void) printf("Projection bi_array\n");
	    (void) printf("lambda = %"FFMT" %"FFMT" %"FFMT"\n",
			  lambda[0],lambda[1],lambda[2]);
	    (void) printf("r[0] = %"FFMT" %"FFMT" %"FFMT", mag = %"FFMT"\n",
			  r[0][0],r[0][1],r[0][2],Mag3d(r[0]));
	    (void) printf("r[1] = %"FFMT" %"FFMT" %"FFMT", mag = %"FFMT"\n",
			  r[1][0],r[1][1],r[1][2],Mag3d(r[1]));
	    (void) printf("r[2] = %"FFMT" %"FFMT" %"FFMT", mag = %"FFMT"\n",
			  r[2][0],r[2][1],r[2][2],Mag3d(r[2]));
	    (void) printf("<r[0],r[1]> = %"FFMT"\n",Dot3d(r[0],r[1]));
	    (void) printf("<r[0],r[2]> = %"FFMT"\n",Dot3d(r[0],r[2]));
	    (void) printf("<r[1],r[2]> = %"FFMT"\n",Dot3d(r[1],r[2]));

	    (void) printf("Projected point list, nv = %d\n",nv);
	    output();
	    (void) printf("X Y RETRIANGULATE_PROJECTED_POINTS\n");
	    for (i = 0; i < nv; ++i)
		(void) printf("%"FFMT" %"FFMT"\n",
			      in.pointlist[2*i],in.pointlist[2*i+1]);
	    (void) printf("%"FFMT" %"FFMT"\n",
			  in.pointlist[0],in.pointlist[1]);
	    (void) printf("\n\n");
	  }
	  if (is_tangled_polygon(in.pointlist,nv))
	  {
	    (void) printf("WARNING in retriangulate_polygon(), "
		          "projected polygon is tangled\n");
	    debug_print("retriang","Left retriangulate_polygon()\n");
	    return NO;
	  }
	  for (i = 0; i < ninternal_v; ++i)
	  {
	    for (j = 0; j < 3; ++j)
	      dp[j] = p[i+nv][j] - pbar[j];
	    in.pointlist[2*(i+nv)] = scalar_product(r[0],dp,3);
	    in.pointlist[2*(i+nv)+1] = scalar_product(r[1],dp,3);
	    if (!(winding_number(in.pointlist,in.pointlist+2*(i+nv),nv)%2))
	    {
	      (void) printf("WARNING in retriangulate_polygon(), "
		            "internal vertex not in polygon\n");
	      debug_print("retriang","Left retriangulate_polygon()\n");
	      return NO;
	    }
	  }

	  /* Call triangulation function */
	  in.numberofpoints = nv+ninternal_v;
	  in.numberofsegments = nv;
	  triangulate(&in,&out,NULL);
	  num_newtris = out.numberoftriangles;
	  if (out.numberofpoints != in.numberofpoints)
	  {
	    (void) printf("WARNING in retriangulate_polygon(), "
		          "plane projection is tangled\n");
	    debug_print("retriang","Left retriangulate_polygon()\n");
	    return NO;
	  }

	  /* determine if neighbors will be consistent */

	  for (i = 0; i < nv; ++i)
	  {
	    if ((nb_tri[i] != NULL) && (nb_tri[i] == nb_tri[(i+nv-1)%nv]))
	    {
	      double dpm[3], pm[2];
	      double *vp, *vn;

	      vp = Coords(v[(i+nv-1)%nv]);
	      vn = Coords(v[(i+1)%nv]);
	      for (k = 0; k < nv; ++k)
	        p[k] = Coords(v[k]);

	      affine_fit(p,3,nv,tnor,pbar,r,lambda);
	      for (l = 0; l < 3; ++l)
		dpm[l] = 0.5*(vp[l]+vn[l]) - pbar[l];
	      pm[0] = scalar_product(r[0],dpm,3);
	      pm[1] = scalar_product(r[1],dpm,3);
	      for (k = 0; k < nv; ++k)
	      {
	        for (l = 0; l < 3; ++l)
		  dp[l] = p[k][l] - pbar[l];
	        in.pointlist[2*k] = scalar_product(r[0],dp,3);
	        in.pointlist[2*k+1] = scalar_product(r[1],dp,3);
	      }
	      if (winding_number(in.pointlist,pm,nv)%2)
	      {
	        (void) printf("WARNING in retriangulate_polygon(), "
		              "fold in polygon\n");
	        debug_print("retriang","Left retriangulate_polygon()\n");
	        return NO;
	      }
	    }
	  }

	  reversed = NO;
	  reversed_set = NO;
	  for (i = 0; i < num_newtris; ++i)
	  {
	    for (j = 0; j < 3; ++j)
	    {
	      indx = TriangulateNeighborOnSide(i,j,out);
	      if (indx < 0)
	      {
		v1 = out.trianglelist[3*i+j];
		v2 = out.trianglelist[3*i+Next_m3(j)];
		if (v2 == (v1+1)%nv)
		    sd = v1;
		else if (v1 == (v2+1)%nv)
		    sd = v2;
		else
		{
		  screen("ERROR in retriangulate_polygon(), "
			 "vertex index inconsistent during reversal test\n");
		  (void) printf("v1 = %d, v2 = %d, nv = %d\n",v1,v2,nv);
		  (void) printf("newtris[%d], side %d - ",i,j);
		  print_tri(newtris[i],intfc);
		  clean_up(ERROR);
	          debug_print("retriang","Left retriangulate_polygon()\n");
		  return NO;
		}
		if (nb_bond_tri && nb_bond_tri[sd])
		{
		  BOND *b = nb_bond_tri[sd]->bond;
		  FT_POINT *ps = b->start, *pe = b->end;
		  if ((ps == pts[v1]) && (pe == pts[v2]))
		  {
		    if (reversed)
		    {
		      (void) printf("WARNING in retriangulate_polyon "
				    "inconsistent orientations\n");
	              debug_print("retriang","Left retriangulate_polygon()\n");
		      return NO;
		    }
		  }
		  else if ((ps == pts[v2]) && (pe == pts[v1]))
		  {
		    if (reversed_set && !reversed)
		    {
		      (void) printf("WARNING in retriangulate_polyon "
				    "inconsistent orientations\n");
	              debug_print("retriang","Left retriangulate_polygon()\n");
		      return NO;
		    }
		    reversed = YES;
		    reversed_set = YES;
		  }
		  else
		  {
		    (void) printf("WARNING in retriangulate_polyon "
				  "inconsistent points at bond\n");
	            debug_print("retriang","Left retriangulate_polygon()\n");
		    return NO;
		  }
		}
		else
		{
		  TRI   *nbtri = nb_tri[sd];

	          if (((k1 = Vertex_of_point(nbtri,pts[v1])) == ERROR) ||
			((k2 = Vertex_of_point(nbtri,pts[v2])) == ERROR))
		  {
		    screen("ERROR in retriangulate_polygon(), "
			   "can't find vertex index\n");
		    (void) printf("pts[%d] %llu %g %g %g\n",v1,
				  point_number(pts[v1]),
				  Coords(pts[v1])[0],
				  Coords(pts[v1])[1],
				  Coords(pts[v1])[2]);
		    (void) printf("pts[%d] %llu %g %g %g\n",v2,
				  point_number(pts[v2]),
				  Coords(pts[v2])[0],
				  Coords(pts[v2])[1],
				  Coords(pts[v2])[2]);
		    (void) printf("nb_tri[%d] - ",sd);
	            debug_print("retriang","Left retriangulate_polygon()\n");
		    return NO;
		  }
		  if (k1 == Next_m3(k2))
		  {
		    if (reversed)
		    {
		      (void) printf("WARNING in retriangulate_polyon "
				    "inconsistent orientations\n");
	              debug_print("retriang","Left retriangulate_polygon()\n");
		      return NO;
		    }
		  }
		  else
		  {
		    if (reversed_set && !reversed)
		    {
		      (void) printf("WARNING in retriangulate_polyon "
				    "inconsistent orientations\n");
	              debug_print("retriang","Left retriangulate_polygon()\n");
		      return NO;
		    }
		    reversed = YES;
		    reversed_set = YES;
		  }
		}
	      }
	    }
	  }
	  if (ii == 0)
	  {
	    if (!reversed) /*Configuration is okay*/
	      break;
	    else
	    {
	      ntnor[0] = -tnor[0];
	      ntnor[1] = -tnor[1];
	      ntnor[2] = -tnor[2];
	      tnor = ntnor;
	    }
	  }
	  else if (ii == 1)
	  {
	    if (!reversed) /*Configuration is okay*/
	      break;
	    else
	    {
	      (void) printf("WARNING in retriangulate_polygon(), "
		            "did not create valid retriangulation\n");
	      debug_print("retriang","Left retriangulate_polygon()\n");
	      return NO;
	    }
	  }
	  else
	  {
	    (void) printf("WARNING in retriangulate_polygon(), "
		          "did not create valid retriangulation\n");
	    debug_print("retriang","Left retriangulate_polygon()\n");
	    return NO;
	  }
	}
	if (ii == 2)
	{
	  (void) printf("WARNING in retriangulate_polygon(), "
		        "did not create valid retriangulation\n");
	  debug_print("retriang","Left retriangulate_polygon()\n");
	  return NO;
	}

	/* Create new triangles */
	if (num_newtris > max_n_newtris)
	{
	    if (newtris != NULL)
		free(newtris);
	    max_n_newtris = 2*num_newtris+1;
	    uni_array(&newtris,max_n_newtris,sizeof(TRI*));
	}

	for (i = 0; i < num_newtris; ++i)
	{
	    i0 = out.trianglelist[3*i];
	    i1 = out.trianglelist[3*i+1];
	    i2 = out.trianglelist[3*i+2];
	    newtris[i] = make_tri(pts[i0],pts[i1],pts[i2],NULL,NULL,NULL,0);
	    insert_tri_at_tail_of_list(newtris[i],s);
	}
	newtris[i] = NULL;

	if (debugging("retriang"))
	{
	    gview_plot_triangle_list("","retriangulate_polygon-newtris",
				     newtris,num_newtris,
				     0.0,0.0,0.1,0.0,0.0,0.9,
				     0.5,BBL,BBU);

	    (void) printf("%d new triangles\n",num_newtris);
	    for (i = 0; i < num_newtris; ++i)
	    {
		(void) printf("newtris[%d] - ",i);
		print_tri(newtris[i],intfc);
	    }
	}

	/* Reset neighbors */
	for (i = 0; i < num_newtris; ++i)
	{
	    if (debugging("retriang"))
	    {
		(void) printf("Setting neighbors of newtri[%d] %llu\n",
			      i,tri_number(newtris[i],s->interface));
	    }
	    for (j = 0; j < 3; ++j)
	    {
		indx = TriangulateNeighborOnSide(i,j,out);
	        if (debugging("retriang"))
		    (void) printf("\tSide %d has neighbor indx %d\n",j,indx);

		if (indx >= 0)
		    Tri_on_side(newtris[i],j) = newtris[indx];
		else
		{
		    v1 = out.trianglelist[3*i+j];
		    v2 = out.trianglelist[3*i+Next_m3(j)];
		    if (v2 == (v1+1)%nv)
			sd = v1;
		    else if (v1 == (v2+1)%nv)
			sd = v2;
		    else
		    {
			screen("ERROR in retriangulate_polygon(), "
			       "vertex index inconsistent\n");
			(void) printf("v1 = %d, v2 = %d, nv = %d\n",v1,v2,nv);
			(void) printf("newtris[%d], side %d - ",i,j);
			print_tri(newtris[i],intfc);
			clean_up(ERROR);
	                debug_print("retriang","Left retriangulate_polygon()\n");
			return NO;
		    }
	            if (debugging("retriang"))
		    {
			(void) printf("\tSide %d connects to "
				      "polygon side %d\n",j,sd);
		    }
		    if (nb_bond_tri && nb_bond_tri[sd])
		    {
	                if (debugging("retriang"))
			{
			    (void) printf("\tLinking side %d to "
					  "nb_bond_tri[%d]\n",j,sd);
			}
			(void) link_tri_to_bond(nb_bond_tri[sd],
						newtris[i],s,
						nb_bond_tri[sd]->bond,
						nb_bond_tri[sd]->curve);
		    }
		    else
		    {
			TRI   *nbtri = nb_tri[sd];

		        Tri_on_side(newtris[i],j) = nbtri;
			if (((k1 = Vertex_of_point(nbtri,pts[v1])) == ERROR) ||
			    ((k2 = Vertex_of_point(nbtri,pts[v2])) == ERROR))
			{
			    screen("ERROR in retriangulate_polygon(), "
				   "can't find vertex index\n");
			    (void) printf("pts[%d] %llu %g %g %g\n",v1,
					  point_number(pts[v1]),
					  Coords(pts[v1])[0],
					  Coords(pts[v1])[1],
					  Coords(pts[v1])[2]);
			    (void) printf("pts[%d] %llu %g %g %g\n",v2,
					  point_number(pts[v2]),
					  Coords(pts[v2])[0],
					  Coords(pts[v2])[1],
					  Coords(pts[v2])[2]);
			    (void) printf("newtris[%d], side %d - ",i,j);
			    print_tri(newtris[i],intfc);
			    (void) printf("nb_tri[%d] - ",sd);
			    print_tri(nbtri,intfc);
			    clean_up(ERROR);
	                    debug_print("retriang","Left retriangulate_polygon()\n");
			    return NO;
			}
			k = (k2 == Next_m3(k1)) ? k1 : k2;
		    	Tri_on_side(nbtri,k) = newtris[i];
	                if (debugging("retriang"))
			{
			    (void) printf("\tLinked side %d to "
					  "nb_tri[%d] %llu\n",j,sd,
					  tri_number(nbtri,s->interface));
			    (void) printf("\tpts[%d] %llu %g %g %g\n",v1,
					  point_number(pts[v1]),
					  Coords(pts[v1])[0],
					  Coords(pts[v1])[1],
					  Coords(pts[v1])[2]);
			    (void) printf("\tpts[%d] %llu %g %g %g\n",v2,
					  point_number(pts[v2]),
					  Coords(pts[v2])[0],
					  Coords(pts[v2])[1],
					  Coords(pts[v2])[2]);
			    (void) printf("\tNeighbor indices k1 = %d, "
					  "k2 = %d\n",k1,k2);
			    (void) printf("\tLinked side %d of nb_tri[%d] %llu "
					  "to newtris[%d] %llu\n",k,sd,
					  tri_number(nbtri,s->interface),i,
					  tri_number(newtris[i],s->interface));
			}
		    }
		}
	    }
	}

	/* Delete old triangles */
	if (oldtris != NULL)
	{
	    for (i = 0; i < nt; ++i)
		remove_tri_from_surface(oldtris[i],s,YES);
	}

	if (pnewtris)
	    *pnewtris = newtris;
	if (pnum_newtris)
	    *pnum_newtris = num_newtris;

	if (debugging("retriang"))
	{
	    (void) printf("%d new triangles with neighbors\n",num_newtris);
	    for (i = 0; i < num_newtris; ++i)
	    {
		(void) printf("newtris[%d] - ",i);
		print_tri(newtris[i],intfc);
	    }
	    if (!consistent_interface(s->interface))
	    {
		screen("ERROR in retriangulate_polygon(), "
		       "output interface is inconsistent\n");
		clean_up(ERROR);
	    }
	}
	debug_print("retriang","Left retriangulate_polygon()\n");
	return YES;
}		/*end retriangulate_polygon*/

LOCAL	boolean is_tangled_polygon(
	const REAL *pointlist,
	int        nv)
{
	int i, j;
	double x0s, y0s, x0e, y0e, dx0, dy0;
	double x1s, y1s, x1e, y1e, dx1, dy1;
	double dx10s, dy10s;
	double den;
	double eps = 10.0*MACH_EPS;/*TOLERANCE*/
	double t0, t1;

	for (i = 0; i < nv; ++i)
	{
	  x0s = pointlist[2*i];
	  y0s = pointlist[2*i+1];
	  x0e = pointlist[2*((i+1)%nv)];
	  y0e = pointlist[2*((i+1)%nv)+1];
	  dx0 = x0e - x0s;
	  dy0 = y0e - y0s;
	  for (j = i+2; j < nv; ++j)
	  {
	    if (i != (j+1)%nv)
	    {
	      x1s = pointlist[2*j];
	      y1s = pointlist[2*j+1];
	      x1e = pointlist[2*((j+1)%nv)];
	      y1e = pointlist[2*((j+1)%nv)+1];
	      dx1 = x1e - x1s;
	      dy1 = y1e - y1s;
	      dx10s = x1s - x0s;
	      dy10s = y1s - y0s;
  
	      den = dx1*dy0 - dx0*dy1;
	      if (den < eps)
		return NO;
	      t0 = (dx1*dy10s - dy1*dx10s)/den;
	      t1 = (dx0*dy10s - dy0*dx10s)/den;
	      if (((0.0 <= t0) && (t0 <= 1.0)) && ((0.0 <= t1) && (t1 <= 1.0)))
		return YES;
	    }
	  }
	}
	return NO;
}		/*end is_tangled_polygon*/

LIB_LOCAL int	winding_number(
	double *pts,
	double *pm,
	int   nv)
{
	double theta;
	int   k, kn;

	theta = 0.0;
	for (k = 0; k < nv; ++k)
	{
	    kn = (k+1)%nv;
	    theta += atan2((pts[2*k]   - pm[0])*(pts[2*kn+1] - pm[1]) -
	                   (pts[2*k+1] - pm[1])*(pts[2*kn]   - pm[0]),
	                   (pts[2*k]   - pm[0])*(pts[2*kn]   - pm[0]) +
		           (pts[2*k+1] - pm[1])*(pts[2*kn+1] - pm[1]));
	}
	return irint(theta/(2.0*PI));
}		/*end winding_number*/

EXPORT	boolean	delete_vertex_of_tri(
	FT_POINT   *pt,
	TRI	*tri,
	SURFACE *s)
{
    	INTERFACE       *intfc = s->interface;
	TRI             **otris;
	double           nor[3];
	int             i, k, nt;
	int             side;
	static BOND_TRI **nb_bond_tri = NULL;
	static FT_POINT    **v = NULL;
	static TRI      **nb_tri = NULL;
	static TRI      **oldtris;
	static int      max_n_t = 0;

	if (Boundary_point(pt))
	{
	    BOND_TRI  *bt;
	    BOND      *b;
	    i = Vertex_of_point(tri,pt);
	    k = (is_side_bdry(tri,i)) ? i : Prev_m3(i);
	    if (!is_side_bdry(tri,k))
	    {
	        nt = set_tri_list_around_point(pt,tri,&otris,intfc);
		tri = otris[0];
	        i = Vertex_of_point(tri,pt);
	        k = (is_side_bdry(tri,i)) ? i : Prev_m3(i);
	        if (!is_side_bdry(tri,k))
		{
		    (void) printf("WARNING in delete_vertex_of_tri(), "
		                  "Boundary_point(pt) and no bond "
				  "on tri side\n");
		    return FUNCTION_FAILED;
		}
	    }
	    bt = Bond_tri_on_side(tri,k);
	    b = bt->bond;
	    if (pt == b->start)
		return delete_start_of_bond(b,bt->curve);
	    else if (pt == b->end)
		return delete_end_of_bond(b,bt->curve);
	    else
	    {
		(void) printf("WARNING in delete_vertex_of_tri(), "
		              "Boundary_point(pt) and point not on bond\n");
		return FUNCTION_FAILED;
	    }
	}

	nt = set_tri_list_around_point(pt,tri,&otris,intfc);
	if ((nt < 3) ||
	    (Next_tri_at_vertex(otris[0],pt) != otris[nt-1]) ||
	    (Prev_tri_at_vertex(otris[nt-1],pt) != otris[0]))
	{
	    screen("ERROR in delete_vertex_of_tri(), "
		   "inconsistent configuration\n");
	    clean_up(ERROR);
	}

	/* Allocate storage for scratch arrays as necessary */
	if (nt > max_n_t)
	{
	    if (v != NULL)
		free(v);
	    if (nb_bond_tri != NULL)
		free(nb_bond_tri);
	    if (nb_tri != NULL)
		free(nb_tri);
	    if (oldtris != NULL)
		free(oldtris);
	    max_n_t = 2*nt;
	    uni_array(&v,max_n_t,sizeof(FT_POINT*));
	    uni_array(&nb_bond_tri,max_n_t,sizeof(BOND_TRI*));
	    uni_array(&nb_tri,max_n_t,sizeof(TRI*));
	    uni_array(&oldtris,max_n_t+1,sizeof(TRI*));
	}

	/* Indentify adjacent vertices and tri neighbors */
	for (i = 0; i < nt; ++i)
	{
	    oldtris[i] = otris[i];
	    side = Next_m3(Vertex_of_point(oldtris[i],pt));
	    if (is_side_bdry(oldtris[i],side))
	    {
		nb_bond_tri[i] = Bond_tri_on_side(oldtris[i],side);
		nb_tri[i] = NULL;
	    }
	    else
	    {
		nb_bond_tri[i] = NULL;
	        nb_tri[i] = Tri_on_side(oldtris[i],side);
	    }
	    v[i] = Point_of_tri(oldtris[i])[side];
	}
	oldtris[nt] = NULL;

	omit_vertex_in_plane_fit();
	plane_fit_normal3d(pt,Hyper_surf_element(tri),Hyper_surf(s),nor);
	while (!retriangulate_polygon(v,nt,NULL,0,nor,oldtris,nt,
				     s,nb_bond_tri,nb_tri,NULL,NULL))
	{
	    int imax;
	    double d, dmax;

	    /* Find point most outside of the plane*/
	    imax = 0;
	    dmax = fabs((Coords(v[0])[0] - Coords(pt)[0])*nor[0] +
		        (Coords(v[0])[1] - Coords(pt)[1])*nor[1] +
		        (Coords(v[0])[2] - Coords(pt)[2])*nor[2]);
	    for (i = 1; i < nt; ++i)
	    {
		d = fabs((Coords(v[i])[0] - Coords(pt)[0])*nor[0] +
		         (Coords(v[i])[1] - Coords(pt)[1])*nor[1] +
		         (Coords(v[i])[2] - Coords(pt)[2])*nor[2]);
		if (d > dmax)
		{
		    imax = i;
		    dmax = d;
		}
	    }
	    side = Next_m3(Vertex_of_point(oldtris[imax],pt));
	    flip_diagonal(oldtris[imax],side);
	    nt = set_tri_list_around_point(pt,tri,&otris,intfc);
	    /* Indentify adjacent vertices and tri neighbors */
	    for (i = 0; i < nt; ++i)
	    {
		oldtris[i] = otris[i];
	        side = Next_m3(Vertex_of_point(oldtris[i],pt));
	        if (is_side_bdry(oldtris[i],side))
	        {
		    nb_bond_tri[i] = Bond_tri_on_side(oldtris[i],side);
		    nb_tri[i] = NULL;
	        }
	        else
	        {
		    nb_bond_tri[i] = NULL;
	            nb_tri[i] = Tri_on_side(oldtris[i],side);
	        }
	        v[i] = Point_of_tri(oldtris[i])[side];
	    }
	    oldtris[nt] = NULL;

	    omit_vertex_in_plane_fit();
	    plane_fit_normal3d(pt,Hyper_surf_element(tri),Hyper_surf(s),nor);
	}
	return YES;
}		/*end delete_vertex_of_tri*/

EXPORT boolean flip_diagonal(
	 TRI		*tri,
	 int		side)
{
	BOND_TRI     *bt;
	TRI          *otri;
	TRI_NEIGHBOR Nbr[3], ONbr[3];
	FT_POINT        *pt1, *pt2;
	int          i, nside, pside;
	int          oside, noside, poside;
	boolean         bdry[3], obdry[3];

	if (is_side_bdry(tri,side))
	    return NO;

	otri = Tri_on_side(tri,side);
	nside = Next_m3(side);
	pside = Prev_m3(side);
	pt1 = Point_of_tri(tri)[pside];

	for (oside = 0; oside < 3; ++oside)
	    if (Tri_on_side(otri,oside) == tri)
		break;

	noside = Next_m3(oside);
	poside = Prev_m3(oside);
	pt2 = Point_of_tri(otri)[poside];

	for (i = 0; i < 3; ++i)
	{
	    bdry[i] = is_side_bdry(tri,i) ? YES : NO;
	    Nbr[i] = Tri_neighbor(tri)[i];
	    obdry[i] = is_side_bdry(otri,i) ? YES : NO;
	    ONbr[i] = Tri_neighbor(otri)[i];
	}

	Point_of_tri(tri)[nside] = pt2;
	Tri_on_side(tri,nside) = otri;
	set_side_bdry(Boundary_tri(tri),nside,0);
	if (obdry[noside])
	{
	    bt = ONbr[noside].btri;
	    (void) link_tri_to_bond(bt,tri,bt->surface,bt->bond,bt->curve);
	}
	else
	{
	    set_side_bdry(Boundary_tri(tri),side,0);
	    Tri_on_side(tri,side) = ONbr[noside].tri;
	    for (i = 0; i < 3; ++i)
	    {
		if (Tri_on_side(ONbr[noside].tri,i) == otri)
		{
		    Tri_on_side(ONbr[noside].tri,i) = tri;
		    break;
		}
	    }
	}

	Point_of_tri(otri)[noside] = pt1;
	Tri_on_side(otri,noside) = tri;
	set_side_bdry(Boundary_tri(otri),noside,0);
	if (bdry[nside])
	{
	    bt = Nbr[nside].btri;
	    (void) link_tri_to_bond(bt,otri,bt->surface,bt->bond,bt->curve);
	}
	else
	{
	    set_side_bdry(Boundary_tri(otri),oside,0);
	    Tri_on_side(otri,oside) = Nbr[nside].tri;
	    for (i = 0; i < 3; ++i)
	    {
		if (Tri_on_side(Nbr[nside].tri,i) == tri)
		{
		    Tri_on_side(Nbr[nside].tri,i) = otri;
		    break;
		}
	    }
	}
	set_normal_of_tri(tri);
	set_normal_of_tri(otri);
	return YES;
}		/*end flip_diagonal*/

EXPORT	boolean delete_side_of_tri(
	TRI     *tri,
	SURFACE *s,
	int     side)
{
	BOND_TRI  *bt;
	BOND      *b;
	CURVE     *c;
	INTERFACE *intfc = s->interface;
	FT_POINT     *p[3];
	TRI       *nbtri;
	TRI       **tris;
	int       nt, i, j, nbside;

	if (debugging("consistency"))
	{
	  if (!consistent_interface(intfc))
	  {
	    screen("ERROR in delete_side_of_tri(), input interface is "
		   "inconsistent\n");
	    clean_up(ERROR);
	  }
	}
	if (is_side_bdry(tri,Prev_m3(side)) && is_side_bdry(tri,Next_m3(side)))
	{
	    (void) printf("WARNING in delete_side_of_tri(), "
			  "can't delete side since this would produce "
			  "a folded back bond pair\n");
	    return NO;
	}
	if (is_side_bdry(tri,side))
	{
	    bt = Bond_tri_on_side(tri,side);
	    b = bt->bond;
	    c = bt->curve;
	    if (!b->prev && !b->next)
	    {
		(void) printf("WARNING in delete_side_of_tri(), "
			      "can't delete side corresponding to a "
			      "single bond curve\n");
		return NO;
	    }
	    average_points(NO,b->start,Hyper_surf_element(tri),Hyper_surf(s),
	                      b->end,Hyper_surf_element(tri),Hyper_surf(s));
	    if (b->prev)
		return delete_start_of_bond(b,c);
	    else
		return delete_end_of_bond(b,c);
	}
	else
	{
	    nbtri = Tri_on_side(tri,side);
	    for (nbside = 0; nbside < 3; ++nbside)
		if (tri == Tri_on_side(nbtri,nbside))
		    break;
	    if (nbside == 3)
	    {
		screen("ERROR in delete_side_of_tri(), "
		       "inconsistent interface, can find neighbor side\n");
		clean_up(ERROR);
		return NO;
	    }
	    if (is_side_bdry(nbtri,Prev_m3(nbside)) &&
		    is_side_bdry(nbtri,Next_m3(nbside)))
	    {
	        (void) printf("WARNING in delete_side_of_tri(), "
			      "can't delete side since this would produce "
			      "a folded back bond pair on the adjacent tri\n");
	        return NO;
	    }
	    p[0] = Point_of_tri(tri)[side];
	    p[1] = Point_of_tri(tri)[Next_m3(side)];
	    average_points(NO,p[0],Hyper_surf_element(tri),Hyper_surf(s),
	                      p[1],Hyper_surf_element(tri),Hyper_surf(s));
	    if (!Boundary_point(p[0]))
	    {
	        nt = set_tri_list_around_point(p[0],tri,&tris,intfc);
		for (i = 0; i < nt; ++i)
		{
		    j = Vertex_of_point(tris[i],p[0]);
		    Point_of_tri(tris[i])[j] = p[1];
		}
	    }
	    else if (!Boundary_point(p[1]))
	    {
	        nt = set_tri_list_around_point(p[1],tri,&tris,intfc);
		for (i = 0; i < nt; ++i)
		{
		    j = Vertex_of_point(tris[i],p[1]);
		    Point_of_tri(tris[i])[j] = p[0];
		}
	    }
	    else
	    {
	      /* This should be a rare case, so I will use a shotgun approach */
	      NODE    **nn;
	      CURVE   **cc;
	      SURFACE **ss;
	      TRI     *t;

	      for (nn = intfc->nodes; nn && *nn; ++nn)
		if ((*nn)->posn == p[0])
		  (*nn)->posn = p[1];
	      for (cc = intfc->curves; cc && *cc; ++cc)
	      {
		for (b=(*cc)->first; b!=NULL; b=b->next)
		{
		  if (b->start == p[0])
		    b->start = p[1];
		  if (b->end == p[0])
		    b->end = p[1];
		}
	      }
	      for (ss = intfc->surfaces; ss && *ss; ++ss)
	      {
	    	for (t = first_tri(*ss); t != NULL; t = t->next)
		{
		  for (j = 0; j < 3; ++j)
		  {
		    if (Point_of_tri(t)[j] == p[0])
		      Point_of_tri(t)[j] = p[1];
		  }
		}
	      }
	    }
	    if (!collapse_tri_on_side(tri,s,side) ||
	        !collapse_tri_on_side(nbtri,s,nbside))
	    {
	        (void) printf("WARNING in delete_side_of_tri(), "
			      "failed to collapse at tri\n");
	        return NO;
	    }
	}
	if (debugging("consistency"))
	{
	  if (!consistent_interface(intfc))
	  {
	    screen("ERROR in delete_side_of_tri(), output interface is "
		   "inconsistent\n");
	    clean_up(ERROR);
	  }
	}
	return YES;
}		/*end delete_side_of_tri*/

/*
*			collapse_tri_on_side():
*
*	Utility subroutine for delete_side_of_tri(),  this function assumes
*	that the POINT pointers for the edge being removed have already
*	been reset. This function resets the neighbor pointers and deletes
*	the tri from the interface.
*/

LOCAL	boolean collapse_tri_on_side(
	TRI     *tri,
	SURFACE *s,
	int     side)
{
	TRI      *tn, *tp;
	BOND_TRI *bt;
	int      i, bsp, bsn;

	bsp = is_side_bdry(tri,Prev_m3(side));
	bsn = is_side_bdry(tri,Next_m3(side));

	if (bsp && bsn)
	{
	    (void) printf("WARNING in collapse_tri_on_side(), "
			  "can't collapse to overlapping bonds\n");
	    return NO;
	}
	else if (bsp)
	{
	    bt = Bond_tri_on_side(tri,Prev_m3(side));
	    tn = Tri_on_side(tri,Next_m3(side));
	    link_tri_to_bond(bt,tn,s,bt->bond,bt->curve);
	}
	else if (bsn)
	{
	    bt = Bond_tri_on_side(tri,Next_m3(side));
	    tp = Tri_on_side(tri,Prev_m3(side));
	    link_tri_to_bond(bt,tp,s,bt->bond,bt->curve);
	}
	else
	{
	    tn = Tri_on_side(tri,Next_m3(side));
	    tp = Tri_on_side(tri,Prev_m3(side));
	    for (i = 0; i < 3; ++i)
	    {
		if (Tri_on_side(tn,i) == tri)
		    Tri_on_side(tn,i) = tp;
		if (Tri_on_side(tp,i) == tri)
		    Tri_on_side(tp,i) = tn;
	    }
	}
	remove_tri_from_surface(tri,s,NO);
	return YES;
}		/*end collapse_tri_on_side*/

EXPORT	ORIENTATION orientation_of_bond_at_tri(
	BOND *b,
	TRI  *tri)
{
	int side = side_of_tri_with_bond(b,tri);
	if (side < 3)
	{
	    FT_POINT *p = Point_of_tri(tri)[side];
	    FT_POINT *np = Point_of_tri(tri)[Next_m3(side)];
	    if ((b->start == p) && (b->end == np))
	        return POSITIVE_ORIENTATION;
	    if ((b->end == p) && (b->start == np))
	        return NEGATIVE_ORIENTATION;
	}
	return ORIENTATION_NOT_SET;
}		/*end orientation_of_bond_at_tri*/

EXPORT	int side_of_tri_with_bond(
	BOND *b,
	TRI  *tri)
{
	int side;
	for (side = 0; side < 3; ++side)
	{
	    if ((is_side_bdry(tri,side)) && (b == Bond_on_side(tri,side)))
		break;
	}
	return side;
}		/*end side_of_tri_with_bond*/


/*
*			next_point():
*
*	Called via the macro next_point().
*
*	Returns through its last three arguments the next point on a 
*	specified interface.   "Next Point" is defined as follows:
*
*	Before a TRUE call to next_point() on a given INTERFACE, it
*	should first be initialized to the start of the INTERFACE
*	by a call with NULL arguments:
*
*			(void) next_point(intfc,NULL,NULL,NULL);
*
*	At the first call to next_point(intfc,..) following such
*	initialization the point returned is the first point on the 
*	first hypersurface of intfc.   
*
*	Otherwise, if the last point returned was not at the
*	end of a hypersurface, then the next point on that hypersurface
*	is returned. Otherwise the first point on the next hypersurface
*	is returned. In this way all points of all HYPER SURFACES are
*	traversed.   Note that lower dimensional structures such as
*	NODES and CURVES in 3d will in general be encountered several times.
*
*	The pointers *P, *HSE, *HS are set to point to pointers to the
*	point, its hypersurface element and hypersurface.
*
*	In two space dimensions HS is a curve on which P lies and
*	HSE is the bond on that	curve STARTING at the point.
*	For an endpoint of a curve, *HSE will be NULL - thus the point is
*	at the end of the bond *C->last if a bond is required.
*
*	In three space dimensions *HS is a surface containing P,  and *HSE
*	is a triangle on that surface with P as a vertex.
*	
*	next_point()  may be freely intermixed with calls to
*	next_bond()  and  next_curve().
*
*	next_point  returns either 1 when the next point is a valid point,
*	or 0 if there is no remaining point, or if an error occurs.  
*/

/*ARGSUSED*/
LIB_LOCAL boolean next_point1d(
	INTERFACE 	*intfc,
	FT_POINT 		**p,
        HYPER_SURF_ELEMENT **hse,
	HYPER_SURF 	**hs)
{
	struct Table 	*T;

	if ((T = table_of_interface(intfc)) == NULL)
	    return NO;

	if (intfc->points == NULL)
	{
	    if (p != NULL)
		*p = NULL;
	    if (hse != NULL)
	        *hse = NULL;
	    if (hs != NULL)
	        *hs = NULL;
	    return NO;
	}

	       			/* Reinitialize to start: */
	if (p == NULL)
	{
	    T->cur_point = intfc->points-1;
	    return YES;
	}

	if (*++(T->cur_point) == NULL)			/* No more Points */
	{
	    *p = NULL;
	    *hse = NULL;
	    *hs = NULL;
	    return NO;
	}

	*p = *T->cur_point;
	*hs = Hyper_surf(*p);
	*hse = NULL;

	return YES;
}		/*end next_point1d*/

EXPORT  boolean next_hypersurface1d(
	INTERFACE	*intfc,
	HYPER_SURF	**HS)
{
	FT_POINT	*P;
	if (HS == NULL)		/* Reinitialize to start */
	    return next_point(intfc,NULL,NULL,NULL);

	P = (*HS != NULL) ? Point_of_hs(*HS) : NULL;
	return next_point(intfc,&P,NULL,HS);
}		/*end next_hypersurface1d*/



LIB_LOCAL boolean next_point2d(
	INTERFACE	*intfc,
	FT_POINT		**P,
	HYPER_SURF_ELEMENT **HSE,
	HYPER_SURF	**HS)
{
	struct Table	*T;

	if ((T = table_of_interface(intfc)) == NULL)
	    return NO;

	       			/* Reinitialize to start: */
	if (P == NULL)
	{
	    T->cur_curve = intfc->curves-1;
	    T->cur_bond = NULL;
	    return YES;
	}

	       			/* Last Point was at end of Curve: */
	if (T->cur_bond==NULL)
	{
	    if (*++(T->cur_curve) == NULL) /* No more Curves */
	    {
	        *P = NULL;
	        *HSE = NULL;
	        *HS = NULL;
	        return NO; 			
	    }
	    else
		T->cur_bond = (*T->cur_curve)->first;
	}
	else    
	    T->cur_bond = T->cur_bond->next;	/* Go to Next Bond */

	*HS = Hyper_surf(*(T->cur_curve));
	if (T->cur_bond == NULL) 	
	{
	    *P = (*(T->cur_curve))->last->end;	/* P at End of Curve */
	    *HSE = Hyper_surf_element((*(T->cur_curve))->last);
	}
	else
	{
	    *P = T->cur_bond->start;
	    *HSE = Hyper_surf_element(T->cur_bond);
	}
	if (*P == NULL) /* No point P! */
	    return NO;

	return YES;
}		/*end next_point2d*/


/*
*				next_bond():
*
*	Similiar in function to next_point(), except that it loops
*	over bonds of an INTERFACE.   May be feely intermixed
*	with calls to next_point() and next_curve().   As with
*	next_point(), next_bond() should be initialized to the
*	new INTERFACE by a call with NULL arguments:
*
*			(void) next_bond(intfc,NULL,NULL);
*
*	This also reinitialzes next_point() and next_curve().
*
*	Returns 1 if a next BOND is found or 0 otherwise.
*/


EXPORT boolean next_bond(
	INTERFACE	*intfc,
	BOND		**B,
	CURVE		**C)
{
	struct Table	*T;

	if ((T = table_of_interface(intfc)) == NULL)
	    return NO;

        if (intfc->curves == NULL)
            return NO;
	    			/* Reinitialize to start: */
	if (B == NULL)
	{
	    T->cur_curve = intfc->curves-1;
	    T->cur_bond = NULL;
	    return YES;
	}

	if (intfc->curves == NULL)
	{
	    *B = NULL;
	    *C = NULL;
	    return NO;
	}

	    			/* Last Point was at end of Curve: */
	if (T->cur_bond==NULL || T->cur_bond->next == NULL)
	{
	    if (*++(T->cur_curve) == NULL) /* No more Curves */
	    {
	    	*B = NULL;
	    	*C = NULL;
	    	return NO;
	    }
	    else
	    	T->cur_bond = (*T->cur_curve)->first;
	}
	else    
	    T->cur_bond = T->cur_bond->next;	/* Go to Next Bond */

	*B = T->cur_bond;
	*C = *(T->cur_curve);

	return YES;
}		/*end next_bond*/


/*
*				next_curve():
*
*	Similiar in function to next_point(), except that it loops
*	over curves of an INTERFACE.   May be feely intermixed
*	with calls to next_point() and next_bond().   Should be
*	initialized to a new INTERFACE by a call with NULL argument:
*
*			(void) next_curve(intfc,NULL);
*
*	This also reinitializes next_point() and next_bond() for
*	that INTERFACE.
*
*	Returns 1 if a next curve is found or 0 otherwise.
*/

EXPORT boolean next_curve(
	INTERFACE	*intfc,
	CURVE		**C)
{
	struct Table	*T;
	static	BOND	Btmp;

	if ((T = table_of_interface(intfc)) == NULL)
	    return NO;

	if (C == NULL) /* Reinitialize to start: */
	{
	    T->cur_curve = intfc->curves-1;
	    T->cur_bond = NULL;
	    return YES;
	}

	    					/* No more Curves */
	if (*++(T->cur_curve) == NULL)
	{
	    *C = NULL;
	    return NO;
	}
	else
	{
	    Btmp.next = (*T->cur_curve)->first;
	    T->cur_bond = &Btmp;
	}

	*C = *(T->cur_curve);

	return YES;
}		/*end next_curve*/

LIB_LOCAL	boolean	next_hypersurface2d(
	INTERFACE	*intfc,
	HYPER_SURF	**HS)
{
	CURVE	*C;

	if (HS == NULL)
	    return next_curve(intfc,NULL);

	C = Curve_of_hs(*HS);
	*HS = NULL;
	if (next_curve(intfc,&C))
	{
	    *HS = Hyper_surf(C);
	    return YES;
	}
	return NO;
}		/*end next_hypersurface2d*/


/*
*				new_table():
*
*	Allocates a new interface table and inserts it at end of the list.
*	Does not allocate any storage for the interface.   Returns 1 if
*	successful or 0 if no space available.   Also sets the cur_IT
*	to the new table and the cur_intfc to the new INTERFACE..
*/

LOCAL struct Table *new_table(
	int		dim)
{
	I_USER_INTERFACE *uh = i_user_hook(dim);
	struct Table	 *next_IT;

	if ((next_IT = GetNextFreeTable()) == NULL)
	    return NULL;
	if (LastIT == NULL)
	{
	    FirstIT = LastIT = next_IT;
	}
	else
	{
	    LastIT->next = next_IT;
	    next_IT->prev = LastIT;
	    LastIT = next_IT;
	}

	cur_IT = next_IT;
	cur_IT->next = NULL;
	cur_IT->first_chunk = cur_IT->last_chunk = NULL;
	cur_IT->big_chunks = NULL;
	cur_IT->remainder = 0;
	cur_IT->num_chunks = 0;
	cur_IT->_ChunkSize = uh->_ChunkSize;
	cur_IT->top = NULL;
	cur_intfc = NULL;
	cur_intfc = cur_IT->interface = (INTERFACE *)store(uh->size_interface);
	cur_intfc->table = cur_IT;
	cur_IT->fixed_grid = NO;
	cur_IT->new_grid = YES;
	cur_IT->max_comp = -9999;
	cur_IT->min_comp =  9999;
	cur_IT->ext_comp =  EXTERIOR_COMP;
	cur_IT->_no_topology_lists = NO;

	cur_intfc->modified = YES;

	return cur_IT;
}		/*end new_table*/

LOCAL	struct Table	*FirstFreeTable = NULL, *LastFreeTable = NULL;

LOCAL	struct Table	*GetNextFreeTable(void)
{
	struct Table *table;

	if (FirstFreeTable != NULL)
	{
	    table = FirstFreeTable;
	    FirstFreeTable = FirstFreeTable->next;
	    if (FirstFreeTable != NULL)
	    	FirstFreeTable->prev = NULL;
	    else
	    	LastFreeTable = NULL;
	}
	else
	{
	    scalar(&table,sizeof(struct Table));
	}
	if (table == NULL)
	    return NULL;
	/* Ensure all default fields are NULL */
	zero_scalar(table,sizeof(struct Table));
	return table;
}		/*end GetNextFreeTable*/

LOCAL	void	ReturnTableToFreeList(
	struct Table *table)
{
	if (table == NULL)
	    return;
	if (LastFreeTable == NULL)
	{
	    FirstFreeTable = LastFreeTable = table;
	    table->prev = table->next = NULL;
	    return;
	}
	LastFreeTable->next = table;
	table->prev = LastFreeTable;
	table->next = NULL;
	LastFreeTable = table;
}		/*end ReturnTableToFreeList*/

LIB_LOCAL void print_table_list(void)
{
	struct Table	*T;

	(void) printf("\n\nInterface Table List\n");
	(void) printf("Current Table = %llu, Current Interface = %llu\n",
	          table_number(cur_IT),interface_number(cur_intfc));
	for (T = FirstIT; T != NULL; T = T->next)
	    print_table(T);
	(void) printf("\n\n");
	(void) printf("End of Interface Table List\n\n");
}		/*end print_table_list*/

LIB_LOCAL void print_table(struct Table *T)
{
	struct Chunk	*chunk;

	(void) printf("\nTable %llu",table_number(T));
	(void) printf("   Interface = %llu    Top = %p  Remainder = %lu\n",
	    	      interface_number(T->interface),(POINTER)T->top,
	    	      T->remainder);
	(void) printf("Blocks-> ");
	for (chunk = T->first_chunk; chunk != NULL; chunk=chunk->next)
	    (void) printf("  %p",chunk);
	(void) printf("\nEnd of Table %llu",table_number(T));
}		/*end print_table*/



/*
*			exists_interface():
*
*	Returns 1 if an interface  intfc  exists, or 0 otherwise.
*/

EXPORT boolean  exists_interface(
	INTERFACE	*intfc)
{
	struct Table	*T;

	for (T=FirstIT; T!=NULL; T=T->next)
	    if (intfc == T->interface)
		return YES;
	return NO;
}		/*end exists_interface*/


/*
*			table_of_interface():
*
*	Returns pointer to the Table entry of an interface, or NULL
*	if interface not in the table.
*/

EXPORT struct Table *table_of_interface(
	INTERFACE	*intfc)
{
	struct Table	*T;

	for (T=FirstIT; T!=NULL; T=T->next)
	    if (intfc == T->interface)
	    	return T;
	return NULL;
}		/*end table_of_interface*/


EXPORT  struct Table *interface_table_list(void)
{
	return FirstIT;
}		/*end interface_table_list*/


/*
*				store():
*
*	Basic allocation routine - keeps track of storage ft_assignment
*	for all interfaces.
*/

EXPORT  POINTER store(
	size_t	size)
{
	static ALIGN empty;		/* Empty Storage Request */
	size_t       naligns;
	POINTER	     oldtop;

	naligns = num_aligns(size);
	if (naligns  == 0)
	    return (POINTER)&empty;

	if (size > cur_IT->_ChunkSize)
	{
	    struct Chunk *chunk;
	    size_t csize;
	    csize = naligns*sizeof(ALIGN)+sizeof(struct Chunk)-sizeof(ALIGN);
	    scalar(&chunk,csize);
	    if (chunk == NULL)
	        return NULL;
	    zero_scalar(chunk,csize);
	    chunk->prev = cur_IT->big_chunks;
	    if (cur_IT->big_chunks != NULL)
		cur_IT->big_chunks->next = chunk;
	    cur_IT->big_chunks = chunk;
	    return (POINTER) ChunkTop(chunk);
	}

	    	/* Allocate more storage in Current Interface Table: */

	if (cur_IT->remainder < naligns)
	    if (new_chunk(cur_IT) == NULL)
	    	return NULL;

	cur_IT->remainder -= naligns ;
	oldtop = (POINTER)(cur_IT->top);
	cur_IT->top += naligns;
	if (cur_intfc != NULL)
	{
	    cur_intfc->modified = YES;
	}

	/*Ensure storage is initialized to zero*/
	zero_scalar(oldtop,size);
	return oldtop;
}		/*end store*/

/*
*			init_table_Store():
*/

EXPORT	POINTER	init_table_Store(
	size_t		size,
	INIT_DATA	*init)
{
	INTERFACE	*cintfc = current_interface();
	POINTER		ptr;

	set_current_interface(i_intfc(init));
	ptr = Store(size);
	set_current_interface(cintfc);
	return ptr;
}		/*end init_table_Store*/

/*
*			Store():
*
*	Calls store() but leaves the current_interface()->modified 
*	flag unchanged.
*/

EXPORT	POINTER Store(
	size_t		size)
{
	boolean		save;
	POINTER		ptr;

	save = current_interface()->modified;
	ptr = store(size);
	current_interface()->modified = save;
	return ptr;
}		/*end Store*/


/*
*			new_chunk():
*
*	Allocate more storage in Current Interface Table
*
*/

LIB_LOCAL  struct Chunk *new_chunk(
	struct Table	*table)
{
	struct Chunk	*last_chunk;

	    	/* Find Storage Location for Next Chunk: */

	if ((last_chunk = GetNextFreeChunk(table)) == NULL)
	    return NULL;
	if (table->last_chunk == NULL)
	{
	    table->last_chunk = table->first_chunk = last_chunk;
	    last_chunk->next = last_chunk->prev = NULL;
	}
	else
	{
	    table->last_chunk->next = last_chunk;
	    last_chunk->prev = table->last_chunk;
	    table->last_chunk = last_chunk;
	    last_chunk->next = NULL;
	}

	table->remainder = table->_ChunkSize/sizeof(ALIGN);
	table->top = ChunkTop(last_chunk);
	++table->num_chunks;

	return last_chunk;
}		/*end new_chunk*/

LOCAL	struct Chunk	*FirstFreeChunk = NULL, *LastFreeChunk = NULL;

LOCAL	struct Chunk *GetNextFreeChunk(
	struct Table	*table)
{
	struct Chunk *chunk;
	size_t csize = table->_ChunkSize + sizeof(struct Chunk) - sizeof(ALIGN);

	if (FirstFreeChunk != NULL)
	{
	    chunk = FirstFreeChunk;
	    FirstFreeChunk = FirstFreeChunk->next;
	    if (FirstFreeChunk != NULL)
	    	FirstFreeChunk->prev = NULL;
	    else
	    	LastFreeChunk = NULL;
	}
	else
	    scalar(&chunk,csize);
	if (chunk == NULL)
	    return NULL;
	zero_scalar(chunk,csize);
	return chunk;
}		/*end GetNextFreeChunk*/

LOCAL	void	ReturnChunksToFreeList(
	struct Chunk *first_chunk,
	struct Chunk *last_chunk)
{
	if (first_chunk == NULL || last_chunk == NULL)
	    return;
	if (LastFreeChunk == NULL)
	{
	    FirstFreeChunk = first_chunk;
	    LastFreeChunk = last_chunk;
	    first_chunk->prev = last_chunk->next = NULL;
	}
	else
	{
	    LastFreeChunk->next = first_chunk;
	    first_chunk->prev = LastFreeChunk;
	    LastFreeChunk = last_chunk;
	    last_chunk->next = NULL;
	}
}		/*end ReturnChunksToFreeList*/

EXPORT	const char *i_boundary_type_as_string(
	int b_type)
{
	static char s[256]; 
	switch (b_type)
	{
	case SUBDOMAIN_BOUNDARY:
	    return "SUBDOMAIN_BOUNDARY";
	case REFLECTION_BOUNDARY:
	    return "REFLECTION_BOUNDARY";
	case MIXED_TYPE_BOUNDARY:
	    return "MIXED_TYPE_BOUNDARY";
	case UNKNOWN_BOUNDARY_TYPE:
	    return "UNKNOWN_BOUNDARY_TYPE";
        case OPEN_BOUNDARY:
            return "OPEN_BOUNDARY";
#if defined(USE_OVERTURE)
        case AMR_SUBDOMAIN_BOUNDARY:
            return "AMR_SUBDOMAIN_BOUNDARY";
#endif /* if defined(USE_OVERTURE) */
	default:
	    (void) sprintf(s,"%d -- ** UNKNOWN boundary type**",b_type);
	    return s;
	}
}		/*end i_boundary_type_as_string*/

/*ARGSUSED*/
LIB_LOCAL	void i_fprint_boundary_type(
	FILE		*file,
	const char	*mesg1,
	int		b_type,
	const char	*mesg2,
	INTERFACE       *intfc)
{
	if (mesg1 != NULL)
	    (void) fprintf(file,"%s",mesg1);
	(void) fprintf(file,"%s",i_boundary_type_as_string(b_type));
	if (mesg2 != NULL)
	    (void) fprintf(file,"%s",mesg2);
}		/*end i_fprint_boundary_type*/

EXPORT	int i_read_boundary_type_from_string(
	const char	*type)
{
	int i;
	static struct { const char *name; int type; } boundary_type_map[] = {
	    {"SUBDOMAIN_BOUNDARY", SUBDOMAIN_BOUNDARY},
	    {"S",                  SUBDOMAIN_BOUNDARY},
	    {"PERIODIC_BOUNDARY",  SUBDOMAIN_BOUNDARY},
	    {"PE",                 SUBDOMAIN_BOUNDARY},
	    {"REFLECTION_BOUNDARY",REFLECTION_BOUNDARY},
	    {"R",                  REFLECTION_BOUNDARY},
	    {"MIXED_TYPE_BOUNDARY",MIXED_TYPE_BOUNDARY},
	    {"M",                  MIXED_TYPE_BOUNDARY},
	    {"OPEN_BOUNDARY",      OPEN_BOUNDARY},
	    {"O",      		   OPEN_BOUNDARY},
	    {NULL,                 UNKNOWN_BOUNDARY_TYPE}
	};
	for (i = 0; boundary_type_map[i].name != NULL; ++i)
	{
	    if (strcasecmp(type,boundary_type_map[i].name) == 0)
	        return boundary_type_map[i].type;
	}
	return UNKNOWN_BOUNDARY_TYPE;
}		/*end i_read_boundary_type_from_string*/

EXPORT	uint64_t interface_number(
	INTERFACE *intfc)
{
	struct Table	*T;
	uint64_t i;

	if (debugging("addresses"))
	    return ptr2ull(intfc);

	if (intfc == NULL)
	    return ptr2ull(intfc);
	for (i = 1, T=FirstIT; T !=NULL; T=T->next)
	    if (intfc == T->interface)
		return i;
	return ptr2ull(intfc);
}		/*end interface_number*/

EXPORT	uint64_t table_number(
	struct Table *table)
{
	struct Table	*T;
	uint64_t i;

	if (debugging("addresses"))
	    return ptr2ull(table);

	for (i = 1, T=FirstIT; T !=NULL; T=T->next)
	    if (table == T)
	        return i;
	return ptr2ull(table);
}		/*end table_number*/

EXPORT	uint64_t point_number(
	FT_POINT		*point)
{
	if (debugging("addresses"))
	    return ptr2ull(point);
	if (current_interface()->dim == 1)
	{
	    FT_POINT    **p;
	    uint64_t i;

	    if ((point == NULL) || (point->interface == NULL))
	    	return ptr2ull(point);
	    for (i = 1, p = point->interface->points; p && *p; ++i, ++p)
	    	if (*p == point)
		    return i;
	}
	return ptr2ull(point);
}		/*end point_number*/

EXPORT	uint64_t node_number(
	NODE *node)
{
	NODE **n;
	uint64_t i;

	if (debugging("addresses"))
	    return ptr2ull(node);

	if ((node == NULL) || (node->interface == NULL))
	    return ptr2ull(node);
	for (i = 1, n = node->interface->nodes; n && *n; ++i, ++n)
	    if (*n == node)
		return i;
	return ptr2ull(node);
}		/*end node_number*/

EXPORT	uint64_t bond_number(
	BOND		*bond,
	INTERFACE	*intfc)
{
	CURVE	**c;
	BOND	*b;
	uint64_t i;

	if (debugging("addresses"))
	    return ptr2ull(bond);

	if ((bond == NULL) || (intfc == NULL))
	    return ptr2ull(bond);

	if (debugging("numbers"))
	{
	    for (i = 1, c = intfc->curves; c && *c; ++c)
	        for (b = (*c)->first; b != NULL; ++i, b = b->next)
	    	    if (b == bond)
		        return i;
	}
	return ptr2ull(bond);
}		/*end bond_number*/

EXPORT	uint64_t curve_number(
	CURVE *curve)
{
	CURVE **c;
	uint64_t i;

	if (debugging("addresses"))
	    return ptr2ull(curve);

	if ((curve == NULL) || (curve->interface == NULL))
	    return ptr2ull(curve);

	for (i = 1, c = curve->interface->curves; c && *c; ++i, ++c)
	    if (*c == curve)
		return i;
	return ptr2ull(curve);
}		/*end curve_number*/

EXPORT	uint64_t hypersurface_number(
	HYPER_SURF *hyper_surf)
{
	HYPER_SURF **hs;
	uint64_t i;

	if (debugging("addresses"))
	    return ptr2ull(hyper_surf);

	if ((hyper_surf == NULL) || (hyper_surf->interface == NULL))
	    return ptr2ull(hyper_surf);

	for (i = 1, hs = hyper_surf->interface->hss; hs && *hs; ++i, ++hs)
	    if (*hs == hyper_surf)
		return i;
	return ptr2ull(hyper_surf);
}		/*end hypersurface_number*/

EXPORT	uint64_t hypersurface_boundary_number(
	HYPER_SURF_BDRY *hyper_surf_bdry)
{
	HYPER_SURF_BDRY **hsb;
	uint64_t i;

	if (debugging("addresses"))
	    return ptr2ull(hyper_surf_bdry);

	if ((hyper_surf_bdry == NULL) ||
	    (hyper_surf_bdry->interface == NULL))
	    return ptr2ull(hyper_surf_bdry);

	for (i=1, hsb=hyper_surf_bdry->interface->hses; hsb && *hsb; ++i,++hsb)
	    if (*hsb == hyper_surf_bdry)
		return i;

	return ptr2ull(hyper_surf_bdry);
}		/*end hypersurface_boundary_number*/

EXPORT	uint64_t hypersurface_element_number(
	HYPER_SURF_ELEMENT		*hse,
	INTERFACE			*intfc)
{
	if (debugging("addresses"))
	    return ptr2ull(hse);

	if ((hse == NULL) || (intfc == NULL))
	    return ptr2ull(hse);

	switch (intfc->dim)
	{
	case 2:
	    return bond_number(Bond_of_hse(hse),intfc);
	case 3:
	    return tri_number(Tri_of_hse(hse),intfc);
	}
	return ptr2ull(hse);
}		/*end hypersurface_element_number*/

EXPORT void delete_from_cross_list(
	CROSS		*cr)
{
	if (!cr)
	    return;
	if (cr->prev)
	    cr->prev->next = cr->next;
	if (cr->next)
	    cr->next->prev = cr->prev;
}		/*end delete_from_cross_list*/

/*
*			i_random01():
*
*	Returns a floating point random number between 0 and 1
*	Is machine specific.
*/

EXPORT	double i_random01(
	INTERFACE *intfc)
{
	return erand48(Random01_seed(intfc));
}		/*end i_random01*/


EXPORT	uint64_t bond_tri_number(
	BOND_TRI	*bt,
	INTERFACE	*intfc)
{
	CURVE	 **c;
	BOND     *b;
	BOND_TRI **btris;
	uint64_t i;

	if (debugging("addresses"))
	    return ptr2ull(bt);
	if ((bt == NULL) || (intfc == NULL))
	    return ptr2ull(bt);
	if (debugging("numbers"))
	{
	    for (i = 1, c = intfc->curves; c && *c; ++c)
	    	for (b = (*c)->first; b != NULL; ++i, b = b->next)
		    for (btris = Btris(b); btris && *btris; ++btris)
	    	        if (bt == *btris)
		            return i;
	}
	return ptr2ull(bt);
}		/*end bond_tri_number*/

EXPORT	uint64_t tri_number(
	TRI		*tri,
	INTERFACE	*intfc)
{
	SURFACE	**s;
	TRI	*t;
	uint64_t i;

	if (debugging("addresses"))
	    return ptr2ull(tri);
	if (debugging("numbers"))
	{
	    if ((tri == NULL) || (intfc == NULL))
	    	return ptr2ull(tri);
	    for (i = 1, s = intfc->surfaces; s && *s; ++s)
	    	for (t = first_tri(*s); t != NULL; ++i, t = t->next)
	    	    if (t == tri)
			return i;
	}
	return ptr2ull(tri);
}		/*end tri_number*/

EXPORT	uint64_t surface_number(
	SURFACE *surface)
{
	SURFACE **s;
	uint64_t i;

	if (debugging("addresses"))
	    return ptr2ull(surface);

	if ((surface == NULL) || (surface->interface == NULL))
	    return ptr2ull(surface);
	for (i = 1, s = surface->interface->surfaces; s && *s; ++i, ++s)
	    if (*s == surface)
		return i;

	return ptr2ull(surface);
}		/*end surface_number*/

/*Notes: This function is to make curve coincide with the implicit assumption */
/*	in function read_print_surface and copy_tris that for any pair of */
/*      bonds b1 and b2 on a curve c,  and any index i, */
/*	Btris(b1)[i] and Btris(b2)[i] lie on the same surface */
/*	with the same orientation with respect to c. */
/* #bjet2 */
EXPORT	void	order_interface(
	INTERFACE	*intfc)
{

	CURVE *cc,**cur;
	BOND  *bb;
	BOND_TRI **btris,**btris0,*tmpbtri;
	int   i,j,k;
	for (cur = intfc->curves; cur && *cur; cur++)
	{
	    cc = *cur;
	    for (bb=cc->first; bb != NULL; bb=bb->next)
	    {
	        for (j=0,btris0=Btris(cc->first); btris0+j && btris0[j]; ++j)
		{
		    for (i=j,btris=Btris(bb); btris+i && btris[i]; ++i)
		    {
		    
			if((btris[i])->surface == (btris0[j])->surface && i != j)
			{
			    tmpbtri = btris[i];
			    btris[i] = btris[j];
			    btris[j] = tmpbtri;
			}
		    }
		}
	    }
	}
}

/*
 * delete one surface s and all curves ONLY related to s, and all nodes only related to 
 * deleted curves
 */ 

EXPORT void  delete_scn(
	SURFACE  *s)
{
INTERFACE	*intfc = s->interface; 
SURFACE		*surf;
CURVE		**c, **curves_to_delete;
NODE		**n, **nodes_to_delete;
BOND		*bs;

	if(s != NULL && !delete_surface(s))
	{
	    printf("ERROR delete_scn, delete surface %p failed.\n", s);
	    clean_up(ERROR);
	}
	
	/*ref: cut_out_curves_in_buffer */
	/*it will only delete subdomain curves */
	curves_to_delete = NULL;
	for(c=intfc->curves; c && *c; c++)
	{
	    for (bs = (*c)->first; bs != NULL; bs = bs->next)
	        if (Btris(bs) != NULL)
		    break;

	    if(bs == NULL)
	        add_to_pointers(*c, &curves_to_delete);
	}
	
	for(c=curves_to_delete; c && *c; c++)
	    if (!delete_curve(*c))
	    {
	        printf("ERROR in delete_scn, "
		       "can't delete curve %llu\n",curve_number(*c));
		clean_up(ERROR);
	    }

	/*delete no curve related nodes */
	nodes_to_delete = NULL;
	for (n = intfc->nodes; n && *n; ++n)
	    add_to_pointers(*n, &nodes_to_delete);
	for (n = nodes_to_delete; n && *n; ++n)
	    (void) delete_node(*n);

}

/* two subdomain contact with each other */
/*i_copy_interface */
/*install_subdomain_bdry_curves */
EXPORT  void  reset_nodes_posn(
	INTERFACE	*intfc)
{
	CURVE   **c;

	for(c = intfc->curves; c && *c; c++)
	{
	    /*DEBUG_TMP printf("#reset_nodes_posn, subdomain curve fixed. Curve %d\n", curve_number(*c)); */
	    if(is_closed_curve(*c) && (*c)->start->posn != (*c)->first->start)
	    {
		(*c)->start->posn = (*c)->first->start;
		(*c)->end->posn = (*c)->first->start;
	    }
	}
	for(c = intfc->curves; c && *c; c++)
	{
	    if((*c)->first->start != (*c)->last->end)
		continue;
	    if((*c)->first->start == (*c)->start->posn && 
	       (*c)->last->end == (*c)->end->posn)
		continue;

	    if((*c)->first->start != (*c)->start->posn && 
	       (*c)->last->end == (*c)->end->posn)
	    {
	        /*DEBUG_TMP printf("#reset_nodes_posn,  fixed start node. Curve %d\n",  */
					/*DEBUG_TMP curve_number(*c)); */
		
		if (!delete_from_pointers((*c),&((*c)->start->out_curves)) || 
		     !add_to_pointers((*c),&((*c)->end->out_curves)) )
		    printf("WARNING in reset_nodes_posn, can not delete/add"
		    	   "a out_curve from/to a node\n");
		
		(*c)->start = (*c)->end;
	    }
	    else if((*c)->first->start == (*c)->start->posn && 
	    	(*c)->last->end != (*c)->end->posn)
	    {
	        /*DEBUG_TMP printf("#reset_nodes_posn,  fixed end node. Curve %d\n",  */
					/*DEBUG_TMP curve_number(*c)); */
		
		if ( !delete_from_pointers((*c),&((*c)->end->in_curves)) ||
		     !add_to_pointers((*c),&((*c)->start->in_curves)) )
		    printf("WARNING in reset_nodes_posn, can not delete/add"
		    	   "a in_curve from/to a node\n");
	
		(*c)->end = (*c)->start;
	    }
	    else
	    {
		printf("WARNING in reset_nodes_posn, both nodes of a "
		       "closed curve is inconsistent.\n");
	    }
	}
}	/* end reset_nodes_posn */

