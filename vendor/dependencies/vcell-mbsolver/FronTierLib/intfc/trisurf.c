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
*                                trisurf.c
*
*        Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*        Performs a full resolution triangulation of an arbitrary
*        plane surface in three dimensions.
*
*        TODO:  merge this file with corresponding trigrid code.
*/

#if defined(THREED)
#include <intfc/iloc.h>

/* LOCAL Function Declarations */
LOCAL   CURVE   *id_curve_by_points(SURFACE*,FT_POINT*,FT_POINT*,ORIENTATION*);
LOCAL   ORIENTATION     is_point_a_node_of_curve(CURVE*,FT_POINT*);
LOCAL   double   value_x(double*,FT_POINT*,double*);
LOCAL   double   value_y(double*,FT_POINT*,double*);
LOCAL   double   value_z(double*,FT_POINT*,double*);
LOCAL   void    set_curve_bdry_flag(CURVE*,RECT_GRID*);
LOCAL   void    normal_of_plane_surface(SURFACE*,double*);
LOCAL   void    strip_curves_in_surface(SURFACE*);
LOCAL   void    right_planar_surface_triangulation(SURFACE*,RECT_GRID*);

#define Next_m4(n)              (((n) + 1) % 4)
#define Prev_m4(n)              (((n) + 3) % 4)

EXPORT  void  planar_surface_triangulation(
	SURFACE      *surf,
	RECT_GRID    *grid,
	const boolean expand_buffer_zones)
{
	double        coords[MAXD];
	double        x1,x2,x3,x4,y1,y2,y3,y4,z1,z2,z3,z4,x,y,z;
	CURVE        **c1;
	FT_POINT        *p1,*p2;
	FT_POINT        *pmin,*pmel,*pmeu,*pmax;
	boolean      hole_flag = NO;
	RECT_GRID    *gr;

	if (expand_buffer_zones == YES)
	{
	    static RECT_GRID rgr;
	    int              i,gmax[3],lbuf[3],ubuf[3];

	    gr = &rgr;
	    for (i = 0; i < 3; i++)
	    {
	        gmax[i] = grid->gmax[i];
	        if (grid->lbuf[i] != 0)
	        {
	            lbuf[i] = grid->lbuf[i] + 1;
	            gmax[i]++;
	        }
	        if (grid->ubuf[i] != 0)
	        {
	            ubuf[i] = grid->ubuf[i] + 1;
	            gmax[i]++;
	        }
	    }
	    set_rect_grid(grid->L,grid->U,grid->GL,grid->GU,lbuf,ubuf,gmax,
	                  grid->dim,&grid->Remap,gr);
	}
	else
	{
	    gr = grid;
	}

	/* Suppose the surface is a planar surf with a rectangle hole in it 
	 * We first need to know the lower and the upper points (x1,y1,z1) 
	 * and (x4,y4,z4). In this version, the curves bounding the surface
	 * have no interior points */

	x1 =  HUGE_VAL;
	y1 =  HUGE_VAL;
	z1 =  HUGE_VAL;
	x4 = -HUGE_VAL;
	y4 = -HUGE_VAL;
	z4 = -HUGE_VAL;

	for (c1 = surf->pos_curves; c1 && *c1; c1++)
	{
	    p1 = (*c1)->start->posn;
	    p2 = (*c1)->end->posn;
	    x = Coords(p1)[0];
	    y = Coords(p1)[1];
	    z = Coords(p1)[2];

	    x1 = (x1 < x) ? x1 : x;
	    x4 = (x4 > x) ? x4 : x;
	    y1 = (y1 < y) ? y1 : y;
	    y4 = (y4 > y) ? y4 : y;
	    z1 = (z1 < z) ? z1 : z;
	    z4 = (z4 > z) ? z4 : z;

	    x = Coords(p2)[0];
	    y = Coords(p2)[1];
	    z = Coords(p2)[2];

	    x1 = (x1 < x) ? x1 : x;
	    x4 = (x4 > x) ? x4 : x;
	    y1 = (y1 < y) ? y1 : y;
	    y4 = (y4 > y) ? y4 : y;
	    z1 = (z1 < z) ? z1 : z;
	    z4 = (z4 > z) ? z4 : z;
	}
	for (c1 = surf->neg_curves; c1 && *c1; c1++)
	{
	    p1 = (*c1)->start->posn;
	    p2 = (*c1)->end->posn;
	    x = Coords(p1)[0];
	    y = Coords(p1)[1];
	    z = Coords(p1)[2];

	    x1 = (x1 < x) ? x1 : x;
	    x4 = (x4 > x) ? x4 : x;
	    y1 = (y1 < y) ? y1 : y;
	    y4 = (y4 > y) ? y4 : y;
	    z1 = (z1 < z) ? z1 : z;
	    z4 = (z4 > z) ? z4 : z;

	    x = Coords(p2)[0];
	    y = Coords(p2)[1];
	    z = Coords(p2)[2];

	    x1 = (x1 < x) ? x1 : x;
	    x4 = (x4 > x) ? x4 : x;
	    y1 = (y1 < y) ? y1 : y;
	    y4 = (y4 > y) ? y4 : y;
	    z1 = (z1 < z) ? z1 : z;
	    z4 = (z4 > z) ? z4 : z;
	}

	/* Now find the middle points (x2,y2,z2) and (x3,y3,z3) 
	 * if we have a hole */

	x2 =  HUGE_VAL;
	y2 =  HUGE_VAL;
	z2 =  HUGE_VAL;
	x3 = -HUGE_VAL;
	y3 = -HUGE_VAL;
	z3 = -HUGE_VAL;

	if (z1 == z4)
	{
	    /*  xy-plane */

	    for (c1 = surf->pos_curves;c1 && *c1;c1++)
	    {
	        p1 = (*c1)->start->posn;
	        p2 = (*c1)->end->posn;

	        x = Coords(p1)[0];
	        y = Coords(p1)[1];
	        z = Coords(p1)[2];

	        if ((x1 < x) && (x < x4))
	        {
	            x2 = (x2 < x) ? x2 : x;
	            x3 = (x3 > x) ? x3 : x;
	            y2 = (y2 < y) ? y2 : y;
	            y3 = (y3 > y) ? y3 : y;
	        }
	        x = Coords(p2)[0];
	        y = Coords(p2)[1];
	        z = Coords(p2)[2];
	        if ((x1 < x) && (x < x4))
	        {
	            x2 = (x2 < x) ? x2 : x;
	            x3 = (x3 > x) ? x3 : x;
	            y2 = (y2 < y) ? y2 : y;
	            y3 = (y3 > y) ? y3 : y;
	        }
	    }
	    for (c1 = surf->neg_curves;c1 && *c1;c1++)
	    {
	        p1 = (*c1)->start->posn;
	        p2 = (*c1)->end->posn;

	        x = Coords(p1)[0];
	        y = Coords(p1)[1];
	        z = Coords(p1)[2];

	        if ((x1 < x) && (x < x4))
	        {
	            x2 = (x2 < x) ? x2 : x;
	            x3 = (x3 > x) ? x3 : x;
	            y2 = (y2 < y) ? y2 : y;
	            y3 = (y3 > y) ? y3 : y;
	        }
	        x = Coords(p2)[0];
	        y = Coords(p2)[1];
	        z = Coords(p2)[2];
	        if ((x1 < x) && (x < x4))
	        {
	            x2 = (x2 < x) ? x2 : x;
	            x3 = (x3 > x) ? x3 : x;
	            y2 = (y2 < y) ? y2 : y;
	            y3 = (y3 > y) ? y3 : y;
	        }
	    }
	    z2 = z1;
	    z3 = z1;
	}

	if (y1 == y4)
	{
	    /*  xz-plane */

	    for (c1 = surf->pos_curves; c1 && *c1;c1++)
	    {
	        p1 = (*c1)->start->posn;
	        p2 = (*c1)->end->posn;

	        x = Coords(p1)[0];
	        y = Coords(p1)[1];
	        z = Coords(p1)[2];

	        if ((x1 < x) && (x < x4))
	        {
	            x2 = (x2 < x) ? x2 : x;
	            x3 = (x3 > x) ? x3 : x;
	            z2 = (z2 < z) ? z2 : z;
	            z3 = (z3 > z) ? z3 : z;
	        }
	        x = Coords(p2)[0];
	        y = Coords(p2)[1];
	        z = Coords(p2)[2];
	        if ((x1 < x) && (x < x4))
	        {
	            x2 = (x2 < x) ? x2 : x;
	            x3 = (x3 > x) ? x3 : x;
	            z2 = (z2 < z) ? z2 : z;
	            z3 = (z3 > z) ? z3 : z;
	        }
	    }
	    for (c1 = surf->neg_curves; c1 && *c1; c1++)
	    {
	        p1 = (*c1)->start->posn;
	        p2 = (*c1)->end->posn;

	        x = Coords(p1)[0];
	        y = Coords(p1)[1];
	        z = Coords(p1)[2];

	        if ((x1 < x) && (x < x4))
	        {
	            x2 = (x2 < x) ? x2 : x;
	            x3 = (x3 > x) ? x3 : x;
	            z2 = (z2 < z) ? z2 : z;
	            z3 = (z3 > z) ? z3 : z;
	        }
	        x = Coords(p2)[0];
	        y = Coords(p2)[1];
	        z = Coords(p2)[2];
	        if ((x1 < x) && (x < x4))
	        {
	            x2 = (x2 < x) ? x2 : x;
	            x3 = (x3 > x) ? x3 : x;
	            z2 = (z2 < z) ? z2 : z;
	            z3 = (z3 > z) ? z3 : z;
	        }
	    }
	    y2 = y1;
	    y3 = y1;
	}

	if (x1 == x4)
	{
	    /*  yz-plane */

	    for (c1 = surf->pos_curves; c1 && *c1; c1++)
	    {
	        p1 = (*c1)->start->posn;
	        p2 = (*c1)->end->posn;

	        x = Coords(p1)[0];
	        y = Coords(p1)[1];
	        z = Coords(p1)[2];

	        if ((y1 < y) && (y < y4))
	        {
	            y2 = (y2 < y) ? y2 : y;
	            y3 = (y3 > y) ? y3 : y;
	            z2 = (z2 < z) ? z2 : z;
	            z3 = (z3 > z) ? z3 : z;
	        }
	        x = Coords(p2)[0];
	        y = Coords(p2)[1];
	        z = Coords(p2)[2];
	        if ((y1 < y) && (y < y4))
	        {
	            y2 = (y2 < y) ? y2 : y;
	            y3 = (y3 > y) ? y3 : y;
	            z2 = (z2 < z) ? z2 : z;
	            z3 = (z3 > z) ? z3 : z;
	        }
	    }
	    for (c1 = surf->neg_curves; c1 && *c1; c1++)
	    {
	        p1 = (*c1)->start->posn;
	        p2 = (*c1)->end->posn;

	        x = Coords(p1)[0];
	        y = Coords(p1)[1];
	        z = Coords(p1)[2];

	        if ((y1 < y) && (y < y4))
	        {
	            y2 = (y2 < y) ? y2 : y;
	            y3 = (y3 > y) ? y3 : y;
	            z2 = (z2 < z) ? z2 : z;
	            z3 = (z3 > z) ? z3 : z;
	        }
	        x = Coords(p2)[0];
	        y = Coords(p2)[1];
	        z = Coords(p2)[2];
	        if ((y1 < y) && (y < y4))
	        {
	            y2 = (y2 < y) ? y2 : y;
	            y3 = (y3 > y) ? y3 : y;
	            z2 = (z2 < z) ? z2 : z;
	            z3 = (z3 > z) ? z3 : z;
	        }
	    }
	    x2 = x1;
	    x3 = x1;
	}
	/* end of finding (x_i,y_i,z_i), i = 1,2,3,4 */

	if ((x2 < x3) || (y2 < y3) || (z2 < z3))
	    hole_flag = YES;

	if (debugging("read_interface"))
	{
	    (void) printf("hole_flag = %s\n",(hole_flag==YES)?"YES":"NO");
	    (void) printf("(x1,y1,z1)=(%g,%g,%g),\n",x1,y1,z1);
	    (void) printf("(x2,y2,z2)=(%g,%g,%g),\n",x2,y2,z2);
	    (void) printf("(x3,y3,z3)=(%g,%g,%g),\n",x3,y3,z3);
	    (void) printf("(x4,y4,z4)=(%g,%g,%g),\n",x4,y4,z4);
	}

	coords[0] = x1;
	coords[1] = y1;
	coords[2] = z1;

	p1 = Point(coords);

	coords[0] = x4;
	coords[1] = y4;
	coords[2] = z4;

	p2 = Point(coords);

	/* NOTE oblique plane is not allowed in this version */

	coords[0] = x1;
	coords[1] = y1;
	coords[2] = z1;
	pmin = Point(coords);

	coords[0] = x2;
	coords[1] = y2;
	coords[2] = z2;
	pmel = Point(coords);

	coords[0] = x3;
	coords[1] = y3;
	coords[2] = z3;
	pmeu = Point(coords);

	coords[0] = x4;
	coords[1] = y4;
	coords[2] = z4;
	pmax = Point(coords);

	if (hole_flag == YES)
	    planar_hole_surface_triangulation(surf,gr,pmin,pmel,pmeu,pmax);
	else
	    right_planar_surface_triangulation(surf,gr);
}                /*end planar_surface_triangulation*/

