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
*
*
* 		ixgraph.c
*
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	J. D. Pinezich
*
*
*/


#include <intfc/iloc.h>

LOCAL	void 	xgraph_curve_within_range(FILE*,CURVE*,double*,double);
LOCAL   void    xgraph_surface_tris(const char*,const char*,SURFACE*,
				    const COORDINATE_PLANE);
LOCAL   void 	xgraph_grid(const char*,RECT_GRID*,
			    const COORDINATE_PLANE);
LOCAL   void 	xgraph_curve_file(const char*,const char*,CURVE*,
				  const COORDINATE_PLANE);

EXPORT	void xgraph_2d_intfc(
	const char *filename,
	INTERFACE *intfc)
{
	double center[MAXD];
	double radius;
	int i,dim;
	RECT_GRID *gr = &topological_grid(intfc);
	
	dim = intfc->dim;
	radius = 0.0;
	for (i = 0; i < dim; ++i)
	{
	    center[i] = 0.5*(gr->L[i] + gr->U[i]);
	    if (radius < 0.75*(gr->U[i] - gr->L[i]))
		radius = 0.75*(gr->U[i] - gr->L[i]);
	}
	xgraph_2d_intfc_within_range(filename,intfc,center,radius,NO);
}	/* end xgraph_2d_intfc */
	
EXPORT	void xgraph_2d_reflection(
	const char *filename,
        INTERFACE *intfc,
	double *coords,
	double *coords_bdry,
	double *coords_ref,
	double *nor)
{
	FILE *file;
	double dist,radius;
	double h = topological_grid(intfc).h[0];

	dist = distance_between_positions(coords,coords_bdry,2);
	radius = (dist/h < 2.0) ? 2.0*h : dist;
	xgraph_2d_intfc_within_range(filename,intfc,coords_bdry,radius,NO);
	file = fopen(filename,"a");
	fprintf(file,"\n\"Reflection\"\n");
	fprintf(file,"%20.14f  %20.14f\n",coords[0],coords[1]);
	fprintf(file,"%20.14f  %20.14f\n",coords_bdry[0],coords_bdry[1]);
	fprintf(file,"%20.14f  %20.14f\n",coords_ref[0],coords_ref[1]);
	fprintf(file,"\n\"Normal\"\n");
	fprintf(file,"%20.14f  %20.14f\n",coords_bdry[0],coords_bdry[1]);
	fprintf(file,"%20.14f  %20.14f\n",coords_bdry[0]+nor[0]*dist,
				 	coords_bdry[1]+nor[1]*dist);
	fclose(file);
	
}	/* end xgraph_2d_reflection */

EXPORT	void xgraph_2d_intfc_within_range(
	const char *filename,
	INTERFACE *intfc,
	double *center,
	double radius,
	boolean draw_grid)
{
	FILE *file;
	CURVE **c;
	RECT_GRID *gr = &topological_grid(intfc);
	double *h = gr->h;
	int i,j,nrad = irint(radius/h[0]);

	file = fopen(filename,"w");
	for (c = intfc->curves; c && *c; ++c)
	    xgraph_curve_within_range(file,*c,center,radius);
	if (draw_grid)
	{
	    fprintf(file,"\n");
	    for (i = -nrad; i <= nrad; ++i)
	    {
	    	for (j = -nrad; j <= nrad; ++j)
	    	{
		    fprintf(file,"%20.14f %20.14f\n",center[0]+i*h[0],
					center[1]+j*h[1]);
	    	}
	        fprintf(file,"\n");
	    }
	    for (j = -nrad; j <= nrad; ++j)
	    {
	    	for (i = -nrad; i <= nrad; ++i)
	    	{
		    fprintf(file,"%20.14f %20.14f\n",center[0]+i*h[0],
					center[1]+j*h[1]);
	    	}
	        fprintf(file,"\n");
	    }
	}
	fclose(file);
}

