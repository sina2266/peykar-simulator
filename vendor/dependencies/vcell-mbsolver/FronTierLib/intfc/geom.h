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
*			geom.h
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*/

#if !defined(_IGEOM_H)
#define _IGEOM_H

#include <util/cdecs.h>
#include <util/vmalloc.h>
#include <intfc/triangledefs.h>

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif


/* The default is to compile all dimensional versions of the code */
#if !defined(ONED) && !defined(TWOD) && !defined(THREED)
#   define ONED
#   define TWOD
#   define THREED
#endif /* !defined(ONED) && !defined(TWOD) && !defined(THREED) */

enum {
#if defined(THREED)
	MAXD = 3
#elif defined(TWOD)
	MAXD = 2
#elif defined(ONED)
	MAXD = 1
#endif /* defined(THREED) */
};

enum {
#if defined(THREED)
	MAX_LSQ_PTS = 100
#elif defined(TWOD)
	MAX_LSQ_PTS = 40
#elif defined(ONED)
	MAX_LSQ_PTS = 5
#endif /* defined(THREED) */
};

#define	NOBUF	NULL

	/* Possible values for the variable remap */
typedef	enum {
	INVALID_REMAP     = 0,
	IDENTITY_REMAP    = 1,	/* identity conformal map */
	CYLINDRICAL_REMAP = 2,	/* cylindrical geometry */
	SPHERICAL_REMAP   = 3	/* spherical geometry (oned) */
} GEOMETRY_REMAP;

typedef	enum {
	COMP_GRID = 1,
	DUAL_GRID = 2,
	EXPANDED_DUAL_GRID = 3
} GRID_TYPE;

#define d_index1d(ix,gmax)       \
        (ix)

#define d_index2d(ix,iy,gmax)                                        \
        ((iy)*((gmax)[0]+1)+(ix))

#define d_index3d(ix,iy,iz,gmax)                                        \
        (((iz)*((gmax)[1]+1)+(iy))*((gmax)[0]+1)+(ix))

#define d_index(icoords,gmax,dim)                               \
        ((dim) == 1 ? d_index1d((icoords[0]),(gmax)) :          \
         (dim) == 2 ? d_index2d((icoords[0]),(icoords[1]),(gmax)) :     \
         d_index3d((icoords[0]),(icoords[1]),(icoords[2]),(gmax)))	


		/* Structure Defining a Regular Rectangular Grid: */
/*
*	Support has been added to support virtual domain buffer zones
*	as well as possibly nonconstant grid spacings.  The storage
*	for the grid lines and spacings is taken from the single
*	array glstore.  The allocated size of glstore should be
*
*				dim-1
*				-----
*				\
*				 \
*	size_of(glstore) =        \	(3*(gmax[i]+ub[i]+lb[i]) + 1)
*				  /
*				 /
*				/
*				-----
*				i = 0
*
*	Which is just the combined length of the edge and dl uni_arrays.
*	This storage should be mapped out as follows
*	
*	edge[0] = glstore + lb[0]
*	edge[i]   = edge[i-1]   + gmax[i-1]   + ub[i-1]   + 1 + lb[i]	(i > 0)
*	dl[0]     = edge[dim-1] + gmax[dim-1] + ub[dim-1] + 1 + lb[0]
*	dl[i]     = dl[i-1]     + gmax[i-1]   + ub[i-1]       + lb[i]	(i > 0)
*	center[0] = dl[dim-1]   + gmax[dim-1] + ub[dim-1]     + lb[0]
*	center[i] = center[i-1] + gmax[i-1]   + ub[i-1]       + lb[i]	(i > 0)
*
*	This arrangement has the following benefits,
*
*	1. Storage need only be allocated once for grid line storage
*	2. The addresses edge[i] - glstore,  center[i] - slstore,
*	   and dl[i] - glstore are absolute numbers independent of processor.
*	   This is exploited in the pointer reconstruction for the topological
*	   grid that is transferred across multiple processors with the
*	   interface.
*
*	   Note that in the the case of the topological grid,  consistency
*	   requires that glstore be allocated by store().
*
*	All allocation of the edge, center, and dl fields should be done through
*	provided functions.
*
*	Besides supporting parallel virtual domains,  the precomputation
*	of the grid cell boundaries and centers should provide increased
*	efficiency in many cases.  Also note that the cell centers, edges
*	and spacings are related by the formulas:
*
*	dl[i][j] = edge[i][j+1] - edge[i][j]
*	center[i][j] = 0.5 * (edge[i][j+1] + edge[i][j])
*/

