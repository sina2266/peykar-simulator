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
*				icheck3d.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	The functions in this file check the pointers of the interface.
*/

#include <intfc/iloc.h>


	/* LOCAL Function Declarations */

#if defined(TWOD)
LOCAL	boolean	check_curve2d(CURVE*,INTERFACE*);
LOCAL	boolean 	i_consistent_interface2d(INTERFACE*);
#endif /* defined(TWOD) */
#if defined(THREED)
LOCAL	boolean 	i_consistent_interface3d(INTERFACE*);
LOCAL	boolean	check_consistency_of_tris_on_surface(SURFACE*);
LOCAL	boolean	check_tri(TRI*,INTERFACE*);
LOCAL	boolean	tri_side_consistent(TRI*,TRI_NEIGHBOR*,int,INTERFACE*);
LOCAL	boolean	allow_null_sides = NO;
LOCAL	boolean	check_curve3d(CURVE*,INTERFACE*);

EXPORT	void	null_sides_are_consistent(void)
{
	allow_null_sides = YES;
}		/*end null_sides_are_consistent*/
#endif /* defined(THREED) */

EXPORT	boolean i_consistent_interface(
	INTERFACE	*intfc)
{
	switch (intfc->dim)
	{
#if defined(TWOD)
	case 2:
	    return i_consistent_interface2d(intfc);
#endif /* defined(TWOD) */
#if defined(THREED)
	case 3:
	    return i_consistent_interface3d(intfc);
#endif /* defined(THREED) */
	}
}	/* end i_consistent_interface */

#if defined(TWOD)
LOCAL	boolean i_consistent_interface2d(
	INTERFACE	*intfc)
{
	CURVE              **c;
	NODE               **n;
	boolean               status = YES;
	const char         *warn = "WARNING in i_consistent_interface(), ";

	/* Check Nodes */
	for (n = intfc->nodes; n && *n; ++n)
	{
	    if ((*n)->interface != intfc)
	    {
		(void) printf("%s n = %llu n->interface (%llu) != intfc (%llu)\n",
			      warn,node_number(*n),
			      interface_number((*n)->interface),
			      interface_number(intfc));
		status = NO;
	    }
	    for (c = (*n)->in_curves; c && *c; ++c)
	    {
		if ((*c)->end != *n)
		{
		    (void) printf("%s inconsistent node (%llu) "
				  "curve (%llu) pair, "
				  "curve->end != n\n",
				  warn,node_number(*n),curve_number(*c));
		    status = NO;
		}
	    }
	    for (c = (*n)->out_curves; c && *c; ++c)
	    {
		if ((*c)->start != *n)
		{
		    (void) printf("%s inconsistent node (%llu) "
				  "curve (%llu) pair, "
				  "curve->start != n\n",
				  warn,node_number(*n),curve_number(*c));
		    status = NO;
		}
	    }
	}

	/* Check Curves */
	for (c = intfc->curves; c && *c; c++)
	{
	    if (!check_curve2d(*c,intfc))
	    {
	        (void) printf("%s inconsistency in curve (%llu) found\n",
			      warn,curve_number(*c));
		status = NO;
	    }
	}
	return status;

}	/* end i_consistent_interface2d */

LOCAL	boolean	check_curve2d(
	CURVE     *c,
	INTERFACE *intfc)
{
	BOND     *b;
	NODE     *ns, *ne;
	boolean     status = YES;
	char     warn[1024];

	(void) sprintf(warn,"WARNING in check_curve(), curve %llu inconsistent ",
		       curve_number(c));
	if (c->interface != intfc)
	{
	    (void) printf("%s c->interface (%llu) != intfc (%llu)\n",
			  warn,interface_number(c->interface),
			  interface_number(intfc));
	    status = NO;
	}
	ns = c->start;
	if (!pointer_is_in_array(c,ns->out_curves))
	{
	    (void) printf("%s curve in not in start node (%llu) "
			  "out_curves\n",warn,node_number(ns));
	    status = NO;
	}
	ne = c->end;
	if (!pointer_is_in_array(c,ne->in_curves))
	{
	    (void) printf("%s curve in not in end node (%llu) "
			  "in_curves\n",warn,node_number(ne));
	    status = NO;
	}
	if (ns->posn != c->first->start)
	{
	    (void) printf("%s ns->posn != c->first->start\n"
			  "c->first->start = %llu, "
			  "ns = %llu, ns->posn = %llu\n",
			  warn,point_number(c->first->start),
			  node_number(ns),point_number(ns->posn));
	    status = NO;
	}
	if (ne->posn != c->last->end)
	{
	    (void) printf("%s ne->posn != c->last->end\n"
			  "c->last->end = %llu, "
			  "ne = %llu, ne->posn = %llu\n",
			  warn,point_number(c->last->end),
			  node_number(ne),point_number(ne->posn));
	    status = NO;
	}
	if (c->first->prev != NULL)
	{
	    (void) printf("%s c->first->prev != NULL\n",warn);
	    print_bond(c->first);
	    print_bond(c->first->prev);
	    status = NO;
	}
	if (c->last->next != NULL)
	{
	    (void) printf("%s c->last->next != NULL\n",warn);
	    print_bond(c->last);
	    print_bond(c->last->next);
	    status = NO;
	}
	for (b = c->first; b != NULL; b = b->next)
	{
	    if (b->next && b->next->start != b->end)
	    {
	        (void) printf("%s bond pair (%llu -> %llu) point pointers "
			      "inconsistent\n",
			      warn,bond_number(b,intfc),
			      bond_number(b->next,intfc));
		print_bond(b);
		print_bond(b->next);
	        status = NO;
	    }
	    if ((b->next && b->next->prev != b) || 
	 	(b->prev && b->prev->next != b) ||
		b->next == b || b->prev == b)
	    {
	        (void) printf("%s bond pair (%llu -> %llu) link pointers "
			      "inconsistent\n",
			      warn,bond_number(b,intfc),
			      bond_number(b->next,intfc));
		print_bond(b);
		print_bond(b->next);
		return NO;	/* this is a potential deadloop, so exit */
	    }
	}
	return status;
}		/*end check_curve2d*/
#endif /* defined(TWOD) */

