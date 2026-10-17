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
*			fscat2d.c
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Warning: despite my best efforts to document the code in this file,
*	it is still extremely complicated, and sometimes works in very subtle
*	and sneaky ways.  No attempt is made to handle all cases.  The code
*	is hopefully robust enough to handle all common cases, and most of
*	the degenerate cases.  When an unhandled/unimplemented situation 
*	occurs, it is assumed that whatever code calls form_subintfc...
*	backs up and tries again.  For an elliptic interface this usually
*	means regridding, and for hyperbolic one, changing the timestep.
*	For anyone who finds themselves needing to modify or even thoroughly
*	understand the following code, be prepared to spend some time on it,
*	probably more than you would like.  In the end, all I can say is
*	good luck.  3/6/96 BKB 
*/


#define DEBUG_STRING	"fscatter"
#include <front/fdecs.h>

/* temp */
LOCAL int DEBUG_CUT = NO; 
#define float_equal(f1,f2)  (fabs((f1)-(f2)) < 10.0*MACH_EPS)   

typedef struct _OVERLAP {
	struct _OVERLAP	*next;
	struct _OVERLAP *prev;
	struct _OVERLAP *match;
	BOND		*b1, *b2;
	CURVE		*c1, *c2;
	NODE		*n1, *n2;
	FT_POINT		*cr1, *cr2; /* points where b1 and b2 cross cut line */
	ORIENTATION	or1, or2;
	double		dist;
	int		index;	    /* for debugging */
} OVERLAP;

typedef struct _REDUNDANCY {
	struct _REDUNDANCY	*next;
	struct _REDUNDANCY	*prev;
	struct _OVERLAP		*ol1, *ol2;
	double			metric; /* used for sorting these structures */
} REDUNDANCY;


#if defined(DEBUG_STRING)
#define DEBUG_INTERFACE(mesg,intfc)					\
	if (DEBUG)							\
	{								\
		NODE **n;						\
									\
		(void) printf("%s\n",(mesg));				\
		(void) printf("Node boundary flags\n");			\
		for (n = (intfc)->nodes; n && *n; ++n)			\
			print_node_flags(*n);				\
		print_interface(intfc);					\
		if (debugging("front_states"))				\
			show_intfc_states(intfc);			\
	}
#else /* defined(DEBUG_STRING) */
#define DEBUG_INTERFACE(mesg,intfc)
#endif /* defined(DEBUG_STRING) */

/* The first six bits of the Boundary() field are reserved in int.h. */
enum {
	CUT_NODE      = 0x0040,
	X_CUT_NODE    = 0x0080,
	CLIP_NODE     = 0x0100,
	LOCAL_NODE    = 0x0200,
	ADJ_BOND_NODE = 0x0400,
	NODE_MASK     = (CUT_NODE|X_CUT_NODE|CLIP_NODE|LOCAL_NODE|ADJ_BOND_NODE)
};

#define	is_cut_node(n)			(Boundary(n) & CUT_NODE)
#define is_x_cut_node(n)		(Boundary(n) & X_CUT_NODE)
#define	is_clip_node(n)			(Boundary(n) & CLIP_NODE)
#define	is_local_node(n)		(Boundary(n) & LOCAL_NODE)
#define	is_adj_bond_node(n)		(Boundary(n) & ADJ_BOND_NODE)

#define	set_adj_bond_cross_node(n)	(Boundary(n) |= ADJ_BOND_NODE)
#define	set_imported_node(n)		(Boundary(n) &= ~LOCAL_NODE)
#define	clear_node_flags(n)		(Boundary(n) &= ~NODE_MASK)

	/* LOCAL Function Declarations */
LOCAL	INTERFACE	*set_send_intfc(INTERFACE*,int,int,int*,int*);
LOCAL	OVERLAP	*find_double_adj_bond_match(OVERLAP*,OVERLAP*,int,Front*);
LOCAL	OVERLAP	*find_remnant_match(OVERLAP*,OVERLAP*,int);
LOCAL	OVERLAP	*find_single_adj_bond_match(OVERLAP*,OVERLAP*,int,Front*);
LOCAL	OVERLAP	*new_overlap(OVERLAP*,CURVE*,ORIENTATION,int);
LOCAL	FT_POINT	*bond_crosses_cut_line(BOND*,int,double,int);
LOCAL	boolean	check_for_cut_nodes(INTERFACE*);
LOCAL	boolean	delete_redundant_overlaps(OVERLAP*,INTERFACE*,int);
LOCAL	boolean	is_half_remnant_degeneracy(OVERLAP*);
LOCAL	boolean	match_overlaps(OVERLAP*,int,Front*);
LOCAL	boolean	merge_double_physical_cut_nodes(INTERFACE*,int);
LOCAL	boolean	overlap_list(Front*,OVERLAP**,int);
LOCAL	boolean	perform_interface_communication(Front*,int*,int*,int);
LOCAL	boolean	set_subdomain_boundary(Front*,COMPONENT);
LOCAL	double	sqr_separation(FT_POINT*,FT_POINT*,int);
LOCAL	int	curve_on_wrong_side_of_cut_line(CURVE*,int,int,double);
LOCAL	int	local_num_curves_at_node(NODE*);
LOCAL	void	delete_curves_outside_of_cut_line(INTERFACE*,double,int,int);
LOCAL	void	merge_components(COMPONENT,COMPONENT,INTERFACE*);
LOCAL	boolean	merge_overlapping_bonds(OVERLAP*,Front*);
LOCAL	void	point_on_cut_line(INTERFACE*,FT_POINT*,BOND*,double,int);
LOCAL	void	print_node_flags(NODE*);
LOCAL	void	print_overlap(OVERLAP*);
LOCAL	void	remove_from_overlap_list(OVERLAP*,OVERLAP*);
LOCAL	void	set_exterior_node_flags(NODE*,CURVE*);
LOCAL	void	set_interior_node_flags(NODE*,boolean,int);
LOCAL 	boolean 	delete_double_cut_curves(INTERFACE*);
LOCAL 	boolean 	seal_cut_node_curves(CURVE*,INTERFACE*);
LOCAL 	boolean 	separated_nodes(NODE*,NODE*,INTERFACE*);


/* THIS "set_none_local" is exclusively used in cut_interface()
 * for overture under the following situation.
 * When merge_fronts_ver2() is called, the patch interface is
 * first cut to fit into patch's computational grid. At this moment,
 * if the node overlaps with the upper rect. computational grid boundry,
 * this node is set to be LOCAL_NODE; if the node overlaps with the
 * lower rect. computational grid boundry, this node is set to be NONE_LOCAL.
 * To distinguish these two cases, we use this flag. When
 * set_none_local = YES, set_imported_node() is called, the node is
 * set to be NONE_LOCAL.
 *
 * This process will make the consistence for clip_interface_with_rect().
 * In clip_interface_with_rect(), nodes overlap with the upper rect. boundary
 * are set to be NONE_LOCAL. nodes overlaps with the lower rect. bondary
 * are set to be LOCAL. Thus when merge_interface() is called. These
 * patch interfaces will be stitched together with the consistent node flags.
 */
LOCAL   int     set_none_local = NO;
LOCAL   int     set_min_sc_sep = NO; /* This flag is used in cut_interface() and
                                      * clip_interface_with_rect() only.
                                      * when set_min_sc_sep = YES, min_sc_sep is
                                      * specified to be not grid spacing dependent.
                                      * This flag is used exclusively in the
                                      * interface assembly and distribution process. 
                                      */
LOCAL   int     use_delete_short_bonds = YES;

LOCAL   void    interface_intersection_segment(INTERFACE*,int,
                     int,double,double,double,boolean);
LOCAL   FT_POINT   *bond_crosses_cut_segment(BOND*,int,int,double,double,double);
LOCAL   boolean    cross_segments(double,double,double,double,double,double,
                     double,double,double*);
LOCAL   void    delete_curves_inside_rect(INTERFACE*,double*,double*);
LOCAL   int     curve_in_rect(CURVE*,double*,double*);
LOCAL   int     node_in_rect(NODE*,double*,double*);

/*
*		f_intfc_communication2d():
*
*	This function drives the interface communication.  Communication is
*	only performed if necessary.  Non-reflecting boundaries are processed
*	first.  One coordinate direction is considered at a time, and the
*	interface merged before going to the other direction.  Also, a
*	consistency check is performed on the components, which should catch
*	any errors made in the interface reconstruction.
*/

EXPORT boolean f_intfc_communication2d(
	Front		*fr)
{
	COMPONENT	i_comp;
	INTERFACE	*intfc = fr->interf;
	INTERFACE	*sav_intfc;
	O_NODE		*onode_list;
	RECT_GRID	*gr = fr->rect_grid;
	double		coords[MAXD];
	boolean		status = FUNCTION_SUCCEEDED;
	int		i, dir, dim = gr->dim;
	static boolean	exists_refl[MAXD];
	static boolean	exists_subd[MAXD];
	static int	**refl_lbuf = NULL, **refl_ubuf = NULL;
	static int	**subd_lbuf = NULL, **subd_ubuf = NULL;

	DEBUG_ENTER(f_intfc_communication2d)
	DEBUG_INTERFACE("Interface into f_intfc_communication2d()",
			intfc);

	if (refl_lbuf == NULL)
	{
	    bi_array(&refl_lbuf,MAXD,MAXD,INT);
	    bi_array(&refl_ubuf,MAXD,MAXD,INT);
	    bi_array(&subd_lbuf,MAXD,MAXD,INT);
	    bi_array(&subd_ubuf,MAXD,MAXD,INT);

	    for (dir = 0; dir < dim; ++dir)
	    {
	    	exists_subd[dir] = NO;
	    	exists_refl[dir] = NO;

	    	for (i = 0; i < dim; ++i)
	    	{
	    	    refl_lbuf[i][dir] = 0;
	    	    refl_ubuf[i][dir] = 0;

	    	    subd_lbuf[i][dir] = 0;
	    	    subd_ubuf[i][dir] = 0;
	    	}

	    	if (rect_boundary_type(intfc,dir,0) == SUBDOMAIN_BOUNDARY)
	    	{
	    	    exists_subd[dir] = YES;
	    	    subd_lbuf[dir][dir] = gr->lbuf[dir];
	    	}
	    	else if (rect_boundary_type(intfc,dir,0) == REFLECTION_BOUNDARY)
	    	{
	    	    exists_refl[dir] = YES;
	    	    refl_lbuf[dir][dir] = gr->lbuf[dir];
	    	}

	    	if (rect_boundary_type(intfc,dir,1) == SUBDOMAIN_BOUNDARY)
	    	{
	    	    exists_subd[dir] = YES;
	    	    subd_ubuf[dir][dir] = gr->ubuf[dir];
	    	}
	    	else if (rect_boundary_type(intfc,dir,1) == REFLECTION_BOUNDARY)
	    	{
	    	    exists_refl[dir] = YES;
	    	    refl_ubuf[dir][dir] = gr->ubuf[dir];
	    	}
	    }
	}

	sav_intfc = current_interface();
	set_current_interface(intfc);

	/* Find an interior component on this processor's domain.  This
	 * is needed to initialize the components on the subdomain
	 * boundaries when no physical curves appear on this processor. */

	for (i = 0; i < dim; ++i)
	    coords[i] = grid_center_coord(i,gr);
	i_comp = (intfc->modified) ?
	    long_component(coords,intfc) : component(coords,intfc);

	delete_subdomain_curves(intfc);
	delete_passive_boundaries(intfc);

	DEBUG_INTERFACE("Interface after delete subdomain and passive curves",
			intfc);

	for (dir = 0; dir < dim; ++dir)
	{
	    if (exists_subd[dir])
		status = perform_interface_communication(fr,subd_lbuf[dir],
							 subd_ubuf[dir],dir);
	    if (!status)
	    {
		(void) printf("WARNING in "
			      "f_intfc_communication2d(), "
		              " perform_interface_communication() failed "
		              "at subdomain boundary\n");
	    }
	    status = pp_min_status(status);
	    if (!status)
	    {
		set_current_interface(sav_intfc);
		DEBUG_LEAVE(f_intfc_communication2d)
		return status;
	    }
	}
	for (dir = 0; dir < dim; ++dir)
	{
	    if (exists_refl[dir])
	        status = perform_interface_communication(fr,refl_lbuf[dir],
							 refl_ubuf[dir],dir);
	    if (!status)
	    {
		(void) printf("WARNING in "
			      "f_intfc_communication2d(), "
		              "perform_interface_communication() failed "
		              "at reflection boundary\n");
	    }
	    status = pp_min_status(status);
	    if (!status)
	    {
		set_current_interface(sav_intfc);
		DEBUG_LEAVE(f_intfc_communication2d)
		return status;
	    }
	}
	status = delete_double_cut_curves(intfc);
	status = pp_min_status(status);
	if (status == FUNCTION_FAILED)
	{
	    (void) printf("First call of check_for_cut_nodes() failed\n");
	    (void) printf("Delete double cut nodes and try again.\n");
	    for (dir = 0; dir < dim; ++dir)
	    {
	    	if (exists_subd[dir])
		    status = perform_interface_communication(fr,subd_lbuf[dir],
							 subd_ubuf[dir],dir);
	    	if (!status)
	    	{
		    (void) printf("WARNING in "
			      "f_intfc_communication2d(), "
		              " perform_interface_communication() failed "
		              "at subdomain boundary\n");
	    	}
	    	status = pp_min_status(status);
	    	if (!status)
	    	{
		    set_current_interface(sav_intfc);
		    DEBUG_LEAVE(f_intfc_communication2d)
		    return status;
	    	}
	    }
	    for (dir = 0; dir < dim; ++dir)
	    {
	    	if (exists_refl[dir])
	            status = perform_interface_communication(fr,refl_lbuf[dir],
							 refl_ubuf[dir],dir);
	    	if (!status)
	    	{
		    (void) printf("WARNING in "
			      "f_intfc_communication2d(), "
		              "perform_interface_communication() failed "
		              "at reflection boundary\n");
	    	}
	    	status = pp_min_status(status);
	    	if (!status)
	    	{
		    set_current_interface(sav_intfc);
		    DEBUG_LEAVE(f_intfc_communication2d)
		    return status;
	    	}
	    }
	}

	/* TODO:  a post-processing loop is needed here to shift the
	 * subdomain nodes onto VL.  A problem can occur for periodic 
	 * boundaries on restart, where some accuracy is lost in VL or VU,
	 * so that the subdomains have different sizes on each side of the
	 * domain.  This will leave the curves hanging over the edge on the
	 * shorter side, and these nodes will not be processed when
	 * creating the subdomain boundary.
	 * A better solution would be to guarantee the location of the
	 * virtual boundaries, perhaps by printing them out as an integer
	 * multiple of the mesh spacing instead of an absolute (double)
	 * value. */

	status = set_subdomain_boundary(fr,i_comp);
	if (!status)
	{
	    (void) printf("WARNING in f_intfc_communication2d(), "
	                  "set_subdomain_boundary() failed\n");
	    if (DEBUG)
	    {
	    	(void) printf("Offending interface: \n");
	    	print_interface(fr->interf);
	    }
	}

	/* The following code is intended to tell whether the scatter
	 * succeeded by identifying problems/inconsistencies in the
	 * new interface. */

	if (check_for_cut_nodes(intfc))
	{
	    status = FUNCTION_FAILED;
	    (void) printf("WARNING in f_intfc_communication2d(), "
	                  "check_for_cut_nodes() detected cut node\n");
	}
	else if (check_comps_at_nodes(fr->interf,&onode_list) != 0)
	{
	    status = FUNCTION_FAILED;
	    (void) printf("WARNING in f_intfc_communication2d(), "
	                  "check_comps_at_nodes() detected inconsistency\n");
	    if (DEBUG)
	    {
		print_onode_list(&onode_list);
		(void) printf("Offending interface\n");
		print_interface(fr->interf);
	    }
	}
	if (status == FUNCTION_FAILED) 
	{
	    set_current_interface(sav_intfc);
	    DEBUG_INTERFACE("Interface after f_intfc_communication2d()",intfc);
	    DEBUG_LEAVE(f_intfc_communication2d)
	    return status;
	}

	/* Zero length bonds can be produced, especially on an elliptic
	 * interface.  If this happens AT a node, the component check gets
	 * confused because it computes an angle for each  curve at a node
	 * using only the node position and the adjacent point.  
	 * The operation should be places after checking nodes consistency
	 *                          Xiaolin Li 6/29/09 */

        intfc_delete_very_short_bonds(fr);

	set_current_interface(sav_intfc);

	DEBUG_INTERFACE("Interface after f_intfc_communication2d()",intfc);
	DEBUG_LEAVE(f_intfc_communication2d)
	return status;
}		/*end f_intfc_communication2d*/


/*
*			perform_interface_communication():
*
*	This function sends this processor's interface to adjacent
*	processors, receives an interface from adjacent processors.  Only
*	those directions for which lbuf or ubuf is nonzero are considered.
*	It then adds on the buffer zones by clipping on appropriate pieces
*	of the adjacent domains.  
*/

LOCAL boolean perform_interface_communication(
	Front		*fr,
	int		*lbuf,
	int		*ubuf,
	int		dir)
{
	INTERFACE    *intfc = fr->interf;
	INTERFACE    *send_intfc[2];
	INTERFACE    *recv_intfc;
	PP_GRID	     *pp_grid = fr->pp_grid;
	RECT_GRID    *gr = fr->rect_grid;
	boolean	     status;
	double	     *nor, p[MAXD];
	int	     myid, dst_id[2], src_id[2];
	int	     *G = pp_grid->gmax;
	int	     side;
	int	     j, dim = intfc->dim;
	int	     *buf;
	int	     me[MAXD], him[MAXD];
	static double nors[] = {  1.0,  0.0,  0.0,
				 0.0,  1.0,  0.0,
				 0.0,  0.0,  1.0,
				-1.0,  0.0,  0.0,
				 0.0, -1.0,  0.0,
				 0.0,  0.0, -1.0};

	DEBUG_ENTER(perform_interface_communication)
	DEBUG_INTERFACE("Interface into perform_interface_communication()",
			fr->interf);

	myid = pp_mynode();
	find_Cartesian_coordinates(myid, pp_grid, me);

	/* Throw out the old subdomains. */
	clip_to_interior_region(intfc,lbuf,ubuf);

	DEBUG_INTERFACE("Interface after clip_to_interior_region()",fr->interf);

	for (side = 0; side < 2; ++side)
	{
	    buf = (side == 0) ? lbuf : ubuf;

	    /* Clip off appropriate portions of interface for
	     * communication to adjacent processors.  */

	    if (buf[dir] > 0)
	    {
	        switch (rect_boundary_type(intfc,dir,side))
	        {
	        case SUBDOMAIN_BOUNDARY:
	    	    dst_id[side] = neighbor_id(him,me,dir,side,pp_grid);
	    	    send_intfc[side] = set_send_intfc(intfc,dir,side,me,G);
	    	    break;
	        case REFLECTION_BOUNDARY:
	    	    nor = nors + 3*dir + 9*side;
	    	    dst_id[side] = myid;
	    	    send_intfc[side] = set_send_intfc(intfc,dir,side,me,G);
	    	    p[dir] = (side) ? gr->U[dir] : gr->L[dir];
	    	    for (j = 1; j < dim; ++j)
	    	    {
	    	        int    k = (j+dir)%dim;

			p[k] = 0.5*(gr->U[k] + gr->L[k]);
		    }
		    reflect_interface(send_intfc[side],p,nor);
		    break;
		default:
		    send_intfc[side] = NULL;
		    break;
		}
	    }
	    else
	    {
	    	send_intfc[side] = NULL;
	    }
	}

	for (side = 0; side < 2; ++side)
	{
	    /* Send clipped interfaces to adjacent processors.  For
	     * periodic or reflecting, no communication may be needed. */

	    if (send_intfc[side])
	    {
	        if (myid == dst_id[side])
	    	    copy_interface_into(send_intfc[side],intfc);
	        else
	    	    send_interface(send_intfc[side],dst_id[side]);
	        (void) delete_interface(send_intfc[side]);
	        send_intfc[side] = NULL;
	    }
	}

	for (side = 0; side < 2; ++side)
	{
	    buf = (side == 0) ? lbuf : ubuf;

	    /* Receive interfaces from adjacent processors. */

	    if ((buf[dir] > 0) &&
	        (rect_boundary_type(intfc,dir,side) == SUBDOMAIN_BOUNDARY))
	    {
	        src_id[side] = neighbor_id(him,me,dir,side,pp_grid);
	        if (myid != src_id[side])
	        {
	    	    recv_intfc = receive_interface(src_id[side]);
	    	    copy_interface_into(recv_intfc,intfc);
	    	    (void) delete_interface(recv_intfc);
	        }
	    }
	}

	/* Merge received interfaces onto this processor's interface. */
	status = merge_interface(fr,dir);

	if (!status)
	{
	    (void) printf("WARNING in perform_interface_communication(), "
		          "merge_interface() failed\n");
	}

	DEBUG_LEAVE(perform_interface_communication)
	return status;
}		/*end perform_interface_communication*/


