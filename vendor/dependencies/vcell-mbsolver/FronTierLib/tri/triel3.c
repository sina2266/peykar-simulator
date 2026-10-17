/*
*			triel3.c
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Performs a full resolution triangle of an arbitrary
*	interface inside a single grid block.
*/


#if defined(TWOD)
#include <tri/trilocaldecs.h>

enum _INTERIOR_SIDE {
	BOTH_SIDES = UNKNOWN_SIDE,
	LEFT_SIDE  = NEGATIVE_SIDE,
	RIGHT_SIDE = POSITIVE_SIDE
};
typedef enum _INTERIOR_SIDE INTERIOR_SIDE;

enum _CURVE_TYPE {
	BOUNDARY_CURVE = 0,
	INTERIOR_CURVE
};
typedef enum _CURVE_TYPE CURVE_TYPE;

struct _RING_POINT
{
	struct _RING_POINT *next, *prev;
	union
	{
	    double *coords;	/* coordinates of position */
	    FT_POINT *pt;	/* Point at ring point */
	} position;
	double		 angle, min_angle;
	LIN_EL_FACE_TYPE side;
	COMPONENT	 comp;
	Locstate	 _ring_point_state;
	INTERFACE	 *intfc;
};
typedef struct _RING_POINT RING_POINT;

#define	ring_point_state(rpt)		((rpt)->_ring_point_state)
#define	Ring_point_coords(rng)		((rng)->position.coords)
#define	Ring_point_point(rng)		((rng)->position.pt)

#define compute_ang_at_ring_pt(prev_ring,ring,next_ring,cut,dim,nor)	\
	cal_angle(Ring_point_coords(prev_ring),Ring_point_coords(ring),	\
	    	  Ring_point_coords(next_ring),cut,dim,nor)

LOCAL const double MIN_RING_POINT_SEPARATION        = 1.0e-4; /*TOLERANCE*/
LOCAL const double MIN_RING_POINT_SCALED_SEPARATION = 1.0e-8; /*TOLERANCE*/

struct _RECUR_TRI {
	INTERFACE	 *intfc;
	void		 (*generate_tri)(RING_POINT*,RING_POINT*,
					 RING_POINT*,struct _RECUR_TRI*);
	double		 *spacing;
	double		 *nor;
	LIN_EL_FACE_TYPE side_value;
	int		 dim;
	TRI_GRID	 *ntg;
	LINEAR_ELEMENT	 **tri;
	int		 *t_num;
};
typedef struct _RECUR_TRI RECUR_TRI;

#define	Generate_tri(r1,r2,r3,recur_tri)				\
	(*(recur_tri)->generate_tri)(r1,r2,r3,recur_tri)

struct _XY {
	struct _XY *prev, *next;
	TG_PT p;
};
typedef struct _XY XY;

struct _CUT_A_PIECE {
	double *nor;
	int dim;
	POINTER xy_start;
	POINTER xy_end;
};
typedef struct _CUT_A_PIECE CUT_A_PIECE;

struct _TRI_CURVE {
	struct _TRI_CURVE *next;
	CURVE	          *curve;
	INTERIOR_SIDE     interior_side; /* interior side of tri_curve */
	LIN_EL_FACE_TYPE  side;          /* either F_SIDE, E_SIDE or I_SIDE. */
	CURVE_TYPE        curve_type;    /* INTERIOR_CURVE or BOUNDARY_CURVE */ 
	int               tg_pts_index;  /* The index of the first tg_pt */
	int               n_pts;         /* number of pts in the list */
};
typedef struct _TRI_CURVE TRI_CURVE;

struct	_TRI_CURVE_STORE {
	TRI_CURVE	*TrCrStr;
	size_t		NextFree;
	size_t		TrCrStrLen;
	struct _TRI_CURVE_STORE *next;
};
typedef	struct  _TRI_CURVE_STORE TRI_CURVE_STORE;

struct	_RING_POINT_STORE {
	RING_POINT	*RgPtStr;
	size_t		NextFree;
	size_t		RgPtStrLen;
	struct _RING_POINT_STORE *next;
};
typedef	struct  _RING_POINT_STORE RING_POINT_STORE;

#undef OPTIMAL_TRI

/* Only one of the following three choices can be selected */ 
/* for the choice of optimal triangulation  */

#undef  MAX_MIN_ASPECT_RATIO 
#define GLOBAL_ASPECT_RATIO
#undef  MAX_MIN_AREA	     
#undef  MIN_LENGTH	     

enum _SEARCH_FOR_HOLE {
	NO_HOLE    = NO,
	HAS_A_HOLE = YES,
	ERROR_IN_SEARCH_FOR_HOLE = -1
};
typedef enum _SEARCH_FOR_HOLE SEARCH_FOR_HOLE;

	/* LOCAL Function Declarations */
LOCAL	SEARCH_FOR_HOLE	search_for_hole(TRI_CURVE**,CURVE**,XY*,XY*);
LOCAL	Locstate	find_state_at_corner(TRI_GRID*,NODE*,double,double,
					     double,double,int,int,int,int);
LOCAL	RING_POINT	*GetNextRingPoint(void);
LOCAL	RING_POINT	*insert_blk_curve_in_ring(TRI_CURVE*,ORIENTATION,
						  RING_POINT**,
						  int*,TRI_GRID*);
LOCAL	RING_POINT	*largest_min_angle(RING_POINT**);
LOCAL	RING_POINT	*make_ring_point(RING_POINT*,RING_POINT*);
LOCAL   RING_POINT_STORE        *AllocRgPtStrStruct(void);
LOCAL	TRI_CURVE	*GetNextTriCurve(void);
LOCAL	TRI_CURVE	*make_tri_curve(CURVE*,CURVE_TYPE,INTERIOR_SIDE,
                                        LIN_EL_FACE_TYPE,TRI_CURVE**,TRI_GRID*);
LOCAL   TRI_CURVE_STORE        *AllocTrCrStrStruct(void);
LOCAL	boolean	check_and_break_holes(INTERFACE*,ORIENTATION,CURVE*,TRI_CURVE**,
				      TRI_GRID*);
LOCAL	boolean	cut_a_piece(CURVE***,NODE***,ORIENTATION,
			    CURVE*,CUT_A_PIECE*,ORIENTATION*);
LOCAL	boolean	delete_tri_curve(TRI_CURVE*,TRI_CURVE**);
LOCAL	boolean	find_boundary_curve(TRI_CURVE*,TRI_CURVE**);
LOCAL	boolean	find_largest_angle_and_split(RING_POINT**,RING_POINT**,
					     int*,int,double*,LIN_EL_FACE_TYPE,
					     double*,double);
LOCAL	boolean	find_tri_curve_from_curve(TRI_CURVE*,CURVE*,TRI_CURVE**);
LOCAL	boolean	insert_curve_in_xylist(CURVE*,ORIENTATION,CUT_A_PIECE*);
LOCAL	boolean	inside_polygon(CURVE*,XY*,XY*);
LOCAL	boolean	is_a_hole(CURVE**,TRI_CURVE*,XY*,XY*);
LOCAL	boolean	is_cross(FT_POINT*,FT_POINT*,BOND*,int);
LOCAL	boolean	is_cross_in_inserted_curve(FT_POINT*, FT_POINT*, INTERFACE*);
LOCAL	boolean	is_intersect_with_other_edge(RING_POINT*,RING_POINT*,int,double);
LOCAL	boolean	make_hole_cut(NODE*,NODE*,COMPONENT,COMPONENT,
			      TRI_CURVE**,TRI_GRID*);
LOCAL	boolean	recursive_decomposition_elN(TRI_GRID*,LINEAR_ELEMENT**,int*,
					    TRI_CURVE*,ORIENTATION,INTERFACE*,
					    TRI_CURVE**);
LOCAL	boolean	recursive_triangulate(RECUR_TRI*,RING_POINT**,int,double);
LOCAL	boolean	same_comp_on_ring(RING_POINT*);
LOCAL	boolean	set_bdry_states_and_tri_curves(TRI_GRID*,INTERFACE*,
					       TRI_CURVE**,double,double,double,
					       double,int,int,int,int);
LOCAL	boolean	set_nodes_for_make_hole_cut(NODE**,NODE**,NODE**,NODE**,
					    CURVE**,CURVE**,INTERFACE*);
LOCAL	void	FreeRingPointList(void);
LOCAL	void	FreeTriCurveList(void);
LOCAL	void	delete_small_separation_ring_points(RING_POINT**,int*,
						    double*,int);
LOCAL	void	evaluate_angles(RING_POINT*,double*);
LOCAL	void	expand_front_points_list(TRI_GRID*,int);
LOCAL	void	free_xy(XY**,XY**);
LOCAL	void	generate_2d_tri(RING_POINT*,RING_POINT*,RING_POINT*,
				RECUR_TRI*);
LOCAL	void	print_ring_point(RING_POINT*);
LOCAL	void	split_2d_curve_in_tri(BOND*,CURVE*,NODE**);
LOCAL	void	strip_excluded_boundaries(INTERFACE*,TRI_GRID*);
#if defined(DEBUG_TRI_GRID)
LOCAL	void	print_a_tri_curve(TRI_CURVE*);
LOCAL	void	print_ring(RING_POINT*);
LOCAL	void	print_ring_positions(RING_POINT*);
LOCAL	void	print_tri_curves(const char*,TRI_CURVE*);
LOCAL	void	print_xylist(XY*,XY*);
#endif /* defined(DEBUG_TRI_GRID) */

#if defined(OPTIMAL_TRI)
LOCAL	boolean	generate_tri(TRI_GRID*,int**,RING_POINT**,int,int,int,
			     LINEAR_ELEMENT**,int*);
LOCAL	boolean	optimal_triangulation(TRI_GRID*,LINEAR_ELEMENT**,int*,
				      RING_POINT**,int);
LOCAL	boolean	set_distance(double**,int,RING_POINT*,double,int);
LOCAL	double	area_or_ratio(int,int,int,RING_POINT**,double**,double);
LOCAL	void	optimization(double**,int**,double**,RING_POINT**,int,double);
LOCAL	void	print_cost(double**,int);
LOCAL	void	print_vertex(int**,int);
#endif /* defined(OPTIMAL_TRI) */

#define Tg_pt_at_ring_point(rng)	((TG_PT *) Ring_point_coords(rng))

#define distance_between_ring_points(rng1,rng2,dim)			\
		distance_between_positions(Ring_point_coords(rng1),	\
					Ring_point_coords(rng2),dim)

LIB_LOCAL	boolean exact_triangulate_el3(
	TRI_GRID	*ntg,
	NODE		**node,
	int		num_nodes,
	int		*icoords,
	int		ncs,
	int		ncw,
	int		ncn,
	int		nce,
	int		jsw,
	int		jse,
	int		jnw,
	int		jne,
	int		*s_list,
	int		*w_list,
	int		*n_list,
	int		*e_list,
	LINEAR_ELEMENT	**lin)
{
	BLK_EL0		*blk_el0;
	INTERFACE	*save_intfc;
	INTERFACE	*blk_intfc = NULL;
	TRI_CURVE	*lin_bc;
	TRI_CURVE	*lin_curves;
	boolean		status;
	double		L[MAXD], U[MAXD];
	double		tol0, tol1;
	double		tol;
	int		*t_num;

	save_intfc = current_interface();
	L[0] = Coords(ntg->node_points+jsw)[0];
	L[1] = Coords(ntg->node_points+jsw)[1];
	U[0] = Coords(ntg->node_points+jne)[0];
	U[1] = Coords(ntg->node_points+jne)[1];
	tol0 = 0.01*IG_NTOL*(U[0]-L[0]);/*TOLERANCE*/
	tol1 = 0.01*IG_NTOL*(U[1]-L[1]);/*TOLERANCE*/
	tol = min(tol0,tol1);/*TOLERANCE*/
	blk_intfc = make_blk_intfc(ntg,ncs,s_list,jse,nce,e_list,jne,ncn,n_list,
				   jnw,ncw,w_list,jsw,num_nodes,node);
	if (set_boundary(blk_intfc,&topological_grid(blk_intfc),NO_COMP,tol)
	    != FUNCTION_SUCCEEDED)
	{
	    screen("ERROR in exact_triangulate_el3(), set_boundary failed\n");
	    (void) printf("blk_intfc\n");
	    print_interface(blk_intfc);
	    (void) printf("ntg->grid_intfc\n");
	    print_interface(ntg->grid_intfc);
	    clean_up(ERROR);
	}
#if defined(DEBUG_TRI_GRID)
	if( debugging("exact_tri") )
	{
	    (void) printf("blk_intfc after set_boundary()\n");
	    print_interface(blk_intfc);
	}
#endif /* defined(DEBUG_TRI_GRID) */

	strip_excluded_boundaries(blk_intfc,ntg);

#if defined(DEBUG_TRI_GRID)
	if( debugging("exact_tri") )
	{
	    (void) printf("blk_intfc after strip_excluded_boundaries()\n");
	    print_interface(blk_intfc);
	}
#endif /* defined(DEBUG_TRI_GRID) */
	lin_curves = NULL;
	if (set_bdry_states_and_tri_curves(ntg,blk_intfc,&lin_curves,L[0],L[1],
					   U[0],U[1],jsw,jse,jnw,jne) !=
					   FUNCTION_SUCCEEDED)
	{
	    (void) printf("WARNING in exact_triangulate_el3(), "
	                  "set_bdry_states_and_tri_curves failed\n");
	    return FUNCTION_FAILED;
	}
	blk_el0 = &Blk_el0(icoords,ntg);
	blk_el0_linear_els(blk_el0) = *lin;
	t_num = &num_lin_els_in_blk(blk_el0);

	if (find_boundary_curve(lin_curves,&lin_bc) == NO)
	{
	    (void) printf("WARNING in exact_triangulate_el3(), "
	                  "the blk_intfc mesh lines have disappeared\n");
	    return FUNCTION_FAILED;
	}

	status = recursive_decomposition_elN(ntg,lin,t_num,lin_bc,
			                     NEGATIVE_ORIENTATION,blk_intfc,
					     &lin_curves);

	if (status != FUNCTION_SUCCEEDED)
	{
	    (void) printf("WARNING in exact_triangulate_el3(), "
	                  "recursive_decomposition_elN() failed\n");
	}
	FreeRingPointList();
	FreeTriCurveList();
	(void) delete_interface(blk_intfc);
	blk_intfc = NULL;
	set_current_interface(save_intfc);
	return status;
}		/*end exact_triangulate_el3*/