LOCAL void right_planar_surface_triangulation(
	SURFACE *surf,
	RECT_GRID *gr)
{
	BOND      *b;
	CURVE     *curve;
	FT_POINT     ***p, *pt[4], *midp, *corner;
	NODE      *ns, *ne;
	RECT_GRID dual_grid;
	TRI       ****tri;
	double     coords[MAXD], nor[MAXD];
	double     n_max;
	int       imax, jmax;
	int       ix, iy, iz;
	int       i, dim, n_dir, ip1, ip2;
	ANGLE_DIRECTION orient;

	set_dual_grid(&dual_grid,gr);
	dim = dual_grid.dim;

	corner = (surf->pos_curves == NULL) ? (*surf->neg_curves)->first->end :
	                                      (*surf->pos_curves)->first->start;

	normal_of_plane_surface(surf,nor);

	n_dir = -1;
	n_max = 0.0;
	for (i = 0; i < dim; i++)
	{
	    if (fabs(nor[i]) > n_max)
	    {
	        n_max = fabs(nor[i]);
	        n_dir = i;
	    }
	}
	switch (n_dir)
	{
	case 2: /* xy plane */

	    imax = dual_grid.gmax[0]+dual_grid.lbuf[0]+dual_grid.ubuf[0];
	    jmax = dual_grid.gmax[1]+dual_grid.lbuf[1]+dual_grid.ubuf[1];
	    bi_array(&p,imax+1,jmax+1,sizeof(FT_POINT *));
	    tri_array(&tri,imax,jmax,4,sizeof(TRI *));

	    for (iy = 0; iy <= jmax; iy++)
	    {
	        for (ix = 0; ix <= imax; ix++)
	        {
	            coords[0] = vd_cell_edge(ix,0,&dual_grid);
	            coords[1] = vd_cell_edge(iy,1,&dual_grid);
	            coords[2] = Coords(corner)[2];
	            p[ix][iy] = Point(coords);
	        }
	    }
	    orient = (nor[2] > 0) ? COUNTER_CLOCK : CLOCKWISE;
	    break;

	case 1: /* xz plane */

	    imax = dual_grid.gmax[2]+dual_grid.lbuf[2]+dual_grid.ubuf[2];
	    jmax = dual_grid.gmax[0]+dual_grid.lbuf[0]+dual_grid.ubuf[0];
	    bi_array(&p,imax+1,jmax+1,sizeof(FT_POINT *));
	    tri_array(&tri,imax,jmax,4,sizeof(TRI *));

	    for (ix = 0; ix <= jmax; ix++)
	    {
	        for (iz = 0; iz <= imax; iz++)
	        {
	            coords[0] = vd_cell_edge(ix,0,&dual_grid);
	            coords[1] = Coords(corner)[1];
	            coords[2] = vd_cell_edge(iz,2,&dual_grid);
	            p[iz][ix] = Point(coords);
	        }
	    }
	    orient = (nor[1] > 0) ? COUNTER_CLOCK : CLOCKWISE;
	    break;

	case 0: /* yz plane */

	    imax = dual_grid.gmax[1]+dual_grid.lbuf[1]+dual_grid.ubuf[1];
	    jmax = dual_grid.gmax[2]+dual_grid.lbuf[2]+dual_grid.ubuf[2];
	    bi_array(&p,imax+1,jmax+1,sizeof(FT_POINT *));
	    tri_array(&tri,imax,jmax,4,sizeof(TRI *));

	    for (iz = 0; iz <= jmax; iz++)
	    {
	        for (iy = 0; iy <= imax; iy++)
	        {
	            coords[0] = Coords(corner)[0];
	            coords[1] = vd_cell_edge(iy,1,&dual_grid);
	            coords[2] = vd_cell_edge(iz,2,&dual_grid);
	            p[iy][iz] = Point(coords);
	        }
	    }
	    orient = (nor[0] > 0) ? COUNTER_CLOCK : CLOCKWISE;
	    break;
	}

	/* make triangles */
	for (iy = 0; iy < jmax; iy++)
	{
	    for (ix = 0; ix < imax; ix++)
	    {
	        pt[0] = p[ix][iy];
	        pt[1] = p[ix+1][iy];
	        pt[2] = p[ix+1][iy+1];
	        pt[3] = p[ix][iy+1];

	        for (i = 0; i < 3; i++)
	            coords[i] = 0.5*(Coords(pt[0])[i]+ Coords(pt[2])[i]);
	        midp = Point(coords);

	        for (i = 0; i < 4; i++)
	        {
	            ip1 = (orient == COUNTER_CLOCK) ? i : Next_m4(i);
	            ip2 = (orient == COUNTER_CLOCK) ? Next_m4(i): i;
	            tri[ix][iy][i] = make_tri(pt[ip1],pt[ip2],midp,
					      NULL,NULL,NULL,YES);
	            insert_tri_at_tail_of_list(tri[ix][iy][i],surf);
	        }
	    }
	}
	/* set tri neighbors */
	for (iy = 0; iy < jmax; iy++)
	{
	    for (ix = 0; ix < imax; ix++)
	    {
	        for (i = 0; i < 4; i++)
	        {
	            set_01_bdry(Boundary_tri(tri[ix][iy][i]),YES);

	            if ((i == 0) && (iy != 0))
	            {
	                /* south */
	                Tri_on_side01(tri[ix][iy][i]) = tri[ix][iy-1][2];
	                Boundary_tri(tri[ix][iy][i]) = NO;
	                set_01_bdry(Boundary_tri(tri[ix][iy][i]),NO);
	            }
	            else if ((i == 1) && ix !=(imax-1))
	            {
	                /* east */
	                Tri_on_side01(tri[ix][iy][i]) = tri[ix+1][iy][3];
	                Boundary_tri(tri[ix][iy][i]) = NO;
	                set_01_bdry(Boundary_tri(tri[ix][iy][i]),NO);
	            }
	            else if ((i == 2) && iy !=(jmax-1))
	            {
	                /* north */
	                Tri_on_side01(tri[ix][iy][i]) = tri[ix][iy+1][0];
	                Boundary_tri(tri[ix][iy][i]) = NO;
	                set_01_bdry(Boundary_tri(tri[ix][iy][i]),NO);
	            }
	            else if ((i == 3) && ix !=0)
	            {
	                /* west */
	                Tri_on_side01(tri[ix][iy][i]) = tri[ix-1][iy][1];
	                Boundary_tri(tri[ix][iy][i]) = NO;
	                set_01_bdry(Boundary_tri(tri[ix][iy][i]),NO);
	            }
	            ip1 = (orient == COUNTER_CLOCK) ? Next_m4(i): Prev_m4(i);
	            ip2 = (orient == COUNTER_CLOCK) ? Prev_m4(i): Next_m4(i);
	            Tri_on_side12(tri[ix][iy][i]) = tri[ix][iy][ip1];
	            Tri_on_side20(tri[ix][iy][i]) = tri[ix][iy][ip2];
	            set_12_bdry(Boundary_tri(tri[ix][iy][i]),NO);
	            set_20_bdry(Boundary_tri(tri[ix][iy][i]),NO);
	        }
	    }
	}

	/* remove curves in surface */
	strip_curves_in_surface(surf);

	/* reset surface binding curve */
	ns = ne = make_node(p[0][0]);
	curve = make_curve(NO_COMP,NO_COMP,ns,ne);
	switch (n_dir)
	{
	case 2: /* xy plane */
	    set_bdry_side(Boundary(curve),0,BOTTOM);
	    set_bdry_side(Boundary(curve),1,BOTTOM);
	    break;
	case 1: /* xz plane */
	    set_bdry_side(Boundary(curve),0,BOTTOM);
	    set_bdry_side(Boundary(curve),2,BOTTOM);
	    break;
	case 0: /* yz plane */
	    set_bdry_side(Boundary(curve),1,BOTTOM);
	    set_bdry_side(Boundary(curve),2,BOTTOM);
	}
	assign_curve_boundary_flag(curve);
	install_curve_in_surface_bdry(surf,curve,POSITIVE_ORIENTATION);

	b = curve->first;
	if (orient == COUNTER_CLOCK)
	{
	    for (ix = 1; ix <= imax; ix++, b = b->next)
	        if (insert_point_in_bond(p[ix][0],b,curve) !=
		    FUNCTION_SUCCEEDED)
	        {
	            screen("ERROR in right_planar_surface_triangulation(), "
		           "insert_point_in_bond() failed\n");
	            clean_up(ERROR);
	        }

	    for (iy = 1; iy <= jmax; iy++, b = b->next)
	        if (insert_point_in_bond(p[imax][iy],b,curve) !=
		    FUNCTION_SUCCEEDED)
	        {
	            screen("ERROR in right_planar_surface_triangulation(), "
		           "insert_point_in_bond() failed\n");
	            clean_up(ERROR);
	        }

	    for (ix = imax-1; ix >= 0; ix--, b = b->next)
	        if (insert_point_in_bond(p[ix][jmax],b,curve) !=
		    FUNCTION_SUCCEEDED)
	        {
	            screen("ERROR in right_planar_surface_triangulation(), "
		           "insert_point_in_bond() failed\n");
	            clean_up(ERROR);
	        }

	    for (iy = jmax-1; iy >= 1; iy--, b = b->next)
	        if (insert_point_in_bond(p[0][iy],b,curve) !=
		    FUNCTION_SUCCEEDED)
	        {
	            screen("ERROR in right_planar_surface_triangulation(), "
		           "insert_point_in_bond() failed\n");
	            clean_up(ERROR);
	        }
	}
	else
	{
	    for (iy = 1; iy <= jmax; iy++, b = b->next)
	        if (insert_point_in_bond(p[0][iy],b,curve) !=
		    FUNCTION_SUCCEEDED)
	        {
	            screen("ERROR in right_planar_surface_triangulation(), "
		           "insert_point_in_bond() failed\n");
	            clean_up(ERROR);
	        }

	    for (ix = 1; ix <= imax; ix++, b = b->next)
	        if (insert_point_in_bond(p[ix][jmax],b,curve) !=
		    FUNCTION_SUCCEEDED)
	        {
	            screen("ERROR in right_planar_surface_triangulation(), "
		           "insert_point_in_bond() failed\n");
	            clean_up(ERROR);
	        }

	    for (iy = jmax-1; iy >= 0; iy--, b = b->next)
	        if (insert_point_in_bond(p[imax][iy],b,curve) !=
		    FUNCTION_SUCCEEDED)
	        {
	            screen("ERROR in right_planar_surface_triangulation(), "
		           "insert_point_in_bond() failed\n");
	            clean_up(ERROR);
	        }

	    for (ix = imax-1; ix >= 1; ix--, b = b->next)
	        if (insert_point_in_bond(p[ix][0],b,curve) !=
		    FUNCTION_SUCCEEDED)
	        {
	            screen("ERROR in right_planar_surface_triangulation(), "
		           "insert_point_in_bond() failed\n");
	            clean_up(ERROR);
	        }
	}
	/* set bond-tris connection */
	b = curve->first;
	if (orient == COUNTER_CLOCK)
	{
	    /* bottom side */
	    for (ix = 0; ix < imax; ix++, b = b->next)
	    {
	        (void) link_tri_to_bond(NULL,tri[ix][0][0],surf,b,curve);
	    }

	    /* right hand side */
	    for (iy = 0; iy < jmax; iy++, b = b->next)
	    {
	        (void) link_tri_to_bond(NULL,tri[imax-1][iy][1],surf,b,curve);
	    }

	    /* top side */
	    for (ix = imax-1; ix >= 0; ix--, b = b->next)
	    {
	        (void) link_tri_to_bond(NULL,tri[ix][jmax-1][2],surf,b,curve);
	    }

	    /* left hand side */
	    for (iy = jmax-1; iy >= 0; iy--, b = b->next)
	    {
	        (void) link_tri_to_bond(NULL,tri[0][iy][3],surf,b,curve);
	    }
	}
	else
	{
	    for (iy = 0; iy < jmax; iy++, b = b->next)
	    {
	        (void) link_tri_to_bond(NULL,tri[0][iy][3],surf,b,curve);
	    }

	    for (ix = 0; ix < imax; ix++, b = b->next)
	    {
	        (void) link_tri_to_bond(NULL,tri[ix][jmax-1][2],surf,b,curve);
	    }

	    for (iy = jmax-1; iy >= 0; iy--, b = b->next)
	    {
	        (void) link_tri_to_bond(NULL,tri[imax-1][iy][1],surf,b,curve);
	    }

	    for (ix = imax-1; ix >= 0; ix--, b = b->next)
	    {
	        (void) link_tri_to_bond(NULL,tri[ix][0][0],surf,b,curve);
	    }
	}

	surf->interface->num_points = surf->interface->num_points+
	                              (imax+1)*(jmax+1)+imax*jmax;
	free(p);
	free(tri);
}		/*end right_planar_surface_triangulation*/