/*
*			clip_to_interior_region():
*
*	Performs the first step in parallel interface communication.
*	This function clips the interface at the edge of the interior
*	region.  The resulting interface will be restricted to the
*	region bounded by the rectangle gr->L,  gr->U,  with the exception
*	that bonds actually crossing the rectangular boundary are
*	preserved (unless it is a reflecting boundary, in which case
*	bonds are clipped right at the boundary).
*
*	Note: this function is really redundant for an elliptic interface,
*	for which this function is reproduced elsewhere before scatter_front()
*	is ever called.  We can get away with entering this function again,
*	and some inefficiency, as long as the node flags are preserved.  If
*	in the future this is not the case, one possible fix would be to add
*	an argument to scatter_front() indicating that it is being called
*	from the elliptic solvers, so that this function should not be
*	entered.  Another possibility might be to check the node flags, and
*	if anything in NODE_MASK is already set, again skip this function.
*/

EXPORT	void	clip_to_interior_region(
	INTERFACE	*intfc,
	int		*lbuf,
	int		*ubuf)
{
	RECT_GRID	*gr = computational_grid(intfc);
	int		dir, dim = gr->dim;
        boolean            force_clip;

	DEBUG_ENTER(clip_to_interior_region)

	for (dir = 0; dir < dim; ++dir)
	{
	    if (lbuf[dir] > 0)
	    {
	        cut_interface(intfc,gr->L[dir],dir,1,YES,
	    	    (rect_boundary_type(intfc,dir,0) == REFLECTION_BOUNDARY) ?
		        YES : NO);
	    }
	    if (ubuf[dir] > 0)
	    {
	        cut_interface(intfc,gr->U[dir],dir,0,YES,
	            (rect_boundary_type(intfc,dir,1) == REFLECTION_BOUNDARY) ?
		        YES : NO);
	    }
	}
	DEBUG_LEAVE(clip_to_interior_region)
}		/*end clip_to_interior_region*/


/*
*			delete_subdomain_curves():
*
*	This is a preparatory step for the communication.  The subdomain
*	boundaries should not be communicated, and so are removed.  They
*	will be regenerated at the end of the reconstruction.
*/

EXPORT	void	delete_subdomain_curves(
	INTERFACE	*intfc)
{
	CURVE		**delete_curves = NULL;
	CURVE		**c;
	NODE		**delete_nodes = NULL;
	NODE		**n;

	DEBUG_ENTER(delete_subdomain_curves)
	delete_curves = NULL;
	for (c = intfc->curves; c && *c; ++c)
	{
	    if (is_subdomain_boundary(Hyper_surf(*c)))
	    {
	    	if (!add_to_pointers(*c,&delete_curves))
		{
		    screen("ERROR in delete_subdomain_curves(), "
			   "add_to_pointers() failed\n");
		    clean_up(ERROR);
		}
	    }
	}
	for (c = delete_curves; c && *c; ++c)
	    (void) delete_curve(*c);

	delete_nodes = NULL;
	for (n = intfc->nodes; n && *n; ++n)
	{
	    if ((((*n)->in_curves == NULL) && ((*n)->out_curves == NULL)) &&
	       !is_source_sink_node(*n))
	    {
	    	if (!add_to_pointers(*n,&delete_nodes))
		{
		    screen("ERROR in delete_subdomain_curves(), "
			   "add_to_pointers() failed\n");
		    clean_up(ERROR);
		}
	    }
	}
	for (n = delete_nodes; n && *n; ++n)
	    (void) delete_node(*n);
	DEBUG_LEAVE(delete_subdomain_curves)
}		/*end delete_subdomain_curves*/


/*
*				set_send_intfc():
*
*	Clips off an appropriate piece of intfc for communication to
*	adjacent processors.  For periodic boundaries, the interface
*	is translated to the opposite side of the global domain.
*/

LOCAL INTERFACE *set_send_intfc(
	INTERFACE	*intfc,
	int		dir,
	int		side,
	int		*me,
	int		*G)
{
	INTERFACE	*tmp_intfc, *sav_intfc, *send_intfc;
	RECT_GRID	*gr = computational_grid(intfc);
	NODE		**n;
	BOND		*b;
	CURVE		**c;
	double		cut;
	boolean		sav_copy;

	DEBUG_ENTER(set_send_intfc)
	sav_intfc = current_interface();
	sav_copy = copy_intfc_states();
	set_size_of_intfc_state(size_of_state(intfc));
	set_copy_intfc_states(YES);
	tmp_intfc = copy_interface(intfc);
	if (tmp_intfc == NULL)
	{
	    screen("ERROR in set_send_intfc(), copy_interface() failed\n");
	    clean_up(ERROR);
	}
	cut = (side == 0) ? gr->L[dir] + (gr->L[dir] - gr->VL[dir]) :
		            gr->U[dir] + (gr->U[dir] - gr->VU[dir]);
	cut_interface(tmp_intfc,cut,dir,side,NO,YES);
	if ((rect_boundary_type(intfc,dir,side) != REFLECTION_BOUNDARY)
					&&
	    ((me[dir]==0 && side==0) || (me[dir]==(G[dir]-1) && side==1)))
	{
	    double T = gr->GU[dir] - gr->GL[dir];

	    if (side == 1)
		T = -T;
	    for (n = tmp_intfc->nodes; n && *n; ++n)
	    	Coords((*n)->posn)[dir] += T;
	    for (c = tmp_intfc->curves; c && *c; ++c)
	    	for (b = (*c)->first; b != (*c)->last; b = b->next)
	    	    Coords(b->end)[dir] += T;
	    gr = computational_grid(tmp_intfc);
	    gr->L[dir] += T;
	    gr->U[dir] += T;
	    set_rect_grid(gr->L,gr->U,gr->GL,gr->GU,gr->lbuf,gr->ubuf,
			  gr->gmax,gr->dim,&gr->Remap,gr);
	    gr = &topological_grid(tmp_intfc);
	    gr->L[dir] += T;
	    gr->U[dir] += T;
	    set_rect_grid(gr->L,gr->U,gr->GL,gr->GU,gr->lbuf,gr->ubuf,
			  gr->gmax,gr->dim,&gr->Remap,gr);
	}
	for (n = tmp_intfc->nodes; n && *n; ++n)
	    if (is_cut_node(*n))
		set_imported_node(*n);

	set_size_of_intfc_state(size_of_state(intfc));
	send_intfc = copy_interface(tmp_intfc);
	(void) delete_interface(tmp_intfc);

	set_current_interface(sav_intfc);
	set_copy_intfc_states(sav_copy);

	DEBUG_LEAVE(set_send_intfc)
	return send_intfc;
}		/*end set_send_intfc*/


/*
*		copy_interface_into():
*
*	Inserts a copy of recv_intfc into intfc.
*/

EXPORT	void	copy_interface_into(
	INTERFACE	*recv_intfc,
	INTERFACE	*intfc)
{
	INTERFACE	*sav_intfc = current_interface();
	NODE		**n, **new_nodes = NULL;
	NODE		*ns, *ne;
	CURVE		**c;
	boolean		sav_copy;
	int		i;

	DEBUG_ENTER(copy_interface_into)

	if (recv_intfc->nodes == NULL)
	{
	    DEBUG_LEAVE(copy_interface_into)
	    return; /* Nothing to do */
	}

	sav_copy = copy_intfc_states();
	set_current_interface(intfc);
	set_copy_intfc_states(YES);
	for (n = recv_intfc->nodes; n && *n; ++n)
	{
	    if (!add_to_pointers(copy_node(*n),&new_nodes))
	    {
	    	screen("ERROR in copy_interface_into(), "
	    	       "add_to_pointers() failed\n");
	    	clean_up(ERROR);
	    }
	}
	n = recv_intfc->nodes;
	for (c = recv_intfc->curves; c && *c; ++c)
	{
	    for (i = 0; n[i] != NULL; ++i)
	    {
	    	if ((*c)->start == n[i])
	    	    ns = new_nodes[i];
	    	if ((*c)->end == n[i])
	    	    ne = new_nodes[i];
	    }
	    Check_return(copy_curve(*c,ns,ne),copy_interface_into)
	}
	set_current_interface(sav_intfc);
	set_copy_intfc_states(sav_copy);

	DEBUG_LEAVE(copy_interface_into)
}		/*end copy_interface_into*/


/*
*			cut_interface():
*
*	Performs the basic operation of cutting an interface along
*	the line defined by coords[dir] = cut, and then removing
*	all objects on the interface on one side of the cut line.
*	If side = 0,  all objects with (coords[dir] > cut) are
*	removed,  which side = 1 removes all objects with (coords[dir] < cut).
*	The resulting interface satisfies the condition that all bonds on
*	the interface either lie on the appropriate side of the cut line,
*	or cross the cut line.
*
*	If the flag force_clip is YES,  then the curves that overlap the cut
*	line are trimmed so that their endpoints align with the cutting
*	line.  Otherwise the bonds will simply overlap the cut line with
*	one point on either side of the cut.
*
*	The flag save_interior is used to distinguish between the two uses
*	of this function.  One use is in clip_to_interior_region(), which is
*	saving parts of the interface interior to the computational domain.
*	The other use is in set_send_intfc() which is saving part of the
*	interior in order shift it to the exterior, in effect saving exterior
*	portions of the interface.
*
*	Bit flags are used in the boundary field of nodes created by this
*	function.  The first is CUT_NODE, which is given to all nodes created
*	here, depending on the direction.  The second is CLIP_NODE, which
*	implies that the node lies exactly on the cut line.  The third is
*	LOCAL_NODE, which is used for all nodes created by
*	clip_to_interior_region().  All CUT_NODE's created in
*	set_send_intfc() are the opposite, denoted by ~LOCAL_NODE.
*
*	For nodes lying on the virtual boundaries (gr->VL and gr->VU), there
*	is enough information to process them completely here.  The boundary
*	field set to YES, and node_type set according to the incident wave
*	types, see set_exterior_node_flags().  In other words, none of the
*	bit flags described above will apply to these nodes.
*
*	Interior nodes (i.e. not lying on VL or VU) are given the node type
*	ERROR if the original node type is not physical (i.e. some type of
*	boundary node).  This allows for the merging of physical nodes when
*	the clipping somehow retains two copies of the node.
*
*	Note: Tangles can sometimes occur across the edge of the computational
*	domain (at gr->L/U and not gr->VL/VU), resulting in the following:
*
*                   |                                 |         
*                   |			   p1 *       |       * p2
*        p1 *---------------* p2       	      \ \     |     / / 
*            \      |      /		       \  \   |   /  /  
*             \     |     /		        \   \ | /   /   
*              \    |    /		         \    *    /    
*               \   |   /		          \   |   /     
*                \  |  /		           \  |  /      
*                 \ | /			            \ | /       
*                  \|/			             \|/        
*                   X			              X         
*                  /|\			             /|\        
*                 / | \			            / | \       
*                /  |  \		           /  |  \      
*               *   |   *     		          *   |   *     
*
*	The subdomain boundary runs down the middle.  Actual curve points
*	are denoted by *'s.  These situations occur at symmetry boundaries
*	(either periodic or reflecting).  On the right, the configuration
*	at the previous time step would have resembled an inverted V, and
*	on the left, an inverted V with the corner cut off.  In both
*	cases, the curves are moving down, and the points adjacent to the
*	symmetry line cross in either direction.  In other words, p1 was
*	originally on the right, and p2 on the left of the symmetry line.
*	The difference between the two configurations is whether or not
*	there was previously a point right on the symmetry line.  This
*	configuration is really a cross node, even though only one curve
*	is involved.  It is as if the piece of the curve on one side of the
*	cut line is of the opposite wave family to the piece on the other
*	side.
*
*	These tangled configurations must be resolved.  To identify the left
*	configuration, we look for three consecutive bonds crossing the
*	cut line, with the first and third crossing each other.  On the right,
*	we look for four bonds, three of which cross the cut line, and the
*	first and fourth crossing each other.  Even if the middle point
*	(between p1 and p2) lies exactly on the cut line, one of the middle
*	bonds is not considered to cross the cut line (see 
*	bond_crosses_cut_line()).  
*
*	The resolution is then to delete p1 and p2, (and the point in
*	between if it exists), leaving a (new) point at the X and two adjacent
*	bonds meeting there.  The X is placed exactly on the cut line to help
*	preserve symmetry.
*
*	It is possible for an equivalent configuration to occur at a
*	non-symmetry boundary (not periodic or reflecting), in which case it
*	is simply a tangle occuring across the cut line.  We allow the code
*	above to operate anyway (deleting points and so on) on the assumption
*	that the configuration is small and this  will not make a large
*	difference.  The fix also may not resolve the problem for this case,
*	so that the reconstruction fails, and the code gets to back up and
*	try something else (hopefully an untangle).
*
*	Note: Consider the following.
*
*	              \          / physical curve
*	               \        /
*	                \      /
*	        -------- \    /-------- cut line
*	                  \  /
*	                   \/
*
*	It is tempting NOT to consider this as a cross if the two crossing
*	bonds are adjacent on the physical curve. The reason is that this
*	leads to splitting the curve, only to rejoin it later; seemingly
*	unecessary work.  This case must be handled, however.  On an elliptic
*	interface, the shifting may cause a configuration where the cut line
*	lies at the very base of the V.  If we do not consider this a cross,
*	the physical curve is then tangled with the boundary.  This case is
*	currently handled by splitting and never rejoining.  This leaves
*	a node on the virtual boundary for use by set_boundary().
*/