struct _RECT_GRID {
	double L[3];	  /* Lower corner of rectangle containing grid */
	double U[3];	  /* Upper corner of rectangle containing grid */
	double h[3];	  /* Average grid spacings in the grid	       */
	int   gmax[3];	  /* Number of grid blocks 		       */
	int   dim;	  /* Dimension of Grid */

		/* Specifications for virtual domains and variable grids */

	double GL[3];	  /* Lower corner of global grid */
	double GU[3];	  /* Upper corner of global grid */
	double VL[3];	  /* Lower corner of virtual domain */
	double VU[3];	  /* Upper corner of virtual domain */
	int   lbuf[3];	  /* Lower buffer zone width */
	int   ubuf[3];	  /* Upper buffer zone width */

		/* Specifications for variable mesh grids */

	double *edges[3];	/* Coordinate cell edges */
	double *centers[3];	/* Coordinate cell centers */
	double *dh[3];	   	/* Coordindate cell widths */
	double *glstore;	   	/* Storage for edges, centers and dh arrays */
	int   variable_mesh[3]; /* YES for variable dh in ith direction */

	struct _REMAP {
	    GEOMETRY_REMAP remap;
	    const char     *remap_name;
	    const char     *dnm[3],   /* Symbolic names for coord. directions*/
	                   *Dnm[3],
	                   *dirs[3];
	    double          area;      /* Total (Remappped) Computational Area */
	    /*grid cell area*/
	    double          (*Area)(const double*,const struct _RECT_GRID*);
	} Remap;
}; 
typedef struct _RECT_GRID RECT_GRID;

struct _COMM_BOX {
        int lmin[4][2][3];
        int lmax[4][2][3];
        int umin[4][2][3];
        int umax[4][2][3];
        double L0;
        double U0;
        double L1;
        double U1;
        int lx;
        int ly;
        int ux;
        int uy;
        int flag;
};
typedef struct _COMM_BOX COMM_BOX;


struct _RECT_BOX {
	struct _RECT_BOX  *prev;
	struct _RECT_BOX  *next;
	int bmin[MAXD];	  /* Lower bound of box indices */
	int bmax[MAXD];	  /* Upper bound of box indices */
	RECT_GRID *grid;
	int smin[MAXD];
	int smax[MAXD];
	int num_cross;
	struct _Cross *cross[100];
};
typedef struct _RECT_BOX RECT_BOX;




#if defined(__cplusplus)
typedef struct _RECT_GRID::_REMAP REMAP;
#else /* defined(__cplusplus) */
typedef	struct _REMAP REMAP;
#endif /* defined(__cplusplus) */


/*
*		Macros related to RECT_GRID
*	(regular grids only are currently supported)
*/

#define	grid_center_coord(i,gr)	(0.5 * ((gr)->L[i] + (gr)->U[i]))
#define	cell_index(p,i,gr)	irint(floor(((p)-(gr)->L[i])/(gr)->h[i]))
#define	cell_floor(p,i,gr)	irint(floor(((p)-(gr)->L[i])/(gr)->h[i]))
#define	cell_ceil(p,i,gr)	irint(ceil(((p)-(gr)->L[i])/(gr)->h[i]))
#define cell_center(indx,i,gr)	((gr)->L[i] + ((indx) + 0.5)*(gr)->h[i])
#define cell_edge(indx,i,gr)	((gr)->L[i] + (indx)*(gr)->h[i])
#define vd_cell_edge(indx,i,gr) ((gr)->VL[i] + (indx)*(gr)->h[i])
#define cell_width(index,i,gr)	((gr)->h[i])

