
/*
*				igview.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Contains printing routines for producing Geomview interface plots.
*/


#include <intfc/iloc.h>
#include <util/cdecs.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef USE_CGNS
#include "pak/cgnslib_3.0/cgnslib.h"
#endif
	/* LOCAL Function Prototypes */
LOCAL	char	*get_list_file_name(char*,const char*,const char*,size_t*);
LOCAL	char	*set_ppfname(char*,const char*,size_t*);
LOCAL	void	gview_plot_cube(const char*,const char*,const double*,
	                        const double*,const double*,const double*);
LOCAL   void    gview_plot_curves(INTERFACE*,const double*,const double*,
	                          const char*,const char*,SURFACE_COLOR,int);
LOCAL	void	gview_plot_surfaces(INTERFACE*,RECT_GRID*,const double*,
	                            const double*,boolean,
				    const char*,const char*,
				    boolean,SURFACE_COLOR,SURFACE_COLOR);
LOCAL	void	print_polyline_description(FILE*,const char*,double* const*,int,
                                           SURFACE_COLOR color,double,int);
LOCAL	void	write_interpolated_color(FILE*,SURFACE_COLOR,
				         SURFACE_COLOR,double,double);
LOCAL   void 	write_color(FILE*,SURFACE_COLOR,double);
LOCAL   void    gview_plot_color_surfaces(INTERFACE*,RECT_GRID*,const double*,const double*, boolean,const char*,const char*,boolean);

/*************************New for CGNS****************************/

#ifdef USE_CGNS
EXPORT  char   *get_cgns_file_name(char*,const char*,const char*,size_t*);
LOCAL   void    cgns_print_box(const char*,const double*,const double*,boolean);
LOCAL   void    cgns_plot_surfaces(INTERFACE*,const double*,
                                  const double*,boolean,
                                  const char*,const char*,
                                  boolean,SURFACE_COLOR,SURFACE_COLOR,boolean,double,int);
LOCAL   void    cgns_plot_curves( INTERFACE*, const double*, 
                                 const double*, const char*, 
                                 const char*,SURFACE_COLOR,int,boolean,double,int);
#endif
/*****************************************************************/
EXPORT  char   *get_vtk_file_name(char*,const char*,const char*,size_t*);
LOCAL   void    vtk_print_box(const char*,const double*,const double*,boolean);
LOCAL   void    vtk_plot_surfaces(INTERFACE*,const double*,const double*,
				boolean, const char*,const char*, boolean,
				SURFACE_COLOR,SURFACE_COLOR,boolean,double,int);
LOCAL   void    vtk_plot_curves( INTERFACE*, const double*, const double*, 
				const char*, const char*,SURFACE_COLOR,int,
				boolean,double,int);
LOCAL   void    vtk_plot_curves2d( INTERFACE*, const double*, const double*, 
				const char*, const char*,SURFACE_COLOR,int,
				boolean,double,int);
LOCAL   void    vtk_plot_curves3d( INTERFACE*, const double*, const double*, 
				const char*, const char*,SURFACE_COLOR,int,
				boolean,double,int);
LOCAL	void    tecplot_surface_special(const char*,FILE*,SURFACE*);
LOCAL	void	tecplot_plot_surfaces(INTERFACE*,RECT_GRID*,const double*,
	                            const double*,boolean,
				    const char*,const char*,
				    boolean,SURFACE_COLOR,SURFACE_COLOR);
LOCAL	void 	tecplot_surface_in_box(const char*,int*,int*,SURFACE*,FILE*);

/* tmp for amr_overture 2d,infc geomview plot */

LOCAL   void    gview_plot_intfc2d(const char*,const char*, INTERFACE*, 
				RECT_GRID*);
/*For GD Movie plots */
#if defined(__GD__)
LOCAL   void    mkImg(int, int*, int*);
LOCAL   void    initialize_image(gdImagePtr, const char*, int, int);
LOCAL   void    freecolors();
LOCAL   void    setcolors(gdImagePtr, int);
LOCAL   double   xtransform(double);
LOCAL   double   ytransform(double);
LOCAL   int     getcolor(int);
#endif /* if defined(__GD__) */

EXPORT void geomview_intfc_plot2d(
        const char    *dname,
        INTERFACE     *intfc,
        RECT_GRID     *gr)
{
        static char   *fname = NULL, *ppfname = NULL;
        static size_t fname_len = 0, ppfname_len = 0;
        char              outname[256],outdir[256];
        int               myid, numnodes;

        myid = pp_mynode(); numnodes = pp_numnodes();
        /*sprintf(outdir,"%s/%s",dname,"geomintfc"); */
        sprintf(outdir,"%s",dname);
        ppfname = set_ppfname(ppfname,"intfc",&ppfname_len);
        sprintf(outname,"%s",ppfname);

        if (create_directory(dname,YES) == FUNCTION_FAILED)
        {
            (void) printf("WARNING in geomview_intfc_plot2d(), directory "
                          "%s doesn't exist and can't be created\n",dname);
            return;
        }
        if (create_directory(outdir,YES) == FUNCTION_FAILED)
        {
            (void) printf("WARNING in geomview_intfc_plot2d(), directory "
                         "%s doesn't exist and can't be created\n",outdir);
            return;
        }

        fname = get_list_file_name(fname,outdir,outname,&fname_len);
        gview_plot_intfc2d(outdir,ppfname,intfc,gr);
}

LOCAL void gview_plot_intfc2d(
        const char    *dname,
        const char    *gname,
        INTERFACE     *intfc,
        RECT_GRID     *gr)
{
        FILE              *fp;
        char              fmt[256];
        static const char *indent = "    ";
        static char       *fname;
        static size_t     fname_len;
        double             coords[MAXD];
        CURVE             **c;
        CURVE             *oldc;
        BOND              *oldb;
        double             *L = gr->L;
        double             *U = gr->U;
        double             *VL = gr->VL;
        double             *VU = gr->VU;

        fname = get_list_file_name(fname,dname,gname,&fname_len);

        if ((fp = fopen(fname,"w")) == NULL)
        {
            (void) printf("WARNING in gview_plot_intfc2d(), "
                           "can't open %s\n",fname);
            return;
        }
        fprintf(fp,"{ LIST \n");
        /* beginning of writting Vect to file */
        for(c = intfc->curves; c && *c;  c++)
        {
            oldc = *c;
            oldb = oldc->first;
            fprintf(fp,"{ \n");
            fprintf(fp,"VECT\n");
            fprintf(fp,"%1d %1d %1d\n", 1, oldc->num_points, 1);
            fprintf(fp,"%1d\n%1d\n", oldc->num_points,1);

            while (oldb)
            {
                fprintf(fp,"%f %f %f \t",  Coords(oldb->start)[0],
                                Coords(oldb->start)[1] , 1.0);
                if (oldb == oldc->last)
                    break;
                oldb = oldb->next;
            }
            fprintf(fp,"%f %f %f \t\n",  Coords(oldb->end)[0],
                  Coords(oldb->end)[1] , 1.0);
            fprintf(fp,"%f %f %f %f \t\n", 1.0, 0.2, 0.2, 0.8 );
            fprintf(fp,"}\n");
        }
        /* end of writting Vect to file     */
        /*  end of LIST OBJ
        fprintf(fp,"}\n");
        */
        /*  computational grid SKEL*/
        /* WHITE YELLOW */
        fprintf(fp,"{ VECT\n");
        fprintf(fp,"%1d %1d %1d\n", 1, 5, 1);
        fprintf(fp,"%1d\n%1d\n", 5,1);
        (void) fprintf(fp,"%f %f %f \t",L[0],L[1],1.0);
        (void) fprintf(fp,"%f %f %f \t",U[0],L[1],1.0);
        (void) fprintf(fp,"%f %f %f \t",U[0],U[1],1.0);
        (void) fprintf(fp,"%f %f %f \t\n",L[0],U[1],1.0);
        (void) fprintf(fp,"%f %f %f \t",L[0],L[1],1.0);
        fprintf(fp,"%f %f %f %f \t\n", 1.0, 1.0, 0.0, 1.0 );
        fprintf(fp,"}\n");

        /*  buffered computational grid SKEL*/
        /* WHITE COLOR   */
        fprintf(fp,"{ VECT\n");
        fprintf(fp,"%1d %1d %1d\n", 1, 5, 1);
        fprintf(fp,"%1d\n%1d\n", 5,1);
        (void) fprintf(fp,"%f %f %f \t",VL[0],VL[1],1.0);
        (void) fprintf(fp,"%f %f %f \t",VU[0],VL[1],1.0);
        (void) fprintf(fp,"%f %f %f \t",VU[0],VU[1],1.0);
        (void) fprintf(fp,"%f %f %f \t\n",VL[0],VU[1],1.0);
        (void) fprintf(fp,"%f %f %f \t",VL[0],VL[1],1.0);
        fprintf(fp,"%f %f %f %f \t\n", 1.0, 1.0, 1.0, 1.0 );
        fprintf(fp,"}\n");

        /* end of LIST OBJ */
        fprintf(fp,"}\n");

        fclose(fp);
}

EXPORT  void tecplot_show_box_tris(
	const char	*fname, 
	TRI		**tris,
	int		num_tris,
	RECT_GRID	*gr,
	int		*icrds)
{
	double		crds_st[MAXD], crds_ed[MAXD];
	double           *L, *h;
	int		i;
	FILE		*file=NULL;
	char		s[256], tname[100];

	L = gr->L;
	h = gr->h;
	for (i = 0; i < 3; ++i)
	{
	    crds_st[i] = L[i] + icrds[i]*h[i];
	    crds_ed[i] = crds_st[i] + h[i];
	}
	
	sprintf(s,"%s-%d.plt",fname,pp_mynode());
	printf("tecplot_show_box_tris fname %s \n",s);
	
	if ((file = fopen(s,"w")) == NULL)
        {
            (void) printf("WARNING in tecplot_tris(), "
                          "can't open %s\n",fname);
            return;
        }

	tecplot_box(NULL, file, crds_st, crds_ed);
	sprintf(tname, "%d_%d_%d", icrds[0], icrds[1], icrds[2]);
	tecplot_show_tris(tname, tris, num_tris, file);
	
	fclose(file);
}

LOCAL double	shx=0.0, shy=0.0, shz=0.0;

/*set shift for tecplot_show_tris and tecplot_box, used for periodic boundary test. */
EXPORT	void	set_shift_for_tecplot(
	double	sx,
	double	sy,
	double	sz)
{
	shx = sx;
	shy = sy;
	shz = sz;
}

EXPORT  void tecplot_show_tris(
	const char	*tname, 
	TRI		**tris,
	int		num_tris,
	FILE		*file)
{
	FT_POINT 	*p;
	int 	i,j;
	
	if(num_tris == 0)
	    return;

	fprintf(file, "ZONE T=\"%s\" N=%d E=%d\nF=FEPOINT, ET=TRIANGLE\n",
			     tname, 3*num_tris, num_tris);

	for (i = 0; i < num_tris; ++i)
	{
	    for (j = 0; j < 3; ++j)
	    {
	    	p = Point_of_tri(tris[i])[j];
	    	(void) fprintf(file, "%-9g %-9g %-9g\n",
		    	Coords(p)[0]+shx,Coords(p)[1]+shy,Coords(p)[2]+shz);
	    }
	}
	for (i = 0; i < num_tris; ++i)
	{
	    (void) fprintf(file,"%-4d %-4d %-4d\n",
			   3*i+1, 3*i+2, 3*i+3);
	}
}

EXPORT	void	tecplot_blk_intfc_plot(
	const char *fname,
	BLK_TRI *blk_tri)
{
	int 	ind, i, j, k,num_tris=0,count=0;
	TRI 	*tri;
	FT_POINT	*p;
	FILE 	*file=NULL;
	static int ntri;
	char	s[256];

	for (i = 0; i < blk_tri->num_surfaces; i++)
	{
	    ind = blk_tri->is[i];
	    num_tris += blk_tri->num_tris[ind];
	}
	sprintf(s,"%s-%d-%d.plt",fname,pp_mynode(),ntri++);
	printf("fname %s \n",s);
	if ((file = fopen(s,"w")) == NULL)
        {
            (void) printf("WARNING in vtk_blk_intfc_plot(), "
                          "can't open %s\n",fname);
            return;
        }
        (void) fprintf(file,"TITLE = \"tecview\"\n"
		       	    "VARIABLES = \"x\", \"y\", \"z\"\n"
			    "ZONE N=%d,  E=%d,\n"
			    "F=FEPOINT, ET=TRIANGLE\n",num_tris*3,num_tris);

	printf("printing point coords\n");
	for (i = 0; i < blk_tri->num_surfaces; i++)
        {
	    ind = blk_tri->is[i];
	    printf("printing points of surface %d\n",ind);
            for(k = 0, tri = blk_tri->first[ind]; k < blk_tri->num_tris[ind]; tri = tri->next, k++)
	    {
		for (j = 0; j < 3; j++)
		{
		    p = Point_of_tri(tri)[j];
		    (void) fprintf(file,"%-9g %-9g %-9g\n",
			  	   Coords(p)[0],Coords(p)[1],Coords(p)[2]);
	        }
	    }
	}
	printf("printing index\n");
	count = 0;
	for (i = 0; i < blk_tri->num_surfaces; i++)
        {
	    ind = blk_tri->is[i];
            for(k = 0, tri = blk_tri->first[ind]; k < blk_tri->num_tris[ind]; tri = tri->next, k++)
	    {
	        (void) fprintf(file,"%d %d %d \n",3*count+1,3*count+2,3*count+3);
		count++;
	    }
	}
	(void) fclose(file);
	printf("leave tecplot_blk_intfc_plot\n");
}	/* end tecplot_blk_intfc_plot */


EXPORT  void tecplot_triad(
	const char *fname, 
	double	   *p,
	double      *x,
	double      *y,
	double      *z,
	double	   ds)
{
	FILE 	*file=NULL;
	char	s[256];
	double   pt[3];

	sprintf(s,"%s-%d.plt",fname,pp_mynode());
	printf("tecplot_triad fname %s \n",s);
	
	if ((file = fopen(s,"w")) == NULL)
        {
            (void) printf("WARNING in tecplot_triad(), "
                          "can't open %s\n",fname);
            return;
        }
        
	fprintf(file,"TITLE = \"tecview\"\n"
		       	    "VARIABLES = \"x\", \"y\", \"z\"\n"
			    "ZONE T=\"x\" I = 2 \n");
	fprintf(file, "%24.16e  %24.16e  %24.16e\n", p[0], p[1], p[2]);
	pt[0] = p[0] + ds*x[0];
	pt[1] = p[1] + ds*x[1];
	pt[2] = p[2] + ds*x[2];
	fprintf(file, "%24.16e  %24.16e  %24.16e\n", pt[0], pt[1], pt[2]);

 	fprintf(file, "ZONE T=\"y\" I = 2 \n");
	fprintf(file, "%24.16e  %24.16e  %24.16e\n", p[0], p[1], p[2]);
	pt[0] = p[0] + ds*y[0];
	pt[1] = p[1] + ds*y[1];
	pt[2] = p[2] + ds*y[2];
	fprintf(file, "%24.16e  %24.16e  %24.16e\n", pt[0], pt[1], pt[2]);
 	
	fprintf(file, "ZONE T=\"z\" I = 2 \n");
	fprintf(file, "%24.16e  %24.16e  %24.16e\n", p[0], p[1], p[2]);
	pt[0] = p[0] + ds*z[0];
	pt[1] = p[1] + ds*z[1];
	pt[2] = p[2] + ds*z[2];
	fprintf(file, "%24.16e  %24.16e  %24.16e\n", pt[0], pt[1], pt[2]);

	fclose(file);
}

EXPORT	void	tecplot_tris(
	const char *fname,
	TRI        **tris, 
	int        num_tris)
{
	int 	j, k;
	FT_POINT	*p;
	FILE 	*file=NULL;
	char	s[256];
	
	sprintf(s,"%s-%d.plt",fname,pp_mynode());
	printf("tecplot_tris fname %s \n",s);
	
	if ((file = fopen(s,"w")) == NULL)
	{
	    (void) printf("WARNING in tecplot_tris(), "
                          "can't open %s\n",fname);
	    return;
	}

	tecplot_show_tris("tec_tris", tris, num_tris, file);
	
	(void) fclose(file);
}	/* end tecplot_blk_intfc_plot */



EXPORT	void	tecplot_box(
	const char *bname,
	FILE	*file,
	double	*L0,
	double	*U0)
{
	double	L[3], U[3];

	ft_assign(L, L0, 3*FLOAT);
	ft_assign(U, U0, 3*FLOAT);

	L[0] += shx;
	L[1] += shy;
	L[2] += shz;
	
	U[0] += shx;
	U[1] += shy;
	U[2] += shz;
	
	if (bname != NULL)/*direct call */
	{
	    if ((file = fopen(bname,"w")) == NULL)
	    {
		screen("WARNING in tecplot_curve(), "
		       "can't open %s\n",bname);
		return;
	    }
	    (void) fprintf(file,"TITLE = \"tecplot box\"\n"
			   	"VARIABLES = \"x\", \"y\", \"z\"\n");
	}
	/*called from tecplot_interface */
	if (file == NULL)
	{
	    screen("ERROR, in tecplot_curve, file is NULL\n");
	    clean_up(ERROR);
	}
	(void) fprintf(file, "ZONE T=\"BOX\" N=%d, E=%d, DATAPACKING=POINT, ZONETYPE=FEBRICK\n",8,1);
	(void) fprintf(file, "%-9g %-9g %-9g\n", L[0],L[1],L[2]);
	(void) fprintf(file, "%-9g %-9g %-9g\n", U[0],L[1],L[2]);
	(void) fprintf(file, "%-9g %-9g %-9g\n", U[0],U[1],L[2]);
	(void) fprintf(file, "%-9g %-9g %-9g\n", L[0],U[1],L[2]);
	(void) fprintf(file, "%-9g %-9g %-9g\n", L[0],L[1],U[2]);
	(void) fprintf(file, "%-9g %-9g %-9g\n", U[0],L[1],U[2]);
	(void) fprintf(file, "%-9g %-9g %-9g\n", U[0],U[1],U[2]);
	(void) fprintf(file, "%-9g %-9g %-9g\n", L[0],U[1],U[2]);
	(void) fprintf(file, "%d %d %d %d %d %d %d %d\n",1,2,3,4,5,6,7,8);
	
	if(bname != NULL)
	    fclose(file);
}	/* end tecplot_box */

EXPORT  void    tecplot_interface(
		const char	*bname,
		INTERFACE	*intfc)
{
	SURFACE	**s;
	CURVE	**cc;
	FILE	*file;
	int	*i;

	if ((file = fopen(bname,"w")) == NULL)
	{
	    screen("WARNING in tecplot_interface(), "
	           "can't open %s\n",bname);
	    return;
	}
	(void) fprintf(file,"TITLE = \"tecplot interface\"\n"
		   	    "VARIABLES = \"x\", \"y\", \"z\"\n");


	/*print curves */
	for (i=0,cc = intfc->curves; cc && *cc; ++i,++cc)
	{
	    /*printf("Curve %d: cc %p *cc %p\n",i,cc,*cc); */
	    if (!debugging("special"))
	        tecplot_curve(NULL,file,*cc);
	}

	/*print surfaces */
	for (s = intfc->surfaces; s && *s; ++s)
	    if (debugging("special"))
	        tecplot_surface_special(NULL,file,*s);
	    else
		tecplot_surface(NULL,file,*s);

	fclose(file);
}	/* end tecplot_interface */

EXPORT  void    tecplot_curve(
		const char	*bname,
		FILE		*file,
		CURVE		*c)
{
	int	i;
	BOND	*b;
	FT_POINT	*p;

	if (bname != NULL)/*direct call */
	{
	    if ((file = fopen(bname,"w")) == NULL)
	    {
		screen("WARNING in tecplot_curve(), "
		       "can't open %s\n",bname);
		return;
	    }
	    (void) fprintf(file,"TITLE = \"tecplot curve\"\n"
			   	"VARIABLES = \"x\", \"y\", \"z\"\n");
	}
	/*called from tecplot_interface */
	if (file == NULL)
	{
	    screen("ERROR, in tecplot_curve, file is NULL\n");
	    clean_up(ERROR);
	}
	(void) fprintf(file, "ZONE T=\"CURVE\" I=%d\n", c->num_points);
	if (!(b=c->first))
	{
	    screen("WARNING, first bond of the curve is NULL\n");
	    return;
	}
	p = b->start;
	(void) fprintf(file,"%-9g %-9g %-9g\n",Coords(p)[0],
 		   		Coords(p)[1], Coords(p)[2]);
	for (i = 0,b = c->first; b != NULL; ++i,b = b->next)
	{
	    p = b->end;
	    (void) fprintf(file,"%-9g %-9g %-9g\n",Coords(p)[0],
			   Coords(p)[1], Coords(p)[2]);
	}
	if ((i+1) != c->num_points)
	{
	    printf("WARNING, num of points in curve is wrong\n");
	    print_curve(c);
	    clean_up(ERROR);
	}
	if (bname != NULL)
	    fclose(file);
}	/* end tecplot_curve */

#define TEC_MAX_TRIS	2000

LOCAL  double  tst_pt[3] = { 1.01255, 0.1418, 1.435 };
LOCAL  double  tst_pt1[3] = { 1.01255, 0.1418, 1.435 };

EXPORT	void	set_tst_posn(double    *pos)
{
	/*ft_assign(tst_pt, pos, 3*FLOAT); */
	/*ft_assign(tst_pt1, pos, 3*FLOAT); */
}

EXPORT  void    tecplot_surface_in_ball(
		const char	*fname,
		SURFACE		*s)
{
	TRI	*tri, *tris[TEC_MAX_TRIS];
	double	*pt;
	int	i,num_tris;
	
	double	tst_pt[3] = {  0.5227539526042586,     0.6525225774422274,       2.1854392147488 };
	double	tst_pt1[3] = { 0.5227539526042586,     0.6525225774422274,       2.1854392147488 };
	double	tol = 1.0/40.0;
	
	if (!(first_tri(s)))
	{
	    screen("WARNING, first bond of the curve is NULL\n");
	    return;
	}

	print_general_vector("#tst_pt=", tst_pt, 3, "\n");

	num_tris = 0;
	for (tri=first_tri(s); !at_end_of_tri_list(tri,s); tri=tri->next)
	{
	    for (i = 0; i < 3; i++)
	    {
		pt = Coords(Point_of_tri(tri)[i]);
		if(distance_between_positions(pt, tst_pt, 3) < tol || 
		   distance_between_positions(pt, tst_pt1, 3) < tol)
		{
		    if(distance_between_positions(pt, tst_pt, 3) < 0.002)
		    {
		        /*printf("#shape bad tri\n"); */
		        /*print_tri(tri, s->interface); */
		    }

		    tris[num_tris] = tri;
		    num_tris++;
		    if(num_tris > TEC_MAX_TRIS)
		    {
		        printf("ERROR: tecplot_surface_in_ball, use larger TEC_MAX_TRIS.\n");
		        clean_up(ERROR);
		    }
		   break;
		}
	    }
	}

	tecplot_tris(fname, tris, num_tris);
}

void	tri_bound_block(double **, TRI *);
boolean	blocks_sect(double**, double**);

