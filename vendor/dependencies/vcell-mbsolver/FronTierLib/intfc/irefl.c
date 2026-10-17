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
*				irefl.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Contains functions for the reflection of interfaces about
*	symmetry planes.
*/

#include <intfc/iloc.h>

LOCAL	void	reflect_coords(double*,double*,double*,int);

#if defined(ONED)
/*ARGSUSED*/
LIB_LOCAL	void	i_reflect_interface1d(
	INTERFACE*	intfc,	/* interface to be reflected */
	double*		p,	/* point on reflection plane */
	double*		nor)	/* plane normal */
{
	FT_POINT	  **pt;
	COMPONENT comp;
	int       n, np = intfc->num_points;

	if (intfc->points == NULL)
	    return;
	for (pt = intfc->points; pt && *pt; ++pt)
	{
	    reflect_point(*pt,p,nor,intfc);
	    comp = positive_component(*pt);
	    positive_component(*pt) = negative_component(*pt);
	    negative_component(*pt) = comp;
	}
	if (np > 1)
	{
	    pt = intfc->points;
	    intfc->points = NULL;
	    for (n = np-1; n >= 0; --n)
	        (void) add_to_pointers((POINTER)pt[n],
		                       (POINTER**)&intfc->points);
	}

	pt = intfc->points;
	for (n = 0; n < (np-1); ++n)
	{
	    if (!is_excluded_comp(positive_component(pt[n]),intfc))
	        positive_component(pt[n]) = negative_component(pt[n+1]) =
		    new_component(UNUSED_COMP);
	}
	if (nor[0] > 0.0)
	{
	    comp = negative_component(pt[0]);
	    if (is_interior_comp(comp,intfc) && !is_excluded_comp(comp,intfc))
	        negative_component(pt[0]) = new_component(UNUSED_COMP);
	}
	else
	{
	    comp = positive_component(pt[np-1]);
	    if (is_interior_comp(comp,intfc) && !is_excluded_comp(comp,intfc))
	        positive_component(pt[np-1]) = new_component(UNUSED_COMP);
	}
}		/*end i_reflect_interface1d*/
#endif /* defined(ONED) */


#if defined(TWOD)
LIB_LOCAL	void	i_reflect_interface2d(
	INTERFACE*	intfc,	/* interface to be reflected */
	double*		p,	/* point on reflection plane */
	double*		nor)	/* plane normal */
{
	NODE	**n;
	CURVE	**c;

	for (n = intfc->nodes; n && *n; ++n)
	    reflect_node(*n,p,nor);
	for (c = intfc->curves; c && *c; ++c)
	    reflect_curve(*c,p,nor);
}		/*end i_reflect_interface2d*/
#endif /* defined(TWOD) */

#if defined(THREED)
LIB_LOCAL	void	i_reflect_interface3d(
	INTERFACE*	intfc,	/* interface to be reflected */
	double*		p,	/* point on reflection plane */
	double*		nor)	/* plane normal */
{
	SURFACE **s;

	/* TODO: add 3D node and curve reflection */

	next_point(intfc,NULL,NULL,NULL); /* reset sort point status */
        for (s = intfc->surfaces; s && *s; ++s)
            reflect_surface(*s,p,nor);
}		/*end i_reflect_interface3d*/
#endif /* defined(THREED) */


EXPORT	void	i_reflect_node(
	NODE*		node,	/* node to be reflected */
	double*		p,	/* point on reflection plane */
	double*		nor)	/* plane normal */
{
	int	dim = node->interface->dim;
	reflect_coords(Coords((node)->posn),p,nor,dim);
}		/*end i_reflect_node*/

/*
*	IMPORTANT NOTE:  reflect_curve and reflect_surface
*	only operate on interior points of that object
*/

EXPORT	void	i_reflect_curve(
	CURVE*		curve,	/* curve to be reflected */
	double*		p,	/* point on reflection plane */
	double*		nor)	/* plane normal */
{
	BOND	*b;

	for (b = curve->first; b != curve->last; b = b->next)
	    reflect_point(b->end,p,nor,curve->interface);
	reverse_curve(curve);
}		/*end i_reflect_curve*/

#if defined(THREED)
/*ARGSUSED*/
EXPORT	void	i_reflect_surface(
	SURFACE*	s,/* surface to be reflected */
	double*		p,	/* point on reflection plane */
	double*		nor)	/* plane normal */
{
	INTERFACE *intfc = s->interface;
        TRI *tri;
        FT_POINT *p_tmp;
        POINTER t_tmp;
        int i, bdry12, bdry20;

        for (tri = first_tri(s); !at_end_of_tri_list(tri,s); tri = tri->next)
        {
            /* Reflect unreflected points on tri */

            for (i = 0; i < 3; ++i)
            {
                if (sorted(Point_of_tri(tri)[i]) == NO)
                {
                    reflect_point(Point_of_tri(tri)[i],p,nor,intfc);
                    sorted(Point_of_tri(tri)[i]) = YES;
                }
            }
            /* Reverse topological orientation */
            p_tmp = Point_of_tri(tri)[0];
            Point_of_tri(tri)[0] = Point_of_tri(tri)[1];
            Point_of_tri(tri)[1] = p_tmp;
            t_tmp = Neighbor_on_side12(tri);
            Neighbor_on_side12(tri) = Neighbor_on_side20(tri);
            Neighbor_on_side20(tri) = t_tmp;
            bdry12 = is_side12_a_bond(tri) ? YES : NO;
            bdry20 = is_side20_a_bond(tri) ? YES : NO;
            set_12_bdry(Boundary_tri(tri),bdry20);
            set_20_bdry(Boundary_tri(tri),bdry12);

        }
}		/*end i_reflect_surface*/
#endif /* defined(THREED) */

EXPORT	void	i_reflect_point(
	FT_POINT		*pt,
	double		*p,
	double		*nor,
	INTERFACE	*intfc)
{
	int dim = intfc->dim;
	reflect_coords(Coords(pt),p,nor,dim);
}		/*end i_reflect_point*/

LOCAL	void	reflect_coords(
	double 		*pt,
	double 		*p,
	double 		*nor, 
	int 		dim)
{
	double 		dp[MAXD], sp;
	int		i;

	for (i = 0; i < dim; ++i)
	    dp[i] = pt[i] - p[i];
	sp = 2.0*scalar_product(dp,nor,dim);
	for (i = 0; i < dim; ++i)
	    pt[i] -= sp*nor[i];
}		/*end reflect_coords*/
