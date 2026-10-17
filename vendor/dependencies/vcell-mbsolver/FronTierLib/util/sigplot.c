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
*			sigplot.c
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	FUNCTIONS FOR SIG, AN INTERMEDIATE PLOT LANGUAGE
*
*			Oliver A. McBryan
*
*
*	All of the coordinate arguments listed below are floats
*	with the exception of the arguments to viewport() and
*	magnify().
*
*
*	fopenpl(file)			Must be called before any plotting
*	fclosepl(file)			Terminates plotting session
*
*
*	fwindow(file,x1,y1,x2,y2) 	Set User Coordinate Clipping Window
*	fviewport(file,fx1,fg1,fx2,fy2)	Set Fractional Viewing Screen Window 
*						in [0.-1.] * [0.-1.]
*	fmagnify(file,xl,yl,xu,yu) 	Magnify the fraction from xl,yl to
*						xu,yu of current frame
*
*
*	fmove(file,x,y)			Move current point to x y
*	fpoint(file,x,y)		Move current point, draw point at x y
*	fcont(file,x,y)			Draw line from current point to x y
*						and set current point to x y
*	fline(file,x1,y1,x2,y2)		Draw line from x1 y1 to x2 y2 and 
*						and set current point to x2 y2
*
*	fpolygon(file,n,v)		Draw a closed polygon with n vertices
*						given by v[] = (x,y)[]
*						Fill with color  col[] .
*	frectangle(file,v)		Draw rectangle with verices v[0],v[1],
*					color v[2], ....  fourth vertex is upper
*					right, first is lower left.
*
*
*	fset_linestyle(file,style)	Set Line style
*
*	farc(file,xc,yc,x1,y1,x2,y2) 	Draw circular arc with specied center
*						passing thru two points
*
*	farrow(file,xc,yc,xl,yl)	Draw arrow centered at xc,yc and with
*						'uni_array'  xl,yl
*
*	fset_text_precision(file,strng,font,slant)
*					Set text precision to STRING or 
*					STROKE precision, specifies a font 
*					number and a slant angle for text,
*					measured  counter-clockwise in degrees
*					from vertical.
*
*	fset_text_size(file,height,width,space,angle)
*					height, width and space
*					between characters as a fraxction
*					of the current viewport dimensions.
*					Angle of text to horizontal in degrees.
*
*	flabel(file,strng)		Draw text string at current point
*					Leave current point at end of line
*
*	fset_up_color_palette(file,num)	Specify Number of Colors in Palette
*
*	fset_up_color_table(file,num)	Specify Number of colors in Table
*
*	fset_color_from_palette(file,col)
*					Set current color to  col  for future 
*						drawing and shading
*
*	fset_color_from_table(file,col)	Set current color to  col  for future 
*						drawing and shading
*
*	fset_color_rgb(file,red,green,blue)
*					Set current color to  col  for future 
*						drawing and shading
*
*	fset_background_color(file,col)	Set background color to col
*
*	fset_color_bar_data(file,min,max,strng)
*					Setup color bar from min to max
*						with string as a label.
*
*	fshade(file)			Color-fill the region containing the
*					current point with current color.
*
*	ferase(file)				Erase the whole screen
*
*	frepeat(file,n)			Repeat the last frame  n  times
*
*	fcreate_segment(file,number)	Creates a Plotting Segment
*
*	fclose_segment(file)		Closes the Current Segment
*
*	fdelete_segment(file,number)	Delete a segment
*
*	fset_visibility(file,number,on/off)
*					Makes a segment visible/invisible
*
*	frename_segment(file,num1,num2)	Rename a segment
*
*	
*/

#include <plotdecs.h>

#define	binary_plot ((is_binary_output()==YES)?BINARY_VERSION:NO_BINARY_VERSION)

#define FOutput(file)	(void) foutput(file),(void) putc('\n',file)

/* LOCAL Function Prototypes*/
LOCAL	void	fputbf1(FILE*,double);
LOCAL	void	fputbf2(FILE*,double,double);
LOCAL	void	fputbf3(FILE*,double,double,double);
LOCAL	void	fputbf4(FILE*,double,double,double,double);
LOCAL	void	fputbf6(FILE*,double,double,double,double,double,double);
LOCAL	void	fputbi1(FILE*,int);
LOCAL	void	fputbi2(FILE*,int,int);