LOCAL	boolean recursive_decomposition_elN(
	TRI_GRID	*ntg,
	LINEAR_ELEMENT	**tri,
	int		*t_num,
	TRI_CURVE	*tri_bc,
	ORIENTATION	orient,
	INTERFACE	*blk_intfc,
	TRI_CURVE	**ptri_curves)
{
#if !defined(OPTIMAL_TRI)
	RECUR_TRI	Recur_tri;
#endif /* !defined(OPTIMAL_TRI) */
	TRI_CURVE	*new_tri_bc, *tri_c, *adj_tri_c;
	RING_POINT	*first_ring, *p_ring;
	CURVE		*adj_c;
	boolean		status;
	ORIENTATION	adj_orient, new_bc_orient;
	double		*h = ntg->comp_grid.h, ls;
	int		i, dim = ntg->comp_grid.dim, num_pts_on_ring=0;

	if (tri_bc == NULL) /* have decomposed all parts in the block intfc */
	     return FUNCTION_SUCCEEDED;

	if (!*ptri_curves)
	    return FUNCTION_SUCCEEDED;

#if defined(DEBUG_TRI_GRID)
	debug_print("recur_decomp","Entered recursive_decomposition_elN()\n");
	if (debugging("recur_decomp"))
	{
	    (void) printf("blk_intfc being triangulated\n");
	    print_interface(blk_intfc);
	    (void) printf("\n");
	    print_tri_curves("The tri_curves in recursive_decomposition_elN()",
			     *ptri_curves);
	    if (debugging("tri_curves"))
	    {
	        (void) printf("Mesh block boundary TRI_CURVE tri_bc\n");
	        print_a_tri_curve(tri_bc);
	    }
	}
#endif /* defined(DEBUG_TRI_GRID) */
  
	for (ls = 0.0, i = 0; i < dim; ++i)
	    ls += h[i]*h[i];
	ls = sqrt(ls);

	if (check_and_break_holes(blk_intfc,Opposite_orient(orient),
			          tri_bc->curve,ptri_curves,ntg) !=
				  FUNCTION_SUCCEEDED)
	{
	    (void) printf("WARNING in recursive_decomposition_elN(), "
	                  "check_and_break_holes() failed\n");
#if defined(DEBUG_TRI_GRID)
	    if (debugging("recur_decomp"))
	    {
	    	print_interface(blk_intfc);
	    	(void) printf("\n");
	    	print_tri_curves("The tri_curves in "
				 "recursive_decomposition_elN()",*ptri_curves);
	    }
#endif /* defined(DEBUG_TRI_GRID) */
	    return FUNCTION_FAILED;
	}

#if defined(DEBUG_TRI_GRID)
	if (debugging("recur_decomp"))
	{
	    (void) printf("This is the boundary curve before "
	                  "insert_blk_curve_in_ring()\n");
	    print_curve(tri_bc->curve);
	    print_orientation("orient =",orient,"\n");
	}

	if (debugging("tri_curves"))
	{
	    print_tri_curves("tri_curves after break_hole",*ptri_curves);
	}
#endif /* defined(DEBUG_TRI_GRID) */

	p_ring = NULL;
	first_ring = insert_blk_curve_in_ring(tri_bc,orient,&p_ring,
					      &num_pts_on_ring,ntg);
	if (first_ring == NULL)
	{
	    (void) printf("WARNING in recursive_decomposition_elN(), "
	                  "insert_blk_curve_in_ring() failed\n");
#if defined(DEBUG_TRI_GRID)
	    if (debugging("recur_decomp"))
	    {
	    	print_interface(blk_intfc);
	    	(void) printf("\n");
	    	print_tri_curves("The tri_curves in "
				 "recursive_decomposition_elN()",*ptri_curves);
	    }
#endif /* defined(DEBUG_TRI_GRID) */
	    return FUNCTION_FAILED;
	}
	if ((adj_c = adjacent_curve(tri_bc->curve,Opposite_orient(orient),
			            CLOCKWISE,&adj_orient)) == NULL)
	{
	    (void) printf("WARNING in recursive_decomposition_elN(), "
	                  "adjacent_curve() (1) returned NULL\n");
	    return FUNCTION_FAILED;
	}

	if (find_tri_curve_from_curve(*ptri_curves,adj_c,&adj_tri_c) !=
							FUNCTION_SUCCEEDED)
	{
	    (void) printf("WARNING in recursive_decomposition_elN(), "
	                  "The adjacent curve is not in tri_curves\n");
#if defined(DEBUG_TRI_GRID)
	    if (debugging("recur_decomp"))
	    {
	    	print_interface(blk_intfc);
	    	(void) printf("\n");
	    	print_tri_curves("The tri_curves in "
				 "recursive_decomposition_elN()",*ptri_curves);
	    }
#endif /* defined(DEBUG_TRI_GRID) */
	    return FUNCTION_FAILED;
	}

	while(adj_tri_c != tri_bc)
	{
	    if (insert_blk_curve_in_ring(adj_tri_c,adj_orient,&p_ring,
	    			         &num_pts_on_ring,ntg) == NULL)
	    {
	        (void) printf("WARNING in recursive_decomposition_elN(), "
		              "insert_blk_curve_in_ring() failed\n");
#if defined(DEBUG_TRI_GRID)
	        if (debugging("recur_decomp"))
	        {
	            print_interface(blk_intfc);
	            (void) printf("\n");
	            print_tri_curves("The tri_curves in "
				     "recursive_decomposition_elN()",
				     *ptri_curves);
	       }
#endif /* defined(DEBUG_TRI_GRID) */
	        return FUNCTION_FAILED;
	    }

	    tri_c = adj_tri_c;
	    orient = adj_orient;
	    if ((adj_c = adjacent_curve(tri_c->curve,Opposite_orient(orient),
					CLOCKWISE,&adj_orient)) == NULL)
	    {
	        (void) printf("WARNING in recursive_decomposition_elN(), "
		              "adjacent_curve() (2) returned NULL\n");
		return FUNCTION_FAILED;
	    }
	    if (find_tri_curve_from_curve(*ptri_curves,adj_c,&adj_tri_c)
							!= FUNCTION_SUCCEEDED)
	    {
	        (void) printf("WARNING in recursive_decomposition_elN(), "
	                      "The the adjacent curve is "
	                      "not in tri_curves\n");
#if defined(DEBUG_TRI_GRID)
		if (debugging("recur_decomp"))
		{
		    print_interface(blk_intfc);
		    (void) printf("\n");
		    print_tri_curves("The tri_curves in "
				     "recursive_decomposition_elN()",
				     *ptri_curves);
	        }
#endif /* defined(DEBUG_TRI_GRID) */
	        return FUNCTION_FAILED;
	    }

	    if (tri_c->curve_type == INTERIOR_CURVE)
	    {
	    	tri_c->curve_type = BOUNDARY_CURVE;
	    	if (orient == POSITIVE_ORIENTATION) 
	    	    tri_c->interior_side = RIGHT_SIDE;
	    	else if (orient == NEGATIVE_ORIENTATION) 
	    	    tri_c->interior_side = LEFT_SIDE;
		else
		{
	            screen("ERROR in recursive_decomposition_elN(), "
		           "inconsistent orient\n");
		    clean_up(ERROR);
		}
	    }
	    else
	    {
	    	if (!delete_tri_curve(tri_c,ptri_curves))
		{
		    (void) printf("WARNING in recursive_decomposition_elN(), "
		                  "delete_tri_curve() failed\n");
#if defined(DEBUG_TRI_GRID)
		    if (debugging("recur_decomp"))
		    {
		        print_interface(blk_intfc);
			(void) printf("\n");
			print_tri_curves("The tri_curves in "
					 "recursive_decomposition_elN()",
				         *ptri_curves);
		    }
#endif /* defined(DEBUG_TRI_GRID) */
		    return FUNCTION_FAILED;
		}

#if defined(DEBUG_TRI_GRID)
		if (debugging("delete_a_tri_curve"))
		{
		    char message[80];

		    (void) sprintf(message,"After delete a tri_c %p\n",
					   (POINTER)tri_c);
		    print_tri_curves(message,*ptri_curves);
		}
#endif /* defined(DEBUG_TRI_GRID) */

	    }
	}
	if (!delete_tri_curve(tri_bc,ptri_curves))
	{
	    (void) printf("WARNING in recursive_decomposition_elN(), "
	                  "delete_tri_curve() failed\n");
#if defined(DEBUG_TRI_GRID)
	    if (debugging("recur_decomp"))
	    {
	        print_interface(blk_intfc);
	    	(void) printf("\n");
	    	print_tri_curves("The tri_curves in "
				 "recursive_decomposition_elN()",*ptri_curves);
	    }
#endif /* defined(DEBUG_TRI_GRID) */
	    return FUNCTION_FAILED;
	}

#if defined(DEBUG_TRI_GRID)
	if (debugging("delete_a_tri_curve"))
	{
	    char message[80];

	    (void) sprintf(message,"After delete a tri_bc %p\n",(POINTER)tri_c);
	    print_tri_curves(message,*ptri_curves);
	}
#endif /* defined(DEBUG_TRI_GRID) */


		/* connect two ends, making it a circular ring */

	first_ring->prev = p_ring;
	p_ring->next = first_ring;

#if defined(DEBUG_TRI_GRID)
	if (debugging("recur_tri")) print_ring_positions(p_ring);
#endif /* defined(DEBUG_TRI_GRID) */

	if (same_comp_on_ring(p_ring) == NO)
	{
	    (void) printf("WARNING in recursive_decomposition_elN(), "
	                  "different components appear in single ring\n");
#if defined(DEBUG_TRI_GRID)
	    if (debugging("recur_decomp"))
	    {
	    	print_interface(blk_intfc);
	    	(void) printf("\n");
	    	print_tri_curves("The tri_curves in "
				 "recursive_decomposition_elN()",*ptri_curves);
	    }
#endif /* defined(DEBUG_TRI_GRID) */
	    return FUNCTION_FAILED;
	}

	delete_small_separation_ring_points(&p_ring,&num_pts_on_ring,
					    ntg->rect_grid.h,
					    ntg->rect_grid.dim);

#if !defined(OPTIMAL_TRI)
	zero_scalar(&Recur_tri,sizeof(RECUR_TRI));
	Recur_tri.generate_tri = generate_2d_tri;
	Recur_tri.spacing = ntg->comp_grid.h;
	Recur_tri.side_value = I_SIDE;
	Recur_tri.dim = ntg->comp_grid.dim;
	Recur_tri.ntg = ntg;
	Recur_tri.tri = tri;
	Recur_tri.t_num = t_num;
#endif /* !defined(OPTIMAL_TRI) */

	if (p_ring != NULL)
	{
	    evaluate_angles(p_ring,NULL);
#if defined(OPTIMAL_TRI)
	    if (!optimal_triangulation(ntg,tri,t_num,&p_ring,num_pts_on_ring))
#else /* defined(OPTIMAL_TRI) */
	    if (!recursive_triangulate(&Recur_tri,&p_ring,num_pts_on_ring,ls))
#endif /* defined(OPTIMAL_TRI) */
	    {
	        (void) printf("WARNING in recursive_decomposition_elN(), "
#if defined(OPTIMAL_TRI)
	                      "optimal_triangulation() failed\n");
#else /* defined(OPTIMAL_TRI) */
	                      "recursive_triangulate() failed\n");
#endif /* defined(OPTIMAL_TRI) */
#if defined(DEBUG_TRI_GRID)
	        if (debugging("recur_decomp"))
		{
		    (void) printf("The blk_intfc in "
				  "recursive_decomposition(), ");
		    print_interface(blk_intfc);
		    (void) printf("\n");
		    print_tri_curves("The tri_curves in "
				     "recursive_decomposition_elN()",
				     *ptri_curves);
		}
#endif /* defined(DEBUG_TRI_GRID) */
	        return FUNCTION_FAILED;
	    }
	}

#if defined(DEBUG_TRI_GRID)
	if (debugging("recur_decomp"))
	{
	    print_tri_curves("The tri_curves before search boundary_curve",
			     *ptri_curves);
	}
#endif /* defined(DEBUG_TRI_GRID) */

	if (find_boundary_curve(*ptri_curves,&new_tri_bc) == NO)
	{
#if defined(DEBUG_TRI_GRID)
	    debug_print("recur_decomp","Left recursive_decomposition_elN()\n");
#endif /* defined(DEBUG_TRI_GRID) */
	    return FUNCTION_SUCCEEDED;
	}
	else
	{
#if defined(DEBUG_TRI_GRID)
	    if (debugging("recur_decomp"))
	    {
	    	(void) printf("\n\nThis in the new boundary_curve\n");
	    	print_a_tri_curve(new_tri_bc);
	    }
#endif /* defined(DEBUG_TRI_GRID) */

	    if (new_tri_bc->interior_side == RIGHT_SIDE)
	    	new_bc_orient = NEGATIVE_ORIENTATION;
	    else if (new_tri_bc->interior_side == LEFT_SIDE)
	    	new_bc_orient = POSITIVE_ORIENTATION;
	    else
	    {
	        screen("ERROR in recursive_decomposition_elN(), "
		       "inconsistent new_tri_bc orient(), "
		       "boundary curve interior to block\n");
		clean_up(ERROR);
	    }
	}

#if defined(DEBUG_TRI_GRID)
	debug_print("recur_decomp",
	      "Recursively calling recursive_decomposition_elN()\n");
#endif /* defined(DEBUG_TRI_GRID) */

	status = recursive_decomposition_elN(ntg,tri,t_num,new_tri_bc,
			new_bc_orient,blk_intfc,ptri_curves);

#if defined(DEBUG_TRI_GRID)
	debug_print("recur_decomp","Left recursive_decomposition_elN()\n");
#endif /* defined(DEBUG_TRI_GRID) */
	return status;
}		/*end recursive_decomposition_elN*/

LOCAL 	void expand_front_points_list(
	TRI_GRID	*ntg,
	int		num)
{
	int		xmax = ntg->rect_grid.gmax[0];
	int		ymax = ntg->rect_grid.gmax[1];
	int 		i;
	int		n_nodes = ntg->n_node_points;
	TG_PT		*opts, *npts;
	TG_PT		*nodes = ntg->node_points;
	LINEAR_ELEMENT	*tri;

	opts = ntg->front_points;
	ntg->n_fr_pts += max((xmax+1)*(ymax+1),num);
	uni_array(&npts,ntg->n_fr_pts,sizeof(TG_PT));
	ntg->front_points = npts;
	for(i = 0; i < ntg->n_tg_pts; ++i)
	{
	    Coords(npts+i)[0] = Coords(opts+i)[0];
	    Coords(npts+i)[1] = Coords(opts+i)[1];
	}
	for(i = 0, tri = ntg->lin_els; i < ntg->n_lin_els; ++i, ++tri)
	{
	    if( tri->p[0] >= nodes && tri->p[0] < nodes+n_nodes )
	    	continue;
	    tri->p[0] = npts + (tri->p[0] - opts);
	    tri->p[1] = npts + (tri->p[1] - opts);
	    tri->p[2] = npts + (tri->p[2] - opts);
	}
	free(opts);
}		/*end expand_front_points_list*/


/*
*			make_tri_curve():
*
*	Creates a new tri_curve which has three fields:
*		curve --- a pointer to a curve;
*       	curve_type --- indicating the curve pointed by field 'curve'
*                        is a boundary curve or a interior curve; (EXPLAIN MORE
*                        WHAT IS A BOUNDARY CURVE OR AN INTERIOR CURVE HERE, 
*                        OR GIVE SOME OTHER NAMES
*		interior_side --- indicate which side is the interior domain.
*
*	Returns a pointer to the created tri_curve or NULL on error..
*/

LOCAL TRI_CURVE *make_tri_curve(
	CURVE		 *c,
	CURVE_TYPE	 curve_type,
	INTERIOR_SIDE	 interior_side,
	LIN_EL_FACE_TYPE side,
	TRI_CURVE	 **tri_curves,
	TRI_GRID	 *ntg)
{
	TRI_CURVE	*tri_c;
	BOND	  	*b;

	if (c == NULL || tri_curves == NULL)
	    return NULL;

	tri_c = GetNextTriCurve();
	if (tri_c == NULL)
	    return NULL;

	tri_c->curve = c;
	tri_c->curve_type = curve_type;
	tri_c->interior_side = interior_side;
	tri_c->side = side;

	if(ntg->n_tg_pts+c->num_points >= ntg->n_fr_pts-1)
	    expand_front_points_list(ntg,c->num_points);

	tri_c->tg_pts_index = ntg->n_tg_pts;
	Coords(ntg->front_points+ntg->n_tg_pts)[0] = Coords(c->start->posn)[0];
	Coords(ntg->front_points+ntg->n_tg_pts)[1] = Coords(c->start->posn)[1];
	++(ntg->n_tg_pts);
	for( b = c->first; b != NULL; b = b->next)
	{
	    Coords(ntg->front_points+ntg->n_tg_pts)[0] = Coords(b->end)[0];
	    Coords(ntg->front_points+ntg->n_tg_pts)[1] = Coords(b->end)[1];
	    ++(ntg->n_tg_pts);
	}

	tri_c->n_pts = ntg->n_tg_pts - tri_c->tg_pts_index;

	if(*tri_curves == NULL) 
	{
	    tri_c->next = NULL;
	    *tri_curves = tri_c;
	}
	else
	{
	    tri_c->next = (*tri_curves)->next;
	    (*tri_curves)->next = tri_c;
	}
	return tri_c;
}		/*end make_tri_curve*/


/* 	
 * 			set_bdry_states_and_tri_curves():
 *
 *   	Set the states for "boundary curves" of the blk_intfc (which
 *	are curves between either mesh points or cross points). 
 *    	This routine also intializes the tri_curves list which contains 
 *	the information used in the recursive triangulation routines.
 *	
 */

