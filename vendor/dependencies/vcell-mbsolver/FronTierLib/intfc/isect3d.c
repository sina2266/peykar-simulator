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
*
*                                 isect3d.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*       Contains the routine i_intersections3d() and its support routines.
*/

#if defined(THREED)
#define DEBUG_STRING "isect3d"
#include <sys/types.h>
#include <sys/stat.h>
#include <intfc/iloc.h>


	/* LOCAL Function Declarations */
LOCAL	C_BOND	*test_coplanar_cross(TRI*,TRI*,int,boolean*,INTERFACE*);
LOCAL	C_BOND	*test_cross(TRI*,TRI*,boolean*,INTERFACE*);
LOCAL	C_CURVE	**install_c_bond_in_curve(C_BOND*,SURFACE*,SURFACE*,C_CURVE**);
LOCAL	C_CURVE	*make_c_curve(SURFACE*,SURFACE*,NODE*,NODE*,C_CURVE***);
LOCAL	C_CURVE	**sort_cross_curves(C_CURVE**,INTERFACE*);
LOCAL	boolean	next_adjacent(C_BOND*,C_BOND*);
LOCAL	boolean	plane_segments_cross(double*,double*,double*,double*);
LOCAL	boolean	prev_adjacent(C_BOND*,C_BOND*);
LOCAL	boolean    same_cb_pt(FT_POINT*,FT_POINT*);
LOCAL	void	substitute_a_point(FT_POINT*,TRI*,int,INTERFACE*);
LOCAL	void    substitute_a_point_on_surface(FT_POINT*,FT_POINT*,TRI*,TRI***,
	                                      INTERFACE*);
LOCAL	void	show_crossing_tris(const char*,TRI*,TRI*,C_BOND*,INTERFACE*);

/*
*			       i_intersections3d():
*
*	Determines all curves of intersection of SURFACES of an INTERFACE,
*	guarding against various degeneracies that can occur.   The
*	boundary SURFACES, if any, of INTERFACE are not included in
*       checking for intersections unless the variable bdry = YES.
*
*       The intersection is defined by an interface, consisting of curves
*       and nodes, that is, a topologically two dimensional interface,
*       imbedded in three-space, or more accurately, a three dimensional
*       interface with no elements of maximal dimension, or having
*       co-dimension 2.  The intersection structure is stored in an
*       INTERFACE, pointed by *cross_intfc.
*
*       Associated with each bond of a curve in this interface will be a
*       pair of triangles and surfaces which give rise to the
*       intersection, and which come from the (tangled) interface.  The
*       natural direction on the bond comes from the tangent t = n1 x
*       n2, where n1 and n2 are the normals to the oriented triangles t1
*       and t2 which intersect. The intersection will continue into
*       neighboring triangles, and in this fashion define adjacent pairs
*       of intersecting triangles (one of which will normally be either
*       t1 or t2).  This pair of neighboring intersecting triangles thus
*       defines the next and prev bond, so the bonds are naturally
*       organized into curves.  There is additional information from the
*       intersection, eg. the triangles and the surfaces to which they
*       belong, which is contained in a companion data structure C_BOND.
*       The C_BOND should thus be thought of as a bond of the
*       intersection interface, augmented with additional information
*       concerning the intersection.
*
*       The C_BOND's are arranged in doubly linked lists. Initially
*	these lists are established and ordered as created, but as a later
*       step (a sort), they are ordered to correspond to a single curve
*       of the intersection interface, and the distinct linked lists are
*       maintained in a cross_list (C_BOND **) corresponding to and
*       ordered identically to the CURVE ** curves of the intersection
*       interface.
*
*       Nodes of the intersection interface are endpoints of curves, and
*	arise when the intersecting triangles t1 or t2 have a
*	surface boundary bond rather than a triangle as the prev or next
*	intersection, that is topologically when the intersection curve
*	runs into the boundary of a surface. Thus a node arises when
*	the curve c describing the intersection of surfaces s1 and s2
*	runs into either the boundary of s1 or of s2. Eventually, the
*	NODES of the intersection interface may require a data structure
*	of their own, to contain the supplementary intersection
*       information.
*
*	The connectivity of the CROSS_ELEMENT structures into curves
*	is contained in the CROSS structure.  It contains a doubly
*	linked list of CROSS_ELEMENTS that correspond to individual
*	bonds on the curve defined by the intersecting surfaces.
*	CROSS is a global object describing the entire intersection
*	of a pair of surfaces,  while CROSS_ELEMENT describes a single
*	pair of crossing surface elements.
*
*	Usage:
*	       status = i_intersections3d(intfc,cross_list,bdry);
*	       INTERFACE *intfc;
*	       C_BOND   ***cross_list;
*	       int bdry;
*
*	returns one of  the following:
*	
*	    1. NO if topology construction fails
*	    2. clean_up() if add_to_pointers() fails
*	    3. YES otherwise
*/

LOCAL	double cr_tol;

LIB_LOCAL  boolean  i_intersections3d(
	INTERFACE  *intfc,  /* interface on which to check intersections */
	CROSS      **cross, /* list of discovered crosses */
	const boolean bdry)    /* Check for boundary intersections if bdry==YES */
{
	C_BOND	  *cbond;
	C_CURVE	  **c;
	C_CURVE	  **c_curves = NULL;
	COMPONENT ***cz, **czy, *czyx;
	CROSS	  *cr, Cr;
	NODE	  **n;
	SURFACE	  *****sz, ****szy, ***szyx, **s, **s0, **s1;
	SURFACE	  *cs0, *cs1;
	TRI	  **t, *tri, *ct0, *ct1;
	TRI	  *****tz, ****tzy, ***tzyx, **t0, **t1;
	int	  ix, iy, iz;
	int	  i, j, xmax, ymax, zmax, nt;
	int	  ***nz, **nzy, *nzyx;
	boolean	  status;

	if (intfc->surfaces == NULL) 
	{
	    *cross = NULL;
	    return YES;
	}

	DEBUG_ENTER(i_intersections3d)

	start_clock("i_intersections3d");

	if (intfc->modified || intfc->table->new_grid)
	{
	    if (DEBUG)
		(void) printf("calling make_tri_lists\n");
	    
	    if (make_tri_lists(intfc) != YES)
	    {
		(void) printf("WARNING in i_intersections3d(), "
		              "make_tri_lists() failed\n");
		stop_clock("i_intersections3d");
		DEBUG_LEAVE(i_intersections3d)
		return NO;
	    }

	    intfc->modified = NO;
	    intfc->table->new_grid = NO;
	}
	intfc->c_curves = NULL;

	    /* Initialize tri cross lists to NULL */

	for (s = intfc->surfaces; s && *s; ++s)
	    for (tri = first_tri(*s); !at_end_of_tri_list(tri,*s);
		 tri = tri->next)
	{
	    Tri_cross_list(tri) = NULL;
	    node_at_point(Point_of_tri(tri)[0]) = NULL;
	    node_at_point(Point_of_tri(tri)[1]) = NULL;
	    node_at_point(Point_of_tri(tri)[2]) = NULL;
	}
	for (n = intfc->nodes; n && *n; ++n)
	    node_at_point((*n)->posn) = *n;

	xmax = topological_grid(intfc).gmax[0];
	ymax = topological_grid(intfc).gmax[1];
	zmax = topological_grid(intfc).gmax[2];
	cr_tol = cross_tolerance(intfc);

	start_clock("make intersection loop");

/*
*	The information that follows is set up in the interface table defined
*	in int.h, and is updated by the function make_tri_lists(), which "bins"
*	the surface elements (TRI's) into grid cubes determined by the
*	topological grid.
*/
	tz = intfc->table->tris;
	sz = intfc->table->surfaces;
	nz = intfc->table->num_of_tris;
	cz = intfc->table->compon3d;

/*
*	The following code sets up a loop over all of the "bins" (i.e. grid
*	cubes, or blocks of the topological grid.)
*/

	for (iz = 0; iz < zmax; ++iz, ++tz, ++sz, ++nz, ++cz)
	{
	    tzy = *tz;	szy = *sz;	nzy = *nz; czy = *cz;
        for (iy = 0; iy < ymax; ++iy, ++tzy, ++szy, ++nzy, ++czy)
	{
	    tzyx = *tzy;   szyx = *szy;	nzyx = *nzy; czyx = *czy;
	for (ix = 0; ix < xmax; ++ix, ++tzyx, ++szyx, ++nzyx, ++czyx)
	{
	    /* This grid cube must be passed through by the front...  */

	    if (*czyx != ONFRONT)
		continue;

	    /* and it must contain 2 or more TRI's.     */
	    
	    if ((nt = *nzyx) < 2)
		continue;

	    /*
	    *    Set t,s equal to, respectively, the list of TRI's and the
	    *    list of SURFACE's for this grid cube.
	    */
	    
	    t = *tzyx;
	    s = *szyx;
	    
	        /* Loop over all pairs of Tris: */
	    for (i = 0, t0 = t, s0 = s; i < nt - 1; ++i, ++t0, ++s0)
	    {

		if (is_bdry(*s0) && !bdry)
		    continue;

		for (j = i+1, t1 = t+j, s1 = s+j; j < nt; ++j, ++t1, ++s1)
		{

		    if (*t1 == Tri_on_side01(*t0) ||
		        *t1 == Tri_on_side12(*t0) ||
			*t1 == Tri_on_side20(*t0)) continue;
		    ct0 = *t0;  ct1 = *t1;
		    cs0 = *s0;  cs1 = *s1;
		    
		    if (is_bdry(cs1) && !bdry)
			continue;

		    /*
		    * In the beginning of this function, the list of
		    * c_curves was initialized as NULL. As triangles cross
		    * - C_BOND's - are found by test_cross() below, the
		    * function install_c_bond_in_curve() creates new 
		    * C_CURVE's and adds them to the list c_curves.
		    */

		    for (c = c_curves;  c && *c;  ++c)
		    {
			if ((cs0 == (*c)->s[0])  &&  (cs1 == (*c)->s[1]))
			    break;
			else if ((cs0 == (*c)->s[1])  &&  (cs1 == (*c)->s[0]))
			{
			    ct0 = *t1;  ct1 = *t0;
			    cs0 = *s1;  cs1 = *s0;
			    break;
			}
		    }
		    
		    if ((cbond = test_cross(ct0,ct1,&status,intfc)) != NULL)
		    {
			(void) add_to_pointers(cbond,&Tri_cross_list(ct0));
			(void) add_to_pointers(cbond,&Tri_cross_list(ct1));
			if (DEBUG)
			    show_crossing_tris("i_intersections3d",
			                       ct0,ct1,cbond,intfc);
			c_curves = install_c_bond_in_curve(cbond,cs0,cs1,
						           c_curves);
		    }
		    else if (status == FUNCTION_FAILED)
		    {
		        DEBUG_LEAVE(i_intersections3d)
			return FUNCTION_FAILED;
		    }
		}
	    }
	}
	}
	}
	stop_clock("make intersection loop");

	c_curves = sort_cross_curves(c_curves,intfc);

	/* Make the cross list */
	Cr.prev = Cr.next = NULL;
	cr = &Cr;
	for (c = c_curves;  c && *c;  ++c)
	{
	    cr->next = (CROSS *)store(sizeof(CROSS));
	    cr->next->prev = cr;
	    cr = cr->next;
	    cr->c_curve = *c;

	    cs0 = (*c)->s[0];
	    cs1 = (*c)->s[1];
	    if (!add_to_pointers(*c,&intfc->c_curves))
	    {
		screen("ERROR in i_intersections3d(), "
		       "add_to_pointers() failed\n");
		clean_up(ERROR);
	    }
	    if (!add_to_pointers(*c,&cs0->c_curves))
	    {
	        screen("ERROR in i_intersections3d(), "
	               "add_to_pointers() failed\n");
	        clean_up(ERROR);
	    }
	    if ((cs0 != cs1) && (!add_to_pointers(*c,&cs1->c_curves)))
	    {
	        screen("ERROR in i_intersections3d(), "
	               "add_to_pointers() failed\n");
	        clean_up(ERROR);
	    }
	}
	cr->next = NULL;
	*cross = Cr.next;
	if (*cross)
	    (*cross)->prev = NULL;

	stop_clock("i_intersections3d");
	DEBUG_LEAVE(i_intersections3d)
	return  YES;
}		/*end i_intersections3d*/