EXPORT	void	fset_binary_mode(
	FILE	*file,
	int	state)
{
	(void) fprintf(file,"b   %d\n",state);
}	/*end fset_binary_mode*/

EXPORT	void	fopenpl(
	FILE	*file)
{
	print_machine_parameters(file);
	set_binary_mode(binary_plot);
}	/*end fopenpl*/


EXPORT	void	fclosepl(
	FILE	*file)
{
	trace_foutput(file);
	if (is_binary_output() == NO)
	    (void) fprintf(file,"F\n");
}	/*end fclosepl*/

EXPORT	void	fspace(
	FILE	*file,
	double	x1,
	double	y1,
	double	x2,
	double	y2)
{
	(void) fprintf(file,"W %g %g %g %g\n",x1,y1,x2,y2);
}	/*end fspace*/

EXPORT	void	frotate(
	FILE	*file,
	double	xc,
	double	yc,
	double	angle)
{
	if (is_binary_output() == YES)
	{
	    (void) putc('Z',file);
	    fputbf3(file,xc,yc,angle);
	}
	else
	    (void) fprintf(file,"Z %g %g %g\n",xc,yc,angle);

}	/*end frotate*/


EXPORT	void	fwindow(
	FILE	*file,
	double	x1,
	double	y1,
	double	x2,
	double	y2)
{
	(void) fprintf(file,"W %g %g %g %g\n",x1,y1,x2,y2);
}	/*end fwindow*/

EXPORT	void	fviewport(
	FILE	*file,
	double	x1,
	double	y1,
	double	x2,
	double	y2)
{
	(void) fprintf(file,"V %g %g %g %g\n",x1,y1,x2,y2);
}	/*end fviewport*/

EXPORT	void	fline(
	FILE	*file,
	double	x1,
	double	y1,
	double	x2,
	double	y2)
{
	if (is_binary_output() == YES)
	{
	    (void) putc('L',file);
	    fputbf4(file,x1,y1,x2,y2);
	}
	else
	    (void) fprintf(file,"L %g %g %g %g\n",x1,y1,x2,y2);
}	/*end fline*/

EXPORT	void	fmove(
	FILE	*file,
	double	x,
	double	y)
{
	if (is_binary_output() == YES)
	{
	    (void) putc('M',file);
	    fputbf2(file,x,y);
	}
	else
	    (void) fprintf(file,"M %g %g\n",x,y);
}	/*end fline*/

EXPORT	void	fcont(
	FILE	*file,
	double	x,
	double	y)
{
	if (is_binary_output() == YES)
	{
	    (void) putc('C',file);
	    fputbf2(file,x,y);
	}
	else
	    (void) fprintf(file,"C %g %g\n",x,y);
}	/*end fcont*/

EXPORT	void	fpolygon(
	FILE	*file,
	int	m,
	double	*v)
{
	int i;
	if (is_binary_output() == YES)
	{
	    TRUEfloat _ff_[3];
	    (void) putc('p',file);
	    fputbi1(file,m); 
	    for (i = 0; i < m; i++)
	    {
	    	_ff_[0] = v[3*i];
	    	_ff_[1] = v[3*i+1];
	    	_ff_[2] = v[3*i+2];
	    	fwrite_ieee_float(_ff_,sizeof(TRUEfloat),3,file);
	    }
	}
	else
	{
	    (void) fprintf(file,"p %d",m); 
	    for (i = 0; i < m; i++)
	    	(void) fprintf(file," %g %g %g",v[3*i],v[3*i+1],v[3*i+2]);
	    (void) fprintf(file,"\n");
	}
}	/*end fpolygon*/

EXPORT	void	frectangle(
	FILE	*file,
	double	*v)
{
	int i;
	if (is_binary_output() == YES)
	{
	    TRUEfloat vv[12];
	    (void) putc('r',file);
	    for (i = 0; i < 12; i++)
	    	vv[i] = v[i];
	    fwrite_ieee_float(vv,sizeof(TRUEfloat),12,file);
	}
	else
	{
	    (void) fprintf(file,"r");
	    for (i = 0; i < 4; i++)
	    	(void) fprintf(file," %g %g %g",v[3*i],v[3*i+1],v[3*i+2]);
	    (void) fprintf(file,"\n");
	}
}	/*end frectangle*/

