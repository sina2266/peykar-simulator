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
*                                   map.c
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*       This file contains functions that convert topological structure
*       into formats that can be used as input of various programs for
*       generating 3D graphical images of the interface structures.
*
*       is_interior_vertex();
*	ArrayOfCurvePoints(curve, coords);	
*	ArrayOfIntfcPoints(intfc, coords);		
* 	
*	ArrayOfCurves(intfc, curves);
*	ArrayOfCurves(intfc, coords, vertex_indices);
*
*	ArrayOfSurfaces(intfc, surfaces);
*
*	ArrayOfSurfTris_FT(surface, tris);		
*	ArrayOfSurfTris(surface, coords, vertex_indices);	
*	ArrayOfIntfcTri_FT(intfc, tris);			
*	ArrayOfIntfcTri(intfc, coords, vertex_indices);	
*
*
*       TODO:
*
*
* Questions:
*       Boundary(c);  	 testing boundary curve? 
*       Boundary(s);	 testing boundary surface? 
*/

#include <intfc/iloc.h>

LOCAL   int crossings_in_direction1d(CRXING**,int*,GRID_DIRECTION,INTERFACE*);
LOCAL   int crossings_in_direction2d(CRXING**,int*,GRID_DIRECTION,INTERFACE*);
LOCAL   int crossings_in_direction3d(CRXING**,int*,GRID_DIRECTION,INTERFACE*);

/*
*                       is_interior_vertex():
*
*       Given a point and a tri, this function tests whether
*       the point p is interior (not on a curve) to the
*       surface by testing if there is a set of tris
*       that surrounds p and yet each bond that has p
*       as a end is not on the boundary. *t_begin is set
*       to the tri that last was tested.
*/

LIB_LOCAL boolean is_interior_vertex(
	TRI       *t,
	FT_POINT     *p,
	TRI       **t_begin,
	INTERFACE *intfc)
{
	TRI		*tri = t;
	int		i, j;
	static int	max_loops = 100; /*TOLERANCE*/

	if (!Boundary_point(p))
	{
	    *t_begin = tri;
	    return YES;
	}
	for (j = 0, tri = t; j < max_loops && tri != NULL; j++)
	{
	    for (i = 0; i < 3; i++)
	    {
	    	if (p == Point_of_tri(tri)[i])
	    	{
	    	    if (is_side_bdry(tri,i))
	    	    {
	    	    	*t_begin = tri;
	    	    	return NO;
	    	    }
	    	    *t_begin = tri;
	    	    tri = Tri_on_side(tri,i);
	    	    if (tri == t)
			return YES;
	    	    break;
	    	}
	    }

	    if (i == 3)
	    {
	       (void) printf("ERROR in is_interior_vertex(), ");
	       (void) printf("point (%llu) = (%g %g %g) is not on tri (%llu)\n",
	    	             point_number(p),
	    	             Coords(p)[0], Coords(p)[1], Coords(p)[2],
	    	             tri_number(tri,current_interface()));
	       print_tri(tri,intfc);
	       clean_up(ERROR);
	       return NO;
	    }
	}
	(void) printf("ERROR in is_interior_vertex()\n"
	              "Probably infinite loop at vertex point ");
	(void) printf("(%llu) = (%g %g %g)\n", point_number(p),
	       Coords(p)[0], Coords(p)[1], Coords(p)[2]);
	clean_up(ERROR);
	return NO;
}		/*end is_interior_vertex*/

/***************************************************************************
 * 	ArrayOfCurvePoints()
 * purpose: 
 * 	return the coords of all points in the given CURVE *c.
 * input: c, coords;
 * output: coords;
 * 	The coordinates of all points in the CURVE *c are stored as a one dimensional
 * 	array. For example, suppose there are three points in the curve with
 * 	the following coordinates (0,1), (2,3), (4,5), (6,7). Then after calling 
 * 	this function, coords is a 1D array containing the following numbers:
 * 	{0,1,2,3,4,5,6,7}.
 *	The size of coords is NumOfCurvePoints(c)*dim; 
 ****************************************************************************/