LOCAL	boolean set_bdry_states_and_tri_curves(
	TRI_GRID	*ntg,
	INTERFACE	*intfc,
	TRI_CURVE	**tri_curves,
	double		XL,
	double		YL,
	double		XU,
	double		YU,
	int		jsw,
	int		jse,
	int		jnw,
	int		jne)
{
	CURVE		**c,*adj_c;	
	ORIENTATION	adj_c_or;
	Locstate	st;

#if defined(DEBUG_TRI_GRID)
	debug_print("set_blk_bdry_states",
	      "Entered set_bdry_states_and_tri_curves()\n");
#endif /* defined(DEBUG_TRI_GRID) */

	for(c = intfc->curves; c && *c; ++c)
	{
	    if (!is_bdry(*c)) 	/* it is an interior curve */
	    {
#if defined(DEBUG_TRI_GRID)
		if(debugging("set_blk_bdry_states"))
		{
		    (void) printf("This is an interior curve\n");
		    print_curve(*c);
		}
#endif /* defined(DEBUG_TRI_GRID) */

		if (make_tri_curve(*c,INTERIOR_CURVE,BOTH_SIDES,F_SIDE,
					tri_curves,ntg) == NULL)
		{
		    (void) printf("WARNING in "
				  "set_bdry_states_and_tri_curves(), "
		                  "make_tri_curve() returns NULL "
		                  "for interior curve\n");
		    return FUNCTION_FAILED;
		}
	    }
	    else
	    {
		/* store the boundary state at start node */
	        if ((adj_c = adjacent_curve(*c,POSITIVE_ORIENTATION,CLOCKWISE,
					    &adj_c_or)) == NULL)
	        {
	            (void) printf("WARNING in "
		                  "set_bdry_states_and_tri_curves(), "
		                  "adjacent_curve() return NULL\n");
		    return FUNCTION_FAILED;
	        }

#if defined(DEBUG_TRI_GRID)
	        if(debugging("set_blk_bdry_states"))
	        {
		    (void) printf("Setting the start bdry states "
		                  "for curve %llu\n",curve_number(*c));
		    print_curve(*c);
		    (void) printf("The CLOCKWISE adjacent curve %llu\n",
			          curve_number(adj_c));
		    print_orientation("adj_c_or ",adj_c_or,"\n");
		    print_curve(adj_c);
	        }
#endif /* defined(DEBUG_TRI_GRID) */


	        if (is_bdry(adj_c))
	        {
		    st = find_state_at_corner(ntg,(*c)->start,XL,YL,XU,YU,
					      jsw,jse,jnw,jne);
		    if( (st == NULL) && (size_of_state(intfc) > 0) )
		    {
		        (void) printf("WARNING in "
		                      "set_bdry_states_and_tri_curves()\n"
		                      "st = null from "
				      "find_state_at_corner()\n");
		        return FUNCTION_FAILED;
		    }
	        }
	        else
	        {
		    st = (adj_c_or == POSITIVE_ORIENTATION) ?
			left_start_state(adj_c) : right_end_state(adj_c);

		    if( (st == NULL) && (size_of_state(intfc) > 0) )
		    {
		        (void) printf("WARNING in "
			              "set_bdry_states_and_tri_curves(),"
		                      " st == NULL from interior_curve\n");
		        return FUNCTION_FAILED;
		    }
	        }

	        left_start_state(*c) = right_start_state(*c) = st;

		/* store the boundary states at end node */
	        if ((adj_c = adjacent_curve(*c,NEGATIVE_ORIENTATION,
		                            COUNTER_CLOCK,&adj_c_or)) == NULL)
	        {
		    (void) printf("WARNING in "
		                  "set_bdry_states_and_tri_curves(), "
		                  "adjacent_curve() return NULL\n");
		    return FUNCTION_FAILED;
	        }

#if defined(DEBUG_TRI_GRID)
	        if( debugging("set_blk_bdry_states") )
	        {
		    (void) printf("Setting the end bdry states for "
		                  "curve %llu\n",curve_number(*c));
		    print_curve(*c);
		    (void) printf("The COUNTER_CLOCK adjacent curve %llu\n",
		           curve_number(adj_c));
		    print_orientation("adj_c_or ",adj_c_or,"\n");
		    print_curve(adj_c);
	        }
#endif /* defined(DEBUG_TRI_GRID) */

	        if (is_bdry(adj_c))
	        {
		    st = find_state_at_corner(ntg,(*c)->end,XL,YL,XU,YU,
					      jsw,jse,jnw,jne);

		    if( (st == NULL) && (size_of_state(intfc) > 0) )
		    {
		        (void) printf("WARNING in "
		                      "set_bdry_states_and_tri_curves()\n"
		                      "st = null from "
				      "find_state_at_corner()\n");
		        return FUNCTION_FAILED;
		    }
	        }
	        else
	        {
		    st = ( adj_c_or == POSITIVE_ORIENTATION ) ?
		        right_start_state(adj_c) : left_end_state(adj_c);
	        }

	        if( (st == NULL) && (size_of_state(intfc) > 0) )
	        {
		    (void) printf("WARNING in "
		                  "set_bdry_states_and_tri_curves(), "
		                  "st == null from interior_curve\n");
		    return FUNCTION_FAILED;
	        }

	        left_end_state(*c) = right_end_state(*c) = st;
	        if (make_tri_curve(*c,BOUNDARY_CURVE,RIGHT_SIDE,
			           E_SIDE,tri_curves,ntg) == NULL)
	        {
		    (void) printf("WARNING in "
		                  "set_bdry_states_and_tri_curves(), "
		                  "make_tri_curve() returns NULL "
		                  "for boundary curve\n");
		    return FUNCTION_FAILED;
	        }
	    }
	}

#if defined(DEBUG_TRI_GRID)
	if (debugging("TRI_storage"))
	{
	    (void) printf("After set_bdry_states_and_tri_curves(), "
	                  "ntg->n_tg_pts = %d\n",ntg->n_tg_pts);
	}
	debug_print("set_blk_bdry_states","Left set_bdry_states_and_tri_curves()\n");
#endif /* defined(DEBUG_TRI_GRID) */
	return FUNCTION_SUCCEEDED;
}		/*end set_bdry_states_and_tri_curves*/

/*		
 * 			find_state_at_corner():
 *
 *	This routine identifies which corner on the block interface
 *	the node n locates and returns the states at that corner (the
 *      state of a mesh point). Since n is one of the four corners,
 *     	the identification is simply identifying the corner which has
 *      shortest distance from node n, (actually the distance is zero).
 *
 */

LOCAL	Locstate find_state_at_corner(
	TRI_GRID	*ntg,
	NODE		*n,
	double		XL,
	double		YL,
	double		XU,
	double		YU,
	int		jsw,
	int		jse,
	int		jnw,
	int		jne)
{
	double		x, y, distance;
	double		xmLs, xmUs, ymLs, ymUs;
	int		j;

	x = Coords(n->posn)[0];		y = Coords(n->posn)[1];
	xmLs = x - XL;		xmLs *= xmLs;
	xmUs = x - XU;		xmUs *= xmUs;
	ymLs = y - YL;		ymLs *= ymLs;
	ymUs = y - YU;		ymUs *= ymUs;

	distance = xmLs + ymLs;
	j = jsw;
	if(distance > xmLs + ymUs)
	{
	    distance = xmLs + ymUs;
	    j = jnw;
	}
	if(distance > xmUs + ymLs)
	{
	    distance = xmUs + ymLs;
	    j = jse;
	}
	if(distance > xmUs + ymUs)
	{
	    j = jne;
	}

	return  ntg->states[j];
}		/*end find_state_at_corner*/



LOCAL	boolean find_boundary_curve(
	TRI_CURVE	*tri_curves,
	TRI_CURVE	**tri_bc)
{
	TRI_CURVE *tri_c;
	COMPONENT comp;

	for (tri_c = tri_curves; tri_c; tri_c = tri_c->next)
	{
	    if(tri_c->curve_type == BOUNDARY_CURVE)
	    {
	        if (tri_c->interior_side == LEFT_SIDE)
		    comp = negative_component(tri_c->curve);
	        else if (tri_c->interior_side == RIGHT_SIDE)
		    comp = positive_component(tri_c->curve);
		else
		{
		    screen("ERROR in find_boundary_curve(), "
		           "both sides of boundary curve are interior\n");
		    clean_up(ERROR);
		}
		if (!is_excluded_comp(comp,tri_c->curve->interface) &&
		    !is_exterior_comp(comp,tri_c->curve->interface))
		{
	    	  *tri_bc = tri_c;
	    	  return YES;
	        }
	    }
	}
	*tri_bc = NULL;
	return NO;
}		/*end find_boundary_curve*/


LOCAL boolean check_and_break_holes(
	INTERFACE	*blk_intfc,
	ORIENTATION	bc_orient,
	CURVE		*bc,
	TRI_CURVE	**ptri_curves,
	TRI_GRID	*ntg)
{
        CURVE		**piece_c_list, **hole_c_list;
	CUT_A_PIECE	Cpiece;
	NODE		**piece_n_list, **hole_n_list;
	NODE		*n1, *n2;
	TRI_CURVE	*tri_c;
	XY		*xy_start, *xy_end;
	ORIENTATION	orient;
	COMPONENT	comp;
	boolean            search_holes;

#if defined(DEBUG_TRI_GRID)
	debug_print("break_hole","\nEnter check_and_break_holes()\n");
#endif /* defined(DEBUG_TRI_GRID) */

	zero_scalar(&Cpiece,sizeof(CUT_A_PIECE));
	Cpiece.dim = ntg->comp_grid.dim;
	
	/* cut a piece from the blk_intfc */
	Cpiece.xy_start = Cpiece.xy_end = NULL;
	if (cut_a_piece(&piece_c_list,&piece_n_list,bc_orient,bc,&Cpiece,
			&orient) != FUNCTION_SUCCEEDED)
	{
	    (void) printf("WARNING in check_and_break_holes(), "
	                  "cut_a_piece() failed\n");
	    return FUNCTION_FAILED;
	}
	xy_start = (XY *) Cpiece.xy_start;	xy_end = (XY *) Cpiece.xy_end;

		/* Make xylist circular */
	xy_start->prev = xy_end; 		xy_end->next = xy_start;

		/* check holes in the cut piece*/

	tri_c = *ptri_curves;

	comp = (orient == POSITIVE_ORIENTATION)? 
			negative_component(bc) : positive_component(bc);

	search_holes = YES;
	while (search_holes)
	{
	    search_holes = NO;
	    switch (search_for_hole(&tri_c,piece_c_list,xy_start,xy_end))
	    {
	    case HAS_A_HOLE:
	        break;
	    case NO_HOLE:
	        /* No hole in the piece done */

#if defined(DEBUG_TRI_GRID)
	        if (debugging("break_hole"))
	        {
	            (void) printf("No hole in the piece\n"
	        	          "Leaving check_and_break_holes()\n");
	        }
#endif /* defined(DEBUG_TRI_GRID) */

	        free_xy(&xy_start,&xy_end);
	        return FUNCTION_SUCCEEDED;
	    case ERROR_IN_SEARCH_FOR_HOLE:
	    default:
	        return FUNCTION_FAILED;
	    }


#if defined(DEBUG_TRI_GRID)
	    if (debugging("break_hole"))
	        (void) printf("a hole is found (curve %p) \n",tri_c->curve);
#endif /* defined(DEBUG_TRI_GRID) */

	        /* cut a hole from the blk_intfc */
	    Cpiece.xy_start = Cpiece.xy_end = NULL;
	    if (cut_a_piece(&hole_c_list,&hole_n_list,orient,tri_c->curve,
	    		&Cpiece,&orient) != FUNCTION_SUCCEEDED)
	    {
	        (void) printf("WARNING in check_and_break_holes(), "
	                      "cut_a_piece() failed\n");
	        return FUNCTION_FAILED;
	    }

	    		/* try to break the hole. */

	    if (!set_nodes_for_make_hole_cut(&n1,&n2,piece_n_list,hole_n_list,
	    		                piece_c_list,hole_c_list,blk_intfc))
	    {
	        delete_list((POINTER **)&hole_c_list);
	        delete_list((POINTER **)&hole_n_list);
	        tri_c = tri_c->next;
	        search_holes = YES;
	    }
	}

	/* Identify nodes of new hole cut curve */

	if (make_hole_cut(n1,n2,comp,comp,ptri_curves,ntg)!=FUNCTION_SUCCEEDED)
	{
	    (void) printf("WARNING in check_and_break_holes(), "
	                  "make_hole_cut() failed\n");
	    return FUNCTION_FAILED;
	}

	free_xy(&xy_start,&xy_end);

	delete_list((POINTER **) &piece_c_list);
	delete_list((POINTER **) &piece_n_list);
	delete_list((POINTER **) &hole_c_list);
	delete_list((POINTER **) &hole_n_list);

	/* Check for other disjoint holes inside polygon */

#if defined(DEBUG_TRI_GRID)
	debug_print("break_hole","Leaving break_hole()\n");
#endif /* defined(DEBUG_TRI_GRID) */

	return check_and_break_holes(blk_intfc,bc_orient,bc,ptri_curves,ntg);
}		/*end check_and_break_holes*/

LOCAL	SEARCH_FOR_HOLE search_for_hole(
	TRI_CURVE	**ptri_c,
	CURVE		**piece_c_list,
	XY		*xy_start,
	XY		*xy_end)
{
	TRI_CURVE	*tri_c;

	if (ptri_c == NULL) 
	{
	    (void) printf("WARNING in search_for_hole(), ptri_c == NULL\n");
	    return ERROR_IN_SEARCH_FOR_HOLE;
	}

#if defined(DEBUG_TRI_GRID)
	debug_print("break_hole","searching for hole\n");
#endif /* defined(DEBUG_TRI_GRID) */

	for (tri_c = *ptri_c; tri_c; tri_c = tri_c->next)
	{
#if defined(DEBUG_TRI_GRID)
	    if (debugging("break_hole"))
	    {
	        (void) printf("check if there is a hole at tri_curve:\n");
	        print_a_tri_curve(tri_c);
	    }
#endif /* defined(DEBUG_TRI_GRID) */

	    if (is_a_hole(piece_c_list,tri_c,xy_start,xy_end) == YES)
	    {
	    	*ptri_c = tri_c;
	    	return HAS_A_HOLE;
	    }
	}
	*ptri_c = NULL;
	return NO_HOLE;
}		/*end search_for_hole*/

LOCAL  boolean make_hole_cut(
	NODE		*n1,
	NODE		*n2,
	COMPONENT	lcomp,
	COMPONENT	rcomp,
	TRI_CURVE	**ptri_curves,
	TRI_GRID	*ntg)
{
	CURVE		*new_c, *adj_c;
	ORIENTATION	adj_c_or;

#if defined(DEBUG_TRI_GRID)
	debug_print("break_hole","Entered make_hole_cut()\n");
	if (debugging("break_hole"))
	{
	    print_tri_curves("tri_curves before insertion\n",*ptri_curves);
	}
#endif /* defined(DEBUG_TRI_GRID) */

	new_c = make_curve(lcomp,rcomp,n1,n2);

	/* Set states at node of new curve */

	adj_c = adjacent_curve(new_c,POSITIVE_ORIENTATION,CLOCKWISE,&adj_c_or);
	right_start_state(new_c) = (adj_c_or == POSITIVE_ORIENTATION) ?
	    left_start_state(adj_c) : right_end_state(adj_c);

	adj_c = adjacent_curve(new_c,POSITIVE_ORIENTATION,COUNTER_CLOCK,
	                       &adj_c_or);
	left_start_state(new_c) = (adj_c_or == POSITIVE_ORIENTATION) ?
	    right_start_state(adj_c) : left_end_state(adj_c);

	adj_c = adjacent_curve(new_c,NEGATIVE_ORIENTATION,CLOCKWISE,&adj_c_or);
	left_end_state(new_c) = (adj_c_or == POSITIVE_ORIENTATION) ?
	    left_start_state(adj_c) : right_end_state(adj_c);

	adj_c = adjacent_curve(new_c,NEGATIVE_ORIENTATION,COUNTER_CLOCK,
								    &adj_c_or);
	right_end_state(new_c) = (adj_c_or == POSITIVE_ORIENTATION) ?
	    right_start_state(adj_c) : left_end_state(adj_c);

	if (make_tri_curve(new_c,INTERIOR_CURVE,BOTH_SIDES,I_SIDE,
						ptri_curves,ntg) == NULL)
	{
	    (void) printf("WARNING in make_hole_cut(), "
	                  "make_tri_curve returns NULL\n");
	    return FUNCTION_FAILED;
	}


#if defined(DEBUG_TRI_GRID)
	if (debugging("break_hole"))
	{
	    print_tri_curves("tri_curves after insertion",*ptri_curves);
	    (void) printf("The follwing new curve is inserted in the "
	                  "blk_intfc\n");
	    print_curve(new_c);
	}
	debug_print("break_hole","Leaving make_hole_cut()\n");
#endif /* defined(DEBUG_TRI_GRID) */
	return FUNCTION_SUCCEEDED;
}		/*end make_hole_cut*/