/*bmin bmax are blocks in top grid. */
LOCAL	void tecplot_surface_in_box(
	const char	*tname,
	int		*bmin,
	int		*bmax,
	SURFACE		*s,
	FILE 		*file)
{
	TRI		*tri, *tris[TEC_MAX_TRIS];
	int		i,num_tris;
	static double	**fbox = NULL, **bbox = NULL;
	RECT_GRID	*gr = &topological_grid(s->interface);

	if(fbox == NULL)
	{
	    bi_array(&fbox, 2, 3, FLOAT);
	    bi_array(&bbox, 2, 3, FLOAT);
	}

	for(i=0; i<3; i++)
	{
	    bbox[0][i] = gr->L[i] + gr->h[i]*bmin[i];
	    bbox[1][i] = gr->L[i] + gr->h[i]*(bmax[i]+1);
	}

	if (!(first_tri(s)))
	{
	    screen("WARNING tecplot_surface_in_box, first tri of the curve is NULL\n");
	    return;
	}

	num_tris = 0;
	for (tri=first_tri(s); !at_end_of_tri_list(tri,s); tri=tri->next)
	{
	    tri_bound_block(fbox, tri);
	    if(!blocks_sect(fbox, bbox))
		continue;
	    
	    tris[num_tris] = tri;
	    num_tris++;
	    if(num_tris > TEC_MAX_TRIS)
	    {
	        printf("ERROR: tecplot_surface_in_box, use larger TEC_MAX_TRIS.\n");
	        clean_up(ERROR);
	    }
	}

	tecplot_show_tris(tname, tris, num_tris, file);
}

EXPORT  void    tecplot_interface_in_box(
	const char	*tname,
	FILE		*file,
	int		*bmin,
	int		*bmax,
	INTERFACE	*intfc)
{
	SURFACE	**s;

	for (s = intfc->surfaces; s && *s; ++s)
	{
	    tecplot_surface_in_box(tname, bmin, bmax, *s, file);
	}
}

EXPORT  void    tecplot_surface(
		const char	*bname,
		FILE		*file,
		SURFACE		*s)
{
	TRI	*tri;
	FT_POINT	*p;
	int	i,npts,ntri,num_tris;

	if (bname != NULL)/*direct call */
	{
	    if ((file = fopen(bname,"w")) == NULL)
	    {
		screen("WARNING in tecplot_surface(), "
		       "can't open %s\n",bname);
		return;
	    }
	    (void) fprintf(file,"TITLE = \"tecplot surface\"\n"
			   	"VARIABLES = \"x\", \"y\", \"z\"\n");
	}
	/*called from tecplot_interface */
	if (file == NULL)
	{
	    screen("ERROR, in tecplot_surface, file is NULL\n");
	    clean_up(ERROR);
	}
	if (!(first_tri(s)))
	{
	    screen("WARNING, first bond of the curve is NULL\n");
	    return;
	}

	/*count number of points(npts) and number of tris(ntri) */
	for (tri=first_tri(s),ntri=0; !at_end_of_tri_list(tri,s); tri=tri->next,ntri++)
	{
	    for (i = 0; i < 3; i++)
	    {
		Index_of_point(Point_of_tri(tri)[i]) = -1;
	    }
	}
	for (tri=first_tri(s),npts=0; !at_end_of_tri_list(tri,s); tri=tri->next)
	{
	    for (i = 0; i < 3; i++)
	    {
		p = Point_of_tri(tri)[i];
		if (Index_of_point(p) == -1)
		{
		    Index_of_point(p) = ++npts;
		}
	    }
	}
	/*end counting */
	
	if (Boundary(s))
	    (void) fprintf(file, "ZONE T=\"BOUNDARY SURFACE\" N=%d E=%d\nF=FEPOINT, ET=TRIANGLE\n",
			     npts,ntri);
	else
	    (void) fprintf(file, "ZONE T=\"INTERIOR SURFACE\" N=%d E=%d\nF=FEPOINT, ET=TRIANGLE\n",
			     npts,ntri);

	for (tri=first_tri(s); !at_end_of_tri_list(tri,s); tri=tri->next)
	{
	    for (i = 0; i < 3; i++)
	    {
		Index_of_point(Point_of_tri(tri)[i]) = -1;
	    }
	}
	for (tri=first_tri(s),npts=0; !at_end_of_tri_list(tri,s); tri=tri->next)
	{
	    for (i = 0; i < 3; i++)
	    {
		p = Point_of_tri(tri)[i];
		if (Index_of_point(p) == -1)
		{
		    Index_of_point(p) = ++npts;
		    fprintf(file,"%-9g %-9g %-9g\n",Coords(p)[0],
				 Coords(p)[1],Coords(p)[2]);
		}
	    }
	}
	for (tri=first_tri(s); !at_end_of_tri_list(tri,s); tri=tri->next)
	{
	    for (i = 0; i < 3; i++)
	    {
		fprintf(file,"%d ",Index_of_point(Point_of_tri(tri)[i]));
	    }
	    fprintf(file,"\n");
	}

	if (ntri != s->num_tri)
	{
	    printf("WARNING, num of tri in surface is wrong\n"); 
	}
	if (bname != NULL)
	    fclose(file);
}	/* end tecplot_surface */

LOCAL  void    tecplot_surface_special(
		const char	*bname,
		FILE		*file,
		SURFACE		*s)
{
	TRI	*tri;
	FT_POINT	*p;
	int	i,npts,ntri,num_tris;

	if (bname != NULL)/*direct call */
	{
	    if ((file = fopen(bname,"w")) == NULL)
	    {
		screen("WARNING in tecplot_surface(), "
		       "can't open %s\n",bname);
		return;
	    }
	    (void) fprintf(file,"TITLE = \"tecplot surface\"\n"
			   	"VARIABLES = \"x\", \"y\", \"z\"\n");
	}
	/*called from tecplot_interface */
	if (file == NULL)
	{
	    screen("ERROR, in tecplot_surface, file is NULL\n");
	    clean_up(ERROR);
	}
	if (!(first_tri(s)))
	{
	    screen("WARNING, first bond of the curve is NULL\n");
	    return;
	}

	/*count number of points(npts) and number of tris(ntri) */
	num_tris = 0; /*number of special tris */
	for (tri=first_tri(s),ntri=0; !at_end_of_tri_list(tri,s); tri=tri->next,ntri++)
	{
	    int	side,flag = NO;
	    /*
            for (side = 0; side < 3; ++side)
            {
                if (!is_side_bdry(tri,side) && Tri_on_side(tri,side) == NULL)
		    flag = YES;
	    }
	    */
	    for (i = 0; i < 3; i++)
	    {
		if (Point_flags(Point_of_tri(tri)[i])._user9)
		    flag = YES;
	    }
	    if (flag)
	    {
	        for (i = 0; i < 3; i++)
	        {
		    Index_of_point(Point_of_tri(tri)[i]) = -1;
	        }
		num_tris++;
	    }
	}
	for (tri=first_tri(s),npts=0; !at_end_of_tri_list(tri,s); tri=tri->next)
	{
	    for (i = 0; i < 3; i++)
	    {
		p = Point_of_tri(tri)[i];
		if (Index_of_point(p) == -1)
		{
		    Index_of_point(p) = ++npts;
		}
	    }
	}
	/*end counting */
	
	if (npts ==0 && num_tris ==0)
	{
	    if (bname != NULL)
	 	fclose(file);
	    return;
	}
	if (Coords(p)[2] < 0.1 || Coords(p)[2] > 0.3)
	{
	    if (bname != NULL)
	 	fclose(file);
	    return;
	}

	(void) fprintf(file, "ZONE T=\"SURFACE\" N=%d E=%d\nF=FEPOINT, ET=TRIANGLE\n",
			     npts,num_tris);

	for (tri=first_tri(s); !at_end_of_tri_list(tri,s); tri=tri->next)
	{
	    int	side,flag = NO;
	    /*
            for (side = 0; side < 3; ++side)
            {
                if (!is_side_bdry(tri,side) && Tri_on_side(tri,side) == NULL)
		    flag = YES;
	    }
	    */
	    for (i = 0; i < 3; i++)
	    {
		if (Point_flags(Point_of_tri(tri)[i])._user9)
		    flag = YES;
	    }
	    if (flag)
	    {
	        for (i = 0; i < 3; i++)
	        {
	 	    Index_of_point(Point_of_tri(tri)[i]) = -1;
	        }
	    }
	}
	for (tri=first_tri(s),npts=0; !at_end_of_tri_list(tri,s); tri=tri->next)
	{
	    int	side,flag = NO;
	    /*
            for (side = 0; side < 3; ++side)
            {
                if (!is_side_bdry(tri,side) && Tri_on_side(tri,side) == NULL)
		    flag = YES;
	    }
	    */
	    for (i = 0; i < 3; i++)
	    {
		if (Point_flags(Point_of_tri(tri)[i])._user9)
		    flag = YES;
	    }
	    if(flag)
	    for (i = 0; i < 3; i++)
	    {
		p = Point_of_tri(tri)[i];
		if (Index_of_point(p) == -1)
		{
		    Index_of_point(p) = ++npts;
		    fprintf(file,"%-9g %-9g %-9g\n",Coords(p)[0],
				 Coords(p)[1],Coords(p)[2]);
		}
	    }
	}
	for (tri=first_tri(s); !at_end_of_tri_list(tri,s); tri=tri->next)
	{
	    int	side,flag = NO;
	    /*
            for (side = 0; side < 3; ++side)
            {
                if (!is_side_bdry(tri,side) && Tri_on_side(tri,side) == NULL)
		    flag = YES;
	    }
	    */
	    for (i = 0; i < 3; i++)
	    {
		if (Point_flags(Point_of_tri(tri)[i])._user9)
		    flag = YES;
	    }
	    if(flag)
	    {
	        for (i = 0; i < 3; i++)
	        {
		    fprintf(file,"%d ",Index_of_point(Point_of_tri(tri)[i]));
	        }
	        fprintf(file,"\n");
	    }
	}

	if (ntri != s->num_tri)
	{
	    printf("WARNING, num of tri in surface is wrong\n"); 
	}
	if (bname != NULL)
	    fclose(file);
}	/* end tecplot_surface_special */

/*from bowrg, export surfaces in one zone */
LOCAL	void	tecplot_plot_surfaces(
	INTERFACE     *intfc,
	RECT_GRID     *gr,
	const double   *BBL,
	const double   *BBU,
	boolean          clip,
	const char    *dname,
	const char    *name,
	boolean	      bdry,
	SURFACE_COLOR color1,
	SURFACE_COLOR color2)
{
	FILE	          *file;
	FT_POINT             *p;
	SURFACE	          **s;
	TRI	          *tri;
	boolean              plot_surf,plot_tri;
	double 	          D, intensity = .5;
	double             L[MAXD],U[MAXD],tol[MAXD];
	double	          *crds;
	int	          num_surfs, num_tris, i, j, k, l;
	int               npts, ntris, count = 0;
	static const char *indent = "    ";
	static double      *pts = NULL;
	static int        *verts = NULL;
	static int        alloc_len_verts = 0, alloc_len_pts = 0;
	static char       *fname = NULL;
	static size_t     fname_len = 0;

	fname = get_list_file_name(fname,dname,name,&fname_len);
	
	for (num_tris = 0, s = intfc->surfaces; s && *s; ++s)
	{
	    num_tris += (*s)->num_tri;
	    for (tri=first_tri(*s); !at_end_of_tri_list(tri,*s); tri=tri->next)
	    {
	        for (k = 0; k < 3; ++k)
		    Index_of_point(Point_of_tri(tri)[k]) = -1;
	    }
	}
	
	if (alloc_len_pts < 3*intfc->num_points)
	{
	    if (pts != NULL)
		free(pts);
	    alloc_len_pts = 3*intfc->num_points;
	    uni_array(&pts,alloc_len_pts,FLOAT);
	}
	if (alloc_len_verts < 4*num_tris)
	{
	    if (verts != NULL)
		free(verts);
	    alloc_len_verts = 4*num_tris;
	    uni_array(&verts,alloc_len_verts,INT);
	}
	for (i = 0; i < 3; i++)
	{
	    L[i] = gr->L[i] - 0.5*gr->h[i];
	    U[i] = gr->U[i] + 0.5*gr->h[i];
	    tol[i] = 0.00001*gr->h[i];
	}

        for (npts=0, ntris=0, num_surfs=0, s = intfc->surfaces; s && *s; ++s)
	{
	    if (bdry == YES  &&  !Boundary(*s))
		continue; 
	    if (bdry == NO  &&  Boundary(*s))
		continue;
	    if (clip == YES)
	    {
		plot_surf = NO;
	        for (tri = first_tri(*s); !at_end_of_tri_list(tri,*s); 
		     tri = tri->next)
	        {
		    plot_tri = NO;
		    for (k = 0; k < 3; ++k)
		    {
			crds = Coords(Point_of_tri(tri)[k]);
	                for (l = 0; l < 3; ++l)
			    if ((crds[l] < L[l] - tol[l]) || 
			        (crds[l] > U[l] + tol[l]))
				break;
			if (l == 3) /* a point is inside the domain */
			{
			    plot_tri = plot_surf = YES;
			    break;
		        }
		    }
		    if (plot_tri)
		    {
			for (k = 0; k < 3; ++k)
			{
		            p = Point_of_tri(tri)[k];
			    if (Index_of_point(p) == -1)
			    {
			        crds = Coords(p);
	                        for (l = 0; l < 3; ++l)
				    pts[3*npts+l] = crds[l];
				Index_of_point(p) = npts++;
			    }
			    verts[4*ntris+k] = Index_of_point(p);
			}
			verts[4*ntris+3] = num_surfs;
			++ntris;
		    }
		}
		if (plot_surf == YES)
		    ++num_surfs;
	    }
	    else
	    {
	        for (tri = first_tri(*s); !at_end_of_tri_list(tri,*s); 
		     tri = tri->next)
	        {
	            for (k = 0; k < 3; ++k)
		    {
		        p = Point_of_tri(tri)[k];
			if (Index_of_point(p) == -1)
			{
			    crds = Coords(p);
	                    for (l = 0; l < 3; ++l)
				pts[3*npts+l] = crds[l];
			    Index_of_point(p) = npts++;
			}
			verts[4*ntris+k] = Index_of_point(p);
		    }
		    verts[4*ntris+3] = num_surfs;
		    ++ntris;
		}
		++num_surfs;
	    }
	}
	if (num_surfs == 0)
	    return;

	if ((file = fopen(fname,"w")) == NULL)
	{
	    (void) printf("WARNING in tecplot_plot_surfaces(), "
	                  "can't open %s\n",fname);
	    return;
	}

	fprintf(file, "TITLE = \"tecview\"\n");
	fprintf(file, "VARIABLES = \"x\", \"y\", \"z\"\n");
	fprintf(file, "ZONE N=%d,  E=%d,\n", npts, ntris); 
	fprintf(file, "DATAPACKING = POINT \n");
	fprintf(file, "ZONETYPE=FETRIANGLE \n\n");

	for (i = 0; i < npts; ++i)
	{
	    (void) fprintf(file,"%-9g %-9g %-9g\n",
			   pts[3*i],pts[3*i+1],pts[3*i+2]);
	}
	
	for (j = 0; j < ntris; ++j)
	{
	    (void) fprintf(file," %-4d %-4d %-4d \n", 
			   verts[4*j]+1,verts[4*j+1]+1,verts[4*j+2]+1);
	}

	(void) fclose(file);
}		/*end tecplot_plot_surfaces*/

EXPORT	void tecplot_show_box_tri(
	const char	*tname,
	RECT_BOX	*box,
	TRI		**tris,
	int		num_tris,
	FILE 		*file)
{
	int 	i,j;
	double	lc[3], rc[3];
	double 	*L = box->grid->L;
	double 	*U = box->grid->U;
	double 	*h = box->grid->h;

	for(i=0; i<3; i++)
	{
	    lc[i] = L[i] + box->bmin[i]*h[i];
	    rc[i] = L[i] + box->bmax[i]*h[i];
	}
	fprintf(file,"TITLE = \"tecplot tris\"\n"
		     "VARIABLES = \"x\", \"y\", \"z\"\n");

	tecplot_box(NULL, file, lc, rc);
	tecplot_show_tris(tname, tris, num_tris, file);
}	/* end tecplot_show_box_tri */

EXPORT void geomview_interface_plot(
	const char *dname,
	INTERFACE  *intfc,
	RECT_GRID  *gr)
{
	double *BBL = topological_grid(intfc).GL;
	double *BBU = topological_grid(intfc).GU;

	if (intfc->dim != 3)
	    return;
	if (create_directory(dname,YES) == FUNCTION_FAILED)
	{
	    (void) printf("WARNING in geomview_interface_plot(), directory "
			  "%s doesn't exist and can't be created\n",dname);
	    return;
	}

	gview_plot_cube(dname,"grid",gr->L,gr->U,BBL,BBU);
	gview_plot_cube(dname,"vgrid",gr->VL,gr->VU,BBL,BBU);
	gview_plot_cube(dname,"ggrid",gr->GL,gr->GU,BBL,BBU);
	gview_plot_axes(dname,"axes",BBL,BBU,BBL,BBU);

	gview_plot_surfaces(intfc,gr,BBL,BBU,YES,dname,"surfs",
			    NO,pRED,pRED);

	gview_plot_surfaces(intfc,gr,BBL,BBU,YES,dname,"bdry",
			    YES,pBLUE,pGREEN);

	gview_plot_curves(intfc,BBL,BBU,dname,"curves",pYELLOW,1);
}		/*end geomview_interface_plot*/

/*	This function plots two rings of triangles around the point  *
 *	p. It assumes that one triangle is attached to the point.    */

EXPORT	void gview_point_tri_rings(
	const char *filename,         
	FT_POINT *p)
{
	FILE *file = fopen(filename,"w");
	INTERFACE *intfc = p->hs->interface;
	TRI **ptris;
	FT_POINT *pp,*pts[20];
	int i,j,k,np1,nt,nt1,nt2,num_bonds;
	TRI *tris1[20],*tris2[40],*t;
	boolean pp_in_list,tri_in_list;
	static const char *indent = "    ";

	nt1 = set_tri_list_around_point(p,Tri_of_hse(p->hse),&ptris,intfc);
	np1 = 0;
	for (i = 0; i < nt1; ++i)
	{
	    t = tris1[i] = ptris[i];
	    for (j = 0; j < 3; ++j)
	    {
                pp = Point_of_tri(t)[j];
                if (pp == p) continue;
                pp_in_list = pointer_in_list((POINTER)p,np1,(POINTER*)pts);
                if (!pp_in_list)
                {
                    pp->hse = Hyper_surf_element(t);
                    pts[np1++] = pp;
                }
	    }
	}
	nt2 = 0;
	for (i = 0; i < np1; ++i)
	{
	    nt = set_tri_list_around_point(pts[i],Tri_of_hse(pts[i]->hse),
					&ptris,intfc);
	    for (j = 0; j < nt; ++j)
	    {
	    	t = ptris[j];
		tri_in_list = pointer_in_list((POINTER)t,nt1,(POINTER*)tris1);
		if (tri_in_list) continue;
		tri_in_list = pointer_in_list((POINTER)t,nt2,(POINTER*)tris2);
		if (!tri_in_list)
		    tris2[nt2++] = t;
	    }
	}
	(void) fprintf(file,"{ LIST\n");
	(void) fprintf(file,"%s{\n%s%sOFF\n%s%s%6d %6d %6d\n",
			indent,indent,indent,
			indent,indent,3*nt1,nt1,0);
	for (i = 0; i < nt1; ++i)
	{
	    for (j = 0; j < 3; ++j)
	    {
		p = Point_of_tri(tris1[i])[j];
		(void) fprintf(file, "%s%s%-9g %-9g %-9g\n",
			indent,indent,
			Coords(p)[0],Coords(p)[1],Coords(p)[2]);
	    }
	}
	for (i = 0; i < nt1; ++i)
	{
	    (void) fprintf(file,"%s%s%-4d %-4d %-4d %-4d ",indent,indent,
		3,3*i,3*i+1,3*i+2);
	    write_color(file,pRED,0.75);
	    (void) fprintf(file,"\n");
	}
	(void) fprintf(file,"%s}\n",indent);
	(void) fprintf(file,"%s{\n%s%sOFF\n%s%s%6d %6d %6d\n",
			indent,indent,indent,
			indent,indent,3*nt2,nt2,0);
	for (i = 0; i < nt2; ++i)
	{
	    for (j = 0; j < 3; ++j)
	    {
		p = Point_of_tri(tris2[i])[j];
		(void) fprintf(file, "%s%s%-9g %-9g %-9g\n",
			indent,indent,
			Coords(p)[0],Coords(p)[1],Coords(p)[2]);
	    }
	}
	for (i = 0; i < nt2; ++i)
	{
	    (void) fprintf(file,"%s%s%-4d %-4d %-4d %-4d ",indent,indent,
		3,3*i,3*i+1,3*i+2);
	    write_color(file,pGREEN,0.75);
	    (void) fprintf(file,"\n");
	}
	(void) fprintf(file,"%s}\n",indent);
	(void) fprintf(file,"}\n");
	(void) fclose(file);

}	/* end gview_point_tri_rings */

EXPORT  void gview_plot_color_interface(
        const char *dname,
        INTERFACE  *intfc,
        boolean       colored)
{
        RECT_GRID *gr = &topological_grid(intfc);
        double *BBL = topological_grid(intfc).GL;
        double *BBU = topological_grid(intfc).GU;

	if (intfc == NULL)
            return;

	if (intfc->dim != 3)
            return;
        if (create_directory(dname,YES) == FUNCTION_FAILED)
        {
            (void) printf("WARNING in geomview_interface_plot(), directory "
                          "%s doesn't exist and can't be created\n",dname);
            return;
        }
        gview_plot_cube(dname,"grid",gr->L,gr->U,BBL,BBU);
        gview_plot_cube(dname,"vgrid",gr->VL,gr->VU,BBL,BBU);
        gview_plot_cube(dname,"ggrid",gr->GL,gr->GU,BBL,BBU);
        gview_plot_axes(dname,"axes",BBL,BBU,BBL,BBU);

        if (colored == YES)
            gview_plot_color_surfaces(intfc,gr,BBL,BBU,YES,dname,"color_surfs",NO);
        else
        {
            gview_plot_surfaces(intfc,gr,BBL,BBU,YES,dname,"surfs",
                            NO,pRED,pRED);
            gview_plot_surfaces(intfc,gr,BBL,BBU,YES,dname,"bdry",
                            YES,pBLUE,pGREEN);
            gview_plot_curves(intfc,BBL,BBU,dname,"curves",pYELLOW,5);
        }
}               /*end gview_plot_color_interface*/