EXPORT	void	cut_interface(
	INTERFACE	*intfc,		/* interface to be cut */
	double		cut,		/* coordinate of cut line */
	int		dir,		/* direction of cut line normal */
	int		side,		/* side to be retained */
	boolean		save_interior,	/* yes if side is on interior */
	boolean		force_clip)	/* if yes clip bond at cut */
{
	CROSS		Cr, *cr;
	BOND		*b;
	CURVE		*c, **cc;
	CURVE		**curves;
	NODE		**n;
	FT_POINT		*newp;
	FT_POINT		*p;
	boolean		clip;
	double		*h = computational_grid(intfc)->h;
	double		min_sc_sep = MIN_SC_SEP(intfc);/*TOLERANCE*/
	const double	eps = MACH_EPS;/*TOLERANCE*/
	int		i, dim = intfc->dim;
	int		cr_index;
	int		num_deletes = 0;
	int		num_cr = 0;
	boolean		sav_intrp;
	static FT_POINT	*cut_p = NULL;
	static boolean	*adj_bond_cross = NULL;
	static int	cr_alloc_len = 0;
	boolean		is_reflect_side;

	/* TMP: to set clip uniformly across subdomains
         * appear to be complicated, so let's try force_clip
         * in all cases, will make permenant after sufficient
         * tests */
        force_clip = YES;

	is_reflect_side = (rect_boundary_type(intfc,dir,side) 
			== REFLECTION_BOUNDARY) ? YES: NO;

	DEBUG_ENTER(cut_interface)
	DEBUG_INTERFACE("Interface into cut_interface()",intfc);

        /* when cut interfaces for assembly and redistribution, it'd be
         * better not to alter the interface.
         * So use this tolerance, which is not grid spacing dependent.
         * The very short bond generated can be deleted by
         * calling intfc_delete_very_short_bonds() later
         * when the assembly operation is done. 
         * See the notes for set_min_sc_sep.  
         */ 
	min_sc_sep = 0.00001;

	if (cut_p == NULL)
	    cut_p = Static_point(intfc);

	if (DEBUG)
	{
	    static const char *dname[3] = { "x", "y", "z"};

	    (void) printf("Removing interface points with %s %s %g\n",
	    	          dname[dir], (side == 0) ? ">" : "<",cut);
	    (void) printf("cut = %g, dir = %d, side = %d\n",cut,dir,side);
	}

	/* Identify bonds crossing cut line */

	Cr.next = Cr.prev = NULL;
	cr = &Cr;
	for (cc = intfc->curves; cc && *cc; ++cc)
	{
	    for (b = (*cc)->first; b != NULL; b = b->next)
	    {
		/* do not cut if the point only touches the cut line */
		if (Coords(b->start)[dir] == cut && b->prev != NULL)
		{
		    if (same_sign(Coords(b->end)[dir]-cut,
			Coords(b->prev->start)[dir]-cut))
			continue;
		}
		else if (Coords(b->end)[dir] == cut && b->next != NULL)
		{
		    if (same_sign(Coords(b->start)[dir]-cut,
			Coords(b->next->end)[dir]-cut))
			continue;
		}

		if ((p = bond_crosses_cut_line(b,dir,cut,side)) == NULL)
			continue;

		/* See note at top of function. */
		if (cr && cr->prev &&
		    b->prev && b->prev->prev && b->prev->prev->prev &&
		    ((cr->b1 == b->prev) || (cr->b1 == b->prev->prev)) &&
		    (cr->prev->b1 == b->prev->prev->prev) &&
		    cross_bonds(b,b->prev->prev->prev,cut_p))
		{
		    num_deletes = 3;
		}
		else if (cr && cr->prev &&
			 b->prev && b->prev->prev &&
			 (cr->b1 == b->prev) &&
			 (cr->prev->b1 == b->prev->prev) &&
			 cross_bonds(b,b->prev->prev,cut_p))
		{
		    num_deletes = 2;
		}

		if (num_deletes != 0)
		{
		    BOND	*newb;

		    Coords(cut_p)[dir] = cut;
		    newp = Point(Coords(cut_p));
		    sav_intrp = interpolate_intfc_states(intfc);
		    interpolate_intfc_states(intfc) = YES;
		    (void) insert_point_in_bond(newp,b,*cc);
		    interpolate_intfc_states(intfc) = sav_intrp;
		    if (cr->prev)
		    {
		    	cr = cr->prev;
		    	cr->next = NULL;
		    }
		    newb = b->next;
		    for (i = 0; i < num_deletes; ++i)
			(void) delete_start_of_bond(newb->prev,*cc);
		    b = newb->prev;
		    num_deletes = 0;
		    if ((p = bond_crosses_cut_line(b,dir,cut,side)) == NULL)
			continue;
	        }
		cr->next = (CROSS *)store(sizeof(CROSS));
		++num_cr;
		cr->next->prev = cr;
		cr = cr->next;
		cr->c1 = *cc;
		cr->b1 = b;
		cr->p = p;
	    }
	}

	if (Cr.next != NULL)
	    Cr.next->prev = NULL;

	/* Check the cross list for adjacent bonds.  Closed loops are checked
	 * so that the first/last bonds are considered adjacent.  Note that
	 * this check must be done here as the bonds stored in the crosses will
	 * no longer be accurate with respect to adjacency once splitting
	 * occurs.   (adj_bond_cross[cr_index] == YES) if the i'th cross is
	 * an adjacent bond cross. */

	if (num_cr > cr_alloc_len)
	{
	    cr_alloc_len = 2*num_cr;
	    if (adj_bond_cross != NULL)
		free(adj_bond_cross);
	    uni_array(&adj_bond_cross,cr_alloc_len,sizeof(boolean));
	}

	for (cr = Cr.next, cr_index = 0; cr != NULL; cr = cr->next, ++cr_index)
	{
	    CROSS	*tmp_cr;

	    adj_bond_cross[cr_index] = NO;
	    for (tmp_cr = Cr.next; tmp_cr != NULL; tmp_cr = tmp_cr->next)
	    {
	    	if (tmp_cr == cr)
		    continue;

	    	if ((tmp_cr->b1 == cr->b1->prev) ||
		    (tmp_cr->b1 == cr->b1->next) ||
		    (   (tmp_cr->c1 == cr->c1) &&
			(node_type(cr->c1->start) == CLOSED_NODE) &&
			(
			    ((cr->b1 == cr->c1->first) &&
			       	(tmp_cr->b1 == cr->c1->last)) ||
			    ((cr->b1 == cr->c1->last)  &&
			       	(tmp_cr->b1 == cr->c1->first))
			)   
		    )
		)
	    	{
	    	    adj_bond_cross[cr_index] = YES;
	    	    break;
	    	}
	    }
	}

	/* Split curves at appropriate endpoint of crossing bonds */

	for (cr = Cr.next, cr_index = 0; cr != NULL; cr = cr->next, ++cr_index)
	{
	    boolean	endpt;

	    b = cr->b1;
	    c = cr->c1;
	    p = cr->p;

            if (DEBUG)
	    {
	    	(void) printf("full cross list at top of split loop\n");
	    	print_cross_list(Cr.next);
	    	for (i = 0; i < num_cr; ++i)
	    	    (void) printf("adj_bond_cross[%d] = %s\n",
			          i,(adj_bond_cross[i]) ? "YES" : "NO");
	    }

	    endpt = (p == b->end) ? YES : NO;
	    clip = (c->num_points <= 3) ? YES : force_clip;

	    point_on_cut_line(intfc,cut_p,b,cut,dir);

	    if (scaled_separation(b->start,cut_p,h,dim) < min_sc_sep &&
			    !is_reflect_side && b != c->first)
	    {
	    	/* If start point is near cut line, simply shift it
	    	 * onto the cut line so as not to create short bonds.
	    	 */

	    	for (i = 0; i < dim; ++i)
	    	    Coords(b->start)[i] = Coords(cut_p)[i];
	    	if (endpt)
	    	{
	    	    if (b->prev != NULL)
	    	    {
	    	        cr->b1 = b = b->prev;
	    	        cr->p = p = (endpt) ? b->end : b->start;
	    	    }
	    	    else
	    	        endpt = NO;
	    	}
	    	clip = YES;
	    }
	    else if (scaled_separation(b->end,cut_p,h,dim) < min_sc_sep &&
			    !is_reflect_side && b != c->last)
	    {
	    	/* If end point is near cut line, simply shift it
	    	 * onto the cut line so as not to create short bonds.
	    	 */

	    	for (i = 0; i < dim; ++i)
	    	    Coords(b->end)[i] = Coords(cut_p)[i];
		if (!endpt)
		{
		    if (b->next != NULL)
		    {
		    	cr->b1 = b = b->next;
		    	cr->p = p = (endpt) ? b->end : b->start;
		    }
		    else
		    	endpt = YES;
		}
		clip = YES;
	    }
	    else if (clip && (sqr_separation(p,cut_p,dim) < eps))
            {
                for (i = 0; i < dim; ++i)
                    Coords(p)[i] = Coords(cut_p)[i];
                cr->p = p;
            }
	    else if (clip)
	    {
	    	/* We only want to add a point if there is not
	    	 * already one there.  This applies mainly to
	    	 * an elliptic interface, where shifting guarantees
	    	 * the existence of a point on the cut line. */
		
	    	newp = Point(Coords(cut_p));
	    	sav_intrp = interpolate_intfc_states(intfc);
	    	interpolate_intfc_states(intfc) = YES;
	    	(void) insert_point_in_bond(newp,b,c);
	    	interpolate_intfc_states(intfc) = sav_intrp;
	    	rcl_after_insert_point(cr,newp,b);
	    	if (!endpt)
		    cr->b1 = b = b->next;
	    	cr->p = p = newp;
	    }

	    if (node_type(c->start) == CLOSED_NODE)
	    {
	    	/* The following is rather subtle.  A closed loop
	    	 * crosses twice.  The first time, the node is
	    	 * simply shifted, and its type reset.  The second
	    	 * time, this block is not entered, and a split
	    	 * is performed, yielding the two curves.
	    	 *
	    	 * Note: this block MUST come before the next two in
	    	 * case the CLOSED_NODE is positioned in the correct
	    	 * place already (which would satisfy the conditions
	    	 * for one of the two following blocks).  This case
	    	 * could also be handled by resetting the node type
	    	 * of CLOSED_NODE's in the next two blocks.  Instead,
	    	 * do all processing of CLOSED_NODE's in this block.
	    	 */

	    	if (endpt)
	    	    (void) move_closed_loop_node(c,b->next);
	    	else
	    	    (void) move_closed_loop_node(c,b);
	    	if (save_interior)
	    	{
	    	    set_interior_node_flags(c->start,clip,dir);
	    	    node_type(c->start) = ERROR;
	    	    if (adj_bond_cross[cr_index])
	    	    	set_adj_bond_cross_node(c->start);
	    	}
	    	else
	    	{
	    	    set_exterior_node_flags(c->start,c);
	    	}
	    }
	    else if ((!endpt) && (b->prev == NULL))
	    {
		/* For an elliptic interface, it is possible to shift
		 * a point onto a subdomain boundary (at U and L, NOT
		 * VU and VL).  This point then becomes interior after
		 * the communication, so it needs to be processed, and
		 * the curves joined back into one.  This is the
		 * reason for resetting SUBDOMAIN_NODE below.
		 */

		if (save_interior)
		{
		    set_interior_node_flags(c->start,clip,dir);
		    if (node_type(c->start) == SUBDOMAIN_NODE)
			node_type(c->start) = ERROR;
		    if (adj_bond_cross[cr_index])
			set_adj_bond_cross_node(c->start);
		}
		else
		{
		    set_exterior_node_flags(c->start,c);
		}
	    }
	    else if (endpt && (b->next == NULL))
	    {
	    	/* See comment for above block. */

	    	if (save_interior)
	    	{
	    	    set_interior_node_flags(c->end,clip,dir);
	    	    if (node_type(c->end) == SUBDOMAIN_NODE)
	    		node_type(c->end) = ERROR;
	    	    if (adj_bond_cross[cr_index])
	    		set_adj_bond_cross_node(c->end);
	    	}
	    	else
	    	{
	    	    set_exterior_node_flags(c->end,c);
	    	}
	    }
	    else
	    {
	    	boolean	sav_scss;

		sav_scss = interpolate_states_at_split_curve_node();
		set_interpolate_states_at_split_curve_node(NO);
		curves = split_curve(p,b,c,negative_component(c),
				     positive_component(c),
				     negative_component(c),
				     positive_component(c));
		set_interpolate_states_at_split_curve_node(sav_scss);

		if ((curves == NULL) && (!is_adj_bond_node(c->start)))
		{
		    /* The curve crosses twice, and has already been
		     * split for the first cross. */

		    break;
		}

		if (endpt)
	    	{
	    	    if (save_interior)
	    	    {
	    		set_interior_node_flags(curves[0]->end,clip,dir);
	    		node_type(curves[0]->end) = ERROR;
	    		if (adj_bond_cross[cr_index])
	    		    set_adj_bond_cross_node(curves[0]->end);
	    	    }
	    	    else
	    	    {
	    		set_exterior_node_flags(curves[0]->end,curves[0]);
	    	    }
	    	}
	    	else
	    	{
	    	    if (save_interior)
	    	    {
	    		set_interior_node_flags(curves[1]->start,clip,dir);
	    		node_type(curves[1]->start) = ERROR;
	    		if (adj_bond_cross[cr_index])
	    		    set_adj_bond_cross_node(curves[1]->start);
	    	    }
	    	    else
	    	    {
	    		set_exterior_node_flags(curves[1]->start,curves[1]);
	    	    }
	    	}
			
		/* Reset pointer lists after split_curve */
		rcl_after_split(cr,p,b,c,curves);
	    }
	}

	delete_curves_outside_of_cut_line(intfc,cut,dir,side);

	/* One more pass over the nodes is now needed.  We are checking
	 * nodes that became CUT_NODE's without a split.  They should be
	 * given node type ERROR if the following tests are satisfied.  The
	 * reason for this loop is that the last test may not be satisfied
	 * until after delete_curves_outside_of_cut_line(). */

	for (n = intfc->nodes; n && *n; ++n)
	{
	    if ((node_type(*n) != ERROR) && is_cut_node(*n) &&
	        (node_type(*n) < FIRST_PHYSICS_NODE_TYPE) &&
	        (num_curves_at_node(*n,NULL,NULL) <= 1))
	    	node_type(*n) = ERROR;
	}

	DEBUG_INTERFACE("Interface at end of cut_interface()",intfc);
	DEBUG_LEAVE(cut_interface)
}		/*end cut_interface*/

LOCAL	void	set_interior_node_flags(
	NODE *n,
	boolean clip,
	int  dir)
{
	Boundary(n) |= CUT_NODE;
	if (dir == 0)
	    Boundary(n) |= X_CUT_NODE;
	Boundary(n) |= LOCAL_NODE;
	if (clip)
	    Boundary(n) |= CLIP_NODE;
}		/*end set_interior_node_flags*/

LOCAL	void	set_exterior_node_flags(
	NODE	*n,
	CURVE	*c)
{
	set_is_bdry(n);
	clear_node_flags(n);
	if (wave_type(c) >= FIRST_PHYSICS_WAVE_TYPE)
	    node_type(n) = SUBDOMAIN_NODE;
	else if (!is_bdry(c))
	    node_type(n) = SUBDOMAIN_NODE;
	else
	    node_type(n) = FIXED_NODE;
}		/*end set_exterior_node_flags*/


/*
*			delete_curve_outside_of_cut_line():
*
*	This is a clean up function for cut_interface().  Curves which lie
*	entirely on the wrong side of the cut line are deleted.
*/

LOCAL	void	delete_curves_outside_of_cut_line(
	INTERFACE	*intfc,		/* interface to be cut */
	double		cut,		/* coordinate of cut line */
	int		dir,		/* direction of cut line normal */
	int		side)		/* side to be retained */
{
	CURVE		**cc;
	CURVE		**delete_curves;
	NODE		**nn;
	NODE		**delete_nodes;

	DEBUG_ENTER(delete_curves_outside_of_cut_line)
	delete_curves = NULL;
	for (cc = intfc->curves; cc && *cc; ++cc)
	{
	    if (curve_on_wrong_side_of_cut_line(*cc,side,dir,cut))
	    {
	    	if (!add_to_pointers(*cc,&delete_curves))
	    	{
	    	    screen("ERROR in delete_curves_outside_of_cut_line(), "
	    	           "add_to_pointers() failed\n");
	    	    clean_up(ERROR);
	    	}
	    	if (DEBUG)
	    	{
	    	    (void) printf("adding curve %llu to delete list\n",
	    			  curve_number(*cc));
	    	    print_curve(*cc);
	    	}
	    }
	}
	for (cc = delete_curves; cc && *cc; ++cc)
	    (void) delete_curve(*cc);

	delete_nodes = NULL;
	for (nn = intfc->nodes; nn && *nn; ++nn)
	{
	    if (((side == 0) && (Coords((*nn)->posn)[dir] > cut))
	        			||
	        ((side == 1) && (Coords((*nn)->posn)[dir] < cut))
	        			||
	        (((*nn)->in_curves == NULL) &&
	         ((*nn)->out_curves == NULL)))
	    {
	    	if (!add_to_pointers(*nn,&delete_nodes))
	    	{
	    	    screen("ERROR in delete_curves_outside_of_cut_line(), "
	    	           "add_to_pointers() failed\n");
	    	    clean_up(ERROR);
	    	}
	    }
	}
	for (nn = delete_nodes; nn && *nn; ++nn)
	    (void) delete_node(*nn);
	DEBUG_LEAVE(delete_curves_outside_of_cut_line)
}		/*end delete_curves_outside_of_cut_line*/


/*
*			bond_crosses_cut_line():
*
*	Determines if the given bond crosses the cut line.  Returning
*	a null pointer indicates that the bond does not cross the cut line.
*	Otherwise, a pointer to the first point on the "exterior" side of
*	the cut line (as determined by side) is returned.  If one end of the
*	bond lies on the cut line, it is said to cross only if the other
*	end lies on the side to be saved.
*/

LOCAL	FT_POINT	*bond_crosses_cut_line(
	BOND	*b,
	int	dir,
	double	cut,
	int	side)
{
	if (side == 0)
	{
	    if ((Coords(b->start)[dir] >= cut) && (Coords(b->end)[dir] < cut))
	    	return b->start;
	    if ((Coords(b->end)[dir] >= cut) && (Coords(b->start)[dir] < cut))
		return b->end;
	}
	else
	{
	    if ((Coords(b->start)[dir] <= cut) && (Coords(b->end)[dir] > cut))
	    	return b->start;
	    if ((Coords(b->end)[dir] <= cut) && (Coords(b->start)[dir] > cut))
		return b->end;
	}
	return NULL;
}		/*end bond_crosses_cut_line*/


/*
*			point_on_cut_line():
*
*	Calculates the exact point where a bond crosses a coordinate
*	line specified by cut.  This is used for reflection boundaries
*	to clip right on the boundary.
*/

LOCAL	void	point_on_cut_line(
	INTERFACE	*intfc,
	FT_POINT		*p,
	BOND		*b,	/* bond being clipped */
	double           cut,    /* coordinate of cut line */
	int             dir)    /* direction of cut line normal */
{
	double *crds_s = Coords(b->start);
	double *crds_e = Coords(b->end);
	double ps = crds_s[dir];
	double pe = crds_e[dir];
	double t;
	int   i, dim = intfc->dim;

	if (pe == ps)
	{
	    for (i = 0; i < dim; ++i)
		Coords(p)[i] = 0.5*(crds_s[i] + crds_e[i]);
	}
	else
	{
	    t = (cut - ps)/(pe - ps);
	    if (t < 0.0)
	        t = 0.0;
	    if (t > 1.0)
	        t = 1.0;
	    for (i = 0; i < dim; ++i)
	        Coords(p)[i] = (1.0 - t)*crds_s[i] + t*crds_e[i];
	}
	Coords(p)[dir] = cut;	/* this value must be exact */
}		/*end point_on_cut_line*/


/*
*			curve_on_wrong_side_of_cut_line():
*
*	Determines if all points on a curve lie on the wrong side of a
*	coordinate line specified by cut.  Wrong is determined by the
*	variable side, as in cut_interface().
*/

LOCAL int curve_on_wrong_side_of_cut_line(
	CURVE		*c,
	int		side,
	int		dir,
	double		cut)
{
	BOND		*b;

	if (side == 0)
	{
	    for (b = c->first; b != NULL; b = b->next)
	    	if (Coords(b->start)[dir] < cut)
	    	    return NO;

	    if (Coords(c->end->posn)[dir] < cut)
	    	return NO;
	}
	else
	{
	    for (b = c->first; b != NULL; b = b->next)
	    	if (Coords(b->start)[dir] > cut)
	    	    return NO;

	    if (Coords(c->end->posn)[dir] > cut)
	    	return NO;
	}
	return YES;

}		/*end curve_on_wrong_side_of_cut_line*/


/*
*			merge_interface():
*
*	This function is the logical inverse of the operation cut_interface().
*	It assumes that intfc has been formed by a combination of
*	cut_interface() followed by some number of copy_interface_into()
*	operations of similarly cut interfaces.  It seeks out the nodes
*	created by the cuts and joins the corresponding curves at the
*	corresponding bonds.  Care is taken to deal to several possible
*	degenerate cases.  
*/

EXPORT	boolean	merge_interface(
	Front		*fr,
	int		dir)
{
	INTERFACE	*intfc = fr->interf;
	OVERLAP		*ol;
	CURVE		**c;
	NODE	     	**n;
	boolean 		sav_intrp = interpolate_intfc_states(intfc);

	DEBUG_ENTER(merge_interface)
	DEBUG_INTERFACE("Interface into merge_interface()",fr->interf);

	if (intfc->curves == NULL)
	{
	    /* Nothing to do */
	    DEBUG_LEAVE(merge_interface)
	    return FUNCTION_SUCCEEDED;
	}

	if (!overlap_list(fr,&ol,dir))
	{
	    (void) printf("WARNING in merge_interface(), "
		          "overlap_list() failed\n");
	    DEBUG_LEAVE(merge_interface)
	    return FUNCTION_FAILED;
	}

	/* Process unique matches */
	for ( ; ol != NULL; ol = ol->next)
	{
	    if(!merge_overlapping_bonds(ol,fr))
	    {
	    	(void) printf("WARNING in merge_interface(), "
	                  "merge_overlapping_bonds() failed\n");
	    	DEBUG_LEAVE(merge_interface)
	    	return FUNCTION_FAILED;
	    }
	}

	DEBUG_INTERFACE("Interface after merge_overlapping_bonds()",
			fr->interf);

	if (!merge_double_physical_cut_nodes(intfc,dir))
	{
	    (void) printf("WARNING in merge_interface(), "
	                  "merge_double_physical_cut_nodes() failed\n");
	    DEBUG_LEAVE(merge_interface)
	    return FUNCTION_FAILED;
	}

	/* The problem addressed in the following block should be
	 * extremely rare, and should only occur for an elliptic
	 * (point shifted) interface.  If a point is shifted onto a
	 * subdomain corner (both coordinates lie on gr->L or gr->U),
	 * then it is impossible to tell whether the resulting
	 * CUT_NODE should be processed as an X_CUT_NODE or a
	 * Y_CUT_NODE.  The node will always be flagged as X_CUT_NODE,
	 * simply because the x-sweep (dir == 0) is always checked
	 * first.  If we find an X_CUT_NODE which lies on the
	 * subdomain in the y-direction after the x-sweep, we toggle
	 * its flag in order to give it another chance to be processed
	 * in the y-sweep.  Its matching node may be sent from an
	 * adjacent processor in the y-direction.
	 */

	if (dir == 0)
	{
	    NODE		**n;
	    RECT_GRID	*gr = computational_grid(intfc);

	    for (n = intfc->nodes; n && *n; ++n)
	    {
	    	if (is_x_cut_node(*n) && is_clip_node(*n) &&
	    	    ((Coords((*n)->posn)[1] == gr->L[1]) ||
	    	     (Coords((*n)->posn)[1] == gr->U[1])))
	    	    Boundary(*n) &= ~X_CUT_NODE;
	    }
	}
	for (c = intfc->curves; c && *c; ++c)
	{
	    if (is_closed_curve(*c))
	    {
		    /* Move the closed node off computational grid bdry */
		BOND *b;
		b = random_bond_on_curve(*c);
		while (b == (*c)->first)
		    b = random_bond_on_curve(*c);
		move_closed_loop_node(*c,b);
	    }
	}
delete_redundant_node:
        for (n = fr->interf->nodes; *n; n++)
        {
	    if (Boundary_hsb(Hyper_surf_bdry(*n))) continue;
	    if (size_of_pointers((*n)->in_curves) == 1 &&
	    	size_of_pointers((*n)->out_curves) == 1)
	    {
	    	CURVE *c1 = *(*n)->in_curves;
	    	CURVE *c2 = *(*n)->out_curves;
		if (c1 != c2 && 
		    negative_component(c1) == negative_component(c2) &&
		    positive_component(c1) == positive_component(c2))
		{
	    	    interpolate_intfc_states(intfc) = YES;
		    join_curves(c1,c2,negative_component(c1),
		    		positive_component(c2),NULL);
		    delete_node(*n);
	    	    interpolate_intfc_states(intfc) = sav_intrp;
		    goto delete_redundant_node;
		}
	    }
        }

	DEBUG_INTERFACE("Interface at end merge_interface()",fr->interf);
	DEBUG_LEAVE(merge_interface)
	return FUNCTION_SUCCEEDED;
}		/*end merge_interface*/


/*
*			overlap_list():
*
*	Sets up the doubly linked list of overlapping bonds formed by the
*	parallel communication.  
*
*	Note:  when removing duplicate overlaps, the one with
*	(or1 == POSITIVE_ORIENTATION) is deleted.  This guarantees that all
*	overlaps will have (or1 == NEGATIVE_ORIENTATION).
*/