/* 
* 		insert_curve_in_xylist():
*
*	This routine inserts a curve in the xy_list which is used for checking
*	and breaking holes inside a piece of the block interface.
*/

LOCAL	XY	*XYStore   = NULL;
LOCAL	size_t	XYStoreLen = 0;
LOCAL	size_t	XYNextFree = 0;

LOCAL 	boolean insert_curve_in_xylist(
	CURVE		*c,
	ORIENTATION	orient,
	CUT_A_PIECE	*cpiece)
{
	BOND		*b;
	FT_POINT		*p;
	XY		*pxy;
	XY		*xy_start = (XY *)cpiece->xy_start;
	XY		*xy_end = (XY *)cpiece->xy_end;

#if defined(DEBUG_TRI_GRID)
	debug_print("break_hole","Enter insert_curve_in_xylist\n");
#endif /* defined(DEBUG_TRI_GRID) */

	if (XYStore == NULL)
	{
	    XYStoreLen = 20;/*TOLERANCE*/
	    uni_array(&XYStore,XYStoreLen,sizeof(XY));
	    if (XYStore == NULL)
	    {
	    	screen("ERROR in insert_curve_in_xylist(), "
	    	       "can't allocate XYStore\n");
	    	clean_up(ERROR);
	    	free_xy((XY **)&cpiece->xy_start,(XY **)&cpiece->xy_end);
	    	return FUNCTION_FAILED;
	    }
	}

	for (b = Bond_at_node(c,orient); b != NULL ;
	     b = Following_bond(b,orient))
	{
	    p = Point_of_bond(b,orient);
	    if (XYNextFree == XYStoreLen)
	    {
	    	XY	*NewXYStore;
	    	size_t	NewXYStoreLen = 2*XYStoreLen;
	    	size_t	i;

	    	uni_array(&NewXYStore,NewXYStoreLen,sizeof(XY));
	    	if (NewXYStore == NULL)
	    	{
	    	    screen("ERROR in insert_curve_in_xylist(), "
	    	           "can allocate XYStore\n");
	    	    clean_up(ERROR);
	    	    free_xy((XY **)&cpiece->xy_start,(XY **)&cpiece->xy_end);
	    	    return FUNCTION_FAILED;
	    	}
	    	for (i = 0; i < XYStoreLen; ++i)
	    	{
	    	    NewXYStore[i].p = XYStore[i].p;
	    	    NewXYStore[i].prev = NewXYStore + (XYStore[i].prev-XYStore);
	    	    NewXYStore[i].next = NewXYStore + (XYStore[i].next-XYStore);
	    	}
	    	xy_start = NewXYStore + (xy_start - XYStore);
	    	xy_end   = NewXYStore + (xy_end   - XYStore);
	    	XYStoreLen = NewXYStoreLen;
	    	free(XYStore);
	    	XYStore = NewXYStore;
	    }
	    pxy = XYStore + XYNextFree++;

	    COORDS(pxy->p)[0] = Coords(p)[0];
	    COORDS(pxy->p)[1] = Coords(p)[1]; 
	    pxy->prev = xy_end;		pxy->next = NULL;
	    if (xy_start == NULL)
	    {
	    	xy_start = pxy;
	    	pxy->prev = pxy->next = NULL;
	    }
	    else
	    {
	    	xy_end->next = pxy;
	    	pxy->prev = xy_end;
	    	pxy->next = NULL;
	    }
	    xy_end = pxy;

#if defined(DEBUG_TRI_GRID)
	    if (debugging("break_hole"))
		(void) printf("The point: xy->p = %g, %g inserted in xylist\n",
			      COORDS(pxy->p)[0],COORDS(pxy->p)[1]);
#endif /* defined(DEBUG_TRI_GRID) */
	}
	cpiece->xy_start = (POINTER) xy_start;
	cpiece->xy_end = (POINTER) xy_end;
#if defined(DEBUG_TRI_GRID)
	debug_print("break_hole","Leaving insert_curve_in_xylist()\n");
#endif /* defined(DEBUG_TRI_GRID) */
	return FUNCTION_SUCCEEDED;
}		/*end insert_curve_in_xylist*/

LOCAL void free_xy(
	XY		**xy_start,
	XY		**xy_end)
{
	zero_scalar(XYStore,XYNextFree*sizeof(XY));
	XYNextFree = 0;
	*xy_start = *xy_end = NULL;
}		/*end free_xy*/



/*
*			is_a_hole():
*
*	This routine checks if there is a hole inside the cut piece from 
*	the block interface.
*/

LOCAL boolean is_a_hole(
	CURVE		**piece,
	TRI_CURVE	*tri_c,
	XY		*xy_start,
	XY		*xy_end)
{
	CURVE		**c;

#if defined(DEBUG_TRI_GRID)
	debug_print("break_hole","Enter is_a_hole()\n");
#endif /* defined(DEBUG_TRI_GRID) */

	for (c = piece; c && *c; ++c)
	{
	    if (    *c == tri_c->curve
	 	|| (*c)->start->posn == tri_c->curve->start->posn
	 	|| (*c)->end->posn   == tri_c->curve->end->posn
	 	|| (*c)->end->posn   == tri_c->curve->start->posn
	 	|| (*c)->start->posn == tri_c->curve->end->posn)
	   {
#if defined(DEBUG_TRI_GRID)
		if (debugging("break_hole"))
		{
		    (void) printf("Leaving is_a_hole() with NO, "
			          "Following curve appears in polygon\n");
		    print_curve(tri_c->curve);
		}
#endif /* defined(DEBUG_TRI_GRID) */
		return NO;
	    }
	}


	if (inside_polygon(tri_c->curve,xy_start,xy_end) == YES)
	{

#if defined(DEBUG_TRI_GRID)
	    if (debugging("break_hole"))
	    {
	    	(void) printf("Leaving is_a_hole() with YES, "
	    	              "Following curve IS inside polygon\n");
	    	print_curve(tri_c->curve);
	    }
#endif /* defined(DEBUG_TRI_GRID) */
	    return YES;
	}

#if defined(DEBUG_TRI_GRID)
	if (debugging("break_hole"))
	{
	    (void) printf("Leaving is_a_hole() with NO, "
	                  "the following curve is NOT inside the polygon\n");
	    print_curve(tri_c->curve);
	}
#endif /* defined(DEBUG_TRI_GRID) */
	return NO;
}		/*end is_a_hole*/

LOCAL boolean inside_polygon(
	CURVE		*c,
	XY		*polygon_start,
	XY		*polygon_end)
{
	XY		*pxy;
	double 		ang;
	double 		d_ang;
	TG_PT  		p;
	int 		i;
	int		dim = c->interface->dim;

#if defined(DEBUG_TRI_GRID)
	if (debugging("inside_polygon"))
	    print_xylist(polygon_start,polygon_end);
#endif /* defined(DEBUG_TRI_GRID) */

	for (i = 0; i < 2; ++i)
	{
	    ang = 0.0;  
	    if (i == 0)
	    {
	    	COORDS(p)[0] = Coords(c->start->posn)[0];
	    	COORDS(p)[1] = Coords(c->start->posn)[1];
	    }
	    else
	    {
	    	COORDS(p)[0] = Coords(c->end->posn)[0];
	    	COORDS(p)[1] = Coords(c->end->posn)[1];
	    }

	    pxy = polygon_start; 
	    while(pxy != NULL)
	    {
	    	d_ang = cal_angle(Coords(&pxy->next->p),Coords(&p),
				  Coords(&pxy->p),MINUS_CUT,dim,NULL);
	    	ang += d_ang;
	    	if (pxy == polygon_end)	break;
	    	else 			pxy = pxy->next;
	    }

#if defined(DEBUG_TRI_GRID)
	    if (debugging("inside_polygon"))
	    	(void) printf("d_ang = %g, ang = %g\n",d_ang,ang);
#endif /* defined(DEBUG_TRI_GRID) */

	    ang = fabs(0.5*ang/PI);

	    if (ang > 0.5)
	    {
#if defined(DEBUG_TRI_GRID)
	    	if (debugging("inside_polygon"))
	    	    (void) printf("point %g, %g is inside the polygon\n",
				  Coords(c->start->posn)[0],
				  Coords(c->start->posn)[1]);
#endif /* defined(DEBUG_TRI_GRID) */
	    	return YES; 
	    }

#if defined(DEBUG_TRI_GRID)
	    if (debugging("inside_polygon"))
	    	(void) printf("point %g, %g is not inside polygon\n",
			      Coords(c->start->posn)[0],
			      Coords(c->start->posn)[1]);
#endif /* defined(DEBUG_TRI_GRID) */
	}
	return NO; 
}		/*end inside_polygon*/


#if !defined(OPTIMAL_TRI)

LOCAL void generate_2d_tri(
	RING_POINT	*r0,
	RING_POINT	*r1,
	RING_POINT	*r2,
	RECUR_TRI	*recur_tri)
{

	set_tri(recur_tri->ntg,recur_tri->tri,recur_tri->t_num,
		Tg_pt_at_ring_point(r0),
		Tg_pt_at_ring_point(r1),
		Tg_pt_at_ring_point(r2),
		ring_point_state(r0),ring_point_state(r1),
		ring_point_state(r2),r0->side,r1->side,r2->side,r0->comp);
}		/*end generate_2d_tri*/

#else /* !defined(OPTIMAL_TRI) */
/*
* 		optimal_triangulation():
*
*	There are four criterions for optimization:
*	1) 	minimize the total internal line segment,
*	2) 	maximize the smallest area of the triangles in the polygan,
*	3) 	maximize the smallest aspect ratio in the polygan,
*	4) 	maximize the total aspect ratio (sum the aspect ratio of each
*		triangle).
*
*	only one of the four criterions can be chosen by modifying the 'define'
*	and the "undef' on the top of this file.
*       This function will generate optimal triangles respect to the given
*	criterion.
*/

LOCAL boolean optimal_triangulation(
	TRI_GRID	*ntg,
	LINEAR_ELEMENT	**tri,
	int		*t_num,
	RING_POINT	**p_ring,
	int		num_pts_on_ring)
{
	RING_POINT	   *r0, *r1, *r2, *ring;
	double              *h = ntg->comp_grid.h;
	double              ls;
	int                dim = ntg->comp_grid.dim;
	int 		   i;
	static	RING_POINT **pt_list = NULL;
	static	double 	   **distance = NULL, **cost = NULL;
	static	int	   **vertex = NULL;
	static	int	   array_len = 0;

#if defined(DEBUG_TRI_GRID)
	debug_print("optimal_tri","Enter optimal_triangulation()\n");
	if (debugging("optimal_tri"))
	{
	    (void) printf("the number of points on ring =  %d\n",
			  num_pts_on_ring);
	    (void) printf("The ring which enters optimal_triangulation()\n"); 
	    print_ring(*p_ring);
	}
#endif /* defined(DEBUG_TRI_GRID) */

	if (num_pts_on_ring < 3)
	{
	    (void) printf("WARNING in optimal_triangulation(), "
	                  "num_pts_on_ring (%d) less then 3\n",
			  num_pts_on_ring);
	    return NO;
	}

	if (num_pts_on_ring == 3)
	{
	    r0 = (*p_ring)->prev;
	    r1 = (*p_ring);
	    r2 = (*p_ring)->next;

	    set_tri(ntg,tri,t_num,Tg_pt_at_ring_point(r0),
		    Tg_pt_at_ring_point(r1),Tg_pt_at_ring_point(r2),
		    ring_point_state(r0),ring_point_state(r1),
		    ring_point_state(r2),r0->side,r1->side,r2->side,r0->comp);

#if defined(DEBUG_TRI_GRID)
            debug_print("optimal_tri","Leaving optimal_triangulation() = GOOD\n");
#endif /* defined(DEBUG_TRI_GRID) */
	    return YES;
	}

	for (ls = 0.0, i = 0; i < dim; ++i)
	    ls += h[i]*h[i];
	ls = sqrt(ls);

	if (distance == NULL)
	{
	    array_len = num_pts_on_ring;
	    bi_array(&distance,array_len,array_len,FLOAT);
	    bi_array(&cost,array_len,array_len-1,FLOAT);
	    bi_array(&vertex,array_len,array_len-3,INT);
	    uni_array(&pt_list,array_len,sizeof(RING_POINT *));
	}
	else if (num_pts_on_ring > array_len)
	{
	    free_these(4,cost,distance,vertex,pt_list);
	    array_len = num_pts_on_ring;
	    bi_array(&distance,array_len,array_len,FLOAT);
	    bi_array(&cost,array_len,array_len-1,FLOAT);
	    bi_array(&vertex,array_len,array_len-3,INT);
	    uni_array(&pt_list,array_len,sizeof(RING_POINT *));
	}

	for (i=0, ring=*p_ring; i<num_pts_on_ring; ++i, ring=ring->next);
	    pt_list[i] = ring; 
	if (set_distance(distance,num_pts_on_ring,*p_ring,ls,dim) != YES)
	{
	    (void) printf("WARNING in optimal_triangulation(), "
	                  "set_distance() failed\n");
	    return NO;
	}
	optimization(cost,vertex,distance,pt_list,num_pts_on_ring,ls);
	if (generate_tri(ntg,vertex,pt_list,0,num_pts_on_ring,
			     num_pts_on_ring,tri,t_num) != YES)
	{
	    (void) printf("WARNING in optimal_triangulation(), "
	                  "generate_tri() failed\n");
	    return NO;
	}

	*p_ring = NULL;
	return YES;
}		/*end optimal_triangulation*/

/* 
 *			set_distance():
 *
 * 	This function sets the distance between points on a polygon.
 *      If the line segment between two points intersects ang edge of the
 *	polygon, that distance to some forbiden value. Therefore such line
 *      segment will not be selected on the triangulation. 
 *	Similarly, when the line segment lies outside of the polygon, the
 *	distance is also set to some forbiden value.
 */