LOCAL	void xgraph_curve_within_range(
	FILE *file,
	CURVE *c,
	double *center,
	double radius)
{
	FT_POINT *p;
	BOND *b;
	boolean prev_p_within_range = NO;
        p = c->start->posn;
	if (point_within_range(p,center,radius,2))
	{
            (void) fprintf(file,"%20.14f %20.14f\n",Coords(p)[0],Coords(p)[1]);
	    prev_p_within_range = YES;
	}
        for (b = c->first; b != NULL; b = b->next)
        {
	    p = b->end;
	    if (point_within_range(p,center,radius,2))
	    {
		if (!prev_p_within_range) 
		    (void) fprintf(file,"\n");
            	(void) fprintf(file,"%20.14f %20.14f\n",Coords(p)[0],
				Coords(p)[1]);
		prev_p_within_range = YES;
	    }
	    else
	    	prev_p_within_range = NO;
        }
	(void) fprintf(file,"\n");
}

EXPORT 	void	xgraph_interface_surfaces(
	const char             *dname,
	const char             *fname,
	INTERFACE 	       *intfc,
	const COORDINATE_PLANE proj)
{
  	SURFACE **s;
	int 	i = 0;
	char 	sname[256];

	if (intfc->dim < 3)
	{
	    return;
	}

	s = intfc->surfaces;
	for ( ; s && *s; s++, i++)
        {
	    (void) sprintf(sname,"%s_surf%d",fname,i);
	    xgraph_surface_tris(dname,sname,*s,proj);
	}
}		/*end xgraph_interface_surfaces*/

EXPORT 	FILE 	*xgraph_file_open(
	const char             *dname,
	const char             *fname,
	const COORDINATE_PLANE proj)
{
	FILE	   *xgraph;
	char	   path[256];
	static const char *x_axis[3] = {"Y-axis","X-axis","X-axis"};
	static const char *y_axis[3] = {"Z-axis","Z-axis","Y-axis"};

	if (create_directory(dname,YES) == FUNCTION_FAILED)
	{
	    screen("WARNING in xgraph_file_open(),",
		   "directory %s doesn't exist and can't be created.\n",dname);
	    clean_up(ERROR);
	}
	(void) sprintf(path,"%s/%s.xg",dname,fname);
	xgraph = fopen(path,"w");
	(void) fprintf(xgraph,"TitleText: %s\n",path);
	(void) fprintf(xgraph,"LargePixels: 1\n");
	(void) fprintf(xgraph,"Background: white\n");
	(void) fprintf(xgraph,"XUnitText: %s\n",x_axis[proj]);
	(void) fprintf(xgraph,"YUnitText: %s\n\n",y_axis[proj]);
	return xgraph;
}		/*end xgraph_file_open*/

EXPORT 	void 	xgraph_tris_list(
	FILE 	               *file,
	TRI 	               **tris,
	int 	               num_tris,
	const COORDINATE_PLANE proj)
{
	int i;

	for (i = 0; i < num_tris; i++)
	    xgraph_tri(file,tris[i],proj);
	(void) printf("xgraph_tris_list()-->%d triangles written\n",i);
}		/*end xgraph_tris_list*/

EXPORT 	void 	xgraph_tri(
	FILE 	               *file,
	TRI 	               *tri,
	const COORDINATE_PLANE proj)
{
	int i,j;

	(void) fprintf(file,"move ");
	for (i = 0; i < 4; i++)
	{
	    for (j = 0; j < 3; j++)
	    {
		if (j != proj)
		    (void) fprintf(file,"%g ",Coords(Point_of_tri(tri)[i%3])[j]);
	    }
	    (void) fprintf(file,"\n");
	}
}		/*end xgraph_tri*/