LOCAL	void	gview_plot_surfaces(
	INTERFACE     *intfc,
	RECT_GRID     *gr,
	const double   *BBL,
	const double   *BBU,
	boolean          clip,
	const char    *dname,
	const char    *name,
	boolean	      bdry,
	SURFACE_COLOR color1,
	SURFACE_COLOR color2)
{
	FILE	          *file;
	FT_POINT             *p;
	SURFACE	          **s;
	TRI	          *tri;
	boolean              plot_surf,plot_tri;
	double 	          D, intensity = .5;
	double             L[MAXD],U[MAXD],tol[MAXD];
	double	          *crds;
	int	          num_surfs, num_tris, i, j, k, l;
	int               npts, ntris, count = 0;
	static const char *indent = "    ";
	static double      *pts = NULL;
	static int        *verts = NULL;
	static int        alloc_len_verts = 0, alloc_len_pts = 0;
	static char       *fname = NULL;
	static size_t     fname_len = 0;

	fname = get_list_file_name(fname,dname,name,&fname_len);

	for (num_tris = 0, s = intfc->surfaces; s && *s; ++s)
	{
	    num_tris += (*s)->num_tri;
	    for (tri=first_tri(*s); !at_end_of_tri_list(tri,*s); tri=tri->next)
	    {
	        for (k = 0; k < 3; ++k)
		    Index_of_point(Point_of_tri(tri)[k]) = -1;
	    }
	}
	
	if (alloc_len_pts < 3*intfc->num_points)
	{
	    if (pts != NULL)
		free(pts);
	    alloc_len_pts = 3*intfc->num_points;
	    uni_array(&pts,alloc_len_pts,FLOAT);
	}
	if (alloc_len_verts < 4*num_tris)
	{
	    if (verts != NULL)
		free(verts);
	    alloc_len_verts = 4*num_tris;
	    uni_array(&verts,alloc_len_verts,INT);
	}
	for (i = 0; i < 3; i++)
	{
	    L[i] = gr->L[i] - 0.5*gr->h[i];
	    U[i] = gr->U[i] + 0.5*gr->h[i];
	    tol[i] = 0.00001*gr->h[i];
	}

        for (npts=0, ntris=0, num_surfs=0, s = intfc->surfaces; s && *s; ++s)
	{
	    if (bdry == YES  &&  !Boundary(*s))
		continue; 
	    if (bdry == NO  &&  Boundary(*s))
		continue;
	    if (clip == YES)
	    {
		plot_surf = NO;
	        for (tri = first_tri(*s); !at_end_of_tri_list(tri,*s); 
		     tri = tri->next)
	        {
		    plot_tri = NO;
		    for (k = 0; k < 3; ++k)
		    {
			crds = Coords(Point_of_tri(tri)[k]);
	                for (l = 0; l < 3; ++l)
			    if ((crds[l] < L[l] - tol[l]) || 
			        (crds[l] > U[l] + tol[l]))
				break;
			if (l == 3) /* a point is inside the domain */
			{
			    plot_tri = plot_surf = YES;
			    break;
		        }
		    }
		    if (plot_tri)
		    {
			for (k = 0; k < 3; ++k)
			{
		            p = Point_of_tri(tri)[k];
			    if (Index_of_point(p) == -1)
			    {
			        crds = Coords(p);
	                        for (l = 0; l < 3; ++l)
				    pts[3*npts+l] = crds[l];
				Index_of_point(p) = npts++;
			    }
			    verts[4*ntris+k] = Index_of_point(p);
			}
			verts[4*ntris+3] = num_surfs;
			++ntris;
		    }
		}
		if (plot_surf == YES)
		    ++num_surfs;
	    }
	    else
	    {
	        for (tri = first_tri(*s); !at_end_of_tri_list(tri,*s); 
		     tri = tri->next)
	        {
	            for (k = 0; k < 3; ++k)
		    {
		        p = Point_of_tri(tri)[k];
			if (Index_of_point(p) == -1)
			{
			    crds = Coords(p);
	                    for (l = 0; l < 3; ++l)
				pts[3*npts+l] = crds[l];
			    Index_of_point(p) = npts++;
			}
			verts[4*ntris+k] = Index_of_point(p);
		    }
		    verts[4*ntris+3] = num_surfs;
		    ++ntris;
		}
		++num_surfs;
	    }
	}
	if (num_surfs == 0)
	    return;

	if ((file = fopen(fname,"w")) == NULL)
	{
	    (void) printf("WARNING in gview_plot_surfaces(), "
	                  "can't open %s\n",fname);
	    return;
	}
	(void) fprintf(file,"{ LIST\n");

	gview_bounding_box(file,BBL,BBU,1,indent);

	(void) fprintf(file,"%s{\n%s%sOFF\n%s%s%6d %6d %6d\n",indent,
		       indent,indent,indent,indent,npts,ntris,0);
	for (i = 0; i < npts; ++i)
	{
	    (void) fprintf(file,"%s%s%-9g %-9g %-9g\n",indent,indent,
			   pts[3*i],pts[3*i+1],pts[3*i+2]);
	}
	D = (num_surfs == 1) ? 1.0 : 1/(num_surfs - 1.0);
	for (j = 0; j < ntris; ++j)
	{
	    (void) fprintf(file,"%s%s%-4d %-4d %-4d %-4d ",indent,indent,
			   3,verts[4*j],verts[4*j+1],verts[4*j+2]);
	    write_interpolated_color(file,color1,color2,verts[4*j+3]/D,
				     intensity);
	}
	(void) fprintf(file,"%s}\n",indent);
	(void) fprintf(file,"}\n");
	(void) fclose(file);
}		/*end gview_plot_surfaces*/

EXPORT	void	set_tri_list_bounding_box(
	TRI   **tris,
	int   nt,
	double *BBL,
	double *BBU,
	boolean  preset,
	boolean  cube)
{
    	const FT_POINT *p;
	int   i, j, k;

	if (!preset)
	{
	    BBL[0] = BBL[1] = BBL[2] =  HUGE_VAL;
	    BBU[0] = BBU[1] = BBU[2] = -HUGE_VAL;
	}
	for (i = 0; i < nt; ++i)
	{
	    if (tris[i])
	    {
	        for (j = 0; j < 3; ++j)
	        {
	            p = Point_of_tri(tris[i])[j];
		    for (k = 0; k < 3; ++k)
		    {
	                if (Coords(p)[k] < BBL[k])
		            BBL[k] = Coords(p)[k];
		        if (Coords(p)[k] > BBU[k])
			    BBU[k] = Coords(p)[k];
		    }
	        }
	    }
	}
	if (cube)
	{
	    double len, mid;

	    len = BBU[0]-BBL[0];
	    for (j = 1; j < 3; ++j)
	    {
		if (len < (BBU[j]-BBL[j]))
		    len = BBU[j]-BBL[j];
	    }
	    for (j = 0; j < 3; ++j)
	    {
		if ((BBU[j]-BBL[j]) < len)
		{
		    mid = 0.5*(BBU[j]+BBL[j]);
		    BBL[j] = mid - 0.5*len;
		    BBU[j] = mid + 0.5*len;
		}
	    }
	}
}		/*end set_tri_list_boundary_box*/

EXPORT	void	set_point_list_bounding_box(
	FT_POINT **pt,
	int   npt,
	double *BBL,
	double *BBU,
	boolean  preset,
	boolean  cube)
{
	const FT_POINT *p;
	int i, j;

	if (!preset)
	{
	    BBL[0] = BBL[1] = BBL[2] =  HUGE_VAL;
	    BBU[0] = BBU[1] = BBU[2] = -HUGE_VAL;
	}
	for (i = 0; i < npt; ++i)
	{
	    p = pt[i];
	    for (j = 0; j < 3; ++j)
	    {
	        if (Coords(p)[j] < BBL[j])
	            BBL[j] = Coords(p)[j];
		if (Coords(p)[j] > BBU[j])
		    BBU[j] = Coords(p)[j];
	    }
	}
	if (cube)
	{
	    double len, mid;

	    len = BBU[0]-BBL[0];
	    for (j = 1; j < 3; ++j)
	    {
		if (len < (BBU[j]-BBL[j]))
		    len = BBU[j]-BBL[j];
	    }
	    for (j = 0; j < 3; ++j)
	    {
		if ((BBU[j]-BBL[j]) < len)
		{
		    mid = 0.5*(BBU[j]+BBL[j]);
		    BBL[j] = mid - 0.5*len;
		    BBU[j] = mid + 0.5*len;
		}
	    }
	}
}		/*end set_point_list_bounding_box*/

EXPORT	void set_vector_bounding_box(
	const double *p,
	const double *u,
	double       c,
	double       *BBL,
	double       *BBU,
	boolean        preset,
	boolean        cube)
{
	double x;
	int j;

	if (!preset)
	{
	    BBL[0] = BBL[1] = BBL[2] =  HUGE_VAL;
	    BBU[0] = BBU[1] = BBU[2] = -HUGE_VAL;
	}
	for (j = 0; j < 3; ++j)
	{
	    x = p[j] + c*u[j];
	    if (x < BBL[j])
		BBL[j] = x;
	    if (x > BBU[j])
		BBU[j] = x;
	}
	if (cube)
	{
	    double len, mid;

	    len = BBU[0]-BBL[0];
	    for (j = 1; j < 3; ++j)
	    {
		if (len < (BBU[j]-BBL[j]))
		    len = BBU[j]-BBL[j];
	    }
	    for (j = 0; j < 3; ++j)
	    {
		if ((BBU[j]-BBL[j]) < len)
		{
		    mid = 0.5*(BBU[j]+BBL[j]);
		    BBL[j] = mid - 0.5*len;
		    BBU[j] = mid + 0.5*len;
		}
	    }
	}
}		/*end set_vector_bounding_box*/

EXPORT	void	gview_plot_triangle_list(
	const char  *dname,
	const char  *name,
	TRI         **tris,
	int         num_tris,
	double       red_start,
	double       green_start,
	double       blue_start,
	double       red_end,
	double       green_end,
	double       blue_end,
	double       alpha,
	const double *BBL,
	const double *BBU)
{
	FILE              *file;
	FT_POINT             **p;
	double             red, green, blue;
	double             D, x;
	int               nt, i, j;
	static char       *fname = NULL;
	static size_t     fname_len = 0;
	static const char *indent = "    ";

	fname = get_list_file_name(fname,dname,name,&fname_len);
	if ((file = fopen(fname,"w")) == NULL)
	{
	    (void) printf("WARNING in gview_plot_triangle_list(), "
	                  "can't open %s\n",fname);
	    return;
	}
	for (nt = 0, i = 0; i < num_tris; ++i)
	    if (tris[i] != NULL) ++nt;

	(void) fprintf(file,"{ LIST\n");
	gview_bounding_box(file,BBL,BBU,1,indent);

	(void) fprintf(file,"%s{\n%s%sOFF\n%s%s%6d %6d %6d\n",indent,
		       indent,indent,indent,indent,3*nt,nt,0);
	for (i = 0; i < num_tris; ++i)
	{
	    if (tris[i] != NULL)
	    {
	        p = Point_of_tri(tris[i]);
	        for (j = 0; j < 3; ++j)
	            (void) fprintf(file,"%s%s%-9g %-9g %-9g\n",indent,indent,
			           Coords(p[j])[0],Coords(p[j])[1],
				   Coords(p[j])[2]);
	    }
	}
	D = num_tris-1;
	for (i = 0; i < num_tris; ++i)
	{
	    x = D==0 ? 1.0 : i/D;

	    red = (1.0 - x)*red_start + x*red_end;
	    if (red < 0.0) red = 0.0; if (red > 1.0) red = 1.0;
	    green = (1.0 - x)*green_start + x*green_end;
	    if (green < 0.0) green = 0.0; if (green > 1.0) green = 1.0;
	    blue = (1.0 - x)*blue_start + x*blue_end;
	    if (blue < 0.0) blue = 0.0; if (blue > 1.0) blue = 1.0;
	    if (tris[i] != NULL)
	    {
	        (void) fprintf(file,"%s%s%-4d %-4d %-4d %-4d %g %g %g %g\n",
			       indent,indent,3,3*i,3*i+1,3*i+2,
			       red,green,blue,alpha);
	    }
	}
	(void) fprintf(file,"%s}\n",indent);

	(void) fprintf(file,"}\n");
	(void) fclose(file);
}		/*end gview_plot_triangle_list*/

EXPORT	void	gview_plot_polyline(
	const char  *dname,
	const char  *name,
	FT_POINT       **v,
	int 	    nv,
	boolean        closed,
	double       red,
	double       green,
	double       blue,
	double       alpha,
	const double *BBL,
	const double *BBU)
{
	FILE              *file;
	int               i;
	static char       *fname = NULL;
	static size_t     fname_len = 0;
	static const char *indent = "    ";

	fname = get_list_file_name(fname,dname,name,&fname_len);
	if ((file = fopen(fname,"w")) == NULL)
	{
	    (void) printf("WARNING in gview_plot_polyline(), "
	                  "can't open %s\n",fname);
	    return;
	}

	(void) fprintf(file,"{ LIST\n");
	gview_bounding_box(file,BBL,BBU,1,indent);

	(void) fprintf(file,"%s{\n%s%sVECT\n",indent,indent,indent);
	(void) fprintf(file,"%s%s%6d %6d %6d\n",indent,indent,
		       1,(closed)?nv+1:nv,1);
	(void) fprintf(file,"%s%s%6d\n",indent,indent,(closed)?nv+1:nv);
	(void) fprintf(file,"%s%s%6d\n",indent,indent,1);
	for (i = 0; i < nv; ++i)
	    (void) fprintf(file,"%s%s%g %g %g\n",indent,indent,
			   Coords(v[i])[0],Coords(v[i])[1],Coords(v[i])[2]);
	if (closed)
	    (void) fprintf(file,"%s%s%g %g %g\n",indent,indent,
			   Coords(v[0])[0],Coords(v[0])[1],Coords(v[0])[2]);
	(void) fprintf(file,"%s%s%g %g %g %g\n",indent,indent,
		       red,green,blue,alpha);
	(void) fprintf(file,"%s}\n",indent);
	(void) fprintf(file,"}\n");
	(void) fclose(file);
}		/*end gview_plot_polyline*/

EXPORT	void	gview_plot_vertices(
	const char  *dname,
	const char  *name,
	FT_POINT       **v,
	int 	    nv,
	const double *BBL,
	const double *BBU)
{
	FILE              *file;
	double             r;
	int               i;
	static char       *fname = NULL;
	static size_t     fname_len = 0;
	static const char *indent = "    ";

	if ((v == NULL) || (nv <= 0))
	    return;
	fname = get_list_file_name(fname,dname,name,&fname_len);
	if ((file = fopen(fname,"w")) == NULL)
	{
	    (void) printf("WARNING in gview_plot_vertices(), "
	                  "can't open %s\n",fname);
	    return;
	}

	r = 0.01*sqrt((BBU[0]-BBL[0])*(BBU[0]-BBL[0]) +
	              (BBU[1]-BBL[1])*(BBU[1]-BBL[1]) +
	              (BBU[2]-BBL[2])*(BBU[2]-BBL[2]));
	(void) fprintf(file,"{ LIST\n");
	gview_bounding_box(file,BBL,BBU,1,indent);
	for (i = 0; i < nv; ++i)
	{
	    (void) fprintf(file,"%s{\n%s%sSPHERE\n",indent,indent,indent);
	    (void) fprintf(file,"%s%s%g\n",indent,indent,r);
	    (void) fprintf(file,"%s%s%g %g %g\n",indent,indent,
			   Coords(v[i])[0],Coords(v[i])[1],Coords(v[i])[2]);
	    (void) fprintf(file,"%s}\n",indent);
	}
	(void) fprintf(file,"}\n");
	(void) fclose(file);
}		/*end gview_plot_vertices*/

EXPORT	void	gview_plot_axes(
	const char  *dname,
	const char  *name,
	const double *L,
	const double *U,
	const double *BBL,
	const double *BBU)
{
	FILE              *file;
	static char       *fname = NULL;
	static size_t     fname_len = 0;
	static const char *indent = "    ";

	fname = get_list_file_name(fname,dname,name,&fname_len);

	if ((file = fopen(fname,"w")) == NULL)
	{
	    (void) printf("WARNING in gview_plot_axes(), "
	                  "can't open %s\n",fname);
	    return;
	}
	(void) fprintf(file,"{ LIST\n");

	gview_bounding_box(file,BBL,BBU,1,indent);

	/* Print Coordinate axes */
	(void) fprintf(file,"%s{\n",indent);
	(void) fprintf(file,"%s%sVECT\n",indent,indent);
	(void) fprintf(file,"%s%s%6d %6d %6d\n",indent,indent,3,6,3);
	(void) fprintf(file,"%s%s%6d %6d %6d\n",indent,indent,2,2,2);
	(void) fprintf(file,"%s%s%6d %6d %6d\n",indent,indent,1,1,1);
	(void) fprintf(file,"%s%s%-9g %-9g %-9g\n",
			    indent,indent,L[0],L[1],L[2]);
	(void) fprintf(file,"%s%s%-9g %-9g %-9g\n",
			    indent,indent,U[0],L[1],L[2]);
	(void) fprintf(file,"%s%s%-9g %-9g %-9g\n",
			    indent,indent,L[0],L[1],L[2]);
	(void) fprintf(file,"%s%s%-9g %-9g %-9g\n",
			    indent,indent,L[0],U[1],L[2]);
	(void) fprintf(file,"%s%s%-9g %-9g %-9g\n",
			    indent,indent,L[0],L[1],L[2]);
	(void) fprintf(file,"%s%s%-9g %-9g %-9g\n",
			    indent,indent,L[0],L[1],U[2]);
	(void) fprintf(file,"%s%s%6g %6g %6g %6g\n",
			    indent,indent,1.0,0.0,0.0,0.0);
	(void) fprintf(file,"%s%s%6g %6g %6g %6g\n",
			    indent,indent,0.0,1.0,0.0,0.0);
	(void) fprintf(file,"%s%s%6g %6g %6g %6g\n",
			    indent,indent,0.0,0.0,1.0,0.0);
	(void) fprintf(file,"%s}\n",indent);

	(void) fprintf(file,"}\n");
	(void) fclose(file);
}		/*end gview_plot_axes*/

EXPORT	void	gview_plot_coord_sys(
	const char  *dname,
	const char  *name,
	const double *o,
	const double *a0,
	const double *a1,
	const double *a2,
	const double *BBL,
	const double *BBU)
{
	FILE              *file;
	static char       *fname = NULL;
	static size_t     fname_len = 0;
	static const char *indent = "    ";

	fname = get_list_file_name(fname,dname,name,&fname_len);

	if ((file = fopen(fname,"w")) == NULL)
	{
	    (void) printf("WARNING in gview_plot_coord_sys(), "
	                  "can't open %s\n",fname);
	    return;
	}
	(void) fprintf(file,"{ LIST\n");

	gview_bounding_box(file,BBL,BBU,1,indent);

	/* Print Coordinate axes */
	(void) fprintf(file,"%s{\n",indent);
	(void) fprintf(file,"%s%sVECT\n",indent,indent);
	(void) fprintf(file,"%s%s%6d %6d %6d\n",indent,indent,3,6,3);
	(void) fprintf(file,"%s%s%6d %6d %6d\n",indent,indent,2,2,2);
	(void) fprintf(file,"%s%s%6d %6d %6d\n",indent,indent,1,1,1);
	(void) fprintf(file,"%s%s%-9g %-9g %-9g\n",
			    indent,indent,o[0]-a0[0],o[1]-a0[1],o[2]-a0[2]);
	(void) fprintf(file,"%s%s%-9g %-9g %-9g\n",
			    indent,indent,o[0]+a0[0],o[1]+a0[1],o[2]+a0[2]);
	(void) fprintf(file,"%s%s%-9g %-9g %-9g\n",
			    indent,indent,o[0]-a1[0],o[1]-a1[1],o[2]-a1[2]);
	(void) fprintf(file,"%s%s%-9g %-9g %-9g\n",
			    indent,indent,o[0]+a1[0],o[1]+a1[1],o[2]+a1[2]);
	(void) fprintf(file,"%s%s%-9g %-9g %-9g\n",
			    indent,indent,o[0]-a2[0],o[1]-a2[1],o[2]-a2[2]);
	(void) fprintf(file,"%s%s%-9g %-9g %-9g\n",
			    indent,indent,o[0]+a2[0],o[1]+a2[1],o[2]+a2[2]);
	(void) fprintf(file,"%s%s%6g %6g %6g %6g\n",
			    indent,indent,1.0,0.0,0.0,0.0);
	(void) fprintf(file,"%s%s%6g %6g %6g %6g\n",
			    indent,indent,0.0,1.0,0.0,0.0);
	(void) fprintf(file,"%s%s%6g %6g %6g %6g\n",
			    indent,indent,0.0,0.0,1.0,0.0);
	(void) fprintf(file,"%s}\n",indent);

	(void) fprintf(file,"}\n");
	(void) fclose(file);
}		/*end gview_plot_coord_sys*/

EXPORT	void	gview_bounding_box(
	FILE        *file,
	const double *BBL,
	const double *BBU,
	int         indent_level,
	const char  *indent)
{
	char bindent[256];
	char vfmt[256];
	int  i;

	bindent[0] = '\0';
	for (i = 0; i < indent_level; ++i)
	    (void) strcat(bindent,indent);

	(void) sprintf(vfmt,"%s%s%s\n",bindent,indent,"%-9g %-9g %-9g");
	/* Print bounding verticies */
	(void) fprintf(file,"%s{\n",bindent);
	(void) fprintf(file,"%s%sSKEL\n",bindent,indent);
	(void) fprintf(file,"%s%s%6d %6d\n",bindent,indent,8,8);
	(void) fprintf(file,vfmt,BBL[0],BBL[1],BBL[2]);
	(void) fprintf(file,vfmt,BBU[0],BBL[1],BBL[2]);
	(void) fprintf(file,vfmt,BBU[0],BBU[1],BBL[2]);
	(void) fprintf(file,vfmt,BBL[0],BBU[1],BBL[2]);
	(void) fprintf(file,vfmt,BBL[0],BBL[1],BBU[2]);
	(void) fprintf(file,vfmt,BBU[0],BBL[1],BBU[2]);
	(void) fprintf(file,vfmt,BBU[0],BBU[1],BBU[2]);
	(void) fprintf(file,vfmt,BBL[0],BBU[1],BBU[2]);
	for (i = 0; i < 8; ++i)
	    (void) fprintf(file,"%s%s%-9d %-9d %-9d\n",bindent,indent,2,i,i);
	(void) fprintf(file,"%s}\n",bindent);
}		/*end gview_bounding_box*/

LOCAL void write_interpolated_color(
	FILE          *file,
	SURFACE_COLOR color1,
	SURFACE_COLOR color2,
	double         d,
	double         intensity)
{
  	double color[8][4] =
	{
	    { 0.0, 0.0, 0.0, 0.75 }, /* black     */
	    { 1.0, 0.0, 0.0, 0.75 }, /* red       */
	    { 0.0, 1.0, 0.0, 0.75 }, /* green     */
	    { 1.0, 1.0, 0.0, 0.75 }, /* yellow    */
	    { 0.0, 0.0, 1.0, 0.75 }, /* blue      */ 
	    { 1.0, 0.0, 1.0, 0.75 }, /* magenta   */
	    { 0.0, 1.0, 1.0, 0.75 }, /* cyan      */
	    { 1.0, 1.0, 1.0, 0.75 }, /* white     */
	};

	double write_color;
	int i;

	for (i = 0 ; i < 4 ; ++i)
        {
	    write_color = ((1.0 - d)*color[color1][i] + d*color[color2][i])*
			  intensity;
	    (void) fprintf(file,"%7.5f ",write_color);
	}
	(void) fprintf(file,"\n");
}		/* end write_interpolated_color */