LOCAL 	boolean set_distance(
	double	   **distance,
	int	   num_pts_on_ring,
	RING_POINT *pring,
	double      ls,
	int	   dim)
{
	RING_POINT	*pi,*pj,*pk;
	double		r,angle;
	double		forbiden;
	int		i, j, k;

#if defined(MIN_LENGTH)
	forbiden = HUGE_VAL;
#else /* defined(MIN_LENGTH) */
	forbiden = 0.0;
#endif /* defined(MIN_LENGTH) */
	

	for (i = 0, pi = pring; i < num_pts_on_ring; ++i, pi = pi->next)
	{
	    distance[i][i] = forbiden;         /* from a point to itself */

	    	/* edge of the polygon */
	    j = (i +1) % num_pts_on_ring;
#if defined(MIN_LENGTH)
	    distance[i][j] = distance[j][i] = 0.0;
#else /* defined(MIN_LENGTH) */
	    pj = pi->next;				
	    distance[i][j] = distance[j][i] =
		distance_between_ring_points(pi,pj,dim);
#endif /* defined(MIN_LENGTH) */
	}

		/* internal line segmwnt between points on polygon */
	for (i = 0, pi = pring; i < num_pts_on_ring-2; ++i, pi = pi->next)
	{
	    for (j=i+2, pj=pi->next->next;
	    	 j < num_pts_on_ring && pj != pi->prev; ++j,pj=pj->next)
	    {
	    	r = distance_between_ring_points(pi,pj,dim);
	    	distance[i][j] = distance[j][i] = r;
	    	if (r < MIN_RING_POINT_SEPARATION*ls)
	    	{
	    			/* point from break hole */
	    	    distance[i][j] = distance[j][i] = forbiden;
	    	    r = distance_between_ring_points(pi->next,pj->prev,dim);
	    	    if (r < MIN_RING_POINT_SEPARATION*ls)
	    	    {
	    	    	/* point from break hole */
	    	    	distance[i][j-1] = distance[j-1][i] = forbiden;
	    		continue;
	    	    }
	    	    r = distance_between_ring_points(pi->prev,pj->next,dim);
	    	    if (r < MIN_RING_POINT_SEPARATION*ls)
	    	    {
	    	    	/* point from break hole */
	    	    	distance[i][j+1] = distance[j+1][i] = forbiden;
	    	    	++j; 	pj = pj->next;
	    	    	continue;
	    	    }
	    	    (void) printf("WARNING in set_distance(), The coincident "
				  "point does not appear as a pair\n");
	    	    return NO;
	    	}	
	    	angle = compute_ang_at_ring_pt(pi->prev,pi,pj,PLUS_CUT,dim,
					       NULL);
	    	if (angle >= pi->angle)  /* lie outside of polygon */
	    	{
	    	    distance[i][j] = distance[j][i] = forbiden;
	    	    continue;
	    	}
	    	angle = compute_ang_at_ring_pt(pj,pi,pi->next,PLUS_CUT,dim,
					       NULL);
	    	if (angle >= pi->angle)  /* lie outside of polygon */
	    	{
	    	    distance[i][j] = distance[j][i] = forbiden;
	    	    continue;
	    	}
	    	angle = compute_ang_at_ring_pt(pj->prev,pj,pi,PLUS_CUT,dim,
					       NULL);
	    	if (angle >= pj->angle)  /* lie outside of polygon */
	    	{
	    	    distance[i][j] = distance[j][i] = forbiden;
	    	    continue;
	    	}
	    	angle = compute_ang_at_ring_pt(pi,pj,pj->next,PLUS_CUT,dim,
					       NULL);
	    	if (angle >= pj->angle)  /* lie outside of polygon */
	    	{
	    	    distance[i][j] = distance[j][i] = forbiden;
	    	    continue;
	    	}
	    	for (k = 0, pk = pring; k < num_pts_on_ring; ++k, pk = pk->next)
		{
		    /*check if it interscts with any polygon edge*/
		    if (is_intersect_with_other_edge(pi,pj,dim,ls) == YES)
		    {
		    	distance[i][j] = distance[j][i] = forbiden;
		    	break;
		    }
		}
	    }
	}
	if (debugging("set_distance"))
	{
	    (void) printf("\nThe matrix element of distances:"
	                  "\ni  j  distance\n");
	    for (i = 0; i < num_pts_on_ring; ++i)
	    	for (j = 0; j < num_pts_on_ring; ++j)
	    	    (void) printf("%d %d %g\n",i,j,distance[i][j]);
	    (void) printf("\n");
	}
	return YES;
}		/*end set_distance*/

LOCAL	void optimization(
	double		**cost,
	int		**vertex,
	double		**distance,
	RING_POINT	**pt_list,
	int		pts,
	double		ls)
{
	int		i,j,k,s,v,p,ibest,newv,uplimit;
	double		best;
	double		f1,f2,f3;

	for (i = 0; i < pts; ++i)	 
	{
#if defined(MIN_LENGTH)
	    cost[1][i] = 0.0;
#else /* defined(MIN_LENGTH) */
	    j = (i + 1 + pts) % pts;
	    k = (i + 2 + pts) % pts;
	    cost[1][i] = area_or_ratio(i,j,k,pt_list,distance,ls);
#endif /* defined(MIN_LENGTH) */
	}	

	for (s = 4; s <= pts; ++s)
	{
	    uplimit = (s == pts)? 1 : pts;
	    for (v = 0; v < uplimit; ++v)
	    {

#if defined(MIN_LENGTH)
	        cost[0][1] = distance[(v+s-1)%pts][(v+1)%pts] +
		    				cost[s-3][(v+1)%pts];
#else /* defined(MIN_LENGTH) */
		f1 = area_or_ratio(v,(v+1)%pts,(v+s-1)%pts,
		    		   pt_list,distance,ls);
	    	f2 = cost[s-3][(v+1)%pts];
#if defined(GLOBAL_ASPECT_RATIO)
	    	cost[0][1] = f1 + f2;
#else /* defined(GLOBAL_ASPECT_RATIO) */
	    	cost[0][1] = (f1 < f2)? f1 : f2;
#endif /* defined(GLOBAL_ASPECT_RATIO) */
#endif /* defined(MIN_LENGTH) */

	    	for (k = 2; k < s-2; ++k)	
	    	{
	    	    p = (v+k)%pts;
	    		newv = (v+s-1)%pts;
#if defined(MIN_LENGTH)
	    	    cost[0][k] = distance[v][p] + distance[newv][p]
	    			+ cost[k-1][v] + cost[s-k-2][p];
#else /* defined(MIN_LENGTH) */
		    f1 = area_or_ratio(v,p,newv,pt_list,distance,ls);
	    	    f2 = cost[k-1][v];
	    	    f3 = cost[s-k-2][p];
#if defined(GLOBAL_ASPECT_RATIO)
	    	    cost[0][k] = f1 + f2 + f3;
#else /* defined(GLOBAL_ASPECT_RATIO) */
	    	    if (f2 < f1) f1 = f2;
	    	    if (f3 < f1) f1 = f3;
	    	    cost[0][k] = f1;
#endif /* defined(GLOBAL_ASPECT_RATIO) */
#endif /* defined(MIN_LENGTH) */
	    	}
#if defined(MIN_LENGTH)
	    	cost[0][s-2] = distance[v][(v+s-2)%pts]
	    			+ cost[s-3][v];
#else /* defined(MIN_LENGTH) */
#if defined(GLOBAL_ASPECT_RATIO)
	    	cost[0][s-2] = f1 + f2;
#else /* defined(GLOBAL_ASPECT_RATIO) */
	    	f1 = area_or_ratio(v,(v+s-2)%pts,(v+s-1)%pts,
	    				    pt_list,distance,ls);
	    	f2 = cost[s-3][v];
	    	cost[0][s-2] = (f1 < f2)? f1 : f2;
#endif /* defined(GLOBAL_ASPECT_RATIO) */
#endif /* defined(MIN_LENGTH) */

	    	for (best=cost[0][1], ibest=1, k=2; k < s-1; ++k)
	    	{
#if defined(MIN_LENGTH)
	    	    if (cost[0][k] < best) 	
#else /* defined(MIN_LENGTH) */
#if defined(GLOBAL_ASPECT_RATIO)
	    	    if (cost[0][k] < best) 	
#else /* defined(GLOBAL_ASPECT_RATIO) */
	    	    if (cost[0][k] > best) 	
#endif /* defined(GLOBAL_ASPECT_RATIO) */
#endif /* defined(MIN_LENGTH) */
	    	    { 		
	    	    	ibest = k;   best = cost[0][k];
	    	    }
	    	}
	    	cost[s-2][v] = best;  vertex[s-4][v] = (v+ibest)%pts;
	    }
	}
}		/*end optimization*/

LOCAL	boolean generate_tri(
	TRI_GRID	*ntg,
	int		**vertex,
	RING_POINT	**pt_list,
	int		v,
	int		pts,
	int		num_pts_on_ring,
	LINEAR_ELEMENT	**tri,
	int		*t_num)
{
	int		newv, newpts;
	RING_POINT	*r0, *r1, *r2;

	if (pts < 2) 
	{
	    (void) printf("WARNING in generate_tri(), "
	                  "pts = %d ( <2 ) in generate_tri\n",pts);
	    return NO;
	}

	if (pts == 2)
	    return YES;

	if (pts == 3)
	{
	    r0 = pt_list[v];
	    r1 = pt_list[v]->next;
	    r2 = pt_list[v]->next->next;

	    set_tri(ntg,tri,t_num,Tg_pt_at_ring_point(r0),
		    Tg_pt_at_ring_point(r1),Tg_pt_at_ring_point(r2),
		    ring_point_state(r0),ring_point_state(r1),
		    ring_point_state(r2),r0->side,r1->side,r2->side,r0->comp);
	    return YES;
	}

	newv = vertex[pts-4][v];
	r0 = pt_list[v];
	r1 = pt_list[newv];
	r2 = pt_list[(v+pts-1)%num_pts_on_ring];

	set_tri(ntg,tri,t_num,Tg_pt_at_ring_point(r0),Tg_pt_at_ring_point(r1),
		Tg_pt_at_ring_point(r2),ring_point_state(r0),
		ring_point_state(r1),ring_point_state(r2),r0->side,r1->side,
		r2->side,r0->comp);
	
	newpts = (newv + num_pts_on_ring - v + 1) % num_pts_on_ring;
	if (!generate_tri(ntg,vertex,pt_list,v,newpts,num_pts_on_ring,
			     tri,t_num))
	{
	    (void) printf("WARNING in generate_tri(), "
	                  "generate_tri() failed on recursive call\n");
	    return NO;
	}
	newpts = (v + pts + num_pts_on_ring - newv) % num_pts_on_ring;
	return generate_tri(ntg,vertex,pt_list,newv,newpts,num_pts_on_ring,
			    tri,t_num);
}		/*end generate_tri*/

LOCAL double area_or_ratio(
	int		i,
	int		j,
	int		k,
	RING_POINT	**pt_list,
	double		**distance,
	double		ls)
{
	double		r0,r1,r2,m;
	double		area, cp[MAXD];

	if (( distance[i][j]*distance[i][k]*distance[j][k]) <
						MIN_RING_POINT_SEPARATION*ls)
	    return 0.0;
	
	area = vector_product_on_points(Ring_point_coords(pt_list[i]),
				        Ring_point_coords(pt_list[j]),
				        Ring_point_coords(pt_list[k]),
				        pt_list[i]->intfc->dim,cp);

#if defined(MAX_MIN_AREA)
	return area;
#else /* defined(MAX_MIN_AREA) */
	r0 = distance[i][j];
	r1 = distance[i][k];
	r2 = distance[j][k];
	/*
	m  = (r0 + r1 + r2) / 3.0;
	return sqr(m)/(sqr(r0 - m) + sqr(r1 - m) + sqr(r2 - m) + sqr(m));
	*/
	m  = sqr(r0 + r1 + r2);
	return area/m;
#endif /* defined(MAX_MIN_AREA) */
}		/*end area_or_ratio*/

LOCAL 	void print_cost(
	double		**cost,
	int		pts)
{
	int		i,j;	

	(void) printf("\nCOST:\ns v cost\n");
	for (i = 1; i < pts-1; ++i)
	    for (j = 0; j < pts; ++j)
	    	(void) printf("%d %d %g\n",i+2,j,cost[i][j]);
}		/*end print_cost*/

LOCAL 	void print_vertex(
	int		**vertex,
	int		pts)
{
	int		i,j;	

	(void) printf("\nVERTEX:\ns v vertex\n");
	for (i = 0; i < pts-3; ++i)
	    for (j = 0; j < pts; ++j)
	    	(void) printf("%d %d %d\n",i+4,j,vertex[i][j]);
}		/*end print_vertex*/
#endif /* !defined(OPTIMAL_TRI) */

LOCAL	boolean delete_tri_curve(
	TRI_CURVE	*tri_c,
	TRI_CURVE	**ptri_curves)
{
	TRI_CURVE	*tc, *prev;

	if (ptri_curves == NULL)
	    return FUNCTION_SUCCEEDED;
#if defined(DEBUG_TRI_GRID)
	if (debugging("recur_decomp"))
	{
	    (void) printf("Deleting TRI_CURVE\n");
	    print_a_tri_curve(tri_c);
	}
#endif /* defined(DEBUG_TRI_GRID) */
	for (tc = *ptri_curves, prev = NULL; tc; prev = tc, tc = tc->next)
	{
	    if (tc == tri_c)
	    {
	    	if (prev == NULL)
	    	    *ptri_curves = tri_c->next;
	    	else
	    	    prev->next = tri_c->next;
	    	return FUNCTION_SUCCEEDED;
	    }
	}

	(void) printf("WARNING in delete_tri_curve(), tri_c = %p is not in the "
		      "ptri_curves link list\n",(POINTER)tri_c);
#if defined(DEBUG_TRI_GRID)
	if (debugging("recur_decomp"))
	{
	    (void) printf("This is the tri_c:\n");
	    print_a_tri_curve(tri_c);
	}
#endif /* defined(DEBUG_TRI_GRID) */
	return FUNCTION_FAILED;
}		/*end delete_tri_curve*/

LOCAL boolean find_tri_curve_from_curve(
	TRI_CURVE	*tri_curves,
	CURVE		*curve,
	TRI_CURVE	**tri_c)
{
	TRI_CURVE	*tmp_tri_c;

	for (tmp_tri_c = tri_curves; tmp_tri_c; tmp_tri_c = tmp_tri_c->next)
	{
	    if (tmp_tri_c->curve == curve)
	    {
	    	*tri_c = tmp_tri_c;
	    	return FUNCTION_SUCCEEDED;
	    }
	}
	(void) printf("WARNING in find_tri_curve_from_curve(), "
	              "curve not in tri curve list\n");
#if defined(DEBUG_TRI_GRID)
	if (debugging("recur_decomp"))
	{
	    (void) printf("\nThe following curve (%llu) "
	                  "is not in the tri_curves\n",curve_number(curve));
	    (void) printf("\nThe curve is:\n");
	    print_curve(curve);
	    print_tri_curves("\nThe tri_curves list is:\n",tri_curves);
	}
#endif /* defined(DEBUG_TRI_GRID) */
	return FUNCTION_FAILED;
}		/*end find_tri_curve_from_curve*/


LOCAL RING_POINT *insert_blk_curve_in_ring(
	TRI_CURVE	*tri_c,
	ORIENTATION	orient,
	RING_POINT	**p_ring,
	int		*num_pts_on_ring,
	TRI_GRID	*ntg)
{
	BOND		*b;
	FT_POINT		*p;
	CURVE		*c;
	RING_POINT	*first_ring;
	Locstate	st;
	int		comp, i;
	RING_POINT	Tmp_ring;

#if defined(DEBUG_TRI_GRID)
	if (debugging("ring"))
	{
	    (void) printf("\nEnter insert_blk_curve_in_ring(), "
	                  "the num_pts_on_ring = %d before insertion\n",
	    	          *num_pts_on_ring);
	}
#endif /* defined(DEBUG_TRI_GRID) */

	c = tri_c->curve;
	if ((*p_ring) == NULL)
	    (*num_pts_on_ring) = 1;
	else
	    ++(*num_pts_on_ring);

	if (orient == POSITIVE_ORIENTATION)
	{
	    p = c->start->posn; 	i = tri_c->tg_pts_index;
	    st = left_start_state(c); 	comp = negative_component(c);
	}
	else
	{
	    p = c->end->posn; 	        i = tri_c->tg_pts_index+tri_c->n_pts-1;
	    st = right_end_state(c); 	comp = positive_component(c); 
	}
	ring_point_state(&Tmp_ring) = st;

	Tmp_ring.intfc = c->interface;
	Tmp_ring.comp = comp;
	Tmp_ring.side = tri_c->side;
	Ring_point_coords(&Tmp_ring) = (double *)((ntg->front_points)+i);
#if defined(DEBUG_TRI_GRID)
	if (debugging("ring"))
	{
	    print_orientation("curve orient =",orient,"\n");
	    print_curve(c);
	}
#endif /* defined(DEBUG_TRI_GRID) */
	*p_ring = make_ring_point(*p_ring,&Tmp_ring);
	if (*p_ring == NULL)
	{
	    (void) printf("WARNING in insert_blk_curve_in_ring(), "
	                  "make_ring_point() failed\n");
	    return NULL;
	}

#if defined(DEBUG_TRI_GRID)
	if (debugging("ring"))
	{
	    (void) printf("inserted point (%g, %g), comp = %d \n",
			  Coords(p)[0],Coords(p)[1],comp);
	}
#endif /* defined(DEBUG_TRI_GRID) */

	first_ring = *p_ring;
	for (b=Bond_at_node(c,orient);
		    b!=Bond_at_node(c,Opposite_orient(orient));
		    b = Following_bond(b,orient))
	{
	    p = Point_of_bond(b,Opposite_orient(orient));
	    if (orient==POSITIVE_ORIENTATION)
	    {
	        ++i;
	        st = left_state(p);
	    }
	    else
	    {
	        --i;
	        st = right_state(p);
	    }
	    ring_point_state(&Tmp_ring) = st;
	    Ring_point_coords(&Tmp_ring) = (double *)((ntg->front_points)+i);
	    *p_ring = make_ring_point(*p_ring,&Tmp_ring);
	    if (*p_ring == NULL)
	    {
	        (void) printf("WARNING in insert_blk_curve_in_ring(), "
		              "make_ring_point() failed\n");
		return NULL;
	    }
	    ++(*num_pts_on_ring);

#if defined(DEBUG_TRI_GRID)
	    if (debugging("ring"))
	    {
	        (void) printf("inserted point (%g, %g), comp = %d \n",
			      Coords(p)[0],Coords(p)[1],comp);
	        (void) printf("\nthe num_pts_on_ring = %d after insertion\n",
			      *num_pts_on_ring);
	    }
#endif /* defined(DEBUG_TRI_GRID) */
	}
	return first_ring;
}		/*end insert_blk_curve_in_ring*/