EXPORT	void ArrayOfCurvePoints(
	CURVE *c,
	double *coords)
{
	int i,dim = c->interface->dim;
	
	BOND *b;
	int n = 0;
	
	for (b = c->first; b != NULL; b = b->next)
	{
	    for (i = 0; i < dim; ++i)
	    	coords[n++] = Coords(b->start)[i];		
	}
	for (i = 0; i < dim; ++i)
	    coords[n++] = Coords(c->last->end)[i];
	    
}	/* end ArrayOfCurvePoints */

/**********************************************************************
 *	ArrayOfIntfcPoints()
 * Return the coords of the points in the INTERFACE *intfc.
 **********************************************************************/
EXPORT	void ArrayOfIntfcPoints(INTERFACE *intfc, double *coords)
{
	int i, n, dim = intfc->dim;
	FT_POINT	*p;
	HYPER_SURF_ELEMENT *phse;
	HYPER_SURF	   *phs;
	
	n=0;
        next_point(intfc,NULL, NULL,NULL);
	for ((void)next_point(intfc, &p, &phse, &phs); p!= NULL; 
	     (void)next_point(intfc, &p, &phse, &phs))
	{
	    /*Index_of_point(p) = n/dim; */
	    for (i = 0; i < dim; ++i)
	    	coords[n++] = Coords(p)[i];
	}
}

EXPORT	void ArrayOfSurfaces(
	INTERFACE *intfc,
	SURFACE **surfaces)
{
	SURFACE **s;
	int n = 0;
	for (s = intfc->surfaces; s && *s; ++s)
	    surfaces[n++] = *s;	    
}	/* end ArrayOfCurves */


/**********************************************************************
 *	ArrayOfSurfTris_FT()
 * return the triangles as an array in a given SURFACE *surface.
 **********************************************************************/	

EXPORT	void ArrayOfSurfTris_FT(SURFACE *surface, TRI **tris)
{
	int n;
	TRI *tri;
	
	n = 0;
	for(tri = first_tri(surface) ; !at_end_of_tri_list(tri,surface); 
			tri = tri->next)
	{
	    tris[n++] = tri;
	}    
}	/* end ArrayOfSurfTris_FT */

/**********************************************************************
 *	ArrayOfSurfTris()
 * return the coords and vertex indices of the triangles for a given 
 * SURFAC *surface.
 *
 * the dim of coords 		is NumOfSurfPoints(surface)*dim;
 * the dim of vertices_index 	is NumOfSurfTris(surface)*3;
 **********************************************************************/

EXPORT	void ArrayOfSurfTris(
	SURFACE *surface, 
	double *coords, 
	int *vertices_index)
{
	int ic, n, i, j;
	FT_POINT *p;
	
	TRI *tri;
	
	/* reset point index */
	for(tri = first_tri(surface); !at_end_of_tri_list(tri,surface); 
			tri = tri->next)
	{
	    for(i = 0; i < 3; i++)
	    {
	        p = Point_of_tri(tri)[i];
		Index_of_point(p) = -1;		
	    }
	}		
	
	/* points	 */
	ic = 0;		n = 0;
	for(tri = first_tri(surface); !at_end_of_tri_list(tri,surface); 
			tri = tri->next)
	{
	    for(i = 0; i < 3; i++)
	    {
	        p = Point_of_tri(tri)[i];
		if (Index_of_point(p) == -1)
		{
		    Index_of_point(p) = n++;		
		    for (j = 0; j < 3; ++j)
	    	        coords[ic++] = Coords(p)[j];	
		}
	    }
	}		
	
	/* tris */
	n = 0;
	for(tri = first_tri(surface); !at_end_of_tri_list(tri,surface); 
			tri = tri->next)
	{
	    for(i = 0; i < 3; i++)
	    {
	        p = Point_of_tri(tri)[i];
		vertices_index[n++] = Index_of_point(p);			
	    }
	}		
}