LOCAL void gview_plot_cube(
	const char *dname,
	const char *gname,
	const double *L,
	const double *U,
	const double *BBL,
	const double *BBU)
{
	FILE              *file;
	char              fmt[256];
	static const char *indent = "    ";
	static char       *fname = NULL;
	static size_t     fname_len = 0;

	fname = get_list_file_name(fname,dname,gname,&fname_len);

	if ((file = fopen(fname,"w")) == NULL)
	{
	    (void) printf("WARNING in gview_plot_cube(), "
	    	          "can't open %s\n",fname);
	    return;
	}
	(void) sprintf(fmt,"%s%s%s\n",indent,indent,
		                      "%-9g %-9g %-9g "
		                      "%-9g %-9g %-9g "
		                      "%-9g %-9g %-9g "
		                      "%-9g %-9g %-9g");
	(void) fprintf(file,"{ LIST\n");
	gview_bounding_box(file,BBL,BBU,1,indent);
	(void) fprintf(file,"%s{\n",indent);
	(void) fprintf(file,"%s%sQUAD\n",indent,indent);
	/* Lower face, z = L[2] */
	(void) fprintf(file,fmt,L[0],L[1],L[2],U[0],L[1],L[2],
				U[0],U[1],L[2],L[0],U[1],L[2]);
	/* Upper face, z = U[2] */
	(void) fprintf(file,fmt,L[0],L[1],U[2],U[0],L[1],U[2],
	                        U[0],U[1],U[2],L[0],U[1],U[2]);
	/* back face, x = L[0] */
	(void) fprintf(file,fmt,L[0],L[1],L[2],L[0],U[1],L[2],
	                        L[0],U[1],U[2],L[0],L[1],U[2]);
	/* front face, x = U[0] */
	(void) fprintf(file,fmt,U[0],L[1],L[2],U[0],U[1],L[2],
	                        U[0],U[1],U[2],U[0],L[1],U[2]);
	/* left face, y = L[1] */
	(void) fprintf(file,fmt,L[0],L[1],L[2],U[0],L[1],L[2],
	                        U[0],L[1],U[2],L[0],L[1],U[2]);
	/* right face, y = U[1] */
	(void) fprintf(file,fmt,L[0],U[1],L[2],U[0],U[1],L[2],
	                        U[0],U[1],U[2],L[0],U[1],U[2]);
	(void) fprintf(file,"%s}\n",indent);
	(void) fprintf(file,"}\n");

	(void) fclose(file);
}		/*end gview_plot_cube*/

LOCAL   void    gview_plot_curves(
        INTERFACE     *intfc,
	const double   *BBL,
	const double   *BBU,
        const char    *dname,
        const char    *name,
	SURFACE_COLOR color,
	int	      width)
{
        FILE              *file;
        FT_POINT             *ps, *pe;
        CURVE             **c;
        BOND              *b;
        static const char *indent = "    ";
        int               num_bonds,i;
	static char       *fname = NULL;
	static size_t     fname_len = 0;

	fname = get_list_file_name(fname,dname,name,&fname_len);

        if ((file = fopen(fname,"w")) == NULL)
        {
	    screen("WARNING in gview_plot_curves(), "
		   "can't open %s\n",fname);
	    return;
        }

        (void) fprintf(file,"{ LIST\n");

	gview_bounding_box(file,BBL,BBU,1,indent);

        for (c = intfc->curves; c && *c; ++c)
        {
            num_bonds = (*c)->num_points - 1;
            (void) fprintf(file,"%s{appearance{*linewidth %d}\n"
			   "%s%sVECT\n%s%s%6d %6d %6d\n",
			   indent,width,indent,indent,indent,indent,
			   num_bonds,2*num_bonds,1);
            /* may make for very long lines! */
            (void) fprintf(file,"%s%s",indent,indent);
            for (i = 0; i < num_bonds; ++i)
            {
                (void) fprintf(file,"2 ");
            }
            (void) fprintf(file,"\n");
            (void) fprintf(file,"%s%s1 ",indent,indent);
            for (i = 0; i < num_bonds - 1; ++i)
            {
                (void) fprintf(file,"0 ");
            }
            (void) fprintf(file,"\n");
            for (b = (*c)->first; b; b = b->next)
            {
                ps = b->start;
                pe = b->end;
                (void) fprintf(file,"%s%s%-9g %-9g %-9g %-9g %-9g %-9g\n",
			       indent,indent,Coords(ps)[0], Coords(ps)[1],
			       Coords(ps)[2],Coords(pe)[0],Coords(pe)[1],
			       Coords(pe)[2]);
            }
	    (void) fprintf(file,"%s%s",indent,indent);
	    write_color(file,color,0.0);
            (void) fprintf(file,"%s}\n",indent);
        }
        (void) fprintf(file,"}\n");
        fclose(file);
}               /*end gview_plot_curves*/

EXPORT  void    gview_plot_curve(
        const CURVE   *c,
        const char    *dname,
        const char    *name,
	SURFACE_COLOR color,
	int	      width)
{
        FILE              *file;
        FT_POINT             *ps,*pe;
        BOND              *b;
	BOND_TRI          **btris;
	INTERFACE         *intfc = c->interface;
	TRI	          **tri_list;
	double             *BBL = topological_grid(intfc).GL;
	double             *BBU = topological_grid(intfc).GU;
        int               num_bonds,i,j,tri_cnt = 0;
        static const char *indent = "    ";
	static char       *fname = NULL, *ppfname = NULL;
	static size_t     fname_len = 0, ppfname_len = 0;

	ppfname = set_ppfname(ppfname,name,&ppfname_len);

	if (create_directory(dname,YES) == FUNCTION_FAILED)
	{
	    screen("WARNING in gview_plot_curve(), "
		   "directory %s doesn't exist and can't be created\n",dname);
	    return;
        }

	fname = get_list_file_name(fname,dname,ppfname,&fname_len);

        if ((file = fopen(fname,"w")) == NULL)
        {
	    screen("WARNING in gview_plot_curve(), "
	           "can't open %s\n",fname);
	    return;
        }


        (void) fprintf(file,"{ LIST\n");

	gview_bounding_box(file,BBL,BBU,0,indent);

	num_bonds = c->num_points - 1;
	(void) fprintf(file,"%s{appearance{*linewidth %d}\n"
		       "%s%sVECT\n%s%s%6d %6d %6d\n",
		       indent,width,indent,indent,indent,indent,
		       num_bonds,2*num_bonds,1);

	(void) fprintf(file,"%s%s",indent,indent);
	for (i = 0; i < num_bonds; ++i)
	{
	    (void) fprintf(file,"2 ");
	}
	(void) fprintf(file,"\n");
	(void) fprintf(file,"%s%s1 ",indent,indent);
	for (i = 0; i < num_bonds - 1; ++i)
	{
	    (void) fprintf(file,"0 ");
	}
	(void) fprintf(file,"\n");
	for (b = c->first; b; b = b->next)
	{
	    ps = b->start;
	    pe = b->end;
	    (void) fprintf(file,"%s%s%-9g %-9g %-9g %-9g %-9g %-9g\n",
			   indent,indent,Coords(ps)[0], Coords(ps)[1],
			   Coords(ps)[2],Coords(pe)[0],Coords(pe)[1],
			   Coords(pe)[2]);
	}
	(void) fprintf(file,"%s",indent);
	write_color(file,color,0.75);
	(void) fprintf(file,"%s}\n",indent);

        (void) fprintf(file,"}\n");

        fclose(file);
	b = c->first;
	for (btris = Btris(b); btris && *btris; ++btris)
	    ++tri_cnt;

	uni_array(&tri_list,num_bonds,sizeof(TRI*));
	for (i = 0; i < tri_cnt; ++i)
	{
	    for (b = c->first, j = 0; b; b = b->next, ++j)
	        tri_list[j] = Btris(b)[i]->tri;
	    gview_plot_tri_list(dname,tri_list,num_bonds);
	}
	free(tri_list);
}               /*end gview_plot_curve*/

EXPORT  void    gview_polyline(
	const char    *dname,
        const char    *fname,
	double *const  *coords,
	int	      n_points,
	SURFACE_COLOR color,
	int	      width)
{
        FILE *file;
	char path[256];
	static char *ppfname = NULL;
	static size_t ppfname_len = 0;

	ppfname = set_ppfname(ppfname,fname,&ppfname_len);

	if (create_directory(dname,YES) == FUNCTION_FAILED)
	{
	    screen("WARNING in gview_polyline(), "
		   "directory %s doesn't exist and can't be created\n",dname);
	    return;
        }
	(void) sprintf(path,"%s/%s.vect",dname,fname);
        if ((file = fopen(path,"w")) == NULL)
        {
	    screen("WARNING in gview_polyline(), "
		   "can't open %s\n",path);
	    return;
        }

	print_polyline_description(file,"",coords,n_points,color,0.75,width);
        fclose(file);
}               /*end gview_polyline*/

LOCAL  void    print_polyline_description(
	FILE          *file,
	const char    *indent,
	double         *const *coords,
	int	      n_points,
	SURFACE_COLOR color,
	double         alpha,
	int	      width)
{
        int  i;

	if ((coords == NULL) || (n_points <= 0))
	    return;
        (void) fprintf(file,"%s{ appearance{*linewidth %d}\n",indent,width);
	(void) fprintf(file,"%s  VECT\n",indent);
	(void) fprintf(file,"%s    %d %d 1\n",indent,n_points-1,2*(n_points-1));

	(void) fprintf(file,"%s    ",indent);
        for (i = 0; i < n_points-1; ++i)
	    (void) fprintf(file,"2 ");
	(void) fprintf(file,"\n%s",indent);
	(void) fprintf(file,"    1 ");
        for (i = 0; i < n_points-2; ++i)
	    (void) fprintf(file,"0 ");

	(void) fprintf(file,"\n");
	for (i = 0; i < n_points-1; ++i)
	{
	    (void) fprintf(file,"%s    %13.8g %13.8g %13.8g"
	                        "    %13.8g %13.8g %13.8g\n",
			   indent,coords[i][0],coords[i][1],coords[i][2],
			   coords[i+1][0],coords[i+1][1],coords[i+1][2]);
	}
	
	(void) fprintf(file,"%s",indent);
	write_color(file,color,alpha);
	(void) fprintf(file,"%s}\n",indent);
}		/*end print_polyline_description*/

/*
*  			gview_local_surface()
*
*	draws the surface only within a ball with specified 
*	radius and center.  Warning, this function is not 
*	fully implemented: it only draws tris with at least one
*	point within the ball.  It is possible for a large 
*	triangle to contain points within the ball but not be drawn.
*/

EXPORT	void	gview_local_surface(
	SURFACE	      *surf,
	const char    *dname,
	const char    *fname,
	SURFACE_COLOR color,
	const double   *center,
	double         radius)
{
	FILE       *file;
	FT_POINT	   *p;
	TRI	   *tri;
	static const char *indent = "    ";
	char   	   path[256];
	int	   num_tris, i, n_tris_written = 0;
	int	   *tri_near_point, tri_cnt;

	if (create_directory(dname,YES) == FUNCTION_FAILED)
	{
	    screen("WARNING in gview_local_surface(),"
		    "directory %s doesn't exist and can't be created\n",dname);
	    return;
	}

	(void) sprintf(path,"%s/%s.off",dname,fname);

	if ((file = fopen(path,"w")) == NULL)
	{
	    screen("WARNING in gview_local_surface(),"
		   "can't open %s\n",path);
	    return;
	}

	num_tris = surf->num_tri;
	uni_array(&tri_near_point,num_tris,sizeof(int));
	for (i = 0; i < num_tris; ++i)
	    tri_near_point[i] = NO;

	tri_cnt = 0;
	for (tri = first_tri(surf); ! at_end_of_tri_list(tri,surf);
	     tri = tri->next)
	{
	    for (i = 0; i < 3; ++i)
	    {
		p = Point_of_tri(tri)[i];
		if (distance_between_positions(Coords(p),center,3) <= radius)
	        {
		    tri_near_point[tri_cnt] = YES;
		    ++n_tris_written;
		    break;
		}
	    }
	    ++tri_cnt;
	}

	(void) fprintf(file,"%s{\n%s%sOFF\n%s%s%6d %6d %6d\n",
			indent,indent,indent,indent,indent,
		       3*n_tris_written,n_tris_written,0);

	tri_cnt = 0;
	for (tri = first_tri(surf); ! at_end_of_tri_list(tri,surf);
	     tri = tri->next)
        {
	    if (tri_near_point[tri_cnt] == YES)
	    {
	        for (i = 0; i < 3; ++i)
		{
		    p = Point_of_tri(tri)[i];
		    (void) fprintf(file,"%s%s%12.8f %12.8f %12.8f\n",
				   indent,indent,
				   Coords(p)[0],Coords(p)[1],Coords(p)[2]);
		}
	    }
	    ++tri_cnt;
	}

	tri_cnt = i = 0;
	for (tri = first_tri(surf); ! at_end_of_tri_list(tri,surf);
	     tri = tri->next)
        {
	    if (tri_near_point[tri_cnt] == YES)
	    {
		(void) fprintf(file,"%s%s%-4d %-4d %-4d %-4d ",
				indent,indent,3,i,i+1,i+2);
		write_color(file,color,0.75);
		i += 3;
	    }
	    ++tri_cnt;
	}
	(void) fprintf(file,"%s}\n",indent);
	(void) fclose(file);
}		/* end gview_local_surface */

LOCAL 	void 	write_color( 		/* for geomview files */
	FILE 	      *file,
	SURFACE_COLOR color,
	double         alpha)
{
  	switch (color)  /* color information  R G B opacity */
	{
	default:
	case pBLACK:
	    (void) fprintf(file,"0.0 0.0 0.0 %g\n",alpha);
	    break;
	case pRED:
	    (void) fprintf(file,"1.0 0.0 0.0 %g\n",alpha); 
	    break;
	case pGREEN:
	    (void) fprintf(file,"0.0 1.0 0.0 %g\n",alpha);
	    break;
	case pYELLOW:
	    (void) fprintf(file,"1.0 1.0 0.0 %g\n",alpha);
	    break;
	case pBLUE:
	    (void) fprintf(file,"0.0 0.0 1.0 %g\n",alpha);
	    break;
	case pMAGENTA:
	    (void) fprintf(file,"1.0 0.0 1.0 %g\n",alpha);
	    break;
	case pCYAN:
	    (void) fprintf(file,"0.0 1.0 1.0 %g\n",alpha);
	    break;
	case pWHITE:
	    (void) fprintf(file,"1.0 1.0 1.0 %g\n",alpha);
	    break;
	}
}  		/*end write_color*/

EXPORT	void	gview_surface(
	SURFACE	      *surface,
	const char    *fname,
	SURFACE_COLOR color)
{
	FILE 	   *file;
	FT_POINT	   *p;
	TRI	   *tri;
	static const char *indent = "    ";
	char	   path[256];
	int	   num_tris, i;
	int	   tri_count = 0;

	if (create_directory("gv",YES) == FUNCTION_FAILED)
	{
	    screen("WARNING in gview_surface(), "
		   "can't open directory gv\n");
	    return;
	}

	if (create_directory("gv/surface",YES) == FUNCTION_FAILED)
	{
	    screen("WARNING in gview_surface(), "
		   "can't open directory gv/surface\n");
	    return;
	}

	(void) sprintf(path,"gv/surface/%s.off",fname);

	if ((file = fopen(path,"w")) == NULL)
	{
	    screen("WARNING in gview_surface(), "
		   "can't open file %s\n",path);
	    return;
	}

	num_tris = surface->num_tri;
	(void) fprintf(file,"%s{\n%s%sOFF\n%s%s%6d %6d %6d\n",indent,
		indent,indent,indent,indent,
		3*num_tris,num_tris,0);
	for (tri = first_tri(surface); !at_end_of_tri_list(tri,surface); 
	     tri = tri->next)
	{
	    ++tri_count;
	    for (i = 0; i < 3; ++i)
	    {
		p = Point_of_tri(tri)[i];
		(void) fprintf(file,"%s%s%-9g %-9g %-9g\n",
			       indent,indent,
			       Coords(p)[0],Coords(p)[1],Coords(p)[2]);
	    }
	}

	i = 0;
	for (tri = first_tri(surface); !at_end_of_tri_list(tri,surface); 
	     tri = tri->next)
	{
	    (void) fprintf(file,"%s%s%-4d %-4d %-4d %-4d ",
			   indent,indent,
			   3,i,i+1,i+2);
	    write_color(file,color,0.75);
	    i += 3;
	}
	(void) fprintf(file,"%s}\n",indent);
	(void) fclose(file);
}		/*end gview_surface*/


EXPORT 	void 	gview_cube(
	FILE        *file,
	const double *L,
	const double *U)
{
	/* Lower face, z = L[2] */
	(void) fprintf(file,"%-9g %-9g %-9g ", L[0],L[1],L[2]);
	(void) fprintf(file,"%-9g %-9g %-9g ", U[0],L[1],L[2]);
	(void) fprintf(file,"%-9g %-9g %-9g ", U[0],U[1],L[2]);
	(void) fprintf(file,"%-9g %-9g %-9g\n",L[0],U[1],L[2]);
	/* Upper face, z = U[2] */
	(void) fprintf(file,"%-9g %-9g %-9g ", L[0],L[1],U[2]);
	(void) fprintf(file,"%-9g %-9g %-9g ", U[0],L[1],U[2]);
	(void) fprintf(file,"%-9g %-9g %-9g ", U[0],U[1],U[2]);
	(void) fprintf(file,"%-9g %-9g %-9g\n",L[0],U[1],U[2]);
	/* back face, x = L[0] */
	(void) fprintf(file,"%-9g %-9g %-9g ", L[0],L[1],L[2]);
	(void) fprintf(file,"%-9g %-9g %-9g ", L[0],U[1],L[2]);
	(void) fprintf(file,"%-9g %-9g %-9g ", L[0],U[1],U[2]);
	(void) fprintf(file,"%-9g %-9g %-9g\n",L[0],L[1],U[2]);
	/* front face, x = U[0] */
	(void) fprintf(file,"%-9g %-9g %-9g ", U[0],L[1],L[2]);
	(void) fprintf(file,"%-9g %-9g %-9g ", U[0],U[1],L[2]);
	(void) fprintf(file,"%-9g %-9g %-9g ", U[0],U[1],U[2]);
	(void) fprintf(file,"%-9g %-9g %-9g\n",U[0],L[1],U[2]);
	/* left face, y = L[1] */
	(void) fprintf(file,"%-9g %-9g %-9g ", L[0],L[1],L[2]);
	(void) fprintf(file,"%-9g %-9g %-9g ", U[0],L[1],L[2]);
	(void) fprintf(file,"%-9g %-9g %-9g ", U[0],L[1],U[2]);
	(void) fprintf(file,"%-9g %-9g %-9g\n",L[0],L[1],U[2]);
	/* right face, y = U[1] */
	(void) fprintf(file,"%-9g %-9g %-9g ", L[0],U[1],L[2]);
	(void) fprintf(file,"%-9g %-9g %-9g ", U[0],U[1],L[2]);
	(void) fprintf(file,"%-9g %-9g %-9g ", U[0],U[1],U[2]);
	(void) fprintf(file,"%-9g %-9g %-9g\n",L[0],U[1],U[2]);
}		/*end gview_cube*/

EXPORT void gview_plot_tri_list(
	const char *dname,
	TRI        **tris,
	int        num_tris)
{
	static const char *indent = "    ";
	int        i,j;
	char       fname[256];
	FILE       *file;
	FT_POINT      *p;

	if (create_directory(dname,YES) == FUNCTION_FAILED)
	{
	    screen("WARNING in gview_plot_tri_list(), "
	           "directory %s doesn't exist and can't be created\n",dname);
	    return;
	}
	(void) sprintf(fname,"%s/tri.list",dname);
	if ((file = fopen(fname,"w")) == NULL)
	{
	    screen("WARNING in gview_plot_tri_list(), "
	           "can't open %s\n",fname);
	    return;
	}
	(void) fprintf(file,"{ LIST\n");
	(void) fprintf(file,"%s{\n%s%sOFF\n%s%s%6d %6d %6d\n",
			indent,indent,indent,
			indent,indent,3*num_tris,num_tris,0);
	for (i = 0; i < num_tris; ++i)
	{
	    for (j = 0; j < 3; ++j)
	    {
		p = Point_of_tri(tris[i])[j];
		(void) fprintf(file, "%s%s%-9g %-9g %-9g\n",
			indent,indent,
			Coords(p)[0],Coords(p)[1],Coords(p)[2]);
	    }
	}
	for (i = 0; i < num_tris; ++i)
	{
	    (void) fprintf(file,"%s%s%-4d %-4d %-4d %-4d\n",indent,indent,
		3,3*i,3*i+1,3*i+2);
	}
	(void) fprintf(file,"%s}\n",indent);
	(void) fprintf(file,"}\n");
	(void) fclose(file);
}               /*end gview_plot_tri_list*/

EXPORT void gview_plot_tri_and_point_list(
	const char    *dname,
	TRI           **tris,
	const double   *color_tri,
	int           num_tris,
	double *const  *coords,
	SURFACE_COLOR lcolor,
	double         alpha,
	int           width,
	int           npts)
{
	static const char *indent = "    ";
	int               i,j;
	char              fname[256];
	FILE              *file;
	FT_POINT             *p;

	if (create_directory(dname,YES) == FUNCTION_FAILED)
	{
	    screen("WARNING in gview_plot_tri_and_point_list(), "
	           "directory %s doesn't exist and can't be created\n",dname);
	    return;
	}
	(void) sprintf(fname,"%s/tri.list",dname);
	if ((file = fopen(fname,"w")) == NULL)
	{
	    screen("WARNING in gview_plot_tri_and_point_list(), "
	           "can't open %s\n",fname);
	    return;
	}
	(void) fprintf(file,"{ LIST\n");
	(void) fprintf(file,"%s{\n%s%sOFF\n%s%s%6d %6d %6d\n",
			indent,indent,indent,
			indent,indent,3*num_tris,num_tris,0);
	for (i = 0; i < num_tris; ++i)
	{
	    for (j = 0; j < 3; ++j)
	    {
		p = Point_of_tri(tris[i])[j];
		(void) fprintf(file, "%s%s%-9g %-9g %-9g\n",
			indent,indent,
			Coords(p)[0],Coords(p)[1],Coords(p)[2]);
	    }
	}
	for (i = 0; i < num_tris; ++i)
	{
	    (void) fprintf(file,"%s%s%-4d %-4d %-4d %-4d",indent,indent,
		           3,3*i,3*i+1,3*i+2);
	    if (color_tri)
	        (void) fprintf(file," %g %g %g %g",color_tri[4*i],
		                                   color_tri[4*i+1],
		                                   color_tri[4*i+2],
		                                   color_tri[4*i+3]);
	    (void) fprintf(file,"\n");
	}
	(void) fprintf(file,"%s}\n",indent);

	print_polyline_description(file,indent,coords,npts,lcolor,alpha,width);
	(void) fprintf(file,"}\n");
	(void) fclose(file);
}               /*end gview_plot_tri_and_point_list*/

EXPORT void gview_plot_c_curve(
	const C_CURVE *c_curve,
	int	      k,
	const char    *dname)
{
	C_BOND            *cb;
	FILE	          *file;
	FT_POINT	          *p;
	TRI               *tri;
	char	          fname[256];
	const char        *color;
	double             *BBL = topological_grid(c_curve->interface).GL;
	double             *BBU = topological_grid(c_curve->interface).GU;
	int	          isurf,j, index, num_c_bonds;
	static const char *indent = "    ";
	static const char red[10] = " 200 0 0", green[10] = " 0 200 0";
	
	
	if (create_directory(dname,YES) == FUNCTION_FAILED)
	{
	    screen("WARNING in gview_plot_c_curve(), "
	           "directory %s doesn't exist and can't be created\n",dname);
	    return;
	}
	(void) sprintf(fname,"%s/curve%d.list",dname,k);
	if ((file = fopen(fname,"w")) == NULL)
	{
	    screen("WARNING in gview_plot_c_curve(), "
	           "can't open %s\n",fname);
	    return;
	}

	num_c_bonds=0;
	for (cb = c_curve->first; cb != NULL; cb = cb->next) 
	    ++num_c_bonds;

	(void) fprintf(file,"{ LIST\n");

	gview_bounding_box(file,BBL,BBU,1,indent);

	(void) fprintf(file,"%s{\n%s%sOFF\n%s%s%6d %6d %6d\n",
		       indent,indent,indent,
		       indent,indent,6*num_c_bonds,2*num_c_bonds,0);
	
	
	for (cb=c_curve->first; cb != NULL; cb=cb->next)
	{
	    for (isurf=0; isurf<2; ++isurf)
	    {
		tri = cb->s[isurf].t; 
	      
		for (j = 0; j < 3; ++j)
		{
		    p = Point_of_tri(tri)[j];
		    (void) fprintf(file, "%s%s%-9g %-9g %-9g\n",indent,indent,
				   Coords(p)[0],Coords(p)[1],Coords(p)[2]);
		}
	    }
	}
	   
	for (j=0; j < num_c_bonds; ++j)
	{
	    for (isurf=0; isurf<2; ++isurf)
	    {
		index = (6*j)+(3*isurf);
		color = (isurf)? red : green ;
	      
		(void) fprintf(file,"%s%s%-4d %-4d %-4d %-4d%s\n",indent,
			       indent,3,index,index+1,index+2,color);
	    }
	}
	
	(void) fprintf(file,"%s}\n",indent);
	(void) fprintf(file,"}\n");
	(void) fclose(file);
}		/*end gview_plot_c_curve*/