LOCAL  void split_2d_curve_in_tri(
	BOND		*b,
	CURVE		*c,
	NODE		**n)
{
	CURVE		**c_split;
	Locstate	l_st,r_st; 
	
	l_st = left_state(b->end);      r_st = right_state(b->end);
	c_split = split_curve(b->end,b,c,
			      negative_component(c),positive_component(c),
			      negative_component(c),positive_component(c));
	left_end_state(c_split[0])    = l_st; 
	left_start_state(c_split[1])  = l_st;
	right_end_state(c_split[0])   = r_st;
	right_start_state(c_split[1]) = r_st;
	*n = c_split[0]->end;
}		/*end split_2d_curve_in_tri*/


/* 
*			same_comp_on_ring():
*
*	Check if all the points on the ring have the same component.
*/

LOCAL boolean same_comp_on_ring(
	RING_POINT	*p_ring)
{
	RING_POINT	*ring;

	for (ring = p_ring->next; ring != p_ring; ring = ring->next)
	    if (ring->comp != p_ring->comp)
	    	return NO;
	return YES;
}


LOCAL	void	print_ring_point(
	RING_POINT	*ring_pt)
{
	int		side;
	int		dim;

	if (ring_pt == NULL)
	{
	    (void) printf("NULL RING_POINT\n");
	    return;
	}
	dim = ring_pt->intfc->dim;
	(void) printf("\nring point %p: prev = %p, next = %p, position (%p) =",
		      (POINTER)ring_pt,(POINTER)ring_pt->prev,
		      (POINTER)ring_pt->next,
		      (POINTER)Ring_point_coords(ring_pt));
	print_general_vector("",Ring_point_coords(ring_pt),dim,"\n");
	(void) printf("angle = %g (%g deg), ", ring_pt->angle,
		      degrees(ring_pt->angle));

	side = ring_pt->side;
	(void) printf("side = %d ",side);

	switch(side)
	{
	case F_SIDE:
	    (void) printf("F_SIDE\n");
	    break;
	case E_SIDE:
	    (void) printf("E_SIDE\n");
	    break;
	case I_SIDE:
	    (void) printf("I_SIDE\n");
	    break;
	default:
	    (void) printf("ERROR_SIDE\n");
	}
	(void) printf("component = %d\n",ring_pt->comp);
	(void) printf("intfc = %p\n",ring_pt->intfc);

	(void) printf("ring_point_state(ring_pt)\n");
	print_state_data(ring_point_state(ring_pt),ring_pt->intfc);
}		/*end print_ring_point*/





#define	ring_positions_are_equal(rng1,rng2)				\
	(Ring_point_coords(rng1) == Ring_point_coords(rng2))

#define	ring_points_are_close(rng1,rng2,spacing,dim)			\
	(_scaled_separation(Ring_point_coords(rng1),			\
			Ring_point_coords(rng2),spacing,dim) <		\
					MIN_RING_POINT_SCALED_SEPARATION)

#define	ring_points_are_the_same(rng1,rng2,spacing,dim)			\
	(ring_positions_are_equal(rng1,rng2) ||				\
		ring_points_are_close(rng1,rng2,spacing,dim)		)

LOCAL	boolean cut_a_piece(
	CURVE		***c_list,
	NODE		***n_list,
	ORIENTATION	bc_orient,
	CURVE		*bc,
	CUT_A_PIECE	*cpiece,
	ORIENTATION	*orient)
{
	CURVE		*c;
	NODE		*n;

	if (bc == NULL)
	{
	    screen("ERROR in cut_a_piece(), bc == NULL\n");
	    (void) printf("current interface\n");
	    print_interface(current_interface());
	    clean_up(ERROR);
	}
	c = bc; 			*orient = bc_orient;
	*c_list = NULL; 		*n_list = NULL; 	

	/* cut a piece from the intfc */

	n = Node_of(c,Opposite_orient(*orient)); 
	
#if defined(DEBUG_TRI_GRID)
	if (debugging("break_hole"))
	{
	    (void) printf("\nThe start boundary curve is:\n");
	    print_curve_with_orient(c,*orient);
	}
#endif /* defined(DEBUG_TRI_GRID) */

	if (!add_to_pointers(bc,c_list) || !add_to_pointers(n,n_list))
	{
	    screen("ERROR in cut_a_piece(), add_to_pointers() failed\n");
	    clean_up(ERROR);
	    return FUNCTION_FAILED;
	}

	if (insert_curve_in_xylist(c,Opposite_orient(*orient),cpiece) !=
							FUNCTION_SUCCEEDED)
	{
	    (void) printf("WARNING in cut_a_piece(), "
	                  "insert_curve_in_xylist() failed\n");
	    return FUNCTION_FAILED;
	}

	while((c=adjacent_curve(c,*orient,CLOCKWISE,orient)) != bc)
	{
	    if (c == NULL)
	    {
	        screen("ERROR in cut_a_piece(), "
		       "adjacent_curve returned NULL\n");
	        (void) printf("bc->interface\n");
	        print_interface(bc->interface);
	        clean_up(ERROR);
	    }
#if defined(DEBUG_TRI_GRID)
	    if (debugging("break_hole"))
	    {
	    	(void) printf("\nThe adjacent curve is:\n");
	    	print_curve_with_orient(c,*orient);
	    }
#endif /* defined(DEBUG_TRI_GRID) */

	    n = Node_of(c,*orient);
	    if (!add_to_pointers(c,c_list) || !add_to_pointers(n,n_list))
	    {
	    	screen("ERROR in cut_a_piece(), add_to_pointers() failed\n");
	    	clean_up(ERROR);
	    	return FUNCTION_FAILED;
	    }
	    if (!insert_curve_in_xylist(c,*orient,cpiece))
	    {
		(void) printf("WARNING in cut_a_piece(), "
		              "insert_curve_in_xylist() failed\n");
		return FUNCTION_FAILED;
	    }
	    *orient = Opposite_orient(*orient);
	}	
	return FUNCTION_SUCCEEDED;
}		/*end cut_a_piece*/


LOCAL  boolean set_nodes_for_make_hole_cut(
	NODE		**n1,
	NODE		**n2,
	NODE		**piece_n_list,
	NODE		**hole_n_list,
	CURVE		**piece_c_list,
	CURVE		**hole_c_list,
	INTERFACE	*intfc)
{
	CURVE	 	**pc1, **pc2, *c1, *c2;
	NODE		**pn1, **pn2;
	BOND 		*b1, *b2;

	*n1 =*n2 = NULL;

 	/* insertion step 1: try insert a curve    */
	/* between a node in piece and a node in hole*/

	for (pn1 = piece_n_list; pn1 && *pn1; ++pn1)
	{
	    for (pn2 = hole_n_list; pn2 && *pn2; ++pn2)
	    {
	        if (is_cross_in_inserted_curve((*pn1)->posn,
					       (*pn2)->posn,intfc) == YES)
		    continue;
#if defined(DEBUG_TRI_GRID)
	        if (debugging("break_hole"))
	        {
	            (void) printf("A curve is inserted between a node in ");
	            (void) printf("piece && a node in hole\n");
	        }
#endif /* defined(DEBUG_TRI_GRID) */
	        *n1 =*pn1; 		*n2 =*pn2;
	        return FUNCTION_SUCCEEDED;
	    }
	}

		/*    insertion step 2: try to insert a curve  */
		/* between a point in piece and a node in hole*/

	for (pc1 = piece_c_list; pc1 && *pc1; ++pc1)
	{
	    for (b1 = (*pc1)->first; b1 != (*pc1)->last; b1 = b1->next)
	    {
	        for (pn2 = hole_n_list; pn2 && *pn2; ++pn2)
	        {
	            if (is_cross_in_inserted_curve(b1->end,(*pn2)->posn,
						   intfc) == YES)
			continue;
#if defined(DEBUG_TRI_GRID)
	            if (debugging("break_hole"))
	            {
			(void) printf("A curve is inserted between a ");
			(void) printf("bond in piece and a node in hole\n");
	            }
#endif /* defined(DEBUG_TRI_GRID) */
	            *n2 =*pn2; 		c1 =*pc1;
	            split_2d_curve_in_tri(b1,c1,n1);
	            return FUNCTION_SUCCEEDED;
	        }
	    }
	}

		/*   insertion step 3: try to insert a curve   */
		/* between a node in piece and a piece in hole*/

	for (pn1 = piece_n_list; pn1 && *pn1; ++pn1)
	{
	    for (pc2 = hole_c_list; pc2 && *pc2; ++pc2)
	    {
	        for (b2=(*pc2)->first; b2 != (*pc2)->last; b2=b2->next)
	        {
	            if (is_cross_in_inserted_curve((*pn1)->posn,b2->end,
						   intfc) == YES)
	                continue;
#if defined(DEBUG_TRI_GRID)
	            if (debugging("break_hole"))
	            {
			(void) printf("A curve is inserted between a node "
			              "in piece and a bond in hole\n");
	            }
#endif /* defined(DEBUG_TRI_GRID) */
	            *n1 =*pn1;

	            c2 =*pc2;
	            split_2d_curve_in_tri(b2,c2,n2);
	            return FUNCTION_SUCCEEDED;
	        }
	    }
	}

	/*    insertion step 4: try to insert a curve   */
	/* between a point in piece and a piece in hole*/

	for (pc1 = piece_c_list; pc1 && *pc1; ++pc1)
	{
	    for (b1 = (*pc1)->first; b1 != (*pc1)->last; b1 = b1->next)
	    {
	        for (pc2 = hole_c_list; pc2 && *pc2; ++pc2)
	        {
	            for (b2=(*pc2)->first; b2!=(*pc2)->last; b2=b2->next)
	            {
	                if (is_cross_in_inserted_curve(b1->end,b2->end,
						       intfc) == YES)
			    continue;
#if defined(DEBUG_TRI_GRID)
	                if (debugging("break_hole"))
	                {
			   (void) printf("A curve is inserted between a ");
			   (void) printf("bond in piece and a bond in hole\n");
	                }
#endif /* defined(DEBUG_TRI_GRID) */
	                c1 =*pc1;	c2 =*pc2;
	            	split_2d_curve_in_tri(b1,c1,n1);
	            	split_2d_curve_in_tri(b2,c2,n2);
	                return FUNCTION_SUCCEEDED;
	            }
	        }
	    }
	}

#if defined(DEBUG_TRI_GRID)
	if (debugging("break_hole"))
	{
	    (void) printf("Can not insert curve between piece and hole.\n"
	                  "Some larger hole must be between them. "
	                  "Search for it.\n");
	}
#endif /* defined(DEBUG_TRI_GRID) */

	return FUNCTION_FAILED;
}		/*end set_nodes_for_make_hole_cut*/

/*
*	 	is_cross_in_inserted_curve():
*
*	This routine checks the possibility that the inserted curve
*	between piece and hole cuts any curves on the block interface.
*
*/

LOCAL	boolean is_cross_in_inserted_curve(
	FT_POINT		*p1,
	FT_POINT		*p2,
	INTERFACE	*blk_intfc)
{
	int		dim = blk_intfc->dim;
	CURVE		**c;
	BOND		*b;

#if defined(DEBUG_TRI_GRID)
	if (debugging("break_hole"))
	{
	    int i;
	    (void) printf("Enter is_cross_in_inserted_curve()\n");
	    (void) printf("Check crossing for curve: ");
	    for (i = 0; i < dim; ++i)
	    	(void) printf("%g ",Coords(p1)[i]);
	    (void) printf("-> ");
	    for (i = 0; i < dim; ++i)
	    	(void) printf("%g ",Coords(p2)[i]);
	    (void) printf("\n");
	}
#endif /* defined(DEBUG_TRI_GRID) */
		
	for (c = blk_intfc->curves; c && *c; ++c)
	{
	    for (b = (*c)->first; b != NULL; b = b->next)
	    {
	    	if (is_cross(p1,p2,b,dim) == YES)
	    	{
#if defined(DEBUG_TRI_GRID)
	    	    if (debugging("break_hole"))
	    	    {
	    		(void) printf("A crossing is found on bond %llu, "
				      "curve %llu\n",
				      bond_number(b,(*c)->interface),
	    			      curve_number(*c));
	    		print_bond(b);
	    		print_curve(*c);
	    	    }
#endif /* defined(DEBUG_TRI_GRID) */
	    		return YES;
	    	}
	    }
	}
	return NO;
}		/*end is_cross_in_inserted_curve*/

/* 
*			is_cross():
*
*	Checks if the inserted line (p1,p2) intersects with b.
*
*	This function assumes the line segment (p1,p2) is
*	coplanar with the bond b.
*
*/ 

LOCAL	boolean is_cross(
	FT_POINT		*p1,
	FT_POINT		*p2,
	BOND		*b,
	int		dim)
{
	int		i;
	double		t[3], v1[3], v2[3], d1[3], d2[3];
	double		dot_prod;

	if ((p1==b->start) || (p1==b->end) || (p2==b->start) || (p2==b->end))
	    return NO;

	for (i = 0; i < dim; ++i)
	{
	    t[i] = Coords(b->end)[i] - Coords(b->start)[i];
	    v1[i] = Coords(p1)[i] - Coords(b->start)[i];
	    v2[i] = Coords(p2)[i] - Coords(b->start)[i];
	}

	dot_prod = 0.0;
	for (i = 0; i < dim; ++i)
	{
	    d1[i] = v1[(i+1)%dim]*t[(i+2)%dim] - v1[(i+2)%dim]*t[(i+1)%dim];
	    d2[i] = v2[(i+1)%dim]*t[(i+2)%dim] - v2[(i+2)%dim]*t[(i+1)%dim];
	    dot_prod += d1[i]*d2[i];
	}
	if (dot_prod > 0.0) return NO;

	for (i = 0; i < dim; ++i)
	{
	    t[i] = Coords(p2)[i] - Coords(p1)[i];
	    v1[i] = Coords(b->start)[i] - Coords(p1)[i];
	    v2[i] = Coords(b->end)[i] - Coords(p1)[i];
	}

	dot_prod = 0.0;
	for (i = 0; i < dim; ++i)
	{
	    d1[i] = v1[(i+1)%dim]*t[(i+2)%dim] - v1[(i+2)%dim]*t[(i+1)%dim];
	    d2[i] = v2[(i+1)%dim]*t[(i+2)%dim] - v2[(i+2)%dim]*t[(i+1)%dim];
	    dot_prod += d1[i]*d2[i];
	}


	if (dot_prod > 0.0)
	    return NO;

	return YES;
}		/*end is_cross*/


