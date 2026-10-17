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
*				igrid.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	This file contains elementary routines for the manipulation
*	and printing of rectangular grids.
*/

#include <intfc/iloc.h>

/*Local Function Prototypes*/
LOCAL	double	area_of_rect_grid(const RECT_GRID*);
LOCAL	double	cylin_Area(const double*,const RECT_GRID*);
LOCAL	double	cylin_Volume(const double*,const RECT_GRID*);
LOCAL	double	ident_Length(const double*,const RECT_GRID*);
LOCAL	double	ident_Volume(const double*,const RECT_GRID*);
LOCAL	double	spherical_Volume(const double*,const RECT_GRID*);
LOCAL	void	prompt_for_remap(int,REMAP*);

LOCAL	REMAP	Remap = { IDENTITY_REMAP,
	    		 "IDENTITY_REMAP",
	     		  { "x", "y", "z" },
	     		  { "X", "Y", "Z" },
	     		  { "x", "x & y", "x, y, & z" },
			  0.0,
			  NULL
			};
LOCAL 	boolean	rot_symmetry;

/*
* 		rect_grid_corner()
*
* 	For the specified RECT_GRID, sets fcoords to the
*	floating point coordinates of the lower corner of the 
*	block with the given icoords.
*/

EXPORT  void	rect_grid_corner(
	const RECT_GRID *rect_grid,
	const int 	*icoords,
	double 		*fcoords)
{
  	int 	i, dim = rect_grid->dim;

	for (i = 0; i < dim; ++i)
	{
	    if ((icoords[i] < 0) || (icoords[i] > rect_grid->gmax[i]))
	    {
		screen("ERROR in rect_grid_corner(), "
		       "icoords not on rect_grid\n");
		print_int_vector("icoords = ",icoords,dim,"\n");
		print_RECT_GRID_structure(rect_grid);
		clean_up(ERROR);
	    }
	    fcoords[i] = cell_edge(icoords[i],i,rect_grid);
	}
}		/*end rect_grid_corner*/

EXPORT  void  	rect_grid_center(
	const RECT_GRID	*rect_grid,
	const int 	*icoords,
	double 		*fcoords)
{
  	int 	i, dim = rect_grid->dim;

	for (i = 0; i < dim; ++i)
	{
	    if ((icoords[i] < 0) || (icoords[i] >= rect_grid->gmax[i]))
	    {
		screen("ERROR in rect_grid_center(), "
		       "center for icoords not on rect_grid:\n");
		print_int_vector("icoords = ",icoords,dim,"\n");
		print_RECT_GRID_structure(rect_grid);
		clean_up(ERROR);
	    }
	    fcoords[i] = cell_center(icoords[i],i,rect_grid);
	}
}		/* end rect_grid_center */
  
EXPORT	REMAP	*remap_info(void)
{
	return &Remap;
}		/*end remap_info*/

EXPORT	void i_init_remap_and_rect_grid(
	RECT_GRID* r_grid)
{
	double		  *L = r_grid->L, *U = r_grid->U;
	int		  i, dim = r_grid->dim;
	const char	  **dnm, **Dnm, **dirs;
	static const char *plural[3] = { "", "s", "s"};

	prompt_for_remap(dim,&Remap);

	dnm = Remap.dnm;
	Dnm = Remap.Dnm;
	dirs = Remap.dirs;
	for (i = 0; i < dim; ++i)
	{
	    screen("Enter the computational limits in the "
	           "%s direction, %sL, %sU: ",dnm[i],Dnm[i],Dnm[i]);
	    (void) Scanf("%f %f\n",L+i,U+i);
	    if (U[i] <= L[i])
	    {
		screen("ERROR in i_init_remap_and_rect_grid(), "
		       "invalid computational limits L[%d] = %g, U[%d] = %g\n",
			i,L[i],i,U[i]);
		clean_up(ERROR);
	    }
	}

	screen("Enter the number%s of grid intervals of the\n\t",plural[dim-1]);
	screen("computational grid in the %s direction%s: ",dirs[dim-1],
	       plural[dim-1]);

	for (i = 0; i < dim; ++i)
	{
	    (void) Scanf("%d",&r_grid->gmax[i]);
	    r_grid->h[i] = (U[i] - L[i]) / r_grid->gmax[i];
	}
	(void) Scanf("\n"); /* Grab trailing newline */

	/* Set Default values for subdomain specifications */
	/* correct for scalar runs */
	set_rect_grid(L,U,L,U,NOBUF,NOBUF,r_grid->gmax,dim,&Remap,r_grid);
	Remap.area = r_grid->Remap.area;
}		/*end i_init_remap_and_rect_grid*/

EXPORT	void set_remap_and_rect_grid(
	double *L,
	double *U,
	int *gmax,
	GEOMETRY_REMAP geom_remap,
	RECT_GRID *r_grid)
{
	int		  i, dim = r_grid->dim;

	set_remap(dim,geom_remap,&Remap);

	for (i = 0; i < dim; ++i)
	{
	    r_grid->L[i] = L[i];
	    r_grid->U[i] = U[i];
	    r_grid->gmax[i] = gmax[i];
	    r_grid->h[i] = (U[i] - L[i]) / gmax[i];
	}

	/* Set Default values for subdomain specifications */
	/* correct for scalar runs */
	set_rect_grid(L,U,L,U,NOBUF,NOBUF,r_grid->gmax,dim,&Remap,r_grid);
	Remap.area = r_grid->Remap.area;
}		/*end i_init_remap_and_rect_grid*/