/*
*                          install_c_bond_in_curve():
*
*	This function groups c_bond's by the surfaces on which the
*	c_bond lies. Each of the group will be further sorted into one
*	or more c_curve's later by another function.
*
*	Returns c_curves on success and NULL on failure. 
*/

LOCAL   C_CURVE	**install_c_bond_in_curve(
	C_BOND  *c_bond,
	SURFACE *s0,
	SURFACE *s1,
	C_CURVE **c_curves)
{
	C_CURVE  **c, *new_c;

	DEBUG_ENTER(install_c_bond_in_curve)
	if ((c_bond == NULL)  ||  (s0 == NULL)  ||  (s1 == NULL))
	{
	    screen("ERROR in install_c_bond_in_curve(), "
	           "c_bond, s0, or s1 is NULL!\n");
	    clean_up(ERROR);
	}

	/*
	* The following loops over all of the C_CURVE's identified so far and
	* checks that the given SURFACE's, s0 and s1, match it identically.
	* At this point, this is sufficient to identify a cross curve data
	* structure, since the linked lists being accumulated of cross bonds
	* are unordered.  If a match is found, the C_BOND is inserted as the
	* first in the list.
	*/

	for (c = c_curves;  c && *c;  ++c)
	{
	    if (s0 == (*c)->s[0]  &&  s1 == (*c)->s[1])
	    { 
		c_bond->next = (*c)->first;
		(*c)->first->prev = c_bond; 
		(*c)->first = c_bond;
		break;
	    }
	}

	/*
	* If not - if the above loop terminates without finding a matching
	* C_CURVE - then a new pair of surfaces has been found.  We need to
	* create a new C_CURVE structure.
	*/

	if ((c == NULL)  ||  (*c == NULL))
	{
	    new_c = make_c_curve(s0,s1,NULL,NULL,&c_curves);
	    new_c->first = c_bond; 
	    new_c->last = c_bond; 
	    new_c->last->next = NULL;
	}
	DEBUG_LEAVE(install_c_bond_in_curve)
	return c_curves;
}		/*end install_c_bond_in_curve*/

/*
*            		         test_cross():
*
*       The test for intersections of triangles is based on linear
*       algebra to solve systems of three equations and betweenness
*       properties.  Given TRIs ti, i = 1,2, with vertices ai, bi, ci,
*       in cyclic order, none of which coincide, we form the directed
*       normal uni_arrays ni, e.g.  ni = (ci - ai) x (bi - ai).  Each
*       triangle either has all its vertices on one side of the plane
*       defined by the other or it has two on one side and one on the
*       other (It's also possible to have one point in the
*       plane and the other two points on opposite sides of it).  In the
*       former case for either triangle, there is no intersection. The
*       test is by comparing dot products. We set B = <bj - ai . ni >
*       and C = <cj - ai . ni >.  Then if sgn B != sgn C, the edge (bj
*       -> cj) of tj cuts the plane defined by ti.
*
*       Furthermore setting lambda = C / (C - B), the point p = lambda
*       bj + (1 - lambda) cj is the point on the bj -> cj edge of tj
*       which cuts the plane of ti.
*
*                                              A  normal of tj
*                           cj                 |
*                          / \                 *
*                     __ /    \                | C
*                      /|      \               |
*            cj-aj   /    nj    \              *<----------ai
*                  /      \/     \             | B
*                /        /\      \    	       |
*           aj /_____________\_____\ bj        *
*                            /                 |
*                      bj-aj
*
*       If fewer than four cut points, two per triangle, are determined
*       in this fashion, there is no intersection.  Otherwise consider
*       the direction l = n1 x n2, which is parallel to the line of
*       intersection of t1 and t2.  For each of the cut points pk, we
*       determine alpha_k = <pk - ai, l>, which parameterize the
*       projections of the cut points onto the line of intersection of
*       the planes through t1 and t2, ie the line with direction
*       parallel to l.  The occurence or nonoccurence of an intersection
*       then depends on the betweenness properties of the alpha_k. The
*       alpha_k also specify the exact location of the intersection in
*       parametric coordinates relative to each triangle ti.
*
*       If the tri's t1 and t2 have a common point then the same ideas
*       apply, but details of the algorithm must be changed to avoid
*       degeneracies.  Similarly, tris which are parallel (within
*       floating point tolerance) are regarded as non-intersecting.
*
*       Returns C_BOND* if the pair forms a cross and NULL otherwise.
*/