/*
* 			recursive_triangulate():
*
*	This is a recursive routine for triangulating points on a block 
*	interface. 
*		
*	The recursive procedure is:
*
*	1)	If there only 3 points on the polygon, 
*			then this is one of the triangles, set_triangle.
*		otherwise
*			do following. 
*
*	2) 	Find the corner with largest angle in the polygon.
*		The polygon used here is not ncessary convex.
*
*	3)	Find the split point on the polygon which will divide
*		the largest angle as equally as possible.
*
*	4)      Split the polygon into two smaller polygons. 
*
*	5)      Triangulate these two polygons recursively.
*
*/

LOCAL boolean recursive_triangulate(
	RECUR_TRI	*recur_tri,
	RING_POINT	**p_ring,
	int		num_pts_on_ring,
	double		ls)
{
	RING_POINT	*r1, *r2, *r3, *split_ring;
	int		num;

#if defined(DEBUG_TRI_GRID)
	debug_print("recur_tri","Enter recursive_triangulate()\n");
	if (debugging("recur_tri"))
	{
	    (void) printf("the number of points on ring =  %d\n",
			  num_pts_on_ring);
	    (void) printf("The ring which enters recursive_triangulate()\n"); 
	    print_ring(*p_ring);
	}
#endif /* defined(DEBUG_TRI_GRID) */

	if (is_excluded_comp((*p_ring)->comp,(*p_ring)->intfc))
	{
#if defined(DEBUG_TRI_GRID)
            if (debugging("recur_tri"))
	    {
	        (void) printf("Not triangulating excluded comp on "
		              "RING_POINT %p\n",*p_ring);
	    }
	    debug_print("recur_tri","Left recursive_triangulate() = GOOD\n");
#endif /* defined(DEBUG_TRI_GRID) */
	    *p_ring = NULL;
	    return FUNCTION_SUCCEEDED;
	}

	if (num_pts_on_ring < 3)
	{
	    (void) printf("WARNING in recursive_triangulate(), "
	                  "num_pts_on_ring (%d) less then 3\n",num_pts_on_ring);
#if defined(DEBUG_TRI_GRID)
	    debug_print("recur_tri","Left recursive_triangulate() = BAD\n");
#endif /* defined(DEBUG_TRI_GRID) */
	    return FUNCTION_FAILED;
	}

	if (num_pts_on_ring == 3)
	{
	    r1 = (*p_ring)->prev; r2 = (*p_ring); r3 = (*p_ring)->next;
	    Generate_tri(r1,r2,r3,recur_tri);
	    *p_ring = NULL;

#if defined(DEBUG_TRI_GRID)
	    debug_print("recur_tri","Left recursive_triangulate() = GOOD\n");
#endif /* defined(DEBUG_TRI_GRID) */
	    return FUNCTION_SUCCEEDED;
	}

	if (find_largest_angle_and_split(p_ring,&split_ring,&num,
					 recur_tri->dim,recur_tri->spacing,
					 recur_tri->side_value,recur_tri->nor,
					 ls) != FUNCTION_SUCCEEDED)
		return FUNCTION_FAILED;

#if defined(DEBUG_TRI_GRID)
	if (debugging("recur_tri"))
	{
	    (void) printf("\nThe ring1 after splitting in "
		          "recursive_triangulate()\n"); 
	    print_ring(*p_ring);
	    (void) printf("\nThe ring2 after splitting in "
	                  "recursive_triangulate()\n"); 
	    print_ring(split_ring);
 	}
#endif /* defined(DEBUG_TRI_GRID) */

	if (*p_ring && !recursive_triangulate(recur_tri,p_ring,num,ls))
	{
#if defined(DEBUG_TRI_GRID)
	    debug_print("recur_tri","Left recursive_triangulate() = BAD\n");
#endif /* defined(DEBUG_TRI_GRID) */
	    return FUNCTION_FAILED;
	}

	if (split_ring && !recursive_triangulate(recur_tri,&split_ring,
						    num_pts_on_ring-num+2,ls))
	{
#if defined(DEBUG_TRI_GRID)
	    debug_print("recur_tri","Left recursive_triangulate() = BAD\n");
#endif /* defined(DEBUG_TRI_GRID) */
	    return FUNCTION_FAILED;
	}
	else
	{
#if defined(DEBUG_TRI_GRID)
	    debug_print("recur_tri","Left recursive_triangulate() = GOOD\n");
#endif /* defined(DEBUG_TRI_GRID) */
	    return FUNCTION_SUCCEEDED;
	}
}		/*end recursive_triangulate*/

/* 
* 		find_largest_angle_and_split():
*
*	This routine finds the corner of a polygon which has the largest 
*	angle in the polygon. Then it finds the split point on the polygon 
*	which will divide the largest angle as equally as possible. Finally
* 	it splits the polygon into two smaller polygons. 
*
*/

LOCAL boolean find_largest_angle_and_split(
	RING_POINT	 **p_ring,
	RING_POINT	 **split_ring,
	int		 *num,
	int		 dim,
	double		 *spacing,
	LIN_EL_FACE_TYPE side,
	double		 *nor,
	double		 ls)
{
	RING_POINT	*ring, *ring_in;
	RING_POINT	*new1_ring, *new2_ring, **split_candidate;
	RING_POINT	*rng1;/* RING_POINT with largest angle *p_ring = rng1 */
	RING_POINT	*splt_rng;
	double		angle, ang[4];
	int		i;
	static RING_POINT Data;
	
#if defined(DEBUG_TRI_GRID)
	debug_print("split_ring","Entered find_largest_angle_and_split()\n");
#endif /* defined(DEBUG_TRI_GRID) */

	*split_ring = splt_rng = NULL;		split_candidate = NULL;
	rng1 = ring_in =*p_ring;		angle = rng1->angle;

	/* search for the largest angle*/
	for (ring = rng1->next; ring && (ring != ring_in); ring=ring->next)
	{
	    if (ring->angle > angle)
	    {
	    	angle = ring->angle;
		rng1 = ring;
	    }
	}
	*p_ring = rng1;

#if defined(DEBUG_TRI_GRID)
	if (debugging("split_ring"))
	{
	    (void) printf("RING_POINT with the largest angle\n");
	    print_ring_point(rng1);
	    (void) printf("Ring to be split\n");	print_ring(rng1);
	}
#endif /* defined(DEBUG_TRI_GRID) */


	for (ring = rng1->next->next; ring->next != rng1; ring = ring->next)
	{

	    if (ring_points_are_the_same(ring,rng1,spacing,dim))
	    	continue;
			 
	    ang[0] = compute_ang_at_ring_pt(ring,rng1,rng1->next,PLUS_CUT,
					    dim,nor);
	    ang[1] = rng1->angle - ang[0];
	    if (ang[1] <= 0.0) 	continue;

	    ang[2] = compute_ang_at_ring_pt(rng1,ring,ring->next,PLUS_CUT,
					    dim,nor);
	    ang[3] = ring->angle - ang[2];
	    if (ang[3] <= 0.0) 	continue;

	    if (is_intersect_with_other_edge(rng1,ring,dim,ls) == YES)
	    	continue;

	    ring->min_angle = ang[0];
	    for (i = 1; i < 4; ++i)
	    	if (ang[i] < ring->min_angle) ring->min_angle = ang[i];
	
#if defined(DEBUG_TRI_GRID)
	    if (debugging("split_ring"))
	    {
	    	(void) printf("Split ring candidate found\n");
	    	print_ring_point(ring);
	    	for (i = 0; i < 4; ++i)
	    	    (void) printf("ang[%d] = %g (%g deg)\n",i,ang[i],
				  degrees(ang[i]));
		(void) printf("min_angle  = %g\n",ring->min_angle);
	    }
#endif /* defined(DEBUG_TRI_GRID) */

	    if (!add_to_pointers(ring,&split_candidate))
	    {
	    	screen("ERROR in find_largest_angle_and_split(), "
	    	       "add_to_pointers() failed\n");
	    	clean_up(ERROR);
	    }
	}

#if defined(DEBUG_TRI_GRID)
	if (debugging("split_ring"))
	{
	   if (split_candidate == NULL || *split_candidate == NULL)
	   { 
		(void) printf("No split ring candidate found\n");
		for (ring = rng1->next->next; ring->next != rng1; 
		    ring = ring->next) 
		{
		    print_ring_point(ring);
		    (void) printf("scaled_separation(Ring_point_coords(ring), "
		                  "Ring_point_coords(rng1)) = %g,",
				  _scaled_separation(Ring_point_coords(ring),
						     Ring_point_coords(rng1),
						     spacing,dim));
		    (void) printf("MIN_RING_POINT_SCALED_SEPARATION = %g\n",
				   MIN_RING_POINT_SCALED_SEPARATION);
		    ang[0] = compute_ang_at_ring_pt(ring,rng1,rng1->next,
							PLUS_CUT,dim,nor);
		    ang[1] = rng1->angle - ang[0];
		    ang[2] = compute_ang_at_ring_pt(rng1,ring,ring->next,
							PLUS_CUT,dim,nor);
		    ang[3] = ring->angle - ang[2];
		    for (i = 0; i < 4; ++i)
		   	 (void) printf("ang[%d] = %g, ",i,ang[i]);
		    (void) printf("\nintersects with other edge = %s\n",
		                  y_or_n(is_intersect_with_other_edge(rng1,
				                                      ring,dim,
								      ls)));
		     (void) printf("\n\n");
		}
	    }
	}
#endif /* defined(DEBUG_TRI_GRID) */

	while(split_candidate && *split_candidate)
	{
	    splt_rng = largest_min_angle(split_candidate); 
	    if (!is_intersect_with_other_edge(rng1,splt_rng,dim,ls))
	    {
	        *split_ring = splt_rng;
		break;
	    } 
	    if (!delete_from_pointers(splt_rng,&split_candidate)) 
	    {
	    	screen("ERROR in find_largest_angle_and_split(), "
	    	       "delete_from_pointers() failed\n");
	    	clean_up(ERROR);
	    }
	}

	if ((*split_ring == NULL) && 
		((rng1->next->next->next->next == rng1 && 
		    ring_positions_are_equal(rng1,rng1->next->next))
	 || 
		    ring_points_are_close(rng1,rng1->next->next,spacing,dim))
	)
	    *split_ring = rng1->next->next;

	if (*split_ring == NULL)
	{
	    (void) printf("WARNING in find_largest_angle_and_split(), "
	                  "unable to find split_ring\n");
	    return FUNCTION_FAILED;
	}

	for (ring = rng1, i = 1;
	     ((ring!=rng1) || (i==1)) && (ring!=NULL) && (ring!=splt_rng);
	     ring = ring->next, ++i); 
	if (ring != splt_rng)
	{
	    INTERFACE *intfc = current_interface();
	    screen("ERROR in find_largest_angle_and_split(), "
		   "splt_rng not in ring list\n");
	    if (ring == NULL)
		(void) printf("End of list reached without finding splt_rng\n");
	    (void) printf("Head of loop, rng1\n");
	    print_ring_point(rng1);
	    (void) printf("splt_rng\n");
	    print_ring_point(splt_rng);
	    (void) printf("current interface\n");
	    print_interface(intfc);
	    clean_up(ERROR);
	}
	*num = i;

#if defined(DEBUG_TRI_GRID)
	if (debugging("split_ring"))
	{
	    (void) printf("The split ring point\n");
	    print_ring_point(splt_rng);
	}
#endif /* defined(DEBUG_TRI_GRID) */

	/*split ring into two rings*/

	new1_ring = NULL;		new2_ring = NULL;

	Data = *rng1;
	Data.side = side;
	new1_ring = make_ring_point(NULL,&Data);
	if (new1_ring == NULL)
	{
	    (void) printf("WARNING in find_largest_angle_and_split(), "
	                  "make_ring_point() failed\n");
	    return FUNCTION_FAILED;
	}

	Data = *splt_rng;

	Data.side = side;
	new2_ring = make_ring_point(NULL,&Data);
	if (new2_ring == NULL)
	{
	    (void) printf("WARNING in find_largest_angle_and_split(), "
	                  "make_ring_point() failed\n");
	    return FUNCTION_FAILED;
	}

	new1_ring->next = splt_rng;	new1_ring->prev = rng1->prev;
	if (new1_ring->prev) new1_ring->prev->next = new1_ring;

	new2_ring->next = rng1;		   new2_ring->prev = splt_rng->prev;
	if (new2_ring->prev) new2_ring->prev->next = new2_ring;
	rng1->prev = new2_ring;
	splt_rng->prev = new1_ring;

	rng1->angle = compute_ang_at_ring_pt(rng1->prev,rng1,rng1->next,
					     PLUS_CUT,dim,nor);
	rng1->prev->angle = compute_ang_at_ring_pt(rng1->prev->prev,rng1->prev,
					           rng1,PLUS_CUT,dim,nor);
	splt_rng->angle = compute_ang_at_ring_pt(splt_rng->prev,splt_rng,
					         splt_rng->next,PLUS_CUT,
						 dim,nor);
	splt_rng->prev->angle = compute_ang_at_ring_pt(splt_rng->prev->prev,
		    			               splt_rng->prev,splt_rng,
					               PLUS_CUT,dim,nor);
	
#if defined(DEBUG_TRI_GRID)
	debug_print("split_ring","Leave find_largest_angle_and_split()\n");
#endif /* defined(DEBUG_TRI_GRID) */

	return FUNCTION_SUCCEEDED;
}		/*end find_largest_angle_and_split*/

LOCAL RING_POINT *largest_min_angle(
	RING_POINT	**split_candidate)
{
	RING_POINT	**ring, *largest_min;

	for (ring = split_candidate, largest_min = *split_candidate; 
						ring && *ring; ++ring)
	{
	    if ((*ring)->min_angle > largest_min->min_angle)
		largest_min =*ring;
	}
	return largest_min;
}		/*end largest_min_angle*/

LOCAL boolean is_intersect_with_other_edge(
	RING_POINT	*p_ring,
	RING_POINT	*split_ring,
	int		dim,
	double		ls)
{
	RING_POINT	*ring;
	FT_POINT		cross_point;
	int		i;
	static	BOND	b1, b2;
	static	FT_POINT	P1, P2, P3, P4;
	static	FT_POINT	*p1 = &P1, *p2 = &P2, *p3 = &P3, *p4 = &P4;
	static	boolean	first = YES;

	if (first == YES)
	{
	    first = NO;
	    b1.start = p1;		b1.end = p2;
	    b2.start = p3;		b2.end = p4;
	}

	for (i = 0; i < dim; ++i)
	{
	    Coords(p1)[i] = Ring_point_coords(p_ring)[i];
	    Coords(p2)[i] = Ring_point_coords(split_ring)[i];
	}
	set_bond_length(&b1,dim);
	for (ring = p_ring->next; ring->next != p_ring ;ring = ring->next)
	{
	    if (ring == split_ring || ring->next == split_ring)
		continue;
	    for (i = 0; i < dim; ++i)
	    {
	    	Coords(p3)[i] = Ring_point_coords(ring)[i];
	    	Coords(p4)[i] = Ring_point_coords(ring->next)[i];
	    }
	    set_bond_length(&b2,dim);

	    if ((separation(p1,p3,dim) < MIN_RING_POINT_SEPARATION*ls) ||
	    	(separation(p1,p4,dim) < MIN_RING_POINT_SEPARATION*ls) ||
	       	(separation(p2,p3,dim) < MIN_RING_POINT_SEPARATION*ls) ||
	    	(separation(p2,p4,dim) < MIN_RING_POINT_SEPARATION*ls))
	    	continue;

	    if (cross_bonds(&b1,&b2,&cross_point))
	        return YES;
	}
	return NO;
}		/*end is_intersect_with_other_edge*/