LOCAL	void	prompt_for_remap(
	int        dim,
	REMAP      *remap)
{
	Prompt_type	  *ptype;
	char		   s[Gets_BUF_SIZE];
	static Prompt_type Remaps[] = {
	    {"Identity Remap (default)","i",   1, {IDENTITY_REMAP}},
	    {"Cylindrical Geometry",    "c",   1, {CYLINDRICAL_REMAP}},
	    {"Spherical Geometry",      "sp",  2, {SPHERICAL_REMAP}},
	    {NULL,                      NULL,  0, {INVALID_REMAP}}
	};

	screen("\n");
	screen_print_long_string("Enter the remapping (Jacobian) converting "
				 "the physical coordinate system to the "
				 "cartesian computational coordinate system. "
				 "The choices are\n");

	for (ptype = Remaps; ptype->prompt != NULL; ++ptype)
	{
	    if (((dim != 1) && (ptype->type.itype == SPHERICAL_REMAP)) ||
		((dim == 3) && (ptype->type.itype == CYLINDRICAL_REMAP)))
		continue;
	    screen("\t\t%s (%s)\n",ptype->prompt,ptype->select);
	}
	screen("\t\t\tType Choice Here: ");
	(void) Gets(s);
	if (s[0] == '\0')
	    remap->remap = IDENTITY_REMAP;
	else
	{
	    remap->remap = INVALID_REMAP;
	    for (ptype = Remaps; ptype->prompt != NULL; ++ptype)
	    {
		if (((dim != 1) && (ptype->type.itype == SPHERICAL_REMAP)) ||
		    ((dim == 3) && (ptype->type.itype == CYLINDRICAL_REMAP)))
		    continue;
		if (strncasecmp(s,ptype->select,ptype->ncmp) == 0)
		{
		    switch (ptype->type.itype)
		    {
		    case IDENTITY_REMAP:
		        remap->remap = IDENTITY_REMAP;
			set_rotational_symmetry(NO);
			break;
		    case CYLINDRICAL_REMAP:
		        remap->remap = CYLINDRICAL_REMAP;
			set_rotational_symmetry(YES);
			break;
		    case SPHERICAL_REMAP:
			set_rotational_symmetry(YES);
		        remap->remap = SPHERICAL_REMAP;
			break;
		    case INVALID_REMAP:
		    default:
		        screen("ERROR in prompt_for_remap(), "
			       "invalid remap %d\n",ptype->type.itype);
			clean_up(ERROR);
			break;
		    }
		    break;
		}
	    }
	}
	set_remap(dim,remap->remap,remap);
}		/*end prompt_for_remap*/

EXPORT	void init_topological_grid(
	RECT_GRID *top_grid,
	const RECT_GRID *r_grid)
{
	char       *c;
	const char *dimname;
	static const char *blanks = " \t";
	const char *fmt1,*fmt2,*fmt3;
	char	   *message;
	double	   cor_fac;
	int	   len;
	int	   i, dim = r_grid->dim;
	int	   n_ints, n_floats, gmax[3];
	static const char *dimnames[] = {"one", "two", "three"};
	char    s[Gets_BUF_SIZE];

	dimname = dimnames[dim-1];
	copy_rect_grid(top_grid,r_grid);
	(void) adjust_top_grid_for_square(top_grid,r_grid);
	for (i = 0; i < dim; ++i)
		gmax[i] = top_grid->gmax[i];

	fmt1 =
		"The topological grid is a grid used for the construction of "
		"the tracked front topology. "
		"It is constrained to be a square grid. "
		"You specify the grid in one of two ways. "
		"If you enter a single number,  it will be used as a "
		"coarseness factor for the topological grid relative to the "
		"computational grid entered above. "
		"In this case the length of a topological grid block cell "
		"side is the nearest allowable multiple of the shortest side "
		"of the computational grid by the coarseness factor. ";
	fmt2 =
		"Otherwise the code will read the %s integers input for the "
		"number of grid cells in each coordinate direction of the "
		"topological grid. "
		"If your input values do not yield a square grid they will "
		"be corrected to produce a square grid. "
		"This correction will attempt to produce values close to those "
		"input, but if the input values are highly rectangular, "
		"the resulting values may differ considerably from those "
		"entered. ";
	fmt3 =
		"The default for this input option is the nearest "
		"square grid that matches the computational grid. "
		"Generally the topological grid is coarser than the "
		"computational grid. "
		"Larger coarseness factors yield coarser grids,  a value one "
		"gives the nearest square grid to the computational grid.\n";

	len = 500;
	uni_array(&message,len,CHAR);
	(void) sprintf(message,fmt1,dimname);
	screen_print_long_string(message);
	(void) sprintf(message,fmt2);
	screen_print_long_string(message);
	(void) sprintf(message,fmt3);
	screen_print_long_string(message);
	free(message);
	message = NULL;
	if (dim == 1)
	{
		screen_print_long_string(
			"\tBe sure to use a decimal point to "
			"indicate a floating point number "
			"if you choose to input a coarseness factor.\n");
	}
	screen("Enter your choice (cor_fac, %s integer%s, or return)\n",
		dimname,(dim > 1) ? "s" : "");
	screen("\t(defaults are");
	for (i = 0; i < dim; ++i) screen(" %d",gmax[i]);
	screen("): ");
	(void) Gets(s);
	if (s[0] != '\0')
	{
	    n_floats = sscan_float(s,&cor_fac);
	    if (dim == 1)
	    {
	        /*
	        *  To distinguish whether the input is a
	        *  coarseness factor search for a "." in s
	        */
	        if (strstr(s,".") == NULL)
	        {
	            n_floats = 0;
	            cor_fac = ERROR_FLOAT;
	        }
	    }
	    for (n_ints = 0, c = strtok(s,blanks); c != NULL;
	                     c = strtok(NULL,blanks), ++n_ints)
	    {
	        (void) sscanf(c,"%d",gmax+n_ints%dim);
	    }
	    if (n_ints == 2*dim)
	    {
	        /* There is a secret undocumemted option here.
	        *  Enter the topological grid sizes twice and the
	        *  code will use these values whether they give a square grid
	        *  or not.  This makes nearest_interface_point()
	        *  invalid,  but you can get what you ask for.
	        */
	        set_rect_grid(top_grid->L,top_grid->U,top_grid->L,top_grid->U,
	                      NOBUF,NOBUF,gmax,dim,&r_grid->Remap,top_grid);
	    }
	    else if ((n_ints == 1) && (n_floats > 0))
	    {
	        int imin;
	        imin = 0;
	        for (i = 1; i < dim; ++i)
	            if (r_grid->h[i] < r_grid->h[imin]) imin = i;

	        top_grid->gmax[imin] = irint(r_grid->gmax[imin]/cor_fac);
	        if (top_grid->gmax[imin] <= 0) top_grid->gmax[imin] = 1;
	        top_grid->h[imin] = (top_grid->U[imin] - top_grid->L[imin]) /
	                    top_grid->gmax[imin];
	        for (i = 0; i < dim; ++i)
	        {
	            double   tmp_gmax;
	            if (i == imin) continue;
	            tmp_gmax = (top_grid->U[i] - top_grid->L[i]) /
	                    top_grid->h[imin];
	            top_grid->gmax[i] = (int)(tmp_gmax);
	        }
	        (void) adjust_top_grid_for_square(top_grid,r_grid);
	        set_rect_grid(top_grid->L,top_grid->U,top_grid->L,top_grid->U,
	            NOBUF,NOBUF,top_grid->gmax,dim,&r_grid->Remap,top_grid);
	    }
	    else if (n_ints == dim)
	    {
	        for (i = 0; i < dim; ++i)
	            top_grid->gmax[i] = gmax[i];
	        (void) adjust_top_grid_for_square(top_grid,r_grid);
	        set_rect_grid(top_grid->L,top_grid->U,top_grid->L,top_grid->U,
	            NOBUF,NOBUF,top_grid->gmax,dim,&r_grid->Remap,top_grid);
	    }
	    else
	    {
	        screen("ERROR in init_topological_grid(), "
	               "invalid input of topogical grid mesh\n");
	        clean_up(ERROR);
	    }
	}
	screen("The topological mesh used is ");
	for (i = 0; i < dim; ++i) screen(" %d",top_grid->gmax[i]);
	screen("\n\n");
}		/*end init_topological_grid*/