LOCAL C_BOND *test_cross(
	TRI       *t0,
	TRI       *t1,
	boolean      *status,
	INTERFACE *intfc)
{
	C_BOND      *new_cb,**cbs;
	FT_POINT       *q[3];
	double  	    line[MAXD];     /* intersection line of t0, t1 planes */
	const double *tnor;

	struct {
	    double   coords[MAXD];    /* point of boundary cross */
	    double  order;           /* projection of coords on line */
	    boolean edge_vertex;     /* is it a edge (1) or vertex (0) */
	    int     label;           /* index of edge or vertex */
	} x[4];     /* x: crossing of boundary of ti thru plane of tj */

	int  l0 = 0, l1 = 2;
	int  cv0[3], cv1[3];
	int  i, j, k, u0, u1,
	     i0, j0, k0, i1, j1, k1,   /* see below */
	     num_com_vertex,

	     num_t0_touching,    /* No.verteces of t0 on plane of t1 */
	     num_t1_touching,
	     num_t0_crossing,    /* No.edges of t0 cross plane of t1 */
	     num_t1_crossing;

	double  *start, *end, a0, a1, r,
	       norm_t0[3], norm_t1[3],    /* normal to t0 and t1 */
	       norm0, norm1,
	       *p_of_t0[3],*p_of_t1[3],         /* coordinates of vertices */
	       proj_of_t0[3], proj_of_t1[3];    /* projection of points */
					        /* of ti on normal of tj */

#define set_c_surf_flag(f,e,o,l)				        \
    ( cs_on_bdry(f) = (o),						\
      cs_edge_vertex(f) = (o) ? (e) : NO,				\
      cs_tri_side_index(f) = ((o) && (e)) ? (l) : -1)

#define set_flag_start(cb,i,e_v,on_bdry,label)         \
    set_c_surf_flag(cs_flag_start((cb)->s[i]),e_v,on_bdry,label)

#define set_flag_end(cb,i,e_v,on_bdry,label)           \
    set_c_surf_flag(cs_flag_end((cb)->s[i]),e_v,on_bdry,label)

	*status = FUNCTION_SUCCEEDED;

		/* Check if cross has already been recorded */

	for (cbs = Tri_cross_list(t0); cbs && *cbs; ++cbs)
	{
	    if (((*cbs)->s[0].t == t0 && (*cbs)->s[1].t == t1) ||
	        ((*cbs)->s[1].t == t0 && (*cbs)->s[0].t == t1))
	        return NULL;
	}

	    /* find num_com_vertex of t[i],t[j] */

	for (i = 0; i < 3; ++i)
	{
	    q[i] = Point_of_tri(t1)[i];
	    p_of_t1[i] = Coords(q[i]);
	}

	/*   i0, i1 will denote the common vertex */
	/*   of t0, t1 if num_com_vertex = 1.     */

	num_com_vertex = 0;
	for (i = 0; i < 3; ++i)
	{
	    FT_POINT *p = Point_of_tri(t0)[i];
		
	    p_of_t0[i] = Coords(p);
	    for (j = 0; j < 3; ++j)
	    {
	    	if (p == q[j])
	    	{
	    	    cv0[num_com_vertex] = i;
	 	    cv1[num_com_vertex] = j;
		    ++num_com_vertex;
		}
	    }
	}

	if (num_com_vertex == 3)
	{
	    screen("ERROR in test_cross(), "
	           "unexpected case num_com_vertex == 3\n");
	    print_tri(t0,intfc);
	    print_tri(t1,intfc);
	    print_interface(intfc);
	    gview_plot_interface("ERROR_IN_TEST_CROSS",intfc);
	    clean_up(ERROR);
	    *status = FUNCTION_FAILED;
	    return NULL;
	}

	tnor = Tri_normal(t0);
	norm0 = Mag3d(tnor);
	for (i = 0; i < 3; ++i)
	    norm_t0[i] = tnor[i]/norm0;
	tnor = Tri_normal(t1);
	norm1 = Mag3d(tnor);
	for (i = 0; i < 3; ++i)
	    norm_t1[i] = tnor[i]/norm1;
	if (num_com_vertex == 2)
	{
	    double v[3], u[3];
	    double sv[3], v0[3], v1[3];
	    int   sd0, sd1;
	    sd0 = (cv0[1] == Next_m3(cv0[0])) ? cv0[0] : cv0[1];
	    sd1 = (cv1[1] == Next_m3(cv1[0])) ? cv1[0] : cv1[1];
	    if (is_side_bdry(t0,sd0) || is_side_bdry(t0,sd0))
	    {
		BOND_TRI *bt0, *bt1;;
	        if (!is_side_bdry(t0,sd0) || !is_side_bdry(t1,sd1))
		{
		    screen("ERROR in test_cross(), inconsistent "
		           "side boundarys of tris with common side, "
			   "is_side_bdry(t0,sd0) = %d, "
			   "is_side_bdry(t1,sd1) = %d\n",
			   is_side_bdry(t0,sd0),is_side_bdry(t1,sd1));
		    (void) printf("sd0 = %d, sd1 = %d\n",sd0,sd1);
	            print_tri(t0,intfc);
	            print_tri(t1,intfc);
	            print_interface(intfc);
	            show_crossing_tris("test_cross",t0,t1,NULL,intfc);
	            gview_plot_interface("ERROR_IN_TEST_CROSS",intfc);
	            clean_up(ERROR);
	            *status = FUNCTION_FAILED;
	            return NULL;
		}
		bt0 = Bond_tri_on_side(t0,sd0);
		bt1 = Bond_tri_on_side(t1,sd1);
		if (bt0->bond != bt1->bond)
		{
	            screen("ERROR in test_cross(), "
	                   "inconsisent bonds on common side of tris\n");
		    (void) printf("sd0 = %d, sd1 = %d\n",sd0,sd1);
	            print_tri(t0,intfc);
	            print_tri(t1,intfc);
	            print_interface(intfc);
	            show_crossing_tris("test_cross",t0,t1,NULL,intfc);
	            gview_plot_interface("ERROR_IN_TEST_CROSS",intfc);
	            clean_up(ERROR);
	            *status = FUNCTION_FAILED;
	            return NULL;

		}
	    }
	    else if ((Tri_on_side(t0,sd0) != t1) || (Tri_on_side(t1,sd1) != t0))
	    {
	        screen("ERROR in test_cross(), "
	               "inconsistent neighboring tris with common side!\n");
		(void) printf("sd0 = %d, sd1 = %d\n",sd0,sd1);
	        print_tri(t0,intfc);
	        print_tri(t1,intfc);
	        print_interface(intfc);
	        show_crossing_tris("test_cross",t0,t1,NULL,intfc);
	        gview_plot_interface("ERROR_IN_TEST_CROSS",intfc);
	        clean_up(ERROR);
	        *status = FUNCTION_FAILED;
	        return NULL;
	    }
	    Cross3d(norm_t0,norm_t1,v);
	    if (Dot3d(v,v) > MIN_SIN_SQR(intfc))
	        return NULL;
	    (void) vector_on_tri_side(t0,sd0,sv);
	    (void) vector_on_tri_side(t0,Prev_m3(sd0),v0);
	    (void) vector_on_tri_side(t1,Next_m3(sd1),v1);
	    Cross3d(norm_t0,sv,v);
	    Cross3d(norm_t1,sv,u);
	    if ((Dot3d(v0,v)*Dot3d(v1,v) < 0.0) ||
	        (Dot3d(v0,u)*Dot3d(v1,u) < 0.0))
	    {
	        double magv, magu;
		double lv0, lv1;
		double dv0v, dv1v, dv0u, dv1u;
	        screen("ERROR in test_cross(), overlapping "
	               "coplanar tris with common side\n");
	        magv = sqrt(Dot3d(v,v));
	        magu = sqrt(Dot3d(u,u));
	        (void) printf("sd0 = %d, sd1 = %d\n",sd0,sd1);
		print_general_vector("sv = ",sv,3,"\n");
		print_general_vector("v0 = ",v0,3,"\n");
		print_general_vector("v1 = ",v1,3,"\n");
		print_general_vector("v = ",v,3,"\n");
		print_general_vector("u = ",u,3,"\n");
		print_general_vector("norm_t0 = ",norm_t0,3,"\n");
		print_general_vector("norm_t1 = ",norm_t1,3,"\n");
		lv0 = length_of_tri_side(t0,Prev_m3(sd0));
		lv1 = length_of_tri_side(t1,Next_m3(sd1));
		dv0v = Dot3d(v0,v);
		dv1v = Dot3d(v1,v);
		(void) printf("Dot3d(v0,v) = %g, Dot3d(v1,v) = %g\n",dv0v,dv1v);
		(void) printf("Cos(v0,v) = %g, Cos(v1,v) = %g\n",
		              dv0v/(lv0*magv),dv1v/(lv1*magv));
		dv0u = Dot3d(v0,u);
		dv1u = Dot3d(v1,u);
		(void) printf("Dot3d(v0,u) = %g, Dot3d(v1,u) = %g\n",dv0u,dv1u);
		(void) printf("Cos(v0,u) = %g, Cos(v1,u) = %g\n",
		              dv0u/(lv0*magu),dv1u/(lv1*magu));
	        Cross3d(norm_t0,norm_t1,v);
		print_general_vector("norm_t0 X norm_t1 = ",v,3,"\n");
		(void) printf("|Sin(norm_t0,norm_t1)| = %g\n",
		              sqrt(Dot3d(v,v)));
	        print_tri(t0,intfc);
	        print_tri(t1,intfc);
	        print_interface(intfc);
	        gview_plot_interface("ERROR_IN_TEST_CROSS",intfc);
	        show_crossing_tris("test_cross",t0,t1,NULL,intfc);
	        clean_up(ERROR);
	        *status = FUNCTION_FAILED;
	    }
	    return NULL;

	}

	Cross3d(norm_t0,norm_t1,line);   /* line = norm_t0 x norm_t1 */

	    /* Are Tri's parallel?  should be EPS4? */

	if (Dot3d(line,line) <= MIN_SIN_SQR(intfc))
	    return NULL;

	    /* projection of ti's points onto the normal of tj */

	a0 = Dot3d(p_of_t0[0], norm_t0);
	a1 = Dot3d(p_of_t1[0], norm_t1);
	for (i = 0; i < 3; ++i)
	{
	    proj_of_t0[i] = Dot3d(p_of_t0[i],norm_t1) - a1;
	    if (fabs(proj_of_t0[i]) <= cr_tol)
		proj_of_t0[i] = 0.0;
	    proj_of_t1[i] = Dot3d(p_of_t1[i],norm_t0) - a0; 
	    if (fabs(proj_of_t1[i]) <= cr_tol)
		proj_of_t1[i] = 0.0;
	}

	    /* intersection of ti's bdry with tj's plane */

	if (num_com_vertex == 1)
	{
	    	/* processing t0 */

	    i0 = cv0[0];
	    j0 = Next_m3(i0);
	    k0 = Prev_m3(i0);
	    if (proj_of_t0[j0]*proj_of_t0[k0] > 0.0)
	    {
		/* Two non-common vertices are on */
		/* the same side of the t1 plane */
		return NULL;
	    }
	    else if ((proj_of_t0[j0] == 0.0) && (proj_of_t0[k0] == 0.0))
	    {
		/* Three t0 vertices are on the same plane of t1 */
		return NULL;
	    }
	    i1 = cv1[0];
	    j1 = Next_m3(i1);
	    k1 = Prev_m3(i1);
	    if (proj_of_t1[j1]*proj_of_t1[k1] > 0.0)
	    {
		/* Two non-common vertices are on */
		/* the same side of the t0 plane */
		return NULL;
	    }
	    else if ((proj_of_t1[j1] == 0.0) && (proj_of_t1[k1] == 0.0))
	    {
		/* Three t1 vertices are on the same plane of t0 */
		return NULL;
	    }

	    	/* determine t0 intersections */
		
	    if (proj_of_t0[j0] == 0.0)
	    {
		num_t0_touching = 2;
		num_t0_crossing = 0;
		for (i = 0; i < 3; ++i)
		    x[0].coords[i] = p_of_t0[j0][i];
		x[0].label = j0;
		x[0].edge_vertex = NO;
	    }
	    else if (proj_of_t0[k0] == 0.0)
	    {
		num_t0_touching = 2;
		num_t0_crossing = 0;
		for (i = 0; i < 3; ++i)
		    x[0].coords[i] = p_of_t0[k0][i];
		x[0].label = k0;
		x[0].edge_vertex = NO;
	    }
	    else
	    {
	        num_t0_touching = 1;
		num_t0_crossing = 1;
		r = proj_of_t0[j0] / (proj_of_t0[j0] - proj_of_t0[k0]);
		for (i = 0; i < 3; ++i)
		    x[0].coords[i] = r*p_of_t0[k0][i] + (1 - r)*p_of_t0[j0][i];
		x[0].label = j0;
		x[0].edge_vertex = YES;
	    }
	    x[0].order = Dot3d(x[0].coords,line);

	    	/* determine t1 intersections */

	    if (proj_of_t1[j1] == 0.0)
	    {
		num_t1_touching = 2;
		num_t1_crossing = 0;
		for (i = 0; i < 3; ++i)
		    x[3].coords[i] = p_of_t1[j1][i];
		x[3].label = j1;
		x[3].edge_vertex = NO;
	    }
	    else if (proj_of_t1[k1] == 0.0)
	    {
		num_t1_touching = 2;
		num_t1_crossing = 0;
		for (i = 0; i < 3; ++i)
		    x[3].coords[i] = p_of_t1[k1][i];
		x[3].label = k1;
		x[3].edge_vertex = NO;
	    }
	    else
	    {
	        num_t1_touching = 1;
		num_t1_crossing = 1;
		r = proj_of_t1[j1]/(proj_of_t1[j1] - proj_of_t1[k1]);
		for (i = 0; i < 3; ++i)
		    x[3].coords[i] = r*p_of_t1[k1][i] + (1 - r)*p_of_t1[j1][i];
		x[3].label = j1;
		x[3].edge_vertex = YES;
	    }
	    x[3].order = Dot3d(x[3].coords,line);

	    	/* intersections due to the common vertex */

	    for (i = 0; i < 3; ++i)
		x[2].coords[i] = x[1].coords[i] = p_of_t0[i0][i];

	    x[2].order = x[1].order = Dot3d(x[1].coords,line);
	    x[2].edge_vertex = x[1].edge_vertex = NO;
	    x[1].label = i0;      x[2].label = i1;
	}
	else    /* if (num_com_vertex == 0) */
	{
	    if ((proj_of_t0[0]*proj_of_t0[1] > 0.0 && 
		                       proj_of_t0[0]*proj_of_t0[2] > 0.0)
				||
	    	(proj_of_t1[0]*proj_of_t1[1] > 0.0 && 
		                       proj_of_t1[0]*proj_of_t1[2] > 0.0))
	    {
		/* at least one triangle has all of its */
		/* vertices on the same side of another */
		return NULL;
	    }
	    num_t0_touching = 0;
	    num_t0_crossing = 0;
	    num_t1_touching = 0;
	    num_t1_crossing = 0;
	    l0 = 0; l1 = 2;
	    for (i = 0; i< 3; ++i)
	    {	
		j = Next_m3(i);
		if (proj_of_t0[i] == 0.0)
		{
		    ++num_t0_touching;
		    if (l0 == 2)
		    {
			(void) printf("WARNING in test_cross(), "
				      "Unexpected case, co-planar tris "
				      "with no common vertices\n");
			(void) printf("l0 = %d, l1 = %d\n",l0,l1);
			print_tri(t0,intfc);
			print_tri(t1,intfc); 
			return test_coplanar_cross(t0,t1,num_com_vertex,
			                           status,intfc);
		    }
		    for (k=0; k< 3; ++k)
			x[l0].coords[k] = p_of_t0[i][k];
		    x[l0].order = Dot3d(x[l0].coords, line);
		    x[l0].edge_vertex = NO;
		    x[l0].label = i;
		    ++l0;
		}
		else if (proj_of_t0[i] * proj_of_t0[j] < 0.0)
		{
		    ++num_t0_crossing;
		    if (l0 == 2)
		    {
			screen("ERROR in test_cross(),  three Crossings!\n");
	                print_tri(t0,intfc);
	                print_tri(t1,intfc);
	                print_interface(intfc);
	                gview_plot_interface("ERROR_IN_TEST_CROSS",intfc);
	                *status = FUNCTION_FAILED;
			clean_up(ERROR);
	                return NULL;
		    }
		    r = proj_of_t0[j] / (proj_of_t0[j] - proj_of_t0[i]);
		    for (k=0; k< 3; ++k)   x[l0].coords[k] =
			r * p_of_t0[i][k] + (1 - r) * p_of_t0[j][k];
		    x[l0].order = Dot3d(x[l0].coords, line);
		    x[l0].edge_vertex = YES;
		    x[l0].label = i;
		    ++l0;
		}

		if (proj_of_t1[i] == 0)
		{
		    ++num_t1_touching;
		    if (l1 == 4)
		    {
			(void) printf("WARNING in test_cross(), "
				      "Unexpected case, co-planar tris "
				      "with no common vertices\n");
			(void) printf("l0 = %d, l1 = %d\n",l0,l1);
			return test_coplanar_cross(t0,t1,num_com_vertex,
			                           status,intfc);
		    }
		    for (k=0; k< 3; ++k)
			x[l1].coords[k] = p_of_t1[i][k];
		    x[l1].order = Dot3d(x[l1].coords, line);
		    x[l1].edge_vertex = NO;   x[l1].label = i;
		    ++l1;
		}
		else if (proj_of_t1[i]*proj_of_t1[j] < 0.0)
		{
		    ++num_t1_crossing;
		    if (l1 == 4)
		    {
			screen("ERROR in test_cross(), three Crossings!\n");
	                print_tri(t0,intfc);
	                print_tri(t1,intfc);
	                print_interface(intfc);
			gview_plot_interface("ERROR_IN_TEST_CROSS",intfc);
	                *status = FUNCTION_FAILED;
			clean_up(ERROR);
	                return NULL;
		    }
		    r = proj_of_t1[j] / (proj_of_t1[j] - proj_of_t1[i]);
		    for (k=0; k< 3; ++k)
			x[l1].coords[k] =
			    r * p_of_t1[i][k] + (1 - r) * p_of_t1[j][k];
		    x[l1].order = Dot3d(x[l1].coords, line);
		    x[l1].edge_vertex = YES;
		    x[l1].label = i;
		    ++l1;
		}
	    }
	}	

		/* Non-Intersecting Cases */

	switch (num_t0_touching)
	{
	case 0:
	    if (num_t0_crossing == 0)
		return NULL;     /* no crossing */
	    if (num_t0_crossing != 2)                 /* shouldn't happen */
	    {
		screen("ERROR in test_cross(), "
		       "No.touch=0 while No.crossing!=2.\n");
	        print_tri(t0,intfc);
	        print_tri(t1,intfc);
	        print_interface(intfc);
		gview_plot_interface("ERROR_IN_TEST_CROSS",intfc);
		clean_up(ERROR);
	        *status = FUNCTION_FAILED;
	        return NULL;
	    }
	    break;

	case 1:
	    if (num_t0_crossing == 0)
		return NULL;    /* merely touching */
	    else if (num_t0_crossing != 1)             /* shouldn't happen */
	    {
		screen("ERROR in test_cross(), "
		       "No.touch =1 while No.crossing >1\n");
	        print_tri(t0,intfc);
	        print_tri(t1,intfc);
	        print_interface(intfc);
		gview_plot_interface("ERROR_IN_TEST_CROSS",intfc);
		clean_up(ERROR);
	        *status = FUNCTION_FAILED;
	        return NULL;
	    }
	    break;

	case 2:
	    if (num_t0_crossing != 0)
	    {
		screen("ERROR in test_cross(), "
		       "No.touch = 2 while No.crossing != 0.\n");
	        print_tri(t0,intfc);
	        print_tri(t1,intfc);
	        print_interface(intfc);
		gview_plot_interface("ERROR_IN_TEST_CROSS",intfc);
		clean_up(ERROR);
	        *status = FUNCTION_FAILED;
	        return NULL;
	    }
	    break;
	
	default:
	    (void) printf("WARNING in test_cross(), "
	           "Unexpected co-planar tris!\n");
	    (void) printf("num_t0_touching = %d, num_t0_crossing = %d\n",
			  num_t0_touching,num_t0_crossing);
	    return test_coplanar_cross(t0,t1,num_com_vertex,status,intfc);
	}


	switch (num_t1_touching)
	{
	case 0:
	    if (num_t1_crossing == 0)
		return NULL;     /* no crossing */
	    if (num_t1_crossing != 2)                 /* shouldn't happen */
	    {
	        screen("ERROR in test_cross(), "
		       "No.touch=0 while No.crossing!=2.\n");
	        print_tri(t0,intfc);
	        print_tri(t1,intfc);
	        print_interface(intfc);
		gview_plot_interface("ERROR_IN_TEST_CROSS",intfc);
		clean_up(ERROR);
	        *status = FUNCTION_FAILED;
	        return NULL;
	    }
	    break;

	case 1:
	    if (num_t1_crossing == 0)
		return NULL;    /* merely touching */
	    if (num_t1_crossing != 1)   	       /* shouldn't happen */
	    {
		(void) printf("\ntest cross, t1: No.touch =1 ");
		(void) printf("while No.crossing >1\n");
		return NULL;
	    }
	    break;

	case 2:
	    if (num_t1_crossing != 0)
	    {
		screen("ERROR in test_cross(), "
		       "No.touch = 2 while No.crossing != 0.\n");
	        print_tri(t0,intfc);
	        print_tri(t1,intfc);
	        print_interface(intfc);
		gview_plot_interface("ERROR_IN_TEST_CROSS",intfc);
		clean_up(ERROR);
	        *status = FUNCTION_FAILED;
	        return NULL;
	    }
	    break;

	default:
	    (void) printf("WARNING in test_cross(), "
	                  "Unexpected co-planar tris!\n");
	    (void) printf("num_t1_touching = %d, num_t1_crossing = %d\n",
			  num_t1_touching,num_t1_crossing);
	    return test_coplanar_cross(t0,t1,num_com_vertex,status,intfc);
	}


	    /* label the four cross points by tri and by order */

	if (x[0].order > x[1].order)
	{
	    l0 = 1;  u0 = 0;
	}
	else
	{
	    l0 = 0;  u0 = 1;
	}

	if (x[2].order > x[3].order)
	{
	    l1 = 3;  u1 = 2;
	}
	else
	{
	    l1 = 2;  u1 = 3;
	}

	if (x[u0].order <= (x[l1].order + cr_tol))
	    return NULL;
	if (x[u1].order <= (x[l0].order + cr_tol))
	    return NULL;

		/* Create new C_BOND */

	new_cb = CBond(NULL,NULL,NULL,t0,t1);

	    /* start side of new_cb */

	if (fabs(x[l0].order - x[l1].order) <= cr_tol)
	{
	    if (x[l0].edge_vertex && x[l1].edge_vertex)
	    {
		for (i= 0; i< 3; ++i)
		{
		    x[l0].coords[i] += x[l1].coords[i];
		    x[l0].coords[i] *= 0.5;
		}
		start = x[l0].coords;
	    }
	    else if (x[l0].edge_vertex && !x[l1].edge_vertex)
	    {
		start = x[l1].coords;
		new_cb->start = Point_of_tri(t1)[x[l1].label];
	    }
	    else if (!x[l0].edge_vertex && x[l1].edge_vertex)
	    {
		start = x[l0].coords;
		new_cb->start = Point_of_tri(t0)[x[l0].label];
	    }
	    else
	    {
		start = x[l0].coords;
		new_cb->start = Point_of_tri(t0)[x[l0].label];
		substitute_a_point(new_cb->start,t1,x[l1].label,intfc);
	    }

	    new_cb->s[0].prev_t = x[l0].edge_vertex ? 
		is_side_bdry(t0,x[l0].label) ?
		    NULL : Tri_on_side(t0,x[l0].label) : NULL;

	    new_cb->s[1].prev_t = x[l1].edge_vertex ? 
		is_side_bdry(t1,x[l1].label) ?
		     NULL : Tri_on_side(t1,x[l1].label) :  NULL;

	    set_flag_start(new_cb,0,x[l0].edge_vertex,YES,x[l0].label);
	    set_flag_start(new_cb,1,x[l1].edge_vertex,YES,x[l1].label);
	}

	else if (x[l0].order > x[l1].order)
	{
	    start = x[l0].coords;
	    new_cb->s[1].prev_t = t1;

	    new_cb->s[0].prev_t = x[l0].edge_vertex ? 
		is_side_bdry(t0,x[l0].label) ?
		     NULL : Tri_on_side(t0,x[l0].label) :  NULL;

	    if (!x[l0].edge_vertex)
		new_cb->start = Point_of_tri(t0)[x[l0].label];

	    set_flag_start(new_cb,0,x[l0].edge_vertex,YES,x[l0].label);
	    set_flag_start(new_cb,1,NO,NO,-1);
	}

	else  /* if (x[l1].order > x[l0].order) */
	{
	    start = x[l1].coords;
	    new_cb->s[0].prev_t = t0;

	    new_cb->s[1].prev_t = x[l1].edge_vertex ? 
		is_side_bdry(t1,x[l1].label) ?
		     NULL : Tri_on_side(t1,x[l1].label) :  NULL;

	    if (!x[l1].edge_vertex)
		new_cb->start = Point_of_tri(t1)[x[l1].label];

	    set_flag_start(new_cb,0,NO,NO,-1);
	    set_flag_start(new_cb,1,x[l1].edge_vertex,YES,x[l1].label);
	}


	    /* end side of new_cb */

	if (fabs(x[u0].order - x[u1].order) <= cr_tol)
	{
	    if (x[u0].edge_vertex && x[u1].edge_vertex)
	    {
		for (i= 0; i< 3; ++i)
		{
		    x[u0].coords[i] += x[u1].coords[i];
		    x[u0].coords[i] *= 0.5;
		}
		end = x[u0].coords;
	    }
	    else if (x[u0].edge_vertex && !x[u1].edge_vertex)
	    {
		end = x[u1].coords;
		new_cb->end = Point_of_tri(t1)[x[u1].label];
	    }
	    else if (!x[u0].edge_vertex && x[u1].edge_vertex)
	    {
		end = x[u0].coords;
		new_cb->end = Point_of_tri(t0)[x[u0].label];
	    }
	    else
	    {
		end = x[u0].coords;
		new_cb->end = Point_of_tri(t0)[x[u0].label];
		substitute_a_point(new_cb->end,t1,x[u1].label,intfc);
	    }

	    new_cb->s[0].next_t = x[u0].edge_vertex ? 
		is_side_bdry(t0,x[u0].label) ?
		     NULL : Tri_on_side(t0,x[u0].label) :  NULL;

	    new_cb->s[1].next_t = x[u1].edge_vertex ? 
		is_side_bdry(t1,x[u1].label) ?
		     NULL : Tri_on_side(t1,x[u1].label) :  NULL;

	    set_flag_end(new_cb,0,x[u0].edge_vertex,YES,x[u0].label);
	    set_flag_end(new_cb,1,x[u1].edge_vertex,YES,x[u1].label);
	}
	else if (x[u0].order < x[u1].order)
	{
	    end = x[u0].coords;		
	    new_cb->s[1].next_t = t1;

	    new_cb->s[0].next_t = x[u0].edge_vertex ? 
		is_side_bdry(t0,x[u0].label) ?
		     NULL : Tri_on_side(t0,x[u0].label) :  NULL;

	    if (!x[u0].edge_vertex)
		new_cb->end = Point_of_tri(t0)[x[u0].label];

	    set_flag_end(new_cb,0,x[u0].edge_vertex,YES,x[u0].label);
	    set_flag_end(new_cb,1,NO,NO,-1);
	}

	else  /* if (x[u1].order < x[u0].order) */
	{
	    end = x[u1].coords;
	    new_cb->s[0].next_t = t0;

	    new_cb->s[1].next_t = x[u1].edge_vertex ? 
		is_side_bdry(t1,x[u1].label) ?
		     NULL : Tri_on_side(t1,x[u1].label) :  NULL;

	    if (!x[u1].edge_vertex)
		new_cb->end = Point_of_tri(t1)[x[u1].label];

	    set_flag_end(new_cb,0,NO,NO,-1);
	    set_flag_end(new_cb,1,x[u1].edge_vertex,YES,x[u1].label);
	}

	if (!new_cb->start)
	    new_cb->start = Point(start);
	if (!new_cb->end)
	    new_cb->end = Point(end);

#undef set_c_surf_flag
#undef set_flag_start
#undef set_flag_end

	return new_cb;
}		/*end test_cross*/