LOCAL	boolean	overlap_list(
	Front		*fr,
	OVERLAP		**olist,
	int		dir)
{
	CURVE		**c;
	INTERFACE	*intfc = fr->interf;
	NODE		**n;
	OVERLAP		Olhead, *ol, *ol1, *ol2;
	static FT_POINT	*ptmp = NULL;

	DEBUG_ENTER(overlap_list)

	if (ptmp == NULL)
	    ptmp = Static_point(intfc);

	Olhead.next = Olhead.prev = NULL;
	ol1 = &Olhead;
	*olist = NULL;

	if (debugging("missing_intfc"))
	{
	    printf("overlap_list step 0 num_points = %d\n",
	    	NumOfInteriorPoints(fr->interf));
	}
	for (n = intfc->nodes; n && *n; ++n)
	{
	    if (DEBUG)
	    {
	    	(void) printf("Testing for overlaps, dir = %d ",dir);
	    	print_node_flags(*n);
	    }

	    if ((!is_cut_node(*n)) ||
	        (is_x_cut_node(*n) && (dir == 1)) ||
	        (!is_x_cut_node(*n) && (dir == 0)))
	    {
	        if (DEBUG)
		    (void) printf("Node skipped\n");
	    	continue;
	    }

	    for (c = (*n)->in_curves; c && *c; ++c)
	    	ol1 = new_overlap(ol1,*c,NEGATIVE_ORIENTATION,dir);
	    for (c = (*n)->out_curves; c && *c; ++c)
	    	ol1 = new_overlap(ol1,*c,POSITIVE_ORIENTATION,dir);
	}
	ol1 = Olhead.next;
	if (ol1 == NULL)
	{
	    /* There are no overlaps. */
	    DEBUG_LEAVE(overlap_list)
	    return FUNCTION_SUCCEEDED;
	}
	ol1->prev = NULL;

	if (debugging("missing_intfc"))
	{
	    printf("overlap_list step 1 num_points = %d\n",
	    	NumOfInteriorPoints(fr->interf));
	}
	if (!match_overlaps(&Olhead,dir,fr))
	{
	    (void) printf("WARNING in overlap_list(), "
	                  "match_overlaps() failed\n");
	    if (DEBUG)
	    {
	    	(void) printf("OVERLAP LIST\n");
	    	for (ol1 = Olhead.next; ol1 != NULL; ol1 = ol1->next)
	    	    print_overlap(ol1);
	    }
	    return FUNCTION_FAILED;
	}
	if (debugging("missing_intfc"))
	{
	    printf("overlap_list step 2 num_points = %d\n",
	    	NumOfInteriorPoints(fr->interf));
	}

	/* Drop duplicates */
	for (ol1 = Olhead.next; ol1 != NULL; ol1 = ol1->next)
	{
	    ol2 = ol1->match;
	    if (ol2 != ol1)
		remove_from_overlap_list(ol2,&Olhead);
	    if (ol1->or1 == POSITIVE_ORIENTATION)
	    {
	    	ol2->prev = ol1->prev;
	    	ol2->next = ol1->next;
	    	if (ol1->prev != NULL)
	    	    ol1->prev->next = ol2;
	    	else
	    	    Olhead.next = ol2;
	    	if (ol1->next != NULL)
	    	    ol1->next->prev = ol2;
	    	ol1 = ol2;
	    }
	    ol1->match = ol1;
	}
	if (debugging("missing_intfc"))
	{
	    printf("overlap_list step 3 num_points = %d\n",
	    	NumOfInteriorPoints(fr->interf));
	}

	if (!delete_redundant_overlaps(&Olhead,intfc,dir))
	{
	    if (DEBUG)
	    {
	    	(void) printf("WARNING in overlap_list(), "
	    	              "delete_redundant_overlaps() failed.\n");
	    }
	    return FUNCTION_FAILED;
	}  
	if (debugging("missing_intfc"))
	{
	    printf("overlap_list step 4 num_points = %d\n",
	    	NumOfInteriorPoints(fr->interf));
	}


	/* This is intended to combat a degeneracy caused by the redistribute,
	 * which does not behave consistently across subdomain boundaries.
	 * The current merging algorithm clips each curve back at least to
	 * the cut line.  For non-clip nodes, this means lopping off the end
	 * bond of each curve.  It is possible that the redistribute algorithm
	 * inserted points such that the end point of one of the new end bonds
	 * lies ON the other bond.  In other words, they intersect, and the
	 * merge will result in a fold-back.  Such cases should be EXTREMELY
	 * rare, but must still be handled.  The fix here is to detect the
	 * intersection, and then move back on one curve by an additional
	 * bond.  Another possibility would be to call delete_fold_back_bonds()
	 * at the end of the scatter, but the code below should be much more
	 * efficient.
	 */

	for (ol = Olhead.next; ol != NULL; ol = ol->next) 
	{
	    if ((ol->b1->prev == NULL) || (ol->b2->next == NULL))
		    continue;

	    if (!is_clip_node(ol->n1))
	    {
		if (cross_bonds(ol->b1->prev,ol->b2->next,ptmp))
		{
		    if (DEBUG)
		    {
			(void) printf("detected fold back in scatter, "
			              "resetting ol->b1\n");
			(void) printf("c1: ");
			print_curve(ol->c1);
			(void) printf("c2: ");
			print_curve(ol->c2);
			(void) printf("b1: ");
			print_bond(ol->b1);
			(void) printf("b1->prev: ");
			print_bond(ol->b1->prev);
			(void) printf("b2: ");
			print_bond(ol->b2);
			(void) printf("ol->b2->next: ");
			print_bond(ol->b2->next);
		    }
		    ol->b1 = ol->b1->prev;
	        }
	    }
	    else if (!is_clip_node(ol->n2))
	    {
		if (cross_bonds(ol->b1->prev,ol->b2->next,ptmp))
		{
		    if (DEBUG)
		    {
			(void) printf("detected fold back in scatter, "
			              "resetting ol->b2\n");
			(void) printf("c1: ");
			print_curve(ol->c1);
			(void) printf("c2: ");
			print_curve(ol->c2);
			(void) printf("b1: ");
			print_bond(ol->b1);
			(void) printf("b1->prev: ");
			print_bond(ol->b1->prev);
			(void) printf("b2: ");
			print_bond(ol->b2);
			(void) printf("ol->b2->next: ");
			print_bond(ol->b2->next);
		    }
		    ol->b2 = ol->b2->next;
	        }
	    }
	}
	if (debugging("missing_intfc"))
	{
	    printf("overlap_list step 5 num_points = %d\n",
	    	NumOfInteriorPoints(fr->interf));
	}

	/* Some nodes have retained their physical node_type to indicate
	 * processing in merge_double_physical_cut_nodes().  If such a node
	 * appears in multiple overlaps, however, this is incorrect, and the
	 * node_type needs to be reset.  The behavior for each of the cases
	 * is rather subtle, see documentation for
	 * merge_double_physical_cut_nodes() and merge_overlapping_bonds().
	 */

	for (n = intfc->nodes; n && *n; ++n)
	{
	    if (node_type(*n) == ERROR)
	        continue;

	    ol2 = NULL;
	    for (ol1 = Olhead.next; ol1 != NULL; ol1 = ol1->next)
	    {
	    	if (((*n) == ol1->n1) || ((*n) == ol1->n2))
	    	{
	    	    if (ol2 == NULL)
	    		ol2 = ol1;
	    	    else
	    	    {
	    	    	node_type(*n) = ERROR;
	    	    	break;
	    	    }
	    	}
	    }
	}
	if (debugging("missing_intfc"))
	{
	    printf("overlap_list step 6 num_points = %d\n",
	    	NumOfInteriorPoints(fr->interf));
	}

	/* Merge components of matched curves
	 * Note: this code is currently unable to handle the following case.
	 *
	 *	    comp1                     comp3
	 *	-------------------*    *----------------
	 *	    comp2                 \   comp4
	 *	                            \
	 *	                              \
	 * 	                                \
	 *	                          comp5   \
	 *
	 * The code is currently not smart enough to figure out that it is
	 * really (comp1 and comp3) and (comp2 and comp5) that should be
	 * compared and possibly merged.  For now, the answer is to do
	 * nothing in this case. */

	for (ol = Olhead.next; ol != NULL; ol = ol->next)
	{
	    if ((num_curves_at_node(ol->n1,NULL,NULL) > 1) ||
	        (num_curves_at_node(ol->n2,NULL,NULL) > 1))
	    	continue;

	    if (positive_component(ol->c1) != positive_component(ol->c2))
	    	merge_components(positive_component(ol->c1),
				 positive_component(ol->c2),fr->interf);
	    if (negative_component(ol->c1) != negative_component(ol->c2))
	    	merge_components(negative_component(ol->c1),
				 negative_component(ol->c2),fr->interf);
	}
	if (debugging("missing_intfc"))
	{
	    printf("overlap_list step 7 num_points = %d\n",
	    	NumOfInteriorPoints(fr->interf));
	}

	if (DEBUG)
	{
	    (void) printf("OVERLAP LIST\n");
	    for (ol1 = Olhead.next; ol1 != NULL; ol1 = ol1->next)
	    	print_overlap(ol1);
	}

	*olist = Olhead.next;
	DEBUG_LEAVE(overlap_list)
	return FUNCTION_SUCCEEDED;
}		/*end overlap_list*/


/*
*			match_overlaps():
*
*	This function is responsible for matching up overlaps which will
*	then be merged.  A number of tests are used to exclude invalid
*	matches.
*
*	To be successful, all overlaps must be matched, and all matches must
*	be consistent (i.e. (A matches B) and (B matches A)).  Note that this
*	does not exclude the possibility of a given node appearing in multiple
*	overlaps.  
*/

LOCAL	boolean match_overlaps(
	OVERLAP		*olhead,
	int		dir,
	Front		*fr)
{
	OVERLAP		*ol1, *ol2;
	double		dist;

	DEBUG_ENTER(match_overlaps)

	if (debugging("missing_intfc"))
	{
	    printf("match_overlap step 0 num_points = %d\n",
	    	NumOfInteriorPoints(fr->interf));
	}
	/* Identify matching bonds */
	for (ol1 = olhead->next; ol1 != NULL; ol1 = ol1->next)
	{
	    if (DEBUG)
	    {
		(void) printf("Searching for overlap match for overlap ");
	        print_overlap(ol1);
	    }
	    for (ol2 = olhead->next; ol2 != NULL; ol2 = ol2->next)
	    {
		/* It would be possible to add more topological or geometric
		 * tests to eliminate invalid matches. */

		    /* an overlap can't match with itself */
		if (ol1 == ol2)
		    continue;

	        if (DEBUG)
	        {
		    (void) printf("Testing for match with overlap ");
	            print_overlap(ol2);
	        }
		
		/* matching curves cannot have the same orientations */
		if (ol1->or1 == ol2->or1)
		{
	            if (DEBUG)
			(void) printf("orientations or1 agree, no match\n");
		    continue;
		}

		/* two local nodes cannot match */
		if (is_local_node(ol1->n1) && is_local_node(ol2->n1))
		{
	            if (DEBUG)
			(void) printf("both n1's are local, no match\n");
		    continue;
		}

		/* two non-local nodes cannot match */
		if (!is_local_node(ol1->n1) && !is_local_node(ol2->n1))
		{
	            if (DEBUG)
			(void) printf("both n1's are not local, no match\n");
		    continue;
		}

                /* matching overlaps must be on the same side of the domain */
                /* 050703. Comment: probably we need to do some more work
                 * to make sure that two matching nodes are really on the same
                 * side of the domain. This "> 0.0" is really not a good judge,
                 * because of the machine round-off error.
                 */
                if (fabs(Coords(ol1->cr1)[dir] - Coords(ol2->cr1)[dir]) > 0.0)
		{
	            if (DEBUG)
		    {
		        (void) printf("fabs(Coords(ol1->cr1)[dir] - "
				      "Coords(ol2->cr1)[dir]) = %g > 0.0 "
				      "no match\n",
				      fabs(Coords(ol1->cr1)[dir] -
					   Coords(ol2->cr1)[dir]));
		    }
		    continue;
		}

		/* the curves must pass a correspondence test */
		if (!correspondence_is_possible(ol1->c1,ol2->c1,
						   NULL,NULL,fr))
		{
	            if (DEBUG)
		    {
			(void) printf("correspondence is not possible, "
				      "no match\n");
		    }
		    continue;
		}

		dist = separation(ol1->cr1,ol2->cr1,2);

		if (DEBUG)
		{
		    (void) printf("node %llu and node %llu, dist = %g\n",
				  node_number(ol1->n1),
				  node_number(ol2->n1),dist);
	        }
		if (dist < ol1->dist)
		{
		    ol1->match = ol2;
		    ol1->c2 = ol2->c1;
		    ol1->b2 = ol2->b1;
		    ol1->n2 = ol2->n1;
		    ol1->or2 = ol2->or1;
		    ol2->cr2 = ol2->cr1;
		    ol1->dist = dist;
	        }
	    }
	}
	if (debugging("missing_intfc"))
	{
	    printf("match_overlap step 1 num_points = %d\n",
	    	NumOfInteriorPoints(fr->interf));
	}

	/* Run a final consistency check, and handle the degenerate cases. */

	for (ol1 = olhead->next; ol1 != NULL; ol1 = ol1->next)
	{
	    if (DEBUG)
	    {
		(void) printf("Final consistency check for overlap ");
	        print_overlap(ol1);
	    }
	    /* Check for a consistent match. */
	    if ((ol1->match != NULL) && (ol1->match->match == ol1))
	    {
	        if (DEBUG)
		    (void) printf("Match is consistent\n");
	    	continue;
	    }

	    if ((ol2=find_single_adj_bond_match(ol1,olhead,dir,fr))!=NULL)
	    {
	        if (DEBUG)
		    (void) printf("Single adjacent bond match\n");
	    	ol1->match = ol2;	ol2->match = ol1;
	    	ol1->c2 = ol2->c1;	ol2->c2 = ol1->c1;
	    	ol1->b2 = ol2->b1;	ol2->b2 = ol1->b1;
	    	ol1->n2 = ol2->n1;	ol2->n2 = ol1->n1;
	    	ol1->or2 = ol2->or1;	ol2->or2 = ol1->or1;
	    	ol1->cr2 = ol2->cr1;	ol2->cr2 = ol1->cr1;
	    	ol1->dist = 0.0;	ol2->dist = 0.0;
	    }
	    else if ((ol2=find_double_adj_bond_match(ol1,olhead,dir,fr))!=NULL)
	    {
	        if (DEBUG)
		    (void) printf("Double adjacent bond match\n");

		average_points(NO,ol1->n1->posn,Hyper_surf_element(ol1->b1),
			Hyper_surf(ol1->c1),ol2->n1->posn,
			Hyper_surf_element(ol2->b1),Hyper_surf(ol2->c1));

	    	change_node_of_curve(ol2->c1,ol2->or1,ol1->n1);

	    	/* Duplicates have not been dropped yet, so must
	    	 * check orientations. */

	    	if (ol2->or1 == NEGATIVE_ORIENTATION)
	    	{
		    /* TO Remove: This produces zero length bond
	    	    (void) insert_point_in_bond(ol2->n1->posn,
	    				        ol2->c1->last,ol2->c1);
		    */
	    	    ol1->b2 = ol2->b1 = ol2->c1->last;
	    	    ol2->b2 = ol1->c1->first;
	    	}
	    	else
	    	{
		    /* TO Remove: This produces zero length bond
	    	    (void) insert_point_in_bond(ol2->n1->posn,
	    				        ol2->c1->first,ol2->c1);
		    */
	    	    ol1->b2 = ol2->b1 = ol2->c1->first;
	    	    ol2->b2 = ol1->c1->last;
	    	}
	    	(void) delete_node(ol2->n1);
	    	ol1->match = ol2;	ol2->match = ol1;
	    	ol1->n2 = ol2->n1 = ol2->n2 =  ol1->n1;
	    	ol1->c2 = ol2->c1;	ol2->c2 = ol1->c1;
	    	ol1->or2 = ol2->or1;	ol2->or2 = ol1->or1;
	    	ol1->cr2 = ol2->cr1;	ol2->cr2 = ol1->cr2;
	    	ol1->dist = 0.0;	ol2->dist = 0.0;
	    }
	    else if ((ol2 = find_remnant_match(ol1,olhead,dir)) != NULL)
	    {
		if (debugging("missing_intfc"))
		{
	    	    printf("match_overlap step 11 num_points = %d\n",
	    	    NumOfInteriorPoints(fr->interf));
		    printf("ol1->c1 = %p\n",ol1->c1);
		    print_curve(ol1->c1);
		}
	        if (DEBUG)
		    (void) printf("Remnant match\n");
	    	if (!delete_curve(ol1->c1))
		    return FUNCTION_FAILED;
		if (debugging("missing_intfc"))
		{
	    	    printf("match_overlap step 12 num_points = %d\n",
	    	    NumOfInteriorPoints(fr->interf));
		}
	    	(void) delete_node(ol1->n1);
	    	(void) delete_node(ol2->n1);

	    	remove_from_overlap_list(ol2,olhead);
	    	remove_from_overlap_list(ol1,olhead);
	    }
	    else if (is_half_remnant_degeneracy(ol1))
	    {
	        if (DEBUG)
		    (void) printf("Half remnant degeneracy\n");
	    	if (!delete_curve(ol1->c1))
		    return FUNCTION_FAILED;
	    	(void) delete_node(ol1->c1->start);
	    	(void) delete_node(ol1->c1->end);

	    	remove_from_overlap_list(ol1,olhead);
	    }
	    else
	    {
	    	/* This is needed for special cases when a CUT_NODE
	    	 * has multiple incident curves.  It may not be
	    	 * possible to consistently match all the overlaps in
	    	 * which the node appears.  This case should be handled
	    	 * properly when the merges are performed.  If not,
	    	 * the scatter will fail due to an unprocessed cut
	    	 * node at the end.
	    	 */

	    	if (DEBUG)
		{
	    	  (void) printf("removing overlap for inconsistency\n");
		}
	    	remove_from_overlap_list(ol1,olhead);
	    }
	}

	DEBUG_LEAVE(match_overlaps)
	return FUNCTION_SUCCEEDED;
}		/*end match_overlaps*/


/*
*			find_single_adj_bond_match():
*
*	There is a degenerate case handled here.
*	Consider the following:
*
*                           cut line         cut line
*			       | *              |       *
*			       |/               |      /
*			      /|                |     /
*			     / |                |    /
*			    /  |                |   /   physical
*		physical   /   |                |  /    curve
*		curve	  *    |                | *
*			   \   |                |  \
*			    \  |                |   \
*			     \ |                |    \
*			      \|                |     \
*			       |\               |      \
*			       | *              |       *
*
*	The interior is between the two vertical lines in the picture (the
*	periodic case).  It is assumed that the code behaves well enough that
*	only two bonds are involved.  cut_interface() produces a single
*	ADJ_BOND_NODE at the exterior point on the left, and throws out the
*	physical curve on the right (at least that part of it which is shown).
*	The interpretation is that the physical curve is moving to the left,
*	and just crossed the boundary at the left boundary.  Since the
*	interior is between the lines, the left boundary should have the more
*	accurate information.  This case is identified below, and the
*	resolution is to match the ADJ_BOND_NODE with itself, simply
*	rejoining the physical curve.
*/

LOCAL OVERLAP *find_single_adj_bond_match(
	OVERLAP		*ol1,
	OVERLAP		*olhead,
	int		dir,
	Front		*fr)
{
	OVERLAP		*single_match = NULL;
	OVERLAP		*ol2;

	for (ol2 = olhead->next; ol2 != NULL; ol2 = ol2->next)
	{
	    if (ol1 == ol2)
	        continue;

	    if ((ol2->match != NULL) && (ol2->match->match == ol2))
	        continue;

	    if (ol1->n1 != ol2->n1)
	        continue;

	    if (ol1->or1 == ol2->or1)
	        continue;

	    if (local_num_curves_at_node(ol1->n1) != 2)
	        continue;

	    if (!is_adj_bond_node(ol1->n1))
	        continue;

	    if (!correspondence_is_possible(ol1->c1,ol2->c1,NULL,NULL,fr))
	        continue;

	    if ((negative_component(ol1->c1) != negative_component(ol2->c1))
	    				||
	        (positive_component(ol1->c1) != positive_component(ol2->c1)))
		continue;

	    if (fabs(Coords((ol1)->cr1)[dir] - Coords((ol2)->cr1)[dir]) != 0.0)
	        continue;

	    single_match = ol2;
	    break;
	}

	if (DEBUG && (single_match != NULL))
	{
	    (void) printf("found single_adj_bond_match\n");
	    (void) printf("ol1 == ");	    print_overlap(ol1);
	    (void) printf("ol2 == ");	    print_overlap(single_match);
	}
	return single_match;
}		/*end find_single_adj_bond_match*/


/*
*			find_double_adj_bond_match():
*
*	This function handles a degenerate case.  Consider the following:
*
*	         --------*-------------*-------  cut line
*	                /               \
*	               /                 \
*	              /                   \
*	             /   physical curve    \
*
*	We have a curve that got clipped off at the cut line, and then a
*	triangular piece was thrown out (above the cut line).  Unfortunately,
*	the adjacent processor did not supply anything for the two cut nodes
*	to match with.  The most likely cause for this is the point shifting
*	on an elliptic interface, where the small triangle looks isolated
*	on the upper processor, and is thus deleted.  The resolution is simply
*	to join the physical curve back together along the cut line.
*/

