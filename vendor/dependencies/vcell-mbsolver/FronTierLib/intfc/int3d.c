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
*				int3d.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*		3D Interface Data Structures and Surgery on Surfaces
*	
*	     We want to describe an interface in 3 dimensions.	 The
*	main  application  is to represent discontinuous fluid flow.
*	In this case the surface  must	have  distinguishable  sides
*	(positive  vs. negative) to represent the distinct values of
*	the jump discontinuity.	 It seems that	the  sides  will  be
*	globally  meaningful,  over  any connected surface making up
*	the  interface.	  This	means  that  each  surface  will  be
*	oriented,  and	its  orientation  will	be  specified by the
*	orientation of the triangles which define  it.	 It  follows
*	that the boundary of each surface is oriented.
*	
*	     The boundary is an interface of 1D objects (CURVEs) and
*	0D  objects  (NODEs).  The curves have an intrinsic orienta-
*	tion, due to their description as a doubly linked list,	 and
*	this  orientation may agree or disagree with the orientation
*	defined by the surface they bound.
*	
*	     We will frequently have to consider the local structure
*	of the interface in the neighborhood of a single curve. At a
*	single curve, the surfaces which  bound	 it,  together	with
*	their  orientations,  are  specified.	This information (in
*	contrast to the analogous problem in two dimensions) is	 not
*	sufficient to label the local fragments of the surface which
*	join the curve. Not only may a surface bound a	curve  twice
*	with  both  orientations, as in a curve which is a slit, but
*	the surface may bound a curve an arbitrary number of  times,
*	and  so	 the orientation does not identify the surface frag-
*	ments in a neighborhood of the curve. To visualize an  exam-
*	ple,  consider	the  case of two parallel cylinders touching
*	along a common line. Because each surface must be connected,
*	we  have  to  regard  these  cylinders as distinct surfaces.
*	However in three dimensions, one may insert a handle between
*	the  two  cylinders, making the surfaces connected globally,
*	and thus necessarily represented by a single surface. In two
*	dimensions, we would have two touching circles, and the han-
*	dles between the circles will reconnect the four curve frag-
*	ments  which  meet at the common point in a different topol-
*	ogy, but they will not make all four curve fragments part of
*	a single connected simple curve.
*	
*	     There is  no  assumption  that  a	SURFACE	 bound	some
*	volume, although this will often be the case.  In particular
*	there is no assumption that the sides of surfaces  are	ade-
*	quately	 labeled  by the components.  The components provide
*	an efficient, but in general incomplete description  of	 the
*	orientation (side) information for each surface.
*	
*	     It is natural to assume that the boundary of a  surface
*	consists of some number of simple closed curves. In contrast
*	to the case of curves in a 2D interface, we  would  like  to
*	allow  this number to be zero, ie a possibly empty boundary.
*	There are two problems with this picture.  First of all, the
*	interface  will	 be  composed  of  many surfaces, which meet
*	along curves which may partially coincide and  may  be	par-
*	tially disjoint.  Thus each simple closed curve in the topo-
*	logical boundary  of  a	 single	 surface  amy  be  naturally
*	represented  as	 a  collection	of  CURVEs, joined at common
*	NODEs.	Secondly we must allow degenerate boundaries, in the
*	form  of  "slits",  i.e.   as  a  single  curve, not closed,
*	"drawn" on the surface.	 As before, such an open  curve	 may
*	consist of several CURVEs, meeting at common NODEs.
*	
*	     The "slit" CURVEs do not derive an orientation from the
*	SURFACE they bound, or more precisely, thay have both orien-
*	tations, according to the neighboring  triangles,  which  on
*	each  side of the slit, give the slit distinct orientations.
*	Thus  these  CURVEs  occur  in	both  the   pos_curves	 and
*	neg_curves   lists  and	 the  SURFACE  occurs  in  both	 the
*	pos_surfaces and the neg_surfaces lists of the CURVE.
*	
*	     The reason that the slits are needed  has	to  do	with
*	intersections.	 We want to resolve any generic intersection
*	which can arise	 between  valid	 interfaces.   SURFACEs	 are
*	required  to  be  non-selfintersecting except at their boun-
*	dries, which can only intersect if they coincide,  i.e.	 are
*	common	to the two SURFACEs. Alternately stated, we want any
*	representation which is a valid interface  in  all  respects
*	except for this non-self-intersection property to be capable
*	of being resolved into a fully valid  interface.   Now	con-
*	sider a plane intersecting with a cylinder of finite height.
*	The intersection is either topologically a circle, or it  is
*	a  line	 segment, i.e. a slit.	Thus the slits arise generi-
*	cally and are needed in the resolution of intersections.
*	
*	     In summary, the boundary of a surface is  an  arbitrary
*	number	(perhaps  zero) of simple closed curves and an arbi-
*	trary number of trees of curves. A tree is a  general  graph
*	with  no cycles. Each tree joins each simple closed curve at
*	most once and the collection of trees cannot add a cycle  to
*	the rest of the boundary. Here each curve may be represented
*	by one or more CURVEs.
*	
*	     There is no restriction that the CURVEs of an INTERFACE
*	arise  as boundaries of SURFACEs. In particular there can be
*	CURVEs but no SURFACEs in an interface.
*	
*	     We require each  SURFACE  to  be  connected,  and	thus
*	define a SURFACE to be a connected component of (interface -
*	topological 1D and 0D bdries).
*	
*	     Surgery on a SURFACE or INTERFACE concerns the  joining
*	or  disconnecting  of SURFACEs along common CURVEs or NODEs.
*	Let us consider typical problems.
*	
*	     SURFACEs may intersect, or a single SURFACE may  inter-
*	sect itself.  Such a situation is excluded by the definition
*	of an INTERFACE, but it may arise anyway.  The	goal  is  to
*	describe  the  intersection and to develop a package of rou-
*	tines which will allow surgery on the  (illegal)  interface,
*	so  that it can be reconstructed as a (legal) INTERFACE.  We
*	are concerned with intersections interior  to  the  SURFACE,
*	other  than those along existing CURVEs and NODEs, shared as
*	common boundary elements.  Thus the  intersections  will  be
*	removed	 by promoting the curves defined by the intersection
*	to CURVEs of the interface, thereby altering the boundary of
*	some  SURFACEs, and possibly dividing some SURFACEs into two
*	or more pieces. Further surgery may be required on a problem
*	dependent  basis,  after  this geometrical resolution of the
*	intersections.
*	
*	
*	     The data structure which defines the intersection is  a
*	CROSS,	and  each CROSS is a curve together with information
*	about which surface triangles it crosses, on which surfaces.
*	Also  whether  and  where  the intersection curves cross and
*	attaach to boundary CURVEs is significant and will be noted.
*	
*	     The first operation is to resolve the  triangles  which
*	define	the  surface,  so  that	 the intersection curve lies
*	along triangle boundaries. This routine is similar in struc-
*	ture  to  the PS_GRID construction.  It has a role analogous
*	to insert_point_in_bond() in a 2D INTERFACE. For this reason
*	it   could  be	called	insert_curve_in_surface_triangles().
*	There seems to be no role for the inverse to  this  function
*	for  a	3D interface.  Rather, insert_curve... will be built
*	of lower level routines, such as  split_tringle_along_line()
*	and    the    inverse	 of    this   low   level   routine,
*	merge_triangles(), will be needed.
*	
*	     The remaining surgery operation and its inverse is most
*	easily	understood  from  its analog in a 2D INTERFACE. This
*	would be merge_nodes() and  the	 inverse  divide_node.	 The
*	first  would  take two distinct nodes, and combine all in or
*	out curves at these nodes into in or out curves at a  single
*	node,  and  assign  some  new  position	 to  this node.	 The
*	inverse would divide the curves at a node into two sets, and
*	ft_assign	some  of them to a new node n1 and the rest to a new
*	node n2.  For a 3D INTERFACE, the routines could  be  called
*	merge_curve() and divide_curve().
*	
*	     How is one to determine if an additional boundary curve
*	will  disconnect a surface? We think of the curves and their
*	endpoints as defining a	 graph.	  The  boundary	 curves	 and
*	nodes  of  a given surface also define a graph.	 A new curve
*	is an edge of the graph. If both ends of the curve ie  nodes
*	are  already  part  of	the graph, and if they belong to the
*	same connected component of the graph, then  the  new  curve
*	(edge)	gives  rise  to	 a  new	 cycle in the graph and thus
*	disconnects the surface. The computation is O(num bdry nodes
*	of surface) to see if both ends of the new curve are part of
*	the surface boundary, and the operation	 count	is  at	most
*	O(num  nodes)**2  * O(num curves) to determine the connected
*	components.  In fact the components can be  computed  induc-
*	tively,	 adding one curve at a time, and O(num nodes) opera-
*	tions will place the ends of a new  curve  in  the  existing
*	components.  The  components  can  be labeled by a function,
*	defined on the nodes, and if  the  curve  connects  distinct
*	components,  the  function  is	redefined  with O(num nodes)
*	operations. It does not seem  likely  that  this  number  of
*	operations  will  be a problem, but if it is, the components
*	can be stored.
*	
*	     The surface will be described geometrically as  a	col-
*	lection	 of  triangles.	  A number of topics related to tri-
*	angulation will be discussed.
*	
*	1. Triangulation of an interface  described  in	 some  other
*	fashion,  such	as  by	analytic  equations,  and the global
*	retriangulation of a surface already described in  terms  of
*	triangles.   There  is	a considerable body of literature on
*	this topic.  We favor using a frontal strategy to sweep	 out
*	an  interface,	and  to add triangles from an outer boundary
*	inward.
*	
*	2. Local improvements in a triangulation,  to  achieve	e.g.
*	better aspect ratios, or designated local refinements.	This
*	is also a previously considered topic.	It is  necessary  to
*	construct  a  library  of local, elementary interface opera-
*	tions, as well as a strategy for using a sequence of elemen-
*	tary  operations  to  improve the fittness of a surface tri-
*	angulation.
*	
*	3. Resolution of a tangled interface, and the  specification
*	of the required new triangluation.
*	
*	4. Access to the points and triangles and storage.  In order
*	to  be able to access the points exactly once during a sweep
*	through the triangles, we  assume  that	 the  triangles	 are
*	ordered,  and  that  flags (binary bits) are included in the
*	boundary  flag	of  the	 tri,  which  indicate	whether	 the
*	Tri_on_side01(tri) (for example) tri occurs earlier or later
*	in    the    ordering.	   With	   this	   knowledge	 for
*	Tri_on_side01(tri),	     Tri_on_side12(tri),	 and
*	Tri_on_side20(tri) the points of a triangle  which  are	 new
*	(not  contained	 in a previous triangle) can be given.	Thus
*	an ordered list of triangles also generates an ordered	list
*	of  surface  points.   It appears that a double linked list,
*	with storage in the interface storage space is the best	 way
*	to store the triangles.
*	
*	5. Structured vs unstructured triangulations.	Unstructured
*	triangulations	will be used for all surfaces, except possi-
*	bly for the surfaces lying on the  external  boundary.	 The
*	external  boundary  faces  are equivalent to a 2 dimensional
*	computation, and it is possible to use an almost  structured
*	grid  there, with regular mesh points except near the inter-
*	face, and special triangles near the interface,	 as  in	 the
*	trigrid construction.
*/

#include <intfc/iloc.h>

	/* LOCAL Function Declarations */
LOCAL	COMPONENT	new_pp_index(COMPONENT,INTERFACE*);
LOCAL	TRI	*tri_at_bond(BOND*,SURFACE*);
LOCAL	boolean	update_neighbor_tris(TRI_NEIGHBOR*,TRI*,TRI*,int);
LOCAL	void	add_bdry_curve_to_hash_table(SURFACE*, SURFACE*, P_LINK*, int);
LOCAL	void	copy_tris(SURFACE*, SURFACE*);
LOCAL	void	fprint_tris_on_surface(FILE*,SURFACE*);
LOCAL	void	fprint_triangle_numbers(FILE*, TRI*);
LOCAL	void	reset_sort_status(INTERFACE*);
LOCAL	void	reset_tri_points_at_bond(TRI*,BOND*);


/*
*			i_make_surface():
*
*	Creates a new surface bounded by specified curves.  The
*	triangulation is set to NULL.
*	Returns a pointer to the created surface or NULL on error.
*	The curve arguments are lists of boundary curve pointers in
*	the interface of the surface, terminated by a NULL pointer.
*/

EXPORT SURFACE *i_make_surface(
	COMPONENT	neg_comp,
	COMPONENT	pos_comp,
	CURVE		**neg,
	CURVE		**pos)
{
	SURFACE		*news;
	INTERFACE	*cur_intfc;
	size_t		size_surface;

	cur_intfc = current_interface();
	size_surface = i_user_interface(cur_intfc).size_surface;
	if ((news = (SURFACE *)store(size_surface)) == NULL)
	    return NULL;
	if (!add_to_pointers(news,&cur_intfc->surfaces))
	    return NULL;

	Hyper_surf(news) = make_hypersurface(neg_comp,pos_comp);
	Surface_of_hs(Hyper_surf(news)) = news;
	news->obj = news;
	news->interface = cur_intfc;
	cur_intfc->modified = YES;
	while (pos && *pos)
	    install_curve_in_surface_bdry(news,*pos++,POSITIVE_ORIENTATION);
	while (neg && *neg)
	    install_curve_in_surface_bdry(news,*neg++,NEGATIVE_ORIENTATION);
	set_not_bdry(news);
	Hyper_surf_index(news) = new_pp_index(NO_COMP,cur_intfc);
	first_tri(news) = tail_of_tri_list(news);
	last_tri(news) = head_of_tri_list(news);
	news->num_tri = 0;
	news->redist_order = 1;		/*default */
	surface_for_head_of_tri_list(head_of_tri_list(news)) = news;
	return news;
}		/*end i_make_surface*/

