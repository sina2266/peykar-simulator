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
*				idiagnositc.c:
*
*
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*/


#include <intfc/iloc.h>
#include <intfc/ilocprotos.h>

LOCAL 	void 	data_of_point(FT_POINT*,int);
	
EXPORT 	int 	index_of_pointer(
	POINTER	*array,
	POINTER p)
{
  	int i=0;
	while (*array != p) 
	{
	    ++array;
	    ++i;
        }
	return i;
} 		/*end index_of_pointer*/

/*
*			points_on_surface():
*
*	Counts number of unique points on surface.
*/	

EXPORT  int 	points_on_surface(
	SURFACE 	*s)
{
  	int		i, num_points;
	TRI		*tri;
	FT_POINT		*p;

	for (tri = first_tri(s); !at_end_of_tri_list(tri,s); tri = tri->next)
	{
	    Index_of_point(Point_of_tri(tri)[0]) = Index_of_point(Point_of_tri(tri)[1]) =
	        Index_of_point(Point_of_tri(tri)[2]) = ERROR;
	}
	num_points = 0;
	for (tri = first_tri(s); !at_end_of_tri_list(tri,s); tri = tri->next)
	{
	    for (i = 0; i < 3; ++i)
	    {
		p = Point_of_tri(tri)[i];
		if (Index_of_point(p) == ERROR)
	        {
		    Index_of_point(p) = num_points++;
		}
	    }
	}
	return num_points;
} 		/*end points_on_surface*/

/*
*
*			points_of_interface()
*
*	Diagnostic function, prints FT_POINT information for all points
*	in interface, in tabular form.
*
*/

EXPORT 	void 	points_of_interface(
	INTERFACE	*intfc)
{
  	FT_POINT *p;
	HYPER_SURF_ELEMENT *hse;
	HYPER_SURF *hs;
	int i = 0;
	if (intfc->dim != 3)
	    return;
	(void) printf("\nBEGIN points_of_interface() intfc = %p\n",intfc);

	next_point(intfc,NULL,NULL,NULL);
	while (next_point(intfc,&p,&hse,&hs))
	{
	    if (i % 20 == 0)
	    {
		(void) printf("\n");
		(void) printf("   pnt   pnt         "
			      "------------coords-----------     "
			      "flag   -private-      ----pointers----\n");
		(void) printf("   cnt   num    bdry    "
			      "x          y          z        "
			      "bdry   sort  int      HSE          HS \n");
		(void) printf("=================================="
			      "====================="
			      "==================    ================\n");
	    }
	    data_of_point(p,i++);
	}
	(void) printf("END points_of_interface() intfc = %p\n",intfc);
	(void) printf("\n");
	return;
}		/*end points_of_interface*/

LOCAL 	void 	data_of_point(
	FT_POINT	 		*p,
	int			i)
{
	(void) printf("%6d %llu %3d %g %g %g %5d %5s    \n",
		      i,point_number(p),Boundary(p),
		      Coords(p)[0],Coords(p)[1],Coords(p)[2],
		      Boundary_point(p),
		      y_or_n(sorted(p)));
}	       /*end data_of_point*/

EXPORT 	void 	find_blk_tri(
	BLK_TRI *blk_tri)
{
	int      i, j, ind;
	TRI      *tri;
	SURFACE  *s;

	for (j = 0; j < blk_tri->num_surfaces; j++)
        {
	    ind = blk_tri->is[j];
	    s = blk_tri->surfs[ind];
    
	    for (i = 0, tri = blk_tri->first[ind]; 
	         i < blk_tri->num_tris[ind];
	         ++i, tri = tri->next)
	    {
	        if(!the_tri(tri))
		    continue;
	        
		printf("\n#blk_tri surface %d  %p   %d %d\n",j, s, 
	            negative_component(s), positive_component(s));
		(void) printf("find_blk_tri  blk_tri = %p\n",blk_tri);
	        (void) printf("num_surfs = %d  num_tris = %p  num_null_sides = %p  "
		      "first = %p\n",
		      blk_tri->blk_info->num_surfs,blk_tri->num_tris, 
		      blk_tri->num_null_sides,blk_tri->first);
	        (void) printf("\n");
	
	        (void) printf("i = %3d\n",i);
	        print_tri(tri,s->interface);
	    }
	}
} 		/*end print_blk_tri*/


EXPORT 	void 	print_blk_tri(
	BLK_TRI *blk_tri)
{
	int      i, j, ind;
	TRI      *tri;
	SURFACE  *s;

  	(void) printf("print_blk_tri()  blk_tri = %p\n",blk_tri);
	(void) printf("num_surfs = %d  num_tris = %p  num_null_sides = %p  "
		      "first = %p\n",
		      blk_tri->blk_info->num_surfs,blk_tri->num_tris, 
		      blk_tri->num_null_sides,blk_tri->first);
	(void) printf("\n");
	
	for (j = 0; j < blk_tri->num_surfaces; j++)
        {
	    ind = blk_tri->is[j];
	    s = blk_tri->surfs[ind];
	    printf("\n#blk_tri surface %d  %p   %d %d\n",j, s, 
	        negative_component(s), positive_component(s));
	    
	    for (i = 0, tri = blk_tri->first[ind]; 
	         i < blk_tri->num_tris[ind];
	         ++i, tri = tri->next)
	    {
	        (void) printf("i = %3d\n",i);
	        print_tri(tri,s->interface);
	    }
	}
} 		/*end print_blk_tri*/