LOCAL void strip_curves_in_surface(
	SURFACE* surf)
{
	INTERFACE *intfc = surf->interface;
	CURVE     **c, *curve;
	NODE      **n;

	for (c = surf->pos_curves; c && *c && surf->pos_curves; c++)
	{
	    curve = *c;
	    if (!(delete_from_pointers(curve,&surf->pos_curves) &&
	             delete_from_pointers(surf,&curve->pos_surfaces)))
	    {
	        screen("ERROR in strip_curves_in_surface(), "
	               "delete_from_pointers() failed\n");
	        clean_up(ERROR);
	    }
	    (void) delete_curve(curve);
	    --c;
	}
	for (c = surf->neg_curves; c && *c && surf->neg_curves; c++)
	{
	    curve = *c;
	    if (!(delete_from_pointers(curve,&surf->neg_curves) &&
	             delete_from_pointers(surf,&curve->neg_surfaces)))
	    {
	        screen("ERROR in strip_curves_in_surface(), "
	               "delete_from_pointers() failed\n");
	        clean_up(ERROR);
	    }
	    (void) delete_curve(curve);
	    --c;
	}
	for (n = intfc->nodes; n && *n && intfc->nodes; n++)
	{
	    if ((*n)->in_curves == NULL && (*n)->out_curves == NULL)
	    {
	        (void) delete_node(*n);
	        --n;
	    }
	}
}		/*end strip_curves_in_surface*/