/**********************************************************************
 * 	ArrayOfIntfcTris_FT(intfc, tris)
 * return the triangles array for a given INTERFACE *intfc.
 **********************************************************************/

EXPORT	void ArrayOfIntfcTris_FT(
	INTERFACE *intfc, 
	TRI **tris)
{

	int n;
	TRI *tri;	
	SURFACE **s;
	
	n = 0;		
	for (s = intfc->surfaces; s && *s; ++s)	
	{ 
	    for(tri = first_tri(*s); !at_end_of_tri_list(tri,*s); 
	    		tri = tri->next)
	    {
	        tris[n++] = tri;
	    }
	} 
}	/* end ArrayOfIntfcTris_FT */

/**********************************************************************
 *	ArrayOfTri(intfc, coords, vertex_indices)
 * return the coords and vertex indices of the triangles for a given 
 * SURFAC *surface.
 *
 * the dim of coords 		is NumOfIntfcPoints(intfc)*dim;
 * the dim of vertex_indices 	is NumOfIntfcTris(intfc)*3;
 **********************************************************************/
EXPORT  void ArrayOfIntfcTris(
	INTERFACE *intfc, 
	double *coords, 
	int *vertex_indices)
{
	TRI *tri;
	SURFACE **s;
	FT_POINT *p;
	HYPER_SURF_ELEMENT *hse;
	HYPER_SURF *hs;
	int i,n,ic;
	
	for (s = intfc->surfaces; s && *s; ++s)
	{
	    for(tri = first_tri(*s); !at_end_of_tri_list(tri,*s); 
	    		tri = tri->next)
	    {
	        for(i = 0; i < 3; i++)
	        {
		    p = Point_of_tri(tri)[i]; 
		    Index_of_point(p) = -1;
		}
	    }
	}
	n = 0;		ic = 0;
	for (s = intfc->surfaces; s && *s; ++s)
	{
	    for(tri = first_tri(*s); !at_end_of_tri_list(tri,*s); 
	    		tri = tri->next)
	    {
	        for(i = 0; i < 3; i++)
	        {
		    p = Point_of_tri(tri)[i]; 
		    if (Index_of_point(p) == -1)
		    {
		    	Index_of_point(p) = n++;
			coords[ic++] = Coords(p)[0];
			coords[ic++] = Coords(p)[1];
			coords[ic++] = Coords(p)[2];
		    }
		}
	    }
	}
	n = 0;
	for (s = intfc->surfaces; s && *s; ++s)
	{
	    for(tri = first_tri(*s); !at_end_of_tri_list(tri,*s); 
	    		tri = tri->next)
	    {
	        for(i = 0; i < 3; i++)
	        {
	            p = Point_of_tri(tri)[i];
		    vertex_indices[n++] = Index_of_point(p);
	        }
	    }
	}
}


/**********************************************************************
*   This call return number of crossings of grid_intfc on gridline    *
*   segment at icoords in direction dir. The underlying rectangular   *
*   grid is the topological_grid(grid_intfc).                         *
**********************************************************************/

#define		MAX_NUM_CRXINGS_ON_SEG		10

EXPORT	int GridSegCrossing(
	CRXING    	**crx_list,
	int 		*icoords,
	GRID_DIRECTION	dir,
	INTERFACE 	*grid_intfc)
{
	int dim = Dimension(grid_intfc);
	int 	  ncross;
	switch (dim)
	{
        case 1:
            ncross = crossings_in_direction1d(crx_list,icoords,dir,grid_intfc);
            break;
        case 2:
            ncross = crossings_in_direction2d(crx_list,icoords,dir,grid_intfc);
            break;
        case 3:
            ncross = crossings_in_direction3d(crx_list,icoords,dir,grid_intfc);
            break;
	}
	return ncross;
}	/* end GridSegCrossing */