LOCAL	char   *get_list_file_name(
	char       *fname,
	const char *dname,
	const char *name,
	size_t     *fname_len)
{
	size_t len;
	if (dname == NULL)
	    dname = "";
	len = strlen(dname)+1+strlen(name)+6;
	if (*fname_len < len)
	{
	    *fname_len = len;
	    if (fname != NULL)
		free(fname);
	    uni_array(&fname,*fname_len,CHAR);
	}
	if (strlen(dname) != 0)
	    (void) sprintf(fname,"%s/%s.list",dname,name);
	else
	    (void) sprintf(fname,"%s.list",name);
	return fname;
}		/*end get_list_file_name*/


LOCAL	char *set_ppfname(
	char       *ppfname,
	const char *fname,
	size_t     *ppfname_len)
{
	size_t len;

        if (pp_numnodes() > 1 )
	{
	    const char   *nd;

	    nd = right_flush(pp_mynode(),4);
	    if (fname == NULL)
	        fname = "";
	    len = strlen(fname)+1+strlen(nd)+1;
	    if (*ppfname_len < len)
	    {
	        *ppfname_len = len;
	        if (ppfname != NULL)
		    free(ppfname);
	        uni_array(&ppfname,*ppfname_len,CHAR);
	    }
	    (void) sprintf(ppfname,"%s.%s",fname,nd);
	}
	else
	{
	    if (fname == NULL)
	        fname = "";
	    len = strlen(fname)+1;
	    if (*ppfname_len < len)
	    {
	        *ppfname_len = len;
	        if (ppfname != NULL)
		    free(ppfname);
	        uni_array(&ppfname,*ppfname_len,CHAR);
	    }
	    (void) strcpy(ppfname,fname);
	}
	return ppfname;
}		/*end set_ppfname*/

LOCAL   void gview_plot_color_surfaces(
        INTERFACE     *intfc,
        RECT_GRID     *gr,
        const double   *BBL,
        const double   *BBU,
        boolean          clip,
        const char    *dname,
        const char    *name,
        boolean          bdry)
{
        FILE              *file;
        FT_POINT             *p;
        SURFACE           **s;
        TRI               *tri;
        boolean              plot_surf;
        double             D, intensity = .5;
        double             L[MAXD],U[MAXD],tol[MAXD];
        double             *crds;
        int               num_surfs, num_tris, i, j, k, l,is;
        int               npt[3], ntri[3];
        static const char *indent = "    ";
        static double      *pos_pts = NULL;
        static double      *neg_pts = NULL;
        static int        *verts = NULL;
        static int        alloc_len_verts = 0, alloc_len_pts = 0;
        static char       *fname = NULL;
        static size_t     fname_len = 0;
        SURFACE_COLOR     pos_color[3];
        SURFACE_COLOR     neg_color[3];
        double             nor[3],nor_len;
        double             p1[3],p2[3];
        double             delta = 0.001;
        neg_color[0] = pRED;
        neg_color[1] = pBLUE;
        neg_color[2] = pWHITE;
        pos_color[0] = pWHITE;
        pos_color[1] = pRED;
        pos_color[2] = pBLUE;

	fname = get_list_file_name(fname,dname,name,&fname_len);

	for (num_tris = 0, s = intfc->surfaces; s && *s; ++s)
        {
             num_tris += (*s)->num_tri;
        }

        if (alloc_len_pts < 3*intfc->num_points)
        {
            if (pos_pts != NULL)
                free(pos_pts);
            if (neg_pts != NULL)
                free(neg_pts);
            alloc_len_pts = 3*intfc->num_points;
            uni_array(&pos_pts,alloc_len_pts,FLOAT);
            uni_array(&neg_pts,alloc_len_pts,FLOAT);
        }
        if (alloc_len_verts < 4*num_tris)
        {
            if (verts != NULL)
                free(verts);
                alloc_len_verts = 4*num_tris;
                uni_array(&verts,alloc_len_verts,INT);
        }
        for (i = 0; i < 3; i++)
        {
            L[i] = gr->L[i] - 0.5*gr->h[i];
            U[i] = gr->U[i] + 0.5*gr->h[i];
            tol[i] = 0.00001*gr->h[i];
        }
        if ((file = fopen(fname,"w")) == NULL)
        {
            (void) printf("WARNING in gview_plot_surfaces(), "
                          "can't open %s\n",fname);
             return;
        }
        (void) fprintf(file,"{ LIST\n");
        gview_bounding_box(file,BBL,BBU,1,indent);

	for (is = 0, num_surfs = 0, s = intfc->surfaces; s && *s; ++s)
        {
            for (tri=first_tri(*s); !at_end_of_tri_list(tri,*s); tri=tri->next)
            {
                for (k = 0; k < 3; ++k)
                     Index_of_point(Point_of_tri(tri)[k]) = -1;
            }
            npt[is] = 0;
            ntri[is] = 0;
            if (bdry == YES  &&  !Boundary(*s))
                continue;
            if (bdry == NO  &&  Boundary(*s))
                continue;
            if (clip == YES)
            {
                plot_surf = NO;
                for (tri = first_tri(*s); !at_end_of_tri_list(tri,*s);
                     tri = tri->next)
                {
                     for (k = 0; k < 3; ++k)
                     {
                         crds = Coords(Point_of_tri(tri)[k]);
                         for (l = 0; l < 3; ++l)
                             if ((crds[l] < L[l] - tol[l]) ||
                                 (U[l] + tol[l] < crds[l]))
                                 break;
                         if (l < 3)
                             break;
                     }
                     if (k == 3)
                     {
                         plot_surf = YES;
                         for (k = 0; k < 3; ++k)
                         {
                             /* shadow */
                             for (l = 0; l < 3; l++)
                             {
                                 p1[l] = Coords(Point_of_tri(tri)[1])[l] -
                                     Coords(Point_of_tri(tri)[0])[l];
                                 p2[l] = Coords(Point_of_tri(tri)[2])[l] -
                                     Coords(Point_of_tri(tri)[0])[l];
                             }
                             Cross3d(p1,p2,nor);
                             nor_len = sqrt(nor[0]*nor[0] + nor[1]*nor[1] + nor[2]*nor[2]);
                             for (l = 0; l < 3; l++)
                                 nor[l] /= nor_len;
                             /***********/
                             p = Point_of_tri(tri)[k];
                            if (Index_of_point(p) == -1)
                            {
                                crds = Coords(p);
                                for (l = 0; l < 3; ++l)
                                {
                                    pos_pts[3*npt[is]+l] = crds[l]+delta*nor[l];                                           neg_pts[3*npt[is]+l] = crds[l]-delta*nor[l];  
				}
                                Index_of_point(p) = npt[is]++;
                            }
                            verts[4*ntri[is]+k] = Index_of_point(p);
                        }
                        verts[4*ntri[is]+3] = num_surfs;
                        ++ntri[is];
                    }   
                }
                if (plot_surf == YES)
                ++num_surfs;

                /* plot surface */
                D = 1.0/(num_surfs - 1.0);
                /* positive surface */
                (void) fprintf(file,"%s{\n%s%sOFF\n%s%s%6d %6d %6d\n",indent,
                               indent,indent,indent,indent,npt[is],ntri[is],0);
                for (i = 0; i < npt[is]; ++i)
                {
                    (void) fprintf(file,"%s%s%-9g %-9g %-9g\n",indent,indent,
                           pos_pts[3*i],pos_pts[3*i+1],pos_pts[3*i+2]);
                }
                for (j = 0; j < ntri[is]; ++j)
                {
                    (void) fprintf(file,"%s%s%-4d %-4d %-4d %-4d ",indent,indent,
                           3,verts[4*j],verts[4*j+1],verts[4*j+2]);
                    write_interpolated_color(file,pos_color[is],pos_color[is],
                           verts[4*j+3]/D,intensity);
                }
                (void) fprintf(file,"%s}\n",indent);
	        /* negative surface */
	        (void) fprintf(file,"%s{\n%s%sOFF\n%s%s%6d %6d %6d\n",indent,
	                       indent,indent,indent,indent,npt[is],ntri[is],0);
                for (i = 0; i < npt[is]; ++i)
                {
                    (void) fprintf(file,"%s%s%-9g %-9g %-9g\n",indent,indent,
                           neg_pts[3*i],neg_pts[3*i+1],neg_pts[3*i+2]);
                }
                for (j = 0; j < ntri[is]; ++j)
                {
                    (void) fprintf(file,"%s%s%-4d %-4d %-4d %-4d ",indent,indent,
                           3,verts[4*j],verts[4*j+1],verts[4*j+2]);
                    write_interpolated_color(file,neg_color[is],neg_color[is],
                           verts[4*j+3]/D,intensity);
                }
                (void) fprintf(file,"%s}\n",indent);
            }
            else
            {
                 for (tri = first_tri(*s); !at_end_of_tri_list(tri,*s);
                      tri = tri->next)
                 {
                     for (k = 0; k < 3; ++k)
                     {
                         /* shadow */
                         for (l = 0; l < 3; l++)
                         {
                             p1[l] = Coords(Point_of_tri(tri)[1])[l] -
                                     Coords(Point_of_tri(tri)[0])[l];
                             p2[l] = Coords(Point_of_tri(tri)[2])[l] -
                                     Coords(Point_of_tri(tri)[0])[l];
                         }
                         Cross3d(p1,p2,nor);
                         nor_len = sqrt(nor[0]*nor[0] + nor[1]*nor[1] + nor[2]*nor[2]);
                         for (l = 0; l < 3; l++)
                         nor[l] /= nor_len;
                         /***********/
                         p = Point_of_tri(tri)[k];
                         if (Index_of_point(p) == -1)
                         {
                             crds = Coords(p);
                             for (l = 0; l < 3; ++l)
                             {
                                 pos_pts[3*npt[is]+l] = crds[l]+delta*nor[l];
                                 neg_pts[3*npt[is]+l] = crds[l]-delta*nor[l];
                             }
                             Index_of_point(p) = npt[is]++;
                         }
                         verts[4*ntri[is]+k] = Index_of_point(p);
                     }
                     verts[4*ntri[is]+3] = num_surfs;
                     ++ntri[is];
                 }
                 ++num_surfs;
                 /* plot surface */
                 D = 1.0/(num_surfs - 1.0);
                 /* positive surface */
                 (void) fprintf(file,"%s{\n%s%sOFF\n%s%s%6d %6d %6d\n",indent,
                                indent,indent,indent,indent,npt[is],ntri[is],0);
                 for (i = 0; i < npt[is]; ++i)
                 {
                     (void) fprintf(file,"%s%s%-9g %-9g %-9g\n",indent,indent,
                            pos_pts[3*i],pos_pts[3*i+1],pos_pts[3*i+2]);
                 }
                 for (j = 0; j < ntri[is]; ++j)
                 {
                     (void) fprintf(file,"%s%s%-4d %-4d %-4d %-4d ",indent,indent,
                            3,verts[4*j],verts[4*j+1],verts[4*j+2]);
                     write_interpolated_color(file,pos_color[is],pos_color[is],
                            verts[4*j+3]/D,intensity);
                 }
                 (void) fprintf(file,"%s}\n",indent);
                 /* negative surface */
                 (void) fprintf(file,"%s{\n%s%sOFF\n%s%s%6d %6d %6d\n",indent,
                                indent,indent,indent,indent,npt[is],ntri[is],0);
                 for (i = 0; i < npt[is]; ++i)
                 {
                     (void) fprintf(file,"%s%s%-9g %-9g %-9g\n",indent,indent,
                            neg_pts[3*i],neg_pts[3*i+1],neg_pts[3*i+2]);
                 }
                 for (j = 0; j < ntri[is]; ++j)
                 {
                     (void) fprintf(file,"%s%s%-4d %-4d %-4d %-4d ",indent,indent,
                            3,verts[4*j],verts[4*j+1],verts[4*j+2]);
                     write_interpolated_color(file,neg_color[is],neg_color[is],
                            verts[4*j+3]/D,intensity);
                 }
                 (void) fprintf(file,"%s}\n",indent);
            }
            ++is;
        }
        (void) fprintf(file,"%s}\n",indent);
        (void) fprintf(file,"}\n");
        (void) fclose(file);
        if (num_surfs == 0)
            return;
}       /* end gview_plot_color_surfaces */

/* END GEOMVIEW FUNCS */

/* START NEW VTK FUNCS */
LOCAL	void   vtk_print_box(
        const char    *dname,
	const double   *BBL,
        const double   *BBU,
	boolean	      print_in_binary
	)
{
	FILE *file;
	static char       *fname = NULL;
	static size_t     fname_len = 0;
	char vfmt[256];
	char str[100];
	int ival[1];
	float val[1];

	(void) sprintf(vfmt,"%s\n","%-9g %-9g %-9g");
	fname = get_vtk_file_name(fname,dname,"box",&fname_len);
        
	if(print_in_binary)
	{
	    if ((file = fopen(fname,"wb")) == NULL)
	    {
	        (void) printf("WARNING in vtk_print_box(), "
		              "can't open %s\n",fname);
		return;
	    }

	    sprintf(str, "# vtk DataFile Version 3.0\n");
	    fwrite(str, sizeof(char), 27, file);
	    sprintf(str, "FronTier Interface\n");
	    fwrite(str, sizeof(char), 19, file);
	    sprintf(str, "BINARY\n");
	    fwrite(str, sizeof(char), 7, file);
	    sprintf(str, "DATASET POLYDATA\n");
	    fwrite(str, sizeof(char), 17, file);
	    sprintf(str, "POINTS 8 float\n");
	    fwrite(str, sizeof(char), 15, file);  
	    
	    if(hardware_is_little_endian())
	    {
	        val[0] = endian_float_swap(BBL[0]);
	        fwrite(val, sizeof(float), 1 , file);
	        val[0] = endian_float_swap(BBL[1]);
	    	fwrite(val, sizeof(float), 1 , file);
	 	val[0] = endian_float_swap(BBL[2]);
		fwrite(val, sizeof(float), 1 , file);
		
		val[0] = endian_float_swap(BBU[0]);
                fwrite(val, sizeof(float), 1 , file);
                val[0] = endian_float_swap(BBL[1]);
                fwrite(val, sizeof(float), 1 , file);
                val[0] = endian_float_swap(BBL[2]);
                fwrite(val, sizeof(float), 1 , file);
	
		val[0] = endian_float_swap(BBU[0]);
                fwrite(val, sizeof(float), 1 , file);
                val[0] = endian_float_swap(BBU[1]);
                fwrite(val, sizeof(float), 1 , file);
                val[0] = endian_float_swap(BBL[2]);
                fwrite(val, sizeof(float), 1 , file);

		val[0] = endian_float_swap(BBL[0]);
                fwrite(val, sizeof(float), 1 , file);
                val[0] = endian_float_swap(BBU[1]);
                fwrite(val, sizeof(float), 1 , file);
                val[0] = endian_float_swap(BBL[2]);
                fwrite(val, sizeof(float), 1 , file);
		
		val[0] = endian_float_swap(BBL[0]);
                fwrite(val, sizeof(float), 1 , file);
                val[0] = endian_float_swap(BBL[1]);
                fwrite(val, sizeof(float), 1 , file);
                val[0] = endian_float_swap(BBU[2]);
                fwrite(val, sizeof(float), 1 , file);
	
		val[0] = endian_float_swap(BBU[0]);
                fwrite(val, sizeof(float), 1 , file);
                val[0] = endian_float_swap(BBL[1]);
                fwrite(val, sizeof(float), 1 , file);
                val[0] = endian_float_swap(BBU[2]);
                fwrite(val, sizeof(float), 1 , file);

		val[0] = endian_float_swap(BBU[0]);
                fwrite(val, sizeof(float), 1 , file);
                val[0] = endian_float_swap(BBU[1]);
                fwrite(val, sizeof(float), 1 , file);
                val[0] = endian_float_swap(BBU[2]);
                fwrite(val, sizeof(float), 1 , file);

		val[0] = endian_float_swap(BBL[0]);
                fwrite(val, sizeof(float), 1 , file);
                val[0] = endian_float_swap(BBU[1]);
                fwrite(val, sizeof(float), 1 , file);
                val[0] = endian_float_swap(BBU[2]);
                fwrite(val, sizeof(float), 1 , file);
	    }
	    else
	    {
		val[0] = BBL[0];
                fwrite(val, sizeof(float), 1 , file);
                val[0] = BBL[1];
                fwrite(val, sizeof(float), 1 , file);
                val[0] = BBL[2];
                fwrite(val, sizeof(float), 1 , file);

                val[0] = BBU[0];
                fwrite(val, sizeof(float), 1 , file);
                val[0] = BBL[1];
                fwrite(val, sizeof(float), 1 , file);
                val[0] = BBL[2];
                fwrite(val, sizeof(float), 1 , file);

                val[0] = BBU[0];
                fwrite(val, sizeof(float), 1 , file);
                val[0] = BBU[1];
                fwrite(val, sizeof(float), 1 , file);
                val[0] = BBL[2];
                fwrite(val, sizeof(float), 1 , file);

                val[0] = BBL[0];
                fwrite(val, sizeof(float), 1 , file);
                val[0] = BBU[1];
                fwrite(val, sizeof(float), 1 , file);
                val[0] = BBL[2];
                fwrite(val, sizeof(float), 1 , file);

                val[0] = BBL[0];
                fwrite(val, sizeof(float), 1 , file);
                val[0] = BBL[1];
                fwrite(val, sizeof(float), 1 , file);
                val[0] = BBU[2];
                fwrite(val, sizeof(float), 1 , file);

                val[0] = BBU[0];
                fwrite(val, sizeof(float), 1 , file);
                val[0] = BBL[1];
                fwrite(val, sizeof(float), 1 , file);
                val[0] = BBU[2];
                fwrite(val, sizeof(float), 1 , file);

                val[0] = BBU[0];
                fwrite(val, sizeof(float), 1 , file);
                val[0] = BBU[1];
                fwrite(val, sizeof(float), 1 , file);
                val[0] = BBU[2];
                fwrite(val, sizeof(float), 1 , file);

                val[0] = BBL[0];
                fwrite(val, sizeof(float), 1 , file);
                val[0] = BBU[1];
                fwrite(val, sizeof(float), 1 , file);
                val[0] = BBU[2];
                fwrite(val, sizeof(float), 1 , file);
	    }
	    sprintf(str, "\nLINES 12 36\n");
	    fwrite(str, sizeof(char), 13, file);
	    if(hardware_is_little_endian())
	        ival[0] = endian_int_swap(2);
            else
		ival[0] = 2;
            fwrite(ival, sizeof(int), 1, file);
	    if(hardware_is_little_endian())
                ival[0] = endian_int_swap(0);
            else
                ival[0] = 0;
            fwrite(ival, sizeof(int), 1, file);
	    if(hardware_is_little_endian())
                ival[0] = endian_int_swap(1);
            else
                ival[0] = 1;
            fwrite(ival, sizeof(int), 1, file);
	    
	    if(hardware_is_little_endian())
                ival[0] = endian_int_swap(2);
            else
                ival[0] = 2;
            fwrite(ival, sizeof(int), 1, file);
            if(hardware_is_little_endian())
                ival[0] = endian_int_swap(1);
            else
                ival[0] = 1;
            fwrite(ival, sizeof(int), 1, file);
            if(hardware_is_little_endian())
                ival[0] = endian_int_swap(2);
            else
                ival[0] = 2;
            fwrite(ival, sizeof(int), 1, file);

	    if(hardware_is_little_endian())
                ival[0] = endian_int_swap(2);
            else
                ival[0] = 2;
            fwrite(ival, sizeof(int), 1, file);
            if(hardware_is_little_endian())
                ival[0] = endian_int_swap(3);
            else
                ival[0] = 3;
            fwrite(ival, sizeof(int), 1, file);
            if(hardware_is_little_endian())
                ival[0] = endian_int_swap(3);
            else
                ival[0] = 3;
            fwrite(ival, sizeof(int), 1, file);

	    if(hardware_is_little_endian())
                ival[0] = endian_int_swap(2);
            else
                ival[0] = 2;
            fwrite(ival, sizeof(int), 1, file);
            if(hardware_is_little_endian())
                ival[0] = endian_int_swap(3);
            else
                ival[0] = 3;
            fwrite(ival, sizeof(int), 1, file);
            if(hardware_is_little_endian())
                ival[0] = endian_int_swap(0);
            else
                ival[0] = 0;
            fwrite(ival, sizeof(int), 1, file);
	 
	    if(hardware_is_little_endian())
                ival[0] = endian_int_swap(2);
            else
                ival[0] = 2;
            fwrite(ival, sizeof(int), 1, file);
            if(hardware_is_little_endian())
                ival[0] = endian_int_swap(4);
            else
                ival[0] = 4;
            fwrite(ival, sizeof(int), 1, file);
            if(hardware_is_little_endian())
                ival[0] = endian_int_swap(5);
            else
                ival[0] = 5;
            fwrite(ival, sizeof(int), 1, file);

   	    if(hardware_is_little_endian())
                ival[0] = endian_int_swap(2);
            else
                ival[0] = 2;
            fwrite(ival, sizeof(int), 1, file);
            if(hardware_is_little_endian())
                ival[0] = endian_int_swap(5);
            else
                ival[0] = 5;
            fwrite(ival, sizeof(int), 1, file);
            if(hardware_is_little_endian())
                ival[0] = endian_int_swap(6);
            else
                ival[0] = 6;
            fwrite(ival, sizeof(int), 1, file);

	    if(hardware_is_little_endian())
                ival[0] = endian_int_swap(2);
            else
                ival[0] = 2;
            fwrite(ival, sizeof(int), 1, file);
            if(hardware_is_little_endian())
                ival[0] = endian_int_swap(6);
            else
                ival[0] = 6;
            fwrite(ival, sizeof(int), 1, file);
            if(hardware_is_little_endian())
                ival[0] = endian_int_swap(7);
            else
                ival[0] = 7;
            fwrite(ival, sizeof(int), 1, file);

	    if(hardware_is_little_endian())
                ival[0] = endian_int_swap(2);
            else
                ival[0] = 2;
            fwrite(ival, sizeof(int), 1, file);
            if(hardware_is_little_endian())
                ival[0] = endian_int_swap(7);
            else
                ival[0] = 7;
            fwrite(ival, sizeof(int), 1, file);
            if(hardware_is_little_endian())
                ival[0] = endian_int_swap(4);
            else
                ival[0] = 4;
            fwrite(ival, sizeof(int), 1, file);

	    if(hardware_is_little_endian())
                ival[0] = endian_int_swap(2);
            else
                ival[0] = 2;
            fwrite(ival, sizeof(int), 1, file);
            if(hardware_is_little_endian())
                ival[0] = endian_int_swap(0);
            else
                ival[0] = 0;
            fwrite(ival, sizeof(int), 1, file);
            if(hardware_is_little_endian())
                ival[0] = endian_int_swap(4);
            else
                ival[0] = 4;
            fwrite(ival, sizeof(int), 1, file);

	    if(hardware_is_little_endian())
                ival[0] = endian_int_swap(2);
            else
                ival[0] = 2;
            fwrite(ival, sizeof(int), 1, file);
            if(hardware_is_little_endian())
                ival[0] = endian_int_swap(1);
            else
                ival[0] = 1;
            fwrite(ival, sizeof(int), 1, file);
            if(hardware_is_little_endian())
                ival[0] = endian_int_swap(5);
            else
                ival[0] = 5;
            fwrite(ival, sizeof(int), 1, file);

	    if(hardware_is_little_endian())
                ival[0] = endian_int_swap(2);
            else
                ival[0] = 2;
            fwrite(ival, sizeof(int), 1, file);
            if(hardware_is_little_endian())
                ival[0] = endian_int_swap(2);
            else
                ival[0] = 2;
            fwrite(ival, sizeof(int), 1, file);
            if(hardware_is_little_endian())
                ival[0] = endian_int_swap(6);
            else
                ival[0] = 6;
            fwrite(ival, sizeof(int), 1, file);

	    if(hardware_is_little_endian())
                ival[0] = endian_int_swap(2);
            else
                ival[0] = 2;
            fwrite(ival, sizeof(int), 1, file);
            if(hardware_is_little_endian())
                ival[0] = endian_int_swap(3);
            else
                ival[0] = 3;
            fwrite(ival, sizeof(int), 1, file);
            if(hardware_is_little_endian())
                ival[0] = endian_int_swap(7);
            else
                ival[0] = 7;
            fwrite(ival, sizeof(int), 1, file);
	}
	else
	{	
   	    if ((file = fopen(fname,"w")) == NULL)
            {
                (void) printf("WARNING in vtk_print_box(), "
                               "can't open %s\n",fname);
                return;
            }
	
	    fprintf(file,"# vtk DataFile Version 3.0\n" 
	    "FronTier Interface\n"
	    "ASCII\n"
	    "DATASET POLYDATA\n"
	    "POINTS 8 double\n");
	    (void) fprintf(file,vfmt,BBL[0],BBL[1],BBL[2]);
            (void) fprintf(file,vfmt,BBU[0],BBL[1],BBL[2]);
            (void) fprintf(file,vfmt,BBU[0],BBU[1],BBL[2]);
            (void) fprintf(file,vfmt,BBL[0],BBU[1],BBL[2]);
            (void) fprintf(file,vfmt,BBL[0],BBL[1],BBU[2]);
            (void) fprintf(file,vfmt,BBU[0],BBL[1],BBU[2]);
            (void) fprintf(file,vfmt,BBU[0],BBU[1],BBU[2]);
            (void) fprintf(file,vfmt,BBL[0],BBU[1],BBU[2]);
	    fprintf(file, "LINES 12 36\n"
	                  "2 0 1\n"
	    		  "2 1 2\n"
	    		  "2 2 3\n"
	    		  "2 3 0\n"
	    		  "2 4 5\n"
	    		  "2 5 6\n"
	    		  "2 6 7\n"
	    		  "2 7 4\n"
	    		  "2 0 4\n"
	    		  "2 1 5\n"
	    		  "2 2 6\n"
	    		  "2 3 7\n");
	}			  
	fclose(file);
}