EXPORT	void	planar_hole_surface_triangulation(
	SURFACE	     *surf,
	RECT_GRID    *gr,
	FT_POINT        *pmin,
	FT_POINT        *pmel,
	FT_POINT        *pmeu,
	FT_POINT        *pmax)
{
	BOND             *b;
	FT_POINT            ***p, *pt[4], *midp;
	TRI              *tri2,****tri;
	TRI              **tri1,**tri_list=NULL;
	double            coords[MAXD], nor[MAXD];
	double            n_max;
	int              imax, jmax;
	int              ix, iy, iz,ix1,ix2,iy1,iy2,iz1,iz2;
	int              i, dim, n_dir;
	ANGLE_DIRECTION  orient;
	int              ip1, ip2;
	double            x1,x2,x3,x4,y1,y2,y3,y4,z1,z2,z3,z4;
	int              imel,imeu,jmel,jmeu;
	double            dx[3],dy[3],dz[3];
	ORIENTATION      orient_o[4],orient_i[4];
	FT_POINT            *P[4],*Q[4];
	CURVE            *cur_o[4],*cur_i[4];
	double            h[3];

	/*  
	 * (imax,jmax)--------index for upper corner of the outer rectangle, 
	 * (imeu,jmeu)--------index for upper corner of the hole 
	 * (imel,jmel)--------index for lower corner of the hole, 
	 * (0,0)--------------index for lower corner,   
	 */

	x1 = Coords(pmin)[0]; 
	y1 = Coords(pmin)[1]; 
	z1 = Coords(pmin)[2];
	x2 = Coords(pmel)[0]; 
	y2 = Coords(pmel)[1]; 
	z2 = Coords(pmel)[2];
	x3 = Coords(pmeu)[0]; 
	y3 = Coords(pmeu)[1]; 
	z3 = Coords(pmeu)[2];
	x4 = Coords(pmax)[0]; 
	y4 = Coords(pmax)[1]; 
	z4 = Coords(pmax)[2];

	/* set dual grid points */

	dim = 3;

	normal_of_plane_surface(surf,nor);

	n_dir = -1;
	n_max = 0.0;
	for (i = 0; i < dim; i++)
	{
	    if (fabs(nor[i]) > n_max)
	    {
	        n_max = fabs(nor[i]);
	        n_dir = i;
	    }
	}

	for (i = 0; i < 3; i++)
	    h[i] = gr->h[i];

	switch(n_dir)
	{
	case 2:                        /* projection to xy plane */
	    ix1 = irint((x2 - x1)/h[0]);
	    iy1 = irint((y2 - y1)/h[1]);
	    ix2 = ix1 + irint((x3 - x2)/h[0]);
	    iy2 = iy1 + irint((y3 - y2)/h[1]);
	    imax = ix2 + irint((x4 - x3)/h[0]);
	    jmax = iy2 + irint((y4 - y3)/h[1]);
	    dx[0] = (x2 - x1)/ix1;
	    dx[1] = (x3 - x2)/(ix2 - ix1);
	    dx[2] = (x4 - x3)/(imax - ix2);
	    dy[0] = (y2 - y1)/iy1;
	    dy[1] = (y3 - y2)/(iy2 - iy1);
	    dy[2] = (y4 - y3)/(jmax - iy2);
	    break;

	case 1:                        /* projection to xz plane */
	    iz1 = irint((z2 - z1)/h[2]);
	    iz2 = iz1 + irint((z3 - z2)/h[2]);
	    imax = iz2 + irint((z4 - z1)/h[2]);
	    ix1 = irint((x2 - x1)/h[0]);
	    ix2 = ix1 + irint((x3 - x2)/h[0]);
	    jmax = ix2 + irint((x4 - x1)/h[0]);
	    dx[0] = (x2 - x1)/ix1;
	    dx[1] = (x3 - x2)/(ix2 - ix1);
	    dx[2] = (x4 - x3)/(jmax - ix2);

	    dz[0] = (z2 - z1)/iz1;
	    dz[1] = (z3 - z2)/(iz2 - iz1);
	    dz[2] = (z4 - z3)/(imax - iz2);
	    break;

	case 0:                        /* projection to yz plane */
	    iz1 = irint((z2 - z1)/h[2]);
	    iz2 = iz1 + irint((z3 - z2)/h[2]);
	    jmax = iz2 + irint((z4 - z1)/h[2]);
	    iy1 = irint((y2 - y1)/h[1]);
	    iy2 = iy1 + irint((y3 - y2)/h[1]);
	    imax = iy2 + irint((y4 - y3)/h[1]);
	    dz[0] = (z2 - z1)/iz1;
	    dz[1] = (z3 - z2)/(iz2 - iz1);
	    dz[2] = (z4 - z3)/(jmax - iz2);

	    dy[0] = (y2 - y1)/iy1;
	    dy[1] = (y3 - y2)/(iy2 - iy1);
	    dy[2] = (y4 - y3)/(imax - iy2);
	    break;
	}

	bi_array(&p,imax+1,jmax+1,sizeof(FT_POINT *));
	tri_array(&tri,imax,jmax,4,sizeof(TRI *));
	
	switch (n_dir)
	{
	case 2:  /* xy plane */

	    imel = ix1;
	    jmel = iy1;
	    imeu = ix2;
	    jmeu = iy2;

	    for (iy = 0; iy <= jmax; iy++)
	    {
	        for (ix = 0; ix <= imax; ix++)
	        {
	            if (!((ix1 < ix) && (ix < ix2) && 
	                (iy1 < iy) && (iy < iy2)))
	            {
	                if (ix < ix1)
	                    coords[0] = x1 + ix*dx[0];
	                else if (ix < ix2)
	                    coords[0] = x2 + (ix - ix1)*dx[1];
	                else
	                    coords[0] = x3 + (ix - ix2)*dx[2];

	                if (iy < iy1)
	                    coords[1] = y1 + iy*dy[0];
	                else if (iy < iy2)
	                    coords[1] = y2 + (iy - iy1)*dy[1];
	                else
	                    coords[1] = y3 + (iy - iy2)*dy[2];
	                coords[2] = Coords(pmin)[2];
	                p[ix][iy] = Point(coords);
	            }
	        }
	    }
	    orient = (nor[2] > 0) ? COUNTER_CLOCK : CLOCKWISE;
	    break;

	case 1:  /* xz plane */

	    imel = iz1;
	    jmel = ix1;
	    imeu = iz2;
	    jmeu = ix2;

	    for (ix = 0; ix <= jmax; ix++)
	    {
	        for (iz = 0; iz <= imax; iz++)
	        {
	            if (!((ix1 < ix) && (ix < ix2) &&
	                (iz1 < iz) && (iz < iz2)))
	            {
	                if (ix < ix1)
	                    coords[0] = x1 + ix*dx[0];
	                else if (ix < ix2)
	                    coords[0] = x2 + (ix - ix1)*dx[1];
	                else
	                    coords[0] = x3 + (ix - ix2)*dx[2];

	                if (iz < iz1)
	                    coords[2] = z1 + iz*dz[0];
	                else if (iz < iz2)
	                    coords[2] = z2 + (iz - iz1)*dz[1];
	                else
	                    coords[2] = z3 + (iz - iz2)*dz[2];

	                coords[1] = Coords(pmin)[1];
	                p[iz][ix] = Point(coords);
	            }
	        }
	    }
	    orient = (nor[1] > 0) ? COUNTER_CLOCK : CLOCKWISE;
	    break;

	case 0:  /* yz plane */
	  
	    imel = iy1;
	    jmel = iz1;
	    imeu = iy2;
	    jmeu = iz2;

	    for (iz = 0; iz <= jmax; iz++)
	    {
	        for (iy = 0; iy <= imax; iy++)
	        {
	            if (!((iy1 < iy) && (iy < iy2) && 
	                (iz1 < iz) && (iz < iz2)))
	            {
	                if (iy < iy1)
	                    coords[1] = y1 + iy*dy[0];
	                else if (iy < iy2)
	                    coords[1] = y2 + (iy - iy1)*dy[1];
	                else
	                    coords[1] = y3 + (iy - iy2)*dy[2];

	                if (iz < iz1)
	                    coords[2] = z1 + iz*dz[0];
	                else if (iz < iz2)
	                    coords[2] = z2 + (iz - iz1)*dz[1];
	                else
	                    coords[2] = z3 + (iz - iz2)*dz[2];

	                coords[0] = Coords(pmin)[0];
	                p[iy][iz] = Point(coords);
	            }
	        }
	    }
	    orient = (nor[0] > 0) ? COUNTER_CLOCK : CLOCKWISE;
	    break;
	}

	P[0] = p[imel][jmel];
	P[1] = p[imeu][jmel];
	P[2] = p[imeu][jmeu];	
	P[3] = p[imel][jmeu];

	Q[0] = p[0][0];
	Q[1] = p[imax][0];
	Q[2] = p[imax][jmax];
	Q[3] = p[0][jmax];

	for (i = 0; i < 4; i++)
        {
	    cur_o[i] = id_curve_by_points(surf,Q[i],Q[(i+1)%4],orient_o+i);
	    cur_i[i] = id_curve_by_points(surf,P[i],P[(i+1)%4],orient_i+i);
	}

	/* let the addresses of the vertices at the corners be the
	   same as those of the bond->start || bond->end */

	if (orient_o[0] == POSITIVE_ORIENTATION)
	    p[0][0] = cur_o[0]->first->start;
	else if (orient_o[0] == NEGATIVE_ORIENTATION)
	    p[0][0] = cur_o[0]->last->end; /* safe to remove 'if' here ? */

 	if (orient_o[1] == POSITIVE_ORIENTATION)
	    p[imax][0] = cur_o[1]->first->start;
	else if (orient_o[1] == NEGATIVE_ORIENTATION)
	    p[imax][0] = cur_o[1]->last->end;

	if (orient_o[2] == POSITIVE_ORIENTATION)
	    p[imax][jmax] = cur_o[2]->first->start;
	else if (orient_o[2] == NEGATIVE_ORIENTATION)
	    p[imax][jmax] = cur_o[2]->last->end;

	if (orient_o[3] == POSITIVE_ORIENTATION)
	    p[0][jmax] = cur_o[3]->first->start;
	else if (orient_o[3] == NEGATIVE_ORIENTATION)
	    p[0][jmax] = cur_o[3]->last->end;

	if (orient_i[0] == POSITIVE_ORIENTATION)
	    p[imel][jmel] = cur_i[0]->first->start;
	else if (orient_i[0] == NEGATIVE_ORIENTATION)
	    p[imel][jmel] = cur_i[0]->last->end;

	if (orient_i[1] == POSITIVE_ORIENTATION)
	    p[imeu][jmel] = cur_i[1]->first->start;
	else if (orient_i[1] == NEGATIVE_ORIENTATION)
	    p[imeu][jmel] = cur_i[1]->last->end;

	if (orient_i[2] == POSITIVE_ORIENTATION)
	    p[imeu][jmeu] = cur_i[2]->first->start;
	else if (orient_i[2] == NEGATIVE_ORIENTATION)
	    p[imeu][jmeu] = cur_i[2]->last->end;

	if (orient_i[3] == POSITIVE_ORIENTATION)
	    p[imel][jmeu] = cur_i[3]->first->start;
	else if (orient_i[3] == NEGATIVE_ORIENTATION)
	    p[imel][jmeu] = cur_i[3]->last->end;

	set_curve_bdry_flag(cur_o[0],gr);
	set_curve_bdry_flag(cur_o[1],gr);
	set_curve_bdry_flag(cur_o[2],gr);
	set_curve_bdry_flag(cur_o[3],gr);

	/* make triangles */
	for (iy = 0; iy < jmax; iy++)
	{
	    for (ix = 0; ix < imax; ix++)
	    {
	        if (!((imel <= ix) && (ix < imeu) &&
	            (jmel <= iy) && (iy < jmeu)))
	        {
	            pt[0] = p[ix][iy];
	            pt[1] = p[ix+1][iy];
	            pt[2] = p[ix+1][iy+1];
	            pt[3] = p[ix][iy+1];

	            for (i = 0; i < 3; i++)
	                coords[i] = 0.5*(Coords(pt[0])[i]+ Coords(pt[2])[i]);
	            midp = Point(coords);

	            for (i = 0; i < 4; i++)
	            {
	                ip1 = (orient == COUNTER_CLOCK) ? i : Next_m4(i);
	                ip2 = (orient == COUNTER_CLOCK) ? Next_m4(i): i;
	                tri[ix][iy][i] = make_tri(pt[ip1],pt[ip2],midp,
	                                          NULL,NULL,NULL,YES);
	                insert_tri_at_tail_of_list(tri[ix][iy][i],surf);
	            }
	        }
	    }
	}

	/* set tri neighbors */
	for (iy = 0; iy < jmax; iy++)
	{
	    for (ix = 0; ix < imax; ix++)
	    {
	        if (!((imel <= ix) && (ix < imeu) &&
	            (jmel <= iy) && (iy < jmeu)))
	        {
	            for (i = 0; i < 4; i++)
	            {
	                set_01_bdry(Boundary_tri(tri[ix][iy][i]),YES);

	                if ((i == 0) && (iy != 0) &&
	                    ((iy != jmeu) || 
	                    ((iy == jmeu) && ((ix < imel) || (ix >= imeu)))))
	                {   /* south */
	                    Tri_on_side01(tri[ix][iy][i]) = tri[ix][iy-1][2];
	                    Boundary_tri(tri[ix][iy][i]) = NO;
	                    set_01_bdry(Boundary_tri(tri[ix][iy][i]),NO);
	                }
	                else if ((i == 1) && (ix != imax-1) &&
	                    ((ix != imel-1) || 
	                    ((ix == imel-1) &&
	                    ((iy < jmel) || (iy >= jmeu)))))
	                {   /* east */
	                    Tri_on_side01(tri[ix][iy][i]) = tri[ix+1][iy][3];
	                    Boundary_tri(tri[ix][iy][i]) = NO;
	                    set_01_bdry(Boundary_tri(tri[ix][iy][i]),NO);
	                }
	                else if ((i == 2) && (iy != jmax-1) && 
	                    ((iy != jmel-1) ||
	                    ((iy == jmel-1) &&
	                    ((ix < imel) || (ix >= imeu)))))
	                {   /* north */
	                    Tri_on_side01(tri[ix][iy][i]) = tri[ix][iy+1][0];
	                    Boundary_tri(tri[ix][iy][i]) = NO;
	                    set_01_bdry(Boundary_tri(tri[ix][iy][i]),NO);
	                }
	                else if ((i == 3) && (ix != 0) && 
	                    ((ix != imeu) ||
	                    ((ix == imeu) &&
	                    ((iy < jmel) || (iy >= jmeu)))))
	                {   /* west */
	                    Tri_on_side01(tri[ix][iy][i]) = tri[ix-1][iy][1];
	                    Boundary_tri(tri[ix][iy][i]) = NO;
	                    set_01_bdry(Boundary_tri(tri[ix][iy][i]),NO);
	                }

	                ip1 = (orient==COUNTER_CLOCK) ? Next_m4(i): Prev_m4(i);
	                ip2 = (orient==COUNTER_CLOCK) ? Prev_m4(i): Next_m4(i);
	                Tri_on_side12(tri[ix][iy][i]) = tri[ix][iy][ip1];
	                Tri_on_side20(tri[ix][iy][i]) = tri[ix][iy][ip2];
	                set_12_bdry(Boundary_tri(tri[ix][iy][i]),NO);
	                set_20_bdry(Boundary_tri(tri[ix][iy][i]),NO);
	            }
	        }
	    }
	}

	/* insert points in curve and link_tri_to_bond of cur_o*/

	if (orient_o[0] == POSITIVE_ORIENTATION)        /*same direction */
	{
	    b = cur_o[0]->first;
	    if (Q[1]==b->end) /*assure not reinstall bond*/
	    {
	        for (ix = 1; ix < imax; ix++,b = b->next)
	            if (insert_point_in_bond(p[ix][0],b,cur_o[0]) !=
			FUNCTION_SUCCEEDED)
	            {
	                screen("ERROR in planar_hole_surface_triangulation(), "
		               "insert_point_in_bond() failed\n");
	                clean_up(ERROR);
	            }
	    }
	}
	else if (orient_o[0] == NEGATIVE_ORIENTATION) /* opposite direction */
	{
	    b = cur_o[0]->first;
	    if (Q[0]==b->end)
	    {
	        for (ix = imax-1;ix > 0; ix--,b = b->next)
	            if (insert_point_in_bond(p[ix][0],b,cur_o[0]) !=
			FUNCTION_SUCCEEDED)
	            {
	                screen("ERROR in planar_hole_surface_triangulation(), "
		               "insert_point_in_bond() failed\n");
	                clean_up(ERROR);
	            }
	    }
	}

	if (orient_o[1] == POSITIVE_ORIENTATION)
	{
	    b = cur_o[1]->first;
	    if (Q[2]==b->end)
	    {
	        for (iy = 1; iy < jmax; iy++,b = b->next)
	            if (insert_point_in_bond(p[imax][iy],b,cur_o[1]) !=
			FUNCTION_SUCCEEDED)
	            {
	                screen("ERROR in planar_hole_surface_triangulation(), "
		               "insert_point_in_bond() failed\n");
	                clean_up(ERROR);
	            }
	    }
	}
	else if (orient_o[1] == NEGATIVE_ORIENTATION)
	{
	    b = cur_o[1]->first;
	    if (Q[1]==b->end)
	    {
	        for (iy = jmax-1; iy > 0; iy--,b = b->next)
	            if (insert_point_in_bond(p[imax][iy],b,cur_o[1]) !=
			FUNCTION_SUCCEEDED)
	            {
	                screen("ERROR in planar_hole_surface_triangulation(), "
		               "insert_point_in_bond() failed\n");
	                clean_up(ERROR);
	            }
	    }
	}

	if (orient_o[2] == NEGATIVE_ORIENTATION)
	{
	    b = cur_o[2]->first;
	    if (Q[2]==b->end)
	    {
	        for (ix = 1; ix < imax; ix++,b = b->next)
	            if (insert_point_in_bond(p[ix][jmax],b,cur_o[2]) !=
			FUNCTION_SUCCEEDED)
	            {
	                screen("ERROR in planar_hole_surface_triangulation(), "
		               "insert_point_in_bond() failed\n");
	                clean_up(ERROR);
	            }
	    }
	}
	else if (orient_o[2] == POSITIVE_ORIENTATION)
	{
	    b = cur_o[2]->first;
	    if (Q[3]==b->end)
	    {
	        for (ix = imax-1; ix > 0; ix--,b = b->next)
	            if (insert_point_in_bond(p[ix][jmax],b,cur_o[2]) !=
			FUNCTION_SUCCEEDED)
	            {
	                screen("ERROR in planar_hole_surface_triangulation(), "
		               "insert_point_in_bond() failed\n");
	                clean_up(ERROR);
	            }
	    }
	}

	if (orient_o[3] == NEGATIVE_ORIENTATION)
	{
	    b = cur_o[3]->first;
	    if (Q[3]==b->end)
	    {
	        for (iy = 1; iy < jmax; iy++,b = b->next)
	            if (insert_point_in_bond(p[0][iy],b,cur_o[3]) !=
			FUNCTION_SUCCEEDED)
	            {
	                screen("ERROR in planar_hole_surface_triangulation(), "
		               "insert_point_in_bond() failed\n");
	                clean_up(ERROR);
	            }
	    }
	}
	else if (orient_o[3] == POSITIVE_ORIENTATION)
	{
	    b = cur_o[3]->first;
	    if (Q[0]==b->end)
	    {
	        for (iy = jmax-1; iy > 0; iy--,b = b->next)
	            if (insert_point_in_bond(p[0][iy],b,cur_o[3]) !=
			FUNCTION_SUCCEEDED)
	            {
	                screen("ERROR in planar_hole_surface_triangulation(), "
		               "insert_point_in_bond() failed\n");
	                clean_up(ERROR);
	            }
	    }
	}

	if (orient_o[0] == POSITIVE_ORIENTATION)
	{
	    b = cur_o[0]->first;
	    for (ix = 0; ix < imax; ix++,b = b->next)
	        (void) link_tri_to_bond(NULL,tri[ix][0][0],surf,b,cur_o[0]);

	}
	else if (orient_o[0] == NEGATIVE_ORIENTATION)
	{
	    b = cur_o[0]->first;
	    for (ix = imax-1; ix >= 0; ix--,b = b->next)
	        (void) link_tri_to_bond(NULL,tri[ix][0][0],surf,b,cur_o[0]);
	}

	if (orient_o[1] == POSITIVE_ORIENTATION)
	{
	    b = cur_o[1]->first;
	    for (iy = 0; iy < jmax; iy++,b = b->next)
	        (void) link_tri_to_bond(NULL,tri[imax-1][iy][1],surf,
					b,cur_o[1]);
	}
	else if (orient_o[1] == NEGATIVE_ORIENTATION)
	{
	    b = cur_o[1]->first;
	    for (iy = jmax-1; iy >= 0; iy--,b = b->next)
	        (void) link_tri_to_bond(NULL,tri[imax-1][iy][1],surf,
					b,cur_o[1]);
	}

	if (orient_o[2] == NEGATIVE_ORIENTATION)
	{
	    b = cur_o[2]->first;
	    for (ix = 0; ix < imax; ix++,b = b->next)
	        (void) link_tri_to_bond(NULL,tri[ix][jmax-1][2],surf,
					b,cur_o[2]);
	}
	else if (orient_o[2] == POSITIVE_ORIENTATION)
	{
	    b = cur_o[2]->first;
	    for (ix = imax-1; ix >= 0; ix--,b = b->next)
	        (void) link_tri_to_bond(NULL,tri[ix][jmax-1][2],surf,
					b,cur_o[2]);
	}

	if (orient_o[3] == NEGATIVE_ORIENTATION)
	{
	    b = cur_o[3]->first;
	    for (iy = 0; iy < jmax; iy++,b = b->next)
	        (void) link_tri_to_bond(NULL,tri[0][iy][3],surf,
					b,cur_o[3]);

	}
	else if (orient_o[3] == POSITIVE_ORIENTATION)
	{
	    b = cur_o[3]->first;
	    for (iy = jmax-1; iy >= 0; iy--,b = b->next)
	        (void) link_tri_to_bond(NULL,tri[0][iy][3],surf,b,cur_o[3]);
	}


	/* insert points in curve and link_tri_to_bond of cur_i*/

	if (orient_i[0] == POSITIVE_ORIENTATION)        /*same direction */
	{
	    b = cur_i[0]->first;
	    for (ix = 1+imel; ix < imeu; ix++,b = b->next)
	        if (insert_point_in_bond(p[ix][jmel],b,cur_i[0]) !=
		    FUNCTION_SUCCEEDED)
	        {
	            screen("ERROR in planar_hole_surface_triangulation(), "
		           "insert_point_in_bond() failed\n");
	            clean_up(ERROR);
	        }
	}
	else if (orient_i[0] == NEGATIVE_ORIENTATION) /* opposite direction */
	{
	    b = cur_i[0]->first;
	    for (ix = imeu-1; ix > imel; ix--,b = b->next)
	        if (insert_point_in_bond(p[ix][jmel],b,cur_i[0]) !=
		    FUNCTION_SUCCEEDED)
	        {
	            screen("ERROR in planar_hole_surface_triangulation(), "
		           "insert_point_in_bond() failed\n");
	            clean_up(ERROR);
	        }
	}

	if (orient_i[1] == POSITIVE_ORIENTATION)
	{
	    b = cur_i[1]->first;
	    for (iy = 1+jmel; iy < jmeu; iy++,b = b->next)
	        if (insert_point_in_bond(p[imeu][iy],b,cur_i[1]) !=
		    FUNCTION_SUCCEEDED)
	        {
	            screen("ERROR in planar_hole_surface_triangulation(), "
		           "insert_point_in_bond() failed\n");
	            clean_up(ERROR);
	        }
	}
	else if (orient_i[1] == NEGATIVE_ORIENTATION)
	{
	    b = cur_i[1]->first;
	    for (iy = jmeu-1; iy > jmel; iy--,b = b->next)
	        if (insert_point_in_bond(p[imeu][iy],b,cur_i[1]) !=
		    FUNCTION_SUCCEEDED)
	        {
	            screen("ERROR in planar_hole_surface_triangulation(), "
		           "insert_point_in_bond() failed\n");
	            clean_up(ERROR);
	        }
	}

	if (orient_i[2] == NEGATIVE_ORIENTATION)
	{
	    b = cur_i[2]->first;
	    for (ix = 1+imel; ix < imeu; ix++,b = b->next)
	        if (insert_point_in_bond(p[ix][jmeu],b,cur_i[2]) !=
		    FUNCTION_SUCCEEDED)
	        {
	            screen("ERROR in planar_hole_surface_triangulation(), "
		           "insert_point_in_bond() failed\n");
	            clean_up(ERROR);
	        }
	}
	else if (orient_i[2] == POSITIVE_ORIENTATION)
	{
	    b = cur_i[2]->first;
	    for (ix = imeu-1; ix > imel; ix--,b = b->next)
	        if (insert_point_in_bond(p[ix][jmeu],b,cur_i[2]) !=
		    FUNCTION_SUCCEEDED)
	        {
	            screen("ERROR in planar_hole_surface_triangulation(), "
		           "insert_point_in_bond() failed\n");
	            clean_up(ERROR);
	        }
	}

	if (orient_i[3] == NEGATIVE_ORIENTATION)
	{
	    b = cur_i[3]->first;
	    for (iy = 1+jmel; iy < jmeu; iy++,b = b->next)
	        if (insert_point_in_bond(p[imel][iy],b,cur_i[3]) !=
		    FUNCTION_SUCCEEDED)
	        {
	            screen("ERROR in planar_hole_surface_triangulation(), "
		           "insert_point_in_bond() failed\n");
	            clean_up(ERROR);
	        }
	}
	else if (orient_i[3] == POSITIVE_ORIENTATION)
	{
	    b = cur_i[3]->first;
	    for (iy = jmeu-1; iy > jmel; iy--,b = b->next)
	        if (insert_point_in_bond(p[imel][iy],b,cur_i[3]) !=
		    FUNCTION_SUCCEEDED)
	        {
	            screen("ERROR in planar_hole_surface_triangulation(), "
		           "insert_point_in_bond() failed\n");
	            clean_up(ERROR);
	        }
	}

	if (orient_i[0] == POSITIVE_ORIENTATION)
	{
	    b = cur_i[0]->first;
	    for (ix = imel; ix < imeu; ix++,b = b->next)
	        (void) link_tri_to_bond(NULL,tri[ix][jmel-1][2],surf,
					b,cur_i[0]);

	}
	else if (orient_i[0] == NEGATIVE_ORIENTATION)
	{
	    b = cur_i[0]->first;
	    for (ix = imeu-1; ix >= imel; ix--,b = b->next)
	        (void) link_tri_to_bond(NULL,tri[ix][jmel-1][2],surf,
					b,cur_i[0]);
	}

	if (orient_i[1] == POSITIVE_ORIENTATION)
	{
	    b = cur_i[1]->first;
	    for (iy = jmel; iy < jmeu; iy++,b = b->next)
	        (void) link_tri_to_bond(NULL,tri[imeu][iy][3],surf,b,cur_i[1]);
	}
	else if (orient_i[1] == NEGATIVE_ORIENTATION)
	{
	    b = cur_i[1]->first;
	    for (iy = jmeu-1; iy >= jmel; iy--,b = b->next)
	        (void) link_tri_to_bond(NULL,tri[imeu][iy][3],surf,b,cur_i[1]);
	}

	if (orient_i[2] == NEGATIVE_ORIENTATION)
	{
	    b = cur_i[2]->first;
	    for (ix = imel; ix < imeu; ix++,b = b->next)
	        (void) link_tri_to_bond(NULL,tri[ix][jmeu][0],surf,b,cur_i[2]);
	}
	else if (orient_i[2] == POSITIVE_ORIENTATION)
	{
	    b = cur_i[2]->first;
	    for (ix = imeu-1; ix >= imel; ix--,b = b->next)
	        (void) link_tri_to_bond(NULL,tri[ix][jmeu][0],surf,b,cur_i[2]);
	}

	if (orient_i[3] == NEGATIVE_ORIENTATION)
	{
	    b = cur_i[3]->first;
	    for (iy = jmel; iy < jmeu; iy++,b = b->next)
	        (void) link_tri_to_bond(NULL,tri[imel-1][iy][1],surf,
					b,cur_i[3]);

	}
	else if (orient_i[3] == POSITIVE_ORIENTATION)
	{
	    b = cur_i[3]->first;
	    for (iy = jmeu-1; iy >= jmel; iy--,b = b->next)
	        (void) link_tri_to_bond(NULL,tri[imel-1][iy][1],surf,
					b,cur_i[3]);
	}
	/* remove the NULL tris from tri_list of surface*/

	for (tri2 = first_tri(surf); tri2 != last_tri(surf); tri2 = tri2->next)
	{
	    if ((Point_of_tri(tri2)[0] == NULL) ||
		(Point_of_tri(tri2)[1] == NULL) ||
	        (Point_of_tri(tri2)[2] == NULL))
	        if (!add_to_pointers(tri2,&tri_list))
	        {
		    return;
		}
	}
	for (tri1 = tri_list; tri1 && *tri1; tri1++)
	    remove_tri_from_surface(*tri1,surf,NO);

	surf->interface->num_points = surf->interface->num_points+ 
	                              (imax+1)*(jmax+1)+imax*jmax-(jmeu-jmel)*
	                              (imeu-imel)-(jmeu-jmel-1)*(imeu-imel-1);
	free(p);
	free(tri);
	return;
}                /*end planar_hole_surface_triangulation*/