/*
*	The following program can be used to identify a triangle
*	for selective debugging. The past experience tells us that
*	it is much easy to debug when a specific triangle is identified.
*	The use of this function requires input of the coordinates
*	for the three vertices of the triangle which must be filled
*	in the commented place.
*
*/

boolean the_tri_rot(TRI *);

EXPORT boolean the_tri_rot(TRI *tri)
{
	boolean	found;
	int	i,j, k;
	double	tol = 1.0e-5;	
	
	double p[3][3] = {{   -0.02499999999999994,    -0.5196872723365785,     -7.075000000000001 },
		 	 {   0.009031057621189484,    -0.5528323463283031,     -7.055905773569994  },
			 {   -0.02906516710305997,    -0.5341372337303483,     -7.056496648997642 } };

	return NO;

	for(k=0; k < 3; k++)
	{
	    found = YES;
	    
	    for (i = 0; i < 3; i++)
	    {
		for (j = 0; j < 3; j++)
		{
		    if (fabs(Coords(Point_of_tri(tri)[i])[j] - p[(i+k)%3][j]) > tol)
		    {
			found = NO;
			break;
		    }
		}
		if(!found)   /*one point is not matching, try next rotation */
		    break;
	    }
	    if(found)
	        return YES;
	}
	return NO;
}	/* end the_tri */

boolean the_tri1(TRI *);
/*DEBUG_TMP  */
EXPORT boolean the_tri1(TRI *tri)
{
	/*DEBUG_TMP int i,j; */
	/*DEBUG_TMP double tol = 1.0e-5;	 vertices coords must have at least  */
				/* five digits after decimal points */

	/*DEBUG_TMP double p[3][3] = {{ -0.07499999999999997,    -0.4987777022673648,     -7.075000000000001  }, */
			 /*DEBUG_TMP { -0.02499999999999997,    -0.5082727175392801,     -7.125000000000001 }, */
			 /*DEBUG_TMP { -0.02499999999999997,    -0.5196872723365785,     -7.075000000000001 } }; */

	/*DEBUG_TMP return NO; */

	/*DEBUG_TMP for (i = 0; i < 3; i++) */
	/*DEBUG_TMP { */
	    /*DEBUG_TMP for (j = 0; j < 3; j++) */
	    /*DEBUG_TMP { */
	    	/*DEBUG_TMP if (fabs(Coords(Point_of_tri(tri)[i])[j] - p[i][j]) > tol) */
		/*DEBUG_TMP { */
		    /*DEBUG_TMP return NO; */
		/*DEBUG_TMP } */
	    /*DEBUG_TMP } */
	/*DEBUG_TMP } */
	/*DEBUG_TMP return YES; */
}

EXPORT boolean the_tri(TRI *tri)
{
	int i,j;
	double tol = 1.0e-5;	/* vertices coords must have at least */
				/* five digits after decimal points */

	double p[3][3] = {{0.50416,0.0,0.0},
			  {0.49583,0.00416,0.0},
			  {0.49583,0.0,0.0}};

	/*return NO; */

	for (i = 0; i < 3; i++)
	{
	    for (j = 0; j < 3; j++)
	    {
	    	if (fabs(Coords(Point_of_tri(tri)[i])[j] - p[i][j]) > tol)
		{
		    return NO;
		}
	    }
	}
	return YES;
}	/* end the_tri */

LOCAL boolean check_pt(double *p1, 
		    double *p2) 
{
	int	i;
	double	tol = 2.0e-3;

	for(i=0; i<3; i++)
	    if(fabs(p1[i]-p2[i]) > tol)
	        return NO;
	return YES;
}

EXPORT boolean the_side(TRI  *tri)
{
	FT_POINT	**p = Point_of_tri(tri);
	int	i;
	double	p1[3] = { 0.021,     0.019911,  0.163  };
	double	p2[3] = { 0.0215992, 0.0189758, 0.161933 };

	return NO;

	for(i=0; i<3; i++)
	{
	    if( (check_pt(Coords(p[i]), p1) && check_pt(Coords(p[Next_m3(i)]), p2))  ||
	        (check_pt(Coords(p[i]), p2) && check_pt(Coords(p[Next_m3(i)]), p1)) )
		return YES;
	}
	return NO;
}

/* 	This function is to catch the triangle for */
/* 	the detection using the function the_tri() */

EXPORT void print_tri_coords(TRI* tri)
{
	FT_POINT *p;
	int i,j;
	for (i = 0; i < 3; i++)
	{
	    p = Point_of_tri(tri)[i];
	    printf("%f %f %f\n",Coords(p)[0],Coords(p)[1],Coords(p)[2]);
	    /*print_general_vector("p= ", Coords(p), 3, "\n"); */
	}
}	/* end print_tri_coords */