/*
*			i_copy_surface():
*
*	Returns a copy of a specified surface.	 The copy will be allocated
*	from space in the current interface and will be installed in
*	that interface.	  The curve arguments are curve pointers in the current
*	interface which should already have been copied using copy_curve().
*	They are NULL terminated arrays of temporary storage.
*
*	THE BELOW ASSUMPTION VIOLATES OBJECT ORIENTED DESIGN
*	AND MUST BE REMOVED.
*	It is assumed that the tri index numbers for the surface have
*	already been set before this routine is called, by a call to 
*	set_tri_array_numbers.
*
*	Returns a pointer to the new surface or NULL on error.
*
*/

EXPORT SURFACE *i_copy_surface(
	SURFACE	*s,
	CURVE	**pos,
	CURVE	**neg,
	boolean    copy_triangles)
{
	SURFACE		*news;
	
	debug_print("copy_surface","Entered copy_surface()\n");
	if (s == NULL)
	    return NULL;
	news = make_surface(negative_component(s),positive_component(s),
			    neg,pos);
	if (news == NULL)
	    return NULL;

	Boundary(news) = Boundary(s);
	news->num_tri = s->num_tri;
	Hyper_surf_index(news) = Hyper_surf_index(s);
	news->redist_order = s->redist_order;

		/* Process Triangles */

	if ((s->num_tri != 0) && copy_triangles)
	    copy_tris(s,news);
	else
	    news->num_tri = 0;

	user_copy_hyper_surf(Hyper_surf(news),Hyper_surf(s));
	news->extra = s->extra;
	debug_print("copy_surface","Left copy_surface\n");
	return news;
}		/*end i_copy_surface*/

/*#bjet2 */
LOCAL	void copy_tris(
	SURFACE		*s,
	SURFACE		*news)
{
	BOND_TRI	**btris, *newbtri;
	TRI		**ntris;
	TRI		*oldtri, *newtri;
	CURVE		**oldc, **newc;
	BOND		*bond, *nbond;
	FT_POINT		*p, *pt[3];
	int		i, j;
	int		h_size;
	P_LINK		*hash_table;
	
	debug_print("copy_tris","Entered copy_tris\n");

		/* Allocate Tmp Storage for Copied Tris */

	h_size = 4*(s->interface->num_points) + 1;
	uni_array(&ntris,s->num_tri,sizeof(TRI *));
	uni_array(&hash_table,h_size,sizeof(P_LINK));
	reset_hash_table(hash_table,h_size);

	/*before copy_tris, function i_copy_surface called i_make_surface,  */
	/*it already adds curves in the surface(install_curve_in_surface_bdry).  */
	/*It means all the points in curves are already in the new surface news, */
	/*They should be added to the hash table first */
	add_bdry_curve_to_hash_table(s,news,hash_table,h_size);

		/* Copy Tris */

	for (i = 0, oldtri = first_tri(s); i < s->num_tri;
			++i, oldtri = oldtri->next)
	{
	    Tri_index(oldtri) = i;
	    for (j = 0; j < 3; ++j)
	    {
	    	p = Point_of_tri(oldtri)[j];
	    	pt[j] = (FT_POINT*)find_from_hash_table((POINTER)p,
						     hash_table,h_size);
	    	if (pt[j] == NULL)
	    	{
	    	    pt[j] = copy_point(p);
	    	    (void) add_to_hash_table((POINTER)p,(POINTER)pt[j],
				             hash_table,h_size);
	    	}
	    }

	    ntris[i] = make_tri(pt[0],pt[1],pt[2],NULL,NULL,NULL,
	    		        Boundary_tri(oldtri));
	    for (j = 0; j < 3; ++j)
	    	ntris[i]->side_length0[j] = oldtri->side_length0[j];

	    if (i)
	    {
	    	ntris[i]->prev = ntris[i-1];
	    	ntris[i-1]->next = ntris[i];
	    }
	}	

		/* Install Linked List in Surface */

	link_tri_list_to_surface(ntris[0],ntris[s->num_tri - 1],news);

		/* Set Tri Neighbors */

	for (i = 0, oldtri = first_tri(s); i < s->num_tri;
			++i, oldtri = oldtri->next)
	{
	    for (j = 0; j < 3; ++j)
	    {
	    	if (is_side_bdry(ntris[i],j))
		    continue;
	    	Tri_on_side(ntris[i],j) =
		    (Tri_on_side(oldtri,j) != NULL) ? 
			ntris[Tri_index(Tri_on_side(oldtri,j))] : NULL;
	    }
	}

		/*Set Boundary Bonds of Tris  */
	
	/*The code assume all curves are already copied to news in the SAME order */
	/*as in s. Two cases */
	/*(1) copy_buffer_surface calls the function in the following order */
	/*so this condition is satisfied. */
	/*copy_buffer_surface */
	/*    matching_curve  for positive curves */
	/*    matching_curve  for negative curves */
	/*    s = copy_surface(as,pos_curves,neg_curves,YES); */
	/*(2) i_copy_interface */
	/*    before calling copy_all_surfaces, i_copy_interface copies all the curves */

	for (oldc = s->pos_curves, newc = news->pos_curves; 
	     oldc && *oldc; ++oldc, ++newc)
	{
	    for (bond=(*oldc)->first, nbond=(*newc)->first; bond; 
	         bond = bond->next, nbond = nbond->next)
	    {
		for (btris = Btris(bond); btris && *btris; ++btris)
		{
		    if (Surface_of_tri((*btris)->tri) != s)
			continue;

		    newtri = ntris[Tri_index((*btris)->tri)];
		    newbtri = link_tri_to_bond(NULL,newtri,news,nbond,*newc);
		    assign_btri_states(newbtri, *btris);
		} 
	    }
	}

	for (oldc = s->neg_curves, newc = news->neg_curves; 
	     oldc && *oldc; ++oldc, ++newc)
	{
	    for (bond=(*oldc)->first, nbond=(*newc)->first; bond; 
	         bond = bond->next, nbond = nbond->next)
	    {
		for (btris = Btris(bond); btris && *btris; ++btris)
		{
		    if (Surface_of_tri((*btris)->tri) != s)
			continue;

		    newtri = ntris[Tri_index((*btris)->tri)];
		    newbtri = link_tri_to_bond(NULL,newtri,news,nbond,*newc);
		    assign_btri_states(newbtri, *btris);
		} 
	    }
	}

	for (i = 0, oldtri = first_tri(s); i < s->num_tri;
				++i, oldtri = oldtri->next)
	    Tri_workspace(oldtri) = (POINTER) ntris[i];
	
	free(ntris);
	free(hash_table);

	debug_print("copy_tris","Left copy_tris\n");
}		/*end copy_tris*/


/*
*			copy_all_surfaces():
*	Copies all surfaces from intfc1 to intfc2, assuming that the
*	curves are already copied and in corresponding order.
*/

LIB_LOCAL void copy_all_surfaces(
	INTERFACE	*intfc1,
	INTERFACE	*intfc2)
{
	CURVE	**pc, **pnc;
	CURVE	**npos_curves, **nneg_curves;
	SURFACE	**ps;
	size_t	num_curves;


	debug_print("copy_surface","Entered copy_all_surfaces\n");

	set_tri_array_numbers(intfc1,LOCAL_INDICES);
	num_curves = size_of_pointers(intfc1->curves);
	uni_array(&npos_curves,num_curves+1,sizeof(CURVE*));
	uni_array(&nneg_curves,num_curves+1,sizeof(CURVE*));
	for (ps = intfc1->surfaces; ps && *ps; ++ps)
	{
	    CURVE **pos_curves, **neg_curves;
	    int i;

	    for (i = 0, pos_curves = (*ps)->pos_curves; 
	    	pos_curves && *pos_curves; ++pos_curves, ++i)
	    {
	    	for (pnc = intfc2->curves, pc = intfc1->curves;
						pc && *pc; ++pc, ++pnc)
		{
		    if (*pc == *pos_curves)
			break;
		}
		if (!(pc && *pc))
		{ 
		    return;
		}
		npos_curves[i] = *pnc;
	    }
	    npos_curves[i] = NULL;

	    for (i = 0, neg_curves = (*ps)->neg_curves; 
	    	neg_curves && *neg_curves; ++neg_curves, ++i)
	    {
	    	for (pnc = intfc2->curves, pc = intfc1->curves;
	    	                                pc && *pc; ++pc, ++pnc)
		{
		    if (*pc == *neg_curves)
		        break;
		}
	    	if (!(pc && *pc))
		{ 
		    return; 	
		}	
	    	nneg_curves[i] = *pnc;
	    }
	    nneg_curves[i] = NULL;

	    (void) copy_surface(*ps,npos_curves,nneg_curves,YES);
	}
	free(npos_curves);
	free(nneg_curves);

			/* Copy Surface Order Bounding Curves */

	null_tri_array_numbers(intfc1);
	debug_print("copy_surface","Left copy_all_surfaces\n");
}		/*end copy_all_surfaces*/

/*
*			i_insert_point_in_tri():
*
*	Inserts the point p into the interior of the triangle tri;
*
*
*	Before insert:
*
*			       p1
*			      /	 \
*			     /	  \
*			    /	   \
*			   /	    \
*			  /	     \
*			 /	      \
*			/	       \
*		       /		\
*		      /			 \
*		     /	      tri	  \
*		    /			   \
*		   /			    \
*		  /			     \
*		 /			      \
*		/			       \
*	       /				\
*	      /					 \
*	     p0----------------------------------p2
*
*
*	After insert:
*				 p1
*			       / * \
*			      /	 *  \
*			     /	 *   \
*			    /	 *    \
*			   /	 *     \
*			  /	 *	\
*			 /	 *	 \
*			/	 *	  \
*		       /	 *	   \
*		      /	 tri	 *new_tri[0]\
*		     /		 *	     \
*		    /	     p2= p =p0	      \
*		   /	     *	||  *	       \
*		  /	   *	p1    *		\
*		 /	 *		*	 \
*		/      *     new_tri[1]	  *	  \
*	       /     *			    *	   \
*	      /	   *			      *	    \
*	     /	 *				*    \
*	     p0-------------------------------------p2
*
*/

EXPORT	boolean	i_insert_point_in_tri(
	FT_POINT	*p,
	TRI	*tri,
	SURFACE	*s)
{
	TRI	*new_tri[2];
	TRI	*nt;
	int	i, side;

	new_tri[0] = make_tri(p,Point_of_tri(tri)[1],Point_of_tri(tri)[2],
			      tri,Neighbor_on_side12(tri),
			      NULL,is_side12_a_bond(tri));
	insert_tri_at_tail_of_list(new_tri[0],s);
	new_tri[1] = make_tri(Point_of_tri(tri)[0],p,Point_of_tri(tri)[2],
			      tri,new_tri[0],Neighbor_on_side20(tri),
			      is_side20_a_bond(tri));
	insert_tri_at_tail_of_list(new_tri[1],s);
	if ((new_tri[0] == NULL) || (new_tri[1] == NULL))
	    return FUNCTION_FAILED;
	++s->interface->num_points;
	Tri_on_side20(new_tri[0]) = new_tri[1];
	Point_of_tri(tri)[2] = p;
	set_normal_of_tri(tri);

	for (side = 1; side < 3; ++side)
	{
	    if (Neighbor_on_side(tri,side) == NULL)
	    	continue;

	    if (is_side_bdry(tri,side))
	    {
	    	BOND_TRI *bt = Bond_tri_on_side(tri,side);
		(void) link_tri_to_bond(bt,new_tri[side-1],s,bt->bond,
					bt->curve);
	    }
	    else
	    {
	    	nt = Tri_on_side(tri,side);
	    	for (i = 0; i < 3; ++i)
	    	    if (tri == Tri_on_side(nt,i))
	    	    	Tri_on_side(nt,i) = new_tri[side-1];
	    }
	}
	Tri_on_side12(tri) = new_tri[0];
	set_12_bdry(Boundary_tri(tri),0);
	Tri_on_side20(tri) = new_tri[1];
	set_20_bdry(Boundary_tri(tri),0);
	Boundary_point(p) = 0;

	return FUNCTION_SUCCEEDED;
}		/*end i_insert_point_in_tri*/

/*ARGSUSED*/
EXPORT	boolean	i_undo_insert_point_in_tri(
	FT_POINT	*p,
	TRI	*tri,
	SURFACE	*s)
{
	TRI *tri12, *tri20;
	int bdry;

	tri12 = Tri_on_side12(tri);
	Neighbor_on_side12(tri) = Neighbor_on_side12(tri12);
	bdry = (is_side12_a_bond(tri12)) ? 1 : 0;
	set_01_bdry(Boundary_tri(tri),bdry);
	tri20 = Tri_on_side20(tri);
	Neighbor_on_side20(tri) = Neighbor_on_side20(tri20);
	bdry = (is_side20_a_bond(tri20)) ? 1 : 0;
	set_20_bdry(Boundary_tri(tri),bdry);
	Point_of_tri(tri)[2] = Point_of_tri(tri12)[2];
	set_normal_of_tri(tri);
	if (update_neighbor_tris(Tri_neighbor(tri12)+1,tri12,tri,1) !=
	     FUNCTION_SUCCEEDED)
	    return FUNCTION_FAILED;
	if (update_neighbor_tris(Tri_neighbor(tri20)+2,tri20,tri,2) !=
	    FUNCTION_SUCCEEDED)
	    return FUNCTION_FAILED;
	
	remove_tri_from_surface(tri12,s,NO);
	remove_tri_from_surface(tri20,s,NO);
	s->interface->num_points--;
	return FUNCTION_SUCCEEDED;
}		/*end i_undo_insert_point_in_tri*/