EXPORT	void	oblique_planar_surface_triangulation(
	SURFACE		*surf,
	RECT_GRID	*gr)
{
	BOND            *b;
	FT_POINT           ***p, *pt[4], *midp, *corner;
	TRI             ****tri;
	double           coords[MAXD], nor[MAXD];
	double           n_max;
	int             imax, jmax;
	int             ix, iy, iz;
	int             i, dim, n_dir;
	ANGLE_DIRECTION orient;
	int             ip1, ip2;
	FT_POINT           *q;
	double           x,y,z,x1,y1,z1,x4,y4,z4;
	CURVE           **c1;
	CURVE           *curve[4];
	FT_POINT           *Q[4];
	ORIENTATION     curve_orient[4];
	double           h0,h1,h2;

	corner = (surf->pos_curves == NULL) ? (*surf->neg_curves)->first->end :
	                                      (*surf->pos_curves)->first->start;

	normal_of_plane_surface(surf,nor);

	/* set dual grid points */
	dim = gr->dim;

	n_dir = -1;
	n_max = 0.0;
	for (i = 0; i < dim; i++)
	{
	    if (fabs(nor[i]) > n_max)
	    {
	        n_max = fabs(nor[i]);
	        n_dir = i;
	    }
	}

	x1 =  HUGE_VAL;
	y1 =  HUGE_VAL;
	z1 =  HUGE_VAL;
	x4 = -HUGE_VAL;
	y4 = -HUGE_VAL;
	z4 = -HUGE_VAL;

	for (c1 = surf->pos_curves; c1 && *c1; c1++)
	{
	    q = (*c1)->first->start;
	    x = Coords(q)[0]; y = Coords(q)[1]; z = Coords(q)[2];
	    if (x < x1) x1 = x; if (y < y1) y1 = y; if (z < z1) z1 = z;
	    if (x4 < x) x4 = x; if (y4 < y) y4 = y; if (z4 < z) z4 = z;
	    for (b = (*c1)->first; b != NULL; b = b->next)
	    {
	        q = b->end;
	        x = Coords(q)[0]; y = Coords(q)[1]; z = Coords(q)[2];
	        if (x < x1) x1 = x; if (y < y1) y1 = y; if (z < z1) z1 = z;
	        if (x4 < x) x4 = x; if (y4 < y) y4 = y; if (z4 < z) z4 = z;
	    }
	}
	for (c1 = surf->neg_curves; c1 && *c1; c1++)
	{
	    q = (*c1)->first->start;
	    x = Coords(q)[0]; y = Coords(q)[1]; z = Coords(q)[2];
	    if (x < x1) x1 = x; if (y < y1) y1 = y; if (z < z1) z1 = z;
	    if (x4 < x) x4 = x; if (y4 < y) y4 = y; if (z4 < z) z4 = z;
	    for (b = (*c1)->first; b != NULL; b = b->next)
	    {
	        q = b->end;
	        x = Coords(q)[0]; y = Coords(q)[1]; z = Coords(q)[2];
	        if (x < x1) x1 = x; if (y < y1) y1 = y; if (z < z1) z1 = z;
	        if (x4 < x) x4 = x; if (y4 < y) y4 = y; if (z4 < z) z4 = z;
	    }
	}

	h0 = gr->h[0];
	h1 = gr->h[1];
	h2 = gr->h[2];

	if ((h0 <= 0)||(h1 <= 0)||(h2 <= 0))
	{
	    screen("ERROR in oblique_planar_surface_triangulation(), "
	           "the rect_grid cell size is wrong\n");
	    clean_up(ERROR);
	}

	switch(n_dir)
	{
	case 2:                        /* projection to xy plane */
	    imax = irint((x4 - x1)/h0);
	    jmax = irint((y4 - y1)/h1);
	    h0 = (x4 - x1)/imax;
	    h1 = (y4 - y1)/jmax;
	    break;

	case 1:                        /* projection to xz plane */
	    imax = irint((z4 - z1)/h2);
	    jmax = irint((x4 - x1)/h0);
	    h2 = (z4 - z1)/imax;
	    h0 = (x4 - x1)/jmax;
	    break;

	case 0:                        /* projection to yz plane */
	    imax = irint((y4 - y1)/h1);
	    jmax = irint((z4 - z1)/h2);
	    h1 = (y4 - y1)/imax;
	    h2 = (z4 - z1)/jmax;
	    break;
	}

	bi_array(&p,imax+1,jmax+1,sizeof(FT_POINT *));
	tri_array(&tri,imax,jmax,4,sizeof(TRI *));
	
	switch (n_dir)
	{
	case 2:  /* xy plane */
	    
	    for (iy = 0; iy <= jmax; iy++)
	    {
	        for (ix = 0; ix <= imax; ix++)
	        {
	            coords[0] = x1 + ix*h0;
	            coords[1] = y1 + iy*h1;
	            coords[2] = 0.0;
	            coords[2] = value_z(coords,corner,nor);
	            p[ix][iy] = Point(coords);
	        }
	    }
	    orient = (nor[2] > 0) ? COUNTER_CLOCK : CLOCKWISE;
	    break;

	case 1:  /* xz plane */
	   
	    for (ix = 0; ix <= jmax; ix++)
	    {
	        for (iz = 0; iz <= imax; iz++)
	        {
	            coords[0] = x1 + ix*h0;
	            coords[1] = 0.0;
	            coords[2] = z1 + iz*h2;
	            coords[1] = value_y(coords,corner,nor);
	            p[iz][ix] = Point(coords);
	        }
	    }
	    orient = (nor[1] > 0) ? COUNTER_CLOCK : CLOCKWISE;
	    break;

	case 0:  /* yz plane */
	    
	    for (iz = 0; iz <= jmax; iz++)
	    {
	        for (iy = 0; iy <= imax; iy++)
	        {
	            coords[0] = 0.0;
	            coords[1] = y1 + iy*h1;
	            coords[2] = z1 + iz*h2;
	            coords[0] = value_x(coords,corner,nor);
	            p[iy][iz] = Point(coords);
	        }
	    }
	    orient = (nor[0] > 0) ? COUNTER_CLOCK : CLOCKWISE;
	    break;
	}

	for (ix = 0; ix <= imax; ix++)
	{
	    Boundary_point(p[ix][0]) = 1;
	    Boundary_point(p[ix][jmax]) = 1;
	}
	for (iy = 0; iy <= jmax; iy++)
	{
	    Boundary_point(p[0][iy]) = 1;
	    Boundary_point(p[imax][iy]) = 1;
	}

	/* identify curves by points */

	Q[3] = p[0][jmax];      Q[2] = p[imax][jmax];

	Q[0] = p[0][  0 ];      Q[1] = p[imax][  0 ];

	for (i = 0; i < 4; i++)  
	    curve[i] = id_curve_by_points(surf,Q[i],Q[(i+1)%4],curve_orient+i);
	
	/* Let the addresses of the vertices at the corners be the
	   same as those of the bond->start or bond->end.          */

	if (curve_orient[0] == POSITIVE_ORIENTATION)
	    p[0][0] = curve[0]->first->start;
	else if (curve_orient[0] == NEGATIVE_ORIENTATION)
	    p[0][0] = curve[0]->last->end;

	if (curve_orient[1] == POSITIVE_ORIENTATION)
	    p[imax][0] = curve[1]->first->start;
	else if (curve_orient[1] == NEGATIVE_ORIENTATION)
	    p[imax][0] = curve[1]->last->end;

	if (curve_orient[2] == POSITIVE_ORIENTATION)
	    p[imax][jmax] = curve[2]->first->start;
	else if (curve_orient[2] == NEGATIVE_ORIENTATION)
	    p[imax][jmax] = curve[2]->last->end;

	if (curve_orient[3] == POSITIVE_ORIENTATION)
	    p[0][jmax] = curve[3]->first->start;
	else if (curve_orient[3] == NEGATIVE_ORIENTATION)
	    p[0][jmax] = curve[3]->last->end;

	/* Set the edge if it has been triangularized */

	if ((curve_orient[0] == POSITIVE_ORIENTATION) && 
	    (curve[0]->first->end != p[imax][0]))
	    for (b = curve[0]->first, ix = 0; ix < imax; ix++, b = b->next)
	        p[ix][0] = b->start;
	else if ((curve_orient[0] == NEGATIVE_ORIENTATION) &&
	         (curve[0]->first->end != p[0][0]))
	    for (b = curve[0]->first, ix = imax; ix > 0; ix--, b = b->next)
	        p[ix][0] = b->start;
	
	if ((curve_orient[1] == POSITIVE_ORIENTATION) &&
	    (curve[1]->first->end != p[imax][jmax]))
	    for (b = curve[1]->first, iy = 0; iy < jmax; iy++, b = b->next)
	        p[imax][iy] = b->start;
	else if ((curve_orient[1] == NEGATIVE_ORIENTATION) &&
	         (curve[1]->first->end != p[imax][0]))
	    for (b = curve[1]->first, iy = jmax; iy > 0; iy--, b = b->next)
	        p[imax][iy] = b->start;
	
	if ((curve_orient[2] == POSITIVE_ORIENTATION) &&
	    (curve[2]->first->end != p[0][jmax]))
	    for (b = curve[2]->first, ix = imax; ix > 0; ix--, b = b->next)
	        p[ix][jmax] = b->start;
	else if ((curve_orient[2] == NEGATIVE_ORIENTATION) &&
	         (curve[2]->first->end != p[imax][jmax]))
	    for (b = curve[2]->first, ix = 0; ix < imax; ix++, b = b->next)
	        p[ix][jmax] = b->start;
	
	if ((curve_orient[3] == POSITIVE_ORIENTATION) &&
	    (curve[3]->first->end != p[0][0]))
	    for (b = curve[3]->first, iy = jmax; iy > 0; iy--, b = b->next)
	        p[0][iy] = b->start;
	else if ((curve_orient[3] == NEGATIVE_ORIENTATION) &&
	         (curve[3]->first->end != p[0][jmax]))
	    for (b = curve[3]->first, iy = 0; iy < jmax; iy++, b = b->next)
	        p[0][iy] = b->start;
	
	/* Make the triangles. */

	for (iy = 0; iy < jmax; iy++)
	{
	    for (ix = 0; ix < imax; ix++)
	    {
	        pt[0] = p[ix][iy];
	        pt[1] = p[ix+1][iy];
	        pt[2] = p[ix+1][iy+1];
	        pt[3] = p[ix][iy+1];

	        for (i = 0; i < 3; i++)
	            coords[i] = 0.5*(Coords(pt[0])[i] + Coords(pt[2])[i]);
	        midp = Point(coords);

	        for (i = 0; i < 4; i++)
	        {
	            ip1 = (orient == COUNTER_CLOCK) ? i : Next_m4(i);
	            ip2 = (orient == COUNTER_CLOCK) ? Next_m4(i): i;
	            tri[ix][iy][i] = make_tri(pt[ip1],pt[ip2],midp,NULL,NULL,
					      NULL,YES);
	            insert_tri_at_tail_of_list(tri[ix][iy][i],surf);
	        }
	    }
	}

	/* set tri neighbors */
	for (iy = 0; iy < jmax; iy++)
	{
	    for (ix = 0; ix < imax; ix++)
	    {
	        for (i = 0; i < 4; i++)
	        {
	            set_01_bdry(Boundary_tri(tri[ix][iy][i]),YES);

	            Tri_on_side01(tri[ix][iy][i]) = NULL;

	            if ((i == 0) && (iy != 0)) /* south */
	            {
	                Tri_on_side01(tri[ix][iy][i]) = tri[ix][iy-1][2];
	                Boundary_tri(tri[ix][iy][i]) = NO;
	                set_01_bdry(Boundary_tri(tri[ix][iy][i]),NO);
	            }
	            else if ((i == 1) && ix != (imax-1)) /* east */
	            {
	                Tri_on_side01(tri[ix][iy][i]) = tri[ix+1][iy][3];
	                Boundary_tri(tri[ix][iy][i]) = NO;
	                set_01_bdry(Boundary_tri(tri[ix][iy][i]),NO);
	            }
	            else if ((i == 2) && iy != (jmax-1)) /* north */
	            {
	                Tri_on_side01(tri[ix][iy][i]) = tri[ix][iy+1][0];
	                Boundary_tri(tri[ix][iy][i]) = NO;
	                set_01_bdry(Boundary_tri(tri[ix][iy][i]),NO);
	            }
	            else if ((i == 3) && ix != 0) /* west */
	            {
	                Tri_on_side01(tri[ix][iy][i]) = tri[ix-1][iy][1];
	                Boundary_tri(tri[ix][iy][i]) = NO;
	                set_01_bdry(Boundary_tri(tri[ix][iy][i]),NO);
	            }

	            ip1 = (orient == COUNTER_CLOCK) ? Next_m4(i) : Prev_m4(i);
	            ip2 = (orient == COUNTER_CLOCK) ? Prev_m4(i) : Next_m4(i);
	            Tri_on_side12(tri[ix][iy][i]) = tri[ix][iy][ip1];
	            Tri_on_side20(tri[ix][iy][i]) = tri[ix][iy][ip2];
	            set_12_bdry(Boundary_tri(tri[ix][iy][i]),NO);
	            set_20_bdry(Boundary_tri(tri[ix][iy][i]),NO);
	        }
	    }
	}

	/* label bdry curve */

	for (i = 0; i < 4; i++)
	    set_curve_bdry_flag(curve[i],gr);
	
	/* insert points in bond and link_tri_to_bond */

	/* bottom side */

	if (curve_orient[0] == POSITIVE_ORIENTATION)        /*same direction */
	{
	    b = curve[0]->first;
	    if (debugging("tri_obl"))
	    {
	        (void) printf("b_start=(%g,%g,%g),b_end=(%g,%g,%g)\n",
	                      Coords(b->start)[0],Coords(b->start)[1],
	                      Coords(b->start)[2],Coords(b->end)[0],
	                      Coords(b->end)[1],Coords(b->end)[2]);
	        (void) printf("start_node= (%g,%g,%g),",
	                      Coords(Q[0])[0],Coords(Q[0])[1],
	                      Coords(Q[0])[2]);
	        (void) printf("end_node=(%g,%g,%g)\n",
	                      Coords(Q[1])[0],Coords(Q[1])[1],
	                      Coords(Q[1])[2]);
	        (void) printf("curve_orient[0]=%d\n",curve_orient[0]);
	        (void) printf("\n");
	    }
	    if (Q[1] == b->end) /*assure not reinstall bond*/
	    {
	        for (ix = 1; ix < imax; ix++,b = b->next)
	            if (insert_point_in_bond(p[ix][0],b,curve[0]) !=
			FUNCTION_SUCCEEDED)
	            {
	                screen("ERROR in "
			       "oblique_planar_surface_triangulation(), "
		               "insert_point_in_bond() failed\n");
	                clean_up(ERROR);
	            }
	    }
	}
	else if (curve_orient[0] == NEGATIVE_ORIENTATION)/*opposite direction*/
	{
	    b = curve[0]->first;
	    if (debugging("tri_obl"))
	    {
	        (void) printf("b_start=(%g,%g,%g),b_end=(%g,%g,%g)\n",
	                      Coords(b->start)[0],Coords(b->start)[1],
	                      Coords(b->start)[2],Coords(b->end)[0],
	                      Coords(b->end)[1],Coords(b->end)[2]);
	        (void) printf("start_node= (%g,%g,%g),",
	                      Coords(Q[1])[0],Coords(Q[1])[1],
	                      Coords(Q[1])[2]);
	        (void) printf("end_node=(%g,%g,%g)\n",
	                      Coords(Q[0])[0],Coords(Q[0])[1],
	                      Coords(Q[0])[2]);
	        (void) printf("curve_orient[0]=%d\n",curve_orient[0]);
	        (void) printf("\n");
	    }
	    if (Q[0] == b->end)
	    {
	        for (ix = imax - 1; ix > 0; ix--,b = b->next)
	            if (insert_point_in_bond(p[ix][0],b,curve[0]) !=
			FUNCTION_SUCCEEDED)
	            {
	                screen("ERROR in "
			       "oblique_planar_surface_triangulation(), "
		               "insert_point_in_bond() failed\n");
	                clean_up(ERROR);
	            }
	    }
	}

	/* right side */

	if (curve_orient[1] == POSITIVE_ORIENTATION)
	{
	    b = curve[1]->first;
	    if (debugging("tri_obl"))
	    {
	        (void) printf("b_start=(%g,%g,%g),b_end=(%g,%g,%g)\n",
	                      Coords(b->start)[0],Coords(b->start)[1],
	                      Coords(b->start)[2],Coords(b->end)[0],
	                      Coords(b->end)[1],Coords(b->end)[2]);
	        (void) printf("start_node= (%g,%g,%g),",
	                      Coords(Q[1])[0],Coords(Q[1])[1],
	                      Coords(Q[1])[2]);
	        (void) printf("end_node=(%g,%g,%g)\n",
	                      Coords(Q[2])[0],Coords(Q[2])[1],
	                      Coords(Q[2])[2]);
	        (void) printf("curve_orient[1]=%d\n",curve_orient[1]);
	        (void) printf("\n");
	    }
	    if (Q[2] == b->end)
	    {
	        for (iy = 1; iy < jmax; iy++,b = b->next)
	            if (insert_point_in_bond(p[imax][iy],b,curve[1]) !=
			FUNCTION_SUCCEEDED)
	            {
	                screen("ERROR in "
			       "oblique_planar_surface_triangulation(), "
		               "insert_point_in_bond() failed\n");
	                clean_up(ERROR);
	            }
	    }
	}
	else if (curve_orient[1] == NEGATIVE_ORIENTATION)
	{
	    b = curve[1]->first;
	    if (debugging("tri_obl"))
	    {
	        (void) printf("b_start=(%g,%g,%g),b_end=(%g,%g,%g)\n",
	                      Coords(b->start)[0],Coords(b->start)[1],
	                      Coords(b->start)[2],Coords(b->end)[0],
	                      Coords(b->end)[1],Coords(b->end)[2]);
	        (void) printf("start_node= (%g,%g,%g),",
	                      Coords(Q[2])[0],Coords(Q[2])[1],
	                      Coords(Q[2])[2]);
	        (void) printf("end_node=(%g,%g,%g)\n",
	                      Coords(Q[1])[0],Coords(Q[1])[1],
	                      Coords(Q[1])[2]);
	        (void) printf("curve_orient[1]=%d\n",curve_orient[1]);
	        (void) printf("\n");
	    }
	    if (Q[1] == b->end)
	    {
	        for (iy = jmax - 1; iy > 0; iy--,b = b->next)
	            if (insert_point_in_bond(p[imax][iy],b,curve[1]) !=
			FUNCTION_SUCCEEDED)
	            {
	                screen("ERROR in "
			       "oblique_planar_surface_triangulation(), "
		               "insert_point_in_bond() failed\n");
	                clean_up(ERROR);
	            }
	    }
	}


	/* top side */
	if (curve_orient[2] == NEGATIVE_ORIENTATION)
	{
	    b = curve[2]->first;
	    if (debugging("tri_obl"))
	    {
	        (void) printf("b_start=(%g,%g,%g),b_end=(%g,%g,%g)\n",
	                      Coords(b->start)[0],Coords(b->start)[1],
	                      Coords(b->start)[2],Coords(b->end)[0],
	                      Coords(b->end)[1],Coords(b->end)[2]);
	        (void) printf("start_node= (%g,%g,%g),",
	                      Coords(Q[3])[0],Coords(Q[3])[1],
	                      Coords(Q[3])[2]);
	        (void) printf("end_node=(%g,%g,%g)\n",
	                      Coords(Q[2])[0],Coords(Q[2])[1],
	                      Coords(Q[2])[2]);
	        (void) printf("curve_orient[2]=%d\n",curve_orient[2]);
	        (void) printf("\n");
	    }
	    if (Q[2] == b->end)
	    {
	        for (ix = 1; ix < imax; ix++,b = b->next)
	            if (insert_point_in_bond(p[ix][jmax],b,curve[2]) !=
			FUNCTION_SUCCEEDED)
	            {
	                screen("ERROR in "
			       "oblique_planar_surface_triangulation(), "
		               "insert_point_in_bond() failed\n");
	                clean_up(ERROR);
	            }
	    }
	}
	else if (curve_orient[2] == POSITIVE_ORIENTATION)
	{
	    b = curve[2]->first;
	    if (debugging("tri_obl"))
	    {
	        (void) printf("b_start=(%g,%g,%g),b_end=(%g,%g,%g)\n",
	                      Coords(b->start)[0],Coords(b->start)[1],
	                      Coords(b->start)[2],Coords(b->end)[0],
	                      Coords(b->end)[1],Coords(b->end)[2]);
	        (void) printf("start_node= (%g,%g,%g),",
	                      Coords(Q[2])[0],Coords(Q[2])[1],
	                      Coords(Q[2])[2]);
	        (void) printf("end_node=(%g,%g,%g)\n",
	                      Coords(Q[3])[0],Coords(Q[3])[1],
	                      Coords(Q[3])[2]);
	        (void) printf("curve_orient[2]=%d\n",curve_orient[2]);
	        (void) printf("\n");
	    }
	    if (Q[3] == b->end)
	    {
	        for (ix = imax - 1; ix > 0; ix--,b = b->next)
	            if (insert_point_in_bond(p[ix][jmax],b,curve[2]) !=
			FUNCTION_SUCCEEDED)
	            {
	                screen("ERROR in "
			       "oblique_planar_surface_triangulation(), "
		               "insert_point_in_bond() failed\n");
	                clean_up(ERROR);
	            }
	    }
	}

	/* left side */

	if (curve_orient[3] == NEGATIVE_ORIENTATION)
	{
	    b = curve[3]->first;
	    if (debugging("tri_obl"))
	    {
	        (void) printf("b_start=(%g,%g,%g),b_end=(%g,%g,%g)\n",
	                      Coords(b->start)[0],Coords(b->start)[1],
	                      Coords(b->start)[2],Coords(b->end)[0],
	                      Coords(b->end)[1],Coords(b->end)[2]);
	        (void) printf("start_node= (%g,%g,%g),",
	                      Coords(Q[0])[0],Coords(Q[0])[1],
	                      Coords(Q[0])[2]);
	        (void) printf("end_node=(%g,%g,%g)\n",
	                      Coords(Q[3])[0],Coords(Q[3])[1],
	                      Coords(Q[3])[2]);
	        (void) printf("curve_orient[3]=%d\n",curve_orient[3]);
	        (void) printf("\n");
	    }
	    if (Q[3] == b->end)
	    {
	        for (iy = 1; iy < jmax; iy++,b = b->next)
	            if (insert_point_in_bond(p[0][iy],b,curve[3]) !=
			FUNCTION_SUCCEEDED)
	            {
	                screen("ERROR in "
			       "oblique_planar_surface_triangulation(), "
		               "insert_point_in_bond() failed\n");
	                clean_up(ERROR);
	            }
	    }
	}
	else if (curve_orient[3] == POSITIVE_ORIENTATION)
	{
	    b = curve[3]->first;
	    if (debugging("tri_obl"))
	    {
	        (void) printf("b_start=(%g,%g,%g),b_end=(%g,%g,%g)\n",
	                      Coords(b->start)[0],Coords(b->start)[1],
	                      Coords(b->start)[2],Coords(b->end)[0],
	                      Coords(b->end)[1],Coords(b->end)[2]);
	        (void) printf("start_node= (%g,%g,%g),",
	                      Coords(Q[3])[0],Coords(Q[3])[1],
	                      Coords(Q[3])[2]);
	        (void) printf("end_node=(%g,%g,%g)\n",
	                      Coords(Q[0])[0],Coords(Q[0])[1],
	                      Coords(Q[0])[2]);
	        (void) printf("curve_orient[3]=%d\n",curve_orient[3]);
	        (void) printf("\n");
	    }
	    if (Q[0] == b->end)
	    {
	        for (iy = jmax - 1; iy > 0; iy--,b = b->next)
	            if (insert_point_in_bond(p[0][iy],b,curve[3]) !=
			FUNCTION_SUCCEEDED)
	            {
	                screen("ERROR in "
			       "oblique_planar_surface_triangulation(), "
		               "insert_point_in_bond() failed\n");
	                clean_up(ERROR);
	            }
	    }
	}


	/* bottom side */
	if (curve_orient[0] == POSITIVE_ORIENTATION)
	{
	    b = curve[0]->first;
	    for (ix = 0; ix < imax; ix++, b = b->next)
	    {
	        (void) link_tri_to_bond(NULL,tri[ix][0][0],surf,b,curve[0]);
	    }
	}
	else if (curve_orient[0] == NEGATIVE_ORIENTATION)
	{
	    b = curve[0]->first;
	    for (ix = imax - 1; ix >= 0; ix--,b = b->next)
	    {
	        (void) link_tri_to_bond(NULL,tri[ix][0][0],surf,b,curve[0]);
	    }
	}

	/* right side */
	if (curve_orient[1] == POSITIVE_ORIENTATION)
	{
	    b = curve[1]->first;
	    for (iy = 0; iy < jmax; iy++,b = b->next)
	    {
	        (void) link_tri_to_bond(NULL,tri[imax-1][iy][1],surf,
					b,curve[1]);
	    }
	}
	else if (curve_orient[1] == NEGATIVE_ORIENTATION)
	{
	    b = curve[1]->first;
	    for (iy = jmax-1; iy >= 0; iy--,b = b->next)
	    {
	        (void) link_tri_to_bond(NULL,tri[imax-1][iy][1],surf,
					b,curve[1]);
	    }
	}

	/* top side */
	if (curve_orient[2] == NEGATIVE_ORIENTATION)
	{
	    b = curve[2]->first;
	    for (ix = 0; ix < imax; ix++,b = b->next)
	    {
	        (void) link_tri_to_bond(NULL,tri[ix][jmax-1][2],surf,
					b,curve[2]);
	    }
	}
	else if (curve_orient[2] == POSITIVE_ORIENTATION)
	{
	    b = curve[2]->first;
	    for (ix = imax-1; ix >= 0; ix--,b = b->next)
	    {
	        (void) link_tri_to_bond(NULL,tri[ix][jmax-1][2],surf,
					b,curve[2]);
	    }
	}

	/* left side */
	if (curve_orient[3] == NEGATIVE_ORIENTATION)
	{
	    b = curve[3]->first;
	    for (iy = 0; iy < jmax; iy++,b = b->next)
	    {
	        (void) link_tri_to_bond(NULL,tri[0][iy][3],surf,b,curve[3]);
	    }
	}
	else if (curve_orient[3] == POSITIVE_ORIENTATION)
	{
	    b = curve[3]->first;
	    for (iy = jmax-1; iy >= 0; iy--,b = b->next)
	    {
	        (void) link_tri_to_bond(NULL,tri[0][iy][3],surf,b,curve[3]);
	    }
	}
	    
	surf->interface->num_points = surf->interface->num_points+ 
	                              (imax+1)*(jmax+1)+imax*jmax;
	free(p);
	free(tri);
}		/*end oblique_planar_surface_triangulation*/