LOCAL C_BOND *test_coplanar_cross(
	TRI       *t0,
	TRI       *t1,
	int       num_com_vertex,
	boolean      *status,
	INTERFACE *intfc)
{
	C_BOND      *cb;
	FT_POINT       *pt, *ps, *pe, **pt0, **pt1;
	double       *p[6], pbar[3], lambda[3], *r[3], r0[3], r1[3], r2[3];
	double       p0[3], p1[3], v0[6], v1[6], ds[3], dn[3];
	double       nor[3], nor0[3], nor1[3];
	double       mag, mag0, mag1;
	const double *tnor;
	int         i, side0, side1, i0, ni0, pi0, i1, ni1, pi1;

	DEBUG_ENTER(test_coplanar_cross);

	if (DEBUG)
	{
	    (void) printf("Testing coplanar tris for cross\n");
	    show_crossing_tris("test_coplanar_cross",t0,t1,NULL,intfc);
	}

	tnor = Tri_normal(t0);
	mag0 = Mag3d(tnor);
	for (i = 0; i < 3; ++i)
	    nor0[i] = tnor[i]/mag0;

	tnor = Tri_normal(t1);
	mag1 = Mag3d(tnor);
	for (i = 0; i < 3; ++i)
	    nor1[i] = tnor[i]/mag1;

	for (i = 0; i < 3; ++i)
	    nor[i] = 0.5*(nor0[i] + nor1[i]);
	mag = Mag3d(nor);
	for (i = 0; i < 3; ++i)
	    nor[i] /= mag;

	pt0 = Point_of_tri(t0);
	pt1 = Point_of_tri(t1);
	cb = NULL;
	switch (num_com_vertex)
	{
	case 0:
	    r[0] = r0; r[1] = r1; r[2] = r2;
	    p[0] = Coords(pt0[0]);
	    p[1] = Coords(pt0[1]);
	    p[2] = Coords(pt0[2]);
	    p[3] = Coords(pt1[0]);
	    p[4] = Coords(pt1[1]);
	    p[5] = Coords(pt1[2]);
	    affine_fit((const double* const*)p,3,6,nor,pbar,r,lambda);
	    for (i = 0; i < 3; ++i)
	    {
	        v0[2*i]   = (Coords(pt0[i])[0] - pbar[0])*r0[0] +
	                    (Coords(pt0[i])[1] - pbar[1])*r0[1] +
	                    (Coords(pt0[i])[2] - pbar[2])*r0[2];
	        v0[2*i+1] = (Coords(pt0[i])[0] - pbar[0])*r1[0] +
	                    (Coords(pt0[i])[1] - pbar[1])*r1[1] +
	                    (Coords(pt0[i])[2] - pbar[2])*r1[2];
	        v1[2*i]   = (Coords(pt1[i])[0] - pbar[0])*r0[0] +
	                    (Coords(pt1[i])[1] - pbar[1])*r0[1] +
	                    (Coords(pt1[i])[2] - pbar[2])*r0[2];
	        v1[2*i+1] = (Coords(pt1[i])[0] - pbar[0])*r1[0] +
	                    (Coords(pt1[i])[1] - pbar[1])*r1[1] +
	                    (Coords(pt1[i])[2] - pbar[2])*r1[2];
	    }
	    if (winding_number(v0,p[3],3)%2 || winding_number(v0,p[4],3)%2 ||
	        winding_number(v0,p[5],3)%2 || winding_number(v1,p[0],3)%2 ||
	        winding_number(v1,p[1],3)%2 || winding_number(v1,p[2],3)%2 ||
		plane_segments_cross(v0  ,v0+2,v1,  v1+2) ||
		plane_segments_cross(v0+2,v0+4,v1,  v1+2) ||
		plane_segments_cross(v0+4,v0,  v1,  v1+2) ||
		plane_segments_cross(v0  ,v0+2,v1+2,v1+4) ||
		plane_segments_cross(v0+2,v0+4,v1+2,v1+4) ||
		plane_segments_cross(v0+4,v0  ,v1+2,v1+4) ||
		plane_segments_cross(v0  ,v0+2,v1+4,v1  ) ||
		plane_segments_cross(v0+2,v0+4,v1+4,v1  ) ||
		plane_segments_cross(v0+4,v0  ,v1+4,v1  ))
	    {
	        *status = FUNCTION_FAILED;
	        screen("ERROR in test_coplanar_cross(), "
	               "overlapping tris with no common vertices\n");
	        print_tri(t0,intfc);
	        print_tri(t1,intfc);
	        print_interface(intfc);
	        gview_plot_interface("ERROR_IN_test_coplanar_cross",intfc);
	        clean_up(ERROR);
	    }
	    break;

	case 1:
	    for (i0 = 0; i0 < 3; ++i0)
	    {
	        pt = pt0[i0];
	        for (i1 = 0; i1 < 3; ++i1)
		{
		    if (pt1[i1] == pt)
		        break;
		}
		if (i1 < 3)
		    break;
	    }
	    if (i0 == 3)
	    {
	        *status = FUNCTION_FAILED;
	        screen("ERROR in test_coplanar_cross(), "
	               "num_com_vertex == 1 and no common vertex\n");
	        print_tri(t0,intfc);
	        print_tri(t1,intfc);
	        print_interface(intfc);
	        gview_plot_interface("ERROR_IN_test_coplanar_cross",intfc);
	        clean_up(ERROR);
		break;
	    }
	    r[0] = r0; r[1] = r1; r[2] = r2;
	    p[0] = Coords(pt);
	    p[1] = Coords(pt0[Prev_m3(i0)]);
	    p[2] = Coords(pt0[Next_m3(i0)]);
	    p[3] = Coords(pt1[Prev_m3(i1)]);
	    p[4] = Coords(pt1[Next_m3(i1)]);
	    affine_fit((const double* const*)p,3,5,nor,pbar,r,lambda);
	    for (i = 0; i < 3; ++i)
	    {
	        v0[2*i]   = (Coords(pt0[i])[0] - pbar[0])*r0[0] +
	                    (Coords(pt0[i])[1] - pbar[1])*r0[1] +
	                    (Coords(pt0[i])[2] - pbar[2])*r0[2];
	        v0[2*i+1] = (Coords(pt0[i])[0] - pbar[0])*r1[0] +
	                    (Coords(pt0[i])[1] - pbar[1])*r1[1] +
	                    (Coords(pt0[i])[2] - pbar[2])*r1[2];
	        v1[2*i]   = (Coords(pt1[i])[0] - pbar[0])*r0[0] +
	                    (Coords(pt1[i])[1] - pbar[1])*r0[1] +
	                    (Coords(pt1[i])[2] - pbar[2])*r0[2];
	        v1[2*i+1] = (Coords(pt1[i])[0] - pbar[0])*r1[0] +
	                    (Coords(pt1[i])[1] - pbar[1])*r1[1] +
	                    (Coords(pt1[i])[2] - pbar[2])*r1[2];
	    }
	    ni0 = Next_m3(i0); pi0 = Prev_m3(i0);
	    ni1 = Next_m3(i1); pi1 = Prev_m3(i1);
	    if (winding_number(v0,p[3],3)%2 || winding_number(v0,p[4],3)%2 ||
	        winding_number(v1,p[1],3)%2 || winding_number(v1,p[2],3)%2 ||
		plane_segments_cross(v1+2*i1, v1+2*ni1,v0+2*ni0,v0+2*pi0) ||
		plane_segments_cross(v1+2*pi1,v1+2*i1, v0+2*ni0,v0+2*pi0) ||
		plane_segments_cross(v0+2*i0, v0+2*ni0,v1+2*ni1,v1+2*pi1) ||
		plane_segments_cross(v0+2*pi0,v0+2*i0, v1+2*ni1,v1+2*pi1))
	    {
	        *status = FUNCTION_FAILED;
	        screen("ERROR in test_coplanar_cross(), "
	               "overlapping tris with common vertex\n");
	        print_tri(t0,intfc);
	        print_tri(t1,intfc);
	        print_interface(intfc);
	        gview_plot_interface("ERROR_IN_test_coplanar_cross",intfc);
	        clean_up(ERROR);
	    }
	    break;

	case 2:
	    for (side0 = 0; side0 < 3; ++side0)
	    {
	        ps = pt0[side0];
	        pe = pt0[Next_m3(side0)];
		for (side1 = 0; side1 < 3; ++side1)
		{
		    if ((pt1[side1] == pe) && (pt1[Next_m3(side1)] == ps))
			break;
		}
		if (side1 < 3)
		    break;
	    }
	    if (side0 == 3)
	    {
	        *status = FUNCTION_FAILED;
	        screen("ERROR in test_coplanar_cross(), "
	               "num_com_vertex == 2 && no common side\n");
	        print_tri(t0,intfc);
	        print_tri(t1,intfc);
	        print_interface(intfc);
	        gview_plot_interface("ERROR_IN_test_coplanar_cross",intfc);
	        clean_up(ERROR);
		break;
	    }
	    i0 = Prev_m3(side0);
	    i1 = Prev_m3(side1);
	    for (i = 0; i < 3; ++i)
	    {
	        ds[i] = Coords(pe)[i] - Coords(ps)[i];
	        p0[i] = Coords(pt0[i0])[i] - Coords(ps)[i];
	        p1[i] = Coords(pt1[i1])[i] - Coords(ps)[i];
	    }
	    mag = sqrt(ds[0]*ds[0]+ds[1]*ds[1]+ds[2]*ds[2]);
	    for (i = 0; i < 3; ++i)
	        ds[i] /= mag;
	    Cross3d(ds,nor,dn);
	    if (Dot3d(p0,dn)*Dot3d(p1,dn) >= 0.0)
	    {
	        *status = FUNCTION_FAILED;
	        screen("ERROR in test_coplanar_cross(), "
	               "overlapping tris with a common side\n");
	        print_tri(t0,intfc);
	        print_tri(t1,intfc);
	        print_interface(intfc);
	        gview_plot_interface("ERROR_IN_test_coplanar_cross",intfc);
	        clean_up(ERROR);
	    }
	    break;

	case 3:
	default:
	    *status = FUNCTION_FAILED;
	    screen("ERROR in test_coplanar_cross(), "
	           "distinct tris with identical vertices\n");
	    print_tri(t0,intfc);
	    print_tri(t1,intfc);
	    print_interface(intfc);
	    gview_plot_interface("ERROR_IN_test_coplanar_cross",intfc);
	    clean_up(ERROR);
	    break;
	}
	DEBUG_LEAVE(test_coplanar_cross);
	return cb;
}		/*end test_coplanar_cross*/