EXPORT	void	fset_text_precision(
	FILE	*file,
	int	precision,
	int	font,
	double	slant)
{
	if (is_binary_output() == YES)
	{
	    (void) fprintf(file,"tp");
	    fputbi2(file,precision,font);
	    fputbf1(file,slant);
	}
	else
	    (void) fprintf(file,"tp %d %d %g\n",precision,font,slant);
}	/*end fset_text_precision*/

EXPORT	void	fset_text_size(
	FILE	*file,
	double	height,
	double	width,
	double	spce,
	double	angle)
{
	if (is_binary_output() == YES)
	{
	    (void) fprintf(file,"ts");
	    fputbf4(file,height,width,spce,angle);
	}
	else
	    (void) fprintf(file,"ts %g %g %g %g\n",height,width,spce,angle);
}	/*end fset_text_size*/

EXPORT	void	ftext_height_angle(
	FILE	*file,
	double	h,
	double	a)
{
	fset_text_size(file,h,6*h,16*h,a);
}	/*end ftext_height_angle*/

EXPORT	void	flabel(
	FILE	   *file,
	const char *strng)
{
	if (is_binary_output() == YES)
	    (void) fprintf(file,"T%s\n",strng);
	else
	    (void) fprintf(file,"T %d %s\n",(int)strlen(strng),strng);
}	/*end flabel*/

EXPORT	void	ferase(
	FILE	*file)
{
	if (is_binary_output() == YES)
	{
	    (void) fprintf(file,"\nE\n####!!!!\n");
	    (void) fflush(file);
	    FOutput(file);
	    set_binary_mode(BINARY_VERSION);
	}
	else
	{
	    (void) fprintf(file,"E\n");
	    (void) fflush(file);
	    FOutput(file);
	    fset_binary_mode(file,NO_BINARY_VERSION);
	}
}	/*end ferase*/

EXPORT	void	frepeat(
	FILE	*file,
	int	num)
{
	if (is_binary_output() == YES)
	{
	    (void) putc('R',file);
	    fputbi1(file,num);
	}
	else
	    (void) fprintf(file,"R %d\n",num);
}	/*end frepeat*/

EXPORT	void	fpoint(
	FILE	*file,
	double	x,
	double	y)
{
	if (is_binary_output() == YES)
	{
	    (void) putc('P',file);
	    fputbf2(file,x,y);
	}
	else
	    (void) fprintf(file,"P %g %g\n",x,y);
}	/*end fpoint*/

EXPORT	void	fset_up_colors(
	FILE	*file,
	int	num)
{
	if (is_binary_output() == YES)
	{
	    (void) fprintf(file,"cu");
	    fputbi1(file,num);
	}
	else
	    (void) fprintf(file,"cu %d\n",num);
}	/*end fset_up_colors*/

EXPORT	void	fset_color(
	FILE	*file,
	int	col)
{
	if (is_binary_output() == YES)
	{
	    (void) fprintf(file,"cc");
	    fputbi1(file,col);
	}
	else
	    (void) fprintf(file,"cc %d\n",col);
}	/*end fset_color*/

EXPORT	void	fset_up_color_palette(
	FILE	*file,
	int	num)
{
	if (is_binary_output() == YES)
	{
	    (void) fprintf(file,"cP");
	    fputbi1(file,num);
	}
	else
	    (void) fprintf(file,"cP %d\n",num);
}	/*end fset_up_color_palette*/

EXPORT	void	fset_up_color_table(
	FILE	*file,
	int	num)
{
	if (is_binary_output() == YES)
	{
	    (void) fprintf(file,"cT");
	    fputbi1(file,num);
	}
	else
	    (void) fprintf(file,"cT %d\n",num);
}	/*end fset_up_color_table*/

EXPORT	void	fset_color_from_palette(
	FILE	*file,
	double	col_frac)
{
	if (is_binary_output() == YES)
	{
	    (void) fprintf(file,"cp");
	    fputbf1(file,col_frac);
	}
	else
	    (void) fprintf(file,"cp %g\n",col_frac);
}	/*end fset_color_from_palette*/

EXPORT	void	fset_color_from_table(
	FILE	*file,
	int	col)
{
	if (is_binary_output() == YES)
	{
	    (void) fprintf(file,"ct");
	    fputbi1(file,col);
	}
	else
	    (void) fprintf(file,"ct %d\n",col);
}	/*end fset_color_from_table*/