/*
*			i_insert_point_in_tri_side():
*
*	Inserts the point p onto the side of the triangle tri;
*	The triangle neighboring tri is also split in a similar fashion.
*
*
*	Before insert:
*
*			       p[(side+1)%2]
*			      /	 \
*			     /	  \
*			    /	   \
*			   /	    \
*			  /	     \
*			 /	      \
*			/	       \
*		       /		\
*		      /			 \
*		     /	      tri	  \
*		    /			   \
*		   /			    \
*		  /			     \
*		 /			      \
*		/			       \
*	       /				\
*	      /					 \
*	     p[side]-----------------------------p[(side+2)%33
*
*
*	After insert:
*			       p[(side+1)%3]
*			      /	 \
*			     /	  \
*			    /	   \
*			   /	    \
*			  /	     \
*			 /  new_tri   \
*			/	       \
*		       /		\
*	p[(side+1)%3]=p=p[side]		 \
*		     /	  *		  \
*		    /	     *		   \
*		   /		*	    \
*		  /		   *	     \
*		 /   tri	     *	      \
*		/			*      \
*	       /			   *	\
*	      /				       * \
*	     p[side]-----------------------------p[(side+2)%3
*
*/

EXPORT	boolean	i_insert_point_in_tri_side(
	FT_POINT	*p,
	int	side,
	TRI	*tri,
	SURFACE	*s)
{
	FT_POINT        *pp[3];
	POINTER      neighbor[3];
	TRI	     *nbr_tri, *new_nbr_tri, *new_tri;
	int	     siden, sidep, nbr_side, nbr_siden, nbr_sidep, bdry;

	if (is_side_bdry(tri,side))
	{
	    BOND_TRI *bt;
	    if ((bt = Bond_tri_on_side(tri,side)) == NULL)
		return FUNCTION_FAILED;
	    return insert_point_in_bond(p,bt->bond,bt->curve);
	}
	nbr_tri = Tri_on_side(tri,side);
	for (nbr_side = 0; nbr_side < 3; ++nbr_side)
	    if (Tri_on_side(nbr_tri,nbr_side) == tri)
		break;
	if (nbr_side == 3)
	    return FUNCTION_FAILED;

	/* Split tri */
	siden = Next_m3(side);
	sidep = Prev_m3(side);
	pp[side] = p;
	pp[siden] = Point_of_tri(tri)[siden];
	pp[sidep] = Point_of_tri(tri)[sidep];
	neighbor[side] = NULL;
	neighbor[siden] = Neighbor_on_side(tri,siden);
	neighbor[sidep] = (POINTER) tri;
	bdry = Boundary_tri(tri);
	set_side_bdry(bdry,sidep,0);
	new_tri = make_tri(pp[0],pp[1],pp[2],
			   neighbor[0],neighbor[1],neighbor[2],bdry);
	insert_tri_at_tail_of_list(new_tri,s);
	Point_of_tri(tri)[siden] = p;
	Tri_on_side(tri,siden) = new_tri;
	Tri_on_side(tri,side) = NULL;
	set_side_bdry(Boundary_tri(tri),siden,0);
	set_normal_of_tri(tri);
	++s->interface->num_points;

	/* Split nbr_tri */
	nbr_siden = Next_m3(nbr_side);
	nbr_sidep = Prev_m3(nbr_side);
	pp[nbr_side] = p;
	pp[nbr_siden] = Point_of_tri(nbr_tri)[nbr_siden];
	pp[nbr_sidep] = Point_of_tri(nbr_tri)[nbr_sidep];
	neighbor[nbr_side] =
	    (Point_of_tri(tri)[side] == Point_of_tri(nbr_tri)[nbr_side]) ? 
	    (POINTER) new_tri : (POINTER) tri;
	neighbor[nbr_siden] = Neighbor_on_side(nbr_tri,nbr_siden);
	neighbor[nbr_sidep] = (POINTER) nbr_tri;
	bdry = Boundary_tri(nbr_tri);
	set_side_bdry(bdry,nbr_sidep,0);
	new_nbr_tri = make_tri(pp[0],pp[1],pp[2],
			       neighbor[0],neighbor[1],neighbor[2],bdry);
	insert_tri_at_tail_of_list(new_nbr_tri,s);
	Point_of_tri(nbr_tri)[nbr_siden] = p;
	Tri_on_side(nbr_tri,nbr_siden) = new_nbr_tri;
	set_side_bdry(Boundary_tri(nbr_tri),nbr_siden,0);
	set_normal_of_tri(nbr_tri);
	Boundary_point(p) = 0;

	/* Set cross neighbors */
	if (Point_of_tri(tri)[side] == Point_of_tri(nbr_tri)[nbr_side])
	{
	    Tri_on_side(new_tri,side) = new_nbr_tri;
	}
	else
	{
	    Tri_on_side(new_tri,side) = nbr_tri;
	    Tri_on_side(nbr_tri,nbr_side) = new_tri;
	    Tri_on_side(tri,side) = new_nbr_tri;
	}
	if (update_neighbor_tris(Tri_neighbor(new_tri)+siden,tri,
				 new_tri,siden) != FUNCTION_SUCCEEDED)
	    return FUNCTION_FAILED;
	if (update_neighbor_tris(Tri_neighbor(new_nbr_tri)+nbr_siden,
				 nbr_tri,new_nbr_tri,nbr_siden)
				 != FUNCTION_SUCCEEDED)
	    return FUNCTION_FAILED;

	return FUNCTION_SUCCEEDED;
}		/*end i_insert_point_in_tri_side*/

EXPORT	boolean	i_undo_insert_point_in_tri_side(
	FT_POINT	*p,
	int	side,
	TRI	*tri,
	SURFACE	*s)
{
	TRI *ntri[3];
	TRI *nbr_tri, *new_nbr_tri, *new_tri;
	int siden, nbr_side, nbr_siden, bdry;
	int i, nside[3];

	siden = Next_m3(side);
	if (is_side_bdry(tri,side))
	{
	    BOND_TRI  *bt;
	    BOND      *b;
	    CURVE     *c;
	    if ((bt = Bond_tri_on_side(tri,side)) == NULL)
		return FUNCTION_FAILED;
	    b = bt->bond;
	    c = bt->curve;
	    if (Point_of_tri(tri)[siden] == b->end)
		delete_start_of_bond(b->next,c);
	    else if (Point_of_tri(tri)[siden] == b->start)
		delete_start_of_bond(b,c);
	    else
		return FUNCTION_FAILED;
	}
	new_tri = ntri[0] = Tri_on_side(tri,side);
	for (i = 0; i < 3; ++i)
	{
	    if (Tri_on_side(ntri[0],i) == tri)
	    {
		nside[0] = i;
		break;
	    }
	}
	if (i == 3)
	    return FUNCTION_FAILED;
	ntri[1] = Tri_on_side(tri,siden);
	for (i = 0; i < 3; ++i)
	{
	    if (Tri_on_side(ntri[1],i) == tri)
	    {
		nside[1] = i;
		break;
	    }
	}
	if (i == 3)
	    return FUNCTION_FAILED;
	ntri[2] = Tri_on_side(ntri[1],side);
	for (i = 0; i < 3; ++i)
	{
	    if (Tri_on_side(ntri[2],i) == ntri[1])
	    {
		nside[2] = i;
		break;
	    }
	}
	if (i == 3)
	    return FUNCTION_FAILED;
	if (Point_of_tri(ntri[0])[nside[0]] == p)
	{
	    new_nbr_tri = ntri[0];
	    nbr_tri = ntri[2];
	    nbr_side = nside[0];
	}
	else if (Point_of_tri(ntri[2])[nside[2]] == p)
	{
	    new_nbr_tri = ntri[2];
	    nbr_tri = ntri[0];
	    nbr_side = nside[1];
	}
	else
	    return FUNCTION_FAILED;
	nbr_siden = Next_m3(nbr_side);

	Point_of_tri(tri)[siden] = Point_of_tri(new_tri)[siden];
	set_normal_of_tri(tri);
	Neighbor_on_side(tri,siden) = Neighbor_on_side(new_tri,siden);
	Neighbor_on_side(tri,side) = nbr_tri;
	bdry = (is_side_bdry(new_tri,siden)) ? 1 : 0;
	set_side_bdry(Boundary_tri(tri),siden,bdry);

	Point_of_tri(nbr_tri)[nbr_siden] = Point_of_tri(new_nbr_tri)[nbr_siden];
	set_normal_of_tri(nbr_tri);
	Neighbor_on_side(nbr_tri,nbr_siden) =
	    Neighbor_on_side(new_nbr_tri,nbr_siden);
	Neighbor_on_side(nbr_tri,nbr_side) = tri;
	bdry = (is_side_bdry(new_nbr_tri,nbr_siden)) ? 1 : 0;
	set_side_bdry(Boundary_tri(nbr_tri),nbr_siden,bdry);

	if (update_neighbor_tris(Tri_neighbor(tri)+siden,new_tri,
				 tri,siden) != FUNCTION_SUCCEEDED)
	    return FUNCTION_FAILED;
	if (update_neighbor_tris(Tri_neighbor(nbr_tri)+nbr_siden,
				 new_nbr_tri,nbr_tri,nbr_siden)
				 != FUNCTION_SUCCEEDED)
	    return FUNCTION_FAILED;

	remove_tri_from_surface(new_tri,s,NO);
	remove_tri_from_surface(new_nbr_tri,s,NO);
	s->interface->num_points--;
	return FUNCTION_SUCCEEDED;
}		/*end i_undo_insert_point_in_tri_side*/

LOCAL  boolean update_neighbor_tris(
	TRI_NEIGHBOR *nb,	/* nbhr of new_t */
	TRI	     *old_t,	/* replace old_t by new_t as nbhr */
	TRI	     *new_t,
	int	     side)	/* side of old_t and new_t */
{
	int		i;

	if (nb == NULL)
	    return FUNCTION_SUCCEEDED;

		/* replace tri by new_t in btris of nb_tri, as a bond */
	if (is_side_bdry(new_t,side))	/* nb is a bond */
	{
	    BOND_TRI *nb_bt = nb->btri;
	    (void) link_tri_to_bond(nb_bt,new_t,nb_bt->surface,nb_bt->bond,
				    nb_bt->curve);
	    return FUNCTION_SUCCEEDED;
	}
	else /* nb is a tri */
	{
	    TRI *nb_tri = nb->tri;
	    for (i = 0; i < 3; ++i)
	    {
	        if (Tri_on_side(nb_tri,i) == old_t)
		{
	    	    Tri_on_side(nb_tri,i) = new_t;
	            return FUNCTION_SUCCEEDED;
		}
	    }
	}
	return FUNCTION_FAILED;
}		/*end update_neighbor_tris*/


EXPORT	void	insert_tri_at_head_of_list(
	TRI	*tri,
	SURFACE	*s)
{
	int i;
	tri->next = first_tri(s);
	first_tri(s)->prev = tri;
	first_tri(s) = tri;	
	first_tri(s)->prev = head_of_tri_list(s);
	++s->num_tri;
	for (i = 0; i < 3; ++i)
	{
	    if (is_side_bdry(tri,i))
	    {
		BOND_TRI *bt = Bond_tri_on_side(tri,i);
		if (bt != NULL)
		    (void) link_tri_to_bond(bt,tri,s,bt->bond,bt->curve);
	    }
	}
	tri->surf = s;
}		/*end insert_tri_at_head_of_list*/


EXPORT	void	insert_tri_at_tail_of_list(
	TRI	*tri,
	SURFACE	*s)
{
	int i;
	tri->prev = last_tri(s);
	last_tri(s)->next = tri;
	last_tri(s) = tri;
	last_tri(s)->next = tail_of_tri_list(s);
	++s->num_tri;
	for (i = 0; i < 3; ++i)
	{
	    if (is_side_bdry(tri,i))
	    {
		BOND_TRI *bt = Bond_tri_on_side(tri,i);
		if (bt != NULL)
		    (void) link_tri_to_bond(bt,tri,s,bt->bond,bt->curve);
	    }
	}
	tri->surf = s;
}		/*end insert_tri_at_tail_of_list*/

EXPORT	void	remove_tri_from_surface(
	TRI	*tri,
	SURFACE	*s,
	boolean preserve_neighbors)
{
	int i,j;

	if (tri == NULL)
	    return;

	--s->num_tri;
	if (tri == first_tri(s))
	{
	    first_tri(s) = tri->next;
	    first_tri(s)->prev = head_of_tri_list(s);
	}
	else
	    tri->prev->next = tri->next;
	if (tri == last_tri(s))
	{
	    last_tri(s) = tri->prev;
	    last_tri(s)->next = tail_of_tri_list(s);
	}
	else
	    tri->next->prev = tri->prev;
	if (!preserve_neighbors)
	{
	    for (i = 0; i < 3; ++i)
	    {
	        if (is_side_bdry(tri,i))
	        {
		    BOND_TRI *bt = Bond_tri_on_side(tri,i);
		    BOND_TRI **btris;
		    if (bt != NULL)
	       	    {
			btris = Btris(bt->bond);
		        bt->tri = NULL;
			bt->bond = NULL;
			bt->curve = NULL;
		        bt->surface = NULL;
			(void) delete_from_pointers(bt,&btris);
	            }
	        }
		else
		{
		    TRI *nbtri = Tri_on_side(tri,i);
		    if (nbtri != NULL)
		    {
		    	for (j = 0; j < 3; ++j)
			{
			    if (Tri_on_side(nbtri,j) == tri)
			    {
			    	Tri_on_side(nbtri,j) = NULL;
				break;
			    }
			}
		    }
		}
	        Neighbor_on_side(tri,i) = NULL;
		Tri_workspace(tri) = NULL;
	    }
	}
}		/*end remove_tri_from_surface*/