#if defined(THREED)
LOCAL	boolean i_consistent_interface3d(
	INTERFACE	*intfc)
{
	HYPER_SURF_ELEMENT *hse;
	HYPER_SURF         *hs;
	CURVE              **c;
	NODE               **n;
	FT_POINT              *p;
	SURFACE	           **ss, *s;
	TRI	           *tri;
	boolean               status = YES;
	const char         *warn = "WARNING in i_consistent_interface(), ";

	/* Check Nodes */
	for (n = intfc->nodes; n && *n; ++n)
	{
	    if ((*n)->interface != intfc)
	    {
		(void) printf("%s n = %llu n->interface (%llu) != intfc (%llu)\n",
			      warn,node_number(*n),
			      interface_number((*n)->interface),
			      interface_number(intfc));
		status = NO;
	    }
	    for (c = (*n)->in_curves; c && *c; ++c)
	    {
		if ((*c)->end != *n)
		{
		    (void) printf("%s inconsistent node (%llu) "
				  "curve (%llu) pair, "
				  "curve->end != n\n",
				  warn,node_number(*n),curve_number(*c));
		    status = NO;
		}
	    }
	    for (c = (*n)->out_curves; c && *c; ++c)
	    {
		if ((*c)->start != *n)
		{
		    (void) printf("%s inconsistent node (%llu) "
				  "curve (%llu) pair, "
				  "curve->start != n\n",
				  warn,node_number(*n),curve_number(*c));
		    status = NO;
		}
	    }
	}

	/* Check Curves */
	for (c = intfc->curves; c && *c; c++)
	{
	    if (!check_curve3d(*c,intfc))
	    {
	        (void) printf("%s inconsistency in curve (%llu) found\n",
			      warn,curve_number(*c));
		status = NO;
	    }
	}

	for (ss = intfc->surfaces; ss && *ss; ++ss)
	{
	    if (!check_consistency_of_tris_on_surface(*ss))
	    {
		(void) printf("%s inconsistency in surface (%llu) found\n",
				  warn,surface_number(*ss));
		status = NO;
	    }
	}

	(void) next_point(intfc,NULL,NULL,NULL);
	while (next_point(intfc,&p,&hse,&hs))
	{
	    BOND        *b = NULL, *bb;
	    BOND_TRI    **bts;
	    CURVE       **c;
	    TRI         **tris;
	    int         i, ntris;
	    int         v, pside, nside;

	    tri = Tri_of_hse(hse);
	    s = Surface_of_hs(hs);
	    if ((v = Vertex_of_point(tri,p)) == ERROR)
	    {
	        (void) printf("%s point not on tri, s = %llu\n",
			      warn,surface_number(s));
	    	(void) printf("p(%llu) - (%g, %g, %g), ",
	    		      point_number(p),
	    		      Coords(p)[0],Coords(p)[1],Coords(p)[2]);
		print_tri(tri,hs->interface);
		status = NO;
	    }
	    if (!Boundary_point(p))
	    {
		ntris = set_tri_list_around_point(p,tri,&tris,intfc);
		if ((tri != tris[0]) ||
		    (tri != Prev_tri_at_vertex(tris[ntris-1],p)))
		{
		    boolean consistent_tri_list = NO;
		    if (allow_null_sides)
		    {
			if ((Next_tri_at_vertex(tris[0],p) == NULL) &&
		            (Prev_tri_at_vertex(tris[ntris-1],p) == NULL))
			  consistent_tri_list = YES;  
		    }
		    if (!consistent_tri_list)
		    {
		        (void) printf("\n%s Corrupt tri list s (%llu) "
	    	                      "p(%llu) - (%g, %g, %g)\n",
				      warn,surface_number(s),
	    		              point_number(p),
	    		              Coords(p)[0],Coords(p)[1],Coords(p)[2]);
		        print_tri(tri,hs->interface);
		        (void) printf("%d tris about point\n",ntris);
		        for (i = 0; i < ntris; ++i)
		        {
			    (void) printf("tris[%d] - ",i);
			    print_tri(tris[i],hs->interface);
		        }
		        (void) printf("End printout of "
				      "Corrupt tri list s (%llu) "
	    	                      "p(%llu) - (%g, %g, %g)\n\n",
				      surface_number(s),point_number(p),
	    		              Coords(p)[0],Coords(p)[1],Coords(p)[2]);
		        status = NO;
		    }
		}
		continue;
	    }
	    nside = v;
	    pside = Prev_m3(v);
	    if (is_side_bdry(tri,nside))
		b = Bond_on_side(tri,nside);
	    else if (is_side_bdry(tri,pside))
		b = Bond_on_side(tri,pside);
	    else    /*#bjet2 */
	    {
		ntris = set_tri_list_around_point(p,tri,&tris,intfc);
		v = Vertex_of_point(tris[0],p);
		nside = v;
		pside = Prev_m3(v);
		if (is_side_bdry(tris[0],nside))
		    b = Bond_on_side(tris[0],nside);
		else if (is_side_bdry(tris[0],pside))
		    b = Bond_on_side(tris[0],pside);
		else
		{
		    int i;
	            (void) printf("%s Boundary_point has no adjacent "
				  "tri with a bond\n",warn);
	    	    (void) printf("p(%llu) - (%g, %g, %g), ",
	    		          point_number(p),
	    		          Coords(p)[0],Coords(p)[1],Coords(p)[2]);
		    print_tri(tri,hs->interface);
		    for (i = 0; i < ntris; ++i)
		    {
			(void) printf("tris[%d] - ",i);
			print_tri(tris[i],hs->interface);
		    }
		    status = NO;
		}
		tri = tris[0];
	    }
	    for (bts = Btris(b); bts && *bts; ++bts)
	        if ((*bts)->tri == tri)
	    	    break;
	    if ((bts == NULL) || (*bts == NULL))
	    {
		(void) printf("%s bond tri for tri  not found\n",warn);
	    	(void) printf("p(%llu) - (%g, %g, %g), ",point_number(p),
	    		      Coords(p)[0],Coords(p)[1],Coords(p)[2]);
		print_tri(tri,hs->interface);
		print_bond(b);
		status = NO;
	    }
	    else
	    {
	        if ((*bts)->bond != b)
	        {
		    (void) printf("%s (*bts)->bond != b\n",warn);
	    	    (void) printf("p(%llu) - (%g, %g, %g), ",point_number(p),
	    		          Coords(p)[0],Coords(p)[1],Coords(p)[2]);
		    print_tri(tri,hs->interface);
		    print_bond(b);
		    status = NO;
	        }
	        if ((*bts)->surface != s)
	        {
	            (void) printf("%s inconsistent surfaces at bond tri\n",
				  warn);
	    	    (void) printf("p(%llu) - (%g, %g, %g), ",point_number(p),
	    		          Coords(p)[0],Coords(p)[1],Coords(p)[2]);
		    print_tri(tri,hs->interface);
		    print_bond(b);
		    status = NO;
	        }
		if (orientation_of_bond_at_tri(b,tri) != (*bts)->orient)
		{
		    (void) printf("%s inconsistent orientation at bond tri\n",
				  warn);
	    	    (void) printf("p(%llu) - (%g, %g, %g), ",point_number(p),
	    		          Coords(p)[0],Coords(p)[1],Coords(p)[2]);
		    print_tri(tri,hs->interface);
		    print_bond(b);
		    status = NO;
		}
	        switch ((*bts)->orient)
	        {
	        case POSITIVE_ORIENTATION:
		    for (c = s->pos_curves; c && *c; c++)
		        if ((*c) == (*bts)->curve)
			    break;
	            break;
	        case NEGATIVE_ORIENTATION:
		    for (c = s->neg_curves; c && *c; c++)
		        if ((*c) == (*bts)->curve)
			    break;
	            break;
	        case ORIENTATION_NOT_SET:
		    c = NULL;
	            (void) printf("%s undetermined orientation at "
				  "bond on tri\n",warn);
	    	    (void) printf("p(%llu) - (%g, %g, %g), ",point_number(p),
	    		          Coords(p)[0],Coords(p)[1],Coords(p)[2]);
		    print_tri(tri,hs->interface);
		    print_bond(b);
		    status = NO;
		    break;
	        }
	        if ((c == NULL) || (*c == NULL))
	        {
		    (void) printf("%s curve with bond on tri not found\n",warn);
	    	    (void) printf("p(%llu) - (%g, %g, %g), ",point_number(p),
	    		          Coords(p)[0],Coords(p)[1],Coords(p)[2]);
		    print_tri(tri,hs->interface);
		    print_bond(b);
		    status = NO;
	        }
	        else
	        {
	            for (bb = (*c)->first; bb != NULL; bb = bb->next)
		        if (bb == b)
		            break;
	            if (bb == NULL)
	            {
		        (void) printf("%s bond not on curve\n",warn);
	    	        (void) printf("p(%llu) - (%g, %g, %g), ",point_number(p),
	    		              Coords(p)[0],Coords(p)[1],Coords(p)[2]);
		        print_tri(tri,hs->interface);
		        print_bond(b);
		        print_curve(*c);
		        status = NO;
	            }
	        }
	    }
	}

	if (status == NO)
	{
	    (void) printf("WARNING in i_consistent_interface(), "
	                  "Inconsistent interface found\n");
	    print_interface(intfc);
	}

	allow_null_sides = NO;
	return status;
}		/*end i_consistent_interface*/