LOCAL OVERLAP *find_double_adj_bond_match(
	OVERLAP		*ol1,
	OVERLAP		*olhead,
	int		dir,
	Front		*fr)
{
	OVERLAP		*double_match = NULL;
	OVERLAP		*ol2;
	double		min_dist = HUGE;
	double		tmp_dist;
	double		*h = computational_grid(fr->interf)->h;
	double		perim_tol = 2.0*(h[0] + h[1]);/*TOLERANCE*/

	for (ol2 = olhead->next; ol2 != NULL; ol2 = ol2->next)
	{
	    if (ol1 == ol2)
	        continue;

	    if ((ol2->match != NULL) && (ol2->match->match == ol2))
	        continue;

	    if (ol1->n1 == ol2->n1)
	        continue;

	    if (ol1->or1 == ol2->or1)
	        continue;

	    if ((local_num_curves_at_node(ol1->n1) != 1) ||
		(local_num_curves_at_node(ol2->n1) != 1))
	        continue;

	    if (!correspondence_is_possible(ol1->c1,ol2->c1,NULL,NULL,fr))
	        continue;

	    if ((negative_component(ol1->c1) != negative_component(ol2->c1))
					||
		(positive_component(ol1->c1) != positive_component(ol2->c1)))
	        continue;

	    if (ol1->c1 == ol2->c1)
	    {
	        if (ol1->c1->num_points < 5)
	    	    continue;
	        if (curve_length(ol1->c1) < perim_tol)
	    	    continue;
	    }
	    else
	    {
	        if (ol1->c1->num_points + ol2->c1->num_points < 5)
	    	    continue;
	        if (curve_length(ol1->c1) + curve_length(ol2->c1) < perim_tol)
		    continue;
	    }

/*     Before use amr_overture, != 0.0 arg. works fine, however, overture found
 *     the difference of two cuts is not strictly zero because of the machine,
 *     so the tolerance is used !!!!
 */
            if (fabs(Coords(ol1->cr1)[dir] - Coords(ol2->cr1)[dir]) != 0.0)
                continue;

	    tmp_dist = separation(ol1->cr1,ol2->cr1,2);
	    if (tmp_dist < min_dist)
	    {
	        min_dist = tmp_dist;
		if (min_dist < h[dir])
	            double_match = ol2;
	    }
	}
	if (DEBUG && (double_match != NULL))
	{
	    (void) printf("found double_adj_bond_match\n");
	    (void) printf("ol1 == ");	print_overlap(ol1);
	    (void) printf("ol2 == ");	print_overlap(double_match);
	}

	return double_match;
}		/*end find_double_adj_bond_match*/


/*
*			find_remnant_match():
*
*	There is a degenerate case handled here.
*	Consider the following:
*
*                           cut line         cut line
*			       | *              |       *
*			       |/               |      /
*			      /|                |     /
*			     / |                |    /
*			    /  |                |   /   physical
*		physical   /   |                |  /    curve
*		curve	  *    |                | *
*			   \   |                |  \
*			    \  |                |   \
*			     \ |                |    \
*			      \|                |     \
*			       |\               |      \
*			       | *              |       *
*
* 	The two cut lines represent either corresponding periodic boundaries
*	on either side of the domain, or corresponding parallel boundaries
*	on adjacent processors.  What has happened is that the propagation
*	proceeded slightly differently at the two boundaries, resulting in
*	the physical curve crossing in one place and not the other.  This
*	can occur in a number of ways.  The simplest would be a simple
*	discrepancy in the propagation algorithms, so that the point at the
*	tip moved further on one processor than the other.  This seems to be
*	rather rare.  Much more common is a discrepancy caused by the
*	redsitribution.  Also, it is possible to delete curves on one
*	processor (e.g. small loops), and not on the other.
*
*	The interior is outside the two vertical lines in the picture
*	(the parallel case).  It is assumed that the code behaves well
*	enough that only two bonds are involved.  cut_interface() flags
*	two ADJ_BOND_NODE's at the top and bottom points on the left, and
*	throws out the physical curve on the right (at least the part shown).
*	The ADJ_BOND_NODE's have nothing to match with.  One interpretation
*	is that physical curve is moving to the right, and a small piece got
*	stranded at the boundary shown on the left.  Since the interior is
*	outside the lines, the right picture should have the more accurate
*	information, so the resolution is to delete the remnant curve.
*
*	Note:  an ugly mess can occur if the physical curve zig zags back and
*	forth across the cut line, resulting in ADJ_BOND_NODE's with multiple
*	incidents.  Such nodes will appear in multiple overlaps. In one
*	overlap, a consistent match is achieved.  In the other, the degenerate
*	case is detected, and the curve is deleted.  It is for this special
*	sub-case that we allow delete_node() to fail when processing the
*	remnant.
*/

LOCAL OVERLAP *find_remnant_match(
	OVERLAP		*ol1,
	OVERLAP		*olhead,
	int		dir)
{
	OVERLAP		*remnant_match = NULL;
	OVERLAP		*ol2;

	for (ol2 = olhead->next; ol2 != NULL; ol2 = ol2->next)
	{
	    if (ol1 == ol2)
		continue;

	    if ((ol2->match != NULL) && (ol2->match->match == ol2))
		continue;

	    if (ol1->c1 != ol2->c1)
		continue;

	    /* Too risky to delete curve with more than 3 bonds */
	    if (ol1->c1->num_points > 4)
	    	continue;

	    if ((local_num_curves_at_node(ol1->c1->start) != 1) &&
		(!((num_curves_at_node(ol1->c1->start,NULL,NULL) == 2) &&
		      is_adj_bond_node(ol1->c1->start)))
					||
		((local_num_curves_at_node(ol1->c1->end) != 1) &&
		 (!((num_curves_at_node(ol1->c1->end,NULL,NULL) == 2) &&
		       is_adj_bond_node(ol1->c1->end)))))
		continue;

	    if (fabs(Coords(ol1->cr1)[dir] - Coords(ol2->cr1)[dir]) != 0.0)
		continue;

	    if (!(((ol1->c1->num_points == 3) &&
		      is_adj_bond_node(ol1->c1->start) &&
		      is_adj_bond_node(ol1->c1->end))
	  			||
		     (is_cut_node(ol1->c1->start) &&
		      is_cut_node(ol1->c1->end) &&
		      (node_type(ol1->c1->start) == ERROR) &&
		      (node_type(ol1->c1->end) == ERROR))))
		continue;

	    remnant_match = ol2;
	    break;
	}
	if (DEBUG && (remnant_match != NULL))
	{
	    (void) printf("found remnant match\n");
	    (void) printf("ol1 == ");	print_overlap(ol1);
	    (void) printf("ol2 == ");	print_overlap(ol2);
	}
	return remnant_match;
}		/*end find_remnant_match*/


/*
*			is_half_remnant_degeneracy():
*
*	This special case occurs at subdomain corners, i.e. the confluence
*	of multiple cut lines.  It does not matter which side of the 
*	horizontal cut line is interior.
*
*	  virtual (exterior) cut line    interior cut line
*	  |                              |
*	  |                              |
*	n1*                              |
*	  |\                             |
*	  | \ c1 (physical)              |
*	  |  \                           |
*	  |---\--------------------------|----------    interior cut line
*	  |    \                         |
*	n2*     \                        |
*	  |\     \                       |
*	  |  \    \                      |
*	  |    \   \                     |
*	  |      \  \                    |
*	  |        \ \                   |
*	  |          \*  n3              |
*	  |            \                 |
*	  |              \ c2 (physical) |
*	  .               .              .
*	  .                .             .
*	  .                 .            .
*	  |                              |
*	  |_________________________________________    virtual (exterior) cut
*
*	When this degeneracy is handled, the virtual boundary does not exist.
*	It is shown in the picture for visualization.  Also, since it is an
*	exterior boundary, this entire problem occurs IN the subdomain.  There
*	is another vertical cut line to the right which is not shown.  The
*	resolution is not really important, as the configuration will be thrown
*	out during the next scatter, but it must be handled to allow the
*	current scatter to complete successfully.  The picture is interpreted
*	as follows.  Some event (probably redistribute()), has caused an
*	inconsistency between the processors above and below, so that at the
*	virtual cut, the physical curve crosses the interior cut line on
*	the upper processor, but not on the lower.  n3 is the only CUT_NODE,
*	c2 continues out of the picture.
*	and c1 is assumed to be only a single bond.  
*
*	This is very similar to the remnants in find_remnant_match().  c1
*	can be thought of as a remnant that has been chopped in half at the
*	virtual boundary.  Since there is only one CUT_NODE, there is only one
*	inconsistent overlap, instead of a pair, which is why this function
*	returns only a boolean.
*
*	The resolution is to delete c1, n1, and n3.  It would also be possible
*	to try to join c1 and c2, but this is much more complicated and totally
*	unnecessary.
*
*	Note: the condition that c1 is a single bond may be too optimistic,
*	and may have to be relaxed.  Also, it may be desirable to require
*	below that the non-cut node lie on the virtual boundary.
*/

LOCAL boolean is_half_remnant_degeneracy(
	OVERLAP		*ol)
{

	if ((ol->c1->num_points == 2) &&
	    (num_curves_at_node(ol->c1->start,NULL,NULL) == 1) &&
	    (num_curves_at_node(ol->c1->end,  NULL,NULL) == 1) &&
	    ((is_cut_node(ol->c1->start) &&
	      is_bdry(ol->c1->end)   && (!is_cut_node(ol->c1->end)))
	     			||
	     (is_cut_node(ol->c1->end) &&
	      is_bdry(ol->c1->start) && (!is_cut_node(ol->c1->start)))))
	{
	    if (DEBUG)
	    {
	    	(void) printf("found half remnant degeneracy\n");
	    	(void) printf("ol == ");	print_overlap(ol);
	    }
	    return YES;
	}
	else
	    return NO;	     

}		/*end is_half_remnant_degeneracy*/



/*
*			delete_redundant_overlaps():
*
*	When a curve crosses back and forth across a subdomain boundary, it
*	is possible to retain redundant pieces of curve from adjacent
*	processors, which can then produce tangles in the scattered interface.
*	One way to think of this is as redundant paths between two cross
*	points on a cut line.  One example of this is related to the single
*	adj bond matches above.  If the relevant pieces of curve ARE saved on
*	both processors, a perfectly valid matching can be found using two
*	pairs of bonds (see picture above).  It is desirable, however, to use
*	a single adj bond match and throw out the short curve.  This is usually
*	due to the redistribute, which results in configurations in which the
*	overlapping curves are not coincident.  This can result in tangles
*	and/or fold-backs in the merged interface.
*
*	This function is responsible for finding these curves and deleting
*	any redundancies such that the interface can be properly reconstructed.
*	The first step is to install any possible single adj bond matches which
*	are not already in the overlap list.  The REDUNDANCY structure is then
*	used to pair overlaps representing redundant matching of individual
*	curves.
*
*	Two cases can occur.  The first below, termed a double curve redundancy,
*	is admittedly rare, and complicated enough that I won't even try to
*	diagram it.  This is the more complex of the two cases, and represents
*	a configuration in which a curve zigs back and forth across a cut line
*	in the process of crossing from one side to the other.  It is rather
*	arbitrary which of the two redundant paths to delete, so we provide
*	a test which takes the min of the start coords in order to get 
*	consistent behavior on the adjacent processors.
*
*	The other case is much simpler, and is depicted in the figure for
*	find_single_adj_bond_match().  In this case, we have two possibilities:
*	a single adj bond match or two matches at either end of a short curve.
*	We prefer to use the adj bond match and throw out the short curve, but
*	this is not always possible.  This is termed a single curve redundancy
*	due to the fact that there is only one possible redundant curve to be
*	deleted.
*
*	The redundancies are ordered using the coordinate in the dir direction
*	of the adj bond node at the start/end of the redundant curve of the
*	structure.  This ordering is intended to force adjacent processors to
*	behave in the same way, overcoming any dependence on the ordering of
*	the curve list.  The sorting is admittedly inefficient, but the
*	redundancy lists should always be very short.
*/

LOCAL boolean delete_redundant_overlaps(
	OVERLAP		*olhead,
	INTERFACE	*intfc,
	int		dir)
{
	REDUNDANCY	Redhead1, Redhead2;
	REDUNDANCY	*red, *red1, *red2;
	CURVE		*red_curve;
	OVERLAP		*ol, *ol1, *ol2;
	OVERLAP		*red_ol;
	OVERLAP		Redolaphead, *redolap;
	NODE		**n;
	int		offdir = (dir+1)%2;

#define remove_from_redundancy_list(rd)				\
{								\
	rd->prev->next = rd->next;				\
	if (rd->next != NULL)					\
	    rd->next->prev = rd->prev;				\
	if (DEBUG)						\
	    (void) printf("removed red %p\n",(POINTER)rd);	\
}

	DEBUG_ENTER(delete_redundant_overlaps)
	Redhead1.next = Redhead1.prev = NULL;
	Redhead2.next = Redhead2.prev = NULL;
	Redolaphead.next = Redolaphead.prev = NULL;
	redolap = &Redolaphead;

	/* Add any single adj bond matches, if not already in ol list. */

	for (n = intfc->nodes; n && *n; ++n)
	{
	    if (!(is_adj_bond_node(*n)))
	    	continue;
	    if ((    is_x_cut_node(*n) && (dir == 1)) ||
	        (!is_x_cut_node(*n) && (dir == 0)))
	    	continue;

	    if (local_num_curves_at_node(*n) != 2)
	    	continue;

	    for (ol = olhead->next; ol != NULL; ol = ol->next)
	    {
	    	if ((ol->n1 == *n) && (ol->n2 == *n))
	    	    break;
	    }
	    if (ol == NULL)
	    {
	    	redolap = new_overlap(redolap,(*n)->in_curves[0],
	    			      NEGATIVE_ORIENTATION,dir);
	    	redolap->n2 = (*n);
	    	redolap->c2 = (*n)->out_curves[0];
	    	redolap->b2 = ((*n)->out_curves[0])->first;
	    	redolap->or2 = POSITIVE_ORIENTATION;
	    	redolap->match = redolap;
	    	redolap->dist = 0.0;
	    }
	}

	if (Redolaphead.next == NULL)
	    return FUNCTION_SUCCEEDED; /* No redundancies */

	if (DEBUG)
	{
	    (void) printf("red olap list: \n");
	    for (ol1 = Redolaphead.next; ol1 != NULL; ol1 = ol1->next)
	    	print_overlap(ol1);
	}

	/* Build redundancy lists.  red1 contains overlaps whose first curves
	 * match, and red2 contains overlaps whose second curves match. */

	for (ol1 = Redolaphead.next; ol1 != NULL; ol1 = ol1->next)
	{
	    for (ol2 = olhead->next; ol2 != NULL; ol2 = ol2->next)
	    {
	        if (ol1->c1 == ol2->c1)
	        {
	            if (DEBUG)
	            {
	        	(void) printf("found c1 redundancy, ol1 %d, ol2 %d\n",
	        		      ol1->index,ol2->index);
	            }

	            red = (REDUNDANCY *)store(sizeof(REDUNDANCY));
	            red->ol1 = ol1;
	            red->ol2 = ol2;
	            red->metric = Coords(ol1->c1->end->posn)[offdir];
	            if (Redhead1.next == NULL)
	            {
	        	Redhead1.next = red;
	        	red->prev = &Redhead1;
	            }
	            else
	            {
	        	for (red1=Redhead1.next; red1 != NULL; red1=red1->next)
	        	{
	        	    if (red->metric < red1->metric)
	        	    {
	        		red->next = red1;
	        		red->prev = red1->prev;
	        		red1->prev->next = red;
	        		red1->prev = red;
	        		break;
	        	    }
	        	    if (red1->next == NULL)
	        	    {
	        		/* Add to end of list */
	        		red->prev = red1;
	        		red1->next = red;
	        		break;
	        	    }
	                }
	            }
	        }
	        if (ol1->c2 == ol2->c2)
	        {
	            if (DEBUG)
	            {
	        	(void) printf("found c2 redundancy, ol1 %d, ol2 %d\n",
	        		      ol1->index,ol2->index);
	            }

	            red = (REDUNDANCY *)store(sizeof(REDUNDANCY));
	            red->ol1 = ol1;
	            red->ol2 = ol2;
	            red->metric = Coords(ol2->c2->start->posn)[offdir];
	            if (Redhead2.next == NULL)
	            {
	        	Redhead2.next = red;
	        	red->prev = &Redhead2;
	            }
	            else
	            {
	        	for (red2=Redhead2.next; red2 != NULL; red2=red2->next)
	        	{
	        	    if (red->metric < red2->metric)
	        	    {
	        		red->next = red2;
	        		red->prev = red2->prev;
	        		red2->prev->next = red;
	        		red2->prev = red;
	        		break;
	        	    }
	        	    if (red2->next == NULL)
	        	    {
	        		/* Add to end of list */
	        		red->prev = red2;
	        		red2->next = red;
	        		break;
	        	    }
	                }
	            }
	        }
	    }
	}
	if (Redolaphead.next != NULL)
	{
	    for (ol = olhead->next; ol->next != NULL; ol = ol->next)
	    	; /* find tail of list */
	    ol->next = Redolaphead.next;
	    Redolaphead.next->prev = ol;
	}

	if (DEBUG)
	{
	    (void) printf("redundancy lists:\n");
	    for (red1 = Redhead1.next; red1 != NULL; red1 = red1->next)
	    {
	        (void) printf("red1 %p\n",(POINTER)red1);
	        (void) printf("\tprev %p, next %p\n",
	    		  (POINTER)red1->prev,(POINTER)red1->next);
	        (void) printf("\tmetric = %g\n",red1->metric);
	        (void) printf("\tol1:\n");	print_overlap(red1->ol1);
	        (void) printf("\tol2:\n");	print_overlap(red1->ol2);
	    }
	    for (red2 = Redhead2.next; red2 != NULL; red2 = red2->next)
	    {
	        (void) printf("red2 %p\n",(POINTER)red2);
	        (void) printf("\tprev %p, next %p\n",
	    		  (POINTER)red2->prev,(POINTER)red2->next);
	        (void) printf("\tmetric = %g\n",red2->metric);
	        (void) printf("\tol1:\n");	print_overlap(red2->ol1);
	        (void) printf("\tol2:\n");	print_overlap(red2->ol2);
	    }
	}

red_loops:
	if (Redhead1.next == NULL)
	{
	    if (Redhead2.next == NULL)
	    {
	        DEBUG_LEAVE(delete_redundant_overlaps)
	        return FUNCTION_SUCCEEDED; /*No redundancies to process*/
	    }
	    else
	    {
	        if (DEBUG)
	          (void) printf("failure in delete_redundant_overlaps(), ");
	        DEBUG_LEAVE(delete_redundant_overlaps)
	        return FUNCTION_FAILED;	/*Something is inconsistent*/
	    }
	}	

	for (red1 = Redhead1.next; red1 != NULL; red1 = red1->next)
	{
	    for (red2 = Redhead2.next; red2 != NULL; red2 = red2->next)
	    {
	        /* The "or" tests identify possible redundant paths.
	         * Redundancies can be thought of as multiple paths
	         * connecting two curves.  If the two connected curves
	         * curves are the same (probably a single redundancy on a
	         * closed loop), we don't want to handle it here. */

	        if ((((red1->ol1->c2 == red2->ol1->c1) &&
	              (red1->ol2->c2 == red2->ol2->c1))
	        		     ||
	             ((red1->ol1->c2 == red2->ol2->c1) &&
	              (red1->ol2->c2 == red2->ol1->c1)))
	            &&
	              (red1->ol1->c1 != red2->ol2->c2))
	        {
	            /* double curve redundancy */

	            red_curve = (Coords(red1->ol1->c2->start->posn)[offdir] <
	        		 Coords(red1->ol2->c2->start->posn)[offdir]) ?
	        			 red1->ol1->c2 : red1->ol2->c2;

	            if (DEBUG)
	            {
	        	(void) printf("found double curve redundancy\n");
	        	(void) printf("red1 = %p\n",(POINTER)red1);
	        	(void) printf("red2 = %p\n",(POINTER)red2);
	        	(void) printf("red_curve:\n");
	        	print_curve(red_curve);
	            }

	            (void) delete_curve(red_curve);
	            if (num_curves_at_node(red_curve->start,NULL,NULL) == 0)
	        	(void) delete_node(red_curve->start);
	            if (num_curves_at_node(red_curve->end,  NULL,NULL) == 0)
	        	(void) delete_node(red_curve->end);

	            for (ol = olhead->next; ol != NULL; ol = ol->next)
	        	if ((ol->c1 == red_curve) || (ol->c2 == red_curve))
	        	    remove_from_overlap_list(ol,olhead);

	            for (red1 = Redhead1.next; red1 != NULL; red1 = red1->next)
	            {
	        	if ((red1->ol1->c2 == red_curve) ||
	        	    (red1->ol2->c2 == red_curve))
	        	    remove_from_redundancy_list(red1);
	            }
	            for (red2 = Redhead2.next; red2 != NULL; red2 = red2->next)
	            {
	        	if ((red2->ol1->c1 == red_curve) ||
	        	    (red2->ol2->c1 == red_curve))
	        	    remove_from_redundancy_list(red2);
	            }
	            goto red_loops;
	        }
	        else
	        {
	            red_curve = NULL;
	            if ((red1->ol1     == red2->ol1) &&
	        	(red1->ol2->c2 == red2->ol2->c1))
	            {
	        	red_curve = red1->ol2->c2;
	        	red_ol = red1->ol1;
	            }
	            else if ((red1->ol2     == red2->ol1) &&
	        	     (red1->ol1->c2 == red2->ol2->c1))
	            {
	        	red_curve = red1->ol1->c2;
	        	red_ol = red1->ol2;
	            }
	            else if ((red1->ol1     == red2->ol2) &&
	        	     (red1->ol2->c2 == red2->ol1->c1))
	            {
	        	red_curve = red1->ol2->c2;
	        	red_ol = red1->ol1;
	            }
	            else if ((red1->ol2     == red2->ol2) &&
	        	     (red1->ol1->c2 == red2->ol1->c1))
	            {
	        	red_curve = red1->ol1->c2;
	        	red_ol = red1->ol2;
	            }

	            if (red_curve != NULL)
	            {
	        	int   ncs = local_num_curves_at_node(red_curve->start);
	        	int   nce = local_num_curves_at_node(red_curve->end);

	        	/* The second half of each test below identifies
	        	 * adjacent redundant curves.  At this point, I see no
	        	 * reason to prefer deleting one over another.  The
	        	 * ordering should guarantee that we always process
	        	 * redundancies in the same order, and after the first
	        	 * adjacent redundant curve is deleted, the second
	        	 * should no longer be identified as redundant (if
	        	 * everything works as intended). */

	        	if (((ncs==1) ||
	        	     ((ncs==2) && is_adj_bond_node(red_curve->start)))
	        		    &&
	        	    ((nce==1) ||
	        	     ((nce==2) && is_adj_bond_node(red_curve->end))))
	        	{
	        	    /* Single curve redundancy */

	        	    if (DEBUG)
	        	    {
	        	       (void) printf("found single curve redundancy\n");
	        	       (void) printf("red1 = %p\n",(POINTER)red1);
	        	       (void) printf("red2 = %p\n",(POINTER)red2);
	        	       (void) printf("red_curve:\n");
	        	       print_curve(red_curve);
	                    }

	        	    (void) delete_curve(red_curve);
	        	    (void) delete_node(red_curve->start);
	        	    (void) delete_node(red_curve->end);
	        	    for (ol = olhead->next; ol != NULL; ol = ol->next)
	        	    {
	        		if ((ol->c1 == red_curve) || 
	        		    (ol->c2 == red_curve))
	        			remove_from_overlap_list(ol,olhead);
	        	    }
	        	    for (red1 = Redhead1.next;
	        		 red1 != NULL;
	        		 red1 = red1->next)
	        	    {
	        		if ((red1->ol1->c1 == red_curve) ||
	        		    (red1->ol1->c2 == red_curve) ||
	        		    (red1->ol2->c2 == red_curve))
	        			remove_from_redundancy_list(red1);
	        	    }
	        	    for (red2 = Redhead2.next;
	        		 red2 != NULL;
	        		 red2 = red2->next)
	        	    {
	        		if ((red2->ol1->c1 == red_curve) ||
	        		    (red2->ol2->c1 == red_curve) ||
	        		    (red2->ol2->c2 == red_curve))
	        			remove_from_redundancy_list(red2);
	        	    }
	                }
	        	else
	        	{
	        	    /* This is an unexpected case, and may in fact
	        	     * be an error. For now, just try to keep going.
	        	     */

	        	    if (DEBUG)
	        	    {
	        	       print_curve(red_curve);
	        	       (void) printf("found non-deletable red_curve\n");
	        	       (void) printf("removing ol %p, ",(POINTER)ol);
	        	       (void) printf("red1 %p, red2 %p\n",
	        			     (POINTER)red1,(POINTER)red2);
	        	    }

	        	    remove_from_overlap_list(red_ol,olhead);
	        	    remove_from_redundancy_list(red1);
	        	    remove_from_redundancy_list(red2);
	                }
	        	goto red_loops;
	            }
	        }
	    }
	}
	DEBUG_LEAVE(delete_redundant_overlaps)
	return FUNCTION_SUCCEEDED;
#undef remove_from_redundancy_list
}		/*end delete_redundant_overlaps*/