EXPORT	void	fset_color_rgb(
	FILE	*file,
	double	red,
	double	green,
	double	blue)
{
	if (is_binary_output() == YES)
	{
	    (void) fprintf(file,"cr");
	    fputbf3(file,red,green,blue);
	}
	else
	    (void) fprintf(file,"cr %g %g %g\n",red,green,blue);
}	/*end fset_color_rgb*/


EXPORT	void	fset_background_color(
	FILE	*file,
	int	col)
{
	if (is_binary_output() == YES)
	{
	    (void) fprintf(file,"cb");
	    fputbi1(file,col);
	}
	else
	    (void) fprintf(file,"cb %d\n",col);
}	/*end fset_background_color*/

EXPORT	void	fset_color_bar_data(
	FILE	   *file,
	double	   mn,
	double	   mx,
	const char *strng)
{
	(void) fprintf(file,"cB %g %g %s\n",mn,mx,strng);
}	/*end fset_color_bar_data*/

EXPORT	void	fshade(
	FILE	*file)
{
	(void) fprintf(file,"cs");
	if (is_binary_output() == NO)
	    (void) fprintf(file,"\n");
}	/*end fshade*/

EXPORT	void	flinemod(
	FILE	   *file,
	const char *strng)
{
	if (is_binary_output() == YES)
	    (void) fprintf(file,"l%s\n",strng);
	else
	    (void) fprintf(file,"l %d %s\n",(int)strlen(strng),strng);

}	/*end flinemod*/

EXPORT	void	fset_linestyle(
	FILE	   *file,
	const char *style)
{
	if (is_binary_output() == YES)
	    (void) fprintf(file,"l%s\n",style);
	else
	    (void) fprintf(file,"l %s\n",style);

}	/*end fset_linestyle*/

EXPORT	void	farc(
	FILE	*file,
	double	xc,
	double	yc,
	double	x1,
	double	y1,
	double	x2,
	double	y2)
{
	if (is_binary_output() == YES)
	{
	    (void) putc('A',file);
	    fputbf6(file,xc,yc,x1,y1,x2,y2);
	}
	else
	    (void) fprintf(file,"A %g %g %g %g %g %g\n",xc,yc,x1,y1,x2,y2);
}	/*end farc*/

EXPORT	void	farrow(
	FILE	*file,
	double	xc,
	double	yc,
	double	xl,
	double	yl)
{
	if (is_binary_output() == YES)
	{
	    (void) putc('a',file);
	    fputbf4(file,xc,yc,xl,yl);
	}
	else
	    (void) fprintf(file,"a %g %g %g %g\n",xc,yc,xl,yl);
}	/*end farrow*/

/*ARGSUSED*/
EXPORT	void	fclosevt(
	FILE	*file)
{
}	/*end fclosevt*/

EXPORT	void	fcreate_segment(
	FILE	*file,
	int	num)
{
	(void) fprintf(file,"_create_seg %d\n",num);
}	/*end fcreate_segment*/

EXPORT	void	fclose_segment(
	FILE	*file)
{
	(void) fprintf(file,"_close_seg\n");
}	/*end fclose_segment*/

EXPORT	void	fdelete_segment(
	FILE	*file,
	int	num)
{
	(void) fprintf(file,"_delete_seg %d\n",num);
}	/*end fdelete_segment*/

EXPORT	void	fset_visibility(
	FILE	*file,
	int	num,
	int	state)
{
	(void) fprintf(file,"_visible %d %d\n",num,state);
}	/*end fset_visibility*/

EXPORT	void	frename_segment(
	FILE	*file,
	int	n1,
	int	n2)
{
	(void) fprintf(file,"_rename_seg %d %d\n",n1,n2);
}	/*end frename_segment*/

EXPORT	void	fset_projection(
	FILE	   *file,
	const char *type,
	double	   x,
	double	   y,
	double	   z)
{
	if (is_binary_output() == YES)
	{
	    (void) fprintf(file,"3p%c",type[1]);
	    fputbf3(file,x,y,z);
	}
	else
	    (void) fprintf(file,"3p%c %g %g %g\n",type[1],x,y,z);
}	/*end fset_projection*/

EXPORT	void	fset_view_reference_point(
	FILE	*file,
	double	x,
	double	y,
	double	z)
{
	if (is_binary_output() == YES)
	{
	    (void) fprintf(file,"3r");
	    fputbf3(file,x,y,z);
	}
	else
	    (void) fprintf(file,"3r %g %g %g\n",x,y,z);
}	/*end fset_view_reference_point*/