EXPORT	char   *get_vtk_file_name(
	char       *fname,
	const char *dname,
	const char *name,
	size_t     *fname_len)
{
	size_t len;
	if (dname == NULL)
	    dname = "";
	len = strlen(dname)+1+strlen(name)+6;
	if (*fname_len < len)
	{
	    *fname_len = len;
	    if (fname != NULL)
		free(fname);
	    uni_array(&fname,*fname_len,CHAR);
	}
	if (strlen(dname) != 0)
	    (void) sprintf(fname,"%s/%s.vtk",dname,name);
	else
	    (void) sprintf(fname,"%s.vtk",name);
	return fname;
}		/*end get_vtk_file_name*/

EXPORT void vtk_interface_plot(
	const char *dname,
	INTERFACE  *intfc,
	boolean print_in_binary,
	double time,
	int step)
{
	double *BBL = topological_grid(intfc).GL;
	double *BBU = topological_grid(intfc).GU;

	if (intfc->dim == 2)
	    vtk_plot_curves(intfc,BBL,BBU,dname,"2d-intfc",pRED,10,
				print_in_binary,time,step);

	if (intfc->dim == 3)
	{
	    vtk_plot_curves(intfc,BBL,BBU,dname,"3d-curves",pRED,10,
				print_in_binary,time,step);
	    vtk_plot_surfaces(intfc,BBL,BBU,NO,dname,"3d-intfc",NO,pRED,pRED,
				print_in_binary,time,step);
	    vtk_print_box(dname,BBL,BBU,print_in_binary);
	}

}		/*end vtk_interface_plot*/


LOCAL	void	vtk_plot_surfaces(
	INTERFACE     *intfc,
	const double   *BBL,
	const double   *BBU,
	boolean          clip,
	const char    *dname,
	const char    *name,
	boolean	      bdry,
	SURFACE_COLOR color1,
	SURFACE_COLOR color2,
	boolean 	      print_in_binary,
	double 	      time,
	int 	      step)
{
	FILE	          *file;
	FT_POINT             *p;
	SURFACE	          **s;
	TRI	          *tri;
	boolean              plot_surf;
	double 	          D, intensity = .5;
	double             L[MAXD],U[MAXD],tol[MAXD];
	double	          *crds;
	int	          num_surfs, num_tris, i, j, k, l;
	int               npts, ntris;
	int		  length, length2;
	static const char *indent = "    ";
	static double      *pts = NULL;
	static int        *verts = NULL;
	static int        alloc_len_verts = 0, alloc_len_pts = 0;
	static char       *fname = NULL;
	char 		  str[100];
	static size_t     fname_len = 0;
	RECT_GRID *gr = &topological_grid(intfc);
	float value1[1];
	float tmp1;
	int value2[1];
	int tmp2;
	
	fname = get_vtk_file_name(fname,dname,name,&fname_len);

	for (num_tris = 0, s = intfc->surfaces; s && *s; ++s)
	{
	    num_tris += (*s)->num_tri;
	    for (tri=first_tri(*s); !at_end_of_tri_list(tri,*s); tri=tri->next)
	    {
	        for (k = 0; k < 3; ++k)
		    Index_of_point(Point_of_tri(tri)[k]) = -1;
	    }
	}
	
	if (alloc_len_pts < 3*intfc->num_points)
	{
	    if (pts != NULL)
		free(pts);
	    alloc_len_pts = 3*intfc->num_points;
	    uni_array(&pts,alloc_len_pts,FLOAT);
	}
	if (alloc_len_verts < 4*num_tris)
	{
	    if (verts != NULL)
		free(verts);
	    alloc_len_verts = 4*num_tris;
	    uni_array(&verts,alloc_len_verts,INT);
	}
	for (i = 0; i < 3; i++)
	{
	    L[i] = gr->L[i] - 0.5*gr->h[i]; 
	    U[i] = gr->U[i] + 0.5*gr->h[i];
	    tol[i] = 0.00001*gr->h[i];
	}

        for (npts=0, ntris=0, num_surfs=0, s = intfc->surfaces; s && *s; ++s)
	{
	    if (bdry == YES  &&  !Boundary(*s))
		continue; 
	    if (bdry == NO  &&  Boundary(*s))
		continue;
	    if (clip == YES)
	    {
		plot_surf = NO;
	        for (tri = first_tri(*s); !at_end_of_tri_list(tri,*s); 
		     tri = tri->next)
	        {
	            for (k = 0; k < 3; ++k)
		    {
			crds = Coords(Point_of_tri(tri)[k]);
	                for (l = 0; l < 3; ++l)
			    if ((crds[l] < L[l] - tol[l]) || 
			        (U[l] + tol[l] < crds[l]))
				break;
			if (l < 3)
			    break;
		    }
		    if (k == 3)
		    {
			plot_surf = YES;
			for (k = 0; k < 3; ++k)
			{
		            p = Point_of_tri(tri)[k];
			    if (Index_of_point(p) == -1)
			    {
			        crds = Coords(p);
	                        for (l = 0; l < 3; ++l)
				    pts[3*npts+l] = crds[l];
				Index_of_point(p) = npts++;
			    }
			    verts[4*ntris+k] = Index_of_point(p);
			}
			verts[4*ntris+3] = num_surfs;
			++ntris;
		    }
		}
		if (plot_surf == YES)
		    ++num_surfs;
	    }
	    else
	    {
	        for (tri = first_tri(*s); !at_end_of_tri_list(tri,*s); 
		     tri = tri->next)
	        {
	            for (k = 0; k < 3; ++k)
		    {
		        p = Point_of_tri(tri)[k];
			if (Index_of_point(p) == -1)
			{
			    crds = Coords(p);
	                    for (l = 0; l < 3; ++l)
				pts[3*npts+l] = crds[l];
			    Index_of_point(p) = npts++;
			}
			verts[4*ntris+k] = Index_of_point(p);
		    }
		    verts[4*ntris+3] = num_surfs;
		    ++ntris;
		}
		++num_surfs;
	    }
	}
         
	if ((file = fopen(fname,"w")) == NULL)
	{
	    (void) printf("WARNING in vtk_plot_surfaces(), "
	                  "can't open %s\n",fname);
	    return;
	}
	
	if(print_in_binary)
	{
	    if((file = fopen(fname, "wb")) == NULL)
	    {
 		(void) printf("WARNING in vtk_plot_surfaces(), "
	  	     	      "cant' open %s\n",fname);
	 	return;
	    }
	
	    sprintf(str,"# vtk DataFile Version 3.0\n");
	    fwrite(str, sizeof(char), 27, file);
	    sprintf(str,"FronTier Interface\n");
	    fwrite(str, sizeof(char), 19, file);
	    sprintf(str,"BINARY\n");
	    fwrite(str, sizeof(char), 7, file);
	    sprintf(str,"DATASET POLYDATA\n"); 
	    fwrite(str, sizeof(char), 17, file);

	    sprintf(str, "FIELD FieldData 2\n");
            fwrite(str, sizeof(char), 18, file);
	    sprintf(str, "TIME 1 1 float\n");
            fwrite(str, sizeof(char), 15, file);
	    tmp1 = time;
            if(hardware_is_little_endian())
	        value1[0] = endian_float_swap(tmp1);
            else
		value1[0] = tmp1;
	    fwrite(value1, sizeof(float), 1, file);

	    sprintf(str, "\n");
            fwrite(str, sizeof(char), 1, file);
	    sprintf(str, "CYCLE 1 1 int\n");
            fwrite(str, sizeof(char), 14, file);
	    #if defined(int)
            #undef int
            #define not_int
            #endif
	    tmp2 = step;
            if(hardware_is_little_endian())
	        value2[0] = endian_int_swap(tmp2);
            else
		value2[0] = tmp2;
	    fwrite(value2, sizeof(int), 1, file);
	    #if defined(not_int)
	    #define int double
     	    #undef not_int
	    #endif
	    sprintf(str, "\n");
            fwrite(str, sizeof(char), 1, file);

            sprintf(str,"POINTS %d float\n", npts);
	    length = count_digits(npts);
	    fwrite(str, sizeof(char), 14 + length, file);
	}
	else
	{
	    if ((file = fopen(fname,"w")) == NULL)
            {
                (void) printf("WARNING in vtk_plot_surfaces(), "
                              "can't open %s\n",fname);
                 return;
            }
 
	    (void) fprintf(file,"# vtk DataFile Version 3.0 \n"
		                "FronTier Interface \n"
		       	        "ASCII \n"
			        "DATASET POLYDATA \n");
	    (void) fprintf(file, "FIELD FieldData 2\n");
	    (void) fprintf(file, "TIME 1 1 float\n");
	    (void) fprintf(file, "%5.5e\n", time);
	    (void) fprintf(file, "CYCLE 1 1 int\n");
	    (void) fprintf(file, "%d\n", step);
	    (void) fprintf(file, "POINTS %d double\n",npts);
	}
	     
	for (i = 0; i < npts; ++i)
	{
	    if(print_in_binary)
	    {
		float vals[1];
		for(j = 0; j < 3; ++j)
		{
		    if(hardware_is_little_endian())
		        vals[0] = endian_float_swap(pts[3*i+j]);
		    else
			vals[0] = pts[3*i+j];
                    fwrite(vals, sizeof(float), 1, file);
		}    
	     }
	     else
	         (void) fprintf(file,"%-9g %-9g %-9g\n",
		       	        pts[3*i],pts[3*i+1],pts[3*i+2]);
	     
	}

	D = (num_surfs == 1) ? 1.0 : 1/(num_surfs - 1.0);
	if(print_in_binary)
	{
	    sprintf(str, "\nPOLYGONS %d %d\n", ntris, ntris*4);
	    length = count_digits(ntris);
	    length2 = count_digits(ntris*4);
	    fwrite(str, sizeof(char), 12 + length + length2, file);
	    for(j = 0; j < ntris; ++j)
	    {
		for(k = 0; k < 3; ++k)
		{
                    int vals[1];
		    if(k == 0)
		    {
		        if(hardware_is_little_endian())
		            vals[0] = endian_int_swap(3);
			else
			    vals[0] = 3;
			fwrite(vals, sizeof(int), 1, file);
		    }
		    if(hardware_is_little_endian())
                        vals[0] = endian_int_swap(verts[4*j+k]);
                    else
                        vals[0] = verts[4*j+k];
                    fwrite(vals, sizeof(int), 1, file);
		}
	    }
	}
	else
	{
	    fprintf(file,"POLYGONS %d %d \n",ntris,ntris*4); 
	    for (j = 0; j < ntris; ++j)
	    {
	        (void) fprintf(file,"%d %d %d %d \n",
			       3,verts[4*j],verts[4*j+1],verts[4*j+2]);
	    }
	} 
	(void) fclose(file);
	    
}	/*end vtk_plot_surfaces*/


LOCAL   void    vtk_plot_curves(
        INTERFACE     *intfc,
	const double   *BBL,
	const double   *BBU,
        const char    *dname,
        const char    *name,
	SURFACE_COLOR color,
	int	      width,
	boolean  	      print_in_binary,
	double 	      time,
	int 	      step)
{
	switch (intfc->dim)
	{
	case 2:
	    vtk_plot_curves2d(intfc,BBL,BBU,dname,name,color,width,
				print_in_binary,time,step);
	    return;
	case 3:
	    vtk_plot_curves3d(intfc,BBL,BBU,dname,name,color,width,
				print_in_binary,time,step);
	    return;
	}
}	/* end vtk_plot_curves */

LOCAL   void    vtk_plot_curves2d(
        INTERFACE     *intfc,
	const double   *BBL,
	const double   *BBU,
        const char    *dname,
        const char    *name,
	SURFACE_COLOR color,
	int	      width,
	boolean  	      print_in_binary,
	double 	      time,
	int 	      step)
{
        FILE              *file;
        FT_POINT             *ps, *pe;
        CURVE             **c;
        BOND              *b;
        static const char *indent = "    ";
        int               num_bonds,i,first,second;
	static char       *fname = NULL;
	static size_t     fname_len = 0;
	int		  tot_num_pnts,tot_num_bonds; 
	char str[100];
	int ivals[1];
	int length, length2;

	fname = get_vtk_file_name(fname,dname,name,&fname_len);

	tot_num_pnts = 0;
	for (c = intfc->curves; c && *c; ++c)
	{
            if (is_subdomain_boundary(Hyper_surf(*c)))continue;
            tot_num_pnts = tot_num_pnts+(*c)->num_points;
	}
	if(print_in_binary)
	{
	    float value1[1],tmp1;
	    int value2[1],tmp2;
	    if ((file = fopen(fname,"wb")) == NULL)
            {
                screen("WARNING in vtk_plot_curves(), "
                       "can't open %s\n",fname);
                return;
	    }
	    sprintf(str,"# vtk DataFile Version 3.0\n");
	    fwrite(str, sizeof(char), 27, file);
	    sprintf(str,"FronTier Interface\n");
	    fwrite(str, sizeof(char), 19, file);
	    sprintf(str,"BINARY\n");
	    fwrite(str, sizeof(char), 7, file);
	    sprintf(str, "DATASET POLYDATA\n");
    	    fwrite(str, sizeof(char), 17, file);

	    sprintf(str, "FIELD FieldData 2\n");
            fwrite(str, sizeof(char), 18, file);
	    sprintf(str, "TIME 1 1 float\n");
            fwrite(str, sizeof(char), 15, file);

	    tmp1 = time;
            if(hardware_is_little_endian())
	        value1[0] = endian_float_swap(tmp1);
            else
		value1[0] = tmp1;
	    fwrite(value1, sizeof(float), 1, file);

	    sprintf(str, "\n");
            fwrite(str, sizeof(char), 1, file);
	    sprintf(str, "CYCLE 1 1 int\n");
            fwrite(str, sizeof(char), 14, file);
	    #if defined(int)
            #undef int
            #define not_int
            #endif
	    tmp2 = step;
            if(hardware_is_little_endian())
	        value2[0] = endian_int_swap(tmp2);
            else
		value2[0] = tmp2;
	    fwrite(value2, sizeof(int), 1, file);
	    #if defined(not_int)
	    #define int double
     	    #undef not_int
	    #endif
	    sprintf(str, "\n");
            fwrite(str, sizeof(char), 1, file);

	    sprintf(str, "POINTS %d float\n",tot_num_pnts);
	    length = count_digits(tot_num_pnts);
	    fwrite(str, sizeof(char), 14 + length, file);

	    tot_num_bonds = 0;
            for (c = intfc->curves; c && *c; ++c)
            {
		float vals[1];
                if (is_subdomain_boundary(Hyper_surf(*c)))continue;
                tot_num_bonds = tot_num_bonds+(*c)->num_points - 1;
		
		for (b = (*c)->first; b; b = b->next)
                {
		    float vals[1];
                    ps = b->start;
                    pe = b->end;
                    
		    if(hardware_is_little_endian())
		    	vals[0] = endian_float_swap(Coords(ps)[0]);
		    else
			vals[0] = Coords(ps)[0];
		    fwrite(vals, sizeof(float), 1, file);
		    if(hardware_is_little_endian())
			vals[0] = endian_float_swap(Coords(ps)[1]);
		    else
		        vals[0] = Coords(ps)[1];
                    fwrite(vals, sizeof(float), 1, file);
		    if(hardware_is_little_endian())
		        vals[0] = endian_float_swap(0.0);
		    else
			vals[0] = 0.0;
                    fwrite(vals, sizeof(float), 1, file);
                }
		

                if(hardware_is_little_endian())
 		    vals[0] = endian_float_swap(Coords(pe)[0]);
		else
		    vals[0] = Coords(pe)[0];
		fwrite(vals, sizeof(float), 1, file);
                if(hardware_is_little_endian())
                    vals[0] = endian_float_swap(Coords(pe)[1]);
                else
                    vals[0] = Coords(pe)[1];
                fwrite(vals, sizeof(float), 1, file);
                if(hardware_is_little_endian())
                    vals[0] = endian_float_swap(0.0);
                else
                    vals[0] = 0.0;
                fwrite(vals, sizeof(float), 1, file);	
	    }
	    sprintf(str, "\nLINES %i %i\n", tot_num_bonds,tot_num_bonds*3);
	    length = count_digits(tot_num_bonds);
	    length2 = count_digits(tot_num_bonds*3);
	    fwrite(str, sizeof(char), 9 + length + length2, file);
	 
	    first = 0;
            second = 1;
            for (c = intfc->curves; c && *c; ++c)
            {
                if (is_subdomain_boundary(Hyper_surf(*c)))continue;
                for (b = (*c)->first; b; b = b->next)
                {
		    if(hardware_is_little_endian())
		        ivals[0] = endian_int_swap(2);
		    else
			ivals[0] = 2;
		    fwrite(ivals, sizeof(int), 1, file);
		    if(hardware_is_little_endian())
                        ivals[0] = endian_int_swap(first);
                    else
                        ivals[0] = first;
                    fwrite(ivals, sizeof(int), 1, file);
                    if(hardware_is_little_endian())
                        ivals[0] = endian_int_swap(second);
                    else
                        ivals[0] = second;
                    fwrite(ivals, sizeof(int), 1, file);
			
                    first++;
                    second++;
                }
                first++;
                second++;
	    }	
        }
	else
	{	
	    if ((file = fopen(fname,"w")) == NULL)
            {
             	screen("WARNING in vtk_plot_curves(), "
                        "can't open %s\n",fname);
                return;
            }

	    (void) fprintf(file,"# vtk DataFile Version 3.0 \n"
		                "FronTier Interface \n"
		       	        "ASCII \n"
			        "DATASET POLYDATA \n");
	    (void) fprintf(file, "FIELD FieldData 2\n");
	    (void) fprintf(file, "TIME 1 1 float\n");
	    (void) fprintf(file, "%5.5e\n", time);
	    (void) fprintf(file, "CYCLE 1 1 int\n");
	    (void) fprintf(file, "%d\n", step);
	    (void) fprintf(file, "POINTS %d double\n",tot_num_pnts);
	
	    tot_num_bonds = 0;
            for (c = intfc->curves; c && *c; ++c)
            {
                if (is_subdomain_boundary(Hyper_surf(*c)))continue;
	        tot_num_bonds = tot_num_bonds+(*c)->num_points - 1;
                for (b = (*c)->first; b; b = b->next)
                {
                    ps = b->start;
                    pe = b->end;
                    (void) fprintf(file,"%-9g %-9g 0.0\n",
			       Coords(ps)[0], Coords(ps)[1]);
            	}
                (void) fprintf(file,"%-9g %-9g 0.0\n",
			       Coords(pe)[0],Coords(pe)[1]);
            }
	    fprintf(file,"LINES %i %i \n",tot_num_bonds,tot_num_bonds*3);
	    first=0;
	    second=1;
            for (c = intfc->curves; c && *c; ++c)
            {
                if (is_subdomain_boundary(Hyper_surf(*c)))continue;
                for (b = (*c)->first; b; b = b->next)
                {
                    (void) fprintf(file,"2 %i %i \n",first,second);
		    first++;
		    second++;
                }
	        first++;
	        second++;
            }
	}
        fclose(file);
}               /*end vtk_plot_curves2d*/