LOCAL  	void 	xgraph_surface_tris(
	const char             *dname,
	const char             *fname,
	SURFACE	               *surf,
	const COORDINATE_PLANE proj)
{
  	TRI 	*t;
	FILE	*xgraph;
	int	i,j,num = 0;
	char	path[256];

	if (create_directory(dname,YES) == FUNCTION_FAILED)
	{
	    screen("WARNING in xgraph_surface_tris(), ",
		   "directory %s doesn't exist and can't be created.\n",dname);
	    return;
	}

	(void) sprintf(path,"%s/%s.xg",dname,fname);
	xgraph = fopen(path,"w");
	(void) fprintf(xgraph,"\"%s\"\n",fname);
	for (t = surf->_First_tri.tri.next; num < surf->num_tri;
	     t = t->next,num++)
	{
	    (void) fprintf(xgraph,"move ");
	    for (i = 0; i < 4; i++)
	    {
	        for (j = 0; j < 3; j++)
		{
		    if (j != proj)
		        (void) fprintf(xgraph,"%g ",
				       Coords(Point_of_tri(t)[ i%3 ])[j]);
		}
		(void) fprintf(xgraph,"\n");
	    }
	}
	fclose(xgraph);
} 		/*end xgraph_surface_tris*/

EXPORT 	void 	xgraph_new_data_set(
	FILE 	*file)
{
  	(void) fprintf(file,"\n"); /* Provides more colors in xgraph display */
}		/*end xgraph_new_data_set*/

EXPORT 	void 	xgraph_point(
	FILE 	               *file,
	double 	               *p,
	const COORDINATE_PLANE proj,
	const char 	       *name)
{
	int j,k;
	if (name != "")
	{
	    (void) fprintf(file,"\"%s\"\n",name);
	}
	(void) fprintf(file,"move ");
	for (k = 0; k < 2; k++)
	{
	    for (j = 0; j < 3; j++)
            {
		if (j != proj)
		    (void) fprintf(file,"%g ",p[j]);
	    }
	    (void) fprintf(file,"\n");
	}
}		/*end xgraph_point*/

EXPORT 	void 	xgraph_affine_vector(
	FILE 	               *file,
	double 	               *start,
	double 	               *v,
	const COORDINATE_PLANE proj,
	const char 	       *msg)
{
	int j,k;
	if (msg != "") (void) fprintf(file,"\"%s\"\n",msg);
	(void) fprintf(file,"move ");
	for (k = 0; k < 2; k++)
	{
	    for (j = 0; j < 3; j++)
	    {
		if (j != proj)
		    (void) fprintf(file,"%g ",start[j]);
	    }
	    (void) fprintf(file,"\n");
	}
	for (k = 0; k < 2; k++)
        {
	    for (j = 0; j < 3; j++)
            {
		if (j != proj)
		    (void) fprintf(file,"%g ",start[j]+v[j]);
	    }
	    (void) fprintf(file,"\n");
	}
}		/*end xgraph_affine_vector*/

EXPORT 	void 	xgraph_line_segment(
	FILE 	               *file,
	double 	               *start,
	double 	               *end,
	const COORDINATE_PLANE proj,
	const char 	       *msg)
{
	int j,k;
	if (msg != "")
	    (void) fprintf(file,"\"%s\"\n",msg);
	(void) fprintf(file,"move ");
	for (k = 0; k < 2; k++)
        {
	    for (j = 0; j < 3; j++)
            {
		if (j != proj)
		    (void) fprintf(file,"%g ",start[j]);
            }
	    (void) fprintf(file,"\n");
        }
	for (k = 0; k < 2; k++)
        {
	    for (j = 0; j < 3; j++)
            {
		if (j != proj)
		    (void) fprintf(file,"%g ",end[j]);
            }
	    (void) fprintf(file,"\n");
	}
}		/*end xgraph_line_segment*/