LOCAL	RING_POINT *make_ring_point(
	RING_POINT	*p_ring,
	RING_POINT	*data)
{
	RING_POINT	*new_ring_point = NULL;

	if (data == NULL)
	{
	    (void) printf("WARNING in make_ring_point(), data == NULL\n");
	    return NULL;
	}

	if ((new_ring_point = GetNextRingPoint()) == NULL)
	{
	    (void) printf("WARNING in i_make_ring_point(), "
	                  "Can't get next ring point\n");
	    return NULL;
	}

	if (p_ring != NULL)
	    p_ring->next = new_ring_point;

	new_ring_point->prev = p_ring;
	new_ring_point->next = NULL;
	new_ring_point->side = data->side;
	new_ring_point->angle = 0.0; 
	new_ring_point->min_angle = HUGE_VAL;
	new_ring_point->intfc = data->intfc;

	Ring_point_coords(new_ring_point) = Ring_point_coords(data);  
	new_ring_point->comp = data->comp; 
	ring_point_state(new_ring_point) = ring_point_state(data);
	return new_ring_point;
}		/*end make_ring_point*/



LOCAL  void delete_small_separation_ring_points(
	RING_POINT	**p_ring,
	int		*num_pts_on_ring,
	double		*h,
	int		dim)
{
	RING_POINT	*ring;
	int		i;

#if defined(DEBUG_TRI_GRID)
	debug_print("ring_separation",
	      "Entered delete_small_separation_ring_points()\n");
	if (debugging("ring_separation"))
	{
	    print_ring(*p_ring);
	    (void) printf("num_pts_on_ring = %d\n",*num_pts_on_ring);
	    print_general_vector("h = ",h,dim,"\n");
	    (void) printf("MIN_RING_POINT_SEPARATION = %g\n",
			   MIN_RING_POINT_SEPARATION);
	    (void) printf("MIN_RING_POINT_SEPARATION = %g\n",
			   MIN_RING_POINT_SEPARATION);
	}
#endif /* defined(DEBUG_TRI_GRID) */

	for (ring = (*p_ring)->next; ; ring = ring->next)
	{
	    if (ring_points_are_close(ring,ring->prev,h,dim))
	    {
#if defined(DEBUG_TRI_GRID)
	        if (debugging("ring_separation"))
	        {
	            (void) printf("The following are too close:\n");
	            print_ring_point(ring->prev);
	            print_ring_point(ring);
	            (void) printf("They will be merged\n");
	        }
#endif /* defined(DEBUG_TRI_GRID) */
	        if (*num_pts_on_ring == 3)
	        {
	            *num_pts_on_ring = 0;
		    *p_ring = NULL;
#if defined(DEBUG_TRI_GRID)
	            if (debugging("ring_separation"))
	            {
	                (void) printf("The last three points have "
	                              "being deleted from ring:\n");
	                print_ring_point(ring->prev);
	                print_ring_point(ring);
	                print_ring_point(ring->next);
	                (void) printf("Left delete_small_separation_"
				      "ring_points() with p_ring == NULL\n");
	            }
#endif /* defined(DEBUG_TRI_GRID) */
	            return;
	        }

	        (*num_pts_on_ring)--;
	        for (i = 0; i < dim; ++i)
	        {
	    	    Ring_point_coords(ring->prev)[i] =
	                0.5 * (Ring_point_coords(ring)[i] +
			       Ring_point_coords(ring->prev)[i]);
	        }
	        ring->prev->next = ring->next;
	        ring->next->prev = ring->prev;
	        if (ring == *p_ring)
		    *p_ring = ring->prev;
	    }
	    if (ring ==*p_ring) 
	    {
#if defined(DEBUG_TRI_GRID)
	        if (debugging("ring_separation"))
	        {
	            (void) printf("The ring leaving delete_small_"
	                          "separation_ring_points()\n");
	            print_ring(*p_ring);
	            (void) printf("num_pts_on_ring=%d\n",*num_pts_on_ring);
	            (void) printf("Left delete_small_"
	                          "separation_ring_points()\n");
	        }
#endif /* defined(DEBUG_TRI_GRID) */
	        return;
	    }
	}
} 		/*end delete_small_separation_ring_points*/


LOCAL	void	evaluate_angles(
	RING_POINT	*p_ring,
	double		*nor)
{
	RING_POINT	*ring;
	int		dim = p_ring->intfc->dim;

	debug_print("evaluate_angle","Entered evaluate_angles()\n");

	p_ring->angle = compute_ang_at_ring_pt(p_ring->prev,p_ring,p_ring->next,
				               PLUS_CUT,dim,nor);
	ring = p_ring;

	if (debugging("ring"))
	{
	     print_general_vector("Ring_point_coords(ring = ",
			          Ring_point_coords(ring),dim,"");
	    print_angle(", angle = ",ring->angle,"\n");
	}

	while((ring = ring->next) != p_ring)
	{
	    ring->angle = compute_ang_at_ring_pt(ring->prev,ring,
					         ring->next,PLUS_CUT,dim,nor);

	}
	debug_print("evaluate_angle","Left evaluate_angles()\n");
}		/*end evaluate_angles*/



LOCAL	TRI_CURVE_STORE	*TrCrStrList = NULL, *CurTrCrStr = NULL;

LOCAL	TRI_CURVE *GetNextTriCurve(void)
{
	TRI_CURVE	*new_tri_curve;
	if (CurTrCrStr == NULL)
	{
	    TrCrStrList = AllocTrCrStrStruct();
	    if (TrCrStrList == NULL)
	    {
	    	(void) printf("WARNING in GetNextTriCurve(), "
	    	              "can't get next ring point\n");
	    	return NULL;
	    }
	    CurTrCrStr = TrCrStrList;
	}
	else if (CurTrCrStr->NextFree == CurTrCrStr->TrCrStrLen)
	{
	    if (CurTrCrStr->next == NULL)
	    {
	    	CurTrCrStr->next = AllocTrCrStrStruct();
	    	if (CurTrCrStr->next == NULL)
	    	{
	    	    (void) printf("WARNING in GetNextTriCurve(), "
	    	                  "can't get next ring point\n");
	    	    return NULL;
	    	}
	    }
	    CurTrCrStr = CurTrCrStr->next;
	}
	new_tri_curve = CurTrCrStr->TrCrStr + CurTrCrStr->NextFree++;
	zero_scalar(new_tri_curve,sizeof(TRI_CURVE));
	return new_tri_curve;
}		/*end GetNextTriCurve*/

LOCAL   TRI_CURVE_STORE        *AllocTrCrStrStruct(void)
{
	TRI_CURVE_STORE *rps;

	scalar(&rps,sizeof(TRI_CURVE_STORE));
	rps->NextFree = 0;
	rps->next = NULL;
	rps->TrCrStrLen = 100;/*TOLERANCE*/
	uni_array(&rps->TrCrStr,rps->TrCrStrLen,sizeof(TRI_CURVE));
	return rps;
}		/*end AllocTrCrStrStruct*/

LOCAL	void FreeTriCurveList(void)
{
	TRI_CURVE_STORE *rps;

	for (rps = TrCrStrList; rps != NULL; rps = rps->next)
	{
	    zero_scalar(rps->TrCrStr,rps->TrCrStrLen*sizeof(TRI_CURVE));
	    rps->NextFree = 0;
	}
	CurTrCrStr = TrCrStrList;
}		/*end FreeTriCurveList*/


LOCAL	RING_POINT_STORE	*RgPtStrList = NULL, *CurRgPtStr = NULL;

LOCAL	RING_POINT *GetNextRingPoint(void)
{
	RING_POINT	*new_ring_point;
	if (CurRgPtStr == NULL)
	{
	    RgPtStrList = AllocRgPtStrStruct();
	    if (RgPtStrList == NULL)
	    {
	    	(void) printf("WARNING in GetNextRingPoint(), "
	    	              "can't get next ring point\n");
	    	return NULL;
	    }
	    CurRgPtStr = RgPtStrList;
	}
	else if (CurRgPtStr->NextFree == CurRgPtStr->RgPtStrLen)
	{
	    if (CurRgPtStr->next == NULL)
	    {
	    	CurRgPtStr->next = AllocRgPtStrStruct();
	    	if (CurRgPtStr->next == NULL)
	    	{
	    	    (void) printf("WARNING in GetNextRingPoint(), "
	    	                  "can't get next ring point\n");
	    	    return NULL;
	    	}
	    }
	    CurRgPtStr = CurRgPtStr->next;
	}
	new_ring_point = CurRgPtStr->RgPtStr + CurRgPtStr->NextFree++;
	zero_scalar(new_ring_point,sizeof(RING_POINT));
	return new_ring_point;
}		/*end GetNextRingPoint*/

LOCAL   RING_POINT_STORE        *AllocRgPtStrStruct(void)
{
	RING_POINT_STORE *rps;

	scalar(&rps,sizeof(RING_POINT_STORE));
	rps->NextFree = 0;
	rps->next = NULL;
	rps->RgPtStrLen = 100;/*TOLERANCE*/
	uni_array(&rps->RgPtStr,rps->RgPtStrLen,sizeof(RING_POINT));
	return rps;
}		/*end AllocRgPtStrStruct*/

LOCAL	void FreeRingPointList(void)
{
	RING_POINT_STORE *rps;

	for (rps = RgPtStrList; rps != NULL; rps = rps->next)
	{
	    zero_scalar(rps->RgPtStr,rps->RgPtStrLen*sizeof(RING_POINT));
	    rps->NextFree = 0;
	}
	CurRgPtStr = RgPtStrList;
}		/*end FreeRingPointList*/

LOCAL	void	strip_excluded_boundaries(
	INTERFACE *blk_intfc,
	TRI_GRID  *ntg)
{
	NODE  **n, **delete_nodes;
	CURVE **c, **delete_curves;
	delete_curves = NULL;
	for (c = blk_intfc->curves; c && *c; ++c)
	{
	    if (is_bdry(*c) &&
	        ( (is_excluded_comp(positive_component(*c),blk_intfc) &&
	           is_exterior_comp(negative_component(*c),blk_intfc)) ) ||
	        ( (is_excluded_comp(negative_component(*c),blk_intfc) &&
	           is_exterior_comp(positive_component(*c),blk_intfc)) ) )
	    {
	        if (!add_to_pointers(*c,&delete_curves))
		{
		    screen("ERROR in strip_excluded_boundaries(), "
		           "add_to_pointers() for delete_curves failed\n");
	            (void) printf("blk_intfc\n");
	            print_interface(blk_intfc);
	            (void) printf("ntg->grid_intfc\n");
	            print_interface(ntg->grid_intfc);
		    clean_up(ERROR);
		}
	    }
	}
	for (c = delete_curves; c && *c; ++c)
	    (void) delete_curve(*c);
	delete_nodes = NULL;
	for (n = blk_intfc->nodes; n && *n; ++n)
	{
	    if ((!is_source_sink_node(*n)) &&
	        ((*n)->in_curves == NULL) && ((*n)->out_curves == NULL))
	    {
	        if (!add_to_pointers(*n,&delete_nodes))
	        {
	            screen("ERROR in strip_excluded_boundaries(), "
		           "add_to_pointers() for delete_nodes failed\n");
	            (void) printf("blk_intfc\n");
	            print_interface(blk_intfc);
	            (void) printf("ntg->grid_intfc\n");
	            print_interface(ntg->grid_intfc);
		    clean_up(ERROR);
	        }
	    }
	}
	for (n = delete_nodes; n && *n; ++n)
	    (void) delete_node(*n);
}		/*end strip_excluded_boundaries*/

#if defined(DEBUG_TRI_GRID)

LOCAL void print_xylist(
	XY		*xy_start,
	XY		*xy_end)
{
	XY		*pxy;

	if (!xy_start || !xy_end) return;

	(void) printf("xy_start = %p,        xy_end = %p\n",
		      (POINTER)xy_start,(POINTER)xy_end);
	for (pxy = xy_start; pxy != NULL; pxy = pxy->next)
	{
	    (void) printf("pxy = %p, pxy->p = %p ,",pxy,(&pxy->p));
	    (void) printf("<%g, %g>, prev = %p, next = %p\n",
	    	          COORDS(pxy->p)[0],COORDS(pxy->p)[1],
			  pxy->prev,pxy->next);
	    if (pxy == xy_end)
		break;
	}
}		/*end print_xylist*/

LOCAL	void	print_ring_positions(
	RING_POINT	*p_ring)
{
	int		i;
	RING_POINT	*rng_pt;
	int		dim = p_ring->intfc->dim;

	for (i = 1, rng_pt = p_ring; rng_pt; ++i, rng_pt = rng_pt->next)
	{
	    (void) printf("The %dth point on the ring:  ",i);
	    print_general_vector("",Ring_point_coords(rng_pt),dim,"\n");
	    if (rng_pt->next == p_ring)
		break;
	}
}		/*end print_ring_positions*/

LOCAL void print_tri_curves(
	const char	*message,
	TRI_CURVE	*tri_curves)
{
	TRI_CURVE	*tri_c;

	if (!debugging("tri_curves"))
	    return;

	(void) printf("%s\n",message);
	(void) printf("\nPrint tri_curves %p:\n",(POINTER)tri_curves);
	for (tri_c = tri_curves; tri_c; tri_c = tri_c->next)
	    print_a_tri_curve(tri_c);
}		/*end print_tri_curves*/


LOCAL void print_a_tri_curve(
	TRI_CURVE	*tri_c)
{
	const char *interior_side, *side_type, *curve_type;
	if (tri_c == NULL)
	{
	    (void) printf("NULL TRI_CURVE\n");
	    return;
	}
	(void) printf("tri_c %p: 	tg_pts_index = %d       n_pts = %d\n", 
		      (POINTER)tri_c,tri_c->tg_pts_index,tri_c->n_pts);
	switch (tri_c->interior_side)
	{
        case LEFT_SIDE:
	    interior_side = "LEFT_SIDE";
	    break;
	case RIGHT_SIDE:
	    interior_side = "RIGHT_SIDE";
	    break;
	case BOTH_SIDES:
	    interior_side = "BOTH_SIDES";
	    break;
	default:
	    interior_side = "UNKNOWN";
	    break;
	}
	switch (tri_c->side)
	{
    	case E_SIDE:
	    side_type = "E_SIDE";
    	    break;
    	case  F_SIDE:
	    side_type = "F_SIDE";
    	    break;
    	case I_SIDE:
	    side_type = "I_SIDE";
    	    break;
    	default:
	    side_type = "UNKNOWN";
    	    break;
	}
	switch (tri_c->curve_type)
	{
	case INTERIOR_CURVE:
	    curve_type = "INTERIOR_CURVE";
	    break;
	case BOUNDARY_CURVE:
	    curve_type = "BOUNDARY_CURVE";
	    break;
	default:	
	    curve_type = "UNKNOWN";
	    break;
	}
	(void) printf("interior_side = %s, side = %s, curve_type = %s\n",
	              interior_side,side_type,curve_type);
	print_curve(tri_c->curve);
}		/*end print_a_tri_curve*/

LOCAL void print_ring(
	RING_POINT	*ring)
{
	RING_POINT	  *rng_pt;
	static const char *hdr[] = {"RING_POINT_X",
				    "RING_POINT_Y",
				    "RING_POINT_Z"};
	int		  i;
	int		  dim = ring->intfc->dim;

	(void) printf("\nPrint ring starting at ring point %p\n",ring);
	for (rng_pt = ring; rng_pt; rng_pt = rng_pt->next)
	{
	    print_ring_point(rng_pt);
	    if (rng_pt->next == ring)
		break;
	}

	/* Printout for graphs*/

	(void) printf("\nGRAPHS TYPE PRINTOUT OF RING\n");
	for (i = 0; i < dim; ++i)
	    (void) printf("%-14s%s",hdr[i],(i<dim-1)?" ":"\n");
	for (rng_pt = ring; rng_pt; rng_pt = rng_pt->next)
	{
	    for (i = 0; i < dim; ++i)
	    {
	       	(void) printf("%-14g%s", Ring_point_coords(rng_pt)[i],
	    		      (i < dim - 1) ? " " : "\n");
	    }
	    if (rng_pt->next == ring)
		break;
	}
	for (i = 0; i < dim; ++i)
	{
	    (void) printf("%-14g%s",Ring_point_coords(ring)[i],
			  (i < dim -1) ? " " : "\n");
	}
	(void) printf("\n");
}		/*end print_ring*/

#endif /* defined(DEBUG_TRI_GRID) */
#endif /* defined(TWOD) */