LOCAL   void    vtk_plot_curves3d(
        INTERFACE     *intfc,
	const double   *BBL,
	const double   *BBU,
        const char    *dname,
        const char    *name,
	SURFACE_COLOR color,
	int	      width,
	boolean  	      print_in_binary,
	double 	      time,
	int 	      step)
{
        FILE              *file;
        FT_POINT             *ps, *pe;
        CURVE             **c;
        BOND              *b;
        static const char *indent = "    ";
        int               num_bonds,i,first,second;
	static char       *fname = NULL;
	static size_t     fname_len = 0;
	int		  tot_num_pnts,tot_num_bonds; 
	char str[100];
	int ivals[1];
	int length, length2;

	fname = get_vtk_file_name(fname,dname,name,&fname_len);

	tot_num_pnts = 0;
	for (c = intfc->curves; c && *c; ++c)
	{
            if (is_subdomain_boundary(Hyper_surf(*c)))continue;
            tot_num_pnts = tot_num_pnts+(*c)->num_points;
	}
	if(print_in_binary)
	{
	    float value1[1],tmp1;
	    int value2[1],tmp2;
	    if ((file = fopen(fname,"wb")) == NULL)
            {
                screen("WARNING in vtk_plot_curves(), "
                       "can't open %s\n",fname);
                return;
	    }
	    sprintf(str,"# vtk DataFile Version 3.0\n");
	    fwrite(str, sizeof(char), 27, file);
	    sprintf(str,"FronTier Interface\n");
	    fwrite(str, sizeof(char), 19, file);
	    sprintf(str,"BINARY\n");
	    fwrite(str, sizeof(char), 7, file);
	    sprintf(str, "DATASET POLYDATA\n");
    	    fwrite(str, sizeof(char), 17, file);

	    sprintf(str, "FIELD FieldData 2\n");
            fwrite(str, sizeof(char), 18, file);
	    sprintf(str, "TIME 1 1 float\n");
            fwrite(str, sizeof(char), 15, file);

	    tmp1 = time;
            if(hardware_is_little_endian())
	        value1[0] = endian_float_swap(tmp1);
            else
		value1[0] = tmp1;
	    fwrite(value1, sizeof(float), 1, file);

	    sprintf(str, "\n");
            fwrite(str, sizeof(char), 1, file);
	    sprintf(str, "CYCLE 1 1 int\n");
            fwrite(str, sizeof(char), 14, file);
	    #if defined(int)
            #undef int
            #define not_int
            #endif
	    tmp2 = step;
            if(hardware_is_little_endian())
	        value2[0] = endian_int_swap(tmp2);
            else
		value2[0] = tmp2;
	    fwrite(value2, sizeof(int), 1, file);
	    #if defined(not_int)
	    #define int double
     	    #undef not_int
	    #endif
	    sprintf(str, "\n");
            fwrite(str, sizeof(char), 1, file);

	    sprintf(str, "POINTS %d float\n",tot_num_pnts);
	    length = count_digits(tot_num_pnts);
	    fwrite(str, sizeof(char), 14 + length, file);

	    tot_num_bonds = 0;
            for (c = intfc->curves; c && *c; ++c)
            {
		float vals[1];
                if (is_subdomain_boundary(Hyper_surf(*c)))continue;
                tot_num_bonds = tot_num_bonds+(*c)->num_points - 1;
		
		for (b = (*c)->first; b; b = b->next)
                {
		    float vals[1];
                    ps = b->start;
                    pe = b->end;
                    
		    if(hardware_is_little_endian())
		    	vals[0] = endian_float_swap(Coords(ps)[0]);
		    else
			vals[0] = Coords(ps)[0];
		    fwrite(vals, sizeof(float), 1, file);
		    if(hardware_is_little_endian())
			vals[0] = endian_float_swap(Coords(ps)[1]);
		    else
		        vals[0] = Coords(ps)[1];
                    fwrite(vals, sizeof(float), 1, file);
		    if(hardware_is_little_endian())
		        vals[0] = endian_float_swap(Coords(ps)[2]);
		    else
			vals[0] = Coords(ps)[2];
                    fwrite(vals, sizeof(float), 1, file);
                }
		

                if(hardware_is_little_endian())
 		    vals[0] = endian_float_swap(Coords(pe)[0]);
		else
		    vals[0] = Coords(pe)[0];
		fwrite(vals, sizeof(float), 1, file);
                if(hardware_is_little_endian())
                    vals[0] = endian_float_swap(Coords(pe)[1]);
                else
                    vals[0] = Coords(pe)[1];
                fwrite(vals, sizeof(float), 1, file);
                if(hardware_is_little_endian())
                    vals[0] = endian_float_swap(Coords(pe)[2]);
                else
                    vals[0] = Coords(pe)[2];
                fwrite(vals, sizeof(float), 1, file);	
	    }
	    sprintf(str, "\nLINES %i %i\n", tot_num_bonds,tot_num_bonds*3);
	    length = count_digits(tot_num_bonds);
	    length2 = count_digits(tot_num_bonds*3);
	    fwrite(str, sizeof(char), 9 + length + length2, file);
	 
	    first = 0;
            second = 1;
            for (c = intfc->curves; c && *c; ++c)
            {
                if (is_subdomain_boundary(Hyper_surf(*c)))continue;
                for (b = (*c)->first; b; b = b->next)
                {
		    if(hardware_is_little_endian())
		        ivals[0] = endian_int_swap(2);
		    else
			ivals[0] = 2;
		    fwrite(ivals, sizeof(int), 1, file);
		    if(hardware_is_little_endian())
                        ivals[0] = endian_int_swap(first);
                    else
                        ivals[0] = first;
                    fwrite(ivals, sizeof(int), 1, file);
                    if(hardware_is_little_endian())
                        ivals[0] = endian_int_swap(second);
                    else
                        ivals[0] = second;
                    fwrite(ivals, sizeof(int), 1, file);
			
                    first++;
                    second++;
                }
                first++;
                second++;
	    }	
        }
	else
	{	
	    if ((file = fopen(fname,"w")) == NULL)
            {
             	screen("WARNING in vtk_plot_curves(), "
                        "can't open %s\n",fname);
                return;
            }

	    (void) fprintf(file,"# vtk DataFile Version 3.0 \n"
		                "FronTier Interface \n"
		       	        "ASCII \n"
			        "DATASET POLYDATA \n");
	    (void) fprintf(file, "FIELD FieldData 2\n");
	    (void) fprintf(file, "TIME 1 1 float\n");
	    (void) fprintf(file, "%5.5e\n", time);
	    (void) fprintf(file, "CYCLE 1 1 int\n");
	    (void) fprintf(file, "%d\n", step);
	    (void) fprintf(file, "POINTS %d double\n",tot_num_pnts);
	
	    tot_num_bonds = 0;
            for (c = intfc->curves; c && *c; ++c)
            {
                if (is_subdomain_boundary(Hyper_surf(*c)))continue;
	        tot_num_bonds = tot_num_bonds+(*c)->num_points - 1;
                for (b = (*c)->first; b; b = b->next)
                {
                    ps = b->start;
                    pe = b->end;
                    (void) fprintf(file,"%-9g %-9g %-9g\n",
			       Coords(ps)[0], Coords(ps)[1], Coords(ps)[2]);
            	}
                (void) fprintf(file,"%-9g %-9g %-9g\n",
			       Coords(pe)[0],Coords(pe)[1],Coords(pe)[2]);
            }
	    fprintf(file,"LINES %i %i \n",tot_num_bonds,tot_num_bonds*3);
	    first=0;
	    second=1;
            for (c = intfc->curves; c && *c; ++c)
            {
                if (is_subdomain_boundary(Hyper_surf(*c)))continue;
                for (b = (*c)->first; b; b = b->next)
                {
                    (void) fprintf(file,"2 %i %i \n",first,second);
		    first++;
		    second++;
                }
	        first++;
	        second++;
            }
	}
        fclose(file);
}               /*end vtk_plot_curves3d*/

/* END NEW VTK */

/* START CGNS FUNCS */
#ifdef USE_CGNS
LOCAL	void   cgns_print_box(
        const char    *dname,
	const double   *BBL,
        const double   *BBU,
	boolean	      print_in_binary
	)
{
	FILE *file;
	static char       *fname = NULL;
	static size_t     fname_len = 0;
	char vfmt[256];
	char str[100];
	int ival[1];

	/************* New for CGNS *******************************/
	double x[8],y[8],z[8];
	int ielem[12][2];
   	int isize[3];
   	int index_file,icelldim,iphysdim,index_base;
   	int index_zone,index_coord,index_section;  
   	int nelem_start,nelem_end,nbdyelem;
   	char basename[33],zonename[33];
	/**********************************************************/

	fname = get_cgns_file_name(fname,dname,"box",&fname_len);
        
	/* WRITE X, Y, Z GRID POINTS TO CGNS FILE */

	/* create gridpoints for simple example: */
	x[0]=BBL[0]; x[1]=BBU[0]; x[2]=BBU[0]; x[3]=BBL[0]; x[4]=BBL[0]; x[5]=BBU[0]; x[6]=BBU[0]; x[7]=BBL[0];
	y[0]=BBL[1]; y[1]=BBL[1]; y[2]=BBU[1]; y[3]=BBU[1]; y[4]=BBL[1]; y[5]=BBL[1]; y[6]=BBU[1]; y[7]=BBU[1];
	z[0]=BBL[2]; z[1]=BBL[2]; z[2]=BBL[2]; z[3]=BBL[2]; z[4]=BBU[2]; z[5]=BBU[2]; z[6]=BBU[2]; z[7]=BBU[2];
	
	/* open CGNS file for write */
   	cg_open(fname,CG_MODE_WRITE,&index_file);
	/* create base (user can give any name)*/
   	strcpy(basename,"Base");
   	icelldim=1;
   	iphysdim=3;
   	cg_base_write(index_file,basename,icelldim,iphysdim,&index_base);

	/* define zone name (user can give any name)*/
   	strcpy(zonename,"Zone 1");

	/* number of vertices*/
   	isize[0]=8;
	/* number of elements*/
   	isize[1]=12;
	/* boundary vertex size (zero if elements not sorted)*/
   	isize[2]=0;
	/* create zone*/
   	cg_zone_write(index_file,index_base,zonename,isize,Unstructured,
		&index_zone);
	/* write grid coordinates (user must use SIDS-standard names here)*/
   	cg_coord_write(index_file,index_base,index_zone,RealDouble,"CoordinateX",
      		x,&index_coord);
   	cg_coord_write(index_file,index_base,index_zone,RealDouble,"CoordinateY",
      		y,&index_coord);
   	cg_coord_write(index_file,index_base,index_zone,RealDouble,"CoordinateZ",
		z,&index_coord);

	/* set element connectivity:*/
	/* index no of first element*/
   	nelem_start=1;

	    ielem[0][0] = 1; ielem[0][1] = 2;
	    ielem[1][0] = 2; ielem[1][1] = 3;
	    ielem[2][0] = 3; ielem[2][1] = 4;
	    ielem[3][0] = 4; ielem[3][1] = 1;
	    ielem[4][0] = 5; ielem[4][1] = 6;
	    ielem[5][0] = 6; ielem[5][1] = 7;
	    ielem[6][0] = 7; ielem[6][1] = 8;
	    ielem[7][0] = 8; ielem[7][1] = 5;
	    ielem[8][0] = 1; ielem[8][1] = 5;
	    ielem[9][0] = 2; ielem[9][1] = 6;
	    ielem[10][0]= 3; ielem[10][1]= 7;
	    ielem[11][0]= 4; ielem[11][1]= 8;

	/* index no of last element */
   	nelem_end = 12;
	/* unsorted boundary elements*/
   	nbdyelem=0;
 	/* write BAR_2 element connectivity (user can give any name)*/
   	cg_section_write(index_file,index_base,index_zone,"Elem",BAR_2,
		nelem_start,nelem_end,nbdyelem,*ielem,&index_section);
	/* close CGNS file*/
   	cg_close(index_file);
}		/*****end cgns_print_box*******/

EXPORT	char   *get_cgns_file_name(
	char       *fname,
	const char *dname,
	const char *name,
	size_t     *fname_len)
{
	size_t len;
	if (dname == NULL)
	    dname = "";
	len = strlen(dname)+1+strlen(name)+6;
	if (*fname_len < len)
	{
	    *fname_len = len;
	    if (fname != NULL)
		free(fname);
	    uni_array(&fname,*fname_len,CHAR);
	}
	if (strlen(dname) != 0)
	    (void) sprintf(fname,"%s/%s.cgns",dname,name);
	else
	    (void) sprintf(fname,"%s.cgns",name);
	return fname;
}		/*end get_cgns_file_name*/

EXPORT void cgns_interface_plot(
	const char *dname,
	INTERFACE  *intfc,
	boolean print_in_binary,
	double time,
	int step)
{
	double *BBL = topological_grid(intfc).GL;
	double *BBU = topological_grid(intfc).GU;

	if (create_directory(dname,YES) == FUNCTION_FAILED)
	{
	    (void) printf("WARNING in cgns_interface_plot(), directory "
			  "%s doesn't exist and can't be created\n",dname);
	    return;
	}
	if (intfc->dim == 2)
	cgns_plot_curves(intfc,BBL, BBU,dname,"2d-intfc", pRED, 10, print_in_binary,time,step);
	if (intfc->dim == 3)
	{
	cgns_plot_surfaces(intfc,BBL,BBU,NO,dname,"3d-intfc",NO,pRED,pRED,print_in_binary,time,step); 
	cgns_print_box(dname,BBL,BBU,print_in_binary);
	}

}		/*end cgns_interface_plot*/


LOCAL	void	cgns_plot_surfaces(
	INTERFACE     *intfc,
	const double   *BBL,
	const double   *BBU,
	boolean          clip,
	const char    *dname,
	const char    *name,
	boolean	      bdry,
	SURFACE_COLOR color1,
	SURFACE_COLOR color2,
	boolean 	      print_in_binary,
	double 	      time,
	int 	      step)
{
	FILE	          *file;
	FT_POINT             *p;
	SURFACE	          **s;
	TRI	          *tri;
	boolean              plot_surf;
	double 	          D, intensity = .5;
	double             L[MAXD],U[MAXD],tol[MAXD];
	double	          *crds;
	int	          num_surfs, num_tris, i, j, k, l;
	int               npts, ntris;
	int		  length, length2;
	static const char *indent = "    ";
	static int        *verts = NULL;
	static int        alloc_len_verts = 0, alloc_len_pts = 0;
	static char       *fname = NULL;
	char 		  str[100];
	static size_t  	  fname_len = 0;
	RECT_GRID *gr = &topological_grid(intfc);
        /************* New for CGNS *******************************/
	static double     *xs = NULL, *ys = NULL, *zs = NULL;
	static int 	  **ielem = NULL;
	int 		  num_pts;
   	int isize[3];
   	int index_file,icelldim,iphysdim,index_base;
   	int index_zone,index_coord,index_section;  
   	int nelem_start, nelem_end, nbdyelem;
   	char basename[33],zonename[33];
	/**********************************************************/
	
	fname = get_cgns_file_name(fname,dname,name,&fname_len);
	
	num_pts = intfc->num_points;
	for (num_tris = 0, s = intfc->surfaces; s && *s; ++s)
	{
	    num_tris += (*s)->num_tri;
	    for (tri=first_tri(*s); !at_end_of_tri_list(tri,*s); tri=tri->next)
	    {
	        for (k = 0; k < 3; ++k)
		    Index_of_point(Point_of_tri(tri)[k]) = -1;
	    }
	}
	
	if (alloc_len_pts < 3*intfc->num_points)
	{
	    uni_array(&xs,num_pts,FLOAT);
	    uni_array(&ys,num_pts,FLOAT);
	    uni_array(&zs,num_pts,FLOAT);
	}
	
	if (alloc_len_verts < 3*num_tris)
	{
	    if (verts != NULL)
		free(verts);
	    alloc_len_verts = 3*num_tris;
	    uni_array(&verts,alloc_len_verts,INT);
	    bi_array(&ielem,num_tris,3,INT);
	}
	for (i = 0; i < 3; i++)
	{
	    L[i] = gr->L[i] - 0.5*gr->h[i]; 
	    U[i] = gr->U[i] + 0.5*gr->h[i];
	    tol[i] = 0.00001*gr->h[i];
	}

        for (npts=0, ntris=0, num_surfs=0, s = intfc->surfaces; s && *s; ++s)
	{
	    if (bdry == YES  &&  !Boundary(*s))
		continue; 
	    if (bdry == NO  &&  Boundary(*s))
		continue;
	    if (clip == YES)
	    {
		plot_surf = NO;
	        for (tri = first_tri(*s); !at_end_of_tri_list(tri,*s); 
		     tri = tri->next)
	        {
	            for (k = 0; k < 3; ++k)
		    {
			crds = Coords(Point_of_tri(tri)[k]);
	                for (l = 0; l < 3; ++l)
			    if ((crds[l] < L[l] - tol[l]) || 
			        (U[l] + tol[l] < crds[l]))
				break;
			if (l < 3)
			    break;
		    }
		    if (k == 3)
		    {
			plot_surf = YES;
			for (k = 0; k < 3; ++k)
			{
		            p = Point_of_tri(tri)[k];
			    if (Index_of_point(p) == -1)
			    {
			        crds = Coords(p);
				xs[npts]=crds[0];
				ys[npts]=crds[1];
				zs[npts]=crds[2];
				Index_of_point(p) = npts++;
			    }
			ielem[ntris][k] = Index_of_point(p)+1;
			}
			++ntris;
		    }
		}
		if (plot_surf == YES)
		    ++num_surfs;
	    }
	    else
	    {
	        for (tri = first_tri(*s); !at_end_of_tri_list(tri,*s); 
		     tri = tri->next)
	        {
	            for (k = 0; k < 3; ++k)
		    {
		        p = Point_of_tri(tri)[k];
			if (Index_of_point(p) == -1)
			{
			    crds = Coords(p);
	                    xs[npts]=crds[0];
			    ys[npts]=crds[1];
			    zs[npts]=crds[2];
			    Index_of_point(p) = npts++;
			}
			ielem[ntris][k] = Index_of_point(p)+1;
		    }
		    ++ntris;
		}
		++num_surfs;
	    }
	}
	if (num_surfs == 0)
	    return;
        
	/* WRITE X, Y, Z GRID POINTS TO CGNS FILE */
	/* open CGNS file for write */
   	cg_open(fname,CG_MODE_WRITE,&index_file);
	/* create base (user can give any name)*/
   	strcpy(basename,"Base");
   	icelldim=2;
   	iphysdim=3;
   	cg_base_write(index_file,basename,icelldim,iphysdim,&index_base);

	/* define zone name (user can give any name)*/
   	strcpy(zonename,"Zone 1");
   
	/* Number of vertices*/
   	isize[0]=npts;
	/* Number of elements*/
   	isize[1]=ntris;
	/* boundary vertex size (zero if elements not sorted)*/
   	isize[2]=0;
	/* create zone*/
   	cg_zone_write(index_file,index_base,zonename,isize,Unstructured,
		&index_zone);
	/* write grid coordinates (user must use SIDS-standard names here)*/
   	cg_coord_write(index_file,index_base,index_zone,RealDouble,"CoordinateX",
      		xs,&index_coord);
   	cg_coord_write(index_file,index_base,index_zone,RealDouble,"CoordinateY",
      		ys,&index_coord);
   	cg_coord_write(index_file,index_base,index_zone,RealDouble,"CoordinateZ",
      		zs,&index_coord);

	/* set element connectivity:*/
	/* index no of first element*/
   	nelem_start=1;
	/* index no of last element */
   	nelem_end = ntris;
	/* unsorted boundary elements*/
   	nbdyelem=0;
 	/* write TRI_3 element connectivity (user can give any name)*/
   	cg_section_write(index_file,index_base,index_zone,"Elem",TRI_3,
		nelem_start,nelem_end,nbdyelem,*ielem,&index_section);
	/* close CGNS file*/
   	cg_close(index_file);
}	/*end cgns_plot_surfaces*/


LOCAL   void    cgns_plot_curves(
        INTERFACE     *intfc,
	const double   *BBL,
	const double   *BBU,
        const char    *dname,
        const char    *name,
	SURFACE_COLOR color,
	int	      width,
	boolean  	      print_in_binary,
	double 	      time,
	int 	      step)
{
        FILE              *file;
        FT_POINT             *ps, *pe;
        CURVE             **c;
        BOND              *b;
        static const char *indent = "    ";
        int               num_bonds,i,j,first,second;
	static char       *fname = NULL;
	static size_t     fname_len = 0;
	int		  tot_num_pnts,tot_num_bonds; 
	char str[100];
	int ivals[1];
	int length, length2;
	/************* New for CGNS *******************************/
	static double     *xs = NULL, *ys = NULL;
	static int 	  **ielem = NULL;
	int 		  num_pts;
   	int isize[3];
   	int index_file,icelldim,iphysdim,index_base;
   	int index_zone,index_coord,index_section;  
   	int nelem_start, nelem_end, nbdyelem;
   	char basename[33],zonename[33];
	/**********************************************************/
	
	fname = get_cgns_file_name(fname,dname,name,&fname_len);

	tot_num_pnts = 0;
	for (c = intfc->curves; c && *c; ++c)
	{
            if (is_subdomain_boundary(Hyper_surf(*c)))continue;
            tot_num_pnts = tot_num_pnts+(*c)->num_points;
	}

	uni_array(&xs,tot_num_pnts,FLOAT);
	uni_array(&ys,tot_num_pnts,FLOAT);
	
	i=0;
	tot_num_bonds = 0;
        for (c = intfc->curves; c && *c; ++c)
        {
            if (is_subdomain_boundary(Hyper_surf(*c)))continue;
	    tot_num_bonds = tot_num_bonds+(*c)->num_points - 1;
	    for (b = (*c)->first; b; b = b->next)
            {
                ps = b->start;
                pe = b->end;
                xs[i] = Coords(ps)[0]; 
		ys[i] = Coords(ps)[1];
		i++;
            }
            xs[i] = Coords(pe)[0]; 
	    ys[i] = Coords(pe)[1];
	    i++;
	    bi_array(&ielem,tot_num_bonds,2,INT);
        }
	first = 1;
	second = 2;
	j = 0;
        for (c = intfc->curves; c && *c; ++c)
        {
            if (is_subdomain_boundary(Hyper_surf(*c)))continue;
            for (b = (*c)->first; b; b = b->next)
            {
		ielem[j][0] = first;
		ielem[j][1] = second;
		first++;
		second++;
		j++;
            }
	    first++;
	    second++;
        }

	/* WRITE X, Y, Z GRID POINTS TO CGNS FILE */
	/* open CGNS file for write */
   	cg_open(fname,CG_MODE_WRITE,&index_file);
	/* create base (user can give any name)*/
   	strcpy(basename,"Base");
   	icelldim=1;
   	iphysdim=2;
   	cg_base_write(index_file,basename,icelldim,iphysdim,&index_base);

	/* define zone name (user can give any name)*/
   	strcpy(zonename,"Zone 1");
   
	/* Number of vertices*/
   	isize[0]=tot_num_pnts;
	/* Number of elements*/
   	isize[1]=tot_num_bonds;
	/* boundary vertex size (zero if elements not sorted)*/
   	isize[2]=0;
	/* create zone*/
   	cg_zone_write(index_file,index_base,zonename,isize,Unstructured,
		&index_zone);
	/* write grid coordinates (user must use SIDS-standard names here)*/
   	cg_coord_write(index_file,index_base,index_zone,RealDouble,"CoordinateX",
      		xs,&index_coord);
   	cg_coord_write(index_file,index_base,index_zone,RealDouble,"CoordinateY",
      		ys,&index_coord);

	/* set element connectivity:*/
	/* index no of first element*/
   	nelem_start=1;
	/* index no of last element */
   	nelem_end = tot_num_bonds;
	/* unsorted boundary elements*/
   	nbdyelem=0;
 	/* write BAR_2 element connectivity (user can give any name)*/
   	cg_section_write(index_file,index_base,index_zone,"Elem",BAR_2,
		nelem_start,nelem_end,nbdyelem,*ielem,&index_section);
	/* close CGNS file*/
   	cg_close(index_file);
}    
/* END CGNS FUNCS */
#endif /*end USE_CGNS */

#if defined (__GD__)

/* 1D data plotter using GD graphics library
 * By Ryan Kaufman
 */

static boolean no_bullet;

struct DATA
{
	/*out file */
	FILE *gifout;

	/*base image */
	gdImagePtr im;
	gdImagePtr locim;