LIB_LOCAL void check_tri_and_bond(
	TRI	   *tri,
	BOND	   *b,
	const char *str,
	INTERFACE  *intfc)
{
	FT_POINT		*ps,*pe,*p;
	int		i, j = 0;
	char		s1[256],s2[256],s3[256];

	(void) sprintf(s2,"in check_tri_and_bond()\n %s: ps==pe\n",str);
	(void) sprintf(s3,"in check_tri_and_bond()\n %s: bond not match tri\n",
	               str);
	(void) sprintf(s1,"in check_tri_and_bond()\n %s: NULL tri\n",str);

	if (tri == NULL)
	{
	    (void) printf("%s",s1);
	    print_bond(b);
	    return;
	}

	ps = b->start, pe = b->end;

	if (ps == pe)
	{
	    (void) printf("%s",s2);
	    print_bond(b);
	    return;
	}

	for (i = 0; i < 3; ++i)
	{
	    p = Point_of_tri(tri)[i];
	    if ((p == ps) || (p == pe))
	    	++j;
	}

	if (j != 2)
	{
	    (void) printf("%s",s3);
	    print_bond(b);
	    print_tri(tri,intfc);
	    return;
	}
}		/*end check_tri_and_bond*/


LOCAL	boolean	check_curve3d(
	CURVE     *c,
	INTERFACE *intfc)
{
	BOND     *b;
	BOND_TRI **bts, **bts0;
	NODE     *ns, *ne;
	SURFACE  *s, **ss;
	TRI      *tri, **tris;
	boolean     status = YES;
	char     warn[1024];
	int      nsides, nbts, i;
	int      ntris;

	(void) sprintf(warn,"WARNING in check_curve3d(), curve %llu inconsistent ",
		       curve_number(c));
	if (c->interface != intfc)
	{
	    (void) printf("%s c->interface (%llu) != intfc (%llu)\n",
			  warn,interface_number(c->interface),
			  interface_number(intfc));
	    status = NO;
	}
	ns = c->start;
	if (!pointer_is_in_array(c,ns->out_curves))
	{
	    (void) printf("%s curve in not in start node (%llu) "
			  "out_curves\n",warn,node_number(ns));
	    status = NO;
	}
	ne = c->end;
	if (!pointer_is_in_array(c,ne->in_curves))
	{
	    (void) printf("%s curve in not in end node (%llu) "
			  "in_curves\n",warn,node_number(ne));
	    status = NO;
	}
	if (ns->posn != c->first->start)
	{
	    (void) printf("%s ns->posn != c->first->start\n"
			  "c->first->start = %llu, "
			  "ns = %llu, ns->posn = %llu\n",
			  warn,point_number(c->first->start),
			  node_number(ns),point_number(ns->posn));
	    status = NO;
	}
	if (ne->posn != c->last->end)
	{
	    (void) printf("%s ne->posn != c->last->end\n"
			  "c->last->end = %llu, "
			  "ne = %llu, ne->posn = %llu\n",
			  warn,point_number(c->last->end),
			  node_number(ne),point_number(ne->posn));

	    print_general_vector("c->last->end", Coords(c->last->end), 3, "\n");
	    print_general_vector("ne->posn", Coords(ne->posn), 3, "\n");
	    print_curve(c);
	    status = NO;
	}
	if (!Boundary_point(ns->posn))
	{
	    (void) printf("%s ns->posn (ns = %llu, ns->posn = %llu) is not a "
			  "boundary point\n",
			  warn,node_number(ns),point_number(ns->posn));
	    status = NO;
	}
	if (!Boundary_point(ne->posn))
	{
	    (void) printf("%s ne->posn (ne = %llu, ne->posn = %llu) is not a "
			  "boundary point\n",
			  warn,node_number(ne),point_number(ne->posn));
	    status = NO;
	}
	if (!Boundary_point(c->first->start))
	{
	    (void) printf("%s c->first->start = %llu is not a "
			  "boundary point\n",
			  warn,point_number(c->first->start));
	    status = NO;
	}
	if (!Boundary_point(c->last->end))
	{
	    (void) printf("%s c->last->end = %llu is not a "
			  "boundary point\n",
			  warn,point_number(c->last->end));
	    status = NO;
	}
	for (ss = c->pos_surfaces; ss && *ss; ++ss)
	{
	    if (!pointer_is_in_array(c,(*ss)->pos_curves))
	    {
	        (void) printf("%s curve in not s->pos_curves "
			      " s = %llu but s is in c->neg_surfaces\n",
			      warn,surface_number(*ss));
		status = NO;
	    }
	}
	for (ss = c->neg_surfaces; ss && *ss; ++ss)
	{
	    if (!pointer_is_in_array(c,(*ss)->neg_curves))
	    {
	        (void) printf("%s curve in not s->neg_curves "
			      " s = %llu but s is in c->neg_surfaces\n",
			      warn,surface_number(*ss));
		status = NO;
	    }
	}
	b = c->first;
	bts0 = Btris(b);
	for (nbts = 0, bts = Btris(b); bts && *bts; ++nbts, ++bts);
	if (nbts == 0)
	{
	    if (c->pos_surfaces || c->neg_surfaces)
	    {
		(void) printf("%s curve has no bond tris but is "
			      "connected to some surface\n",warn);
		status = NO;
	    }
	}
	if (c->first->prev != NULL)
	{
	    (void) printf("%s c->first->prev != NULL\n",warn);
	    print_bond(c->first);
	    print_bond(c->first->prev);
	    status = NO;
	}
	if (c->last->next != NULL)
	{
	    (void) printf("%s c->last->next != NULL\n",warn);
	    print_bond(c->last);
	    print_bond(c->last->next);
	    status = NO;
	}
	for (b = c->first; b != NULL; b = b->next)
	{
	    if (b->next && b->next->start != b->end)
	    {
	        (void) printf("%s bond pair (%llu -> %llu) point pointers "
			      "inconsistent\n",
			      warn,bond_number(b,intfc),
			      bond_number(b->next,intfc));
		print_bond(b);
		print_bond(b->next);
	        status = NO;
	    }
	    if (!Boundary_point(b->start))
	    {
	        (void) printf("%s b->start = %llu is not a "
			      "boundary point\n",
			      warn,point_number(b->start));
		print_bond(b);
	        status = NO;
	    }
	    if (!Boundary_point(b->end))
	    {
	        (void) printf("%s b->end = %llu is not a "
			      "boundary point\n",
			      warn,point_number(b->end));
		print_bond(b);
	        status = NO;
	    }
	    for (i = 0, bts = Btris(b); bts && *bts; ++i, ++bts)
	    {
		if ((i < nbts) &&
		    !(((*bts)->surface == bts0[i]->surface) &&
		          ((*bts)->orient == bts0[i]->orient)))
		{
	            (void) printf("%s inconsistent surface numbers on "
				  "bond tri\n",warn);
		    (void) printf("surface = %llu surface[%d] = %llu\n",
				  surface_number((*bts)->surface),i,
				  surface_number(bts0[i]->surface));
		    (void) printf("orient = %s orient[%d] = %s\n",
				  orientation_name((*bts)->orient),i,
				  orientation_name(bts0[i]->orient));
		    print_bond(b);
	            status = NO;
		}
	    }
	    if (i != nbts)
	    {
	        (void) printf("%s inconsistent %d != %d number of bond tris "
			      "on bond\n",warn,i,nbts);
		print_bond(b);
	        status = NO;
	    }
	    for (bts = Btris(b); bts && *bts; ++bts)
	    {
		if ((*bts)->curve != c)
		{
		    (void) printf("%s bond tri curve field (%llu) != c\n",
				  warn,curve_number((*bts)->curve));
		    print_bond(b);
		    status = NO;
		}
		if ((*bts)->bond != b)
		{
		    (void) printf("%s bond tri bond field (%llu) != b (%llu)\n",
				  warn,bond_number((*bts)->bond,intfc),
				  bond_number(b,intfc));
		    print_bond(b);
		    status = NO;
		}
		tri = (*bts)->tri;
		s = Surface_of_tri(tri);
		if ((*bts)->surface != s)
		{
		    (void) printf("%s bond tri surface field (%llu)"
				  "!= Surface_of_tri(tri) (%llu)\n",
				  warn,surface_number((*bts)->surface),
				  surface_number(s));
		    print_bond(b);
		    status = NO;
		}
		for (nsides = 0, i = 0; i < 3; ++i)
		{
		    if (is_side_bdry(tri,i) && (b==Bond_on_side(tri,i)))
			++nsides;
		}
		if (nsides == 0)
		{
		    (void) printf("%s bond not found on tri side\n",warn);
		    print_bond(b);
		    print_tri(tri,intfc);
		    status = NO;
		}
		else if (nsides > 1)
		{
		    (void) printf("%s bond found on multiple tri sides\n",warn);
		    print_bond(b);
		    print_tri(tri,intfc);
		    status = NO;
		}
		else
		{
		    if (orientation_of_bond_at_tri(b,tri) != (*bts)->orient)
		    {
		        (void) printf("%s inconsistent orientation at "
				      "bond tri\n",warn);
		        print_tri(tri,intfc);
		        print_bond(b);
		        status = NO;
		    }
		    switch ((*bts)->orient)
		    {
		    case POSITIVE_ORIENTATION:
	                if (!pointer_is_in_array(c,s->pos_curves))
	                {
	                    (void) printf("%s curve in not s->pos_curves "
					  " s = %llu\n",
			                  warn,surface_number(s));
		            print_bond(b);
		            print_tri(tri,intfc);
	                    status = NO;
	                }
	                if (!pointer_is_in_array(s,c->pos_surfaces))
	                {
	                    (void) printf("%s surface in not c->pos_surfaces "
					  " s = %llu\n",
			                  warn,surface_number(s));
		            print_bond(b);
		            print_tri(tri,intfc);
	                    status = NO;
	                }
			break;
		    case NEGATIVE_ORIENTATION:
	                if (!pointer_is_in_array(c,s->neg_curves))
	                {
	                    (void) printf("%s curve in not s->neg_curves "
					  " s = %llu\n",
			                  warn,surface_number(s));
		            print_bond(b);
		            print_tri(tri,intfc);
	                    status = NO;
	                }
	                if (!pointer_is_in_array(s,c->neg_surfaces))
	                {
	                    (void) printf("%s surface in not c->neg_surfaces "
					  " s = %llu\n",
			                  warn,surface_number(s));
		            print_bond(b);
		            print_tri(tri,intfc);
	                    status = NO;
	                }
			break;
		    case ORIENTATION_NOT_SET:
			(void) printf("%s inconsistent point and tri "
				      "points\n",warn);
		        print_bond(b);
		        print_tri(tri,intfc);
	                status = NO;
			break;
		    }
		}
		if (b->prev)
		{
		    TRI *t0, *t1;
	            ntris = set_tri_list_around_point(b->start,tri,&tris,intfc);
		    t0 = tris[0]; t1 = tris[ntris-1];
		    if (!(((side_of_tri_with_bond(b,t0) < 3) &&
			      (side_of_tri_with_bond(b->prev,t1) < 3))
		        ||
			     ((side_of_tri_with_bond(b,t1) < 3) &&
			      (side_of_tri_with_bond(b->prev,t0) < 3)))
		       )
		    {
			(void) printf("%s, corrupt tri list at b->start\n",
				      warn);
		        (void) printf("Bond b\n"); print_bond(b);
		        (void) printf("Bond b->prev\n"); print_bond(b->prev);
		        print_tri(tri,intfc);
			(void) printf("number of tris at point = %d\n",ntris);
			for (i = 0; i < ntris; ++i)
			{
			    (void) printf("tris[%d] - ",i);
			    print_tri(tris[i],intfc);
			}
			status = NO;
		    }
		}
	    }
	}
	return status;
}		/*end check_curve3d*/