EXPORT	boolean check_tri_and_neighbor(TRI *tri)
{
	int i,j;
	TRI *nbtri;
	boolean status = YES;
	for (i = 0; i < 3; i++)
	{
	    if (is_side_bdry(tri,i))
	    	continue;
	    nbtri = Tri_on_side(tri,i);
	    if (nbtri != NULL)
	    {
	    	for (j = 0; j < 3; j++)
		{
		    if (is_side_bdry(nbtri,j))
		    	continue;
		    if (Tri_on_side(nbtri,j) == tri)
		    {
		    	if (Point_of_tri(tri)[i] != 
			    Point_of_tri(nbtri)[Next_m3(j)] ||
		    	    Point_of_tri(tri)[Next_m3(i)] != 
			    Point_of_tri(nbtri)[j])
			{
			    printf("Inconsistency on tri side %d: "
			    	"ps = %p  pe = %p\n",i,
				Point_of_tri(tri)[i],
				Point_of_tri(tri)[Next_m3(i)]);
			    printf("Inconsistency on nbtri side %d: "
			    	"ps = %p  pe = %p\n",j,
				Point_of_tri(nbtri)[Next_m3(j)],
				Point_of_tri(nbtri)[j]);
			    status = NO;
			}
		    }
		    break;
		}
		if (j == 3)
		{
		    printf("The %d-th neighbor is not linked\n",i);
		}
	    }
	}
	return status;
}	/* end check_tri_and_neighbor */

/*
*	The following program can be used to identify a bond
*	for selective debugging. The past experience tells us that
*	it is much easy to debug when a specific bond is identified.
*	The use of this function requires input of the coordinates
*	for the two end points of the bond which must be filled
*	in the commented place.
*
*/

EXPORT boolean the_bond(BOND *b)
{
	int i;
	double tol = 1.0e-5;	/* vertices coords must have at least */
				/* five digits after decimal points */

	double p[2][3] = {{0,1,0.491666666666666641},	/* Place holder for */
			 {0,1,0.508333333333333304}};	/* coords of end points */
	
	for (i = 0; i < 3; i++)
	{
	    if (fabs(Coords(b->start)[i] - p[0][i]) > tol)
			return NO;
	}
	for (i = 0; i < 3; i++)
	{
	    if (fabs(Coords(b->end)[i] - p[1][i]) > tol)
			return NO;
	}
	return YES;
}	/* end the_bond */

/*
*	The following program can be used to identify a 3D point
*	for selective debugging. The past experience tells us that
*	it is much easy to debug when a specific point is identified.
*	The use of this function requires input of the coordinates
*	for the point. 
*
*/
LOCAL boolean the_point_one(FT_POINT *pt, double *p)
{
	int i;
	double tol = 0.01;	/* vertices coords must have at least */
				/* five digits after decimal points */
	int dim = current_interface()->dim;
	
	for (i = 0; i < dim; i++)
	{
	    if (fabs(Coords(pt)[i] - p[i]) > tol)
			return NO;
	}
	return YES;
}

EXPORT boolean the_point(FT_POINT *pt)
{
	double	p1[3] = {0.025000, 0.050025, 0.0};
	double	p2[3] = {7.116667, 11.783334, 17.999809};

	if(the_point_one(pt,p1) || the_point_one(pt,p2))
	    return YES;
	return NO;
}

LOCAL   boolean the_pt_one(double *pt, double *p)
{
	int i;
	double tol = 1.0e-4;	/* vertices coords must have at least */
				/* five digits after decimal points */
	
	int dim = current_interface()->dim;
	
	for (i = 0; i < dim; i++)
	{
	    if (fabs(pt[i] - p[i]) > tol)
			return NO;
	}
	return YES;
}

EXPORT  boolean  the_pt(double *pt)
{
	double	p1[3] = {0.512068, 3.906350, 34.733800};
	double	p2[3] = {0.513022, 3.884300, 34.735400};
	
	if(the_pt_one(pt,p1) || the_pt_one(pt,p2))
	    return YES;
	return NO;
}

EXPORT boolean search_the_tri_in_intfc(INTERFACE *intfc)
{
	boolean the_tri_found = NO;
        SURFACE **s;
        for (s = intfc->surfaces; s && *s; ++s)
        {
            the_tri_found = search_the_tri_in_surf(*s);
        }
	if (the_tri_found)
	{
	    (void) printf("The tri is in the interface %d\n",intfc);
	    return YES;
	}
	return NO;
}

EXPORT boolean search_the_tri_in_surf(SURFACE *s)
{
        TRI *tri;
	boolean the_tri_found = NO;

        for (tri = first_tri(s); !at_end_of_tri_list(tri,s); tri = tri->next)
        {
            if (the_tri(tri))
            {
                (void) printf("The tri found on surface %d\n",s);
		the_tri_found = YES;
            }
        }
	return the_tri_found;
}