EXPORT	GEOMETRY_REMAP	read_remap_from_string(
	const char	*s)
{
	if (strstr(s,"IDENTITY") != NULL)
	    return IDENTITY_REMAP;
	else if (strstr(s,"CYLINDRICAL") != NULL)
	    return CYLINDRICAL_REMAP;
	else if (strstr(s,"SPHERICAL") != NULL)
	    return SPHERICAL_REMAP;
	else
	    return INVALID_REMAP;
}		/*end read_remap_from_string*/

EXPORT  void    set_remap_identity(
	int             dim,
	GEOMETRY_REMAP  remap)
{
	set_remap(dim,remap,&Remap);
}		/*end set_remap_identity*/

EXPORT	void	set_remap(
	int	        dim,
	GEOMETRY_REMAP	remap_type,
	REMAP           *remap)
{
	remap->remap = remap_type;
	switch (remap_type) 
	{
	case IDENTITY_REMAP:
	    remap->remap_name = "IDENTITY_REMAP";
	    remap->dnm[0] = "x";
	    remap->dnm[1] = "y";
	    remap->dnm[2] = "z";
	    remap->Dnm[0] = "X";
	    remap->Dnm[1] = "Y";
	    remap->Dnm[2] = "Z";
	    remap->dirs[0] = "x";
	    remap->dirs[1] = "x & y";
	    remap->dirs[2] = "x, y, & z";
	    switch (dim)
	    {
	    case 1:
	    	remap->Area = ident_Length;
	    	break;
	    case 2:
	    	remap->Area = ident_Area;
	    	break;
	    case 3:
	    	remap->Area = ident_Volume;
	    	break;
	    }
	    break;
	case CYLINDRICAL_REMAP:
	    remap->remap_name = "CYLINDRICAL_REMAP";
	    remap->dnm[0] = "radial";
	    remap->dnm[1] = "vertical";
	    remap->dnm[2] = "angular";
	    remap->Dnm[0] = "R";
	    remap->Dnm[1] = "Z";
	    remap->Dnm[2] = "theta";
	    remap->dirs[0] = "r";
	    remap->dirs[1] = "r & z";
	    remap->dirs[2] = "r, z, & theta";
	    switch (dim)
	    {
	    case 1:
	    	remap->Area = cylin_Area;
	    	break;
	    case 2:
	    	remap->Area = cylin_Volume;
	    	break;
	    }
	    break;
	case SPHERICAL_REMAP:
	    remap->remap_name = "SPHERICAL_REMAP";
	    remap->dnm[0] = "radial";
	    remap->dnm[1] = "longitudinal";
	    remap->dnm[2] = "latitudinal";
	    remap->Dnm[0] = "R";
	    remap->Dnm[1] = "theta";
	    remap->Dnm[2] = "phi";
	    remap->dirs[0] = "r";
	    remap->dirs[1] = "r & theta";
	    remap->dirs[2] = "r, theta, & phi";
	    remap->Area = spherical_Volume;
	    break;
	default:
	    remap->remap_name = "INVALID_REMAP";
	    screen("ERROR in set_remap(), "
	           "illegal or unavailable geometry\n");
	    clean_up(ERROR);
	}
}		/*end set_remap*/

EXPORT	void i_print_remap_values(void)
{
	(void) printf("\t[IDENTITY %d ",IDENTITY_REMAP);
	(void) printf("CYLINDRICAL %d",CYLINDRICAL_REMAP);
	(void) printf("SPHERICAL %d]\n",SPHERICAL_REMAP);
}		/*end i_print_remap_values*/