LOCAL	boolean check_consistency_of_tris_on_surface(
	SURFACE		*s)
{
	TRI       *t, *tri;
	INTERFACE *intfc = s->interface;
	boolean      status = YES;
	char      warn[1024];
	int       i;

	(void) sprintf(warn,"WARNING in "
			    "check_consistency_of_tris_on_surface(), "
	                    "surface %llu inconsistent ",surface_number(s));

	for (i=0, t=first_tri(s); !at_end_of_tri_list(t,s); t=t->next, ++i)
	{
	    if (t == NULL)
	    {
		(void) printf("%s null tri found in tri list\n",warn);
		status = NO;
		break;
	    }
	}
	if (i != s->num_tri)
	{
	    (void) printf("%s number of tris on surface inconsistent\n",warn);
	    (void) printf("counted = %d	s->num_tri = %d\n",i,s->num_tri);
	    (void) printf("head = 0x%p  tail = 0x%p  first = 0x%p\n",
	    	          head_of_tri_list(s),
	    	          tail_of_tri_list(s), 
	    	          first_tri(s));
	    (void) printf("first->prev = 0x%p  first->next = 0x%p\n",
	    	          first_tri(s)->prev,first_tri(s)->next);
	    status = NO;
	}

	for (i=0, tri=first_tri(s); i < s->num_tri; ++i, tri=tri->next)
	{
	    if (tri == NULL)
		break;
	    if (!check_tri(tri,intfc))
	    {
		(void) printf("%s check_tri failed\n",warn);
		status = NO;
	    }
	}
	return status;
}		/*end check_consistency_of_tris_on_surface*/