/*
*			i_fprint_surface():
*
*	Prints a formatted surface. It is assumed that the tri index numbers
*	have already been set before this routine is called.
*/

LIB_LOCAL void i_fprint_surface(
	FILE		*file,
	SURFACE		*s)
{
	size_t num_pos_c, num_neg_c;
	int    num_tri;
	int    i, num_points;
	CURVE  **pos_cur,**neg_cur;
	TRI    *tri;
	FT_POINT  *p;

	(void) fprintf(file,"\tSurface %llu:\n",surface_number(s));
	if (s == NULL)
	{
	    (void) fprintf(file,"\t\t NULL Surface\n\tEnd of Surface\n\n");
	    return;
	}
	(void) fprintf(file,"\tHypersurface of surface = %llu\n",
		       hypersurface_number(Hyper_surf(s)));
	(void) fprintf(file,"\tHypersurface index = %d\n",
		       Hyper_surf_index(s));
	(void) fprintf(file,"\tPositive Component = %-4d   "
			    "Negative Component = %-4d    ",
		            positive_component(s),negative_component(s));
	(void) fprintf(file,"%s\n\n",
		       is_bdry(s) ? "Boundary Surface" : "Interior Surface");

	num_pos_c = Num_pos_curves_of_surface(s);
	num_neg_c = Num_neg_curves_of_surface(s);
	pos_cur = s->pos_curves;
	neg_cur = s->neg_curves;

	(void) fprintf(file,"\t%lu Positively Oriented Bounding Curves : ",
		       num_pos_c);
	while (num_pos_c--)
		(void) fprintf(file,"%llu ",curve_number(*pos_cur++));
	(void) fprintf(file,"\n");
	(void) fprintf(file,"\t%lu Negatively Oriented Bounding Curves : ",
		       num_neg_c);
	while (num_neg_c--)
		(void) fprintf(file,"%llu ",curve_number(*neg_cur++));
	(void) fprintf(file,"\n\n");

	for (tri = first_tri(s); !at_end_of_tri_list(tri,s); tri = tri->next)
	{
		Index_of_point(Point_of_tri(tri)[0]) =
		    Index_of_point(Point_of_tri(tri)[1]) =
		    Index_of_point(Point_of_tri(tri)[2]) = ERROR;
	}
	num_points = 0;
	for (tri = first_tri(s); !at_end_of_tri_list(tri,s); tri = tri->next)
	{
	    for (i = 0; i < 3; ++i)
	    {
	    	p = Point_of_tri(tri)[i];
	    	if (Index_of_point(p) == ERROR)
	    	    Index_of_point(p) = num_points++;
	    }
	}
	(void) fprintf(file,"\t%d Points on Surface\n",num_points);

	num_tri = s->num_tri;
	(void) fprintf(file,"\t%d Triangles on Surface\n",num_tri);
	for (tri = first_tri(s); !at_end_of_tri_list(tri,s); tri = tri->next)
	    fprint_triangle_numbers(file,tri);

	user_fprint_surface(file,s);
	(void) fprintf(file,"\tEnd of Surface\n\n");
}		/*end i_fprint_surface*/

/*
*			read_print_surface():
*
*	This routine reads a formatted surface from a file and installs it
*	in a given interface. It is assumed that the nodes and curves for
*	this interface have already been read and installed in the interface.
*
*	The triangle boundary information is printed as an index number, for
*	a triangle boundary and as an address for a bond boundary.
*
*	TO DO too many arguments
*/

/*ARGSUSED*/
LIB_LOCAL SURFACE *read_print_surface(
	INTERFACE	    *intfc,
	const IO_TYPE       *io_type,
	int		    s_index,
	INTERFACE_ADDRESSES *iaddr,
	boolean                overlay)
{
	FILE		*file = io_type->file;
	COMPONENT	pcomp = 0, ncomp = 0;
	CURVE		**newc;
	CURVE		**pos_curves, **neg_curves;
	FT_POINT		**new_pts;
	SURFACE		*s;
	TRI		**new_tris, *tri;
	int		j, k, n_pos_curves, n_neg_curves, ncurves, ntris;
	ORIENTATION	orient;
	int		npts;
	uint64_t	old_surf, old_curve;
	size_t		size_tri;
	int		indx[3],hs_index;
	char		bdry_string[20];
	char		indx_str[3][20];

	debug_print("restart","Entered read_print_surface\n");

	ncurves = iaddr->num_curves;
	if (ncurves != size_of_pointers(intfc->curves))
	{
	    screen("ERROR in read_print_surface(), "
	    	   "inconsistent interface curve list\n");
	    clean_up(ERROR);
	    debug_print("restart","Left read_print_surface\n");
	    return NULL;
	}

		/* Read Components and Boundary Flag: */

	(void) fgetstring(file,"Surface");
	(void) fscanf(file,"%llu:",&old_surf);
	iaddr->surfaces[s_index] = old_surf;
	(void) fgetstring(file,"Hypersurface index =");
	(void) fscanf(file,"%d:",&hs_index);
	(void) fgetstring(file,"Positive");
	(void) fscanf(file,"%*s %*s %d %*s %*s %*s %d %s %*s",
		      &pcomp,&ncomp,bdry_string);

		/* Determine the CURVES which bound it: */

	(void) fscanf(file,"%d %*s %*s %*s %*s %*s",&n_pos_curves);
	iaddr->num_pcurves[s_index] = n_pos_curves;
	uni_array(&pos_curves,n_pos_curves+1,sizeof(CURVE *));
	uni_array(iaddr->pcurves+s_index,n_pos_curves,sizeof(uint64_t));
	for (j = 0; j < n_pos_curves; ++j)
	{
	    (void) fscanf(file,"%llu",&old_curve);
	    iaddr->pcurves[s_index][j] = old_curve;
	    for (k = 0; k < ncurves; ++k)
	    	if (old_curve == iaddr->curves[k])
		    break;
	    if (k == ncurves)
	    {
	    	screen("ERROR in read_print_surface(), "
	    	       "inconsistent pos curve list\n");
	    	clean_up(ERROR);
	        debug_print("restart","Left read_print_surface\n");
	    	return NULL;
	    }
	    pos_curves[j] = intfc->curves[k];
	}
	pos_curves[n_pos_curves] = NULL;
	(void) fscanf(file,"%d %*s %*s %*s %*s %*s",&n_neg_curves);
	iaddr->num_ncurves[s_index] = n_neg_curves;
	uni_array(&neg_curves,n_neg_curves+1,sizeof(CURVE *));
	uni_array(iaddr->ncurves+s_index,n_neg_curves,sizeof(uint64_t));
	for (j = 0; j < n_neg_curves; ++j)
	{
	    (void) fscanf(file,"%llu",&old_curve);
	    iaddr->ncurves[s_index][j] = old_curve;
	    for (k = 0; k < ncurves; ++k)
	    	if (old_curve == iaddr->curves[k])
		    break;
	    if (k == ncurves)
	    {
	    	screen("ERROR in read_print_surface(), "
	    	       "inconsistent neg curve list\n");
	    	clean_up(ERROR);
	        debug_print("restart","Left read_print_surface\n");
	    	return NULL;
	    }
	    neg_curves[j] = intfc->curves[k];
	}
	neg_curves[n_neg_curves] = NULL;

	s = make_surface(ncomp,pcomp,neg_curves,pos_curves);
	Hyper_surf_index(s) = hs_index;
	if (strcmp(bdry_string,"Boundary") == 0)
	    set_is_bdry(s);
	else
	    set_not_bdry(s);
	size_tri = i_user_interface(s->interface).size_tri;

			/* Read Triangles */

	(void) fscanf(file,"%d %*s %*s %*s",&npts);
	(void) fscanf(file,"%d %*s %*s %*s",&ntris);
	if (ntris == 0)
	{
	    (void) printf("WARNING in read_print_surface(), "
			  "no tris on surface\n");
	    user_read_print_surface(s,io_type,overlay);
	    (void) fgetstring(file,"End of Surface");
	    debug_print("restart","Left read_print_surface\n");
	    return s;
	}

	uni_array(&new_pts,npts,sizeof(FT_POINT *));
	for (j = 0; j < npts; ++j) 
	{
	    new_pts[j] = Point(NULL);
	    Index_of_point(new_pts[j]) = j;
	}

	uni_array(&new_tris,ntris,sizeof(TRI *));
	for (j = 0; j < ntris; ++j)
	    new_tris[j] = (TRI *)store(size_tri);

	new_tris[0]->prev = head_of_tri_list(s);
	new_tris[0]->next = tail_of_tri_list(s);
	first_tri(s) = last_tri(s) = new_tris[0];
	s->num_tri = 1;
	new_tris[0]->surf = s;

	for (j = 1; j < ntris; ++j)
	    insert_tri_at_tail_of_list(new_tris[j],s);

	for (j = 0; j < ntris; ++j)
	{
	    tri = new_tris[j];
	    (void) fscanf(file,"%*s %*d %*s %*s %s %*s %s %*s %s %*s %d\n",
			       indx_str[0],indx_str[1],indx_str[2],
			       &Boundary_tri(tri));

	    for (k = 0; k < 3; ++k)
	    {
	        if (strcmp(indx_str[k],"NULL") == 0)
	    	    Tri_on_side(tri,k) = NULL;
		else
		{
		    (void) sscanf(indx_str[k],"%d",&Index_on_side(tri,k));
		    if (!is_side_bdry(tri,k))
			Tri_on_side(tri,k) = new_tris[Index_on_side(tri,k)];
		    else
			Bond_tri_on_side(tri,k) = NULL;
		}
	    }

	    (void) fgetstring(file,"Points - Indices");
	    (void) fscanf(file,"%d %d %d",indx,indx+1,indx+2);
	    for (k = 0; k < 3; ++k)
	    	Point_of_tri(tri)[k] = new_pts[indx[k]];
	    (void) fgetstring(file,"Positions");
	    if (getc(file) != '\f')	/* NOBINARY */
	    {
	        static const char *fmt = "%lf %lf %lf %*s %lf %lf %lf %*s %lf %lf %lf";
	        (void) fscanf(file,fmt,Coords(Point_of_tri(tri)[0]),
				       Coords(Point_of_tri(tri)[0])+1,
	    			       Coords(Point_of_tri(tri)[0])+2,
				       Coords(Point_of_tri(tri)[1]),
				       Coords(Point_of_tri(tri)[1])+1,
				       Coords(Point_of_tri(tri)[1])+2,
				       Coords(Point_of_tri(tri)[2]),
				       Coords(Point_of_tri(tri)[2])+1,
				       Coords(Point_of_tri(tri)[2])+2);
	    }
	    else
	    {
	        (void) getc(file);
		(void) read_binary_real_array(Coords(Point_of_tri(tri)[0]),3,
		                              io_type);
		(void) read_binary_real_array(Coords(Point_of_tri(tri)[1]),3,
		                              io_type);
		(void) read_binary_real_array(Coords(Point_of_tri(tri)[2]),
		                              3,io_type);
	    }
	    Tri_index(tri) = j;	
	    set_normal_of_tri(tri);
	}

		/* Set Boundary Bonds of Tris */

	orient = POSITIVE_ORIENTATION;
	for (newc = s->pos_curves; ; ++newc)
	{
	    BOND *nbond;
	    int  ibond, itri;

	    /* Test both pos and neg oriented curves */

	    if (!(newc && *newc))
	    {
	    	if (orient == POSITIVE_ORIENTATION)
	    	{
	    	    newc = s->neg_curves;
	    	    orient = NEGATIVE_ORIENTATION;
	    	    if (newc == NULL)
	    	        break;
	    	    newc--;
	    	    continue;
	    	}
	    	else
		    break;
	    }

	    /* Reject second occurence of same curve as both pcomp and ncomp */

	    if (orient == NEGATIVE_ORIENTATION)
	    {
		if (pointer_is_in_array(*newc,s->pos_curves))
		    continue;
	    }
	    if (!index_of_pointer_in_array(*newc,intfc->curves,&j))
	    {
	    	screen("ERROR in read_print_surface(), "
	    	       "inconsistent curve list\n");
	    	clean_up(ERROR);
	        debug_print("restart","Left read_print_surface\n");
	    	return NULL;
	    }
	    
	    /**************IMPORTANT NOTE*****************************/
	    /*
	    *	The construction below makes the implicit assumption
	    *	that the triangles in the Btris(bond) list along a given
	    *	curve are parallel in the sense the for any pair of
	    *	bonds b1 and b2 on a curve c,  and any index i,
	    *	Btris(b1)[i] and Btris(b2)[i] lie on the same surface
	    *	with the same orientation with respect to c.  The use of
	    *	this assumption allows the correspondence code to check
	    *	the surfaces of only the first bond on c.
	    */

	    for (k = 0; k < iaddr->num_surfs[j]; ++k)
	    {
	    	if (old_surf != iaddr->surfs[j][k])
		    continue;

	    	ibond = 0; nbond = (*newc)->first;
	    	for (; nbond != NULL; nbond = nbond->next, ++ibond)
	    	{
	    	    itri = iaddr->tris[j][ibond][k];
	    	    tri = new_tris[itri];

	    	    /*
	    	     * Reset the point pointer for a bond
	    	     * to the pointers for its node.
	    	     * Reset point pointers for a tri to
	    	     * the pointers for its bond.
	    	     * Otherwise, bonds and nodes will be
	    	      * reset by following surfaces.
	    	    */

		    reset_tri_points_at_bond(tri,nbond);
	    	    (void) link_tri_to_bond(NULL,tri,s,nbond,*newc);
	    	}
	    }
	}
	free_these(4,new_tris,pos_curves,neg_curves,new_pts);
	user_read_print_surface(s,io_type,overlay);
	(void) fgetstring(file,"End of Surface");
	debug_print("restart","Left read_print_surface\n");
	return s;
}		/*end read_print_surface*/