EXPORT void set_rect_grid(
	const double *L,
	const double *U,
	const double *GL,
	const double *GU,
	const int   *lbuf,
	const int   *ubuf,
	const int   *gmax,
	int	    dim,
	const REMAP *remap,
	RECT_GRID *rect_grid)
{
	int i;
	int nobuf[3] = {0, 0, 0};

	if (rect_grid == NULL)
	    return;

	rect_grid->Remap = *remap;
	rect_grid->dim = dim;
	if (lbuf == NULL)
	    lbuf = nobuf;
	if (ubuf == NULL)
	    ubuf = nobuf;
	for (i = 0; i < dim; ++i)
	{
	    rect_grid->L[i] = L[i];
	    rect_grid->U[i] = U[i];
	    rect_grid->GL[i] = GL[i];
	    rect_grid->GU[i] = GU[i];
	    rect_grid->lbuf[i] = lbuf[i];
	    rect_grid->ubuf[i] = ubuf[i];
	    rect_grid->gmax[i] = gmax[i];
	    rect_grid->h[i] = (gmax[i] > 0) ? (U[i] - L[i])/gmax[i] : 0;
	    rect_grid->VL[i] = rect_grid->L[i] - lbuf[i]*rect_grid->h[i];
	    rect_grid->VU[i] = rect_grid->U[i] + ubuf[i]*rect_grid->h[i];
	    rect_grid->edges[i] = NULL;
	    rect_grid->centers[i] = NULL;
	    rect_grid->dh[i] = NULL;
	    rect_grid->variable_mesh[i] = NO;
	}
	rect_grid->glstore = NULL;
	rect_grid->Remap.area = area_of_rect_grid(rect_grid);
}		/*end set_rect_grid*/


LOCAL	double area_of_rect_grid(
	const RECT_GRID *rect_grid)
{
	double       area;
	const double *L = rect_grid->L, *U = rect_grid->U;
	int         i, dim = rect_grid->dim;

	area = -HUGE_VAL;
	switch (rect_grid->Remap.remap) 
	{
	case IDENTITY_REMAP:
	    area = U[0] - L[0];
	    for (i = 1; i < dim; ++i)
	    	area *= U[i] - L[i];
	    break;
	case CYLINDRICAL_REMAP:
	    if (L[0] < 0.0)
	        area = PI*(U[0]*U[0] + L[0]*L[0]);
	    else
	        area = PI*(U[0]+L[0])*(U[0]-L[0]);
	    for (i = 1; i < dim; ++i)
	    	area *= U[i] - L[i];
	    break;
	case SPHERICAL_REMAP:
	    area = 4.0/3.0*PI*(U[0]*U[0]*U[0] - L[0]*L[0]*L[0]);
	    break;
	default:
	    screen("ERROR in area_of_rect_grid(), "
	           "illegal or unavailable geometry\n");
	    clean_up(ERROR);
	}
	if (area <= 0.0)
	{
	    screen("ERROR in area_of_rect_grid(), "
	           "Nonpositive computational area\n");
	    (void) printf("dim = %d, remap = %d, area = %g\n",
	                  dim,rect_grid->Remap.remap,area);
	    if (1 <= dim && dim <= 3)
	    {
	        print_general_vector("L = ",L,dim,", ");
	        print_general_vector("U = ",U,dim,"\n");
	    }
	    else
	        (void) printf("Invalid dimension %d\n",dim);
	    clean_up(ERROR);
	}
	return area;
}		/*end area_of_rect_grid*/


EXPORT	void	set_dual_grid(
	RECT_GRID       *dual_gr,
	const RECT_GRID *gr)
{
	int		dlbuf[MAXD], dubuf[MAXD];
	int		i;

	if (dual_gr == NULL || gr == NULL)
	    return;
	dual_gr->dim = gr->dim;
	for (i = 0; i < gr->dim; ++i)
	{
	    dlbuf[i] = gr->lbuf[i] - 1;
	    if (dlbuf[i] < 0)
	        dlbuf[i] = 0;
	    dubuf[i] = gr->ubuf[i] - 1;
	    if (dubuf[i] < 0)
	        dubuf[i] = 0;
	    dual_gr->gmax[i] = gr->gmax[i] + 1;
	    dual_gr->L[i] = gr->L[i] - 0.5*gr->h[i];
	    dual_gr->U[i] = gr->U[i] + 0.5*gr->h[i];
	    /*This is wrong */
	    dual_gr->GL[i] = gr->GL[i] - 0.5*gr->h[i];
	    dual_gr->GU[i] = gr->GU[i] + 0.5*gr->h[i];
	}
	set_rect_grid(dual_gr->L,dual_gr->U,gr->GL,gr->GU,dlbuf,dubuf,
		      dual_gr->gmax,gr->dim,&gr->Remap,dual_gr);
	for (i = 0; i < gr->dim; ++i)
	    dual_gr->h[i] = gr->h[i];
}		/*end set_dual_grid*/

/*
*			copy_rect_grid():
*
*	Copy the RECT_GRID structure data_gr into the rect_grid copy_gr.
*/

EXPORT	void	copy_rect_grid(
	RECT_GRID	*copy_gr,
	const RECT_GRID	*data_gr)
{
	int		i, dim;

	debug_print("cp_rect_grid","Entered copy_rect_grid()\n");

	if (copy_gr == NULL || data_gr == NULL)
	{
	    debug_print("cp_rect_grid","Left copy_rect_grid(), Null grid\n");
	    return;
	}
	dim = data_gr->dim;
	*copy_gr = *data_gr;
	for (i = 0; i < dim; ++i)
	{
	    copy_gr->edges[i] = NULL;
	    copy_gr->centers[i] = NULL;
	    copy_gr->dh[i] = NULL;
	    copy_gr->variable_mesh[i] = NO;
	}
	copy_gr->glstore = NULL;
	debug_print("cp_rect_grid","Left copy_rect_grid()\n");
}		/*end copy_rect_grid*/