/*
 * Note on SIDE.  Indicates the side of a hypersurface,  where the positive
 * side is the side of the hypersurface into which the normal points.
 * See the note regarding postive and negative components.
 */

enum _SIDE {
    UNKNOWN_SIDE  = -3,
    POSITIVE_SIDE =  0,
    NEGATIVE_SIDE =  1,
    ONEDGE	  =  2,
    ONVERTEX      =  3,
    COPLANAR      =  4
};
typedef enum _SIDE SIDE;

enum {
	PLUS_CUT  =  1,
	MINUS_CUT = -1
};

IMPORT	double	EPSILON;

 /* Direction names for oriented hypersurfaces */

typedef	enum {
	POSITIVE_ORIENTATION=1,
	NEGATIVE_ORIENTATION=2,
	ORIENTATION_NOT_SET=-1
} ORIENTATION;
typedef enum {
	COUNTER_CLOCK=1,
	CLOCKWISE=2,
	ANGLE_DIRECTION_NOT_SET=-1
} ANGLE_DIRECTION;

#define Opposite_orient(orient)						\
	(((orient)==POSITIVE_ORIENTATION)				\
	 ? NEGATIVE_ORIENTATION : POSITIVE_ORIENTATION)


#define Opposite_ang_dir(dir) ((dir) == CLOCKWISE ? COUNTER_CLOCK : CLOCKWISE)

 /* Angle orientation of curves */

#define curve_ang_oriented_l_to_r(ang_dir,orient)			      \
	((((ang_dir) == CLOCKWISE && (orient) == POSITIVE_ORIENTATION) ||     \
	((ang_dir) == COUNTER_CLOCK && (orient) == NEGATIVE_ORIENTATION)) ?  \
	YES : NO)

#define	Next_m3(n)		(((n) + 1) % 3)

#define	Prev_m3(n)		(((n) + 2) % 3)

/* Debugging printing of angles */

#define print_angle(mesg,ang,end)					\
	(void) printf("%s %g (%g deg)%s",mesg,ang,degrees(ang),end)

/*
*			Fourier Polynomial
*
*	           num_modes-1
*		     -----
*		     \                                 
*	z(p) = z0 +   \     A[k]*sin(<nu[k],p>) + phase[k])
*		      /                                
*		     /
*		     -----
*	             k = 0
*
*/

typedef struct {
	int   num_modes;
	int   dim;
	double **nu;
	double z0;
	double *A, *phase;
	double *L,*U; 		/* domain length in each direction */
} FOURIER_POLY;

/*
*			Legendre Polynomial
*
*		        max_degree
*			 -----
*		         \                                 
*		r(x) =    \     A[n]*P (x)
*			  /           n                    
*			 /
*			 -----
*		         n = 0
*
*	P(x) = Legendre polynomial of degree n.
*	 n
*
*	P(x) = 1
*        0
*
*	P(x) = x
*	 1
*
*	P(x) = 2*x*P(x) - P(x) - [x*P(x) - P(x)]/(n+1)   n >= 2
*	 n+1        n      n-1       n      n-1
*/

typedef struct {
	int   max_degree;
	double *A;
} LEGENDRE_POLY;

typedef struct {
        double *L;
        double *U;
	struct _INTERFACE *intfc;
} BDRY_BOX_PARAMS;

typedef struct {
	double 	      cen[MAXD];		/* Center of Ellipsoid */
	double 	      rad[MAXD];		/* Lengths of radii */
	double 	      ThetaS[2],
		      ThetaE[2];	/* Spherical coords of start and end */
	boolean          closed;		/* Closed ellipsoid if YES */
	ORIENTATION   nor_orient;	/* Specifies inward or outward normal */
	FOURIER_POLY  *fpoly;		/* Fourier Perturbation factors */
	LEGENDRE_POLY *lpoly;		/* Legendre Perturbation factors */
	int 	      dim;		/* Dimension of embedding space */
	RECT_GRID     *gr;
} ELLIP_PARAMS;