EXPORT SURFACE *i_read_surface(
	INTERFACE	*intfc,
	int		s_index)
{
	SURFACE		*s;
	int		i, k, num_pos_curve, num_neg_curve, pcomp, ncomp;

	screen("Enter Pos Neg Components of Surface %d: ",s_index);
	(void) Scanf("%d %d\n",&pcomp,&ncomp);
	s = make_surface(ncomp,pcomp,NULL,NULL);
	screen("Enter Number of Pos, Neg Boundary Curves: ");
	(void) Scanf("%d %d\n",&num_pos_curve,&num_neg_curve);
	for (k = 0; k < num_pos_curve; ++k)
	{
	    screen("Enter Curve Number for Pos Curve: ");
	    (void) Scanf("%d\n",&i);
	    install_curve_in_surface_bdry(s,intfc->curves[i-1],
				          POSITIVE_ORIENTATION);
	}
	for (k = 0; k < num_neg_curve; ++k)
	{
	    screen("Enter Curve Number for Neg Curve: ");
	    (void) Scanf("%d\n",&i);
	    install_curve_in_surface_bdry(s,intfc->curves[i-1],
			                  NEGATIVE_ORIENTATION);
	}
	screen("\n");
	if (debugging("read_surface"))
	    print_surface(s);
	debug_print("read_surface","Left read_surface\n");
	return s;
}		/*end i_read_surface*/

EXPORT int i_delete_surface(
	SURFACE		*s)
{
	BOND		*b;
	BOND_TRI	**btris;
	CURVE		**c, **clist;
	INTERFACE       *intfc;
	SURFACE		***slist;
	int		i;

	if (s==NULL || s->interface==NULL || s->interface->surfaces==NULL)
	    return 0;

	intfc = s->interface;
	for (c = clist = s->pos_curves; ; ++c)
	{
	    if (c == NULL || *c == NULL)
	    {
	    	if (clist == s->pos_curves)
	    	    c = clist = s->neg_curves;
	    	if (c == NULL || *c == NULL)
	    	    break;
	    }
	    slist = (clist == s->pos_curves) ?	&(*c)->pos_surfaces :
	    					&(*c)->neg_surfaces;

	    for (i=0, btris=Btris((*c)->first); btris && *btris; ++i, ++btris)
	    {
	      if ((*btris)->surface == s)
	      {
	        for (b = (*c)->first; b; b = b->next)
	        {
	          if (!delete_from_ordered_pointers_at_location(i,&Btris(b)))
	  	    return 0;
	     	}
	     	if (Btris((*c)->first) == NULL)
	          break;
	     	btris--;
	     	i--;
	      }
	    }
	    if (!delete_from_pointers(s,slist))
	        return 0;
	}

	if (!delete_from_pointers(s,&intfc->surfaces))
	    return 0;
	if (!delete_from_pointers(Hyper_surf(s),&hyper_surf_list(intfc)))
	    return 0;
	intfc->modified = YES;
	Hyper_surf(s)->interface = s->interface = NULL;
	return 1;
}		/*end i_delete_surface*/

EXPORT TRI *i_make_tri(
	FT_POINT		*p0,
	FT_POINT		*p1,
	FT_POINT		*p2,
	POINTER		neighbor01,
	POINTER		neighbor12,
	POINTER		neighbor20,
	int		bdry)
{
	TRI    *tri;
	size_t size_tri = i_user_interface(current_interface()).size_tri;

	if ((tri = (TRI *)store(size_tri)) == NULL)
	    return NULL;

	Point_of_tri(tri)[0] = p0;
	Point_of_tri(tri)[1] = p1;
	Point_of_tri(tri)[2] = p2;
	p0->hse = p1->hse = p2->hse = Hyper_surf_element(tri);
	Neighbor_on_side01(tri) = neighbor01;
	Neighbor_on_side12(tri) = neighbor12;
	Neighbor_on_side20(tri) = neighbor20;
	Boundary_tri(tri) = bdry;
	tri->prev = NULL;
	tri->next = NULL;
	Tri_index(tri) = 0;
	Tri_workspace(tri) = NULL;
	set_normal_of_tri(tri);
	return tri;
}		/*end i_make_tri*/


LIB_LOCAL void set_tri_array_numbers(
	INTERFACE	*intfc,
	int		index_type)
{
	SURFACE		**s;
	int		i;
	TRI		*tri;

	i = 0;
	for (s = intfc->surfaces; s && *s; ++s)
	{
	    if (index_type == LOCAL_INDICES)
		i = 0;
	    for (tri = first_tri(*s); !at_end_of_tri_list(tri,*s);
							tri = tri->next)
	    	Tri_index(tri) = i++;
	}
}		/*end set_tri_array_numbers*/

EXPORT void null_tri_array_numbers(
	INTERFACE	*intfc)
{
	SURFACE		**s;
	TRI		*tri;

	for (s = intfc->surfaces; s && *s; ++s)
	    for (tri=first_tri(*s); !at_end_of_tri_list(tri,*s); tri=tri->next)
		Tri_index(tri) = 0;
}		/*end null_tri_array_numbers*/
	
EXPORT void print_tri(
	TRI	  *tri,
	INTERFACE *intfc)
{
	fprint_tri(stdout,tri,intfc);
}		/*end print_tri*/

LIB_LOCAL void fprint_tri(
	FILE	  *file,
	TRI	  *tri,
	INTERFACE *intfc)
{
	FT_POINT	           *p[3];
	SURFACE            *s;
	const double* const *sv;
	const double        *tnor;
	int	           i, side;
	static const char  *sideno[] = {"01", "12", "20"};
	static const char  *sep[] = {" ", " ", "\n"};

	if (tri == NULL)
	{
	    (void) fprintf(file,"NULL Triangle\n"); 
	    return;
	}
	(void) fprintf(file,"Tri = %llu\n",tri_number(tri,intfc));
	for (i = 0; i < 3; ++i)
	{
	    p[i] = Point_of_tri(tri)[i];
	    (void) fprintf(file,"p%d: ",i);
	    if (p[i] != NULL)
	    {
	        (void) fprintf(file,"( %"FFMT" %"FFMT" %"FFMT" ) ",
			   Coords(p[i])[0],Coords(p[i])[1],Coords(p[i])[2]);
	        (void) fprintf(file,"%llu Boundary %d Boundary_point %d\n",
			       point_number(p[i]),Boundary(p[i]),
			       Boundary_point(p[i]));
	    }
	    else
	        (void) fprintf(file,"NULL ");
	}

	s = Surface_of_tri(tri);
	if (s == NULL)
	    (void) fprintf(file,"Triangle is not connected to a surface\n");
	else
	    intfc = Surface_of_tri(tri)->interface;
	(void) fprintf(file,"Tri = %llu ",tri_number(tri,intfc));
	for (side = 0; side < 3; ++side)
	{
	    if (is_side_bdry(tri,side))             
	        (void) fprintf(file,"Bond%2s = %llu",sideno[side],
			       bond_number(Bond_on_side(tri,side),intfc));

	    else if (Tri_on_side(tri,side) == NULL) 
	        (void) fprintf(file,"Tri%2s  =       NULL",sideno[side]);

	    else                                        
	        (void) fprintf(file,"Tri%2s  = %llu",sideno[side],
			       tri_number(Tri_on_side(tri,side),intfc));
	    (void) fprintf(file,"%s",sep[side]);
	}

	(void) fprintf(file,"Boundary = %2d ",Boundary_tri(tri));	    
	(void) fprintf(file,"bdry12 = %3s ",
		       is_side01_a_bond(tri)? "YES" : "NO");
	(void) fprintf(file,"bdry23 = %3s ",
		       is_side12_a_bond(tri)? "YES" : "NO");
	(void) fprintf(file,"bdry31 = %3s\n",
		       is_side20_a_bond(tri) ? "YES" : "NO");
	(void) fprintf(file,"prev = %llu next = %llu ",
		       tri_number(tri->prev,intfc),tri_number(tri->next,intfc));
	(void) fprintf(file,"index = %-10d ",Tri_index(tri));
	(void) fprintf(file,"work space = %p\n",(POINTER)Tri_workspace(tri));
	tnor = Tri_normal(tri);
	(void) fprintf(file,"Normal = ( %"FFMT" %"FFMT" %"FFMT" ) "
		      "Surface_of_tri = %llu\n\n",
		      tnor[0],tnor[1],tnor[2],
		      surface_number(Surface_of_tri(tri)));

	sv = side_vector(tri);
	for (i = 0; i < 3; ++i)
	{
	    if (sv[i] != NULL)
	        (void) fprintf(file,"side_vector%d = "
			       "(%"FFMT" %"FFMT" %"FFMT"), len = %"FFMT"\n",i,
			       sv[i][0],sv[i][1],sv[i][2],length_side(tri)[i]);
	    else
	        (void) fprintf(file,"side_vector%d = NULL\n",i);
	}

	if (is_side01_a_bond(tri))
	{
	    (void) fprintf(file,"\n");
	    (void) fprintf(file,"SIDE01 ");
	    fprint_bond(file,Bond_on_side01(tri));
	}
	if (is_side12_a_bond(tri))
	{
	    (void) fprintf(file,"\n");
	    (void) fprintf(file,"SIDE12 ");
	    fprint_bond(file,Bond_on_side12(tri));
	}
	if (is_side20_a_bond(tri))
	{
	    (void) fprintf(file,"\n");
	    (void) fprintf(file,"SIDE20 ");
	    fprint_bond(file,Bond_on_side20(tri));
	}
	(void) fprintf(file,"\n");
}		/*end fprint_tri*/

	

/*
*			fprint_triangle_numbers():
*
*	This routine prints the triangle and the information on its three
*	boundaries. If the boundary is a triangle, its index number is printed.
*	If it is a bond, the bond address is printed. It is assumed that the
*	index numbers are correctly stored in Tri_index(tri).
*/


LOCAL void fprint_triangle_numbers(
	FILE		*file,
	TRI		*tri)
{
	INTERFACE  *intfc = current_interface();
	FT_POINT	   *p;
	int	   i, side;
	static const char *sideno[] = {"01", "12", "20"};

	if (tri == NULL)
	{
	    (void) fprintf(file,"NULL Triangle\n"); 
	    return;
	}
	(void) fprintf(file,"\tTri %d Borders",Tri_index(tri));
	for (side = 0; side < 3; ++side)
	{
	    if (is_side_bdry(tri,side))
	    {
		BOND_TRI *bt = Bond_tri_on_side(tri,side);
		if (bt == NULL)
		    (void) fprintf(file," Bond%s NULL",sideno[side]);
		else
	    	    (void) fprintf(file," Bond%s %llu",sideno[side],
				    bond_number(bt->bond,intfc));
	    }
	    else if (Tri_on_side(tri,side) == NULL)
		(void) fprintf(file," Tri%s NULL",sideno[side]);
	    else
		(void) fprintf(file," Tri%s %d",sideno[side],
			       Tri_index(Tri_on_side(tri,side)));

	}
	(void) fprintf(file," Boundary %d\n",Boundary_tri(tri));

	(void) fprintf(file,"\tPoints - Indices %-4d %-4d %-4d Positions",
		       Index_of_point(Point_of_tri(tri)[0]),
		       Index_of_point(Point_of_tri(tri)[1]),
		       Index_of_point(Point_of_tri(tri)[2]));
	if (is_binary_output() == YES)
	{
	    (void) fprintf(file,"\f%c",9);
	    for (i = 0; i < 3; ++i)
	    {
	    	p = Point_of_tri(tri)[i];
	    	(void) fwrite((const void *) Coords(p),FLOAT,3,file);
	    }
	}
	else
	{
	    static const char *fmt[] = {	" %-"FFMT" %-"FFMT" %-"FFMT" ->",
					        " %-"FFMT" %-"FFMT" %-"FFMT" ->",
					        " %-"FFMT" %-"FFMT" %-"FFMT};
	    for (i = 0; i < 3; ++i)
	    {
	    	p = Point_of_tri(tri)[i];
	    	(void) fprintf(file,fmt[i],
			       Coords(p)[0],Coords(p)[1],Coords(p)[2]);
	    }
	}
	(void) fprintf(file,"\n");
}		/*end fprint_triangle_numbers*/

LIB_LOCAL void print_tris_on_surface(
	SURFACE		*surf)
{
	fprint_tris_on_surface(stdout,surf);
}		/*end print_tris_on_surface*/