EXPORT 	void	xgraph_interface_nodes(
	const char             *dname,
	const char             *fname,
	INTERFACE 	       *intfc,
	const COORDINATE_PLANE proj)
{
  	NODE 	**n;
	int 	i=0,j,k;
	char 	path[256];
	FILE 	*xgraph;

	if (create_directory(dname,YES) == FUNCTION_FAILED)
	{
	    screen("WARNING in xgraph_interface_nodes(),"
		   "directory %s doesn't exist and can't be created\n",dname);
	    return;
        }

	(void) sprintf(path,"%s/%s_nodes.xg",dname,fname);
	xgraph = fopen(path,"w");
	(void) fprintf(xgraph,"\"%s\"\n",fname);

	for (n = intfc->nodes; n && *n; n++,i++)
	{
	    (void) fprintf(xgraph,"move ");
	    for (k = 0; k < 2; k++)
	    {
		for (j = 0; j < 3; j++)
		{
		    if (j != proj)
		        (void) fprintf(xgraph,"%g ",Coords((*n)->posn)[j]);
	        }
		(void) fprintf(xgraph,"\n");
	    }
	}
	(void) fprintf(xgraph,"\n");
}		/*end xgraph_interface_nodes*/

EXPORT  void 	xgraph_interface_curves(
	const char             *dname,
	const char             *fname,
	INTERFACE 	       *intfc,
	const COORDINATE_PLANE proj)
{
  	CURVE 	**c;
	int 	i=0;
	char 	sname[256];

	for (c = intfc->curves; c && *c; c++,i++)
        {
	    (void) sprintf(sname,"%s_curv%d",fname,i);
	    xgraph_curve_file(dname,sname,*c,proj);
	}
}		/*end xgraph_interface_curves*/

LOCAL  	void 	xgraph_curve_file(
	const char             *dname,
	const char             *fname,
	CURVE 	               *c,
	const COORDINATE_PLANE proj)
{
  	BOND 	*b;
	FT_POINT	*start,*end;
  	FILE	*xgraph;
	int	j;
	char	path[256];

	if (create_directory(dname,YES) == FUNCTION_FAILED)
	{
	    screen("WARNING in xgraph_curve_file(),"
		   "directory %s doesn't exist and can't be created.\n",dname);
	    return;
	}

	(void) sprintf(path,"%s/%s.xg",dname,fname);

	xgraph = fopen(path,"w");
	(void) fprintf(xgraph,"\"%s\"\n",fname);

	start = c->start->posn;
	for (j = 0; j < 3; j++)
        {
	    if (j != proj)
	        (void) fprintf(xgraph,"%g ",Coords(start)[j]);
	}
	(void) fprintf(xgraph,"\n");

	for (b = c->first; b != c->last; b = b->next)
        {
	    for (j = 0; j < 3; j++)
            {
		if (j != proj)
		    (void) fprintf(xgraph,"%g ",Coords(b->end)[j]);
	    }
	    (void) fprintf(xgraph,"\n");
	}

	end = c->end->posn;
	for (j = 0; j < 3; j++)
	{
	    if (j != proj)
	        (void) fprintf(xgraph,"%g ",Coords(end)[j]);
	}
	(void) fprintf(xgraph,"\n\n");
	fclose(xgraph);
}		/*end xgraph_curve_file*/

EXPORT 	void 	xgraph_curve(
	FILE 	               *file,
	CURVE 	               *c,
	const COORDINATE_PLANE proj)
{
  	BOND 	*b;
	FT_POINT	*start,*end;
	int	j;


	start = c->start->posn;
	for (j = 0; j < 3; j++)
	{
	    if (j != proj)
	        (void) fprintf(file,"%g ",Coords(start)[j]);
	}
	(void) fprintf(file,"\n");

	for (b = c->first; b != c->last; b = b->next)
        {
	    for (j = 0; j < 3; j++)
	    {
		if (j != proj)
		    (void) fprintf(file,"%g ",Coords(b->end)[j]);
	    }
	    (void) fprintf(file,"\n");
	}

	end = c->end->posn;
	for (j = 0; j < 3; j++)
        {
	    if (j != proj)
	        (void) fprintf(file,"%g ",Coords(end)[j]);
	}
	(void) fprintf(file,"\n\n");
}		/*end xgraph_curve*/