LOCAL void normal_of_plane_surface(
	SURFACE *surf,
	double   *nor)
{
	CURVE **c;
	BOND  *b;
	FT_POINT *p;
	double area, p_cnt[MAXD], p1[MAXD], p2[MAXD];
	double pnor[3], nnor[3];
	int   i, num, dim;

	dim = surf->interface->dim;
	/* find the center point */
	for (i = 0; i < dim; i++)
	    p_cnt[i] = 0.0;

	for (c = surf->pos_curves, num = 0; c && * c; c++, num++)
	{
	    for (b = (*c)->first; b; b = b->next)
	    {
	        p = b->start;
	        for (i = 0; i < dim; i++)
	            p_cnt[i] += Coords(p)[i];
	    }
	}
	for (c = surf->neg_curves; c && * c; c++, num++)
	{
	    for (b = (*c)->first; b; b = b->next)
	    {
	        p = b->end;
	        for (i = 0; i < dim; i++)
	            p_cnt[i] += Coords(p)[i];
	    }
	}

	for (i = 0; i < dim; i++)
	{
	    pnor[i] = 0.0;
	    nnor[i] = 0.0;
	    p_cnt[i] /= num;
	}

	for (c = surf->pos_curves; c && * c; c++, num++)
	{
	    for (b = (*c)->first; b; b = b->next)
	    {
	        for (i = 0; i < dim; i++)
	        {
	            p1[i] = Coords(b->start)[i] - p_cnt[i];
	            p2[i] = Coords(b->end)[i] - p_cnt[i];
	        }
	        pnor[0] += p1[1]*p2[2] - p1[2]*p2[1];
	        pnor[1] += p1[2]*p2[0] - p1[0]*p2[2];
	        pnor[2] += p1[0]*p2[1] - p1[1]*p2[0];
	    }
	}

	for (c = surf->neg_curves; c && * c; c++, num++)
	{
	    for (b = (*c)->first; b; b = b->next)
	    {
	        for (i = 0; i < dim; i++)
	        {
	            p1[i] = Coords(b->end)[i] - p_cnt[i];
	            p2[i] = Coords(b->start)[i] - p_cnt[i];
	        }
	        nnor[0] += p1[1]*p2[2] - p1[2]*p2[1];
	        nnor[1] += p1[2]*p2[0] - p1[0]*p2[2];
	        nnor[2] += p1[0]*p2[1] - p1[1]*p2[0];
	    }
	}
	nor[0] = pnor[0] + nnor[0];
	nor[1] = pnor[1] + nnor[1];
	nor[2] = pnor[2] + nnor[2];
	area = mag_vector(nor,dim);
	for (i = 0; i < dim; i++)
	    nor[i] /= area;

	return;
}                /*end normal_of_plane_surface*/


