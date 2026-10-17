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
*				fcheck3d.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Tests consistency of interface structures.
*
*/

#if defined(THREED)

#include <front/fdecs.h>

	/* LOCAL Function Declarations */

EXPORT	boolean	f_consistent_interface(
	INTERFACE *intfc)
{
	BOND               *b;
	BOND_TRI	   **bts;
	CURVE              **c;
	HYPER_SURF         *hs;
	HYPER_SURF_ELEMENT *hse;
	FT_POINT	           *p;
	SURFACE            *s;
	TRI		   *tri;
	boolean            status;
	int                i;
	const char         *warn = "WARNING in f_consistent_interface(), ";

	if (intfc->dim != 3)
	    return NO;

	status = i_consistent_interface(intfc);

	if (size_of_state(intfc) == 0)
	    return status;

	/* Check for allocation of states */
	(void) next_point(intfc,NULL,NULL,NULL);
	while (next_point(intfc,&p,&hse,&hs))
	{
	    if (left_state(p) == NULL)
	    {
		s = Surface_of_hs(hs);
		tri = Tri_of_hse(hse);
		(void) printf("%s left state at point is NULL\n",warn);
		(void) printf("p %llu %g %g %g\n",point_number(p),
			      Coords(p)[0],Coords(p)[1],Coords(p)[2]);
		(void) printf("surface = %llu\n",surface_number(s));
		print_tri(tri,intfc);
		status = NO;
	    }
	    if (right_state(p) == NULL)
	    {
		s = Surface_of_hs(hs);
		tri = Tri_of_hse(hse);
		(void) printf("%s right state at point is NULL\n",warn);
		(void) printf("p %llu %g %g %g\n",point_number(p),
			      Coords(p)[0],Coords(p)[1],Coords(p)[2]);
		(void) printf("surface = %llu\n",surface_number(s));
		print_tri(tri,intfc);
		status = NO;
	    }
	}

	for (c = intfc->curves; c && *c; ++c)
	{
	  for (b = (*c)->first; b != NULL; b = b->next)
	  {
	    for (i = 0, bts = Btris(b); bts && *bts; ++i, ++bts)
	    {
	      if (left_start_btri_state(*bts) == NULL)
	      {
	        (void) printf("%s left start btri state is NULL\n",warn);
	        (void) printf("curve = %llu\n",curve_number(*c));
	        (void) printf("surface = %llu\n",surface_number((*bts)->surface));
	        (void) printf("orient = %s\n",orientation_name((*bts)->orient));
	        print_bond(b);
	        print_tri((*bts)->tri,intfc);
	        status = NO;
	      }
	      if (right_start_btri_state(*bts) == NULL)
	      {
	        (void) printf("%s right start btri state is NULL\n",warn);
	        (void) printf("curve = %llu\n",curve_number(*c));
	        (void) printf("surface = %llu\n",surface_number((*bts)->surface));
	        (void) printf("orient = %s\n",orientation_name((*bts)->orient));
	        print_bond(b);
	        print_tri((*bts)->tri,intfc);
	        status = NO;
	      }
	      if (b->prev)
	      {
	        if (i < size_of_pointers(Btris(b->prev)))
	        {
	          if (left_start_btri_state(*bts) !=
		      left_end_btri_state(Btris(b->prev)[i]))
	          {
	            (void) printf("%s left states different at "
	                          "b->start\n",warn);
	            (void) printf("curve = %llu\n",curve_number(*c));
	            (void) printf("surface = %llu\n",surface_number((*bts)->surface));
	            (void) printf("orient = %s\n",
	                          orientation_name((*bts)->orient));
	            (void) printf("prev surface = %llu\n",
	                          surface_number(Btris(b->prev)[i]->surface));
	            (void) printf("prev orient = %s\n",
	                          orientation_name(Btris(b->prev)[i]->orient));
	            (void) printf("b - "); print_bond(b);
	            (void) printf("b->prev - "); print_bond(b->prev);
	            (void) printf("tri - ");
		    print_tri((*bts)->tri,intfc);
	            (void) printf("prev tri - ");
		    print_tri(Btris(b->prev)[i]->tri,intfc);
	            status = NO;
	          }
	          if (right_start_btri_state(*bts) !=
		      right_end_btri_state(Btris(b->prev)[i]))
	          {
	            (void) printf("%s right states different at "
	                          "b->start\n",warn);
	            (void) printf("curve = %llu\n",curve_number(*c));
	            (void) printf("surface = %llu\n",surface_number((*bts)->surface));
	            (void) printf("orient = %s\n",
	                          orientation_name((*bts)->orient));
	            (void) printf("prev surface = %llu\n",
	                          surface_number(Btris(b->prev)[i]->surface));
	            (void) printf("prev orient = %s\n",
	                          orientation_name(Btris(b->prev)[i]->orient));
	            (void) printf("b - "); print_bond(b);
	            (void) printf("b->prev - "); print_bond(b->prev);
	            (void) printf("tri - ");
		    print_tri((*bts)->tri,intfc);
	            (void) printf("prev tri - ");
		    print_tri(Btris(b->prev)[i]->tri,intfc);
	            status = NO;
	          }
	        }
	        else
	        {
	          (void) printf("%s inconsistent numbers of "
	                        "bond tris\n",warn);
	          (void) printf("b has %d bond tris while "
	                        "b->prev has %d bond tris\n",
	                        (int)size_of_pointers(Btris(b)),
	                        (int)size_of_pointers(Btris(b->prev)));
	          (void) printf("curve = %llu\n",curve_number(*c));
	          (void) printf("surface = %llu\n",surface_number((*bts)->surface));
	          (void) printf("orient = %s\n",
	                        orientation_name((*bts)->orient));
	          (void) printf("b - "); print_bond(b);
	          (void) printf("b->prev - "); print_bond(b->prev);
	          print_tri((*bts)->tri,intfc);
	          status = NO;
	        }
	      }
	      if (b->next)
	      {
	        if (i < size_of_pointers(Btris(b->next)))
	        {
	          if (left_end_btri_state(*bts) !=
		      left_start_btri_state(Btris(b->next)[i]))
	          {
	            (void) printf("%s left states different at "
	                          "b->end\n",warn);
	            (void) printf("curve = %llu\n",curve_number(*c));
	            (void) printf("surface = %llu\n",
		                  surface_number((*bts)->surface));
	            (void) printf("orient = %s\n",
	                          orientation_name((*bts)->orient));
	            (void) printf("next surface = %llu\n",
	                          surface_number(Btris(b->next)[i]->surface));
	            (void) printf("next orient = %s\n",
	                          orientation_name(Btris(b->next)[i]->orient));
	            (void) printf("b - "); print_bond(b);
	            (void) printf("b->next - "); print_bond(b->next);
	            (void) printf("tri - ");
		    print_tri((*bts)->tri,intfc);
	            (void) printf("next tri - ");
		    print_tri(Btris(b->next)[i]->tri,intfc);
	            status = NO;
	          }
	          if (right_end_btri_state(*bts) !=
		      right_start_btri_state(Btris(b->next)[i]))
	          {
	            (void) printf("%s right states different at "
	                          "b->end\n",warn);
	            (void) printf("curve = %llu\n",curve_number(*c));
	            (void) printf("surface = %llu\n",
		                  surface_number((*bts)->surface));
	            (void) printf("orient = %s\n",
	                          orientation_name((*bts)->orient));
	            (void) printf("next surface = %llu\n",
	                          surface_number(Btris(b->next)[i]->surface));
	            (void) printf("next orient = %s\n",
	                          orientation_name(Btris(b->next)[i]->orient));
	            (void) printf("b - "); print_bond(b);
	            (void) printf("b->next - "); print_bond(b->next);
	            (void) printf("tri - ");
		    print_tri((*bts)->tri,intfc);
	            (void) printf("next tri - ");
		    print_tri(Btris(b->next)[i]->tri,intfc);
	            status = NO;
	          }
	        }
	        else
	        {
	            (void) printf("%s inconsistent numbers of "
	                          "bond tris\n",warn);
	            (void) printf("b has %d bond tris while "
	                          "b->next has %d bond tris\n",
	                          (int)size_of_pointers(Btris(b)),
	                          (int)size_of_pointers(Btris(b->next)));
	            (void) printf("curve = %llu\n",curve_number(*c));
	            (void) printf("surface = %llu\n",
		                  surface_number((*bts)->surface));
	            (void) printf("orient = %s\n",
	                          orientation_name((*bts)->orient));
	            (void) printf("b - "); print_bond(b);
	            (void) printf("b->next - "); print_bond(b->next);
	            print_tri((*bts)->tri,intfc);
	            status = NO;
	        }
	      }
	    }
	  }
	}
	if (status == NO)
	{
	    (void) printf("Inconsistent interface\n");
	    print_interface(intfc);
	}
	return status;
}        /*end f_consistent_interface*/

#endif /* defined(THREED) */