/*
*		adjust_top_grid_for_square():
*
*	Finds a square grid as close as possible to an input
*	topological grid.  Return YES if the input values
*	are modified,  NO otherwise.
*/


EXPORT	boolean	adjust_top_grid_for_square(
	RECT_GRID	*top_grid,
	const RECT_GRID	*r_grid)
{
	int		   dim;
	int		   n[MAXD];
	int		   i, j, k;
	boolean		   square_grid;
	double		   h[MAXD];
	double		   X[MAXD];
	double		   sqr_tol;
	static const double SQR_TOL  = 0.001; /* TOLERANCE */
	static const int   NUM_ITER = 20;	 /* TOLERANCE */

	if (top_grid == NULL || r_grid == NULL)
	{
	    (void) printf("WARNING in adjust_top_grid_for_square(), ");
	    if (top_grid == NULL && r_grid == NULL)
	    	(void) printf("top_grid = NULL, and r_grid = NULL\n");
	    else if (top_grid == NULL)
	    	(void) printf("top_grid = NULL\n");
	    else
	    	(void) printf("r_grid = NULL\n");
	    return NO;
	}
	dim = top_grid->dim;
	sqr_tol = SQR_TOL;
	for (i = 0; i < dim; ++i)
	    sqr_tol = min(sqr_tol,0.5*r_grid->h[i]);

	for (i = 0; i < dim; ++i)
	{
	    X[i] = top_grid->U[i] - top_grid->L[i];
	    n[i] = top_grid->gmax[i];
	    h[i] = X[i]/n[i];
	}

	square_grid = YES;
	for (i = 0; i < dim-1; ++i)
	{
	    for (j = i+1; j < dim; ++j)
	    {
	    	if (fabs(h[i]/h[j] - 1.0) > sqr_tol)
	    	{
	    	    square_grid = NO;
	    	    break;
	    	}
	    }
	    if (square_grid == NO)
	        break;
	}

	if (square_grid == NO)
	{
	    for (i = 0; i < dim-1; ++i)
	    {
	    	h[i] = X[i]/n[i];
	    	for (j = i+1; j < dim; ++j)
	    	{
	    	    h[j] = X[j]/n[j];
	    	    while (h[i]/h[j] > 1.0)
	    	    {
	    	    	if (fabs(h[i]/h[j] - 1.0) < sqr_tol)
	    	    		break;
	    	    	if (n[j] == 1)
			    break;
	    	    	n[j]--;
	    	    	h[j] = X[j]/n[j];
	    	    }
	    	    while (h[i]/h[j] < 1.0)
	    	    {
	    	    	if (fabs(h[i]/h[j] - 1.0) < sqr_tol)
	    	    	    break;
	    	    	if (n[i] == 1)
			    break;
	    	    	n[i]--;
	    	    	h[i] = X[i]/n[i];
	    	    }
	    	}
	    }

	    for (k = 0; k < NUM_ITER; ++k)
	    {
	    	for (i = 0; i < dim-1; ++i)
	    	{
	    	    h[i] = X[i]/n[i];
	    	    for (j = i+1; j < dim; ++j)
	    	    {
	    	    	h[j] = X[j]/n[j];
	    	    	while (h[i]/h[j] < 1.0)
	    	    	{
	    	    	    if (fabs(h[i]/h[j] - 1.0) < sqr_tol)
	    	    		break;
	    	    	    ++n[j];
	    	    	    h[j] = X[j]/n[j];
	    	    	}
	    	    	while (h[i]/h[j] > 1.0)
	    	    	{
	    	    	    if (fabs(h[i]/h[j] - 1.0) < sqr_tol)
	    	    	    	break;
	    	    	    ++n[i];
	    	    	    h[i] = X[i]/n[i];
	    	    	}
	    	    }
	    	}
	    }
	}
	set_rect_grid(top_grid->L,top_grid->U,top_grid->GL,top_grid->GU,
		      top_grid->lbuf,top_grid->ubuf,n,top_grid->dim,
		      &r_grid->Remap,top_grid);
	return (square_grid == YES) ? NO : YES;
}		/*end adjust_top_grid_for_square*/


/*
*			set_grid_lines():
*
*	Sets up the grid line portion of the regular rectangular grid structure.
*	Returns 0 if out of space, and 1 otherwise.
*
*	TODO:
*		Could upgrade to an irregular grid (with the restriction
*		that every midpoint of the underlying regular computational
*		hyp grid lie at a the crossing point of two grid lines).
*		This would help for the case of more than two interface
*		nodes lying in a mesh block of the underlying computational
*		hyp grid.
*		Note that the function flux() requires changing in this case
*		(uses constant grid spacing).
*/

EXPORT	int set_grid_lines(
	RECT_GRID	*rgr)
{
	register int	j, jmin, jmax;
	register double	*edges, *centers, *dh, h, L;

	double	*store;
	int 	i, dim = rgr->dim;
	int	lbuf[MAXD], ubuf[MAXD];
	int	len[MAXD], tlen;
	static const	int	EXTRA_BUF = 2;

	if (rgr == NULL)
	{ 
	    return NO;
	}

	tlen = 0;
	for (i = 0; i < dim; ++i)
	{
	    lbuf[i] = rgr->lbuf[i] + EXTRA_BUF;
	    ubuf[i] = rgr->ubuf[i] + EXTRA_BUF;
	    len[i] = (lbuf[i] + rgr->gmax[i] + ubuf[i]);
	    tlen += 3*len[i]+1;
	}

	uni_array(&rgr->glstore,tlen,FLOAT);
	if (rgr->glstore == NULL)
	{
	    return NO;
	}

	store = rgr->glstore;
	for (i = 0; i < dim; ++i)
	{
	    rgr->edges[i] = store + lbuf[i];	store += len[i]+1;
	    rgr->centers[i] = store + lbuf[i];	store += len[i];
	    rgr->dh[i] = store + lbuf[i];	store += len[i];

	    jmin = -lbuf[i];	jmax = rgr->gmax[i] + ubuf[i];
	    L = rgr->L[i];		h = rgr->h[i];
	    edges = rgr->edges[i] + jmin;
	    centers = rgr->centers[i] + jmin;
	    dh = rgr->dh[i] + jmin;
	    for (j = jmin;  j < jmax;  ++j)
	    {
	    	*edges = L + j*h;
	    	*centers++ = *edges++ + 0.5*h;
	    	*dh++ = h;
	    }
	    *edges = L + jmax*h;
	}
	return YES;
}		/*end set_grid_lines*/