LOCAL ORIENTATION is_point_a_node_of_curve(
	CURVE                *curve,
	FT_POINT                *p)
{
	ORIENTATION which_point = ORIENTATION_NOT_SET;
	FT_POINT       *p1,*p2;

	p1 = curve->start->posn;
	p2 = curve->end->posn;

	/* if p is curve->start return 1 */
        /* if p is curve->end   return 2 */

	if (p1 == p)
	    which_point = POSITIVE_ORIENTATION;
	else if (p2 == p)
	    which_point = NEGATIVE_ORIENTATION;

	return which_point;
}                /*end is_point_a_node_of_curve*/

LOCAL CURVE *id_curve_by_points(
	SURFACE     *surf,
	FT_POINT       *ps,
	FT_POINT       *pe,
	ORIENTATION *orientation)
{
	ORIENTATION x = ORIENTATION_NOT_SET, y = ORIENTATION_NOT_SET;
	CURVE       **c1, *ans_c = NULL;

	for (c1 = surf->pos_curves; c1 && *c1; c1++)
	{
	    x = is_point_a_node_of_curve(*c1,ps);
	    y = is_point_a_node_of_curve(*c1,pe);

	    if ((x==POSITIVE_ORIENTATION) && (y==NEGATIVE_ORIENTATION))
	    { 
	        /* ps is the start node of curve, pe the end */
	        *orientation = POSITIVE_ORIENTATION;
	        ans_c = *c1;
	        break;
	    }
	    else if ((x==NEGATIVE_ORIENTATION) && (y==POSITIVE_ORIENTATION))
	    {
	        *orientation = NEGATIVE_ORIENTATION;
	        ans_c = *c1;
	        break;
	    }
	}

	for (c1 = surf->neg_curves; c1 && *c1; c1++)
	{
	    x = is_point_a_node_of_curve(*c1,ps);
	    y = is_point_a_node_of_curve(*c1,pe);

	    if ((x==POSITIVE_ORIENTATION) && (y==NEGATIVE_ORIENTATION))
	    { 
	        /* ps is the start node of curve, pe the end */
	        *orientation = POSITIVE_ORIENTATION;
	        ans_c = *c1;
	        break;
	    }
	    else if ((x==NEGATIVE_ORIENTATION) && (y==POSITIVE_ORIENTATION))
	    {
	        *orientation = NEGATIVE_ORIENTATION;
	        ans_c = *c1;
	        break;
	    }
	}
	return ans_c;
}                /*end id_curve_by_points*/