EXPORT	void	fset_view_plane_normal(
	FILE	*file,
	double	nx,
	double	ny,
	double	nz)
{
	if (is_binary_output() == YES)
	{
	    (void) fprintf(file,"3n");
	    fputbf3(file,nx,ny,nz);
	}
	else
	    (void) fprintf(file,"3n %g %g %g\n",nx,ny,nz);
}	/*end fset_view_plane_normal*/

EXPORT	void	fset_view_up_vector(
	FILE	*file,
	double	nx,
	double	ny,
	double	nz)
{
	if (is_binary_output() == YES)
	{
	    (void) fprintf(file,"3u");
	    fputbf3(file,nx,ny,nz);
	}
	else
	    (void) fprintf(file,"3u %g %g %g\n",nx,ny,nz);
}	/*end fset_view_up_vector*/

EXPORT	void	fline_3(
	FILE	*file,
	double	x1,
	double	y1,
	double	z1,
	double	x2,
	double	y2,
	double	z2)
{
	if (is_binary_output() == YES)
	{
	       (void) fprintf(file,"3L");
	       fputbf6(file,x1,y1,z1,x2,y2,z2);
	}
	else
	       (void) fprintf(file,"3L %g %g %g %g %g %g\n",x1,y1,z1,x2,y2,z2);
}	/*end fline_3*/

EXPORT	void	fmove_3(
	FILE	*file,
	double	x,
	double	y,
	double	z)
{
	if (is_binary_output() == YES)
	{
	    (void) fprintf(file,"3M");
	    fputbf3(file,x,y,z);
	}
	else
	    (void) fprintf(file,"3M %g %g %g\n",x,y,z);
}	/*end fmove_3*/

EXPORT	void	fcont_3(
	FILE	*file,
	double	x,
	double	y,
	double	z)
{
	if (is_binary_output() == YES)
	{
	    (void) fprintf(file,"3C");
	    fputbf3(file,x,y,z);
	}
	else
	    (void) fprintf(file,"3C %g %g %g\n",x,y,z);
}	/*end fcont_3*/

EXPORT	void	farrow_3(
	FILE	*file,
	double	x,
	double	y,
	double	z,
	double	vx,
	double	vy,
	double	vz)
{
	if (is_binary_output() == YES)
	{
	    (void) fprintf(PLOTFILE,"3a");
	    fputbf6(file,x,y,z,vx,vy,vz);
	}
	else
	    (void) fprintf(PLOTFILE,"3a %g %g %g %g %g %g\n",x,y,z,vx,vy,vz);
}	/*end farrow_3*/

EXPORT	void	fpolygon_3(
	FILE	*file,
	double	col,
	int	m,
	double	*v)
{
	int i;

	if (is_binary_output() == YES)
	{
	    (void) fprintf(PLOTFILE,"3p ");
	    fputbf1(file,col);
	    fputbi1(file,m);
	    for (i = 0; i < m; i++)
	    	fputbf3(file,v[3*i],v[3*i+1],v[3*i+2]);
	}
	else
	{
	    (void) fprintf(file,"3p %g %d",col,m);
	    for (i = 0; i < m; i++)
	    	(void) fprintf(file," %g %g %g",v[3*i],v[3*i+1],v[3*i+2]);
	    (void) fprintf(file,"\n");
	}
}	/*end fpolygon_3*/

EXPORT	void	fset_hidden_surface_removal(
	FILE	*file,
	int	flag)
{
	if (is_binary_output() == YES)
	{
	    (void) fprintf(file,"3hs");
	    fputbi1(file,flag);
	}
	else
	    (void) fprintf(file,"3hs %d\n",flag);
}	/*end fset_hidden_surface_removal*/

EXPORT	void	fset_hidden_line_removal(
	FILE	*file,
	int	flag)
{
	if (is_binary_output() == YES)
	{
	    (void) fprintf(file,"3hl");
	    fputbi1(file,flag);
	}
	else
	    (void) fprintf(file,"3hl %d\n",flag);
}	/*end fset_hidden_line_removal*/

EXPORT	void	fset_convex_polyhedra(
	FILE	*file,
	int	flag)
{
	if (is_binary_output() == YES)
	{
	    (void) fprintf(file,"3hc");
	    fputbi1(file,flag);
	}
	else
	    (void) fprintf(file,"3hc %d\n",flag);
}	/*end fset_convex_polyhedra*/