/*
*			free_grid_lines():
*
*	Frees the storage that has been allocated during the
*	a call to set_grid_lines.
*/

EXPORT	void free_grid_lines(
	RECT_GRID	*rgr)
{
	int		i, dim;

	if (rgr == NULL)
	{ 
	    return;
	}
	dim = rgr->dim;
	if (rgr->glstore != NULL)
	    free(rgr->glstore);
	rgr->glstore = NULL;

	for (i = 0; i < dim; ++i)
	{
	    rgr->edges[i] = NULL;
	    rgr->centers[i] = NULL;
	    rgr->dh[i] = NULL;
	}
}		/*end free_grid_lines*/


/*
*			print_RECT_GRID_structure():
*/

EXPORT	void print_RECT_GRID_structure(
	const RECT_GRID	*rect_grid)
{
	int		i, dim;

	(void) printf("\n\t\tRECT_GRID %p structure\n",(POINTER)rect_grid);

	if (rect_grid == NULL)
	{
	    (void) printf("\t\tNULL RECT_GRID\n");
	    return;
	}
	dim = rect_grid->dim;
	(void) printf("rect_grid->dim = %d\n",dim);
	(void) printf("\n%3s %11s %11s %11s %4s\n",
		      "dir","L","U","h","gmax");
	for (i = 0; i < dim; ++i)
	{
	    (void) printf(" %2d %11g %11g %11g %4d\n",
			  i,rect_grid->L[i],rect_grid->U[i],rect_grid->h[i],
			  rect_grid->gmax[i]);
	}
	(void) printf("\n%3s %11s %11s %11s %11s %4s %4s\n",
		      "dir","GL","GU","VL","VU","lbuf","ubuf");
	for (i = 0; i < dim; ++i)
	{
	    (void) printf(" %2d %11g %11g %11g %11g %4d %4d\n",
			  i,rect_grid->GL[i],rect_grid->GU[i],
			  rect_grid->VL[i],rect_grid->VU[i],
			  rect_grid->lbuf[i],rect_grid->ubuf[i]);
	}
	(void) printf("\n\t\tEnd RECT_GRID %p structure\n\n",
		      (POINTER) rect_grid);
}		/*end print_RECT_GRID_structure*/



#define PRINT_GRID_DIMENSION \
	"                    Grid Dimension = %-13d\n"

EXPORT void print_rectangular_grid(
	const RECT_GRID	*grid)
{
	fprint_rectangular_grid(stdout,grid);
}		/*end print_rectantular_grid*/

EXPORT void fprint_rectangular_grid(
	FILE		*file,
	const RECT_GRID	*grid)
{
	const char* const* Dnm = grid->Remap.Dnm;
	const char* const* dnm = grid->Remap.dnm;
	boolean		b_oput = is_binary_output();
	int		i;
	int		dim;

	if (grid == NULL)
	    return;
	dim = grid->dim;
	(void) fprintf(file,PRINT_GRID_DIMENSION,grid->dim);
	if (b_oput == YES)
	{
	    (void) fprintf(file,"\f%c",0);
	    (void) fprintf(file,"MAXD = %d FLOAT = %d\n",MAXD,(int)FLOAT);
	}
	/* Print grid widths */
	for (i = 0; i < dim; ++i)
	{
	    (void) fprintf(file,"%10s = %-"FFMT"%s",
			        Dnm[i],grid->U[i]-grid->L[i],
			        (i==dim-1) ? "\n" : "              ");
	}

#define print_grid_float(x)						\
	(b_oput == YES) ? (void) fwrite((const void *)&(x),FLOAT,1,file) :\
		   (void) fprintf(file,"%-"FFMT,(x))

	/* Print grid endpoints */
	for (i = 0; i < dim; ++i)
	{
	    (void) fprintf(file,"   %sL = ",Dnm[i]);
	    print_grid_float(grid->L[i]);
	    (void) fprintf(file,"    %sU = ",Dnm[i]);
	    print_grid_float(grid->U[i]);
	    (void) fprintf(file,"%s",(i==dim-1)?"\n":"    ");
	}
	/* Print grid spacings */
	for (i = 0; i < dim; ++i)
	{
	    (void) fprintf(file,"   h%s = ",dnm[i]);
	    print_grid_float(grid->h[i]);
	    (void) fprintf(file,"  %smax = %-10d%s",dnm[i],grid->gmax[i],
	    	                (i==dim-1)?"\n":"    ");
	}
	/* Print global grid endpoints */
	for (i = 0; i < dim; ++i)
	{
	    (void) fprintf(file,"  G%sL = ",Dnm[i]);
	    print_grid_float(grid->GL[i]);
	    (void) fprintf(file,"   G%sU = ",Dnm[i]);
	    print_grid_float(grid->GU[i]);
	    (void) fprintf(file,"%s",(i==dim-1)?"\n":"    ");
	}
	/* Print virtual domain grid endpoints */
	for (i = 0; i < dim; ++i)
	{
	    (void) fprintf(file,"  V%sL = ",Dnm[i]);
	    print_grid_float(grid->VL[i]);
	    (void) fprintf(file,"   V%sU = ",Dnm[i]);
	    print_grid_float(grid->VU[i]);
	    (void) fprintf(file,"%s",(i==dim-1)?"\n":"    ");
	}
	/* Print buffer zone widths in grid units */
	for (i = 0; i < dim; ++i)
	{
	    (void) fprintf(file,"%slbuf = %-10d %subuf = %-10d%s",
	    	                dnm[i],grid->lbuf[i],dnm[i],grid->ubuf[i],
			        (i==dim-1)?"\n":"    ");
	}

	/*TODO Provide printing of variable grid spacings */

#undef print_grid_float
}		/*end fprint_rectangular_grid*/