LOCAL         void set_curve_bdry_flag(
	CURVE     *curve,
	RECT_GRID *gr)
{
	FT_POINT *p3,*p4;
	double VL[3],VU[3];
	double x2,y2,z2,x3,y3,z3;
	int   m1,m2,m3;
	int   i;

	for (i = 0; i < 3; i++)
	{
	    VL[i] = gr->VL[i];
	    VU[i] = gr->VU[i];
	}

	m1 = 0;
	m2 = 0;
	m3 = 0;

	p3 = curve->start->posn;
	p4 = curve->end->posn;

	x2 = Coords(p3)[0];
	y2 = Coords(p3)[1];
	z2 = Coords(p3)[2];

	x3 = Coords(p4)[0];
	y3 = Coords(p4)[1];
	z3 = Coords(p4)[2];

	if ((x2 == x3) && (x2 == VL[0]))
	    m1 = 1;
	else if ((x2 == x3) && (x2 == VU[0]))
	    m1 = 2;

	if ((y2 == y3) && (y2 == VL[1]))
	    m2 = 1;
	else if ((y2 == y3) && (y2 == VU[1]))
	    m2 = 2;

	if ((z2 == z3) && (z2 == VL[0]))
	    m3 = 1;
	else if ((z2 == z3) && (z2 == VU[0]))
	    m3 = 2;

	set_bdry_side(Boundary(curve),0,m1);
	set_bdry_side(Boundary(curve),1,m2);
	set_bdry_side(Boundary(curve),2,m3);
}                /*end set_curve_bdry_flags*/

LOCAL double value_x(
	double *coords,
	FT_POINT *corner,
	double *nor)
{
	double x,y,z;
	double a,b,c;
	double x3,y3,z3;

	z = coords[2];
	y = coords[1];

	a = nor[0];
	b = nor[1];
	c = nor[2];

	x3 = Coords(corner)[0];
	y3 = Coords(corner)[1];
	z3 = Coords(corner)[2];

	if (a == 0.0)
	{
	    screen("ERROR in value_x(), "
	           "normal direction error in x_value()\n");
	    clean_up(ERROR);
	}
	x = x3 + (c*(z3 - z) + b*(y3 - y))/a;

	return x;
}                /*end value_x*/


LOCAL double value_y(
	double *coords,
	FT_POINT *corner,
	double *nor)
{
	double x,y,z;
	double a,b,c;
	double x3,y3,z3;

	x = coords[0];
	z = coords[2];

	a = nor[0];
	b = nor[1];
	c = nor[2];

	x3 = Coords(corner)[0];
	y3 = Coords(corner)[1];
	z3 = Coords(corner)[2];

	if (b == 0.0)
	{
	    screen("ERROR in value_y(), "
	           "normal direction error in y_value()\n");
	    clean_up(ERROR);
	}
	y = y3 + (a*(x3 - x) + c*(z3 - z))/b;

	return y;
}                /*end value_y*/

LOCAL double value_z(
	double *coords,
	FT_POINT *corner,
	double *nor)
{
	double x,y,z;
	double a,b,c;
	double x3,y3,z3;

	x = coords[0];
	y = coords[1];

	a = nor[0];
	b = nor[1];
	c = nor[2];

	x3 = Coords(corner)[0];
	y3 = Coords(corner)[1];
	z3 = Coords(corner)[2];

	if (c == 0.0)
	{
	    screen("ERROR in value_z(), "
	           "normal direction error in z_value()\n");
	    clean_up(ERROR);
	}
	z = z3 + (a*(x3 - x) + b*(y3 - y))/c;

	return z;
}                /*end value_z*/

#endif /* defined(THREED) */