/*
*			merge_components():
*
*	This function merges two component numbers when joining overlaps
*	across a cut line.  This is needed when a tangle occurs across
*	a subdomain boundary.  The processors on each side of the boundary
*	will perform the untangle, generating new component numbers which
*	may or may not match.
*/

LOCAL	void	merge_components(
	COMPONENT	comp1,
	COMPONENT	comp2,
	INTERFACE	*intfc)
{
	CURVE		**c;
 
	for (c = intfc->curves; c && *c; ++c)
	{
	    if (positive_component(*c) == comp2)
	    	positive_component(*c) = comp1;
	    if (negative_component(*c) == comp2)
	    	negative_component(*c) = comp1;
	}
}		/*end merge_components*/


/*
*			merge_overlapping_bonds():
*
*	Merges a pair of overlapping bonds and their corresponding curves.
*/

LOCAL	boolean	merge_overlapping_bonds(
	OVERLAP		*ol,
	Front		*fr)
{
	CURVE   *newc;
	OVERLAP *ol1;
	boolean sav_intrp = interpolate_intfc_states(fr->interf);
	int     num_cut;

	DEBUG_ENTER(merge_overlapping_bonds)

	if (DEBUG)
	{
	    (void) printf("Merging bonds at overlap\n");
	    print_overlap(ol);
	}

	/* Note: it is possible that (ol->n1 == ol->n2).  This case must be
	 * handled, see documentation for cut_interface(). */

	if ((node_type(ol->n1) == ERROR) && (node_type(ol->n2) == ERROR))
	{
	    if (DEBUG)
		(void) printf("Both nodes have node type ERROR\n");

	    num_cut = 0;
	    if (ol->n1 != ol->n2)
	    {
	        if (!is_clip_node(ol->n1))
	        {
	            double min_sc_sep = MIN_SC_SEP(fr->interf);

	            if (ol->c1->num_points > 2)
	            {
	                MIN_SC_SEP(fr->interf) = 0.0;
	                cut_curve(ol->b1->start,ol->b1->prev,ol->c1,
	                          NEGATIVE_ORIENTATION,fr,
	                          left_state(ol->b1->start),
	                          right_state(ol->b1->start));
	                MIN_SC_SEP(fr->interf) = min_sc_sep;
	                ++num_cut;
	            }
	        }
	        if (!is_clip_node(ol->n2))
	        {
	            double min_sc_sep = MIN_SC_SEP(fr->interf);

	            if (ol->c2->num_points > 2)
	            {
	                MIN_SC_SEP(fr->interf) = 0.0;
	                cut_curve(ol->b2->end,ol->b2->next,ol->c2,
	                          POSITIVE_ORIENTATION,fr,
	                          left_state(ol->b2->end),
	                          right_state(ol->b2->end));
	                MIN_SC_SEP(fr->interf) = min_sc_sep;
	                ++num_cut;
	            }
	        }
	        if (is_clip_node(ol->n1) && !is_clip_node(ol->n2))
	        {
	            /*
		     * Something has caused an inconsistency, and
	             * we are joining one clip and one non-clip
	             * node.  n2->posn (the non-clip node) is
	             * discarded here.
		     */

	            change_node_of_curve(ol->c2,POSITIVE_ORIENTATION,ol->n1);
	            (void) delete_node(ol->n2);
	            ol->n2 = ol->n1;
	        }
	        else
	        {
	            Locstate    sl, sr;
	            /*
	             * If n1 is clip and n2 is not, n1->posn is
	             * discarded here, otherwise both node
	             * positions are retained. In the case of two
	             * clip nodes, one node is lost, but both
	             * positions should be the same.
	             */

	            change_node_of_curve(ol->c1,NEGATIVE_ORIENTATION,ol->n2);
	            if (num_cut == 2)
	            {
	                sl = Left_state_at_node(ol->c1,NEGATIVE_ORIENTATION);
	                sr = Right_state_at_node(ol->c1,NEGATIVE_ORIENTATION);
	                (void) insert_point_in_bond(ol->n1->posn,ol->c1->last,
	                                            ol->c1);
	                ft_assign(left_state(ol->c1->last->start),sl,fr->sizest);
	                ft_assign(right_state(ol->c1->last->start),sr,fr->sizest);
	            }
	            sl = Left_state_at_node(ol->c2,POSITIVE_ORIENTATION);
	            sr = Right_state_at_node(ol->c2,POSITIVE_ORIENTATION);
	            ft_assign(Left_state_at_node(ol->c1,NEGATIVE_ORIENTATION),
	                   sl,fr->sizest);
	            ft_assign(Right_state_at_node(ol->c1,NEGATIVE_ORIENTATION),
	                   sr,fr->sizest);
	            (void) delete_node(ol->n1);
	            ol->n1 = ol->n2;
	        }
	    }

	    if (ol->c1 == ol->c2)
	    {
	        node_type(ol->n1) = CLOSED_NODE;
	        clear_node_flags(ol->n1);
	        set_not_bdry(ol->n1);
	        DEBUG_LEAVE(merge_overlapping_bonds)
	        return FUNCTION_SUCCEEDED;
	    }
	    interpolate_intfc_states(fr->interf) = YES;
	    newc = join_curves(ol->c1,ol->c2,negative_component(ol->c1),
	                       positive_component(ol->c1),NULL);
	    interpolate_intfc_states(fr->interf) = sav_intrp;

	    if (!delete_node(ol->n1))
	    {
	        FT_POINT    *newp;
	        CURVE    **c;

	        /*
		 * In this case, there is still at least one curve
	         * incident on n1.  n1 is assumed to appear in another
	         * overlap(s), but n1->posn now appears in two places:
	         * at the end of the other curve(s) at n1, and as
	         * an interior point of newc.  To avoid later problems
	         * when processing n1, create a new point.
	         */

	        newp = Point(Coords(ol->n1->posn));
	        ol->n1->posn = newp;
	        for (c = ol->n1->out_curves; c && *c; ++c)
	            (*c)->first->start = newp;
	        for (c = ol->n1->in_curves; c && *c; ++c)
	            (*c)->last->end = newp;
	    }
	    for (ol1 = ol->prev; ol1 != NULL; ol1 = ol1->prev)
	    {
	        if (ol1 == ol)
		    continue;
	        if (ol1->c1 == ol->c1)
		    ol1->c1 = newc;
	        if (ol1->c2 == ol->c1)
		    ol1->c2 = newc;
	        if (ol1->c1 == ol->c2)
		    ol1->c1 = newc;
	        if (ol1->c2 == ol->c2)
		    ol1->c2 = newc;
	    }
	    for (ol1 = ol->next; ol1 != NULL; ol1 = ol1->next)
	    {
	        if (ol1 == ol)
		    continue;
	        if (ol1->c1 == ol->c1)
		    ol1->c1 = newc;
	        if (ol1->c2 == ol->c1)
		    ol1->c2 = newc;
	        if (ol1->c1 == ol->c2)
		    ol1->c1 = newc;
	        if (ol1->c2 == ol->c2)
		    ol1->c2 = newc;
	    }
	}
	else if (node_type(ol->n1) == ERROR)
	{
	    /*
	     * In this case, n2 was allowed to retain its physical
	     * node type due to the incidence of multiple curves.  n1
	     * has a single incident curve, and the resolution is simply
	     * to hook this curve to n2.  This is a special case of the
	     * procedure merge_double_physical_cut_nodes().  It is handled
	     * here to simplify setting node types in cut_interface().
	     */

	    if (DEBUG)
		(void) printf("node type ol->n1 == ERROR\n");

	    change_node_of_curve(ol->c1,NEGATIVE_ORIENTATION,ol->n2);
	    (void) delete_node(ol->n1);
	    clear_node_flags(ol->n2);
	}
	else if (node_type(ol->n2) == ERROR)
	{
	    /*
	     * In this case, n1 was allowed to retain its physical
	     * node type due to the incidence of multiple curves.  n2
	     * has a single incident curve, and the resolution is simply
	     * to hook this curve to n1.  This is a special case of the
	     * procedure merge_double_physical_cut_nodes().  It is handled
	     * here to simplify setting node types in cut_interface().
	     */

	    if (DEBUG)
		(void) printf("node type ol->n2 == ERROR\n");

	    change_node_of_curve(ol->c2,POSITIVE_ORIENTATION,ol->n1);
	    (void) delete_node(ol->n2);
	    clear_node_flags(ol->n1);
	}
	else if (ol->n1 == ol->n2 && 
		 is_node_of_closed_curve_only(ol->n1))
	{
	    node_type(ol->n1) = CLOSED_NODE;
	    clear_node_flags(ol->n1);
	    set_not_bdry(ol->n1);
	    return FUNCTION_SUCCEEDED;
	}
	else
	{
	    screen("ERROR in merge_overlapping_bonds(), "
	           "unrecognized configuration.\n");
	    (void) printf("Node %llu boundary %d\n",node_number(ol->n1),
	                  Boundary(ol->n1));
	    (void) printf("Node %llu boundary %d\n",node_number(ol->n2),
	                  Boundary(ol->n2));
	    print_node(ol->n1);
	    print_node(ol->n2);
	    print_curve(*ol->n1->in_curves);
	    print_curve(*ol->n1->out_curves);
	    return FUNCTION_FAILED;
	}
	DEBUG_LEAVE(merge_overlapping_bonds)
	return FUNCTION_SUCCEEDED;
}		/*end merge_overlapping_bonds*/


/*
*			merge_double_physical_cut_nodes():
*
*	This situation happens when a node is near a subdomain boundary.  Both
*	of two adjacent processors shift the node onto the cut line, leaving
*	two CUT_NODE's with known (physical) node type. The resolution is to
*	transfer all curves from one node to the other and then delete the
*	resulting curveless node.
*/

LOCAL	boolean	merge_double_physical_cut_nodes(
	INTERFACE	*intfc,
	int		dir)
{
	CURVE		**c;
	NODE		**n1, **n2;
	NODE		*node;
	double		dist, min_dist;
	int		dim = intfc->dim;

	DEBUG_ENTER(merge_double_physical_cut_nodes)
redo_node_loop:
	for (n1 = intfc->nodes; n1 && *n1; ++n1)
	{
	    if ((!is_cut_node(*n1)) ||
	        ( is_x_cut_node(*n1) && (dir == 1)) ||
	        (!is_x_cut_node(*n1) && (dir == 0)) ||
	        (node_type(*n1) == ERROR))
	    	continue;

	    min_dist = HUGE_VAL;
	    node = NULL;
	    for (n2 = n1+1; n2 && *n2; ++n2)
	    {
	    	if (!is_cut_node(*n2))
		    continue;
	    	if (node_type(*n1) != node_type(*n2))
		    continue;
	    	dist = sqr_separation((*n1)->posn,(*n2)->posn,dim);
	    	if (dist < min_dist)
	    	{
	    	    min_dist = dist;
	    	    node = *n2;
	    	}
	    }
	    if (node == NULL)
	    {
	    	(void) printf("WARNING in merge_double_physical_cut_nodes(), "
	    	              "can't find node corresponding to\n");
	    	print_node(*n1);
	    	return FUNCTION_FAILED;
	    }
	    if (DEBUG)
	    {
	    	(void) printf("Merging nodes:\n");
	    	print_node(*n1);
	    	print_node(node);
	    }

	    /* change_node_of_curve() uses delete_from_pointers(), which
	     * copies the last element into the space for the deleted
	     * element.  This means the for loops require no increment. */

	    for (c = node->in_curves; c && *c; )
	    	change_node_of_curve(*c,NEGATIVE_ORIENTATION,*n1);
	    for (c = node->out_curves; c && *c; )
	    	change_node_of_curve(*c,POSITIVE_ORIENTATION,*n1);

	    (void) delete_node(node);
	    clear_node_flags(*n1);
	    goto redo_node_loop;
	}
	DEBUG_LEAVE(merge_double_physical_cut_nodes)
	return FUNCTION_SUCCEEDED;
}		/*end merge_double_physical_cut_nodes*/


/*
*			new_overlap():
*
*	Creates a new OVERLAP structure and adds it to the linked list.
*/

LOCAL	OVERLAP	*new_overlap(
	OVERLAP		*ol,
	CURVE		*c,
	ORIENTATION	orient,
	int		dir)
{
	static	int	cnt = 0;

	DEBUG_ENTER(new_overlap)

	ol->next = (OVERLAP *)store(sizeof(OVERLAP));
	ol->next->prev = ol;
	ol = ol->next;
	ol->c1 = c;
	ol->b1 = Bond_at_node(c,orient);
	ol->n1 = Node_of(c,orient);
	ol->or1 = orient;
	ol->b2 = NULL;
	ol->c2 = NULL;
	ol->cr2 = NULL;
	ol->or2 = ORIENTATION_NOT_SET;
	ol->dist = HUGE_VAL;
	ol->index = ++cnt;
	if (is_clip_node(ol->n1))
	    ol->cr1 = ol->n1->posn;
	else
	{
	    INTERFACE	*intfc = c->interface;
	    RECT_GRID	*gr = computational_grid(intfc);
	    double		cut;

	    ol->cr1 = Point(NULL);
	    cut = ((Coords(ol->b1->start)[dir] - gr->L[dir]) *
	    	   (Coords(ol->b1->end)[dir]   - gr->L[dir]) <= 0) ?
				gr->L[dir] : gr->U[dir];

	    point_on_cut_line(intfc,ol->cr1,ol->b1,cut,dir);
	}

	if (DEBUG)
	{
	    (void) printf("New overlap\n");
	    print_overlap(ol);
	}
	DEBUG_LEAVE(new_overlap)
	return ol;
}		/*end new_overlap*/


/*
*			remove_from_overlap_list():
*
*	Delinks an OVERLAP structure from the linked list.
*/

LOCAL	void	remove_from_overlap_list(
	OVERLAP		*ol,
	OVERLAP		*olhead)
{

	DEBUG_ENTER(remove_from_overlap_list)

	if (DEBUG)
	{
	    print_overlap(ol);
	}

	if (ol->next != NULL)
	    ol->next->prev = ol->prev;
	if (ol->prev != NULL)
	    ol->prev->next = ol->next;

	if (olhead->next == ol)
	    olhead->next = ol->next;

	DEBUG_LEAVE(remove_from_overlap_list)
}		/*end remove_from_overlap_list*/


/*
*			set_subdomain_boundary():
*
*	Wraps the subdomain boundary around an interface reconstructed
*	by parallel communication.
*/