EXPORT void read_rectangular_grid(
	const IO_TYPE *io_type,
	RECT_GRID     *gr,
	boolean	      bufzones,
	REMAP         *remap)
{
	FILE *file = io_type->file;
	char Line[2048];
	char ss[120];
	long offset;
	int  i, c;
	int  dim;
	int  maxd = MAXD, size_float = FLOAT;
	boolean b_iput;

	if (gr == NULL)
	{
	    (void) printf("WARNING in read_rectangular_grid(), grid is null\n");
	    return;
	}
	zero_scalar(gr,sizeof(RECT_GRID));
	gr->Remap = *remap;

	if (fgetstring(file,"Grid Dimension = ") == FUNCTION_FAILED)
	{
	    gr->dim = 2;
	}
	else
	{
	    (void) fscanf(file,"%d",&dim);
	    while ((c = getc(file)) != '\n');
	    if ((c = getc(file)) == '\f') /* Binary input */
	    {
	        c = getc(file);
	        if (c == 1) /*oldstyle printout*/
	        {
		    (void) printf("WARNING in read_rectangular_grid(), "
		                  "old style binary IO only valid for\n"
				  "reading from runs with same floating "
				  "point precision and endian as output\n");
	            (void) fread((void *)gr,sizeof(RECT_GRID),1,file);
		    return;
		}
		if (c != 0)
		{
		    screen("ERROR in read_rectangular_grid(), "
		           "Improper output format\n");
		    clean_up(ERROR);
		    return;
		}
		b_iput = YES;
		(void) fscanf(file,"%*s%*s%d%*s%*s%d",&maxd,&size_float);
		(void) getc(file); /* newline */
	    }
	    else
	    {
	        b_iput = NO;
		(void) ungetc(c,file);
	    }
	}
	gr->dim = dim;
	(void) sprintf(ss,"%10s = ",remap->Dnm[0]);
	if (fgetstring(file,ss) == FUNCTION_FAILED)
	{
	    (void) printf("WARNING in read_rectangular_grid(), "
			  "grid not found\n");
	    return;
	}
	(void) fgets(Line,2046,file);		/* clear end of X,Y line */

#define read_grid_float(x)						\
	if (b_iput)							\
	{								\
	    (void) getc(file); /* get blank */				\
	    (void) read_binary_real_array(x,1,io_type);			\
	}								\
	else								\
	    (void) fscan_float(file,(x))

	/* Read grid endpoints */
	for (i = 0; i < dim; ++i)
	{
	    (void) fscanf(file,"%*s%*s");
	    read_grid_float(gr->L+i);
	    (void) fscanf(file,"%*s%*s");
	    read_grid_float(gr->U+i);
	}
	/* Read grid spacings */
	for (i = 0; i < dim; ++i)
	{
	    (void) fscanf(file,"%*s%*s");
	    read_grid_float(gr->h+i);
	    (void) fscanf(file,"%*s%*s%d",gr->gmax+i);
	}

	offset = ftell(file);
	(void) sprintf(ss,"  G%sL = ",remap->Dnm[0]);
	if (fgetstring(file,ss) == FUNCTION_FAILED)
	{
	    for (i = 0; i < dim; ++i)
	    {
	    	if (bufzones == YES)
	    	{
	    	    gr->lbuf[i] = gr->ubuf[i] = 1;
	            gr->GL[i] = gr->L[i];
		    gr->GU[i] = gr->U[i];
		    gr->VL[i] = gr->L[i] - cell_width(0,i,gr);
		    gr->VU[i] = gr->U[i] + cell_width(gr->gmax[i]-1,i,gr);
		}
		else
		{
		    gr->lbuf[i] = gr->ubuf[i] = 0;
		    gr->GL[i] = gr->L[i];
		    gr->GU[i] = gr->U[i];
		    gr->VL[i] = gr->L[i];
		    gr->VU[i] = gr->U[i];
		}
	    }
	    return;
	}
	(void) fseek(file,offset,SEEK_SET);
	/* Read global grid endpoints */
	for (i = 0; i < dim; ++i)
	{
	    (void) fscanf(file,"%*s%*s");
	    read_grid_float(gr->GL+i);
	    (void) fscanf(file,"%*s%*s");
	    read_grid_float(gr->GU+i);
	}
	/* Read virtual domain endpoints */
	for (i = 0; i < dim; ++i)
	{
	    (void) fscanf(file,"%*s%*s");
	    read_grid_float(gr->VL+i);
	    (void) fscanf(file,"%*s%*s");
	    read_grid_float(gr->VU+i);
	}
	/* Read buffer zone widths */
	for (i = 0; i < dim; ++i)
	    (void) fscanf(file,"%*s%*s%d%*s%*s%d",gr->lbuf+i,gr->ubuf+i);

	set_rect_grid(gr->L,gr->U,gr->GL,gr->GU,gr->lbuf,gr->ubuf,gr->gmax,
		      dim,&gr->Remap,gr);

#undef read_grid_float
	return;
}		/*end read_rectangular_grid*/