LOCAL	boolean plane_segments_cross(
	double *s0,
	double *e0,
	double *s1,
	double *e1)
{
	boolean status;
	double t0, t1;
	double ds0, ds1, v00, v01, v10, v11;
	double den;

	ds0 = s1[0] - s0[0]; ds1 = s1[1] - s0[1];
	v00 = e0[0] - s0[0]; v01 = e0[1] - s0[1];
	v10 = e1[0] - s1[0]; v11 = e1[1] - s1[1];
	den = v00*v11 - v10*v01;
	if (den == 0.0)
	    return NO;
	t0 = (ds0*v11 - ds1*v10)/den;
	t1 = (ds0*v01 - ds1*v00)/den;

	status = ((0.0 <= t0) && (t0 <= 1.0) && (0.0 <= t1) && (t1 <= 1.0)) ?
	     YES : NO;

	return status;
}		/*end plane_segments_cross*/

EXPORT	C_BOND	*i_CBond(
	C_BOND *cb,
	FT_POINT  *start,
	FT_POINT  *end,
	TRI    *t1,
	TRI    *t2)
{
	if (cb == NULL)
	    cb = (C_BOND*)store(sizeof(C_BOND));
	cb->start = start;
	cb->end = end;
	cb->s[0].t = t1;
	cb->s[1].t = t2;
	return cb;
}		/*end i_CBond*/