LOCAL	int crossings_in_direction1d(
	CRXING		**crx_list,
	int		*icoords,
	GRID_DIRECTION	dir,
	INTERFACE	*grid_intfc)
{
	RECT_GRID *rgr = &topological_grid(grid_intfc);
	int xmax;
	int nix = icoords[0];
	int i, j, k, nc = 0;
	int *start;
	Table *T = table_of_interface(grid_intfc);

	if (grid_intfc == NULL)
	{
	    crx_list[nc] = NULL;
	    return nc;
	}
	xmax = rgr->gmax[0];

	if ((nix < 0) || (nix > xmax))
	{
	    crx_list[nc] = NULL;
	    return nc;
	}

	switch (dir)
	{
	case WEST:
	    if (nix != 0)
	    {
	    	k  = nix-1;
		start = T->seg_crx_lists[k];
	    	nc = T->seg_crx_count[k];
	    	for (i = 0, j = nc-1; i < nc; ++i, --j)
	    	    crx_list[i] = T->crx_store + start[j];
	    }
	    break;

	case EAST:
	    if (nix != xmax)
	    {
	    	k  = nix;
		start = T->seg_crx_lists[k];
	    	nc = T->seg_crx_count[k];
	    	for (i = 0; i < nc; ++i)
	    	    crx_list[i] = T->crx_store + start[i];
	    }
	    break;
	}
	crx_list[nc] = NULL;
	return nc;
}		/*end crossings_in_direction1d*/


LOCAL	int crossings_in_direction2d(
	CRXING		**crx_list,
	int		*icoords,
	GRID_DIRECTION	dir,
	INTERFACE	*grid_intfc)
{
	RECT_GRID	*rgr = &topological_grid(grid_intfc);
	int		i, j, k, nc = 0;
	int		list;
	Table		*T = table_of_interface(grid_intfc);
	int		*gmax = rgr->gmax;

	if (grid_intfc == NULL)
	{
	    crx_list[nc] = NULL;
	    return nc;
	}

	k = seg_index2d(icoords[0],icoords[1],dir,gmax);
	if (k < 0) return nc;
	nc = T->seg_crx_count[k];
	if (nc > MAX_NUM_CRX)
	{
	    screen("ERROR: In crossings_in_direction2d(), nc = %d\n",nc);
	    screen("Find out why!\n");
	    clean_up(ERROR);
	}

	switch (dir)
	{
	case SOUTH:
	case WEST:
	    for (i = 0, j = nc-1; i < nc; ++i, --j)
	    {
	    	list = T->seg_crx_lists[k][j];
	    	crx_list[i] = T->crx_store + list;
	    }
	    break;

	case NORTH:
	case EAST:
	    for (i = 0; i < nc; ++i)
	    {
	    	list = T->seg_crx_lists[k][i];
	    	crx_list[i] = T->crx_store + list;
	    }
	    break;
	}
	crx_list[nc] = NULL;
	return nc;
}		/*end crossings_in_direction2d*/