	/*Data range: */
	double maxx;
	double minx;
	double maxy;
	double miny;

	/*Window range: */
	int xmax;
	int xmin;
	int ymax;
	int ymin;

	double xheight;
	double yheight;

	/*colors */
	int fg[3];
	int bg[3];
	int fg_col; /*pointers to allocated color */
	int bg_col; /*... */

	/*interfaces */
	int Numi;
	int section;

	/*last positions (from previous call to plotdata) */
	int lastx;
	int lasty;
};
struct DATA PLOT_DATA;

/***********************************************************************/

LOCAL void mkImg(
	int N, 
	int *x, 
	int *y)
{
	static const int r=5;
	gdImagePtr locim;
	int fg,bg;
	int i,Color,Numi;

	if(PLOT_DATA.section == 0)
        {
	    locim = gdImageCreate(PLOT_DATA.xmax, PLOT_DATA.ymax);
	    bg = gdImageColorAllocate(locim, 
		PLOT_DATA.bg[0], PLOT_DATA.bg[1], PLOT_DATA.bg[2]);
	    fg = gdImageColorAllocate(locim, 
		PLOT_DATA.fg[0], PLOT_DATA.fg[1], PLOT_DATA.fg[2]);
	    PLOT_DATA.fg_col = fg;
	    PLOT_DATA.bg_col = fg;
	}
	else
	{
	    locim = PLOT_DATA.locim;
	    fg = PLOT_DATA.fg_col;
	    bg = PLOT_DATA.bg_col;
	    /*
            gdImageLine(locim, x[0], y[0], 
	                PLOT_DATA.lastx, PLOT_DATA.lasty, fg);
	    */
	}

	Numi = PLOT_DATA.Numi;

	if(Numi != 0) 
	{
	    if(PLOT_DATA.section == 0)
	        setcolors(locim, Numi);
            Color = getcolor(PLOT_DATA.section);
	}
        else
            Color = fg;
	for(i=0; i<N-1; i++)
	{
            gdImageLine(locim, x[i], y[i], x[i+1],y[i+1], Color);
	    if (!no_bullet)
	        gdImageArc(locim, x[i], y[i], r, r, 0, 360, Color);
	}
	if (!no_bullet)
	    gdImageArc(locim, x[i], y[i], r, r, 0, 360, Color);
	PLOT_DATA.lastx = x[i]; PLOT_DATA.lasty = y[i];

	if(Numi != 0 && PLOT_DATA.section == Numi)
	{
	    freecolors();
	}


        PLOT_DATA.locim = locim;

	return;
}	/*END mkImg() */

/* Transforms given coordinates into pixel coordinates on the graph */
LOCAL double xtransform(double x)
{
	register double xmax = PLOT_DATA.xmax - 0.1*PLOT_DATA.xheight;
	register double xmin = PLOT_DATA.xmin + 0.1*PLOT_DATA.xheight;

        return (int)( ((double)(xmax-xmin)/
	  (PLOT_DATA.maxx-PLOT_DATA.minx))*(x-PLOT_DATA.minx)
	  + xmin);
}	/*END xtransform() */

/* Transforms given coordinates into pixel coordinates on the graph */
LOCAL double ytransform(double y)
{
	register double ymax = PLOT_DATA.ymax - 0.1*PLOT_DATA.yheight;
	register double ymin = PLOT_DATA.ymin + 0.1*PLOT_DATA.yheight;

        return PLOT_DATA.ymax-(int)(((double)(ymax-ymin)/
	  (PLOT_DATA.maxy-PLOT_DATA.miny))*(y-PLOT_DATA.miny)
	  + ymin);
}	/*END ytransform() */

/*Sets up the template image... prints title, axes, etc. */
LOCAL void initialize_image(
	gdImagePtr im, 
	const char *title, 
	int fg, 
	int bg)
{
        int i,N;       /* 10 xtics */
	int x;         /* location along axis */
	char s[10];    /* string to print */
	int dist;      /* spacing from axis to letters */
	int bot,top,left,right;
	gdFontPtr Font;

	/*print title: */
	if(title!=NULL)
	{
	    gdFontPtr Font;
	    Font = gdFontGetLarge();
	    gdImageString(im,Font,15,0,(unsigned char*)title,fg);
        }

	/*draw box axes */
	bot =   PLOT_DATA.ymax - (int) (0.1*(double)PLOT_DATA.yheight);
	top =   PLOT_DATA.ymin + (int) (0.1*(double)PLOT_DATA.yheight);
	left =  PLOT_DATA.xmin + (int) (0.1*(double)PLOT_DATA.xheight);
	right = PLOT_DATA.xmax - (int) (0.1*(double)PLOT_DATA.xheight);

	/*left */
	gdImageLine(im, left,top, 
		left, bot, fg);

	/*top */
	gdImageLine(im, left,top, 
		right,top, fg);

	/*right */
	gdImageLine(im, right,top, 
		right,bot, fg);

	/*bottom */
	gdImageLine(im, right,bot, 
		left,bot, fg);

	/*Make Graph Tics */
	Font = gdFontGetTiny();

	/*make xtics */
	N=10; /* 10 xtics */
	for(i=0; i<N; i++)
	{
	    x = left + i*(right-left)/(N-1);
	    gdImageLine(im,x,bot,x,bot+(int)(0.01*(double)PLOT_DATA.yheight),fg);
   	    sprintf(s,"%.2f", 
	    	PLOT_DATA.minx + i*(PLOT_DATA.maxx - PLOT_DATA.minx)/(N-1));
	    dist = 5*strlen(s) + 5;
	    gdImageStringUp(im,Font,x-4,bot+dist,(unsigned char*)s,fg);
	}

	/*make ytics */
	N=10; /*10 ytics */
	for(i=0; i<N; i++)
	{
	    x = bot - i*(bot - top)/(N-1);
	    gdImageLine(im,left,x,
	    	left-(int)(0.01*(double)PLOT_DATA.xheight),x,fg);
   	    sprintf(s,"%.2f", 
	    	PLOT_DATA.miny + i*(PLOT_DATA.maxy - PLOT_DATA.miny)/(N-1));
	    dist = 5*strlen(s) + 5;
	    gdImageString(im,Font,left-dist,x-4,(unsigned char*)s,fg);
	}
}	/*END initialize_image() */


EXPORT void gd_closeplot()
{
	fclose(PLOT_DATA.gifout);
        gdImageDestroy(PLOT_DATA.im);
	if(PLOT_DATA.locim != NULL)
            gdImageDestroy(PLOT_DATA.locim);
	memset(&PLOT_DATA,0,sizeof(struct DATA));
}	/*END close_plot() */



/* Called to plot the stored frame (all sections) */
EXPORT void gd_plotframe(char *subtitle)
{
	
    	gdImagePtr locim = PLOT_DATA.locim;
    	gdImagePtr im = PLOT_DATA.im;

	gdImageCopy(locim, im, PLOT_DATA.xmin, PLOT_DATA.ymin, 
		PLOT_DATA.xmin, PLOT_DATA.ymin, 
		PLOT_DATA.xmax, PLOT_DATA.yheight*0.1+1);

	gdImageCopy(locim, im, PLOT_DATA.xmin, PLOT_DATA.ymin, 
		PLOT_DATA.xmin, PLOT_DATA.ymin, 
		PLOT_DATA.xheight*0.1+1, PLOT_DATA.ymax);
  
	gdImageCopy(locim, im, 
		PLOT_DATA.xmin, PLOT_DATA.ymax - PLOT_DATA.yheight*0.1-1,
		PLOT_DATA.xmin, PLOT_DATA.ymax - PLOT_DATA.yheight*0.1-1,
		PLOT_DATA.xmax, PLOT_DATA.yheight*0.1+1);

	gdImageCopy(locim, im, 
		PLOT_DATA.xmax - PLOT_DATA.xheight*0.1, PLOT_DATA.ymin,
		PLOT_DATA.xmax - PLOT_DATA.xheight*0.1, PLOT_DATA.ymin,
		PLOT_DATA.xheight*0.1+1, PLOT_DATA.ymax);

        /*draw subtitle: */
	if(subtitle != NULL)
	{
            gdFontPtr Font;
            Font = gdFontGetSmall();
            gdImageString(locim,Font,15,15,(unsigned char*)subtitle, 
	    			PLOT_DATA.fg_col);
	}

	/*write to gif file */
        gdImageGifAnimAdd(locim, PLOT_DATA.gifout, 0, 
			0, 0, 10, gdDisposalNone,0 );

	/*Clean up temp image */
        gdImageDestroy(locim);

	/*Reset PLOT_DATA for next frame */
	PLOT_DATA.locim = NULL;
	PLOT_DATA.section = 0;
}	/*END plotframe() */

/*
 * This is the function into which you drop the data for a single
 * frame of the plot animation. It will write the frame to the outfile.
 */
EXPORT void gd_plotdata(
	int N, 
	double *x, 
	double *y)
{
		/*printf("enter plotdata\n"); */

    	int *xl,*yl;
	int i;
	/*These are for the transformed coordinates */
	uni_array(&xl,N,INT);
	uni_array(&yl,N,INT);

	/*make transformed coordinates */
    	for(i = 0; i < N; i++)
    	{
	    xl[i] = xtransform(x[i]);
	    yl[i] = ytransform(y[i]);
    	}
	
	/* update the stored image by plotting these segments */
    	mkImg(N,xl,yl);

	/* Clean up memory allocation */
        free_these(2,xl,yl);

	/* On to the next section */
	PLOT_DATA.section++;

}	/* end plotdata */

/** Initialize a plot
 * set out file name and data range
 */
EXPORT void gd_initplot(char *file, char *title, 
	      double minx, double maxx, double miny, double maxy, int Numi)
{
        int bg,fg;
	no_bullet = NO;
	/*Window range */
	PLOT_DATA.xmin = 0; PLOT_DATA.xmax = 600;
	PLOT_DATA.ymin = 0; PLOT_DATA.ymax = 400;

	PLOT_DATA.xheight = PLOT_DATA.xmax-PLOT_DATA.xmin;
	PLOT_DATA.yheight = PLOT_DATA.ymax-PLOT_DATA.ymin;

	/*Data range */
	PLOT_DATA.minx = minx; PLOT_DATA.maxx = maxx;
	PLOT_DATA.miny = miny; PLOT_DATA.maxy = maxy;

	/*Set colors */
	PLOT_DATA.bg[0] = 255;
	PLOT_DATA.bg[1] = 255;
	PLOT_DATA.bg[2] = 255;

	/*set number of interfaces */
	PLOT_DATA.Numi = Numi;
	PLOT_DATA.section = 0;

	PLOT_DATA.fg[0] = 0;
	PLOT_DATA.fg[1] = 0;
	PLOT_DATA.fg[2] = 0;

	/*initialize base image handle and colors: */
	PLOT_DATA.im = gdImageCreate(PLOT_DATA.xmax, PLOT_DATA.ymax);
	PLOT_DATA.locim = NULL;

	bg = gdImageColorAllocate(PLOT_DATA.im, 
		PLOT_DATA.bg[0], PLOT_DATA.bg[1], PLOT_DATA.bg[2]);
	fg = gdImageColorAllocate(PLOT_DATA.im, 
		PLOT_DATA.fg[0], PLOT_DATA.fg[1], PLOT_DATA.fg[2]);
	PLOT_DATA.fg_col = fg;
	PLOT_DATA.bg_col = bg;

	setcolors(PLOT_DATA.im,Numi);
	freecolors();

	/* initialize the image, lay axes, etc. */
	initialize_image(PLOT_DATA.im, title, fg, bg);
	
	/* open output file */
	PLOT_DATA.gifout = fopen(file,"wb");

	/* Call gd Initialization function for GIF animation */
	gdImageGifAnimBegin(PLOT_DATA.im, PLOT_DATA.gifout, -1, 0);

	return;
}	/* END initplot() */

EXPORT void gd_appendplot(char *file, char *title, 
	      double minx, double maxx, double miny, double maxy, int Numi)
{
        int bg,fg;
	no_bullet = NO;
	/*Window range */
	PLOT_DATA.xmin = 0; PLOT_DATA.xmax = 600;
	PLOT_DATA.ymin = 0; PLOT_DATA.ymax = 400;

	PLOT_DATA.xheight = PLOT_DATA.xmax-PLOT_DATA.xmin;
	PLOT_DATA.yheight = PLOT_DATA.ymax-PLOT_DATA.ymin;

	/*Data range */
	PLOT_DATA.minx = minx; PLOT_DATA.maxx = maxx;
	PLOT_DATA.miny = miny; PLOT_DATA.maxy = maxy;

	/*Set colors */
	PLOT_DATA.bg[0] = 255;
	PLOT_DATA.bg[1] = 255;
	PLOT_DATA.bg[2] = 255;

	/*set number of interfaces */
	PLOT_DATA.Numi = Numi;
	PLOT_DATA.section = 0;

	PLOT_DATA.fg[0] = 0;
	PLOT_DATA.fg[1] = 0;
	PLOT_DATA.fg[2] = 0;

	/*initialize base image handle and colors: */
	PLOT_DATA.im = gdImageCreate(PLOT_DATA.xmax, PLOT_DATA.ymax);
	PLOT_DATA.locim = NULL;

	bg = gdImageColorAllocate(PLOT_DATA.im, 
		PLOT_DATA.bg[0], PLOT_DATA.bg[1], PLOT_DATA.bg[2]);
	fg = gdImageColorAllocate(PLOT_DATA.im, 
		PLOT_DATA.fg[0], PLOT_DATA.fg[1], PLOT_DATA.fg[2]);
	PLOT_DATA.fg_col = fg;
	PLOT_DATA.bg_col = bg;

	setcolors(PLOT_DATA.im,Numi);
	freecolors();

	/* initialize the image, lay axes, etc. */
	initialize_image(PLOT_DATA.im, title, fg, bg);
	
	/* open output file */
	fclose(PLOT_DATA.gifout);
	PLOT_DATA.gifout = fopen(file,"a");

	return;
}	/* END gd_appendplot() */

/*--------------------COLOR GEN CODE---------------------- */

LOCAL int *color_list;

EXPORT int colorval(int i, int N, int j)
{
        double frac;
	N++;
	if( j<0 || j>2) return -1;

	frac = ((double)i/(double)N);
	if(j==0)
	{
	    if(frac < 1.0/6.0 || frac >= 5.0/6.0)
	        return 255;
	    else if(frac < 2.0/6.0)
	    	return (int)((1.0 - 6.0*(frac - 1.0/6.0))*255);
	    else if(frac < 4.0/6.0)
	        return 0;
	    else if(frac < 5.0/6.0)
	    	return (int)((6.0*(frac - 4.0/6.0))*255);
	    else
	    	return -2;
	}
	if(j==1)
	{
	    if(frac < 1.0/6.0)
	    	return (int)((6.0*(frac))*255);
	    else if(frac < 3.0/6.0)
	    	return 255;
	    else if(frac < 4.0/6.0)
	    	return (int)((1.0 - 6.0*(frac - 3.0/6.0))*255);
	    else if(frac <= 1.0)
	        return 0;
	    else
	    	return -2;
	}
	if(j==2)
	{
	    if(frac < 2.0/6.0)
	        return 0;
	    else if(frac < 3.0/6.0)
	    	return (int)((6.0*(frac-2.0/6.0))*255);
	    else if(frac < 5.0/6.0)
	    	return 255;
	    else if(frac <= 1.0)
	    	return (int)((1.0 - 6.0*(frac - 5.0/6.0))*255);
	    else
	    	return -2;

	}
}


LOCAL void setcolors(gdImagePtr im, int N)
{
        int i;
	N++;

	if (N > 255) 
	{
	    printf("Error, too many components\n");
            return;
	}

	uni_array(&color_list,N,INT);
	for (i = 0; i < N; i++)
	{
            color_list[i] = gdImageColorAllocate(im, 
		    colorval(i,N,0), colorval(i,N,1),colorval(i,N,2));
        }
}

LOCAL void freecolors()
{
    	free_these(1,color_list);
}

LOCAL int getcolor(int i)
{
	return color_list[i];
}

EXPORT void gd_2d_intfc(
	char *dirname,
	char *label,
	INTERFACE *intfc,
	RECT_GRID *gr,
	int resolution_level,
	boolean use_bullet)
{
	/*Window range */
	char gname[100];
	int pixel[4] = {100000,200000,400000,800000};
	int num_xpix,num_ypix,total_pix;
	double *L = gr->GL;
	double *U = gr->GU;
	double lambda = (U[0] - L[0])/(U[1] - L[1]);
	int num_points,max_num_points,num_curves;
	CURVE **c;
	BOND *b;
	int i,j,bg,fg;
	static boolean first = YES;
	static int current_max_num_points = 0;
	static double *x,*y;

#if defined __MPI__
	if (pp_mynode() != 0) 
	    goto make_frame;
#endif /* defined __MPI__ */
	if (first) 
	    first = NO;
	else 
	    goto make_frame;
	no_bullet = (use_bullet == YES) ? NO : YES;
	if (resolution_level <= 0) resolution_level = 1;
	if (resolution_level >= 4) resolution_level = 4;
	total_pix = pixel[resolution_level-1];
	num_ypix = min(1000,(int)sqrt(total_pix/lambda));
	num_xpix = (int)(lambda*num_ypix);
	if (num_xpix > 1000)
	{
	    num_xpix = 1000;
	    num_ypix = (int)num_xpix/lambda;
	}

	PLOT_DATA.xmin = 0; PLOT_DATA.xmax = num_xpix;
	PLOT_DATA.ymin = 0; PLOT_DATA.ymax = num_ypix;

	PLOT_DATA.xheight = PLOT_DATA.xmax-PLOT_DATA.xmin;
	PLOT_DATA.yheight = PLOT_DATA.ymax-PLOT_DATA.ymin;

	/*Data range */
	PLOT_DATA.minx = L[0]; PLOT_DATA.maxx = U[0];
	PLOT_DATA.miny = L[1]; PLOT_DATA.maxy = U[1];

	/*Set colors */
	PLOT_DATA.bg[0] = 255;
	PLOT_DATA.bg[1] = 255;
	PLOT_DATA.bg[2] = 255;

	/*set number of interfaces */
	PLOT_DATA.Numi = 0;
	PLOT_DATA.section = 0;

	PLOT_DATA.fg[0] = 0;
	PLOT_DATA.fg[1] = 0;
	PLOT_DATA.fg[2] = 0;

	/*initialize base image handle and colors: */
	PLOT_DATA.im = gdImageCreate(PLOT_DATA.xmax, PLOT_DATA.ymax);
	PLOT_DATA.locim = NULL;

	bg = gdImageColorAllocate(PLOT_DATA.im, 
		PLOT_DATA.bg[0], PLOT_DATA.bg[1], PLOT_DATA.bg[2]);
	fg = gdImageColorAllocate(PLOT_DATA.im, 
		PLOT_DATA.fg[0], PLOT_DATA.fg[1], PLOT_DATA.fg[2]);
	PLOT_DATA.fg_col = fg;
	PLOT_DATA.bg_col = bg;

	setcolors(PLOT_DATA.im,PLOT_DATA.Numi);
	freecolors();

	/* initialize the image, lay axes, etc. */
	initialize_image(PLOT_DATA.im,"INTERFACE",fg,bg);
	
	/* open output file */
	sprintf(gname,"%s/intfc-gd.gif",dirname);
	PLOT_DATA.gifout = fopen(gname,"wb");

	/* Call gd Initialization function for GIF animation */
	gdImageGifAnimBegin(PLOT_DATA.im, PLOT_DATA.gifout, -1, 0);

make_frame:
	max_num_points = num_curves = 0;
	for (c = intfc->curves; c && *c; ++c)
	{
	    if (is_bdry(*c)) continue;
	    num_points = (*c)->num_points;
	    if (num_points > max_num_points)
		max_num_points = num_points;
	    num_curves++;
	}
	if (num_curves == 0) return;
#if defined __MPI__
	pp_global_imax(&max_num_points,1);
#endif /* defined __MPI__ */
	if (current_max_num_points < max_num_points)
	{
	    if (x != NULL) free_these(2,x,y);
	    uni_array(&x,max_num_points,FLOAT);
	    uni_array(&y,max_num_points,FLOAT);
	    current_max_num_points = max_num_points;
	}
#if defined __MPI__
	if (pp_mynode() != 0)
	{
	    pp_send(10,(POINTER)&num_curves,INT,0);
	}
#endif /* defined __MPI__ */
	for (c = intfc->curves; c && *c; ++c)
	{
	    if (is_bdry(*c)) continue;
	    num_points = 0;
	    for (b = (*c)->first; b != NULL; b = b->next)
	    {
		x[num_points] = Coords(b->start)[0];
		y[num_points] = Coords(b->start)[1];
		num_points++;
	    }
	    x[num_points] = Coords((*c)->last->end)[0];
	    y[num_points] = Coords((*c)->last->end)[1];
	    num_points++;
#if defined __MPI__
	    if (pp_mynode() != 0)
	    {
		pp_send(10,(POINTER)&num_points,INT,0);
		pp_send(10,(POINTER)x,num_points*FLOAT,0);
		pp_send(10,(POINTER)y,num_points*FLOAT,0);
	    }
	    else
#endif /* defined __MPI__ */
	    	gd_plotdata(num_points,x,y);
	}
#if defined __MPI__
	if (pp_mynode() == 0)
	{
	    for (i = 1; i < pp_numnodes(); ++i)
	    {
	    	pp_recv(10,i,(POINTER)&num_curves,INT);
	    	for (j = 0; j < num_curves; ++j)
	    	{
	    	    pp_recv(10,i,(POINTER)&num_points,INT);
	    	    pp_recv(10,i,(POINTER)x,num_points*FLOAT);
	    	    pp_recv(10,i,(POINTER)y,num_points*FLOAT);
	    	    gd_plotdata(num_points,x,y);
	    	}
	    }
#endif /* defined __MPI__ */
	    gd_plotframe(label);
	    fflush(PLOT_DATA.gifout);
#if defined __MPI__
	}
#endif /* defined __MPI__ */
}	/* end gd_2d_intfc */
#endif /* if defined(__GD__) */

EXPORT void sdl_interface_plot(
	const char *dname,
	INTERFACE  *intfc)
{
	char file_name[200];
	FILE *sdl_file;
	SURFACE **s;
	int i;
	FT_POINT *p;
	TRI *tri;

	if (intfc->dim != 3) return;

	if (create_directory(dname,YES) == FUNCTION_FAILED)
	{
	    (void) printf("WARNING in vtk_interface_plot(), directory "
			  "%s doesn't exist and can't be created\n",dname);
	    return;
	}
	sprintf(file_name,"%s/surfs.sdl",dname);
	sdl_file = fopen(file_name,"w");

	for (s = intfc->surfaces; s && *s; ++s)
	{
	    for (tri = first_tri(*s); !at_end_of_tri_list(tri,*s);
	    		tri = tri->next)
	    {
		const double *norm = Tri_normal(tri);
		double denom = Mag3d(norm);
		fprintf(sdl_file,"facet normal  %"FFMT"  %"FFMT"  %"FFMT"\n",
			norm[0]/denom,norm[1]/denom,norm[2]/denom);
		fprintf(sdl_file," outer loop\n");
		for (i = 0; i < 3; ++i)
		{
		    p = Point_of_tri(tri)[i];
	    	    fprintf(sdl_file,"vertex  %"FFMT"  %"FFMT"  %"FFMT"\n",
			Coords(p)[0],Coords(p)[1],Coords(p)[2]);
		}
		fprintf(sdl_file," endloop\n");
		fprintf(sdl_file,"endfacet\n");
	    }
	}
}	/*end sdl_interface_plot*/