LOCAL	boolean	check_tri(
	TRI	  *tri,
	INTERFACE *intfc)
{
	FT_POINT **p;
	boolean  status = YES;
	int   side;

	if (tri == NULL)
	{
	    (void) printf("WARNING in check_tri(), tri is NULL\n");
	    return NO;
	}

	p = Point_of_tri(tri);
	if ((p[0] == p[1]) || (p[0] == p[2]) || (p[1] == p[2]))
	{
	    (void) printf("WARNING in check_tri(), two points are equal\n");
	    print_tri(tri,intfc);
	    status = NO;
	}

	for (side = 0; side < 3; ++side)
	{
	    if (!tri_side_consistent(tri,Tri_neighbor(tri)+side,side,intfc))
	    {
		(void) printf("WARNING in check_tri(), "
			      "side %d of tri inconsistent\n",side);
	        print_tri(tri,intfc);
		status = NO;
	    }
	}
	return status;
}		/*end check_tri*/



LOCAL	boolean tri_side_consistent(
	TRI	     *tri,
	TRI_NEIGHBOR *neighbor,
	int	     side,
	INTERFACE    *intfc)
{
	TRI	*side_tri;
	FT_POINT	*ps = Point_of_tri(tri)[side];
	FT_POINT	*pe = Point_of_tri(tri)[Next_m3(side)];
	boolean status = YES;
	int	i, bd;

	bd = Boundary_tri(tri);
	if (bd < 0 || bd > 7)
	{
	    (void) printf("WARNING in tri_side_consistent(), "
	        	  "Boundary_tri(tri) = %d is bad\n", bd);
	    print_tri(tri,intfc);
	    status = NO;
	}

	if (neighbor == NULL)
	{
	    if (!allow_null_sides)
	    {
	        (void) printf("WARNING in tri_side_consistent(), "
	        	      "neighbor is null\n");
	        print_tri(tri,intfc);
	        status = NO;
	    }
	}
	else if (is_side_bdry(tri,side))
	{
	    BOND        *b;
	    BOND_TRI	*bt = neighbor->btri;
	    BOND_TRI	**btris;
	    ORIENTATION orient;
	    boolean        btri_is_in_list;
	    int         num_tri_in_btri;

	    if (bt == NULL)
	    {
	        (void) printf("WARNING in tri_side_consistent(), "
	        	      "NULL BOND_TRI on tri side %d\n",side);
	        print_tri(tri,intfc);
	        status = NO;
	    }
	    else if ((b = bt->bond) == NULL)
	    {
	        (void) printf("WARNING in tri_side_consistent(), "
	        	      "NULL bond on BOND_TRI on side %d\n",side);
	        print_tri(tri,intfc);
	        status = NO;
	    }
	    else
	    {
		btri_is_in_list = NO;
	        for (num_tri_in_btri = 0, btris = Btris(b); btris && *btris;
		     ++btris)
	        {
	    	    if ((*btris)->tri == tri)
		    {
			++num_tri_in_btri;
			if (bt == *btris)
			    btri_is_in_list = YES;
		    }
	        }
	        if (num_tri_in_btri == 0)
	        {
	            (void) printf("WARNING in tri_side_consistent(), "
	    	                  "Tri is not on bond btris list,\n");
	    	    (void) printf("ps(%llu) - (%g, %g, %g), ",
	    		          point_number(ps),
	    		          Coords(ps)[0],Coords(ps)[1],Coords(ps)[2]);
	    	    (void) printf("\tpe(%llu) - (%g, %g, %g)\n",
	    		          point_number(pe),
	    		          Coords(pe)[0],Coords(pe)[1],Coords(pe)[2]);
	    	    (void) printf("tri - \n");
	            print_tri(tri,intfc);
	    	    (void) printf("bond - \n");
	            print_bond(b);
		    status = NO;
	        }
		else if (num_tri_in_btri != 1)
		{
	            (void) printf("WARNING in tri_side_consistent(), Tri is "
				  "in bond btris list more than once,\n");
	    	    (void) printf("ps(%llu) - (%g, %g, %g), ",
	    		          point_number(ps),
	    		          Coords(ps)[0],Coords(ps)[1],Coords(ps)[2]);
	    	    (void) printf("\tpe(%llu) - (%g, %g, %g)\n",
	    		          point_number(pe),
	    		          Coords(pe)[0],Coords(pe)[1],Coords(pe)[2]);
	    	    (void) printf("tri - \n");
	            print_tri(tri,intfc);
	    	    (void) printf("bond - \n");
	            print_bond(b);
		    status = NO;
		}
		if (!btri_is_in_list)
		{
	            (void) printf("WARNING in tri_side_consistent(), Bond tri "
				  "is not in bond btris list,\n");
	    	    (void) printf("ps(%llu) - (%g, %g, %g), ",
	    		          point_number(ps),
	    		          Coords(ps)[0],Coords(ps)[1],Coords(ps)[2]);
	    	    (void) printf("\tpe(%llu) - (%g, %g, %g)\n",
	    		          point_number(pe),
	    		          Coords(pe)[0],Coords(pe)[1],Coords(pe)[2]);
	    	    (void) printf("tri - \n");
	            print_tri(tri,intfc);
	    	    (void) printf("bond - \n");
	            print_bond(b);
		    status = NO;
		}
		orient = orientation_of_bond_at_tri(b,tri);
	        if (orient == ORIENTATION_NOT_SET)
	        {
	            (void) printf("WARNING in tri_side_consistent(), "
	    	                  "Tri side points not on bond,\n");
	    	    (void) printf("ps(%llu) - (%g, %g, %g), ",
	    		          point_number(ps),
	    		          Coords(ps)[0],Coords(ps)[1],Coords(ps)[2]);
	    	    (void) printf("\tpe(%llu) - (%g, %g, %g)\n",
	    		          point_number(pe),
	    		          Coords(pe)[0],Coords(pe)[1],Coords(pe)[2]);
	    	    (void) printf("tri - \n");	print_tri(tri,intfc);
	    	    (void) printf("bond - \n");	print_bond(b);
		    status = NO;
	        }
		if (orient != bt->orient)
		{
	            (void) printf("WARNING in tri_side_consistent(), "
	    	                  "inconsistent bond tri orientation,\n");
	    	    (void) printf("ps(%llu) - (%g, %g, %g), ",
	    		          point_number(ps),
	    		          Coords(ps)[0],Coords(ps)[1],Coords(ps)[2]);
	    	    (void) printf("\tpe(%llu) - (%g, %g, %g)\n",
	    		          point_number(pe),
	    		          Coords(pe)[0],Coords(pe)[1],Coords(pe)[2]);
	    	    (void) printf("tri - \n");	print_tri(tri,intfc);
	    	    (void) printf("bond - \n");	print_bond(b);
		    status = NO;
		}
	    }
	}
	else
	{
	    if ((side_tri = neighbor->tri) == NULL)
	    {
	        if (!allow_null_sides)
		{
		    status = NO;
	            (void) printf("WARNING in tri_side_consistent(), "
	                          "neighbor->tri == NULL\n");
	            (void) printf("tri - \n");
	            print_tri(tri,intfc);
		}
	    }
	    else
	    {
	      for (i = 0; i < 3; ++i)
	      {
	        if (Tri_on_side(side_tri,i) == tri)
	        {
	          FT_POINT *p0, *p1;
	          p0 = Point_of_tri(side_tri)[i];
	          p1 = Point_of_tri(side_tri)[Next_m3(i)];
	          if (p0 != pe || p1 != ps)   
	          {
	              (void) printf("WARNING in tri_side_consistent(), "
	    	                    "Points on side %d do not match,\n",i);
	    	      (void) printf("ps(%llu) - (%g, %g, %g), ",
	    	                    point_number(ps),
	    		            Coords(ps)[0],
				    Coords(ps)[1],
				    Coords(ps)[2]);
	    	      (void) printf("\tpe(%llu) - (%g, %g, %g)\n",
	    		            point_number(pe),
	    		            Coords(pe)[0],
				    Coords(pe)[1],
				    Coords(pe)[2]);
	    	      (void) printf("p1(%llu) - (%g, %g, %g), ",
	    		            point_number(p1),
	    		            Coords(p1)[0],
				    Coords(p1)[1],
				    Coords(p1)[2]);
	    	      (void) printf("\tp0(%llu) - (%g, %g, %g)\n",
	    		            point_number(p0),
	    		            Coords(p0)[0],
				    Coords(p0)[1],
				    Coords(p0)[2]);
	    	      (void) printf("tri - ");	  print_tri(tri,intfc);
	    	      (void) printf("side_tri - "); print_tri(side_tri,intfc);
		      status = NO;
	          }
		  break;
	        }
	      }
	      if (i == 3)
	      {
	        (void) printf("WARNING in tri_side_consistent(), "
	                      "No side matches tri\n");
	        (void) printf("ps(%llu): (%g, %g, %g), "
			      "\tpe(%llu): (%g, %g, %g)\n",
	    	              point_number(ps),
	    	              Coords(ps)[0],Coords(ps)[1],Coords(ps)[2],
	    	              point_number(pe),
	    	              Coords(pe)[0],Coords(pe)[1],Coords(pe)[2]);
	        (void) printf("tri - \n");
	        print_tri(tri,intfc);
	        (void) printf("side_tri - \n");
	        print_tri(side_tri,intfc);
		status = NO;
	      }
	    }
	}
	return status;
}		/*end tri_side_consistent*/