LOCAL void fprint_tris_on_surface(
	FILE	*file,
	SURFACE *surf)
{
	INTERFACE *intfc = surf->interface;
	TRI	  *tri;

	(void) fprintf(file,"\n\nPrint the triangles on surface (%llu)\n",
		surface_number(surf));
	(void) fprintf(file,"There are %d triangles on the surface\n\n",
		       surf->num_tri);
	for (tri = first_tri(surf); !at_end_of_tri_list(tri,surf);
							tri = tri->next)
		fprint_tri(file,tri,intfc);
	(void) fprintf(file,"END\n");
}		/*end fprint_tris_on_surface*/

/*
*			fprint_tris_on_curve():
*
*	This routine prints the bond addresses and for each bond,
*	the bounding tris in angle order.  The tris are printed in terms of
*	their index number in the double linked list (ie distance from first)
*	in the surface they belong to.	It is assumed that these index numbers 
*	are already set before this routine is called, by a call to 
*	set_tri_array_numbers for each surface in question.
*/

LIB_LOCAL void fprint_tris_on_curve(
	FILE		*file,
	CURVE		*curve)
{
	BOND		*b;
	BOND_TRI	**btris;
	char		s[20];
	int		i;
 
	(void) fprintf(file,"\n");
	b = curve->first;
	(void) fprintf(file,"\tOrdered tri indices\n");
	b = curve->first;
	(void) fprintf(file,"%-10s ","Bond");
	for (btris = Btris(b), i = 0; btris && *btris; ++btris, ++i)
	{
	    (void) sprintf(s,"btri[%d]",i);
	    (void) fprintf(file,"%-10s",s);
	}
	(void) fprintf(file,"\n");

	for (b = curve->first; b; b = b->next)
	{
	    (void) fprintf(file,"%-10llu ",bond_number(b,curve->interface));
	    for (btris = Btris(b); btris && *btris; ++btris)
	    	(void) fprintf(file,"%-10d ",Tri_index((*btris)->tri));
	    (void) fprintf(file,"\n");
	}
	(void) fprintf(file,"\n");
}		/*end fprint_tris_on_curve*/

/*
*			read_print_tris_on_curve():
*
*	The inverse to print_tris_on_curve().
*/


/* ARGSUSED */
LIB_LOCAL void read_print_tris_on_curve(
	FILE		    *file,
	CURVE		    *curve,
	int		    c_index,
	INTERFACE_ADDRESSES *iaddr)
{
	BOND     *b;
	uint64_t *bonds;
	int	 **tris, i, k;

	if (curve->interface->dim != 3)
	    return;

	(void) fgetstring(file,"Ordered tri indices");
	(void) getc(file);			/* Grab trailing \n */
	while (getc(file) != '\n'); /* next line */
	uni_array(iaddr->tris+c_index,iaddr->num_bonds[c_index],sizeof(int*));
	uni_array(iaddr->bonds+c_index,iaddr->num_bonds[c_index],sizeof(uint64_t));
	tris = iaddr->tris[c_index];
	bonds = iaddr->bonds[c_index];
	for (b = curve->first, i = 0; b != NULL; b = b->next, ++i)
	{
	    (void) fscanf(file,"%llu",bonds+i);
	    uni_array(tris+i,iaddr->num_surfs[c_index],INT);
	    for (k = 0; k < iaddr->num_surfs[c_index]; ++k)
	    	(void) fscanf(file,"%d",tris[i]+k);
	}
}		/*end read_print_tris_on_curve*/

/*
*		curve_is_in_surface_bdry():
*
*	Determines whether a curve is installed in a surface boundary.
*	If YES then the orientation of the first entry in the surface
*	boundary structure is returned.
*/

EXPORT	boolean curve_is_in_surface_bdry(
	SURFACE     *s,
	CURVE       *c,
	ORIENTATION *orient)
{
	boolean pc, ps, nc, ns;
	pc = pointer_is_in_array(c,s->pos_curves);
	ps = pointer_is_in_array(s,c->pos_surfaces);
	if (pc && ps)
	{
	    *orient = POSITIVE_ORIENTATION;
	    return YES;
	}
	nc = pointer_is_in_array(c,s->neg_curves);
	ns = pointer_is_in_array(s,c->neg_surfaces);
	if (nc && ns)
	{
	    *orient = NEGATIVE_ORIENTATION;
	    return YES;
	}
	*orient = ORIENTATION_NOT_SET;
	if ((pc && !ps) || (!ps && ps) || (nc && !ns) || (!nc && ns))
	{
	    screen("ERROR in curve_is_in_surface_bdry(), inconsistent "
		   "curve and surface pointers\n");
	    if (pc)
	        (void) printf("c is a pos curve of s\n");
	    if (ps)
	        (void) printf("s is a pos surface of c\n");
	    if (nc)
	        (void) printf("c is a neg curve of s\n");
	    if (ns)
	        (void) printf("s is a neg surface of c\n");
	    clean_up(ERROR);
	}
	return NO;
}		/*end curve_is_in_surface_bdry*/

/*
*		install_curve_in_surface_bdry():
*	
*	Assigns the boundary pointers correctly.
*/

EXPORT void install_curve_in_surface_bdry(
	SURFACE		*s,
	CURVE		*c,
	ORIENTATION	orient)
{

	if (!s || !c)
	    return;

	switch (orient)
	{
	case POSITIVE_ORIENTATION:
	    if (!unique_add_to_pointers(s,&c->pos_surfaces) ||
		!unique_add_to_pointers(c,&s->pos_curves))
	    {
	    	screen("ERROR in install_curve_in_surface_bdry(), "
	    	       "add_to_pointers() failed for POSITIVE_ORIENTATION\n");
	    	clean_up(ERROR);
	    }
	    break;
	case NEGATIVE_ORIENTATION:
	    if (!unique_add_to_pointers(s,&c->neg_surfaces) ||
		!unique_add_to_pointers(c,&s->neg_curves))
	    {
	    	screen("ERROR in install_curve_in_surface_bdry(), "
	    	       "add_to_pointers() failed for NEGATIVE_ORIENTATION\n");
	    	clean_up(ERROR);
	    }
	    break;
	default:
	    screen("ERROR in install_curve_in_surface_bdry(), "
	    	   "invalid orientation\n");
	    clean_up(ERROR);
	    break;
	}
}		/*end install_curve_in_surface_bdry*/

/*
*		remove_curve_from_surface_bdry():
*	
*	Deletes the boundary pointers correctly.
*/

EXPORT boolean remove_curve_from_surface_bdry(
	SURFACE		*s,
	CURVE		*c,
	ORIENTATION	orient)
{

	if (!s || !c)
	    return NO;

	switch (orient)
	{
	case POSITIVE_ORIENTATION:
	    if (!delete_from_pointers(s,&c->pos_surfaces) ||
		!delete_from_pointers(c,&s->pos_curves))
		return NO;
	    break;
	case NEGATIVE_ORIENTATION:
	    if (!delete_from_pointers(s,&c->neg_surfaces) ||
		!delete_from_pointers(c,&s->neg_curves))
		return NO;
	    break;
	default:
	    screen("ERROR in remove_curve_from_surface_bdry(), "
	    	   "invalid orientation\n");
	    clean_up(ERROR);
	    break;
	}
	return YES;
}		/*end remove_curve_from_surface_bdry*/



EXPORT boolean next_tri(
	INTERFACE	*intfc,
	TRI		**t,
	SURFACE		**s)
{
	struct Table	*T;

	if (((T=table_of_interface(intfc))==NULL) || (intfc->surfaces==NULL))
	    return NO;

			/* Reinitialize to start: */
	if (t == NULL)
	{
	    T->cur_surface = intfc->surfaces-1;
	    T->cur_tri = NULL;
	    return YES;
	}

			/* Last Tri was at end of Surface: */
	if (T->cur_tri==NULL ||
			at_end_of_tri_list(T->cur_tri->next,*T->cur_surface))
	{
			/* No more Surfaces */

	    if (*++(T->cur_surface) == NULL)
	    {
	    	*t = NULL;	*s = NULL;
	    	return NO;			
	    }
	    else
	        T->cur_tri = first_tri(*T->cur_surface);
	}
	else
	    T->cur_tri = T->cur_tri->next;
	if (at_end_of_tri_list(T->cur_tri,*T->cur_surface))
	    T->cur_tri = NULL;
	*s = *(T->cur_surface);
	*t = T->cur_tri;
	if (*t == NULL)
	{
	    screen("ERROR in next_tri(), no triangles on surface\n");
	    clean_up(ERROR);
	    return NO;
	}

	return YES;
}		/*end next_tri*/

/*NOTE: if a point is on the intersection curve of n surfaces, it will be visited  */
/*n times. */
LIB_LOCAL boolean next_point3d(
	INTERFACE	   *intfc,
	FT_POINT		   **P,
	HYPER_SURF_ELEMENT **HSE,
	HYPER_SURF	   **HS)
{
	struct Table	*T;
	int		flag;

	if ((T = table_of_interface(intfc)) == NULL)
	    return NO;

					/* Reinitialize to start: */
	if (P == NULL)  /*initialize all variables */
	{
	    reset_sort_status(intfc);
	    T->cur_surface = (intfc->surfaces!=NULL) ? intfc->surfaces-1 : NULL;
	    T->cur_tri = NULL;
	    T->cur_curve = NULL;
	    T->cur_bond = NULL;
	    T->np_do_p_curs = NO;
	    T->np_do_n_curs = NO;
	    return YES;
	}

	if ((T->cur_surface != (intfc->surfaces-1)) &&
	    ((T->cur_surface==NULL) || (*(T->cur_surface) == NULL)))
	{
	    *P = NULL;
	    *HSE = NULL;
	    *HS = NULL;
	    return NO;
	}

	if (T->cur_tri==NULL && T->np_do_p_curs==NO && T->np_do_n_curs==NO)
	{
	    if (*++(T->cur_surface) == NULL)  
		/*already go over all surfaces, return */
	    {
	    	*P = NULL;
	    	*HSE = NULL;
	    	*HS = NULL;
	    	return NO;
	    }
	    if (no_tris_on_surface(*T->cur_surface))
	    {
	    	T->cur_tri = NULL;
	    	*P = NULL;
	    	*HSE = NULL;
	    	*HS = NULL;
	    	screen("ERROR in next_point3d(), no triangles on surface\n");
		clean_up(ERROR);
		return NO;
	    }
	    if ((*T->cur_surface)->pos_curves != NULL)  
		/*begin to loop over pos_curves */
	    {
	    	T->np_do_p_curs = YES;
	    	T->cur_curve = (*T->cur_surface)->pos_curves - 1;
	    }
	    else if ((*T->cur_surface)->neg_curves != NULL)
	    {  /*if no positive curve, begin to loop over neg_curves */
	    	T->np_do_p_curs = NO;
	    	T->np_do_n_curs = YES;
	    	T->cur_curve = (*T->cur_surface)->neg_curves - 1;
	    }
	    else
	    {   /*no curves on surfaces */
	    	T->np_do_p_curs = NO;
	    	T->np_do_n_curs = NO;
	    }
	    T->cur_bond = NULL;
	}

	/*loop over all points on pos_curves */
	if (T->np_do_p_curs == YES)
	{
	    if (T->cur_bond==NULL)
	    {
	    	if (*++(T->cur_curve) == NULL)
	    	{
	    	    T->np_do_p_curs = NO;
	    	    if ((*T->cur_surface)->neg_curves != NULL)
	    	    {
	    	    	T->np_do_n_curs = YES;
	    	    	T->cur_curve = (*T->cur_surface)->neg_curves - 1;
	    	    }
	    	}
	    	else
		    T->cur_bond = (*T->cur_curve)->first;
	    }
	    else
	    	T->cur_bond = T->cur_bond->next;

	    if (T->np_do_p_curs == YES)
	    {
	        *HS = Hyper_surf(*T->cur_surface);
	        if (T->cur_bond==NULL) /* P at End of Curve */
	        {
	    	    *P = (*T->cur_curve)->last->end;
	    	    *HSE = Hyper_surf_element(
			tri_at_bond((*(T->cur_curve))->last,*T->cur_surface));
	    	    sorted(*P) = YES;
	        }
	        else
		{
		    *P = T->cur_bond->start;
		    *HSE = Hyper_surf_element(
			tri_at_bond(T->cur_bond,*T->cur_surface));
		    sorted(*P) = YES;
	        }
		return YES;
	    }
	}

	/*loop over all points on neg_curves */
	if (T->np_do_n_curs == YES)
	{
	    if (T->cur_bond==NULL)
	    {
	    	if (*++(T->cur_curve) == NULL)  
	    	{
		    /*After here, np_do_n_curs==NO and np_do_p_curs==NO */
		    /*will begin to loop over tris */
	    	    T->np_do_n_curs = NO;
	    	    T->cur_curve = NULL;
	    	}
	    	else
	    	    T->cur_bond = (*T->cur_curve)->first;
	    }
	    else
	    	T->cur_bond = T->cur_bond->next;

	    if (T->np_do_n_curs == YES)
	    {
	        *HS = Hyper_surf(*T->cur_surface);
	        if (T->cur_bond==NULL) /* P at End of Curve */
	        {
	    	    *P = (*(T->cur_curve))->last->end;
	    	    *HSE = Hyper_surf_element(
			tri_at_bond((*T->cur_curve)->last,*T->cur_surface));
	    	    sorted(*P) = YES;
	        }
	        else
	        {
	    	    *P = T->cur_bond->start;
	    	    *HSE = Hyper_surf_element(
			tri_at_bond(T->cur_bond,*T->cur_surface));
	    	    sorted(*P) = YES;
	        }
	        return YES;
	    }
	}

	/*loop over points on tris */
	if (T->cur_tri == NULL)
	    T->cur_tri = first_tri(*T->cur_surface);

	do
	{
	    flag = 0;
	    if (sorted(Point_of_tri(T->cur_tri)[0]) == YES)
		flag += 1;
	    if (sorted(Point_of_tri(T->cur_tri)[1]) == YES)
		flag += 2;
	    if (sorted(Point_of_tri(T->cur_tri)[2]) == YES)
		flag += 4;
	    switch (flag)
	    {
	    case 0:
	    case 2:
	    case 4:
	    case 6:
	    	*P = Point_of_tri(T->cur_tri)[0];
	    	break;
	    case 1:
	    case 5:
	    	*P = Point_of_tri(T->cur_tri)[1];
	    	break;
	    case 3:
	    	*P = Point_of_tri(T->cur_tri)[2];
	    	break;
	    case 7:
	    	if (T->cur_tri == last_tri(*T->cur_surface))
	    	{   /*go to the next surface */
	    	    T->cur_tri = NULL;
	    	    return next_point3d(intfc,P,HSE,HS);
	    	}
		T->cur_tri = T->cur_tri->next;
		break;
	    }
	}
	while (flag == 7);

	*HSE = Hyper_surf_element(T->cur_tri);
	*HS = Hyper_surf(*(T->cur_surface));

	sorted(*P) = YES;
	return YES;
}		/*end next_point3d*/

