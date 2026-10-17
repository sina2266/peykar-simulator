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
*			plotdecs.h
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	MACRO DEFINITIONS FOR SIG, AN INTERMEDIATE PLOT LANGUAGE
*
*			Oliver A. McBryan
*
*
*	All of the coordinate arguments listed below are floats
*	with the exception of the arguments to viewport() and
*	magnify().
*
*
*	PLOTFILE			File pointer to plotting file
*	openpl()			Must be called before any plotting
*	closepl()			Terminates plotting session
*
*
*	window(x1,y1,x2,y2) 		Set User Coordinate Clipping Window
*	viewport(fx1,fg1,fx2,fy2)	Set Fractional Viewing Screen Window 
*						in [0.-1.] * [0.-1.]
*	magnify(xl,yl,xu,yu) 		Magnify the fraction from xl,yl to
*						xu,yu of current frame
*
*
*	move(x,y)			Move current point to x y
*	point(x,y)			Move current point, draw point at x y
*	cont(x,y)			Draw line from current point to x y
*						and set current point to x y
*	line(x1,y1,x2,y2)		Draw line from x1 y1 to x2 y2 and 
*						and set current point to x2 y2
*
*	polygon(n,v)			Draw a closed polygon with n vertices
*						given by v[] = (x,y)[]
*						Fill with color  col[] .
*	rectangle(v)			draw rectangle with verices v[0],v[1],
*					color v[2], ....  fourth vertex is upper
*					right, first is lower left.
*
*
*	set_linestyle(style)		Set Line style
*
*	arc(xc,yc,x1,y1,x2,y2) 		Draw circular arc with specied center
*						passing thru two points
*
*	arrow(xc,yc,xl,yl)		Draw arrow centered at xc,yc and with
*						'uni_array'  xl,yl
*
*	set_text_precision(strng,font,slant) Set text precision to STRING or 
*					STROKE precision, specifies a font 
*					number and a slant angle for text,
*					measured  counter-clockwise in degrees
*					from vertical.
*
*	set_text_size(height,width,space,angle)  height, width and space
*					between characters as a fraxction
*					of the current viewport dimensions.
*					Angle of text to horizontal in degrees.
*
*	text_height_angle(height,angle)	Obsolete, see previous call.
*
*	label(strng)			Draw text string at current point
*					Leave current point at end of line
*
*	set_up_color_palette(num)	Specify Number of Colors in Palette
*
*	set_up_color_table(num)		Specify Number of colors in Table
*
*	set_color_from_palette(col)	Set current color to  col  for future 
*						drawing and shading
*
*	set_color_from_table(col)	Set current color to  col  for future 
*						drawing and shading
*
*	set_color_rgb(red,green,blue)	Set current color to  col  for future 
*						drawing and shading
*
*	set_background_color(col)	Set background color to col
*
*	set_color_bar_data(min,max,strng) Setup color bar from min to max
*						with string as a label.
*
*	shade()				Color-fill the region containing the
*					current point with current color.
*
*	erase()				Erase the whole screen
*
*	repeat(n)			Repeat the last frame  n  times
*
*	create_segment(number)		Creates a Plotting Segment
*
*	close_segment()			Closes the Current Segment
*
*	delete_segment(number)		Delete a segment
*
*	set_visibility(number,on/off)	Makes a segment visible/invisible
*
*	rename_segment(num1,num2)	Rename a segment
*
*	
*/


#if !defined(_PLOTDECS_H)
#define _PLOTDECS_H

#include <cdecs.h>

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif

enum {
	NO_BINARY_VERSION = 0,
	BINARY_VERSION	  = 1
};

enum {
	BLACK = 0,
	RED,
	GREEN,
	BLUE,
	YELLOW,
	MAGENTA,
	CYAN,
	WHITE
};

enum {
	STRING_PRECISION = 0,
	STROKE_PRECISION = 1
};

#if defined(cray) && !defined(_CRAYIEEE) /*a cray ieee machine*/
IMPORT	void	cray_fwrite_ieee_float(const void*,size_t,int,FILE*);
IMPORT	void	cray_fwrite_ieee_int(const void*,size_t,int,FILE*);
#define fwrite_ieee_float(f,size,num,file)				\
	cray_fwrite_ieee_float((const void*)f,size,num,file)
#define fwrite_ieee_int(i,size,num,file)				\
	cray_fwrite_ieee_int((const void*)i,size,num,file)