LOCAL	boolean	set_subdomain_boundary(
	Front		*fr,
	COMPONENT	i_comp)
{
	INTERFACE	*intfc = fr->interf;
	CURVE		**c;
	NODE		**n;
	RECT_GRID	*gr = computational_grid(intfc);
	double		tol[MAXD];
	int		i, j, dim = gr->dim;
	ORIENTATION	orient;
        double           grid_tol;  

        /*grid_tol = grid_tolerance(gr);   */
        grid_tol = 1.0e-7*grid_tolerance(gr);  

	DEBUG_ENTER(set_subdomain_boundary)

	if (DEBUG)
	{
	    (void) printf("Interface into set_subdomain_boundary()\n");
	    print_interface(intfc);
	}
	/* This can fail if a tangle occurs across the virtual boundary. */

	if (!set_boundary(intfc,gr,i_comp,grid_tol))
	{
	    (void) printf("WARNING in set_subdomain_boundary(), "
			  "set_boundary() failed\n");
	    return FUNCTION_FAILED;
	}
	if (DEBUG)
	{
	    (void) printf("Interface in set_subdomain_boundary(), "
			  "after set_boundary\n");
	    print_interface(intfc);
	}

	for (i = 0; i < dim; ++i)
	    tol[i] = MIN_SC_SEP(intfc) * gr->h[i];

	orient = (fr->step % 2) ? POSITIVE_ORIENTATION : NEGATIVE_ORIENTATION;

	for (c = intfc->curves; c && *c; ++c)
	{
	    if (is_bdry(*c) && 
		(wave_type(*c) == ERROR || wave_type(*c) == UNKNOWN_WAVE_TYPE))
	    {
	        rect_bdry_side_for_curve(&i,&j,*c,gr);
		switch(rect_boundary_type(intfc,i,j))
		{
		case PASSIVE_BOUNDARY:
	    	    wave_type(*c) = PASSIVE_BOUNDARY;
		    break;
		case SUBDOMAIN_BOUNDARY:
		case REFLECTION_BOUNDARY:
	    	    wave_type(*c) = SUBDOMAIN_BOUNDARY;
		    break;
		case MIXED_TYPE_BOUNDARY:
		    if (is_excluded_comp(positive_component(*c),intfc) &&
		        is_excluded_comp(negative_component(*c),intfc))
	    	        wave_type(*c) = PASSIVE_BOUNDARY;
		    break;
		default:
		    screen("ERROR in set_subdomain_boundary(), "
			   "unexpected case for rect boundary type\n");
		    clean_up(ERROR);
		}
	    	rect_bdry_curve_redist(*c,orient,gr,tol);
		if (size_of_state(intfc) != 0)
		{
		    BOND *b;
		    size_t sizest = size_of_state(intfc);
		    obstacle_state(intfc,left_start_state(*c),sizest);
		    obstacle_state(intfc,right_start_state(*c),sizest);
		    for (b=(*c)->first; b!=NULL && b!=(*c)->last; b=b->next)
		    {
		        obstacle_state(intfc,left_state(b->end),sizest);
		        obstacle_state(intfc,right_state(b->end),sizest);
		    }
		    obstacle_state(intfc,left_end_state(*c),sizest);
		    obstacle_state(intfc,right_end_state(*c),sizest);
	        }
	    }
	}
	for (n = intfc->nodes; n && *n; ++n)
	{
	    if (is_bdry(*n) && (node_type(*n) == ERROR))
	    	node_type(*n) = FIXED_NODE;
	}
	DEBUG_LEAVE(set_subdomain_boundary)
	return FUNCTION_SUCCEEDED;
}		/*end set_subdomain_boundary*/


LOCAL boolean check_for_cut_nodes(
	INTERFACE	*intfc)
{
	NODE		**n, **n2;
	boolean		status = NO;

	for (n = intfc->nodes; n && *n; ++n)
	{
	    if (is_cut_node(*n))
	    {
		char open_name[100];
		RECT_GRID *gr = computational_grid(intfc);
	    	(void) printf("WARNING in check_for_cut_nodes(), cut node "
			      "found at end of parallel communication.\n");
		print_node(*n);
		sprintf(open_name,"cut-%d",pp_mynode());
		xgraph_2d_intfc_within_range(open_name,intfc,
				Coords((*n)->posn),3.0*gr->h[0],YES);
		/*print_interface(intfc); */
	    	status = YES;
	    }
	}
	return status;
}		/*end check_for_cut_nodes*/


/*
*			local_num_curves_at_node():
*
*	This hacked up version of num_curves_at_node() is provided to look
*	at closed loops.  Sometimes we want to count the same curve twice
*	(as here) and sometimes we don't (as in the original function).
*/

LOCAL int local_num_curves_at_node(
	NODE		*node)
{
	int		num_total = 0;
	CURVE		**c;

	num_total = 0;

	for (c = node->in_curves;  c && *c; ++c)
	    ++num_total;
	
	for (c = node->out_curves; c && *c; ++c)
	    ++num_total;

	return num_total;
}		/*end local_num_curves_at_node*/


LOCAL	double	sqr_separation(
	FT_POINT		*p1,
	FT_POINT		*p2,
	int		dim)
{
	int		i;
	double		sep = 0.0;

	for (i = 0; i < dim; ++i)
	    sep += sqr(Coords(p1)[i] - Coords(p2)[i]);
	return sep;
}		/*end sqr_separation*/


LOCAL	void print_overlap(
	OVERLAP		*ol)
{
	(void) printf("OVERLAP %d, prev %p, next %p, match %d\n",
		      ol->index,(POINTER)ol->prev,(POINTER)ol->next,
		      (ol->match == NULL) ? -1 : ol->match->index);
	(void) printf("\tBonds b1 %llu",bond_number(ol->b1,current_interface()));
	if (ol->b1 != NULL)
	    (void) printf(" (%g %g) -> (%g %g)",
	    	          Coords(ol->b1->start)[0],Coords(ol->b1->start)[1],
			  Coords(ol->b1->end)[0],Coords(ol->b1->end)[1]);
	(void) printf("\n");
	(void) printf("\t      b2 %llu",bond_number(ol->b2,current_interface()));
	if (ol->b2 != NULL)
	    (void) printf(" (%g %g) -> (%g %g)",
			  Coords(ol->b2->start)[0],Coords(ol->b2->start)[1],
			  Coords(ol->b2->end)[0],Coords(ol->b2->end)[1]);
	(void) printf("\n");
	(void) printf("\tCurves c1 %llu c2 %llu\n",
		      curve_number(ol->c1),curve_number(ol->c2));
	(void) printf("\tNodes  n1 %llu n2 %llu\n",
		      node_number(ol->n1),node_number(ol->n2));
	print_general_vector("\tcr1   ",Coords(ol->cr1),2,"\n");
	if (ol->cr2 != NULL)
	    print_general_vector("\tcr2   ",Coords(ol->cr2),2,"\n");
	else
	    (void) printf("\tcr2   NULL\n");
	(void) printf("\tOrientations ");
	print_orientation("or1 = ",ol->or1,", ");
	print_orientation("or2 = ",ol->or2,"\n");
	(void) printf("\tdist %g\n",ol->dist);
	(void) printf("\n");
}		/*end print_overlap*/


LOCAL void print_node_flags(
	NODE		*n)
{
	(void) printf("Node %3llu boundary %5d  ",node_number(n),Boundary(n));
	if (is_bdry(n))
	    (void) printf("BDRY ");
	else
	    (void) printf("     ");
	if (is_cut_node(n))
	{
	    (void) printf("CUT ");

	    if (is_x_cut_node(n))
		(void) printf("X_CUT ");
	    else
		(void) printf("Y_CUT ");
	}
	else
	    (void) printf("    ");
	if (is_clip_node(n))
	    (void) printf("CLIP ");
	else
	    (void) printf("     ");
	if (is_local_node(n))
	    (void) printf("LOCAL ");
	else
	    (void) printf("      ");
	if (is_adj_bond_node(n))
	    (void) printf("ADJ_BOND ");
	else
	    (void) printf("         ");
	(void) printf("\n");
}		/*end print_boundary_flags*/


        /* We want to build a function which can cut a rectangular
         * region on the interface. The interface which is
         * inside this rectangle is throwed out at the end of the
         * cut operation. Only the interface
         * outside the rectangle is remained. This function is an
         * reversed operation of clip_to_interior_region().
         */

EXPORT  void    clip_interface_with_rect(
        INTERFACE       *intfc,         /* interface to be cut */
        double           *L,            /* coordinate of lower cut line */
        double           *U,            /* coordinate of upper cut line */
        boolean            force_clip)     /* if yes clip bond at cut */
{
        int             i, dim = intfc->dim;
        NODE            **n;


        DEBUG_ENTER(clip_interface_with_rect)
        DEBUG_INTERFACE("Interface into clip_interface_with_rect()",intfc);

        /* Interface cross with left rectangle boundary */
        interface_intersection_segment(intfc, 0, 0, L[0], L[1], U[1], force_clip);
        /* Interface cross with right rectangle boundary */
        interface_intersection_segment(intfc, 0, 1, U[0], L[1], U[1], force_clip);
        /* Interface cross with lower rectangle boundary */
        interface_intersection_segment(intfc, 1, 0, L[1], L[0], U[0], force_clip);
        /* Interface cross with upper rectangle boundary */
        interface_intersection_segment(intfc, 1, 1, U[1], L[0], U[0], force_clip);

        delete_curves_inside_rect(intfc, L, U);

        /* One more pass over the nodes is now needed.  We are checking
         * nodes that became CUT_NODE's without a split.  They should be
         * given node type ERROR if the following tests are satisfied.  The
         * reason for this loop is that the last test may not be satisfied
         * until after delete_curves_outside_of_cut_line(). */
        for (n = intfc->nodes; n && *n; ++n)
        {
            /*
            printf("NODE[%g,%g] node flags\n",
              Coords((*n)->posn)[0], Coords((*n)->posn)[1]);
            print_node_flags(*n);
            */
            if ((node_type(*n) != ERROR) && is_cut_node(*n) &&
                (node_type(*n) < FIRST_PHYSICS_NODE_TYPE) &&
                (num_curves_at_node(*n,NULL,NULL) <= 1))
                node_type(*n) = ERROR;
        }

        DEBUG_INTERFACE("Interface into clip_interface_with_rect()",intfc);
        DEBUG_LEAVE(clip_interface_with_rect)
        return;
}

LOCAL   void    delete_curves_inside_rect(
        INTERFACE       *intfc,         /* interface to be cut */
        double           *L,
        double           *U)
{
        CURVE           **cc;
        CURVE           **delete_curves;
        NODE            **nn;
        NODE            **delete_nodes;

        DEBUG_ENTER(delete_curves_inside_rect)
        delete_curves = NULL;
        for (cc = intfc->curves; cc && *cc; ++cc)
        {
            if (curve_in_rect(*cc,L,U))
            {
                if (!add_to_pointers(*cc,&delete_curves))
                {
                    screen("ERROR in delete_curves_outside_of_cut_line(), "
                           "add_to_pointers() failed\n");
                    clean_up(ERROR);
                }
                if (DEBUG)
                {
                    printf("Inside rect L[%g,%g], U[%g,%g]\n",
                           L[0], L[1], U[0], U[1]);
                    (void) printf("adding curve %llu to delete list\n",
                                  curve_number(*cc));
                    print_curve(*cc);
                }
            }
        }
        for (cc = delete_curves; cc && *cc; ++cc)
            (void) delete_curve(*cc);

        delete_nodes = NULL;
        for (nn = intfc->nodes; nn && *nn; ++nn)
        {
            if ( node_in_rect(*nn,L,U) &&
                 (*nn)->in_curves == NULL &&
                 (*nn)->out_curves == NULL
               )
            {
                if (!add_to_pointers(*nn,&delete_nodes))
                {
                    screen("ERROR in delete_curves_outside_of_cut_line(), "
                           "add_to_pointers() failed\n");
                    clean_up(ERROR);
                }
            }
        }
        for (nn = delete_nodes; nn && *nn; ++nn)
            (void) delete_node(*nn);

        DEBUG_LEAVE(delete_curves_inside_rect)
        return;
}

/*
*                       node_in_rect():
*       Determines if nodes lie in the rectangle.
*/
LOCAL int node_in_rect(
        NODE           *n,
        double          *L,
        double          *U)
{
        if (Coords(n->posn)[0] > U[0] ||
            Coords(n->posn)[0] < L[0] ||
            Coords(n->posn)[1] < L[1] ||
            Coords(n->posn)[1] > U[1])
            return NO;
        return YES;

}               /*end node_in_rect*/

/*
*                       curve_in_rect():
*       Determines if all points on a curve lie in the rectangle.
*/

LOCAL int curve_in_rect(
        CURVE           *c,
        double           *L,
        double           *U)
{
        BOND            *b;

        for (b = c->first; b != NULL; b = b->next)
        {
            if (Coords(b->start)[0] > U[0] ||
                Coords(b->start)[0] < L[0] ||
                Coords(b->start)[1] < L[1] ||
                Coords(b->start)[1] > U[1])
                return NO;
        }
        if (Coords(c->end->posn)[0] > U[0] ||
            Coords(c->end->posn)[0] < L[0] ||
            Coords(c->end->posn)[1] < L[1] ||
            Coords(c->end->posn)[1] > U[1])
            return NO;
        return YES;

}               /*end curve_in_rect*/


        /*
         *    interface_intersection_segment() finds out the
         *    interface and the segment intersection. The segment
         *    is the rectangle boundary, The part of the interface inside this
         *    rectangle is throwed when clip_interface_with_rect() is called.
         *    This is the reason we specify the input argument "dir, side".
         *    This function call can be thought of as a restricted analogous
         *    operation of cut_interface() except that the splitted interface
         *    part contained inside the rectangle is not deleted.
         */

LOCAL  void    interface_intersection_segment(
        INTERFACE       *intfc,        /* interface to be cut */
        int             dir,           /* direction of cut line normal */
        int             side,          /* side to be retained */
        double           cut,           /* coordinate of cut line */
        double           L,             /* lower coordinate of cut line in other direction */
        double           U,             /* upper coordinate of cut line in other direction */
        boolean            force_clip)     /* if yes clip bond at cut */
{
        CROSS           Cr, *cr;
        BOND            *b;
        CURVE           *c, **cc;
        CURVE           **curves;
        NODE            **n;
        FT_POINT           *newp;
        FT_POINT           *p;
        boolean            clip;
        double           *h = computational_grid(intfc)->h;
        double           min_sc_sep = MIN_SC_SEP(intfc);/*TOLERANCE*/
        const double     eps = MACH_EPS;/*TOLERANCE*/
        int             i, dim = intfc->dim;
        int             cr_index;
        int             num_deletes = 0;
        int             num_cr = 0;
        boolean            sav_intrp;
        static FT_POINT    *cut_p = NULL;
        static boolean     *adj_bond_cross = NULL;
        static int      cr_alloc_len = 0;
        boolean            save_interior = YES; /* Since this cross test
                                              * is a local operation.
                                              * This flag is set to YES.
                                              */
        double           start_sep, end_sep;

        DEBUG_ENTER(interface_intersection_segment)
        DEBUG_INTERFACE("Interface into interface_intersection_segment()",intfc);

        /* when cut interfaces for assembly and redistribution, it'd be
         * better not to alter the interface.
         * So use this tolerance, which is not grid spacing dependent.
         * The very short bond generated can be deleted by
         * calling intfc_delete_very_short_bonds() later
         * when the assembly operation is done.
         * See the notes for set_min_sc_sep.
         */
        if(YES == set_min_sc_sep)
            min_sc_sep = 100.0*MACH_EPS;

        if (cut_p == NULL)
            cut_p = Static_point(intfc);

        if (DEBUG_CUT && float_equal(cut,2.0))
        {
            static const char *dname[3] = { "x", "y", "z"};
            (void) printf("Removing interface points with %s %s %g\n",
                          dname[dir], (side == 0) ? ">" : "<",cut);
            (void) printf("cut = [%g, %g]->[%g, %g], dir = %d, side = %d\n",
                            cut, L, cut, U, dir, side);
        }

        /* Identify bonds crossing cut line */

        Cr.next = Cr.prev = NULL;
        cr = &Cr;
        for (cc = intfc->curves; cc && *cc; ++cc)
        {
            for (b = (*cc)->first; b != NULL; b = b->next)
            {
                if ((p = bond_crosses_cut_segment(b,dir,side,cut,L,U)) == NULL)
                        continue;

                /* See note at top of function. */
                if (cr && cr->prev &&
                    b->prev && b->prev->prev && b->prev->prev->prev &&
                    ((cr->b1 == b->prev) || (cr->b1 == b->prev->prev)) &&
                    (cr->prev->b1 == b->prev->prev->prev) &&
                    cross_bonds(b,b->prev->prev->prev,cut_p))
                {
                    num_deletes = 3;
                }
                else if (cr && cr->prev &&
                         b->prev && b->prev->prev &&
                         (cr->b1 == b->prev) &&
                         (cr->prev->b1 == b->prev->prev) &&
                         cross_bonds(b,b->prev->prev,cut_p))
                {
                    num_deletes = 2;
                }

                if (num_deletes != 0)
                {
                    BOND        *newb;
                    int         b_cross_seg = NO;

                    Coords(cut_p)[dir] = cut;
                    newp = Point(Coords(cut_p));
                    sav_intrp = interpolate_intfc_states(intfc);
                    interpolate_intfc_states(intfc) = YES;
                    (void) insert_point_in_bond(newp,b,*cc);
                    interpolate_intfc_states(intfc) = sav_intrp;
                    cr = cr->prev;
                    cr->next = NULL;
                    newb = b->next;
                    for (i = 0; i < num_deletes; ++i)
                        (void) delete_start_of_bond(newb->prev,*cc);
                    b = newb->prev;
                    num_deletes = 0;
                    if ((p = bond_crosses_cut_segment(b,dir,side,cut,L,U)) == NULL)
                        continue;
                }
                cr->next = (CROSS *)store(sizeof(CROSS));
                ++num_cr;
                cr->next->prev = cr;
                cr = cr->next;
                cr->c1 = *cc;
                cr->b1 = b;
                cr->p = p;
            }
        }

        if (Cr.next != NULL)
            Cr.next->prev = NULL;

        /* Check the cross list for adjacent bonds.  Closed loops are checked
         * so that the first/last bonds are considered adjacent.  Note that
         * this check must be done here as the bonds stored in the crosses will
         * no longer be accurate with respect to adjacency once splitting
         * occurs.   (adj_bond_cross[cr_index] == YES) if the i'th cross is
         * an adjacent bond cross. */

        if (num_cr > cr_alloc_len)
        {
            cr_alloc_len = 2*num_cr;
            if (adj_bond_cross != NULL)
                free(adj_bond_cross);
            uni_array(&adj_bond_cross,cr_alloc_len,sizeof(boolean));
        }

        for (cr = Cr.next, cr_index = 0; cr != NULL; cr = cr->next, ++cr_index)
        {
            CROSS       *tmp_cr;

            adj_bond_cross[cr_index] = NO;
            for (tmp_cr = Cr.next; tmp_cr != NULL; tmp_cr = tmp_cr->next)
            {
                if (tmp_cr == cr)
                    continue;

                if ((tmp_cr->b1 == cr->b1->prev) ||
                    (tmp_cr->b1 == cr->b1->next) ||
                    (   (tmp_cr->c1 == cr->c1) &&
                        (node_type(cr->c1->start) == CLOSED_NODE) &&
                        (
                            ((cr->b1 == cr->c1->first) &&
                                (tmp_cr->b1 == cr->c1->last)) ||
                            ((cr->b1 == cr->c1->last)  &&
                                (tmp_cr->b1 == cr->c1->first))
                        )
                    )
                )
                {
                    adj_bond_cross[cr_index] = YES;
                    break;
                }
            }
        }

        /* Split curves at appropriate endpoint of crossing bonds */

        for (cr = Cr.next, cr_index = 0; cr != NULL; cr = cr->next, ++cr_index)
        {
            boolean        endpt;

            b = cr->b1;
            c = cr->c1;
            p = cr->p;

            if (DEBUG_CUT && float_equal(cut,2.0) )
            {
                (void) printf("full cross list at top of split loop\n");
                print_cross_list(Cr.next);
                for (i = 0; i < num_cr; ++i)
                    (void) printf("adj_bond_cross[%d] = %s\n",
                                  i,(adj_bond_cross[i]) ? "YES" : "NO");
            }

            endpt = (p == b->end) ? YES : NO;
            clip = (c->num_points <= 3) ? YES : force_clip;

            point_on_cut_line(intfc,cut_p,b,cut,dir);

            if(YES == set_min_sc_sep)
            {
                clip = YES;  
                start_sep = sqr_separation(b->start,cut_p,dim);
                end_sep  = sqr_separation(b->end,cut_p,dim);
            }
            else
            {
                start_sep = scaled_separation(b->start,cut_p,h,dim);
                end_sep  = scaled_separation(b->end,cut_p,h,dim);
            }
            if (start_sep < min_sc_sep)
            {
                /* If start point is near cut line, simply shift it
                 * onto the cut line so as not to create short bonds.
                 */
                if (DEBUG_CUT && float_equal(cut,2.0))
                {
                    printf("Entered scaled_separation(b->start)\n");
                    printf("cut_p [%17.15f, %17.15f]\n", 
                            Coords(cut_p)[0],Coords(cut_p)[1]);
                    printf("b->start [[%17.15f, %17.15f]\n", 
                            Coords(b->start)[0],Coords(b->start)[1]);
                }

                for (i = 0; i < dim; ++i)
                    Coords(b->start)[i] = Coords(cut_p)[i];
                if (endpt)
                {
                    if (b->prev != NULL)
                    {
                        cr->b1 = b = b->prev;
                        cr->p = p = (endpt) ? b->end : b->start;
                    }
                    else
                        endpt = NO;
                }
                clip = YES;
            }
            else if (end_sep < min_sc_sep) 
            {
                /* If end point is near cut line, simply shift it
                 * onto the cut line so as not to create short bonds.
                 */
                if (DEBUG_CUT && float_equal(cut,2.0))
                {
                    printf("Entered scaled_separation(b->end)\n");
                    printf("cut_p [%17.15f, %17.15f]\n", 
                            Coords(cut_p)[0],Coords(cut_p)[1]);
                    printf("b->end [[%17.15f, %17.15f]\n", 
                            Coords(b->end)[0],Coords(b->end)[1]);
                }
                for (i = 0; i < dim; ++i)
                    Coords(b->end)[i] = Coords(cut_p)[i];
                if (!endpt)
                {
                    if (b->next != NULL)
                    {
                        cr->b1 = b = b->next;
                        cr->p = p = (endpt) ? b->end : b->start;
                    }
                    else
                        endpt = YES;
                }
                clip = YES;
            }
            else if (clip && (sqr_separation(p,cut_p,dim) >= eps))
            {
                /* We only want to add a point if there is not
                 * already one there.  This applies mainly to
                 * an elliptic interface, where shifting guarantees
                 * the existence of a point on the cut line. */

                if (DEBUG_CUT && float_equal(cut,2.0))
                {
                    printf("Entered clip\n");
                    printf("cut_p [%17.15f, %17.15f]\n",
                       Coords(cut_p)[0],Coords(cut_p)[1]);
                }

                newp = Point(Coords(cut_p));
                sav_intrp = interpolate_intfc_states(intfc);
                interpolate_intfc_states(intfc) = YES;
                (void) insert_point_in_bond(newp,b,c);
                interpolate_intfc_states(intfc) = sav_intrp;
                rcl_after_insert_point(cr,newp,b);
                if (!endpt)
                    cr->b1 = b = b->next;
                cr->p = p = newp;
            }
            else if (YES == set_min_sc_sep)
            {
                printf("\nERROR In interface_intersection_segment clip = %s\n",
                          clip == YES ? "YES" : "NO");
                printf("The defined min_sc_sep = %18.16f failed to catch"
                       " the cut bond cases\n",min_sc_sep);
                printf("b->start [[%18.16f, %18.16f]\n",
                        Coords(b->start)[0],Coords(b->start)[1]);
                printf("b->end [[%18.16f, %18.16f]\n",
                        Coords(b->end)[0],Coords(b->end)[1]);
                printf("cut_p [%18.16f, %18.16f]\n",
                        Coords(cut_p)[0],Coords(cut_p)[1]);
                printf("p [%18.16f, %18.16f]\n", Coords(p)[0],Coords(p)[1]);
                printf("scaled_separation(b->start,cut_p) = %18.16f\n",
                        scaled_separation(b->start,cut_p,h,dim));
                printf("scaled_separation(b->end,cut_p) = %18.16f\n",
                        scaled_separation(b->end,cut_p,h,dim));

                printf("sqr_separation(b->start,cut_p) = %18.16f\n",
                        sqr_separation(b->start,cut_p,dim));
                printf("sqr_separation(b->end,cut_p) = %18.16f\n",
                        sqr_separation(b->end,cut_p,dim));
                printf("sqr_separation(p,cut_p) = %18.16f\n",
                        sqr_separation(p,cut_p,dim));
                printf("start_sep = %18.16f, end_sep = %18.16f\n",
                       start_sep, end_sep);  
                print_curve(c);
                clean_up(ERROR);
            }


            if (node_type(c->start) == CLOSED_NODE)
            {
                /* The following is rather subtle.  A closed loop
                 * crosses twice.  The first time, the node is
                 * simply shifted, and its type reset.  The second
                 * time, this block is not entered, and a split
                 * is performed, yielding the two curves.
                 *
                 * Note: this block MUST come before the next two in
                 * case the CLOSED_NODE is positioned in the correct
                 * place already (which would satisfy the conditions
                 * for one of the two following blocks).  This case
                 * could also be handled by resetting the node type
                 * of CLOSED_NODE's in the next two blocks.  Instead,
                 * do all processing of CLOSED_NODE's in this block.
                 */

                if (endpt)
                    (void) move_closed_loop_node(c,b->next);
                else
                    (void) move_closed_loop_node(c,b);
                if (save_interior)
                {
                    set_interior_node_flags(c->start,clip,dir);
                    node_type(c->start) = ERROR;
                    if (adj_bond_cross[cr_index])
                        set_adj_bond_cross_node(c->start);
                    /*
                     * 051503: Set the node which overlaps with cut
                     * rectangle upper boundary to be NONE_LOCAL_NODE.
                     * This is because at the later merging interface
                     * process, the nodes of the interface come from fine
                     * level grids are set to be: the node overlaps with
                     * fine grid upper level is LOCAL_NODE.
                     * To check side flag is enough to decide the node location.
                     */
                    if(1 == side)
                        set_imported_node(c->start);
                }
                else
                {
                    set_exterior_node_flags(c->start,c);
                }
            }
            else if ((!endpt) && (b->prev == NULL))
            {
                /* For an elliptic interface, it is possible to shift
                 * a point onto a subdomain boundary (at U and L, NOT
                 * VU and VL).  This point then becomes interior after
                 * the communication, so it needs to be processed, and
                 * the curves joined back into one.  This is the
                 * reason for resetting SUBDOMAIN_NODE below.
                 */

                if (save_interior)
                {
                    set_interior_node_flags(c->start,clip,dir);
                    if (node_type(c->start) == SUBDOMAIN_NODE)
                        node_type(c->start) = ERROR;
                    if (adj_bond_cross[cr_index])
                        set_adj_bond_cross_node(c->start);

                    /* See above node flag comments on 051503 */
                    if(1 == side)
                        set_imported_node(c->start);
                }
                else
                {
                    set_exterior_node_flags(c->start,c);
                }
            }
            else if (endpt && (b->next == NULL))
            {
                /* See comment for above block. */

                if (save_interior)
                {
                    set_interior_node_flags(c->end,clip,dir);
                    if (node_type(c->end) == SUBDOMAIN_NODE)
                        node_type(c->end) = ERROR;
                    if (adj_bond_cross[cr_index])
                        set_adj_bond_cross_node(c->end);

                    /* See above node flag comments on 051503 */
                    if(1 == side)
                        set_imported_node(c->end);
                }
                else
                {
                    set_exterior_node_flags(c->end,c);
                }
            }
            else
            {
                boolean    sav_scss;

                sav_scss = interpolate_states_at_split_curve_node();
                set_interpolate_states_at_split_curve_node(NO);
                curves = split_curve(p,b,c,negative_component(c),
                                     positive_component(c),
                                     negative_component(c),
                                     positive_component(c));
                set_interpolate_states_at_split_curve_node(sav_scss);

                if ((curves == NULL) && (!is_adj_bond_node(c->start)))
                {
                    /* The curve crosses twice, and has already been
                     * split for the first cross. */

                    break;
                }

                if (endpt)
                {
                    if (save_interior)
                    {
                        set_interior_node_flags(curves[0]->end,clip,dir);
                        node_type(curves[0]->end) = ERROR;
                        if (adj_bond_cross[cr_index])
                            set_adj_bond_cross_node(curves[0]->end);

                        /* See above node flag comments on 051503 */
                        if(1 == side)
                            set_imported_node(curves[0]->end);
                    }
                    else
                    {
                        set_exterior_node_flags(curves[0]->end,curves[0]);
                    }
                }
                else
                {
                    if (save_interior)
                    {
                        set_interior_node_flags(curves[1]->start,clip,dir);
                        node_type(curves[1]->start) = ERROR;
                        if (adj_bond_cross[cr_index])
                            set_adj_bond_cross_node(curves[1]->start);

                        /* See above node flag comments on 051503 */
                        if(1 == side)
                            set_imported_node(curves[1]->start);
                    }
                    else
                    {
                        set_exterior_node_flags(curves[1]->start,curves[1]);
                    }
                }

                /* Reset pointer lists after split_curve */
                rcl_after_split(cr,p,b,c,curves);
            }
        }

        DEBUG_INTERFACE("Interface into interface_intersection_segment()",intfc);
        DEBUG_LEAVE(interface_intersection_segment)
        return;
}