EXPORT	void	fmagnify(
	FILE	*file,
	double	xl,
	double	yl,
	double	xu,
	double	yu)
{
	(void) fprintf(file,"m %g %g %g %g\n",xl,yl,xu,yu);
}	/*end fmagnify*/

#if defined(cray) && !defined(_CRAYIEEE)

FORTRAN	int CRAY2IEG(int*,int*,void*,int*,const void*);

EXPORT	void	cray_fwrite_ieee_float(
	const void	*f,
	size_t		size_float,
	int		num,
	FILE		*file)
{
	int bitoff = 0;
	static	void		*buf = NULL;
	static	int		buf_len = 0;
	int type = 2;

	if (buf_len < num)
	{
	    if (buf != NULL)
	    	free(buf);
	    buf_len = 2*num;
	    buf = (void*)malloc(buf_len*size_float);
	}
	(void) CRAY2IEG(&type,&num,&buf,&bitoff,f);
	(void) fwrite((const void *)buf,size_float,num,file);
}	/*end cray_fwrite_ieee_float*/

EXPORT	void	cray_fwrite_ieee_int(
	const void	*i,
	size_t		size_int,
	int		num,
	FILE		*file)
{
	int bitoff = 0;
	static	void		*buf = NULL;
	static	int		buf_len = 0;
	int type = 1;

	if (buf_len < num)
	{
	    if (buf != NULL)
	    	free(buf);
	    buf_len = 2*num;
	    buf = (void*)malloc(buf_len*size_int);
	}
	(void) CRAY2IEG(&type,&num,&buf,&bitoff,i);
	(void) fwrite((const void *)buf,size_int,num,file);
}	/*end cray_fwrite_ieee_int*/
#endif /* defined(cray) && !defined(_CRAYIEEE) */

LOCAL	void	fputbf1(
	FILE	*file,
	double	f1)
{
	TRUEfloat	_ff_ = f1;
	fwrite_ieee_float(&_ff_,sizeof(TRUEfloat),1,file);
}	/* end fputbf1*/

LOCAL	void	fputbf2(
	FILE	*file,
	double	f1,
	double	f2)
{
	TRUEfloat	_ff_[2];
	_ff_[0] = f1;
	_ff_[1] = f2;
	fwrite_ieee_float(_ff_,sizeof(TRUEfloat),2,file);
}	/* end fputbf2*/

LOCAL	void	fputbf3(
	FILE	*file,
	double	f1,
	double	f2,
	double	f3)
{
	TRUEfloat	_ff_[3];
	_ff_[0] = f1;
	_ff_[1] = f2;
	_ff_[2] = f3;
	fwrite_ieee_float(_ff_,sizeof(TRUEfloat),3,file);
}	/* end fputbf3*/

LOCAL	void	fputbf4(
	FILE	*file,
	double	f1,
	double	f2,
	double	f3,
	double	f4)
{
	TRUEfloat	_ff_[4];
	_ff_[0] = f1;
	_ff_[1] = f2;
	_ff_[2] = f3;
	_ff_[3] = f4;
	fwrite_ieee_float(_ff_,sizeof(TRUEfloat),4,file);
}	/* end fputbf4*/

LOCAL	void	fputbf6(
	FILE	*file,
	double	f1,
	double	f2,
	double	f3,
	double	f4,
	double	f5,
	double	f6)
{
	TRUEfloat	_ff_[6];
	_ff_[0] = f1;
	_ff_[1] = f2;
	_ff_[2] = f3;
	_ff_[3] = f4;
	_ff_[4] = f5;
	_ff_[5] = f6;
	fwrite_ieee_float(_ff_,sizeof(TRUEfloat),6,file);
}	/* end fputbf3*/

LOCAL	void	fputbi1(
	FILE	*file,
	int	i1)
{
	fwrite_ieee_int((const void*)&i1,sizeof(int),1,file);
}	/* end fputbi1*/

LOCAL	void	fputbi2(
	FILE	*file,
	int	i1,
	int	i2)
{
	int	_ii_[2];
	_ii_[0] = i1;
	_ii_[1] = i2;
	fwrite_ieee_int((const void*)_ii_,sizeof(int),2,file);
}	/* end fputbi2*/