/*
*			     sort_cross_curves():
*
*	This routine sorts the crosses into a data structure corresponding
*	to an interface.  The interface is defined by the intersections,
*	and thus it consists of one dimensional objects (curves, bonds,
*	nodes, etc) in three dimensions.  In other words it has co-dimension
*	at least two.
*
*	The sorting is done in three passes. The first tabulates all pairs of
*	intersection triangles in lists, having common surfaces s1 and s2.
*	The second pass takes the intersecting tri pairs for a fixed
*	surface pair s1 and s2, and arranges them in bond order. If
*	necessary, the bond orientation chosen locally when the intersection
*	was identified has to be identified at this point. The bond order
*	identifies maximal chains, which are the curves of the intersection
*	interface. The third pass identifies the nodes at the curve ends,
*	and identifies how the intersection curves are joined at nodes.
*/

LOCAL   C_CURVE	**sort_cross_curves(
	C_CURVE   **c_curves,
	INTERFACE *intfc)
{
	C_BOND        *cr, *first, *last, *head, *tail;
	C_CURVE       **c, **new_curves, *new_c;
	SURFACE       *s[2];

	DEBUG_ENTER(sort_cross_curves)

#define print_C_SURF_FLAG(m,f,end)					\
    (void) printf("%s label %d on_bdry %s edge_vertex %s%s",m,		\
		   cs_tri_side_index(f),y_or_n(cs_on_bdry(f)),		\
		   y_or_n(cs_edge_vertex(f)),end)

	    /* First Pass: Sort Bonds To Form Curve(s) */

	if (size_of_pointers((POINTER *)c_curves) == 0)
	{
	    DEBUG_LEAVE(sort_cross_curves)
	    return NULL;
	}
	new_curves = NULL;
	if (debugging("sort_c_curves"))
	{
	    (void) printf("Entering sort_cross_curves(), num_curves = %d\n",
			  (int)size_of_pointers((POINTER *)c_curves));
	    (void) printf("Cross curve list before sorting\n");
	    for (c = c_curves; c && *c; ++c)
		print_c_curve(*c,intfc);
	}

	for (c = c_curves; c && *c; ++c)
	{
	    s[0] = (*c)->s[0];
	    s[1] = (*c)->s[1];
	    new_c = *c;
	    new_c->num_points = 2;
	    head = tail = new_c->first;
	    cr = tail->next;
	    
	    while (cr)
	    {
		/* add at beginning (head) and restart from tail */
		if (prev_adjacent(cr,head))
		{
		    cr->end = head->start;
		    cr->prev->next = cr->next;
		    if (cr->next)
			cr->next->prev = cr->prev;

		    head->prev = cr;
		    cr->next = head;
		    cr->prev = NULL;
		    head = cr;
		    cr = tail->next;
		    new_c->num_points += 1;
		}

		/* add after tail and restart from last */
		else if (next_adjacent(cr,tail))
		{
		    cr->start = tail->end;
		    new_c->num_points += 1;
		    if (cr->prev == tail)
		    {
			tail = cr;
			cr = cr->next;
		    }
		    else
		    {
			cr->prev->next = cr->next;
			if (cr->next)
			    cr->next->prev = cr->prev;
			
			cr->next = tail->next;
			tail->next->prev = cr;	
			cr->prev = tail;
			tail->next = cr;
			tail = cr;
			cr = tail->next;
		    }
		}
		else
		{
		    cr = cr->next;
		}
		
		if (cr == NULL)		/* at end of list */
		{
		    new_c->first = head;
		    new_c->last = tail;
		    cr = tail->next;

		    if (cr != NULL)   /* more crosses */
		    {
			/* Remove max segment and Restart Loop */

			head = tail = cr;
			cr = cr->next;
			new_c->last->next = NULL;

			new_c = make_c_curve(s[0],s[1],NULL,NULL,&new_curves);
			new_c->num_points = 2;
		    }
		}
	    }
	}

	for (c = new_curves; c && *c; ++c)
	{
	    if (!add_to_pointers(*c,&c_curves))
	    {
	        screen("ERROR in sort_cross_curves(), "
	               "add_to_pointers() failed\n");
	        clean_up(ERROR);
	    }
	}

	/* adjust the cross surface order along c_curves */

	for (c = c_curves; c && *c; ++c)
	{
	    tail = (*c)->last;
	    for (cr = (*c)->first; cr; cr = cr->next)
	    {
		if ((tail->s[0].next_t == cr->s[1].t &&
		     tail->s[1].next_t == cr->s[0].t) ||
		    (tail->s[0].t == cr->s[1].prev_t &&
		     tail->s[1].t == cr->s[0].prev_t))
		{
		    C_SURF cs_tmp = cr->s[0];
		    cr->s[0] = cr->s[1];
		    cr->s[1] = cs_tmp;
		    tail = cr;
		}
		else if ((tail->s[0].next_t == cr->s[0].t &&
		     tail->s[1].next_t == cr->s[1].t) ||
		    (tail->s[0].t == cr->s[0].prev_t &&
		     tail->s[1].t == cr->s[1].prev_t))
		{
		    tail->end = cr->start;
		    break;
		}
		else
		    break;
	    }
	}


	    /* Second Pass: Sort Curves, Identify Intersection Nodes */
	
	for (c = c_curves; c && *c; ++c)
	{
	    first = (*c)->first;
	    last = (*c)->last;

	        /* Closed Loops */

	    if (prev_adjacent(last,first))
	    {
		last->end = first->start;
		continue;
	    }
	}

	for (c = c_curves; c && *c; ++c)
	    (*c)->first->prev = (*c)->last->next = NULL;

	if (debugging("sort_c_curves"))
	{
	    int k;
	    (void) printf("Leaving sort_cross_curves(), num_curves = %d\n",
			  (int)size_of_pointers((POINTER *)c_curves));
	    (void) printf("Cross curve list after sorting\n");
	    for (k = 0, c = c_curves;  c && *c;  ++k, ++c)
	    {
		print_c_curve(*c,intfc);
		if (debugging("gview_c_curve"))
		    gview_plot_c_curve(*c,k,"cross");
	    }
	}

#undef print_C_SURF_FLAG

	DEBUG_LEAVE(sort_cross_curves)
	return c_curves;
}		/*end sort_cross_curves*/



                  
/*
*			    prev_adjacent():
*
* 	Determines whether cb is the prev adjacent of head.
*	
*                               prev 
*                             <-----     
*                        cb    ----->   head  
*                               next     
*
*	TODO: the determination of prevt1, t2, nextt1, t2 in the case
*	where not an edge crossing  but rather a boundary (vertex)
*	crossing has occured; This can certainly be helped
*	by the cross list associated with each tri that is
*	the parent of at least one cross. This has to be done at the
*	sorting phase.
*/