#else /* defined(cray) && !defined(_CRAYIEEE) */
#define fwrite_ieee_float(f,size,num,file)				\
	(void) fwrite((const void*)f,size,num,file)
#define fwrite_ieee_int(i,size,num,file)				\
	(void) fwrite((const void*)i,size,num,file)
#endif /* defined(cray) && !defined(_CRAYIEEE) */

/* plot.c function prototypes */
IMPORT	void	fset_binary_mode(FILE*,int);
IMPORT	void	fopenpl(FILE*);
IMPORT	void	fclosepl(FILE*);
IMPORT	void	fspace(FILE*,double,double,double,double);
IMPORT	void	frotate(FILE*,double,double,double);
IMPORT	void	fwindow(FILE*,double,double,double,double);
IMPORT	void	fviewport(FILE*,double,double,double,double);
IMPORT	void	fline(FILE*,double,double,double,double);
IMPORT	void	fmove(FILE*,double,double);
IMPORT	void	fcont(FILE*,double,double);
IMPORT	void	fpolygon(FILE*,int,double*);
IMPORT	void	frectangle(FILE*,double*);
IMPORT	void	fset_text_precision(FILE*,int,int,double);
IMPORT	void	fset_text_size(FILE*,double,double,double,double);
IMPORT	void	ftext_height_angle(FILE*,double,double);
IMPORT	void	flabel(FILE*,const char*);
IMPORT	void	ferase(FILE*);
IMPORT	void	frepeat(FILE*,int);
IMPORT	void	fpoint(FILE*,double,double);
IMPORT	void	fset_up_colors(FILE*,int);
IMPORT	void	fset_color(FILE*,int);
IMPORT	void	fset_up_color_palette(FILE*,int);
IMPORT	void	fset_up_color_table(FILE*,int);
IMPORT	void	fset_color_from_palette(FILE*,double);
IMPORT	void	fset_color_from_table(FILE*,int);
IMPORT	void	fset_color_rgb(FILE*,double,double,double);
IMPORT	void	fset_background_color(FILE*,int);
IMPORT	void	fset_color_bar_data(FILE*,double,double,const char*);
IMPORT	void	fshade(FILE*);
IMPORT	void	flinemod(FILE*,const char*);
IMPORT	void	fset_linestyle(FILE*,const char*);
IMPORT	void	farc(FILE*,double,double,double,double,double,double);
IMPORT	void	farrow(FILE*,double,double,double,double);
IMPORT	void	fclosevt(FILE*);
IMPORT	void	fcreate_segment(FILE*,int);
IMPORT	void	fclose_segment(FILE*);
IMPORT	void	fdelete_segment(FILE*,int);
IMPORT	void	fset_visibility(FILE*,int,int);
IMPORT	void	frename_segment(FILE*,int,int);
IMPORT	void	fset_projection(FILE*,const char*,double,double,double);
IMPORT	void	fset_view_reference_point(FILE*,double,double,double);
IMPORT	void	fset_view_plane_normal(FILE*,double,double,double);
IMPORT	void	fset_view_up_vector(FILE*,double,double,double);
IMPORT	void	fline_3(FILE*,double,double,double,double,double,double);
IMPORT	void	fmove_3(FILE*,double,double,double);
IMPORT	void	fcont_3(FILE*,double,double,double);
IMPORT	void	farrow_3(FILE*,double,double,double,double,double,double);
IMPORT	void	fpolygon_3(FILE*,double,int,double*);
IMPORT	void	fset_hidden_surface_removal(FILE*,int);
IMPORT	void	fset_hidden_line_removal(FILE*,int);
IMPORT	void	fset_convex_polyhedra(FILE*,int);
IMPORT	void	fmagnify(FILE*,double,double,double,double);

#if defined(c_plusplus) || defined(__cplusplus)
}
#endif


/* macros for plotting */
#define	set_binary_mode(state)	fset_binary_mode(PLOTFILE,state)
#define	openpl()		fopenpl(PLOTFILE)
#define	closepl()		fclosepl(PLOTFILE)
#define	space(x1,y1,x2,y2)	fspace(PLOTFILE,x1,y1,x2,y2)
#define	rotate(xc,yc,angle)	frotate(PLOTFILE,xc,yc,angle)
#define	window(x1,y1,x2,y2)	fwindow(PLOTFILE,x1,y1,x2,y2)
#define	viewport(x1,y1,x2,y2)	fviewport(PLOTFILE,x1,y1,x2,y2)
#define	line(x1,y1,x2,y2)	fline(PLOTFILE,x1,y1,x2,y2)
#define	move(x,y)		fmove(PLOTFILE,x,y)
#define	cont(x,y)		fcont(PLOTFILE,x,y)
#define	polygon(m,v)		fpolygon(PLOTFILE,m,v)
#define	rectangle(v)		frectangle(PLOTFILE,v)
#define	set_text_precision(precision,font,slant)			\
				fset_text_precision(PLOTFILE,precision,font,slant)