typedef struct {
        double N[MAXD];		/* normal of the plane */
        double P[MAXD];		/* a point on the plane */
} PLANE_PARAMS;

typedef struct {
        double x0;
        double x1;
        double y;
        double z;
        double R;
        double rr;
} DUMBBELL_PARAMS;

typedef struct {
	/* equation for line is a*x + b*y = c */
        double a;		
        double b;		
        double c;		
} LINE_PARAMS;

typedef struct {
	/* equation for line is x^2/a^2 + y^2/b^2 = 1 */
        double x0;		
        double y0;		
        double a;		
        double b;		
} ELLIP2D_PARAMS;

typedef struct {
        /* equation for line is x^2/a^2 + y^2/b^2 = 1 */
        double x0;
        double y0;
        double a;
        double b;
        double theta;
} ELLIP2D_TILT_PARAMS;

typedef struct {
        double x[3];
        double y[3];
} TRIANGLE_PARAMS;

typedef struct {
        double x0;
        double y0;
        double a; /*horizontal length */
        double b; /*vertical length */
} RECTANGLE_PARAMS;

typedef struct {
        double x0;
	double x1;
	double x2;
        double y;
        double r0;
        double r1;
	double r2;
} COSMOS_PARAMS;

typedef struct {
        double x0;
        double x1;
        double x2;
	double x3;
	double x4;
        double y;
        double r0;
        double r1;
} TAEGEUK_PARAMS;

typedef struct {
	double x0;
	double x1;
	double y0;
	double y1;
	double a0;
	double a1;
	double b0;
	double b1;
} WING_PARAMS;

typedef struct {
        double x[1];
        double y[1];
	int NofW;
	double r[2];
} PROPELLER_PARAMS;

typedef struct {
	/* equation for line is x^2/a^2 + y^2/b^2 = 1 */
        double x0;		
        double y0;		
        double r;		
        double w;		
        double h;		
} TDISK_PARAMS;

typedef struct {
	int dim;
	int num_cir;
	double **cen;
	double *rad;
} MC_PARAMS;

typedef struct {
        int           num_ellip;
        double         **cen;            /* Center of Ellipsoid 1 */
        double         **rad;            /* Lengths of radii 1 */
        ORIENTATION   nor_orient;       /* Specifies inward or outward normal */        int           dim;              /* Dimension of embedding space */
        RECT_GRID     *gr;
} M_ELLIP_PARAMS;

typedef struct {
                /* Equation: sum((coords[i] - cen[i])^2 = R^2, i=0,...dim-1 */
        int dim;
        boolean add_plan_surf;
        boolean add_perturbation;
        double cen[MAXD];
        double R;
        double H;
	FOURIER_POLY *fpoly;
} CIRCLE_PARAMS;

typedef struct {
        int dim;
        double cen[MAXD];
        double R;
        double r;
        double h;
	double theta;
} PROJECTILE_PARAMS;

typedef struct {
                /* Equation: z = z_0 + A*sin(m*coords[0] + phi) */
        int m;
        double A;
        double phi;
        double z0;
} SINE_PARAMS;

 /* Geometry EXPORTED Function Declarations*/

/*	geomutils.c*/
IMPORT	ANGLE_DIRECTION	fread_angle_direction(FILE*);
IMPORT	SIDE    Opposite_side(const SIDE);
IMPORT	double	dscaled_hypot(const double*,const double*,int);
IMPORT	const char *angle_direction_name(ANGLE_DIRECTION);
IMPORT	const char *orientation_name(ORIENTATION);
IMPORT	const char *side_name(SIDE);
IMPORT	double	_scaled_separation(const double*,const double*,const double*,int);
IMPORT	double	angle(double,double);
IMPORT	double	avg_angle_and_normalize(double,double);
IMPORT	double	cal_angle(const double*,const double*,const double*,
			  int,int,const double*);