LOCAL   boolean  prev_adjacent(
	C_BOND *cb,
	C_BOND *head)
{
	C_SURF      cs_tmp;
	FT_POINT       *p_tmp;
	TRI         *t_tmp;
	C_SURF_FLAG flag_tmp;

	if (cb == NULL || head == NULL)
	    return NO;

	if (same_cb_pt(cb->end,head->start))
	{
	    if ((cb->s[0].t == head->s[1].prev_t  &&
	         cb->s[1].t == head->s[0].prev_t) ||
	        (cb->s[0].next_t == head->s[1].t  &&
	         cb->s[1].next_t == head->s[0].t))
	    {
	    	cs_tmp = cb->s[0];
	    	cb->s[0] = cb->s[1];
	    	cb->s[1] = cs_tmp;
	    }
	    return YES;
	}

	if (same_cb_pt(cb->start,head->start))
	{
	    p_tmp = cb->start;
	    cb->start = cb->end;
	    cb->end = p_tmp;
	    flag_tmp = cb_flag_start(cb,0);
	    cb_flag_start(cb,0) = cb_flag_end(cb,0);
	    cb_flag_end(cb,0) = flag_tmp;
	    flag_tmp = cb_flag_start(cb,1);
	    cb_flag_start(cb,1) = cb_flag_end(cb,1);
	    cb_flag_end(cb,1) = flag_tmp;
	    t_tmp = cb->s[0].prev_t;
	    cb->s[0].prev_t = cb->s[0].next_t;
	    cb->s[0].next_t = t_tmp;
	    t_tmp = cb->s[1].prev_t;
	    cb->s[1].prev_t = cb->s[1].next_t;
	    cb->s[1].next_t = t_tmp;
	    if ((cb->s[0].t == head->s[1].prev_t  &&
	         cb->s[1].t == head->s[0].prev_t) ||
	        (cb->s[0].next_t == head->s[1].t  &&
	         cb->s[1].next_t == head->s[0].t))
	    {
	    	cs_tmp = cb->s[0];
	    	cb->s[0] = cb->s[1];
	    	cb->s[1] = cs_tmp;
	    }
	    return YES;
	}
	return NO;
}		/*end prev_adjacent*/



/* 
*                             next_adjacent():
*
* 	Determines whether cb is the next adjacent c_bond of  tail.
*
*	                       prev
*                             <----
*                  tail       ---->     cb  
*                              next  
*/

LOCAL   boolean  next_adjacent(
	C_BOND *cb,
	C_BOND *tail)
{
	C_SURF      cs_tmp;
	FT_POINT       *p_tmp;
	TRI         *t_tmp;
	C_SURF_FLAG flag_tmp;

	if (cb == NULL || tail == NULL)
	    return NO;

	if (same_cb_pt(cb->start,tail->end))
	{
            if ((cb->s[0].t == tail->s[1].next_t  &&
	         cb->s[1].t == tail->s[0].next_t) ||
	        (cb->s[0].prev_t == tail->s[1].t  &&
	         cb->s[1].prev_t == tail->s[0].t))
	    {
	    	cs_tmp = cb->s[0];
	    	cb->s[0] = cb->s[1];
	    	cb->s[1] = cs_tmp;
	    }
	    return YES;
	}

	if (same_cb_pt(cb->end,tail->end))
	{
	    p_tmp = cb->start;
	    cb->start = cb->end;
	    cb->end = p_tmp;
	    flag_tmp = cb_flag_start(cb,0);
	    cb_flag_start(cb,0) = cb_flag_end(cb,0);
	    cb_flag_end(cb,0) = flag_tmp;
	    flag_tmp = cb_flag_start(cb,1);
	    cb_flag_start(cb,1) = cb_flag_end(cb,1);
	    cb_flag_end(cb,1) = flag_tmp;
	    t_tmp = cb->s[0].prev_t;
	    cb->s[0].prev_t = cb->s[0].next_t;
	    cb->s[0].next_t = t_tmp;
	    t_tmp = cb->s[1].prev_t;
	    cb->s[1].prev_t = cb->s[1].next_t;
	    cb->s[1].next_t = t_tmp;
            if ((cb->s[0].t == tail->s[1].next_t  &&
	         cb->s[1].t == tail->s[0].next_t) ||
	        (cb->s[0].prev_t == tail->s[1].t  &&
	         cb->s[1].prev_t == tail->s[0].t))
	    {
	    	cs_tmp = cb->s[0];
	    	cb->s[0] = cb->s[1];
	    	cb->s[1] = cs_tmp;
	    }
	    return YES;
	}
	return NO;
}		/*end next_adjacent*/

LOCAL	boolean same_cb_pt(
	FT_POINT *p1,
	FT_POINT *p2)
{
	if (p1 == p2)
	    return YES;
	if (fabs(Coords(p1)[0] - Coords(p2)[0]) < cr_tol &&
	    fabs(Coords(p1)[1] - Coords(p2)[1]) < cr_tol &&
	    fabs(Coords(p1)[2] - Coords(p2)[2]) < cr_tol)
		return YES;
	return NO;
}	/* end same_cb_pt */


/*
*                       substitute_a_point():
*
*	Given a point p and a tri, substitute p for the vertex of the
*	tri with index index globally (throughout the interface).
*/