LOCAL	int crossings_in_direction3d(
	CRXING		**crx_list,
	int		*icoords,
	GRID_DIRECTION	dir,
	INTERFACE	*grid_intfc)
{
	RECT_GRID	*rgr = &topological_grid(grid_intfc);
	int		i, j, k, nc = 0;
	int		xmax,ymax,zmax;
	int		list;
	Table		*T = table_of_interface(grid_intfc);
	int		*gmax = rgr->gmax;

	if (grid_intfc == NULL)
	{
	    crx_list[nc] = NULL;
	    return nc;
	}

	xmax = rgr->gmax[0];
	ymax = rgr->gmax[1];
	zmax = rgr->gmax[2];

	if ((icoords[0] < 0) || (icoords[0] == 0 && dir == WEST) ||
	    (icoords[1] < 0) || (icoords[1] == 0 && dir == SOUTH) ||
	    (icoords[2] < 0) || (icoords[2] == 0 && dir == LOWER) ||
	    (icoords[0] > xmax) || (icoords[0] == xmax && dir == EAST) ||
	    (icoords[1] > ymax) || (icoords[1] == ymax && dir == NORTH) ||
	    (icoords[2] > zmax) || (icoords[2] == zmax && dir == UPPER))
	{
	    crx_list[nc] = NULL;
	    return nc;
	}

	k = seg_index3d(icoords[0],icoords[1],icoords[2],dir,gmax);
	nc = T->seg_crx_count[k];
	if (nc > MAX_NUM_CRX)
	{
	    screen("ERROR: In crossings_in_direction3d(), nc = %d\n",nc);
	    screen("Find out why!\n");
	    clean_up(ERROR);
	}

	if (nc == 0)
	{
	    crx_list[nc] = NULL;
	    return nc;
	}

	switch (dir)
	{
	case WEST:
	case SOUTH:
	case LOWER:
	    for (i = 0, j = nc-1; i < nc; ++i, --j)
	    {
	    	list = T->seg_crx_lists[k][j];
	    	crx_list[i] = T->crx_store + list;
	    }
	    break;
	case EAST:
	case NORTH:
	case UPPER:
	    for (i = 0; i < nc; ++i)
	    {
	    	list = T->seg_crx_lists[k][i];
	    	crx_list[i] = T->crx_store + list;
	    }
	    break;
	}
	crx_list[nc] = NULL;
	return nc;
}		/*end crossings_in_direction3d*/

EXPORT	int NumOfInteriorPoints(
        INTERFACE *intfc)
{
        CURVE **c;
        int num_points = 0;
        for (c = intfc->curves; c && *c; ++c)
        {
            if (Boundary_hs(Hyper_surf(*c)))
                continue;
            num_points += (*c)->num_points;
        }
        return num_points;
}       /* end NumOfInteriorPoints */

EXPORT	COMPONENT *GridIntfcComp(
	INTERFACE *grid_intfc)
{
	struct Table    *T;
        T = table_of_interface(grid_intfc);
        if (T != NULL)
            return T->components;
        else
            return NULL;
}	/* GridIntfcComp */

/**************************************************************************
*	This function returns a chain of points centered at point pc.     *
*	It returns NO if the curve does not have enough number of points  *
*	If the curve is not a closed curve, the chain of points is biased *
*	when the number of point on either side of pc is less than np/2   *
**************************************************************************/

EXPORT	boolean IntfcGetPointChain(
	FT_POINT *pc,		/* center of the chain */
	FT_POINT **pts,		/* memory already allocated */
	int num_pts)
{
	CURVE *c = Curve_of_hs(pc->hs);
	BOND *b = Bond_of_hse(pc->hse);
	BOND *b_current,*btmp;
	int i,iprev,inext;

	if (c->num_points < num_pts) return NO;

	inext = 0;
	b_current = (pc == b->end) ? Next_bond(b,c) : b;
	while (b_current) 
	{
	    inext++;
	    if (inext == num_pts/2) break;
	    b_current = Next_bond(b_current,c);
	}

	b_current = (pc == b->end) ? b : Prev_bond(b,c);
	iprev = 0;
	while(b_current)	
	{
	    iprev++;
	    if (iprev == num_pts-inext-1) break;
	    btmp = Prev_bond(b_current,c);
	    if (btmp == NULL) break;
	    b_current = btmp;
	}
	if (iprev == 0) b_current = b;
	for (i = 0; i < num_pts && b_current != NULL; 
		++i, b_current = Next_bond(b_current,c))
	{
	    pts[i] = b_current->start;
	    if (i < num_pts-1) pts[i+1] = b_current->end;
	}

	return YES;
}	/* end FrontGetPointChain */