IMPORT	double	distance_between_positions(const double*,const double*,int);
IMPORT	double	dscalar_product(const double*,const double*,int);
IMPORT	double	grid_size_in_direction(const double*,const double*,int);
IMPORT	double	mag_vector(const double*,int);
IMPORT	double	normalized_angle(double);
IMPORT	double	random_gaussian(double,double,unsigned short int [3]);
IMPORT	double	scalar_product(const double*,const double*,const int);
IMPORT	double	scaled_hypot(const double*,const double*,int);
IMPORT	double	triple_product(const double*,const double*,const double*,int);
IMPORT	double	vector_product(const double*,const double*,double*,int);
IMPORT	double	vector_product_on_points(const double*,const double*,
					 const double*,int,double*);
IMPORT	int	is_new_angle_smaller(double,double,double,double,int);
IMPORT	int	outside_point(const double*,const double*,const double*,int);
IMPORT	void	affine_fit(const double* const*,int,int,const double*,
			   double*,double**,double*);
IMPORT	void	fprint_angle_direction(FILE*,const char*,ANGLE_DIRECTION,
				       const char*);
IMPORT	void	fprint_general_vector(FILE*,const char*,const double*,
				      int,const char*);
IMPORT	void	fprint_orientation(FILE*,const char*,ORIENTATION,const char*);
IMPORT	void	sprint_general_vector(char*,const char*,const double*,
				      int,const char*);
IMPORT	void	print_angle_direction(const char*,ANGLE_DIRECTION,const char*);
IMPORT	void	print_general_vector(const char*,const double*,int,const char*);
IMPORT	void	print_side(const char*,SIDE,const char*);
IMPORT	void	print_orientation(const char*,ORIENTATION,const char*);
IMPORT	double	plane_angle(double*,double*);

/*	igrid.c*/
IMPORT	GEOMETRY_REMAP	read_remap_from_string(const char*);
IMPORT	REMAP	*remap_info(void);
IMPORT	boolean	adjust_top_grid_for_square(RECT_GRID*,const RECT_GRID*);
IMPORT	boolean 	point_in_buffer(const double*,const RECT_GRID*);
IMPORT	boolean 	is_rotational_symmetry(void);
IMPORT	boolean	rect_in_which(const double*,int*,const RECT_GRID*);
IMPORT	double	grid_tolerance(const RECT_GRID*);
IMPORT	double	ident_Area(const double*,const RECT_GRID*);
IMPORT	int	set_grid_lines(RECT_GRID*);
IMPORT	void	set_rotational_symmetry(boolean);
IMPORT	void	copy_rect_grid(RECT_GRID*,const RECT_GRID*);
IMPORT	void	fprint_rectangular_grid(FILE*,const RECT_GRID*);
IMPORT	void	free_grid_lines(RECT_GRID*);
IMPORT	void	i_init_remap_and_rect_grid(RECT_GRID*);
IMPORT	void	i_print_remap_values(void);
IMPORT	void	print_RECT_GRID_structure(const RECT_GRID*);
IMPORT	void	print_rectangular_grid(const RECT_GRID*);
IMPORT	void	read_rectangular_grid(const IO_TYPE*,RECT_GRID*,boolean,REMAP*);
IMPORT	void	set_dual_grid(RECT_GRID*,const RECT_GRID*);
IMPORT	void	set_rect_grid(const double*,const double*,const double*,
			      const double*,const int*,const int*,const int*,
			      int,const REMAP*,RECT_GRID*);
IMPORT	void	set_remap(int,GEOMETRY_REMAP,REMAP*);
IMPORT	void	set_remap_identity(int,GEOMETRY_REMAP);
IMPORT	void	zoom_rect_grid(RECT_GRID*,const RECT_GRID*);
IMPORT  void	rect_grid_corner(const RECT_GRID*,const int*,double*);
IMPORT  void	rect_grid_center(const RECT_GRID*,const int*,double*);
IMPORT	void	init_topological_grid(RECT_GRID*,const RECT_GRID*);
IMPORT  void    set_remap_and_rect_grid(double*, double*,int*,GEOMETRY_REMAP,
					RECT_GRID*);

#if defined(c_plusplus) || defined(__cplusplus)
}
#endif


#endif /* !defined(_IGEOM_H) */