LOCAL  void  substitute_a_point(
	FT_POINT     *p,
	TRI       *tri,
	int       index,
	INTERFACE *intfc)
{
	FT_POINT  		*old_p;
	BOND_TRI 	**bt;
	TRI 		*t_begin, *t, **tri_list = NULL, **tl;
	boolean		break_bt;
	int		i;
	NODE 		*n;
	CURVE 		**c;
	BOND  		*b;
	static ORIENTATION  	orient[2] = {POSITIVE_ORIENTATION,
					     NEGATIVE_ORIENTATION};

	DEBUG_ENTER(substitute_a_point)

	old_p = Point_of_tri(tri)[index];
	if (old_p == p) 		/* No need to substitute */
	{
	    DEBUG_LEAVE(substitute_a_point)
	    return;
	}

	if ((n = node_at_point(old_p)) != NULL)
	{
	    node_at_point(old_p)->posn = p;
	    for (i = 0; i < 2; ++i)
	    {
		for (c = (orient[i] == POSITIVE_ORIENTATION) ?
			n->out_curves : n->in_curves; c && *c; ++c)
		{
		    b = Bond_at_node(*c,orient[i]);
		    b->start = b->start == old_p ? p : b->start;
		    b->end = b->end == old_p ? p : b->end;

		    for (bt=Btris(b); bt && *bt; ++bt)
		    {
			break_bt = NO;
			for (tl=tri_list; tl && *tl; ++tl)
			{
			    if ((*bt)->tri == *tl)
			    {
				break_bt = YES;
				break;
			    }
			}
			if (break_bt == YES)
				break;
			substitute_a_point_on_surface(p,old_p,
				(*bt)->tri,&tri_list,intfc);
		    }
		}
	    }
	}
	else
	{
	    if (is_interior_vertex(tri,old_p, &t_begin,intfc))
	    {
		t = tri;
		do
		{
		    if (old_p == Point_of_tri(t)[0])
		    {
			Point_of_tri(t)[0] = p;
			t = Tri_on_side20(t);
		    }
		    else if (old_p == Point_of_tri(t)[1])
		    {
			Point_of_tri(t)[1] = p;
			t = Tri_on_side01(t);
		    }
		    else if (old_p == Point_of_tri(t)[2])
		    {
			Point_of_tri(t)[2] = p;
			t = Tri_on_side12(t);
		    }
		    else
		    {
			screen("ERROR in substitute_a_point(), "
			       "oldp not on interior tri\n");
			clean_up(ERROR);
		    }
	        }
		while (t != tri);
	    }
	    else  /* old_p is on one curve and one curve only */
	    {
		if (old_p == Point_of_tri(t_begin)[0])
		    b = Bond_on_side01(t_begin);
		else if (old_p == Point_of_tri(t_begin)[1])
		    b = Bond_on_side12(t_begin);
		else if (old_p == Point_of_tri(t_begin)[2])
		    b = Bond_on_side20(t_begin);
		else
		{
		    screen("ERROR in substitute_a_point(), "
		           "oldp not on boundary tri\n");
		    clean_up(ERROR);
		}

		for (bt=Btris(b); bt && *bt; ++bt)
		    substitute_a_point_on_surface(p,old_p,(*bt)->tri,
			                          &tri_list,intfc);
	    }
	}
	DEBUG_LEAVE(substitute_a_point)
}		/*end substitute_a_point*/


/*
*		substitute_a_point_on_surface():
*
*       Given a point p and a tri, substitute p for the vertex of the
*	tris with index index through out the surface in which tri
*	lies.
*/

LOCAL  void  substitute_a_point_on_surface(
	FT_POINT     *p,
	FT_POINT     *old_p,
	TRI       *t,
	TRI       ***tri_list,
	INTERFACE *intfc)
{
	TRI *t_begin, *tri;
	int  done = 0;

	DEBUG_ENTER(substitute_a_point_on_surface)
	(void) is_interior_vertex(t,old_p,&t_begin,intfc);
	tri = t_begin;
	while (!done)
	{
	    if (!add_to_pointers(tri,tri_list))
	    {
	        screen("ERROR in substitute_a_point_on_surface(), "
	               "add_to_pointers() failed\n");
	        clean_up(ERROR);
	    }

	    if (old_p == Point_of_tri(tri)[0])
	    {
	        Point_of_tri(tri)[0] = p;
	        if (is_side20_a_bond(tri))
	    	    done = 1;
	        else
	    	    tri = Tri_on_side20(tri);
	    }
	    else if (old_p == Point_of_tri(tri)[1])
	    {
	        Point_of_tri(tri)[1] = p;
	        if (is_side01_a_bond(tri))
	    	    done = 1;
	        else
	    	    tri = Tri_on_side01(tri);
	    }
	    else if (old_p == Point_of_tri(tri)[2])
	    {
	        Point_of_tri(tri)[2] = p;
	        if (is_side12_a_bond(tri))
	    	    done = 1;
	        else
	    	    tri = Tri_on_side12(tri);
	    }
	    else
	    {
	        screen("ERROR in substitute_a_point_on_surface(), "
	               "oldp not on tri\n");
	        clean_up(ERROR);
	    }
	}
	DEBUG_LEAVE(substitute_a_point_on_surface)
}		/*end substitute_a_point_on_surface*/

/*
*			 make_c_curve():
*
*	Allocates a new c_curve structure in the current interface
*	consisting of a single bond joining two specified nodes. The
*	curve will be installed in a c_curve list passed to the
*	function.
*
*	Returns a pointer to the created curve or NULL on error.
*/

/* ARGSUSED */
LOCAL  C_CURVE  *make_c_curve(
	SURFACE *s0,
	SURFACE *s1,
	NODE    *start,
	NODE    *end,
	C_CURVE ***c_curves)
{
	C_CURVE  *new_c;
	INTERFACE  *intfc = current_interface();

	DEBUG_ENTER(make_c_curve)

	if ((new_c = (C_CURVE *)store(sizeof(C_CURVE))) == NULL)
	{
	     if (DEBUG)
		 (void) printf("make_c_curve returns NULL (4)\n");
	     DEBUG_LEAVE(make_c_curve)
	     return NULL;
	}

	if (!add_to_pointers(new_c,c_curves))
	{
	     if (DEBUG)
		 (void) printf("make_c_curve returns NULL (5)\n");
	     DEBUG_LEAVE(make_c_curve)
	     return NULL;
	}

	new_c->interface = intfc;
	new_c->s[0] = s0;
	new_c->s[1] = s1;
	new_c->start = start;
	new_c->end = end;
	new_c->last = new_c->first = NULL;

	new_c->num_points = -1;

	intfc->modified = YES;
	DEBUG_LEAVE(make_c_curve)
	return new_c;
}		/*end make_c_curve*/



/*ARGSUSED*/
EXPORT	void i_print_intersections3d(
	CROSS		*cross,
	INTERFACE	*intfc)
{
	int num;
	CROSS *cr;

	for (cr = cross, num = 1; cr; cr = cr->next, ++num)
	{
	    	(void) printf("Cross number %d\n",num);
		i_print_crossing_elements3d(cr,intfc);
	}
	(void) printf("Number of crossings: %d\n",num);
}		/*end i_print_intersections3d*/

/*ARGSUSED*/
LIB_LOCAL	void	i_print_crossing_elements3d(
	CROSS		*cross,
	INTERFACE	*intfc)
{
	C_CURVE *cc;
	C_BOND *cb;

	if (cross == NULL)
	    return;

	(void) printf("cross = %p\n",(POINTER)cross);
	cc = cross->c_curve;
	if (cc->boundary)
	    (void) printf("Boundary crossing\n");
	(void) printf("Number of points on crossing curve: %d\n",
		      cc->num_points);
	(void) printf("Crossing surfaces: s1 = %llu  s2 = %llu\n",
		      surface_number(cc->s[0]),surface_number(cc->s[1]));
	(void) printf("Crossing curve: curve = %llu\n",curve_number(cc->curve));
	print_curve(cc->curve);
	for (cb = cc->first; cb != NULL; cb = cb->next)
	{
	    print_bond(cb->bond);
	}

}		/*end i_print_crossing_elements3d*/

LOCAL	void	show_crossing_tris(
	const char *caller,
	TRI	   *ct0,
	TRI	   *ct1,
	C_BOND     *cbond,
	INTERFACE  *intfc)
{
	double     *pts[2];
	TRI	  **tlist = NULL;
	double     *color_tri;
	int       i, n;
	char	  ss[1024];

	(void) unique_add_to_pointers(ct0,&tlist);
	(void) unique_add_to_pointers(ct1,&tlist);
	(void) printf("Crossing tris found\n");
	(void) printf("ct0\n"); print_tri(ct0,intfc);
	for (n = 0; n < 3; ++n)
	{
	    if (is_side_bdry(ct0,n))
	    {
		(void) printf("side %d is bond:\n",n);
		print_bond(Bond_on_side(ct0,n));
	    }
	    else if (Tri_on_side(ct0,n))
	    {
	        (void) unique_add_to_pointers(Tri_on_side(ct0,n),&tlist);
		(void) printf("Tri on side %d:\n",n);
		print_tri(Tri_on_side(ct0,n),intfc);
	    }
	}
	(void) printf("ct1\n"); print_tri(ct1,intfc);
	for (n = 0; n < 3; ++n)
	{
	    if (is_side_bdry(ct1,n))
	    {
	        (void) printf("side %d is bond:\n",n);
	        print_bond(Bond_on_side(ct1,n));
	    }
	    else if (Tri_on_side(ct1,n))
	    {
	        (void) unique_add_to_pointers(Tri_on_side(ct1,n),&tlist);
	        (void) printf("Tri on side %d:\n",n);
	        print_tri(Tri_on_side(ct1,n),intfc);
	    }
	}
	n = (int) size_of_pointers(tlist);
	color_tri = (double*)store(4*n*FLOAT);
	color_tri[0] = 1.0;
	color_tri[1] = 0.0;
	color_tri[2] = 0.0;
	color_tri[3] = 1.0;
	color_tri[4] = 0.0;
	color_tri[5] = 1.0;
	color_tri[6] = 0.0;
	color_tri[7] = 1.0;
	for (i = 2; i < n; ++i)
	{
	    color_tri[4*i]   = 0.5;
	    color_tri[4*i+1] = 0.5;
	    color_tri[4*i+2] = 0.5;
	    color_tri[4*i+3] = 1.0;
	}
	if (caller != NULL)
	    (void) sprintf(ss,"%s/isect_%llu-%llu",caller,
			      tri_number(ct0,intfc),tri_number(ct1,intfc));
	else
	    (void) sprintf(ss,"isect_%llu-%llu",
			      tri_number(ct0,intfc),tri_number(ct1,intfc));
	if (cbond != NULL)
	{
	    pts[0] = Coords(cbond->start);
	    pts[1] = Coords(cbond->end);
	    gview_plot_tri_and_point_list(ss,tlist,color_tri,n,pts,
	                                  pBLUE,1.0,3,2);
	}
	else
	    gview_plot_tri_and_point_list(ss,tlist,color_tri,n,NULL,
	                                  pBLUE,0.0,0,0);
	(void) printf("\n");
}		/*end show_crossing_tris*/
#endif /* defined(THREED) */