LIB_LOCAL	boolean	next_hypersurface3d(
	INTERFACE	*intfc,
	HYPER_SURF	**HS)
{
	struct Table *T;

	if (HS == NULL)
	    return next_point(intfc,NULL,NULL,NULL);

	if ((T = table_of_interface(intfc)) == NULL)
	    return NO;

	T->np_do_p_curs = NO;
	T->np_do_n_curs = NO;
	T->cur_tri = NULL;

	if (*++(T->cur_surface) == NULL)
	{
	    *HS = NULL;
	    return NO;
	}
	*HS = Hyper_surf(*(T->cur_surface));
	if (no_tris_on_surface(*T->cur_surface))
	{
	    T->cur_tri = NULL;
	    *HS = NULL;
	    screen("ERROR in next_hypersurface3d(), "
	           "no triangles on surface\n");
	    clean_up(ERROR);
	    return NO;
	}

	if ((*T->cur_surface)->pos_curves != NULL)
	{
	    T->np_do_p_curs = YES;
	    T->cur_curve = (*T->cur_surface)->pos_curves - 1;
	    T->cur_bond = NULL;
	}
	else if ((*T->cur_surface)->neg_curves != NULL)
	{
	    T->np_do_p_curs = NO;
	    T->np_do_n_curs = YES;
	    T->cur_curve = (*T->cur_surface)->neg_curves - 1;
	    T->cur_bond = NULL;
	}
	else
	    T->cur_tri = first_tri(*T->cur_surface);

	return YES;
}		/*end next_hypersurface3d*/

LOCAL	TRI*	tri_at_bond(
	BOND		*b,
	SURFACE		*s)
{
	BOND_TRI	**btris;

	for (btris = Btris(b); btris && *btris; ++btris)
	    if (Surface_of_tri((*btris)->tri) == s)
	    {
	    	return (*btris)->tri;
	    }
	return NULL;
}		/*end tri_at_bond*/

EXPORT	void link_tri_list_to_surface(
	TRI	*first,
	TRI	*last,
	SURFACE *s)
{
	TRI	*t;
	int	n;

	first_tri(s) = first;
	first_tri(s)->prev = head_of_tri_list(s);
	last_tri(s)  = last;
	last_tri(s)->next = tail_of_tri_list(s);
	for (n=0, t=first_tri(s); !at_end_of_tri_list(t,s); ++n, t=t->next)
	    t->surf = s;
	s->num_tri = n;
}		/*end link_tri_list_to_surface*/

EXPORT	TRI	*Prev_tri_at_vertex(
	TRI	*tri,
	FT_POINT	*p)
{
	int	side;

	if (tri == NULL)
	    return NULL;
	side = Prev_side_at_vertex(tri,p);
	if (side == ERROR || is_side_bdry(tri,side))
	    return NULL;

	return Tri_on_side(tri,side);
}		/*end Prev_tri_at_vertex*/

EXPORT	TRI	*Next_tri_at_vertex(
	TRI	*tri,
	FT_POINT	*p)
{
	int	side;

	if (tri == NULL)
	    return NULL;
	side = Next_side_at_vertex(tri,p);
	if (side == ERROR || is_side_bdry(tri,side))
	    return NULL;

	return Tri_on_side(tri,side);
}		/*end Next_tri_at_vertex*/

/*
*			reset_tri_points_at_bond():
*
*	Resets the point pointers on a tri to agree with an adjacent bond.
*/

LOCAL	void reset_tri_points_at_bond(
	TRI  *tri,
	BOND *b)
{
	TRI	    *t;
	FT_POINT	    **p, *ps, *pe, *psold, *peold;
	int	    i, j, is, ie, side;
	double	    *cs, *ce, *c[3], ds, min_ds;
	ORIENTATION orient;

	ps = b->start; cs = Coords(ps);
	pe = b->end;   ce = Coords(pe);
	p = Point_of_tri(tri);
	c[0] = Coords(p[0]); c[1] = Coords(p[1]); c[2] = Coords(p[2]);
	orient = ORIENTATION_NOT_SET;
	min_ds = HUGE_VAL;
	for (i = 0; i < 3; ++i)
	{
	    j = Next_m3(i);
	    ds = sqr(c[j][0]-ce[0])+sqr(c[j][1]-ce[1])+sqr(c[j][2]-ce[2])+
	         sqr(c[i][0]-cs[0])+sqr(c[i][1]-cs[1])+sqr(c[i][2]-cs[2]);
	    if (ds < min_ds)
	    {
		min_ds = ds;
	        orient = POSITIVE_ORIENTATION;
		is = i;
		ie = j;
	    }
	    ds = sqr(c[j][0]-cs[0])+sqr(c[j][1]-cs[1])+sqr(c[j][2]-cs[2])+
	         sqr(c[i][0]-ce[0])+sqr(c[i][1]-ce[1])+sqr(c[i][2]-ce[2]);
	    if (ds < min_ds)
	    {
		min_ds = ds;
	        orient = NEGATIVE_ORIENTATION;
		is = j;
		ie = i;
	    }
	}
	psold = p[is];
	peold = p[ie];

	/* reset the pointers of vertices of neighbor tris*/

	t = tri;
	while (t != NULL)
	{
	    Point_of_tri(t)[is] = ps;
	    side = (orient==POSITIVE_ORIENTATION) ? Prev_m3(is) : is;
	    if (!is_side_bdry(t,side))
	    {
		t = Tri_on_side(t,side);
		if (t != NULL)
		{
		    for (is = 0; is < 3; ++is)
		    	if (Point_of_tri(t)[is] == psold)
			    break;
		    if (is == 3)
		    {
			screen("ERROR in reset_tri_points_at_bond(), "
			       "point not on adjacent triangle\n");
			clean_up(ERROR);
		    }
		}
	    }
	    else
	        t = NULL;
	}
	t = tri;
	while (t != NULL)
	{
	    Point_of_tri(t)[ie] = pe;
	    side = (orient==POSITIVE_ORIENTATION) ? ie : Prev_m3(ie);
	    if (!is_side_bdry(t,side))
	    {
		t = Tri_on_side(t,side);
		if (t != NULL)
		{
		    for (ie = 0; ie < 3; ++ie)
		    	if (Point_of_tri(t)[ie] == peold)
			    break;
		    if (ie == 3)
		    {
			screen("ERROR in reset_tri_points_at_bond(), "
			       "point not on adjacent triangle\n");
			clean_up(ERROR);
		    }
		}
	    }
	    else
		t = NULL;
	}
}		/*end reset_tri_points_at_bond*/

/*
*			i_link_tri_to_bond():
*
*	Sets the neighbor points to connect a triangle to a bond.
*/

EXPORT	BOND_TRI *i_link_tri_to_bond(
	BOND_TRI *btri,
	TRI	 *tri,
	SURFACE  *s,
	BOND	 *b,
	CURVE    *c)
{
	ORIENTATION orient = ORIENTATION_NOT_SET;
	INTERFACE   *cur_intfc = current_interface();
	size_t	    size_bond_tri;
	FT_POINT	    *ps, *pe, **p;
	int	    i, side;

	if (btri == NULL)
	{
	    size_bond_tri = i_user_interface(cur_intfc).size_bond_tri;
	    if ((btri = (BOND_TRI *)store(size_bond_tri)) == NULL)
	        return NULL;
	    if (!add_to_pointers(btri,&Btris(b)))
	    {
	        screen("ERROR in i_link_tri_to_bond(), "
		       "add_to_pointers() failed\n");
	        clean_up(ERROR);
	    }
	}
	else
	{
	    BOND_TRI **bt;
	    for (bt = Btris(b); bt && *bt; ++bt)
		if (*bt == btri)
		    break;
	    if ((bt == NULL) || (*bt != btri))
	    {
	        screen("ERROR in i_link_tri_to_bond(), "
		       "btri not in bond tri list of b\n");
		clean_up(ERROR);
	    }
	}
	btri->tri = tri;
	btri->surface = s;
	btri->bond = b;
	btri->curve = c;
	ps = b->start;
	pe = b->end;
	p = Point_of_tri(tri);
	for (i = 0; i < 3; i++)
	{
	    if (p[i] == ps && p[Next_m3(i)] == pe)
	    {
	    	orient = POSITIVE_ORIENTATION;
	    	side = i;
	    }
	    else if (p[i] == ps && p[Prev_m3(i)] == pe)
	    {
	    	orient = NEGATIVE_ORIENTATION;
	    	side = Prev_m3(i);
	    }
	}
	if (orient == ORIENTATION_NOT_SET)
	{
	    FT_POINT  **p;

	    screen("ERROR in i_link_tri_to_bond(), no side "
		   "of tri shares common points with bond\n");
	    /*print_tri(tri,s->interface);  can not print_tri here */
	    p = Point_of_tri(tri);
	    
	    print_tri_coords(tri);
	    printf("%llu\n", point_number(p[0]));
	    printf("%llu\n", point_number(p[1]));
	    printf("%llu\n", point_number(p[2]));
	    
	    print_bond(b);
	    printf("tri bond %p %p\n", tri, b);

	    if (debugging("link_tri_to_bond"))
	    {
	        FILE *xgraph = xgraph_file_open("xg","link_tri_bond",XY_PLANE);
		xgraph_tri(xgraph,tri,XY_PLANE);
		xgraph_new_data_set(xgraph);
		xgraph_line_segment(xgraph,Coords(b->start),Coords(b->end),
				    XY_PLANE,"bond");
		fclose(xgraph);
	    }
	    clean_up(ERROR);
	}
	btri->orient = orient;
	install_curve_in_surface_bdry(s,c,orient);
	Bond_tri_on_side(tri,side) = btri;
	set_side_bdry(Boundary_tri(tri),side,1);
	Boundary_point(b->start) = Boundary_point(b->end) = 1;

	return btri;
}		/*end i_link_tri_to_bond*/

EXPORT	void i_reverse_bond(
	BOND *b)
{
	FT_POINT *ptmp;
	BOND_TRI **btris;

	ptmp = b->start;
	b->start = b->end;
	b->end = ptmp;

	for (btris = Btris(b); btris && *btris; btris++)
	    (*btris)->orient = Opposite_orient((*btris)->orient);
}	/* end i_reverse_bond */

EXPORT	boolean i_sort_bond_tris(
	INTERFACE *intfc)
{
	BOND     *b, *b0;
	BOND_TRI **bts, **bts0;
	CURVE    **c;
	boolean  status = YES;
	int      i, j, nbts, nbts0;

	for (c = intfc->curves; c && *c; ++c)
	{
	    b0 = (*c)->first;
	    if (b0->next == NULL)
		continue;
	    for (nbts0 = 0, bts = Btris(b0); bts && *bts; ++nbts0, ++bts);
	    for (i = 0, bts0 = Btris(b0); bts0 && *bts0; ++i, ++bts0)
	    {
	        for (b = b0->next; b != NULL; b = b->next)
		{
	            for (nbts = 0, bts = Btris(b); bts && *bts; ++nbts, ++bts);
	            if (nbts != nbts0)
		    {
			(void) printf("WARNING in i_sort_bond_tris(), "
				      "inconsistent numbers of bond "
				      "tris\n");
			status = NO;
		    }
		    for (j=0,bts=Btris(b);bts && *bts && (j<nbts0);++j,++bts)
		    {
			if (((*bts)->surface == (*bts0)->surface) &&
			    ((*bts)->orient == (*bts0)->orient))
			{
			    if (j != i)
			    {
			        BOND_TRI *bt = Btris(b)[i];
			        Btris(b)[i] = Btris(b)[j];
			        Btris(b)[j] = bt;
			    }
			    break;
			}
		    }
		    if (j == nbts0)
		    {
			(void) printf("WARNING in i_sort_bond_tris(), "
				      "bond tri not found\n");
			status = NO;
		    }
		}
	    }
	}
	return status;
}		/*end i_sort_bond_tris*/