EXPORT 	void 	xgraph_RECT_GRID(
	const char *dname,
	RECT_GRID  *rect_grid)
{
 	char fname[100];

	if (create_directory(dname,YES) == FUNCTION_FAILED)
        {
	    screen("WARNING in xgraph_RECT_GRID(),"
		   "directory %s doesn't exist and can't be created.\n",dname);
	    return;
	}
	(void) sprintf(fname,"%s/XY",dname);
	xgraph_grid(fname,rect_grid,XY_PLANE);

	if (rect_grid->dim > 2)
	    return;
	(void) sprintf(fname,"%s/YZ",dname);
	xgraph_grid(fname,rect_grid,YZ_PLANE);
	(void) sprintf(fname,"%s/XZ",dname);
	xgraph_grid(fname,rect_grid,XZ_PLANE);
}		/*end xgraph_RECT_GRID*/

LOCAL  	void 	xgraph_grid(
	const char 	       *fname,
	RECT_GRID 	       *grid,
	const COORDINATE_PLANE proj)
{
  	int	   i,j;
	FILE	   *xgraph;
	double	   x_l,x_u,y_l,y_u,x,y;
	int	   x_n,y_n;
	double	   delta;
	int	   map[3][2] = {{1,2},
			        {0,2},
			        {0,1}};
	char	   file[100],tag[100];
	static const char *x_axis[3] = {"Y-axis","X-axis","X-axis"};
	static const char *y_axis[3] = {"Z-axis","Z-axis","Y-axis"};

	for (i = 0; i < 2; i++)
	{
	    if (i == 0)
	    {
	        (void) sprintf(file,"%s_REAL.xg",fname);
		(void) sprintf(tag,"REAL");
		x_u = grid->U[map[proj][0]];
		x_l = grid->L[map[proj][0]];
		x_n = grid->gmax[map[proj][0]];
		y_u = grid->U[map[proj][1]];
		y_l = grid->L[map[proj][1]];
		y_n = grid->gmax[map[proj][1]];
	    }
	    else
	    {
	        (void) sprintf(file,"%s_VIRT.xg",fname);
		(void) sprintf(tag,"VIRT");
		x_u = grid->VU[map[proj][0]];
		x_l = grid->VL[map[proj][0]];
		x_n = grid->gmax[map[proj][0]]+
		    grid->lbuf[map[proj][0]]+
		    grid->ubuf[map[proj][0]];
		y_u = grid->VU[map[proj][1]];
		y_l = grid->VL[map[proj][1]];
		y_n = grid->gmax[map[proj][1]]+
		    grid->lbuf[map[proj][1]]+
		    grid->ubuf[map[proj][1]];
	    }

	    xgraph = fopen(file,"w");
	    (void) fprintf(xgraph,"TitleText: %s\n",fname);
	    (void) fprintf(xgraph,"LargePixels: 1\n");
	    (void) fprintf(xgraph,"Background: white\n");
	    (void) fprintf(xgraph,"XUnitText: %s\n",x_axis[proj]);
	    (void) fprintf(xgraph,"YUnitText: %s\n",y_axis[proj]);
	    (void) fprintf(xgraph,"\"%s\"\n",tag);

	    delta = (x_u-x_l)/x_n;
	    for (j = 0; j <= x_n; j++)
	    {
	        x = x_l+j*delta;
		(void) fprintf(xgraph,"move %g %g\n     %g %g\n",x,y_l,x,y_u);
	    }
	    delta = (y_u-y_l)/y_n;
	    for (j = 0; j <= y_n; j++)
	    {
	        y = y_l+j*delta;
		(void) fprintf(xgraph,"move %g %g\n     %g %g\n",x_l,y,x_u,y);
	    }
	    fclose(xgraph);
	}
}		/*end xgraph_grid*/

EXPORT	boolean point_within_range(
	FT_POINT *p,
	double *center,
	double radius,
	int dim)
{
	int i;
	double dist;
	dist = 0.0;
	for (i = 0; i < dim; ++i)
	{
	    dist += sqr(Coords(p)[i] - center[i]);
	}
	dist = sqrt(dist);
	return (dist < radius) ? YES : NO;
}