#define	set_text_size(height,width,spce,angle)				\
				fset_text_size(PLOTFILE,height,width,spce,angle)
#define	text_height_angle(h,a) ftext_height_angle(PLOTFILE,h,a)
#define	label(strng)		flabel(PLOTFILE,strng)
#define	erase()			ferase(PLOTFILE)
#define	repeat(num)		frepeat(PLOTFILE,num)
#define	point(x,y)		fpoint(PLOTFILE,x,y)
#define	set_up_colors(num)	fset_up_colors(PLOTFILE,num)
#define	set_color(col)		fset_color(PLOTFILE,col)
#define	set_up_color_palette(num)					\
				fset_up_color_palette(PLOTFILE,num)
#define	set_up_color_table(num)	fset_up_color_table(PLOTFILE,num)
#define	set_color_from_palette(col_frac)				\
				fset_color_from_palette(PLOTFILE,col_frac)
#define	set_color_from_table(col)					\
				fset_color_from_table(PLOTFILE,col)
#define	set_color_rgb(red,green,blue)					\
				fset_color_rgb(PLOTFILE,red,green,blue)
#define	set_background_color(col)					\
				fset_background_color(PLOTFILE,col)
#define	set_color_bar_data(min,max,strng)				\
				fset_color_bar_data(PLOTFILE,min,max,strng)
#define	shade()			fshade(PLOTFILE)
#define	linemod(strng)		flinemod(PLOTFILE,strng)
#define	set_linestyle(style)	fset_linestyle(PLOTFILE,style)
#define	arc(xc,yc,x1,y1,x2,y2)	farc(PLOTFILE,xc,yc,x1,y1,x2,y2)
#define	arrow(xc,yc,x1,y1)	farrow(PLOTFILE,xc,yc,x1,y1)
#define	closevt()		fclosevt(PLOTFILE);
#define	create_segment(num)	fcreate_segment(PLOTFILE,num)
#define	close_segment()		fclose_segment(PLOTFILE)
#define	delete_segment(num)	fdelete_segment(PLOTFILE,num)
#define	set_visibility(num,state)					\
				fset_visibility(PLOTFILE,num,state)
#define	rename_segment(n1,n2)	frename_segment(PLOTFILE,n1,n2)
#define	set_projection(type,x,y,z)					\
				fset_projection(PLOTFILE,type,x,y,z)
#define	set_view_reference_point(x,y,z)					\
				fset_view_reference_point(PLOTFILE,x,y,z)
#define	set_view_plane_normal(nx,ny,nz)					\
				fset_view_plane_normal(PLOTFILE,nx,ny,nz)
#define	set_view_up_vector(nx,ny,nz)					\
				fset_view_up_vector(PLOTFILE,nx,ny,nz)
#define	line_3(x1,y1,z1,x2,y2,z2)					\
				fline_3(PLOTFILE,x1,y1,z1,x2,y2,z2)
#define	move_3(x,y,z)		fmove_3(PLOTFILE,x,y,z)
#define	cont_3(x,y,z)		fcont_3(PLOTFILE,x,y,z)
#define	arrow_3(x,y,z,vx,vy,vz)	farrow_3(PLOTFILE,x,y,z,vx,vy,vz)
#define	polygon_3(col,m,v)	fpolygon_3(PLOTFILE,col,m,v)
#define	set_hidden_surface_removal(flag)				\
				fset_hidden_surface_removal(PLOTFILE,flag)
#define	set_hidden_line_removal(flag)				\
				fset_hidden_line_removal(PLOTFILE,flag)
#define	set_convex_polyhedra(flag)				\
				fset_convex_polyhedra(PLOTFILE,flag)
#define	magnify(xl,yl,xu,yu)	fmagnify(PLOTFILE,xl,yl,xu,yu)

#define  PLOTFILE		stdout

#endif /* !defined(_PLOTDECS_H) */