/*
*                       bond_crosses_cut_segment():
*
*       Determines if the given bond crosses the cut segments of a given
*       rectangle.  Returning a null pointer indicates that the
*       bond does not cross the rectangle boundary.
*       Otherwise, a pointer to the point contained inside the rectangle
*       is returned. If one end of the
*       bond lies on the cut line, it is said to cross only if the other
*       end lies on the side to be saved. This function is implimented
*       by using same logic as used in bond_crosses_cut_line().
*/
LOCAL   FT_POINT   *bond_crosses_cut_segment(
        BOND    *b,
        int     dir,
        int     side,
        double   cut,
        double   L,
        double   U)
{
        double   crs_crd[MAXD];
        double   x1, y1, x2, y2;

        if(dir == 0)
        {
            x1 = cut; y1 = L;
            x2 = cut; y2 = U;
        }
        else
        {
             x1 = L; y1 = cut;
             x2 = U; y2 = cut;
        }
        if (side == 0)
        {
            if (Coords(b->start)[dir] >= cut &&
                Coords(b->end)[dir] < cut)
            {
                if(cross_segments(Coords(b->start)[0],
                     Coords(b->start)[1],
                     Coords(b->end)[0],
                     Coords(b->end)[1],
                     x1, y1, x2, y2,
                     crs_crd))
                {
                    return b->start;
                }
            }
            if (Coords(b->end)[dir] >= cut &&
                Coords(b->start)[dir] < cut)
            {
                if(cross_segments(Coords(b->start)[0],
                     Coords(b->start)[1],
                     Coords(b->end)[0],
                     Coords(b->end)[1],
                     x1, y1, x2, y2,
                     crs_crd))
                {
                    return b->end;
                }
            }
        }
        else
        {
            if (Coords(b->start)[dir] <= cut &&
                Coords(b->end)[dir] > cut)
            {
                if(cross_segments(Coords(b->start)[0],
                     Coords(b->start)[1],
                     Coords(b->end)[0],
                     Coords(b->end)[1],
                     x1, y1, x2, y2,
                     crs_crd))
                {
                    return b->start;
                }
            }
            if (Coords(b->end)[dir] <= cut &&
                Coords(b->start)[dir] > cut)
            {
                if(cross_segments(Coords(b->start)[0],
                     Coords(b->start)[1],
                     Coords(b->end)[0],
                     Coords(b->end)[1],
                     x1, y1, x2, y2,
                     crs_crd))
                {
                    return b->end;
                }
            }
        }
        return NULL;
}               /*end bond_crosses_cut_segment*/

      /* (x1,y1): start point of bond1,
       * (x2,y2): end point of bond1;
       * (x2,y2): start point of bond2,
       * (x3,y3): end point of bond2.
       * crs_crd: if cross, the crossing point coords.
       */
LOCAL boolean cross_segments(
        double       x1,
        double       y1,
        double       x2,
        double       y2,
        double       x3,
        double       y3,
        double       x4,
        double       y4,
        double       *crs_crd)
{
        double          nor_dist_t,nor_dist_s;
        double          sinth;          /* sin of angle between bonds */
        double          xcross,ycross;  /* coord of intersection
                                           of lines 12 34 */
        double          x00,y00;        /* beginning of long bond */
        double          x0,y0;          /* beginning of short bond after
                                           coord translation */
        double          x,y;            /* end of long bond after
                                           coord translation */
        double          dx,dy;          /* short bond end - start */
        double          t;              /* fractional distance on short bond */
        double          s;              /* fractional distance on long bond */
        double          len12;          /* length b1 * length b2 */
        double          parallel = PARALLEL(current_interface());
        int             i;
        double           b1_len, b2_len;

        b1_len = sqrt(sqr(x1-x2) + sqr(y1-y2));
        b2_len = sqrt(sqr(x3-x4) + sqr(y3-y4));

        if (b1_len > b2_len)
        {
            x00 = x1;           y00 = y1;
            x0 = x3 - x1;       y0 = y3 - y1;
            x  = x2 - x1;       y  = y2 - y1;
            dx = x4 - x3;       dy = y4 - y3;
        }
        else
        {
            x00 = x3;           y00 = y3;
            x0 = x1 - x3;       y0 = y1 - y3;
            x  = x4 - x3;       y  = y4 - y3;
            dx = x2 - x1;       dy = y2 - y1;
        }
        sinth = dx*y - dy*x;
        nor_dist_t = x0*y - y0*x;
        nor_dist_s = dx*y0 - dy*x0;
        len12 = b1_len * b2_len;

        if (fabs(sinth) <= parallel * len12)
        {
            /* Case of parallel lines */
            if (fabs(nor_dist_t) <= parallel * len12)
            {
                /* Lines coincide */
                if (Between(x0,0.0,x) && Between(y0,0.0,y))
                {
                    /* Cross at x0,y0 */
                    crs_crd[0] = (double)(x0 + x00);
                    crs_crd[1] = (double)(y0 + y00);
                    return YES;
                }
                if (Between(x0+dx,0.0,x) && Between(y0+dy,0.0,y))
                {
                    /* Cross at x0+dx,y0+dy */
                    crs_crd[0] = (double)(x0 + dx + x00);
                    crs_crd[1] = (double)(y0 + dy + y00);
                    return YES;
                }
                return NO; /* No cross; line segments don't overlap */
            }
            return NO; /* No cross; lines distinct although parallel */
        }

                /* Now lines are not parallel */

        t = - nor_dist_t / sinth;
        s = nor_dist_s / sinth;
        if (t < 0.0 || t > 1.0 || s < 0.0 || s > 1.0)
            return NO;
        xcross = 0.5*(x0 + t*dx + s*x);
        ycross = 0.5*(y0 + t*dy + s*y);
        crs_crd[0] = (double)(xcross + x00);
        crs_crd[1] = (double)(ycross + y00);
        return YES;
}

EXPORT void set_cut_none_local_flag(
        int     yes_or_no)
{
        set_none_local = yes_or_no;
}
EXPORT int get_cut_none_local_flag(void)
{
        return set_none_local;
}


EXPORT void set_min_sc_sep_val_flag(
        int    flag)
{
        set_min_sc_sep = flag;
}

EXPORT void set_use_delete_short_bonds_flag(void)
{
        use_delete_short_bonds = NO;
}

EXPORT void set_debug_cut_flag(
        int     flag)
{
        DEBUG_CUT = flag;  
}

LOCAL boolean delete_double_cut_curves(INTERFACE *intfc)
{
	NODE 	**n,*ns,*ne;
	CURVE 	**c,*curve;
	boolean	status = FUNCTION_SUCCEEDED;

	for (c = intfc->curves; c && *c; ++c)
	{
	    curve = *c;
	    if (is_cut_node(curve->start) && is_cut_node(curve->end))
	    {
		if (debugging("cut_node"))
		{
		    (void) printf("Double cut node curve found:\n");
		    (void) print_curve(curve);
		}
		if (curve->num_points <=3)
		{
		    if (debugging("cut_node"))
		    {
		    	(void) printf("Small piece of curve, delete.\n");
		    }
		    ns = curve->start;
		    ne = curve->end;
		    delete_curve(curve);
		    delete_node(ns);
		    delete_node(ne);
		}
		else
		{
		    ns = curve->start;
		    ne = curve->end;
		    if (debugging("cut_node"))
		    {
		    	(void) printf("Large piece of curve, close it.\n");
		    	(void) printf("Before operation the curve:\n");
		    	(void) print_curve(curve);
		    	(void) printf("The nodes:\n");
			(void) print_node(ns);
			(void) print_node(ne);
		    }
		    change_node_of_curve(curve,POSITIVE_ORIENTATION,ne);
		    insert_point_in_bond(ns->posn,curve->first,curve);
		    node_type(curve->start) = CLOSED_NODE;
		    clear_node_flags(curve->start);
		    delete_node(ns);
		    if (debugging("cut_node"))
		    {
		    	(void) printf("After operation:\n");
		    	(void) print_curve(curve);
		    	(void) printf("The node:\n");
			(void) print_node(curve->start);
		    }
		}
	    }
	    else if (is_cut_node(curve->start) || is_cut_node(curve->end))
	    {
		if (debugging("cut_node"))
		{
	    	    (void) printf("Single cut node curve:\n");
		    (void) print_curve(curve);
		}
		status = seal_cut_node_curves(curve,intfc);
		if (status) 
		    c = intfc->curves;	/* start from beginning */
	    }
	}
	for (n = intfc->nodes; n && *n; ++n)
	{
	    if (is_cut_node(*n))
	    {
		(void) printf("Single cut node found:\n");
		(void) print_node(*n);
		status = FUNCTION_FAILED;
	    }
	}
	return status;
}	/* end delete_double_cut_curves */


LOCAL boolean seal_cut_node_curves(
	CURVE *curve,
	INTERFACE *intfc)
{
	NODE *ns,*ne;
	CURVE **c,*curve1;
	boolean sav_interp = interpolate_intfc_states(intfc);

	interpolate_intfc_states(intfc) = YES;
	if (is_cut_node(curve->start))
	{
	    ne = curve->start;    
	    for (c = intfc->curves; c && *c; ++c)
	    {
		curve1 = *c;
	    	if (is_cut_node(curve1->end))
		{
		    ns = curve1->end;
		    if (separated_nodes(ns,ne,intfc))
		    	continue;
		    if (debugging("cut_node"))
		    {
		    	(void) printf("Matching cut nodes found:\n");
			(void) print_node(ns);
			(void) print_node(ne);
		    	(void) printf("Matching curves before operation:\n");
			(void) print_curve(curve1);
			(void) print_curve(curve);
		    }
		    change_node_of_curve(curve,POSITIVE_ORIENTATION,ns);
		    insert_point_in_bond(ne->posn,curve->first,curve);
		    curve = join_curves(curve1,curve,negative_component(curve1),
		    		positive_component(curve1),NULL);
		    delete_node(ns);
		    delete_node(ne);
		    if (debugging("cut_node"))
		    {
		    	(void) printf("Matching curve after operation:\n");
			(void) print_curve(curve);
		    }
		    interpolate_intfc_states(intfc) = sav_interp;
		    return FUNCTION_SUCCEEDED;
		}
	    }
	}
	else if (is_cut_node(curve->end))
	{
	    ns = curve->end;    
	    for (c = intfc->curves; c && *c; ++c)
	    {
		curve1 = *c;
	    	if (is_cut_node(curve1->start))
		{
		    ne = curve1->start;
		    if (separated_nodes(ns,ne,intfc))
		    	continue;
		    if (debugging("cut_node"))
		    {
		    	(void) printf("Matching cut nodes found:\n");
			(void) print_node(ns);
			(void) print_node(ne);
		    	(void) printf("Matching curves before operation:\n");
			(void) print_curve(curve1);
			(void) print_curve(curve);
		    }
		    change_node_of_curve(curve,NEGATIVE_ORIENTATION,ne);
		    insert_point_in_bond(ns->posn,curve->last,curve);
		    curve = join_curves(curve,curve1,negative_component(curve),
		    		positive_component(curve),NULL);
		    delete_node(ns);
		    delete_node(ne);
		    if (debugging("cut_node"))
		    {
		    	(void) printf("Matching curve after operation:\n");
			(void) print_curve(curve);
		    }
		    interpolate_intfc_states(intfc) = sav_interp;
		    return FUNCTION_SUCCEEDED;
		}
	    }
	}
	interpolate_intfc_states(intfc) = sav_interp;
	if (debugging("cut_node"))
	    (void) printf("No matching cut node, "
	    	 	  "seal_cut_node_curve() failed\n");
	return FUNCTION_FAILED;
}	/* end seal_cut_node_curve */

LOCAL boolean separated_nodes(
	NODE *n1,
	NODE *n2,
	INTERFACE *intfc)
{
	NODE **n;
	double *p1,*p2,*p;
	int i;

	for (n = intfc->nodes; n && *n; ++n)
	{
	    if (*n == n1 || *n == n2) continue;
	    p1 = Coords(n1->posn);
	    p2 = Coords(n2->posn);
	    p = Coords((*n)->posn);
	    for (i = 0; i < 2; ++i)
	    {
	    	if (p1[i] != p[i] || p2[i] != p[i])
		    continue;
	    	if (p1[(i+1)%2] <= p[(i+1)%2] && p[(i+1)%2] <= p2[(i+1)%2])
		    return YES;
	    	if (p1[(i+1)%2] >= p[(i+1)%2] && p[(i+1)%2] >= p2[(i+1)%2])
		    return YES;
	    }
	}
	return NO;
}	/* end separated_nodes */