/*
*			rect_in_which():
*
*	Determines the grid block icoords = ix,iy,iz of a rectangular grid in
*	which a point coords = x,y,z lies.   Points that are just outside the
*	grid are moved in first.
*	Returns 1 if successful or 0 if x,y,z lies substantially outside
*	the grid (in that case the returned ix, iy, iz are still correct).
*
*	For a 2D interface with 3D compilation, icoords[2] = iz = 0 and it is
*	assumed that icoords points to 3 ints of storage.
*/


EXPORT boolean rect_in_which(
	const double     *coords,
	int	        *icoords,
	const RECT_GRID	*grid)
{
	boolean	    status = FUNCTION_SUCCEEDED;
	const double *h = grid->h;
	const double *VL = grid->VL, *VU = grid->VU;
	const int   *gmax = grid->gmax;
	const int   *lbuf = grid->lbuf, *ubuf = grid->ubuf;
	int	    i, dim = grid->dim;
	static const double SHIFT = 0.2; /* TOLERANCE */

		/* Find Grid Block and points outside and moved in */

	for(i = 0; i < dim; ++i)
	{
	    if (grid->h[i] == 0.0)
	    {
	    	icoords[i] = 0;
	    }
	    else
	    {
	        icoords[i] = cell_index(coords[i],i,grid);

	        if (icoords[i] < -lbuf[i])
	        {
	    	    if (VL[i] - coords[i] <= SHIFT*h[i])
	    	        icoords[i] = -lbuf[i];
	    	    else
	    	        status = FUNCTION_FAILED;
	        }
	        if (icoords[i] >= gmax[i]+ubuf[i])
	        {
	    	    if (coords[i] - VU[i] <= SHIFT*h[i])
	    	        icoords[i] = gmax[i] + ubuf[i] - 1;
	    	    else
	    	        status = FUNCTION_FAILED;
	        }
	    }
	}
	return status;
}		/*end rect_in_which*/

/*	
*			point_in_buffer():
*
*	Is the point in the buffer zone?  Return YES if so, NO otherwise
*
*/

EXPORT boolean point_in_buffer(
	const double     *posn,
	const RECT_GRID *rgr)
{
	const double *VL = rgr->VL, *VU = rgr->VU;
	const double *L = rgr->L, *U = rgr->U;
	int	    j, dim = rgr->dim;

	for (j = 0; j < dim; ++j)
	{
	    if ((posn[j] - VL[j])*(posn[j] - L[j]) < 0.0 ||
		(posn[j] - VU[j])*(posn[j] - U[j]) < 0.0)
	    {
		return YES;
	    }
	}

	return NO;
}		/*end point_in_buffer*/

/*
*			zoom_rect_grid():
*
*	Projects a rect grid gr1 onto a rect_grid gr2.  The resultant
*	grid gr1 has limits given by gr2 and grid size given by the
*	original gr1.
*/

EXPORT	void zoom_rect_grid(
	RECT_GRID       *gr1,
	const RECT_GRID *gr2)
{
	double		h[MAXD];
	double		L[MAXD], U[MAXD];
	int		gmax[MAXD];
	int		dim, i;

	dim = gr1->dim;
	for (i = 0; i < dim; ++i)
	{
	    L[i] = gr2->L[i];
	    U[i] = gr2->U[i];
	    h[i] = gr1->h[i];
	    gmax[i] = irint((U[i] - L[i])/h[i]);
	}
	set_rect_grid(L,U,gr2->GL,gr2->GU,gr2->lbuf,gr2->ubuf,gmax,dim,
		      &gr2->Remap,gr1);
}		/*end zoom_rect_grid*/

EXPORT	double	grid_tolerance(
	const RECT_GRID	*gr)
{
	double		min_h;
	int		i;

	min_h = gr->h[0];
	for (i = 1; i < gr->dim; ++i)
	    if (gr->h[i] < min_h)
		min_h = gr->h[i];

	return 0.0005*min_h; /* TOLERANCE */
}		/*end grid_tolerance*/

EXPORT	void set_rotational_symmetry(boolean yes_no)
{
	rot_symmetry = yes_no;
}	/* end set_rotational_symmetry */

EXPORT	boolean is_rotational_symmetry(void)
{
	return rot_symmetry;
}	/* end is_rotational_symmetry */

/* ARGSUSED */
LOCAL	double ident_Length(
	const double	*coords,
	const RECT_GRID	*rgr)
{
	return rgr->h[0];
}		/*end ident_Length*/


/* ARGSUSED */
EXPORT	double ident_Area(
	const double	*coords,
	const RECT_GRID	*rgr)
{
	const double *h = rgr->h;
	return h[0]*h[1];
}		/*end ident_Area*/


/* ARGSUSED */
LOCAL	double ident_Volume(
	const double	*coords,
	const RECT_GRID	*rgr)
{
	const double *h = rgr->h;
	return  h[0]*h[1]*h[2];
}		/*end ident_Volume*/


LOCAL	double cylin_Volume(
	const double     *coords,
	const RECT_GRID *rgr)
{
	const double *h = rgr->h;
	double       r = coords[0];
	
	if (r < 0.0)
	{
	    r = fabs(r + h[0]);
	    return PI*r*r*h[1];
	}
	return  2.0*PI*r*h[0]*h[1];
}		/*end cylin_Volume*/

LOCAL	double cylin_Area(
	const double     *coords,
	const RECT_GRID	*rgr)
{
	double	dr = rgr->h[0];
	double   r = coords[0];
	
	if (r < 0.0)
	{
	    r = fabs(r + dr);
	    return PI*r*r;
	}
	return  2.0*PI*r*dr;
}		/*end cylin_Area*/

LOCAL	double spherical_Volume(
	const double	*coords,
	const RECT_GRID	*rgr)
{
	double	dr = rgr->h[0];
	double   r = coords[0];
	
	if (r < 0.0)
	{
	    r = fabs(r + dr);
	    return (4.0/3.0)*PI*r*r*r;
	}
	return  4.0*PI*dr*(r*r + dr*dr/12.0);
}		/*end sperical_Volume*/