EXPORT	SURFACE  *i_join_surfaces(
	CURVE *c)
{
	BOND     *b;
	NODE     *ns, *ne;
	FT_POINT    *ps, *pe;
	SURFACE  *sp, *sn;
	SURFACE  *news;
	TRI      *nt, *t, *ptri, *ntri;
	int      pside, nside;

	debug_print("join_surfaces","Entered i_join_surfaces()\n");
	if (debugging("join_surfaces"))
	{
	    (void) printf("joining surfaces at curve %llu\n",curve_number(c));
	    print_curve(c);
	}

	if (!find_surfaces_to_join_at_curve(c,&sn,&sp))
	{
	    debug_print("join_surfaces","Left i_join_surfaces()\n");
	    return NULL;
	}
	if (debugging("join_surfaces"))
	{
	    (void) printf("joining surfaces sp = %llu sn = %llu\n",
			  surface_number(sp),surface_number(sn));
	}
	if (sp == sn)
	    news = sp;
	else
	{
	    CURVE **cc, **neg, **pos;
	    if (positive_component(sp) != positive_component(sn))
	    {
	        (void) printf("WARNING in i_join_surfaces(), "
			      "unequal positive components\n");
	        debug_print("join_surfaces","Left i_join_surfaces()\n");
		return NULL;
	    }
	    if (negative_component(sp) != negative_component(sn))
	    {
	        (void) printf("WARNING in i_join_surfaces(), "
			      "unequal negative components\n");
	        debug_print("join_surfaces","Left i_join_surfaces()\n");
		return NULL;
	    }
	    neg = NULL;
	    for (cc = sp->neg_curves; cc && *cc; ++cc)
	    {
		if ((*cc != c) && (!add_to_pointers(*cc,&neg)))
	        {
	            screen("ERROR in i_join_surfaces(), "
		           "add_to_pointers() failed\n");
	            clean_up(ERROR);
	        }
	    }
	    for (cc = sn->neg_curves; cc && *cc; ++cc)
	    {
		if ((*cc != c) && (!add_to_pointers(*cc,&neg)))
	        {
	            screen("ERROR in i_join_surfaces(), "
		           "add_to_pointers() failed\n");
	            clean_up(ERROR);
	        }
	    }
	    for (cc = sp->pos_curves; cc && *cc; ++cc)
	    {
		if ((*cc != c) && (!add_to_pointers(*cc,&pos)))
	        {
	            screen("ERROR in i_join_surfaces(), "
		           "add_to_pointers() failed\n");
	            clean_up(ERROR);
	        }
	    }
	    for (cc = sn->pos_curves; cc && *cc; ++cc)
	    {
		if ((*cc != c) && (!add_to_pointers(*cc,&pos)))
	        {
	            screen("ERROR in i_join_surfaces(), "
		           "add_to_pointers() failed\n");
	            clean_up(ERROR);
	        }
	    }
	    news = copy_surface(sn,neg,pos,NO);
	    for (t = first_tri(sn); !at_end_of_tri_list(t,sn); t = nt)
	    {
		nt = t->next;
		remove_tri_from_surface(t,sn,YES);
		insert_tri_at_tail_of_list(t,news);
	    }
	    for (t = first_tri(sp); !at_end_of_tri_list(t,sp); t = nt)
	    {
		nt = t->next;
		remove_tri_from_surface(t,sp,YES);
		insert_tri_at_tail_of_list(t,news);
	    }
	}
	for (b = c->first; b != NULL; b = b->next)
	{
	    if (!tris_on_side_of_bond_for_join(b,&ntri,&nside,&ptri,&pside))
	    {
	        (void) printf("WARNING in i_join_surfaces(), "
			      "can't find tris to join\n");
	        debug_print("join_surfaces","Left i_join_surfaces()\n");
	        return NULL;
	    }
	    Tri_on_side(ptri,pside) = ntri;
	    set_side_bdry(Boundary_tri(ptri),pside,0);
	    Tri_on_side(ntri,nside) = ptri;
	    set_side_bdry(Boundary_tri(ntri),nside,0);
	    if (b->next)
	        Boundary_point(b->end) = 0;
	}
	ns = c->start;
	ps = ns->posn;
	ne = c->end;
	pe = ne->posn;
	if (!delete_curve(c))
	{
	    (void) printf("WARNING in i_join_surfaces(), "
			  "can't delete curve\n");
	    debug_print("join_surfaces","Left i_join_surfaces()\n");
	    return NULL;
	}
	if (delete_node(ns))
	    Boundary_point(ps) = 0;
	if ((ne != ns) && delete_node(ne))
	    Boundary_point(pe) = 0;
	if ((news != sp) && (!delete_surface(sp)))
	{
	    (void) printf("WARNING in i_join_surfaces(), "
			  "can't delete surface sp\n");
	    debug_print("join_surfaces","Left i_join_surfaces()\n");
	    return NULL;
	}
	if ((news != sn) && (!delete_surface(sn)))
	{
	    (void) printf("WARNING in i_join_surfaces(), "
			  "can't delete surface sn\n");
	    debug_print("join_surfaces","Left i_join_surfaces()\n");
	    return NULL;
	}
	debug_print("join_surfaces","Left i_join_surfaces()\n");
	return news;
}		/*end i_join_surfaces*/

EXPORT	boolean find_surfaces_to_join_at_curve(
	CURVE   *c,
	SURFACE **psn,
	SURFACE **psp)
{
	SURFACE *sp, *sn;

	debug_print("join_surfaces","Entered find_surfaces_to_join_at_curve()\n");
	if ((c->pos_surfaces == NULL) ||
	    (c->pos_surfaces[0] == NULL) || (c->pos_surfaces[1] != NULL) ||
	    (c->neg_surfaces == NULL) ||
	    (c->neg_surfaces[0] == NULL) || (c->neg_surfaces[1] != NULL))
	{
	    if (c->pos_surfaces == NULL)
	        (void) printf("WARNING in find_surfaces_to_join_at_curve(), "
			      "c->pos_surfaces == NULL\n");
	    else if (c->pos_surfaces[0] == NULL)
	        (void) printf("WARNING in find_surfaces_to_join_at_curve(), "
			      "c->pos_surfaces[0] == NULL\n");
	    else if (c->pos_surfaces[1] != NULL)
	        (void) printf("WARNING in find_surfaces_to_join_at_curve(), "
			      "more than one positive surface\n");
	    if (c->neg_surfaces == NULL)
	        (void) printf("WARNING in find_surfaces_to_join_at_curve(), "
			      "c->neg_surfaces == NULL\n");
	    else if (c->neg_surfaces[0] == NULL)
	        (void) printf("WARNING in find_surfaces_to_join_at_curve(), "
			      "c->neg_surfaces[0] == NULL\n");
	    else if (c->neg_surfaces[1] != NULL)
	        (void) printf("WARNING in find_surfaces_to_join_at_curve(), "
			      "more than one negative surface\n");
	    debug_print("join_surfaces","Left find_surfaces_to_join_at_curve()\n");
	    return NO;
	}
	sp = c->pos_surfaces[0];
	if (psp)
	    *psp = sp;
	sn = c->neg_surfaces[0];
	if (psn)
	    *psn = sn;
	debug_print("join_surfaces","Left find_surfaces_to_join_at_curve()\n");
	return YES;
}		/*end find_surfaces_to_join_at_curve*/

EXPORT	boolean tris_on_side_of_bond_for_join(
	BOND *b,
	TRI  **ntri,
	int  *nside,
	TRI  **ptri,
	int  *pside)
{
	BOND_TRI    **bts;
	ORIENTATION orient[2];
	boolean        status;

	bts = Btris(b);
	orient[0] = bts[0]->orient;
	orient[1] = bts[1]->orient;
	if ((orient[0]==POSITIVE_ORIENTATION) &&
	    (orient[1]==NEGATIVE_ORIENTATION))
	{
	    *ptri = bts[0]->tri;
	    *pside = side_of_tri_with_bond(b,*ptri);
	    *ntri = bts[1]->tri;
	    *nside = side_of_tri_with_bond(b,*ntri);
	    status = YES;
	}
	else if ((orient[1]==POSITIVE_ORIENTATION) &&
	         (orient[0]==NEGATIVE_ORIENTATION))
	{
	    *ptri = bts[1]->tri;
	    *pside = side_of_tri_with_bond(b,*ptri);
	    *ntri = bts[0]->tri;
	    *nside = side_of_tri_with_bond(b,*ntri);
	    status = YES;
	}
	else
	{
	    *ptri = *ntri = NULL;
	    *pside = *nside = -1;
	    status = NO;
	}
	return status;
}		/*end tris_on_side_of_bond_for_join*/

LOCAL void reset_sort_status(
	INTERFACE	*intfc)
{
	SURFACE		**s;
	TRI		*t;

	for (s = intfc->surfaces; s && *s; ++s)
	{
	    for (t = first_tri(*s); !at_end_of_tri_list(t,*s); t = t->next)
	    {
		sorted(Point_of_tri(t)[0]) = NO;
		sorted(Point_of_tri(t)[1]) = NO;
		sorted(Point_of_tri(t)[2]) = NO;
	    }
	}
}		/*end reset_sort_status*/

LOCAL void add_bdry_curve_to_hash_table(
	SURFACE		*s1,
	SURFACE		*s2,
	P_LINK		*hash_table,
	int		h_size)
{
	CURVE		**c1,**c2;
	BOND		*b1,*b2;
	FT_POINT		*p;

	for (c1=s1->pos_curves, c2=s2->pos_curves; c2 && *c2; ++c1, ++c2)
	{
	    b1 = (*c1)->first;	b2 = (*c2)->first;
	    
	    
	    p = (FT_POINT*)find_from_hash_table((POINTER)b1->start,
				             hash_table,h_size);
	    if (p == NULL)
	    	(void) add_to_hash_table((POINTER)b1->start,(POINTER)b2->start,
				         hash_table,h_size);
	    else
		b2->start = p;
		
	    for (; b1 && b2; b1=b1->next, b2=b2->next)
	    {
	    	p = (FT_POINT*)find_from_hash_table((POINTER)b1->end,
				                 hash_table,h_size);
	    	if (p != NULL)
	    	{
	    	    b2->end = p;
	    	    if (b2->next)
	    	    	b2->next->start = p;
	    	}
	    	else
	    	    (void) add_to_hash_table((POINTER)b1->end,(POINTER)b2->end,
				             hash_table,h_size);
	    }
	}
	for (c1=s1->neg_curves, c2=s2->neg_curves; c2 && *c2; ++c1, ++c2)
	{
	    b1 = (*c1)->first;	b2 = (*c2)->first;
	    
	    p = (FT_POINT*)find_from_hash_table((POINTER)b1->start,
	    		                     hash_table,h_size);
	    if (p == NULL)
	    	(void) add_to_hash_table((POINTER)b1->start,(POINTER)b2->start,
	    		                 hash_table,h_size);
	    else
		b2->start = p;
	    for (; b1 && b2; b1=b1->next, b2=b2->next)
	    {
	    	

	    	p = (FT_POINT*)find_from_hash_table((POINTER)b1->end,
				                 hash_table,h_size);
	    	if (p != NULL)
	    	{
	    	    b2->end = p;
	    	    if (b2->next)
	    	    	b2->next->start = p;
	    	}
	    	else
	    	    (void) add_to_hash_table((POINTER)b1->end,(POINTER)b2->end,
				             hash_table,h_size);
	    }
	}
}		/*end add_bdry_curve_to_hash_table*/

/*
*	IMPORTANT NOTE ON PARALLELISM
*
*	The function new_pp_index() requires a synchronization of all
*	processors.  This step in needed to ensure the global uniqueness
*	and compatibility of the component and pp_index values.
*
*	THIS IS NOT CURRENTLY IMPLEMENTED AND IS A MAJOR REQUIREMENT
*	BEFORE ANY DYNAMIC MODIFICIATION OF A PARALLEL INTERFACE IS
*	POSSIBLE.
*/

LOCAL COMPONENT new_pp_index(
	COMPONENT indx,
	INTERFACE *intfc)
{

	if (indx != NO_COMP)
	    max_pp_index(intfc) = max(indx,max_pp_index(intfc));
	else
	    indx = ++max_pp_index(intfc);
	return indx;
}		/*end new_pp_index*/

EXPORT	void rotate_triangle(
	TRI *tri,
	int id)
{
	FT_POINT        *ptmp[3];
	TRI_NEIGHBOR nbtmp[3];
	int          i,bdry[3];

	if (id == 0)
	    return;
	for (i = 0; i < 3; i++)
	{
	    bdry[i] = is_side_bdry(tri,i) ? 1 : 0;
	    ptmp[i] = Point_of_tri(tri)[i];
	    nbtmp[i] = Tri_neighbor(tri)[i];
	}
	for (i = 0; i < 3; i++)
	    set_side_bdry(Boundary_tri(tri),i,bdry[(i+id)%3]);
	Point_of_tri(tri)[0] = ptmp[id];
	Point_of_tri(tri)[1] = ptmp[(id+1)%3];
	Point_of_tri(tri)[2] = ptmp[(id+2)%3];
	Tri_neighbor(tri)[0] = nbtmp[id];
	Tri_neighbor(tri)[1] = nbtmp[(id+1)%3];
	Tri_neighbor(tri)[2] = nbtmp[(id+2)%3];
}		/*end rotate_triangle*/

EXPORT boolean link_neighbor_tris(
	TRI *tri1,
	TRI *tri2)
{
	int i,j;

	for (i = 0; i < 3; ++i)
	{
	    for (j = 0; j < 3; ++j)
	    {
	    	if (Point_of_tri(tri1)[i] == Point_of_tri(tri2)[Next_m3(j)] &&
		    Point_of_tri(tri1)[Next_m3(i)] == Point_of_tri(tri2)[j])
		{
		    Tri_on_side(tri1,i) = tri2;
		    Tri_on_side(tri2,j) = tri1;
		    return YES;
		}
	    }
	}
	return NO;
}	/* end link_neighbor_tris */