EXPORT	void check_double_cone_point(
	INTERFACE *intfc)
{
	SURFACE **s;
	TRI *tri,*ptris[30],**tris;
	int i,j,num_ptris,num_tris;
	FT_POINT *p;
	HYPER_SURF *hs;
	HYPER_SURF_ELEMENT *hse;
	boolean ptri_in_tri_list;

	(void) printf("Start checking double cone point\n");
	next_point(intfc,NULL,NULL,NULL);
	while (next_point(intfc,&p,&hse,&hs))
	{
	    num_ptris = 0;
	    num_tris = set_tri_list_around_point(p,Tri_of_hse(hse),
	    	&tris,intfc);
	    for (s = intfc->surfaces; s && *s; ++s)
	    {
	    	for (tri = first_tri(*s); !at_end_of_tri_list(tri,*s);
				tri = tri->next)
		{
		    if (Point_of_tri(tri)[0] == p ||
		    	Point_of_tri(tri)[1] == p ||
			Point_of_tri(tri)[2] == p)
			ptris[num_ptris++] = tri;
		}
	    }
	    for (i = 0; i < num_ptris; ++i)
	    {
	    	ptri_in_tri_list = NO;
	    	tri = ptris[i];
		for (j = 0; j < num_tris; ++j)
		{
		    if (tri == tris[j])
		    {
		    	ptri_in_tri_list = YES;
			break;
		    }
		}
		if (!ptri_in_tri_list)
		{
		    (void) printf("double cone point found:\n");
		    (void) printf("double cone point p(%p) = %f %f %f\n",p,
		    		Coords(p)[0],Coords(p)[1],Coords(p)[2]);
		}
	    }
	}
	(void) printf("End checking double cone point\n");
}	/* end check_double_cone_point */
#endif /* defined(THREED) */
